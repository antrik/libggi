/* $Id: visual.c,v 1.30 2004/11/27 16:42:19 soyt Exp $
*****************************************************************************

   LibGGI DirectX target - Initialization

   Copyright (C) 1999 John Fortin       [fortinj@ibm.net]
   Copyright (C) 2000 Marcus Sundberg   [marcus@ggi-project.org]
   Copyright (C) 2004 Peter Ekberg      [peda@lysator.liu.se]

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
#include <ggi/internal/ggi-dl.h>
#include <ggi/internal/ggi_debug.h>
#include <ggi/display/directx.h>

#include <stdlib.h>
#include <ctype.h>
#include "ddinit.h"


typedef struct {
	HANDLE hWnd;
	HINSTANCE hInstance;
} GGIGII, *lpGGIGII;


static const gg_option optlist[] = {
	{"inwin", "no"},
	{"noinput", "no"},
	{"nocursor", "no"},
	{"physz", "0,0"},
	{"keepcursor", "no"},
	{"fullscreen", "no"}
};

#define OPT_INWIN	0
#define OPT_NOINPUT	1
#define OPT_NOCURSOR	2
#define OPT_PHYSZ	3
#define OPT_KEEPCURSOR	4
#define OPT_FULLSCREEN	5

#define NUM_OPTS	(sizeof(optlist)/sizeof(gg_option))


static int
GGI_directx_setflags(ggi_visual *vis, ggi_flags flags)
{
	directx_priv *priv = GGIDIRECTX_PRIV(vis);

	if ((LIBGGI_FLAGS(vis) & GGIFLAG_ASYNC) && !(flags & GGIFLAG_ASYNC))
		ggiFlush(vis);
	/* Clear out unknown flags */
	LIBGGI_FLAGS(vis) = flags & GGIFLAG_ASYNC;

	if(LIBGGI_FLAGS(vis) & GGIFLAG_ASYNC) {
		if (priv->timer_id)
			KillTimer(priv->hWnd, priv->timer_id);
		priv->timer_id = 0;
	}
	else
		priv->timer_id = SetTimer(priv->hWnd, 1, 33, NULL);
	return GGI_OK;
}

static int
GGIclose(ggi_visual * vis, struct ggi_dlhandle *dlh)
{
	directx_priv *priv = GGIDIRECTX_PRIV(vis);

	EnterCriticalSection(&priv->cs);
	DDShutdown(priv);
	LeaveCriticalSection(&priv->cs);
	DeleteCriticalSection(&priv->cs);
	DeleteCriticalSection(&priv->spincs);
	DeleteCriticalSection(&priv->sizingcs);
	free(priv);

	if (LIBGGI_GC(vis))
		free(LIBGGI_GC(vis));

	return 0;
}


static int
GGIopen(ggi_visual * vis, struct ggi_dlhandle *dlh,
	const char *args, void *argptr, uint32 * dlret)
{
	int err = GGI_OK;
	directx_priv *priv;
	GGIGII ggigii;
	gg_option options[NUM_OPTS];

	DPRINT("DirectX-target starting\n");

	memcpy(options, optlist, sizeof(options));

	priv = malloc(sizeof(directx_priv));
	if (priv == NULL) {
		err = GGI_ENOMEM;
		goto err0;
	}
	if ((LIBGGI_GC(vis) = malloc(sizeof(ggi_gc))) == NULL) {
		err = GGI_ENOMEM;
		goto err1;
	}

	memset(priv, 0, sizeof(directx_priv));
	LIBGGI_PRIVATE(vis) = priv;

	InitializeCriticalSection(&priv->cs);
	InitializeCriticalSection(&priv->spincs);
	priv->redraw = 1;
	priv->setpalette = 1;

	InitializeCriticalSection(&priv->sizingcs);
	priv->xmin = 0;
	priv->ymin = 0;
	priv->xmax = 0;
	priv->ymax = 0;
	priv->xstep = -1;
	priv->ystep = -1;

	if (args) {
		args = ggParseOptions((char *) args, options, NUM_OPTS);
		if (args == NULL) {
			fprintf(stderr, "display-directx: error in "
				"arguments.\n");
		}
	}

	if (_ggi_physz_parse_option(options[OPT_PHYSZ].result,
				    &(priv->physzflags), &(priv->physz))) {
		err = GGI_EARGINVAL;
		goto err3;
	}

	if (options[OPT_KEEPCURSOR].result[0] == 'n') {
		priv->cursortype =
		    (options[OPT_NOCURSOR].result[0] == 'n') ? 1 : 0;
	} else {
		priv->cursortype = 2;
	}

	if (options[OPT_INWIN].result[0] != 'n') {
		if (strcmp(options[OPT_INWIN].result, "root")) {
			priv->hParent = (HANDLE)
			    strtoul(options[OPT_INWIN].result, NULL, 0);
			if (!IsWindow(priv->hParent)) {
				fprintf(stderr,
					"display-directx: 0x%08x "
					"is not a valid window handle.\n",
					(unsigned) priv->hParent);
				priv->hParent = NULL;
			}
		} else
			priv->hParent = GetDesktopWindow();
	}

	if (options[OPT_FULLSCREEN].result[0] == 'n') {
		priv->fullscreen = 0;
	} else {
		priv->fullscreen = 1;
	}

	if (!DDInit(vis)) {
		err = GGI_ENODEVICE;
		goto err3;
	}

	ggigii.hWnd = priv->hWnd;
	ggigii.hInstance = priv->hInstance;

	if (tolower((uint8) options[OPT_NOINPUT].result[0]) == 'n' &&
	    /* FIXME: dxinput doesn't work with -inwin yet; the following
	       condition disables the default input target if -inwin has been
	       specified */
	    (!priv->hParent ||
	     getenv("GGI_INPUT") || getenv("GGI_INPUT_directx"))) {
		gii_input *inp;

		inp = giiOpen("directx", &ggigii, NULL);
		if (inp == NULL) {
			DPRINT_MISC("Unable to open directx inputlib\n");
			GGIclose(vis, dlh);
			err = GGI_ENODEVICE;
			goto err3;
		}

		priv->inp = inp;
		/* Now join the new event source in. */
		vis->input = giiJoinInputs(vis->input, inp);
	} else {
		priv->inp = NULL;
	}

	vis->opdisplay->setmode = GGI_directx_setmode;
	vis->opdisplay->getmode = GGI_directx_getmode;
	vis->opdisplay->setflags = GGI_directx_setflags;
	vis->opdisplay->checkmode = GGI_directx_checkmode;
	vis->opdisplay->flush = GGI_directx_flush;
	vis->opdisplay->getapi = GGI_directx_getapi;

	*dlret = GGI_DL_OPDISPLAY | GGI_DL_OPDRAW;
	return GGI_OK;

err3:
	DeleteCriticalSection(&priv->cs);
	DeleteCriticalSection(&priv->spincs);
	DeleteCriticalSection(&priv->sizingcs);
err2:
	free(LIBGGI_GC(vis));
err1:
	free(priv);
err0:
	return err;
}



EXPORTFUNC int
GGIdl_directx(int func, void **funcptr);

int
GGIdl_directx(int func, void **funcptr)
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
