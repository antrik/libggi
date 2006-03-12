/* $Id: copybox.c,v 1.7 2006/03/12 23:15:12 soyt Exp $
******************************************************************************
   Graphics library for GGI.

   Copyright (C) 1998 Andrew Apted	[andrew@ggi-project.org]
   Copyright (C) 1999 Marcus Sundberg	[marcus@ggi-project.org]

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

#include "stublib.h"

/* Maximum amount of bytes to allocate on the stack */
#define MAX_STACKBYTES	4096

static inline void
do_copy(struct ggi_visual *vis, int x, int y, int w, int h, int nx, int ny, void *buf)
{
		if (ny > y) {
			for (y+=h-1, ny+=h-1; h > 0; h--, y--, ny--) {
				ggiGetHLine(vis, x,  y,  w, buf);
				ggiPutHLine(vis, nx, ny, w, buf);
			}
		} else {
			for (; h > 0; h--, y++, ny++) {
				ggiGetHLine(vis, x,  y,  w, buf);
				ggiPutHLine(vis, nx, ny, w, buf);
			}
		}
}


int
GGI_stubs_copybox(struct ggi_visual *vis, int x, int y, int w, int h, int nx, int ny)
{
	size_t size;

	LIBGGICLIP_COPYBOX(vis, x, y, w, h, nx, ny);

	size = GT_ByPPP(w, LIBGGI_GT(vis));
	if (size <= MAX_STACKBYTES) {
		uint8_t buf[MAX_STACKBYTES];

		do_copy(vis, x, y, w, h, nx, ny, buf);
	} else {
		uint8_t *buf = malloc(size);

		if (!buf) return GGI_ENOMEM;
		do_copy(vis, x, y, w, h, nx, ny, buf);
		free(buf);
	}

	return 0;
}
