/* $Id: box.c,v 1.1 2005/12/30 23:39:35 cegger Exp $
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


int GGI_3dlabs_pm2_drawbox(ggi_visual *vis, int x, int y, int w, int h)
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
#endif		
		vis->accelactive = 1;
	}

	return 0;
}


int GGI_3dlabs_pm2_fillscreen(ggi_visual *vis)
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
