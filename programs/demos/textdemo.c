/* $Id: textdemo.c,v 1.1 2001/05/12 23:03:41 cegger Exp $
******************************************************************************

   textdemo.c - demonstrate text mode on apropriate targets

   Authors:	1998 Andreas Beck		[becka@ggi-project.org]

   This software is placed in the public domain and can be used freely
   for any purpose. It comes without any kind of warranty, either
   expressed or implied, including, but not limited to the implied
   warranties of merchantability or fitness for a particular purpose.
   Use it at your own risk. the author is not responsible for any damage
   or consequences raised by use or inability to use this program.

******************************************************************************
*/

#include <stdio.h>
#include <string.h>

#include <ggi/ggi.h>

ggi_visual_t vis;

int
main(int argc, char *argv[]) {
	const char hex[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };
	int a, b;
	char *hello = "Hello World";
	ggi_mode mode;

	if (ggiInit() != 0) {
		fprintf(stderr,
			"%s: unable to initialize LibGGI, exiting.\n",
			argv[0]);
		exit(1);
	}
	if ((vis=ggiOpen(NULL)) == NULL) {
		fprintf(stderr,
			"%s: unable to open default visual, retrying with terminfo.\n",
			argv[0]);
		if ((vis=ggiOpen("terminfo",NULL)) == NULL) {
			fprintf(stderr,
				"%s: unable to open terminfo, exiting.\n",
				argv[0]);
			exit(1);
		}
	}
	if (ggiSetTextMode(vis, 80, 25, 80, 25, 9, 14, GT_TEXT16) != 0) {
		fprintf(stderr,
			"%s: unable to set text mode, retrying with terminfo.\n",
			argv[0]);
		ggiClose(vis);
		if ((vis=ggiOpen("terminfo",NULL)) == NULL) {
			fprintf(stderr,
				"%s: unable to open terminfo, exiting.\n",
				argv[0]);
			exit(1);
		}
		if (ggiSetTextMode(vis, 80, 25, 80, 25, 9, 14, GT_TEXT16) != 0) {
			fprintf(stderr,
				"%s: unable to set text mode, exiting.\n",
				argv[0]);
			ggiClose(vis);
			exit(1);
		}
	}		
	ggiSetGCForeground(vis,1<<8);

	for ( a = 0 ; a < 25 ; a++ ) {
		for ( b = 0 ; b < 80 ; b++ ) {
			ggiPutc(vis, b, a, 'x');
		}
	}

	ggiSetGCForeground(vis,9<<8);
	ggiPuts(vis, 34, 13, "Hello World");
	ggiPuts(vis, 34, 14, hello);
	ggiFlush(vis);

	ggiGetc(vis);
       
	for ( a = 0 ; a <= 0xff ; a++ ) {
		ggiSetGCForeground(vis, a << 8);
		for ( b = 0 ; b < 25 ; b++ ) {
			ggiPuts(vis, a&31, b, "Testing all colors ... ");
		}
		ggiFlush(vis);
		ggUSleep(20000);
	}

	ggiGetc(vis);

	ggiSetGCForeground(vis,1<<8);
	for ( a = 0 ; a < 25 ; a++ ) {
		for ( b = 0 ; b < 80 ; b++ ) {
			ggiPutc(vis, b, a, 'x');
		}
	}

	ggiSetGCForeground(vis, 7<<8);
	ggiPuts(vis, 0, 1, "BG0");
	for ( a = 1 ; a < 16 ; a++ ) {
		char str[4];
		sprintf(str, "BG%c", hex[a]);
		ggiSetGCForeground(vis, a<<12);
		ggiPuts(vis, 0, a+1, str);
	}

	ggiSetGCForeground(vis, 15<<12);
	ggiPuts(vis, 3, 0, "FG0");
	for ( a = 1 ; a < 16 ; a++ ) {
		char str[4];
		sprintf(str, "FG%c", hex[a]);
		ggiSetGCForeground(vis,a<<8);
		ggiPuts(vis, (a+1) * 3, 0, str);
	}

	for ( a = 0 ; a < 16 ; a++ ) {
		for ( b = 0 ; b < 16 ; b++ ) {
			ggiSetGCForeground(vis,(a<<12)|(b<<8));
			ggiPuts(vis, b*3 + 3, a + 1, "###");
		}
	}

	ggiFlush(vis);

	sleep(1);

	ggiSetGCForeground(vis,4<<8);
	ggiPuts(vis, 56, 5, "  LibGGI dynamic  ");

	ggiFlush(vis);

	sleep(1);

	ggiGetMode(vis, &mode);

	b = mode.visible.y * mode.dpp.y;

#if 0	/* This requires linking to the misc extension and initializing it. */
	for ( a = b ; a >= 0 ; a-- ) {
		ggiSetSplitline(vis, a);
		ggUSleep(10000);
	}
	for ( a = 0 ; a <= b ; a++ ) {
		ggiSetSplitline(vis, a);
		ggUSleep(10000);
	}
#endif

	ggiGetc(vis);

	ggiClose(vis);
	ggiExit();
	return 0;
}
