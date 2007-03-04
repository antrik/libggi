/* $Id: swar.c,v 1.2 2007/03/04 14:44:53 soyt Exp $
******************************************************************************

   LibGGI SWAR support.

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
#include <ggi/internal/ggi.h>
#include <ggi/internal/ggi_debug.h>
#include <ggi/gg.h>

#include "swar.h"


static gg_swartype     swars_selected    = 0;


gg_swartype _ggiGetSwarType(void)
{
	return (swars_selected & ggGetSwarType());
}


int _ggiSwarInit(void)
{
#ifdef DO_SWAR_NONE
	swars_selected |= GG_SWAR_NONE;
#endif
#ifdef DO_SWAR_32BITC
	swars_selected |= GG_SWAR_32BITC;
#endif
#ifdef DO_SWAR_64BITC
	swars_selected |= GG_SWAR_64BITC;
#endif
#ifdef DO_SWAR_ALTIVEC
	swars_selected |= GG_SWAR_ALTIVEC;
#endif
#ifdef DO_SWAR_SSE2
	swars_selected |= GG_SWAR_SSE2;
#endif
#ifdef DO_SWAR_SSE
	swars_selected |= GG_SWAR_SSE;
#endif
#ifdef DO_SWAR_MMX
	swars_selected |= GG_SWAR_MMX;
#endif
#ifdef DO_SWAR_MMXPLUS
	swars_selected |= GG_SWAR_MMXPLUS;
#endif
#ifdef DO_SWAR_3DNOW
	swars_selected |= GG_SWAR_3DNOW;
#endif
#ifdef DO_SWAR_ADV3DNOW
	swars_selected |= GG_SWAR_ADV3DNOW;
#endif
#ifdef DO_SWAR_MVI
	swars_selected |= GG_SWAR_MVI;
#endif
#ifdef DO_SWAR_MAX
	swars_selected |= GG_SWAR_MAX;
#endif
#ifdef DO_SWAR_MAX2
	swars_selected |= GG_SWAR_MAX2;
#endif
#ifdef DO_SWAR_SIGD
	swars_selected |= GG_SWAR_SIGD;
#endif
#ifdef DO_SWAR_MDMX
	swars_selected |= GG_SWAR_MDMX;
#endif
#ifdef DO_SWAR_VIS
	swars_selected |= GG_SWAR_VIS;
#endif
#ifdef DO_SWAR_MAJC
	swars_selected |= GG_SWAR_MAJC;
#endif
	/* TODO: disable SWARs with environment variable */

	if (!swars_selected) {
		fprintf(stderr, 
			"LibGGI: No SWARs selected.  Need at least one.\n");
		return GGI_ENOFUNC;
	}

	return GGI_OK;
}
