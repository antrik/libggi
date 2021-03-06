/* $Id: vline.c,v 1.6 2007/05/18 21:53:45 pekberg Exp $
******************************************************************************

   Graphics library for GGI.

   Copyright (C) 1997 Jason McMullan	[jmcc@ggi-project.org]
   Copyright (C) 1995 Andreas Beck	[becka@ggi-project.org]

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

#include "lin8lib.h"


static inline void
do_drawvline(struct ggi_visual *vis, int x, int y, int h)
{
	uint8_t *ptr;
	int stride = LIBGGI_FB_W_STRIDE(vis);
	uint8_t color = LIBGGI_GC_FGCOLOR(vis);

	PREPARE_FB(vis);

	ptr = (uint8_t*)LIBGGI_CURWRITE(vis) + y*stride + x;

	for(; h > 0; h--, ptr += stride) {
		*ptr = color;
	} 
}


int GGI_lin8_drawvline(struct ggi_visual *vis, int x, int y, int h)
{
	LIBGGICLIP_XYH(vis, x, y, h);
	
	do_drawvline(vis, x, y, h);
	
	return 0;
}

int GGI_lin8_drawvline_nc(struct ggi_visual *vis, int x, int y, int h)
{
	do_drawvline(vis, x, y, h);

	return 0;
}

int GGI_lin8_putvline(struct ggi_visual *vis, int x, int y, int h, const void *buffer)
{
	uint8_t *ptr;
	const uint8_t *buf8 = buffer;
	int stride = LIBGGI_FB_W_STRIDE(vis);

	LIBGGICLIP_XYH_BUFMOD(vis, x, y, h, buf8, *1);
	PREPARE_FB(vis);

	ptr = ((uint8_t *)LIBGGI_CURWRITE(vis)) + y*stride + x;

	for(; h > 0; h--, ptr += stride) {
		*ptr = *(buf8++); 
	}
	
	return 0;
}

int GGI_lin8_getvline(struct ggi_visual *vis, int x, int y, int h, void *buffer)
{
	uint8_t *ptr, *buf8 = buffer;
	int stride = LIBGGI_FB_R_STRIDE(vis);

	if (x < 0 || x >= LIBGGI_VIRTX(vis))
		return 0;

	if (y < 0) {
		h += y;
		buf8 -= y;
		y = 0;
	}
	if (y + h > LIBGGI_VIRTY(vis))
		h = LIBGGI_VIRTY(vis) - y;
	if (h <= 0)
		return 0;

	PREPARE_FB(vis);

	ptr = ((uint8_t *)LIBGGI_CURREAD(vis)) + y*stride + x;

	for (; h > 0; h--, ptr += stride) {
		*(buf8++) = *ptr; 
	}
	
	return 0;
}
