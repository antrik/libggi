/* $Id: showaccel.c,v 1.11 2006/03/21 12:38:16 pekberg Exp $
******************************************************************************

   showaccel.c

   Written in 1998 by Andreas Beck	[becka@ggi-project.org]

   This software is placed in the public domain and can be used freely
   for any purpose. It comes without any kind of warranty, either
   expressed or implied, including, but not limited to the implied
   warranties of merchantability or fitness for a particular purpose.
   Use it at your own risk. the author is not responsible for any damage
   or consequences raised by use or inability to use this program.

******************************************************************************

   This programs gives an estimate about the amount of acceleration in a
   target. It measures the speed of drawing lines using drawbox as compared
   to drawing them with multiple horizontal lines.

   As a rough guideline, here are values from to targets as measured on the
   authors system:

   GGI_DISPLAY="x"    ./showaccel  Ratio :   134479 :   114610 = 1.173362
   GGI_DISPLAY="xlib" ./showaccel  Ratio :   226929 :    23071 = 9.836115

   What you can derive from the above example is:

   Ratios of up to around 1.2 are not indicating a significant drawing
   acceleration. This is just calling overhead
   Running over xlib _dramatically_ shows another kind of "calling overhead",
   as the xlib target produces a network call for each primitive call.
   So be careful when interpreting the results.

******************************************************************************
*/

#include "config.h"
#include <ggi/gg.h>
#include <ggi/gii.h>
#include <ggi/ggi.h>

#include <stdlib.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif

static ggi_visual_t vis;

int
myKbhit(ggi_visual_t vis)
{
	struct timeval t={0,0};

	return (giiEventPoll((gii_input)vis, emKeyPress | emKeyRepeat, &t)
		!= emZero);
}

/* wrapper function for easy porting. returns a number between
 * 0 and max-1 (including borders).
 */
static int randnum(int max)
{
	return (rand()%max);
}

/* Draw a box from individual horizontal lines. This effectively disables
 * acceleration code for boxes to some extent. Depending on the underlying
 * implementation, this may not be a good solution.
 */

static inline int slow_drawbox(int x,int y,int w,int h)
{
	while(h--) ggiDrawHLine(vis,x,y++,w);
	return 0;
}

/* The main function. Draw boxes with two different methods and measure the
 * difference in performance.
 */
int main(int argc, char *argv[])
{
	/* Loop counters and other helpers
	 */
	int c,y;

	/* Used for timing :
	 */
	struct timeval tv;

	/* The two performance counters
	 */
	int c1,c2;

	/* The visible screen sizes
	 */
	int sx,sy;

	/* The font size
	 */
	int tx,ty;

	/* The palette and wanted color table.
	 */
	ggi_color pal[256];

	/* The color translation table.
	 */
	ggi_pixel paletteval[256];

	/* Initialize GII/GGI. Error out, if it fails.
	 */
	if (giiInit() != 0) {
		fprintf(stderr, "%s: unable to initialize LibGII, exiting.\n",
			argv[0]);
		exit(1);
	}
	if (ggiInit() != 0) {
		fprintf(stderr, "%s: unable to initialize LibGGI, exiting.\n",
			argv[0]);
		exit(1);
	}
	if ((vis=ggNewStem()) == NULL) {
		fprintf(stderr,
			"%s: unable to create stem, exiting.\n",
			argv[0]);
		exit(1);
	}
	if (giiAttach(vis) != 0) {
		fprintf(stderr,
			"%s: unable to attach gii api to stem, exiting.\n",
			argv[0]);
		exit(1);
	}
	if (ggiAttach(vis) != 0) {
		fprintf(stderr,
			"%s: unable to attach ggi api to stem, exiting.\n",
			argv[0]);
		exit(1);
	}
	/* Open the default GGI visual. Exit on error.
	 */
	if (ggiOpen(vis, NULL) != 0) {
		ggiExit();
		giiExit();
		fprintf(stderr,
			"%s: unable to open default visual, exiting.\n",
			argv[0]);
		exit(1);
	}

        /* If mode setup fails, fall through to the end.
         */
	if (ggiSetSimpleMode(vis, GGI_AUTO, GGI_AUTO, GGI_AUTO, GT_AUTO)
	    == 0) {
		/* We asked for automatic setup. So we better ask what we got.
		 * We will need the visible dimensions.
		 */
		{
			ggi_mode mode;
			ggiGetMode(vis,&mode);
			sx=mode.visible.x;
			sy=mode.visible.y;
		}

		/* Get size of visible character cell
		 */
		ggiGetCharSize(vis, &tx, &ty);

		/* Clear the screen
		 */
		ggiSetGCForeground(vis,0);
		ggiFillscreen(vis);

		/* Set up a colorful palette.
		 */
		for(c=0;c<256;c++) {
			pal[c].r=c*256;
			pal[c].g=256*(255-c);
			pal[c].b=(128-abs(128-c))*7*64;
		}
		/* And send it to the visual. In case we are in a
		 * non-palettized mode, this simply fails.
		 */
		ggiSetPalette(vis,0,256,pal);

		/* However this call ensures, that we always get a set of
		 * "handles" for the colors we want. This makes programs
		 * designed for use with 8 bit palette mode run on the
		 * other graphtypes as well. You just get back the color
		 * value to use for the closest available color.
		 */
		for(c=0;c<256;c++)
			paletteval[c]=ggiMapColor(vis,&pal[c]);

		/* Set foreground to color 255 (red) and print the headlines.
		 */
		ggiSetGCForeground(vis,paletteval[255]);
		ggiPuts(vis,0,0,"Accelerated");
		ggiPuts(vis,sx/2,0,"Mundane");

		/* Set up the counters for both methods.
		 */
		c1=c2=0;

		/* Do a million runs max.
		 */
		for(y=0;y<1000000;y++) {

			/* Set some arbitrary foreground color
			 */
			ggiSetGCForeground(vis,paletteval[randnum(256)]);

			/* Check the time
			 */
			ggCurTime(&tv);

			/* If we are on an odd 1/10th second, we draw accelerated,
			 * if we are on an even one, we draw "mundane".
			 * This assumes, that the time needed to execute a single
			 * drawbox is neglectable compared to 1/10th second.
			 * Otherwise a systematic error my build up. Keep that in
			 * mind for slow systems. Eventually use whole seconds.
			 */
			if ((tv.tv_usec/100000)&1) {
				ggiDrawBox(vis, randnum(sx/4),randnum(sy/2-ty)+ty,
						randnum(sx/4),randnum(sy/2-ty));
				c1++;
			} else {
				slow_drawbox(randnum(sx/4)+sx/2,randnum(sy/2-ty)+ty,
					     randnum(sx/4)     ,randnum(sy/2-ty));
				c2++;
			}

			/* If we have drawn a significant amount of boxes,
			 * give an intermediate estimate, to keep the user
			 * happy and give an indication of how long it will
			 * take, or when to break things.
			 */
			if ((y&1023)==0) {
				char str[20];

				/* Select red again.
				*/
				ggiSetGCForeground(vis,paletteval[255]);

				/* Make strings with the numbers and print them.
				 */
				sprintf(str,"%8d",c1);
				ggiPuts(vis,sx  /4,0,str);
				sprintf(str,"%8d",c2);
				ggiPuts(vis,sx*3/4,0,str);

				/* In case the user hit a key, he wants to quit.
				 */
				if (myKbhit(vis)) break;
			}
		}
		/* Print the measured data
		 */
		if (c2==0) c2=1;
		printf("Ratio : %8d : %8d = %2f\n",c1,c2,(double)c1/(double)c2);
	}

	/* Close down LibGGI.
	 */
	ggiClose(vis);
        ggDelStem(vis);

        ggiExit();
        giiExit();
	return(0);
}
