/* $Id: pixfmt.c,v 1.9 2007/03/14 23:01:18 cegger Exp $
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
#include <ggi/internal/ggi.h>
#include <ggi/ggi.h>

#include <string.h>

#include "testsuite.inc.c"


static void pixfmt_parse_check(const char *desc, const char *pixfmt,
			uint32_t pixfmt_flags_expect)
{
	ggi_pixel r_mask, g_mask, b_mask, a_mask;
	uint32_t pixfmt_flags;
	int ret;

	ggi_pixel r_mask_expect = 0xF800;
	ggi_pixel g_mask_expect = 0x07E0;
	ggi_pixel b_mask_expect = 0x001F;
	ggi_pixel a_mask_expect = 0x0000;
	int ret_expect = GGI_OK;

	printteststart(__FILE__, __PRETTY_FUNCTION__, EXPECTED2PASS, desc);
	if (dontrun) return;

	ret = _ggi_parse_pixfmtstr(pixfmt, '\0', NULL,
				strlen(pixfmt)+1,
				&r_mask, &g_mask, &b_mask, &a_mask,
				&pixfmt_flags);

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
	if (pixfmt_flags != pixfmt_flags_expect) {
		printfailure("expected pixfmt_flags value: \"%X\"\n"
			"actual pixfmt_flags value: \"%X\"\n",
			pixfmt_flags_expect, pixfmt_flags);
	}

	printsuccess();
	return;
}

static void testcase1(const char *desc)
{
	pixfmt_parse_check(desc, "r5g6b5", 0);
}

static void testcase2(const char *desc)
{
	pixfmt_parse_check(desc, "r5g6b5[R]", GGI_PF_REVERSE_ENDIAN);
}

static void testcase3(const char *desc)
{
	pixfmt_parse_check(desc, "r5g6b5[H,ham,E]",
		GGI_PF_HIGHBIT_RIGHT | GGI_PF_HAM | GGI_PF_EXTENDED);
}


int main(int argc, char * const argv[])
{
	parseopts(argc, argv);
	printdesc("Regression testsuite for _ggi_parse_pixfmtstr().\n\n");

	testcase1("Check for correct parsing of \"r5g6b5\" pixfmt string.");
	testcase2("Check for correct parsing of \"r5g6b5[R]\" pixfmt string.");
	testcase3("Check for correct parsing of \"r5g6b5[H,ham,E]\" pixfmt string");

	printsummary();

	return 0;
}
