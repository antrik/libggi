/* $Id: ggi-auto.inc,v 1.3 2004/10/14 09:08:41 cegger Exp $
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
 * <ggi/internal/internal.h> has been included before
 */

static void _GGIhandle_ggiauto(ggi_mode *mode, int def_x, int def_y)
{
	if (mode->frames == GGI_AUTO) {
		mode->frames = 1;
	}

	if (mode->dpp.x == GGI_AUTO) {
		mode->dpp.x = (GT_SCHEME(mode->graphtype)==GT_TEXT) ? 8 : 1;
	}

	if (mode->dpp.y == GGI_AUTO) {
		mode->dpp.y = (GT_SCHEME(mode->graphtype)==GT_TEXT) ? 8 : 1;
	}

	if (mode->visible.x==GGI_AUTO && mode->virt.x==GGI_AUTO) {
		mode->visible.x = mode->virt.x = def_x;

	} else if (mode->visible.x == GGI_AUTO) {
		mode->visible.x = mode->virt.x;

	} else if (mode->virt.x == GGI_AUTO) {
		mode->virt.x = mode->visible.x;
	}

	if (mode->visible.y==GGI_AUTO && mode->virt.y==GGI_AUTO) {
		mode->visible.y = mode->virt.y = def_y;

	} else if (mode->visible.y == GGI_AUTO) {
		mode->visible.y = mode->virt.y;

	} else if (mode->virt.y == GGI_AUTO) {
		mode->virt.y = mode->visible.y;
	}

}
