/* $Id: ext.c,v 1.10 2007/03/01 11:28:41 cegger Exp $
******************************************************************************

   LibGGI extension support.

   Copyright (C) 1997 Jason McMullan		[jmcc@ggi-project.org]
   Copyright (C) 1998-2000 Marcus Sundberg	[marcus@ggi-project.org]
   Copyright (C) 2005 Christoph Egger

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
#include <ggi/internal/internal.h>
#include <ggi/internal/ggi_debug.h>
#include <ggi/gg.h>

#include "ext.h"
#include <string.h>


int ggiIndicateChange(ggi_visual_t v, int whatchanged)
{
	struct ggi_visual *vis = GGI_VISUAL(v);

	DPRINT_CORE("ggiIndicateChange(%p, 0x%x) called\n",
			vis, whatchanged);

	/* Tell all attached extensions on this visual */
	DPRINT_CORE("ggiIndicateChange: %i changed for %p.\n",
			whatchanged, vis);

	ggBroadcast(libggi->channel, whatchanged, vis);

	return 0;
}
