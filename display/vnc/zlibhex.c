/* $Id: zlibhex.c,v 1.3 2006/11/22 22:32:44 pekberg Exp $
******************************************************************************

   display-vnc: RFB ZlibHex encoding

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
#include "encoding.h"
#include "common.h"

struct zlibhex_ctx_t {
	z_stream zstr[2];
	ggi_vnc_buf wbuf;
};

/* subencoding */
#define ZLIBHEX_RAW      (1<<0)
#define ZLIBHEX_BG       (1<<1)
#define ZLIBHEX_FG       (1<<2)
#define ZLIBHEX_SUBRECTS (1<<3)
#define ZLIBHEX_COLORED  (1<<4)
#define ZLIBHEX_ZRAW     (1<<5)
#define ZLIBHEX_ZHEX     (1<<6)

static void
zip(ggi_vnc_client *client, int ztream, uint8_t *src, int len)
{
	struct zlibhex_ctx_t *ctx = client->zlibhex_ctx;
	int start = client->wbuf.size;
	int avail;
	uint16_t done = 0;

	ctx->zstr[ztream].next_in = src;
	ctx->zstr[ztream].avail_in = len;

	client->wbuf.size += 2;

	avail = client->wbuf.limit - client->wbuf.size;

	for (;;) {
		ctx->zstr[ztream].next_out =
			&client->wbuf.buf[client->wbuf.size + done];
		ctx->zstr[ztream].avail_out = avail - done;
		deflate(&ctx->zstr[ztream], Z_SYNC_FLUSH);
		done = avail - ctx->zstr[ztream].avail_out;
		if (ctx->zstr[ztream].avail_out)
			break;
		avail += 1000;
		GGI_vnc_buf_reserve(&client->wbuf, client->wbuf.size + avail);
	}

	insert_hilo_16(&client->wbuf.buf[start], done);
	client->wbuf.size += done;
}

static inline int
tile(ggi_vnc_client *client, uint8_t *src, int xs, int ys, int bpp)
{
	uint8_t subencoding = *src;
	int size;
	int pos;

	if (subencoding & ZLIBHEX_RAW) {
		size = 1 + xs * ys * bpp;
		if (size >= 20) {
			client->wbuf.buf[client->wbuf.size++] = ZLIBHEX_ZRAW;
			zip(client, 0, src + 1, size - 1);
			return size;
		}
		goto copy;
	}

	size = pos = 1;
	if (subencoding & ZLIBHEX_BG) {
		size += bpp;
		pos += bpp;
	}
	if (subencoding & ZLIBHEX_FG) {
		size += bpp;
		pos += bpp;
	}
	if (subencoding & ZLIBHEX_SUBRECTS) {
		int rects = src[pos];
		size += 1 + 2 * rects;
		if (subencoding & ZLIBHEX_COLORED)
			size += bpp * rects;
	}

	if (size >= 20) {
		client->wbuf.buf[client->wbuf.size++] =
			subencoding | ZLIBHEX_ZHEX;
		zip(client, 1, src + 1, size - 1);
		return size;
	}

copy:
	memcpy(&client->wbuf.buf[client->wbuf.size], src, size);
	client->wbuf.size += size;
	return size;
}

int
GGI_vnc_zlibhex(ggi_vnc_client *client, ggi_rect *update)
{
	struct ggi_visual *vis = client->owner;
	ggi_vnc_priv *priv = VNC_PRIV(vis);
	struct zlibhex_ctx_t *ctx = client->zlibhex_ctx;
	ggi_vnc_buf tmp_buf;
	uint8_t *src;
	int xt, yt;
	int xtiles, ytiles;
	int xs, ys;
	int xs_last, ys_last;
	int bpp;
	int done;

	if (!client->vis)
		bpp = GT_ByPP(LIBGGI_GT(priv->fb));
	else
		bpp = GT_ByPP(LIBGGI_GT(client->vis));

	/* fake it for the hextile encoder so that it renders the update
	 * in an intermediate buffer
	 */
	tmp_buf = client->wbuf;
	client->wbuf = ctx->wbuf;
	client->wbuf.pos = client->wbuf.size = 0;

	GGI_vnc_hextile(client, update);

	/* swap the buffers back */
	ctx->wbuf = client->wbuf;
	client->wbuf = tmp_buf;

	/* change header to zlibhex encoding */
	ctx->wbuf.buf[11] = 8; /* zlibhex */

	/* copy the revised header to the real buffer */
	GGI_vnc_buf_reserve(&client->wbuf, client->wbuf.size + 16);
	memcpy(&client->wbuf.buf[client->wbuf.size], ctx->wbuf.buf, 12);

	client->wbuf.size += 12;
	done = -client->wbuf.size;

	GGI_vnc_buf_reserve(&client->wbuf,
		client->wbuf.size + ctx->wbuf.size + 1000);

	/* step through the tiles and zip them up if that seems like
	 * a good idea...
	 */
	src = &ctx->wbuf.buf[12];

	xtiles = (ggi_rect_width(update) + 15) / 16;
	ytiles = (ggi_rect_height(update) + 15) / 16;
	xs_last = ggi_rect_width(update) & 0xf;
	if (!xs_last)
		xs_last = 16;
	ys_last = ggi_rect_height(update) & 0xf;
	if (!ys_last)
		ys_last = 16;

	ys = 16;
	for (yt = 0; yt < ytiles; ++yt) {
		if (yt == ytiles - 1)
			ys = ys_last;
		xs = 16;
		for (xt = 0; xt < xtiles; ++xt) {
			if (xt == xtiles - 1)
				xs = xs_last;

			if (client->wbuf.limit - client->wbuf.size < 1000)
				GGI_vnc_buf_reserve(&client->wbuf,
					client->wbuf.size + 1000);

			src += tile(client, src, xs, ys, bpp);
		}
	}

	done += client->wbuf.size;
	DPRINT_MISC("hextile %d z %d %d%%\n", ctx->wbuf.size - 12,
		done, done * 100 / (ctx->wbuf.size - 12));

	return 1;
}

struct zlibhex_ctx_t *
GGI_vnc_zlibhex_open(int level)
{
	struct zlibhex_ctx_t *ctx = _ggi_calloc(sizeof(*ctx));
	int i;

	if (level == -1)
		level = Z_DEFAULT_COMPRESSION;

	for (i = 0; i < 2; ++i) {
		ctx->zstr[i].zalloc = Z_NULL;
		ctx->zstr[i].zfree = Z_NULL;
		ctx->zstr[i].opaque = Z_NULL;
		deflateInit(&ctx->zstr[i], level);
	}

	return ctx;
}

void
GGI_vnc_zlibhex_close(struct zlibhex_ctx_t *ctx)
{
	free(ctx->wbuf.buf);
	deflateEnd(&ctx->zstr[1]);
	deflateEnd(&ctx->zstr[0]);
	free(ctx);
}
