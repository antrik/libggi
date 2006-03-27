/* $Id: stubs.c,v 1.13 2006/03/27 21:02:31 cegger Exp $
******************************************************************************

   Display-multi: stubs

   Copyright (C) 1995 Andreas Beck	[becka@ggi-project.org]
   Copyright (C) 1997 Jason McMullan	[jmcc@ggi-project.org]
   Copyright (C) 1998 Andrew Apted	[andrew@ggi-project.org]
   Copyright (C) 1999 Marcus SUndberg	[marcus@ggi-project.org]

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
#include <ggi/display/multi.h>
#include <ggi/internal/ggi_debug.h>


void GGI_multi_gcchanged(struct ggi_visual *vis, int mask)
{
	ggi_multi_priv *priv = GGIMULTI_PRIV(vis);
	MultiVis *cur;

	DPRINT("display-multi: GCCHANGED %d\n", mask);

	GG_SLIST_FOREACH(cur, &priv->vis_list, visuals) {
		ggi_gc *gc = LIBGGI_GC(vis);

		if (mask & GGI_GCCHANGED_FG) {
			ggiSetGCForeground(cur->vis->stem, gc->fg_color);
		}
		
		if (mask & GGI_GCCHANGED_BG) {
			ggiSetGCBackground(cur->vis->stem, gc->bg_color);
		}

		if (mask & GGI_GCCHANGED_CLIP) {
			ggiSetGCClipping(cur->vis->stem, 
					 gc->cliptl.x, gc->cliptl.y,
					 gc->clipbr.x, gc->clipbr.y);
		}
	}
}

int GGI_multi_putpixel(struct ggi_visual *vis, int x, int y, ggi_pixel col)
{ 
	ggi_multi_priv *priv = GGIMULTI_PRIV(vis);
	MultiVis *cur;
	int err = 0;

	GG_SLIST_FOREACH(cur, &priv->vis_list, visuals) {
		if (LIBGGIPutPixel(cur->vis, x, y, col) != 0) err = -1;
	}

	return err;
}

int GGI_multi_drawpixel(struct ggi_visual *vis, int x, int y)
{
	ggi_multi_priv *priv = GGIMULTI_PRIV(vis);
	MultiVis *cur;
	int err = 0;

	GG_SLIST_FOREACH(cur, &priv->vis_list, visuals) {
		if (LIBGGIDrawPixel(cur->vis, x, y) != 0) err = -1;
	}

	return err;
}

int GGI_multi_drawbox(struct ggi_visual *vis, int x, int y, int w, int h)
{
	ggi_multi_priv *priv = GGIMULTI_PRIV(vis);
	MultiVis *cur;
	int err = 0;

	GG_SLIST_FOREACH(cur, &priv->vis_list, visuals) {
		if (ggiDrawBox(cur->vis->stem, x, y, w, h) != 0) err = -1;
	}

	return err;
}

int GGI_multi_puthline(struct ggi_visual *vis, int x, int y, int w, const void *buffer)
{ 
	ggi_multi_priv *priv = GGIMULTI_PRIV(vis);
	MultiVis *cur;
	int err = 0;

	GG_SLIST_FOREACH(cur, &priv->vis_list, visuals) {
		if (ggiPutHLine(cur->vis->stem, x, y, w, buffer) != 0) err = -1;
	}

	return err;
}

int GGI_multi_putvline(struct ggi_visual *vis, int x, int y, int h, const void *buffer)
{
	ggi_multi_priv *priv = GGIMULTI_PRIV(vis);
	MultiVis *cur;
	int err = 0;

	GG_SLIST_FOREACH(cur, &priv->vis_list, visuals) {
		if (ggiPutVLine(cur->vis->stem, x, y, h, buffer) != 0) err = -1;
	}

	return err;
}

int GGI_multi_putbox(struct ggi_visual *vis, int x, int y, int w, int h, const void *buffer)
{ 
	ggi_multi_priv *priv = GGIMULTI_PRIV(vis);
	MultiVis *cur;
	int err = 0;

	GG_SLIST_FOREACH(cur, &priv->vis_list, visuals) {
		if (ggiPutBox(cur->vis->stem, x, y, w, h, buffer) != 0) err = -1;
	}

	return err;
}

int GGI_multi_crossblit(struct ggi_visual *src, int sx, int sy, int w, int h,
			struct ggi_visual *dst, int dx, int dy)
{ 
	ggi_multi_priv *priv = GGIMULTI_PRIV(dst);
	MultiVis *cur;
	int err = 0;

	GG_SLIST_FOREACH(cur, &priv->vis_list, visuals) {
		if (ggiCrossBlit(src->stem, sx, sy, w, h, cur->vis->stem, dx, dy) != 0) {
			err = -1;
		}
	}

	return err;
}

int GGI_multi_fillscreen(struct ggi_visual *vis)
{
	ggi_multi_priv *priv = GGIMULTI_PRIV(vis);
	MultiVis *cur;
	int err = 0;

	GG_SLIST_FOREACH(cur, &priv->vis_list, visuals) {
		if (ggiFillscreen(cur->vis->stem) != 0) err = -1;
	}

	return err;
}

int GGI_multi_putc(struct ggi_visual *vis, int x, int y, char c)
{
	ggi_multi_priv *priv = GGIMULTI_PRIV(vis);
	MultiVis *cur;
	int err = 0;

	GG_SLIST_FOREACH(cur, &priv->vis_list, visuals) {
		if (ggiPutc(cur->vis->stem, x, y, c) != 0) err = -1;
	}

	return err;
}

int GGI_multi_puts(struct ggi_visual *vis, int x, int y, const char *str)
{
	ggi_multi_priv *priv = GGIMULTI_PRIV(vis);
	MultiVis *cur;
	int err = 0;

	GG_SLIST_FOREACH(cur, &priv->vis_list, visuals) {
		if (ggiPuts(cur->vis->stem, x, y, str) != 0) err = -1;
	}

	return err;
}

int GGI_multi_drawhline(struct ggi_visual *vis, int x, int y, int w)
{
	ggi_multi_priv *priv = GGIMULTI_PRIV(vis);
	MultiVis *cur;
	int err = 0;

	GG_SLIST_FOREACH(cur, &priv->vis_list, visuals) {
		if (ggiDrawHLine(cur->vis->stem, x, y, w) != 0) err = -1;
	}

	return err;
}

int GGI_multi_drawline(struct ggi_visual *vis, int x1, int y1, int x2, int y2)
{ 
	ggi_multi_priv *priv = GGIMULTI_PRIV(vis);
	MultiVis *cur;
	int err = 0;

	GG_SLIST_FOREACH(cur, &priv->vis_list, visuals) {
		if (ggiDrawLine(cur->vis->stem, x1, y1, x2, y2) != 0) err = -1;
	}

	return err;
}

int GGI_multi_drawvline(struct ggi_visual *vis, int x, int y, int h)
{
	ggi_multi_priv *priv = GGIMULTI_PRIV(vis);
	MultiVis *cur;
	int err = 0;

	GG_SLIST_FOREACH(cur, &priv->vis_list, visuals) {
		if (ggiDrawVLine(cur->vis->stem, x, y, h) != 0) err = -1;
	}

	return err;
}

int GGI_multi_copybox(struct ggi_visual *vis, int x, int y, int w, int h, int nx, int ny)
{
	ggi_multi_priv *priv = GGIMULTI_PRIV(vis);
	MultiVis *cur;
	int err = 0;

	GG_SLIST_FOREACH(cur, &priv->vis_list, visuals) {
		if (ggiCopyBox(cur->vis->stem, x, y, w, h, nx, ny) != 0) err = -1;
	}

	return err;
}

int GGI_multi_setgamma(struct ggi_visual *vis, ggi_float r, ggi_float g, ggi_float b)
{
	ggi_multi_priv *priv = GGIMULTI_PRIV(vis);
	MultiVis *cur;
	int err = 0;

	GG_SLIST_FOREACH(cur, &priv->vis_list, visuals) {
		if (ggiSetGamma(cur->vis->stem, r, g, b) != 0) err = -1;
	}

	return err;
} 

int GGI_multi_setgammamap(struct ggi_visual *vis, int start, int len, const ggi_color *cmap)
{
	ggi_multi_priv *priv = GGIMULTI_PRIV(vis);
	MultiVis *cur;
	int err = 0;

	GG_SLIST_FOREACH(cur, &priv->vis_list, visuals) {
		if (ggiSetGammaMap(cur->vis->stem, start, len, cmap) != 0) err = -1;
	}

	return err;
}


int GGI_multi_setpalvec(struct ggi_visual *vis, int start, int len, const ggi_color *cmap)
{
	ggi_multi_priv *priv = GGIMULTI_PRIV(vis);
	MultiVis *cur;
	int err = 0;

	GG_SLIST_FOREACH(cur, &priv->vis_list, visuals) {
		if (ggiSetPalette(cur->vis->stem, start, len, cmap) != 0) err = -1;
	}

	return err;
}


int GGI_multi_setorigin(struct ggi_visual *vis, int x, int y)
{
	ggi_multi_priv *priv = GGIMULTI_PRIV(vis);
	MultiVis *cur;
	int err = 0;

	GG_SLIST_FOREACH(cur, &priv->vis_list, visuals) {
		if (ggiSetOrigin(cur->vis->stem, x, y) != 0) err = -1;
	}

	if (! err) {
		vis->origin_x = x;
		vis->origin_y = y;
	}
	
	return err;
}


/* We just use the first visual for these ops
 */

int GGI_multi_getpixel(struct ggi_visual *vis, int x, int y, ggi_pixel *col)
{
	ggi_multi_priv *priv = GGIMULTI_PRIV(vis);
	struct ggi_visual *cvis = GG_SLIST_FIRST(&priv->vis_list)->vis;

	return LIBGGIGetPixel(cvis, x, y, col);
}


int GGI_multi_gethline(struct ggi_visual *vis, int x, int y, int w, void *buffer)
{
	ggi_multi_priv *priv = GGIMULTI_PRIV(vis);
	struct ggi_visual *cvis = GG_SLIST_FIRST(&priv->vis_list)->vis;

	return cvis->opdraw->gethline(cvis, x, y, w, buffer);
}


int GGI_multi_getvline(struct ggi_visual *vis, int x, int y, int h, void *buffer)
{
	ggi_multi_priv *priv = GGIMULTI_PRIV(vis);
	struct ggi_visual *cvis = GG_SLIST_FIRST(&priv->vis_list)->vis;

	return cvis->opdraw->getvline(cvis, x, y, h, buffer);
}


int GGI_multi_getbox(struct ggi_visual *vis, int x, int y, int w, int h, void *buffer)
{
	ggi_multi_priv *priv = GGIMULTI_PRIV(vis);
	struct ggi_visual *cvis = GG_SLIST_FIRST(&priv->vis_list)->vis;

	return cvis->opdraw->getbox(cvis, x, y, w, h, buffer);
}


int
GGI_multi_getgamma(struct ggi_visual *vis, ggi_float *r, ggi_float *g, ggi_float *b)
{
	ggi_multi_priv *priv = GGIMULTI_PRIV(vis);
	struct ggi_visual *cvis = GG_SLIST_FIRST(&priv->vis_list)->vis;

	return cvis->opcolor->getgamma(cvis, r, g, b);
}


int GGI_multi_getgammamap(struct ggi_visual *vis, int start, int len, ggi_color *cmap)
{
	ggi_multi_priv *priv = GGIMULTI_PRIV(vis);
	struct ggi_visual *cvis = GG_SLIST_FIRST(&priv->vis_list)->vis;

	return cvis->opcolor->getgammamap(cvis, start, len, cmap);
}


int GGI_multi_getcharsize(struct ggi_visual *vis, int *width, int *height)
{
	ggi_multi_priv *priv = GGIMULTI_PRIV(vis);
	struct ggi_visual *cvis = GG_SLIST_FIRST(&priv->vis_list)->vis;

	return cvis->opdraw->getcharsize(cvis, width, height);
}


ggi_pixel GGI_multi_mapcolor(struct ggi_visual *vis, const ggi_color *col)
{
	ggi_multi_priv *priv = GGIMULTI_PRIV(vis);
	struct ggi_visual *cvis = GG_SLIST_FIRST(&priv->vis_list)->vis;

	return LIBGGIMapColor(cvis, col);
}


int GGI_multi_unmappixel(struct ggi_visual *vis, ggi_pixel pixel, ggi_color *col)
{
	ggi_multi_priv *priv = GGIMULTI_PRIV(vis);
	struct ggi_visual *cvis = GG_SLIST_FIRST(&priv->vis_list)->vis;

	return LIBGGIUnmapPixel(cvis, pixel, col);
}


int GGI_multi_packcolors(struct ggi_visual *vis, void *buf, const ggi_color *cols, int len)
{
	ggi_multi_priv *priv = GGIMULTI_PRIV(vis);
	struct ggi_visual *cvis = GG_SLIST_FIRST(&priv->vis_list)->vis;

	return cvis->opcolor->packcolors(cvis, buf, cols, len);
}


int GGI_multi_unpackpixels(struct ggi_visual *vis, const void *buf, ggi_color *cols, int len)
{
	ggi_multi_priv *priv = GGIMULTI_PRIV(vis);
	struct ggi_visual *cvis = GG_SLIST_FIRST(&priv->vis_list)->vis;

	return cvis->opcolor->unpackpixels(cvis, buf, cols, len);
}


int GGI_multi_getpalvec(struct ggi_visual *vis, int start, int len, ggi_color *cmap)
{
	ggi_multi_priv *priv = GGIMULTI_PRIV(vis);
	struct ggi_visual *cvis = GG_SLIST_FIRST(&priv->vis_list)->vis;

	return cvis->opcolor->getpalvec(cvis, start, len, cmap);
}
