/* $Id: visual.c,v 1.21 2006/10/16 05:34:11 cegger Exp $
******************************************************************************

   FreeBSD vgl(3) target: initialization

   Copyright (C) 2000 Alcove - Nicolas Souchu <nsouch@freebsd.org>

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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <ctype.h>

#include <ggi/display/vgl.h>
#include <ggi/internal/ggi_debug.h>
#include <ggi/gii.h>
#include <ggi/gii-module.h>

static int usagecounter = 0;


ggfunc_channel_control_cb _ggi_vgl_listener;

static const gg_option optlist[] =
{
	{ "nokbd", "no" },
	{ "nomouse", "no" },
	{ "noinput", "no" },
	{ "novt", "no" },
	{ "physz", "0,0" },
	{ ":dev", "" }
};

#define OPT_NOKBD	0
#define OPT_NOMOUSE	1
#define OPT_NOINPUT	2
#define OPT_NOVT	3
#define OPT_PHYSZ	4
#define OPT_DEV		5

#define NUM_OPTS	(sizeof(optlist)/sizeof(gg_option))

#define MAX_DEV_LEN	63
#define DEFAULT_FBNUM	0



static void
switchreq(void *arg)
{
	struct ggi_visual *vis = arg;
	vgl_priv *priv = VGL_PRIV(vis);
	ggi_cmddata_switchrequest data;

	DPRINT_MISC("switchreq(%p) called\n", vis);

	data.request = GGI_REQSW_UNMAP;

	ggBroadcast(priv->linvt_channel, GGICMD_REQUEST_SWITCH, &data);
	priv->switchpending = 1;
}


static void
switching(void *arg)
{
	struct ggi_visual *vis = arg;
	vgl_priv *priv = VGL_PRIV(vis);

	DPRINT_MISC("switching(%p) called\n", vis);

	priv->ismapped = 0;
	priv->switchpending = 0;
}


static void
switchback(void *arg)
{
	struct ggi_visual *vis = arg;
	vgl_priv *priv = VGL_PRIV(vis);
	gii_event ev;

	DPRINT_MISC("switched_back(%p) called\n", vis);

	giiEventBlank(&ev, sizeof(gii_expose_event));

	ev.any.size = sizeof(gii_expose_event);
	ev.any.type = evExpose;

	ev.expose.x = ev.expose.y = 0;
	ev.expose.w = LIBGGI_VIRTX(vis);
	ev.expose.h = LIBGGI_VIRTY(vis);

	ggBroadcast(priv->linvt_channel, GII_CMDCODE_EXPOSE, &ev);
	DPRINT_MISC("EXPOSE sent.\n");


	/* See notes about palette member reuse in color.c */
	priv->ismapped = 1;
}


int
_ggi_vgl_listener(void *arg, uint32_t flag, void *data)
{
	struct ggi_visual *vis = arg;
	vgl_priv *priv = VGL_PRIV(vis);

	DPRINT_MISC("_ggi_vgl_listener() called\n");

	if ((flag & GGICMD_ACKNOWLEDGE_SWITCH) == GGICMD_ACKNOWLEDGE_SWITCH) {
		DPRINT_MISC("listener: switch acknowledge\n");
		if (priv->switchpending) {
			priv->doswitch(vis);
		}
	}

	if ((flag & GGICMD_NOHALT_ON_UNMAP) == GGICMD_NOHALT_ON_UNMAP) {
		DPRINT_MISC("listener: nohalt on\n");
		priv->dohalt = 0;
		priv->autoswitch = 0;
	}

	if ((flag & GGICMD_HALT_ON_UNMAP) == GGICMD_HALT_ON_UNMAP) {
		DPRINT_MISC("listener: halt on\n");
		priv->dohalt = 1;
		priv->autoswitch = 1;
		if (priv->switchpending) {
			/* Do switch and halt */
			priv->doswitch(vis);
			pause();
		}
	}

	return 0;
}





void _GGI_vgl_freedbs(struct ggi_visual *vis);

void _GGI_vgl_freedbs(struct ggi_visual *vis)
{
	int i;

	for (i=LIBGGI_APPLIST(vis)->num-1; i >= 0; i--) {
		_ggi_db_free(LIBGGI_APPBUFS(vis)[i]);
		_ggi_db_del_buffer(LIBGGI_APPLIST(vis), i);
	}
}

static int _GGIcheckvglmodes(struct ggi_visual *vis)
{
	struct video_info modeinfo;
	int modes = 0, error;
	int i;
	vgl_priv *priv = VGL_PRIV(vis);

	DPRINT_MISC("display-vgl: Checking modes\n");

	for (i=1; i <= M_VESA_MODE_MAX; i++) {
		modeinfo.vi_mode = i;

		/* XXX should be added to libvgl and not do the ioctl here */
		if ((error = ioctl(0, CONS_MODEINFO, &modeinfo))) {
			;
		} else if (modeinfo.vi_mode == i) {
			ggi_modelistmode *sgmode;
			int bpp, size;

			switch (modeinfo.vi_depth) {
			case 1:
			case 4:
			case 8:
			case 15:
			case 16:
			case 24:
				bpp = modeinfo.vi_depth;
				break;
			case 32:
				bpp = 24;
				break;
			default:
				continue;
			}
			size = modeinfo.vi_pixel_size * 8;	/* XXX 1 char is 8 bits */
			sgmode = &priv->availmodes[modes];
			modes++;
			sgmode->x = modeinfo.vi_width;
			sgmode->y = modeinfo.vi_height;
			sgmode->bpp = bpp;
			sgmode->gt = GT_CONSTRUCT(bpp, (bpp <= 8) ?
			      GT_PALETTE : GT_TRUECOLOR, size);

			DPRINT_MISC("display-vgl: found mode %dx%dx%d %d\n",
				sgmode->x, sgmode->y, sgmode->bpp, size);
		}
	}

	if (modes == 0) {
		DPRINT_MISC("display-vgl: no mode!\n");
		return GGI_ENOMATCH;
	} else {
		priv->availmodes = realloc(priv->availmodes,
					   (modes+1)*sizeof(ggi_modelistmode));
		priv->availmodes[modes].bpp = 0;
	}

	return 0;
}


static int do_cleanup(struct ggi_visual *vis)
{
	vgl_priv *priv = VGL_PRIV(vis);

	if (priv->vgl_use_db)
		_GGI_vgl_freedbs(vis);

	if (priv->kbd_inp != NULL) {
		ggCloseModule(priv->kbd_inp);
		priv->kbd_inp = NULL;
	}
	if (priv->ms_inp != NULL) {
		ggCloseModule(priv->ms_inp);
		priv->ms_inp = NULL;
	}

	if (priv->vgl_init_done) {
		priv->vgl_init_done = 0;
		VGLEnd();
	}

	if (priv->availmodes != NULL) {
		free(priv->availmodes);
	}

	free(LIBGGI_GC(vis));

	ggUnregisterCleanup((ggcleanup_func *)do_cleanup, vis);
	
	usagecounter--;

	return 0;
}


static int GGIopen(struct ggi_visual *vis, struct ggi_dlhandle *dlh,
			const char *args, void *argptr, uint32_t *dlret)
{
	vgl_priv *priv;
	int error;
	ggi_linvtsw_arg vtswarg;
	gg_option options[NUM_OPTS];
	struct gg_api *gii;
	int novt = 0;
	

	DPRINT("GGIopen start.\n");

	memcpy(options, optlist, sizeof(options));

	if (args) {
		args = ggParseOptions(args, options, NUM_OPTS);
		if (args == NULL) {
			fprintf(stderr, "display-vgl: error in arguments.\n");
			return GGI_EARGINVAL;
		}
	}

	gii = ggGetAPIByName("gii");
	ggLock(_ggi_global_lock); /* Entering protected section */
	if (usagecounter > 0) {
		ggUnlock(_ggi_global_lock);
		fprintf(stderr, "display-vgl: You can only open this target "
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
	LIBGGI_PRIVATE(vis) = priv = malloc(sizeof(struct vgl_priv));
	if (priv == NULL) {
		do_cleanup(vis);
		error = GGI_ENOMEM;
		goto error;
	}
	memset(priv, 0, sizeof(*priv));

	memset(priv->vgl_palred, 0, sizeof(priv->vgl_palred));
	memset(priv->vgl_palgreen, 0, sizeof(priv->vgl_palgreen));
	memset(priv->vgl_palblue, 0, sizeof(priv->vgl_palblue));

	priv->dohalt = 1;
	priv->autoswitch = 1;
	priv->switchpending = 0;
	priv->ismapped = 1;
	priv->doswitch = NULL;

	/* Default is no DirectBuffer */
	priv->vgl_use_db = 0;

	/* XXX check if the modes are lfb capable */

	if (args) {
		if (strncmp(args, "-usedb:", 7) == 0) {
			DPRINT_MISC("display-vgl: Enabling DB\n");
			priv->vgl_use_db = 1;
			args += 7;
		}
		if (strncmp(args, "-nodb:", 6) == 0) {
			DPRINT_MISC("display-vgl: Disabling DB\n");
			priv->vgl_use_db = 0;
			args += 6;
		}
	}

	priv->inputs = INP_KBD | INP_MOUSE;

	priv->availmodes = malloc(M_VESA_MODE_MAX*sizeof(ggi_modelistmode));
	if (priv->availmodes == NULL) {
		do_cleanup(vis);
		error = GGI_ENOMEM;
		goto error;
	}
	if (_GGIcheckvglmodes(vis) != 0) {
		error = GGI_ENODEVICE;
		goto error;
	}


	priv->linvt_channel = ggNewChannel(vis, _ggi_vgl_listener));
	priv->kbd_inp = NULL;
	priv->ms_inp = NULL;


	priv->inputs = INP_KBD | INP_MOUSE;

	if (toupper((uint8_t)options[OPT_NOKBD].result[0]) != 'N') {
		priv->inputs &= ~INP_KBD;
	}
	if (toupper((uint8_t)options[OPT_NOMOUSE].result[0]) != 'N') {
		priv->inputs &= ~INP_MOUSE;
	}
	if (toupper((uint8_t)options[OPT_NOINPUT].result[0]) != 'N') {
		priv->inputs &= ~(INP_KBD | INP_MOUSE);
	}
	if (toupper((uint8_t)options[OPT_NOVT].result[0]) != 'N') {
		priv->inputs = 0;
		novt = 1;
	}

	DPRINT("Parsing phyzs options.\n");
	do {
		int err;
		err = _ggi_physz_parse_option(options[OPT_PHYSZ].result,
					&(priv->physzflags), &(priv->physz));
		if (err != GGI_OK) {
			do_cleanup(vis);
			return err;
		}
	} while (0);
 

	vtswarg.switchreq = switchreq;
	vtswarg.switching = switching;
	vtswarg.switchback = switchback;
	vtswarg.funcarg = vis;

	vtswarg.dohalt = &priv->dohalt;
	vtswarg.autoswitch = &priv->autoswitch;
	vtswarg.onconsole = 1;
	if (getenv("GGI_NEWVT")) {
		vtswarg.forcenew = 1;
	} else {
		vtswarg.forcenew = 0;
	}
	vtswarg.novt = novt;


	/* Open keyboard and mouse input */
	if (priv->inputs & INP_KBD) {
		struct gg_module *inp;

		if (gii != NULL && STEM_HAS_API(vis->stem, gii)) {
			inp = ggOpenModule(gii, vis->stem,
					"input-vgl", NULL, NULL);
			if (inp == NULL) {
				fprintf(stderr,
"display-vgl: Unable to open vgl, trying stdin input.\n");
				inp = ggOpenModule(gii, vis->stem,
						"stdin", "ansikey", NULL);
				if (inp == NULL) {
					fprintf(stderr,
"display-vgl: Unable to open stdin input, try running with '-nokbd'.\n");
					do_cleanup(vis);
					error = GGI_ENODEVICE;
					goto error;
				}
			}
			priv->kbd_inp = inp;
		}
	}
	if (priv->inputs & INP_MOUSE) {
		struct gg_module *inp;
		if (gii != NULL && STEM_HAS_API(vis->stem, gii)) {
			inp = ggOpenModule(gii, vis->stem,
					"input-linux-mouse", "MouseSystems,/dev/sysmouse", &args);
			if (inp == NULL) {
				fprintf(stderr, 
		"display-vgl: Unable to join inputs\n");
				do_cleanup(vis);
				error = GGI_ENODEVICE;
				goto error;
			}
			priv->ms_inp = inp;
		}
	}

	/* Has mode management */
	vis->opdisplay->flush		= GGI_vgl_flush;
	vis->opdisplay->getmode		= GGI_vgl_getmode;
	vis->opdisplay->setmode		= GGI_vgl_setmode;
	vis->opdisplay->getapi		= GGI_vgl_getapi;
	vis->opdisplay->checkmode	= GGI_vgl_checkmode;
	vis->opdisplay->setflags	= GGI_vgl_setflags;

	*dlret = GGI_DL_OPDISPLAY;
	return 0;

error:
	DPRINT("display-vgl: open failed (%d)\n", error);
	return error;
}


static int GGIclose(struct ggi_visual *vis, struct ggi_dlhandle *dlh)
{
	return do_cleanup(vis);
}


EXPORTFUNC
int GGIdl_vgl(int func, void **funcptr);

int GGIdl_vgl(int func, void **funcptr)
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
