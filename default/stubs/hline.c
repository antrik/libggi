/* $Id: hline.c,v 1.7 2007/04/04 18:45:40 ggibecka Exp $
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


int GGI_stubs_drawhline(struct ggi_visual *vis, int x, int y, int w)
{
	LIBGGICLIP_XYW(vis, x, y, w);

	for (; w > 0; w--, x++) {
		LIBGGIDrawPixelNC(vis, x, y);
	}

	return 0;
}

int GGI_stubs_drawhline_nc(struct ggi_visual *vis, int x, int y, int w)
{
	for (; w > 0; w--, x++) {
		LIBGGIDrawPixelNC(vis, x, y);
	}

	return 0;
}


int _GGI_stubs_L1_puthline(struct ggi_visual *vis, int x, int y, int w, const void *buffer)
{
	const uint8_t *src = (const uint8_t *) buffer;

	LIBGGICLIP_XYW_BUFMOD(vis, x, y, w, src, *1);

	for (; w > 0; w--, x++, src++) {
		LIBGGIPutPixelNC(vis, x, y, *src);
	}

	return 0;
}

int _GGI_stubs_L2_puthline(struct ggi_visual *vis, int x, int y, int w, const void *buffer)
{
	const uint16_t *src = (const uint16_t *) buffer;

	LIBGGICLIP_XYW_BUFMOD(vis, x, y, w, src, *1);

	for (; w > 0; w--, x++, src++) {
		LIBGGIPutPixelNC(vis, x, y, *src);
	}

	return 0;
}

int _GGI_stubs_L3_puthline(struct ggi_visual *vis, int x, int y, int w, const void *buffer)
{
	const uint8_t *src = (const uint8_t *) buffer;

	LIBGGICLIP_XYW_BUFMOD(vis, x, y, w, src, *3);

	for (; w > 0; w--, x++, src += 3) {
		LIBGGIPutPixelNC(vis, x, y,
			(unsigned)(src[0] | (src[1] << 8U) | (src[2] << 16U)));
	}

	return 0;
}

int _GGI_stubs_L4_puthline(struct ggi_visual *vis, int x, int y, int w, const void *buffer)
{
	const uint32_t *src = (const uint32_t *) buffer;

	LIBGGICLIP_XYW_BUFMOD(vis, x, y, w, src, *1);

	for (; w > 0; w--, x++, src++) {
		LIBGGIPutPixelNC(vis, x, y, *src);
	}

	return 0;
}



int _GGI_stubs_L1_gethline_nc(struct ggi_visual *vis, int x, int y, int w, void *buffer)
{
	uint8_t *dest = (uint8_t *) buffer;
	ggi_pixel pix;
		
	for (; w > 0; w--, x++) {
		LIBGGIGetPixel(vis, x, y, &pix);
		*dest++ = (uint8_t) pix;
	}

	return 0;
}

int _GGI_stubs_L1_gethline(struct ggi_visual *vis, int x, int y, int w, void *buffer)
{
	uint8_t *dest = (uint8_t *) buffer;
	ggi_pixel pix;

	/* clip to virtual size */
	if (y<0||y>=LIBGGI_VIRTY(vis)) return 0;
	if (x<0) {
		w+=x;	/* x is negative. w will _de_crease */
		dest-=x;
		x=0;
	}
	if (x+w>LIBGGI_VIRTX(vis)) {
		w=LIBGGI_VIRTX(vis)-x;
	}
	if (w<0) return 0;

	for (; w > 0; w--, x++) {
		LIBGGIGetPixel(vis, x, y, &pix);
		*dest++ = (uint8_t) pix;
	}

	return 0;
}

int _GGI_stubs_L2_gethline_nc(struct ggi_visual *vis, int x, int y, int w, void *buffer)
{
	uint16_t *dest = (uint16_t *) buffer;
	ggi_pixel pix;

	for (; w > 0; w--, x++) {
		LIBGGIGetPixel(vis, x, y, &pix);
		*dest++ = (uint16_t) pix;
	}

	return 0;
}

int _GGI_stubs_L2_gethline(struct ggi_visual *vis, int x, int y, int w, void *buffer)
{
	uint16_t *dest = (uint16_t *) buffer;
	ggi_pixel pix;

	/* clip to virtual size */
	if (y<0||y>=LIBGGI_VIRTY(vis)) return 0;
	if (x<0) {
		w+=x;	/* x is negative. w will _de_crease */
		dest-=x;
		x=0;
	}
	if (x+w>LIBGGI_VIRTX(vis)) {
		w=LIBGGI_VIRTX(vis)-x;
	}
	if (w<0) return 0;

	for (; w > 0; w--, x++) {
		LIBGGIGetPixel(vis, x, y, &pix);
		*dest++ = (uint16_t) pix;
	}

	return 0;
}

int _GGI_stubs_L3_gethline_nc(struct ggi_visual *vis, int x, int y, int w, void *buffer)
{
	uint8_t *dest = (uint8_t *) buffer;
	ggi_pixel pix;

	for (; w > 0; w--, x++) {
		LIBGGIGetPixel(vis, x, y, &pix);
		*dest++ = (uint8_t) pix; pix >>= 8;
		*dest++ = (uint8_t) pix; pix >>= 8;
		*dest++ = (uint8_t) pix;
	}

	return 0;
}

int _GGI_stubs_L3_gethline(struct ggi_visual *vis, int x, int y, int w, void *buffer)
{
	uint8_t *dest = (uint8_t *) buffer;
	ggi_pixel pix;

	/* clip to virtual size */
	if (y<0||y>=LIBGGI_VIRTY(vis)) return 0;
	if (x<0) {
		w+=x;	/* x is negative. w will _de_crease */
		dest-=3*x;
		x=0;
	}
	if (x+w>LIBGGI_VIRTX(vis)) {
		w=LIBGGI_VIRTX(vis)-x;
	}
	if (w<0) return 0;

	for (; w > 0; w--, x++) {
		LIBGGIGetPixel(vis, x, y, &pix);
		*dest++ = (uint8_t) pix; pix >>= 8;
		*dest++ = (uint8_t) pix; pix >>= 8;
		*dest++ = (uint8_t) pix;
	}

	return 0;
}

int _GGI_stubs_L4_gethline_nc(struct ggi_visual *vis, int x, int y, int w, void *buffer)
{
	uint32_t *dest = (uint32_t *) buffer;
	ggi_pixel pix;

	for (; w > 0; w--, x++) {
		LIBGGIGetPixel(vis, x, y, &pix);
		*dest++ = (uint32_t) pix;
	}

	return 0;
}

int _GGI_stubs_L4_gethline(struct ggi_visual *vis, int x, int y, int w, void *buffer)
{
	uint32_t *dest = (uint32_t *) buffer;
	ggi_pixel pix;

	/* clip to virtual size */
	if (y<0||y>=LIBGGI_VIRTY(vis)) return 0;
	if (x<0) {
		w+=x;	/* x is negative. w will _de_crease */
		dest-=x;
		x=0;
	}
	if (x+w>LIBGGI_VIRTX(vis)) {
		w=LIBGGI_VIRTX(vis)-x;
	}
	if (w<0) return 0;

	for (; w > 0; w--, x++) {
		LIBGGIGetPixel(vis, x, y, &pix);
		*dest++ = (uint32_t) pix;
	}

	return 0;
}
