/* $Id: events.c,v 1.1 2001/05/12 23:02:08 cegger Exp $
******************************************************************************

   Display-kgi: event management

   Copyright (C) 1997 Jason McMullan    [jmcc@ggi-project.org]

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

#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>

#include <ggi/internal/ggi-dl.h>
#include "../common/evqueue.inc"

/************************** GGI Functions ***************************/
/* Event Handling */
ggi_event_mask GGIeventpoll(ggi_visual_t vis,ggi_event_mask mask,
			    struct timeval *t)
{
#define BUFFER_SIZE (sizeof(ggi_event)*4)
	fd_set fds;
	int err;
	struct timeval *timeout,t_zero={0,0};
	ggi_event_mask evmask;
	unsigned char *bp,buff[BUFFER_SIZE];

GGIDPRINT("GGIeventpoll(%p,0x%.8x",vis,mask);
if (t==NULL) {
	GGIDPRINT(",NULL)\n");
} else {
	GGIDPRINT(",{%d,%d})\n",t->tv_sec,t->tv_usec);
}

	evmask=_ggiEvQueueSeen(vis,mask);
	if (evmask!=0)  
		return evmask;

	if (LIBGGI_SELECT_FD(vis) < 0) 
		return 0;

	timeout = &t_zero;
	do {
		FD_ZERO(&fds);
		FD_SET(LIBGGI_SELECT_FD(vis),&fds);

		/* !!! FIXME  The following EINTR handling code relies
		 * on the fact that Linux modifies the timeout to
		 * indicate the time not slept.
		 */

		err=select(LIBGGI_SELECT_FD(vis)+1,&fds,NULL,NULL,timeout);

		if ((err < 0) && (errno == EINTR)) {
			continue;
		}
		if (err < 0) {
			return 0;
		}

		timeout=t;

		if (FD_ISSET(LIBGGI_SELECT_FD(vis),&fds)) {
			err=read(LIBGGI_SELECT_FD(vis),buff,BUFFER_SIZE);
			if (err<=0) 
				continue;

			for (bp=buff;err > 0;err-=*bp,bp+=*bp) 
				_ggiEvQueueAdd(vis,(ggi_event *)bp);
		
			evmask = _ggiEvQueueSeen(vis,mask);
		}
	} while (evmask==0 && t==NULL);

	return evmask;
}

int GGIeventread(ggi_visual_t vis,ggi_event *ev,ggi_event_mask mask)
{
	/* Block if we don't have anything queued... */
	GGIeventpoll(vis,mask,NULL);
	return _ggiEvQueueRelease(vis,ev,mask);
}
