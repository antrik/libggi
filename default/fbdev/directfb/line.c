/* $Id: line.c,v 1.2 2002/03/23 05:50:24 skids Exp $
******************************************************************************

   LibGGI - DirectFB driver Acceleration for the fbdev target

   Copyright (C) 2001 Brian S. Julin  [bri@calyx.com]

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


int GGI_directfb_drawline(ggi_visual *vis, int x, int y, int x2, int y2)
{
	struct directfb_priv *priv = DIRECTFB_PRIV(vis);
	int yadd = vis->w_frame_num * LIBGGI_VIRTY(vis);
	DFBRegion dfbobj;

	if (yadd) {
		y += yadd;
		y2 += yadd;
	}

	directfb_gcupdate(vis, priv, LIBGGI_MODE(vis), LIBGGI_GC(vis), 
			  LIBGGI_VIRTX(vis), yadd, DFXL_DRAWLINE);

	dfbobj.x1 = x;
	dfbobj.y1 = y;
	dfbobj.x2 = x2;
	dfbobj.y2 = y2;

	priv->device.funcs.DrawLine(priv->device.driver_data,
				    priv->device.device_data,
				    &dfbobj);

	vis->accelactive = 1;

	return 0;
}
