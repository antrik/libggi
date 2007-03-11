/* $Id: mode.c,v 1.45 2007/03/11 00:48:57 soyt Exp $
*****************************************************************************

   LibGGI DirectX target - Mode management

   Copyright (C) 1999-2000 John Fortin	[fortinj@ibm.net]
   Copyright (C) 2004      Peter Ekberg	[peda@lysator.liu.se]

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

#include <string.h>

#include "config.h"
#include <ggi/internal/gg_replace.h>
#include <ggi/internal/ggi-dl.h>
#include <ggi/internal/ggi_debug.h>
#include <ggi/display/directx.h>

#include "ddinit.h"

#include "../common/pixfmt-setup.inc"

static int
directx_acquire(ggi_resource *res, uint32_t actype)
{
	ggi_directbuffer *dbuf;
	struct ggi_visual *vis;
	directx_priv *priv;
	int bufnum;

	DPRINT_MISC("directx_acquire(%p, 0x%x) called\n", res, actype);

	if (actype & ~(GGI_ACTYPE_READ | GGI_ACTYPE_WRITE)) {
		return GGI_EARGINVAL;
	}
	vis = res->priv;
	priv = GGIDIRECTX_PRIV(vis);
	dbuf = res->self;
	bufnum = dbuf->frame;

	if (res->count > 0) {
		if (dbuf->write != NULL) {
			dbuf->read = dbuf->write;
		} else if (dbuf->read != NULL) {
			dbuf->write = dbuf->read;
		}
		res->count++;
		return 0;
	}
	GGI_directx_Lock(priv->cs);
	dbuf->write = priv->lpSurfaceAdd[dbuf->frame];
	dbuf->read = dbuf->write;
	GGI_directx_Unlock(priv->cs);

	res->curactype |= actype;
	res->count++;

	DPRINT_MISC("directx_acquire - success, count: %d\n",
		       res->count);

	return 0;
}

static int
directx_release(ggi_resource *res)
{
	DPRINT_MISC("directx_release(%p) called\n", res);

	if (res->count < 1)
		return GGI_ENOTALLOC;

	res->count--;

	return 0;
}

int
GGI_directx_flush(struct ggi_visual *vis, int x, int y, int w, int h, int tryflag)
{
	directx_priv *priv = GGIDIRECTX_PRIV(vis);
	GGI_directx_Lock(priv->cs);
	GGI_directx_DDRedraw(vis, x, y, w, h);
	GGI_directx_Unlock(priv->cs);
	return 0;
}

int
GGI_directx_getapi(struct ggi_visual * vis, int num, char *apiname, char *arguments)
{
	*arguments = '\0';
	switch (num) {
	case 0:
		strcpy(apiname, "display-directx");
		return 0;
	case 1:
		strcpy(apiname, "generic-stubs");
		return 0;
	case 2:
		strcpy(apiname, "generic-color");
		return 0;
	case 3:
		sprintf(apiname, "generic-linear-%d", GT_SIZE(LIBGGI_GT(vis)));
		return 0;
	}

	return GGI_ENOMATCH;
}


static void
GetScreenParams(int *depth, int *wpix, int *hpix, int *wmm, int *hmm)
{
	HWND wnd = GetDesktopWindow();
	HDC dc = GetDC(wnd);
	*depth = GetDeviceCaps(dc, BITSPIXEL);
	*wpix = GetDeviceCaps(dc, HORZRES);
	*hpix = GetDeviceCaps(dc, VERTRES);
	*wmm = GetDeviceCaps(dc, HORZSIZE);
	*hmm = GetDeviceCaps(dc, VERTSIZE);
	ReleaseDC(wnd, dc);
}


static ggi_graphtype
depth_to_graphtype(int depth)
{
	switch (depth) {
	case  1:	return GT_1BIT;
	case  2:	return GT_2BIT;
	case  4:	return GT_4BIT;
	case  8:	return GT_8BIT;
	case 15:	return GT_15BIT;
	case 16:	return GT_16BIT;
	case 24:	return GT_24BIT;
	case 32:	return GT_32BIT;
	default:	return GT_INVALID;
	}
}

static int
do_checkmode(struct ggi_visual *vis, ggi_mode *mode)
{
	directx_priv *priv = GGIDIRECTX_PRIV(vis);
	int err = GGI_OK;
	int depth, width, height, sizex, sizey, defwidth, defheight;
	ggi_graphtype deftype;

	GetScreenParams(&depth, &width, &height, &sizex, &sizey);
	if (priv->hParent) {
		RECT r;
		GetWindowRect(priv->hParent, &r);
		defwidth = r.right - r.left;
		defheight = r.bottom - r.top;
	} else {
		defwidth = width * 9 / 10;
		defheight = height * 9 / 10;
	}

	if (mode->frames == GGI_AUTO)
		mode->frames = 1;

	if (mode->dpp.x == GGI_AUTO)
		mode->dpp.x = 1;

	if (mode->dpp.y == GGI_AUTO)
		mode->dpp.y = 1;

	if (priv->fullscreen) {
		if (!GGI_directx_DDMatchMode(vis, mode, &depth,
					     &defwidth, &defheight))
			err = GGI_ENOMATCH;
		deftype = depth_to_graphtype(depth);
		if (mode->graphtype != GT_AUTO && mode->graphtype != deftype)
			err = GGI_ENOMATCH;
		mode->graphtype = deftype;

		if (mode->visible.x != GGI_AUTO && mode->visible.x != defwidth)
			err = GGI_ENOMATCH;
		mode->visible.x = defwidth;

		if (mode->visible.y != GGI_AUTO && mode->visible.y != defheight)
			err = GGI_ENOMATCH;
		mode->visible.y = defheight;

		width = defwidth;
		height = defheight;
	}
	else {
		deftype = depth_to_graphtype(depth);
		if (deftype == GT_INVALID) {
			deftype = GT_AUTO;
			err = GGI_ENOMATCH;
		}

		if (mode->graphtype == GT_AUTO)
			mode->graphtype = deftype;

		if (GT_SIZE(mode->graphtype) != (unsigned) depth) {
			mode->graphtype = deftype;
			err = GGI_ENOMATCH;
		}
	}

	if (mode->visible.x==GGI_AUTO
		&& mode->virt.x==GGI_AUTO
		&& mode->size.x==GGI_AUTO)
		mode->visible.x = mode->virt.x = defwidth;
	else if (mode->visible.x==GGI_AUTO && mode->virt.x==GGI_AUTO)
		mode->visible.x = mode->virt.x = mode->size.x * width / sizex;
	else if (mode->visible.x == GGI_AUTO)
		mode->visible.x = mode->virt.x;
	else if (mode->virt.x == GGI_AUTO)
		mode->virt.x = mode->visible.x;

	if (mode->visible.y==GGI_AUTO
		&& mode->virt.y==GGI_AUTO
		&& mode->size.y==GGI_AUTO)
		mode->visible.y = mode->virt.y = defheight;
	else if (mode->visible.y==GGI_AUTO && mode->virt.y==GGI_AUTO)
		mode->visible.y = mode->virt.y = mode->size.y * height / sizey;
	else if (mode->visible.y == GGI_AUTO)
		mode->visible.y = mode->virt.y;
	else if (mode->virt.y == GGI_AUTO)
		mode->virt.y = mode->visible.y;

	DPRINT_MODE("visible (%i,%i) virt (%i,%i) size (%i,%i)\n",
		mode->visible.x, mode->visible.y,
		mode->virt.x, mode->virt.y,
		mode->size.x, mode->size.y);

	if (mode->frames < 1) {
		err = GGI_ENOMATCH;
		mode->frames = 1;
	} else if (mode->frames > GGI_DISPLAY_DIRECTX_FRAMES) {
		err = GGI_ENOMATCH;
		mode->frames = GGI_DISPLAY_DIRECTX_FRAMES;
	}

	if (!(mode->visible.x > 0
	      && mode->visible.y > 0
	      && mode->visible.x <= width
	      && mode->visible.y <= height
	      && (!priv->hParent
		  || (mode->visible.x == defwidth
		      && mode->visible.y == defheight)))) {
		mode->visible.x = defwidth;
		mode->visible.y = defheight;
		mode->size.x = GGI_AUTO;
		mode->size.y = GGI_AUTO;
		err = GGI_ENOMATCH;
	}

	if (mode->virt.x < mode->visible.x) {
		mode->virt.x = mode->visible.x;
		err = GGI_ENOMATCH;
	}
	if (mode->virt.y < mode->visible.y) {
		mode->virt.y = mode->visible.y;
		err = GGI_ENOMATCH;
	}

	if (mode->dpp.x != 1 || mode->dpp.y != 1)
		err = GGI_ENOMATCH;
	mode->dpp.x = mode->dpp.y = 1;

	if (err)
		return err;

	GGI_directx_Lock(priv->cs);
	err = _ggi_physz_figure_size(mode, priv->physzflags,
				     &priv->physz,
				     width * 254 / sizex / 10,
				     height * 254 / sizey / 10,
				     width, height);
	GGI_directx_Unlock(priv->cs);

	DPRINT_MODE
	    ("checkmode returns %dx%d#%dx%dF%d[0x%02x]\n",
	     mode->visible.x, mode->visible.y, mode->virt.x, mode->virt.y,
	     mode->frames, mode->graphtype);

	return err;
}


int
GGI_directx_checkmode(struct ggi_visual *vis, ggi_mode *mode)
{
	return do_checkmode(vis, mode);
}

static int
compatible_mode(struct ggi_visual *vis, ggi_mode *mode)
{
	directx_priv *priv = GGIDIRECTX_PRIV(vis);

	if (priv->fullscreen)
		return !memcmp(mode, LIBGGI_MODE(vis), sizeof(ggi_mode));

	if (mode->frames != LIBGGI_MODE(vis)->frames)
		return 0;
	if (mode->graphtype != LIBGGI_MODE(vis)->graphtype)
		return 0;
	if (memcmp(&mode->dpp, &LIBGGI_MODE(vis)->dpp, sizeof(ggi_coord)))
		return 0;

	return 1;
}

static void
free_dbs(struct ggi_visual *vis)
{
	int i;

	for (i=LIBGGI_APPLIST(vis)->num-1; i >= 0; i--) {
		_ggi_db_free(LIBGGI_APPBUFS(vis)[i]);
		_ggi_db_del_buffer(LIBGGI_APPLIST(vis), i);
	}
}

int
GGI_directx_setmode(struct ggi_visual *vis, ggi_mode *mode)
{
	directx_priv *priv = GGIDIRECTX_PRIV(vis);
	int i, id, ret;
	char lib[GGI_MAX_APILEN], args[GGI_MAX_APILEN];
	int compatible;
	int change = -1;

	ret = do_checkmode(vis, mode);
	if (ret != 0) {
		return GGI_ENOMATCH;
	}

	GGI_directx_Lock(priv->cs);

	free_dbs(vis);

	compatible = compatible_mode(vis, mode);
	if (!compatible) {
		_ggiZapMode(vis, 0);
		if(priv->lpddp) {
			LIBGGI_PAL(vis)->clut.size = 0;
			if (LIBGGI_PAL(vis)->clut.data)
				free(LIBGGI_PAL(vis)->clut.data);
			LIBGGI_PAL(vis)->rw_start = 0;
			LIBGGI_PAL(vis)->rw_stop  = 0;
			vis->opcolor->setpalvec = NULL;
		}
		change = GGI_CHG_APILIST;
	}

	/* Fill in ggi_pixelformat */
	memset(LIBGGI_PIXFMT(vis), 0, sizeof(ggi_pixelformat));
	setup_pixfmt(LIBGGI_PIXFMT(vis), mode->graphtype);

	_ggi_build_pixfmt(LIBGGI_PIXFMT(vis));

	GGI_directx_DDChangeMode(vis, mode);

	vis->d_frame_num = 0;
	vis->r_frame_num = 0;
	vis->w_frame_num = 0;

	/* Set Up Direct Buffers */

	for (i = 0; i < mode->frames; i++) {
		ggi_resource *res;

		res = malloc(sizeof(ggi_resource));
		if (res == NULL) {
			GGI_directx_Unlock(priv->cs);
			return GGI_EFATAL;
		}
		LIBGGI_APPLIST(vis)->last_targetbuf
		    = _ggi_db_add_buffer(LIBGGI_APPLIST(vis),
					 _ggi_db_get_new());
		LIBGGI_APPBUFS(vis)[i]->resource = res;
		LIBGGI_APPBUFS(vis)[i]->resource->acquire = directx_acquire;
		LIBGGI_APPBUFS(vis)[i]->resource->release = directx_release;
		LIBGGI_APPBUFS(vis)[i]->resource->self =
		    LIBGGI_APPBUFS(vis)[i];
		LIBGGI_APPBUFS(vis)[i]->resource->priv = vis;
		LIBGGI_APPBUFS(vis)[i]->resource->count = 0;
		LIBGGI_APPBUFS(vis)[i]->resource->curactype = 0;
		LIBGGI_APPBUFS(vis)[i]->frame = i;
		LIBGGI_APPBUFS(vis)[i]->type =
		    GGI_DB_NORMAL | GGI_DB_SIMPLE_PLB;
		LIBGGI_APPBUFS(vis)[i]->read =
		    LIBGGI_APPBUFS(vis)[i]->write = priv->lpSurfaceAdd[i];
		LIBGGI_APPBUFS(vis)[i]->layout = blPixelLinearBuffer;
		LIBGGI_APPBUFS(vis)[i]->buffer.plb.stride = priv->pitch;
		LIBGGI_APPBUFS(vis)[i]->buffer.plb.pixelformat =
		    LIBGGI_PIXFMT(vis);

		DPRINT_MODE("DB: %d, addr: %p, stride: %d\n", i,
			       LIBGGI_APPBUFS(vis)[i]->read,
			       LIBGGI_APPBUFS(vis)[i]->buffer.plb.stride);
	}

	vis->r_frame = LIBGGI_APPBUFS(vis)[0];
	vis->w_frame = LIBGGI_APPBUFS(vis)[0];

	LIBGGI_APPLIST(vis)->first_targetbuf
	    = LIBGGI_APPLIST(vis)->last_targetbuf - (mode->frames - 1);

	memcpy(LIBGGI_MODE(vis), mode, sizeof(ggi_mode));
	if (!compatible) {
		for (id = 1; !GGI_directx_getapi(vis, id, lib, args); id++) {
			int rc = _ggiOpenDL(vis, _ggiGetConfigHandle(),
					lib, args, NULL);
			if (rc != 0) {
				fprintf(stderr,
					"display-directx: Error opening the "
					"%s (%s) library\n", lib, args);
				return rc;
			}
			DPRINT("Success in loading %s (%s)\n",
				  lib, args);
		}
		vis->opdraw->setorigin = GGI_directx_setorigin;
		vis->opdraw->setdisplayframe = GGI_directx_setdisplayframe;
	}

	if(priv->lpddp) {
		LIBGGI_PAL(vis)->clut.size = 256;
		LIBGGI_PAL(vis)->clut.data =
			_ggi_malloc(sizeof(ggi_color) *
			LIBGGI_PAL(vis)->clut.size);
		if (LIBGGI_PAL(vis)->clut.data == NULL) {
			GGI_directx_Unlock(priv->cs);
			return GGI_EFATAL;
		}
		vis->opcolor->setpalvec = GGI_directx_setpalvec;
		LIBGGI_PAL(vis)->rw_start = 256;
		LIBGGI_PAL(vis)->rw_stop  = 0;
	}

	GGI_directx_Unlock(priv->cs);

	if (change != -1)
		ggiIndicateChange(vis->instance.stem, change);

	return 0;
}


int
GGI_directx_getmode(struct ggi_visual *vis, ggi_mode *tm)
{
	directx_priv *priv = GGIDIRECTX_PRIV(vis);

	APP_ASSERT(vis != NULL,
			 "directx: GGIgetmode: Visual == NULL");

	GGI_directx_Lock(priv->cs);
	/* We assume the mode in the visual to be OK */
	memcpy(tm, LIBGGI_MODE(vis), sizeof(ggi_mode));
	GGI_directx_Unlock(priv->cs);

	return 0;
}

int
GGI_directx_setpalvec(struct ggi_visual *vis,
	int start, int len, const ggi_color *colormap)
{
	directx_priv *priv = LIBGGI_PRIVATE(vis);

	DPRINT_COLOR(
	"GGI_directx_setpalvec(%p, %d, %d, {%d, %d, %d}) called\n",
	vis, start, len, colormap->r,colormap->g ,colormap->b);

	if (((int)start) == GGI_PALETTE_DONTCARE) {
		start = 10;
		if (start+len > 256)
			start = 256 - (start+len);
		if (start < 0)
			start = 0;
	}

/*	if (((int)(start+len) > 246) || (start < 10) )
		return GGI_EARGINVAL;*/

	GGI_directx_Lock(priv->cs);

	memcpy(LIBGGI_PAL(vis)->clut.data + start,
		colormap,
		len * sizeof(ggi_color));

	if ((size_t)start < LIBGGI_PAL(vis)->rw_start) {
		LIBGGI_PAL(vis)->rw_start = start;
	}
	if ((size_t)(start+len) > LIBGGI_PAL(vis)->rw_stop) {
		LIBGGI_PAL(vis)->rw_stop  = start+len;
	}

	DPRINT_COLOR("setPalette success\n");

/*	if (!(LIBGGI_FLAGS(vis) & GGIFLAG_ASYNC))*/
		GGI_directx_DDChangePalette(vis);

	GGI_directx_Unlock(priv->cs);

	return start;
}

int
GGI_directx_setorigin(struct ggi_visual *vis, int x, int y)
{
	if (x < 0)
		return GGI_EARGINVAL;
	if (y < 0)
		return GGI_EARGINVAL;
	if (x > LIBGGI_VIRTX(vis) - LIBGGI_X(vis))
		return GGI_EARGINVAL;
	if (y > LIBGGI_VIRTY(vis) - LIBGGI_Y(vis))
		return GGI_EARGINVAL;
	vis->origin_x = x;
	vis->origin_y = y;
	if (LIBGGI_FLAGS(vis) & GGIFLAG_ASYNC)
		GGI_directx_DDRedrawAll(vis);
	return 0;
}

int
GGI_directx_setdisplayframe(struct ggi_visual *vis, int num)
{
	ggi_directbuffer *db = _ggi_db_find_frame(vis, num);

	if (db == NULL)
		return GGI_ENOSPACE;

	vis->d_frame_num = num;

	return 0;
}
