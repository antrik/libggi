/* $Id: win32thread.c,v 1.1 2004/02/12 09:09:53 pekberg Exp $
******************************************************************************

   MANSYNC_WIN32THREAD implementation.

   Copyright (C) 1998  Steve Cheng   [steve@ggi-project.org]
   Copyright (C) 2004  Peter Ekberg  [peda@lysator.liu.se]

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

   Helper library for the implementation of SYNC mode on targets which are
   inherently ASYNC (e.g. X) and require manual flushes of the framebuffer.

******************************************************************************
*/

#include <ggi/display/mansync.h>

#include <stdlib.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_WINDOWS_H
#include <windows.h>
#endif

#ifndef USE_THREADS
#warning You might want to compile LibGGI with multithreading support.
#endif


/* Hook structure to helperpriv of mansync visual. */

struct mansync_hook {
	int isasync;
	HANDLE thread;
};


#undef MANSYNC_ISASYNC
#define MANSYNC_PRIV(vis)	((struct mansync_hook *)vis->helperpriv)
#define MANSYNC_ISASYNC(vis)	(MANSYNC_PRIV(vis)->isasync)

static DWORD WINAPI _GGI_mansync_thread(LPVOID arg);
static DWORD WINAPI _GGI_mansync_thread(LPVOID arg)
{
	ggi_visual *vis = arg;
	int fpsrate = MANSYNC_FPS;
	char *str = getenv("GGI_MANSYNC_FPS");

	if (str) {
		fpsrate = atoi(str);
		if (fpsrate <= 0) fpsrate = MANSYNC_FPS;
	}

	while (!MANSYNC_ISASYNC(vis)) {
		GGIDPRINT("Doing mansync-flush.\n");
		ggiFlush(vis);

		ggUSleep(1000000/fpsrate);
	}

	return 0;
}


int _GGI_mansync_init(ggi_visual *vis)
{
	vis->helperpriv = _ggi_malloc(sizeof(struct mansync_hook));
	MANSYNC_ISASYNC(vis) = 1;	/* Yes, this SHOULD be initialized to 1. */
	return 0;
}


int _GGI_mansync_deinit(ggi_visual *vis)
{
	_GGI_mansync_stop(vis);

	free(vis->helperpriv);
	vis->helperpriv = NULL;

	return 0;
}


int _GGI_mansync_start(ggi_visual *vis)
{
	DWORD threadid;
	
	GGIDPRINT("_GGI_mansync_start() (MANSYNC_WIN32THREAD) called.\n");

	if(!MANSYNC_ISASYNC(vis))
		return -1;

	MANSYNC_ISASYNC(vis) = 0;

	MANSYNC_PRIV(vis)->thread = CreateThread(NULL, 0, _GGI_mansync_thread,
						 vis, 0, &threadid);
	if(MANSYNC_PRIV(vis)->thread == NULL)
		return -1;

	return 0;
}


int _GGI_mansync_stop(ggi_visual *vis)
{
	GGIDPRINT("_GGI_mansync_stop() (MANSYNC_WIN32THREAD) called.\n");

	if(MANSYNC_PRIV(vis)->thread == NULL)
		return -1;

	if(MANSYNC_ISASYNC(vis))
		return -1;

	/* Thread should die automatically */
	MANSYNC_ISASYNC(vis) = 1;

	WaitForSingleObject(MANSYNC_PRIV(vis)->thread, INFINITE);
	MANSYNC_PRIV(vis)->thread = NULL;
	return 0;
}


/* Threads can't be arbitrarily suspended so these functions
   terminate/restart the threads instead. */

int _GGI_mansync_ignore(ggi_visual *vis)
{
	return _GGI_mansync_stop(vis);
}


int _GGI_mansync_cont(ggi_visual *vis)
{
	if (LIBGGI_FLAGS(vis) & GGIFLAG_ASYNC)
		return -1;
		
	return _GGI_mansync_start(vis);
}
