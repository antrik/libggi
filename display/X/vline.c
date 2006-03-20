/* $Id: vline.c,v 1.16 2006/03/20 14:12:14 pekberg Exp $
******************************************************************************

   LibGGI - vertical lines for display-x

   Copyright (C) 1998-1999 Marcus Sundberg	[marcus@ggi-project.org]
   Copyright (C) 2002 Brian S. Julin	        [bri@tull.umassp.edu]

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

int GGI_X_drawvline_slave(struct ggi_visual *vis, int x, int y, int h)
{
	ggi_x_priv *priv;
	priv = GGIX_PRIV(vis);

        LIBGGICLIP_XYH(vis, x, y, h);
	priv->slave->opdraw->drawvline_nc(priv->slave, x, y, h);
	GGI_X_DIRTY(vis, x, y, 1, h);
	return 0;
}

int GGI_X_drawvline_nc_slave(struct ggi_visual *vis, int x, int y, int h)
{
	ggi_x_priv *priv;
	priv = GGIX_PRIV(vis);

	priv->slave->opdraw->drawvline_nc(priv->slave, x, y, h);
	GGI_X_DIRTY(vis, x, y, 1, h);
	return 0;
}

int GGI_X_putvline_slave(struct ggi_visual *vis, int x, int y, int h, const void *data)
{
	ggi_x_priv *priv;
	priv = GGIX_PRIV(vis);

	/* Use slave's clipping for depth adjustment */
	priv->slave->opdraw->putvline(priv->slave, x, y, h, data);
        LIBGGICLIP_XYH(vis, x, y, h);
	GGI_X_DIRTY(vis, x, y, 1, h);
	return 0;
}

int GGI_X_getvline_slave(struct ggi_visual *vis, int x, int y, int h, void *data)
{	
	ggi_x_priv *priv;
	priv = GGIX_PRIV(vis);

	/* Slave is always up to date */
	return (priv->slave->opdraw->getvline(priv->slave, x, y, h, data));
}

int GGI_X_drawvline_slave_draw(struct ggi_visual *vis, int x, int y, int h)
{
	ggi_x_priv *priv;
	priv = GGIX_PRIV(vis);

        LIBGGICLIP_XYH(vis, x, y, h);
	GGI_X_CLEAN(vis, x, y, 1, h);
	priv->slave->opdraw->drawvline_nc(priv->slave, x, y, h);
	y = GGI_X_WRITE_Y;
	GGI_X_LOCK_XLIB(vis);
	XDrawLine(priv->disp, priv->drawable, priv->gc, x, y, x, y+h-1);
	GGI_X_MAYBE_SYNC(vis);
	GGI_X_UNLOCK_XLIB(vis);
	return 0;
}

int GGI_X_drawvline_nc_slave_draw(struct ggi_visual *vis, int x, int y, int h)
{
	ggi_x_priv *priv;
	priv = GGIX_PRIV(vis);

	GGI_X_CLEAN(vis, x, y, 1, h);
	priv->slave->opdraw->drawvline_nc(priv->slave, x, y, h);
	y = GGI_X_WRITE_Y;
	GGI_X_LOCK_XLIB(vis);
	XDrawLine(priv->disp, priv->drawable, priv->gc, x, y, x, y+h-1);
	GGI_X_MAYBE_SYNC(vis);
	GGI_X_UNLOCK_XLIB(vis);
	return 0;
}

int GGI_X_drawvline_draw(struct ggi_visual *vis, int x, int y, int h)
{
	ggi_x_priv *priv;
	priv = GGIX_PRIV(vis);

	y = GGI_X_WRITE_Y;
	GGI_X_LOCK_XLIB(vis);
	XDrawLine(priv->disp, priv->drawable, priv->gc, x, y, x, y+h-1);
	GGI_X_MAYBE_SYNC(vis);
	GGI_X_UNLOCK_XLIB(vis);
	return 0;
}

int GGI_X_putvline_draw(struct ggi_visual *vis, int x, int y, int h, const void *data)
{
        XImage *ximg;
	ggi_x_priv *priv;
	priv = GGIX_PRIV(vis);

	GGI_X_LOCK_XLIB(vis);
#warning 1,2,4-bit support needed.
	ximg = _ggi_x_create_ximage( vis, (char*)data, 1, h);
	if (ximg == NULL) return GGI_ENOMEM;

        XPutImage(priv->disp, priv->drawable, priv->gc, ximg,
                  0, 0, x, GGI_X_WRITE_Y, 1, (unsigned)h);
#ifndef HAVE_XINITIMAGE
	XFree(ximg);
#else
	free(ximg);
#endif

	GGI_X_MAYBE_SYNC(vis);
	GGI_X_UNLOCK_XLIB(vis);
        return 0;
}

static int geterror;

static int errorhandler (Display * disp, XErrorEvent * event)
{
	if (event->error_code == BadMatch) geterror = 1;

	return 0;
}

int GGI_X_getvline_draw(struct ggi_visual *vis, int x, int y, int h, void *data)
{	
	ggi_x_priv *priv;
	XImage *ximg;
	int     (*olderrorhandler) (Display *, XErrorEvent *);
	int ret = 0;
	uint8_t *data8;
	priv = GGIX_PRIV(vis);

        GGI_X_LOCK_XLIB(vis);
	XSync(priv->disp, 0);
	ggLock(_ggi_global_lock);
	geterror = 0;
	olderrorhandler = XSetErrorHandler(errorhandler);
	/* This will cause a BadMatch error when the window is
	   iconified or on another virtual screen... */
	ximg = XGetImage(priv->disp, priv->drawable, x, GGI_X_READ_Y,
			 1, (unsigned)h, AllPlanes, ZPixmap);
	XSync(priv->disp,0);
	XSetErrorHandler(olderrorhandler);

#warning honor various ximage format fields here.
#warning 1,2,4-bit support needed

	if (geterror) {
		ret = -1;
		goto out;
	}

	if (ximg->byte_order == 
#ifdef GGI_LITTLE_ENDIAN
	    LSBFirst
#else
	    MSBFirst
#endif
	    ) goto noswab;
	
	if (ximg->bits_per_pixel == 16) {
		uint8_t *ximgptr;
		data8 = (uint8_t *)data;
		ximgptr = (uint8_t *)(ximg->data) + ximg->xoffset * 2;
		while (h--) {
			*(data8) = *(ximgptr + 1);
			*(data8 + 1) = *(ximgptr);
			ximgptr += ximg->bytes_per_line;
			data8 += 2;
		}
	}
	else if (ximg->bits_per_pixel == 32) {
		uint8_t *ximgptr;
		data8 = (uint8_t *)data;
		ximgptr = (uint8_t *)(ximg->data) + ximg->xoffset * 4;
		while (h--) {
			*(data8) = *(ximgptr + 3);
			*(data8 + 1) = *(ximgptr + 2);
			*(data8 + 2) = *(ximgptr + 1);
			*(data8 + 3) = *(ximgptr);
			ximgptr += ximg->bytes_per_line;
			data8 += 4;
		}
	}
	else {
		uint8_t *ximgptr;

	noswab:

		ximgptr = (uint8_t *)(ximg->data) + 
			(ximg->xoffset * ximg->bits_per_pixel)/8;
		data8 = (uint8_t *)data;
		while (h--) {
			memcpy(data8, ximgptr,
				(unsigned)ximg->bits_per_pixel/8);
			ximgptr += ximg->bytes_per_line;
			data8 += ximg->bits_per_pixel/8;
		}
	}
	XDestroyImage(ximg);
 out:
	ggUnlock(_ggi_global_lock);
	GGI_X_UNLOCK_XLIB(vis);

	return ret;
}
