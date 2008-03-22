/* $Id: shm.c,v 1.58 2008/03/22 23:25:14 cegger Exp $
******************************************************************************

   MIT-SHM extension support for display-x

   Copyright (C) 1997      Jason McMullan       [jmcc@ggi-project.org]
   Copyright (C) 1998      Andreas Beck         [becka@ggi-project.org]
   Copyright (C) 1998      Steve Cheng          [steve@ggi-project.org]
   Copyright (C) 1998-2000 Marcus Sundberg      [marcus@ggi-project.org]
   Copyright (C) 2002      Brian S. Julin	[bri@tull.umassp.edu]

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
#include <ggi/internal/ggi-dl.h>
#include <ggi/internal/ggi_debug.h>
#include <ggi/display/x.h>
#include <ggi/internal/gg_replace.h>	/* for snprintf */
#include <sys/shm.h>
#include <X11/extensions/XShm.h>
#include <ggi/internal/ggi-module.h>

#include <string.h>

/* Hack to fall back when shm is not working. */
static int      shmerror;
static int      (*oldshmerrorhandler)(Display *, XErrorEvent *);

static int shmerrorhandler (Display * disp, XErrorEvent * event)
{
	if (event->error_code == BadAccess) {
		shmerror = 1;
	} else {
		oldshmerrorhandler(disp, event);
	}

	return 0;
}

static int GGI_XSHM_flush_ximage_child(struct ggi_visual *vis, 
			     int x, int y, int w, int h, int tryflag)
{
	ggi_x_priv *priv;
	priv = GGIX_PRIV(vis);

	if (tryflag == 0) {
		/* flush later, this is in signal handler context
		 * when using the signal based scheduler
		 */
		ggUnlock(priv->flushlock);
		return 0;
	}

	if (priv->opmansync) MANSYNC_ignore(vis);

	if (tryflag != 2) GGI_X_LOCK_XLIB(vis);

	priv->flush_cmap(vis);		/* Update the palette/gamma */

	/* Flush any pending Xlib operations. */
	XSync(priv->disp, 0);

	if (priv->fullflush || 
	    (GGI_ACTYPE_WRITE & (vis->w_frame->resource->curactype))) {
		/* Flush all requested data */
		if (tryflag != 2) {
			GGI_X_CLEAN(vis, x, y, w, h);
			y = GGI_X_WRITE_Y;
		} /* else it's a non-translated exposure event. */
		XShmPutImage(priv->disp, priv->drawable, priv->tempgc, priv->ximage,
			  x, y, x, y, (unsigned)w, (unsigned)h, 0);
	} else {
		/* Just flush the intersection with the dirty region */
		int x2, y2;

		if (priv->dirtytl.x > priv->dirtybr.x) goto clean;
		if (x > priv->dirtybr.x) goto clean;
		if (y > priv->dirtybr.y) goto clean;
		x2 = x + w - 1;
		if (x2 < priv->dirtytl.x) goto clean;
		y2 = y + h - 1;
		if (y2 < priv->dirtytl.y) goto clean;
		if (x < priv->dirtytl.x)  x  = priv->dirtytl.x;
		if (y < priv->dirtytl.y)  y  = priv->dirtytl.y;
		if (x2 > priv->dirtybr.x) x2 = priv->dirtybr.x;
		if (y2 > priv->dirtybr.y) y2 = priv->dirtybr.y;
		w = x2 - x + 1;
		h = y2 - y + 1;
		if ((w <= 0) || (h <= 0)) goto clean;

		XShmPutImage(priv->disp, priv->drawable, priv->tempgc, priv->ximage,
			  x, GGI_X_WRITE_Y, x, GGI_X_WRITE_Y,
			  (unsigned)w, (unsigned)h, 0);
		GGI_X_CLEAN(vis, x, y, w, h);
	}

	/* Tell X Server to start blitting */
	XFlush(priv->disp);
 clean:
	if (tryflag != 2) GGI_X_UNLOCK_XLIB(vis);
	if (priv->opmansync) MANSYNC_cont(vis);
	return 0;
}

static void
free_dbs(struct ggi_visual *vis)
{
	int i, first, last;

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

/* XImage allocation for normal client-side buffer */
static void _ggi_xshm_free_ximage(struct ggi_visual *vis)
{
	ggi_x_priv *priv;
	XShmSegmentInfo *myshminfo;

	priv = GGIX_PRIV(vis);
	myshminfo = priv->priv;
	if (myshminfo == NULL) return;

	if (priv->slave) {
		struct gg_stem *stem = priv->slave->instance.stem;
		ggiClose(priv->slave->instance.stem);
		ggDelStem(stem);
	}
	priv->slave = NULL;

	if (priv->ximage) {

		XShmDetach(priv->disp, myshminfo);
		/* Seems OK to destroy image before fb for SHM */
	  	XDestroyImage(priv->ximage);
	  	shmdt(myshminfo->shmaddr);
		/* shmid has already been removed, see below. */
		priv->fb = NULL;
	}
	if (priv->fb) free(priv->fb);
	priv->ximage = NULL;
	priv->fb = NULL;

	free(myshminfo);
	priv->priv = NULL;

	free_dbs(vis);
}

static int _ggi_xshm_create_ximage(struct ggi_visual *vis)
{
	char target[GGI_MAX_APILEN];
	ggi_mode tm;
	ggi_x_priv *priv;
	struct gg_stem *stem;
	int err, i;
	XShmSegmentInfo *myshminfo;
	size_t shmsize;


	err = GGI_OK;
	i = 0; /* ??? Cranky GCC */

	priv = GGIX_PRIV(vis);

	DPRINT_MODE("X: MIT-SHM: Creating shared MIT-SHM buffer\n");

	_ggi_xshm_free_ximage(vis);

	priv->priv = calloc(1, sizeof(XShmSegmentInfo));
	if (!priv->priv) return GGI_ENOMEM;
	myshminfo = priv->priv;

	priv->ximage = XShmCreateImage(priv->disp,
				priv->vilist[priv->viidx].vi->visual, 
				(unsigned)priv->vilist[priv->viidx].vi->depth,
				ZPixmap,		/* format */
				NULL,		/* data */
				myshminfo,	/* shm object */
				(unsigned)LIBGGI_VIRTX(vis), 
				(unsigned)(LIBGGI_VIRTY(vis) * LIBGGI_MODE(vis)->frames));
	if (priv->ximage == NULL) {
		DPRINT("XShmCreateImage() failed.");
		err = GGI_ENOMEM;
		goto err0;
	}

	shmsize = priv->ximage->bytes_per_line
			* LIBGGI_VIRTY(vis) * LIBGGI_MODE(vis)->frames;

	DPRINT_MODE("X: MIT-SHM: Try to shmget() a buffer of %lu (0x%lx) size bytes\n",
		shmsize, shmsize);

	myshminfo->shmid = shmget(IPC_PRIVATE, shmsize, IPC_CREAT | 0777);
	if (myshminfo->shmid == -1) {
		DPRINT("shmget() failed.\n");
		priv->fb = NULL;
		err = GGI_ENOMEM;
		goto err1;
	}

	priv->fb = shmat(myshminfo->shmid,0,0);
	if (priv->fb == (void *)-1) {
		DPRINT("shmat() failed.\n");
		priv->fb = NULL;
		err = GGI_ENOMEM;
		goto err1;
	}
	myshminfo->shmaddr = priv->ximage->data = (char *)priv->fb;
	DPRINT_MODE("X: MIT-SHM: shmat success at %p.\n", priv->fb);

	myshminfo->readOnly = False;

	ggLock(_ggi_global_lock); /* Entering protected section */
	shmerror = 0;
	DPRINT_MODE("X: MIT-SHM: install error handler\n");
	oldshmerrorhandler = XSetErrorHandler(shmerrorhandler);
	DPRINT_MODE("X: MIT-SHM: Attach shm to display\n");
	XShmAttach(priv->disp, myshminfo);

	XSync(priv->disp, 0);
	DPRINT_MODE("X: MIT-SHM: restore error handler\n");
	XSetErrorHandler(oldshmerrorhandler);
	if (shmerror) {
		ggUnlock(_ggi_global_lock); /* Exiting protected section */
		DPRINT("can not access XSHM.\n");
		err = GGI_ENOMEM;
		goto err2;
	} else {
		/* Take the shmid away so noone else can get it. */
		shmctl(myshminfo->shmid, IPC_RMID, 0);
		DPRINT_MODE("X: MIT-SHM: ShmImage #%d allocated\n", i);
	}
	ggUnlock(_ggi_global_lock); /* Exiting protected section */


	LIBGGI_APPLIST(vis)->first_targetbuf = -1;
	for (i = 0; i < LIBGGI_MODE(vis)->frames; i++) {
		ggi_directbuffer *db;

		db = _ggi_db_get_new();
		if (!db) {
			DPRINT("frame %u allocation failed.\n", i);
			err = GGI_ENOMEM;
			goto err3;
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
		  = LIBGGI_APPLIST(vis)->last_targetbuf - (LIBGGI_MODE(vis)->frames-1);
	}

	/* The core doesn't init this soon enough for us. */
	vis->w_frame = LIBGGI_APPBUFS(vis)[0];

	/* We assume LIBGGI_MODE(vis) structure has already been filled out */
	memcpy(&tm, LIBGGI_MODE(vis), sizeof(ggi_mode));

	/* Make sure we do not fail due to physical size constraints,
	 * which are meaningless on a memory visual.
	 */
	tm.size.x=tm.size.y=GGI_AUTO;

	i = 0;
	i += snprintf(target, GGI_MAX_APILEN, "display-memory:-pixfmt=");

	memset(target+i, '\0', 64);
	_ggi_build_pixfmtstr(vis, target + i, sizeof(target) - i, 1);
	i = strlen(target);

	snprintf(target + i, GGI_MAX_APILEN - i, ":-layout=%iplb%i:-physz=%i,%i:pointer",
		priv->ximage->bytes_per_line * LIBGGI_VIRTY(vis),
		priv->ximage->bytes_per_line,
		LIBGGI_MODE(vis)->size.x, LIBGGI_MODE(vis)->size.y);

	stem = ggNewStem(libggi, NULL);
	if (stem == NULL) {
		DPRINT("ggNewStem() failed.\n");
		err = GGI_ENOMEM;
		goto err3;
	}
	if (ggiOpen(stem, target, priv->fb) != GGI_OK) {
		DPRINT("ggiOpen(%p, \"%s\", %p) failed\n",
			stem, target, priv->fb);
		ggDelStem(stem);
		err = GGI_ENOMEM;
		goto err3;
	}
	priv->slave = STEM_API_DATA(stem, libggi, struct ggi_visual *);
	if (ggiSetMode(priv->slave->instance.stem, &tm) < 0) {
		DPRINT("ggiSetMode(%p, %p) failed.\n",
			priv->slave->instance.stem, &tm); 
		err = GGI_ENOMEM;
		goto err3;
	}

	priv->ximage->byte_order = ImageByteOrder(priv->disp);
	priv->ximage->bitmap_bit_order = BitmapBitOrder(priv->disp);

	vis->opdisplay->flush		= GGI_XSHM_flush_ximage_child;

	DPRINT_MODE("X: MIT-SHM: XSHMImage and slave visual %p share buffer at %p\n",
		       priv->slave, priv->fb);

	return GGI_OK;

err3:
	fprintf(stderr,
		"XSHM extension failed to initialize. Retry with -noshm\n");
	_ggi_xshm_free_ximage(vis);
	return err;

err2:
	XShmDetach(priv->disp, myshminfo);
	shmdt(priv->fb);
	priv->fb = NULL;
err1:
	XDestroyImage(priv->ximage);
	priv->ximage = NULL;
err0:
	fprintf(stderr,
		"XSHM extension failed to initialize. Retry with -noshm\n");
	return err;
}

static int
GGI_helper_x_shm_setup(struct ggi_helper *helper,
			const char *args, void *argptr)
{
	ggi_x_priv *priv;
	int major, minor;
	Bool pixmaps;

	DPRINT_LIBS("GGI_helper_x_shm_setup(%p, %s, %p) called\n",
		helper, args, argptr);

	priv = GGIX_PRIV(helper->visual);

	if (XShmQueryExtension(priv->disp) != True)
		return GGI_ENOFUNC;
	if (XShmQueryVersion(priv->disp, &major, &minor, &pixmaps) != True)
		return GGI_ENOFUNC;

	DPRINT_LIBS("X: MIT-SHM: SHM version %i.%i %s pixmap support\n",
		major, minor, pixmaps ? "with" : "without");

	priv->createfb = _ggi_xshm_create_ximage;
	priv->freefb = _ggi_xshm_free_ximage;

	return GGI_OK;
}

static void
GGI_helper_x_shm_teardown(struct ggi_helper *helper)
{
	ggi_x_priv *priv;

	DPRINT_LIBS("GGI_helper_x_shm_teardown(%p) called\n",
		helper);

	priv = GGIX_PRIV(helper->visual);

	if (priv && priv->freefb) {
		priv->freefb(helper->visual);
		priv->freefb = NULL;
	}

	return;
}


struct ggi_module_helper GGI_helper_x_shm = {
	GG_MODULE_INIT("helper-x-shm", 0, 1, GGI_MODULE_HELPER),
	GGI_helper_x_shm_setup,
	GGI_helper_x_shm_teardown
};

static struct ggi_module_helper *_GGIdl_helper_x_shm[] = {
	&GGI_helper_x_shm,
	NULL
};

EXPORTFUNC
int GGIdl_helper_x_shm(int func, void **funcptr);

int GGIdl_helper_x_shm(int func, void **funcptr)
{
	struct ggi_module_helper ***modulesptr;

	switch (func) {
	case GG_DLENTRY_MODULES:
		modulesptr = (struct ggi_module_helper ***)funcptr;
		*modulesptr = _GGIdl_helper_x_shm;
		return GGI_OK;
	default:
		*funcptr = NULL;
	}

	return GGI_ENOTFOUND;
}
