/* $Id: draw.c,v 1.3 2002/09/06 09:25:20 cegger Exp $
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


int GGI_tele_drawpixel_nc(ggi_visual *vis, int x, int y)
{
	return GGI_tele_putpixel_nc(vis, x, y, LIBGGI_GC_FGCOLOR(vis));
}	/* GGI_tele_drawpixel_nc */


int GGI_tele_drawpixel(ggi_visual *vis, int x, int y)
{
	CHECKXY(vis, x, y);

	return GGI_tele_putpixel_nc(vis, x, y, LIBGGI_GC_FGCOLOR(vis));
}	/* GGI_tele_drawpixel */



int GGI_tele_drawline(struct ggi_visual *vis, int x,int y, int xe,int ye)
{
	ggi_tele_priv *priv = TELE_PRIV(vis);
	TeleCmdDrawLineData *p;
	TeleEvent ev;

	int err;

	p = tclient_new_event(priv->client, &ev, TELE_CMD_DRAWLINE,
			      sizeof(TeleCmdDrawLineData), 0);
	p->x = x;
	p->y = y;
	p->xe  = xe;
	p->ye = ye;
	p->pixel  = LIBGGI_GC_FGCOLOR(vis);

	err = tclient_write(priv->client, &ev);

	if (err == TELE_ERROR_SHUTDOWN) {
		TELE_HANDLE_SHUTDOWN;
	}	/* if */

	return err;
}	/* GGI_tele_drawline */



/* FIXME !!! this is confused */

int GGI_tele_drawbox_nc(ggi_visual *vis, int x, int y, int w, int h)
{
	ggi_tele_priv *priv = TELE_PRIV(vis);
	TeleCmdDrawBoxData *p;
	TeleEvent ev;

	int err;

	p = tclient_new_event(priv->client, &ev, TELE_CMD_DRAWBOX,
			      sizeof(TeleCmdDrawBoxData), 0);
	p->x = x;
	p->y = y;
	p->width  = w;
	p->height = h;
	p->pixel  = LIBGGI_GC_FGCOLOR(vis);

	err = tclient_write(priv->client, &ev);

	if (err == TELE_ERROR_SHUTDOWN) {
		TELE_HANDLE_SHUTDOWN;
	}	/* if */

	return err;
}	/* GGI_tele_drawbox_nc */


int GGI_tele_drawbox(ggi_visual *vis, int x, int y, int w, int h)
{
	ggi_tele_priv *priv = TELE_PRIV(vis);
	TeleCmdDrawBoxData *p;
	TeleEvent ev;

	int err;

	LIBGGICLIP_XYWH(vis, x, y, w, h);

	p = tclient_new_event(priv->client, &ev, TELE_CMD_DRAWBOX,
			      sizeof(TeleCmdDrawBoxData), 0);
	p->x = x;
	p->y = y;
	p->width  = w;
	p->height = h;
	p->pixel  = LIBGGI_GC_FGCOLOR(vis);

	err = tclient_write(priv->client, &ev);

	if (err == TELE_ERROR_SHUTDOWN) {
		TELE_HANDLE_SHUTDOWN;
	}	/* if */

	return err;
}	/* GGI_tele_drawbox */


int GGI_tele_copybox(ggi_visual *vis, int x, int y, int w, int h,
			int nx, int ny)
{
	ggi_tele_priv *priv = TELE_PRIV(vis);
	TeleCmdCopyBoxData *p;
	TeleEvent ev;

	int err;

	LIBGGICLIP_COPYBOX(vis, x,y,w,h, nx,ny);

	p = tclient_new_event(priv->client, &ev, TELE_CMD_COPYBOX,
			      sizeof(TeleCmdCopyBoxData), 0);
	p->sx = x;
	p->sy = y;
	p->width  = w;
	p->height = h;
	p->dx = nx;
	p->dy = ny;
	
	err = tclient_write(priv->client, &ev);

	if (err == TELE_ERROR_SHUTDOWN) {
		TELE_HANDLE_SHUTDOWN;
	}	/* if */

	return err;
}	/* GGI_tele_copybox */


/* ---------------------------------------------------------------------- */


int GGI_tele_drawhline_nc(ggi_visual *vis, int x, int y, int w)
{
	return GGI_tele_drawbox_nc(vis, x, y, w, 1);
}	/* GGI_tele_drawhline_nc */


int GGI_tele_drawvline_nc(ggi_visual *vis, int x, int y, int h)
{
	return GGI_tele_drawbox_nc(vis, x, y, 1, h);
}	/* GGI_tele_drawvline_nc */


int GGI_tele_drawhline(ggi_visual *vis, int x, int y, int w)
{
	return GGI_tele_drawbox(vis, x, y, w, 1);
}	/* GGI_tele_drawhline */


int GGI_tele_drawvline(ggi_visual *vis, int x, int y, int h)
{
	return GGI_tele_drawbox(vis, x, y, 1, h);
}	/* GGI_tele_drawvline */
