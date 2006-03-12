/* $Id: copybox.c,v 1.2 2006/03/12 23:15:04 soyt Exp $
******************************************************************************

   LibGGI - Permedia2 acceleration for fbdev target

   Copyright (C) 2005	Christoph Egger

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

#include "3dlabs_pm2.h"


int GGI_3dlabs_pm2_copybox(struct ggi_visual *vis, int x, int y, int w, int h,
		       int dstx, int dsty)
{
	struct _3dlabs_pm2_priv *priv = PM2_PRIV(vis);
	volatile uint8_t *mmioaddr = FBDEV_PRIV(vis)->mmioaddr;
	int yadd = vis->w_frame_num * LIBGGI_VIRTY(vis);
	uint8_t align;
	unsigned int xdir = 0, ydir = 0;

	DPRINT_DRAW("copybox(%p, %i,%i, %i,%i, %i,%i) entered\n",
		vis, x,y, w,h, dstx, dsty);

	dsty += yadd;
	y += vis->r_frame_num*LIBGGI_VIRTY(vis);

	pm2_gcupdate(mmioaddr, priv, LIBGGI_MODE(vis),
		     LIBGGI_GC(vis), yadd);

	align = (dstx & priv->bppalign) - (x & priv->bppalign);

	if (x >= dstx) xdir |= PM2F_RENDER_XPOSITIVE;
	if (y >= dsty) ydir |= PM2F_RENDER_YPOSITIVE;

#if 1
	pm2_waitfifo(mmioaddr, 6);
	pm2_out32(mmioaddr, UNIT_DISABLE, PM2R_COLOR_DDA_MODE);
	pm2_out32(mmioaddr, priv->pprod | PM2F_FB_READ_SOURCE_ENABLE
			/* | PM2F_FB_READ_DEST_ENABLE */,
		PM2R_FB_READ_MODE);
	pm2_loadcoord(mmioaddr, dstx, dsty, w, h);
	pm2_out32(mmioaddr, (((y-dsty) & 0x0fff) << 16)
			| ((x-dstx) & 0x0fff),
		PM2R_FB_SOURCE_DELTA);
	pm2_out32(mmioaddr, PM2F_RENDER_RECTANGLE | xdir | ydir, PM2R_RENDER);

#else

	/* This causes strange effects in XGGI when moving an xterm
	 * window left/right */
	pm2_waitfifo(mmioaddr, 7);
	pm2_out32(mmioaddr, UNIT_DISABLE, PM2R_COLOR_DDA_MODE);
	pm2_out32(mmioaddr, priv->pprod | PM2F_FB_READ_SOURCE_ENABLE
		| PM2F_FB_PACKED,
		PM2R_FB_READ_MODE);
	pm2_loadcoord(mmioaddr, dstx >> priv->bppshift, dsty,
			(w + 7) >> priv->bppshift, h);
	pm2_out32(mmioaddr, align << 29 | dstx << 16 | (dstx + w),
		PM2R_PACKED_DATA_LIMITS);
	pm2_out32(mmioaddr, (((y-dsty) & 0x0fff) << 16)
		| (((x & ~priv->bppalign) - (dstx & ~priv->bppalign)) & 0x0fff),
		PM2R_FB_SOURCE_DELTA);
	pm2_out32(mmioaddr, PM2F_RENDER_RECTANGLE | xdir | ydir,
		PM2R_RENDER);
#endif

	vis->accelactive = 1;

	/* enforce setting of a new fg color 
	 * to re-enable PM2R_COLOR_DDA_MODE when needed
	 */
	priv->oldfgcol = ~priv->oldfgcol;

	return 0;
}
