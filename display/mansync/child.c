/* $Id: child.c,v 1.1 2001/05/12 23:02:11 cegger Exp $
******************************************************************************

   MANSYNC_CHILD implementation.

   Copyright (C) 1998 Steve Cheng      [steve@ggi-project.org]
   Copyright (C) 1998 Marcus Sundberg  [marcus@ggi-project.org]

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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#ifdef HAVE_SYS_WAIT_H
#include <sys/wait.h>
#endif

#define MANSYNC_SIGNAL SIGPROF


/* Because each process can only have one signal handler, all mansync visuals
   need to be flushed from one shared handler.  The following structure
   stores global information about the mansync visuals.  */

static struct {
	ggi_visual	**visuals;	/* Dynamically-allocated list of mansync visuals */
	int		nrvisuals;	/* Number of visuals above */
	int		nrsync;		/* SYNC mode usage counter: start or stop mansync when zero. */
	int		childpid;	/* Child process which periodically sends signals to flush */
	void	(*oldsynchandler)(int);	/* Previous signal handler */
	int		ignore_counter; /* Allows nested ignore/cont calls */
	int		usecs;		/* The number of microseconds between
					   each flush */
	int		skip;		/* Skip the next flush */
} _GGI_mansync_state = {NULL,0,0,0,NULL};


/* Hook structure to helperpriv of mansync visual. */

struct mansync_hook {
	int isasync;
};


#undef MANSYNC_ISASYNC
#define MANSYNC_PRIV(vis)	((struct mansync_hook *)vis->helperpriv)
#define MANSYNC_ISASYNC(vis)	(MANSYNC_PRIV(vis)->isasync)


static void _GGI_mansync_handler(int unused)
{
#ifndef HAVE_SIGPENDING
	struct timeval oldtv, newtv;
#endif
	int i;

	signal(MANSYNC_SIGNAL, SIG_IGN);

	if (_GGI_mansync_state.skip) {
		_GGI_mansync_state.skip = 0;
		signal(MANSYNC_SIGNAL, _GGI_mansync_handler);
		return;
	}

	_GGI_mansync_state.ignore_counter++;

#ifndef HAVE_SIGPENDING
	ggCurTime(&oldtv);
#endif
	if (_GGI_mansync_state.nrsync) {
		for (i = 0; i< _GGI_mansync_state.nrvisuals; i++) {
			if (!MANSYNC_ISASYNC(_GGI_mansync_state.visuals[i])) {
				ggi_visual *vis= _GGI_mansync_state.visuals[i];
				_ggiInternFlush(vis, 0, 0, LIBGGI_X(vis),
						LIBGGI_Y(vis), 0);
			}
		}
	}
#ifdef HAVE_SIGPENDING
	do {
		sigset_t set;

		sigpending(&set);
		if (sigismember(&set, MANSYNC_SIGNAL)) {
			_GGI_mansync_state.skip = 1;
		}
	} while(0);
#else /* HAVE_SIGPENDING */
	ggCurTime(&newtv);
	newtv.tv_usec -= oldtv.tv_usec;
	newtv.tv_usec += (newtv.tv_sec - oldtv.tv_sec)*1000000;
	if (newtv.tv_usec > _GGI_mansync_state.usecs) {
		_GGI_mansync_state.skip = 1;
	}
#endif /* HAVE_SIGPENDING */

	_GGI_mansync_state.ignore_counter--;
	signal(MANSYNC_SIGNAL, _GGI_mansync_handler);
}


int _GGI_mansync_start(ggi_visual *vis)
{
	int fpsrate = MANSYNC_FPS;
	char *str;

	GGIDPRINT("_GGI_mansync_start() (MANSYNC_CHILD) called.\n");

	if (!MANSYNC_ISASYNC(vis)) return -1;

	/* Child process already started...? */
	if (_GGI_mansync_state.nrsync) {
		_GGI_mansync_state.nrsync++;
		MANSYNC_ISASYNC(vis) = 0;
		return 0;
	}

	str = getenv("GGI_MANSYNC_FPS");
	if (str) {
		fpsrate = atoi(str);
		if (fpsrate <= 0) fpsrate = MANSYNC_FPS;
		GGIDPRINT_MISC("mansync: fpsrate: %d\n", fpsrate);
	}

	_GGI_mansync_state.ignore_counter = 0;
	_GGI_mansync_state.usecs = 1000000/fpsrate;
	_GGI_mansync_state.skip = 0;

	switch (_GGI_mansync_state.childpid = fork()) {
	case 0: {
		int ppid = getppid();

		while (1) {
			ggUSleep(_GGI_mansync_state.usecs);
			if (kill(ppid, MANSYNC_SIGNAL) != 0) _exit(0);
		}
	} break;
	case -1:
		fprintf(stderr, "_GGI_mansync_start(): fork() failed!\n");
		return -1;
	default:
		_GGI_mansync_state.oldsynchandler
			= signal(MANSYNC_SIGNAL, _GGI_mansync_handler);
		_GGI_mansync_state.nrsync++;
		MANSYNC_ISASYNC(vis) = 0;
	}

	return 0;
}


int _GGI_mansync_stop(ggi_visual *vis)
{
	GGIDPRINT("_GGI_mansync_stop() (MANSYNC_CHILD) called.\n");

	if (MANSYNC_ISASYNC(vis))
		return -1;

	MANSYNC_ISASYNC(vis) = 1;

	if (--_GGI_mansync_state.nrsync)
		return 0;	/* Continue process if other visuals left */

	signal(MANSYNC_SIGNAL, SIG_IGN);

	kill(_GGI_mansync_state.childpid, SIGKILL);
	waitpid(_GGI_mansync_state.childpid, NULL, 0);
	signal(MANSYNC_SIGNAL, _GGI_mansync_state.oldsynchandler);

	return 0; 
}


int _GGI_mansync_ignore(ggi_visual *vis)
{
	if (MANSYNC_ISASYNC(vis)) 
		return -1;
	
	if (_GGI_mansync_state.ignore_counter == 0) {
		signal(MANSYNC_SIGNAL, SIG_IGN);
	}
	_GGI_mansync_state.ignore_counter++;

	return 0;
}


int _GGI_mansync_cont(ggi_visual *vis)
{
	if (LIBGGI_FLAGS(vis) & GGIFLAG_ASYNC) {
		/* Obscure situation where a visual called _mansync_ignore(),
		   then sets its flags to ASYNC before calling this.
		   Other visuals' mansync would have been stuck. */
		if (_GGI_mansync_state.nrsync) {
			signal(MANSYNC_SIGNAL, _GGI_mansync_handler);
			return 0;
		} else
			return -1;
	}	

	if (MANSYNC_ISASYNC(vis))
		return _GGI_mansync_start(vis);
	else {
		_GGI_mansync_state.ignore_counter--;
		if (_GGI_mansync_state.ignore_counter == 0)
			signal(MANSYNC_SIGNAL, _GGI_mansync_handler);
	}

	return 0;
}


/*  Add given visual to _GGI_mansync_state list. */

int _GGI_mansync_init(ggi_visual *vis)
{
	int nrvisuals;

	vis->helperpriv = _ggi_malloc(sizeof(struct mansync_hook));
	MANSYNC_ISASYNC(vis) = 1; /* Yes, this SHOULD be initialized to 1 */

	nrvisuals = ++_GGI_mansync_state.nrvisuals;

	GGIDPRINT("_GGI_mansync_init(): nrvisuals = %d\n", nrvisuals);

	_GGI_mansync_state.visuals = _ggi_realloc(
		_GGI_mansync_state.visuals, sizeof(ggi_visual*)*nrvisuals);

	_GGI_mansync_state.visuals[nrvisuals-1] = vis;
	
	return 0;
}


/* Remove given visual from _GGI_mansync_state.  */

int _GGI_mansync_deinit(ggi_visual *vis)
{
	int i, nrvisuals = _GGI_mansync_state.nrvisuals;

	_GGI_mansync_stop(vis);

	for (i = 0; i < nrvisuals; i++) {
		if (_GGI_mansync_state.visuals[i] == vis) {
			i++;

			if (--_GGI_mansync_state.nrvisuals) {
				memmove(_GGI_mansync_state.visuals+i-1,
					_GGI_mansync_state.visuals+i,
					sizeof(ggi_visual*) * (nrvisuals-i));

				_GGI_mansync_state.visuals = _ggi_realloc(
					_GGI_mansync_state.visuals,
					sizeof(ggi_visual*) * (nrvisuals-1));
			} else {
				free(_GGI_mansync_state.visuals);
				_GGI_mansync_state.visuals = NULL;
			}

			break;
		}
	}
	
	free(vis->helperpriv);
	vis->helperpriv = NULL;

	return 0;
}

