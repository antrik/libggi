/* $Id: gamma.c,v 1.11 2007/02/25 17:21:06 cegger Exp $
******************************************************************************

  Generic gamma correction library

  Copyright (C) 1997 Jason McMullan [jmcc@ggi-project.org]
  Copyright (C) 1998 Andrew Apted   [andrew@ggi-project.org]

   Permission is hereby granted, free of charge, to any person obtaining a
   copy of this software and associated documentation files (the "Software"),
   to deal in the Software without restriction, including without limitation
   the rights to use, copy, modify, merge, publish, distribute, sublicense,
   and/or sell copies of the Software, and to permit persons to whom the
   Software is furnished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be included in
   all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
   THE AUTHOR(S) BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
   IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
   CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

******************************************************************************
*/

#include <math.h>
#include <string.h>

#ifdef _AIX
#include <sys/types.h>
#include <unistd.h>
#endif

#include "config.h"
#include "color.h"
#include <ggi/internal/ggi_debug.h>

int GGI_color_getgamma(struct ggi_visual *vis, ggi_float *r, ggi_float *g, ggi_float *b)
{
	if (!vis->gamma) {
		*r = *g = *b = 1.0;	/* principal of least surprise */
		return GGI_ENOFUNC;
	}
	*r = vis->gamma->gamma_r;
	*g = vis->gamma->gamma_g;
	*b = vis->gamma->gamma_b;

	return GGI_OK;
}

int GGI_color_setgamma(struct ggi_visual *vis, ggi_float r, ggi_float g, ggi_float b)
{
	int err, i;
	ggi_float ir, ig, ib;
	ggi_float intensity_r, intensity_g, intensity_b;
	ggi_float delta_r, delta_g, delta_b;
	ggi_color map[256]; /* This gets most visuals in one function call */

	if (!vis->gamma) return GGI_ENOFUNC;

	if ((r <= 0.0) || (g <= 0.0) || (b <= 0.0))
		return GGI_EARGINVAL;

	if ((vis->gamma->maxwrite_r < 0) ||
	    (vis->gamma->maxwrite_g < 0) ||
	    (vis->gamma->maxwrite_b < 0)) {
		DPRINT("vis %p missing ggiSetGamma implementation.\n");
		return GGI_ENOFUNC;
	}

	/* inverse the gamma values */
	ir = 1.0 / r;
	ig = 1.0 / g;
	ib = 1.0 / b;

	delta_r = 1.0 / vis->gamma->maxwrite_r;
	delta_g = 1.0 / vis->gamma->maxwrite_g;
	delta_b = 1.0 / vis->gamma->maxwrite_b;

	intensity_r = intensity_g = intensity_b = 0.0;

	/* calculate gammamap */
	memset(map, 0, 256 * sizeof(ggi_color));
	i = 0;
	while (1) { /* Do as many 256-color blocks as needed. */
		int j, maxj;
		maxj = 0;
		for (j=0; 
		     (j < 256) && ((i + j) < vis->gamma->maxwrite_r);
		     j++) {
		  map[j].r = (uint16_t) floor(pow(intensity_r, ir) * 65536.0);
		  intensity_r += delta_r;
		}
		if (maxj < j) maxj = j;
		for (j=0; 
		     (j < 256) && ((i + j) < vis->gamma->maxwrite_g);
		     j++) {
		  map[j].g = (uint16_t) floor(pow(intensity_g, ig) * 65536.0);
		  intensity_g += delta_g;
		}
		if (maxj < j) maxj = j;
		for (j=0; 
		     (j < 256) && ((i + j) < vis->gamma->maxwrite_b);
		     j++) {
		  map[j].b = (uint16_t) floor(pow(intensity_b, ib) * 65536.0);
		  intensity_b += delta_b;
		}
		if (maxj < j) maxj = j;
		if ((err = ggiSetGammaMap(vis->stem, i, maxj, map)) != 0) {
		  return err;
		}
		i += maxj;
		if (maxj < 256) break;
	}

	/* SetGammaMap was successful, so update values */
	vis->gamma->gamma_r = r;
	vis->gamma->gamma_g = g;
	vis->gamma->gamma_b = b;

	return 0;
} 
