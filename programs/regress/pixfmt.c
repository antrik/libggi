/* $Id: pixfmt.c,v 1.5 2004/07/28 09:25:20 cegger Exp $
******************************************************************************

   This is a regression-test for LibGGI pixelformat operations.

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
#include <ggi/ggi.h>

#include <string.h>

#include "testsuite.inc.c"


static void testcase1(const char *desc)
{
	const char *pixfmt="r5g6b5";

	ggi_pixel r_mask, g_mask, b_mask, a_mask;
	int ret;

	ggi_pixel r_mask_expect = 0xF800;
	ggi_pixel g_mask_expect = 0x07E0;
	ggi_pixel b_mask_expect = 0x001F;
	ggi_pixel a_mask_expect = 0x0000;
	int ret_expect = GGI_OK;

	printteststart(__FILE__, __PRETTY_FUNCTION__, EXPECTED2PASS, desc);

	ret = _ggi_parse_pixfmtstr(pixfmt, NULL, NULL,
				strlen(pixfmt)+1,
				&r_mask, &g_mask, &b_mask, &a_mask);

	if (ret != ret_expect) {
		printfailure("expected return value: \"%i\"\n"
			"actual return value: \"%i\"\n",
			ret_expect, ret);
		return;
	}


	if (r_mask != r_mask_expect) {
		printfailure("expected r_mask value: \"%X\"\n"
			"actual r_mask value: \"%X\"\n",
			r_mask_expect, r_mask);
		return;
	}
	if (g_mask != g_mask_expect) {
		printfailure("expected g_mask value: \"%X\"\n"
			"actual g_mask value: \"%X\"\n",
			g_mask_expect, g_mask);
		return;
	}
	if (b_mask != b_mask_expect) {
		printfailure("expected b_mask value: \"%X\"\n"
			"actual b_mask value: \"%X\"\n",
			b_mask_expect, b_mask);
		return;
	}
	if (a_mask != a_mask_expect) {
		printfailure("expected a_mask value: \"%X\"\n"
			"actual a_mask value: \"%X\"\n",
			a_mask_expect, a_mask);
		return;
	}

	printsuccess();
	return;
}


int main(int argc, char * const argv[])
{
	parseopts(argc, argv);
	printdesc("Regression testsuite for _ggi_parse_pixfmtstr().\n\n");

	testcase1("Check for correct parsing of \"r5g6b5\" pixfmt string.");

	printsummary();

	return 0;
}
