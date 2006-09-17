/* $Id: giievents.c,v 1.6 2006/09/17 14:08:53 aldot Exp $
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
#include <ggi/internal/ggi_debug.h>
#include <ggi/gii.h>
#include <ggi/gii-module.h>
#include <ggi/gii-keyboard.h>
#include <ggi/display/aa.h>


static void add_key_event(struct gii_source *src, unsigned int key, 
			gii_event_mask type)
{
	gii_event giiev;

	giiEventBlank(&giiev, sizeof(gii_key_event));

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

	giiev.any.size   = sizeof(gii_key_event);
	giiev.any.type   = type;
	giiev.any.origin = src->origin;

	giiev.key.sym = giiev.key.label = giiev.key.button = key;
	
	giiPostEvent(src, &giiev);
}

static inline gii_event_mask
do_mouse(struct gii_source *src, ggi_aa_priv *priv)
{
	gii_event_mask ret = 0;
	gii_event giiev;
	int x, y, buttons;

	aa_getmouse(priv->context, &x, &y, &buttons);

	/* Mouse movement is in screen coordinates;
	   we assume one screen pixel : four image pixels */
	x *= AA_SCRMULT_X;
	y *= AA_SCRMULT_Y;

	if (x != priv->lx || y != priv->ly) {
		giiEventBlank(&giiev, sizeof(gii_pmove_event));
		giiev.any.size   = sizeof(gii_pmove_event);
		giiev.any.type   = evPtrAbsolute;
		giiev.any.origin = src->origin;

		giiev.pmove.x = x;
		giiev.pmove.y = y;
		
		ret |= emPtrAbsolute;
		giiPostEvent(src, &giiev);

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
			
			giiEventBlank(&giiev, sizeof(gii_pbutton_event));
			giiev.any.size = sizeof(gii_pbutton_event);
			giiev.any.origin = src->origin;
			if ((diff & buttons)) {
				giiev.any.type = evPtrButtonPress;
				ret |= emPtrButtonPress;
			} else {
				giiev.any.type = evPtrButtonRelease;
				ret |= emPtrButtonRelease;
			}
			giiev.pbutton.button = i+1;

			giiPostEvent(src, &giiev);
		}
		priv->lb = buttons;
	}
	
	return ret;
}	

#if 0
gii_event_mask GII_aa_poll(struct gii_source *src)
{

	ggi_aa_priv *priv = src->priv;
	gii_event_mask evmask = 0;
	unsigned int aatype;

	DPRINT_EVENTS("GII_aa_poll\n");

	if (!priv->context) return 0;

	while ((aatype = aa_getevent(priv->context, 0)) != AA_NONE) {
		DPRINT_EVENTS("AA: got event %x\n", aatype);

		if (aatype == AA_MOUSE)	{
			evmask |= do_mouse(src, priv);
		} else if (aatype >= 1 && aatype <= AA_RELEASE) {
			if (priv->lastkey == 0) {
				/* First hit */
				evmask |= emKeyPress;
				add_key_event(src, aatype, evKeyPress);
			} else if (priv->lastkey == aatype) {
				/* Repeated keypress */
				evmask |= emKeyRepeat;
				add_key_event(src, aatype, evKeyRepeat);
			} else {
				if (!priv->haverelease) {
					/* Whoops, different key! We send a
					   release for the lastkey first. */
					evmask |= emKeyRelease;
					add_key_event(src, priv->lastkey,
						      evKeyRelease);
				}
				evmask |= emKeyPress;
				add_key_event(src, aatype, evKeyPress);
			}
			priv->lastkey = aatype;
		} else if (aatype > AA_RELEASE) {
			/* Release given key.  It should match lastkey, but
			   if it doesn't, tough luck, we clear it anyway. */
			evmask |= emKeyRelease;
			add_key_event(src, aatype, evKeyRelease);
			priv->lastkey = 0;
			priv->haverelease = 1;
		} else if (aatype == AA_RESIZE || aatype == AA_UNKNOWN) {
		}

	}
	if (!priv->haverelease && priv->lastkey) {
		/* No more events.  If priv->lastkey != 0, we release that key. */
		evmask |= emKeyRelease;
		add_key_event(src, priv->lastkey, evKeyRelease);
		priv->lastkey = 0;
	}

	return evmask;
}
#endif
