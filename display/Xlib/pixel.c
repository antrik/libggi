/* $Id: pixel.c,v 1.2 2002/09/08 21:37:45 soyt Exp $
******************************************************************************

   Graphics library for GGI. Pixels for Xlib.

   Copyright (C) 1998 Marcus Sundberg [marcus@ggi-project.org]

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

#include <string.h>

#include "config.h"
#include <ggi/internal/ggi-dl.h>
#include <ggi/display/xlib.h>

/*******************************/
/* draw/get/put a single pixel */
/*******************************/

int GGI_Xlib_drawpixel(ggi_visual *vis,int x,int y)
{
	ggi_xlib_priv *priv = LIBGGI_PRIVATE(vis);

	XDrawPoint(priv->xwin.x.display, priv->xwin.window,
		   priv->xwin.x.gc, x, y); 
	
	XLIB_DOSYNC(vis);
	return 0;
}

int GGI_Xlib_putpixel(ggi_visual *vis,int x,int y,ggi_pixel col)
{
	ggi_xlib_priv *priv = LIBGGI_PRIVATE(vis);

	XSetForeground(priv->xwin.x.display, priv->tempgc, col);
	XDrawPoint(priv->xwin.x.display, priv->xwin.window, priv->tempgc, x, y);

	XLIB_DOSYNC(vis);
	return 0;
}

/* Hack to fall back when shm is not working. */
static int	geterror;

static int errorhandler (Display * disp, XErrorEvent * event)
{
	if (event->error_code == BadMatch) geterror = 1;

	return 0;
}

int GGI_Xlib_getpixel(ggi_visual *vis,int x,int y,ggi_pixel *pixel)
{
	ggi_xlib_priv *priv = LIBGGI_PRIVATE(vis);
	XImage *ximg;
	int	(*olderrorhandler) (Display *, XErrorEvent *);
	int ret = 0;
	
	XLIB_DOSYNC(vis);

	ggLock(_ggi_global_lock);

	geterror = 0;
	olderrorhandler = XSetErrorHandler(errorhandler);
	/* This will cause a BadMatch error when the window is
	   iconified or on another virtual screen... */
	ximg = XGetImage(priv->xwin.x.display, priv->xwin.window, x, y,
			  1, 1, AllPlanes, ZPixmap);
	XSetErrorHandler(olderrorhandler);
	*pixel = 0;
	if (!geterror) {
		memcpy(pixel, ximg->data, LIBGGI_PIXFMT(vis)->size/8);
		XDestroyImage(ximg);
	} else {
		ret = -1;
	}
	ggUnlock(_ggi_global_lock);

	return ret;
}
