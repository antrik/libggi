/* $Id: gtext.c,v 1.9 2005/03/28 20:33:34 pekberg Exp $
******************************************************************************

   Graphics library for GGI. Textfunctions for X.

   Copyright (C) 1998 Marcus Sundberg [marcus@ggi-project.org]

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

#include <string.h>

#include "config.h"
#include <ggi/internal/ggi-dl.h>
#include <ggi/display/x.h>

int GGI_X_getcharsize_font(ggi_visual *vis, int *width, int *height)
{
	ggi_x_priv *priv;
	priv = GGIX_PRIV(vis);

	*width = priv->textfont->max_bounds.width;
	*height = priv->textfont->max_bounds.ascent
		+ priv->textfont->max_bounds.descent;

	return 0;
}

int GGI_X_putc_slave_draw(ggi_visual *vis, int x, int y, char c)
{
	ggi_x_priv *priv;
	uint8  *datafg, *databg, *ptr;
	ggi_color *colors, gccolor;
	int i, w, h;

	priv = GGIX_PRIV(vis);

	w = priv->textfont->max_bounds.width;
	h = priv->textfont->max_bounds.ascent
	  + priv->textfont->max_bounds.descent;

#warning 1,2,4-bpp support needed.

	/* Font to backbuffer.  Fun fun fun fun fun! */
	colors = malloc(sizeof(ggi_color) * w * h);
	if (!colors) return GGI_ENOMEM;
	datafg = malloc((size_t)(w * h * priv->fontimg->bits_per_pixel)/8);
	if (!datafg) { free(colors); return GGI_ENOMEM; }
	databg = malloc((size_t)(w * h * priv->fontimg->bits_per_pixel)/8);
	if (!databg) { free(datafg); free(colors); return GGI_ENOMEM; }

	/* This looks overcomplicated, but is more ready to be 
	 * adapted to 1,2,4-bit.  In fact will already work for 
	 * certain width fonts.
	 */

	priv->slave->opcolor->unmappixel(priv->slave, 
					 LIBGGI_GC_FGCOLOR(priv->slave),
					 &gccolor);
	for (i = 0 ; i < w * h ; i++)
       		memcpy(colors + i, &gccolor, sizeof(ggi_color));
	priv->slave->opcolor->packcolors(priv->slave, datafg, colors, w * h);
	priv->slave->opcolor->unmappixel(priv->slave, 
					 LIBGGI_GC_BGCOLOR(priv->slave),
					 &gccolor);
	for (i = 0 ; i < w * h ; i++)
       		memcpy(colors + i, &gccolor, sizeof(ggi_color));
	priv->slave->opcolor->packcolors(priv->slave, databg, colors, w * h);
	free(colors);
	ptr = (uint8 *)(priv->fontimg->data) + 
	  (w * (unsigned int)(unsigned char)c + priv->fontimg->xoffset) * 
	  priv->fontimg->bits_per_pixel / 8;
	for (i = 0; i < h; i++) {
		int j, w2;
		w2 = w * priv->fontimg->bits_per_pixel / 8;
		for (j = 0; j < w2; j++) {
			*(datafg+j + i*w2) &= *(ptr + j);
			*(databg+j + i*w2) &= ~*(ptr + j);
			*(datafg+j + i*w2) |= *(databg+j + i*w2);
		}
		ptr += priv->fontimg->bytes_per_line;
	}

	GGI_X_CLEAN(vis, x, y, w, h);
	priv->slave->opdraw->putbox(priv->slave, x, y, w, h, datafg);
	free(datafg);
	free(databg);


	y = GGI_X_WRITE_Y;

	GGI_X_LOCK_XLIB(vis);

	XSetForeground(priv->disp, priv->gc, LIBGGI_GC_BGCOLOR(vis));
	XFillRectangle(priv->disp, priv->drawable, priv->gc, x, y,
		       (unsigned)w, (unsigned)h);
	XSetForeground(priv->disp, priv->gc, LIBGGI_GC_FGCOLOR(vis));
	XDrawString(priv->disp, priv->drawable, priv->gc,
		    x, y+priv->textfont->max_bounds.ascent, &c, 1);

	GGI_X_MAYBE_SYNC(vis);

	GGI_X_UNLOCK_XLIB(vis);

	return 0;
}

int GGI_X_putc_draw(ggi_visual *vis, int x, int y, char c)
{
	ggi_x_priv *priv;
	priv = GGIX_PRIV(vis);


	y = GGI_X_WRITE_Y;

	GGI_X_LOCK_XLIB(vis);

	XSetForeground(priv->disp, priv->gc, LIBGGI_GC_BGCOLOR(vis));
	XFillRectangle(priv->disp, priv->drawable, priv->gc,
                       x, y, (unsigned)priv->textfont->max_bounds.width, 
		       (unsigned)priv->textfont->max_bounds.ascent
		       + priv->textfont->max_bounds.descent);
	XSetForeground(priv->disp, priv->gc, LIBGGI_GC_FGCOLOR(vis));

	XDrawString(priv->disp, priv->drawable, priv->gc,
		    x, y+priv->textfont->max_bounds.ascent, &c, 1);
	
	GGI_X_MAYBE_SYNC(vis);

	GGI_X_UNLOCK_XLIB(vis);

	return 0;
}


