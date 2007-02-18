/* $Id: visual.c,v 1.21 2007/02/18 16:01:39 cegger Exp $
******************************************************************************

   AAlib target for GGI.

   Copyright (C) 1997 Andreas Beck	[becka@ggi-project.org]
   Copyright (C) 2000 Steve Cheng	[steve@ggi-project.org]
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

#include "config.h"
#include <stdlib.h>
#include <string.h>

#include <ggi/gii.h>
#include <ggi/gii-module.h>
#include <ggi/display/aa.h>
#include <ggi/internal/ggi_debug.h>


static gg_option optlist[] =
{
	{ "fastrender", "no" }
};


#define OPT_FASTRENDER	0

#define NUM_OPTS	(sizeof(optlist)/sizeof(gg_option))



void _GGI_aa_freedbs(struct ggi_visual *vis)
{
	int i;

	for (i=LIBGGI_APPLIST(vis)->num-1; i >= 0; i--) {
		_ggi_db_free(LIBGGI_APPBUFS(vis)[i]);
		_ggi_db_del_buffer(LIBGGI_APPLIST(vis), i);
	}
}

static int GGIclose(struct ggi_visual *vis, struct ggi_dlhandle *dlh)
{
	ggi_aa_priv *priv = AA_PRIV(vis);

	if (priv->inp) {
		ggCloseModule(priv->inp);
		priv->inp = NULL;
	}

	_GGI_aa_freedbs(vis);

	if (priv != NULL) {
		if (priv->context) {
			aa_uninitmouse(priv->context);
			aa_uninitkbd(priv->context);
			aa_close(priv->context);
		}
		free(priv->opmansync);		
		ggLockDestroy(priv->aalock);
		free(priv);
	}

	free(LIBGGI_GC(vis));

	return 0;
}

static int GGIopen(struct ggi_visual *vis, struct ggi_dlhandle *dlh,
		   const char *args, void *argptr, uint32_t *dlret)
{
	ggi_aa_priv *priv;
	void *lock;
	int err = GGI_ENOMEM;
	gg_option options[NUM_OPTS];
	
	DPRINT_LIBS("display-aa: Starting\n");

	memcpy(options, optlist, sizeof(options));
	
	/* Get options from environment variable AAOPTS */
	if (!aa_parseoptions(NULL, NULL, NULL, NULL)) {
		fprintf(stderr,
			"display-aa: warning: parsing AAOPTS failed\n");
	}

	priv = malloc(sizeof(ggi_aa_priv));
	if (priv == NULL) return GGI_ENOMEM;

	LIBGGI_GC(vis) = malloc(sizeof(ggi_gc));
	if (LIBGGI_GC(vis) == NULL) {
		goto out_freepriv;
	}

	priv->opmansync = malloc(sizeof(_ggi_opmansync));
	if (priv->opmansync == NULL) {
		goto out_freegc;
	}

	lock = ggLockCreate();
	if (lock == NULL) {
		goto out_freeopmansync;
	}

	priv->aalock = lock;
	priv->context = NULL;

	if (args) {
		args = ggParseOptions(args, options, NUM_OPTS);
		if (!args) {
			fprintf(stderr, "display-aa: error in arguments\n");
		}

		priv->fastrender = (*args && options[OPT_FASTRENDER].result[0] == 'y');
	}

	err = _ggiAddDL(vis, _ggiGetConfigHandle(),
			"helper-mansync", NULL, priv->opmansync, 0);
	if (err) {
		fprintf(stderr, 
			"display-aa: Cannot load required helper-mansync!\n");
		goto out_freelock;
	}

	LIBGGI_PRIVATE(vis) = priv;

	if (priv->opmansync) {
		MANSYNC_init(vis);
		if (!(LIBGGI_FLAGS(vis) & GGIFLAG_ASYNC)) {
			MANSYNC_start(vis);
		}
	}

	do {
		struct gg_api *gii;

		DPRINT_MISC("display-aa: gii starting\n");

		/* First allocate a new gii_input descriptor. */
		gii = ggGetAPIByName("gii");
		if (gii != NULL && STEM_HAS_API(vis->stem, gii)) {
			priv->inp = ggOpenModule(gii, vis->stem,
				"input-aa", NULL, NULL);
		}
		if (priv->inp == NULL) {
			DPRINT_MISC("display-aa: ggOpenModule failed\n");
			GGIclose(vis, dlh);
			return GGI_ENOMEM;
		}
		DPRINT_MISC("display-aa: gii inp=%p\n",priv->inp);

	} while(0);


	/* Has mode management */
	vis->opdisplay->flush=GGI_aa_flush;
	vis->opdisplay->getmode=GGI_aa_getmode;
	vis->opdisplay->setmode=GGI_aa_setmode;
	vis->opdisplay->checkmode=GGI_aa_checkmode;
	vis->opdisplay->getapi =GGI_aa_getapi;
	vis->opdisplay->setflags=GGI_aa_setflags;

	*dlret = GGI_DL_OPDISPLAY;
	return 0;

  out_freelock:
	ggLockDestroy(priv->aalock);
  out_freeopmansync:
	free(priv->opmansync);
  out_freegc:
	free(LIBGGI_GC(vis));
  out_freepriv:
	free(priv);

	return err;
}

static int GGIexit(struct ggi_visual *vis, struct ggi_dlhandle *dlh)
{
	ggi_aa_priv *priv = AA_PRIV(vis);

	if (priv->opmansync) {
		if (!(LIBGGI_FLAGS(vis) & GGIFLAG_ASYNC)) {
			MANSYNC_stop(vis);
		}
		MANSYNC_deinit(vis);
	}
	return 0;
}


EXPORTFUNC
int GGIdl_aa(int func, void **funcptr);

int GGIdl_aa(int func, void **funcptr)
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
