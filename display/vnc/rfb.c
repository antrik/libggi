/* $Id: rfb.c,v 1.11 2006/08/26 05:20:19 pekberg Exp $
******************************************************************************

   Display-vnc: RFB protocol

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
	DPRINT_MISC("  size/depth %d/%d\n", priv->buf[4], priv->buf[5]);
#ifdef GGI_BIG_ENDIAN
	priv->reverse_endian = !priv->buf[6];
#else
	priv->reverse_endian = priv->buf[6];
#endif
	DPRINT_MISC("  endian: %s\n", priv->reverse_endian ? "reverse" : "match");
	DPRINT_MISC("  type: %s\n", priv->buf[7] ? "truecolor" : "palette");
	DPRINT_MISC("  red max (shift):   %u (%d)\n", red_max,   priv->buf[14]);
	DPRINT_MISC("  green max (shift): %u (%d)\n", green_max, priv->buf[15]);
	DPRINT_MISC("  blue max (shift):  %u (%d)\n", blue_max,  priv->buf[16]);

	memset(&pixfmt, 0, sizeof(ggi_pixelformat));
	pixfmt.size = priv->buf[4];
	if (red_max == 3 && green_max == 3 && blue_max == 3)
		pixfmt.depth = 6;
	else if (red_max == 1 && green_max == 1 && blue_max == 1)
		pixfmt.depth = 3;
	else
		pixfmt.depth = priv->buf[5];
	if (priv->buf[7]) {
		pixfmt.red_mask = red_max << priv->buf[14];
		pixfmt.green_mask = green_max << priv->buf[15];
		pixfmt.blue_mask = blue_max << priv->buf[16];
	}
	else
		pixfmt.clut_mask = (1 << pixfmt.depth) - 1;

	/*
	if (priv->reverse_endian)
		pixfmt.flags |= GGI_PF_REVERSE_ENDIAN;
	*/

	_ggi_build_pixfmt(&pixfmt);

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
	/*
	if (priv->reverse_endian)
		GT_SETSUBSCHEME(mode.graphtype, GT_SUB_REVERSE_ENDIAN);
	*/
	mode.dpp.x = mode.dpp.y = 1;

	i = 0;
	if (priv->buf[7]) {
		i += snprintf(target, GGI_MAX_APILEN, "display-memory:-pixfmt=");
	
		/* Need a visual to call _ggi_build_pixfmtstr, so fake one... */
		fake_vis.pixfmt = &pixfmt;
		fake_vis.mode = &mode;
		memset(target + i, '\0', sizeof(target) - i);
		_ggi_build_pixfmtstr(&fake_vis, target + i, sizeof(target) - i, GGI_PIXFMT_CHANNEL);
		i = strlen(target);
	}
	else
		i += snprintf(target, GGI_MAX_APILEN, "display-memory");

	stem = ggNewStem();
	if (stem == NULL) {
		DPRINT("ggNewStem failed\n");
		return -1;
	}
	if (ggiAttach(stem) != GGI_OK) {
		ggDelStem(stem);
		DPRINT("ggiAttach failed\n");
		return -1;
	}
	if (ggiOpen(stem, target, NULL) != GGI_OK) {
		ggDelStem(stem);
		DPRINT("ggiOpen failed\n");
		return -1;
	}
	priv->client_vis = STEM_API_DATA(stem, libggi, struct ggi_visual *);
	if (ggiSetMode(priv->client_vis->stem, &mode)) {
		ggiClose(priv->client_vis->stem);
		ggDelStem(priv->client_vis->stem);
		priv->client_vis = NULL;
		DPRINT("ggiSetMode failed\n");
		return -1;
	}

	/* fix endian mismatch behind the back of display-memory */
	/* But this doesn't trigger a slow crossblit anyway, and
	   even if it did, default-color does not support reverse
	   endian modes anyway. So comment it out...
	if (priv->reverse_endian) {
		GT_SETSUBSCHEME(LIBGGI_GT(priv->client_vis), GT_SUB_REVERSE_ENDIAN);
		LIBGGI_PIXFMT(priv->client_vis)->flags |= GGI_PF_REVERSE_ENDIAN;
	}
	*/

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

static int
vnc_client_update(struct ggi_visual *vis)
{
	ggi_vnc_priv *priv = VNC_PRIV(vis);
	struct ggi_visual *cvis;
	const ggi_directbuffer *db;
	unsigned char header[16];

	DPRINT("client_update\n");

	if (priv->buf_size < 10) {
		/* wait for more data */
		priv->client_action = vnc_client_update;
		return 0;
	}

	if (!priv->client_vis)
		cvis = vis;
	else {
		cvis = priv->client_vis;
		ggiCrossBlit(vis->stem, 0, 0, LIBGGI_VIRTX(cvis), LIBGGI_VIRTY(cvis), cvis->stem, 0, 0);
	}

	if (priv->palette_dirty) {
		unsigned char *vnc_palette;
		int colors = 1 << GT_DEPTH(LIBGGI_GT(cvis));
		ggi_color ggi_palette[256];
		int i;

		ggiGetPalette(priv->client_vis->stem, 0, colors, ggi_palette);

		vnc_palette = malloc(6 + 6 * colors);
		vnc_palette[0] = 1;
		vnc_palette[1] = 0;
		vnc_palette[2] = 0;
		vnc_palette[3] = 0;
		vnc_palette[4] = colors >> 8;
		vnc_palette[5] = colors & 0xff;

		for (i = 0; i < colors; ++i) {
			vnc_palette[6 + 6 * i + 0] = ggi_palette[i].r >> 8;
			vnc_palette[6 + 6 * i + 1] = ggi_palette[i].r & 0xff;
			vnc_palette[6 + 6 * i + 2] = ggi_palette[i].g >> 8;
			vnc_palette[6 + 6 * i + 3] = ggi_palette[i].g & 0xff;
			vnc_palette[6 + 6 * i + 4] = ggi_palette[i].b >> 8;
			vnc_palette[6 + 6 * i + 5] = ggi_palette[i].b & 0xff;
		}

		write(priv->cfd, vnc_palette, 6 + 6 * colors);
		priv->palette_dirty = 0;
	}

	db = ggiDBGetBuffer(cvis->stem, 0);
	ggiResourceAcquire(db->resource, GGI_ACTYPE_READ);

	header[ 0] = 0;
	header[ 1] = 0;
	header[ 2] = 0;
	header[ 3] = 1;
	header[ 4] = 0;
	header[ 5] = 0;
	header[ 6] = 0;
	header[ 7] = 0;
	header[ 8] = LIBGGI_VIRTX(cvis) >> 8;
	header[ 9] = LIBGGI_VIRTX(cvis) & 0xff;
	header[10] = LIBGGI_VIRTY(cvis) >> 8;;
	header[11] = LIBGGI_VIRTY(cvis) & 0xff;
	header[12] = 0;
	header[13] = 0;
	header[14] = 0;
	header[15] = 0;
	write(priv->cfd, &header, sizeof(header));
	if (priv->reverse_endian && GT_SIZE(LIBGGI_GT(cvis)) > 8) {
		int i;
		int bpp = GT_SIZE(LIBGGI_GT(cvis)) / 8;
		int count = GT_ByPPP(LIBGGI_VIRTX(cvis) * LIBGGI_VIRTY(cvis), LIBGGI_GT(cvis)) / bpp;
		void *buf = malloc(count * bpp);
		if (bpp == 2) {
			uint16_t *tmp = (uint16_t *)buf;
			for (i = 0; i < count; ++i)
				tmp[i] = GGI_BYTEREV16(*((uint16_t *)db->read + i));
		}
		else if (bpp == 3) {
			uint8_t *tmp = (uint8_t *)buf;
			for (i = 0; i < count; ++i) {
				tmp[i * 3    ] = *((uint16_t *)db->read + i * 3 + 2);
				tmp[i * 3 + 1] = *((uint16_t *)db->read + i * 3 + 1);
				tmp[i * 3 + 2] = *((uint16_t *)db->read + i * 3    );
			}
		}
		else if (bpp == 4) {
			uint32_t *tmp = (uint32_t *)buf;
			for (i = 0; i < count; ++i)
				tmp[i] = GGI_BYTEREV32(*((uint32_t *)db->read + i));
		}
		else {
			free(buf);
			buf = db->read;
		}
		write(priv->cfd, buf, count * bpp);
		if (buf != db->read)
			free(buf);
	}
	else
		write(priv->cfd, db->read, GT_ByPPP(LIBGGI_VIRTX(cvis) * LIBGGI_VIRTY(cvis), LIBGGI_GT(cvis)));
	ggiResourceRelease(db->resource);

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
	server_init[4] = GT_SIZE(LIBGGI_GT(vis));
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
	long flags;

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
