/* $Id: mode.c,v 1.27 2004/07/28 09:33:14 ggibecka Exp $
******************************************************************************

   Graphics library for GGI. X target.

   Copyright (C) 1997      Jason McMullan	[jmcc@ggi-project.org]
   Copyright (C) 1998      Andreas Beck		[becka@ggi-project.org]
   Copyright (C) 1998      Steve Cheng		[steve@ggi-project.org]
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

#include "config.h"
#include <ggi/internal/ggi-dl.h>
#include <ggi/display/x.h>
#include <ggi/display/mansync.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


/*
 * "this" is a C++ keyword, so we use "cthis" instead.
 */


/*
 * Check mode
 */
static int GGI_X_checkmode_internal(ggi_visual *vis, ggi_mode *tm, int *viidx)
{
	ggi_x_priv	*priv;
	unsigned int	w, h;
	int		idx;
	ggi_x_vi	*best;

	LIBGGI_APPASSERT(vis != NULL, "GGIcheckmode: vis == NULL");

	priv = LIBGGI_PRIVATE(vis);

	best = NULL;
	w = h = idx = 0;
	while (idx < priv->nvisuals) {
	  	ggi_x_vi	*cthis;
		ggi_mode	dummy;

		dummy.visible.x = dummy.visible.y = 0;

		cthis = priv->vilist + idx++;

		if (cthis->flags & GGI_X_VI_NON_FB)
			continue;
#warning GT_SUBSCHEME needs to be validated here.
		if (_ggi_x_scheme_vs_class(tm->graphtype, cthis) == GT_INVALID)
			continue;
		if (_ggi_x_fit_geometry(vis, tm, cthis, &dummy) != GGI_OK) 
			continue;

		if ((unsigned)(dummy.visible.x * dummy.visible.y) < (w * h))
			continue;

		best = cthis;
		*viidx = idx - 1;
		w = dummy.visible.x;
		h = dummy.visible.y;
	}
	if (best == NULL) goto suggest;

	/* We have a successful mode :-) Adjust the values in tm. */
	tm->graphtype = _ggi_x_scheme_vs_class(tm->graphtype, best);
	LIBGGI_APPASSERT(tm->graphtype != GT_INVALID, "Should not fail here");
	if (_ggi_x_fit_geometry(vis, tm, best, tm) != GGI_OK) {
		GGIDPRINT("This should not happen\n");
	}
	return GGI_OK;

 suggest:
	/* Now that we know we are failing, we redo it from the top. */
	best = NULL;
	w = h = idx = 0;
	while (idx < priv->nvisuals) {
	  ggi_x_vi	*cthis;
		ggi_mode	dummy;

		dummy.visible.x = dummy.visible.y = 0;

		cthis = priv->vilist + idx++;

		if (cthis->flags & GGI_X_VI_NON_FB)
			continue;
#warning GT_SUBSCHEME needs to be validated here.
		if (_ggi_x_scheme_vs_class(tm->graphtype, cthis) == GT_INVALID)
			continue;
		_ggi_x_fit_geometry(vis, tm, cthis, &dummy);

		if ((unsigned)(dummy.visible.x * dummy.visible.y) < (w * h))
			continue;

		best = cthis;
		*viidx = idx - 1;
		w = dummy.visible.x;
		h = dummy.visible.y;
	}
	if (best == NULL) goto suggest2;

	tm->graphtype = _ggi_x_scheme_vs_class(tm->graphtype, best);
	LIBGGI_APPASSERT(tm->graphtype != GT_INVALID, "Should not fail here");
	if (_ggi_x_fit_geometry(vis, tm, best, tm) != GGI_OK) {
		GGIDPRINT("This should not happen\n");
	}
	return -1;

 suggest2:
	/* Adjust the graphtype up one notch if possible.  
	 * Recall that the vilist is sorted from worst to best visuals. 
	 */
	best = NULL;
	idx = w = h = 0;
	while (idx < priv->nvisuals) {
	  	ggi_x_vi	*cthis;
		ggi_graphtype	thisgt;

		cthis = priv->vilist + idx++;

		if (cthis->flags & GGI_X_VI_NON_FB)
			continue;
		thisgt = _ggi_x_scheme_vs_class(GGI_AUTO, cthis);
		if (thisgt == GT_INVALID) 
			continue;

		if (_ggi_x_is_better_gt(tm->graphtype, thisgt)) {
			best = cthis;
			*viidx = idx - 1;
			break;
		}
	}
	if (best == NULL) goto suggest3;
	tm->graphtype = _ggi_x_scheme_vs_class(GGI_AUTO, best);
	LIBGGI_APPASSERT(tm->graphtype != GT_INVALID, "Should not fail here");
	_ggi_x_fit_geometry(vis, tm, best, tm);
	return -1;

 suggest3:
	best = NULL;
	idx = w = h = 0;
	while (idx < priv->nvisuals) {
		ggi_x_vi	*cthis;

		cthis = priv->vilist + idx++;

		if (cthis->flags & GGI_X_VI_NON_FB)
			continue;
		best = cthis;
		*viidx = idx - 1;
	}

	if (best == NULL) {
		tm->graphtype = GT_INVALID;
		return -1;
	}
	tm->graphtype = _ggi_x_scheme_vs_class(GGI_AUTO, best);
	LIBGGI_APPASSERT(tm->graphtype != GT_INVALID, "Should not fail here");
	_ggi_x_fit_geometry(vis, tm, best, tm);
	return -1;
}

int GGI_X_checkmode_normal(ggi_visual *vis, ggi_mode *tm)
{
	ggi_x_priv *priv;
	int dummy;
	int rc;
	rc = GGI_X_checkmode_internal(vis, tm, &dummy);

	priv = GGIX_PRIV(vis);

	GGIDPRINT_MODE("X (checkmode_normal): mlfuncs.validate = %p\n",
			priv->mlfuncs.validate);
	if (priv->mlfuncs.validate != NULL) {
		priv->cur_mode = priv->mlfuncs.validate(vis, -1, tm);
		if (priv->cur_mode < 0) {
			GGIDPRINT_MODE("X: mlfuncs.validate failed: %i\n",
				priv->cur_mode);
			/* An error occured */
			dummy = priv->cur_mode;
			priv->cur_mode = 0;
			return dummy;
		}	/* if */
		GGIDPRINT_MODE("X: mlfuncs.validate successful: %i\n",
			priv->cur_mode);
	}	/* if */

	return rc;
}

int GGI_X_checkmode_fixed(ggi_visual *vis, ggi_mode *tm)
{
	ggi_x_priv *priv;
	Window root;
	int w, h, dummy;

	priv = GGIX_PRIV(vis);

#warning retcode checking here
	XGetGeometry(priv->disp, priv->parentwin, &root, &dummy, &dummy,
		     &w, &h, &dummy, &dummy);

	if (tm->visible.x == GGI_AUTO) tm->visible.x = w;
	if (tm->visible.y == GGI_AUTO) tm->visible.y = h;
	if ((tm->visible.x != w) || (tm->visible.y != h))
		return GGI_EARGINVAL;

	dummy = GGI_X_checkmode_internal(vis, tm, &dummy);
	if ((dummy != GGI_OK) || (tm->visible.x != w) || (tm->visible.y != h)) {
		tm->visible.x = w;
		tm->visible.y = h;
	}

	GGIDPRINT_MODE("X (checkmode_fixed): mlfuncs.validate = %p\n",
			priv->mlfuncs.validate);
	if (priv->mlfuncs.validate != NULL) {
		priv->cur_mode = priv->mlfuncs.validate(vis, -1, tm);
		if (priv->cur_mode < 0) {
			GGIDPRINT_MODE("X: mlfuncs.validate failed: %i\n",
					priv->cur_mode);
			/* An error occured */
			dummy = priv->cur_mode;
			priv->cur_mode = 0;
			return dummy;
		}	/* if */
		GGIDPRINT_MODE("X: mlfuncs.validate successful: %i\n",
				priv->cur_mode);
	}	/* if */

	return dummy;
}

static int ggi_x_load_mode_libs(ggi_visual *vis)
{
	int err,id;
	char sugname[256],args[256];

	_ggiZapMode(vis, 0);
        for (id=1; 0 == vis->opdisplay->getapi(vis,id,sugname,args); id++) {
		err = _ggiOpenDL(vis, sugname, args, NULL);
		if (err) {
			fprintf(stderr,
				"display-x: Can't open the %s (%s) library.\n",
				sugname, args);
                        return err;
                } else {
		  	GGIDPRINT_LIBS("X: GGIsetmode: success in loading "
				       "%s (%s)\n", sugname, args);
		}
        }
        ggiIndicateChange(vis, GGI_CHG_APILIST);

        return 0;
}

static void _ggi_x_load_slaveops(ggi_visual *vis) {
	ggi_x_priv *priv;
	priv = GGIX_PRIV(vis);

	vis->opgc->gcchanged		= GGI_X_gcchanged;
	vis->opdisplay->flush		= GGI_X_flush_ximage_child;
	vis->opdraw->setorigin		= GGI_X_setorigin_child;
	vis->opdraw->setdisplayframe	= GGI_X_setdisplayframe_child;
	vis->opdraw->setreadframe	= GGI_X_setreadframe_slave;
	vis->opdraw->setwriteframe	= GGI_X_setwriteframe_slave;
	vis->opdraw->drawpixel		= GGI_X_drawpixel_slave;
	vis->opdraw->drawpixel_nc	= GGI_X_drawpixel_nc_slave;
	vis->opdraw->putpixel		= GGI_X_putpixel_slave;
	vis->opdraw->putpixel_nc	= GGI_X_putpixel_nc_slave;
	vis->opdraw->getpixel		= GGI_X_getpixel_slave;
	vis->opdraw->drawhline		= GGI_X_drawhline_slave;
	vis->opdraw->drawhline_nc	= GGI_X_drawhline_nc_slave;
	vis->opdraw->puthline		= GGI_X_puthline_slave;
	vis->opdraw->gethline		= GGI_X_gethline_slave;
	vis->opdraw->drawvline		= GGI_X_drawvline_slave;
	vis->opdraw->drawvline_nc	= GGI_X_drawvline_nc_slave;
	vis->opdraw->putvline		= GGI_X_putvline_slave;
	vis->opdraw->getvline		= GGI_X_getvline_slave;
	vis->opdraw->drawline		= GGI_X_drawline_slave;
	vis->opdraw->drawbox		= GGI_X_drawbox_slave;
	vis->opdraw->putbox		= GGI_X_putbox_slave;
	vis->opdraw->getbox		= GGI_X_getbox_slave;
	vis->opdraw->copybox		= GGI_X_copybox_slave;
	vis->opdraw->fillscreen		= GGI_X_fillscreen_slave;
}

int GGI_X_setmode_normal(ggi_visual *vis, ggi_mode *tm)
{
	int err, viidx;
	XEvent event;
	XSetWindowAttributes attrib;
	XVisualInfo	*vi;
	ggi_x_priv      *priv;
	int destroychild, destroyparent, createchild, createparent;

	GGIDPRINT("GGI_X_setmode_normal(%p, %p) called\n",vis,tm);
		
	priv = GGIX_PRIV(vis);

	if ((err=GGI_X_checkmode_internal(vis,tm,&viidx))) return err;
	memcpy(LIBGGI_MODE(vis), tm, sizeof(ggi_mode));
	priv->viidx = viidx;

	GGIDPRINT("* viidx = %i\n", priv->viidx);

	ggLock(priv->xliblock);

	vi = (priv->vilist + viidx)->vi;

	_ggi_x_build_pixfmt(vis, tm, vi); /* Fill in ggi_pixelformat */

#warning TODO: do not destroy windows unless necessary.

	destroychild = destroyparent = 1;
	createchild  = createparent  = 1;
	if (priv->parentwin == None)	      destroyparent = 0;
	if (priv->win == None) 		      destroychild = 0;

	if (destroychild)  {
		/* Don't destroy window, when not created */
		if (priv->win != 0) XDestroyWindow(priv->disp, priv->win);
	}
	if (destroyparent) {
		/* Don't destroy window, when not created */
		if (priv->parentwin != 0) XDestroyWindow(priv->disp, priv->parentwin);
	}
	if (!createparent) goto oldparent;


	GGIDPRINT_MODE("X (setmode_normal): mlfuncs.restore = %p\n",
			priv->mlfuncs.restore);
	if (priv->mlfuncs.restore != NULL) {
		err = priv->mlfuncs.restore(vis);
		GGIDPRINT_MODE("X: mlfuncs.restore retcode: %i\n",
				err);
		if (err) goto err0;
	}	/* if */


	/* Parent windows are merely clipping frames, just use defaults. */

#warning all this could probably use more error checking

	priv->parentwin = 
		XCreateSimpleWindow(priv->disp,
				    RootWindow(priv->disp, vi->screen),
				    0, 0,
				    (unsigned)tm->visible.x,
				    (unsigned)tm->visible.y, 0,
				    0, 0);
	_ggi_x_dress_parentwin(vis, tm);

	GGIDPRINT_MODE("X: Prepare to resize.\n");
	XResizeWindow(priv->disp,priv->parentwin,
			(unsigned)tm->visible.x, (unsigned)tm->visible.y);
	GGIDPRINT_MODE("X: About to map parent\n");

	/* Map window. */
	GGIDPRINT_MODE("X: Parent win: Map Input\n");
	XSelectInput(priv->disp, priv->parentwin, ExposureMask);
	GGIDPRINT_MODE("X: Parent win: Raise Mapping\n");
	XMapRaised(priv->disp, priv->parentwin);

	/* Wait for window to become mapped */
	XNextEvent (priv->disp, &event);
	GGIDPRINT_MODE("X: Window Mapped\n");

	/* We let the parent window listen for the keyboard.
	 * this allows to have the windowmanager decide about keyboard 
	 * focus as usual.
	 * The child window listens for the rest.
	 * Note, that the child window also listens for the keyboard for
	 * those cases where we don't have a parent.
	 */
	XSelectInput(priv->disp, priv->parentwin,
		     KeymapStateMask | KeyPressMask | KeyReleaseMask);

oldparent:
	GGIDPRINT_MODE("X: running in parent window 0x%x\n", priv->parentwin);

	ggi_x_load_mode_libs(vis);
	_ggi_x_load_slaveops(vis);
	GGIDPRINT("* viidx = %i\n", priv->viidx);
	if (priv->createfb != NULL) {
		err = priv->createfb(vis);
		if (err) {
			/* xlib lock is still acquired here - unlock before exiting */
			GGIDPRINT("priv->createfb failed.\n");
			ggUnlock(priv->xliblock);
			goto err0;
		}
	}

	_ggi_x_free_colormaps(vis);
	XSync(priv->disp, 0);
	GGIDPRINT("Create colormap.\n");
	_ggi_x_create_colormaps(vis, vi);

	/* Create the child window */
	attrib.colormap = priv->cmap;
	attrib.border_pixel = BlackPixel(priv->disp, vi->screen);
	priv->win = XCreateWindow(priv->disp, priv->parentwin,
				  0, 0, (unsigned)tm->virt.x, 
				  (unsigned)tm->virt.y * tm->frames, 0,
				  vi->depth, InputOutput,
				  vi->visual, CWColormap | CWBorderPixel, 
				  &attrib);
	GGIDPRINT_MODE("X: About to map child\n");

	/* Have the parent window tell the WM its children have colormaps */
	XSetWMColormapWindows(priv->disp, priv->parentwin, &(priv->win), 1);

	/* Map window. */
	XSelectInput(priv->disp, priv->win, ExposureMask);
	XMapWindow(priv->disp, priv->win);

	/* Wait for window to become mapped */
	XNextEvent (priv->disp, &event);
	GGIDPRINT_MODE("X: Window Mapped\n");

	/* Select input events to listen for */
	XSelectInput(priv->disp, priv->win,
		     KeymapStateMask | KeyPressMask | KeyReleaseMask |
		     ButtonPressMask | ButtonReleaseMask |
		     EnterWindowMask | LeaveWindowMask |
		     ExposureMask | PointerMotionMask);

	if (priv->gc) XFreeGC(priv->disp, priv->gc);
	priv->gc = XCreateGC(priv->disp, priv->win, 0, 0);
	XSetGraphicsExposures(priv->disp, priv->gc, True);
	if (priv->textfont)
		XSetFont(priv->disp, priv->gc, priv->textfont->fid);

	if (priv->tempgc) XFreeGC(priv->disp, priv->tempgc);
	priv->tempgc = XCreateGC(priv->disp, priv->win, 0, 0);
	XSetGraphicsExposures(priv->disp, priv->tempgc, True);
	if (priv->textfont)
		XSetFont(priv->disp, priv->tempgc, priv->textfont->fid);
	_ggi_x_set_xclip(NULL, priv->disp, priv->tempgc, 0, 0, 
			 LIBGGI_VIRTX(vis), 
			 LIBGGI_VIRTY(vis) * vis->mode->frames);
	GGIDPRINT_MODE("X GCs allocated.\n");

	/* Create a cursor (frees old cursor) */
	if (priv->createcursor) priv->createcursor(vis);
	
	/* Turn on backing store */
	attrib.backing_store = Always;
	XChangeWindowAttributes(priv->disp,priv->win, CWBackingStore, &attrib);

	ggUnlock(priv->xliblock);

	GGIDPRINT_MODE("X: Sync\n");
	XSync(priv->disp, 0);
	GGIDPRINT_MODE("X: Sync done\n");

	if (priv->createdrawable) {
		err = priv->createdrawable(vis);
		if (err) goto err1;
	}

	GGIDPRINT_MODE("X (setmode_normal): mlfuncs.enter = %p\n",
			priv->mlfuncs.enter);
	if (priv->mlfuncs.enter != NULL) {
		err = priv->mlfuncs.enter(vis, priv->cur_mode);
		GGIDPRINT_MODE("X: mlfuncs.enter retcode = %i\n",
				err);
		if (err) goto err1;
	}	/* if */

	/* Tell inputlib about the new window */
	if (priv->inp) {
		gii_event ev;
		gii_xwin_cmddata_setparam *data
			= (gii_xwin_cmddata_setparam *) ev.cmd.data;

		GGIDPRINT_MODE("X (setmode_normal): tell inputlib about new window\n");

		ev.cmd.size = sizeof(gii_cmd_event);
		ev.cmd.type = evCommand;
		ev.cmd.target = priv->inp->origin;
		ev.cmd.code = GII_CMDCODE_XWINSETPARAM;
		data->win = priv->win;
		if (data->win == None) data->win = priv->parentwin;
		data->ptralwaysrel = 0;
		data->parentwin = priv->parentwin;

		giiEventSend(priv->inp, &ev);
	}


	GGIDPRINT_MODE("X (setmode_normal): set dirty region\n");

	/* ggiOpen will dirty the whole screen for us by calling fillscreen */
	priv->dirtytl.x = 1; priv->dirtybr.x = 0;

	if (priv->opmansync) MANSYNC_cont(vis);

	GGIDPRINT_MODE("X (setmode_normal): return code = %i\n", err);
	return err;

err1:
	priv->freefb(vis);
err0:
	return err;
}



int GGI_X_setmode_fixed(ggi_visual *vis, ggi_mode *tm)
{
	int err, viidx, w, h, dummy;
	Window root;
	XEvent event;
	unsigned long attribmask;
	XSetWindowAttributes attrib;
	XVisualInfo	*vi;
	ggi_x_priv      *priv;

	GGIDPRINT("GGI_X_setmode_fixed(%p, %p) called\n",vis,tm);
		
	priv = GGIX_PRIV(vis);

        XGetGeometry(priv->disp, priv->parentwin, &root, &dummy, &dummy,
		     &w, &h, &dummy, &dummy);

	if (tm->visible.x == GGI_AUTO) tm->visible.x = w;
	if (tm->visible.y == GGI_AUTO) tm->visible.y = h;
	if ((tm->visible.x != w) || (tm->visible.y != h))
	    return GGI_EARGINVAL;

	err = GGI_X_checkmode_internal(vis, tm, &viidx);
	if ((err != GGI_OK) || (tm->visible.x != w) || (tm->visible.y != h)) {
		tm->visible.x = w;
		tm->visible.y = h;
	}
	if(err) return err;

	memcpy(LIBGGI_MODE(vis), tm, sizeof(ggi_mode));
	priv->viidx = viidx;

	ggLock(priv->xliblock);

	vi = (priv->vilist + viidx)->vi;

	_ggi_x_build_pixfmt(vis, tm, vi); /* Fill in ggi_pixelformat */

	if (priv->win != priv->parentwin) {
#warning only destroy childwin when necessary
		/* Don't destroy window, when not created */
		if (priv->win != 0) XDestroyWindow(priv->disp, priv->win);
	}

	ggi_x_load_mode_libs(vis);
	_ggi_x_load_slaveops(vis);
	if (priv->createfb != NULL) {
		err = priv->createfb(vis);
		if (err) {
			/* xlib lock is still acquired here - unlock before exiting */
			GGIDPRINT("priv->createfb failed.\n");
			ggUnlock(priv->xliblock);
			goto err0;
		}
	}

	_ggi_x_free_colormaps(vis);
	XSync(priv->disp, 0);
	_ggi_x_create_colormaps(vis, vi);

	attrib.colormap = priv->cmap;
	attribmask = CWBackingStore;
	if (priv->win == root) {
		GGIDPRINT_MODE("X (setmode_fixed): mlfuncs.restore = %p\n",
			priv->mlfuncs.restore);
		if (priv->mlfuncs.restore != NULL) {
			err = priv->mlfuncs.restore(vis);
			GGIDPRINT_MODE("X: mlfuncs.restore retcode = %i\n",
					err);
			if (err) goto err0;
		}	/* if */

		attribmask = CWColormap;
		goto nochild;
	}
	/* Create the child window */
	priv->win = XCreateWindow(priv->disp, priv->parentwin,
				  0, 0, (unsigned)tm->virt.x,
				  (unsigned)tm->virt.y, 0,
				  vi->depth, InputOutput,
				  vi->visual, CWColormap, &attrib);
	GGIDPRINT_MODE("X: About to map child\n");

	/* Have the parent window tell the WM its children have colormaps */
	XSetWMColormapWindows(priv->disp, priv->parentwin, &(priv->win), 1);

	/* Map window. */
	XSelectInput(priv->disp, priv->win, ExposureMask);
	XMapWindow(priv->disp, priv->win);

	/* Wait for window to become mapped */
	XNextEvent (priv->disp, &event);
	GGIDPRINT_MODE("X: Window Mapped\n");

	/* Select input events to listen for */
	XSelectInput(priv->disp, priv->win,
		     KeymapStateMask | KeyPressMask | KeyReleaseMask |
		     ButtonPressMask | ButtonReleaseMask |
		     EnterWindowMask | LeaveWindowMask |
		     ExposureMask | PointerMotionMask);

 nochild:
	if (priv->gc) XFreeGC(priv->disp, priv->gc);
	priv->gc = XCreateGC(priv->disp, priv->win, 0, 0);
	XSetGraphicsExposures(priv->disp, priv->gc, True);

	if (priv->textfont) {
		XSetFont(priv->disp, priv->gc, priv->textfont->fid);
	}

	if (priv->tempgc) XFreeGC(priv->disp, priv->tempgc);
	priv->tempgc = XCreateGC(priv->disp, priv->win, 0, 0);
	XSetGraphicsExposures(priv->disp, priv->tempgc, True);
	_ggi_x_set_xclip(NULL, priv->disp, priv->tempgc, 0, 0, 
			 LIBGGI_VIRTX(vis), 
			 LIBGGI_VIRTY(vis) * vis->mode->frames);
	GGIDPRINT_MODE("X GCs allocated.\n");

	/* Create a cursor (destroys old one) */
	if (priv->createcursor) priv->createcursor(vis);

	/* Turn on backing store (unless root window) */
	attrib.backing_store = Always;
	XChangeWindowAttributes(priv->disp, priv->win, attribmask, &attrib);

	ggUnlock(priv->xliblock);

	GGIDPRINT_MODE("X: Sync\n");
	XSync(priv->disp, 0);
	GGIDPRINT_MODE("X: Sync done\n");

	if (priv->createdrawable) {
		err = priv->createdrawable(vis);
		if (err) goto err1;
	}

	GGIDPRINT_MODE("X (setmode_fixed): mlfuncs.enter = %p\n",
		priv->mlfuncs.enter);
	if (priv->mlfuncs.enter != NULL) {
		err = priv->mlfuncs.enter(vis, priv->cur_mode);
		GGIDPRINT_MODE("X: mlfuncs.enter retcode = %i\n",
				err);
		if (err) goto err1;
	}	/* if */

	/* Tell inputlib about the new window */
	if (priv->inp) {
		gii_event ev;
		gii_xwin_cmddata_setparam *data
			= (gii_xwin_cmddata_setparam *) ev.cmd.data;

		GGIDPRINT_MODE("X (setmode_fixed): tell inputlib about new window\n");

		ev.cmd.size = sizeof(gii_cmd_event);
		ev.cmd.type = evCommand;
		ev.cmd.target = priv->inp->origin;
		ev.cmd.code = GII_CMDCODE_XWINSETPARAM;
		data->win = priv->win;
		if (data->win == None) data->win = priv->parentwin;
		data->ptralwaysrel = 0;

		giiEventSend(priv->inp, &ev);
	}

	GGIDPRINT_MODE("X (setmode_fixed): set dirty region\n");

	/* ggiOpen will dirty the whole screen for us by calling fillscreen */
	priv->dirtytl.x = 1; priv->dirtybr.x = 0;

	if (priv->opmansync) MANSYNC_cont(vis);

	GGIDPRINT_MODE("X (setmode_fixed): return code = %i\n", err);
	return err;

err1:
	priv->freefb(vis);
err0:
	return err;
}

/************************/
/* get the current mode */
/************************/
int GGI_X_getmode(ggi_visual *vis,ggi_mode *tm)
{
	LIBGGI_APPASSERT(vis != NULL, "GGIgetmode: Visual == NULL");

	/* We assume the mode in the visual to be o.k. */
	memcpy(tm,LIBGGI_MODE(vis),sizeof(ggi_mode));

	return 0;
}

#if 0

int _ggi_x_resize(ggi_visual_t vis, int w, int h, ggi_event *ev)
{
	ggi_cmddata_switchrequest *swreq;

	GGIDPRINT_DRAW("_ggi_x_resize(%p, %dx%d, %p) called\n",
		       vis, w, h, ev);

	if (LIBGGI_MODE(vis)->visible.x == w &&
	    LIBGGI_MODE(vis)->visible.y == h) {
		return 1;
	}

	ev->any.size=	sizeof(gii_cmd_nodata_event)+
			sizeof(ggi_cmddata_switchrequest);
	ev->any.type=	evCommand;
	ev->cmd.code=	GGICMD_REQUEST_SWITCH;

	swreq = (ggi_cmddata_switchrequest *) ev->cmd.data;
	swreq->request = GGI_REQSW_MODE;
	ggiGetMode(vis, &swreq->mode);
	swreq->mode.visible.x = w;
	swreq->mode.visible.y = h;

	return 0;
}

#endif
