/* $Id: TIvisual.h,v 1.7 2006/03/20 20:44:53 cegger Exp $
 *
 * Copyright 1998 MenTaLguY - mentalg@geocities.com
 *
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
 */

#include "config.h"
#include <ggi/internal/ggi-dl.h>

#ifdef __GNUC__
# undef alloca
# define alloca __builtin_alloca
#else
# if HAVE_ALLOCA_H
#  include <alloca.h>
# else
#  ifdef _AIX
#pragma alloca
#  else
#   ifndef alloca /* predefined by HP cc +Olibcalls */
char *alloca ();
#   endif
#  endif
# endif
#endif

#ifdef HAVE_NCURSES_H
#include <ncurses.h>
#elif defined(HAVE_NCURSES_NCURSES_H)
#include <ncurses/ncurses.h>
#else
#include <curses.h>
#endif

struct TIhooks {
	SCREEN *scr;
	FILE *f_in, *f_out;
	int f_private;
	struct { int x, y; } origin;
	int splitline;
	int virgin;
	chtype color16_table[256];
	chtype charmap[256];
	struct ggi_visual *vis;
	int physzflags;
	ggi_coord physz;
};

#define TERMINFO_PRIV(vis)	((struct TIhooks *)LIBGGI_PRIVATE(vis))

/* Prototypes
 */

void _terminfo_init_ncurses(void);
void _terminfo_finalize_ncurses(void);

SCREEN *_terminfo_new_screen(const char *term_type, FILE *out, FILE *in);
void _terminfo_select_screen(SCREEN *scr);
void _terminfo_release_screen(void);
void _terminfo_destroy_screen(void);

ggifunc_flush		GGI_terminfo_flush;
ggifunc_getmode		GGI_terminfo_getmode;
ggifunc_setmode		GGI_terminfo_setmode;
ggifunc_checkmode	GGI_terminfo_checkmode;
ggifunc_getapi		GGI_terminfo_getapi;
ggifunc_setflags	GGI_terminfo_setflags;

giifunc_eventpoll	GII_terminfo_eventpoll;
giifunc_sendevent	GII_terminfo_sendevent;

extern int paint_ncurses_window(struct ggi_visual *, WINDOW *, int, int);
extern void _GGI_terminfo_freedbs(struct ggi_visual *);
