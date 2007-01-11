/* $Id: rre.c,v 1.1 2007/01/11 01:05:12 pekberg Exp $
******************************************************************************

   display-vnc: RFB rre encoding

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

#include <ggi/display/vnc.h>
#include <ggi/internal/ggi_debug.h>
#include "rect.h"
#include "encoding.h"
#include "common.h"

struct rre_ctx_t {
	int *done;
	int size;
};

typedef void (tile_func)(ggi_vnc_buf *buf, int *done,
	uint8_t *src, int xs, int ys, int stride, int rev);

static uint8_t
scan_8(uint8_t *src, int xs, int ys, int stride)
{
	int count[3] = { 0, 0, 0 };
	uint8_t color[3];
	int cidx = 0;
	int x, y;
	int i;
	int j;
	int select_cnt;
	int select_idx;

	color[cidx] = src[0];

	for (y = 0; y < ys; ++y) {
		i = y * stride;
		for (x = 0; x < xs; ++i, ++x) {
			if (color[cidx] == src[i]) {
				++count[cidx];
				continue;
			}
			select_idx = cidx;
			select_cnt = count[cidx];
			j = (cidx + 1) % 3;
			while (j != cidx) {
				if (count[j] && (color[j] == src[i])) {
					select_idx = j;
					break;
				}
				if (count[j] < select_cnt) {
					select_cnt = count[j];
					select_idx = j;
				}
				j = (j + 1) % 3;
			}
			cidx = select_idx;
			if (color[cidx] != src[i]) {
				color[cidx] = src[i];
				count[cidx] = 0;
			}
			++count[cidx];
		}
	}

	if (!count[1])
		/* 1 color */
		return color[0];
	if (!count[2])
		/* 2 colors */
		return (count[0] >= count[1]) ? color[0] : color[1];

	/* 3 or more colors */
	if (count[0] >= count[1] && count[0] >= count[2])
		return color[0];
	if (count[1] >= count[0] && count[1] >= count[2])
		return color[1];
	return color[2];
}

static uint16_t
scan_16(uint16_t *src, int xs, int ys, int stride)
{
	int count[3] = { 0, 0, 0 };
	uint16_t color[3];
	int cidx = 0;
	int x, y;
	int i;
	int j;
	int select_cnt;
	int select_idx;

	color[cidx] = src[0];

	for (y = 0; y < ys; ++y) {
		i = y * stride;
		for (x = 0; x < xs; ++i, ++x) {
			if (color[cidx] == src[i]) {
				++count[cidx];
				continue;
			}
			select_idx = cidx;
			select_cnt = count[cidx];
			j = (cidx + 1) % 3;
			while (j != cidx) {
				if (count[j] && (color[j] == src[i])) {
					select_idx = j;
					break;
				}
				if (count[j] < select_cnt) {
					select_cnt = count[j];
					select_idx = j;
				}
				j = (j + 1) % 3;
			}
			cidx = select_idx;
			if (color[cidx] != src[i]) {
				color[cidx] = src[i];
				count[cidx] = 0;
			}
			++count[cidx];
		}
	}

	if (!count[1])
		/* 1 color */
		return color[0];
	if (!count[2])
		/* 2 colors */
		return (count[0] >= count[1]) ? color[0] : color[1];

	/* 3 or more colors */
	if (count[0] >= count[1] && count[0] >= count[2])
		return color[0];
	if (count[1] >= count[0] && count[1] >= count[2])
		return color[1];
	return color[2];
}

static uint32_t
scan_32(uint32_t *src, int xs, int ys, int stride)
{
	int count[3] = { 0, 0, 0 };
	uint32_t color[3];
	int cidx = 0;
	int x, y;
	int i;
	int j;
	int select_cnt;
	int select_idx;

	color[cidx] = src[0];

	for (y = 0; y < ys; ++y) {
		i = y * stride;
		for (x = 0; x < xs; ++i, ++x) {
			if (color[cidx] == src[i]) {
				++count[cidx];
				continue;
			}
			select_idx = cidx;
			select_cnt = count[cidx];
			j = (cidx + 1) % 3;
			while (j != cidx) {
				if (count[j] && (color[j] == src[i])) {
					select_idx = j;
					break;
				}
				if (count[j] < select_cnt) {
					select_cnt = count[j];
					select_idx = j;
				}
				j = (j + 1) % 3;
			}
			cidx = select_idx;
			if (color[cidx] != src[i]) {
				color[cidx] = src[i];
				count[cidx] = 0;
			}
			++count[cidx];
		}
	}

	if (!count[1])
		/* 1 color */
		return color[0];
	if (!count[2])
		/* 2 colors */
		return (count[0] >= count[1]) ? color[0] : color[1];

	/* 3 or more colors */
	if (count[0] >= count[1] && count[0] >= count[2])
		return color[0];
	if (count[1] >= count[0] && count[1] >= count[2])
		return color[1];
	return color[2];
}

static int
rectify_8(ggi_vnc_buf *buf, int *done, uint8_t *src, int xs, int ys,
	int stride, uint8_t bg)
{
	int xx, yy;
	int i;
	int x, y;
	int rects;
	int count;
	uint8_t c;
	ggi_rect r;
	uint8_t *dst = &buf->buf[buf->size];
	uint8_t *subrects = dst;

	count = 0;

	rects = (xs * ys - 5) / 9;
	if (rects <= 0)
		return 0;

	memset(done, 0, xs * sizeof(int));

	dst += 4;
	*dst++ = bg;

	for (yy = 0; yy < ys; ++yy) {
		i = yy * stride;
		for (xx = 0; xx < xs; ++i, ++xx) {
			if (src[i] == bg)
				continue;
			if (done[xx] > yy)
				continue;
			if (++count > rects)
				return 0;
			c = src[i];
			*dst++ = c;
			r.tl.x = xx;
			r.tl.y = y = yy;
			for (x = r.tl.x; x < xs; ++x) {
				if (src[y * stride + x] != c)
					break;
				if (done[x] > y)
					break;
			}
			r.br.x = x;
			for (x = r.tl.x; y < ys; ++y) {
				if (src[y * stride + x] != c)
					break;
				if (done[x] > y)
					break;
			}
			r.br.y = y;
			if (ggi_rect_width(&r) >= ggi_rect_height(&r)) {
				for (y = r.tl.y; y < r.br.y; ++y) {
					for (x = r.tl.x; x < r.br.x; ++x) {
						if (src[y * stride + x] != c)
							break;
					}
					if (x != r.br.x)
						break;
				}
				r.br.y = y;
				for (x = r.tl.x; x < r.br.x; ++x)
					done[x] = r.br.y;
			}
			else {
				for (x = r.tl.x; x < r.br.x; ++x) {
					for (y = r.tl.y; y < r.br.y; ++y) {
						if (src[y * stride + x] != c)
							break;
					}
					if (y != r.br.y)
						break;
					done[x] = r.br.y;
				}
				r.br.x = x;
			}
			dst = insert_hilo_16(dst, r.tl.x);
			dst = insert_hilo_16(dst, r.tl.y);
			dst = insert_hilo_16(dst, ggi_rect_width(&r));
			dst = insert_hilo_16(dst, ggi_rect_height(&r));
		}
	}

	insert_hilo_32(subrects, count);
	buf->size = dst - buf->buf;
	return 1;
}

typedef uint8_t *(insert_16_t)(uint8_t *dst, uint16_t pixel);

static int
rectify_16(ggi_vnc_buf *buf, int *done, uint16_t *src, int xs, int ys,
	int stride, uint16_t bg, insert_16_t *insert)
{
	int xx, yy;
	int i;
	int x, y;
	int rects;
	int count;
	uint16_t c;
	ggi_rect r;
	uint8_t *dst = &buf->buf[buf->size];
	uint8_t *subrects = dst;

	count = 0;

	rects = (xs * ys - 6) / 10;
	if (rects <= 0)
		return 0;

	memset(done, 0, xs * sizeof(int));

	dst += 4;
	dst = insert(dst, bg);

	for (yy = 0; yy < ys; ++yy) {
		i = yy * stride;
		for (xx = 0; xx < xs; ++i, ++xx) {
			if (src[i] == bg)
				continue;
			if (done[xx] > yy)
				continue;
			if (++count > rects)
				return 0;
			c = src[i];
			dst = insert(dst, c);
			r.tl.x = xx;
			r.tl.y = y = yy;
			for (x = r.tl.x; x < xs; ++x) {
				if (src[y * stride + x] != c)
					break;
				if (done[x] > y)
					break;
			}
			r.br.x = x;
			for (x = r.tl.x; y < ys; ++y) {
				if (src[y * stride + x] != c)
					break;
				if (done[x] > y)
					break;
			}
			r.br.y = y;
			if (ggi_rect_width(&r) >= ggi_rect_height(&r)) {
				for (y = r.tl.y; y < r.br.y; ++y) {
					for (x = r.tl.x; x < r.br.x; ++x) {
						if (src[y * stride + x] != c)
							break;
					}
					if (x != r.br.x)
						break;
				}
				r.br.y = y;
				for (x = r.tl.x; x < r.br.x; ++x)
					done[x] = r.br.y;
			}
			else {
				for (x = r.tl.x; x < r.br.x; ++x) {
					for (y = r.tl.y; y < r.br.y; ++y) {
						if (src[y * stride + x] != c)
							break;
					}
					if (y != r.br.y)
						break;
					done[x] = r.br.y;
				}
				r.br.x = x;
			}
			dst = insert_hilo_16(dst, r.tl.x);
			dst = insert_hilo_16(dst, r.tl.y);
			dst = insert_hilo_16(dst, ggi_rect_width(&r));
			dst = insert_hilo_16(dst, ggi_rect_height(&r));
		}
	}

	insert_hilo_32(subrects, count);
	buf->size = dst - buf->buf;
	return 1;
}

typedef uint8_t *(insert_32_t)(uint8_t *dst, uint32_t pixel);

static int
rectify_32(ggi_vnc_buf *buf, int *done, uint32_t *src, int xs, int ys,
	int stride, uint32_t bg, insert_32_t *insert)
{
	int xx, yy;
	int i;
	int x, y;
	int rects;
	int count;
	uint32_t c;
	ggi_rect r;
	uint8_t *dst = &buf->buf[buf->size];
	uint8_t *subrects = dst;

	count = 0;

	rects = (xs * ys - 8) / 12;
	if (rects <= 0)
		return 0;

	memset(done, 0, xs * sizeof(int));

	dst += 4;
	dst = insert(dst, bg);

	for (yy = 0; yy < ys; ++yy) {
		i = yy * stride;
		for (xx = 0; xx < xs; ++i, ++xx) {
			if (src[i] == bg)
				continue;
			if (done[xx] > yy)
				continue;
			if (++count > rects)
				return 0;
			c = src[i];
			dst = insert(dst, c);
			r.tl.x = xx;
			r.tl.y = y = yy;
			for (x = r.tl.x; x < xs; ++x) {
				if (src[y * stride + x] != c)
					break;
				if (done[x] > y)
					break;
			}
			r.br.x = x;
			for (x = r.tl.x; y < ys; ++y) {
				if (src[y * stride + x] != c)
					break;
				if (done[x] > y)
					break;
			}
			r.br.y = y;
			if (ggi_rect_width(&r) >= ggi_rect_height(&r)) {
				for (y = r.tl.y; y < r.br.y; ++y) {
					for (x = r.tl.x; x < r.br.x; ++x) {
						if (src[y * stride + x] != c)
							break;
					}
					if (x != r.br.x)
						break;
				}
				r.br.y = y;
				for (x = r.tl.x; x < r.br.x; ++x)
					done[x] = r.br.y;
			}
			else {
				for (x = r.tl.x; x < r.br.x; ++x) {
					for (y = r.tl.y; y < r.br.y; ++y) {
						if (src[y * stride + x] != c)
							break;
					}
					if (y != r.br.y)
						break;
					done[x] = r.br.y;
				}
				r.br.x = x;
			}
			dst = insert_hilo_16(dst, r.tl.x);
			dst = insert_hilo_16(dst, r.tl.y);
			dst = insert_hilo_16(dst, ggi_rect_width(&r));
			dst = insert_hilo_16(dst, ggi_rect_height(&r));
		}
	}

	insert_hilo_32(subrects, count);
	buf->size = dst - buf->buf;
	return 1;
}

static void
tile_8(ggi_vnc_buf *buf, int *done,
	uint8_t *src, int xs, int ys, int stride, int rev)
{
	uint8_t bg;
	uint8_t *dst;
	int y;

	bg = scan_8(src, xs, ys, stride);

	if (rectify_8(buf, done, src, xs, ys, stride, bg))
		return;

	dst = &buf->buf[buf->size];
	buf->size += xs * ys;
	*(dst - 1) = 0; /* raw */

	for (y = 0; y < ys; ++y, dst += xs, src += stride)
		memcpy(dst, src, xs);
}

static void
tile_16(ggi_vnc_buf *buf, int *done,
	uint8_t *src8, int xs, int ys, int stride, int rev)
{
	uint16_t *src = (uint16_t *)src8;
	uint16_t bg;
	uint8_t *dst;
	int x, y;
	insert_16_t *insert = rev ? insert_rev_16 : insert_16;

	bg = scan_16(src, xs, ys, stride);

	if (rectify_16(buf, done, src, xs, ys, stride, bg, insert))
		return;

	dst = &buf->buf[buf->size];
	buf->size += xs * ys * 2;
	*(dst - 1) = 0; /* raw */

	if (!rev) {
		for (y = 0; y < ys; ++y) {
			memcpy(dst, src, xs * 2);
			src += stride;
			dst += xs * 2;
		}
		return;
	}
	for (y = 0; y < ys; ++y) {
		for (x = 0; x < xs; ++x)
			dst = insert_rev_16(dst, src[x]);
		src += stride;
	}
}

static void
tile_32(ggi_vnc_buf *buf, int *done,
	uint8_t *src8, int xs, int ys, int stride, int rev)
{
	uint32_t *src = (uint32_t *)src8;
	uint8_t *dst;
	uint32_t bg;
	int x, y;
	insert_32_t *insert = rev ? insert_rev_32 : insert_32;

	bg = scan_32(src, xs, ys, stride);

	if (rectify_32(buf, done, src, xs, ys, stride, bg, insert))
		return;

	dst = &buf->buf[buf->size];
	buf->size += xs * ys * 4;
	*(dst - 1) = 0; /* raw */

	if (!rev) {
		for (y = 0; y < ys; ++y) {
			memcpy(dst, src, xs * 4);
			src += stride;
			dst += xs * 4;
		}
		return;
	}
	for (y = 0; y < ys; ++y) {
		for (x = 0; x < xs; ++x)
			dst = insert_rev_32(dst, src[x]);
		src += stride;
	}
}

static void
tile(ggi_vnc_client *client, struct ggi_visual *cvis,
	const ggi_directbuffer *db, ggi_rect *update)
{
	struct rre_ctx_t *ctx = client->rre_ctx;
	ggi_graphtype gt;
	int bpp;
	int count;
	unsigned char *header;
	int stride;
	tile_func *tile_bpp;

	DPRINT("tile %dx%d - %dx%d\n",
		update->tl.x, update->tl.y,
		update->br.x, update->br.y);

	gt = LIBGGI_GT(cvis);

	bpp = GT_ByPP(gt);
	count = ggi_rect_width(update) * ggi_rect_height(update);
	GGI_vnc_buf_reserve(&client->wbuf,
		client->wbuf.size + 12 + count * bpp);
	GGI_vnc_buf_reserve(&client->wbuf,
		client->wbuf.size + 12 + 4 + bpp);

	header = &client->wbuf.buf[client->wbuf.size];
	insert_header(header, &update->tl, update, 2); /* rre */
	client->wbuf.size += 12;

	stride = LIBGGI_VIRTX(cvis);

	if (bpp == 1)
		tile_bpp = tile_8;
	else if (bpp == 2)
		tile_bpp = tile_16;
	else
		tile_bpp = tile_32;

	if (ctx->size < ggi_rect_width(update)) {
		ctx->size = ggi_rect_width(update);
		ctx->done = _ggi_realloc(ctx->done, ctx->size * sizeof(int));
	}

	tile_bpp(&client->wbuf, ctx->done, (uint8_t *)db->read +
		(update->tl.y * stride + update->tl.x) * bpp,
		ggi_rect_width(update), ggi_rect_height(update),
		stride, client->reverse_endian);
}

int
GGI_vnc_rre(ggi_vnc_client *client, ggi_rect *update)
{
	struct ggi_visual *vis = client->owner;
	ggi_vnc_priv *priv = VNC_PRIV(vis);
	struct ggi_visual *cvis;
	const ggi_directbuffer *db;
	ggi_rect vupdate;
	int d_frame_num;

	DPRINT("rre update %dx%d - %dx%d\n",
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

	tile(client, cvis, db, &vupdate);

	ggiResourceRelease(db->resource);

	return 1;
}

struct rre_ctx_t *
GGI_vnc_rre_open(void)
{
	struct rre_ctx_t *ctx = _ggi_calloc(sizeof(*ctx));

	return ctx;
}

void
GGI_vnc_rre_close(struct rre_ctx_t *ctx)
{
	free(ctx->done);
	free(ctx);
}
