/* $Id: color.c,v 1.3 2002/10/11 01:30:26 redmondp Exp $
******************************************************************************

   Display-FBDEV

   Copyright (C) 1998 Andrew Apted	[andrew@ggi-project.org]
   Copyright (C) 1999 Marcus Sundberg	[marcus@ggi-project.org]

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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>

#include "config.h"
#include <ggi/internal/ggi-dl.h>
#include <ggi/display/kgi.h>


int
GGI_kgi_setpalvec(ggi_visual *vis, int start, int len, ggi_color *colormap)
{
	kgic_ilut_set_request_t ilut;
	int nocols = 1 << GT_DEPTH(LIBGGI_GT(vis));

	GGIDPRINT_COLOR("display-kgi: SetPalVec(%d,%d)\n", start, len);
	
	if (start == GGI_PALETTE_DONTCARE) {
		start = 0;
	}

	if ((start < 0) || (len < 0) || (start+len > nocols)) {
		return -1;
	}

	memcpy(vis->palette+start, colormap, len*sizeof(ggi_color));

	ilut.image = 0;
	ilut.resource = 0;
	ilut.lut = 0;
	ilut.idx = start;
	ilut.cnt = len;
	ilut.am = KGI_AM_COLORS;
	ilut.data = malloc(len*3*sizeof(ilut.data[0]));

	for (start = 0; len > 0; start++, colormap++, len--) {
		ilut.data[start*3]     = colormap->r;
		ilut.data[start*3 + 1] = colormap->g;
		ilut.data[start*3 + 2] = colormap->b;
	}

	if (ioctl(KGI_CTX(vis).mapper.fd, KGIC_RESOURCE_CLUT_SET, &ilut) < 0) {
		GGIDPRINT_COLOR("display-kgi: PUTCMAP failed.");
		return -1;
	}

	free(ilut.data);

	return 0;
}
