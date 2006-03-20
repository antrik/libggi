/* $Id: line.c,v 1.5 2006/03/20 14:12:14 pekberg Exp $
******************************************************************************

   Graphics library for GGI.  Arbitrary lines for display-X

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

#include "../../default/common/clip.c"
#include <ggi/display/x.h>

int GGI_X_drawline_slave(struct ggi_visual *vis,int x1, int y1, int x2, int y2)
{
	ggi_x_priv *priv;
	int dummy;
	priv = GGIX_PRIV(vis);

	LIB_ASSERT(priv->slave->opdraw->drawline != NULL, "Null pointer bug");

        priv->slave->opdraw->drawline(priv->slave, x1, y1, x2, y2);
	if (!_ggi_clip2d(vis,&x1,&y1,&x2,&y2,&dummy,&dummy)) return GGI_OK;
        GGI_X_DIRTY(vis, x1, y1, x2-x1, y2-y1);
	return 0;
}

int GGI_X_drawline_slave_draw(struct ggi_visual *vis, int x1, int y1, int x2, int y2)
{
	ggi_x_priv *priv;
	priv = GGIX_PRIV(vis);

	LIB_ASSERT(priv->slave->opdraw->drawline != NULL, "Null pointer bug");

        priv->slave->opdraw->drawline(priv->slave, x1, y1, x2, y2);
	y1 = (y1 + LIBGGI_VIRTY(vis) * vis->w_frame_num);
	y2 = (y2 + LIBGGI_VIRTY(vis) * vis->w_frame_num);
	GGI_X_LOCK_XLIB(vis);
	XDrawLine(priv->disp, priv->drawable, priv->gc,
		  x1, y1, x2, y2);
	GGI_X_MAYBE_SYNC(vis);
	GGI_X_UNLOCK_XLIB(vis);
	return 0;
}

int GGI_X_drawline_draw(struct ggi_visual *vis,int x1,int y1,int x2,int y2)
{
	ggi_x_priv *priv;
	priv = GGIX_PRIV(vis);

	y1 = (y1 + LIBGGI_VIRTY(vis) * vis->w_frame_num);
	y2 = (y2 + LIBGGI_VIRTY(vis) * vis->w_frame_num);
	XDrawLine(priv->disp, priv->drawable, priv->gc,
		  x1, y1, x2, y2);

	GGI_X_MAYBE_SYNC(vis);
	return 0;
}
