/* $Id: line.c,v 1.1 2002/10/23 23:42:39 redmondp Exp $
******************************************************************************

   ATI Radeon line acceleration

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

*******************************************************************************/

#include "radeon_accel.h"

int GGI_kgi_radeon_drawhline(ggi_visual *vis, int x, int y, int w)
{
	return 0;
}

int GGI_kgi_radeon_drawvline(ggi_visual *vis, int x, int y, int h)
{
	return 0;
}

int GGI_kgi_radeon_drawline(ggi_visual *vis, int x1, int y1, int x2, int y2)
{
	return 0;
}
