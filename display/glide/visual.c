/* $Id: visual.c,v 1.16 2008/01/19 23:18:39 cegger Exp $
******************************************************************************

   GLIDE target - Initialization

   Copyright (C) 1997-1998	Jon Taylor	[taylorj@ecs.csus.edu]
   Copyright (C) 1998-2000	Marcus Sundberg	[marcus@ggi-project.org]

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
#include <string.h>

#include "config.h"
#include <ggi/display/glide.h>
#include <ggi/display/linvtsw.h>
#include <ggi/internal/gg_replace.h>


void _GGI_glide_freedbs(struct ggi_visual *vis)
{
	int i;
	int first = LIBGGI_APPLIST(vis)->first_targetbuf;
	int last = LIBGGI_APPLIST(vis)->last_targetbuf;
	
	if (first < 0) {
		return;
	}
	for (i = (last - first); i >= 0; i--) {
		while (LIBGGI_APPBUFS(vis)[i+first]->resource->count > 0) {
			ggiResourceRelease(LIBGGI_APPBUFS(vis)[i+first]->resource);
		}
		free(LIBGGI_APPBUFS(vis)[i+first]->resource);
		_ggi_db_free(LIBGGI_APPBUFS(vis)[i+first]);
		_ggi_db_del_buffer(LIBGGI_APPLIST(vis), i+first);
	}
	LIBGGI_APPLIST(vis)->first_targetbuf = -1;
}


static void
switching(void *vis)
{
	if (GLIDE_PRIV((struct ggi_visual*)vis)->setmodesuccess) {
		grSstControl(GR_CONTROL_DEACTIVATE);
	}
}


static void
switchback(void *vis)
{
	if (GLIDE_PRIV((struct ggi_visual*)vis)->setmodesuccess) {
		grSstControl(GR_CONTROL_ACTIVATE);
	}
}


static int GGIopen(struct ggi_visual *vis, struct ggi_dlhandle *dlh,
			const char *args, void *argptr, uint32_t *dlret)
{
	ggi_linvtsw_arg vtswarg;
	GrHwConfiguration hwconf;
	int currentcard = 0;
	glide_priv *priv;
	char strbuf[64];
	const char *inputname;
	char *str;
	int vtnum = -1, novt = 0, useinput = 1;
	int on_linux_cons;
	gii_input *inp;
	
	DPRINT("GLIDE-lib starting\n");

	if (args != NULL) {
		currentcard = strtol(args, NULL, 0);
		if (currentcard < 0) currentcard = 0;
	}

	/* YUCK! There's no way to determine if we are allowed to access
	   the 3DFX hardware, and if we can't we get a segfault here. :-(
	*/
	grSstQueryBoards(&hwconf);

	if (hwconf.num_sst < 1) {
		fprintf(stderr, "display-glide: No 3DFX cards detected!\n");
		return GGI_ENODEVICE;
	} else if (currentcard >= hwconf.num_sst) {
		fprintf(stderr, "display-glide: Can't use card number %d,"
			" only %d 3DFX card%s present.\n",
			currentcard + 1, hwconf.num_sst,
			hwconf.num_sst > 1 ? "s" : "");
		return GGI_ENODEVICE;
	}

	priv = calloc(1, sizeof(glide_priv));
	if (priv == NULL) {
		return GGI_ENOMEM;
	}
	LIBGGI_GC(vis) = calloc(1, sizeof(ggi_gc));
	if (LIBGGI_GC(vis) == NULL) {
		free(priv);
		return GGI_ENOMEM;
	}
	
	if (getenv("DISPLAY") != NULL) {
		inputname = "x";
	} else {
		inputname = "input-linux-kbd";
	}
	if (strstr(inputname, "input-linux-kbd") != NULL) {
		priv->autoswitch = 1;

		vtswarg.switchreq = NULL;
		vtswarg.switching = switching;
		vtswarg.switchback = switchback;
		vtswarg.funcarg = vis;

		if (getenv("GGI_GLIDE_HALTONSWITCH")) {
			priv->dohalt = 1;
		} else {
			priv->dohalt = 0;
		}
		vtswarg.dohalt     = &priv->dohalt;
		vtswarg.autoswitch = &priv->autoswitch;
		vtswarg.onconsole = 0;
		if (getenv("GGI_NEWVT")) {
			vtswarg.forcenew = 1;
		} else {
			vtswarg.forcenew = 0;
		}
		vtswarg.novt = novt;

		priv->module_vtswitch = ggPlugModule(libggi,
						vis->instance.stem,
						"helper-linux-vtswitch",
						NULL, &vtswarg);
		if (priv->module_vtswitch) == NULL) {
			vtnum = -1;
		} else {
			vtnum = vtswarg.vtnum;
		}

		if (vtswarg.refcount > 1) {
			/* No inputs unless we're first */
			useinput = 0;
		}

		if (vtnum != -1) {
			snprintf(strbuf, sizeof(strbuf),
				"input-linux-kbd:/dev/tty%d", vtnum);
			inputname = strbuf;
		}
		on_linux_cons = 1;
	} else {
		on_linux_cons = 0;
	}

	/* Open keyboard and mouse input */
	if (useinput) {
		vis->input = giiOpen(inputname, NULL);
		if (vis->input == NULL) {
			if (vtnum != -1) {
				snprintf(strbuf, sizeof(strbuf),
					"linux-kbd:/dev/vc/%d", vtnum);
				vis->input = giiOpen(inputname, NULL);
			}
			if (vis->input == NULL) {
				fprintf(stderr,
					"display-glide: Couldn't open input.\n");
				free(LIBGGI_GC(vis));
				free(priv);
				return GGI_ENODEVICE;
			}
		}
		if (on_linux_cons) {
			inp = giiOpen("linux-mouse:auto", &args, NULL);
			if (inp != NULL) {
				vis->input = giiJoinInputs(vis->input, inp);
			}
		}
	}

	/* Prevent 3DFX splash-screen from being displayed */
	if (getenv("FX_GLIDE_NO_SPLASH") == NULL) {
		putenv("FX_GLIDE_NO_SPLASH=1");
	}

	grGlideInit();
	grSstSelect(currentcard);
	
	str = getenv("GGI_GLIDE_MAXFREQ");
	if (str != NULL) {
		priv->maxvfreq = strtol(str, NULL, 0);
	} else {
		priv->maxvfreq = GGIGLIDE_DEFAULT_VFREQ;
	}
	str = getenv("GGI_GLIDE_MAXHFREQ");
	if (str != NULL) {
		priv->maxhfreq = strtol(str, NULL, 0);
	} else {
		priv->maxhfreq = GGIGLIDE_DEFAULT_HFREQ;
	}
	priv->currentcard = currentcard;
	grSstQueryHardware(&priv->hwconf);
	priv->fbmem
		= priv->hwconf.SSTs[currentcard].sstBoard.VoodooConfig.fbRam
		* 1024 * 1024;
	priv->setmodesuccess = 0;
	MONOTEXT_PRIV(vis) = priv;

	/* Has mode management */
	vis->opdisplay->flush = GGI_glide_flush;
	vis->opdisplay->getmode = GGI_glide_getmode;
	vis->opdisplay->setmode = GGI_glide_setmode;
	vis->opdisplay->checkmode = GGI_glide_checkmode;
	vis->opdisplay->getapi = GGI_glide_getapi;
	vis->opdisplay->flush = GGI_glide_flush;

	*dlret = GGI_DL_OPDISPLAY;
	return 0;
}

static int GGIclose(struct ggi_visual *vis, struct ggi_dlhandle *dlh)
{
	glide_priv *priv = MONOTEXT_PRIV(vis);

	giiClose(vis->input);
	vis->input = NULL;

	if (priv->module_vtswitch != NULL) {
		ggClosePlugin(priv->module_vtswitch);
		priv->module_vtswitch = NULL;
	}

	free(priv);
	free(LIBGGI_GC(vis));
	
	grGlideShutdown();

	return 0;
}


EXPORTFUNC
int GGIdl_glide(int func, void **funcptr);

int GGIdl_glide(int func, void **funcptr)
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
