/* $Id: pointer.c,v 1.1 2004/10/01 12:41:58 pekberg Exp $
******************************************************************************

   This is a GGI test application. It is only valid for targets that can
   generate absolute and releative pointer motion simultaneously.

   Copyright (C) 2004 Peter Ekberg	[peda@lysator.liu.se]

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

#include <ggi/ggi.h>

#include <stdio.h>

int
main(int argc, char **argv)
{
	ggi_visual_t vis = NULL;
	ggi_mode mode;
	int err;
	int rx = 0, ry = 0;
	int ax = 0, ay = 0;
	int dx = 0, dy = 0;
	int rdx = 0, rdy = 0;
	int adx = 0, ady = 0;
	int ch_x, ch_y;
	ggi_pixel white;
	ggi_pixel black;
	ggi_color color;
	char tmpstr[2000];

	if (ggiInit() != 0) {
		fprintf(stderr, "%s: unable to initialize LibGGI, exiting.\n",
			argv[0]);
		goto out;
	}

	vis = ggiOpen(NULL);
	if (vis == NULL) {
		fprintf(stderr, "%s: unable to open visual, exiting.\n",
			argv[0]);
		goto out;
	}

	ggiSetFlags(vis, GGIFLAG_ASYNC);

	ggiCheckSimpleMode(vis, GGI_AUTO, GGI_AUTO, GGI_AUTO, GT_AUTO, &mode);

	err = ggiSetMode(vis, &mode);
	if (err < 0) {
		fprintf(stderr, "%s: unable to set simple mode, exiting.\n",
			argv[0]);
		goto out;
	}

	ggiSetColorfulPalette(vis);

	color.r = color.g = color.b = 0xFFFF;
	white = ggiMapColor(vis, &color);
	color.r = color.g = color.b = 0x0;
	black = ggiMapColor(vis, &color);

	ggiSetGCForeground(vis, white);
	ggiSetGCBackground(vis, black);

	ggiGetCharSize(vis, &ch_x, &ch_y);

	ggiAddEventMask(vis, emPtrRelative | emPtrAbsolute);

	while (ggiEventPoll(vis, emAll, NULL)) {
		ggi_event event;
		char t = ' ';
		ggiEventRead(vis, &event, emAll);

		if (event.any.type == evPtrRelative) {
			rdx = event.pmove.x;
			rdy = event.pmove.y;
			rx += event.pmove.x;
			ry += event.pmove.y;
			t = 'R';
		}

		else if (event.any.type == evPtrAbsolute) {
			adx = event.pmove.x - ax;
			ady = event.pmove.y - ay;
			ax = event.pmove.x;
			ay = event.pmove.y;
			t = 'A';
		}
		
		else if (event.any.type == evKeyPress)
			break;

		if (t == ' ')
			continue;

		sprintf(tmpstr, "%c rel(%-4d,%-4d) "
			"abs(%-4d,%-4d) diff(%-3d,%-3d)   ",
			t, rx, ry, ax, ay, rx-ax, ry-ay);
		dx = rx-ax;
		dy = ry-ay;
		ggiPuts(vis, 0, t == 'R' ? ch_y : 0, tmpstr);
		ggiFlush(vis);
	}

out:
	if (vis)
		ggiClose(vis);
	ggiExit();	

	return 0;
}
