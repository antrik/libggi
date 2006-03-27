/* $Id: monitest.c,v 1.12 2006/03/27 19:46:31 pekberg Exp $
******************************************************************************

   Monitor test pattern generator

   Written in 1998 by Hartmut Niemann

   This software is placed in the public domain and can be used freely
   for any purpose. It comes without any kind of warranty, either
   expressed or implied, including, but not limited to the implied
   warranties of merchantability or fitness for a particular purpose.
   Use it at your own risk. the author is not responsible for any damage
   or consequences raised by use or inability to use this program.
 
******************************************************************************

   This program creates a test pattern like ctmon3 and the
   Nokia montest program created or is/was broadcast on tv
   during program breaks, when there was no 24h program.

   The second part is a convergence test researched from the
   Nokia program, to check whether red, green and blue beams
   are adjusted properly.

   for a first glance: run
	monitest 640x480 
   or
	monitest

   Very much is copied from ./demo.c
   The text output would require about 400x300 dots, but it works
   with less too.

******************************************************************************
*/

#include "config.h"
#include <ggi/gg.h>
#include <ggi/gii.h>
#include <ggi/gii-keyboard.h>
#include <ggi/ggi.h>

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#include "monitest.h"
#include "ggitext.h"
#include "menu.h"

#include "drawlib.h"

#ifdef HAVE_KGIMON_H
#include "kgitune.h"
#else
#define is_kgicondev(x) (0)
#define kgitune(x)	do{}while(0)
#endif

static ggi_visual_t vis;
static int kgidevice;

ggi_pixel white, black, red, green, blue, yellow, magenta, cyan;

static void usage(const char *prog)
{
	fprintf(stderr, "Usage:\n\n"
		"%s <bpp> <xsize> <ysize>\n\n"
		"Default: %s 8 320 200\n", prog, prog);
	exit(1);
}

static int
myGetc(ggi_visual_t vis)
{
	gii_event ev;

	/* Block until we get a key. */
	giiEventRead(vis, &ev, emKeyPress | emKeyRepeat);

	return ev.key.sym;
}

int waitabit(ggi_visual_t _vis)
{
	int key;
	key = myGetc(_vis);

	if (toupper((uint8_t)key) == 'Q') {
		/* Q pressed */
		ggDelStem(_vis);
		ggiExit();
		giiExit();
		exit(1);
	} else if (key == GIIUC_Escape || toupper((uint8_t)key) == 'B') {
		return 1;
	}
	return 0;
}


static void setcolors(void)
{
	/* just set the needed colour names */
	ggi_color col;

	col.r = 0xFFFF;
	col.g = 0xFFFF;
	col.b = 0xFFFF;
	white = ggiMapColor(vis, &col);

	col.r = 0xFFFF;
	col.g = 0xFFFF;
	col.b = 0x0000;
	yellow = ggiMapColor(vis, &col);

	col.r = 0xFFFF;
	col.g = 0x0000;
	col.b = 0xFFFF;
	magenta = ggiMapColor(vis, &col);

	col.r = 0xFFFF;
	col.g = 0x0000;
	col.b = 0x0000;
	red = ggiMapColor(vis, &col);

	col.r = 0x0000;
	col.g = 0xFFFF;
	col.b = 0xFFFF;
	cyan = ggiMapColor(vis, &col);

	col.r = 0x0000;
	col.g = 0xFFFF;
	col.b = 0x0000;
	green = ggiMapColor(vis, &col);

	col.r = 0x0000;
	col.g = 0x0000;
	col.b = 0xFFFF;
	blue = ggiMapColor(vis, &col);

	col.r = 0x0000;
	col.g = 0x0000;
	col.b = 0x0000;
	black = ggiMapColor(vis, &col);
}


static void convergence(ggi_visual_t _vis)
{
#define PARTSHOR 16
#define PARTSVERT 12
#define PARTX(x) (signed)((x)*(xmax-1)/PARTSHOR)
#define PARTWIDTH (signed)(xmax/PARTSHOR)
#define PARTY(y) (signed)((y)*(ymax-1)/PARTSVERT)
#define PARTHEIGHT (signed)(ymax/PARTSVERT)

	unsigned int i, j, k, xmax, ymax;
	unsigned int f[3];
	ggi_mode currmode;

	ggiGetMode(_vis, &currmode);
	xmax = currmode.visible.x;
	ymax = currmode.visible.y;

	f[0] = red;
	f[1] = green;
	f[2] = blue;

	for (k = 0; k < 4; k++) {
		ggiSetGCForeground(_vis, black);
		ggiFillscreen(_vis);
		for (i = 0; i <= PARTSHOR; i++) {
			for (j = 0; j <= PARTSVERT; j++) {
				ggiSetGCForeground(_vis,
						   f[(i + j + k) % 3]);
				ggiDrawHLine(_vis, PARTX(i), PARTY(j),
					     PARTWIDTH / 2);
				ggiDrawVLine(_vis, PARTX(i), PARTY(j),
					     PARTHEIGHT / 2);
				ggiDrawHLine(_vis,
					     PARTX(i) - PARTWIDTH / 2,
					     PARTY(j), PARTWIDTH / 2);
				ggiDrawVLine(_vis, PARTX(i),
					     PARTY(j) - PARTHEIGHT / 2,
					     PARTHEIGHT / 2);
			}
		}
		ggiFlush(_vis);
		if (waitabit(_vis))
			return;
	}
}


static void moiree(ggi_visual_t _vis)
{
	int xmax, ymax;
	ggi_mode currmode;

	ggiGetMode(_vis, &currmode);
	xmax = currmode.visible.x;
	ymax = currmode.visible.y;
	stripevert(_vis, 0, 0, xmax - 1, ymax - 1,
		   black, white, 1);
	ggiFlush(_vis);
	if (waitabit(_vis))
		return;
	stripevert(_vis, 0, 0, xmax - 1, ymax - 1,
		   red, white, 1);
	ggiFlush(_vis);
	if (waitabit(_vis))
		return;
	stripevert(_vis, 0, 0, xmax - 1, ymax - 1,
		   green, white, 1);
	ggiFlush(_vis);
	if (waitabit(_vis))
		return;
	stripevert(_vis, 0, 0, xmax - 1, ymax - 1,
		   blue, white, 1);
	ggiFlush(_vis);
	if (waitabit(_vis))
		return;

	dotone(_vis, 0, 0, xmax - 1, ymax - 1, black, white);
	ggiFlush(_vis);
	if (waitabit(_vis))
		return;
	dotone(_vis, 0, 0, xmax - 1, ymax - 1, red, white);
	ggiFlush(_vis);
	if (waitabit(_vis))
		return;
	dotone(_vis, 0, 0, xmax - 1, ymax - 1, green, white);
	ggiFlush(_vis);
	if (waitabit(_vis))
		return;
	dotone(_vis, 0, 0, xmax - 1, ymax - 1, blue,
	       white);
	ggiFlush(_vis);
	if (waitabit(_vis))
		return;

	chessboardone(_vis, 0, 0, xmax - 1, ymax - 1, black, white);
	ggiFlush(_vis);
	if (waitabit(_vis))
		return;
	chessboardone(_vis, 0, 0, xmax - 1, ymax - 1, red, white);
	ggiFlush(_vis);
	if (waitabit(_vis))
		return;
	chessboardone(_vis, 0, 0, xmax - 1, ymax - 1, green, white);
	ggiFlush(_vis);
	if (waitabit(_vis))
		return;
	chessboardone(_vis, 0, 0, xmax - 1, ymax - 1, blue, white);
	ggiFlush(_vis);
	if (waitabit(_vis))
		return;
}

#if 0	/* defined but not used */
char *helptext = {
	"GGI screntest program               \n"
	    "(c) H. Niemann, $Id: monitest.c,v 1.12 2006/03/27 19:46:31 pekberg Exp $               \n"
	    "h:   this help screen               \n"
	    "q:   quit this testscreen           \n" ""
};

static int help(void)
{
	ggi_mode currmode;

	ggiGetMode(vis, &currmode);

	ggiGraphTextLongPuts(vis, currmode.visible.x / 2,
			     currmode.visible.y / 2, 0, 0, GGI_TEXT_CENTER,
			     helptext);
	waitabit(vis);
	return 0;
}
#endif


static const ggi_coord resolutions[] = {
	{320, 200},
	{320, 240},
	{320, 350},		/* EGA text */
	{360, 400},		/* VGA text */
	{400, 300},
	{480, 300},
	{512, 384},
	{640, 200},		/* EGA */
	{640, 350},		/* EGA */
	{640, 400},
	{640, 480},
	{720, 350},		/* MDA text */
	{720, 400},		/* VGA text */
	{800, 600},
	{1024, 768},
	{1152, 864},
	{1280, 1024},
	{1600, 1200},
	{0, 0}			/* End mark!! */
};

static const ggi_graphtype graphtypes[] = {
	/* can't handle textmode yet */
	GT_1BIT,		/*  1 bpp graphics              */
	GT_4BIT,		/*  4 bpp graphics              */
	GT_8BIT,		/*  8 bpp graphics              */
	GT_15BIT,		/* 15 bpp graphics              */
	GT_16BIT,		/* 16 bpp graphics              */
	GT_24BIT,		/* 24 bpp graphics              */
	GT_32BIT,		/* 24 bpp word aligned          */
	GT_INVALID
};

static int resindex = -1;		/* 'base' resolution of the mode set */
static int gtindex = -1;		/* graphtype, i.e. bit depth         */
static int xres = GGI_AUTO;		/* visual resolution, might be different */
static int yres = GGI_AUTO;		/* from the mode list */

static int guessmode(void)
{
/* try to guess the mode you're in and set resindex and gtindex  */
/* done on startup.                                              */
/* I could set up a certain mode at startup, but I don't want to */

	ggi_mode currmode;

	int i;

	ggiGetMode(vis, &currmode);
	fprintf(stderr, "Current mode is %dx%d %u/%u.\n",
		currmode.visible.x, currmode.visible.y,
		GT_DEPTH(currmode.graphtype), GT_SIZE(currmode.graphtype));


	xres = currmode.visible.x;
	yres = currmode.visible.y;

	/* find most possible resolution */
	resindex = 0;		/* if everything else fails: assume first mode:320x200 */
	i = 0;
	while (resolutions[i].x != 0) {
		/* for now: only test exact match */
		if ((xres == resolutions[i].x)
		    && (yres == resolutions[i].y)) {
			resindex = i;
			fprintf(stderr, "Detected resolution %d: %dx%d.\n",
				resindex,
				resolutions[resindex].x,
				resolutions[resindex].y);
		}
		i++;
	}
	/* find mode you're in */
	gtindex = -1;
	i = 0;
	while (graphtypes[i] != GT_INVALID) {
		if (graphtypes[i] == currmode.graphtype) {
			gtindex = i;
			fprintf(stderr, "Detected graphtype %u/%u\n",
				GT_DEPTH(graphtypes[gtindex]),
				GT_SIZE(graphtypes[gtindex]));
			break;
		}
		i++;
	}
	if (gtindex == -1) {
		return -1;
	}
	return 0;
}


static int changeresmenu(void)
{
	struct menu cm;
	static int _select = 0;

	ggi_mode suggmode;

	char s[32];
	char nextmodeline[100];
	char prevmodeline[100];

	char nextgtline[100];
	char prevgtline[100];

	char bottom[100];

	int nextresindex, prevresindex;
	int nextgtindex, prevgtindex;

	nextresindex = prevresindex = 0;
	nextgtindex = prevgtindex = 0;

	default_menu(&cm, vis);

	cm.w.title = " Change Screen resolution ";

	cm.lastentry = 8;
	cm.entry[0].text = "1 Increase X";
	cm.entry[1].text = "2 Decrease X";
	cm.entry[2].text = "3 Increase Y";
	cm.entry[3].text = "4 Decrease Y";
	cm.entry[8].text = "9 Back";

	sprintf(bottom, "  ");

	for (;;) {

		nextresindex = resindex + 1;
		if (resolutions[nextresindex].x == 0) {
			nextresindex--;
		};
		prevresindex = resindex - 1;
		if (prevresindex < 0) {
			prevresindex = 0;
		}

		nextgtindex = gtindex + 1;
		if (graphtypes[nextgtindex] == GT_INVALID) {
			nextgtindex--;
		}
		prevgtindex = gtindex - 1;
		if (prevgtindex < 0) {
			prevgtindex = 0;
		}
		sprintf(cm.entry[4].text = nextgtline,
			"5 Increase depth: %u/%u",
			GT_DEPTH(graphtypes[nextgtindex]),
			GT_SIZE(graphtypes[nextgtindex]));
		sprintf(cm.entry[5].text = prevgtline,
			"6 Decrease depth: %u/%u",
			GT_DEPTH(graphtypes[prevgtindex]),
			GT_SIZE(graphtypes[prevgtindex]));
		sprintf(cm.entry[6].text =
			nextmodeline, "7 next res.:  %dx%d",
			resolutions[nextresindex].x,
			resolutions[nextresindex].y);
		sprintf(cm.entry[7].text =
			prevmodeline, "8 prev. res.: %dx%d",
			resolutions[prevresindex].x,
			resolutions[prevresindex].y);


		sprintf(s, "current: %4dx%3dx%u/%u", xres, yres,
			GT_DEPTH(graphtypes[gtindex]),
			GT_SIZE(graphtypes[gtindex]));

		cm.toptext = s;
		cm.bottomtext = bottom;

		calculate_menu(&cm);
		center_menu(&cm);

		/*sleep(1); */
		testpattern(vis);
		ggiFlush(vis);
		switch (_select = do_menu(&cm, _select)) {
		case 0:
			xres += 1;
			break;
		case 1:
			xres -= 1;
			break;
		case 2:
			yres += 1;
			break;
		case 3:
			yres -= 1;
			break;
		case 4:
			gtindex = nextgtindex;
			break;
		case 5:
			gtindex = prevgtindex;
			break;
		case 6:
			resindex = nextresindex;
			xres = resolutions[resindex].x;
			yres = resolutions[resindex].y;
			break;
		case 7:
			resindex = prevresindex;
			xres = resolutions[resindex].x;
			yres = resolutions[resindex].y;
			break;
		case 8:
		case -1:
			testpattern(vis);	/* clear menu */
			return (0);
		default:
			ggPanic("Internal error, wrong menu selection");
		}
		/* try mode */
		if (ggiCheckSimpleMode
		    (vis, xres, yres, 1, graphtypes[gtindex],
		     &suggmode) != 0) {
			/* failed */
			sprintf(bottom, "mode set failed");
		} else {
			if (ggiSetSimpleMode(vis, xres, yres, 1,
					     graphtypes[gtindex]) != 0) {
				fprintf(stderr, "Set may not fail!!\n");
			} else {
				sprintf(bottom, "mode set succeeded");
			}
			/*sleep(1); */
			/*ggiFlush(vis); */
			if (GT_SCHEME(graphtypes[gtindex]) == GT_PALETTE)
				ggiSetColorfulPalette(vis);
		}
		setcolors();	/* necessary when depth changed */

	}
	return 0; /* never get here */
}



static int mainmenu(void)
{
	struct menu mainm;
	static int _select = 0;

	default_menu(&mainm, vis);
	mainm.w.title = " * Main menu * ";

	mainm.entry[0].text = "1 Geometry and Colors";
	mainm.entry[1].text = "2 Convergence";
	mainm.entry[2].text = "3 Resolution";
	mainm.entry[3].text = "4 Moiree";
	mainm.entry[4].text = "5 Change Resolution";
	mainm.entry[5].text = "6 Flat panel check";
	if (kgidevice) {
		mainm.entry[6].text = "7 Tune KGIcon monitor timings";
		mainm.entry[7].text = "8 Exit";
		mainm.lastentry = 7;
	} else {
		mainm.entry[6].text = "7 Exit";
		mainm.lastentry = 6;
	}

	mainm.toptext = "Please choose with arrows";
	mainm.bottomtext = "and press <return>";

	calculate_menu(&mainm);

	for (;;) {
		center_menu(&mainm);
		/* if resolution changes, we must recenter */
		switch (_select = do_menu(&mainm, _select)) {
		case 0:
			testpattern(vis);
			waitabit(vis);
			break;
		case 1:
			convergence(vis);
			break;
		case 2:
			resolution(vis);
			break;
		case 3:
			moiree(vis);
			break;
		case 4:
			changeresmenu();
			break;
		case 5:
			flatpanel(vis);
			break;
		case 6:
			if (kgidevice) {
				testpattern(vis);
				kgitune(vis);
			} else {
				return 0;
			}
			break;
		case 7:
		case -1:
			return 0;
			break; /* never get here */
		default:
			ggPanic("Internal error, wrong menu selection");
		}
	}
	return 0; /* never get here */
}


int main(int argc, char **argv)
{
	const char *prog = argv[0];
	ggi_mode mo;

	if (argc == 1) {
		ggiParseMode("", &mo);
	} else if (argc == 2) {
		ggiParseMode(argv[1], &mo);
	} else {
		usage(prog);
		return 1;
	}


	if (giiInit() < 0) {
		fprintf(stderr, "unable to initialize LibGGI, exiting.\n");
		exit(1);
	}

	if (ggiInit() < 0) {
		fprintf(stderr, "unable to initialize LibGGI, exiting.\n");
		giiExit();
		exit(1);
	}

	vis = ggNewStem();
	if (vis == NULL) {
		ggPanic("unable to create stem, exiting.\n");
	}

	if (giiAttach(vis) < 0) {
		ggPanic("unable to attach LibGII, exiting.\n");
	}

	if (ggiAttach(vis) < 0) {
		ggPanic("unable to attach LibGGI, exiting.\n");
	}

	if (ggiOpen(vis, NULL) < 0) { /* Null gives the default visual */
		ggPanic("unable to open default visual, exiting.\n");
	}

	ggiSetFlags(vis, GGIFLAG_ASYNC);

	printf("Trying mode ");
	ggiPrintMode(&mo);
	printf("\n");

	ggiCheckMode(vis, &mo);
	/*ggiCheckTextMode(vis,80,25,80,25,9,14,GT_TEXT16,&mo,NULL); */

	printf("Suggested mode ");
	ggiPrintMode(&mo);
	printf("\n");

	if (ggiSetMode(vis, &mo) != 0) {
		ggPanic("unable to set any mode at all, exiting!\n");
		return 2;
	}

	if (GT_SCHEME(mo.graphtype) == GT_PALETTE)
		ggiSetColorfulPalette(vis);
	kgidevice = is_kgicondev(vis);

	setcolors();

	if (guessmode() != 0) {
		fprintf(stderr,
			"Warning: Could not guess mode, probably not a standard mode\n");
	}

	testpattern(vis);

	mainmenu();

	ggDelStem(vis);

	ggiExit();

	giiExit();

	return 0;
}
