/* $Id: radeon_accel.h,v 1.9 2003/02/07 01:35:09 skids Exp $
******************************************************************************

   ATI Radeon sublib function prototypes

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
#ifndef _RADEON_ACCEL_H
#define _RADEON_ACCEL_H

#include <ggi/internal/ggi-dl.h>

#include "kgi/config.h"
#include <ggi/display/kgi.h>
#include "radeon_cce.h"

typedef struct
{

	ggi_accel_t *accel;
	uint32 ctx_loaded;

	kgi_size_t swatch_inuse;
#define RADEON_BAD_CTX 0
#define RADEON_BASE_CTX 1
	struct {
		cce_type0_header_t h1;
		uint32 rb3d_cntl;
		uint32 rb3d_coloroffset;
		uint32 re_width_height;
		uint32 rb3d_colorpitch;
		uint32 se_cntl;
		uint32 se_coord_fmt;
		cce_type0_header_t h2;
		uint32 se_cntl_status;
		cce_type0_header_t h3;
		uint32 re_top_left;
		cce_type0_header_t h4;
		uint32 txablend;
	} base_ctx;
#define RADEON_SOLIDFILL_CTX 2
	struct {
		cce_type0_header_t h1;
		uint32 pp_cntl;
	} solidfill_ctx;
#define RADEON_COPYBOX_CTX 3
	struct {
                cce_type0_header_t h1;
                uint32 pp_cntl;
                cce_type0_header_t h2;
                pp_txformat_t txformat;
                uint32 txoffset;
		uint32 txcblend;
                cce_type0_header_t h3;
                pp_tex_size_t tex_size;
                pp_txpitch_t txpitch;
	} copybox_ctx;
#define RADEON_PUT_CTX 4
	struct {
                cce_type0_header_t h1;
                uint32 pp_cntl;
                cce_type0_header_t h2;
                pp_txformat_t txformat;
                uint32 txoffset;
		uint32 txcblend;
                cce_type0_header_t h3;
                pp_tex_size_t tex_size;
                pp_txpitch_t txpitch;
	} put_ctx;
#define RADEON_GUI2D_CTX 5
	cce_pitch_offset_t src_pitch_offset;
	uint32 dst_type;
	struct {
		cce_type0_header_t h1;
		cce_pitch_offset_t default_pitch_offset;
		cce_type0_header_t h2;
		cce_scissor_t default_sc_bot_right;
	} gui2d_ctx;
#define RADEON_TEXT_CTX 6
	struct {
                cce_type0_header_t h1;
                uint32 pp_cntl;
                cce_type0_header_t h2;
                pp_txformat_t txformat;
                uint32 txoffset;
		uint32 txcblend;
	} text_ctx;
#define RADEON_CROSSBLIT_CTX 7
	struct {
                cce_type0_header_t h1;
                uint32 pp_cntl;
                cce_type0_header_t h2;
                pp_txformat_t txformat;
                uint32 txoffset;
		uint32 txcblend;
                cce_type0_header_t h3;
                pp_tex_size_t tex_size;
                pp_txpitch_t txpitch;
	} crossblit_ctx;
} radeon_context_t;

#define RADEON_CONTEXT(vis) ((radeon_context_t *)KGI_ACCEL_PRIV(vis))
#define RADEON_ACCEL(vis) (RADEON_CONTEXT(vis)->accel)

#define RADEON_BUFFER_SIZE_ORDER  1
#define RADEON_BUFFER_SIZE        (0x1000 << RADEON_BUFFER_SIZE_ORDER)
#define RADEON_BUFFER_MASK        (RADEON_BUFFER_SIZE - 1)
#define RADEON_BUFFER_SIZE32      (RADEON_BUFFER_SIZE >> 2)
#define RADEON_BUFFER_MASK32      (RADEON_BUFFER_SIZE32 - 1)

#define RADEON_BUFFER_NUM         16

#define RADEON_TOTAL_SIZE         (RADEON_BUFFER_SIZE * RADEON_BUFFER_NUM)
#define RADEON_TOTAL_SIZE32       (RADEON_BUFFER_SIZE32 * RADEON_BUFFER_NUM)

#define RADEON_FLUSH(vis) \
GGI_ACCEL_FLUSH_u32(RADEON_ACCEL(vis), RADEON_BUFFER_SIZE32, RADEON_TOTAL_SIZE32)

#define RADEON_CHECK(vis, n) \
GGI_ACCEL_CHECK_TOTAL_u32(RADEON_ACCEL(vis), n, RADEON_BUFFER_SIZE32, RADEON_TOTAL_SIZE32)

#define RADEON_WRITE(vis, val) \
GGI_ACCEL_WRITE_u32(RADEON_ACCEL(vis), val)

#define RADEON_WRITEPACKET(vis, data)\
do {									\
		uint32 i = sizeof(data) / 4;				\
		uint32 *ptr = (uint32*)&data;				\
		RADEON_CHECK(vis, sizeof(data) / 4);			\
		while (i--)						\
			RADEON_WRITE(vis, *ptr++);			\
} while(0)

#define RADEON_RESTORE_CTX(vis, whatctx) \
do {									\
  if (RADEON_CONTEXT(vis)->ctx_loaded == RADEON_BAD_CTX)		\
    RADEON_WRITEPACKET(vis, (RADEON_CONTEXT(vis)->base_ctx));		\
  if (RADEON_CONTEXT(vis)->ctx_loaded != whatctx) {			\
    switch (whatctx) {							\
    case RADEON_SOLIDFILL_CTX:						\
      RADEON_WRITEPACKET(vis, (RADEON_CONTEXT(vis)->solidfill_ctx));	\
      break;								\
    case RADEON_PUT_CTX:						\
      RADEON_WRITEPACKET(vis, (RADEON_CONTEXT(vis)->put_ctx));		\
      break;								\
    case RADEON_COPYBOX_CTX:						\
      RADEON_WRITEPACKET(vis, (RADEON_CONTEXT(vis)->copybox_ctx));	\
      break;								\
    case RADEON_GUI2D_CTX:						\
      RADEON_WRITEPACKET(vis, (RADEON_CONTEXT(vis)->gui2d_ctx));	\
      break;								\
    case RADEON_TEXT_CTX:						\
      RADEON_WRITEPACKET(vis, (RADEON_CONTEXT(vis)->text_ctx));		\
      break;								\
    case RADEON_CROSSBLIT_CTX:						\
      RADEON_WRITEPACKET(vis, (RADEON_CONTEXT(vis)->crossblit_ctx));	\
      break;								\
    default: break; }}							\
  RADEON_CONTEXT(vis)->ctx_loaded = whatctx;				\
} while (0)
	
ggifunc_drawhline   GGI_kgi_radeon_drawhline;
ggifunc_drawvline   GGI_kgi_radeon_drawvline;
ggifunc_drawline    GGI_kgi_radeon_drawline_2d;
ggifunc_drawline    GGI_kgi_radeon_drawline_3d;
ggifunc_drawbox	    GGI_kgi_radeon_drawbox_2d;
ggifunc_drawbox	    GGI_kgi_radeon_drawbox_3d;
ggifunc_copybox     GGI_kgi_radeon_copybox_2d;
ggifunc_copybox     GGI_kgi_radeon_copybox_3d;
ggifunc_gcchanged   GGI_kgi_radeon_gcchanged_2d;
ggifunc_gcchanged   GGI_kgi_radeon_gcchanged_3d;
ggifunc_putc        GGI_kgi_radeon_putc_2d;
ggifunc_putc        GGI_kgi_radeon_putc_3d;
ggifunc_puts        GGI_kgi_radeon_puts_2d;
ggifunc_puts        GGI_kgi_radeon_puts_3d;
ggifunc_putbox      GGI_kgi_radeon_putbox_3d;
ggifunc_putvline    GGI_kgi_radeon_puthline_3d;
ggifunc_puthline    GGI_kgi_radeon_putvline_3d;
ggifunc_crossblit   GGI_kgi_radeon_crossblit_3d;
ggifunc_getcharsize GGI_kgi_radeon_getcharsize;

void GGI_kgi_radeon_clut_ilut_sync(ggi_visual *vis);

#endif
