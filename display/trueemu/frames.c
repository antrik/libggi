/* $Id: frames.c,v 1.1 2001/05/12 23:02:37 cegger Exp $
******************************************************************************

   Display-trueemu: frame handling

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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <ggi/display/trueemu.h>


int GGI_trueemu_setreadframe(ggi_visual *vis, int num)
{
	ggi_trueemu_priv *priv = TRUEEMU_PRIV(vis);
	
	return priv->mem_opdraw->setreadframe(vis, num);
}

int GGI_trueemu_setwriteframe(ggi_visual *vis, int num)
{
	ggi_trueemu_priv *priv = TRUEEMU_PRIV(vis);

	/* !!! probably some stuff needed WRT the dirty region */

	return priv->mem_opdraw->setwriteframe(vis, num);
}

int GGI_trueemu_setdisplayframe(ggi_visual *vis, int num)
{
	/* ggi_trueemu_priv *priv = TRUEEMU_PRIV(vis); */

        ggi_directbuffer *db = _ggi_db_find_frame(vis, num);

        if (db == NULL) {
                return -1;
        }

        vis->d_frame_num = num;

	_ggi_trueemu_Transfer(vis, 0, 0, LIBGGI_VIRTX(vis), LIBGGI_VIRTY(vis));

	/* !!! probably some stuff needed WRT the dirty region */

	return 0;
}
