/* $Id: shm.c,v 1.11 2003/02/09 00:10:19 cegger Exp $
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
#include <ggi/display/x.h>
#include <sys/shm.h>
#include <X11/extensions/XShm.h>

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

int GGI_XSHM_flush_ximage_child(ggi_visual *vis, 
			     int x, int y, int w, int h, int tryflag)
{
	ggi_x_priv *priv;
	priv = GGIX_PRIV(vis);

	if (priv->opmansync) MANSYNC_ignore(vis);

	if (tryflag == 0) {
		if (ggTryLock(priv->xliblock) != 0) {
			GGIDPRINT_MISC("xshm: TRYLOCK fail (in flush_ximage_child)!\n");
			if (priv->opmansync) MANSYNC_cont(vis);
			return 0;
		}
	} else if (tryflag != 2) {
		ggLock(priv->xliblock);
	}
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
		XShmPutImage(priv->disp, priv->win, priv->tempgc, priv->ximage,
			  x, y, x, y, w, h, 0);
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

		XShmPutImage(priv->disp, priv->win, priv->tempgc, priv->ximage,
			  x, GGI_X_WRITE_Y, x, GGI_X_WRITE_Y, w, h, 0);
		GGI_X_CLEAN(vis, x, y, w, h);
	}

	/* Tell X Server to start blitting */
	XFlush(priv->disp);
 clean:
	if (tryflag != 2) ggUnlock(priv->xliblock);
	if (priv->opmansync) MANSYNC_cont(vis);
	return 0;
}

/* XImage allocation for normal client-side buffer */
void _ggi_xshm_free_ximage(ggi_visual *vis) {
	ggi_x_priv *priv;
	int i, first, last;
	XShmSegmentInfo *myshminfo;

	priv = GGIX_PRIV(vis);
	myshminfo = priv->priv;
	if (myshminfo == NULL) return;

	if (priv->slave) ggiClose(priv->slave);
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

	first = LIBGGI_APPLIST(vis)->first_targetbuf;
	last = LIBGGI_APPLIST(vis)->last_targetbuf;
	if (first < 0) {
		return;
	}
	for (i = (last - first); i >= 0; i--) {
		free(LIBGGI_APPBUFS(vis)[i]->resource);
		_ggi_db_free(LIBGGI_APPLIST(vis)->bufs[i+first]);
		_ggi_db_del_buffer(LIBGGI_APPLIST(vis), i+first);
	}
	LIBGGI_APPLIST(vis)->first_targetbuf = -1;
}

int _ggi_xshm_create_ximage(ggi_visual *vis) {

	char target[256];
	ggi_mode tm;
	ggi_x_priv *priv;
	int i;
	XShmSegmentInfo *myshminfo;
	ggi_directbuffer *db;


	i = 0; /* ??? Cranky GCC */

	priv = GGIX_PRIV(vis);

	GGIDPRINT_MODE("X: Creating shared MIT-SHM buffer\n");

	_ggi_xshm_free_ximage(vis);

	priv->priv = calloc(1, sizeof(XShmSegmentInfo));
	if (!priv->priv) return GGI_ENOMEM;
	myshminfo = priv->priv;

	priv->ximage = XShmCreateImage(priv->disp,
				       priv->vilist[priv->viidx].vi->visual, 
				       priv->vilist[priv->viidx].vi->depth,
				       ZPixmap,		/* format */
				       NULL,		/* data */
				       myshminfo,	/* shm object */
				       LIBGGI_VIRTX(vis), 
				       LIBGGI_VIRTY(vis) * vis->mode->frames);

	myshminfo->shmid = 
		shmget(IPC_PRIVATE,
		       priv->ximage->bytes_per_line * 
		       LIBGGI_VIRTY(vis) * vis->mode->frames,
		       IPC_CREAT | 0777);
	
	priv->fb = myshminfo->shmaddr = priv->ximage->data =
		shmat(myshminfo->shmid,0,0);
	GGIDPRINT_MODE("X: shmat success at %p.\n", priv->fb);

	myshminfo->readOnly = False;

	ggLock(_ggi_global_lock); /* Entering protected section */
	shmerror = 0;
	oldshmerrorhandler = XSetErrorHandler(shmerrorhandler);
	XShmAttach(priv->disp, myshminfo);

	XSync(priv->disp, 0);
	XSetErrorHandler(oldshmerrorhandler);
	if (shmerror) {
		if (priv->ximage) {
			/* Seems OK to destroy image before fb for SHM */
			XDestroyImage(priv->ximage);
			priv->ximage = NULL;
		}
		if (priv->fb != NULL) {
			shmdt(priv->fb);
			/* The shmid has already been removed, see below. */
			priv->fb = NULL;
		}
		fprintf(stderr, "XSHM extension failed to initialize. Retry with -noshm\n");
		ggUnlock(_ggi_global_lock); /* Exiting protected section */
		return GGI_ENOMEM;
	} else {
		/* Take the shmid away so noone else can get it. */
		shmctl(myshminfo->shmid, IPC_RMID, 0);
		GGIDPRINT_MODE("X: ShmImage #%d allocated\n", i);
	}
	ggUnlock(_ggi_global_lock); /* Exiting protected section */


	LIBGGI_APPLIST(vis)->first_targetbuf = -1;
	for (i = 0; i < vis->mode->frames; i++) {
		db = _ggi_db_get_new();
		if (!db) {
			_ggi_xshm_free_ximage(vis);
			return GGI_ENOMEM;
		}

		LIBGGI_APPLIST(vis)->last_targetbuf
		  = _ggi_db_add_buffer(LIBGGI_APPLIST(vis), db);
		LIBGGI_APPBUFS(vis)[i]->frame = i;
		LIBGGI_APPBUFS(vis)[i]->type
		  = GGI_DB_NORMAL | GGI_DB_SIMPLE_PLB;
		LIBGGI_APPBUFS(vis)[i]->read = LIBGGI_APPBUFS(vis)[i]->write =
		  priv->fb + i * vis->mode->virt.y * 
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
		  = LIBGGI_APPLIST(vis)->last_targetbuf - (vis->mode->frames-1);
	}

	/* The core doesn't init this soon enough for us. */
	vis->w_frame = LIBGGI_APPBUFS(vis)[0];

	/* We assume vis->mode structure has already been filled out */
	memcpy(&tm, vis->mode, sizeof(ggi_mode));

	i = 0;
	i += sprintf(target, "display-memory:-pixfmt=");
	memset(target+i, '\0', 64);
	_ggi_pixfmtstr(vis, target + i, 1);
	i = strlen(target);
	sprintf(target + i, ":-layout=%iplb%i:-physz=%i,%i:pointer",
		priv->ximage->bytes_per_line * vis->mode->virt.y,
		priv->ximage->bytes_per_line,
		vis->mode->size.x, vis->mode->size.y);

	priv->slave = ggiOpen(target, priv->fb);
	if (priv->slave == NULL || ggiSetMode(priv->slave, &tm)) {
		_ggi_xshm_free_ximage(vis);
		return GGI_ENOMEM;
	}

#ifdef GGI_LITTLE_ENDIAN
	priv->ximage->byte_order = LSBFirst;
	priv->ximage->bitmap_bit_order = LSBFirst;
#else
	priv->ximage->byte_order = MSBFirst;
	priv->ximage->bitmap_bit_order = MSBFirst;
#endif

	vis->opdisplay->flush		= GGI_XSHM_flush_ximage_child;

	GGIDPRINT_MODE("X: XSHMImage and slave visual %p share buffer at %p\n",
		       priv->slave, priv->fb);

	return GGI_OK;
}

static int GGIopen(ggi_visual *vis, struct ggi_dlhandle *dlh,
		   const char *args, void *argptr, uint32 *dlret)
{
	ggi_x_priv *priv;
	int major, minor;
	Bool pixmaps;

	priv = GGIX_PRIV(vis);

	if (XShmQueryExtension(priv->disp) != True) return GGI_ENOFUNC;
	if (XShmQueryVersion(priv->disp, &major, &minor, &pixmaps)
	    != True) return GGI_ENOFUNC;

	GGIDPRINT_LIBS("X: SHM version %i.%i %s pixmap support\n",
		       major, minor, pixmaps ? "with" : "without");

	priv->createfb = _ggi_xshm_create_ximage;
	priv->freefb = _ggi_xshm_free_ximage;

	*dlret = 0;
	return GGI_OK;
}

static int GGIclose(ggi_visual *vis, struct ggi_dlhandle *dlh)
{
	ggi_x_priv *priv;
	priv = GGIX_PRIV(vis);

	if (priv && priv->freefb) priv->freefb(vis);
	priv->freefb = NULL;

	/* Hack.  close the display now, because otherwise callback segv's */

	XSync(priv->disp,0);

	if (priv->slave) ggiClose(priv->slave);
	priv->slave = NULL;

	if (priv->freefb) priv->freefb(vis);

	/* Exit any initialized helper libs if called from GGIopen. */
	if (vis->extlib) {
		_ggiExitDL(vis, vis->extlib);
		_ggiZapDL(vis, &vis->extlib);
	}

	if (priv->win != priv->parentwin) {
		if (priv->win != 0) XDestroyWindow(priv->disp,priv->win);
	}
	if (!priv->parentwin) goto skip;

	/* Do special cleanup for -inwin and root windows */
	if (vis->opdisplay->checkmode == priv->shmhack_checkmode_fixed) {
		unsigned int dummy;
		Window root;
		int screen;
		XSetWindowAttributes wa;

		screen = priv->vilist[priv->viidx].vi->screen;
		XGetGeometry(priv->disp, priv->parentwin, &root, (int *)&dummy,
			     (int *)&dummy, &dummy, &dummy, &dummy, &dummy);
		if (priv->parentwin == root) 
			XSetWindowColormap(priv->disp, priv->parentwin,
					   DefaultColormap(priv->disp,screen));
		wa.cursor = None;
		XChangeWindowAttributes(priv->disp, priv->parentwin, 
					CWCursor, &wa);
	} else  XDestroyWindow(priv->disp, priv->parentwin);

skip:
	priv->shmhack_free_cmaps(vis);

	if (priv->cursor != None)   XFreeCursor(priv->disp,priv->cursor);
	if (priv->textfont != None) XFreeFont(priv->disp, priv->textfont);
	if (priv->fontimg)	    XDestroyImage(priv->fontimg);
	if (priv->visual)           XFree(priv->visual);
	if (priv->buflist)	    XFree(priv->buflist);
	if (priv->disp)		    XCloseDisplay(priv->disp);
	if (priv->vilist)	    free(priv->vilist);
	if (priv->modes)	    free(priv->modes);
	if (priv->opmansync)	    free(priv->opmansync);

	priv->disp = NULL;
	return 0;
}

int GGIdl_helper_x_shm(int func, void **funcptr)
{
	switch (func) {
	case GGIFUNC_open:
		*funcptr = GGIopen;
		return 0;
	case GGIFUNC_exit:
		*funcptr = NULL;
		return 0;
	case GGIFUNC_close:
		*funcptr = GGIclose;
		return 0;
	default:
		*funcptr = NULL;
	}

	return GGI_ENOTFOUND;
}

#include <ggi/internal/ggidlinit.h>
