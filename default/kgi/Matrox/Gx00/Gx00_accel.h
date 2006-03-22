/* $Id: Gx00_accel.h,v 1.7 2006/03/22 20:29:10 cegger Exp $
******************************************************************************

   Matrox Gx00 accel sublib function prototypes

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
#ifndef _Gx00_ACCEL_H
#define _Gx00_ACCEL_H

#include <ggi/internal/ggi-dl.h>
#include <ggi/internal/ggi_debug.h>

#include "kgi/config.h"
#include <ggi/display/kgi.h>

#include "Gx00.h"

/* *********************************************
 *    Matrox vertex data structures
 * *********************************************/

typedef float mga_vertex_float_t;

typedef struct mga_vertex_color_s {
  kgi_u8_t blue, green, red, alpha;
} mga_vertex_color_t;

typedef struct mga_vertex_s {
  mga_vertex_float_t x; /* screen coordinates */
  mga_vertex_float_t y;
  mga_vertex_float_t z; /* Z buffer depth, in 0.0 - 1.0 */
  mga_vertex_float_t rhw; /* 1/w (reciprocal of homogeneous w) */
  mga_vertex_color_t color; /* vertex color */
  mga_vertex_color_t specular; /* specular component. Alpha is fog factor */
  mga_vertex_float_t tu0; /* texture coordinate stage 0 */
  mga_vertex_float_t tv0;
} mga_vertex_t;

typedef struct mga_vertex2_s {
  mga_vertex_float_t x; /* screen coordinates */
  mga_vertex_float_t y;
  mga_vertex_float_t z; /* 0.0 - 1.0 */
  mga_vertex_float_t rhw; /* 1/w (reciprocal of homogeneous w) */
  mga_vertex_color_t color; /* vertex color */
  mga_vertex_color_t specular; /* specular component. Alpha is fog factor */
  mga_vertex_float_t tu0; /* texture coordinate stage 0 */
  mga_vertex_float_t tv0;
  mga_vertex_float_t tu1; /* texture coordinate stage 1 */
  mga_vertex_float_t tv1;
} mga_vertex2_t;


/* *********************************************
 *    Matrox raw DMA buffer data structures
 * *********************************************/

/*
** Data structures
*/

/* Type of buffer */
typedef enum
{
  MGA_DMA_GENERAL_PURPOSE,
  MGA_DMA_VERTEX_TRIANGLE_LIST,
  MGA_DMA_VERTEX_TRIANGLE_STRIP,
  MGA_DMA_VERTEX_TRIANGLE_FAN
} mga_dma_buffer_type_t;

/* Returns a string for printing */
#define __MGA_DMA_BUFFER_TYPE_STRING(e,v,other) \
 (((e) == (v)) ? #v : (other))
#define MGA_DMA_BUFFER_TYPE_STRING(t)                             \
  __MGA_DMA_BUFFER_TYPE_STRING((t),MGA_DMA_GENERAL_PURPOSE,       \
  __MGA_DMA_BUFFER_TYPE_STRING((t),MGA_DMA_VERTEX_TRIANGLE_LIST,  \
  __MGA_DMA_BUFFER_TYPE_STRING((t),MGA_DMA_VERTEX_TRIANGLE_STRIP, \
  __MGA_DMA_BUFFER_TYPE_STRING((t),MGA_DMA_VERTEX_TRIANGLE_FAN,   \
  "unknown"))))

/* Buffer data structure */
typedef struct _mga_dma_buffer_s mga_dma_buffer_t;

#define MGA_DMA_BUFFER_MIN_SIZE 1

/* *********************************************
 *    Matrox raw DMA buffer
 * *********************************************/

/*
** Data structures
*/

/* Rank within a DMA block (1sr, 2nd, 3rd or 4th entry of the block)
 */
typedef enum
{
  MGA_DMA_FIRST_ENTRY = 0,
  MGA_DMA_SECOND_ENTRY = 1,
  MGA_DMA_THIRD_ENTRY = 2,
  MGA_DMA_FORTH_ENTRY = 3,
  MGA_DMA_NOMORE_ENTRY = 4
} _mga_dma_block_entry_t;
#define MGA_DMA_BLOCK_ENTRY_MASK 0x3

/* Size of a block in dwords */
#define _MGA_DMA_BLOCK_SIZE 5
/* Number of useful entries */
#define _MGA_DMA_BLOCK_ENTRIES_NUMBER 4

/* Returns a string for printing
 */
#define __MGA_DMA_BLOCK_ENTRY_STRING(e,v,other) \
 (((e) == (v)) ? #v : (other))
#define _MGA_DMA_BLOCK_ENTRY_STRING(e) \
  __MGA_DMA_BLOCK_ENTRY_STRING((e),MGA_DMA_FIRST_ENTRY, \
  __MGA_DMA_BLOCK_ENTRY_STRING((e),MGA_DMA_SECOND_ENTRY,\
  __MGA_DMA_BLOCK_ENTRY_STRING((e),MGA_DMA_THIRD_ENTRY, \
  __MGA_DMA_BLOCK_ENTRY_STRING((e),MGA_DMA_FORTH_ENTRY, \
  __MGA_DMA_BLOCK_ENTRY_STRING((e),MGA_DMA_NOMORE_ENTRY, \
  "unknown")))))

/*
 * Each DMA buffer contains an initial 32bits tag indicating its type.
 * A general DMA buffer is made of consecutive 5-dword blocks. In each
 * block, the initial dword contains 4 8bits register indexes, and the
 * next 4 dwords contain the values to store in these registers.
 * A vertex buffer is made of consecutive mga_vertex_t or mga_vertex2_t
 * data structures.
 */

typedef struct
{
  ggi_accel_t *accel;

  uint32_t dwgctl; /* Drawing command (DWGCTL reg) cached */
  uint32_t dstorg; /* Destination surface origin (write frame) cached */
  uint32_t srcorg; /* Source surface origin (read frame) cached */

  int hwgc_mask; /* Memorizes some differences between soft&hw gc */

  /* current buffer type */
  mga_dma_buffer_type_t type;

  /* current step in a regular DMA buffer */
  _mga_dma_block_entry_t entry;
  /* and the associated data stored temporarily */
  kgi_u32_t dma_regs;
  kgi_u32_t dma1;
  kgi_u32_t dma2;
  kgi_u32_t dma3;
  kgi_u32_t dma4;

} Gx00_context_t;

#define GX00_CONTEXT(vis) ((Gx00_context_t *)KGI_ACCEL_PRIV(vis))
#define GX00_ACCEL(vis) (GX00_CONTEXT(vis)->accel)

#define GX00_BUFFER_SIZE_ORDER  1
#define GX00_BUFFER_SIZE        (0x1000 << GX00_BUFFER_SIZE_ORDER)
#define GX00_BUFFER_MASK        (GX00_BUFFER_SIZE - 1)
#define GX00_BUFFER_SIZE32      (GX00_BUFFER_SIZE >> 2)
#define GX00_BUFFER_MASK32      (GX00_BUFFER_SIZE32 - 1)

#define GX00_BUFFER_NUM         2

#define GX00_TOTAL_SIZE         (GX00_BUFFER_SIZE * GX00_BUFFER_NUM)
#define GX00_TOTAL_SIZE32       (GX00_BUFFER_SIZE32 * GX00_BUFFER_NUM)

#define _GX00_FLUSH(vis) \
GGI_ACCEL_FLUSH_u32(GX00_ACCEL(vis), GX00_BUFFER_SIZE32, GX00_TOTAL_SIZE32)

#define GX00_CHECK(vis, n) \
GGI_ACCEL_CHECK_TOTAL_u32(GX00_ACCEL(vis), (n), GX00_BUFFER_SIZE32, GX00_TOTAL_SIZE32)

#define GX00_WRITE_u32(vis, val) \
GGI_ACCEL_WRITE_u32(GX00_ACCEL(vis), (val))


/*
** Clever DMA buffer manipulation functions
*/
#undef GX00_ACCEL_DEBUG
#undef GX00_ACCEL_DEBUG_2

/* Returns the number of available 32bits words in the
 * current buffer (remaining before flush).
 */
static inline uint32_t GX00_SPACE(struct ggi_visual *vis)
{
#ifdef GX00_ACCEL_DEBUG
  fprintf(stderr, "Remaining space: %i 32bits words\n",
	  (GX00_BUFFER_SIZE32 -
	   ((GX00_ACCEL(vis)->u32.current) & GX00_BUFFER_MASK32)));
#endif
  return (GX00_BUFFER_SIZE32 -
	  ((GX00_ACCEL(vis)->u32.current) & GX00_BUFFER_MASK32));
}

static inline void GX00_INIT(struct ggi_visual *vis)
{
  GX00_CONTEXT(vis)->type = MGA_DMA_GENERAL_PURPOSE;
  GX00_CONTEXT(vis)->entry = MGA_DMA_FIRST_ENTRY;
  GX00_CONTEXT(vis)->dma_regs = 0;
  GX00_WRITE_u32(vis, MGAG_ACCEL_TAG_DRAWING_ENGINE);
}

static inline void GX00_RESET(struct ggi_visual *vis, mga_dma_buffer_type_t type)
{
  _GX00_FLUSH(vis);
  switch (type)
    {
    case MGA_DMA_GENERAL_PURPOSE:
      /* tag the buffer */
      GX00_WRITE_u32(vis, MGAG_ACCEL_TAG_DRAWING_ENGINE);
      GX00_CONTEXT(vis)->entry = MGA_DMA_FIRST_ENTRY;
      break;
    case MGA_DMA_VERTEX_TRIANGLE_LIST:
      /* tag the buffer */
      GX00_WRITE_u32(vis, MGAG_ACCEL_TAG_WARP_TRIANGLE_LIST);
      break;
    default:
      ggPanic("Unknown reset buffer type (%i)", type);
      break;
    }
  GX00_CONTEXT(vis)->type = type;
}

static void _GX00_REGS_FINALIZE(struct ggi_visual *vis)
{
  if (GX00_CONTEXT(vis)->type != MGA_DMA_GENERAL_PURPOSE)
    ggPanic("Trying to flush regs to a non general purpose DMA!");

  /* Finish the output */
  switch (GX00_CONTEXT(vis)->entry)
    {
#if 1
    case MGA_DMA_FIRST_ENTRY:
      /* Nothing to finalize */
      break;
    case MGA_DMA_SECOND_ENTRY:
      /* If no more space is left for a block, do a flush
       * and a reset of the type
       */
      if (GX00_SPACE(vis) < 2)
	{
	  GX00_RESET(vis, MGA_DMA_GENERAL_PURPOSE);
	  /* Here we assume that there is more than 5 u32 per buffer :-) */
	}
#ifdef GX00_ACCEL_DEBUG_2
      fprintf(stderr,"Actual write: regs=%.8x dma1=%.8x",
	      GX00_CONTEXT(vis)->dma_regs,
	      GX00_CONTEXT(vis)->dma1);
#endif
      /* Now does the real transfer of the DMA block to the memory buffer */
      GX00_WRITE_u32(vis, GX00_CONTEXT(vis)->dma_regs
		     | MGA_DMA(DMAPAD) << 8
		     | MGA_DMA(DMAPAD) << 16
		     | MGA_DMA(DMAPAD) << 24);
      GX00_WRITE_u32(vis, GX00_CONTEXT(vis)->dma1);
      break;
    case MGA_DMA_THIRD_ENTRY:
      /* If no more space is left for a block, do a flush
       * and a reset of the type
       */
      if (GX00_SPACE(vis) < 3)
	{
	  GX00_RESET(vis, MGA_DMA_GENERAL_PURPOSE);
	  /* Here we assume that there is more than 5 u32 per buffer :-) */
	}
#ifdef GX00_ACCEL_DEBUG_2
      fprintf(stderr,"Actual write: regs=%.8x dma1=%.8x dma2=%.8x",
	      GX00_CONTEXT(vis)->dma_regs,
	      GX00_CONTEXT(vis)->dma1, GX00_CONTEXT(vis)->dma2);
#endif
      /* Now does the real transfer of the DMA block to the memory buffer */
      GX00_WRITE_u32(vis, GX00_CONTEXT(vis)->dma_regs
		     | MGA_DMA(DMAPAD) << 16
		     | MGA_DMA(DMAPAD) << 24);
      GX00_WRITE_u32(vis, GX00_CONTEXT(vis)->dma1);
      GX00_WRITE_u32(vis, GX00_CONTEXT(vis)->dma2);
      break;
    case MGA_DMA_FORTH_ENTRY:
      /* If no more space is left for a block, do a flush
       * and a reset of the type
       */
      if (GX00_SPACE(vis) < 4)
	{
	  GX00_RESET(vis, MGA_DMA_GENERAL_PURPOSE);
	  /* Here we assume that there is more than 5 u32 per buffer :-) */
	}
#ifdef GX00_ACCEL_DEBUG_2
      fprintf(stderr,"Actual write: regs=%.8x dma1=%.8x dma2=%.8x dma3=%.8x",
	      GX00_CONTEXT(vis)->dma_regs,
	      GX00_CONTEXT(vis)->dma1, GX00_CONTEXT(vis)->dma2,
	      GX00_CONTEXT(vis)->dma3);
#endif
      /* Now does the real transfer of the DMA block to the memory buffer */
      GX00_WRITE_u32(vis, GX00_CONTEXT(vis)->dma_regs
		     | MGA_DMA(DMAPAD) << 24);
      GX00_WRITE_u32(vis, GX00_CONTEXT(vis)->dma1);
      GX00_WRITE_u32(vis, GX00_CONTEXT(vis)->dma2);
      GX00_WRITE_u32(vis, GX00_CONTEXT(vis)->dma3);
      break;
#endif
    case MGA_DMA_NOMORE_ENTRY:
      /* If no more space is left for a block, do a flush
       * and a reset of the type
       */
      if (GX00_SPACE(vis) < 5)
	{
	  GX00_RESET(vis, MGA_DMA_GENERAL_PURPOSE);
	  /* Here we assume that there is more than 5 u32 per buffer :-) */
	}
#ifdef GX00_ACCEL_DEBUG_2
      fprintf(stderr,"Actual write: regs=%.8x dma1=%.8x dma2=%.8x dma3=%.8x"
	      " dma4=%.8x\n", GX00_CONTEXT(vis)->dma_regs,
	      GX00_CONTEXT(vis)->dma1, GX00_CONTEXT(vis)->dma2,
	      GX00_CONTEXT(vis)->dma3, GX00_CONTEXT(vis)->dma4);
#endif
      /* Now does the real transfer of the DMA block to the memory buffer */
      GX00_WRITE_u32(vis, GX00_CONTEXT(vis)->dma_regs);
      GX00_WRITE_u32(vis, GX00_CONTEXT(vis)->dma1);
      GX00_WRITE_u32(vis, GX00_CONTEXT(vis)->dma2);
      GX00_WRITE_u32(vis, GX00_CONTEXT(vis)->dma3);
      GX00_WRITE_u32(vis, GX00_CONTEXT(vis)->dma4);
      break;
    }
  /* Mandatory!: reinits the dma_regs */
  GX00_CONTEXT(vis)->dma_regs = 0;
}

static inline void GX00_FLUSH_START(struct ggi_visual *vis)
{
#ifdef GX00_ACCEL_DEBUG
  fprintf(stderr, "Starting an accel flush and reset\n");
#endif
  if (GX00_CONTEXT(vis)->type == MGA_DMA_GENERAL_PURPOSE)
    _GX00_REGS_FINALIZE(vis);

  GX00_RESET(vis, GX00_CONTEXT(vis)->type);
}

static void GX00_WRITE_REG(struct ggi_visual *vis, kgi_u32_t val, kgi_u32_t reg)
{
  if (GX00_CONTEXT(vis)->type != MGA_DMA_GENERAL_PURPOSE)
    ggPanic("Trying to write a register to a non general purpose DMA!");

#ifdef GX00_ACCEL_DEBUG_2
  fprintf(stderr, "Writing %.8x to reg %.8x reg\n", val, reg);
#endif

  /* Indicate that some sync is needed with the accel engine */
  vis->accelactive = 1;

  /* Update the index */
  switch (GX00_CONTEXT(vis)->entry)
    {
    case MGA_DMA_FIRST_ENTRY:
      GX00_CONTEXT(vis)->dma_regs = MGA_DMA(reg);
      GX00_CONTEXT(vis)->dma1 = val;
      GX00_CONTEXT(vis)->entry = MGA_DMA_SECOND_ENTRY;
      break;
    case MGA_DMA_SECOND_ENTRY:
      GX00_CONTEXT(vis)->dma_regs |= MGA_DMA(reg) << 8;
      GX00_CONTEXT(vis)->dma2 = val;
      GX00_CONTEXT(vis)->entry = MGA_DMA_THIRD_ENTRY;
      break;
    case MGA_DMA_THIRD_ENTRY:
      GX00_CONTEXT(vis)->dma_regs |= MGA_DMA(reg) << 16;
      GX00_CONTEXT(vis)->dma3 = val;
      GX00_CONTEXT(vis)->entry = MGA_DMA_FORTH_ENTRY;
      break;
    case MGA_DMA_FORTH_ENTRY:
      GX00_CONTEXT(vis)->dma_regs |= MGA_DMA(reg) << 24;
      GX00_CONTEXT(vis)->dma4 = val;
      GX00_CONTEXT(vis)->entry = MGA_DMA_NOMORE_ENTRY;
      _GX00_REGS_FINALIZE(vis);
      GX00_CONTEXT(vis)->entry = MGA_DMA_FIRST_ENTRY;
      break;
    case MGA_DMA_NOMORE_ENTRY:
      ggPanic("Incorrect DMA entry state!");
      break;
    }
}

/* Cached versions */
static inline void GX00_WRITE_DWGCTL(struct ggi_visual *vis, kgi_u32_t val)
{
  if (GX00_CONTEXT(vis)->dwgctl == val)
    return;
  GX00_CONTEXT(vis)->dwgctl = val;
  GX00_WRITE_REG(vis, val, DWGCTL);
}
static inline void GX00_WRITE_DSTORG(struct ggi_visual *vis, kgi_u32_t val)
{
  if (GX00_CONTEXT(vis)->dstorg == val)
    return;
  GX00_CONTEXT(vis)->dstorg = val;
  GX00_WRITE_REG(vis, val, DSTORG);
}
static inline void GX00_WRITE_SRCORG(struct ggi_visual *vis, kgi_u32_t val)
{
  if (GX00_CONTEXT(vis)->srcorg == val)
    return;
  GX00_CONTEXT(vis)->srcorg = val;
  GX00_WRITE_REG(vis, val, SRCORG);
}

static void GX00_WRITE_TRIANGLE(struct ggi_visual *vis,
				mga_vertex_t *v1,
				mga_vertex_t *v2,
				mga_vertex_t *v3)
{
  uint32_t i;
  uint32_t *ptr;

  if (GX00_CONTEXT(vis)->type != MGA_DMA_VERTEX_TRIANGLE_LIST)
    ggPanic("Trying to write a triangle to a non-triangle list DMA!");

  if (GX00_SPACE(vis) < (3 * (sizeof(mga_vertex_t)/4)))
    {
      GX00_FLUSH_START(vis);
      /* Here we assume that there is enough space for 3 vertices
       * per buffer :-) */
    }

  /* Indicate that some sync is needed with the accel engine */
  vis->accelactive = 1;

  ptr = (uint32_t*)v1;
  i = sizeof(mga_vertex_t) / 4;
  while (i--)
    GX00_WRITE_u32(vis, *ptr++);

  ptr = (uint32_t*)v2;
  i = sizeof(mga_vertex_t) / 4;
  while (i--)
    GX00_WRITE_u32(vis, *ptr++);

  ptr = (uint32_t*)v3;
  i = sizeof(mga_vertex_t) / 4;
  while (i--)
    GX00_WRITE_u32(vis, *ptr++);
}

/* Internal hardware-gc update function */
void GGI_kgi_Gx00_updatehwgc(struct ggi_visual *);

ggifunc_drawhline   GGI_kgi_Gx00_drawhline;
ggifunc_drawvline   GGI_kgi_Gx00_drawvline;
ggifunc_drawhline   GGI_kgi_Gx00_drawhline_nc;
ggifunc_drawvline   GGI_kgi_Gx00_drawvline_nc;
ggifunc_drawline    GGI_kgi_Gx00_drawline;
ggifunc_drawbox	    GGI_kgi_Gx00_drawbox;
ggifunc_drawbox     GGI_kgi_Gx00_drawbox_nc; /* used by {h,v}line_nc, fillscreen */
ggifunc_fillscreen  GGI_kgi_Gx00_fillscreen;
ggifunc_copybox     GGI_kgi_Gx00_copybox;
ggifunc_gcchanged   GGI_kgi_Gx00_gcchanged;
ggifunc_putc        GGI_kgi_Gx00_putc;
ggifunc_puts        GGI_kgi_Gx00_puts;
ggifunc_getcharsize GGI_kgi_Gx00_getcharsize;

#endif
