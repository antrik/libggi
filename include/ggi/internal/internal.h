/* $Id: internal.h,v 1.1 2001/05/12 23:03:23 cegger Exp $
******************************************************************************

   LibGGI internal functions and macros

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

#ifndef _GGI_INTERNAL_H
#define _GGI_INTERNAL_H

#define _INTERNAL_LIBGGI

#include <ggi/types.h>
#include <ggi/internal/gii.h>
#include <ggi/internal/plat.h>
#include <ggi/internal/debug.h>
#include <ggi/internal/structs.h>

/*
******************************************************************************
 Library internal functions and variables
******************************************************************************
*/

__BEGIN_DECLS

/* Exported variables */
#ifdef BUILDING_LIBGGI
extern void     *_ggi_global_lock;
#else
IMPORTVAR void  *_ggi_global_lock;
#endif

/* conf.c */
extern void *_ggiConfigHandle;

/* db.c */
ggi_directbuffer *_ggi_db_get_new(void);
void _ggi_db_free(ggi_directbuffer *db);
int _ggi_db_add_buffer(ggi_db_list *dbl, ggi_directbuffer *buf);
int _ggi_db_del_buffer(ggi_db_list *dbl, int idx);
int _ggi_db_move_buffer(ggi_db_list *dst, ggi_db_list *src, int idx);
ggi_directbuffer *_ggi_db_find_frame(ggi_visual *vis, int frameno);

/* dl.c */
ggi_dlhandle *_ggiAddExtDL(ggi_visual *vis, const char *filename,
			   const char *args, void *argptr,
			   const char *symprefix);
int _ggiOpenDL(ggi_visual *vis, const char *name,
	       const char *args, void *argptr);
void _ggiExitDL(ggi_visual *vis, ggi_dlhandle_l *lib);
void _ggiZapDL(ggi_visual *vis, ggi_dlhandle_l **lib);
int _ggiAddDL(ggi_visual *vis, const char *drv, const char *args,
	      void *argptr, int type);

/* internal.c */
int _ggi_mask2shift(uint32 mask);
void _ggi_build_pixfmt(ggi_pixelformat *pixfmt);
int _ggi_match_palette(ggi_color *pal, int pal_len, ggi_color *col);
ggifunc_setreadframe _ggi_default_setreadframe;
ggifunc_setwriteframe _ggi_default_setwriteframe;
void _ggi_build_palette(ggi_color *pal, int num);
void _ggi_smart_match_palettes(ggi_color *pal, int size,
                               ggi_color *ref_pal, int ref_size);

/* mode.c */
void _ggiSetDefaultMode(const char *str);

/* stubs.c */
int _ggiInternFlush(ggi_visual *vis, int x, int y, int w, int h, int tryflag);
int _ggiPutPixelNC(ggi_visual *vis,int x,int y,ggi_pixel p);
int _ggiDrawPixelNC(ggi_visual *vis,int x,int y);
int _ggiDrawHLineNC(ggi_visual *vis,int x,int y,int w);
int _ggiDrawVLineNC(ggi_visual *vis,int x,int y,int h);
int _ggiIdleAccel(ggi_visual *vis);
int _ggiSendKGICommand(ggi_visual *vis,int cmd,void *arg);

/* visual.c */
void *_ggi_malloc(size_t siz);
void *_ggi_calloc(size_t siz);
void *_ggi_realloc(void *ptr, size_t siz);
void _ggi_mem_error(void);
int _ggi_alloc_drvpriv(void);
void _ggi_free_drvpriv(int id);
void _ggiZapMode(ggi_visual *vis, int zapall);

ggi_visual *_ggiNewVisual(void);
void _ggiDestroyVisual(ggi_visual *vis);

__END_DECLS


/*
******************************************************************************
 Macros for quickly accessing performance critical LibGGI functions.
******************************************************************************
*/

#define LIBGGIMapColor(vis,col)		(vis->opcolor->mapcolor(vis,col))
#define LIBGGIUnmapPixel(vis,pixel,col)	\
	(vis->opcolor->unmappixel(vis,pixel,col))

#define LIBGGIDrawPixel(vis,x,y)	(vis->opdraw->drawpixel(vis,x,y))
#define LIBGGIDrawPixelNC(vis,x,y)	(vis->opdraw->drawpixel_nc(vis,x,y))
#define LIBGGIPutPixel(vis,x,y,col)	(vis->opdraw->putpixel(vis,x,y,col))
#define LIBGGIPutPixelNC(vis,x,y,col)	(vis->opdraw->putpixel_nc(vis,x,y,col))
#define LIBGGIGetPixel(vis,x,y,col)	(vis->opdraw->getpixel(vis,x,y,col))

#define LIBGGIIdleAccel(vis)		(vis->opdisplay->idleaccel(vis))


/*
******************************************************************************
 Macros to access members of LibGGI structures.
 Please use these instead of directly referencing the members.
******************************************************************************
*/

#define LIBGGI_EXT(vis,extid)	((vis)->extlist[(extid)].priv)
#define LIBGGI_EXTAC(vis,extid)	((vis)->extlist[(extid)].attachcount)

#define LIBGGI_FLAGS(vis)	((vis)->flags)
#define LIBGGI_FD(vis)		((vis)->fd)

#define LIBGGI_MODE(vis)	((vis)->mode)
#define LIBGGI_PIXFMT(vis)	((vis)->pixfmt)
#define LIBGGI_GC(vis)		((vis)->gc)
#define LIBGGI_GC_FGCOLOR(vis)	((vis)->gc->fg_color)
#define LIBGGI_GC_BGCOLOR(vis)	((vis)->gc->bg_color)

#define LIBGGI_APPLIST(vis)	((vis)->app_dbs)
#define LIBGGI_PRIVLIST(vis)	((vis)->priv_dbs)
#define LIBGGI_APPBUFS(vis)	(LIBGGI_APPLIST(vis)->bufs)
#define LIBGGI_PRIVBUFS(vis)	(LIBGGI_PRIVLIST(vis)->bufs)
#define LIBGGI_CURREAD(vis)	((vis)->r_frame->read)
#define LIBGGI_CURWRITE(vis)	((vis)->w_frame->write)

#define LIBGGI_DLHANDLE(vis)	((vis)->dlhandle)
#define LIBGGI_PRIVATE(vis)	((vis)->targetpriv)
#define LIBGGI_DRVPRIV(vis,idx)	((vis)->drvpriv[(idx)])

#define LIBGGI_X(vis)		((vis)->mode->visible.x)
#define LIBGGI_Y(vis)		((vis)->mode->visible.y)
#define LIBGGI_VIRTX(vis)	((vis)->mode->virt.x)
#define LIBGGI_VIRTY(vis)	((vis)->mode->virt.y)
#define LIBGGI_GT(vis)		((vis)->mode->graphtype)

#define LIBGGI_FB_SIZE(mode)	(((GT_SIZE((mode)->graphtype)*(mode)->virt.x*(mode)->virt.y)+7)/8)
#define LIBGGI_FB_R_STRIDE(vis)	((vis)->r_frame->buffer.plb.stride)
#define LIBGGI_FB_W_STRIDE(vis)	((vis)->w_frame->buffer.plb.stride)

#define LIBGGI_R_PLAN(vis)	((vis)->r_frame->buffer.plan)
#define LIBGGI_W_PLAN(vis)	((vis)->w_frame->buffer.plan)

#endif /* _GGI_INTERNAL_H */
