/* $Id: mode.c,v 1.7 2004/09/21 06:03:16 pekberg Exp $
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

	vis = ggiOpen("memory", NULL);
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


int main(int argc, char * const argv[])
{
	parseopts(argc, argv);
	printdesc("Regression testsuite mode handling\n\n");

	testcase1("Check that ggiCheckMode() doesn't return GGI_AUTO");
	testcase2("Check that ggiSetMode() can actually set the mode that has been suggested by ggiCheckMode");
	testcase3("Check setting a mode with a given number of frames");
	testcase4("Check setting a mode by it's physical size");

	printsummary();

	return 0;
}
