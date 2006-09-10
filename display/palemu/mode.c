/* $Id: mode.c,v 1.21 2006/09/10 06:53:13 cegger Exp $
******************************************************************************

   Display-palemu: mode management

   Copyright (C) 1998 Andrew Apted    [andrew@ggi-project.org]

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

#include "config.h"
#include <ggi/display/palemu.h>
#include <ggi/internal/ggi_debug.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../common/pixfmt-setup.inc"
#include "../common/gt-auto.inc"


static void _GGI_palemu_freedbs(struct ggi_visual *vis) 
{
	int i;

	for (i=LIBGGI_PRIVLIST(vis)->num-1; i >= 0; i--) {
		_ggi_db_free(LIBGGI_PRIVBUFS(vis)[i]);
		_ggi_db_del_buffer(LIBGGI_PRIVLIST(vis), i);
	}
}

int GGI_palemu_getapi(struct ggi_visual *vis, int num, char *apiname, char *arguments)
{
	ggi_palemu_priv *priv = PALEMU_PRIV(vis);
	*arguments = '\0';

	switch(num) {
	case 0:
		if (priv->target == PALEMU_TARGET)
			strcpy(apiname, "display-palemu");
		else
			strcpy(apiname, "display-monotext");
		return 0;

	case 1: 
		strcpy(apiname, "generic-stubs");
		return 0;

	case 2: 
		sprintf(apiname, "generic-linear-%u%s",
			GT_DEPTH(LIBGGI_GT(vis)),
			(LIBGGI_GT(vis) & GT_SUB_HIGHBIT_RIGHT) ? "-r" : "");
		return 0;

	case 3: 
		strcpy(apiname, "generic-color");
		return 0;
		
	case 4: 
		strcpy(apiname, "generic-pseudo-stubs");
		sprintf(arguments, "%p", (void *)PALEMU_PRIV(vis)->parent);
		return 0;
	}

	return GGI_ENOMATCH;
}


/*
 * Attempt to get the default framebuffer.
 */

static int do_dbstuff(struct ggi_visual *vis)
{
	ggi_palemu_priv *priv = PALEMU_PRIV(vis);
	int i;

	/* allocate memory */
	priv->frame_size = LIBGGI_FB_SIZE(LIBGGI_MODE(vis));
	priv->fb_size = priv->frame_size * LIBGGI_MODE(vis)->frames;
	priv->fb_ptr  = malloc((size_t)priv->fb_size);
	
	DPRINT_MODE("display-palemu: fb=%p size=%d frame=%d\n", 
		priv->fb_ptr, priv->fb_size, priv->frame_size);

	if (priv->fb_ptr == NULL) {
		fprintf(stderr, "display-palemu: Out of memory.\n");
		return GGI_ENOMEM;
	}

        /* clear all frames */
	memset(priv->fb_ptr, 0, (size_t)priv->fb_size);

	/* set up pixel format */
	memset(LIBGGI_PIXFMT(vis), 0, sizeof(ggi_pixelformat));
	setup_pixfmt(LIBGGI_PIXFMT(vis), LIBGGI_GT(vis));
	_ggi_build_pixfmt(LIBGGI_PIXFMT(vis));

	/* set up direct buffers */
	for (i=0; i < LIBGGI_MODE(vis)->frames; i++) {
		ggi_directbuffer *buf;
		
		_ggi_db_add_buffer(LIBGGI_PRIVLIST(vis), _ggi_db_get_new());

		buf = LIBGGI_PRIVBUFS(vis)[i];

		buf->frame = i;
		buf->type  = GGI_DB_NORMAL | GGI_DB_SIMPLE_PLB;
		buf->read  = (char *) priv->fb_ptr + i * priv->frame_size;
		buf->write = buf->read;
		buf->layout = blPixelLinearBuffer;

		buf->buffer.plb.stride = 
			GT_ByPPP(LIBGGI_VIRTX(vis), LIBGGI_GT(vis));
		buf->buffer.plb.pixelformat = LIBGGI_PIXFMT(vis);
	}

	/* Set up palette */
	if (LIBGGI_PAL(vis)->clut.data) {
		free(LIBGGI_PAL(vis)->clut.data);
 		LIBGGI_PAL(vis)->clut.data = NULL;
	}
	if (GT_SCHEME(LIBGGI_GT(vis)) == GT_PALETTE) {
		LIBGGI_PAL(vis)->clut.data = _ggi_malloc((1 << GT_DEPTH(LIBGGI_GT(vis))) *
						sizeof(ggi_color));
		LIBGGI_PAL(vis)->clut.size = 1 << GT_DEPTH(LIBGGI_GT(vis));
	}

	return 0;
}

static int do_setmode(struct ggi_visual *vis)
{
	ggi_palemu_priv *priv = PALEMU_PRIV(vis);
	char libname[GGI_MAX_APILEN], libargs[GGI_MAX_APILEN];
	int err, id;


	_GGI_palemu_freedbs(vis);

	if ((err = do_dbstuff(vis)) != 0) {
		return err;
	}
	
	/* load libraries */

	for (id=1; GGI_palemu_getapi(vis, id, libname, libargs) == 0; id++) {
		err = _ggiOpenDL(vis, _ggiGetConfigHandle(),
				libname, libargs, NULL);
		if (err) {
			fprintf(stderr, "display-palemu: Error opening "
				" %s (%s) library.\n", libname, libargs);
			return GGI_EFATAL;
		}

		DPRINT("Success in loading %s (%s)\n", libname, libargs);
	}


	/* Backup the current 2D operations, and override them with our
	 * own (which then call on these backups to do the work).
	 */

	priv->mem_opdraw = _ggi_malloc(sizeof(struct ggi_visual_opdraw));

	*priv->mem_opdraw = *vis->opdraw;

	if (priv->target == PALEMU_TARGET) {
		vis->opdraw->putbox = GGI_palemu_putbox;
		vis->opdraw->drawbox = GGI_palemu_drawbox;
		vis->opdraw->copybox = GGI_palemu_copybox;
		vis->opdraw->crossblit = GGI_palemu_crossblit;
		vis->opdraw->fillscreen = GGI_palemu_fillscreen;
	} else {
		vis->opdraw->putbox = GGI_monotext_putbox;
		vis->opdraw->drawbox = GGI_monotext_drawbox;
		vis->opdraw->copybox = GGI_monotext_copybox;
		vis->opdraw->crossblit = GGI_monotext_crossblit;
		vis->opdraw->fillscreen = GGI_monotext_fillscreen;
	}

	vis->opdraw->drawpixel_nc=GGI_palemu_drawpixel_nc;
	vis->opdraw->drawpixel=GGI_palemu_drawpixel;
	vis->opdraw->drawhline_nc=GGI_palemu_drawhline_nc;
	vis->opdraw->drawhline=GGI_palemu_drawhline;
	vis->opdraw->drawvline_nc=GGI_palemu_drawvline_nc;
	vis->opdraw->drawvline=GGI_palemu_drawvline;
	vis->opdraw->drawline=GGI_palemu_drawline;

	vis->opdraw->putc=GGI_palemu_putc;
	vis->opdraw->putpixel_nc=GGI_palemu_putpixel_nc;
	vis->opdraw->putpixel=GGI_palemu_putpixel;
	vis->opdraw->puthline=GGI_palemu_puthline;
	vis->opdraw->putvline=GGI_palemu_putvline;


	vis->opdraw->setorigin=GGI_palemu_setorigin;

	/* colormap initialization */
	LIBGGI_PAL(vis)->setPalette = GGI_palemu_setPalette;

	vis->opdraw->setreadframe=GGI_palemu_setreadframe;
	vis->opdraw->setwriteframe=GGI_palemu_setwriteframe;
	vis->opdraw->setdisplayframe=GGI_palemu_setdisplayframe;

	ggiIndicateChange(vis->stem, GGI_CHG_APILIST);

	/* set initial frames */
	priv->mem_opdraw->setreadframe(vis, 0);
	priv->mem_opdraw->setwriteframe(vis, 0);

	return 0;
}

/* XXX Query target! */
static int target_width = 80;
static int target_height = 25;

int GGI_palemu_setmode(struct ggi_visual *vis, ggi_mode *mode)
{ 
	ggi_palemu_priv *priv = PALEMU_PRIV(vis);
	int err;

	DPRINT_MODE("display-palemu: setmode %dx%d#%dx%dF%d[0x%02x]\n",
			mode->visible.x, mode->visible.y,
			mode->virt.x, mode->virt.y, 
			mode->frames, mode->graphtype);

	MANSYNC_ignore(vis);

	if ((err = ggiCheckMode(vis->stem, mode)) != 0) {
		return err;
	}

	_ggiZapMode(vis, 0);

	*LIBGGI_MODE(vis) = *mode;
	
	priv->parent_mode.visible = mode->visible;
	priv->parent_mode.virt    = mode->virt;
	priv->parent_mode.dpp     = mode->dpp;
	priv->parent_mode.size    = mode->size;
	priv->parent_mode.frames  = 1;

	if ((err = do_setmode(vis)) != 0) {
		DPRINT_MODE("display-palemu: setmode failed (%d).\n", err);
		return err;
	}

	priv->squish.x = mode->visible.x / target_width;
	priv->squish.y = mode->visible.y / target_height;

	DPRINT_MODE("display-palemu: Attempting to setmode on parent "
		"visual...\n");

	if (priv->target == PALEMU_TARGET) {
		err = _ggi_palemu_Open(vis);
	} else {
		err = _ggi_monotext_Open(vis);
	}
	if (err != 0) {
		return err;
	}

	/* Initialize palette */
	ggiSetColorfulPalette(vis->stem);

	MANSYNC_SETFLAGS(vis, LIBGGI_FLAGS(vis));
	MANSYNC_cont(vis);

	DPRINT_MODE("display-palemu: setmode succeeded.\n");

	return 0;
}

int GGI_palemu_resetmode(struct ggi_visual *vis)
{
	ggi_palemu_priv *priv = PALEMU_PRIV(vis);

	DPRINT("display-palemu: GGIresetmode(%p)\n", vis);

	if (priv->fb_ptr != NULL) {

		_ggi_palemu_Close(vis);

		_GGI_palemu_freedbs(vis);

		free(priv->fb_ptr);
		priv->fb_ptr = NULL;
	}

	return 0;
}


static int calc_squish(ggi_palemu_priv *priv, ggi_mode *mode,
			int _target_width, int _target_height)
{
	int sq_x, sq_y;
	int totw = _target_width * priv->accuracy.x;
	int toth = _target_height * priv->accuracy.y;

#if 0	/* Preliminary mode-improvement code */
	while ((mode->visible.x % totw) != 0) mode.visible.x++;
	while ((mode->visible.y % toth) != 0) mode.visible.y++;
	mode->virt.x = mode->visible.x;
	mode->virt.y = mode->visible.y;
#endif

	if (((mode->visible.x % totw) != 0) ||
	    ((mode->visible.y % toth) != 0)) {
		DPRINT_MODE("display-monotext: visible size is not a "
			"multiple of the target size.\n");
		return GGI_ENOMATCH;
	}

	sq_x = mode->visible.x / totw;
	sq_y = mode->visible.y / toth;

	if (sq_x <= 0 || sq_y <= 0) {
		DPRINT_MODE("display-monotext: visible size is not a "
			"multiple of the target size.\n");
		return GGI_ENOMATCH;
	}

	if (mode->visible.x / priv->accuracy.x / sq_x != totw ||
	    mode->visible.y / priv->accuracy.y / sq_y != toth) {
		return GGI_ENOMATCH;
	}

	return 0;
}


int GGI_palemu_checkmode(struct ggi_visual *vis, ggi_mode *mode)
{
	ggi_palemu_priv *priv = PALEMU_PRIV(vis);
	ggi_mode par_mode;
	int tmperr, err = 0;

	DPRINT_MODE("display-palemu: checkmode %dx%d#%dx%dF%d[0x%02x]\n",
			mode->visible.x, mode->visible.y,
			mode->virt.x, mode->virt.y, 
			mode->frames, mode->graphtype);

	/* Handle graphtype */
	if (GT_SCHEME(mode->graphtype) == GT_AUTO) {
		GT_SETSCHEME(mode->graphtype, GT_PALETTE);
	}

	mode->graphtype = _GGIhandle_gtauto(mode->graphtype);

	if (GT_SCHEME(mode->graphtype) != GT_PALETTE) {
		GT_SETSCHEME(mode->graphtype, GT_PALETTE);
		err = -1;
	}

	if (GT_DEPTH(mode->graphtype) > 8) {
		GT_SETDEPTH(mode->graphtype, 8);
		err = -1;
	}

	if (GT_SIZE(mode->graphtype) != GT_DEPTH(mode->graphtype)) {
		GT_SETSIZE(mode->graphtype, GT_DEPTH(mode->graphtype));
		err = -1;
	}

	/* Handle geometry */
	if (mode->visible.x == GGI_AUTO) {
		mode->visible.x = priv->parent_mode.visible.x;
	}
	if (mode->visible.y == GGI_AUTO) {
		mode->visible.y = priv->parent_mode.visible.y;
	}
	if (mode->virt.x == GGI_AUTO) {
		mode->virt.x = priv->parent_mode.virt.x;
	}
	if (mode->virt.y == GGI_AUTO) {
		mode->virt.y = priv->parent_mode.virt.y;
	}
	if (mode->dpp.x == GGI_AUTO) {
		mode->dpp.x = priv->parent_mode.dpp.x;
	}
	if (mode->dpp.y == GGI_AUTO) {
		mode->dpp.y = priv->parent_mode.dpp.y;
	}
	if (mode->size.x == GGI_AUTO) {
		mode->size.x = priv->parent_mode.size.x;
	}
	if (mode->size.y == GGI_AUTO) {
		mode->size.y = priv->parent_mode.size.y;
	}
	if (mode->frames == GGI_AUTO) {
		mode->frames = 1;
	}

	/* Now check mode against the parent target (letting the parent
	 * target handle any remaining GT_AUTO and GGI_AUTO values).
	 */
	par_mode = *mode;

	par_mode.graphtype = priv->parent_mode.graphtype;

	tmperr = ggiCheckMode(priv->parent, &par_mode);
	if (tmperr) err = tmperr;

	mode->visible = par_mode.visible;
	mode->virt    = par_mode.virt;
	mode->dpp     = par_mode.dpp;
	mode->size    = par_mode.size;

	/* When the parent is palettized, we must limit the
	 * resulting depth to be <= the parent depth.
	 */

	if ((GT_SCHEME(par_mode.graphtype) == GT_PALETTE) &&
	    (GT_DEPTH(par_mode.graphtype) < 
	     GT_DEPTH(mode->graphtype))) {
		GT_SETDEPTH(mode->graphtype, 
			GT_DEPTH(par_mode.graphtype));
		GT_SETSIZE(mode->graphtype, 
			GT_DEPTH(par_mode.graphtype));
		err = -1;
	}
	
	DPRINT_MODE("display-palemu: result %d %dx%d#%dx%dF%d[0x%02x]\n",
			err, mode->visible.x, mode->visible.y,
			mode->virt.x, mode->virt.y, 
			mode->frames, mode->graphtype);
	return err;
}


int GGI_monotext_checkmode(struct ggi_visual *vis, ggi_mode *mode)
{
	ggi_palemu_priv *priv = PALEMU_PRIV(vis);
	int err = 0;

	if ((vis == NULL) || (mode == NULL)) {
		DPRINT_MODE("display-monotext: vis/mode == NULL\n");
		return GGI_EARGINVAL;
	}

	DPRINT_MODE("display-monotext: checkmode %dx%d (gt=%d)\n",
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
		mode->visible.x = mode->virt.y;
	}


	if ((mode->visible.y == GGI_AUTO) &&
	    (mode->virt.y    == GGI_AUTO)) {
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
		mode->visible.y = target_width * priv->accuracy.y;
		err = -1;
	}

	return err;
}

int GGI_palemu_getmode(struct ggi_visual *vis, ggi_mode *mode)
{
	if ((vis == NULL) || (mode == NULL) || (LIBGGI_MODE(vis) == NULL)) {
		DPRINT_MODE("display-palemu: vis/mode == NULL\n");
		return GGI_EARGINVAL;
	}
	
	DPRINT_MODE("display-palemu: getmode.\n");

	memcpy(mode, LIBGGI_MODE(vis), sizeof(ggi_mode));

	return 0;
}

int GGI_palemu_setflags(struct ggi_visual *vis, ggi_flags flags)
{
	LIBGGI_FLAGS(vis) = flags;

	MANSYNC_SETFLAGS(vis, flags);
	LIBGGI_FLAGS(vis) &= GGIFLAG_ASYNC; /* Unkown flags don't take. */

	return 0;
}

int GGI_palemu_flush(struct ggi_visual *vis, int x, int y, int w, int h, int tryflag)
{
	ggi_palemu_priv *priv = PALEMU_PRIV(vis);
	int err;

	MANSYNC_ignore(vis);

	ggLock(priv->flush_lock);

	if (priv->target == PALEMU_TARGET) {
		err = _ggi_palemu_Flush(vis);
	} else {
		err = _ggi_monotext_Flush(vis);
	}

	if (! err) {
		err = _ggiInternFlush(GGI_VISUAL(priv->parent), x, y, w, h, tryflag);
	}

	ggUnlock(priv->flush_lock);

	MANSYNC_cont(vis);

	return err;
}
