/* $Id: frames.c,v 1.10 2007/04/24 22:12:02 pekberg Exp $
******************************************************************************

   Tile target: frame handling functions

   Copyright (C) 1998 Steve Cheng    [steve@ggi-project.org]
   Copyright (C) 1998 Andrew Apted   [andrew@ggi-project.org]

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
#include <ggi/display/tile.h>
#include <ggi/internal/ggi_debug.h>


int GGI_tile_setdisplayframe_db(struct ggi_visual *vis, int num)
{
	ggi_directbuffer *db;

	DPRINT_MISC("GGI_tile_setdisplayframe_db(%p, %i) entered\n",
			(void *)vis, num);
	db = _ggi_db_find_frame(vis, num);

	if (db == NULL) {
		DPRINT_MISC("GGI_tile_setdisplayframe_db: no frame found\n");
		return GGI_ENOSPACE;
	}

	vis->d_frame_num = num;
	TILE_PRIV(vis)->d_frame = db;

	DPRINT_MISC("GGI_tile_setdisplayframe_db: leaving\n");
	return 0;
}

int GGI_tile_setdisplayframe(struct ggi_visual *vis, int num)
{
	ggi_tile_priv *priv = TILE_PRIV(vis);
	struct multi_vis *elm;
	int rc;

	tile_FOREACH(priv, elm) {
		rc = ggiSetDisplayFrame(elm->vis, num);
		if (rc < 0) return rc;
	}

	vis->d_frame_num = num;
	return 0;
}

int GGI_tile_setreadframe(struct ggi_visual *vis, int num)
{
	ggi_tile_priv *priv = TILE_PRIV(vis);
	struct multi_vis *elm;
	int rc;

	tile_FOREACH(priv, elm) {
		rc = ggiSetReadFrame(elm->vis, num);
		if (rc < 0) return rc;
	}

	vis->r_frame_num = num;
	return 0;
}

int GGI_tile_setwriteframe(struct ggi_visual *vis, int num)
{
	ggi_tile_priv *priv = TILE_PRIV(vis);
	struct multi_vis *elm;
	int rc;

	tile_FOREACH(priv, elm) {
		rc = ggiSetWriteFrame(elm->vis, num);
		if (rc < 0) return rc;
	}

	vis->w_frame_num = num;
	return 0;
}

