/* $Id: visual.m,v 1.1 2002/12/22 12:59:38 cegger Exp $
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
#include <ggi/display/quartz.h>

static const gg_option optlist[] =
{
	{ "physz", "0,0" },
	{ "noinput", "no" },
};

#define OPT_PHYSZ	0
#define OPT_NOINPUT	1

#define NUM_OPTS	(sizeof(optlist)/sizeof(gg_option))




static int GGIclose(ggi_visual *vis, struct ggi_dlhandle *dlh)
{
	ggi_quartz_priv *priv;
	NSAutoreleasePool *pool;

	priv = QUARTZ_PRIV(vis);

	pool = [ [ NSAutoreleasePool alloc ] init ];

	[ priv->GGIApp terminate:priv->window ];

	/* Restore original screen resolution/bpp */
	CGDisplaySwitchToMode (priv->display_id, priv->save_mode);
	CGDisplayRelease (priv->display_id);
	CGPaletteRelease (priv->palette);

	/* Restore gamma settings */
	CGDisplayRestoreColorSyncSettings ();

	/* Ensure the cursor will be visible and working when we quit */
	CGDisplayShowCursor (priv->display_id);
	CGAssociateMouseAndMouseCursorPosition (1);

	free(LIBGGI_PRIVATE(vis));
	free(LIBGGI_GC(vis));
	LIBGGI_PRIVATE(vis) = NULL;
	LIBGGI_GC(vis) = NULL;

	[ pool release ];

	return 0;
}	/* GGIclose */


static int GGIopen(ggi_visual *vis, struct ggi_dlhandle *dlh,
			const char *args, void *argptr, uint32 *dlret)
{
	int err;
	ggi_quartz_priv *priv;
	gg_option options[NUM_OPTS];

	NSAutoreleasePool *pool;

	GGIDPRINT_MISC("display-quartz coming up.\n");

	memcpy(options, optlist, sizeof(options));

	LIBGGI_GC(vis) = calloc(1, sizeof(ggi_gc));
	if (!LIBGGI_GC(vis)) return GGI_ENOMEM;

	/* Allocate descriptor for screen memory */
	priv = LIBGGI_PRIVATE(vis) = calloc(1,sizeof(ggi_quartz_priv));
	if (!priv) {
		free(LIBGGI_GC(vis));
		return GGI_ENOMEM;
	}	/* if */

	if (args) {
		args = ggParseOptions((char *) args, options, NUM_OPTS);
		if (args == NULL) {
			fprintf(stderr, "display-quartz: error in "
				"arguments.\n");
		}	/* if */
	}	/* if */

	fprintf(stderr, "create pool\n");

	pool = [ [ NSAutoreleasePool alloc ] init ];

	err = _ggi_parse_physz(options[OPT_PHYSZ].result,
				&(priv->physzflags),
				&(priv->physz));
	if (err != GGI_OK) goto out;

	fprintf(stderr, "call sharedApplication\n");

	/* Ensure the application object is initialized */
	[ NSApplication sharedApplication ];

	priv->GGIApp = NSApp;
	priv->window = [ priv->GGIApp keyWindow ];

	/* Initialize the video settings; this data persists between mode switches */
	priv->display_id = CGMainDisplayID();
	priv->save_mode  = CGDisplayCurrentMode    (priv->display_id);
	priv->cur_mode  = CGDisplayCurrentMode    (priv->display_id);
	priv->mode_list  = CGDisplayAvailableModes (priv->display_id);
	priv->palette    = CGPaletteCreateDefaultColorPalette ();

	/* Put up the blanking window (a window above all other windows) */
	if ( CGDisplayNoErr != CGDisplayCapture (priv->display_id) ) {
		/* Failed capturing display */
		goto out;
	}	/* if */


	vis->opdisplay->checkmode	= GGI_quartz_checkmode;
	vis->opdisplay->setmode		= GGI_quartz_setmode;
	vis->opdisplay->getmode		= GGI_quartz_getmode;

	vis->opdisplay->getapi		= GGI_quartz_getapi;
	vis->opdisplay->setflags	= GGI_quartz_setflags;
	vis->opdisplay->flush		= GGI_quartz_flush;

#if 0
	vis->opgc			= GGI_quartz_gcchanged;
#endif

	vis->opcolor->setpalvec		= GGI_quartz_setpalvec;
	vis->opcolor->setgamma		= GGI_quartz_setgamma;
	vis->opcolor->getgamma		= GGI_quartz_getgamma;
	vis->opcolor->setgammamap	= GGI_quartz_setgammamap;
	vis->opcolor->getgammamap	= GGI_quartz_getgammamap;


	if (tolower((int)options[OPT_NOINPUT].result[0]) == 'n') {
		gii_input *inp;
		gii_inputcocoa_arg args;

		args.GGIApp = NSApp;
		args.window = priv->window;
#if 0
		args.gglock = lock;
#endif
		fprintf(stderr, "giiOpen(cocoa)\n");

		inp = giiOpen("cocoa", &args, NULL);
		if (inp == NULL) {
			GGIDPRINT_MISC("Unable to open cocoa inputlib\n");
			err = GGI_ENODEVICE;
			fprintf(stderr, "Unable to open cocoa inputlib\n");
			goto out;
		}	/* if */

		priv->inp = inp;
		/* Now join the new event source in. */
		vis->input = giiJoinInputs(vis->input, inp);
	} else {
		priv->inp = NULL;
	}	/* if */

	fprintf(stderr, "release pool\n");

	[ pool release ];

	*dlret = GGI_DL_OPDISPLAY;
	return 0;

 out:
	fprintf(stderr, "release pool\n");

	[ pool release ];

	fprintf(stderr, "GGIopen: out\n");
	GGIclose(vis, dlh);
	return err;
}	/* GGIopen */


static int GGIexit(ggi_visual *vis, struct ggi_dlhandle *dlh)
{
#if 0
	if (QUARTZ_PRIV(vis)->opmansync) MANSYNC_deinit(vis);
#endif
	return 0;
}     /* GGIexit */



int GGIdl_quartz(int func, void **funcptr)
{
	switch (func) {
	case GGIFUNC_open:
		*funcptr = GGIopen;
		return 0;
	case GGIFUNC_exit:
		*funcptr = GGIexit;
		return 0;
	case GGIFUNC_close:
		*funcptr = GGIclose;
		return 0;
	default:
		*funcptr = NULL;
	}	/* switch */

	return GGI_ENOTFOUND;
}	/* GGIdl_quartz */

#include <ggi/internal/ggidlinit.h>
