/* $Id: visual.c,v 1.2 2001/05/31 21:55:21 skids Exp $
******************************************************************************

   Display-memory: mode management

   Copyright (C) 1995 Andreas Beck	[becka@ggi-project.org]
   Copyright (C) 1997 Jason McMullan	[jmcc@ggi-project.org]
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

#include <ggi/display/memory.h>

static const gg_option optlist[] =
{
	{ "input", "" },
	{ "physz", "0,0" }
};

#define OPT_INPUT	0
#define OPT_PHYSZ	1

#define NUM_OPTS	(sizeof(optlist)/sizeof(gg_option))

ggi_event_mask GII_memory_poll(gii_input_t inp, void *arg)
{
	ggi_memory_priv *priv=inp->priv;
	ggi_event ev;
	int rc=0;
	
	while(priv->inputoffset!=priv->inputbuffer->writeoffset)
	{
		if (priv->inputbuffer->buffer[priv->inputoffset++]!=MEMINPMAGIC)
		{
			GGIDPRINT_MISC("OUT OF SYNC in meminput !\n");
			priv->inputoffset=0;	/* Try to resync */
			return 0;
		}
		memcpy(&ev, &(priv->inputbuffer->buffer[priv->inputoffset]),
		       priv->inputbuffer->buffer[priv->inputoffset]);
		_giiEvQueueAdd(inp, &ev);
		priv->inputoffset += ev.any.size;
		rc |= 1<<ev.any.type;
		if (priv->inputoffset >= INPBUFSIZE-sizeof(ggi_event)-sizeof(priv->inputbuffer->writeoffset)-10) {
			priv->inputoffset=0;
		}
	}
	return rc;
}

int GII_memory_send(gii_input_t inp, ggi_event *event)
{
	ggi_memory_priv *priv=inp->priv;
	int size;
	
	priv->inputbuffer->buffer[priv->inputbuffer->writeoffset++]=MEMINPMAGIC;
	memcpy(&(priv->inputbuffer->buffer[priv->inputbuffer->writeoffset]),
		event,size=event->any.size);
	priv->inputbuffer->writeoffset+=size;
	if (priv->inputbuffer->writeoffset>=INPBUFSIZE-sizeof(ggi_event)-sizeof(priv->inputbuffer->writeoffset)-10)
		priv->inputbuffer->writeoffset=0;
	priv->inputbuffer->buffer[priv->inputbuffer->writeoffset]=MEMINPMAGIC-1;	/* "break"-symbol */

	return 0;
}


static int GGIopen(ggi_visual *vis, struct ggi_dlhandle *dlh,
			const char *args, void *argptr, uint32 *dlret)
{
	ggi_memory_priv *priv;
	gg_option options[NUM_OPTS];

	GGIDPRINT_MISC("display-memory coming up.\n");

	memcpy(options, optlist, sizeof(options));

	LIBGGI_GC(vis) = malloc(sizeof(ggi_gc));
	if (!LIBGGI_GC(vis)) return GGI_ENOMEM;

	/* Allocate descriptor for screen memory */
	priv = LIBGGI_PRIVATE(vis) = malloc(sizeof(ggi_memory_priv));
	if (!priv) {
		free(LIBGGI_GC(vis));
		return GGI_ENOMEM;
	}

	priv->memtype = MT_MALLOC;	/* Default to mallocing. */
	priv->inputbuffer = NULL;	/* Default to no input */
	priv->inputoffset = 0;		/* Setup offset. */

	if (args) {
		args = ggParseOptions((char *) args, options, NUM_OPTS);
		if (args == NULL) {
			fprintf(stderr, "display-memory: error in "
				"arguments.\n");
		}
	}

	if (args && *args)	/* We have parameters. Analyze them. */
	{
		GGIDPRINT("display-memory has args.\n");
#ifdef HAVE_SHM
		if (strncmp(args,"shmid:",6)==0)
		{
			sscanf(args+6,"%i",&(priv->shmid));
			GGIDPRINT("display-memory has shmid-arg:%d.\n",
				priv->shmid);
			priv->memptr=shmat(priv->shmid,NULL,0);
			GGIDPRINT("display-memory: shmat at %p.\n",
				priv->memptr);
			if (priv->memptr!=(void *)-1) 
			{
				priv->memtype=MT_SHMID;
				if (options[OPT_INPUT].result[0])
				{
					priv->inputbuffer=priv->memptr;
					priv->memptr=(char *)priv->memptr+INPBUFSIZE;
					GGIDPRINT("display-memory: moved mem to %p for input-buffer.\n",
						priv->memptr);
				}
			}
		}
		else if (strncmp(args,"keyfile:",8)==0)
		{
			int size;
			char id;
			char filename[1024];

			sscanf(args+8,"%d:%c:%s",&size,&id,filename);
			GGIDPRINT("display-memory has keyfile-arg:%d:%c:%s.\n",
				size,id,filename);

			priv->shmid=shmget(ftok(filename,id), size, IPC_CREAT|0666);
			GGIDPRINT("display-memory has shmid:%d.\n",
				priv->shmid);

			priv->memptr=shmat(priv->shmid,NULL,0);
			GGIDPRINT("display-memory: shmat at %p.\n",
				priv->memptr);
			if (priv->memptr!=(void *)-1) 
			{
				priv->memtype=MT_SHMID;
				if (options[OPT_INPUT].result[0])
				{
					priv->inputbuffer=priv->memptr;
					priv->memptr=(char *)priv->memptr+INPBUFSIZE;
					GGIDPRINT("display-memory: moved mem to %p for input-buffer.\n",
						priv->memptr);
				}
			}
		} else 
#endif
		if (strncmp(args,"pointer",7)==0)
		{
			priv->memptr = argptr;
			if (priv->memptr)
				priv->memtype=MT_EXTERN;
		}
	}

	vis->opdisplay->getmode=GGI_memory_getmode;
	vis->opdisplay->setmode=GGI_memory_setmode;
	vis->opdisplay->getapi=GGI_memory_getapi;
	vis->opdisplay->checkmode=GGI_memory_checkmode;
	vis->opdisplay->setflags=GGI_memory_setflags;

	if (priv->inputbuffer)
	{
		gii_input *inp;

		priv->inputbuffer->visx=
		priv->inputbuffer->visy=
		priv->inputbuffer->virtx=
		priv->inputbuffer->virty=
		priv->inputbuffer->frames=
		priv->inputbuffer->visframe=0;

		GGIDPRINT_MISC("Adding gii to shmem-memtarget\n");

		/* First allocate a new gii_input descriptor. */

		if (NULL==(inp=_giiInputAlloc()))
		{
			GGIDPRINT_MISC("giiInputAlloc failure.\n");
			goto out;
		}
		GGIDPRINT_MISC("gii inp=%p\n",inp);

		/* Now fill in the blanks. */

		inp->priv=priv;	/* We need that in poll() */
		priv->inputbuffer->writeoffset=0;	/* Not too good, but ... */
		inp->targetcan= emAll;
		inp->GIIseteventmask(inp,inp->targetcan);
		inp->maxfd=0;	/* This is polled. */
		inp->flags|=GII_FLAGS_HASPOLLED;

		inp->GIIeventpoll=GII_memory_poll;
		inp->GIIsendevent=GII_memory_send;

		/* Now join the new event source in. */
		vis->input=giiJoinInputs(vis->input,inp);
		out:
		while(0){};
	}
	
	*dlret = GGI_DL_OPDISPLAY;
	return 0;
}


static int GGIclose(ggi_visual *vis, struct ggi_dlhandle *dlh)
{
	_GGI_memory_resetmode(vis);

	switch(MEMORY_PRIV(vis)->memtype) {
	case MT_MALLOC:
	case MT_EXTERN:	/* Nothing to be done. */
	  	break;
#ifdef HAVE_SHM		
	case MT_SHMKEYFILE: /* FIXME ? Should we RMID the area ? */
	case MT_SHMID: 
	  	shmdt(MEMORY_PRIV(vis)->memptr);
	  	break;
#endif	
	default:
		break;
	}

	free(LIBGGI_PRIVATE(vis));
	free(LIBGGI_GC(vis));

	return 0;
}


int GGIdl_memory(int func, void **funcptr)
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
