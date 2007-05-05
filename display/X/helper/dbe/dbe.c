/* $Id: dbe.c,v 1.15 2007/05/05 18:59:47 mooz Exp $
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
			priv->win, XdbeUndefined);

	if (backBuffer != None) {
		priv->drawable = backBuffer;
		
	} else { 
		DPRINT_LIBS("X: DOUBLE-BUFFER: Back buffer allocation failed\n");
		return GGI_EFATAL;
	}
  
	priv->swapdrawable = GGI_DBE_swap;
	    
	return 0;
}

static int GGIopen(struct ggi_visual *vis, struct ggi_dlhandle *dlh,
		   const char *args, void *argptr, uint32_t *dlret)
{
	ggi_x_priv *priv = GGIX_PRIV(vis);
	int major_version, minor_version;
	Status rc;
	
	/* Check if DBE is present before initialising */
	rc = XdbeQueryExtension(priv->disp, &major_version, &minor_version);
	if (rc != True) return GGI_ENOFUNC;

	DPRINT_LIBS("X: DOUBLE-BUFFER: DBE version %i.%i\n",
			major_version, minor_version);

	priv->createdrawable = GGI_DBE_create_window_drawable;

	*dlret = 0;
	return GGI_OK;
}

static int GGIclose(struct ggi_visual *vis, struct ggi_dlhandle *dlh)
{
	XdbeBackBuffer  backBuffer;	/* Back buffer */
	ggi_x_priv *priv = GGIX_PRIV(vis);
	/* Deallocate back buffer */
	if(!XdbeDeallocateBackBufferName(priv->disp, priv->drawable)) {
		DPRINT_LIBS("X: DOUBLE-BUFFER: Unable to deallocate back buffer.\n");
		return GGI_EFATAL;
	}
	return GGI_OK;
}


EXPORTFUNC
int GGIdl_helper_x_dbe(int func, void **funcptr);

int GGIdl_helper_x_dbe(int func, void **funcptr)
{
	ggifunc_open **openptr;
	ggifunc_close **closeptr;

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
	default:
		*funcptr = NULL;
	}

	return GGI_ENOTFOUND;
}
