/* $Id: monitest.h,v 1.3 2005/07/31 15:30:41 soyt Exp $
******************************************************************************

   Monitest common declarations

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

/* I want to work with color names. Call me lazy :-) */
extern ggi_pixel white, black, red, green, blue, yellow, magenta, cyan;

/* Returns non-zero when the user requested 'break' */
int waitabit(ggi_visual_t vis);


void testpattern(ggi_visual_t vis);
void resolution(ggi_visual_t vis);
void flatpanel(ggi_visual_t vis);

