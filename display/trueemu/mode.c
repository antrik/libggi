/* $Id: mode.c,v 1.18 2007/03/04 18:26:45 soyt Exp $
******************************************************************************

   Display-trueemu : mode management

   Copyright (C) 1998 Andrew Apted	[andrew@ggi-project.org]
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

#include "config.h"
#include <ggi/display/trueemu.h>
#include <ggi/internal/ggi_debug.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../common/pixfmt-setup.inc"
#include "../common/gt-auto.inc"


static void _GGI_trueemu_freedbs(struct ggi_visual *vis) 
{
	int i;

	for (i=LIBGGI_PRIVLIST(vis)->num-1; i >= 0; i--) {
		_ggi_db_free(LIBGGI_PRIVBUFS(vis)[i]);
		_ggi_db_del_buffer(LIBGGI_PRIVLIST(vis), i);
	}
}

int GGI_trueemu_getapi(struct ggi_visual *vis, int num, char *apiname, char *arguments)
{
	*arguments = '\0';

	switch(num) {

	case 0: strcpy(apiname, "display-trueemu");
		return 0;

	case 1: strcpy(apiname, "generic-stubs");
		return 0;

	case 2: sprintf(apiname, "generic-linear-%u%s",
			GT_SIZE(LIBGGI_GT(vis)),
			(LIBGGI_GT(vis) & GT_SUB_HIGHBIT_RIGHT) ? "-r" : "");
		return 0;

	case 3: strcpy(apiname, "generic-color");
		return 0;
		
	case 4: strcpy(apiname, "generic-pseudo-stubs");
		sprintf(arguments, "%p", (void *)TRUEEMU_PRIV(vis)->parent);
		return 0;
	}

	return GGI_ENOMATCH;
}

static int do_dbstuff(struct ggi_visual *vis)
{
	ggi_trueemu_priv *priv = TRUEEMU_PRIV(vis);
	int i;

	/* allocate memory */
	if (priv->fb_ptr != NULL) {
		free(priv->fb_ptr);
	}
	priv->frame_size = LIBGGI_FB_SIZE(LIBGGI_MODE(vis));
	priv->fb_size = priv->frame_size * LIBGGI_MODE(vis)->frames;
	priv->fb_ptr  = malloc((size_t)(priv->fb_size));

	DPRINT("display-trueemu: fb=%p size=%d frame=%d\n", 
		priv->fb_ptr, priv->fb_size, priv->frame_size);

	if (priv->fb_ptr == NULL) {
		fprintf(stderr, "display-trueeemu: Out of memory.\n");
		return GGI_ENOMEM;
	}

	/* clear all frames */
	memset(priv->fb_ptr, 0, (size_t)(priv->fb_size));

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

	return 0;
}

static int do_setmode(struct ggi_visual *vis)
{
	ggi_trueemu_priv *priv = TRUEEMU_PRIV(vis);
	char libname[GGI_MAX_APILEN], libargs[GGI_MAX_APILEN];
	int err, id;

	_GGI_trueemu_freedbs(vis);

	if ((err = do_dbstuff(vis)) != 0) {
		return err;
	}

	/* load libraries */
	for (id=1; GGI_trueemu_getapi(vis, id, libname, libargs) == 0; id++) {
		err = _ggiOpenDL(vis, _ggiGetConfigHandle(),
				libname, libargs, NULL);
		if (err) {
			fprintf(stderr,"display-tryeeny: Error opening the "
				"%s (%s) library.\n", libname, libargs);
			return GGI_EFATAL;
		}

		DPRINT("Success in loading %s (%s)\n", libname, libargs);
	}

	/* Backup the current 2D operations, and override them with our
	 * own (which then call on these backups to do the work).
	 */
	*priv->mem_opdraw = *vis->opdraw;

	vis->opdraw->drawpixel_nc=GGI_trueemu_drawpixel_nc;
	vis->opdraw->drawpixel=GGI_trueemu_drawpixel;
	vis->opdraw->drawhline_nc=GGI_trueemu_drawhline_nc;
	vis->opdraw->drawhline=GGI_trueemu_drawhline;
	vis->opdraw->drawvline_nc=GGI_trueemu_drawvline_nc;
	vis->opdraw->drawvline=GGI_trueemu_drawvline;
	vis->opdraw->drawline=GGI_trueemu_drawline;

	vis->opdraw->putc=GGI_trueemu_putc;
	vis->opdraw->putpixel_nc=GGI_trueemu_putpixel_nc;
	vis->opdraw->putpixel=GGI_trueemu_putpixel;
	vis->opdraw->puthline=GGI_trueemu_puthline;
	vis->opdraw->putvline=GGI_trueemu_putvline;
	vis->opdraw->putbox=GGI_trueemu_putbox;

	vis->opdraw->drawbox=GGI_trueemu_drawbox;
	vis->opdraw->copybox=GGI_trueemu_copybox;
	vis->opdraw->crossblit=GGI_trueemu_crossblit;
	vis->opdraw->fillscreen=GGI_trueemu_fillscreen;
	vis->opdraw->setorigin=GGI_trueemu_setorigin;

	vis->opdraw->setreadframe=GGI_trueemu_setreadframe;
	vis->opdraw->setwriteframe=GGI_trueemu_setwriteframe;
	vis->opdraw->setdisplayframe=GGI_trueemu_setdisplayframe;

	ggiIndicateChange(vis->stem, GGI_CHG_APILIST);


	/* set initial frames */

	priv->mem_opdraw->setreadframe(vis, 0);
	priv->mem_opdraw->setwriteframe(vis, 0);

	return 0;
}

int GGI_trueemu_setmode(struct ggi_visual *vis, ggi_mode *mode)
{ 
	ggi_trueemu_priv *priv = TRUEEMU_PRIV(vis);
	int err;

	DPRINT_MODE("display-trueemu: setmode %dx%d#%dx%dF%d[0x%02x]\n",
			mode->visible.x, mode->visible.y,
			mode->virt.x, mode->virt.y, 
			mode->frames, mode->graphtype);

	MANSYNC_ignore(vis);

	if ((err = ggiCheckMode(vis->stem, mode)) != 0) {
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
		DPRINT_MODE("display-trueemu: setmode failed (%d).\n", err);
		return err;
	}

	DPRINT_MODE("display-trueemu: Attempting to setmode on parent "
		"visual...\n");

	if ((err = _ggi_trueemu_Open(vis)) != 0) {
		return err;
	}

	MANSYNC_SETFLAGS(vis, LIBGGI_FLAGS(vis));
	MANSYNC_cont(vis);

	DPRINT_MODE("display-trueemu: setmode succeeded.\n");

	return 0;
}

int GGI_trueemu_checkmode(struct ggi_visual *vis, ggi_mode *mode)
{
	ggi_trueemu_priv *priv = TRUEEMU_PRIV(vis);
	ggi_mode par_mode;
	int tmperr, err = 0;
	
	DPRINT_MODE("display-trueemu: checkmode %dx%d#%dx%dF%d[0x%02x]\n",
		       mode->visible.x, mode->visible.y,
		       mode->virt.x, mode->virt.y,
		       mode->frames, mode->graphtype);

	/* Handle graphtype */
	if (GT_SCHEME(mode->graphtype) == GT_AUTO) {
		GT_SETSCHEME(mode->graphtype, GT_TRUECOLOR);
	}

	mode->graphtype = _GGIhandle_gtauto(mode->graphtype);

	if (GT_SCHEME(mode->graphtype) != GT_TRUECOLOR) {
		GT_SETSCHEME(mode->graphtype, GT_TRUECOLOR);
		err = -1;
	}

	if (GT_DEPTH(mode->graphtype) != 24) {
		GT_SETDEPTH(mode->graphtype, 24);
		err = -1;
	}

	if ((GT_SIZE(mode->graphtype) != GT_DEPTH(mode->graphtype)) &&
	    (GT_SIZE(mode->graphtype) != 32)) {
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

	/* Now let the parent target have it's say in the checkmode
	 * process.  It can deal with any remaining GGI_AUTO or GT_AUTO
	 * values that came from priv->mode.
	 */
	par_mode = *mode;

	par_mode.graphtype = priv->mode.graphtype;

	tmperr = ggiCheckMode(priv->parent, &par_mode);
	if (tmperr) err = tmperr;

	mode->visible = par_mode.visible;
	mode->virt    = par_mode.virt;
	mode->dpp     = par_mode.dpp;
	mode->size    = par_mode.size;

	DPRINT_MODE("display-trueemu: upgraded to %dx%d#%dx%dF%d[0x%02x]\n",
			mode->visible.x, mode->visible.y,
			mode->virt.x, mode->virt.y,
			mode->frames, mode->graphtype);

	return err;
}

int GGI_trueemu_getmode(struct ggi_visual *vis, ggi_mode *mode)
{
	if ((vis == NULL) || (mode == NULL) || (LIBGGI_MODE(vis) == NULL)) {
		DPRINT("display-trueemu: vis/mode == NULL\n");
		return GGI_ENOSPACE;
	}
	
	DPRINT("display-trueemu: getmode.\n");

	memcpy(mode, LIBGGI_MODE(vis), sizeof(ggi_mode));

	return 0;
}


int GGI_trueemu_resetmode(struct ggi_visual *vis)
{
	ggi_trueemu_priv *priv = TRUEEMU_PRIV(vis);

	_GGI_trueemu_freedbs(vis);

	_ggi_trueemu_Close(vis);

	if (priv->fb_ptr) {
		free(priv->fb_ptr);
		priv->fb_ptr = NULL;
	}

	return 0;
}


int GGI_trueemu_setflags(struct ggi_visual *vis, uint32_t flags)
{
	LIBGGI_FLAGS(vis) = flags;

	MANSYNC_SETFLAGS(vis, flags);
	LIBGGI_FLAGS(vis) &= GGIFLAG_ASYNC; /* Unkown flags don't take. */

	return 0;
}


int GGI_trueemu_flush(struct ggi_visual *vis, int x, int y, int w, int h, int tryflag)
{
	ggi_trueemu_priv *priv = TRUEEMU_PRIV(vis);
	
	int err;

	MANSYNC_ignore(vis);

	ggLock(priv->flush_lock);

	err = _ggi_trueemu_Flush(vis);

	if (! err) {
		err = _ggiInternFlush(GGI_VISUAL(priv->parent), x, y, w, h, tryflag);
	}

	ggUnlock(priv->flush_lock);

	MANSYNC_cont(vis);

	return err;
}
