/* $Id: mode.c,v 1.2 2002/03/14 11:45:27 cegger Exp $
******************************************************************************

   Graphics library for GGI. X target.

   Copyright (C) 1997      Jason McMullan	[jmcc@ggi-project.org]
   Copyright (C) 1998      Andreas Beck		[becka@ggi-project.org]
   Copyright (C) 1998      Steve Cheng		[steve@ggi-project.org]
   Copyright (C) 1998-2000 Marcus Sundberg	[marcus@ggi-project.org]

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
#include <ggi/display/x.h>
#include <ggi/display/mansync.h>


static int GGI_X_setorigin(ggi_visual *vis,int x,int y)
{
	ggi_x_priv *priv=LIBGGI_PRIVATE(vis);
	ggi_mode *mode=LIBGGI_MODE(vis);

	if ( x<0 || x> mode->virt.x-mode->visible.x ||
	     y<0 || y> mode->virt.y-priv->ysplit )
	     return -1;

	priv->xoff=x;
	priv->yoff=y;

	vis->origin_x=x;
	vis->origin_y=y;
	
	return 0;
}


static int GGI_X_setdisplayframe(ggi_visual *vis, int num)
{
	ggi_x_priv *priv=LIBGGI_PRIVATE(vis);
	ggi_directbuffer *db = _ggi_db_find_frame(vis, num);

	if (db == NULL) {
		return -1;
	}

	vis->d_frame_num = num;
	priv->ximage = priv->ximage_list[num];

	return 0;
}


int _ggi_x_do_blit(ggi_x_priv *priv, int x, int y, int w, int h)
{
	GGIDPRINT_DRAW("_ggi_x_do_blit(%p, %dx%d, %dx%d) called\n",
		       priv, x, y, w, h);

	/* Expose event may be queued from a previous (larger) mode.
	   In that case we just ignore it and return.
	   (Calls from ggiFlush* are guaranteed to have been checked
	    already) */
	if (x+w > priv->viswidth || y+h > priv->visheight) return 0;

	/* Handle ysplit */
	if (y+h > priv->ysplit) {
		h = priv->ysplit - y;
		if (h < 0) h = 0;
	}
#ifdef HAVE_SHM
	if (priv->have_shm) {
		/* Sync before starting a new blit so we don't flood the
		   X server with ShmPut requests. */
		XSync(priv->xwin.x.display, 0);
		XShmPutImage(priv->xwin.x.display, priv->xwin.window,
			     priv->xwin.x.gc,
			     priv->ximage, 
			     priv->xoff+x, priv->yoff+y, /* panning ! */
			     x, y,
			     w, h, 0);

		if (priv->ysplit < priv->visheight) {
			XShmPutImage(priv->xwin.x.display, priv->xwin.window,
				     priv->xwin.x.gc,
				     priv->ximage, 
				     priv->xoff+x, priv->yoff+y,
				     x, priv->ysplit,
				     w, priv->visheight-priv->ysplit, 0);
		}
	} else 
#endif
	{
		/* Sync before starting a new blit so we don't flood the
		   X server with Put requests. */
		XSync(priv->xwin.x.display, 0);
		XPutImage(priv->xwin.x.display, priv->xwin.window,
			  priv->xwin.x.gc,
			  priv->ximage, 
			  priv->xoff+x, priv->yoff+y, /* panning ! */
			  x, y,
			  w, h);

		if (priv->ysplit<priv->visheight) {
			XPutImage(priv->xwin.x.display, priv->xwin.window,
				  priv->xwin.x.gc,
				  priv->ximage, 
				  priv->xoff+x, priv->yoff+y,
				  x, priv->ysplit,
				  w, priv->visheight - priv->ysplit);
		}
	}
	/* Now tell the X server to start blitting. */
	XFlush(priv->xwin.x.display);

	return 0;
}

int GGI_X_flush(ggi_visual *vis, int x, int y, int w, int h, int tryflag)
{
	ggi_x_priv *priv = LIBGGI_PRIVATE(vis);

	GGIDPRINT_DRAW("GGI_X_flush(%p, %d) called\n", vis, tryflag);


	if (tryflag == 0) {
		if (ggTryLock(priv->xwin.x.xliblock) != 0) {
			GGIDPRINT_MISC("TRYLOCK fail!\n");
			return 0;
		}
	} else {
		ggLock(priv->xwin.x.xliblock);
	}

	if (priv->xwin.x.cmap && priv->xwin.cmap_first < priv->xwin.cmap_last) {
		int x;
		XColor xcol;
	       	for (x=priv->xwin.cmap_first; x < priv->xwin.cmap_last; x++) {
       			xcol.red   = vis->palette[x].r;
	       		xcol.green = vis->palette[x].g;
       			xcol.blue  = vis->palette[x].b;
       			xcol.pixel = x;
	       		xcol.flags = DoRed | DoGreen | DoBlue;
       			XStoreColor(priv->xwin.x.display, priv->xwin.x.cmap, &xcol);
	       	}
		priv->xwin.cmap_first = priv->xwin.x.nocols;
		priv->xwin.cmap_last = 0;
		XSetWindowColormap(priv->xwin.x.display, priv->xwin.window,
				   priv->xwin.x.cmap);
	}

	_ggi_x_do_blit(priv, x, y, w, h);

	ggUnlock(priv->xwin.x.xliblock);

	return 0;
}

#ifdef HAVE_SHM
/* Hack to fall back when shm is not working. */
static int	shmerror;
static int	(*oldshmerrorhandler)(Display *, XErrorEvent *);

static int shmerrorhandler (Display * disp, XErrorEvent * event)
{
	if (event->error_code == BadAccess) shmerror = 1;
	else oldshmerrorhandler(disp, event);

	return 0;
}
#endif

/*************************/
/* set the current flags */
/*************************/
int GGI_X_setflags(ggi_visual *vis,ggi_flags flags)
{
	LIBGGI_FLAGS(vis)=flags;

	MANSYNC_SETFLAGS(vis,flags);

	return 0;
}

#define GGI_X_TARGET
#include "./mode.inc"
