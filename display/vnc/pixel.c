/* $Id: pixel.c,v 1.5 2008/01/20 19:26:44 pekberg Exp $
******************************************************************************

   display-vnc: pixel

   Copyright (C) 2006 Peter Rosin	[peda@lysator.liu.se]

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

#include <ggi/display/vnc.h>
#include <ggi/internal/ggi.h>
#include <ggi/internal/ggi_debug.h>


int
GGI_vnc_drawpixel(struct ggi_visual *vis, int x, int y)
{
	ggi_vnc_priv *priv = VNC_PRIV(vis);

	int res = _ggiDrawPixel(priv->fb, x, y);
	GGI_vnc_invalidate_xyxy(vis, x, y, x + 1, y + 1);

	return res;
}

int
GGI_vnc_drawpixel_nc(struct ggi_visual *vis, int x, int y)
{
	ggi_vnc_priv *priv = VNC_PRIV(vis);

	int res = _ggiDrawPixelNC(priv->fb, x, y);
	GGI_vnc_invalidate_xyxy(vis, x, y, x + 1, y + 1);

	return res;
}

int
GGI_vnc_putpixel(struct ggi_visual *vis, int x, int y, ggi_pixel col)
{
	ggi_vnc_priv *priv = VNC_PRIV(vis);

	int res = _ggiPutPixel(priv->fb, x, y, col);
	GGI_vnc_invalidate_xyxy(vis, x, y, x + 1, y + 1);

	return res;
}

int
GGI_vnc_putpixel_nc(struct ggi_visual *vis, int x, int y, ggi_pixel col)
{
	ggi_vnc_priv *priv = VNC_PRIV(vis);

	int res = _ggiPutPixelNC(priv->fb, x, y, col);
	GGI_vnc_invalidate_xyxy(vis, x, y, x + 1, y + 1);

	return res;
}

int
GGI_vnc_getpixel(struct ggi_visual *vis, int x, int y, ggi_pixel *col)
{
	ggi_vnc_priv *priv = VNC_PRIV(vis);

	return _ggiGetPixel(priv->fb, x, y, col);
}

int
GGI_vnc_getpixel_nc(struct ggi_visual *vis, int x, int y, ggi_pixel *col)
{
	ggi_vnc_priv *priv = VNC_PRIV(vis);

	return _ggiGetPixelNC(priv->fb, x, y, col);
}
