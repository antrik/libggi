/* $Id: mode.c,v 1.3 2004/09/20 12:42:27 pekberg Exp $
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

	printteststart(__FILE__, __PRETTY_FUNCTION__, EXPECTED2PASS, desc);
	if (dontrun) return;

	err = ggiInit();
	printassert(err == GGI_OK, "ggiInit failed with %i\n", err);

	vis = ggiOpen(NULL);
	printassert(vis != NULL, "ggiOpen() failed\n");

	mode.frames    = GGI_AUTO;
	mode.visible.x = GGI_AUTO;
	mode.visible.y = GGI_AUTO;
	mode.virt.x    = GGI_AUTO;
	mode.virt.y    = GGI_AUTO;
	mode.size.x    = 40;
	mode.size.y    = 30;
	mode.graphtype = GT_AUTO;
	mode.dpp.x     = mode.dpp.y = GGI_AUTO;

	err = ggiCheckMode(vis, &mode);

	if (mode.size.x == GGI_AUTO) {
		printfailure(
			"ggiCheckMode: expected return value: not GGI_AUTO\n"
					"actual return value: GGI_AUTO\n");
		ggiClose(vis);
		ggiExit();
		return;
	}

	if (mode.size.y == GGI_AUTO) {
		printfailure(
			"ggiCheckMode: expected return value: not GGI_AUTO\n"
					"actual return value: GGI_AUTO\n");
		ggiClose(vis);
		ggiExit();
		return;
	}

	ggiClose(vis);
	ggiExit();
	
	if (err == GGI_OK || err == GGI_ENOMATCH) {
		printsuccess();
		return;
	}

	printfailure(
		"ggiSetMode: expected return value: GGI_OK or GGI_ENOMATCH\n"
				"actual return value: %i\n", err);
	return;
}


int main(int argc, char * const argv[])
{
	parseopts(argc, argv);
	printdesc("Regression testsuite mode handling\n\n");

	testcase1("Check that ggiCheckMode() doesn't return GGI_AUTO");
	testcase2("Check that ggiSetMode() can actually set the mode that has been suggested by ggiCheckMode");
	testcase3("Check setting a mode with a given number of frames");
	testcase4("Check setting a 40x30 mm mode");

	printsummary();

	return 0;
}
