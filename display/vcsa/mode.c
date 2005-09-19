/* $Id: mode.c,v 1.11 2005/09/19 18:46:44 cegger Exp $
******************************************************************************

   Display-VCSA: mode management

   Copyright (C) 1998 Andrew Apted	[andrew@ggi-project.org]
   Copyright (C) 2000 Marcus Sundberg	[marcus@ggi-project.org]

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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

#include <sys/types.h>
#include <sys/stat.h>

#include "config.h"
#include <ggi/display/vcsa.h>
#include <ggi/internal/ggi_debug.h>

#include "../common/ggi-auto.inc"


int GGI_vcsa_getapi(ggi_visual *vis, int num, char *apiname, char *arguments)
{
	*arguments = '\0';

	switch (num) {
		case 0: strcpy(apiname, "display-vcsa");
			return 0;

		case 1: strcpy(apiname, "generic-stubs");
			return 0;
	}

	return GGI_ENOMATCH;
}

int GGI_vcsa_setmode(ggi_visual *vis, ggi_mode *mode)
{ 
	char libname[GGI_MAX_APILEN], libargs[GGI_MAX_APILEN];
	int err, id;

        if ((err = ggiCheckMode(vis, mode)) != 0) {
		return err;
	}

	DPRINT_MODE("display-vcsa: setmode %dx%d#%dx%dF%d[0x%02x]\n",
			mode->visible.x, mode->visible.y,
			mode->virt.x, mode->virt.y, 
			mode->frames, mode->graphtype);

	memcpy(LIBGGI_MODE(vis), mode, sizeof(ggi_mode));

	_ggiZapMode(vis, 0);

	/* load API libraries */
	for (id=1; GGI_vcsa_getapi(vis, id, libname, libargs) == 0; id++) {
		err = _ggiOpenDL(vis, _ggiGetConfigHandle(), libname, libargs, NULL);
		if (err) {
			fprintf(stderr,"display-vcsa: Error opening the "
				"%s (%s) library.\n", libname, libargs);
			return GGI_EFATAL;
		}

		DPRINT_LIBS("Success in loading %s (%s)\n", libname, libargs);
	}

	/* setup drawing primitives */
	vis->opdraw->putpixel_nc  = GGI_vcsa_putpixel_nc;
	vis->opdraw->getpixel     = GGI_vcsa_getpixel;
	vis->opdraw->putc         = GGI_vcsa_putc;
	vis->opdraw->puts         = GGI_vcsa_puts;
	vis->opdraw->getcharsize  = GGI_vcsa_getcharsize;
	vis->opdraw->drawhline_nc = GGI_vcsa_drawhline_nc;
	vis->opdraw->puthline     = GGI_vcsa_puthline;
	vis->opdraw->gethline     = GGI_vcsa_gethline;

	vis->opcolor->mapcolor    = GGI_vcsa_mapcolor;
	vis->opcolor->unmappixel  = GGI_vcsa_unmappixel;

	/* indicate API change */
	ggiIndicateChange(vis, GGI_CHG_APILIST);

	DPRINT_MODE("display-vcsa: setmode Success.\n");

	return 0;
}

int GGI_vcsa_resetmode(ggi_visual *vis)
{
	/* Nothing to do .. */

	return 0;
}
	
int GGI_vcsa_checkmode(ggi_visual *vis, ggi_mode *mode)
{
	ggi_vcsa_priv *priv = VCSA_PRIV(vis);
	int err;

	DPRINT_MODE("display-vcsa: checkmode %dx%d#%dx%dF%d[0x%02x]\n",
			mode->visible.x, mode->visible.y,
			mode->virt.x, mode->virt.y, 
			mode->frames, mode->graphtype);

	/* handle GT_AUTO in graphtype */
	if (GT_SCHEME(mode->graphtype) == GT_AUTO) {
		GT_SETSCHEME(mode->graphtype, GT_TEXT);
	}
	if (GT_DEPTH(mode->graphtype) == GT_AUTO) {
		GT_SETDEPTH(mode->graphtype, 4);
	}
	if (GT_SIZE(mode->graphtype) == GT_AUTO) {
		GT_SETSIZE(mode->graphtype, 16);
	}

	/* handle GGI_AUTO in ggi_mode */
	if ((mode->dpp.x != 1 && mode->dpp.x != GGI_AUTO) ||
	    (mode->dpp.y != 1 && mode->dpp.y != GGI_AUTO)) {
		err = -1;
	}
	mode->dpp.x = mode->dpp.y = 1;

	_GGIhandle_ggiauto(mode, priv->width, priv->height);

	/* now check stuff */
	err = 0;

	if (GT_SCHEME(mode->graphtype) != GT_TEXT) {
		GT_SETSCHEME(mode->graphtype, GT_TEXT);
		err = -1;
	}
	if (GT_DEPTH(mode->graphtype) != 4) {
		GT_SETDEPTH(mode->graphtype, 4);
		err = -1;
	}
	if (GT_SIZE(mode->graphtype) != 16) {
		GT_SETSIZE(mode->graphtype, 16);
		err = -1;
	}

	if (mode->visible.x != priv->width) {
		mode->visible.x = priv->width;
		err = -1;
	}
	if (mode->visible.y != priv->height) {
		mode->visible.y = priv->height;
		err = -1;
	}
	if (mode->virt.x != priv->width) {
		mode->virt.x = priv->width;
		err = -1;
	}
	if (mode->virt.y != priv->height) {
		mode->virt.y = priv->height;
		err = -1;
	}

	if (mode->frames != 1) {
		mode->frames = 1;
		err = -1;
	}

	err = _ggi_physz_figure_size(mode, priv->physzflags, &(priv->physz),
				0, 0, mode->visible.x, mode->visible.y);
		
	DPRINT_MODE("display-vcsa: result %d %dx%d#%dx%dF%d[0x%02x]\n",
			err, mode->visible.x, mode->visible.y,
			mode->virt.x, mode->virt.y, 
			mode->frames, mode->graphtype);
	return err;
}

int GGI_vcsa_getmode(ggi_visual *vis, ggi_mode *mode)
{
	DPRINT_MODE("display-vcsa: getmode\n");
	
	memcpy(mode, LIBGGI_MODE(vis), sizeof(ggi_mode));

	return 0;
}

int GGI_vcsa_setflags(ggi_visual *vis, ggi_flags flags)
{
	LIBGGI_FLAGS(vis) = flags;
	LIBGGI_FLAGS(vis) &= GGIFLAG_ASYNC; /* Unkown flags don't take. */

	return 0;
}
