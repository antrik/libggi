/* $Id: gtext.c,v 1.4 2006/03/12 23:15:05 soyt Exp $
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

#include <string.h>
#include "m2164w.h"


int GGI_m2164w_getcharsize(struct ggi_visual *vis, int *width, int *height)
{
	/* The stubs' font is 8x8, so that is what we return */
	*width = FWIDTH;
	*height = FHEIGHT;

	return 0;
}

static inline void
blitchar(struct ggi_visual *vis, int x, int y, ggi_pixel color, uint8_t *field)
{
	int xp, bp;
	int ywidth = FHEIGHT;

	/* Clipping is done via the PutPixel call ... we should pre-clip */
	while (ywidth--) {
		for (xp=0, bp=0x80; xp < FWIDTH; xp++) {
			if ((*field & bp)) {
				LIBGGIPutPixel(vis, x+xp, y, color);
			}
      			if (!(bp >>= 1)) {
				bp = 0x80;
				field++;
			}
    		}
		y++;
	}
}


static inline void
drawbox(struct ggi_visual *vis, int x, int y, int w)
{
	struct m2164w_priv *priv = M2164W_PRIV(vis);
	volatile uint8_t *mmioaddr = FBDEV_PRIV(vis)->mmioaddr;
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
	mga_out32(mmioaddr, (unsigned)(RS16(x + w) << 16) | RS16(x), 
		  FXBNDRY);
	mga_out32(mmioaddr, (unsigned)(RS16(y) << 16) | FHEIGHT,
		  YDSTLEN | EXECUTE);

	vis->accelactive = 1;
}


int GGI_m2164w_putc(struct ggi_visual *vis, int x, int y, char c)
{
	ggi_pixel fgcol = LIBGGI_GC_FGCOLOR(vis);

	LIBGGI_GC_FGCOLOR(vis) = LIBGGI_GC_BGCOLOR(vis);
	drawbox(vis, x, y, FWIDTH);
	LIBGGI_GC_FGCOLOR(vis) = fgcol;
	blitchar(vis, x, y, fgcol, M2164W_PRIV(vis)->font + (((uint8_t)c) << 3));

	return 0;
}

int GGI_m2164w_puts(struct ggi_visual *vis, int x, int y, const char *str)
{
	ggi_pixel fgcol = LIBGGI_GC_FGCOLOR(vis);
	int len, count;
	int tlx, brx;

	/* Vertically out of the clipping area ? */
	if ((y+FHEIGHT < LIBGGI_GC(vis)->cliptl.y) ||
	    (y >= LIBGGI_GC(vis)->clipbr.y)) {
		return 0;
	}
	len = strlen(str);
	
	LIBGGI_GC_FGCOLOR(vis) = LIBGGI_GC_BGCOLOR(vis);
	drawbox(vis, x, y, FWIDTH * len);
	LIBGGI_GC_FGCOLOR(vis) = fgcol;

	tlx = LIBGGI_GC(vis)->cliptl.x;
	brx = LIBGGI_GC(vis)->clipbr.x;
	    
	for (count=0; len > 0; len--, str++, x += FWIDTH) {
		/* Horizontally within the clipping area ? */
		if (x+FWIDTH >= tlx && x < brx) {
			blitchar(vis, x, y, fgcol,
				 M2164W_PRIV(vis)->font
				 + (((uint8_t)*str) << 3));
			count++;
		}
	}

	return count;
}


int GGI_m2164w_fastputc(struct ggi_visual *vis, int x, int y, char c)
{
	volatile uint8_t *mmioaddr = FBDEV_PRIV(vis)->mmioaddr;
	struct m2164w_priv *priv = M2164W_PRIV(vis);
	int yadd = vis->w_frame_num * LIBGGI_VIRTY(vis);
	uint32_t ar3, dwgctl;
	
	y += yadd;
	
	mga_gcupdate(mmioaddr, priv, LIBGGI_MODE(vis),
		     LIBGGI_GC(vis), LIBGGI_VIRTX(vis), yadd);

	ar3 = priv->fontoffset + priv->charadd * (uint8_t)c;
	dwgctl = OP_BITBLT | BOP_COPY | BLTMOD_BMONOWF | SHFTZERO |
		SGNZERO | LINEAR;

	if (priv->dwgctl != dwgctl) {
		mga_waitfifo(mmioaddr, 5);
		mga_setdwgctl(mmioaddr, priv, dwgctl);
	} else {
		mga_waitfifo(mmioaddr, 4);
	}
	mga_out32(mmioaddr, RS27(ar3), AR3);
	mga_out32(mmioaddr, RS18(ar3 + priv->charadd - 1), AR0);
	mga_out32(mmioaddr, (unsigned)(RS16(x + FWIDTH - 1) << 16) | RS16(x),
		  FXBNDRY);
	mga_out32(mmioaddr, (unsigned)(RS16(y) << 16) | FHEIGHT,
		  YDSTLEN | EXECUTE);
   
	vis->accelactive = 1;

	return 0;
}


int GGI_m2164w_fastputs(struct ggi_visual *vis, int x, int y, const char *str)
{
	volatile uint8_t *mmioaddr = FBDEV_PRIV(vis)->mmioaddr;
	struct m2164w_priv *priv = M2164W_PRIV(vis);
	int yadd = vis->w_frame_num * LIBGGI_VIRTY(vis);
	int virtx = LIBGGI_VIRTX(vis);
	uint32_t oldar3 = 0xffffffff;
	
	y += yadd;

	mga_gcupdate(mmioaddr, priv, LIBGGI_MODE(vis),
		     LIBGGI_GC(vis), LIBGGI_VIRTX(vis), yadd);

	mga_waitfifo(mmioaddr, 1);
	mga_setdwgctl(mmioaddr, priv, OP_BITBLT | BOP_COPY | BLTMOD_BMONOWF |
		      SHFTZERO | SGNZERO | LINEAR);

	while (*str && x < virtx) {
		uint32_t ar3 = priv->fontoffset + priv->charadd * (uint8_t)*str;

		if (ar3 != oldar3) {
			oldar3 = ar3;
			mga_waitfifo(mmioaddr, 4);
			mga_out32(mmioaddr, RS18(ar3 + priv->charadd - 1),
				  AR0);
		} else {
			mga_waitfifo(mmioaddr, 3);
		}
		mga_out32(mmioaddr, RS27(ar3), AR3);
		mga_out32(mmioaddr, (unsigned)(RS16(y) << 16) | FHEIGHT, 
			  YDSTLEN);
		mga_out32(mmioaddr, (unsigned)(RS16(x + FWIDTH - 1) << 16) | RS16(x),
			  FXBNDRY | EXECUTE);

		str++;
		x += FWIDTH;
	}

	vis->accelactive = 1;

	return 0;
}
