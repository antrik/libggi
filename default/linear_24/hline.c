/* $Id: hline.c,v 1.4 2005/07/30 11:40:00 cegger Exp $
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

#include "lin24lib.h"


static inline void
do_drawhline(ggi_visual *vis, int x, int y, int w)
{
	uint32_t colors[3], *dest32, i;
	uint8_t *colp = (uint8_t*)colors, *dest8;
	
	PREPARE_FB(vis);
	dest8 = (uint8_t *)LIBGGI_CURWRITE(vis)
		+ y*LIBGGI_FB_W_STRIDE(vis) + x*3;

	while ((x & 3)) {
		*dest8++ = LIBGGI_GC_FGCOLOR(vis)      ;
		*dest8++ = LIBGGI_GC_FGCOLOR(vis) >>  8;
		*dest8++ = LIBGGI_GC_FGCOLOR(vis) >> 16;
		x++;
		w--;
		if (!w) return;
	}
	for (i = 0; i < 4; i++) {
		*colp++ = LIBGGI_GC_FGCOLOR(vis)      ;
		*colp++ = LIBGGI_GC_FGCOLOR(vis) >>  8;
		*colp++ = LIBGGI_GC_FGCOLOR(vis) >> 16;
	}
	dest32 = (uint32_t*)dest8;
	while (w > 3) {
		*dest32++ = colors[0];
		*dest32++ = colors[1];
		*dest32++ = colors[2];
		w -= 4;
	}
	dest8 = (uint8_t*)dest32;
	while (w) {
		*dest8++ = LIBGGI_GC_FGCOLOR(vis)      ;
		*dest8++ = LIBGGI_GC_FGCOLOR(vis) >>  8;
		*dest8++ = LIBGGI_GC_FGCOLOR(vis) >> 16;
		w--;
	}
}


int GGI_lin24_drawhline(ggi_visual *vis,int x,int y,int w)
{
	LIBGGICLIP_XYW(vis, x, y, w);

	do_drawhline(vis, x, y, w);

	return 0;
}

int GGI_lin24_drawhline_nc(ggi_visual *vis, int x, int y, int w)
{
	do_drawhline(vis, x, y, w);

	return 0;
}

int GGI_lin24_puthline(ggi_visual *vis, int x, int y, int w, const void *buffer)
{ 
	const uint8_t *buf8 = buffer;

	LIBGGICLIP_XYW_BUFMOD(vis, x, y, w, buf8, *3);
	PREPARE_FB(vis);

	memcpy((uint8_t *)LIBGGI_CURWRITE(vis)
	       + y*LIBGGI_FB_W_STRIDE(vis) + x*3, buf8, (size_t)(w*3));

	return 0;
}

int GGI_lin24_gethline(ggi_visual *vis, int x, int y, int w, void *buffer)
{
	PREPARE_FB(vis);

	memcpy(buffer,(uint8_t *)LIBGGI_CURREAD(vis)
	       + y*LIBGGI_FB_R_STRIDE(vis) + x*3, (size_t)(w*3));

	return 0;
}
