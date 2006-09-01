/* $Id: zlib.c,v 1.1 2006/09/01 15:23:03 pekberg Exp $
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


static void
zip(ggi_vnc_priv *priv, uint8_t *src, int len)
{
	zlib_ctx_t *ctx = priv->zlib_ctx;
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

	DPRINT_MISC("raw %d z %d %d%%\n", len, done, done * 100 / len);
}

void
GGI_vnc_zlib(struct ggi_visual *vis, ggi_rect *update)
{
	ggi_vnc_priv *priv = VNC_PRIV(vis);
	zlib_ctx_t *ctx = priv->zlib_ctx;
	ggi_vnc_buf tmp_buf;

	/* fake it for the raw encoder so that it renders the update
	 * in an intermediate buffer
	 */
	tmp_buf = priv->wbuf;
	priv->wbuf = ctx->wbuf;
	priv->wbuf.pos = priv->wbuf.size = 0;

	GGI_vnc_raw(vis, update);

	/* swap the buffers back */
	ctx->wbuf = priv->wbuf;
	priv->wbuf = tmp_buf;

	/* change header to zlib encoding */
	ctx->wbuf.buf[15] = 6; /* zlib */

	/* copy the revised header to the real buffer */
	GGI_vnc_buf_reserve(&priv->wbuf, priv->wbuf.size + 20);
	memcpy(&priv->wbuf.buf[priv->wbuf.size], ctx->wbuf.buf, 16);

	priv->wbuf.size += 16;

	/* zip up the intermediate buffer */
	zip(priv, &ctx->wbuf.buf[16], ctx->wbuf.size - 16);
}

zlib_ctx_t *
GGI_vnc_zlib_open(int level)
{
	zlib_ctx_t *ctx = malloc(sizeof(*ctx));

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
GGI_vnc_zlib_close(zlib_ctx_t *ctx)
{
	if (ctx->wbuf.buf)
		free(ctx->wbuf.buf);
	deflateEnd(&ctx->zstr);
	free(ctx);
}
