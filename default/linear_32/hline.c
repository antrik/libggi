/* $Id: hline.c,v 1.5 2005/07/30 11:40:01 cegger Exp $
******************************************************************************

   Graphics library for GGI. Horizontal lines.

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

#include <string.h>

#include "lin32lib.h"


static inline void
do_drawhline(ggi_visual *vis, int x, int y, int w)
{
	uint32_t *buf32;
	uint32_t color = LIBGGI_GC_FGCOLOR(vis);

	PREPARE_FB(vis);
	buf32 = (uint32_t*)((uint8_t*)LIBGGI_CURWRITE(vis)
			  + y*LIBGGI_FB_W_STRIDE(vis) + x*4);

	while (w--) {
		*(buf32++) = color;
	}
}


int GGI_lin32_drawhline(ggi_visual *vis, int x, int y, int w)
{
	LIBGGICLIP_XYW(vis, x, y, w);

	do_drawhline(vis, x, y, w);

	return 0;
}


int GGI_lin32_drawhline_nc(ggi_visual *vis, int x, int y, int w)
{
	do_drawhline(vis, x, y, w);
	
	return 0;
}


int GGI_lin32_puthline(ggi_visual *vis, int x, int y, int w, const void *buffer)
{
	const uint32_t *buf32 = buffer;
	uint8_t  *mem;

	LIBGGICLIP_XYW_BUFMOD(vis, x, y, w, buf32, *1);
	PREPARE_FB(vis);

	mem = (uint8_t *)LIBGGI_CURWRITE(vis) + y*LIBGGI_FB_W_STRIDE(vis) + x*4;
	memcpy(mem, buf32, (size_t)(w*4));

	return 0;
}


int GGI_lin32_gethline(ggi_visual *vis, int x, int y, int w, void *buffer)
{
	uint8_t *mem;

	PREPARE_FB(vis);

	mem = (uint8_t *)LIBGGI_CURREAD(vis) + y*LIBGGI_FB_R_STRIDE(vis) + x*4;

	memcpy(buffer, mem, (size_t)(w*4));

	return 0;
}
