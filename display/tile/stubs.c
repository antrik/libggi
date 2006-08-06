/* $Id: stubs.c,v 1.14 2006/08/06 06:00:55 pekberg Exp $
******************************************************************************

   Code stolen from the graphics library for GGI.

   Seriously, these functions split the *Box, *HLine and *VLine operations
   into the component operations to the underlying "tile" visuals.  However,
   if using DirectBuffer, operations are done to the linear framebuffer by
   generic-linear-*, instead of using these functions.

   Copyright (C) 1998 Steve Cheng	[steve@ggi-project.org]
   Copyright (C) 2000 Marcus Sundberg	[marcus@ggi-project.org]

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
#include <stdlib.h>
#include <ggi/display/tile.h>

int GGI_tile_flush(struct ggi_visual *vis, int x, int y, int w, int h, int tryflag)
{
	ggi_tile_priv *priv = TILE_PRIV(vis);
	struct multi_vis *elm;

	tile_FOREACH(priv, elm) {
		ggiFlushRegion(elm->vis, x, y, w, h);
	}

	return 0;
}


/* Hack: Copy GC changes to each child visual. */

#ifndef max
#define max(x,y) ((x)>(y)?(x):(y))
#endif
#ifndef min
#define min(x,y) ((x)<(y)?(x):(y))
#endif

void GGI_tile_gcchanged(struct ggi_visual *vis, int mask)
{
	ggi_tile_priv *priv = TILE_PRIV(vis);
	struct multi_vis *elm;
	struct ggi_visual *currvis;
	ggi_gc *gc;

	/* Irrelevant. */
	if ((priv->use_db) && (mask & GGI_GCCHANGED_CLIP))
		mask &= ~GGI_GCCHANGED_CLIP;

	gc = LIBGGI_GC(vis);
	tile_FOREACH(priv, elm) {
		currvis = GGI_VISUAL(elm->vis);

#if 0	/* Don't blindly copy the GC. */
		if (!priv->multi_mode) {
			gc = LIBGGI_GC(currvis);
		}

		memcpy(LIBGGI_GC(currvis), gc, sizeof(ggi_gc));
#else
		if (mask & GGI_GCCHANGED_FG)
			LIBGGI_GC(currvis)->fg_color = gc->fg_color;

		if (mask & GGI_GCCHANGED_BG)
			LIBGGI_GC(currvis)->bg_color = gc->bg_color;

		if (mask & GGI_GCCHANGED_CLIP) {
			if (priv->multi_mode)
				ggiSetGCClipping(currvis->stem,
					gc->cliptl.x, gc->cliptl.y,
					gc->clipbr.x, gc->clipbr.y);
			else
				ggiSetGCClipping(currvis->stem,
					max(elm->origin.x, gc->cliptl.x) - elm->origin.x,
					max(elm->origin.y, gc->cliptl.y) - elm->origin.y,
					min(elm->clipbr.x, gc->clipbr.x) - elm->origin.x,
					min(elm->clipbr.y, gc->clipbr.y) - elm->origin.y);
		}

		gc->version++;
#endif

		if ((!priv->multi_mode) && (currvis->opgc->gcchanged))
			currvis->opgc->gcchanged(currvis, mask);
	}
}


/**********************/
/* draw/get/put a box */
/**********************/

int GGI_tile_drawbox(struct ggi_visual *vis, int _x, int _y, int _width, int _length)
{
	ggi_tile_priv *priv = TILE_PRIV(vis);
	struct multi_vis *elm;
	ggi_coord cliptl, clipbr;
	int x, y, width, length, diff;

	tile_FOREACH(priv, elm) {
		cliptl = elm->origin;
		clipbr = elm->clipbr;
		x = _x;
		y = _y;
		width = _width;
		length = _length;

		if (y < cliptl.y) {
			diff = cliptl.y - y;
			y     +=diff;
			length-=diff;
		}

		if (y + length > clipbr.y)
			length = clipbr.y - y;

		if (x < cliptl.x) {
			diff = cliptl.x - x;
			x     +=diff;
			width-=diff;
		}

		if (x + width > clipbr.x)
			width = clipbr.x - x;

		if (length <= 0 || width <= 0 )
			continue;

		ggiDrawBox(elm->vis,
			x - cliptl.x, y - cliptl.y, width, length);
	}

	return 0;
}

int GGI_tile_putbox(struct ggi_visual *vis, int _x, int _y, int _width, int _length, const void *buffer)
{
	ggi_tile_priv *priv = TILE_PRIV(vis);
	struct multi_vis *elm;
	int rowadd = (LIBGGI_PIXFMT(vis)->size+7)/8;
	ggi_coord cliptl, clipbr;
	int x, y, width, length, diff;

	tile_FOREACH(priv, elm) {
		cliptl = elm->origin;
		clipbr = elm->clipbr;
		x = _x;
		y = _y;
		width = _width;
		length = _length;

		if (y < cliptl.y) {
			diff = cliptl.y - y;
			y     +=diff;
			length-=diff;
		}

		if (y + length > clipbr.y)
			length = clipbr.y - y;

		if (x < cliptl.x) {
			diff = cliptl.x - x;
			x     +=diff;
			width-=diff;
		}

		if (x + width > clipbr.x)
			width = clipbr.x - x;

		if (length <= 0 || width <= 0 )
			continue;

		while(length--) {
			ggiPutHLine(elm->vis,
				x - cliptl.x, y - cliptl.y + length, width,
				((const uint8_t*)buffer + rowadd*_width * (y-_y+length) + rowadd * (x-_x)));
		}
	}

	return 0;
}

int GGI_tile_getbox(struct ggi_visual *vis, int _x, int _y, int _width, int _length, void *buffer)
{
	ggi_tile_priv *priv = TILE_PRIV(vis);
	struct multi_vis *elm;
	int rowadd = (LIBGGI_PIXFMT(vis)->size+7)/8;
	ggi_coord cliptl, clipbr;
	int x, y, width, length, diff;

	tile_FOREACH(priv, elm) {
		cliptl = elm->origin;
		clipbr = elm->clipbr;
		x = _x;
		y = _y;
		width = _width;
		length = _length;

		if (y < cliptl.y) {
			diff = cliptl.y - y;
			y     +=diff;
			length-=diff;
		}

		if (y + length > clipbr.y)
			length = clipbr.y - y;

		if (x < cliptl.x) {
			diff = cliptl.x - x;
			x     +=diff;
			width-=diff;
		}

		if (x + width > clipbr.x)
			width = clipbr.x - x;

		if (length <= 0 || width <= 0 )
			continue;

		while(length--) {
			ggiGetHLine(elm->vis,
				x - cliptl.x, y - cliptl.y + length, width,
				((uint8_t*)buffer + rowadd*_width*(y-_y+length) + rowadd*(x-_x)));
		}
	}

	return 0;
}

int GGI_tile_copybox(struct ggi_visual *vis, int x, int y, int width, int height,
		     int nx,int ny)
{
	ggi_tile_priv *priv = TILE_PRIV(vis);
	struct multi_vis *elm;
	ggi_coord cliptl, clipbr;
	char *buf;

	/* We check if both the source and destination of the copy are wholely
	   contained in one of the tile visuals.
	*/
	tile_FOREACH(priv, elm) {
		cliptl = elm->origin;
		clipbr = elm->clipbr;

		if (x < cliptl.x || y < cliptl.y ||
		    x + width > clipbr.x || y + height > clipbr.y ||
		    nx < cliptl.x || ny < cliptl.y ||
		    nx + width > clipbr.x || ny + height > clipbr.y) {
			continue;
		}

		return ggiCopyBox(elm->vis,
				  x - cliptl.x, y - cliptl.y, width, height,
				  nx - cliptl.x, ny - cliptl.y);
	}

	/* ARGGH!!! */
	buf = malloc((size_t)((LIBGGI_PIXFMT(vis)->size+7)/8*width*height));
	if (buf == NULL) {
		/* Tough luck kid... */
		return GGI_ENOMEM;
	}

	ggiGetBox(vis->stem, x, y, width, height, buf);
	ggiPutBox(vis->stem, nx, ny, width, height, buf);

	free(buf);

	return 0;
}

int GGI_tile_fillscreen(struct ggi_visual *vis)
{
	ggi_tile_priv *priv = TILE_PRIV(vis);
	struct multi_vis *elm;

	tile_FOREACH(priv, elm) {
		ggiFillscreen(elm->vis);
	}

	return 0;
}

#if 0
int GGI_tile_putc(struct ggi_visual *vis,int x,int y,char c)
{
	int err=EOK;
	ggi_tile_priv *priv = TILE_PRIV(vis);
	struct multi_vis *elm;

	tile_FOREACH(priv, elm) {
		err = ggiPutc(priv->vislist[i].vis,x,y,c);
	}

	return err;
}


int GGI_tile_puts(struct ggi_visual *vis,int x,int y,const char *str)
{
	int err=EOK;
	int i;
	ggi_tile_priv *priv = TILE_PRIV(vis);

	for (i = 0; i < priv->numvis; i++)
		err = ggiPuts(priv->vislist[i],x,y,str);

	return err;
}

int GGI_tile_drawline(struct ggi_visual *vis,int x1,int y1,int x2,int y2)
{
	int err=EOK;
	int i;
	ggi_tile_priv *priv = TILE_PRIV(vis);

	for (i = 0; i < priv->numvis; i++)
		err = ggiDrawLine(priv->vislist[i],x1,y1,x2,y2);

	return err;
}
#endif

int GGI_tile_drawhline_nc(struct ggi_visual *vis,int _x,int y,int _width)
{
	ggi_tile_priv *priv = TILE_PRIV(vis);
	struct multi_vis *elm;
	ggi_coord cliptl, clipbr;
	int x, width, diff;

	tile_FOREACH(priv, elm) {
		cliptl = elm->origin;
		clipbr = elm->clipbr;
		x = _x;
		width = _width;

		if (y < cliptl.y || y >= clipbr.y)
			continue;

		if (x < cliptl.x) {
			diff = cliptl.x - x;
			x     +=diff;
			width-=diff;
		}

		if (x + width > clipbr.x)
			width = clipbr.x - x;

		if (width <= 0)
			continue;

		_ggiDrawHLineNC(GGI_VISUAL(elm->vis),
			x - cliptl.x, y - cliptl.y, width);
	}

	return 0;
}

int GGI_tile_drawhline(struct ggi_visual *vis,int x,int y,int w)
{
	/* Clipping */
	if (y<(LIBGGI_GC(vis)->cliptl.y) || y>=(LIBGGI_GC(vis)->clipbr.y)) return 0;
	if (x< (LIBGGI_GC(vis)->cliptl.x)) {
		int diff=(LIBGGI_GC(vis)->cliptl.x)-x;
		x+=diff;
		w-=diff;
	}
	if (x+w>(LIBGGI_GC(vis)->clipbr.x)) {
		w=(LIBGGI_GC(vis)->clipbr.x)-x;
	}
	if (w>0)
		return GGI_tile_drawhline_nc(vis,x,y,w);
	else
		return 0;	/* ??? Shouldn't this be an error? */
}

int GGI_tile_puthline(struct ggi_visual *vis,int _x,int y,int _width,const void *buffer)
{
	ggi_tile_priv *priv = TILE_PRIV(vis);
	struct multi_vis *elm;
	int rowadd = (LIBGGI_PIXFMT(vis)->size+7)/8;
	ggi_coord cliptl, clipbr;
	int x, width, diff;

	tile_FOREACH(priv, elm) {
		cliptl = elm->origin;
		clipbr = elm->clipbr;
		x = _x;
		width = _width;

		if (y < cliptl.y || y >= clipbr.y)
			continue;

		if (x < cliptl.x) {
			diff = cliptl.x - x;
			x     +=diff;
			width-=diff;
		} else {
			diff = 0;
		}

		if (x + width > clipbr.x)
			width = clipbr.x - x;

		if (width <= 0)
			continue;

		ggiPutHLine(elm->vis,
			x - cliptl.x, y - cliptl.y, width,
			((const uint8_t*)buffer + diff * rowadd));
	}

	return 0;
}

int GGI_tile_gethline(struct ggi_visual *vis,int _x,int y,int _width,void *buffer)
{
	ggi_tile_priv *priv = TILE_PRIV(vis);
	struct multi_vis *elm;
	int rowadd = (LIBGGI_PIXFMT(vis)->size+7)/8;
	ggi_coord cliptl, clipbr;
	int x, width, diff;

	tile_FOREACH(priv, elm) {
		cliptl = elm->origin;
		clipbr = elm->clipbr;
		x = _x;
		width = _width;

		if (y < cliptl.y || y >= clipbr.y)
			continue;

		if (x < cliptl.x) {
			diff = cliptl.x - x;
			x     +=diff;
			width-=diff;
		} else {
			diff = 0;
		}

		if (x + width > clipbr.x)
			width = clipbr.x - x;

		if (width <= 0)
			continue;

		ggiGetHLine(elm->vis,
			x - cliptl.x, y - cliptl.y, width,
			((uint8_t*)buffer + diff * rowadd));
	}

	return 0;
}

int GGI_tile_drawvline_nc(struct ggi_visual *vis,int x,int _y,int _height)
{
	ggi_tile_priv *priv = TILE_PRIV(vis);
	struct multi_vis *elm;
	ggi_coord cliptl, clipbr;
	int y, length, diff;

	tile_FOREACH(priv, elm) {
		cliptl = elm->origin;
		clipbr = elm->clipbr;
		y = _y;
		length = _height;

		if (x < cliptl.x || x >= clipbr.x)
			continue;

		if (y < cliptl.y) {
			diff = cliptl.y - y;
			y     +=diff;
			length-=diff;
		}

		if (y + length > clipbr.y)
			length = clipbr.y - y;

		if (length <= 0)
			continue;

		_ggiDrawVLineNC(GGI_VISUAL(elm->vis),
			x - cliptl.x, y - cliptl.y, length);
 	}

	return 0;
}

int GGI_tile_drawvline(struct ggi_visual *vis,int x,int y,int height)
{
	/* Clipping */
	if (x< (LIBGGI_GC(vis)->cliptl.x) ||
	    x>=(LIBGGI_GC(vis)->clipbr.x)) return 0;
	if (y< (LIBGGI_GC(vis)->cliptl.y)) {
		int diff=(LIBGGI_GC(vis)->cliptl.y)-y;
		y     +=diff;
		height-=diff;
	}
	if (y+height>(LIBGGI_GC(vis)->clipbr.y)) {
		height=(LIBGGI_GC(vis)->clipbr.y)-y;
	}

	if (height>0)
		return GGI_tile_drawvline_nc(vis,x,y,height);
	else
		return 0;	/* ??? Shouldn't this be an error? */
}

int GGI_tile_putvline(struct ggi_visual *vis,int x,int _y,int _height,const void *buffer)
{
	ggi_tile_priv *priv = TILE_PRIV(vis);
	struct multi_vis *elm;
	int rowadd = (LIBGGI_PIXFMT(vis)->size+7)/8;
	ggi_coord cliptl, clipbr;
	int y, length, diff;

	tile_FOREACH(priv, elm) {
		cliptl = elm->origin;
		clipbr = elm->clipbr;
		y = _y;
		length = _height;

		if (x < cliptl.x || x >= clipbr.x)
			continue;

		if (y < cliptl.y) {
			diff = cliptl.y - y;
			y     +=diff;
			length-=diff;
		} else {
			diff = 0;
		}

		if (y + length > clipbr.y)
			length = clipbr.y - y;

		if (length <= 0)
			continue;

		ggiPutVLine(elm->vis,
			x - cliptl.x, y - cliptl.y, length,
			((const uint8_t*)buffer + diff * rowadd));
 	}

	return 0;
}

int GGI_tile_getvline(struct ggi_visual *vis,int x,int _y,int _height,void *buffer)
{
	ggi_tile_priv *priv = TILE_PRIV(vis);
	struct multi_vis *elm;
	int rowadd = (LIBGGI_PIXFMT(vis)->size+7)/8;
	ggi_coord cliptl, clipbr;
	int y, length, diff;

	tile_FOREACH(priv, elm) {
		cliptl = elm->origin;
		clipbr = elm->clipbr;
		y = _y;
		length = _height;

		if (x < cliptl.x || x >= clipbr.x)
			continue;

		if (y < cliptl.y) {
			diff = cliptl.y - y;
			y     +=diff;
			length-=diff;
		} else {
			diff = 0;
		}

		if (y + length > clipbr.y)
			length = clipbr.y - y;

		if (length <= 0)
			continue;

		ggiGetVLine(elm->vis,
			x - cliptl.x, y - cliptl.y, length,
			((uint8_t*)buffer + diff * rowadd));
 	}

	return 0;
}

int GGI_tile_drawpixel_nc(struct ggi_visual *vis,int x,int y)
{
	ggi_tile_priv *priv = TILE_PRIV(vis);
	struct multi_vis *elm;
	ggi_coord cliptl, clipbr;

	tile_FOREACH(priv, elm) {
		cliptl = elm->origin;
		clipbr = elm->clipbr;

		if (x < cliptl.x || y < cliptl.y ||
			x >= clipbr.x || y >= clipbr.y)
			continue;

		/* Do we disallow overlapping tiles? */
		_ggiDrawPixelNC(GGI_VISUAL(elm->vis),
			x - cliptl.x, y - cliptl.y);
	}

	return 0;
}

int GGI_tile_drawpixel(struct ggi_visual *vis,int x,int y)
{
	CHECKXY(vis,x,y);
	return GGI_tile_drawpixel_nc(vis, x, y);
}

int GGI_tile_putpixel_nc(struct ggi_visual *vis, int x, int y, ggi_pixel col)
{
	ggi_tile_priv *priv = TILE_PRIV(vis);
	struct multi_vis *elm;
	ggi_coord cliptl, clipbr;

	tile_FOREACH(priv, elm) {
		cliptl = elm->origin;
		clipbr = elm->clipbr;

		if (x < cliptl.x || y < cliptl.y ||
			x >= clipbr.x || y >= clipbr.y)
			continue;

		/* Do we disallow overlapping tiles? */
		ggiPutPixel(elm->vis,
			x - cliptl.x, y - cliptl.y, col);
	}

	return 0;
}

int GGI_tile_putpixel(struct ggi_visual *vis, int x, int y, ggi_pixel col)
{
	CHECKXY(vis,x,y);
	return GGI_tile_putpixel_nc(vis, x, y, col);
}

int GGI_tile_getpixel(struct ggi_visual *vis, int x, int y, ggi_pixel *col)
{
	ggi_tile_priv *priv = TILE_PRIV(vis);
	struct multi_vis *elm;
	ggi_coord cliptl, clipbr;

	tile_FOREACH(priv, elm) {
		cliptl = elm->origin;
		clipbr = elm->clipbr;

		if (x < cliptl.x || y < cliptl.y ||
			x >= clipbr.x || y >= clipbr.y)
			continue;

		return ggiGetPixel(elm->vis,
			x - cliptl.x, y - cliptl.y, col);
	}

	return GGI_ENOSPACE;
}
