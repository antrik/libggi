/* $Id: listener.c,v 1.11 2008/02/21 07:48:15 cegger Exp $
******************************************************************************

   LibGGI - listener for display-quartz

   Copyright (C) 2006	Christoph Egger

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
#include "quartz.h"
#include <ggi/input/quartz.h>
#include <ggi/internal/ggi_debug.h>


int GGI_quartz_listener(void *arg, uint32_t ctl, void *data)
{
	struct ggi_visual *vis = arg;
	ggi_quartz_priv *priv = QUARTZ_PRIV(vis);

	switch (ctl) {
	case GII_CMDCODE_RESIZE:
		do {
			gii_event ev;
			struct gii_cmddata_resize *resize;
			ggi_cmddata_switchrequest *swreq;
			int xsize, ysize;

			resize = (struct gii_cmddata_resize *)data;
			DPRINT_MISC("GGI_quartz_listener: received GII_CMDCODE_RESIZE event\n"); 

			xsize = resize->curRect.right - resize->curRect.left;
			ysize = resize->curRect.bottom - resize->curRect.top;

			ev.any.size = sizeof(gii_cmd_nodata_event) +
				sizeof(ggi_cmddata_switchrequest);
			ev.any.type = evCommand;
			ev.cmd.code = GGICMD_REQUEST_SWITCH;

			swreq = (ggi_cmddata_switchrequest *)ev.cmd.data;
			swreq->request = GGI_REQSW_MODE;
			swreq->mode = *LIBGGI_MODE(vis);

			swreq->mode.visible.x = xsize;
			swreq->mode.visible.y = ysize;
			if (swreq->mode.virt.x < xsize)
				swreq->mode.virt.x = xsize;
			if (swreq->mode.virt.y < ysize)
				swreq->mode.virt.y = ysize;
			swreq->mode.size.x = GGI_AUTO;
			swreq->mode.size.y = GGI_AUTO;

			ggControl(priv->inp->channel, GII_CMDCODE_RESIZE, &ev);
		} while (0);
		break;

	case GII_CMDCODE_QZWINCLOSE:
		do {
			DPRINT_MISC("GGI_quartz_listener: received GII_CMDCODE_QZWINCLOSE event\n"); 
		} while (0);
		break;
	}

	return 0;
}
