/* $Id: cbconsist.c,v 1.31 2008/11/06 20:45:13 pekberg Exp $
******************************************************************************

   This is a consistency-test and benchmark application for LibGGI
   crossblit operations.

   Written in 2002 by Brian S. Julin	[bri@tull.umassp.edu]

   This software is placed in the public domain and can be used freely
   for any purpose. It comes without any kind of warranty, either
   expressed or implied, including, but not limited to the implied
   warranties of merchantability or fitness for a particular purpose.
   Use it at your own risk. the author is not responsible for any damage
   or consequences raised by use or inability to use this program.

******************************************************************************
*/

/* TODO: 
 * 1) Allow specifying independent modes for each visual.
 * 2) Vary size of box in speed and consistency checks.
 * 3) Test layouts other than linear.
 * 4) Loop through all normal pixelformats.
 * 5) Test grayscale.
 * 6) Use CPU cycle counter when available to get better benchmarks
 *    and to throw out bad marks from task scheduling.
 */

#include "config.h"
#include <ggi/internal/ggi.h>
#include <ggi/gg.h>
#include <ggi/internal/gg_replace.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#ifdef HAVE_GETOPT_H
# include <getopt.h>
#endif
#ifdef HAVE_UNISTD_H
# include <unistd.h>
#endif

struct cbcstate_s;
typedef struct cbcstate_s cbcstate;

struct cbcstate_s {
	ggi_visual_t svis, dvis;
	const char *svisstr, *dvisstr;
	ggi_mode smode, dmode;
	ggi_pixel sblack, dblack;
	int flags;

#define CBC_ABORT	1	/* Whether to halt at first inconsistency */
#define CBC_REALSRC	2	/* Use a real visual as the source */
#define CBC_REALDST	4	/* Use a real visual as the destination */
#define CBC_FLUSHALOT	8	/* Do extra (slow) flushing to work 
				   around any races. */
#define CBC_NOTIMING	16	/* Don't time. */
#define CBC_NOCONSIST	32	/* Don't run consistency tests. */
#define CBC_NOPADDING	64      /* Don't check how any padding bits in
				   the source affects the consistency. */
};

#define BAILOUT(string, label) \
do {                           \
  fprintf(stderr, string);     \
  goto label;                  \
} while(0);

#define PROGNAME "cbconsist"
#define CBC_VERSION "1.0"

#define CBC_USAGE \
PROGNAME ", version " CBC_VERSION "\n\
Check the pixel correctness and speed of GGI crossblits.\n\
\n\
Synopsis: " PROGNAME " -h | [options]\n\
Available options: (default values are in parenthesis)\n\
  -s srcvis        The source visual (display-memory)\n\
  -d dstvis        The destination visual (display-memory)\n\
  -a               Abort when first inconsistency found.\n\
  -f               Flush a lot (for buggy targets. Very slow).\n\
  -t               Run without timing tests.\n\
  -c               Run without consistency tests.\n\
  -p               Ignore padding bits in source during consistency tests.\n\
\n\
Note that this application depends on bug-free color ops.\n\
Also note timing measurements don't account for system load.\n\
\n"

static void usage(FILE * fp)
{
	fprintf(fp, CBC_USAGE);
}

static ggi_pixel full_mask(ggi_visual_t vis)
{
	const ggi_pixelformat *pixfmt = ggiGetPixelFormat(vis);
	return  pixfmt->red_mask |
		pixfmt->green_mask |
		pixfmt->blue_mask |
		pixfmt->alpha_mask |
		pixfmt->clut_mask |
		pixfmt->fg_mask |
		pixfmt->bg_mask |
		pixfmt->texture_mask;
}

static ggi_pixel cbconsist(cbcstate * s)
{
	ggi_pixel bad;
	ggi_coord box;
	ggi_pixel count;
	int shift = 0;
	ggi_pixel mask = full_mask(s->dvis);

	box = s->smode.virt;
	if (box.x > s->dmode.virt.x)
		box.x = s->dmode.virt.x;
	if (box.y > s->dmode.virt.y)
		box.y = s->dmode.virt.y;
	bad = 0;

	if (s->flags & CBC_NOPADDING) {
		count = full_mask(s->svis);
		if (!count)
			count = 1;

		while (!(count & 1)) {
			count >>= 1;
			++shift;
		}
	}
	else
		count = 0xffffffff >> (32 - GT_SIZE(s->smode.graphtype));

	do {
		uint32_t num;
		ggi_pixel curr;

		num = box.x * box.y;
		if (num > count)
			num = count + 1;
		curr = count;
		while (num--) {
			ggiPutPixel(s->svis,
				(signed)(num % box.x),
				(signed)(num / box.x),
				curr << shift);
			if (s->flags & CBC_FLUSHALOT)
				ggiFlush(s->svis);
			curr--;
		}

		ggiFlush(s->svis);

		ggiCrossBlit(s->svis, 0, 0, box.x, box.y, s->dvis, 0, 0);
		ggiFlush(s->svis);
		ggiFlush(s->dvis);

		num = box.x * box.y;
		if (num > count)
			num = count + 1;
		curr = count;
		while (num--) {
			ggi_pixel res, correct;
			ggi_color col;

			ggiGetPixel(s->dvis,
				(signed)(num % box.x),
				(signed)(num / box.x),
				&res);
			ggiUnmapPixel(s->svis, curr << shift, &col);
			correct = ggiMapColor(s->dvis, &col);

			if (correct != res & mask) {
				if (s->flags & CBC_ABORT) {
					if (curr == 0) {
						/* This will be a pretty rare occurance. */
						fprintf(stderr,
							"Kludge -- error is actually at 0.\n");
						return 1;
					}
					return curr;
				}
				bad++;
			}

			curr--;
		}

		if (count < (unsigned) box.x * box.y)
			count = 0;
		else
			count -= box.x * box.y;
	} while (count);

	return bad;
}


static void cbtime(cbcstate * s)
{
	ggi_pixel bad;
	ggi_coord box;
	int num;
	ggi_pixel curr;
	struct timeval tvstart, tvend;

	box = s->smode.virt;
	if (box.x > s->dmode.virt.x)
		box.x = s->dmode.virt.x;
	if (box.y > s->dmode.virt.y)
		box.y = s->dmode.virt.y;
	bad = 0;

	num = box.x * box.y;
	while (num--) {
		curr = (rand() % 0x10000) | ((rand() & 0xffff) << 16);
		ggiPutPixel(s->svis, num % box.x, num / box.x, curr);
		if (s->flags & CBC_FLUSHALOT)
			ggiFlush(s->svis);
		curr--;
	}

	ggiFlush(s->svis);

	num = 0x10000000 / box.x / box.y;

	ggCurTime(&tvstart);

	while (num--) {
		ggiCrossBlit(s->svis, 0, 0, box.x, box.y, s->dvis, 0, 0);
		ggiFlush(s->svis);
		ggiFlush(s->dvis);
	}

	ggCurTime(&tvend);

	if (tvend.tv_usec < tvstart.tv_usec) {
		tvstart.tv_sec--;
		tvend.tv_usec += 1000000;
	}
	tvend.tv_sec -= tvstart.tv_sec;
	tvend.tv_usec -= tvstart.tv_usec;

	fprintf(stdout, "0x10000000 pixels in %i.%6.6i seconds\n",
		(int) tvend.tv_sec, (int) tvstart.tv_usec);
}

/* This is quickie code.  We may want to keep a table like this
 * to test common pixelformats first, but we should walk all 
 * possible pixel formats.
 */
#define MAX_MEMVIS_FMTS 8
static const char *memvis_fmts[MAX_MEMVIS_FMTS] = {
	"memory:-pixfmt=r5g6b5",
	"memory:-pixfmt=r4g4b4p4",
	"memory:-pixfmt=b5g6r5",
	"memory",
	"memory",
	"memory:-pixfmt=b8g8r8",
	"memory:-pixfmt=r8g8b8p8",
	"memory:-pixfmt=p8r8g8b8"
};

static ggi_graphtype memvis_gts[MAX_MEMVIS_FMTS] = {
	GT_16BIT,
	GT_16BIT,
	GT_16BIT,
	GT_8BIT,
	GT_4BIT,
	GT_24BIT,
	GT_32BIT,
	GT_32BIT
};

static int mkmemvis(int i, const char **str,
	     ggi_visual_t * vis, ggi_mode * mode, ggi_pixel * black)
{
	ggi_color color;

	if (*vis) {
		ggDelStem(*vis);
		*vis = NULL;
	}
	if (i > MAX_MEMVIS_FMTS - 1)
		return -1;
	*str = memvis_fmts[i];
	if ((*vis=ggNewStem(libggi, NULL)) == NULL) {
		fprintf(stderr,
			"unable to create stem, exiting.\n");
		return -1;
	}

	if (ggiOpen(*vis, *str, NULL) < 0) {
		ggDelStem(*vis);
		fprintf(stderr,
			"unable to open default visual, exiting.\n");
		return -1;
	}
	ggiSetFlags(*vis, GGIFLAG_ASYNC);
	if (ggiCheckSimpleMode
	    (*vis, GGI_AUTO, GGI_AUTO, 1, memvis_gts[i], mode)) {
		fprintf(stderr,
			"Could not find memory visual mode "
			"for gt %"PRIx32"!\n",
			memvis_gts[i]);
		return -1;
	}
	if (ggiSetMode(*vis, mode)) {
		fprintf(stderr,
			"Could not set up memory visual mode "
			"with gt %"PRIx32"!\n",
			memvis_gts[i]);
		return -1;
	}
	if (GT_SCHEME(mode->graphtype) == GT_PALETTE)
		ggiSetColorfulPalette(*vis);
	color.r = color.b = color.g = color.a = 0;
	*black = ggiMapColor(*vis, &color);
	ggiSetGCForeground(*vis, *black);
	ggiFillscreen(*vis);
	ggiFlush(*vis);
	return i;
}

static const char optstring[] = "s:d:haftcp";

int main(int argc, char * const argv[])
{
	ggi_color color;
	cbcstate s;
	int c;

	srand((unsigned)time(NULL));

	/* Default values */
	s.flags = 0;
	s.svis = NULL;
	s.dvis = NULL;

	/* Handle command-line options. */
	opterr = 0;
	while ((c = getopt(argc, argv, optstring)) != EOF) {
		switch (c) {

		case 'h':
			usage(stdout);
			return (0);

		case 's':
			s.flags |= CBC_REALSRC;
			s.svisstr = optarg;
			break;

		case 'd':
			s.flags |= CBC_REALDST;
			s.dvisstr = optarg;
			break;

		case 'a':
			s.flags |= CBC_ABORT;
			break;

		case 'f':
			s.flags |= CBC_FLUSHALOT;
			break;

		case 't':
			s.flags |= CBC_NOTIMING;
			break;

		case 'c':
			s.flags |= CBC_NOCONSIST;
			break;

		case 'p':
			s.flags |= CBC_NOPADDING;
			break;

		case '?':
		default:
			if (strchr(optstring, optopt))
				fprintf(stderr,
					PROGNAME ": option -%c requires"
					" an argument\n", optopt);
			else
				fprintf(stderr,
					PROGNAME
					": unrecognised option -%c\n",
					optopt);
			usage(stderr);
			return 1;
		}
	}

	if (ggiInit() != 0)
		BAILOUT("Unable to initialize LibGGI, exiting.\n", err0);

	if (s.flags & CBC_REALSRC) {
		char pixfmt[200];
		if ((s.svis=ggNewStem(libggi, NULL)) == NULL)
			BAILOUT("unable to create stem, exiting.\n", err1);
		if (ggiOpen(s.svis, s.svisstr, NULL) < 0)
			BAILOUT("unable to open source visual, exiting.\n", err1);
		ggiSetFlags(s.svis, GGIFLAG_ASYNC);
		ggiCheckSimpleMode(s.svis, GGI_AUTO, GGI_AUTO, 1, GT_AUTO,
				   &s.smode);
		if (ggiSetMode(s.svis, &s.smode) < 0) {
			ggPanic("Source Visual: Mode setting failed!\n");
			exit(1);
		}
		_ggi_build_pixfmtstr(GGI_VISUAL(s.svis),
			pixfmt, sizeof(pixfmt), GGI_PIXFMT_CHANNEL);
		printf("Source pixfmt:%s\n", pixfmt);
		if (GT_SCHEME(s.smode.graphtype) == GT_PALETTE)
			ggiSetColorfulPalette(s.svis);
		color.r = color.b = color.g = color.a = 0;
		s.sblack = ggiMapColor(s.svis, &color);
		ggiSetGCForeground(s.svis, s.sblack);
		ggiFillscreen(s.svis);
		ggiFlush(s.svis);
	}

	if (s.flags & CBC_REALDST) {
		char pixfmt[200];
		if ((s.dvis=ggNewStem(libggi, NULL)) == NULL)
			BAILOUT("unable to create stem, exiting.\n", err1);
		if (ggiOpen(s.dvis, s.dvisstr, NULL) < 0)
			BAILOUT("unable to open dest visual, exiting.\n", err1);
		ggiSetFlags(s.dvis, GGIFLAG_ASYNC);
		ggiCheckSimpleMode(s.dvis, GGI_AUTO, GGI_AUTO, 1, GT_AUTO,
				   &s.dmode);
		if (ggiSetMode(s.dvis, &s.dmode)) {
			ggPanic("Destination Visual: Mode setting failed!\n");
			exit(1);
		}
		_ggi_build_pixfmtstr(GGI_VISUAL(s.dvis),
			pixfmt, sizeof(pixfmt), GGI_PIXFMT_CHANNEL);
		printf("Dest pixfmt:%s\n", pixfmt);
		if (GT_SCHEME(s.dmode.graphtype) == GT_PALETTE)
			ggiSetColorfulPalette(s.dvis);
		color.r = color.b = color.g = color.a = 0;
		s.dblack = ggiMapColor(s.dvis, &color);
		ggiSetGCForeground(s.dvis, s.dblack);
		ggiFillscreen(s.dvis);
		ggiFlush(s.dvis);
	}

	if ((s.flags & CBC_REALDST) && (s.flags & CBC_REALSRC)) {
		ggi_pixel res;
		if (!(s.flags & CBC_NOTIMING)) {
			fprintf(stdout, "Timing  %s --> %s ...", s.svisstr,
				s.dvisstr);
			fflush(stdout);
			cbtime(&s);
		}
		if (!(s.flags & CBC_NOCONSIST)) {
			fprintf(stdout, "Testing %s --> %s ...", s.svisstr,
				s.dvisstr);
			fflush(stdout);
			res = cbconsist(&s);
			if (res && (s.flags & CBC_ABORT)) {
				fprintf(stdout,
					"\nBad value converting pixel "
					"value %"PRIx32".\n",
					res);
				goto err1;
			}
			fprintf(stdout, "%"PRId32" bad values.\n", res);
		}
	} else if (s.flags & CBC_REALDST) {
		int i;
		i = 0;
		while (mkmemvis
		       (i, &s.svisstr, &s.svis, &s.smode,
			&s.sblack) >= 0) {
			ggi_pixel res;

			if (!(s.flags & CBC_NOTIMING)) {
				fprintf(stdout, "Timing  %s --> %s ...",
					s.svisstr, s.dvisstr);
				fflush(stdout);
				cbtime(&s);
			}
			if (!(s.flags & CBC_NOCONSIST)) {
				fprintf(stdout, "Testing %s --> %s ...",
					s.svisstr, s.dvisstr);
				fflush(stdout);
				res = cbconsist(&s);
				if (res && (s.flags & CBC_ABORT)) {
					fprintf(stdout,
						"\nBad value converting "
						"pixel value %"PRIx32".\n",
						res);
					goto err1;
				}
				fprintf(stdout,
					"%"PRId32" bad values.\n", res);
			}
			i++;
		}
	} else if (s.flags & CBC_REALSRC) {
		int i;
		i = 0;
		while (mkmemvis
		       (i, &s.dvisstr, &s.dvis, &s.dmode,
			&s.dblack) >= 0) {
			ggi_pixel res;

			if (!(s.flags & CBC_NOTIMING)) {
				fprintf(stdout, "Timing  %s --> %s ...",
					s.svisstr, s.dvisstr);
				fflush(stdout);
				cbtime(&s);
			}
			if (!(s.flags & CBC_NOCONSIST)) {
				fprintf(stdout, "Testing %s --> %s ...",
					s.svisstr, s.dvisstr);
				fflush(stdout);
				res = cbconsist(&s);
				if (res && (s.flags & CBC_ABORT)) {
					fprintf(stdout,
						"\nBad value converting pixel value %"PRIx32".\n",
						res);
					goto err1;
				}
				fprintf(stdout, "%"PRId32" bad values.\n", res);
			}
			i++;
		}
	} else {
		int i;
		i = 0;
		while (mkmemvis
		       (i, &s.dvisstr, &s.dvis, &s.dmode,
			&s.dblack) >= 0) {
			int j;
			j = 0;
			while (mkmemvis
			       (j, &s.svisstr, &s.svis, &s.smode,
				&s.sblack) >= 0) {
				ggi_pixel res;

				if (!(s.flags & CBC_NOTIMING)) {
					fprintf(stdout,
						"Timing  %s --> %s ...",
						s.svisstr, s.dvisstr);
					fflush(stdout);
					cbtime(&s);
				}
				if (!(s.flags & CBC_NOCONSIST)) {
					fprintf(stdout,
						"Testing %s --> %s ...",
						s.svisstr, s.dvisstr);
					fflush(stdout);
					res = cbconsist(&s);
					if (res && (s.flags & CBC_ABORT)) {
						fprintf(stdout,
							"\nBad value converting pixel value %"PRIx32".\n",
							res);
						goto err1;
					}
					fprintf(stdout,
						"%"PRId32" bad values.\n",
						res);
				}
				j++;
			}
			i++;
		}
	}

      err1:
	if (s.dvis)
		ggDelStem(s.dvis);
	if (s.svis)
		ggDelStem(s.svis);

	ggiExit();
      err0:
	exit(1);
}
