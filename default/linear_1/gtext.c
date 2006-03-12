/* $Id: gtext.c,v 1.4 2006/03/12 23:15:06 soyt Exp $
******************************************************************************

   Linear 1 character drawing.

   Copyright (C) 1995 Andreas Beck    [becka@ggi-project.org]
   Copyright (C) 1997 Jason McMullan   [jmcc@ggi-project.org]
   Copyright (C) 1998 Andrew Apted   [andrew@ggi-project.org]

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

#include "lin1lib.h"
#include <ggi/internal/font/8x8>


int GGI_lin1_putc(struct ggi_visual *vis, int x, int y, char c)
{
	int h=8, stride, rev;

	uint8_t *src, *dest;
	uint8_t mask, mask0, mask1;
	int shift0, shift1;

	if ((x   >= LIBGGI_GC(vis)->clipbr.x)  || 
	    (y   >= LIBGGI_GC(vis)->clipbr.y)  ||
	    (x+8 <= LIBGGI_GC(vis)->cliptl.x)  || 
	    (y+8 <= LIBGGI_GC(vis)->cliptl.y)) 
		return 0;

	if ((LIBGGI_GC_FGCOLOR(vis)&1) == (LIBGGI_GC_BGCOLOR(vis)&1)) {
		/* Just a solid box when fg == bg */
		return ggiDrawBox(vis, x, y, 8, 8);
	}

	src = (uint8_t *)(font) + ((int) (uint8_t) c << 3);
	rev = (LIBGGI_GC_BGCOLOR(vis) & 1);

	if (y < LIBGGI_GC(vis)->cliptl.y) {
		int diff = LIBGGI_GC(vis)->cliptl.y - y;
		h   -= diff;
		y   += diff;
		src += diff;

	} 
	if (y+h > LIBGGI_GC(vis)->clipbr.y) {
		h = LIBGGI_GC(vis)->clipbr.y - y;
	}

	PREPARE_FB(vis);

	stride = LIBGGI_FB_W_STRIDE(vis);
	dest = (uint8_t *) LIBGGI_CURWRITE(vis) + y*stride + (x>>3);

	if ((x & 7) == 0) {

		/* aligned putc */

		mask = 0xff;

		if (x < LIBGGI_GC(vis)->cliptl.x) {
			mask &= 0xff >> (LIBGGI_GC(vis)->cliptl.x - x);
		}
		if (x+8 > LIBGGI_GC(vis)->clipbr.x) {
			mask &= 0xff << (x+8 - LIBGGI_GC(vis)->clipbr.x);
		}
		
		if ((mask == 0xff) && !rev) {

			for(; h > 0; h--, dest += stride, src++) {
				*dest = *src;
			}

		} else if ((mask == 0xff) && rev) {

			for(; h > 0; h--, dest += stride, src++) {
				*dest = ~*src;
			}

		} else if (!rev) {

			for(; h > 0; h--, dest += stride, src++) {
				*dest = (*src & mask) | (*dest & ~mask);
			}
		} else {
			for(; h > 0; h--, dest += stride, src++) {
				*dest = (~*src & mask) | (*dest & ~mask);
			}
		}

		return 0;
	}

	/* non-aligned putc */

	mask = 0xff;

	if (x < LIBGGI_GC(vis)->cliptl.x) {
		mask &= 0xff >> (LIBGGI_GC(vis)->cliptl.x - x);
	}
	if (x+8 > LIBGGI_GC(vis)->clipbr.x) {
		mask &= 0xff << (x+8 - LIBGGI_GC(vis)->clipbr.x);
	}
	
	shift0 = x & 7; 	   /* shift to the right */
	shift1 = (7 - (x & 7));    /* shift to the left */

	mask0 = mask >> shift0;
	mask1 = mask << shift1;
	
	if (!rev) {
		for(; h > 0; h--, dest += stride, src++) {
			dest[0]=((*src>>shift0)&mask0) | (dest[0]&~mask0);
			dest[1]=((*src<<shift1)&mask1) | (dest[1]&~mask1);
		}
	} else {
		for(; h > 0; h--, dest += stride, src++) {
			dest[0]=((~*src>>shift0)&mask0) | (dest[0]&~mask0);
			dest[1]=((~*src<<shift1)&mask1) | (dest[1]&~mask1);
		}
	}

	return 0;
}
