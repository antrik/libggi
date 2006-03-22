/* $Id: pan.c,v 1.8 2006/03/22 20:22:27 cegger Exp $
******************************************************************************

   Display-FBDEV

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

#include <linux/fb.h>

#include "config.h"
#include <ggi/display/fbdev.h>
#include <ggi/internal/ggi_debug.h>


int GGI_fbdev_setorigin(struct ggi_visual *vis, int x, int y)
{
	ggi_fbdev_priv *priv = FBDEV_PRIV(vis);
	int max_x = LIBGGI_VIRTX(vis) - LIBGGI_X(vis);
	int max_y = LIBGGI_VIRTY(vis) - LIBGGI_Y(vis);
	int err = 0;

	if ((priv->fix.xpanstep == 0) && (priv->fix.ypanstep == 0)) {
		DPRINT("display-fbdev: panning not supported.\n");
		return -1;
	}
	
	if ((x < 0) || (y < 0) || (x > max_x) || (y > max_y)) {
		DPRINT("display-fbdev: panning out of range:"
			"(%d,%d) > (%d,%d)\n", x, y, max_x, max_y);
		return GGI_EARGINVAL;
	}

	if (priv->fix.xpanstep == 0) {
		x = 0;
	}
	if (priv->fix.ypanstep == 0) {
		y = 0;
	}

	priv->var.xoffset = x;
	priv->var.yoffset = y + vis->d_frame_num * LIBGGI_VIRTY(vis);

	err = fbdev_doioctl(vis, FBIOPAN_DISPLAY, &priv->var); 
	
	if (err) {
		DPRINT("display-fbdev: PAN_DISPLAY failed.\n");
		return err;
	}

	vis->origin_x = x;
	vis->origin_y = y;
	
	return 0;
}

int GGI_fbdev_setdisplayframe(struct ggi_visual *vis, int num)
{
        ggi_directbuffer *db = _ggi_db_find_frame(vis, num);

        if (db == NULL) {
                return GGI_ENOMATCH;
        }
        vis->d_frame_num = num;

	return GGI_fbdev_setorigin(vis, vis->origin_x, vis->origin_y);
}
