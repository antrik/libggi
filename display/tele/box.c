/* $Id: box.c,v 1.1 2001/05/12 23:02:27 cegger Exp $
******************************************************************************

   TELE target.

   Copyright (C) 1998 Andrew Apted    [andrew@ggi-project.org]

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

#include <ggi/internal/ggi-dl.h>

#include "libtele.h"
#include <ggi/display/tele.h>


/* !!! FIXME for pixels != 8 bit wide */

#define MAX_PIXELS  TELE_MAXIMUM_RAW(TeleCmdGetPutData)


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
	
	((uint8 *) p->pixel)[0] = col;
	
	err = tclient_write(priv->client, &ev);

	if (err == TELE_ERROR_SHUTDOWN) {
		TELE_HANDLE_SHUTDOWN;
	}

	return err;
}

int GGI_tele_getpixel(ggi_visual *vis, int x, int y, ggi_pixel *col)
{
	ggi_tele_priv *priv = TELE_PRIV(vis);
	TeleCmdGetPutData *p;
	TeleEvent ev;

	int err;

	if ((x < 0) || (y < 0) ||
	    (x >= LIBGGI_MODE(vis)->virt.x) ||
	    (y >= LIBGGI_MODE(vis)->virt.y)) {

	    	/* illegal coordinates */
		return -1;
	}

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
	}
	
	tele_receive_reply(vis, &ev, TELE_CMD_GETBOX, ev.sequence);

	*col = ((uint8 *) p->pixel)[0];

	return 0;
}


/* ---------------------------------------------------------------------- */


int GGI_tele_putbox(ggi_visual *vis, int x, int y, int w, int h, void *buf)
{ 
	ggi_tele_priv *priv = TELE_PRIV(vis);
	TeleCmdGetPutData *p;
	TeleEvent ev;

	uint8 *src = buf;
	int stride = w;

	int xstep, ystep;
	int curx;


	/* clip */

	if ((x+w) > LIBGGI_GC(vis)->clipbr.x) {
		w = LIBGGI_GC(vis)->clipbr.x - x;
	}
	if ((y+h) > LIBGGI_GC(vis)->clipbr.y) {
		h = LIBGGI_GC(vis)->clipbr.y - y;
	}
	if (x < LIBGGI_GC(vis)->cliptl.x) {
		w   -= (LIBGGI_GC(vis)->cliptl.x - x);
		src += (LIBGGI_GC(vis)->cliptl.x - x);
		x = LIBGGI_GC(vis)->cliptl.x;
	}
	if (y < LIBGGI_GC(vis)->cliptl.y) {
		h   -= (LIBGGI_GC(vis)->cliptl.y - y);
		src += (LIBGGI_GC(vis)->cliptl.y - y) * stride;
		y = LIBGGI_GC(vis)->cliptl.y;
	}
	if ((w <= 0) || (h <= 0)) {
		return 0;
	}

	xstep = w;
	ystep = MAX_PIXELS / w;

	if (ystep == 0) {
		xstep = MAX_PIXELS;
		ystep = 1;
	}

	curx = 0; 

	while (h > 0) {
		int i, j, err;

		int ww = (w < xstep) ? w : xstep;
		int hh = (h < ystep) ? h : ystep;

		uint8 *dest;

		p = tclient_new_event(priv->client, &ev, TELE_CMD_PUTBOX,
				      sizeof(TeleCmdGetPutData)-4, ww*hh);
		p->x = x + curx;
		p->y = y;
		p->width  = ww;
		p->height = hh;

		dest = (uint8 *) p->pixel;

		for (j=0; j < hh; j++)
		for (i=0; i < ww; i++) {
			dest[j*ww + i] = src[j*stride + curx + i];
		}

		err = tclient_write(priv->client, &ev);

		if (err == TELE_ERROR_SHUTDOWN) {
			TELE_HANDLE_SHUTDOWN;
		} else if (err < 0) {
			return err;
		}
	

		curx += xstep;

		if (curx >= w) {
			curx = 0;
			src += stride * ystep;
			y += ystep;
			h -= ystep;
		}
	}

	return 0;   /* success */
}


/* ---------------------------------------------------------------------- */


int GGI_tele_getbox(ggi_visual *vis, int x, int y, int w, int h, void *buf)
{ 
	ggi_tele_priv *priv = TELE_PRIV(vis);
	TeleCmdGetPutData *p;
	TeleEvent ev;

	uint8 *dest = buf;
	int stride = w;

	int xstep, ystep;
	int curx;


	if ((x < 0) || (y < 0) ||
	    (x+w > LIBGGI_MODE(vis)->virt.x) ||
	    (y+h > LIBGGI_MODE(vis)->virt.y) ||
	    (w <= 0) || (h <= 0)) {

	    	/* invalid request */
		return -1;
	}

	xstep = w;
	ystep = MAX_PIXELS / w;

	if (ystep == 0) {
		xstep = MAX_PIXELS;
		ystep = 1;
	}

	curx = 0; 

	while (h > 0) {
		int i, j, err;

		int ww = (w < xstep) ? w : xstep;
		int hh = (h < ystep) ? h : ystep;

		uint8 *src;

		p = tclient_new_event(priv->client, &ev, TELE_CMD_GETBOX,
				      sizeof(TeleCmdGetPutData)-4, ww*hh);
		p->x = x + curx;
		p->y = y;
		p->width  = ww;
		p->height = hh;

		err = tclient_write(priv->client, &ev);

		if (err == TELE_ERROR_SHUTDOWN) {
			TELE_HANDLE_SHUTDOWN;
		} else if (err < 0) {
			return err;
		}

		tele_receive_reply(vis, &ev, TELE_CMD_GETBOX, 
					ev.sequence);

		src = (uint8 *) p->pixel;

		for (j=0; j < hh; j++)
		for (i=0; i < ww; i++) {
			dest[j*stride + curx + i] = src[j*ww + i];
		}

		curx += xstep;

		if (curx >= w) {
			curx = 0;
			src += stride * ystep;
			y += ystep;
			h -= ystep;
		}
	}

	return 0;
}


/* ---------------------------------------------------------------------- */


int GGI_tele_putpixel(ggi_visual *vis, int x, int y, ggi_pixel col)
{ 
	CHECKXY(vis, x, y);

	return GGI_tele_putpixel_nc(vis, x, y, col);
}

int GGI_tele_puthline(ggi_visual *vis, int x, int y, int w, void *buf)
{
	return GGI_tele_putbox(vis, x, y, w, 1, buf);
}

int GGI_tele_putvline(ggi_visual *vis, int x, int y, int h, void *buf)
{
	return GGI_tele_putbox(vis, x, y, 1, h, buf);
}

int GGI_tele_gethline(ggi_visual *vis, int x, int y, int w, void *buf)
{
	return GGI_tele_getbox(vis, x, y, w, 1, buf);
}

int GGI_tele_getvline(ggi_visual *vis, int x, int y, int h, void *buf)
{
	return GGI_tele_getbox(vis, x, y, 1, h, buf);
}
