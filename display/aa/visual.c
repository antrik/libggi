/* $Id: visual.c,v 1.3 2003/07/06 10:25:21 cegger Exp $
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

#include <stdlib.h>

#include "config.h"
#include <ggi/internal/ggi-dl.h>
#include <ggi/display/aa.h>


void _GGI_aa_freedbs(ggi_visual *vis) {
	int i;

	for (i=LIBGGI_APPLIST(vis)->num-1; i >= 0; i--) {
		_ggi_db_free(LIBGGI_APPBUFS(vis)[i]);
		_ggi_db_del_buffer(LIBGGI_APPLIST(vis), i);
	}
}

static int GGIclose(ggi_visual *vis, struct ggi_dlhandle *dlh)
{
	ggi_aa_priv *priv;

	_GGI_aa_freedbs(vis);

	if ((priv = LIBGGI_PRIVATE(vis)) != NULL) {
		if (priv->context) {
			aa_uninitmouse(priv->context);
			aa_uninitkbd(priv->context);
			aa_close(priv->context);
		}
		free(priv->opmansync);		
		ggLockDestroy(priv->aalock);
		free(LIBGGI_PRIVATE(vis));
	}

	free(LIBGGI_GC(vis));

	return 0;
}

static int GGIopen(ggi_visual *vis, struct ggi_dlhandle *dlh,
		   const char *args, void *argptr, uint32 *dlret)
{
	ggi_aa_priv *priv;
	void *lock;
	int err = GGI_ENOMEM;
	gg_option optlist[] = {
		{ "fastrender", "" }};
	
	GGIDPRINT_LIBS("display-aa: Starting\n");
	
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
	priv->context = 0;
	priv->lx = 0;
	priv->ly = 0;
	priv->lb = 0;
	priv->lastkey = 0;
	priv->lastkeyticks = 0;
	priv->haverelease = 0;

	if (args) {
		args = ggParseOptions((char *)args, optlist,
					sizeof(optlist)/sizeof(gg_option));
		if(!args) {
			fprintf(stderr, "display-aa: error in arguments\n");
		}

		priv->fastrender = (*args && optlist[0].result[0]=='y');
	}

	err = _ggiAddDL(vis, "helper-mansync", NULL, priv->opmansync, 0);
	if (err) {
		fprintf(stderr, 
			"display-aa: Cannot load required helper-mansync!\n");
		goto out_freelock;
	}

	LIBGGI_PRIVATE(vis) = priv;

	MANSYNC_init(vis);

	{
		gii_input *inp;
		GGIDPRINT_MISC("display-aa: gii starting\n");

		/* First allocate a new gii_input descriptor. */
		inp = _giiInputAlloc();
		if (inp == NULL) {
			GGIDPRINT_MISC("display-aa: _giiInputAlloc failed\n");
			GGIclose(vis, dlh);
			return GGI_ENOMEM;
		}
		GGIDPRINT_MISC("display-aa: gii inp=%p\n",inp);

		/* Now fill in the blanks. */
		inp->priv = priv; /* We need that in GII_aa_poll() */

		inp->maxfd = 0;
		inp->flags = GII_FLAGS_HASPOLLED;

		/* What events _can_  we create at all ?
		   Save useless polling time. */
		inp->curreventmask = inp->targetcan = emKey |
			emPtrButtonPress | emPtrButtonRelease |	emPtrAbsolute ;

		/* We only need the "poll" function. For all others,
		   defaults are fine. */
		inp->GIIeventpoll = GII_aa_poll;

		/* Now join the new event source in. */
		vis->input = giiJoinInputs(vis->input,inp);
		
		GGIDPRINT_MISC("display-aa: input joined into %p\n",
			       vis->input);
	}

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

static int GGIexit(ggi_visual *vis, struct ggi_dlhandle *dlh)
{
	MANSYNC_deinit(vis);

	return 0;
}


int GGIdl_aa(int func, void **funcptr);

int GGIdl_aa(int func, void **funcptr)
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
