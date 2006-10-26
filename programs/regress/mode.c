/* $Id: mode.c,v 1.22 2006/10/26 07:31:30 pekberg Exp $
******************************************************************************

   This is a regression-test for mode handling.

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
#include <ggi/internal/internal.h>
#include <ggi/internal/ggi_debug.h>
#include <ggi/ggi.h>
#include <ggi/errors.h>

#include <string.h>

#include "testsuite.inc.c"

#include <ggi/display/modelist.h>
#define WANT_MODELIST2
#define WANT_LIST_CHECKMODE
#include "../../display/common/modelist.inc"


static void testcase1(const char *desc)
{
	int err;
	ggi_visual_t vis;
	ggi_mode mode;


	printteststart(__FILE__, __PRETTY_FUNCTION__, EXPECTED2PASS, desc);
	if (dontrun) return;

	vis = ggNewStem(libggi, NULL);
	printassert(vis != NULL, "ggNewStem failed\n");

	err = ggiOpen(vis, NULL);
	printassert(err == GGI_OK, "ggiOpen() failed with %i\n", err);

	err = ggiCheckSimpleMode(vis, GGI_AUTO, GGI_AUTO, GGI_AUTO, GT_AUTO, &mode);
	if (err == GGI_OK) {
		if (mode.visible.x == GGI_AUTO) {
			printfailure("visible.x: expected return value: != GGI_AUTO\n"
					"actual return value: GGI_AUTO\n");
			return;
		}
		if (mode.visible.y == GGI_AUTO) {
			printfailure("visible.y: expected return value: != GGI_AUTO\n"
					"actual return value: GGI_AUTO\n");
			return;
		}
		if (mode.virt.x == GGI_AUTO) {
			printfailure("virt.x: expected return value: != GGI_AUTO\n"
					"actual return value: GGI_AUTO\n");
			return;
		}
		if (mode.virt.y == GGI_AUTO) {
			printfailure("virt.y: expected return value: != GGI_AUTO\n"
					"actual return value: GGI_AUTO\n");
			return;
		}
		if (mode.size.x == GGI_AUTO) {
			printfailure("size.x: expected return value: != GGI_AUTO\n"
					"actual return value: GGI_AUTO\n");
			return;
		}
		if (mode.size.y == GGI_AUTO) {
			printfailure("size.y: expected return value: != GGI_AUTO\n"
					"actual return value: GGI_AUTO\n");
			return;
		}
		if (mode.frames == GGI_AUTO) {
			printfailure("frames: expected return value: != GGI_AUTO\n"
					"actual return value: GGI_AUTO\n");
			return;
		}
		if (mode.graphtype == GT_AUTO) {
			printfailure("graphtype: expected return value: != GT_AUTO\n"
					"actual return value: GT_AUTO\n");
			return;
		}
	}


	ggDelStem(vis);

	printsuccess();
	return;
}


static void testcase2(const char *desc)
{
	int err;
	ggi_visual_t vis;
	ggi_mode mode;


	printteststart(__FILE__, __PRETTY_FUNCTION__, EXPECTED2PASS, desc);
	if (dontrun) return;

	vis = ggNewStem(libggi, NULL);
	printassert(vis != NULL, "ggNewStem failed\n");

	err = ggiOpen(vis, NULL);
	printassert(err == GGI_OK, "ggiOpen() failed with %i\n", err);

	err = ggiCheckSimpleMode(vis, GGI_AUTO, GGI_AUTO, GGI_AUTO, GT_AUTO, &mode);

	err = ggiSetMode(vis, &mode);
	if (err != GGI_OK) {
		printfailure("ggiSetMode: expected return value: GGI_OK\n"
					"actual return value: %i\n", err);
		return;
	}

	ggDelStem(vis);

	printsuccess();
	return;
}


static void testcase3(const char *desc)
{
	int err;
	ggi_visual_t vis;
	ggi_mode mode;


	printteststart(__FILE__, __PRETTY_FUNCTION__, EXPECTED2PASS, desc);
	if (dontrun) return;

	vis = ggNewStem(libggi, NULL);
	printassert(vis != NULL, "ggNewStem failed\n");

	err = ggiOpen(vis, NULL);
	printassert(err == GGI_OK, "ggiOpen() failed with %i\n", err);

	err = ggiCheckSimpleMode(vis, GGI_AUTO, GGI_AUTO, 2, GT_AUTO, &mode);
	printassert(err == GGI_OK, "frames are apparently not supported\n");
	if (err != GGI_OK) {
		ggDelStem(vis);
		printsuccess();
		return;
	}

	err = ggiSetMode(vis, &mode);
	if (err != GGI_OK) {
		printfailure("ggiSetMode: expected return value: GGI_OK\n"
					"actual return value: %i\n", err);
		return;
	}

	ggDelStem(vis);


	printsuccess();
	return;
}


static void testcase4(const char *desc)
{
	int err;
	ggi_visual_t vis;
	ggi_mode mode;
	ggi_coord size;

	printteststart(__FILE__, __PRETTY_FUNCTION__, EXPECTED2PASS, desc);
	if (dontrun) return;

	vis = ggNewStem(libggi, NULL);
	printassert(vis != NULL, "ggNewStem failed\n");

	err = ggiOpen(vis, NULL);
	printassert(err == GGI_OK, "ggiOpen() failed with %i\n", err);

	err = ggiCheckSimpleMode(
		vis, GGI_AUTO, GGI_AUTO, GGI_AUTO, GT_AUTO, &mode);
	printassert(err == GGI_OK, "ggiCheckSimpleMode: can't find a mode\n");
	if(err != GGI_OK) {
		ggDelStem(vis);
		printsuccess();
		return;
	}

	printassert(mode.size.x != GGI_AUTO && mode.size.y != GGI_AUTO,
		"physical size is apparently not supported\n");
	if(mode.size.x == GGI_AUTO || mode.size.y == GGI_AUTO) {
		ggDelStem(vis);
		printsuccess();
		return;
	}

	/* Clear out all but the physical size */
	mode.frames    = GGI_AUTO;
	mode.visible.x = GGI_AUTO;
	mode.visible.y = GGI_AUTO;
	mode.virt.x    = GGI_AUTO;
	mode.virt.y    = GGI_AUTO;
	mode.graphtype = GT_AUTO;
	mode.dpp.x     = GGI_AUTO;
	mode.dpp.y     = GGI_AUTO;

	size = mode.size;

	/* This mode should be there */
	err = ggiCheckMode(vis, &mode);
	ggDelStem(vis);

	if (err != GGI_OK) {
		printfailure("ggiCheckMode: expected return value: GGI_OK\n"
					"actual return value: %i\n", err);
		return;
	}

	if (mode.size.x != size.x) {
		printfailure(
			"ggiCheckMode: size.x: expected return value: %i\n"
					"actual return value: %i\n",
			size.x, mode.size.x);
		return;
	}

	if (mode.size.y != size.y) {
		printfailure(
			"ggiCheckMode: size.y: expected return value: %i\n"
					"actual return value: %i\n",
			size.y, mode.size.y);
		return;
	}

	printsuccess();
}


static void testcase5(const char *desc)
{
	int err;
	ggi_visual_t vis;
	ggi_mode sug_mode, final_mode;
	int visible_w, visible_h;

	printteststart(__FILE__, __PRETTY_FUNCTION__, EXPECTED2PASS, desc);
	if (dontrun) return;

	vis = ggNewStem(libggi, NULL);
	printassert(vis != NULL, "ggNewStem failed\n");

	err = ggiOpen(vis, NULL);
	printassert(err == GGI_OK, "ggiOpen() failed with %i\n", err);

	ggiSetFlags(vis, GGIFLAG_ASYNC);

	/* Get the default mode */
	err = ggiCheckGraphMode (vis, GGI_AUTO, GGI_AUTO, GGI_AUTO, GGI_AUTO,
				GT_AUTO, &sug_mode);
	if (err != GGI_OK) {
		printfailure("ggiCheckGraphMode: No graphic mode available\n");
		ggDelStem(vis);
		return;
	}

	visible_w = sug_mode.visible.x;
	visible_h = sug_mode.visible.y;

	err = ggiCheckGraphMode(vis, visible_w, visible_h, visible_w, visible_h*2,
				GT_AUTO, &final_mode);
	if (!err) {
		/* actually print an info output */
		printassert(0 == 1, "Info: Applications may assume now,"
				" panning via ggiSetOrigin() is available\n");

		/* Note, Applications have no other way to figure out if
		 * ggiSetOrigin() is available or not
		 */
	} else {
		final_mode = sug_mode;
	}

	err = ggiSetMode(vis, &final_mode);
	if (err) {
		printfailure("ggiSetMode() failed although ggiCheckGraphMode() was OK!\n");
		ggDelStem(vis);
		return;
	}

	ggDelStem(vis);

	printsuccess();
	return;
}


static void testcase6(const char *desc)
{
	int err;
	ggi_visual_t vis;
	ggi_mode mode;

	printteststart(__FILE__, __PRETTY_FUNCTION__, EXPECTED2PASS, desc);
	if (dontrun) return;

	err = ggiInit();
	printassert(err >= 0, "ggiInit failed with %i\n", err);

	vis = ggNewStem(libggi, NULL);
	printassert(vis != NULL, "ggNewStem failed\n");

	err = ggiOpen(vis, NULL);
	printassert(err == GGI_OK, "ggiOpen() failed with %i\n", err);

	/* async mode disables mansync if used */
	ggiSetFlags(vis, GGIFLAG_ASYNC);

	/* Get the default mode */
	err = ggiCheckGraphMode (vis, 640, 480, GGI_AUTO, GGI_AUTO,
				GT_AUTO, &mode);
	if (err != GGI_OK) {
		printfailure("ggiCheckGraphMode: #1: No 640x480 mode available\n");
		ggDelStem(vis);
		return;
	}

	err = ggiSetMode(vis, &mode);
	if (err != GGI_OK) {
		printfailure("ggiSetMode() #1: failed although ggiCheckGraphMode() was OK!\n");
		ggDelStem(vis);
		return;
	}


	err = ggiCheckGraphMode(vis, 320, 200, GGI_AUTO, GGI_AUTO,
				GT_AUTO, &mode);
	if (err != GGI_OK) {
		printfailure("ggiCheckGraphMode: #2: No 320x200 mode available\n");
		ggDelStem(vis);
		return;
	}

	err = ggiSetMode(vis, &mode);
	if (err != GGI_OK) {
		printfailure("ggiSetMode() #2: resetting a mode failed although ggiCheckGraphMode() was OK!\n");
		ggDelStem(vis);
		return;
	}


	ggDelStem(vis);

	printsuccess();
	return;
}


static void testcase7(const char *desc)
{
	int err;
	ggi_visual_t vis;
	ggi_mode mode;

	printteststart(__FILE__, __PRETTY_FUNCTION__, EXPECTED2PASS, desc);
	if (dontrun) return;

	vis = ggNewStem(libggi, NULL);
	printassert(vis != NULL, "ggNewStem failed\n");

	err = ggiOpen(vis, NULL);
	printassert(err == GGI_OK, "ggiOpen() failed with %i\n", err);

	/* sync mode enables mansync if used */

	/* Get the default mode */
	err = ggiCheckGraphMode (vis, 640, 480, GGI_AUTO, GGI_AUTO,
				GT_AUTO, &mode);
	if (err != GGI_OK) {
		printfailure("ggiCheckGraphMode: #1: No 640x480 mode available\n");
		ggDelStem(vis);
		return;
	}

	err = ggiSetMode(vis, &mode);
	if (err != GGI_OK) {
		printfailure("ggiSetMode() #1: failed although ggiCheckGraphMode() was OK!\n");
		ggDelStem(vis);
		return;
	}


	err = ggiCheckGraphMode(vis, 320, 200, GGI_AUTO, GGI_AUTO,
				GT_AUTO, &mode);
	if (err != GGI_OK) {
		printfailure("ggiCheckGraphMode: #2: No 320x200 mode available\n");
		ggDelStem(vis);
		return;
	}

	err = ggiSetMode(vis, &mode);
	if (err != GGI_OK) {
		printfailure("ggiSetMode() #2: resetting a mode failed although ggiCheckGraphMode() was OK!\n");
		ggDelStem(vis);
		return;
	}


	ggDelStem(vis);

	printsuccess();
	return;
}


static void testcase8(const char *desc)
{
	int err;
	ggi_visual_t vis;
	ggi_mode mode;

	printteststart(__FILE__, __PRETTY_FUNCTION__, EXPECTED2PASS, desc);
	if (dontrun) return;

	vis = ggNewStem(libggi, NULL);
	printassert(vis != NULL, "ggNewStem failed\n");

	err = ggiOpen(vis, NULL);
	printassert(err == GGI_OK, "ggiOpen() failed with %i\n", err);

	mode.virt.x = mode.virt.y = GGI_AUTO;
	mode.visible.x = mode.visible.y = GGI_AUTO;
	mode.frames = GGI_AUTO;
	mode.graphtype = GT_AUTO;
	mode.dpp.x = mode.dpp.y = 1;
        mode.size.x = -19493;
        mode.size.y = 31831;

	/* Get the default mode */
	ggiCheckMode(vis, &mode);

	err = ggiSetMode(vis, &mode);
	if (err != GGI_OK) {
		printfailure("ggiSetMode() failed even though ggiCheckMode() was called!\n");
		ggDelStem(vis);
		return;
	}

	ggDelStem(vis);

	printsuccess();
	return;
}


static void modelist_helper(unsigned int mcount, ggi_mode *modes,
	unsigned int tcount, ggi_mode *tests, int *match, int *exp_mode)
{
	int err;
	unsigned int i;
	char request_mode[256];
	char expect_mode[256];
	char return_mode[256];
	ggi_modelist *ml;
	ggi_mode_padded mp;

	ml = _GGI_modelist_create(mcount);

	for (i = 0; i < mcount; ++i) {
		mp.mode = modes[i];
		mp.user_data = NULL;
		_GGI_modelist_append(ml, &mp);
	}

	for (i = 0; i < tcount; ++i) {
		int matched;
		mp.mode = tests[i];
		mp.user_data = NULL;
		err = _GGI_modelist_checkmode(ml, &mp);

		matched = err == GGI_OK;
		if (matched != match[i]) {
			ggiSPrintMode(request_mode, &tests[i]);
			ggiSPrintMode(expect_mode, &modes[exp_mode[i]]);
			ggiSPrintMode(return_mode, &mp.mode);
			printfailure("_GGI_modelist_checkmode() %s\n"
				"Test:      %d\n"
				"Requested: %s\n"
				"Expected:  %s\n"
				"%s %s\n",
				i,
				matched ? "succeeded without a match!" :
					"failed when there is a match!",
				request_mode,
				expect_mode,
				matched ? "Returned: " : "Suggested:",
				return_mode);
			break;
		}

		if (memcmp(&mp.mode, &modes[exp_mode[i]], sizeof(ggi_mode))) {
			ggiSPrintMode(request_mode, &tests[i]);
			ggiSPrintMode(expect_mode, &modes[exp_mode[i]]);
			ggiSPrintMode(return_mode, &mp.mode);
			printfailure("_GGI_modelist_checkmode() %s "
				"the wrong mode!\n"
				"Test:      %d\n"
				"Requested: %s\n"
				"Expected:  %s\n"
				"%s %s\n",
				matched ? "returned" : "suggested",
				i,
				request_mode,
				expect_mode,
				matched ? "Returned: " : "Suggested:",
				return_mode);
			break;
		}
	}

	_GGI_modelist_destroy(ml);

	if (i == tcount)
		printsuccess();
}

static void testcase9(const char *desc)
{
	/* database of modes */
	ggi_mode modes[] = {
		{ 1, { 100, 100}, { 100, 100}, { 100, 100}, GT_32BIT, {1,1} },
		{ 1, { 200, 200}, { 200, 200}, { 200, 200}, GT_16BIT, {1,1} }
	};
	/* list of modes to test */
	ggi_mode tests[] = {
		{ GGI_AUTO, {GGI_AUTO,GGI_AUTO}, {GGI_AUTO,GGI_AUTO},
			{200,200}, GT_AUTO, {GGI_AUTO,GGI_AUTO} },
		{ GGI_AUTO, {GGI_AUTO,GGI_AUTO}, {GGI_AUTO,GGI_AUTO},
			{100,100}, GT_AUTO, {GGI_AUTO,GGI_AUTO} },
		{ GGI_AUTO, {100,100}, {GGI_AUTO,GGI_AUTO},
			{GGI_AUTO,GGI_AUTO}, GT_AUTO, {GGI_AUTO,GGI_AUTO} },
		{ GGI_AUTO, {101,101}, {GGI_AUTO,GGI_AUTO},
			{GGI_AUTO,GGI_AUTO}, GT_AUTO, {GGI_AUTO,GGI_AUTO} },
		{ GGI_AUTO, {200,200}, {GGI_AUTO,GGI_AUTO},
			{GGI_AUTO,GGI_AUTO}, GT_AUTO, {GGI_AUTO,GGI_AUTO} }
	};
	/* is a perfect match expected for above tests? */
	int match[] = {
		1,
		1,
		1,
		0,
		1
	};
	/* what mode should be returned/suggested for above tests? */
	int exp_mode[] = {
		1,
		0,
		0,
		1,
		1
	};

	printteststart(__FILE__, __PRETTY_FUNCTION__, EXPECTED2PASS, desc);
	if (dontrun) return;

	modelist_helper(sizeof(modes)/sizeof(modes[0]), modes,
		sizeof(tests)/sizeof(tests[0]), tests, match, exp_mode);
}


int main(int argc, char * const argv[])
{
	parseopts(argc, argv);
	printdesc("Regression testsuite mode handling\n\n");

	testcase1("Check that ggiCheckMode() doesn't return GGI_AUTO");
	testcase2("Check that ggiSetMode() can actually set the mode that has been suggested by ggiCheckMode");
	testcase3("Check setting a mode with a given number of frames");
	testcase4("Check setting a mode by its physical size");
	testcase5("Set up the mode in the ggiterm way");
	testcase6("Check that re-setting of a different mode works [async mode]");
	testcase7("Check that re-setting of a different mode works [sync mode]");
	testcase8("Check checking then setting a mode with braindamaged visual size");
	testcase9("Check modelist for list of modes");

	printsummary();

	return 0;
}
