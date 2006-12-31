/* $Id: input.c,v 1.9 2006/12/31 04:17:41 pekberg Exp $
******************************************************************************

   This is a regression-test for visual <-> input association.

   Written in 2004 by Christoph Egger

   This software is placed in the public domain and can be used freely
   for any purpose. It comes without any kind of warranty, either
   expressed or implied, including, but not limited to the implied
   warranties of merchantability or fitness for a particular purpose.
   Use it at your own risk. the author is not responsible for any damage
   or consequences raised by use or inability to use this program.

******************************************************************************
*/


#include "config.h"
#include <ggi/gg.h>
#include <ggi/ggi.h>
#include <ggi/gii.h>

#include <string.h>

#include "testsuite.inc.c"

typedef struct ggi_originbounds {
	uint32_t first,last;
} ggi_originbounds_t;

ggi_visual_t vis;
ggi_mode mode;

static int ggi_findorigin(ggi_visual_t visual,ggi_originbounds_t *origins)
{
	struct gii_source_iter src_iter;
	uint32_t origin;

	origins->first = 0xffffffff;
	origins->last  = 0x00000000;

	src_iter.stem = visual;
	giiIterSources(&src_iter);
	GG_ITER_FOREACH(&src_iter) {
		origin = src_iter.origin;
		if (origins->first > origin) origins->first = origin;
		if (origins->last  < origin) origins->last  = origin;
	}
	GG_ITER_DONE(&src_iter);
	return (origins->first > origins->last);
}


static int precase(void)
{
	int err;

	if (dontrun) return 0;

	vis = ggNewStem(libgii, libggi, NULL);
	if (vis == NULL) {
		printfailure("ggiOpen: Couldn\'t create and attach LibGGI to stem.\n");
		return GGI_EUNKNOWN;
	}

	err = ggiOpen(vis, NULL);
	if (err < 0) {
		printfailure("ggiOpen: Couldn\'t open default visual.\n");
		return err;
	}

	ggiCheckSimpleMode(vis, GGI_AUTO, GGI_AUTO, GGI_AUTO, GT_AUTO, &mode);

	ggiSetMode(vis, &mode);

	return 0;
}

static int postcase(void)
{
	int err;

	if (dontrun) return 0;

	err = ggiClose(vis);
	if (err < 0) {
		printfailure("ggiClose: Couldn\'t close default visual.\n");
		return err;
	}
	ggDelStem(vis);

	return 0;
}


static int ggi_opentestvis(ggi_visual_t *visual, ggi_originbounds_t *ori, 
			   struct gg_stem *input, const char *visname)
{
	uint32_t origins;
	int err;

	*visual = ggNewStem(libgii, libggi, NULL);
	printassert(*visual != NULL, "ggNewStem failed\n");

	err = ggiOpen(*visual, NULL);
	if (err != GGI_OK) {
		printfailure("expected return value for ggiOpen: \"GGI_OK\"\n"
			"actual return value: \"%i\"\n", err);
		return -1;
	}

	err = ggiCheckSimpleMode(*visual, GGI_AUTO, GGI_AUTO, GGI_AUTO, GT_AUTO, &mode);
	err = ggiSetMode(*visual, &mode);
	err = ggi_findorigin(*visual, ori);
	if (err != GGI_OK) {
		printfailure("expected return value for findorigin: 0\n"
			"actual return value: %i\n", err);
		return -1;
	}

	for (origins = ori->first; origins <= ori->last; origins++) {
		err = giiTransfer(*visual, input, origins);
		if (err != GGI_OK) {
			printfailure("joined inps: expected return value: \"GGI_OK\"\n"
				"actual return value: \"%i\"\n", err);
			return -1;
		}
	}
	return 0;
}

static int ggi_closetestvis(ggi_visual_t *visual,ggi_originbounds_t *ori, 
			    struct gg_stem *input, const char *visname,int islast)
{
	uint32_t origins;
	struct gg_stem *newhand;
	int errcode = GGI_EUNKNOWN;

	for (origins = ori->first; origins <= ori->last; origins++) {
		newhand = ggNewStem(libgii, NULL);
		errcode = giiTransfer(input, newhand, origins);
		ggDelStem(newhand);
	}
	if (errcode != GGI_OK) {
		printfailure("last haveone expected return value: \"GGI_OK\"\n"
			"actual return value: \"%i\"\n");
		return -1;
	}

	ggiClose(*visual);
	ggDelStem(*visual);
	return 0;
}

static void testcase1(const char *desc)
{
	ggi_visual_t vis1,vis2;
	ggi_originbounds_t oris1,oris2;
	struct gg_stem *joinedinps;

	printteststart(__FILE__, __PRETTY_FUNCTION__, EXPECTED2PASS, desc);
	if (dontrun) return;

	joinedinps = ggNewStem(libgii, NULL);

	if (ggi_opentestvis(&vis1, &oris1, joinedinps, "vis1")) return;
	if (ggi_opentestvis(&vis2, &oris2, joinedinps, "vis2")) return;

	if (ggi_closetestvis(&vis1, &oris1, joinedinps, "vis1", 0)) return;
	if (ggi_closetestvis(&vis2, &oris2, joinedinps, "vis2", 1)) return;

	ggDelStem(joinedinps);

	printsuccess();
	return;
}

static void testcase2(const char *desc)
{
	ggi_visual_t vis1,vis2;
	ggi_originbounds_t oris1,oris2;
	struct gg_stem *joinedinps;

	printteststart(__FILE__, __PRETTY_FUNCTION__, EXPECTED2PASS, desc);
	if (dontrun) return;

	joinedinps = ggNewStem(libgii, NULL);

	if (ggi_opentestvis(&vis1, &oris1, joinedinps, "vis1")) return;
	if (ggi_opentestvis(&vis2, &oris2, joinedinps, "vis2")) return;

	if (ggi_closetestvis(&vis2, &oris2, joinedinps, "vis2", 0)) return;
	if (ggi_closetestvis(&vis1, &oris1, joinedinps, "vis1", 1)) return;

	ggDelStem(joinedinps);

	printsuccess();
	return;
}


int main(int argc, char * const argv[])
{
	int rc;

	parseopts(argc, argv);
	printdesc("Regression testsuite visual <-> input association\n\n");

	rc = precase();
	if (rc != 0) exit(rc);

	testcase1("Check that joining/splitting multiple visuals works (overlapping).");
	testcase2("Check that joining/splitting multiple visuals works (bracketed).");

	rc = postcase();
	if (rc != 0) exit(rc);

	printsummary();

	return 0;
}
