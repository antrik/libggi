/* $Id: gt-auto.inc,v 1.1 2001/05/12 23:01:57 cegger Exp $
******************************************************************************

   Common GT_AUTO handling code.

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

static ggi_graphtype _GGIhandle_gtauto(ggi_graphtype gt)
{
	if (GT_SCHEME(gt) == GT_AUTO) {

		if ((GT_DEPTH(gt)!=GT_AUTO && GT_DEPTH(gt)>=9) ||
		    (GT_SIZE(gt) !=GT_AUTO && GT_SIZE(gt) >=9)) {

			GT_SETSCHEME(gt, GT_TRUECOLOR);
		} else {
			GT_SETSCHEME(gt, GT_PALETTE);
		}
	}

	if (GT_SCHEME(gt) == GT_TEXT) {

		if (GT_DEPTH(gt)==GT_AUTO && GT_SIZE(gt)==GT_AUTO) {
			GT_SETDEPTH(gt, 4);
			GT_SETSIZE(gt, 16);
			
		} else if (GT_DEPTH(gt) == GT_AUTO) {
			GT_SETDEPTH(gt, (GT_SIZE(gt) <= 16) ? 4 : 8);
			
		} else if (GT_SIZE(gt) == GT_AUTO) {
			GT_SETSIZE(gt, (GT_DEPTH(gt) <= 4) ? 16 : 32);
		}

		return gt;
	}

	if (GT_DEPTH(gt)==GT_AUTO && GT_SIZE(gt)==GT_AUTO) {
		GT_SETDEPTH(gt, (GT_SCHEME(gt)==GT_TRUECOLOR) ? 24 : 8);
		
	} else if (GT_DEPTH(gt) == GT_AUTO) {
		GT_SETDEPTH(gt, (GT_SIZE(gt) <= 24) ? GT_SIZE(gt) : 24);
	}

	if (GT_SIZE(gt) == GT_AUTO) {

		if (GT_DEPTH(gt) <= 8) {

			if (GT_DEPTH(gt) == 3) {
				GT_SETSIZE(gt, 4);
				
			} else if (GT_DEPTH(gt) >= 5) {
				GT_SETSIZE(gt, 8);
			} else {
				GT_SETSIZE(gt, GT_DEPTH(gt));
			}
		} else {
			GT_SETSIZE(gt, (GT_DEPTH(gt) + 7) & ~7);
		}
	}

	return gt;
}
