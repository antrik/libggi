/*
 * tunemode.c - Author 1998 Andreas Beck   becka@ggi-project.org
 *
 * User mode program for timing-list-driver. Allows interactive adjust-
 * ment of modes.
 *
 *   This software is placed in the public domain and can be used freely
 *   for any purpose. It comes without any kind of warranty, either
 *   expressed or implied, including, but not limited to the implied
 *   warranties of merchantability or fitness for a particular purpose.
 *   Use it at your own risk. the author is not responsible for any damage
 *   or consequences raised by use or inability to use this program.
 *
 */

/* NEVER do this in your own programs. They will break without warning. */

#include <kgi/kgi.h>
#include <kgi/kgi_commands.h>

#include <ggi/ggi.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>


/* We want to draw circles, but not use LibGGI2D.
 */
#include "mycircle.inc"

ggi_visual_t vis;

int widthx, widthy;

struct kgi_monitor mondesc = {
	"Unknown Manufacturer",
	"Unknown Model",
	0,
	0,
	{800, 600},
	{285, 213},		/* Phys size. */
	0,			/* disptype */
	0,			/* syntype */
	{0, 0},			/* bandwidth */
	{0, 0},			/* hfreq */
	{0, 0},			/* vfreq */
	{0, 0, 0},		/* white */
	1,			/* gamma */
};

struct kgi_preset prefmode[50] = { {25175000,
				    {640, 640, 680, 776, 784, 800, 0},
				    {400, 407, 412, 414, 442, 449, 1}
				    },
};

int pref_num = 1;

struct kgi_monitor *mondptr = &mondesc;
struct kgi_preset trymode;
struct kgi_preset oldmode;

static int verbose = 2;

void beep(void)
{
	fputs("\a", stderr);
}

void toup(char *what)
{
	do {
		*what = toupper(*what);
	} while (*what++);
}

#define verb(a,b) do{ if (verbose>=a) printf(b,token);}while(0)

int ggi_ioctl(int arg, void *data)
{
	return ioctl(vis->fd, arg, data);
}

void parse_file(char *name)
{
	char buffer[1024], *token;
	double hlp;
	int ignoreflag = 1;
	FILE *infile;

	if ((infile = fopen(name, "r")) == NULL)
		ggiPanic("Can't open input-file !");

	while (!feof(infile)) {
		fgets(buffer, 1024, infile);
		if (*buffer == '#' || *buffer == '\n')
			continue;
		toup(buffer);
		token = strtok(buffer, " \t\n");
		if (strcmp(token, "SECTION") == 0) {
			if (strcmp(strtok(NULL, "\" "), "MONITOR") == 0)
				ignoreflag = 0;
		}
		if (ignoreflag)
			continue;
		if (strcmp(token, "ENDSECTION") == 0)
			break;
		else if (strcmp(token, "MODELINE") == 0) {
			token = strtok(NULL, "\" ");
			verb(1, "Loading Mode: %s\n");
			sscanf(token = strtok(NULL, " \t\n"), "%lf", &hlp);
			verb(2, "%s  ");
			prefmode[pref_num].dclk = 1000000 * hlp;	/* MHz ! */
			prefmode[pref_num].x.width = atoi(token =
							  strtok(NULL,
								 " \t\n"));
			verb(2, "%s ");
			prefmode[pref_num].x.blankstart =
			    prefmode[pref_num].x.syncstart = atoi(token =
								  strtok
								  (NULL,
								   " \t\n"));
			verb(2, "%s ");
			prefmode[pref_num].x.blankend =
			    prefmode[pref_num].x.syncend = atoi(token =
								strtok
								(NULL,
								 " \t\n"));
			verb(2, "%s ");
			prefmode[pref_num].x.total = atoi(token =
							  strtok(NULL,
								 " \t\n"));
			verb(2, "%s  ");
			prefmode[pref_num].y.width = atoi(token =
							  strtok(NULL,
								 " \t\n"));
			verb(2, "%s ");
			prefmode[pref_num].y.blankstart =
			    prefmode[pref_num].y.syncstart = atoi(token =
								  strtok
								  (NULL,
								   " \t\n"));
			verb(2, "%s ");
			prefmode[pref_num].y.blankend =
			    prefmode[pref_num].y.syncend = atoi(token =
								strtok
								(NULL,
								 " \t\n"));
			verb(2, "%s ");
			prefmode[pref_num].y.total = atoi(token =
							  strtok(NULL,
								 " \t\n"));
			verb(2, "%s\n");
			if ((token = strtok(NULL, " \t\n"))) {
				puts("Warning: Flags are not yet handled !");
			}
			pref_num++;
		} else if (strcmp(token, "VENDORNAME") == 0) {
			token = strtok(NULL, "\" ");
			verb(1, "Manufacturer : %s\n");
			strncpy(mondesc.manufact, token,
				sizeof(mondesc.manufact));
			mondesc.manufact[sizeof(mondesc.manufact) - 1] =
			    '\0';
		} else if (strcmp(token, "MODELNAME") == 0) {
			token = strtok(NULL, "\" ");
			verb(1, "Model : %s\n");
			strncpy(mondesc.model, token,
				sizeof(mondesc.model));
			mondesc.model[sizeof(mondesc.model) - 1] = '\0';
		} else if (strcmp(token, "BANDWIDTH") == 0) {
			puts(token = strtok(NULL, "-"));
		} else if (strcmp(token, "HORIZSYNC") == 0) {
			sscanf(token = strtok(NULL, "-"), "%lf", &hlp);
			mondesc.hfreq.min = hlp * 1000;
			verb(1, "HSync : %s - ");
			sscanf(token = strtok(NULL, " \t\n"), "%lf", &hlp);
			mondesc.hfreq.max = hlp * 1000;
			verb(1, "%s\n");
		} else if (strcmp(token, "VERTREFRESH") == 0) {
			mondesc.vfreq.min = atoi(token =
						 strtok(NULL, "-"));
			verb(1, "VSync : %s - ");
			mondesc.vfreq.max = atoi(token =
						 strtok(NULL, " \t\n,"));
			verb(1, "%s\n");
		}
	}
	if (ggi_ioctl(MONITOR_SETINFO, &mondptr)) {
		perror("ggi_ioctl : ");
	}
}

#define ggiPrintf(vis,x,y,args...) \
	do { char buffer[1024]; sprintf(buffer,args); ggiPuts(vis,x,y,buffer); } while(0)

enum movemode { MM_MOVE = 0x000000,
	MM_RESIZE = 0x010000,
	MM_POLARITY = 0x020000,
	MM_SYNCWIDTH = 0x030000,
	MM_BMOVE = 0x040000,
	MM_BRESIZE = 0x050000,
	MM_STEP = 0x010000
};

char *mmstring[] = { "Move", "Resize",
	"Polarity", "Syncwidth",
	"BlankMove", "BlankResize"
};

void testpic(struct kgi_preset *pp, enum movemode mm)
{
	int mx, my, x;
	static ggi_color pal[256] = { {0x0000, 0x0000, 0xff00},	/* blue */
	{0xff00, 0xff00, 0xff00},	/* white */
	{0x8000, 0x8000, 0x8000},	/* grey  */
	{0xff00, 0x8000, 0x8000},	/* ltred */
	{0xa000, 0xa000, 0xa000},	/* ltgrey */
	{0x5000, 0xff00, 0x5000},	/* green */
	{0x0000, 0x0000, 0x0000},	/* black */
	};


	ggiSetPalette(vis, 0, 7, pal);
	ggiSetGCForeground(vis, 6);
	ggiSetGCBackground(vis, 6);
	ggiFillscreen(vis);
	ggiSetGCForeground(vis, 1);
	ggiDrawHLine(vis, 0, 0, widthx);
	ggiDrawHLine(vis, 0, widthy - 1, widthx);
	ggiDrawVLine(vis, 0, 0, widthy);
	ggiDrawVLine(vis, widthx - 1, 0, widthy);

	ggiSetGCForeground(vis, 2);
	for (x = (mx = (widthx / 2)) % 50; x < widthx - 1; x += 50)
		if (x > 0)
			ggiDrawVLine(vis, x, 1, widthy - 2);
	for (x = (my = (widthy / 2)) % 50; x < widthy - 1; x += 50)
		if (x > 0)
			ggiDrawHLine(vis, 1, x, widthx - 2);

	ggiSetGCForeground(vis, 3);
	for (x = 1; x < 20; x++) {
		ggiDrawPixel(vis, x, x);
		ggiDrawPixel(vis, widthx - 1 - x, x);
		ggiDrawPixel(vis, widthx - 1 - x, widthy - 1 - x);
		ggiDrawPixel(vis, x, widthy - 1 - x);
	}

	ggiSetGCForeground(vis, 4);
	myDrawCircle(vis, mx, my, my - my % 10 - 10);

	ggiSetGCForeground(vis, 5);
	ggiPrintf(vis, 10, my + 1,
		  "        wdth blnk sync send bend total");
	ggiPrintf(vis, 10, my + 17, "Xsync%c: %4d %4d %4d %4d %4d %4d",
		  pp->x.polarity > 0 ? '+' : '-', pp->x.width,
		  pp->x.blankstart, pp->x.syncstart, pp->x.syncend,
		  pp->x.blankend, pp->x.total);
	ggiPrintf(vis, 10, my + 33, "Ysync%c: %4d %4d %4d %4d %4d %4d",
		  pp->y.polarity > 0 ? '+' : '-', pp->y.width,
		  pp->y.blankstart, pp->y.syncstart, pp->y.syncend,
		  pp->y.blankend, pp->y.total);
	ggiPrintf(vis, 10, my + 49, "Freq  : %5.3fMhz/%5.2fkHz/%5.2fHz",
		  pp->dclk / 1000000.0, 0.001 * pp->dclk / pp->x.total,
		  1.0 * pp->dclk / pp->x.total / pp->y.total);
	ggiPrintf(vis, 10, my + 66, "Press 'h' for help.");
	ggiPrintf(vis, 10 + 150, my + 66, mmstring[mm / MM_STEP]);
}

void cls(void)
{
	printf("\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n");
}

void interactive(int wantx, int wanty)
{
	int y;

	enum movemode mmode = 0;

	cls();

	oldmode.x.width = wantx;
	oldmode.y.width = wanty;
	if (ggi_ioctl(MONITOR_GETPRESETMODE, &oldmode)) {
		perror("getpresetmode : ");
	}
	trymode = oldmode;
	while (1) {
		if (ggi_ioctl(MONITOR_SETPRESETMODE, &trymode)) {
			perror("setpresetmode : ");
		}
#ifdef DEBUG
		printf("trymode: x: %d %d %d %d %d %d\n"
		       "         y: %d %d %d %d %d %d\n",
		       trymode.x.width, trymode.x.blankstart,
		       trymode.x.syncstart, trymode.x.syncend,
		       trymode.x.blankend, trymode.x.total,
		       trymode.y.width, trymode.y.blankstart,
		       trymode.y.syncstart, trymode.y.syncend,
		       trymode.y.blankend, trymode.y.total);
#endif

		if (ggiSetSimpleMode(vis, widthx, widthy, 1, GT_8BIT))
			return;

		testpic(&trymode, mmode);
	      retry:
		switch ((y = ggiGetc(vis)) | mmode) {
		case MM_MOVE | GIIK_Left:
			if (trymode.x.syncend + 8 < trymode.x.total) {
				trymode.x.syncstart += 8;
				trymode.x.syncend += 8;
				if (trymode.x.blankend < trymode.x.syncend)
					trymode.x.blankend += 8;
			} else
				beep();
			break;
		case MM_MOVE | GIIK_Right:
			if (trymode.x.syncstart - 8 > trymode.x.width) {
				trymode.x.syncstart -= 8;
				trymode.x.syncend -= 8;
				if (trymode.x.blankstart >
				    trymode.x.syncstart)
					trymode.x.blankstart -= 8;
			} else
				beep();
			break;
		case MM_MOVE | GIIK_Up:
			if (trymode.y.syncend + 1 < trymode.y.total) {
				trymode.y.syncstart++;
				trymode.y.syncend++;
				if (trymode.y.blankend < trymode.y.syncend)
					trymode.y.blankend++;
			} else
				beep();
			break;
		case MM_MOVE | GIIK_Down:
			if (trymode.y.syncstart - 1 > trymode.y.width) {
				trymode.y.syncstart--;
				trymode.y.syncend--;
				if (trymode.y.blankstart >
				    trymode.y.syncstart)
					trymode.y.blankstart--;
			} else
				beep();
			break;
		case MM_BMOVE | GIIK_Right:
			if (trymode.x.blankend + 8 <= trymode.x.total &&
			    trymode.x.blankstart + 8 <=
			    trymode.x.syncstart) {
				trymode.x.blankstart += 8;
				trymode.x.blankend += 8;
			} else
				beep();
			break;
		case MM_BMOVE | GIIK_Left:
			if (trymode.x.blankstart - 8 >= trymode.x.width &&
			    trymode.x.blankend - 8 >= trymode.x.syncend) {
				trymode.x.blankstart -= 8;
				trymode.x.blankend -= 8;
			} else
				beep();
			break;
		case MM_BMOVE | GIIK_Down:
			if (trymode.y.blankend + 1 <= trymode.y.total &&
			    trymode.y.blankstart + 1 <=
			    trymode.y.syncstart) {
				trymode.y.blankstart++;
				trymode.y.blankend++;
			} else
				beep();
			break;
		case MM_BMOVE | GIIK_Up:
			if (trymode.y.blankstart - 1 >= trymode.y.width &&
			    trymode.y.blankend - 1 >= trymode.y.syncend) {
				trymode.y.blankstart--;
				trymode.y.blankend--;
			} else
				beep();
			break;
		case MM_BRESIZE | GIIK_Right:
			if (trymode.x.blankend + 8 <= trymode.x.total)
				trymode.x.blankend += 8;
			if (trymode.x.blankstart - 8 >= trymode.x.width)
				trymode.x.blankstart -= 8;
			break;
		case MM_BRESIZE | GIIK_Left:
			if (trymode.x.blankstart + 8 <=
			    trymode.x.syncstart)
				trymode.x.blankstart += 8;
			if (trymode.x.blankend - 8 >= trymode.x.syncend)
				trymode.x.blankend -= 8;
			break;
		case MM_BRESIZE | GIIK_Down:
			if (trymode.y.blankend + 1 <= trymode.y.total)
				trymode.y.blankend += 1;
			if (trymode.y.blankstart - 1 >= trymode.y.width)
				trymode.y.blankstart -= 1;
			break;
		case MM_BRESIZE | GIIK_Up:
			if (trymode.y.blankstart + 1 <=
			    trymode.y.syncstart)
				trymode.y.blankstart += 1;
			if (trymode.y.blankend - 1 >= trymode.y.syncend)
				trymode.y.blankend -= 1;
			break;
		case MM_RESIZE | GIIK_Up:
			if (trymode.y.total - 1 > trymode.y.blankend)
				trymode.y.total--;
			else
				beep();
			break;
		case MM_RESIZE | GIIK_Down:
			trymode.y.total++;
			break;
		case MM_RESIZE | GIIK_Left:
			if (trymode.x.total - 8 > trymode.x.blankend)
				trymode.x.total -= 8;
			else
				beep();
			break;
		case MM_RESIZE | GIIK_Right:
			trymode.x.total += 8;
			break;
		case MM_POLARITY | GIIK_Left:
			trymode.x.polarity = 0;
			break;
		case MM_POLARITY | GIIK_Right:
			trymode.x.polarity = 1;
			break;
		case MM_POLARITY | GIIK_Up:
			trymode.y.polarity = 0;
			break;
		case MM_POLARITY | GIIK_Down:
			trymode.y.polarity = 1;
			break;
		case 'h':
		case 'H':
			/* Display help ... */
			break;
		case MM_SYNCWIDTH | GIIK_Left:
			if (trymode.x.syncend - 8 > trymode.x.syncstart)
				trymode.x.syncend -= 8;
			else
				beep();
			break;
		case MM_SYNCWIDTH | GIIK_Right:
			if (trymode.x.syncend + 8 < trymode.x.total) {
				trymode.x.syncend += 8;
				if (trymode.x.blankend < trymode.x.syncend)
					trymode.x.blankend += 8;
			} else
				beep();
			break;
		case MM_SYNCWIDTH | GIIK_Up:
			if (trymode.y.syncend - 1 > trymode.y.syncstart)
				trymode.y.syncend--;
			else
				beep();
			break;
		case MM_SYNCWIDTH | GIIK_Down:
			if (trymode.y.syncend + 1 < trymode.y.total) {
				trymode.y.syncend++;
				if (trymode.y.blankend < trymode.y.syncend)
					trymode.y.blankend++;
			} else
				beep();
			break;
		}
		switch (y) {
		case GIIK_Enter:
			oldmode = trymode;
			break;
		case GIIK_Left:
		case GIIK_Right:
		case GIIK_Up:
		case GIIK_Down:
		case 27:
			break;
		case 'h':
		case 'H':	/* Display help ... */
			break;
		case 'm':
		case 'M':
			mmode = MM_MOVE;
			break;
		case 'r':
		case 'R':
			mmode = MM_RESIZE;
			break;
		case 'p':
		case 'P':
			mmode = MM_POLARITY;
			break;
		case 's':
		case 'S':
			mmode = MM_SYNCWIDTH;
			break;
		case 'b':
		case 'B':
			mmode = MM_BMOVE;
			break;
		case 'e':
		case 'E':
			mmode = MM_BRESIZE;
			break;

		default:
			printf("Unknown key: 0x%x\n", y);
			goto retry;
		}
		if (y == GIIK_Enter || y == 27)
			break;
	}
	if (ggi_ioctl(MONITOR_SETPRESETMODE, &oldmode)) {
		perror("setpresetmode : ");
	}
}

#if 0

void download(void)
{
	int x;

	if (ggi_ioctl(MONITOR_CLEARPREF, NULL))
		ggiPanic
		    ("Clear preferred modes failed.Is the timelist-driver loaded ?");

	for (x = 0; x < pref_num; x++) {
		prefptr = &prefmode[x];
		if (ggi_ioctl(MONITOR_SETPREFMODE, &prefptr)) {
			perror("setprefmode : ");
		}
	}
}

void save(char *name)
{
	int x;
	FILE *out;

	if ((out = fopen(name, "w")) == NULL) {
		perror("save : ");
		return;
	}

	fputs("SECTION \"MONITOR\"\n", out);
	fprintf(out, "VENDORNAME \"%s\"\n", mondesc.manufact);
	fprintf(out, "MODELNAME \"%s\"\n", mondesc.model);
	fprintf(out, "HORIZSYNC %d-%d\n", mondesc.hfreq.min / 1000,
		mondesc.hfreq.max / 1000);
	fprintf(out, "VERTREFRESH %d-%d\n", mondesc.vfreq.min,
		mondesc.vfreq.max);
	for (x = 1; x < pref_num; x++) {
		prefptr = &prefmode[x];
		fprintf(out,
			"ModeLine \"%dx%d\"\t %4.3f %5d %5d %5d %5d  %5d %5d %5d %5d\n",
			prefptr->x.width, prefptr->y.width,
			prefptr->clock / 1000000.0, prefptr->xwidth,
			prefptr->xsyncstart, prefptr->xsyncend,
			prefptr->xend, prefptr->ywidth,
			prefptr->ysyncstart, prefptr->ysyncend,
			prefptr->yend);
	}
	fputs("ENDSECTION\n", out);

}
#endif

int main(int argc, char *argv[])
{
	int x, y;

	if (argc < 3) {

		fprintf(stderr, "usage: %s sizex sizey\n", argv[0]);
		return 1;
	}

	if (ggiInit() != 0) {
		fprintf(stderr,
			"%s: unable to initialize LibGGI, exiting.\n",
			argv[0]);
		exit(1);
	}
	widthx = x = atoi(argv[1]);
	widthy = y = atoi(argv[2]);

	if ((vis = ggiOpen(NULL)) == NULL) {
		fprintf(stderr,
			"%s: unable to open default visual, exiting.\n",
			argv[0]);
		exit(1);
	}

	/* Make sure at least 640x400 is reasonable for crash-recovery */
	if (ggi_ioctl(MONITOR_SETPRESETMODE, &prefmode[0])) {
		perror("setprefmode : ");
	}

	if (ggiSetSimpleMode(vis, widthx, widthy, 1, GT_8BIT))
		return 1;
	if (x < 400)
		x *= 2;
	if (y < 300)
		y *= 2;

	/* Fixme ... need something better : */


#if 0
	parse_file(argv[1]);

	if (argc < 3)
		download();
	else {
		interactive();
		download();
		save(argv[2]);
	}
#endif

	interactive(x, y);

	ggiExit();
	return (0);
}
