/* $Id: visual.c,v 1.6 2004/11/26 21:35:33 cegger Exp $
******************************************************************************

   Graphics library for GGI. Generic RAMDAC via IOCTL driver

   Copyright (C) 1997 Jason McMullan   [jmcc@ggi-project.org]
   Copyright (C) 1998 Andrew Apted     [andrew@ggi-project.org]

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

#include <stdlib.h>
#include <stdio.h>
#include <sys/ioctl.h>

#ifdef _AIX
#include <sys/types.h>
#include <unistd.h>
#endif

#include "config.h"
#include <ggi/internal/ggi-dl.h>
#include <kgi/kgi_commands.h>

int GGI_ramdac_setpalvec(ggi_visual *vis, int start, int len, const ggi_color *colormap);
int GGI_ramdac_getpalvec(ggi_visual *vis, int start, int len, ggi_color *colormap);

static int GGIopen(ggi_visual *vis, struct ggi_dlhandle *dlh,
			const char *args, void *argptr, uint32 *dlret)
{
	int err;

	GGIDPRINT("generic-ramdac: Init.\n");

	vis->palette = malloc(256*sizeof(ggi_color));
	if (vis->palette == NULL) return GGI_ENOMEM;

	if ((GT_SCHEME(LIBGGI_GT(vis)) != GT_TRUECOLOR) &&
	    (GT_DEPTH(LIBGGI_GT(vis)) > 8))
	{
		fprintf(stderr, "generic-ramdac: too many colors (%d)\n",
			1 << GT_DEPTH(LIBGGI_GT(vis)));
		return GGI_ENOMATCH;
	}

	err = _ggiSendKGICommand(vis, (int)RAMDAC_GETCLUT, vis->palette);
	if (err < 0) {
		fprintf(stderr,"generic-ramdac: Can't get default colormap\n");
		return GGI_ENODEVICE;
	}

#if 0  /* rely on generic-color */
	vis->opcolor->getpalvec=GGI_ramdac_getpalvec;
#endif
	vis->opcolor->setpalvec=GGI_ramdac_setpalvec;

	*dlret = GGI_DL_OPCOLOR;
	return 0;
}

static int GGIclose(ggi_visual *vis, struct ggi_dlhandle *dlh)
{
	free(vis->palette);
	return 0;
}


EXPORTFUNC
int GGIdl_ramdac(int func, void **funcptr);

int GGIdl_ramdac(int func, void **funcptr)
{
	switch (func) {
	case GGIFUNC_open:
		*funcptr = (void *)GGIopen;
		return 0;
	case GGIFUNC_exit:
		*funcptr = NULL;
		return 0;
	case GGIFUNC_close:
		*funcptr = (void *)GGIclose;
		return 0;
	default:
		*funcptr = NULL;
	}

	return GGI_ENOTFOUND;
}

#include <ggi/internal/ggidlinit.h>
