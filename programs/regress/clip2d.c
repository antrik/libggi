/* $Id: clip2d.c,v 1.3 2004/05/04 10:23:24 cegger Exp $
******************************************************************************

   This is a regression-test and for LibGGI clipping operations.

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
#include <limits.h>

#include "../../default/common/clip.c"

#include "common.inc.c"


static ggi_visual_t vis;


static void testcase1(void)
{
	int x0, x1, y0, y1, clip_first, clip_last;
	int ret;

	int x0_expect = 50;
	int y0_expect = 50;
	int x1_expect = 52;
	int y1_expect = 52;
	int clip_first_expect = 0;
	int clip_last_expect = 0;
	int ret_expect = 1;

	x0 = 50;
	y0 = 50;
	x1 = 52;
	y1 = 52;


	printteststart(__PRETTY_FUNCTION__);

	ret = _ggi_clip2d(vis, &x0, &y0, &x1, &y1,
			&clip_first, &clip_last);

	if (ret != ret_expect) {
		printfailure("expected return value: \"%i\"\n"
			"actual return value: \"%i\"\n",
			ret_expect, ret);
		return;
	}


	if (x0 != x0_expect) {
		printfailure("expected x0 value: \"%i\"\n"
			"actual x0 value: \"%i\"\n",
			x0_expect, x0);
		return;
	}
	if (x1 != x1_expect) {
		printfailure("expected x1 value: \"%i\"\n"
			"actual x1 value: \"%i\"\n",
			x1_expect, x1);
		return;
	}
	if (y0 != y0_expect) {
		printfailure("expected y0 value: \"%i\"\n"
			"actual y0 value: \"%i\"\n",
			y0_expect, y0);
		return;
	}
	if (y1 != y1_expect) {
		printfailure("expected y1 value: \"%i\"\n"
			"actual y1 value: \"%i\"\n",
			y1_expect, y1);
		return;
	}
	if (clip_first != clip_first_expect) {
		printfailure("expected clip_first value: \"%i\"\n"
			"actual clip_first value: \"%i\"\n",
			clip_first_expect, clip_first);
		return;
	}
	if (clip_last != clip_last_expect) {
		printfailure("expected clip_last value: \"%i\"\n"
			"actual clip_last value: \"%i\"\n",
			clip_last_expect, clip_last);
		return;
	}

	printsuccess();
	return;
}


static void testcase2(void)
{
	int x0, x1, y0, y1, clip_first, clip_last;
	int ret;

	int x0_expect = 50;
	int y0_expect = 50;
	int x1_expect = 52;
	int y1_expect = 52;
	int clip_first_expect = 0;
	int clip_last_expect = 0;
	int ret_expect = 1;

	x0 = 50;
	y0 = 50;
	x1 = 52;
	y1 = INT_MIN;


	printteststart(__PRETTY_FUNCTION__);

	ret = _ggi_clip2d(vis, &x0, &y0, &x1, &y1,
			&clip_first, &clip_last);

	if (ret != ret_expect) {
		printfailure("expected return value: \"%i\"\n"
			"actual return value: \"%i\"\n",
			ret_expect, ret);
		return;
	}


	if (x0 != x0_expect) {
		printfailure("expected x0 value: \"%i\"\n"
			"actual x0 value: \"%i\"\n",
			x0_expect, x0);
		return;
	}
	if (x1 != x1_expect) {
		printfailure("expected x1 value: \"%i\"\n"
			"actual x1 value: \"%i\"\n",
			x1_expect, x1);
		return;
	}
	if (y0 != y0_expect) {
		printfailure("expected y0 value: \"%i\"\n"
			"actual y0 value: \"%i\"\n",
			y0_expect, y0);
		return;
	}
	if (y1 != y1_expect) {
		printfailure("expected y1 value: \"%i\"\n"
			"actual y1 value: \"%i\"\n",
			y1_expect, y1);
		return;
	}
	if (clip_first != clip_first_expect) {
		printfailure("expected clip_first value: \"%i\"\n"
			"actual clip_first value: \"%i\"\n",
			clip_first_expect, clip_first);
		return;
	}
	if (clip_last != clip_last_expect) {
		printfailure("expected clip_last value: \"%i\"\n"
			"actual clip_last value: \"%i\"\n",
			clip_last_expect, clip_last);
		return;
	}

	printsuccess();
	return;
}



int main(void)
{
	int rc;
	ggi_mode tm;

	rc = ggiInit();
	if (rc < 0) return 1;
	vis = ggiOpen("display-memory", NULL);
	if (!vis) return 1;

	rc = ggiCheckSimpleMode(vis, 640, 480, 1, GT_AUTO, &tm);
	rc = ggiSetMode(vis, &tm);
	if (rc < 0) ggiPanic("Mode initialization failed.");

	/* run tests */
	testcase1();
	testcase2();

	rc = ggiClose(vis);

	printsummary();

	return 0;
}
