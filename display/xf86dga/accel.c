/* $Id: accel.c,v 1.5 2004/09/12 21:01:40 cegger Exp $
******************************************************************************

   XF86DGA display target - acceleration

   Copyright (C) 1999      Marcus Sundberg	[marcus@ggi-project.org]

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
#include <ggi/display/xf86dga.h>


int GGI_xf86dga_drawbox(ggi_visual * vis, int x, int y, int w, int h)
{
	ggidga_priv *priv = DGA_PRIV(vis);
	int add = vis->w_frame_num * LIBGGI_VIRTY(vis);

	/* We can't draw outside the root window :-( */
	if ((unsigned) (y + add + h) > priv->height)
		return priv->drawbox(vis, x, y, w, h);

	y += add;

	_ggi_XF86DGAFillRectangle(priv->x.display, priv->x.screen,
				  DefaultRootWindow(priv->x.display),
				  priv->x.gc, x, y,
				  (unsigned) w, (unsigned) h);

	vis->accelactive = 1;

	DGA_DOSYNC(priv);

	return 0;
}

int GGI_xf86dga_copybox(ggi_visual * vis, int x, int y, int w, int h,
			int nx, int ny)
{
	ggidga_priv *priv = DGA_PRIV(vis);
	int add = vis->w_frame_num * LIBGGI_VIRTY(vis);

	y += vis->r_frame_num * LIBGGI_VIRTY(vis);

	/* We can't draw outside the root window :-( */
	if ((unsigned) (ny + add + h) > priv->height) {
		return priv->copybox(vis, x, y, w, h, nx, ny);
	}

	ny += add;

	_ggi_XF86DGACopyArea(priv->x.display, priv->x.screen,
			     DefaultRootWindow(priv->x.display),
			     priv->x.gc, x, y, (unsigned) w, (unsigned) h,
			     nx, ny);

	vis->accelactive = 1;

	DGA_DOSYNC(priv);

	return 0;
}
