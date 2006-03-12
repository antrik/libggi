/* $Id: dltypes.h,v 1.14 2006/03/12 22:39:11 cegger Exp $
******************************************************************************

   LibGGI - typedefs for internal API functions

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

#ifndef _GGI_INTERNAL_DLTYPES_H
#define _GGI_INTERNAL_DLTYPES_H

#include <ggi/types.h>

/*
******************************************************************************
 Typedef internal function types
******************************************************************************
*/

/* Sublib handling
 */

/* Sublib functions */
#define GGIFUNC_open	1
#define GGIFUNC_exit	2
#define GGIFUNC_close	3

typedef int (ggifunc_dlentry)(int func, void **funcptr);

typedef int (ggifunc_open)(struct ggi_visual *vis, struct ggi_dlhandle *dlh,
			   const char *args, void *argptr, uint32_t *dlret);
typedef int (ggifunc_exit)(struct ggi_visual *vis,
			   struct ggi_dlhandle *dlh);
typedef int (ggifunc_close)(struct ggi_visual *vis,
			    struct ggi_dlhandle *dlh);

/* Resource management */
typedef int (ggifunc_resacquire)(struct ggi_resource *res, uint32_t actype);
typedef int (ggifunc_resrelease)(struct ggi_resource *res);


/* Mode Setting routines */
typedef int (ggifunc_getmode)(ggi_visual_t v,ggi_mode *tm);
typedef int (ggifunc_setmode)(ggi_visual_t v,ggi_mode *tm);
typedef int (ggifunc_checkmode)(ggi_visual_t v,ggi_mode *tm);

/* Flag settings */
typedef int (ggifunc_setflags)(ggi_visual_t v,ggi_flags flags);

/* Flushing the display
 */
typedef int (ggifunc_flush)(ggi_visual_t v, int x, int y, int w, int h,
			    int tryflag);

/* Idle the accelerator */
typedef int (ggifunc_idleaccel)(struct ggi_visual *vis);


/* Frame handling
 */
typedef int (ggifunc_setdisplayframe)(ggi_visual_t v, int num);
typedef int (ggifunc_setreadframe)(ggi_visual_t v, int num);
typedef int (ggifunc_setwriteframe)(ggi_visual_t v, int num);

typedef ggi_pixel (ggifunc_mapcolor)(ggi_visual_t v,const ggi_color *color);
typedef int (ggifunc_unmappixel)(ggi_visual_t v,ggi_pixel pixel,ggi_color *color);
typedef int (ggifunc_packcolors)(ggi_visual_t v,void *buf,const ggi_color *colors,int len);
typedef int (ggifunc_unpackpixels)(ggi_visual_t v,const void *buf,ggi_color *colors,int len);

typedef int (ggifunc_setpalvec)(ggi_visual_t v,int start,int len,const ggi_color *colormap);
typedef int (ggifunc_getpalvec)(ggi_visual_t v,int start,int len,ggi_color *colormap);

/* Colormap */
typedef int (ggifunc_setPalette)(ggi_visual_t v, size_t start, size_t size, const ggi_color *cmap);

/* Gamma correction */
typedef int (ggifunc_getgamma)(ggi_visual_t v,ggi_float *r,ggi_float *g,ggi_float *b);
typedef int (ggifunc_setgamma)(ggi_visual_t v,ggi_float r,ggi_float g,ggi_float b);
typedef int (ggifunc_setgammamap)(ggi_visual_t v,int start,int len,const ggi_color *gammamap);
typedef int (ggifunc_getgammamap)(ggi_visual_t v,int start,int len,ggi_color *gammamap);

typedef int (ggifunc_setorigin)(ggi_visual_t v,int x,int y);

typedef int (ggifunc_putc)(ggi_visual_t v,int x,int y,char c);
typedef int (ggifunc_puts)(ggi_visual_t v,int x,int y,const char *str);
typedef int (ggifunc_getcharsize)(ggi_visual_t v,int *width,int *height);

/* Generic drawing routines
 */
typedef int (ggifunc_fillscreen)(ggi_visual_t v);

typedef int (ggifunc_drawpixel_nc)(ggi_visual_t v,int x,int y);
typedef int (ggifunc_drawpixel)(ggi_visual_t v,int x,int y);
typedef int (ggifunc_putpixel_nc)(ggi_visual_t v,int x,int y,ggi_pixel pixel);
typedef int (ggifunc_putpixel)(ggi_visual_t v,int x,int y,ggi_pixel pixel);
typedef int (ggifunc_getpixel)(ggi_visual_t v,int x,int y,ggi_pixel *pixel);

typedef int (ggifunc_drawline)(ggi_visual_t v,int x,int y,int xe,int ye);

typedef int (ggifunc_drawhline_nc)(ggi_visual_t v,int x,int y,int w);
typedef int (ggifunc_drawhline)(ggi_visual_t v,int x,int y,int w);
typedef int (ggifunc_puthline)(ggi_visual_t v,int x,int y,int w,const void *buf);
typedef int (ggifunc_gethline)(ggi_visual_t v,int x,int y,int w,void *buf);

typedef int (ggifunc_drawvline_nc)(ggi_visual_t v,int x,int y,int h);
typedef int (ggifunc_drawvline)(ggi_visual_t v,int x,int y,int h);
typedef int (ggifunc_putvline)(ggi_visual_t v,int x,int y,int h,const void *buf);
typedef int (ggifunc_getvline)(ggi_visual_t v,int x,int y,int h,void *buf);

typedef int (ggifunc_drawbox)(ggi_visual_t v,int x,int y,int w,int h);
typedef int (ggifunc_putbox)(ggi_visual_t v,int x,int y,int w,int h,const void *buf);
typedef int (ggifunc_getbox)(ggi_visual_t v,int x,int y,int w,int h,void *buf);

typedef int (ggifunc_copybox)(ggi_visual_t v,int x,int y,int w,int h,int nx,int ny);
typedef int (ggifunc_crossblit)(ggi_visual_t src,int sx,int sy,int w,int h,
		 ggi_visual_t dst,int dx,int dy);

typedef void (ggifunc_gcchanged)(struct ggi_visual *vis,int mask);
typedef int  (ggifunc_getapi)(struct ggi_visual *vis, int num, char *apiname, char *arguments);

typedef int (ggifunc_kgicommand)(struct ggi_visual *vis,int cmd,void *args);

#endif /* _GGI_INTERNAL_DLTYPES_H */
