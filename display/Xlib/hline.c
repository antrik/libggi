/* $Id: hline.c,v 1.1 2001/05/12 23:01:55 cegger Exp $
******************************************************************************

   LibGGI - horizontal lines for Xlib

   Copyright (C) 1998-1999 Marcus Sundberg	[marcus@ggi-project.org]

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

#include <ggi/internal/ggi-dl.h>
#include <ggi/display/xlib.h>

int GGI_Xlib_drawhline(ggi_visual *vis,int x,int y,int w)
{
	ggi_xlib_priv *priv = LIBGGI_PRIVATE(vis);

	XDrawLine(priv->xwin.x.display, priv->xwin.window, priv->xwin.x.gc,
		  x, y, x+w-1, y);
	
	XLIB_DOSYNC(vis);
	return 0;     
}

int GGI_Xlib_puthline(ggi_visual *vis, int x, int y, int w, void *data)
{
	ggi_xlib_priv *priv = LIBGGI_PRIVATE(vis);
	XImage *ximg;
	
	ximg = XCreateImage(priv->xwin.x.display, priv->xwin.visual.visual,
			    LIBGGI_PIXFMT(vis)->depth, ZPixmap, 0,
			    data, w, 1, 8, 0);
	XPutImage(priv->xwin.x.display, priv->xwin.window, priv->xwin.x.gc, ximg,
		  0, 0, x, y, w, 1);
	XFree(ximg);

	XLIB_DOSYNC(vis);
	return 0;
}
