/* $Id: draw.c,v 1.4 2004/12/01 23:08:07 cegger Exp $
******************************************************************************

   Display-monotext: drawing operations

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

#include <ggi/display/monotext.h>


/* The following operations update the 'dirty region', which is used to
 * limit the area blit to the parent at ggiFlush() time.
 */

/* (!!! FIXME some of these are confused) */

int GGI_monotext_drawpixel_nc(ggi_visual *vis, int x, int y)
{
	ggi_monotext_priv *priv = MONOTEXT_PRIV(vis);
	int err;
	
	UPDATE_MOD(priv, x, y, 1, 1);
	
	if ((err = priv->mem_opdraw->drawpixel_nc(vis, x, y)) < 0) {
		return err;
	}

	UPDATE_SYNC;
	return 0;
}

int GGI_monotext_drawpixel(ggi_visual *vis, int x, int y)
{
	ggi_monotext_priv *priv = MONOTEXT_PRIV(vis);
	int err;
	
	UPDATE_MOD(priv, x, y, 1, 1);
	
	if ((err = priv->mem_opdraw->drawpixel(vis, x, y)) < 0) {
		return err;
	}

	UPDATE_SYNC;
	return 0;
}

int GGI_monotext_putpixel_nc(ggi_visual *vis, int x, int y, ggi_pixel col)
{ 
	ggi_monotext_priv *priv = MONOTEXT_PRIV(vis);
	int err;
	
	UPDATE_MOD(priv, x, y, 1, 1);
	
	if ((err = priv->mem_opdraw->putpixel_nc(vis, x, y, col)) < 0) {
		return err;
	}

	UPDATE_SYNC;
	return 0;
}

int GGI_monotext_putpixel(ggi_visual *vis, int x, int y, ggi_pixel col)
{ 
	ggi_monotext_priv *priv = MONOTEXT_PRIV(vis);
	int err;
	
	UPDATE_MOD(priv, x, y, 1, 1);
	
	if ((err = priv->mem_opdraw->putpixel(vis, x, y, col)) < 0) {
		return err;
	}

	UPDATE_SYNC;
	return 0;
}

int GGI_monotext_drawline(ggi_visual *vis, int x1, int y1, int x2, int y2)
{ 
	ggi_monotext_priv *priv = MONOTEXT_PRIV(vis);
	int err;

	int sx = MIN(x1, x2);
	int sy = MIN(y1, y2);

	int ex = MAX(x1, x2);
	int ey = MAX(y1, y2);

	UPDATE_MOD(priv, sx, sy, ex-sx, ey-sy);

	if ((err = priv->mem_opdraw->drawline(vis, x1, y1, x2, y2)) < 0) {
		return err;
	}

	UPDATE_SYNC;
	return 0;
}

int GGI_monotext_drawhline_nc(ggi_visual *vis, int x, int y, int w)
{
	ggi_monotext_priv *priv = MONOTEXT_PRIV(vis);
	int err;
	
	err = priv->mem_opdraw->drawhline_nc(vis, x, y, w);

	if (! err) {
		_ggi_monotextUpdate(vis, x, y, w, 1);
	}

	UPDATE_SYNC;
	return err;
}

int GGI_monotext_drawhline(ggi_visual *vis, int x, int y, int w)
{
	ggi_monotext_priv *priv = MONOTEXT_PRIV(vis);
	int err;
	
	err = priv->mem_opdraw->drawhline(vis, x, y, w);

	if (! err) {
		_ggi_monotextUpdate(vis, x, y, w, 1);
	}

	UPDATE_SYNC;
	return err;
}

int GGI_monotext_puthline(ggi_visual *vis, int x, int y, int w, const void *buffer)
{ 
	ggi_monotext_priv *priv = MONOTEXT_PRIV(vis);
	int err;
	
	err = priv->mem_opdraw->puthline(vis, x, y, w, buffer);
	
	if (! err) {
		_ggi_monotextUpdate(vis, x, y, w, 1);
	}

	UPDATE_SYNC;
	return err;
}

int GGI_monotext_drawvline_nc(ggi_visual *vis, int x, int y, int h)
{
	ggi_monotext_priv *priv = MONOTEXT_PRIV(vis);
	int err;
	
	UPDATE_MOD(priv, x, y, 1, h);
	
	if ((err = priv->mem_opdraw->drawvline_nc(vis, x, y, h)) < 0) {
		return err;
	}

	UPDATE_SYNC;
	return 0;
}

int GGI_monotext_drawvline(ggi_visual *vis, int x, int y, int h)
{
	ggi_monotext_priv *priv = MONOTEXT_PRIV(vis);
	int err;
	
	UPDATE_MOD(priv, x, y, 1, h);
	
	if ((err = priv->mem_opdraw->drawvline(vis, x, y, h)) < 0) {
		return err;
	}

	UPDATE_SYNC;
	return 0;
}

int GGI_monotext_putvline(ggi_visual *vis, int x, int y, int h, const void *buffer)
{
	ggi_monotext_priv *priv = MONOTEXT_PRIV(vis);
	int err;
	
	UPDATE_MOD(priv, x, y, 1, h);
	
	if ((err = priv->mem_opdraw->putvline(vis, x, y, h, buffer)) < 0) {
		return err;
	}

	UPDATE_SYNC;
	return 0;
}


/* ---------------------------------------------------------------------- */


/* The following operations DON'T update the 'dirty region', instead
 * they simply perform the blit to the parent.  Note: it is assumed that
 * the affected area is relatively large. 
 */
 
int GGI_monotext_putbox(ggi_visual *vis, int x, int y, int w, int h, const void *buffer)
{ 
	ggi_monotext_priv *priv = MONOTEXT_PRIV(vis);
	int err;
	
	if ((err = priv->mem_opdraw->putbox(vis, x, y, w, h, buffer)) < 0) {
		return err;
	}

	return _ggi_monotextUpdate(vis, x, y, w, h);
}

int GGI_monotext_drawbox(ggi_visual *vis, int x, int y, int w, int h)
{
	ggi_monotext_priv *priv = MONOTEXT_PRIV(vis);
	int err;
	
	if ((err = priv->mem_opdraw->drawbox(vis, x, y, w, h)) < 0) {
		return err;
	}

	return _ggi_monotextUpdate(vis, x, y, w, h);
}

int GGI_monotext_copybox(ggi_visual *vis, int x, int y, int w, int h, int nx, int ny)
{
	ggi_monotext_priv *priv = MONOTEXT_PRIV(vis);
	int err;
	
	if ((err = priv->mem_opdraw->copybox(vis, x, y, w, h, nx, ny)) < 0) {
		return err;
	}

	return _ggi_monotextUpdate(vis, nx, ny, w, h);
}

int GGI_monotext_crossblit(ggi_visual *src, int sx, int sy, int w, int h,
                 ggi_visual *vis, int dx, int dy)
{
	ggi_monotext_priv *priv = MONOTEXT_PRIV(vis);
	int err;
	
	if ((err = priv->mem_opdraw->crossblit(src, sx, sy, w, h, 
					   vis, dx, dy)) < 0) {
		return err;
	}

	return _ggi_monotextUpdate(vis, dx, dy, w, h);
}

int GGI_monotext_fillscreen(ggi_visual *vis)
{
	ggi_monotext_priv *priv = MONOTEXT_PRIV(vis);
	int err;
	
	if ((err = priv->mem_opdraw->fillscreen(vis)) < 0) {
		return err;
	}

	return _ggi_monotextUpdate(vis, 0, 0,
		LIBGGI_VIRTX(vis), LIBGGI_VIRTY(vis));
}


/* ---------------------------------------------------------------------- */


/* The following functions are just pass-throughs to the parent visual.
 */

int GGI_monotext_setorigin(ggi_visual *vis, int x, int y)
{
	ggi_monotext_priv *priv = MONOTEXT_PRIV(vis);

	int err;
	
	if ((err = ggiSetOrigin(priv->parent, x, y)) != 0)
		return err;

	vis->origin_x=x;
	vis->origin_y=y;
	
	return 0;
}
