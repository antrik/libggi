/* $Id: draw.c,v 1.2 2002/09/08 21:37:47 soyt Exp $
******************************************************************************

   Display-VCSA: drawing primitives

   Copyright (C) 1998 Andrew Apted    [andrew@ggi-project.org]

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

#include <unistd.h>
#include "config.h"
#include <ggi/internal/ggi-dl.h>

#include <ggi/display/vcsa.h>


#define VCSA_SEEK(vis, priv, x, y)  \
	lseek(LIBGGI_FD(vis), 4 + (y)*(priv)->width*2 + (x)*2, SEEK_SET)


int GGI_vcsa_putpixel_nc(ggi_visual *vis, int x, int y, ggi_pixel p)
{
	ggi_vcsa_priv *priv = VCSA_PRIV(vis);

	unsigned short buf[4];
	
	if (VCSA_SEEK(vis, priv, x, y) < 0) {
		GGIDPRINT_DRAW("display-vcsa: seek failed.\n");
		return -1;
	}

	buf[0] = (unsigned short) p;

	if (write(LIBGGI_FD(vis), buf, 2) != 2) {
		GGIDPRINT_DRAW("display-vcsa: write failed.\n");
		return -1;
	}

	return 0;
}

int GGI_vcsa_drawhline_nc(ggi_visual *vis, int x, int y, int w)
{
	ggi_vcsa_priv *priv = VCSA_PRIV(vis);

	unsigned short buf[256];

	int i;
	

	if (w <= 0) 
		return 0;

	if (w > 256)	    /* yeah, right */
		w = 256;

	if (VCSA_SEEK(vis, priv, x, y) < 0) {
		GGIDPRINT_DRAW("display-vcsa: seek failed.\n");
		return -1;
	}

	for (i=0; i < w; i++) {
		buf[i] = LIBGGI_GC_FGCOLOR(vis);
	}
	
	w *= 2;

	if (write(LIBGGI_FD(vis), buf, w) != w) {
		GGIDPRINT_DRAW("display-vcsa: write failed.\n");
		return -1;
	}

	return 0;
}


/* ---------------------------------------------------------------------- */


int GGI_vcsa_puthline(ggi_visual *vis, int x, int y, int w, void *buf)
{
	ggi_vcsa_priv *priv = VCSA_PRIV(vis);

	int diff;

	/* clip */

	if ((y <  LIBGGI_GC(vis)->cliptl.y) || 
	    (y >= LIBGGI_GC(vis)->clipbr.y))
		return 0;

	if (x < LIBGGI_GC(vis)->cliptl.x) {
		diff = LIBGGI_GC(vis)->cliptl.x - x;
		x += diff;
		w -= diff;
		buf = (uint16 *) buf + diff;
	}

	if (x+w > LIBGGI_GC(vis)->clipbr.x) {
		w = LIBGGI_GC(vis)->clipbr.x - x;
	}
	
	if (w <= 0) 
		return 0;


	if (VCSA_SEEK(vis, priv, x, y) < 0) {
		GGIDPRINT_DRAW("display-vcsa: seek failed.\n");
		return -1;
	}

	w *= 2;

	if (write(LIBGGI_FD(vis), buf, w) != w) {
		GGIDPRINT_DRAW("display-vcsa: write failed.\n");
		return -1;
	}

	return 0;
}

int GGI_vcsa_gethline(ggi_visual *vis, int x, int y, int w, void *buf)
{
	ggi_vcsa_priv *priv = VCSA_PRIV(vis);

	int diff;

	/* clip */

	if ((y <  LIBGGI_GC(vis)->cliptl.y) || 
	    (y >= LIBGGI_GC(vis)->clipbr.y))
		return 0;

	if (x < LIBGGI_GC(vis)->cliptl.x) {
		diff = LIBGGI_GC(vis)->cliptl.x - x;
		x += diff;
		w -= diff;
		buf = (uint16 *) buf + diff;
	}

	if (x+w > LIBGGI_GC(vis)->clipbr.x) {
		w = LIBGGI_GC(vis)->clipbr.x - x;
	}
	
	if (w <= 0) 
		return 0;


	if (VCSA_SEEK(vis, priv, x, y) < 0) {
		GGIDPRINT_DRAW("display-vcsa: seek failed.\n");
		return -1;
	}

	w *= 2;

	if (read(LIBGGI_FD(vis), buf, w) != w) {
		GGIDPRINT_DRAW("display-vcsa: read failed.\n");
		return -1;
	}

	return 0;
}


/* ---------------------------------------------------------------------- */


int GGI_vcsa_getpixel(ggi_visual *vis, int x, int y, ggi_pixel *p)
{
	ggi_vcsa_priv *priv = VCSA_PRIV(vis);

	unsigned short buf[4];
	
	if (VCSA_SEEK(vis, priv, x, y) < 0) {
		GGIDPRINT_DRAW("display-vcsa: seek failed.\n");
		return -1;
	}

	if (read(LIBGGI_FD(vis), buf, 2) != 2) {
		GGIDPRINT_DRAW("display-vcsa: read failed.\n");
		return -1;
	}

	*p = buf[0];

	return 0;
}


/* ---------------------------------------------------------------------- */


int GGI_vcsa_getcharsize(ggi_visual *vis, int *width, int *height)
{
	*width = *height = 1;
	
	return 0;
}

int GGI_vcsa_putc(ggi_visual *vis, int x, int y, char c)
{
	return ggiPutPixel(vis, x, y, (c & 0xff) |
		((LIBGGI_GC_BGCOLOR(vis) & 0x0f00) << 4) |
		 (LIBGGI_GC_FGCOLOR(vis) & 0x0f00));
}

int GGI_vcsa_puts(ggi_visual *vis, int x, int y, const char *str)
{
	unsigned short buf[256];

	int len;

	for (len=0; *str && len < 256; str++, len++) {
		buf[len] = (*str & 0xff) |
		((LIBGGI_GC_BGCOLOR(vis) & 0x0f00) << 4) |
		 (LIBGGI_GC_FGCOLOR(vis) & 0x0f00);
	}

	return ggiPutHLine(vis, x, y, len, buf);
}
