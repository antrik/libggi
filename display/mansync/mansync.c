/* $Id: mansync.c,v 1.10 2004/10/31 14:25:01 cegger Exp $
******************************************************************************

   Helper library for the implementation of SYNC mode on targets which are
   inherently ASYNC (e.g. X) and require manual flushes of the framebuffer.

   Copyright (C) 2004  Peter Ekberg  [peda@lysator.liu.se]
   Copyright (C) 1998  Steve Cheng   [steve@ggi-project.org]

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

#include "config.h"
#include <ggi/internal/ggi-dl.h>
#include <ggi/internal/ggi_debug.h>
#include <ggi/display/mansync.h>

#include <stdlib.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_STRING_H
#include <string.h>
#endif

#include <ggi/gg.h>


/* Hook structure to helperpriv of mansync visual. */

struct mansync_hook {
	int isasync;
	int ignore;
	struct gg_task task;

	int running;
};


#undef MANSYNC_ISASYNC
#define MANSYNC_PRIV(vis)	(vis->helperpriv)
#define MANSYNC_HOOK(vis)	((struct mansync_hook *)MANSYNC_PRIV(vis))
#define MANSYNC_ISASYNC(vis)	(MANSYNC_HOOK(vis)->isasync)
#define MANSYNC_IGNORE(vis)	(MANSYNC_HOOK(vis)->ignore)
#define MANSYNC_TASK(vis)	(MANSYNC_HOOK(vis)->task)


static int _GGI_mansync_task(struct gg_task *task)
{
	ggi_visual *vis = task->hook;

	if (MANSYNC_IGNORE(vis))
		return 0;

	if (MANSYNC_ISASYNC(vis))
		return 0;

#warning Want to call ggiFlush for threaded ggTask implementations, but that hangs display-x on signal based ggTask.
	_ggiInternFlush(vis, 0, 0, LIBGGI_VIRTX(vis), LIBGGI_VIRTY(vis), 0);

	return 0;
}


int _GGI_mansync_init(ggi_visual *vis)
{
	MANSYNC_PRIV(vis) = _ggi_malloc(sizeof(struct mansync_hook));
	memset(&MANSYNC_TASK(vis), 0, sizeof(struct gg_task));
	MANSYNC_ISASYNC(vis) = 1; /* Yes, this SHOULD be initialized to 1. */
	MANSYNC_IGNORE(vis) = 1;
	MANSYNC_TASK(vis).cb = _GGI_mansync_task;
	MANSYNC_TASK(vis).hook = vis;
	return 0;
}


int _GGI_mansync_deinit(ggi_visual *vis)
{
	LIBGGI_ASSERT(!MANSYNC_HOOK(vis)->running,
		"Can't deinit mansync as long as mansync is running");

	free(MANSYNC_PRIV(vis));
	MANSYNC_PRIV(vis) = NULL;

	return 0;
}


int _GGI_mansync_start(ggi_visual *vis)
{
	int fpsrate = 0;
	char *str;
	int tick;

	GGIDPRINT("_GGI_mansync_start() (MANSYNC_TASK) called.\n");

	if (!MANSYNC_ISASYNC(vis))
		return -1;

	str = getenv("GGI_MANSYNC_FPS");
	if (str)
		fpsrate = atoi(str);

	if (fpsrate <= 0)
		fpsrate = MANSYNC_FPS;

	tick = ggTimeBase();
	MANSYNC_TASK(vis).pticks = 1000000 / (tick * fpsrate);
	if (MANSYNC_TASK(vis).pticks <= 0)
		MANSYNC_TASK(vis).pticks = 1;
	if (MANSYNC_TASK(vis).pticks >= GG_SCHED_TICK_WRAP)
		MANSYNC_TASK(vis).pticks = GG_SCHED_TICK_WRAP - 1;
	MANSYNC_TASK(vis).ncalls = 0;
	MANSYNC_ISASYNC(vis) = 0;
	MANSYNC_IGNORE(vis) = 0;

	if (ggAddTask(&MANSYNC_TASK(vis))) {
		return -1;
	}

	MANSYNC_HOOK(vis)->running = 1;
	return 0;
}


int _GGI_mansync_stop(ggi_visual *vis)
{
	int ret;

	GGIDPRINT("_GGI_mansync_stop() (MANSYNC_TASK) called.\n");

	LIBGGI_ASSERT(MANSYNC_HOOK(vis)->running,
		"Can't stop mansync without starting it first");

	if (MANSYNC_ISASYNC(vis))
		return -1;

	MANSYNC_ISASYNC(vis) = 1;
	MANSYNC_IGNORE(vis) = 1;

	ret = ggDelTask(&MANSYNC_TASK(vis));
	if (ret != 0) return ret;

	MANSYNC_HOOK(vis)->running = 0;
	return 0;
}


/* Tasks can't be arbitrarily suspended so these functions
 * just sets a flag so that the task handler does nothing.
 * Can't add/del the task as cont/ignore can be called from
 * within flush, which is called from the task handler.
 */

int _GGI_mansync_ignore(ggi_visual *vis)
{
	GGIDPRINT("_GGI_mansync_ignore() (MANSYNC_TASK) called.\n");

	if (MANSYNC_IGNORE(vis)) {
		return -1;
	}

	MANSYNC_IGNORE(vis) = 1;
	return 0;
}


int _GGI_mansync_cont(ggi_visual *vis)
{
	GGIDPRINT("_GGI_mansync_cont() (MANSYNC_TASK) called.\n");

	if (LIBGGI_FLAGS(vis) & GGIFLAG_ASYNC)
		return -1;

	if (!MANSYNC_IGNORE(vis)) {
		return -1;
	}

	MANSYNC_IGNORE(vis) = 0;
	return 0;
}
