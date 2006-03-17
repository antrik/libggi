/* $Id: pixfmt-setup.inc,v 1.3 2006/03/17 21:55:42 cegger Exp $
******************************************************************************

   Set up vis->pixfmt for 'default' modes when the target doesn't care. 
   E.g. memory target. 

   If your target's mode deviate in any way from the 'default' then do
   NOT use this code.

   Copyright (C) 1998  Andrew Apted     [andrew.apted@ggi-project.org]
   Copyright (C) 1998  Steve Cheng      [steve@ggi-project.org]

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

/* Assumes pixfmt is zero'ed, but you could also set some
   flags and this will not overwrite it. */

static int setup_pixfmt(ggi_pixelformat *pixfmt, ggi_graphtype gt)
{

	pixfmt->depth = GT_DEPTH(gt);
	pixfmt->size  = GT_SIZE(gt);

	pixfmt->stdformat=0;

	switch (GT_SCHEME(gt)) {
	case GT_PALETTE:
	case GT_GREYSCALE:
		pixfmt->clut_mask = (1 << pixfmt->depth) - 1;
		return 0;

	case GT_TEXT:
		switch (pixfmt->size) {
		case 16:
			pixfmt->texture_mask = 0x00FF;
			pixfmt->fg_mask      = 0x0F00;
			pixfmt->bg_mask      = 0xF000;
			return 0;

		case 32:
			pixfmt->texture_mask = 0xFF000000;
			pixfmt->fg_mask      = 0x000000FF;
			pixfmt->bg_mask      = 0x0000FF00;
			return 0;
		}
		break;

	case GT_TRUECOLOR:
		if (pixfmt->depth >= 3) {
			int rnum = (pixfmt->depth+1) / 3;
			int gnum = (pixfmt->depth+2) / 3;
			int bnum = (pixfmt->depth)   / 3;

			pixfmt->red_mask   = ((1 << rnum)-1) << (bnum+gnum);
			pixfmt->green_mask = ((1 << gnum)-1) << (bnum);
			pixfmt->blue_mask  = ((1 << bnum)-1);

			return 0;
		}
	}


	ggPanic("SETUP_PIXFMT: called with unsupported graphtype!"
		" (0x%08x)\n"
		"Please report this error to the target maintainer", gt);

	return GGI_ENOMATCH; /* Avoid compiler warning */
}
