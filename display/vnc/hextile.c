/* $Id: hextile.c,v 1.2 2006/09/08 19:43:00 pekberg Exp $
******************************************************************************

   display-vnc: RFB hextile encoding

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

#include "config.h"

#include <stdlib.h>
#ifdef HAVE_STRING_H
#include <string.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include <ggi/display/vnc.h>
#include <ggi/internal/ggi_debug.h>
#include "rect.h"
#include "encoding.h"

#ifdef GGI_BIG_ENDIAN
#define GGI_HTONL(x) (x)
#else
#define GGI_HTONL(x) GGI_BYTEREV32(x)
#endif

struct hextile_ctx_t {
	ggi_pixel bg;
	ggi_pixel fg;
	int flags;
};

/* subencoding */
#define HEXTILE_RAW      (1<<0)
#define HEXTILE_BG       (1<<1)
#define HEXTILE_FG       (1<<2)
#define HEXTILE_SUBRECTS (1<<3)
#define HEXTILE_COLORED  (1<<4)

typedef void (tile_func)(struct hextile_ctx_t *ctx, uint8_t **buf,
	uint8_t *src, int xs, int ys, int stride, int rev);

static uint8_t
scan_8(uint8_t *src,
	int size, uint8_t *bg, uint8_t *fg)
{
	int count[3] = { 0, 0, 0 };
	uint8_t color[3];
	int cidx = 0;
	int i;
	int j;
	int select_cnt;
	int select_idx;

	color[cidx] = src[0];

	for (i = 0; i < size; ++i) {
		if (color[cidx] == src[i]) {
			++count[cidx];
			continue;
		}
		select_idx = cidx;
		select_cnt = count[cidx];
		j = (cidx + 1) % 3;
		while (j != cidx) {
			if (count[j] && (color[j] == src[i])) {
				select_idx = j;
				break;
			}
			if (count[j] < select_cnt) {
				select_cnt = count[j];
				select_idx = j;
			}
			j = (j + 1) % 3;
		}
		cidx = select_idx;
		if (color[cidx] != src[i]) {
			color[cidx] = src[i];
			count[cidx] = 0;
		}
		++count[cidx];
	}

	if (!count[1]) {
		/* 1 color */
		*bg = color[0];
		return HEXTILE_BG;
	}
	if (!count[2]) {
		/* 2 colors */
		if (count[0] >= count[1]) {
			*bg = color[0];
			*fg = color[1];
		}
		else {
			*bg = color[1];
			*fg = color[0];
		}
		return HEXTILE_BG | HEXTILE_FG | HEXTILE_SUBRECTS;
	}

	/* 3 or more colors, only care about bg */
	if (count[0] >= count[1] && count[0] >= count[2])
		*bg = color[0];
	else if (count[1] >= count[0] && count[1] >= count[2])
		*bg = color[1];
	else
		*bg = color[2];

	return HEXTILE_BG | HEXTILE_SUBRECTS | HEXTILE_COLORED;
}

static uint8_t
scan_16(uint16_t *src,
	int size, uint16_t *bg, uint16_t *fg)
{
	int count[3] = { 0, 0, 0 };
	uint16_t color[3];
	int cidx = 0;
	int i;
	int j;
	int select_cnt;
	int select_idx;

	color[cidx] = src[0];

	for (i = 0; i < size; ++i) {
		if (color[cidx] == src[i]) {
			++count[cidx];
			continue;
		}
		select_idx = cidx;
		select_cnt = count[cidx];
		j = (cidx + 1) % 3;
		while (j != cidx) {
			if (count[j] && (color[j] == src[i])) {
				select_idx = j;
				break;
			}
			if (count[j] < select_cnt) {
				select_cnt = count[j];
				select_idx = j;
			}
			j = (j + 1) % 3;
		}
		cidx = select_idx;
		if (color[cidx] != src[i]) {
			color[cidx] = src[i];
			count[cidx] = 0;
		}
		++count[cidx];
	}

	if (!count[1]) {
		/* 1 color */
		*bg = color[0];
		return HEXTILE_BG;
	}
	if (!count[2]) {
		/* 2 colors */
		if (count[0] >= count[1]) {
			*bg = color[0];
			*fg = color[1];
		}
		else {
			*bg = color[1];
			*fg = color[0];
		}
		return HEXTILE_BG | HEXTILE_FG | HEXTILE_SUBRECTS;
	}

	/* 3 or more colors, only care about bg */
	if (count[0] >= count[1] && count[0] >= count[2])
		*bg = color[0];
	else if (count[1] >= count[0] && count[1] >= count[2])
		*bg = color[1];
	else
		*bg = color[2];

	return HEXTILE_BG | HEXTILE_SUBRECTS | HEXTILE_COLORED;
}

static uint8_t
scan_32(uint32_t *src,
	int size, uint32_t *bg, uint32_t *fg)
{
	int count[3] = { 0, 0, 0 };
	uint32_t color[3];
	int cidx = 0;
	int i;
	int j;
	int select_cnt;
	int select_idx;

	color[cidx] = src[0];

	for (i = 0; i < size; ++i) {
		if (color[cidx] == src[i]) {
			++count[cidx];
			continue;
		}
		select_idx = cidx;
		select_cnt = count[cidx];
		j = (cidx + 1) % 3;
		while (j != cidx) {
			if (count[j] && (color[j] == src[i])) {
				select_idx = j;
				break;
			}
			if (count[j] < select_cnt) {
				select_cnt = count[j];
				select_idx = j;
			}
			j = (j + 1) % 3;
		}
		cidx = select_idx;
		if (color[cidx] != src[i]) {
			color[cidx] = src[i];
			count[cidx] = 0;
		}
		++count[cidx];
	}

	if (!count[1]) {
		/* 1 color */
		*bg = color[0];
		return HEXTILE_BG;
	}
	if (!count[2]) {
		/* 2 colors */
		if (count[0] >= count[1]) {
			*bg = color[0];
			*fg = color[1];
		}
		else {
			*bg = color[1];
			*fg = color[0];
		}
		return HEXTILE_BG | HEXTILE_FG | HEXTILE_SUBRECTS;
	}

	/* 3 or more colors, only care about bg */
	if (count[0] >= count[1] && count[0] >= count[2])
		*bg = color[0];
	else if (count[1] >= count[0] && count[1] >= count[2])
		*bg = color[1];
	else
		*bg = color[2];

	return HEXTILE_BG | HEXTILE_SUBRECTS | HEXTILE_COLORED;
}

static inline uint8_t *
insert_hilo_16(uint8_t *dst, uint16_t pixel)
{
	*dst++ = pixel >> 8;
	*dst++ = pixel;
	return dst;
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
insert_32(uint8_t *dst, uint32_t pixel)
{
#ifdef GGI_BIG_ENDIAN
	return insert_hilo_32(dst, pixel);
#else
	return insert_lohi_32(dst, pixel);
#endif
}

static uint8_t *
rectify_8(uint8_t *dst, uint8_t *data, int xs, int ys,
	uint8_t subencoding, uint8_t bg, uint8_t fg)
{
	int i;
	int x, y;
	int rects;
	uint8_t *count = dst++;
	uint8_t color;
	ggi_rect c;

	*count = 0;

	rects = xs * ys - 1;
	rects -= (subencoding & HEXTILE_BG) ? 1 : 0;
	rects -= (subencoding & HEXTILE_FG) ? 1 : 0;
	rects /= 2 + ((subencoding & HEXTILE_COLORED) ? 1 : 0);

	for (i = 0; i < xs * ys; ++i) {
		if (data[i] == bg)
			continue;
		if (++*count > rects)
			return NULL;
		color = data[i];
		if (subencoding & HEXTILE_COLORED)
			*dst++ = color;
		c.tl.x = i % xs;
		c.tl.y = y = i / xs;
		for (x = c.tl.x; x < xs; ++x) {
			if (data[y * xs + x] != color)
				break;
		}
		c.br.x = x;
		for (x = c.tl.x; y < ys; ++y) {
			if (data[y * xs + x] != color)
				break;
		}
		c.br.y = y;
		if (ggi_rect_width(&c) >= ggi_rect_height(&c)) {
			for (y = c.tl.y; y < c.br.y; ++y) {
				for (x = c.tl.x; x < c.br.x; ++x) {
					if (data[y * xs + x] != color)
						break;
				}
				if (x != c.br.x)
					break;
				for (x = c.tl.x; x < c.br.x; ++x)
					data[y * xs + x] = bg;
			}
			c.br.y = y;
		}
		else {
			for (x = c.tl.x; x < c.br.x; ++x) {
				for (y = c.tl.y; y < c.br.y; ++y) {
					if (data[y * xs + x] != color)
						break;
				}
				if (y != c.br.y)
					break;
				for (y = c.tl.y; y < c.br.y; ++y)
					data[y * xs + x] = bg;
			}
			c.br.x = x;
		}
		*dst++ = (c.tl.x << 4) | c.tl.y;
		*dst++ = ((ggi_rect_width(&c) - 1) << 4) |
			(ggi_rect_height(&c) - 1);
	}

	return dst;
}

typedef uint8_t *(insert_16_t)(uint8_t *dst, uint16_t pixel);

static uint8_t *
rectify_16(uint8_t *dst, uint16_t *data, int xs, int ys,
	uint8_t subencoding, uint16_t bg, uint16_t fg, insert_16_t *insert)
{
	int i;
	int x, y;
	int rects;
	uint8_t *count = dst++;
	uint16_t color;
	ggi_rect c;

	*count = 0;

	rects = xs * ys - 1;
	rects -= (subencoding & HEXTILE_BG) ? 2 : 0;
	rects -= (subencoding & HEXTILE_FG) ? 2 : 0;
	rects /= 2 + ((subencoding & HEXTILE_COLORED) ? 2 : 0);

	for (i = 0; i < xs * ys; ++i) {
		if (data[i] == bg)
			continue;
		if (++*count > rects)
			return NULL;
		color = data[i];
		if (subencoding & HEXTILE_COLORED)
			dst = insert(dst, color);
		c.tl.x = i % xs;
		c.tl.y = y = i / xs;
		for (x = c.tl.x; x < xs; ++x) {
			if (data[y * xs + x] != color)
				break;
		}
		c.br.x = x;
		for (x = c.tl.x; y < ys; ++y) {
			if (data[y * xs + x] != color)
				break;
		}
		c.br.y = y;
		if (ggi_rect_width(&c) >= ggi_rect_height(&c)) {
			for (y = c.tl.y; y < c.br.y; ++y) {
				for (x = c.tl.x; x < c.br.x; ++x) {
					if (data[y * xs + x] != color)
						break;
				}
				if (x != c.br.x)
					break;
				for (x = c.tl.x; x < c.br.x; ++x)
					data[y * xs + x] = bg;
			}
			c.br.y = y;
		}
		else {
			for (x = c.tl.x; x < c.br.x; ++x) {
				for (y = c.tl.y; y < c.br.y; ++y) {
					if (data[y * xs + x] != color)
						break;
				}
				if (y != c.br.y)
					break;
				for (y = c.tl.y; y < c.br.y; ++y)
					data[y * xs + x] = bg;
			}
			c.br.x = x;
		}
		*dst++ = (c.tl.x << 4) | c.tl.y;
		*dst++ = ((ggi_rect_width(&c) - 1) << 4) |
			(ggi_rect_height(&c) - 1);
	}

	return dst;
}

typedef uint8_t *(insert_32_t)(uint8_t *dst, uint32_t pixel);

static uint8_t *
rectify_32(uint8_t *dst, uint32_t *data, int xs, int ys,
	uint8_t subencoding, uint32_t bg, uint32_t fg, insert_32_t *insert)
{
	int i;
	int x, y;
	int rects;
	uint8_t *count = dst++;
	uint32_t color;
	ggi_rect c;

	*count = 0;

	rects = xs * ys - 1;
	rects -= (subencoding & HEXTILE_BG) ? 4 : 0;
	rects -= (subencoding & HEXTILE_FG) ? 4 : 0;
	rects /= 2 + ((subencoding & HEXTILE_COLORED) ? 4 : 0);

	for (i = 0; i < xs * ys; ++i) {
		if (data[i] == bg)
			continue;
		if (++*count > rects)
			return NULL;
		color = data[i];
		if (subencoding & HEXTILE_COLORED)
			dst = insert(dst, color);
		c.tl.x = i % xs;
		c.tl.y = y = i / xs;
		for (x = c.tl.x; x < xs; ++x) {
			if (data[y * xs + x] != color)
				break;
		}
		c.br.x = x;
		for (x = c.tl.x; y < ys; ++y) {
			if (data[y * xs + x] != color)
				break;
		}
		c.br.y = y;
		if (ggi_rect_width(&c) >= ggi_rect_height(&c)) {
			for (y = c.tl.y; y < c.br.y; ++y) {
				for (x = c.tl.x; x < c.br.x; ++x) {
					if (data[y * xs + x] != color)
						break;
				}
				if (x != c.br.x)
					break;
				for (x = c.tl.x; x < c.br.x; ++x)
					data[y * xs + x] = bg;
			}
			c.br.y = y;
		}
		else {
			for (x = c.tl.x; x < c.br.x; ++x) {
				for (y = c.tl.y; y < c.br.y; ++y) {
					if (data[y * xs + x] != color)
						break;
				}
				if (y != c.br.y)
					break;
				for (y = c.tl.y; y < c.br.y; ++y)
					data[y * xs + x] = bg;
			}
			c.br.x = x;
		}
		*dst++ = (c.tl.x << 4) | c.tl.y;
		*dst++ = ((ggi_rect_width(&c) - 1) << 4) |
			(ggi_rect_height(&c) - 1);
	}

	return dst;
}

static void
tile_8(struct hextile_ctx_t *ctx, uint8_t **buf,
	uint8_t *src, int xs, int ys, int stride, int rev)
{
	uint8_t subencoding = 0;
	uint8_t *dst = *buf;
	uint8_t bg, fg;
	uint8_t data[256];
	uint8_t *tdst;
	uint8_t *tsrc;
	int y;

	++dst; /* make room for subencoding */

	tdst = data;
	tsrc = src;
	for (y = 0; y < ys; ++y, tdst += xs, tsrc += stride)
		memcpy(tdst, tsrc, xs);

	subencoding = scan_8(data, xs * ys, &bg, &fg);

	if (subencoding & HEXTILE_BG) {
		if ((ctx->flags & HEXTILE_BG) && (ctx->bg == bg))
			/* same bg */
			subencoding &= ~HEXTILE_BG;
		else
			*dst++ = bg;
	}

	if (subencoding & HEXTILE_FG) {
		if ((ctx->flags & HEXTILE_FG) && (ctx->fg == fg))
			/* same fg */
			subencoding &= ~HEXTILE_FG;
		else
			*dst++ = fg;
	}

	if (subencoding & HEXTILE_SUBRECTS) {
		dst = rectify_8(dst, data, xs, ys, subencoding, bg, fg);
		if (!dst) {
			subencoding = HEXTILE_RAW;
			dst = *buf + 1;
			for (y = 0; y < ys; ++y, dst += xs, src += stride)
				memcpy(dst, src, xs);
			goto done;
		}
	}

	if (subencoding & HEXTILE_BG) {
		ctx->flags |= HEXTILE_BG;
		ctx->bg = bg;
	}
	if (subencoding & HEXTILE_FG) {
		ctx->flags |= HEXTILE_FG;
		ctx->fg = fg;
	}
	else
		ctx->flags &= ~HEXTILE_FG;

done:
	**buf = subencoding;
	*buf = dst;
}

static void
tile_16(struct hextile_ctx_t *ctx, uint8_t **buf,
	uint8_t *src8, int xs, int ys, int stride, int rev)
{
	uint16_t *src = (uint16_t *)src8;
	uint8_t subencoding = 0;
	uint8_t *dst = *buf;
	uint16_t bg, fg;
	uint16_t data[256];
	uint16_t *tdst;
	uint16_t *tsrc;
	int x, y;
	insert_16_t *insert = rev ? insert_rev_16 : insert_16;

	++dst; /* make room for subencoding */

	tdst = data;
	tsrc = src;
	for (y = 0; y < ys; ++y, tdst += xs, tsrc += stride)
		memcpy(tdst, tsrc, xs * 2);

	subencoding = scan_16(data, xs * ys, &bg, &fg);

	if (subencoding & HEXTILE_BG) {
		if ((ctx->flags & HEXTILE_BG) && (ctx->bg == bg))
			/* same bg */
			subencoding &= ~HEXTILE_BG;
		else
			dst = insert(dst, bg);
	}

	if (subencoding & HEXTILE_FG) {
		if ((ctx->flags & HEXTILE_FG) && (ctx->fg == fg))
			/* same fg */
			subencoding &= ~HEXTILE_FG;
		else
			dst = insert(dst, fg);
	}

	if (subencoding & HEXTILE_SUBRECTS) {
		dst = rectify_16(dst,
			data, xs, ys, subencoding, bg, fg, insert);
		if (!dst) {
			subencoding = HEXTILE_RAW;
			dst = *buf + 1;
			if (!rev) {
				for (y = 0; y < ys; ++y) {
					memcpy(dst, src, xs * 2);
					src += stride;
					dst += xs * 2;
				}
				goto done;
			}
			for (y = 0; y < ys; ++y) {
				for (x = 0; x < xs; ++x)
					dst = insert_rev_16(dst, src[x]);
				src += stride;
			}
			goto done;
		}
	}

	if (subencoding & HEXTILE_BG) {
		ctx->flags |= HEXTILE_BG;
		ctx->bg = bg;
	}
	if (subencoding & HEXTILE_FG) {
		ctx->flags |= HEXTILE_FG;
		ctx->fg = fg;
	}
	else
		ctx->flags &= ~HEXTILE_FG;

done:
	**buf = subencoding;
	*buf = dst;
}

static void
tile_32(struct hextile_ctx_t *ctx, uint8_t **buf,
	uint8_t *src8, int xs, int ys, int stride, int rev)
{
	uint32_t *src = (uint32_t *)src8;
	uint8_t subencoding = 0;
	uint8_t *dst = *buf;
	uint32_t bg, fg;
	uint32_t data[256];
	uint32_t *tdst;
	uint32_t *tsrc;
	int x, y;
	insert_32_t *insert = rev ? insert_rev_32 : insert_32;

	++dst; /* make room for subencoding */

	tdst = data;
	tsrc = src;
	for (y = 0; y < ys; ++y, tdst += xs, tsrc += stride)
		memcpy(tdst, tsrc, xs * 4);

	subencoding = scan_32(data, xs * ys, &bg, &fg);

	if (subencoding & HEXTILE_BG) {
		if ((ctx->flags & HEXTILE_BG) && (ctx->bg == bg))
			/* same bg */
			subencoding &= ~HEXTILE_BG;
		else
			dst = insert(dst, bg);
	}

	if (subencoding & HEXTILE_FG) {
		if ((ctx->flags & HEXTILE_FG) && (ctx->fg == fg))
			/* same fg */
			subencoding &= ~HEXTILE_FG;
		else
			dst = insert(dst, fg);
	}

	if (subencoding & HEXTILE_SUBRECTS) {
		dst = rectify_32(dst,
			data, xs, ys, subencoding, bg, fg, insert);
		if (!dst) {
			subencoding = HEXTILE_RAW;
			dst = *buf + 1;
			if (!rev) {
				for (y = 0; y < ys; ++y) {
					memcpy(dst, src, xs * 4);
					src += stride;
					dst += xs * 4;
				}
				goto done;
			}
			for (y = 0; y < ys; ++y) {
				for (x = 0; x < xs; ++x)
					dst = insert(dst, src[x]);
				src += stride;
			}
			goto done;
		}
	}

	if (subencoding & HEXTILE_BG) {
		ctx->flags |= HEXTILE_BG;
		ctx->bg = bg;
	}
	if (subencoding & HEXTILE_FG) {
		ctx->flags |= HEXTILE_FG;
		ctx->fg = fg;
	}
	else
		ctx->flags &= ~HEXTILE_FG;

done:
	**buf = subencoding;
	*buf = dst;
}

int
GGI_vnc_hextile(ggi_vnc_client *client, ggi_rect *update)
{
	struct ggi_visual *vis = client->owner;
	ggi_vnc_priv *priv = VNC_PRIV(vis);
	struct hextile_ctx_t *ctx = client->hextile_ctx;
	struct ggi_visual *cvis;
	const ggi_directbuffer *db;
	ggi_graphtype gt;
	int bpp;
	int count;
	unsigned char *buf;
	unsigned char *header;
	int xtiles, ytiles;
	tile_func *tile;
	int xt, yt;
	int xs, ys;
	int xs_last, ys_last;
	int stride;
	ggi_rect vupdate;
	int d_frame_num;

	DPRINT("hextile update %dx%d - %dx%d\n",
		update->tl.x, update->tl.y,
		update->br.x, update->br.y);

	vupdate = *update;
	ggi_rect_shift_xy(&vupdate, vis->origin_x, vis->origin_y);

	ggi_rect_subtract(&client->dirty, &vupdate);
	client->update.tl.x = client->update.br.x = 0;

	DPRINT("dirty %dx%d - %dx%d\n",
		client->dirty.tl.x, client->dirty.tl.y,
		client->dirty.br.x, client->dirty.br.y);

	if (!client->vis) {
		cvis = priv->fb;
		d_frame_num = vis->d_frame_num;
	}
	else {
		int r_frame_num = _ggiGetReadFrame(priv->fb);
		_ggiSetReadFrame(priv->fb, _ggiGetDisplayFrame(vis));

		cvis = client->vis;
		_ggiCrossBlit(priv->fb,
			vupdate.tl.x, vupdate.tl.y,
			ggi_rect_width(&vupdate),
			ggi_rect_height(&vupdate),
			cvis,
			vupdate.tl.x, vupdate.tl.y);

		_ggiSetReadFrame(priv->fb, r_frame_num);
		d_frame_num = 0;
	}

	gt = LIBGGI_GT(cvis);

	bpp = GT_ByPP(gt);
	count = ggi_rect_width(&vupdate) * ggi_rect_height(&vupdate);
	xtiles = (ggi_rect_width(&vupdate) + 15) / 16;
	ytiles = (ggi_rect_height(&vupdate) + 15) / 16;
	GGI_vnc_buf_reserve(&client->wbuf,
		client->wbuf.size + 12 + xtiles * ytiles + count * bpp);

	header = &client->wbuf.buf[client->wbuf.size];
	header[ 0] = update->tl.x >> 8;
	header[ 1] = update->tl.x & 0xff;
	header[ 2] = update->tl.y >> 8;
	header[ 3] = update->tl.y & 0xff;
	header[ 4] = ggi_rect_width(&vupdate) >> 8;
	header[ 5] = ggi_rect_width(&vupdate) & 0xff;
	header[ 6] = ggi_rect_height(&vupdate) >> 8;
	header[ 7] = ggi_rect_height(&vupdate) & 0xff;
	header[ 8] = 0;
	header[ 9] = 0;
	header[10] = 0;
	header[11] = 5; /* hextile */
	client->wbuf.size += 12;

	buf = &client->wbuf.buf[client->wbuf.size];

	if (bpp == 1)
		tile = tile_8;
	else if (bpp == 2)
		tile = tile_16;
	else
		tile = tile_32;

	ctx->flags = 0;

	stride = LIBGGI_VIRTX(cvis);

	ys_last = ggi_rect_height(&vupdate) & 0xf;
	if (!ys_last)
		ys_last = 16;
	xs_last = ggi_rect_width(&vupdate) & 0xf;
	if (!xs_last)
		xs_last = 16;

	db = ggiDBGetBuffer(cvis->stem, d_frame_num);
	ggiResourceAcquire(db->resource, GGI_ACTYPE_READ);

	ys = 16;
	for (yt = 0; yt < ytiles; ++yt) {
		if (yt == ytiles - 1)
			ys = ys_last;
		xs = 16;
		for (xt = 0; xt < xtiles; ++xt) {
			if (xt == xtiles - 1)
				xs = xs_last;
			tile(ctx, &buf, (uint8_t *)db->read +
				((vupdate.tl.y + 16 * yt) * stride +
				 vupdate.tl.x + 16 * xt) * bpp,
				xs, ys, stride, client->reverse_endian);
		}
	}

	ggiResourceRelease(db->resource);

	client->wbuf.size = buf - client->wbuf.buf;

	LIB_ASSERT(client->wbuf.size <= client->wbuf.limit,
		"buffer overrun");

	return 1;
}

struct hextile_ctx_t *
GGI_vnc_hextile_open(void)
{
	struct hextile_ctx_t *ctx = malloc(sizeof(*ctx));
	memset(ctx, 0, sizeof(*ctx));
	return ctx;
}

void
GGI_vnc_hextile_close(struct hextile_ctx_t *ctx)
{
	free(ctx);
}
