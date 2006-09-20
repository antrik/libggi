/* $Id: copyrect.c,v 1.3 2006/09/20 09:29:36 pekberg Exp $
******************************************************************************

   display-vnc: RFB copyrect encoding for panning

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
GGI_vnc_copyrect_pan(ggi_vnc_client *client, ggi_rect *update)
{
	struct ggi_visual *vis = client->owner;
	ggi_vnc_encode *encode = client->encode;
	int rects = 0;
	ggi_rect src;
	ggi_rect dst;
	ggi_rect csrc;
	ggi_rect cdst;
	ggi_rect upd;
	uint8_t *copyrect;

	src.tl = client->origin;
	src.br.x = src.tl.x + LIBGGI_X(vis);
	src.br.y = src.tl.y + LIBGGI_Y(vis);

	dst.tl.x = vis->origin_x;
	dst.tl.y = vis->origin_y;
	dst.br.x = dst.tl.x + LIBGGI_X(vis);
	dst.br.y = dst.tl.y + LIBGGI_Y(vis);

	ggi_rect_intersect(&src, &dst);

	if (ggi_rect_isempty(&src)) {
		update->tl.x = update->tl.y = 0;
		update->br.x = LIBGGI_X(vis);
		update->br.y = LIBGGI_Y(vis);
		return rects;
	}

	if (!encode)
		encode = GGI_vnc_raw;

	csrc = src;
	ggi_rect_antishift(&csrc, &client->origin);
	cdst = src;
	ggi_rect_shift_xy(&cdst, -vis->origin_x, -vis->origin_y);

	client->origin.x = vis->origin_x;
	client->origin.y = vis->origin_y;

	DPRINT("copyrect %dx%d - %dx%d -> %dx%d\n",
		csrc.tl.x, csrc.tl.y,
		csrc.br.x, csrc.br.y,
		cdst.tl.x, cdst.tl.y);
	++rects;
	GGI_vnc_buf_reserve(&client->wbuf, client->wbuf.size + 16);
	copyrect = &client->wbuf.buf[client->wbuf.size];
	client->wbuf.size += 16;

	copyrect = insert_header(copyrect, &cdst.tl, &cdst, 1); /* copyrect */
	copyrect = insert_hilo_16(copyrect, csrc.tl.x);
	copyrect = insert_hilo_16(copyrect, csrc.tl.y);

	if (LIBGGI_X(vis) - ggi_rect_width(&csrc) >
		LIBGGI_Y(vis) - ggi_rect_height(&csrc)) {
		/* wider area first */
		ggi_rect_set_xyxy(&upd,
			0, cdst.tl.y, LIBGGI_X(vis), cdst.br.y);
		ggi_rect_subtract(&upd, &cdst);
		upd.tl.y = 0;
		upd.br.y = LIBGGI_Y(vis);

		rects += encode(client, &upd);
		ggi_rect_subtract(update, &upd);

		if (ggi_rect_height(&csrc) == LIBGGI_Y(vis))
			return rects;

		ggi_rect_set_xyxy(&upd,
			cdst.tl.x, 0, cdst.br.x, LIBGGI_Y(vis));
		ggi_rect_subtract(&upd, &cdst);

		rects += encode(client, &upd);
		ggi_rect_subtract(update, &upd);
	}
	else {
		/* higher area first */
		ggi_rect_set_xyxy(&upd,
			cdst.tl.x, 0, cdst.br.x, LIBGGI_Y(vis));
		ggi_rect_subtract(&upd, &cdst);
		upd.tl.x = 0;
		upd.br.x = LIBGGI_X(vis);

		rects += encode(client, &upd);
		ggi_rect_subtract(update, &upd);

		if (ggi_rect_width(&csrc) == LIBGGI_X(vis))
			return rects;

		ggi_rect_set_xyxy(&upd,
			0, cdst.tl.y, LIBGGI_X(vis), cdst.br.y);
		ggi_rect_subtract(&upd, &cdst);

		rects += encode(client, &upd);
		ggi_rect_subtract(update, &upd);
	}

	return rects;
}
