/* $Id: visual.c,v 1.14 2006/03/20 20:41:05 cegger Exp $
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

#include "config.h"
#include <ggi/display/monotext.h>
#include <ggi/internal/ggi_debug.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>


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


static int GGIopen(struct ggi_visual *vis, struct ggi_dlhandle *dlh,
			const char *args, void *argptr, uint32_t *dlret)
{
	ggi_monotext_priv *priv;
	struct ggi_visual *parentvis;
	gg_option options[NUM_OPTS];
	char target[1024] = "";
	int  val;

	DPRINT("display-monotext: GGIdlinit start.\n");

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

	if (*target == '\0') {
		strcpy(target, "auto");
	}

	parentvis = ggiOpen(target, NULL);
	if (parentvis == NULL) {
		fprintf(stderr,
			"display-monotext: Failed to open target: %s\n",
			target);
		return GGI_ENODEVICE;
	}
	ggiSetFlags(parentvis, GGIFLAG_ASYNC);

	priv = malloc(sizeof(ggi_monotext_priv));
	if (priv == NULL) {
		ggiClose(parentvis);
		return GGI_ENOMEM;
	}

	LIBGGI_GC(vis) = malloc(sizeof(ggi_gc));
	if (LIBGGI_GC(vis) == NULL) {
		free(priv);
		ggiClose(parentvis);
		return GGI_ENOMEM;
	}

	/* set defaults */
	priv->parent = parentvis;
	priv->parent_gt = GT_TEXT16;
	priv->flags = 0;
	priv->squish.x = priv->squish.y = 1;

	val = strtol(options[OPT_A].result, NULL, 0);
	if (val != 0) {
		priv->accuracy.x = priv->accuracy.y = val;
	} else {
		priv->accuracy.x = strtol(options[OPT_X].result, NULL, 0);
		priv->accuracy.y = strtol(options[OPT_Y].result, NULL, 0);
	}

	/* add giiInputs, if we have them */
	if (priv->parent->input) {
		vis->input = giiJoinInputs(vis->input, priv->parent->input);
		priv->parent->input = NULL; /* destroy old reference */
	}
	
	LIBGGI_PRIVATE(vis) = priv;

	vis->opdisplay->getmode   = GGI_monotext_getmode;
	vis->opdisplay->setmode   = GGI_monotext_setmode;
	vis->opdisplay->checkmode = GGI_monotext_checkmode;
	vis->opdisplay->getapi    = GGI_monotext_getapi;
	vis->opdisplay->flush     = GGI_monotext_flush;
	vis->opdisplay->setflags  = GGI_monotext_setflags;
	
	DPRINT("display-monotext: GGIdlinit succeeded.\n");

	*dlret = GGI_DL_OPDISPLAY;
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

		giiClose(vis->input);
		vis->input = NULL;
	}

	free(priv);
	free(LIBGGI_GC(vis));

	DPRINT("display-monotext: GGIdlcleanup done.\n");

	return 0;
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
