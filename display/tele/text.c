/* $Id: text.c,v 1.8 2004/02/02 19:21:59 cegger Exp $
******************************************************************************

   TELE target.

   Copyright (C) 1998 Andrew Apted    [andrew@ggi-project.org]
                 2002 Tobias Hunger   [tobias@fresco.org]

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
#include <unistd.h>


int GGI_tele_getcharsize(ggi_visual *vis, int *width, int *height)
{
	ggi_tele_priv *priv = TELE_PRIV(vis);
	TeleCmdGetCharSizeData *p;
	TeleEvent ev;
	int err;

        p = tclient_new_event(priv->client, &ev, TELE_CMD_GETCHARSIZE,
				sizeof(TeleCmdGetCharSizeData), 0);

	err = tclient_write(priv->client, &ev);

	if (err == TELE_ERROR_SHUTDOWN) {
		TELE_HANDLE_SHUTDOWN;
	} else if (err < 0) {
		return err;
	}	/* if */

	tele_receive_reply(vis, &ev, TELE_CMD_GETCHARSIZE,
			ev.sequence);

	*width = p->width;
	*height = p->height;

	return 0;
}	/* GGI_tele_getcharsize */


int GGI_tele_putc(ggi_visual *vis, int x, int y, char c)
{
	char s[2];
	s[0] = c; s[1] = '\0';

	return GGI_tele_puts(vis, x, y, s);
}	/* GGI_tele_putc */



int GGI_tele_puts(ggi_visual *vis, int x, int y, const char * s)
{
	ggi_tele_priv *priv = TELE_PRIV(vis);
	TeleCmdPutStrData *p;
	TeleEvent ev;
	int err = 0;
	unsigned int i = 0;

	T_Long * data;

	p = tclient_new_event(priv->client, &ev, TELE_CMD_PUTSTR,
				sizeof(TeleCmdPutStrData)-4,
				(signed)((strlen(s) + 1) * sizeof(T_Long)));

	p->x = (T_Long)(x);
	p->y = (T_Long)(y);
	p->length = (T_Long)strlen(s);
	p->fg = (T_Long)(LIBGGI_GC_FGCOLOR(vis));
	p->bg = (T_Long)(LIBGGI_GC_BGCOLOR(vis));

	data = (T_Long *)(p->text);

	for (i = 0 ; i <= strlen(s); ++i) {	/* Copy the trailing \0! */
		data[i] = (T_Long)(s[i]);
	}

	err = tclient_write(priv->client, &ev);

	if (err == TELE_ERROR_SHUTDOWN) {
		TELE_HANDLE_SHUTDOWN;
	}	/* if */

	return err;
}	/* GGI_tele_puts */
