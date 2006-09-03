/* $Id: zrle.c,v 1.31 2006/09/03 21:00:29 pekberg Exp $
******************************************************************************

   display-vnc: RFB zrle encoding

   Copyright (C) 2006 Peter Rosin	[peda@lysator.liu.se]

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

#include <zlib.h>

#include <ggi/display/vnc.h>
#include <ggi/internal/ggi_debug.h>
#include "rect.h"
#include "encoding.h"

#ifdef GGI_BIG_ENDIAN
#define GGI_HTONL(x) (x)
#else
#define GGI_HTONL(x) GGI_BYTEREV32(x)
#endif

struct zrle_ctx_t {
	z_stream zstr;
	ggi_vnc_buf work;
};

/* subencodings */
#define ZRLE_RAW              0
#define ZRLE_SOLID            1
#define ZRLE_PACKED_1         2
#define ZRLE_PACKED_2_START   3
#define ZRLE_PACKED_2_END     4
#define ZRLE_PACKED_4_START   5
#define ZRLE_PACKED_4_END    16
#define ZRLE_PACKED_BASE      0
#define ZRLE_PACKED_START     2
#define ZRLE_PACKED_END      16
#define ZRLE_RLE            128
#define ZRLE_PRLE_BASE      128
#define ZRLE_PRLE_START     130
#define ZRLE_PRLE_END       255

typedef void (tile_func)(uint8_t **buf, uint8_t *src,
	int xs, int ys, int stride, int bpp);

static void
zip(ggi_vnc_priv *priv, uint8_t *src, int len)
{
	ggi_vnc_client *client = priv->client;
	struct zrle_ctx_t *ctx = client->zrle_ctx;
	int start = client->wbuf.size;
	int avail;
	uint32_t *zlen;
	uint32_t done = 0;

	ctx->zstr.next_in = src;
	ctx->zstr.avail_in = len;

	client->wbuf.size += 4;
	
	avail = client->wbuf.limit - client->wbuf.size;

	for (;;) {
		ctx->zstr.next_out =
			&client->wbuf.buf[client->wbuf.size + done];
		ctx->zstr.avail_out = avail - done;
		deflate(&ctx->zstr, Z_SYNC_FLUSH);
		done = avail - ctx->zstr.avail_out;
		if (ctx->zstr.avail_out)
			break;
		avail += 1000;
		GGI_vnc_buf_reserve(&client->wbuf, client->wbuf.size + avail);
	}

	zlen = (uint32_t *)&client->wbuf.buf[start];
	*zlen = GGI_HTONL(done);
	client->wbuf.size += done;

	DPRINT_MISC("rle %d z %d %d%%\n", len, done, done * 100 / len);
}

static uint8_t
select_subencoding(int xs, int ys, int cbpp,
	int colors, int single, int multi, int *best)
{
	int bytes;
	int subencoding;

	/* solid */
	if (colors == 1) {
		*best = cbpp;
		return ZRLE_SOLID;
	}

	/* raw */
	*best = xs * ys * cbpp;
	subencoding = ZRLE_RAW;

	/* palette rle */
	if (2 <= colors && colors <= 127) {
		bytes = cbpp * colors + single + 9 * multi / 4;
		if (bytes < *best) {
			*best = bytes;
			subencoding = ZRLE_PRLE_BASE + colors;
		}
	}

	/* plain rle */
	bytes = (1 + cbpp) * single + cbpp * multi + 9 * multi / 4;
	if (bytes < *best) {
		*best = bytes;
		subencoding = ZRLE_RLE;
	}

	/* packed palette */
	bytes = *best + 1;
	if (colors == 2)
		bytes = cbpp * colors + (xs + 7) / 8 * ys;
	else if (colors == 3 || colors == 4)
		bytes = cbpp * colors + (xs + 3) / 4 * ys;
	else if (5 <= colors && colors <= 16)
		bytes = cbpp * colors + (xs + 1) / 2 * ys;
	if (bytes < *best) {
		*best = bytes;
		subencoding = ZRLE_PACKED_BASE + colors;
	}

	return subencoding;
}

static inline uint8_t
palette_match_8(uint8_t *palette, int colors, uint8_t color)
{
	int c;

	for (c = 0; c < colors; ++c) {
		if (palette[c] == color)
			break;
	}

	return c;
}

static inline uint8_t
palette_match_16(uint16_t *palette, int colors, uint16_t color)
{
	int c;

	for (c = 0; c < colors; ++c) {
		if (palette[c] == color)
			break;
	}

	return c;
}

static inline uint8_t
palette_match_32(uint32_t *palette, int colors, uint32_t color)
{
	int c;

	for (c = 0; c < colors; ++c) {
		if (palette[c] == color)
			break;
	}

	return c;
}

static inline uint8_t
scan_8(uint8_t *src,
	int xs, int ys, int stride, uint8_t *palette, int *colors)
{
	int x, y;
	uint8_t last = *src;
	uint8_t here;
	int rl = 0;
	int single = 0;
	int multi = 0;
	int bytes;
	int c;

	*colors = 1;
	palette[0] = *src;

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
			if (*colors == 128)
				continue;
			c = palette_match_8(palette, *colors, here);
			if (c == *colors)
				palette[(*colors)++] = here;
		}
		src += stride;
	}
	if (rl == 1)
		++single;
	else
		++multi;

	return select_subencoding(
		xs, ys, 1, *colors, single, multi, &bytes);
}

static inline uint8_t
scan_16(uint16_t *src,
	int xs, int ys, int stride, uint16_t *palette, int *colors)
{
	int x, y;
	uint16_t last = *src;
	uint16_t here;
	int rl = 0;
	int single = 0;
	int multi = 0;
	int bytes;
	int c;

	*colors = 1;
	palette[0] = *src;

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
			if (*colors == 128)
				continue;
			c = palette_match_16(palette, *colors, here);
			if (c == *colors)
				palette[(*colors)++] = here;
		}
		src += stride;
	}
	if (rl == 1)
		++single;
	else
		++multi;

	return select_subencoding(
		xs, ys, 2, *colors, single, multi, &bytes);
}

static inline uint8_t
scan_32(uint32_t *src,
	int xs, int ys, int stride, uint32_t *palette, int *colors, int cbpp)
{
	int x, y;
	uint32_t last = *src;
	uint32_t here;
	int rl = 0;
	int single = 0;
	int multi = 0;
	int bytes;
	int c;

	*colors = 1;
	palette[0] = *src;

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
			if (*colors == 128)
				continue;
			c = palette_match_32(palette, *colors, here);
			if (c == *colors)
				palette[(*colors)++] = here;
		}
		src += stride;
	}
	if (rl == 1)
		++single;
	else
		++multi;

	return select_subencoding(
		xs, ys, cbpp, *colors, single, multi, &bytes);
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
	uint16_t c = palette_match_16(palette, colors, color);
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
	uint32_t c = palette_match_32(palette, colors, color);
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

static inline uint8_t *
insert_hilo_16(uint8_t *dst, uint16_t pixel)
{
	*dst++ = pixel >> 8;
	*dst++ = pixel;
	return dst;
}

static inline uint8_t *
insert_hilo_24l(uint8_t *dst, uint32_t pixel)
{
	*dst++ = pixel >> 16;
	*dst++ = pixel >> 8;
	*dst++ = pixel;
	return dst;
}

static inline uint8_t *
insert_hilo_24h(uint8_t *dst, uint32_t pixel)
{
	*dst++ = pixel >> 24;
	*dst++ = pixel >> 16;
	*dst++ = pixel >> 8;
	return dst;
}

static inline uint8_t *
insert_hilo_24(uint8_t *dst, uint32_t pixel, int lower)
{
	if (lower)
		return insert_hilo_24l(dst, pixel);
	else
		return insert_hilo_24h(dst, pixel);
}

static inline uint8_t *
insert_hilo_32(uint8_t *dst, uint32_t pixel)
{
	*dst++ = pixel >> 24;
	*dst++ = pixel >> 16;
	*dst++ = pixel >> 8;
	*dst++ = pixel;
	return dst;
}

static inline uint8_t *
insert_lohi_16(uint8_t *dst, uint16_t pixel)
{
	*dst++ = pixel;
	*dst++ = pixel >> 8;
	return dst;
}

static inline uint8_t *
insert_lohi_24l(uint8_t *dst, uint32_t pixel)
{
	*dst++ = pixel;
	*dst++ = pixel >> 8;
	*dst++ = pixel >> 16;
	return dst;
}

static inline uint8_t *
insert_lohi_24h(uint8_t *dst, uint32_t pixel)
{
	*dst++ = pixel >> 8;
	*dst++ = pixel >> 16;
	*dst++ = pixel >> 24;
	return dst;
}

static inline uint8_t *
insert_lohi_24(uint8_t *dst, uint32_t pixel, int lower)
{
	if (lower)
		return insert_lohi_24l(dst, pixel);
	else
		return insert_lohi_24h(dst, pixel);
}

static inline uint8_t *
insert_lohi_32(uint8_t *dst, uint32_t pixel)
{
	*dst++ = pixel;
	*dst++ = pixel >> 8;
	*dst++ = pixel >> 16;
	*dst++ = pixel >> 24;
	return dst;
}

static inline uint8_t *
insert_rev_16(uint8_t *dst, uint16_t pixel)
{
#ifdef GGI_BIG_ENDIAN
	return insert_lohi_16(dst, pixel);
#else
	return insert_hilo_16(dst, pixel);
#endif
}

static inline uint8_t *
insert_rev_24l(uint8_t *dst, uint32_t pixel)
{
#ifdef GGI_BIG_ENDIAN
	return insert_lohi_24l(dst, pixel);
#else
	return insert_hilo_24l(dst, pixel);
#endif
}

static inline uint8_t *
insert_rev_24h(uint8_t *dst, uint32_t pixel)
{
#ifdef GGI_BIG_ENDIAN
	return insert_lohi_24h(dst, pixel);
#else
	return insert_hilo_24h(dst, pixel);
#endif
}

static inline uint8_t *
insert_rev_32(uint8_t *dst, uint32_t pixel)
{
#ifdef GGI_BIG_ENDIAN
	return insert_lohi_32(dst, pixel);
#else
	return insert_hilo_32(dst, pixel);
#endif
}

static inline uint8_t *
insert_16(uint8_t *dst, uint16_t pixel)
{
#ifdef GGI_BIG_ENDIAN
	return insert_hilo_16(dst, pixel);
#else
	return insert_lohi_16(dst, pixel);
#endif
}

static inline uint8_t *
insert_24l(uint8_t *dst, uint32_t pixel)
{
#ifdef GGI_BIG_ENDIAN
	return insert_hilo_24l(dst, pixel);
#else
	return insert_lohi_24l(dst, pixel);
#endif
}

static inline uint8_t *
insert_24h(uint8_t *dst, uint32_t pixel)
{
#ifdef GGI_BIG_ENDIAN
	return insert_hilo_24h(dst, pixel);
#else
	return insert_lohi_24h(dst, pixel);
#endif
}

static inline uint8_t *
insert_32(uint8_t *dst, uint32_t pixel)
{
#ifdef GGI_BIG_ENDIAN
	return insert_hilo_32(dst, pixel);
#else
	return insert_lohi_32(dst, pixel);
#endif
}

static void
tile_8(uint8_t **buf, uint8_t *src, int xs, int ys, int stride, int bpp)
{
	uint8_t palette[128];
	int colors;
	int y;
	int subencoding;

	uint8_t *dst = *buf;

	stride -= xs;

	*dst++ = subencoding =
		scan_8(src, xs, ys, stride, palette, &colors);

	if (subencoding == ZRLE_RAW) {
		/* raw */
		for (y = 0; y < ys; ++y) {
			memcpy(dst, src, xs);
			src += stride + xs;
			dst += xs;
		}
		goto done;
	}

	if (subencoding == ZRLE_RLE) {
		/* plain rle */
		dst = plain_rle_8(dst, src, xs, ys, stride);
		goto done;
	}

	/* palettized subencodings follows */
	memcpy(dst, palette, colors);
	dst += colors;

	if (subencoding == ZRLE_SOLID)
		/* solid */
		goto done;

	if (subencoding >= ZRLE_PRLE_START) {
		/* palette rle */
		dst = palette_rle_8(
			dst, src, xs, ys, stride, palette, colors);
		goto done;
	}

	if (subencoding == ZRLE_PACKED_1) {
		/* packed palette */
		dst = packed_palette_8(dst, src, xs, ys, stride, palette, 1);
		goto done;
	}

	if (subencoding <= ZRLE_PACKED_2_END) {
		/* packed palette */
		dst = packed_palette_8(dst, src, xs, ys, stride, palette, 2);
		goto done;
	}

	if (subencoding <= ZRLE_PACKED_4_END) {
		/* packed palette */
		dst = packed_palette_8(dst, src, xs, ys, stride, palette, 4);
		goto done;
	}

done:
	*buf = dst;
}

static void
tile_16(uint8_t **buf, uint8_t *src8, int xs, int ys, int stride, int rev)
{
	uint16_t *src = (uint16_t *)src8;
	uint16_t palette[128];
	int colors;
	int x, y;
	int c;
	int subencoding;

	uint8_t *dst = *buf;

	stride -= xs;

	*dst++ = subencoding =
		scan_16(src, xs, ys, stride, palette, &colors);

	if (subencoding == ZRLE_RAW) {
		/* raw */
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

	if (subencoding == ZRLE_RLE) {
		/* plain rle */
		if (!rev)
			dst = plain_rle_16(
				dst, src, xs, ys, stride, insert_16);
		else
			dst = plain_rle_16(
				dst, src, xs, ys, stride, insert_rev_16);
		goto done;
	}

	/* palettized subencodings follows */
	if (!rev) {
		memcpy(dst, palette, colors * 2);
		dst += colors * 2;
	}
	else {
		for (c = 0; c < colors; ++c)
			dst = insert_rev_16(dst, palette[c]);
	}

	if (subencoding == ZRLE_SOLID)
		/* solid */
		goto done;

	if (subencoding >= ZRLE_PRLE_START) {
		/* palette rle */
		dst = palette_rle_16(
			dst, src, xs, ys, stride, palette, colors);
		goto done;
	}

	if (subencoding == ZRLE_PACKED_1) {
		/* packed palette */
		dst = packed_palette_16(dst, src, xs, ys, stride, palette, 1);
		goto done;
	}

	if (subencoding <= ZRLE_PACKED_2_END) {
		/* packed palette */
		dst = packed_palette_16(dst, src, xs, ys, stride, palette, 2);
		goto done;
	}

	if (subencoding <= ZRLE_PACKED_4_END) {
		/* packed palette */
		dst = packed_palette_16(dst, src, xs, ys, stride, palette, 4);
		goto done;
	}

done:
	*buf = dst;
}

static void
tile_24(uint8_t **buf,
	uint8_t *src8, int xs, int ys, int stride, int lower)
{
	uint32_t *src = (uint32_t *)src8;
	uint32_t palette[128];
	int colors;
	int x, y;
	int c;
	int subencoding;

	uint8_t *dst = *buf;

	stride -= xs;

	*dst++ = subencoding =
		scan_32(src, xs, ys, stride, palette, &colors, 3);

	if (subencoding == ZRLE_RAW) {
		/* raw */
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

	if (subencoding == ZRLE_RLE) {
		/* plain rle */
		if (lower)
			dst = plain_rle_32(
				dst, src, xs, ys, stride, insert_24l);
		else
			dst = plain_rle_32(
				dst, src, xs, ys, stride, insert_24h);
		goto done;
	}

	/* palettized subencodings follows */
	if (lower) {
		for (c = 0; c < colors; ++c)
			dst = insert_24l(dst, palette[c]);
	}
	else {
		for (c = 0; c < colors; ++c)
			dst = insert_24h(dst, palette[c]);
	}

	if (subencoding == ZRLE_SOLID)
		/* solid */
		goto done;

	if (subencoding >= ZRLE_PRLE_START) {
		/* palette rle */
		dst = palette_rle_32(
			dst, src, xs, ys, stride, palette, colors);
		goto done;
	}

	if (subencoding == ZRLE_PACKED_1) {
		/* packed palette */
		dst = packed_palette_32(dst, src, xs, ys, stride, palette, 1);
		goto done;
	}

	if (subencoding <= ZRLE_PACKED_2_END) {
		/* packed palette */
		dst = packed_palette_32(dst, src, xs, ys, stride, palette, 2);
		goto done;
	}

	if (subencoding <= ZRLE_PACKED_4_END) {
		/* packed palette */
		dst = packed_palette_32(dst, src, xs, ys, stride, palette, 4);
		goto done;
	}

done:
	*buf = dst;
}

static void
tile_rev_24(uint8_t **buf,
	uint8_t *src8, int xs, int ys, int stride, int lower)
{
	uint32_t *src = (uint32_t *)src8;
	uint32_t palette[128];
	int colors;
	int x, y;
	int c;
	int subencoding;

	uint8_t *dst = *buf;

	stride -= xs;

	*dst++ = subencoding =
		scan_32(src, xs, ys, stride, palette, &colors, 3);

	if (subencoding == ZRLE_RAW) {
		/* raw */
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

	if (subencoding == ZRLE_RLE) {
		/* plain rle */
		if (lower)
			dst = plain_rle_32(
				dst, src, xs, ys, stride, insert_rev_24l);
		else
			dst = plain_rle_32(
				dst, src, xs, ys, stride, insert_rev_24h);
		goto done;
	}

	/* palettized subencodings follows */
	if (lower) {
		for (c = 0; c < colors; ++c)
			dst = insert_rev_24l(dst, palette[c]);
	}
	else {
		for (c = 0; c < colors; ++c)
			dst = insert_rev_24h(dst, palette[c]);
	}

	if (subencoding == ZRLE_SOLID)
		/* solid */
		goto done;

	if (subencoding >= ZRLE_PRLE_START) {
		/* palette rle */
		dst = palette_rle_32(
			dst, src, xs, ys, stride, palette, colors);
		goto done;
	}

	if (subencoding == ZRLE_PACKED_1) {
		/* packed palette */
		dst = packed_palette_32(dst, src, xs, ys, stride, palette, 1);
		goto done;
	}

	if (subencoding <= ZRLE_PACKED_2_END) {
		/* packed palette */
		dst = packed_palette_32(dst, src, xs, ys, stride, palette, 2);
		goto done;
	}

	if (subencoding <= ZRLE_PACKED_4_END) {
		/* packed palette */
		dst = packed_palette_32(dst, src, xs, ys, stride, palette, 4);
		goto done;
	}

done:
	*buf = dst;
}

static void
tile_32(uint8_t **buf, uint8_t *src8, int xs, int ys, int stride, int rev)
{
	uint32_t *src = (uint32_t *)src8;
	uint32_t palette[128];
	int colors;
	int x, y;
	int c;
	int subencoding;

	uint8_t *dst = *buf;

	stride -= xs;

	*dst++ = subencoding =
		scan_32(src, xs, ys, stride, palette, &colors, 4);

	if (subencoding == ZRLE_RAW) {
		/* raw */
		if (!rev) {
			for (y = 0; y < ys; ++y) {
				memcpy(dst, src, xs * 2);
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

	if (subencoding == ZRLE_RLE) {
		/* plain rle */
		if (!rev)
			dst = plain_rle_32(
				dst, src, xs, ys, stride, insert_32);
		else
			dst = plain_rle_32(
				dst, src, xs, ys, stride, insert_rev_32);
		goto done;
	}

	/* palettized subencodings follows */
	if (!rev) {
		memcpy(dst, palette, colors * 4);
		dst += colors * 4;
	}
	else {
		for (c = 0; c < colors; ++c)
			dst = insert_rev_32(dst, palette[c]);
	}

	if (subencoding == ZRLE_SOLID)
		/* solid */
		goto done;

	if (subencoding >= ZRLE_PRLE_START) {
		/* palette rle */
		dst = palette_rle_32(
			dst, src, xs, ys, stride, palette, colors);
		goto done;
	}

	if (subencoding == ZRLE_PACKED_1) {
		/* packed palette */
		dst = packed_palette_32(dst, src, xs, ys, stride, palette, 1);
		goto done;
	}

	if (subencoding <= ZRLE_PACKED_2_END) {
		/* packed palette */
		dst = packed_palette_32(dst, src, xs, ys, stride, palette, 2);
		goto done;
	}

	if (subencoding <= ZRLE_PACKED_4_END) {
		/* packed palette */
		dst = packed_palette_32(dst, src, xs, ys, stride, palette, 4);
		goto done;
	}

done:
	*buf = dst;
}

void
GGI_vnc_zrle(struct ggi_visual *vis, ggi_rect *update)
{
	ggi_vnc_priv *priv = VNC_PRIV(vis);
	ggi_vnc_client *client = priv->client;
	struct zrle_ctx_t *ctx = client->zrle_ctx;
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
	ggi_rect vupdate, visible;
	int d_frame_num;

	DPRINT("update %dx%d - %dx%d\n",
		update->tl.x, update->tl.y,
		update->br.x, update->br.y);

	vupdate = *update;
	ggi_rect_shift_xy(&vupdate, vis->origin_x, vis->origin_y);

	visible.tl.x = vis->origin_x;
	visible.tl.y = vis->origin_y;
	visible.br.x = vis->origin_x + LIBGGI_X(vis);
	visible.br.y = vis->origin_y + LIBGGI_Y(vis);
	ggi_rect_intersect(&client->dirty, &visible);

	ggi_rect_subtract(&client->dirty, &vupdate);
	client->update.tl.x = client->update.br.x = 0;

	DPRINT("dirty %dx%d - %dx%d\n",
		client->dirty.tl.x, client->dirty.tl.y,
		client->dirty.br.x, client->dirty.br.y);

	if (!client->vis) {
		cvis = priv->fb;
		d_frame_num = vis->d_frame_num;
	}
	else {
		int r_frame_num = _ggiGetReadFrame(priv->fb);
		_ggiSetReadFrame(priv->fb, _ggiGetDisplayFrame(vis));

		cvis = client->vis;
		_ggiCrossBlit(priv->fb,
			vupdate.tl.x, vupdate.tl.y,
			ggi_rect_width(&vupdate),
			ggi_rect_height(&vupdate),
			cvis,
			vupdate.tl.x, vupdate.tl.y);

		_ggiSetReadFrame(priv->fb, r_frame_num);
		d_frame_num = 0;
	}

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
	xtiles = (ggi_rect_width(&vupdate) + 63) / 64;
	ytiles = (ggi_rect_height(&vupdate) + 63) / 64;
	GGI_vnc_buf_reserve(&ctx->work, xtiles * ytiles + count * cbpp);
	work = ctx->work.buf;
	GGI_vnc_buf_reserve(&client->wbuf, client->wbuf.size + 20);
	header = &client->wbuf.buf[client->wbuf.size];
	header[ 0] = 0;
	header[ 1] = 0;
	header[ 2] = 0;
	header[ 3] = 1;
	header[ 4] = update->tl.x >> 8;
	header[ 5] = update->tl.x & 0xff;
	header[ 6] = update->tl.y >> 8;
	header[ 7] = update->tl.y & 0xff;
	header[ 8] = ggi_rect_width(&vupdate) >> 8;
	header[ 9] = ggi_rect_width(&vupdate) & 0xff;
	header[10] = ggi_rect_height(&vupdate) >> 8;
	header[11] = ggi_rect_height(&vupdate) & 0xff;
	header[12] = 0;
	header[13] = 0;
	header[14] = 0;
	header[15] = 16; /* zrle */

	client->wbuf.size += 16;
	buf = work;

	db = ggiDBGetBuffer(cvis->stem, d_frame_num);
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

	ys_last = ggi_rect_height(&vupdate) & 0x3f;
	if (!ys_last)
		ys_last = 64;
	xs_last = ggi_rect_width(&vupdate) & 0x3f;
	if (!xs_last)
		xs_last = 64;

	ys = 64;
	for (yt = 0; yt < ytiles; ++yt) {
		if (yt == ytiles - 1)
			ys = ys_last;
		xs = 64;
		for (xt = 0; xt < xtiles; ++xt) {
			if (xt == xtiles - 1)
				xs = xs_last;
			tile(&buf, (uint8_t *)db->read +
				((vupdate.tl.y + 64 * yt) * stride +
				 vupdate.tl.x + 64 * xt) * bpp,
				xs, ys, stride, tile_param);
		}
	}

	ggiResourceRelease(db->resource);

	zip(priv, work, buf - work);
}

struct zrle_ctx_t *
GGI_vnc_zrle_open(int level)
{
	struct zrle_ctx_t *ctx = malloc(sizeof(*ctx));

	memset(ctx, 0, sizeof(*ctx));

	ctx->zstr.zalloc = Z_NULL;
	ctx->zstr.zfree = Z_NULL;
	ctx->zstr.opaque = Z_NULL;

	if (level == -1)
		level = Z_DEFAULT_COMPRESSION;

	deflateInit(&ctx->zstr, level);
	
	return ctx;
}

void
GGI_vnc_zrle_close(struct zrle_ctx_t *ctx)
{
	if (ctx->work.buf)
		free(ctx->work.buf);
	deflateEnd(&ctx->zstr);
	free(ctx);
}
