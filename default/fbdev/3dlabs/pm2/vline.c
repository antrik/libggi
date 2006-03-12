/* $Id: vline.c,v 1.4 2006/03/12 23:15:04 soyt Exp $
******************************************************************************

   LibGGI - 3DLabs Permedia 2 acceleration for fbdev target

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


int GGI_3dlabs_pm2_drawvline(struct ggi_visual *vis, int x, int y, int h)
{
	struct _3dlabs_pm2_priv *priv = PM2_PRIV(vis);
	volatile uint8_t *mmioaddr = FBDEV_PRIV(vis)->mmioaddr;
	int yadd = vis->w_frame_num * LIBGGI_VIRTY(vis);

	DPRINT_DRAW("drawvline(%p, %i,%i, %i) entered\n",
		vis, x,y,h);

	y += yadd;
	
	pm2_gcupdate(mmioaddr, priv, LIBGGI_MODE(vis), LIBGGI_GC(vis),
		     yadd);

	pm2_waitfifo(mmioaddr, 6);
	pm2_out32(mmioaddr, x << 16, PM2R_START_X_DOM);
	pm2_out32(mmioaddr, y << 16, PM2R_START_Y);

	/* Vertical */
	pm2_out32(mmioaddr, 0 << 16, PM2R_D_X_DOM);
	pm2_out32(mmioaddr, 1 << 16, PM2R_D_Y);

	pm2_out32(mmioaddr, h, PM2R_COUNT);
	pm2_out32(mmioaddr, PM2F_RENDER_LINE
		| PM2F_RENDER_XPOSITIVE | PM2F_RENDER_YPOSITIVE,
		PM2R_RENDER);

	vis->accelactive = 1;

	return 0;
}


int GGI_3dlabs_pm2_putvline(struct ggi_visual *vis, int x, int y, int h,
			const void *buf)
{
	struct _3dlabs_pm2_priv *priv = PM2_PRIV(vis);
	volatile uint8_t *mmioaddr = FBDEV_PRIV(vis)->mmioaddr;
	int yadd = vis->w_frame_num * LIBGGI_VIRTY(vis);
	int count;
#if 0
	uint32_t srcwidth;
#endif
	const uint8_t *src;
	const uint32_t *srcp;
	uint32_t *dest;


	DPRINT_DRAW("putvline(%p, %i,%i, %i, %p) entered\n",
		vis, x,y,h, buf);

	/* 0 height not OK */
	if (h == 0) return 0;

	y += yadd;
	
	pm2_gcupdate(mmioaddr, priv, LIBGGI_MODE(vis), LIBGGI_GC(vis),
		     yadd);

	pm2_waitfifo(mmioaddr, 6);

	/* Setting for Image download from Host */
	pm2_out32(mmioaddr, priv->pprod, PM2R_FB_READ_MODE);
	pm2_out32(mmioaddr, UNIT_ENABLE, PM2R_FB_WRITE_MODE);
	pm2_out32(mmioaddr, UNIT_DISABLE, PM2R_COLOR_DDA_MODE);

	pm2_loadcoord(mmioaddr, x,y, 1,h);
	pm2_out32(mmioaddr,
		PM2F_RENDER_RECTANGLE | PM2F_RENDER_XPOSITIVE
		| PM2F_RENDER_YPOSITIVE | PM2F_RENDER_SYNC_ON_HOST,
		PM2R_RENDER);

	src = (const uint8_t *)buf;
	dest = (uint32_t *)((volatile uint8_t *)
			(mmioaddr + PM2R_OUT_FIFO + 4));

	count = h;
	srcp = (const uint32_t *)src;
	while (count >= priv->fifosize) {
		pm2_waitfifo(mmioaddr, priv->fifosize);
		/* 0x0155 is the TAG for FBSourceData */
		pm2_out32(mmioaddr,
			((priv->fifosize - 2) << 16) | 0x0155,
			PM2R_OUT_FIFO);

		pm2_move32(dest, srcp, priv->fifosize - 1);

		count -= priv->fifosize - 1;
		srcp += priv->fifosize - 1;
	}
	if (count) {
		pm2_waitfifo(mmioaddr, count + 1);
		pm2_out32(mmioaddr,
			((count - 1) << 16) | 0x0155,
			PM2R_OUT_FIFO);

		pm2_move32(dest, srcp, count);
	}

	pm2_waitfifo(mmioaddr, 2);
	pm2_out32(mmioaddr, 0, PM2R_WAIT_FOR_COMPLETION);


	priv->oldfgcol = ~priv->oldfgcol;

	vis->accelactive = 1;

	return 0;
}
