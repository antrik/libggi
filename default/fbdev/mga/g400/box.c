/* $Id: box.c,v 1.1 2001/06/17 01:58:44 ggibecka Exp $
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

#include "mga_g400.h"


int GGI_mga_g400_drawbox(ggi_visual *vis, int x, int y, int w, int h)
{
	if (w > 0 && h > 0) {	/* 0 width is not OK! */
		struct mga_g400_priv *priv = MGA_G400_PRIV(vis);
		volatile uint8 *mmioaddr = FBDEV_PRIV(vis)->mmioaddr;
		int yadd = vis->w_frame_num * LIBGGI_VIRTY(vis);

		y += yadd;

		mga_gcupdate(mmioaddr, priv, LIBGGI_MODE(vis), LIBGGI_GC(vis),
			     LIBGGI_VIRTX(vis), yadd);

		if (priv->dwgctl != priv->drawboxcmd) {
			mga_waitfifo(mmioaddr, 3);
			mga_setdwgctl(mmioaddr, priv, priv->drawboxcmd);
		} else {
			mga_waitfifo(mmioaddr, 2);
		}
		mga_out32(mmioaddr, (RS16(x + w) << 16) | RS16(x), FXBNDRY);
		mga_out32(mmioaddr, (RS16(y) << 16) | RS16(h),
			  YDSTLEN | EXECUTE);

		vis->accelactive = 1;
	}

	return 0;
}


int GGI_mga_g400_fillscreen(ggi_visual *vis)
{
	struct mga_g400_priv *priv = MGA_G400_PRIV(vis);
	volatile uint8 *mmioaddr = FBDEV_PRIV(vis)->mmioaddr;
	int virtx = LIBGGI_VIRTX(vis);
	int virty = LIBGGI_VIRTY(vis);
	int yadd = vis->w_frame_num * LIBGGI_VIRTY(vis);

	mga_gcupdate(mmioaddr, priv, LIBGGI_MODE(vis),
		     LIBGGI_GC(vis), LIBGGI_VIRTX(vis), yadd);

	if (priv->dwgctl != priv->drawboxcmd) {
		mga_waitfifo(mmioaddr, 3);
		mga_setdwgctl(mmioaddr, priv, priv->drawboxcmd);
	} else {
		mga_waitfifo(mmioaddr, 2);
	}
	mga_out32(mmioaddr, RS16(virtx) << 16, FXBNDRY);
	mga_out32(mmioaddr, (RS16(yadd) << 16) | RS16(virty + yadd),
		  YDSTLEN | EXECUTE);

	vis->accelactive = 1;

	return 0;
}
