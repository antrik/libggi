/* $Id: internal.c,v 1.7 2002/05/24 21:46:44 skids Exp $
******************************************************************************

   Misc internal-only functions

   Copyright (C) 1998-1999  Andrew Apted  [andrew@ggi-project.org]

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

#include <ggi/internal/internal.h>

int _ggi_countbits(uint32 val)
{
	int cnt;
	for (cnt = 0; val != 0; val >>= 1) {
		if (val & 1) cnt++;
	}
	return cnt;
}

int _ggi_mask2shift(uint32 mask)
{
	int shift;
	for (shift = 32; mask != 0; mask >>= 1) {
		shift--;
	}
	if (shift == 32) shift = 0;

	return shift;
}

void _ggi_build_pixfmt(ggi_pixelformat *pixfmt)
{
	int i, j = 0, bmsub = 0, oldbmsub = 0, bmtype = 0, oldbmtype = 0;
	int revendian;

	revendian = (pixfmt->flags & GGI_PF_REVERSE_ENDIAN);

	for (i = 0; i < pixfmt->depth; i++) {
		int bitmask = (1<<i);
		int colsize = 0, h;

		if (pixfmt->clut_mask & bitmask) {
			bmsub = GGI_BM_SUB_CLUT;
			bmtype = GGI_BM_TYPE_COLOR;
			for (colsize = 0, h = i; (pixfmt->clut_mask & (1<<h));
			     h++, colsize++) ;
		} else if (pixfmt->red_mask & bitmask) {
			bmsub = GGI_BM_SUB_RED;
			bmtype = GGI_BM_TYPE_COLOR;
			for (colsize = 0, h = i; (pixfmt->red_mask & (1<<h));
			     h++, colsize++) ;
		} else if (pixfmt->green_mask & bitmask) {
			bmsub = GGI_BM_SUB_GREEN;
			bmtype = GGI_BM_TYPE_COLOR;
			for (colsize = 0, h = i; (pixfmt->green_mask & (1<<h));
			     h++, colsize++) ;
		} else if (pixfmt->blue_mask & bitmask) {
			bmsub = GGI_BM_SUB_BLUE;
			bmtype = GGI_BM_TYPE_COLOR;
			for (colsize = 0, h = i; (pixfmt->blue_mask & (1<<h));
			     h++, colsize++) ;
		} else if (pixfmt->alpha_mask & bitmask) {
			bmsub = GGI_BM_SUB_ALPHA;
			bmtype = GGI_BM_TYPE_ATTRIB;
			for (colsize = 0, h = i; (pixfmt->alpha_mask & (1<<h));
			     h++, colsize++) ;
		} else if (pixfmt->fg_mask & bitmask) {
			bmsub = GGI_BM_SUB_FGCOL;
			bmtype = GGI_BM_TYPE_ATTRIB;
			for (colsize = 0, h = i; (pixfmt->fg_mask & (1<<h));
			     h++, colsize++) ;
		} else if (pixfmt->bg_mask & bitmask) {
			bmsub = GGI_BM_SUB_BGCOL;
			bmtype = GGI_BM_TYPE_ATTRIB;
			for (colsize = 0, h = i; (pixfmt->bg_mask & (1<<h));
			     h++, colsize++) ;
		} else if (pixfmt->texture_mask & bitmask) {
			bmsub = GGI_BM_SUB_TEXNUM;
			bmtype = GGI_BM_TYPE_ATTRIB;
			for (colsize = 0, h = i;
			     (pixfmt->texture_mask & (1<<h)); h++, colsize++) ;
		} else {
			bmsub = 0;
		}
		if (bmsub != oldbmsub || bmtype != oldbmtype) {
			j = 256-colsize;
			oldbmsub = bmsub;
			oldbmtype = bmtype;
		}
		if (bmsub) {
			pixfmt->bitmeaning[i]
				= bmtype | bmsub | j;
			j++;
		}
	}
	pixfmt->red_shift     = _ggi_mask2shift(pixfmt->red_mask);
	pixfmt->green_shift   = _ggi_mask2shift(pixfmt->green_mask);
	pixfmt->blue_shift    = _ggi_mask2shift(pixfmt->blue_mask);
	pixfmt->alpha_shift   = _ggi_mask2shift(pixfmt->alpha_mask);
	pixfmt->clut_shift    = _ggi_mask2shift(pixfmt->clut_mask);
	pixfmt->fg_shift      = _ggi_mask2shift(pixfmt->fg_mask);
	pixfmt->bg_shift      = _ggi_mask2shift(pixfmt->bg_mask);
	pixfmt->texture_shift = _ggi_mask2shift(pixfmt->texture_mask);

	if ((pixfmt->flags && pixfmt->flags != GGI_PF_REVERSE_ENDIAN) ||
	    (pixfmt->alpha_mask|pixfmt->fg_mask|pixfmt->bg_mask|
	     pixfmt->texture_mask)) {
		/* Not a standard format */
		return;
	}
	switch (pixfmt->size) {
	case 32:
		if (pixfmt->clut_mask) break;
		if (pixfmt->red_mask	== 0x00ff0000 &&
		    pixfmt->green_mask	== 0x0000ff00 &&
		    pixfmt->blue_mask	== 0x000000ff) {
			pixfmt->stdformat = GGI_DB_STD_24a32u8r8g8b8;
			break;
		}
		if (pixfmt->red_mask	== 0x0000ff00 &&
		    pixfmt->green_mask	== 0x00ff0000 &&
		    pixfmt->blue_mask	== 0xff000000) {
			pixfmt->stdformat = GGI_DB_STD_24a32b8g8r8u8;
			break;
		}
		if (pixfmt->red_mask	== 0xff000000 &&
		    pixfmt->green_mask	== 0x00ff0000 &&
		    pixfmt->blue_mask	== 0x0000ff00) {
			pixfmt->stdformat = GGI_DB_STD_24a32r8g8b8u8;
			break;
		}
		if (pixfmt->red_mask	== 0x000000ff &&
		    pixfmt->green_mask	== 0x0000ff00 &&
		    pixfmt->blue_mask	== 0x00ff0000) {
			pixfmt->stdformat = GGI_DB_STD_24a32u8b8g8r8;
			break;
		}
		break;
	case 24:
		if (pixfmt->clut_mask) break;
		if (pixfmt->red_mask	== 0xff0000 &&
		    pixfmt->green_mask	== 0x00ff00 &&
		    pixfmt->blue_mask	== 0x0000ff) {
			pixfmt->stdformat = GGI_DB_STD_24a24r8g8b8;
			break;
		}
		if (pixfmt->red_mask	== 0x0000ff &&
		    pixfmt->green_mask	== 0x00ff00 &&
		    pixfmt->blue_mask	== 0xff0000) {
			pixfmt->stdformat = GGI_DB_STD_24a24b8g8r8;
			break;
		}
		break;
	case 16:
		if (pixfmt->clut_mask) break;
		if (pixfmt->red_mask	== 0xf800 &&
		    pixfmt->green_mask	== 0x07e0 &&
		    pixfmt->blue_mask	== 0x001f) {
			if (revendian) {
				pixfmt->stdformat = GGI_DB_STD_16a16r5g6b5rev;
			} else {
				pixfmt->stdformat = GGI_DB_STD_16a16r5g6b5;
			}
			break;
		}
		if (pixfmt->red_mask	== 0x001f &&
		    pixfmt->green_mask	== 0x07e0 &&
		    pixfmt->blue_mask	== 0xf800) {
			if (revendian) {
				pixfmt->stdformat = GGI_DB_STD_16a16b5g6r5rev;
			} else {
				pixfmt->stdformat = GGI_DB_STD_16a16b5g6r5;
			}
			break;
		}
		if (pixfmt->red_mask	== 0x7c00 &&
		    pixfmt->green_mask	== 0x03f0 &&
		    pixfmt->blue_mask	== 0x001f) {
			if (revendian) {
				pixfmt->stdformat
					= GGI_DB_STD_15a16u1r5g5b5rev;
			} else {
				pixfmt->stdformat
					= GGI_DB_STD_15a16u1r5g5b5;
			}
			break;
		}
		if (pixfmt->red_mask	== 0x001f &&
		    pixfmt->green_mask	== 0x03f0 &&
		    pixfmt->blue_mask	== 0x7c00) {
			if (revendian) {
				pixfmt->stdformat
					= GGI_DB_STD_15a16u1b5g5r5rev;
			} else {
				pixfmt->stdformat
					= GGI_DB_STD_15a16u1b5g5r5;
			}
			break;
		}
		break;
	case 8:
		if ((pixfmt->red_mask|pixfmt->green_mask|pixfmt->blue_mask)) {
			break;
		}
		if (pixfmt->clut_mask == 0xff) {
			pixfmt->stdformat = GGI_DB_STD_8a8i8;
			break;
		}
		break;
	}
}

/* Generate a string representing the pixelformat e.g. r5g6b5.
   This can be used for dl loading or for passing pixfmt through 
   stringified/user-accessible interfaces.   Note these strings
   don't use the same format as GGI_DB_STD_*, though it would be
   nice if we could change GGI_DB_STD_* and depricate old values. */
/* TODO: flesh this out                                       */
void _ggi_pixfmtstr (ggi_visual *vis, char* str, int flags)
{
	if (flags & 1) {
		char alpha_or_pad, *ptr;
		ggi_pixelformat *pixfmt;
		int idx;

		pixfmt = LIBGGI_PIXFMT(vis);
		alpha_or_pad = (flags & 2) ? 'a' : 'p';
		idx = pixfmt->depth - 1;
		if (idx > 31) return; /* paranoia never hurts. */
		ptr = str;

		while (1) {
			switch(pixfmt->bitmeaning[idx] & 0x00ffff00) {
			      case GGI_BM_TYPE_COLOR | GGI_BM_SUB_RED:
				*(ptr++) = 'r';
				break;
			      case GGI_BM_TYPE_COLOR | GGI_BM_SUB_GREEN:
				*(ptr++) = 'g';
				break;
			      case GGI_BM_TYPE_COLOR | GGI_BM_SUB_BLUE:
				*(ptr++) = 'b';
				break;
			      case GGI_BM_TYPE_ATTRIB | GGI_BM_SUB_ALPHA:
				*(ptr++) = alpha_or_pad;
				break;
			      default:
				*(ptr++) = 'p';
				break;
			}
			while ((pixfmt->bitmeaning[idx] & 0x00ffff00) ==
			       (pixfmt->bitmeaning[idx - 1] & 0x00ffff00)) {
			  if (idx == 0) break;
			  idx--;
			}
			ptr += sprintf(ptr, "%d", 
				       256-(pixfmt->bitmeaning[idx] & 0xff)
				       );
			idx--;
			if (idx < 0) break;

		}
		*ptr = '\0';
	}
	else sprintf(str, "%d", GT_SIZE(LIBGGI_GT(vis)));
}

int _ggi_match_palette(ggi_color *pal, int pal_len, ggi_color *col)
{
	int i, closest=0;
	int r = col->r, g = col->g, b = col->b;

	uint32 closest_dist = (1 << 31);

	for (i=0; i < pal_len; i++) {
#undef ABS
#define ABS(val)	((val) < 0 ? -(val) : val)
		int dist =
			ABS(r - pal[i].r) +
			ABS(g - pal[i].g) +
			ABS(b - pal[i].b);
#undef ABS

		if (dist < closest_dist) {
			closest = i;
			if (dist == 0) {
				/* Exact match */
				break;
			}
			closest_dist = dist;
		}
	}

	GGIDPRINT_COLOR("match-color: %02x%02x%02x -> %02x%02x%02x (%d).\n",
		     col->r >> 8, col->g >> 8, col->b >> 8,
		     pal[closest].r >> 8, pal[closest].g >> 8,
		     pal[closest].b >> 8, closest);

	return closest;
}

int _ggi_default_setreadframe(ggi_visual *vis, int num)
{
        ggi_directbuffer *db = _ggi_db_find_frame(vis, num);

        if (db == NULL) {
                return -1;
        }

        vis->r_frame_num = num;
        vis->r_frame = db;

        return 0;
}

int _ggi_default_setwriteframe(ggi_visual *vis, int num)
{
        ggi_directbuffer *db = _ggi_db_find_frame(vis, num);

        if (db == NULL) {
                return -1;
        }

        vis->w_frame_num = num;
        vis->w_frame = db;

        return 0;
}

void _ggi_smart_match_palettes(ggi_color *pal, int size,
                               ggi_color *ref_pal, int ref_size)
{
	/* This function moves the color entries in the palette `pal'
	 * around so to produce the best match against the reference
	 * palette.  Lower indexed entries in the reference palette have
	 * higher priority than higher indexed entries, so they are
	 * matched first.  The reference palette can be smaller *or*
	 * larger than the one to be re-arranged.
	 */
	
	int i;
	int smallest = (size < ref_size) ? size : ref_size;

	for (i=0; i < smallest; i++) {

		ggi_color tmp;

		int n = i + _ggi_match_palette(pal + i, size - i, 
		                               ref_pal + i);

		tmp = pal[i]; pal[i] = pal[n]; pal[n] = tmp;
	}
}

void _ggi_build_palette(ggi_color *pal, int num)
{
	ggi_color black  = { 0x0000, 0x0000, 0x0000 };
	ggi_color white  = { 0xffff, 0xffff, 0xffff };
	ggi_color blue   = { 0x0000, 0x0000, 0xffff };
	ggi_color yellow = { 0xffff, 0xffff, 0x0000 };

	int i, depth, N;

	int rnum,  gnum,  bnum;
	int rmask, gmask, bmask;
	
	/* handle small palettes */

	if (num == 0) return;

	pal[0] = black;   if (num == 1) return;
	pal[1] = white;   if (num == 2) return;
	pal[2] = blue;    if (num == 3) return;
	pal[3] = yellow;  if (num == 4) return;

	/* handle large palettes. */

	depth = 0;  /* work out depth so that (1 << depth) >= num */

	for (N = num-1; N > 0; N /= 2) {
		depth++;
	}

	gnum = (depth + 2) / 3;  /* green has highest priority */
	rnum = (depth + 1) / 3;  /* red has second highest priority */
	bnum = (depth + 0) / 3;  /* blue has lowest priority */

	gmask = (1 << gnum) - 1;
	rmask = (1 << rnum) - 1;
	bmask = (1 << bnum) - 1;

	for (i=0; i < num; i++) {

		/* When num < (1 << depth), we interpolate the values so
		 * that we still get a good range.  There's probably a
		 * better way...
		 */
		
		int j = i * ((1 << depth) - 1) / (num - 1);
		int r, g, b;
		
		b = j & bmask;  j >>= bnum;
		r = j & rmask;  j >>= rnum;
		g = j & gmask;

		pal[i].r = r * 0xffff / rmask;
		pal[i].g = g * 0xffff / gmask;
		pal[i].b = b * 0xffff / bmask;
	}
}


int _ggi_parse_physz(char *optstr, int *physzflag, ggi_coord *physz) {

	/* This function parses a string gotten through the -physz= option,
	 * contained in optstr, and fills out the values physzflag and physz 
	 * based on what is in that string.  The latter two are stored in
	 * the visual's target private area (not all targets use the -physz
	 * option.)
         *
	 * physz gets the integer values in the option string. physzflag can 
	 * contain two flags, one (GGI_PHYSZ_OVERRIDE) designating that the 
	 * values are not defaults, rather ovverrides for a misconfigured 
	 * display.  The second, GGI_PHYSZ_DPI designates that the sizes
	 * in the string are in dots-per-inch, otherwise the sizes are
	 * assumed to be the full size of the display  in millimeters. 
	 * (which may not be the same as the size of the visual, on targets 
	 * where the visual is a subregion of a display system such as X).
	 */

	char *nptr, *endptr;
	nptr = optstr;

	*physzflag = 0;
	physz->x =physz->y = GGI_AUTO;
	
	/* The 'N' is there by default, if the option was not filled in. */
	if (*nptr == 'N' || *nptr == 'n') return GGI_OK;

	/* Check if we should *always* override the X server values */
	if(*nptr == '=') {
		nptr++;
		*physzflag |= GGI_PHYSZ_OVERRIDE;
	}

	physz->x = strtoul(nptr, &endptr, 0);

	if (*nptr == '\0' || *endptr != ',') {
		*physzflag = 0;
		physz->x = physz->y = GGI_AUTO;
		return GGI_EARGINVAL;
	}

	nptr = endptr + 1;

	physz->y = strtoul(nptr, &endptr, 0);

	if (*nptr != '\0' && 
	    (*endptr == 'd' || *endptr == 'D') && 
	    (*(endptr + 1) == 'p' || *(endptr + 1) == 'P') && 
	    (*(endptr + 2) == 'i' || *(endptr + 2) == 'I'))  {
		endptr += 3;
		*physzflag |= GGI_PHYSZ_DPI;
	}

	if (*nptr == '\0' || *endptr != '\0') {
		*physzflag = 0;
		physz->x =physz->y = GGI_AUTO;
		return GGI_EARGINVAL;
	}

	return GGI_OK;
}


int _ggi_figure_physz(ggi_mode *mode, int physzflag, ggi_coord *op_sz, 
		      int dpix, int dpiy, int dsx, int dsy) {

	/* This function validates/suggests values in mode->size to
	 * designate the physical screen size in millimeters.
	 *
	 * mode->visible and mode->dpp are assumed to already contain 
	 * valid values.
	 *
	 * The physflag and op_sz parameters are from the visual's 
	 * target private area, as set by the above _ggi_parse_physz function.
	 *
	 * The dpix, dpiy parameters contain the dpi of the display.
	 *
	 * The dsx, dsy parameters contain the size in pixels of the
	 * entire display, which on visuals using a subregion of 
	 * a display system, such as X, is the size of the entire screen.
	 */

	long xsize, ysize;
	int err = GGI_OK;

	xsize = ysize = 0;

	if (physzflag & GGI_PHYSZ_DPI) {
		xsize = (physzflag & GGI_PHYSZ_OVERRIDE) ? op_sz->x : dpix;
		ysize = (physzflag & GGI_PHYSZ_OVERRIDE) ? op_sz->y : dpiy;
		if (xsize <= 0 || ysize <= 0) {
			xsize = op_sz->x;
			ysize = op_sz->y;
		}
		if (xsize <= 0 || ysize <= 0) goto nosize;
		/* find absolute size in mm */
		xsize = mode->visible.x * mode->dpp.x * 254 / xsize / 10;
		ysize = mode->visible.y * mode->dpp.y * 254 / ysize / 10;
	} else {
		if (physzflag & GGI_PHYSZ_OVERRIDE) {
			xsize = op_sz->x;
			ysize = op_sz->y;
		} 
		else if (dpix > 0 && dpiy > 0) {
			xsize = (dsx * mode->dpp.x * 254 / dpix / 10);
			ysize = (dsy * mode->dpp.y * 254 / dpiy / 10);
		}
		if (xsize <= 0 || ysize <= 0) {
			xsize = op_sz->x;
			ysize = op_sz->y;
		}
		if (xsize <= 0 || ysize <= 0) goto nosize;
		if (dsx <= 0 || dsy <= 0) goto nosize;
		xsize = xsize * mode->visible.x / dsx;
		ysize = ysize * mode->visible.y / dsy;
	}

	if ((mode->size.x != xsize && mode->size.x != GGI_AUTO) ||
	    (mode->size.y != ysize && mode->size.y != GGI_AUTO)) {
		err = GGI_ENOMATCH;
	}

	mode->size.x = (int)xsize;
	mode->size.y = (int)ysize;

	return err;

 nosize:
	if ((mode->size.x != GGI_AUTO) || (mode->size.y != GGI_AUTO)) 
		err = GGI_ENOMATCH;
	return err;
	
} 
