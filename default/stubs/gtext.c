/* $Id: gtext.c,v 1.3 2005/07/30 11:40:03 cegger Exp $
******************************************************************************

   Graphics library for GGI.

   Copyright (C) 1995 Andreas Beck     [becka@ggi-project.org]
   Copyright (C) 1997 Jason McMullan   [jmcc@ggi-project.org]
   Copyright (C) 1998 MenTaLguY        [mentalg@geocities.com]
   Copyright (C) 1998 Andrew Apted     [andrew.apted@ggi-project.org]

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

#include "stublib.h"

#include <ggi/internal/font/8x8>

int GGI_stubs_getcharsize(ggi_visual *vis, int *width, int *height)
{
	/* The stubs' font is 8x8, so that is what we return */

	*width = *height = 8;
	return 0;
}

static inline int 
GGIblit2c(ggi_visual *vis, int x, int y, int xwidth, int ywidth, void *field)
{
	int xp,bp;
	ggi_pixel color;

	/* Clipping is done via the PutPixel call ... we should pre-clip */

	for(;ywidth--;y++) {
		for(xp=0,bp=0x80;xp<xwidth;xp++) {
			color=(*(char *)field & bp) ? LIBGGI_GC_FGCOLOR(vis) 
						    : LIBGGI_GC_BGCOLOR(vis);
			ggiPutPixel(vis,x+xp,y,color);
      			if (!(bp>>=1)) {
				bp=0x80;
				field = ((uint8_t *) field) + 1;
			}
    		}
	}

	return 0;
}

int GGI_stubs_putc(ggi_visual *vis,int x,int y,char c)
{
	return GGIblit2c(vis, x, y, 8, 8, font+((uint8_t)c<<3));
}

/* Write a null-terminated string of characters, returns the number of
 * characters at least partially displayed 
 */

int GGI_stubs_puts(ggi_visual *vis, int x, int y, const char *str)
{
	int count;
	int char_w, char_h;

	ggiGetCharSize(vis, &char_w, &char_h);
	
	/* vertically out of the clipping area ? */

	if ((y+char_h < LIBGGI_GC(vis)->cliptl.y) ||
	    (y >= LIBGGI_GC(vis)->clipbr.y)) {

		return 0;
	}
	    
	for (count=0; *str && (x < LIBGGI_VIRTX(vis));
	     str++, x += char_w) {

		/* horizontally within the clipping area ? */

		if ((x+char_w >= LIBGGI_GC(vis)->cliptl.x) &&
		    (x < LIBGGI_GC(vis)->clipbr.x)) {

			ggiPutc(vis, x, y, *str); 
			count++;
		}
	}

	return count;
}
