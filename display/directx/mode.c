/* $Id: mode.c,v 1.6 2003/10/10 05:35:07 cegger Exp $
*****************************************************************************

   LibGGI DirectX target - Mode management

   Copyright (C) 1999-2000 John Fortin	[fortinj@ibm.net]

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

#include <string.h>

#include "config.h"
#include <ggi/internal/ggi-dl.h>
#include <ggi/display/directx.h>

#include "ddinit.h"

#include "../common/pixfmt-setup.inc"
#include "../common/ggi-auto.inc"

static int
directx_acquire(ggi_resource * res, uint32 actype)
{
	ggi_directbuffer *dbuf;
	ggi_visual *vis;
	directx_priv *priv;
	int bufnum;

	GGIDPRINT_MISC("directx_acquire(%p, 0x%x) called\n", res, actype);

	if (actype & ~(GGI_ACTYPE_READ | GGI_ACTYPE_WRITE)) {
		return GGI_EARGINVAL;
	}
	vis = res->priv;
	priv = LIBGGI_PRIVATE(vis);
	dbuf = res->self;
	bufnum = dbuf->frame;

	if (res->count > 0) {
		if (dbuf->write != NULL) {
			dbuf->read = dbuf->write;
		} else if (dbuf->read != NULL) {
			dbuf->write = dbuf->read;
		}
		res->count++;
		return 0;
	}
	ggLock(priv->lock);
	dbuf->write = priv->lpSurfaceAdd;
	dbuf->read = dbuf->write;
	ggUnlock(priv->lock);

	res->curactype |= actype;
	res->count++;

	GGIDPRINT_MISC("directx_acquire - success, count: %d\n", res->count);

	return 0;
}

static int
directx_release(ggi_resource * res)
{
	GGIDPRINT_MISC("directx_release(%p) called\n", res);

	if (res->count < 1)
		return GGI_ENOTALLOC;

	res->count--;

	return 0;
}

int GGI_directx_flush(ggi_visual * vis, int x, int y, int w, int h, int tryflag)
{
	directx_priv *priv = LIBGGI_PRIVATE(vis);
	ggLock(priv->lock);
	DDRedraw(priv);
	ggUnlock(priv->lock);
	return 0;
}

int GGI_directx_getapi(ggi_visual * vis, int num, char *apiname, char *arguments)
{
	switch (num) {
	case 0:
		strcpy(apiname, "display-directx");
		strcpy(arguments, "");
		return 0;
	case 1:
		strcpy(apiname, "generic-stubs");
		strcpy(arguments, "");
		return 0;
	case 2:
		strcpy(apiname, "generic-color");
		strcpy(arguments, "");
		return 0;
	case 3:
		sprintf(apiname, "generic-linear-%d", GT_SIZE(LIBGGI_MODE(vis)->graphtype));
		strcpy(arguments, "");
		return 0;
	}

	return -1;
}


static void GetScreenParams(int *depth, int *width, int *height)
{
	HWND wnd = GetDesktopWindow();
	HDC dc = GetDC(wnd);
	*depth = GetDeviceCaps(dc, BITSPIXEL);
	*width = GetDeviceCaps(dc, HORZRES);
	*height = GetDeviceCaps(dc, VERTRES);
	ReleaseDC(wnd, dc);
}



int GGI_directx_checkmode(ggi_visual * vis, ggi_mode * mode)
{
	directx_priv *priv = LIBGGI_PRIVATE(vis);
	uint8 i;
	uint8 err = 0;
	int depth, width, height, defwidth, defheight;
	ggi_graphtype deftype;

	GetScreenParams(&depth, &width, &height);
	defwidth = width * 9 / 10;
	defheight = height * 9 / 10;

	/* handle AUTO */
	_GGIhandle_ggiauto(mode, defwidth, defheight);

	if (mode->frames < 1) {
		err = -1;
		mode->frames = 1;
	} else if (mode->frames > 2) {
		err = -1;
		mode->frames = 1;
	}

	switch (depth) {
	case 1: 
		deftype = GT_1BIT; 
		break;
	case 2: 
		deftype = GT_2BIT;  
		break;
	case 4: 
		deftype = GT_4BIT;
		break;   
	case 8:
		deftype = GT_8BIT;
		break;
	case 15:
		deftype = GT_15BIT;
		break;
	case 16:
		deftype = GT_16BIT;
		break;
	case 24:
		deftype = GT_24BIT;
		break;
	case 32:
		deftype = GT_32BIT;  
		break;
	default:
		deftype = GT_AUTO;
		err = -1;
		break;
	}

	if (GT_DEPTH(mode->graphtype) == GT_AUTO) {
		mode->graphtype = deftype;
	}

	if (!(mode->visible.x > 0
	   && mode->visible.y > 0
	   && mode->visible.x <= width
	   && mode->visible.y <= height
	   && GT_SIZE(mode->graphtype) == depth))
	{
		mode->visible.x = defwidth;
		mode->visible.y = defheight;
		mode->graphtype = deftype;
		err = -1;
	}

	if (mode->virt.x != mode->visible.x) {
		mode->virt.x = mode->visible.x;
		err = -1;
	}       
	if (mode->virt.y != mode->visible.y) {
		mode->virt.y = mode->visible.y;
		err = -1;
	}

	if ((mode->dpp.x != 1 && mode->dpp.x != GGI_AUTO) ||
	    (mode->dpp.y != 1 && mode->dpp.y != GGI_AUTO))
	{
		err = -1;
	}
	mode->dpp.x = mode->dpp.y = 1;

	if (err) return err;

	ggLock(priv->lock);
	err = _ggi_figure_physz(mode, priv->physzflags,
				&priv->physz,
				0, 0, mode->visible.x, mode->visible.y);
	ggUnlock(priv->lock);
                
	return err;
}




int GGI_directx_setmode(ggi_visual * vis, ggi_mode * mode)
{
	directx_priv *priv = LIBGGI_PRIVATE(vis);
	int i, id, ret;
	char libname[GGI_MAX_APILEN], libargs[GGI_MAX_APILEN];

	ret = ggiCheckMode(vis, mode);
	if (ret != 0) {
		return -1;
	}

	ggLock(priv->lock);

	/* Fill in ggi_pixelformat */
	memset(LIBGGI_PIXFMT(vis), 0, sizeof(ggi_pixelformat));
	setup_pixfmt(LIBGGI_PIXFMT(vis), mode->graphtype);

	priv->BPP = mode->graphtype / 8;

	_ggi_build_pixfmt(LIBGGI_PIXFMT(vis));

	DDChangeMode(priv, mode->visible.x, mode->visible.y, priv->BPP * 8);

	mode->virt.x = mode->visible.x;
	mode->virt.y = mode->visible.y;

	vis->d_frame_num = 0;
	vis->r_frame_num = 0;
	vis->w_frame_num = 0;

	/* Set Up Direct Buffers */

	for (i = 0; i < 1; i++) {	/* Fix-me for multi frames */
		ggi_resource *res;

		res = malloc(sizeof(ggi_resource));
		if (res == NULL) {
			ggUnlock(priv->lock);
			return GGI_EFATAL;
		}
		LIBGGI_APPLIST(vis)->last_targetbuf
		    = _ggi_db_add_buffer(LIBGGI_APPLIST(vis),
					 _ggi_db_get_new());
		LIBGGI_APPBUFS(vis)[i]->resource = res;
		LIBGGI_APPBUFS(vis)[i]->resource->acquire = directx_acquire;
		LIBGGI_APPBUFS(vis)[i]->resource->release = directx_release;
		LIBGGI_APPBUFS(vis)[i]->resource->self = LIBGGI_APPBUFS(vis)[i];
		LIBGGI_APPBUFS(vis)[i]->resource->priv = vis;
		LIBGGI_APPBUFS(vis)[i]->resource->count = 0;
		LIBGGI_APPBUFS(vis)[i]->resource->curactype = 0;
		LIBGGI_APPBUFS(vis)[i]->frame = i;
		LIBGGI_APPBUFS(vis)[i]->type = GGI_DB_NORMAL | GGI_DB_SIMPLE_PLB;
		LIBGGI_APPBUFS(vis)[i]->read = LIBGGI_APPBUFS(vis)[i]->write = NULL;
		LIBGGI_APPBUFS(vis)[i]->layout = blPixelLinearBuffer;
		LIBGGI_APPBUFS(vis)[i]->buffer.plb.stride = priv->pitch;
		LIBGGI_APPBUFS(vis)[i]->buffer.plb.pixelformat = LIBGGI_PIXFMT(vis);

		GGIDPRINT_MODE("DB: %d, addr: %p, stride: %d\n", i,
			       LIBGGI_APPBUFS(vis)[i]->read,
			       LIBGGI_APPBUFS(vis)[i]->buffer.plb.stride);
	}

	vis->r_frame = LIBGGI_APPBUFS(vis)[0];
	vis->w_frame = LIBGGI_APPBUFS(vis)[0];

	LIBGGI_CURWRITE(vis) = priv->lpSurfaceAdd;
	LIBGGI_CURREAD(vis) = priv->lpSurfaceAdd;

	LIBGGI_APPLIST(vis)->first_targetbuf
	    = LIBGGI_APPLIST(vis)->last_targetbuf - (mode->frames - 1);

	memcpy(LIBGGI_MODE(vis), mode, sizeof(ggi_mode));

	ggUnlock(priv->lock);

	for (id = 1; GGI_directx_getapi(vis, id, libname, libargs) == 0; id++) {
		if (_ggiOpenDL(vis, libname, libargs, NULL) != 0) {
			fprintf(stderr, "display-directx: Error opening the "
				"%s (%s) library\n", libname, libargs);
			return -1;
		}
		GGIDPRINT("Success in loading %s (%s)\n", libname, libargs);
	}

	ggiIndicateChange(vis, GGI_CHG_APILIST);

	return 0;
}


int GGI_directx_getmode(ggi_visual * vis, ggi_mode * tm)
{
	LIBGGI_APPASSERT(vis != NULL, "directx: GGIgetmode: Visual == NULL");

	/* We assume the mode in the visual to be OK */
	memcpy(tm, LIBGGI_MODE(vis), sizeof(ggi_mode));

	return 0;
}
