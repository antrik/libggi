/* $Id: line.c,v 1.2 2005/07/30 11:39:57 cegger Exp $
******************************************************************************

   LibGGI - Millennium II acceleration for fbdev target

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

#include "ati_mach64.h"


int GGI_ati_mach64_drawline(ggi_visual *vis, int x, int y, int x2, int y2)
{
	struct ati_mach64_priv *priv = ATI_MACH64_PRIV(vis);
        int dx,dy;
	uint32_t dst_cntl;
	uint32_t small,large;
	uint32_t err,inc,dec;

	if (vis->w_frame_num) {
		y += vis->w_frame_num * LIBGGI_VIRTY(vis);
		y2 += vis->w_frame_num * LIBGGI_VIRTY(vis);
	}
	dst_cntl=DST_LAST_PEL;

	/* Determine x & y deltas and x & y direction bits. */
	dx=x-x2;
	if (dx<0) {
    	    dx=-dx;
	    dst_cntl|=DST_X_LEFT_TO_RIGHT;
	};
	dy=y-y2;
	if (dy<0) {
    	    dy=-dy;
	    dst_cntl|=DST_Y_TOP_TO_BOTTOM;
	};
	/* Determine x & y min and max values; also determine y major bit. */
	if (dx<dy) {
    	    small=dx;
    	    large=dy;
    	    dst_cntl|=DST_Y_MAJOR;
	} else {
    	    small=dy;
    	    large=dx;
        };

	/* Calculate Bresenham parameters and draw line. */
        inc = 2*small;
	err = (2*small) - large;
        dec = 0x3ffff - (2 * (large - small));

        /* Allow setting of last pel bit and polygon outline bit for line drawing. */
        set_dst_cntl(priv,dst_cntl);

        /* Wait for idle before reading GUI registers. */
        wait_for_fifo(5,priv);

        /* Draw Bresenham line. */
        aty_st_le32(DST_Y_X,((uint32_t) x << 16) | (uint32_t) y,priv);
        aty_st_le32(DST_BRES_ERR,err,priv);
        aty_st_le32(DST_BRES_INC,inc,priv);
        aty_st_le32(DST_BRES_DEC,dec,priv);
        aty_st_le32(DST_BRES_LNTH,large + 1,priv);

	vis->accelactive=1;

	return 0;
}
