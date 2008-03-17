/* $Id: mode.c,v 1.22 2008/03/17 12:26:36 pekberg Exp $
******************************************************************************

   display-vnc: mode management

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
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include <ggi/display/vnc.h>
#include <ggi/internal/ggi_debug.h>
#include <ggi/internal/gg_replace.h>	/* for snprintf() */

#include "common.h"

static int
vnc_acquire(ggi_resource *res, uint32_t actype)
{
	struct ggi_visual *vis = res->priv;
	ggi_directbuffer *buf = res->self;

	DPRINT_MISC("acquire(%p, 0x%x) called\n", res, actype);

	if (actype & ~(GGI_ACTYPE_READ | GGI_ACTYPE_WRITE))
		return GGI_EARGINVAL;

	res->curactype |= actype;
	res->count++;

	DPRINT_MISC("acquire - success, count: %d\n", res->count);

	if (LIBGGI_FLAGS(vis) & (GGIFLAG_ASYNC | GGIFLAG_TIDYBUF))
		return 0;

	/* sync mode and no tidy buffer, so trigger pending update.
	 * could have used mansync instead, I suppose...
	 */
	if (vis->d_frame_num == buf->frame)
		GGI_vnc_invalidate_nc_xyxy(vis,
			vis->origin_x,
			vis->origin_y,
			vis->origin_x + LIBGGI_X(vis),
			vis->origin_y + LIBGGI_Y(vis));

	return 0;
}

static int
vnc_release(ggi_resource *res)
{
	struct ggi_visual *vis = res->priv;
	ggi_directbuffer *buf = res->self;

	DPRINT_MISC("release(%p) called\n", res);

	if (res->count < 1)
		return GGI_ENOTALLOC;

	if (--res->count)
		return 0;

	if (!(res->curactype & GGI_ACTYPE_WRITE))
		goto out;

	if (LIBGGI_FLAGS(vis) & GGIFLAG_TIDYBUF)
		goto out;

	/* no tidy buffer, so assume whole visible
	 * area has been clobbered on release of
	 * the visible frame
	 */
	if (vis->d_frame_num == buf->frame)
		GGI_vnc_invalidate_nc_xyxy(vis,
			vis->origin_x,
			vis->origin_y,
			vis->origin_x + LIBGGI_X(vis),
			vis->origin_y + LIBGGI_Y(vis));

out:
	res->curactype = 0;
	return 0;
}

static int
GGI_vnc_flush(struct ggi_visual *vis, 
	int x, int y, int w, int h, int tryflag)
{
	ggi_vnc_priv *priv = VNC_PRIV(vis);
	ggi_vnc_client *client;
	ggi_directbuffer *buf = LIBGGI_APPBUFS(vis)[vis->d_frame_num];
	ggi_rect flush;

	int res = _ggiFlushRegion(priv->fb, x, y, w, h);

	if (GG_LIST_EMPTY(&priv->clients))
		return res;

	if (!(LIBGGI_FLAGS(vis) & (GGIFLAG_ASYNC | GGIFLAG_TIDYBUF)))
		return res;

	flush.tl.x = x;
	flush.tl.y = y;
	flush.br.x = x + w;
	flush.br.y = y + h;

	if (ggi_rect_isempty(&flush))
		return res;

	if (LIBGGI_FLAGS(vis) & GGIFLAG_TIDYBUF) {
		if (buf->resource->curactype & GGI_ACTYPE_WRITE) {
			/* tidy buffer and buffer locked, update it */
			GG_LIST_FOREACH(client, &priv->clients, siblings)
				GGI_vnc_invalidate_nc_xyxy(vis,
					flush.tl.x, flush.tl.y,
					flush.br.x, flush.br.y);
			return res;
		}
	}

	GG_LIST_FOREACH(client, &priv->clients, siblings) {
		ggi_rect flush_client = flush;
		ggi_rect_intersect(&flush_client, &client->fdirty);
		ggi_rect_subtract(&client->fdirty, &flush);

		GGI_vnc_client_invalidate_nc_xyxy(client,
			flush_client.tl.x, flush_client.tl.y,
			flush_client.br.x, flush_client.br.y);
	}

	return res;
}

int
GGI_vnc_getapi(struct ggi_visual *vis, int num, char *apiname, char *arguments)
{
	*arguments = '\0';

	if (num == 0) { 
		strcpy(apiname, "display-vnc");
		return 0;
	}

	return GGI_ENOMATCH;
}

static void
_ggi_freedbs(struct ggi_visual *vis) 
{
	int i;
	ggi_directbuffer *buf;

	for (i = LIBGGI_APPLIST(vis)->num - 1; i >= 0; i--) {
		buf = LIBGGI_APPBUFS(vis)[i];
		free(buf->resource);
		_ggi_db_free(buf);
		_ggi_db_del_buffer(LIBGGI_APPLIST(vis), i);
	}
}

static int
_ggi_domode(struct ggi_visual *vis)
{
	ggi_vnc_priv *priv = VNC_PRIV(vis);
	int err, i;
	char name[GGI_MAX_APILEN];
	char args[GGI_MAX_APILEN];

	_ggiZapMode(vis, 0);
	_ggi_freedbs(vis);

	DPRINT("_ggi_domode: zapped\n");

	ggiSetMode(priv->fb->instance.stem, LIBGGI_MODE(vis));

	/* set up pixel format */
	memcpy(LIBGGI_PIXFMT(vis), LIBGGI_PIXFMT(priv->fb), sizeof(ggi_pixelformat));

	vis->d_frame_num = priv->fb->d_frame_num;
	vis->r_frame_num = priv->fb->r_frame_num;
	vis->w_frame_num = priv->fb->w_frame_num;

	/* Set Up Direct Buffers */

	for (i = 0; i < LIBGGI_MODE(vis)->frames; i++) {
		ggi_directbuffer *buf;
		ggi_directbuffer *membuf;
		ggi_resource *res;

		res = malloc(sizeof(ggi_resource));
		if (!res)
			return GGI_EFATAL;

		LIBGGI_APPLIST(vis)->last_targetbuf =
			_ggi_db_add_buffer(LIBGGI_APPLIST(vis),
					   _ggi_db_get_new());
		buf = LIBGGI_APPBUFS(vis)[i];
		membuf = LIBGGI_APPBUFS(priv->fb)[i];

		res->acquire = vnc_acquire;
		res->release = vnc_release;
		res->self = buf;
		res->priv = vis;
		res->count = 0;
		res->curactype = 0;

		buf->resource = res;
		buf->frame = i;
		buf->type = GGI_DB_NORMAL | GGI_DB_SIMPLE_PLB;
		buf->read = buf->write = membuf->read;
		buf->layout = blPixelLinearBuffer;
		buf->buffer.plb.stride = membuf->buffer.plb.stride;
		buf->buffer.plb.pixelformat = LIBGGI_PIXFMT(vis);
	}

	vis->r_frame = LIBGGI_APPBUFS(vis)[0];
	vis->w_frame = LIBGGI_APPBUFS(vis)[0];

	LIBGGI_APPLIST(vis)->first_targetbuf
	    = LIBGGI_APPLIST(vis)->last_targetbuf - (LIBGGI_MODE(vis)->frames - 1);

	/* Set up palette */
	if (LIBGGI_PAL(vis)->clut.data) {
		free(LIBGGI_PAL(vis)->clut.data);
		LIBGGI_PAL(vis)->clut.data = NULL;
	}

	if (GT_SCHEME(LIBGGI_GT(vis)) == GT_PALETTE) {
		int num_cols = 1 << GT_DEPTH(LIBGGI_GT(vis));
		LIBGGI_PAL(vis)->clut.data =
			_ggi_malloc(sizeof(ggi_color) * num_cols);
		LIBGGI_PAL(vis)->clut.size = num_cols;
	}

	for(i=1; GGI_vnc_getapi(vis, i, name, args) == 0; i++) {
		err = _ggiOpenDL(vis, libggi->config, name, args, NULL);
		if (err) {
			DPRINT_LIBS("Can't open the %s (%s) library.\n",
				name, args);
			return GGI_EFATAL;
		} else {
			DPRINT_LIBS("Success in loading "
				       "%s (%s)\n", name, args);
		}
	}

	vis->opgc->gcchanged		= GGI_vnc_gcchanged;
	vis->opdisplay->flush		= GGI_vnc_flush;

	vis->opdraw->setdisplayframe	= GGI_vnc_setdisplayframe;
	vis->opdraw->setreadframe	= GGI_vnc_setreadframe;
	vis->opdraw->setwriteframe	= GGI_vnc_setwriteframe;
	vis->opdraw->setorigin		= GGI_vnc_setorigin;

	vis->opdraw->drawpixel		= GGI_vnc_drawpixel;
	vis->opdraw->drawpixel_nc	= GGI_vnc_drawpixel_nc;
	vis->opdraw->putpixel		= GGI_vnc_putpixel;
	vis->opdraw->putpixel_nc	= GGI_vnc_putpixel_nc;
	vis->opdraw->getpixel		= GGI_vnc_getpixel;
	vis->opdraw->getpixel_nc	= GGI_vnc_getpixel_nc;

	vis->opdraw->drawline		= GGI_vnc_drawline;
	vis->opdraw->drawhline		= GGI_vnc_drawhline;
	vis->opdraw->drawhline_nc	= GGI_vnc_drawhline_nc;
	vis->opdraw->puthline		= GGI_vnc_puthline;
	vis->opdraw->gethline		= GGI_vnc_gethline;
	vis->opdraw->drawvline		= GGI_vnc_drawvline;
	vis->opdraw->drawvline_nc	= GGI_vnc_drawvline_nc;
	vis->opdraw->putvline		= GGI_vnc_putvline;
	vis->opdraw->getvline		= GGI_vnc_getvline;

	vis->opdraw->drawbox		= GGI_vnc_drawbox;
	vis->opdraw->putbox		= GGI_vnc_putbox;
	vis->opdraw->getbox		= GGI_vnc_getbox;
	vis->opdraw->copybox		= GGI_vnc_copybox;
	vis->opdraw->crossblit		= GGI_vnc_crossblit;
	vis->opdraw->fillscreen		= GGI_vnc_fillscreen;

	vis->opdraw->putc		= GGI_vnc_putc;
	vis->opdraw->puts		= GGI_vnc_puts;
	vis->opdraw->getcharsize	= GGI_vnc_getcharsize;

	vis->opcolor->setpalvec		= GGI_vnc_setpalvec;
	vis->opcolor->getpalvec		= GGI_vnc_getpalvec;
	vis->opcolor->mapcolor		= GGI_vnc_mapcolor;
	vis->opcolor->unmappixel	= GGI_vnc_unmappixel;
	vis->opcolor->packcolors	= GGI_vnc_packcolors;
	vis->opcolor->unpackpixels	= GGI_vnc_unpackpixels;

	vis->opcolor->setgamma		= GGI_vnc_setgamma;
	vis->opcolor->getgamma		= GGI_vnc_getgamma;
	vis->opcolor->setgammamap	= GGI_vnc_setgammamap;
	vis->opcolor->getgammamap	= GGI_vnc_getgammamap;

	return 0;
}

int
GGI_vnc_setmode(struct ggi_visual *vis, ggi_mode *mode)
{ 
	ggi_vnc_priv *priv = VNC_PRIV(vis);
	ggi_vnc_client *client;
	ggi_vnc_client *next;
	int err;
	int desktop_size = 0;

	if (vis==NULL || mode==NULL || LIBGGI_MODE(vis)==NULL) {
		return GGI_EARGINVAL;
	}
	
	DPRINT_MODE("setmode %dx%d#%dx%dF%d[0x%02x]\n",
			mode->visible.x, mode->visible.y,
			mode->virt.x, mode->virt.y, 
			mode->frames, mode->graphtype);

	if ((err = ggiCheckMode(vis->instance.stem, mode)) != 0) {
		return err;
	}

	if (mode->visible.x != LIBGGI_X(vis) ||
		mode->visible.y != LIBGGI_Y(vis))
	{
		desktop_size = 1;
	}

	*LIBGGI_MODE(vis) = *mode;

	err = _ggi_domode(vis);

	if (err) {
		DPRINT("domode failed (%d)\n",err);
		return err;
	}

	client = GG_LIST_FIRST(&priv->clients);
	for (; client; client = next) {
		next = GG_LIST_NEXT(client, siblings);

		if (desktop_size) {
			/* "Incompatible" mode change, send desktop size. */

			if (!(client->desktop_size & DESKSIZE_OK_INIT)) {
				/* ouch, no support */
				GGI_vnc_close_client(client);
				continue;
			}

			if (client->desktop_size == DESKSIZE_OK_INIT)
				/* still ok to resize freely */
				continue;

			/* inform client of size change, later */
			/* but update any client visual right away */
			client->desktop_size |= DESKSIZE_SEND;
		}
		GGI_vnc_change_pixfmt(client);
	}

	if (desktop_size) {
		ggi_rect virt;

		virt.tl.x = virt.tl.y = 0;
		virt.br = mode->virt;

		if (LIBGGI_FLAGS(vis) & GGIFLAG_ASYNC) {
			GG_LIST_FOREACH(client, &priv->clients, siblings) {
				client->fdirty = virt;
				ggi_rect_intersect(&client->dirty, &virt);
				ggi_rect_intersect(&client->update, &virt);
			}
		}
		else {
			GG_LIST_FOREACH(client, &priv->clients, siblings) {
				client->dirty.tl.x = 0;
				client->dirty.tl.y = 0;
				client->dirty.br.x = 0;
				client->dirty.br.y = 0;
				ggi_rect_intersect(&client->fdirty, &virt);
				ggi_rect_intersect(&client->update, &virt);
				GGI_vnc_client_invalidate_nc_xyxy(client,
					0, 0, mode->virt.x, mode->virt.y);
			}
		}
	}

	ggiIndicateChange(vis->instance.stem, GGI_CHG_APILIST);
	DPRINT("change indicated\n",err);

	return 0;
}

int
GGI_vnc_checkmode(struct ggi_visual *vis, ggi_mode *mode)
{
	ggi_vnc_priv *priv = VNC_PRIV(vis);
	int err;

	DPRINT_MODE("checkmode %dx%d#%dx%dF%d[0x%02x]\n",
			mode->visible.x, mode->visible.y,
			mode->virt.x, mode->virt.y, 
			mode->frames, mode->graphtype);

	err = ggiCheckMode(priv->fb->instance.stem, mode);

	DPRINT_MODE("result %d %dx%d#%dx%dF%d[0x%02x]\n",
			err, mode->visible.x, mode->visible.y,
			mode->virt.x, mode->virt.y, 
			mode->frames, mode->graphtype);

	return err;	
}

int
GGI_vnc_getmode(struct ggi_visual *vis, ggi_mode *mode)
{
	DPRINT("getmode(%p,%p)\n", vis, mode);

	if (vis==NULL || mode==NULL || LIBGGI_MODE(vis)==NULL) {
		return GGI_EARGINVAL;
	}

	memcpy(mode, LIBGGI_MODE(vis), sizeof(ggi_mode));

	return 0;
}

int
GGI_vnc_setflags(struct ggi_visual *vis, uint32_t flags)
{
	ggi_vnc_priv *priv = VNC_PRIV(vis);
	ggi_vnc_client *client;
	int invalidate;
	int res = _ggiSetFlags(priv->fb, flags);

	/* Unknown flags don't take. */
	flags &= GGIFLAG_ASYNC | GGIFLAG_TIDYBUF;

	/* invalidate if going from async to sync */
	invalidate =
		!(flags & GGIFLAG_ASYNC) &&
		(LIBGGI_FLAGS(vis) & GGIFLAG_ASYNC);

	LIBGGI_FLAGS(vis) = flags;

	if (!invalidate)
		return res;

	GG_LIST_FOREACH(client, &priv->clients, siblings)
		GGI_vnc_client_invalidate_nc_xyxy(client,
			client->fdirty.tl.x, client->fdirty.tl.y,
			client->fdirty.br.x, client->fdirty.br.y);

	return res;
}
