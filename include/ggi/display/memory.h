/* $Id: memory.h,v 1.3 2002/04/28 17:57:01 skids Exp $
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
#define MEMINPMAGIC	'M'

#ifdef HAVE_SYS_SHM_H
#define HAVE_SHM
#include <sys/shm.h>
#endif

ggifunc_getmode		GGI_memory_getmode;
ggifunc_setmode		GGI_memory_setmode;
ggifunc_getapi		GGI_memory_getapi;
ggifunc_checkmode	GGI_memory_checkmode;
ggifunc_setflags	GGI_memory_setflags;
ggifunc_setpalvec	GGI_memory_setpalvec;

enum memtype { 
	MT_MALLOC, 	/* No parameters  : memory is malloced */
	MT_EXTERN, 	/* only a pointer : draw there. Take care it
			   is big enough. */
	MT_SHMID, 	/* "shmid:%i"     : map shared memory at shmid */
	MT_SHMKEYFILE 	/* "keyfile:%c:%s": create and map shared mem
			   corresponding to keyfile see ftok */
};

typedef struct {
	int writeoffset;	/* We should lock access to that one ... */
	int visx,visy,virtx,virty,frames,visframe,type;
	char buffer[1];		/* This index will be used "overflowing" */
} inpbuffer;

typedef struct {
	enum memtype memtype;
	void *memptr;
	inpbuffer *inputbuffer;
	int  inputoffset;
	int         physzflags;
	ggi_coord   physz;
	ggi_pixel r_mask, g_mask, b_mask;
#ifdef HAVE_SHM
	int	shmid;
#endif
} ggi_memory_priv;

#define MEMORY_PRIV(vis) ((ggi_memory_priv *)LIBGGI_PRIVATE(vis))

int _GGI_memory_resetmode(ggi_visual *vis);

#endif /* _GGI_DISPLAY_MEMORY_H */
