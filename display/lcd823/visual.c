/* $Id: visual.c,v 1.2 2002/09/08 21:37:46 soyt Exp $
******************************************************************************

   Display-lcd823: visual handling

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
#include <ggi/internal/ggi-dl.h>
#include <ggi/display/lcd823.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>

#ifndef MAP_FAILED
#define MAP_FAILED ((void*)-1)
#endif


#define LCD_DEVNAME	"/dev/lcd823"


extern void _GGI_lcd823_free_dbs(ggi_visual *vis);


static int
GGI_lcd823_flush(ggi_visual *vis, int x, int y, int w, int h, int tryflag)
{
	return 0;
}


static int do_cleanup(ggi_visual *vis)
{
	ggi_lcd823_priv *priv = LIBGGI_PRIVATE(vis);

	/* We may be called more than once due to the LibGG cleanup stuff */
	if (priv == NULL) return 0;

	GGIDPRINT("display-lcd823: do_cleanup start.\n");

	_GGI_lcd823_free_dbs(vis);

	if (LIBGGI_FD(vis) >= 0) {
		if (priv->fb_ptr) {
			munmap(priv->fb_ptr, priv->fb_size);
		}
		ioctl(LIBGGI_FD(vis), 2); /* Disable LCD */
		close(LIBGGI_FD(vis));
		LIBGGI_FD(vis) = -1;
	}

	if (vis->input != NULL) {
		giiClose(vis->input);
		vis->input = NULL;
	}

	free(priv);
	LIBGGI_PRIVATE(vis) = NULL;

	if (LIBGGI_GC(vis)) {
		free(LIBGGI_GC(vis));
	}

	ggUnregisterCleanup((ggcleanup_func *)do_cleanup, vis);

	GGIDPRINT("display-lcd823: do_cleanup done.\n");

	return 0;
}


#define MAX_DEV_LEN	63

static int GGIopen(ggi_visual *vis, struct ggi_dlhandle *dlh,
		   const char *args, void *argptr, uint32 *dlret)
{
	ggi_lcd823_priv *priv;
	int size;

	GGIDPRINT("display-lcd823: GGIopen start.\n");

	LIBGGI_PRIVATE(vis) = priv = malloc(sizeof(ggi_lcd823_priv));
	if (priv == NULL) {
		return GGI_ENOMEM;
	}
	
	priv->fb_ptr = NULL;

	/* Now open the framebuffer device */
	LIBGGI_FD(vis) = open(LCD_DEVNAME, O_RDWR);
	if (LIBGGI_FD(vis) < 0) {
		fprintf(stderr, "display-lcd823: Couldn't open "
			"framebuffer device %s: %s\n", LCD_DEVNAME,
			strerror(errno));
		do_cleanup(vis);
		return GGI_ENODEVICE;
	}
	if (ioctl(LIBGGI_FD(vis), 1) != 0) {
		fprintf(stderr, "display-lcd823: Unable to enable LCD: %s\n",
			strerror(errno));
		do_cleanup(vis);
		return GGI_ENODEVICE;
	}
	if (ioctl(LIBGGI_FD(vis), 5, &size) != 0) {
		fprintf(stderr, "display-lcd823: Unable to get size of LCD "
			"memory: %s\n",
			strerror(errno));
		do_cleanup(vis);
		return GGI_ENODEVICE;
	}

	priv->fb_size = size * getpagesize();
	priv->fb_ptr = mmap(NULL, priv->fb_size, PROT_READ | PROT_WRITE,
			    MAP_SHARED, LIBGGI_FD(vis), 0);
	if (priv->fb_ptr == MAP_FAILED) {
		fprintf(stderr, "display-lcd823: Unable to map LCD "
			"memory: %s\n",
			strerror(errno));
		priv->fb_ptr = NULL;
		do_cleanup(vis);
		return GGI_ENODEVICE;
	}
	priv->frame_size = priv->fb_size;

	LIBGGI_GC(vis) = malloc(sizeof(ggi_gc));
	if (LIBGGI_GC(vis) == NULL) {
		do_cleanup(vis);
		return GGI_ENOMEM;
	}

	/* Mode management */
	vis->opdisplay->getmode   = GGI_lcd823_getmode;
	vis->opdisplay->setmode   = GGI_lcd823_setmode;
	vis->opdisplay->checkmode = GGI_lcd823_checkmode;
	vis->opdisplay->getapi    = GGI_lcd823_getapi;
	vis->opdisplay->flush     = GGI_lcd823_flush;
	vis->opdisplay->setflags  = GGI_lcd823_setflags;

	/* Register cleanup handler */
	ggRegisterCleanup((ggcleanup_func *)do_cleanup, vis);

	GGIDPRINT("display-lc823: GGIopen success.\n");

	*dlret = GGI_DL_OPDISPLAY;
	return 0;
}

static int GGIclose(ggi_visual *vis, struct ggi_dlhandle *dlh)
{
	return do_cleanup(vis);
}


int GGIdl_lcd823(int func, void **funcptr)
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
