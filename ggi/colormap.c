/* $Id: colormap.c,v 1.4 2004/10/31 13:15:00 cegger Exp $
******************************************************************************

   LibGGI core - target independent colormap implementation

   Copyright (C) 2003 Christoph Egger	[Christoph_Egger@t-online.de]

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


size_t _ggiColormapGetPrivsize(ggi_visual_t vis)
{
	return LIBGGI_PAL(vis)->getPrivSize(vis);
}	/* _ggiColormapGetPrivsize */


int _ggiColormapSetRW(ggi_visual_t vis, size_t start, size_t end)
{
	size_t ro_start, ro_stop;
	ggi_colormap *map = LIBGGI_PAL(vis);

	if (start > end) return GGI_EARGINVAL;
	if (end < start) return GGI_EARGINVAL;
	if (end >= map->clut.size) return GGI_EARGINVAL;

	_ggiColormapGetRO(vis, &ro_start, &ro_stop);

	if (ro_stop > 0) {
		/* there's an RO range, RW range is
		 * not allowed to overlap
		 */
		if (!(start > ro_stop)) return GGI_EARGINVAL;
		if ((ro_start > 0) && (!(end < ro_start)))
			return GGI_EARGINVAL;
	}	/* if */

	return map->setRW(vis, start, end);
}	/* _ggiColormapSetRW */


int _ggiColormapGetRW(ggi_visual_t vis, size_t *start, size_t *end)
{
	ggi_colormap *map = LIBGGI_PAL(vis);

	LIBGGI_ASSERT(start != NULL, "NULL pointer bug!");
	LIBGGI_ASSERT(end != NULL, "NULL pointer bug!");
	LIBGGI_ASSERT(map->getRW != _ggiColormapGetRW, "forever loop bug detected!");

	return map->getRW(vis, start, end);
}	/* _ggiColormapGetRW */


int _ggiColormapSetRO(ggi_visual_t vis, size_t start, size_t end)
{
	size_t rw_start, rw_stop;
	ggi_colormap *map = LIBGGI_PAL(vis);

	if (start > end) return GGI_EARGINVAL;
	if (end < start) return GGI_EARGINVAL;
	if (end >= map->clut.size) return GGI_EARGINVAL;

	_ggiColormapGetRW(vis, &rw_start, &rw_stop);

	if (rw_stop > 0) {
		/* there's an RW range, RO range is
		 * not allowed to overlap
		 */
		if (!(start > rw_stop)) return GGI_EARGINVAL;
		if ((rw_start > 0) && (!(end < rw_start)))
			return GGI_EARGINVAL;
	}	/* if */

	return map->setRW(vis, start, end);
}	/* _ggiColormapSetRO */


int _ggiColormapGetRO(ggi_visual_t vis, size_t *start, size_t *end)
{
	ggi_colormap *map = LIBGGI_PAL(vis);

	LIBGGI_ASSERT(start != NULL, "NULL pointer bug!");
	LIBGGI_ASSERT(end != NULL, "NULL pointer bug!");
	LIBGGI_ASSERT(map->getRO != _ggiColormapGetRO, "forever loop bug detected!");

	return map->getRO(vis, start, end);
}	/* _ggiColormapGetRO */



int _ggiColormapSetPalette(ggi_visual_t vis, size_t start,
				size_t size, const ggi_color *cmap)
{
	return LIBGGI_PAL(vis)->setPalette(vis, start, size, cmap);
}	/* _ggiColormapSetPalette */


ssize_t _ggiColormapFindByColor(ggi_visual_t vis, const ggi_color *color,
				enum ggi_colormap_region region)
{
	size_t idx;
	ssize_t rc;

	ggi_colormap *map = LIBGGI_PAL(vis);

	LIBGGI_ASSERT(color != NULL, "NULL pointer bug!");

	switch (region) {
	case GGI_COLORMAP_RW_REGION:
	case GGI_COLORMAP_RO_REGION:
	case GGI_COLORMAP_RW_RO_REGION:
		break;
	default:
		return GGI_EARGINVAL;
	}	/* switch */

	for (idx = 0; idx < map->clut.size; idx++) {
		rc = _ggiColormapMatchByColor(vis, &(map->clut.data[idx]),
					color, region);
		if (rc == GGI_OK) {
			return (ssize_t)(idx);
		}	/* if */
	}	/* for */

	return GGI_ENOTFOUND;
}	/* _ggiColormapFindByColor */


ssize_t _ggiColormapFindByIdx(ggi_visual_t vis, size_t idx,
				enum ggi_colormap_region region)
{
	size_t idx_tmp;
	ssize_t rc;
	size_t rw_start, rw_stop;
	size_t ro_start, ro_stop;

	ggi_colormap *map = LIBGGI_PAL(vis);

	if (idx >= map->clut.size) return GGI_EARGINVAL;

	switch (region) {
	case GGI_COLORMAP_RW_REGION:
		_ggiColormapGetRW(vis, &rw_start, &rw_stop);

		if (idx >= rw_stop) return GGI_EARGINVAL;
		if ((rw_start > 0) && (!(idx < rw_start))) {
			return GGI_EARGINVAL;
		}	/* if */
		break;
	case GGI_COLORMAP_RO_REGION:
		_ggiColormapGetRO(vis, &ro_start, &ro_stop);

		if (idx >= ro_stop) return GGI_EARGINVAL;
		if ((ro_start > 0) && (!(idx < ro_start))) {
			return GGI_EARGINVAL;
		}	/* if */
		break;

	case GGI_COLORMAP_RW_RO_REGION:
		break;
	default:
		return GGI_EARGINVAL;
	}	/* switch */


	for (idx_tmp = 0; idx_tmp < map->clut.size; idx_tmp++) {
		rc = _ggiColormapMatchByIdx(vis, idx,
					idx_tmp, region);
		if (rc == GGI_OK) {
			return idx_tmp;
		}	/* if */
	}	/* for */

	return GGI_ENOTFOUND;
}	/* _ggiColormapFindByIdx */



int _ggiColormapMatchByColor(ggi_visual_t vis, const ggi_color *color1,
				const ggi_color *color2,
				enum ggi_colormap_region region)
{
	LIBGGI_ASSERT(color1 != NULL, "NULL pointer bug!");
	LIBGGI_ASSERT(color2 != NULL, "NULL pointer bug!");

	return LIBGGI_PAL(vis)->matchByColor(vis, color1,
					color2, region);
}	/* _ggiColormapMatchByColor */



int _ggiColormapMatchByIdx(ggi_visual_t vis, size_t idx1, size_t idx2,
				enum ggi_colormap_region region)
{
	size_t rw_start, rw_stop;
	size_t ro_start, ro_stop;

	ggi_colormap *map = LIBGGI_PAL(vis);

	if (idx1 >= map->clut.size) return GGI_EARGINVAL;
	if (idx2 >= map->clut.size) return GGI_EARGINVAL;

	switch (region) {
	case GGI_COLORMAP_RW_REGION:
		_ggiColormapGetRW(vis, &rw_start, &rw_stop);

		if (idx1 >= rw_stop) return GGI_EARGINVAL;
		if ((rw_start > 0) && (!(idx1 < rw_start))) {
			return GGI_EARGINVAL;
		}	/* if */

		if (idx2 >= rw_stop) return GGI_EARGINVAL;
		if ((rw_start > 0) && (!(idx2 < rw_start))) {
			return GGI_EARGINVAL;
		}	/* if */
		break;

	case GGI_COLORMAP_RO_REGION:
		_ggiColormapGetRO(vis, &ro_start, &ro_stop);

		if (idx1 >= ro_stop) return GGI_EARGINVAL;
		if ((ro_start > 0) && (!(idx1 < ro_start))) {
			return GGI_EARGINVAL;
		}	/* if */

		if (idx2 >= ro_stop) return GGI_EARGINVAL;
		if ((ro_start > 0) && (!(idx2 < ro_start))) {
			return GGI_EARGINVAL;
		}	/* if */
		break;

	case GGI_COLORMAP_RW_RO_REGION:
		break;
	default:
		return GGI_EARGINVAL;
	}	/* switch */

	return map->matchByIdx(vis, idx1, idx2, region);
}	/* _ggiColormapMatchByIdx */
