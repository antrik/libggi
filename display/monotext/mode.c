/* $Id: mode.c,v 1.4 2003/07/06 10:25:23 cegger Exp $
******************************************************************************

   Display-monotext: mode management

   Copyright (C) 1998 Andrew Apted		[andrew@ggi-project.org]
   Copyright (C) 1999-2000 Marcus Sundberg	[marcus@ggi-project.org]

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
#include <ggi/internal/ggi-dl.h>

#include <ggi/display/monotext.h>

/* Evil, FIXME - query target! */
static int target_width  = 80;
static int target_height = 25;


static void _GGIfreedbs(ggi_visual *vis) 
{
	int i;

	for (i=LIBGGI_PRIVLIST(vis)->num-1; i >= 0; i--) {
		_ggi_db_free(LIBGGI_PRIVBUFS(vis)[i]);
		_ggi_db_del_buffer(LIBGGI_PRIVLIST(vis), i);
	}
}

int GGI_monotext_getapi(ggi_visual *vis, int num, char *apiname, char *arguments)
{
	strcpy(arguments, "");

	switch(num) {
	case 0:
		strcpy(apiname, "display-monotext");
		return 0;

	case 1:
		strcpy(apiname, "generic-stubs");
		return 0;

	case 2:
		strcpy(apiname, "generic-linear-8");
		return 0;

	case 3: strcpy(apiname, "generic-color");
		return 0;
	}

	return -1;
}


/*
 * Attempt to get the default framebuffer.
 */

static int do_dbstuff(ggi_visual *vis)
{
	ggi_monotext_priv *priv = LIBGGI_PRIVATE(vis);

	priv->fb_size = LIBGGI_FB_SIZE(LIBGGI_MODE(vis));
	priv->fb_ptr  = malloc((size_t)(priv->fb_size));
	
	GGIDPRINT_MODE("display-monotext: fb=%p size=%d\n", 
		       priv->fb_ptr, priv->fb_size);

	if (priv->fb_ptr == NULL) {
		fprintf(stderr, "display-monotext: Out of memory.\n");
		return GGI_ENOMEM;
	}

	/* Set up direct buffers */
	_ggi_db_add_buffer(LIBGGI_PRIVLIST(vis), _ggi_db_get_new());

	LIBGGI_PRIVBUFS(vis)[0]->type  = GGI_DB_NORMAL | GGI_DB_SIMPLE_PLB;
	LIBGGI_PRIVBUFS(vis)[0]->frame = 0;
	LIBGGI_PRIVBUFS(vis)[0]->read  = priv->fb_ptr;
	LIBGGI_PRIVBUFS(vis)[0]->write = priv->fb_ptr;
	LIBGGI_PRIVBUFS(vis)[0]->layout = blPixelLinearBuffer;
	LIBGGI_PRIVBUFS(vis)[0]->buffer.plb.stride = 
		(LIBGGI_VIRTX(vis) * GT_SIZE(LIBGGI_GT(vis)) + 7) / 8;
	LIBGGI_PRIVBUFS(vis)[0]->buffer.plb.pixelformat = LIBGGI_PIXFMT(vis);

	/* Set up palette */
	if (vis->palette) {
		free(vis->palette);
		vis->palette = NULL;
	}
	if (GT_SCHEME(LIBGGI_GT(vis)) == GT_PALETTE) {
		vis->palette = _ggi_malloc((1 << GT_DEPTH(LIBGGI_GT(vis)))*
					   sizeof(ggi_color));
	}

	return 0;
}

static int do_setmode(ggi_visual *vis, ggi_mode *mode)
{
	ggi_monotext_priv *priv = LIBGGI_PRIVATE(vis);
	char libname[256], libargs[256];
	int err, id;

	_GGIfreedbs(vis);

	if ((err = do_dbstuff(vis)) != 0) {
		return err;
	}

	/* Fill in ggi_pixelformat */
	memset(LIBGGI_PIXFMT(vis), 0, sizeof(ggi_pixelformat));
	LIBGGI_PIXFMT(vis)->depth = GT_DEPTH(mode->graphtype);
	LIBGGI_PIXFMT(vis)->size  = GT_SIZE(mode->graphtype);
	LIBGGI_PIXFMT(vis)->clut_mask = 0xff;

	_ggi_build_pixfmt(LIBGGI_PIXFMT(vis));

	priv->squish.x = mode->visible.x / target_width;
	priv->squish.y = mode->visible.y / target_height;
	
	/* Load libraries */
	for(id=1; GGI_monotext_getapi(vis, id, libname, libargs) == 0; id++) {
		err = _ggiOpenDL(vis, libname, libargs, NULL);
		if (err) {
			fprintf(stderr, "display-monotext: Error opening "
				" %s (%s) library.\n", libname, libargs);
			return GGI_EFATAL;
		}
		GGIDPRINT_LIBS("Success in loading %s (%s)\n", libname,
			       libargs);
	}


	/* Backup the current 2D operations, and override them with our
	 * own (which then call on these backups to do the work).
	 */

	priv->mem_opdraw = _ggi_malloc(sizeof(struct ggi_visual_opdraw));

	*priv->mem_opdraw = *vis->opdraw;

	vis->opdraw->drawpixel_nc=GGI_monotext_drawpixel_nc;
	vis->opdraw->drawpixel=GGI_monotext_drawpixel;
	vis->opdraw->drawhline_nc=GGI_monotext_drawhline_nc;
	vis->opdraw->drawhline=GGI_monotext_drawhline;
	vis->opdraw->drawvline_nc=GGI_monotext_drawvline_nc;
	vis->opdraw->drawvline=GGI_monotext_drawvline;
	vis->opdraw->drawline=GGI_monotext_drawline;

	vis->opdraw->putc=GGI_monotext_putc;
	vis->opdraw->putpixel_nc=GGI_monotext_putpixel_nc;
	vis->opdraw->putpixel=GGI_monotext_putpixel;
	vis->opdraw->puthline=GGI_monotext_puthline;
	vis->opdraw->putvline=GGI_monotext_putvline;
	vis->opdraw->putbox=GGI_monotext_putbox;

	vis->opdraw->drawbox=GGI_monotext_drawbox;
	vis->opdraw->copybox=GGI_monotext_copybox;
	vis->opdraw->crossblit=GGI_monotext_crossblit;
	vis->opdraw->fillscreen=GGI_monotext_fillscreen;

	vis->opdraw->setorigin=GGI_monotext_setorigin;
	vis->opcolor->setpalvec=GGI_monotext_setpalvec;
	
	ggiIndicateChange(vis, GGI_CHG_APILIST);

	GGIDPRINT_MODE("display-monotext: Attempting to setmode on parent "
		       "visual...\n");

	return _ggi_monotextOpen(vis);
}

int GGI_monotext_setmode(ggi_visual *vis, ggi_mode *mode)
{ 
	int err;

	if ((vis == NULL) || (mode == NULL) || (LIBGGI_MODE(vis) == NULL)) {
		GGIDPRINT_MODE("display-monotext: vis/mode == NULL\n");
		return -1;
	}
	
	GGIDPRINT_MODE("display-monotext: setmode %dx%d (gt=%d)\n",
		       mode->visible.x, mode->visible.y, mode->graphtype);

	if ((err = ggiCheckMode(vis, mode)) != 0) {
		return err;
	}

	_ggiZapMode(vis, 0);

	memcpy(LIBGGI_MODE(vis), mode, sizeof(ggi_mode));
	
	if ((err = do_setmode(vis, mode)) != 0) {
		GGIDPRINT_MODE("display-monotext: setmode failed (%d)\n",
			       err);
		return err;
	}

	GGIDPRINT_MODE("display-monotext: setmode succeeded\n", vis, mode);

	return 0;
}

static int calc_squish(ggi_monotext_priv *priv, ggi_mode *mode, 
		       int _target_width, int _target_height)
{
	int sq_x, sq_y;
	int totw = _target_width *priv->accuracy.x;
	int toth = _target_height * priv->accuracy.y;

#if 0   /* Preliminary mode-improvement code */
	while ((mode->visible.x % totw)  != 0) mode->visible.x++;
	while ((mode->visible.y % toth) != 0) mode->visible.y++;
	mode->virt.x = mode->visible.x;
	mode->virt.y = mode->visible.y;
#endif

	if (((mode->visible.x % totw)  != 0) ||
	    ((mode->visible.y % toth) != 0)) {
		GGIDPRINT_MODE("display-monotext: visible size is not a "
			       "multiple of the target size.\n");
		return -1;
	}
	
	sq_x = mode->visible.x / totw;
	sq_y = mode->visible.y / toth;

	if (sq_x <= 0 || sq_y <= 0) {
		GGIDPRINT_MODE("display-monotext: visible size is not a "
			       "multiple of the target size.\n");
		return -1;
	}

	if (mode->visible.x / priv->accuracy.x / sq_x != totw ||
	    mode->visible.y / priv->accuracy.y / sq_y != toth) {
		return -1;
	}

	return 0;
}

int GGI_monotext_checkmode(ggi_visual *vis, ggi_mode *mode)
{
	ggi_monotext_priv *priv = LIBGGI_PRIVATE(vis);
	int err = 0;

	if ((vis == NULL) || (mode == NULL)) {
		GGIDPRINT_MODE("display-monotext: vis/mode == NULL\n");
		return -1;
	}

	GGIDPRINT_MODE("display-monotext: checkmode %dx%d (gt=%d)\n",
		  mode->visible.x, mode->visible.y, mode->graphtype);

	/* Handle GGI_AUTO */
	if (mode->graphtype == GGI_AUTO) {
		mode->graphtype = GT_8BIT;
	}

	if ((mode->visible.x == GGI_AUTO) &&
	    (mode->virt.x    == GGI_AUTO)) {
	    mode->visible.x = mode->virt.x = target_width * priv->accuracy.x;
	} else if (mode->virt.x == GGI_AUTO) {
	    mode->virt.x = mode->visible.x;
	} else if ((mode->visible.x == GGI_AUTO) ||
		   (mode->visible.x > mode->virt.x)) {
	    mode->visible.x = mode->virt.x;
	}

	if ((mode->visible.y == GGI_AUTO) &&
	    (mode->virt.y == GGI_AUTO)) {
	    mode->visible.y = mode->virt.y = target_height * priv->accuracy.y;
	} else if (mode->virt.y == GGI_AUTO) {
	    mode->virt.y = mode->visible.y;
	} else if ((mode->visible.y == GGI_AUTO) ||
		   (mode->visible.y > mode->virt.y)) {
	    mode->visible.y = mode->virt.y;
	}

	if (mode->frames != 1 && mode->frames != GGI_AUTO) {
		err = -1;
	}
	mode->frames = 1;

	if ((mode->dpp.x != 1 && mode->dpp.x != GGI_AUTO) ||
	    (mode->dpp.y != 1 && mode->dpp.y != GGI_AUTO)) {
		err = -1;
	}
	mode->dpp.x = mode->dpp.y = 1;

	if (mode->size.x != GGI_AUTO || mode->size.y != GGI_AUTO) {
		err = -1;
	}
	mode->size.x = mode->size.y = GGI_AUTO;

	/* Check stuff */
	if (mode->graphtype != GT_8BIT) {
		mode->graphtype = GT_8BIT;
		err = -1;
	}

	if (mode->visible.x != mode->virt.x) {
		mode->virt.x = mode->visible.x;
		err = -1;
	}
	if (mode->visible.y != mode->virt.y) {
		mode->virt.y = mode->visible.y;
		err = -1;
	}

	if (calc_squish(priv, mode, target_width, target_height) != 0) {
		mode->visible.x = target_width * priv->accuracy.x;
		mode->visible.y = target_height * priv->accuracy.y;
		err = -1;
	}

	return 0;
}

int GGI_monotext_getmode(ggi_visual *vis, ggi_mode *mode)
{
	GGIDPRINT_MODE("display-monotext: getmode.\n");

	memcpy(mode, LIBGGI_MODE(vis), sizeof(ggi_mode));

	return 0;
}

int GGI_monotext_setflags(ggi_visual *vis, ggi_flags flags)
{
	LIBGGI_FLAGS(vis) = flags;
	LIBGGI_FLAGS(vis) &= GGIFLAG_ASYNC; /* Unkown flags don't take. */

	return 0;
}

int
GGI_monotext_flush(ggi_visual *vis, int x, int y, int w, int h, int tryflag)
{
	ggi_monotext_priv *priv = LIBGGI_PRIVATE(vis);
	int err;

	if ((err = _ggi_monotextFlush(vis)) < 0) {
		return err;
	}

	return _ggiInternFlush(priv->parent, x, y, w, h, tryflag);
}
