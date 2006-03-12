/* $Id: mode.c,v 1.27 2006/03/12 08:42:23 cegger Exp $
******************************************************************************

   Display memory : mode management

   Copyright (C) 1997 Jason McMullan	[jmcc@ggi-project.org]
   Copyright (C) 1998 Steve Cheng	[steve@ggi-project.org]
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
#include <ggi/display/memory.h>
#include <ggi/internal/ggi_debug.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../common/pixfmt-setup.inc"
#include "../common/ggi-auto.inc"
#include "../common/gt-auto.inc"

/* Used to prevent ggiSetMode core code from destroying data
 * by calling fillscreen when -noblank option is specified.
 */
static int _strawman_fillscreen(struct ggi_visual *vis) {
	if (vis->w_frame_num == LIBGGI_MODE(vis)->frames - 1) 
		vis->opdraw->fillscreen = MEMORY_PRIV(vis)->oldfillscreen;
	return GGI_OK;
}

static int _dummy_setdisplayframe(struct ggi_visual *vis, int frame) {
	return GGI_OK;
}

static void _GGIfreedbs(struct ggi_visual *vis) 
{
	int i;

	for (i=LIBGGI_APPLIST(vis)->num-1; i >= 0; i--) {
		if ((0==i) && (MEMORY_PRIV(vis)->memtype==MT_MALLOC))
			free(LIBGGI_APPBUFS(vis)[0]->write);
		_ggi_db_free(LIBGGI_APPBUFS(vis)[i]);
		_ggi_db_del_buffer(LIBGGI_APPLIST(vis), i);
	}
}


/*
 * _Attempt_ to get the default framebuffer.. 
 */
static int alloc_fb(struct ggi_visual *vis, ggi_mode *mode)
{
	int i;
	char *fbaddr;
	ggi_memory_priv *priv;
	int fstride, lstride, pstride;

	priv = MEMORY_PRIV(vis);

	pstride = 0; /* Silence, GCC. */

	if (priv->layout == blPixelPlanarBuffer) {
		lstride = priv->buffer.plan.next_line ?
			priv->buffer.plan.next_line : 
			(mode->virt.x + 7) / 8;
		pstride = priv->buffer.plan.next_plane ?
			priv->buffer.plan.next_plane : 
			(lstride * mode->virt.y);
		if (pstride > lstride)
			fstride = (pstride * GT_DEPTH(mode->graphtype));
		else 
			fstride = lstride * mode->virt.y;
		fstride = priv->fstride ? priv->fstride : fstride;
	}
	else {
		lstride = priv->buffer.plb.stride ?
			priv->buffer.plb.stride : 
			(int)GT_ByPPP(mode->virt.x, mode->graphtype);
		fstride = priv->fstride ? 
			priv->fstride : (lstride * mode->virt.y);
	}

	_GGIfreedbs(vis);

	if (priv->memtype==MT_MALLOC) {
		fbaddr = malloc((size_t)(fstride * mode->frames));
		if (! fbaddr) {
			DPRINT("Out of memory!");
			return GGI_ENOMEM;
		}
	} else {
		fbaddr = priv->memptr;
	}

	memset(LIBGGI_PIXFMT(vis), 0, sizeof(ggi_pixelformat));
	setup_pixfmt(LIBGGI_PIXFMT(vis), mode->graphtype);
	if (priv->r_mask && priv->g_mask && priv->b_mask) {
		LIBGGI_PIXFMT(vis)->red_mask   = priv->r_mask;
		LIBGGI_PIXFMT(vis)->green_mask = priv->g_mask;
		LIBGGI_PIXFMT(vis)->blue_mask  = priv->b_mask;
		LIBGGI_PIXFMT(vis)->alpha_mask  = priv->a_mask;
	}
	_ggi_build_pixfmt(LIBGGI_PIXFMT(vis));

	/* Set up directbuffer */
	if (priv->layout == blPixelLinearBuffer) {
		for (i = 0; i < mode->frames; i++) {
			ggi_directbuffer *buf;
			_ggi_db_add_buffer(LIBGGI_APPLIST(vis),
					   _ggi_db_get_new());
			buf = LIBGGI_APPBUFS(vis)[i];
			buf->frame = i;
			buf->type = GGI_DB_NORMAL | GGI_DB_SIMPLE_PLB;
			buf->read = buf->write = fbaddr + fstride * i;
			buf->layout = blPixelLinearBuffer;
			buf->buffer.plb.stride = lstride;
			buf->buffer.plb.pixelformat = LIBGGI_PIXFMT(vis);
		}
	}
	else {
		for (i = 0; i < mode->frames; i++) {
			ggi_directbuffer *buf;
			_ggi_db_add_buffer(LIBGGI_APPLIST(vis),
					   _ggi_db_get_new());
			buf = LIBGGI_APPBUFS(vis)[i];
			buf->frame = i;
			buf->type = GGI_DB_NORMAL;
			buf->read = buf->write = fbaddr + fstride * i;
			buf->layout = blPixelPlanarBuffer;
			buf->buffer.plan.next_line = lstride;
			buf->buffer.plan.next_plane = pstride;
			buf->buffer.plan.pixelformat = LIBGGI_PIXFMT(vis);
		}
	}
	LIBGGI_APPLIST(vis)->first_targetbuf
        	= LIBGGI_APPLIST(vis)->last_targetbuf - (mode->frames-1);

	/* Set up palette */
	if (LIBGGI_PAL(vis)->clut.data) {
		free(LIBGGI_PAL(vis)->clut.data);
		LIBGGI_PAL(vis)->clut.data = NULL;
	}
	if (GT_SCHEME(LIBGGI_GT(vis)) == GT_PALETTE) {
		LIBGGI_PAL(vis)->clut.data = _ggi_malloc((1 << GT_DEPTH(LIBGGI_GT(vis)))*
					   	sizeof(ggi_color));
		LIBGGI_PAL(vis)->clut.size     = 1 << GT_DEPTH(LIBGGI_GT(vis));
	}

	return 0;
}

int GGI_memory_getapi(struct ggi_visual *vis,int num, char *apiname ,char *arguments)
{
	ggi_mode *mode = LIBGGI_MODE(vis);

	*arguments = '\0';

	switch(num) { 

	case 0: strcpy(apiname, "display-memory");
		return 0;

	case 1: strcpy(apiname, "generic-stubs");
		return 0;

	case 2: if (GT_SCHEME(LIBGGI_GT(vis)) == GT_TEXT)
			return GGI_ENOMATCH;

		strcpy(apiname, "generic-color");
		return 0;

	case 3: if (GT_SCHEME(LIBGGI_GT(vis)) == GT_TEXT) {
			sprintf(apiname, "generic-text-%u",
				GT_SIZE(mode->graphtype));
			return 0;
		}
		if (MEMORY_PRIV(vis)->layout == blPixelPlanarBuffer) {
			sprintf(apiname, "generic-planar");
			return 0;
		}
		sprintf(apiname, "generic-linear-%u%s", 
			GT_SIZE(LIBGGI_GT(vis)),
			(LIBGGI_GT(vis) & GT_SUB_HIGHBIT_RIGHT) ? "-r" : "");
		return 0;

	}

	return GGI_ENOMATCH;
}

static int _GGIdomode(struct ggi_visual *vis, ggi_mode *mode)
{
	int err, i;
	char	name[GGI_MAX_APILEN];
	char	args[GGI_MAX_APILEN];
	
	DPRINT("display-memory: _GGIdomode: called\n");

	_ggiZapMode(vis, 0);

	DPRINT("display-memory: _GGIdomode: zap\n");

	if ((err=alloc_fb(vis,mode)) != 0)
		return err;

	DPRINT("display-memory: _GGIdomode: got framebuffer memory\n");

	for(i=1; 0==GGI_memory_getapi(vis, i, name, args); i++) {
		err = _ggiOpenDL(vis, _ggiGetConfigHandle(),
				name, args, NULL);
		if (err) {
			fprintf(stderr,"display-memory: Can't open the "
				"%s (%s) library.\n", name, args);
			return GGI_EFATAL;
		} else {
			DPRINT_LIBS("Success in loading %s (%s)\n",
				name, args);
		}
	}

	if (GT_SCHEME(LIBGGI_GT(vis)) == GT_PALETTE) {
		LIBGGI_PAL(vis)->setPalette = GGI_memory_setPalette;
	}

	vis->opgc->gcchanged = NULL; /* kick _default_error off hook. */
	vis->opdraw->setdisplayframe	= _dummy_setdisplayframe;
	vis->opdraw->setreadframe	= _ggi_default_setreadframe;
	vis->opdraw->setwriteframe	= _ggi_default_setwriteframe;

	if (MEMORY_PRIV(vis)->noblank) {
		MEMORY_PRIV(vis)->oldfillscreen = vis->opdraw->fillscreen;
		vis->opdraw->fillscreen = _strawman_fillscreen;
	}
	
	return 0;
}

int GGI_memory_setmode(struct ggi_visual *vis, ggi_mode *mode)
{ 
	int err;
	ggi_memory_priv *priv;

	priv = MEMORY_PRIV(vis);

	DPRINT("display-memory: GGIsetmode: called\n");

	APP_ASSERT(vis != NULL, "GGI_memory_setmode: Visual == NULL");
	
	err = ggiCheckMode(vis->stem, mode);
	if (err < 0) {
		DPRINT("GGI_memory_setmode: ggiCheckMode() failed with error %i\n", err);
		return err;
	}

	/* some elements of the mode setup rely on this. */
	memcpy(LIBGGI_MODE(vis), mode, sizeof(ggi_mode));

	err = _GGIdomode(vis, mode);
	DPRINT("display-memory: GGIsetmode: domode=%d\n",err);
	if (err)
		return err;

	if (priv->inputbuffer) {
		priv->inputbuffer->visx=mode->visible.x;
		priv->inputbuffer->visy=mode->visible.y;
		priv->inputbuffer->virtx=mode->virt.x;
		priv->inputbuffer->virty=mode->virt.y;
		priv->inputbuffer->frames=mode->frames;
		priv->inputbuffer->type=mode->graphtype;
		priv->inputbuffer->visframe=0;
	}

	ggiIndicateChange(vis->stem, GGI_CHG_APILIST);
	DPRINT("display-memory:GGIsetmode: change indicated\n",err);

	return 0;
}

int GGI_memory_checkmode(struct ggi_visual *vis, ggi_mode *mode)
{
	int err = 0;

	/* handle AUTO */
	_GGIhandle_ggiauto(mode, 640, 400);

	mode->graphtype = _GGIhandle_gtauto(mode->graphtype);

	/* do some checks */
	if (GT_SIZE(mode->graphtype) < 8) {
	
		int align = 8 / GT_SIZE(mode->graphtype);

		if (mode->visible.x % align != 0) {
			mode->visible.x += align-(mode->visible.x % align);
			err = -1;
		}
		
		if (mode->virt.x % align != 0) {
			mode->virt.x += align-(mode->virt.x % align);
			err = -1;
		}
	}

	if (mode->virt.x < mode->visible.x) {
		mode->virt.x = mode->visible.x;
		err = -1;
	}

	if (mode->virt.y < mode->visible.y) {
		mode->virt.y = mode->visible.y;
		err = -1;
	}

	if (mode->frames < 1) {
		err = -1;
		mode->frames = 1;
	}

	if ((mode->dpp.x != 1 && mode->dpp.x != GGI_AUTO) ||
	    (mode->dpp.y != 1 && mode->dpp.y != GGI_AUTO)) {
		err = -1;
	}
	mode->dpp.x = mode->dpp.y = 1;

	if (err) return err;
	err = _ggi_physz_figure_size(mode, MEMORY_PRIV(vis)->physzflags, 
				&(MEMORY_PRIV(vis)->physz),
				0, 0, mode->visible.x, mode->visible.y);

	return err;	
}

int GGI_memory_getmode(struct ggi_visual *vis, ggi_mode *mode)
{
	ggi_memory_priv *priv;
	ggi_mode mymode;
	DPRINT("display-memory: GGIgetmode(%p,%p)\n", vis, mode);

	priv = MEMORY_PRIV(vis);

	memcpy(&mymode, LIBGGI_MODE(vis), sizeof(ggi_mode));
	if (priv->inputbuffer) {
		mymode.visible.x = priv->inputbuffer->visx;
		mymode.visible.y = priv->inputbuffer->visy;
		mymode.virt.x    = priv->inputbuffer->virtx;
		mymode.virt.y    = priv->inputbuffer->virty;
		mymode.frames    = priv->inputbuffer->frames;
		mymode.graphtype = priv->inputbuffer->type;
	}
	memcpy(mode, &mymode, sizeof(ggi_mode));

	return 0;
}

int _GGI_memory_resetmode(struct ggi_visual *vis)
{
	DPRINT("display-memory: GGIresetmode(%p)\n", vis);

	_GGIfreedbs(vis);

	return 0;
}

int GGI_memory_setflags(struct ggi_visual *vis,ggi_flags flags)
{
	LIBGGI_FLAGS(vis)=flags;
	LIBGGI_FLAGS(vis) &= GGIFLAG_ASYNC; /* Unkown flags don't take. */
	return 0;
}

int GGI_memory_setPalette(ggi_visual_t v,size_t start,size_t size,const ggi_color *colormap)
{
	struct ggi_visual *vis;
	DPRINT("memory setpalette.\n");

	vis = GGI_VISUAL(v);	                      
	memcpy(LIBGGI_PAL(vis)->clut.data+start, colormap, size*sizeof(ggi_color)); 
	
	return 0;
}
