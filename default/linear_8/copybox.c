/* $Id: copybox.c,v 1.2 2003/07/05 22:13:42 cegger Exp $
******************************************************************************

   LibGGI linear 8 - copybox

   Copyright (C) 1997 Jason McMullan   [jmcc@ggi-project.org]

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


int
GGI_lin8_copybox(ggi_visual *vis, int x, int y, int w, int h, int nx, int ny)
{
	uint8 *src, *dest;
	int stride = LIBGGI_FB_W_STRIDE(vis);
	int line;

	LIBGGICLIP_COPYBOX(vis, x, y, w, h, nx, ny);
	PREPARE_FB(vis);

	if (ny < y) {
		src  = (uint8 *)LIBGGI_CURREAD(vis)  + y*stride  + x;
		dest = (uint8 *)LIBGGI_CURWRITE(vis) + ny*stride + nx;
		for (line=0; line < h; line++, src += stride, dest += stride) {
			memmove(dest, src, (size_t)(w));
		}
	} else {
		src  = (uint8 *)LIBGGI_CURREAD(vis)  + (y+h-1)*stride  + x;
		dest = (uint8 *)LIBGGI_CURWRITE(vis) + (ny+h-1)*stride + nx;
		for (line=0; line < h; line++, src -= stride, dest -= stride) {
			memmove(dest, src, (size_t)(w));
		}
	}

	return 0;
}
