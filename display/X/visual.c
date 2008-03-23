/* $Id: visual.c,v 1.83 2008/03/23 07:50:55 cegger Exp $
******************************************************************************

   LibGGI Display-X target: initialization

   Copyright (C) 1995      Andreas Beck		[becka@ggi-project.org]
   Copyright (C) 1997      Jason McMullan	[jmcc@ggi-project.org]
   Copyright (C) 1998-2000 Marcus Sundberg	[marcus@ggi-project.org]
   Copyright (C) 2002      Brian S. Julin	[bri@tull.umassp.edu]

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
#include <string.h>
#include <ctype.h>

#include "config.h"
#include <ggi/internal/ggi-dl.h>
#include <ggi/internal/ggi_debug.h>
#include <ggi/display/x.h>
#include <ggi/gii.h>
#include <ggi/internal/gg_replace.h>

/* X extension names */
const char *ggi_x_extensions_name[] = 
{
	"MIT-SHM",                        /* X11/extensions/shmstr.h     SHM-NAME          */
	"DOUBLE-BUFFER",                  /* X11/extensions/Xdbeproto.h  DBE_PROTOCOL_NAME */
	"XFree86-DGA",                    /* X11/extensions/xf86dgastr.h XF86DGANAME       */
	"Extended-Visual-Information",    /* X11/extensions/XEVIstr.h    EVINAME           */
	"XFree86-VidModeExtension",       /* X11/extensions/xf86vmstr.h  XF86VIDMODENAME   */
	/* Next to come : RANDR, Render, etc... */
	NULL	/* terminate list */
};

/* Helper names */
const char *ggi_x_helper_name[] = 
{
	"helper-x-shm",
	"helper-x-dbe",
	"helper-x-dga",
	"helper-x-evi",
	"helper-x-vidmode",
	NULL
};

/* Options honored by this target */
static const gg_option optlist[] =
{
	{ "screen", "no" },
	{ "inwin",  "no" },
	{ "fullscreen", "no" },
	{ "noinput", "no" },
	{ "nocursor", "no" },
	{ "noshm", "no"},
	{ "nodbe", "no"},
	{ "nodga", "no"},
	{ "novidmode", "no"},
	{ "noaccel", "no"},
	{ "nomansync", "no"},
	{ "nobuffer", "no"},
	{ "physz", "0,0"},
	{ "keepcursor", "no"}
};

#define OPT_SCREEN	0
#define OPT_INWIN	1
#define OPT_FULLSCREEN	2
#define OPT_NOINPUT	3
#define OPT_NOCURSOR	4
#define OPT_NOSHM	5
#define OPT_NODBE	6
#define OPT_NODGA	7
#define OPT_NOVIDMODE	8
#define OPT_NOACCEL	9
#define OPT_NOMANSYNC	10
#define OPT_NOBUFFER	11
#define OPT_PHYSZ	12
#define OPT_KEEPCURSOR	13

#define NUM_OPTS	(sizeof(optlist)/sizeof(gg_option))

/*
 * Check if the user wants to use the x target
 */ 
static int GGI_X_getapi(struct ggi_visual *vis,int num,
			char *apiname ,char *arguments)
{
	*arguments = '\0';
	switch (num) {
	case 0:
		strcpy(apiname, "display-x");
		return 0;
	case 1:
		strcpy(apiname, "generic-stubs");
		return 0;
	case 2:
		strcpy(apiname, "generic-color");
		return 0;
	}
	return GGI_ENOMATCH;
}

/*
 * Update GC (graphical context) w/r mask value 
 */
void GGI_X_gcchanged(struct ggi_visual *vis, int mask)
{
	ggi_x_priv *priv = GGIX_PRIV(vis);

	/* Handle slave */
	if (priv->slave)
	{
		if ((mask & GGI_GCCHANGED_CLIP)) {
			ggiSetGCClipping(priv->slave->instance.stem,
					 LIBGGI_GC(vis)->cliptl.x,
					 LIBGGI_GC(vis)->cliptl.y, 
					 LIBGGI_GC(vis)->clipbr.x,
					 LIBGGI_GC(vis)->clipbr.y);
		}
		if ((mask & GGI_GCCHANGED_FG)) {
			ggiSetGCForeground(priv->slave->instance.stem, LIBGGI_GC_FGCOLOR(vis));
		}
		if ((mask & GGI_GCCHANGED_BG)) {
			ggiSetGCBackground(priv->slave->instance.stem, LIBGGI_GC_BGCOLOR(vis));
		}

		if (priv->drawable == None) return; /* No Xlib clipping */
	}
	
	/* No slave/standard case */
	if ((mask & GGI_GCCHANGED_CLIP)) {
		GGI_X_LOCK_XLIB(vis);
		_ggi_x_set_xclip(vis, priv->disp, priv->gc,
				 LIBGGI_GC(vis)->cliptl.x, 
				 LIBGGI_GC(vis)->cliptl.y,
				 LIBGGI_GC(vis)->clipbr.x -
				 LIBGGI_GC(vis)->cliptl.x,
				 LIBGGI_GC(vis)->clipbr.y -
				 LIBGGI_GC(vis)->cliptl.y);
		GGI_X_UNLOCK_XLIB(vis);
	}
	if ((mask & GGI_GCCHANGED_FG)) {
		GGI_X_LOCK_XLIB(vis);
		XSetForeground(priv->disp, priv->gc, LIBGGI_GC_FGCOLOR(vis));
		GGI_X_UNLOCK_XLIB(vis);
	}
	if ((mask & GGI_GCCHANGED_BG)) {
		GGI_X_LOCK_XLIB(vis);
		XSetBackground(priv->disp, priv->gc, LIBGGI_GC_BGCOLOR(vis));
		GGI_X_UNLOCK_XLIB(vis);
	}
}

static int GGI_X_setflags(struct ggi_visual *vis, uint32_t flags) {
	ggi_x_priv *priv;
	priv = GGIX_PRIV(vis);
	if ((LIBGGI_FLAGS(vis) & GGIFLAG_ASYNC) && !(flags & GGIFLAG_ASYNC))
		_ggiFlush(vis);
	LIBGGI_FLAGS(vis) = flags;
	/* Unknown flags don't take. */
	LIBGGI_FLAGS(vis) &= GGIFLAG_ASYNC | GGIFLAG_TIDYBUF;
	if (priv->opmansync) {
		MANSYNC_SETFLAGS(vis,flags);
		if ((flags & GGIFLAG_TIDYBUF)
		   && (vis->w_frame)	/* Avoid crash, when flags are set before mode is up */
		   && (vis->w_frame->resource->curactype & GGI_ACTYPE_WRITE))
		{
			MANSYNC_stop(vis);
		}
	}
	return GGI_OK;
}

static void GGI_X_lock_xlib(struct ggi_visual *vis)
{
	ggi_x_priv *priv = GGIX_PRIV(vis);
	ggLock(priv->xliblock);
}

static void GGI_X_unlock_xlib(struct ggi_visual *vis)
{
	ggi_x_priv *priv = GGIX_PRIV(vis);
	if (ggTryLock(priv->flushlock) == 0)
		vis->opdisplay->flush(vis, 0, 0, LIBGGI_VIRTX(vis), LIBGGI_VIRTY(vis), 2);
	ggUnlock(priv->xliblock);
}

/*
 * Close X display
 */
static int GGIclose(struct ggi_visual *vis, struct ggi_dlhandle *dlh)
{
	ggi_x_priv *priv;
	int        i;
	priv = GGIX_PRIV(vis);

	DPRINT_MISC("GGIclose(%p, %p) called\n", vis, dlh);

	if (priv == NULL) goto skip;
	if (priv->disp == NULL) goto skip2;

	XSync(priv->disp,0);

	for (i = 0; ggi_x_helper_name[i] != NULL; i++) {
		if (priv->helper[i]) {
			ggClosePlugin(priv->helper[i]);
			priv->helper[i] = NULL;
		}
	}
	
	if (priv->inp)
		ggClosePlugin(priv->inp);
	priv->inp = NULL;

	DPRINT_MISC("GGIclose: call freefb hook\n");
	if (priv->freefb) {
		priv->freefb(vis);
		priv->freefb = NULL;
	}
	LIB_ASSERT(priv->slave == NULL, "leaking slave target\n");

	if (priv->win != priv->parentwin) {
		/* Don't destroy window, when not created */
		if (priv->win != 0) XDestroyWindow(priv->disp,priv->win);
	}

	/* free saved titles
	 */
	if (priv->windowtitle) {
		free((void *)priv->windowtitle);
		priv->windowtitle=NULL;
	}
	if (priv->icontitle) {
		free((void *)priv->icontitle);
		priv->icontitle=NULL;
	}

	if (!priv->parentwin) goto skip3;

	/* Do special cleanup for -inwin and root windows */
	if ( ! priv->ok_to_resize )  {
		unsigned int dummy;
		Window root;
		int screen;
		XSetWindowAttributes wa;

		DPRINT_MISC("GGIclose: special cleanup for -inwin and root windows\n");
		
		screen = priv->vilist[priv->viidx].vi->screen;
		XGetGeometry(priv->disp, priv->parentwin, &root, (int *)&dummy,
			     (int *)&dummy, &dummy, &dummy, &dummy, &dummy);
		if (priv->parentwin == root) 
			XSetWindowColormap(priv->disp, priv->parentwin,
					   DefaultColormap(priv->disp,screen));
		wa.cursor = priv->oldcursor;
		XChangeWindowAttributes(priv->disp, priv->parentwin, 
					CWCursor, &wa);
		if (priv->oldcursor != None)
			XFreeCursor(priv->disp, priv->oldcursor);
	} else {
		/* Don't destroy window, when not created */
		if (priv->parentwin != 0)
			XDestroyWindow(priv->disp, priv->parentwin);
	}

skip3:
	DPRINT_MISC("GGIclose: free colormaps\n");
	_ggi_x_free_colormaps(vis);

	DPRINT_MISC("GGIclose: free cursor\n");
	if (priv->cursor != None)   XFreeCursor(priv->disp,priv->cursor);

	DPRINT_MISC("GGIclose: free font\n");
	if (priv->textfont != None) XFreeFont(priv->disp, priv->textfont);
	if (priv->fontimg)	    XDestroyImage(priv->fontimg);

	DPRINT_MISC("GGIclose: free X visual and buffers\n");
	if (priv->visual)           XFree(priv->visual);
	if (priv->buflist)	    XFree(priv->buflist);

	DPRINT_MISC("GGIclose: close display\n");
	if (priv->disp)		    XCloseDisplay(priv->disp);

	DPRINT_MISC("GGIclose: free visual and mode list\n");
	if (priv->vilist)	    free(priv->vilist);
	if (priv->modes)	    free(priv->modes);

	DPRINT_MISC("GGIclose: free mansync\n");
	if (priv->opmansync)	    free(priv->opmansync);
 skip2:
	DPRINT_MISC("GGIclose: destroy flushlock\n");
	if (priv->flushlock)	    ggLockDestroy(priv->flushlock);
	DPRINT_MISC("GGIclose: destroy xliblock\n");
	if (priv->xliblock)	    ggLockDestroy(priv->xliblock);
	free(priv);
 skip:
	DPRINT_MISC("GGIclose: free GC\n");
	if (LIBGGI_GC(vis) != NULL) free(LIBGGI_GC(vis));
	DPRINT_MISC("X-target closed\n");
	return 0;
}

/* prototypes for functions in mode.c that we need in 
 * order to initialize xpriv->cm_adjust and xpriv->cm_adapt */
void _GGI_X_checkmode_adapt( ggi_mode * m,
			     ggi_x_vi * vi,
			     ggi_x_priv * priv );
void _GGI_X_checkmode_adjust( ggi_mode *req,
			      ggi_mode *sug,
			      ggi_x_priv *priv );

/*
 * Open X target
 */
static int GGIopen(struct ggi_visual *vis, struct ggi_dlhandle *dlh,
		   const char *args, void *argptr, uint32_t *dlret)

{
	int err, i, tmp1, tmp2, tmp3;  /* Return value, scratch variables */
	XVisualInfo	vi_template;
	long		vi_mask;
	gg_option 	options[NUM_OPTS];
	ggi_x_priv 	*priv;
	Display 	*disp;
	void 		*lock;
	Bool        xerr;
	memcpy(options, optlist, sizeof(options));

	if (args) {
		args = ggParseOptions(args, options, NUM_OPTS);
		if (args == NULL) {
			fprintf(stderr, "display-x: error in arguments.\n");
			return GGI_EARGINVAL;
		}
	}

	err = GGI_ENOMEM;

	/* Create our visual's GC and it's private data area */
	LIBGGI_GC(vis) = calloc(1, sizeof(ggi_gc));
	if (LIBGGI_GC(vis) == NULL) goto out;
	priv = calloc(1, sizeof(ggi_x_priv));
	if (priv == NULL) goto out; /* Allocation failed */
	LIBGGI_PRIVATE(vis) = priv;
	vis->gamma = &(priv->gamma);
	
	/* Init the title defaults */
	priv->windowtitle=NULL;
	priv->icontitle  =NULL;
	
	/* Init the flipping offset */
	priv->pf_offset=0;

	/* Create a lock to regularly flush */
	lock = ggLockCreate();
	if (lock == NULL) goto out;
	ggLock(lock);
	priv->flushlock = lock;
	priv->lock_xlib    = GGI_X_lock_xlib;
	priv->unlock_xlib = GGI_X_unlock_xlib;

	/* Create a lock to prevent concurrent Xlib access */
	lock = ggLockCreate();
	if (lock == NULL) goto out;
	priv->xliblock = lock;

	/* Obtain the X11 display handle */
	DPRINT_MISC("X: want display %s\n", args);
	disp = XOpenDisplay(args);
	if (disp == NULL) goto out;
	DPRINT_MISC("X: have display %s\n", DisplayString(disp));
	DPRINT_MISC("X: number of screens on this display: %i\n",
			ScreenCount(disp));
	DPRINT_MISC("X: defaultscreen on this display: %i\n",
			DefaultScreen(disp));
	priv->disp = disp;


	/* Link in some functions that would otherwise need symbol entries */
	priv->acquire = GGI_X_db_acquire;
	priv->release = GGI_X_db_release;
	priv->flush_cmap = _ggi_x_flush_cmap;

	/* See what extensions are available on this display. */
	for(i = 0; ggi_x_extensions_name[i] != NULL; ++i) {
		xerr = XQueryExtension(disp, ggi_x_extensions_name[i], &tmp1, &tmp2, &tmp3);
		if (tmp1 && (xerr == True)) {
			DPRINT_MISC("%s X extension found\n", ggi_x_extensions_name[i]);
			priv->use_Xext |= 1 << i;
		} else {
			DPRINT_MISC("no %s extension\n", ggi_x_extensions_name[i]);
		}
	}

	if (options[OPT_FULLSCREEN].result[0] == 'n') {
		/* Don't use DGA and VIDMODE if fullscreen is not requested */
		priv->use_Xext &= ~GGI_X_USE_DGA;
		priv->use_Xext &= ~GGI_X_USE_VIDMODE;
	} else {
		/* Turn off inwin option, when we go to fullscreen */
		options[OPT_INWIN].result[0] = 'n';
	}

	/* Turn off extensions which the user does not want */
	if ((options[OPT_NOSHM].result[0] != 'n') ||
	    (options[OPT_NOBUFFER].result[0] != 'n'))
		priv->use_Xext &= ~GGI_X_USE_SHM;
	if (options[OPT_NODBE].result[0] != 'n')
		priv->use_Xext &= ~GGI_X_USE_DBE;
	if (options[OPT_NODGA].result[0] != 'n')
		priv->use_Xext &= ~GGI_X_USE_DGA;
	if (options[OPT_NOVIDMODE].result[0] != 'n') 
		priv->use_Xext &= ~GGI_X_USE_VIDMODE;
	/* Xevi disabled until EVIGetVisualInfo BadLength fixed. */
	priv->use_Xext &= ~GGI_X_USE_EVI;

	/* DGA and Vidmode are exclusive to each other
	 * Some cards don't handle DGA (namely nvidia).
	 * And it seems this extension is less and less used.
	 * This case is handled below.
	 */
	if(priv->use_Xext & GGI_X_USE_VIDMODE)
	{
		priv->use_Xext &=~GGI_X_USE_DGA;
	}
	
	if(priv->use_Xext & GGI_X_USE_DGA)
	{
		priv->use_Xext &=~GGI_X_USE_VIDMODE; /* DGA has own mode support */
		priv->use_Xext &=~GGI_X_USE_SHM;     /* DGA precludes SHM */
	}

	
	/* Get a list of possibly compatible X11 visuals */
	memset(&vi_template, 0, sizeof(XVisualInfo));
	vi_mask = VisualNoMask;

	if (options[OPT_INWIN].result[0] != 'r' ||
	    options[OPT_INWIN].result[1] != 'o' ||
	    options[OPT_INWIN].result[2] != 'o' ||
	    options[OPT_INWIN].result[3] != 't' ||
	    options[OPT_INWIN].result[4] != '\0'
	    ) {

		priv->ok_to_resize = 1;
		if (options[OPT_INWIN].result[0] != 'n') {
			/* We will be using an already existing window */
			XWindowAttributes wa;
			priv->parentwin = priv->win =
				strtol(options[OPT_INWIN].result, NULL, 0);
			DPRINT_MISC("X: using window id 0x%x\n", 
				       priv->parentwin);

			/* Get window attributes */
			vi_mask = VisualScreenMask;
			XGetWindowAttributes(priv->disp, priv->parentwin, &wa);
			vi_template.screen = XScreenNumberOfScreen(wa.screen);

			/* The windows is not resizable */
			priv->ok_to_resize = 0;
		}
	} else {
		/* We will be using the root window */
		XWindowAttributes wa;
		
		/* Was the screen explicitely specified? */
		if (options[OPT_SCREEN].result[0] != 'n') {
			vi_template.screen =
				strtoul(options[OPT_SCREEN].result, NULL, 0);
		} else {
			vi_template.screen = DefaultScreen(priv->disp);
		}

		priv->win = priv->parentwin = 
			RootWindow(priv->disp, vi_template.screen);
		XGetWindowAttributes(priv->disp, priv->win, &wa);
		vi_mask = VisualIDMask | VisualScreenMask;
		vi_template.visualid = XVisualIDFromVisual(wa.visual);
		priv->ok_to_resize = 0; /* The root window is not resizable */

		DPRINT_MISC("X: using root window of screen %u\n", 
			       vi_template.screen);
	}

	/*
	 * Todo : add a hook to get the visual info list (usefull for some target, and ggigl) 
	 */
	priv->visual = XGetVisualInfo(priv->disp, vi_mask, 
				      &vi_template, &priv->nvisuals);
	if (priv->visual == NULL || priv->nvisuals <= 0) {
		DPRINT("X: No acceptable X11 visuals.\n");
		err = GGI_ENOFUNC;
		goto out;
	}
	priv->vilist = calloc((size_t)priv->nvisuals, sizeof(ggi_x_vi));
	if (priv->vilist == NULL) goto out;

	priv->buflist = XListPixmapFormats(disp, &(priv->nbufs));
	if (priv->buflist == NULL) goto out;

	DPRINT_MISC("X: Sort/complete visual list.\n");
	_ggi_x_build_vilist(vis);	/* Sorts/completes vilist */


	/* Parse physical size options */
	err = _ggi_physz_parse_option(options[OPT_PHYSZ].result, 
			       &(priv->physzflags), 
			       &(priv->physz)); 
	if (err != GGI_OK) goto out;

	/* Hook default functions to create the cursor */
	if (options[OPT_KEEPCURSOR].result[0] == 'n') {
		priv->createcursor = (options[OPT_NOCURSOR].result[0] == 'n') ?
		  _ggi_x_create_dot_cursor : _ggi_x_create_invisible_cursor; 
	}

	/* Set our default displayops. Helper libs may override. */
	vis->opdisplay->getmode  = GGI_X_getmode;
	vis->opdisplay->getapi   = GGI_X_getapi;
	vis->opdisplay->setflags = GGI_X_setflags;
	
	vis->opdisplay->checkmode = GGI_X_checkmode;
	vis->opdisplay->setmode = GGI_X_setmode;

	/* An extension might want to overload these to provide different
	 * Visual-based modes. */
	priv->cm_adapt = _GGI_X_checkmode_adapt;
	priv->cm_adjust = _GGI_X_checkmode_adjust;

	/* Try the extensions that haven't been disabled. */
	DPRINT_MISC("X: Load X extensions.\n");

	/* Order is important here -- XCloseDisplay has sharp hooks! */
	for(i=0; i<GGI_X_HELPER_COUNT; ++i)
	{
		if(!(priv->use_Xext & (1<<i))) continue;
		priv->helper[i] = ggPlugModule(libggi, vis->instance.stem, 
		                               ggi_x_helper_name[i], NULL, NULL);
		if(priv->helper[i] == NULL)
		{
			fprintf(stderr, "X: Cannot load %s\n", ggi_x_helper_name[i]);
			priv->use_Xext &= ~(1<<i);
		}
	}

	/* Make the unavoidable adjustements */
	if(priv->use_Xext & GGI_X_USE_SHM)
	{
		priv->shmhack_free_cmaps = _ggi_x_free_colormaps;
	}

	if(priv->use_Xext & GGI_X_USE_EVI)
	{
		/* See if Xevi disqualified all the visuals (should not happen) */
		tmp1 = tmp2 = 0;
		while (tmp1 < priv->nvisuals) {
			tmp2 += !(priv->vilist[tmp1++].flags && GGI_X_VI_NON_FB);
		}
		if (!tmp2) {
			DPRINT("X: No acceptable X11 visuals.\n");
			err = GGI_ENOFUNC;
			goto out;
		}
	}
	
 	priv->createdrawable = GGI_X_create_window_drawable;

	/* Use no intermediate drawable? */
	if (options[OPT_NOBUFFER].result[0] != 'n') {
		priv->createfb = NULL;
		priv->freefb = NULL;
		goto nomansync;
	}
	if (!priv->createfb) {
		priv->createfb = _ggi_x_createfb;
		priv->freefb = _ggi_x_freefb;
	}

	/* See if we want/need/have mansync */
	if (options[OPT_NOMANSYNC].result[0] != 'n') goto nomansync;
	if (priv->use_Xext & GGI_X_USE_DGA) goto nomansync; 
	if (!(priv->use_Xext & GGI_X_USE_SHM) &&
	    options[OPT_NOBUFFER].result[0] != 'n') goto nomansync;

	priv->opmansync = malloc(sizeof(_ggi_opmansync));
	if (priv->opmansync == NULL) {
		goto out;
	}
	MANSYNC_open(vis, priv);
	if (priv->mod_mansync == NULL) {
		fprintf(stderr,
			"display-X: Cannot load required helper-mansync!\n");
		err = GGI_ENODEVICE;
		goto out;
	}

 nomansync:
	if (options[OPT_NOACCEL].result[0] != 'n') {
		if (priv->createfb == NULL) {
			DPRINT("X: No rendering path!\n");
			err = GGI_ENODEVICE;
			goto out;
		}
		DPRINT("disabling drawable\n");
		priv->createdrawable = NULL;
	}

	/* Load default font */
	if (priv->createdrawable != NULL)
	{
		priv->textfont = XLoadQueryFont(disp, "fixed");
		if (priv->textfont != NULL) {
			DPRINT_MISC("Xlib: using font with "
				       "dimension %dx%d\n",
				       priv->textfont->max_bounds.width,
				       priv->textfont->max_bounds.ascent
				       + priv->textfont->max_bounds.descent);
		}
    }

	/* Plug input */
	if (tolower((uint8_t)options[OPT_NOINPUT].result[0]) == 'n') {
		struct gii_inputxwin_arg _args;
		struct gg_instance *inp = NULL;
		struct gg_api *gii;

		_args.disp = priv->disp;
		_args.wait = 1;
		_args.exposearg = vis;
                _args.resizearg = vis;
		_args.lockfunc = (gii_inputxwin_lockfunc*)priv->lock_xlib;
                _args.lockarg = vis;
		_args.unlockfunc = (gii_inputxwin_unlockfunc*)priv->unlock_xlib;
                _args.unlockarg = vis;
                
		gii = ggGetAPIByName("gii");
		if (gii != NULL && STEM_HAS_API(vis->instance.stem, gii)) {
			inp = ggPlugModule(gii,
					   vis->instance.stem,
					   "input-xwin",
					   NULL,
					   &_args);
			ggObserve(inp->channel, GGI_X_listener, vis);
		}

		DPRINT_MISC("X: ggPlugModule returned with %p\n", inp);

		if (inp == NULL) {
			DPRINT_MISC("Unable to open xwin inputlib\n");
			err = GGI_ENODEVICE;
			goto out;
		}

		priv->inp = inp;
	}
	else {
		priv->inp = NULL;
	}


	if (priv->opmansync) {
		MANSYNC_init(vis);
		if (!(LIBGGI_FLAGS(vis) & GGIFLAG_ASYNC)) {
			MANSYNC_start(vis);
		}
	}

	DPRINT_MISC("X-target fully up\n");

	*dlret = GGI_DL_OPDISPLAY;
	return 0;

#undef GGI_X_TEST_XEXT
 out:
	GGIclose(vis, dlh);
	return err;
}

static int GGIexit(struct ggi_visual *vis, struct ggi_dlhandle *dlh)
{
	LIB_ASSERT(vis != NULL, "GGIexit: vis == NULL");
	LIB_ASSERT(GGIX_PRIV(vis) != NULL, "GGIexit: GGIX_PRIV(vis) == NULL");

	if (GGIX_PRIV(vis)->opmansync) {
		if (!(LIBGGI_FLAGS(vis) & GGIFLAG_ASYNC)) {
			MANSYNC_stop(vis);
		}
		MANSYNC_deinit(vis);
		MANSYNC_close(GGIX_PRIV(vis));
	}
	return 0;
}

EXPORTFUNC
int GGIdl_X(int func, void **funcptr);

int GGIdl_X(int func, void **funcptr)
{
	ggifunc_open **openptr;
	ggifunc_exit **exitptr;
	ggifunc_close **closeptr;

	switch (func) {
	case GGIFUNC_open:
		openptr = (ggifunc_open **)funcptr;
		*openptr = GGIopen;
		return 0;
	case GGIFUNC_exit:
		exitptr = (ggifunc_exit **)funcptr;
		*exitptr = GGIexit;
		return 0;
	case GGIFUNC_close:
		closeptr = (ggifunc_close **)funcptr;
		*closeptr = GGIclose;
		return 0;
	default:
		*funcptr = NULL;
	}

	return GGI_ENOTFOUND;
}
