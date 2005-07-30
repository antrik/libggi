/* $Id: visual.c,v 1.7 2005/07/30 10:58:24 cegger Exp $
******************************************************************************

   Auto target for GGI.

   Copyright (C) 2004 Christoph Egger

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
#include <ggi/internal/ggi_debug.h>
#include <ggi/display/auto.h>


static int GGIopen(ggi_visual *vis, struct ggi_dlhandle *dlh,
		   const char *args, void *argptr, uint32_t *dlret)
{
	ggi_auto_priv *priv;
	ggi_visual **_vis;

	DPRINT_LIBS("display-auto: Starting\n");

	LIB_ASSERT(argptr != NULL, "Detected invalid pointer");

	_vis = (ggi_visual **)argptr;

	priv = calloc((size_t)(1), sizeof(ggi_auto_priv));
	if (priv == NULL) return GGI_ENOMEM;

	DPRINT("display-auto: Find optimal target.\n");
	*_vis = _GGI_auto_findOptimalTarget(priv);
	DPRINT("display-auto: Found optimal target: %p\n", *_vis);

	free(priv);

	*dlret = GGI_DL_OPDISPLAY;
	return 0;
}


static int GGIclose(ggi_visual *vis, struct ggi_dlhandle *dlh)
{
	return 0;
}


static int GGIexit(ggi_visual *vis, struct ggi_dlhandle *dlh)
{
	return 0;
}


EXPORTFUNC
int GGIdl_auto(int func, void **funcptr);

int GGIdl_auto(int func, void **funcptr)
{
	switch (func) {
	case GGIFUNC_open:
		*funcptr = (void *)GGIopen;
		return 0;
	case GGIFUNC_exit:
		*funcptr = (void *)GGIexit;
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
