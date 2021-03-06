/* $Id: encoding.h,v 1.15 2009/09/21 12:20:20 pekberg Exp $
******************************************************************************

   display-vnc: encoding interface

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

#ifndef _GGI_VNC_ENCODING_H
#define _GGI_VNC_ENCODING_H

#include <ggi/display/vnc.h>

int GGI_vnc_buf_reserve(ggi_vnc_buf *buf, int limit);
struct ggi_visual *GGI_vnc_encode_init(ggi_vnc_client *client,
	ggi_rect *update, ggi_rect *vupdate, int *d_frame_num);

ggi_vnc_encode GGI_vnc_raw;
ggi_vnc_encode GGI_vnc_copyrect_pan;

struct rre_ctx_t;
ggi_vnc_encode GGI_vnc_rre;
struct rre_ctx_t *GGI_vnc_rre_open(void);
void GGI_vnc_rre_close(struct rre_ctx_t *ctx);

ggi_vnc_encode GGI_vnc_corre;

struct hextile_ctx_t;
ggi_vnc_encode GGI_vnc_hextile;
struct hextile_ctx_t *GGI_vnc_hextile_open(void);
void GGI_vnc_hextile_close(struct hextile_ctx_t *ctx);

struct trle_ctx_t;
ggi_vnc_encode GGI_vnc_trle;
struct trle_ctx_t *GGI_vnc_trle_open(void);
void GGI_vnc_trle_close(struct trle_ctx_t *ctx);

#ifdef HAVE_ZLIB

struct zlib_ctx_t;
ggi_vnc_encode GGI_vnc_zlib;
struct zlib_ctx_t *GGI_vnc_zlib_open(int level);
void GGI_vnc_zlib_close(struct zlib_ctx_t *ctx);

struct zlibhex_ctx_t;
ggi_vnc_encode GGI_vnc_zlibhex;
struct zlibhex_ctx_t *GGI_vnc_zlibhex_open(int level);
void GGI_vnc_zlibhex_close(struct zlibhex_ctx_t *ctx);

struct zrle_ctx_t;
ggi_vnc_encode GGI_vnc_zrle;
struct zrle_ctx_t *GGI_vnc_zrle_open(int level);
void GGI_vnc_zrle_close(struct zrle_ctx_t *ctx);

ggi_vnc_encode GGI_vnc_tight;
struct tight_ctx_t *GGI_vnc_tight_open(void);
void GGI_vnc_tight_close(struct tight_ctx_t *ctx);
void GGI_vnc_tight_quality(struct tight_ctx_t *ctx, int quality);

#endif /* HAVE_ZLIB */

#endif /* _GGI_VNC_ENCODING_H */
