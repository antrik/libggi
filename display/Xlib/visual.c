/* $Id: visual.c,v 1.1 2001/05/12 23:01:55 cegger Exp $
******************************************************************************

   Display-Xlib initialization.

   Copyright (C) 1995 Andreas Beck		[becka@ggi-project.org]
   Copyright (C) 1997 Jason McMullan		[jmcc@ggi-project.org]
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
#include <ggi/display/xlib.h>

static const gg_option optlist[] =
{
	{ "inroot", "no" },
	{ "inwin",  "no" },
	{ "noinput", "no" }
};

#define OPT_INROOT	0
#define OPT_INWIN	1
#define OPT_NOINPUT	2

#define NUM_OPTS	(sizeof(optlist)/sizeof(gg_option))


static inline Cursor make_cursor(Display *disp, Window root)
{
	char data[] = { 0xf8, 0xfa, 0xf8 };
	char mask[] = { 0xfa, 0xff, 0xfa };
	Pixmap crsrpix, crsrmask;
	XColor black = { 0, 0x0, 0x0, 0x0 },
	       white = { 0, 0xffff, 0xffff, 0xffff };
	Cursor mycrsr;

	crsrpix = XCreateBitmapFromData(disp, root, data, 3, 3);
	crsrmask = XCreateBitmapFromData(disp, root, mask, 3, 3);
	mycrsr = XCreatePixmapCursor(disp, crsrpix, crsrmask,
				     &black, &white, 1, 1);
	XFreePixmap(disp, crsrpix);
	XFreePixmap(disp, crsrmask);

	return mycrsr;
}


static int GGIclose(ggi_visual *vis, struct ggi_dlhandle *dlh)
{
	ggi_xlib_priv *priv = LIBGGI_PRIVATE(vis);

	if (priv->xwin.x.cmap) XFreeColormap(priv->xwin.x.display, priv->xwin.x.cmap);
	if (priv->textfont) XUnloadFont(priv->xwin.x.display, priv->textfont->fid);
	if (priv->xwin.x.gc) XFreeGC(priv->xwin.x.display, priv->xwin.x.gc);
	if (priv->tempgc) XFreeGC(priv->xwin.x.display, priv->tempgc);
	if (priv->xwin.window && priv->xwin.wintype == GGIX_NORMAL) {
		XDestroyWindow(priv->xwin.x.display, priv->xwin.window);
	}
	if (priv->xwin.cursor != None) {
		XFreeCursor(priv->xwin.x.display, priv->xwin.cursor);
	}
	XSync(priv->xwin.x.display, 0);
	XCloseDisplay(priv->xwin.x.display);
	
	ggLockDestroy(priv->xwin.x.xliblock);
	
	free(priv);

	free(LIBGGI_GC(vis));

	GGIDPRINT_MISC("Xlib-target closed\n");

	return 0;
}


static int GGIopen(ggi_visual *vis, struct ggi_dlhandle *dlh,
		   const char *args, void *argptr, uint32 *dlret)
{
	ggi_xlib_priv *priv;
	Display *disp;
	gg_option options[NUM_OPTS];
	void *lock;
	int err = GGI_ENOMEM;

	memcpy(options, optlist, sizeof(options));

	if (args) {
		args = ggParseOptions((char *)args, options, NUM_OPTS);
		if (args == NULL) {
			fprintf(stderr, "display-xlib: error in arguments.\n");
			return GGI_EARGINVAL;
		}
	}

	GGIDPRINT_MISC("Xlib-target wants display %s\n", args);
	disp = XOpenDisplay(args);
	if (disp == NULL) return GGI_ENODEVICE;
	GGIDPRINT_MISC("Xlib: has display %s\n", args);

	if ((lock = ggLockCreate()) == NULL) {
		XCloseDisplay(disp);
		return GGI_ENOMEM;
	}

	LIBGGI_GC(vis) = malloc(sizeof(ggi_gc));
	if (LIBGGI_GC(vis) == NULL) {
		goto out_freelock;
	}

	priv = malloc(sizeof(ggi_xlib_priv));
	if (priv == NULL) {
		goto out_freegc;
	}

	priv->xwin.x.display = disp;
	priv->xwin.x.screen = DefaultScreen(priv->xwin.x.display);
	GGIDPRINT_MISC("Xlib: has screen %d\n", priv->xwin.x.screen);

	priv->xwin.window = RootWindow(priv->xwin.x.display,
				       priv->xwin.x.screen);
	priv->xwin.x.cmap = 0;
	priv->xwin.x.gc = 0;
	priv->tempgc = 0;
	priv->xwin.wintype = GGIX_NORMAL;
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
		GGIDPRINT_MISC("Xlib: using root window\n");
	} else if (options[OPT_INWIN].result[0] != 'n') {
		priv->xwin.wintype = GGIX_WIN;
		priv->xwin.window = strtol(options[OPT_INWIN].result, NULL, 0);
		GGIDPRINT_MISC("Xlib: using window id 0x%x\n",
			       priv->xwin.window);
	} else {
		priv->xwin.wintype = GGIX_NORMAL;
	}

	priv->xwin.cursor = make_cursor(disp, RootWindow(priv->xwin.x.display,
							 priv->xwin.x.screen));

	/* This doesn't work yet... */
#if 0
	if ((priv->textfont = XLoadQueryFont(disp, "fixed")) != NULL) {
		GGIDPRINT_MISC("Xlib: using font with "
			       "dimension %dx%d\n",
			       priv->textfont->max_bounds.width,
			       priv->textfont->max_bounds.ascent
			       + priv->textfont->max_bounds.descent);
	}
#else
	priv->textfont = NULL;
#endif

	LIBGGI_PRIVATE(vis) = priv;

	/* Has mode management */
	vis->opdisplay->flush=GGI_Xlib_flush;
	vis->opdisplay->getmode=GGI_Xlib_getmode;
	vis->opdisplay->setmode=GGI_Xlib_setmode;
	vis->opdisplay->checkmode=GGI_Xlib_checkmode;
	vis->opdisplay->getapi =GGI_Xlib_getapi;
	vis->opdisplay->setflags=GGI_Xlib_setflags;

	if (tolower((int)options[OPT_NOINPUT].result[0]) == 'n') {
		gii_inputxwin_arg args;
		gii_input *inp;
		
		args.disp = priv->xwin.x.display;
		args.ptralwaysrel = 0;
		args.wait = 1;
		args.exposefunc = args.exposearg = NULL;
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

	GGIDPRINT("Xlib fully up.\n");

	*dlret = GGI_DL_OPDISPLAY;
	return 0;

  out_freegc:
	free(LIBGGI_GC(vis));
  out_freelock:
	ggLockDestroy(lock);
	XCloseDisplay(disp);

	return err;
}


int GGIdl_Xlib(int func, void **funcptr)
{
	switch (func) {
	case GGIFUNC_open:
		*funcptr = GGIopen;
		return 0;
	case GGIFUNC_exit:
		*funcptr = NULL;
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
