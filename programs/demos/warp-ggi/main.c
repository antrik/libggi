/* $Id: main.c,v 1.3 2004/02/02 19:22:01 cegger Exp $
******************************************************************************
  
   Warp-GGI
 
   General Graphics Interface demo: realtime picture 'gooing'
  
   Written by Emmanuel Marty <core@ggi-project.org>
   Thanks to Jaromir Koutek <miri@punknet.cz> for TGA reader and hacks.
  
   This software is placed in the public domain and can be used
   freely for any purpose. It comes with absolutely NO WARRANTY,
   either expressed or implied, including, but not limited to the
   implied warranties of merchantability or fitness for a particular
   purpose.  USE IT AT YOUR OWN RISK. The author is not responsible
   for any damage or consequences raised by use or inability to use
   this program.

******************************************************************************
*/

#include <ggi/ggi.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "rawpict.h"
#include "warp.h"


/* Selects the best display mode for the picture, if possible */

static int checkmode(uint32 width, uint32 height, uint32 depth,
		     uint32 * swidth, uint32 * sheight, uint32 * smode,
		     uint32 * spixsize)
{

	switch (depth) {
	case 8:
		*smode = GT_CONSTRUCT(8, GT_PALETTE, 8);
		*spixsize = 1;
		break;

	case 15:
		*smode = GT_CONSTRUCT(15, GT_TRUECOLOR, 16);
		*spixsize = 2;
		break;

	case 16:
		*smode = GT_CONSTRUCT(16, GT_TRUECOLOR, 16);
		*spixsize = 2;
		break;

	case 24:
		*smode = GT_CONSTRUCT(24, GT_TRUECOLOR, 24);
		*spixsize = 3;
		break;

	case 32:
		*smode = GT_CONSTRUCT(24, GT_TRUECOLOR, 32);
		*spixsize = 4;
		break;

	default:
		return 0;
	}

	if (!(width || height))
		return 0;

	if (width <= 320)
		*swidth = 320;
	else if (width <= 640)
		*swidth = 640;
	else if (width <= 800)
		*swidth = 800;
	else if (width <= 1024)
		*swidth = 1024;
	else if (width <= 1280)
		*swidth = 1280;
	else if (width <= 1600)
		*swidth = 1600;
	else if (width <= 2048)
		*swidth = 2048;
	else
		*swidth = (width + 63) & 0x7FFFFFC0;

	if (height <= 200)
		*sheight = 200;
	else if (height <= 240)
		*sheight = 240;
	else if (height <= 400)
		*sheight = 400;
	else if (height <= 480)
		*sheight = 480;
	else if (height <= 600)
		*sheight = 600;
	else if (height <= 1024)
		*sheight = 1024;
	else
		*sheight = (height + 64) & 0x7FFFFFC0;

	return 1;
}

/* error handling */

#define fail(reason) { fprintf (stderr, reason); fflush (stderr); done = -1; }


typedef void (func_warpfunc) (struct warp * w, sint32 xw, sint32 yw,
			  sint32 cw);

/* main program	*/

int main(int argc, char **argv)
{
	ggi_visual_t disp = NULL;
	const ggi_directbuffer *dbuf = NULL;
	struct raw_pict rp;
	struct warp *w = NULL;
	char *picname = NULL;
	uint32 swidth, sheight, smode, spixsize;
	uint32 width, height, udepth = 0, stride;
	sint32 err, step = 0, done = 0;
	sint32 nextarg, i, tval;

	func_warpfunc	*warpfunc;

	rp.clut = NULL;
	while (!done)
		switch (step) {
		case 0:
			i = 1;
			nextarg = 0;

			while ((i < argc) && (!done)) {
				if ((argv[i]) && (argv[i][0] == '-')) {
					if (!strcmp(argv[i], "-d"))
						nextarg = 1;
					else
						done = -1;
				} else {
					if (!nextarg) {
						if (!picname)
							picname = argv[i];
						else
							done = -1;
					} else
						udepth = atoi(argv[i]);
				}

				i++;
			}

			if (!done)
				step++;
			else
				fail("Usage: warp [picture.[pcx|tga]] [-d depth]\n");
			break;

		case 1:
			if (!picname)
				picname = "leeloo.pcx";

			if (((err = readTGA(picname, &rp, udepth))) &&
			    ((err = readPCX(picname, &rp, udepth))))
				switch (err) {
				case RPREAD_NOMEM:
					fail("Not enough memory (file).\n");
					break;

				case RPREAD_NOFILE:
					fail("Can't open picture.\n");
					break;

				case RPREAD_READERR:
					fail("Error loading picture.\n");
					break;

				case RPREAD_BADFMT:
					fail("Bad picture type.\n");
					break;

				default:
					fail("Unknown error reading picture.\n");
					break;
			} else {
				if (!udepth) {
					if (rp.depth == 24)
						udepth = 16;
					else
						udepth = rp.depth;
				}
				if ((udepth > 8) || (rp.depth == 8))
					step++;
				else
					fail("No color quantization.\n");
			}
			break;

		case 2:
			if (checkmode((uint32)rp.width, (uint32)rp.height, 
				      udepth, &swidth, &sheight, &smode,
				      &spixsize))
				step++;
			else
				fail("Unsupported picture format.\n");
			break;

		case 3:
			if (ggiInit() < 0)
				fail("Error initializing LibGGI.\n");

			if ((disp = ggiOpen(NULL)))
				step++;
			else
				fail("Error opening GGI display.\n");
			break;

		case 4:
			if (!(err = ggiSetSimpleMode(disp,
						(signed)swidth, 
						(signed)sheight,
						GGI_AUTO, smode)))
				step++;
			else {
				fprintf(stderr,
					"Error switching to %dx%d %d bpp mode.\n"
					"Under X, try using -d depth_of_your_screen.\n",
					swidth, sheight, udepth);
				done = -1;
			}
			break;

		case 5:
			if ((dbuf = ggiDBGetBuffer(disp, 0)))
				step++;
			else
				fail("Error getting display buffer.\n");
			break;

		case 6:
			if ((dbuf->type & GGI_DB_SIMPLE_PLB))
				step++;
			else
				fail("Error: nonlinear display buffer.\n");
			break;

		case 7:
			if ((w = initWarp((unsigned)rp.width,
					  (unsigned)rp.height, spixsize,
					  rp.framebuf,
					  rp.width * spixsize)))
				step++;
			else
				fail("Not enough memory.\n");
			break;

		case 8:
			if (udepth != (unsigned)rp.depth)
				convertbpp(&rp, udepth);

			if ((udepth <= 8) && (rp.clut))
				ggiSetPalette(disp, 0, 1 << rp.depth,
					      rp.clut);

			width = rp.width * spixsize;
			height = rp.height;

			if (spixsize == 1)
				warpfunc = doWarp8bpp;
			else if (spixsize == 2)
				warpfunc = doWarp16bpp;
			else if (spixsize == 3)
				warpfunc = doWarp24bpp;
			else
				warpfunc = doWarp32bpp;

			while (ggiKbhit(disp));

			tval = 0;

			while (!ggiKbhit(disp)) {
				sint32 xw, yw, cw;
				char *src, *dest;

				xw = (int) (sin((tval + 100) * M_PI / 128)
					    * 30);
				yw = (int) (sin((tval) * M_PI / 256) *
					    -35);
				cw = (int) (sin((tval - 70) * M_PI / 64) *
					    50);
				xw +=
				    (int) (sin((tval - 10) * M_PI / 512) *
					   40);
				yw +=
				    (int) (sin((tval + 30) * M_PI / 512) *
					   40);

				warpfunc(w, xw, yw, cw);

				src = w->framebuf;

				/* Acquire DirectBuffer before we use it. */
				if (ggiResourceAcquire
				    (dbuf->resource,
				     GGI_ACTYPE_WRITE) != 0) {
					fail("Error acquiring DirectBuffer\n");
				}
				dest = dbuf->write;
				stride = dbuf->buffer.plb.stride;

				/* one would wait start of vblank here. */
				if (width == stride) {
					memcpy(dest, src, width * height);
				} else {
					uint32 y;

					for (y = 0; y < height; y++) {
						memcpy(dest, src, width);
						src += width;
						dest += stride;
					}
				}

				/* Release DirectBuffer when done with it. */
				ggiResourceRelease(dbuf->resource);

				tval = (tval + 1) & 511;
			}

			done = 1;
			break;
		}

	if (step > 3)
		ggiClose(disp);

	if (step > 2)
		disposeWarp(w);

	if (step > 1)
		free(rp.framebuf);
	if (rp.clut)
		free(rp.clut);

	if (done > 0)
		fail("Warp by Emmanuel Marty <core@ggi-project.org>\n");
	return ((done > 0) ? 0 : 2);
}
