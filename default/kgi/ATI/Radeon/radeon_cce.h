/* $Id: radeon_cce.h,v 1.1 2002/10/31 03:20:17 redmondp Exp $
******************************************************************************

   ATI Radeon CCE packet structures

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
#ifndef _RADEON_CCE_H
#define _RADEON_CCE_H

typedef unsigned long bits;
typedef unsigned long dword;

typedef struct
{

	bits 	reserved  : 8, /* 0 by default */
		it_opcode : 8,
		count     : 14,
		type      : 2; /* must be 0x3 */

} cce_type3_header_t;

/* defines for cce_type3_header_t.it_opcode
*/
#define CCE_IT_OPCODE_NOP 			0x10
#define CCE_IT_OPCODE_PAINT 			0x91
#define CCE_IT_OPCODE_BITBLT			0x92
#define CCE_IT_OPCODE_SMALL_TEXT		0x93
#define CCE_IT_OPCODE_HOSTDATA_BLT 		0x94
#define CCE_IT_OPCODE_POLYLINE 			0x95
#define CCE_IT_OPCODE_POLYSCANLINES 		0x98
#define CCE_IT_OPCODE_NEXTCHAR 			0x19
#define CCE_IT_OPCODE_PLY_NEXTSCAN 		0x1D
#define CCE_IT_OPCODE_SET_SCISSORS 		0x1E
#define CCE_IT_OPCODE_PAINT_MULTI 		0x9A
#define CCE_IT_OPCODE_BITBLT_MULTI 		0x9B
#define CCE_IT_OPCODE_TRANS_BITBLT 		0x9C


typedef struct
{
	bits 	src_pitch_off       : 1,
		dst_pitch_off       : 1,
		src_clipping        : 1,
		dst_clipping        : 1,
		brush_type          : 4,
		dst_type            : 4,
		src_type            : 2,
		pix_order           : 1,
		color_convt         : 1,
		win31_rop           : 8,
		src_load            : 3,
		src_type3           : 1,
		gmc_clr_cmp_fcn_dis : 1,
		reserved            : 1,
		gmc_wr_msk_dis      : 1,
		brush_flag          : 1;
	
} cce_gui_control_t;

/* defines for cce_gui_control_t.win31_rop 
*/
#define ROP3_PATCOPY				0xF0
#define ROP3_SRCCOPY				0xCC

/* setup bodies (used depending on bits 0-7 or gui_control_t) 
*/
typedef struct
{

	bits	offset      : 22,
		pitch       : 8,
		tiled       : 1,
		microtiling : 1;

} cce_src_pitch_offset_t,
  cce_dst_pitch_offset_t;

typedef struct
{

	bits	right  : 14,
		dummy  : 2,
		bottom : 14;
	
} cce_src_sc_bot_rite_t;

typedef struct
{

	bits	left  : 14,
		dummy : 2,
		top   : 14;
	
} cce_sc_top_left_t;

typedef struct
{

	bits	right  : 14,
		dummy  : 2,
		bottom : 14;
	
} cce_sc_bot_rite_t;

typedef struct
{
	
	bits	x       : 5,
		dummy   : 3,
		y       : 5,
		dummy2  : 3,
		initial : 5;
	
} cce_brush_y_x_t;

typedef struct
{

	bits value : 32;
	
} cce_color_t;

/* brush packet structures for each BRUSH_TYPE */

typedef struct
{

	cce_color_t bkgrd_color, frgrd_color;
	dword mono_bmp_1, mono_bmp_2;

} cce_brush_packet_0_t;

typedef struct
{

	cce_color_t frgrd_color;
	dword mono_bmp_1, mono_bmp_2;
	      
} cce_brush_packet_1_t;

typedef struct
{

	cce_color_t bkgrd_color, frgrd_color;
	dword mono_bmp_1;
	      
} cce_brush_packet_6_t;

typedef struct
{

	cce_color_t frgrd_color;
	dword mono_bmp_1;
	      
} cce_brush_packet_7_t;

typedef struct
{
	
	dword color_bmp[16];
	
} cce_brush_packet_10_8bpp_t;

typedef struct
{

	dword color_bmp[16*2];

} cce_brush_packet_10_16bpp_t;

typedef struct
{

	dword color_bmp[16*4];
	
} cce_brush_packet_10_24bpp_t, 
  cce_brush_packet_10_32bpp_t;

typedef struct
{

	cce_color_t frgrd_color;

} cce_brush_packet_13_t, 
  cce_brush_packet_14_t;

/* DATA_BLOCK structures */

typedef struct
{

	bits left   : 16,
	     top    : 16,
	     right  : 16,
	     bottom : 16;

} cce_paint_t;

typedef struct
{

	bits x0 : 16,
	     y0 : 16,
	     x1 : 16,
	     y1 : 16;

} cce_polyline_t;

typedef struct
{

	bits src_y1 : 16,
	     src_x1 : 16,
	     dst_y1 : 16,
	     dst_x1 : 16,
	     src_h1 : 14,
	     dummy  : 2,
	     src_w1 : 14;
	     
} cce_bitblt_t;

#endif