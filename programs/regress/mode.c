/* $Id: mode.c,v 1.10 2005/01/18 20:22:46 cegger Exp $
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
#include <ggi/ggi.h>
#include <ggi/errors.h>

#include <string.h>

#include "testsuite.inc.c"


static void testcase1(const char *desc)
{
	int err;
	ggi_visual_t vis;
	ggi_mode mode;


	printteststart(__FILE__, __PRETTY_FUNCTION__, EXPECTED2PASS, desc);
	if (dontrun) return;

	err = ggiInit();
	printassert(err == GGI_OK, "ggiInit failed with %i\n", err);

	vis = ggiOpen(NULL);
	printassert(vis != NULL, "ggiOpen() failed\n");

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


	ggiClose(vis);
	ggiExit();

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

	err = ggiInit();
	printassert(err == GGI_OK, "ggiInit failed with %i\n", err);

	vis = ggiOpen(NULL);
	printassert(vis != NULL, "ggiOpen() failed\n");

	err = ggiCheckSimpleMode(vis, GGI_AUTO, GGI_AUTO, GGI_AUTO, GT_AUTO, &mode);

	err = ggiSetMode(vis, &mode);
	if (err != GGI_OK) {
		printfailure("ggiSetMode: expected return value: GGI_OK\n"
					"actual return value: %i\n", err);
		return;
	}

	ggiClose(vis);
	ggiExit();

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

	err = ggiInit();
	printassert(err == GGI_OK, "ggiInit failed with %i\n", err);

	vis = ggiOpen(NULL);
	printassert(vis != NULL, "ggiOpen() failed\n");

	err = ggiCheckSimpleMode(vis, GGI_AUTO, GGI_AUTO, 2, GT_AUTO, &mode);
	printassert(err == GGI_OK, "frames are apparently not supported\n");
	if (err != GGI_OK) {
		ggiClose(vis);
		ggiExit();
		printsuccess();
		return;
	}

	err = ggiSetMode(vis, &mode);
	if (err != GGI_OK) {
		printfailure("ggiSetMode: expected return value: GGI_OK\n"
					"actual return value: %i\n", err);
		return;
	}

	ggiClose(vis);
	ggiExit();


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

	err = ggiInit();
	printassert(err == GGI_OK, "ggiInit failed with %i\n", err);

	vis = ggiOpen(NULL);
	printassert(vis != NULL, "ggiOpen() failed\n");

	err = ggiCheckSimpleMode(
		vis, GGI_AUTO, GGI_AUTO, GGI_AUTO, GT_AUTO, &mode);
	printassert(err == GGI_OK, "ggiCheckSimpleMode: can't find a mode\n");
	if(err != GGI_OK) {
		ggiClose(vis);
		ggiExit();
		printsuccess();
		return;
	}

	printassert(mode.size.x != GGI_AUTO && mode.size.y != GGI_AUTO,
		"physical size is apparently not supported\n");
	if(mode.size.x == GGI_AUTO || mode.size.y == GGI_AUTO) {
		ggiClose(vis);
		ggiExit();
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
	ggiClose(vis);
	ggiExit();

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

	err = ggiInit();
	printassert(err == GGI_OK, "ggiInit failed with %i\n", err);

	vis = ggiOpen(NULL);
	printassert(vis != NULL, "ggiOpen() failed\n");

	ggiSetFlags(vis, GGIFLAG_ASYNC);

	/* Get the default mode */
	err = ggiCheckGraphMode (vis, GGI_AUTO, GGI_AUTO, GGI_AUTO, GGI_AUTO,
				GT_AUTO, &sug_mode);
	if (err != GGI_OK) {
		printfailure("ggiCheckGraphMode: No graphic mode available\n");
		ggiClose(vis);
		ggiExit();
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
		ggiClose(vis);
		ggiExit();
	}

	ggiClose(vis);
	ggiExit();

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
	printassert(err == GGI_OK, "ggiInit failed with %i\n", err);

	vis = ggiOpen(NULL);
	printassert(vis != NULL, "ggiOpen() failed\n");

	/* async mode disables mansync if used */
	ggiSetFlags(vis, GGIFLAG_ASYNC);

	/* Get the default mode */
	err = ggiCheckGraphMode (vis, 640, 480, GGI_AUTO, GGI_AUTO,
				GT_AUTO, &mode);
	if (err != GGI_OK) {
		printfailure("ggiCheckGraphMode: #1: No 640x480 mode available\n");
		ggiClose(vis);
		ggiExit();
		return;
	}

	err = ggiSetMode(vis, &mode);
	if (err != GGI_OK) {
		printfailure("ggiSetMode() #1: failed although ggiCheckGraphMode() was OK!\n");
		ggiClose(vis);
		ggiExit();
		return;
	}


	err = ggiCheckGraphMode(vis, 320, 200, GGI_AUTO, GGI_AUTO,
				GT_AUTO, &mode);
	if (err != GGI_OK) {
		printfailure("ggiCheckGraphMode: #2: No 320x200 mode available\n");
		ggiClose(vis);
		ggiExit();
		return;
	}

	err = ggiSetMode(vis, &mode);
	if (err != GGI_OK) {
		printfailure("ggiSetMode() #2: resetting a mode failed although ggiCheckGraphMode() was OK!\n");
		ggiClose(vis);
		ggiExit();
		return;
	}


	ggiClose(vis);
	ggiExit();

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

	err = ggiInit();
	printassert(err == GGI_OK, "ggiInit failed with %i\n", err);

	vis = ggiOpen(NULL);
	printassert(vis != NULL, "ggiOpen() failed\n");

	/* sync mode enables mansync if used */

	/* Get the default mode */
	err = ggiCheckGraphMode (vis, 640, 480, GGI_AUTO, GGI_AUTO,
				GT_AUTO, &mode);
	if (err != GGI_OK) {
		printfailure("ggiCheckGraphMode: #1: No 640x480 mode available\n");
		ggiClose(vis);
		ggiExit();
		return;
	}

	err = ggiSetMode(vis, &mode);
	if (err != GGI_OK) {
		printfailure("ggiSetMode() #1: failed although ggiCheckGraphMode() was OK!\n");
		ggiClose(vis);
		ggiExit();
		return;
	}


	err = ggiCheckGraphMode(vis, 320, 200, GGI_AUTO, GGI_AUTO,
				GT_AUTO, &mode);
	if (err != GGI_OK) {
		printfailure("ggiCheckGraphMode: #2: No 320x200 mode available\n");
		ggiClose(vis);
		ggiExit();
		return;
	}

	err = ggiSetMode(vis, &mode);
	if (err != GGI_OK) {
		printfailure("ggiSetMode() #2: resetting a mode failed although ggiCheckGraphMode() was OK!\n");
		ggiClose(vis);
		ggiExit();
		return;
	}


	ggiClose(vis);
	ggiExit();

	printsuccess();
	return;
}


int main(int argc, char * const argv[])
{
	parseopts(argc, argv);
	printdesc("Regression testsuite mode handling\n\n");

	testcase1("Check that ggiCheckMode() doesn't return GGI_AUTO");
	testcase2("Check that ggiSetMode() can actually set the mode that has been suggested by ggiCheckMode");
	testcase3("Check setting a mode with a given number of frames");
	testcase4("Check setting a mode by it's physical size");
	testcase5("Set up the mode in the ggiterm way");
	testcase6("Check that re-setting of a different mode works [async mode]");
	testcase7("Check that re-setting of a different mode works [sync mode]");

	printsummary();

	return 0;
}
