/* $Id: color.c,v 1.5 2006/03/12 23:15:08 soyt Exp $
******************************************************************************

   Graphics library for GGI. pack/unpack

   Copyright (C) 1997 Jason McMullan	[jmcc@ggi-project.org]
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

#include "lin4lib.h"


/* Pack the colors into an array
 */
int GGI_lin4_packcolors(struct ggi_visual *vis, void *outbuf, const ggi_color *cols, int len)
{
	uint8_t *obuf = (uint8_t *)outbuf;
	int i;

	for (i=0; i < len/2; i++) {
		register uint8_t val;
		val = (uint8_t)(LIBGGIMapColor(vis, (cols++)) << 4);
		val |= (uint8_t)(LIBGGIMapColor(vis, (cols++)));
		*obuf = val;
		obuf++;
	}
	
	if (len & 1) {
		*obuf = LIBGGIMapColor(vis, cols) << 4;
	}
	
	return 0;
}

	
/* Unpack into the ggi_color array the values of the pixels
 */
int GGI_lin4_unpackpixels(struct ggi_visual *vis, const void *outbuf, ggi_color *cols, int len)
{
	const uint8_t *obuf = (const uint8_t *)outbuf;
	int i;
	ggi_pixel tmp;
	
	for (i=0; i < len/2; i++) {
		tmp = *obuf >> 4;
		LIBGGIUnmapPixel(vis, tmp, cols++);
		tmp = *(obuf++) & 0x0F;
		LIBGGIUnmapPixel(vis, tmp, cols++);
	}
	
	if (len & 1) {
		tmp = *obuf >> 4;
		LIBGGIUnmapPixel(vis, tmp, cols++);
	}

	return 0;
}	
	
  
