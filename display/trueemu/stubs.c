/* $Id: stubs.c,v 1.1 2001/05/12 23:02:38 cegger Exp $
******************************************************************************

   Display-trueemu : stubs

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

#include <string.h>
#include <ggi/internal/ggi-dl.h>

#include <ggi/display/trueemu.h>


/* The following operations update the 'dirty region', which is used to
 * limit the area blit to the parent at ggiFlush() time.
 */

int GGI_trueemu_putpixel_nc(ggi_visual *vis, int x, int y, ggi_pixel col)
{ 
	ggi_trueemu_priv *priv = TRUEEMU_PRIV(vis);
	
	UPDATE_MOD(vis, x, y, 1, 1);
	
	return priv->mem_opdraw->putpixel_nc(vis, x, y, col);
}

int GGI_trueemu_putpixel(ggi_visual *vis, int x, int y, ggi_pixel col)
{ 
	ggi_trueemu_priv *priv = TRUEEMU_PRIV(vis);
	
	UPDATE_MOD(vis, x, y, 1, 1);
	
	return priv->mem_opdraw->putpixel(vis, x, y, col);
}

int GGI_trueemu_drawpixel_nc(ggi_visual *vis, int x, int y)
{
	ggi_trueemu_priv *priv = TRUEEMU_PRIV(vis);
	
	UPDATE_MOD(vis, x, y, 1, 1);
	
	return priv->mem_opdraw->drawpixel_nc(vis, x, y);
}

int GGI_trueemu_drawpixel(ggi_visual *vis, int x, int y)
{
	ggi_trueemu_priv *priv = TRUEEMU_PRIV(vis);
	
	UPDATE_MOD(vis, x, y, 1, 1);
	
	return priv->mem_opdraw->drawpixel(vis, x, y);
}

int GGI_trueemu_drawline(ggi_visual *vis, int x1, int y1, int x2, int y2)
{ 
	ggi_trueemu_priv *priv = TRUEEMU_PRIV(vis);

	int sx=MIN(x1,x2);
	int sy=MIN(y1,y2);

	int ex=MAX(x1,x2);
	int ey=MAX(y1,y2);

	UPDATE_MOD(vis, sx, sy, ex-sx, ey-sy);

	return priv->mem_opdraw->drawline(vis, x1, y1, x2, y2);
}

int GGI_trueemu_drawvline_nc(ggi_visual *vis, int x, int y, int h)
{
	ggi_trueemu_priv *priv = TRUEEMU_PRIV(vis);
	
	UPDATE_MOD(vis, x, y, 1, h);
	
	return priv->mem_opdraw->drawvline_nc(vis, x, y, h);
}

int GGI_trueemu_drawvline(ggi_visual *vis, int x, int y, int h)
{
	ggi_trueemu_priv *priv = TRUEEMU_PRIV(vis);
	
	UPDATE_MOD(vis, x, y, 1, h);
	
	return priv->mem_opdraw->drawvline(vis, x, y, h);
}

int GGI_trueemu_putvline(ggi_visual *vis, int x, int y, int h, void *buffer)
{
	ggi_trueemu_priv *priv = TRUEEMU_PRIV(vis);
	
	UPDATE_MOD(vis, x, y, 1, h);
	
	return priv->mem_opdraw->putvline(vis, x, y, h, buffer);
}

#undef putc

int GGI_trueemu_putc(ggi_visual *vis, int x, int y, char c)
{
	ggi_trueemu_priv *priv = TRUEEMU_PRIV(vis);

	int char_w;
	int char_h;
	
	ggiGetCharSize(vis, &char_w, &char_h);

	UPDATE_MOD(vis, x, y, char_w, char_h);
	
	return priv->mem_opdraw->putc(vis, x, y, c);
}


/* ---------------------------------------------------------------------- */


int GGI_trueemu_drawhline_nc(ggi_visual *vis, int x, int y, int w)
{
	ggi_trueemu_priv *priv = TRUEEMU_PRIV(vis);
	
	UPDATE_MOD(vis, x, y, w, 1);

	return priv->mem_opdraw->drawhline_nc(vis, x, y, w);
}

int GGI_trueemu_drawhline(ggi_visual *vis, int x, int y, int w)
{
	ggi_trueemu_priv *priv = TRUEEMU_PRIV(vis);
	
	UPDATE_MOD(vis, x, y, w, 1);

	return priv->mem_opdraw->drawhline(vis, x, y, w);
}

int GGI_trueemu_puthline(ggi_visual *vis, int x, int y, int w, void *buffer)
{ 
	ggi_trueemu_priv *priv = TRUEEMU_PRIV(vis);
	
	UPDATE_MOD(vis, x, y, w, 1);

	return priv->mem_opdraw->puthline(vis, x, y, w, buffer);
}

int GGI_trueemu_putbox(ggi_visual *vis, int x, int y, int w, int h, void *buffer)
{ 
	ggi_trueemu_priv *priv = TRUEEMU_PRIV(vis);
	
	UPDATE_MOD(vis, x, y, w, h);

	return priv->mem_opdraw->putbox(vis, x, y, w, h, buffer);
}

int GGI_trueemu_drawbox(ggi_visual *vis, int x, int y, int w, int h)
{
	ggi_trueemu_priv *priv = TRUEEMU_PRIV(vis);
	
	UPDATE_MOD(vis, x, y, w, h);

	return priv->mem_opdraw->drawbox(vis, x, y, w, h);
}

int GGI_trueemu_fillscreen(ggi_visual *vis)
{
	ggi_trueemu_priv *priv = TRUEEMU_PRIV(vis);
	
	UPDATE_MOD(vis, 0, 0, LIBGGI_VIRTX(vis), LIBGGI_VIRTY(vis));

	return priv->mem_opdraw->fillscreen(vis);
}


/* ---------------------------------------------------------------------- */


int GGI_trueemu_copybox(ggi_visual *vis, int x, int y, int w, int h, 
			int nx, int ny)
{
	ggi_trueemu_priv *priv = TRUEEMU_PRIV(vis);
	
	UPDATE_MOD(vis, nx, ny, w, h);

	return priv->mem_opdraw->copybox(vis, x, y, w, h, nx, ny);
}

int GGI_trueemu_crossblit(ggi_visual *src, int sx, int sy, int w, int h,
                	  ggi_visual *vis, int dx, int dy)
{
	ggi_trueemu_priv *priv = TRUEEMU_PRIV(vis);
	
	UPDATE_MOD(vis, dx, dy, w, h);
	
	return priv->mem_opdraw->crossblit(src, sx, sy, w, h, vis, dx, dy);
}


/* ---------------------------------------------------------------------- */


#if 0
/* The following routine is treated specially.  We assume that copybox
 * on the parent will be faster than performing the conversion blit
 * (especially when the parent uses acceleration).
 */

int GGI_trueemu_copybox(ggi_visual *vis, int x, int y, int w, int h, int nx, int ny)
{
	ggi_trueemu_priv *priv = TRUEEMU_PRIV(vis);
	int err;
	
	if ((err = ggiCopyBox(priv->parent, x, y, w, h, nx, ny)) < 0) {
		return err;
	}

	if (! (LIBGGI_FLAGS(vis) & GGIFLAG_ASYNC)) {
		ggiFlush(priv->parent);
	}

	return priv->mem_opdraw->copybox(vis, x, y, w, h, nx, ny);
}
#endif


/* ---------------------------------------------------------------------- */


/* The following functions are just pass-throughs to the parent visual.
 */

int GGI_trueemu_setorigin(ggi_visual *vis, int x, int y)
{
	ggi_trueemu_priv *priv = TRUEEMU_PRIV(vis);

	int err;

	if ((err = ggiSetOrigin(priv->parent, x, y)) < 0) {
		return err;
	}

	vis->origin_x = x;
	vis->origin_y = y;
	
	return 0;
}

#if 0
int GGI_trueemu_getcharsize(ggi_visual *vis, int *width, int *height)
{
	ggi_trueemu_priv *priv = TRUEEMU_PRIV(vis);

	return ggiGetCharSize(priv->parent, width, height);
}
#endif
