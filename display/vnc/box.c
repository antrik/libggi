/* $Id: box.c,v 1.1 2006/08/27 11:45:14 pekberg Exp $
******************************************************************************

   display-vnc: boxes

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
#include <ggi/internal/internal.h>
#include <ggi/internal/ggi_debug.h>

int
GGI_vnc_drawbox(struct ggi_visual *vis, int x, int y, int w, int h)
{
	ggi_vnc_priv *priv = VNC_PRIV(vis);

	int res = _ggiDrawBox(priv->fb, x, y, w, h);
	GGI_vnc_invalidate_xyxy(vis, x, y, x + w, y + h);

	return res;
}


int
GGI_vnc_putbox(struct ggi_visual *vis,
	int x, int y, int w, int h, const void *data)
{
	ggi_vnc_priv *priv = VNC_PRIV(vis);

	int res = _ggiPutBox(priv->fb, x, y, w, h, data);
	GGI_vnc_invalidate_xyxy(vis, x, y, x + w, y + h);

	return res;
}

int
GGI_vnc_getbox(struct ggi_visual *vis,
	int x, int y, int w, int h, void *data)
{
	ggi_vnc_priv *priv = VNC_PRIV(vis);

	return _ggiGetBox(priv->fb, x, y, w, h, data);
}

int
GGI_vnc_copybox(struct ggi_visual *vis,
	int x, int y, int w, int h, int nx, int ny)
{
	ggi_vnc_priv *priv = VNC_PRIV(vis);

	int res = _ggiCopyBox(priv->fb, x, y, w, h, nx, ny);
	GGI_vnc_invalidate_xyxy(vis, nx, ny, nx + w, ny + h);

	return res;
}

int
GGI_vnc_crossblit(struct ggi_visual *src, int sx, int sy, int w, int h,
	struct ggi_visual *vis, int dx, int dy)
{
	ggi_vnc_priv *priv = VNC_PRIV(vis);

	int res = _ggiCrossBlit(src, sx, sy, w, h, priv->fb, dx, dy);
	GGI_vnc_invalidate_xyxy(vis, dx, dy, dx + w, dy + h);

	return res;
}

int
GGI_vnc_fillscreen(struct ggi_visual *vis)
{
	ggi_vnc_priv *priv = VNC_PRIV(vis);

	int res = _ggiFillscreen(priv->fb);
	GGI_vnc_invalidate_xyxy(vis,
		0, 0, LIBGGI_MODE(vis)->virt.x, LIBGGI_MODE(vis)->virt.y);

	return res;
}
