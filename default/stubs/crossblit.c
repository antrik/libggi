/* $Id: crossblit.c,v 1.7 2008/01/20 22:14:57 pekberg Exp $
******************************************************************************

   Graphics library for GGI.

   Copyright (C) 1995 Andreas Beck     [becka@ggi-project.org]
   Copyright (C) 1997 Jason McMullan   [jmcc@ggi-project.org]
   Copyright (C) 1998 Andrew Apted     [andrew.apted@ggi-project.org]
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

#include "stublib.h"

/* Cross blitting of a rectangular area.
 */
int GGI_stubs_crossblit(struct ggi_visual *src, int sx, int sy, int w, int h, 
			struct ggi_visual *dst, int dx, int dy)
{
	ggi_pixel cur_src;
	ggi_pixel cur_dst = 0;

	LIBGGICLIP_CROSSBLIT(src, sx, sy, w, h, dst, dx, dy);
	
	_ggiGetPixelNC(src, sx, sy, &cur_src);
	cur_src++; /* assure safe init */

	for (; h > 0; h--, sy++, dy++) {
		int x;
		for (x=0; x < w; x++) {
			ggi_pixel pixel;

			_ggiGetPixelNC(src, sx+x, sy, &pixel);
			if (pixel != cur_src) {
				ggi_color col;
				_ggiUnmapPixel(src, pixel, &col);

				cur_dst = _ggiMapColor(dst, &col);
				cur_src = pixel;
			}
			
			_ggiPutPixelNC(dst, dx+x, dy, cur_dst);
		}
	}

	return 0;
}
