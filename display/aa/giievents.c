/* $Id: giievents.c,v 1.4 2004/11/06 12:49:53 cegger Exp $
******************************************************************************

   Graphics library for GGI.  Events for AA target.

   Copyright (C) 1997 Andreas Beck	[becka@ggi-project.org]
   Copyright (C) 1998 Steve Cheng	[steve@ggi-project.org]
   Copyright (C) 1999 Marcus Sundberg	[marcus@ggi-project.org]

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

#include "config.h"
#include <ggi/display/aa.h>
#include <ggi/internal/ggi_debug.h>


static void add_key_event(gii_input_t inp, unsigned int key, 
			ggi_event_mask type)
{
	ggi_event ggievent;

	_giiEventBlank(&ggievent, sizeof(ggi_key_event));

	if (type == evKeyRelease) key &= ~AA_RELEASE;
	
	switch (key) {
		case AA_UP: 		key = GIIK_Up; break;
		case AA_DOWN:		key = GIIK_Down; break;
		case AA_LEFT:		key = GIIK_Left; break;
		case AA_RIGHT:		key = GIIK_Right; break;
		case AA_BACKSPACE:	key = GIIUC_BackSpace; break;
		case AA_ESC:		key = GIIUC_Escape; break;
		/*default:		key = LATIN_KEY(key);*/
	}

	ggievent.any.size   = sizeof(ggi_key_event);
	ggievent.any.type   = type;
	ggievent.any.origin = inp->origin;

	ggievent.key.sym = ggievent.key.label = ggievent.key.button = key;
	
	_giiEvQueueAdd(inp, &ggievent);
}

static inline ggi_event_mask
do_mouse(gii_input_t inp, ggi_aa_priv *priv)
{
	ggi_event_mask ret = 0;
	ggi_event ggievent;
	int x, y, buttons;

	aa_getmouse(priv->context, &x, &y, &buttons);

	/* Mouse movement is in screen coordinates;
	   we assume one screen pixel : four image pixels */
	x *= AA_SCRMULT_X;
	y *= AA_SCRMULT_Y;

	if (x != priv->lx || y != priv->ly) {
		_giiEventBlank(&ggievent, sizeof(ggi_pmove_event));
		ggievent.any.size   = sizeof(ggi_pmove_event);
		ggievent.any.type   = evPtrAbsolute;
		ggievent.any.origin = inp->origin;

		ggievent.pmove.x = x;
		ggievent.pmove.y = y;
		
		ret |= emPtrAbsolute;
		_giiEvQueueAdd(inp, &ggievent);

		priv->lx = x;
		priv->ly = y;
	}

	/* Now check the buttons */
	if (buttons != priv->lb) {
		int change = buttons ^ priv->lb;
		int i;

		for (i = 0; i < 3; i++) {
			int diff = change & (1<<i);
			if (! diff) continue;
			
			_giiEventBlank(&ggievent, sizeof(ggi_pbutton_event));
			ggievent.any.size = sizeof(ggi_pbutton_event);
			ggievent.any.origin = inp->origin;
			if ((diff & buttons)) {
				ggievent.any.type = evPtrButtonPress;
				ret |= emPtrButtonPress;
			} else {
				ggievent.any.type = evPtrButtonRelease;
				ret |= emPtrButtonRelease;
			}
			ggievent.pbutton.button = i+1;

			_giiEvQueueAdd(inp, &ggievent);
		}
		priv->lb = buttons;
	}
	
	return ret;
}	


ggi_event_mask GII_aa_poll(gii_input_t inp, void *arg)
{

	ggi_aa_priv *priv = inp->priv;
	ggi_event_mask evmask = 0;
	unsigned int aatype;

	GGIDPRINT_EVENTS("GII_aa_poll\n");

	if (!priv->context) return 0;

	while ((aatype = aa_getevent(priv->context, 0)) != AA_NONE) {
		GGIDPRINT_EVENTS("AA: got event %x\n", aatype);

		if (aatype == AA_MOUSE)	{
			evmask |= do_mouse(inp, priv);
		} else if (aatype >= 1 && aatype <= AA_RELEASE) {
			if (priv->lastkey == 0) {
				/* First hit */
				evmask |= emKeyPress;
				add_key_event(inp, aatype, evKeyPress);
			} else if (priv->lastkey == aatype) {
				/* Repeated keypress */
				evmask |= emKeyRepeat;
				add_key_event(inp, aatype, evKeyRepeat);
			} else {
				if (!priv->haverelease) {
					/* Whoops, different key! We send a
					   release for the lastkey first. */
					evmask |= emKeyRelease;
					add_key_event(inp, priv->lastkey,
						      evKeyRelease);
				}
				evmask |= emKeyPress;
				add_key_event(inp, aatype, evKeyPress);
			}
			priv->lastkey = aatype;
		} else if (aatype > AA_RELEASE) {
			/* Release given key.  It should match lastkey, but
			   if it doesn't, tough luck, we clear it anyway. */
			evmask |= emKeyRelease;
			add_key_event(inp, aatype, evKeyRelease);
			priv->lastkey = 0;
			priv->haverelease = 1;
		} else if (aatype == AA_RESIZE || aatype == AA_UNKNOWN) {
		}

	}
	if (!priv->haverelease && priv->lastkey) {
		/* No more events.  If priv->lastkey != 0, we release that key. */
		evmask |= emKeyRelease;
		add_key_event(inp, priv->lastkey, evKeyRelease);
		priv->lastkey = 0;
	}

	return evmask;
}
