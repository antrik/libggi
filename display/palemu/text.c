/* $Id: text.c,v 1.7 2007/03/08 20:54:08 soyt Exp $
******************************************************************************

   Display-palemu: text

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

#include "config.h"
#include <ggi/internal/ggi-dl.h>

#include <ggi/display/palemu.h>

#include <stdio.h>
#include <stdlib.h>

#undef putc


int GGI_palemu_putc(struct ggi_visual *vis, int x, int y, char c)
{
	ggi_palemu_priv *priv = PALEMU_PRIV(vis);
	int char_w;
	int char_h;

	ggiGetCharSize(vis->module.stem, &char_w, &char_h);

	UPDATE_MOD(vis, x, y, char_w, char_h);

	return priv->mem_opdraw->putc(vis, x, y, c);
}

#if 0
int GGI_palemu_getcharsize(struct ggi_visual *vis, int *width, int *height)
{
	ggi_palemu_priv *priv = PALEMU_PRIV(vis);

	return ggiGetCharSize(priv->parent, width, height);
}
#endif
