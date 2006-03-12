/* $Id: box.c,v 1.5 2006/03/12 23:15:08 soyt Exp $
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

#include "lin24lib.h"


int GGI_lin24_drawbox(struct ggi_visual *vis, int origx, int y, int origw, int h)
{
	uint32_t colors[3], *dest32;
	uint8_t *colp, *dest8;
	int i, linediff;
	
	LIBGGICLIP_XYWH(vis, origx, y, origw, h);
	PREPARE_FB(vis);

	colp = (uint8_t*) colors;
	for (i = 0; i < 4; i++) {
		*colp++ = LIBGGI_GC_FGCOLOR(vis)      ;
		*colp++ = LIBGGI_GC_FGCOLOR(vis) >>  8;
		*colp++ = LIBGGI_GC_FGCOLOR(vis) >> 16;
	}
	colp = (uint8_t*) colors;

	dest8 = (uint8_t*)LIBGGI_CURWRITE(vis)
		+ y*LIBGGI_FB_W_STRIDE(vis) + origx*3;
	linediff = LIBGGI_FB_W_STRIDE(vis) - origw*3;

	while (h) {
		int x = origx+y*LIBGGI_FB_W_STRIDE(vis);
		int w = origw;
		while ((x & 3)) {
			*dest8++ = colp[0];
			*dest8++ = colp[1];
			*dest8++ = colp[2];
			x++;
			w--;
			if (!w) goto end_of_loop;
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
			*dest8++ = colp[0];
			*dest8++ = colp[1];
			*dest8++ = colp[2];
			w--;
		}
	  end_of_loop:
		h--;
		dest8 += linediff;
	}

	return 0;
}


int GGI_lin24_putbox(struct ggi_visual *vis, int x, int y, int w, int h, const void *buffer)
{
	const uint8_t *src = buffer;
	uint8_t *dest;
	int srcwidth = w*3;
	int destwidth = LIBGGI_FB_W_STRIDE(vis);

	LIBGGICLIP_PUTBOX(vis,x,y,w,h,src,srcwidth, *3);
	PREPARE_FB(vis);
	
	dest = (uint8_t *)LIBGGI_CURWRITE(vis) + (y*destwidth + x*3);

	/* Width should be in bytes */
	w *= 3;

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

