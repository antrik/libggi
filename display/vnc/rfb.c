/* $Id: rfb.c,v 1.44 2006/09/03 13:19:56 pekberg Exp $
******************************************************************************

   display-vnc: RFB protocol

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
#include <ctype.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif

#include <sys/types.h>
#ifdef HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif
#ifdef HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif

#include <errno.h>

#include <ggi/gg.h>
#include <ggi/display/vnc.h>
#include <ggi/input/vnc.h>
#include <ggi/internal/ggi_debug.h>
#include "rect.h"

#include "d3des.h"
#include "encoding.h"

static int vnc_client_run(struct ggi_visual *vis);

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

	if (!buf->size) {
		if (buf->buf != NULL)
			free(buf->buf);
		buf->buf = malloc(limit);
	}
	else {
		unsigned char *tmp;
		tmp = realloc(buf->buf, limit);
		if (tmp == NULL)
			return 1;
		buf->buf = tmp;
	}

	if (buf->buf)
		buf->limit = limit;

	return 0;
}

static void
close_client(ggi_vnc_priv *priv, ggi_vnc_client *client)
{
	if (!client)
		return;
	if (client->cfd == -1)
		return;

	DPRINT("close_client.\n");
	priv->del_cfd(priv->gii_ctx, client->cfd);
	if (client->write_pending)
		priv->del_cwfd(priv->gii_ctx, client->cfd);
	client->write_pending = 0;
	if (client->wbuf.buf) {
		free(client->wbuf.buf);
		memset(&client->wbuf, 0, sizeof(client->wbuf));
	}
	close(client->cfd);
	client->cfd = -1;
	client->buf_size = 0;
	client->dirty.tl.x = client->dirty.br.x = 0;
	client->update.tl.x = client->update.br.x = 0;

#ifdef HAVE_ZLIB
	if (client->zlib_ctx) {
		GGI_vnc_zlib_close(client->zlib_ctx);
		client->zlib_ctx = NULL;
	}

	if (client->zrle_ctx) {
		GGI_vnc_zrle_close(client->zrle_ctx);
		client->zrle_ctx = NULL;
	}
#endif
	free(client);
	priv->client = NULL;
}

static int
write_client(ggi_vnc_priv *priv, ggi_vnc_buf *buf)
{
	ggi_vnc_client *client = priv->client;
	int res;

	if (client->write_pending) {
		DPRINT("impatient client...\n");
		close_client(priv, client);
	}

again:
	res = write(client->cfd, buf->buf + buf->pos, buf->size - buf->pos);

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
#if EAGAIN != EWOULDBLOCK
	case EWOULDBLOCK:
#endif
		/* queue it */
		/* DPRINT("write would block\n"); */
		client->write_pending = 1;
		priv->add_cwfd(priv->gii_ctx, client->cfd);
		return 0;
	default:
		DPRINT("write error.\n");
		close_client(priv, client);
		return res;
	}
}

static int
vnc_remove(struct ggi_visual *vis, int count)
{
	ggi_vnc_priv *priv = VNC_PRIV(vis);
	ggi_vnc_client *client = priv->client;

	memmove(client->buf, &client->buf[count], client->buf_size - count);
	client->buf_size -= count;

	if (client->action == vnc_client_run)
		return client->buf_size;

	client->action = vnc_client_run;
	return vnc_client_run(vis);
}

static int
vnc_client_pixfmt(struct ggi_visual *vis)
{
	ggi_vnc_priv *priv = VNC_PRIV(vis);
	ggi_vnc_client *client = priv->client;
	int err = 0;
	uint16_t red_max;
	uint16_t green_max;
	uint16_t blue_max;
	ggi_mode mode;
	ggi_pixelformat pixfmt;
	struct ggi_visual fake_vis;
	int i;
	char target[GGI_MAX_APILEN];
	struct gg_stem *stem;

	DPRINT("client_pixfmt\n");

	if (client->buf_size < 20) {
		/* wait for more data */
		client->action = vnc_client_pixfmt;
		return 0;
	}

	if (client->vis) {
		stem = client->vis->stem;
		ggiClose(stem);
		ggDelStem(stem);
		client->vis = NULL;
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

	memset(&pixfmt, 0, sizeof(ggi_pixelformat));
	pixfmt.size = client->buf[4];
	/* Don't trust depth from the client, they often
	 * ask for the wrong thing...
	 */
	if (client->buf[7]) {
		pixfmt.depth = color_bits(red_max) +
			color_bits(green_max) +
			color_bits(blue_max);
		if (pixfmt.size < pixfmt.depth)
			pixfmt.depth = client->buf[5];
		pixfmt.red_mask = red_max << client->buf[14];
		pixfmt.green_mask = green_max << client->buf[15];
		pixfmt.blue_mask = blue_max << client->buf[16];
	}
	else {
		pixfmt.depth = client->buf[5];
		pixfmt.clut_mask = (1 << pixfmt.depth) - 1;
	}

	_ggi_build_pixfmt(&pixfmt);
	DPRINT_MISC("Evaluated as GGI pixfmt:\n");
	DPRINT_MISC("  depth/size: %d/%d\n", pixfmt.depth, pixfmt.size);
	DPRINT_MISC("  red mask (shift):   %08x (%d)\n",
		pixfmt.red_mask,   pixfmt.red_shift);
	DPRINT_MISC("  green mask (shift): %08x (%d)\n",
		pixfmt.green_mask, pixfmt.green_shift);
	DPRINT_MISC("  blue mask (shift):  %08x (%d)\n",
		pixfmt.blue_mask,  pixfmt.blue_shift);
	DPRINT_MISC("  clut mask (shift):  %08x (%d)\n",
		pixfmt.clut_mask,  pixfmt.clut_shift);

	client->palette_dirty = !client->buf[7];

	if (pixfmt.red_mask == LIBGGI_PIXFMT(vis)->red_mask &&
		pixfmt.green_mask == LIBGGI_PIXFMT(vis)->green_mask &&
		pixfmt.blue_mask == LIBGGI_PIXFMT(vis)->blue_mask &&
		pixfmt.clut_mask == LIBGGI_PIXFMT(vis)->clut_mask &&
		pixfmt.size == LIBGGI_PIXFMT(vis)->size)
		goto done;

	memset(&mode, 0, sizeof(mode));
	mode.frames = 1;
	mode.visible.x = LIBGGI_X(vis);
	mode.visible.y = LIBGGI_Y(vis);
	mode.virt.x    = LIBGGI_VIRTX(vis);
	mode.virt.y    = LIBGGI_VIRTY(vis);
	mode.size.x = mode.size.y = GGI_AUTO;
	GT_SETDEPTH(mode.graphtype, pixfmt.depth);
	GT_SETSIZE(mode.graphtype, pixfmt.size);
	if (client->buf[7])
		GT_SETSCHEME(mode.graphtype, GT_TRUECOLOR);
	else
		GT_SETSCHEME(mode.graphtype, GT_PALETTE);
	mode.dpp.x = mode.dpp.y = 1;

	if (client->buf[7]) {
		i = snprintf(target,
			GGI_MAX_APILEN, "display-memory:-pixfmt=");

		/* Need a visual to call _ggi_build_pixfmtstr, so fake
		 * one...
		 */
		fake_vis.pixfmt = &pixfmt;
		fake_vis.mode = &mode;
		memset(target + i, '\0', sizeof(target) - i);
		_ggi_build_pixfmtstr(&fake_vis,
			target + i, sizeof(target) - i, GGI_PIXFMT_CHANNEL);
		i = strlen(target);
	}
	else
		i = snprintf(target, GGI_MAX_APILEN, "display-memory");

	stem = ggNewStem();
	if (stem == NULL) {
		DPRINT("ggNewStem failed\n");
		err = -1;
		goto err0;
	}
	if (ggiAttach(stem) != GGI_OK) {
		DPRINT("ggiAttach failed\n");
		err = -1;
		goto err1;
	}
	if (ggiOpen(stem, target, NULL) != GGI_OK) {
		DPRINT("ggiOpen failed\n");
		err = -1;
		goto err2;
	}
	client->vis = STEM_API_DATA(stem, libggi, struct ggi_visual *);
	if (ggiSetMode(stem, &mode)) {
		DPRINT("ggiSetMode failed\n");
		err = -1;
		goto err3;
	}

	DPRINT_MISC("fb stdformat 0x%x, cvis stdformat 0x%x\n",
		priv->fb->pixfmt->stdformat,
		client->vis->pixfmt->stdformat);

	if (!client->buf[7])
		ggiSetColorfulPalette(stem);

done:
	return vnc_remove(vis, 20);

err3:
	ggiClose(stem);
err2:
	ggiDetach(stem);
err1:
	ggDelStem(stem);
err0:
	client->vis = NULL;
	return err;

}

static void
set_encodings(ggi_vnc_priv *priv, int32_t *encodings, unsigned int count)
{
	ggi_vnc_client *client = priv->client;
	ggi_vnc_encode *encode;

	while (count--) {
		encode = NULL;

		switch(ntohl(*encodings++)) {
		case 0:
			DPRINT_MISC("Raw encoding\n");
			encode = GGI_vnc_raw;
			break;
		case 1:
			DPRINT_MISC("CopyRect encoding\n");
			break;
		case 2:
			DPRINT_MISC("RRE encoding\n");
			break;
		case 4:
			DPRINT_MISC("CoRRE encoding\n");
			break;
		case 5:
			DPRINT_MISC("Hextile encoding\n");
			break;
		case 16:
			DPRINT_MISC("ZRLE encoding\n");
#ifdef HAVE_ZLIB
			if (client->encode || priv->zrle_level == -2)
				break;
			if (!client->zrle_ctx)
				client->zrle_ctx =
					GGI_vnc_zrle_open(priv->zrle_level);
			if (client->zrle_ctx)
				encode = GGI_vnc_zrle;
#endif
			break;
		case -239:
			DPRINT_MISC("Cursor pseudo-encoding\n");
			break;
		case -223:
			DPRINT_MISC("DesktopSize pseudo-encoding\n");
			break;
		case 6:
			DPRINT_MISC("zlib encoding\n");
#ifdef HAVE_ZLIB
			if (client->encode || priv->zlib_level == -2)
				break;
			if (!client->zlib_ctx)
				client->zlib_ctx = GGI_vnc_zlib_open(-1);
			if (client->zlib_ctx)
				encode = GGI_vnc_zlib;
#endif
			break;
		case 7:
			DPRINT_MISC("tight encoding\n");
			break;
		case 8:
			DPRINT_MISC("zlibhex encoding\n");
			break;
		case -256:
		case -255:
		case -254:
		case -253:
		case -252:
		case -251:
		case -250:
		case -249:
		case -248:
		case -247:
			DPRINT_MISC("tight compression %d subencoding\n",
				ntohl(*(encodings-1)) + 256);
			break;
		case -32:
		case -31:
		case -30:
		case -29:
		case -28:
		case -27:
		case -26:
		case -25:
		case -24:
		case -23:
			DPRINT_MISC("tight quality %d subencoding\n",
				ntohl(*(encodings-1)) + 32);
			break;
		default:
			DPRINT_MISC("Unknown (%i) encoding\n",
				ntohl(*encodings));
			break;
		}

		if (client->encode == NULL)
			client->encode = encode;
	}
}

static int
vnc_client_set_encodings_cont(struct ggi_visual *vis)
{
	ggi_vnc_priv *priv = VNC_PRIV(vis);
	ggi_vnc_client *client = priv->client;

	DPRINT("client_set_encodings (cont)\n");

	if (client->buf_size < 4) {
		/* wait for more data */
		return 0;
	}

	if (client->buf_size < 4 * client->encoding_count) {
		/* wait for more data */
		int tail = client->buf_size & 3;
		unsigned int count = client->buf_size / 4;
		set_encodings(priv, (int32_t *)client->buf, count);
		client->encoding_count -= count;
		memcpy(client->buf,
			&client->buf[client->buf_size - tail],
			tail);
		client->buf_size = tail;
		/* wait for more data */
		return 0;
	}

	set_encodings(priv, (int32_t *)client->buf, client->encoding_count);

	if (client->encode == NULL)
		client->encode = GGI_vnc_raw;

	return vnc_remove(vis, 4 * client->encoding_count);
}

static int
vnc_client_set_encodings(struct ggi_visual *vis)
{
	ggi_vnc_priv *priv = VNC_PRIV(vis);
	ggi_vnc_client *client = priv->client;

	DPRINT("client_set_encodings\n");

	if (client->buf_size < 4) {
		/* wait for more data */
		client->action = vnc_client_set_encodings;
		return 0;
	}

	client->encode = NULL;

	memcpy(&client->encoding_count,
		&client->buf[2],
		sizeof(client->encoding_count));
	client->encoding_count = ntohs(client->encoding_count);

	DPRINT("%d encodings\n", client->encoding_count);

	if (client->buf_size < 4 + 4 * client->encoding_count) {
		/* wait for more data */
		int tail = client->buf_size & 3;
		unsigned int count = client->buf_size / 4 - 1;
		set_encodings(priv, (int32_t *)&client->buf[4], count);
		client->encoding_count -= count;
		memcpy(client->buf,
			&client->buf[client->buf_size - tail],
			tail);
		client->buf_size = tail;

		/* wait for more data */
		client->action = vnc_client_set_encodings_cont;
		return 0;
	}

	set_encodings(priv,
		(int32_t *)&client->buf[4],
		client->encoding_count);

	if (client->encode == NULL)
		client->encode = GGI_vnc_raw;

	return vnc_remove(vis, 4 + 4 * client->encoding_count);
}

static void
do_client_update(struct ggi_visual *vis, ggi_rect *update)
{
	ggi_vnc_priv *priv = VNC_PRIV(vis);
	ggi_vnc_client *client = priv->client;

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

		ggi_palette = malloc(colors * sizeof(*ggi_palette));
		_ggiGetPalette(cvis, 0, colors, ggi_palette);

		GGI_vnc_buf_reserve(&client->wbuf, 6 + 6 * colors);
		client->wbuf.size += 6 + 6 * colors;
		vnc_palette = client->wbuf.buf;
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
		client->palette_dirty = 0;
	}

	if (ggi_rect_isempty(update))
		goto done;

	if (!client->encode)
		client->encode = GGI_vnc_raw;

	client->encode(vis, update);

done:
	write_client(priv, &client->wbuf);
}

static void
pending_client_update(struct ggi_visual *vis)
{
	ggi_vnc_priv *priv = VNC_PRIV(vis);
	ggi_vnc_client *client = priv->client;
	ggi_rect update, dirty;

	if (client->write_pending)
		return;

	update = client->update;
	dirty = client->dirty;
	ggi_rect_shift_xy(&dirty, -vis->origin_x, -vis->origin_y);
	ggi_rect_intersect(&update, &dirty);

	if (ggi_rect_isempty(&update) && !client->palette_dirty)
		return;

	do_client_update(vis, &update);
}

static int
vnc_client_update(struct ggi_visual *vis)
{
	ggi_vnc_priv *priv = VNC_PRIV(vis);
	ggi_vnc_client *client = priv->client;
	int incremental;
	ggi_rect request, update, dirty;

	if (client->buf_size < 10) {
		/* wait for more data */
		client->action = vnc_client_update;
		return 0;
	}

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

	if (ggi_rect_isempty(&request))
		goto done;

	ggi_rect_union(&request, &client->update);

	if (client->write_pending) {
		/* overly anxious client, just remember the requested rect.
		 * TODO: remember this event and send a new update request
		 * when the outstanding write completes. Also remember if
		 * any such anxious request were non-incremental.
		 */
		client->update = update;
		goto done;
	}

	update = request;

	if (incremental) {
		dirty = client->dirty;
		ggi_rect_shift_xy(&dirty, -vis->origin_x, -vis->origin_y);
		ggi_rect_intersect(&update, &dirty);
		if (ggi_rect_isempty(&update)) {
			client->update = request;
			if (!client->palette_dirty)
				goto done;
		}
	}

	do_client_update(vis, &update);

done:
	return vnc_remove(vis, 10);
}

static int
vnc_client_key(struct ggi_visual *vis)
{
	ggi_vnc_priv *priv = VNC_PRIV(vis);
	ggi_vnc_client *client = priv->client;
	uint32_t key;

	DPRINT("client_key\n");

	if (client->buf_size < 8) {
		/* wait for more data */
		client->action = vnc_client_key;
		return 0;
	}

	memcpy(&key, &client->buf[4], sizeof(key));
	priv->key(priv->gii_ctx, client->buf[1], ntohl(key));

	return vnc_remove(vis, 8);
}

static int
vnc_client_pointer(struct ggi_visual *vis)
{
	ggi_vnc_priv *priv = VNC_PRIV(vis);
	ggi_vnc_client *client = priv->client;
	uint16_t x;
	uint16_t y;

	DPRINT("client_pointer\n");

	if (client->buf_size < 6) {
		/* wait for more data */
		client->action = vnc_client_pointer;
		return 0;
	}

	memcpy(&x, &client->buf[2], sizeof(x));
	memcpy(&y, &client->buf[4], sizeof(y));
	priv->pointer(priv->gii_ctx, client->buf[1], ntohs(x), ntohs(y));

	return vnc_remove(vis, 6);
}

static int
vnc_client_cut(struct ggi_visual *vis)
{
	ggi_vnc_priv *priv = VNC_PRIV(vis);
	ggi_vnc_client *client = priv->client;
	uint32_t count;

	DPRINT("client_cut\n");

	if (client->buf_size < 8) {
		/* wait for more data */
		client->action = vnc_client_cut;
		return 0;
	}

	memcpy(&count, &client->buf[4], sizeof(count));
	count = ntohl(count);

	if (client->buf_size < 8 + count) {
		/* wait for more data */
		client->action = vnc_client_cut;

		/* remove all received stuff so far and adjust
		 * how much more is expected
		 */
		count = htonl(count - (client->buf_size - 8));
		client->buf_size = 8;
		memcpy(&client->buf[4], &count, sizeof(count));
		return 0;
	}

	return vnc_remove(vis, 8 + count);
}

static int
vnc_client_run(struct ggi_visual *vis)
{
	ggi_vnc_priv *priv = VNC_PRIV(vis);
	ggi_vnc_client *client = priv->client;
	int res = 0;

	while (client->buf_size) {
		/* DPRINT("client_run size %d cmd %d\n",
			priv->buf_size, priv->buf[0]); */
		switch (client->buf[0]) {
		case 0:
			res = vnc_client_pixfmt(vis);
			break;
		case 2:
			res = vnc_client_set_encodings(vis);
			break;
		case 3:
			res = vnc_client_update(vis);
			break;
		case 4:
			res = vnc_client_key(vis);
			break;
		case 5:
			res = vnc_client_pointer(vis);
			break;
		case 6:
			res = vnc_client_cut(vis);
			break;
		default:
			DPRINT("client_run unknown type %d\n",
				client->buf[0]);
			return 1;
		}

		if (res <= 0)
			break;
	}

	return res;
}

static int
vnc_client_init(struct ggi_visual *vis)
{
	ggi_vnc_priv *priv = VNC_PRIV(vis);
	ggi_vnc_client *client = priv->client;
	unsigned char *server_init;
	uint16_t tmp16;
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

	if (client->buf[0])
		DPRINT("Client want's to share connection. Not supported.\n");

	client->buf_size = 0;
	client->action = vnc_client_run;

	GGI_vnc_buf_reserve(&client->wbuf, 24 + strlen(priv->title));
	client->wbuf.size += 24 + strlen(priv->title);
	server_init = client->wbuf.buf;
	tmp16 = htons(LIBGGI_X(vis));
	memcpy(&server_init[0],  &tmp16, sizeof(tmp16));
	tmp16 = htons(LIBGGI_Y(vis));
	memcpy(&server_init[2],  &tmp16, sizeof(tmp16));
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
	tmp16 = htons(color_max(pixfmt->red_mask));
	memcpy(&server_init[8],  &tmp16, sizeof(tmp16));
	tmp16 = htons(color_max(pixfmt->green_mask));
	memcpy(&server_init[10], &tmp16, sizeof(tmp16));
	tmp16 = htons(color_max(pixfmt->blue_mask));
	memcpy(&server_init[12], &tmp16, sizeof(tmp16));
	server_init[14] = color_shift(pixfmt->red_mask);
	server_init[15] = color_shift(pixfmt->green_mask);
	server_init[16] = color_shift(pixfmt->blue_mask);
	server_init[17] = 0;
	server_init[18] = 0;
	server_init[19] = 0;
	tmp32 = htonl(strlen(priv->title));
	memcpy(&server_init[20], &tmp32, sizeof(tmp32));
	memcpy(&server_init[24], priv->title, ntohl(tmp32));

	/* desired pixel-format */
	write_client(priv, &client->wbuf);

	DPRINT("client_init done\n");

	return 0;
}

static int
vnc_client_challenge(struct ggi_visual *vis)
{
	ggi_vnc_priv *priv = VNC_PRIV(vis);
	ggi_vnc_client *client = priv->client;

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

		usekey(priv->cooked_key);
		des(&client->challenge[0], &good_response[0]);
		des(&client->challenge[8], &good_response[8]);

		if (memcmp(good_response,
				client->buf, sizeof(good_response))) {
			DPRINT("bad password.\n");
			return -1;
		}
	}

	client->buf_size = 0;
	client->action = vnc_client_init;

	/* ok */
	GGI_vnc_buf_reserve(&client->wbuf, 4);
	client->wbuf.size += 4;
	client->wbuf.buf[0] = 0;
	client->wbuf.buf[1] = 0;
	client->wbuf.buf[2] = 0;
	client->wbuf.buf[3] = 0;
	write_client(priv, &client->wbuf);

	return 0;
}

static int
vnc_client_security(struct ggi_visual *vis)
{
	ggi_vnc_priv *priv = VNC_PRIV(vis);
	ggi_vnc_client *client = priv->client;
	unsigned int security_type;
	int i;
	struct timeval now;

	if (client->write_pending)
		return 0;

	DPRINT("client_security\n");

	if (client->buf_size > 1) {
		DPRINT("Too much data.\n");
		return -1;
	}

	if (client->buf_size < 1)
		/* wait for more data */
		return 0;

	security_type = client->buf[0];
	switch (security_type) {
	case 1:
		if (priv->passwd)
			break;
		client->buf_size = 0;
		client->action = vnc_client_init;

		/* ok */
		GGI_vnc_buf_reserve(&client->wbuf, 4);
		client->wbuf.size += 4;
		client->wbuf.buf[0] = 0;
		client->wbuf.buf[1] = 0;
		client->wbuf.buf[2] = 0;
		client->wbuf.buf[3] = 0;
		write_client(priv, &client->wbuf);
		return 0;

	case 2:
		if (!priv->passwd)
			break;
		client->buf_size = 0;
		client->action = vnc_client_challenge;

		/* Mix in some bits that will change with time */
		ggCurTime(&now);
		for (i = 0; i < sizeof(now); ++i)
			client->challenge[i & 7] ^= *(((uint8_t *)&now) + i);

		/* scramble using des to get the final challenge */
		usekey(priv->randomizer);
		des(&client->challenge[0], &client->challenge[0]);
		/* chain the two blocks to propagate the "randomness" */
		for (i = 0; i < 8; ++i)
			client->challenge[i + 8] ^= client->challenge[i];
		des(&client->challenge[8], &client->challenge[8]);

		/* challenge client */
		GGI_vnc_buf_reserve(&client->wbuf, 16);
		client->wbuf.size += 16;
		memcpy(client->wbuf.buf, client->challenge, 16);
		write_client(priv, &client->wbuf);
		return 0;
	}

	DPRINT("Invalid security type requested (%u)\n", security_type);
	return -1;
}

static int
vnc_client_version(struct ggi_visual *vis)
{
	ggi_vnc_priv *priv = VNC_PRIV(vis);
	ggi_vnc_client *client = priv->client;
	unsigned int major, minor;
	char str[13];

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
	sprintf(str, "RFB %03u.%03u\n", major, minor);
	if (strcmp(str, (char *)client->buf)) {
		DPRINT("Invalid protocol version requested\n");
		return -1;
	}

	DPRINT("Client wants protocol version RFB %03u.%03u\n", major, minor);
	if (major == 3 && minor == 5)
		/* Interpret 3.5 as 3.3, 3.5 was never published */
		minor = 3;

	if ((major != 3) || (minor != 8 && minor != 7 && minor != 3)) {
		DPRINT("Can't handle requested protocol version "
			"(only 3.3, 3.7 and 3.8).\n");
		return -1;
	}

	client->protover = minor;

	if (client->protover <= 3) {
		client->buf_size = 0;
		client->action = vnc_client_init;

		/* ok, decide security */
		GGI_vnc_buf_reserve(&client->wbuf, 4);
		client->wbuf.size += 4;
		client->wbuf.buf[0] = 0;
		client->wbuf.buf[1] = 0;
		client->wbuf.buf[2] = 0;
		client->wbuf.buf[3] = priv->passwd ? 2 : 1;
		write_client(priv, &client->wbuf);
		if (priv->passwd) {
			/* fake a client request of vnc auth security type */
			client->buf[0] = 2;
			client->buf_size = 1;
			return vnc_client_security(vis);
		}
		return 0;
	}

	client->buf_size = 0;
	client->action = vnc_client_security;

	/* supported security types */
	GGI_vnc_buf_reserve(&client->wbuf, 2);
	client->wbuf.size += 2;
	client->wbuf.buf[0] = 1;
	client->wbuf.buf[1] = priv->passwd ? 2 : 1;
	write_client(priv, &client->wbuf);

	return 0;
}

void
GGI_vnc_new_client_finish(struct ggi_visual *vis, int cfd)
{
	ggi_vnc_priv *priv = VNC_PRIV(vis);
	ggi_vnc_client *client;
	long flags;

	DPRINT("new_client(%d)\n", cfd);

	priv->client = client = malloc(sizeof(*priv->client));
	memset(client, 0, sizeof(*client));
	client->cfd = cfd;
	client->dirty.tl.x = 0;
	client->dirty.tl.y = 0;
	client->dirty.br.x = LIBGGI_VIRTX(vis);
	client->dirty.br.y = LIBGGI_VIRTY(vis);

	priv->add_cfd(priv->gii_ctx, client->cfd);

	client->write_pending = 0;

	flags = fcntl(client->cfd, F_GETFL);
	fcntl(client->cfd, F_SETFL, flags | O_NONBLOCK);

	client->action = vnc_client_version;

	/* Support max protocol version 3.8 */
	GGI_vnc_buf_reserve(&client->wbuf, 12);
	client->wbuf.size += 12;
	memcpy(client->wbuf.buf, "RFB 003.008\n", 12);
	write_client(priv, &client->wbuf);
}

void
GGI_vnc_new_client(void *arg)
{
	struct ggi_visual *vis = arg;
	ggi_vnc_priv *priv = VNC_PRIV(vis);
	struct sockaddr_in sa;
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

	close_client(priv, priv->client);

	GGI_vnc_new_client_finish(vis, cfd);
}

void
GGI_vnc_client_data(void *arg, int cfd)
{
	struct ggi_visual *vis = arg;
	ggi_vnc_priv *priv = VNC_PRIV(vis);
	ggi_vnc_client *client = priv->client;
	unsigned char buf[100];
	ssize_t len;

	if (cfd != client->cfd)
		return;

	len = read(cfd, buf, sizeof(buf));

	if (len < 0) {
		DPRINT("Error reading\n");
		close_client(priv, client);
		return;
	}

	if (len == 0) {
		close_client(priv, client);
		return;
	}

	if (len + client->buf_size > sizeof(client->buf)) {
		DPRINT("Avoiding buffer overrun\n");
		close_client(priv, client);
		return;
	}

	memcpy(client->buf + client->buf_size, buf, len);
	client->buf_size += len;

	if (client->action(vis)) {
		close_client(priv, client);
		return;
	}
}

void
GGI_vnc_write_client(void *arg, int fd)
{
	struct ggi_visual *vis = arg;
	ggi_vnc_priv *priv = VNC_PRIV(vis);
	ggi_vnc_client *client = priv->client;

	if (fd != client->cfd)
		return;

	if (!client->write_pending) {
		DPRINT("spurious write completed notification\n");
		return;
	}

	/* DPRINT("write some more\n"); */

	client->write_pending = 0;
	priv->del_cwfd(priv->gii_ctx, fd);

	if (write_client(priv, &client->wbuf) < 0)
		return;

	if (client->write_pending)
		return;

	if (!client->buf_size)
		return;

	if (client->action(vis)) {
		close_client(priv, client);
		return;
	}
}

void
GGI_vnc_close_client(struct ggi_visual *vis)
{
	ggi_vnc_priv *priv = VNC_PRIV(vis);
	ggi_vnc_client *client = priv->client;

	close_client(priv, client);
}

void
GGI_vnc_invalidate_nc_xyxy(struct ggi_visual *vis,
	uint16_t tlx, uint16_t tly, uint16_t brx, uint16_t bry)
{
	ggi_vnc_priv *priv = VNC_PRIV(vis);
	ggi_vnc_client *client = priv->client;

	if (!client)
		return;
	ggi_rect_union_xyxy(&client->dirty, tlx, tly, brx, bry);

	if (client->cfd != -1 && !ggi_rect_isempty(&client->update))
		pending_client_update(vis);
}

void
GGI_vnc_invalidate_xyxy(struct ggi_visual *vis,
	uint16_t tlx, uint16_t tly, uint16_t brx, uint16_t bry)
{
	ggi_vnc_priv *priv;
	ggi_vnc_client *client;

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
	client = priv->client;
	if (!client)
		return;
	ggi_rect_union_xyxy(&client->dirty, tlx, tly, brx, bry);

	if (client->cfd != -1 && !ggi_rect_isempty(&client->update))
		pending_client_update(vis);
}

void
GGI_vnc_invalidate_palette(struct ggi_visual *vis)
{
	ggi_vnc_priv *priv = VNC_PRIV(vis);
	ggi_vnc_client *client = priv->client;

	if (!client)
		return;

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
		pending_client_update(vis);
}