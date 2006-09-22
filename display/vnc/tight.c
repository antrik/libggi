/* $Id: tight.c,v 1.10 2006/09/22 21:20:16 pekberg Exp $
******************************************************************************

   display-vnc: RFB tight encoding

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

#include <stdio.h>
#include <stdlib.h>
#ifdef HAVE_STRING_H
#include <string.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include <zlib.h>
#include <jpeglib.h>

#include <ggi/display/vnc.h>
#include <ggi/internal/ggi_debug.h>
#include "rect.h"
#include "encoding.h"
#include "common.h"

struct tight_ctx_t {
	int reset;
	z_stream zstr[4];
	ggi_vnc_buf work[2];
	struct jpeg_compress_struct cinfo;
	struct jpeg_error_mgr jerr;
	int jpeg_quality;
};

/* compression control */
#define TIGHT_ZTREAM0_RESET (0x01)
#define TIGHT_ZTREAM1_RESET (0x02)
#define TIGHT_ZTREAM2_RESET (0x04)
#define TIGHT_ZTREAM3_RESET (0x08)
#define TIGHT_ZTREAM_RESET  (0x0f)
#define TIGHT_TYPE          (0xf0)
#define TIGHT_ZTREAM_SHIFT  (4)
#define TIGHT_ZTREAM_MASK   (0x30)
#define TIGHT_RAW           (0x00)
#define TIGHT_FILTER        (0x40)
#define TIGHT_SOLID         (0x80)
#define TIGHT_JPEG          (0x90)

/* filters */
#define TIGHT_COPY          (0)
#define TIGHT_PALETTE       (1)
#define TIGHT_GRADIENT      (2)

static void
buf_dest_init_destination(j_compress_ptr cinfo)
{
	struct tight_ctx_t *ctx = cinfo->client_data;

	GGI_vnc_buf_reserve(&ctx->work[0], ctx->work[0].size + 1000);

	cinfo->dest->next_output_byte = &ctx->work[0].buf[ctx->work[0].size];
	cinfo->dest->free_in_buffer = ctx->work[0].limit - ctx->work[0].size;
}

static boolean
buf_dest_empty_output_buffer(j_compress_ptr cinfo)
{
	struct tight_ctx_t *ctx = cinfo->client_data;

	ctx->work[0].size = ctx->work[0].limit;

	GGI_vnc_buf_reserve(&ctx->work[0], ctx->work[0].size + 1000);

	cinfo->dest->next_output_byte = &ctx->work[0].buf[ctx->work[0].size];
	cinfo->dest->free_in_buffer = ctx->work[0].limit - ctx->work[0].size;

	return TRUE;
}

static void
buf_dest_term_destination(j_compress_ptr cinfo)
{
	struct tight_ctx_t *ctx = cinfo->client_data;

	ctx->work[0].size = cinfo->dest->next_output_byte - ctx->work[0].buf;

	DPRINT("jpeg (size %d)\n", ctx->work[0].size);
}

static void
buf_dest(j_compress_ptr cinfo)
{
	struct jpeg_destination_mgr *dest;

	if (cinfo->dest == NULL) {
		cinfo->dest = (struct jpeg_destination_mgr *)
			cinfo->mem->alloc_small((j_common_ptr) cinfo,
				JPOOL_PERMANENT,
				sizeof(*dest));
	}
	dest = cinfo->dest;

	dest->init_destination    = buf_dest_init_destination;
	dest->empty_output_buffer = buf_dest_empty_output_buffer;
	dest->term_destination    = buf_dest_term_destination;

	buf_dest_init_destination(cinfo);
}

static void
zip(ggi_vnc_client *client, int ztream, uint8_t *src, int len)
{
	struct tight_ctx_t *ctx = client->tight_ctx;
	int avail;
	uint32_t done = 0;

	ctx->zstr[ztream].next_in = src;
	ctx->zstr[ztream].avail_in = len;

	avail = ctx->work[1].limit - ctx->work[1].size;

	for (;;) {
		ctx->zstr[ztream].next_out =
			&ctx->work[1].buf[ctx->work[1].size + done];
		ctx->zstr[ztream].avail_out = avail - done;
		deflate(&ctx->zstr[ztream], Z_SYNC_FLUSH);
		done = avail - ctx->zstr[ztream].avail_out;
		if (ctx->zstr[ztream].avail_out)
			break;
		avail += 1000;
		GGI_vnc_buf_reserve(&ctx->work[1], ctx->work[1].size + avail);
	}

	ctx->work[1].size += done;

	DPRINT_MISC("tight %d z %d %d%%\n", len, done, done * 100 / len);
}

static inline int
insert_data_size(uint8_t *dst, uint32_t size)
{
	int count = 1;
	LIB_ASSERT(size < 4194304, "size overflow");

	dst[0] = size;
	if (size >= 128) {
		dst[0] |= 0x80;
		++count;
		dst[1] = size >> 7;
		if (size >= 16384) {
			dst[1] |= 0x80;
			++count;
			dst[2] = size >> 14;
		}
	}

	return count;
}

static inline uint8_t
select_subencoding(int xs, int ys, int cbpp, int colors, int *best)
{
	int bytes;
	int subencoding;

	if (colors == 1) {
		*best = cbpp;
		return TIGHT_SOLID;
	}

	*best = xs * ys * cbpp;
	subencoding = TIGHT_RAW | (0 << TIGHT_ZTREAM_SHIFT);

	/* FIXME: Should investigate if gradient is suitable */
	/* TIGHT_FILTER | TIGHT_GRADIENT | (3 << TIGHT_ZTREAM_SHIFT) */

	/* packed palette */
	if (colors == 2) {
		bytes = 1 + 1 + cbpp * colors + (xs + 7) / 8 * ys;
		if (bytes < *best) {
			*best = bytes;
			subencoding = TIGHT_FILTER | TIGHT_PALETTE
				| (1 << TIGHT_ZTREAM_SHIFT);
		}
	}

	if (cbpp == 1)
		/* 1 bpp palette useless for 1 bpp modes, use raw */
		return subencoding;

	/* too many colors for anything but raw/jpeg/gradient */
	if (!colors) {
		if (cbpp == 3 && xs >= 16 && ys >= 16 && *best > 3000)
			subencoding = TIGHT_JPEG;
		return subencoding;
	}

	/* palette */
	bytes = 1 + 1 + cbpp * colors + xs * ys;
	if (bytes < *best) {
		*best = bytes;
		subencoding = TIGHT_FILTER | TIGHT_PALETTE
			| (2 << TIGHT_ZTREAM_SHIFT);
	}

	return subencoding;
}

static inline uint8_t
scan_8(uint8_t *src,
	int xs, int ys, int stride, uint8_t *palette, int *colors)
{
	int x, y;
	uint8_t last = *src;
	uint8_t here;
	int bytes;
	int c;

	stride -= xs;

	*colors = 1;
	palette[0] = last;

	for (y = 0; y < ys; ++y) {
		for (x = 0; x < xs; ++x) {
			here = *src++;
			if (last == here)
				continue;
			last = here;
			c = palette_match_8(palette, *colors, here);
			if (c != *colors)
				continue;
			if (*colors < 256)
				palette[(*colors)++] = here;
			else {
				*colors = 0;
				y = ys;
				break;
			}
		}
		src += stride;
	}

	return select_subencoding(xs, ys, 1, *colors, &bytes);
}

static inline uint8_t
scan_16(uint8_t *src8,
	int xs, int ys, int stride, uint16_t *palette, int *colors)
{
	int x, y;
	uint16_t *src = (uint16_t *)src8;
	uint16_t last = *src;
	uint16_t here;
	int bytes;
	int c;

	stride -= xs;

	*colors = 1;
	palette[0] = last;

	for (y = 0; y < ys; ++y) {
		for (x = 0; x < xs; ++x) {
			here = *src++;
			if (last == here)
				continue;
			last = here;
			c = palette_match_16(palette, *colors, here);
			if (c != *colors)
				continue;
			if (*colors < 256)
				palette[(*colors)++] = here;
			else {
				*colors = 0;
				y = ys;
				break;
			}
		}
		src += stride;
	}

	return select_subencoding(xs, ys, 2, *colors, &bytes);
}

static inline uint8_t
scan_888(uint8_t *src,
	int xs, int ys, int stride, uint32_t *palette, int *colors)
{
	int x, y;
	uint32_t last = (src[0] << 16) | (src[1] << 8) | src[2];
	uint32_t here;
	int bytes;
	int c;

	stride = (stride - xs) * 3;

	*colors = 1;
	palette[0] = last;

	for (y = 0; y < ys; ++y) {
		for (x = 0; x < xs; ++x) {
			here = *src++ << 16;
			here |= *src++ << 8;
			here |= *src++;
			if (last == here)
				continue;
			last = here;
			c = palette_match_32(palette, *colors, here);
			if (c != *colors)
				continue;
			if (*colors < 256)
				palette[(*colors)++] = here;
			else {
				*colors = 0;
				y = ys;
				break;
			}
		}
		src += stride;
	}

	return select_subencoding(xs, ys, 3, *colors, &bytes);
}

static inline uint8_t
scan_32(uint8_t *src8,
	int xs, int ys, int stride, uint32_t *palette, int *colors)
{
	int x, y;
	uint32_t *src = (uint32_t *)src8;
	uint32_t last = *src;
	uint32_t here;
	int bytes;
	int c;

	stride -= xs;

	*colors = 1;
	palette[0] = last;

	for (y = 0; y < ys; ++y) {
		for (x = 0; x < xs; ++x) {
			here = *src++;
			if (last == here)
				continue;
			last = here;
			c = palette_match_32(palette, *colors, here);
			if (c != *colors)
				continue;
			if (*colors < 256)
				palette[(*colors)++] = here;
			else {
				*colors = 0;
				y = ys;
				break;
			}
		}
		src += stride;
	}

	return select_subencoding(xs, ys, 4, *colors, &bytes);
}

static uint8_t *
raw_8(uint8_t *dst, uint8_t *src, int xs, int ys, int stride)
{
	int y;

	for (y = 0; y < ys; ++y) {
		memcpy(dst, src, xs);
		src += stride;
		dst += xs;
	}

	return dst;
}

static uint8_t *
raw_16(uint8_t *dst, uint8_t *src8, int xs, int ys, int stride, int rev)
{
	uint16_t *src = (uint16_t *)src8;
	int x, y;

	if (!rev) {
		xs *= 2;
		for (y = 0; y < ys; ++y) {
			memcpy(dst, src, xs);
			src += stride;
			dst += xs;
		}
		return dst;
	}

	for (y = 0; y < ys; ++y) {
		for (x = 0; x < xs; ++x)
			dst = insert_rev_16(dst, src[x]);
		src += stride;
	}

	return dst;
}

static uint8_t *
raw_888(uint8_t *dst, uint8_t *src, int xs, int ys, int stride)
{
	int y;

	xs *= 3;
	stride *= 3;

	for (y = 0; y < ys; ++y) {
		memcpy(dst, src, xs);
		src += stride;
		dst += xs;
	}

	return dst;
}

static uint8_t *
raw_32(uint8_t *dst, uint8_t *src8, int xs, int ys, int stride, int rev)
{
	uint32_t *src = (uint32_t *)src8;
	int x, y;

	if (!rev) {
		xs *= 4;
		for (y = 0; y < ys; ++y) {
			memcpy(dst, src, xs);
			src += stride;
			dst += xs;
		}
		return dst;
	}

	for (y = 0; y < ys; ++y) {
		for (x = 0; x < xs; ++x)
			dst = insert_rev_32(dst, src[x]);
		src += stride;
	}

	return dst;
}

static uint8_t *
jpeg_888(struct tight_ctx_t *ctx, uint8_t *src, int xs, int ys, int stride)
{
	int y;

	stride *= 3;

	ctx->cinfo.image_width = xs;
	ctx->cinfo.image_height = ys;
	ctx->cinfo.input_components = 3;
	ctx->cinfo.in_color_space = JCS_RGB;
	jpeg_set_defaults(&ctx->cinfo);
	jpeg_set_quality(&ctx->cinfo, ctx->jpeg_quality, TRUE);
	jpeg_start_compress(&ctx->cinfo, TRUE);

	for (y = 0; y < ys; ++y) {
		jpeg_write_scanlines(&ctx->cinfo, &src, 1);
		src += stride;
	}

	jpeg_finish_compress(&ctx->cinfo);

	return &ctx->work[0].buf[ctx->work[0].size];
}

static inline uint8_t *
packed_palette_8(uint8_t *dst, uint8_t *src,
	int xs, int ys, int stride, uint8_t *palette)
{
	int x, y;

	stride -= xs;

	for (y = 0; y < ys; ++y) {
		int pel = 7;
		*dst = 0;
		for (x = 0; x < xs; ++x) {
			*dst |= palette_match_8(palette, 2, *src++) << pel;
			pel -= 1;
			if (pel < 0) {
				pel = 7;
				*++dst = 0;
			}
		}
		src += stride;
		if (pel != 7)
			++dst;
	}

	return dst;
}

static inline uint8_t *
packed_palette_16(uint8_t *dst, uint8_t *src8,
	int xs, int ys, int stride, uint16_t *palette)
{
	uint16_t *src = (uint16_t *)src8;
	int x, y;

	stride -= xs;

	for (y = 0; y < ys; ++y) {
		int pel = 7;
		*dst = 0;
		for (x = 0; x < xs; ++x) {
			*dst |= palette_match_16(palette, 2, *src++) << pel;
			pel -= 1;
			if (pel < 0) {
				pel = 7;
				*++dst = 0;
			}
		}
		src += stride;
		if (pel != 7)
			++dst;
	}

	return dst;
}

static inline uint8_t *
packed_palette_888(uint8_t *dst, uint8_t *src,
	int xs, int ys, int stride, uint32_t *palette)
{
	int x, y;
	uint32_t pixel;

	stride = (stride - xs) * 3;

	for (y = 0; y < ys; ++y) {
		int pel = 7;
		*dst = 0;
		for (x = 0; x < xs; ++x) {
			pixel = *src++ << 16;
			pixel |= *src++ << 8;
			pixel |= *src++;
			*dst |= palette_match_32(palette, 2, pixel) << pel;
			pel -= 1;
			if (pel < 0) {
				pel = 7;
				*++dst = 0;
			}
		}
		src += stride;
		if (pel != 7)
			++dst;
	}

	return dst;
}

static inline uint8_t *
packed_palette_32(uint8_t *dst, uint8_t *src8,
	int xs, int ys, int stride, uint32_t *palette)
{
	uint32_t *src = (uint32_t *)src8;
	int x, y;

	stride -= xs;

	for (y = 0; y < ys; ++y) {
		int pel = 7;
		*dst = 0;
		for (x = 0; x < xs; ++x) {
			*dst |= palette_match_32(palette, 2, *src++) << pel;
			pel -= 1;
			if (pel < 0) {
				pel = 7;
				*++dst = 0;
			}
		}
		src += stride;
		if (pel != 7)
			++dst;
	}

	return dst;
}

static inline uint8_t *
palette_16(uint8_t *dst, uint8_t *src8,
	int xs, int ys, int stride, uint16_t *palette)
{
	uint16_t *src = (uint16_t *)src8;
	int x, y;

	stride -= xs;

	for (y = 0; y < ys; ++y) {
		for (x = 0; x < xs; ++x)
			*dst++ = palette_match_16(palette, 256, *src++);
		src += stride;
	}

	return dst;
}

static inline uint8_t *
palette_888(uint8_t *dst, uint8_t *src,
	int xs, int ys, int stride, uint32_t *palette)
{
	int x, y;
	uint32_t pixel;

	stride = (stride - xs) * 3;

	for (y = 0; y < ys; ++y) {
		*dst = 0;
		for (x = 0; x < xs; ++x) {
			pixel = *src++ << 16;
			pixel |= *src++ << 8;
			pixel |= *src++;
			*dst++ = palette_match_32(palette, 256, pixel);
		}
		src += stride;
	}

	return dst;
}

static inline uint8_t *
palette_32(uint8_t *dst, uint8_t *src8,
	int xs, int ys, int stride, uint32_t *palette)
{
	uint32_t *src = (uint32_t *)src8;
	int x, y;

	stride -= xs;

	for (y = 0; y < ys; ++y) {
		for (x = 0; x < xs; ++x)
			*dst++ = palette_match_32(palette, 256, *src++);
		src += stride;
	}

	return dst;
}

static inline uint8_t
bound_888(int value)
{
	if (value < 0)
		return 0;
	if (value > 255)
		return 255;
	return value;
}

static inline uint8_t *
gradient_888(uint8_t *dst, uint8_t *src, int xs, int ys, int stride)
{
	int x, y;
	uint8_t *prev;

	xs *= 3;

	/* first row */
	/* first pixel */
	for (x = 0; x < 3; ++x)
		dst[x] = src[x];

	/* rest of row */
	for (; x < xs; ++x)
		dst[x] = src[x] - bound_888(src[x - 3]);

	dst += xs;
	prev = src;
	src += stride;

	/* following rows */
	for (y = 1; y < ys; ++y) {
		/* first pixel */
		for (x = 0; x < 3; ++x)
			dst[x] = src[x] - bound_888(prev[x]);

		/* rest of row */
		for (; x < xs; ++x)
			dst[x] = src[x] -
				bound_888(src[x - 3] + prev[x] - prev[x - 3]);

		dst += xs;
		prev = src;
		src += stride;
	}

	return dst;
}

static void
tile_8(struct tight_ctx_t *ctx, uint8_t **buf,
	uint8_t *src, int xs, int ys, int stride)
{
	uint8_t *dst = *buf;
	uint8_t palette[256];
	int colors;
	uint8_t subencoding;
	uint8_t filter;
	int c;

	subencoding = scan_8(src, xs, ys, stride, palette, &colors);

	filter = subencoding & ~TIGHT_TYPE;
	subencoding &= TIGHT_TYPE;

	*dst++ = subencoding | ctx->reset;
	ctx->reset = 0;

	if (subencoding == TIGHT_RAW) {
		*buf = raw_8(dst, src, xs, ys, stride);
		return;
	}

	if (subencoding == TIGHT_SOLID) {
		*dst++ = palette[0];
		*buf = dst;
		return;
	}

	/* subencoding & TIGHT_FILTER */

	*dst++ = filter;

	/* filter == TIGHT_PALETTE */

	*dst++ = colors - 1;
	for (c = 0; c < colors; ++c)
		*dst++ = palette[c];

	/* colors == 2 */
	*buf = packed_palette_8(dst, src, xs, ys, stride, palette);
}

static void
tile_16(struct tight_ctx_t *ctx, uint8_t **buf,
	uint8_t *src, int xs, int ys, int stride, int rev)
{
	uint8_t *dst;
	uint16_t palette[256];
	int colors;
	uint8_t subencoding;
	uint8_t filter;
	int c;

	subencoding = scan_16(src, xs, ys, stride, palette, &colors);

	filter = subencoding & ~TIGHT_TYPE;
	subencoding &= TIGHT_TYPE;
	ctx->work[0].buf[ctx->work[0].size++] = subencoding | ctx->reset;
	ctx->reset = 0;

	*buf = &ctx->work[0].buf[ctx->work[0].size];
	dst = *buf;

	/* TODO
	if (subencoding == TIGHT_JPEG) {
		*buf = jpeg_16(ctx, src, xs, ys, stride);
		return;
	}
	*/

	if (subencoding == TIGHT_RAW) {
		*buf = raw_16(dst, src, xs, ys, stride, rev);
		return;
	}

	if (subencoding == TIGHT_SOLID) {
		if (rev)
			*buf = insert_rev_16(dst, palette[0]);
		else
			*buf = insert_16(dst, palette[0]);
		return;
	}

	/* subencoding & TIGHT_FILTER */

	*dst++ = filter;

	/* TODO
	if (filter == TIGHT_GRADIENT) {
		*buf = gradient_16(dst, src, xs, ys, stride, rev);
		return;
	}
	*/

	/* filter == TIGHT_PALETTE */

	*dst++ = colors - 1;
	if (rev)
		for (c = 0; c < colors; ++c)
			dst = insert_rev_16(dst, palette[c]);
	else
		for (c = 0; c < colors; ++c)
			dst = insert_16(dst, palette[c]);

	if (colors == 2)
		*buf = packed_palette_16(dst, src, xs, ys, stride, palette);
	else
		*buf = palette_16(dst, src, xs, ys, stride, palette);
}

static void
tile_888(struct tight_ctx_t *ctx, uint8_t **buf,
	uint8_t *src, int xs, int ys, int stride)
{
	uint8_t *dst;
	uint32_t palette[256];
	int colors;
	uint8_t subencoding;
	uint8_t filter;
	int c;

	subencoding = scan_888(src, xs, ys, stride, palette, &colors);

	filter = subencoding & ~TIGHT_TYPE;
	subencoding &= TIGHT_TYPE;
	ctx->work[0].buf[ctx->work[0].size++] = subencoding | ctx->reset;
	ctx->reset = 0;

	*buf = &ctx->work[0].buf[ctx->work[0].size];
	dst = *buf;

	if (subencoding == TIGHT_JPEG) {
		*buf = jpeg_888(ctx, src, xs, ys, stride);
		return;
	}

	if (subencoding == TIGHT_RAW) {
		*buf = raw_888(dst, src, xs, ys, stride);
		return;
	}

	if (subencoding == TIGHT_SOLID) {
		*buf = insert_888(dst, palette[0]);
		return;
	}

	/* subencoding & TIGHT_FILTER */

	*dst++ = filter;

	if (filter == TIGHT_GRADIENT) {
		*buf = gradient_888(dst, src, xs, ys, stride);
		return;
	}

	/* filter == TIGHT_PALETTE */

	*dst++ = colors - 1;
	for (c = 0; c < colors; ++c)
		dst = insert_888(dst, palette[c]);

	if (colors == 2)
		*buf = packed_palette_888(dst, src, xs, ys, stride, palette);
	else
		*buf = palette_888(dst, src, xs, ys, stride, palette);
}

static void
tile_32(struct tight_ctx_t *ctx, uint8_t **buf,
	uint8_t *src, int xs, int ys, int stride, int rev)
{
	uint8_t *dst;
	uint32_t palette[256];
	int colors;
	uint8_t subencoding;
	uint8_t filter;
	int c;

	subencoding = scan_32(src, xs, ys, stride, palette, &colors);

	filter = subencoding & ~TIGHT_TYPE;
	subencoding &= TIGHT_TYPE;
	ctx->work[0].buf[ctx->work[0].size++] = subencoding | ctx->reset;
	ctx->reset = 0;

	*buf = &ctx->work[0].buf[ctx->work[0].size];
	dst = *buf;

	/* TODO
	if (subencoding == TIGHT_JPEG) {
		*buf = jpeg_32(ctx, src, xs, ys, stride);
		return;
	}
	*/

	if (subencoding == TIGHT_RAW) {
		*buf = raw_32(dst, src, xs, ys, stride, rev);
		return;
	}

	if (subencoding == TIGHT_SOLID) {
		if (rev)
			*buf = insert_rev_32(dst, palette[0]);
		else
			*buf = insert_32(dst, palette[0]);
		return;
	}

	/* subencoding & TIGHT_FILTER */

	*dst++ = filter;

	/* TODO
	if (filter == TIGHT_GRADIENT) {
		*buf = gradient_32(dst, src, xs, ys, stride, rev);
		return;
	}
	*/

	/* filter == TIGHT_PALETTE */

	*dst++ = colors - 1;
	if (rev)
		for (c = 0; c < colors; ++c)
			dst = insert_rev_32(dst, palette[c]);
	else
		for (c = 0; c < colors; ++c)
			dst = insert_32(dst, palette[c]);

	if (colors == 2)
		*buf = packed_palette_32(dst, src, xs, ys, stride, palette);
	else
		*buf = palette_32(dst, src, xs, ys, stride, palette);
}

static int
tile(ggi_vnc_client *client, struct ggi_visual *cvis,
	const ggi_directbuffer *db, ggi_rect *update)
{
	struct tight_ctx_t *ctx = client->tight_ctx;
	ggi_graphtype gt;
	int bpp;
	int count;
	unsigned char *buf;
	unsigned char *header;
	int stride;
	int work_buf;
	int want_size;

	DPRINT("tile %dx%d - %dx%d\n",
		update->tl.x, update->tl.y,
		update->br.x, update->br.y);

	gt = LIBGGI_GT(cvis);

	bpp = GT_ByPP(gt);
	count = ggi_rect_width(update) * ggi_rect_height(update);
	GGI_vnc_buf_reserve(&client->wbuf,
		client->wbuf.size + 12 + 3 + 256 * bpp);

	header = &client->wbuf.buf[client->wbuf.size];
	insert_header(header, &update->tl, update, 7); /* tight */
	client->wbuf.size += 12;

	GGI_vnc_buf_reserve(&ctx->work[0], 2 + count * bpp);
	GGI_vnc_buf_reserve(&ctx->work[0], 3 + 256 * bpp + count);
	ctx->work[0].size = ctx->work[0].pos = 0;
	ctx->work[1].size = ctx->work[1].pos = 0;
	buf = ctx->work[0].buf;

	stride = LIBGGI_VIRTX(cvis);

	if (bpp == 1)
		tile_8(ctx, &buf, (uint8_t *)db->read +
			(update->tl.y * stride + update->tl.x) * bpp,
			ggi_rect_width(update), ggi_rect_height(update),
			stride);
	else if (bpp == 2)
		tile_16(ctx, &buf, (uint8_t *)db->read +
			(update->tl.y * stride + update->tl.x) * bpp,
			ggi_rect_width(update), ggi_rect_height(update),
			stride, client->reverse_endian);
	else if (bpp == 3)
		tile_888(ctx, &buf, (uint8_t *)db->read +
			(update->tl.y * stride + update->tl.x) * bpp,
			ggi_rect_width(update), ggi_rect_height(update),
			stride);
	else
		tile_32(ctx, &buf, (uint8_t *)db->read +
			(update->tl.y * stride + update->tl.x) * bpp,
			ggi_rect_width(update), ggi_rect_height(update),
			stride, client->reverse_endian);

	ctx->work[0].size = buf - ctx->work[0].buf;

	LIB_ASSERT(ctx->work[0].size <= ctx->work[0].limit,
		"buffer overrun");

	client->wbuf.buf[client->wbuf.size++] = ctx->work[0].buf[0];
	++ctx->work[0].pos;

	work_buf = 0;
	want_size = 3;
	if ((ctx->work[0].buf[0] & TIGHT_TYPE) != TIGHT_JPEG) {
		if (ctx->work[0].buf[0] & TIGHT_FILTER) {
			int pal_size;

			/* copy filter type */
			client->wbuf.buf[client->wbuf.size++] =
				 ctx->work[0].buf[1];
			++ctx->work[0].pos;

			if (ctx->work[0].buf[1] == TIGHT_PALETTE) {
				/* palette filter, copy palette */
				pal_size = 1 + bpp * (ctx->work[0].buf[2] + 1);
				memcpy(&client->wbuf.buf[client->wbuf.size],
					&ctx->work[0].buf[2], pal_size);
				client->wbuf.size += pal_size;
				ctx->work[0].pos += pal_size;
			}
		}
		if (ctx->work[0].size - ctx->work[0].pos < 12)
			want_size = 0;
		else {
			GGI_vnc_buf_reserve(&ctx->work[1], 1000);
			zip(client,
				(ctx->work[0].buf[0] & TIGHT_ZTREAM_MASK) >>
					TIGHT_ZTREAM_SHIFT,
				&ctx->work[0].buf[ctx->work[0].pos],
				ctx->work[0].size - ctx->work[0].pos);
			work_buf = 1;
		}
	}

	GGI_vnc_buf_reserve(&client->wbuf,
		client->wbuf.size + want_size +
		ctx->work[work_buf].size - ctx->work[work_buf].pos);
	if (want_size) 
		client->wbuf.size += insert_data_size(
			&client->wbuf.buf[client->wbuf.size],
			ctx->work[work_buf].size - ctx->work[work_buf].pos);
	memcpy(&client->wbuf.buf[client->wbuf.size],
		&ctx->work[work_buf].buf[ctx->work[work_buf].pos],
		ctx->work[work_buf].size - ctx->work[work_buf].pos);
	client->wbuf.size +=
		ctx->work[work_buf].size - ctx->work[work_buf].pos;

	return ggi_rect_height(update);
}

#define MAX_HEIGHT 48
#define MAX_WIDTH  48

static inline int
column(ggi_vnc_client *client, struct ggi_visual *cvis,
	const ggi_directbuffer *db, ggi_rect *update)
{
	ggi_rect row_update = *update;
	int y;
	int vnc_rects = 0;

	DPRINT("column %dx%d - %dx%d\n",
		update->tl.x, update->tl.y,
		update->br.x, update->br.y);

	for (y = update->tl.y; y < update->br.y; y += MAX_HEIGHT) {
		row_update.tl.y = y;
		if (y + MAX_HEIGHT > update->br.y)
			row_update.br.y = update->br.y;
		else
			row_update.br.y = y + MAX_HEIGHT;
		tile(client, cvis, db, &row_update);
		++vnc_rects;
	}

	return vnc_rects;
}

int
GGI_vnc_tight(ggi_vnc_client *client, ggi_rect *update)
{
	struct ggi_visual *vis = client->owner;
	ggi_vnc_priv *priv = VNC_PRIV(vis);
	struct ggi_visual *cvis;
	const ggi_directbuffer *db;
	ggi_rect vupdate;
	ggi_rect col_update;
	int d_frame_num;
	int vnc_rects = 0;
	int x;

	DPRINT("tight update %dx%d - %dx%d\n",
		update->tl.x, update->tl.y,
		update->br.x, update->br.y);

	vupdate = *update;
	ggi_rect_shift_xy(&vupdate, vis->origin_x, vis->origin_y);

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

	db = ggiDBGetBuffer(cvis->stem, d_frame_num);
	ggiResourceAcquire(db->resource, GGI_ACTYPE_READ);

	DPRINT("vupdate %dx%d - %dx%d\n",
		vupdate.tl.x, vupdate.tl.y,
		vupdate.br.x, vupdate.br.y);
	col_update = vupdate;
	for (x = vupdate.tl.x; x < vupdate.br.x; x += MAX_WIDTH) {
		col_update.tl.x = x;
		if (x + MAX_WIDTH > vupdate.br.x)
			col_update.br.x = vupdate.br.x;
		else
			col_update.br.x = x + MAX_WIDTH;
		vnc_rects += column(client, cvis, db, &col_update);
	}

	ggiResourceRelease(db->resource);

	return vnc_rects;
}

void
GGI_vnc_tight_quality(struct tight_ctx_t *ctx, int quality)
{
	ctx->jpeg_quality = 5 + 10 * quality;
	DPRINT("jpeg quality %d\n", ctx->jpeg_quality);
}

struct tight_ctx_t *
GGI_vnc_tight_open(void)
{
	struct tight_ctx_t *ctx = malloc(sizeof(*ctx));
	int i;

	memset(ctx, 0, sizeof(*ctx));

	ctx->reset = TIGHT_ZTREAM_RESET;
	ctx->jpeg_quality = 65;

	for (i = 0; i < 4; ++i) {
		ctx->zstr[i].zalloc = Z_NULL;
		ctx->zstr[i].zfree = Z_NULL;
		ctx->zstr[i].opaque = Z_NULL;
		deflateInit(&ctx->zstr[i], Z_DEFAULT_COMPRESSION);
	}

	ctx->cinfo.client_data = ctx;
	ctx->cinfo.err = jpeg_std_error(&ctx->jerr);

	jpeg_create_compress(&ctx->cinfo);
	buf_dest(&ctx->cinfo);

	return ctx;
}

void
GGI_vnc_tight_close(struct tight_ctx_t *ctx)
{
	jpeg_destroy_compress(&ctx->cinfo);
	free(ctx->work[0].buf);
	free(ctx->work[1].buf);
	deflateEnd(&ctx->zstr[3]);
	deflateEnd(&ctx->zstr[2]);
	deflateEnd(&ctx->zstr[1]);
	deflateEnd(&ctx->zstr[0]);
	free(ctx);
}
