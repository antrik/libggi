/* $Id: drawlib.c,v 1.4 2004/09/08 17:51:02 cegger Exp $
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
#include "drawlib.h"

#define ASSERT(x) do {}while (0)

int circle(ggi_visual_t vis,int xcenter,int ycenter,int radius)
{
	/* easiest implementation, not time-optimised on weird archs. */
        int x,y,od,md,sd;

	/* original clipping code deleted. Leave clipping to ggiDrawPixel */
	/* shouldn't clip in this application anyway. */

        x=radius;y=md=0;
        while(x>=y) {
		ggiDrawPixel(vis, xcenter-y, ycenter-x);
		ggiDrawPixel(vis, xcenter+y, ycenter-x);
		ggiDrawPixel(vis, xcenter+x, ycenter-y);
		ggiDrawPixel(vis, xcenter-x, ycenter-y);
		ggiDrawPixel(vis, xcenter-x, ycenter+y);
		ggiDrawPixel(vis, xcenter+x, ycenter+y);
		ggiDrawPixel(vis, xcenter+y, ycenter+x);
		ggiDrawPixel(vis, xcenter-y, ycenter+x);
		od = md+y+y+1;
		sd = od-x-x-1;
		y++;
		md = od;
		if ((abs(sd)) < (abs(od))) {
			x--;
			md = sd;
		}
        }

        return(0);
}

void
dotone(ggi_visual_t vis, int x1, int y1, int x2, int y2,
       ggi_pixel col1, ggi_pixel col2)
{
	int x, y;
	ggiSetGCForeground(vis, col1);
	ggiDrawBox(vis, x1, y1, x2-x1+1, y2-y1+1);

	ggiSetGCForeground(vis, col2);
	for (y=y1; y<=y2;y+=2)
		for (x=x1; x<=x2;x+=2)
			ggiPutPixel(vis, x, y, col2);
}

void
chessboardone(ggi_visual_t vis, int x1, int y1, int x2, int y2,
	      ggi_pixel col1, ggi_pixel col2)
{
	int x, y;

	ggiSetGCForeground(vis, col1);
	ggiDrawBox(vis, x1, y1, x2-x1+1, y2-y1+1);

	for (y=y1; y<y2;y+=2){
		for (x=x1; x<x2;x+=2){
			ggiPutPixel(vis, x,y, col2);
			ggiPutPixel(vis, x+1, y+1, col2);
		}
	}

}
                 
void
stripevert(ggi_visual_t vis, int x1, int y1, int x2, int y2,
	   ggi_pixel col1, ggi_pixel col2, unsigned int s)
{
	int x, xi;
	ASSERT(s > 0);

	ggiSetGCForeground(vis, col1);
	ggiDrawBox(vis, x1, y1, x2-x1+1, y2-y1+1);

	ggiSetGCForeground(vis, col2);
	for (x = x1; x <= x2; x += s+s){
		for (xi = 0; xi < (signed)s; xi++){
			ggiDrawVLine(vis, x+xi, y1, y2-y1+1);
		}
	}
}

void
stripehor(ggi_visual_t vis, int x1, int y1, int x2, int y2,
	  ggi_pixel col1, ggi_pixel col2, unsigned int s)
{
	int y, yi;
	ASSERT(s > 0);

	ggiSetGCForeground(vis, col1);
	ggiDrawBox(vis, x1, y1, x2-x1+1, y2-y1+1);

	ggiSetGCForeground(vis, col2);
	for (y = y1; y <= y2; y += s+s){
		for (yi = 0;yi < (signed)s; yi++){
			ggiDrawHLine(vis, x1, y+yi, x2-x1+1);
		}
	}
}

void
starone(ggi_visual_t vis, int x1, int y1, int x2, int y2,
	ggi_pixel col1, ggi_pixel col2)
{
	int x, y;

	ggiSetGCForeground(vis, col1);
	ggiDrawBox(vis, x1, y1, x2-x1+1, y2-y1+1);

	ggiSetGCForeground(vis, col2);
	for (x=x1; x<=x2;x+=2){
		ggiDrawLine(vis, x,y1, x2+x1-x, y2);
	}
	for (y=y1; y<=y2;y+=2){
		ggiDrawLine(vis, x1, y,x2, y2+y1-y);
	}
}

void
starfive(ggi_visual_t vis, int x1, int y1, int x2, int y2,
	 ggi_pixel col1, ggi_pixel col2)
{
	int x, y;
	ggiSetGCForeground(vis, col1);
	ggiDrawBox(vis, x1, y1, x2-x1+1, y2-y1+1);

        ggiSetGCForeground(vis, col2);
	for (x=x1; x<=x2;x+=5){
		ggiDrawLine(vis, x,y1, x2+x1-x, y2);
	}
	for (y=y1; y<=y2;y+=5){
		ggiDrawLine(vis, x1, y,x2, y2+y1-y);
	}
}

void
startwoten(ggi_visual_t vis, int x1, int y1, int x2, int y2,
	   ggi_pixel col1, ggi_pixel col2)
{
	int x, y;

	ggiSetGCForeground(vis, col1);
	ggiDrawBox(vis, x1, y1, x2-x1+1, y2-y1+1);

	ggiSetGCForeground(vis, col2);
	x = x1 ;
	while (x < x2){
		ggiDrawLine(vis, x,y1, x2+x1-x, y2);
		ggiDrawLine(vis, x+1, y1, x2+x1-x-1, y2);
		x +=10;
	}
	y = y1 ;
	while (y < y2) {
		ggiDrawLine(vis, x1, y,x2, y2+y1-y);
		ggiDrawLine(vis, x1, y+1, x2, y2+y1-y-1);
		y += 10;
	}
}
