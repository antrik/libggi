/* $Id: visual.c,v 1.43 2007/01/11 01:05:12 pekberg Exp $
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

#define VNC_OPTIONS \
	VNC_OPTION(client,   "")           \
	VNC_OPTION(copyrect, "")           \
	VNC_OPTION(corre,    "")           \
	VNC_OPTION(display,  "no")         \
	VNC_OPTION(hextile,  "")           \
	VNC_OPTION(kold,     "no")         \
	VNC_OPTION(passwd,   "")           \
	VNC_OPTION(physz,    "0,0")        \
	VNC_OPTION(rre,      "")           \
	VNC_OPTION(server,   "default")    \
	VNC_OPTION(stdio,    "no")         \
	VNC_OPTION(tight,    "")           \
	VNC_OPTION(title,    "GGI on vnc") \
	VNC_OPTION(viewonly, "no")         \
	VNC_OPTION(viewpw,   "")           \
	VNC_OPTION(zlibhex,  "")           \
	VNC_OPTION(zlib,     "")           \
	VNC_OPTION(zrle,     "")

#define VNC_OPTION(name, default) \
	{ #name, default },
static const gg_option optlist[] = {
	VNC_OPTIONS
};
#undef VNC_OPTION

#define VNC_OPTION(name, default) \
	OPT_ ## name,
enum {
	VNC_OPTIONS
	NUM_OPTS
};
#undef VNC_OPTION

static unsigned char random17[] = "random17";

#if defined(__WIN32__) && !defined(__CYGWIN__)
static int
socket_init(void)
{
	WORD wVersionRequested;
	WSADATA wsaData;
	int err;

	wVersionRequested = MAKEWORD(2, 0);

	err = WSAStartup(wVersionRequested, &wsaData);
	if (err != 0)
		return GGI_ENODEVICE;

	if (LOBYTE(wsaData.wVersion) < 1) {
		WSACleanup();
		return GGI_ENODEVICE;
	}

	if (LOBYTE(wsaData.wVersion) == 1 && HIBYTE(wsaData.wVersion) < 1) {
		WSACleanup();
		return GGI_ENODEVICE;
	}

	if (LOBYTE(wsaData.wVersion) > 2) {
		WSACleanup();
		return GGI_ENODEVICE;
	}

	return GGI_OK;
}

static void
socket_cleanup(void)
{
	WSACleanup(); 
}
#endif

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
	char *display_memory;

	DPRINT_MISC("coming up (args='%s').\n", args);

	memcpy(options, optlist, sizeof(options));
	args = ggParseOptions(args, options, NUM_OPTS);
	if (args == NULL) {
		DPRINT_MISC("error in arguments.\n");
		return GGI_EARGINVAL;
	}

#if defined(__WIN32__) && !defined(__CYGWIN__)
	err = socket_init();
	if (err != GGI_OK)
		return err;
#endif

	priv = LIBGGI_PRIVATE(vis) = malloc(sizeof(ggi_vnc_priv));
	if (priv == NULL) {
		err = GGI_ENOMEM;
		goto out_socketcleanup;
	}

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
			DPRINT_MISC("error in $GGI_VNC_OPTIONS.\n");
			err = GGI_EARGINVAL;
			goto out_freegc;
		}
	}

	if (options[OPT_copyrect].result[0] == 'a') /* always */
		priv->copyrect = 2;
	else if (options[OPT_copyrect].result[0] == 'n') /* never */
		priv->copyrect = 0;
	else
		priv->copyrect = 1;

	if (options[OPT_corre].result[0] == 'n') /* never */
		priv->corre = 0;
	else
		priv->corre = 1;

	if (options[OPT_display].result[0] != 'n')
		priv->display = strtoul(options[OPT_display].result, NULL, 0);
	else
		priv->display = 0;

	if (options[OPT_hextile].result[0] == 'n') /* never */
		priv->hextile = 0;
	else
		priv->hextile = 1;

	if (options[OPT_kold].result[0] == 'n') /* no */
		priv->kill_on_last_disconnect = 0;
	else
		priv->kill_on_last_disconnect = 1;

	if (options[OPT_rre].result[0] == 'n') /* never */
		priv->rre = 0;
	else
		priv->rre = 1;

	if (options[OPT_passwd].result[0] != '\0') {
		FILE *f;
		char passwd[9];
		int i;

		priv->passwd = 1;
		memset(passwd, 0, sizeof(passwd));
		f = fopen(options[OPT_passwd].result, "rt");
		if (!f) {
			DPRINT_MISC("error opening passwd file.\n");
			err = GGI_EARGINVAL;
			goto out_freegc;
		}
		if (!fgets(passwd, sizeof(passwd), f)) {
			if (ferror(f)) {
				fclose(f);
				DPRINT_MISC("error reading passwd file.\n");
				err = GGI_EARGINVAL;
				goto out_freegc;
			}
		}
		fclose(f);
		if (passwd[0] && passwd[strlen(passwd) - 1] == '\n')
			passwd[strlen(passwd) - 1] = '\0';

		/* Should apparently bitreverse the password bytes.
		 * I just love undocumented quirks to standard algorithms...
		 */
		for (i = 0; i < 8; ++i)
			passwd[i] = GGI_BITREV1(passwd[i]);

		deskey((unsigned char *)passwd, EN0);
		cpkey(priv->passwd_key);

		/* Pick some random password, and use the des algorithm to
		 * generate pseudo random numbers.
		 */
		deskey(random17, EN0);
		cpkey(priv->randomizer);
	}
	else
		priv->passwd = 0;

	if (options[OPT_viewonly].result[0] == 'n') /* no */
		priv->view_only = 0;
	else
		priv->view_only = 1;

	if (options[OPT_viewpw].result[0] != '\0') {
		FILE *f;
		char viewpw[9];
		int i;

		priv->viewpw = 1;
		memset(viewpw, 0, sizeof(viewpw));
		f = fopen(options[OPT_viewpw].result, "rt");
		if (!f) {
			DPRINT_MISC("error opening viewpw file.\n");
			err = GGI_EARGINVAL;
			goto out_freegc;
		}
		if (!fgets(viewpw, sizeof(viewpw), f)) {
			if (ferror(f)) {
				fclose(f);
				DPRINT_MISC("error reading viewpw file.\n");
				err = GGI_EARGINVAL;
				goto out_freegc;
			}
		}
		fclose(f);
		if (viewpw[0] && viewpw[strlen(viewpw) - 1] == '\n')
			viewpw[strlen(viewpw) - 1] = '\0';

		/* Should apparently bitreverse the password bytes.
		 * I just love undocumented quirks to standard algorithms...
		 */
		for (i = 0; i < 8; ++i)
			viewpw[i] = GGI_BITREV1(viewpw[i]);

		deskey((unsigned char *)viewpw, EN0);
		cpkey(priv->viewpw_key);

		/* Pick some random password, and use the des algorithm to
		 * generate pseudo random numbers.
		 */
		deskey(random17, EN0);
		cpkey(priv->randomizer);
	}
	else
		priv->viewpw = 0;

	if (options[OPT_tight].result[0] == 'n') /* no */
		priv->tight = 0;
	else
		priv->tight = 1;

	ggstrlcpy(priv->title,
		options[OPT_title].result, sizeof(priv->title));

	if (options[OPT_zlib].result[0] == '\0')
		priv->zlib_level = -1; /* default compression */
	else if (options[OPT_zlib].result[0] == 'n')
		priv->zlib_level = -2; /* disable */
	else {
		priv->zlib_level =
			strtoul(options[OPT_zlib].result, NULL, 0);
		if (priv->zlib_level < 0)
			priv->zlib_level = 0;
		else if (priv->zlib_level > 9)
			priv->zlib_level = 9;
	}

	if (options[OPT_zlibhex].result[0] == '\0')
		priv->zlibhex_level = -1; /* default compression */
	else if (options[OPT_zlibhex].result[0] == 'n')
		priv->zlibhex_level = -2; /* disable */
	else {
		priv->zlibhex_level =
			strtoul(options[OPT_zlibhex].result, NULL, 0);
		if (priv->zlibhex_level < 0)
			priv->zlibhex_level = 0;
		else if (priv->zlibhex_level > 9)
			priv->zlibhex_level = 9;
	}

	if (options[OPT_zrle].result[0] == '\0')
		priv->zrle_level = -1; /* default compression */
	else if (options[OPT_zrle].result[0] == 'n')
		priv->zrle_level = -2; /* disable */
	else {
		priv->zrle_level =
			strtoul(options[OPT_zrle].result, NULL, 0);
		if (priv->zrle_level < 0)
			priv->zrle_level = 0;
		else if (priv->zrle_level > 9)
			priv->zrle_level = 9;
	}

	stem = ggNewStem(libggi, NULL);
	if (!stem) {
		err = GGI_ENOMEM;
		goto out_freegc;
	}

	display_memory = malloc(22 + strlen(options[OPT_physz].result) + 1);
	if (!display_memory) {
		err = GGI_ENOMEM;
		goto out_delstem;
	}
	strcpy(display_memory, "display-memory:-physz=");
	strcat(display_memory, options[OPT_physz].result);

	err = ggiOpen(stem, display_memory);
	free(display_memory);
	if (err != GGI_OK)
		goto out_delstem;
	priv->fb = STEM_API_DATA(stem, libggi, struct ggi_visual *);

	if (options[OPT_server].result[0] == 's')
		priv->sfd = 0; /* stdio */
	else if (options[OPT_server].result[0] != 'd' ||
	         (options[OPT_client].result[0] == '\0' &&
	          options[OPT_stdio].result[0] == 'n'))
	{
		/* default port, or override -display */
	     	short port;

		if (options[OPT_server].result[0] == 'd')
			port = priv->display;
		else
			port = strtoul(options[OPT_server].result, NULL, 0);

		priv->sfd = socket(PF_INET, SOCK_STREAM, IPPROTO_IP);
		if (priv->sfd == -1) {
			err = GGI_ENODEVICE;
			goto out_closefb;
		}

		memset(&sa, 0, sizeof(sa));
		sa.sin_family = AF_INET;
		sa.sin_addr.s_addr = htonl(INADDR_ANY);
		sa.sin_port = htons(5900 + port);

		if (bind(priv->sfd, (struct sockaddr *)&sa, sizeof(sa))) {
			err = GGI_ENODEVICE;
			DPRINT_MISC("bind failed\n");
			goto out_closefds;
		}

		if (listen(priv->sfd, 3)) {
			err = GGI_ENODEVICE;
			DPRINT_MISC("listen failed\n");
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
	if (gii == NULL || !STEM_HAS_API(vis->stem, gii)) {
		err = GGI_ENODEVICE;
		DPRINT_MISC("gii not attached to stem\n");
		goto out_closefds;
	}

	inp = ggOpenModule(gii, vis->stem, "input-vnc", NULL, &iargs);

	DPRINT_MISC("ggOpenModule returned with %p\n", inp);

	if (inp == NULL) {
		DPRINT_MISC("unable to open vnc inputlib\n");
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

	if (options[OPT_client].result[0] != '\0') {
		struct hostent *h;
		int cfd;

		h = gethostbyname(options[OPT_client].result);

		if (!h) {
			DPRINT_MISC("gethostbyname error\n");
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

		GGI_vnc_new_client_finish(vis, cfd, cfd);
	}

	if (options[OPT_stdio].result[0] != 'n')
		GGI_vnc_new_client_finish(vis, 0, 1);

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
out_socketcleanup:
#if defined(__WIN32__) && !defined(__CYGWIN__)
	socket_cleanup();
#endif

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
	struct gg_stem *stem;

	if (!priv)
		goto skip;

	priv->kill_on_last_disconnect = 0;
	while(!GG_LIST_EMPTY(&priv->clients))
		GGI_vnc_close_client(GG_LIST_FIRST(&priv->clients));

	if (priv->inp)
		ggCloseModule(priv->inp);
	priv->inp = NULL;

	if (priv->sfd != -1)
		close(priv->sfd);
	priv->sfd = -1;

	if (priv->fb) {
		stem = priv->fb->stem;
		ggiClose(stem);
		ggDelStem(stem);
	}

	free(priv);

skip:
	if (LIBGGI_GC(vis))
		free(LIBGGI_GC(vis));

#if defined(__WIN32__) && !defined(__CYGWIN__)
	socket_cleanup();
#endif

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
