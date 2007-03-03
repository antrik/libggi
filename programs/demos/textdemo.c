/* $Id: textdemo.c,v 1.14 2007/03/03 19:36:16 cegger Exp $
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

#include "config.h"
#include <ggi/gg.h>
#include <ggi/gii.h>
#include <ggi/ggi.h>

#include <stdio.h>
#include <string.h>
#ifdef HAVE_UNISTD_H
# include <unistd.h>
#endif


static ggi_visual_t vis;

static int
myGetc(ggi_visual_t _vis)
{
	gii_event ev;

	/* Block until we get a key. */
	giiEventRead(_vis, &ev, emKeyPress | emKeyRepeat);

	return ev.key.sym;
}

int main(int argc, const char *argv[])
{
	const char hex[] = { '0', '1', '2', '3', '4', '5', '6', '7',
		'8', '9', 'A', 'B', 'C', 'D', 'E', 'F'
	};
	int a, b;
	const char *hello = "Hello World";
	ggi_mode mode;

	if (giiInit() != 0) {
		fprintf(stderr,
			"%s: unable to initialize LibGII, exiting.\n",
			argv[0]);
		exit(1);
	}
	if (ggiInit() != 0) {
		fprintf(stderr,
			"%s: unable to initialize LibGGI, exiting.\n",
			argv[0]);
		exit(1);
	}
	if ((vis = ggNewStem(NULL)) == NULL) {
		fprintf(stderr,
			"%s: unable to create stem, exiting.\n",
			argv[0]);
		exit(1);
	}
	if (giiAttach(vis) < 0) {
		fprintf(stderr,
			"%s: unable to attach LibGII, exiting.\n",
			argv[0]);
		exit(1);
	}
	if (ggiAttach(vis) < 0) {
		fprintf(stderr,
			"%s: unable to attach LibGGI, exiting.\n",
			argv[0]);
		exit(1);
	}
	if (ggiOpen(vis, NULL) < 0) {
		fprintf(stderr,
			"%s: unable to open default visual, retrying with terminfo.\n",
			argv[0]);
		if (ggiOpen(vis, "terminfo", NULL) < 0) {
			fprintf(stderr,
				"%s: unable to open terminfo, exiting.\n",
				argv[0]);
			exit(1);
		}
	}
	ggiCheckTextMode(vis, GGI_AUTO, GGI_AUTO, GGI_AUTO, GGI_AUTO,
			 GGI_AUTO, GGI_AUTO, GT_TEXT16, &mode);
	if (ggiSetMode(vis, &mode) != 0) {
		fprintf(stderr,
			"%s: unable to set text mode, retrying with terminfo.\n",
			argv[0]);
		ggiClose(vis);
		if (ggiOpen(vis, "terminfo", NULL) < 0) {
			fprintf(stderr,
				"%s: unable to open terminfo, exiting.\n",
				argv[0]);
			exit(1);
		}
		ggiCheckTextMode(vis, GGI_AUTO, GGI_AUTO, GGI_AUTO, GGI_AUTO,
				 GGI_AUTO, GGI_AUTO, GT_TEXT16, &mode);
		if (ggiSetMode(vis, &mode) != 0) {
			fprintf(stderr,
				"%s: unable to set text mode, exiting.\n",
				argv[0]);
			ggiClose(vis);
			exit(1);
		}
	}
	ggiSetGCForeground(vis, 1 << 8);

	ggiGetMode(vis, &mode);
	if (mode.visible.y < 17 || mode.visible.x < 52) {
		fprintf(stderr,
			"Need at least 52x17 chars for this demo\n");
		ggDelStem(vis);
		ggiExit();
		giiExit();
		exit(-1);
	}

	for (a = 0; a < mode.visible.y; a++) {
		for (b = 0; b < mode.visible.x; b++) {
			ggiPutc(vis, b, a, 'x');
		}
	}


	ggiSetGCForeground(vis, 9 << 8);
	ggiPuts(vis, (mode.visible.x - 7) / 2, mode.visible.y / 2,
		"Hello World");
	ggiPuts(vis, (mode.visible.x - 7) / 2, mode.visible.y / 2, hello);
	ggiFlush(vis);

	myGetc(vis);

	for (a = 0; a <= 0xff; a++) {
		ggiSetGCForeground(vis, (unsigned) (a) << 8);
		for (b = 0; b < mode.visible.y; b++) {
			ggiPuts(vis, a & 31, b, "Testing colors");
		}
		ggiFlush(vis);
		ggUSleep(20000);
	}

	myGetc(vis);

	ggiSetGCForeground(vis, 1 << 8);
	for (a = 0; a < mode.visible.y; a++) {
		for (b = 0; b < mode.visible.y; b++) {
			ggiPutc(vis, b, a, 'x');
		}
	}

	ggiSetGCForeground(vis, 7 << 8);
	ggiPuts(vis, 0, 1, "BG0");
	for (a = 1; a < 16; a++) {
		char str[4];
		sprintf(str, "BG%c", hex[a]);
		ggiSetGCForeground(vis, (unsigned) (a) << 12);
		ggiPuts(vis, 0, a + 1, str);
	}

	ggiSetGCForeground(vis, 15 << 12);
	ggiPuts(vis, 3, 0, "FG0");
	for (a = 1; a < 16; a++) {
		char str[4];
		sprintf(str, "FG%c", hex[a]);
		ggiSetGCForeground(vis, (unsigned) (a) << 8);
		ggiPuts(vis, (a + 1) * 3, 0, str);
	}

	for (a = 0; a < 16; a++) {
		for (b = 0; b < 16; b++) {
			ggiSetGCForeground(vis,
					   ((unsigned) (a) << 12) |
					   ((unsigned) (b) << 8));
			ggiPuts(vis, b * 3 + 3, a + 1, "###");
		}
	}

	ggiFlush(vis);

	ggUSleep(1000);

	ggiSetGCForeground(vis, 4 << 8);
	ggiPuts(vis, 56, 5, "  LibGGI dynamic  ");

	ggiFlush(vis);

	ggUSleep(1000);

	ggiGetMode(vis, &mode);

	b = mode.visible.y * mode.dpp.y;

#if 0				/* This requires linking to the misc extension and initializing it. */
	for (a = b; a >= 0; a--) {
		ggiSetSplitline(vis, a);
		ggUSleep(10000);
	}
	for (a = 0; a <= b; a++) {
		ggiSetSplitline(vis, a);
		ggUSleep(10000);
	}
#endif

	myGetc(vis);

	ggDelStem(vis);
	ggiExit();
	giiExit();
	return 0;
}
