/* $Id: copybox.c,v 1.3 2005/07/30 11:40:01 cegger Exp $
******************************************************************************

   LibGGI linear 4 - copybox

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

#include "lin4lib.h"


int
GGI_lin4_copybox(ggi_visual *vis, int x, int y, int w, int h, int nx, int ny)
{
	uint8_t *src, *dest;
	int linew = LIBGGI_FB_W_STRIDE(vis);
	int line;
	int left, right;
	
	LIBGGICLIP_COPYBOX(vis, x, y, w, h, nx, ny);
	PREPARE_FB(vis);

	/* if x is odd, left pixel dangles. */
	/* if x is odd and w is even, right dangles. */
	/* if x is even and w is odd, right dangles. */

	left  = (x & 0x01);
	right = (x ^ w) & 0x01;
	w -= left + right;
	
	if (ny < y) {
		src  = (uint8_t *)LIBGGI_CURWRITE(vis) + y*linew  + (x/2);
		dest = (uint8_t *)LIBGGI_CURWRITE(vis) + ny*linew + (nx/2);
		if (left) {
			dest++;
			src++;
		}
		for (line=0; line < h; line++, src += linew, dest += linew) {
			if (left) {
				*(dest-1) = (*(dest-1) & 0xF0) | *(src-1);
			}
			memmove(dest, src, (size_t)(w/2));
			if (right) {
				*(dest+w) = (*(dest+w) & 0x0F)
					| (*(src+w) << 4);
			}
		}
	} else {
		src  = (uint8_t *)LIBGGI_CURWRITE(vis) + (y+h-1)*linew + (x/2);
		dest = (uint8_t *)LIBGGI_CURWRITE(vis) + (ny+h-1)*linew+ (nx/2);
		if (left) {
			dest++;
			src++;
		}
		for (line=0; line < h; line++, src -= linew, dest -= linew) {
			if (left) {
				*(dest-1) = (*(dest-1) & 0xF0) | *(src-1);
			}
			memmove(dest, src, (size_t)(w/2));
			if (right) {
				*(dest+w) = (*(dest+w) & 0x0F)
					| (*(src+w) << 4);
			}
		}
	}

	return 0;
}


