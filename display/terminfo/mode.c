/* $Id: mode.c,v 1.23 2008/01/21 22:56:48 cegger Exp $
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

#include "config.h"
#include "TIvisual.h"
#include <ggi/internal/ggi_debug.h>

#include "../common/pixfmt-setup.inc"

static int GGI_terminfo_setorigin(struct ggi_visual *vis, int x, int y)
{
	ggi_mode *mode = LIBGGI_MODE(vis);

	x /= mode->dpp.x; y /= mode->dpp.y; /* terminfo can only set origin
                                               with pixel granularity, so
                                               internally, terminfo setorigin
                                               works in pixels */

	if ( ( x < 0 ) || ( x > ( mode->virt.x - mode->visible.x ) ) )
		return -1;
	if ( ( y < 0 ) || ( y > ( mode->virt.y - mode->visible.y ) ) )
		return -1;

	vis->origin_x=x;
	vis->origin_y=y;
	
	return 0;
}

#if 0
int GGI_terminfo_setsplitline(struct ggi_visual *vis, int line)
{
	struct TIhooks *priv;
	ggi_mode *mode;

	priv = TERMINFO_PRIV(vis);
	mode = LIBGGI_MODE(vis);
	
	line /= mode->dpp.y; /* terminfo can only set splitline with pixel
                                granularity, so internally, terminfo splitline
                                works in pixels */ 

	if ( ( line < 0 ) || ( line > mode->visible.y ) )
		return -1;

	priv->splitline = line;

	return 0;
}
#endif

int GGI_terminfo_flush(struct ggi_visual *vis, int x, int y, int w, int h,
		       int tryflag)
{
	struct TIhooks *priv;
	ggi_mode *mode;

	priv = TERMINFO_PRIV(vis);
	mode = LIBGGI_MODE(vis);

	_terminfo_select_screen(priv->scr);
	paint_ncurses_window(vis, stdscr, COLS, LINES);
	refresh();
	_terminfo_release_screen();
	
	return 0;
}

int GGI_terminfo_getapi(struct ggi_visual *vis, int num, char *apiname, char *arguments)
{
	*arguments = '\0';
	switch(num) {
		case 0:
			strcpy(apiname, "display-terminfo");
			return 0;
		case 1:
			strcpy(apiname, "generic-stubs");
			return 0;
		case 2:
			switch (LIBGGI_GT(vis)) {
				case GT_TEXT16: strcpy(apiname, "generic-text-16"); break;
				case GT_TEXT32: strcpy(apiname, "generic-text-32"); break;
				default: return GGI_ENOMATCH;
			}

			return 0;
	}

	return GGI_ENOMATCH;
}

static int _GGI_terminfo_loadstubs(struct ggi_visual *vis)
{
	int i, err;
	char sugname[GGI_MAX_APILEN], args[GGI_MAX_APILEN];

	for (i = 1; GGI_terminfo_getapi(vis, i, sugname, args)==0; i++) {
		err = _ggiOpenDL(vis, libggi->config, sugname, args, NULL);
		if (err) {
			fprintf(stderr, "display-terminfo: Unable to load an "
					"appropriate library for %s (%s)\n", sugname,
					args);
			return GGI_EFATAL;
		} else {
			DPRINT("display-terminfo: Loaded %s (%s)\n", sugname, args);
		}
	}

	ggiIndicateChange(vis->instance.stem, GGI_CHG_APILIST);

	return 0;
}

static int _GGI_terminfo_domode(struct ggi_visual *vis)
{
	struct TIhooks *priv = TERMINFO_PRIV(vis);

	_ggiZapMode(vis, 0);

	_GGI_terminfo_loadstubs(vis);

	vis->opdraw->setorigin = GGI_terminfo_setorigin;
#if 0
	vis->opdraw->setsplitline = GGI_terminfo_setsplitline;
#endif

#if 0   /* ++Andrew: The generic-text-* libraries supply versions of
	 * these functions which do the job.
	 */
	 
	vis->opcolor->mapcolor = GGI_terminfo_mapcolor;
	vis->opcolor->unmappixel = GGI_terminfo_unmappixel;
#endif

	priv->virgin = 0;
	vis->origin_x = vis->origin_y = 0;
	priv->splitline = LIBGGI_Y(vis);

	_terminfo_select_screen(priv->scr);
	wclear(stdscr);
	refresh();
	_terminfo_release_screen();
	return 0;
} 

int GGI_terminfo_setmode(struct ggi_visual *vis, ggi_mode *tm)
{
	struct TIhooks *priv;
	int status;

	priv = TERMINFO_PRIV(vis);

	DPRINT("display-terminfo: setmode mode %8x %dx%d (%dx%d dots, %dx%d font)\n",
		tm->graphtype,
		tm->visible.x, tm->visible.y,
		tm->visible.x * tm->dpp.x, tm->visible.y * tm->dpp.y,
		tm->dpp.x, tm->dpp.y);

	status = GGI_terminfo_checkmode(vis, tm);
	if ( status ) return status;

	DPRINT("display-terminfo: approved mode %8x %dx%d (%dx%d dots, %dx%d font)\n",
		tm->graphtype,
		tm->visible.x, tm->visible.y,
		tm->visible.x * tm->dpp.x, tm->visible.y * tm->dpp.y,
		tm->dpp.x, tm->dpp.y);

	_GGI_terminfo_freedbs(vis);

	memset(LIBGGI_PIXFMT(vis), 0, sizeof(ggi_pixelformat));
	setup_pixfmt(LIBGGI_PIXFMT(vis), tm->graphtype);
	_ggi_build_pixfmt(LIBGGI_PIXFMT(vis));

	/* These are private or public buffers ? (-steve) */

	_ggi_db_add_buffer(LIBGGI_PRIVLIST(vis), _ggi_db_get_new());
	LIBGGI_PRIVBUFS(vis)[0]->type  = GGI_DB_NORMAL | GGI_DB_SIMPLE_PLB;
	LIBGGI_PRIVBUFS(vis)[0]->frame = 0;
	LIBGGI_PRIVBUFS(vis)[0]->read 
		= LIBGGI_PRIVBUFS(vis)[0]->write
		= _ggi_malloc(LIBGGI_FB_SIZE(tm));
	LIBGGI_PRIVBUFS(vis)[0]->layout = blPixelLinearBuffer;
	LIBGGI_PRIVBUFS(vis)[0]->buffer.plb.stride = GT_ByPPP(tm->virt.x, tm->graphtype);
	LIBGGI_PRIVBUFS(vis)[0]->buffer.plb.pixelformat = LIBGGI_PIXFMT(vis);

	memcpy(LIBGGI_MODE(vis), tm, sizeof(ggi_mode));

	return _GGI_terminfo_domode(vis);
}

int GGI_terminfo_checkmode(struct ggi_visual *vis, ggi_mode *tm)
{
	struct TIhooks *priv = TERMINFO_PRIV(vis);
	int xdpp, ydpp;
	int err = 0;

	if (tm->frames != 1 && tm->frames != GGI_AUTO) {
		err = -1;
	}
	tm->frames = 1;

	xdpp = ydpp = 8;
	if ((tm->dpp.x != xdpp && tm->dpp.x != GGI_AUTO) ||
	    (tm->dpp.y != ydpp && tm->dpp.y != GGI_AUTO)) {
		err = -1;
	}
	tm->dpp.x = xdpp;
	tm->dpp.y = ydpp;

	_terminfo_select_screen(priv->scr);
	if ((tm->visible.x != COLS  && tm->visible.x != GGI_AUTO) || 
	    (tm->visible.y != LINES && tm->visible.y != GGI_AUTO)) {
		err = -1;
	}
	tm->visible.x = COLS;
	tm->visible.y = LINES;
	_terminfo_release_screen();

	if (tm->virt.x == GGI_AUTO) tm->virt.x = tm->visible.x;
	if (tm->virt.y == GGI_AUTO) tm->virt.y = tm->visible.y;

	if (tm->virt.x < tm->visible.x) {
		tm->virt.x = tm->visible.x;
		err = -1;
	}
	if (tm->virt.y < tm->visible.y) {
		tm->virt.y = tm->visible.y;
		err = -1;
	}

	err = _ggi_physz_figure_size(tm, priv->physzflags, &(priv->physz),
				0, 0, tm->visible.x, tm->visible.y);

	if (tm->graphtype == GT_TEXT) tm->graphtype = GT_TEXT32;
	if (tm->graphtype != GT_TEXT16 && tm->graphtype != GT_TEXT32) {
		tm->graphtype = GT_TEXT16;
		err = -1;
	}

	return err;
}

int GGI_terminfo_getmode(struct ggi_visual *vis, ggi_mode *tm)
{
	memcpy(tm, LIBGGI_MODE(vis), sizeof(ggi_mode));
	DPRINT("display-terminfo: getmode mode %8x %dx%d (%dx%d dots, %dx%d font)\n",
		tm->graphtype,
		tm->visible.x, tm->visible.y,
		tm->visible.x * tm->dpp.x, tm->visible.y * tm->dpp.y,
		tm->dpp.x, tm->dpp.y);
	return 0;
}

int GGI_terminfo_setflags(struct ggi_visual *vis, uint32_t flags)
{
	/* Doesn't support sync mode */
	LIBGGI_FLAGS(vis) = flags;
	LIBGGI_FLAGS(vis) &= GGIFLAG_ASYNC; /* Unkown flags don't take. */
	return 0;
}
