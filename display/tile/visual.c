/* $Id: visual.c,v 1.4 2004/09/12 20:48:22 cegger Exp $
******************************************************************************

   Initializing tiles

   Copyright (C) 1998 Steve Cheng	[steve@ggi-project.org]
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

#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include <ggi/display/tile.h>

void _GGI_tile_freedbs(ggi_visual *vis)
{
	int i;

	for (i=LIBGGI_APPLIST(vis)->num-1; i >= 0; i--) {
		free(LIBGGI_APPBUFS(vis)[i]->write);
		_ggi_db_free(LIBGGI_APPBUFS(vis)[i]);
		_ggi_db_del_buffer(LIBGGI_APPLIST(vis), i);
	}
}


static const char argument_format[] = "display-tile:\n\
    The argument format is `offx,offy,sizex,sizey,(subdisplay):...',\n\
    where offx and offy are the tile's offset from the main display,\n\
    sizex and sizey are the size of the tile,\n\
    subdisplay is the display string to open for the tile,\n\
    and ... is more tiles following the same format as above...\n";


static int GGIopen(ggi_visual *vis, struct ggi_dlhandle *dlh,
			const char *args, void *argptr, uint32 *dlret)
{
	ggi_tile_priv *priv;
	char target[1024];
	int sx, sy, vx, vy, n, i=0;
	int err = GGI_ENOMEM;

	if (!args || *args == '\0') {
		fprintf(stderr, "display-tile needs the real targets as arguments.\n");
		fprintf(stderr, argument_format);
		return GGI_EARGREQ;
	}

	priv = TILE_PRIV(vis) = malloc(sizeof(ggi_tile_priv));
	if (priv == NULL) return GGI_ENOMEM;

	LIBGGI_GC(vis) = malloc(sizeof(ggi_gc));
	if (LIBGGI_GC(vis) == NULL) {
		goto out_freepriv;
	}

	priv->opmansync = malloc(sizeof(_ggi_opmansync));
	if (priv->opmansync == NULL) {
		goto out_freegc;
	}

	priv->buf = NULL;
	priv->use_db = 1;

	/* parse each visual */
	for (;;) {
		sx = sy = vx = vy = 0;

		while (*args && isspace((int)*args)) args++;

		if (! *args) break;

		if (strncmp(args, "-usedb:", 7) == 0) {
			GGIDPRINT_MISC("display-tile: Enabling DB\n");
			priv->use_db = 1;
			args += 7; continue;
		}
		if (strncmp(args, "-nodb:", 6) == 0) {
			GGIDPRINT_MISC("display-tile: Disabling DB\n");
			priv->use_db = 0;
			args += 6; continue;
		}

		/* Avoid overflowing tables. If someone ever needs more than
		   256 we'll do dynamic allocation instead. */
		if (i==MAX_VISUALS) {
			ggiPanic("display-tile: FIXME: visual limit reached!\n");
		}

		err = GGI_EARGINVAL;

		if ((sscanf(args, "%d , %d , %d , %d %n", &sx, &sy,
			    &vx, &vy, &n) != 4) ||
		    (args += n, *args++ != ',')) {
			fprintf(stderr, argument_format);
			goto out_freeopmansync;
		}

		/* Empty tile! */
		if(vx<=0 || vy<=0 || sx<0 || sy<0) {
			fprintf(stderr, "display-tile: erroneous coordinates for tile #%d!\n", i);
			goto out_freeopmansync;
		}

		priv->vis_origins[i].x = sx;
		priv->vis_origins[i].y = sy;
		priv->vis_sizes[i].x = vx;
		priv->vis_sizes[i].y = vy;

		args = ggParseTarget((char *)args, target, 1024);

		if (! args) {
			fprintf(stderr,"display-tile: parsetarget error.\n");
			goto out_freeopmansync;
		}

		if (*target == '\0') {
			strcpy(target, "auto");
		}

		GGIDPRINT_MISC("display-tile: visual #%d is %s (%d,%d)[%dx%d]\n",
			i, target, sx, sy, vx, vy);

		if (! (priv->vislist[i]=ggiOpen(target,NULL)) ) {
			fprintf(stderr,"display-tile: Opening of target %s failed.\n", target);
			err = GGI_ENODEVICE;
			goto out_freeopmansync;
		}

		/* Add giiInputs, if we have them. */
		if (priv->vislist[i]->input) {
			vis->input=giiJoinInputs(vis->input,priv->vislist[i]->input);
			priv->vislist[i]->input=NULL;	/* Destroy old reference */
		}

		if (priv->use_db) {
			/* Don't need SYNC mode, we do it ourselves */
			ggiSetFlags(priv->vislist[i], GGIFLAG_ASYNC);
		}

		/* check for ':' separator */

		while (*args && isspace((int)*args)) args++;

		if (*args && (*args != ':')) {
			fprintf(stderr, "display-tile: expecting ':' between targets.\n");
			err = GGI_EARGINVAL;
			goto out_closevisuals;
		}

		if (*args == ':') args++;
		i++;
	}

	priv->numvis=i;

	if (priv->numvis == 0) {
		fprintf(stderr, "display-tile needs the real targets as arguments.\n");
		err = GGI_EARGINVAL;
		goto out_freeopmansync;
	}

	if (priv->use_db) {
		err = _ggiAddDL(vis, "helper-mansync", NULL,
				    priv->opmansync, 0);
		if (err) {
			fprintf(stderr, "display-tile: Cannot load required helper-mansync! (for DB mode)\n");
			goto out_closevisuals;

		}

		MANSYNC_init(vis);
	}

	vis->opdisplay->getmode=GGI_tile_getmode;
	vis->opdisplay->setmode=GGI_tile_setmode;
	vis->opdisplay->checkmode=GGI_tile_checkmode;
	vis->opdisplay->getapi=GGI_tile_getapi;
	vis->opdisplay->setflags=GGI_tile_setflags;

	if (priv->use_db) {
		vis->opdisplay->flush=GGI_tile_flush_db;
	} else {
		vis->opdisplay->flush=GGI_tile_flush;
	}

	*dlret = GGI_DL_OPDISPLAY;
	return 0;

  out_closevisuals:
	/* Close opened visuals. */
	while (i--) {
		ggiClose(priv->vislist[i]);
	}
  out_freeopmansync:
	free(priv->opmansync);
  out_freegc:
	free(LIBGGI_GC(vis));
  out_freepriv:
	free(priv);

	return err;
}

static int GGIclose(ggi_visual *vis, struct ggi_dlhandle *dlh)
{
	ggi_tile_priv *priv = TILE_PRIV(vis);
	int i;

	if (priv->use_db) {
		_GGI_tile_freedbs(vis);
	}

	if (priv->buf) free(priv->buf);

	/* Shut down targets in reverse order to avoid any nesting problems,
	 * if external dependencies (like saving/restoring state on subsystems
	 * that are used by multiple targets).
	 */
	for (i = priv->numvis; i>=0; i--) {
		ggiClose(priv->vislist[i]);
	}

	free(priv->opmansync);
	free(priv);
	free(LIBGGI_GC(vis));

	return 0;
}


static int GGIexit(ggi_visual *vis, struct ggi_dlhandle *dlh)
{
	ggi_tile_priv *priv = TILE_PRIV(vis);

	if (priv->use_db) {
		MANSYNC_deinit(vis);
	}

	return 0;
}

EXPORTFUNC
int GGIdl_tile(int func, void **funcptr);

int GGIdl_tile(int func, void **funcptr)
{
	switch (func) {
	case GGIFUNC_open:
		*funcptr = GGIopen;
		return 0;
	case GGIFUNC_exit:
		*funcptr = GGIexit;
		return 0;
	case GGIFUNC_close:
		*funcptr = GGIclose;
		return 0;
	default:
		*funcptr = NULL;
	}

	return GGI_ENOTFOUND;
}

#include <ggi/internal/ggidlinit.h>
