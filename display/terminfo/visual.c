/* $Id: visual.c,v 1.22 2007/03/08 20:54:08 soyt Exp $
******************************************************************************

   Terminfo target

   Copyright (C) 1998 MenTaLguY		[mentalg@geocities.com]
   Copyright (C) 2000 Marcus Sundberg	[marcus@ggi-project.org]

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

#include "config.h"
#include "TIvisual.h"
#include <ggi/gii.h>
#include <ggi/gii-module.h>
#include <ggi/input/terminfo.h>
#include <ggi/internal/ggi_debug.h>


static const gg_option optlist[] =
{
	{ ":path",	"" },
	{ ":term",	"" },
	{ "physz",	"0,0" },
};

#define OPT_PATH	0
#define OPT_TERM	1
#define OPT_PHYSZ	2
#define NUM_OPTS        (sizeof(optlist)/sizeof(gg_option))

void _GGI_terminfo_freedbs(struct ggi_visual *vis)
{
	int i;

	for (i=LIBGGI_APPLIST(vis)->num-1; i >= 0; i--) {
		free(LIBGGI_APPBUFS(vis)[i]->write);
		_ggi_db_free(LIBGGI_APPBUFS(vis)[i]);
		_ggi_db_del_buffer(LIBGGI_APPLIST(vis), i);
	}
}

/* Constructs an approximate mapping from IBM-ASCII to the charset of the
   current display */
static void construct_charmap(chtype charmap[256])
{
	int i;

	/* low ascii ( control characters ) */
	for ( i = 0 ; i < 32 ; i++ ) charmap[i] = '*';
	charmap[4] = ACS_DIAMOND;
	charmap[7] = ACS_BULLET;
	charmap[8] = ACS_BULLET | A_REVERSE;
	charmap[9] = 'o';
	charmap[10] = 'o' | A_REVERSE;
	charmap[16] = '>';
	charmap[17] = '<';
	charmap[24] = ACS_UARROW;
	charmap[25] = ACS_DARROW;
	charmap[26] = ACS_RARROW;
	charmap[27] = ACS_DARROW;
	charmap[30] = '^';
	charmap[31] = 'v';

	for ( ; i < 127 ; i++ ) charmap[i] = i;
	charmap[127] = '^';
	for ( ; i < 256 ; i++ ) charmap[i] = '*';

	/* "international" characters */
	charmap[128] = 'C'; /* capital cedila */
	charmap[129] = 'u'; /* u + umlaut */
	charmap[130] = 'e';
	charmap[131] = charmap[132] = charmap[133] = charmap[134] = 'a';
	charmap[135] = 'c'; /* lower-case cedila */
	charmap[136] = charmap[137] = charmap[138] = 'e';
	charmap[139] = charmap[140] = charmap[141] = 'i';
	charmap[142] = charmap[143] = 'A';
	charmap[144] = 'E';
	charmap[145] = 'e'; /* ae digraph */
	charmap[146] = 'E'; /* AE digraph */
	charmap[147] = charmap[148] = charmap[149] = 'o';
	charmap[150] = charmap[151] = 'u';
	charmap[152] = 'y';
	charmap[153] = 'O';
	charmap[154] = 'U'; /* U + umlaut */
	charmap[160] = 'a';
	charmap[161] = 'i';
	charmap[162] = 'o';
	charmap[163] = 'u';
	charmap[164] = 'n'; /* n + ~ */
	charmap[165] = 'N'; /* N + ~ */
	charmap[168] = '?'; /* spanish inverted question mark */
	charmap[169] = '!'; /* spanish inverted exclamation mark */

	/* line drawing characters */
	charmap[179] = ACS_VLINE;
	charmap[180] = charmap[181] = charmap[182] = ACS_RTEE;
	charmap[183] = charmap[184] = ACS_LLCORNER;
	charmap[185] = ACS_RTEE;
	charmap[186] = ACS_VLINE;
	charmap[187] = ACS_LLCORNER;
	charmap[188] = charmap[189] = charmap[190] = ACS_ULCORNER;
	charmap[191] = ACS_LLCORNER;
	charmap[192] = ACS_URCORNER;
	charmap[193] = ACS_BTEE;
	charmap[194] = ACS_TTEE;
	charmap[195] = ACS_LTEE;
	charmap[196] = ACS_HLINE;
	charmap[197] = ACS_PLUS;
	charmap[198] = charmap[199] = ACS_LTEE;
	charmap[200] = ACS_URCORNER;
	charmap[201] = ACS_LRCORNER;
	charmap[202] = ACS_BTEE;
	charmap[203] = ACS_TTEE;
	charmap[204] = ACS_LTEE;
	charmap[205] = ACS_HLINE;
	charmap[206] = ACS_PLUS;
	charmap[207] = charmap[208] = ACS_BTEE;
	charmap[209] = charmap[210] = ACS_TTEE;
	charmap[211] = charmap[212] = ACS_URCORNER;
	charmap[213] = charmap[214] = ACS_LRCORNER;
	charmap[215] = charmap[216] = ACS_PLUS;
	charmap[217] = ACS_ULCORNER;
	charmap[218] = ACS_LRCORNER;

	charmap[218] = ACS_ULCORNER;
	charmap[192] = ACS_LLCORNER;
	charmap[191] = ACS_URCORNER;
	charmap[217] = ACS_LRCORNER;
	charmap[180] = ACS_RTEE;
	charmap[195] = ACS_LTEE;
	charmap[193] = ACS_BTEE;
	charmap[194] = ACS_TTEE;
	charmap[196] = ACS_HLINE;
	charmap[179] = ACS_VLINE;
	charmap[197] = ACS_PLUS;

	/* block drawing characters */
	charmap[176] = ACS_CKBOARD;
	charmap[177] = ACS_CKBOARD;
	charmap[178] = ACS_CKBOARD;
	charmap[219] = ACS_BLOCK;

	/* miscellaneous characters */
	charmap[155] = 'c'; /* cents */
#ifdef ACS_STERLING
	charmap[156] = ACS_STERLING; /* pounds sterling symbol */
#endif
	charmap[157] = 'Y'; /* yen symbol */
	charmap[158] = 'P'; /* Pt symbol */
	charmap[159] = 'f'; /* cursive 'f' */
	charmap[174] = '<'; /* << */
	charmap[175] = '>'; /* >> */
#ifdef ACS_PI
	charmap[227] = ACS_PI;
#endif /* ACS_PI */
	charmap[232] = ACS_LANTERN; /* iota */
#ifdef ACS_LEQUAL
	charmap[243] = ACS_LEQUAL;
#endif
#ifdef ACS_GEQUAL
	charmap[242] = ACS_GEQUAL;
#endif
	charmap[248] = ACS_DEGREE;
	charmap[241] = ACS_PLMINUS;
	charmap[249] = charmap[250] = ACS_BULLET;
	charmap[251] = 'J'; /* radical sign */
	charmap[253] = '2'; /* superscript 2 */
	/* what, no euro symbol? :) */
}

static int GGIopen(struct ggi_visual *vis, struct ggi_dlhandle *dlh,
		   const char *args, void *argptr, uint32_t *dlret)
{
	struct TIhooks *priv;
	gg_option options[NUM_OPTS];
	char *term_type;
	char *term_path;
	int i, err;

        memcpy(options, optlist, sizeof(options));
	if (args != NULL) {
		args = ggParseOptions(args, options, NUM_OPTS);
		if (args == NULL) {
			fprintf(stderr, "display-terminfo: error in arguments.\n");
			return GGI_EARGINVAL;
		}
        }
	term_path = options[OPT_PATH].result;
	term_type = options[OPT_TERM].result;
	if ((*term_type) == '\0') term_type = NULL;

	DPRINT("display-terminfo: initializing %s on %s.\n", term_type, ( ( *term_path == '\0' ) ? "stdin/stdout" : term_path ));

	priv = (struct TIhooks *)malloc(sizeof(struct TIhooks));
	if (priv == NULL) return GGI_ENOMEM;
	LIBGGI_PRIVATE(vis) = priv;

	err = _ggi_physz_parse_option(options[OPT_PHYSZ].result, 
			       &(priv->physzflags), &(priv->physz)); 
	if (err != GGI_OK) {
		free(priv);
		return err;
	}

	LIBGGI_GC(vis) = malloc(sizeof(ggi_gc));
	if (LIBGGI_GC(vis) == NULL) {
		err = GGI_ENOMEM;
		free(priv);
		goto err1;
	}

	priv->splitline = 0;

	priv->virgin = 1;

	if ( *term_path == '\0' ) {
		priv->f_in = fdopen(dup(fileno(stdin)), "r");
		priv->f_out = fdopen(dup(fileno(stdout)), "w");
	} else {
		priv->f_in = priv->f_out = fopen(term_path, "rw");
	}

	_terminfo_init_ncurses();

	priv->scr = _terminfo_new_screen(term_type, priv->f_out, priv->f_in);
	if (priv->scr == NULL) {
		fprintf(stderr, "display-terminfo: error creating ncurses"
				" SCREEN\n");
		err = GGI_ENODEVICE;
		goto err3;
	}

	LIBGGI_FD(vis) = fileno(priv->f_out);

	if ( has_colors() ) {
		static const int vga_color[8] = {
			COLOR_BLACK,
			COLOR_BLUE,
			COLOR_GREEN,
			COLOR_CYAN,
			COLOR_RED,
			COLOR_MAGENTA,
			COLOR_YELLOW,
			COLOR_WHITE
		};
		int j;
		DPRINT("display-terminfo: terminal supports %d colors\n", COLORS);
		DPRINT("display-terminfo: initializing %d - 1 color pairs\n", COLOR_PAIRS);
		for ( i = 1 ; i < COLOR_PAIRS ; i++ ) {
			if ( init_pair(i, COLORS - ( i % COLORS ) - 1, i / COLORS) == ERR ) {
				DPRINT("display-terminfo: error initializing color pair %d to %d,%d\n", i, COLORS - ( i % COLORS ) - 1, i / COLORS);
				fprintf(stderr, "display-terminfo: error initializing colors\n");
				break;
			}
		}
		for ( i = 0 ; i < 16 ; i++ ) {
			for ( j = 0 ; j < 16 ; j++ ) {
				priv->color16_table[i+(j<<4)] =
					COLOR_PAIR(((COLORS-vga_color[i&0x07]%COLORS-1)
					 +(vga_color[j&0x07]%COLORS*COLORS))%COLOR_PAIRS)
					| ( ( i > 7 ) ? A_BOLD : A_NORMAL )
					| ( ( j > 7 ) ? A_BLINK : A_NORMAL );
			}
		}

	} else {
		DPRINT("display-terminfo: terminal lacks color support\n");
	}
	construct_charmap(priv->charmap);
#if ( NCURSES_MOUSE_VERSION == 1 ) 
	DPRINT("display-terminfo: mouse support is enabled\n");
	mousemask(REPORT_MOUSE_POSITION | BUTTON1_PRESSED | BUTTON1_RELEASED |
			BUTTON2_PRESSED | BUTTON2_RELEASED | BUTTON3_PRESSED |
			BUTTON3_RELEASED | BUTTON4_PRESSED | BUTTON4_RELEASED,
			NULL);
#else
	DPRINT("display-terminfo: mouse support is disabled\n");
#endif

	/* mode management */
	vis->opdisplay->flush     = GGI_terminfo_flush;
	vis->opdisplay->getmode   = GGI_terminfo_getmode;
	vis->opdisplay->setmode   = GGI_terminfo_setmode;
	vis->opdisplay->checkmode = GGI_terminfo_checkmode;
	vis->opdisplay->getapi    = GGI_terminfo_getapi;
	vis->opdisplay->setflags  = GGI_terminfo_setflags;

	/* event management */
	do {
		struct gg_module *inp = NULL;
		struct gg_api *api;
		gii_terminfo_arg _args;

		_args.scr = priv->scr;
		_args.select_screen = _terminfo_select_screen;
		_args.release_screen = _terminfo_release_screen;

		api = ggGetAPIByName("gii");
		if ((api != NULL) && (STEM_HAS_API(vis->module.stem, api))) {
			inp = ggOpenModule(api, vis->module.stem,
					"input-terminfo", NULL, &_args);
		}			

		DPRINT_MISC("ggOpenModule() returned with %p\n", inp);
		if (inp == NULL) {
			fprintf(stderr, "display-terminfo: error allocating gii_input\n");
			err = GGI_ENOMEM;
			goto err4;
		}

		priv->inp = inp;
	} while(0);

	_terminfo_release_screen();
 
	*dlret = GGI_DL_OPDISPLAY;
	return 0;

err4:
	_terminfo_destroy_screen();
err3:
	fclose(priv->f_in);
	fclose(priv->f_out);
	free(LIBGGI_GC(vis));
err1:
	free(priv);
	return err;
}

static int GGIclose(struct ggi_visual *vis, struct ggi_dlhandle *dlh)
{
	struct TIhooks *priv;

	priv = TERMINFO_PRIV(vis);
	if (priv != NULL) {
		if (priv->scr != NULL) {
			ggCloseModule(priv->inp);
			_terminfo_select_screen(priv->scr);
			if (!priv->virgin) {
				wclear(stdscr);
				refresh();
			}
			_terminfo_destroy_screen();
		}
		if (priv->f_in != NULL) {
			fclose(priv->f_in);
		}
		if ((priv->f_out != NULL) && (priv->f_out != priv->f_in)) {
			fclose(priv->f_out);
		}

		_GGI_terminfo_freedbs(vis);

		free(priv);
	}	

	free(LIBGGI_GC(vis));

	_terminfo_finalize_ncurses();

	return 0;
}


EXPORTFUNC
int GGIdl_terminfo(int func, void **funcptr);

int GGIdl_terminfo(int func, void **funcptr)
{
	ggifunc_open **openptr;
	ggifunc_close **closeptr;

	switch (func) {
	case GGIFUNC_open:
		openptr = (ggifunc_open **)funcptr;
		*openptr = GGIopen;
		return 0;
	case GGIFUNC_exit:
		*funcptr = NULL;
		return 0;
	case GGIFUNC_close:
		closeptr = (ggifunc_close **)funcptr;
		*closeptr = GGIclose;
		return 0;
	default:
		*funcptr = NULL;
	}

	return GGI_ENOTFOUND;
}
