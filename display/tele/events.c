/* $Id: events.c,v 1.13 2006/10/14 12:54:41 cegger Exp $
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

#include "config.h"
#include <ggi/internal/ggi-dl.h>
#include <ggi/internal/ggi_debug.h>
#include <ggi/display/tele.h>
#include <ggi/input/tele.h>

#include <stdlib.h>
#include <string.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif


#define MINSLEEPTIME  (20*1000)  /* microseconds */


int GGI_tele_listener(void *arg, uint32_t flag, void *data)
{
	struct ggi_visual *vis = arg;
	ggi_tele_priv *priv = TELE_PRIV(vis);

	if ((flag & GII_CMDCODE_EXPOSE) == GII_CMDCODE_EXPOSE) {
		DPRINT("listener: EXPOSE event received\n");
	}

	if ((flag & GII_CMDCODE_TELE_UNLOCK) == GII_CMDCODE_TELE_UNLOCK) {
		DPRINT("listener: TELE_UNLOCK event received\n");

		priv->reply = 1;
	}

	return 0;
}

int tele_receive_reply(struct ggi_visual *vis, TeleEvent *ev, 
			   long type, long seq)
{
	ggi_tele_priv *priv = TELE_PRIV(vis);
	struct gii_cmddata_tele_event w4e;

	ev->size = 0;

	memcpy(&w4e.ev, ev, sizeof(w4e.ev));
	w4e.type = type;
	w4e.sequence = seq;
	priv->reply = 0;

	DPRINT_EVENTS("display-tele: WAITING FOR (type=0x%08lx "
	              "seq=0x%08lx)\n", type, seq);
	ggNotifyObservers(priv->publisher, GII_CMDCODE_TELE_WAIT4EVENT,
			&w4e);

	for (;;) {
		if (!priv->reply) {
			ggUSleep(MINSLEEPTIME);
			continue;
		}

		break;
	}

	DPRINT_EVENTS("display-tele: WAIT OVER (type=0x%08lx "
	              "seq=0x%08lx)\n", type, seq);

	return 0;
}
