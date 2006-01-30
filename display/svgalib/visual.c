/* $Id: visual.c,v 1.24 2006/01/30 18:39:08 cegger Exp $
******************************************************************************

   SVGAlib target: initialization

   Copyright (C) 1998 Steve Cheng		[steve@ggi-project.org]
   Copyright (C) 1999-2000 Marcus Sundberg	[marcus@ggi-project.org]

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
#include <ggi/internal/ggi_debug.h>
#include <ggi/internal/gg_replace.h>	/* for snprintf() */
#include <ggi/display/svgalib.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>

#ifdef HAVE_STRING_H
#include <string.h>	/* memcpy */
#endif

#ifdef HAVE_SIGNAL_H
#include <signal.h>
#endif
#if defined(HAVE_SYS_VT_H) || defined(HAVE_LINUX_VT_H)
# ifdef HAVE_SYS_VT_H
# include <sys/vt.h>
# else
# include <linux/vt.h>
# endif
# define HAVE_VTSTUFF
#endif


static int usagecounter = 0;

#define RELSIG	SIGUSR1
#define ACQSIG	SIGUSR2
/* SVGAlib's signal handlers */
static const int vga_signals[] = {
	RELSIG, ACQSIG,
	SIGHUP, SIGINT, SIGQUIT, SIGILL, SIGABRT, /* == SIGIOT */
#ifdef SIGTRAP
	SIGTRAP,
#endif
#ifdef SIGBUS
	SIGBUS,
#endif
	SIGFPE,	SIGSEGV, SIGPIPE, SIGALRM, SIGTERM,
#ifdef SIGXCPU
	SIGXCPU,
#endif
#ifdef SIGXFSZ
	SIGXFSZ,
#endif
#ifdef SIGVTALRM
	SIGVTALRM,
#endif
#ifdef SIGPROF
	SIGPROF,
#endif
#ifdef SIGPWR
	SIGPWR
#endif
};
#define NUMVGASIGS	(sizeof(vga_signals)/sizeof(int))


static const gg_option optlist[] =
{
        { "physz",      "0,0" }
};
#define OPT_PHYSZ       0
#define NUM_OPTS        (sizeof(optlist)/sizeof(gg_option))

void _GGI_svga_freedbs(ggi_visual *vis)
{
	int i;

	for (i=LIBGGI_APPLIST(vis)->num-1; i >= 0; i--) {
		_ggi_db_free(LIBGGI_APPBUFS(vis)[i]);
		_ggi_db_del_buffer(LIBGGI_APPLIST(vis), i);
	}
}


static int _GGIchecksvgamodes(ggi_visual *vis)
{
	int modes = 0;
	int i;
	svga_priv *priv = SVGA_PRIV(vis);

	for (i=1; i <= vga_lastmodenumber(); i++) {
		if (vga_hasmode(i)) {
			vga_modeinfo *modeinfo;
			ggi_modelistmode *sgmode;
			int bpp, size;
			modeinfo = vga_getmodeinfo(i);
			switch(modeinfo->colors) {
			case 2:
				continue; /* 1 bpp doesn't work */
				bpp = size = 1;
				break;
			case 16:
				continue; /* 4 bpp doesn't work */
				bpp = size = 4;
				break;
			case 256:
				bpp = size = 8;
				break;
			case 1<<15:
				bpp = size = 15;
				break;
			case 1<<16:
				bpp = size = 16;
				break;
			case 1<<24:
				bpp = 24;
				if (modeinfo->bytesperpixel==3) {
					size = 24;
				} else {
					size = 32;
				}
				break;
			default:
				continue;
			}
			sgmode = priv->availmodes + modes;
			modes++;
			sgmode->x = modeinfo->width;
			sgmode->y = modeinfo->height;
			sgmode->bpp = bpp;
			sgmode->gt = GT_CONSTRUCT(bpp, (bpp <= 8) ?
			      GT_PALETTE : GT_TRUECOLOR, size);
			if ((modeinfo->flags & IS_MODEX))
				sgmode->ismodex = 1;
			else
				sgmode->ismodex = 0;
		}
	}

	if (modes == 0) {
		return GGI_ENOMATCH;
	} else {
		priv->availmodes = realloc(priv->availmodes,
					   (modes+1)*sizeof(ggi_modelistmode));
		priv->availmodes[modes].bpp = 0;
	}

	return 0;
}


static void 
do_setpalette(ggi_visual *vis)
{
	ggi_graphtype gt = LIBGGI_GT(vis);
	int len = 1 << GT_DEPTH(gt);

	vga_setpalvec(0, len, LIBGGI_PAL(vis)->priv);
}


static void
switching(void *arg)
{
	ggi_visual *vis = arg;
	svga_priv *priv = SVGA_PRIV(vis);

#if 0
	save_palette(vis);
#endif
	priv->ismapped = 0;
	priv->switchpending = 0;
	priv->gfxmode = vga_getcurrentmode();
	_ggi_svgalib_setmode(TEXT);
}

static void
switchreq(void *arg)
{
	ggi_visual *vis = arg;
	svga_priv *priv = SVGA_PRIV(vis);
	gii_event ev;
	ggi_cmddata_switchrequest *data;

	DPRINT_MISC("display-svga: switched_away() called\n");
		
	_giiEventBlank(&ev, sizeof(gii_cmd_event));

	data = (void *)ev.cmd.data;

	ev.size   = sizeof(gii_cmd_event);
	ev.cmd.type = evCommand;
	ev.cmd.code = GGICMD_REQUEST_SWITCH;
	data->request = GGI_REQSW_UNMAP;

	_giiSafeAdd(vis->input, &ev);
		
	priv->switchpending = 1;
}

static void
switchback(void *arg)
{
	ggi_visual *vis = arg;
	gii_event ev;

	DPRINT_MISC("display-svga: switched_back() called\n");

	_giiEventBlank(&ev, sizeof(gii_expose_event));

	ev.any.size   = sizeof(gii_expose_event);
	ev.any.type   = evExpose;

	ev.expose.x = ev.expose.y = 0;
	ev.expose.w = LIBGGI_VIRTX(vis);
	ev.expose.h = LIBGGI_VIRTY(vis);

	_giiSafeAdd(vis->input, &ev);
	DPRINT_MISC("svga: EXPOSE sent.\n");

	_ggi_svgalib_setmode(SVGA_PRIV(vis)->gfxmode);
	do_setpalette(vis);
	SVGA_PRIV(vis)->ismapped = 1;
}


static int 
GGI_svga_sendevent(ggi_visual *vis, gii_event *ev)
{
	svga_priv *priv = SVGA_PRIV(vis);

	DPRINT_MISC("GGI_svga_sendevent() called\n");

	if (ev->any.type != evCommand) {
		return GGI_EEVUNKNOWN;
	}
	switch (ev->cmd.code) {
	case GGICMD_ACKNOWLEDGE_SWITCH:
		DPRINT_MISC("display-svga: switch acknowledge\n");
		if (priv->switchpending) {
			priv->doswitch(vis);
			return 0;
		} else {
			/* No switch pending */
			return GGI_EEVNOTARGET;
		}
		break;
	case GGICMD_NOHALT_ON_UNMAP:
		DPRINT_MISC("display-svga: nohalt on\n");
		priv->dohalt = 0;
		priv->autoswitch = 0;
		break;
	case GGICMD_HALT_ON_UNMAP:
		DPRINT_MISC("display-svga: halt on\n");
		priv->dohalt = 1;
		priv->autoswitch = 1;
		if (priv->switchpending) {
			/* Do switch and halt */
			priv->doswitch(vis);
			pause();
		}
		break;
	}
	
	return GGI_EEVUNKNOWN;
}


/* This is exported from svgalib, we use it to check if
   we're called from GGI's svgalib wrapper. */
#define GSW_MAGIC	(-4711)
extern int __svgalib_tty_fd;

static int do_cleanup(ggi_visual *vis)
{
	svga_priv *priv = SVGA_PRIV(vis);

	DPRINT("display-svga: GGIdlcleanup start.\n");

	/* Restore to text mode */
	_ggi_svgalib_setmode(TEXT);

	_GGI_svga_freedbs(vis);

	if (vis->input != NULL) {
		giiClose(vis->input);
		vis->input = NULL;
	}

	if (priv) {
		if (priv->availmodes != NULL) {
			free(priv->availmodes);
		}
		free(priv);
	}
	free(LIBGGI_GC(vis));

	ggUnregisterCleanup((ggcleanup_func *)do_cleanup, vis);
	
	usagecounter--;

	DPRINT("display-svga: GGIdlcleanup done.\n");
	return 0;
}


static int GGIopen(ggi_visual *vis, struct ggi_dlhandle *dlh,
			const char *args, void *argptr, uint32_t *dlret)
{
	gg_option options[NUM_OPTS];
	ggi_linvtsw_arg vtswarg;
	int  vtnum = -1, novt = 0;
	svga_priv *priv;
	struct sigaction old_signals[NUMVGASIGS];
#ifdef HAVE_VTSTUFF
	struct vt_mode temp_vtmode;
#endif
	unsigned int i;
	int err;

	memcpy(options, optlist, sizeof(options));
        if (args != NULL) {
                args = ggParseOptions(args, options, NUM_OPTS);
                if (args == NULL) {
                        fprintf(stderr, "display-x: error in arguments.\n");
                        return GGI_EARGINVAL;
                }
        }
	
	if (__svgalib_tty_fd == GSW_MAGIC) {
		ggiPanic("SVGAlib target called from the SVGAlib wrapper!"
			 " Terminating.\n");
	}
	
	ggLock(_ggi_global_lock); /* Entering protected section */
	if (usagecounter > 0) {
		ggUnlock(_ggi_global_lock);
		fprintf(stderr, "display-svga: You can only open this target "
			"once in an application.\n");
		return GGI_EBUSY;
	}
	usagecounter++;
	ggUnlock(_ggi_global_lock); /* Exiting protected section */

	/* Save original signal handlers because SVGAlib will set its own */
	for(i = 0; i < NUMVGASIGS; i++) {
		sigaction(vga_signals[i], NULL, old_signals+i);
	}

	if (!_ggiDebug) vga_disabledriverreport();

	if (vga_init()) {
		fprintf(stderr, "display-SVGAlib: vga_init() failed\n");
		usagecounter--;
		return GGI_ENODEVICE;
	}

#ifdef HAVE_VTSTUFF
	/* Save VT mode */
	ioctl(__svgalib_tty_fd, VT_GETMODE, &temp_vtmode);
#endif

	/* Trigger the setup code in SVGAlib */
	_ggi_svgalib_setmode(TEXT);

#if 0
	/* Save SVGAlib VT handlers for our own use */
	do {
		struct sigaction sa;

		sigaction(RELSIG, NULL, &sa);
		release = sa.sa_handler;
		sigaction(ACQSIG, NULL, &sa);
		acquire = sa.sa_handler;
	} while(0);
#endif

#ifdef HAVE_VTSTUFF
	/* Restore VT mode */
	ioctl(__svgalib_tty_fd, VT_SETMODE, &temp_vtmode);
#endif

	/* Get rid of SVGAlib's signal handlers */
	for (i = 0; i < NUMVGASIGS; i++) {
		sigaction(vga_signals[i], old_signals+i, NULL);
	}

	ggRegisterCleanup((ggcleanup_func *)do_cleanup, vis);
	ggCleanupForceExit();
    
	LIBGGI_GC(vis) = malloc(sizeof(ggi_gc));
	if (LIBGGI_GC(vis) == NULL) {
		usagecounter--;
		return GGI_ENOMEM;
	}
	priv = LIBGGI_PRIVATE(vis) = malloc(sizeof(struct svga_priv));
	if (priv == NULL) {
		do_cleanup(vis);
		return GGI_ENOMEM;
	}
	LIBGGI_PAL(vis)->priv = NULL;
	priv->inputs = INP_KBD | INP_MOUSE;
	priv->dohalt = 1;
	priv->autoswitch = 1;
	priv->switchpending = 0;
	priv->ismapped = 1;
	priv->doswitch = NULL;

	err = _ggi_physz_parse_option(options[OPT_PHYSZ].result, 
                               &(priv->physzflags), &(priv->physz)); 
        if (err != GGI_OK) {
          do_cleanup(vis);
          return err;
        }


	priv->availmodes = 
		malloc(vga_lastmodenumber()*sizeof(ggi_modelistmode));
	if (priv->availmodes == NULL) {
		do_cleanup(vis);
		return GGI_ENOMEM;
	}
	if (_GGIchecksvgamodes(vis) != 0) {
		return GGI_ENODEVICE;
	}

	do {
		vtswarg.switchreq = switchreq;
		vtswarg.switching = switching;
		vtswarg.switchback = switchback;
		vtswarg.funcarg = vis;

		vtswarg.dohalt     = &priv->dohalt;
		vtswarg.autoswitch = &priv->autoswitch;
		vtswarg.onconsole = 1;
		if (getenv("GGI_NEWVT")) {
			vtswarg.forcenew = 1;
		} else {
			vtswarg.forcenew = 0;
		}
		vtswarg.novt = novt;

		err = _ggiAddDL(vis, _ggiGetConfigHandle(),
				"helper-linux-vtswitch", NULL,
				&vtswarg, 0);
		if (!err) {
			vtnum = vtswarg.vtnum;
			priv->doswitch = vtswarg.doswitch;
		} else {
			vtnum = -1;
			priv->doswitch = NULL;
		}
	} while (0);

	if (vtswarg.refcount > 1) {
		/* No inputs unless we're first */
		DPRINT_MISC("display-svga: linvtsw refcount: %d\n",
			       vtswarg.refcount); 
		priv->inputs = 0;
	}

	/* Open keyboard and mouse input */
	if (priv->inputs & INP_KBD) {
		char strbuf[64];
		const char *inputstr = "input-linux-kbd";

		if (vtnum != -1) {
			snprintf(strbuf, 64, "linux-kbd:/dev/tty%d", vtnum);
			inputstr = strbuf;
		}

		vis->input = giiOpen(inputstr, NULL);
		if (vis->input == NULL) {
			if (vtnum != -1) {
				snprintf(strbuf, 64, "linux-kbd:/dev/vc/%d", vtnum);
				vis->input = giiOpen(inputstr, NULL);
			}
			if (vis->input == NULL) {
				fprintf(stderr,
"display-svga: Unable to open linux-kbd, trying stdin input.\n");
				/* We're on the Linux console so we want
				   ansikey. */
				vis->input = giiOpen("stdin:ansikey", NULL);
				if (vis->input == NULL) {
					fprintf(stderr,
"display-svga: Unable to open stdin input, try running with '-nokbd'.\n");
					do_cleanup(vis);
					return GGI_ENODEVICE;
				}
			}
		}
	}
	if (priv->inputs & INP_MOUSE) {
		gii_input *inp;
		if ((inp = giiOpen("linux-mouse:auto", &args, NULL)) != NULL) {
			vis->input = giiJoinInputs(vis->input, inp);
		}
	}

	/* Has mode management */
	vis->opdisplay->flush		= GGI_svga_flush;
	vis->opdisplay->getmode		= GGI_svga_getmode;
	vis->opdisplay->setmode		= GGI_svga_setmode;
	vis->opdisplay->getapi		= GGI_svga_getapi;
	vis->opdisplay->checkmode	= GGI_svga_checkmode;
	vis->opdisplay->setflags	= GGI_svga_setflags;
	vis->opdisplay->sendevent	= GGI_svga_sendevent;

	*dlret = GGI_DL_OPDISPLAY;
	return 0;
}


static int GGIclose(ggi_visual *vis, struct ggi_dlhandle *dlh)
{
	return do_cleanup(vis);
}


EXPORTFUNC
int GGIdl_svgalib(int func, void **funcptr);

int GGIdl_svgalib(int func, void **funcptr)
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

#include <ggi/internal/ggidlinit.h>
