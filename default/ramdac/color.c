/* $Id: color.c,v 1.7 2004/11/27 20:03:30 soyt Exp $
******************************************************************************

   Graphics library for GGI.  Generic RAMDAC via IOCTL driver

   Copyright (C) 1997 Jason McMullan   [jmcc@ggi-project.org]
   Copyright (C) 1998 Andrew Apted     [andrew@ggi-project.org]

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
#include <sys/ioctl.h>

#ifdef _AIX
#include <sys/types.h>
#include <unistd.h>
#endif

#include "config.h"
#include <ggi/internal/ggi-dl.h>
#include <kgi/kgi_commands.h>


int GGI_ramdac_setpalvec(ggi_visual *vis, int start, int len, const ggi_color *colormap)
{
	int nocols = 256;

	if (start == GGI_PALETTE_DONTCARE) {
		start = 0;
	}

	if (GT_SCHEME(LIBGGI_GT(vis)) != GT_TRUECOLOR) {
		nocols = 1 << GT_DEPTH(LIBGGI_GT(vis));
	}
	
	if ((start < 0) || (start+len > nocols)) {
		return GGI_ENOSPACE;
	}

	memcpy(vis->palette+start, colormap, len*sizeof(ggi_color));

	return _ggiSendKGICommand(vis, (int)RAMDAC_SETCLUT, vis->palette);
}

int GGI_ramdac_getpalvec(ggi_visual *vis,int start,int len,ggi_color *colormap)
{
	if (start < 0 || start+len > 256)
		return GGI_ENOSPACE;

	memcpy(colormap, vis->palette+start, len*sizeof(ggi_color));

	return 0;
}

/* !!! FIXME: add GGIsetgammmap and GGIgetgammamap here
 * (calling RAMDAC_SETCLUT and RAMDAC_GETCLUT respectively)
 */
