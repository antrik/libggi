/* $Id: radeon_cce.h,v 1.6 2005/07/30 11:39:59 cegger Exp $
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

typedef uint32_t bits;
typedef uint32_t dword;

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
#define CCE_IT_OPCODE_3D_DRAW_IMMD 		0x29


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

} cce_pitch_offset_t;

typedef struct
{

	bits	x      : 14,
		dummy  : 2,
		y      : 14;
	
} cce_scissor_t;

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

typedef struct
{

	dword frgd_color;
	bits bas_x : 16,
	     bas_y : 16;

} cce_smalltext_t;

typedef struct 
{

	bits dx : 8,
	     dy : 8,
	     w  : 8,
	     h  : 8;
	/* using ascii fixed 8x8 mono bitmap */
	dword raster1,
	      raster2;
} cce_smallchar_t;


typedef struct
{
	bits w0 : 1,
	     fpcolor : 1,
	     fpalpha : 1,
	     pkcolor : 1,
	     fpspec : 1,
	     fpfog : 1,
	     pkspec : 1,
	     st0 : 1,
	     st1 : 1,
	     q1 : 1,
	     st2: 1,
	     q2 : 1,
	     st3 : 1,
	     q3 : 1,
	     q0 : 1,
	     blnd_weight_cnt : 3,
	     n0 : 1,
	     pad: 8,
	     xy1: 1,
	     z1 : 1,
	     w1 : 1,
	     n1 : 1,
	     z : 1;
} cce_se_se_vtx_fmt_t;

typedef struct {
        bits prim_type : 4,
	     prim_walk : 2,
	     color_order : 1,
	     en_maos : 1,
	     fmt_mode : 1,
	     pad : 7,
	     num_vertices : 16;
} cce_se_se_vf_cntl_t;


/* CCE type 0 packets */

typedef struct
{
	bits 	base_index  : 15,
		one_reg_wr : 1,
		count : 14,
		type : 2; /* must be 0x0 */
} cce_type0_header_t;

typedef struct
{
	bits	txformat : 5,
		apple_yuv : 1,
		alpha_enable : 1,
		non_power2 : 1,
		txwidth : 4,
		txheight : 4,
		face_width_5 : 4,
		face_height_5 : 4,
		st_route : 2,
		endian_swap : 2,
	        alpha_mask_enable : 1,
		chroma_key_enable : 1,
		cubic_map_enable : 1,
		perspective_enable : 1;
} pp_txformat_t;

typedef struct
{
	bits	usize : 11,
		pad : 5,
	        vsize : 11,
		pad2 : 5;
} pp_tex_size_t;

typedef struct
{
	bits	pad : 5,
                txpitch : 11,
		pad2 : 16;
} pp_txpitch_t;



#define CRTC_OFFSET 0x224

/****************** Registers used by 2D engine CCE0 *******************/

#define DEFAULT_PITCH_OFFSET 0x16e0
#define DEFAULT_SC_BOT_RIGHT 0x16e8

/****************** Registers used by 3D engine CCE0 *******************/

#define PP_TXFORMAT_1 0x1c70 /* also 0x2c70 */
#define PP_TXOFFSET_1 0x1c74 /* also 0x2c74 */
#define PP_TXCBLEND_1 0x1c78 /* also 0x1c78 */
#define PP_TXABLEND_1 0x1c7c /* also 0x1c7c */
#define PP_TEX_SIZE_1 0x1d0c /* also 0x2d0c */
#define PP_CNTL 0x1c38 /* also 0x2c38 */

/* X_LEFT 10:0 Y_TOP 26:16 */
#define RE_TOP_LEFT 0x26c0 

/* WIDTH 10:0 (minus 1) HEIGHT 26:16 (minus 1) */
#define RE_WIDTH_HEIGHT 0x1c44 /* also 0x2644 */

#define RE_SOLID_COLOR 0x1c1c /* also 0x261c */
#define SRC_CLUT_ADDRESS 0x1780
#define SRC_CLUT_DATA_W  0x1784
#define PP_TFACTOR_1 0x1c80 /* also 0x2c80  This is used for GC bgcolor. */
/* Above two organized as a8r8g8b8 */
#define SE_CNTL 0x1c4c /* also 0x2088 */
#define SE_CNTL_STATUS 0x2140
#define SE_TCL_UCP_VERT_BLEND_CTL 0x2264
#define SE_TCL_TEXTURE_PROC_CTL 0x2268
#define RB3D_COLOROFFSET 0x1c40 /* also 0x3240 */
#define RB3D_COLORPITCH  0x1c48 /* also 0x3248 */
/* Use bits 12 to 3, others used for tiling/endian, 0,1,2 are set 0 */
#define RB3D_CNTL 0x1c3c /* also 0x323C */
/* Z_ENABLE bit 8, depthxy_offset_enable bit 9, colorformat bit 13:10 as so:

3=ARGB1555
4=RGB565
6=ARGB8888
7=RGB332
8=Y8
9=RGB8
11=YUV422 packed (VYUY)
12=YUV422 packed (YVYU)
14=aYUV444
15=ARGB4444 

*/



#endif
