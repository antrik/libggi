/* $Id: visual.c,v 1.38 2007/03/08 20:54:07 soyt Exp $
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

#include "config.h"
#include <ggi/gii-events.h>
#include <ggi/display/memory.h>
#include <ggi/internal/gg_replace.h>
#include <ggi/internal/ggi_debug.h>

static const gg_option optlist[] =
{
	{ "input", "" },
	{ "physz", "0,0" },
	{ "pixfmt", "" },
	{ "layout", "no"},
	{ "noblank", "no"}
};

#define OPT_INPUT	0
#define OPT_PHYSZ	1
#define OPT_PIXFMT	2
#define OPT_LAYOUT	3
#define OPT_NOBLANK	4

#define NUM_OPTS	(sizeof(optlist)/sizeof(gg_option))

static int GGI_memory_flush(struct ggi_visual *vis, 
			    int x, int y, int w, int h, int tryflag)
{
	/* Dummy function to avoid leaving _default_error on hook */
	return 0;
}

#if defined(HAVE_SHM) && !defined(HAVE_SYS_SHM_H) && defined(HAVE_WINDOWS_H)

static const char *ftok(const char *pathname, int id)
{
	static char object[MAX_PATH];
	char *ptr;

	snprintf(object, sizeof(object),
		"ggi-display-memory-shm:%s:%d", pathname, id);

	ptr = object;
	while(ptr = strchr(ptr, '\\'))
		*ptr++ = '/';
	return object;
}

#endif /* HAVE_SHM && !HAVE_SYS_SHM_H && HAVE_WINDOWS_H */

static int GGIopen(struct ggi_visual *vis, struct ggi_dlhandle *dlh,
			const char *args, void *argptr, uint32_t *dlret)
{
	ggi_memory_priv *priv;
	gg_option options[NUM_OPTS];
	char inputstr[1024];

	DPRINT_MISC("GGIopen: coming up.\n");

	memcpy(options, optlist, sizeof(options));
	memset(inputstr, 0, sizeof(inputstr));

	LIBGGI_GC(vis) = malloc(sizeof(ggi_gc));
	if (!LIBGGI_GC(vis)) return GGI_ENOMEM;

	/* Allocate descriptor for screen memory */
	priv = calloc(1, sizeof(ggi_memory_priv));
	if (!priv) {
		free(LIBGGI_GC(vis));
		return GGI_ENOMEM;
	}
	LIBGGI_PRIVATE(vis) = priv;

	priv->inp = NULL;
	priv->memtype = MT_MALLOC;	/* Default to mallocing. */
	priv->inputbuffer = NULL;	/* Default to no input */
	priv->inputoffset = 0;		/* Setup offset. */

	if (args) {
		args = ggParseOptions(args, options, NUM_OPTS);
		if (args == NULL) {
			fprintf(stderr, "display-memory: error in "
				"arguments.\n");
		}
	}

	if (_ggi_physz_parse_option(options[OPT_PHYSZ].result, 
			     &(priv->physzflags), &(priv->physz)))
	{ 
		free(priv);
		free(LIBGGI_GC(vis));
		return GGI_EARGINVAL;
	}

	if (args && *args) {	/* We have parameters. Analyze them. */
		DPRINT("has args: \"%s\"\n", args);
#ifdef HAVE_SHM
		if (strncmp(args, "shmid:", 6) == 0) {
			sscanf(args + 6, "%i", &(priv->shmid));
			DPRINT("has shmid-arg: %d.\n", priv->shmid);
			priv->memptr = shmat(priv->shmid, NULL, 0);
			DPRINT("shmat at %p.\n", priv->memptr);
			if (priv->memptr != (void *)-1) {
				priv->memtype = MT_SHMID;
				if (options[OPT_INPUT].result[0]) {
					priv->inputbuffer = priv->memptr;
					priv->memptr = (char *)priv->memptr
							+ INPBUFSIZE;
					DPRINT("moved mem to %p for input-buffer.\n",
						priv->memptr);
				} 
			}
		} else if (strncmp(args, "keyfile:", 8) == 0) {
			unsigned int size;
			char id;
			char filename[1024];

			sscanf(args + 8, "%u:%c:%s", &size, &id, filename);
			DPRINT("has keyfile-arg:%d:%c:%s.\n",
				size, id, filename);

			priv->shmid = shmget(ftok(filename,id), size,
						IPC_CREAT|0666);
			DPRINT("has shmid:%d.\n", priv->shmid);

			priv->memptr = shmat(priv->shmid,NULL,0);
			DPRINT("shmat at %p.\n", priv->memptr);
			if (priv->memptr != (void *)-1) {
				priv->memtype = MT_SHMID;
				if (options[OPT_INPUT].result[0]) {
					priv->inputbuffer = priv->memptr;
					priv->memptr = (char *)priv->memptr
							+ INPBUFSIZE;
					DPRINT("moved mem to %p for input-buffer.\n",
						priv->memptr);
				}
			}
		} else 
#endif
		if (strncmp(args, "pointer", 7) == 0) {
			priv->memptr = argptr;
			if (priv->memptr) {
				priv->memtype = MT_EXTERN;
			}
		}
	}

	/* Explicit pixelformat. */
	if (options[OPT_PIXFMT].result[0]) {
		_ggi_parse_pixfmtstr(options[OPT_PIXFMT].result, '\0', NULL,
				strlen(options[OPT_PIXFMT].result)+1,
				&priv->r_mask, &priv->g_mask, &priv->b_mask,
				&priv->a_mask);
	}

	/* Explicit layout for preallocated buffers with nontrivial layouts. */
	if (options[OPT_LAYOUT].result[0] != 'n') {
		char *idx;
		priv->fstride = strtoul(options[OPT_LAYOUT].result, &idx, 10);
		if (strncmp(idx, "plb", 3) == 0) {
			priv->layout = blPixelLinearBuffer;
			idx += 3;
			priv->buffer.plb.stride = strtoul(idx, NULL, 10);
		} else if (strncmp(idx, "plan", 4) == 0) {
			priv->layout = blPixelPlanarBuffer;
			idx += 4;
			priv->buffer.plan.next_plane = strtoul(idx, &idx, 10);
			if (*idx != ',') {
				priv->buffer.plan.next_line = 0;
			} else {
				idx++;
				priv->buffer.plan.next_line = 
				  strtoul(idx, &idx, 10);
			}
		} else { 
			if (*idx != '\0') 
				fprintf(stderr, "bad layout params\n");
			priv->layout = blPixelLinearBuffer;
			priv->buffer.plb.stride = 0;
		}
	}

	/* Do not blank the framebuffer on SetMode.
	 * (Preserves data in prealloced memory area.) 
	 */
	priv->noblank = (options[OPT_NOBLANK].result[0] != 'n');

	vis->opdisplay->flush = GGI_memory_flush; 
	vis->opdisplay->getmode = GGI_memory_getmode;
	vis->opdisplay->setmode = GGI_memory_setmode;
	vis->opdisplay->getapi = GGI_memory_getapi;
	vis->opdisplay->checkmode = GGI_memory_checkmode;
	vis->opdisplay->setflags = GGI_memory_setflags;

	if (priv->inputbuffer) {
		struct gg_api *gii;

#if 0
		priv->inputbuffer->visx =
		priv->inputbuffer->visy =
		priv->inputbuffer->virtx =
		priv->inputbuffer->virty =
		priv->inputbuffer->frames =
		priv->inputbuffer->visframe = 0;
#endif

		DPRINT_MISC("Adding gii to shmem-memtarget\n");
		gii = ggGetAPIByName("gii");
		if (gii != NULL && STEM_HAS_API(vis->module.stem, gii)) {
			snprintf(inputstr, sizeof(inputstr),
				"-size=%i:-pointer", INPBUFSIZE);
			DPRINT("\"input-memory\" inputstr \"%s\" at %p\n",
				inputstr, priv->inputbuffer->buffer);
			priv->inp = ggOpenModule(gii, vis->module.stem,
						"input-memory", inputstr,
						priv->inputbuffer->buffer);
			DPRINT("ggOpenModule for input-memory returned %p\n",
				priv->inp);
		}
	}
	
	*dlret = GGI_DL_OPDISPLAY;
	return 0;
}


static int GGIclose(struct ggi_visual *vis, struct ggi_dlhandle *dlh)
{
	ggi_memory_priv *priv = MEMORY_PRIV(vis);

	_GGI_memory_resetmode(vis);

	if (priv->inp) {
		ggCloseModule(priv->inp);
		priv->inp = NULL;
	}

	switch(priv->memtype) {
	case MT_MALLOC:
	case MT_EXTERN:	/* Nothing to be done. */
		break;
#ifdef HAVE_SHM
	case MT_SHMKEYFILE:
		if (priv->inputbuffer)
			shmdt((void *)priv->inputbuffer);
		else
			shmdt(priv->memptr);
#if !defined(HAVE_SYS_SHM_H)
		/* FIXME ? Should we RMID the area for unix as well? */
		shmctl(priv->shmid, IPC_RMID, NULL);
#endif
		break;
	case MT_SHMID:
		if (priv->inputbuffer)
			shmdt((void *)priv->inputbuffer);
		else
			shmdt(priv->memptr);
	  	break;
#endif /* HAVE_SHM */
	default:
		break;
	}

	free(priv);
	free(LIBGGI_GC(vis));
	LIBGGI_PRIVATE(vis) = NULL;
	LIBGGI_GC(vis) = NULL;

	return 0;
}


EXPORTFUNC
int GGIdl_memory(int func, void **funcptr);

int GGIdl_memory(int func, void **funcptr)
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
	}

	return GGI_ENOTFOUND;
}
