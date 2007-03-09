/* $Id: visual.c,v 1.29 2007/03/09 08:22:17 antrik Exp $
******************************************************************************

   Display-kgi: initialization

   Copyright (C) 1995 Andreas Beck      [andreas@ggi-project.org]
   Copyright (C) 1997 Jason McMullan    [jmcc@ggi-project.org]
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

#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include <ggi/display/kgi.h>
#include <ggi/internal/gg_replace.h>
#include <ggi/internal/ggi_debug.h>

static const gg_option optlist[] =
{
	{ "device", "/dev/graphic,/dev/kgi/graphic" },
	{ "no3d", "no" },
	{ "swatchsize",  "auto" },
	{ "noinput",  "no" }
};

static int GGIopen(struct ggi_visual *vis, struct ggi_dlhandle *dlh,
			const char *args, void *argptr, uint32_t *dlret)
{
	kgi_version_t version = { 0, 0, 1, 0 };
	gg_option options[KGI_NUM_OPTS];

	LIBGGI_PRIVATE(vis) = calloc(1, sizeof(ggi_kgi_priv));
	if (KGI_PRIV(vis) == NULL)
		return GGI_ENOMEM;

	LIBGGI_GC(vis) = calloc(1, sizeof(ggi_gc));
	if(LIBGGI_GC(vis) == NULL)
		goto err_freepriv;
	
	memcpy(options, optlist, sizeof(options));
	if (args) {
		args = ggParseOptions((char*)args, options, KGI_NUM_OPTS);
		if (args == NULL) {
			DPRINT_LIBS("Error in arguments\n");
			goto err_freepriv;
		}
	}
	
	if(kgiInit(&KGI_CTX(vis), "ggi", &version, options) != KGI_EOK){
		DPRINT_LIBS("Unable to initialize kgi\n");
		goto err_freegc;
	}

	KGI_PRIV(vis)->use3d = (options[KGI_OPT_NO3D].result[0] == 'n');

	if (!strncmp(options[KGI_OPT_SWATCHSIZE].result, "auto", 4)) {
		KGI_PRIV(vis)->swatch_size = 0;
	} else {
		KGI_PRIV(vis)->swatch_size = 
		  strtoul(options[KGI_OPT_SWATCHSIZE].result, NULL, 10);
		if (KGI_PRIV(vis)->swatch_size < 2048) {
			KGI_PRIV(vis)->swatch_size = -1;
		}
	}

	/* accel sublib private data */
	KGI_ACCEL_PRIV(vis) = NULL;

	KGI_PRIV(vis)->map_accel = GGI_kgi_map_accelerator;

	/* Has mode management */
	vis->opdisplay->getmode   = GGI_kgi_getmode;
	vis->opdisplay->setmode   = GGI_kgi_setmode;
	vis->opdisplay->checkmode = GGI_kgi_checkmode;
	vis->opdisplay->getapi    = GGI_kgi_getapi;
	vis->opdisplay->setflags  = GGI_kgi_setflags;

	/* If GGI_INPUT or GII_INPUT is set, it overrides the default
	 * behaviour of KGI display: let GGI open the input. Otherwise,
	 * try to open the /dev/event corresponding to ower /dev/graphic
	 */
	if (tolower((uint8_t)options[KGI_OPT_NOINPUT].result[0]) == 'n') {
		struct gg_api *gii;
		char eventname[32];

		snprintf(eventname, sizeof(eventname), "-device=/dev/event%i",
			 KGI_CTX(vis).mapper.graphic);

		gii = ggGetAPIByName("gii");
		if (gii != NULL && STEM_HAS_API(vis->module.stem, gii)) {
			KGI_PRIV(vis)->inp = ggOpenModule(gii,
				vis->module.stem, "input-kii", eventname,
				NULL);
		}
		if (KGI_PRIV(vis)->inp == NULL) {
			DPRINT_MISC("Unable to open KII inputlib\n");
			goto err_freegc;
		}
	}

	KGI_PRIV(vis)->ilut_touched = 1;

	*dlret = GGI_DL_OPDISPLAY | GGI_DL_OPDRAW;
	return 0;

 err_freegc:
	free(LIBGGI_GC(vis));
 err_freepriv:
	free(KGI_PRIV(vis));
	     
	return GGI_ENOMEM;
}

static int GGIclose(struct ggi_visual *vis, struct ggi_dlhandle *dlh)
{
	if (LIBGGI_FD(vis) > -1)
		close(LIBGGI_FD(vis));

	if (KGI_PRIV(vis)->inp) {
		ggCloseModule(KGI_PRIV(vis)->inp);
		KGI_PRIV(vis)->inp = NULL;
	}

	return 0;
}
		

EXPORTFUNC
int GGIdl_kgi(int func, void **funcptr);

int GGIdl_kgi(int func, void **funcptr)
{
	ggifunc_open **openptr;
	ggifunc_close **closeptr;

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
