/* $Id: ggi-auto-physz.inc,v 1.2 2007/03/05 19:49:59 cegger Exp $
******************************************************************************

   Common GGI_AUTO handling code.
   
   Copyright (C) 1998  Andrew Apted  <andrew.apted@ggi-project.org>
   
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

/* ASSUMPTION:
 *
 * <ggi/internal/ggi.h> has been included before
 */

static int _GGIhandle_ggiauto_physz(ggi_mode *mode, int def_x, int def_y,
				int physzflag, const ggi_coord *screen_size,
				const ggi_coord *screen_res)
{
	int err;

	if (mode->frames == GGI_AUTO) {
		mode->frames = 1;
	}

	if (mode->dpp.x == GGI_AUTO) {
		mode->dpp.x = (GT_SCHEME(mode->graphtype)==GT_TEXT) ? 8 : 1;
	}

	if (mode->dpp.y == GGI_AUTO) {
		mode->dpp.y = (GT_SCHEME(mode->graphtype)==GT_TEXT) ? 8 : 1;
	}

	err = _ggi_physz_figure_visible(mode, def_x, def_y, physzflag,
				screen_size, screen_res);

	return err;
}
