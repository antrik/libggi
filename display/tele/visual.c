/* $Id: visual.c,v 1.3 2003/07/06 10:25:23 cegger Exp $
******************************************************************************

   Teletarget.

   Copyright (C) 1998 Andrew Apted	[andrew@ggi-project.org]
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

#include <stdlib.h>
#include <string.h>

#include "config.h"
#include <ggi/internal/ggi-dl.h>

#include "libtele.h"
#include <ggi/display/tele.h>


static int GGIclose(ggi_visual *vis, struct ggi_dlhandle *dlh)
{
	ggi_tele_priv *priv = TELE_PRIV(vis);

	if (priv->mode_up) {
		GGI_tele_resetmode(vis);
	}

	if (vis->input) {
		giiClose(vis->input);
		vis->input = NULL;
	}

	if (priv->connected) {
		tclient_close(priv->client);
		priv->connected = 0;
	}

	free(priv->client);
	free(LIBGGI_GC(vis));
	free(priv);

	return 0;
}

static int GGIopen(ggi_visual *vis, struct ggi_dlhandle *dlh,
			const char *args, void *argptr, uint32 *dlret)
{
	ggi_tele_priv *priv;
	int err = GGI_ENOMEM;

	/* initialize */
	priv = LIBGGI_PRIVATE(vis) = malloc(sizeof(ggi_tele_priv));
	if (priv == NULL) return GGI_ENOMEM;

	LIBGGI_GC(vis) = malloc(sizeof(ggi_gc));
	if (LIBGGI_GC(vis) == NULL) {
		goto out_freepriv;
	}

	priv->client = malloc(sizeof(TeleClient));
	if (priv->client == NULL) {
		goto out_freegc;
	}

	priv->connected = 0;
	priv->mode_up = 0;
	priv->wait_event = NULL;

	/* connect to the server */
	fprintf(stderr, "Connecting to the TeleServer...\n");

	if (args) {
		err = tclient_open(priv->client, args);
	} else {
		err = tclient_open(priv->client, "inet:127.0.0.1:27780");
	}
	if (err < 0) {
		err = GGI_ENODEVICE;
		goto out_freeclient;
	}

	priv->connected = 1;

	fprintf(stderr, "... connection established.\n");

	/* set up GII */
	GGIDPRINT_MISC("gii starting\n");

	/* first allocate a new gii_input descriptor */
	if ((priv->input = _giiInputAlloc()) == NULL) {
		GGIDPRINT_MISC("giiInputAlloc failure.\n");
		GGIclose(vis, dlh);
		return GGI_ENOMEM;
	}
	GGIDPRINT_MISC("gii input=%p\n", priv->input);

	/* now fill in the blanks */
	priv->input->priv = priv;
	priv->input->targetcan = emAll;
	priv->input->GIIseteventmask(priv->input, priv->input->targetcan);
	priv->input->maxfd = 0;	/* this is polled */
	priv->input->flags |= GII_FLAGS_HASPOLLED;

	/* We only need the "poll" function.  For all others, the
	 * defaults are fine.
	 */
	priv->input->GIIeventpoll = GII_tele_poll;
	/* now join the new event source in */
	vis->input = giiJoinInputs(vis->input, priv->input);

	/* set up function pointers */
	vis->opdisplay->getmode=GGI_tele_getmode;
	vis->opdisplay->setmode=GGI_tele_setmode;
	vis->opdisplay->checkmode=GGI_tele_checkmode;
	vis->opdisplay->flush=GGI_tele_flush;

	*dlret = GGI_DL_OPDISPLAY;
	return 0;

  out_freeclient:
	free(priv->client);
  out_freegc:
	free(LIBGGI_GC(vis));
  out_freepriv:
	free(priv);

	return err;
}

int GGI_tele_flush(ggi_visual *vis, int x, int y, int w, int h, int tryflag)
{
	ggi_tele_priv *priv = TELE_PRIV(vis);
	TeleEvent ev;

	int err;

	if (! priv->mode_up) {
		return -1;
	}

	tclient_new_event(priv->client, &ev, TELE_CMD_FLUSH, 0, 0);

	err = tclient_write(priv->client, &ev);

	if (err == TELE_ERROR_SHUTDOWN) {
		TELE_HANDLE_SHUTDOWN;
	}

	return err;
}


int GGIdl_tele(int func, void **funcptr);

int GGIdl_tele(int func, void **funcptr)
{
	switch (func) {
	case GGIFUNC_open:
		*funcptr = GGIopen;
		return 0;
	case GGIFUNC_exit:
		*funcptr = NULL;
		return 0;
	case GGIFUNC_close:
		*funcptr = GGIclose;
		return 0;
	default:
		*funcptr = NULL;
	}

	return GGI_ENOTFOUND;
}

#include <ggi/internal/ggidlinit.h>
