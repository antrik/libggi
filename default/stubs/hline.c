/* $Id: hline.c,v 1.1 2001/05/12 23:01:49 cegger Exp $
******************************************************************************

   Generic horizontal lines.

   Copyright (C) 1995 Andreas Beck  [becka@ggi-project.org]
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

#include "stublib.h"


int GGI_stubs_drawhline(ggi_visual *vis, int x, int y, int w)
{
	LIBGGICLIP_XYW(vis, x, y, w);

	for (; w > 0; w--, x++) {
		LIBGGIDrawPixelNC(vis, x, y);
	}

	return 0;
}

int GGI_stubs_drawhline_nc(ggi_visual *vis, int x, int y, int w)
{
	for (; w > 0; w--, x++) {
		LIBGGIDrawPixelNC(vis, x, y);
	}

	return 0;
}


int _GGI_stubs_L1_puthline(ggi_visual *vis, int x, int y, int w, void *buffer)
{
	uint8 *src = (uint8 *) buffer;

	LIBGGICLIP_XYW_BUFMOD(vis, x, y, w, src, /**/);

	for (; w > 0; w--, x++, src++) {
		LIBGGIPutPixelNC(vis, x, y, *src);
	}

	return 0;
}

int _GGI_stubs_L2_puthline(ggi_visual *vis, int x, int y, int w, void *buffer)
{
	uint16 *src = (uint16 *) buffer;

	LIBGGICLIP_XYW_BUFMOD(vis, x, y, w, src, /**/);

	for (; w > 0; w--, x++, src++) {
		LIBGGIPutPixelNC(vis, x, y, *src);
	}

	return 0;
}

int _GGI_stubs_L3_puthline(ggi_visual *vis, int x, int y, int w, void *buffer)
{
	uint8 *src = (uint8 *) buffer;

	LIBGGICLIP_XYW_BUFMOD(vis, x, y, w, src, *3);

	for (; w > 0; w--, x++, src += 3) {
		LIBGGIPutPixelNC(vis, x, y,
				 src[0] | (src[1] << 8) | (src[2] << 16));
	}

	return 0;
}

int _GGI_stubs_L4_puthline(ggi_visual *vis, int x, int y, int w, void *buffer)
{
	uint32 *src = (uint32 *) buffer;

	LIBGGICLIP_XYW_BUFMOD(vis, x, y, w, src, /**/);

	for (; w > 0; w--, x++, src++) {
		LIBGGIPutPixelNC(vis, x, y, *src);
	}

	return 0;
}



int _GGI_stubs_L1_gethline(ggi_visual *vis, int x, int y, int w, void *buffer)
{
	uint8 *dest = (uint8 *) buffer;
	ggi_pixel pix;
		
	for (; w > 0; w--, x++) {
		LIBGGIGetPixel(vis, x, y, &pix);
		*dest++ = (uint8) pix;
	}

	return 0;
}

int _GGI_stubs_L2_gethline(ggi_visual *vis, int x, int y, int w, void *buffer)
{
	uint16 *dest = (uint16 *) buffer;
	ggi_pixel pix;

	for (; w > 0; w--, x++) {
		LIBGGIGetPixel(vis, x, y, &pix);
		*dest++ = (uint16) pix;
	}

	return 0;
}

int _GGI_stubs_L3_gethline(ggi_visual *vis, int x, int y, int w, void *buffer)
{
	uint8 *dest = (uint8 *) buffer;
	ggi_pixel pix;

	for (; w > 0; w--, x++) {
		LIBGGIGetPixel(vis, x, y, &pix);
		*dest++ = (uint8) pix; pix >>= 8;
		*dest++ = (uint8) pix; pix >>= 8;
		*dest++ = (uint8) pix;
	}

	return 0;
}

int _GGI_stubs_L4_gethline(ggi_visual *vis, int x, int y, int w, void *buffer)
{
	uint32 *dest = (uint32 *) buffer;
	ggi_pixel pix;

	for (; w > 0; w--, x++) {
		LIBGGIGetPixel(vis, x, y, &pix);
		*dest++ = (uint32) pix;
	}

	return 0;
}
