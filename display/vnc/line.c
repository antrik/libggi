/* $Id: line.c,v 1.2 2007/03/05 19:49:59 cegger Exp $
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
GGI_vnc_drawline(struct ggi_visual *vis, int x, int y, int xe, int ye)
{
	ggi_vnc_priv *priv = VNC_PRIV(vis);

	int res = _ggiDrawLine(priv->fb, x, y, xe, ye);
	if (x <= xe) {
		if (y <= ye)
			GGI_vnc_invalidate_xyxy(vis, x, y, xe + 1, ye + 1);
		else
			GGI_vnc_invalidate_xyxy(vis, x, ye, xe + 1, y + 1);
	}
	else {
		if (y <= ye)
			GGI_vnc_invalidate_xyxy(vis, xe, y, x + 1, ye + 1);
		else
			GGI_vnc_invalidate_xyxy(vis, xe, ye, x + 1, y + 1);
	}

	return res;
}

int
GGI_vnc_drawhline(struct ggi_visual *vis, int x, int y, int w)
{
	ggi_vnc_priv *priv = VNC_PRIV(vis);

	int res = _ggiDrawHLine(priv->fb, x, y, w);
	GGI_vnc_invalidate_xyxy(vis, x, y, x + w, y + 1);

	return res;
}

int
GGI_vnc_drawhline_nc(struct ggi_visual *vis, int x, int y, int w)
{
	ggi_vnc_priv *priv = VNC_PRIV(vis);

	int res = _ggiDrawHLineNC(priv->fb, x, y, w);
	GGI_vnc_invalidate_xyxy(vis, x, y, x + w, y + 1);

	return res;
}

int
GGI_vnc_puthline(struct ggi_visual *vis, int x, int y, int w, const void *buf)
{
	ggi_vnc_priv *priv = VNC_PRIV(vis);

	int res = _ggiPutHLine(priv->fb, x, y, w, buf);
	GGI_vnc_invalidate_xyxy(vis, x, y, x + w, y + 1);

	return res;
}

int
GGI_vnc_gethline(struct ggi_visual *vis, int x, int y, int w, void *buf)
{
	ggi_vnc_priv *priv = VNC_PRIV(vis);

	return _ggiGetHLine(priv->fb, x, y, w, buf);
}

int
GGI_vnc_drawvline(struct ggi_visual *vis, int x, int y, int h)
{
	ggi_vnc_priv *priv = VNC_PRIV(vis);

	int res = _ggiDrawVLine(priv->fb, x, y, h);
	GGI_vnc_invalidate_xyxy(vis, x, y, x + 1, y + h);

	return res;
}

int
GGI_vnc_drawvline_nc(struct ggi_visual *vis, int x, int y, int h)
{
	ggi_vnc_priv *priv = VNC_PRIV(vis);

	int res = _ggiDrawVLineNC(priv->fb, x, y, h);
	GGI_vnc_invalidate_xyxy(vis, x, y, x + 1, y + h);

	return res;
}

int
GGI_vnc_putvline(struct ggi_visual *vis, int x, int y, int h, const void *buf)
{
	ggi_vnc_priv *priv = VNC_PRIV(vis);

	int res = _ggiPutVLine(priv->fb, x, y, h, buf);
	GGI_vnc_invalidate_xyxy(vis, x, y, x + 1, y + h);

	return res;
}

int
GGI_vnc_getvline(struct ggi_visual *vis, int x, int y, int h, void *buf)
{
	ggi_vnc_priv *priv = VNC_PRIV(vis);

	return _ggiGetVLine(priv->fb, x, y, h, buf);
}
