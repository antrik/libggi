/* $Id: visual.c,v 1.8 2004/11/27 16:42:29 soyt Exp $
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

#include <ggi/display/vgl.h>
#include <ggi/internal/ggi_debug.h>

static int usagecounter = 0;



void _GGI_vgl_freedbs(ggi_visual *vis);

void _GGI_vgl_freedbs(ggi_visual *vis)
{
	int i;

	for (i=LIBGGI_APPLIST(vis)->num-1; i >= 0; i--) {
		_ggi_db_free(LIBGGI_APPBUFS(vis)[i]);
		_ggi_db_del_buffer(LIBGGI_APPLIST(vis), i);
	}
}

static int _GGIcheckvglmodes(ggi_visual *vis)
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

static int 
GGI_vgl_sendevent(ggi_visual *vis, gii_event *ev)
{
	DPRINT_MISC("GGI_vgl_sendevent() called\n");

	if (ev->any.type != evCommand) {
		return GGI_EEVUNKNOWN;
	}

#ifdef notyet
	switch (ev->cmd.code) {
	case GGICMD_ACKNOWLEDGE_SWITCH:
		DPRINT_MISC("display-vgl: switch acknowledge\n");
		if (priv->switchpending) {
			priv->doswitch(vis);
			return 0;
		} else {
			/* No switch pending */
			return GGI_EEVNOTARGET;
		}
		break;
	case GGICMD_NOHALT_ON_UNMAP:
		DPRINT_MISC("display-vgl: nohalt on\n");
		priv->dohalt = 0;
		priv->autoswitch = 0;
		break;
	case GGICMD_HALT_ON_UNMAP:
		DPRINT_MISC("display-vgl: halt on\n");
		priv->dohalt = 1;
		priv->autoswitch = 1;
		if (priv->switchpending) {
			/* Do switch and halt */
			priv->doswitch(vis);
			pause();
		}
		break;
	}
#endif
	
	return GGI_EEVUNKNOWN;
}

static int do_cleanup(ggi_visual *vis)
{
	vgl_priv *priv = VGL_PRIV(vis);

	if (priv->vgl_use_db)
		_GGI_vgl_freedbs(vis);

	if (vis->input != NULL) {
		giiClose(vis->input);
		vis->input = NULL;
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


static int GGIopen(ggi_visual *vis, struct ggi_dlhandle *dlh,
			const char *args, void *argptr, uint32 *dlret)
{
	vgl_priv *priv;
	int error;
	
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
	VGL_PRIV(vis) = malloc(sizeof(struct vgl_priv));
	if (VGL_PRIV(vis) == NULL) {
		do_cleanup(vis);
		error = GGI_ENOMEM;
		goto error;
	}
	priv = VGL_PRIV(vis);
	memset(priv, 0, sizeof(*priv));

	memset(priv->vgl_palred, 0, sizeof(priv->vgl_palred));
	memset(priv->vgl_palgreen, 0, sizeof(priv->vgl_palgreen));
	memset(priv->vgl_palblue, 0, sizeof(priv->vgl_palblue));

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

	/* Open keyboard and mouse input */
	if (priv->inputs & INP_KBD) {
		char *inputstr = "input-vgl";

		vis->input = giiOpen(inputstr, NULL);
		if (vis->input == NULL) {
			fprintf(stderr,
"display-vgl: Unable to open vgl, trying stdin input.\n");
			vis->input = giiOpen("stdin:ansikey", NULL);
			if (vis->input == NULL) {
				fprintf(stderr,
"display-vgl: Unable to open stdin input, try running with '-nokbd'.\n");
				do_cleanup(vis);
				error = GGI_ENODEVICE;
				goto error;
			}
		}
	}
	if (priv->inputs & INP_MOUSE) {
		gii_input *inp;
		if ((inp = giiOpen("linux-mouse:MouseSystems,/dev/sysmouse", &args, NULL)) != NULL) {
			vis->input = giiJoinInputs(vis->input, inp);
			if (vis->input == NULL) {
				fprintf(stderr, 
"display-vgl: Unable to join inputs\n");
				do_cleanup(vis);
				error = GGI_ENODEVICE;
				goto error;
			}
		}
	}

	/* Has mode management */
	vis->opdisplay->flush		= GGI_vgl_flush;
	vis->opdisplay->getmode		= GGI_vgl_getmode;
	vis->opdisplay->setmode		= GGI_vgl_setmode;
	vis->opdisplay->getapi		= GGI_vgl_getapi;
	vis->opdisplay->checkmode	= GGI_vgl_checkmode;
	vis->opdisplay->setflags	= GGI_vgl_setflags;
	vis->opdisplay->sendevent	= GGI_vgl_sendevent;

	*dlret = GGI_DL_OPDISPLAY;
	return 0;

error:
	DPRINT("display-vgl: open failed (%d)\n", error);
	return error;
}


static int GGIclose(ggi_visual *vis, struct ggi_dlhandle *dlh)
{
	return do_cleanup(vis);
}


EXPORTFUNC
int GGIdl_vgl(int func, void **funcptr);

int GGIdl_vgl(int func, void **funcptr)
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
