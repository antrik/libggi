/* $Id: visual.c,v 1.5 2003/07/06 10:25:24 cegger Exp $
******************************************************************************

   XF86DGA display target.

   Copyright (C) 1997-1998 Steve Cheng		[steve@ggi-project.org]
   Copyright (C) 1999-2000 Marcus Sundberg	[marcus@ggi-project.org]

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

#include "config.h"
#include <ggi/internal/ggi-dl.h>
#include <ggi/display/xf86dga.h>
#include "xf86dga.h"

#include <stdlib.h>
#include <ctype.h>
#include <string.h>


static const gg_option optlist[] =
{
	{ "noinput",	"no" },
	{ "nocursor",	"no" },
	{ "physz",	"0,0"}
};

#define OPT_NOINPUT     0
#define OPT_NOCURSOR    1
#define OPT_PHYSZ       2

#define NUM_OPTS        (sizeof(optlist)/sizeof(gg_option))


static int GGI_xf86dga_idleaccel(ggi_visual *vis)
{
	XSync(DGA_PRIV(vis)->x.display, 0);

	vis->accelactive = 0;

	return 0;
}

static int GGI_xf86dga_flush(ggi_visual *vis, int x, int y, int w, int h,
			     int tryflag)
{
	XSync(DGA_PRIV(vis)->x.display, 0);

	vis->accelactive = 0;

	return 0;
}

void _GGI_xf86dga_freedbs(ggi_visual *vis)
{
	int i;
	int first = LIBGGI_APPLIST(vis)->first_targetbuf;
	int last = LIBGGI_APPLIST(vis)->last_targetbuf;

	if (first < 0) {
		return;
	}
	for (i = (last - first); i >= 0; i--) {
		if (LIBGGI_APPBUFS(vis)[i+first]->resource) {
			while (LIBGGI_APPBUFS(vis)[i+first]->resource->count
			       > 0) {
				ggiResourceRelease(LIBGGI_APPBUFS(vis)[i+first]
						   ->resource);
			}
			free(LIBGGI_APPBUFS(vis)[i+first]->resource);
		}
		_ggi_db_free(LIBGGI_APPBUFS(vis)[i+first]);
		_ggi_db_del_buffer(LIBGGI_APPLIST(vis), i+first);
	}
	LIBGGI_APPLIST(vis)->first_targetbuf = -1;
}


static int _GGI_xf86dga_getbpp(ggidga_priv *priv)
{
	XImage *bppcheckimage;
	unsigned int bits_per_pixel = 0;

	if ((bppcheckimage = XGetImage(priv->x.display,
				       RootWindow(priv->x.display, priv->x.screen),
				       0, 0, 1, 1, AllPlanes, ZPixmap))
	    != NULL) {
		bits_per_pixel = bppcheckimage->bits_per_pixel;
		XDestroyImage(bppcheckimage);
	}
	return bits_per_pixel;
}

/* X was designed before C++ ... */
#if defined(__cplusplus) || defined(c_plusplus)
#define private		c_private
#endif

static int do_cleanup(ggi_visual *vis)
{
	ggidga_priv *priv = LIBGGI_PRIVATE(vis);
	int i;

	/* We may be called more than once due to the LibGG cleanup stuff */
	if (priv == NULL) return 0;

	_GGI_xf86dga_freedbs(vis);

	_ggi_XF86DGADirectVideo(priv->x.display, priv->x.screen, 0);
	XSync(priv->x.display, False);

	if (priv->x.inp) {
		XUngrabPointer(priv->x.display, CurrentTime);
		XUngrabKeyboard(priv->x.display, CurrentTime);
	}

	if (priv->x.cmap)XFreeColormap(priv->x.display,priv->x.cmap);
	if (priv->cmap2) XFreeColormap(priv->x.display,priv->cmap2);

	if (priv->dgamodes) {
		_ggi_XF86VidModeSwitchToMode(priv->x.display, priv->x.screen,
					     priv->dgamodes[0]);
		/* Free the modelines */
		for (i = 0; i < priv->num_modes; i++) {
			if (priv->dgamodes[i]->privsize > 0)
				XFree(priv->dgamodes[i]->private);
		}
		XFree(priv->dgamodes);
	}
	if (priv->modes) free(priv->modes);

	_ggi_XF86DGAUnmap();

	XSync(priv->x.display, False);
	XCloseDisplay(priv->x.display);

	ggLockDestroy(priv->x.xliblock);

	free(LIBGGI_PRIVATE(vis));

	free(LIBGGI_GC(vis));

	LIBGGI_PRIVATE(vis) = NULL;

	ggUnregisterCleanup((ggcleanup_func *)do_cleanup, vis);

	return 0;
}


static int GGIopen(ggi_visual *vis, struct ggi_dlhandle *dlh,
			const char *args, void *argptr, uint32 *dlret)
{
	ggidga_priv *priv;
	gg_option options[NUM_OPTS];
	Display *disp;
	int dgafeat, err, x, y;
	unsigned z;
	Window root;

	memcpy(options, optlist, sizeof(options));

	if (args) {
		args = ggParseOptions((char *)args, options, NUM_OPTS);
		if (args == NULL) {
			fprintf(stderr, "display-x: error in arguments.\n");
			return GGI_EARGINVAL;
		}
	}

	GGIDPRINT_MISC("display-DGA starting.\n");

	GGIDPRINT_MISC("display-DGA wants display %s.\n", args);

	disp = XOpenDisplay(args);
	if (disp == NULL) return GGI_ENODEVICE;

	GGIDPRINT("display-DGA has display %s.\n",args);

	err = GGI_ENODEVICE;
	_ggi_XF86DGAQueryVersion(disp, &x, &y);
	GGIDPRINT("display-DGA version %d.%d\n", x, y);
	if (x < 1) {
		fprintf(stderr, "Your XF86DGA is too old (%d.%d).\n", x, y);
		goto out_closex;
	}

	_ggi_XF86VidModeQueryVersion(disp, &x, &y);
	GGIDPRINT("XF86VidMode version %d.%d\n", x, y);
	
	_ggi_XF86DGAQueryDirectVideo(disp, DefaultScreen(disp), &dgafeat);
	if (! (dgafeat & XF86DGADirectPresent)) {
		fprintf(stderr, "display-DGA: No direct video capability available!\n");
		goto out_closex;
	}

	err = GGI_ENOMEM;
	priv = malloc(sizeof(*priv));
	if (priv == NULL) goto out_closex;

	LIBGGI_GC(vis) = malloc(sizeof(ggi_gc));
	if (LIBGGI_GC(vis) == NULL) goto out_freepriv;

	priv->x.xliblock = ggLockCreate();
	if (priv->x.xliblock == NULL) goto out_freegc;

	err = _ggi_parse_physz(options[OPT_PHYSZ].result,
                               &(priv->x.physzflags),
                               &(priv->x.physz));
        if (err != GGI_OK) goto out_freegc;

	priv->x.display	= disp;
	priv->x.screen	= DefaultScreen(priv->x.display);
	priv->dgafeat	= dgafeat;
	priv->x.gc	= 0;
	priv->x.cmap	= 0;
	priv->cmap2	= 0;
	priv->x.nocols	= 0;

	err = GGI_ENODEVICE;
	if (!_ggi_XF86DGAGetVideo(priv->x.display, priv->x.screen,
				  (char**) &priv->fb, &priv->stride,
				  &priv->bank_size, &priv->mem_size)) {
		fprintf(stderr, "display-DGA: Unable to map video memory!\n");
		goto out_destroylock;
	}

	GGIDPRINT("fb: %p, stride: %d, bank_size: %d, mem_size: %d\n",
	       priv->fb, priv->stride, priv->bank_size, priv->mem_size);

	if (priv->bank_size != priv->mem_size*1024) {
		fprintf(stderr, "display-DGA: Sorry, banked framebuffer layout not supported.\n");
		goto out_unmap;
	}

	/* Get virtual dimensions */
	XGetGeometry(priv->x.display,
		     RootWindow(priv->x.display, priv->x.screen), &root,
		     &x, &y, &priv->width, &priv->height, &z, &priv->depth);

	priv->size = _GGI_xf86dga_getbpp(priv);

	GGIDPRINT_MISC("Virtwidth: %d, depth: %d, size: %d\n", 
		    priv->width, priv->depth, priv->size);

	LIBGGI_PRIVATE(vis) = priv;

	/* Register cleanup handler */
	ggRegisterCleanup((ggcleanup_func *)do_cleanup, vis);

	/* Get XF86VidMode modelines */
	_ggi_XF86VidModeGetAllModeLines(priv->x.display, priv->x.screen,
					&priv->num_modes, &priv->dgamodes);

	priv->modes = malloc((priv->num_modes+1)*sizeof(ggi_modelistmode));
	if (priv->modes == NULL) {
		do_cleanup(vis);
		return GGI_ENOMEM;
	}
	
	for (x = 0; x<priv->num_modes; x++) {
		priv->modes[x].x = priv->dgamodes[x]->hdisplay;
		priv->modes[x].y = priv->dgamodes[x]->vdisplay;
		priv->modes[x].bpp = priv->depth;
		priv->modes[x].gt = GT_CONSTRUCT(priv->depth,
			(priv->depth <= 8) ? GT_PALETTE : GT_TRUECOLOR,
			priv->size);
		GGIDPRINT_MISC("Found mode: %dx%d\n",
			       priv->modes[x].x, priv->modes[x].y);
	}
	priv->modes[priv->num_modes].bpp = 0;

	priv->x.inp = NULL;
	if (tolower((int)options[OPT_NOINPUT].result[0]) == 'n') {
		gii_inputxwin_arg _args;
		gii_input *inp;
	
		/* Get all the events */
		XGrabKeyboard(priv->x.display, DefaultRootWindow(priv->x.display), 
			      True, GrabModeAsync, GrabModeAsync, CurrentTime);

		XGrabPointer(priv->x.display, DefaultRootWindow(priv->x.display), True,
			     ButtonPressMask | ButtonReleaseMask | PointerMotionMask,
			     GrabModeAsync, GrabModeAsync,
			     None, None, CurrentTime);
	
		_args.disp = priv->x.display;
		_args.win = DefaultRootWindow(priv->x.display);
		_args.ptralwaysrel = 1;
		_args.wait = 0;
		_args.exposefunc = _args.exposearg
			= _args.resizefunc = _args.resizearg = NULL;
		_args.gglock = priv->x.xliblock;

		if ((inp = giiOpen("xwin", &_args, NULL)) == NULL) {
			GGIDPRINT_MISC("Unable to open xwin inputlib\n");
			do_cleanup(vis);
			return GGI_ENODEVICE;
		}

		priv->x.inp = inp;
		/* Now join the new event source in. */
		vis->input = giiJoinInputs(vis->input, inp);
        }

	/* Has mode management */
	vis->opdisplay->getmode   = GGI_xf86dga_getmode;
	vis->opdisplay->setmode   = GGI_xf86dga_setmode;
	vis->opdisplay->checkmode = GGI_xf86dga_checkmode;
	vis->opdisplay->getapi    = GGI_xf86dga_getapi;
	vis->opdisplay->setflags  = GGI_xf86dga_setflags;
	vis->opdisplay->flush     = GGI_xf86dga_flush;
	vis->opdisplay->idleaccel = GGI_xf86dga_idleaccel;

	*dlret = GGI_DL_OPDISPLAY;
	return 0;

  out_unmap:
	_ggi_XF86DGAUnmap();
  out_destroylock:
	ggLockDestroy(priv->x.xliblock);
  out_freegc:
	free(LIBGGI_GC(vis));
  out_freepriv:
	free(priv);
  out_closex:
	XCloseDisplay(disp);
	return err;
}


static int GGIclose(ggi_visual *vis, struct ggi_dlhandle *dlh)
{
	return do_cleanup(vis);
}


int GGIdl_xf86dga(int func, void **funcptr);

int GGIdl_xf86dga(int func, void **funcptr)
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
