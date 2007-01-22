/* $Id: color.c,v 1.5 2007/01/22 12:23:46 pekberg Exp $
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

static inline int
unpacked_packcolors(struct ggi_visual *vis,
	uint8_t *obuf, const ggi_color *cols, int len)
{
	int i;

	for (i = 0; i < len; ++i)
		*obuf++ = ggiMapColor(vis->stem, cols++);

	return 0;
}

/* Pack the colors into an array
 */
int GGI_lin1_packcolors(struct ggi_visual *vis, void *outbuf, const ggi_color *cols,int len)
{
	uint8_t tmp=0,*obuf=(uint8_t *)outbuf;
	int mask,i;

	if (!(GT_SUBSCHEME(LIBGGI_GT(vis)) & GT_SUB_PACKED_GETPUT))
		return unpacked_packcolors(vis, obuf, cols, len);

	mask=7;
	for (i=0;i<len;i++) {
		tmp |= ggiMapColor(vis->stem,(cols++)) << mask--;
		if (mask<0) {
			*(obuf++)=tmp;
			tmp=0;
			mask=7;
		}
	}

	return 0;
}	
	
static inline int
unpacked_unpackpixels(struct ggi_visual *vis,
	const uint8_t *ibuf, ggi_color *cols, int len)
{
	int i;

	for (i = 0; i < len; ++i)
		ggiUnmapPixel(vis->stem, *ibuf++ & 1, cols++);

	return 0;
}

/* Unpack into the ggi_color array the values of the pixels
 */
int GGI_lin1_unpackpixels(struct ggi_visual *vis,const void *inbuf,ggi_color *cols,int len)
{
	const uint8_t *ibuf=(const uint8_t *)inbuf;
	int i,mask;
	ggi_pixel tmp;

	if (!(GT_SUBSCHEME(LIBGGI_GT(vis)) & GT_SUB_PACKED_GETPUT))
		return unpacked_unpackpixels(vis, ibuf, cols, len);

	mask=7;
	for (i=0;i<len;i++) {
		tmp=((*ibuf) >> mask--)&1;
		ggiUnmapPixel(vis->stem,tmp,(cols++));
		if (mask<0) {
			ibuf++;
			mask=7;
		}
	}
	return 0;
}	
