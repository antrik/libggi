/* $Id: visual.inc,v 1.1 2001/05/23 17:46:03 cegger Exp $
******************************************************************************

   shared visual code for display-X and display-Xlib.

   Copyright ...

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

/*
 * This file is included by Xlib/visual.c and X/visual.c
 */


static inline Cursor _ggi_x_make_cursor(Display *disp, Window root, int visible)
{
	Pixmap crsrpix, crsrmask;
	XColor black = { 0, 0x0, 0x0, 0x0 },
	       white = { 0, 0xffff, 0xffff, 0xffff };
	Cursor mycrsr;

	/* when visible, the mouse cursor is a small 3x3 dot */
	   
	if (visible) {
		char data_dot[] = { 0xf8, 0xfa, 0xf8 };
		char mask_dot[] = { 0xfa, 0xff, 0xfa };
	    
		crsrpix = XCreateBitmapFromData(disp, root, data_dot, 3, 3);
		crsrmask = XCreateBitmapFromData(disp, root, mask_dot, 3, 3);
	} else {
		char data_invisible[] = { 0 };
		char mask_invisible[] = { 0 };
	    
		crsrpix = XCreateBitmapFromData(disp, root, data_invisible, 1, 1);
		crsrmask = XCreateBitmapFromData(disp, root, mask_invisible, 1, 1);
	}
	mycrsr = XCreatePixmapCursor(disp, crsrpix, crsrmask,
				     &black, &white, 1, 1);
	XFreePixmap(disp, crsrpix);
	XFreePixmap(disp, crsrmask);

	return mycrsr;
}
