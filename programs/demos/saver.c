/* $Id: saver.c,v 1.18 2008/03/13 19:54:44 cegger Exp $
******************************************************************************

   speed.c - screensaver like application

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
#include <ggi/gg.h>
#include <ggi/gii.h>
#include <ggi/ggi.h>

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <time.h>
#ifdef HAVE_UNISTD_H
# include <unistd.h>
#endif

#include <ggi/internal/gg_replace.h>

/* The time in minutes till activation
 */
static int timeout = 0;

/* The visual to draw on
 */
static ggi_visual_t visual;

/* The colormap
 */
static ggi_color map[256];

/* The colormap
 */
#define STARANZ 100

/* This is outdated. We'll have to add that back later.
 */
#define ggiMonPower(x)

/* Visible sizes.
 */
static int xsize, ysize;

/* Get the interrupt counters. This is Linux specific.
 */
static int getic(void)
{
	FILE *hand;
	int num, cnt;
	int c2 = 0;

	if ((hand = fopen("/proc/interrupts", "r")) == NULL) {
		perror("open /proc/interrupts");
		ggPanic("Do you have /proc mounted ?\n");
	}

	while (!feof(hand)) {
		fscanf(hand, "%d:%d%*[^\n]", &num, &cnt);
		switch (num) {	/* Add other interrupts as you want it ... */
		case 1:	/* Keyboard */
		case 3:	/* Mice on Com2/Com1 */
		case 4:
		case 12:	/* PS/2 Mouse */
			c2 += cnt;
			break;
		}
	}

	fclose(hand);
	return (c2);
}

/* These are the graphics effects displayed when active 
 */

/* Simple warpper that makes a random double between 0 and 1.
 */
static double rand01(void)
{
	return (rand() & 0xffff) / (double) 0x10000;
}

/* Fly through a starfield
 */
static void ge_stars(void)
{
	int x1, y, z;

	typedef struct {
		double x, y, z;
	} pix3d;
	typedef struct {
		int x1, y;
	} pix2d;
	pix3d pix[STARANZ], *pp;
	pix2d oldpos[STARANZ];
	double da, nda, dz, ndz, cx, sx, cz, sz, z0, hlp;

	z0 = 2.0 * ysize;
	memset(pix, 0, sizeof(pix));

	for (z = 0; z < STARANZ; z++)
		pix[z].z = -z0 + 50;
	memset(oldpos, 0, sizeof(oldpos));

	y = 0;
	da = nda = 0.0;
	dz = ndz = 0.0;
	while (!giiKbhit(visual)) {

		if (rand01() < 0.001)
			nda = (rand01() - 0.5) / 200.0;
		da = (nda + 999.0 * da) / 1000.0;
		cx = cos(da);
		sx = sin(da);
		if (rand01() < 0.001)
			ndz = (rand01() - 0.5) / 200.0;
		dz = (ndz - 10 * nda + 989.0 * dz) / 1000.0;
		cz = cos(dz / 10);
		sz = sin(dz / 10);
		for (z = 0; z < STARANZ; z++) {
			ggiSetGCForeground(visual, 0);
			ggiDrawPixel(visual, oldpos[z].x1, oldpos[z].y);

			pp = &pix[z];
		      retry:
			pp->z -= 5.0;
			hlp = pp->x * cx + pp->y * sx;
			pp->y = pp->x * -sx + pp->y * cx;
			pp->x = hlp;
			hlp = pp->x * cz + pp->z * sz;
			pp->z = pp->x * -sz + pp->z * cz;
			pp->x = hlp;
			hlp = z0 / (pp->z + z0);
			x1 = xsize / 2 + pp->x * hlp;
			y = ysize / 2 + pp->y * hlp;

			if (pp->z < -z0 + 100.0 || x1 < 10
			    || x1 >= xsize - 10 || y < 10
			    || y >= ysize - 10) {
				pp->x = rand01() * z0 * 2 - 1 * z0;
				pp->y = rand01() * 2 * z0 - 1 * z0;
				pp->z = rand01() * z0 * 10 + z0;
				goto retry;
			}
			ggiSetGCForeground(visual, 128);
			ggiDrawPixel(visual, oldpos[z].x1 =
				     x1, oldpos[z].y = y);
		}
		if (rand01() < 0.01)
			ggUSleep(1);
	}
}

/* Fly through a 3D-starfield red/blue glasses required
 */
static void ge_stars3d(void)
{
	int x1, x2, y, z;

	typedef struct {
		double x, y, z;
	} pix3d;
	typedef struct {
		int x1, x2, y;
	} pix2d;
	pix3d pix[STARANZ], *pp;
	pix2d oldpos[STARANZ];
	double da, nda, dz, ndz, cx, sx, cz, sz, z0, hlp;

	z0 = 2.0 * ysize;
	memset(pix, 0, sizeof(pix));
	for (z = 0; z < STARANZ; z++)
		pix[z].z = -z0 + 50;
	memset(oldpos, 0, sizeof(oldpos));

	y = 0;
	da = nda = 0.0;
	dz = ndz = 0.0;
	while (!giiKbhit(visual)) {
		if (rand01() < 0.001)
			nda = (rand01() - 0.5) / 500.0;
		da = (nda + 999.0 * da) / 1000.0;
		cx = cos(da);
		sx = sin(da);
		if (rand01() < 0.001)
			ndz = (rand01() - 0.5) / 500.0;
		dz = (ndz - 10 * nda + 989.0 * dz) / 1000.0;
		cz = cos(dz / 10);
		sz = sin(dz / 10);
		for (z = 0; z < STARANZ; z++) {
			ggiSetGCForeground(visual, 0);
			ggiDrawPixel(visual, oldpos[z].x1, oldpos[z].y);
			ggiDrawPixel(visual, oldpos[z].x2, oldpos[z].y);

			pp = &pix[z];
		      retry:
			pp->z -= 5.0;
			hlp = pp->x * cx + pp->y * sx;
			pp->y = pp->x * -sx + pp->y * cx;
			pp->x = hlp;
			hlp = pp->x * cz + pp->z * sz;
			pp->z = pp->x * -sz + pp->z * cz;
			pp->x = hlp;
			hlp = z0 / (pp->z + z0);
			x1 = xsize / 2 + (pp->x - xsize / 8) * hlp;
			x2 = x1 + xsize / 4 * hlp;
			y = ysize / 2 + pp->y * hlp;

			if (pp->z < -z0 + 100.0 || x1 < 10
			    || x1 >= xsize - 10 || x2 < 10
			    || x2 >= xsize - 10 || y < 10
			    || y >= ysize - 10) {
				pp->x = rand01() * z0 * 2 - 1 * z0;
				pp->y = rand01() * 2 * z0 - 1 * z0;
				pp->z = rand01() * z0 * 10 + z0;
				goto retry;
			}
			ggiSetGCForeground(visual, 128);
			ggiDrawPixel(visual, oldpos[z].x1 =
				     x1, oldpos[z].y = y);
			ggiSetGCForeground(visual, 1);
			ggiDrawPixel(visual, oldpos[z].x2 = x2, y);
		}
		if (rand01() < 0.01)
			ggUSleep(1);
	}
}

/* Show a bouncing blob
 */
static void ge_bounce(void)
{
	unsigned int c;
	int lx, ly;
	double x, y, dx, dy;
	int xx[256], yy[256];

	srand((unsigned) time(NULL));
	lx = x = xsize / 2;
	ly = y = 10;
	for (c = 0; c < 256; c++) {
		xx[c] = yy[c] = 5;
	}

	c = 0;
	ggiSetGCForeground(visual, c);
	ggiFillscreen(visual);
	while (!giiKbhit(visual)) {
		dx = (rand() % 0x1ff) / 255.0 - 1;
		dy = (rand() % 0x3ff) / 511.0 - 0.3;
		while (!giiKbhit(visual)) {
			x += dx;
			y += dy;
			if (x < 10 || x > xsize - 11) {
				dx = -dx;
				x += dx;
			}
			if (y < 10 || y > ysize - 11) {
				dy = -dy;
				y += dy;
			}
			if (y < ysize - 20)
				dy += 0.001;
			dx *= .9999;
			dy *= .9999;
			if (fabs(dx) < 1e-2)
				break;
			if (lx == (int) x && ly == (int) y)
				continue;
			c++;
			c &= 0xff;
			if (!c)
				c++;
			ggiSetGCForeground(visual, 0);
			ggiDrawBox(visual, xx[c] - 5, yy[c] - 5, xx[c] + 5,
				   yy[c] + 5);
			lx = xx[c] = x;
			ly = yy[c] = y;
			ggiSetGCForeground(visual, c);
			ggiDrawBox(visual, xx[c] - 5, yy[c] - 5, xx[c] + 5,
				   yy[c] + 5);
			if (!(c & 33))
				ggUSleep(1);
		}
	}
}

/* Show a two jumping lines. Look at that for a few minutes and you'll be nuts.
 */
static void ge_crazy(void)
{
	int x, y;

	for (x = -(ysize - 1);;) {

		if (giiKbhit(visual))
			break;

		y = x;
		if (y < 0)
			y = -y;

		ggiSetGCForeground(visual, 1);
		ggiDrawHLine(visual, 0, y, xsize / 2);

		ggiSetGCForeground(visual, 128);
		ggiDrawHLine(visual, xsize / 2, ysize - 1 - y, xsize / 2);

		if ((x & 15) == 0)
			ggUSleep(1);
		ggiSetGCForeground(visual, 0);
		ggiDrawHLine(visual, 0, y, xsize / 2);
		ggiDrawHLine(visual, xsize / 2, ysize - 1 - y, xsize / 2);

		if (++x > ysize - 1)
			x = -(ysize - 1);
	}
}

/* This is a reference for the father of all arcade games. Pong.
 */
static void ge_pong(void)
{
	int x, y, dx, dy, h;

	x = 30;
	y = 20;
	dx = 1;
	dy = 1;
	while (1) {
		if (giiKbhit(visual))
			break;

		if (!(x & 3))
			ggUSleep(1);
		if (x == xsize - 6 || x == 5)
			dx = -dx;
		if (y == ysize - 6 || y == 5)
			dy = -dy;
		ggiSetGCForeground(visual, 0);
		h = y;
		if (h < 10)
			h = 10;
		if (h >= ysize - 10)
			h = ysize - 11;
		ggiDrawBox(visual, 0, h - 10, 5, 20);
		ggiDrawBox(visual, xsize - 6, h - 10, 5, 20);
#if 0
		ggiDrawCircle(visual, x, y, 3);
		ggiDrawCircle(visual, x, y, 2);
		ggiDrawCircle(visual, x, y, 1);
#else
		ggiDrawBox(visual, x-2, y-2, 5, 5);
		ggiDrawBox(visual, x-1, y-1, 3, 3);
		ggiDrawBox(visual, x, y, 1, 1);
#endif
		x += dx;
		y += dy;
		ggiSetGCForeground(visual, 64);
#if 0
		ggiDrawCircle(visual, x, y, 3);
		ggiDrawCircle(visual, x, y, 2);
		ggiDrawCircle(visual, x, y, 1);
#else
		ggiDrawBox(visual, x-2, y-2, 5, 5);
		ggiDrawBox(visual, x-1, y-1, 3, 3);
		ggiDrawBox(visual, x, y, 1, 1);
#endif
		ggiSetGCForeground(visual, 1);
		h = y;
		if (h < 10)
			h = 10;
		if (h >= ysize - 10)
			h = ysize - 11;
		ggiDrawBox(visual, 0, h - 10, 5, 20);
		ggiSetGCForeground(visual, 128);
		ggiDrawBox(visual, xsize - 6, h - 10, 5, 20);
	}
}

/* Vesa blanker. This is currently inactive. This will be added back later.
 */
static void ge_vesa_blank(void)
{
	ggiMonPower(PWR_STANDBY);
	while (!giiKbhit(visual));
	ggiMonPower(PWR_ON);
}

/* Vesa blanker. This is currently inactive. This will be added back later.
 */
static void ge_vesa_blank2(void)
{
	ggiMonPower(PWR_SUSPEND);
	while (!giiKbhit(visual));
	ggiMonPower(PWR_ON);
}

/* Vesa blanker. This is currently inactive. This will be added back later.
 */
static void ge_vesa_blank3(void)
{
	ggiMonPower(PWR_OFF);
	while (!giiKbhit(visual));
	ggiMonPower(PWR_ON);
}

/* List of available saver functions.
 */
static struct scrsaver {
	void (*func) (void);
	const char *name;
} SaverList[] = { {
ge_bounce, "Jumping blob"}, {
ge_stars, "Starfield"}, {
ge_stars3d, "Starfield Stereo"}, {
ge_crazy, "Crazy Lines"}, {
ge_pong, "Ping Pong"}, {
ge_vesa_blank, "Power Standby"}, {
ge_vesa_blank2, "Power Suspend"}, {
ge_vesa_blank3, "Power Off"}, {
NULL, NULL}			/* Terminator */
};

static struct scrsaver *SaverActive = SaverList;

/* Execute a saver function.
 */
static void do_saver(void)
{
	ggiSetGCForeground(visual, 0);
	ggiFillscreen(visual);
	SaverActive->func();
	while (giiKbhit(visual))
		giiGetc(visual);
}

/* Show the setup-menu.
 */
static void blank_screen2(int interactive)
{
	int c;
	char hlpbuf[128];
	ggi_mode mode;

	if ((visual = ggNewStem(NULL)) == NULL) {
		fprintf(stderr, "cannot create stem.\n");
		exit(1);
	}

	if (giiAttach(visual) < 0) {
		fprintf(stderr, "cannot attach LibGII.\n");
		exit(1);
	}

	if (ggiAttach(visual) < 0) {
		fprintf(stderr, "cannot attach LibGGI.\n");
		exit(1);
	}

	if (ggiOpen(visual, NULL) < 0) {
		fprintf(stderr, "cannot open device.\n");
		exit(1);
	}

	ggiCheckSimpleMode(visual, GGI_AUTO, GGI_AUTO, GGI_AUTO, GT_8BIT,
			   &mode);
	if (ggiSetMode(visual, &mode)) {
		ggPanic("Cannot open one of the default modes.");
	} else {
		ggiGetMode(visual, &mode);
		xsize = mode.visible.x;
		ysize = mode.visible.y;
	}

	for (c = 1; c < 256; c++) {
		map[c].r = 63 * 8 * abs(c - 128);
		map[c].b = 63 * 7 * (128 - abs(c - 128));
		map[c].g = 63 * 100;
	}

	map[0].r = map[0].g = map[0].b = 0;
	ggiSetPalette(visual, 0, 256, map);
	ggiFillscreen(visual);

	while (1) {
		for (c = 0; c < ysize; c++) {
			ggiSetGCForeground(visual, c / 2U + 1U);
			ggiDrawHLine(visual, 0, c, xsize);
		}
		ggiSetGCForeground(visual, 128);
		ggiPuts(visual, 10, 10, "*** Screen - Saver ***");
		ggiPuts(visual, 10, 30, "Configuration Screen :");
		snprintf(hlpbuf, sizeof(hlpbuf),
			"n/p Type: %15s", SaverActive->name);
		ggiPuts(visual, 10, 50, hlpbuf);
		snprintf(hlpbuf, sizeof(hlpbuf),
			"+/- Time: %4d minutes", timeout);
		ggiPuts(visual, 10, 70, hlpbuf);
		ggiPuts(visual, 10, 100, "Switch away to activate");
		c = giiGetc(visual);
		switch (c) {
		case '+':
			if (timeout < 1440)
				timeout++;
			break;
		case '-':
			if (timeout > 1)
				timeout--;
			break;
		case 'p':
			if (SaverActive - SaverList > 0)
				SaverActive--;
			break;
		case 'n':
			if ((SaverActive + 1)->func)
				SaverActive++;
			break;
		case 't':
			ggiSetGCForeground(visual, 0);
			ggiFillscreen(visual);
			do_saver();
			break;
		case '\x1b':
		case 'q':
			ggiSetGCForeground(visual, 0);
			ggDelStem(visual);
			ggiExit();
			giiExit();
			exit(0);
		default:
			printf("Sym is %x.\n", c);
			break;
		}
	}
	ggDelStem(visual);
}

/* Main function. Wait for timeout, then activate the saver.
 */
int main(int argc, const char *argv[])
{
	int ic, ic2, cnt;
	uint32_t x;
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
		giiExit();
		exit(1);
	}

	ic = cnt = 0;

	if (argc >= 2) {
		x = atoi(argv[1]);
		if (x > 0 && x < 1440 /* 1 Day ... */ )
			timeout = x;
	}

	if (argc >= 3) {
		x = atoi(argv[2]);
		if (x >= 0 && x < sizeof(SaverList) / sizeof(SaverList[0]))
			SaverActive = SaverList + x;
	}

	if (timeout == 0)
		blank_screen2(0);
	else
		while (1) {

			if ((ic2 = getic()) == ic) {
				if (++cnt >= timeout) {
					if ((visual =
					     ggNewStem(NULL)) == NULL) {
						ggPanic
						    ("cannot create stem.\n");
					}

					if (giiAttach(visual) < 0) {
						ggPanic
						    ("cannot attach LibGII.\n");
					}

					if (ggiAttach(visual) < 0) {
						ggPanic
						    ("cannot attach LibGGI.\n");
					}

					if (ggiOpen(visual, NULL) < 0) {
						ggPanic
						    ("cannot open default visual.\n");
					}

					ggiCheckSimpleMode
						(visual, GGI_AUTO, GGI_AUTO,
						 GGI_AUTO, GT_8BIT, &mode);
					if (ggiSetMode(visual, &mode)) {
						ggPanic
						    ("cannot set 8bpp mode.");
					} else {
						ggiGetMode(visual, &mode);
						xsize = mode.visible.x;
						ysize = mode.visible.y;
					}
					do_saver();
					ggDelStem(visual);
					cnt = 0;
				}
			} else {
				cnt = 0;
				ic = ic2;
			}
			ggUSleep(60000);
		}

	ggDelStem(visual);
	ggiExit();
	giiExit();
	return (0);
}
