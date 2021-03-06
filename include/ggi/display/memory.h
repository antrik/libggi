/* $Id: memory.h,v 1.17 2008/12/12 03:53:10 pekberg Exp $
******************************************************************************

   Display-memory: headers

   Copyright (C) 1997 Andreas Beck		[becka@ggi-project.org]
   Copyright (C) 1998-2000 Marcus Sundberg	[marcus@ggi-project.org]

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

#ifndef _GGI_DISPLAY_MEMORY_H
#define _GGI_DISPLAY_MEMORY_H

#include <ggi/internal/ggi-dl.h>


#define INPBUFSIZE	8192

#ifdef HAVE_SYS_SHM_H
#include <sys/shm.h>
#endif /* HAVE_SYS_SHM_H */
#include <ggi/internal/gg_replace.h>

ggifunc_getmode			GGI_memory_getmode;
ggifunc_setmode			GGI_memory_setmode;
ggifunc_getapi			GGI_memory_getapi;
ggifunc_checkmode		GGI_memory_checkmode;
ggifunc_setflags		GGI_memory_setflags;
ggifunc_setPalette	GGI_memory_setPalette;

enum memtype {
	MT_MALLOC, 	/* No parameters  : memory is malloced */
	MT_EXTERN, 	/* only a pointer : draw there. Take care it
			   is big enough. */
	MT_SHMID, 	/* "shmid:%i"     : map shared memory at shmid */
	MT_SHMKEYFILE 	/* "keyfile:%c:%s": create and map shared mem
			   corresponding to keyfile see ftok */
};

typedef struct {
#if 0
	int writeoffset;	/* We should lock access to that one ... */
	int visx,visy,virtx,virty,frames,visframe,type;
#endif
	uint8_t buffer[1];	/* This index will be used "overflowing" */
} inpbuffer;

typedef struct {
	PHYSZ_DATA

	struct gg_instance *inp;
	enum memtype   	memtype;
	void	       *memptr;
	inpbuffer      *inputbuffer;
	ggi_pixel	r_mask, g_mask, b_mask, a_mask;
	uint32_t	pixfmt_flags;
	int		fstride;
	int		noblank;
	ggifunc_fillscreen     *oldfillscreen;
	ggi_bufferlayout	layout;
        union {
		ggi_pixellinearbuffer plb;
                ggi_pixelplanarbuffer plan;
        } buffer;

#ifdef _GG_HAVE_SHM
	int		shmid;
#endif
} ggi_memory_priv;

#define MEMORY_PRIV(vis) ((ggi_memory_priv *)LIBGGI_PRIVATE(vis))

int _GGI_memory_resetmode(struct ggi_visual *vis);

#endif /* _GGI_DISPLAY_MEMORY_H */
