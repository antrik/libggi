/* $Id: mode.c,v 1.1 2001/05/12 23:01:55 cegger Exp $
******************************************************************************

   Graphics library for GGI. Mode management.

   Copyright (C) 1997      Jason McMullan	[jmcc@ggi-project.org]
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <ggi/internal/ggi-dl.h>
#include <ggi/display/xlib.h>

static void GGI_Xlib_gcchanged(ggi_visual *vis, int mask)
{
	ggi_xlib_priv *priv = LIBGGI_PRIVATE(vis);

	if ((mask & GGI_GCCHANGED_CLIP)) {
		XRectangle xrect;
		xrect.x = LIBGGI_GC(vis)->cliptl.x;
		xrect.y = LIBGGI_GC(vis)->cliptl.y;
		xrect.width 
			= LIBGGI_GC(vis)->clipbr.x-LIBGGI_GC(vis)->cliptl.x;
		xrect.height 
			= LIBGGI_GC(vis)->clipbr.y-LIBGGI_GC(vis)->cliptl.y;
		XSetClipRectangles(priv->xwin.x.display, priv->xwin.x.gc,
				   0, 0, &xrect, 1, Unsorted);
		XSetClipRectangles(priv->xwin.x.display, 
				   priv->tempgc,
				   0, 0, &xrect, 1, Unsorted);
	}
	if ((mask & GGI_GCCHANGED_FG)) {
		XSetForeground(priv->xwin.x.display, priv->xwin.x.gc,
			       LIBGGI_GC_FGCOLOR(vis));
	}
	if ((mask & GGI_GCCHANGED_BG)) {
		XSetBackground(priv->xwin.x.display, priv->xwin.x.gc,
			       LIBGGI_GC_BGCOLOR(vis));
	}
}

int GGI_Xlib_flush(ggi_visual *vis, int x, int y, int w, int h, int tryflag)
{
	ggi_xlib_priv *priv = LIBGGI_PRIVATE(vis);
	
	if (tryflag == 0) {
		if (ggTryLock(priv->xwin.x.xliblock) != 0) {
			GGIDPRINT_MISC("TRYLOCK fail!\n");
			return 0;
		}
	} else {
		ggLock(priv->xwin.x.xliblock);
	}

	if (priv->xwin.x.cmap && priv->xwin.cmap_first<priv->xwin.cmap_last) {
		int x;
		XColor xcol;
		for(x=priv->xwin.cmap_first;x<priv->xwin.cmap_last;x++)	{
			xcol.red  =vis->palette[x].r;
			xcol.green=vis->palette[x].g;
			xcol.blue =vis->palette[x].b;
			xcol.pixel=x;
			xcol.flags= DoRed | DoGreen | DoBlue ;
			XStoreColor(priv->xwin.x.display, priv->xwin.x.cmap,&xcol);
		}
		priv->xwin.cmap_first=256;
		priv->xwin.cmap_last=0;
		XSetWindowColormap(priv->xwin.x.display, priv->xwin.window, priv->xwin.x.cmap);
	}
	
	XFlush(priv->xwin.x.display);

	ggUnlock(priv->xwin.x.xliblock);
	
	return 0;
}

/*************************/
/* set the current flags */
/*************************/
int GGI_Xlib_setflags(ggi_visual *vis,ggi_flags flags)
{
	LIBGGI_FLAGS(vis) = flags;
	return 0;
}

#define GGI_X_flush	GGI_Xlib_flush
#define GGI_X_getmode	GGI_Xlib_getmode
#define GGI_X_setmode	GGI_Xlib_setmode
#define GGI_X_checkmode	GGI_Xlib_checkmode
#define GGI_X_getapi	GGI_Xlib_getapi
#define GGI_X_setpalvec	GGI_Xlib_setpalvec
#define ggi_x_priv	ggi_xlib_priv

#define GGI_XLIB_TARGET
#include "../X/mode.inc"
