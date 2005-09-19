/* $Id: mode.c,v 1.15 2005/09/19 18:46:43 cegger Exp $
******************************************************************************

   TELE target.

   Copyright (C) 1998 Andrew Apted	[andrew@ggi-project.org]
   Copyright (C) 2000 Marcus Sundberg	[marcus@ggi-project.org]

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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../common/gt-auto.inc"

#include "libtele.h"
#include <ggi/display/tele.h>


static int GGI_tele_getapi(ggi_visual *vis, int num,
			char *apiname, char *arguments)
{
	ggi_graphtype gt = LIBGGI_GT(vis);

	*arguments = '\0';

	switch(num) {
		case 0: strcpy(apiname, "display-tele");
			return 0;

		case 1: strcpy(apiname, "generic-stubs");
			return 0;

		case 2: if (GT_SCHEME(gt) == GT_TEXT) {
				return GGI_ENOMATCH;
			}

			strcpy(apiname, "generic-color");
			return 0;
	}

	return GGI_ENOMATCH;
}

int GGI_tele_resetmode(ggi_visual *vis)
{
	ggi_tele_priv *priv = TELE_PRIV(vis);
	TeleEvent ev;

	if (LIBGGI_PAL(vis)->clut.data) {
		free(LIBGGI_PAL(vis)->clut.data);
		LIBGGI_PAL(vis)->clut.data = NULL;
	}
	if (LIBGGI_PAL(vis)->priv) {
		free(LIBGGI_PAL(vis)->priv);
		LIBGGI_PAL(vis)->priv = NULL;
	}

	tclient_new_event(priv->client, &ev, TELE_CMD_CLOSE, 0, 0);

	priv->mode_up = 0;

	return tclient_write(priv->client, &ev);
}

static int GGI_tele_getpixelfmt(ggi_visual *vis, ggi_pixelformat * format)
{
  ggi_tele_priv *priv = TELE_PRIV(vis);
  int err;
  TeleEvent ev;
  TeleCmdPixelFmtData *d;

  d = tclient_new_event(priv->client, &ev, TELE_CMD_GETPIXELFMT, 
			sizeof(TeleCmdPixelFmtData), 0);

  err = tclient_write(priv->client, &ev);
  
  if (err == TELE_ERROR_SHUTDOWN) {
    TELE_HANDLE_SHUTDOWN;
  } else if (err < 0) {
    return err;
  }
  
  /* get reply */
  err = tele_receive_reply(vis, &ev, TELE_CMD_GETPIXELFMT, ev.sequence);

  format->depth         = (int)d->depth;
  format->size          = (int)d->size;
  format->red_mask      = (ggi_pixel)d->red_mask;
  format->green_mask    = (ggi_pixel)d->green_mask;
  format->blue_mask     = (ggi_pixel)d->blue_mask;
  format->alpha_mask    = (ggi_pixel)d->alpha_mask;
  format->clut_mask     = (ggi_pixel)d->clut_mask;
  format->fg_mask       = (ggi_pixel)d->fg_mask;
  format->bg_mask       = (ggi_pixel)d->bg_mask;
  format->texture_mask  = (ggi_pixel)d->texture_mask;
  format->flags         = (uint32_t)d->flags;
  format->stdformat     = (uint32_t)d->stdformat;

  _ggi_build_pixfmt(format);

  return err;
}

int GGI_tele_setmode(ggi_visual *vis, ggi_mode *mode)
{
	ggi_tele_priv *priv = TELE_PRIV(vis);
	TeleCmdOpenData *w;
	TeleEvent ev;

	ggi_pixelformat * format;

	char libname[200], libargs[200];
	int id, err;
	
	
	/* if window already open, close it */
	if (priv->mode_up) {
	        GGI_tele_resetmode(vis);
	}

        if ((err = GGI_tele_checkmode(vis, mode)) != 0) {
		return err;
	}

	memcpy(LIBGGI_MODE(vis), mode, sizeof(ggi_mode));

	format = LIBGGI_PIXFMT(vis);
	memset(format, 0, sizeof(ggi_pixelformat));
	_ggi_build_pixfmt(format);	

	/* set up palette */
	if (GT_SCHEME(LIBGGI_GT(vis)) == GT_PALETTE) {
	        LIBGGI_PAL(vis)->clut.size = 1 << GT_DEPTH(LIBGGI_GT(vis));
		LIBGGI_PAL(vis)->clut.data = _ggi_malloc(LIBGGI_PAL(vis)->clut.size * sizeof(ggi_color));
		LIBGGI_PAL(vis)->setPalette = GGI_tele_setPalette;
	}

	/* send open window request */
	w = tclient_new_event(priv->client, &ev, TELE_CMD_OPEN,
			      sizeof(TeleCmdOpenData), 0);


	w->graphtype      = (T_Long) mode->graphtype;
	w->frames         = (T_Long) mode->frames;
	w->visible.width  = (T_Long) mode->visible.x;
	w->visible.height = (T_Long) mode->visible.y;
	w->size.width     = (T_Long) mode->size.x;
	w->size.height    = (T_Long) mode->size.y;
	w->virt.width     = (T_Long) mode->virt.x;
	w->virt.height    = (T_Long) mode->virt.y;
	w->dot.width      = (T_Long) mode->dpp.x;
	w->dot.height     = (T_Long) mode->dpp.y;

	err = tclient_write(priv->client, &ev);

	if (err == TELE_ERROR_SHUTDOWN) {
		TELE_HANDLE_SHUTDOWN;
	} else if (err < 0) {
		return err;
	}

	/* receive reply */
	tele_receive_reply(vis, &ev, TELE_CMD_OPEN, ev.sequence);

	if (! w->error) {
		priv->mode_up = 1;
	}

	mode->graphtype = (ggi_graphtype) w->graphtype;
	mode->frames    = (uint32_t) w->frames;
	mode->visible.x = (int16_t) w->visible.width;
	mode->visible.y = (int16_t) w->visible.height;
	mode->virt.x    = (int16_t) w->virt.width;
	mode->virt.y    = (int16_t) w->virt.height;
	mode->size.x    = (int16_t) w->size.width;
	mode->size.y    = (int16_t) w->size.height;
	mode->dpp.x     = (int16_t) w->dot.width;
	mode->dpp.y     = (int16_t) w->dot.height;

	priv->width  = mode->virt.x;
	priv->height = mode->virt.y;

	if ((err = GGI_tele_getpixelfmt(vis, format)) != 0) {
	  DPRINT_MODE("GGI_tele_setmode: FAILED to set Pixelformat! (%d)\n",
			 err);
	  return err;
	}

	/* load libraries */
	for (id=1; 0==GGI_tele_getapi(vis, id, libname, libargs); id++) {
		err = _ggiOpenDL(vis, _ggiGetConfigHandle(),
				libname, libargs, NULL);
		if (err) {
			fprintf(stderr,"display-tele: Can't open the "
				"%s (%s) library.\n", libname, libargs);
			return GGI_EFATAL;
		} else {
			DPRINT_MODE("display-tele: Success in loading "
				"%s (%s)\n", libname, libargs);
		}
	}

	/* override stuff */
	vis->opdraw->putpixel_nc=GGI_tele_putpixel_nc;
	vis->opdraw->putpixel=GGI_tele_putpixel;
	vis->opdraw->puthline=GGI_tele_puthline;
	vis->opdraw->putvline=GGI_tele_putvline;
	vis->opdraw->putbox=GGI_tele_putbox;

	vis->opdraw->getpixel=GGI_tele_getpixel;
	vis->opdraw->gethline=GGI_tele_gethline;
	vis->opdraw->getvline=GGI_tele_getvline;
	vis->opdraw->getbox=GGI_tele_getbox;

	vis->opdraw->drawpixel_nc=GGI_tele_drawpixel_nc;
	vis->opdraw->drawpixel=GGI_tele_drawpixel;
	vis->opdraw->drawhline_nc=GGI_tele_drawhline_nc;
	vis->opdraw->drawhline=GGI_tele_drawhline;
	vis->opdraw->drawvline_nc=GGI_tele_drawvline_nc;
	vis->opdraw->drawvline=GGI_tele_drawvline;
	vis->opdraw->drawline=GGI_tele_drawline;
	vis->opdraw->drawbox=GGI_tele_drawbox;
	vis->opdraw->copybox=GGI_tele_copybox;
	vis->opdraw->crossblit=GGI_tele_crossblit;

	vis->opdraw->putc=GGI_tele_putc;
	vis->opdraw->puts=GGI_tele_puts;
	vis->opdraw->getcharsize=GGI_tele_getcharsize;

	vis->opdraw->setorigin=GGI_tele_setorigin;

	return err;
}

int GGI_tele_checkmode(ggi_visual *vis, ggi_mode *mode)
{
	ggi_tele_priv *priv = TELE_PRIV(vis);
	TeleCmdOpenData *w;
	TeleEvent ev;
	int err = 0;

	mode->graphtype = _GGIhandle_gtauto(mode->graphtype);

	if (mode->visible.x > mode->virt.x) {
		mode->virt.x = mode->visible.x;
		err = -1;
	}

	if (mode->visible.y > mode->virt.y) {
		mode->virt.y = mode->visible.y;
		err = -1;
	}

	if (mode->size.x != GGI_AUTO || mode->size.y != GGI_AUTO) {
		err = -1;
	}
	mode->size.x = mode->size.y = GGI_AUTO;

	/* pass check onto server */
	DPRINT_MODE("GGI_tele_checkmode: Sending check request...\n");

	w = tclient_new_event(priv->client, &ev, TELE_CMD_CHECK,
			      sizeof(TeleCmdOpenData), 0);

	w->graphtype      = (T_Long) mode->graphtype;
	w->frames         = (T_Long) mode->frames;
	w->visible.width  = (T_Long) mode->visible.x;
	w->visible.height = (T_Long) mode->visible.y;
	w->size.width     = (T_Long) mode->size.x;
	w->size.height    = (T_Long) mode->size.y;
	w->virt.width     = (T_Long) mode->virt.x;
	w->virt.height    = (T_Long) mode->virt.y;
	w->dot.width      = (T_Long) mode->dpp.x;
	w->dot.height     = (T_Long) mode->dpp.y;

	err = tclient_write(priv->client, &ev);

	if (err == TELE_ERROR_SHUTDOWN) {
		TELE_HANDLE_SHUTDOWN;
	} else if (err < 0) {
		return err;
	}

	/* get reply */
	DPRINT_MODE("GGI_tele_checkmode: Waiting for reply...\n");

	tele_receive_reply(vis, &ev, TELE_CMD_CHECK, ev.sequence);

	DPRINT_MODE("GGI_tele_checkmode: REPLY %d...\n", (int) w->error);

	mode->graphtype = (ggi_graphtype) w->graphtype;
	mode->frames    = (uint32_t) w->frames;
	mode->visible.x = (int16_t) w->visible.width;
	mode->visible.y = (int16_t) w->visible.height;
	mode->virt.x    = (int16_t) w->virt.width;
	mode->virt.y    = (int16_t) w->virt.height;
	mode->size.x    = (int16_t) w->size.width;
	mode->size.y    = (int16_t) w->size.height;
	mode->dpp.x     = (int16_t) w->dot.width;
	mode->dpp.y     = (int16_t) w->dot.height;

	return 0; /* w->error; */
}

int GGI_tele_getmode(ggi_visual *vis, ggi_mode *mode)
{
	ggi_tele_priv *priv = TELE_PRIV(vis);

	if (! priv->mode_up) {
		return GGI_ENOMATCH;
	}

	memcpy(mode, LIBGGI_MODE(vis), sizeof(ggi_mode));

	return 0;
}

int GGI_tele_setorigin(ggi_visual *vis, int x, int y)
{
	ggi_tele_priv *priv = TELE_PRIV(vis);
	TeleCmdSetOriginData *d;
	TeleEvent ev;

        int max_x = LIBGGI_VIRTX(vis) - LIBGGI_X(vis);
        int max_y = LIBGGI_VIRTY(vis) - LIBGGI_Y(vis);

	int err;


	if ((x < 0) || (y < 0) || (x > max_x) || (y > max_y)) {
		DPRINT("display-tele: setorigin out of range:"
			"(%d,%d) > (%d,%d)\n", x, y, max_x, max_y);
		return GGI_ENOSPACE;
	}

	d = tclient_new_event(priv->client, &ev, TELE_CMD_SETORIGIN,
			      sizeof(TeleCmdSetOriginData), 0);
	d->x = x;
	d->y = y;

	err = tclient_write(priv->client, &ev);

	if (err == TELE_ERROR_SHUTDOWN) {
		TELE_HANDLE_SHUTDOWN;
	}

	vis->origin_x = x;
	vis->origin_y = y;

	return err;
}
