/* $Id: gamma.c,v 1.1 2001/05/12 23:01:32 cegger Exp $
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

#ifdef _AIX
#include <sys/types.h>
#include <unistd.h>
#endif

#include <ggi/internal/ggi-dl.h>


#if 0  /* REDUNDANT */
int GGI_color_setgammamap(ggi_visual *vis, int start, int len, ggi_color *colormap)
{
	if (GT_SCHEME(LIBGGI_GT(vis)) != GT_TRUECOLOR)
		return -2;

	return ggiSetPalette(vis, start, len, colormap);
}

int GGI_color_getgammamap(ggi_visual *vis, int start, int len, ggi_color *colormap)
{
	if (GT_SCHEME(LIBGGI_GT(vis)) != GT_TRUECOLOR)
		return -2;

	return ggiGetPalette(vis, start, len, colormap);
}
#endif


int GGI_color_getgamma(ggi_visual *vis, ggi_float *r, ggi_float *g, ggi_float *b)
{
	*r = vis->gamma_red;
	*g = vis->gamma_green;
	*b = vis->gamma_blue;

	return 0;
}

int GGI_color_setgamma(ggi_visual *vis, ggi_float r, ggi_float g, ggi_float b)
{
	int err, i;
	
	ggi_float intensity, delta;
	ggi_float ir, ig, ib;

	ggi_color map[256];


	if (GT_SCHEME(LIBGGI_GT(vis)) != GT_TRUECOLOR)
		return -2;

	if ((r <= 0.0) || (g <= 0.0) || (b <= 0.0))
		return -1;

	/* inverse the gamma values */

	ir = 1.0 / r;
	ig = 1.0 / g;
	ib = 1.0 / b;

	intensity = 0.0;
	delta = 1.0 / 256;

	/* calculate gammamap */

	for (i=0; i < 256; i++, intensity += delta) {
		map[i].r = (uint16) floor(pow(intensity, ir) * 65536.0);
		map[i].g = (uint16) floor(pow(intensity, ig) * 65536.0);
		map[i].b = (uint16) floor(pow(intensity, ib) * 65536.0);
	}
	
	if ((err = ggiSetGammaMap(vis, 0, 256, map)) != 0) {
		return err;
	}

	/* SetGammaMap was successful, so update values */

	vis->gamma_red   = r;
	vis->gamma_green = g;
	vis->gamma_blue  = b;

	return 0;
} 
