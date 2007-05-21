/* $Id: box.c,v 1.12 2007/05/21 09:37:12 pekberg Exp $
******************************************************************************

   Generic box drawing

   Copyright (C) 1995 Andreas Beck     [becka@ggi-project.org]
   Copyright (C) 1997 Jason McMullan   [jmcc@ggi-project.org]
   Copyright (C) 1998 Andrew Apted     [andrew@ggi-project.org]

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

int GGI_stubs_drawbox(struct ggi_visual *vis, int x, int y, int w, int h)
{
	/* Do correct clipping so we can use faster _ggiDrawHLineNC */
  
	if (y < LIBGGI_GC(vis)->cliptl.y) {
		int diff = LIBGGI_GC(vis)->cliptl.y - y;
		y += diff;
		h -= diff;
	}
	if (y+h > LIBGGI_GC(vis)->clipbr.y) {
		h = LIBGGI_GC(vis)->clipbr.y - y;
	}
	if (x < LIBGGI_GC(vis)->cliptl.x) {
		int diff = LIBGGI_GC(vis)->cliptl.x - x;
		x += diff;
		w -= diff;
	}
	if (x+w > LIBGGI_GC(vis)->clipbr.x) {
		w = LIBGGI_GC(vis)->clipbr.x - x;
	}

	if ((h <= 0) || (w <= 0)) 
		return 0;

	for (; h > 0; h--, y++) {
		_ggiDrawHLineNC(vis, x, y, w);
	}

	return 0;
}

int GGI_stubs_putbox(struct ggi_visual *vis, int x, int y, int w, int h, const void *buffer)
{ 
	const uint8_t *src = (const uint8_t *) buffer;
	int rowadd;

	if (LIBGGI_GT(vis) & GT_SUB_PACKED_GETPUT) {
		rowadd = GT_ByPPP(w, LIBGGI_GT(vis));
	} else {
		rowadd = w * GT_ByPP(LIBGGI_GT(vis));
	}

	/* Pre-clipping, so we don't call PutHline without need */
	if (y < LIBGGI_GC(vis)->cliptl.y) {
		int diff = LIBGGI_GC(vis)->cliptl.y - y;
		y += diff;
		h -= diff;
		src += diff*rowadd;
	}

	if (y+h > LIBGGI_GC(vis)->clipbr.y) {
		h = LIBGGI_GC(vis)->clipbr.y - y;
	}

	if ((h < 0) ||
	    (x >= LIBGGI_GC(vis)->clipbr.x) ||
	    (x+w <= LIBGGI_GC(vis)->cliptl.x)) {
		return 0;
	}
	
	for (; h > 0; h--, y++, src += rowadd) {
		_ggiPutHLine(vis, x, y, w, src);
	}

	return 0;
}

int GGI_stubs_getbox(struct ggi_visual *vis, int x, int y, int w, int h, void *buffer)
{ 
	uint8_t *dest = (uint8_t *) buffer;
	int rowadd;

	/* Unclipped */


	if (LIBGGI_GT(vis) & GT_SUB_PACKED_GETPUT) {
		rowadd = GT_ByPPP(w, LIBGGI_GT(vis));
	} else {
		rowadd = w * GT_ByPP(LIBGGI_GT(vis));
	}

	if (y < 0) {
		dest += -y * rowadd;
		h += y;
		y = 0;
	}
	if (y + h > LIBGGI_VIRTY(vis))
		h = LIBGGI_VIRTY(vis) - y;

	for (; h > 0; h--, y++, dest += rowadd) {
		_ggiGetHLine(vis, x, y, w, dest);
	}

	return 0;
}
