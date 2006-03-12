/* $Id: line.c,v 1.2 2006/03/12 23:15:04 soyt Exp $
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

#if 0
int GGI_3dlabs_pm2_drawline(struct ggi_visual *vis, int x, int y, int x2, int y2)
{
	struct _3dlabs_pm2_priv *priv = PM2_PRIV(vis);
	volatile uint8_t *mmioaddr = FBDEV_PRIV(vis)->mmioaddr;
	int yadd = vis->w_frame_num * LIBGGI_VIRTY(vis);
	int dx, dy, len;

	DPRINT_DRAW("drawline(%p, %i,%i, %i,%i) entered\n",
		vis, x,y, x2,y2);

	dx = x2 - x;
	dy = y2 - y;

	if (dx == 0) {
		return GGI_3dlabs_pm2_drawhline(vis, x,y, abs(dx));
	}
	if (dy == 0) {
		return GGI_3dlabs_pm2_drawvline(vis, x,y, abs(dy));
	}

	if (dx >= dy) {	/* x major */
		len = dx / dy;
	} else {	/* y major */
		len = dy / dx;
	}

	y += yadd;
	y2 += yadd;

	pm2_gcupdate(mmioaddr, priv, LIBGGI_MODE(vis), LIBGGI_GC(vis),
		     yadd);

	pm2_waitfifo(mmioaddr, 6);
	pm2_out32(mmioaddr, x << 16, PM2R_START_X_DOM);
	pm2_out32(mmioaddr, y << 16, PM2R_START_Y);

	pm2_out32(mmioaddr, dx << 16, PM2R_D_X_DOM);
	pm2_out32(mmioaddr, dy << 16, PM2R_D_Y);

	pm2_out32(mmioaddr, len, PM2R_COUNT);
	pm2_out32(mmioaddr, PM2F_RENDER_LINE, PM2R_RENDER);

	vis->accelactive = 1;

	return 0;
}
#endif
