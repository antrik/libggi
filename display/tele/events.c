/* $Id: events.c,v 1.1 2001/05/12 23:02:28 cegger Exp $
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

#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <ggi/internal/ggi-dl.h>

#include <ggi/display/tele.h>


#define MINSLEEPTIME  (20*1000)  /* microseconds */


static void handle_telecmd_event(ggi_tele_priv *priv, TeleEvent *ev)
{
	/* handle TELE_CMD_CLOSE ??? */

	if ((priv->wait_event == NULL) ||
	    (priv->wait_type != ev->type) ||
	    (priv->wait_sequence != ev->sequence)) {
		
		GGIDPRINT_MISC("display-tele: UNEXPECTED CMD EVENT "
		            "(type=0x%08x seq=0x%08x)\n",
			    ev->type, ev->sequence);
		return;
	}

	GGIDPRINT_EVENTS("display-tele: GOT REPLY (type=0x%08lx "
		      "seq=0x%08lx)\n", ev->type, ev->sequence);

	memcpy(priv->wait_event, ev, ev->size * sizeof(long));
}


static int translate_to_ggi(gii_input *inp, gii_event *ev, TeleEvent *tv)
{
	ggi_tele_priv *priv = (ggi_tele_priv *) inp->priv;
	
	if ((tv->type & TELE_EVENT_TYPE_MASK) != TELE_INP_BASE) {
		GGIDPRINT_MISC("display-tele: unrecognised event from "
			    "server (0x%08x).\n", tv->type);
		return -1;
	}
	
	_giiEventBlank(ev, sizeof(gii_event));

	ev->any.time.tv_sec  = tv->time.sec;
	ev->any.time.tv_usec = tv->time.nsec / 1000;

	ev->any.origin = tv->device;

	switch (tv->type) {
	
		case TELE_INP_KEY:
		case TELE_INP_KEYUP:
		{
			TeleInpKeyData *d = (void *) tv->data;

			ev->any.size = sizeof(gii_key_event);
			ev->any.type = (tv->type == TELE_INP_KEY) ?
				evKeyPress : evKeyRelease;

			ev->key.modifiers = d->modifiers;

			ev->key.sym    = d->key;
			ev->key.label  = d->label;
			ev->key.button = d->button;

			return 0;
		}

		case TELE_INP_BUTTON:
		case TELE_INP_BUTTONUP:
		{
			TeleInpButtonData *d = (void *) tv->data;

			ev->any.size = sizeof(gii_pbutton_event);
			ev->any.type = (tv->type == TELE_INP_BUTTON) ?
				evPtrButtonPress : evPtrButtonRelease;
			
			ev->pbutton.button = d->button;

			return 0;
		}

		case TELE_INP_MOUSE:
		case TELE_INP_TABLET:
		{
			TeleInpAxisData *d = (void *) tv->data;

			ev->any.size = sizeof(gii_pmove_event);
			ev->any.type = (tv->type == TELE_INP_MOUSE) ?
				evPtrRelative : evPtrAbsolute;

			ev->pmove.x     = (d->count >= 1) ? d->axes[0] : 0;
			ev->pmove.y     = (d->count >= 2) ? d->axes[1] : 0;
			ev->pmove.z     = (d->count >= 3) ? d->axes[2] : 0;
			ev->pmove.wheel = (d->count >= 4) ? d->axes[3] : 0;

			return 0;
		}

		case TELE_INP_JOYSTICK:
		{
			TeleInpAxisData *d = (void *) tv->data;
			int i;

			if (d->count > 32) {
				return -1;
			}
			
			ev->any.size = sizeof(gii_val_event);
			ev->any.type = evValAbsolute;
			
			ev->val.first = 0;
			ev->val.count = d->count;

			for (i=0; i < d->count; i++) {
				ev->val.value[i] = d->axes[i];
			}

			return 0;
		}

		case TELE_INP_EXPOSE:
		{
			ev->any.size = sizeof(gii_expose_event);
			ev->any.type = evExpose;
			
			/* the whole screen */

			ev->expose.x = 0;
			ev->expose.y = 0;
			ev->expose.w = priv->width;
			ev->expose.h = priv->height;

			return 0;
		}
	}
	
	GGIDPRINT_MISC("display-tele: unknown input event (0x%08x).\n",
		    tv->type);
	return -1;
}


/* ---------------------------------------------------------------------- */


gii_event_mask GII_tele_poll(gii_input *inp, void *arg)
{
	ggi_tele_priv *priv = (ggi_tele_priv *) inp->priv;

	gii_event_mask evmask = 0;
	gii_event ev;

	TeleEvent th_ev;

	int err;

	GGIDPRINT_EVENTS("display-tele: poll event.\n");

  	if (! priv->connected) {
  		return 0;
  	}

	if (tclient_poll(priv->client)) {
		
		err = tclient_read(priv->client, &th_ev);

		if (err == TELE_ERROR_SHUTDOWN) {
			TELE_HANDLE_SHUTDOWN;

		} else if (err < 0) {
			GGIDPRINT_EVENTS("tclient_read: ZERO\n");
			return 0;
		}
		
		GGIDPRINT_EVENTS("display-tele: got event (type=0x%08x "
			"seq=0x%08x)\n", (int) th_ev.type,
			(int) th_ev.sequence);

		if ((th_ev.type & TELE_EVENT_TYPE_MASK) == TELE_CMD_BASE) {
			handle_telecmd_event(priv, &th_ev);

		} else if (translate_to_ggi(inp, &ev, &th_ev) == 0) {
			evmask |= (1 << ev.any.type);
			_giiEvQueueAdd(inp, &ev);
		}
	}

	return evmask;
}


/* ---------------------------------------------------------------------- */


int tele_receive_reply(ggi_visual *vis, TeleEvent *ev, 
			   long type, long seq)
{
	ggi_tele_priv *priv = TELE_PRIV(vis);

	ev->size = 0;

	priv->wait_event = ev;
	priv->wait_type  = type;
	priv->wait_sequence = seq;

	GGIDPRINT_EVENTS("display-tele: WAITING FOR (type=0x%08lx "
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

	GGIDPRINT_EVENTS("display-tele: WAIT OVER (type=0x%08lx "
	              "seq=0x%08lx)\n", type, seq);

	priv->wait_event = NULL;

	return 0;
}
