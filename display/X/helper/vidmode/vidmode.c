/* $Id: vidmode.c,v 1.4 2003/11/27 18:24:58 cegger Exp $
******************************************************************************

   XFree86-VidMode extension support for display-x

   Copyright (C) 1997-1998 Steve Cheng		[steve@ggi-project.org]
   Copyright (C) 1999-2000 Marcus Sundberg	[marcus@ggi-project.org]
   Copyright (C) 2002      Brian S. Julin	[bri@tull.umassp.edu]
   Copyright (C) 2003      Vincent Cruz         [vcruz@free.fr]

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

/*
  Check video mode
*/
static int ggi_xvidmode_getmodelist(ggi_visual * vis)
{
	ggi_x_priv *priv;
	ggi_graphtype gt;
	XF86VidModeModeInfo **modes;
	int i;

	GGIDPRINT_MODE("ggi_xvidmode_getmodelist\n");

	priv = GGIX_PRIV(vis);

#warning figure out scheme/gt from main visual here  old code was lacking
	gt = GT_CONSTRUCT(priv->vilist[priv->viidx].vi->depth,
			  GT_TRUECOLOR,
			  priv->vilist[priv->viidx].buf->bits_per_pixel);

	priv->modes_num = 0;
	if (!XF86VidModeGetAllModeLines
	    (priv->disp, priv->vilist[priv->viidx].vi->screen,
	     &(priv->modes_num), &modes)) {
		GGIDPRINT_MODE("\tXF86VidModeGetAllModeLines failed\n");
		return GGI_ENODEVICE;
	}
	if (modes == NULL)
		return GGI_ENODEVICE;
	if (priv->modes_num <= 0)
		return GGI_ENODEVICE;

	priv->modes_priv = modes;

	priv->modes = calloc(sizeof(ggi_modelistmode),
				(size_t)(priv->modes_num));
	if (priv->modes == NULL) {
#warning LEAK: figure out proper cleanup here.
		return GGI_ENOMEM;
	}

	for (i = 0; i < priv->modes_num; i++) {
		priv->modes[i].x = modes[i]->hdisplay;
		priv->modes[i].y = modes[i]->vdisplay;
		priv->modes[i].bpp =
		    priv->vilist[priv->viidx].buf->bits_per_pixel;
		priv->modes[i].gt = gt;
		GGIDPRINT_MODE("Found mode: %dx%d %dbpp\n",
			       priv->modes[i].x, priv->modes[i].y,
			       priv->modes[i].bpp);
	}

	GGIDPRINT_MODE("\n");

	return GGI_OK;
}

/* 
   Set video mode
*/
static int ggi_xvidmode_enter_mode(ggi_visual * vis, int num)
{
	ggi_x_priv *priv;
	XF86VidModeModeInfo **modes;

	GGIDPRINT_MODE("ggi_xvidmode_enter_mode\n");

	priv = GGIX_PRIV(vis);

	if ((num + 1) > priv->modes_num) {
		GGIDPRINT_MODE
		    ("helper-x-vidmode: .Bug somewhere -- bad mode index.\n");
		return GGI_ENODEVICE;
	}

	modes = (XF86VidModeModeInfo **) (priv->modes_priv);

	GGIDPRINT_MODE
	    ("\tXF86VidModeSwitchToMode(%x, %d, %x) called with:",
	     priv->disp, priv->vilist[priv->viidx].vi->screen, modes[num]);

	GGIDPRINT_MODE("\tmodes[%d]:\n", num);
	GGIDPRINT_MODE("\tdotclock    %d\n", modes[num]->dotclock);
	GGIDPRINT_MODE("\thdisplay    %d\n", modes[num]->hdisplay);
	GGIDPRINT_MODE("\thsyncstart  %d\n", modes[num]->hsyncstart);
	GGIDPRINT_MODE("\thsyncend    %d\n", modes[num]->hsyncend);
	GGIDPRINT_MODE("\thtotal      %d\n", modes[num]->htotal);
	GGIDPRINT_MODE("\tvdisplay    %d\n", modes[num]->vdisplay);
	GGIDPRINT_MODE("\t vsyncstart %d\n", modes[num]->vsyncstart);
	GGIDPRINT_MODE("\tvsyncend    %d\n", modes[num]->vsyncend);
	GGIDPRINT_MODE("\tvtotal      %d\n", modes[num]->vtotal);
	GGIDPRINT_MODE("\tflags       %d\n", modes[num]->flags);
	GGIDPRINT_MODE("\tprivsize    %d\n", modes[num]->privsize);
	GGIDPRINT_MODE("\tprivate     %x\n", modes[num]->private);

	if (!XF86VidModeSwitchToMode(priv->disp,
				     priv->vilist[priv->viidx].vi->screen,
				     modes[num])) {
		return GGI_ENODEVICE;
	}

	/* TODO (viewport info) */
	XF86VidModeSetViewPort(priv->disp,
			       priv->vilist[priv->viidx].vi->screen, 0, 0);

	return GGI_OK;
}

static int ggi_xvidmode_validate_mode(ggi_visual * vis, int num,
				      ggi_mode * maxed)
{
	ggi_x_priv *priv = GGIX_PRIV(vis);

	GGIDPRINT_MODE("ggi_xvidmode_validate_mode: %x %x\n", priv,
		       priv->modes);

	GGIDPRINT_MODE("\tmode number:%d of %d\n", num + 1,
		       priv->modes_num);
	if ((priv->modes_num < 1) && (!priv->modes_num))
		ggi_xvidmode_getmodelist(vis);

	GGIDPRINT_MODE("\trequested mode: depth:%d  bpp:%d w:%d y:%d\n",
		       GT_DEPTH(maxed->graphtype),
		       GT_ByPP(maxed->graphtype), maxed->visible.x,
		       maxed->visible.y);
	GGIDPRINT_MODE("\tavailable mode: bpp:%d w:%d h:%d\n",
		       priv->modes[num + 1].bpp, priv->modes[num + 1].x,
		       priv->modes[num + 1].y);

	if ((maxed->visible.x == priv->modes[num + 1].x) &&
	    (maxed->visible.y == priv->modes[num + 1].y) &&
	    (GT_DEPTH(maxed->graphtype) ==
	     (unsigned) priv->modes[num + 1].bpp))
		return (num + 1);

	return -1;
}

/*
  Restore video mode
*/
static int ggi_xvidmode_restore_mode(ggi_visual * vis)
{
	ggi_x_priv *priv = GGIX_PRIV(vis);

	GGIDPRINT_MODE("ggi_xvidmode_restore_mode\n");

	/* The original mode is the first one in the list */
	XF86VidModeSwitchToMode(priv->disp,
				priv->vilist[priv->viidx].vi->screen,
				((XF86VidModeModeInfo **) priv->
				 modes_priv)[0]);

	/* TODO */
	XF86VidModeSetViewPort(priv->disp,
			       priv->vilist[priv->viidx].vi->screen, 0, 0);

	return GGI_OK;
}

static int GGIopen(ggi_visual * vis, struct ggi_dlhandle *dlh,
		   const char *args, void *argptr, uint32 * dlret)
{
	ggi_x_priv *priv;
	int x, y;

	priv = GGIX_PRIV(vis);

	if (!XF86VidModeQueryVersion(priv->disp, &x, &y))
		return GGI_ENOFUNC;
	GGIDPRINT_MODE("XFree86 VideoMode Extension version %d.%d\n", x,
		       y);

	/*
	   overload mode list functions
	 */

	priv->mlfuncs.getlist = ggi_xvidmode_getmodelist;
	priv->mlfuncs.restore = ggi_xvidmode_restore_mode;
	priv->mlfuncs.enter = ggi_xvidmode_enter_mode;
	priv->mlfuncs.validate = ggi_xvidmode_validate_mode;


	*dlret = 0;
	return GGI_OK;
}

static int GGIclose(ggi_visual * vis, struct ggi_dlhandle *dlh)
{
	return GGI_OK;
}

int GGIdl_helper_x_vidmode(int func, void **funcptr);

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
