/* $Id: pointer.c,v 1.8 2006/09/23 09:05:22 cegger Exp $
******************************************************************************

   This is a GGI test application. It is only valid for targets that can
   generate absolute and releative pointer motion simultaneously.
   Click a pointer button to sync up relative with absolute coordinates.

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

#include "config.h"
#include <ggi/gii.h>
#include <ggi/gii-keyboard.h>
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
	int ox = 0, oy = 0;
	int old_type;
	int ch_x, ch_y;
	int box_size;
	int quit = 0;
	ggi_pixel white;
	ggi_pixel red;
	ggi_pixel yellow;
	ggi_pixel black;
	ggi_color color;
	char tmpstr[2000];

	if (giiInit() != 0) {
		fprintf(stderr, "%s: unable to initialize LibGII, exiting.\n",
			argv[0]);
		goto out;
	}
	if (ggiInit() != 0) {
		fprintf(stderr, "%s: unable to initialize LibGGI, exiting.\n",
			argv[0]);
		goto out;
	}

	vis = ggNewStem(NULL);
	if (!vis) {
		fprintf(stderr, "%s: unable to creat stem, exiting.\n",
			argv[0]);
		goto out;
	}
	if (ggiAttach(vis) < 0) {
		fprintf(stderr, "%s: unable to attach ggi, exiting.\n",
			argv[0]);
		goto out;
	}
	if (giiAttach(vis) < 0) {
		fprintf(stderr, "%s: unable to attach gii, exiting.\n",
			argv[0]);
		goto out;
	}
	if (ggiOpen(vis, NULL) < 0) {
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
	color.b = 0x0;
	yellow = ggiMapColor(vis, &color);
	color.g = 0x0;
	red = ggiMapColor(vis, &color);
	color.r = 0x0;
	black = ggiMapColor(vis, &color);

	ggiSetGCForeground(vis, white);
	ggiSetGCBackground(vis, black);

	ggiGetCharSize(vis, &ch_x, &ch_y);
	if (ch_x > ch_y)
		box_size = 2 * ch_x;
	else
		box_size = 2 * ch_y;

	giiAddEventMask(vis, emPtrRelative | emPtrAbsolute);

	while (!quit) {
		int n;

		giiEventPoll(vis, emAll, NULL);

		n = giiEventsQueued(vis, emAll);

		while (n--) {
			gii_event event;
			giiEventRead(vis, &event, emAll);
	
			if (event.any.type == evPtrRelative) {
				ox = rx;
				oy = ry;
				old_type = 0;
				rx += event.pmove.x;
				ry += event.pmove.y;
				sprintf(tmpstr, " rel(%-4d,%-4d)   ",
					rx, ry);
				ggiPuts(vis, 0, 1 * ch_y, tmpstr);
			}
	
			else if (event.any.type == evPtrAbsolute) {
				ox = ax;
				oy = ay;
				old_type = 1;
				ax = event.pmove.x;
				ay = event.pmove.y;
				sprintf(tmpstr, " abs(%-4d,%-4d)   ",
					ax, ay);
				ggiPuts(vis, 0, 0 * ch_y, tmpstr);
			}

			else if (event.any.type == evPtrButtonPress) {
				ox = rx;
				oy = ry;
				old_type = 0;
				rx = ax;
				ry = ay;
				sprintf(tmpstr, " rel(%-4d,%-4d)   ",
					rx, ry);
				ggiPuts(vis, 0, 1 * ch_y, tmpstr);
			}
			
			else if (event.any.type == evKeyPress) {
				if (event.key.label == GIIUC_Escape)
					quit = 1;
				if (event.key.label == GIIUC_Q)
					quit = 1;
				break;
			}

			else
				continue;

			sprintf(tmpstr, "diff(%-4d,%-4d)   ",
				rx-ax, ry-ay);
			ggiPuts(vis, 0, 2 * ch_y, tmpstr);

			ggiSetGCForeground(vis, black);
			if (old_type) {
				ggiDrawBox(vis, ox - box_size, oy + 1,
					   box_size, box_size);
				ggiDrawBox(vis, ox + 1, oy - box_size,
					   box_size, box_size);
			}
			else {
				ggiDrawBox(vis, ox - box_size, oy - box_size,
					   box_size, box_size);
				ggiDrawBox(vis, ox + 1, oy + 1,
					   box_size, box_size);
			}

			ggiSetGCForeground(vis, red);
			ggiDrawBox(vis, ax - box_size, ay + 1,
				   box_size, box_size);
			ggiDrawBox(vis, ax + 1, ay - box_size,
				   box_size, box_size);
			ggiSetGCForeground(vis, white);
			ggiSetGCBackground(vis, red);
			ggiPutc(vis,
				ax - 3 * box_size / 4,
				ay + box_size / 4 + 1, 'A');
			ggiPutc(vis,
				ax + box_size / 4 + 1,
				ay - 3 * box_size / 4, 'A');

			ggiSetGCForeground(vis, yellow);
			ggiDrawBox(vis, rx - box_size, ry - box_size,
				   box_size, box_size);
			ggiDrawBox(vis, rx + 1, ry + 1,
				   box_size, box_size);
			ggiSetGCForeground(vis, black);
			ggiSetGCBackground(vis, yellow);
			ggiPutc(vis,
				rx - 3 * box_size / 4,
				ry - 3 * box_size / 4, 'R');
			ggiPutc(vis,
				rx + box_size / 4 + 1,
				ry + box_size / 4 + 1, 'R');

			ggiSetGCForeground(vis, white);
			ggiSetGCBackground(vis, black);
		}
		ggiFlush(vis);
	}

out:
	if (vis) {
		ggiClose(vis);
		ggDelStem(vis);
	}
	ggiExit();
	giiExit();

	return 0;
}
