/* $Id: box.c,v 1.1 2001/05/12 23:01:45 cegger Exp $
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

#include "lin8lib.h"


int GGI_lin8_drawbox(ggi_visual *vis, int x, int y, int w, int h)
{
	uint8 *dest;
	int destwidth = LIBGGI_FB_W_STRIDE(vis);
	uint8 col = LIBGGI_GC_FGCOLOR(vis);

	LIBGGICLIP_XYWH(vis, x, y, w, h);
	PREPARE_FB(vis);

	dest = (uint8 *)LIBGGI_CURWRITE(vis) + (y*destwidth + x);

	/* Optimized full-width case */
	if (w == destwidth && x == 0) {
		memset(dest, col, w*h);
		return 0;
	}
	
	while (h > 0) {
		memset(dest, col, w);
		dest += destwidth;
		h--;
	}
	return 0;
}


int GGI_lin8_putbox(ggi_visual *vis, int x, int y, int w, int h, void *buffer)
{
	uint8 *src = buffer;
	uint8 *dest;
	int srcwidth = w;
	int destwidth = LIBGGI_FB_W_STRIDE(vis);

	LIBGGICLIP_PUTBOX(vis,x,y,w,h,src,srcwidth, );
	PREPARE_FB(vis);

	dest = (uint8 *)LIBGGI_CURWRITE(vis) + (y*destwidth + x);

	/* Optimized full-width case */
	if (w == destwidth && x == 0) {
		memcpy(dest, src, w*h);
		return 0;
	}
	
	while (h > 0) {
		memcpy(dest, src, w);
		dest += destwidth;
		src += srcwidth;
		h--;
	}
	return 0;
}
