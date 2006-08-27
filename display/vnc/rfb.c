/* $Id: rfb.c,v 1.15 2006/08/27 12:24:04 cegger Exp $
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

#include <ggi/gg.h>
#include <ggi/display/vnc.h>
#include <ggi/input/vnc.h>
#include <ggi/internal/ggi_debug.h>
#include "rect.h"

#include "d3des.h"

static int vnc_client_run(struct ggi_visual *vis);

static int
vnc_remove(struct ggi_visual *vis, int count)
{
	ggi_vnc_priv *priv = VNC_PRIV(vis);

	memmove(priv->buf, &priv->buf[count], priv->buf_size - count);
	priv->buf_size -= count;

	if (priv->client_action == vnc_client_run)
		return priv->buf_size;

	priv->client_action = vnc_client_run;
	return vnc_client_run(vis);
}

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
vnc_client_pixfmt(struct ggi_visual *vis)
{
	ggi_vnc_priv *priv = VNC_PRIV(vis);
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

	if (priv->buf_size < 20) {
		/* wait for more data */
		priv->client_action = vnc_client_pixfmt;
		return 0;
	}

	if (priv->client_vis) {
		stem = priv->client_vis->stem;
		ggiClose(stem);
		ggDelStem(stem);
		priv->client_vis = NULL;
	}

	memcpy(&red_max, &priv->buf[8], sizeof(red_max));
	red_max = ntohs(red_max);
	memcpy(&green_max, &priv->buf[10], sizeof(green_max));
	green_max = ntohs(green_max);
	memcpy(&blue_max, &priv->buf[12], sizeof(blue_max));
	blue_max = ntohs(blue_max);

	DPRINT_MISC("Requested pixfmt:\n");
	DPRINT_MISC("  depth/size %d/%d\n", priv->buf[5], priv->buf[4]);
#ifdef GGI_BIG_ENDIAN
	priv->reverse_endian = !priv->buf[6];
#else
	priv->reverse_endian = priv->buf[6];
#endif
	DPRINT_MISC("  endian: %s\n",
		priv->reverse_endian ? "reverse" : "match");
	DPRINT_MISC("  type: %s\n",
		priv->buf[7] ? "truecolor" : "palette");
	DPRINT_MISC("  red max (shift):   %u (%d)\n",
		red_max,   priv->buf[14]);
	DPRINT_MISC("  green max (shift): %u (%d)\n",
		green_max, priv->buf[15]);
	DPRINT_MISC("  blue max (shift):  %u (%d)\n",
		blue_max,  priv->buf[16]);

	memset(&pixfmt, 0, sizeof(ggi_pixelformat));
	pixfmt.size = priv->buf[4];
	/* Don't trust depth from the client, they often
	 * ask for the wrong thing...
	 */
	if (priv->buf[7]) {
		pixfmt.depth = color_bits(red_max) +
			color_bits(green_max) +
			color_bits(blue_max);
		if (pixfmt.size < pixfmt.depth)
			pixfmt.depth = priv->buf[5];
		pixfmt.red_mask = red_max << priv->buf[14];
		pixfmt.green_mask = green_max << priv->buf[15];
		pixfmt.blue_mask = blue_max << priv->buf[16];
	}
	else {
		pixfmt.depth = priv->buf[5];
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

	mode.frames = 1;
	mode.visible.x = LIBGGI_VIRTX(vis);
	mode.visible.y = LIBGGI_VIRTY(vis);
	mode.virt = mode.visible;
	mode.size.x = mode.size.y = GGI_AUTO;
	GT_SETDEPTH(mode.graphtype, pixfmt.depth);
	GT_SETSIZE(mode.graphtype, pixfmt.size);
	if (priv->buf[7])
		GT_SETSCHEME(mode.graphtype, GT_TRUECOLOR);
	else
		GT_SETSCHEME(mode.graphtype, GT_PALETTE);
	mode.dpp.x = mode.dpp.y = 1;

	if (priv->buf[7]) {
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
		return -1;
	}
	if (ggiAttach(stem) != GGI_OK) {
		DPRINT("ggiAttach failed\n");
		ggDelStem(stem);
		return -1;
	}
	if (ggiOpen(stem, target, NULL) != GGI_OK) {
		DPRINT("ggiOpen failed\n");
		ggDelStem(stem);
		return -1;
	}
	priv->client_vis = STEM_API_DATA(stem, libggi, struct ggi_visual *);
	if (ggiSetMode(priv->client_vis->stem, &mode)) {
		struct gg_stem *client_stem = priv->client_vis->stem;

		DPRINT("ggiSetMode failed\n");
		ggiClose(priv->client_vis->stem);
		ggDelStem(client_stem);
		priv->client_vis = NULL;
		return -1;
	}

	if (!priv->buf[7]) {
		ggiSetColorfulPalette(priv->client_vis->stem);
		priv->palette_dirty = 1;
	}
	else
		priv->palette_dirty = 0;

	return vnc_remove(vis, 20);
}

static int
vnc_client_set_encodings(struct ggi_visual *vis)
{
	ggi_vnc_priv *priv = VNC_PRIV(vis);
	uint16_t count;

	DPRINT("client_set_encodings\n");

	if (priv->buf_size < 4) {
		/* wait for more data */
		priv->client_action = vnc_client_set_encodings;
		return 0;
	}

	memcpy(&count, &priv->buf[2], sizeof(count));
	count = ntohs(count);

	if (priv->buf_size < 4 + 4 * count) {
		/* wait for more data */
		priv->client_action = vnc_client_set_encodings;
		return 0;
	}

	DPRINT("got %d encodings\n", count);

	return vnc_remove(vis, 4 + 4 * count);
}

static void
do_client_update(struct ggi_visual *vis, ggi_rect *update)
{
	ggi_vnc_priv *priv = VNC_PRIV(vis);
	struct ggi_visual *cvis;
	const ggi_directbuffer *db;
	unsigned char header[16];
	ggi_graphtype gt;

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

		vnc_palette = malloc(6 + 6 * colors);
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

		write(priv->cfd, vnc_palette, 6 + 6 * colors);
		free(ggi_palette);
		priv->palette_dirty = 0;
	}

	db = ggiDBGetBuffer(cvis->stem, 0);
	ggiResourceAcquire(db->resource, GGI_ACTYPE_READ);

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
	header[15] = 0;
	write(priv->cfd, &header, sizeof(header));
	if (priv->reverse_endian && GT_SIZE(gt) > 8) {
		int i, j;
		int bpp = GT_ByPP(gt);
		int count = (update->br.x - update->tl.x) *
			(update->br.y - update->tl.y);
		void *buf = malloc(count * bpp);
		int stride = db->buffer.plb.stride / bpp;

		if (bpp == 2) {
			uint16_t *tmp = (uint16_t *)buf;
			uint16_t *src = (uint16_t *)db->read +
				update->tl.x + update->tl.y * stride;
			for (j = 0; j < update->br.y - update->tl.y; ++j) {
				for (i = 0; i < update->br.x - update->tl.x; ++i)
					*tmp++ = GGI_BYTEREV16(src[i]);
				src += stride;
			}
		}
		else if (bpp == 4) {
			uint32_t *tmp = (uint32_t *)buf;
			uint32_t *src = (uint32_t *)db->read +
				update->tl.x + update->tl.y * stride;
			for (j = 0; j < update->br.y - update->tl.y; ++j) {
				for (i = 0; i < update->br.x - update->tl.x; ++i)
					*tmp++ = GGI_BYTEREV32(src[i]);
				src += stride;
			}
		}
		else { /* bpp == 3 */
			uint8_t *tmp = (uint8_t *)buf;
			uint8_t *src = (uint8_t *)db->read +
				update->tl.x + update->tl.y * stride * bpp;
			for (j = 0; j < update->br.y - update->tl.y; ++j) {
				for (i = 0; i < update->br.x - update->tl.x; ++i) {
					*tmp++ = src[i * 3 + 2];
					*tmp++ = src[i * 3 + 1];
					*tmp++ = src[i * 3];
				}
				src += stride * bpp;
			}
		}
		write(priv->cfd, buf, count * bpp);
		free(buf);
	}
	else if (update->br.x - update->tl.x != LIBGGI_VIRTX(cvis)) {
		int i;
		int bpp = GT_ByPP(gt);
		int count = (update->br.x - update->tl.x) * (update->br.y - update->tl.y);
		uint8_t *buf = malloc(count * bpp);
		uint8_t *dst = buf;

		for (i = update->tl.y; i < update->br.y; ++i, dst += (update->br.x - update->tl.x) * bpp)
			memcpy(dst,
				(uint8_t *)db->read + (update->tl.x + i * LIBGGI_VIRTX(cvis)) * bpp,
				(update->br.x - update->tl.x) * bpp);

		write(priv->cfd, buf, count * bpp);
		free(buf);
	}
	else
		write(priv->cfd,
			(uint8_t *)db->read + GT_ByPPP(LIBGGI_VIRTX(cvis) * update->tl.y, gt),
			GT_ByPPP(LIBGGI_VIRTX(cvis) * (update->br.y - update->tl.y), gt));

	ggiResourceRelease(db->resource);
}

static void
pending_client_update(struct ggi_visual *vis)
{
	ggi_vnc_priv *priv = VNC_PRIV(vis);
	ggi_rect update;

	update = priv->update;
	ggi_rect_intersect(&update, &priv->dirty);

	if (ggi_rect_isempty(&update))
		return;

	do_client_update(vis, &update);
}

static int
vnc_client_update(struct ggi_visual *vis)
{
	ggi_vnc_priv *priv = VNC_PRIV(vis);
	int incremental;
	ggi_rect request, update;

	if (priv->buf_size < 10) {
		/* wait for more data */
		priv->client_action = vnc_client_update;
		return 0;
	}

	incremental = priv->buf[1];
	memcpy(&request.tl.x, &priv->buf[2], sizeof(request.tl.x));
	request.tl.x = ntohs(request.tl.x);
	memcpy(&request.tl.y, &priv->buf[4], sizeof(request.tl.y));
	request.tl.y = ntohs(request.tl.y);
	memcpy(&request.br.x, &priv->buf[6], sizeof(request.br.x));
	request.br.x = request.tl.x + ntohs(request.br.x);
	memcpy(&request.br.y, &priv->buf[8], sizeof(request.br.y));
	request.br.y = request.tl.y + ntohs(request.br.y);

	DPRINT("client_update(%d, %dx%d - %dx%d)\n",
		incremental,
		request.tl.x, request.tl.y,
		request.br.x, request.br.y);

	if (ggi_rect_isempty(&request))
		goto done;

	ggi_rect_union(&request, &priv->update);
	update = request;

	if (incremental) {
		ggi_rect_intersect(&update, &priv->dirty);
		if (ggi_rect_isempty(&update)) {
			priv->update = request;
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
	uint32_t key;

	DPRINT("client_key\n");

	if (priv->buf_size < 8) {
		/* wait for more data */
		priv->client_action = vnc_client_key;
		return 0;
	}

	memcpy(&key, &priv->buf[4], sizeof(key));
	priv->key(priv->gii_ctx, priv->buf[1], ntohl(key));

	return vnc_remove(vis, 8);
}

static int
vnc_client_pointer(struct ggi_visual *vis)
{
	ggi_vnc_priv *priv = VNC_PRIV(vis);
	uint16_t x;
	uint16_t y;

	DPRINT("client_pointer\n");

	if (priv->buf_size < 6) {
		/* wait for more data */
		priv->client_action = vnc_client_pointer;
		return 0;
	}

	memcpy(&x, &priv->buf[2], sizeof(x));
	memcpy(&y, &priv->buf[4], sizeof(y));
	priv->pointer(priv->gii_ctx, priv->buf[1], ntohs(x), ntohs(y));

	return vnc_remove(vis, 6);
}

static int
vnc_client_cut(struct ggi_visual *vis)
{
	ggi_vnc_priv *priv = VNC_PRIV(vis);
	uint32_t count;

	DPRINT("client_cut\n");

	if (priv->buf_size < 8) {
		/* wait for more data */
		priv->client_action = vnc_client_cut;
		return 0;
	}

	memcpy(&count, &priv->buf[4], sizeof(count));
	count = ntohl(count);

	if (priv->buf_size < 8 + count) {
		/* wait for more data */
		priv->client_action = vnc_client_cut;
		return 0;
	}

	return vnc_remove(vis, 8 + count);
}

static int
vnc_client_run(struct ggi_visual *vis)
{
	ggi_vnc_priv *priv = VNC_PRIV(vis);
	int res = 0;

	while (priv->buf_size) {
		switch (priv->buf[0]) {
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
			DPRINT("client_run unknown type %d\n", priv->buf[0]);
			return 1;
		}

		if (res <= 0)
			break;
	}

	return res;
}

static int
color_max(ggi_pixel mask)
{
	while (!(mask & 1))
		mask >>= 1;

	return mask;
}

static int
color_shift(ggi_pixel mask)
{
	int shift = 0;

	while (!(mask & 1)) {
		mask >>= 1;
		++shift;
	}

	return shift;
}

static int
vnc_client_init(struct ggi_visual *vis)
{
	ggi_vnc_priv *priv = VNC_PRIV(vis);
	unsigned char server_init[34];
	uint16_t tmp16;
	uint32_t tmp32;
	ggi_pixelformat *pixfmt;
	int size;

	DPRINT("client_init\n");

	if (priv->buf_size > 1) {
		DPRINT("Too much data.\n");
		return -1;
	}

	if (priv->buf_size < 1)
		/* wait for more data */
		return 0;

	if (priv->buf[0])
		DPRINT("Client want's to share connection. Not supported.\n");

	priv->buf_size = 0;
	priv->client_action = vnc_client_run;

	tmp16 = htons(LIBGGI_VIRTX(vis));
	memcpy(&server_init[0],  &tmp16, sizeof(tmp16));
	tmp16 = htons(LIBGGI_VIRTY(vis));
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
	tmp32 = htonl(10);
	memcpy(&server_init[20], &tmp32, sizeof(tmp32));
	memcpy(&server_init[24], "GGI on vnc", ntohl(tmp32));

	/* block signals? */

	/* desired pixel-format */
	write(priv->cfd, server_init, sizeof(server_init));

	return 0;
}

static int
vnc_client_challenge(struct ggi_visual *vis)
{
	ggi_vnc_priv *priv = VNC_PRIV(vis);
	uint32_t result = htonl(0);

	DPRINT("client_challenge\n");

	if (priv->buf_size > 16) {
		DPRINT("Too much data.\n");
		return -1;
	}

	if (priv->buf_size < 16)
		/* wait for more data */
		return 0;

	if (priv->passwd) {
		uint8_t good_response[16];

		usekey(priv->cooked_key);
		des(&priv->challenge[0], &good_response[0]);
		des(&priv->challenge[8], &good_response[8]);

		if (memcmp(good_response, priv->buf, sizeof(good_response))) {
			DPRINT("bad password.\n");
			return -1;
		}
	}

	priv->buf_size = 0;
	priv->client_action = vnc_client_init;

	/* block signals? */

	/* ok */
	write(priv->cfd, &result, sizeof(result));

	return 0;
}

static int
vnc_client_security(struct ggi_visual *vis)
{
	ggi_vnc_priv *priv = VNC_PRIV(vis);
	unsigned int security_type;
	uint32_t ok = htonl(0);
	int i;
	struct timeval now;

	DPRINT("client_security\n");

	if (priv->buf_size > 1) {
		DPRINT("Too much data.\n");
		return -1;
	}

	if (priv->buf_size < 1)
		/* wait for more data */
		return 0;

	security_type = priv->buf[0];
	switch (security_type) {
	case 1:
		if (priv->passwd)
			break;
		priv->buf_size = 0;
		priv->client_action = vnc_client_init;

		/* block signals? */

		/* ok */
		write(priv->cfd, &ok, sizeof(ok));
		return 0;

	case 2:
		if (!priv->passwd)
			break;
		priv->buf_size = 0;
		priv->client_action = vnc_client_challenge;

		/* Mix in some bits that will change with time */
		ggCurTime(&now);
		for (i = 0; i < sizeof(now.tv_sec); ++i)
			priv->challenge[i & 7] ^= *(((uint8_t *)&now.tv_sec) + i);
		for (i = 0; i < sizeof(now.tv_usec); ++i)
			priv->challenge[i & 7] ^= *(((uint8_t *)&now.tv_usec) + i);

		/* scramble using des to get the final challenge */
		usekey(priv->randomizer);
		des(&priv->challenge[0], &priv->challenge[0]);
		/* chain the two blocks to propagate the "randomness" */
		for (i = 0; i < 8; ++i)
			priv->challenge[i + 8] ^= priv->challenge[i];
		des(&priv->challenge[8], &priv->challenge[8]);

		/* block signals? */

		/* challenge client */
		write(priv->cfd, priv->challenge, sizeof(priv->challenge));
		return 0;
	}

	DPRINT("Invalid security type requested (%u)\n", security_type);
	return -1;
}

static int
vnc_client_version(struct ggi_visual *vis)
{
	ggi_vnc_priv *priv = VNC_PRIV(vis);
	unsigned int major, minor;
	char str[13];
	unsigned char security[] = { 1, 0 };

	if (priv->buf_size > 12) {
		DPRINT("Too much data.\n");
		return -1;
	}

	if (priv->buf_size < 12)
		/* wait for more data */
		return 0;

	priv->buf[12] = '\0';
	major = atoi((char *)priv->buf + 4);
	minor = atoi((char *)priv->buf + 8);
	if (major > 999 || minor > 999) {
		DPRINT("Invalid protocol version requested\n");
		return -1;
	}
	sprintf(str, "RFB %03u.%03u\n", major, minor);
	if (strcmp(str, (char *)priv->buf)) {
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

	priv->protover = minor;

	if (priv->protover <= 3) {
		uint32_t security_v3;

		priv->buf_size = 0;
		priv->client_action = vnc_client_init;

		/* block signals? */

		/* ok, decide security */
		security_v3 = htonl(priv->passwd ? 2 : 1);
		write(priv->cfd, &security_v3, sizeof(security_v3));
		if (priv->passwd) {
			/* fake a client request of vnc auth security type */
			priv->buf[0] = 2;
			priv->buf_size = 1;
			return vnc_client_security(vis);
		}
		return 0;
	}

	priv->buf_size = 0;
	priv->client_action = vnc_client_security;

	security[1] = priv->passwd ? 2 : 1;

	/* block signals? */

	/* supported security types */
	write(priv->cfd, security, sizeof(security));

	return 0;
}

static void
vnc_close_client(struct ggi_visual *vis, int cfd)
{
	ggi_vnc_priv *priv = VNC_PRIV(vis);

	if (priv->cfd == -1)
		return;
	if (cfd != priv->cfd)
		return;

	DPRINT("close_client.\n");
	priv->del_cfd(priv->gii_ctx, cfd);
	close(priv->cfd);
	priv->cfd = -1;
	priv->buf_size = 0;
	priv->dirty.tl.x = priv->dirty.br.x = 0;
	priv->update.tl.x = priv->update.br.x = 0;
}

void
GGI_vnc_new_client_finish(struct ggi_visual *vis)
{
	ggi_vnc_priv *priv = VNC_PRIV(vis);
	long flags;

	DPRINT("new_client(%d)\n", priv->cfd);
	priv->add_cfd(priv->gii_ctx, priv->cfd);

	flags = fcntl(priv->cfd, F_GETFL);
	fcntl(priv->cfd, F_SETFL, flags | O_NONBLOCK);

	priv->client_action = vnc_client_version;

	/* block signals? */

	/* Support max protocol version 3.8 */
	write(priv->cfd, "RFB 003.008\n", 12);
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

	vnc_close_client(vis, priv->cfd);
	priv->cfd = cfd;

	GGI_vnc_new_client_finish(vis);
}

void
GGI_vnc_client_data(void *arg, int cfd)
{
	struct ggi_visual *vis = arg;
	ggi_vnc_priv *priv = VNC_PRIV(vis);
	unsigned char buf[100];
	ssize_t len;

	if (cfd != priv->cfd)
		return;

	len = read(cfd, buf, sizeof(buf));

	if (len < 0) {
		DPRINT("Error reading\n");
		vnc_close_client(vis, cfd);
		return;
	}

	if (len == 0) {
		vnc_close_client(vis, cfd);
		return;
	}

	if (len + priv->buf_size > sizeof(priv->buf)) {
		DPRINT("Avoiding buffer overrun\n");
		vnc_close_client(vis, cfd);
		return;
	}

	memcpy(priv->buf + priv->buf_size, buf, len);
	priv->buf_size += len;

	if (priv->client_action(vis)) {
		vnc_close_client(vis, cfd);
		return;
	}
}

void
GGI_vnc_invalidate_xyxy(struct ggi_visual *vis,
	uint16_t tlx, uint16_t tly, uint16_t brx, uint16_t bry)
{
	ggi_vnc_priv *priv;

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
	ggi_rect_union_xyxy(&priv->dirty, tlx, tly, brx, bry);

	if (priv->cfd != -1 && !ggi_rect_isempty(&priv->update))
		pending_client_update(vis);
}
