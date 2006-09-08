/* $Id: draw.c,v 1.6 2006/09/08 22:26:14 cegger Exp $
******************************************************************************

   Display-palemu: draw

   Copyright (C) 1998 Andrew Apted    [andrew@ggi-project.org]

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
#include <ggi/display/palemu.h>


/* The following operations update the 'dirty region', which is used to
 * limit the area blitted to the parent at ggiFlush() time.
 */

int GGI_palemu_drawpixel_nc(struct ggi_visual *vis, int x, int y)
{
	ggi_palemu_priv *priv = PALEMU_PRIV(vis);
	
	UPDATE_MOD(vis, x, y, 1, 1);
	
	return priv->mem_opdraw->drawpixel_nc(vis, x, y);
}

int GGI_palemu_drawpixel(struct ggi_visual *vis, int x, int y)
{
	ggi_palemu_priv *priv = PALEMU_PRIV(vis);
	
	UPDATE_MOD(vis, x, y, 1, 1);
	
	return priv->mem_opdraw->drawpixel(vis, x, y);
}

int GGI_palemu_putpixel_nc(struct ggi_visual *vis, int x, int y, ggi_pixel col)
{ 
	ggi_palemu_priv *priv = PALEMU_PRIV(vis);
	
	UPDATE_MOD(vis, x, y, 1, 1);
	
	return priv->mem_opdraw->putpixel_nc(vis, x, y, col);
}

int GGI_palemu_putpixel(struct ggi_visual *vis, int x, int y, ggi_pixel col)
{ 
	ggi_palemu_priv *priv = PALEMU_PRIV(vis);
	
	UPDATE_MOD(vis, x, y, 1, 1);
	
	return priv->mem_opdraw->putpixel(vis, x, y, col);
}

int GGI_palemu_drawline(struct ggi_visual *vis, int x1, int y1, int x2, int y2)
{ 
	ggi_palemu_priv *priv = PALEMU_PRIV(vis);

	int sx = MIN(x1, x2);
	int sy = MIN(y1, y2);

	int ex = MAX(x1, x2);
	int ey = MAX(y1, y2);

	UPDATE_MOD(vis, sx, sy, ex-sx, ey-sy);

	return priv->mem_opdraw->drawline(vis, x1, y1, x2, y2);
}

int GGI_palemu_drawhline_nc(struct ggi_visual *vis, int x, int y, int w)
{
	ggi_palemu_priv *priv = PALEMU_PRIV(vis);
	
	UPDATE_MOD(vis, x, y, w, 1);

	return priv->mem_opdraw->drawhline_nc(vis, x, y, w);
}

int GGI_palemu_drawhline(struct ggi_visual *vis, int x, int y, int w)
{
	ggi_palemu_priv *priv = PALEMU_PRIV(vis);
	
	UPDATE_MOD(vis, x, y, w, 1);

	return priv->mem_opdraw->drawhline(vis, x, y, w);
}

int GGI_palemu_puthline(struct ggi_visual *vis, int x, int y, int w, const void *buffer)
{ 
	ggi_palemu_priv *priv = PALEMU_PRIV(vis);
	
	UPDATE_MOD(vis, x, y, w, 1);

	return priv->mem_opdraw->puthline(vis, x, y, w, buffer);
}

int GGI_palemu_drawvline_nc(struct ggi_visual *vis, int x, int y, int h)
{
	ggi_palemu_priv *priv = PALEMU_PRIV(vis);
	
	UPDATE_MOD(vis, x, y, 1, h);
	
	return priv->mem_opdraw->drawvline_nc(vis, x, y, h);
}

int GGI_palemu_drawvline(struct ggi_visual *vis, int x, int y, int h)
{
	ggi_palemu_priv *priv = PALEMU_PRIV(vis);
	
	UPDATE_MOD(vis, x, y, 1, h);
	
	return priv->mem_opdraw->drawvline(vis, x, y, h);
}

int GGI_palemu_putvline(struct ggi_visual *vis, int x, int y, int h, const void *buffer)
{
	ggi_palemu_priv *priv = PALEMU_PRIV(vis);
	
	UPDATE_MOD(vis, x, y, 1, h);
	
	return priv->mem_opdraw->putvline(vis, x, y, h, buffer);
}

int GGI_palemu_putbox(struct ggi_visual *vis, int x, int y, int w, int h, const void *buffer)
{ 
	ggi_palemu_priv *priv = PALEMU_PRIV(vis);
	
	UPDATE_MOD(vis, x, y, w, h);

	return priv->mem_opdraw->putbox(vis, x, y, w, h, buffer);
}

int GGI_palemu_drawbox(struct ggi_visual *vis, int x, int y, int w, int h)
{
	ggi_palemu_priv *priv = PALEMU_PRIV(vis);
	
	UPDATE_MOD(vis, x, y, w, h);

	return priv->mem_opdraw->drawbox(vis, x, y, w, h);
}

int GGI_palemu_fillscreen(struct ggi_visual *vis)
{
	ggi_palemu_priv *priv = PALEMU_PRIV(vis);
	
	UPDATE_MOD(vis, 0, 0, LIBGGI_VIRTX(vis), LIBGGI_VIRTY(vis));

	return priv->mem_opdraw->fillscreen(vis);
}


/* ---------------------------------------------------------------------- */


int GGI_palemu_copybox(struct ggi_visual *vis, int x, int y, int w, int h, int nx, int ny)
{
	ggi_palemu_priv *priv = PALEMU_PRIV(vis);
	
	UPDATE_MOD(vis, nx, ny, w, h);

	return priv->mem_opdraw->copybox(vis, x, y, w, h, nx, ny);
}

int GGI_palemu_crossblit(struct ggi_visual *src, int sx, int sy, int w, int h,
			 struct ggi_visual *vis, int dx, int dy)
{
	ggi_palemu_priv *priv = PALEMU_PRIV(vis);
	
	UPDATE_MOD(vis, dx, dy, w, h);

	return priv->mem_opdraw->crossblit(src, sx, sy, w, h, vis, dx, dy);
}


/* ---------------------------------------------------------------------- */


/* The following functions are just pass-throughs to the parent visual.
 */

int GGI_palemu_setorigin(struct ggi_visual *vis, int x, int y)
{
	ggi_palemu_priv *priv = PALEMU_PRIV(vis);

	int err;
	
	if ((err = ggiSetOrigin(priv->parent, x, y)) != 0) {
		return err;
	}

	vis->origin_x = x;
	vis->origin_y = y;
	
	return 0;
}
