/* $Id: vline.c,v 1.4 2004/12/01 23:08:06 cegger Exp $
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


int GGI_stubs_drawvline(ggi_visual *vis, int x, int y, int h)
{
	LIBGGICLIP_XYH(vis, x, y, h);

	for (; h > 0; h--, y++) {
		LIBGGIDrawPixelNC(vis, x, y);
	}
	
	return 0;
}

int GGI_stubs_drawvline_nc(ggi_visual *vis, int x, int y, int h)
{
	for (; h > 0; h--, y++) {
		LIBGGIDrawPixelNC(vis, x, y);
	}
	
	return 0;
}


int _GGI_stubs_L1_putvline(ggi_visual *vis, int x, int y, int h, const void *buffer)
{
	const uint8 *src = (const uint8 *) buffer;

	LIBGGICLIP_XYH_BUFMOD(vis, x, y, h, src, *1);

	for (; h > 0; h--, y++, src++) {
		LIBGGIPutPixelNC(vis, x, y, *src);
	}

	return 0;
}

int _GGI_stubs_L2_putvline(ggi_visual *vis, int x, int y, int h, const void *buffer)
{
	const uint16 *src = (const uint16 *) buffer;

	LIBGGICLIP_XYH_BUFMOD(vis, x, y, h, src, *1);

	for (; h > 0; h--, y++, src++) {
		LIBGGIPutPixelNC(vis, x, y, *src);
	}

	return 0;
}

int _GGI_stubs_L3_putvline(ggi_visual *vis, int x, int y, int h, const void *buffer)
{
	const uint8 *src = (const uint8 *) buffer;

	LIBGGICLIP_XYH_BUFMOD(vis, x, y, h, src, *3);

	for (; h > 0; h--, y++, src+=3) {
		LIBGGIPutPixelNC(vis, x, y,
			(unsigned)(src[0] | (src[1] << 8) | (src[2] << 16U)));
	}

	return 0;
}

int _GGI_stubs_L4_putvline(ggi_visual *vis, int x, int y, int h, const void *buffer)
{
	const uint32 *src = (const uint32 *) buffer;

	LIBGGICLIP_XYH_BUFMOD(vis, x, y, h, src, *1);

	for (; h > 0; h--, y++, src++) {
		LIBGGIPutPixelNC(vis, x, y, *src);
	}

	return 0;
}


int _GGI_stubs_L1_getvline(ggi_visual *vis, int x, int y, int h, void *buffer)
{
	uint8 *dest = (uint8 *) buffer;
	ggi_pixel pix;
		
	for (; h > 0; h--, y++) {
		LIBGGIGetPixel(vis, x, y, &pix);
		*dest++ = (uint8) pix;
	}

	return 0;
}

int _GGI_stubs_L2_getvline(ggi_visual *vis, int x, int y, int h, void *buffer)
{
	uint16 *dest = (uint16 *) buffer;
	ggi_pixel pix;

	for (; h > 0; h--, y++) {
		LIBGGIGetPixel(vis, x, y, &pix);
		*dest++ = (uint16) pix;
	}

	return 0;
}

int _GGI_stubs_L3_getvline(ggi_visual *vis, int x, int y, int h, void *buffer)
{
	uint8 *dest = (uint8 *) buffer;
	ggi_pixel pix;

	for (; h > 0; h--, y++) {
		LIBGGIGetPixel(vis, x, y, &pix);
		*dest++ = (uint8) pix; pix >>= 8;
		*dest++ = (uint8) pix; pix >>= 8;
		*dest++ = (uint8) pix;
	}

	return 0;
}

int _GGI_stubs_L4_getvline(ggi_visual *vis, int x, int y, int h, void *buffer)
{
	uint32 *dest = (uint32 *) buffer;

	for (; h > 0; h--, y++) {
		ggi_pixel pix;
		
		LIBGGIGetPixel(vis, x, y, &pix);
		*dest++ = (uint32) pix;
	}

	return 0;
}
