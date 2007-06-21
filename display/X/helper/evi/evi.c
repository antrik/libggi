/* $Id: evi.c,v 1.13 2007/06/21 08:00:53 cegger Exp $
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
#include <ggi/internal/ggi_debug.h>
#include <ggi/display/xevi.h>
#include <ggi/internal/ggi-module.h>

static int GGIopen(struct ggi_visual *vis, struct ggi_dlhandle *dlh,
		   const char *args, void *argptr, uint32_t *dlret)
{
	ggi_x_priv *xpriv = GGIX_PRIV(vis);
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

	DPRINT("Xevi found %i visuals:\n", priv->nevi);
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
					DPRINT("Visual %x is an overlay/"
						  "underlay, disabled.\n", id);
					(xpriv->vilist + j)->flags |=
						GGI_X_VI_NON_FB;
					j++;
				}
			}
		}

#warning decide what to do about conflicts here

		DPRINT("ID: %x screen: %i level: %i transp:"
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

static int GGIclose(struct ggi_visual *vis, struct ggi_dlhandle *dlh)
{
	ggi_x_priv *xpriv = GGIX_PRIV(vis);
	ggi_xevi_priv *priv = xpriv->evilist;

	if (priv != NULL) {
		if (priv->evi != NULL) XFree (priv->evi);
		free(priv);
	}
	return GGI_OK;
}


static int
GGI_helper_x_evi_setup(struct ggi_helper *helper,
			const char *args, void *argptr)
{
	ggi_x_priv *xpriv = GGIX_PRIV(helper->visual);
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

	DPRINT("Xevi found %i visuals:\n", priv->nevi);
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
					DPRINT("Visual %x is an overlay/"
						  "underlay, disabled.\n", id);
					(xpriv->vilist + j)->flags |=
						GGI_X_VI_NON_FB;
					j++;
				}
			}
		}

#warning decide what to do about conflicts here

		DPRINT("ID: %x screen: %i level: %i transp:"
			  "%x/%x cmaps: %i/%i (%i conflicts.)\n",
			  ptr->core_visual_id, ptr->screen, ptr->level,
			  ptr->transparency_type,
			  ptr->transparency_value,
			  ptr->min_hw_colormaps, ptr->max_hw_colormaps,
			  ptr->num_colormap_conflicts
			  );
		i++;
	}

	return GGI_OK;

 nofunc_freepriv:
	free(priv);
 nofunc:
	return GGI_ENOFUNC;
}

static void
GGI_helper_x_evi_teardown(struct ggi_helper *helper)
{
	ggi_x_priv *xpriv = GGIX_PRIV(helper->visual);
	ggi_xevi_priv *priv = xpriv->evilist;

	if (priv != NULL) {
		if (priv->evi != NULL)
			XFree (priv->evi);
		free(priv);
	}
}

struct ggi_module_helper GGI_helper_x_evi = {
	GG_MODULE_INIT("helper-x-evi", 0, 1, GGI_MODULE_HELPER),
	GGI_helper_x_evi_setup,
	GGI_helper_x_evi_teardown
};

static struct ggi_module_helper *_GGIdl_helper_x_evi[] = {
	&GGI_helper_x_evi,
	NULL
};

EXPORTFUNC
int GGIdl_helper_x_evi(int func, void **funcptr);

int GGIdl_helper_x_evi(int func, void **funcptr)
{
	ggifunc_open **openptr;
	ggifunc_close **closeptr;
	struct ggi_module_helper ***modulesptr;

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
	case GG_DLENTRY_MODULES:
		modulesptr = (struct ggi_module_helper ***)funcptr;
		*modulesptr = _GGIdl_helper_x_evi;
		return GGI_OK;
	default:
		*funcptr = NULL;
	}

	return GGI_ENOTFOUND;
}
