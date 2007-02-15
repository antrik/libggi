/* $Id: consistency.c,v 1.11 2007/02/15 20:35:57 cegger Exp $
******************************************************************************

   This is a consistency-test application.

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
#include <ggi/gii.h>
#include <ggi/ggi.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>


/* If this flag is set, break out early when inconsitencies are detected.
 * This is useful for quick-checks, while for fixing things, it is often
 * more useful to get _all_ error messages.
 */
static enum flags {
	BREAK_UNCONSISTENT=1,	/* Stop consistency tests
				   when a few failures have been recorded */
} flags = 0;

/* Global data about the opened visual and the mode on it.
 */
static struct {
	ggi_visual_t vis;
	int sx,sy,vx,vy;
} mode;

/**********************************************************************/

/* The pixelvalue of the color "white".
 */
static ggi_pixel white_pixel, black_pixel;

/* Print the name of the currently running test in the top left corner
 */
static void TestName(const char *name)
{
	ggiSetGCForeground(mode.vis,white_pixel);
	ggiPuts(mode.vis,0,0,name);
}

/* This gets the number of pixels with a nonzero pixelvalue within the
 * rectangle enclosed by x1,y2,x2,y2 - including both borders.
 */
static int getnumpixels(int x1,int y1,int x2,int y2)
{
	int x,y,c;
	ggi_pixel pix;
	c=0;
	for (y=y1; y <= y2; y++) {
		for (x=x1; x <= x2; x++) {
			ggiGetPixel(mode.vis, x, y, &pix);
			if (pix != 0) c++;
		}
	}
	return c;
}

/* That is just an arbitrary value, that ensures we always get the same
 * pseudo-random sequence.
 */
#define RANDSEED 0x12345678

/* Draw a random sequence into a rectangle.
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

/* Check, if the pixels set in a rectangle are those that would have been
 * drawn by the above routine.
 */
static int checkrandom(int x1,int y1,int x2,int y2)
{
	int x, y;
	ggi_pixel pix;

	srand(RANDSEED);
	for (y=y1; y<=y2; y++) {
		for (x=x1; x<=x2; x++) {
			ggiGetPixel(mode.vis, x, y, &pix);
			if (!pix != !(rand()&1)) return 1;
		}
	}
	return 0;
}

/* A few very basic checks, that test the checking system itself.
 * If these fail, something is _very_ wrong.
 */
static void BasicConsistency(void)
{
	int c,x,y,pass;
	ggi_pixel rc;
	ggi_pixel pix;

	fprintf(stderr,"Consistency: Testing GC color get/set ... ");

	pass=1;
	for(x=1;x!=0;x<<=1) {
		ggiSetGCForeground(mode.vis, (ggi_pixel)x);
		ggiGetGCForeground(mode.vis,&rc);
		if ((unsigned)x != rc) {
			pass=0;break;
		}
	}
	if (pass) fprintf(stderr,"passed.\n"); else fprintf(stderr,"failed at %x.\n",x);

	/* Get/Put/DrawPixel consistency test */
	ggiSetGCForeground(mode.vis, black_pixel);
	ggiFillscreen(mode.vis);

	/* The screen should now be all zero. */
	c=0;
	for (y=0;y<mode.vy;y++) {
		for (x=0;x<mode.vx;x++)	{
			ggiGetPixel(mode.vis, x, y, &pix);
			if (pix != 0) {
				fprintf(stderr,
					"Warning: Screen not blank at %d,%d (%x)\n",
					x,y,pix);
				c++;
				if (c > 16) {
					fprintf(stderr, "Error: Screen not blank or GetPixel not working. Consitency checks useless.\n");
					y=mode.vy;
					break;
				}
			}
		}
	}
}

/* Most targets have a continuous range of possible color values, and loop
 * at the end. As soon as we set something larger than the maximum, we get
 * back the modulus (usually 0). We check if, and when that happens.
 * The output is neither pass/fail, nor does it indicate much, but it
 * can be helpful to target developers.
 */
static void ColorWrap(void)
{
	ggi_pixel pix, pix2;
	/* Now we check when we have a color wraparound. */
	fprintf(stderr, "Consistency: Testing color wraparound ... ");
	for (pix=0; pix != 0xffffffff; pix++) {
		ggiPutPixel(mode.vis, 0, 0, pix);
		ggiGetPixel(mode.vis, 0, 0, &pix2);
		if (pix != pix2) {
			fprintf(stderr, "at %08x.\n", pix);
			break;
		}
	}
	if (pix == 0xffffffff) fprintf(stderr,"none\n");
}

/* Test horizontal lines.
 */
static void Hline(void)
{
	int x,y,c,pass;

	TestName("Hline");
	ggiSetGCForeground(mode.vis, black_pixel);
	ggiFillscreen(mode.vis);
	pass=1;
	fprintf(stderr,"Consistency: Testing HLine ... ");
	for (x=0;x<mode.vx;x++) {
		for (y=0;y<=mode.vx-x;y++) {
			ggiSetGCForeground(mode.vis,white_pixel);
			ggiDrawHLine(mode.vis,x,0,y);
			ggiSetGCForeground(mode.vis, black_pixel);
			/* for speed reasons, we hope for no stray pixels far away ... */
			if ((c=getnumpixels(0,0,mode.vx-1,1))!=y || getnumpixels(x,0,x+y-1,0)!=y ) {
				ggiFillscreen(mode.vis);
				pass=0;
				if (flags&BREAK_UNCONSISTENT) { x=mode.vx; break; }
				fprintf(stderr,"Error:Hline(%d,0,%d); consistency failed (%d pixels measured).\n",x,y,c);
			} else {
				ggiDrawHLine(mode.vis,x,0,y);
			}
		}
	}
	if (pass) fprintf(stderr,"passed.\n"); else fprintf(stderr,"failed at %x.\n",x);
}

/* Test vertical lines.
 */
static void Vline(void)
{
	int x=0, y, c, pass;

	pass=1;
	fprintf(stderr,"Consistency: Testing VLine ... ");
	for (y=0;y<mode.vy;y++)
	{
		ggiSetGCForeground(mode.vis, black_pixel);
		ggiFillscreen(mode.vis);
		ggiSetGCForeground(mode.vis,white_pixel);
		for (x=0;x<=mode.vy-y;x++)
		{
			ggiDrawVLine(mode.vis,0,y,x);
			/* for speed reasons, we hope for no stray pixels far away ... */
			if ((c=getnumpixels(0,0,1,mode.vy-1))!=x || (c=getnumpixels(0,y,0,x+y-1))!=x )
			{
				pass=0;
				if (flags&BREAK_UNCONSISTENT) { y=mode.vy; break; }
				fprintf(stderr,"Error:Vline(0,%d,%d); consistency failed (%d pixels measured).\n",y,x,c);
			}
		}
	}
	if (pass) fprintf(stderr,"passed.\n"); else fprintf(stderr,"failed at %x.\n",x);
}

/* Subroutine that checks, if DrawLine is "exact". That is, it follows a
 * given ruleset for rounding and such. The test is compliant with the
 * article by "the master of lines", Bresenham - at least I hope so ;-).
 */
static int CheckLine(ggi_visual_t vis,int x1,int y1,int x2,int y2)
{
	int xx,max,cx,cy,fail;
	ggi_pixel pix;

	fail=0;
	ggiSetGCForeground(vis,white_pixel);
	ggiDrawLine(vis,x1,y1,x2,y2);
	ggiSetGCForeground(vis, black_pixel);
	max=abs(x2-x1);if (abs(y2-y1)>max) max=abs(y2-y1);
	if (max==0) return 0;
	for (xx=0;xx<=max;xx++)
	{
		cx=(x1*xx+x2*(max-xx)+max/2)/max;
		cy=(y1*xx+y2*(max-xx)+max/2)/max;
		ggiGetPixel (vis, cx, cy, &pix);
		if (pix == 0) {
			fprintf(stderr,
				"Line: Unset pixel %d,%d in line(%d,%d,%d,%d).\n",
				cx,cy,x1,y1,x2,y2);
			fail = 1;
			if (flags&BREAK_UNCONSISTENT) break;
		}
		ggiDrawPixel(vis,cx,cy);
	}
	if ((pix = getnumpixels(0,0,120,120)) != black_pixel) {
		fprintf(stderr,
			"Line: %d surplus pixels in line(%d,%d,%d,%d).\n",
			pix,x1,y1,x2,y2);
		ggiFillscreen(vis);
		fail=1;
	}
	return fail;
}

/* Do an exhaustive check for correctly working line(). Note, that it is
 * important to check _all_ directions. This might not be complete, as there
 * could be errors at specific major lengths, which are not completely checked.
 * Maybe we should add a few more ...
 */
static void Line(void)
{
	int x,failcnt;

	fprintf(stderr,"Consistency: Testing Line ... ");
	failcnt=0;

	ggiSetGCForeground(mode.vis, black_pixel);
	ggiFillscreen(mode.vis);

	for (x=10;x<110;x++)
		failcnt+=CheckLine(mode.vis, 10, 10,x,110);
	for (x=10;x<110;x++)
		failcnt+=CheckLine(mode.vis, 10, 10,110,x);
	for (x=10;x<110;x++)
		failcnt+=CheckLine(mode.vis,110, 10,x,110);
	for (x=10;x<110;x++)
		failcnt+=CheckLine(mode.vis,110, 10, 10,x);
	for (x=10;x<110;x++)
		failcnt+=CheckLine(mode.vis, 10,110,x, 10);
	for (x=10;x<110;x++)
		failcnt+=CheckLine(mode.vis, 10,110,110,x);
	for (x=10;x<110;x++)
		failcnt+=CheckLine(mode.vis,110,110,x, 10);
	for (x=10;x<110;x++)
		failcnt+=CheckLine(mode.vis,110,110, 10,x);

	if (failcnt==0)	fprintf(stderr,"passed.\n");
	else fprintf(stderr,"failed.\n");

}

/* Check boxes.
 */
static void Box(void)
{
	int x,y,c,pass;

	pass=1;
	fprintf(stderr,"Consistency: Testing box height=0 ... ");
	ggiSetGCForeground(mode.vis, black_pixel);
	ggiFillscreen(mode.vis);

	/* Degenerate case - height=0 */
	for (x=0;x<mode.vx;x++) {
		for (y=0;y<=mode.vx-x;y++) {
			ggiSetGCForeground(mode.vis,white_pixel);
			ggiDrawBox(mode.vis,x,0,y,0);
			ggiSetGCForeground(mode.vis, black_pixel);
			/* for speed reasons, we hope for no stray pixels far away ... */
			if ((c=getnumpixels(0,0,mode.vx-1,1))!=0) {
				ggiFillscreen(mode.vis);
				pass=0;
				if (flags&BREAK_UNCONSISTENT) { x=mode.vx; break; }
				fprintf(stderr,"Error:Box(%d,0,%d,0); consistency failed (%d pixels measured - should be 0).\n",x,y,c);
			} else {
				ggiDrawBox(mode.vis,x,0,y,0);
			}
		}
	}
	if (pass) fprintf(stderr,"passed.\n"); else fprintf(stderr,"failed at %x.\n",x);
	pass=1;
	fprintf(stderr,"Consistency: Testing box height=1 ... ");
	for (x=0;x<mode.vx;x++) {
		for (y=0;y<=mode.vx-x;y++) {
			ggiSetGCForeground(mode.vis,white_pixel);
			ggiDrawBox(mode.vis,x,0,y,1);
			ggiSetGCForeground(mode.vis, black_pixel);
			/* for speed reasons, we hope for no stray pixels far away ... */
			if ((c=getnumpixels(0,0,mode.vx-1,1))!=y || getnumpixels(x,0,x+y-1,0)!=y) {
				ggiFillscreen(mode.vis);
				pass=0;
				if (flags&BREAK_UNCONSISTENT) { x=mode.vx; break; }
				fprintf(stderr,"Error:Box(%d,0,%d,1); consistency failed (%d pixels measured - should be %d).\n",x,y,c,y);
			} else {
				ggiDrawBox(mode.vis,x,0,y,1);
			}
		}
	}
	if (pass) fprintf(stderr,"passed.\n"); else fprintf(stderr,"failed at %x.\n",x);
	pass=1;
	fprintf(stderr,"Consistency: Testing box all aspects ... ");
	for (x=0;x<32;x++) {
		for (y=0;y<=32;y++) {
			ggiSetGCForeground(mode.vis,white_pixel);
			ggiDrawBox(mode.vis,0,0,x,y);
			ggiSetGCForeground(mode.vis, black_pixel);
			/* for speed reasons, we hope for no stray pixels far away ... */
			if ((c=getnumpixels(0,0,120,120))!=x*y || getnumpixels(0,0,x-1,y-1)!=x*y) {
				ggiFillscreen(mode.vis);
				pass=0;
				if (flags&BREAK_UNCONSISTENT) { x=32; break; }
				fprintf(stderr,"Error:Box(0,0,%d,%d); consistency failed (%d pixels measured - should be %d).\n",x,y,c,x*y);
			} else {
				ggiDrawBox(mode.vis,0,0,x,y);
			}
		}
	}
	if (pass) fprintf(stderr,"passed.\n"); else fprintf(stderr,"failed at %x.\n",x);
	pass=1;
	fprintf(stderr,"Consistency: Testing box all alignments ... ");
	for (x=0;x<15;x++) {
		for (y=0;y<=15;y++) {
			int x1,y1;
			for (x1=0;x1<15;x1++) {
				for (y1=0;y1<=15;y1++) {
					ggiSetGCForeground(mode.vis,white_pixel);
					ggiDrawBox(mode.vis,x,y,x1,y1);
					ggiSetGCForeground(mode.vis, black_pixel);
					/* for speed reasons, we hope for no stray pixels far away ... */
					if ((c=getnumpixels(0,0,40,40))!=x1*y1 || getnumpixels(x,y,x+x1-1,y+y1-1)!=x1*y1) {
						ggiFillscreen(mode.vis);
						if (flags&BREAK_UNCONSISTENT) { x=y=x1=y1=32; break; }
						pass=0;
						fprintf(stderr,"Error:Box(%d,%d,%d,%d); consistency failed (%d pixels measured - should be %d).\n",x,y,x1,y1,c,x1*y1);
					} else {
						ggiDrawBox(mode.vis,x,y,x1,y1);
					}
				}
			}
		}
	}
	if (pass) fprintf(stderr,"passed.\n"); else fprintf(stderr,"failed at %x.\n",x);
}

/* Check copyboxs. This exhaustively checks overlapping conditions and such,
 * as they often cause problems.
 */
static void CopyBox(void)
{
	int x,y;
	int pass;

	pass=1;
	fprintf(stderr,"Consistency: Testing copybox - no overlap ... ");
	ggiSetGCForeground(mode.vis, black_pixel);
	ggiFillscreen(mode.vis);

	for (x=0;x<32;x++) {
		for (y=0;y<32;y++) {
			ggiSetGCForeground(mode.vis, black_pixel);
			ggiFillscreen(mode.vis);
			ggiSetGCForeground(mode.vis,white_pixel);
			drawrandom(10,10,x+10-1,y+10-1);
			ggiCopyBox(mode.vis,10,10,x,y,50,10);
			pass=!checkrandom(50,10,x+50-1,y+10-1);
			if (!pass) break;
		}
		if (!pass) break;
	}
	if (pass) fprintf(stderr,"passed.\n"); else fprintf(stderr,"failed at %x.\n",x);

	pass=1;
	fprintf(stderr,"Consistency: Testing copybox - full overlap ... ");
	ggiSetGCForeground(mode.vis, black_pixel);
	ggiFillscreen(mode.vis);

	for (x=0;x<32;x++) {
		for (y=0;y<32;y++) {
			ggiSetGCForeground(mode.vis, black_pixel);
			ggiFillscreen(mode.vis);
			ggiSetGCForeground(mode.vis,white_pixel);
			drawrandom(10,10,x+10-1,y+10-1);
			ggiCopyBox(mode.vis,10,10,x,y,10,10);
			pass=!checkrandom(10,10,x+10-1,y+10-1);
			if (!pass) break;
		}
		if (!pass) break;
	}
	if (pass) fprintf(stderr,"passed.\n"); else fprintf(stderr,"failed at %x.\n",x);

	pass=1;
	fprintf(stderr,"Consistency: Testing copybox - rightdown overlap ... ");
	ggiSetGCForeground(mode.vis, black_pixel);
	ggiFillscreen(mode.vis);

	for (x=0;x<32;x++) {
		for (y=0;y<32;y++) {
			ggiSetGCForeground(mode.vis, black_pixel);
			ggiFillscreen(mode.vis);
			ggiSetGCForeground(mode.vis,white_pixel);
			drawrandom(10,10,x+10-1,y+10-1);
			ggiCopyBox(mode.vis,10,10,x,y,10+x/2,10+y/2);
			pass=!checkrandom(10+x/2,10+y/2,x+10+x/2-1,y+10+y/2-1);
			if (!pass) break;
		}
		if (!pass) break;
	}
	if (pass) fprintf(stderr,"passed.\n"); else fprintf(stderr,"failed at %x.\n",x);

	pass=1;
	fprintf(stderr,"Consistency: Testing copybox - leftdown overlap ... ");
	ggiSetGCForeground(mode.vis, black_pixel);
	ggiFillscreen(mode.vis);

	for (x=0;x<32;x++) {
		for (y=0;y<32;y++) {
			ggiSetGCForeground(mode.vis, black_pixel);
			ggiFillscreen(mode.vis);
			ggiSetGCForeground(mode.vis,white_pixel);
			drawrandom(10+x/2,10,x+x/2+10-1,y+10-1);
			ggiCopyBox(mode.vis,10+x/2,10,x,y,10,10+y/2);
			pass=!checkrandom(10,10+y/2,x+10-1,y+10+y/2-1);
			if (!pass) break;
		}
		if (!pass) break;
	}
	if (pass) fprintf(stderr,"passed.\n"); else fprintf(stderr,"failed at %x.\n",x);

	pass=1;
	fprintf(stderr,"Consistency: Testing copybox - rightup overlap ... ");
	ggiSetGCForeground(mode.vis, black_pixel);
	ggiFillscreen(mode.vis);

	for (x=0;x<32;x++) {
		for (y=0;y<32;y++) {
			ggiSetGCForeground(mode.vis, black_pixel);
			ggiFillscreen(mode.vis);
			ggiSetGCForeground(mode.vis,white_pixel);
			drawrandom(10,10+y/2,x+10-1,y+y/2+10-1);
			ggiCopyBox(mode.vis,10,10+y/2,x,y,10+x/2,10);
			pass=!checkrandom(10+x/2,10,x+10+x/2-1,y+10-1);
			if (!pass) break;
		}
		if (!pass) break;
	}
	if (pass) fprintf(stderr,"passed.\n"); else fprintf(stderr,"failed at %x.\n",x);

	pass=1;
	fprintf(stderr,"Consistency: Testing copybox - leftup overlap ... ");
	ggiSetGCForeground(mode.vis, black_pixel);
	ggiFillscreen(mode.vis);

	for (x=0;x<32;x++) {
		for (y=0;y<32;y++) {
			ggiSetGCForeground(mode.vis, black_pixel);
			ggiFillscreen(mode.vis);
			ggiSetGCForeground(mode.vis,white_pixel);
			drawrandom(10+x/2,10+y/2,x+x/2+10-1,y+10-1+y/2);
			ggiCopyBox(mode.vis,10+x/2,10+y/2,x,y,10,10);
			pass=!checkrandom(10,10,x+10-1,y+10-1);
			if (!pass) break;
		}
		if (!pass) break;
	}
	if (pass) fprintf(stderr,"passed.\n"); else fprintf(stderr,"failed at %x.\n",x);

}


/* This is an array of all available tests. It is used for parameter
 * checking and contains which checks to run.
 */
static struct test
{
	const char *name;
	void (*func)(void);
	int active;
} tests[]=
{	{"BasicConsistency", BasicConsistency, 1},
		/* Enabled by default to keep it from stupid tries when
		   getpixel is broken. */
	{"ColorWrap",	ColorWrap,	0},
	{"Hline",	Hline,		0},
	{"Vline",	Vline,		0},
	{"Line",	Line,		0},
	{"Box",		Box,		0},
	{"CopyBox",	CopyBox,	0},
	{ NULL,NULL,0 }
};

/* Give a usage summary
 */
static void usage(const char *prog)
{
	fprintf(stderr,"Usage:\n\n"
		       "%s [-flags] [--tests]\n\n"
		       "Supported flags are :\n",prog);

	fprintf(stderr,	"-b bail out if any consistency check fails\n"
			"-? list available tests\n");

	exit(1);
}

/* Give a usage summary of the available tests
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

/* Parse arguments
 */
static int parse_args(int argc,char **argv)
{
	int x,testnum;

	for (x=1;x<argc;x++) {
		if (*argv[x]=='-') {
			switch(argv[x][1]) {

				case 'b': flags|=BREAK_UNCONSISTENT;break;
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

/* Set up the mode to work on. We set the default mode. Use GGI_DEFMODE to
 * override the target default.
 */
static int setup_mode(void)
{
	int err;
	ggi_color map[256];
	ggi_mode gmode;

	if ((mode.vis=ggNewStem(libgii, libggi, NULL)) == NULL) {
		fprintf(stderr,
			"unable to create stem, exiting.\n");
		exit(1);
	}

	if (ggiOpen(mode.vis, NULL) < 0) {
		fprintf(stderr,
			"unable to open default visual, exiting.\n");
		exit(1);
	}

	if ((err=ggiSetSimpleMode(mode.vis, GGI_AUTO, GGI_AUTO, GGI_AUTO,
				  GT_AUTO)) == 0) {
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

	map[0].r = 0x0000;
	map[0].g = 0x0000;
	map[0].b = 0x0000;
	black_pixel = ggiMapColor(mode.vis, &map[0]);
	map[0].r = 0xFFFF;
	map[0].g = 0xFFFF;
	map[0].b = 0xFFFF;
	white_pixel = ggiMapColor(mode.vis, &map[0]);

	return 0;
}

/* The main function. Check arguments, then run the tests.
 */
int
main(int argc,char **argv)
{
	int testnum;

	if (parse_args(argc,argv)) return 1;

	srand((unsigned)time(NULL));
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
