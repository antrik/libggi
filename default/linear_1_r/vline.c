/* $Id: vline.c,v 1.5 2007/05/21 07:58:32 pekberg Exp $
******************************************************************************

   Linear 1 vertical lines (high-bit-right).

   Copyright (C) 1995 Andreas Beck	[becka@ggi-project.org]
   Copyright (C) 1998 Andrew Apted	[andrew@ggi-project.org]
   Copyright (C) 2007 Peter Rosin	[peda@lysator.liu.se]

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

#include "lin1rlib.h"


static inline void
do_drawvline(struct ggi_visual *vis, int x, int y, int height)
{
	uint8_t *adr;
	int i, sw, bm;

	PREPARE_FB(vis);

	bm = 1 << (x & 7);
	sw = LIBGGI_FB_W_STRIDE(vis);
	adr = (uint8_t *)LIBGGI_CURWRITE(vis) + (x>>3) + y * sw;

	if (LIBGGI_GC_FGCOLOR(vis) & 1) 
		for (i = height; i--; adr += sw)
			*adr |= bm;
	else
		for (i = height; i--; adr += sw)
			*adr &= ~bm;
}

int
GGI_lin1r_drawvline(struct ggi_visual *vis, int x, int y, int height)
{
	LIBGGICLIP_XYH(vis, x, y, height);

	do_drawvline(vis, x, y, height);

	return 0;
}

int
GGI_lin1r_drawvline_nc(struct ggi_visual *vis, int x, int y, int height)
{
	do_drawvline(vis, x, y, height);

	return 0;
}

int
GGI_lin1r_unpacked_putvline(struct ggi_visual *vis,
	int x, int y, int height, const void *buffer)
{
	uint8_t *adr;
	const uint8_t *buff = (const uint8_t *)buffer;
	int sw, i, bm;

	LIBGGICLIP_XYH_BUFMOD(vis, x, y, height, buff, *1);
	PREPARE_FB(vis);

	bm = 1 << (x & 7);
	sw = LIBGGI_FB_W_STRIDE(vis);
	adr = (uint8_t *)LIBGGI_CURWRITE(vis) + (x >> 3) + y * sw;

	for (i = 0; i < height; ++i, adr += sw) {
		if (*buff++ & 1) 
			*adr |= bm;
		else
			*adr &= ~bm;
	}

  	return 0;
}

int
GGI_lin1r_packed_putvline(struct ggi_visual *vis,
	int x, int y, int height, const void *buffer)
{ 
	uint8_t *adr;
	const uint8_t *buff = (const uint8_t *)buffer;
	uint8_t mask = 1;
	int sw, i, bm;

	/* Clipping */
	if (x< (LIBGGI_GC(vis)->cliptl.x) ||
	    x>=(LIBGGI_GC(vis)->clipbr.x)) return 0;
	if (y< (LIBGGI_GC(vis)->cliptl.y)) {
		int diff = LIBGGI_GC(vis)->cliptl.y - y;
		y += diff;
		height -= diff;
		buff += diff >> 3;
		mask <<= diff & 7;
	}
	if (y+height>(LIBGGI_GC(vis)->clipbr.y)) {
		height = LIBGGI_GC(vis)->clipbr.y - y;
	}
	if (height <= 0) return 0;

	PREPARE_FB(vis);

	bm = 1 << (x & 7);
	sw = LIBGGI_FB_W_STRIDE(vis);
	adr = (uint8_t *)LIBGGI_CURWRITE(vis) + (x >> 3) + y * sw;

	for (i = 0; i < height; ++i, adr += sw) {
		if (*buff & mask) 
			*adr |= bm;
		else
			*adr &= ~bm;
		mask <<= 1;
		if (!mask) {
			mask = 1;
			++buff;
		}
	}

  	return 0;
}

int
GGI_lin1r_unpacked_getvline(struct ggi_visual *vis,
	int x, int y, int height, void *buffer)
{ 
	uint8_t *adr;
	uint8_t *buff = (uint8_t *)buffer;
	int sw, i, bm;

	PREPARE_FB(vis);

	sw = LIBGGI_FB_R_STRIDE(vis);
	adr = (uint8_t *)LIBGGI_CURREAD(vis) + (x >> 3) + y * sw;

	bm = 1 << (x & 7);
	for (i = 0; i < height; ++i, adr += sw)
		*buff++ = !!(*adr & bm);

  	return 0;
}

int
GGI_lin1r_packed_getvline(struct ggi_visual *vis,
	int x, int y, int height, void *buffer)
{ 
	uint8_t *adr, *buff = (uint8_t *)buffer;
	uint8_t pix, mask, bm;
	int sw, i, j, shift = 0;

	if (x < 0 || x >= LIBGGI_VIRTX(vis))
		return 0;

	if (y < 0) {
		height -= -y;
		buff += -y >> 3;
		shift = -y & 7;
		y = 0;
	}
	if (y + height > LIBGGI_VIRTY(vis))
		height = LIBGGI_VIRTY(vis) - y;
	if (height <= 0)
		return 0;

	PREPARE_FB(vis);

	sw = LIBGGI_FB_R_STRIDE(vis);
	adr = (uint8_t *)LIBGGI_CURREAD(vis) + (x >> 3) + y * sw;

	mask = 1 << shift;
	bm = 1 << (x & 7);

	j = height - ((8 - shift) & 7);

	if (shift) {
		/* Uneven clipping above */
		if (j <= 0) {
			/* Get to one byte only */
			pix = *buff &
				((0xff >> (8 - shift)) | (0xff << (j + 8)));
			for (; height > 0; --height, mask <<= 1, adr += sw)
				pix |= (*adr & bm) ? mask : 0;
			*buff = pix;
			return 0;
		}

		pix = *buff & (0xff >> (8 - shift));
		for (; mask; mask <<= 1, adr += sw)
			pix |= (*adr & bm) ? mask : 0;
		*buff++ = pix;
		mask = 1;
	}

	for (; j > 8; j -= 8) {
		pix = 0;
		for (i = 0; i < 8; ++i, mask <<= 1, adr += sw)
			pix |= (*adr & bm) ? mask : 0;
		*buff++ = pix;
		mask = 1;
	}

	pix = *buff & (0xff << j);
	for (; j > 0; --j, mask <<= 1, adr += sw)
		pix |= (*adr & bm) ? mask : 0;
	*buff = pix;
  	return 0;
}
