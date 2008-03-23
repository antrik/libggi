/* $Id: dbs.inc,v 1.3 2008/03/23 14:15:00 cegger Exp $
******************************************************************************

   LibGGI Display-X target: directbuffer functions shared with helper-x-*

   Copyright (C) 2008      Christoph Egger

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

/* Note, this file is not build standalone. It must be included
 * in a C file
 */

static void
_ggi_free_dbs(struct ggi_visual *vis)
{
	int i, first, last;

	DPRINT_MODE("_ggi_free_dbs(%p) called\n", vis);

	first = LIBGGI_APPLIST(vis)->first_targetbuf;
	last = LIBGGI_APPLIST(vis)->last_targetbuf;
	if (first < 0) {
		return;
	}
	for (i = (last - first); i >= 0; i--) {
		free(LIBGGI_APPBUFS(vis)[i]->resource);
		_ggi_db_free(LIBGGI_APPLIST(vis)->bufs[i + first]);
		_ggi_db_del_buffer(LIBGGI_APPLIST(vis), i + first);
	}
	LIBGGI_APPLIST(vis)->first_targetbuf = -1;
}

static int
_ggi_create_dbs(struct ggi_visual *vis)
{
	int i, err = GGI_OK;
	ggi_mode *mode;
	ggi_directbuffer *db;
	ggi_x_priv *priv = GGIX_PRIV(vis);

	DPRINT_MODE("_ggi_create_dbs(%p) called\n", vis);

	mode = LIBGGI_MODE(vis);
	LIBGGI_APPLIST(vis)->first_targetbuf = -1;
	for (i = 0; i < mode->frames; i++) {
		db = _ggi_db_get_new();
		if (!db) {
			DPRINT("_ggi_create_dbs: frame %u allocation failed.\n", i);
			err = GGI_ENOMEM;
			goto err0;
		}
		LIBGGI_APPLIST(vis)->last_targetbuf
			= _ggi_db_add_buffer(LIBGGI_APPLIST(vis), db);
		LIBGGI_APPBUFS(vis)[i]->frame = i;
		LIBGGI_APPBUFS(vis)[i]->type
			= GGI_DB_NORMAL | GGI_DB_SIMPLE_PLB;
		LIBGGI_APPBUFS(vis)[i]->read = LIBGGI_APPBUFS(vis)[i]->write =
			priv->fb + i * LIBGGI_VIRTY(vis) * 
			priv->ximage->bytes_per_line;
		LIBGGI_APPBUFS(vis)[i]->layout = blPixelLinearBuffer;
		LIBGGI_APPBUFS(vis)[i]->buffer.plb.stride
			= priv->ximage->bytes_per_line;
		LIBGGI_APPBUFS(vis)[i]->buffer.plb.pixelformat
			= LIBGGI_PIXFMT(vis);
		LIBGGI_APPBUFS(vis)[i]->resource = 
			_ggi_malloc(sizeof(struct ggi_resource));
		LIBGGI_APPBUFS(vis)[i]->resource->priv = vis;
		LIBGGI_APPBUFS(vis)[i]->resource->acquire = priv->acquire;
		LIBGGI_APPBUFS(vis)[i]->resource->release = priv->release;
		LIBGGI_APPBUFS(vis)[i]->resource->curactype = 0;
		LIBGGI_APPBUFS(vis)[i]->resource->count = 0;

		LIBGGI_APPLIST(vis)->first_targetbuf
			= LIBGGI_APPLIST(vis)->last_targetbuf -
			  (mode->frames - 1);
	}

	/* The core doesn't init this soon enough for us. */
	vis->r_frame = LIBGGI_APPBUFS(vis)[0];
	vis->w_frame = LIBGGI_APPBUFS(vis)[0];

	return err;

err0:
	_ggi_free_dbs(vis);
	return err;
}


static int
_ggi_openslave(struct ggi_visual *vis, const char *target, ggi_mode *mode)
{
	ggi_x_priv *priv;
	struct gg_stem *stem;
	int err;

	DPRINT_MODE("_ggi_openslave(%p) called\n", vis);

	err = GGI_OK;
	priv = GGIX_PRIV(vis);

	stem = ggNewStem(libggi, NULL);
	if (stem == NULL) {
		DPRINT("ggNewStem() failed.\n");
		err = GGI_ENOMEM;
		goto err0;
	}

	err = ggiOpen(stem, target, priv->fb);
	if (err < GGI_OK) {
		DPRINT("ggiOpen(%p, \"%s\", %p) failed\n",
			stem, target, priv->fb);
		err = GGI_ENOMEM;
		goto err1;
	}

	priv->slave = STEM_API_DATA(stem, libggi, struct ggi_visual *);
	err = ggiSetMode(priv->slave->instance.stem, mode);
	if (err < GGI_OK) {
		DPRINT("ggiSetMode(%p, %p) failed.\n",
			priv->slave->instance.stem, mode);
		err = GGI_ENOMEM;
		goto err2;
	}

	return err;

err2:
	ggiClose(stem);
err1:
	ggDelStem(stem);
err0:
	return err;
}
