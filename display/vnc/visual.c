/* $Id: visual.c,v 1.1 2006/08/19 23:31:32 pekberg Exp $
******************************************************************************

   Display-vnc: initialization

   Copyright (C) 2006 Peter Rosin	[peda@lysator.liu.se]

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
#include <ctype.h>
#include <unistd.h>
#include <fcntl.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "config.h"
#include <ggi/display/vnc.h>
#include <ggi/input/vnc.h>
#include <ggi/internal/ggi_debug.h>

static const gg_option optlist[] =
{
	{ "display", "no" },
};

#define OPT_DISPLAY	0

#define NUM_OPTS	(sizeof(optlist)/sizeof(gg_option))

static int
GGIopen(struct ggi_visual *vis,
	struct ggi_dlhandle *dlh,
	const char *args,
	void *argptr,
	uint32_t *dlret)
{
	ggi_vnc_priv *priv;
	gg_option options[NUM_OPTS];
	int err = 0;
	struct sockaddr_in sa;
	gii_vnc_arg iargs;
	struct gg_module *inp = NULL;
	struct gg_api *gii;

	DPRINT_MISC("coming up (args='%s').\n", args);

	memcpy(options, optlist, sizeof(options));
	args = ggParseOptions(args, options, NUM_OPTS);
	if (args == NULL) {
		fprintf(stderr, "display-vnc: error in arguments.\n");
		return GGI_EARGINVAL;
	}

	priv = LIBGGI_PRIVATE(vis) = malloc(sizeof(ggi_vnc_priv));
	if (priv == NULL)
		return GGI_ENOMEM;

	memset(priv, 0, sizeof(*priv));
	priv->sfd = -1;
	priv->cfd = -1;

	LIBGGI_GC(vis) = malloc(sizeof(ggi_gc));
	if (LIBGGI_GC(vis) == NULL) {
		err = GGI_ENOMEM;
		goto out_freepriv;
	}

	/* Handle arguments */
	if (getenv("GGI_VNC_OPTIONS") != NULL) {
		if (ggParseOptions(getenv("GGI_VNC_OPTIONS"), options,
				   NUM_OPTS) == NULL) {
			fprintf(stderr,
				"display-vnc: error in $GGI_VNC_OPTIONS.\n");
			err = GGI_EARGINVAL;
			goto out_freegc;
		}
	}

	if (options[OPT_DISPLAY].result[0] != 'n')
		priv->display = strtoul(options[OPT_DISPLAY].result, NULL, 0);
	else
		priv->display = 0;

	priv->sfd = socket(PF_INET, SOCK_STREAM, IPPROTO_IP);
	if (priv->sfd == -1) {
		err = GGI_ENODEVICE;
		goto out_freegc;
	}

	memset(&sa, 0, sizeof(sa));
	sa.sin_family = AF_INET;
	sa.sin_addr.s_addr = htonl(INADDR_ANY);
	sa.sin_port = htons(5900 + priv->display);

	if (bind(priv->sfd, (struct sockaddr *)&sa, sizeof(sa))) {
		err = GGI_ENODEVICE;
		fprintf(stderr,
			"display-vnc: bind failed\n");
		goto out_closesfd;
	}

	if (listen(priv->sfd, 3)) {
		err = GGI_ENODEVICE;
		fprintf(stderr,
			"display-vnc: listen failed\n");
		goto out_closesfd;
	}

	DPRINT("Now listening for connections.\n");

	priv->cfd = -1;

	iargs.sfd         = priv->sfd;
	iargs.new_client  = GGI_vnc_new_client;
	iargs.client_data = GGI_vnc_client_data;
	iargs.usr_ctx     = vis;

	gii = ggGetAPIByName("gii");
	if (gii == NULL && !STEM_HAS_API(vis->stem, gii)) {
		err = GGI_ENODEVICE;
		fprintf(stderr,
			"display-vnc: gii not attached to stem\n");
		goto out_closesfd;
	}

	inp = ggOpenModule(gii, vis->stem, "input-vnc", NULL, &iargs);

	DPRINT_MISC("ggOpenModule returned with %p\n", inp);

	if (inp == NULL) {
		fprintf(stderr,
			"display-vnc: unable to open vnc inputlib\n");
		err = GGI_ENODEVICE;
		goto out_closesfd;
	}

	priv->inp         = inp;
	priv->add_cfd     = iargs.add_cfd;
	priv->del_cfd     = iargs.del_cfd;
	priv->key         = iargs.key;
	priv->gii_ctx     = iargs.gii_ctx;

	vis->opdisplay->getmode=GGI_vnc_getmode;
	vis->opdisplay->setmode=GGI_vnc_setmode;
	vis->opdisplay->getapi=GGI_vnc_getapi;
	vis->opdisplay->checkmode=GGI_vnc_checkmode;
	vis->opdisplay->setflags=GGI_vnc_setflags;

	*dlret = GGI_DL_OPDISPLAY;
	return 0;

out_closesfd:
  	close(priv->sfd);
out_freegc:
	free(LIBGGI_GC(vis));
out_freepriv:
	free(priv);

	return err;
}

static int
GGIexit(struct ggi_visual *vis,
	struct ggi_dlhandle *dlh)
{
	DPRINT_MISC("going down.\n");

	return 0;
}


static int
GGIclose(struct ggi_visual *vis,
	 struct ggi_dlhandle *dlh)
{
	ggi_vnc_priv *priv = VNC_PRIV(vis);

	if (!priv)
		goto skip;

	if (priv->inp)
		ggCloseModule(priv->inp);
	priv->inp = NULL;

	if (priv->sfd != -1)
		close(priv->sfd);
	priv->sfd = -1;

	if (priv->cfd != -1)
		close(priv->sfd);
	priv->cfd = -1;

	free(priv);

skip:
	if (LIBGGI_GC(vis))
		free(LIBGGI_GC(vis));

	return 0;
}


EXPORTFUNC
int
GGIdl_vnc(int func, void **funcptr);

int
GGIdl_vnc(int func, void **funcptr)
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
		closeptr = (ggifunc_close **)funcptr;
		*closeptr = GGIclose;
		return 0;
	default:
		*funcptr = NULL;
	}

	return GGI_ENOTFOUND;
}
