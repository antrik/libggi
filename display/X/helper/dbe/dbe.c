/* $Id: dbe.c,v 1.5 2004/09/18 14:42:40 cegger Exp $
******************************************************************************

   DBE extension support for display-x

   Copyright (C) 2002      Brian S. Julin	[bri@tull.umassp.edu]

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
#include <ggi/display/x.h>
#include <X11/extensions/Xdbe.h>

static int GGIopen(ggi_visual *vis, struct ggi_dlhandle *dlh,
		   const char *args, void *argptr, uint32 *dlret)
{
	ggi_x_priv *priv = GGIX_PRIV(vis);
	int major_version, minor_version;
	Status rc;

	/* Check if DBE is present before initialising */
	rc = XdbeQueryExtension(priv->disp, &major_version, &minor_version);
	if (rc != True) return GGI_ENOFUNC;

	GGIDPRINT_LIBS("X: DOUBLE-BUFFER: DBE version %i.%i\n",
			major_version, minor_version);


	*dlret = 0;
	return GGI_OK;
}

static int GGIclose(ggi_visual *vis, struct ggi_dlhandle *dlh)
{
	return GGI_OK;
}


EXPORTFUNC
int GGIdl_helper_x_dbe(int func, void **funcptr);

int GGIdl_helper_x_dbe(int func, void **funcptr)
{
	switch (func) {
	case GGIFUNC_open:
		*funcptr = GGIopen;
		return 0;
	case GGIFUNC_exit:
		*funcptr = NULL;
		return 0;
	case GGIFUNC_close:
		*funcptr = GGIclose;
		return 0;
	default:
		*funcptr = NULL;
	}

	return GGI_ENOTFOUND;
}

#include <ggi/internal/ggidlinit.h>
