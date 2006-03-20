/* $Id: visual.c,v 1.15 2006/03/20 20:06:32 cegger Exp $
******************************************************************************

   Display-file: initialization

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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "config.h"
#include <ggi/display/file.h>
#include <ggi/internal/ggi_debug.h>

static const gg_option optlist[] =
{
	{ "flushcmd", "" },
	{ "flushframe",  "0" },
	{ "flushtime",  "0.0" }
};

#define OPT_FLUSHCMD	0
#define OPT_FLUSHFRAME	1
#define OPT_FLUSHTIME	2

#define NUM_OPTS	(sizeof(optlist)/sizeof(gg_option))


static int GGIopen(struct ggi_visual *vis, struct ggi_dlhandle *dlh,
		   const char *args, void *argptr, uint32_t *dlret)
{
	ggi_file_priv *priv;
	gg_option options[NUM_OPTS];
	double fltime;
	int err = GGI_ENOMEM;

	DPRINT_MISC("coming up (filename='%s').\n", args);

	if (!args || !args[0]) {
		fprintf(stderr, "display-file: Missing filename.\n");
		return GGI_EARGREQ;
	}

	memcpy(options, optlist, sizeof(options));
	args = ggParseOptions(args, options, NUM_OPTS);
	if (args == NULL) {
		fprintf(stderr, "display-file: error in arguments.\n");
		return GGI_EARGINVAL;
	}

	priv = LIBGGI_PRIVATE(vis) = malloc(sizeof(ggi_file_priv));
	if (priv == NULL) return GGI_ENOMEM;

	LIBGGI_GC(vis) = malloc(sizeof(ggi_gc));
	if (LIBGGI_GC(vis) == NULL) {
		goto out_freepriv;
	}

	priv->flags = 0;
	priv->writer = (file_writer_func *) NULL;
	priv->fb_ptr = priv->file_mmap = NULL;

	/* Handle arguments */
	if (getenv("GGI_FILE_OPTIONS") != NULL) {
		if (ggParseOptions(getenv("GGI_FILE_OPTIONS"), options,
				   NUM_OPTS) == NULL) {
			fprintf(stderr,
				"display-file: error in $GGI_FILE_OPTIONS.\n");
			err = GGI_EARGINVAL;
			goto out_freegc;
		}
	}

	priv->filename   = strdup(args);
	priv->flushcmd   = options[OPT_FLUSHCMD].result[0]
		? strdup(options[OPT_FLUSHCMD].result) : NULL;
	priv->flushevery = atoi(options[OPT_FLUSHFRAME].result);
	fltime         = atof(options[OPT_FLUSHTIME].result);
	priv->flushcnt   = 0;
	priv->flushtotal = 0;
	gettimeofday(&priv->flushlast,NULL);
	priv->flushstep.tv_sec  = fltime;
	priv->flushstep.tv_usec = (fltime-priv->flushstep.tv_sec)*1000000;

	if (_ggi_file_ppm_detect(priv->filename)) {
		priv->writer = &_ggi_file_ppm_write;
	} else {
		priv->flags |= FILEFLAG_RAW;
	}

	vis->opdisplay->getmode=GGI_file_getmode;
	vis->opdisplay->setmode=GGI_file_setmode;
	vis->opdisplay->getapi=GGI_file_getapi;
	vis->opdisplay->checkmode=GGI_file_checkmode;
	vis->opdisplay->setflags=GGI_file_setflags;

	*dlret = GGI_DL_OPDISPLAY;
	return 0;

  out_freegc:
	free(LIBGGI_GC(vis));
  out_freepriv:
	free(priv);

	return err;
}

/* Do the work part of the cleanup in exit instead of in close
 * as the default helper libs are cleaning up in close, but are
 * earlier in the chain, i.e. they are still accessible when
 * display-file is in exit, not so when it is in close. This is
 * unclean and will break when/if some default lib cleans up in
 * exit, but it is a workaround to get display-file to write
 * the last dump before the visual closes.
 */
static int GGIexit(struct ggi_visual *vis, struct ggi_dlhandle *dlh)
{
	ggi_file_priv *priv = FILE_PRIV(vis);

	DPRINT_MISC("going down.\n");

	if (priv->fb_ptr != NULL) {
		GGI_file_resetmode(vis);
	}

	return 0;
}


static int GGIclose(struct ggi_visual *vis, struct ggi_dlhandle *dlh)
{
	ggi_file_priv *priv = FILE_PRIV(vis);

	free(priv->filename);
	free(priv->flushcmd);

	free(priv);
	free(LIBGGI_GC(vis));

	return 0;
}


EXPORTFUNC
int GGIdl_file(int func, void **funcptr);

int GGIdl_file(int func, void **funcptr)
{
	ggifunc_open **openptr;
	ggifunc_exit **exitptr;
	ggifunc_close **closeptr;

	switch (func) {
	case GGIFUNC_open:
		openptr = (ggifunc_open **)funcptr;
		*openptr = GGIopen;
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
