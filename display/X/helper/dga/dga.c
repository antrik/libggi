/* $Id: dga.c,v 1.7 2004/11/06 22:48:25 cegger Exp $
******************************************************************************

   XFree86-DGA extension support for display-x

   Copyright (C) 1997-1998 Steve Cheng		[steve@ggi-project.org]
   Copyright (C) 1999-2000 Marcus Sundberg	[marcus@ggi-project.org]
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
#include <ggi/display/x.h>
#include <ggi/internal/ggi_debug.h>
#include <X11/extensions/xf86dga.h>

#if 0
static int ggi_xdga_getmodelist(ggi_visual *vis) {
	ggi_x_priv *priv;
	XDGAMode *modes;
	int i;

	priv = GGIX_PRIV(vis);

	priv->modes_num = 0;
	(XDGAMode *)(priv->modes_priv) = modes = 
		XDGAQueryModes(priv->disp, priv->screen, &(priv->modes_num));
	if (priv->modes_priv == NULL) return GGI_ENODEVICE;
	if (priv->modes_num <= 0) return GGI_ENODEVICE;

        priv->modes = calloc(sizeof(ggi_modelistmode), priv->modes_num);
        if (priv->modes == NULL) {
		XFree(priv->modes_priv);
		return GGI_ENOMEM;
        }
        
        for (i = 0; i < priv->modes_num; i++) {

		priv->modes[i].x = modes[i].viewportWidth;
		priv->modes[i].y = modes[i].viewportHeight;
		priv->modes[i].bpp = modes[i].depth;

#define GGI_XDGA_GTCONSTRUCT(ggigt) \
priv->modes[i].gt = GT_CONSTRUCT(modes[i].depth, ggigt, modes[i].bitsPerPixel);

		switch(modes[i].visualClass) {
		case TrueColor:
		case DirectColor: 
			GGI_XDGA_GTCONSTRUCT(GT_TRUECOLOR);   
			break;
		case PseudoColor: 
			GGI_XDGA_GTCONSTRUCT(GT_PALETTE);     
			break;
		case StaticGray:
		case GrayScale:   
			GGI_XDGA_GTCONSTRUCT(GT_GREYSCALE);   
			break;
		case StaticColor: 
			GGI_XDGA_GTCONSTRUCT(GT_STATIC_PALETTE);
			break;
		default:
#warning handle this case
			break;
		}
		GGIDPRINT_MISC("Found mode: %dx%d\n",
			       priv->modes[i].x, priv->modes[i].y);
        }
	return GGI_OK;
}

static int ggi_xdga_restore_mode(ggi_visual *vis) {
	ggi_x_priv *priv;
	priv = GGIX_PRIV(vis);

	if (priv->priv != NULL) XFree(priv->priv);
	priv->priv = XDGASetMode(priv->disp, priv->screen, 0);
	if (priv->priv != NULL) XFree(priv->priv); /* Docs not explicit */

	return GGI_OK;
}

static int ggi_xdga_enter_mode(ggi_visual *vis, int num) {
	ggi_x_priv *priv;
	XDGADevice *dev;
	XDGAMode *modes;

	priv = GGIX_PRIV(vis);
	dev = priv->priv;

	if (dev != NULL) XFree(dev);

	if ((num+1) > priv->modes_num) {
		GGIDPRINT("helper-x-dga: Bug somewhere -- bad mode index.\n");
		return GGI_ENODEVICE;
	}

	modes = (XDGAMode *)(priv->modes_priv);
	num = modes[num].num;
	priv->priv = dev = XDGASetMode(priv->disp, priv->screen, num);
	if (dev == NULL) return GGI_ENODEVICE;
	priv->fb = dev->data;
	priv->drawable = dev->pixmap;
	return GGI_OK;
}

static int ggi_xdga_validate_mode(ggi_visual *vis, int num, ggi_mode *maxed) {
	ggi_x_priv *priv;
	XDGAMode *modes;

	priv = GGIX_PRIV(vis);

	modes = (XDGAMode *)(priv->modes_priv);

	/* Find max values for maxed->virt and such. */

	return GGI_OK;

}


static int ggi_xdga_mmap (ggi_visual *vis) {
	ggi_x_priv *priv;

	priv = GGIX_PRIV(vis);

	/* This should have been taken care of by enter_mode */

#warning deal with banking here when backporting to older DGA
	return GGI_OK;
}

static int ggi_xdga_makerenderer (ggi_visual *vis) {
	ggi_x_priv *priv;
	XDGADevice *dev;

	priv = GGIX_PRIV(vis);
	dev = priv->priv;

	if (priv->slave != NULL) ggiClose(priv->slave);

#warning is dev->pixmap guaranteed None when pixmap flag not set in DGAmode?
	if (dev->pixmap) {
		priv->drawable = dev->pixmap;
#warning load dga accels here
		return GGI_OK;
	}

#warning create slave vis here.
	return GGI_OK;
}

#endif

static int GGIopen(ggi_visual *vis, struct ggi_dlhandle *dlh,
		   const char *args, void *argptr, uint32 *dlret)
{
	ggi_x_priv *priv;
	int dgafeat, i, j;

	priv = GGIX_PRIV(vis);

	XF86DGAQueryVersion(priv->disp, &i, &j);
	GGIDPRINT("display-DGA version %d.%d\n", i, j);
	if (i < 1) {
		fprintf(stderr, "Your XF86DGA is too old (%d.%d).\n", i, j);
		return GGI_ENODEVICE;
	}
	
	XF86DGAQueryDirectVideo(priv->disp,DefaultScreen(priv->disp),&dgafeat);
	if (!(dgafeat & XF86DGADirectPresent)) {
		fprintf(stderr, "helper-x-dga: No direct video capability!\n");
		return GGI_ENODEVICE;
	}

#if 0
	priv->createfb = ggi_xdga_mmap;
	priv->createdrawable = ggi_xdga_makerenderer;
#endif

	*dlret = 0;
	return GGI_OK;
}

static int GGIclose(ggi_visual *vis, struct ggi_dlhandle *dlh)
{
	return GGI_OK;
}


EXPORTFUNC
int GGIdl_helper_x_dga(int func, void **funcptr);

int GGIdl_helper_x_dga(int func, void **funcptr)
{
	switch (func) {
	case GGIFUNC_open:
		*funcptr = (void *)GGIopen;
		return GGI_OK;
	case GGIFUNC_exit:
		*funcptr = NULL;
		return GGI_OK;
	case GGIFUNC_close:
		*funcptr = (void *)GGIclose;
		return GGI_OK;
	default:
		*funcptr = NULL;
	}

	return GGI_ENOTFOUND;
}

#include <ggi/internal/ggidlinit.h>
