/* $Id: visual.c,v 1.1 2001/05/12 23:02:08 cegger Exp $
******************************************************************************

   Display-kgi: initialization

   Copyright (C) 1995 Andreas Beck      [andreas@ggi-project.org]
   Copyright (C) 1997 Jason McMullan    [jmcc@ggi-project.org]
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

#include <unistd.h>
#include <fcntl.h>
#include <signal.h>

#include <kgi/kgi_commands.h>
#include <ggi/internal/ggi-dl.h>

void _ignore_SIGBUS(int unused)
{ 
	signal(SIGBUS,_ignore_SIGBUS);
	sleep(1);	/* Ignore the SIGBUSes */
}

static int GGIopen(ggi_visual *vis, struct ggi_dlhandle *dlh,
			const char *args, void *argptr, uint32 *dlret)
{
	LIBGGI_FD(vis) = open(args,O_RDWR);
	if (LIBGGI_FD(vis) < 0) {
		return GGI_ENODEVICE;
	}

	/* Has mode management */
	vis->opdisplay->getmode=GGIgetmode;
	vis->opdisplay->setmode=GGIsetmode;
	vis->opdisplay->checkmode=GGIcheckmode;
	vis->opdisplay->kgicommand=GGIkgicommand;
	vis->opdisplay->setflags=GGIsetflags;

	/* Has Event management */
	vis->opdisplay->eventpoll=GGIeventpoll;
	vis->opdisplay->eventread=GGIeventread;
	vis->opdisplay->seteventmask=GGIseteventmask;

	vis->opdraw->setorigin=GGIsetorigin;

	/* temporary hack to do away with the SIGBUS ... */
	signal(SIGBUS,_ignore_SIGBUS);

	*dlret = GGI_DL_OPDISPLAY | GGI_DL_OPDRAW;
	return 0;
}

static int GGIclose(ggi_visual *vis, struct ggi_dlhandle *dlh)
{
	if (LIBGGI_FD(vis) > -1)
		close(LIBGGI_FD(vis));

	return 0;
}
		

int GGIdl_kgi(int func, void **funcptr)
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
