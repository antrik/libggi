/* $Id: buffer.c,v 1.6 2002/12/05 20:03:30 cegger Exp $
******************************************************************************

   LibGGI Display-X target: buffer and buffer syncronization handling.

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

#include <string.h>

#include "config.h"
#include <ggi/internal/ggi-dl.h>
#include <ggi/display/x.h>

int GGI_X_db_acquire(ggi_resource_t res, uint32 actype) {
	res->curactype = actype;
	res->count++;
	return 0;
}

int GGI_X_db_release(ggi_resource_t res) {
	ggi_visual *vis;
	vis = res->priv;
	if ((vis->w_frame->resource == res) & 
	    (res->curactype & GGI_ACTYPE_WRITE)) ggiFlush(vis);
	res->curactype = 0;
	res->count--;
	return 0;
}

int GGI_X_setdisplayframe_child(ggi_visual *vis, int num) {

	ggi_x_priv *priv;
	priv = GGIX_PRIV(vis);

        if (_ggi_db_find_frame(vis, num) == NULL) return GGI_EARGINVAL;
	vis->d_frame_num = num;
	XMoveWindow(priv->disp, priv->win, -vis->origin_x, 
		    - vis->origin_y - LIBGGI_VIRTY(vis) * num);
	GGI_X_MAYBE_SYNC(vis);
	return 0;
}

int GGI_X_setorigin_child(ggi_visual *vis, int x, int y) {

	ggi_x_priv *priv;
	priv = GGIX_PRIV(vis);

	if (x > LIBGGI_VIRTX(vis) - LIBGGI_X(vis)) return GGI_EARGINVAL;
	if (y > LIBGGI_VIRTY(vis) - LIBGGI_Y(vis)) return GGI_EARGINVAL;
	vis->origin_x = x;
	vis->origin_y = y;
	XMoveWindow(priv->disp, priv->win, -x, 
		    - y - LIBGGI_VIRTY(vis) * (vis->d_frame_num));
	GGI_X_MAYBE_SYNC(vis);

	return 0;
}

int GGI_X_setreadframe_slave(ggi_visual *vis, int num) {
	int err;
	ggi_x_priv *priv;
	
	priv = GGIX_PRIV(vis);

	err = _ggi_default_setreadframe(vis, num);
	if (err) return err;
	err = priv->slave->opdraw->setreadframe(priv->slave, num);
	return err;
}

int GGI_X_setwriteframe_slave(ggi_visual *vis, int num) {
	int err;
	ggi_x_priv *priv;
	
	priv = GGIX_PRIV(vis);

	ggiFlush(vis); /* Dirty region doesn't span frames. */
        priv->dirtytl.x = 1; priv->dirtybr.x = 0;
	err = _ggi_default_setwriteframe(vis, num);
	if (err) return err;
	err = priv->slave->opdraw->setwriteframe(priv->slave, num);
	return err;
}

/* XImage allocation for normal client-side buffer */
void _ggi_x_free_ximage(ggi_visual *vis) {
	ggi_x_priv *priv;
	int i, first, last;

	priv = GGIX_PRIV(vis);

	if (priv->slave) ggiClose(priv->slave);
	priv->slave = NULL;
	if (priv->ximage) XDestroyImage(priv->ximage); /* frees priv->fb */ 
	else if (priv->fb) free(priv->fb);
	priv->ximage = NULL;
	priv->fb = NULL;

	first = LIBGGI_APPLIST(vis)->first_targetbuf;
	last = LIBGGI_APPLIST(vis)->last_targetbuf;
	if (first < 0) {
		return;
	}
	for (i = (last - first); i >= 0; i--) {
		free(LIBGGI_APPBUFS(vis)[i]->resource);
		_ggi_db_free(LIBGGI_APPLIST(vis)->bufs[i+first]);
		_ggi_db_del_buffer(LIBGGI_APPLIST(vis), i+first);
	}
	LIBGGI_APPLIST(vis)->first_targetbuf = -1;
}

int _ggi_x_create_ximage(ggi_visual *vis) {

	char target[256];
	ggi_mode tm;
	ggi_x_priv *priv;
	int i;

	priv = GGIX_PRIV(vis);

	GGIDPRINT("viidx = %i\n", priv->viidx);

	GGIDPRINT_MODE("X: Creating vanilla XImage client-side buffer\n");

	_ggi_x_free_ximage(vis);

	priv->fb = malloc((vis->mode->virt.x * 
			   vis->mode->virt.y *
			   vis->mode->frames *
			   GT_SIZE(vis->mode->graphtype) + 7) / 8);
	if (priv->fb == NULL) return GGI_ENOMEM;

	/* We assume vis->mode structure has already been filled out */
	memcpy(&tm, vis->mode, sizeof(ggi_mode));

	i = 0;
	i += sprintf(target, "display-memory:-noblank:-pixfmt=");
	memset(target+i, '\0', 64);
	_ggi_pixfmtstr(vis, target + i, 1);
	i = strlen(target);
	sprintf(target + i, ":-physz=%i,%i:pointer", 
		vis->mode->size.x, vis->mode->size.y);

	priv->slave = ggiOpen(target, priv->fb);
	if (priv->slave == NULL || ggiSetMode(priv->slave, &tm)) {
		free(priv->fb);
		priv->fb = NULL;
		return GGI_ENOMEM;
	}
	
	priv->ximage = XCreateImage(priv->disp, 
				    priv->vilist[priv->viidx].vi->visual, 
				    priv->vilist[priv->viidx].vi->depth,
				    ZPixmap,	/* format */
				    0,		/* offset */
				    priv->fb,	/* data */
				    vis->mode->virt.x, 
				    vis->mode->virt.y * vis->mode->frames,
				    8,		/* bitmap_pad*/
				    0);
	if (priv->ximage == NULL) {
		ggiClose(priv->slave);
		priv->slave = NULL;
		free(priv->fb);
		priv->fb = NULL;
		return GGI_ENOMEM;
	}

#ifdef GGI_LITTLE_ENDIAN
	priv->ximage->byte_order = LSBFirst;
	priv->ximage->bitmap_bit_order = LSBFirst;
#else
	priv->ximage->byte_order = MSBFirst;
	priv->ximage->bitmap_bit_order = MSBFirst;
#endif

	/* TODO: Some brave soul do alloc error handling here. */

	for (i = 0; i < vis->mode->frames; i++) {
		LIBGGI_APPLIST(vis)->last_targetbuf
		  = _ggi_db_add_buffer(LIBGGI_APPLIST(vis),
				       _ggi_db_get_new());
		LIBGGI_APPBUFS(vis)[i]->frame = i;
		LIBGGI_APPBUFS(vis)[i]->type
		  = GGI_DB_NORMAL | GGI_DB_SIMPLE_PLB;
		LIBGGI_APPBUFS(vis)[i]->read = LIBGGI_APPBUFS(vis)[i]->write
		  = priv->fb + i * vis->mode->virt.y * 
		  priv->ximage->bytes_per_line;
		LIBGGI_APPBUFS(vis)[i]->layout = blPixelLinearBuffer;
		LIBGGI_APPBUFS(vis)[i]->buffer.plb.stride
		  = priv->ximage->bytes_per_line;
		LIBGGI_APPBUFS(vis)[i]->buffer.plb.pixelformat
		  = LIBGGI_PIXFMT(vis);
		LIBGGI_APPBUFS(vis)[i]->resource = 
		  _ggi_malloc(sizeof(struct ggi_resource));
		LIBGGI_APPBUFS(vis)[i]->resource->priv = vis;
		LIBGGI_APPBUFS(vis)[i]->resource->acquire = GGI_X_db_acquire;
		LIBGGI_APPBUFS(vis)[i]->resource->release = GGI_X_db_release;
		LIBGGI_APPBUFS(vis)[i]->resource->curactype = 0;
		LIBGGI_APPBUFS(vis)[i]->resource->count = 0;
        }
	LIBGGI_APPLIST(vis)->first_targetbuf
	  = LIBGGI_APPLIST(vis)->last_targetbuf - (vis->mode->frames-1);

	GGIDPRINT_MODE("X: XImage %p and slave visual %p share buffer at %p\n",
		       priv->ximage, priv->slave, priv->fb);

	return GGI_OK;
}

int GGI_X_flush_draw(ggi_visual *vis, 
		     int x, int y, int w, int h, int tryflag)
{
	ggi_x_priv *priv;
	priv = GGIX_PRIV(vis);

	if (tryflag == 0) {
		if (ggTryLock(priv->xliblock) != 0) {
			GGIDPRINT_MISC("X: TRYLOCK fail (in flush_draw)!\n");
			return 0;
		}
	} else if (tryflag != 2) { 
		ggLock(priv->xliblock);
	}
	_ggi_x_flush_cmap(vis);		/* Update the palette/gamma */

	/* Flush any pending Xlib operations. */
	XFlush(priv->disp);

	if (tryflag != 2) ggUnlock(priv->xliblock);
	return 0;
}

int GGI_X_create_window_drawable (ggi_visual *vis) {
	ggi_x_priv *priv;
	priv = GGIX_PRIV(vis);

	priv->drawable = priv->win;
	if (priv->drawable == None) priv->drawable = priv->parentwin;

	vis->opdraw->drawpixel		= GGI_X_drawpixel_slave_draw;
	vis->opdraw->drawpixel_nc	= GGI_X_drawpixel_nc_slave_draw;
	vis->opdraw->drawhline		= GGI_X_drawhline_slave_draw;
	vis->opdraw->drawhline_nc	= GGI_X_drawhline_nc_slave_draw;
	vis->opdraw->drawvline		= GGI_X_drawvline_slave_draw;
	vis->opdraw->drawvline_nc	= GGI_X_drawvline_nc_slave_draw;
	vis->opdraw->drawline		= GGI_X_drawline_slave_draw;
	vis->opdraw->drawbox		= GGI_X_drawbox_slave_draw;
	vis->opdraw->copybox		= GGI_X_copybox_slave_draw;
	vis->opdraw->fillscreen		= GGI_X_fillscreen_slave_draw;

	_ggi_x_readback_fontdata(vis);
	if (priv->fontimg) {
		vis->opdraw->putc		= GGI_X_putc_slave_draw;
		vis->opdraw->getcharsize	= GGI_X_getcharsize_font;
	}
	/* else stub will do it */

	if (priv->fb) return 0;

	vis->opgc->gcchanged		= GGI_X_gcchanged;
	vis->opdraw->setorigin		= GGI_X_setorigin_child;
	vis->opdraw->setdisplayframe	= GGI_X_setdisplayframe_child;

	vis->opdisplay->flush		= GGI_X_flush_draw;
	vis->opdraw->drawpixel		= GGI_X_drawpixel_draw;
	vis->opdraw->drawpixel_nc	= GGI_X_drawpixel_draw;
	vis->opdraw->putpixel		= GGI_X_putpixel_draw;
	vis->opdraw->putpixel_nc	= GGI_X_putpixel_draw;
	vis->opdraw->getpixel		= GGI_X_getpixel_draw;
	vis->opdraw->drawhline		= GGI_X_drawhline_draw;
	vis->opdraw->drawhline_nc	= GGI_X_drawhline_draw;
	vis->opdraw->puthline		= GGI_X_puthline_draw;
	vis->opdraw->gethline		= GGI_X_gethline_draw;
	vis->opdraw->drawvline		= GGI_X_drawvline_draw;
	vis->opdraw->drawvline_nc	= GGI_X_drawvline_draw;
	vis->opdraw->drawline		= GGI_X_drawline_draw;
	vis->opdraw->putvline		= GGI_X_putvline_draw;
	vis->opdraw->getvline		= GGI_X_getvline_draw;
	vis->opdraw->drawbox		= GGI_X_drawbox_draw;
	vis->opdraw->putbox		= GGI_X_putbox_draw;
	vis->opdraw->copybox		= GGI_X_copybox_draw;
	vis->opdraw->fillscreen		= GGI_X_fillscreen_draw;

	vis->opdraw->putc		= GGI_X_putc_draw;
	vis->opdraw->getcharsize	= GGI_X_getcharsize_font;

	if (!priv->slave) vis->opdraw->getbox = GGI_X_getbox_draw;

	return 0;

}

int GGI_X_expose(void *arg, int x, int y, int w, int h) {
	ggi_visual *vis;
	ggi_x_priv *priv;
	int err;
	vis = arg;
	priv = GGIX_PRIV(vis);

        /* Expose event may be queued from a previous (larger) mode.
           In that case we just ignore it and return. */
        if ((x+w > LIBGGI_VIRTX(vis)) || 
	    y+h > LIBGGI_VIRTY(vis) * (vis->d_frame_num + 1)) 
		return 0;

	priv->fullflush = 1;
	err = _ggiInternFlush(vis, x, y, w, h, 2);
	priv->fullflush = 0;
	return err;
}

int GGI_X_flush_ximage_child(ggi_visual *vis, 
			     int x, int y, int w, int h, int tryflag)
{
	ggi_x_priv *priv;
	priv = GGIX_PRIV(vis);

	if (priv->opmansync) MANSYNC_ignore(vis);

	if (tryflag == 0) {
		if (ggTryLock(priv->xliblock) != 0) {
			GGIDPRINT_MISC("X: TRYLOCK fail (in flush_ximage_child)!\n");
			if (priv->opmansync) MANSYNC_cont(vis);
			return 0;
		}
	} else if (tryflag != 2) { 
		ggLock(priv->xliblock);
	}
	_ggi_x_flush_cmap(vis);		/* Update the palette/gamma */

	/* Flush any pending Xlib operations. */
	XSync(priv->disp, 0);

	if (priv->fullflush || (GGI_ACTYPE_WRITE &
	    (LIBGGI_APPBUFS(vis)[vis->w_frame_num]->resource->curactype))) {
		/* Flush all requested data */
		if (tryflag != 2) {
			GGI_X_CLEAN(vis, x, y, w, h);
			y = GGI_X_WRITE_Y;
		} /* else it's a non-translated exposure event. */
		XPutImage(priv->disp, priv->win, priv->tempgc, priv->ximage, 
			  x, y, x, y, w, h);
		
	} else {
		/* Just flush the intersection with the dirty region */
 	  	int x2, y2;

		if (priv->dirtytl.x > priv->dirtybr.x) goto clean;
		if (x > priv->dirtybr.x) goto clean;
		if (y > priv->dirtybr.y) goto clean;
		x2 = x + w - 1;
		if (x2 < priv->dirtytl.x) goto clean;
		y2 = y + h - 1;
		if (y2 < priv->dirtytl.y) goto clean;
		if (x < priv->dirtytl.x)  x  = priv->dirtytl.x;
		if (y < priv->dirtytl.y)  y  = priv->dirtytl.y;
		if (x2 > priv->dirtybr.x) x2 = priv->dirtybr.x;
		if (y2 > priv->dirtybr.y) y2 = priv->dirtybr.y;
		w = x2 - x + 1;
		h = y2 - y + 1;
		if ((w <= 0) || (h <= 0)) goto clean;

		XPutImage(priv->disp, priv->win, priv->tempgc, priv->ximage, 
			  x, GGI_X_WRITE_Y, x, GGI_X_WRITE_Y, w, h);
		GGI_X_CLEAN(vis, x, y, w, h);
	}

	/* Tell X Server to start blitting */
	XFlush(priv->disp);
 clean:
	if (tryflag != 2) ggUnlock(priv->xliblock);
	if (priv->opmansync) MANSYNC_cont(vis);
	return 0;
}

