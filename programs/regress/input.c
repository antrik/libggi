/* $Id: input.c,v 1.1 2004/08/08 19:50:06 cegger Exp $
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


static void testcase1(const char *desc)
{
	printteststart(__FILE__, __PRETTY_FUNCTION__, EXPECTED2PASS, desc);

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
	parseopts(argc, argv);
	printdesc("Regression testsuite visual <-> input association\n\n");

	ggiInit();

	vis = ggiOpen(NULL);

	ggiCheckSimpleMode(vis, GGI_AUTO, GGI_AUTO, GGI_AUTO, GT_AUTO, &mode);

	ggiSetMode(vis, &mode);

	testcase1("Check that input detaches from visual correctly.");
	testcase2("Check that input attaches to visual correctly.");

	ggiClose(vis);
	ggiExit();

	printsummary();

	return 0;
}
