/* $Id: visual.c,v 1.19 2005/07/30 10:58:25 cegger Exp $
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


static ggi_event_mask GII_ipc_poll(gii_input_t inp, void *arg)
{
	ggi_ipc_priv *priv=inp->priv;
	ggi_event ev;
	int rc=0;
  
	while(priv->inputoffset!=priv->inputbuffer->writeoffset) {
		if (priv->inputbuffer->buffer[priv->inputoffset++]
		   != MEMINPMAGIC)
		{
			DPRINT_MISC("OUT OF SYNC in shm input !\n");
			priv->inputoffset=0;	/* Try to resync */
			return 0;
		}	/* if */

		memcpy(&ev, &(priv->inputbuffer->buffer[priv->inputoffset]),
			(size_t)(priv->inputbuffer->buffer[priv->inputoffset]));

		_giiEvQueueAdd(inp, &ev);
		priv->inputoffset += ev.any.size;
		rc |= 1 << ev.any.type;
		if (priv->inputoffset >= (signed)(INPBUFSIZE - sizeof(ggi_event)
		    - sizeof(priv->inputbuffer->writeoffset) - 10))
		{
			priv->inputoffset=0;
		}	/* if */
	}	/* while */
	return rc;
}	/* GII_ipc_poll */


static int GII_ipc_send(gii_input_t inp, ggi_event *event)
{
	ggi_ipc_priv *priv=inp->priv;
	size_t size = event->any.size;
  
	priv->inputbuffer->buffer[priv->inputbuffer->writeoffset++] = MEMINPMAGIC;
	memcpy(&(priv->inputbuffer->buffer[priv->inputbuffer->writeoffset]),
		event, size);

	priv->inputbuffer->writeoffset += size;
	if (priv->inputbuffer->writeoffset
	  >= (signed)(INPBUFSIZE - sizeof(ggi_event)
		- sizeof(priv->inputbuffer->writeoffset) - 10))
	{
		priv->inputbuffer->writeoffset=0;
	}	/* if */
	priv->inputbuffer->buffer[priv->inputbuffer->writeoffset]=MEMINPMAGIC-1;	/* "break"-symbol */
  
	return 0;
}	/* GII_ipc_send */


int GGI_ipc_flush(struct ggi_visual *vis, int x, int y, int w, int h,
		  int tryflag)
{
	char buffer[32];
	ggi_ipc_priv *priv = IPC_PRIV(vis);

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



static int GGIopen(ggi_visual *vis, struct ggi_dlhandle *dlh,
		   const char *args, void *argptr, uint32_t *dlret)
{
	ggi_ipc_priv *priv;
	gg_option options[NUM_OPTS];
	struct sockaddr_un address;

	DPRINT_MISC("display-ipc coming up.\n");
	memcpy(options, optlist, sizeof(options));

	LIBGGI_GC(vis) = malloc(sizeof(ggi_gc));
	if (!LIBGGI_GC(vis)) return GGI_ENOMEM;

	/* Allocate descriptor for screen memory */
	priv = malloc(sizeof(ggi_ipc_priv));

	if (!priv) {
		free(LIBGGI_GC(vis));
		return GGI_ENOMEM;
	}	/* if */

	LIBGGI_PRIVATE(vis) = priv;
	priv->inputbuffer = NULL;	/* Default to no input */
	priv->inputoffset = 0;		/* Setup offset. */

	if (!args) {
		DPRINT("display-ipc: required arguments missing\n");
		return GGI_EARGREQ;
	}	/* if */

	args = ggParseOptions(args, options, NUM_OPTS);
	if (args == NULL) {
		DPRINT("display-ipc: error in arguments.\n");
		return GGI_EARGREQ;
	}	/* if */

	if (_ggi_physz_parse_option(options[OPT_PHYSZ].result,
			     &(priv->physzflags), &(priv->physz)))
	{
		free(priv);
		free(LIBGGI_GC(vis));
		return GGI_EARGINVAL;   
        }	/* if */

	if (!options[OPT_SOCKET].result[0]
	   && !options[OPT_SEMID].result[0]
	   && !options[OPT_SHMID].result[0])
	{
		DPRINT("display-ipc: required arguments missing\n");
		return GGI_EARGREQ;
	}	/* if */

	if (!(sscanf(options[OPT_SOCKET].result,"%s", address.sun_path)
	   && sscanf(options[OPT_SEMID].result,"%d", &(priv->semid))
	   && sscanf(options[OPT_SHMID].result,"%d", &(priv->shmid))))
	{
		DPRINT("display-ipc: argument format error\n");
		return GGI_EARGREQ;
	}	/* if */

	DPRINT("display-ipc parsed args: socket: %s semid: %d shmid: %d\n",
		   address.sun_path, priv->semid, priv->shmid);
	address.sun_family = AF_UNIX;
	if ((priv->sockfd = socket(PF_UNIX, SOCK_STREAM, 0)) == -1
	   || connect(priv->sockfd, (const struct sockaddr *)(&address),
		sizeof(struct sockaddr_un)) == -1
	   || (priv->memptr = (char *)shmat(priv->shmid, 0, 0)) == (char *)-1)
	{
		DPRINT("display-ipc initialization failed : %s\n", strerror(errno));
		return GGI_ENODEVICE;
	}	/* if */

	if (options[OPT_INPUT].result[0]) {
		priv->inputbuffer=priv->memptr;
		priv->memptr=(char *)priv->memptr+INPBUFSIZE;
		DPRINT("display-ipc: moved mem to %p for input-buffer.\n",
			priv->memptr);
	}	/* if */

	vis->opdisplay->flush     = GGI_ipc_flush;
	vis->opdisplay->getmode   = GGI_ipc_getmode;
	vis->opdisplay->setmode   = GGI_ipc_setmode;
	vis->opdisplay->getapi    = GGI_ipc_getapi;
	vis->opdisplay->checkmode = GGI_ipc_checkmode;
	vis->opdisplay->setflags  = GGI_ipc_setflags;
  
	if (priv->inputbuffer) {
		gii_input *inp;
      
		priv->inputbuffer->visx		=
		priv->inputbuffer->visy		=
		priv->inputbuffer->virtx	=
		priv->inputbuffer->virty	=
		priv->inputbuffer->frames	=
		priv->inputbuffer->visframe	= 0;

		DPRINT_MISC("Adding gii to shmem-memtarget\n");

		/* First allocate a new gii_input descriptor. */

		if (NULL==(inp=_giiInputAlloc())) {
			DPRINT_MISC("giiInputAlloc failure.\n");
			goto out;
		}	/* if */
		DPRINT_MISC("gii inp=%p\n",inp);

		/* Now fill in the blanks. */
      
		inp->priv = priv;	/* We need that in poll() */
		priv->inputbuffer->writeoffset = 0;	/* Not too good, but ... */

		inp->targetcan= emAll;
		inp->GIIseteventmask(inp,inp->targetcan);
		inp->maxfd = 0;		/* This is polled. */
		inp->flags |= GII_FLAGS_HASPOLLED;
      
		inp->GIIeventpoll = GII_ipc_poll;
		inp->GIIsendevent = GII_ipc_send;
      
		/* Now join the new event source in. */
		vis->input=giiJoinInputs(vis->input,inp);
  out:
		while(0){};
	}	/* if */
  
	*dlret = GGI_DL_OPDISPLAY;
	return 0;

}	/* GGIopen */


static int GGIclose(ggi_visual *vis, struct ggi_dlhandle *dlh)
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
	}	/* switch */
  
	return GGI_ENOTFOUND;
}	/* GGIdl_ipc */


#include <ggi/internal/ggidlinit.h>
