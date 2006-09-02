/* $Id: raw.c,v 1.4 2006/09/02 15:26:12 pekberg Exp $
******************************************************************************

   display-vnc: RFB raw encoding

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

void
GGI_vnc_raw(struct ggi_visual *vis, ggi_rect *update)
{
	ggi_vnc_priv *priv = VNC_PRIV(vis);
	ggi_vnc_client *client = priv->client;
	struct ggi_visual *cvis;
	const ggi_directbuffer *db;
	ggi_graphtype gt;
	int bpp;
	int count;
	void *buf;
	unsigned char *header;
	int pal_size;

	DPRINT("update %dx%d - %dx%d\n",
		update->tl.x, update->tl.y,
		update->br.x, update->br.y);

	ggi_rect_subtract(&client->dirty, update);
	client->update.tl.x = client->update.br.x = 0;

	DPRINT("dirty %dx%d - %dx%d\n",
		client->dirty.tl.x, client->dirty.tl.y,
		client->dirty.br.x, client->dirty.br.y);

	if (!client->client_vis)
		cvis = priv->fb;
	else {
		cvis = client->client_vis;
		_ggiCrossBlit(priv->fb,
			update->tl.x, update->tl.y,
			update->br.x - update->tl.x,
			update->br.y - update->tl.y,
			cvis,
			update->tl.x, update->tl.y);
	}

	gt = LIBGGI_GT(cvis);

	db = ggiDBGetBuffer(cvis->stem, 0);
	ggiResourceAcquire(db->resource, GGI_ACTYPE_READ);

	bpp = GT_ByPP(gt);
	count = (update->br.x - update->tl.x) *
		(update->br.y - update->tl.y);
	pal_size = client->wbuf.size;
	GGI_vnc_buf_reserve(&client->wbuf, pal_size + 16 + count * bpp);
	client->wbuf.size += 16 + count * bpp;
	header = &client->wbuf.buf[pal_size];
	buf = &header[16];
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
	header[10] = (update->br.y - update->tl.y) >> 8;
	header[11] = (update->br.y - update->tl.y) & 0xff;
	header[12] = 0;
	header[13] = 0;
	header[14] = 0;
	header[15] = 0;
	if (client->reverse_endian && GT_SIZE(gt) > 8) {
		int i, j;
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
	}
	else if (update->br.x - update->tl.x != LIBGGI_VIRTX(cvis)) {
		int i;
		uint8_t *dst = buf;

		for (i = update->tl.y; i < update->br.y; ++i, dst += (update->br.x - update->tl.x) * bpp)
			memcpy(dst,
				(uint8_t *)db->read + (update->tl.x + i * LIBGGI_VIRTX(cvis)) * bpp,
				(update->br.x - update->tl.x) * bpp);
	}
	else {
		memcpy(buf, 
			(uint8_t *)db->read + GT_ByPPP(LIBGGI_VIRTX(cvis) * update->tl.y, gt),
			count * bpp);
	}

	ggiResourceRelease(db->resource);
}
