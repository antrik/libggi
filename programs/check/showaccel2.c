/* $Id: showaccel2.c,v 1.9 2005/07/29 16:30:53 soyt Exp $
******************************************************************************

   showaccel2.c - same as showaccel.c but uses fork() instead of
		  "cooperative multitasking"

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
TODO: notify on exit (terminate with single keystroke then print final ratio.
	Also account for different timeslices).
*/

#include <ggi/ggi.h>
#include <ggi/gg.h>

#include <stdlib.h>
#include <unistd.h>

#include "config.h"

static ggi_visual_t vis;

/* wrapper function for easy porting. returns a number between
 * 0 and max-1 (including borders).
 */
static int randnum(int max)
{
	return (rand() % max);
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

/* The main function. Draw boxes with two different methods from two different
 * processes.
 */
int main(int argc, char *argv[])
{
	/* Loop counters and other helpers
	 */
	int c,y;

	/* The two performance counters
	 */
	int c1,c2;

	/* Pipe to pass result string cnt from child to parent
	 */
	int p_fd[2];
	char cnt[8] = "\0\0\0\0\0\0\0\0";

	/* The visible screen sizes
	 */
	int sx,sy;

	/* Size of visible character cell
	 */
	int tx,ty;

	/* The palette and wanted color table.
	 */
	ggi_color pal[256];

	/* The color translation table.
	 */
	ggi_pixel paletteval[256];

	/* Initialize GGI. Error out, if it fails.
	 */
	if (ggiInit() != 0) {
		fprintf(stderr, "%s: unable to initialize LibGGI, exiting.\n",
			argv[0]);
		exit(1);
	}
	/* Open the default GGI visual. Exit on error.
	 */
	if ((vis=ggiOpen(NULL)) == NULL) {
		ggiExit();
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

		/* Setup pipe used to report the results
		 */
		pipe(p_fd);

		switch(fork()) {
			case -1:
				ggiClose(vis);
				ggiExit();
				fprintf(stderr,
					"%s: fork failed, exiting.\n",
					argv[0]);
				exit(1);
				break;
			case 0:
				/* close stdout and use p_fd[1] to write
				 */
				close(1);
				dup(p_fd[1]);
				close(p_fd[0]); /* child does not pipe_read */
				for(y=0;y<1000000;y++) {
					ggiSetGCForeground(vis,randnum(256));
					ggiDrawBox(vis, randnum(sx/4),randnum(sy/2-ty/2)+ty,
							randnum(sx/4),randnum(sy/2-ty/2));
					c1++;
					/* Print the counter once in a while */
					if ((y&1023)==0)
					{
						char str[20];

						/* Select red again.
						 */
						ggiSetGCForeground(vis,
							paletteval[250]);

						/* Make string with the number
						 * and print it.
						 */
						sprintf(str,"%8d",c1);
						ggiPuts(vis,sx  /4,0,str);
					}
					/* bail out if user hit a key
					 */
					if (ggiKbhit(vis))
					{
						break;
					}
				}
				printf("%d",c1);
				fprintf(stderr,
					"child exiting at iteration %d\n",c1);
				exit(0);
				break;
			default:
				/* close stdin and use p_fd[0] to read
				 */
				close(0);
				dup(p_fd[0]);
				close(p_fd[1]); /* parent does not pipe_write */
				for(y=0;y<1000000;y++) {
					ggiSetGCForeground(vis,randnum(256));
				slow_drawbox(randnum(sx/4)+sx/2,randnum(sy/2-ty/2)+ty,
					     randnum(sx/4)     ,randnum(sy/2-ty/2));
					c2++;
					/* Print the counter once in a while
					 */
					if ((y&127)==0)
					{
						char str[20];

						/* Select red again.
						 */
						ggiSetGCForeground(vis,
							paletteval[255]);

						/* Make string with the number
						 * and print it.
						 */
						sprintf(str,"%8d",c2);
						ggiPuts(vis,sx*3/4,0,str);
					}
					/* bail out if user hit a key
					 */
					if (ggiKbhit(vis))
					{
						break;
					}
				}
				fprintf(stderr,
					"parent exiting at iteration %d\n",c2);
				read(p_fd[0], cnt, 7);
				break;
		}
		/* Print the measured data
		 */
		c1 = atoi(cnt);
		if (c2==0) c2=1;
		printf("Ratio : %8d : %8d = %2f\n",c1,c2,(double)c1/(double)c2);
	}

	/* Close down LibGGI.
	 */
	ggiClose(vis);
	ggiExit();
	return(0);
}
