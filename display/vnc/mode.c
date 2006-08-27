/* $Id: mode.c,v 1.4 2006/08/27 11:45:17 pekberg Exp $
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

static int
GGI_vnc_flush(struct ggi_visual *vis, 
	int x, int y, int w, int h, int tryflag)
{
	ggi_vnc_priv *priv = VNC_PRIV(vis);

	return _ggiFlushRegion(priv->fb, x, y, w, h);
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

	priv->dirty.tl.x = priv->dirty.tl.y = 0;
	priv->dirty.br = priv->dirty.tl;

	ggiSetMode(priv->fb->stem, LIBGGI_MODE(vis));

	/* set up pixel format */
	memcpy(LIBGGI_PIXFMT(vis), LIBGGI_PIXFMT(priv->fb), sizeof(ggi_pixelformat));

	vis->d_frame_num = priv->fb->d_frame_num;
	vis->r_frame_num = priv->fb->r_frame_num;
	vis->w_frame_num = priv->fb->w_frame_num;

	/* Set Up Direct Buffers */

#if 0
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
#endif

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
	ggi_vnc_priv *priv = VNC_PRIV(vis);
	int err;

	DPRINT_MODE("checkmode %dx%d#%dx%dF%d[0x%02x]\n",
			mode->visible.x, mode->visible.y,
			mode->virt.x, mode->virt.y, 
			mode->frames, mode->graphtype);

	err = ggiCheckMode(priv->fb->stem, mode);

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
	ggi_vnc_priv *priv = VNC_PRIV(vis);

	LIBGGI_FLAGS(vis) = flags;

	LIBGGI_FLAGS(vis) &= GGIFLAG_ASYNC; /* Unknown flags don't take. */

	return _ggiSetFlags(priv->fb, flags);
}
