/* $Id: clip2d.c,v 1.25 2004/08/27 09:25:42 pekberg Exp $
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
#ifdef HAVE_LIMITS_H
#include <limits.h>
#endif

#include "../../default/common/clip.c"

#include "testsuite.inc.c"
#include "clipdb.inc.c"

#define MIN(a, b)	(a < b) ? (a) : (b)
#define MAX(a, b)	(a > b) ? (a) : (b)

#define MODE_SIZE_X	640
#define MODE_SIZE_Y	480


static ggi_visual_t vis;


static int checkresult(int x0, int y0, int x1, int y1,
			int x0_expect, int y0_expect,
			int x1_expect, int y1_expect,
			int ret_expect, int finish)
{
	int clip_first = 0;
	int clip_last = 0;
	int clip_first_expect = (x0 != x0_expect) || (y0 != y0_expect);
	int clip_last_expect  = (x1 != x1_expect) || (y1 != y1_expect);
	int ret;

	ret = _ggi_clip2d(vis, &x0, &y0, &x1, &y1,
			&clip_first, &clip_last);

	if (ret != ret_expect) {
		printfailure("expected return value: \"%i\"\n"
			"actual return value: \"%i\"\n",
			ret_expect, ret);
		return -1;
	}
	if (ret == 0)
		goto success;

	if (x0 != x0_expect) {
		printfailure("expected x0 value: \"%i\"\n"
			"actual x0 value: \"%i\"\n",
			x0_expect, x0);
		return -1;
	}
	if (x1 != x1_expect) {
		printfailure("expected x1 value: \"%i\"\n"
			"actual x1 value: \"%i\"\n",
			x1_expect, x1);
		return -1;
	}
	if (y0 != y0_expect) {
		printfailure("expected y0 value: \"%i\"\n"
			"actual y0 value: \"%i\"\n",
			y0_expect, y0);
		return -1;
	}
	if (y1 != y1_expect) {
		printfailure("expected y1 value: \"%i\"\n"
			"actual y1 value: \"%i\"\n",
			y1_expect, y1);
		return -1;
	}
	if (clip_first != clip_first_expect) {
		printfailure("expected clip_first value: \"%i\"\n"
			"actual clip_first value: \"%i\"\n",
			clip_first_expect, clip_first);
		return -1;
	}
	if (clip_last != clip_last_expect) {
		printfailure("expected clip_last value: \"%i\"\n"
			"actual clip_last value: \"%i\"\n",
			clip_last_expect, clip_last);
		return -1;
	}

success:
	if(finish)
		printsuccess();
	return 0;
}



static void testcase1(const char *desc)
{
	int i;

	printteststart(__FILE__, __PRETTY_FUNCTION__, EXPECTED2PASS, desc);
	if (dontrun) return;

	for(i = 0; i < CLIPDBSIZE; ++i)
		if(checkresult(
			db[i][0], db[i][1], db[i][2], db[i][3],
			db[i][4], db[i][5], db[i][6], db[i][7],
			db[i][8], 0))
			return;

	printsuccess();
}


static void testcase2(const char *desc)
{
	int x0 = INT_MIN;
	int y0 = INT_MIN;
	int x1 = INT_MAX;
	int y1 = INT_MAX;

	int x0_expect = 0;
	int y0_expect = 0;
	int x1_expect = MIN(MODE_SIZE_X, MODE_SIZE_Y) - 1;
	int y1_expect = MIN(MODE_SIZE_X, MODE_SIZE_Y) - 1;
	int ret_expect = 1;


	printteststart(__FILE__, __PRETTY_FUNCTION__, EXPECTED2PASS, desc);
	if (dontrun) return;

	checkresult(x0, y0, x1, y1,
		x0_expect, y0_expect, x1_expect, y1_expect,
		ret_expect, 1);
}


static void testcase3(const char *desc)
{
	int x0 = 140000;
	int y0 = 70000;
	int x1 = 0;
	int y1 = 0;

	int x0_expect = MODE_SIZE_X - 1;
	int y0_expect = MODE_SIZE_X / 2;
	int x1_expect = 0;
	int y1_expect = 0;
	int ret_expect = 1;


	printteststart(__FILE__, __PRETTY_FUNCTION__, EXPECTED2PASS, desc);
	if (dontrun) return;

	checkresult(x0, y0, x1, y1,
		x0_expect, y0_expect, x1_expect, y1_expect,
		ret_expect, 1);
}


static void testcase4(const char *desc)
{
	/* Tests longest possible diagonal line that succeeds, I think. */
	/* delta will be 32768 on 32 bit arches. */
	int delta = (INT_MAX >> sizeof(int)*4) + 1;

	printteststart(__FILE__, __PRETTY_FUNCTION__, EXPECTED2PASS, desc);
	if (dontrun) return;

	checkresult(
		MODE_SIZE_X - 1 + delta, MODE_SIZE_Y - 1 + delta,
		MODE_SIZE_X - 1,         MODE_SIZE_Y - 1,
		MODE_SIZE_X - 1,         MODE_SIZE_Y - 1,
		MODE_SIZE_X - 1,         MODE_SIZE_Y - 1,
		1, 1);
}


static void testcase5(const char *desc)
{
	/* Tests shortest possible line that fails, I think. */
	/* delta will be 32768 on 32 bit arches. */
	int delta = (INT_MAX >> sizeof(int)*4) + 1;

	printteststart(__FILE__, __PRETTY_FUNCTION__, EXPECTED2PASS, desc);
	if (dontrun) return;

	checkresult(
		MODE_SIZE_X - 1 + delta, MODE_SIZE_Y - 1 + delta + 1,
		MODE_SIZE_X - 1,         MODE_SIZE_Y - 1,
		MODE_SIZE_X - 1,         MODE_SIZE_Y - 1,
		MODE_SIZE_X - 1,         MODE_SIZE_Y - 1,
		1, 1);
}


static void testcase6(const char *desc)
{
	/* dx == INT_MIN (or INT_MAX+1) generates divide by zero,
	 * and the same goes for dy.
	 */

	printteststart(__FILE__, __PRETTY_FUNCTION__, EXPECTED2PASS, desc);
	if (dontrun) return;

	if(checkresult(
		INT_MIN,          0,
		0,                0,
		0,                0,
		0,                0,
		1, 0)) return;
	if(checkresult(
		0,                0,
		INT_MIN,          0,
		0,                0,
		0,                0,
		1, 0)) return;
	if(checkresult(
		0,                INT_MIN,
		0,                0,
		0,                0,
		0,                0,
		1, 0)) return;
	if(checkresult(
		0,                0,
		0,                INT_MIN,
		0,                0,
		0,                0,
		1, 0)) return;
	if(checkresult(
		-1,               INT_MAX,
		0,                0,
		0,                MODE_SIZE_Y - 1,
		0,                0,
		1, 0)) return;
	if(checkresult(
		0,                0,
		-1,               INT_MAX,
		0,                0,
		0,                MODE_SIZE_Y - 1,
		1, 0)) return;
	if(checkresult(
		INT_MAX,         -1,
		0,                0,
		MODE_SIZE_X - 1,  0,
		0,                0,
		1, 0)) return;
	if(checkresult(
		0,                0,
		INT_MAX,         -1,
		0,                0,
		MODE_SIZE_X - 1,  0,
		1, 0)) return;

	printsuccess();
}


static void testcase7(const char *desc)
{
	/* This line is clipped incorrectly due to overflow. */

	printteststart(__FILE__, __PRETTY_FUNCTION__, EXPECTED2PASS, desc);
	if (dontrun) return;

	checkresult(
		INT_MIN/2 + 200, 10,
		100,              5,
		0,                5,
		100,              5,
		1, 1);
}


#define ENDPOINTS (4)
#define LINES     (4)
#define LENGTH    (98)

static void get_coords(int i, int *x0, int *y0, int *x1, int *y1)
{
	int endpoint = i/(LENGTH*LINES);
	int line = (i-endpoint*LENGTH*LINES) / LENGTH;
	int point = i % LENGTH;

	switch(endpoint) {
	case 0: *x1 =  3; *y1 =  3; break;
	case 1: *x1 = 96; *y1 =  3; break;
	case 2: *x1 =  3; *y1 = 96; break;
	case 3: *x1 = 96; *y1 = 96; break;
	}

	switch(line) {
	case 0: *x0 =  1 + point, *y0 =  1;         break;
	case 1: *x0 =  1 + point, *y0 = 98;         break;
	case 2: *x0 =  1,         *y0 =  1 + point; break;
	case 3: *x0 = 98,         *y0 =  1 + point; break;
	}	
}


static int endpoint_checker(int dx, int dy)
{
	int x0, y0, x1, y1, i;
	int x0t, y0t, x1t, y1t;
	int clip_first, clip_last;
	int ret;
	ggi_color whitec = { 0xffff, 0xffff, 0xffff, 0 };
	ggi_color blackc = { 0, 0, 0, 0 };
	ggi_pixel white = ggiMapColor(vis, &whitec);
	ggi_pixel black = ggiMapColor(vis, &blackc);
	ggi_pixel pixel;
	
	ggiSetGCForeground(vis, black);
	ggiFillscreen(vis);

	for(i = 0; i < ENDPOINTS*LINES*LENGTH; ++i) {
		get_coords(i, &x0, &y0, &x1, &y1);
		x0 += dx;  y0 += dy;
		x1 += 100; y1 += 100;
		x0t = x0; y0t = y0; x1t = x1; y1t = y1;
		ggiSetGCClipping(vis, 100, 100, 200, 200);
		ret = _ggi_clip2d(vis, &x0t, &y0t, &x1t, &y1t,
				&clip_first, &clip_last);
		ggiSetGCClipping(vis, 0, 0, MODE_SIZE_X, MODE_SIZE_Y);
		if(!ret)
			continue;

		ggiSetGCForeground(vis, white);
		ggiDrawLine(vis, x0, y0, x1, y1);
		ggiFlush(vis);

		ggiGetPixel(vis, x0t, y0t, &pixel);
		ggiSetGCForeground(vis, black);
		ggiFillscreen(vis);
		if(pixel != white) {
			printfailure("point (%i,%i) expected pixel value 0x%x\n"
				"point (%i,%i) actual pixel value 0x%x\n",
				x0t, y0t, white, x0t, y0t, pixel);
			return -1;
		}
	}

	return 0;
}


static void testcase8(const char *desc)
{
	/* Check if the clipping endpoint is really on the actual line
	 * for a bunch of lines.
	 */
	printteststart(__FILE__, __PRETTY_FUNCTION__, EXPECTED2PASS, desc);
	if (dontrun) return;

	if(endpoint_checker(  0,   0))
		return;
	if(endpoint_checker(100,   0))
		return;
	if(endpoint_checker(200,   0))
		return;
	if(endpoint_checker(  0, 100))
		return;

	if(endpoint_checker(200, 100))
		return;
	if(endpoint_checker(  0, 200))
		return;
	if(endpoint_checker(100, 200))
		return;
	if(endpoint_checker(200, 200))
		return;

	printsuccess();
}


static void testcase9(const char *desc)
{
	printteststart(__FILE__, __PRETTY_FUNCTION__, EXPECTED2PASS, desc);
	if (dontrun) return;

	checkresult(
		MODE_SIZE_X/4, MODE_SIZE_Y/4,
		MODE_SIZE_X/2, MODE_SIZE_Y/2,
		MODE_SIZE_X/4, MODE_SIZE_Y/4,
		MODE_SIZE_X/2, MODE_SIZE_Y/2,
		1, 1);
}


static void testcase10(const char *desc)
{
	printteststart(__FILE__, __PRETTY_FUNCTION__, EXPECTED2PASS, desc);
	if (dontrun) return;

	ggiSetGCClipping(vis, 10, 10, MODE_SIZE_X, MODE_SIZE_Y);
	checkresult(
		INT_MIN+1, INT_MIN,
		10, 10,
		10, 10,
		10, 10,
		1, 1);
	ggiSetGCClipping(vis, 0, 0, MODE_SIZE_X, MODE_SIZE_Y);
}


#if 0
#define DBSIZE (1000)
static void generate_clipdb(void)
{
	int db[DBSIZE][9];
	int i;
	FILE *f=fopen("clipdb.inc.c", "wt");
	srand(time(NULL));
	fprintf(f, "#define CLIPDBSIZE (%d)\n", DBSIZE);
	fprintf(f, "int db[CLIPDBSIZE][9] = {\n");
	for(i = 0; i < DBSIZE; ++i) {
		int clip1, clip2, j;
		db[i][0] = (rand() % (2*MODE_SIZE_X)) - MODE_SIZE_X / 2;
		db[i][1] = (rand() % (2*MODE_SIZE_Y)) - MODE_SIZE_Y / 2;
		db[i][2] = (rand() % (2*MODE_SIZE_X)) - MODE_SIZE_X / 2;
		db[i][3] = (rand() % (2*MODE_SIZE_Y)) - MODE_SIZE_Y / 2;
		db[i][4] = db[i][0];
		db[i][5] = db[i][1];
		db[i][6] = db[i][2];
		db[i][7] = db[i][3];
		db[i][8] = _ggi_clip2d(vis, &db[i][4], &db[i][5], &db[i][6], &db[i][7],
				&clip1, &clip2);
		if(!db[i][8])
			db[i][4] = db[i][5] = db[i][6] = db[i][7] = 0;

		for(j = 0; j < 9; ++j) {
			if(!j)
				fprintf(f, "\t{ ");
			else
				fprintf(f, ",\t");
			fprintf(f, "%d", db[i][j]);
		}
		if(i < DBSIZE - 1)
			fprintf(f, " },\n");
		else
			fprintf(f, " }\n");
	}
	fprintf(f, "};\n");
	fclose(f);
}
#endif


int main(int argc, char * const argv[])
{
	int rc;
	ggi_mode tm;

	parseopts(argc, argv);
	printdesc("Regression testsuite for _ggi_clip2d().\n\n");

	rc = ggiInit();
	if (rc < 0) return 1;
	vis = ggiOpen("display-memory", NULL);
	if (!vis) return 1;

	rc = ggiCheckSimpleMode(vis, MODE_SIZE_X, MODE_SIZE_Y, 1, GT_AUTO, &tm);
	rc = ggiSetMode(vis, &tm);
	if (rc < 0) ggiPanic("Mode initialization failed.");

	if(tm.graphtype & GT_PALETTE)
		ggiSetColorfulPalette(vis);

	/* run tests */
	testcase1("Check correct clipping of some randomized lines.");
	testcase2("Check clipping from upper left corner (INT_MIN/INT_MIN) "
		  "to bottom right (INT_MAX/INT_MAX).");
	testcase3("Check clipping of bottom right (140000/70000) to upper left (0/0).");
	testcase4("Tests longest possible diagonal line that succeeds - "
		  "dx = 32768 and dy = 32768 on 32 bit arches.");
	testcase5("Tests shortest possible line that fails - "
		  "dx = 32768 and dy = 32769 on 32 bit arches.");
	testcase6("dx == INT_MIN (or INT_MAX+1) generates divide by zero. "
		  "The same goes for dy.");
	testcase7("Check if clipping is resistent against overflows.");
	testcase8("Check if the clipping endpoint is really on the actual line "
		  "for a bunch of lines.");
	testcase9("Check algorithm on a line within the clipping area.");
	testcase10("Tests line that fails even if doubling the precision.");

	rc = ggiClose(vis);

	printsummary();

	return 0;
}
