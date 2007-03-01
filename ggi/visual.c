/* $Id: visual.c,v 1.15 2007/03/01 15:04:40 cegger Exp $
******************************************************************************

   Graphics library for GGI. Handles visuals.

   Copyright (C) 1997 Jason McMullan		[jmcc@ggi-project.org]
   Copyright (C) 2000 Marcus Sundberg		[marcus@ggi-project.org]

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

#include <stdlib.h>

#include "config.h"
#include <ggi/internal/internal.h>
#include <ggi/internal/ggi_debug.h>


/*
******************************************************************************
 Default functions for op-structs
******************************************************************************
*/

static int _default_error(void)
{
        DPRINT_MISC("_default_error() called\n");
	return GGI_ENOFUNC;
}


/*
******************************************************************************
 Memory allocation
******************************************************************************
*/

void _ggi_mem_error(void) {
	ggPanic("LibGGI is out of memory!\n");
}

void *_ggi_malloc(size_t siz)
{
	void *mem = calloc(1, siz);

	if (mem == NULL) {
		_ggi_mem_error();
	}
	return mem;
}

void *_ggi_calloc(size_t siz)
{
	void *mem = calloc(1, siz);

	if (mem == NULL) {
		_ggi_mem_error();
	}
	return mem;
}

void *_ggi_realloc(void *ptr, size_t siz)
{
	void *mem = realloc(ptr, siz);

	if (mem == NULL) {
		_ggi_mem_error();
	}
	return mem;
}


/*
******************************************************************************
 Allocate space for helper private data
******************************************************************************
*/

static uint32_t _ggi_drvpriv_inuse = 0;

int _ggi_alloc_drvpriv(void)
{
	int idx, v;

	for (idx = 0, v = 1; idx < _GGI_NROF_HELPERS; idx++, v<<=1) {
		if ((_ggi_drvpriv_inuse & v) == 0) {
			_ggi_drvpriv_inuse |= v;
			return idx;
		}
	}

	return -1;
}

void _ggi_free_drvpriv(int idx)
{
	_ggi_drvpriv_inuse &= ~(1<<idx);
}

/*
******************************************************************************
 Creation and destruction of visuals
******************************************************************************
*/

typedef int   (*__simp_int)(void);

static void *_ggi_alloc_op(int numfuncs)
{
	return malloc(sizeof(struct ggi_op_head)
		      + sizeof(__simp_int*) * numfuncs);
}

static void *_ggi_init_op(struct ggi_op_head *head, int numfuncs)
{
	__simp_int *funcarr;
	int i;

	GG_SLIST_INIT(&head->dlhandle);
	head->dummy = NULL;

	funcarr = (__simp_int*)(head+1);
	for (i = 0; i < numfuncs; i++) {
		funcarr[i] = _default_error;
	}

	return head;
}

static void _ggi_init_allops(struct ggi_visual *vis, int initall)
{
	_ggi_init_op((struct ggi_op_head *)vis->opdraw, GGI_OPDRAW_NUMFUNCS);
	vis->opdraw->head.version	= GGI_VERSION_VISUAL_OPDRAW;
	_ggi_init_op((struct ggi_op_head *)vis->opcolor, GGI_OPCOLOR_NUMFUNCS);
	vis->opcolor->head.version	= GGI_VERSION_VISUAL_OPCOLOR;
	_ggi_init_op((struct ggi_op_head *)vis->opgc, GGI_OPGC_NUMFUNCS);
	vis->opgc->head.version		= GGI_VERSION_VISUAL_OPGC;
	if (initall) {
		_ggi_init_op((struct ggi_op_head *)vis->opdisplay,
			     GGI_OPDISPLAY_NUMFUNCS);
		vis->opdisplay->head.version = GGI_VERSION_VISUAL_OPDISPLAY;
	}
}

static void _ggiCloseDL(struct ggi_visual *vis, int zapall)
{
	_ggiExitDL(vis, GG_SLIST_FIRST(&vis->generic_ext));
	_ggiExitDL(vis, GG_SLIST_FIRST(&vis->opdraw->head.dlhandle));
	_ggiExitDL(vis, GG_SLIST_FIRST(&vis->opcolor->head.dlhandle));
	_ggiExitDL(vis, GG_SLIST_FIRST(&vis->opgc->head.dlhandle));
	if (zapall) _ggiExitDL(vis, GG_SLIST_FIRST(&vis->opdisplay->head.dlhandle));

	_ggiZapDL(vis, &GG_SLIST_FIRST(&vis->generic_ext));
	_ggiZapDL(vis, &GG_SLIST_FIRST(&vis->opdraw->head.dlhandle));
	_ggiZapDL(vis, &GG_SLIST_FIRST(&vis->opcolor->head.dlhandle));
	_ggiZapDL(vis, &GG_SLIST_FIRST(&vis->opgc->head.dlhandle));
	if (zapall) _ggiZapDL(vis, &GG_SLIST_FIRST(&vis->opdisplay->head.dlhandle));
}

void _ggiZapMode(struct ggi_visual *vis, int zapall)
{
	_ggiCloseDL(vis, zapall);
	_ggi_init_allops(vis, zapall);
}

struct ggi_visual *_ggiNewVisual(void)
{
	struct ggi_visual *vis;

	vis = malloc(sizeof(*vis));
	if (vis == NULL) return NULL;

	vis->channel = ggNewChannel(vis, NULL);
	if (vis->channel == NULL) goto out_freevis;

	vis->mutex = ggLockCreate();
	if (vis->mutex == NULL) goto out_delchannel;

	vis->version = GGI_VERSION_VISUAL;
	vis->numknownext = 0;

	LIBGGI_MODE(vis) = calloc(1, sizeof(ggi_mode));
	if (LIBGGI_MODE(vis) == NULL) {
		goto out_destroylock;
	}

	LIBGGI_PIXFMT(vis) = calloc(1, sizeof(ggi_pixelformat));
	if (LIBGGI_PIXFMT(vis) == NULL) {
		goto out_freemode;
	}

	LIBGGI_APPLIST(vis) = calloc(1, sizeof(ggi_db_list));
	if (LIBGGI_APPLIST(vis) == NULL) {
		goto out_freepixfmt;
	}

	LIBGGI_PRIVLIST(vis) = calloc(1, sizeof(ggi_db_list));
	if (LIBGGI_PRIVLIST(vis) == NULL) {
		goto out_freeapplist;
	}

	vis->opdraw = _ggi_alloc_op(GGI_OPDRAW_NUMFUNCS);
	if (!vis->opdraw) goto out_freeprivlist;
	vis->opcolor = _ggi_alloc_op(GGI_OPCOLOR_NUMFUNCS);
	if (!vis->opcolor) goto out_freeopdraw;
	vis->opgc = _ggi_alloc_op(GGI_OPGC_NUMFUNCS);
	if (!vis->opgc) goto out_freeopcolor;
	vis->opdisplay = _ggi_alloc_op(GGI_OPDISPLAY_NUMFUNCS);
	if (!vis->opdisplay) goto out_freeopgc;

	LIBGGI_APPLIST(vis)->num = LIBGGI_PRIVLIST(vis)->num = 0;
	LIBGGI_APPLIST(vis)->first_targetbuf
		= LIBGGI_PRIVLIST(vis)->first_targetbuf = -1;
	LIBGGI_APPBUFS(vis) = LIBGGI_PRIVBUFS(vis) = NULL;

	LIBGGI_FLAGS(vis) = 0;
	LIBGGI_FD(vis) = -1;

	GG_SLIST_INIT(&LIBGGI_DLHANDLE(vis));
	GG_SLIST_INIT(&vis->generic_ext);

	vis->d_frame_num = vis->r_frame_num = vis->w_frame_num = 0;
	vis->r_frame = vis->w_frame = NULL;
	vis->origin_x = vis->origin_y = 0;
	vis->needidleaccel = vis->accelactive = 0;
	vis->gamma = NULL;

	LIBGGI_PAL(vis) = _ggi_malloc(sizeof(ggi_colormap));
	if (!LIBGGI_PAL(vis)) goto out_freeopdisplay;
 	
 	LIBGGI_PAL(vis)->clut.data = NULL;
 	LIBGGI_PAL(vis)->clut.size = 0;
 	LIBGGI_PAL(vis)->rw_start  = 0;
 	LIBGGI_PAL(vis)->rw_stop   = 0;
 	LIBGGI_PAL(vis)->ro_start  = 0;
 	LIBGGI_PAL(vis)->ro_stop   = 0;
 	LIBGGI_PAL(vis)->priv      = NULL;

	_ggi_init_allops(vis, 1);

	return vis;

	/* Error occured. */
  out_freeopdisplay:
 	free(vis->opdisplay);
  out_freeopgc:
	free(vis->opgc);
  out_freeopcolor:
	free(vis->opcolor);
  out_freeopdraw:
	free(vis->opdraw);
  out_freeprivlist:
	free(LIBGGI_PRIVLIST(vis));
  out_freeapplist:
	free(LIBGGI_APPLIST(vis));
  out_freepixfmt:
	free(LIBGGI_PIXFMT(vis));
  out_freemode:
	free(LIBGGI_MODE(vis));
  out_destroylock:
	ggLockDestroy(vis->mutex);
  out_delchannel:
	ggDelChannel(vis->channel);
  out_freevis:
	free(vis);

	return NULL;
}


void _ggiDestroyVisual(struct ggi_visual *vis)
{
	_ggiCloseDL(vis, 1);
	vis->stem = NULL;

	if (LIBGGI_PAL(vis)) {
		if (LIBGGI_PAL(vis)->priv) free(LIBGGI_PAL(vis)->priv);
		if (LIBGGI_PAL(vis)->clut.data)
			free(LIBGGI_PAL(vis)->clut.data);
 	 	free(LIBGGI_PAL(vis));
	}
	
	free(vis->opdisplay);
	free(vis->opgc);
	free(vis->opcolor);
	free(vis->opdraw);
	free(LIBGGI_PRIVLIST(vis));
	free(LIBGGI_APPLIST(vis));
	free(LIBGGI_PIXFMT(vis));
	free(LIBGGI_MODE(vis));
	ggLockDestroy(vis->mutex);
	ggDelChannel(vis->channel);
	free(vis);
}
