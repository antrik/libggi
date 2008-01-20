/* $Id: draw.c,v 1.15 2008/01/20 19:26:43 pekberg Exp $
******************************************************************************

   FreeBSD vgl(3) target: vgl drawing

   Copyright (C) 2000 Alcove - Nicolas Souchu <nsouch@freebsd.org>

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

#include "config.h"
#include <ggi/display/vgl.h>
#include <ggi/internal/ggi_debug.h>

int
GGI_vgl_putbox(struct ggi_visual *vis, int x, int y, int w, int h, const void *buffer)
{ 
	int pixelsize = (LIBGGI_PIXFMT(vis)->size+7)/8;
	int rowadd = w * pixelsize;
	const uint8_t *buf = buffer;

	LIBGGICLIP_PUTBOX(vis, x, y, w, h, buf, rowadd, * pixelsize);

	while (h--) {
		ggiPutHLine(vis->instance.stem, x, y, w, buf);
		y++;
		buf += rowadd;
	}
	
	return 0;
}

int GGI_vgl_drawbox(struct ggi_visual *vis, int x, int y, int w, int h)
{
	LIBGGICLIP_XYWH(vis, x, y, w, h);
  
	VGLFilledBox(VGLDisplay, x, y, x+w, y+h, (long)LIBGGI_GC_FGCOLOR(vis));

	return 0;
}

int GGI_vgl_drawline(struct ggi_visual *vis,int x,int y,int xe,int ye)
{
	VGLLine(VGLDisplay, x, y, xe, ye, (long)LIBGGI_GC_FGCOLOR(vis));

	return 0;
}

int GGI_vgl_puthline(struct ggi_visual *vis,int x,int y,int w,const void *buffer)
{
	int pixelsize = (LIBGGI_PIXFMT(vis)->size+7)/8;
	const byte *buf = buffer;
	int i;

	for (i = 0; i < w*pixelsize; i++)
		VGLSetXY(VGLDisplay, x+i, y, *buf++);
		
	return 0;
}

int GGI_vgl_drawhline_nc(struct ggi_visual *vis, int x, int y, int w)
{
	VGLLine(VGLDisplay, x, y, x+w-1, y, (long)LIBGGI_GC_FGCOLOR(vis));

	return 0;
}

int GGI_vgl_drawhline(struct ggi_visual *vis, int x, int y, int w)
{
	LIBGGICLIP_XYW(vis, x, y, w);

	VGLLine(VGLDisplay, x, y, x+w-1, y, (long)LIBGGI_GC_FGCOLOR(vis));

	return 0;
}

int GGI_vgl_drawvline(struct ggi_visual *vis, int x, int y, int height)
{
	/* Clipping */
	if (x< (LIBGGI_GC(vis)->cliptl.x) ||
	    x>=(LIBGGI_GC(vis)->clipbr.x)) return 0;
	if (y< (LIBGGI_GC(vis)->cliptl.y)) {
		int diff=(LIBGGI_GC(vis)->cliptl.y)-y;
		y     +=diff;
		height-=diff;
	}
	if (y+height>(LIBGGI_GC(vis)->clipbr.y)) {
		height=(LIBGGI_GC(vis)->clipbr.y)-y;
	}
	if (height<1)
		return 0;
	
	VGLLine(VGLDisplay, x, y, x, y+height-1, (long)LIBGGI_GC_FGCOLOR(vis));
	
	return 0;
}

int GGI_vgl_drawvline_nc(struct ggi_visual *vis, int x, int y, int h)
{
	VGLLine(VGLDisplay, x, y, x, y+h-1, (long)LIBGGI_GC_FGCOLOR(vis));
	
	return 0;
}

int
GGI_vgl_setpalvec(struct ggi_visual *vis, int start, int len, const ggi_color *colormap)
{
	vgl_priv *priv = VGL_PRIV(vis);
	unsigned int maxlen = 1 << GT_DEPTH(LIBGGI_GT(vis));
	int i;

	if (start == GGI_PALETTE_DONTCARE) start = 0;

	if (maxlen > 256) {
		DPRINT("display-vgl: incorrect palette maxlen (%d)\n", maxlen);
		return GGI_ENOSPACE;
	}

	if (start < 0 || start+len > maxlen) {
		DPRINT("display-vgl: incorrect palette len (%d)\n", maxlen);
		return GGI_ENOSPACE;
	}

	memcpy(LIBGGI_PAL(vis)->clut.data+start, colormap, len*sizeof(ggi_color));

	/* VGLSetPalette takes 8-bit r,g,b,
	   so we need to scale ggi_color's 16-bit values. */
	for (i = start; i < len; i++) {
		priv->vgl_palred[i] = colormap[i].r >> 10;
		priv->vgl_palgreen[i] = colormap[i].g >> 10;
		priv->vgl_palblue[i] = colormap[i].b >> 10;
	}

	VGLSetPalette(priv->vgl_palred, priv->vgl_palgreen, priv->vgl_palblue);

	DPRINT("display-vgl: Palette set, ok (0x%x, 0x%x, 0x%x)\n",
			start, len, maxlen);

	return 0;
}

int GGI_vgl_getpixel_nc(struct ggi_visual *vis, int x, int y, ggi_pixel *col)
{

	*col = (ggi_pixel)VGLGetXY(VGLDisplay, x, y);

	return 0;
}

int GGI_vgl_putpixel(struct ggi_visual *vis, int x, int y, ggi_pixel col)
{
	CHECKXY(vis, x, y);

	VGLSetXY(VGLDisplay, x, y, (long)col);

	return 0;
}

int GGI_vgl_putpixel_nc(struct ggi_visual *vis, int x, int y, ggi_pixel col)
{
	VGLSetXY(VGLDisplay, x, y, (long)col);

	return 0;
}

int GGI_vgl_drawpixel(struct ggi_visual *vis, int x, int y)
{
	CHECKXY(vis, x, y);

	VGLSetXY(VGLDisplay, x, y, (long)LIBGGI_GC_FGCOLOR(vis));

	return 0;
}

int GGI_vgl_drawpixel_nc(struct ggi_visual *vis, int x, int y)
{
	VGLSetXY(VGLDisplay, x, y, (long)LIBGGI_GC_FGCOLOR(vis));

	return 0;
}
