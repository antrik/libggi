/* $Id: gtext.c,v 1.1 2001/05/12 23:02:07 cegger Exp $
******************************************************************************

   LibGGI GLIDE target - Text functions

   Copyright (C) 1998 Marcus Sundberg	[marcus@ggi-project.org]

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

#include <ggi/display/glide.h>

#include <ggi/internal/font/8x8>


#define BITMASK		0x80
#define BUFSIZE		8*8*sizeof(ggi_pixel)

int
GGI_glide16_putc(ggi_visual *vis, int x, int y, char ch)
{
	int tc, lc, rc, bc;
	int xc, yc;
	int h = 8;
	int w = 8;
	uint8 buf[BUFSIZE];
	uint16 *data = (uint16 *) buf;
	uint8 *fontptr = font+((uint8)ch<<3);

	/* Number of clipped lines for each of top, left, bottom, right. */
	tc = LIBGGI_GC(vis)->cliptl.y - y;
	lc = LIBGGI_GC(vis)->cliptl.x - x;
	bc = y + h - LIBGGI_GC(vis)->clipbr.y;
	rc = x + w - LIBGGI_GC(vis)->clipbr.x;

	if (tc >= h || lc >= w || bc >= h || rc >= w)
		return 0;

	if (tc > 0) {
		fontptr += tc;
		y += tc;
		h -= tc;
	}

	if (lc > 0) {
		w -= lc;
		x += lc;
	} else {
		lc = 0;
	}

	if (bc > 0) {
		h -= bc;
	}

	if (rc > 0) {
		w -= rc;
	} else {
		rc = 0;
	}

	for (yc = 0; yc < h; yc++) {
		uint8 fontrow = *(fontptr++);
		fontrow <<= lc;
		for (xc = 0; xc < w; xc++) {
			*(data++) = (fontrow & BITMASK) ? LIBGGI_GC_FGCOLOR(vis)
				: LIBGGI_GC_BGCOLOR(vis);
			fontrow <<= 1;
		}
		data += lc + rc;
	}

	grLfbWriteRegion(GLIDE_PRIV(vis)->writebuf, x, y,
			 GLIDE_PRIV(vis)->src_format, w, h, 16, (uint16 *)buf);
	
	return 0;
}

int
GGI_glide32_putc(ggi_visual *vis, int x, int y, char ch)
{
	int tc, lc, rc, bc;
	int xc, yc;
	int h = 8;
	int w = 8;
	uint8 buf[BUFSIZE];
	uint32 *data = (uint32 *) buf;
	uint8 *fontptr = font+((uint8)ch<<3);

	/* Number of clipped lines for each of top, left, bottom, right. */
	tc = LIBGGI_GC(vis)->cliptl.y - y;
	lc = LIBGGI_GC(vis)->cliptl.x - x;
	bc = y + h - LIBGGI_GC(vis)->clipbr.y;
	rc = x + w - LIBGGI_GC(vis)->clipbr.x;

	if (tc >= h || lc >= w || bc >= h || rc >= w)
		return 0;

	if (tc > 0) {
		fontptr += tc;
		y += tc;
		h -= tc;
	}

	if (lc > 0) {
		w -= lc;
		x += lc;
	} else {
		lc = 0;
	}

	if (bc > 0) {
		h -= bc;
	}

	if (rc > 0) {
		w -= rc;
	} else {
		rc = 0;
	}

	for (yc = 0; yc < h; yc++) {
		uint8 fontrow = *(fontptr++);
		fontrow <<= lc;
		for (xc = 0; xc < w; xc++) {
			*(data++) = (fontrow & BITMASK) ? LIBGGI_GC_FGCOLOR(vis)
				: LIBGGI_GC_BGCOLOR(vis);
			fontrow <<= 1;
		}
		data += lc + rc;
	}

	grLfbWriteRegion(GLIDE_PRIV(vis)->writebuf, x, y,
			 GLIDE_PRIV(vis)->src_format, w, h, 32, (uint32 *)buf);
	
	return 0;
}

