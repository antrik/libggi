/* $Id: color.c,v 1.10 2008/01/20 22:14:55 pekberg Exp $
******************************************************************************

   Linear 1 pixel handling

   Copyright (C) 1997 Jason McMullan  [jmcc@ggi-project.org]
   Copyright (C) 1998 Andrew Apted  [andrew@ggi-project.org]

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

#include "lin1lib.h"

/* Pack the colors into an array
 */
int GGI_lin1_packcolors(struct ggi_visual *vis, void *outbuf, const ggi_color *cols,int len)
{
	uint8_t tmp=0,*obuf=(uint8_t *)outbuf;
	int mask,i;

	mask=7;
	for (i=0;i<len;i++) {
		tmp |= _ggiMapColor(vis,(cols++)) << mask--;
		if (mask<0) {
			*(obuf++)=tmp;
			tmp=0;
			mask=7;
		}
	}
	if (mask != 7)
		*obuf = tmp;

	return 0;
}	
	
/* Unpack into the ggi_color array the values of the pixels
 */
int GGI_lin1_unpackpixels(struct ggi_visual *vis,const void *inbuf,ggi_color *cols,int len)
{
	const uint8_t *ibuf=(const uint8_t *)inbuf;
	int i,mask;
	ggi_pixel tmp;

	mask=7;
	for (i=0;i<len;i++) {
		tmp=((*ibuf) >> mask--)&1;
		_ggiUnmapPixel(vis,tmp,(cols++));
		if (mask<0) {
			ibuf++;
			mask=7;
		}
	}
	return 0;
}	
