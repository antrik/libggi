/* $Id: crossblit.c,v 1.3 2002/10/09 22:54:08 cegger Exp $
******************************************************************************

   Graphics library for GGI.

   Copyright (C) 1997 Jason McMullan	[jmcc@ggi-project.org]
   Copyright (C) 1998 Andrew Apted	[andrew.apted@ggi-project.org]
   Copyright (C) 1998 Andreas Beck	[becka@ggi-project.org]
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

#include <string.h>
#include "config.h"
#include <ggi/internal/ggi-dl.h>


/* Default fallback */
static inline void
fallback(ggi_visual *src, int sx, int sy, int w, int h, 
	 ggi_visual *dst, int dx, int dy)
{
	ggi_pixel cur_src;
	ggi_pixel cur_dst = 0;
	uint8 *dstptr;
	int stride;

	LIBGGIGetPixel(src, sx, sy, &cur_src);
	cur_src++; /* assure safe init */
	
	stride = LIBGGI_FB_W_STRIDE(dst);
	dstptr = (uint8*) LIBGGI_CURWRITE(dst) + dy*stride + dx;

	for (; h > 0; h--, sy++, dy++) {
		int x;
		for (x=0; x < w; x++) {
			ggi_pixel pixel;

			LIBGGIGetPixel(src, sx+x, sy, &pixel);
			if (pixel != cur_src) {
				ggi_color col;
				LIBGGIUnmapPixel(src, pixel, &col);

				cur_dst = LIBGGIMapColor(dst, &col);
				cur_src = pixel;
			}
			*(dstptr+x) = cur_dst;
		}
		dstptr += stride;
	}
}


/* 8 bit to 8 bit crossblitting.
 */
static inline void
crossblit_8_to_8(ggi_visual *src, int sx, int sy, int w, int h,
		  ggi_visual *dst, int dx, int dy)
{
	uint8 *srcp, *dstp;
	int srcstride = LIBGGI_FB_R_STRIDE(src);
	int dststride = LIBGGI_FB_W_STRIDE(dst);
	uint8 conv_tab[256];

	GGIDPRINT_DRAW("linear-8: crossblit_8_to_8.\n");

	/* Creates the conversion table. A bit simplistic approach, perhaps?
	 */
	do {
		int i;
		for (i = 0; i < 256; i++) {
			ggi_color col;

			LIBGGIUnmapPixel(src, i, &col);
			conv_tab[i] = LIBGGIMapColor(dst, &col);
		}
	} while (0);

	srcp = (uint8*)LIBGGI_CURREAD(src)  + srcstride*sy + sx;
	dstp = (uint8*)LIBGGI_CURWRITE(dst) + dststride*dy + dx*2;

	srcstride -= w;
	dststride -= w;

	for (; h > 0; h--) {
		int i = w / 8;

		/* We don't believe in the optimizing capabilities of the
		 * compiler hence unroll manually.
		 */
		switch (w & 0x7) {
		default:
			for (; i > 0; i--) {
			case 0x0: *dstp++ = conv_tab[*srcp++];
			case 0x7: *dstp++ = conv_tab[*srcp++];
			case 0x6: *dstp++ = conv_tab[*srcp++];
			case 0x5: *dstp++ = conv_tab[*srcp++];
			case 0x4: *dstp++ = conv_tab[*srcp++];
			case 0x3: *dstp++ = conv_tab[*srcp++];
			case 0x2: *dstp++ = conv_tab[*srcp++];
			case 0x1: *dstp++ = conv_tab[*srcp++];
			}
		}

		srcp += srcstride;
		dstp += dststride;
	}
}

/* Blitting bewteen identical visuals
 */
static inline void
crossblit_same(ggi_visual *src, int sx, int sy, int w, int h,
	       ggi_visual *dst, int dx, int dy)
{
	uint8 *srcp, *dstp;
	int srcstride = LIBGGI_FB_R_STRIDE(src);
	int dststride = LIBGGI_FB_W_STRIDE(dst);

	GGIDPRINT_DRAW("linear-8: DB-accelerating crossblit.\n");
	
	srcp = (uint8*)LIBGGI_CURREAD(src)  + srcstride*sy + sx;
	dstp = (uint8*)LIBGGI_CURWRITE(dst) + dststride*dy + dx;

	for (; h != 0; h--) {
		memcpy(dstp, srcp, w);
		srcp += srcstride;
		dstp += dststride;
	}
}

int GGI_lin8_crossblit(ggi_visual *src, int sx, int sy, int w, int h, 
			ggi_visual *dst, int dx, int dy)
{
	LIBGGICLIP_COPYBOX(dst,sx,sy,w,h,dx,dy);
	PREPARE_FB(dst);

	/* Check if the src-read and dst-write buffers are in the same layout
	   and that the destination pixelformat is sane.
	 */
	if (src->r_frame && src->r_frame->layout == dst->w_frame->layout &&
	    dst->w_frame->buffer.plb.pixelformat->stdformat != 0) {
		uint32 srcformat
			= src->r_frame->buffer.plb.pixelformat->stdformat;
		uint32 dstformat
			= dst->w_frame->buffer.plb.pixelformat->stdformat;
		int pixels = w*h;

		PREPARE_FB(src);

		/* These are the accelerated cases. If neither suits then
		 * fall back to the default.
		 */
		if (srcformat == dstformat && pixels > 512) {
			if (memcmp(dst->palette, src->palette,
				   sizeof(ggi_color)*256) == 0) {
				crossblit_same(src, sx, sy, w, h, dst, dx, dy);
			} else {
				crossblit_8_to_8(src, sx, sy, w, h,
						 dst, dx, dy);
			}
			return 0;
		}
	}

	fallback(src, sx, sy, w, h, dst, dx, dy);
	return 0;
}
