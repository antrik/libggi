/* $Id: color.c,v 1.1 2007/01/23 16:08:32 pekberg Exp $
******************************************************************************

   Linear 2 pixel handling (high-pair-right)

   Copyright (C) 1997 Jason McMullan  [jmcc@ggi-project.org]
   Copyright (C) 1998 Andrew Apted    [andrew@ggi-project.org]
   Copyright (C) 2007 Peter Rosin     [peda@lysator.liu.se]

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

#include "lin2rlib.h"

/* Pack the colors into an array
 */
int
GGI_lin2r_packcolors(struct ggi_visual *vis,
	void *outbuf, const ggi_color *cols, int len)
{
	uint8_t tmp = 0;
	uint8_t *obuf = (uint8_t *)outbuf;
	int mask, i;

	mask = 0;
	for (i = 0; i < len; ++i) {
		tmp |= LIBGGIMapColor(vis, cols++) << mask;
		mask += 2;
		if (mask > 6) {
			*obuf++ = tmp;
			tmp = 0;
			mask = 0;
		}
	}
	if (mask)
		*obuf = tmp;

	return 0;
}	
	
/* Unpack into the ggi_color array the values of the pixels
 */
int
GGI_lin2r_unpackpixels(struct ggi_visual *vis,
	const void *inbuf, ggi_color *cols, int len)
{
	const uint8_t *ibuf = (const uint8_t *)inbuf;
	int i, mask;
	ggi_pixel tmp;

	mask = 0;
	for (i = 0; i < len; ++i) {
		tmp = (*ibuf >> mask) & 3;
		LIBGGIUnmapPixel(vis, tmp, cols++);
		mask += 2;
		if (mask > 6) {
			++ibuf;
			mask = 0;
		}
	}
	return 0;
}	