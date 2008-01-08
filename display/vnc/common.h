/* $Id: common.h,v 1.6 2008/01/08 13:33:33 pekberg Exp $
******************************************************************************

   display-vnc: common encoder operations

   Copyright (C) 2006 Peter Rosin	[peda@lysator.liu.se]

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

#ifndef _GGI_VNC_COMMON_H
#define _GGI_VNC_COMMON_H

#include <ggi/ggi-types.h>
/* need display/vnc.h for ggi_rect typedef, not so pretty... */
#include <ggi/display/vnc.h>
#include "rect.h"

#define DESKSIZE_INIT           (1<<4)
#define DESKSIZE_PENDING        (1<<3)
#define DESKSIZE_OK             (1<<2)
#define DESKSIZE_PIXFMT         (1<<1)
#define DESKSIZE_SEND           (1<<0)
#define DESKSIZE_PENDING_PIXFMT (DESKSIZE_PENDING | DESKSIZE_PIXFMT)
#define DESKSIZE_PENDING_SEND   (DESKSIZE_PENDING | DESKSIZE_SEND)
#define DESKSIZE_ACTIVATE       (~(DESKSIZE_INIT | DESKSIZE_PENDING))
#define DESKSIZE_OK_SEND        (DESKSIZE_OK | DESKSIZE_SEND)
#define DESKSIZE_OK_INIT        (DESKSIZE_OK | DESKSIZE_INIT)
#define DESKSIZE_PIXFMT_SEND    (DESKSIZE_PIXFMT | DESKSIZE_SEND)

#define DESKNAME_OK             (1<<0)
#define DESKNAME_PENDING        (1<<1)
#define DESKNAME_SEND           (DESKNAME_PENDING | DESKNAME_OK)

static inline int
palette_match_8(uint8_t *palette, int colors, uint8_t color)
{
	int c;

	for (c = 0; c < colors; ++c) {
		if (palette[c] == color)
			break;
	}

	return c;
}

static inline int
palette_match_16(uint16_t *palette, int colors, uint16_t color)
{
	int c;

	for (c = 0; c < colors; ++c) {
		if (palette[c] == color)
			break;
	}

	return c;
}

static inline int
palette_match_32(uint32_t *palette, int colors, uint32_t color)
{
	int c;

	for (c = 0; c < colors; ++c) {
		if (palette[c] == color)
			break;
	}

	return c;
}

static inline uint8_t *
insert_hilo_16(uint8_t *dst, uint16_t pixel)
{
	*dst++ = pixel >> 8;
	*dst++ = pixel;
	return dst;
}

static inline uint8_t *
insert_hilo_24l(uint8_t *dst, uint32_t pixel)
{
	*dst++ = pixel >> 16;
	*dst++ = pixel >> 8;
	*dst++ = pixel;
	return dst;
}

static inline uint8_t *
insert_hilo_24h(uint8_t *dst, uint32_t pixel)
{
	*dst++ = pixel >> 24;
	*dst++ = pixel >> 16;
	*dst++ = pixel >> 8;
	return dst;
}

static inline uint8_t *
insert_hilo_24(uint8_t *dst, uint32_t pixel, int lower)
{
	if (lower)
		return insert_hilo_24l(dst, pixel);
	else
		return insert_hilo_24h(dst, pixel);
}

static inline uint8_t *
insert_hilo_32(uint8_t *dst, uint32_t pixel)
{
	*dst++ = pixel >> 24;
	*dst++ = pixel >> 16;
	*dst++ = pixel >> 8;
	*dst++ = pixel;
	return dst;
}

static inline uint8_t *
insert_lohi_16(uint8_t *dst, uint16_t pixel)
{
	*dst++ = pixel;
	*dst++ = pixel >> 8;
	return dst;
}

static inline uint8_t *
insert_lohi_24l(uint8_t *dst, uint32_t pixel)
{
	*dst++ = pixel;
	*dst++ = pixel >> 8;
	*dst++ = pixel >> 16;
	return dst;
}

static inline uint8_t *
insert_lohi_24h(uint8_t *dst, uint32_t pixel)
{
	*dst++ = pixel >> 8;
	*dst++ = pixel >> 16;
	*dst++ = pixel >> 24;
	return dst;
}

static inline uint8_t *
insert_lohi_24(uint8_t *dst, uint32_t pixel, int lower)
{
	if (lower)
		return insert_lohi_24l(dst, pixel);
	else
		return insert_lohi_24h(dst, pixel);
}

static inline uint8_t *
insert_lohi_32(uint8_t *dst, uint32_t pixel)
{
	*dst++ = pixel;
	*dst++ = pixel >> 8;
	*dst++ = pixel >> 16;
	*dst++ = pixel >> 24;
	return dst;
}

static inline uint8_t *
insert_rev_16(uint8_t *dst, uint16_t pixel)
{
#ifdef GGI_BIG_ENDIAN
	return insert_lohi_16(dst, pixel);
#else
	return insert_hilo_16(dst, pixel);
#endif
}

static inline uint8_t *
insert_rev_24l(uint8_t *dst, uint32_t pixel)
{
#ifdef GGI_BIG_ENDIAN
	return insert_lohi_24l(dst, pixel);
#else
	return insert_hilo_24l(dst, pixel);
#endif
}

static inline uint8_t *
insert_rev_24h(uint8_t *dst, uint32_t pixel)
{
#ifdef GGI_BIG_ENDIAN
	return insert_lohi_24h(dst, pixel);
#else
	return insert_hilo_24h(dst, pixel);
#endif
}

static inline uint8_t *
insert_rev_32(uint8_t *dst, uint32_t pixel)
{
#ifdef GGI_BIG_ENDIAN
	return insert_lohi_32(dst, pixel);
#else
	return insert_hilo_32(dst, pixel);
#endif
}

static inline uint8_t *
insert_16(uint8_t *dst, uint16_t pixel)
{
#ifdef GGI_BIG_ENDIAN
	return insert_hilo_16(dst, pixel);
#else
	return insert_lohi_16(dst, pixel);
#endif
}

static inline uint8_t *
insert_24l(uint8_t *dst, uint32_t pixel)
{
#ifdef GGI_BIG_ENDIAN
	return insert_hilo_24l(dst, pixel);
#else
	return insert_lohi_24l(dst, pixel);
#endif
}

static inline uint8_t *
insert_24h(uint8_t *dst, uint32_t pixel)
{
#ifdef GGI_BIG_ENDIAN
	return insert_hilo_24h(dst, pixel);
#else
	return insert_lohi_24h(dst, pixel);
#endif
}

static inline uint8_t *
insert_888(uint8_t *dst, uint32_t pixel)
{
	return insert_hilo_24l(dst, pixel);
}

static inline uint8_t *
insert_32(uint8_t *dst, uint32_t pixel)
{
#ifdef GGI_BIG_ENDIAN
	return insert_hilo_32(dst, pixel);
#else
	return insert_lohi_32(dst, pixel);
#endif
}

static inline uint8_t *
insert_header(uint8_t *header, ggi_coord *tl, ggi_rect *size, uint32_t encoding)
{
	header = insert_hilo_16(header, tl->x);
	header = insert_hilo_16(header, tl->y);
	header = insert_hilo_16(header, ggi_rect_width(size));
	header = insert_hilo_16(header, ggi_rect_height(size));
	return   insert_hilo_32(header, encoding);
}

#endif /* _GGI_VNC_COMMON_H */
