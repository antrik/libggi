/* $Id: hline.c,v 1.3 2005/07/30 11:39:57 cegger Exp $
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

#include "m2164w.h"


int GGI_m2164w_drawhline(ggi_visual *vis, int x, int y, int w)
{
	struct m2164w_priv *priv = M2164W_PRIV(vis);
	volatile uint8_t *mmioaddr = FBDEV_PRIV(vis)->mmioaddr;
	int yadd = vis->w_frame_num * LIBGGI_VIRTY(vis);
	uint32_t dwgctl;

	y += yadd;
	y = RS16(y) << 16;

	dwgctl = OP_AUTOLINE_CLOSE | SOLID | SHFTZERO | BOP_COPY |
		BLTMOD_BFCOL;

	mga_gcupdate(mmioaddr, priv, LIBGGI_MODE(vis), LIBGGI_GC(vis),
		     LIBGGI_VIRTX(vis), yadd);

	if (priv->dwgctl != dwgctl) {
		mga_waitfifo(mmioaddr, 3);
		mga_setdwgctl(mmioaddr, priv, dwgctl);
	} else {
		mga_waitfifo(mmioaddr, 2);
	}
	/* y has been shifted above */
	mga_out32(mmioaddr, (unsigned)RS16(x) | y, XYSTRT);
	mga_out32(mmioaddr, (unsigned)RS16(x + w-1) | y, XYEND | EXECUTE);

	vis->accelactive = 1;

	return 0;
}
