/* $Id: box.c,v 1.5 2002/09/08 21:37:47 soyt Exp $
******************************************************************************

   TELE target.

   Copyright (C) 1998 Andrew Apted    [andrew@ggi-project.org]
                 2002 Tobias Hunger   [tobias@fresco.org]

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

#include "libtele.h"
#include <ggi/display/tele.h>

#define BYTES_PER_PIXEL(vis)	(GT_ByPP(LIBGGI_GT(vis)))
#define MAX_PIXELS(vis)		\
	TELE_MAXIMUM_RAW(TeleCmdGetPutData) / BYTES_PER_PIXEL(vis)

int GGI_tele_putpixel_nc(ggi_visual *vis, int x, int y, ggi_pixel col)
{ 
	ggi_tele_priv *priv = TELE_PRIV(vis);
	TeleCmdGetPutData *p;
	TeleEvent ev;

	int err;

	p = tclient_new_event(priv->client, &ev, TELE_CMD_PUTBOX,
			      sizeof(TeleCmdGetPutData)-4, 1);
	p->x = x;
	p->y = y;
	p->width  = 1;
	p->height = 1;
	
	p->pixel[0] = col;

	err = tclient_write(priv->client, &ev);

	if (err == TELE_ERROR_SHUTDOWN) {
		TELE_HANDLE_SHUTDOWN;
	}	/* if */

	return err;
}	/* GGI_tele_putpixel_nc */



int GGI_tele_getpixel(ggi_visual *vis, int x, int y, ggi_pixel *col)
{
	ggi_tele_priv *priv = TELE_PRIV(vis);
	TeleCmdGetPutData *p;
	TeleEvent ev;

	int err;

	if ((x < 0) || (y < 0)
	  || (x >= LIBGGI_MODE(vis)->virt.x)
	  || (y >= LIBGGI_MODE(vis)->virt.y))
	{
	    	/* illegal coordinates */
		return -1;
	}	/* if */


	p = tclient_new_event(priv->client, &ev, TELE_CMD_GETBOX,
				sizeof(TeleCmdGetPutData)-4, 1);
	p->x = x;
	p->y = y;
	p->width  = 1;
	p->height = 1;
	
	err = tclient_write(priv->client, &ev);

	if (err == TELE_ERROR_SHUTDOWN) {
		TELE_HANDLE_SHUTDOWN;
	} else if (err < 0) {
		return err;
	}	/* if */
	
	tele_receive_reply(vis, &ev, TELE_CMD_GETBOX, ev.sequence);

	*col = p->pixel[0];

	return 0;
}	/* GGI_tele_getpixel */


/* ---------------------------------------------------------------------- */


int GGI_tele_putbox(ggi_visual *vis, int x, int y, int w, int h, void *buf)
{
	ggi_tele_priv *priv = TELE_PRIV(vis);
	TeleCmdGetPutData *p;
	TeleEvent ev;

	uint8 *src = (uint8 *)(buf);
	int srcwidth = w;
	int stride = w * BYTES_PER_PIXEL(vis);

	int xstep, ystep;
	int curx;

	LIBGGICLIP_PUTBOX(vis, x,y, w,h, src, srcwidth, );

	xstep = w;
	ystep = (MAX_PIXELS(vis) / w);

	if (ystep == 0) {
		xstep = MAX_PIXELS(vis);
		ystep = 1;
	}	/* if */

	curx = 0;

	while (h > 0) {
		int j, err;

		int ww = (w < xstep) ? w : xstep;
		int hh = (h < ystep) ? h : ystep;

		uint8 *dest;

		p = tclient_new_event(priv->client, &ev, TELE_CMD_PUTBOX,
				      sizeof(TeleCmdGetPutData)-4,
				      ww*hh*BYTES_PER_PIXEL(vis));
		p->x = x + curx;
		p->y = y;
		p->width  = ww;
		p->height = hh;

		dest = (uint8 *)(p->pixel);

		for (j = 0; j < hh; j++) {
		  memcpy(&(dest[j * ww * BYTES_PER_PIXEL(vis)]),
			 &(src[j * stride +
			       (curx) * BYTES_PER_PIXEL(vis)]),
			 ww * BYTES_PER_PIXEL(vis));
		}	/* for */

		err = tclient_write(priv->client, &ev);

		if (err == TELE_ERROR_SHUTDOWN) {
			TELE_HANDLE_SHUTDOWN;
		} else if (err < 0) {
			return err;
		}	/* if */
	
		curx += xstep;
		if (curx >= w) {
			curx = 0;
			src += stride * ystep;
			y += ystep;
			h -= ystep;
		}	/* if */
	}	/* while */

	return 0;
}	/* GGI_tele_putbox */



/* ---------------------------------------------------------------------- */

int GGI_tele_getbox(ggi_visual *vis, int x, int y, int w, int h, void *buf)
{ 
 	ggi_tele_priv *priv = TELE_PRIV(vis);
	TeleCmdGetPutData *p;
	TeleEvent ev;

	uint8 *dest = (uint8 *)buf;
	int stride = w * BYTES_PER_PIXEL(vis);

	int xstep, ystep;
	int curx;

	if ((x < 0) || (y < 0) ||
	    (x+w > LIBGGI_MODE(vis)->virt.x) ||
	    (y+h > LIBGGI_MODE(vis)->virt.y) ||
	    (w <= 0) || (h <= 0)) {

	    	/* invalid request */
		return -1;
	}	/* if */

	xstep = w;
	ystep = (MAX_PIXELS(vis) / w);

	if (ystep == 0) {
		xstep = MAX_PIXELS(vis);
		ystep = 1;
	}	/* if */

	curx = 0; 
	while (h > 0) {
		int j, err;

		int ww = (w < xstep) ? w : xstep;
		int hh = (h < ystep) ? h : ystep;

		uint8 *src;

		p = tclient_new_event(priv->client, &ev, TELE_CMD_GETBOX,
				      sizeof(TeleCmdGetPutData)-4,
				      ww*hh*BYTES_PER_PIXEL(vis));
		p->x = x + curx;
		p->y = y;
		p->width  = ww;
		p->height = hh;
		p->bpp = BYTES_PER_PIXEL(vis);

		err = tclient_write(priv->client, &ev);

		if (err == TELE_ERROR_SHUTDOWN) {
			TELE_HANDLE_SHUTDOWN;
		} else if (err < 0) {
			return err;
		}

		tele_receive_reply(vis, &ev, TELE_CMD_GETBOX, 
					ev.sequence);

		src = (uint8 *)p->pixel;

		for(j = 0; j < hh; ++j) {
		  memcpy(&(dest[(j*stride + curx)]),
			&(src[j * ww * BYTES_PER_PIXEL(vis)]),
				ww * BYTES_PER_PIXEL(vis));
		}	/* for */

		curx += xstep;
		if (curx >= w) {
			curx = 0;
			dest += stride * ystep;
			y += ystep;
			h -= ystep;
		}	/* if */
	}	/* while */

	return 0;
}	/* GGI_tele_getbox */


/* ---------------------------------------------------------------------- */



int GGI_tele_crossblit(ggi_visual *src, int sx, int sy, int w, int h,
                       ggi_visual *vis, int dx, int dy)
{ 
	int err = 0;
	ggi_pixel * packed_buf;
	ggi_color * buf;

	LIBGGICLIP_XYWH(src, sx,sy, w,h);
	LIBGGICLIP_XYWH(vis, dx,dy, w,h);

 
	/*
	 * FIXME: Get real values, don't assume that anything must be
	 * smaller then ggi_pixel
	 */
	packed_buf = malloc(sizeof(ggi_pixel) * w * h);
	if (!packed_buf) {
		err = GGI_ENOMEM;
		goto err0;
	}	/* if */
	buf = malloc(sizeof(ggi_color) * w * h);
	if (!buf) {
		err = GGI_ENOMEM;
		goto err1;
	}	/* if */

	ggiGetBox(src, sx, sy, w, h, packed_buf);

	ggiUnpackPixels(src, packed_buf, buf, w * h);
	ggiPackColors(vis, packed_buf, buf, w * h);

	err = ggiPutBox(vis, dx, dy, w, h, packed_buf);

	free(packed_buf);
	free(buf);

	return err;

err1:
	free(packed_buf);
err0:
	return err;
}	/* GGI_tele_crossblit */
 



/* ---------------------------------------------------------------------- */


int GGI_tele_putpixel(ggi_visual *vis, int x, int y, ggi_pixel col)
{ 
	CHECKXY(vis, x, y);

	return GGI_tele_putpixel_nc(vis, x, y, col);
}	/* GGI_tele_putpixel */


int GGI_tele_puthline(ggi_visual *vis, int x, int y, int w, void *buf)
{
	return GGI_tele_putbox(vis, x, y, w, 1, buf);
}	/* GGI_tele_puthline */


int GGI_tele_putvline(ggi_visual *vis, int x, int y, int h, void *buf)
{
	return GGI_tele_putbox(vis, x, y, 1, h, buf);
}	/* GGI_tele_putvline */


int GGI_tele_gethline(ggi_visual *vis, int x, int y, int w, void *buf)
{
	return GGI_tele_getbox(vis, x, y, w, 1, buf);
}	/* GGI_tele_gethline */


int GGI_tele_getvline(ggi_visual *vis, int x, int y, int h, void *buf)
{
	return GGI_tele_getbox(vis, x, y, 1, h, buf);
}	/* GGI_tele_getvline */
