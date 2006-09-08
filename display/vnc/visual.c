/* $Id: visual.c,v 1.15 2006/09/08 19:43:01 pekberg Exp $
******************************************************************************

   display-vnc: initialization

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

#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#ifdef HAVE_STRING_H
#include <string.h>
#endif
#include <ctype.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include <sys/types.h>
#ifdef HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif
#ifdef HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif
#ifdef HAVE_NETDB_H
#include <netdb.h>
#endif

#include <ggi/display/vnc.h>
#include <ggi/input/vnc.h>
#include <ggi/internal/ggi_debug.h>

#include "d3des.h"

static const gg_option optlist[] =
{
	{ "client",  "" },
	{ "display", "no" },
	{ "passwd",  "" },
	{ "title",   "GGI on vnc" },
	{ "zlib",    "" },
	{ "zrle",    "" },
};

#define OPT_CLIENT	0
#define OPT_DISPLAY	1
#define OPT_PASSWD	2
#define OPT_TITLE	3
#define OPT_ZLIB	4
#define OPT_ZRLE	5

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
	struct gg_stem *stem;
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
	GG_LIST_INIT(&priv->clients);

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

	if (options[OPT_PASSWD].result[0] != '\0') {
		unsigned char passwd[8];
		int i;

		priv->passwd = 1;
		memset(passwd, 0, sizeof(passwd));
		strncpy(passwd, options[OPT_PASSWD].result, 8);

		/* Should apparently bitreverse the password bytes.
		 * I just love undocumented quirks to standard algorithms...
		 */
		for (i = 0; i < 8; ++i)
			passwd[i] = GGI_BITREV1(passwd[i]);

		deskey(passwd, EN0);
		cpkey(priv->cooked_key);

		/* Pick some random password, and use the des algorithm to
		 * generate pseudo random numbers.
		 */
		deskey("random17", EN0);
		cpkey(priv->randomizer);
	}
	else
		priv->passwd = 0;

	ggstrlcpy(priv->title,
		options[OPT_TITLE].result, sizeof(priv->title));

	if (options[OPT_ZLIB].result[0] == '\0')
		priv->zlib_level = -1; /* default compression */
	else if (options[OPT_ZLIB].result[0] == 'n')
		priv->zlib_level = -2; /* disable */
	else {
		priv->zlib_level =
			strtoul(options[OPT_ZLIB].result, NULL, 0);
		if (priv->zlib_level < 0)
			priv->zlib_level = 0;
		else if (priv->zlib_level > 9)
			priv->zlib_level = 9;
	}

	if (options[OPT_ZRLE].result[0] == '\0')
		priv->zrle_level = -1; /* default compression */
	else if (options[OPT_ZRLE].result[0] == 'n')
		priv->zrle_level = -2; /* disable */
	else {
		priv->zrle_level =
			strtoul(options[OPT_ZRLE].result, NULL, 0);
		if (priv->zrle_level < 0)
			priv->zrle_level = 0;
		else if (priv->zrle_level > 9)
			priv->zrle_level = 9;
	}

	stem = ggNewStem();
	if (!stem) {
		err = GGI_ENOMEM;
		goto out_freegc;
	}
	err = ggiAttach(stem);
	if (err != GGI_OK)
		goto out_delstem;
	err = ggiOpen(stem, "display-memory");
	if (err != GGI_OK)
		goto out_delstem;
	priv->fb = STEM_API_DATA(stem, libggi, struct ggi_visual *);

	if (options[OPT_CLIENT].result[0] == '\0') {
		priv->sfd = socket(PF_INET, SOCK_STREAM, IPPROTO_IP);
		if (priv->sfd == -1) {
			err = GGI_ENODEVICE;
			goto out_closefb;
		}

		memset(&sa, 0, sizeof(sa));
		sa.sin_family = AF_INET;
		sa.sin_addr.s_addr = htonl(INADDR_ANY);
		sa.sin_port = htons(5900 + priv->display);

		if (bind(priv->sfd, (struct sockaddr *)&sa, sizeof(sa))) {
			err = GGI_ENODEVICE;
			fprintf(stderr,
				"display-vnc: bind failed\n");
			goto out_closefds;
		}

		if (listen(priv->sfd, 3)) {
			err = GGI_ENODEVICE;
			fprintf(stderr,
				"display-vnc: listen failed\n");
			goto out_closefds;
		}

		DPRINT("Now listening for connections.\n");
	}

	iargs.sfd          = priv->sfd;
	iargs.new_client   = GGI_vnc_new_client;
	iargs.client_data  = GGI_vnc_client_data;
	iargs.write_client = GGI_vnc_write_client;
	iargs.usr_ctx      = vis;

	gii = ggGetAPIByName("gii");
	if (gii == NULL && !STEM_HAS_API(vis->stem, gii)) {
		err = GGI_ENODEVICE;
		fprintf(stderr,
			"display-vnc: gii not attached to stem\n");
		goto out_closefds;
	}

	inp = ggOpenModule(gii, vis->stem, "input-vnc", NULL, &iargs);

	DPRINT_MISC("ggOpenModule returned with %p\n", inp);

	if (inp == NULL) {
		fprintf(stderr,
			"display-vnc: unable to open vnc inputlib\n");
		err = GGI_ENODEVICE;
		goto out_closefds;
	}

	priv->inp         = inp;
	priv->add_cfd     = iargs.add_cfd;
	priv->del_cfd     = iargs.del_cfd;
	priv->add_cwfd    = iargs.add_cwfd;
	priv->del_cwfd    = iargs.del_cwfd;
	priv->key         = iargs.key;
	priv->pointer     = iargs.pointer;
	priv->gii_ctx     = iargs.gii_ctx;

	if (options[OPT_CLIENT].result[0] != '\0') {
		struct hostent *h;
		int cfd;

		h = gethostbyname(options[OPT_CLIENT].result);

		if (!h) {
			fprintf(stderr, "display-vnc: gethostbyname error\n");
			err = GGI_ENODEVICE;
			goto out_closefds;
		}

		cfd = socket(PF_INET, SOCK_STREAM, IPPROTO_IP);
		if (cfd == -1) {
			err = GGI_ENODEVICE;
			goto out_closefds;
		}

		memset(&sa, 0, sizeof(sa));
		sa.sin_family      = AF_INET;
		sa.sin_addr        = *((struct in_addr *)h->h_addr);
		sa.sin_port        = htons(5500 + priv->display);

		if (connect(cfd, (struct sockaddr *)&sa, sizeof(sa))) {
			err = GGI_ENODEVICE;
			close(cfd);
			goto out_closefds;
		}

		GGI_vnc_new_client_finish(vis, cfd);
	}

	vis->opdisplay->getmode=GGI_vnc_getmode;
	vis->opdisplay->setmode=GGI_vnc_setmode;
	vis->opdisplay->getapi=GGI_vnc_getapi;
	vis->opdisplay->checkmode=GGI_vnc_checkmode;
	vis->opdisplay->setflags=GGI_vnc_setflags;

	*dlret = GGI_DL_OPDISPLAY;
	return 0;

out_closefds:
	if (priv->inp)
		ggCloseModule(inp);
	if (priv->sfd != -1)
		close(priv->sfd);
out_closefb:
	ggiClose(priv->fb->stem);
out_delstem:
	ggDelStem(stem);
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
	struct gg_stem *tmp_stem;

	if (!priv)
		goto skip;

	while(!GG_LIST_EMPTY(&priv->clients))
		GGI_vnc_close_client(GG_LIST_FIRST(&priv->clients));

	if (priv->inp)
		ggCloseModule(priv->inp);
	priv->inp = NULL;

	if (priv->sfd != -1)
		close(priv->sfd);
	priv->sfd = -1;

	if (priv->fb) {
		tmp_stem = priv->fb->stem;
		ggiClose(tmp_stem);
		ggDelStem(tmp_stem);
	}

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
