/* $Id: crossblit.c,v 1.3 2002/09/29 19:27:41 skids Exp $
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
#include <ggi/internal/ggi-dl.h>

/* Default fallback to lower GGI primitive functions (slow).
 */
static inline void
fallback(ggi_visual *src, int sx, int sy, int w, int h, 
	 ggi_visual *dst, int dx, int dy)
{
	ggi_pixel cur_src;
	uint16 cur_dst = 0;
	uint16 *dstptr;
	int stride;

	GGIDPRINT_DRAW("linear-16: Fallback to slow crossblit.\n");

	LIBGGIGetPixel(src, sx, sy, &cur_src);
	cur_src++; /* assure safe init */
	
	stride = LIBGGI_FB_W_STRIDE(dst);
	dstptr = (uint16*) ((uint8*)LIBGGI_CURWRITE(dst) + dy*stride + dx*2);

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
		dstptr = (uint16*) ((uint8*)dstptr + stride);
	}
}

/* Blitting between identical visuals... simple memcpy.
 */
static inline void
crossblit_same(ggi_visual *src, int sx, int sy, int w, int h,
	       ggi_visual *dst, int dx, int dy)
{
	uint8 *srcp, *dstp;
	int srcstride = LIBGGI_FB_R_STRIDE(src);
	int dststride = LIBGGI_FB_W_STRIDE(dst);

	GGIDPRINT_DRAW("linear-16: direct memcpy crossblit.\n");
	
	srcp = (uint8*)LIBGGI_CURREAD(src)  + srcstride*sy + sx*2;
	dstp = (uint8*)LIBGGI_CURWRITE(dst) + dststride*dy + dx*2;

	w *= 2;		/* Use width in bytes. */

	for (; h != 0; h--) {
		memcpy(dstp, srcp, w);
		srcp += srcstride;
		dstp += dststride;
	}
}

/* 4 bit to 16 bit crossblitting.
 */
static inline void
cb4to16(ggi_visual *src, int sx, int sy, int w, int h,
	ggi_visual *dst, int dx, int dy)
{
	uint8 *srcp, *dstp;
	int srcstride = LIBGGI_FB_R_STRIDE(src);
	int dststride = LIBGGI_FB_W_STRIDE(dst);
	uint16 conv_tab[16];

	GGIDPRINT_DRAW("linear-16: cb4to16.\n");

	do {
		int i;
		for (i = 0; i < 16; i++) {
			ggi_color col;

			LIBGGIUnmapPixel(src, i, &col);
			conv_tab[i] = LIBGGIMapColor(dst, &col);
		}
	} while (0);

	srcp = (uint8*)LIBGGI_CURREAD(src)  + srcstride*sy + sx/2;
	dstp = (uint8*)LIBGGI_CURWRITE(dst) + dststride*dy + dx*2;

	if ((sx ^ w) & 1) {
		for (; h > 0; h--) {
	    		uint16 *dstpw = (uint16*) dstp;
			uint8  *srcpb = srcp;
			
			int i = w / 8;
			
			/* Unroll manually. */
			switch (w & 0x7) {
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
			uint16 *dstpw = (uint16*) dstp;
			uint8  *srcpb = srcp;
			
			int i = w / 8;

			/* Unroll manually. */			
			switch (w & 0x7) {
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
cb8to16(ggi_visual *src, int sx, int sy, int w, int h,
		  ggi_visual *dst, int dx, int dy)
{
	uint8 *srcp, *dstp;
	int srcstride = LIBGGI_FB_R_STRIDE(src);
	int dststride = LIBGGI_FB_W_STRIDE(dst);
	uint16 conv_tab[256];

	GGIDPRINT_DRAW("linear-16: cb8to16.\n");

	/* Creates the conversion table. 
	 * A bit simplistic approach for 256 colors, perhaps?
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

	for (; h > 0; h--) {
		uint16 *dstpw = (uint16*) dstp;
		uint8  *srcpb = (uint8*)  srcp;
		int i = w / 8;

		/* Unroll manually. */
		switch (w & 0x7) {
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

/* 16 bit to 16 bit crossblitting.
 */
static inline void cb16to16(ggi_visual *src, int sx, int sy, int w, int h, 
			    ggi_visual *dst, int dx, int dy) {
	int i, j, rshift[16], gshift[16], bshift[16];
	ggi_pixel mask[31];
	int nl, nr;
	
	GGIDPRINT_DRAW("linear-16: cb16to16.\n");
	
	/* Create a table of shift and mask operations needed to translate
	 * the source visual pixelformat to that of the destination visual.
	 */
	
	memset(mask, 0, 31 * sizeof(ggi_pixel));
	memset(rshift, 0xff, 16 * sizeof(int));	/* Load with -1s */
	memset(gshift, 0xff, 16 * sizeof(int));	/* Load with -1s */
	memset(bshift, 0xff, 16 * sizeof(int));	/* Load with -1s */
	
	for (i = 0; i < 16; i++) {
		ggi_pixel bm;
		int val;
		
		bm = src->r_frame->buffer.plb.pixelformat->bitmeaning[i];
		val = (bm & 0xff) - 240;
		
		switch(bm & 0xffffff00) {
		case GGI_BM_TYPE_COLOR | GGI_BM_SUB_RED:
			rshift[val] = i;
			break;
		case GGI_BM_TYPE_COLOR | GGI_BM_SUB_GREEN:
			gshift[val] = i;
			break;
		case GGI_BM_TYPE_COLOR | GGI_BM_SUB_BLUE:
			bshift[val] = i;
			break;
		default:
		}
	}
	
	/* Ensure pixel-correct fillout when destination channel is deeper. 
	 */
	for (i=15,j=15; i >= 0; i--) if (rshift[i] < 0) rshift[i]= rshift[j--];
	for (i=15,j=15; i >= 0; i--) if (gshift[i] < 0) gshift[i]= gshift[j--];
	for (i=15,j=15; i >= 0; i--) if (bshift[i] < 0) bshift[i]= bshift[j--];
	
	for (i = 0; i < 16; i++) {
		ggi_pixel bm;
		int val;
	
		bm = dst->w_frame->buffer.plb.pixelformat->bitmeaning[i];
		val = (bm & 0xff) - 240;
		
		switch(bm & 0xffffff00) {
		case GGI_BM_TYPE_COLOR | GGI_BM_SUB_RED:
			mask[rshift[val] + 15 - i] |= 1 << rshift[val];
			break;
		case GGI_BM_TYPE_COLOR | GGI_BM_SUB_GREEN:
			mask[gshift[val] + 15 - i] |= 1 << gshift[val];
			break;
		case GGI_BM_TYPE_COLOR | GGI_BM_SUB_BLUE:
			mask[bshift[val] + 15 - i] |= 1 << bshift[val];
			break;
		default:
		}
	}
	
	/* Precipitate the array of masks and create a corresponding array
	 * of shifts.  Note that rshift, gshift, and bshift areas are reused 
	 * for efficiency -- rshift will afterwards contain right-shift counts,
	 * and bshift will contain left-shift counts.  Masks for left-shift
	 * operations will start at index 16 in the array mask; index 15 will 
	 * contain a mask for zero-shift.
	 */
	for (i = 0, j = 0; i < 15; i++) if (mask[i]) {
		mask[j] = mask[i];
		bshift[j] = 15 - i;
		j++;
	}
	nl = j;
	for (i = 16, j = 16; i < 31; i++) if (mask[i]) {
		mask[j] = mask[i];
		rshift[j - 16] = i - 15;
		j++;
	}
	nr = j - 16;
	
	do {
		uint16 *stoprow, *dstp, *srcp;
		int dstride, sstride;
		
		dstp = (uint16*)((uint8*)LIBGGI_CURWRITE(dst) + 
				 dy*(LIBGGI_FB_W_STRIDE(dst)) + dx*2);
		srcp = (uint16*)((uint8*)LIBGGI_CURREAD(src) + 
				 sy*(LIBGGI_FB_R_STRIDE(src)) + sx*2);
		dstride = LIBGGI_FB_W_STRIDE(dst)/2;
		sstride = LIBGGI_FB_R_STRIDE(src)/2;
		
		stoprow = dstp + h * dstride;
		dstride -= w;
		sstride -= w;
		
		while (stoprow > dstp) {
			uint16 *stopcol;
			
			stopcol = dstp + w;
			while (stopcol > dstp) {
				ggi_pixel tmp, cache;
				
				tmp = 0;
				cache = *srcp;
				switch (nl & 0xf) {
				case 15:
				  tmp |= (cache & mask[14]) << bshift[14];
				case 14:
				  tmp |= (cache & mask[13]) << bshift[13];
				case 13:
				  tmp |= (cache & mask[12]) << bshift[12];
				case 12:
				  tmp |= (cache & mask[11]) << bshift[11];
				case 11:
				  tmp |= (cache & mask[10]) << bshift[10];
				case 10:
				  tmp |= (cache & mask[9]) << bshift[9];
				case 9:
				  tmp |= (cache & mask[8]) << bshift[8];
				case 8:
				  tmp |= (cache & mask[7]) << bshift[7];
				case 7:
				  tmp |= (cache & mask[6]) << bshift[6];
				case 6:
				  tmp |= (cache & mask[5]) << bshift[5];
				case 5:
				  tmp |= (cache & mask[4]) << bshift[4];
				case 4:
				  tmp |= (cache & mask[3]) << bshift[3];
				case 3:
				  tmp |= (cache & mask[2]) << bshift[2];
				case 2:
				  tmp |= (cache & mask[1]) << bshift[1];
				case 1:
				  tmp |= (cache & mask[0]) << bshift[0];
				case 0:
				  break;
				}
				if (mask[15]) tmp |= cache & mask[15];
				switch (nr & 0xf) {
				case 15:
				  tmp |= (cache & mask[30]) >> rshift[14];
				case 14:
				  tmp |= (cache & mask[29]) >> rshift[13];
				case 13:
				  tmp |= (cache & mask[28]) >> rshift[12];
				case 12:
				  tmp |= (cache & mask[27]) >> rshift[11];
				case 11:
				  tmp |= (cache & mask[26]) >> rshift[10];
				case 10:
				  tmp |= (cache & mask[25]) >> rshift[9];
				case 9:
				  tmp |= (cache & mask[24]) >> rshift[8];
				case 8:
				  tmp |= (cache & mask[23]) >> rshift[7];
				case 7:
				  tmp |= (cache & mask[22]) >> rshift[6];
				case 6:
				  tmp |= (cache & mask[21]) >> rshift[5];
				case 5:
				  tmp |= (cache & mask[20]) >> rshift[4];
				case 4:
				  tmp |= (cache & mask[19]) >> rshift[3];
				case 3:
				  tmp |= (cache & mask[18]) >> rshift[2];
				case 2:
				  tmp |= (cache & mask[17]) >> rshift[1];
				case 1:
				  tmp |= (cache & mask[16]) >> rshift[0];
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
	} while (0);
	return;
}

/* 24 bit to 16 bit crossblitting.
 */
static inline void cb24to16(ggi_visual *src, int sx, int sy, int w, int h, 
			    ggi_visual *dst, int dx, int dy) {
	int i, j, rshift[16], gshift[16], bshift[16];
	ggi_pixel mask[39];
	int nl, nr;
	
	GGIDPRINT_DRAW("linear-16: cb24to16.\n");
	
	/* Create a table of shift and mask operations needed to translate
	 * the source visual pixelformat to that of the destination visual.
	 */
	memset(mask, 0, 39 * sizeof(ggi_pixel));
	memset(rshift, 0xff, 16 * sizeof(int));	/* Load with -1s */
	memset(gshift, 0xff, 16 * sizeof(int));	/* Load with -1s */
	memset(bshift, 0xff, 16 * sizeof(int));	/* Load with -1s */
	
	for (i = 0; i < 24; i++) {
		ggi_pixel bm;
		int val;
		
		bm = src->r_frame->buffer.plb.pixelformat->bitmeaning[i];
		val = (bm & 0xff) - 240;
		if (val < 0) continue;
		
		switch(bm & 0xffffff00) {
		case GGI_BM_TYPE_COLOR | GGI_BM_SUB_RED:
			rshift[val] = i;
			break;
		case GGI_BM_TYPE_COLOR | GGI_BM_SUB_GREEN:
			gshift[val] = i;
			break;
		case GGI_BM_TYPE_COLOR | GGI_BM_SUB_BLUE:
			bshift[val] = i;
			break;
		default:
		}
	}
	
	/* Ensure pixel-correct fillout when destination channel is deeper. 
	 */
	for (i=15,j=15; i >= 0; i--) if (rshift[i] < 0) rshift[i]= rshift[j--];
	for (i=15,j=15; i >= 0; i--) if (gshift[i] < 0) gshift[i]= gshift[j--];
	for (i=15,j=15; i >= 0; i--) if (bshift[i] < 0) bshift[i]= bshift[j--];

	for (i = 0; i < 16; i++) {
		ggi_pixel bm;
		int val;

		bm = dst->w_frame->buffer.plb.pixelformat->bitmeaning[i];
		val = (bm & 0xff) - 240;
		
		switch(bm & 0xffffff00) {
		case GGI_BM_TYPE_COLOR | GGI_BM_SUB_RED:
			mask[rshift[val] + 15 - i] |= 1 << rshift[val];
			break;
		case GGI_BM_TYPE_COLOR | GGI_BM_SUB_GREEN:
			mask[gshift[val] + 15 - i] |= 1 << gshift[val];
			break;
		case GGI_BM_TYPE_COLOR | GGI_BM_SUB_BLUE:
			mask[bshift[val] + 15 - i] |= 1 << bshift[val];
			break;
		default:
		}
	}
	
	
	/* Precipitate the array of masks and create a corresponding array
	 * of shifts.  Note that rshift, gshift, and bshift areas are reused
	 * for efficiency -- rshift will afterwards contain right-shift counts,
	 * and bshift,gshift will contain left-shift counts.  Masks for 
	 * left-shift operations will start at index 16 in the array mask; 
	 * index 15 will contain a mask for zero-shift.
	 */
	for (i = 0, j = 0; i < 15; i++) if (mask[i]) {
		mask[j] = mask[i];
		bshift[j] = 15 - i;
		j++;
	}
	nl = j;
	for (i = 16, j = 16; i < 39; i++) if (mask[i]) {
		mask[j] = mask[i];
		if (j > 31) gshift[j - 32] = i - 15;
		else rshift[j - 16] = i - 15;
		j++;
	}  
	nr = j - 16;

	do {
		uint16 *stoprow, *dstp;
		uint8 *srcp;
		int dstride, sstride;
		
		dstp = (uint16*)((uint8*)LIBGGI_CURWRITE(dst) + 
				 dy*(LIBGGI_FB_W_STRIDE(dst)) + dx*2);
		srcp = (uint8*)LIBGGI_CURREAD(src) + 
		  sy*(LIBGGI_FB_R_STRIDE(src)) + sx*3;
		dstride = LIBGGI_FB_W_STRIDE(dst)/2;
		sstride = LIBGGI_FB_R_STRIDE(src);
		
		stoprow = dstp + h * dstride;
		dstride -= w;
		sstride -= w * 3;
		
		while (stoprow > dstp) {
			uint16 *stopcol;
			
			stopcol = dstp + w;
			while (stopcol > dstp) {
				ggi_pixel tmp, cache;
				
				tmp = 0;
				cache = ((ggi_pixel)*(srcp + 2) << 16) | 
				  ((ggi_pixel)*(srcp + 1) << 8) | 
				  (ggi_pixel)*srcp;
				switch (nl & 0xf) {
				case 15:
				  tmp |= (cache & mask[14]) << bshift[14];
				case 14:
				  tmp |= (cache & mask[13]) << bshift[13];
				case 13:
				  tmp |= (cache & mask[12]) << bshift[12];
				case 12:
				  tmp |= (cache & mask[11]) << bshift[11];
				case 11:
				  tmp |= (cache & mask[10]) << bshift[10];
				case 10:
				  tmp |= (cache & mask[9]) << bshift[9];
				case 9:
				  tmp |= (cache & mask[8]) << bshift[8];
				case 8:
				  tmp |= (cache & mask[7]) << bshift[7];
				case 7:
				  tmp |= (cache & mask[6]) << bshift[6];
				case 6:
				  tmp |= (cache & mask[5]) << bshift[5];
				case 5:
				  tmp |= (cache & mask[4]) << bshift[4];
				case 4:
				  tmp |= (cache & mask[3]) << bshift[3];
				case 3:
				  tmp |= (cache & mask[2]) << bshift[2];
				case 2:
				  tmp |= (cache & mask[1]) << bshift[1];
				case 1:
				  tmp |= (cache & mask[0]) << bshift[0];
				case 0:
				  break;
				}
				if (mask[15]) tmp |= cache & mask[15];
				switch (nr & 0x1f) {
				case 23:
				  tmp |= (cache & mask[38]) >> gshift[6];
				case 22:
				  tmp |= (cache & mask[37]) >> gshift[5];
				case 21:
				  tmp |= (cache & mask[36]) >> gshift[4];
				case 20:
				  tmp |= (cache & mask[35]) >> gshift[3];
				case 19:
				  tmp |= (cache & mask[34]) >> gshift[2];
				case 18:
				  tmp |= (cache & mask[33]) >> gshift[1];
				case 17:
				  tmp |= (cache & mask[32]) >> gshift[0];
				case 16:
				  tmp |= (cache & mask[31]) >> rshift[15];
				case 15:
				  tmp |= (cache & mask[30]) >> rshift[14];
				case 14:
				  tmp |= (cache & mask[29]) >> rshift[13];
				case 13:
				  tmp |= (cache & mask[28]) >> rshift[12];
				case 12:
				  tmp |= (cache & mask[27]) >> rshift[11];
				case 11:
				  tmp |= (cache & mask[26]) >> rshift[10];
				case 10:
				  tmp |= (cache & mask[25]) >> rshift[9];
				case 9:
				  tmp |= (cache & mask[24]) >> rshift[8];
				case 8:
				  tmp |= (cache & mask[23]) >> rshift[7];
				case 7:
				  tmp |= (cache & mask[22]) >> rshift[6];
				case 6:
				  tmp |= (cache & mask[21]) >> rshift[5];
				case 5:
				  tmp |= (cache & mask[20]) >> rshift[4];
				case 4:
				  tmp |= (cache & mask[19]) >> rshift[3];
				case 3:
				  tmp |= (cache & mask[18]) >> rshift[2];
				case 2:
				  tmp |= (cache & mask[17]) >> rshift[1];
				case 1:
				  tmp |= (cache & mask[16]) >> rshift[0];
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
	} while (0);
	return;
}

/* 32 bit to 16 bit crossblitting.
 */
static inline void cb32to16(ggi_visual *src, int sx, int sy, int w, int h, 
			    ggi_visual *dst, int dx, int dy) {
	int i, j, rshift[16], gshift[16], bshift[16];
	ggi_pixel mask[47];
	int nl, nr;
	
	GGIDPRINT_DRAW("linear-16: cb32to16.\n");
	
	/* Create a table of shift and mask operations needed to translate
	 * the source visual pixelformat to that of the destination visual.
	 */
	
	memset(mask, 0, 47 * sizeof(ggi_pixel));
	memset(rshift, 0xff, 16 * sizeof(int));	/* Load with -1s */
	memset(gshift, 0xff, 16 * sizeof(int));	/* Load with -1s */
	memset(bshift, 0xff, 16 * sizeof(int));	/* Load with -1s */
	
	for (i = 0; i < 32; i++) {
		ggi_pixel bm;
		int val;
		
		bm = src->r_frame->buffer.plb.pixelformat->bitmeaning[i];
		val = (bm & 0xff) - 240;
		if (val < 0) continue;
		
		switch(bm & 0xffffff00) {
		case GGI_BM_TYPE_COLOR | GGI_BM_SUB_RED:
			rshift[val] = i;
			break;
		case GGI_BM_TYPE_COLOR | GGI_BM_SUB_GREEN:
			gshift[val] = i;
			break;
		case GGI_BM_TYPE_COLOR | GGI_BM_SUB_BLUE:
			bshift[val] = i;
			break;
		default:
		}
	}
	
	/* Ensure pixel-correct fillout when destination channel is deeper. 
	 */
	for (i=15,j=15; i >= 0; i--) if (rshift[i] < 0) rshift[i]= rshift[j--];
	for (i=15,j=15; i >= 0; i--) if (gshift[i] < 0) gshift[i]= gshift[j--];
	for (i=15,j=15; i >= 0; i--) if (bshift[i] < 0) bshift[i]= bshift[j--];
	
	for (i = 0; i < 16; i++) {
		ggi_pixel bm;
		int val;
		
		bm = dst->w_frame->buffer.plb.pixelformat->bitmeaning[i];
		val = (bm & 0xff) - 240;
		
		switch(bm & 0xffffff00) {
		case GGI_BM_TYPE_COLOR | GGI_BM_SUB_RED:
			mask[rshift[val] + 15 - i] |= 1 << rshift[val];
			break;
		case GGI_BM_TYPE_COLOR | GGI_BM_SUB_GREEN:
			mask[gshift[val] + 15 - i] |= 1 << gshift[val];
			break;
		case GGI_BM_TYPE_COLOR | GGI_BM_SUB_BLUE:
			mask[bshift[val] + 15 - i] |= 1 << bshift[val];
			break;
		default:
		}
	}
	
	/* Precipitate the array of masks and create a corresponding array
	 * of shifts.  Note that rshift, gshift, and bshift areas are reused 
	 * for efficiency -- rshift will afterwards contain right-shift counts,
	 * and bshift,gshift will contain left-shift counts.  Masks for 
	 * left-shift operations will start at index 16 in the array mask; 
	 * index 15 will contain a mask for zero-shift.
	 */
	for (i = 0, j = 0; i < 15; i++) if (mask[i]) {
	  mask[j] = mask[i];
	  bshift[j] = 15 - i;
	  j++;
	}
	nl = j;
	for (i = 16, j = 16; i < 47; i++) if (mask[i]) {
	  mask[j] = mask[i];
	  if (j > 31) gshift[j - 32] = i - 15;
	  else rshift[j - 16] = i - 15;
	  j++;
	}  
	nr = j - 16;
	
	do {
		uint16 *stoprow, *dstp;
		uint32 *srcp;
		int dstride, sstride;
		
		dstp = (uint16*)((uint8*)LIBGGI_CURWRITE(dst) + 
				 dy*(LIBGGI_FB_W_STRIDE(dst)) + dx*2);
		srcp = (uint32*)((uint8*)LIBGGI_CURREAD(src) + 
				 sy*(LIBGGI_FB_R_STRIDE(src)) + sx*4);
		dstride = LIBGGI_FB_W_STRIDE(dst)/2;
		sstride = LIBGGI_FB_R_STRIDE(src)/4;
		
		stoprow = dstp + h * dstride;
		dstride -= w;
		sstride -= w;
		
		while (stoprow > dstp) {
			uint16 *stopcol;
			
			stopcol = dstp + w;
			while (stopcol > dstp) {
				ggi_pixel tmp, cache;
				
				tmp = 0;
				cache = *srcp;
			  switch (nl & 0xf) {
			  case 15:
			    tmp |= (cache & mask[14]) << bshift[14];
			  case 14:
			    tmp |= (cache & mask[13]) << bshift[13];
			  case 13:
			    tmp |= (cache & mask[12]) << bshift[12];
			  case 12:
			    tmp |= (cache & mask[11]) << bshift[11];
			  case 11:
			    tmp |= (cache & mask[10]) << bshift[10];
			  case 10:
			    tmp |= (cache & mask[9]) << bshift[9];
			  case 9:
			    tmp |= (cache & mask[8]) << bshift[8];
			  case 8:
			    tmp |= (cache & mask[7]) << bshift[7];
			  case 7:
			    tmp |= (cache & mask[6]) << bshift[6];
			  case 6:
			    tmp |= (cache & mask[5]) << bshift[5];
			  case 5:
			    tmp |= (cache & mask[4]) << bshift[4];
			  case 4:
			    tmp |= (cache & mask[3]) << bshift[3];
			  case 3:
			    tmp |= (cache & mask[2]) << bshift[2];
			  case 2:
			    tmp |= (cache & mask[1]) << bshift[1];
			  case 1:
			    tmp |= (cache & mask[0]) << bshift[0];
			  case 0:
			    break;
			  }
			  if (mask[15]) { tmp |= cache & mask[15]; }
			  switch (nr & 0x1f) {
			  case 31:
			    tmp |= (cache & mask[46]) >> gshift[14];
			  case 30:
			    tmp |= (cache & mask[45]) >> gshift[13];
			  case 29:
			    tmp |= (cache & mask[44]) >> gshift[12];
			  case 28:
			    tmp |= (cache & mask[43]) >> gshift[11];
			  case 27:
			    tmp |= (cache & mask[42]) >> gshift[10];
			  case 26:
			    tmp |= (cache & mask[41]) >> gshift[9];
			  case 25:
			    tmp |= (cache & mask[40]) >> gshift[8];
			  case 24:
			    tmp |= (cache & mask[39]) >> gshift[7];
			  case 23:
			    tmp |= (cache & mask[38]) >> gshift[6];
			  case 22:
			    tmp |= (cache & mask[37]) >> gshift[5];
			  case 21:
			    tmp |= (cache & mask[36]) >> gshift[4];
			  case 20:
			    tmp |= (cache & mask[35]) >> gshift[3];
			  case 19:
			    tmp |= (cache & mask[34]) >> gshift[2];
			  case 18:
			    tmp |= (cache & mask[33]) >> gshift[1];
			  case 17:
			    tmp |= (cache & mask[32]) >> gshift[0];
			  case 16:
			    tmp |= (cache & mask[31]) >> rshift[15];
			  case 15:
			    tmp |= (cache & mask[30]) >> rshift[14];
			  case 14:
			    tmp |= (cache & mask[29]) >> rshift[13];
			  case 13:
			    tmp |= (cache & mask[28]) >> rshift[12];
			  case 12:
			    tmp |= (cache & mask[27]) >> rshift[11];
			  case 11:
			    tmp |= (cache & mask[26]) >> rshift[10];
			  case 10:
			    tmp |= (cache & mask[25]) >> rshift[9];
			  case 9:
			    tmp |= (cache & mask[24]) >> rshift[8];
			  case 8:
			    tmp |= (cache & mask[23]) >> rshift[7];
			  case 7:
			    tmp |= (cache & mask[22]) >> rshift[6];
			  case 6:
			    tmp |= (cache & mask[21]) >> rshift[5];
			  case 5:
			    tmp |= (cache & mask[20]) >> rshift[4];
			  case 4:
			    tmp |= (cache & mask[19]) >> rshift[3];
			  case 3:
			    tmp |= (cache & mask[18]) >> rshift[2];
			  case 2:
			    tmp |= (cache & mask[17]) >> rshift[1];
			  case 1:
			    tmp |= (cache & mask[16]) >> rshift[0];
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
	} while (0);
	return;
}

/* Main function hook -- does some common-case preprocessing and
 * dispatches to one of the above functions.
 */
int GGI_lin16_crossblit(ggi_visual *src, int sx, int sy, int w, int h, 
			ggi_visual *dst, int dx, int dy)
{
	LIBGGICLIP_COPYBOX(dst,sx,sy,w,h,dx,dy);
	PREPARE_FB(dst);

	/* Check if src read buffer is also a blPixelLinearBuffer. */
	if (src->r_frame == NULL) goto fallback;
	if (src->r_frame->layout != blPixelLinearBuffer) goto fallback;

	/* No optimizations yet for reverse endian and other such weirdness */
	if (LIBGGI_PIXFMT(src)->flags) goto fallback;

	PREPARE_FB(src);

	switch (GT_SIZE(LIBGGI_MODE(src)->graphtype)) {
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
		if (GT_SCHEME(LIBGGI_MODE(src)->graphtype) == GT_TRUECOLOR)
		  cb16to16(src, sx, sy, w, h, dst, dx, dy);
		else goto fallback;
		return 0;
	case 24:
		if (GT_SCHEME(LIBGGI_MODE(src)->graphtype) == GT_TRUECOLOR)
		  cb24to16(src, sx, sy, w, h, dst, dx, dy);
		else goto fallback;
		return 0;
	case 32:
		if (GT_SCHEME(LIBGGI_MODE(src)->graphtype) == GT_TRUECOLOR)
		  cb32to16(src, sx, sy, w, h, dst, dx, dy);
		else goto fallback;
		return 0;
	default:
	}
	
 fallback:
	fallback(src, sx, sy, w, h, dst, dx, dy);
	return 0;
}

