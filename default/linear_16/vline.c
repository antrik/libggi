/* $Id: vline.c,v 1.4 2005/07/30 11:40:00 cegger Exp $
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

#include "lin16lib.h"


static inline void
do_drawvline(ggi_visual *vis, int x, int y, int h)
{
	uint16_t *ptr;
	int stride = LIBGGI_FB_W_STRIDE(vis)/2;
	uint16_t color = LIBGGI_GC_FGCOLOR(vis);

	PREPARE_FB(vis);

	ptr = ((uint16_t *)LIBGGI_CURWRITE(vis)) + y*stride + x;

	for (; h > 0; h--, ptr += stride) {
		*ptr = color; 
	}
}


int GGI_lin16_drawvline(ggi_visual *vis, int x, int y, int h)
{
	LIBGGICLIP_XYH(vis, x, y, h);
	
	do_drawvline(vis, x, y, h);
	
	return 0;
}

int GGI_lin16_drawvline_nc(ggi_visual *vis, int x, int y, int h)
{
	do_drawvline(vis, x, y, h);

	return 0;
}

int GGI_lin16_putvline(ggi_visual *vis, int x, int y, int h, const void *buffer)
{
	uint16_t *ptr; 
	const uint16_t *buf16 = buffer;
	int stride = LIBGGI_FB_W_STRIDE(vis)/2;

	LIBGGICLIP_XYH_BUFMOD(vis, x, y, h, buf16, *1);
	PREPARE_FB(vis);

	ptr = ((uint16_t *)LIBGGI_CURWRITE(vis)) + y*stride + x;

	for(; h > 0; h--, ptr += stride) {
		*ptr = *(buf16++); 
	}
	
	return 0;
}

int GGI_lin16_getvline(ggi_visual *vis, int x, int y, int h, void *buffer)
{
	uint16_t *ptr, *buf16;
	int stride = LIBGGI_FB_R_STRIDE(vis)/2;

	PREPARE_FB(vis);

	ptr = ((uint16_t *)LIBGGI_CURREAD(vis)) + y*stride + x;
	buf16  = buffer;

	for (; h > 0; h--, ptr += stride) {
		*(buf16++) = *ptr; 
	}
	
	return 0;
}
