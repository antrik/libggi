/* $Id: visual.c,v 1.29 2007/03/08 20:54:07 soyt Exp $
******************************************************************************

   display-ipc: transfer drawing commands to other processes

   Copyright (C) 2001 Stefan Seefeld	[stefan@berlin-consortium.org]

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
#include <ggi/display/ipc.h>
#include <ggi/internal/ggi_debug.h>
#include <ggi/gii-events.h>

#include <stdlib.h>
#include <string.h>
#include <errno.h>

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

static const gg_option optlist[] =
{
	{ "socket", "" },
	{ "semid", "" },
	{ "shmid", "" },
	{ "input", "" },
	{ "physz", "0,0" }
};

#define OPT_SOCKET	0
#define OPT_SEMID	1
#define OPT_SHMID	2
#define OPT_INPUT	3
#define OPT_PHYSZ	4

#define NUM_OPTS	(sizeof(optlist)/sizeof(gg_option))


int GGI_ipc_flush(struct ggi_visual *vis, int x, int y, int w, int h,
		  int tryflag)
{
	char buffer[32];
	ipc_priv *priv = IPC_PRIV(vis);

	if (priv->sockfd == -1) return 0;

	/* may be some day we want to send something
	 * other than flush messages...
	 */
	buffer[0] = 'F';
	memcpy(buffer + 1, &x, sizeof(int));
	memcpy(buffer + 1 + sizeof(int), &y, sizeof(int));
	memcpy(buffer + 1 + 2*sizeof(int), &w, sizeof(int));
	memcpy(buffer + 1 + 3*sizeof(int), &h, sizeof(int));

	write(priv->sockfd, buffer, 1 + 4*sizeof(int));

	return 0;
}	/* GGI_ipc_flush */



static int GGIopen(struct ggi_visual *vis, struct ggi_dlhandle *dlh,
		   const char *args, void *argptr, uint32_t *dlret)
{
	ipc_priv *priv;
	gg_option options[NUM_OPTS];
	struct sockaddr_un address;
	struct gg_api *gii;
	int err = 0;
	int rc;

	DPRINT_MISC("display-ipc coming up.\n");
	memcpy(options, optlist, sizeof(options));

	if (!args) {
		fprintf(stderr, "display-ipc: required arguments missing\n");
		return GGI_EARGREQ;
	}	/* if */

	args = ggParseOptions(args, options, NUM_OPTS);
	if (args == NULL) {
		fprintf(stderr, "display-ipc: error in arguments.\n");
		return GGI_EARGINVAL;
	}	/* if */

	LIBGGI_GC(vis) = malloc(sizeof(ggi_gc));
	if (!LIBGGI_GC(vis)) return GGI_ENOMEM;

	/* Allocate descriptor for screen memory */
	priv = malloc(sizeof(ipc_priv));

	if (!priv) {
		err = GGI_ENOMEM;
		goto err0;
	}	/* if */

	LIBGGI_PRIVATE(vis) = priv;
	priv->inputbuffer = NULL;	/* Default to no input */

	if (_ggi_physz_parse_option(options[OPT_PHYSZ].result,
			     &(priv->physzflags), &(priv->physz)))
	{
		err = GGI_EARGINVAL;
		goto err1;
        }	/* if */

	if (!options[OPT_SOCKET].result[0]
	   && !options[OPT_SEMID].result[0]
	   && !options[OPT_SHMID].result[0])
	{
		DPRINT("required arguments missing\n");
		err = GGI_EARGREQ;
		goto err1;
	}	/* if */

	if (!(sscanf(options[OPT_SOCKET].result,"%s", address.sun_path)
	   && sscanf(options[OPT_SEMID].result,"%d", &(priv->semid))
	   && sscanf(options[OPT_SHMID].result,"%d", &(priv->shmid))))
	{
		DPRINT("argument format error\n");
		err = GGI_EARGINVAL;
		goto err1;
	}	/* if */

	DPRINT("parsed args: socket: %s semid: %i shmid: %i\n",
		   address.sun_path, priv->semid, priv->shmid);
	address.sun_family = AF_UNIX;
	priv->sockfd = socket(PF_UNIX, SOCK_STREAM, 0);
	if (priv->sockfd == -1) {
		DPRINT("could not open stream socket\n");
		err = GGI_ENODEVICE;
		goto err1;
	}

	rc = connect(priv->sockfd, (const struct sockaddr *)(&address),
			sizeof(struct sockaddr_un));
	if (rc == -1) {
		DPRINT("could not connect to socket\n");
		err = GGI_ENODEVICE;
		goto err2;
	}

	priv->memptr = (char *)shmat(priv->shmid, NULL, 0);
	if (priv->memptr == (char *)-1) {
		DPRINT("initialization failed : %s\n", strerror(errno));
		err = GGI_ENODEVICE;
		goto err2;
	}	/* if */

	if (options[OPT_INPUT].result[0]) {
		priv->inputbuffer = priv->memptr;
		priv->memptr=(char *)priv->memptr + INPBUFSIZE;
		DPRINT("moved mem to %p for input-buffer.\n",
			priv->memptr);
	}	/* if */

	vis->opdisplay->flush     = GGI_ipc_flush;
	vis->opdisplay->getmode   = GGI_ipc_getmode;
	vis->opdisplay->setmode   = GGI_ipc_setmode;
	vis->opdisplay->getapi    = GGI_ipc_getapi;
	vis->opdisplay->checkmode = GGI_ipc_checkmode;
	vis->opdisplay->setflags  = GGI_ipc_setflags;

	gii = ggGetAPIByName("gii");
	if (gii != NULL && STEM_HAS_API(vis->module.stem, gii)) {
		char inputstr[512];

		snprintf(inputstr, sizeof(inputstr),
			 "-size=%i:-pointer", INPBUFSIZE);
		DPRINT("\"input-memory\" inputstr \"%s\" at %p\n",
		       inputstr, priv->inputbuffer->buffer);
		priv->inp = ggOpenModule(gii, vis->module.stem,
					 "input-memory", inputstr,
					 priv->inputbuffer->buffer);
		DPRINT("ggOpenModule returned with %p\n",
		       priv->inp);

		if (priv->inp == NULL) {
			fprintf(stderr,
				"display-ipc: unable to open input-memory\n");
			err = GGI_ENODEVICE;
			goto err2;
		}
	}

	*dlret = GGI_DL_OPDISPLAY;
	return 0;

err2:
	close(priv->sockfd);
err1:
	free(priv);
err0:
	free(LIBGGI_GC(vis));

	*dlret = GGI_DL_OPDISPLAY;
	return err;

}	/* GGIopen */


static int GGIclose(struct ggi_visual *vis, struct ggi_dlhandle *dlh)
{
	_GGI_ipc_resetmode(vis);
	shmdt(IPC_PRIV(vis)->memptr); 
	free(IPC_PRIV(vis));
	free(LIBGGI_GC(vis));

	return 0;
}	/* GGIclose */


EXPORTFUNC
int GGIdl_ipc(int func, void **funcptr);

int GGIdl_ipc(int func, void **funcptr)
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
	}	/* switch */
  
	return GGI_ENOTFOUND;
}	/* GGIdl_ipc */
