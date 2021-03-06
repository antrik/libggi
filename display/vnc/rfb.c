/* $Id: rfb.c,v 1.134 2009/12/04 10:25:40 pekberg Exp $
******************************************************************************

   display-vnc: RFB protocol

   Copyright (C) 2008-2009 Peter Rosin	[peda@lysator.liu.se]

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
#include <ctype.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif

#ifdef _WIN32
#ifdef HAVE_WINDOWS_H
# include <windows.h>
#endif
#ifdef HAVE_WS2TCPIP_H
# include <ws2tcpip.h>
#endif
#endif

#include <sys/types.h>
#ifdef HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif
#ifdef HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif
#ifdef HAVE_ARPA_INET_H
#include <arpa/inet.h>
#endif
#ifdef HAVE_NETDB_H
#include <netdb.h>
#endif

#include <errno.h>

#include <ggi/gg.h>
#include <ggi/internal/gg_replace.h>
#include <ggi/display/vnc.h>
#include <ggi/input/vnc.h>
#include <ggi/internal/ggi_debug.h>
#include "common.h"

#ifdef HAVE_OPENSSL
#include <openssl/des.h>
#else
#include "d3des.h"
#endif /* HAVE_OPENSSL */

#include "encoding.h"

static int vnc_client_run(ggi_vnc_client *client);
int GGI_vnc_read_ready(ggi_vnc_client *client);
int GGI_vnc_write_ready(ggi_vnc_client *client);
int GGI_vnc_client_init(ggi_vnc_client *client);
int GGI_vnc_client_vnc_auth(ggi_vnc_client *client);
int GGI_vnc_client_vencrypt_security(ggi_vnc_client *client);

#ifndef HAVE_OPENSSL
#define GGI_vnc_client_vencrypt_security(client) -1
#endif

static int
color_bits(uint16_t max)
{
	int bits = 0;

	while (max) {
		max >>= 1;
		++bits;
	}

	return bits;
}

static int
color_max(ggi_pixel mask)
{
	while (mask && !(mask & 1))
		mask >>= 1;

	return mask;
}

static int
color_shift(ggi_pixel mask)
{
	int shift = 0;

	while (mask && !(mask & 1)) {
		mask >>= 1;
		++shift;
	}

	return shift;
}

int
GGI_vnc_buf_reserve(ggi_vnc_buf *buf, int limit)
{
	if (buf->limit >= limit)
		return 0;

	if (!buf->limit) {
		free(buf->buf);
		buf->buf = _ggi_malloc(limit);
	}
	else {
		unsigned char *tmp;
		tmp = _ggi_realloc(buf->buf, limit);
		if (tmp == NULL)
			return 1;
		buf->buf = tmp;
	}

	if (buf->buf)
		buf->limit = limit;

	return 0;
}

static void
close_client(ggi_vnc_client *client)
{
	struct ggi_visual *vis = client->owner;
	ggi_vnc_priv *priv = VNC_PRIV(vis);

	if (client->cfd == -1)
		return;

	DPRINT("close_client.\n");
	priv->del_cfd(priv->gii_ctx, client, client->cfd);
	if (client->write_pending)
		priv->del_cwfd(priv->gii_ctx, client, client->cfd);
	client->write_pending = 0;
#ifdef HAVE_OPENSSL
	GGI_vnc_vencrypt_stop_tls(client);
#endif
	free(client->wbuf.buf);
	memset(&client->wbuf, 0, sizeof(client->wbuf));
	close(client->cfd);
	if (client->cwfd != client->cfd)
		close(client->cwfd);
	client->cfd = -1;
	client->cwfd = -1;
	client->buf_size = 0;
	client->dirty.tl.x = client->dirty.br.x = 0;
	client->update.tl.x = client->update.br.x = 0;

	if (client->hextile_ctx) {
		GGI_vnc_hextile_close(client->hextile_ctx);
		client->hextile_ctx = NULL;
	}

	if (client->rre_ctx) {
		GGI_vnc_rre_close(client->rre_ctx);
		client->rre_ctx = NULL;
	}

	if (client->trle_ctx) {
		GGI_vnc_trle_close(client->trle_ctx);
		client->trle_ctx = NULL;
	}

#ifdef HAVE_ZLIB
	if (client->zlib_ctx) {
		GGI_vnc_zlib_close(client->zlib_ctx);
		client->zlib_ctx = NULL;
	}

	if (client->zlibhex_ctx) {
		GGI_vnc_zlibhex_close(client->zlibhex_ctx);
		client->zlibhex_ctx = NULL;
	}

	if (client->zrle_ctx) {
		GGI_vnc_zrle_close(client->zrle_ctx);
		client->zrle_ctx = NULL;
	}

	if (client->tight_ctx) {
		GGI_vnc_tight_close(client->tight_ctx);
		client->tight_ctx = NULL;
	}
#endif

	GG_LIST_REMOVE(client, siblings);

	if (client->buf)
		free(client->buf);
	free(client);

	if (priv->kill_on_last_disconnect) {
		if (GG_LIST_EMPTY(&priv->clients))
			ggPanic("No VNC clients, exiting...\n");
	}
}

static int
vnc_safe_write(ggi_vnc_client *client, int close_on_error)
{
	struct ggi_visual *vis = client->owner;
	ggi_vnc_priv *priv = VNC_PRIV(vis);
	ggi_vnc_buf *buf = &client->wbuf;
	int res;

	if (client->write_pending) {
		DPRINT("impatient client...\n");
		if (close_on_error)
			close_client(client);
		return -1;
	}

again:
#if defined(__WIN32__) && !defined(__CYGWIN__)
	res = send(client->cwfd,
		buf->buf + buf->pos, buf->size - buf->pos, 0);
#else
	res = write(client->cwfd, buf->buf + buf->pos, buf->size - buf->pos);
#endif

	if (res == buf->size - buf->pos) {
		/* DPRINT("complete write\n"); */
		buf->size = buf->pos = 0;
		return res;
	}

	if (res > 0) {
		/* DPRINT("partial write\n"); */
		buf->pos += res;
		goto again;
	}

	switch (errno) {
	case EINTR:
		goto again;
	case EAGAIN:
#ifdef EWOULDBLOCK
#if EAGAIN != EWOULDBLOCK
	case EWOULDBLOCK:
#endif
#endif
#ifdef WSAEWOULDBLOCK
#if EAGAIN != WSAEWOULDBLOCK
	case WSAEWOULDBLOCK:
#endif
#endif
		/* queue it */
		/* DPRINT("write would block\n"); */
		client->write_pending = 1;
		priv->add_cwfd(priv->gii_ctx, client, client->cwfd);
		return 0;
	default:
		DPRINT("write error (%d, \"%s\").\n", errno, strerror(errno));
		if (close_on_error)
			close_client(client);
		return res;
	}
}

static int
vnc_remove(ggi_vnc_client *client, int count)
{
	memmove(client->buf, &client->buf[count], client->buf_size - count);
	client->buf_size -= count;

	if (client->action == vnc_client_run)
		return client->buf_size;

	client->action = vnc_client_run;
	return vnc_client_run(client);
}

static int
vnc_reserve(ggi_vnc_client *client, int count)
{
	if (client->buf_limit < count + 100) {
		/* need more space */
		uint8_t *buf = malloc(count + 100);
		if (!buf)
			return 1;
		memcpy(buf, client->buf, client->buf_size);
		free(client->buf);
		client->buf = buf;
		client->buf_limit = count + 100;
	}

	return 0;
}

static int
change_pixfmt(ggi_vnc_client *client)
{
	struct ggi_visual *vis = client->owner;
	ggi_vnc_priv *priv = VNC_PRIV(vis);
	int err = 0;
	ggi_mode mode;
	struct ggi_visual fake_vis;
	int i;
	char target[GGI_MAX_APILEN];
	struct gg_stem *stem;

	DPRINT_MISC("change_pixfmt:\n");
	DPRINT_MISC("  depth/size: %d/%d\n",
		client->pixfmt.depth, client->pixfmt.size);
	DPRINT_MISC("  red mask (shift):   %08x (%d)\n",
		client->pixfmt.red_mask,   client->pixfmt.red_shift);
	DPRINT_MISC("  green mask (shift): %08x (%d)\n",
		client->pixfmt.green_mask, client->pixfmt.green_shift);
	DPRINT_MISC("  blue mask (shift):  %08x (%d)\n",
		client->pixfmt.blue_mask,  client->pixfmt.blue_shift);
	DPRINT_MISC("  clut mask (shift):  %08x (%d)\n",
		client->pixfmt.clut_mask,  client->pixfmt.clut_shift);

	if (client->vis) {
		stem = client->vis->instance.stem;
		ggiClose(stem);
		ggDelStem(stem);
		client->vis = NULL;
	}

#ifdef HAVE_ZLIB
	if (client->encode == GGI_vnc_tight) {
		if (((client->pixfmt.red_mask << client->pixfmt.red_shift)
			== 0xff000000) &&
		    ((client->pixfmt.green_mask << client->pixfmt.green_shift)
			== 0xff000000) &&
		    ((client->pixfmt.blue_mask << client->pixfmt.blue_shift)
			== 0xff000000))
		{
			client->pixfmt.red_mask    = 0x000000ff;
			client->pixfmt.green_mask  = 0x0000ff00;
			client->pixfmt.blue_mask   = 0x00ff0000;
			client->pixfmt.size  = 24;
			client->pixfmt.depth = 24;
			_ggi_build_pixfmt(&client->pixfmt);
		}
	}
#endif

	if (client->pixfmt.red_mask == LIBGGI_PIXFMT(vis)->red_mask &&
		client->pixfmt.green_mask == LIBGGI_PIXFMT(vis)->green_mask &&
		client->pixfmt.blue_mask == LIBGGI_PIXFMT(vis)->blue_mask &&
		client->pixfmt.clut_mask == LIBGGI_PIXFMT(vis)->clut_mask &&
		client->pixfmt.size == LIBGGI_PIXFMT(vis)->size)
		return 0;

	memset(&mode, 0, sizeof(mode));
	mode.frames = 1;
	mode.visible.x = LIBGGI_X(vis);
	mode.visible.y = LIBGGI_Y(vis);
	mode.virt.x    = LIBGGI_VIRTX(vis);
	mode.virt.y    = LIBGGI_VIRTY(vis);
	mode.size.x = mode.size.y = GGI_AUTO;
	GT_SETDEPTH(mode.graphtype, client->pixfmt.depth);
	GT_SETSIZE(mode.graphtype, client->pixfmt.size);
	if (!client->pixfmt.clut_mask)
		GT_SETSCHEME(mode.graphtype, GT_TRUECOLOR);
	else
		GT_SETSCHEME(mode.graphtype, GT_PALETTE);
	mode.dpp.x = mode.dpp.y = 1;

	if (!client->pixfmt.clut_mask) {
		i = snprintf(target,
			GGI_MAX_APILEN, "display-memory:-pixfmt=");

		/* Need a visual to call _ggi_build_pixfmtstr, so fake
		 * one...
		 */
		fake_vis.pixfmt = &client->pixfmt;
		fake_vis.mode = &mode;
		memset(target + i, '\0', sizeof(target) - i);
		_ggi_build_pixfmtstr(&fake_vis,
			target + i, sizeof(target) - i, GGI_PIXFMT_CHANNEL);
		i = strlen(target);
	}
	else
		i = snprintf(target, GGI_MAX_APILEN, "display-memory");

	stem = ggNewStem(libggi, NULL);
	if (stem == NULL) {
		DPRINT("ggNewStem failed\n");
		err = -1;
		goto out;
	}
	if (ggiOpen(stem, target, NULL) != GGI_OK) {
		DPRINT("ggiOpen failed\n");
		err = -1;
		goto del_stem;
	}
	client->vis = STEM_API_DATA(stem, libggi, struct ggi_visual *);
	if (ggiSetMode(stem, &mode)) {
		DPRINT("ggiSetMode failed\n");
		err = -1;
		goto close_vis;
	}

	DPRINT_MISC("fb stdformat 0x%x, cvis stdformat 0x%x\n",
		priv->fb->pixfmt->stdformat,
		client->vis->pixfmt->stdformat);

	if (client->pixfmt.clut_mask)
		ggiSetColorfulPalette(stem);

	return 0;

close_vis:
	ggiClose(stem);
del_stem:
	ggDelStem(stem);
out:
	client->vis = NULL;
	return err;
}

static int
vnc_client_pixfmt(ggi_vnc_client *client)
{
	uint16_t red_max;
	uint16_t green_max;
	uint16_t blue_max;

	DPRINT("client_pixfmt\n");

	if (client->buf_size < 20) {
		/* wait for more data */
		client->action = vnc_client_pixfmt;
		return 0;
	}

	memcpy(&red_max, &client->buf[8], sizeof(red_max));
	red_max = ntohs(red_max);
	memcpy(&green_max, &client->buf[10], sizeof(green_max));
	green_max = ntohs(green_max);
	memcpy(&blue_max, &client->buf[12], sizeof(blue_max));
	blue_max = ntohs(blue_max);

	DPRINT_MISC("Requested pixfmt:\n");
	DPRINT_MISC("  depth/size %d/%d\n", client->buf[5], client->buf[4]);
#ifdef GGI_BIG_ENDIAN
	client->reverse_endian = !client->buf[6];
#else
	client->reverse_endian = client->buf[6];
#endif
	DPRINT_MISC("  endian: %s\n",
		client->reverse_endian ? "reverse" : "match");
	DPRINT_MISC("  type: %s\n",
		client->buf[7] ? "truecolor" : "palette");
	DPRINT_MISC("  red max (shift):   %u (%d)\n",
		red_max,   client->buf[14]);
	DPRINT_MISC("  green max (shift): %u (%d)\n",
		green_max, client->buf[15]);
	DPRINT_MISC("  blue max (shift):  %u (%d)\n",
		blue_max,  client->buf[16]);

	memset(&client->pixfmt, 0, sizeof(ggi_pixelformat));
	client->pixfmt.size = client->buf[4];
	/* Don't trust depth from the client, they often
	 * ask for the wrong thing...
	 */
	if (client->buf[7]) {
		client->pixfmt.depth = color_bits(red_max) +
			color_bits(green_max) +
			color_bits(blue_max);
		if (client->pixfmt.size < client->pixfmt.depth)
			client->pixfmt.depth = client->buf[5];
		client->pixfmt.red_mask = red_max << client->buf[14];
		client->pixfmt.green_mask = green_max << client->buf[15];
		client->pixfmt.blue_mask = blue_max << client->buf[16];
	}
	else {
		client->pixfmt.depth = client->buf[5];
		client->pixfmt.clut_mask = (1 << client->pixfmt.depth) - 1;
	}

	_ggi_build_pixfmt(&client->pixfmt);

	client->palette_dirty = !client->buf[7];

	client->requested_pixfmt = client->pixfmt;
	if (change_pixfmt(client))
		return -1;

	return vnc_remove(client, 20);
}

typedef ggi_vnc_encode * (vnc_encoding)
	(ggi_vnc_client *client, int32_t encoding, const char *format);

static ggi_vnc_encode *
print_enc(ggi_vnc_client *client, int32_t encoding, const char *format)
{
	DPRINT_MISC(format, encoding);
	return NULL;
}

static ggi_vnc_encode *
raw_enc(ggi_vnc_client *client, int32_t encoding, const char *format)
{
	DPRINT_MISC(format);
	return GGI_vnc_raw;
}

static ggi_vnc_encode *
copyrect_enc(ggi_vnc_client *client, int32_t encoding, const char *format)
{
	struct ggi_visual *vis = client->owner;
	ggi_vnc_priv *priv = VNC_PRIV(vis);

	DPRINT_MISC(format);
	if (!priv->copyrect)
		return NULL;
	if (client->encode && priv->copyrect == 1)
		return NULL;
	client->copy_rect = 1;

	return NULL;
}

static ggi_vnc_encode *
rre_enc(ggi_vnc_client *client, int32_t encoding, const char *format)
{
	struct ggi_visual *vis = client->owner;
	ggi_vnc_priv *priv = VNC_PRIV(vis);

	DPRINT_MISC(format);
	if (!priv->rre)
		return NULL;
	if (client->encode)
		return NULL;
	if (!client->rre_ctx)
		client->rre_ctx = GGI_vnc_rre_open();
	if (!client->rre_ctx)
		return NULL;

	return GGI_vnc_rre;
}

static ggi_vnc_encode *
corre_enc(ggi_vnc_client *client, int32_t encoding, const char *format)
{
	struct ggi_visual *vis = client->owner;
	ggi_vnc_priv *priv = VNC_PRIV(vis);

	DPRINT_MISC(format);
	if (!priv->corre)
		return NULL;
	if (client->encode)
		return NULL;

	return GGI_vnc_corre;
}

static ggi_vnc_encode *
hextile_enc(ggi_vnc_client *client, int32_t encoding, const char *format)
{
	struct ggi_visual *vis = client->owner;
	ggi_vnc_priv *priv = VNC_PRIV(vis);

	if (format)
		DPRINT_MISC(format);
	if (!priv->hextile)
		return NULL;
	if (client->encode)
		return NULL;
	if (!client->hextile_ctx)
		client->hextile_ctx =
			GGI_vnc_hextile_open();
	if (!client->hextile_ctx)
		return NULL;

	return GGI_vnc_hextile;
}

#ifdef HAVE_ZLIB
static ggi_vnc_encode *
zlib_enc(ggi_vnc_client *client, int32_t encoding, const char *format)
{
	struct ggi_visual *vis = client->owner;
	ggi_vnc_priv *priv = VNC_PRIV(vis);

	DPRINT_MISC(format);
	if (client->encode || priv->zlib_level == -2)
		return NULL;
	if (!client->zlib_ctx)
		client->zlib_ctx = GGI_vnc_zlib_open(-1);
	if (!client->zlib_ctx)
		return NULL;

	return GGI_vnc_zlib;
}
#else
#define zlib_enc print_enc
#endif

#ifdef HAVE_ZLIB
static ggi_vnc_encode *
zlibhex_enc(ggi_vnc_client *client, int32_t encoding, const char *format)
{
	struct ggi_visual *vis = client->owner;
	ggi_vnc_priv *priv = VNC_PRIV(vis);

	DPRINT_MISC(format);
	if (client->encode || priv->zlibhex_level == -2)
		return NULL;
	if (!client->zlibhex_ctx)
		client->zlibhex_ctx = GGI_vnc_zlibhex_open(-1);
	if (!client->zlibhex_ctx)
		return NULL;

	if (!hextile_enc(client, encoding, NULL))
		return NULL;

	return GGI_vnc_zlibhex;
}
#else
#define zlibhex_enc print_enc
#endif

#ifdef HAVE_ZLIB
static ggi_vnc_encode *
tight_enc(ggi_vnc_client *client, int32_t encoding, const char *format)
{
	struct ggi_visual *vis = client->owner;
	ggi_vnc_priv *priv = VNC_PRIV(vis);

	DPRINT_MISC(format);
	if (client->encode || !(priv->tight & 2))
		return NULL;
	if (!client->tight_ctx)
		client->tight_ctx =
			GGI_vnc_tight_open();
	if (!client->tight_ctx)
		return NULL;

	client->update_pixfmt ^= 1;

	return GGI_vnc_tight;
}

static ggi_vnc_encode *
tight_quality_enc(ggi_vnc_client *client, int32_t encoding, const char *format)
{
	DPRINT_MISC(format, encoding);

	if (client->tight_ctx)
		GGI_vnc_tight_quality(client->tight_ctx,
			encoding);

	return NULL;
}
#else
#define tight_enc print_enc
#define tight_quality_enc print_enc
#endif

static ggi_vnc_encode *
trle_enc(ggi_vnc_client *client,
	int32_t encoding, const char *format)
{
	struct ggi_visual *vis = client->owner;
	ggi_vnc_priv *priv = VNC_PRIV(vis);

	if (format)
		DPRINT_MISC(format);
	if (!priv->trle)
		return NULL;
	if (client->encode)
		return NULL;
	if (!client->trle_ctx)
		client->trle_ctx = GGI_vnc_trle_open();
	if (!client->trle_ctx)
		return NULL;

	return GGI_vnc_trle;
}

#ifdef HAVE_ZLIB
static ggi_vnc_encode *
zrle_enc(ggi_vnc_client *client,
	int32_t encoding, const char *format)
{
	struct ggi_visual *vis = client->owner;
	ggi_vnc_priv *priv = VNC_PRIV(vis);

	DPRINT_MISC(format);
	if (client->encode || priv->zrle_level == -2)
		return NULL;
	if (!client->zrle_ctx)
		client->zrle_ctx =
			GGI_vnc_zrle_open(priv->zrle_level);
	if (!client->zrle_ctx)
		return NULL;

	return GGI_vnc_zrle;
}
#else
#define zrle_enc print_enc
#endif

static ggi_vnc_encode *
wmvi_enc(ggi_vnc_client *client,
	int32_t encoding, const char *format)
{
	struct ggi_visual *vis = client->owner;
	ggi_vnc_priv *priv = VNC_PRIV(vis);

	DPRINT_MISC(format);
	if (!priv->wmvi)
		return NULL;

	if ((client->desktop_size & DESKSIZE_WMVI_PIXFMT)
		== DESKSIZE_PIXFMT)
	{
		client->pixfmt = *LIBGGI_PIXFMT(vis);
		if (client->pixfmt.size <= 8)
			client->pixfmt.size = 8;
		else if (client->pixfmt.size <= 16)
			client->pixfmt.size = 16;
		else
			client->pixfmt.size = 32;
		client->requested_pixfmt = client->pixfmt;
		change_pixfmt(client);
	}

	client->desktop_size |= DESKSIZE_OK | DESKSIZE_WMVI;

	return NULL;
}

static ggi_vnc_encode *
desktop_size_enc(ggi_vnc_client *client,
	int32_t encoding, const char *format)
{
	struct ggi_visual *vis = client->owner;
	ggi_vnc_priv *priv = VNC_PRIV(vis);

	DPRINT_MISC(format);
	if (!priv->desktop_size)
		return NULL;

	client->desktop_size |= DESKSIZE_OK;

	return NULL;
}

static ggi_vnc_encode *
gii_enc(ggi_vnc_client *client,
	int32_t encoding, const char *format)
{
	DPRINT_MISC(format);
	if (client->gii == 1) {
		client->gii = 2;

		/* ack gii.
		 * XXX send a pseudo-rect instead?
		 */
		DPRINT("enabling gii events\n");
		GGI_vnc_buf_reserve(&client->wbuf, 8);
		client->wbuf.size += 8;
		client->wbuf.buf[0] = 253;
		client->wbuf.buf[1] = 0x81;
		insert_hilo_16(&client->wbuf.buf[2], 4);
		insert_hilo_16(&client->wbuf.buf[4], 1);
		insert_hilo_16(&client->wbuf.buf[6], 1);
		client->safe_write(client, 0);
	}

	return NULL;
}

static ggi_vnc_encode *
desktop_name_enc(ggi_vnc_client *client,
	int32_t encoding, const char *format)
{
	struct ggi_visual *vis = client->owner;
	ggi_vnc_priv *priv = VNC_PRIV(vis);

	DPRINT_MISC(format);
	if (!priv->desktop_name)
		return NULL;

	client->desktop_name |= DESKNAME_OK;

	return NULL;
}


struct encodings {
	int32_t encoding;
	int32_t base;
	const char *format;
	vnc_encoding *action;
};

struct encodings encode_tbl[] = {
	{      0,    0, "Raw encoding\n",                 raw_enc },
	{      1,    0, "CopyRect encoding\n",            copyrect_enc },
	{      2,    0, "RRE encoding\n",                 rre_enc },
	{      4,    0, "CoRRE encoding\n",               corre_enc },
	{      5,    0, "Hextile encoding\n",             hextile_enc },
	{      6,    0, "Zlib encoding\n",                zlib_enc },
	{      7,    0, "Tight encoding\n",               tight_enc },
	{      8,    0, "ZlibHex encoding\n",             zlibhex_enc },
	{      9,    0, "Ultra encoding\n",               print_enc },
	{     15,    0, "TRLE encoding\n",                trle_enc },
	{     16,    0, "ZRLE encoding\n",                zrle_enc },
	{     17,    0, "ZYWRLE encoding\n",              print_enc },
	{ 0x574d5669,0, "WMVi pseudo-encoding\n",         wmvi_enc },
	{ 0x574d56ff, 0x574d5600,
	                "VMWare %d encoding\n",           print_enc },
	{    -23,  -32, "Tight quality %d subencoding\n", tight_quality_enc },
	{   -219,    0, "Background pseudo-encoding\n",   print_enc },
	{   -223,    0, "DesktopSize pseudo-encoding\n",  desktop_size_enc },
	{   -224,    0, "LastRect pseudo-encoding\n",     print_enc },
	{   -232,    0, "PointerPos pseudo-encoding\n",   print_enc },
	{   -238,    0, "SoftCursor pseudo-encoding\n",   print_enc },
	{   -239,    0, "Cursor pseudo-encoding\n",       print_enc },
	{   -240,    0, "XCursor pseudo-encoding\n",      print_enc },
	{   -247, -256, "Tight compress %d subencoding\n",print_enc },
	{     -1, -256, "Tight %d subencoding\n",         print_enc },
	{   -257,    0, "PointerType pseudo-encoding\n",  print_enc },
	{   -257, -272, "Liguori %d pseudo-encoding\n",   print_enc },
	{   -273, -304, "VMWare %d pseudo-encoding\n",    print_enc },
	{   -305,    0, "gii pseudo-encoding\n",          gii_enc },
	{   -306,    0, "popa pseudo-encoding\n",         print_enc },
	{   -307,    0, "DesktopName pseudo-encoding\n",  desktop_name_enc },
	{   -308,    0, "ExtendedDesktopSize pseudo-encoding\n",
	                                                  print_enc },
	{   -309,    0, "xvp pseudo-encoding\n",          print_enc },
	{   -412, -512, "TurboVNC fine-grained quality %d subencoding\n",
	                                                  print_enc },
	{   -763, -768, "TurboVNC subsampling %d subencoding\n",
	                                                  print_enc },
	{ -65527,    0, "UltraZip encoding\n",            print_enc },
	{ -65528,    0, "SolMonoZip encoding\n",          print_enc },
	{ -65529,    0, "CacheZip encoding\n",            print_enc },
	{ -65530,    0, "XOREnable encoding\n",           print_enc },
	{ -65531,    0, "SolidColor encoding\n",          print_enc },
	{ -65532,    0, "XORMultiColor_Zlib encoding\n",  print_enc },
	{ -65533,    0, "XORMonoColor_Zlib encoding\n",   print_enc },
	{ -65534,    0, "XOR_Zlib encoding\n",            print_enc },
	{ -65535,    0, "CacheEnable encoding\n",         print_enc },
	{ -65536,    0, "Cache encoding\n",               print_enc },
	{      0,    0, "Unknown (%d) encoding\n",        NULL }
};

static void
set_encodings(ggi_vnc_client *client, int32_t *encodings, unsigned int count)
{
	ggi_vnc_encode *encode;
	int32_t encoding = 0;
	int i;

	while (count--) {
		encode = NULL;

		encoding = ntohl(*encodings++);
		for (i = 0; encode_tbl[i].action; ++i) {
			if (encode_tbl[i].base) {
				if (encoding < encode_tbl[i].base)
					continue;
				if (encode_tbl[i].encoding < encoding)
					continue;
			}
			else if (encoding != encode_tbl[i].encoding)
				continue;

			encode = encode_tbl[i].action(client,
				encoding - encode_tbl[i].base,
				encode_tbl[i].format);
			break;
		}
		if (!encode_tbl[i].action) {
			print_enc(client, encoding, encode_tbl[i - 1].format);
			continue;
		}

		if (!client->encode && encode) {
			DPRINT_MISC("Selected %s", encode_tbl[i].format);
			client->encode = encode;
			if (client->update_pixfmt) {
				client->pixfmt = client->requested_pixfmt;
				change_pixfmt(client);
			}
		}
	}
}

static int
vnc_client_set_encodings(ggi_vnc_client *client)
{
	uint16_t encoding_count;

	DPRINT("client_set_encodings\n");

	if (client->buf_size < 4) {
		/* wait for more data */
		client->action = vnc_client_set_encodings;
		return 0;
	}

	memcpy(&encoding_count,
		&client->buf[2],
		sizeof(encoding_count));
	encoding_count = ntohs(encoding_count);

	if (client->buf_size < 4 + 4 * encoding_count) {
		/* wait for more data */
		if (vnc_reserve(client, 4 + 4 * encoding_count))
			return 1;
		client->action = vnc_client_set_encodings;
		return 0;
	}

	client->desktop_size &= DESKSIZE_SEND;
	client->desktop_name &= ~DESKNAME_OK;

	client->update_pixfmt = 0;
#ifdef HAVE_ZLIB
	if (client->encode == GGI_vnc_tight)
		client->update_pixfmt = 1;
#endif
	client->encode = NULL;
	client->copy_rect = 0;

	DPRINT("%d encodings\n", encoding_count);

	set_encodings(client,
		(int32_t *)&client->buf[4],
		encoding_count);

	if ((client->desktop_size & DESKSIZE_OK_SEND) == DESKSIZE_SEND)
		/* pending desktop size no longer allowed, die */
		return 1;

	if (client->encode == NULL)
		client->encode = GGI_vnc_raw;

	return vnc_remove(client, 4 + 4 * encoding_count);
}

struct ggi_visual *
GGI_vnc_encode_init(ggi_vnc_client *client, ggi_rect *update,
	ggi_rect *vupdate, int *d_frame_num)
{
	struct ggi_visual *vis = client->owner;
	ggi_vnc_priv *priv = VNC_PRIV(vis);
	int r_frame_num;

	*vupdate = *update;
	ggi_rect_shift_xy(vupdate, vis->origin_x, vis->origin_y);

	ggi_rect_subtract(&client->dirty, vupdate);
	client->update.tl.x = client->update.br.x = 0;

	DPRINT("dirty %dx%d - %dx%d\n",
		client->dirty.tl.x, client->dirty.tl.y,
		client->dirty.br.x, client->dirty.br.y);

	if (!client->vis) {
		*d_frame_num = vis->d_frame_num;
		return priv->fb;
	}

	r_frame_num = _ggiGetReadFrame(priv->fb);
	_ggiSetReadFrame(priv->fb, _ggiGetDisplayFrame(vis));

	_ggiCrossBlit(priv->fb,
		vupdate->tl.x, vupdate->tl.y,
		ggi_rect_width(vupdate),
		ggi_rect_height(vupdate),
		client->vis,
		vupdate->tl.x, vupdate->tl.y);

	_ggiSetReadFrame(priv->fb, r_frame_num);
	*d_frame_num = 0;
	return client->vis;
}

static int
do_client_update(ggi_vnc_client *client, ggi_rect *update, int pan)
{
	struct ggi_visual *vis = client->owner;
	ggi_vnc_priv *priv = VNC_PRIV(vis);
	int rects = 0;
	uint8_t *fb_update;
	int fb_update_idx;
	ggi_vnc_encode *encode = client->encode;
	ggi_rect visible;

	if ((client->desktop_size & DESKSIZE_WMVI_OK_SEND) == DESKSIZE_SEND)
		/* A pending send sits there, but is not ok, die */
		return -1;

	if (client->palette_dirty) {
		struct ggi_visual *cvis;
		unsigned char *vnc_palette;
		unsigned char *dst;
		int colors;
		ggi_color *ggi_palette;
		int i;

		if (!client->vis)
			cvis = priv->fb;
		else
			cvis = client->vis;

		colors = 1 << GT_DEPTH(LIBGGI_GT(cvis));

		ggi_palette = _ggi_malloc(colors * sizeof(*ggi_palette));
		_ggiGetPalette(cvis, 0, colors, ggi_palette);

		GGI_vnc_buf_reserve(&client->wbuf, 6 + 6 * colors);
		client->wbuf.size += 6 + 6 * colors;
		vnc_palette = client->wbuf.buf;
		vnc_palette[0] = 1; /* set colormap */
		vnc_palette[1] = 0;
		insert_hilo_16(&vnc_palette[2], 0);
		insert_hilo_16(&vnc_palette[4], colors);

		dst = &vnc_palette[6];
		for (i = 0; i < colors; ++i) {
			dst = insert_hilo_16(dst, ggi_palette[i].r);
			dst = insert_hilo_16(dst, ggi_palette[i].g);
			dst = insert_hilo_16(dst, ggi_palette[i].b);
		}

		free(ggi_palette);
		client->palette_dirty = 0;
	}

	if (ggi_rect_isempty(update) && !pan
		&& client->desktop_name != DESKNAME_SEND)
	{
		goto done;
	}

	GGI_vnc_buf_reserve(&client->wbuf, client->wbuf.size + 4);
	fb_update_idx = client->wbuf.size;
	client->wbuf.size += 4;

	ggi_rect_set_xyxy(&visible,
		vis->origin_x, vis->origin_y,
		vis->origin_x + LIBGGI_X(vis), vis->origin_y + LIBGGI_Y(vis));
	ggi_rect_intersect(&client->dirty, &visible);

	if ((client->desktop_size & DESKSIZE_WMVI)
		&& (client->desktop_size & DESKSIZE_PIXFMT_SEND))
	{
		struct ggi_visual *cvis;
		unsigned char *wmvi;
		ggi_pixelformat *pixfmt;
		int size;

		cvis = client->vis ? client->vis : priv->fb;

		DPRINT_MISC("Sending WMVi\n");
		GGI_vnc_buf_reserve(&client->wbuf,
			client->wbuf.size + 28);
		wmvi = &client->wbuf.buf[client->wbuf.size];
		client->wbuf.size += 28;
		insert_hilo_16(&wmvi[0], 0);
		insert_hilo_16(&wmvi[2], 0);
		insert_hilo_16(&wmvi[4], LIBGGI_X(vis));
		insert_hilo_16(&wmvi[6], LIBGGI_Y(vis));
		insert_hilo_32(&wmvi[8], 0x574d5669);

		/* Only sizes 8, 16 and 32 allowed in RFB */
		size = GT_SIZE(LIBGGI_GT(cvis));
		if (size <= 8)
			size = 8;
		else if (size <= 16)
			size = 16;
		else
			size = 32;
		wmvi[12] = size;
		wmvi[13] = GT_DEPTH(LIBGGI_GT(cvis));
#ifdef GGI_BIG_ENDIAN
		wmvi[14] = 1;
#else
		wmvi[14] = 0;
#endif
		wmvi[15] = GT_SCHEME(LIBGGI_GT(cvis)) == GT_TRUECOLOR;
		pixfmt = LIBGGI_PIXFMT(cvis);
		insert_hilo_16(&wmvi[16], color_max(pixfmt->red_mask));
		insert_hilo_16(&wmvi[18], color_max(pixfmt->green_mask));
		insert_hilo_16(&wmvi[20], color_max(pixfmt->blue_mask));
		wmvi[22] = color_shift(pixfmt->red_mask);
		wmvi[23] = color_shift(pixfmt->green_mask);
		wmvi[24] = color_shift(pixfmt->blue_mask);
		wmvi[25] = 0;
		wmvi[26] = 0;
		wmvi[27] = 0;

		++rects;

		client->desktop_size &= ~DESKSIZE_PIXFMT_SEND;

		client->palette_dirty = !wmvi[15];
	}

	if (client->desktop_name == DESKNAME_SEND) {
		unsigned char *desktop_name;
		uint32_t length = strlen(priv->title);
		DPRINT_MISC("Sending desktop name \"%s\"\n", priv->title);
		DPRINT_MISC("- length %u\n", length);
		GGI_vnc_buf_reserve(&client->wbuf,
			client->wbuf.size + 16 + length);
		desktop_name = &client->wbuf.buf[client->wbuf.size];
		client->wbuf.size += 16 + length;
		insert_hilo_16(&desktop_name[0], 0);
		insert_hilo_16(&desktop_name[2], 0);
		insert_hilo_16(&desktop_name[4], 0);
		insert_hilo_16(&desktop_name[6], 0);
		insert_hilo_32(&desktop_name[8], -307);
		insert_hilo_32(&desktop_name[12], length);
		memcpy(&desktop_name[16], priv->title, length);

		++rects;

		client->desktop_name &= ~DESKNAME_PENDING;
	}

	if (pan) {
		if (client->copy_rect)
			rects += GGI_vnc_copyrect_pan(client, update);
		else {
			update->tl.x = update->tl.y = 0;
			update->br.x = LIBGGI_X(vis);
			update->br.y = LIBGGI_Y(vis);
		}
	}

	client->origin.x = vis->origin_x;
	client->origin.y = vis->origin_y;

	if (!ggi_rect_isempty(update)) {
		if (!encode)
			encode = GGI_vnc_raw;
		rects += encode(client, update);
	}
	client->update.tl.x = client->update.br.x = 0;

	if (client->desktop_size & DESKSIZE_SEND) {
		unsigned char *desktop_size;
		DPRINT_MISC("Sending desktop size %dx%d\n",
			LIBGGI_X(vis), LIBGGI_Y(vis));
		GGI_vnc_buf_reserve(&client->wbuf, client->wbuf.size + 12);
		desktop_size = &client->wbuf.buf[client->wbuf.size];
		client->wbuf.size += 12;
		insert_hilo_16(&desktop_size[0], 0);
		insert_hilo_16(&desktop_size[2], 0);
		insert_hilo_16(&desktop_size[4], LIBGGI_X(vis));
		insert_hilo_16(&desktop_size[6], LIBGGI_Y(vis));
		insert_hilo_32(&desktop_size[8], -223);

		++rects;

		client->desktop_size &= ~DESKSIZE_SEND;
	}

	if (rects) {
		fb_update = &client->wbuf.buf[fb_update_idx];
		fb_update[0] = 0; /* fb update */
		fb_update[1] = 0;
		insert_hilo_16(&fb_update[2], rects);
	}
	else
		client->wbuf.size -= 4;

done:
	if (client->wbuf.size)
		client->safe_write(client, 1);
	return 0;
}

static int
full_update(struct ggi_visual *vis)
{
	ggi_directbuffer *buf = LIBGGI_APPBUFS(vis)[vis->d_frame_num];
	if (LIBGGI_FLAGS(vis) & (GGIFLAG_ASYNC | GGIFLAG_TIDYBUF))
		return 0;
	return buf->resource->curactype & GGI_ACTYPE_WRITE;
}

static int
pending_client_update(ggi_vnc_client *client)
{
	struct ggi_visual *vis = client->owner;
	ggi_rect update, dirty;
	int pan = 0;
	int pseudo = 0;

	if (client->write_pending)
		return 0;

	update = client->update;

	if (!full_update(vis)) {
		if (!ggi_rect_isempty(&update)) {
			pan = vis->origin_x != client->origin.x ||
				vis->origin_y != client->origin.y;

			pseudo = client->desktop_name == DESKNAME_SEND;
		}

		dirty = client->dirty;
		ggi_rect_shift_xy(&dirty, -vis->origin_x, -vis->origin_y);
		ggi_rect_intersect(&update, &dirty);
	}
	else {
		/* should region outside 'update' be dirtied? */
	}

	if (ggi_rect_isempty(&update) && !pan && !pseudo)
		return 0;

	/* subtract updated rect from fdirty, use dirty as a tmp variable */
	dirty = update;
	ggi_rect_shift_xy(&dirty, vis->origin_x, vis->origin_y);
	ggi_rect_subtract(&client->fdirty, &dirty);

	return do_client_update(client, &update, pan);
}

static int
vnc_client_update(ggi_vnc_client *client)
{
	struct ggi_visual *vis = client->owner;
	int incremental;
	ggi_rect request, update, dirty, visible;
	int pan;

	if (client->buf_size < 10) {
		/* wait for more data */
		client->action = vnc_client_update;
		return 0;
	}

	client->desktop_size &= ~DESKSIZE_INIT;

	incremental = client->buf[1];
	memcpy(&request.tl.x, &client->buf[2], sizeof(request.tl.x));
	request.tl.x = ntohs(request.tl.x);
	memcpy(&request.tl.y, &client->buf[4], sizeof(request.tl.y));
	request.tl.y = ntohs(request.tl.y);
	memcpy(&request.br.x, &client->buf[6], sizeof(request.br.x));
	request.br.x = request.tl.x + ntohs(request.br.x);
	memcpy(&request.br.y, &client->buf[8], sizeof(request.br.y));
	request.br.y = request.tl.y + ntohs(request.br.y);

	DPRINT("client_update(%d, %dx%d - %dx%d)\n",
		incremental,
		request.tl.x, request.tl.y,
		request.br.x, request.br.y);

	visible.tl.x = visible.tl.y = 0;
	visible.br.x = LIBGGI_X(vis);
	visible.br.y = LIBGGI_Y(vis);
	ggi_rect_intersect(&request, &visible);

	if (ggi_rect_isempty(&request))
		goto done;

	if (client->write_pending) {
		/* overly anxious client, just remember the requested rect. */
		if (!incremental)
			ggi_rect_union(&client->dirty, &request);
		ggi_rect_union(&client->update, &request);
		goto done;
	}

	ggi_rect_union(&request, &client->update);

	update = request;

	pan = vis->origin_x != client->origin.x ||
		vis->origin_y != client->origin.y;

	if (incremental) {
		if (!full_update(vis)) {
			dirty = client->dirty;
			ggi_rect_shift_xy(&dirty,
				-vis->origin_x, -vis->origin_y);
			ggi_rect_intersect(&update, &dirty);
		}
		else {
			/* should region outside 'update' be dirtied? */
			pan = 0;
		}
		if (ggi_rect_isempty(&update) && !pan) {
			client->update = request;
			if (!client->palette_dirty &&
				client->desktop_name != DESKNAME_SEND)
			{
				goto done;
			}
		}
	}

	/* subtract updated rect from fdirty, use dirty as a tmp variable */
	dirty = update;
	ggi_rect_shift_xy(&dirty, vis->origin_x, vis->origin_y);
	ggi_rect_subtract(&client->fdirty, &dirty);

	if (do_client_update(client, &update, pan))
		return 1;

done:
	return vnc_remove(client, 10);
}

static int
vnc_client_key(ggi_vnc_client *client)
{
	struct ggi_visual *vis = client->owner;
	ggi_vnc_priv *priv = VNC_PRIV(vis);
	uint32_t key;

	if (client->buf_size < 8) {
		/* wait for more data */
		client->action = vnc_client_key;
		return 0;
	}

	if (client->input) {
		DPRINT("client_key\n");

		memcpy(&key, &client->buf[4], sizeof(key));
		priv->key(priv->gii_ctx, client->buf[1], ntohl(key));
	}

	return vnc_remove(client, 8);
}

static int
vnc_client_pointer(ggi_vnc_client *client)
{
	struct ggi_visual *vis = client->owner;
	ggi_vnc_priv *priv = VNC_PRIV(vis);
	uint16_t x;
	uint16_t y;

	if (client->buf_size < 6) {
		/* wait for more data */
		client->action = vnc_client_pointer;
		return 0;
	}

	if (client->input) {
		DPRINT("client_pointer\n");

		memcpy(&x, &client->buf[2], sizeof(x));
		memcpy(&y, &client->buf[4], sizeof(y));
		priv->pointer(priv->gii_ctx,
			client->buf[1], ntohs(x), ntohs(y));
	}

	return vnc_remove(client, 6);
}

static int
vnc_client_drain_cut(ggi_vnc_client *client)
{
	const int max_chunk = 0x100000;
	uint32_t rest;
	uint32_t limit;
	int chunk;

	DPRINT("client_drain_cut\n");

	if (client->buf_size < 4) {
		client->action = vnc_client_drain_cut;
		return 0;
	}

	memcpy(&rest, client->buf, sizeof(rest));
	rest = ntohl(rest);

	limit = max_chunk;
	if (client->buf_size - 4 < max_chunk)
		limit = client->buf_size - 4;

	chunk = rest < limit ? rest : limit;

	if (rest > limit) {
		memmove(&client->buf[4], &client->buf[4 + chunk],
			client->buf_size - (4 + chunk));
		client->buf_size -= chunk;
		rest -= limit;
		insert_hilo_32(client->buf, rest);
		if (chunk > 0)
			return vnc_client_drain_cut(client);
		client->action = vnc_client_drain_cut;
		return 0;
	}

	return vnc_remove(client, 4 + chunk);
}

static int
vnc_client_cut(ggi_vnc_client *client)
{
	const uint32_t max_count = 0x10000;
	uint32_t count;
	int len;

	DPRINT("client_cut\n");

	if (client->buf_size < 8) {
		/* wait for more data */
		client->action = vnc_client_cut;
		return 0;
	}

	memcpy(&count, &client->buf[4], sizeof(count));
	count = ntohl(count);
	len = count < max_count ? count : max_count;

	if (client->buf_size < 8 + len) {
		/* wait for more data */
		if (vnc_reserve(client, 8 + len))
			return 1;
		client->action = vnc_client_cut;
		return 0;
	}

	if (client->input) {
		struct ggi_vnc_cmddata_clipboard clip;
		clip.data = &client->buf[8];
		clip.size = len;
		ggBroadcast(client->owner->instance.channel,
			GGI_VNC_CLIPBOARD, &clip);
	}

	if (count > max_count) {
		/* remove all received stuff so far and adjust
		 * how much more is expected
		 */
		memmove(&client->buf[4], &client->buf[8 + len],
			client->buf_size - (8 + len));
		client->buf_size -= 4 + len;
		count -= max_count;
		insert_hilo_32(client->buf, count);
		return vnc_client_drain_cut(client);
	}

	return vnc_remove(client, 8 + count);
}

static int
vnc_client_scale(ggi_vnc_client *client)
{
	uint8_t scale;

	DPRINT("client_scale\n");

	if (client->buf_size < 4) {
		/* wait for more data */
		client->action = vnc_client_scale;
		return 0;
	}

	scale = client->buf[1];

	return vnc_remove(client, 4);
}

static int
vnc_client_inject_gii(ggi_vnc_client *client)
{
	struct ggi_visual *vis = client->owner;
	ggi_vnc_priv *priv = VNC_PRIV(vis);
	uint16_t count;
	uint8_t reverse;
	int zap = 0;

	DPRINT("client_inject_gii\n");

	if (client->buf_size < 4) {
		/* wait for more data */
		client->action = vnc_client_inject_gii;
		return 0;
	}

#ifdef GG_BIG_ENDIAN
	reverse = ~client->buf[1] & 0x80;
#else
	reverse = client->buf[1] & 0x80;
#endif

	memcpy(&count, &client->buf[2], sizeof(count));
	if (reverse)
		count = GGI_BYTEREV16(count);

	if (client->input && priv->inject && client->gii == 2) {
		zap = priv->inject(priv->gii_ctx, client,
			client->buf[1], count,
			&client->buf[4], client->buf_size - 4);
		if (zap < 0) {
			DPRINT("gii extension failure\n");
			return zap;
		}
	}
	else if (client->buf_size < 4 + count)
		zap = client->buf_size - 4;
	else
		zap = count;

	if (zap < count) {
		/* remove all received stuff so far and adjust
		 * how much more is expected
		 */
		count -= zap;
		if (reverse)
			count = GGI_BYTEREV16(count);
		client->buf_size -= zap;
		memmove(&client->buf[4],
			&client->buf[4 + zap], client->buf_size - 4);
		memcpy(&client->buf[2], &count, sizeof(count));

		/* wait for more data */
		client->action = vnc_client_inject_gii;
		return 0;
	}

	DPRINT("removing gii message\n");
	return vnc_remove(client, 4 + zap);
}

static int
vnc_client_run(ggi_vnc_client *client)
{
	int res = 0;

	while (client->buf_size) {
		/* DPRINT("client_run size %d cmd %d\n",
			priv->buf_size, priv->buf[0]); */
		switch (client->buf[0]) {
		case 0:
			res = vnc_client_pixfmt(client);
			break;
		case 2:
			res = vnc_client_set_encodings(client);
			break;
		case 3:
			res = vnc_client_update(client);
			break;
		case 4:
			res = vnc_client_key(client);
			break;
		case 5:
			res = vnc_client_pointer(client);
			break;
		case 6:
			res = vnc_client_cut(client);
			break;
		case 8:
			res = vnc_client_scale(client);
			break;
		case 253:
			res = vnc_client_inject_gii(client);
			break;
		default:
			DPRINT("client_run unknown type %d (size %d)\n",
				client->buf[0], client->buf_size);
			return 1;
		}

		if (res <= 0)
			break;
	}

	return res;
}

static inline uint8_t *
insert_cap(uint8_t *buf, uint32_t code, const char *vendor_name)
{
	buf = insert_hilo_32(buf, code);
	memcpy(buf, vendor_name, 12);
	return buf + 12;
}

int
GGI_vnc_client_init(ggi_vnc_client *client)
{
	struct ggi_visual *vis = client->owner;
	ggi_vnc_priv *priv = VNC_PRIV(vis);
	unsigned char *server_init;
	uint32_t tmp32;
	ggi_pixelformat *pixfmt;
	int size;

	if (client->write_pending)
		return 0;

	DPRINT("client_init\n");

	if (client->buf_size > 1) {
		DPRINT("Too much data.\n");
		return -1;
	}

	if (client->buf_size < 1)
		/* wait for more data */
		return 0;

	if (!client->buf[0]) {
		/* I'm not sharing.
		 * Kill'em all.
		 */
		ggi_vnc_client *i;
		ggi_vnc_client *next;
		for (i = GG_LIST_FIRST(&priv->clients); i; i = next) {
			next = GG_LIST_NEXT(i, siblings);
			if (i == client)
				/* Err, not *all*, not me, myself and I. */
				continue;
			close_client(i);
		}
	}

	client->buf_size = 0;
	client->action = vnc_client_run;

	GGI_vnc_buf_reserve(&client->wbuf, 24 + strlen(priv->title));
	client->wbuf.size += 24 + strlen(priv->title);
	server_init = client->wbuf.buf;
	insert_hilo_16(&server_init[0], LIBGGI_X(vis));
	insert_hilo_16(&server_init[2], LIBGGI_Y(vis));
	/* Only sizes 8, 16 and 32 allowed in RFB */
	size = GT_SIZE(LIBGGI_GT(vis));
	if (size <= 8)
		size = 8;
	else if (size <= 16)
		size = 16;
	else
		size = 32;
	server_init[4] = size;
	server_init[5] = GT_DEPTH(LIBGGI_GT(vis));
#ifdef GGI_BIG_ENDIAN
	server_init[6] = 1;
#else
	server_init[6] = 0;
#endif
	server_init[7] = GT_SCHEME(LIBGGI_GT(vis)) == GT_TRUECOLOR;
	pixfmt = LIBGGI_PIXFMT(vis);
	insert_hilo_16(&server_init[8], color_max(pixfmt->red_mask));
	insert_hilo_16(&server_init[10], color_max(pixfmt->green_mask));
	insert_hilo_16(&server_init[12], color_max(pixfmt->blue_mask));
	server_init[14] = color_shift(pixfmt->red_mask);
	server_init[15] = color_shift(pixfmt->green_mask);
	server_init[16] = color_shift(pixfmt->blue_mask);
	server_init[17] = 0;
	server_init[18] = 0;
	server_init[19] = 0;
	tmp32 = strlen(priv->title);
	insert_hilo_32(&server_init[20], tmp32);
	memcpy(&server_init[24], priv->title, tmp32);

	client->desktop_size &= ~DESKSIZE_OK;
	client->desktop_name &= ~DESKNAME_PENDING;

	client->pixfmt = *pixfmt;
	client->requested_pixfmt = client->pixfmt;
	change_pixfmt(client);

	if (client->tight_extension) {
		int idx = client->wbuf.size;
		unsigned char *tight_init;
		uint16_t server_messages = 0;
		uint16_t client_messages = 0;
		uint16_t encodings = 0;

		if (priv->gii) {
			++server_messages;
			++client_messages;
			++encodings;
		}
		encodings +=
			(priv->copyrect != 0) +
			priv->rre +
			priv->corre +
			priv->hextile +
#ifdef HAVE_ZLIB
			(priv->zlib_level != -2) +
			(priv->tight & 2) +
#ifdef HAVE_JPEG
			!!(priv->tight & 2) +
#endif /* HAVE_JPEG */
			(priv->zlibhex_level != -2) +
#endif /* HAVE_ZLIB */
			priv->trle +
#ifdef HAVE_ZLIB
			(priv->zrle_level != -2) +
#endif /* HAVE_ZLIB */
			priv->desktop_size +
			priv->desktop_name;

		GGI_vnc_buf_reserve(&client->wbuf,
			idx + 8 + (server_messages + client_messages +
			encodings) * 16);
		tight_init = &client->wbuf.buf[idx];
		insert_hilo_16(&tight_init[0], server_messages);
		insert_hilo_16(&tight_init[2], client_messages);
		insert_hilo_16(&tight_init[4], encodings);
		tight_init[6] = 0;
		tight_init[7] = 0;
		idx = 8;

		if (priv->gii) {
			/* server message */
			insert_cap(&tight_init[idx], 253, "GGI_" "GII_SERV");
			idx += 16;
			/* client message */
			insert_cap(&tight_init[idx], 253, "GGI_" "GII_CLNT");
			idx += 16;
		}

		/* encodings */
		if (priv->copyrect) {
			insert_cap(&tight_init[idx], 1, "STDV" "COPYRECT");
			idx += 16;
		}

		if (priv->rre) {
			insert_cap(&tight_init[idx], 2, "STDV" "RRE_____");
			idx += 16;
		}

		if (priv->corre) {
			insert_cap(&tight_init[idx], 4, "STDV" "CORRE___");
			idx += 16;
		}

		if (priv->hextile) {
			insert_cap(&tight_init[idx], 5, "STDV" "HEXTILE_");
			idx += 16;
		}

#ifdef HAVE_ZLIB
		if (priv->zlib_level != -2) {
			insert_cap(&tight_init[idx], 6, "TRDV" "ZLIB____");
			idx += 16;
		}

		if (priv->tight & 2) {
			insert_cap(&tight_init[idx], 7, "TGHT" "TIGHT___");
			idx += 16;
		}

		if (priv->zlibhex_level != -2) {
			insert_cap(&tight_init[idx], 8, "TRDV" "ZLIBHEX_");
			idx += 16;
		}

		if (priv->trle) {
			insert_cap(&tight_init[idx], 15, "STDV" "TRLE____");
			idx += 16;
		}

		if (priv->zrle_level != -2) {
			insert_cap(&tight_init[idx], 16, "STDV" "ZRLE____");
			idx += 16;
		}

		if (priv->tight & 2) {
			insert_cap(&tight_init[idx], -256, "TGHT" "COMPRLVL");
			idx += 16;
#ifdef HAVE_JPEG
			insert_cap(&tight_init[idx], -32, "TGHT" "JPEGQLVL");
			idx += 16;
#endif /* HAVE_JPEG */
		}
#endif /* HAVE_ZLIB */

		if (priv->desktop_size) {
			insert_cap(&tight_init[idx], -224, "TGHT" "NEWFBSIZ");
			idx += 16;
		}

		if (priv->gii) {
			insert_cap(&tight_init[idx], -305, "GGI_" "GII_____");
			idx += 16;
		}

		if (priv->desktop_name) {
			insert_cap(&tight_init[idx], -307, "TGHT" "DESKNAME");
			idx += 16;
		}

		client->wbuf.size += idx;
	}

	/* desired pixel-format */
	client->safe_write(client, 1);

	DPRINT("client_init done\n");

	return 0;
}

static int
vnc_client_challenge(ggi_vnc_client *client)
{
	struct ggi_visual *vis = client->owner;
	ggi_vnc_priv *priv = VNC_PRIV(vis);
	int ok = 0;

	if (client->write_pending)
		return 0;

	DPRINT("client_challenge\n");

	if (client->buf_size > 16) {
		DPRINT("Too much data.\n");
		return -1;
	}

	if (client->buf_size < 16)
		/* wait for more data */
		return 0;

	if (priv->passwd) {
		uint8_t good_response[16];

		DES_ecb_encrypt((DES_cblock *)&client->challenge[0],
			(DES_cblock *)&good_response[0],
			priv->passwd_ks, DES_ENCRYPT);
		DES_ecb_encrypt((DES_cblock *)&client->challenge[8],
			(DES_cblock *)&good_response[8],
			priv->passwd_ks, DES_ENCRYPT);

		if (!memcmp(good_response,
				client->buf, sizeof(good_response)))
			ok = 1;
	}
	if (!ok && priv->viewpw) {
		uint8_t good_response[16];

		DES_ecb_encrypt((DES_cblock *)&client->challenge[0],
			(DES_cblock *)&good_response[0],
			priv->viewpw_ks, DES_ENCRYPT);
		DES_ecb_encrypt((DES_cblock *)&client->challenge[8],
			(DES_cblock *)&good_response[8],
			priv->viewpw_ks, DES_ENCRYPT);

		if (!memcmp(good_response,
				client->buf, sizeof(good_response))) {
			ok = 1;
			client->input = 0;
		}
	}
	if (!ok && (priv->passwd || priv->viewpw)) {
		DPRINT("bad password.\n");
		return -1;
	}

	client->buf_size = 0;
	client->action = GGI_vnc_client_init;

	/* ok */
	GGI_vnc_buf_reserve(&client->wbuf, 4);
	client->wbuf.size += 4;
	insert_hilo_32(&client->wbuf.buf[0], 0);
	client->safe_write(client, 1);

	return 0;
}

int
GGI_vnc_client_vnc_auth(ggi_vnc_client *client)
{
	struct ggi_visual *vis = client->owner;
	ggi_vnc_priv *priv = VNC_PRIV(vis);
	unsigned int i;
	struct timeval now;
	DES_key_schedule random_ks;

	DPRINT("client_vnc_auth\n");

	if (priv->vencrypt && !client->vencrypt)
		return -1;
	if (!priv->passwd && !priv->viewpw)
		return -1;

	client->buf_size = 0;
	client->action = vnc_client_challenge;

	/* Mix in some bits that will change with time */
	ggCurTime(&now);
	for (i = 0; i < sizeof(now); ++i) {
		priv->random17[i & 7] += *(((uint8_t *)&now) + i);
		client->challenge[i & 7] ^= *(((uint8_t *)&now) + i);
		client->challenge[15 - (i & 7)] ^= *(((uint8_t *)&now) + i);
	}

	DES_set_key_unchecked((DES_cblock *)priv->random17, &random_ks);

	/* scramble using des to get the final challenge */
	DES_ecb_encrypt((DES_cblock *)&client->challenge[0],
		(DES_cblock *)&client->challenge[0],
		&random_ks, DES_ENCRYPT);
	/* chain the two blocks to to make them more dissimilar */
	for (i = 0; i < 8; ++i)
		client->challenge[i + 8] ^= client->challenge[i];
	DES_ecb_encrypt((DES_cblock *)&client->challenge[8],
		(DES_cblock *)&client->challenge[8],
		&random_ks, DES_ENCRYPT);

	/* challenge client */
	GGI_vnc_buf_reserve(&client->wbuf, 16);
	client->wbuf.size += 16;
	memcpy(client->wbuf.buf, client->challenge, 16);
	client->safe_write(client, 1);
	return 0;
}

static int
vnc_client_tight_security(ggi_vnc_client *client)
{
	struct ggi_visual *vis = client->owner;
	ggi_vnc_priv *priv = VNC_PRIV(vis);
	uint32_t security_type;

	if (client->write_pending)
		return 0;

	DPRINT("client_tight_security\n");

	if (client->buf_size < 4)
		/* wait for more data */
		return 0;

	memcpy(&security_type, &client->buf[0], sizeof(security_type));
	security_type = ntohl(security_type);

	if (client->buf_size > 4) {
		DPRINT("Too much data.\n");
		return -1;
	}

	switch (security_type) {
	case 2:
		if (priv->vencrypt && !client->vencrypt)
			break;
		if (GGI_vnc_client_vnc_auth(client))
			break;
		return 0;

	case 19:
		if (!priv->vencrypt || client->vencrypt)
			break;

		return GGI_vnc_client_vencrypt_security(client);
	}

	DPRINT("Invalid security type requested (%u)\n", security_type);
	return -1;
}

static int
vnc_client_security(ggi_vnc_client *client)
{
	struct ggi_visual *vis = client->owner;
	ggi_vnc_priv *priv = VNC_PRIV(vis);
	unsigned int security_type;

	if (client->write_pending)
		return 0;

	DPRINT("client_security\n");

	if (client->buf_size < 1)
		/* wait for more data */
		return 0;

	security_type = client->buf[0];

	if (client->protover == 7 && security_type == 1)
		/* Client init msg may follow */
		memmove(client->buf, client->buf + 1, --client->buf_size);
	else if (client->buf_size > 1) {
		DPRINT("Too much data.\n");
		return -1;
	}

	switch (security_type) {
	case 1:
		if (priv->vencrypt && !client->vencrypt)
			break;
		if (priv->passwd || priv->viewpw)
			break;
		client->action = GGI_vnc_client_init;

		if (client->protover == 7) {
			if (client->buf_size)
				return GGI_vnc_client_init(client);
			client->buf_size = 0;
			return 0;
		}
		client->buf_size = 0;

		/* ok */
		GGI_vnc_buf_reserve(&client->wbuf, 4);
		client->wbuf.size += 4;
		insert_hilo_32(&client->wbuf.buf[0], 0);
		client->safe_write(client, 1);
		return 0;

	case 2:
		if (priv->vencrypt && !client->vencrypt)
			break;
		if (GGI_vnc_client_vnc_auth(client))
			break;
		return 0;

	case 16:
		if (!(priv->tight & 1))
			break;
		client->tight_extension = 1;
		client->buf_size = 0;

		GGI_vnc_buf_reserve(&client->wbuf, 12);
		client->wbuf.size += 8;
		/* No tunnels supported */
		insert_hilo_32(&client->wbuf.buf[0], 0);

		if (priv->vencrypt && !client->vencrypt) {
			/* One security type supported */
			insert_hilo_32(&client->wbuf.buf[4], 1);
			/* vencrypt supported */
			GGI_vnc_buf_reserve(&client->wbuf,
				client->wbuf.size + 16);
			client->wbuf.size += 16;
			insert_hilo_32(&client->wbuf.buf[8], 19);
			memcpy(&client->wbuf.buf[12], "VENC" "VENCRYPT", 12);
			client->action = vnc_client_tight_security;
		}
		else if (priv->passwd || priv->viewpw) {
			/* One auth type supported */
			insert_hilo_32(&client->wbuf.buf[4], 1);
			/* vnc-auth supported */
			GGI_vnc_buf_reserve(&client->wbuf,
				client->wbuf.size + 16);
			client->wbuf.size += 16;
			insert_hilo_32(&client->wbuf.buf[8], 2);
			memcpy(&client->wbuf.buf[12], "STDV" "VNCAUTH_", 12);
			client->action = vnc_client_tight_security;
		}
		else {
			/* No authentication supported */
			insert_hilo_32(&client->wbuf.buf[4], 0);
			client->action = GGI_vnc_client_init;
			client->buf_size = 0;

			if (client->protover > 7) {
				/* ok */
				client->wbuf.size += 4;
				insert_hilo_32(&client->wbuf.buf[8], 0);
			}
		}
		client->safe_write(client, 1);
		return 0;

	case 19:
		if (!priv->vencrypt || client->vencrypt)
			break;

		return GGI_vnc_client_vencrypt_security(client);
	}

	DPRINT("Invalid security type requested (%u)\n", security_type);
	return -1;
}

static int
vnc_client_version(ggi_vnc_client *client)
{
	struct ggi_visual *vis = client->owner;
	ggi_vnc_priv *priv = VNC_PRIV(vis);
	unsigned int major, minor;
	char str[13];
	uint8_t idx;

	if (client->write_pending)
		return 0;

	if (client->buf_size > 12) {
		DPRINT("Too much data.\n");
		return -1;
	}

	if (client->buf_size < 12)
		/* wait for more data */
		return 0;

	client->buf[12] = '\0';
	major = atoi((char *)client->buf + 4);
	minor = atoi((char *)client->buf + 8);
	if (major > 999 || minor > 999) {
		DPRINT("Invalid protocol version requested\n");
		return -1;
	}
	snprintf(str, sizeof(str), "RFB %03u.%03u\n", major, minor);
	if (strcmp(str, (char *)client->buf)) {
		DPRINT("Invalid protocol version requested\n");
		return -1;
	}

	DPRINT("Client wants protocol version RFB %03u.%03u\n", major, minor);
	if (major == 3 && minor == 4)
		/* Interpret 3.4 as 3.3, 3.4 is UltraVNC specific, but
		 * apparently works just like 3.3. Unsure though...
		 */
		minor = 3;

	if (major == 3 && minor == 5)
		/* Interpret 3.5 as 3.3, 3.5 was never published */
		minor = 3;

	if ((major != 3) || (minor != 8 && minor != 7 && minor != 3)) {
		DPRINT("Can't handle requested protocol version "
			"(only 3.3 (3.4, 3.5), 3.7 and 3.8).\n");
		return -1;
	}

	client->protover = minor;

	if (client->protover <= 3) {
		client->buf_size = 0;
		client->action = GGI_vnc_client_init;

		/* ok, decide security */
		GGI_vnc_buf_reserve(&client->wbuf, 4);
		client->wbuf.size += 4;
		if (priv->vencrypt)
			return -1;
		if (priv->passwd || priv->viewpw)
			insert_hilo_32(&client->wbuf.buf[0], 2);
		else
			insert_hilo_32(&client->wbuf.buf[0], 1);
		client->safe_write(client, 1);
		if (priv->passwd || priv->viewpw) {
			/* fake a client request */
			client->buf[0] = 2;
			client->buf_size = 1;
			return vnc_client_security(client);
		}
		return 0;
	}

	client->buf_size = 0;
	client->action = vnc_client_security;

	/* supported security types */
	GGI_vnc_buf_reserve(&client->wbuf, 3);
	idx = 0;
	if (priv->tight & 1)
		client->wbuf.buf[++idx] = 16;
	if (priv->vencrypt)
		client->wbuf.buf[++idx] = 19;
	else if (priv->passwd || priv->viewpw)
		client->wbuf.buf[++idx] = 2;
	else
		client->wbuf.buf[++idx] = 1;
	client->wbuf.size += 1 + idx;
	client->wbuf.buf[0] = idx;
	client->safe_write(client, 1);

	return 0;
}

void
GGI_vnc_new_client_finish(struct ggi_visual *vis, int cfd, int cwfd)
{
	ggi_vnc_priv *priv = VNC_PRIV(vis);
	ggi_vnc_client *client;
	long flags;
	struct linger linger;

	DPRINT("new_client(%d, %d)\n", cfd, cwfd);

	client = _ggi_calloc(sizeof(*client));
	client->buf_limit = 256;
	client->buf = _ggi_malloc(client->buf_limit);

	GG_LIST_INSERT_HEAD(&priv->clients, client, siblings);
	client->owner = vis;

	client->cfd = cfd;
	client->cwfd = cwfd;
	client->input = !priv->view_only;
	client->dirty.tl.x = 0;
	client->dirty.tl.y = 0;
	client->dirty.br.x = LIBGGI_VIRTX(vis);
	client->dirty.br.y = LIBGGI_VIRTY(vis);

	priv->add_cfd(priv->gii_ctx, client, client->cfd);

	client->write_ready = GGI_vnc_write_ready;
	client->read_ready = GGI_vnc_read_ready;
	client->safe_write = vnc_safe_write;

	client->tight_extension = 0;
	client->write_pending = 0;
	client->desktop_size = priv->desktop_size ? DESKSIZE_OK_INIT : 0;
	client->gii = priv->gii;
	client->desktop_name = 0;

#if defined(F_GETFL)
	flags = fcntl(client->cfd, F_GETFL);
	fcntl(client->cfd, F_SETFL, flags | O_NONBLOCK);

	if (cfd != cwfd) {
		flags = fcntl(client->cwfd, F_GETFL);
		fcntl(client->cwfd, F_SETFL, flags | O_NONBLOCK);
	}
#elif defined(FIONBIO)
	flags = 1;
	ioctlsocket(client->cfd, FIONBIO, &flags);

	if (cfd != cwfd)
		ioctlsocket(client->cwfd, FIONBIO, &flags);
#endif

	/* Don't linger on close, shutdown harshly */
	linger.l_onoff = 1;
	linger.l_linger = 0;
	setsockopt(client->cwfd, SOL_SOCKET,
		SO_LINGER, &linger, sizeof(linger));

	client->action = vnc_client_version;

	/* Support max protocol version 3.8 */
	GGI_vnc_buf_reserve(&client->wbuf, 12);
	client->wbuf.size += 12;
	memcpy(client->wbuf.buf, "RFB 003.008\n", 12);
	client->safe_write(client, 1);
}

void
GGI_vnc_new_client(void *arg)
{
	struct ggi_visual *vis = arg;
	ggi_vnc_priv *priv = VNC_PRIV(vis);
#ifdef HAVE_GETADDRINFO
	struct sockaddr_storage sa;
	char address[100];
	char service[50];
#else
	struct sockaddr_in sa;
#endif
#ifdef HAVE_SOCKLEN_T
	socklen_t sa_len = sizeof(sa);
#else
	int sa_len = sizeof(sa);
#endif
	int cfd;

	cfd = accept(priv->sfd, (struct sockaddr *)&sa, &sa_len);
	if (cfd == -1) {
		DPRINT("Error accept(2)ing connection\n");
		return;
	}

#if defined(HAVE_GETADDRINFO)
	if (getnameinfo((struct sockaddr *)&sa, sa_len,
		address, sizeof(address), service, sizeof(service),
		NI_NUMERICHOST | NI_NUMERICSERV) != 0)
	{
		ggstrlcpy(address, "???", sizeof(address));
		ggstrlcpy(service, "???", sizeof(service));
	}
	DPRINT("fd %d connecting from %s service %s\n",
		cfd, address, service);
#elif defined(HAVE_ARPA_INET_H)
	DPRINT("fd %d connecting from %s:%d\n",
		cfd, inet_ntoa(sa.sin_addr),
		ntohs(sa.sin_port));
#endif

	GGI_vnc_new_client_finish(vis, cfd, cfd);
}

int
GGI_vnc_read_ready(ggi_vnc_client *client)
{
	unsigned char buf[100];
	ssize_t len;

#if defined(__WIN32__) && !defined(__CYGWIN__)
	len = recv(client->cfd, buf, sizeof(buf), 0);
#else
	len = read(client->cfd, buf, sizeof(buf));
#endif

	if (len < 0) {
		DPRINT("Error reading\n");
		close_client(client);
		return -1;
	}

	if (len == 0) {
		close_client(client);
		return -1;
	}

	if (len + client->buf_size > client->buf_limit) {
		DPRINT("Avoiding buffer overrun\n");
		close_client(client);
		return -1;
	}

	memcpy(client->buf + client->buf_size, buf, len);
	client->buf_size += len;

	if (client->action(client)) {
		close_client(client);
		return -1;
	}
	return 0;
}

int
GGI_vnc_write_ready(ggi_vnc_client *client)
{
	struct ggi_visual *vis = client->owner;
	ggi_vnc_priv *priv = VNC_PRIV(vis);

	if (!client->write_pending) {
		DPRINT("spurious write completed notification\n");
		return 0;
	}

	/* DPRINT("write some more\n"); */

	client->write_pending = 0;
	priv->del_cwfd(priv->gii_ctx, client, client->cwfd);

	if (client->safe_write(client, 1) < 0)
		return -1;

	if (client->write_pending)
		return 0;

	if (client->buf_size) {
		if (client->action(client)) {
			close_client(client);
			return -1;
		}
	}

	if (!ggi_rect_isempty(&client->update))
		pending_client_update(client);

	return 0;
}

int
GGI_vnc_client_data(void *arg, int cfd)
{
	ggi_vnc_client *client = arg;
	return client->read_ready(client);
}

int
GGI_vnc_write_client(void *arg, int fd)
{
	ggi_vnc_client *client = arg;
	return client->write_ready(client);
}

int
GGI_vnc_safe_write(void *arg, const uint8_t *data, int count)
{
	ggi_vnc_client *client = arg;

	GGI_vnc_buf_reserve(&client->wbuf, client->wbuf.size + count);
	memcpy(client->wbuf.buf + client->wbuf.size, data, count);
	client->wbuf.size += count;

	if (client->write_pending)
		return 0;

	return client->safe_write(client, 0);
}

void
GGI_vnc_close_client(ggi_vnc_client *client)
{
	close_client(client);
}

int
GGI_vnc_change_pixfmt(ggi_vnc_client *client)
{
	return change_pixfmt(client);
}

void
GGI_vnc_client_invalidate_nc_xyxy(ggi_vnc_client *client,
	int tlx, int tly, int brx, int bry)
{
	ggi_rect_union_xyxy(&client->dirty, tlx, tly, brx, bry);

	if (client->cfd != -1 && !ggi_rect_isempty(&client->update))
		pending_client_update(client);
}

void
GGI_vnc_invalidate_nc_xyxy(struct ggi_visual *vis,
	int tlx, int tly, int brx, int bry)
{
	ggi_vnc_priv *priv = VNC_PRIV(vis);
	ggi_vnc_client *client;

	GG_LIST_FOREACH(client, &priv->clients, siblings) {
		ggi_rect_union_xyxy(&client->dirty, tlx, tly, brx, bry);

		if (client->cfd != -1 && !ggi_rect_isempty(&client->update))
			pending_client_update(client);
	}
}

void
GGI_vnc_invalidate_xyxy(struct ggi_visual *vis,
	int tlx, int tly, int brx, int bry)
{
	ggi_vnc_priv *priv;
	ggi_vnc_client *client;

	if (vis->w_frame_num != vis->d_frame_num)
		/* invalidating invisible frame, who cares... */
		return;

	if (tlx < LIBGGI_GC(vis)->cliptl.x)
		tlx = LIBGGI_GC(vis)->cliptl.x;
	if (LIBGGI_GC(vis)->clipbr.x < brx)
		brx = LIBGGI_GC(vis)->clipbr.x;
	if (tly < LIBGGI_GC(vis)->cliptl.y)
		tly = LIBGGI_GC(vis)->cliptl.y;
	if (LIBGGI_GC(vis)->clipbr.y < bry)
		bry = LIBGGI_GC(vis)->clipbr.y;

	if (brx <= tlx || bry <= tly)
		return;

	priv = VNC_PRIV(vis);

	if (LIBGGI_FLAGS(vis) & GGIFLAG_ASYNC) {
		GG_LIST_FOREACH(client, &priv->clients, siblings)
			ggi_rect_union_xyxy(&client->fdirty,
				tlx, tly, brx, bry);
		return;
	}

	GG_LIST_FOREACH(client, &priv->clients, siblings) {
		ggi_rect_union_xyxy(&client->dirty, tlx, tly, brx, bry);

		if (client->cfd != -1 && !ggi_rect_isempty(&client->update))
			pending_client_update(client);
	}
}

void
GGI_vnc_invalidate_palette(struct ggi_visual *vis)
{
	ggi_vnc_priv *priv = VNC_PRIV(vis);
	ggi_vnc_client *client;

	GG_LIST_FOREACH(client, &priv->clients, siblings) {
		if (client->vis) {
			/* non-matching client pixfmt, trigger crossblit */
			client->dirty.tl.x = vis->origin_x;
			client->dirty.tl.y = vis->origin_y;
			client->dirty.br.x = vis->origin_x + LIBGGI_X(vis);
			client->dirty.br.y = vis->origin_y + LIBGGI_Y(vis);
		}
		else
			client->palette_dirty = 1;

		if (client->cfd != -1 && !ggi_rect_isempty(&client->update))
			pending_client_update(client);
	}
}
