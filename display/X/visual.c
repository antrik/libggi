/* $Id: visual.c,v 1.5 2001/07/31 08:15:25 cegger Exp $
******************************************************************************

   Display-X: initialization

   Copyright (C) 1995      Andreas Beck		[becka@ggi-project.org]
   Copyright (C) 1997      Jason McMullan	[jmcc@ggi-project.org]
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

#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include <ggi/internal/ggi-dl.h>
#include <ggi/display/x.h>

#define GGI_X_TARGET
#include "./visual.inc"

static const gg_option optlist[] =
{
	{ "inroot", "no" },
	{ "inwin",  "no" },
	{ "noinput", "no" },
	{ "nocursor", "no" },
	{ "physz", "0,0"}
};

#define OPT_INROOT	0
#define OPT_INWIN	1
#define OPT_NOINPUT	2
#define OPT_NOCURSOR	3
#define OPT_PHYSZ	4

#define NUM_OPTS	(sizeof(optlist)/sizeof(gg_option))


void _GGI_X_freedbs(ggi_visual *vis, ggi_x_priv *priv) {
	int i;
	int first = LIBGGI_APPLIST(vis)->first_targetbuf;
	int last = LIBGGI_APPLIST(vis)->last_targetbuf;

	if (first < 0) {
		return;
	}
	for (i = (last - first); i >= 0; i--) {

		if (priv->ximage_list[i]) {
			XDestroyImage(priv->ximage_list[i]);
			priv->ximage_list[i] = NULL;
		}

#ifdef HAVE_SHM
		if (priv->shminfo[i].shmid != -1) {
			XShmDetach(priv->xwin.x.display, &(priv->shminfo[i]));
			GGIDPRINT_MISC("_GGI_X_freedbs: XShmDetach(%p, %d)\n",
			       priv->xwin.x.display, priv->shminfo[i]);
			if (LIBGGI_APPLIST(vis)->bufs[i]->write != NULL)
				shmdt(LIBGGI_APPLIST(vis)->bufs[i]->write);
			priv->shminfo[i].shmid = -1;
		}
#endif
		_ggi_db_free(LIBGGI_APPLIST(vis)->bufs[i+first]);
		_ggi_db_del_buffer(LIBGGI_APPLIST(vis), i+first);
	}
	LIBGGI_APPLIST(vis)->first_targetbuf = -1;
	priv->ximage = NULL;
}


static int GGIclose(ggi_visual *vis, struct ggi_dlhandle *dlh)
{
	ggi_x_priv *priv = LIBGGI_PRIVATE(vis);

	if (priv->xwin.x.cmap) {
		if (priv->xwin.wintype == GGIX_ROOT)
		  XSetWindowColormap(priv->xwin.x.display, priv->xwin.window,
				     DefaultColormap(priv->xwin.x.display,
						     priv->xwin.x.screen));
		XFreeColormap(priv->xwin.x.display, priv->xwin.x.cmap);
	}
	_GGI_X_freedbs(vis, priv);
	if (priv->xwin.x.gc) XFreeGC(priv->xwin.x.display, priv->xwin.x.gc);
	if (priv->xwin.window && priv->xwin.wintype == GGIX_NORMAL) {
		XDestroyWindow(priv->xwin.x.display, priv->xwin.window);
	}
	if (priv->xwin.cursor != None) {
		XFreeCursor(priv->xwin.x.display, priv->xwin.cursor);
	}
	XSync(priv->xwin.x.display,0);
	XCloseDisplay(priv->xwin.x.display);

	free(priv->opmansync);
	
	ggLockDestroy(priv->xwin.x.xliblock);
	
	free(priv);

	free(LIBGGI_GC(vis));

	GGIDPRINT_MISC("X-target closed\n");
	
	return 0;
}


static int GGIopen(ggi_visual *vis, struct ggi_dlhandle *dlh,
		   const char *args, void *argptr, uint32 *dlret)
{
	ggi_x_priv *priv;
	Display *disp;
	gg_option options[NUM_OPTS];
	void *lock;
	int i, err = GGI_ENOMEM;

	memcpy(options, optlist, sizeof(options));

	if (args) {
		args = ggParseOptions((char *)args, options, NUM_OPTS);
		if (args == NULL) {
			fprintf(stderr, "display-x: error in arguments.\n");
			return GGI_EARGINVAL;
		}
	}

	GGIDPRINT_MISC("X-target wants display %s\n", args);
	disp = XOpenDisplay(args);
	if (disp == NULL) return GGI_ENODEVICE;
	GGIDPRINT_MISC("X: has display %s\n", args);

	if ((lock = ggLockCreate()) == NULL) {
		XCloseDisplay(disp);
		return GGI_ENOMEM;
	}

	LIBGGI_GC(vis) = malloc(sizeof(ggi_gc));
	if (LIBGGI_GC(vis) == NULL) {
		goto out_freelock;
	}

	priv = malloc(sizeof(ggi_x_priv));
	if (priv == NULL) {
		goto out_freegc;
	}

	priv->opmansync = malloc(sizeof(_ggi_opmansync));
	if (priv->opmansync == NULL) {
		goto out_freepriv;
	}

	err = _ggi_parse_physz(options[OPT_PHYSZ].result, 
			       &(priv->xwin.x.physzflags), 
			       &(priv->xwin.x.physz)); 
	if (err != GGI_OK) goto out_freepriv;

	priv->xwin.x.display = disp;
	priv->xwin.x.screen = DefaultScreen(priv->xwin.x.display);
	GGIDPRINT_MISC("X: has screen %d\n", priv->xwin.x.screen);

	priv->xwin.window = RootWindow(priv->xwin.x.display,
				       priv->xwin.x.screen);
	priv->ximage = NULL;
	priv->xwin.x.cmap = 0;
	priv->xwin.x.gc = 0;
	priv->xwin.x.xliblock = lock;

	/* Get size of root window */
	{
		Window dummywin;
		int dummy;
		unsigned int w, h, udummy;

		XGetGeometry(disp, RootWindow(priv->xwin.x.display,
					      priv->xwin.x.screen),
			     &dummywin, &dummy, &dummy, &w, &h,
			     &udummy, &udummy);
		if (w > 640) w = 640;
		if (h > 480) h = 480;
		priv->xwin.defsize.x = w;
		priv->xwin.defsize.y = h;
	}

	if (options[OPT_INROOT].result[0] != 'n') {
		priv->xwin.wintype = GGIX_ROOT;
		GGIDPRINT_MISC("X: using root window\n");
	} else if (options[OPT_INWIN].result[0] != 'n') {
		priv->xwin.wintype = GGIX_WIN;
		priv->xwin.window = strtol(options[OPT_INWIN].result, NULL, 0);
		GGIDPRINT_MISC("X: using window id 0x%x\n", priv->xwin.window);
	} else {
		priv->xwin.wintype = GGIX_NORMAL;
	}

	priv->xwin.cursor =
		_ggi_x_make_cursor(disp, RootWindow(priv->xwin.x.display,
						    priv->xwin.x.screen),
				   (options[OPT_NOCURSOR].result[0] == 'n'));

	for(i = 0; i < X_FRAME_MAXNUM; i++) {
		priv->ximage_list[i] = NULL;
#ifdef HAVE_SHM
		priv->shminfo[i].shmid = -1;
#endif
	}

#ifdef HAVE_SHM
	if (XShmQueryExtension(priv->xwin.x.display)) {
		GGIDPRINT_MISC("X: Enabling use of XSHM extension\n");
		priv->have_shm = 1;
	} else {
		priv->have_shm = 0;
	}
#endif

	LIBGGI_PRIVATE(vis) = priv;

	/* Has mode management */
	vis->opdisplay->flush=GGI_X_flush;
	vis->opdisplay->getmode=GGI_X_getmode;
	vis->opdisplay->setmode=GGI_X_setmode;
	vis->opdisplay->checkmode=GGI_X_checkmode;
	vis->opdisplay->getapi =GGI_X_getapi;
	vis->opdisplay->setflags=GGI_X_setflags;

	if (tolower((int)options[OPT_NOINPUT].result[0]) == 'n') {
		gii_inputxwin_arg args;
		gii_input *inp;
		
		args.disp = priv->xwin.x.display;
		args.ptralwaysrel = 0;
		args.wait = 1;
		args.exposefunc = (gii_inputxwin_exposefunc*)_ggi_x_do_blit;
		args.exposearg = priv;
		args.resizefunc = (gii_inputxwin_resizefunc*)_ggi_x_resize;
		args.resizearg = vis;
		args.gglock = lock;
		
		if ((inp = giiOpen("xwin", &args, NULL)) == NULL) {
			GGIDPRINT_MISC("Unable to open xwin inputlib\n");
			GGIclose(vis, dlh);
			return GGI_ENODEVICE;
		}

		priv->xwin.x.inp = inp;
		/* Now join the new event source in. */
		vis->input = giiJoinInputs(vis->input, inp);
        } else {
		priv->xwin.x.inp = NULL;
	}

	err = _ggiAddDL(vis, "helper-mansync", NULL, priv->opmansync, 0);
	if (err) {
		fprintf(stderr,
			"display-X: Cannot load required helper-mansync!\n");
		GGIclose(vis, dlh);
		return err;
	}

	MANSYNC_init(vis);

	GGIDPRINT_MISC("X-target fully up\n");

	*dlret = GGI_DL_OPDISPLAY;
	return 0;

  out_freepriv:
	free(priv);
  out_freegc:
	free(LIBGGI_GC(vis));
  out_freelock:
	ggLockDestroy(lock);
	XCloseDisplay(disp);

	return err;
}


static int GGIexit(ggi_visual *vis, struct ggi_dlhandle *dlh)
{
	MANSYNC_deinit(vis);

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
