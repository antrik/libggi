/* $Id: suidhook.h,v 1.1 2001/05/12 23:02:22 cegger Exp $
******************************************************************************

   Display-SUID: definitions

   Copyright (C) 1995,1998 Andreas Beck   [becka@ggi-project.org]
   Copyright (C) 1997 Jason McMullan      [jmcc@ggi-project.org]
   Copyright (C) 1998 Andrew Apted        [andrew@ggi-project.org]

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

#ifndef _DISPLAY_SUIDKGI_H
#define _DISPLAY_SUIDKGI_H

#include <ggi/internal/ggi-dl.h>
#include <kgi/kgi_commands.h>

#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>


typedef struct suid_hook
{
	int is_up;
	int dev_mem;

	int vt_fd;
	void *vt_priv;
	
	size_t	mmap_length;

	/* ... */

} suid_hook;

extern int _suidtarget_dev_mem;	/* Required as global for the KGI module */

#define SUIDHOOK(vis)  ((suid_hook *) LIBGGI_PRIVATE(vis))

#define VTSWITCH_VT_FD(vis)  (SUIDHOOK(vis)->vt_fd)
#define VTSWITCH_PRIV(vis)   (SUIDHOOK(vis)->vt_priv)


/* Linux_common routines */

int  vtswitch_open(ggi_visual *vis);
void vtswitch_close(ggi_visual *vis);


/* Internal stuff */

int GGIdlcleanup(ggi_visual *vis);
int graph_ioctl(unsigned int cmd, ...);


/* LibGGI API implementation */

ggifunc_getmode		GGI_suidkgi_getmode;
ggifunc_setmode		GGI_suidkgi_setmode;
ggifunc_checkmode	GGI_suidkgi_checkmode;
ggifunc_getapi		GGI_suidkgi_getapi;
ggifunc_setflags	GGI_suidkgi_setflags;
ggifunc_kgicommand	GGI_suidkgi_kgicommand;
		
ggifunc_setpalvec	GGI_suidkgi_setpalvec;
ggifunc_setorigin	GGI_suidkgi_setorigin;
ggifunc_setdisplayframe	GGI_suidkgi_setdisplayframe;


#endif /* _DISPLAY_SUIDKGI_H */
