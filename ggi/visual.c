/* $Id: visual.c,v 1.22 2007/05/25 21:22:19 soyt Exp $
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
#include <ggi/internal/ggi.h>
#include <ggi/internal/ggi-module.h>
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
	DPRINT_LIBS("_ggiCloseDL(%p, %i) called\n", vis, zapall);

	DPRINT_LIBS("_ggiCloseDL: call _ggiExitDL on generic_ext\n");
	_ggiExitDL(vis, GG_SLIST_FIRST(&vis->generic_ext));
	DPRINT_LIBS("_ggiCloseDL: call _ggiExitDL on opdraw\n");
	_ggiExitDL(vis, GG_SLIST_FIRST(&vis->opdraw->head.dlhandle));
	DPRINT_LIBS("_ggiCloseDL: call _ggiExitDL on opcolor\n");
	_ggiExitDL(vis, GG_SLIST_FIRST(&vis->opcolor->head.dlhandle));
	DPRINT_LIBS("_ggiCloseDL: call _ggiExitDL on opgc\n");
	_ggiExitDL(vis, GG_SLIST_FIRST(&vis->opgc->head.dlhandle));
	if (zapall) {
		DPRINT_LIBS("_ggiCloseDL: call _ggiExitDL on opdisplay (zapall)\n");
		_ggiExitDL(vis, GG_SLIST_FIRST(&vis->opdisplay->head.dlhandle));
	}

	DPRINT_LIBS("_ggiCloseDL: call _ggiZapDL on generic_ext\n");
	_ggiZapDL(vis, &GG_SLIST_FIRST(&vis->generic_ext));
	DPRINT_LIBS("_ggiCloseDL: call _ggiZapDL on opdraw\n");
	_ggiZapDL(vis, &GG_SLIST_FIRST(&vis->opdraw->head.dlhandle));
	DPRINT_LIBS("_ggiCloseDL: call _ggiZapDL on opcolor\n");
	_ggiZapDL(vis, &GG_SLIST_FIRST(&vis->opcolor->head.dlhandle));
	DPRINT_LIBS("_ggiCloseDL: call _ggiZapDL on opgc\n");
	_ggiZapDL(vis, &GG_SLIST_FIRST(&vis->opgc->head.dlhandle));
	if (zapall) {
		DPRINT_LIBS("_ggiCloseDL: call _ggiZapDL on opdisplay (zapall)\n");
		_ggiZapDL(vis, &GG_SLIST_FIRST(&vis->opdisplay->head.dlhandle));
	}
}

void _ggiZapMode(struct ggi_visual *vis, int zapall)
{
	_ggiCloseDL(vis, zapall);
	_ggi_init_allops(vis, zapall);
}

struct ggi_visual *_ggiNewVisual(void)
{
	struct ggi_visual *vis;

	vis = calloc(1, sizeof(*vis));
	if (vis == NULL) return NULL;

	vis->instance.channel = ggNewChannel(vis, NULL);
	if (vis->instance.channel == NULL) goto out_freevis;

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

	GG_LIST_INIT(&vis->helpers);

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
	ggDelChannel(vis->instance.channel);
  out_freevis:
	free(vis);

	return NULL;
}


void _ggiDestroyVisual(struct ggi_visual *vis)
{
	_ggiCloseDL(vis, 1);
	vis->instance.stem = NULL;

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
	ggDelChannel(vis->instance.channel);
	free(vis);
}


int
_ggiVisualOpenDisplay(struct ggi_visual *vis,
		      const char* name,
		      const char * args,
		      void *argp)
{
	int err;
	union {
		struct gg_module *any;
		struct ggi_module_display *display;
	} module;
	
	LIB_ASSERT(vis->instance.module == NULL, "already opened!");
	
	module.any = ggOpenModule(libggi, name);
	
	if (module.any == NULL)
		return GGI_ENOTFOUND;
	
	if (module.any->klass != GGI_MODULE_DISPLAY) {
		ggCloseModule(module.any);
		return GGI_ENOMATCH;
	}
	
	vis->instance.module = module.any;
	err = module.display->open(vis, args, argp);
	
	if(err) {
		vis->instance.module = NULL;
		ggCloseModule(module.any);
	}

	return err;
}

void
_ggiVisualCloseDisplay(struct ggi_visual *vis)
{
	union {
		struct gg_module *any;
		struct ggi_module_display *display;
	} module;
	
	LIB_ASSERT(vis->instance.module != NULL, "no display!");
	
	module.any = vis->instance.module;
	
	if(module.display->close)
		module.display->close(vis);
	
	vis->instance.module = NULL;
	ggCloseModule(module.any);
}

int
_ggiVisualLoadHelper(struct ggi_visual *vis,
		     const char *name,
		     const char *args,
		     void *argp,
		     struct ggi_helper **res)
{
	int err;
	union {
		struct gg_module *any;
		struct ggi_module_helper *helper;
	} module;
	struct ggi_helper *helper;
	
	LIB_ASSERT(vis->instance.module != NULL, "not opened!");
	
	module.any = NULL;
	err = GGI_ENOMEM;
	
	helper = calloc(1, sizeof(*helper));
	if (helper == NULL)
		goto end;
	
	err = GGI_ENOTFOUND;
	module.any = ggOpenModule(libggi, name);
	if (module.any == NULL)
		goto end;
	
	err = GGI_ENOMATCH;
	if (module.any->klass != GGI_MODULE_HELPER)
		goto end;
	
	helper->plugin.module = module.any;
	helper->visual = vis;
	err = module.helper->setup(helper, args, argp);
 end:
	if(err) {
		free(helper);
		if(module.any)
			ggCloseModule(module.any);
		if(res)
			*res = NULL;
	} else {
		GG_LIST_INSERT_HEAD(&vis->helpers, helper, h_list);
		if(res)
			*res = helper;
	}
	
	return err;
}


void
_ggiVisualTeardownHelper(struct ggi_helper *helper)
{
	struct ggi_visual *vis;
	union {
		struct gg_module *any;
		struct ggi_module_helper *helper;
	} module;
	
	module.any = helper->plugin.module;
	vis = helper->visual;
	
	GG_LIST_REMOVE(helper, h_list);
	if(module.helper->teardown)
		module.helper->teardown(helper);
	
	free(helper);
	ggCloseModule(module.any);
}
