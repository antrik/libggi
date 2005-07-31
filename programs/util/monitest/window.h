/* $Id: window.h,v 1.3 2005/07/31 15:30:41 soyt Exp $
******************************************************************************

   Universal window for LibGGI

   Written in 1998 by Hartmut Niemann

   This software is placed in the public domain and can be used freely
   for any purpose. It comes without any kind of warranty, either
   expressed or implied, including, but not limited to the implied
   warranties of merchantability or fitness for a particular purpose.
   Use it at your own risk. the author is not responsible for any damage
   or consequences raised by use or inability to use this program.

******************************************************************************
*/

#ifndef _WINDOW_H
#define _WINDOW_H

#include <ggi/ggi.h>

struct window {
	ggi_visual_t vis ;           /* where to display */

	int xorigin;                 /* upper left corner */
	int yorigin;                 /* relative to visual */
	int xsize;                   /* graph width       */
	int ysize;

	ggi_color backgroundcolor;

	int borderwidth;             /* coloured border with # pixels */
	ggi_color bordercolor;

	const char *title ;
	ggi_color titlecolor;
};

int default_window(struct window *w, ggi_visual_t vis);

int draw_window(struct window *w);

#endif /* _WINDOW_H */
