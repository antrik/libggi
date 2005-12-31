/* $Id: hline.c,v 1.2 2005/12/31 22:22:01 cegger Exp $
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


int GGI_3dlabs_pm2_drawhline(ggi_visual *vis, int x, int y, int w)
{
	struct _3dlabs_pm2_priv *priv = PM2_PRIV(vis);
	volatile uint8_t *mmioaddr = FBDEV_PRIV(vis)->mmioaddr;
	int yadd = vis->w_frame_num * LIBGGI_VIRTY(vis);

#if 0
	DPRINT_DRAW("drawhline(%p, %i,%i, %i) entered\n",
		vis, x,y, w);
#endif
	y += yadd;

	pm2_gcupdate(mmioaddr, priv, LIBGGI_MODE(vis), LIBGGI_GC(vis),
		     yadd);

	pm2_waitfifo(mmioaddr, 6);
	pm2_out32(mmioaddr, x << 16, PM2R_START_X_DOM);
	pm2_out32(mmioaddr, y << 16, PM2R_START_Y);

	/* Horizontal */
	pm2_out32(mmioaddr, 1 << 16, PM2R_D_X_DOM);
	pm2_out32(mmioaddr, 0 << 16, PM2R_D_Y);

	pm2_out32(mmioaddr, w, PM2R_COUNT);
	pm2_out32(mmioaddr, PM2F_RENDER_LINE
		| PM2F_RENDER_XPOSITIVE | PM2F_RENDER_YPOSITIVE,
		PM2R_RENDER);

	vis->accelactive = 1;

	return 0;
}
