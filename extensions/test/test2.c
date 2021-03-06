/* $Id: test2.c,v 1.13 2007/12/28 13:20:12 cegger Exp $
******************************************************************************

   Test extension test2.c

   Copyright (C) 1997 Uwe Maurer - uwe_maurer@t-online.de
   Copyright (C) 1998 Andreas Beck - becka@ggi-project.org
  
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

#include <stdio.h>
#include <string.h>

#include "config.h"
#include <ggi/internal/ggi.h>
#include <ggi/gg.h>

#include "test1.h"
#include "test2.h"

/*
 * The API of Extension #2 looks a bit like this :
 * It happens to need Extension #1 internally.
 */

static ggfunc_api_op_init test2_init;
static ggfunc_api_op_exit test2_exit;
static ggfunc_api_op_attach test2_attach;
static ggfunc_api_op_detach test2_detach;
/*
static ggfunc_api_op_getenv test2_getenv;
static ggfunc_api_op_plug test2_plug;
static ggfunc_api_op_unplug test2_unplug;
*/

static struct gg_api_ops test2_ops = {
	test2_init,
	test2_exit,
	test2_attach,
	test2_detach,
	NULL, /* test2_getenv, */
	NULL, /* test2_plug, */
	NULL  /* test2_unplug */
};

static struct gg_api test2 = GG_API_INIT("test2", 1, 0, &test2_ops);

struct gg_api *ggitest2 = &test2;

static struct gg_observer *observer;


static int changed(struct ggi_visual *vis,int whatchanged)
{
	printf("load extension 2 - vis: %p %i\n",
		(void *)vis, whatchanged);
	return 0;
}


static void
_test2_attach_finalize(struct gg_stem *stem)
{
	const char *str;
	struct ggi_visual *vis = GGI_VISUAL(stem);

	printf("_test2_attach_finalize %p\n", (void *)stem);

	str = (const char *)STEM_API_PRIV(stem, ggitest2);
	str = strdup("Test 2 private Data !");

	/* Now fake an "API change" so the right libs get loaded */
	changed(vis, GGI_CHG_APILIST);
}

static int
test2_attach(struct gg_api *api, struct gg_stem *stem)
{
	struct ggi_visual *vis;
	int rc;

	rc = ggiAttach(stem);
	if (rc < 0) {
		printf("_test2_attach: ggiAttach failed. rc %i\n", rc);
		return rc;
	}

	vis = GGI_VISUAL(stem);

	if (!vis) {
		printf("_test2_attach: defer until visual is opened.\n");
		return 0;
	}

	_test2_attach_finalize(stem);

	return 0;
}

static void
test2_detach(struct gg_api *api, struct gg_stem *stem)
{
	ggiDetach(stem);
}

static int
observe_visuals(void *arg, uint32_t flag, void *data)
{
	struct ggi_visual *vis = data;

	if (!STEM_HAS_API(vis->instance.stem, ggitest2))
		return 0;

	switch (flag) {
	case GGI_OBSERVE_VISUAL_OPENED:
		_test2_attach_finalize(vis->instance.stem);
		break;

	case GGI_OBSERVE_VISUAL_APILIST:
		changed(vis, flag);
		break;
	}

	return 0;
}



int ggiTest2Init(void)
{
	int rc;

	ggiTest1Init();	/* Make sure #1 is there - we need it */

	rc = ggInitAPI(ggitest2);
	printf("Initialized Test2 extension. Return value: %i\n", rc);

	return rc;
}

static int
test2_init(struct gg_api *api)
{

	ggiInit();
	observer = ggObserve(libggi->channel, observe_visuals, NULL);

	return GGI_OK;
}


int ggiTest2Exit(void)
{
	int rc;

	rc = ggExitAPI(ggitest2);
	printf("DeInitailized Test2 extension. rc=%i\n", rc);

	if (rc>=0) ggiTest1Exit();	/* Release #1. */

	return rc;
}

static void
test2_exit(struct gg_api *api)
{
	ggDelObserver(observer);
	ggiExit();

	return;
}


int ggiTest2Attach(struct gg_stem *stem)
{
	int rc;

	rc = ggAttach(ggitest2, stem);
	printf("Attached Test2 extension to %p. rc=%i\n",
		(void *)stem, rc);

	ggiTest1Attach(stem);

	return rc;
}

int ggiTest2Detach(struct gg_stem *stem)
{
	int rc;

	rc = ggDetach(ggitest2, stem);
	printf("Detached Test2 extension from %p. rc=%i\n",
		(void *)stem, rc);
	if (rc == 0)
		ggiTest1Detach(stem);

	return rc;
}

void ggiTest2PrintLocaldata(struct gg_stem *stem)
{
	ggiTest1PrintLocaldata(stem);
	printf("%s\n",(char *)STEM_API_PRIV(stem,ggitest2));
}

void ggiTest2SetLocaldata(struct gg_stem *stem, const char *content)
{
	char *str;

	str = (char *)STEM_API_PRIV(stem, ggitest2);
	if (str != NULL) free(str);

	str = strdup(content);
}

