/* $Id: gtext.c,v 1.2 2003/05/03 16:39:18 cegger Exp $
******************************************************************************

   LibGGI - ATI Mach64 acceleration for fbdev target

   Copyright (C) 1999 Marcus Sundberg	[marcus@ggi-project.org]

   Permission is hereby granted, free of charge, to any person obtaining a
   copy of this software and associated documentation files (the "Software"),
   to deal in the Software without restriction, including without limitation
   the rights to use, copy, modify, merge, publish, distribute, sublicense,
   and/or sell copies of the Software, and to permit persons to whom the
   Software is furnished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be included in
   all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
   THE AUTHOR(S) BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
   IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
   CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

******************************************************************************
*/

#include <string.h>
#include "ati_mach64.h"

/*
 * The idea:
 *
 * For slow characters we setup the engine for monochrome expansion. The
 * source of the data is set to the host data port.
 * We then write the monochrome character to the host data port
 *
 * For fast characters the font is stored in offscreen video ram.
 * We setup the engine for monochrome expansion, but this time the source
 * of the data is set to a strict linear video ram source, so the 
 * engine copies the data from video ram
 */

static inline void draw_char(int hostwrites, uint32 *cdat,
                             struct ati_mach64_priv *priv)
{
    /* feed a character to the engine */
    while (hostwrites>0) {
        wait_for_fifo(1,priv);
        aty_st_le32(HOST_DATA0,*cdat,priv);
        cdat++;
        hostwrites--;
    };
}

int GGI_ati_mach64_getcharsize(ggi_visual *vis, int *width, int *height)
{
	/* The stubs' font is 8x8, so that is what we return */
	*width = FWIDTH;
	*height = FHEIGHT;

	return 0;
}

int GGI_ati_mach64_putc(ggi_visual *vis, int x, int y, char c)
{
	struct ati_mach64_priv *priv = ATI_MACH64_PRIV(vis);
	int hostwrites;
	uint32 *cdat;

	/* Tell the engine what we want to do. */
	set_dp_src(priv,MONO_SRC_HOST | FRGD_SRC_FRGD_CLR | BKGD_SRC_BKGD_CLR);
	/* Auto update x position */
	set_dst_cntl(priv,DST_LAST_PEL | DST_Y_TOP_TO_BOTTOM |
                     DST_X_LEFT_TO_RIGHT | DST_X_TILE);
	hostwrites=(FWIDTH*FHEIGHT+31)/32;
	/* Set destination location */
	wait_for_fifo(2,priv);
	aty_st_le32(DST_Y_X,x << 16 | y,priv);
    	aty_st_le32(DST_HEIGHT_WIDTH,(FWIDTH<<16)|FHEIGHT,priv); /* Initiates operation. */

    	/* Feed the data to the engine */
    	cdat = (uint32 *)(priv->font + c * FHEIGHT);
    	draw_char(hostwrites,cdat,priv);
	return 0;
}

int GGI_ati_mach64_puts(ggi_visual *vis, int x, int y, const char *str)
{
	struct ati_mach64_priv *priv = ATI_MACH64_PRIV(vis);
	int count,hostwrites;
	uint32 *cdat;

	/* Tell the engine what we want to do. */
	set_dp_src(priv,MONO_SRC_HOST | FRGD_SRC_FRGD_CLR | BKGD_SRC_BKGD_CLR);
	/* Auto update x position */
	set_dst_cntl(priv,DST_LAST_PEL | DST_Y_TOP_TO_BOTTOM |
                     DST_X_LEFT_TO_RIGHT | DST_X_TILE);
	/* Set destination location */
	wait_for_fifo(2,priv);
	aty_st_le32(DST_Y_X,x << 16 | y,priv);
	aty_st_le32(DST_HEIGHT,FHEIGHT,priv);

	hostwrites=(FWIDTH*FHEIGHT+31)/32;
	count=0;
	while (*str!=0) {
    	    wait_for_fifo(1,priv);
    	    aty_st_le32(DST_WIDTH,FWIDTH,priv); /* Initiates operation. */

    	    /* Feed the data to the engine */
    	    cdat = (uint32 *)(priv->font + *str * FHEIGHT);
    	    draw_char(hostwrites,cdat,priv);

    	    count++;
    	    str++;
	};
	return count;
};

int GGI_ati_mach64_fastputc(ggi_visual *vis, int x, int y, char c)
{
	struct ati_mach64_priv *priv = ATI_MACH64_PRIV(vis);
	uint32 pitch,pix_width;

	/* Tell the engine what we want to do. */
	set_dp_src(priv,MONO_SRC_BLIT | FRGD_SRC_FRGD_CLR | BKGD_SRC_BKGD_CLR);
	/* Auto update x position */
	set_dst_cntl(priv,DST_Y_TOP_TO_BOTTOM | DST_X_LEFT_TO_RIGHT | DST_X_TILE);
	/* Set destination location */
	wait_for_fifo(4,priv);
	aty_st_le32(DST_Y_X,x << 16 | y,priv);
	aty_st_le32(DST_HEIGHT,FHEIGHT,priv);
	/* Set the source location & pixel depth */
	aty_st_le32(SRC_Y_X,0,priv);
	pix_width=aty_ld_le32(DP_PIX_WIDTH,priv);
        aty_st_le32(DP_PIX_WIDTH,(pix_width & 0xfffff0ff) | SRC_1BPP,priv);

	pitch=aty_ld_le32(SRC_OFF_PITCH,priv) & 0xffc00000;
    	wait_for_fifo(5,priv);
	aty_st_le32(SRC_OFF_PITCH,
		    pitch|((priv->fontoffset/8) + c),priv);
	aty_st_le32(SRC_HEIGHT1_WIDTH1,FWIDTH*FHEIGHT<<16|1,priv);
    	aty_st_le32(DST_WIDTH,FWIDTH,priv); /* Initiates operation. */
	aty_st_le32(SRC_OFF_PITCH,pitch,priv);
	aty_st_le32(DP_PIX_WIDTH,pix_width,priv);
	vis->accelactive = 1;
	return 0;
}


int GGI_ati_mach64_fastputs(ggi_visual *vis, int x, int y, const char *str)
{
	struct ati_mach64_priv *priv = ATI_MACH64_PRIV(vis);
	int count;
	uint32 pitch,pix_width;

	/* Tell the engine what we want to do. */
	set_dp_src(priv,MONO_SRC_BLIT | FRGD_SRC_FRGD_CLR | BKGD_SRC_BKGD_CLR);
	/* Auto update x position */
	set_dst_cntl(priv,DST_Y_TOP_TO_BOTTOM | DST_X_LEFT_TO_RIGHT | DST_X_TILE);
	/* Set destination location */
	wait_for_fifo(5,priv);
	aty_st_le32(DST_Y_X,x << 16 | y,priv);
	aty_st_le32(DST_HEIGHT,FHEIGHT,priv);
	/* Set the source location & pixel depth */
	aty_st_le32(SRC_HEIGHT1,1,priv);
	aty_st_le32(SRC_Y_X,0,priv);
	pix_width=aty_ld_le32(DP_PIX_WIDTH,priv);
        aty_st_le32(DP_PIX_WIDTH,(pix_width & 0xfffff0ff) | SRC_1BPP,priv);

	count=0;
	pitch=aty_ld_le32(SRC_OFF_PITCH,priv) & 0xffc00000;
	while (*str!=0) {
    	    wait_for_fifo(3,priv);
	    aty_st_le32(SRC_OFF_PITCH,
			pitch|((priv->fontoffset/8) + *str),priv);
	    aty_st_le32(SRC_WIDTH1,FWIDTH*FHEIGHT,priv);
    	    aty_st_le32(DST_WIDTH,FWIDTH,priv); /* Initiates operation. */

    	    count++;
    	    str++;
	};
	wait_for_fifo(2,priv);
	aty_st_le32(SRC_OFF_PITCH,pitch,priv);
	aty_st_le32(DP_PIX_WIDTH,pix_width,priv);
	vis->accelactive = 1;
	return count;
}
