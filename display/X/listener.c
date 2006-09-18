/* $Id: listener.c,v 1.5 2006/09/18 19:42:42 cegger Exp $
******************************************************************************

   LibGGI - listener for display-x

   Copyright (C) 2006	Christoph Egger

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


int GGI_X_listener(void *arg, int flag, void *data)
{
	struct ggi_visual *vis = arg;
	ggi_x_priv *priv = GGIX_PRIV(vis);

	if (flag & GII_CMDCODE_RESIZE) {
		struct gii_cmddata_resize *resize;

		resize = (struct gii_cmddata_resize *)data;

		/* todo: update priv structure here.
		 *  Don't resize the window itself.
		 *  This is libggiwmh's job.
		 */
	}

	if ((flag & GII_CMDCODE_XWINEXPOSE) == GII_CMDCODE_XWINEXPOSE) {
		struct gii_cmddata_expose *expose;

		expose = (struct gii_cmddata_expose *)data;

		GGI_X_expose(expose->arg,
			expose->x, expose->y, expose->w, expose->h);
	}

	/* GII_CMDCODE_XWINPOINTER and GII_CMDCODE_XWINSETPARAM
	 * are not relevant here.
	 * So we ignore this event.
	 */
	return 0;
}
