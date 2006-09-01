/* $Id: zrle.c,v 1.3 2006/09/01 05:09:09 pekberg Exp $
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


typedef void (tile_func)(uint8_t **buf, uint8_t *src,
	int xs, int ys, int stride, int bpp);

static void
zip(ggi_vnc_priv *priv, uint8_t *src, int len)
{
	zrle_ctx_t *ctx = priv->zrle_ctx;
	int start = priv->wbuf.size;
	int avail;
	uint32_t *zlen;
	int done = 0;

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

	DPRINT_MISC("raw %d z %d %d%%\n", len, done, done * 100 / len);
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

void
GGI_vnc_zrle(struct ggi_visual *vis, ggi_rect *update)
{
	ggi_vnc_priv *priv = VNC_PRIV(vis);
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

	if (priv->palette_dirty) {
		unsigned char *vnc_palette;
		unsigned char *dst;
		int colors = 1 << GT_DEPTH(gt);
		ggi_color *ggi_palette;
		int i;

		ggi_palette = malloc(colors * sizeof(*ggi_palette));
		ggiGetPalette(priv->client_vis->stem, 0, colors, ggi_palette);

		GGI_vnc_buf_reserve(&priv->wbuf, 6 + 6 * colors);
		priv->wbuf.size += 6 + 6 * colors;
		vnc_palette = priv->wbuf.buf;
		vnc_palette[0] = 1;
		vnc_palette[1] = 0;
		vnc_palette[2] = 0;
		vnc_palette[3] = 0;
		vnc_palette[4] = colors >> 8;
		vnc_palette[5] = colors & 0xff;

		dst = &vnc_palette[6];
		for (i = 0; i < colors; ++i) {
			*dst++ = ggi_palette[i].r >> 8;
			*dst++ = ggi_palette[i].r & 0xff;
			*dst++ = ggi_palette[i].g >> 8;
			*dst++ = ggi_palette[i].g & 0xff;
			*dst++ = ggi_palette[i].b >> 8;
			*dst++ = ggi_palette[i].b & 0xff;
		}

		free(ggi_palette);
		priv->palette_dirty = 0;
	}

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
	work = malloc(xtiles * ytiles + count * cbpp);
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
	header[10] = (update->br.y - update->tl.y) >> 8;;
	header[11] = (update->br.y - update->tl.y) & 0xff;
	header[12] = 0;
	header[13] = 0;
	header[14] = 0;
	header[15] = 16; /* zrle */

	priv->wbuf.size += 16;
	buf = work;

	db = ggiDBGetBuffer(cvis->stem, 0);
	ggiResourceAcquire(db->resource, GGI_ACTYPE_READ);

	if (priv->reverse_endian && bpp != cbpp) {
		lower_or_bpp = lower;
		tile = do_ctile_rev;
	}
	else if (priv->reverse_endian && GT_SIZE(gt) > 8) {
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

zrle_ctx_t *
GGI_vnc_zrle_open(int level)
{
	zrle_ctx_t *ctx = malloc(sizeof(*ctx));

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
GGI_vnc_zrle_close(zrle_ctx_t *ctx)
{
	deflateEnd(&ctx->zstr);
	free(ctx);
}
