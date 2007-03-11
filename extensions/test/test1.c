/* $Id: test1.c,v 1.13 2007/03/11 21:54:44 soyt Exp $
******************************************************************************

   Test extension test1.c

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

/*
 * The API of Extension #1 looks a bit like this :
 */

static ggfunc_api_op_init test1_init;
static ggfunc_api_op_exit test1_exit;
static ggfunc_api_op_attach test1_attach;
static ggfunc_api_op_detach test1_detach;
/*
static ggfunc_api_op_getenv test1_getenv;
static ggfunc_api_op_plug test1_plug;
static ggfunc_api_op_unplug test1_unplug;
*/

static struct gg_api_ops test1_ops = {
	test1_init,
	test1_exit,
	test1_attach,
	test1_detach,
	NULL, /* test1_getenv, */
	NULL, /* test1_plug, */
	NULL  /* test1_unplug */
};

static struct gg_api test1 = GG_API_INIT("test1", 1, 0, &test1_ops);

struct gg_api *ggitest1 = &test1;

static struct gg_observer *observer;


static int changed(struct ggi_visual *vis,int whatchanged)
{
	printf("changed called for extension 1 - vis=%p, %i \n",
		(void *)vis, whatchanged);

	switch(whatchanged) {
	case GGI_CHG_APILIST:
		{	int temp;
			char api[GGI_MAX_APILEN];
			char args[GGI_MAX_APILEN];
			for (temp=0;
			    0 == ggiGetAPI(vis->instance.stem, temp, api, args);
			    temp++)
			{
				ggstrlcat(api,"-test1", sizeof(api));
				printf("Would now load #%d: %s(%s)\n",
					temp, api, args);
			}
		}
		break;
	}

	return 0;
}

static void
_test1_attach_finalize(struct gg_stem *stem)
{
	const char *str;
	struct ggi_visual *vis = GGI_VISUAL(stem);

	printf("_test1_attach_finalize %p\n", (void *)stem);

	str = (const char *)STEM_API_PRIV(stem, ggitest1);
	str = strdup("Test 1 private Data !");

	/* Now fake an "API change" so the right libs get loaded */
	changed(vis, GGI_CHG_APILIST);
}

static int
test1_attach(struct gg_api *api, struct gg_stem *stem)
{
	struct ggi_visual *vis;
	int rc;

	rc = ggiAttach(stem);
	if (rc < 0) {
		printf("_test1_attach: ggiAttach failed. rc %i\n", rc);
		return rc;
	}

	vis = GGI_VISUAL(stem);

	if (!vis) {
		printf("_test1_attach: defer until visual is opened.\n");
		return 0;
	}

	_test1_attach_finalize(stem);

	return 0;
}

static void
test1_detach(struct gg_api *api, struct gg_stem *stem)
{
	ggiDetach(stem);
}


static int
observe_visuals(void *arg, uint32_t flag, void *data)
{
	struct ggi_visual *vis = data;

	if (!STEM_HAS_API(vis->instance.stem, ggitest1))
		return 0;

	switch (flag) {
	case GGI_OBSERVE_VISUAL_OPENED:
		_test1_attach_finalize(vis->instance.stem);
		break;

	case GGI_OBSERVE_VISUAL_APILIST:
		changed(vis, flag);
		break;
	}

	return 0;
}


int ggiTest1Init(void)
{
	int rc;

	rc = ggInitAPI(ggitest1);
	printf("Initialized Test1 extension. Return value: %i\n", rc);
	return rc;
}


static int
test1_init(struct gg_api *api)
{

	ggiInit();
	observer = ggObserve(libggi->channel, observe_visuals, NULL);

	return GGI_OK;
}

int ggiTest1Exit(void)
{
	int rc;

	rc = ggExitAPI(ggitest1);
	printf("DeInitailized Test1 extension. rc=%i\n", rc);

	return rc;
}

static void
test1_exit(struct gg_api *api)
{
	ggDelObserver(observer);
	ggiExit();

	return;
}


int ggiTest1Attach(struct gg_stem *stem)
{
	int rc;

	rc = ggAttach(ggitest1, stem);
	printf("Attached Test1 extension to %p. rc=%i\n",
		(void *)stem, rc);

	return rc;
}

int ggiTest1Detach(struct gg_stem *stem)
{
	int rc;

	rc = ggDetach(ggitest1, stem);
	printf("Detached Test1 extension from %p. rc=%i\n",
		(void *)stem, rc);

	return rc;
}

void ggiTest1PrintLocaldata(struct gg_stem *stem)
{
	printf("%s\n",(char *)STEM_API_PRIV(stem,ggitest1));
}

void ggiTest1SetLocaldata(struct gg_stem *stem, const char *content)
{
	const char *str;

	str = (const char *)STEM_API_PRIV(stem, ggitest1);
	if (str != NULL) free(str);

	str = strdup(content);
}
