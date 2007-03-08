/* $Id: visual.c,v 1.18 2007/03/08 20:54:07 soyt Exp $
******************************************************************************

   Display-monotext: visual management

   Copyright (C) 1998 Andrew Apted		[andrew@ggi-project.org]
   Copyright (C) 1999-2000 Marcus Sundberg	[marcus@ggi-project.org]

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
#include <ctype.h>

#include <ggi/gg.h>
#include <ggi/gii.h>
#include <ggi/gii-module.h>

#include "config.h"
#include <ggi/display/monotext.h>
#include <ggi/internal/ggi_debug.h>


static const gg_option optlist[] =
{
	{ "a",	"0" },
	{ "x",	"2" },
	{ "y",	"4" }
};

#define OPT_A		0
#define OPT_X		1
#define OPT_Y		2

#define NUM_OPTS	(sizeof(optlist)/sizeof(gg_option))


static int
transfer_gii_src(void *arg, int flag, void *data)
{
	struct gg_stem *stem = arg;
	ggi_monotext_priv *priv = MONOTEXT_PRIV(GGI_VISUAL(stem));
	struct gii_source *src = data;

	if (flag == GII_PUBLISH_SOURCE_OPENED)
		giiTransfer(priv->parent, stem, src->origin);
	return 0;
}

static int GGIclose(struct ggi_visual *vis, struct ggi_dlhandle *dlh)
{
	ggi_monotext_priv *priv = MONOTEXT_PRIV(vis);

	DPRINT("display-monotext: GGIdlcleanup start.\n");

	if (priv->fb_ptr != NULL) {
		_ggi_monotextClose(vis);
		free(priv->fb_ptr);
	}

	if (priv->parent != NULL) {
		ggiClose(priv->parent);

		ggDelStem(priv->parent);
		priv->parent = NULL;
	}

	ggLockDestroy(priv->flush_lock);
	free(priv->opmansync);
	free(priv);
	free(LIBGGI_GC(vis));

	DPRINT("display-monotext: GGIdlcleanup done.\n");

	return 0;
}

static int GGIopen(struct ggi_visual *vis, struct ggi_dlhandle *dlh,
			const char *args, void *argptr, uint32_t *dlret)
{
	ggi_monotext_priv *priv;
	gg_option options[NUM_OPTS];
	char target[1024] = "";
	struct gg_api *api;
	struct gg_observer *obs = NULL;
	int val;
	int err = 0;

	DPRINT("display-monotext: GGIopen start.\n");

	memcpy(options, optlist, sizeof(options));

	if (args) {
		args = ggParseOptions(args, options, NUM_OPTS);
		if (args == NULL) {
			fprintf(stderr,
				"display-monotext: error in arguments\n");
			return GGI_EARGINVAL;
		}
	}

	/* open the parent visual */
	DPRINT("display-monotext: opening target: %s\n", args);
	
	if (args != NULL) {
		if (ggParseTarget(args, target, 1024) == NULL) {
			/* error occurred */
			return GGI_EARGINVAL;
		}
	}

	/* Find out the parent target */
	while (args && *args && isspace((uint8_t)*args)) {
		args++;
	}

	*target = '\0';
	if (args) {
		if (ggParseTarget(args, target, 1024) == NULL) {
			return GGI_EARGINVAL;
		}
	}

	if (*target == '\0') {
		strcpy(target, "auto");
	}

	LIBGGI_GC(vis) = malloc(sizeof(ggi_gc));
	if (LIBGGI_GC(vis) == NULL) {
		err = GGI_ENOMEM;
		goto err0;
	}
	LIBGGI_PRIVATE(vis) = priv = malloc(sizeof(ggi_monotext_priv));
	if (priv == NULL) {
		err = GGI_ENOMEM;
		goto err1;
	}

	priv->flush_lock = ggLockCreate();
	if (priv->flush_lock == NULL) {
		err = GGI_ENOMEM;
		goto err2;
	}
	priv->opmansync = malloc(sizeof(_ggi_opmansync));
	if (priv->opmansync == NULL) {
		err = GGI_ENOMEM;
		goto err3;
	}

	priv->flags = 0;
	priv->fb_ptr = NULL;


	DPRINT("display-monotext: opening target: %s\n", target);
	priv->parent = ggNewStem();
	if (priv->parent == NULL) {
		fprintf(stderr,
			"display-monotext: Failed to create stem for target: %s\n",
			target);
		err = GGI_ENODEVICE;
		goto err3;
	}
	/* FIXME! Should iterate over the apis attached to vis->module.stem
	 * instead of only looking for ggi and gii.
	 */
	if (ggiAttach(priv->parent) < 0) {
		ggDelStem(priv->parent);
		priv->parent = NULL;
		fprintf(stderr,
			"display-monotext: Failed to attach ggi to stem for target: %s\n",
			target);
		err = GGI_ENODEVICE;
		goto err4;
	}	 

	api = ggGetAPIByName("gii");
	if (api != NULL) {
		/* FIXME! This should probably be done in pseudo-stubs-gii */
		if (STEM_HAS_API(vis->module.stem, api)) {
			if (ggAttach(api, priv->parent) < 0) {
				ggDelStem(priv->parent);
				priv->parent = NULL;
				fprintf(stderr,
					"Failed to attach gii to stem for target: %s\n",
					target);
				err = GGI_ENODEVICE;
				goto err4;
			}
			obs = ggAddObserver(ggGetPublisher(api, priv->parent,
					GII_PUBLISHER_SOURCE_CHANGE),
				transfer_gii_src, vis->module.stem);
		}
	}

	if (ggiOpen(priv->parent, target, NULL) < 0) {
		if (obs) {
			ggDelObserver(obs);
			obs = NULL;
		}
		fprintf(stderr,
			"display-monotext: Failed to open target: '%s'\n",
			target);
		ggDelStem(priv->parent);
		priv->parent = NULL;
		err = GGI_ENODEVICE;
		goto err4;
	}
	if (obs) {
		ggDelObserver(obs);
		obs = NULL;
	}

	ggiSetFlags(priv->parent, GGIFLAG_ASYNC);



	/* set defaults */
	priv->parent_gt.graphtype = GT_TEXT16;
	priv->flags = 0;
	priv->squish.x = priv->squish.y = 1;

	val = strtol(options[OPT_A].result, NULL, 0);
	if (val != 0) {
		priv->accuracy.x = priv->accuracy.y = val;
	} else {
		priv->accuracy.x = strtol(options[OPT_X].result, NULL, 0);
		priv->accuracy.y = strtol(options[OPT_Y].result, NULL, 0);
	}

	/* Setup mansync */
	err = _ggiAddDL(vis, _ggiGetConfigHandle(),
			"helper-mansync", NULL, priv->opmansync, 0);
	if (err) {
		fprintf(stderr,
			"display-monotext: Cannot load helper-mansync!\n");
		GGIclose(vis, dlh);
		goto err4;
	}

	MANSYNC_init(vis);
	if (!(LIBGGI_FLAGS(vis) & GGIFLAG_ASYNC)) {
		MANSYNC_start(vis);
	}

	vis->opdisplay->getmode   = GGI_monotext_getmode;
	vis->opdisplay->setmode   = GGI_monotext_setmode;
	vis->opdisplay->checkmode = GGI_monotext_checkmode;
	vis->opdisplay->getapi    = GGI_monotext_getapi;
	vis->opdisplay->flush     = GGI_monotext_flush;
	vis->opdisplay->setflags  = GGI_monotext_setflags;
	
	DPRINT("display-monotext: GGIopen succeeded.\n");

	*dlret = GGI_DL_OPDISPLAY;
	return 0;

err4:
	free(priv->opmansync);
err3:
	ggLockDestroy(priv->flush_lock);
err2:
	free(priv);	
err1:
	free(LIBGGI_GC(vis));
err0:
	return err;
}



EXPORTFUNC
int GGIdl_monotext(int func, void **funcptr);

int GGIdl_monotext(int func, void **funcptr)
{
	ggifunc_open **openptr;
	ggifunc_close **closeptr;

	switch (func) {
	case GGIFUNC_open:
		openptr = (ggifunc_open **)funcptr;
		*openptr = GGIopen;
		return 0;
	case GGIFUNC_exit:
		*funcptr = NULL;
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
