/* $Id: crossblit.c,v 1.3 2002/09/29 19:27:41 skids Exp $
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

#define put24pixel(addr,pixel)	\
do { \
	(addr)[0] = (pixel) & 0xff; \
	(addr)[1] = ((pixel)>>8)  & 0xff; \
	(addr)[2] = ((pixel)>>16) & 0xff; \
}while(0)

/* Default fallback */
static inline void
fallback(ggi_visual *src, int sx, int sy, int w, int h, 
	 ggi_visual *dst, int dx, int dy)
{
	ggi_pixel cur_src;
	uint32 cur_dst = 0;
	uint8 *dstptr;
	int stride;

	GGIDPRINT_DRAW("linear-24: fallback\n");

	LIBGGIGetPixel(src, sx, sy, &cur_src);
	cur_src++; /* assure safe init */
	
	stride = LIBGGI_FB_W_STRIDE(dst);
	dstptr = (uint8*)LIBGGI_CURWRITE(dst) + dy*stride + dx*3;

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
			put24pixel(dstptr+x*3, cur_dst);
		}
		dstptr += stride;
	}
}


/* Blitting from a truecolor visual */
static inline void
true_fallback(ggi_visual *src, int sx, int sy, int w, int h, 
	      ggi_visual *dst, int dx, int dy, int srcsize)
{
	ggi_pixel cur_src;
	uint32 cur_dst = 0;
	uint8 *dstptr;
	uint8 *srcptr;
	int dststride, srcstride;
	int drmask, dgmask, dbmask;
	int srmask, sgmask, sbmask;
	int rshift, gshift, bshift;

	GGIDPRINT_DRAW("linear-24: true_fallback\n");

	dststride = LIBGGI_FB_W_STRIDE(dst);
	dstptr = (uint8*) LIBGGI_CURWRITE(dst) + dy*dststride + dx*3;
	srcstride = LIBGGI_FB_R_STRIDE(src);
	srcptr = (uint8*) LIBGGI_CURWRITE(src) + sy*srcstride + sx*srcsize;

	dststride -= w*3;
	srcstride -= w*srcsize;

	drmask = LIBGGI_PIXFMT(dst)->red_mask;
	dgmask = LIBGGI_PIXFMT(dst)->green_mask;
	dbmask = LIBGGI_PIXFMT(dst)->blue_mask;
	srmask = LIBGGI_PIXFMT(src)->red_mask;
	sgmask = LIBGGI_PIXFMT(src)->green_mask;
	sbmask = LIBGGI_PIXFMT(src)->blue_mask;
	
	rshift = LIBGGI_PIXFMT(src)->red_shift
		- LIBGGI_PIXFMT(dst)->red_shift;
	gshift = LIBGGI_PIXFMT(src)->green_shift
		- LIBGGI_PIXFMT(dst)->green_shift;
	bshift = LIBGGI_PIXFMT(src)->blue_shift
		- LIBGGI_PIXFMT(dst)->blue_shift;

	switch (srcsize) {
	case 1: {
		cur_src = *srcptr + 1; /* assure safe init */
	
		for (; h > 0; h--) {
			int ww = w;
			while (ww--) {
				ggi_pixel pixel = *srcptr;

				if (pixel != cur_src) {
					cur_dst	=
						(SSHIFT(pixel&srmask, rshift)
						 & drmask) |
						(SSHIFT(pixel&sgmask, gshift)
						 & dgmask) |
						(SSHIFT(pixel&sbmask, bshift)
						 & dbmask);
					cur_src = pixel;
				}
				put24pixel(dstptr, cur_dst);
				srcptr++;
				dstptr+=3;
			}
			srcptr += srcstride;
			dstptr += dststride;
		}
	}
	case 2: {
		uint16 *src16 = (uint16*) srcptr;
		cur_src = *src16 + 1; /* assure safe init */
	
		for (; h > 0; h--) {
			int ww = w;
			while (ww--) {
				ggi_pixel pixel = *src16;

				if (pixel != cur_src) {
					cur_dst	=
						(SSHIFT(pixel&srmask, rshift)
						 & drmask) |
						(SSHIFT(pixel&sgmask, gshift)
						 & dgmask) |
						(SSHIFT(pixel&sbmask, bshift)
						 & dbmask);
					cur_src = pixel;
				}
				put24pixel(dstptr, cur_dst);
				src16++;
				dstptr += 3;
			}
			src16  = (uint16*) ((uint8*)src16 + srcstride);
			dstptr += dststride;
		}
	}
#define get24pixel(addr)       	((addr)[0]+((addr)[1]<<8)+((addr)[2]<<16))
	case 3: {
		cur_src = get24pixel(srcptr) + 1; /* assure safe init */
	
		for (; h > 0; h--) {
			int ww = w;
			while (ww--) {
				ggi_pixel pixel = get24pixel(srcptr);

				if (pixel != cur_src) {
					cur_dst	=
						(SSHIFT(pixel&srmask, rshift)
						 & drmask) |
						(SSHIFT(pixel&sgmask, gshift)
						 & dgmask) |
						(SSHIFT(pixel&sbmask, bshift)
						 & dbmask);
					cur_src = pixel;
				}
				put24pixel(dstptr, cur_dst);
				srcptr += 3;
				dstptr += 3;
			}
			srcptr += srcstride;
			dstptr += dststride;
		}
	}
	case 4: {
		uint32 *src32 = (uint32*) srcptr;
		cur_src = *src32 + 1; /* assure safe init */
	
		for (; h > 0; h--) {
			int ww = w;
			while (ww--) {
				ggi_pixel pixel = *src32;

				if (pixel != cur_src) {
					cur_dst	=
						(SSHIFT(pixel&srmask, rshift)
						 & drmask) |
						(SSHIFT(pixel&sgmask, gshift)
						 & dgmask) |
						(SSHIFT(pixel&sbmask, bshift)
						 & dbmask);
					cur_src = pixel;
				}
				put24pixel(dstptr, cur_dst);
				src32++;
				dstptr += 3;
			}
			src32  = (uint32*) ((uint8*)src32 + srcstride);
			dstptr += dststride;
		}
	}
	}
}


/* 8 bit to 24 bit crossblitting.
 */
static inline void
crossblit_8_to_24(ggi_visual *src, int sx, int sy, int w, int h,
		  ggi_visual *dst, int dx, int dy)
{
	uint8 *srcp, *dstp;
	int srcstride = LIBGGI_FB_R_STRIDE(src);
	int dststride = LIBGGI_FB_W_STRIDE(dst);
	uint32 conv_tab[256];

	GGIDPRINT_DRAW("linear-24: crossblit_8_to_24.\n");

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
	dstp = (uint8*)LIBGGI_CURWRITE(dst) + dststride*dy + dx*3;

	srcstride -= w;
	dststride -= w*3;

	for (; h > 0; h--) {
		int i = w / 8;

		/* We don't believe in the optimizing capabilities of the
		 * compiler hence unroll manually.
		 */
		switch (w & 0x7) {
			for (; i > 0; i--) {
			case 0x0: put24pixel(dstp, conv_tab[*srcp]);
				dstp += 3;
				srcp++;
			case 0x7: put24pixel(dstp, conv_tab[*srcp]);
				dstp += 3;
				srcp++;
			case 0x6: put24pixel(dstp, conv_tab[*srcp]);
				dstp += 3;
				srcp++;
			case 0x5: put24pixel(dstp, conv_tab[*srcp]);
				dstp += 3;
				srcp++;
			case 0x4: put24pixel(dstp, conv_tab[*srcp]);
				dstp += 3;
				srcp++;
			case 0x3: put24pixel(dstp, conv_tab[*srcp]);
				dstp += 3;
				srcp++;
			case 0x2: put24pixel(dstp, conv_tab[*srcp]);
				dstp += 3;
				srcp++;
			case 0x1: put24pixel(dstp, conv_tab[*srcp]);
				dstp += 3;
				srcp++;
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

	GGIDPRINT_DRAW("linear-24: DB-accelerating crossblit.\n");
	
	srcp = (uint8*)LIBGGI_CURREAD(src)  + srcstride*sy + sx*3;
	dstp = (uint8*)LIBGGI_CURWRITE(dst) + dststride*dy + dx*3;

	/* Width should be in bytes */
	w *= 3;

	for (; h != 0; h--) {
		memcpy(dstp, srcp, w);
		srcp += srcstride;
		dstp += dststride;
	}
}


int GGI_lin24_crossblit(ggi_visual *src, int sx, int sy, int w, int h, 
			ggi_visual *dst, int dx, int dy)
{
	LIBGGICLIP_COPYBOX(dst,sx,sy,w,h,dx,dy);
	PREPARE_FB(dst);

	/* Check if the src-read and dst-write buffers are in the same layout
	   and that the destination pixelformat is sane.
	 */
	if (src->r_frame && src->r_frame->layout == dst->w_frame->layout) {
		uint32 srcformat
			= src->r_frame->buffer.plb.pixelformat->stdformat;
		uint32 dstformat
			= dst->w_frame->buffer.plb.pixelformat->stdformat;

		PREPARE_FB(src);

		if (dstformat != 0) {
			if (srcformat == GGI_DB_STD_8a8i8 && w * h > 256 ) {
				crossblit_8_to_24(src, sx, sy, w, h,
						  dst, dx, dy);
				return 0;
			} else if (srcformat == dstformat) {
				crossblit_same(src, sx, sy, w, h, dst, dx, dy);
				return 0;
			}
		}
		if (GT_SCHEME(LIBGGI_MODE(src)->graphtype) == GT_TRUECOLOR &&
		    (GT_SIZE(LIBGGI_MODE(src)->graphtype) % 8) == 0) {
			true_fallback(src, sx, sy, w, h, dst, dx, dy,
				      GT_SIZE(LIBGGI_MODE(src)->graphtype)/8);
			return 0;
		}
	}

	fallback(src, sx, sy, w, h, dst, dx, dy);
	return 0;
}
