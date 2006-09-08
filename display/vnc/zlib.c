/* $Id: zlib.c,v 1.5 2006/09/08 08:27:08 pekberg Exp $
******************************************************************************

   display-vnc: RFB zlib encoding

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

#ifdef GGI_BIG_ENDIAN
#define GGI_HTONL(x) (x)
#else
#define GGI_HTONL(x) GGI_BYTEREV32(x)
#endif

struct zlib_ctx_t {
	z_stream zstr;
	ggi_vnc_buf wbuf;
};

static void
zip(ggi_vnc_priv *priv, uint8_t *src, int len)
{
	ggi_vnc_client *client = priv->client;
	struct zlib_ctx_t *ctx = client->zlib_ctx;
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

	DPRINT_MISC("raw %d z %d %d%%\n", len, done, done * 100 / len);
}

int
GGI_vnc_zlib(struct ggi_visual *vis, ggi_rect *update)
{
	ggi_vnc_priv *priv = VNC_PRIV(vis);
	ggi_vnc_client *client = priv->client;
	struct zlib_ctx_t *ctx = client->zlib_ctx;
	ggi_vnc_buf tmp_buf;

	/* fake it for the raw encoder so that it renders the update
	 * in an intermediate buffer
	 */
	tmp_buf = client->wbuf;
	client->wbuf = ctx->wbuf;
	client->wbuf.pos = client->wbuf.size = 0;

	GGI_vnc_raw(vis, update);

	/* swap the buffers back */
	ctx->wbuf = client->wbuf;
	client->wbuf = tmp_buf;

	/* change header to zlib encoding */
	ctx->wbuf.buf[11] = 6; /* zlib */

	/* copy the revised header to the real buffer */
	GGI_vnc_buf_reserve(&client->wbuf, client->wbuf.size + 16);
	memcpy(&client->wbuf.buf[client->wbuf.size], ctx->wbuf.buf, 12);

	client->wbuf.size += 12;

	/* zip up the intermediate buffer */
	zip(priv, &ctx->wbuf.buf[12], ctx->wbuf.size - 12);
	return 1;
}

struct zlib_ctx_t *
GGI_vnc_zlib_open(int level)
{
	struct zlib_ctx_t *ctx = malloc(sizeof(*ctx));

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
GGI_vnc_zlib_close(struct zlib_ctx_t *ctx)
{
	free(ctx->wbuf.buf);
	deflateEnd(&ctx->zstr);
	free(ctx);
}
