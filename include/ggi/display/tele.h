/* $Id: tele.h,v 1.15 2008/01/20 19:26:44 pekberg Exp $
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

#ifndef _GGI_DISPLAY_TELE_H
#define _GGI_DISPLAY_TELE_H

#include "libtele.h"

#include <ggi/internal/ggi-dl.h>


typedef struct {
	TeleClient *client;

	int connected;
	int mode_up;

	struct gg_instance *input;

	int width, height;  /* for generating evExpose events */
	int reply;

	struct gg_observer *observer;
} ggi_tele_priv;

#define TELE_PRIV(vis)  ((ggi_tele_priv *) LIBGGI_PRIVATE(vis))


#define TELE_HANDLE_SHUTDOWN  \
	do {  \
		fprintf(stderr, "display-tele: Server GONE !\n");  \
		exit(2);  \
	} while(0)


/* internal functions */

int GGI_tele_resetmode(struct ggi_visual *vis);

int tele_receive_reply(struct ggi_visual *vis,
			TeleEvent *ev, long type, long seq);

int GGI_tele_listener(void *arg, uint32_t flag, void *data);


/* Prototypes
 */

ggifunc_getmode		GGI_tele_getmode;
ggifunc_setmode		GGI_tele_setmode;
ggifunc_checkmode	GGI_tele_checkmode;
ggifunc_flush		GGI_tele_flush;

ggifunc_setPalette	GGI_tele_setPalette;
ggifunc_putpixel_nc	GGI_tele_putpixel_nc;
ggifunc_putpixel	GGI_tele_putpixel;
ggifunc_puthline	GGI_tele_puthline;
ggifunc_putvline	GGI_tele_putvline;
ggifunc_putbox		GGI_tele_putbox;
ggifunc_getpixel_nc	GGI_tele_getpixel_nc;
ggifunc_getpixel	GGI_tele_getpixel;
ggifunc_gethline	GGI_tele_gethline;
ggifunc_getvline	GGI_tele_getvline;
ggifunc_getbox		GGI_tele_getbox;
ggifunc_drawpixel_nc	GGI_tele_drawpixel_nc;
ggifunc_drawpixel	GGI_tele_drawpixel;
ggifunc_drawhline_nc	GGI_tele_drawhline_nc;
ggifunc_drawhline	GGI_tele_drawhline;
ggifunc_drawvline_nc	GGI_tele_drawvline_nc;
ggifunc_drawvline	GGI_tele_drawvline;
ggifunc_drawline	GGI_tele_drawline;
ggifunc_drawbox		GGI_tele_drawbox;
ggifunc_drawbox		GGI_tele_drawbox_nc;
ggifunc_copybox		GGI_tele_copybox;
ggifunc_crossblit	GGI_tele_crossblit;
ggifunc_putc		GGI_tele_putc;
ggifunc_puts		GGI_tele_puts;
ggifunc_getcharsize	GGI_tele_getcharsize;
ggifunc_setorigin	GGI_tele_setorigin;


#endif /* _GGI_DISPLAY_TELE_H */
