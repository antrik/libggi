/* $Id: tight.c,v 1.2 2006/09/20 08:02:28 pekberg Exp $
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
	z_stream zstr;
	ggi_vnc_buf work[2];
};

/* compression control */
#define TIGHT_ZTREAM0_RESET (0x01)
#define TIGHT_ZTREAM1_RESET (0x02)
#define TIGHT_ZTREAM2_RESET (0x04)
#define TIGHT_ZTREAM3_RESET (0x08)
#define TIGHT_ZTREAM_RESET  (0x0f)
#define TIGHT_ZTREAM_SHIFT  (4)
#define TIGHT_FILTER        (0x40)
#define TIGHT_SOLID         (0x80)
#define TIGHT_JPEG          (0x90)

/* filters */
#define TIGHT_COPY          (0)
#define TIGHT_PALETTE       (1)
#define TIGHT_GRADIENT      (2)

static void
zip(ggi_vnc_client *client, uint8_t *src, int len)
{
	struct tight_ctx_t *ctx = client->tight_ctx;
	int avail;
	uint32_t done = 0;

	ctx->zstr.next_in = src;
	ctx->zstr.avail_in = len;

	avail = ctx->work[1].limit - ctx->work[1].size;

	for (;;) {
		ctx->zstr.next_out =
			&ctx->work[1].buf[ctx->work[1].size + done];
		ctx->zstr.avail_out = avail - done;
		deflate(&ctx->zstr, Z_SYNC_FLUSH);
		done = avail - ctx->zstr.avail_out;
		if (ctx->zstr.avail_out)
			break;
		avail += 1000;
		++ctx->work[1].size;
		GGI_vnc_buf_reserve(&ctx->work[1], ctx->work[1].size + avail);
		--ctx->work[1].size;
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

static void
tile_8(struct tight_ctx_t *ctx, uint8_t **buf,
	uint8_t *src, int xs, int ys, int stride)
{
	uint8_t *dst = *buf;
	int y;

	/* FIXME: retarded */
	*dst++ = ctx->reset | (0 << TIGHT_ZTREAM_SHIFT);
	ctx->reset = 0;

	for (y = 0; y < ys; ++y) {
		memcpy(dst, src, xs);
		src += stride;
		dst += xs;
	}
 
	*buf = dst;
}

static void
tile_16(struct tight_ctx_t *ctx, uint8_t **buf,
	uint8_t *src8, int xs, int ys, int stride, int rev)
{
	uint8_t *dst = *buf;
	uint16_t *src = (uint16_t *)src8;
	int x, y;

	/* FIXME: retarded */
	*dst++ = ctx->reset | (0 << TIGHT_ZTREAM_SHIFT);
	ctx->reset = 0;

	if (!rev) {
		for (y = 0; y < ys; ++y) {
			memcpy(dst, src, xs * 2);
			src += stride;
			dst += xs * 2;
		}
		goto done;
	}
	for (y = 0; y < ys; ++y) {
		for (x = 0; x < xs; ++x)
			dst = insert_rev_16(dst, src[x]);
		src += stride + xs;
	}

done:
	*buf = dst;
}

static void
tile_888(struct tight_ctx_t *ctx, uint8_t **buf,
	uint8_t *src, int xs, int ys, int stride)
{
	uint8_t *dst = *buf;
	int y;

	stride *= 3;

	/* FIXME: retarded */
	*dst++ = ctx->reset | (0 << TIGHT_ZTREAM_SHIFT);
	ctx->reset = 0;

	for (y = 0; y < ys; ++y) {
		memcpy(dst, src, xs * 3);
		src += stride;
		dst += xs * 3;
	}

	*buf = dst;
}

static void
tile_32(struct tight_ctx_t *ctx, uint8_t **buf,
	uint8_t *src8, int xs, int ys, int stride, int rev)
{
	uint8_t *dst = *buf;
	uint32_t *src = (uint32_t *)src8;
	int x, y;

	/* FIXME: retarded */
	*dst++ = ctx->reset | (0 << TIGHT_ZTREAM_SHIFT);
	ctx->reset = 0;

	if (!rev) {
		for (y = 0; y < ys; ++y) {
			memcpy(dst, src, xs * 4);
			src += stride;
			dst += xs * 4;
		}
		goto done;
	}
	for (y = 0; y < ys; ++y) {
		for (x = 0; x < xs; ++x)
			dst = insert_rev_32(dst, src[x]);
		src += stride + xs;
	}

done:
	*buf = dst;
}

static int
rect(ggi_vnc_client *client, struct ggi_visual *cvis,
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

	gt = LIBGGI_GT(cvis);

	bpp = GT_ByPP(gt);
	count = ggi_rect_width(update) * ggi_rect_height(update);
	GGI_vnc_buf_reserve(&client->wbuf,
		client->wbuf.size + 12 + 2);

	header = &client->wbuf.buf[client->wbuf.size];
	header[ 0] = update->tl.x >> 8;
	header[ 1] = update->tl.x & 0xff;
	header[ 2] = update->tl.y >> 8;
	header[ 3] = update->tl.y & 0xff;
	header[ 4] = ggi_rect_width(update) >> 8;
	header[ 5] = ggi_rect_width(update) & 0xff;
	header[ 6] = ggi_rect_height(update) >> 8;
	header[ 7] = ggi_rect_height(update) & 0xff;
	header[ 8] = 0;
	header[ 9] = 0;
	header[10] = 0;
	header[11] = 7; /* tight */
	client->wbuf.size += 12;

	ctx->work[0].size = 1;
	GGI_vnc_buf_reserve(&ctx->work[0], 1 + count * bpp);
	ctx->work[0].size = ctx->work[0].pos = 0;
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

	if ((ctx->work[0].buf[0] & 0x80) || ctx->work[0].size < 12)
		/* solid, jpeg or less than 12 -> no zip */
		work_buf = 0;
	else {
		client->wbuf.buf[client->wbuf.size++] = ctx->work[0].buf[0];
		++ctx->work[0].pos;
		if (ctx->work[0].buf[0] & TIGHT_FILTER) {
			client->wbuf.buf[client->wbuf.size++] =
				 ctx->work[0].buf[1];
			++ctx->work[0].pos;
		}
		ctx->work[1].size = 1;
		GGI_vnc_buf_reserve(&ctx->work[1], 1000);
		ctx->work[1].size = 0;
		zip(client, &ctx->work[0].buf[ctx->work[0].pos],
			ctx->work[0].size - ctx->work[0].pos);
		work_buf = 1;
	}

	GGI_vnc_buf_reserve(&client->wbuf,
		client->wbuf.size + 3 +
		ctx->work[work_buf].size - ctx->work[work_buf].pos);
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

static inline int
column(ggi_vnc_client *client, struct ggi_visual *cvis,
	const ggi_directbuffer *db, ggi_rect *update)
{
	ggi_rect row_update = *update;
	int y;
	int vnc_rects = 0;

	for (y = update->tl.y; y < update->br.y; y += 256) {
		row_update.tl.y = y;
		if (y + 256 > update->br.y)
			row_update.br.y = update->br.y;
		else
			row_update.br.y = y + 256;
		rect(client, cvis, db, &row_update);
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

	col_update = vupdate;
	for (x = vupdate.tl.x; x < vupdate.br.x; x += 2048) {
		col_update.tl.x = x;
		if (x + 2048 > vupdate.br.x)
			col_update.br.x = vupdate.br.x;
		else
			col_update.br.x = x + 2048;
		vnc_rects += column(client, cvis, db, &col_update);
	}

	ggiResourceRelease(db->resource);

	return vnc_rects;
}

struct tight_ctx_t *
GGI_vnc_tight_open(void)
{
	struct tight_ctx_t *ctx = malloc(sizeof(*ctx));

	memset(ctx, 0, sizeof(*ctx));

	ctx->reset = TIGHT_ZTREAM_RESET;

	ctx->zstr.zalloc = Z_NULL;
	ctx->zstr.zfree = Z_NULL;
	ctx->zstr.opaque = Z_NULL;

	deflateInit(&ctx->zstr, Z_DEFAULT_COMPRESSION);

	return ctx;
}

void
GGI_vnc_tight_close(struct tight_ctx_t *ctx)
{
	free(ctx->work[0].buf);
	free(ctx->work[1].buf);
	deflateEnd(&ctx->zstr);
	free(ctx);
}
