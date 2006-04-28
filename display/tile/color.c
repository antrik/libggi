/* $Id: color.c,v 1.10 2006/04/28 06:05:37 cegger Exp $
******************************************************************************

   Tile target: color management

   Copyright (C) 1998 Steve Cheng    [steve@ggi-project.org]

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
#include <ggi/display/tile.h>

ggi_pixel GGI_tile_mapcolor(struct ggi_visual *vis, const ggi_color *col)
{
	ggi_tile_priv *priv = TILE_PRIV(vis);
	struct multi_vis *first = tile_FIRST(priv);

	return ggiMapColor(first->vis, col);
}

int GGI_tile_unmappixel(struct ggi_visual *vis,ggi_pixel pixel,ggi_color *col)
{
	ggi_tile_priv *priv = TILE_PRIV(vis);
	struct multi_vis *first = tile_FIRST(priv);

	return ggiUnmapPixel(first->vis, pixel, col);
}

int GGI_tile_setpalvec(struct ggi_visual *vis,int start,int len,const ggi_color *colormap)
{
	ggi_tile_priv *priv = TILE_PRIV(vis);
	struct multi_vis *elm;
	int rc;

	tile_FOREACH(priv, elm) {
		rc = ggiSetPalette(elm->vis, start, len, colormap);
		if (rc < 0) return rc;
	}

	return 0;
}

int GGI_tile_getpalvec(struct ggi_visual *vis,int start,int len,ggi_color *colormap)
{
	ggi_tile_priv *priv = TILE_PRIV(vis);
	struct multi_vis *first = tile_FIRST(priv);

	return ggiGetPalette(first->vis, start, len, colormap);
}
