/* $Id: input.c,v 1.3 2004/10/31 14:25:04 cegger Exp $
******************************************************************************

   Terminfo target

   Copyright (C) 1997 Jason McMullan    [jmcc@ggi-project.org]
   Copyright (C) 1998 MenTaLguY         [mentalg@geocities.com]

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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>

#include "TIvisual.h"
#include <ggi/internal/ggi_debug.h>
#include <ggi/internal/gii.h>
#include <ggi/gii.h>
#include <ggi/keyboard.h>
#include <ggi/events.h>

static int translate_key(int key, uint32 *modifiers)
{
	GGIDPRINT("terminfo: TRANSLATEKEY %04x\n", key);

	*modifiers = 0;

	if ((0x00 <= key) && (key <= 0x7f)) {
		return key;
	}

	if ((KEY_F0+1 <= key) && (key <= KEY_F0+20)) {
		return GII_KEY(GII_KT_FN, key-1);
	}
	
	if ((KEY_F0+21 <= key) && (key <= KEY_F0+63)) {
		return GII_KEY(GII_KT_FN, key+9);
	}
	
	switch (key) {

		case '\015':		return GIIK_Enter;

		case KEY_BREAK:		return GIIK_Break;
		case KEY_DOWN:		return GIIK_Down;
		case KEY_UP:		return GIIK_Up;
		case KEY_LEFT:		return GIIK_Left;
		case KEY_RIGHT:		return GIIK_Right;
		case KEY_BACKSPACE:	return 8;

		case KEY_DC:		return GIIK_Clear;
		case KEY_IC:		return GIIK_Insert;
		case KEY_EIC:		return GIIK_Insert;  /* ??? */
		case KEY_SF:		return GIIK_ScrollForw;
		case KEY_SR:		return GIIK_ScrollBack;
		case KEY_NPAGE:		return GIIK_PageDown;
		case KEY_PPAGE:		return GIIK_PageUp;
		case KEY_ENTER:		return GIIK_Enter;
		case KEY_SRESET:	return GIIK_SAK;   /* ??? */
		case KEY_RESET:		return GIIK_Boot;  /* ??? */
		case KEY_A1:		return GIIK_Home;
		case KEY_A3:		return GIIK_PageUp;
		case KEY_C1:		return GIIK_End;
		case KEY_C3:		return GIIK_PageDown;

		case KEY_END:		return GIIK_End;
		case KEY_FIND:		return GIIK_Find;
		case KEY_HELP:		return GIIK_Help;
		case KEY_UNDO:		return GIIK_Undo;
		case KEY_NEXT:		return GIIK_Next;
		case KEY_PREVIOUS:	return GIIK_Prior;
		case KEY_SELECT:	return GIIK_Select;
		case KEY_SUSPEND:	return GIIK_Pause;  /* ??? */

		/*** NOT DONE :

		case KEY_HOME           Home key (upward+left arrow)
		case KEY_DL             Delete line
		case KEY_IL             Insert line
		case KEY_CLEAR:		Clear
		case KEY_EOS:		Clear to end of screen
		case KEY_EOL:		Clear to end of line
		case KEY_STAB:		Set tab
		case KEY_CTAB:		Clear tab
		case KEY_CATAB:		Clear all tabs
		case KEY_PRINT:		Print or copy
		case KEY_LL:		Home down or bottom (lower left).
		case KEY_B2:		Centre of keypad

		case KEY_BTAB:		Back tab key
		case KEY_BEG:		Beg(inning) key
		case KEY_CANCEL		Cancel key
		case KEY_CLOSE:		Close key
		case KEY_COMMAND:	Cmd (command) key
		case KEY_COPY:		Copy key
		case KEY_CREATE:	Create key
		case KEY_EXIT:		Exit key
		case KEY_MARK:		Mark key
		case KEY_MESSAGE:	Message key
		case KEY_MOVE:		Move key
		case KEY_OPEN:		Open key
		case KEY_OPTIONS:	Options key
		case KEY_REDO:		Redo key
		case KEY_REFERENCE:	Ref(erence) key
		case KEY_REFRESH:	Refresh key
		case KEY_REPLACE:	Replace key
		case KEY_RESTART:	Restart key
		case KEY_RESUME:	Resume key
		case KEY_SAVE:		Save key

		case KEY_SBEG:		Shifted beginning key
		case KEY_SCANCEL:	Shifted cancel key
		case KEY_SCOMMAND:	Shifted command key
		case KEY_SCOPY:		Shifted copy key
		case KEY_SCREATE:	Shifted create key
		case KEY_SDC:		Shifted delete char key
		case KEY_SDL:		Shifted delete line key
		case KEY_SEND:		Shifted end key
		case KEY_SEOL:		Shifted clear line key
		case KEY_SEXIT:		Shifted exit key
		case KEY_SFIND:		Shifted find key
		case KEY_SHELP:		Shifted help key
		case KEY_SHOME:		Shifted home key
		case KEY_SIC:		Shifted input key
		case KEY_SLEFT:		Shifted left arrow key
		case KEY_SMESSAGE:	Shifted message key
		case KEY_SMOVE:		Shifted move key
		case KEY_SNEXT:		Shifted next key
		case KEY_SOPTIONS:	Shifted options key
		case KEY_SPREVIOUS:	Shifted prev key
		case KEY_SPRINT:	Shifted print key
		case KEY_SREDO:		Shifted redo key
		case KEY_SREPLACE:	Shifted replace key
		case KEY_SRIGHT:	Shifted right arrow
		case KEY_SRSUME:	Shifted resume key
		case KEY_SSAVE:		Shifted save key
		case KEY_SSUSPEND:	Shifted suspend key
		case KEY_SUNDO:		Shifted undo key

		***/
	}

	return GIIK_VOID;   /* unknown */
}

gii_event_mask GII_terminfo_eventpoll(gii_input *inp, void *arg)
{
	ggi_event ev;
	gii_event_mask mask;
	int key;
	struct TIhooks *tiinfo;

	tiinfo = (struct TIhooks *)inp->priv;

	mask = 0;

	_terminfo_select_screen(tiinfo->scr);

	key = wgetch(stdscr);
	switch (key) {
		case ERR: break;
#if ( NCURSES_MOUSE_VERSION == 1 )		
		case KEY_MOUSE: {
			MEVENT mev;

			getmouse(&mev);
			_giiEventBlank(&ev, sizeof(gii_event));

			switch (mev.bstate) {
				case BUTTON1_PRESSED:
				case BUTTON2_PRESSED:
				case BUTTON3_PRESSED:
				case BUTTON4_PRESSED: {
					ev.any.type = evPtrButtonPress;
					ev.any.size = sizeof(gii_pbutton_event);
					mask |= emPtrButtonPress;
					switch (mev.bstate) {
					case BUTTON1_PRESSED:
						ev.pbutton.button = 1;
						break;
					case BUTTON2_PRESSED:
						ev.pbutton.button = 3;
						break;
					case BUTTON3_PRESSED:
						ev.pbutton.button = 2;
						break;
					case BUTTON4_PRESSED:
						ev.pbutton.button = 4;
						break;
					}
				} break;
				case BUTTON1_RELEASED:
				case BUTTON2_RELEASED:
				case BUTTON3_RELEASED:
				case BUTTON4_RELEASED: {
					ev.any.type = evPtrButtonRelease;
					ev.any.size = sizeof(gii_pbutton_event);
					mask |= emPtrButtonRelease;
					switch (mev.bstate) {
					case BUTTON1_PRESSED:
						ev.pbutton.button = 1;
						break;
					case BUTTON2_PRESSED:
						ev.pbutton.button = 3;
						break;
					case BUTTON3_PRESSED:
						ev.pbutton.button = 2;
						break;
					case BUTTON4_PRESSED:
						ev.pbutton.button = 4;
						break;
					}
				} break;
				default: {
					ggi_mode *mode;

					mode = LIBGGI_MODE(tiinfo->vis);

					ev.any.type = evPtrAbsolute;
					ev.any.size = sizeof(gii_pmove_event);
					ev.pmove.x = mev.x * mode->dpp.x;
					ev.pmove.y = mev.y * mode->dpp.y;
					mask |= emPtrAbsolute;
				}
			}
			_giiEvQueueAdd(inp, &ev);
		} break;
#endif /* NCURSES_MOUSE_VERSION */
		case '\033': {
			int temp;
			timeout(1);
			temp = wgetch(stdscr);
			timeout(0);
			if ( temp != ERR ) {
				_giiEventBlank(&ev, sizeof(ggi_key_event));
				ev.any.size = sizeof(ggi_key_event);
				ev.any.type = evKeyPress;
				ev.key.modifiers = GII_MOD_ALT;
				ev.key.sym = translate_key(key, &ev.key.modifiers);
				ev.key.label = ev.key.sym;  /* FIXME !!! */
				ev.key.button = temp;
				_giiEvQueueAdd(inp, &ev);

				_giiEventBlank(&ev, sizeof(ggi_key_event));
				ev.any.size = sizeof(ggi_key_event);
				ev.any.type = evKeyRelease;
				ev.key.modifiers = GII_MOD_ALT;
				ev.key.sym = translate_key(key, &ev.key.modifiers);
				ev.key.label = ev.key.sym;  /* FIXME !!! */
				ev.key.button = temp;
				_giiEvQueueAdd(inp, &ev);

				mask |= emKeyPress | emKeyRelease;

				break;
			} /* else, normal key, and default... */
		} 
		default: {
			_giiEventBlank(&ev, sizeof(ggi_key_event));
			ev.any.size = sizeof(ggi_key_event);
			ev.any.type = evKeyPress;
			ev.key.modifiers = 0;
			ev.key.sym = translate_key(key, &ev.key.modifiers);
			ev.key.label = ev.key.sym;  /* FIXME !!! */
			ev.key.button = key;
			_giiEvQueueAdd(inp, &ev);

			_giiEventBlank(&ev, sizeof(ggi_key_event));
			ev.any.size = sizeof(ggi_key_event);
			ev.any.type = evKeyRelease;
			ev.key.modifiers = 0;
			ev.key.sym = translate_key(key, &ev.key.modifiers);
			ev.key.label = ev.key.sym;  /* FIXME !!! */
			ev.key.button = key;

			mask |= emKeyPress | emKeyRelease;

			_giiEvQueueAdd(inp, &ev);
		}
	}
	_terminfo_release_screen();
	
	return mask;
}

static gii_cmddata_getdevinfo terminfo_devinfo = {
	"Terminfo", /* long name */
	"tinfo", /* short name */
#if ( NCURSES_MOUSE_VERSION == 1 )
	emKey | emPtrButton | emPtrAbsolute,
	4, /* max 4 buttons */
	2  /* axes */
#else
	emKey, /* without mouse support */
	0,
	0
#endif
};

static int GII_terminfo_senddevinfo(gii_input *inp) {
	gii_event ev;
	
	_giiEventBlank(&ev, sizeof(gii_cmd_nodata_event)
		       + sizeof(gii_cmddata_getdevinfo));
	ev.any.size = sizeof(gii_cmd_nodata_event)
	              + sizeof(gii_cmddata_getdevinfo);
	ev.any.type = evCommand;
	ev.any.origin = inp->origin;
	ev.cmd.code = GII_CMDCODE_GETDEVINFO;

	memcpy((void *)ev.cmd.data, (void *)&terminfo_devinfo,
	       sizeof(gii_cmddata_getdevinfo));

	return _giiEvQueueAdd(inp, &ev);
}

int GII_terminfo_sendevent(gii_input *inp, gii_event *ev) {
	if ( ( ev->any.target != inp->origin )
	     && ( ev->any.target != GII_EV_TARGET_ALL ) )
	{
		return -1; /* not intended for us */
	}

	if ( ev->any.type != evCommand ) {
		return -1; /* not interested */
	}

	switch (ev->cmd.code) {
		case GII_CMDCODE_GETDEVINFO:
			return GII_terminfo_senddevinfo(inp);
			break;
		default:
			return -1;
	}
}
