/* $Id: box.c,v 1.4 2006/03/12 23:15:05 soyt Exp $
******************************************************************************

   LibGGI - DirectFB driver acceleration for the fbdev target

   Copyright (C) 2001 Brian S. Julin	[bri@calyx.com]

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

#include "ggidirectfb.h"


int GGI_directfb_drawbox(struct ggi_visual *vis, int x, int y, int w, int h)
{
	if (w > 0 && h > 0) {	/* 0 width is not OK! */
		struct directfb_priv *priv = DIRECTFB_PRIV(vis);
		int yadd = vis->w_frame_num * LIBGGI_VIRTY(vis);
		DFBRectangle dfbobj;

		y += yadd;

		directfb_gcupdate(vis, priv, LIBGGI_MODE(vis), LIBGGI_GC(vis),
				  LIBGGI_VIRTX(vis), yadd, DFXL_FILLRECTANGLE);

		dfbobj.x = x;
		dfbobj.y = y;
		dfbobj.h = h;
		dfbobj.w = w;
		priv->device.funcs.FillRectangle(priv->device.driver_data,
						 priv->device.device_data,
						 &dfbobj);

		vis->accelactive = 1;
	}

	return 0;
}


int GGI_directfb_fillscreen(struct ggi_visual *vis)
{
	struct directfb_priv *priv = DIRECTFB_PRIV(vis);
	int virtx = LIBGGI_VIRTX(vis);
	int virty = LIBGGI_VIRTY(vis);
	int yadd = vis->w_frame_num * LIBGGI_VIRTY(vis);
	DFBRectangle dfbobj;

        directfb_gcupdate(vis, priv, LIBGGI_MODE(vis), LIBGGI_GC(vis),
                          LIBGGI_VIRTX(vis), yadd, DFXL_FILLRECTANGLE);

	dfbobj.x = 0;
	dfbobj.y = 0;
	dfbobj.h = virty;
	dfbobj.w = virtx;

	priv->device.funcs.FillRectangle(priv->device.driver_data,
					 priv->device.device_data,
					 &dfbobj);

	vis->accelactive = 1;

	return 0;
}


