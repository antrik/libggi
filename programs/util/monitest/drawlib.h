/* $Id: drawlib.h,v 1.1 2001/05/12 23:03:50 cegger Exp $
******************************************************************************

   Monitest drawing library: fields with stripes and stars, circle.

   Written in 1998 by Hartmut Niemann

   This software is placed in the public domain and can be used freely
   for any purpose. It comes without any kind of warranty, either
   expressed or implied, including, but not limited to the implied
   warranties of merchantability or fitness for a particular purpose.
   Use it at your own risk. the author is not responsible for any damage
   or consequences raised by use or inability to use this program.
 
******************************************************************************
*/

#include <ggi/ggi.h>

int circle(ggi_visual_t vis, int xcenter, int ycenter, int radius);

void dotone(ggi_visual_t vis,
	    int x1, int y1, int x2, int y2,
	    ggi_pixel col1, ggi_pixel col2);

void chessboardone(ggi_visual_t vis,
		   int x1, int y1, int x2, int y2,
		   ggi_pixel col1, ggi_pixel col2);
                 
void stripevert(ggi_visual_t vis,
		int x1, int y1, int x2, int y2,
		ggi_pixel col1, ggi_pixel col2, unsigned int s);

void stripehor(ggi_visual_t vis,
	       int x1, int y1, int x2, int y2,
	       ggi_pixel col1, ggi_pixel col2, unsigned int s);

void starone(ggi_visual_t vis,
	     int x1, int y1, int x2, int y2,
	     ggi_pixel col1, ggi_pixel col2);

void starfive(ggi_visual_t vis,
	      int x1, int y1, int x2, int y2,
	      ggi_pixel col1, ggi_pixel col2);

void startwoten(ggi_visual_t vis,
		int x1, int y1, int x2, int y2,
		ggi_pixel col1, ggi_pixel col2);


