/* $Id: input.c,v 1.3 2004/08/08 21:01:55 cegger Exp $
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
#include <ggi/ggi.h>

#include <string.h>

#include "testsuite.inc.c"

ggi_visual_t vis;
ggi_mode mode;
gii_input_t inp;


static int precase(void)
{
	if (dontrun) return 0;

	ggiInit();

	vis = ggiOpen(NULL);

	ggiCheckSimpleMode(vis, GGI_AUTO, GGI_AUTO, GGI_AUTO, GT_AUTO, &mode);

	ggiSetMode(vis, &mode);

	return 0;
}

static int postcase(void)
{
	if (dontrun) return 0;

	ggiClose(vis);
	ggiExit();

	return 0;
}


static void testcase1(const char *desc)
{
	printteststart(__FILE__, __PRETTY_FUNCTION__, EXPECTED2PASS, desc);
	if (dontrun) return;

	inp = ggiDetachInput(vis);


	if (inp == NULL) {
		printfailure("expected return value: \"non-NULL\"\n"
			"actual return value: \"NULL\"\n");
		return;
	}


	printsuccess();
	return;
}


static void testcase2(const char *desc)
{
	printteststart(__FILE__, __PRETTY_FUNCTION__, EXPECTED2PASS, desc);
	if (dontrun) return;

	inp = ggiJoinInputs(vis, inp);

	if (inp == NULL) {
		printfailure("expected return value: \"non-NULL\"\n"
			"actual return value: \"NULL\"\n");
		return;
	}

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

	testcase1("Check that input detaches from visual correctly.");
	testcase2("Check that input attaches to visual correctly.");

	rc = postcase();
	if (rc != 0) exit(rc);

	printsummary();

	return 0;
}
