/* $Id: vline.c,v 1.4 2007/05/18 20:46:37 pekberg Exp $
******************************************************************************

   Graphics library for GGI.

   Copyright (C) 1995 Andreas Beck	[becka@ggi-project.org]
   Copyright (C) 1997 Jason McMullan	[jmcc@ggi-project.org]
   Copyright (C) 1999 Marcus Sundberg	[marcus@ggi-project.org]
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

#include "lin2rlib.h"

/********************************/
/* draw a vertical line */
/********************************/

static inline void
do_drawvline(struct ggi_visual *vis, int x, int y, int h)
{
	uint8_t *ptr;
	int stride = LIBGGI_FB_W_STRIDE(vis);
	uint8_t shift = (x & 3) << 1;
	uint8_t color = (LIBGGI_GC_FGCOLOR(vis) & 0x03) << shift;
	uint8_t mask = ~(0x03 << shift);

	PREPARE_FB(vis);

	ptr = (uint8_t*)LIBGGI_CURWRITE(vis) + y*stride + x/4;

	for(; h > 0; --h, ptr += stride)
		*ptr = (*ptr & mask) | color;
}

int
GGI_lin2r_drawvline(struct ggi_visual *vis, int x, int y, int h)
{
	LIBGGICLIP_XYH(vis, x, y, h);
	
	do_drawvline(vis, x, y, h);

	return 0;
}

int
GGI_lin2r_drawvline_nc(struct ggi_visual *vis,int x,int y,int h)
{
	do_drawvline(vis, x, y, h);
	
	return 0;
}

int
GGI_lin2r_packed_putvline(struct ggi_visual *vis,
	int x, int y, int height, const void *buffer)
{ 
	uint8_t *adr;
	const uint8_t *buff = (const uint8_t *)buffer;
	int shift = 0;
	int sw, i, bm, xshift;
	uint8_t tmp;

	/* Clipping */
	if (x< (LIBGGI_GC(vis)->cliptl.x) ||
	    x>=(LIBGGI_GC(vis)->clipbr.x)) return 0;
	if (y< (LIBGGI_GC(vis)->cliptl.y)) {
		int diff = LIBGGI_GC(vis)->cliptl.y - y;
		y += diff;
		height -= diff;
		buff += diff >> 2;
		shift = (diff & 3) << 1;
	}
	if (y+height>(LIBGGI_GC(vis)->clipbr.y)) {
		height = LIBGGI_GC(vis)->clipbr.y - y;
	}
	if (height <= 0) return 0;

	PREPARE_FB(vis);

	xshift = (x & 3) << 1;
	bm = 3 << xshift;
	sw = LIBGGI_FB_W_STRIDE(vis);
	adr = (uint8_t *)LIBGGI_CURWRITE(vis) + (x >> 2) + y * sw;

	for (i = 0; i < height; ++i, adr += sw) {
		tmp = *adr & ~bm;
		if (xshift > shift)
			*adr = tmp | ((*buff << (xshift - shift)) & bm);
		else
			*adr = tmp | ((*buff >> (shift - xshift)) & bm);
		shift += 2;
		if (shift & 8) {
			shift = 0;
			++buff;
		}
	}

  	return 0;
}

int
GGI_lin2r_packed_getvline(struct ggi_visual *vis,
	int x, int y, int height, void *buffer)
{ 
	uint8_t *adr;
	uint8_t *buff = (uint8_t *)buffer;
	int shift = 0;
	int sw, i, j, bm, xshift;
	uint8_t pix;

	if (x < 0 || x >= LIBGGI_VIRTX(vis))
		return 0;

	if (y < 0) {
		height -= -y;
		buff += -y >> 2;
		shift = (-y & 3) << 1;
		y = 0;
	}
	if (y + height > LIBGGI_VIRTY(vis))
		height = LIBGGI_VIRTY(vis) - y;
	if (height <= 0)
		return 0;

	PREPARE_FB(vis);

	sw = LIBGGI_FB_R_STRIDE(vis);
	adr = (uint8_t *)LIBGGI_CURREAD(vis) + (x >> 2) + y * sw;

	xshift = (x & 3) << 1;
	bm = 3 << xshift;

	j = (height << 1) - ((8 - shift) & 7);

	if (shift) {
		/* Uneven clipping above */
		if (j <= 0) {
			/* Get to one byte only */
			pix = *buff &
				((0xff >> (8 - shift)) | (0xff << (j + 8)));
			for (; height > 0; --height, shift += 2, adr += sw)
				pix |= SSHIFT(*adr & bm, shift - xshift);
			*buff = pix;
			return 0;
		}

		pix = *buff & (0xff >> (8 - shift));
		for (; shift < 8; shift += 2, adr += sw)
			pix |= SSHIFT(*adr & bm, shift - xshift);
		*buff++ = pix;
		shift = 0;
	}

	for (; j > 8; j -= 8) {
		pix = 0;
		for (i = 0; i < 4; ++i, shift += 2, adr += sw)
			pix |= SSHIFT(*adr & bm, shift - xshift);
		*buff++ = pix;
		shift = 0;
	}

	pix = *buff & (0xff << j);
	for (; j > 0; j -= 2, shift += 2, adr += sw)
		pix |= SSHIFT(*adr & bm, shift - xshift);
	*buff = pix;
  	return 0;
}
