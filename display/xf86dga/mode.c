/* $Id: mode.c,v 1.16 2004/09/12 19:49:43 cegger Exp $
******************************************************************************

   Mode management for XF86DGA

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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"
#include <ggi/internal/ggi-dl.h>
#include <ggi/display/xf86dga.h>
#include "xf86dga.h"


static void GGI_xf86dga_gcchanged(ggi_visual * vis, int mask)
{
	ggidga_priv *priv = LIBGGI_PRIVATE(vis);

	if ((mask & GGI_GCCHANGED_CLIP)) {
		XRectangle xrect;

		xrect.x = LIBGGI_GC(vis)->cliptl.x;
		xrect.y = LIBGGI_GC(vis)->cliptl.y
		    + vis->w_frame_num * LIBGGI_VIRTY(vis);
		xrect.width
		    = LIBGGI_GC(vis)->clipbr.x - LIBGGI_GC(vis)->cliptl.x;
		xrect.height
		    = LIBGGI_GC(vis)->clipbr.y - LIBGGI_GC(vis)->cliptl.y;
		XSetClipRectangles(priv->x.display, priv->x.gc, 0, 0,
				   &xrect, 1, Unsorted);
	}
	if ((mask & GGI_GCCHANGED_FG)) {
		XSetForeground(priv->x.display, priv->x.gc,
			       LIBGGI_GC(vis)->fg_color);
	}
	if ((mask & GGI_GCCHANGED_BG)) {
		XSetBackground(priv->x.display, priv->x.gc,
			       LIBGGI_GC(vis)->fg_color);
	}
}


static int GGI_xf86dga_setdisplayframe(ggi_visual * vis, int num)
{
	ggidga_priv *priv = LIBGGI_PRIVATE(vis);
	ggi_directbuffer *db = _ggi_db_find_frame(vis, num);

	if (db == NULL) {
		return -1;
	}

	vis->d_frame_num = num;

	_ggi_XF86DGASetViewPort(priv->x.display, priv->x.screen,
				vis->origin_x, LIBGGI_MODE(vis)->virt.y
				* vis->d_frame_num + vis->origin_y);

	return 0;
}


static int GGI_xf86dga_setwriteframe(ggi_visual * vis, int num)
{
	ggi_directbuffer *db = _ggi_db_find_frame(vis, num);

	if (db == NULL) {
		return -1;
	}

	vis->w_frame_num = num;
	vis->w_frame = db;

	GGI_xf86dga_gcchanged(vis, GGI_GCCHANGED_CLIP);

	return 0;
}


static int GGI_xf86dga_setorigin(ggi_visual * vis, int x, int y)
{
	ggidga_priv *priv = LIBGGI_PRIVATE(vis);
	ggi_mode *mode = LIBGGI_MODE(vis);

	if (x < 0 || x > mode->virt.x - mode->visible.x ||
	    y < 0 || y > mode->virt.y - mode->visible.y) {
		return -1;
	}

	_ggi_XF86DGASetViewPort(priv->x.display, priv->x.screen,
				x, mode->virt.y * vis->d_frame_num + y);

	vis->origin_x = x;
	vis->origin_y = y;

	return 0;
}

static int xf86dga_acquire(ggi_resource * res, uint32 actype)
{
	ggi_visual *vis;

	if (actype & ~(GGI_ACTYPE_READ | GGI_ACTYPE_WRITE)) {
		return GGI_EARGINVAL;
	}

	res->count++;
	res->curactype |= actype;
	if (res->count > 1)
		return 0;

	vis = res->priv;
	LIBGGIIdleAccel(vis);

	return 0;
}


static int xf86dga_release(ggi_resource * res)
{
	if (res->count < 1)
		return GGI_ENOTALLOC;

	res->count--;
	if (res->count == 0) {
		res->curactype = 0;
	}

	return 0;
}


/*
** Finds the appropriate XF86VidMode modeline for x/y.
*/
static int _GGI_xf86dga_findmode(ggi_visual * vis, int visible_x,
				 int visible_y)
{
	ggidga_priv *priv = LIBGGI_PRIVATE(vis);
	int i;

	/* Find a suitable XF86VidMode and return it */
	for (i = 0; i < priv->num_modes; i++)
		if (visible_x == priv->modes[i].x &&
		    visible_y == priv->modes[i].y) {
			return i;
		}

	/* mode not found */
	return -1;
}


int GGI_xf86dga_getapi(ggi_visual * vis, int num, char *apiname,
		       char *arguments)
{
	*arguments = '\0';
	switch (num) {
	case 0:
		strcpy(apiname, "display-dga");
		return 0;
	case 1:
		strcpy(apiname, "generic-stubs");
		return 0;
	case 2:
		strcpy(apiname, "generic-color");
		return 0;
	case 3:
		sprintf(apiname, "generic-linear-%u",
			GT_SIZE(LIBGGI_GT(vis)));
		return 0;
	}
	return -1;
}


int GGI_xf86dga_setmode(ggi_visual * vis, ggi_mode * tm)
{
	ggidga_priv *priv;
	XVisualInfo vinfo;
	int flags, i, err, id;
	char sugname[GGI_MAX_APILEN], args[GGI_MAX_APILEN];

	/* First, check if the mode can be set */
	if (GGI_xf86dga_checkmode(vis, tm))
		return -1;

	priv = LIBGGI_PRIVATE(vis);

	/* Set XF86VidMode */
	_ggi_XF86VidModeSwitchToMode(priv->x.display, priv->x.screen,
				     priv->
				     dgamodes[_GGI_xf86dga_findmode
					      (vis, tm->visible.x,
					       tm->visible.y)]);

	if (priv->x.cmap)
		XFreeColormap(priv->x.display, priv->x.cmap);
	if (priv->cmap2)
		XFreeColormap(priv->x.display, priv->cmap2);
	if (priv->x.gc)
		XFreeGC(priv->x.display, priv->x.gc);
	priv->x.gc = XCreateGC(priv->x.display,
			       DefaultRootWindow(priv->x.display), 0, 0);

	XMatchVisualInfo(priv->x.display, priv->x.screen,
			 (signed) GT_DEPTH(tm->graphtype),
			 (GT_SCHEME(tm->graphtype) == GT_PALETTE) ?
			 PseudoColor : TrueColor, &vinfo);

	if (GT_SCHEME(tm->graphtype) == GT_PALETTE) {
		free(LIBGGI_PAL(vis)->priv);
		LIBGGI_PAL(vis)->priv = NULL;

		priv->x.cmap = XCreateColormap(priv->x.display,
					       DefaultRootWindow(priv->x.
								 display),
					       vinfo.visual, AllocAll);
		priv->cmap2 =
		    XCreateColormap(priv->x.display,
				    DefaultRootWindow(priv->x.display),
				    vinfo.visual, AllocAll);

		GGIDPRINT("%d-bit visual: X-lib colormap allocated %x.\n",
			  GT_DEPTH(tm->graphtype), priv->x.cmap);
	} else {
		priv->x.cmap = 0;
		priv->cmap2 = 0;
	}

	GGIDPRINT("Ready, now taking crash course!\n");

	flags = XF86DGADirectGraphics | XF86DGADirectMouse |
	    ((priv->dgafeat & XF86DGAAccelPresent) ? XF86DGADoAccel : 0);
	_ggi_XF86DGADirectVideo(priv->x.display, priv->x.screen, flags);

	_ggiZapMode(vis, 0);

	/* Palette */
	if (LIBGGI_PAL(vis)->clut.data) {
		free(LIBGGI_PAL(vis)->clut.data);
		LIBGGI_PAL(vis)->clut.data = NULL;
	}

	if (GT_SCHEME(tm->graphtype) == GT_PALETTE) {
		LIBGGI_PAL(vis)->clut.size = priv->x.nocols =
		    1 << GT_DEPTH(tm->graphtype);
		priv->activecmap = 0;

		LIBGGI_PAL(vis)->clut.data = _ggi_malloc(sizeof(ggi_color) *
						    LIBGGI_PAL(vis)->clut.size);

		/* Set an initial palette. */
		ggiSetColorfulPalette(vis);
	}

	priv->pixperframe = GT_ByPPP(priv->stride * tm->virt.y, tm->graphtype);
	vis->d_frame_num = 0;

	_GGI_xf86dga_freedbs(vis);

	/* Set up directbuffers */
	for (i = 0; i < tm->frames; i++) {
		LIBGGI_APPLIST(vis)->last_targetbuf
		    = _ggi_db_add_buffer(LIBGGI_APPLIST(vis),
					 _ggi_db_get_new());
		if ((priv->dgafeat & XF86DGAAccelPresent)) {
			ggi_resource *res;

			res = malloc(sizeof(ggi_resource));
			if (res == NULL)
				return GGI_EFATAL;
			LIBGGI_APPBUFS(vis)[i]->resource = res;
			LIBGGI_APPBUFS(vis)[i]->resource->acquire
			    = xf86dga_acquire;
			LIBGGI_APPBUFS(vis)[i]->resource->release
			    = xf86dga_release;
			LIBGGI_APPBUFS(vis)[i]->resource->self
			    = LIBGGI_APPBUFS(vis)[i];
			LIBGGI_APPBUFS(vis)[i]->resource->priv = vis;
			LIBGGI_APPBUFS(vis)[i]->resource->count = 0;
			LIBGGI_APPBUFS(vis)[i]->resource->curactype = 0;
		}

		LIBGGI_APPBUFS(vis)[i]->frame = i;
		LIBGGI_APPBUFS(vis)[i]->type
		    = GGI_DB_NORMAL | GGI_DB_SIMPLE_PLB;
		LIBGGI_APPBUFS(vis)[i]->read
		    = LIBGGI_APPBUFS(vis)[i]->write
		    = (uint8 *) priv->fb + priv->pixperframe * i;
		LIBGGI_APPBUFS(vis)[i]->layout = blPixelLinearBuffer;
		LIBGGI_APPBUFS(vis)[i]->buffer.plb.stride
		    = GT_ByPPP(priv->stride, tm->graphtype);
		LIBGGI_APPBUFS(vis)[i]->buffer.plb.pixelformat
		    = LIBGGI_PIXFMT(vis);
		GGIDPRINT_MODE("DB: %d, addr: %p, stride: %d\n", i,
			       LIBGGI_APPBUFS(vis)[i]->read,
			       LIBGGI_APPBUFS(vis)[i]->buffer.plb.stride);
	}
	LIBGGI_APPLIST(vis)->first_targetbuf
	    = LIBGGI_APPLIST(vis)->last_targetbuf - (tm->frames - 1);

	vis->origin_x = 0;
	vis->origin_y = 0;

	/* Fill in ggi_pixelformat */
	memset(LIBGGI_PIXFMT(vis), 0, sizeof(ggi_pixelformat));
	LIBGGI_PIXFMT(vis)->red_mask = vinfo.red_mask;
	LIBGGI_PIXFMT(vis)->green_mask = vinfo.green_mask;
	LIBGGI_PIXFMT(vis)->blue_mask = vinfo.blue_mask;
	LIBGGI_PIXFMT(vis)->depth = GT_DEPTH(tm->graphtype);
	LIBGGI_PIXFMT(vis)->size = GT_SIZE(tm->graphtype);

	if (GT_SCHEME(tm->graphtype) == GT_PALETTE)
		LIBGGI_PIXFMT(vis)->clut_mask
		    = (1 << GT_DEPTH(tm->graphtype)) - 1;

	_ggi_build_pixfmt(LIBGGI_PIXFMT(vis));

	memcpy(LIBGGI_MODE(vis), tm, sizeof(ggi_mode));

	if ((priv->dgafeat & XF86DGAAccelPresent)) {
		vis->needidleaccel = 1;
	} else {
		vis->needidleaccel = 0;
	}
	vis->accelactive = 0;

	for (id = 1; GGI_xf86dga_getapi(vis, id, sugname, args) == 0; id++) {
		err = _ggiOpenDL(vis, sugname, args, NULL);
		if (err) {
			fprintf(stderr,
				"display-dga: Can't open the %s (%s) library.\n",
				sugname, args);
			/* In our special case, fail is always fatal. */
			return GGI_EFATAL;
		} else {
			GGIDPRINT_MODE("Success in loading %s (%s)\n",
				       sugname, args);
		}
	}

	vis->opdraw->setorigin = GGI_xf86dga_setorigin;
	vis->opdraw->setdisplayframe = GGI_xf86dga_setdisplayframe;
	if ((priv->dgafeat & XF86DGAAccelPresent)) {
		vis->opdraw->setwriteframe = GGI_xf86dga_setwriteframe;
		priv->drawbox = vis->opdraw->drawbox;
		priv->copybox = vis->opdraw->copybox;
		vis->opdraw->drawbox = GGI_xf86dga_drawbox;
		vis->opdraw->copybox = GGI_xf86dga_copybox;
		vis->opgc->gcchanged = GGI_xf86dga_gcchanged;
	}

	/* color */
	if (GT_SCHEME(tm->graphtype) == GT_PALETTE) {
		LIBGGI_PAL(vis)->setPalette = GGI_xf86dga_setPalette;
	}

	ggiIndicateChange(vis, GGI_CHG_APILIST);

	return 0;
}

#define WANT_CHECKONEBPP
#define _GGIcheckonebpp _GGI_xf86dga_checkonebpp
#include "../common/modelist.inc"

/**********************************/
/* check any mode (text/graphics) */
/**********************************/
int GGI_xf86dga_checkmode(ggi_visual * vis, ggi_mode * tm)
{
	ggidga_priv *priv;
	int err = 0;

	if (vis == NULL) {
		GGIDPRINT("Visual==NULL\n");
		return -1;
	}

	priv = LIBGGI_PRIVATE(vis);

	if (tm->visible.x == GGI_AUTO)
		tm->visible.x = priv->width;
	if (tm->visible.y == GGI_AUTO)
		tm->visible.y = tm->virt.y;

	if ((tm->dpp.x != 1 && tm->dpp.x != GGI_AUTO) ||
	    (tm->dpp.y != 1 && tm->dpp.y != GGI_AUTO)) {
		err = -1;
	}
	tm->dpp.x = tm->dpp.y = 1;

	if (GT_DEPTH(tm->graphtype) != priv->depth
	    || GT_SIZE(tm->graphtype) != priv->size) {
		if (tm->graphtype != GT_AUTO)
			err = -1;
		tm->graphtype = GT_CONSTRUCT(priv->depth,
					     (priv->depth <=
					      8) ? GT_PALETTE :
					     GT_TRUECOLOR, priv->size);
	}

	/* Try to find a suitable XF86DGA mode. */
#if 1
	if (_GGI_xf86dga_checkonebpp(vis, tm, priv->modes))
		err = -1;
#else
	if (_GGI_xf86dga_findmode(vis, tm->visible.x, tm->visible.y) == -1) {
		int sug_x, sug_y, new_x, new_y, i;

		/* If that cannot be done, flag it as an error */
		err = -1;

		/* Then try to find another mode with
		   the next higher resolution */

		sug_x = sug_y = 0x7FFFFFFF;	/* Just the maximum */

		for (i = 0; i < priv->num_modes; i++) {
			new_x = priv->modes[i].x;
			new_y = priv->modes[i].y;
			if (new_x >= tm->visible.x &&
			    new_y >= tm->visible.y &&
			    new_x <= sug_x && new_y <= sug_y) {
				sug_x = new_x;
				sug_y = new_y;
			}
		}

		/* If the above failed, then use
		   the highest resolution possible */
		if (sug_x == 0x7FFFFFFF && sug_y == 0x7FFFFFFF) {
			sug_x = sug_y = 0;

			for (i = 0; i < priv->num_modes; i++) {
				new_x = priv->modes[i].x;
				new_y = priv->modes[i].y;
				if (new_x >= sug_x && new_y >= sug_y) {
					sug_x = new_x;
					sug_y = new_y;
				}
			}
		}

		tm->visible.x = sug_x;
		tm->visible.y = sug_y;
	}
#endif
	if (tm->virt.x == GGI_AUTO)
		tm->virt.x = priv->width;
	if (tm->virt.y == GGI_AUTO)
		tm->virt.y = tm->visible.y;

	/* Virtual x-resolution is unchangeable */
	if ((unsigned) tm->virt.x != priv->width) {
		tm->virt.x = priv->width;
		err = -1;
	}
	if ((unsigned) tm->virt.y > priv->height) {
		tm->virt.y = priv->height;
		err = -1;
	} else if (tm->virt.y < tm->visible.y) {
		tm->virt.y = tm->visible.y;
		err = -1;
	}

	if ((signed)
	    (GT_ByPPP(priv->stride * tm->virt.y * tm->frames,
	    tm->graphtype)) > priv->mem_size * 1024)
	{
		tm->frames = priv->mem_size * 1024 /
		    GT_ByPPP(priv->stride * tm->virt.y, tm->graphtype);
		err = -1;
	}
	if (tm->frames < 1) {
		if (tm->frames != GGI_AUTO) {
			err = -1;
		}
		tm->frames = 1;
	}
#define SCREENWMM DisplayWidthMM(priv->x.display, priv->x.screen)
#define SCREENW   DisplayWidth(priv->x.display, priv->x.screen)
#define SCREENHMM DisplayHeightMM(priv->x.display, priv->x.screen)
#define SCREENH   DisplayHeight(priv->x.display, priv->x.screen)
#define SCREENDPIX \
((SCREENWMM <= 0) ?  0 : (SCREENW * tm->dpp.x * 254 / SCREENWMM / 10))
#define SCREENDPIY \
((SCREENHMM <= 0) ?  0 : (SCREENH * tm->dpp.x * 254 / SCREENHMM / 10))

	if (!err) {
		err = _ggi_physz_figure_size(tm,
					priv->x.physzflags,
					&(priv->x.physz), SCREENDPIX,
					SCREENDPIY, SCREENW, SCREENH);
	}
#undef SCREENWMM
#undef SCREENW
#undef SCREENHMM
#undef SCREENH
#undef SCREENDPIX
#undef SCREENDPIY

	return err;
}

/************************/
/* get the current mode */
/************************/
int GGI_xf86dga_getmode(ggi_visual * vis, ggi_mode * tm)
{
	GGIDPRINT("In GGI_xf86dga_getmode(%p,%p)\n", vis, tm);
	if (vis == NULL)
		return -1;

	if (LIBGGI_MODE(vis) == NULL)
		return -1;

	memcpy(tm, LIBGGI_MODE(vis), sizeof(ggi_mode));

	return 0;
}

/*************************/
/* set the current flags */
/*************************/
int GGI_xf86dga_setflags(ggi_visual * vis, ggi_flags flags)
{
	LIBGGI_FLAGS(vis) = flags;
	LIBGGI_FLAGS(vis) &= GGIFLAG_ASYNC;	/* Unkown flags don't take. */
	return 0;
}
