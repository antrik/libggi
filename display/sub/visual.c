/* $Id: visual.c,v 1.3 2004/02/23 14:25:16 pekberg Exp $
******************************************************************************

   Display-sub

   Copyright (C) 1998 Andreas Beck	[becka@ggi-project.org]
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

#include <ggi/display/sub.h>

static int GGIopen(ggi_visual *vis, struct ggi_dlhandle *dlh,
		   const char *args, void *argptr, uint32 *dlret)
{
	ggi_sub_priv *priv;

	if (! argptr) {
		fprintf(stderr, "display-sub needs pointer to real visual as argument.\n");
		return -1;
	}

	priv = malloc(sizeof(ggi_sub_priv));
	if (!priv) return GGI_ENOMEM;

	LIBGGI_GC(vis) = malloc(sizeof(ggi_gc));
	if (!LIBGGI_GC(vis)) {
		free(priv);
		return GGI_ENOMEM;
	}

	priv->parent = argptr;

	priv->position.x = priv->position.y = 0;
	priv->botright.x = priv->botright.y = 0;

	LIBGGI_PRIVATE(vis) = priv;

	/* Has mode management */
	vis->opdisplay->getmode=GGI_sub_getmode;
	vis->opdisplay->setmode=GGI_sub_setmode;
	vis->opdisplay->checkmode=GGI_sub_checkmode;
	vis->opdisplay->flush=GGI_sub_flush;
	vis->opdisplay->getapi=GGI_sub_getapi;
	vis->opdisplay->setflags=GGI_sub_setflags;

	vis->opdraw->fillscreen=GGI_sub_fillscreen;

	vis->opdraw->putc=GGI_sub_putc;
	vis->opdraw->puts=GGI_sub_puts;

	vis->opdraw->drawpixel_nc=GGI_sub_drawpixel;
	vis->opdraw->drawpixel=GGI_sub_drawpixel;
	vis->opdraw->putpixel_nc=GGI_sub_putpixel;
	vis->opdraw->putpixel=GGI_sub_putpixel;
	vis->opdraw->getpixel=GGI_sub_getpixel;

	vis->opdraw->drawhline=GGI_sub_drawhline;
	vis->opdraw->puthline=GGI_sub_puthline;
	vis->opdraw->gethline=GGI_sub_gethline;

	vis->opdraw->drawvline=GGI_sub_drawvline;
	vis->opdraw->putvline=GGI_sub_putvline;
	vis->opdraw->getvline=GGI_sub_getvline;

	vis->opdraw->drawbox=GGI_sub_drawbox;
	vis->opdraw->putbox=GGI_sub_putbox;
	vis->opdraw->getbox=GGI_sub_getbox;

	vis->opdraw->drawline=GGI_sub_drawline;
	vis->opdraw->copybox=GGI_sub_copybox;

	vis->opdraw->crossblit=GGI_sub_crossblit;

	vis->opcolor->getgamma=GGI_sub_getgamma;
	vis->opcolor->setgamma=GGI_sub_setgamma;
	vis->opcolor->getgammamap=GGI_sub_getgammamap;
	vis->opcolor->setgammamap=GGI_sub_setgammamap;

	vis->opcolor->mapcolor=GGI_sub_mapcolor;
	vis->opcolor->unmappixel=GGI_sub_unmappixel;
	vis->opcolor->setpalvec=GGI_sub_setpalvec;
	vis->opcolor->getpalvec=GGI_sub_getpalvec;

	*dlret = GGI_DL_OPDISPLAY;
	return 0;
}

static int GGIclose(ggi_visual *vis, struct ggi_dlhandle *dlh)
{
	free(LIBGGI_GC(vis));
	free(LIBGGI_PRIVATE(vis));

	return 0;
}


EXPORTFUNC
int GGIdl_sub(int func, void **funcptr);

int GGIdl_sub(int func, void **funcptr)
{
	switch (func) {
	case GGIFUNC_open:
		*funcptr = GGIopen;
		return 0;
	case GGIFUNC_exit:
		*funcptr = NULL;
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
