/* $Id: vline.c,v 1.6 2007/05/18 21:56:09 pekberg Exp $
******************************************************************************

   Graphics library for GGI.

   Copyright (C) 1997 Jason McMullan [jmcc@ggi-project.org]
   Copyright (C) 1995 Andreas Beck [becka@ggi-project.org]

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

#include "lin32lib.h"


static inline void
do_drawvline(struct ggi_visual *vis, int x, int y, int h)
{
	uint32_t *ptr;
	int stride = LIBGGI_FB_W_STRIDE(vis)/4;
	uint32_t color = LIBGGI_GC_FGCOLOR(vis);

	PREPARE_FB(vis);
	ptr = ((uint32_t *)LIBGGI_CURWRITE(vis)) + y*stride + x;

	for (; h > 0; h--, ptr += stride) {
		*ptr = color; 
	}
}


int GGI_lin32_drawvline(struct ggi_visual *vis, int x, int y, int h)
{
	LIBGGICLIP_XYH(vis, x, y, h);
	
	do_drawvline(vis, x, y, h);
	
	return 0;
}

int GGI_lin32_drawvline_nc(struct ggi_visual *vis, int x, int y, int h)
{
	do_drawvline(vis, x, y, h);

	return 0;
}

int GGI_lin32_putvline(struct ggi_visual *vis, int x, int y, int h, const void *buffer)
{
	uint32_t *ptr;
	const uint32_t *buf32 = buffer;
	int stride = LIBGGI_FB_W_STRIDE(vis)/4;

	LIBGGICLIP_XYH_BUFMOD(vis, x, y, h, buf32, *1);
	PREPARE_FB(vis);

	ptr = ((uint32_t *)LIBGGI_CURWRITE(vis)) + y*stride + x;

	for(; h > 0; h--, ptr += stride) {
		*ptr = *(buf32++); 
	}
	
	return 0;
}

int GGI_lin32_getvline(struct ggi_visual *vis, int x, int y, int h, void *buffer)
{
	uint32_t *ptr, *buf32 = buffer;
	int stride = LIBGGI_FB_R_STRIDE(vis)/4;

	if (x < 0 || x >= LIBGGI_VIRTX(vis))
		return 0;

	if (y < 0) {
		h += y;
		buf32 -= y;
		y = 0;
	}
	if (y + h > LIBGGI_VIRTY(vis))
		h = LIBGGI_VIRTY(vis) - y;
	if (h <= 0)
		return 0;

	PREPARE_FB(vis);

	ptr = ((uint32_t *)LIBGGI_CURREAD(vis)) + y*stride + x;

	for (; h > 0; h--, ptr += stride) {
		*(buf32++) = *ptr; 
	}
	
	return 0;
}
