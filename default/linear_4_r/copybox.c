/* $Id: copybox.c,v 1.5 2008/09/03 10:18:13 pekberg Exp $
******************************************************************************

   LibGGI linear 4 - copybox

   Copyright (C) 1997 Jason McMullan   [jmcc@ggi-project.org]

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

#include "lin4rlib.h"

/* Maximum amount of bytes to allocate on the stack */
#define MAX_STACKBYTES	4096

static inline void
do_copy(struct ggi_visual *vis, int x, int y, int w, int h,
	int nx, int ny, void *buf)
{
	int step = 1;

	if (ny > y) {
		step = -1;
		y += h - 1;
		ny += h - 1;
	}

	for (; h > 0; --h, y += step, ny += step) {
		_ggiGetHLine(vis, x,  y,  w, buf);
		_ggiPutHLine(vis, nx, ny, w, buf);
	}
}


static inline int
fallback(struct ggi_visual *vis, int x, int y, int w, int h, int nx, int ny)
{
	size_t size;

	if (LIBGGI_GT(vis) & GT_SUB_PACKED_GETPUT)
		size = GT_ByPPP(w, LIBGGI_GT(vis));
	else
		size = w * GT_ByPP(LIBGGI_GT(vis));

	if (size <= MAX_STACKBYTES) {
		uint8_t buf[MAX_STACKBYTES];

		do_copy(vis, x, y, w, h, nx, ny, buf);
	}
	else {
		uint8_t *buf = malloc(size);
		if (!buf)
			return GGI_ENOMEM;
		do_copy(vis, x, y, w, h, nx, ny, buf);
		free(buf);
	}

	return 0;
}

int
GGI_lin4r_copybox(struct ggi_visual *vis, int x, int y, int w, int h, int nx, int ny)
{
	uint8_t *src, *dst;
	int linew = LIBGGI_FB_W_STRIDE(vis);
	int line;
	int first, last;
	int f, l, fm, lm;

	LIBGGICLIP_COPYBOX(vis, x, y, w, h, nx, ny);
	if ((x ^ nx) & 1)
		return fallback(vis, x, y, w, h, nx, ny);
	PREPARE_FB(vis);

	/* if x is odd, left pixel dangles. */
	/* if x is odd and w is even, right dangles. */
	/* if x is even and w is odd, right dangles. */

	first = x & 0x01;
	last  = (x ^ w) & 0x01;
	w -= first + last;

	src = (uint8_t *)LIBGGI_CURWRITE(vis) + y * linew  + (x / 2);
	dst = (uint8_t *)LIBGGI_CURWRITE(vis) + ny * linew + (nx / 2);
	if (first) {
		++dst;
		++src;
	}
	if (ny > y) {
		/* go from bottom up */
		src += (h - 1) * linew;
		dst += (h - 1) * linew;
		linew = -linew;
	}
	if (nx > x) {
		/* go from right to left (swap first and last) */
		l = first;
		first = last;
		last = l;
		f = w / 2;
		fm = 0xf0;
		l = -1;
		lm = 0x0f;
	}
	else {
		f = -1;
		fm = 0x0f;
		l = w / 2;
		lm = 0xf0;
	}
	for (line=0; line < h; line++, src += linew, dst += linew) {
		if (first)
			*(dst + f) = (*(dst + f) & fm) | (*(src + f) & lm);
		memmove(dst, src, (size_t)(w / 2));
		if (last)
			*(dst + l) = (*(dst + l) & lm) | (*(src + l) & fm);
	}

	return 0;
}
