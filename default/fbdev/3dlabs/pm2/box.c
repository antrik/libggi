/* $Id: box.c,v 1.4 2006/03/12 23:15:04 soyt Exp $
******************************************************************************

   LibGGI - 3Dlabs Permedia 2 acceleration for fbdev target

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


int GGI_3dlabs_pm2_drawbox(struct ggi_visual *vis, int x, int y, int w, int h)
{
	if (w > 0 && h > 0) {	/* 0 width is not OK! */
		struct _3dlabs_pm2_priv *priv = PM2_PRIV(vis);
		volatile uint8_t *mmioaddr = FBDEV_PRIV(vis)->mmioaddr;
		int yadd = vis->w_frame_num * LIBGGI_VIRTY(vis);

		DPRINT_DRAW("drawbox(%p, %i,%i, %i,%i) entered\n",
			vis, x, y, w, h);

		y += yadd;

		pm2_gcupdate(mmioaddr, priv, LIBGGI_MODE(vis), LIBGGI_GC(vis),
				yadd);

#if 1
		pm2_waitfifo(mmioaddr, 3);
		pm2_loadcoord(mmioaddr, x, y, w, h);
		pm2_out32(mmioaddr,
			PM2F_RENDER_RECTANGLE | PM2F_RENDER_XPOSITIVE
			| PM2F_RENDER_YPOSITIVE,
			PM2R_RENDER);

#else

		pm2_waitfifo(mmioaddr, 5);
		pm2_out32(mmioaddr, UNIT_DISABLE, PM2R_COLOR_DDA_MODE);
		pm2_out32(mmioaddr, priv->pprod, PM2R_FB_READ_MODE);
		/* above pm2_gcupdate set fgcolor */
		pm2_loadcoord(mmioaddr, x,y, w,h);
		pm2_out32(mmioaddr,
			PM2F_RENDER_RECTANGLE | PM2F_RENDER_XPOSITIVE
			| PM2F_RENDER_YPOSITIVE /* | PM2F_RENDER_FASTFILL */,
			PM2R_RENDER);

		priv->oldfgcol = ~priv->oldfgcol;
#endif		
		vis->accelactive = 1;
	}

	return 0;
}


int GGI_3dlabs_pm2_fillscreen(struct ggi_visual *vis)
{
	struct _3dlabs_pm2_priv *priv = PM2_PRIV(vis);
	volatile uint8_t *mmioaddr = FBDEV_PRIV(vis)->mmioaddr;
	int virtx = LIBGGI_VIRTX(vis);
	int virty = LIBGGI_VIRTY(vis);
	int yadd = vis->w_frame_num * LIBGGI_VIRTY(vis);

	DPRINT_DRAW("fillscreeen(%p) entered\n", vis);
	/* yadd is there for the correct frame */

	pm2_gcupdate(mmioaddr, priv, LIBGGI_MODE(vis),
		     LIBGGI_GC(vis), yadd);

	pm2_waitfifo(mmioaddr, 3);
	pm2_loadcoord(mmioaddr, 0, yadd, virtx, virty+yadd);
	pm2_out32(mmioaddr,
		PM2F_RENDER_RECTANGLE | PM2F_RENDER_XPOSITIVE
		| PM2F_RENDER_YPOSITIVE /* | PM2F_RENDER_FASTFILL */,
		PM2R_RENDER);

	vis->accelactive = 1;

	return 0;
}


int GGI_3dlabs_pm2_putbox(struct ggi_visual *vis, int x, int y, int w, int h,
			const void *buf)
{
	struct _3dlabs_pm2_priv *priv = PM2_PRIV(vis);
	volatile uint8_t *mmioaddr = FBDEV_PRIV(vis)->mmioaddr;
	int yadd = vis->w_frame_num * LIBGGI_VIRTY(vis);
	int count;
	uint32_t srcwidth;
	const uint8_t *src;
	const uint32_t *srcp;
	uint32_t *dest;

	/* 0 width is not OK! */
	if (w == 0 || h == 0) return 0;


	DPRINT_DRAW("putbox(%p, %i,%i, %i,%i, %p) entered\n",
		vis, x, y, w, h, buf);

	y += yadd;

	if (LIBGGI_GT(vis) & GT_SUB_PACKED_GETPUT) {
		srcwidth = GT_ByPPP(w, LIBGGI_GT(vis));
	} else {
		srcwidth = w * GT_ByPP(LIBGGI_GT(vis));
	}

	pm2_gcupdate(mmioaddr, priv, LIBGGI_MODE(vis), LIBGGI_GC(vis),
			yadd);

	pm2_waitfifo(mmioaddr, 6);

	/* Setting for Image download from Host */
	pm2_out32(mmioaddr, priv->pprod, PM2R_FB_READ_MODE);
	pm2_out32(mmioaddr, UNIT_ENABLE, PM2R_FB_WRITE_MODE);
	pm2_out32(mmioaddr, UNIT_DISABLE, PM2R_COLOR_DDA_MODE);

	pm2_loadcoord(mmioaddr, x,y, w,h);
	pm2_out32(mmioaddr,
		PM2F_RENDER_RECTANGLE | PM2F_RENDER_XPOSITIVE
		| PM2F_RENDER_YPOSITIVE | PM2F_RENDER_SYNC_ON_HOST,
		PM2R_RENDER);

	src = (const uint8_t *)buf;
	dest = (uint32_t *)((volatile uint8_t *)
			(mmioaddr + PM2R_OUT_FIFO + 4));

	while (h--) {
		count = w;
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
		src += srcwidth;
	}

#if 1
	/* Re-enable fb readmode when done */
	pm2_waitfifo(mmioaddr, 2);
	pm2_out32(mmioaddr, 0, PM2R_WAIT_FOR_COMPLETION);
	pm2_out32(mmioaddr, priv->pprod, PM2R_FB_READ_MODE);
#else
	pm2_waitidle(mmioaddr);
#endif

	priv->oldfgcol = ~priv->oldfgcol;

	vis->accelactive = 1;

	return 0;
}


int GGI_3dlabs_pm2_getbox(struct ggi_visual *vis, int x, int y, int w, int h,
			void *buf)
{
	struct _3dlabs_pm2_priv *priv = PM2_PRIV(vis);
	volatile uint8_t *mmioaddr = FBDEV_PRIV(vis)->mmioaddr;
	int yadd = vis->r_frame_num * LIBGGI_VIRTY(vis);
	int count;
	uint32_t destwidth;
	const uint32_t *src;
	uint8_t *dest;
	uint32_t *destp;

	/* 0 width is not OK! */
	if (w == 0 || h == 0) return 0;

	DPRINT_DRAW("getbox(%p, %i,%i, %i,%i, %p) entered\n",
		vis, x, y, w, h, buf);

	y += yadd;

	if (LIBGGI_GT(vis) & GT_SUB_PACKED_GETPUT) {
		destwidth = GT_ByPPP(w, LIBGGI_GT(vis));
	} else {
		destwidth = w * GT_ByPP(LIBGGI_GT(vis));
	}

	pm2_gcupdate(mmioaddr, priv, LIBGGI_MODE(vis), LIBGGI_GC(vis),
			yadd);

	pm2_waitfifo(mmioaddr, 6);

	/* Setting for Image upload to Host */
	pm2_out32(mmioaddr, priv->pprod | PM2F_FB_READ_DEST_ENABLE,
		PM2R_FB_READ_MODE);
	pm2_out32(mmioaddr, UNIT_DISABLE, PM2R_FB_WRITE_MODE);
	pm2_out32(mmioaddr, UNIT_DISABLE, PM2R_COLOR_DDA_MODE);

	pm2_loadcoord(mmioaddr, x,y, w,h);
	pm2_out32(mmioaddr,
		PM2F_RENDER_RECTANGLE | PM2F_RENDER_XPOSITIVE
		| PM2F_RENDER_YPOSITIVE | PM2F_RENDER_SYNC_ON_HOST,
		PM2R_RENDER);

	dest = (uint8_t *)buf;
	src = (const uint32_t *)((volatile uint8_t *)
			(mmioaddr + PM2R_OUT_FIFO + 4));

#if 0
	while (h--) {
		count = w;
		destp = (const uint32_t *)src;
		while (count >= priv->fifosize) {
			pm2_waitfifo(mmioaddr, priv->fifosize);
			/* 0x0153 is the TAG for FBColor */
			pm2_out32(mmioaddr,
				((priv->fifosize - 2) << 16) | 0x0153,
				PM2R_OUT_FIFO);

			pm2_move32(destp, src, priv->fifosize - 1);

			count -= priv->fifosize - 1;
			destp += priv->fifosize - 1;
		}
		if (count) {
			pm2_waitfifo(mmioaddr, count + 1);
			pm2_out32(mmioaddr,
				((count - 1) << 16) | 0x0153,
				PM2R_OUT_FIFO);

			pm2_move32(destp, src, count);
		}
		dest += destwidth;
	}
#endif

	/* Re-enable fb read/write mode when done */
	pm2_waitfifo(mmioaddr, 3);
	pm2_out32(mmioaddr, 0, PM2R_WAIT_FOR_COMPLETION);
	pm2_out32(mmioaddr, priv->pprod, PM2R_FB_READ_MODE);
	pm2_out32(mmioaddr, UNIT_ENABLE, PM2R_FB_WRITE_MODE);

	priv->oldfgcol = ~priv->oldfgcol;

	return 0;
}
