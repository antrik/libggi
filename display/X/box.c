/* $Id: box.c,v 1.10 2004/11/03 08:36:01 cegger Exp $
******************************************************************************

   LibGGI - boxes for display-x

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
#include <ggi/internal/ggi_debug.h>
#include <ggi/display/x.h>

int GGI_X_drawbox_slave(ggi_visual *vis, int x, int y, int w, int h)
{
	ggi_x_priv *priv;
	priv = GGIX_PRIV(vis);

	LIBGGI_ASSERT(priv->slave != NULL, "GGIX_PRIV(vis)->slave == NULL");

	LIBGGICLIP_XYWH(vis, x, y, w, h);
	priv->slave->opdraw->drawbox(priv->slave, x, y, w, h);
	GGI_X_DIRTY(vis, x, y, w, h);
	return GGI_OK;
}

int GGI_X_putbox_slave(ggi_visual *vis, int x, int y, int w, int h, void *data)
{
	ggi_x_priv *priv;
	priv = GGIX_PRIV(vis);

	LIBGGI_ASSERT(priv->slave != NULL, "GGIX_PRIV(vis)->slave == NULL");

	priv->slave->opdraw->putbox(priv->slave, x, y, w, h, data);
	LIBGGICLIP_XYWH(vis, x, y, w, h);
	GGI_X_DIRTY(vis, x, y, w, h);
	return GGI_OK;
}

int GGI_X_getbox_slave(ggi_visual *vis, int x, int y, int w, int h, void *data)
{	
	ggi_x_priv *priv;
	priv = GGIX_PRIV(vis);

	LIBGGI_ASSERT(priv->slave != NULL, "GGIX_PRIV(vis)->slave == NULL");

	/* Slave is always up to date */
	return (priv->slave->opdraw->getbox(priv->slave, x, y, w, h, data));
}

int GGI_X_copybox_slave(ggi_visual *vis, int x, int y, 
			int w, int h, int nx, int ny)
{
	ggi_x_priv *priv;
	priv = GGIX_PRIV(vis);

	LIBGGI_ASSERT(priv->slave != NULL, "GGIX_PRIV(vis)->slave == NULL");

	priv->slave->opdraw->copybox(priv->slave, x, y, w, h, nx, ny);
	LIBGGICLIP_COPYBOX(vis,x,y,w,h,nx,ny);
	GGI_X_DIRTY(vis, nx, ny, w, h);
	return GGI_OK;
}

int GGI_X_drawbox_slave_draw(ggi_visual *vis, int x, int y, int w, int h)
{
	ggi_x_priv *priv;
	priv = GGIX_PRIV(vis);

	LIBGGI_ASSERT(priv->slave != NULL, "GGIX_PRIV(vis)->slave == NULL");

	LIBGGICLIP_XYWH(vis, x, y, w, h);
	GGI_X_CLEAN(vis, x, y, w, h);
	priv->slave->opdraw->drawbox(priv->slave, x, y, w, h);
	y = GGI_X_WRITE_Y;
	ggLock(priv->xliblock);
	XFillRectangle(priv->disp, priv->drawable, priv->gc,
			x, y, (unsigned)w, (unsigned)h);
	GGI_X_MAYBE_SYNC(vis);
	ggUnlock(priv->xliblock);
	return GGI_OK;
}

int GGI_X_copybox_slave_draw(ggi_visual *vis, int x, int y, 
			     int w, int h, int nx, int ny)
{
	ggi_x_priv *priv;
	priv = GGIX_PRIV(vis);

	LIBGGI_ASSERT(priv->slave != NULL, "GGIX_PRIV(vis)->slave == NULL");

	LIBGGICLIP_COPYBOX(vis,x,y,w,h,nx,ny);
	/* XCopyArea will generate appropriate expose events, 
	   so it's OK to declare the area clean. */
	GGI_X_CLEAN(vis, nx, ny, w, h);	
	priv->slave->opdraw->copybox(priv->slave, x, y, w, h, nx, ny);
	y = GGI_X_READ_Y;
	ny += LIBGGI_VIRTY(vis) * vis->w_frame_num;
	ggLock(priv->xliblock);
	XCopyArea(priv->disp,  priv->drawable, priv->drawable,
		  priv->gc, x, y, (unsigned)w, (unsigned)h, nx, ny);
	GGI_X_MAYBE_SYNC(vis);
	ggUnlock(priv->xliblock);
	return GGI_OK;
}

int GGI_X_drawbox_draw(ggi_visual *vis, int x, int y, int w, int h)
{
	ggi_x_priv *priv;
	priv = GGIX_PRIV(vis);

	y = GGI_X_WRITE_Y;
	ggLock(priv->xliblock);
	XFillRectangle(priv->disp, priv->drawable, priv->gc,
                       x, y, (unsigned)w, (unsigned)h);
	GGI_X_MAYBE_SYNC(vis);
	ggUnlock(priv->xliblock);
	return GGI_OK;
}

int GGI_X_putbox_draw(ggi_visual *vis, int x, int y, int w, int h, void *data)
{
	XImage *ximg;
	ggi_x_priv *priv;
	priv = GGIX_PRIV(vis);

	ggLock(priv->xliblock);
#warning 1,2,4-bit support needed.
	ximg = XCreateImage(priv->disp, priv->vilist[priv->viidx].vi->visual,
			    (unsigned)LIBGGI_PIXFMT(vis)->depth, ZPixmap, 0,
			    data, (unsigned)w, (unsigned)h, 8, 0);
	y = GGI_X_WRITE_Y;

#ifdef GGI_LITTLE_ENDIAN
	ximg->byte_order = LSBFirst;
	ximg->bitmap_bit_order = LSBFirst;
#else
	ximg->byte_order = MSBFirst;
	ximg->bitmap_bit_order = MSBFirst;
#endif

	XPutImage(priv->disp, priv->drawable, priv->gc, ximg,
		  0, 0, x, y, (unsigned)w, (unsigned)h);
	XFree(ximg); /* XDestroyImage would free(data) (bad).
			Luckily, this doesn't leak (?) */
	GGI_X_MAYBE_SYNC(vis);
	ggUnlock(priv->xliblock);
	return GGI_OK;
}

int GGI_X_copybox_draw(ggi_visual *vis, int x, int y, 
		       int w, int h, int nx, int ny)
{
	ggi_x_priv *priv;
	priv = GGIX_PRIV(vis);

	y = GGI_X_READ_Y;
	ny += LIBGGI_VIRTY(vis) * vis->w_frame_num;
	ggLock(priv->xliblock);
	XCopyArea(priv->disp,  priv->drawable, priv->drawable,
		  priv->gc, x, y, (unsigned)w, (unsigned)h, nx, ny);
	GGI_X_MAYBE_SYNC(vis);
	ggUnlock(priv->xliblock);
	return GGI_OK;
}

static int geterror;

static int errorhandler (Display * disp, XErrorEvent * event)
{
	if (event->error_code == BadMatch) geterror = 1;
	return 0;
}

int GGI_X_getbox_draw(ggi_visual *vis, int x, int y, int w, int h, void *data)
{
	ggi_x_priv *priv;
	XImage *ximg;
	int     (*olderrorhandler) (Display *, XErrorEvent *);
	int ret = 0;
	uint8 *data8;
	priv = GGIX_PRIV(vis);

	/* TODO: chunk transfer for performance/memory profile.
	   hlines are too small.  Find out optimal chunk size.
	 */

	ggLock(priv->xliblock);
	XSync(priv->disp, 0);
	ggLock(_ggi_global_lock);

	geterror = 0;
	olderrorhandler = XSetErrorHandler(errorhandler);
	/* This will cause a BadMatch error when the window is
	   iconified or on another virtual screen... */
	ximg = XGetImage(priv->disp, priv->drawable, x, GGI_X_READ_Y,
			 (unsigned)w, (unsigned)h, AllPlanes, ZPixmap);
	XSync(priv->disp, 0);
	XSetErrorHandler(olderrorhandler);

	if (geterror) {
		ret = -1;
		goto out;
	}

#warning honor all ximage format fields here.
#warning 1,2,4-bpp support needed.

	if (ximg->byte_order == 
#ifdef GGI_LITTLE_ENDIAN
	    LSBFirst
#else
	    MSBFirst
#endif
	    ) goto noswab;

	if (ximg->bits_per_pixel == 16) {
		uint8 *ximgptr;
		ximgptr = (uint8 *)(ximg->data) + ximg->xoffset * 2;
		data8 = (uint8 *)data;
		while (h--) {
			int j;
			for (j = 0; j < w * 2; j += 2) {
				*(data8 + j) = *(ximgptr + j + 1);
				*(data8 + j + 1) = *(ximgptr + j);
			}
			ximgptr += ximg->bytes_per_line;
			data8 += ximg->width * 2;
		}
	}
	else if (ximg->bits_per_pixel == 32) {
		uint8 *ximgptr;
		ximgptr = (uint8 *)(ximg->data) + ximg->xoffset * 4;
		data8 = (uint8 *)data;
		while (h--) {
			int j;
			for (j = 0; j < w * 4; j += 4) {
				*(data8 + j) = *(ximgptr + j + 3);
				*(data8 + j + 1) = *(ximgptr + j + 2);
				*(data8 + j + 2) = *(ximgptr + j + 1);
				*(data8 + j + 3) = *(ximgptr + j);
			}
			ximgptr += ximg->bytes_per_line;
			data8 += ximg->width * 4;
		}
	}
	else {
		uint8 *ximgptr;

	noswab:

		ximgptr = (uint8 *)(ximg->data) + 
			(ximg->xoffset * ximg->bits_per_pixel)/8;
		data8 = (uint8 *)data;
		while (h--) {
			memcpy(data8, ximgptr,
				(size_t)(w * ximg->bits_per_pixel)/8);
			ximgptr += ximg->bytes_per_line;
			data8 += 
			  ximg->width * ximg->bits_per_pixel/8;
		}
	}
	XDestroyImage(ximg);
 out:
	ggUnlock(_ggi_global_lock);
	ggUnlock(priv->xliblock);

	return ret;
}
