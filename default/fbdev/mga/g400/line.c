/* $Id: line.c,v 1.4 2006/03/12 23:15:05 soyt Exp $
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


int GGI_mga_g400_drawline(struct ggi_visual *vis, int x, int y, int x2, int y2)
{
	struct mga_g400_priv *priv = MGA_G400_PRIV(vis);
	volatile uint8_t *mmioaddr = FBDEV_PRIV(vis)->mmioaddr;
	int yadd = vis->w_frame_num * LIBGGI_VIRTY(vis);
	uint32_t dwgctl;

	if (yadd) {
		y += yadd;
		y2 += yadd;
	}

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
	mga_out32(mmioaddr, (unsigned)RS16(x) | (RS16(y) << 16), XYSTRT);
	mga_out32(mmioaddr, (unsigned)RS16(x2) | (RS16(y2) << 16),
		  XYEND | EXECUTE);

	vis->accelactive = 1;

	return 0;
}
