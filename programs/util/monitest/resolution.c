/* $Id: resolution.c,v 1.2 2003/07/05 14:04:25 cegger Exp $
******************************************************************************

   Monitest resolution test

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
#include "monitest.h"

void resolution(ggi_visual_t vis)
{
	int xmax, ymax;
	ggi_mode currmode;

	ggiGetMode(vis, &currmode);
	xmax = currmode.visible.x;
	ymax = currmode.visible.y;

	ggiSetGCForeground(vis, blue);
	ggiDrawBox(vis, 0, 0, xmax, ymax);

	stripevert(vis, 0, 0, xmax - 1, ymax - 1,
		   black, white, 1);
	ggiFlush(vis);
	if (waitabit(vis))
		return;

	stripevert(vis, 0, 0, xmax - 1, ymax - 1,
		   black, white, 2);
	ggiFlush(vis);
	if (waitabit(vis))
		return;

	stripevert(vis, 0, 0, xmax - 1, ymax - 1,
		   black, white, 3);
	ggiFlush(vis);
	if (waitabit(vis))
		return;

	stripevert(vis, 0, 0, xmax - 1, ymax - 1,
		   black, white, 4);
	ggiFlush(vis);
	if (waitabit(vis))
		return;

	stripehor(vis, 0, 0, xmax - 1, ymax - 1, black, white, 1);
	ggiFlush(vis);
	if (waitabit(vis))
		return;

	stripehor(vis, 0, 0, xmax - 1, ymax - 1, black, white, 2);
	ggiFlush(vis);
	if (waitabit(vis))
		return;

	starone(vis, 0, 0, xmax - 1, ymax - 1, black, white);
	ggiFlush(vis);
	if (waitabit(vis))
		return;

	starfive(vis, 0, 0, xmax - 1, ymax - 1, white, black);
	ggiFlush(vis);
	if (waitabit(vis))
		return;

	startwoten(vis, 0, 0, xmax - 1, ymax - 1, white, black);
	ggiFlush(vis);
	if (waitabit(vis))
		return;

	ggiSetGCForeground(vis, black);
	ggiDrawBox(vis, 0, 0, xmax, ymax);

	stripevert(vis, 1 * xmax / 16, 2 * ymax / 8, 3 * xmax / 16 - 1,
		   3 * ymax / 8 - 1, black, red, 4);
	stripevert(vis, 3 * xmax / 16, 2 * ymax / 8, 5 * xmax / 16 - 1,
		   3 * ymax / 8 - 1, black, red, 3);
	stripevert(vis, 5 * xmax / 16, 2 * ymax / 8, 7 * xmax / 16 - 1,
		   3 * ymax / 8 - 1, black, red, 2);
	stripevert(vis, 7 * xmax / 16, 2 * ymax / 8, 9 * xmax / 16 - 1,
		   3 * ymax / 8 - 1, black, red, 1);
	stripevert(vis, 9 * xmax / 16, 2 * ymax / 8, 11 * xmax / 16 - 1,
		   3 * ymax / 8 - 1, black, red, 2);
	stripevert(vis, 11 * xmax / 16, 2 * ymax / 8, 13 * xmax / 16 - 1,
		   3 * ymax / 8 - 1, black, red, 3);
	stripevert(vis, 13 * xmax / 16, 2 * ymax / 8, 15 * xmax / 16 - 1,
		   3 * ymax / 8 - 1, black, red, 4);
	stripevert(vis, 1 * xmax / 16, 3 * ymax / 8, 3 * xmax / 16 - 1,
		   4 * ymax / 8 - 1, black, green, 4);
	stripevert(vis, 3 * xmax / 16, 3 * ymax / 8, 5 * xmax / 16 - 1,
		   4 * ymax / 8 - 1, black, green, 3);
	stripevert(vis, 5 * xmax / 16, 3 * ymax / 8, 7 * xmax / 16 - 1,
		   4 * ymax / 8 - 1, black, green, 2);
	stripevert(vis, 7 * xmax / 16, 3 * ymax / 8, 9 * xmax / 16 - 1,
		   4 * ymax / 8 - 1, black, green, 1);
	stripevert(vis, 9 * xmax / 16, 3 * ymax / 8, 11 * xmax / 16 - 1,
		   4 * ymax / 8 - 1, black, green, 2);
	stripevert(vis, 11 * xmax / 16, 3 * ymax / 8, 13 * xmax / 16 - 1,
		   4 * ymax / 8 - 1, black, green, 3);
	stripevert(vis, 13 * xmax / 16, 3 * ymax / 8, 15 * xmax / 16 - 1,
		   4 * ymax / 8 - 1, black, green, 4);
	stripevert(vis, 1 * xmax / 16, 4 * ymax / 8, 3 * xmax / 16 - 1,
		   5 * ymax / 8 - 1, black, blue, 4);
	stripevert(vis, 3 * xmax / 16, 4 * ymax / 8, 5 * xmax / 16 - 1,
		   5 * ymax / 8 - 1, black, blue, 3);
	stripevert(vis, 5 * xmax / 16, 4 * ymax / 8, 7 * xmax / 16 - 1,
		   5 * ymax / 8 - 1, black, blue, 2);
	stripevert(vis, 7 * xmax / 16, 4 * ymax / 8, 9 * xmax / 16 - 1,
		   5 * ymax / 8 - 1, black, blue, 1);
	stripevert(vis, 9 * xmax / 16, 4 * ymax / 8, 11 * xmax / 16 - 1,
		   5 * ymax / 8 - 1, black, blue, 2);
	stripevert(vis, 11 * xmax / 16, 4 * ymax / 8, 13 * xmax / 16 - 1,
		   5 * ymax / 8 - 1, black, blue, 3);
	stripevert(vis, 13 * xmax / 16, 4 * ymax / 8, 15 * xmax / 16 - 1,
		   5 * ymax / 8 - 1, black, blue, 4);
	stripevert(vis, 1 * xmax / 16, 5 * ymax / 8, 3 * xmax / 16 - 1,
		   6 * ymax / 8 - 1, black, white, 4);
	stripevert(vis, 3 * xmax / 16, 5 * ymax / 8, 5 * xmax / 16 - 1,
		   6 * ymax / 8 - 1, black, white, 3);
	stripevert(vis, 5 * xmax / 16, 5 * ymax / 8, 7 * xmax / 16 - 1,
		   6 * ymax / 8 - 1, black, white, 2);
	stripevert(vis, 7 * xmax / 16, 5 * ymax / 8, 9 * xmax / 16 - 1,
		   6 * ymax / 8 - 1, black, white, 1);
	stripevert(vis, 9 * xmax / 16, 5 * ymax / 8, 11 * xmax / 16 - 1,
		   6 * ymax / 8 - 1, black, white, 2);
	stripevert(vis, 11 * xmax / 16, 5 * ymax / 8, 13 * xmax / 16 - 1,
		   6 * ymax / 8 - 1, black, white, 3);
	stripevert(vis, 13 * xmax / 16, 5 * ymax / 8, 15 * xmax / 16 - 1,
		   6 * ymax / 8 - 1, black, white, 4);
	ggiFlush(vis);
	if (waitabit(vis))
		return;
}
