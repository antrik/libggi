/* $Id: slimy.c,v 1.14 2007/03/03 19:36:16 cegger Exp $
******************************************************************************

   Slimy Plasma Spinner by WolfWings ShadowFlight

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
#include <stdlib.h>
#include <time.h>
#include <math.h>

#ifndef M_PI
# define M_PI	3.141592654
#endif


static ggi_visual_t disp = NULL;
static const ggi_directbuffer *dbuf;
static uint8_t *bitmap;

static signed long int xcosv[1024 + 512];
static signed long int ycosv[1024 + 512];
static signed long int *sx, *sy;

static int width;		/* Visible screen width, in pixels  */
static int height;		/* Visible screen height, in pixels */

static int show_fps = 0;

static ggi_color black = { 0x0000, 0x0000, 0x0000 };

static int
myKbhit(ggi_visual_t vis)
{
	struct timeval t={0,0};

	return (giiEventPoll((gii_input)vis, emKeyPress | emKeyRepeat, &t)
		!= emZero);
}

static int
myGetc(ggi_visual_t vis)
{
	gii_event ev;

	/* Block until we get a key. */
	giiEventRead(vis, &ev, emKeyPress | emKeyRepeat);

	return ev.key.sym;
}



static void fail(const char *reason)
{
	fprintf(stderr, "%s", reason);
	if (disp != NULL) {
		ggDelStem(disp);
		ggiExit();
		giiExit();
	}
	exit(1);
}

static void load_bitmap(void)
{
	int p, x, y, xm, ym;
	double f;
	ggi_color gc;

	bitmap = malloc(32 * 32);
	if (bitmap == NULL)
		fail("Error allocating memory!\n");

	for (y = 0; y < 16; y++) {
		for (x = 0; x < 16; x++) {
			bitmap[(y << 5) + x] = (y << 4) + x;
			bitmap[(y << 5) + (31 - x)] = (y << 4) + x;
			bitmap[((31 - y) << 5) + x] = (y << 4) + x;
			bitmap[((31 - y) << 5) + (31 - x)] = (y << 4) + x;
		}
	}

	for (y = 0; y < 16; y++) {
		for (x = 0; x < 16; x++) {
			gc.r = x << 12;
			gc.g = (x + y) << 11;
			gc.b = y << 12;
			ggiSetPalette(disp, (y << 4) + x, 1, &gc);
		}
	}

	xm = (640 << 12) / width;
	ym = (480 << 12) / height;

	for (p = 0; p < (1024 + 512); p++) {
		f = cos((p & 1023) * (M_PI / 512));
		xcosv[p] = f * xm;
		ycosv[p] = f * ym;
	}

	sx = malloc(sizeof(signed long int) * width * 2);
	if (sx == NULL)
		fail("Error allocating memory!\n");

	for (p = 0; p < width; p++) {
		sx[p] = sx[p + width] =
		    cos(p * 2 * (M_PI / width)) * xm * 64;
	}

	sy = malloc(sizeof(signed long int) * height * 2);
	if (sy == NULL)
		fail("Error allocating memory!\n");

	for (p = 0; p < height; p++) {
		sy[p] = sy[p + height] = cos(p * 2 * (M_PI / height))
		    * ym * 64 * 32;
	}
}

static void draw_rotation(int angle, int basex, int basey, int movex,
			  int movey)
{
	unsigned int xrowadd, yrowadd, xcoladd, ycoladd;
	unsigned int x, y, vx, vy, padd;
	uint8_t *fbptr;

	xrowadd = ycosv[angle];
	yrowadd = ycosv[angle + 256];
	xcoladd = xcosv[angle + 256];
	ycoladd = xcosv[angle + 512];
	vx = (xcoladd * movex) + (ycoladd * movey);
	vy = ((xrowadd * movex) + (yrowadd * movey)) * 32;
	yrowadd *= 32;
	ycoladd *= 32;
	xrowadd -= (xcoladd * width);
	yrowadd -= (ycoladd * width);
	sy += basey;
	sx += basex;

	/* Acquire DirectBuffer before we use it. */
	if (ggiResourceAcquire(dbuf->resource, GGI_ACTYPE_WRITE) != 0) {
		fail("Error acquiring DirectBuffer\n");
	}
	fbptr = dbuf->write;
	padd = dbuf->buffer.plb.stride - width;

	for (y = height; y > 0; y--) {
		vy += *sy;
		for (x = width; x > 0; x--) {
			vx += xcoladd;
			vy += ycoladd;
			*fbptr++ = bitmap[(((vx + *sx++) & 0x1F0000)
					   | (vy & 0x3E00000)) >> 16];
		}
		sx -= width;
		vy -= *sy++;
		vx += xrowadd;
		vy += yrowadd;
		fbptr += padd;
	}

	/* Release DirectBuffer when done with it. */
	ggiResourceRelease(dbuf->resource);

	sy -= height;
	sy -= basey;
	sx -= basex;
}

static void SmoothPalette(ggi_color * c)
{
	int xv, yv, z;
	ggi_color t[256];
	for (xv = 0; xv < 16; xv++) {
		for (yv = 0; yv < 16; yv++) {
			z = (c[0].r * xv * yv);
			z += (c[1].r * xv * (16 - yv));
			z += (c[2].r * (16 - xv) * yv);
			z += (c[3].r * (16 - xv) * (16 - yv));
			t[(yv << 4) + xv].r = z;
			z = (c[0].g * xv * yv);
			z += (c[1].g * xv * (16 - yv));
			z += (c[2].g * (16 - xv) * yv);
			z += (c[3].g * (16 - xv) * (16 - yv));
			t[(yv << 4) + xv].g = z;
			z = (c[0].b * xv * yv);
			z += (c[1].b * xv * (16 - yv));
			z += (c[2].b * (16 - xv) * yv);
			z += (c[3].b * (16 - xv) * (16 - yv));
			t[(yv << 4) + xv].b = z;
		}
	}
	ggiSetPalette(disp, 0, 256, t);
}

static void InitGraphics(void)
{
	ggi_mode m;

	if (giiInit() < 0) {
		fprintf(stderr, "Unable to initialize LibGGI, exiting.\n");
		exit(1);
	}

	if (ggiInit() < 0) {
		fprintf(stderr, "Unable to initialize LibGGI, exiting.\n");
		giiExit();
		exit(1);
	}

	if (!(disp = ggNewStem(NULL))) {
		ggPanic("Unable to create stem, exiting.\n");
	}

	if (giiAttach(disp) < 0) {
		fail("Unable to attach LibGII, exiting.\n");
	}

	if (ggiAttach(disp) < 0) {
		fail("Unable to attach LibGGI, exiting.\n");
	}

	if (ggiOpen(disp, NULL) < 0) {
		fail("Unable to open default visual, exiting.\n");
	}

	ggiSetFlags(disp, GGIFLAG_ASYNC);

	ggiCheckSimpleMode(disp, GGI_AUTO, GGI_AUTO, GGI_AUTO, GT_8BIT, &m);
	if (ggiSetMode(disp, &m)) {
		fail("Error switching to 8bpp mode.\n"
		     "You may want to try using GGI_DISPLAY=\"tile:0,0,640,480,(palemu:auto)\" ./slimy\n"
		     "if you think the problem is due to running on a truecolor-only target.\n");
	}

	ggiGetMode(disp, &m);

	width = m.virt.x;
	height = m.virt.y;

	if (!(dbuf = ggiDBGetBuffer(disp, 0))) {
		fail("No DirectBuffer available.\n");
	}

	if (!(dbuf->type & GGI_DB_SIMPLE_PLB)) {
		fail("Error: non-standard display buffer\n");
	}
}

static void CloseGraphics(void)
{
	ggDelStem(disp);
	ggiExit();
	giiExit();
}

static void RunSpinner(void)
{
	double fps = .0;
	int quit = 0, frames = 0;
	time_t tt;
	int a, x, y, xdir, ydir, adir, vx, vy, vxdir, vydir;
	int z, g;
	ggi_color e[4], c[4], d[4] =
	    { {0, 0, 0}, {0, 127, 255}, {255, 127, 0}, {255, 255, 255} };

	x = rand() % width;
	y = rand() % height;
	vx = rand() % width;
	vy = rand() % height;
	xdir = (rand() & 1) ? ((rand() & 1) ? width - 2 : 2)
	    : ((rand() & 1) ? width - 1 : 1);
	ydir = (rand() & 1) ? ((rand() & 1) ? height - 2 : 2)
	    : ((rand() & 1) ? height - 1 : 1);
	vxdir = (rand() & 1) ? ((rand() & 1) ? -2 : 2)
	    : ((rand() & 1) ? -1 : 1);
	vydir = (rand() & 1) ? ((rand() & 1) ? -2 : 2)
	    : ((rand() & 1) ? -1 : 1);
	adir = 7;
	g = 0;
	a = 0;
	tt = time(NULL);
	do {
		if (myKbhit(disp)) {
			int key = myGetc(disp);
			if ((key == 'f') || (key == 'F'))
				show_fps = !show_fps;
			else
				quit = 1;
		}
		draw_rotation(a, x, y, vx, vy);
		a = (a + adir) & 1023;
		if (!(rand() & 1023))
			adir = -adir;
		x = (x + xdir) % width;
		if (!(rand() & 1023)) {
			xdir =
			    (rand() & 1) ? ((rand() & 1) ? width - 2 : 2)
			    : ((rand() & 1) ? width - 1 : 1);
		}
		y = (y + ydir) % height;
		if (!(rand() & 1023)) {
			ydir =
			    (rand() & 1) ? ((rand() & 1) ? height - 2 : 2)
			    : ((rand() & 1) ? height - 1 : 1);
		}
		vx += vxdir;
		if (vx < 0)
			vxdir = (rand() & 1) + 1;
		else if (vx > width)
			vxdir = (rand() & 1) - 2;
		vy += vydir;
		if (vy < 0)
			vydir = (rand() & 1) + 1;
		else if (vy > height)
			vydir = (rand() & 1) - 2;
		if (!g) {
			for (z = 0; z < 4; z++) {
				c[z] = d[z];
				d[z].r = rand() & 255;
				d[z].g = rand() & 255;
				d[z].b = rand() & 255;
			}
		}
		for (z = 0; z < 4; z++) {
			e[z].r = ((d[z].r * g) + (c[z].r * (64 - g))) / 64;
			e[z].g = ((d[z].g * g) + (c[z].g * (64 - g))) / 64;
			e[z].b = ((d[z].b * g) + (c[z].b * (64 - g))) / 64;
		}
		SmoothPalette(e);
		g = ((g + 1) & 63);

		if ((time(NULL) - tt) != 0)
			fps = frames / (time(NULL) - tt);

		if (show_fps) {
			char str[18];
			ggi_pixel _bg;
			sprintf(str, "FPS : %f", fps);
			ggiGetGCBackground(disp, &_bg);
			ggiSetGCBackground(disp,
					   ggiMapColor(disp, &black));
			ggiPuts(disp, 0, 0, str);
			ggiSetGCBackground(disp, _bg);
		}

		ggiFlush(disp);
		frames++;
	} while (!quit);
	printf(" %f fps\n", fps);
}

int main(int argc, const char *argv[])
{

	InitGraphics();

	load_bitmap();

	RunSpinner();

	CloseGraphics();

	exit(0);

	/* Eliminate "control reaches end of non-void function"
	 * compiler warning
	 */
	return 0;
}
