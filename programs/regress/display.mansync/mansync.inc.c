/* $Id: mansync.inc.c,v 1.4 2006/03/17 21:55:43 cegger Exp $
******************************************************************************

   This is a regression-test for correct mansync usage.

   Written in 2004 by Christoph Egger

   This software is placed in the public domain and can be used freely
   for any purpose. It comes without any kind of warranty, either
   expressed or implied, including, but not limited to the implied
   warranties of merchantability or fitness for a particular purpose.
   Use it at your own risk. the author is not responsible for any damage
   or consequences raised by use or inability to use this program.

******************************************************************************
*/


#include <string.h>

#include "../testsuite.inc.c"


static void testcase1(const char *desc)
{
	int ret;
	ggi_visual_t vis;
	ggi_mode mode;
	ggi_pixel pixel_color;
	ggi_color color_color;

#if 0
	unsigned int j, size;
	const ggi_directbuffer *dbuf = NULL;
#endif


	color_color.r = 0x0000;
	color_color.g = 0xffff;
	color_color.b = 0xffff;

	printteststart(__FILE__, __PRETTY_FUNCTION__, EXPECTED2PASS, desc);
	if (dontrun) return;


	vis = ggiOpen(DISPLAYSTR, NULL);
	if (vis == NULL) {
		printfailure("Couldn't open %s", DISPLAYSTR);
		return;
	}

#if 0
	/* Here, XGGI would switch to async mode,
	 * if there were a place to hook in ggiFlush()
	 */
	ggiSetFlags(vis, GGIFLAG_ASYNC);
#endif

	/* Now check, if mansync is actually running
	 * - note, we are in sync mode here.
	 */

	/* stop mansync. If it runs as it should,
	 * we don't fail with an assertion here */
	ret = MANSYNC_stop(vis);
	if (ret != 0) {
		printfailure("BUG: mansync not running - forgot to call "
			"MANSYNC_start(vis) right after MANSYNC_init(vis)?\n");
		goto exit_testcase;
	}

	/* now restart it */
	ret = MANSYNC_start(vis);
	if (ret != 0) {
		printfailure("BUG: mansync couldn't be relaunched\n");
		goto exit_testcase;
	}

	ggiParseMode("", &mode);
	ggiCheckMode(vis, &mode);

	ret = ggiSetMode(vis, &mode);
	if (ret != GGI_OK) {
		printfailure("expected return value: \"%i\"\n"
			"actual return value: \"%i\"\n",
			GGI_OK, ret);
		goto exit_testcase;
	}

	/* Disable directbuffer as this is not required to
	 * perform the test
	 */

#if 0
	ret = -1;
	size = GT_SIZE(mode.graphtype);
	for (j = 0;
		(dbuf = ggiDBGetBuffer(vis, j)) != NULL;
	     j++)
	{
		if ((dbuf->type & GGI_DB_SIMPLE_PLB)
		    && ((8*dbuf->buffer.plb.stride) % size) == 0)
		{
			/* found */
			ret = GGI_OK;
			break;
		}
	}

	if (ret != GGI_OK) {
		printfailure("This mode and target has no suitable DirectBuffer\n");
		goto exit_testcase;
	}

	if (ggiResourceAcquire(dbuf->resource,
			GGI_ACTYPE_WRITE | GGI_ACTYPE_READ)
	   != 0)
	{
		printfailure("Unable to acquire DirectBuffer\n");
		goto exit_testcase;
	}
#endif

	/* ... and now draw something.
	 */
	pixel_color = ggiMapColor(vis, &color_color);
	ggiSetGCForeground(vis, pixel_color);
	ggiFillscreen(vis);

#if 0
	ggiResourceRelease(dbuf->resource);
#endif


exit_testcase:
	ggiClose(vis);
	ggiExit();

	printsuccess();
	return;
}


int main(int argc, char * const argv[])
{
	int rc;

	parseopts(argc, argv);
	printdesc("Regression testsuite for display-x(7).\n\n");

	rc = ggiInit();
	if (rc < 0) ggPanic("Couldn't initialize libggi");

	testcase1("See, if mansync actually runs in SYNC mode - sets up the mode the same way as XGGI does");

	printsummary();

	return 0;
}
