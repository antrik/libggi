/* $Id: ncurses.c,v 1.1 2001/05/12 23:02:32 cegger Exp $
******************************************************************************

   Terminfo target - miscellaneous ncurses stuff

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

#include <ggi/internal/ggi-dl.h>
#include "TIvisual.h"

static void *ncurses_lock;
static SCREEN *ncurses_screen;
static int count=0;

void _terminfo_init_ncurses()
{
	if (!(count++)) { /* FIXME !!! race condition */
		ncurses_lock = ggLockCreate();
		ggLock(ncurses_lock);
		ncurses_screen = NULL;
		ggUnlock(ncurses_lock);
	} else {
		ggLock(ncurses_lock);
	}
}

void _terminfo_finalize_ncurses()
{
	ggLock(ncurses_lock);
	if (!(--count)) {
		ggUnlock(ncurses_lock);
		ggLockDestroy(ncurses_lock);
	} else {
		ggUnlock(ncurses_lock);
	}
}

void _terminfo_select_screen(SCREEN *scr)
{
	ggLock(ncurses_lock);
	if ( ncurses_screen != scr ) {
		set_term(scr);
		ncurses_screen = scr;
	}
}

void _terminfo_release_screen()
{
	ggUnlock(ncurses_lock);
}

SCREEN *_terminfo_new_screen(const char *term_type, FILE *out, FILE *in)
{
	SCREEN *newscr;
	ggLock(ncurses_lock);
        if ( term_type == NULL ) {
                term_type = getenv("TERM");
                if ( term_type == NULL ) {
                        term_type = "vt100";
                }
        }
	{ char *temp;
		temp = (char *)malloc(sizeof(char) * ( strlen(term_type) + 1 ));
		strcpy(temp, term_type);
		newscr = newterm(temp, out, in);
		free(temp);
	}
	if ( newscr == NULL ) {
		ggUnlock(ncurses_lock);
	} else {
		ncurses_screen = newscr;
		set_term(newscr);
		start_color();
		cbreak();
		noecho();
		nonl();
		timeout(0);
		meta(stdscr, TRUE);
		keypad(stdscr, TRUE);
	}
	return newscr;
}

void _terminfo_destroy_screen()
{
	endwin();
	delscreen(ncurses_screen);
	ncurses_screen = NULL;
	ggUnlock(ncurses_lock);
}
