/* $Id: mode.c,v 1.1 2006/08/19 23:31:32 pekberg Exp $
******************************************************************************

   Display-vnc: mode management

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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>

#include "config.h"
#include <ggi/display/vnc.h>
#include <ggi/internal/ggi_debug.h>
#include <ggi/internal/gg_replace.h>	/* for snprintf() */

#include "../common/pixfmt-setup.inc"
#include "../common/ggi-auto.inc"
#include "../common/gt-auto.inc"

/*
static int
vnc_acquire(ggi_resource *res, uint32_t actype)
{
	ggi_directbuffer *dbuf;
	struct ggi_visual *vis;
	ggi_vnc_priv *priv;
	int bufnum;

	DPRINT_MISC("acquire(%p, 0x%x) called\n", res, actype);

	if (actype & ~(GGI_ACTYPE_READ | GGI_ACTYPE_WRITE)) {
		return GGI_EARGINVAL;
	}
	vis = res->priv;
	priv = VNC_PRIV(vis);
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
	dbuf->write = foo?
	dbuf->read = dbuf->write;

	res->curactype |= actype;
	res->count++;

	DPRINT_MISC("acquire - success, count: %d\n", res->count);

	return 0;
}

static int
vnc_release(ggi_resource *res)
{
	DPRINT_MISC("release(%p) called\n", res);

	if (res->count < 1)
		return GGI_ENOTALLOC;

	res->count--;

	return 0;
}
*/

static int GGI_vnc_flush(struct ggi_visual *vis, 
			int x, int y, int w, int h, int tryflag)
{
	/* ggi_vnc_priv *priv = VNC_PRIV(vis); */

	return 0;
}

int GGI_vnc_getapi(struct ggi_visual *vis, int num, char *apiname, char *arguments)
{
	ggi_graphtype gt = LIBGGI_GT(vis);

	*arguments = '\0';

	switch(num) { 

	case 0: strcpy(apiname, "display-vnc");
		return 0;

	case 1: strcpy(apiname, "generic-stubs");
		return 0;
		
	case 2: strcpy(apiname, "generic-color");
		return 0;

	case 3: if (GT_SCHEME(gt) == GT_TEXT) {
			sprintf(apiname, "generic-text-%u", GT_SIZE(gt));
			return 0;
		}

		sprintf(apiname, "generic-linear-%u%s", GT_SIZE(gt),
			(gt & GT_SUB_HIGHBIT_RIGHT) ? "-r" : "");
		return 0;
	}

	return GGI_ENOMATCH;
}

static void _ggi_freedbs(struct ggi_visual *vis) 
{
	int i;

	for (i=LIBGGI_APPLIST(vis)->num-1; i >= 0; i--) {
		_ggi_db_free(LIBGGI_APPBUFS(vis)[i]);
		_ggi_db_del_buffer(LIBGGI_APPLIST(vis), i);
	}
}

static int _ggi_domode(struct ggi_visual *vis)
{
	ggi_vnc_priv *priv = VNC_PRIV(vis);
	int err, i;
	char name[GGI_MAX_APILEN];
	char args[GGI_MAX_APILEN];

	_ggiZapMode(vis, 0);
	_ggi_freedbs(vis);

	DPRINT("_ggi_domode: zapped\n");

	priv->fb = malloc(GT_ByPPP(
				LIBGGI_VIRTX(vis) *
				LIBGGI_VIRTY(vis) *
				LIBGGI_MODE(vis)->frames, LIBGGI_GT(vis)));
	if (!priv->fb)
		return GGI_ENOMEM;

	/* set up pixel format */
	memset(LIBGGI_PIXFMT(vis), 0, sizeof(ggi_pixelformat));
	setup_pixfmt(LIBGGI_PIXFMT(vis), LIBGGI_GT(vis));
	_ggi_build_pixfmt(LIBGGI_PIXFMT(vis));

	vis->d_frame_num = 0;
	vis->r_frame_num = 0;
	vis->w_frame_num = 0;

	/* Set Up Direct Buffers */

	for (i = 0; i < LIBGGI_MODE(vis)->frames; i++) {
		/*
		ggi_resource *res;

		res = malloc(sizeof(ggi_resource));
		if (!res) {
			return GGI_EFATAL;
		}
		*/
		LIBGGI_APPLIST(vis)->last_targetbuf =
			_ggi_db_add_buffer(LIBGGI_APPLIST(vis),
							_ggi_db_get_new());
		LIBGGI_APPBUFS(vis)[i]->resource = NULL; /* res; */
/*		LIBGGI_APPBUFS(vis)[i]->resource->acquire = vnc_acquire;
		LIBGGI_APPBUFS(vis)[i]->resource->release = vnc_release;
		LIBGGI_APPBUFS(vis)[i]->resource->self =
			LIBGGI_APPBUFS(vis)[i];
		LIBGGI_APPBUFS(vis)[i]->resource->priv = vis;
		LIBGGI_APPBUFS(vis)[i]->resource->count = 0;
		LIBGGI_APPBUFS(vis)[i]->resource->curactype = 0; */
		LIBGGI_APPBUFS(vis)[i]->frame = i;
		LIBGGI_APPBUFS(vis)[i]->type =
			GGI_DB_NORMAL | GGI_DB_SIMPLE_PLB;
		LIBGGI_APPBUFS(vis)[i]->read = LIBGGI_APPBUFS(vis)[i]->write =
			priv->fb +
			GT_ByPPP(LIBGGI_VIRTX(vis) * LIBGGI_VIRTY(vis) * i,
				LIBGGI_GT(vis));
		LIBGGI_APPBUFS(vis)[i]->layout = blPixelLinearBuffer;
		LIBGGI_APPBUFS(vis)[i]->buffer.plb.stride =
			GT_ByPPP(LIBGGI_VIRTX(vis), LIBGGI_GT(vis));
		LIBGGI_APPBUFS(vis)[i]->buffer.plb.pixelformat =
			LIBGGI_PIXFMT(vis);

		DPRINT_MODE("DB: %d, addr: %p, stride: %d\n", i,
			       LIBGGI_APPBUFS(vis)[i]->read,
			       LIBGGI_APPBUFS(vis)[i]->buffer.plb.stride);
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
		LIBGGI_PAL(vis)->clut.data = _ggi_malloc(sizeof(ggi_color) * num_cols);
		LIBGGI_PAL(vis)->clut.size = num_cols;
	}

	for(i=1; GGI_vnc_getapi(vis, i, name, args) == 0; i++) {
		err = _ggiOpenDL(vis, _ggiGetConfigHandle(),
				name, args, NULL);
		if (err) {
			fprintf(stderr,"display-vnc: Can't open the "
				"%s (%s) library.\n", name, args);
			return GGI_EFATAL;
		} else {
			DPRINT_LIBS("Success in loading "
				       "%s (%s)\n", name, args);
		}
	}

	if (GT_SCHEME(LIBGGI_GT(vis)) == GT_PALETTE) {
		LIBGGI_PAL(vis)->setPalette = GGI_vnc_setPalette;
	}
	vis->opdisplay->flush = GGI_vnc_flush;
	
	return 0;
}

int GGI_vnc_setmode(struct ggi_visual *vis, ggi_mode *mode)
{ 
	/* ggi_vnc_priv *priv = VNC_PRIV(vis); */
	int err;

	if (vis==NULL || mode==NULL || LIBGGI_MODE(vis)==NULL) {
		return GGI_EARGINVAL;
	}
	
	DPRINT_MODE("setmode %dx%d#%dx%dF%d[0x%02x]\n",
			mode->visible.x, mode->visible.y,
			mode->virt.x, mode->virt.y, 
			mode->frames, mode->graphtype);

	if ((err = ggiCheckMode(vis->stem, mode)) != 0) {
		return err;
	}

	*LIBGGI_MODE(vis) = *mode;

	err = _ggi_domode(vis);

	if (err) {
		DPRINT("domode failed (%d)\n",err);
		return err;
	}

	ggiIndicateChange(vis->stem, GGI_CHG_APILIST);
	DPRINT("change indicated\n",err);

	return 0;
}

int GGI_vnc_checkmode(struct ggi_visual *vis, ggi_mode *mode)
{
	/* ggi_vnc_priv *priv = VNC_PRIV(vis); */
	int err = 0;

	DPRINT_MODE("checkmode %dx%d#%dx%dF%d[0x%02x]\n",
			mode->visible.x, mode->visible.y,
			mode->virt.x, mode->virt.y, 
			mode->frames, mode->graphtype);

	/* handle GT_AUTO and GGI_AUTO */
	_GGIhandle_ggiauto(mode, 640, 400);

	mode->graphtype = _GGIhandle_gtauto(mode->graphtype);

	/* do some checks */
	if (mode->virt.x < mode->visible.x) {
		mode->virt.x = mode->visible.x;
		err = -1;
	}

	if (mode->virt.y < mode->visible.y) {
		mode->virt.y = mode->visible.y;
		err = -1;
	}

	if (mode->frames != 1 && mode->frames != GGI_AUTO) {
		err = -1;
	}
	mode->frames = 1;

	if ((mode->dpp.x != 1 && mode->dpp.x != GGI_AUTO) ||
	    (mode->dpp.y != 1 && mode->dpp.y != GGI_AUTO)) {
		err = -1;
	}
	mode->dpp.x = mode->dpp.y = 1;

	DPRINT_MODE("result %d %dx%d#%dx%dF%d[0x%02x]\n",
			err, mode->visible.x, mode->visible.y,
			mode->virt.x, mode->virt.y, 
			mode->frames, mode->graphtype);

	return err;	
}

int GGI_vnc_getmode(struct ggi_visual *vis, ggi_mode *mode)
{
	DPRINT("getmode(%p,%p)\n", vis, mode);

	if (vis==NULL || mode==NULL || LIBGGI_MODE(vis)==NULL) {
		return GGI_EARGINVAL;
	}

	memcpy(mode, LIBGGI_MODE(vis), sizeof(ggi_mode));

	return 0;
}

int GGI_vnc_setflags(struct ggi_visual *vis, ggi_flags flags)
{
	LIBGGI_FLAGS(vis) = flags;

	LIBGGI_FLAGS(vis) &= GGIFLAG_ASYNC; /* Unkown flags don't take. */

	return 0;
}

int GGI_vnc_setPalette(struct ggi_visual *vis, size_t start, size_t size, const ggi_color *colormap)
{
	return 0;
}
