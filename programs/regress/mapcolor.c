/* $Id: mapcolor.c,v 1.3 2006/03/28 07:21:22 pekberg Exp $
******************************************************************************

   This is a regression-test for color (un)mapping

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
	ggi_color white_color, black_color,
		red_color, green_color, blue_color;
	ggi_color white_color2, black_color2,
		red_color2, green_color2, blue_color2;

	ggi_pixel white_pixel, black_pixel,
		red_pixel, green_pixel, blue_pixel;


	white_color.r = white_color.g = white_color.b = 0xFFFF;
	black_color.r = black_color.g = black_color.b = 0x0000;
	red_color.r = 0xFFFF; red_color.g = red_color.b = 0x0000;
	green_color.g = 0xFFFF; green_color.r = green_color.b = 0x0000;
	blue_color.b = 0xFFFF; blue_color.r = blue_color.g = 0x0000;


	printteststart(__FILE__, __PRETTY_FUNCTION__, EXPECTED2PASS, desc);
	if (dontrun) return;

	err = ggiInit();
	printassert(err == GGI_OK, "ggiInit failed with %i\n", err);

	vis = ggNewStem();
	printassert(vis != NULL, "ggNewStem failed\n");

	err = ggiAttach(vis);
	printassert(err == GGI_OK, "ggiAttach failed with %i\n", err);

	err = ggiOpen(vis, NULL);
	printassert(err == GGI_OK, "ggiOpen() failed with %i\n", err);

	err = ggiCheckSimpleMode(vis, GGI_AUTO, GGI_AUTO, GGI_AUTO, GT_AUTO, &mode);

	err = ggiSetMode(vis, &mode);
	printassert(err == GGI_OK, "mode setting failed\n");

	ggiSetColorfulPalette(vis);

	white_pixel = ggiMapColor(vis, &white_color);
	black_pixel = ggiMapColor(vis, &black_color);
	red_pixel = ggiMapColor(vis, &red_color);
	green_pixel = ggiMapColor(vis, &green_color);
	blue_pixel = ggiMapColor(vis, &blue_color);

	ggiUnmapPixel(vis, white_pixel, &white_color2);
	ggiUnmapPixel(vis, black_pixel, &black_color2);
	ggiUnmapPixel(vis, red_pixel, &red_color2);
	ggiUnmapPixel(vis, green_pixel, &green_color2);
	ggiUnmapPixel(vis, blue_pixel, &blue_color2);

	ggDelStem(vis);
	ggiExit();


	fprintf(stderr, "white_color: %X,%X,%X  , white_pixel: %X , white_color2: %X,%X,%X\n",
		white_color.r, white_color.g, white_color.b, white_pixel,
		white_color2.r, white_color2.g, white_color2.b);
	fprintf(stderr, "black_color: %X,%X,%X  , black_pixel: %X , black_color2: %X,%X,%X\n",
		black_color.r, black_color.g, black_color.b, black_pixel,
		black_color2.r, black_color2.g, black_color2.b);

	fprintf(stderr, "red_color: %X,%X,%X  , red_pixel: %X , red_color2: %X,%X,%X\n",
		red_color.r, red_color.g, red_color.b, red_pixel,
		red_color2.r, red_color2.g, red_color2.b);
	fprintf(stderr, "green_color: %X,%X,%X  , green_pixel: %X , green_color2: %X,%X,%X\n",
		green_color.r, green_color.g, green_color.b, green_pixel,
		green_color2.r, green_color2.g, green_color2.b);
	fprintf(stderr, "blue_color: %X,%X,%X  , blue_pixel: %X , blue_color2: %X,%X,%X\n",
		blue_color.r, blue_color.g, blue_color.b, blue_pixel,
		blue_color2.r, blue_color2.g, blue_color2.b);

	/* white color */
	if (white_color.r != white_color2.r) {
		printfailure("white_color.r: expected value: %X\n"
			"actual value: %X\n", white_color.r, white_color2.r);
		return;
	}
	if (white_color.g != white_color2.g) {
		printfailure("white_color.g: expected value: %X\n"
			"actual value: %X\n", white_color.g, white_color2.g);
		return;
	}
	if (white_color.b != white_color2.b) {
		printfailure("white_color.b: expected value: %X\n"
			"actual value: %X\n", white_color.b, white_color2.b);
		return;
	}

	/* black color */
	if (black_color.r != black_color2.r) {
		printfailure("black_color.r: expected value: %X\n"
			"actual value: %X\n", black_color.r, black_color2.r);
		return;
	}
	if (black_color.g != black_color2.g) {
		printfailure("black_color.g: expected value: %X\n"
			"actual value: %X\n", black_color.g, black_color2.g);
		return;
	}
	if (black_color.b != black_color2.b) {
		printfailure("black_color.b: expected value: %X\n"
			"actual value: %X\n", black_color.b, black_color2.b);
		return;
	}


	/* red color */
	if (red_color.r != red_color2.r) {
		printfailure("red_color.r: expected value: %X\n"
			"actual value: %X\n", red_color.r, red_color2.r);
		return;
	}
	if (red_color.g != red_color2.g) {
		printfailure("red_color.g: expected value: %X\n"
			"actual value: %X\n", red_color.g, red_color2.g);
		return;
	}
	if (red_color.b != red_color2.b) {
		printfailure("red_color.b: expected value: %X\n"
			"actual value: %X\n", red_color.b, red_color2.b);
		return;
	}


	/* green color */
	if (green_color.r != green_color2.r) {
		printfailure("green_color.r: expected value: %X\n"
			"actual value: %X\n", green_color.r, green_color2.r);
		return;
	}
	if (green_color.g != green_color2.g) {
		printfailure("green_color.g: expected value: %X\n"
			"actual value: %X\n", green_color.g, green_color2.g);
		return;
	}
	if (green_color.b != green_color2.b) {
		printfailure("green_color.b: expected value: %X\n"
			"actual value: %X\n", green_color.b, green_color2.b);
		return;
	}


	/* blue color */
	if (blue_color.r != blue_color2.r) {
		printfailure("blue_color.r: expected value: %X\n"
			"actual value: %X\n", blue_color.r, blue_color2.r);
		return;
	}
	if (blue_color.g != blue_color2.g) {
		printfailure("blue_color.g: expected value: %X\n"
			"actual value: %X\n", blue_color.g, blue_color2.g);
		return;
	}
	if (blue_color.b != blue_color2.b) {
		printfailure("blue_color.b: expected value: %X\n"
			"actual value: %X\n", blue_color.b, blue_color2.b);
		return;
	}


	printsuccess();
	return;
}


int main(int argc, char * const argv[])
{
	parseopts(argc, argv);
	printdesc("Regression testsuite color/pixel mapping\n\n");

	testcase1("Check ggiMapColor()/ggiUnmapPixel()");

	printsummary();

	return 0;
}
