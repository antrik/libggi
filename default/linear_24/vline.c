/* $Id: vline.c,v 1.4 2006/03/12 23:15:08 soyt Exp $
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

#include "lin24lib.h"


static inline void
do_drawvline(struct ggi_visual *vis, int x, int y, int h)
{
	uint8_t *ptr;
	int stride = LIBGGI_FB_W_STRIDE(vis);
	uint8_t color0 = ((LIBGGI_GC_FGCOLOR(vis)    ) & 0xff);
	uint8_t color1 = ((LIBGGI_GC_FGCOLOR(vis)>>8 ) & 0xff);
	uint8_t color2 = ((LIBGGI_GC_FGCOLOR(vis)>>16) & 0xff);

	PREPARE_FB(vis);

	ptr = (uint8_t *)LIBGGI_CURWRITE(vis) + y*stride + x*3;

	for(; h > 0; h--, ptr += stride) {
		ptr[0] = color0;
		ptr[1] = color1;
		ptr[2] = color2;
	}
}


int GGI_lin24_drawvline(struct ggi_visual *vis, int x, int y, int h)
{
	LIBGGICLIP_XYH(vis, x, y, h);
	
	do_drawvline(vis, x, y, h);
	
	return 0;
}


int GGI_lin24_drawvline_nc(struct ggi_visual *vis, int x, int y, int h)
{
	do_drawvline(vis, x, y, h);

	return 0;
}


int GGI_lin24_putvline(struct ggi_visual *vis, int x, int y, int h, const void *buffer)
{
	uint8_t *ptr;
	const uint8_t *buf8 = buffer;
	int stride = LIBGGI_FB_W_STRIDE(vis);

	LIBGGICLIP_XYH_BUFMOD(vis, x, y, h, buf8, *3);
	PREPARE_FB(vis);

	ptr = (uint8_t *)LIBGGI_CURREAD(vis) + y*stride + x*3;

	for(; h > 0; h--, ptr += stride) {
		ptr[0] = buf8[0]; 
		ptr[1] = buf8[1]; 
		ptr[2] = buf8[2];
		buf8 += 3;
	}
	
	return 0;
}


int GGI_lin24_getvline(struct ggi_visual *vis, int x, int y, int h, void *buffer)
{
	uint8_t *ptr, *buf8;
	int stride = LIBGGI_FB_R_STRIDE(vis);

	PREPARE_FB(vis);

	ptr = (uint8_t *)LIBGGI_CURREAD(vis) + y*stride + x*3;
	buf8 = buffer;

	for(; h > 0; h--, ptr += stride) { 
		buf8[0] = ptr[0];
		buf8[1] = ptr[1];
		buf8[2] = ptr[2];
		buf8 += 3;
	}
	
	return 0;
}
