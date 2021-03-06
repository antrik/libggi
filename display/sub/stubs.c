/* $Id: stubs.c,v 1.14 2007/03/11 00:48:58 soyt Exp $
******************************************************************************

   Display-sub: stubs

   Copyright (C) 1998 Andreas Beck    [becka@ggi-project.org]
   Copyright (C) 1998 Rudolphe Ortalo

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

/* FIXME: Should we adjust parent clipping for "get" calls ? */

#include "config.h"
#include <ggi/display/sub.h>

#define IS_SAVING_GC	ggi_pixel _foreground, _background; \
			int _l,_t,_r,_b;

#define SETUP_AND_SAVE_GC \
	do {	_foreground = LIBGGI_GC(priv->parent)->fg_color; \
		_background = LIBGGI_GC(priv->parent)->bg_color; \
		_l = LIBGGI_GC(priv->parent)->cliptl.x; \
		_t = LIBGGI_GC(priv->parent)->cliptl.y; \
		_r = LIBGGI_GC(priv->parent)->clipbr.x; \
		_b = LIBGGI_GC(priv->parent)->clipbr.y; \
		LIBGGI_GC(priv->parent)->fg_color = LIBGGI_GC(vis)->fg_color; \
		LIBGGI_GC(priv->parent)->bg_color = LIBGGI_GC(vis)->bg_color; \
		LIBGGI_GC(priv->parent)->cliptl.x = LIBGGI_GC(vis)->cliptl.x + priv->position.x; \
	   	LIBGGI_GC(priv->parent)->cliptl.y = LIBGGI_GC(vis)->cliptl.y + priv->position.y; \
	   	LIBGGI_GC(priv->parent)->clipbr.x = LIBGGI_GC(vis)->clipbr.x + priv->position.x; \
	   	LIBGGI_GC(priv->parent)->clipbr.y = LIBGGI_GC(vis)->clipbr.y + priv->position.y; \
		/* check limits... (in case leaving visual) */ \
		if (LIBGGI_GC(priv->parent)->clipbr.x > priv->botright.x) \
			LIBGGI_GC(priv->parent)->clipbr.x = priv->botright.x; \
		if (LIBGGI_GC(priv->parent)->clipbr.y > priv->botright.y) \
			LIBGGI_GC(priv->parent)->clipbr.y = priv->botright.y; \
		/* ensure update in KGI */ \
		LIBGGI_GC(priv->parent)->version++; \
	} while (0);

#define RESTORE_GC \
	do {	LIBGGI_GC(priv->parent)->fg_color = _foreground; \
		LIBGGI_GC(priv->parent)->bg_color = _background; \
		LIBGGI_GC(priv->parent)->cliptl.x= _l; \
	   	LIBGGI_GC(priv->parent)->cliptl.y= _t; \
	   	LIBGGI_GC(priv->parent)->clipbr.x= _r; \
	   	LIBGGI_GC(priv->parent)->clipbr.y= _b; \
		/* ensure update in KGI */ \
		LIBGGI_GC(priv->parent)->version++; \
	} while (0);


/*
**	Functions undergoing a translation
**	with GC saving/restoring
*/

/* NB: Here the src is not directly translated... */
int GGI_sub_crossblit(struct ggi_visual *src, int sx, int sy, int w, int h,
		      struct ggi_visual *vis, int dx, int dy)
{
	ggi_sub_priv *priv = SUB_PRIV(vis); /* Destination is subvisual */
	int err;
	IS_SAVING_GC;

	SETUP_AND_SAVE_GC;
	err = ggiCrossBlit(src->instance.stem, sx, sy, w, h,
			   priv->parent->instance.stem,
			   dx + priv->position.x, dy + priv->position.y);
	RESTORE_GC;
	return err;
}

int GGI_sub_drawbox(struct ggi_visual *vis,int x,int y,int w,int h)
{
	ggi_sub_priv *priv = SUB_PRIV(vis);
	int err;
	IS_SAVING_GC;

	SETUP_AND_SAVE_GC;
	err = ggiDrawBox(priv->parent->instance.stem,
			 x + priv->position.x,
			 y + priv->position.y,
			 w, h);
	RESTORE_GC;
	return err;
}

int GGI_sub_putbox(struct ggi_visual *vis, int x, int y,
			int w, int h,
			const void *buf)
{ 
	ggi_sub_priv *priv = SUB_PRIV(vis);
	int err;
	IS_SAVING_GC;

	SETUP_AND_SAVE_GC;
	err = ggiPutBox(priv->parent->instance.stem,
			x + priv->position.x,
			y + priv->position.y,
			w, h, buf);
	RESTORE_GC;
	return err;
}

int GGI_sub_getbox(struct ggi_visual *vis,
			int x, int y,
			int w, int h,
			void *buf)
{ 
	ggi_sub_priv *priv = SUB_PRIV(vis);
	int err;
	IS_SAVING_GC;

	SETUP_AND_SAVE_GC;
	err = ggiGetBox(priv->parent->instance.stem,
			x + priv->position.x,
			y + priv->position.y,
			w, h, buf);
	RESTORE_GC;
	return err;
}

int GGI_sub_fillscreen(struct ggi_visual *vis)
{
	ggi_sub_priv *priv = SUB_PRIV(vis);
	int err;
	IS_SAVING_GC;

	SETUP_AND_SAVE_GC;
	err = ggiDrawBox(priv->parent->instance.stem,
			 priv->position.x, priv->position.y,
			 priv->botright.x - priv->position.x,
			 priv->botright.y - priv->position.y);
	RESTORE_GC;
	return err;
}

int GGI_sub_putc(struct ggi_visual *vis, int x, int y, char c)
{
	ggi_sub_priv *priv = SUB_PRIV(vis);
	int err;
	IS_SAVING_GC;

	SETUP_AND_SAVE_GC;
	err = ggiPutc(priv->parent->instance.stem,
		      x + priv->position.x,
		      y + priv->position.y, c);
	RESTORE_GC;
	return err;
}


int GGI_sub_puts(struct ggi_visual *vis, int x, int y, const char *str)
{
	ggi_sub_priv *priv = SUB_PRIV(vis);
	int err;
	IS_SAVING_GC;

	SETUP_AND_SAVE_GC;
	err = ggiPuts(priv->parent->instance.stem,
		      x + priv->position.x,
		      y + priv->position.y, str);
	RESTORE_GC;
	return err;
}


int GGI_sub_getcharsize(struct ggi_visual *vis, int *width, int *height)
{
	ggi_sub_priv *priv = SUB_PRIV(vis);

	return ggiGetCharSize(priv->parent->instance.stem, width, height);
}

int GGI_sub_drawhline(struct ggi_visual *vis, int x, int y, int w)
{
	ggi_sub_priv *priv = SUB_PRIV(vis);
	int err;
	IS_SAVING_GC;

	SETUP_AND_SAVE_GC;
	err = ggiDrawHLine(priv->parent->instance.stem,
			   x + priv->position.x,
			   y + priv->position.y, w);
	RESTORE_GC;
	return err;
}

int GGI_sub_drawline(struct ggi_visual *vis, int x1, int y1, int x2, int y2)
{ 
	ggi_sub_priv *priv = SUB_PRIV(vis);
	int err;
	IS_SAVING_GC;

	SETUP_AND_SAVE_GC;
	err = ggiDrawLine(priv->parent->instance.stem,
			  x1 + priv->position.x, y1 + priv->position.y,
			  x2 + priv->position.x, y2 + priv->position.y);
	RESTORE_GC;
	return err;
}

int GGI_sub_drawvline(struct ggi_visual *vis, int x, int y, int h)
{
	ggi_sub_priv *priv = SUB_PRIV(vis);
	int err;
	IS_SAVING_GC;

	SETUP_AND_SAVE_GC;
	err = ggiDrawVLine(priv->parent->instance.stem,
			   x + priv->position.x,
			   y + priv->position.y, h);
	RESTORE_GC;
	return err;
}

int GGI_sub_drawpixel(struct ggi_visual *vis, int x, int y)
{
	ggi_sub_priv *priv = SUB_PRIV(vis);
	int err;
	IS_SAVING_GC;

	SETUP_AND_SAVE_GC;
	err = ggiDrawPixel(priv->parent->instance.stem,
			   x + priv->position.x,
			   y + priv->position.y);
	RESTORE_GC;
	return err;
}

int GGI_sub_putpixel(struct ggi_visual *vis, int x, int y, ggi_pixel col)
{ 
	ggi_sub_priv *priv = SUB_PRIV(vis);
	int err;
	IS_SAVING_GC;

	SETUP_AND_SAVE_GC;
	err = ggiPutPixel(priv->parent->instance.stem,
			  x + priv->position.x,
			  y + priv->position.y, col);
	RESTORE_GC;
	return err;
}

int GGI_sub_getpixel(struct ggi_visual *vis, int x, int y, ggi_pixel *col)
{
	ggi_sub_priv *priv = SUB_PRIV(vis);
	int err;
	IS_SAVING_GC;

	SETUP_AND_SAVE_GC;
	err = ggiGetPixel(priv->parent->instance.stem,
			  x + priv->position.x,
			  y + priv->position.y, col);
	RESTORE_GC;
	return err;
}

int GGI_sub_puthline(struct ggi_visual *vis, int x, int y, int w, const void *buf)
{ 
	ggi_sub_priv *priv = SUB_PRIV(vis);
	int err;
	IS_SAVING_GC;

	SETUP_AND_SAVE_GC;
	err = ggiPutHLine(priv->parent->instance.stem,
			  x + priv->position.x,
			  y + priv->position.y, w, buf);
	RESTORE_GC;
	return err;
}

int GGI_sub_gethline(struct ggi_visual *vis, int x, int y, int w, void *buf)
{ 
	ggi_sub_priv *priv = SUB_PRIV(vis);
	int err;
	IS_SAVING_GC;

	SETUP_AND_SAVE_GC;
	err = ggiGetHLine(priv->parent->instance.stem,
			  x + priv->position.x,
			  y + priv->position.y, w, buf);
	RESTORE_GC;
	return err;
}

int GGI_sub_putvline(struct ggi_visual *vis, int x, int y, int h, const void *buf)
{
	ggi_sub_priv *priv = SUB_PRIV(vis);
	int err;
	IS_SAVING_GC;

	SETUP_AND_SAVE_GC;
	err = ggiPutVLine(priv->parent->instance.stem,
			  x + priv->position.x,
			  y + priv->position.y, h, buf);
	RESTORE_GC;
	return err;
}

int GGI_sub_getvline(struct ggi_visual *vis, int x, int y, int h, void *buf)
{
	ggi_sub_priv *priv = SUB_PRIV(vis);
	int err;
	IS_SAVING_GC;

	SETUP_AND_SAVE_GC;
	err = ggiGetVLine(priv->parent->instance.stem,
			  x + priv->position.x,
			  y + priv->position.y, h, buf);
	RESTORE_GC;
	return err;
}

int GGI_sub_copybox(struct ggi_visual *vis, int x, int y, int w, int h, int nx, int ny)
{
	ggi_sub_priv *priv = SUB_PRIV(vis);
	int err;
	IS_SAVING_GC;

	SETUP_AND_SAVE_GC;
	err = ggiCopyBox(priv->parent->instance.stem,
			 x + priv->position.x,
			 y + priv->position.y,
			 w, h, nx, ny);
	RESTORE_GC;
	return err;
}

/*
**	Simple "call parent" functions
*/

int GGI_sub_setgammamap(struct ggi_visual *vis, int start, int len, const ggi_color *colormap)
{
	ggi_sub_priv *priv = SUB_PRIV(vis);

	/* Should we allow that ? */
	return ggiSetGammaMap(priv->parent->instance.stem, start, len, colormap);
}

int GGI_sub_getgammamap(struct ggi_visual *vis, int start, int len, ggi_color *colormap)
{
	ggi_sub_priv *priv = SUB_PRIV(vis);

	return ggiGetGammaMap(priv->parent->instance.stem, start, len, colormap);
}


int GGI_sub_getgamma(struct ggi_visual *vis, double *r, double *g, double *b)
{
	ggi_sub_priv *priv = SUB_PRIV(vis);

	return ggiGetGamma(priv->parent->instance.stem, r, g, b);
}

int GGI_sub_setgamma(struct ggi_visual *vis, double r, double g, double b)
{
	ggi_sub_priv *priv = SUB_PRIV(vis);

	/* Should we allow that ? */
	return ggiSetGamma(priv->parent->instance.stem, r, g, b);
} 

ggi_pixel GGI_sub_mapcolor(struct ggi_visual *vis, const ggi_color *col)
{
	ggi_sub_priv *priv = SUB_PRIV(vis);

	return ggiMapColor(priv->parent->instance.stem, col);
}

int GGI_sub_unmappixel(struct ggi_visual *vis, ggi_pixel pixel, ggi_color *col)
{
	ggi_sub_priv *priv = SUB_PRIV(vis);

	return ggiUnmapPixel(priv->parent->instance.stem, pixel, col);
}

int GGI_sub_setpalvec(struct ggi_visual *vis, int start, int len, const ggi_color *colormap)
{
	ggi_sub_priv *priv = SUB_PRIV(vis);

	/* Should we allow that ? */
	return ggiSetPalette(priv->parent->instance.stem, start, len, colormap);
}

int GGI_sub_getpalvec(struct ggi_visual *vis, int start, int len, ggi_color *colormap)
{
	ggi_sub_priv *priv = SUB_PRIV(vis);

	return ggiGetPalette(priv->parent->instance.stem, start, len, colormap);
}
