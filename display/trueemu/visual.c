/* $Id: visual.c,v 1.2 2002/09/08 21:37:47 soyt Exp $
******************************************************************************

   Display-trueemu: initialization

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
#include <ggi/display/trueemu.h>


static const gg_option optlist[] =
{
	{ "parent", ""    },
	{ "dither", "4"   },
	{ "model",  "rgb" }
};

#define OPT_PARENT	0
#define OPT_DITHER	1
#define OPT_MODEL	2

#define NUM_OPTS	(sizeof(optlist)/sizeof(gg_option))


extern int GGI_trueemu_resetmode(ggi_visual *vis);


static int GGIclose(ggi_visual *vis, struct ggi_dlhandle *dlh)
{
	ggi_trueemu_priv *priv = TRUEEMU_PRIV(vis);
	
	GGIDPRINT("display-trueemu: GGIclose start.\n");

	if (priv->fb_ptr != NULL) {
		GGI_trueemu_resetmode(vis);
	}
	if (priv->parent != NULL) {
		ggiClose(priv->parent);
	}

	ggLockDestroy(priv->flush_lock);
	free(priv->opmansync);
	free(priv->mem_opdraw);
	free(priv);
	free(LIBGGI_GC(vis));

	GGIDPRINT("display-trueemu: GGIclose done.\n");

	return 0;
}


static int GGIopen(ggi_visual *vis, struct ggi_dlhandle *dlh,
		   const char *args, void *argptr, uint32 *dlret)
{
	ggi_trueemu_priv *priv;
	gg_option options[NUM_OPTS];
	char target[1024];
	int err = GGI_ENOMEM;

	GGIDPRINT("display-trueemu: GGIopen start.\n");

	/* handle arguments */
	memcpy(options, optlist, sizeof(options));
	if (args) {
		args = ggParseOptions(args, options, NUM_OPTS);

		if (args == NULL) {
			fprintf(stderr,
				"display-trueemu: error in arguments.\n");
			return GGI_EARGINVAL;
		}
	}
	if (getenv("GGI_TRUEEMU_OPTIONS") != NULL) {
		if (ggParseOptions(getenv("GGI_TRUEEMU_OPTIONS"), options,
				   NUM_OPTS) == NULL) {
			fprintf(stderr, "display-trueemu: error in $GGI_TRUEEMU_OPTIONS.\n");
			return GGI_EARGINVAL;
		}
	}

	/* Find out the parent target. */
	while (args && *args && isspace((int)*args)) {
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
	LIBGGI_PRIVATE(vis) = priv = malloc(sizeof(*priv));
	if (priv == NULL) {
		goto out_freegc;
	}
	priv->mem_opdraw = malloc(sizeof(struct ggi_visual_opdraw));
	if (priv->mem_opdraw == NULL) {
		goto out_freepriv;
	}
	priv->opmansync = malloc(sizeof(_ggi_opmansync));
	if (priv->opmansync == NULL) {
		goto out_freeopdraw;
	}
	priv->flush_lock = ggLockCreate();
	if (priv->flush_lock == NULL) {
		goto out_freeopmansync;
	}

	priv->flags  = 0;
	priv->src_buf = priv->dest_buf = NULL;
	priv->R = priv->G = priv->B = NULL;
	priv->T = NULL;

	priv->fb_ptr = NULL;

	/* parse the results */
	GGIDPRINT("trueemu: parent is '%s'.\n", options[OPT_PARENT].result);
	GGIDPRINT("trueemu: dither is '%s'.\n", options[OPT_DITHER].result);
	GGIDPRINT("trueemu: model  is '%s'.\n", options[OPT_MODEL].result);
	ggiParseMode(options[OPT_PARENT].result, &priv->mode);

	switch (options[OPT_DITHER].result[0]) {
		case '0': priv->flags |= TRUEEMU_F_DITHER_0; break;
		case '2': priv->flags |= TRUEEMU_F_DITHER_2; break;
		case '4': priv->flags |= TRUEEMU_F_DITHER_4; break;

		default:
			fprintf(stderr, "display-trueemu: Unknown dither "
				"'%s'.\n", options[OPT_DITHER].result);
	}

	switch (options[OPT_MODEL].result[0]) {
		case 'r': priv->flags |= TRUEEMU_F_RGB;    break;
		case 'c': priv->flags |= TRUEEMU_F_CUBE;   break;
		case 'p': priv->flags |= TRUEEMU_F_PASTEL; break;

		default:
			fprintf(stderr, "display-trueemu: Unknown model "
				"'%s'.\n", options[OPT_MODEL].result);
	}

	GGIDPRINT("display-trueemu: opening target: %s\n", target);
	priv->parent = ggiOpen(target, NULL);
	if (priv->parent == NULL) {
		fprintf(stderr,
			"display-trueemu: Failed to open target: '%s'\n",
			target);
		err = GGI_ENODEVICE;
		goto out_freelock;
	}

	ggiSetFlags(priv->parent, GGIFLAG_ASYNC);

	/* Setup mansync */
	err = _ggiAddDL(vis, "helper-mansync", NULL, priv->opmansync, 0);
	if (err) {
		fprintf(stderr,
			"display-trueemu: Cannot load helper-mansync!\n");
		GGIclose(vis, dlh);
		return err;
	}

	MANSYNC_init(vis);

	/* add giiInputs, if we have them */
	if (priv->parent->input) {
		vis->input = giiJoinInputs(vis->input, priv->parent->input);
		priv->parent->input = NULL; /* destroy old reference */
	}

	/* Mode management */
	vis->opdisplay->getmode = GGI_trueemu_getmode;
	vis->opdisplay->setmode = GGI_trueemu_setmode;
	vis->opdisplay->checkmode = GGI_trueemu_checkmode;
	vis->opdisplay->getapi = GGI_trueemu_getapi;
	vis->opdisplay->flush = GGI_trueemu_flush;
	vis->opdisplay->setflags = GGI_trueemu_setflags;
	
	GGIDPRINT("display-trueemu: GGIopen succeeded.\n");

	*dlret = GGI_DL_OPDISPLAY;
	return 0;

  out_freelock:
	ggLockDestroy(priv->flush_lock);
  out_freeopmansync:
	free(priv->opmansync);
  out_freeopdraw:
	free(priv->mem_opdraw);
  out_freepriv:
	free(priv);
  out_freegc:
	free(LIBGGI_GC(vis));

	return err;
}


static int GGIexit(ggi_visual *vis, struct ggi_dlhandle *dlh)
{
	MANSYNC_deinit(vis);

	return 0;
}


int GGIdl_trueemu(int func, void **funcptr)
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
