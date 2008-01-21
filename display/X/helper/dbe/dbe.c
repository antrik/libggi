/* $Id: dbe.c,v 1.18 2008/01/21 23:08:41 cegger Exp $
******************************************************************************

   DBE extension support for display-x

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
#include <ggi/display/x.h>
#include <X11/extensions/Xdbe.h>
#include <ggi/internal/ggi-module.h>

#if 0
static void GGI_DBE_swap(struct ggi_visual *vis)
{
	ggi_x_priv   *priv;
	XdbeSwapInfo swapInfo;

	priv = GGIX_PRIV(vis);

	/* Set swapping informations */
	swapInfo.swap_window = priv->win;
	swapInfo.swap_action = XdbeUndefined;
	
	XdbeSwapBuffers(priv->disp, &swapInfo, 1); /* Swap buffer */
}

static int GGI_DBE_create_window_drawable (struct ggi_visual *vis)
{
	ggi_x_priv *priv = GGIX_PRIV(vis);
	XdbeBackBuffer  backBuffer;

	if(priv->win == None)
		return GGI_ENODEVICE;

	priv->drawable = priv->win;

	/* Allocate back buffer */
	backBuffer = XdbeAllocateBackBufferName(priv->disp, 
			priv->win, XdbeUntouched);

	if (backBuffer != None) {
		priv->drawable = backBuffer;
		
	} else { 
		DPRINT_LIBS("X: DOUBLE-BUFFER: Back buffer allocation failed\n");
		return GGI_EFATAL;
	}
      
	return 0;
}
#endif /* 0 */

static int
GGI_helper_x_dbe_setup(struct ggi_helper *helper,
			const char *args, void *argptr)
{
	ggi_x_priv *priv = GGIX_PRIV(helper->visual);
	int major_version, minor_version;
	Status rc;

	/* Check if DBE is present before initialising */
	rc = XdbeQueryExtension(priv->disp, &major_version, &minor_version);
	if (rc != True) return GGI_ENOFUNC;

	DPRINT_LIBS("X: DOUBLE-BUFFER: DBE version %i.%i\n",
			major_version, minor_version);

	return GGI_OK;
}

static void
GGI_helper_x_dbe_teardown(struct ggi_helper *helper)
{
#if 0
	ggi_x_priv *priv = GGIX_PRIV(helper->visual);
	XdbeBackBuffer backBuffer;

	/* Deallocate back buffer */
	if(!XdbeDeallocateBackBufferName(priv->disp, priv->drawable)) {
		DPRINT_LIBS("X: DOUBLE-BUFFER: Unable to deallocate back buffer.\n");
		return GGI_EFATAL;
	}
#endif
}

struct ggi_module_helper GGI_helper_x_dbe = {
	GG_MODULE_INIT("helper-x-dbe", 0, 1, GGI_MODULE_HELPER),
	GGI_helper_x_dbe_setup,
	GGI_helper_x_dbe_teardown
};

static struct ggi_module_helper *_GGIdl_helper_x_dbe[] = {
	&GGI_helper_x_dbe,
	NULL
};

EXPORTFUNC
int GGIdl_helper_x_dbe(int func, void **funcptr);

int GGIdl_helper_x_dbe(int func, void **funcptr)
{
	struct ggi_module_helper ***modulesptr;

	switch (func) {
	case GG_DLENTRY_MODULES:
		modulesptr = (struct ggi_module_helper ***)funcptr;
		*modulesptr = _GGIdl_helper_x_dbe;
		return GGI_OK;
	default:
		*funcptr = NULL;
	}

	return GGI_ENOTFOUND;
}
