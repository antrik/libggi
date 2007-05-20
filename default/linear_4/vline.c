/* $Id: vline.c,v 1.13 2007/05/20 10:55:55 pekberg Exp $
******************************************************************************

   Graphics library for GGI.

   Copyright (C) 1997 Jason McMullan   [jmcc@ggi-project.org]
   Copyright (C) 1995 Andreas Beck     [becka@ggi-project.org]

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

#include "lin4lib.h"

/********************************/
/* draw/get/put a vertical line */
/********************************/

static inline void
do_drawvline(struct ggi_visual *vis, int x, int y, int h)
{
	uint8_t *ptr;
	int stride = LIBGGI_FB_W_STRIDE(vis);
	uint8_t shift = (x & 0x01) << 2;
	uint8_t color = (LIBGGI_GC_FGCOLOR(vis) & 0x0f) << (shift^4);
	uint8_t mask = 0x0f << shift;

	PREPARE_FB(vis);

	ptr = (uint8_t*)LIBGGI_CURWRITE(vis) + y*stride + x/2;

	for(; h > 0; h--, ptr += stride) {
		*ptr = color | (*ptr & mask);
	}
	
}

int GGI_lin4_drawvline(struct ggi_visual *vis, int x, int y, int h)
{
	LIBGGICLIP_XYH(vis, x, y, h);
	
	do_drawvline(vis, x, y, h);

	return 0;
}

int GGI_lin4_drawvline_nc(struct ggi_visual *vis,int x,int y,int h)
{
	do_drawvline(vis, x, y, h);
	
	return 0;
}

int GGI_lin4_packed_putvline(struct ggi_visual *vis,int x,int y,int h,const void *buffer)
{
	uint8_t *ptr;
	const uint8_t *buf8=(const uint8_t *)buffer;
	int stride=LIBGGI_FB_W_STRIDE(vis);
	uint8_t shift = (x & 0x01) << 2;
	uint8_t mask = 0x0f << shift;
	uint8_t antishift = shift ^ 4;
	int diff = 0;

	if (x < LIBGGI_GC(vis)->cliptl.x || x >= LIBGGI_GC(vis)->clipbr.x)
		return 0;
	if (y < LIBGGI_GC(vis)->cliptl.y) {
		diff = LIBGGI_GC(vis)->cliptl.y - y;
		y += diff;
		buf8 += diff >> 1;
		h -= diff;
	}
	if (y + h > LIBGGI_GC(vis)->clipbr.y)
		h = LIBGGI_GC(vis)->clipbr.y - y;
	if (h <= 0)
		return 0;

	PREPARE_FB(vis);

	ptr=(uint8_t *)LIBGGI_CURWRITE(vis)+y*stride+(x>>1);

	if (diff & 1) {
		*ptr = ((*buf8++ & 0x0f) << antishift) | (*ptr & mask);
		ptr += stride;
		--h;
	}

	for(; h > 1; h-=2, ptr+=(stride<<1)) {
		*ptr=((*buf8 & 0xf0) >> shift) | (*ptr & mask);
		*(ptr+stride) = ((*buf8++ & 0x0f) << antishift)
			| (*(ptr+stride) & mask);
	}
	
	if (h) {
		*ptr=((*buf8 & 0xf0) >> shift) | (*ptr & mask);
	}
		
	return 0;
}

int GGI_lin4_packed_getvline(struct ggi_visual *vis,int x,int y,int h,void *buffer)
{
	uint8_t *ptr,*buf8=(uint8_t *)buffer;
	int stride=LIBGGI_FB_W_STRIDE(vis);
	uint8_t shift = (x & 0x01) << 2;
	uint8_t mask = 0xf0 >> shift;
	uint8_t antishift = shift ^ 4;
	int diff = 0;

	if (x < 0 || x >= LIBGGI_VIRTX(vis))
		return 0;

	if (y < 0) {
		diff = -y;
		y = 0;
		buf8 += diff >> 1;
		h -= diff;
	}
	if (y + h > LIBGGI_VIRTY(vis))
		h = LIBGGI_VIRTY(vis) - y;
	if (h <= 0)
		return 0;

	PREPARE_FB(vis);
	ptr = (uint8_t *)LIBGGI_CURREAD(vis)+y*stride+(x>>1);

	if (diff & 1) {
		uint8_t pix = *buf8 & 0xf0;
		*buf8++ = pix | ((*ptr & mask) >> antishift);
		ptr += stride;
		--h;
	}

	/* Warning: unnecessary bit operations ahead! */
	for (; h > 1; h-=2, ptr+=stride<<1) {
		*buf8++ = ((*ptr & mask) << shift)
			| ((*(ptr+stride) & mask) >> antishift);
	}
	
	/* Here we can't lazily stick the extra pixel into the buffer, since
	 * it might be off the screen. */
	if (h) {
		*buf8 = (*buf8 & 0x0f) | ((*ptr & mask) << shift);
	}
	
	return 0;
}
