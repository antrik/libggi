/* $Id: init.c,v 1.4 2004/09/10 16:38:47 cegger Exp $
******************************************************************************

   This is a regression-test for init/exit handling.

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

	printteststart(__FILE__, __PRETTY_FUNCTION__, EXPECTED2PASS, desc);
	if (dontrun) return;

	err = ggiInit();
	if (err != GGI_OK) {
		printfailure("expected return value: 0\n"
			"actual return value: %i\n", err);
		return;
	}

	err = ggiInit();
	if (err != GGI_OK) {
		printfailure("expected return value: 0\n"
			"actual return value: %i\n", err);
		return;
	}

	printsuccess();
	return;
}


static void testcase2(const char *desc)
{
	int err;

	printteststart(__FILE__, __PRETTY_FUNCTION__, EXPECTED2PASS, desc);
	if (dontrun) return;

	err = ggiExit();
	if (err != 1) {
		printfailure("expected return value: 1\n"
			"actual return value: %i\n", err);
		return;
	}

	err = ggiExit();
	if (err != 0) {
		printfailure("expected return value: 0\n"
			"actual return value: %i\n", err);
		return;
	}

	printsuccess();
	return;
}


static void testcase3(const char *desc)
{
	int err;
	ggi_visual_t vis;

	printteststart(__FILE__, __PRETTY_FUNCTION__, EXPECTED2PASS, desc);
	if (dontrun) return;

	err = ggiInit();
	if (err != GGI_OK) {
		printfailure("ggiInit: expected return value: 0\n"
			"actual return value: %i\n", err);
		return;
	}

	vis = ggiOpen(NULL);
	if (vis == NULL) {
		printfailure("ggiOpen: Couldn\'t open default visual.");
		return;
	}

	err = ggiClose(vis);
	if (err != GGI_OK) {
		printfailure("ggiClose: expected return value: %i\n"
			"actual return value: %i\n",
			GGI_OK, err);
		return;
	}

	err = ggiExit();
	if (err != 0) {
		printfailure("ggiExit: expected return value: 0\n"
			"actual return value: %i\n", err);
		return;
	}

	printsuccess();
	return;
}

static void testcase4(const char *desc)
{
	int err;

	printteststart(__FILE__, __PRETTY_FUNCTION__, EXPECTED2PASS, desc);
	if (dontrun) return;

	err = ggiClose(NULL);
	if (err != GGI_EARGINVAL) {
		printfailure("ggiClose: expected return value: %i\n"
			"actual return value: %i\n",
			GGI_EARGINVAL, err);
		return;
	}

	printsuccess();
	return;
}

static void testcase5(const char *desc)
{
	int err;
	ggi_visual_t vis;

	printteststart(__FILE__, __PRETTY_FUNCTION__, EXPECTED2PASS, desc);
	if (dontrun) return;

	err = ggiInit();
	if (err != GGI_OK) {
		printfailure("ggiInit: expected return value: 0\n"
			"actual return value: %i\n", err);
		return;
	}


	vis = ggiOpen(NULL);
	if (vis == NULL) {
		printfailure("ggiOpen: Attempt 1: Couldn\'t open default visual.");
		return;
	}

	err = ggiClose(vis);
	if (err != GGI_OK) {
		printfailure("ggiClose: Attempt 1: expected return value: %i\n"
			"actual return value: %i\n",
			GGI_OK, err);
		return;
	}


	vis = ggiOpen(NULL);
	if (vis == NULL) {
		printfailure("ggiOpen: Attempt 2: Couldn\'t open default visual again.");
		return;
	}

	err = ggiClose(vis);
	if (err != GGI_OK) {
		printfailure("ggiClose: Attempt 2: expected return value: %i\n"
			"actual return value: %i\n",
			GGI_OK, err);
		return;
	}


	err = ggiExit();
	if (err != 0) {
		printfailure("ggiExit: expected return value: 0\n"
			"actual return value: %i\n", err);
		return;
	}

	printsuccess();
	return;
}


static void testcase6(const char *desc)
{
	int err;
	ggi_visual_t vis, vis2;

	printteststart(__FILE__, __PRETTY_FUNCTION__, EXPECTED2PASS, desc);
	if (dontrun) return;

	err = ggiInit();
	if (err != GGI_OK) {
		printfailure("ggiInit: expected return value: 0\n"
			"actual return value: %i\n", err);
		return;
	}

	vis2 = ggiOpen(NULL);
	if (vis2 == NULL) {
		printfailure("ggiOpen: visual 2: Couldn\'t open default visual again.");
		return;
	}


	vis = ggiOpen(NULL);
	if (vis == NULL) {
		printfailure("ggiOpen: visual 1: Attempt 1: Couldn\'t open default visual.");
		return;
	}

	err = ggiClose(vis);
	if (err != GGI_OK) {
		printfailure("ggiClose: visual 1: Attempt 1: expected return value: %i\n"
			"actual return value: %i\n",
			GGI_OK, err);
		return;
	}


	vis = ggiOpen(NULL);
	if (vis == NULL) {
		printfailure("ggiOpen: visual 1: Attempt 2: Couldn\'t open default visual.");
		return;
	}

	err = ggiClose(vis);
	if (err != GGI_OK) {
		printfailure("ggiClose: visual 1: Attempt 2: expected return value: %i\n"
			"actual return value: %i\n",
			GGI_OK, err);
		return;
	}


	err = ggiClose(vis2);
	if (err != GGI_OK) {
		printfailure("ggiClose: visual 2: expected return value: %i\n"
			"actual return value: %i\n",
			GGI_OK, err);
		return;
	}


	err = ggiExit();
	if (err != 0) {
		printfailure("ggiExit: expected return value: 0\n"
			"actual return value: %i\n", err);
		return;
	}


	printsuccess();
	return;
}




int main(int argc, char * const argv[])
{
	parseopts(argc, argv);
	printdesc("Regression testsuite libggi init/exit handling\n\n");

	testcase1("Check that ggiInit() behaves as documented.");
	testcase2("Check that ggiExit() behaves as documented.");
	testcase3("Check ggiOpen()/ggiClose() to open/close a default visual.");
	testcase4("Check ggiClose() works correct with an invalid argument.");
	testcase5("ggiOpen()/ggiClose() a default visual twice times using the same visual variable.");
	testcase6("As testcase5, but kind of nested in a second visual");

	printsummary();

	return 0;
}
