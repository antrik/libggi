/* $Id: hline.c,v 1.1 2007/01/23 01:01:38 pekberg Exp $
******************************************************************************

   Linear 1 horizontal lines (high-bit-right).

   Copyright (C) 1995 Andreas Beck   [becka@ggi-project.org]
   Copyright (C) 1998 Andrew Apted   [andrew@ggi-project.org]
   Copyright (C) 2007 Peter Rosin    [peda@lysator.liu.se]

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
#include "lin1rlib.h"


static inline void
do_drawhline(struct ggi_visual *vis, int x, int y, int w)
{
	uint8_t *adr;
	int i, j, mask, color;

	PREPARE_FB(vis);

	color = (LIBGGI_GC_FGCOLOR(vis) & 1) * 0xFF;

	adr = (uint8_t *)LIBGGI_CURWRITE(vis) +
		x / 8 + y * LIBGGI_FB_W_STRIDE(vis);

	/* Draw 'front' pixels */ 
	j = w;
	i = x & 7;
	if (i) {
		j += i - 8;
		if (j <= 0) {
			mask = (0xff << i) & (0xff >> -j);
			*adr = (*adr & ~mask) | (color & mask);
			return;
		}

		mask = 0xff << i;
		*adr = (*adr & ~mask) | (color & mask);
		++adr;
	}
	
	while ((j -= 8) >= 0)
		*adr++ = color;
    
	/* Draw `back` pixels if necessary */
	mask = ~(0xff << (j & 7));
	*adr = (*adr & ~mask) | (color & mask);
}

int GGI_lin1r_drawhline(struct ggi_visual *vis, int x, int y, int w)
{
	LIBGGICLIP_XYW(vis, x, y, w);

	do_drawhline(vis, x, y, w);
		
	return 0;
}

int GGI_lin1r_drawhline_nc(struct ggi_visual *vis, int x, int y, int w)
{
	do_drawhline(vis, x, y, w);
		
	return 0;
}

int
GGI_lin1r_unpacked_puthline(struct ggi_visual *vis,
	int x, int y, int w, const void *buffer)
{
	uint8_t *adr;
	const uint8_t *buff = (const uint8_t *)buffer;
	int i;
	uint8_t bm;

	LIBGGICLIP_XYW_BUFMOD(vis, x, y, w, buff, >> 3);
	PREPARE_FB(vis);

	adr = (uint8_t *)LIBGGI_CURWRITE(vis) +
		x / 8 + y * LIBGGI_FB_W_STRIDE(vis);

	bm = 1 << (x & 7);
	for (i = 0; i < w; ++i, bm <<= 1) {
		if (!bm) {
			bm = 1;
			++adr;
		}
		if (*buff++ & 1)
			*adr |= bm;
		else
			*adr &= ~bm;
	}

	return 0;
}

int
GGI_lin1r_packed_puthline(struct ggi_visual *vis,
	int x, int y, int w, const void *buffer)
{ 
	uint8_t *adr;
	const uint8_t *buff = (const uint8_t *)buffer;
	uint8_t mask;
	int i, j, diff = 0;
	uint8_t foo;
	uint16_t color;

	/* Clipping */
	if (y<(LIBGGI_GC(vis)->cliptl.y) || y>=(LIBGGI_GC(vis)->clipbr.y)) return 0;
	if (x< (LIBGGI_GC(vis)->cliptl.x)) {
		diff = LIBGGI_GC(vis)->cliptl.x - x;
		x += diff;
		w -= diff;
		buff += (diff >> 3);
		diff &= 7;
	}
	if (x+w>(LIBGGI_GC(vis)->clipbr.x)) {
		w=(LIBGGI_GC(vis)->clipbr.x)-x;
	}
	if (w<=0) return 0;

	PREPARE_FB(vis);

	adr = (uint8_t *)LIBGGI_CURWRITE(vis) +
		x / 8 + y * LIBGGI_FB_W_STRIDE(vis);

	color = buff[0] << 8;
	i = x & 7;
	if (i) {
		j = w + i - 8;
		if (j < 0)
			mask = (0xff << i) & (0xff >> -j); 
		else
			mask = 0xff << i;

		i -= diff;
		if (i <= 0) {
			i += 8;
			color >>= 8;
			if (diff + w > 8)
				color |= *(++buff) << 8;
		}
		
		foo = *adr & ~mask;
		*adr = foo | ((color >> (8 - i)) & mask);

		if (j < 0)
			return 0;
		adr++;
	}
	else {
		j = w;
		i = 8 - diff;
	}

	while ((j -= 8) > 0) {
		color >>= 8;
		color |= *(++buff) << 8;
		*adr++ = color >> (8 - i);
	}

	color >>= 8;
	if (!j) {
		if (i != 8)
			color |= *(++buff) << 8;
		*adr = color >> (8 - i);
		return 0;
	}

	if (i < j + 8)
		color |= *(++buff) << 8;

	mask = 0xff >> -j;
	foo = *adr & ~mask;
	*adr = foo | ((color >> (8 - i)) & mask);

	return 0;
}

int
GGI_lin1r_unpacked_gethline(struct ggi_visual *vis,
	int x, int y, int w, void *buffer)
{ 
	uint8_t *adr;
	uint8_t *buff = (uint8_t *)buffer;
	int i;
	uint8_t bm;

	PREPARE_FB(vis);

	adr = (uint8_t *)LIBGGI_CURREAD(vis) +
		x / 8 + y * LIBGGI_FB_R_STRIDE(vis);

	bm = 1 << (x & 7);
	for (i = 0; i < w; ++i, bm <<= 1) {
		if (!bm) {
			bm = 1;
			++adr;
		}
		*buff++ = !!(*adr & bm);
	}

  	return 0;
}

int
GGI_lin1r_packed_gethline(struct ggi_visual *vis,
	int x, int y, int w, void *buffer)
{ 
	uint8_t *adr;
	uint8_t *buff = (uint8_t *)buffer;
	int i,j;
	uint8_t color;

	if (w <= 0)
		return 0;
	
	PREPARE_FB(vis);

	adr = (uint8_t *)LIBGGI_CURREAD(vis) +
		x / 8 + y * LIBGGI_FB_R_STRIDE(vis);

	i = x & 7;
	if (!i) {
		memcpy(buff, adr, (w + 7) >> 3);
		return 0;
	}

	j = i + w - 8;
	*buff = *adr++ >> i;

	if (j <= 0)
		return 0;
	
	for (; j > 0; j -= 8) {
		color = *adr++;
		*buff++ |= color << (8 - i);
		*buff = color >> i;
	}

	if (j & 7)
		*buff |= *adr << (8 - i);
  
	return 0;
}
