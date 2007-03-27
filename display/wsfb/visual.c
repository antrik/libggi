/* $Id: visual.c,v 1.13 2007/03/27 18:20:30 soyt Exp $
******************************************************************************

   wsconsole(4) wsfb target: initialization

   Copyright (C) 2003 Todd T. Fries <todd@openbsd.org>

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

#include <ggi/internal/ggi.h>
#include <ggi/internal/ggi_debug.h>
#include <ggi/display/wsfb.h>


static int usagecounter = 0;

static int do_cleanup(struct ggi_visual *vis)
{
	int rc = 0;
	wsfb_priv *priv = WSFB_PRIV(vis);

	DPRINT("do_cleanup\n");

	/*
	if (vis->input != NULL) {
		giiClose(vis->input);
		vis->input = NULL;
	}
	*/
	if (priv->availmodes != NULL) {
		free(priv->availmodes);
	}	/* if */

	if (priv->fd >= 0) {
		if (priv->origmode == 0) {
			priv->mode = WSDISPLAYIO_MODE_EMUL;
			rc = ioctl(priv->fd, WSDISPLAYIO_SMODE, &priv->mode);
			if (rc == -1) {
				DPRINT("ioctl WSDISPLAYIO_SMODE error: %s %s\n",
					priv->devname, strerror(errno));
			}	/* if */
		}	/* if */

		munmap(priv->base, priv->mapsize);
		rc = ioctl(priv->fd, WSDISPLAYIO_PUTCMAP, &priv->ocmap);
		if (rc < 0) {
			DPRINT("ioctl WSDISPLAYIO_PUTCMAP error: %s %s\n",
				priv->devname, strerror(errno));
		}	/* if */
		close(priv->fd);
	}	/* if */

	free(LIBGGI_GC(vis));

	ggUnregisterCleanup((ggcleanup_func *)do_cleanup, vis);

	usagecounter--;

	return 0;
}	/* do_cleanup */


static int GGIopen(struct ggi_visual *vis, struct ggi_dlhandle *dlh,
			const char *args, void *argptr, uint32_t *dlret)
{
	wsfb_priv *priv = NULL;
	int error = 0, i;

	DPRINT("GGIopen\n");
	
	ggLock(_ggi_global_lock); /* Entering protected section */
	if (usagecounter > 0) {
		ggUnlock(_ggi_global_lock);
		DPRINT("display-wsfb: You can only open this target "
			"once in an application.\n");
		error = GGI_EBUSY;
		goto error;
	}
	usagecounter++;
	ggUnlock(_ggi_global_lock); /* Exiting protected section */

	ggRegisterCleanup((ggcleanup_func *)do_cleanup, vis);
	ggCleanupForceExit();

	LIBGGI_GC(vis) = malloc(sizeof(ggi_gc));
	if (LIBGGI_GC(vis) == NULL) {
		usagecounter--;
		error = GGI_ENOMEM;
		goto error;
	}
	WSFB_PRIV(vis) = malloc(sizeof(struct wsfb_priv));
	if (WSFB_PRIV(vis) == NULL) {
		do_cleanup(vis);
		error = GGI_ENOMEM;
		goto error;
	}
	priv = WSFB_PRIV(vis);
	memset(priv, 0, sizeof(*priv));

	priv->devname = strdup("/dev/ttyC0");

	priv->fd = open(priv->devname, O_RDWR);
	if (priv->fd < 0) {
		DPRINT("fd open error: %s %s\n",
			priv->devname, strerror(errno));
		error = GGI_ENODEVICE;
		goto error;
	}


	if (ioctl(priv->fd, WSDISPLAYIO_GINFO, &priv->info) == -1) {
		DPRINT("ioctl WSDISPLAYIO_GINFO error: %s %s\n",
			priv->devname, strerror(errno));
		error = GGI_ENODEVICE;
		goto error;
	}
	if (ioctl(priv->fd, WSDISPLAYIO_GTYPE, &priv->wstype) == -1) {
		DPRINT("ioctl WSDISPLAYIO_GTYPE error: %s %s\n",
			priv->devname, strerror(errno));
		error = GGI_ENODEVICE;
		goto error;
	}

/* WSDISPLAYIO_LINEBYTES is not available under NetBSD
 */
#ifdef WSDISPLAYIO_LINEBYTES
	if (ioctl(priv->fd, WSDISPLAYIO_LINEBYTES, &priv->linebytes) == -1) {
		DPRINT("ioctl WSDISPLAYIO_LINEBYTES error: %s %s\n",
			priv->devname, strerror(errno));
		error = GGI_ENODEVICE;
		goto error;
	}
#endif

	DPRINT("info: depth: %d height: %d width: %d\n",
		priv->info.depth, priv->info.height, priv->info.width);

	if (ioctl(priv->fd, WSDISPLAYIO_GMODE, &priv->origmode) == -1) {
		DPRINT("ioctl WSDISPLAYIO_GMODE ERROR: %s %s\n",
			priv->devname, strerror(errno));
		error = GGI_ENODEVICE;
		goto error;
	}

	// printf("original mode: 0x%x\n", priv->origmode);

	if (priv->origmode == 0) {
		priv->mode = WSDISPLAYIO_MODE_MAPPED;
		if (ioctl(priv->fd, WSDISPLAYIO_SMODE, &priv->mode) == -1) {
			DPRINT("ioctl WSDISPLAYIO_SMODE ERROR: %s %s\n",
				priv->devname, strerror(errno));
			error = GGI_ENODEVICE;
			goto error;
		}
	}

	priv->size = priv->info.height * priv->info.width;	
	priv->Base = 0;

	/* Round to the nearest page */

	priv->pagemask = getpagesize() - 1;
	priv->mapsize = ((int)priv->size  + priv->pagemask) & ~(priv->pagemask);

	/* Open keyboard and mouse input */
	/*
	vis->input = giiOpen("stdin:ansikey", NULL);
	if (vis->input == NULL) {
		DPRINT(
"display-wsfb: Unable to open stdin input, try running with '-nokbd'.\n");
			do_cleanup(vis);
			error = GGI_ENODEVICE;
			goto error;
	}
	*/
	vis->opdisplay->getmode		= GGI_wsfb_getmode;
	vis->opdisplay->setmode		= GGI_wsfb_setmode;
	vis->opdisplay->getapi		= GGI_wsfb_getapi;
	vis->opdisplay->checkmode	= GGI_wsfb_checkmode;

	*dlret = GGI_DL_OPDISPLAY;
	return 0;

error:
	DPRINT("display-wsfb: open failed (%s %s)\n",
		priv->devname, strerror(errno));
	return error;
}


static int GGIclose(struct ggi_visual *vis, struct ggi_dlhandle *dlh)
{
	DPRINT("GGIclose\n");
	return do_cleanup(vis);
}


EXPORTFUNC
int GGIdl_wsfb(int func, void **funcptr);

int GGIdl_wsfb(int func, void **funcptr)
{
	ggifunc_open **openptr;
	ggifunc_close **closeptr;

	DPRINT("GGIdl_wsfb\n");
	switch (func) {
	case GGIFUNC_open:
		openptr = (ggifunc_open **)funcptr;
		*openptr = GGIopen;
		return 0;
	case GGIFUNC_exit:
		*funcptr = NULL;
		return 0;
	case GGIFUNC_close:
		closeptr = (ggifunc_close **)funcptr;
		*closeptr = GGIclose;
		return 0;
	default:
		*funcptr = NULL;
	}

	return GGI_ENOTFOUND;
}
