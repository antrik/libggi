/* $Id: visual.c,v 1.9 2002/07/08 09:44:11 cegger Exp $
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

#include <ggi/internal/ggi-dl.h>
#include <ggi/display/x.h>

/* Options honored by this target */
static const gg_option optlist[] =
{
	{ "screen", "no" },
	{ "inwin",  "no" },
	{ "noinput", "no" },
	{ "nocursor", "no" },
	{ "noshm", "no"},
	{ "nodbe", "no"},
	{ "nodga", "no"},
	{ "novidmode", "no"},
	{ "noaccel", "no"},
	{ "nomansync", "no"},
	{ "nobuffer", "no"},
	{ "physz", "0,0"}
};

#define OPT_SCREEN	0
#define OPT_INWIN	1
#define OPT_NOINPUT	2
#define OPT_NOCURSOR	3
#define OPT_NOSHM	4
#define OPT_NODBE	5
#define OPT_NODGA	6
#define OPT_NOVIDMODE	7
#define OPT_NOACCEL	8
#define OPT_NOMANSYNC	9
#define OPT_NOBUFFER	10
#define OPT_PHYSZ	11

#define NUM_OPTS	(sizeof(optlist)/sizeof(gg_option))


int GGI_X_getapi(ggi_visual *vis,int num, char *apiname ,char *arguments)
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
	return -1;
}

void GGI_X_gcchanged(ggi_visual *vis, int mask) {
	ggi_x_priv *priv;
	priv = LIBGGI_PRIVATE(vis);

	if (!priv->slave) goto noslave;
	if ((mask & GGI_GCCHANGED_CLIP)) {
		ggiSetGCClipping(priv->slave,
				 LIBGGI_GC(vis)->cliptl.x,
				 LIBGGI_GC(vis)->cliptl.y, 
				 LIBGGI_GC(vis)->clipbr.x,
				 LIBGGI_GC(vis)->clipbr.y);
	}
	if ((mask & GGI_GCCHANGED_FG)) {
		ggiSetGCForeground(priv->slave, LIBGGI_GC_FGCOLOR(vis));
	}
	if ((mask & GGI_GCCHANGED_BG)) {
		ggiSetGCBackground(priv->slave, LIBGGI_GC_BGCOLOR(vis));
	}

	if (priv->drawable == None) return; /* No Xlib clipping */

 noslave:
	if ((mask & GGI_GCCHANGED_CLIP)) {
		ggLock(priv->xliblock);
		_ggi_x_set_xclip(priv->disp, priv->gc,
				 LIBGGI_GC(vis)->cliptl.x, 
				 LIBGGI_GC(vis)->cliptl.y,
				 LIBGGI_GC(vis)->clipbr.x -
				 LIBGGI_GC(vis)->cliptl.x,
				 LIBGGI_GC(vis)->clipbr.y -
				 LIBGGI_GC(vis)->cliptl.y);
		ggUnlock(priv->xliblock);
	}
	if ((mask & GGI_GCCHANGED_FG)) {
		ggLock(priv->xliblock);
		XSetForeground(priv->disp, priv->gc, LIBGGI_GC_FGCOLOR(vis));
		ggUnlock(priv->xliblock);
	}
	if ((mask & GGI_GCCHANGED_BG)) {
		ggLock(priv->xliblock);
		XSetBackground(priv->disp, priv->gc, LIBGGI_GC_BGCOLOR(vis));
		ggUnlock(priv->xliblock);
	}
}

static int GGI_X_setflags(ggi_visual *vis, ggi_flags flags) {
	ggi_x_priv *priv;
	priv = GGIX_PRIV(vis);
	if ((LIBGGI_FLAGS(vis) & GGIFLAG_ASYNC) && !(flags & GGIFLAG_ASYNC)) 
		ggiFlush(vis);
	LIBGGI_FLAGS(vis) = flags;
	if (priv->opmansync) MANSYNC_SETFLAGS(vis,flags);
	LIBGGI_FLAGS(vis) &= GGIFLAG_ASYNC; /* Unknown flags don't take. */
	return GGI_OK;
}

static int GGIclose(ggi_visual *vis, struct ggi_dlhandle *dlh)
{
	ggi_x_priv *priv;
	priv = GGIX_PRIV(vis);

	if (priv == NULL) goto skip;
	if (priv->disp == NULL) goto skip2;

	XSync(priv->disp,0);

	if (priv->slave) ggiClose(priv->slave);
	priv->slave = NULL;

	if (priv->freefb) priv->freefb(vis);

	/* Exit any initialized helper libs if called from GGIopen. */
	if (vis->extlib) {
		_ggiExitDL(vis, vis->extlib);
		_ggiZapDL(vis, &vis->extlib);
	}

	if (priv->win != priv->parentwin) XDestroyWindow(priv->disp,priv->win);
	if (!priv->parentwin) goto skip3;

	/* Do special cleanup for -inwin and root windows */
	if (vis->opdisplay->checkmode == GGI_X_checkmode_fixed) {
		unsigned int dummy;
		Window root;
		int screen;
		XSetWindowAttributes wa;
		
		screen = priv->vilist[priv->viidx].vi->screen;
		XGetGeometry(priv->disp, priv->parentwin, &root, (int *)&dummy,
			     (int *)&dummy, &dummy, &dummy, &dummy, &dummy);
		if (priv->parentwin == root) 
			XSetWindowColormap(priv->disp, priv->parentwin,
					   DefaultColormap(priv->disp,screen));
		wa.cursor = None;
		XChangeWindowAttributes(priv->disp, priv->parentwin, 
					CWCursor, &wa);
	} else  XDestroyWindow(priv->disp, priv->parentwin);

skip3:
	_ggi_x_free_colormaps(vis);

	if (priv->cursor != None)   XFreeCursor(priv->disp,priv->cursor);
	if (priv->textfont != None) XFreeFont(priv->disp, priv->textfont);
	if (priv->fontimg)	    XDestroyImage(priv->fontimg);
	if (priv->visual)           XFree(priv->visual);
	if (priv->buflist)	    XFree(priv->buflist);
	if (priv->disp)		    XCloseDisplay(priv->disp);
	if (priv->vilist)	    free(priv->vilist);
	if (priv->modes)	    free(priv->modes);
	if (priv->opmansync)	    free(priv->opmansync);
 skip2:
	if (priv->xliblock)	    ggLockDestroy(priv->xliblock);
	free(priv);
 skip:
	if (LIBGGI_GC(vis) != NULL) free(LIBGGI_GC(vis));
	GGIDPRINT_MISC("X-target closed\n");
	return 0;
}

static int GGIopen(ggi_visual *vis, struct ggi_dlhandle *dlh,
		   const char *args, void *argptr, uint32 *dlret)

{
	int err, tmp1, tmp2, tmp3;  /* Return value, scratch variables */
	char 		*tmpstr;
	XVisualInfo	vi_template;
	long		vi_mask;
	gg_option 	options[NUM_OPTS];
	ggi_x_priv 	*priv;
	Display 	*disp;
	void 		*lock;

	memcpy(options, optlist, sizeof(options));

	if (args) {
		args = ggParseOptions((char *)args, options, NUM_OPTS);
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
	if (priv == NULL) goto out;
	LIBGGI_PRIVATE(vis) = priv;
	vis->gamma = &(priv->gamma);

	/* Create a lock to prevent concurrent Xlib access */
	lock = ggLockCreate();
	if (lock == NULL) goto out;
	priv->xliblock = lock;

	/* Obtain the X11 display handle */
	GGIDPRINT_MISC("X: want display %s\n", args);
	disp = XOpenDisplay(args);
	if (disp == NULL) goto out;
	GGIDPRINT_MISC("X: have display %s\n", args);
	priv->disp = disp;

	/* Link in some functions that would otherwise need symbol entries */
	priv->acquire = GGI_X_db_acquire;
	priv->release = GGI_X_db_release;
	priv->flush_cmap = _ggi_x_flush_cmap;

	/* See what extensions are available on this display. */
#define GGI_X_CHECK_XEXT(extname, flag)					\
	tmpstr = extname;						\
	XQueryExtension(disp, tmpstr, &tmp1, &tmp2, &tmp3);		\
	if (tmp1) {							\
		GGIDPRINT_MISC("%s X extension found\n", tmpstr);	\
		priv->use_Xext |= flag;					\
	}								\
	else GGIDPRINT_MISC("no %s extension\n", tmpstr)
	
	GGI_X_CHECK_XEXT("Extended-Visual-Information",	GGI_X_USE_EVI);
	GGI_X_CHECK_XEXT("MIT-SHM",			GGI_X_USE_SHM);
	GGI_X_CHECK_XEXT("DOUBLE-BUFFER",		GGI_X_USE_DBE);
	GGI_X_CHECK_XEXT("XFree86-DGA",			GGI_X_USE_DGA);
	GGI_X_CHECK_XEXT("XFree86-VidModeExtension",	GGI_X_USE_VIDMODE);

	/* Turn off extensions which the user does not want */
	if (options[OPT_NOSHM].result[0] != 'n') 
		priv->use_Xext &=~GGI_X_USE_SHM;
	if (options[OPT_NODBE].result[0] != 'n')
		priv->use_Xext &=~GGI_X_USE_DBE;
	if (options[OPT_NODGA].result[0] != 'n')
		priv->use_Xext &=~GGI_X_USE_DGA;
	if (options[OPT_NOVIDMODE].result[0] != 'n') 
		priv->use_Xext &=~GGI_X_USE_VIDMODE;

	/* Get a list of possibly compatible X11 visuals */
	memset(&vi_template, 0, sizeof(XVisualInfo));
	vi_mask = VisualNoMask;

	if (options[OPT_INWIN].result[0] != 'r' ||
	    options[OPT_INWIN].result[1] != 'o' ||
	    options[OPT_INWIN].result[2] != 'o' ||
	    options[OPT_INWIN].result[3] != 't' ||
	    options[OPT_INWIN].result[4] != '\0'
	    ) {
		/* DGA and Vidmode are only for root-window visuals */
		priv->use_Xext &= ~GGI_X_USE_DGA;
		priv->use_Xext &= ~GGI_X_USE_VIDMODE;
		vis->opdisplay->checkmode = GGI_X_checkmode_normal;
		vis->opdisplay->setmode = GGI_X_setmode_normal;
		if (options[OPT_INWIN].result[0] != 'n') {
			XWindowAttributes wa;
			priv->parentwin = priv->win =
				strtol(options[OPT_INWIN].result, NULL, 0);
			GGIDPRINT_MISC("X: using window id 0x%x\n", 
				       priv->parentwin);
			vi_mask = VisualScreenMask;
			XGetWindowAttributes(priv->disp, priv->parentwin, &wa);
			vi_template.screen = XScreenNumberOfScreen(wa.screen);
			vis->opdisplay->checkmode = GGI_X_checkmode_fixed;
			vis->opdisplay->setmode = GGI_X_setmode_fixed;
		}
	} else {
		XWindowAttributes wa;
		vi_template.screen = (options[OPT_SCREEN].result[0] != 'n')  ? 
		  strtoul(options[OPT_SCREEN].result, NULL, 0) : 
		  DefaultScreen(priv->disp);
		priv->win = priv->parentwin = 
			RootWindow(priv->disp, vi_template.screen);
		XGetWindowAttributes(priv->disp, priv->win, &wa);
		vi_mask = VisualIDMask | VisualScreenMask;
		vi_template.visualid = XVisualIDFromVisual(wa.visual);
		vis->opdisplay->checkmode = GGI_X_checkmode_fixed;
		vis->opdisplay->setmode = GGI_X_setmode_fixed;

		GGIDPRINT_MISC("X: using root window of screen %ui\n", 
			       vi_template.screen);
	}

	priv->visual = XGetVisualInfo(priv->disp, vi_mask, 
				      &vi_template, &priv->nvisuals);
	if (priv->visual == NULL || priv->nvisuals <= 0) {
		GGIDPRINT("X: No acceptable X11 visuals.\n");
		err = GGI_ENOFUNC;
		goto out;
	}
	priv->vilist = calloc(priv->nvisuals, sizeof(ggi_x_vi));
	if (priv->vilist == NULL) goto out;

	priv->buflist = XListPixmapFormats(disp, &(priv->nbufs));
	if (priv->buflist == NULL) goto out;

	_ggi_x_build_vilist(vis);	/* Sorts/completes vilist */

	/* Parse physical size options */
	err = _ggi_parse_physz(options[OPT_PHYSZ].result, 
			       &(priv->physzflags), 
			       &(priv->physz)); 
	if (err != GGI_OK) goto out;

	/* Hook default functions to create the cursor */
	priv->createcursor = (options[OPT_NOCURSOR].result[0] == 'n') ?
	  _ggi_x_create_dot_cursor : _ggi_x_create_invisible_cursor; 

	/* Set the rest of our default displayops. Helper libs may override. */
	vis->opdisplay->getmode  = GGI_X_getmode;
	vis->opdisplay->getapi   = GGI_X_getapi;
	vis->opdisplay->setflags = GGI_X_setflags;

	/* Try the extensions that haven't been disabled. */
#define GGI_X_TEST_XEXT(flag, helper, abort_label)		\
	if (!(priv->use_Xext & flag)) goto abort_label;		\
	tmpstr = helper;					\
	err = _ggiAddDL(vis, tmpstr, NULL, NULL, 0);		\
	if (err) {						\
		fprintf(stderr, "X: Cannot load %s\n", tmpstr);	\
		priv->use_Xext &= ~flag;			\
		goto abort_label;				\
	}

	/* Order is important here -- XCloseDisplay has sharp hooks! */

	GGI_X_TEST_XEXT(GGI_X_USE_DGA, "helper-x-dga", nodga);
	priv->use_Xext &=~GGI_X_USE_VIDMODE; /* DGA has own mode support */
	priv->use_Xext &=~GGI_X_USE_SHM;     /* DGA precludes SHM */

 nodga:
	if (options[OPT_NOBUFFER].result[0] != 'n') goto noshm;
	GGI_X_TEST_XEXT(GGI_X_USE_SHM, "helper-x-shm", noshm);
	priv->shmhack_free_cmaps = _ggi_x_free_colormaps;
	priv->shmhack_checkmode_fixed = GGI_X_checkmode_fixed;

 noshm:
	priv->use_Xext &= ~GGI_X_USE_EVI;
	goto noevi; /* Xevi disabled until EVIGetVisualInfo BadLength fixed. */
	GGI_X_TEST_XEXT(GGI_X_USE_EVI, "helper-x-evi", noevi);
	/* See if Xevi disqualified all the visuals (should not happen) */
	tmp1 = tmp2 = 0;
	while (tmp1 < priv->nvisuals) {
		tmp2 += !(priv->vilist[tmp1++].flags && GGI_X_VI_NON_FB);
	}
	if (!tmp2) {
		GGIDPRINT("X: No acceptable X11 visuals.\n");
		err = GGI_ENOFUNC;
		goto out;
	}

 noevi:
	GGI_X_TEST_XEXT(GGI_X_USE_VIDMODE, "helper-x-vidmode", novidmode);

 novidmode:
	GGI_X_TEST_XEXT(GGI_X_USE_DBE, "helper-x-dbe", nodbe);

 nodbe:
	priv->createdrawable = GGI_X_create_window_drawable;

	if (options[OPT_NOBUFFER].result[0] != 'n') {
		priv->createfb = NULL;
		priv->freefb = NULL;
		goto nomansync;
	}
	if (!priv->createfb) {
		priv->createfb = _ggi_x_create_ximage;
		priv->freefb = _ggi_x_free_ximage;
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
	err = _ggiAddDL(vis, "helper-mansync", NULL, priv->opmansync, 0);
	if (err) {
		fprintf(stderr,
			"display-X: Cannot load required helper-mansync!\n");
		goto out;
	}

 nomansync:
	if (options[OPT_NOACCEL].result[0] != 'n') {
		if (priv->createfb == NULL) {
			GGIDPRINT("X: No rendering path!\n");
			err = GGI_ENODEVICE;
			goto out;
		}
		fprintf(stderr,"disabling drawable\n");
		priv->createdrawable = NULL;
	}

	if (priv->createdrawable != NULL) {
		priv->textfont = XLoadQueryFont(disp, "fixed");
		if (priv->textfont != NULL)
                	GGIDPRINT_MISC("Xlib: using font with "
				       "dimension %dx%d\n",
				       priv->textfont->max_bounds.width,
				       priv->textfont->max_bounds.ascent
				       + priv->textfont->max_bounds.descent);
        }

	if (tolower((int)options[OPT_NOINPUT].result[0]) == 'n') {
		gii_inputxwin_arg args;
		gii_input *inp;
                
		args.disp = priv->disp;
		args.ptralwaysrel = 0;
		args.wait = 1;
                args.exposefunc = (gii_inputxwin_exposefunc*)GGI_X_expose;
                args.exposearg = vis;
      /* args.resizefunc = (gii_inputxwin_resizefunc*)GGI_X_resize;*/
		args.resizefunc = NULL;
                args.resizearg = vis;
		args.gglock = lock;
                
		if ((inp = giiOpen("xwin", &args, NULL)) == NULL) {
			GGIDPRINT_MISC("Unable to open xwin inputlib\n");
			GGIclose(vis, dlh);
			return GGI_ENODEVICE;
		}

		priv->inp = inp;
		/* Now join the new event source in. */
		vis->input = giiJoinInputs(vis->input, inp);
	}
	else {
		priv->inp = NULL;
	}


	if (priv->opmansync) MANSYNC_init(vis);

	GGIDPRINT_MISC("X-target fully up\n");

	*dlret = GGI_DL_OPDISPLAY;
	return 0;

 out:
	GGIclose(vis, dlh);
	return err;
}

static int GGIexit(ggi_visual *vis, struct ggi_dlhandle *dlh)
{
	if (GGIX_PRIV(vis)->opmansync) MANSYNC_deinit(vis);	
	return 0;
}

int GGIdl_X(int func, void **funcptr)
{
	switch (func) {
	case GGIFUNC_open:
		*funcptr = GGIopen;
		return 0;
	case GGIFUNC_exit:
		*funcptr = GGIexit;
		return 0;
	case GGIFUNC_close:
		*funcptr = GGIclose;
		return 0;
	default:
		*funcptr = NULL;
	}

	return GGI_ENOTFOUND;
}

#include <ggi/internal/ggidlinit.h>
