/* $Id: fillscreen.c,v 1.3 2003/07/06 10:25:21 cegger Exp $
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

int GGI_X_fillscreen_slave(ggi_visual *vis) {
	ggi_x_priv *priv;
	priv = GGIX_PRIV(vis);

	if (LIBGGI_GC(vis)->cliptl.x > 0
	    || LIBGGI_GC(vis)->cliptl.y > 0
	    || LIBGGI_GC(vis)->clipbr.x < LIBGGI_MODE(vis)->virt.x
	    || LIBGGI_GC(vis)->clipbr.y < LIBGGI_MODE(vis)->virt.x) {
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

	ggLock(priv->xliblock);
	XSetWindowBackground(priv->disp, priv->drawable,
			     LIBGGI_GC(vis)->fg_color);

	if (LIBGGI_GC(vis)->cliptl.x > 0
	    || LIBGGI_GC(vis)->cliptl.y > 0
	    || LIBGGI_GC(vis)->clipbr.x < LIBGGI_MODE(vis)->virt.x
	    || LIBGGI_GC(vis)->clipbr.y < LIBGGI_MODE(vis)->virt.x) {
		int y;

		GGI_X_CLEAN(vis, 
			    LIBGGI_GC(vis)->cliptl.x, LIBGGI_GC(vis)->cliptl.y,
			    LIBGGI_GC(vis)->clipbr.x-LIBGGI_GC(vis)->cliptl.x,
			    LIBGGI_GC(vis)->clipbr.y-LIBGGI_GC(vis)->cliptl.y);
		priv->slave->opdraw->fillscreen(priv->slave);
		y = LIBGGI_GC(vis)->cliptl.y;
		y = GGI_X_WRITE_Y;
		XClearArea(priv->disp, priv->drawable,
			   LIBGGI_GC(vis)->cliptl.x, y,
			   (unsigned)LIBGGI_GC(vis)->clipbr.x-LIBGGI_GC(vis)->cliptl.x,
			   (unsigned)LIBGGI_GC(vis)->clipbr.y-LIBGGI_GC(vis)->cliptl.y, 
			   False);
	} else {
		GGI_X_CLEAN(vis, 0, 0, LIBGGI_VIRTX(vis), LIBGGI_VIRTY(vis));
		priv->slave->opdraw->fillscreen(priv->slave);
		XClearWindow(priv->disp, priv->drawable);
	}
	GGI_X_MAYBE_SYNC(vis);
	ggUnlock(priv->xliblock);
	return GGI_OK;
}


int GGI_X_fillscreen_draw(ggi_visual *vis)
{
	ggi_x_priv *priv;
	priv = GGIX_PRIV(vis);

	ggLock(priv->xliblock);
	XSetWindowBackground(priv->disp, priv->drawable,
			     LIBGGI_GC(vis)->fg_color);

	if (LIBGGI_GC(vis)->cliptl.x > 0
	    || LIBGGI_GC(vis)->cliptl.y > 0
	    || LIBGGI_GC(vis)->clipbr.x < LIBGGI_MODE(vis)->virt.x
	    || LIBGGI_GC(vis)->clipbr.y < LIBGGI_MODE(vis)->virt.x) {
		int y;
		y = LIBGGI_GC(vis)->cliptl.y;
		y = GGI_X_WRITE_Y;
		XClearArea(priv->disp, priv->drawable,
			   LIBGGI_GC(vis)->cliptl.x, y,
			   (unsigned)LIBGGI_GC(vis)->clipbr.x-LIBGGI_GC(vis)->cliptl.x,
			   (unsigned)LIBGGI_GC(vis)->clipbr.y-LIBGGI_GC(vis)->cliptl.y, 
			   False);
	} else {
		XClearWindow(priv->disp, priv->drawable);
	}
	GGI_X_MAYBE_SYNC(vis);
	ggUnlock(priv->xliblock);

	return 0;
}
