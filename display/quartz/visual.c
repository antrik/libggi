/* $Id: visual.c,v 1.24 2007/03/11 10:05:26 cegger Exp $
******************************************************************************

   Display-quartz: initialization

   Copyright (C) 2002 Christoph Egger

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
#include <string.h>

#include "config.h"
#include "quartz.h"
#include <ggi/internal/ggi_debug.h>
#include <ggi/input/quartz.h>

static const gg_option optlist[] =
{
	{ "physz", "0,0" },
	{ "noinput", "no" },
	{ "nomansync", "no" },
	{ "fullscreen", "no" },
};

#define OPT_PHYSZ	0
#define OPT_NOINPUT	1
#define OPT_NOMANSYNC	2
#define OPT_FULLSCREEN	3

#define NUM_OPTS	(sizeof(optlist)/sizeof(gg_option))




static int GGIclose(struct ggi_visual *vis, struct ggi_dlhandle *dlh)
{
	ggi_quartz_priv *priv;

	priv = QUARTZ_PRIV(vis);

	DPRINT_LIBS("GGIclose(%p, %p) called\n", vis, dlh);

	if (priv->memvis != NULL) {
		ggiClose(priv->memvis->instance.stem);
		free(priv->fb);
		priv->memvis = NULL;
	}

	if (priv->inp) {
		ggDelInstance(priv->inp);
		priv->inp = NULL;
	}

	/* Restore original screen resolution/bpp */
	if (priv->fullscreen) {
		CGDisplaySwitchToMode (priv->display_id, priv->save_mode);
		CGDisplayRelease (priv->display_id);
		CGPaletteRelease (priv->palette);

		/* Ensure the cursor will be visible and working when we quit */
		CGDisplayShowCursor (priv->display_id);
		CGAssociateMouseAndMouseCursorPosition (1);
	}

	/* Restore gamma settings */
	CGDisplayRestoreColorSyncSettings ();

	if (vis->gamma) free(vis->gamma);
	if (priv->opmansync) free(priv->opmansync);
	free(priv);
	free(LIBGGI_GC(vis));
	vis->gamma = NULL;
	LIBGGI_PRIVATE(vis) = NULL;
	LIBGGI_GC(vis) = NULL;

	return 0;
}	/* GGIclose */


static int GGIopen(struct ggi_visual *vis, struct ggi_dlhandle *dlh,
			const char *args, void *argptr, uint32_t *dlret)
{
	int err;
	ggi_quartz_priv *priv;
	gg_option options[NUM_OPTS];

	DPRINT_LIBS("GGIopen(%p, %p, \"%s\", %p) called\n",
		vis, dlh, args, argptr);

	memcpy(options, optlist, sizeof(options));

	LIBGGI_GC(vis) = calloc(1, sizeof(ggi_gc));
	DPRINT_MISC("Allocated graphic context: %p\n", (void *)LIBGGI_GC(vis));
	if (LIBGGI_GC(vis) == NULL) {
		goto err0;
	}

	/* Allocate descriptor for screen memory */
	priv = calloc(1,sizeof(ggi_quartz_priv));
	DPRINT_MISC("Allocated private structure: %p\n", (void *)priv);
	if (priv == NULL) {
		goto err1;
	}	/* if */

	LIBGGI_PRIVATE(vis) = priv;

	/* Allocate Gamma Map memory */
	vis->gamma = calloc((size_t)1U, sizeof(struct ggi_gammastate));
	DPRINT_MISC("Allocated gamma map: %p\n", (void *)vis->gamma);
	if (vis->gamma == NULL) {
		goto err2;
	}

	if (args) {
		args = ggParseOptions(args, options, NUM_OPTS);
		if (args == NULL) {
			fprintf(stderr, "display-quartz: error in "
				"arguments.\n");
		}	/* if */
	}	/* if */

	err = _ggi_physz_parse_option(options[OPT_PHYSZ].result,
				&(priv->physzflags),
				&(priv->physz));
	if (err != GGI_OK) goto out;

	if (tolower((uint8_t)options[OPT_NOMANSYNC].result[0]) == 'n') {

		priv->opmansync = malloc(sizeof(_ggi_opmansync));
		if (priv->opmansync == NULL) {
			err = GGI_ENOMEM;
			goto out;
		}	/* if */

		DPRINT_LIBS("loading helper-mansync\n");
		priv->mod_mansync = ggCreateModuleInstance(libggi,
					vis->instance.stem,
					"helper-mansync", NULL,
					priv->opmansync);
		if (priv->mod_mansync == NULL) {
			DPRINT_LIBS("loading helper-mansync failed\n");
			free(priv->opmansync);
			priv->opmansync = NULL;
			err = GGI_ENODEVICE;
			goto out;
		}
	}	/* if */

	/* windowed mode is default */
	if (tolower((uint8_t)options[OPT_FULLSCREEN].result[0]) != 'n') {
		/* switch over to fullscreen mode, if possible */
		DPRINT_MISC("turn on fullscreen mode");
		priv->fullscreen = 1;
	}


#if !defined(MACOSX_FINDER_SUPPORT)
	do {
		/* This chunk of code is heavily based off SDL_macosx.m from SDL
		 * It uses an Apple private function to request foreground operation.
		 */

		extern void CPSEnableForegroundOperation(ProcessSerialNumber *psn);
		ProcessSerialNumber myProc, frProc;
		Boolean sameProc;

		if (GetFrontProcess(&frProc) != noErr) break;
		if (GetCurrentProcess(&myProc) != noErr) break;
		if (SameProcess(&frProc, &myProc, &sameProc) == noErr && !sameProc) {
			CPSEnableForegroundOperation(&myProc);
		}
		SetFrontProcess(&myProc);
	} while(0);
#endif

	/* Initialize the video settings; this data persists between mode switches */
	priv->display_id = CGMainDisplayID();
	priv->save_mode  = CGDisplayCurrentMode    (priv->display_id);
	priv->cur_mode  = CGDisplayCurrentMode    (priv->display_id);
	priv->mode_list  = CGDisplayAvailableModes (priv->display_id);
	priv->palette    = CGPaletteCreateDefaultColorPalette ();

	/* Put up the blanking window (a window above all other windows) */
	if ( priv->fullscreen && CGDisplayNoErr != CGDisplayCapture (priv->display_id) ) {
		/* Failed capturing display */
		goto out;
	}	/* if */


	priv->windowAttrs = kWindowCollapseBoxAttribute
				| kWindowStandardHandlerAttribute;

	vis->opdisplay->checkmode	= GGI_quartz_checkmode;
	vis->opdisplay->setmode		= GGI_quartz_setmode;
	vis->opdisplay->getmode		= GGI_quartz_getmode;

	vis->opdisplay->getapi		= GGI_quartz_getapi;
	vis->opdisplay->setflags	= GGI_quartz_setflags;
	vis->opdisplay->flush		= GGI_quartz_flush;


#if 0
	vis->opgc			= GGI_quartz_gcchanged;
#endif

#if 0
	vis->opcolor->setpalvec		= GGI_quartz_setpalvec;
	vis->opcolor->setgamma		= GGI_quartz_setgamma;
	vis->opcolor->getgamma		= GGI_quartz_getgamma;
	vis->opcolor->setgammamap	= GGI_quartz_setgammamap;
	vis->opcolor->getgammamap	= GGI_quartz_getgammamap;
#endif

	if (tolower((uint8_t)options[OPT_NOINPUT].result[0]) == 'n') {
		struct gg_instance *inp = NULL;
		gii_inputquartz_arg _args;
		struct gg_api *gii;

		_args.theWindow = priv->theWindow;

		_args.gglock = NULL;

		DPRINT_MISC("open input-quartz\n");
		gii = ggGetAPIByName("gii");
		if (gii != NULL && STEM_HAS_API(vis->instance.stem, gii)) {
			inp = ggCreateModuleInstance(gii,
						     vis->instance.stem,
						     "input-quartz",
						     NULL,
						     &_args);
		} else {
			err = GGI_ENODEVICE;
			fprintf(stderr,
				"display-quartz: gii not attached to stem\n");
			goto out;
		}

		DPRINT_MISC("ggCreateModuleInstance returned with %p\n", inp);
		if (inp == NULL) {
			err = GGI_ENODEVICE;
			fprintf(stderr, "display-quartz: Unable to open quartz inputlib\n");
			goto out;
		}	/* if */

		ggObserve(inp->channel, GGI_quartz_listener, vis);
		priv->inp = inp;
	} else {

		DPRINT_MISC("no input handling\n");
		priv->inp = NULL;
	}	/* if */


	if (priv->opmansync) {
		MANSYNC_init(vis);
		if (!(LIBGGI_FLAGS(vis) & GGIFLAG_ASYNC)) {
			MANSYNC_start(vis);
		}	/* if */
	}	/* if */


	*dlret = GGI_DL_OPDISPLAY;
	return 0;

 out:
	DPRINT("GGIopen: out\n");
	GGIclose(vis, dlh);
	return err;

err2:
	free(priv);
err1:
	free(LIBGGI_GC(vis));
err0:
	return GGI_ENOMEM;
}	/* GGIopen */


static int GGIexit(struct ggi_visual *vis, struct ggi_dlhandle *dlh)
{
	ggi_quartz_priv *priv;

	LIB_ASSERT(vis != NULL, "GGIexit: vis == NULL\n");

	priv = QUARTZ_PRIV(vis);
	LIB_ASSERT(priv != NULL, "GGIexit: QUARTZ_PRIV(vis) == NULL\n");
	DPRINT_LIBS("GGIexit(%p, %p) called\n", vis, dlh);

	if (priv->opmansync) {
		DPRINT_LIBS("deinitializing and unloading helper-mansync\n");
		if (!(LIBGGI_FLAGS(vis) & GGIFLAG_ASYNC)) {
			MANSYNC_stop(vis);
		}
		MANSYNC_deinit(vis);
		ggDelInstance(priv->mod_mansync);
	}

	return 0;
}	/* GGIexit */



EXPORTFUNC
int GGIdl_quartz(int func, void **funcptr);

int GGIdl_quartz(int func, void **funcptr)
{
	ggifunc_open **openptr;
	ggifunc_exit **exitptr;
	ggifunc_close **closeptr;

	switch (func) {
	case GGIFUNC_open:
		openptr = (ggifunc_open **)funcptr;
		*openptr = GGIopen;
		return 0;
	case GGIFUNC_exit:
		exitptr = (ggifunc_exit **)funcptr;
		*exitptr = GGIexit;
		return 0;
	case GGIFUNC_close:
		closeptr = (ggifunc_exit **)funcptr;
		*closeptr = GGIclose;
		return 0;
	default:
		*funcptr = NULL;
	}	/* switch */

	return GGI_ENOTFOUND;
}	/* GGIdl_quartz */
