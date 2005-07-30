/* $Id: visual.c,v 1.14 2005/07/30 10:58:26 cegger Exp $
******************************************************************************

   Display-multi: initialization

   Copyright (C) 1995 Andreas Beck		[becka@ggi-project.org]
   Copyright (C) 1997 Jason McMullan		[jmcc@ggi-project.org]
   Copyright (C) 1998 Andrew Apted		[andrew@ggi-project.org]
   Copyright (C) 1999-2000 Marcus SUndberg	[marcus@ggi-project.org]

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
#include <ggi/display/multi.h>
#include <ggi/internal/ggi_debug.h>


static int GGIopen(ggi_visual *vis, struct ggi_dlhandle *dlh,
		   const char *args, void *argptr, uint32_t *dlret)
{
	ggi_multi_priv *priv;
	MultiVis *cur;
	char target[1024];
	int err = GGI_ENOMEM;

	if (!args || *args == '\0') {
		fprintf(stderr, "display-multi: missing target names.\n");
		return GGI_EARGREQ;
	}

	priv = calloc(1, sizeof(ggi_multi_priv));
	if (priv == NULL) return GGI_ENOMEM;
	LIBGGI_PRIVATE(vis) = priv;

	LIBGGI_GC(vis) = malloc(sizeof(ggi_gc));
	if (LIBGGI_GC(vis) == NULL) {
		goto out_freepriv;
	}


	priv->vis_num  = 0;
	GG_SLIST_INIT(&priv->vis_list);

	err = GGI_EARGINVAL;
	for (;;) {
		args = ggParseTarget(args, target, 1024);

		if (args == NULL) {
			goto out_freeall;
		}

		if (*target == 0) {
			strcpy(target, "auto");
		}
		
		cur = malloc(sizeof(MultiVis));
		if (cur == NULL) continue;

		DPRINT("display-multi: opening sub #%d: %s\n",
			priv->vis_num+1, target);

		cur->vis = ggiOpen(target, NULL);
		if (cur->vis == NULL) {
			fprintf(stderr, "display-multi: failed trying "
				"to open: %s\n", target);
			free(cur);
			continue;
		}

		/* add to head */
		GG_SLIST_INSERT_HEAD(&priv->vis_list, cur, visuals);
		priv->vis_num++;

		/* Add giiInputs, if we have them. */
		if (cur->vis->input) {
			vis->input = giiJoinInputs(vis->input,cur->vis->input);
			cur->vis->input = vis->input; /* This _must_ be set to
							 NULL before closing
							 the subvisual */
		}

		while (*args && isspace((uint8_t)*args)) args++;

		if (*args == 0) {
			break;
		}

		if (*args != ':') {
			fprintf(stderr, "display-multi: expecting ':' "
				"between targets.\n");
			goto out_freeall;
		}

		args++;  /* skip ':' */
	}
	
	/* Has mode management */
	vis->opdisplay->getmode		= GGI_multi_getmode;
	vis->opdisplay->setmode		= GGI_multi_setmode;
	vis->opdisplay->checkmode	= GGI_multi_checkmode;
	vis->opdisplay->flush		= GGI_multi_flush;
	vis->opdisplay->setflags	= GGI_multi_setflags;

	vis->opgc->gcchanged		= GGI_multi_gcchanged;

	vis->opdraw->fillscreen		= GGI_multi_fillscreen;
	vis->opdraw->setorigin		= GGI_multi_setorigin;

	vis->opdraw->putc		= GGI_multi_putc;
	vis->opdraw->puts		= GGI_multi_puts;
	vis->opdraw->getcharsize	= GGI_multi_getcharsize;

	vis->opdraw->drawpixel_nc	= GGI_multi_drawpixel;
	vis->opdraw->drawpixel		= GGI_multi_drawpixel;
	vis->opdraw->putpixel_nc	= GGI_multi_putpixel;
	vis->opdraw->putpixel		= GGI_multi_putpixel;
	vis->opdraw->getpixel		= GGI_multi_getpixel;

	vis->opdraw->drawline		= GGI_multi_drawline;
	vis->opdraw->drawhline		= GGI_multi_drawhline;
	vis->opdraw->puthline		= GGI_multi_puthline;
	vis->opdraw->gethline		= GGI_multi_gethline;

	vis->opdraw->drawvline		= GGI_multi_drawvline;
	vis->opdraw->putvline		= GGI_multi_putvline;
	vis->opdraw->getvline		= GGI_multi_getvline;

	vis->opdraw->drawbox		= GGI_multi_drawbox;
	vis->opdraw->putbox		= GGI_multi_putbox;
	vis->opdraw->getbox		= GGI_multi_getbox;
	vis->opdraw->copybox		= GGI_multi_copybox;
	vis->opdraw->crossblit		= GGI_multi_crossblit;

	vis->opcolor->getgamma		= GGI_multi_getgamma;
	vis->opcolor->setgamma		= GGI_multi_setgamma;
	vis->opcolor->getgammamap	= GGI_multi_getgammamap;
	vis->opcolor->setgammamap	= GGI_multi_setgammamap;

	vis->opcolor->mapcolor		= GGI_multi_mapcolor;
	vis->opcolor->unmappixel	= GGI_multi_unmappixel;
	vis->opcolor->packcolors	= GGI_multi_packcolors;
	vis->opcolor->unpackpixels	= GGI_multi_unpackpixels;

	vis->opcolor->setpalvec		= GGI_multi_setpalvec;
	vis->opcolor->getpalvec		= GGI_multi_getpalvec;

	*dlret = GGI_DL_OPDISPLAY | GGI_DL_OPCOLOR
		| GGI_DL_OPDRAW | GGI_DL_OPGC;
	return 0;

  out_freeall:
	while (!GG_SLIST_EMPTY(&priv->vis_list)) {
		MultiVis *temp = GG_SLIST_FIRST(&priv->vis_list);
		GG_SLIST_REMOVE_HEAD(&priv->vis_list, visuals);
		free(temp);
	}

  out_freepriv:
	free(priv);

	return err;
}

/* Destroy visuals in inverse order. This is a bit hard on singly linked lists.
 * So we just use recursion to traverse to the end of the list and then destroy
 * the visuals while backtracking.
 */
static void destroyvisuals(MultiVis *cur)
{
	if (cur) destroyvisuals(GG_SLIST_NEXT(cur, visuals));
	else return;
	cur->vis->input = NULL;
	ggiClose(cur->vis);
	free(cur);
}

static int GGIclose(ggi_visual *vis, struct ggi_dlhandle *dlh)
{
	ggi_multi_priv *mm = GGIMULTI_PRIV(vis);

	giiClose(vis->input);
	destroyvisuals(GG_SLIST_FIRST(&mm->vis_list));
	mm->vis_num=0;

	free(mm);
	free(LIBGGI_GC(vis));

	return 0;
}


EXPORTFUNC
int GGIdl_multi(int func, void **funcptr);

int GGIdl_multi(int func, void **funcptr)
{
	switch (func) {
	case GGIFUNC_open:
		*funcptr = (void *)GGIopen;
		return 0;
	case GGIFUNC_exit:
		*funcptr = NULL;
		return 0;
	case GGIFUNC_close:
		*funcptr = (void *)GGIclose;
		return 0;
	default:
		*funcptr = NULL;
	}

	return GGI_ENOTFOUND;
}

#include <ggi/internal/ggidlinit.h>
