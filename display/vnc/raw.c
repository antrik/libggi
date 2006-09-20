/* $Id: raw.c,v 1.11 2006/09/20 09:27:35 pekberg Exp $
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
#include "common.h"

int
GGI_vnc_raw(ggi_vnc_client *client, ggi_rect *update)
{
	struct ggi_visual *vis = client->owner;
	ggi_vnc_priv *priv = VNC_PRIV(vis);
	struct ggi_visual *cvis;
	const ggi_directbuffer *db;
	ggi_graphtype gt;
	int bpp;
	int count;
	void *buf;
	unsigned char *header;
	int pal_size;
	ggi_rect vupdate;
	int d_frame_num;

	DPRINT("raw update %dx%d - %dx%d\n",
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

	gt = LIBGGI_GT(cvis);

	db = ggiDBGetBuffer(cvis->stem, d_frame_num);
	ggiResourceAcquire(db->resource, GGI_ACTYPE_READ);

	bpp = GT_ByPP(gt);
	count = ggi_rect_width(&vupdate) * ggi_rect_height(&vupdate);
	pal_size = client->wbuf.size;
	GGI_vnc_buf_reserve(&client->wbuf, pal_size + 12 + count * bpp);
	client->wbuf.size += 12 + count * bpp;
	header = &client->wbuf.buf[pal_size];
	buf = &header[12];
	header[ 0] = update->tl.x >> 8;
	header[ 1] = update->tl.x & 0xff;
	header[ 2] = update->tl.y >> 8;
	header[ 3] = update->tl.y & 0xff;
	header[ 4] = ggi_rect_width(&vupdate) >> 8;
	header[ 5] = ggi_rect_width(&vupdate) & 0xff;
	header[ 6] = ggi_rect_height(&vupdate) >> 8;
	header[ 7] = ggi_rect_height(&vupdate) & 0xff;
	header[ 8] = 0;
	header[ 9] = 0;
	header[10] = 0;
	header[11] = 0; /* raw */
	if (client->reverse_endian && GT_SIZE(gt) > 8) {
		int i, j;
		int stride = db->buffer.plb.stride / bpp;

		if (bpp == 2) {
			uint16_t *src = (uint16_t *)db->read +
				vupdate.tl.x + vupdate.tl.y * stride;
			for (j = 0; j < ggi_rect_height(&vupdate); ++j) {
				for (i = 0; i < ggi_rect_width(&vupdate); ++i)
					buf = insert_rev_16(buf, src[i]);
				src += stride;
			}
		}
		else { /* bpp == 4 */
			uint32_t *src = (uint32_t *)db->read +
				vupdate.tl.x + vupdate.tl.y * stride;
			for (j = 0; j < ggi_rect_height(&vupdate); ++j) {
				for (i = 0; i < ggi_rect_width(&vupdate); ++i)
					buf = insert_rev_32(buf, src[i]);
				src += stride;
			}
		}
	}
	else if (vupdate.br.x - vupdate.tl.x != LIBGGI_VIRTX(cvis)) {
		int i;
		uint8_t *dst = buf;

		for (i = vupdate.tl.y; i < vupdate.br.y; ++i, dst += ggi_rect_width(&vupdate) * bpp)
			memcpy(dst,
				(uint8_t *)db->read + (vupdate.tl.x + i * LIBGGI_VIRTX(cvis)) * bpp,
				ggi_rect_width(&vupdate) * bpp);
	}
	else {
		memcpy(buf, 
			(uint8_t *)db->read + GT_ByPPP(LIBGGI_VIRTX(cvis) * vupdate.tl.y, gt),
			count * bpp);
	}

	ggiResourceRelease(db->resource);
	return 1;
}
