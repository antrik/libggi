/* $Id: pageflip.c,v 1.1 2001/05/12 23:03:37 cegger Exp $
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
#include <ggi/ggi.h>

ggi_color black  = { 0x0000, 0x0000, 0x0000 };
ggi_color white  = { 0xffff, 0xffff, 0xffff };
ggi_color blue   = { 0x0000, 0x0000, 0xffff };
ggi_color yellow = { 0xffff, 0xffff, 0x0000 };


int main(int argc, char *argv[])
{
	ggi_visual_t	vis;
	ggi_mode	mode;
	int		i, j, cx, cy, c;
	char		buf[80];

	/* Set up the random number generator */
	srandom(time(NULL));

	/* Initialize LibGGI */
	if(ggiInit()) {
		fprintf(stderr, "Cannot initialize LibGGI!\n");
		return 1;
	}
	
	/* Open default visual */
	vis = ggiOpen(NULL);
	if(!vis) {
		fprintf(stderr, "Cannot open default visual!\n");
		ggiExit();
		return 1;
	}
	
	/* Set visual to async mode (drawing not immediate) */
	ggiSetFlags(vis, GGIFLAG_ASYNC);

	/* Set default mode, but with multiple buffering */
	if (argc>1) {
		ggiParseMode(argv[1], &mode);
	} else {
		ggiParseMode("", &mode);
		mode.frames = 2;
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
	for(i = 0; i < mode.frames; i++) {

		if(ggiSetWriteFrame(vis, i)) {
			fprintf(stderr, "Cannot set write frame!\n");
			ggiClose(vis);
			ggiExit();
			return 1;
		}
		
		ggiSetGCBackground(vis, ggiMapColor(vis, &black));
		ggiSetGCForeground(vis, ggiMapColor(vis, &black));
		ggiFillscreen(vis);

		sprintf(buf, "Hello World #%d!", i);

		for (j=0; j < mode.visible.y; j += cy) {

			ggi_color col;

			int x = random() % mode.visible.x;

			int h = (random() & 0x7fff) + 0x8000;
			int l = (random() & 0x7fff);

			/* Use different colors for different frames */
			col.r = ((i+1) & 1) ? h : l;
			col.g = ((i+1) & 2) ? h : l;
			col.b = ((i+1) & 4) ? h : l;

			ggiSetGCForeground(vis, ggiMapColor(vis, &col));
			ggiPuts(vis, x, j, buf);
		}
		/* Flush commands before proceeding to the next frame */
		ggiFlush(vis);
	}	

	/* Cycle through frames */
	i = 0;
	do {
		if(ggiSetDisplayFrame(vis, i)) {
			ggiPanic("Cannot set display frame!\n");
		}
		/* Flush command before waiting for input */
		ggiFlush(vis);

		/* Wait */
		do {
			c = ggiGetc(vis);
		} while (c==GIIK_VOID || GII_KTYP(c)==GII_KT_MOD);

		i = (i+1) % mode.frames;

	} while (c != 'q' && c != 'Q' && c != 'x' && c != 'X' && 
		 c != GIIUC_Escape);
	
	ggiClose(vis);
	ggiExit();
	return 0;
}
