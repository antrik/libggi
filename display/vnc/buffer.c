/* $Id: buffer.c,v 1.2 2006/09/03 13:19:56 pekberg Exp $
******************************************************************************

   display-vnc: direct buffer

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
GGI_vnc_setorigin(struct ggi_visual *vis, int x, int y)
{
	if (x < 0 || y < 0)
		return GGI_EARGINVAL;
	if (x > LIBGGI_VIRTX(vis) - LIBGGI_X(vis))
		return GGI_EARGINVAL;
	if (y > LIBGGI_VIRTY(vis) - LIBGGI_Y(vis))
		return GGI_EARGINVAL;

	DPRINT("origin old %dx%d new %dx%d\n",
		vis->origin_x, vis->origin_y, x, y);

	vis->origin_x = x;
	vis->origin_y = y;

	GGI_vnc_invalidate_nc_xyxy(vis,
		vis->origin_x,
		vis->origin_y,
		vis->origin_x + LIBGGI_X(vis),
		vis->origin_y + LIBGGI_Y(vis));

	return GGI_OK;
}

int
GGI_vnc_setdisplayframe(struct ggi_visual *vis, int frameno)
{
	ggi_vnc_priv *priv = VNC_PRIV(vis);

	int res = _ggiSetDisplayFrame(priv->fb, frameno);
	vis->d_frame_num = _ggiGetDisplayFrame(priv->fb);

	return res;
}

int
GGI_vnc_setreadframe(struct ggi_visual *vis, int frameno)
{
	ggi_vnc_priv *priv = VNC_PRIV(vis);

	int res = _ggiSetReadFrame(priv->fb, frameno);
	vis->r_frame_num = _ggiGetReadFrame(priv->fb);

	return res;
}

int
GGI_vnc_setwriteframe(struct ggi_visual *vis, int frameno)
{
	ggi_vnc_priv *priv = VNC_PRIV(vis);

	int res = _ggiSetWriteFrame(priv->fb, frameno);
	vis->w_frame_num = _ggiGetWriteFrame(priv->fb);

	return res;
}
