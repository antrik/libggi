/* $Id: evi.c,v 1.2 2002/09/08 21:37:44 soyt Exp $
******************************************************************************

   Extended Visual Information extension support for display-x

   Copyright (C) 2002      Brian S. Julin	[bri@tull.umassp.edu]

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
#include <ggi/internal/ggi-dl.h>
#include <ggi/display/xevi.h>

static int GGIopen(ggi_visual *vis, struct ggi_dlhandle *dlh,
		   const char *args, void *argptr, uint32 *dlret)
{
	ggi_x_priv *xpriv = LIBGGI_PRIVATE(vis);
	ggi_xevi_priv *priv;
	int i;

	if (XeviQueryExtension(xpriv->disp) != True) goto nofunc;

	priv = calloc(sizeof(*priv), 1);
	if (priv == NULL) return GGI_ENOMEM;

	if (XeviQueryVersion(xpriv->disp, &priv->major, &priv->minor) 
	    != True) goto nofunc_freepriv;

	if (XeviGetVisualInfo(xpriv->disp, NULL, 0, &priv->evi, &priv->nevi)
	    != Success) goto nofunc_freepriv;

	xpriv->evilist = priv;

	GGIDPRINT("Xevi found %i visuals:\n", priv->nevi);
	i = 0;
	while (i < priv->nevi) {
		ExtendedVisualInfo *ptr = priv->evi + i;

		if (ptr->level != 0) {

			/* tell display-x not to use overlays/underlays */
			int j = 0;
			while (j < xpriv->nvisuals) {
				VisualID id;

				id = (xpriv->vilist + j)->vi->visualid;
				if (id  == ptr->core_visual_id) {
					GGIDPRINT("Visual %x is an overlay/"
						  "underlay, disabled.\n", id);
					(xpriv->vilist + j)->flags |=
						GGI_X_VI_NON_FB;
					j++;
				}
			}
		}

#warning decide what to do about conflicts here

		GGIDPRINT("ID: %x screen: %i level: %i transp:"
			  "%x/%x cmaps: %i/%i (%i conflicts.)\n",
			  ptr->core_visual_id, ptr->screen, ptr->level,
			  ptr->transparency_type,
			  ptr->transparency_value,
			  ptr->min_hw_colormaps, ptr->max_hw_colormaps,
			  ptr->num_colormap_conflicts
			  );
		i++;
	}

	*dlret = 0;
	return GGI_OK;

 nofunc_freepriv:
	free(priv);
 nofunc:
	return GGI_ENOFUNC;
}

static int GGIclose(ggi_visual *vis, struct ggi_dlhandle *dlh)
{
	ggi_x_priv *xpriv = LIBGGI_PRIVATE(vis);
	ggi_xevi_priv *priv = xpriv->evilist;

	if (priv != NULL) {
		if (priv->evi != NULL) XFree (priv->evi);
		free(priv);
	}
	return GGI_OK;
}

int GGIdl_helper_x_evi(int func, void **funcptr)
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
