/* $Id: palette.c,v 1.4 2006/09/23 09:05:22 cegger Exp $
******************************************************************************

   This is a test program for palette handling.

   Written in 2004 by Peter Ekberg

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
#include <ggi/errors.h>

#include "../regress/testsuite.inc.c"

static void testcase1(const char *desc)
{
	ggi_visual_t vis;
	ggi_color green, back;
	int err;

	printteststart(__FILE__, __PRETTY_FUNCTION__, EXPECTED2PASS, desc);
	if (dontrun) return;

	err = ggiInit();
	printassert(err == GGI_OK, "ggiInit failed with %i\n", err);

	vis = ggNewStem(NULL);
	printassert(vis != NULL, "ggNewStem() failed\n");

	err = ggiAttach(vis);
	printassert(err >= 0, "ggiAttach() failed\n");

	err = ggiOpen(vis, NULL);
	printassert(err >= 0, "ggiOpen() failed\n");

	err = ggiSetGraphMode(vis, GGI_AUTO, GGI_AUTO, GGI_AUTO, GGI_AUTO, GT_8BIT);
	if(err < 0) {
		printassert(0, "Palettized mode not available.\n");
		printsuccess();
		ggDelStem(vis);
		ggiExit();
		return;
	}

	green.r = 100;
	green.g = 40000;
	green.b = 4000;
	err = ggiSetPalette(vis, GGI_PALETTE_DONTCARE, 1, &green);
	if (err < 0) {
		printfailure("Unable to install colormap with one entry.\n");
		ggDelStem(vis);
		ggiExit();
		return;
	}

	printassert(err == (int)ggiMapColor(vis, &green),
		"ggiMapColor inconsistent with retval of ggiSetPalette.\n");

	ggiUnmapPixel(vis, err, &back);
	if(green.r != back.r || green.g != back.g || green.b != back.b) {
		printfailure("Unexpected color from ggiUnmapPixel.\n");
		ggDelStem(vis);
		ggiExit();
		return;
	}

	ggiSetGCForeground(vis, err);
	ggiDrawBox(vis, 0, 0, 3000, 3000);
	ggiFlush(vis);

	/* You should see a green square, how to test this? */
	ggUSleep(5000000);
	printsuccess();

	ggDelStem(vis);
	ggiExit();
}

int main(int argc, char * const argv[])
{
	parseopts(argc, argv);
	printdesc("Regression testsuite palette handling\n\n");

	testcase1("Draw green box");

	printsummary();

	return 0;
}
