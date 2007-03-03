/* $Id: monotext.h,v 1.10 2007/03/03 18:19:12 soyt Exp $
******************************************************************************

   Display-monotext: graphics emulation on text modes

   Copyright (C) 1998 Andrew Apted		[andrew@ggi-project.org]
   Copyright (C) 1998-2000 Marcus Sundberg	[marcus@ggi-project.org]

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

#ifndef _GGI_DISPLAY_MONOTEXT_H
#define _GGI_DISPLAY_MONOTEXT_H

#include <ggi/internal/ggi-dl.h>
#include <ggi/display/mansync.h>


/**************************************************
 **
 **  MonoText private defines
 **
 **************************************************/


#ifndef MIN
#define MIN(a, b)  ((a) < (b) ? (a) : (b))
#endif

#ifndef MAX
#define MAX(a, b)  ((a) > (b) ? (a) : (b))
#endif

#define UPDATE_MOD(vis, _x1, _y1, w, h) \
	do {								\
		ggi_monotext_priv *_priv = MONOTEXT_PRIV(vis);		\
		int _x2 = (_x1) + (w);					\
		int _y2 = (_y1) + (h);					\
									\
		if ((_x1) < _priv->dirty_tl.x) _priv->dirty_tl.x =	\
			MAX((_x1), (vis)->gc->cliptl.x);		\
		if ((_y1) < _priv->dirty_tl.y) _priv->dirty_tl.y = 	\
			MAX((_y1), (vis)->gc->cliptl.y);		\
		if ((_x2) > _priv->dirty_br.x) _priv->dirty_br.x =	\
			MIN((_x2), (vis)->gc->clipbr.x);		\
		if ((_y2) > _priv->dirty_br.y) _priv->dirty_br.y =	\
			MIN((_y2), (vis)->gc->clipbr.y);		\
	} while (0)


/****************************************************
 **
 **  MonoText private data
 **
 ****************************************************/


typedef struct ggi_monotext_priv {
	int flags;

	ggi_visual_t parent;
	ggi_mode parent_gt;

	ggi_coord size;
	ggi_coord accuracy;
	ggi_coord squish;

	/* framebuffer */
	void *fb_ptr;
	long  fb_size;

	/* color info */
	uint8_t *greymap;
	ggi_color *colormap;

	uint8_t *rgb_to_grey;	/* index = RGB 5:5:5 */

	double red_gamma;
	double green_gamma;
	double blue_gamma;

	/* 2D operations on memory buffer */
	struct ggi_visual_opdraw *mem_opdraw;

	ggi_coord dirty_tl;
	ggi_coord dirty_br;

	/* blitter function */
	void (* do_blit) (struct ggi_monotext_priv *priv, void *dest,
			  void *src, int w);

	/* mansync info */
	void *flush_lock;
	MANSYNC_DATA;
} ggi_monotext_priv;

#define MONOTEXT_PRIV(vis) ((ggi_monotext_priv *)LIBGGI_PRIVATE(vis))


/****************************************************
 **
 **  MonoText private functions
 **
 ****************************************************/


extern int _ggi_monotextOpen(struct ggi_visual *vis);
extern int _ggi_monotextClose(struct ggi_visual *vis);
extern int _ggi_monotextUpdate(struct ggi_visual *vis, int x, int y, int w, int h);
extern int _ggi_monotextFlush(struct ggi_visual *vis);


/****************************************************
 **
 **  MonoText internal interfaces 
 **
 ****************************************************/


ggifunc_getmode		GGI_monotext_getmode;
ggifunc_setmode		GGI_monotext_setmode;
ggifunc_checkmode	GGI_monotext_checkmode;
ggifunc_getapi		GGI_monotext_getapi;
ggifunc_flush		GGI_monotext_flush;
ggifunc_setflags	GGI_monotext_setflags;

ggifunc_drawpixel	GGI_monotext_drawpixel;
ggifunc_drawpixel_nc	GGI_monotext_drawpixel_nc;
ggifunc_drawhline_nc	GGI_monotext_drawhline_nc;
ggifunc_drawhline	GGI_monotext_drawhline;
ggifunc_drawvline_nc	GGI_monotext_drawvline_nc;
ggifunc_drawvline	GGI_monotext_drawvline;
ggifunc_drawline	GGI_monotext_drawline;

ggifunc_putc		GGI_monotext_putc;
ggifunc_putpixel_nc	GGI_monotext_putpixel_nc;
ggifunc_putpixel	GGI_monotext_putpixel;
ggifunc_puthline	GGI_monotext_puthline;
ggifunc_putvline	GGI_monotext_putvline;

ggifunc_putbox		GGI_monotext_putbox;
ggifunc_drawbox		GGI_monotext_drawbox;
ggifunc_copybox		GGI_monotext_copybox;
ggifunc_crossblit	GGI_monotext_crossblit;
ggifunc_fillscreen	GGI_monotext_fillscreen;
ggifunc_setorigin	GGI_monotext_setorigin;
ggifunc_setPalette	GGI_monotext_setPalette;


#define MANSYNC_init(vis)   MANSYNC_DECL_INIT(MONOTEXT_PRIV(vis), vis)
#define MANSYNC_deinit(vis) MANSYNC_DECL_DEINIT(MONOTEXT_PRIV(vis), vis)
#define MANSYNC_start(vis)  MANSYNC_DECL_START(MONOTEXT_PRIV(vis), vis)
#define MANSYNC_stop(vis)   MANSYNC_DECL_STOP(MONOTEXT_PRIV(vis), vis)
#define MANSYNC_ignore(vis) MANSYNC_DECL_IGNORE(MONOTEXT_PRIV(vis), vis)
#define MANSYNC_cont(vis)   MANSYNC_DECL_CONT(MONOTEXT_PRIV(vis), vis)


#endif /* _GGI_DISPLAY_MONOTEXT_H */
