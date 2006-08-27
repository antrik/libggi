/* $Id: color.c,v 1.1 2006/08/27 11:45:15 pekberg Exp $
******************************************************************************

   display-vnc: color

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

#include <string.h>

#include <ggi/display/vnc.h>
#include <ggi/internal/internal.h>
#include <ggi/internal/ggi_debug.h>


void
GGI_vnc_gcchanged(struct ggi_visual *vis, int mask)
{
	ggi_vnc_priv *priv = VNC_PRIV(vis);
	ggi_gc *gc;

	gc = LIBGGI_GC(vis);

	if (mask & GGI_GCCHANGED_FG)
		LIBGGI_GC(priv->fb)->fg_color = gc->fg_color;

	if (mask & GGI_GCCHANGED_BG)
		LIBGGI_GC(priv->fb)->bg_color = gc->bg_color;

	if (mask & GGI_GCCHANGED_CLIP)
		ggiSetGCClipping(priv->fb->stem,
			gc->cliptl.x, gc->cliptl.y,
			gc->clipbr.x, gc->clipbr.y);

	if (priv->fb->opgc->gcchanged)
		priv->fb->opgc->gcchanged(priv->fb, mask);
}

int
GGI_vnc_setpalvec(struct ggi_visual *vis,
	int start, int len, const ggi_color *colormap)
{
	ggi_vnc_priv *priv = VNC_PRIV(vis);

	memcpy(LIBGGI_PAL(vis)->clut.data + start,
		colormap,
		len * sizeof(ggi_color));

	priv->palette_dirty = 1;

	return _ggiSetPalette(priv->fb, start, len, colormap);
}

int
GGI_vnc_getpalvec(struct ggi_visual *vis,
	int start, int len, ggi_color *colormap)
{
	ggi_vnc_priv *priv = VNC_PRIV(vis);

	return _ggiGetPalette(priv->fb, start, len, colormap);
}

ggi_pixel
GGI_vnc_mapcolor(struct ggi_visual *vis, const ggi_color *col)
{
	ggi_vnc_priv *priv = VNC_PRIV(vis);

	return _ggiMapColor(priv->fb, col);
}

int
GGI_vnc_unmappixel(struct ggi_visual *vis, ggi_pixel pixel, ggi_color *col)
{
	ggi_vnc_priv *priv = VNC_PRIV(vis);

	return _ggiUnmapPixel(priv->fb, pixel, col);
}

int
GGI_vnc_packcolors(struct ggi_visual *vis,
	void *buf, const ggi_color *cols, int len)
{
	ggi_vnc_priv *priv = VNC_PRIV(vis);

	return _ggiPackColors(priv->fb, buf, cols, len);
}

int
GGI_vnc_unpackpixels(struct ggi_visual *vis,
	const void *buf, ggi_color *cols, int len)
{
	ggi_vnc_priv *priv = VNC_PRIV(vis);

	return _ggiUnpackPixels(priv->fb, buf, cols, len);
}

int
GGI_vnc_getgamma(struct ggi_visual *vis,
	ggi_float *r, ggi_float *g, ggi_float *b)
{
	ggi_vnc_priv *priv = VNC_PRIV(vis);

	return _ggiGetGamma(priv->fb, r, g, b);
}

int
GGI_vnc_setgamma(struct ggi_visual *vis,
	ggi_float r, ggi_float g, ggi_float b)
{
	ggi_vnc_priv *priv = VNC_PRIV(vis);

	return _ggiSetGamma(priv->fb, r, g, b);
}

int
GGI_vnc_getgammamap(struct ggi_visual *vis,
	int s, int len, ggi_color *gammamap)
{
	ggi_vnc_priv *priv = VNC_PRIV(vis);

	return _ggiGetGammaMap(priv->fb, s, len, gammamap);
}

int
GGI_vnc_setgammamap(struct ggi_visual *vis,
	int s, int len, const ggi_color *gammamap)
{
	ggi_vnc_priv *priv = VNC_PRIV(vis);

	return _ggiSetGammaMap(priv->fb, s, len, gammamap);
}
