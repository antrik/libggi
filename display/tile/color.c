/* $Id: color.c,v 1.8 2006/03/22 19:54:46 cegger Exp $
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
	return ggiMapColor(TILE_PRIV(vis)->vislist[0].vis->stem, col);
}

int GGI_tile_unmappixel(struct ggi_visual *vis,ggi_pixel pixel,ggi_color *col)
{
	return ggiUnmapPixel(TILE_PRIV(vis)->vislist[0].vis->stem, pixel, col);
}

int GGI_tile_setpalvec(struct ggi_visual *vis,int start,int len,const ggi_color *colormap)
{
	ggi_tile_priv *priv = TILE_PRIV(vis);
	int i;
	int rc;

	for (i = 0; i < priv->numvis; i++) {
		rc = ggiSetPalette(priv->vislist[i].vis->stem, start, len, colormap);
		if (rc < 0) return rc;
	}

	return 0;
}

int GGI_tile_getpalvec(struct ggi_visual *vis,int start,int len,ggi_color *colormap)
{
	return ggiGetPalette(TILE_PRIV(vis)->vislist[0].vis->stem, start, len, colormap);
}
