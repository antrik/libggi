/* $Id: unit.h,v 1.1 2004/02/14 13:45:40 cegger Exp $
******************************************************************************

   LibGGI core - conversion between units

   Copyright (C) 2004 Christoph Egger	[Christoph_Egger@]

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


static inline void _ggi_unit_dpi2mm(ggi_coord *mm, const ggi_coord *dpi)
{
	mm->x = dpi->x * 254 / 10;
	mm->y = dpi->y * 254 / 10;

	return;
}


static inline void _ggi_unit_mm2dpi(ggi_coord *dpi, const ggi_coord *mm)
{
	dpi->x = mm->x * 10 / 254;
	dpi->y = mm->y * 10 / 254;

	return;
}


static inline void _ggi_unit_dpi2pix(ggi_coord *pix, const ggi_coord *dpi,
			const ggi_coord *dpp)
{
	LIBGGI_ASSERT(dpp->x != 0, "Division by zero");
	LIBGGI_ASSERT(dpp->y != 0, "Division by zero");

	pix->x = dpi->x / dpp->x;
	pix->y = dpi->y / dpp->y;

	return;
}


static inline void _ggi_unit_pix2dpi(ggi_coord *dpi, const ggi_coord *pix,
			const ggi_coord *dpp)
{
	LIBGGI_ASSERT(dpp->x != 0, "Division by zero");
	LIBGGI_ASSERT(dpp->y != 0, "Division by zero");

	dpi->x = pix->x / dpp->x;
	dpi->y = pix->y / dpp->y;

	return;
}


static inline void _ggi_unit_mm2pix(ggi_coord *pix, const ggi_coord *mm,
			const ggi_coord *dpp)
{
	ggi_coord dpi;

	LIBGGI_ASSERT(dpp != NULL, "Invalid argument");
	LIBGGI_ASSERT(dpp != NULL, "Invalid argument");
	LIBGGI_ASSERT(mm != NULL, "Invalid argument");
	LIBGGI_ASSERT(mm != NULL, "Invalid argument");
	LIBGGI_ASSERT(pix != NULL, "Invalid argument");
	LIBGGI_ASSERT(pix != NULL, "Invalid argument");

	_ggi_unit_mm2dpi(&dpi, mm);
	_ggi_unit_dpi2pix(pix, &dpi, dpp);

	return;
}


static inline void _ggi_unit_pix2mm(ggi_coord *mm, const ggi_coord *pix,
			const ggi_coord *dpp)
{
	ggi_coord dpi;

	LIBGGI_ASSERT(dpp->x != 0, "Division by zero");
	LIBGGI_ASSERT(dpp->y != 0, "Division by zero");

	_ggi_unit_pix2dpi(&dpi, pix, dpp);
	_ggi_unit_dpi2mm(mm, &dpi);

	return;
}
