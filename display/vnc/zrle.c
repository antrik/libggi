/* $Id: zrle.c,v 1.11 2006/09/02 00:12:23 pekberg Exp $
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

typedef void (tile_func)(uint8_t **buf, uint8_t *src,
	int xs, int ys, int stride, int bpp);

static void
zip(ggi_vnc_priv *priv, uint8_t *src, int len)
{
	struct zrle_ctx_t *ctx = priv->zrle_ctx;
	int start = priv->wbuf.size;
	int avail;
	uint32_t *zlen;
	uint32_t done = 0;

	ctx->zstr.next_in = src;
	ctx->zstr.avail_in = len;

	priv->wbuf.size += 4;
	
	avail = priv->wbuf.limit - priv->wbuf.size;

	for (;;) {
		ctx->zstr.next_out = &priv->wbuf.buf[priv->wbuf.size + done];
		ctx->zstr.avail_out = avail - done;
		deflate(&ctx->zstr, Z_SYNC_FLUSH);
		done = avail - ctx->zstr.avail_out;
		if (ctx->zstr.avail_out)
			break;
		avail += 1000;
		GGI_vnc_buf_reserve(&priv->wbuf, priv->wbuf.size + avail);
	}

	zlen = (uint32_t *)&priv->wbuf.buf[start];
	*zlen = GGI_HTONL(done);
	priv->wbuf.size += done;

	DPRINT_MISC("rle %d z %d %d%%\n", len, done, done * 100 / len);
}

static void
do_tile(uint8_t **buf, uint8_t *src,
	int xs, int ys, int stride, int bpp)
{
	int y;
	uint8_t *dst = *buf;

	*dst++ = 0; /* raw */
	for (y = 0; y < ys; ++y) {
		memcpy(dst, src, xs * bpp);
		src += stride * bpp;
		dst += xs * bpp;
	}

	*buf = dst;
}

static void
do_ctile(uint8_t **buf, uint8_t *src,
	int xs, int ys, int stride, int lower)
{
	int x, y;
	uint8_t *dst = *buf;

	*dst++ = 0; /* raw */

	if (lower) {
		uint32_t *src32 = (uint32_t *)src;
		for (y = 0; y < ys; ++y) {
			for (x = 0; x < xs; ++x) {
#ifdef GGI_BIG_ENDIAN
				*dst++ = src32[x] >> 16;
				*dst++ = src32[x] >> 8;
				*dst++ = src32[x];
#else
				*dst++ = src32[x];
				*dst++ = src32[x] >> 8;
				*dst++ = src32[x] >> 16;
#endif
			}
			src32 += stride;
		}
	}
	else {
		uint32_t *src32 = (uint32_t *)src;
		for (y = 0; y < ys; ++y) {
			for (x = 0; x < xs; ++x) {
#ifdef GGI_BIG_ENDIAN
				*dst++ = src32[x] >> 24;
				*dst++ = src32[x] >> 16;
				*dst++ = src32[x] >> 8;
#else
				*dst++ = src32[x] >> 8;
				*dst++ = src32[x] >> 16;
				*dst++ = src32[x] >> 24;
#endif
			}
			src32 += stride;
		}
	}

	*buf = dst;
}

static void
do_tile_rev(uint8_t **buf, uint8_t *src,
	int xs, int ys, int stride, int bpp)
{
	int x, y;
	uint8_t *dst = *buf;

	*dst++ = 0; /* raw */

	if (bpp == 2) {
		uint16_t *src16 = (uint16_t *)src;
		for (y = 0; y < ys; ++y) {
			for (x = 0; x < xs; ++x) {
#ifdef GGI_BIG_ENDIAN
				*dst++ = src16[x];
				*dst++ = src16[x] >> 8;
#else
				*dst++ = src16[x] >> 8;
				*dst++ = src16[x];
#endif
			}
			src16 += stride;
		}
	}
	else {
		uint32_t *src32 = (uint32_t *)src;
		for (y = 0; y < ys; ++y) {
			for (x = 0; x < xs; ++x) {
#ifdef GGI_BIG_ENDIAN
				*dst++ = src32[x];
				*dst++ = src32[x] >> 8;
				*dst++ = src32[x] >> 16;
				*dst++ = src32[x] >> 24;
#else
				*dst++ = src32[x] >> 24;
				*dst++ = src32[x] >> 16;
				*dst++ = src32[x] >> 8;
				*dst++ = src32[x];
#endif
			}
			src32 += stride;
		}
	}

	*buf = dst;
}

static void
do_ctile_rev(uint8_t **buf, uint8_t *src,
	int xs, int ys, int stride, int lower)
{
	int x, y;
	uint8_t *dst = *buf;

	*dst++ = 0; /* raw */

	if (lower) {
		uint32_t *src32 = (uint32_t *)src;
		for (y = 0; y < ys; ++y) {
			for (x = 0; x < xs; ++x) {
#ifdef GGI_BIG_ENDIAN
				*dst++ = src32[x];
				*dst++ = src32[x] >> 8;
				*dst++ = src32[x] >> 16;
#else
				*dst++ = src32[x] >> 16;
				*dst++ = src32[x] >> 8;
				*dst++ = src32[x];
#endif
			}
			src32 += stride;
		}
	}
	else {
		uint32_t *src32 = (uint32_t *)src;
		for (y = 0; y < ys; ++y) {
			for (x = 0; x < xs; ++x) {
#ifdef GGI_BIG_ENDIAN
				*dst++ = src32[x] >> 8;
				*dst++ = src32[x] >> 16;
				*dst++ = src32[x] >> 24;
#else
				*dst++ = src32[x] >> 24;
				*dst++ = src32[x] >> 16;
				*dst++ = src32[x] >> 8;
#endif
			}
			src32 += stride;
		}
	}

	*buf = dst;
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
		return 1;
	}

	/* raw */
	*best = xs * ys;
	subencoding = 0;

	/* palette rle */
	if (2 <= colors && colors <= 127) {
		bytes = cbpp * colors + single + 9 * multi / 4;
		if (bytes < *best) {
			*best = bytes;
			subencoding = 128 + colors;
		}
	}

	/* plain rle */
	bytes = (1 + cbpp) * single + cbpp * multi + 9 * multi / 4;
	if (bytes < *best) {
		*best = bytes;
		subencoding = 128;
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
		subencoding = colors;
	}

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

static inline uint8_t
palette8_match(uint8_t *palette, int colors, uint8_t color)
{
	int c;

	for (c = 0; c < colors; ++c) {
		if (palette[c] == color)
			break;
	}

	return c;
}

static inline uint8_t *
insert8_palrle_rl(uint8_t *dst,
	uint8_t *palette, int colors, uint8_t color, int rl)
{
	uint8_t c = palette8_match(palette, colors, color);
	if (!rl)
		*dst++ = c;
	else {
		*dst++ = 128 + c;
		dst = insert_rl(dst, rl);
	}
	return dst;
}

static void
do_tile8(uint8_t **buf, uint8_t *src, int xs, int ys, int stride, int bpp)
{
	uint8_t palette[128];
	int colors = 1;
	int single = 0;
	int multi = 0;
	int rl = 0;
	uint8_t here;
	uint8_t last = *src;
	int x, y;
	uint8_t *scan = src;
	palette[0] = *scan;
	int c;
	int subencoding;
	int bytes;

	uint8_t *dst = *buf;

	for (y = 0; y < ys; ++y) {
		for (x = 0; x < xs; ++x) {
			here = *scan++;
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
			c = palette8_match(palette, colors, here);
			if (c == colors)
				palette[colors++] = here;
		}
		scan += stride - xs;
	}
	if (rl == 1)
		++single;
	else
		++multi;

	*dst++ = subencoding = select_subencoding(
		xs, ys, 1, colors, single, multi, &bytes);

	if (subencoding == 0) {
		/* raw */
		for (y = 0; y < ys; ++y) {
			memcpy(dst, src, xs);
			src += stride;
			dst += xs;
		}
		goto done;
	}

	if (subencoding == 128) {
		/* plain rle */
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
			src += stride - xs;
		}
		*dst++ = last;
		while (rl > 254) {
			*dst++ = 255;
			rl -= 255;
		}
		*dst++ = rl;
		goto done;
	}

	/* palettized subencodings follows */
	memcpy(dst, palette, colors);
	dst += colors;

	if (subencoding == 1)
		/* solid */
		goto done;

	if (subencoding >= 130) {
		/* palette rle */
		rl = -1;
		last = *src;
		for (y = 0; y < ys; ++y) {
			for (x = 0; x < xs; ++x) {
				here = *src++;
				if (last == here) {
					++rl;
					continue;
				}
				dst = insert8_palrle_rl(
					dst, palette, colors, last, rl);
				last = here;
				rl = 0;
			}
			src += stride - xs;
		}
		dst = insert8_palrle_rl(dst, palette, colors, last, rl);
		goto done;
	}

	if (subencoding == 2) {
		/* packed palette */
		for (y = 0; y < ys; ++y) {
			int pel = 7;
			*dst = 0;
			for (x = 0; x < xs; ++x) {
				*dst |= palette8_match(palette, colors, *src++) << pel;
				if (--pel < 0) {
					pel = 7;
					*++dst = 0;
				}
			}
			src += stride - xs;
			if (xs & 7)
				++dst;
		}
		goto done;
	}

	if (subencoding <= 4) {
		/* packed palette */
		for (y = 0; y < ys; ++y) {
			int pel = 6;
			*dst = 0;
			for (x = 0; x < xs; ++x) {
				*dst |= palette8_match(palette, colors, *src++) << pel;
				pel -= 2;
				if (pel < 0) {
					pel = 6;
					*++dst = 0;
				}
			}
			src += stride - xs;
			if (xs & 3)
				++dst;
		}
		goto done;
	}

	if (subencoding <= 16) {
		/* packed palette */
		for (y = 0; y < ys; ++y) {
			int pel = 4;
			*dst = 0;
			for (x = 0; x < xs; ++x) {
				*dst |= palette8_match(palette, colors, *src++) << pel;
				pel -= 4;
				if (pel < 0) {
					pel = 4;
					*++dst = 0;
				}
			}
			src += stride - xs;
			if (xs & 1)
				++dst;
		}
		goto done;
	}

done:
	*buf = dst;
}

void
GGI_vnc_zrle(struct ggi_visual *vis, ggi_rect *update)
{
	ggi_vnc_priv *priv = VNC_PRIV(vis);
	struct zrle_ctx_t *ctx = priv->zrle_ctx;
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
	int lower_or_bpp;
	tile_func *tile;
	int xt, yt;
	int xs, ys;
	int xs_last, ys_last;
	int stride;

	DPRINT("update %dx%d - %dx%d\n",
		update->tl.x, update->tl.y,
		update->br.x, update->br.y);

	ggi_rect_subtract(&priv->dirty, update);
	priv->update.tl.x = priv->update.br.x = 0;

	DPRINT("dirty %dx%d - %dx%d\n",
		priv->dirty.tl.x, priv->dirty.tl.y,
		priv->dirty.br.x, priv->dirty.br.y);

	if (!priv->client_vis)
		cvis = priv->fb;
	else {
		cvis = priv->client_vis;
		_ggiCrossBlit(priv->fb,
			update->tl.x, update->tl.y,
			update->br.x - update->tl.x,
			update->br.y - update->tl.y,
			cvis,
			update->tl.x, update->tl.y);
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
	count = (update->br.x - update->tl.x) *
		(update->br.y - update->tl.y);
	xtiles = (update->br.x - update->tl.x + 63) / 64;
	ytiles = (update->br.y - update->tl.y + 63) / 64;
	GGI_vnc_buf_reserve(&ctx->work, xtiles * ytiles + count * cbpp);
	work = ctx->work.buf;
	GGI_vnc_buf_reserve(&priv->wbuf, priv->wbuf.size + 20);
	header = &priv->wbuf.buf[priv->wbuf.size];
	header[ 0] = 0;
	header[ 1] = 0;
	header[ 2] = 0;
	header[ 3] = 1;
	header[ 4] = update->tl.x >> 8;
	header[ 5] = update->tl.x & 0xff;
	header[ 6] = update->tl.y >> 8;
	header[ 7] = update->tl.y & 0xff;
	header[ 8] = (update->br.x - update->tl.x) >> 8;
	header[ 9] = (update->br.x - update->tl.x) & 0xff;
	header[10] = (update->br.y - update->tl.y) >> 8;
	header[11] = (update->br.y - update->tl.y) & 0xff;
	header[12] = 0;
	header[13] = 0;
	header[14] = 0;
	header[15] = 16; /* zrle */

	priv->wbuf.size += 16;
	buf = work;

	db = ggiDBGetBuffer(cvis->stem, 0);
	ggiResourceAcquire(db->resource, GGI_ACTYPE_READ);

	if (bpp == 1) {
		lower_or_bpp = bpp;
		tile = do_tile8;
	}
	else if (priv->reverse_endian && bpp != cbpp) {
		lower_or_bpp = lower;
		tile = do_ctile_rev;
	}
	else if (priv->reverse_endian) {
		lower_or_bpp = bpp;
		tile = do_tile_rev;
	}
	else if (bpp == cbpp) {
		lower_or_bpp = bpp;
		tile = do_tile;
	}
	else {
		lower_or_bpp = lower;
		tile = do_ctile;
	}
	
	stride = LIBGGI_VIRTX(cvis);

	ys_last = (update->br.y - update->tl.y) & 0x3f;
	if (!ys_last)
		ys_last = 64;
	xs_last = (update->br.x - update->tl.x) & 0x3f;
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
				((update->tl.y + 64 * yt) * stride +
				 update->tl.x + 64 * xt) * bpp,
				xs, ys, stride, lower_or_bpp);
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
