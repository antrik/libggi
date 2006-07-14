/* $Id: visual.c,v 1.27 2006/07/14 02:35:48 pekberg Exp $
******************************************************************************

   Initializing tiles

   Copyright (C) 1998 Steve Cheng	[steve@ggi-project.org]
   Copyright (C) 2000 Marcus Sundberg	[marcus@ggi-project.org]

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

#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "config.h"
#include <ggi/display/tile.h>
#include <ggi/internal/ggi_debug.h>
#include <ggi/gg.h>
#include <ggi/gii.h>
#include <ggi/gii-module.h>


void _GGI_tile_freedbs(struct ggi_visual *vis)
{
	int i;

	for (i=LIBGGI_APPLIST(vis)->num-1; i >= 0; i--) {
		free(LIBGGI_APPBUFS(vis)[i]->write);
		_ggi_db_free(LIBGGI_APPBUFS(vis)[i]);
		_ggi_db_del_buffer(LIBGGI_APPLIST(vis), i);
	}
}

struct transfer {
	struct gg_stem *src;
	struct gg_stem *dst;
};

static int
transfer_gii_src(void *arg, int flag, void *data)
{
	struct transfer *xfer = arg;
	struct gii_source *src = data;

	if (flag == GII_PUBLISH_SOURCE_OPENED) {
		giiTransfer(xfer->src, xfer->dst, src->origin);
	}
	return 0;
}



static const char argument_format[] = "display-tile:\n\
    The argument format is `offx,offy,sizex,sizey,(subdisplay):...',\n\
    where offx and offy are the tile's offset from the main display,\n\
    sizex and sizey are the size of the tile,\n\
    subdisplay is the display string to open for the tile,\n\
    and ... is more tiles following the same format as above...\n";


static int GGIopen_tile(struct ggi_visual *vis, struct ggi_dlhandle *dlh,
			const char *args, void *argptr, uint32_t *dlret)
{
	ggi_tile_priv *priv;
	char target[1024];
	struct multi_vis *mvis;
	int sx, sy, vx, vy, n, i=0;
	int err = GGI_ENOMEM;
	struct gg_api *api;
	struct gg_observer *obs = NULL;
	struct transfer xfer;

	DPRINT_LIBS("GGIopen(%p, %p, %s, %p, %u) entered\n",
			(void *)vis, (void *)dlh, args, argptr, dlret);

	if (!args || *args == '\0') {
		fprintf(stderr, "display-tile needs the real targets as arguments.\n");
		fprintf(stderr, argument_format);
		return GGI_EARGREQ;
	}

	priv = calloc(1, sizeof(ggi_tile_priv));
	if (priv == NULL) return GGI_ENOMEM;
	LIBGGI_PRIVATE(vis) = priv;

	LIBGGI_GC(vis) = malloc(sizeof(ggi_gc));
	if (LIBGGI_GC(vis) == NULL) {
		goto out_freepriv;
	}

	priv->opmansync = malloc(sizeof(_ggi_opmansync));
	if (priv->opmansync == NULL) {
		goto out_freegc;
	}

	GG_TAILQ_INIT(&priv->vislist);
	priv->buf = NULL;
	priv->use_db = 1;
	priv->multi_mode = 0;

	api = ggGetAPIByName("gii");

	/* parse each visual */
	for (;;) {
		sx = sy = vx = vy = 0;

		while (*args && isspace((uint8_t)*args)) args++;

		if (! *args) break;

		if (strncmp(args, "-usedb:", 7) == 0) {
			DPRINT_MISC("display-tile: Enabling DB\n");
			priv->use_db = 1;
			args += 7; continue;
		}
		if (strncmp(args, "-nodb:", 6) == 0) {
			DPRINT_MISC("display-tile: Disabling DB\n");
			priv->use_db = 0;
			args += 6; continue;
		}

		err = GGI_EARGINVAL;

		if ((sscanf(args, "%d , %d , %d , %d %n", &sx, &sy,
			    &vx, &vy, &n) != 4) ||
		    (args += n, *args++ != ',')) {
			fprintf(stderr, argument_format);
			goto out_freeopmansync;
		}

		/* Empty tile! */
		if(vx<=0 || vy<=0 || sx<0 || sy<0) {
			fprintf(stderr, "display-tile: erroneous coordinates for tile #%d!\n", i);
			goto out_freeopmansync;
		}

		mvis = calloc(1, sizeof(struct multi_vis));
		if (mvis == NULL) {
			fprintf(stderr, "display-tile: out of memory\n");
			goto out_freeopmansync;
		}

		mvis->origin.x = sx;
		mvis->origin.y = sy;
		mvis->size.x = vx;
		mvis->size.y = vy;

		args = ggParseTarget(args, target, sizeof(target));

		if (! args) {
			fprintf(stderr,"display-tile: parsetarget error.\n");
			goto out_freeopmansync;
		}

		if (*target == '\0') {
			ggstrlcpy(target, "auto", sizeof(target));
		}

		DPRINT_MISC("display-tile: visual #%d is %s (%d,%d)[%dx%d]\n",
			i, target, sx, sy, vx, vy);

		mvis->vis = ggNewStem();
		if (mvis->vis == NULL) {
			fprintf(stderr,
				"display-tile: Failed to create stem for target '%s'\n",
				target);
			err = GGI_ENODEVICE;
			goto out_freeopmansync;
		}
		/* XXX Should iterate over the apis attached to vis->stem
		 * instead of only looking for ggi and gii.
		 */
		if (ggiAttach(mvis->vis) < 0) {
			ggDelStem(mvis->vis);
			mvis->vis = NULL;
			fprintf(stderr,
				"display-tile: Failed to attach ggi to stem for target '%s'\n",
				target);
			err = GGI_ENODEVICE;
			goto out_freeopmansync;
		}
		if (api != NULL && STEM_HAS_API(vis->stem, api)) {
			if (ggAttach(api, mvis->vis) < 0) {
				ggDelStem(mvis->vis);
				mvis->vis = NULL;
				fprintf(stderr,
					"display-tile: Failed to attach gii to stem for target '%s'\n",
					target);
				err = GGI_ENODEVICE;
				goto out_freeopmansync;
			}
			xfer.src = mvis->vis;
			xfer.dst = vis->stem;
			obs = ggAddObserver(ggGetPublisher(api, mvis->vis,
						GII_PUBLISHER_SOURCE_CHANGE),
					transfer_gii_src, &xfer);
		}

		if (ggiOpen(mvis->vis, target,NULL) < 0) {
			if (obs != NULL) {
				ggDelObserver(obs);
				obs = NULL;
			}
			fprintf(stderr,"display-tile: Opening of target %s failed.\n", target);
			ggDelStem(mvis->vis);
			mvis->vis = NULL;
			err = GGI_ENODEVICE;
			goto out_freeopmansync;
		}
		if (obs != NULL) {
			ggDelObserver(obs);
			obs = NULL;
		}

		DPRINT_MISC("GGIopen: Collect input sources\n");

		if (priv->use_db) {
			/* Don't need SYNC mode, we do it ourselves */
			ggiSetFlags(mvis->vis, GGIFLAG_ASYNC);
		}

		/* check for ':' separator */

		while (*args && isspace((uint8_t)*args)) args++;

		if (*args && (*args != ':')) {
			fprintf(stderr, "display-tile: expecting ':' between targets.\n");
			err = GGI_EARGINVAL;
			goto out_closevisuals;
		}

		if (*args == ':') args++;
		i++;

		tile_INSERT(priv, mvis);
	}

	if (tile_EMPTY(priv)) {
		fprintf(stderr, "display-tile needs the real targets as arguments.\n");
		err = GGI_EARGINVAL;
		goto out_freeopmansync;
	}

	if (priv->use_db) {
		err = _ggiAddDL(vis, _ggiGetConfigHandle(),
				"helper-mansync", NULL,
				priv->opmansync, 0);
		if (err) {
			fprintf(stderr, "display-tile: Cannot load required helper-mansync! (for DB mode)\n");
			goto out_closevisuals;

		}

		DPRINT_LIBS("GGIopen: initialize mansync\n");
		MANSYNC_init(vis);
		if (!(LIBGGI_FLAGS(vis) & GGIFLAG_ASYNC)) {
			MANSYNC_start(vis);
		}
	}

	vis->opdisplay->getmode=GGI_tile_getmode;
	vis->opdisplay->setmode=GGI_tile_setmode;
	vis->opdisplay->checkmode=GGI_tile_checkmode;
	vis->opdisplay->getapi=GGI_tile_getapi;
	vis->opdisplay->setflags=GGI_tile_setflags;

	if (priv->use_db) {
		vis->opdisplay->flush=GGI_tile_flush_db;
	} else {
		vis->opdisplay->flush=GGI_tile_flush;
	}

	*dlret = GGI_DL_OPDISPLAY;
	return 0;

  out_closevisuals:
	/* Close opened visuals. */
	while (!tile_EMPTY(priv)) {
		mvis = tile_FIRST(priv);
		tile_REMOVE(priv, mvis);
		ggiClose(mvis->vis);
		free(mvis);
	}
  out_freeopmansync:
	free(priv->opmansync);
  out_freegc:
	free(LIBGGI_GC(vis));
  out_freepriv:
	free(priv);

	return err;
}

static int GGIopen_multi(struct ggi_visual *vis, struct ggi_dlhandle *dlh,
		const char *args, void *argptr, uint32_t *dlret)
{
	ggi_tile_priv *priv;
	struct multi_vis *mvis;
	char target[1024];
	struct gg_api *api;
	struct gg_observer *obs = NULL;
	struct transfer xfer;
	int err = GGI_ENOMEM;
	int i = 0;

	if (!args || *args == '\0') {
		fprintf(stderr, "display-multi: missing target names.\n");
		return GGI_EARGREQ;
	}

	priv = calloc(1, sizeof(ggi_tile_priv));
	if (priv == NULL) return GGI_ENOMEM;
	LIBGGI_PRIVATE(vis) = priv;

	LIBGGI_GC(vis) = malloc(sizeof(ggi_gc));
	if (LIBGGI_GC(vis) == NULL) {
		goto out_freepriv;
	}

	
	GG_TAILQ_INIT(&(priv->vislist));

	err = GGI_EARGINVAL;
	priv->buf = NULL;
	priv->use_db = 0;
	priv->multi_mode = 1;

	api = ggGetAPIByName("gii");

	for (;;) {
		if (! *args) break;

		args = ggParseTarget(args, target, 1024);

		if (args == NULL) {
			goto out_freeall;
		}

		if (*target == '\0') {
			ggstrlcpy(target, "display-auto", sizeof(target));
		}

		mvis = calloc(1, sizeof(struct multi_vis));
		if (mvis == NULL) continue;

		DPRINT("display-multi: opening sub #%d: %s\n",
			i, target);

		mvis->origin.x = 0;
		mvis->origin.y = 0;
		mvis->size.x = 0;
		mvis->size.y = 0;


		mvis->vis = ggNewStem();
		if (mvis->vis == NULL) {
			fprintf(stderr,
				"display-multi: Failed to create stem for target '%s'\n",
				target);
			err = GGI_ENODEVICE;
			goto out_freeall;
		}
		/* XXX Should iterate over the apis attached to vis->stem
		 * instead of only looking for ggi and gii.
		 */
		if (ggiAttach(mvis->vis) < 0) {
			ggDelStem(mvis->vis);
			mvis->vis = NULL;
			fprintf(stderr,
				"display-multi: Failed to attach ggi to stem for target '%s'\n",
				target);
			err = GGI_ENODEVICE;
			goto out_freeall;
		}
		if (api != NULL && STEM_HAS_API(vis->stem, api)) {
			if (ggAttach(api, mvis->vis) < 0) {
				ggDelStem(mvis->vis);
				mvis->vis = NULL;
				fprintf(stderr,
					"display-multi: Failed to attach gii to stem for target '%s'\n",
					target);
				err = GGI_ENODEVICE;
				goto out_freeall;
			}
			xfer.dst = mvis->vis;
			xfer.src = mvis->vis;
			obs = ggAddObserver(ggGetPublisher(api, mvis->vis,
						GII_PUBLISHER_SOURCE_CHANGE),
					transfer_gii_src, &xfer);
		}

		ggiOpen(mvis->vis, target, NULL);
		if (mvis->vis == NULL) {
			if (obs != NULL) {
				ggDelObserver(obs);
				obs = NULL;
			}
			fprintf(stderr, "display-multi: failed trying "
				"to open %s\n", target);
			ggDelStem(mvis->vis);
			err = GGI_ENODEVICE;
			goto out_freeall;
		}
		if (obs != NULL) {
			ggDelObserver(obs);
			obs = NULL;
		}

		/* check for ':' separator */
		while (*args && isspace((uint8_t)*args)) args++;

		if (*args && (*args != ':')) {
			fprintf(stderr, "display-multi: args: %c\n", *args);
			fprintf(stderr, "display-multi: expecting ':' between targets.\n");
			err = GGI_EARGINVAL;
			goto out_freeall;
		}

		args++; /* skip ':' */

		/* add to list */
		i++;
		tile_INSERT(priv, mvis);
	}	
	
	if (tile_EMPTY(priv)) {
		fprintf(stderr, "display-multi: no targets specified\n");
		err = GGI_EARGINVAL;
		goto out_freepriv;
	}

	vis->opdisplay->getmode = GGI_tile_getmode;
	vis->opdisplay->setmode = GGI_tile_setmode;
	vis->opdisplay->checkmode = GGI_tile_checkmode;
	vis->opdisplay->setflags = GGI_tile_setflags;
	vis->opdisplay->getapi = GGI_tile_getapi;
	vis->opdisplay->flush = GGI_tile_flush;

	*dlret = GGI_DL_OPDISPLAY;
	return 0;

  out_freeall:
	/* Close opened visuals */
	while (!tile_EMPTY(priv)) {
		mvis = tile_FIRST(priv);
		tile_REMOVE(priv, mvis);
		ggiClose(mvis->vis);
		free(mvis);
	}
  out_freepriv:
	free(priv);

	return err;
}


static int GGIclose(struct ggi_visual *vis, struct ggi_dlhandle *dlh)
{
	ggi_tile_priv *priv = TILE_PRIV(vis);
	struct multi_vis *elm;

	DPRINT_LIBS("GGIclose(%p, %p) entered\n",
			(void *)vis, (void *)dlh);

	if (priv->use_db) {
		_GGI_tile_freedbs(vis);
	}

	if (priv->buf) free(priv->buf);

	/* Shut down targets in reverse order to avoid any nesting problems,
	 * if external dependencies (like saving/restoring state on subsystems
	 * that are used by multiple targets).
	 */
	while (!tile_EMPTY(priv)) {
		elm = tile_FIRST(priv);
		tile_REMOVE(priv, elm);
		ggiClose(elm->vis);
		free(elm);		
	}

	free(priv->opmansync);
	free(priv);
	free(LIBGGI_GC(vis));

	return 0;
}


static int GGIexit(struct ggi_visual *vis, struct ggi_dlhandle *dlh)
{
	ggi_tile_priv *priv = TILE_PRIV(vis);

	DPRINT_LIBS("GGIexit(%p, %p) entered\n",
			(void *)vis, (void *)dlh);

	if (priv->use_db) {
		DPRINT_LIBS("GGIexit: de-initialize mansync\n");

		if (!(LIBGGI_FLAGS(vis) & GGIFLAG_ASYNC)) {
			MANSYNC_stop(vis);
		}
		MANSYNC_deinit(vis);
	}

	return 0;
}

EXPORTFUNC
int GGIdl_tile(int func, void **funcptr);

int GGIdl_tile(int func, void **funcptr)
{
	ggifunc_open **openptr;
	ggifunc_exit **exitptr;
	ggifunc_close **closeptr;

	switch (func) {
	case GGIFUNC_open:
		openptr = (ggifunc_open **)funcptr;
		*openptr = GGIopen_tile;
		return 0;
	case GGIFUNC_exit:
		exitptr = (ggifunc_exit **)funcptr;
		*exitptr = GGIexit;
		return 0;
	case GGIFUNC_close:
		closeptr = (ggifunc_close **)funcptr;
		*closeptr = GGIclose;
		return 0;
	default:
		*funcptr = NULL;
	}

	return GGI_ENOTFOUND;
}

EXPORTFUNC
int GGIdl_multi(int func, void **funcptr);

int GGIdl_multi(int func, void **funcptr)
{
	ggifunc_open **openptr;
	ggifunc_exit **exitptr;
	ggifunc_close **closeptr;

	switch (func) {
	case GGIFUNC_open:
		openptr = (ggifunc_open **)funcptr;
		*openptr = GGIopen_multi;
		return 0;
	case GGIFUNC_exit:
		exitptr = (ggifunc_exit **)funcptr;
		*exitptr = GGIexit;
		return 0;
	case GGIFUNC_close:
		closeptr = (ggifunc_close **)funcptr;
		*closeptr = GGIclose;
		return 0;
	default:
		*funcptr = NULL;
	}

	return GGI_ENOTFOUND;
}
