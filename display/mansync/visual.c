/* $Id: visual.c,v 1.12 2007/06/20 08:17:21 pekberg Exp $
******************************************************************************

   Helper library for the implementation of SYNC mode on targets which are
   inherently ASYNC (e.g. X) and require manual flushes of the framebuffer.

   Mansync initialization.

   Copyright (C) 1998 Steve Cheng		[steve@ggi-project.org]
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

   Helper library for the implementation of SYNC mode on targets which are
   inherently ASYNC (e.g. X) and require manual flushes of the framebuffer.

******************************************************************************
*/

#include "config.h"
#include <ggi/internal/ggi-dl.h>
#include <ggi/display/mansync.h>
#include <ggi/internal/ggi-module.h>
#include <ggi/internal/ggi_debug.h>


static int
GGI_mansync_setup(struct ggi_helper *helper, const char *args, void *argptr)
{
	_ggi_opmansync *ops = (_ggi_opmansync *) argptr;

	DPRINT_LIBS("GGI_mansync_setup(%p, %s, %p) called\n",
		helper, args, argptr);

	if (ops == NULL) {
		ggPanic("Target tried to use mansync helper in a wrong way!\n");
	}

	ops->init   = _GGI_mansync_init;
	ops->deinit = _GGI_mansync_deinit;
	ops->start  = _GGI_mansync_start;
	ops->stop   = _GGI_mansync_stop;
	ops->ignore = _GGI_mansync_ignore;
	ops->cont   = _GGI_mansync_cont;

	return GGI_OK;
}

static int GGIopen(struct ggi_visual *vis, struct ggi_dlhandle *dlh,
			const char *args, void *argptr, uint32_t *dlret)
{
	*dlret = 0;
	return GGI_mansync_setup(NULL, args, argptr);
}

struct ggi_module_helper GGI_mansync = {
	GG_MODULE_INIT("helper-mansync", 0, 1, GGI_MODULE_HELPER),
	GGI_mansync_setup,
	NULL /* teardown */
};

static struct ggi_module_helper *_GGIdl_mansync[] = {
	&GGI_mansync,
	NULL
};

EXPORTFUNC
int GGIdl_mansync(int func, void **funcptr);

int GGIdl_mansync(int func, void **funcptr)
{
	ggifunc_open **openptr;
	struct ggi_module_helper ***modulesptr;

	switch (func) {
	case GGIFUNC_open:
		openptr = (ggifunc_open **)funcptr;
		*openptr = GGIopen;
		return 0;
	case GGIFUNC_exit:
	case GGIFUNC_close:
		*funcptr = NULL;
		return 0;
	case GG_DLENTRY_MODULES:
		modulesptr = (struct ggi_module_helper ***)funcptr;
		*modulesptr = _GGIdl_mansync;
		return GGI_OK;
	default:
		*funcptr = NULL;
	}

	return GGI_ENOTFOUND;
}
