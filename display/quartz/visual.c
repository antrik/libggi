/* $Id: visual.c,v 1.1 2004/12/27 20:50:33 cegger Exp $
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
#include <ggi/internal/ggi_debug.h>
#include <ggi/display/quartz.h>
#include <ggi/input/quartz.h>

static const gg_option optlist[] =
{
	{ "physz", "0,0" },
	{ "noinput", "no" },
	{ "fullscreen", "no" },
};

#define OPT_PHYSZ	0
#define OPT_NOINPUT	1
#define OPT_FULLSCREEN	2

#define NUM_OPTS	(sizeof(optlist)/sizeof(gg_option))




static int GGIclose(ggi_visual *vis, struct ggi_dlhandle *dlh)
{
	ggi_quartz_priv *priv;

	priv = QUARTZ_PRIV(vis);

	if (priv->memvis != NULL) {
		ggiClose(priv->memvis);
		free(priv->fb);
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
	free(priv);
	free(LIBGGI_GC(vis));
	vis->gamma = NULL;
	LIBGGI_PRIVATE(vis) = NULL;
	LIBGGI_GC(vis) = NULL;

	return 0;
}	/* GGIclose */


static int GGIopen(ggi_visual *vis, struct ggi_dlhandle *dlh,
			const char *args, void *argptr, uint32 *dlret)
{
	int err;
	ggi_quartz_priv *priv;
	gg_option options[NUM_OPTS];

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
		args = ggParseOptions((char *) args, options, NUM_OPTS);
		if (args == NULL) {
			fprintf(stderr, "display-quartz: error in "
				"arguments.\n");
		}	/* if */
	}	/* if */

	err = _ggi_physz_parse_option(options[OPT_PHYSZ].result,
				&(priv->physzflags),
				&(priv->physz));
	if (err != GGI_OK) goto out;

	/* windowed mode is default */
	if (tolower((uint8)options[OPT_FULLSCREEN].result[0]) != 'n') {
		/* switch over to fullscreen mode, if possible */
		DPRINT_CORE("turn on fullscreen mode");
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


	priv->windowAttrs = kWindowStandardDocumentAttributes
				| kWindowStandardHandlerAttribute
				| kWindowCompositingAttribute;
#if 0	/* This belongs into libggiwmh */
				| kWindowLiveResizeAttribute;
#endif

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

	if (tolower((uint8)options[OPT_NOINPUT].result[0]) == 'n') {
		gii_input *inp;
		gii_inputquartz_arg _args;

		_args.theWindow = priv->theWindow;

		_args.gglock = NULL;

		DPRINT_MISC("open input-quartz\n");
		inp = giiOpen("input-quartz", &_args, NULL);
		if (inp == NULL) {
			DPRINT_MISC("Unable to open quartz inputlib\n");
			err = GGI_ENODEVICE;
			fprintf(stderr, "Unable to open quartz inputlib\n");
			goto out;
		}	/* if */

		priv->inp = inp;
		/* Now join the new event source in. */
		vis->input = giiJoinInputs(vis->input, inp);
	} else {
		priv->inp = NULL;
	}	/* if */

	priv->inp = NULL;

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


static int GGIexit(ggi_visual *vis, struct ggi_dlhandle *dlh)
{
#if 0
	if (QUARTZ_PRIV(vis)->opmansync) MANSYNC_deinit(vis);
#endif
	return 0;
}     /* GGIexit */



EXPORTFUNC
int GGIdl_quartz(int func, void **funcptr);

int GGIdl_quartz(int func, void **funcptr)
{
	switch (func) {
	case GGIFUNC_open:
		*funcptr = (void *)GGIopen;
		return 0;
	case GGIFUNC_exit:
		*funcptr = (void *)GGIexit;
		return 0;
	case GGIFUNC_close:
		*funcptr = (void *)GGIclose;
		return 0;
	default:
		*funcptr = NULL;
	}	/* switch */

	return GGI_ENOTFOUND;
}	/* GGIdl_quartz */

#include <ggi/internal/ggidlinit.h>
