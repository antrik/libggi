/* $Id: box.c,v 1.6 2006/03/12 23:15:08 soyt Exp $
******************************************************************************

   Graphics library for GGI. Boxes.

   Copyright (C) 1999 Marcus Sundberg	[marcus@ggi-project.org]

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


int GGI_lin32_drawbox(struct ggi_visual *vis, int x, int y, int origw, int h)
{
	uint32_t *buf32;
	uint32_t color = LIBGGI_GC_FGCOLOR(vis);
	int linediff;

	LIBGGICLIP_XYWH(vis, x, y, origw, h);
	PREPARE_FB(vis);

	buf32 = (uint32_t*)((uint8_t*)LIBGGI_CURWRITE(vis)
			  + y*LIBGGI_FB_W_STRIDE(vis) + x*4);
	linediff = LIBGGI_FB_W_STRIDE(vis) - origw*4;

	while (h--) {
		int w = origw;
		while (w--) {
			*(buf32++) = color;
		}
		buf32 = (uint32_t*) ((uint8_t*)buf32 + linediff);
	}

	return 0;
}


int GGI_lin32_putbox(struct ggi_visual *vis, int x, int y, int w, int h, const void *buffer)
{
	const uint32_t *src = buffer;
	uint8_t *dest;
	int srcwidth = w;
	int destwidth = LIBGGI_FB_W_STRIDE(vis);

	LIBGGICLIP_PUTBOX(vis,x,y,w,h,src,srcwidth, *1);
	PREPARE_FB(vis);

	dest = (uint8_t *)LIBGGI_CURWRITE(vis) + (y*destwidth + x*4);

	/* Width should be in bytes */
	w *= 4;

	/* Optimized full-width case */
	if (w == destwidth && x == 0) {
		memcpy(dest, src, (size_t)(w*h));
		return 0;
	}
	
	while (h > 0) {
		memcpy(dest, src, (size_t)(w));
		dest += destwidth;
		src += srcwidth;
		h--;
	}
	return 0;
}

