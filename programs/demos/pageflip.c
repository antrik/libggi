/* $Id: pageflip.c,v 1.5 2004/01/08 21:27:12 skids Exp $
******************************************************************************

   pageflip.c - test the multiple buffering functions of LibGGI

   Authors:	1998 	  Steve Cheng	[steve@ggi-project.org]

   This software is placed in the public domain and can be used freely
   for any purpose. It comes without any kind of warranty, either
   expressed or implied, including, but not limited to the implied
   warranties of merchantability or fitness for a particular purpose.
   Use it at your own risk. the author is not responsible for any damage
   or consequences raised by use or inability to use this program.

******************************************************************************

   This is a demonstration of LibGGI's functions and can be used as a 
   reference programming example.

   Usage: ./pageflip [mode-spec]

   Example: ./pageflip F2
   Example: ./pageflip 320x200F8[8]

******************************************************************************
*/

/* This is needed for the HAVE_* macros */
#include "config.h"
#ifndef HAVE_RANDOM
# define random		rand
# define srandom	srand
#endif

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <ggi/ggi.h>
#include <math.h>

ggi_color black = { 0x0000, 0x0000, 0x0000 };
ggi_color white = { 0xffff, 0xffff, 0xffff };
ggi_color blue = { 0x0000, 0x0000, 0xffff };
ggi_color yellow = { 0xffff, 0xffff, 0x0000 };

static void animate_one_frame(ggi_visual_t vis, int x, int y, int w, int h,
			      ggi_pixel * buf, int x2)
{

	ggiDrawHLine(vis, x, y, w);
	ggiDrawHLine(vis, x, y + h - 1, w);
	ggiDrawVLine(vis, x, y, h);
	ggiDrawVLine(vis, x + w - 1, y, h);
	ggiDrawBox(vis, x + 2, y + 2, w - 4, h - 4);
	ggiGetBox(vis, x + 5, y + 5, w - 10, h / 2 - 7, buf);
	ggiPutBox(vis, x + 5, y + 5, w - 10, h / 2 - 7, buf);
	ggiCopyBox(vis, x + 5, y + h / 2, w - 10, h / 2 - 5, x + 5,
		   y + h / 2);
	ggiPuts(vis, x + 7, y + 7, "Get/Put");
	ggiPuts(vis, x + 7, y + h / 2 + 2, "Copy");

	ggiDrawBox(vis, x2, y, 5, 5);

}

static void animate(ggi_visual_t vis, ggi_mode * mode, int j)
{
	int f, w, h;
	ggi_pixel *buf;
	ggi_color c1, c2;

	c1.r = ((sin((double) j / 100) + 1) * 32768);
	c1.g = ((sin(2.094394 + (double) j / 100) + 1) * 32768);
	c1.b = ((sin(4.188789 + (double) j / 100) + 1) * 32768);
	c1.a = 0;

	c2.r = ((sin(3.141592 + (double) j / 100) + 1) * 32768);
	c2.g = ((sin(5.235986 + (double) j / 100) + 1) * 32768);
	c2.b = ((sin(7.330381 + (double) j / 100) + 1) * 32768);
	c2.a = 0;

	ggiSetGCForeground(vis, ggiMapColor(vis, &c1));
	ggiSetGCBackground(vis, ggiMapColor(vis, &c2));

	w = mode->virt.x / mode->frames;
	h = mode->virt.y / 5;

	buf = malloc(w * h * GT_SIZE(mode->graphtype));
	if (buf == NULL)
		return;

	ggiSetOrigin(vis, j % (mode->virt.x - mode->visible.x + 1),
		     j % (mode->virt.y - mode->visible.y + 1));

	for (f = 0; f < mode->frames; f++) {
		int x, y, yspan;

		x = f * w;
		yspan = mode->virt.y - h;
		y = (j + (yspan * 2 * (f / mode->frames))) % (2 * yspan);
		y += yspan * 2 * f / mode->frames;
		y %= yspan * 2;
		if (y >= yspan)
			y = yspan * 2 - y;
		x += (sin(3.14159264 * 4 * y / yspan) + 1) * (w * 1 / 10);
		ggiSetWriteFrame(vis, f);
		ggiSetReadFrame(vis, (f + 1) % mode->frames);
		animate_one_frame(vis, x, y, w * 4 / 5, h, buf,
				  (x + w * (mode->frames - 1) + w / 2) %
				  mode->virt.x);
	}

	free(buf);
}



int main(int argc, char *argv[])
{
	ggi_visual_t vis;
	ggi_mode mode;
	int i, j, cx, cy, c;
	char buf[80];

	/* Set up the random number generator */
	srandom((unsigned)time(NULL));

	/* Initialize LibGGI */
	if (ggiInit()) {
		fprintf(stderr, "Cannot initialize LibGGI!\n");
		return 1;
	}

	/* Open default visual */
	vis = ggiOpen(NULL);
	if (!vis) {
		fprintf(stderr, "Cannot open default visual!\n");
		ggiExit();
		return 1;
	}

	/* Set visual to async mode (drawing not immediate) */
	ggiSetFlags(vis, GGIFLAG_ASYNC);

	/* Set default mode, but with multiple buffering */
	if (argc > 1) {
		ggiParseMode(argv[1], &mode);
	} else {
		ggiParseMode("", &mode);
		if (mode.frames < 2) mode.frames = 2;
	}

	if (ggiSetMode(vis, &mode)) {
		fprintf(stderr, "Cannot set mode!\n");
		ggiClose(vis);
		ggiExit();
		return 1;
	}

	ggiGetCharSize(vis, &cx, &cy);

	/* Setup palette */
	if (GT_SCHEME(mode.graphtype) == GT_PALETTE) {
		ggiSetColorfulPalette(vis);
	}

	/* Write something into each frame */
	for (i = 0; i < mode.frames; i++) {

		if (ggiSetWriteFrame(vis, i)) {
			fprintf(stderr, "Cannot set write frame!\n");
			ggiClose(vis);
			ggiExit();
			return 1;
		}

		ggiSetGCBackground(vis, ggiMapColor(vis, &white));
		ggiSetGCForeground(vis, ggiMapColor(vis, &white));
		ggiFillscreen(vis);
	}

	/* Clip a small border so that clipping can be verified. */
	ggiSetGCClipping(vis, 5, 5, mode.virt.x - 5, mode.virt.y - 5);

	/* Write something into each frame */
	for (i = 0; i < mode.frames; i++) {

		ggiSetWriteFrame(vis, i);

		ggiSetGCBackground(vis, ggiMapColor(vis, &black));
		ggiSetGCForeground(vis, ggiMapColor(vis, &black));
		ggiFillscreen(vis);

		sprintf(buf, "Hello World #%d!", i);

		for (j = 0; j < mode.virt.y; j += cy) {

			ggi_color col;

			int x = random() % mode.virt.x;

			int h = (random() & 0x7fff) + 0x8000;
			int l = (random() & 0x7fff);

			/* Use different colors for different frames */
			col.r = ((i + 1) & 1) ? h : l;
			col.g = ((i + 1) & 2) ? h : l;
			col.b = ((i + 1) & 4) ? h : l;

			ggiSetGCForeground(vis, ggiMapColor(vis, &col));
			ggiPuts(vis, x, j, buf);
		}
		/* Flush commands before proceeding to the next frame */
		ggiFlush(vis);
	}

	/* Cycle through frames */
	i = 0;
	j = 0;
	do {
		if (ggiSetDisplayFrame(vis, i)) {
			ggiPanic("Cannot set display frame!\n");
		}
		/* Flush command before waiting for input */
		ggiFlush(vis);

		/* Wait */
		c = GIIK_VOID;
		do {
			struct timeval tv = { 0, 0 };
			int key;

			key = ggiEventPoll(vis, emKeyPress, &tv);
			if (key & emKeyPress)
				c = ggiGetc(vis);
			ggUSleep(50000);
			animate(vis, &mode, j);
			j++;
		} while (c == GIIK_VOID || GII_KTYP(c) == GII_KT_MOD);

		i = (i + 1) % mode.frames;

	} while (c != 'q' && c != 'Q' && c != 'x' && c != 'X' &&
		 c != GIIUC_Escape);

	ggiClose(vis);
	ggiExit();
	return 0;
}
