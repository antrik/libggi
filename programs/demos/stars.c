/* $Id: stars.c,v 1.6 2005/01/25 11:47:19 pekberg Exp $
******************************************************************************

   stars.c - rotating startfield

   Authors:	1998 	  Nathan Strong
   		1998	  Andreas Beck		[becka@ggi-project.org]
   		1998	  Andrew Apted		[andrew@ggi-project.org]
   		1999	  Marcus Sundberg	[marcus@ggi-project.org]

   Based on stars.bas, written by a friend of mine who goes by 'beek' in
   the world of the internet :-) There is no copyright, no restrictions,
   and most definitely no warranty on this software. I won't guarantee
   that it will compile, or that it will not cause damage to your system
   or your hardware. It's not like this is incredibly complicated code,
   anyway, so it's your own fault for not checking the code first!

   You can do anything you want with this code. Use it in your next demo!
   print it out and use it as wallpaper! Write me flamemail telling me how
   much I suck at programming (my e-mail is gblues@gstis.net)!

   The only thing I ask is for credit, even if it's a "This portion
   based on stars.c by Nathan Strong" buried deep in the source code
   that nobody will ever see. I was nice enough to credit my friend beek
   in the source, you can do the same for me :)

   You're still reading this? What's the matter with you? You must be
   a lawyer or something :)

******************************************************************************

   This is a demonstration of LibGGI's functions and can be used as a 
   reference programming example.

******************************************************************************
*/

#include <ggi/ggi.h>
#include <ggi/gg.h>

#include <stdlib.h>
#include <stdio.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <time.h>
#include <math.h>

static ggi_visual_t vis = NULL, dgavis = NULL;
static ggi_pixel lookup[256];
static int xoff, yoff;

static double angle = 0.0;
static double speed = 0.5;
static double scale;

static int count = 100, head = 1, tail = 0;

static int stars[300][3];
static int starsr[300][3];
static int posx[300][60];
static int posy[300][60];

static void SetupNewVisual(ggi_visual_t setvis);
static ggi_visual_t InitVisual(const char *visname);
static void CleanUp(void);
static void InitStars(void);
static void Transform(int *ta, int *tb);

int main(int argc, char **argv)
{
	int i, quit = 0;
	double fps = 0.0;
	int color;
	time_t st;
	int frames = 0;
	ggi_visual_t curvis;

	if (argc > 1) {
		count = atoi(argv[1]) % 301;
	}
	if (argc > 2) {
		head = atoi(argv[2]) % 60;
	}
	if (argc > 3) {
		speed = atof(argv[3]);
	}

	if (ggiInit() != 0) {
		fprintf(stderr, "unable to initialize LibGGI, exiting.\n");
		exit(1);
	}

	vis = InitVisual(NULL);	/* Null gives the default visual */
	if (vis == NULL) {
		ggiPanic("unable to open default visual, exiting.\n");
	}
	InitStars();

	curvis = vis;
	st = time(NULL);
	do {
		for (i = 0; i < count; i++) {
			starsr[i][0] = stars[i][0];
			starsr[i][1] = stars[i][1];
			starsr[i][2] = stars[i][2];

			Transform(&starsr[i][1], &starsr[i][2]);
			Transform(&starsr[i][0], &starsr[i][2]);
			Transform(&starsr[i][0], &starsr[i][1]);

			ggiPutPixel(curvis, posx[i][tail], posy[i][tail],
				    lookup[0]);

			posx[i][head] =
			    xoff +
			    (int) floor((256 * starsr[i][0]) /
					(double) (starsr[i][2] -
						  1024) * scale);
			posy[i][head] =
			    yoff +
			    (int) floor((256 * starsr[i][1]) /
					(double) (starsr[i][2] -
						  1024) * scale);

			color = (int) floor((starsr[i][2] + 721) / 5.5);

			ggiPutPixel(curvis, posx[i][head], posy[i][head],
				    lookup[color]);
		}
		ggUSleep(10000);
		ggiFlush(curvis);
		frames++;
		angle += speed;
		head = (head + 1) % 60;
		tail = (tail + 1) % 60;
		if (ggiKbhit(curvis)) {
			int key = ggiGetc(curvis);

			switch (key) {
			case ' ':
				/* Switch from/to DGA visual if possible */
				if (dgavis == NULL) {
					dgavis = InitVisual("DGA");
					if (dgavis) {
						curvis = dgavis;
					}
				} else {
					ggiClose(dgavis);
					dgavis = NULL;
					SetupNewVisual(vis);
					curvis = vis;
				}
				break;
			default:
				quit = 1;
				break;
			}
		}
	} while (!quit);

	if ((time(NULL) - st) != 0)
		fps = frames / (time(NULL) - st);

	printf("%f frames per second\n", fps);
	printf("beek / lightspeed\nPorted to GGI by Nathan Strong\n");
	CleanUp();

	return 0;
}

void Transform(int *ta, int *tb)
{
	double y = 0.0, z = 0.0;
	y = (cos((angle / 20)) * *ta) - (sin((angle / 20)) * *tb);
	z = (sin((angle / 20)) * *ta) + (cos((angle / 20)) * *tb);
	*ta = (int) floor(y);
	*tb = (int) floor(z);
}

void CleanUp(void)
{
	if (dgavis) {
		ggiClose(dgavis);
	}
	if (vis) {
		ggiClose(vis);
	}
	ggiExit();
}

void InitStars(void)
{
	int i;
	srand((unsigned)time(NULL));
	for (i = 0; i < count; i++) {
		stars[i][0] = ((rand() % 320) + 1 - 160) * 3;
		stars[i][1] = ((rand() % 320) + 1 - 160) * 3;
		stars[i][2] = ((rand() % 128) + 1 - 64) * 5;
	}
}

void SetupNewVisual(ggi_visual_t setvis)
{
	int i;
	ggi_mode mode;

	ggiGetMode(setvis, &mode);

	xoff = mode.visible.x / 2;
	yoff = mode.visible.y / 2;

	scale = ((double) mode.visible.x / 200.0);

	if (GT_SCHEME(mode.graphtype) == GT_PALETTE) {

		int nocols = 1 << GT_DEPTH(mode.graphtype);

		ggi_color *Palette = malloc(sizeof(ggi_color) * nocols);

		if (Palette == NULL) {
			ggiPanic("Unable to allocate temporary memory!\n");
		}

		for (i = 0; i < nocols; i++) {
			Palette[i].r = i * 0xffff / (nocols - 1);
			Palette[i].g = i * 0xffff / (nocols - 1);
			Palette[i].b = i * 0xffff / (nocols - 1);
		}

		ggiSetPalette(setvis, 0, nocols, Palette);

		free(Palette);
	}

	for (i = 0; i < 256; i++) {

		ggi_color col;

		col.r = col.g = col.b = i * 0xffff / 255;

		lookup[i] = ggiMapColor(setvis, &col);
	}

	ggiSetGCForeground(setvis, lookup[0]);
	ggiFillscreen(setvis);
}

ggi_visual_t InitVisual(const char *visname)
{
	ggi_visual_t newvis;

	/* Adding an extra NULL when visname is NULL doesn't hurt */
	if ((newvis = ggiOpen(visname, NULL)) == NULL) {
		return NULL;
	}
	ggiSetFlags(newvis, GGIFLAG_ASYNC);

	if (ggiSetSimpleMode(newvis, GGI_AUTO, GGI_AUTO, GGI_AUTO, GT_AUTO)
	    != 0) {
		ggiPanic("Cannot set default mode!\n");
	}

	SetupNewVisual(newvis);

	return newvis;
}
