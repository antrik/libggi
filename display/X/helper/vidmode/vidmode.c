/* $Id: vidmode.c,v 1.9 2004/10/17 19:06:04 mooz Exp $
******************************************************************************

   XFree86-VidMode extension support for display-x

   Copyright (C) 1997-1998 Steve Cheng		[steve@ggi-project.org]
   Copyright (C) 1999-2000 Marcus Sundberg	[marcus@ggi-project.org]
   Copyright (C) 2002      Brian S. Julin	[bri@tull.umassp.edu]
   Copyright (C) 2003-2004 Vincent Cruz         [vcruz@free.fr]

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
 * The requested mode must be one available X mode.
 * We don't search for the highest closest mode, then
 * create a window with the found mode. And finally
 * reparent priv->parent_win to the newly created window
 * because the behaviour of XReparent window changes from
 * on window manager to an other. This technique works well
 * under KDE, icewm, xfce, twm. But it failed under gnome
 * (priv->parent_win turned to overrideride_ridection mode),
 * and under window maker (the children didn't get mapped).
 *
 * Frameless windows should be handled by libggiwmh wia ICCM.
 *
 * I hate Window Managers :)
 */

/*
 *  Private vimode structure
 */
typedef struct {
  /* Mode lines */
  XF86VidModeModeInfo **modes; 

  /* Initial viewport settings */
  int old_x, old_y;

  /* 
   *  Mode validation flag 
   *  -1 validation failed
   *   0 no exact mode found but a closest one was (unused)
   *   1 mode found
   */
  char validation_flag; 
} ggi_x_vidmode;

/*
  Clean up 
*/
static void ggi_xvidmode_cleanup(ggi_visual * vis) {
  ggi_x_priv    *priv    = GGIX_PRIV(vis);
  
  if(priv->modes_priv) {
    ggi_x_vidmode *vidmode = (ggi_x_vidmode*)priv->modes_priv;

    if(vidmode->modes) {
      XFree(vidmode->modes);
      vidmode->modes = NULL;
    }

    free(priv->modes_priv);
    priv->modes_priv = NULL;
  }
}

/*
  Check video mode
*/
static int ggi_xvidmode_getmodelist(ggi_visual * vis)
{
	ggi_x_priv    *priv;
	ggi_graphtype gt;
	ggi_x_vidmode *vidmode;
	int i;

	GGIDPRINT_MODE("ggi_xvidmode_getmodelist\n");

	priv = GGIX_PRIV(vis);

	/* Allocate private vidmode structure     */
	/* Reuse allocated data whenever possible */
	if(priv->modes_priv) {
	  vidmode = (ggi_x_vidmode*)priv->modes_priv;
	  if(vidmode->modes) {  
	    XFree(vidmode->modes);
	    vidmode->modes = NULL;
	  }
	}
	else {
	  vidmode = (ggi_x_vidmode*)calloc(1, sizeof(ggi_x_vidmode));
	  priv->modes_priv = vidmode;
	}
	
	if(vidmode == NULL) {
	  GGIDPRINT_MODE("\tggi_x_vidmode allocation failed\n");
	  goto error_mem;
	}
  
	/* Save current viewport */
	XF86VidModeGetViewPort(priv->disp, priv->vilist[priv->viidx].vi->screen,
			       &(vidmode->old_x), &(vidmode->old_y));

	/* Get available modes list */
#warning figure out scheme/gt from main visual here  old code was lacking
	gt = GT_CONSTRUCT(priv->vilist[priv->viidx].vi->depth,
			  GT_TRUECOLOR,
			  priv->vilist[priv->viidx].buf->bits_per_pixel);

	priv->modes_num = 0;
	if (!XF86VidModeGetAllModeLines
	    (priv->disp, priv->vilist[priv->viidx].vi->screen,
	     &(priv->modes_num), &(vidmode->modes))) {
		GGIDPRINT_MODE("\tXF86VidModeGetAllModeLines failed\n");
		goto error_device;
	}
	if (vidmode->modes == NULL) {
		GGIDPRINT_MODE("\tNo modes found (empty mode array).\n");
		goto error_device;
	}
	if (priv->modes_num <= 0) {
		GGIDPRINT_MODE("\tNo modes found (mode number <= 0).\n");
		goto error_device;
	}

	priv->modes = calloc(sizeof(ggi_modelistmode),
				(size_t)(priv->modes_num));
	if (priv->modes == NULL) {
		GGIDPRINT_MODE("\tNo enough memory.\n");
		goto error_mem;
	}

	GGIDPRINT_MODE("\t# modes: %d\n",priv->modes_num);

	for (i = 0; i < priv->modes_num; i++) {
		priv->modes[i].x = vidmode->modes[i]->hdisplay;
		priv->modes[i].y = vidmode->modes[i]->vdisplay;
		priv->modes[i].bpp =
		    priv->vilist[priv->viidx].buf->depth;
		priv->modes[i].gt = gt;
		GGIDPRINT_MODE("Found mode: %dx%d %dbpp\n",
			       priv->modes[i].x, priv->modes[i].y,
			       priv->modes[i].bpp);
	}

	return GGI_OK;

 error_mem:
	ggi_xvidmode_cleanup(vis);
	return GGI_ENOMEM;

 error_device:
	ggi_xvidmode_cleanup(vis);
	return GGI_ENODEVICE;
}

/* 
   Set video mode
*/
static int ggi_xvidmode_enter_mode(ggi_visual * vis, int num)
{
	ggi_x_priv *priv;

	ggi_x_vidmode *vidmode;

	Window child_return;
	Window root;

	int    x, y;

	priv = GGIX_PRIV(vis);
	
	GGIDPRINT_MODE("ggi_xvidmode_enter_mode (mode # %d, actual mode #: %d)\n",
		       num, priv->cur_mode);

	vidmode = (ggi_x_vidmode*)priv->modes_priv;

	if((!num) && 
	   (vidmode->validation_flag < 0)) {
	       GGIDPRINT_MODE
	         ("helper-x-vidmode: No suitable mode found.\n");
	       return GGI_OK;
	}

	if (num >= priv->modes_num) {
		GGIDPRINT_MODE
		    ("helper-x-vidmode: .Bug somewhere -- bad mode index.\n");
		return GGI_ENODEVICE;
	}

	GGIDPRINT_MODE
	    ("\tXF86VidModeSwitchToMode(%x, %d, %x) %d called with:",
	     priv->disp, priv->vilist[priv->viidx].vi->screen, vidmode->modes[num], DefaultScreen(priv->disp));

	GGIDPRINT_MODE("\tmodes[%d]:\n", num);
	GGIDPRINT_MODE("\tdotclock    %d\n", vidmode->modes[num]->dotclock);
	GGIDPRINT_MODE("\thdisplay    %d\n", vidmode->modes[num]->hdisplay);
	GGIDPRINT_MODE("\thsyncstart  %d\n", vidmode->modes[num]->hsyncstart);
	GGIDPRINT_MODE("\thsyncend    %d\n", vidmode->modes[num]->hsyncend);
	GGIDPRINT_MODE("\thtotal      %d\n", vidmode->modes[num]->htotal);
	GGIDPRINT_MODE("\tvdisplay    %d\n", vidmode->modes[num]->vdisplay);
	GGIDPRINT_MODE("\tvsyncstart  %d\n", vidmode->modes[num]->vsyncstart);
	GGIDPRINT_MODE("\tvsyncend    %d\n", vidmode->modes[num]->vsyncend);
	GGIDPRINT_MODE("\tvtotal      %d\n", vidmode->modes[num]->vtotal);
	GGIDPRINT_MODE("\tflags       %d\n", vidmode->modes[num]->flags);
	GGIDPRINT_MODE("\tprivsize    %d\n", vidmode->modes[num]->privsize);
	GGIDPRINT_MODE("\tprivate     %x\n", vidmode->modes[num]->private);

	XMoveWindow(priv->disp, priv->parentwin, 0, 0);

	GGIDPRINT_MODE("Unlock mode switching\n");
	/* Unlock mode switching */
	XF86VidModeLockModeSwitch(priv->disp,
				  priv->vilist[priv->viidx].vi->screen, 
				  False);
		
	GGIDPRINT_MODE("Switching to mode %d\n", num);
	if (!XF86VidModeSwitchToMode(priv->disp,
				     priv->vilist[priv->viidx].vi->screen,
				     vidmode->modes[num])) {
	  GGIDPRINT_MODE("XF86VidModeSwitchToMode failed.\n");
	  return GGI_ENODEVICE;
	}
	
	GGIDPRINT_MODE("Setting viewport\n");
	/* 
	 * Here we translate window origin to Root window origin 
	 * in order to get the viewport centered on the window.
	 * So no need to use override-redirect nor iCCM.
	 */
	root = DefaultRootWindow(priv->disp);
	
	XTranslateCoordinates(priv->disp, priv->parentwin, root, 
			      0, 0, &x, &y, &child_return);
	
	XF86VidModeSetViewPort(priv->disp,
			       priv->vilist[priv->viidx].vi->screen, 
			       x,
			       y);
	
	GGIDPRINT_MODE("Lock mode switching\n");
	/* Lock mode switching */
	XF86VidModeLockModeSwitch(priv->disp,
				  priv->vilist[priv->viidx].vi->screen,
				  True);


	
	GGI_X_MAYBE_SYNC(vis);
	
	return GGI_OK;
}

static int ggi_xvidmode_validate_mode(ggi_visual * vis, int num,
				      ggi_mode * maxed)
{
	ggi_x_priv    *priv    = GGIX_PRIV(vis);
	ggi_x_vidmode *vidmode;


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
	
	vidmode = (ggi_x_vidmode*)priv->modes_priv;

	/* 
	 * First pass : Check for correct mode 
	 */
	do {
	  if ((maxed->visible.x == priv->modes[num + 1].x) &&
	      (maxed->visible.y == priv->modes[num + 1].y) &&
	      (GT_DEPTH(maxed->graphtype) ==
	       (unsigned) priv->modes[num + 1].bpp)) {	
	      GGIDPRINT_MODE("\tvalid mode: bpp:%d w:%d h:%d\n",
			     priv->modes[num + 1].bpp, priv->modes[num + 1].x,
			     priv->modes[num + 1].y);
	      /* Mode found */
	      vidmode->validation_flag = 1;
	      return (num + 1);
	  }

	  ++num;
	} while(num < priv->modes_num);

	/*
	 * No second pass (see comment at the begining of the file)
	 */

	/* No valid mode found */
	vidmode->validation_flag = -1;

	return -1;
}

/*
  Restore video mode
*/
static int ggi_xvidmode_restore_mode(ggi_visual * vis)
{
	ggi_x_priv    *priv    = GGIX_PRIV(vis);
	ggi_x_vidmode *vidmode = (ggi_x_vidmode*)priv->modes_priv;

	GGIDPRINT_MODE("ggi_xvidmode_restore_mode\n");

	/* Unlock mode switching */
	XF86VidModeLockModeSwitch(priv->disp,
				  priv->vilist[priv->viidx].vi->screen, 
				  False);

	/* Switch back to 'root' mode */
	XF86VidModeSwitchToMode(priv->disp,
				priv->vilist[priv->viidx].vi->screen,
				vidmode->modes[0]);

	/* Set the viewport to root window origin */
	XF86VidModeSetViewPort(priv->disp,
			       priv->vilist[priv->viidx].vi->screen,
			       vidmode->old_x, vidmode->old_y);

	return GGI_OK;
}

static int GGIopen(ggi_visual * vis, struct ggi_dlhandle *dlh,
		   const char *args, void *argptr, uint32 * dlret)
{
	ggi_x_priv *priv;
	int x, y;

	priv = GGIX_PRIV(vis);

	if (!XF86VidModeQueryVersion(priv->disp, &x, &y)) {
	        GGIDPRINT_MODE("\tXF86VidModeQueryVersion failed\n");
		return GGI_ENOFUNC;
	}
	GGIDPRINT_MODE("XFree86 VideoMode Extension version %d.%d\n", 
		       x, y);

	/*
	   overload mode list functions
	 */

	priv->mlfuncs.getlist  = ggi_xvidmode_getmodelist;
	priv->mlfuncs.restore  = ggi_xvidmode_restore_mode;
	priv->mlfuncs.enter    = ggi_xvidmode_enter_mode;
	priv->mlfuncs.validate = ggi_xvidmode_validate_mode;


	*dlret = 0;
	return GGI_OK;
}

static int GGIclose(ggi_visual * vis, struct ggi_dlhandle *dlh)
{
        ggi_xvidmode_restore_mode(vis);
	ggi_xvidmode_cleanup     (vis);

	return GGI_OK;
}

EXPORTFUNC
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
