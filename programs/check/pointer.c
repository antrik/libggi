/* $Id: pointer.c,v 1.2 2004/10/01 17:12:18 pekberg Exp $
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
	int ch_x, ch_y;
	int quit = 0;
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

	while (!quit) {
		int n;

		ggiEventPoll(vis, emAll, NULL);

		n = ggiEventsQueued(vis, emAll);

		while (n--) {
			ggi_event event;
			ggiEventRead(vis, &event, emAll);
	
			if (event.any.type == evPtrRelative) {
				rx += event.pmove.x;
				ry += event.pmove.y;
				sprintf(tmpstr, " rel(%-4d,%-4d)   ",
					rx, ry);
				ggiPuts(vis, 0, 1 * ch_y, tmpstr);
				sprintf(tmpstr, "diff(%-4d,%-4d)   ",
					rx-ax, ry-ay);
				ggiPuts(vis, 0, 2 * ch_y, tmpstr);
			}
	
			else if (event.any.type == evPtrAbsolute) {
				ax = event.pmove.x;
				ay = event.pmove.y;
				sprintf(tmpstr, " abs(%-4d,%-4d)   ",
					ax, ay);
				ggiPuts(vis, 0, 0 * ch_y, tmpstr);
				sprintf(tmpstr, "diff(%-4d,%-4d)   ",
					rx-ax, ry-ay);
				ggiPuts(vis, 0, 2 * ch_y, tmpstr);
			}
			
			else if (event.any.type == evKeyPress) {
				quit = 1;
				break;
			}

			else
				continue;
		}
		ggiFlush(vis);
	}

out:
	if (vis)
		ggiClose(vis);
	ggiExit();	

	return 0;
}
