/* $Id: clip2d.c,v 1.13 2004/05/27 11:30:11 pekberg Exp $
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

#include "common.inc.c"
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



static void testcase1(void)
{
	int x0 = 50;
	int y0 = 50;
	int x1 = 52;
	int y1 = 52;

	int x0_expect = 50;
	int y0_expect = 50;
	int x1_expect = 52;
	int y1_expect = 52;
	int ret_expect = 1;


	printteststart(__FILE__, __PRETTY_FUNCTION__, EXPECTED2PASS);

	checkresult(x0, y0, x1, y1,
		x0_expect, y0_expect, x1_expect, y1_expect,
		ret_expect, 1);
}


static void testcase2(void)
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


	printteststart(__FILE__, __PRETTY_FUNCTION__, EXPECTED2FAIL);

	checkresult(x0, y0, x1, y1,
		x0_expect, y0_expect, x1_expect, y1_expect,
		ret_expect, 1);
}


static void testcase3(void)
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


	printteststart(__FILE__, __PRETTY_FUNCTION__, EXPECTED2FAIL);

	checkresult(x0, y0, x1, y1,
		x0_expect, y0_expect, x1_expect, y1_expect,
		ret_expect, 1);
}


static void testcase4(void)
{
	/* Tests longest possible diagonal line that succeeds, I think */
	/* delta will be 32768 on 32 bit arches. */
	int delta = (INT_MAX >> sizeof(int)*4) + 1;

	printteststart(__FILE__, __PRETTY_FUNCTION__, EXPECTED2PASS);

	checkresult(
		MODE_SIZE_X - 1 + delta, MODE_SIZE_Y - 1 + delta,
		MODE_SIZE_X - 1,         MODE_SIZE_Y - 1,
		MODE_SIZE_X - 1,         MODE_SIZE_Y - 1,
		MODE_SIZE_X - 1,         MODE_SIZE_Y - 1,
		1, 1);
}


static void testcase5(void)
{
	/* Tests shortest possible line that fails, I think */
	/* delta will be 32768 on 32 bit arches. */
	int delta = (INT_MAX >> sizeof(int)*4) + 1;

	printteststart(__FILE__, __PRETTY_FUNCTION__, EXPECTED2FAIL);

	checkresult(
		MODE_SIZE_X - 1 + delta, MODE_SIZE_Y - 1 + delta + 1,
		MODE_SIZE_X - 1,         MODE_SIZE_Y - 1,
		MODE_SIZE_X - 1,         MODE_SIZE_Y - 1,
		MODE_SIZE_X - 1,         MODE_SIZE_Y - 1,
		1, 1);
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

static void testcase6(void)
{
	int i;

	printteststart(__FILE__, __PRETTY_FUNCTION__, EXPECTED2PASS);

	for(i = 0; i < CLIPDBSIZE; ++i)
		if(checkresult(
			db[i][0], db[i][1], db[i][2], db[i][3],
			db[i][4], db[i][5], db[i][6], db[i][7],
			db[i][8], 0))
			return;

	printsuccess();
}



int main(void)
{
	int rc;
	ggi_mode tm;

	rc = ggiInit();
	if (rc < 0) return 1;
	vis = ggiOpen("display-memory", NULL);
	if (!vis) return 1;

	rc = ggiCheckSimpleMode(vis, MODE_SIZE_X, MODE_SIZE_Y, 1, GT_AUTO, &tm);
	rc = ggiSetMode(vis, &tm);
	if (rc < 0) ggiPanic("Mode initialization failed.");

	/* run tests */
	testcase1();
	testcase2();
	testcase3();
	testcase4();
	testcase5();
	testcase6();

	rc = ggiClose(vis);

	printsummary();

	return 0;
}
