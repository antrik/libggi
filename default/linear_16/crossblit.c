/* $Id: crossblit.c,v 1.26 2008/01/20 22:14:55 pekberg Exp $
******************************************************************************

   16-bpp linear direct-access framebuffer renderer for LibGGI:

    -- functions implementing cross-blitting from other visuals.

   Copyright (C) 1997 Jason McMullan	[jmcc@ggi-project.org]
   Copyright (C) 1998 Andrew Apted	[andrew.apted@ggi-project.org]
   Copyright (C) 1998 Andreas Beck	[becka@ggi-project.org]
   Copyright (C) 1999 Marcus Sundberg	[marcus@ggi-project.org]
   Copyright (C) 2002 Brian S. Julin	[bri@tull.umassp.edu]

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
#include "lin16lib.h"
#include <ggi/internal/ggi_debug.h>

/* Default fallback to lower GGI primitive functions (slow).
 */
static inline void
fallback(struct ggi_visual *src, int sx, int sy, int w, int h, 
	 struct ggi_visual *dst, int dx, int dy)
{
	ggi_pixel cur_src;
	uint16_t cur_dst = 0;
	uint16_t *dstptr;
	int stride;

	DPRINT_DRAW("linear-16: Fallback to slow crossblit.\n");

	_ggiGetPixelNC(src, sx, sy, &cur_src);
	cur_src++; /* assure safe init */
	
	stride = LIBGGI_FB_W_STRIDE(dst);
	dstptr = (uint16_t*) ((uint8_t*)LIBGGI_CURWRITE(dst) + dy*stride + dx*2);

	for (; h > 0; h--, sy++, dy++) {
		int x;
		for (x=0; x < w; x++) {
			ggi_pixel pixel;

			_ggiGetPixelNC(src, sx+x, sy, &pixel);
			if (pixel != cur_src) {
				ggi_color col;
				_ggiUnmapPixel(src, pixel, &col);

				cur_dst = _ggiMapColor(dst, &col);
				cur_src = pixel;
			}
			*(dstptr+x) = cur_dst;
		}
		dstptr = (uint16_t*) ((uint8_t*)dstptr + stride);
	}
}

/* Blitting between identical visuals... simple memcpy.
 */
static inline void
crossblit_same(struct ggi_visual *src, int sx, int sy, int w, int h,
	       struct ggi_visual *dst, int dx, int dy)
{
	uint8_t *srcp, *dstp;
	int srcstride = LIBGGI_FB_R_STRIDE(src);
	int dststride = LIBGGI_FB_W_STRIDE(dst);

	DPRINT_DRAW("linear-16: direct memcpy crossblit.\n");
	
	srcp = (uint8_t*)LIBGGI_CURREAD(src)  + srcstride*sy + sx*2;
	dstp = (uint8_t*)LIBGGI_CURWRITE(dst) + dststride*dy + dx*2;

	w *= 2;		/* Use width in bytes. */

	for (; h != 0; h--) {
		memcpy(dstp, srcp, (size_t)(w));
		srcp += srcstride;
		dstp += dststride;
	}
}

/* 4 bit to 16 bit crossblitting.
 * TODO: a 256-entry lookup table could map two pixels at once and
 * avoid a lot of bit-fiddling.
 */
static inline void
cb4to16(struct ggi_visual *src, int sx, int sy, int w, int h,
	struct ggi_visual *dst, int dx, int dy)
{
	uint8_t *srcp, *dstp;
	int srcstride = LIBGGI_FB_R_STRIDE(src);
	int dststride = LIBGGI_FB_W_STRIDE(dst);
	uint16_t conv_tab[16];

	DPRINT_DRAW("linear-16: cb4to16.\n");

	do {
		unsigned int i;
		for (i = 0; i < 16; i++) {
			ggi_color col;

			_ggiUnmapPixel(src, i, &col);
			conv_tab[i] = _ggiMapColor(dst, &col);
		}
	} while (0);

	srcp = (uint8_t*)LIBGGI_CURREAD(src)  + srcstride*sy + sx/2;
	dstp = (uint8_t*)LIBGGI_CURWRITE(dst) + dststride*dy + dx*2;

	if (((sx ^ w) & 1) && (GT_SUBSCHEME(LIBGGI_GT(src)) & GT_SUB_HIGHBIT_RIGHT)) {
		for (; h > 0; --h) {
			uint16_t *dstpw = (uint16_t*) dstp;
			uint8_t  *srcpb = srcp;

			int i = (w + 7) / 8;

			/* Unroll manually. */
			switch (w & 0x7) {
			default:
				for (; i > 0; --i) {
				case 0:
				*dstpw++ = conv_tab[(*srcpb++ & 0xf0) >> 4];
				case 7:
				*dstpw++ = conv_tab[(*srcpb & 0x0f)];
				case 6:
				*dstpw++ = conv_tab[(*srcpb++ & 0xf0) >> 4];
				case 5:
				*dstpw++ = conv_tab[(*srcpb & 0x0f)];
				case 4:
				*dstpw++ = conv_tab[(*srcpb++ & 0xf0) >> 4];
				case 3:
				*dstpw++ = conv_tab[(*srcpb & 0x0f)];
				case 2:
				*dstpw++ = conv_tab[(*srcpb++ & 0xf0) >> 4];
				case 1:
				*dstpw++ = conv_tab[(*srcpb & 0x0f)];
				}
			}

			srcp += srcstride;
			dstp += dststride;
		}
		return;
	}
	if (GT_SUBSCHEME(LIBGGI_GT(src)) & GT_SUB_HIGHBIT_RIGHT) {
		for (; h > 0; --h) {
			uint16_t *dstpw = (uint16_t*) dstp;
			uint8_t  *srcpb = srcp;

			int i = (w + 7) / 8;

			/* Unroll manually. */			
			switch (w & 0x7) {
			default:
				for (; i > 0; --i) {
				case 0:
				*dstpw++ = conv_tab[(*srcpb & 0x0f)];
				case 7:
				*dstpw++ = conv_tab[(*srcpb++ & 0xf0) >> 4];
				case 6:
				*dstpw++ = conv_tab[(*srcpb & 0x0f)];
				case 5:
				*dstpw++ = conv_tab[(*srcpb++ & 0xf0) >> 4];
				case 4:
				*dstpw++ = conv_tab[(*srcpb & 0x0f)];
				case 3:
				*dstpw++ = conv_tab[(*srcpb++ & 0xf0) >> 4];
				case 2:
				*dstpw++ = conv_tab[(*srcpb & 0x0f)];
				case 1:
				*dstpw++ = conv_tab[(*srcpb++ & 0xf0) >> 4];
				}
			}

			srcp += srcstride;
			dstp += dststride;
		}
		return;
	}
	if ((sx ^ w) & 1) {
		for (; h > 0; h--) {
	    		uint16_t *dstpw = (uint16_t*) dstp;
			uint8_t  *srcpb = srcp;
			
			int i = (w + 7) / 8;
			
			/* Unroll manually. */
			switch (w & 0x7) {
			default:
				for (; i > 0; i--) {
				case 0:
				  *dstpw++ = conv_tab[(*srcpb++ & 0x0f)];
				case 7:
				  *dstpw++ = conv_tab[(*srcpb & 0xf0) >> 4];
				case 6:
				  *dstpw++ = conv_tab[(*srcpb++ & 0x0f)];
				case 5:
				  *dstpw++ = conv_tab[(*srcpb & 0xf0) >> 4];
				case 4:
				  *dstpw++ = conv_tab[(*srcpb++ & 0x0f)];
				case 3:
				  *dstpw++ = conv_tab[(*srcpb & 0xf0) >> 4];
				case 2:
				  *dstpw++ = conv_tab[(*srcpb++ & 0x0f)];
				case 1:
				  *dstpw++ = conv_tab[(*srcpb & 0xf0) >> 4];
				}
			}
			
			srcp += srcstride;
			dstp += dststride;
		}
	}
	else {
		for (; h > 0; h--) {
			uint16_t *dstpw = (uint16_t*) dstp;
			uint8_t  *srcpb = srcp;
			
			int i = (w + 7) / 8;

			/* Unroll manually. */			
			switch (w & 0x7) {
			default:
				for (; i > 0; i--) {
				case 0:
				  *dstpw++ = conv_tab[(*srcpb & 0xf0) >> 4];
				case 7:
				  *dstpw++ = conv_tab[(*srcpb++ & 0x0f)];
				case 6:
				  *dstpw++ = conv_tab[(*srcpb & 0xf0) >> 4];
				case 5:
				  *dstpw++ = conv_tab[(*srcpb++ & 0x0f)];
				case 4:
				  *dstpw++ = conv_tab[(*srcpb & 0xf0) >> 4];
				case 3:
				  *dstpw++ = conv_tab[(*srcpb++ & 0x0f)];
				case 2:
				  *dstpw++ = conv_tab[(*srcpb & 0xf0) >> 4];
				case 1:
				  *dstpw++ = conv_tab[(*srcpb++ & 0x0f)];
				}
			}
			
			srcp += srcstride;
			dstp += dststride;
		}
	}
}

/* 8 bit to 16 bit crossblitting.
 */
static inline void
cb8to16(struct ggi_visual *src, int sx, int sy, int w, int h,
	struct ggi_visual *dst, int dx, int dy)
{
	uint8_t *srcp, *dstp;
	int srcstride = LIBGGI_FB_R_STRIDE(src);
	int dststride = LIBGGI_FB_W_STRIDE(dst);
	uint16_t conv_tab[256];

	DPRINT_DRAW("linear-16: cb8to16.\n");

	/* Create a conversion table. */
	do {
		unsigned int i;
		for (i = 0; i < 256; i++) {
			ggi_color col;

			_ggiUnmapPixel(src, i, &col);
			conv_tab[i] = _ggiMapColor(dst, &col);
		}
	} while (0);

	srcp = (uint8_t*)LIBGGI_CURREAD(src)  + srcstride*sy + sx;
	dstp = (uint8_t*)LIBGGI_CURWRITE(dst) + dststride*dy + dx*2;

	for (; h > 0; h--) {
		uint16_t *dstpw = (uint16_t*) dstp;
		uint8_t  *srcpb = (uint8_t*)  srcp;
		int i = (w + 7) / 8;

		/* Unroll manually. */
		switch (w & 0x7) {
		default:
			for (; i > 0; i--) {
			case 0x0: *dstpw++ = conv_tab[*srcpb++];
			case 0x7: *dstpw++ = conv_tab[*srcpb++];
			case 0x6: *dstpw++ = conv_tab[*srcpb++];
			case 0x5: *dstpw++ = conv_tab[*srcpb++];
			case 0x4: *dstpw++ = conv_tab[*srcpb++];
			case 0x3: *dstpw++ = conv_tab[*srcpb++];
			case 0x2: *dstpw++ = conv_tab[*srcpb++];
			case 0x1: *dstpw++ = conv_tab[*srcpb++];
			}
		}

		srcp += srcstride;
		dstp += dststride;
	}
}



/* Create a table of shift and mask operations needed to translate the source 
 * visual pixelformat to that of the destination visual.  This is a complex 
 * do-all function that can create tables suitable for various different SWAR 
 * implementations.  Most of the complexities are mainly for MMX-style SWARs 
 * which have the most deficiencies, so the inlining should produce a much 
 * more simple function for other SWARs.
 *
 * src and dst are the visuals.
 * rshift, gshift, and bshift are temporary arrays used to unpack 
 *   the "bitmeaning" array in the visual's pixelformat.
 * shift is the location to store the first element of the column in the
 *   table which contains the shifts.  It may overlap with 
 *   rshift/gshift/bshift.
 * sskip is the number of bytes to skip between shift values, in case the
 *   SWAR works best when the shift and mask values are interleaved, and/or
 *   in case the SWAR works best with different size values than int32_t.
 * soff defines a bit offset added to bitshifts. The actual direction of the
 *   shift may be altered by this offset.
 * mask is the location to store the first element of the column in the 
 *   table which contains the bitmasks.  It must NOT overlap with 
 *   rshift/gshift/bshift.
 * mskip is the number of bytes to skip between mask values, in case the
 *   SWAR works best when the shift and mask values are interleaved, and/or
 *   in case the SWAR works best with different size values than ggi_pixel.
 * maskpost is a bitflag register:  
 *   If bit 0 is set than left masks are set to values that are appropriate 
 *     to apply after the shift operation, else before the shift operation.
 *   If bit 1 is set than right masks are set to values that are appropriate 
 *     to apply after the shift operation, else before the shift operation.
 * nl returns the number of actual left shifts after the effect of soff is 
 *     factored in.  This can also be found by counting the number 
 *     of nonzero shift values at the beggining of the shift column.
 * nr returns the number of actual right shifts after the effect of soff
 *     is factored in.  This can also be found by counting the 
 *     number of nonzero mask values in the mask column after the value 
 *     corresponding to the zero-shift value.
 * 
 * Thus, neglecting actual memory layout, the produced table looks like this:
 *
 * [mask != 0] [left shift count > 0]
 * [mask != 0] [left shift count > 0]
 * [mask != 0] [left shift count > 0]
 * [...]
 * [mask]      [shift == 0]
 * [mask != 0] [right shift count > 0]
 * [mask != 0] [right shift count > 0]
 * [mask != 0] [right shift count > 0]
 * [...]
 * [mask == 0]
 *
 * Note if nl is 0 there won't be any left shift rows and the table will
 * start with the zero shift, and if nr is 0 the zero mask will immediately
 * succeed the zero shift row.  The SWAR can either use nr and nl or branch
 * on the telltale zeroes in the table.
 *   
 */

static inline void build_masktab(struct ggi_visual *src, struct ggi_visual *dst, 
				 int32_t *rshift,int32_t *gshift,int32_t *bshift,
				 int32_t *shift, int sskip, int soff,
				 ggi_pixel *mask, int masklen, int mskip,
				 int maskpost, int *nl, int *nr) {
	int i, j;

	for (i = 0; i < masklen * mskip; i += mskip) mask[i] = 0;
	for (i = 0; i < 16 * sskip; i += sskip)
		rshift[i] = bshift[i] = gshift[i] = -1;

	for (i = 0; i < masklen - 16; i++) {
		ggi_pixel bm;
		int val;
		
		bm = src->r_frame->buffer.plb.pixelformat->bitmeaning[i];
		val = (bm & 0xff) - 240;
		if (val < 0) continue;
		
		switch(bm & 0xffffff00) {
		case GGI_BM_TYPE_COLOR | GGI_BM_SUB_RED:
			rshift[val * sskip] = i;
			break;
		case GGI_BM_TYPE_COLOR | GGI_BM_SUB_GREEN:
			gshift[val * sskip] = i;
			break;
		case GGI_BM_TYPE_COLOR | GGI_BM_SUB_BLUE:
			bshift[val * sskip] = i;
			break;
		default:
			break;
		}
	}

	/* Ensure pixel-correct fillout when destination channel is deeper. 
	 */
	for (i=15,j=15; i >= 0; i--) if (rshift[i * sskip] < 0)
		rshift[i * sskip] = rshift[j-- * sskip];
	for (i=15,j=15; i >= 0; i--) if (gshift[i * sskip] < 0)
		gshift[i * sskip] = gshift[j-- * sskip];
	for (i=15,j=15; i >= 0; i--) if (bshift[i * sskip] < 0)
		bshift[i * sskip] = bshift[j-- * sskip];

	for (i = 0; i < 16; i++) {
		ggi_pixel bm;
		int val, stmp;
        
		bm = dst->w_frame->buffer.plb.pixelformat->bitmeaning[i];
		val = (bm & 0xff) - 240;
		if (val < 0) continue;

#define SETMASK(arr) \
stmp = arr[val * sskip] + 15 - i;				\
if (stmp <= 15) {						\
	if (maskpost & 1) mask[stmp * mskip] |= 1 << i;		\
	else mask[stmp * mskip] |= 1 << arr[val * sskip];	\
} else {							\
	if (maskpost & 2) mask[stmp * mskip] |= 1 << i;	        \
	else mask[stmp * mskip] |= 1 << arr[val * sskip];	\
}

		switch(bm & 0xffffff00) {
		case GGI_BM_TYPE_COLOR | GGI_BM_SUB_RED:
		  SETMASK(rshift);
		  break;
		case GGI_BM_TYPE_COLOR | GGI_BM_SUB_GREEN:
		  SETMASK(gshift);
		  break;
		case GGI_BM_TYPE_COLOR | GGI_BM_SUB_BLUE:
		  SETMASK(bshift);
		  break;
		default:
		  break;
		}

#undef SETMASK

	}

	/* Precipitate the array of masks and generate accompanying shifts */
	for (i = 0, j = 0; i < 15 - soff; i++) 
		if (mask[i * mskip]) {
			mask[j * mskip] = mask[i * mskip];
			shift[j * sskip] = 15 - i - soff;
			j++;
		}
	*nl = j;
	mask[j * mskip] = mask[(15 - soff) * mskip];
	shift[j * sskip] = 0;
	j++; i++;
	for (; i < masklen; i++) 
		if (mask[i * mskip]) {
			mask[j * mskip] = mask[i * mskip];
			shift[j * sskip] = i - 15 + soff;
			j++;
		}
	*nr = j - *nl - 1;
	mask[j * mskip] = 0;

}

/* 24 bit to 16 bit crossblitting.
 */
static inline void cb24to16(struct ggi_visual *src, int sx, int sy, int w, int h, 
			    struct ggi_visual *dst, int dx, int dy) {
	int32_t shifts[48], rshifts[24];
	ggi_pixel masks[40], rmasks[24];
	int nl, nr;
	uint16_t *stoprow, *dstp;
	uint8_t *srcp;
	int dstride, sstride;
	
	DPRINT_DRAW("linear-16: cb24to16.\n");

	build_masktab(src, dst, shifts, shifts + 16, shifts + 32, 
		      shifts, 1, 0, masks, 40, 1, 0, &nl, &nr);

	dstp = (uint16_t*)((uint8_t*)LIBGGI_CURWRITE(dst) + 
			 dy*(LIBGGI_FB_W_STRIDE(dst)) + dx*2);
	srcp = (uint8_t*)LIBGGI_CURREAD(src) + 
	  sy*(LIBGGI_FB_R_STRIDE(src)) + sx*3;
	dstride = LIBGGI_FB_W_STRIDE(dst)/2;
	sstride = LIBGGI_FB_R_STRIDE(src);
	
	stoprow = dstp + h * dstride;
	dstride -= w;
	sstride -= w * 3;

	memcpy(rmasks, masks + nl + 1, nr * sizeof(ggi_pixel));
	memcpy(rshifts, shifts + nl + 1, nr * sizeof(int32_t));
	
	while (stoprow > dstp) {
		uint16_t *stopcol;

		stopcol = dstp + w;
		while (stopcol > dstp) {
			ggi_pixel tmp, cache;
			
			tmp = 0;
			cache = ((ggi_pixel)*(srcp + 2) << 16) | 
			  ((ggi_pixel)*(srcp + 1) << 8) | 
			  (ggi_pixel)*srcp;
			switch (nl) {
			case 15:
				tmp |= (cache & masks[14]) << shifts[14];
			case 14:
				tmp |= (cache & masks[13]) << shifts[13];
			case 13:
				tmp |= (cache & masks[12]) << shifts[12];
			case 12:
				tmp |= (cache & masks[11]) << shifts[11];
			case 11:
				tmp |= (cache & masks[10]) << shifts[10];
			case 10:
				tmp |= (cache & masks[9]) << shifts[9];
			case 9:
				tmp |= (cache & masks[8]) << shifts[8];
			case 8:
				tmp |= (cache & masks[7]) << shifts[7];
			case 7:
				tmp |= (cache & masks[6]) << shifts[6];
			case 6:
				tmp |= (cache & masks[5]) << shifts[5];
			case 5:
				tmp |= (cache & masks[4]) << shifts[4];
			case 4:
				tmp |= (cache & masks[3]) << shifts[3];
			case 3:
				tmp |= (cache & masks[2]) << shifts[2];
			case 2:
				tmp |= (cache & masks[1]) << shifts[1];
			case 1:
				tmp |= (cache & masks[0]) << shifts[0];
			case 0:
				break;
			}
			if (masks[nl]) { tmp |= cache & masks[nl]; }
			switch (nr) {
			case 23:
				tmp |= (cache & rmasks[22]) >> rshifts[22];
			case 22:
				tmp |= (cache & rmasks[21]) >> rshifts[21];
			case 21:
				tmp |= (cache & rmasks[20]) >> rshifts[20];
			case 20:
				tmp |= (cache & rmasks[19]) >> rshifts[19];
			case 19:
				tmp |= (cache & rmasks[18]) >> rshifts[18];
			case 18:
				tmp |= (cache & rmasks[17]) >> rshifts[17];
			case 17:
				tmp |= (cache & rmasks[16]) >> rshifts[16];
			case 16:
				tmp |= (cache & rmasks[15]) >> rshifts[15];
			case 15:
				tmp |= (cache & rmasks[14]) >> rshifts[14];
			case 14:
				tmp |= (cache & rmasks[13]) >> rshifts[13];
			case 13:
				tmp |= (cache & rmasks[12]) >> rshifts[12];
			case 12:
				tmp |= (cache & rmasks[11]) >> rshifts[11];
			case 11:
				tmp |= (cache & rmasks[10]) >> rshifts[10];
			case 10:
				tmp |= (cache & rmasks[9]) >> rshifts[9];
			case 9:
				tmp |= (cache & rmasks[8]) >> rshifts[8];
			case 8:
				tmp |= (cache & rmasks[7]) >> rshifts[7];
			case 7:
				tmp |= (cache & rmasks[6]) >> rshifts[6];
			case 6:
				tmp |= (cache & rmasks[5]) >> rshifts[5];
			case 5:
				tmp |= (cache & rmasks[4]) >> rshifts[4];
			case 4:
				tmp |= (cache & rmasks[3]) >> rshifts[3];
			case 3:
				tmp |= (cache & rmasks[2]) >> rshifts[2];
			case 2:
				tmp |= (cache & rmasks[1]) >> rshifts[1];
			case 1:
				tmp |= (cache & rmasks[0]) >> rshifts[0];
			case 0:
				break;
			}
			
			*dstp = tmp;
			dstp++;
			srcp += 3;
		}
		srcp += sstride;
		dstp += dstride;
	}
	return;
}

#ifdef DO_SWAR_NONE

/* 16 bit to 16 bit crossblitting.
 */
static inline void cb16to16(struct ggi_visual *src, int sx, int sy, int w, int h, 
			    struct ggi_visual *dst, int dx, int dy) {
	int shifts[48], rshifts[16];
	ggi_pixel masks[32], rmasks[16];
	int nl, nr;
	uint16_t *stoprow, *dstp, *srcp;
	int dstride, sstride;
	
	DPRINT_DRAW("linear-16: cb16to16.\n");

	build_masktab(src, dst, shifts, shifts + 16, shifts + 32, 
		      shifts, 1, 0, masks, 32, 1, 0, &nl, &nr);
		
	dstp = (uint16_t*)((uint8_t*)LIBGGI_CURWRITE(dst) + 
			 dy*(LIBGGI_FB_W_STRIDE(dst)) + dx*2);
	srcp = (uint16_t*)((uint8_t*)LIBGGI_CURREAD(src) + 
			 sy*(LIBGGI_FB_R_STRIDE(src)) + sx*2);
	dstride = LIBGGI_FB_W_STRIDE(dst)/2;
	sstride = LIBGGI_FB_R_STRIDE(src)/2;
	
	stoprow = dstp + h * dstride;
	dstride -= w;
	sstride -= w;

	memcpy(rmasks, masks + nl + 1, nr * sizeof(ggi_pixel));
	memcpy(rshifts, shifts + nl + 1, nr * sizeof(int32_t));
	
	while (stoprow > dstp) {
		uint16_t *stopcol;
		
		stopcol = dstp + w;
		while (stopcol > dstp) {
			ggi_pixel tmp, cache;
			
			tmp = 0;
			cache = *srcp;
			switch (nl) {
			case 15:
				tmp |= (cache & masks[14]) << shifts[14];
			case 14:
				tmp |= (cache & masks[13]) << shifts[13];
			case 13:
				tmp |= (cache & masks[12]) << shifts[12];
			case 12:
				tmp |= (cache & masks[11]) << shifts[11];
			case 11:
				tmp |= (cache & masks[10]) << shifts[10];
			case 10:
				tmp |= (cache & masks[9]) << shifts[9];
			case 9:
				tmp |= (cache & masks[8]) << shifts[8];
			case 8:
				tmp |= (cache & masks[7]) << shifts[7];
			case 7:
				tmp |= (cache & masks[6]) << shifts[6];
			case 6:
				tmp |= (cache & masks[5]) << shifts[5];
			case 5:
				tmp |= (cache & masks[4]) << shifts[4];
			case 4:
				tmp |= (cache & masks[3]) << shifts[3];
			case 3:
				tmp |= (cache & masks[2]) << shifts[2];
			case 2:
				tmp |= (cache & masks[1]) << shifts[1];
			case 1:
				tmp |= (cache & masks[0]) << shifts[0];
			case 0:
				break;
			}
			if (masks[nl]) tmp |= cache & masks[nl];
			switch (nr) {
			case 15:
				tmp |= (cache & rmasks[14]) >> rshifts[14];
			case 14:
				tmp |= (cache & rmasks[13]) >> rshifts[13];
			case 13:
				tmp |= (cache & rmasks[12]) >> rshifts[12];
			case 12:
				tmp |= (cache & rmasks[11]) >> rshifts[11];
			case 11:
				tmp |= (cache & rmasks[10]) >> rshifts[10];
			case 10:
				tmp |= (cache & rmasks[9]) >> rshifts[9];
			case 9:
				tmp |= (cache & rmasks[8]) >> rshifts[8];
			case 8:
				tmp |= (cache & rmasks[7]) >> rshifts[7];
			case 7:
				tmp |= (cache & rmasks[6]) >> rshifts[6];
			case 6:
				tmp |= (cache & rmasks[5]) >> rshifts[5];
			case 5:
				tmp |= (cache & rmasks[4]) >> rshifts[4];
			case 4:
				tmp |= (cache & rmasks[3]) >> rshifts[3];
			case 3:
				tmp |= (cache & rmasks[2]) >> rshifts[2];
			case 2:
				tmp |= (cache & rmasks[1]) >> rshifts[1];
			case 1:
				tmp |= (cache & rmasks[0]) >> rshifts[0];
			case 0:
				break;
			}
			
			*dstp = tmp;
			dstp++;
			srcp++;
		}
		srcp += sstride;
		dstp += dstride;
	}
	return;
}

/* 32 bit to 16 bit crossblitting.
 */
static inline void cb32to16(struct ggi_visual *src, int sx, int sy, int w, int h, 
			    struct ggi_visual *dst, int dx, int dy) {
	int32_t shifts[48], rshifts[32];
	ggi_pixel masks[48], rmasks[32];
	int nl, nr;
	uint16_t *stoprow, *dstp;
	uint32_t *srcp;
	int dstride, sstride;
	
	DPRINT_DRAW("linear-16: cb32to16.\n");

	build_masktab(src, dst, shifts, shifts + 16, shifts + 32, 
		      shifts, 1, 0, masks, 48, 1, 0, &nl, &nr);

	dstp = (uint16_t*)((uint8_t*)LIBGGI_CURWRITE(dst) + 
			 dy*(LIBGGI_FB_W_STRIDE(dst)) + dx*2);
	srcp = (uint32_t*)((uint8_t*)LIBGGI_CURREAD(src) + 
			 sy*(LIBGGI_FB_R_STRIDE(src)) + sx*4);
	dstride = LIBGGI_FB_W_STRIDE(dst)/2;
	sstride = LIBGGI_FB_R_STRIDE(src)/4;
		
	stoprow = dstp + h * dstride;
	dstride -= w;
	sstride -= w;

	memcpy(rmasks, masks + nl + 1, nr * sizeof(ggi_pixel));
	memcpy(rshifts, shifts + nl + 1, nr * sizeof(int32_t));

	while (stoprow > dstp) {
		uint16_t *stopcol;
		
		stopcol = dstp + w;
		while (stopcol > dstp) {
			ggi_pixel tmp, cache;

			tmp = 0;
			cache = *srcp;
			switch (nl) {
			case 15:
				tmp |= (cache & masks[14]) << shifts[14];
			case 14:
				tmp |= (cache & masks[13]) << shifts[13];
			case 13:
				tmp |= (cache & masks[12]) << shifts[12];
			case 12:
				tmp |= (cache & masks[11]) << shifts[11];
			case 11:
				tmp |= (cache & masks[10]) << shifts[10];
			case 10:
				tmp |= (cache & masks[9]) << shifts[9];
			case 9:
				tmp |= (cache & masks[8]) << shifts[8];
			case 8:
				tmp |= (cache & masks[7]) << shifts[7];
			case 7:
				tmp |= (cache & masks[6]) << shifts[6];
			case 6:
				tmp |= (cache & masks[5]) << shifts[5];
			case 5:
				tmp |= (cache & masks[4]) << shifts[4];
			case 4:
				tmp |= (cache & masks[3]) << shifts[3];
			case 3:
				tmp |= (cache & masks[2]) << shifts[2];
			case 2:
				tmp |= (cache & masks[1]) << shifts[1];
			case 1:
				tmp |= (cache & masks[0]) << shifts[0];
			case 0:
				break;
			}
			if (masks[nl]) { tmp |= cache & masks[nl]; }
			switch (nr) {
			case 31:
				tmp |= (cache & rmasks[30]) >> rshifts[30];
			case 30:
				tmp |= (cache & rmasks[29]) >> rshifts[29];
			case 29:
				tmp |= (cache & rmasks[28]) >> rshifts[28];
			case 28:
				tmp |= (cache & rmasks[27]) >> rshifts[27];
			case 27:
				tmp |= (cache & rmasks[26]) >> rshifts[26];
			case 26:
				tmp |= (cache & rmasks[25]) >> rshifts[25];
			case 25:
				tmp |= (cache & rmasks[24]) >> rshifts[24];
			case 24:
				tmp |= (cache & rmasks[23]) >> rshifts[23];
			case 23:
				tmp |= (cache & rmasks[22]) >> rshifts[22];
			case 22:
				tmp |= (cache & rmasks[21]) >> rshifts[21];
			case 21:
				tmp |= (cache & rmasks[20]) >> rshifts[20];
			case 20:
				tmp |= (cache & rmasks[19]) >> rshifts[19];
			case 19:
				tmp |= (cache & rmasks[18]) >> rshifts[18];
			case 18:
				tmp |= (cache & rmasks[17]) >> rshifts[17];
			case 17:
				tmp |= (cache & rmasks[16]) >> rshifts[16];
			case 16:
				tmp |= (cache & rmasks[15]) >> rshifts[15];
			case 15:
				tmp |= (cache & rmasks[14]) >> rshifts[14];
			case 14:
				tmp |= (cache & rmasks[13]) >> rshifts[13];
			case 13:
				tmp |= (cache & rmasks[12]) >> rshifts[12];
			case 12:
				tmp |= (cache & rmasks[11]) >> rshifts[11];
			case 11:
				tmp |= (cache & rmasks[10]) >> rshifts[10];
			case 10:
				tmp |= (cache & rmasks[9]) >> rshifts[9];
			case 9:
				tmp |= (cache & rmasks[8]) >> rshifts[8];
			case 8:
				tmp |= (cache & rmasks[7]) >> rshifts[7];
			case 7:
				tmp |= (cache & rmasks[6]) >> rshifts[6];
			case 6:
				tmp |= (cache & rmasks[5]) >> rshifts[5];
			case 5:
				tmp |= (cache & rmasks[4]) >> rshifts[4];
			case 4:
				tmp |= (cache & rmasks[3]) >> rshifts[3];
			case 3:
				tmp |= (cache & rmasks[2]) >> rshifts[2];
			case 2:
				tmp |= (cache & rmasks[1]) >> rshifts[1];
			case 1:
				tmp |= (cache & rmasks[0]) >> rshifts[0];
			case 0:
				break;
			}
			
			*dstp = tmp;
			dstp++;
			srcp++;
		}
		srcp += sstride;
		dstp += dstride;
	}
	return;
}

/* Main function hook -- does some common-case preprocessing and
 * dispatches to one of the above functions.
 */
int GGI_lin16_crossblit(struct ggi_visual *src, int sx, int sy, int w, int h, 
			struct ggi_visual *dst, int dx, int dy)
{
	LIBGGICLIP_CROSSBLIT(src, sx, sy, w, h, dst, dx, dy);

	PREPARE_FB(dst);

	/* Check if src read buffer is also a blPixelLinearBuffer. */
	if (src->r_frame == NULL) goto fallback;
	if (src->r_frame->layout != blPixelLinearBuffer) goto fallback;

	/* No optimizations yet for reverse endian and other such weirdness */
	if (LIBGGI_PIXFMT(src)->flags) goto fallback;
	if (LIBGGI_PIXFMT(dst)->flags) goto fallback;

	PREPARE_FB(src);

	switch (GT_SIZE(LIBGGI_GT(src))) {
	case 1:
		/* TODO */
	case 2:
		/* TODO */
		goto fallback;
	case 4:
		if (w * h > 15) cb4to16(src, sx, sy, w, h, dst, dx, dy);
		else goto fallback;
		return 0;
	case 8:
		if (w * h > 255) cb8to16(src, sx, sy, w, h, dst, dx, dy);
		else goto fallback;
		return 0;
	case 16:
		if (!dst->w_frame->buffer.plb.pixelformat->stdformat) 
		  goto notsame;
		if (dst->w_frame->buffer.plb.pixelformat->stdformat !=
		    src->r_frame->buffer.plb.pixelformat->stdformat) 
		  goto notsame;
		crossblit_same(src, sx, sy, w, h, dst, dx, dy);
		return 0;
	notsame:
		if (GT_SCHEME(LIBGGI_GT(src)) == GT_TRUECOLOR)
		  cb16to16(src, sx, sy, w, h, dst, dx, dy);
		else goto fallback;
		return 0;
	case 24:
		if (GT_SCHEME(LIBGGI_GT(src)) == GT_TRUECOLOR)
		  cb24to16(src, sx, sy, w, h, dst, dx, dy);
		else goto fallback;
		return 0;
	case 32:
		if (GT_SCHEME(LIBGGI_GT(src)) == GT_TRUECOLOR)
		  cb32to16(src, sx, sy, w, h, dst, dx, dy);
		else goto fallback;
		return 0;
	default:
		break;
	}
	
 fallback:
	fallback(src, sx, sy, w, h, dst, dx, dy);
	return 0;
}

#endif

#ifdef DO_SWAR_64BITC

/* 16 bit to 16 bit crossblitting using 64Bit C SWAR.
 */
static inline void cb16to16_64bitc(struct ggi_visual *src, int sx, int sy, int w, 
				   int h, struct ggi_visual *dst, int dx, int dy) {
	char sbuf[48 * 4 + 7], *stab;
	char mbuf[48 * 8 + 7], *mtab;
	int nl, nr;
	uint16_t *stoprow, *dstp, *srcp;
	int dstride, sstride, i;

	DPRINT_DRAW("linear-16: cb16to16_64bitc.\n");

	if (sizeof(char *) == 8) {
		stab = sbuf + (8 - (((uint64_t)sbuf) % 8));
		mtab = mbuf + (8 - (((uint64_t)mbuf) % 8));
	}
	else {
		stab = sbuf + (8 - (((uint32_t)sbuf) % 8));
		mtab = mbuf + (8 - (((uint32_t)mbuf) % 8));
	}

	build_masktab(src, dst, (int32_t *)stab, 
		      (int32_t *)(stab + 64), 
		      (int32_t *)(stab + 128),
		      (int32_t *)stab, 1, 0,
		      (ggi_pixel *)(mtab), 32, 2, 3, &nl, &nr);

	for (i = 0; i < 32; i++) {
		uint64_t val;
		val = *((ggi_pixel *)mtab + i * 2) & 0xffff;
		val |= val << 16;
		val |= val << 32;
		*((uint64_t *)mtab + i) = val;
	}

	dstp = (uint16_t*)((uint8_t*)LIBGGI_CURWRITE(dst) + 
			 dy*(LIBGGI_FB_W_STRIDE(dst)) + dx*2);
	srcp = (uint16_t*)((uint8_t*)LIBGGI_CURREAD(src) + 
			 sy*(LIBGGI_FB_R_STRIDE(src)) + sx*2);
	dstride = LIBGGI_FB_W_STRIDE(dst)/2;
	sstride = LIBGGI_FB_R_STRIDE(src)/2;

	stoprow = dstp + h * dstride;
	dstride -= w;
	sstride -= w;

	while (stoprow > dstp) {
		uint16_t *stopcol;
		
		stopcol = dstp + w;
		while (stopcol > dstp) {
			ggi_pixel tmp, cache;

			if ((stopcol > dstp + 3) && 
			    !((sizeof(char *) == 8) ?
			      ((uint64_t)dstp % 8) : ((uint32_t)dstp % 8))) {
				while (stopcol > dstp + 3) {
					uint64_t tmp64, cache64;

					tmp64 = 0;
					cache64 = *((uint64_t *)srcp);
					i = 0;

					while (*((uint64_t *)mtab + i)) {
						tmp64 |= (cache64 << 
							*((int32_t *)stab + i)) &
						  *((uint64_t *)mtab + i);
						if (!*((int32_t *)stab + i)) 
							goto doright64;
						i++;
					}
					if (!*((int32_t *)stab + i)) 
						goto doright64;
				done64:
					*((uint64_t *)dstp) = tmp64;
					dstp+=4;
					srcp+=4;
					continue;
					
				doright64:
					i++;
					while (*((uint64_t *)mtab + i)) {
						tmp64 |= (cache64 >>
							*((int32_t *)stab + i)) &
						  *((uint64_t *)mtab + i);
						i++;
					}
					goto done64;
				}
			}

			if (stopcol <= dstp) break;
			tmp = 0;
			cache = *srcp;
			i = 0;

			while (*((uint64_t *)mtab + i)) {
				tmp |= (cache << 
					*((int32_t *)stab + i)) &
				  *((ggi_pixel *)mtab + i * 2);
				if (!*((int32_t *)stab + i)) 
				  goto doright;
				i++;
			}
			if (!*((int32_t *)stab + i)) 
			  goto doright;
		done:
			*dstp = tmp;
			dstp++;
			srcp++;
			continue;

		doright:
			i++;
			while (*((uint64_t *)mtab + i)) {
				tmp |= (cache >>
					*((int32_t *)stab + i)) &
				  *((ggi_pixel *)mtab + i * 2);
				i++;
			}
			goto done;
		}
		dstp += dstride;
		srcp += sstride;
	}			
}

/* 32 bit to 16 bit crossblitting using 64bit C SWAR.
 */
static inline void cb32to16_64bitc(struct ggi_visual *src, int sx, int sy, int w, 
				   int h, struct ggi_visual *dst, int dx, int dy) {
	char sbuf[48 * 4 + 7], *stab;
	char mbuf[48 * 8 + 7], *mtab;
	int nl, nr;
	uint16_t *stoprow, *dstp;
	uint32_t *srcp;
	int dstride, sstride, i;

	DPRINT_DRAW("linear-16: cb32to16_64bitc.\n");

	if (sizeof(char *) == 8) {
		stab = sbuf + (8 - (((uint64_t)sbuf) % 8));
		mtab = mbuf + (8 - (((uint64_t)mbuf) % 8));
	}
	else {
		stab = sbuf + (8 - (((uint32_t)sbuf) % 8));
		mtab = mbuf + (8 - (((uint32_t)mbuf) % 8));
	}

	build_masktab(src, dst, (int32_t *)stab, 
		      (int32_t *)(stab + 64), 
		      (int32_t *)(stab + 128),
		      (int32_t *)stab, 1, 0,
		      (ggi_pixel *)(mtab), 48, 2, 3, &nl, &nr);

	for (i = 0; i < 32; i++) {
		uint64_t val;
		val = *((ggi_pixel *)mtab + i * 2) & 0xffff;
		val |= val << 32;
		*((uint64_t *)mtab + i) = val;
	}

	dstp = (uint16_t*)((uint8_t*)LIBGGI_CURWRITE(dst) + 
			 dy*(LIBGGI_FB_W_STRIDE(dst)) + dx*2);
	srcp = (uint32_t*)((uint8_t*)LIBGGI_CURREAD(src) + 
			 sy*(LIBGGI_FB_R_STRIDE(src)) + sx*4);
	dstride = LIBGGI_FB_W_STRIDE(dst)/2;
	sstride = LIBGGI_FB_R_STRIDE(src)/4;
		
	stoprow = dstp + h * dstride;
	dstride -= w;
	sstride -= w;
	
	while (stoprow > dstp) {
		uint16_t *stopcol;
		
		stopcol = dstp + w;
		while (stopcol > dstp) {
			uint32_t tmp;
			ggi_pixel cache;

			if ((stopcol > dstp + 3) && 
			    !((sizeof(char *) == 8) ?
			      ((uint64_t)dstp % 8) : ((uint32_t)dstp % 8))) {
				while (stopcol > dstp + 3) {
					uint64_t tmpa64, tmpb64;
					uint64_t cachea64, cacheb64;

					tmpa64 = 0;
					tmpb64 = 0;
					cachea64 = *((uint64_t *)srcp);
					cacheb64 = *((uint64_t *)srcp + 1);
					i = 0;

					while (*((uint64_t *)mtab + i)) {
						tmpa64 |= (cachea64 << 
							   *((int *)stab+i)) &
						  *((uint64_t *)mtab + i);
						tmpb64 |= (cacheb64 << 
							   *((int *)stab+i)) &
						  *((uint64_t *)mtab + i);
						if (!*((int *)stab + i)) 
							goto doright64;
						i++;
					}
					if (!*((int *)stab + i)) 
						goto doright64;
				done64:
#ifdef GGI_LITTLE_ENDIAN
					tmpa64 |= tmpa64 >> 16;
					tmpa64 &= 0x00000000ffffffffLL;
					tmpa64 |= tmpb64 << 32;
					tmpb64 &= 0x0000ffff00000000LL;
					tmpa64 |= tmpb64 << 16;
					*((uint64_t *)dstp) = tmpa64;
#else
					tmpb64 |= tmpa64 >> 16;
					tmpb64 &= 0x00000000ffffffffLL;
					tmpb64 |= tmpa64 << 32;
					tmpa64 &= 0x0000ffff00000000LL;
					tmpb64 |= tmpa64 << 16;
					*((uint64_t *)dstp) = tmpb64;
#endif
					dstp+=4;
					srcp+=4;
					continue;
					
				doright64:
					i++;
					while (*((uint64_t *)mtab + i)) {
						tmpa64 |= (cachea64 >>
							   *((int *)stab+i)) &
						  *((uint64_t *)mtab + i);
						tmpb64 |= (cacheb64 >>
							   *((int *)stab+i)) &
						  *((uint64_t *)mtab + i);
						i++;
					}
					goto done64;
				}
			}

			if (stopcol <= dstp) break;
			tmp = 0;
			cache = *srcp;
			i = 0;

			while (*((uint64_t *)mtab + i)) {
				tmp |= (cache << 
					*((int *)stab + i)) &
				  *((ggi_pixel *)mtab + i * 2);
				if (!*((int *)stab + i)) 
				  goto doright;
				i++;
			}
			if (!*((int *)stab + i)) 
			  goto doright;
		done:
			*dstp = tmp;
			dstp++;
			srcp++;
			continue;

		doright:
			i++;
			while (*((uint64_t *)mtab + i)) {
				tmp |= (cache >>
					*((int *)stab + i)) &
				  *((ggi_pixel *)mtab + i * 2);
				i++;
			}
			goto done;
		}

		dstp += dstride;
		srcp += sstride;
	}

}

/* Main function hook for 64bit C SWAR -- does some common-case preprocessing 
 * and dispatches to one of the above functions.
 */
int GGI_lin16_crossblit_64bitc(struct ggi_visual *src, int sx, int sy, int w, int h, 
			    struct ggi_visual *dst, int dx, int dy)
{
	LIBGGICLIP_CROSSBLIT(src, sx, sy, w, h, dst, dx, dy);
	PREPARE_FB(dst);

	/* Check if src read buffer is also a blPixelLinearBuffer. */
	if (src->r_frame == NULL) goto fallback;
	if (src->r_frame->layout != blPixelLinearBuffer) goto fallback;

	/* No optimizations yet for reverse endian and other such weirdness */
	if (LIBGGI_PIXFMT(src)->flags) goto fallback;
	if (LIBGGI_PIXFMT(dst)->flags) goto fallback;

	PREPARE_FB(src);

	switch (GT_SIZE(LIBGGI_GT(src))) {
	case 1:
		/* TODO */
	case 2:
		/* TODO */
		goto fallback;
	case 4:
		if (w * h > 15) cb4to16(src, sx, sy, w, h, dst, dx, dy);
		else goto fallback;
		return 0;
	case 8:
		if (w * h > 255) cb8to16(src, sx, sy, w, h, dst, dx, dy);
		else goto fallback;
		return 0;
	case 16:
		if (!dst->w_frame->buffer.plb.pixelformat->stdformat) 
		  goto notsame;
		if (dst->w_frame->buffer.plb.pixelformat->stdformat !=
		    src->r_frame->buffer.plb.pixelformat->stdformat) 
		  goto notsame;
		crossblit_same(src, sx, sy, w, h, dst, dx, dy);
		return 0;
	notsame:
		if (GT_SCHEME(LIBGGI_GT(src)) == GT_TRUECOLOR)
		  cb16to16_64bitc(src, sx, sy, w, h, dst, dx, dy);
		else goto fallback;
		return 0;
	case 24:
		if (GT_SCHEME(LIBGGI_GT(src)) == GT_TRUECOLOR)
		  cb24to16(src, sx, sy, w, h, dst, dx, dy);
		else goto fallback;
		return 0;
	case 32:
		if (GT_SCHEME(LIBGGI_GT(src)) == GT_TRUECOLOR)
		  cb32to16_64bitc(src, sx, sy, w, h, dst, dx, dy);
		else goto fallback;
		return 0;
	default:
		break;
	}
	
 fallback:
	fallback(src, sx, sy, w, h, dst, dx, dy);
	return 0;
}

#endif


#ifdef DO_SWAR_MMX

/* 16 bit to 16 bit crossblitting using MMX SWAR.
 */
static inline void cb16to16_mmx(struct ggi_visual *src, int sx, int sy, int w, int h, 
				struct ggi_visual *dst, int dx, int dy) {
	char tabbuf[8 * 96 + 7], *tab;
	int nl, nr;
	uint16_t *stoprow, *dstp, *srcp;
	int dstride, sstride, i;

	DPRINT_DRAW("linear-16: cb16to16_mmx.\n");

	tab = tabbuf + (8 - (((uint32_t)tabbuf) % 8));

	build_masktab(src, dst, (int *)tab, 
		      (int *)(tab + 256), (int *)(tab + 512), 
		      (int *)tab, 4, 0,
		      (ggi_pixel *)(tab + 8), 32, 4, 3, &nl, &nr);

	for (i = 0; i < 32; i++) {
		*((unsigned long long *)tab + i*2) = *((int *)tab + i*4);
		*((uint16_t *)tab + i*8 + 6) = *((ggi_pixel *)tab + i*4 + 2);
		*((uint16_t *)tab + i*8 + 7) = *((ggi_pixel *)tab + i*4 + 2);
		*((uint16_t *)tab + i*8 + 5) = *((ggi_pixel *)tab + i*4 + 2);   
	}

	dstp = (uint16_t*)((uint8_t*)LIBGGI_CURWRITE(dst) + 
			 dy*(LIBGGI_FB_W_STRIDE(dst)) + dx*2);
	srcp = (uint16_t*)((uint8_t*)LIBGGI_CURREAD(src) + 
			 sy*(LIBGGI_FB_R_STRIDE(src)) + sx*2);
	dstride = LIBGGI_FB_W_STRIDE(dst)/2;
	sstride = LIBGGI_FB_R_STRIDE(src)/2;

	stoprow = dstp + h * dstride;
	dstride -= w;
	sstride -= w;

	while (stoprow > dstp) {
		uint16_t *stopcol;
		
		stopcol = dstp + w;
		while (stopcol > dstp) {
			ggi_pixel tmp, cache;

			if ((stopcol > dstp + 11) && 
			    !((unsigned int)dstp % 8)) {
				while (stopcol > dstp + 11) {
				  __asm__ __volatile__(
				   "movq  (%1), %%mm0\n\t"
                                   "pxor  %%mm5, %%mm5\n\t"
                                   "movq  8(%1), %%mm1\n\t"
                                   "pxor  %%mm6, %%mm6\n\t"
                                   "movq  16(%1), %%mm2\n\t"
                                   "pxor  %%mm7, %%mm7\n\t"
                                   "add   $24, %1\n\t"

                                   ".Lleft%=:"
                                   "cmp   $0, 8(%4)\n\t"
                                   "je    .Lmiddle%=\n\t"

                                   "movq  %%mm0, %%mm3\n\t"
                                   "movq  %%mm1, %%mm4\n\t"
                                   "psllw (%4), %%mm3\n\t"
                                   "psllw (%4), %%mm4\n\t"
                                   "pand  8(%4), %%mm3\n\t"
                                   "pand  8(%4), %%mm4\n\t"
                                   "por   %%mm3, %%mm5\n\t"
                                   "movq  %%mm2, %%mm3\n\t"
                                   "por   %%mm4, %%mm6\n\t"
                                   "psllw (%4), %%mm3\n\t"
                                   "pand  8(%4), %%mm3\n\t"
                                   "cmp   $0, (%4)\n\t"
                                   "por   %%mm3, %%mm7\n\t"
                                   "je    .Lright%=\n\t"
                                   "add   $16, %4\n\t"
                                   "jmp   .Lleft%=\n\t"

                                   ".Lmiddle%=:\n\t"
                                   "cmp   $0, (%4)\n\t"
                                   "je    .Lright%=\n\t"

                                   ".Ldone%=:\n\t"
                                   "movq  %%mm5, (%0)\n\t"
                                   "movq  %%mm6, 8(%0)\n\t"
                                   "movq  %%mm7, 16(%0)\n\t"
                                   "add   $24, %0\n\t"
                                   "jmp   .Lout%=\n\t"

                                   ".Lright%=:\n\t"
                                   "add   $16, %4\n\t"
                                   "cmp   $0, 8(%4)\n\t"
                                   "je    .Ldone%=\n\t"

                                   "movq  %%mm0, %%mm3\n\t"
                                   "movq  %%mm1, %%mm4\n\t"
                                   "psrlw (%4), %%mm3\n\t"
                                   "psrlw (%4), %%mm4\n\t"
                                   "pand  8(%4), %%mm3\n\t"
                                   "pand  8(%4), %%mm4\n\t"
                                   "por   %%mm3, %%mm5\n\t"
                                   "movq  %%mm2, %%mm3\n\t"
                                   "por   %%mm4, %%mm6\n\t"
                                   "psrlw (%4), %%mm3\n\t"
                                   "pand  8(%4), %%mm3\n\t"
                                   "por   %%mm3, %%mm7\n\t"

                                   "jmp   .Lright%=\n\t"

                                   ".Lout%=:\n\t"
                                   "emms\n\t"
				   : "=q" (dstp), "=q" (srcp)
				   : "0" (dstp), "1" (srcp), "q" (tab)
				   : "cc", "memory");
				}
			}

			if (stopcol <= dstp) break;
			tmp = 0;
			cache = *srcp;
			i = 0;

			while (*((ggi_pixel *)tab + i * 4 + 2)) {
				tmp |= (cache << 
					*((unsigned long long *)tab + i*2)) &
				  *((ggi_pixel *)tab + i * 4 + 2);
				if (!*((unsigned long long *)tab + i * 2)) 
				  goto doright;
				i++;
			}
			if (!*((unsigned long long *)tab + i * 2)) 
			  goto doright;
		done:
			*dstp = tmp;
			dstp++;
			srcp++;
			continue;

		doright:
			i++;
			while (*((ggi_pixel *)tab + i * 4 + 2)) {
				tmp |= (cache >>
					*((unsigned long long *)tab + i * 2)) &
				  *((ggi_pixel *)tab + i * 4 + 2);
				i++;
			}
			goto done;
		}
		dstp += dstride;
		srcp += sstride;
	}			
}

/* 32 bit to 16 bit crossblitting using MMX SWAR.
 */
static inline void cb32to16_mmx(struct ggi_visual *src, int sx, int sy, int w, int h, 
				struct ggi_visual *dst, int dx, int dy) {
	/* TODO: Align stuff here. */
	char tabbuf[8 * 96 + 7], *tab;
	int nl, nr;
	uint16_t *stoprow, *dstp;
	uint32_t *srcp;
	int dstride, sstride, i, right;
	
	DPRINT_DRAW("linear-16: cb32to16_mmx.\n");

	tab = tabbuf + (8 - (((uint32_t)tabbuf) % 8));

	build_masktab(src, dst, (int *)tab, 
		      (int *)(tab + 256), (int *)(tab + 512), 
		      (int *)tab, 4, -16,
		      (ggi_pixel *)(tab + 8), 48, 4, 3, &nl, &nr);

	/* Translate the values to sizes acceptable to MMX. */

	right = 0;
	for (i = 0; i < 48; i++) {
		*((unsigned long long *)tab + i*2) = 
			*((int *)tab + i*4) + right;
		if (!*((unsigned long long *)tab + i*2)) right = 16;
		*((uint16_t *)tab + i*8 + 6) = *((ggi_pixel *)tab + i*4 + 2);
		*((uint16_t *)tab + i*8 + 7) = *((ggi_pixel *)tab + i*4 + 2);
		*((uint16_t *)tab + i*8 + 5) = *((ggi_pixel *)tab + i*4 + 2);   
	}

	dstp = (uint16_t*)((uint8_t*)LIBGGI_CURWRITE(dst) + 
			 dy*(LIBGGI_FB_W_STRIDE(dst)) + dx*2);
	srcp = (uint32_t*)((uint8_t*)LIBGGI_CURREAD(src) + 
			 sy*(LIBGGI_FB_R_STRIDE(src)) + sx*4);
	dstride = LIBGGI_FB_W_STRIDE(dst)/2;
	sstride = LIBGGI_FB_R_STRIDE(src)/4;
		
	stoprow = dstp + h * dstride;
	dstride -= w;
	sstride -= w;
	
	while (stoprow > dstp) {
		uint16_t *stopcol;
		
		stopcol = dstp + w;
		while (stopcol > dstp) {
			uint32_t tmp;
			ggi_pixel cache;

			if ((stopcol > dstp + 7) && 
			    !((unsigned int)dstp % 8)) {
				while (stopcol > dstp + 7) {
				  __asm__ __volatile__(

				   "movq  (%1), %%mm0\n\t"
                                   "movq  8(%1), %%mm1\n\t"
                                   "movq  16(%1), %%mm2\n\t"
                                   "movq  24(%1), %%mm3\n\t"
                                   "add   $32, %1\n\t"
                                   "pxor  %%mm7, %%mm7\n\t"
                                   "pxor  %%mm6, %%mm6\n\t"

                                   ".Lleft%=:"
                                   "cmp   $0, 8(%4)\n\t"
                                   "je    .Lmiddle%=\n\t"

                                   "movq  %%mm1, %%mm4\n\t"
                                   "movq  %%mm0, %%mm5\n\t"
                                   "pslld (%4), %%mm4\n\t"
                                   "pslld (%4), %%mm5\n\t"
                                   "psrad $16, %%mm4\n\t"
                                   "psrad $16, %%mm5\n\t"
                                   "packssdw %%mm4, %%mm5\n\t"
                                   "movq  %%mm3, %%mm4\n\t"
                                   "pand  8(%4), %%mm5\n\t"
                                   "por   %%mm5, %%mm6\n\t"
                                   "movq  %%mm2, %%mm5\n\t"
                                   "pslld (%4), %%mm4\n\t"
                                   "pslld (%4), %%mm5\n\t"
                                   "psrad $16, %%mm4\n\t"
                                   "psrad $16, %%mm5\n\t"
                                   "packssdw %%mm4, %%mm5\n\t"
                                   "pand  8(%4), %%mm5\n\t"
                                   "cmp   $0, (%4)\n\t"
                                   "por   %%mm5, %%mm7\n\t"
                                   "je    .Lright%=\n\t"
                                   "add   $16, %4\n\t"
                                   "jmp   .Lleft%=\n\t"

                                   ".Lmiddle%=:\n\t"
                                   "cmp   $0, (%4)\n\t"
                                   "je    .Lright%=\n\t"

                                   ".Ldone%=:\n\t"
                                   "movq  %%mm6, (%0)\n\t"
                                   "movq  %%mm7, 8(%0)\n\t"
                                   "add   $16, %0\n\t"
                                   "jmp   .Lout%=\n\t"

                                   ".Lright%=:\n\t"
                                   "add   $16, %4\n\t"
                                   "cmp   $0, 8(%4)\n\t"
                                   "je    .Ldone%=\n\t"

                                   "movq  %%mm1, %%mm4\n\t"
                                   "movq  %%mm0, %%mm5\n\t"
                                   "psrld (%4), %%mm4\n\t"
                                   "psrld (%4), %%mm5\n\t"
                                   "packssdw %%mm4, %%mm5\n\t"
                                   "movq  %%mm3, %%mm4\n\t"
                                   "pand  8(%4), %%mm5\n\t"
                                   "por   %%mm5, %%mm6\n\t"
                                   "movq  %%mm2, %%mm5\n\t"
                                   "psrld (%4), %%mm4\n\t"
                                   "psrld (%4), %%mm5\n\t"
                                   "packssdw %%mm4, %%mm5\n\t"
                                   "pand  8(%4), %%mm5\n\t"
                                   "por   %%mm5, %%mm7\n\t"
                                   "jmp   .Lright%=\n\t"

                                    ".Lout%=:\n\t"
                                   "emms\n\t"
				   : "=q" (dstp), "=q" (srcp)
				   : "0" (dstp), "1" (srcp), "q" (tab)
				   : "cc", "memory");
				}
			}

			if (stopcol <= dstp) break;
			tmp = 0;
			cache = *srcp;
			i = 0;

			while (*((ggi_pixel *)tab + i*4 + 2)) {
				tmp |= ((cache << 
					 (*((unsigned long long *)tab + i*2))) 
					>> 16) &
				  *((ggi_pixel *)tab + i * 4 + 2);
				if (!*((unsigned long long *)tab + i * 2)) 
				  goto doright;
				i++;
			}
			if (!*((unsigned long long *)tab + i * 2)) 
			  goto doright;
		done:
			*dstp = tmp;
			dstp++;
			srcp++;
			continue;

		doright:
			i++;
			while (*((ggi_pixel *)tab + i * 4 + 2)) {
			  tmp |= ((cache >>
				   *((unsigned long long *)tab + i * 2))) &
			    *((ggi_pixel *)tab + i * 4 + 2);
			  i++;
			}
			goto done;
		}

		dstp += dstride;
		srcp += sstride;
	}

}

/* Main function hook for MMX SWAR -- does some common-case preprocessing 
 * and dispatches to one of the above functions.
 */
int GGI_lin16_crossblit_mmx(struct ggi_visual *src, int sx, int sy, int w, int h, 
			    struct ggi_visual *dst, int dx, int dy)
{
	LIBGGICLIP_CROSSBLIT(src, sx, sy, w, h, dst, dx, dy);
	PREPARE_FB(dst);

	/* Check if src read buffer is also a blPixelLinearBuffer. */
	if (src->r_frame == NULL) goto fallback;
	if (src->r_frame->layout != blPixelLinearBuffer) goto fallback;

	/* No optimizations yet for reverse endian and other such weirdness */
	if (LIBGGI_PIXFMT(src)->flags) goto fallback;
	if (LIBGGI_PIXFMT(dst)->flags) goto fallback;

	PREPARE_FB(src);

	switch (GT_SIZE(LIBGGI_GT(src))) {
	case 1:
		/* TODO */
	case 2:
		/* TODO */
		goto fallback;
	case 4:
		if (w * h > 15) cb4to16(src, sx, sy, w, h, dst, dx, dy);
		else goto fallback;
		return 0;
	case 8:
		if (w * h > 255) cb8to16(src, sx, sy, w, h, dst, dx, dy);
		else goto fallback;
		return 0;
	case 16:
		if (!dst->w_frame->buffer.plb.pixelformat->stdformat) 
		  goto notsame;
		if (dst->w_frame->buffer.plb.pixelformat->stdformat !=
		    src->r_frame->buffer.plb.pixelformat->stdformat) 
		  goto notsame;
		crossblit_same(src, sx, sy, w, h, dst, dx, dy);
		return 0;
	notsame:
		if (GT_SCHEME(LIBGGI_GT(src)) == GT_TRUECOLOR)
		  cb16to16_mmx(src, sx, sy, w, h, dst, dx, dy);
		else goto fallback;
		return 0;
	case 24:
		if (GT_SCHEME(LIBGGI_GT(src)) == GT_TRUECOLOR)
		  cb24to16(src, sx, sy, w, h, dst, dx, dy);
		else goto fallback;
		return 0;
	case 32:
		if (GT_SCHEME(LIBGGI_GT(src)) == GT_TRUECOLOR)
		  cb32to16_mmx(src, sx, sy, w, h, dst, dx, dy);
		else goto fallback;
		return 0;
	default:
		break;
	}
	
 fallback:
	fallback(src, sx, sy, w, h, dst, dx, dy);
	return 0;
}

#endif
