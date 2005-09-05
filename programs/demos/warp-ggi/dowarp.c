/* $Id: dowarp.c,v 1.6 2005/09/05 01:32:10 pekberg Exp $
******************************************************************************
  
   Warp-GGI
 
   General Graphics Interface demo: realtime picture 'gooing'
   Written by Emmanuel Marty <core@ggi-project.org>

   dowarp.c: warp routines
  
   This is a demonstration of LibGGI's functions and can be used as a
   reference programming example.
  
   This software is placed in the public domain and can be used
   freely for any purpose. It comes with absolutely NO WARRANTY,
   either expressed or implied, including, but not limited to the
   implied warranties of merchantability or fitness for a particular
   purpose.  USE IT AT YOUR OWN RISK. The author is not responsible
   for any damage or consequences raised by use or inability to use
   this program.

******************************************************************************
*/

#include "config.h"
#include <ggi/ggi.h>
#include <sys/types.h>
#include <stdlib.h>

#include "rawpict.h"
#include "warp.h"

static void initSinTable(struct warp *w)
{
	int32_t *tptr, *tsinptr;
	double i;

	tsinptr = tptr = w->sintable;

	for (i = 0; i < 1024; i++)
		*tptr++ = (int) (sin(i * M_PI / 512) * 32767);

	for (i = 0; i < 256; i++)
		*tptr++ = *tsinptr++;
}

static void initOffsTable(struct warp *w)
{
	int32_t width, height, len, y;
	uint8_t *source;
	void **offptr;

	offptr = w->offstable;
	width = w->width;
	height = w->height;
	source = (uint8_t *) w->source;
	len = w->srclinelen;

	for (y = 0; y < height; y++) {
		*offptr++ = (void *) source;
		source += len;
	}
}

static void initDistTable(struct warp *w)
{
	int32_t halfw, halfh, *distptr;
	double x, y, m;

	halfw = w->width >> 1;
	halfh = w->height >> 1;

	distptr = w->disttable;

	m = sqrt((double) (halfw * halfw + halfh * halfh));

	for (y = -halfh; y < halfh; y++)
		for (x = -halfw; x < halfw; x++)
			*distptr++ =
			    ((int) ((sqrt(x * x + y * y) * 511.9999) / m))
			    << 1;
}

struct warp *initWarp(uint32_t width, uint32_t height, uint32_t pixsize,
		      void *source, uint32_t srclinelen)
{
	struct warp *w;

	if ((w = (struct warp *) malloc(sizeof(struct warp)))) {
		if ((w->offstable = malloc(height * sizeof(char *)))) {

			if ((w->disttable =
			     malloc(width * height * sizeof(int)))) {
				if ((w->framebuf =
				     malloc(width * height * pixsize))) {
					w->width = width;
					w->height = height;
					w->pixsize = pixsize;
					w->source = source;
					w->srclinelen = srclinelen;

					initSinTable(w);
					initOffsTable(w);
					initDistTable(w);

					return (w);
				}
				free(w->disttable);
			}
			free(w->offstable);
		}
		free(w);
	}

	return (NULL);
}

void disposeWarp(struct warp *w)
{
	if (w) {
		free(w->framebuf);
		free(w->disttable);
		free(w->offstable);
		free(w);
	}
}

void doWarp8bpp(struct warp *w, int32_t xw, int32_t yw, int32_t cw)
{
	int32_t c, i, x, y, dx, dy, maxx, maxy;
	int32_t width, height, *ctable, *ctptr, *distptr;
	int32_t *sintable, *disttable;
	uint8_t *destptr, **offstable;

	ctptr = ctable = &(w->ctable[0]);
	sintable = &(w->sintable[0]);
	offstable = (uint8_t **) w->offstable;
	distptr = disttable = w->disttable;
	width = w->width;
	height = w->height;
	destptr = (uint8_t *) w->framebuf;

	c = 0;

	for (x = 0; x < 512; x++) {
		i = (c >> 3) & 0x3FE;
		*ctptr++ = ((sintable[i] * yw) >> 15);
		*ctptr++ = ((sintable[i + 256] * xw) >> 15);
		c += cw;
	}

	maxx = width - 1;
	maxy = height - 1;

	for (y = 0; y < height; y++)
		for (x = 0; x < width; x++) {
			i = *distptr++;
			dx = ctable[i + 1] + x;
			dy = ctable[i] + y;

			if (dx < 0)
				goto clipxmin;
			if (dx > maxx)
				goto clipxmax;
		      xclipok:if (dy < 0)
				goto clipymin;
			if (dy > maxy)
				goto clipymax;
		      yclipok:*destptr++ =
			    *(offstable[dy] +
			      dx);

		}
	return;

      clipxmin:
	dx = 0;
	goto xclipok;
      clipxmax:
	dx = maxx;
	goto xclipok;
      clipymin:
	dy = 0;
	goto yclipok;
      clipymax:
	dy = maxy;
	goto yclipok;
}

void doWarp16bpp(struct warp *w, int32_t xw, int32_t yw, int32_t cw)
{
	int32_t c, i, x, y, dx, dy, maxx, maxy;
	int32_t width, height, *ctable, *ctptr, *distptr;
	int32_t *sintable, *disttable;
	uint16_t *destptr, **offstable;

	ctptr = ctable = &(w->ctable[0]);
	sintable = &(w->sintable[0]);
	offstable = (uint16_t **) w->offstable;
	distptr = disttable = w->disttable;
	width = w->width;
	height = w->height;
	destptr = (uint16_t *) w->framebuf;

	c = 0;

	for (x = 0; x < 512; x++) {
		i = (c >> 3) & 0x3FE;
		*ctptr++ = ((sintable[i] * yw) >> 15);
		*ctptr++ = ((sintable[i + 256] * xw) >> 15);
		c += cw;
	}

	maxx = width - 1;
	maxy = height - 1;

	for (y = 0; y < height; y++)
		for (x = 0; x < width; x++) {
			i = *distptr++;
			dx = ctable[i + 1] + x;
			dy = ctable[i] + y;

			if (dx < 0)
				goto clipxmin;
			if (dx > maxx)
				goto clipxmax;
		      xclipok:if (dy < 0)
				goto clipymin;
			if (dy > maxy)
				goto clipymax;
		      yclipok:*destptr++ =
			    *(offstable[dy] +
			      dx);

		}
	return;

      clipxmin:
	dx = 0;
	goto xclipok;
      clipxmax:
	dx = maxx;
	goto xclipok;
      clipymin:
	dy = 0;
	goto yclipok;
      clipymax:
	dy = maxy;
	goto yclipok;
}

void doWarp24bpp(struct warp *w, int32_t xw, int32_t yw, int32_t cw)
{
	int32_t c, i, x, y, dx, dy, maxx, maxy;
	int32_t width, height, *ctable, *ctptr, *distptr;
	int32_t *sintable, *disttable;
	uint8_t *destptr, **offstable, *pptr;

	ctptr = ctable = &(w->ctable[0]);
	sintable = &(w->sintable[0]);
	offstable = (uint8_t **) w->offstable;

	distptr = disttable = w->disttable;
	width = w->width;
	height = w->height;
	destptr = (uint8_t *) w->framebuf;

	c = 0;

	for (x = 0; x < 512; x++) {
		i = (c >> 3) & 0x3FE;
		*ctptr++ = ((sintable[i] * yw) >> 15);
		*ctptr++ = ((sintable[i + 256] * xw) >> 15);
		c += cw;
	}

	maxx = width - 1;
	maxy = height - 1;

	for (y = 0; y < height; y++)
		for (x = 0; x < width; x++) {
			i = *distptr++;
			dx = ctable[i + 1] + x;
			dy = ctable[i] + y;

			if (dx < 0)
				goto clipxmin;
			if (dx > maxx)
				goto clipxmax;
		      xclipok:if (dy < 0)
				goto clipymin;
			if (dy > maxy)
				goto clipymax;
		      yclipok:pptr =
			    offstable[dy] + dx + dx +
			    dx;

			*destptr++ = *pptr++;
			*destptr++ = *pptr++;
			*destptr++ = *pptr++;
		}
	return;

      clipxmin:
	dx = 0;
	goto xclipok;
      clipxmax:
	dx = maxx;
	goto xclipok;
      clipymin:
	dy = 0;
	goto yclipok;
      clipymax:
	dy = maxy;
	goto yclipok;
}

void doWarp32bpp(struct warp *w, int32_t xw, int32_t yw, int32_t cw)
{
	int32_t c, i, x, y, dx, dy, maxx, maxy;
	int32_t width, height, *ctable, *ctptr, *distptr;
	int32_t *sintable, *disttable;
	uint32_t *destptr, **offstable;

	ctptr = ctable = &(w->ctable[0]);
	sintable = &(w->sintable[0]);
	offstable = (uint32_t **) w->offstable;
	distptr = disttable = w->disttable;
	width = w->width;

	height = w->height;
	destptr = (uint32_t *) w->framebuf;

	c = 0;

	for (x = 0; x < 512; x++) {
		i = (c >> 3) & 0x3FE;
		*ctptr++ = ((sintable[i] * yw) >> 15);
		*ctptr++ = ((sintable[i + 256] * xw) >> 15);
		c += cw;
	}

	maxx = width - 1;
	maxy = height - 1;

	for (y = 0; y < height; y++)
		for (x = 0; x < width; x++) {
			i = *distptr++;
			dx = ctable[i + 1] + x;
			dy = ctable[i] + y;

			if (dx < 0)
				goto clipxmin;
			if (dx > maxx)
				goto clipxmax;
		      xclipok:if (dy < 0)
				goto clipymin;
			if (dy > maxy)
				goto clipymax;
		      yclipok:*destptr++ =
			    *(offstable[dy] +
			      dx);

		}
	return;

      clipxmin:
	dx = 0;
	goto xclipok;
      clipxmax:
	dx = maxx;
	goto xclipok;
      clipymin:
	dy = 0;
	goto yclipok;
      clipymax:
	dy = maxy;
	goto yclipok;
}
