/* $Id: internal.c,v 1.31 2007/03/04 14:44:53 soyt Exp $
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

#include "config.h"

#include <ggi/internal/ggi.h>
#include <ggi/internal/ggi_debug.h>
#include <ggi/internal/gg_replace.h>


int _ggi_countbits(uint32_t val)
{
	int cnt;
	for (cnt = 0; val != 0; val >>= 1) {
		if (val & 1) cnt++;
	}
	return cnt;
}

int _ggi_mask2shift(uint32_t mask)
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

	for (i = 0; i < pixfmt->size; i++) {
		ggi_pixel bitmask = (1<<i);
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
			pixfmt->stdformat = GGI_DB_STD_24a32p8r8g8b8;
			break;
		}
		if (pixfmt->red_mask	== 0x0000ff00 &&
		    pixfmt->green_mask	== 0x00ff0000 &&
		    pixfmt->blue_mask	== 0xff000000) {
			pixfmt->stdformat = GGI_DB_STD_24a32b8g8r8p8;
			break;
		}
		if (pixfmt->red_mask	== 0xff000000 &&
		    pixfmt->green_mask	== 0x00ff0000 &&
		    pixfmt->blue_mask	== 0x0000ff00) {
			pixfmt->stdformat = GGI_DB_STD_24a32r8g8b8p8;
			break;
		}
		if (pixfmt->red_mask	== 0x000000ff &&
		    pixfmt->green_mask	== 0x0000ff00 &&
		    pixfmt->blue_mask	== 0x00ff0000) {
			pixfmt->stdformat = GGI_DB_STD_24a32p8b8g8r8;
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
		    pixfmt->green_mask	== 0x03e0 &&
		    pixfmt->blue_mask	== 0x001f) {
			if (revendian) {
				pixfmt->stdformat
					= GGI_DB_STD_15a16p1r5g5b5rev;
			} else {
				pixfmt->stdformat
					= GGI_DB_STD_15a16p1r5g5b5;
			}
			break;
		}
		if (pixfmt->red_mask	== 0x001f &&
		    pixfmt->green_mask	== 0x03f0 &&
		    pixfmt->blue_mask	== 0x7c00) {
			if (revendian) {
				pixfmt->stdformat
					= GGI_DB_STD_15a16p1b5g5r5rev;
			} else {
				pixfmt->stdformat
					= GGI_DB_STD_15a16p1b5g5r5;
			}
			break;
		}
		break;
	case 8:
		if ((pixfmt->red_mask|pixfmt->green_mask|pixfmt->blue_mask)) {
			if (pixfmt->red_mask	== 0xe0 &&
			    pixfmt->green_mask	== 0x1c &&
			    pixfmt->blue_mask	== 0x03) {
				pixfmt->stdformat = GGI_DB_STD_8a8r3g3b2;
			}
			break;
		}
		if (pixfmt->clut_mask == 0xff) {
			pixfmt->stdformat = GGI_DB_STD_8a8i8;
			break;
		}
		break;
	}
}



int _ggi_parse_pixfmtstr(const char *pixfmtstr,
		char separator, const char **endptr,
		size_t pixfmtstr_len,
		ggi_pixel *r_mask, ggi_pixel *g_mask,
		ggi_pixel *b_mask, ggi_pixel *a_mask)
{
	ggi_pixel *curr = NULL;
	const char *ptr = pixfmtstr;
	unsigned long nbits;

	LIB_ASSERT(pixfmtstr_len > 0, "Invalid pixfmtstr_len");
	LIB_ASSERT(r_mask != NULL, "r_mask cannot be NULL");
	LIB_ASSERT(g_mask != NULL, "g_mask cannot be NULL");
	LIB_ASSERT(b_mask != NULL, "b_mask cannot be NULL");
	LIB_ASSERT(a_mask != NULL, "a_mask cannot be NULL");

	*r_mask = *g_mask = *b_mask = *a_mask = 0;

	while (pixfmtstr_len--
		&& (*ptr != '\0' || *ptr != separator))
	{
		switch (*ptr) {
		case 'p':	/* pad */
			curr = NULL;
			break;
		case 'a':	/* alpha */
			curr = a_mask;
			break;
		case 'r':	/* red */
			curr = r_mask;
			break;
		case 'g':	/* green */
			curr = g_mask;
			break;
		case 'b':	/* blue */
			curr = b_mask;
			break;

		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
			nbits = strtoul(ptr, NULL, 10);
			*r_mask <<= nbits;
			*g_mask <<= nbits;
			*b_mask <<= nbits;
			*a_mask <<= nbits;
			if (curr != NULL)
				*curr |= ((1 << nbits) - 1);
			break;

		default:
			DPRINT("_ggi_parse_pixfmtstr: Detected invalid character: %c\n",
					*ptr);
			goto err0;
		}
		ptr++;
	}

	if (endptr != NULL) *endptr = ptr;

	return GGI_OK;

err0:
	if (endptr != NULL) *endptr = ptr;
	return GGI_ENOMATCH;
}	/* _ggi_parse_pixfmtstr */


/* Generate a string representing the pixelformat e.g. r5g6b5.
   This can be used for dl loading or for passing pixfmt through 
   stringified/user-accessible interfaces.   Note these strings
   don't use the same format as GGI_DB_STD_*, though it would be
   nice if we could change GGI_DB_STD_* and depricate old values. */
/* TODO: flesh this out                                       */
int _ggi_build_pixfmtstr (struct ggi_visual *vis, char *pixfmtstr,
			size_t pixfmtstr_len, int flags)
{
	LIB_ASSERT(vis != NULL, "Invalid visual");
	LIB_ASSERT(pixfmtstr != NULL, "Invalid string pointer");
	LIB_ASSERT(pixfmtstr_len > 0, "Invalid string length");

	if ((flags != GGI_PIXFMT_GRAPHTYPE)
	   && (flags != GGI_PIXFMT_CHANNEL)
	   && (flags == GGI_PIXFMT_ALPHA_USED))
	{
		return GGI_EARGINVAL;
	}

	/* Now, we can assume there's at least room
	 * for the terminating \0.
	 */

	if (flags & GGI_PIXFMT_CHANNEL) {
		char alpha_or_pad, *ptr;
		ggi_pixelformat *pixfmt;
		size_t tmp;
		int idx;
		unsigned count;

		pixfmt = LIBGGI_PIXFMT(vis);
		alpha_or_pad = (flags & GGI_PIXFMT_ALPHA_USED) ? 'a' : 'p';
		idx = pixfmt->size - 1;
		if (idx > 31) {		/* paranoia never hurts. */
			return GGI_ENOMATCH;
		}
		ptr = pixfmtstr;

		while (pixfmtstr_len != 0) {
			pixfmtstr_len--;
			LIB_ASSERT(pixfmtstr_len > 0,
				"pixfmtstr_len too short. Not enough memory allocated for pixfmtstr.");

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

			count = 1;
			while ((pixfmt->bitmeaning[idx] & 0x00ffff00) ==
			       (pixfmt->bitmeaning[idx - 1] & 0x00ffff00))
			{
				if (idx == 0) break;
				idx--;
				count++;
			}

			tmp = snprintf(ptr, pixfmtstr_len, "%u", count);
			LIB_ASSERT(tmp < pixfmtstr_len,
				"pixfmtstr_len too short. Not enough memory allocated for pixfmtstr.");
			LIB_ASSERT(tmp <= pixfmtstr_len,
				"Off-by-one bug: Not even room for string termination.");

			pixfmtstr_len -= tmp;
			ptr += tmp;
			idx--;
			if (idx < 0) break;
		}
		LIB_ASSERT(pixfmtstr_len >= 1, "Off-by-one bug! No room for string termination.");
		*ptr = '\0';

	} else {
		size_t tmp;

		tmp = snprintf(pixfmtstr, pixfmtstr_len, "%u", GT_SIZE(LIBGGI_GT(vis)));
		LIB_ASSERT(tmp < pixfmtstr_len, "pixfmtstr has been truncated");
	}

	return GGI_OK;
}

int _ggi_match_palette(ggi_color *pal, int pal_len, const ggi_color *col)
{
	int i, closest=0;
	int r = col->r, g = col->g, b = col->b;

	uint32_t closest_dist = (1U << 31U);

	for (i=0; i < pal_len; i++) {
#undef ABS
#define ABS(val)	((val) < 0 ? -(val) : val)
		uint32_t dist =
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

	DPRINT_COLOR("match-color: %02x%02x%02x -> %02x%02x%02x (%d).\n",
		     col->r >> 8, col->g >> 8, col->b >> 8,
		     pal[closest].r >> 8, pal[closest].g >> 8,
		     pal[closest].b >> 8, closest);

	return closest;
}

int _ggi_default_setreadframe(struct ggi_visual *vis, int num)
{
        ggi_directbuffer *db = _ggi_db_find_frame(vis, num);

        if (db == NULL) {
                return GGI_ENOSPACE;
        }

        vis->r_frame_num = num;
        vis->r_frame = db;

        return 0;
}

int _ggi_default_setwriteframe(struct ggi_visual *vis, int num)
{
        ggi_directbuffer *db = _ggi_db_find_frame(vis, num);

        if (db == NULL) {
                return GGI_ENOSPACE;
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
	ggi_color black  = { 0x0000, 0x0000, 0x0000, 0x0000 };
	ggi_color white  = { 0xffff, 0xffff, 0xffff, 0x0000 };
	ggi_color blue   = { 0x0000, 0x0000, 0xffff, 0x0000 };
	ggi_color yellow = { 0xffff, 0xffff, 0x0000, 0x0000 };

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


