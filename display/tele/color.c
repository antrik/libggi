/* $Id: color.c,v 1.11 2006/03/22 20:22:29 cegger Exp $
******************************************************************************

   TELE target.

   Copyright (C) 1998 Andrew Apted    [andrew@ggi-project.org]

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
#include <ggi/internal/ggi-dl.h>

#include "libtele.h"
#include <ggi/display/tele.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#define MAX_COLORS  TELE_MAXIMUM_TLONG(TeleCmdSetPaletteData)


int GGI_tele_setPalette(struct ggi_visual *vis, size_t start, size_t size, const ggi_color *cols)
{
	ggi_tele_priv *priv = TELE_PRIV(vis);
	TeleCmdSetPaletteData *c;
	TeleEvent ev;

	size_t no_cols;

	int err;

	int len = (int)size;

	if (cols == NULL) return GGI_EARGINVAL;

	if (GT_SCHEME(LIBGGI_GT(vis)) != GT_PALETTE) {
		return GGI_ENOMATCH;
	}

	no_cols = 1 << GT_DEPTH(LIBGGI_GT(vis));

	if ((start+size) > no_cols) {
		return GGI_ENOSPACE;
	}

	memcpy(LIBGGI_PAL(vis)->clut.data+start, cols, size*sizeof(ggi_color)); 


	/* send palette to the server */

	for (; len > 0; ) {

		unsigned int i;
		unsigned int num = len;

		if (num > MAX_COLORS) {
			num = MAX_COLORS;
		}

		c = tclient_new_event(priv->client, &ev, TELE_CMD_SETPALETTE,
				(signed)(sizeof(TeleCmdSetPaletteData) + num*4),
				0);
				
		c->start = start;
		c->len   = num;

		for (i = 0; i < num; i++) {
			c->colors[i] = ((cols->r & 0xff00) << 8) |
				       ((cols->g & 0xff00)     ) |
				       ((cols->b & 0xff00) >> 8);
			cols++; start++; len--;
		}

		err = tclient_write(priv->client, &ev);

		if (err == TELE_ERROR_SHUTDOWN) {
			TELE_HANDLE_SHUTDOWN;
		} else if (err < 0) {
			return err;
		}
	}
	
	return 0;
}
