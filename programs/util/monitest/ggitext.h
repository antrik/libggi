/* $Id: ggitext.h,v 1.3 2005/07/31 15:30:40 soyt Exp $
******************************************************************************

   Header for ggitext functions for formatted text output

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

int ggiGraphTextCharwidth(ggi_visual_t  vis, const char c);
int ggiGraphTextCharheight(ggi_visual_t  vis, const char c);
int ggiGraphTextStringwidth(ggi_visual_t  vis, const char * c);
int ggiGraphTextStringheight(ggi_visual_t  vis, const char * c);

/* int ggiGraphTextFont(vis, char * fontnameorpath, uint size, int flags ); */

/*int ggiGraphTextDirection(vis, int direction);*/
#define GGI_TEXT_DIRECTION_RIGHT 0
#define GGI_TEXT_DIRECTION_UP 90
#define GGI_TEXT_DIRECTION_LEFT 180
#define GGI_TEXT_DIRECTION_DOWN 270


int ggiGraphTextPuts(ggi_visual_t  vis,
		     int x, int y, int width, int height,
		     int flags,
		     char * text );
int ggiGraphTextLongPuts(ggi_visual_t  vis,
		     int x, int y, int width, int height,
		     int flags,
		     char * text );
/* breaks text in individual lines (at \n points)*/

/* centering flags: */
#define GGI_TEXT_CENTER 0
#define GGI_TEXT_LEFT   1
#define GGI_TEXT_RIGHT  2
#define GGI_TEXT_JUSTIFY 3  /* means right and left, i.e. "Blocksatz" */
/* good luck when implementing */
#define GGI_TEXT_TOP    4
#define GGI_TEXT_BOTTOM 8

/* normal for would be GGI_TEXT_LEFT+GGI_TEXT_BOTTOM, reference point
  is the left end base line */

#define GGI_TEXT_START  16
#define GGI_TEXT_END    32
#define GGI_TEXT_BOTTOMLINE 64
#define GGI_TEXT_TOPLINE  128
/* this means positioning relative to the printed (and rotated) text,
   not to the "bounding box" */

#define GGI_TEXT_FRAME 256 /* paint the box */
