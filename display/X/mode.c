/* $Id: mode.c,v 1.47 2005/03/28 20:33:34 pekberg Exp $
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

	/* Let's store this physical size data for the benifit 
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
		 * Don't create a window who's handles/borders are offscreen */
		m->visible.x = screenw * 9 / 10;
		m->visible.y = screenh * 9 / 10;

		/* We only support virtual widths that are multiples
		 * of four, so let's make it likely that the visible 
		 * width will equal the virtual one. */
		m->visible.x = FourMultiple( m->visible.x );
	}
	else if( priv->parentwin != None && priv->parentwin == priv->win ) {
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
		m->visible.x = screenw;
		m->visible.y = screenh;
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
	if( priv->ok_to_resize && reqx != GGI_AUTO && reqx < sug->visible.x )
		sug->visible.x = reqx;
	if( priv->ok_to_resize && reqy != GGI_AUTO && reqy < sug->visible.y )
		sug->visible.y = reqy;

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

	
#define SCREENPIX ( screenwmm<=0 ?  0 :  screenw * 254 / screenwmm / 10 )
#define SCREENPIY ( screenhmm<=0 ?  0 :  screenh * 254 / screenhmm / 10 )

	/* Now for size... */
	_ggi_physz_figure_size( sug, priv->physzflags, &(priv->physz),
				(signed) SCREENPIX,
				(signed) SCREENPIY,
				(signed) screenw, (signed) screenh );

	/* Want frames? You bet! */
	sug->frames = 
		req->frames==GGI_AUTO ? 1 : req->frames;
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
int GGI_X_checkmode_internal( ggi_visual *vis, ggi_mode *tm, int *viidx )
{
	int i;
	ggi_checkmode_t * cm;
	ggi_x_vi * vi;
	ggi_x_priv *priv = GGIX_PRIV(vis);

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

	i = _GGI_generic_checkmode_finish( cm, tm, (intptr_t*)viidx );
	_GGI_generic_checkmode_destroy( cm );
	return i;

}



/*
 * "this" is a C++ keyword, so we use "cthis" instead.
 */


#if 0
/*
 * Check mode
 */
static int GGI_X_checkmode_internal(ggi_visual *vis, ggi_mode *tm, int *viidx)
{
	ggi_x_priv	*priv;
	unsigned int	w, h;
	int		idx;
	ggi_x_vi	*best;

	APP_ASSERT(vis != NULL, "GGIcheckmode: vis == NULL");

	priv = GGIX_PRIV(vis);

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
	APP_ASSERT(tm->graphtype != GT_INVALID, "Should not fail here");
	if (_ggi_x_fit_geometry(vis, tm, best, tm) != GGI_OK) {
		DPRINT("This should not happen\n");
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
	APP_ASSERT(tm->graphtype != GT_INVALID, "Should not fail here");
	if (_ggi_x_fit_geometry(vis, tm, best, tm) != GGI_OK) {
		DPRINT("This should not happen\n");
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
	APP_ASSERT(tm->graphtype != GT_INVALID, "Should not fail here");
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
	APP_ASSERT(tm->graphtype != GT_INVALID, "Should not fail here");
	_ggi_x_fit_geometry(vis, tm, best, tm);
	return -1;
}
#endif

#if 0
int GGI_X_checkmode_normal(ggi_visual *vis, ggi_mode *tm)
{
	ggi_x_priv *priv;
	int dummy;
	int rc;
	unsigned char auto_virt;
	
	auto_virt = ((tm->virt.x == GGI_AUTO) && (tm->virt.y == GGI_AUTO));
	
	rc = GGI_X_checkmode_internal(vis, tm, &dummy);

	priv = GGIX_PRIV(vis);

	DPRINT_MODE("X (checkmode_normal): mlfuncs.validate = %p\n",
			priv->mlfuncs.validate);
	if (priv->mlfuncs.validate != NULL) {
		priv->cur_mode = priv->mlfuncs.validate(vis, -1, tm);
		if (priv->cur_mode < 0) {
			DPRINT_MODE("X: mlfuncs.validate failed: %i\n",
				priv->cur_mode);
			/* An error occured */
			dummy = priv->cur_mode;
			priv->cur_mode = 0;
			return dummy;
		}	
		else {
		  if(auto_virt) {
		    tm->virt.x = GGI_AUTO;
		    tm->virt.y = GGI_AUTO;
		  }

		  _ggi_x_fit_geometry(vis, tm, priv->vilist + dummy, tm);
		}        /* if */
		DPRINT_MODE("X: mlfuncs.validate successful: %i\n",
			priv->cur_mode);
	}	/* if */

	return rc;
}

int GGI_X_checkmode_fixed(ggi_visual *vis, ggi_mode *tm)
{
	ggi_x_priv *priv;
	Window root;
	int w, h;
	unsigned int depth, dummy;
	int vi_idx;
	unsigned char auto_virt;

	priv = GGIX_PRIV(vis);

	if (! XGetGeometry(priv->disp, priv->parentwin, &root,
			(int *)&dummy, (int *)&dummy,
			(unsigned int *)&w, (unsigned int *)&h,
			&dummy, &depth))
	{
		DPRINT_MODE("X (checkmode_fixed):"
				"no reply from X11 server\n");
		return GGI_EEVNOTARGET;
	}

	if (tm->visible.x == GGI_AUTO) tm->visible.x = w;
	if (tm->visible.y == GGI_AUTO) tm->visible.y = h;
	if ((tm->visible.x != w) || (tm->visible.y != h))
		return GGI_EARGINVAL;

	auto_virt = ((tm->virt.x == GGI_AUTO) && (tm->virt.y == GGI_AUTO));

	dummy = GGI_X_checkmode_internal(vis, tm, &vi_idx);
	if ((dummy != GGI_OK) || (tm->visible.x != w) || (tm->visible.y != h)) {
		tm->visible.x = w;
		tm->visible.y = h;
	}

	DPRINT_MODE("X (checkmode_fixed): mlfuncs.validate = %p\n",
			priv->mlfuncs.validate);
	if (priv->mlfuncs.validate != NULL) {
		priv->cur_mode = priv->mlfuncs.validate(vis, -1, tm);
		if (priv->cur_mode < 0) {
			DPRINT_MODE("X: mlfuncs.validate failed: %i\n",
					priv->cur_mode);
			/* An error occured */
			dummy = priv->cur_mode;
			priv->cur_mode = 0;
			return dummy;
		}
		else {
		  if(auto_virt) {
		    tm->virt.x = GGI_AUTO;
		    tm->virt.y = GGI_AUTO;
		  }

		  _ggi_x_fit_geometry(vis, tm, priv->vilist + vi_idx, tm);
		}        /* if */
		DPRINT_MODE("X: mlfuncs.validate successful: %i\n",
				priv->cur_mode);
	}	/* if */

	return dummy;
}
#endif

int GGI_X_checkmode(ggi_visual *vis, ggi_mode *tm)
{
	ggi_x_priv *priv = GGIX_PRIV(vis);

	int auto_virt, rc, vi_idx;

	auto_virt = ((tm->virt.x == GGI_AUTO) && (tm->virt.y == GGI_AUTO));

	rc = GGI_X_checkmode_internal(vis, tm, &vi_idx);

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

static int ggi_x_load_mode_libs(ggi_visual *vis)
{
	int err,id;
	char sugname[GGI_MAX_APILEN],args[GGI_MAX_APILEN];

	_ggiZapMode(vis, 0);
        for (id=1; 0 == vis->opdisplay->getapi(vis,id,sugname,args); id++) {
		err = _ggiOpenDL(vis, sugname, args, NULL);
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

#if USE_OLD_SETMODE
/**************************************************
 *                                                *
 *             START OF OLD SETMODE               *
 *                                                *
 **************************************************/

int GGI_X_setmode_normal(ggi_visual *vis, ggi_mode *tm)
{
	int err, viidx;
	XEvent event;
	XSetWindowAttributes attrib;
	XVisualInfo	*vi;
	ggi_x_priv      *priv;
	int destroychild, destroyparent, createchild, createparent;

	DPRINT("GGI_X_setmode_normal(%p, %p) called\n",vis,tm);

	priv = GGIX_PRIV(vis);

	if ((err=GGI_X_checkmode_internal(vis,tm,&viidx))) return err;
	memcpy(LIBGGI_MODE(vis), tm, sizeof(ggi_mode));
	priv->viidx = viidx;

	DPRINT("* viidx = %i\n", priv->viidx);

	if (priv->opmansync) MANSYNC_ignore(vis);

	GGI_X_LOCK_XLIB(vis);

	vi = (priv->vilist + viidx)->vi;

	_ggi_x_build_pixfmt(vis, tm, vi); /* Fill in ggi_pixelformat */

	/* Restore original mode */
	DPRINT_MODE("X (setmode_normal): mlfuncs.restore = %p\n",
			priv->mlfuncs.restore);
	if (priv->mlfuncs.restore != NULL) {
		err = priv->mlfuncs.restore(vis);
		DPRINT_MODE("X: mlfuncs.restore retcode: %i\n",
				err);
		if (err) goto err0;
	}	/* if */
	
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

	/* Parent windows are merely clipping frames, just use defaults. */

#warning all this could probably use more error checking

	priv->parentwin = 
		XCreateSimpleWindow(priv->disp,
				    RootWindow(priv->disp, vi->screen),
				    0, 0,
				    (unsigned int)tm->visible.x,
				    (unsigned int)tm->visible.y, 0,
				    0, 0);
	_ggi_x_dress_parentwin(vis, tm);

	DPRINT_MODE("X: Prepare to resize.\n");
	XResizeWindow(priv->disp,priv->parentwin,
			(unsigned int)tm->visible.x,
			(unsigned int)tm->visible.y);
	DPRINT_MODE("X: About to map parent (%p)\n", priv->parentwin);

	/* Map window. */
	DPRINT_MODE("X: Parent win: Map Input\n");
	XSelectInput(priv->disp, priv->parentwin, ExposureMask);
	DPRINT_MODE("X: Parent win: Raise Mapping\n");
	XMapRaised(priv->disp, priv->parentwin);
	DPRINT_MODE("X: Parent win: Map requested\n");

	/* Wait for window to become mapped */
	XNextEvent (priv->disp, &event);
	DPRINT_MODE("X: Window Mapped\n");

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
	DPRINT_MODE("X: running in parent window 0x%x\n", priv->parentwin);

	ggi_x_load_mode_libs(vis);
	_ggi_x_load_slaveops(vis);
	DPRINT("* viidx = %i\n", priv->viidx);
	if (priv->createfb != NULL) {
		err = priv->createfb(vis);
		if (err) {
			/* xlib lock is still acquired here - unlock before exiting */
			DPRINT("priv->createfb failed.\n");
			GGI_X_UNLOCK_XLIB(vis);
			goto err0;
		}
	}

	_ggi_x_free_colormaps(vis);
	XSync(priv->disp, 0);
	DPRINT("Create colormap.\n");
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
	DPRINT_MODE("X: About to map child\n");

	/* Have the parent window tell the WM its children have colormaps */
	XSetWMColormapWindows(priv->disp, priv->parentwin, &(priv->win), 1);

	/* Map window. */
	XSelectInput(priv->disp, priv->win, ExposureMask);
	XMapWindow(priv->disp, priv->win);

	/* Wait for window to become mapped */
	XNextEvent (priv->disp, &event);
	DPRINT_MODE("X: Window Mapped\n");

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
			 LIBGGI_VIRTY(vis) * LIBGGI_MODE(vis)->frames);
	DPRINT_MODE("X GCs allocated.\n");

	/* Create a cursor (frees old cursor) */
	if (priv->createcursor) priv->createcursor(vis);
	
	/* Turn on backing store */
	attrib.backing_store = Always;
	XChangeWindowAttributes(priv->disp,priv->win, CWBackingStore, &attrib);

	GGI_X_UNLOCK_XLIB(vis);

	DPRINT_MODE("X: Sync\n");
	XSync(priv->disp, 0);
	DPRINT_MODE("X: Sync done\n");

	if (priv->createdrawable) {
		err = priv->createdrawable(vis);
		if (err) goto err1;
	}

	DPRINT_MODE("X (setmode_normal): mlfuncs.enter = %p\n",
			priv->mlfuncs.enter);
	if (priv->mlfuncs.enter != NULL) {
		err = priv->mlfuncs.enter(vis, priv->cur_mode);
		DPRINT_MODE("X: mlfuncs.enter retcode = %i\n",
				err);
		if (err) goto err1;
	}	/* if */

	/* Tell inputlib about the new window */
	if (priv->inp) {
		gii_event ev;
		gii_xwin_cmddata_setparam data;

		DPRINT_MODE("X (setmode_normal): tell inputlib about new window\n");

		ev.cmd.size = sizeof(gii_cmd_event);
		ev.cmd.type = evCommand;
		ev.cmd.target = priv->inp->origin;
		ev.cmd.code = GII_CMDCODE_XWINSETPARAM;
		data.win = priv->win;
		if (data.win == None) {
			data.win = priv->parentwin;
		}
		data.ptralwaysrel = 0;
		data.parentwin = priv->parentwin;

		/* Assure aligned memory access. Some platforms
		 * (i.e. NetBSD/sparc64) rely on this.
		 */
		memcpy(ev.cmd.data, &data, sizeof(gii_xwin_cmddata_setparam));

		giiEventSend(priv->inp, &ev);
	}


	DPRINT_MODE("X (setmode_normal): set dirty region\n");

	/* ggiOpen will dirty the whole screen for us by calling fillscreen */
	priv->dirtytl.x = 1; priv->dirtybr.x = 0;

	if (priv->opmansync) MANSYNC_cont(vis);

	DPRINT_MODE("X (setmode_normal): return code = %i\n", err);
	return err;

err1:
	priv->freefb(vis);
err0:
	if (priv->opmansync) MANSYNC_cont(vis);
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

	DPRINT("GGI_X_setmode_fixed(%p, %p) called\n",vis,tm);
		
	priv = GGIX_PRIV(vis);

        XGetGeometry(priv->disp, priv->parentwin, &root, &dummy, &dummy,
		(unsigned int *)&w, (unsigned int *)&h,
		(unsigned int *)&dummy, (unsigned int *)&dummy);

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

	if (priv->opmansync) MANSYNC_ignore(vis);

	GGI_X_LOCK_XLIB(vis);

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
			DPRINT("priv->createfb failed.\n");
			/* xlib lock is still acquired here 
			 * - unlock before exiting */
			GGI_X_UNLOCK_XLIB(vis);
			goto err0;
		}
	}

	_ggi_x_free_colormaps(vis);
	XSync(priv->disp, 0);
	_ggi_x_create_colormaps(vis, vi);

	attrib.colormap = priv->cmap;
	attribmask = CWBackingStore;
	if (priv->win == root) {
		DPRINT_MODE("X (setmode_fixed): mlfuncs.restore = %p\n",
			priv->mlfuncs.restore);
		if (priv->mlfuncs.restore != NULL) {
			err = priv->mlfuncs.restore(vis);
			DPRINT_MODE("X: mlfuncs.restore retcode = %i\n",
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
	DPRINT_MODE("X: About to map child\n");

	/* Have the parent window tell the WM its children have colormaps */
	XSetWMColormapWindows(priv->disp, priv->parentwin, &(priv->win), 1);

	/* Map window. */
	XSelectInput(priv->disp, priv->win, ExposureMask);
	XMapWindow(priv->disp, priv->win);

	/* Wait for window to become mapped */
	XNextEvent (priv->disp, &event);
	DPRINT_MODE("X: Window Mapped\n");

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
			 LIBGGI_VIRTY(vis) * LIBGGI_MODE(vis)->frames);
	DPRINT_MODE("X GCs allocated.\n");

	/* Create a cursor (destroys old one) */
	if (priv->createcursor) priv->createcursor(vis);

	/* Turn on backing store (unless root window) */
	attrib.backing_store = Always;
	XChangeWindowAttributes(priv->disp, priv->win, attribmask, &attrib);

	GGI_X_UNLOCK_XLIB(vis);

	DPRINT_MODE("X: Sync\n");
	XSync(priv->disp, 0);
	DPRINT_MODE("X: Sync done\n");

	if (priv->createdrawable) {
		err = priv->createdrawable(vis);
		if (err) goto err1;
	}

	DPRINT_MODE("X (setmode_fixed): mlfuncs.enter = %p\n",
		priv->mlfuncs.enter);

	if (priv->mlfuncs.enter != NULL) {
		err = priv->mlfuncs.enter(vis, priv->cur_mode);
		DPRINT_MODE("X: mlfuncs.enter retcode = %i\n",
				err);
		if (err) goto err1;
	}

	/* Tell inputlib about the new window */
	if (priv->inp) {
		gii_event ev;
		gii_xwin_cmddata_setparam data;

		DPRINT_MODE("X (setmode_fixed): tell inputlib about new window\n");

		ev.cmd.size = sizeof(gii_cmd_event);
		ev.cmd.type = evCommand;
		ev.cmd.target = priv->inp->origin;
		ev.cmd.code = GII_CMDCODE_XWINSETPARAM;
		data.win = priv->win;
		if (data.win == None) {
			data.win = priv->parentwin;
		}
		data.ptralwaysrel = 0;
		data.parentwin = priv->parentwin;

		/* Assure aligned memory access. Some platforms
		 * (i.e. NetBSD/sparc64) rely on this.
		 */
		memcpy(ev.cmd.data, &data, sizeof(gii_xwin_cmddata_setparam));

		giiEventSend(priv->inp, &ev);
	}

	DPRINT_MODE("X (setmode_fixed): set dirty region\n");

	/* ggiOpen will dirty the whole screen for us by calling fillscreen */
	priv->dirtytl.x = 1; priv->dirtybr.x = 0;

	if (priv->opmansync) MANSYNC_cont(vis);

	DPRINT_MODE("X (setmode_fixed): return code = %i\n", err);
	return err;

err1:
	priv->freefb(vis);
err0:
	if (priv->opmansync) MANSYNC_cont(vis);
	return err;
}

int GGI_X_setmode(ggi_visual * vis, ggi_mode * tm)
{
	ggi_x_priv * priv  = GGIX_PRIV(vis);
	if(priv->ok_to_resize)
		return GGI_X_setmode_normal(vis, tm);
	else
		return GGI_X_setmode_fixed(vis, tm);
}

/**************************************************
 *                                                *
 *             END OF OLD SETMODE                 *
 *                                                *
 **************************************************/

#else


int GGI_X_setmode(ggi_visual * vis, ggi_mode * tm)
{
	int err, viidx;
	XEvent event;
	XSetWindowAttributes attrib;
	XWindowAttributes attrib2;
	XVisualInfo *vi;
	ggi_x_priv *priv;

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

	if ((err = GGI_X_checkmode_internal(vis, tm, &viidx)))
		return err;

	memcpy(LIBGGI_MODE(vis), tm, sizeof(ggi_mode));
	priv->viidx = viidx;

	DPRINT("* viidx = %i\n", priv->viidx);

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
		/* XXX: Cleanup question: is this check neccessary? */
		destroychild = destroychild && priv->win != priv->parentwin;
		destroyparent = 0;
		createparent = 0;
	}

	if( destroychild )
		XDestroyWindow(priv->disp, priv->win);

	if (destroyparent) 
		XDestroyWindow(priv->disp, priv->parentwin);

	if ( createparent ) {

		/* Parent windows are merely clipping frames, 
		 * just use defaults. */

		/* XXX: all this could probably use more error checking */

		priv->parentwin =
		    XCreateSimpleWindow(priv->disp,
					RootWindow(priv->disp, vi->screen),
					0, 0,
					(unsigned int) tm->visible.x,
					(unsigned int) tm->visible.y, 0,
					0, 0);
		_ggi_x_dress_parentwin(vis, tm);

		DPRINT_MODE("X: Prepare to resize.\n");
		XResizeWindow(priv->disp, priv->parentwin,
			      (unsigned int) tm->visible.x,
			      (unsigned int) tm->visible.y);
		DPRINT_MODE("X: About to map parent (%p)\n",
			    priv->parentwin);

		/* Map window. */
		DPRINT_MODE("X: Parent win: Map Input\n");
		XSelectInput(priv->disp, priv->parentwin, ExposureMask);
		DPRINT_MODE("X: Parent win: Raise Mapping\n");
		XMapRaised(priv->disp, priv->parentwin);
		DPRINT_MODE("X: Parent win: Map requested\n");

		/* Wait for window to become mapped */
		XNextEvent(priv->disp, &event);
		DPRINT_MODE("X: Window Mapped\n");

		/* We let the parent window listen for the keyboard.
		 * this allows to have the windowmanager decide about keyboard 
		 * focus as usual.
		 * The child window listens for the rest.
		 * Note, that the child window also listens for the keyboard for
		 * those cases where we don't have a parent.
		 */
		XSelectInput(priv->disp, priv->parentwin,
			     KeymapStateMask | KeyPressMask |
			     KeyReleaseMask);

	}

	DPRINT_MODE("X: running in parent window 0x%x\n", priv->parentwin);

	ggi_x_load_mode_libs(vis);
	_ggi_x_load_slaveops(vis);

	DPRINT("* viidx = %i\n", priv->viidx);
	DPRINT("* visual id = 0x%X\n", priv->vilist[priv->viidx].vi->visualid);

	if (priv->createfb != NULL) {
		err = priv->createfb(vis);
		if (err) {
			/* xlib lock is still acquired here 
			 * - unlock before exiting */
			DPRINT("priv->createfb failed.\n");
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

	if( !priv->ok_to_resize && priv->win == root ) {
		/* Turn off CWBackingStore for -inwin=root.
		 * XXX: We are setting CWColormap, do we only
		 * want this for -inwin=root? */
		attribmask = CWColormap;
	}
	else {

		unsigned long win_attribmask;
		unsigned win_width;

		/* XXX: our priv->win width is multiplied by the number
		 * of frames only if we are not -inwin or fullscreen...
		 * Is this proper??? */
		win_width = (unsigned) tm->virt.x;
		win_width *=  priv->ok_to_resize ? tm->frames : 1;

		attrib.border_pixel = BlackPixel(priv->disp, vi->screen);

		win_attribmask = CWColormap;
		win_attribmask |= priv->ok_to_resize ? CWBorderPixel : 0;

		priv->win = XCreateWindow(priv->disp, priv->parentwin,
					  0, 0, (unsigned) tm->virt.x,
					  win_width,
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
			     ExposureMask | PointerMotionMask);

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
		gii_event ev;
		gii_xwin_cmddata_setparam data;

		DPRINT_MODE("X (setmode): tell inputlib about new window\n");

		ev.cmd.size = sizeof(gii_cmd_event);
		ev.cmd.type = evCommand;
		ev.cmd.target = priv->inp->origin;
		ev.cmd.code = GII_CMDCODE_XWINSETPARAM;
		data.win = priv->win;
		if (data.win == None) {
			data.win = priv->parentwin;
		}
		data.ptralwaysrel = 0;
		data.parentwin = priv->parentwin;

		/* Assure aligned memory access. Some platforms
		 * (i.e. NetBSD/sparc64) rely on this.
		 */
		memcpy(ev.cmd.data, &data,
		       sizeof(gii_xwin_cmddata_setparam));

		giiEventSend(priv->inp, &ev);
	}

	DPRINT_MODE("X (setmode): set dirty region\n");

	/* ggiOpen will dirty the whole screen for us by calling fillscreen */
	priv->dirtytl.x = 1;
	priv->dirtybr.x = 0;

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

#endif 




/************************/
/* get the current mode */
/************************/
int GGI_X_getmode(ggi_visual *vis,ggi_mode *tm)
{
	APP_ASSERT(vis != NULL, "GGIgetmode: Visual == NULL");

	/* We assume the mode in the visual to be o.k. */
	memcpy(tm,LIBGGI_MODE(vis),sizeof(ggi_mode));

	return 0;
}

#if 0

int _ggi_x_resize(ggi_visual_t vis, int w, int h, ggi_event *ev)
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
