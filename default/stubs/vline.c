/* $Id: vline.c,v 1.9 2008/01/20 22:14:57 pekberg Exp $
******************************************************************************

   Generic vertical lines.

   Copyright (C) 1995 Andreas Beck	[becka@ggi-project.org]
   Copyright (C) 1997 Jason McMullan	[jmcc@ggi-project.org]
   Copyright (C) 1998 Andrew Apted	[andrew@ggi-project.org]

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


int GGI_stubs_drawvline(struct ggi_visual *vis, int x, int y, int h)
{
	LIBGGICLIP_XYH(vis, x, y, h);

	for (; h > 0; h--, y++) {
		_ggiDrawPixelNC(vis, x, y);
	}
	
	return 0;
}

int GGI_stubs_drawvline_nc(struct ggi_visual *vis, int x, int y, int h)
{
	for (; h > 0; h--, y++) {
		_ggiDrawPixelNC(vis, x, y);
	}
	
	return 0;
}


int _GGI_stubs_L1_putvline(struct ggi_visual *vis, int x, int y, int h, const void *buffer)
{
	const uint8_t *src = (const uint8_t *) buffer;

	LIBGGICLIP_XYH_BUFMOD(vis, x, y, h, src, *1);

	for (; h > 0; h--, y++, src++) {
		_ggiPutPixelNC(vis, x, y, *src);
	}

	return 0;
}

int _GGI_stubs_L2_putvline(struct ggi_visual *vis, int x, int y, int h, const void *buffer)
{
	const uint16_t *src = (const uint16_t *) buffer;

	LIBGGICLIP_XYH_BUFMOD(vis, x, y, h, src, *1);

	for (; h > 0; h--, y++, src++) {
		_ggiPutPixelNC(vis, x, y, *src);
	}

	return 0;
}

int _GGI_stubs_L3_putvline(struct ggi_visual *vis, int x, int y, int h, const void *buffer)
{
	const uint8_t *src = (const uint8_t *) buffer;

	LIBGGICLIP_XYH_BUFMOD(vis, x, y, h, src, *3);

	for (; h > 0; h--, y++, src+=3) {
		_ggiPutPixelNC(vis, x, y,
			(unsigned)(src[0] | (src[1] << 8) | (src[2] << 16U)));
	}

	return 0;
}

int _GGI_stubs_L4_putvline(struct ggi_visual *vis, int x, int y, int h, const void *buffer)
{
	const uint32_t *src = (const uint32_t *) buffer;

	LIBGGICLIP_XYH_BUFMOD(vis, x, y, h, src, *1);

	for (; h > 0; h--, y++, src++) {
		_ggiPutPixelNC(vis, x, y, *src);
	}

	return 0;
}


int _GGI_stubs_L1_getvline(struct ggi_visual *vis, int x, int y, int h, void *buffer)
{
	uint8_t *dest = (uint8_t *) buffer;
	ggi_pixel pix;
		
	if (x < 0 || x >= LIBGGI_VIRTX(vis))
		return 0;

	if (y < 0) {
		h += y;
		dest -= y;
		y = 0;
	}
	if (y + h > LIBGGI_VIRTY(vis))
		h = LIBGGI_VIRTY(vis) - y;

	for (; h > 0; h--, y++) {
		_ggiGetPixelNC(vis, x, y, &pix);
		*dest++ = (uint8_t) pix;
	}

	return 0;
}

int _GGI_stubs_L2_getvline(struct ggi_visual *vis, int x, int y, int h, void *buffer)
{
	uint16_t *dest = (uint16_t *) buffer;
	ggi_pixel pix;

	if (x < 0 || x >= LIBGGI_VIRTX(vis))
		return 0;

	if (y < 0) {
		h += y;
		dest -= y;
		y = 0;
	}
	if (y + h > LIBGGI_VIRTY(vis))
		h = LIBGGI_VIRTY(vis) - y;

	for (; h > 0; h--, y++) {
		_ggiGetPixelNC(vis, x, y, &pix);
		*dest++ = (uint16_t) pix;
	}

	return 0;
}

int _GGI_stubs_L3_getvline(struct ggi_visual *vis, int x, int y, int h, void *buffer)
{
	uint8_t *dest = (uint8_t *) buffer;
	ggi_pixel pix;

	if (x < 0 || x >= LIBGGI_VIRTX(vis))
		return 0;

	if (y < 0) {
		h += y;
		dest -= y * 3;
		y = 0;
	}
	if (y + h > LIBGGI_VIRTY(vis))
		h = LIBGGI_VIRTY(vis) - y;

	for (; h > 0; h--, y++) {
		_ggiGetPixelNC(vis, x, y, &pix);
		*dest++ = (uint8_t) pix; pix >>= 8;
		*dest++ = (uint8_t) pix; pix >>= 8;
		*dest++ = (uint8_t) pix;
	}

	return 0;
}

int _GGI_stubs_L4_getvline(struct ggi_visual *vis, int x, int y, int h, void *buffer)
{
	uint32_t *dest = (uint32_t *) buffer;

	if (x < 0 || x >= LIBGGI_VIRTX(vis))
		return 0;

	if (y < 0) {
		h += y;
		dest -= y;
		y = 0;
	}
	if (y + h > LIBGGI_VIRTY(vis))
		h = LIBGGI_VIRTY(vis) - y;

	for (; h > 0; h--, y++) {
		ggi_pixel pix;
		
		_ggiGetPixelNC(vis, x, y, &pix);
		*dest++ = (uint32_t) pix;
	}

	return 0;
}
