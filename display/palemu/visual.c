/* $Id: visual.c,v 1.9 2004/10/29 22:34:31 cegger Exp $
******************************************************************************

   Display-palemu: initialization

   Copyright (C) 1998 Andrew Apted	[andrew@ggi-project.org]
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
#include <ggi/internal/ggi-dl.h>
#include <ggi/display/palemu.h>


static const gg_option optlist[] =
{
	{ "parent", "[]" }
};

#define OPT_PARENT	0

#define NUM_OPTS	(sizeof(optlist)/sizeof(gg_option))



static int GGIclose(ggi_visual *vis, struct ggi_dlhandle *dlh)
{
	ggi_palemu_priv *priv = PALEMU_PRIV(vis);

	GGIDPRINT("display-palemu: GGIclose start.\n");

	if (priv->fb_ptr != NULL) {
		GGI_palemu_resetmode(vis);
	}

	if (priv->parent != NULL) {
		ggiClose(priv->parent);
	}

	ggLockDestroy(priv->flush_lock);
	free(priv->opmansync);
	free(priv);
	free(LIBGGI_GC(vis));

	GGIDPRINT("display-palemu: GGIclose done.\n");

	return 0;
}


static int GGIopen(ggi_visual *vis, struct ggi_dlhandle *dlh,
		   const char *args, void *argptr, uint32 *dlret)
{
	ggi_palemu_priv *priv;
	gg_option options[NUM_OPTS];
	char target[1024];
	int err = GGI_ENOMEM;

	GGIDPRINT("display-palemu: GGIopen start.\n");

	/* handle arguments */
	memcpy(options, optlist, sizeof(options));
	if (args) {
		args = ggParseOptions(args, options, NUM_OPTS);

		if (args == NULL) {
			fprintf(stderr,
				"display-palemu: error in arguments.\n");
			return GGI_EARGINVAL;
		}
	}
	if (getenv("GGI_PALEMU_OPTIONS") != NULL) {
		if (ggParseOptions(getenv("GGI_PALEMU_OPTIONS"), options,
				   NUM_OPTS) == NULL) {
			fprintf(stderr, "display-palemu: error in ""$GGI_PALEMU_OPTIONS.\n");
			return GGI_EARGINVAL;
		}
	}

	/* Find out the parent target. */
	while (args && *args && isspace((uint8)*args)) {
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
		return GGI_ENOMEM;
	}
	PALEMU_PRIV(vis) = priv = malloc(sizeof(*priv));
	if (priv == NULL) {
		goto out_freegc;
	}
	priv->flush_lock = ggLockCreate();
	if (priv->flush_lock == NULL) {
		goto out_freepriv;
	}
	priv->opmansync = malloc(sizeof(_ggi_opmansync));
	if (priv->opmansync == NULL) {
		goto out_freelock;
	}

	priv->flags  = 0;
	priv->fb_ptr = NULL;

	GGIDPRINT("display-palemu: parent mode is '%s'\n",
		  options[OPT_PARENT].result);
	ggiParseMode(options[OPT_PARENT].result, &priv->mode);

	GGIDPRINT("display-palemu: opening target: %s\n", target);
	priv->parent = ggiOpen(target, NULL);
	if (priv->parent == NULL) {
		fprintf(stderr,
			"display-palemu: Failed to open target: '%s'\n",
			target);
		err = GGI_ENODEVICE;
		goto out_freeopmansync;
	}

	ggiSetFlags(priv->parent, GGIFLAG_ASYNC);

	/* Setup mansync */
	err = _ggiAddDL(vis, "helper-mansync", NULL, priv->opmansync, 0);
	if (err) {
		fprintf(stderr,
			"display-palemu: Cannot load helper-mansync!\n");
		GGIclose(vis, dlh);
		return err;
	}

	MANSYNC_init(vis);
	if (!(LIBGGI_FLAGS(vis) & GGIFLAG_ASYNC)) {
		MANSYNC_start(vis);
	}

	/* add giiInputs, if we have them */
	if (priv->parent->input) {
		vis->input = giiJoinInputs(vis->input, priv->parent->input);
		priv->parent->input = NULL; /* destroy old reference */
	}

	/* Has mode management */
	vis->opdisplay->getmode = GGI_palemu_getmode;
	vis->opdisplay->setmode = GGI_palemu_setmode;
	vis->opdisplay->checkmode = GGI_palemu_checkmode;
	vis->opdisplay->getapi = GGI_palemu_getapi;
	vis->opdisplay->flush = GGI_palemu_flush;
	vis->opdisplay->setflags = GGI_palemu_setflags;

	GGIDPRINT("display-palemu: GGIopen succeeded.\n");

	*dlret = GGI_DL_OPDISPLAY;
	return 0;

  out_freelock:
	ggLockDestroy(priv->flush_lock);
  out_freeopmansync:
	free(priv->opmansync);
  out_freepriv:
	free(priv);
  out_freegc:
	free(LIBGGI_GC(vis));

	return err;
}


static int GGIexit(ggi_visual *vis, struct ggi_dlhandle *dlh)
{
	if (PALEMU_PRIV(vis) && PALEMU_PRIV(vis)->opmansync) {
		if (!(LIBGGI_FLAGS(vis) & GGIFLAG_ASYNC)) {
			MANSYNC_stop(vis);
		}
		MANSYNC_deinit(vis);
	}

	return 0;
}


EXPORTFUNC
int GGIdl_palemu(int func, void **funcptr);

int GGIdl_palemu(int func, void **funcptr)
{
	switch (func) {
	case GGIFUNC_open:
		*funcptr = GGIopen;
		return 0;
	case GGIFUNC_exit:
		*funcptr = GGIexit;
		return 0;
	case GGIFUNC_close:
		*funcptr = GGIclose;
		return 0;
	default:
		*funcptr = NULL;
	}

	return GGI_ENOTFOUND;
}

#include <ggi/internal/ggidlinit.h>
