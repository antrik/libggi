/* $Id: visual.c,v 1.1 2001/05/12 23:01:37 cegger Exp $
******************************************************************************

   LibGGI - kgicon specific overrides for fbcon
   Initialization

   Copyright (C) 1998-2000 Marcus Sundberg	[marcus@ggi-project.org]

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
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <ggi/default/genkgi.h>


static int GGIclose(ggi_visual *vis, struct ggi_dlhandle *dlh)
{
	struct genkgi_priv *priv;

	priv = GENKGI_PRIV(vis);

	if (priv->mapped_kgicommand) {
		munmap((void*) priv->mapped_kgicommand,
		       priv->kgicommand_buffersize);
		GGIDPRINT_MISC("gengki: Unmapped kgicommand\n");
	}
	if (priv->fd_kgicommand >= 0) {
		close(priv->fd_kgicommand);
	}

	if (priv->mapped_gc) {
		munmap((void*) priv->mapped_gc, priv->gc_size);
		GGIDPRINT_MISC("gengki: Unmapped GC\n");
	}
	if (priv->close_gc) {
		close(priv->fd_gc);
	}

	free(priv);
	GENKGI_PRIV(vis) = NULL;

	return 0;
}
		

static int GGIopen(ggi_visual *vis, struct ggi_dlhandle *dlh,
		   const char *args, void *argptr, uint32 *dlret)
{
	struct genkgi_priv *priv;
	unsigned long gc_offset = 0;
	char gc[13];
	char kgicommand[21];
	
	sprintf(gc, "/proc/gfx%d/gc", FBDEV_PRIV(vis)->fbnum);
	sprintf(kgicommand, "/proc/gfx%d/kgicommand", FBDEV_PRIV(vis)->fbnum);

	priv = malloc(sizeof(struct genkgi_priv));
	if (priv == NULL) {
		return GGI_ENOMEM;
	}

	priv->mapped_gc = NULL;
	priv->gc_size = getpagesize();
	priv->close_gc = 1;
	priv->mapped_kgicommand = NULL;
	priv->fd_kgicommand = -1;
	/* FIXME: Need to handle this more intelligently */
	priv->kgicommand_buffersize = getpagesize() * 1024;

	GENKGI_PRIV(vis) = priv;

	priv->fd_gc = open(gc, O_RDWR);
	if (priv->fd_gc < 0) {
		priv->fd_gc = LIBGGI_FD(vis);
		priv->close_gc = 0;
		gc_offset = MMAP_TYPE_GC;
	}

	priv->mapped_gc = (ggi_gc *) mmap(NULL, priv->gc_size,
					  PROT_READ | PROT_WRITE,
					  MAP_SHARED, priv->fd_gc,
					  gc_offset);
	if (priv->mapped_gc == MAP_FAILED) {
		GGIclose(vis, dlh);
		return GGI_ENODEVICE;
	}
	GGIDPRINT_MISC("gengki: Mapped GC at %p\n", priv->mapped_gc);
	
	priv->fd_kgicommand = open(kgicommand, O_RDWR);
	if (priv->fd_kgicommand >= 0 &&
	    (priv->mapped_kgicommand = mmap(NULL, priv->kgicommand_buffersize,
					   PROT_READ | PROT_WRITE, MAP_SHARED,
					   priv->fd_kgicommand, 0))
	    != MAP_FAILED) {
		priv->kgicommand_ptr = priv->mapped_kgicommand;
		GGIDPRINT_MISC("gengki: Mapped kgicommand pingpong FIFO at %p\n", 
			       priv->mapped_kgicommand);
	}

	LIBGGI_GC(vis) = priv->mapped_gc;

	priv->drawline = vis->opdraw->drawline;
	priv->drawbox = vis->opdraw->drawbox;
	priv->copybox = vis->opdraw->copybox;
	priv->fillscreen = vis->opdraw->fillscreen;

	vis->opdraw->drawline = GGI_genkgi_drawline;
	vis->opdraw->drawbox = GGI_genkgi_drawbox;
	vis->opdraw->copybox = GGI_genkgi_copybox;
	vis->opdraw->fillscreen = GGI_genkgi_fillscreen;

	*dlret = GGI_DL_OPDRAW;
	return 0;
}


int GGIdl_genkgi(int func, void **funcptr)
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
