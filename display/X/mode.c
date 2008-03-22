/* $Id: mode.c,v 1.82 2008/03/22 20:37:27 cegger Exp $
******************************************************************************

   Graphics library for GGI. X target.

   Copyright (C) 1997      Jason McMullan	[jmcc@ggi-project.org]
   Copyright (C) 1998      Andreas Beck		[becka@ggi-project.org]
   Copyright (C) 1998      Steve Cheng		[steve@ggi-project.org]
   Copyright (C) 1998-2000 Marcus Sundberg	[marcus@ggi-project.org]
   Copyright (C) 2002      Brian S. Julin	[bri@tull.umassp.edu]
   Copyright (C) 2005	   Joseph Crayne	[oh.hello.joe@gmail.com]

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
#include <ggi/internal/ggi_debug.h>
#include <ggi/display/x.h>
#include <ggi/display/mansync.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Rounds up to a multiple of 4 */
#define FourMultiple(x) ( (x + 3) & ~3 )

/* a few prototypes */
void _GGI_X_checkmode_adapt( ggi_mode * m,
			     ggi_x_vi * vi,
			     ggi_x_priv * priv );
void _GGI_X_checkmode_adjust( ggi_mode *req,
			      ggi_mode *sug,
			      ggi_x_priv *priv );

/* Convert the ggi_x_vi structure to a ggi_mode structure
 * suitable for pasing as suggested mode to _GGI_generic_mode_update()
 *
 * This function sets:
 * 	m->dpp
 * 	m->gt
 * 	m->visible ( checkmode_adjust() will adjust this downward )
 *
 * These are set strangely and will be corrected by checkmode_adjust():
 * 	m->virt ( the size of the screen in pixels )
 * 	m->size ( the size of the screen in milimeters )
 *
 * Left undefined: ( checkmode_adjust() will set them )
 * 	m->frames
 */
void _GGI_X_checkmode_adapt( ggi_mode * m,
			     ggi_x_vi * vi,
			     ggi_x_priv * priv )
{

	int class2scheme[12] = {
		StaticGray, GT_STATIC_PALETTE,
		GrayScale, GT_GREYSCALE,
		StaticColor, GT_STATIC_PALETTE,
		PseudoColor, GT_PALETTE,
		TrueColor, GT_TRUECOLOR,
		DirectColor, GT_TRUECOLOR };
	
	int i;
	int screenw, screenh, screenwmm, screenhmm;
	Window dummywin;
	unsigned dummy, w, h;


	screenw = DisplayWidth(priv->disp, vi->vi->screen);
	screenh = DisplayHeight(priv->disp, vi->vi->screen);
	screenwmm = DisplayWidthMM(priv->disp, vi->vi->screen);
	screenhmm = DisplayHeightMM(priv->disp, vi->vi->screen);

	/* Let's store this physical size data for the benefit 
	 * of _GGI_X_checkmode_adjust() which will calculate 
	 * the proper values after the visible.x and visible.y 
	 * are known for certain. */
	m->virt.x = screenw;
	m->virt.y = screenh;
	m->size.x = screenwmm;
	m->size.y = screenhmm;
	
	
	/* Graphtype...
	 * First use our table to look up the scheme */
	for( i = 0; vi->vi->class != class2scheme[i] && i<12; i+=2 );
	i++;
	if( i==13 ) /* 13 is unlucky */
		m->graphtype = GT_INVALID;
	else 
		m->graphtype = GT_CONSTRUCT(vi->vi->depth, 
				class2scheme[i], 
				vi->buf->bits_per_pixel);


	/* Now dpp, this is simple.. */
	m->dpp.x = 1;
	m->dpp.y = 1;

	/* Now m->visible.  We'll compute the max values now
	 * and adjust them down later in _GGI_X_checkmode_adjust() */

	if( priv->ok_to_resize ) {
		/* Not a root window...
		 * We do not restrict sizes, if the user has requested them. 
		 * With virtual window managers even sizes larger than
		 * screen width make sense. */
		m->visible.x = 16384;	/* X internal limits may be at 32767, */
		m->visible.y = 16384;	/* so we just keep it reasonable. */

		/* We only support virtual widths that are multiples
		 * of four, so let's make it likely that the visible 
		 * width will equal the virtual one. */
		m->visible.x = FourMultiple( m->visible.x );
	}
	else {
		char inroot = 0; 

		if (priv->parentwin == RootWindow(priv->disp, vi->vi->screen))
			inroot = 1;

		if ( priv->parentwin != None && !inroot ) {
			/* This case is for -inwin=(some window other than root).. */
			XGetGeometry(priv->disp, priv->parentwin, 
					&dummywin, 
					(int *) &dummy, (int *) &dummy,
					&w, &h,
					&dummy, &dummy );
			m->visible.x = w;
			m->visible.y = h;
		}
		else {
			/* Root window or fullscreen.. */
                
			if ( (m->visible.x == GGI_AUTO) || inroot )
				m->visible.x = screenw;
	
			if ( (m->visible.y == GGI_AUTO) || inroot )
				m->visible.y = screenh;
		}
	}
}

/* Adjust our suggested mode sug to make it more closely match
 * the requested mode req for _GGI_generic_modecheck_update().
 * It is expects:
 * 	sug->visible is set to maximum values
 * 	sug->virt    is set to screen size in pixels
 * 	sug->size    is set to screen size in milimeters
 *
 * It set's the following fields:
 * 	sug->visible (anything reasonable unless ok_to_resize is false)
 * 	sug->frames  (anything resonable)
 * 	sug->virt    (anything reasonable so long as x is a multiple of 4)
 * 	sug->size    (this part is less flexible ;)
 */
void _GGI_X_checkmode_adjust( ggi_mode *req,
			      ggi_mode *sug,
			      ggi_x_priv *priv )
{
	int reqx, reqy;
	int screenw, screenh, screenwmm, screenhmm;
	
	/* Physical size data curtesy of _GGI_X_checkmode_adapt() */
	screenw = sug->virt.x;
	screenh = sug->virt.y;
	screenwmm = sug->size.x;
	screenhmm = sug->size.y;

	/* If they specified width or height in either visible
	 * or virt fields, let's store that. */
	reqx = req->visible.x != GGI_AUTO  
		? req->visible.x 
		: req->virt.x;
	reqy = req->visible.y != GGI_AUTO  
		? req->visible.y 
		: req->virt.y;

	/* Let's fixup our resolution to match what they want 
	 * if we can.  It's assumed that sug->visible is 
	 * maximized on entry to this function. */
	if (priv->ok_to_resize) {
		if (reqx != GGI_AUTO) {
			if ( reqx < sug->visible.x ) {
				sug->visible.x = reqx;
			}
		} else {
			/* If GGI_AUTO was requested, keep 10% for borders 
			 * so we don't create a window who's handles/borders 
			 * are offscreen. */
			sug->visible.x = screenw * 9 / 10;
		}
		if (reqy != GGI_AUTO) {
			if ( reqy < sug->visible.y ) {
				sug->visible.y = reqy;
			}
		} else {
			sug->visible.y = screenh * 9 / 10;
		}
	}

	/* Minimums for the virtual dimensions... */
	sug->virt.x = FourMultiple( sug->visible.x );
	sug->virt.y = sug->visible.y;

	/* This time, lets give precedence to the virt field... */
	reqx = req->virt.x != GGI_AUTO  
		? req->virt.x
		: req->visible.x;
	reqy = req->virt.y != GGI_AUTO  
		? req->virt.y
		: req->visible.y;

	/* ... so we can try to please them virtually. */
	if( reqx != GGI_AUTO && reqx > sug->virt.x )
		sug->virt.x = reqx;
	if( reqy != GGI_AUTO && reqy > sug->virt.y )
		sug->virt.y = reqy;

	/* Now for size... */
	_ggi_physz_figure_size( sug, GGI_PHYSZ_MM, &(priv->physz),
				(signed) screenwmm,
				(signed) screenhmm,
				(signed) screenw, (signed) screenh );


	/* Want frames? You bet! */
	sug->frames = (req->frames == GGI_AUTO) ? 1 : req->frames;
}

/* If two modes are equivelent as far as the generic checkmode
 * compare is concerned, then we will use this function to
 * distinguish which is "better".  Returns negative if the first
 * argument is prefered, positive for the other, and zero for
 * neutral.
 */
static 
int _GGI_X_checkmode_compare_visuals( ggi_mode *requested,
				      int via_num,
				      int vib_num,
				      ggi_x_priv *priv )
{
	int tmp;
	XVisualInfo *via, *vib;

	DPRINT_MODE( "Falling back on compare_visuals()...\n");

	via = priv->vilist[via_num].vi;
	vib = priv->vilist[vib_num].vi;

	do {
		tmp = _ggi_x_is_better_fmt(via, vib);
		DPRINT_MODE( "_ggi_x_is_better_fmt() returns %i\n", tmp);
		if( tmp != 0 ) break;

		tmp = _ggi_x_is_better_screen( ScreenOfDisplay(priv->disp, 
							       via->screen),
					       ScreenOfDisplay(priv->disp, 
							       vib->screen) );
		DPRINT_MODE( "_ggi_x_is_better_screen() returns %i\n", tmp);
		if( tmp != 0 ) break;

		tmp = via->visualid - vib->visualid;

		DPRINT_MODE( "<is_better_visualid> returns %i\n", tmp);

	} while(0);

	DPRINT_MODE( "compare_visuals() returns %i\n", tmp);
	return tmp;
}

#include <ggi/display/modelist.h>
#define WANT_GENERIC_CHECKMODE
#include "../common/modelist.inc"

static
int GGI_X_checkmode_internal( struct ggi_visual *vis, ggi_mode *tm, int *viidx )
{
	int i;
	intptr_t viidx_ptr;
	ggi_checkmode_t * cm;
	ggi_x_vi * vi;
	ggi_x_priv *priv = GGIX_PRIV(vis);

	viidx_ptr = *viidx;
	cm = _GGI_generic_checkmode_create();

	/* hook in our last-resort compare in order to prefer some
	 * visuals over other aparently equally capable visuals */
	cm->user_cmp = (ggi_user_cmp *)_GGI_X_checkmode_compare_visuals;
	cm->user_param = priv;

	_GGI_generic_checkmode_init( cm, tm );

	for(i=0, vi= priv->vilist; i < priv->nvisuals; i++, vi++ ) {

		/* The evi helper disqualifies some visuals */
		if( vi->flags & GGI_X_VI_NON_FB )
			continue;

		/* initialize mode suggestion */
		priv->cm_adapt( tm, vi, priv );
		/* adjust for wildcard matching in some fields */
		priv->cm_adjust( &cm->req, tm, priv );

		/* Let cm decide if it's the best suggestion yet */
		_GGI_generic_checkmode_update( cm, tm, i );
	}

	i = _GGI_generic_checkmode_finish( cm, tm, &viidx_ptr );
	*viidx = (int)viidx_ptr;
	_GGI_generic_checkmode_destroy( cm );
	return i;

}



int GGI_X_checkmode(struct ggi_visual *vis, ggi_mode *tm)
{
	ggi_x_priv *priv = GGIX_PRIV(vis);

	int auto_virt, rc, vi_idx;

	auto_virt = ((tm->virt.x == GGI_AUTO) && (tm->virt.y == GGI_AUTO));

	DPRINT_MODE("vis %dx%d virt %dx%d size %dx%d\n",
		    tm->visible.x, tm->visible.y,
		    tm->virt.x, tm->virt.y,
		    tm->size.x, tm->size.y);

	rc = GGI_X_checkmode_internal(vis, tm, &vi_idx);

	DPRINT_MODE("vis %dx%d virt %dx%d size %dx%d\n",
		    tm->visible.x, tm->visible.y,
		    tm->virt.x, tm->virt.y,
		    tm->size.x, tm->size.y);

	if (rc==GGI_OK && priv->mlfuncs.validate != NULL) {
		priv->cur_mode = priv->mlfuncs.validate(vis, -1, tm);
		if (priv->cur_mode < 0) {
			DPRINT_MODE("X: mlfuncs.validate failed: %i\n",
				priv->cur_mode);
			/* An error occured */
			rc = priv->cur_mode;
			priv->cur_mode = 0;
			/* return rc; */
		}	
#if 0
		else {
		  if(auto_virt) {
		    tm->virt.x = GGI_AUTO;
		    tm->virt.y = GGI_AUTO;
		  }

		  _ggi_x_fit_geometry(vis, tm, priv->vilist + vi_idx, tm);
		  rc = GGI_OK;
		}        /* if */
#endif
		DPRINT_MODE("X: mlfuncs.validate successful: %i\n",
			priv->cur_mode);
	}	/* if */

	return rc;
		
}

static int ggi_x_load_mode_libs(struct ggi_visual *vis)
{
	int err,id;
	char sugname[GGI_MAX_APILEN],args[GGI_MAX_APILEN];

	_ggiZapMode(vis, 0);
        for (id=1; 0 == vis->opdisplay->getapi(vis,id,sugname,args); id++) {
		err = _ggiOpenDL(vis, libggi->config, sugname, args, NULL);
		if (err) {
			fprintf(stderr,
				"display-x: Can't open the %s (%s) library.\n",
				sugname, args);
                        return err;
                } else {
		  	DPRINT_LIBS("X: GGIsetmode: success in loading "
				       "%s (%s)\n", sugname, args);
		}
        }
        ggiIndicateChange(vis->instance.stem, GGI_CHG_APILIST);

        return 0;
}

static void _ggi_x_load_slaveops(struct ggi_visual *vis) {
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
	vis->opdraw->getpixel_nc	= GGI_X_getpixel_nc_slave;
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

static int
compatible_mode(struct ggi_visual *vis, ggi_mode *mode)
{
	ggi_x_priv *priv = GGIX_PRIV(vis);

	if (!priv->ok_to_resize)
		/* fullscreen or -inwin */
		return !memcmp(mode, LIBGGI_MODE(vis), sizeof(ggi_mode));

	if (mode->frames != LIBGGI_MODE(vis)->frames)
		return 0;
	if (mode->graphtype != LIBGGI_GT(vis))
		return 0;
	if (memcmp(&mode->dpp, &LIBGGI_MODE(vis)->dpp, sizeof(ggi_coord)))
		return 0;

	return 1;
}

int GGI_X_setmode(struct ggi_visual * vis, ggi_mode * tm)
{
	int err, viidx;
	XEvent event;
	XSetWindowAttributes attrib;
	XWindowAttributes attrib2;
	XVisualInfo *vi;
	ggi_x_priv *priv;
	int compatible;

	Window root = 0;

	/* attribmask is used for ChangeAttributes to turn on 
	 * Backing store or not. */
	unsigned long attribmask = 0;

	int destroychild, destroyparent, createchild, createparent;

	priv = GGIX_PRIV(vis);

	/* Get the root window if we are running -inwin. */
	if( priv->parentwin != None ) {
		XGetWindowAttributes( priv->disp, priv->parentwin, &attrib2);
		root = attrib2.root;
	} else 
		root = None;

	err = GGI_X_checkmode_internal(vis, tm, &viidx);
	if (err)
		return err;

	compatible = compatible_mode(vis, tm);
	memcpy(LIBGGI_MODE(vis), tm, sizeof(ggi_mode));
	DPRINT("* change viidx = %i -> %i\n", priv->viidx, viidx);
	priv->viidx = viidx;

	if (priv->opmansync)
		MANSYNC_ignore(vis);

	GGI_X_LOCK_XLIB(vis);

	vi = (priv->vilist + viidx)->vi;

	_ggi_x_build_pixfmt(vis, tm, vi);	/* Fill in ggi_pixelformat */

	destroychild = destroyparent = 1;
	createchild = createparent = 1;
	if (priv->parentwin == None)
		destroyparent = 0;
	if (priv->win == None)
		destroychild = 0;

	if( ! priv->ok_to_resize ) {
		/* This check is necessary if -inwin option is used. */
		destroychild = destroychild && priv->win != priv->parentwin;
		destroyparent = 0;
		if (priv->parentwin != None) createparent = 0;
	}

	if( destroychild )
		XDestroyWindow(priv->disp, priv->win);

	if (destroyparent) 
		XDestroyWindow(priv->disp, priv->parentwin);

	if ( createparent ) {
		
		XSetWindowAttributes parentwin_attr;
		unsigned long parentwin_attr_mask;
		
		int screen;
		
		Atom wm_del_win;
		/* Parent windows are merely clipping frames, 
		 * just use defaults. */

		/* XXX: all this could probably use more error checking */

		parentwin_attr.backing_store = Always;
		parentwin_attr.save_under    = True;
		parentwin_attr_mask = CWBackingStore | CWSaveUnder;
		
		if(!priv->ok_to_resize)
		{
			/* Fullscreen windows don't have any wm decorations.
			 * So we'll set the override redirect attribute and
			 * don't attach any WM_ATOM to it. */
			/* Note for later : maybe we should let wmh handles
			 * fullscreen switching. (ie) When focus is lost
			 * switch back to windowed mode and recreate the
			 * parentwin with full decorations. */
			parentwin_attr.override_redirect = True;
			parentwin_attr_mask |= CWOverrideRedirect;
		}

		/*
		priv->parentwin =
		    XCreateSimpleWindow(priv->disp,
					RootWindow(priv->disp, vi->screen),
					0, 0,
					(unsigned int) tm->visible.x,
					(unsigned int) tm->visible.y, 0,
					0, 0);
		
		*/	
		screen = DefaultScreen(priv->disp);
		priv->parentwin =
			XCreateWindow(priv->disp,
				      RootWindow(priv->disp, vi->screen),
				      0, 0,
				      (unsigned int) tm->visible.x,
				      (unsigned int) tm->visible.y,
				      0,
				      DefaultDepth(priv->disp, screen),
				      InputOutput, DefaultVisual(priv->disp, screen),
				      parentwin_attr_mask,
				      &parentwin_attr);

		if(priv->parentwin == None)
		{
			DPRINT("X: Parent window creation failed.\n", err);
			GGI_X_UNLOCK_XLIB(vis);
			goto err0;
		}
		
		_ggi_x_dress_parentwin(vis, tm);

		DPRINT_MODE("X: Prepare to resize (%i,%i).\n",
				tm->visible.x, tm->visible.y);
		XResizeWindow(priv->disp, priv->parentwin,
			      (unsigned int) tm->visible.x,
			      (unsigned int) tm->visible.y);
		DPRINT_MODE("X: About to map parent (%p)\n",
			    priv->parentwin);

		/* Map window. */
		DPRINT_MODE("X: Parent win: Map Input\n");
		XSelectInput(priv->disp, priv->parentwin, ExposureMask);
		DPRINT_MODE("X: Parent win: Raise Mapping\n");
		XRaiseWindow(priv->disp, priv->parentwin);
		XMapRaised(priv->disp, priv->parentwin);
		DPRINT_MODE("X: Parent win: Map requested\n");

		/* Wait for window to become mapped */
		XNextEvent(priv->disp, &event);
		DPRINT_MODE("X: Window Mapped\n");

		if(priv->ok_to_resize)
		{
			/* register the WM_DELETE protocol */
			wm_del_win = XInternAtom(priv->disp, "WM_DELETE_WINDOW", False);
			XSetWMProtocols(priv->disp, priv->parentwin, &wm_del_win, 1);
		}
		
		/* We let the parent window listen for the keyboard.
		 * this allows to have the windowmanager decide about keyboard 
		 * focus as usual.
		 * The child window listens for the rest.
		 * Note, that the child window also listens for the keyboard for
		 * those cases where we don't have a parent.
		 */
		XSelectInput(priv->disp, priv->parentwin,
			     KeymapStateMask | KeyPressMask |
			     KeyReleaseMask |
			     FocusChangeMask | PropertyChangeMask |
			     StructureNotifyMask | VisibilityChangeMask);
	}

	DPRINT_MODE("X: running in parent window 0x%x\n", priv->parentwin);

	ggi_x_load_mode_libs(vis);
	_ggi_x_load_slaveops(vis);

	DPRINT("X: (setmode): * viidx = %i\n", priv->viidx);
	DPRINT("X: (setmode): * visual id = 0x%X\n",
		priv->vilist[priv->viidx].vi->visualid);

	if (priv->createfb != NULL) {
		err = priv->createfb(vis);
		if (err) {
			/* xlib lock is still acquired here 
			 * - unlock before exiting */
			DPRINT("priv->createfb failed with err=%i\n", err);
			GGI_X_UNLOCK_XLIB(vis);
			goto err0;
		}
	}

	_ggi_x_free_colormaps(vis);
	XSync(priv->disp, 0);

	DPRINT("Create colormap.\n");
	_ggi_x_create_colormaps(vis, vi);

	/* XXX: attribmask maybe should have CWColormap ORed in
	 * as well.  As it is, that attribute is ignored in the
	 * later call to XChangeWindowAttributes() in the case
	 * that we are not running with -inwin=root */
	attribmask = CWBackingStore;
	attrib.colormap = priv->cmap;

	DPRINT_MODE ("X (setmode): mlfuncs.restore = %p\n",
	     priv->mlfuncs.restore);

	if (priv->mlfuncs.restore != NULL) {
		err = priv->mlfuncs.restore(vis);
		DPRINT_MODE ("X: mlfuncs.restore retcode = %i\n", err);
		if (err) goto err0;
	}

	if( !priv->ok_to_resize && 
	    priv->win == root   && 
	    root != None ) {
		/* Turn off CWBackingStore for -inwin=root.
		 * XXX: We are setting CWColormap, do we only
		 * want this for -inwin=root? */
		attribmask = CWColormap;
	}
	else {

		unsigned long win_attribmask;
		unsigned int win_height;


		if (!priv->ok_to_resize) {
			/* -inwin or fullscreen window */
			win_height = (unsigned) tm->virt.y;
		} else {
			win_height = (unsigned) tm->virt.y * tm->frames;
		}
		attrib.border_pixel = BlackPixel(priv->disp, vi->screen);

		win_attribmask = CWColormap;
		win_attribmask |= priv->ok_to_resize ? CWBorderPixel : 0;

		priv->win = XCreateWindow(priv->disp, priv->parentwin,
					  0, 0, (unsigned) tm->virt.x,
					  win_height,
					   0, vi->depth,
					  InputOutput, vi->visual,
					  win_attribmask, 
					  &attrib);

		DPRINT_MODE("X: About to map child\n");

		/* Have the parent window tell the WM its children have 
		 * colormaps */
		XSetWMColormapWindows(priv->disp, priv->parentwin, 
				&(priv->win), 1);

		/* Map window. */
		XSelectInput(priv->disp, priv->win, ExposureMask);
		XMapWindow(priv->disp, priv->win);

		/* Wait for window to become mapped */
		XNextEvent(priv->disp, &event);
		DPRINT_MODE("X: Window Mapped\n");

		/* Select input events to listen for */
		XSelectInput(priv->disp, priv->win,
			     KeymapStateMask | KeyPressMask | KeyReleaseMask |
			     ButtonPressMask | ButtonReleaseMask |
			     EnterWindowMask | LeaveWindowMask |
			     ExposureMask | PointerMotionMask |
			     PropertyChangeMask |
			     StructureNotifyMask | VisibilityChangeMask);

	}


	/* Create a cursor (frees old cursor) */
	if (priv->createcursor)
		priv->createcursor(vis);


	/* Turn on backing store (unless root window) */
	attrib.backing_store = Always;
	XChangeWindowAttributes(priv->disp, priv->win, attribmask, &attrib);

	GGI_X_UNLOCK_XLIB(vis);

	DPRINT_MODE("X: Sync\n");
	XSync(priv->disp, 0);
	DPRINT_MODE("X: Sync done\n");

	if (priv->createdrawable) {
		err = priv->createdrawable(vis);
		if (err)
			goto err1;
	}

	DPRINT_MODE("X (setmode): mlfuncs.enter = %p\n",
		    priv->mlfuncs.enter);

	if (priv->mlfuncs.enter != NULL) {
		err = priv->mlfuncs.enter(vis, priv->cur_mode);
		DPRINT_MODE("X: mlfuncs.enter retcode = %i\n", err);
		if (err)
			goto err1;
	}


	/* Setup priv->gc and priv->tempgc ... */
	if (priv->gc)
		XFreeGC(priv->disp, priv->gc);
	priv->gc = XCreateGC(priv->disp, priv->drawable, 0, 0);
	XSetGraphicsExposures(priv->disp, priv->gc, True);

	if (priv->textfont) {
		XSetFont(priv->disp, priv->gc, priv->textfont->fid);
	}

	if (priv->tempgc)
		XFreeGC(priv->disp, priv->tempgc);
	priv->tempgc = XCreateGC(priv->disp, priv->drawable, 0, 0);
	XSetGraphicsExposures(priv->disp, priv->tempgc, True);
	if (priv->ok_to_resize) {
		if (priv->textfont)
			XSetFont(priv->disp, priv->tempgc,
				 priv->textfont->fid);
	}
	_ggi_x_set_xclip(NULL, priv->disp, priv->tempgc, 0, 0,
			 LIBGGI_VIRTX(vis),
			 LIBGGI_VIRTY(vis) * LIBGGI_MODE(vis)->frames);
	DPRINT_MODE("X GCs allocated.\n");

	/* if */
	/* Tell inputlib about the new window */
	if (priv->inp) {
		struct gii_xwin_cmddata_setparam data;
#if 0
		struct gii_xwin_cmddata_pointer  dataptr;
#endif


		if(priv->use_Xext & GGI_X_USE_VIDMODE) {	
			/* Grab pointers */
			DPRINT_MODE("X (setmode): grab pointers\n");
		}


		DPRINT_MODE("X (setmode): tell inputlib about new window\n");
		data.win = priv->win;
		if (data.win == None) {
			data.win = priv->parentwin;
		}
		data.parentwin = priv->parentwin;
		data.frames = tm->frames;
		
		ggControl(priv->inp->channel, GII_CMDCODE_XWINSETPARAM,
		    &data);
	}

	DPRINT_MODE("X (setmode): set dirty region\n");

	/* ggiOpen will dirty the whole screen for us by calling fillscreen */
	priv->dirtytl.x = 1;
	priv->dirtybr.x = 0;
	
	/* Reset displayed frame offset to 0 */
	priv->pf_offset = 0;

	if (priv->opmansync)
		MANSYNC_cont(vis);

	DPRINT_MODE("X (setmode): return code = %i\n", err);

	return err;

err1:
	priv->freefb(vis);
err0:
	if (priv->opmansync)
		MANSYNC_cont(vis);
	return err;
}





/************************/
/* get the current mode */
/************************/
int GGI_X_getmode(struct ggi_visual *vis,ggi_mode *tm)
{
	APP_ASSERT(vis != NULL, "GGIgetmode: Visual == NULL");

	/* We assume the mode in the visual to be o.k. */
	memcpy(tm,LIBGGI_MODE(vis),sizeof(ggi_mode));

	return 0;
}

#if 0

int _ggi_x_resize(struct ggi_visual_t vis, int w, int h, ggi_event *ev)
{
	ggi_cmddata_switchrequest *swreq;

	DPRINT_DRAW("_ggi_x_resize(%p, %dx%d, %p) called\n",
		       vis, w, h, ev);

	if ((LIBGGI_X(vis) == w) &&
	    (LIBGGI_Y(vis) == h))
	{
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
