/* $Id: vidmode.c,v 1.2 2002/09/08 21:37:44 soyt Exp $
******************************************************************************

   XFree86-VidMode extension support for display-x

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
#include <ggi/internal/ggi-dl.h>
#include <ggi/display/x.h>
#include <X11/extensions/xf86vmode.h>

#if 0
static int ggi_xvidmode_getmodelist(ggi_visual *vis) {
	ggi_x_priv *priv;
	XF86VidModeModeInfo **modes;
	unsigned int depth = 0;
	ggi_graphtype gt;
	XImage *bpptest;
	unsigned int bpp = 0;
	int i;

	priv = GGIX_PRIV(vis);

	do {
		Window root;
		int x, y;
		unsigned int w, h, b;
		if(XGetGeometry(priv->disp, 
				RootWindow(priv->disp, priv->screen),
				&root, &x, &y, &w, &h, &b, &depth))
			return GGI_ENODEVICE;
		if (!depth) return GGI_ENODEVICE;
	} while(0);

	if ((bpptest = XGetImage(priv->disp,
				 RootWindow(priv->disp, priv->screen),
				 0, 0, 1, 1, AllPlanes, ZPixmap)) != NULL) {
		bpp = bpptest->bits_per_pixel;
		XDestroyImage(bpptest);
	} else return GGI_ENODEVICE;

#warning figure out scheme/gt from main visual here  old code was lacking
	gt = GT_CONSTRUCT(depth, GT_TRUECOLOR, bpp);

	priv->modes_num = 0;
	if (XF86VidModeGetAllModeLines(priv->disp, priv->screen, 
				       &(priv->modes_num), &modes)) {
		return GGI_ENODEVICE;
	}
	if (modes == NULL) return GGI_ENODEVICE;
	if (priv->modes_num <= 0) return GGI_ENODEVICE;

	priv->modes_priv = modes;

#warning old code has modes_num + 1 here ??
        priv->modes = calloc(sizeof(ggi_modelistmode), priv->modes_num);
        if (priv->modes == NULL) {
#warning LEAK: figure out proper cleanup here.
		return GGI_ENOMEM;
        }
        
        for (i = 0; i < priv->modes_num; i++) {

		priv->modes[i].x = modes[i]->hdisplay;
		priv->modes[i].y = modes[i]->vdisplay;
		priv->modes[i].bpp = depth;
		priv->modes[i].gt = gt;
		GGIDPRINT_MISC("Found mode: %dx%d\n",
			       priv->modes[i].x, priv->modes[i].y);
        }
	return GGI_OK;
}

static int ggi_xvidmode_restore_mode(ggi_visual *vis) {
	ggi_x_priv *priv;

	priv = GGIX_PRIV(vis);

	/* First mode in list is original mode. */
	XF86VidModeSwitchToMode(priv->disp, priv->screen, 
				((XF86VidModeModeInfo **)priv->modes_priv)[0]);

	return GGI_OK;
}

static int ggi_xdga_enter_mode(ggi_visual *vis, int num) {
	ggi_x_priv *priv;
	XF86VidModeModeInfo **modes;

	priv = GGIX_PRIV(vis);

#warning check for additional member here (see old code)
	if ((num+1) > priv->modes_num) {
		GGIDPRINT("helper-x-vidmode: Bug -- bad mode index.\n");
		return GGI_ENODEVICE;
	}

	modes = priv->modes_priv;
	if (XF86VidModeSwitchToMode(priv->disp, priv->screen, modes[num])) {
		return GGI_ENODEVICE;
	}
	return GGI_OK;
}

static int ggi_xdga_validate_mode(ggi_visual *vis, int num, ggi_mode *maxed) {
	ggi_x_priv *priv;
	XF86VidModeModeInfo **modes;

	priv = GGIX_PRIV(vis);

	modes = priv->modes_priv;

	/* Find max values for maxed->virt and such. */

	return GGI_OK;
}

#endif 

static int GGIopen(ggi_visual *vis, struct ggi_dlhandle *dlh,
		   const char *args, void *argptr, uint32 *dlret)
{
	ggi_x_priv *priv;
	int x, y;

	priv = GGIX_PRIV(vis);

	if (XF86VidModeQueryVersion(priv->disp, &x, &y)) return GGI_ENOFUNC;
	GGIDPRINT("XFree86 VideoMode Extension version %d.%d\n", x, y);

	*dlret = 0;
	return GGI_OK;
}

static int GGIclose(ggi_visual *vis, struct ggi_dlhandle *dlh)
{
#warning LEAK: deallocate modes here
	return GGI_OK;
}


int GGIdl_helper_x_vidmode(int func, void **funcptr)
{
	switch (func) {
	case GGIFUNC_open:
		*funcptr = GGIopen;
		return GGI_OK;
	case GGIFUNC_exit:
		*funcptr = NULL;
		return GGI_OK;
	case GGIFUNC_close:
		*funcptr = GGIclose;
		return GGI_OK;
	default:
		*funcptr = NULL;
	}

	return GGI_ENOTFOUND;
}

#include <ggi/internal/ggidlinit.h>
