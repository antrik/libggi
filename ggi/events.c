/* $Id: events.c,v 1.1 2001/05/12 23:03:16 cegger Exp $
******************************************************************************

   Graphics library for GGI. Events handling.

   Copyright (C) 1998 Andreas Beck      [becka@ggi-project.org]

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

#include <ggi/internal/internal.h>

gii_event_mask ggiEventPoll(ggi_visual *vis, gii_event_mask mask,
			    struct timeval *t)
{
	return giiEventPoll(vis->input, mask, t);
}

int ggiEventsQueued(ggi_visual *vis, gii_event_mask mask)
{
	return giiEventsQueued(vis->input, mask);
}

int ggiEventRead(ggi_visual *vis, gii_event *ev, gii_event_mask mask)
{
	return giiEventRead(vis->input, ev, mask);
}

int ggiSetEventMask(ggi_visual *vis, gii_event_mask evm)
{
	return giiSetEventMask(vis->input, evm);
}

gii_event_mask ggiGetEventMask(ggi_visual *vis)
{
	return giiGetEventMask(vis->input);
}

int ggiEventSend(ggi_visual *vis, gii_event *ev)
{
	if (ev->any.type == evCommand &&
	    (ev->cmd.code & (GII_CMDFLAG_EXTERNAL | GGI_CMDFLAG_LIBGGI))
	    == (GII_CMDFLAG_EXTERNAL | GGI_CMDFLAG_LIBGGI)) {
		return vis->opdisplay->sendevent(vis, ev);
	}
	return giiEventSend(vis->input, ev);
}


/* This can also be used to query the vis->input member.
 */
gii_input_t ggiJoinInputs(ggi_visual *vis, gii_input_t inp)
{
	if (vis->input == NULL) {
		return (vis->input = inp);
	}
	return (vis->input = giiJoinInputs(vis->input, inp));
}

int ggiKbhit(ggi_visual_t vis)
{
	struct timeval t={0,0};

	return (giiEventPoll(vis->input, emKeyPress | emKeyRepeat, &t)
		!= emZero);
}

int ggiGetc(ggi_visual_t vis)
{
	gii_event ev;

	/* Block until we get a key. */
	giiEventRead(vis->input, &ev, emKeyPress | emKeyRepeat);

	return ev.key.sym;
}
