/* $Id: trle.c,v 1.1 2009/10/10 08:51:12 pekberg Exp $
******************************************************************************

   display-vnc: RFB trle encoding

   Copyright (C) 2006-2009 Peter Rosin	[peda@lysator.liu.se]

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

#include "config.h"

#include <stdlib.h>
#ifdef HAVE_STRING_H
#include <string.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include <ggi/display/vnc.h>
#include <ggi/internal/ggi_debug.h>
#include "rect.h"
#include "encoding.h"
#include "common.h"

union palette_union {
	uint8_t p8[128];
	uint16_t p16[128];
	uint32_t p32[128];
};

struct trle_ctx_t {
	int palette_size;
	union palette_union palette;
};

/* subencodings */
#define TRLE_RAW              0
#define TRLE_SOLID            1
#define TRLE_PACKED_1         2
#define TRLE_PACKED_2_START   3
#define TRLE_PACKED_2_END     4
#define TRLE_PACKED_4_START   5
#define TRLE_PACKED_4_END    16
#define TRLE_PACKED_BASE      0
#define TRLE_PACKED_START     2
#define TRLE_PACKED_END      16
#define TRLE_PACKED_MEM     127
#define TRLE_RLE            128
#define TRLE_PRLE_BASE      128
#define TRLE_PRLE_MEM       129
#define TRLE_PRLE_START     130
#define TRLE_PRLE_END       255

typedef void (tile_func)(struct trle_ctx_t *ctx, uint8_t **buf, uint8_t *src,
	int xs, int ys, int stride, int bpp);

static uint8_t
select_subencoding(struct trle_ctx_t *ctx, int xs, int ys, int cbpp,
	int colors, int single, int multi)
{
	int best;
	int bytes;
	int subencoding;

	/* solid */
	if (colors == 1) {
		best = cbpp;
		return TRLE_SOLID;
	}

	/* raw */
	best = xs * ys * cbpp;
	subencoding = TRLE_RAW;

	/* palette rle (remembered palette) */
	if (2 <= ctx->palette_size && ctx->palette_size <= 127) {
		bytes = single + 9 * multi / 4;
		if (bytes < best) {
			best = bytes;
			subencoding = TRLE_PRLE_MEM;
		}
	}

	/* palette rle */
	if (2 <= colors && colors <= 127) {
		bytes = cbpp * colors + single + 9 * multi / 4;
		if (bytes < best) {
			best = bytes;
			subencoding = TRLE_PRLE_BASE + colors;
		}
	}

	/* plain rle */
	bytes = (1 + cbpp) * single + cbpp * multi + 9 * multi / 4;
	if (bytes < best) {
		best = bytes;
		subencoding = TRLE_RLE;
	}

	/* packed palette (remembered palette) */
	bytes = best + 1;
	if (ctx->palette_size == 2)
		bytes = (xs + 7) / 8 * ys;
	else if (ctx->palette_size == 3 || ctx->palette_size == 4)
		bytes = (xs + 3) / 4 * ys;
	else if (5 <= ctx->palette_size && ctx->palette_size <= 16)
		bytes = (xs + 1) / 2 * ys;
	if (bytes < best) {
		best = bytes;
		subencoding = TRLE_PACKED_MEM;
	}

	/* packed palette */
	bytes = best + 1;
	if (colors == 2)
		bytes = cbpp * colors + (xs + 7) / 8 * ys;
	else if (colors == 3 || colors == 4)
		bytes = cbpp * colors + (xs + 3) / 4 * ys;
	else if (5 <= colors && colors <= 16)
		bytes = cbpp * colors + (xs + 1) / 2 * ys;
	if (bytes < best) {
		best = bytes;
		subencoding = TRLE_PACKED_BASE + colors;
	}

	return subencoding;
}

static inline uint8_t
scan_8(struct trle_ctx_t *ctx, uint8_t *src,
	int xs, int ys, int stride)
{
	uint8_t subencoding;
	int x, y;
	uint8_t last = *src;
	uint8_t here;
	int rl = 0;
	int single = 0;
	int multi = 0;
	int c;
	uint8_t new_palette[128];
	uint8_t *palette;
	int colors;

	if (ctx->palette_size) {
		c = palette_match_8(
			ctx->palette.p8, ctx->palette_size, *src);
		if (c == ctx->palette_size)
			/* old palette not matching, erase it */
			ctx->palette_size = 0;
		palette = ctx->palette_size ? new_palette : ctx->palette.p8;
	}
	else
		palette = ctx->palette.p8;
	palette[0] = *src;
	colors = 1;

	for (y = 0; y < ys; ++y) {
		for (x = 0; x < xs; ++x) {
			here = *src++;
			if (last == here) {
				++rl;
				continue;
			}
			last = here;
			if (rl == 1)
				++single;
			else {
				++multi;
				rl = 1;
			}
			if (colors == 128)
				continue;
			c = palette_match_8(palette, colors, here);
			if (c == colors)
				palette[colors++] = here;
			if (!ctx->palette_size)
				/* no old palette */
				continue;
			c = palette_match_8(
				ctx->palette.p8, ctx->palette_size, here);
			if (c == ctx->palette_size) {
				/* old palette not matching, replace it */
				memcpy(ctx->palette.p8, new_palette,
					sizeof(here) * colors);
				palette = ctx->palette.p8;
				ctx->palette_size = 0;
			}
		}
		src += stride;
	}
	if (rl == 1)
		++single;
	else
		++multi;

	subencoding = select_subencoding(ctx,
		xs, ys, 1, colors, single, multi);

	if (subencoding == TRLE_PRLE_MEM || subencoding == TRLE_PACKED_MEM)
		return subencoding;
	if (ctx->palette_size)
		memcpy(ctx->palette.p8, new_palette, sizeof(here) * colors);
	ctx->palette_size = colors;
	return subencoding;
}

static inline uint8_t
scan_16(struct trle_ctx_t *ctx, uint16_t *src,
	int xs, int ys, int stride)
{
	uint8_t subencoding;
	int x, y;
	uint16_t last = *src;
	uint16_t here;
	int rl = 0;
	int single = 0;
	int multi = 0;
	int c;
	uint16_t new_palette[128];
	uint16_t *palette;
	int colors;

	if (ctx->palette_size) {
		c = palette_match_16(
			ctx->palette.p16, ctx->palette_size, *src);
		if (c == ctx->palette_size)
			/* old palette not matching, erase it */
			ctx->palette_size = 0;
		palette = ctx->palette_size ? new_palette : ctx->palette.p16;
	}
	else
		palette = ctx->palette.p16;
	palette[0] = *src;
	colors = 1;

	for (y = 0; y < ys; ++y) {
		for (x = 0; x < xs; ++x) {
			here = *src++;
			if (last == here) {
				++rl;
				continue;
			}
			last = here;
			if (rl == 1)
				++single;
			else {
				++multi;
				rl = 1;
			}
			if (colors == 128)
				continue;
			c = palette_match_16(palette, colors, here);
			if (c == colors)
				palette[colors++] = here;
			if (!ctx->palette_size)
				/* no old palette */
				continue;
			c = palette_match_16(
				ctx->palette.p16, ctx->palette_size, here);
			if (c == ctx->palette_size) {
				/* old palette not matching, replace it */
				memcpy(ctx->palette.p16, new_palette,
					sizeof(here) * colors);
				palette = ctx->palette.p16;
				ctx->palette_size = 0;
			}
		}
		src += stride;
	}
	if (rl == 1)
		++single;
	else
		++multi;

	subencoding = select_subencoding(ctx,
		xs, ys, 2, colors, single, multi);

	if (subencoding == TRLE_PRLE_MEM || subencoding == TRLE_PACKED_MEM)
		return subencoding;
	if (ctx->palette_size)
		memcpy(ctx->palette.p16, new_palette, sizeof(here) * colors);
	ctx->palette_size = colors;
	return subencoding;
}

static inline uint8_t
scan_32(struct trle_ctx_t *ctx, uint32_t *src,
	int xs, int ys, int stride, int cbpp)
{
	uint8_t subencoding;
	int x, y;
	uint32_t last = *src;
	uint32_t here;
	int rl = 0;
	int single = 0;
	int multi = 0;
	int c;
	uint32_t new_palette[128];
	uint32_t *palette;
	int colors;

	if (ctx->palette_size) {
		c = palette_match_32(
			ctx->palette.p32, ctx->palette_size, *src);
		if (c == ctx->palette_size)
			/* old palette not matching, erase it */
			ctx->palette_size = 0;
		palette = ctx->palette_size ? new_palette : ctx->palette.p32;
	}
	else
		palette = ctx->palette.p32;
	palette[0] = *src;
	colors = 1;

	for (y = 0; y < ys; ++y) {
		for (x = 0; x < xs; ++x) {
			here = *src++;
			if (last == here) {
				++rl;
				continue;
			}
			last = here;
			if (rl == 1)
				++single;
			else {
				++multi;
				rl = 1;
			}
			if (colors == 128)
				continue;
			c = palette_match_32(palette, colors, here);
			if (c == colors)
				palette[colors++] = here;
			if (!ctx->palette_size)
				/* no old palette */
				continue;
			c = palette_match_32(
				ctx->palette.p32, ctx->palette_size, here);
			if (c == ctx->palette_size) {
				/* old palette not matching, replace it */
				memcpy(ctx->palette.p32, new_palette,
					sizeof(here) * colors);
				palette = ctx->palette.p32;
				ctx->palette_size = 0;
			}
		}
		src += stride;
	}
	if (rl == 1)
		++single;
	else
		++multi;

	subencoding = select_subencoding(ctx,
		xs, ys, cbpp, colors, single, multi);

	if (subencoding == TRLE_PRLE_MEM || subencoding == TRLE_PACKED_MEM)
		return subencoding;
	if (ctx->palette_size)
		memcpy(ctx->palette.p32, new_palette, sizeof(here) * colors);
	ctx->palette_size = colors;
	return subencoding;
}

static inline uint8_t *
insert_rl(uint8_t *dst, int rl)
{
	while (rl > 254) {
		*dst++ = 255;
		rl -= 255;
	}
	*dst++ = rl;
	return dst;
}

static inline uint8_t *
plain_rle_8(uint8_t *dst, uint8_t *src,
	int xs, int ys, int stride)
{
	int rl;
	uint8_t here;
	uint8_t last;
	int x, y;

	rl = -1;
	last = *src;
	for (y = 0; y < ys; ++y) {
		for (x = 0; x < xs; ++x) {
			here = *src++;
			if (last == here) {
				++rl;
				continue;
			}
			*dst++ = last;
			dst = insert_rl(dst, rl);
			last = here;
			rl = 0;
		}
		src += stride;
	}
	*dst++ = last;
	return insert_rl(dst, rl);
}

typedef uint8_t *(insert_16_t)(uint8_t *dst, uint16_t pixel);

static inline uint8_t *
plain_rle_16(uint8_t *dst, uint16_t *src,
	int xs, int ys, int stride, insert_16_t *insert)
{
	int rl;
	uint16_t here;
	uint16_t last;
	int x, y;

	rl = -1;
	last = *src;
	for (y = 0; y < ys; ++y) {
		for (x = 0; x < xs; ++x) {
			here = *src++;
			if (last == here) {
				++rl;
				continue;
			}
			dst = insert(dst, last);
			dst = insert_rl(dst, rl);
			last = here;
			rl = 0;
		}
		src += stride;
	}
	dst = insert(dst, last);
	return insert_rl(dst, rl);
}

typedef uint8_t *(insert_32_t)(uint8_t *dst, uint32_t pixel);

static inline uint8_t *
plain_rle_32(uint8_t *dst, uint32_t *src,
	int xs, int ys, int stride, insert_32_t *insert)
{
	int rl;
	uint32_t here;
	uint32_t last;
	int x, y;

	rl = -1;
	last = *src;
	for (y = 0; y < ys; ++y) {
		for (x = 0; x < xs; ++x) {
			here = *src++;
			if (last == here) {
				++rl;
				continue;
			}
			dst = insert(dst, last);
			dst = insert_rl(dst, rl);
			last = here;
			rl = 0;
		}
		src += stride;
	}
	dst = insert(dst, last);
	return insert_rl(dst, rl);
}

static inline uint8_t *
insert_palrle_rl_8(uint8_t *dst,
	uint8_t *palette, int colors, uint8_t color, int rl)
{
	uint8_t c = palette_match_8(palette, colors, color);
	if (!rl)
		*dst++ = c;
	else {
		*dst++ = 128 + c;
		dst = insert_rl(dst, rl);
	}
	return dst;
}

static inline uint8_t *
insert_palrle_rl_16(uint8_t *dst,
	uint16_t *palette, int colors, uint16_t color, int rl)
{
	uint8_t c = palette_match_16(palette, colors, color);
	if (!rl)
		*dst++ = c;
	else {
		*dst++ = 128 + c;
		dst = insert_rl(dst, rl);
	}
	return dst;
}

static inline uint8_t *
insert_palrle_rl_32(uint8_t *dst,
	uint32_t *palette, int colors, uint32_t color, int rl)
{
	uint8_t c = palette_match_32(palette, colors, color);
	if (!rl)
		*dst++ = c;
	else {
		*dst++ = 128 + c;
		dst = insert_rl(dst, rl);
	}
	return dst;
}

static inline uint8_t *
palette_rle_8(uint8_t *dst, uint8_t *src,
	int xs, int ys, int stride, uint8_t *palette, int colors)
{
	int x, y;
	uint8_t last;
	uint8_t here;
	int rl;

	rl = -1;
	last = *src;
	for (y = 0; y < ys; ++y) {
		for (x = 0; x < xs; ++x) {
			here = *src++;
			if (last == here) {
				++rl;
				continue;
			}
			dst = insert_palrle_rl_8(
				dst, palette, colors, last, rl);
			last = here;
			rl = 0;
		}
		src += stride;
	}
	return insert_palrle_rl_8(dst, palette, colors, last, rl);
}

static inline uint8_t *
palette_rle_16(uint8_t *dst, uint16_t *src,
	int xs, int ys, int stride, uint16_t *palette, int colors)
{
	int x, y;
	uint16_t last;
	uint16_t here;
	int rl;

	rl = -1;
	last = *src;
	for (y = 0; y < ys; ++y) {
		for (x = 0; x < xs; ++x) {
			here = *src++;
			if (last == here) {
				++rl;
				continue;
			}
			dst = insert_palrle_rl_16(
				dst, palette, colors, last, rl);
			last = here;
			rl = 0;
		}
		src += stride;
	}
	return insert_palrle_rl_16(dst, palette, colors, last, rl);
}

static inline uint8_t *
palette_rle_32(uint8_t *dst, uint32_t *src,
	int xs, int ys, int stride, uint32_t *palette, int colors)
{
	int x, y;
	uint32_t last;
	uint32_t here;
	int rl;

	rl = -1;
	last = *src;
	for (y = 0; y < ys; ++y) {
		for (x = 0; x < xs; ++x) {
			here = *src++;
			if (last == here) {
				++rl;
				continue;
			}
			dst = insert_palrle_rl_32(
				dst, palette, colors, last, rl);
			last = here;
			rl = 0;
		}
		src += stride;
	}
	return insert_palrle_rl_32(dst, palette, colors, last, rl);
}

static inline uint8_t *
packed_palette_8(uint8_t *dst, uint8_t *src,
	int xs, int ys, int stride, uint8_t *palette, int bits)
{
	int x, y;

	for (y = 0; y < ys; ++y) {
		int pel = 8 - bits;
		*dst = 0;
		for (x = 0; x < xs; ++x) {
			*dst |= palette_match_8(palette, 16, *src++) << pel;
			pel -= bits;
			if (pel < 0) {
				pel = 8 - bits;
				*++dst = 0;
			}
		}
		src += stride;
		if (pel != 8 - bits)
			++dst;
	}

	return dst;
}

static inline uint8_t *
packed_palette_16(uint8_t *dst, uint16_t *src,
	int xs, int ys, int stride, uint16_t *palette, int bits)
{
	int x, y;

	for (y = 0; y < ys; ++y) {
		int pel = 8 - bits;
		*dst = 0;
		for (x = 0; x < xs; ++x) {
			*dst |= palette_match_16(palette, 16, *src++) << pel;
			pel -= bits;
			if (pel < 0) {
				pel = 8 - bits;
				*++dst = 0;
			}
		}
		src += stride;
		if (pel != 8 - bits)
			++dst;
	}

	return dst;
}

static inline uint8_t *
packed_palette_32(uint8_t *dst, uint32_t *src,
	int xs, int ys, int stride, uint32_t *palette, int bits)
{
	int x, y;

	for (y = 0; y < ys; ++y) {
		int pel = 8 - bits;
		*dst = 0;
		for (x = 0; x < xs; ++x) {
			*dst |= palette_match_32(palette, 16, *src++) << pel;
			pel -= bits;
			if (pel < 0) {
				pel = 8 - bits;
				*++dst = 0;
			}
		}
		src += stride;
		if (pel != 8 - bits)
			++dst;
	}

	return dst;
}

static void
tile_8(struct trle_ctx_t *ctx, uint8_t **buf,
	uint8_t *src, int xs, int ys, int stride, int bpp)
{
	int y;
	int subencoding;

	uint8_t *dst = *buf;

	stride -= xs;

	*dst++ = subencoding =
		scan_8(ctx, src, xs, ys, stride);

	if (subencoding == TRLE_RAW) {
		/* raw */
		ctx->palette_size = 0;
		for (y = 0; y < ys; ++y) {
			memcpy(dst, src, xs);
			src += stride + xs;
			dst += xs;
		}
		goto done;
	}

	if (subencoding == TRLE_RLE) {
		/* plain rle */
		ctx->palette_size = 0;
		dst = plain_rle_8(dst, src, xs, ys, stride);
		goto done;
	}

	/* palettized subencodings follows */

	if (subencoding == TRLE_PACKED_MEM)
		subencoding = TRLE_PACKED_BASE + ctx->palette_size;
	else if (subencoding == TRLE_PRLE_MEM)
		subencoding = TRLE_PRLE_BASE + ctx->palette_size;
	else {
		memcpy(dst, ctx->palette.p8, ctx->palette_size);
		dst += ctx->palette_size;
	}

	if (subencoding == TRLE_SOLID) {
		/* solid */
		ctx->palette_size = 0;
		goto done;
	}

	if (subencoding >= TRLE_PRLE_START) {
		/* palette rle */
		dst = palette_rle_8(dst, src, xs, ys, stride,
			ctx->palette.p8, ctx->palette_size);
		goto done;
	}

	if (subencoding == TRLE_PACKED_1) {
		/* packed palette */
		dst = packed_palette_8(dst, src, xs, ys, stride,
			ctx->palette.p8, 1);
		goto done;
	}

	if (subencoding <= TRLE_PACKED_2_END) {
		/* packed palette */
		dst = packed_palette_8(dst, src, xs, ys, stride,
			ctx->palette.p8, 2);
		goto done;
	}

	if (subencoding <= TRLE_PACKED_4_END) {
		/* packed palette */
		dst = packed_palette_8(dst, src, xs, ys, stride,
			ctx->palette.p8, 4);
		goto done;
	}

done:
	*buf = dst;
}

static void
tile_16(struct trle_ctx_t *ctx, uint8_t **buf,
	uint8_t *src8, int xs, int ys, int stride, int rev)
{
	uint16_t *src = (uint16_t *)src8;
	int x, y;
	int c;
	int subencoding;

	uint8_t *dst = *buf;

	stride -= xs;

	*dst++ = subencoding = scan_16(ctx, src, xs, ys, stride);

	if (subencoding == TRLE_RAW) {
		/* raw */
		ctx->palette_size = 0;
		if (!rev) {
			for (y = 0; y < ys; ++y) {
				memcpy(dst, src, xs * 2);
				src += stride + xs;
				dst += xs * 2;
			}
			goto done;
		}
		for (y = 0; y < ys; ++y) {
			for (x = 0; x < xs; ++x)
				dst = insert_rev_16(dst, src[x]);
			src += stride + xs;
		}
		goto done;
	}

	if (subencoding == TRLE_RLE) {
		/* plain rle */
		ctx->palette_size = 0;
		if (!rev)
			dst = plain_rle_16(
				dst, src, xs, ys, stride, insert_16);
		else
			dst = plain_rle_16(
				dst, src, xs, ys, stride, insert_rev_16);
		goto done;
	}

	/* palettized subencodings follows */

	if (subencoding == TRLE_PACKED_MEM)
		subencoding = TRLE_PACKED_BASE + ctx->palette_size;
	else if (subencoding == TRLE_PRLE_MEM)
		subencoding = TRLE_PRLE_BASE + ctx->palette_size;
	else if (!rev) {
		memcpy(dst, ctx->palette.p16, ctx->palette_size * 2);
		dst += ctx->palette_size * 2;
	}
	else {
		for (c = 0; c < ctx->palette_size; ++c)
			dst = insert_rev_16(dst, ctx->palette.p16[c]);
	}

	if (subencoding == TRLE_SOLID) {
		/* solid */
		ctx->palette_size = 0;
		goto done;
	}

	if (subencoding >= TRLE_PRLE_START) {
		/* palette rle */
		dst = palette_rle_16(dst, src, xs, ys, stride,
			ctx->palette.p16, ctx->palette_size);
		goto done;
	}

	if (subencoding == TRLE_PACKED_1) {
		/* packed palette */
		dst = packed_palette_16(dst, src, xs, ys, stride,
			ctx->palette.p16, 1);
		goto done;
	}

	if (subencoding <= TRLE_PACKED_2_END) {
		/* packed palette */
		dst = packed_palette_16(dst, src, xs, ys, stride,
			ctx->palette.p16, 2);
		goto done;
	}

	if (subencoding <= TRLE_PACKED_4_END) {
		/* packed palette */
		dst = packed_palette_16(dst, src, xs, ys, stride,
			ctx->palette.p16, 4);
		goto done;
	}

done:
	*buf = dst;
}

static void
tile_24(struct trle_ctx_t *ctx, uint8_t **buf,
	uint8_t *src8, int xs, int ys, int stride, int lower)
{
	uint32_t *src = (uint32_t *)src8;
	int x, y;
	int c;
	int subencoding;

	uint8_t *dst = *buf;

	stride -= xs;

	*dst++ = subencoding = scan_32(ctx, src, xs, ys, stride, 3);

	if (subencoding == TRLE_RAW) {
		/* raw */
		ctx->palette_size = 0;
		for (y = 0; y < ys; ++y) {
			for (x = 0; x < xs; ++x) {
#ifdef GGI_BIG_ENDIAN
				dst = insert_hilo_24(dst, src[x], lower);
#else
				dst = insert_lohi_24(dst, src[x], lower);
#endif
			}
			src += stride + xs;
		}
		goto done;
	}

	if (subencoding == TRLE_RLE) {
		/* plain rle */
		ctx->palette_size = 0;
		if (lower)
			dst = plain_rle_32(
				dst, src, xs, ys, stride, insert_24l);
		else
			dst = plain_rle_32(
				dst, src, xs, ys, stride, insert_24h);
		goto done;
	}

	/* palettized subencodings follows */

	if (subencoding == TRLE_PACKED_MEM)
		subencoding = TRLE_PACKED_BASE + ctx->palette_size;
	else if (subencoding == TRLE_PRLE_MEM)
		subencoding = TRLE_PRLE_BASE + ctx->palette_size;
	else if (lower) {
		for (c = 0; c < ctx->palette_size; ++c)
			dst = insert_24l(dst, ctx->palette.p32[c]);
	}
	else {
		for (c = 0; c < ctx->palette_size; ++c)
			dst = insert_24h(dst, ctx->palette.p32[c]);
	}

	if (subencoding == TRLE_SOLID) {
		/* solid */
		ctx->palette_size = 0;
		goto done;
	}

	if (subencoding >= TRLE_PRLE_START) {
		/* palette rle */
		dst = palette_rle_32(dst, src, xs, ys, stride,
			ctx->palette.p32, ctx->palette_size);
		goto done;
	}

	if (subencoding == TRLE_PACKED_1) {
		/* packed palette */
		dst = packed_palette_32(dst, src, xs, ys, stride,
			ctx->palette.p32, 1);
		goto done;
	}

	if (subencoding <= TRLE_PACKED_2_END) {
		/* packed palette */
		dst = packed_palette_32(dst, src, xs, ys, stride,
			ctx->palette.p32, 2);
		goto done;
	}

	if (subencoding <= TRLE_PACKED_4_END) {
		/* packed palette */
		dst = packed_palette_32(dst, src, xs, ys, stride,
			ctx->palette.p32, 4);
		goto done;
	}

done:
	*buf = dst;
}

static void
tile_rev_24(struct trle_ctx_t *ctx, uint8_t **buf,
	uint8_t *src8, int xs, int ys, int stride, int lower)
{
	uint32_t *src = (uint32_t *)src8;
	int x, y;
	int c;
	int subencoding;

	uint8_t *dst = *buf;

	stride -= xs;

	*dst++ = subencoding = scan_32(ctx, src, xs, ys, stride, 3);

	if (subencoding == TRLE_RAW) {
		/* raw */
		ctx->palette_size = 0;
		for (y = 0; y < ys; ++y) {
			for (x = 0; x < xs; ++x) {
#ifdef GGI_BIG_ENDIAN
				dst = insert_lohi_24(dst, src[x], lower);
#else
				dst = insert_hilo_24(dst, src[x], lower);
#endif
			}
			src += stride + xs;
		}
		goto done;
	}

	if (subencoding == TRLE_RLE) {
		/* plain rle */
		ctx->palette_size = 0;
		if (lower)
			dst = plain_rle_32(
				dst, src, xs, ys, stride, insert_rev_24l);
		else
			dst = plain_rle_32(
				dst, src, xs, ys, stride, insert_rev_24h);
		goto done;
	}

	/* palettized subencodings follows */

	if (subencoding == TRLE_PACKED_MEM)
		subencoding = TRLE_PACKED_BASE + ctx->palette_size;
	else if (subencoding == TRLE_PRLE_MEM)
		subencoding = TRLE_PRLE_BASE + ctx->palette_size;
	else if (lower) {
		for (c = 0; c < ctx->palette_size; ++c)
			dst = insert_rev_24l(dst, ctx->palette.p32[c]);
	}
	else {
		for (c = 0; c < ctx->palette_size; ++c)
			dst = insert_rev_24h(dst, ctx->palette.p32[c]);
	}

	if (subencoding == TRLE_SOLID) {
		/* solid */
		ctx->palette_size = 0;
		goto done;
	}

	if (subencoding >= TRLE_PRLE_START) {
		/* palette rle */
		dst = palette_rle_32(dst, src, xs, ys, stride,
			ctx->palette.p32, ctx->palette_size);
		goto done;
	}

	if (subencoding == TRLE_PACKED_1) {
		/* packed palette */
		dst = packed_palette_32(dst, src, xs, ys, stride,
			ctx->palette.p32, 1);
		goto done;
	}

	if (subencoding <= TRLE_PACKED_2_END) {
		/* packed palette */
		dst = packed_palette_32(dst, src, xs, ys, stride,
			ctx->palette.p32, 2);
		goto done;
	}

	if (subencoding <= TRLE_PACKED_4_END) {
		/* packed palette */
		dst = packed_palette_32(dst, src, xs, ys, stride,
			ctx->palette.p32, 4);
		goto done;
	}

done:
	*buf = dst;
}

static void
tile_32(struct trle_ctx_t *ctx, uint8_t **buf,
	uint8_t *src8, int xs, int ys, int stride, int rev)
{
	uint32_t *src = (uint32_t *)src8;
	int x, y;
	int c;
	int subencoding;

	uint8_t *dst = *buf;

	stride -= xs;

	*dst++ = subencoding = scan_32(ctx, src, xs, ys, stride, 4);

	if (subencoding == TRLE_RAW) {
		/* raw */
		ctx->palette_size = 0;
		if (!rev) {
			for (y = 0; y < ys; ++y) {
				memcpy(dst, src, xs * 4);
				src += stride + xs;
				dst += xs * 4;
			}
			goto done;
		}
		for (y = 0; y < ys; ++y) {
			for (x = 0; x < xs; ++x)
				dst = insert_rev_32(dst, src[x]);
			src += stride + xs;
		}
		goto done;
	}

	if (subencoding == TRLE_RLE) {
		/* plain rle */
		ctx->palette_size = 0;
		if (!rev)
			dst = plain_rle_32(
				dst, src, xs, ys, stride, insert_32);
		else
			dst = plain_rle_32(
				dst, src, xs, ys, stride, insert_rev_32);
		goto done;
	}

	/* palettized subencodings follows */

	if (subencoding == TRLE_PACKED_MEM)
		subencoding = TRLE_PACKED_BASE + ctx->palette_size;
	else if (subencoding == TRLE_PRLE_MEM)
		subencoding = TRLE_PRLE_BASE + ctx->palette_size;
	if (!rev) {
		memcpy(dst, ctx->palette.p32, ctx->palette_size * 4);
		dst += ctx->palette_size * 4;
	}
	else {
		for (c = 0; c < ctx->palette_size; ++c)
			dst = insert_rev_32(dst, ctx->palette.p32[c]);
	}

	if (subencoding == TRLE_SOLID) {
		/* solid */
		ctx->palette_size = 0;
		goto done;
	}

	if (subencoding >= TRLE_PRLE_START) {
		/* palette rle */
		dst = palette_rle_32(dst, src, xs, ys, stride,
			ctx->palette.p32, ctx->palette_size);
		goto done;
	}

	if (subencoding == TRLE_PACKED_1) {
		/* packed palette */
		dst = packed_palette_32(dst, src, xs, ys, stride,
			ctx->palette.p32, 1);
		goto done;
	}

	if (subencoding <= TRLE_PACKED_2_END) {
		/* packed palette */
		dst = packed_palette_32(dst, src, xs, ys, stride,
			ctx->palette.p32, 2);
		goto done;
	}

	if (subencoding <= TRLE_PACKED_4_END) {
		/* packed palette */
		dst = packed_palette_32(dst, src, xs, ys, stride,
			ctx->palette.p32, 4);
		goto done;
	}

done:
	*buf = dst;
}

int
GGI_vnc_trle(ggi_vnc_client *client, ggi_rect *update)
{
	struct trle_ctx_t *ctx = client->trle_ctx;
	struct ggi_visual *cvis;
	const ggi_directbuffer *db;
	ggi_graphtype gt;
	int bpp;
	int cbpp;
	int count;
	unsigned char *buf;
	unsigned char *header;
	int xtiles, ytiles;
	unsigned char *work;
	int lower = 1;
	int tile_param;
	tile_func *tile;
	int xt, yt;
	int xs, ys;
	int xs_last, ys_last;
	int stride;
	ggi_rect vupdate;
	int d_frame_num;

	DPRINT("trle update %dx%d - %dx%d\n",
		update->tl.x, update->tl.y,
		update->br.x, update->br.y);

	cvis = GGI_vnc_encode_init(client, update, &vupdate, &d_frame_num);

	gt = LIBGGI_GT(cvis);

	bpp = GT_ByPP(gt);
	if (bpp == 4) {
		ggi_pixel mask =
			LIBGGI_PIXFMT(cvis)->red_mask |
			LIBGGI_PIXFMT(cvis)->green_mask |
			LIBGGI_PIXFMT(cvis)->blue_mask;
		if (!(mask & 0xff000000))
			cbpp = 3;
		else if (!(mask & 0xff)) {
			lower = 0;
			cbpp = 3;
		}
		else
			cbpp = 4;
	}
	else
		cbpp = bpp;
	count = ggi_rect_width(&vupdate) * ggi_rect_height(&vupdate);
	xtiles = (ggi_rect_width(&vupdate) + 15) / 16;
	ytiles = (ggi_rect_height(&vupdate) + 15) / 16;
	GGI_vnc_buf_reserve(&client->wbuf, client->wbuf.size + 12
		+ xtiles * ytiles + count * cbpp);
	header = &client->wbuf.buf[client->wbuf.size];
	insert_header(header, &update->tl, &vupdate, 15); /* trle */

	client->wbuf.size += 12;
	work = &client->wbuf.buf[client->wbuf.size];
	buf = work;

	db = ggiDBGetBuffer(cvis->instance.stem, d_frame_num);
	ggiResourceAcquire(db->resource, GGI_ACTYPE_READ);

	if (bpp == 1) {
		tile_param = 0;
		tile = tile_8;
	}
	else if (bpp == 2) {
		tile_param = client->reverse_endian;
		tile = tile_16;
	}
	else if (cbpp == 3 && !client->reverse_endian) {
		tile_param = lower;
		tile = tile_24;
	}
	else if (cbpp == 3) {
		tile_param = lower;
		tile = tile_rev_24;
	}
	else {
		tile_param = client->reverse_endian;
		tile = tile_32;
	}

	stride = LIBGGI_VIRTX(cvis);

	ys_last = ggi_rect_height(&vupdate) & 0xf;
	if (!ys_last)
		ys_last = 16;
	xs_last = ggi_rect_width(&vupdate) & 0xf;
	if (!xs_last)
		xs_last = 16;

	ys = 16;
	for (yt = 0; yt < ytiles; ++yt) {
		if (yt == ytiles - 1)
			ys = ys_last;
		xs = 16;
		for (xt = 0; xt < xtiles; ++xt) {
			if (xt == xtiles - 1)
				xs = xs_last;
			tile(ctx, &buf, (uint8_t *)db->read +
				((vupdate.tl.y + 16 * yt) * stride +
				 vupdate.tl.x + 16 * xt) * bpp,
				xs, ys, stride, tile_param);
		}
	}

	ggiResourceRelease(db->resource);

	client->wbuf.size += buf - work;
	return 1;
}

struct trle_ctx_t *
GGI_vnc_trle_open(void)
{
	struct trle_ctx_t *ctx = _ggi_calloc(sizeof(*ctx));

	return ctx;
}

void
GGI_vnc_trle_close(struct trle_ctx_t *ctx)
{
	free(ctx);
}
