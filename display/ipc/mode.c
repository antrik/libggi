/* $Id: mode.c,v 1.24 2008/03/12 10:39:45 cegger Exp $
******************************************************************************

   display-ipc : mode management

   Copyright (C) 1997 Jason McMullan	[jmcc@ggi-project.org]
   Copyright (C) 1998 Steve Cheng	[steve@ggi-project.org]
   Copyright (C) 1998 Andrew Apted	[andrew@ggi-project.org]
   Copyright (C) 2000 Marcus Sundberg	[marcus@ggi-project.org]
   Copyright (C) 2000 Stefan Seefeld	[stefan@berlin-consortium.org]

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
#include <ggi/display/ipc.h>
#include <ggi/internal/ggi_debug.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../common/pixfmt-setup.inc"
#include "../common/ggi-auto.inc"
#include "../common/gt-auto.inc"

static void _GGIfreedbs(struct ggi_visual *vis) 
{
	int i;

	for (i=LIBGGI_APPLIST(vis)->num-1; i >= 0; i--) {
		_ggi_db_free(LIBGGI_APPBUFS(vis)[i]);
		_ggi_db_del_buffer(LIBGGI_APPLIST(vis), i);
	}
}


/*
 * _Attempt_ to get the default framebuffer.. 
 */
static int alloc_fb(struct ggi_visual *vis, ggi_mode *mode)
{
	char *fbaddr;

	_GGIfreedbs(vis);

	fbaddr = IPC_PRIV(vis)->memptr;

	memset(LIBGGI_PIXFMT(vis), 0, sizeof(ggi_pixelformat));
	setup_pixfmt(LIBGGI_PIXFMT(vis), mode->graphtype);
	_ggi_build_pixfmt(LIBGGI_PIXFMT(vis));

	/* Set up directbuffer */
	_ggi_db_add_buffer(LIBGGI_APPLIST(vis), _ggi_db_get_new());
	LIBGGI_APPBUFS(vis)[0]->frame = 0;
	LIBGGI_APPBUFS(vis)[0]->type = GGI_DB_NORMAL | GGI_DB_SIMPLE_PLB;
	LIBGGI_APPBUFS(vis)[0]->read = LIBGGI_APPBUFS(vis)[0]->write = fbaddr;
	LIBGGI_APPBUFS(vis)[0]->layout = blPixelLinearBuffer;
	LIBGGI_APPBUFS(vis)[0]->buffer.plb.stride
		= GT_ByPPP(mode->virt.x, mode->graphtype);
	LIBGGI_APPBUFS(vis)[0]->buffer.plb.pixelformat = LIBGGI_PIXFMT(vis);

	/* Set up palette */
	if (LIBGGI_PAL(vis)->clut.data) {
 		free(LIBGGI_PAL(vis)->clut.data);
 		LIBGGI_PAL(vis)->clut.data = NULL;
	}
	if (GT_SCHEME(LIBGGI_GT(vis)) == GT_PALETTE) {
		LIBGGI_PAL(vis)->clut.size = 1 << GT_DEPTH(LIBGGI_GT(vis));
 		LIBGGI_PAL(vis)->clut.data = _ggi_malloc(LIBGGI_PAL(vis)->clut.size * sizeof(ggi_color));
	}
	
	return 0;
}

int GGI_ipc_getapi(struct ggi_visual *vis,int num, char *apiname ,char *arguments)
{
	ggi_mode *mode = LIBGGI_MODE(vis);

	*arguments = '\0';

	switch(num) { 

	case 0: strcpy(apiname, "display-ipc");
		return 0;

	case 1: strcpy(apiname, "generic-stubs");
		return 0;
		
	case 2: if (GT_SCHEME(LIBGGI_GT(vis)) == GT_TEXT) {
			sprintf(apiname, "generic-text-%u",
				GT_SIZE(mode->graphtype));
			return 0;
		}

		sprintf(apiname, "generic-linear-%u%s", 
			GT_SIZE(LIBGGI_GT(vis)),
			(LIBGGI_GT(vis) & GT_SUB_HIGHBIT_RIGHT) ? "-r" : "");
		return 0;

	case 3: if (GT_SCHEME(LIBGGI_GT(vis)) == GT_TEXT)
			return GGI_ENOMATCH;

		strcpy(apiname, "generic-color");
		return 0;
	}

	return GGI_ENOMATCH;
}

static int _GGIdomode(struct ggi_visual *vis, ggi_mode *mode)
{
	int err, i;
	char	name[GGI_MAX_APILEN];
	char	args[GGI_MAX_APILEN];
	
	DPRINT("display-ipc: _GGIdomode: called\n");

	_ggiZapMode(vis, 0);

	DPRINT("display-ipc: _GGIdomode: zap\n");

	if ((err=alloc_fb(vis,mode)) != 0)
		return err;

	DPRINT("display-ipc: _GGIdomode: got framebuffer memory\n");

	for(i=1; 0==GGI_ipc_getapi(vis, i, name, args); i++) {
		err = _ggiOpenDL(vis, libggi->config, name, args, NULL);
		if (err) {
			fprintf(stderr,"display-ipc: Can't open the "
				"%s (%s) library.\n", name, args);
			return GGI_EFATAL;
		} else {
			DPRINT_LIBS("Success in loading %s (%s)\n",
				name, args);
		}
	}

	if (GT_SCHEME(LIBGGI_GT(vis)) == GT_PALETTE) {
		LIBGGI_PAL(vis)->setPalette = GGI_ipc_setPalette;
	}
	
	return 0;
}

int GGI_ipc_setmode(struct ggi_visual *vis, ggi_mode *mode)
{ 
	int err;
	ipc_priv *priv;

	priv = IPC_PRIV(vis);
	DPRINT("GGIsetmode: called\n");

	APP_ASSERT(vis != NULL, "GGI_ipc_setmode: Visual == NULL");
	
	if ((err=ggiCheckMode(vis->instance.stem, mode)) != 0)	return err;

	/* some elements of the mode setup rely on this. */
	memcpy(LIBGGI_MODE(vis), mode, sizeof(ggi_mode));

	err=_GGIdomode(vis, mode);
	DPRINT("display-ipc: GGIsetmode: domode=%d\n",err);
	if (err)
		return err;

	ggiIndicateChange(vis->instance.stem, GGI_CHG_APILIST);
	DPRINT("GGIsetmode: change indicated\n",err);

	return 0;
}

int GGI_ipc_checkmode(struct ggi_visual *vis, ggi_mode *mode)
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

	if (err) return err;
	err = _ggi_physz_figure_size(mode, IPC_PRIV(vis)->physzflags,
				&(IPC_PRIV(vis)->physz),
				0, 0, mode->visible.x, mode->visible.y);

	return err;	
}

int GGI_ipc_getmode(struct ggi_visual *vis, ggi_mode *mode)
{
	ggi_mode mymode;
	ipc_priv *priv;

	priv = IPC_PRIV(vis);
	DPRINT("GGIgetmode(%p,%p)\n", vis, mode);

	memcpy(&mymode, LIBGGI_MODE(vis), sizeof(ggi_mode));
#if 0
	if (priv->inputbuffer) {
		mymode.visible.x = priv->inputbuffer->visx;
		mymode.visible.y = priv->inputbuffer->visy;
		mymode.virt.x    = priv->inputbuffer->virtx;
		mymode.virt.y    = priv->inputbuffer->virty;
		mymode.frames    = priv->inputbuffer->frames;
		mymode.graphtype = priv->inputbuffer->type;
	}
#endif
	memcpy(mode, &mymode, sizeof(ggi_mode));

	return 0;
}

int _GGI_ipc_resetmode(struct ggi_visual *vis)
{
	DPRINT("GGIresetmode(%p)\n", vis);

	_GGIfreedbs(vis);

	return 0;
}

int GGI_ipc_setflags(struct ggi_visual *vis, uint32_t flags)
{
	LIBGGI_FLAGS(vis)=flags;

	LIBGGI_FLAGS(vis) &= GGIFLAG_ASYNC; /* Unkown flags don't take. */

	return 0;
}

int GGI_ipc_setPalette(struct ggi_visual *vis, size_t start, size_t size,
			const ggi_color *colormap)
{
	DPRINT("ipc setpalette.\n");
	              
	memcpy(LIBGGI_PAL(vis)->clut.data+start, colormap,
		size*sizeof(ggi_color));
		
	return 0;
}
