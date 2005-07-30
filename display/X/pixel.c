/* $Id: pixel.c,v 1.6 2005/07/30 10:58:22 cegger Exp $
******************************************************************************

   Graphics library for GGI.  Pixels for display-X.

   Copyright (C) 1998 Marcus Sundberg [marcus@ggi-project.org]
   Copyright (C) 2001 Brian S. Julin [bri@tull.umassp.edu]

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

/*******************************/
/* draw/get/put a single pixel */
/*******************************/

int GGI_X_drawpixel_slave(ggi_visual *vis, int x, int y)
{
	ggi_x_priv *priv;
	priv = GGIX_PRIV(vis);

        CHECKXY(vis,x,y);
	priv->slave->opdraw->drawpixel_nc(priv->slave, x, y);
	GGI_X_DIRTY(vis, x, y, 1, 1);
	return 0;
}

int GGI_X_drawpixel_nc_slave(ggi_visual *vis, int x, int y)
{
	ggi_x_priv *priv;
	priv = GGIX_PRIV(vis);

	priv->slave->opdraw->drawpixel_nc(priv->slave, x, y);
	GGI_X_DIRTY(vis, x, y, 1, 1);
	return 0;
}

int GGI_X_putpixel_slave(ggi_visual *vis, int x, int y, ggi_pixel col)
{
	ggi_x_priv *priv;
	priv = GGIX_PRIV(vis);

        CHECKXY(vis,x,y);
	priv->slave->opdraw->putpixel_nc(priv->slave, x, y, col);
	GGI_X_DIRTY(vis, x, y, 1, 1);
	return 0;
}

int GGI_X_putpixel_nc_slave(ggi_visual *vis, int x, int y, ggi_pixel col)
{
	ggi_x_priv *priv;
	priv = GGIX_PRIV(vis);

	priv->slave->opdraw->putpixel_nc(priv->slave, x, y, col);
	GGI_X_DIRTY(vis, x, y, 1, 1);
	return 0;
}

int GGI_X_getpixel_slave(ggi_visual *vis,int x, int y, ggi_pixel *pixel)
{	
	ggi_x_priv *priv;
	priv = GGIX_PRIV(vis);

	/* Slave is always up to date */
	return (priv->slave->opdraw->getpixel(priv->slave, x, y, pixel));
}

int GGI_X_drawpixel_slave_draw(ggi_visual *vis, int x, int y)
{
	ggi_x_priv *priv;
	priv = GGIX_PRIV(vis);

        CHECKXY(vis,x,y);
	GGI_X_CLEAN(vis, x, y, 1, 1);
	priv->slave->opdraw->drawpixel_nc(priv->slave, x, y);
	GGI_X_LOCK_XLIB(vis);
	XDrawPoint(priv->disp, priv->drawable, priv->gc, x, GGI_X_WRITE_Y); 
	GGI_X_MAYBE_SYNC(vis);
	GGI_X_UNLOCK_XLIB(vis);
	return 0;
}

int GGI_X_drawpixel_nc_slave_draw(ggi_visual *vis, int x, int y)
{
	ggi_x_priv *priv;
	priv = GGIX_PRIV(vis);

	GGI_X_CLEAN(vis, x, y, 1, 1);
	priv->slave->opdraw->drawpixel_nc(priv->slave, x, y);
	GGI_X_LOCK_XLIB(vis);
	XDrawPoint(priv->disp, priv->drawable, priv->gc, x, GGI_X_WRITE_Y);
	GGI_X_MAYBE_SYNC(vis);
	GGI_X_UNLOCK_XLIB(vis);
	return 0;
}

int GGI_X_drawpixel_draw(ggi_visual *vis, int x, int y)
{
	ggi_x_priv *priv;
	priv = GGIX_PRIV(vis);

	GGI_X_LOCK_XLIB(vis);
	XDrawPoint(priv->disp, priv->drawable, priv->gc, x, GGI_X_WRITE_Y); 
	GGI_X_MAYBE_SYNC(vis);
	GGI_X_UNLOCK_XLIB(vis);
	return 0;
}

int GGI_X_putpixel_draw(ggi_visual *vis, int x, int y, ggi_pixel col)
{
	ggi_x_priv *priv;
	priv = GGIX_PRIV(vis);

        CHECKXY(vis,x,y); /* tempgc is used in flush and has clip=fullscreen. */
	GGI_X_LOCK_XLIB(vis);
        XSetForeground(priv->disp, priv->tempgc, col);
        XDrawPoint(priv->disp, priv->drawable, priv->tempgc, x, GGI_X_WRITE_Y);
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

int GGI_X_getpixel_draw(ggi_visual *vis, int x, int y, ggi_pixel *pixel)
{	
	ggi_x_priv *priv;
	XImage *ximg;
	int     (*olderrorhandler) (Display *, XErrorEvent *);
	int ret = 0;
	priv = GGIX_PRIV(vis);
        
	GGI_X_LOCK_XLIB(vis);
	XSync(priv->disp, 0);
	ggLock(_ggi_global_lock);

	geterror = 0;
	olderrorhandler = XSetErrorHandler(errorhandler);
	/* This will cause a BadMatch error when the window is
	   iconified or on another virtual screen... */
	ximg = XGetImage(priv->disp, priv->drawable, x, GGI_X_READ_Y,
			 1, 1, AllPlanes, ZPixmap);
	XSync(priv->disp, 0);
	XSetErrorHandler(olderrorhandler);
	*pixel = 0;


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
		ximgptr = (uint8_t *)(ximg->data) + 
			(ximg->xoffset * ximg->bits_per_pixel)/8;
		*((uint8_t *)pixel) = *(ximgptr + 1);
		*((uint8_t *)pixel + 1) = *(ximgptr);
	}
	else if (ximg->bits_per_pixel == 32) {
		uint8_t *ximgptr;
		ximgptr = (uint8_t *)(ximg->data) + 
			(ximg->xoffset * ximg->bits_per_pixel)/8;
		*((uint8_t *)pixel) = *(ximgptr + 3);
		*((uint8_t *)pixel + 1) = *(ximgptr + 2);
		*((uint8_t *)pixel + 2) = *(ximgptr + 1);
		*((uint8_t *)pixel + 3) = *(ximgptr);
	}
	else {
		uint8_t *ximgptr;

	noswab:
		ximgptr = (uint8_t *)(ximg->data) + 
			(ximg->xoffset * ximg->bits_per_pixel)/8;
		memcpy(pixel, ximgptr, (size_t)ximg->bits_per_pixel/8);
	}
	XDestroyImage(ximg);
 out:
	ggUnlock(_ggi_global_lock);
	GGI_X_UNLOCK_XLIB(vis);

	return ret;
}
