/* $Id: x.h,v 1.1 2001/05/12 23:03:19 cegger Exp $
******************************************************************************

   Display-X: data

   Copyright (C) 1997 Andreas Beck		[becka@ggi-project.org]
   Copyright (C) 1998-1999 Marcus Sundberg	[marcus@ggi-project.org]

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

#ifndef _GGI_DISPLAY_X_H
#define _GGI_DISPLAY_X_H

#include <ggi/internal/ggi-dl.h>
#include <ggi/input/xwin.h>
#include <ggi/display/mansync.h>

#include <ggi/display/xcommon.h>

#ifdef HAVE_SYS_SHM_H
#define HAVE_SHM
#include <sys/shm.h>
#include <X11/extensions/XShm.h>
#endif

#if defined(__osf__)
extern int XShmQueryExtension(Display *);
#endif

ggifunc_flush		GGI_X_flush;
ggifunc_getmode		GGI_X_getmode;
ggifunc_setmode		GGI_X_setmode;
ggifunc_checkmode	GGI_X_checkmode;
ggifunc_getapi 		GGI_X_getapi;
ggifunc_setflags	GGI_X_setflags;

ggifunc_setpalvec	GGI_X_setpalvec;

typedef struct {
	ggi_xwin_common	 xwin;

	/* x only data */
	int      xoff, yoff;	/* We can pan, too */
	int      ysplit;	/* Emulating Splitline ;-) */
	int      viswidth, visheight;
	XImage  *ximage;		/* Current frame */
	XImage  *ximage_list[8];	/* List of frames */
	_ggi_opmansync *opmansync;
	
#ifdef HAVE_SHM
	XShmSegmentInfo shminfo[8];	/* Segment info. */
	int     have_shm;
#endif
} ggi_x_priv;

#define GGIX_PRIV(vis) ((ggi_x_priv *)LIBGGI_PRIVATE(vis))

/* Defined in mode.c */
int _ggi_x_do_blit(ggi_x_priv *priv, int x, int y, int w, int h);
int _ggi_x_resize (ggi_visual_t vis, int w, int h, ggi_event *ev);
/* Defined in visual.c */
void _GGI_X_freedbs(ggi_visual *, ggi_x_priv *);

#define MANSYNC_init(vis)   GGIX_PRIV(vis)->opmansync->init(vis)
#define MANSYNC_deinit(vis) GGIX_PRIV(vis)->opmansync->deinit(vis)
#define MANSYNC_start(vis)  GGIX_PRIV(vis)->opmansync->start(vis)
#define MANSYNC_stop(vis)   GGIX_PRIV(vis)->opmansync->stop(vis)
#define MANSYNC_ignore(vis) GGIX_PRIV(vis)->opmansync->ignore(vis)
#define MANSYNC_cont(vis)   GGIX_PRIV(vis)->opmansync->cont(vis)

#endif /* _GGI_DISPLAY_X_H */
