/* $Id: listener.c,v 1.2 2006/10/14 15:14:02 cegger Exp $
******************************************************************************

   LIBGGI: listener for display-vcsa

   Copyright (C) 2006 Christoph Egger

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

#include <string.h>

#include "config.h"
#include <ggi/display/vcsa.h>
#include <ggi/internal/ggi_debug.h>

int GGI_vcsa_kbd_listener(void *arg, uint32_t flag, void *data)
{
#if 0
	struct ggi_visual *vis = arg;
	ggi_vcsa_priv *priv = VCSA_PRIV(vis);
#endif

	DPRINT_MISC("GGI_vcsa_kbd_listener: received event, flag 0x%X\n",
			flag);
	return 0;
}

int GGI_vcsa_ms_listener(void *arg, uint32_t flag, void *data)
{

	DPRINT_MISC("GGI_vcsa_ms_listener: received event, flag 0x%X\n",
			flag);
	return 0;
}

