/* $Id: fillscreen.c,v 1.6 2005/02/10 04:55:20 orzo Exp $
******************************************************************************

   Graphics library for GGI. Fillscreenfunctions for X.

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

#include "config.h"
#include <ggi/internal/ggi-dl.h>
#include <ggi/display/x.h>
#include <ggi/internal/ggi_debug.h>

int GGI_X_fillscreen_slave(ggi_visual *vis) {
	ggi_x_priv *priv;
	priv = GGIX_PRIV(vis);

	DPRINT("X_fillscreen_slave enter!\n");
	if (LIBGGI_GC(vis)->cliptl.x > 0
	    || LIBGGI_GC(vis)->cliptl.y > 0
	    || LIBGGI_GC(vis)->clipbr.x < LIBGGI_VIRTX(vis)
	    || LIBGGI_GC(vis)->clipbr.y < LIBGGI_VIRTY(vis)) {
		GGI_X_DIRTY(vis, 
			    LIBGGI_GC(vis)->cliptl.x, LIBGGI_GC(vis)->cliptl.y,
			    LIBGGI_GC(vis)->clipbr.x-LIBGGI_GC(vis)->cliptl.x,
			    LIBGGI_GC(vis)->clipbr.y-LIBGGI_GC(vis)->cliptl.y);
	}
	else {
		GGI_X_DIRTY(vis, 0, 0, LIBGGI_VIRTX(vis), LIBGGI_VIRTY(vis));
	}
	priv->slave->opdraw->fillscreen(priv->slave);
	return GGI_OK;
}

int GGI_X_fillscreen_slave_draw(ggi_visual *vis)
{
	ggi_x_priv *priv;
	priv = GGIX_PRIV(vis);
	XGCValues gcValue;
	GC gc;

	DPRINT("X_fillscreen_slave_draw enter!\n");

	ggLock(priv->xliblock);
	
	/* XXX: What is priv->gc ?  is it appropriate to use that here? */
	gcValue.foreground = LIBGGI_GC(vis)->fg_color;
	gcValue.background = LIBGGI_GC(vis)->fg_color;
	gcValue.function   = GXcopy;
	gc = XCreateGC( priv->disp, priv->drawable,
	 		GCForeground | GCBackground | GCFunction, &gcValue);

	if (LIBGGI_GC(vis)->cliptl.x > 0
	    || LIBGGI_GC(vis)->cliptl.y > 0
	    || LIBGGI_GC(vis)->clipbr.x < LIBGGI_VIRTX(vis)
	    || LIBGGI_GC(vis)->clipbr.y < LIBGGI_VIRTY(vis)) {
		int y;

		DPRINT("X_fillscreen_slave_draw small clip!\n");
		GGI_X_CLEAN(vis, 
			    LIBGGI_GC(vis)->cliptl.x, LIBGGI_GC(vis)->cliptl.y,
			    LIBGGI_GC(vis)->clipbr.x-LIBGGI_GC(vis)->cliptl.x,
			    LIBGGI_GC(vis)->clipbr.y-LIBGGI_GC(vis)->cliptl.y);
		DPRINT("X_fillscreen_slave_draw calling opdraw->fillscreen\n");
		priv->slave->opdraw->fillscreen(priv->slave);
		/* What is this?  y is set twice?  which value is 
		 * proper?  Answer: GGI_X_WRITE_Y is a macro that makes
		 * use of the value stored in y. */
		y = LIBGGI_GC(vis)->cliptl.y;
		y = GGI_X_WRITE_Y;

#define CLIPSIZE(xy) \
		(unsigned)LIBGGI_GC(vis)->clipbr.xy-LIBGGI_GC(vis)->cliptl.xy

		XFillRectangle(priv->disp, priv->drawable, 
			gc, 
		        LIBGGI_GC(vis)->cliptl.x, y,
			CLIPSIZE(x),
			CLIPSIZE(y)
			);
	} else {
		DPRINT("X_fillscreen_slave_draw large clip!\n");
		GGI_X_CLEAN(vis, 0, 0, LIBGGI_VIRTX(vis), LIBGGI_VIRTY(vis));
		DPRINT("X_fillscreen_slave_draw calling opdraw->fillscreen\n");
		priv->slave->opdraw->fillscreen(priv->slave);
		XFillRectangle(priv->disp, priv->drawable, 
			gc, 0, 0, LIBGGI_VIRTX(vis), LIBGGI_VIRTY(vis) );
	}
	GGI_X_MAYBE_SYNC(vis);
	ggUnlock(priv->xliblock);
	return GGI_OK;
}


int GGI_X_fillscreen_draw(ggi_visual *vis)
{
	ggi_x_priv *priv;
	XGCValues gcValue;
	GC gc;
	priv = GGIX_PRIV(vis);

	DPRINT("X_fillscreen_draw enter!\n");

	ggLock(priv->xliblock);
	
	/* XXX: What is priv->gc ?  is it appropriate to use that here? */
	gcValue.foreground = LIBGGI_GC(vis)->fg_color;
	gcValue.background = LIBGGI_GC(vis)->fg_color;
	gcValue.function   = GXcopy;
	gc = XCreateGC( priv->disp, priv->drawable,
	 		GCForeground | GCBackground | GCFunction, &gcValue);

	if (LIBGGI_GC(vis)->cliptl.x > 0
	    || LIBGGI_GC(vis)->cliptl.y > 0
	    || LIBGGI_GC(vis)->clipbr.x < LIBGGI_VIRTX(vis)
	    || LIBGGI_GC(vis)->clipbr.y < LIBGGI_VIRTY(vis)) {
		int y;
		/* Note: GGI_X_WRITE_Y is a macro that makes use
		 * of the value stored in y. */
		y = LIBGGI_GC(vis)->cliptl.y;
		y = GGI_X_WRITE_Y;

#define CLIPSIZE(xy) \
		(unsigned)LIBGGI_GC(vis)->clipbr.xy-LIBGGI_GC(vis)->cliptl.xy

		XFillRectangle(priv->disp, priv->drawable, 
			gc, 
		        LIBGGI_GC(vis)->cliptl.x, y,
			CLIPSIZE(x),
			CLIPSIZE(y)
			);
	} else {
		XFillRectangle(priv->disp, priv->drawable, 
			gc, 0, 0, LIBGGI_VIRTX(vis), LIBGGI_VIRTY(vis) );
	}
	GGI_X_MAYBE_SYNC(vis);
	ggUnlock(priv->xliblock);

	DPRINT_LIBS("X_fillscreen_draw exit!\n");
	return 0;
}
