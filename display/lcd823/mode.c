/* $Id: mode.c,v 1.1 2001/05/12 23:02:08 cegger Exp $
******************************************************************************

   Display-lcd823

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

#include <ggi/display/lcd823.h>

#include "../common/ggi-auto.inc"


#define XRES	640
#define YRES	480


void _GGI_lcd823_free_dbs(ggi_visual *vis) 
{
	int first = LIBGGI_APPLIST(vis)->first_targetbuf;
	int last = LIBGGI_APPLIST(vis)->last_targetbuf;
	int i;

	if (first < 0) return;

	for (i = (last - first); i >= 0; i--) {
		_ggi_db_free(LIBGGI_APPLIST(vis)->bufs[i+first]);
		_ggi_db_del_buffer(LIBGGI_APPLIST(vis), i+first);
	}
	LIBGGI_APPLIST(vis)->first_targetbuf = -1;
}


int GGI_lcd823_getapi(ggi_visual *vis, int num, char *apiname, char *arguments)
{
	int size = GT_SIZE(LIBGGI_GT(vis));

	strcpy(arguments, "");

	switch(num) {
	case 0: strcpy(apiname, "display-lcd823");
		return 0;

	case 1: strcpy(apiname, "generic-stubs");
		return 0;

	case 2: strcpy(apiname, "generic-color");
		return 0;

	case 3:
		if (GT_SCHEME(LIBGGI_GT(vis)) == GT_TEXT) {
			sprintf(apiname, "generic-text-%d", size);
			return 0;
		}

		sprintf(apiname, "generic-linear-%d", size);
		return 0;
	}

	return -1;
}


static int do_mmap(ggi_visual *vis)
{
	ggi_lcd823_priv *priv = LIBGGI_PRIVATE(vis);
	ggi_graphtype gt = LIBGGI_GT(vis);
	int xres_in_bytes = XRES;
	int i;

	/* Clear framebuffer */
	memset(priv->fb_ptr, 0, priv->fb_size);

	/* Set up pixel format */
	memset(LIBGGI_PIXFMT(vis), 0, sizeof(ggi_pixelformat));
	LIBGGI_PIXFMT(vis)->size  = GT_SIZE(gt);
	LIBGGI_PIXFMT(vis)->depth = GT_DEPTH(gt);
	LIBGGI_PIXFMT(vis)->clut_mask = (1 << GT_DEPTH(gt)) - 1;
	_ggi_build_pixfmt(LIBGGI_PIXFMT(vis));

	/* Set up DirectBuffers */
	for (i=0; i < LIBGGI_MODE(vis)->frames; i++) {
		ggi_directbuffer *buf;

		_ggi_db_add_buffer(LIBGGI_APPLIST(vis), _ggi_db_get_new());

		buf = LIBGGI_APPBUFS(vis)[i];

		buf->frame = i;
		buf->type  = GGI_DB_NORMAL;
		buf->read  = (uint8 *) priv->fb_ptr + i * priv->frame_size;
		buf->write = buf->read;

		buf->type  |= GGI_DB_SIMPLE_PLB;
		buf->layout = blPixelLinearBuffer;
		buf->buffer.plb.stride = xres_in_bytes;
		buf->buffer.plb.pixelformat = LIBGGI_PIXFMT(vis);
	}

	return 0;
}


static int do_setmode(ggi_visual *vis)
{
	char libname[256], libargs[256];
	ggi_graphtype gt;
	int err, id;

	err = do_mmap(vis); 
	if (err) return err;

	_ggiZapMode(vis, 0);
	for (id=1; GGI_lcd823_getapi(vis, id, libname, libargs) == 0; id++) {
		if (_ggiOpenDL(vis, libname, libargs, NULL) != 0) {
			fprintf(stderr,"display-lcd823: Error opening the "
				"%s (%s) library.\n", libname, libargs);
			return GGI_EFATAL;
		}

		GGIDPRINT_LIBS("Success in loading %s (%s)\n",
			       libname, libargs);
	}

	/* Set up palette */
	gt = LIBGGI_GT(vis);
	if ((GT_SCHEME(gt) == GT_PALETTE) || (GT_SCHEME(gt) == GT_TEXT)) {
	    	int nocols = 1 << GT_DEPTH(gt);

		vis->palette = _ggi_malloc(nocols * sizeof(ggi_color));
		vis->opcolor->setpalvec = GGI_lcd823_setpalvec;
		/* Initialize palette */
		ggiSetColorfulPalette(vis);
	}

	ggiIndicateChange(vis, GGI_CHG_APILIST);

	GGIDPRINT_MODE("display-lcd823: do_setmode SUCCESS\n");

	return 0;
}


int GGI_lcd823_setmode(ggi_visual *vis, ggi_mode *mode)
{ 
	int err;

        if ((err = ggiCheckMode(vis, mode)) != 0) {
		return err;
	}

	if (vis->palette) {
		free(vis->palette);
		vis->palette = NULL;
	}

	_GGI_lcd823_free_dbs(vis);

	memcpy(LIBGGI_MODE(vis), mode, sizeof(ggi_mode));

	/* Now actually set the mode */
	err = do_setmode(vis);
	if (err != 0) {
		return err;
	}

	/* Reset panning and frames */
        vis->d_frame_num = vis->origin_x = vis->origin_y = 0;

	return 0;
}


int GGI_lcd823_checkmode(ggi_visual *vis, ggi_mode *mode)
{
	int err = 0;

	GGIDPRINT_MODE("display-lcd823: checkmode %dx%d#%dx%dF%d[0x%02x]\n",
			mode->visible.x, mode->visible.y,
			mode->virt.x, mode->virt.y, 
			mode->frames, mode->graphtype);

	_GGIhandle_ggiauto(mode, XRES, YRES);

	if (mode->graphtype == GT_AUTO) {
		mode->graphtype = GT_8BIT;
	}

	if (mode->graphtype != GT_8BIT) {
		mode->graphtype = GT_8BIT;
		err = -1;
	}
	if (mode->virt.x != mode->visible.x || mode->visible.x != XRES) {
		mode->virt.x = mode->visible.x = XRES;
		err = -1;
	}
	if (mode->virt.y != mode->visible.y || mode->visible.y != YRES) {
		mode->virt.y = mode->visible.y = YRES;
		err = -1;
	}

	if (mode->frames != 1 && mode->frames != GGI_AUTO) {
		err = -1;
	}
	mode->frames = 1;

	if ((tm->dpp.x != 1 && tm->dpp.x != GGI_AUTO) ||
	    (tm->dpp.y != 1 && tm->dpp.y != GGI_AUTO)) {
		err = -1;
	}
	tm->dpp.x = tm->dpp.y = 1;

	if (tm->size.x != GGI_AUTO || tm->size.y != GGI_AUTO) {
		err = -1;
	}
	tm->size.x = tm->size.y = GGI_AUTO;

	GGIDPRINT_MODE("display-lcd823: result %d %dx%d#%dx%dF%d[0x%02x]\n",
		       err, mode->visible.x, mode->visible.y,
		       mode->virt.x, mode->virt.y, 
		       mode->frames, mode->graphtype);

	return err;
}


int GGI_lcd823_getmode(ggi_visual *vis, ggi_mode *mode)
{
	GGIDPRINT_MODE("display-lcd823: getmode\n");

	memcpy(mode, LIBGGI_MODE(vis), sizeof(ggi_mode));

	return 0;
}


int GGI_lcd823_setflags(ggi_visual *vis, ggi_flags flags)
{
	LIBGGI_FLAGS(vis) = flags;

	return 0;
}
