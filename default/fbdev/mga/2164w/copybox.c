/* $Id: copybox.c,v 1.4 2005/07/31 09:58:44 cegger Exp $
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


int GGI_m2164w_copybox(ggi_visual *vis, int x, int y, int w, int h,
		       int dstx, int dsty)
{
	struct m2164w_priv *priv = M2164W_PRIV(vis);
	volatile uint8_t *mmioaddr = FBDEV_PRIV(vis)->mmioaddr;
	int virtx = LIBGGI_VIRTX(vis);
	int yadd = vis->w_frame_num * LIBGGI_VIRTY(vis);
	int32_t ar5 = virtx;
	int32_t begin, end;
	uint32_t sgn = 0;
	uint32_t dwgctl;

#define COPY_LEFT	1
#define COPY_UP		4

	dsty += yadd;
	y += vis->r_frame_num*LIBGGI_VIRTY(vis);

	if (dsty > y) {
		sgn |= COPY_UP;
		y += h - 1;
		dsty += h - 1;
		ar5 = - ar5;
	}
	
	begin = end = y * virtx + x;

	w--;
	if (dstx > x) {
		sgn |= COPY_LEFT;
		begin += w;
	} else {
		end += w;
	}

	dwgctl = BLTMOD_BFCOL | BOP_COPY | SHFTZERO | OP_BITBLT
		| (sgn ? 0 : SGNZERO);

	mga_gcupdate(mmioaddr, priv, LIBGGI_MODE(vis),
		     LIBGGI_GC(vis), virtx, yadd);

	if (priv->dwgctl != dwgctl) {
		if (sgn) mga_waitfifo(mmioaddr, 7);
		else mga_waitfifo(mmioaddr, 6);
		mga_setdwgctl(mmioaddr, priv, dwgctl);
	} else {
		if (sgn) mga_waitfifo(mmioaddr, 6);
		else mga_waitfifo(mmioaddr, 5);
	}
	if (sgn) {
		mga_out32(mmioaddr, sgn, SGN);
	}
	mga_out32(mmioaddr, RS18(end), AR0);
	mga_out32(mmioaddr, RS24(begin), AR3);
	mga_out32(mmioaddr, RS18(ar5), AR5);
	mga_out32(mmioaddr, (unsigned)(RS16(dstx + w) << 16) | RS16(dstx), 
		  FXBNDRY);
	mga_out32(mmioaddr, (unsigned)(RS16(dsty) << 16) | RS16(h), 
		  YDSTLEN | EXECUTE);

	vis->accelactive = 1;

	return 0;
}
