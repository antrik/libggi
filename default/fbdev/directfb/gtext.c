/* $Id: gtext.c,v 1.2 2005/07/30 11:39:57 cegger Exp $
******************************************************************************

   LibGGI - DirectFB driver acceleration for the fbdev target.

   Copyright (C) 2001 Brian S. Julin	[bri@calyx.com]

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
#include "ggidirectfb.h"


int GGI_directfb_getcharsize(ggi_visual *vis, int *width, int *height)
{
	/* The stubs' font is 8x8, so that is what we return */
	*width = FWIDTH;
	*height = FHEIGHT;

	return 0;
}

static inline void
blitchar(ggi_visual *vis, int x, int y, ggi_pixel color, uint8_t *field)
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
drawbox(ggi_visual *vis, int x, int y, int w)
{
	struct directfb_priv *priv = DIRECTFB_PRIV(vis);
	int yadd = vis->w_frame_num * LIBGGI_VIRTY(vis);
#if 0
	y += yadd;
        directfb_gcupdate(vis, priv, LIBGGI_MODE(vis), LIBGGI_GC(vis),
                          LIBGGI_VIRTX(vis), yadd);
#endif

	vis->accelactive = 1;
}


int GGI_directfb_putc(ggi_visual *vis, int x, int y, char c)
{
	ggi_pixel fgcol = LIBGGI_GC_FGCOLOR(vis);

#if 0
	LIBGGI_GC_FGCOLOR(vis) = LIBGGI_GC_BGCOLOR(vis);
	drawbox(vis, x, y, FWIDTH);
	LIBGGI_GC_FGCOLOR(vis) = fgcol;
	blitchar(vis, x, y, fgcol, DIRECTFB_PRIV(vis)->font + (((uint8_t)c) << 3));
#endif

	return 0;
}

int GGI_directfb_puts(ggi_visual *vis, int x, int y, const char *str)
{
#if 0
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
				 DIRECTFB_PRIV(vis)->font
				 + (((uint8_t)*str) << 3));
			count++;
		}
	}

	return count;
#endif
	return 0;
}


int GGI_directfb_fastputc(ggi_visual *vis, int x, int y, char c)
{
#if 0
	volatile uint8_t *mmioaddr = FBDEV_PRIV(vis)->mmioaddr;
	struct directfb_priv *priv = DIRECTFB_PRIV(vis);
	int yadd = vis->w_frame_num * LIBGGI_VIRTY(vis);
	uint32_t ar3, dwgctl;
	
	y += yadd;

        directfb_gcupdate(vis, priv, LIBGGI_MODE(vis), LIBGGI_GC(vis),
                          LIBGGI_VIRTX(vis), yadd);


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
	mga_out32(mmioaddr, (RS16(x + FWIDTH - 1) << 16) | RS16(x), FXBNDRY);
	mga_out32(mmioaddr, (RS16(y) << 16) | FHEIGHT, YDSTLEN | EXECUTE);
   
	vis->accelactive = 1;
#endif
	return 0;
}


int GGI_directfb_fastputs(ggi_visual *vis, int x, int y, const char *str)
{

	struct directfb_priv *priv = DIRECTFB_PRIV(vis);
	int yadd = vis->w_frame_num * LIBGGI_VIRTY(vis);
	int virtx = LIBGGI_VIRTX(vis);

	/* Later */

#if 0
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
		mga_out32(mmioaddr, (RS16(y) << 16) | FHEIGHT, YDSTLEN);
		mga_out32(mmioaddr, (RS16(x + FWIDTH - 1) << 16) | RS16(x),
			  FXBNDRY | EXECUTE);

		str++;
		x += FWIDTH;
	}

	vis->accelactive = 1;

#endif

	return 0;
}




