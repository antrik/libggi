/* $Id: events.c,v 1.9 2006/09/29 23:07:54 cegger Exp $
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

#include <stdlib.h>
#include <string.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif


#define MINSLEEPTIME  (20*1000)  /* microseconds */

int tele_receive_reply(struct ggi_visual *vis, TeleEvent *ev, 
			   long type, long seq)
{
	ggi_tele_priv *priv = TELE_PRIV(vis);

	ev->size = 0;

	priv->wait_event = ev;
	priv->wait_type  = type;
	priv->wait_sequence = seq;

	DPRINT_EVENTS("display-tele: WAITING FOR (type=0x%08lx "
	              "seq=0x%08lx)\n", type, seq);

	for (;;) {
		struct timeval tv;

		tv.tv_sec = tv.tv_usec = 0;

		GII_tele_poll(priv->input, NULL);

		if (ev->size != 0) {
			break;
		}

		ggUSleep(MINSLEEPTIME);
	}

	DPRINT_EVENTS("display-tele: WAIT OVER (type=0x%08lx "
	              "seq=0x%08lx)\n", type, seq);

	priv->wait_event = NULL;

	return 0;
}
