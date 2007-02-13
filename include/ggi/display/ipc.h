/* $Id: ipc.h,v 1.10 2007/02/13 06:00:15 cegger Exp $
******************************************************************************

   Display-memory: headers

   Copyright (C) 1997 Andreas Beck	[becka@ggi-project.org]
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

#ifndef _GGI_DISPLAY_IPC_H
#define _GGI_DISPLAY_IPC_H

#include <ggi/internal/ggi-dl.h>


#define INPBUFSIZE	8192

#if defined(__WIN32__) && !defined(__CYGWIN__)
# ifdef HAVE_WINSOCK2_H
#  include <winsock2.h>
# endif
# ifdef HAVE_WINSOCK_H
#  include <winsock.h>
# endif
#endif

#ifdef HAVE_SYS_TYPES_H
# include <sys/types.h>
#endif

#ifdef HAVE_SYS_SOCKET_H
# include <sys/socket.h>
#endif

#ifdef HAVE_SYS_UN_H
# include <sys/un.h>
#endif

#ifdef HAVE_SYS_SHM_H
# include <sys/shm.h>
#endif

ggifunc_flush            GGI_ipc_flush;
ggifunc_getmode		 GGI_ipc_getmode;
ggifunc_setmode		 GGI_ipc_setmode;
ggifunc_getapi		 GGI_ipc_getapi;
ggifunc_checkmode	 GGI_ipc_checkmode;
ggifunc_setflags	 GGI_ipc_setflags;
ggifunc_setPalette       GGI_ipc_setPalette;

struct inpbuffer {
#if 0
	int writeoffset;	/* We should lock access to that one ... */
	int visx,visy,virtx,virty,frames,visframe,type;
#endif
	uint8_t buffer[1];	/* This index will be used "overflowing" */
};

typedef struct
{
	PHYSZ_DATA

	struct gg_module *inp;

	void *memptr;
	struct inpbuffer *inputbuffer;
	int sockfd;
	int semid;
	int shmid;
} ipc_priv;

#define IPC_PRIV(vis) ((ipc_priv *)LIBGGI_PRIVATE(vis))

int _GGI_ipc_resetmode(struct ggi_visual *vis);

#endif /* _GGI_DISPLAY_IPC_H */
