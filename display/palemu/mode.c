/* $Id: mode.c,v 1.14 2004/11/13 15:56:23 cegger Exp $
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


static void _GGI_palemu_freedbs(ggi_visual *vis) 
{
	int i;

	for (i=LIBGGI_PRIVLIST(vis)->num-1; i >= 0; i--) {
		_ggi_db_free(LIBGGI_PRIVBUFS(vis)[i]);
		_ggi_db_del_buffer(LIBGGI_PRIVLIST(vis), i);
	}
}

int GGI_palemu_getapi(ggi_visual *vis, int num, char *apiname, char *arguments)
{
	*arguments = '\0';

	switch(num) {

	case 0: strcpy(apiname, "display-palemu");
		return 0;

	case 1: strcpy(apiname, "generic-stubs");
		return 0;

	case 2: sprintf(apiname, "generic-linear-%u%s",
			GT_DEPTH(LIBGGI_GT(vis)),
			(LIBGGI_GT(vis) & GT_SUB_HIGHBIT_RIGHT) ? "-r" : "");
		return 0;

	case 3: strcpy(apiname, "generic-color");
		return 0;
		
	case 4: strcpy(apiname, "generic-pseudo-stubs");
		sprintf(arguments, "%p", PALEMU_PRIV(vis)->parent);
		return 0;
	}

	return GGI_ENOMATCH;
}


/*
 * Attempt to get the default framebuffer.
 */

static int do_dbstuff(ggi_visual *vis)
{
	ggi_palemu_priv *priv = PALEMU_PRIV(vis);
	int i;

	/* allocate memory */
	priv->frame_size = LIBGGI_FB_SIZE(LIBGGI_MODE(vis));
	priv->fb_size = priv->frame_size * LIBGGI_MODE(vis)->frames;
	priv->fb_ptr  = malloc((size_t)priv->fb_size);
	
	GGIDPRINT_MODE("display-palemu: fb=%p size=%d frame=%d\n", 
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

static int do_setmode(ggi_visual *vis)
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
		err = _ggiOpenDL(vis, libname, libargs, NULL);
		if (err) {
			fprintf(stderr, "display-palemu: Error opening "
				" %s (%s) library.\n", libname, libargs);
			return GGI_EFATAL;
		}

		GGIDPRINT("Success in loading %s (%s)\n", libname, libargs);
	}


	/* Backup the current 2D operations, and override them with our
	 * own (which then call on these backups to do the work).
	 */

	priv->mem_opdraw = _ggi_malloc(sizeof(struct ggi_visual_opdraw));

	*priv->mem_opdraw = *vis->opdraw;

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
	vis->opdraw->putbox=GGI_palemu_putbox;

	vis->opdraw->drawbox=GGI_palemu_drawbox;
	vis->opdraw->copybox=GGI_palemu_copybox;
	vis->opdraw->crossblit=GGI_palemu_crossblit;
	vis->opdraw->fillscreen=GGI_palemu_fillscreen;

	vis->opdraw->setorigin=GGI_palemu_setorigin;

	/* colormap initialization */
	LIBGGI_PAL(vis)->setPalette = GGI_palemu_setPalette;

	vis->opdraw->setreadframe=GGI_palemu_setreadframe;
	vis->opdraw->setwriteframe=GGI_palemu_setwriteframe;
	vis->opdraw->setdisplayframe=GGI_palemu_setdisplayframe;

	ggiIndicateChange(vis, GGI_CHG_APILIST);

	/* set initial frames */
	priv->mem_opdraw->setreadframe(vis, 0);
	priv->mem_opdraw->setwriteframe(vis, 0);

	return 0;
}

int GGI_palemu_setmode(ggi_visual *vis, ggi_mode *mode)
{ 
	ggi_palemu_priv *priv = PALEMU_PRIV(vis);
	int err;

	GGIDPRINT_MODE("display-palemu: setmode %dx%d#%dx%dF%d[0x%02x]\n",
			mode->visible.x, mode->visible.y,
			mode->virt.x, mode->virt.y, 
			mode->frames, mode->graphtype);

	MANSYNC_ignore(vis);

	if ((err = ggiCheckMode(vis, mode)) != 0) {
		return err;
	}

	_ggiZapMode(vis, 0);

	*LIBGGI_MODE(vis) = *mode;
	
	priv->mode.visible = mode->visible;
	priv->mode.virt    = mode->virt;
	priv->mode.dpp     = mode->dpp;
	priv->mode.size    = mode->size;
	priv->mode.frames  = 1;

	if ((err = do_setmode(vis)) != 0) {
		GGIDPRINT_MODE("display-palemu: setmode failed (%d).\n", err);
		return err;
	}

	GGIDPRINT_MODE("display-palemu: Attempting to setmode on parent "
		"visual...\n");

	if ((err = _ggi_palemu_Open(vis)) != 0) {
		return err;
	}

	/* Initialize palette */
	ggiSetColorfulPalette(vis);

	MANSYNC_SETFLAGS(vis, LIBGGI_FLAGS(vis));
	MANSYNC_cont(vis);

	GGIDPRINT_MODE("display-palemu: setmode succeeded.\n");

	return 0;
}

int GGI_palemu_resetmode(ggi_visual *vis)
{
	ggi_palemu_priv *priv = PALEMU_PRIV(vis);

	GGIDPRINT("display-palemu: GGIresetmode(%p)\n", vis);

	if (priv->fb_ptr != NULL) {

		_ggi_palemu_Close(vis);

		_GGI_palemu_freedbs(vis);

		free(priv->fb_ptr);
		priv->fb_ptr = NULL;
	}

	return 0;
}

int GGI_palemu_checkmode(ggi_visual *vis, ggi_mode *mode)
{
	ggi_palemu_priv *priv = PALEMU_PRIV(vis);
	ggi_mode par_mode;
	int tmperr, err = 0;

	GGIDPRINT_MODE("display-palemu: checkmode %dx%d#%dx%dF%d[0x%02x]\n",
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
		mode->visible.x = priv->mode.visible.x;
	}
	if (mode->visible.y == GGI_AUTO) {
		mode->visible.y = priv->mode.visible.y;
	}
	if (mode->virt.x == GGI_AUTO) {
		mode->virt.x = priv->mode.virt.x;
	}
	if (mode->virt.y == GGI_AUTO) {
		mode->virt.y = priv->mode.virt.y;
	}
	if (mode->dpp.x == GGI_AUTO) {
		mode->dpp.x = priv->mode.dpp.x;
	}
	if (mode->dpp.y == GGI_AUTO) {
		mode->dpp.y = priv->mode.dpp.y;
	}
	if (mode->size.x == GGI_AUTO) {
		mode->size.x = priv->mode.size.x;
	}
	if (mode->size.y == GGI_AUTO) {
		mode->size.y = priv->mode.size.y;
	}
	if (mode->frames == GGI_AUTO) {
		mode->frames = 1;
	}

	/* Now check mode against the parent target (letting the parent
	 * target handle any remaining GT_AUTO and GGI_AUTO values).
	 */
	par_mode = *mode;

	par_mode.graphtype = priv->mode.graphtype;

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
	
	GGIDPRINT_MODE("display-palemu: result %d %dx%d#%dx%dF%d[0x%02x]\n",
			err, mode->visible.x, mode->visible.y,
			mode->virt.x, mode->virt.y, 
			mode->frames, mode->graphtype);
	return err;
}

int GGI_palemu_getmode(ggi_visual *vis, ggi_mode *mode)
{
	if ((vis == NULL) || (mode == NULL) || (LIBGGI_MODE(vis) == NULL)) {
		GGIDPRINT_MODE("display-palemu: vis/mode == NULL\n");
		return GGI_EARGINVAL;
	}
	
	GGIDPRINT_MODE("display-palemu: getmode.\n");

	memcpy(mode, LIBGGI_MODE(vis), sizeof(ggi_mode));

	return 0;
}

int GGI_palemu_setflags(ggi_visual *vis, ggi_flags flags)
{
	LIBGGI_FLAGS(vis) = flags;

	MANSYNC_SETFLAGS(vis, flags);
	LIBGGI_FLAGS(vis) &= GGIFLAG_ASYNC; /* Unkown flags don't take. */

	return 0;
}

int GGI_palemu_flush(ggi_visual *vis, int x, int y, int w, int h, int tryflag)
{
	ggi_palemu_priv *priv = PALEMU_PRIV(vis);
	int err;

	MANSYNC_ignore(vis);

	ggLock(priv->flush_lock);

	err = _ggi_palemu_Flush(vis);

	if (! err) {
		err = _ggiInternFlush(priv->parent, x, y, w, h, tryflag);
	}

	ggUnlock(priv->flush_lock);

	MANSYNC_cont(vis);

	return err;
}
