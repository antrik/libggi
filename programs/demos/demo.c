/* $Id: demo.c,v 1.16 2004/05/21 20:09:07 aldot Exp $
******************************************************************************

   demo.c - the main LibGGI demo

   Authors:	1997 	  Jason McMullan	[jmcc@ggi-project.org]
   		1997-1998 Andreas Beck		[becka@ggi-project.org]
   		1999	  Marcus Sundberg	[marcus@ggi-project.org]

   This software is placed in the public domain and can be used freely
   for any purpose. It comes without any kind of warranty, either
   expressed or implied, including, but not limited to the implied
   warranties of merchantability or fitness for a particular purpose.
   Use it at your own risk. the author is not responsible for any damage
   or consequences raised by use or inability to use this program.

******************************************************************************

   This is a demonstration of LibGGI's functions and can be used as a 
   reference programming example. Thus it is heavily commented to explain
   you every single step to make your own applications.

******************************************************************************
*/

/* This is needed for the HAVE_* macros */
#include "config.h"
#ifndef HAVE_RANDOM
# define random		rand
# define srandom	srand
#endif

/* Include the LibGGI declarations.
 */
#include <ggi/ggi.h>
#include <ggi/gg.h>

/* Include the necessary headers used for e.g. error-reporting.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

/* We are running on a single primary visual which is made accessible
 * globally so subroutines like "waitabit" can access it.
 * See its allocation in main() for details.
 */
ggi_visual_t vis;


/* In case we were called with wrong parameters, give an explanation.
 */
static void usage(const char *prog)
{
	fprintf(stderr, "Usage:\n\n"
		        "%s [--clip] [--target <target>] "
			"<xsize>x<ysize>[#<virtx>x<virty>]"
			"['['<bpp>']']\n\n"
		       "Example: %s 320x200[8]\n"
		"For textmode, try %s 80x25T\n",prog,prog,prog);
	exit(1);
}

/* Wait for a keypress. Shut down everything, if "q" is pressed.
 */
static void waitabit(void)
{
	int key;

	/* Make sure the display is in sync with the drawing command queue. 
	 */
	ggiFlush(vis);

#if 0	/* Activate this to test waiting for max times. 
	 * It will cause the wait to not block indefinitely but only for
	 * 5 seconds.
         */
	{
		struct timeval tv={5,0};
		key=ggiEventPoll(vis,emKeyPress,&tv);
		if (! (key & emKeyPress)) return;
		key = ggiGetc(vis);
	}

#else
	/* ggiGetc will blocking-wait for a keypress. 
	 * we ignore modifier keys.
	 */
	do {
		key = ggiGetc(vis);

	} while ((key == GIIK_VOID) || (GII_KTYP(key) == GII_KT_MOD));
#endif

	if ((key == 'q') || (key == 'Q'))	/* Q pressed */
	{	
		ggiClose(vis);
		ggiExit();
		exit(0);
	}
}

/* Pixel value for white and black. See main() on how to get at it.
 */
ggi_pixel white;
ggi_pixel black;

/* Print the name of the current test in the top left corner.
 */
static void TestName(const char *name)
{
	int left, right, top, bottom;
	int ch_width=8, ch_height=8;
	ggi_mode mode;

	/* Get the current mode. This is needed to determine the size of the
	 * visible area.
	 */
	ggiGetMode(vis, &mode);

	/* Get the font dimensions for ggiPuts.
	 */
	ggiGetCharSize(vis, &ch_width, &ch_height);
	
	/* Save away the old clipping area, and set one that allows to
	 * print the test name in the top left corner.
	 */
	ggiGetGCClipping(vis, &left, &top, &right, &bottom);
	ggiSetGCClipping(vis, 0, 0, mode.virt.x, ch_height);

	/* Now draw a nice black background. Note, that the rendered text
	 * already has a background color. But it is nicer, if the whole 
	 * line is black ...
	 */
	ggiSetGCForeground(vis, black);
	ggiDrawBox(vis, 0, 0, mode.virt.x, ch_height);

	/* Now set the colors of the text and render it.
	 */
	ggiSetGCBackground(vis, black);
	ggiSetGCForeground(vis, white);
	ggiPuts(vis, 0, 0, name);

	/* Flush commands.
	 */
	ggiFlush(vis);

	/* Restore the old clipping rectangle.
	 */
	ggiSetGCClipping(vis, left, top, right, bottom);
}

/* Timing helper routines 
*/
static struct timeval test_start_time;

static void TestStart(void)
{
	ggCurTime(&test_start_time);
}

static int TestTime(void)	/* result is in seconds */
{
	int seconds;
	int micros;

	struct timeval cur_time;

	ggCurTime(&cur_time);

	seconds = cur_time.tv_sec  - test_start_time.tv_sec;
	micros  = cur_time.tv_usec - test_start_time.tv_usec;

	if (micros < 0) {
		micros += 1000000;
		seconds--;
	}

	return seconds;
}


/* The main routine.
 * It will set up a graphics mode as requested on the commandline and 
 * do an extensive test using about all graphics primitives LibGGI
 * knows.
 */
int main(int argc, char **argv)
{
	/* First we define a bunch of variables we will access throughout the
	 * main() function. Most of them are pretty meaningless loop-counters
	 * and helper-variables.
	 */

	const char *prog;	/* Make an alias for the program name */
	
	/* This is an enum holding the requested type of graphics mode.
	 * See the mode setting code below for details.
	 */
	 
	ggi_graphtype type;

	/* This holds the palette we will soon set for GT_8BIT type
	 * graphics modes
	 */
	 
	ggi_color pal[256];

	/* The depth is used for chosing a graphics type.  it is mapped
	 * to the right GT_* by a switch-statement.
	 */
	 
	int depth=0;

	/* Demonstrate clipping ? */

	int doclip=0;

	/* Use a different target than the default ? */

	char *target_name=NULL;

	/* These holde the visible screen size (sx,sy) and
	 * the virtual size (vx,vy). On most targets you can 
	 * request a certain larger area on which the visible
	 * area can be placed.
	 */

	int sx,sy,vx,vy;
	
	/* Pretty meaningless helpers used everywhere. */

	int c,x,y,dx,dy,w,h,i,err,count;

	int sx_5, sx_12;   /* 1/5 and 1/12 respectively */
	int sy_5, sy_12;

	/* Yet another array of colors used for the color-
	 * mapping test
	 */

	ggi_color map[256];
		
	ggi_visual_t memvis;

	/* This buffer will hold data for the Get/Put functions.
	 * Hope this is enough ... 
	 */
	char put_buf[64*1024];

	/* This is a struct containing visual and virtual screen size as
	 * well as the bpp/textmode information 
	 */

	ggi_mode mode = { /* This will cause the default mode to be set */
		1,                      /* 1 frame [???] */
		{GGI_AUTO,GGI_AUTO},    /* Default size */
		{GGI_AUTO,GGI_AUTO},    /* Virtual */
		{0,0},                  /* size in mm don't care */
		GT_AUTO,               /* Mode */
		{GGI_AUTO,GGI_AUTO}     /* Font size */
	};

	/* Get the arguments from the command line. 
	 * Set defaults for optional arguments.
	 */

	prog=*argv; argv++; argc--;
	if (strrchr(prog, '/') != NULL) prog=strrchr(prog, '/')+1;

	if ((argc > 0) &&
	    ((strcmp(*argv, "-h")==0) || 
	     (strcmp(*argv, "--help")==0))) {

		usage(prog);	/* This is not an allowed call format. */
		return 1;	/* Tell the user how to call and fail. */
	}

	if ((argc > 0) && 
	    ((strcmp(*argv, "-c") == 0) ||
	     (strcmp(*argv, "--clip") == 0))) {

		/* Enable clipping */
		doclip=1; argv++; argc--;
	}
	
	if ((argc > 1) && 
	    ((strcmp(*argv, "-t") == 0) ||
	     (strcmp(*argv, "--target") == 0))) {

		target_name=argv[1]; argv+=2; argc-=2;
	}
	
	if (argc > 1) {
		usage(prog);	/* This is not an allowed call format. */
		return 1;	/* Tell the user how to call and fail. */
	}
	
	/* Set up the random number generator. */
	srandom((unsigned)time(NULL));

	/* Initialize the GGI library. This must be called before any other 
	 * GGI function. Otherwise behaviour is undefined.
	 */
	if (ggiInit() != 0) {
		fprintf(stderr, "%s: unable to initialize LibGGI, exiting.\n",
			prog);
		exit(1);
	}

	if (target_name == NULL) {

		/* Open the default visual. This is automatically
		 * selected depending on the calling environment (Linux
		 * console, X, ...). You can explicitly set this in the
		 * GGI_DISPLAY environment variable.
		 */

		vis=ggiOpen(NULL);
	} else {

		/* Open a specific visual.  Note the NULL, which is used
		 * to mark the end of a *list* of LibGGI sub-library names
		 * to be loaded (although at present, it only makes sense 
		 * having either 0 or 1 names in the list).
		 */
		
		vis=ggiOpen(target_name, NULL);
	}

	if (vis == NULL) {
		fprintf(stderr,
			"%s: unable to open default visual, exiting.\n",
			prog);
		ggiExit();
		exit(1);
	}
	/* Using ASYNC mode can give great performance improvements on
	   some targets so it should be used as much as possible.
	   Turning ASYNC mode on right after ggiOpen() ensures best operation
	*/
	ggiSetFlags(vis, GGIFLAG_ASYNC);

	if (argc == 1) { /* A mode-string was given - so we parse it */
		if (ggiParseMode(*argv, &mode) != 0) {
			/* Error parsing mode */
			fprintf(stderr,
				"Error parsing mode: %s\n"
				"Valid modes are:\n"
"  X x Y # XVIRT x YVIRT D dpp.x x dpp.y F frames (T) [bpp] + XOFF + YOFF\n"
"  X x Y # XVIRT x YVIRT D dpp.x x dpp.y F frames [graphtype] + XOFF + YOFF\n\n"
"  Everything (!) can be omitted, all ommitted values default to GGI_AUTO,\n"
"  with the exception of a textmode with unspecified size which defaults\n"
"  to TEXT16.\n  Whitespace is ignored.\n"
				, *argv);
			exit(1);
		}
		/* mode now holds the requested visible 
		 * and virtual size of the screen.
		 */
	}

	/* that's what we try. See what we get ... */
	printf("Trying mode ");
	ggiFPrintMode(stdout,&mode);
	printf("\n");

	/* Is the mode possible ? If not, a better one will be
	 * suggested. 
	 */

	ggiCheckMode(vis,&mode);

	printf("Suggested mode ");
	ggiFPrintMode(stdout,&mode);
	printf("\n");

	err=ggiSetMode(vis,&mode);   /* now try it. it *should* work! */
	
	/* Check if there were errors. Almost all LibGGI functions
	 * return 0 on success and an error code otherwise. No messing
	 * with errno, as this is not thread- friendly. The functions
	 * that are supposed to return a pointer normally return NULL on
	 * error.
	 */

	if (err) {
		fprintf(stderr,"Can't set mode\n");
		ggiClose(vis);
		ggiExit();
		return 2;
	}


	/* Now we read back the set mode, as it might have been
	 * autoselected or changed.
	 */

	type=mode.graphtype;
	vx=mode.virt.x;    vy=mode.virt.y;
	sx=mode.visible.x; sy=mode.visible.y;
	depth=GT_DEPTH(mode.graphtype);

	sx_5 = sx/5; sx_12 = sx/12;
	sy_5 = sy/5; sy_12 = sy/12;

	printf("Visual size is %d mm x %d mm (0 means unknown)\n", mode.size.x, mode.size.y);
	
	/* Set a colorful palette for the tests.
	   Please note that GGI always uses 16 bit color components,
	   so stretch the values accordingly when porting from DOS 
	   or other libs that make assumptions about palette size.

	   On some fixed-palette modes the ggiSetColorfulPalette()
	   call will fail.  We silently ignore that.
	 */
	 
	if (GT_SCHEME(mode.graphtype) == GT_PALETTE) {
		ggiSetColorfulPalette(vis);
		ggiGetPalette(vis, 0, 1<<depth, pal);
	}
	

	/* Find the colors "white" and "black".
	*/
	map[0].r=0xFFFF; /* this is the highest intensity value for the red part. */
	map[0].g=0xFFFF;
	map[0].b=0xFFFF;
	
	white=ggiMapColor(vis, &map[0]);
	printf("white=%d\n", white);

	map[0].r= map[0].g= map[0].b= 0x0;
	black=ggiMapColor(vis, &map[0]);
	printf("black=%d\n", black);

	/* Set the drawing color to black and clear the screen.
	   (The screen is cleared to black at setmode time, but the
	   palette change above might have ruined that)
	*/
	ggiSetGCForeground(vis, black);
	ggiFillscreen(vis);
	/* Set the drawing color to white and background color to black.
	   Then display some text at 0/0.
	 */
	ggiSetGCForeground(vis, white);
	ggiSetGCBackground(vis, black);
	ggiPuts(vis,0,0,"Press any key to begin tests...");
	ggiFlush(vis);

	/* Wait for any keypress. 
	 * This is a blocking call which returns a 16 bit wide unicode
	 * character
	 */
	ggiGetc(vis);

	if (doclip) {
		ggiDrawHLine(vis, 0, sy_5-1,  sx);
		ggiDrawHLine(vis, 0, sy-sy_5, sx);
		ggiDrawVLine(vis, sx_5-1,  0, sy);
		ggiDrawVLine(vis, sx-sx_5, 0, sy);
		ggiSetGCClipping(vis, sx_5, sy_5, sx-sx_5, sy-sy_5);
	}


	/* Here we turn on SYNC mode again beacuse we want to make sure
	   the user sees the drawing as it happens.
	   Note that in SYNC mode no ggiFlush() calls are necessary.
	*/
	ggiRemoveFlags(vis, GGIFLAG_ASYNC);

	/* Now for the first test. Put some text at random places. 
	 * ggiPuts and friends are very simple and always use a small
	 * pixmap font.   Scalable and outline fonts are found in
	 * LibGGI2D.
	 */
	TestStart();
	for (i=0; TestTime() < 3; i++) {
		ggi_color col;

		/* Set the foreground color to some random value */
		col.r = random();
		col.g = random();
		col.b = random();

		ggiSetGCForeground(vis, ggiMapColor(vis, &col));

		/* Set the background color to some random value */
		col.r = random();
		col.g = random();
		col.b = random();

		ggiSetGCBackground(vis, ggiMapColor(vis, &col));

		/* Get some random starting point on the virtual screen. */
		x = random() % vx;
		y = random() % vy;
		
		ggiPuts(vis, x, y, "ggiPuts test!");

		/* If the user hits a key abort this demo. The key will
		 * be available for fetching with ggiGetc() what will
		 * happen in waitabit().
		 */
		if (ggiKbhit(vis)) break;
	}
	printf("Puts(): %d strings\n", i);
	ggiSetGCBackground(vis, black);	
	ggiSetGCForeground(vis, white);	
	i = 25;
	ggiPutc(vis, i, i, (signed)'P'); i+= 10;
	ggiPutc(vis, i, i, (signed)'u'); i+= 10;
	ggiPutc(vis, i, i, (signed)'t'); i+= 10;
	ggiPutc(vis, i, i, (signed)'c'); i+= 10;
	ggiPutc(vis, i, i, (signed)' '); i+= 10;
	ggiPutc(vis, i, i, (signed)'T'); i+= 10;
	ggiPutc(vis, i, i, (signed)'e'); i+= 10;
	ggiPutc(vis, i, i, (signed)'s'); i+= 10;
	ggiPutc(vis, i, i, (signed)'t'); i+= 10;


	/* Check the colormapping code. Draw four horizontal bars in red/green
	 * blue and grey.
	 */
	TestName("MapColor");
	waitabit();

	for (x=0; x<sx; x++) {

		/* Make i (intensity) so it varies from 0-0xFFFF over
		 * the visible length
		 */
		i = 0xFFFF*x/(sx-1);
		
		/* Now first make a color descriptor for the red bar. It
		 * will holf the R/G/B triplet x/0/0 describing the
		 * shades of red.
		 */
		map[0].r=i; map[0].g=0; map[0].b=0;

		/* ggiMapColor will return the color value that is
		 * closest to the requested color. We set this as active
		 * drawing color.
		 */
		ggiSetGCForeground(vis,ggiMapColor(vis, &map[0]));
		
		/* Then we draw a small vertical line at y=20 and height=20.
		 */
		ggiDrawVLine(vis, x, 4*sy_12, sy_12);
		
		/* Now we do the same with green. 
		 */
		map[1].r=0; map[1].g=i; map[1].b=0;
		ggiSetGCForeground(vis,ggiMapColor(vis, &map[1]));
		ggiDrawVLine(vis, x, 5*sy_12, sy_12);

		/* blue */
		map[2].r=0; map[2].g=0; map[2].b=i;
		ggiSetGCForeground(vis,ggiMapColor(vis, &map[2]));
		ggiDrawVLine(vis, x, 6*sy_12, sy_12);

		/* grey */
		map[3].r=i; map[3].g=i; map[3].b=i;
		ggiSetGCForeground(vis,ggiMapColor(vis, &map[3]));
		ggiDrawVLine(vis, x, 7*sy_12, sy_12);
	}


	/* Now the memvisual and crossblitting code.
	 * This is an advanced topic which you should ignore if you are
	 * a beginner.
	 */
	TestName("MemoryVisual");
	waitabit();

	/* Open a "memory-visual" which is simply a simulated display
	 * in memory. You can draw on it as you would on a screen.
	 * Nifty for preparing things in the background.
	 */
	if ((memvis=ggiOpen("display-memory", NULL)) == NULL) {
		goto no_mem_targ;
	}

	/* Set it to a small 32 bit mode. 
	 */
	err = ggiSetSimpleMode(memvis, 160, 40, 1, GT_32BIT);
		
	/* Check for errors
	 */
	if (err) {
		fprintf(stderr, "memvisual error ...\n");
		goto no_mem_targ;
	}

	/* Now draw some strings in the truecolor visual.
	 */
	for (i=0; i < 50; i++) {
		ggi_color col;

		col.r = random();
		col.g = random();
		col.b = random();
		ggiSetGCForeground(memvis, ggiMapColor(memvis, &col));

		col.r = random();
		col.g = random();
		col.b = random();
		ggiSetGCBackground(memvis, ggiMapColor(memvis, &col));

		x = random() % 150;
		y = random() % 35;
		ggiPuts(memvis, x, y, "ggiXBLit !");
	}

	/* Repeatedly blit the memory visual to the main visual.
	 */
	TestStart();
	for (i=0; TestTime() < 3; i++) {
		x = random() % sx;
		y = random() % (sy-8);
		/* Note: This automatically converts colorspace. 
		 * Take care, this might be expensive ...
		 */
		ggiCrossBlit(memvis, 0, 0, 160, 40, vis, x, y);
		if (ggiKbhit(vis)) break;
	}
	printf("CrossBlit(): %d blits\n", i);

	ggiClose(memvis);
	
	no_mem_targ:

	/* Blit rectangular solid boxes into a zig-zagging pattern
	 */
	TestName("DrawBox");
	waitabit();
	TestStart();

	x=random() % (vx-sy_5);
	y=random() % (vy-sy_5);
	dx=1;
	dy=1;

	for (i=0; TestTime() < 4; i++) {

		ggi_color col;

		col.r = (i*229*7) & 0xFFFF;
		col.g = 0xFFFF - ((i*19) & 0xFFFF);
		col.b = (i*5) & 0xFFFF;
		
		ggiSetGCForeground(vis, ggiMapColor(vis, &col));
		ggiDrawBox(vis, x, y, sy_5, sy_5);
		
		x += dx;
		y += dy;
		
		if (x<0) { x=0; dx=-dx; }
		if (y<0) { y=0; dy=-dy; }
		if (x>(vx-sy_5)) { x=vx-sy_5-1; dx=-dx; }
		if (y>(vy-sy_5)) { y=vy-sy_5-1; dy=-dy; }
		if (ggiKbhit(vis)) break;
	}
	printf("DrawBox(): %d boxes\n", i);

	/* SetOrigin is for panning around a larger virtual screen.
	 * There are no guarantees this works. Just as with splitline.
	 * Please note that the code below tries to scroll over the
	 * possible range to check, if the driver clips the request
	 * correctly.
	 */

	/* SetOrigin Y test - doesn't need to work 
	 */
	dy=0;
	if (mode.visible.y < mode.virt.y) {

		TestName("SetOrigin Y");
		waitabit();

		for (dy=0; dy < mode.virt.y-mode.visible.y; dy++) 
		{
			ggiSetOrigin(vis,0,dy);
			ggUSleep(2000);
			if (ggiKbhit(vis)) break;
		}
		ggiSetOrigin(vis,0,0);
	}

	/* SetOrigin X test - doesn't need to work.
	 * We now move the window horizontally.
	 */
	if (mode.visible.x < mode.virt.x) {

		TestName("SetOrigin X");
		waitabit();

		for (dx=0; dx < mode.virt.x-mode.visible.x; dx++) 
		{
			ggiSetOrigin(vis,dx,0);
			ggUSleep(2000);
			if (ggiKbhit(vis)) break;
		}
		ggiSetOrigin(vis,0,0);
	}

	/* Hline tests.
	 * Draw horizontal lines.
	 */
	TestName("HLine");
	waitabit();

	ggiSetGCForeground(vis, black);
	ggiFillscreen(vis);
	TestStart();

	for (c=0; TestTime() < 3; c++) {
		for (i=8; i<sy; i++) {
			ggi_color col;

			col.r = 0;
			col.g = 0;
			col.b = (c*100+i*300) & 0xFFFF;
			
			ggiSetGCForeground(vis, ggiMapColor(vis, &col));

			/* Draw Horizontal lines starting at x=y=i of
			 * length (sx / 2)
			 */
			ggiDrawHLine(vis, i, i, sx/2);
		}
		if (ggiKbhit(vis)) break;
	}

	/* VLine Tests.
	 * Draw vertical lines.
	 */
	TestName("VLine");
	waitabit();

	ggiSetGCForeground(vis, black);
	ggiFillscreen(vis);
	TestStart();

	for (c=0; TestTime() < 3; c++) {
		for (i=8; i<sy; i++) {
			ggi_color col;

			col.r = 0;
			col.g = (c*100+i*300) & 0xFFFF;
			col.b = 0;
			
			ggiSetGCForeground(vis, ggiMapColor(vis, &col));

			/* Draw vertical lines at x=vx-i-1, y=i, and with
			 * height=sy / 5.
			 */
			ggiDrawVLine(vis, vx-i-1, i, sy_5);
		}
		if (ggiKbhit(vis)) break;
	}

	/* Put horizontal lines. "Putting" means blitting the contents
	 * of a memory buffer.  
	 *
	 * Note that the format of this buffer depends on the graphtype.
	 * For example, on GT_8BIT mode the buffer is just a set of
	 * bytes, each value 0..255 corresponds to the color set by
	 * the ggiSetPalette() call.
	 */
	TestName("Hline Put");
	waitabit();

	for(x = 0; x < (signed)sizeof(put_buf); x++) put_buf[x]=x&0xff;

	for(y=10; y<vy; y++) {
		ggiPutHLine(vis, 0, y, vx, put_buf);
	}

	/* The same with vlines.
	 */
	TestName("Vline Put");
	waitabit();

	for(x=0; x<vx; x++) {
		ggiPutVLine(vis, x, 10, vy - 10, put_buf);
	}

	/* The same with boxes.
	 */
	TestName("PutBox");
	waitabit();

	for(y=10; y<vy; y+=30) {
		ggiPutBox(vis, y, y, 40, 40, put_buf);
	}

	/* check if "getting" a box works.
	 */
	TestName("GetBox");
	waitabit();

	ggiGetBox(vis, 0, 0, 40, 40, put_buf);

	for(y=10; y<vy; y+=40) {
		ggiPutBox(vis, vx/2+y, y, 40, 40, put_buf);
	}

	/* The usual "draw random sized box" test */
	TestName("Boxes");
	waitabit();

	ggiSetGCForeground(vis,0);
	ggiFillscreen(vis);
	TestStart();

	for (i=0; TestTime() < 3; i++) {
		ggi_color col;
		col.r = random();
		col.g = random();
		col.b = random();

		x = random() % vx;
		y = random() % vy;
		w = random() % (vx-x) + 1;
		h = random() % (vy-y) + 1;

		ggiSetGCForeground(vis, ggiMapColor(vis, &col));
		ggiDrawBox(vis, x, y, w, h);
		if (ggiKbhit(vis)) break;
	}

	/* Linedrawing tests */
	TestName("Lines");
	waitabit();
	
	ggiSetGCForeground(vis, black);
	ggiFillscreen(vis);

	TestStart();
	for (i=0; TestTime() < 3; i++) {
		ggi_color col;

		col.r = random();
		col.g = random();
		col.b = random();

		x = random() % vx;
		y = random() % vy;
		w = random() % vx;	/* really x2, y2 */
		h = random() % vy;

		ggiSetGCForeground(vis, ggiMapColor(vis, &col));
		ggiDrawLine(vis, x, y, w, h);
		if (ggiKbhit(vis)) break;
	}
	printf("DrawLine(): %d lines\n", i);
		
	/* CopyBox tests */

	TestName("CopyBox");
	waitabit();

	count = 0;
	TestStart();
	while (TestTime() < 4) {
		for (i=0; (i < sy_5) && (TestTime() < 4); i++) {
			ggiCopyBox(vis, /* from */ sx_5*2, sy_5*2, sx_5*2, sy_5*2,
				   /* to */ sx_5*2, sy_5*2 - 1);
			if (ggiKbhit(vis)) goto copybox_end;
		}
		count += i;
		if (TestTime() >= 4) break;

		for (i=0; (i < sx_5) && (TestTime() < 4); i++) {
			ggiCopyBox(vis, /* from */ sx_5, sy_5, sx_5*2, sy_5*2,
				   /* to */ sx_5 - 1, sy_5);
			if (ggiKbhit(vis)) goto copybox_end;
		}
		count += i;
		if (TestTime() >= 4) break;

		for (i=0; (i < sx_5) && (TestTime() < 4); i++) {
			ggiCopyBox(vis, /* from */ 10, sy_5*2, sx_5*2, sy_5*2,
				   /* to */ 11, sy_5*2);
			if (ggiKbhit(vis)) goto copybox_end;
		}
		count += i;
		if (TestTime() >= 4) break;

		for (i=0; (i < sy_5) && (TestTime() < 4); i++) {
			ggiCopyBox(vis, /* from */ sx_5, 0, sx_5*2, sy_5*2,
				   /* to */ sx_5, 1);
			if (ggiKbhit(vis)) goto copybox_end;
		}
		count += i;
	}
  copybox_end:
	printf("CopyBox(): %d boxes\n", count);

	/* Check color packing i.e. converting an array of ggi_color to
	 * somethat that ggiPutHline() etc. can use.
	 */
	TestName("ColorPack");
	waitabit();

	ggiSetGCForeground(vis, black);
	ggiFillscreen(vis);

	/* Make up a grayscale map.
	 */
	for (i=0; i<128; i++) {
		map[i].r=0xffff*i/127;
		map[i].g=0xffff*i/127;
		map[i].b=0xffff*i/127;
	}

	/* Convert it to get/put buffer format, and the blit it to the
	 * screen.
	 */
	ggiPackColors(vis, put_buf, map, 128);

	for (i=0; i<128; i++) {
		ggiPutHLine(vis, 16, 16+i,  128, put_buf);
		ggiPutVLine(vis, 160+i, 16, 128, put_buf);
	}


	/* DirectBuffer.  Not all targets support directbuffer,
	 * and using it correctly requires a lot of code to compensate
	 * for the many types of pixel formats and framebuffer layouts
	 * available.  If you want portability, you should stick to
	 * using the normal LibGGI drawing functions -- a lot of applications
	 * use directbuffers when they do not have to.
	 *
	 * This test fills the framebuffer with random data.  Since it
	 * is random data, we do not need to care about the pixel format.
	 * All we have to do is make sure that we obey the access rules
	 * for the directbuffer.  This is much easier than actually drawing 
	 * something.
	 */
	do {
		const ggi_directbuffer   *dbuf;
		int numplanes, stride, stride2, wordsize;
		
		if (!(dbuf = ggiDBGetBuffer (vis, 0))) break;

		/* First we check what widths we can use to access the 
		 * buffer.  There are some directbuffers where it is
		 * not legal to read or write 16-bit values, for example.
		 * We will try to get either 32, 16 or 8-bit access. 
		 *
		 * There is also a dbuf->align, but as long as we make
		 * sure to never access unaligned values in the buffer,
		 * we don't have to look at it.
		 */
		wordsize = 32;
		if (dbuf->noaccess & 32) {
			wordsize = 16;
				if (dbuf->noaccess & 16) {
					wordsize = 8;
					if (dbuf->noaccess & 8) break;
				}
		}

		/* Now we have to make sure not to write anywhere inside the
		 * directbuffer area where we are not invited, since the
		 * buffer may not be contiguous.  If we access any data 
		 * which we should not, we are not guaranteed good behavior
		 * by the target.
		 */
		switch (dbuf->layout) {
		case blPixelLinearBuffer:
			stride = dbuf->buffer.plb.stride;
			stride2 = 0;
			numplanes = 1;
			break;
		case blPixelPlanarBuffer:
			stride = dbuf->buffer.plan.next_line;
			stride2 = dbuf->buffer.plan.next_plane;
			numplanes = depth;
		default:
			/* Other, even more complicated, layouts exist. */
			stride = stride2 = numplanes = 0;
			break;
		}
		if (!numplanes) break;

		TestName("DirectBuffer");
		waitabit();

		/* Before using a directbuffer, it should be acquired.
		 * While it is acquired, we do not use any other
		 * LibGGI drawing commands.
		 */
		if (ggiResourceAcquire(dbuf->resource, GGI_ACTYPE_WRITE) != 0)
			break;

		TestStart();
		i = 0;
		while ((TestTime() < 10) && (i < numplanes)) {
			for (y = 0; (TestTime() < 15) && (y < vy); y++) {
				uint8 *linestart;
				linestart = (uint8 *)dbuf->write + 
					stride2 * i + stride * y;
				x = 0;
				if (ggiKbhit(vis)) goto dbuf_tidy;
				while (x < vx * (signed)GT_SIZE(type)/wordsize) {
					switch(wordsize) {
					case 32:
					  *((uint32 *)linestart+x) = random();
					  break;
					case 16:
					  *((uint16 *)linestart+x) = random();
					  break;
					case 8:
					  *(linestart+x) = random();
					  break;
					}
					x++;
				}
			}
			i++;
		}

		while (TestTime() < 10) {
			uint8 *linestart;

			if (ggiKbhit(vis)) goto dbuf_tidy;

			i = random() % numplanes;
			y = random() % vy;
			x = random() % (vx * GT_SIZE(type)/wordsize);

			linestart = (uint8 *)dbuf->write + 
			  stride2 * i + stride * y;

			switch(wordsize) {
			case 32:
				*((uint32 *)linestart+x) = random();
				break;
			case 16:
				*((uint16 *)linestart+x) = random();
				break;
			case 8:
				*(linestart+x) = random();
				break;
			}
		}

	dbuf_tidy:

		ggiAddFlags(vis, GGIFLAG_TIDYBUF);
		if (ggiGetFlags(vis) & GGIFLAG_TIDYBUF) {
			TestName("Directbuffer (with GGIFLAG_TIDYBUF flag)");
			waitabit();
			TestStart();

			i = 0;
			dx = (100 < vx) ? 100 : vx;
			dy = (100 < vy) ? 100 : vy;

			c = 0;
			while (TestTime() < 10) {
				uint8 *linestart;

				if (ggiKbhit(vis)) goto dbuf_end;

				i = random() % numplanes;
				y = (vy - dy) / 2 + random() % dy;
				x = (vx - dx) / 2 + random() % dx;
				x *= GT_SIZE(type);
				x /= wordsize;

				linestart = (uint8 *)dbuf->write + 
				  stride2 * i + stride * y;

				switch(wordsize) {
				case 32:
					*((uint32 *)linestart+x) = random();
					break;
				case 16:
					*((uint16 *)linestart+x) = random();
					break;
				case 8:
					*(linestart+x) = random();
					break;
				}

				c++;
				if (c > 100) {
					ggiFlushRegion(vis, 
						       (vx-dx)/2,(vy-dy)/2,
						       dx,dy);
					c = 0;
				}
			}
		}	/* if */
	dbuf_end:
		ggiResourceRelease(dbuf->resource);

	} while (0);


	/* Asynchronous mode. From now on LibGGI is no longer
	 * responsible for keeping display and command in sync. You need
	 * to explicitly call ggiFlush() to make sure the screen gets
	 * updated.
	 *
	 * All applications are recommended to use async mode as much as
	 * possible, as it's much faster on most targets.  There are
	 * very few apps that really need sync mode anyway.
	 *
	 * Just remember only to call ggiFlush() when it's neccesary,
	 * which in most cases equals once every frame.
	 */
	
	ggiSetFlags(vis,GGIFLAG_ASYNC);
	if (!(ggiGetFlags(vis) & GGIFLAG_ASYNC)) goto palette;
	
	TestName("Async mode");
	waitabit();

	TestStart();
	for (i=0; TestTime() < 3; i++) {
		ggi_color col;

		col.r = random();
		col.g = random();
		col.b = random();

		x = random() % vx;
		y = random() % vy;
		w = random() % (vx-x) + 1;
		h = random() % (vy-y) + 1;

		ggiSetGCForeground(vis, ggiMapColor(vis, &col));
		ggiDrawBox(vis, x, y, w, h);

		/* This will only _force_ an update every 1024 boxes.
		 */
		if ((i & 0x3ff)==0x3ff) ggiFlush(vis);
		if (ggiKbhit(vis)) break;
	}

 palette:
	/* Palette Test. We do some colorcycling here.
	 */
	if (GT_SCHEME(mode.graphtype) == GT_PALETTE) {
		ggi_color cols[512];

		TestName("Palette ");
		waitabit();

		memcpy(cols,     pal, 256*sizeof(ggi_color));
		memcpy(cols+256, pal, 256*sizeof(ggi_color));

		TestStart();
		for (i=0; TestTime() < 3; i += 5) {
			
			ggiSetPalette(vis, 0, 1<<depth, cols+(i&0xff));
			ggiFlush(vis);
			ggUSleep(20000);
			if (ggiKbhit(vis)) break;
		}

		ggiSetPalette(vis, 0, 1<<depth, pal);
	}
	if (!ggiGammaMax(vis, GGI_BM_TYPE_COLOR | GGI_BM_SUB_RED, &i, &i)) {
		ggi_float gr, gb, gg;
		TestName("Gamma ");
		waitabit();

		TestStart();
		for (gr = gb = gg = 1.0; 
		     (gr > 0.0) && (TestTime() < 10); 
		     gr -= 0.02) {
			gb = gg = gr;
			ggiSetGamma(vis, gr, gb, gg);
			ggiFlush(vis);
			ggUSleep(20000);
			if (ggiKbhit(vis)) goto gamma_done;
		}
		for (gr = 0.1; 
		     (gr < 1.0) && (TestTime() < 10); 
		     gr += 0.02) {
			ggiSetGamma(vis, gr, 0.1, 0.1);
			ggiFlush(vis);
			ggUSleep(20000);
			if (ggiKbhit(vis)) goto gamma_done;
		}
		for (gb = 0.1; 
		     (gb < 1.0) && (TestTime() < 10); 
		     gb += 0.02) {
			ggiSetGamma(vis, 1.0, gb, 0.1);
			ggiFlush(vis);
			ggUSleep(20000);
			if (ggiKbhit(vis)) goto gamma_done;
		}
		for (gg = 0.1; 
		     (gg < 1.0) && (TestTime() < 10); 
		     gg += 0.02) {
			ggiSetGamma(vis, 1.0, 1.0, gg);
			ggiFlush(vis);
			ggUSleep(20000);
			if (ggiKbhit(vis)) goto gamma_done;
		}
	gamma_done:
		ggiSetGamma(vis, 1.0, 1.0, 1.0);
	}

	TestName("That's all folks");
	TestStart();
	while (TestTime() < 1) { };
	
	/* O.K. - all done. we will now cleanly shut down LibGGI.
	 */

	/* First close down the visual we requested. Any further calls
	 * using vis have undefined behaviour. LibGGI is still up and 
	 * you could open another visual
	 */
	ggiClose(vis);

	/* Now close down LibGGI. Every LibGGI call except ggiInit has 
	 * undefined behaviour now. It is not recommended to needlessly
	 * deinit-reinit LibGGI, but it's possible.
	 */
	ggiExit();	

	/* Terminate the program.
	 */
	return 0;
}
