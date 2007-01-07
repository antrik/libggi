/* $Id: speed.c,v 1.13 2007/01/07 17:19:14 pekberg Exp $
******************************************************************************

   speed.c - LibGGI speed-test application.

   Written in 1998 by Andreas Beck	[becka@ggi-project.org]

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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#ifdef HAVE_SYS_RESOURCE_H
#include <sys/resource.h>
#else
#include <sys/times.h>
#endif

#ifdef HAVE_LIMITS_H
#include <limits.h>
#endif
#ifndef CLK_TCK
#include <time.h>
#endif
#ifndef CLK_TCK
# define  CLK_TCK	sysconf(_SC_CLK_TCK)
#endif

static struct {
	ggi_visual_t vis;
	int sx,sy,vx,vy;
} mode;

/**********************************************************************/

#ifdef HAVE_SYS_RESOURCE_H
static struct rusage timer;
#else
static struct tms timer;
#endif
static double u_time, s_time;

/* A few simple timing routines. Note that this is Unixish and might need
 * autoconfiguration later ...
 */

/* Start a timer.
 */
static void time_start(void)
{
#ifdef HAVE_SYS_RESOURCE_H
	getrusage(RUSAGE_SELF, &timer);
#else
	times(&timer);
#endif
	u_time = 0.0;
	s_time = 0.0;
}

/* Sample an intermediate result. This is done for later correction for
 * calling overhead and such.
 */
static void time_offset(void)
{
#ifdef HAVE_SYS_RESOURCE_H
	struct rusage end; 
	getrusage(RUSAGE_SELF, &end);
	u_time = -((double)end.ru_utime.tv_sec - timer.ru_utime.tv_sec)
		- ((double)end.ru_utime.tv_usec - timer.ru_utime.tv_usec) / 1000000.0;
	s_time = -((double)end.ru_stime.tv_sec - timer.ru_stime.tv_sec)
		- ((double)end.ru_stime.tv_usec - timer.ru_stime.tv_usec) / 1000000.0;
#else
	struct tms end; 
	times(&end);
	u_time = -((double)end.tms_utime - timer.tms_utime) / CLK_TCK;
	s_time = -((double)end.tms_stime - timer.tms_stime) / CLK_TCK;
#endif
	time_start();
}

/* Stop the timer and make the final result. The correction obtained in 
 * time_offset is applied.
 * u_time and s_time contain the user- and system times afterwards.
 */
static void time_stop(void)
{
#ifdef HAVE_SYS_RESOURCE_H
	struct rusage end; 
	getrusage(RUSAGE_SELF, &end);
	u_time += ((double)end.ru_utime.tv_sec - timer.ru_utime.tv_sec)
		+ ((double)end.ru_utime.tv_usec - timer.ru_utime.tv_usec) / 1000000.0;
	s_time += ((double)end.ru_stime.tv_sec - timer.ru_stime.tv_sec)
		+ ((double)end.ru_stime.tv_usec - timer.ru_stime.tv_usec) / 1000000.0;
#else
	struct tms end; 
	times(&end);
	u_time += ((double)end.tms_utime - timer.tms_utime) / CLK_TCK;
	s_time += ((double)end.tms_stime - timer.tms_stime) / CLK_TCK;
#endif
}

/* The pixelvalue for the color white.
 */
static ggi_pixel white_pixel;

/* Print the name of the currently excuting test in the top left corner.
 */
static void TestName(const char *name)
{
	ggiSetGCForeground(mode.vis, white_pixel);
	ggiPuts(mode.vis, 0, 0, name);
}

/* This is an empty function that is used for calibration.
 */
static void nothing(ggi_visual_t vis, int foo, int bar, int foo2, int bar2)
{}

#if 0
/* Get the number of pixels that are in the rectangle (including _all_
 * borders).
 */
static int getnumpixels(int x1,int y1,int x2,int y2)
{
	int x, y, c; 
	ggi_pixel pix;

	c = 0;
	for (y=y1; y <= y2; y++) {
		for (x=x1; x <= x2; x++) {
	  		ggiGetPixel(mode.vis, x, y, &pix);
	  		if (pix != 0) c++;
		}
	}
	return c;
}
#endif

#define RANDSEED 0x12345678

#if 0
/* Draw a random pattern. See speed.c.
 */
static void drawrandom(int x1,int y1,int x2,int y2)
{
	int x,y; 
	srand(RANDSEED);
	for (y=y1; y<=y2; y++) {
		for (x=x1; x<=x2; x++) {
	  		if (rand()&1) ggiDrawPixel(mode.vis,x,y);
		}
	}
}
#endif

#if 0
/* Check for the above generated pattern.
 */
static int checkrandom(int x1,int y1,int x2,int y2)
{
	int x, y; 
	ggi_pixel pix;

	srand(RANDSEED);
	for (y=y1; y <= y2; y++) {
		for (x = x1; x <= x2; x++) {
	  		ggiGetPixel(mode.vis, x, y, &pix);
	  		if (!pix != !(rand()&1)) return 1;
		}
	}
	return 0;
}
#endif

/* Check DrawLine speed.
 */
static void Line(void)
{
	int x,c,mxcnt;

	for (mxcnt=1000;mxcnt<1000000000;mxcnt*=10) {
		time_start();
		for (c=mxcnt;c>0;c--)
			nothing(mode.vis,10,10,10,10);
		time_offset();
		for (c=mxcnt;c>0;c--)
			ggiDrawLine(mode.vis,10,10,10,10);
		time_stop();
		if ( u_time+s_time > 1.0 ) break;
	}
	printf("Line          1: %15.2f %6.3f %6.3f\n",mxcnt/(u_time+s_time),u_time,s_time);
	for (mxcnt=1;mxcnt<1000000000;mxcnt*=10)
	{
		time_start();
		for (c=mxcnt/10;c>0;c--)
			for (x=10;x<20;x++)
				nothing(mode.vis,10,10,19,x);
		time_offset();
		for (c=mxcnt/10;c>0;c--)
			for (x=10;x<20;x++)
				ggiDrawLine(mode.vis,10,10,19,x);
		time_stop();
		if ( u_time+s_time > 1.0 ) break;
	}
	printf("Line         10: %15.2f %6.3f %6.3f\n",mxcnt/(u_time+s_time),u_time,s_time);
	for (mxcnt=1;mxcnt<1000000000;mxcnt*=10)
	{
		time_start();
		for (c=mxcnt/100;c>0;c--)
			for (x=10;x<110;x++)
				nothing(mode.vis,10,10,109,x);
		time_offset();
		for (c=mxcnt/100;c>0;c--)
			for (x=10;x<110;x++)
				ggiDrawLine(mode.vis,10,10,109,x);
		time_stop();
		if ( u_time+s_time > 1.0 ) break;
	}
	printf("Line        100: %15.2f %6.3f %6.3f\n",mxcnt/(u_time+s_time),u_time,s_time);
}

/* Check DrawBox speed.
 */
static void Box(void)
{
	int c,mxcnt;

	for (mxcnt=1;mxcnt<1000000000;mxcnt*=10)	{
		time_start();
		for (c=0;c<mxcnt;c++) {
			nothing(mode.vis,10,10,1,1);
		}
		time_offset();
		for (c=0;c<mxcnt;c++) {
			ggiDrawBox(mode.vis,10,10,1,1);
		}
		time_stop();
		if ( u_time+s_time > 1.0 ) break;
	}
	printf("Box           1: %15.2f %6.3f %6.3f\n",mxcnt/(u_time+s_time),u_time,s_time);
	for (mxcnt=1;mxcnt<1000000000;mxcnt*=10)
	{
		time_start();
		for (c=0;c<mxcnt;c++)
			nothing(mode.vis,10,10,10,10);
		time_offset();
		for (c=0;c<mxcnt;c++)
			ggiDrawBox(mode.vis,10,10,10,10);
		time_stop();
		if ( u_time+s_time > 1.0 ) break;
	}
	printf("Box          10: %15.2f %6.3f %6.3f\n",mxcnt/(u_time+s_time),u_time,s_time);
	for (mxcnt=1;mxcnt<1000000000;mxcnt*=10)
	{
		time_start();
		for (c=0;c<mxcnt;c++)
			nothing(mode.vis,10,10,100,100);
		time_offset();
		for (c=0;c<mxcnt;c++)
			ggiDrawBox(mode.vis,10,10,100,100);
		time_stop();
		if ( u_time+s_time > 1.0 ) break;
	}
	printf("Box         100: %15.2f %6.3f %6.3f\n",mxcnt/(u_time+s_time),u_time,s_time);
}

/* Check CopyBox speed. Check all overlapping cases. This can make a real
 * difference. Cache prediction and such ...
 */
static void CopyBox(void)
{
	int c,mxcnt;

	for (mxcnt=1;mxcnt<1000000000;mxcnt*=10)	{
		time_start();
		for (c=0;c<mxcnt;c++) {
			nothing(mode.vis,10,10,1,1);
		}
		time_offset();
		for (c=0;c<mxcnt;c++) {
			ggiCopyBox(mode.vis,10,10,1,1,120,10);
		}
		time_stop();
		if ( u_time+s_time > 1.0 ) break;
	}
	printf("CopyBox no    1: %15.2f %6.3f %6.3f\n",mxcnt/(u_time+s_time),u_time,s_time);
	for (mxcnt=1;mxcnt<1000000000;mxcnt*=10)
	{
		time_start();
		for (c=0;c<mxcnt;c++)
			nothing(mode.vis,10,10,10,10);
		time_offset();
		for (c=0;c<mxcnt;c++)
			ggiCopyBox(mode.vis,10,10,10,10,120,10);
		time_stop();
		if ( u_time+s_time > 1.0 ) break;
	}
	printf("CopyBox no   10: %15.2f %6.3f %6.3f\n",mxcnt/(u_time+s_time),u_time,s_time);
	for (mxcnt=1;mxcnt<1000000000;mxcnt*=10)
	{
		time_start();
		for (c=0;c<mxcnt;c++)
			nothing(mode.vis,10,10,100,100);
		time_offset();
		for (c=0;c<mxcnt;c++)
			ggiCopyBox(mode.vis,10,10,100,100,120,10);
		time_stop();
		if ( u_time+s_time > 1.0 ) break;
	}
	printf("CopyBox no  100: %15.2f %6.3f %6.3f\n",mxcnt/(u_time+s_time),u_time,s_time);

	for (mxcnt=1;mxcnt<1000000000;mxcnt*=10)	{
		time_start();
		for (c=0;c<mxcnt;c++) {
			nothing(mode.vis,10,10,1,1);
		}
		time_offset();
		for (c=0;c<mxcnt;c++) {
				ggiCopyBox(mode.vis,10,10,1,1,10,10);
		}
		time_stop();
		if ( u_time+s_time > 1.0 ) break;
	}
	printf("CopyBox ful   1: %15.2f %6.3f %6.3f\n",mxcnt/(u_time+s_time),u_time,s_time);
	for (mxcnt=1;mxcnt<1000000000;mxcnt*=10)
	{
		time_start();
		for (c=0;c<mxcnt;c++)
			nothing(mode.vis,10,10,10,10);
		time_offset();
		for (c=0;c<mxcnt;c++)
			ggiCopyBox(mode.vis,10,10,10,10,10,10);
		time_stop();
		if ( u_time+s_time > 1.0 ) break;
	}
	printf("CopyBox ful  10: %15.2f %6.3f %6.3f\n",mxcnt/(u_time+s_time),u_time,s_time);
	for (mxcnt=1;mxcnt<1000000000;mxcnt*=10)
	{
		time_start();
		for (c=0;c<mxcnt;c++)
			nothing(mode.vis,10,10,100,100);
		time_offset();
		for (c=0;c<mxcnt;c++)
			ggiCopyBox(mode.vis,10,10,100,100,10,10);
		time_stop();
		if ( u_time+s_time > 1.0 ) break;
	}
	printf("CopyBox ful 100: %15.2f %6.3f %6.3f\n",mxcnt/(u_time+s_time),u_time,s_time);

	for (mxcnt=1;mxcnt<1000000000;mxcnt*=10)
	{
		time_start();
		for (c=0;c<mxcnt;c++)
			nothing(mode.vis,10,10,10,10);
		time_offset();
		for (c=0;c<mxcnt;c++)
			ggiCopyBox(mode.vis,10,10,10,10,15,15);
		time_stop();
		if ( u_time+s_time > 1.0 ) break;
	}
	printf("CopyBox rd   10: %15.2f %6.3f %6.3f\n",mxcnt/(u_time+s_time),u_time,s_time);
	for (mxcnt=1;mxcnt<1000000000;mxcnt*=10)
	{
		time_start();
		for (c=0;c<mxcnt;c++)
			nothing(mode.vis,10,10,100,100);
		time_offset();
		for (c=0;c<mxcnt;c++)
			ggiCopyBox(mode.vis,10,10,100,100,60,60);
		time_stop();
		if ( u_time+s_time > 1.0 ) break;
	}
	printf("CopyBox rd  100: %15.2f %6.3f %6.3f\n",mxcnt/(u_time+s_time),u_time,s_time);

	for (mxcnt=1;mxcnt<1000000000;mxcnt*=10)
	{
		time_start();
		for (c=0;c<mxcnt;c++)
			nothing(mode.vis,10,10,10,10);
		time_offset();
		for (c=0;c<mxcnt;c++)
			ggiCopyBox(mode.vis,15,10,10,10,10,15);
		time_stop();
		if ( u_time+s_time > 1.0 ) break;
	}
	printf("CopyBox ld   10: %15.2f %6.3f %6.3f\n",mxcnt/(u_time+s_time),u_time,s_time);
	for (mxcnt=1;mxcnt<1000000000;mxcnt*=10)
	{
		time_start();
		for (c=0;c<mxcnt;c++)
			nothing(mode.vis,10,10,100,100);
		time_offset();
		for (c=0;c<mxcnt;c++)
			ggiCopyBox(mode.vis,60,10,100,100,10,60);
		time_stop();
		if ( u_time+s_time > 1.0 ) break;
	}
	printf("CopyBox ld  100: %15.2f %6.3f %6.3f\n",mxcnt/(u_time+s_time),u_time,s_time);

	for (mxcnt=1;mxcnt<1000000000;mxcnt*=10)
	{
		time_start();
		for (c=0;c<mxcnt;c++)
			nothing(mode.vis,10,10,10,10);
		time_offset();
		for (c=0;c<mxcnt;c++)
			ggiCopyBox(mode.vis,10,15,10,10,15,10);
		time_stop();
		if ( u_time+s_time > 1.0 ) break;
	}
	printf("CopyBox ru   10: %15.2f %6.3f %6.3f\n",mxcnt/(u_time+s_time),u_time,s_time);
	for (mxcnt=1;mxcnt<1000000000;mxcnt*=10)
	{
		time_start();
		for (c=0;c<mxcnt;c++)
			nothing(mode.vis,10,10,100,100);
		time_offset();
		for (c=0;c<mxcnt;c++)
			ggiCopyBox(mode.vis,10,60,100,100,60,10);
		time_stop();
		if ( u_time+s_time > 1.0 ) break;
	}
	printf("CopyBox ru  100: %15.2f %6.3f %6.3f\n",mxcnt/(u_time+s_time),u_time,s_time);

	for (mxcnt=1;mxcnt<1000000000;mxcnt*=10)
	{
		time_start();
		for (c=0;c<mxcnt;c++)
			nothing(mode.vis,10,10,10,10);
		time_offset();
		for (c=0;c<mxcnt;c++)
			ggiCopyBox(mode.vis,15,15,10,10,10,10);
		time_stop();
		if ( u_time+s_time > 1.0 ) break;
	}
	printf("CopyBox lu   10: %15.2f %6.3f %6.3f\n",mxcnt/(u_time+s_time),u_time,s_time);
	for (mxcnt=1;mxcnt<1000000000;mxcnt*=10)
	{
		time_start();
		for (c=0;c<mxcnt;c++)
			nothing(mode.vis,10,10,100,100);
		time_offset();
		for (c=0;c<mxcnt;c++)
			ggiCopyBox(mode.vis,60,60,100,100,10,10);
		time_stop();
		if ( u_time+s_time > 1.0 ) break;
	}
	printf("CopyBox lu  100: %15.2f %6.3f %6.3f\n",mxcnt/(u_time+s_time),u_time,s_time);
}


/* List of tests.
 */
static struct test 
{
	const char *name;
	void (*func)(void);
	int active;
} tests[]=
{
	{"Line",	Line,		0},
	{"Box",		Box,		0},
	{"CopyBox",	CopyBox,	0},
	{ NULL,NULL,0 }
};

/* Display usage info.
 */
static void usage(const char *prog)
{
	fprintf(stderr,"Usage:\n\n"
		       "%s [-flags] [--tests] \n\n"
		       "Default: %s --all 8 320 200 320 200\n"
		       "Supported flags are :\n",prog,prog);

	fprintf(stderr,	"-? list available tests\n");

	exit(1);
}

/* Display usage info. List of tests.
 */
static void list_tests(void)
{
	int testnum;
	
	fprintf(stderr,"Available tests are :\n");
	
	for (testnum=0;tests[testnum].name;testnum++)
	{
		fprintf(stderr,"--%s\n",tests[testnum].name);
	}
}

/* Check arguments.
 */
static int parse_args(int argc,char **argv)
{
	int x,testnum;

	for (x=1;x<argc;x++) {
		if (*argv[x]=='-') {
			switch(argv[x][1]) {

				case '?': list_tests();return 1;
				case '-':
					for (testnum=0;tests[testnum].name;testnum++)
					{
						if (strcmp(tests[testnum].name,argv[x]+2)==0 ||
						    strcmp("all",              argv[x]+2)==0 )
							tests[testnum].active=1;
					} break;
				default:
					fprintf(stderr,"%s: Unknown switch '%s' !\n\n",argv[0],argv[x]);
					usage(argv[0]);
					return 1;
			}
		}
		else {
			fprintf(stderr,"%s: Can't parse '%s'.\n\n",argv[0],argv[x]);
			usage(argv[0]);
			return 1;
		}
	}	

	return 0;
}

/* Set up default mode.
 */
static int setup_mode(void)
{
	int err;
	ggi_color map[256];
	ggi_mode gmode;

	if ((mode.vis=ggNewStem(NULL)) == NULL) {
		fprintf(stderr,
			"unable to create stem, exiting.\n");
		exit(1);
	}

	if (ggiAttach(mode.vis) < 0) {
		fprintf(stderr,
			"unable to attach ggi, exiting.\n");
		exit(1);
	}

	if (ggiOpen(mode.vis, NULL) < 0) {
		fprintf(stderr,
			"unable to open default visual, exiting.\n");
		exit(1);
	}

	if ((err = ggiSetSimpleMode(mode.vis, GGI_AUTO, GGI_AUTO, 1, GT_AUTO))
	    == 0) {
		ggiGetMode(mode.vis,&gmode);
		mode.sx=gmode.visible.x;
		mode.sy=gmode.visible.y;
		mode.vx=gmode.virt.x;
		mode.vy=gmode.virt.y;
		printf("Graph mode %dx%d (%dx%d virt)\n",
			mode.sx,mode.sy,mode.vx,mode.vy);
	} else {
		fprintf(stderr,"Can't set mode\n");
		return 1;
	}

	map[0].r=0xFFFF;
	map[0].g=0xFFFF;
	map[0].b=0xFFFF;
	white_pixel=ggiMapColor(mode.vis, &map[0]);
	printf("white=%u\n",white_pixel);

	return 0;
}

/* Main function. Check parameters. Set up default mode. Run all tests.
 */
int
main(int argc,char **argv)
{
	int testnum;

	if (parse_args(argc,argv)) return 1;

	srandom((unsigned)time(NULL));
	if (ggiInit() != 0) {
		fprintf(stderr, "%s: unable to initialize LibGGI, exiting.\n",
			argv[0]);
		exit(1);
	}

	if (setup_mode()) return 2;
	
	for (testnum=0;tests[testnum].name;testnum++) {
		if (!tests[testnum].active) continue;
		TestName(tests[testnum].name);
		tests[testnum].func();
	}

	ggDelStem(mode.vis);
	ggiExit();	

	return 0;
}
