/* $Id: mach64_accel.h,v 1.4 2005/01/23 21:57:19 nsouch Exp $
******************************************************************************

   ATI Mach64 sublib function prototypes

   Copyright (C) 2002 Paul Redmond

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
#ifndef _MACH64LIB_H
#define _MACH64LIB_H

#include <ggi/internal/ggi-dl.h>
#include <ggi/internal/ggi_debug.h>

#include "kgi/config.h"
#include <ggi/display/kgi.h>

/* !! The underlying constants are taken from the MACH64.h
 * !! definitions of the graphic driver.
 */

/*
 * Userspace processes should use the index itself as that is what
 * the DMA engine expects. No bank translation.
 */
#define MM(r)   r
#define UNMM(r) r

/* Draw Engine Destination Trajectory */
#define MACH64_DST_OFF_PITCH                  MM(0x40)
#define   MACH64_40_DST_OFFSETMask            0x000fffff
#define   MACH64_40_DST_OFFSETShift                    0
#define   MACH64_40_DST_PITCHMask             0xffc00000
#define   MACH64_40_DST_PITCHShift                    22
#define MACH64_DST_X                          MM(0x41)
#define   MACH64_41_DST_XMask                 0x00001fff
#define   MACH64_41_DST_XShift                         0
#define MACH64_DST_Y                          MM(0x42)
#define   MACH64_42_DST_YMask                 0x00007fff
#define   MACH64_42_DST_YShift                         0
#define MACH64_DST_Y_X                        MM(0x43)
#define   MACH64_43_DST_YMask                 0x00007fff
#define   MACH64_43_DST_YShift                         0
#define   MACH64_43_DST_XMask                 0x1fff0000
#define   MACH64_43_DST_XShift                        16
#define MACH64_DST_WIDTH                      MM(0x44)
#define   MACH64_44_DST_WIDTHMask             0x00001fff /*FIXME Diff on IIC */
#define   MACH64_44_DST_WIDTHShift                     0
#define MACH64_DST_HEIGHT                     MM(0x45)
#define   MACH64_45_DST_HEIGHTMask            0x00007fff
#define   MACH64_45_DST_HEIGHTShift                    0
#define MACH64_DST_HEIGHT_WIDTH               MM(0x46)
#define   MACH64_46_DST_HEIGHTMask            0x00007fff
#define   MACH64_46_DST_HEIGHTShift                    0
#define   MACH64_46_DST_WIDTHMask             0x1fff0000
#define   MACH64_46_DST_WIDTHShift                    16
#define MACH64_DST_X_WIDTH                    MM(0x47)
#define   MACH64_47_DST_XMask                 0x00001fff
#define   MACH64_47_DST_XShift                         0
#define   MACH64_47_DST_WIDTHMask             0x1fff0000
#define   MACH64_47_DST_WIDTHShift                    16
#define MACH64_DST_BRES_LNTH                  MM(0x48)
#define   MACH64_48_DST_BRES_LNTHMask         0x00007fff
#define   MACH64_48_DST_BRES_LNTHShift                 0
#define   MACH64_48_DRAW_TRAP                 0x00008000
#define   MACH64_48_TRAIL_XMask               0x1fff0000
#define   MACH64_48_TRAIL_XShift                      16
#define   MACH64_48_DST_BRES_LNTH_LINE_DIS    0x80000000
#define MACH64_DST_BRES_ERR                   MM(0x49)
#define   MACH64_49_DST_BRES_ERRMask          0x0003ffff
#define   MACH64_49_DST_BRES_ERRShift                  0
#define MACH64_DST_BRES_INC                   MM(0x4A)
#define   MACH64_4A_DST_BRES_INCMask          0x0003ffff
#define   MACH64_4A_DST_BRES_INCShift                  0
#define MACH64_DST_BRES_DEC                   MM(0x4B)
#define   MACH64_4B_DST_BRES_DECMask          0x0003ffff
#define   MACH64_4B_DST_BRES_DECShift                  0
#define MACH64_DST_CNTL                       MM(0x4C)
#define   MACH64_4C_DST_X_DIR                 0x00000001
#define   MACH64_4C_DST_Y_DIR                 0x00000002
#define   MACH64_4C_DST_Y_MAJOR               0x00000004
#define   MACH64_4C_DST_X_TILE                0x00000008
#define   MACH64_4C_DST_Y_TILE                0x00000010
#define   MACH64_4C_DST_LAST_PEL              0x00000020
#define   MACH64_4C_DST_POLYGON_EN            0x00000040
#define   MACH64_4C_DST_24_ROT_EN             0x00000080
#define   MACH64_4C_DST_24_ROTMask            0x00000700
#define   MACH64_4C_DST_24_ROTShift                    8
#define   MACH64_4C_DST_BRES_SIGN             0x00000800
#define   MACH64_4C_DST_POLYGON_RTEDGE_DIS    0x00001000
#define   MACH64_4C_TRAIL_X_DIR               0x00002000
#define   MACH64_4C_TRAIL_FILL_DIR            0x00004000
#define   MACH64_4C_TRAIL_BRES_SIGN           0x00008000
#define   MACH64_4C_BRES_SIGN_AUTO            0x00020000
#define MACH64_TRAIL_BRES_ERR                 MM(0x4E)
#define   MACH64_4E_TRAIL_BRES_ERRMask        0x0003ffff
#define   MACH64_4E_TRAIL_BRES_ERRShift                0
#define MACH64_TRAIL_BRES_INC                 MM(0x4F)
#define   MACH64_4F_TRAIL_BRES_INCMask        0x0003ffff
#define   MACH64_4F_TRAIL_BRES_INCShift                0
#define MACH64_TRAIL_BRES_DEC                 MM(0x50)
#define   MACH64_50_TRAIL_BRES_DECMask        0x0003ffff
#define   MACH64_50_TRAIL_BRES_DECShift                0
#define MACH64_Z_OFF_PITCH                    MM(0x52)
#define   MACH64_52_Z_OFFSETMask              0x000fffff
#define   MACH64_52_Z_OFFSETShift                      0
#define   MACH64_52_Z_PITCHMask               0xffc00000
#define   MACH64_52_Z_PITCHShift                      22
#define MACH64_Z_CNTL                         MM(0x53)
#define   MACH64_53_Z_EN                      0x00000001
#define   MACH64_53_Z_SRC                     0x00000002
#define   MACH64_53_Z_TESTMask                0x00000070
#define   MACH64_53_Z_TESTShift                        4
#define   MACH64_53_Z_MASK                    0x00000100

/* Draw Engine Source Trajectory */
#define MACH64_SRC_OFF_PITCH                  MM(0x60)
#define   MACH64_60_SRC_OFFSETMask            0x000fffff
#define   MACH64_60_SRC_OFFSETShift                    0
#define   MACH64_60_SRC_PITCHMask             0xffc00000
#define   MACH64_60_SRC_PITCHShift                    22
#define MACH64_SRC_X                          MM(0x61)
#define   MACH64_61_SRC_XMask                 0x00001fff
#define   MACH64_61_SRC_XShift                         0
#define MACH64_SRC_Y                          MM(0x62)
#define   MACH64_62_SRC_YMask                 0x00007fff
#define   MACH64_62_SRC_YShift                         0
#define MACH64_SRC_Y_X                        MM(0x63)
#define   MACH64_63_SRC_YMask                 0x00007fff
#define   MACH64_63_SRC_YShift                         0
#define   MACH64_63_SRC_XMask                 0x1fff0000
#define   MACH64_63_SRC_XShift                        16
#define MACH64_SRC_WIDTH1                     MM(0x64)
#define   MACH64_64_SRC_WIDTH1Mask            0x00001fff
#define   MACH64_64_SRC_WIDTH1Shift                    0
#define MACH64_SRC_HEIGHT1                    MM(0x65)
#define   MACH64_65_SRC_HEIGHT1Mask           0x00007fff
#define   MACH64_65_SRC_HEIGHT1Shift                   0
#define MACH64_SRC_HEIGHT1_WIDTH1             MM(0x66)
#define   MACH64_66_SRC_HEIGHT1Mask           0x00007fff
#define   MACH64_66_SRC_HEIGHT1Shift                   0
#define   MACH64_66_SRC_WIDTH1Mask            0x1fff0000
#define   MACH64_66_SRC_WIDTH1Shift                   16
#define MACH64_SRC_X_START                    MM(0x67)
#define   MACH64_67_SRC_X_STARTMask           0x00001fff
#define   MACH64_67_SRC_X_STARTShift                   0
#define MACH64_SRC_Y_START                    MM(0x68)
#define   MACH64_68_SRC_Y_STARTMask           0x00007fff
#define   MACH64_68_SRC_Y_STARTShift                   0
#define MACH64_SRC_Y_X_START                  MM(0x69)
#define   MACH64_69_SRC_Y_STARTMask           0x00007fff
#define   MACH64_69_SRC_Y_STARTShift                   0
#define   MACH64_69_SRC_X_STARTMask           0x1fff0000
#define   MACH64_69_SRC_X_STARTShift                  16
#define MACH64_SRC_WIDTH2                     MM(0x6A)
#define   MACH64_6A_SRC_WIDTH2Mask            0x00001fff
#define   MACH64_6A_SRC_WIDTH2Shift                    0
#define MACH64_SRC_HEIGHT2                    MM(0x6B)
#define   MACH64_6B_SRC_HEIGHT2Mask           0x00007fff
#define   MACH64_6B_SRC_HEIGHT2Shift                   0
#define MACH64_SRC_HEIGHT2_WIDTH2             MM(0x6C)
#define   MACH64_6C_SRC_HEIGHT2Mask           0x00007fff
#define   MACH64_6C_SRC_HEIGHT2Shift                   0
#define   MACH64_6C_SRC_WIDTH2Mask            0x1fff0000
#define   MACH64_6C_SRC_WIDTH2Shift                   16
#define MACH64_SRC_CNTL                       MM(0x6D)
#define   MACH64_6D_SRC_PATT_EN               0x00000001
#define   MACH64_6D_SRC_PATT_ROT_EN           0x00000002
#define   MACH64_6D_SRC_LINEAR_EN             0x00000004
#define   MACH64_6D_SRC_BYTE_ALIGN            0x00000008
#define   MACH64_6D_SRC_LINE_X_DIR            0x00000010
#define   MACH64_6D_SRC_8x8x8_BRUSH           0x00000020
#define   MACH64_6D_FAST_FILL_EN              0x00000040
#define   MACH64_6D_SRC_TRACK_DST_EN          0x00000080
#define   MACH64_6D_BUS_MASTER_EN             0x00000100
#define   MACH64_6D_BUS_MASTER_SYNC           0x00000200
#define   MACH64_6D_BUS_MASTER_OPMask         0x00000c00
#define   MACH64_6D_BUS_MASTER_OPShift                10
#define   MACH64_6D_SRC_8x8x8_BRUSH_LOADED    0x00001000
#define   MACH64_6D_COLOR_REG_WRITE_EN        0x00002000
#define   MACH64_6D_BLOCK_WRITE_EN            0x00004000

/* Host Data */
#define MACH64_HOST_CNTL                      MM(0x90)
#define   MACH64_90_HOST_BYTE_ALIGN           0x00000001
#define   MACH64_90_HOST_BIG_ENDIAN_EN        0x00000002

/* Pattern */
#define MACH64_PAT_REG0                       MM(0xA0)
#define MACH64_PAT_REG1                       MM(0xA1)
#define MACH64_PAT_CNTL                       MM(0xA2)
#define   MACH64_A2_PAT_MONO_EN               0x00000001
#define   MACH64_A2_PAT_CLR_4x2_EN            0x00000002
#define   MACH64_A2_PAT_CLR_8x1_EN            0x00000004

/* Scissors */
#define MACH64_SC_LEFT                        MM(0xA8)
#define   MACH64_A8_SC_LEFTMask               0x00001fff
#define   MACH64_A8_SC_LEFTShift                       0
#define MACH64_SC_RIGHT                       MM(0xA9)
#define   MACH64_A9_SC_RIGHTMask              0x00001fff
#define   MACH64_A9_SC_RIGHTShift                      0
#define MACH64_SC_LEFT_RIGHT                  MM(0xAA)
#define   MACH64_AA_SC_LEFTMask               0x00001fff
#define   MACH64_AA_SC_LEFTShift                       0
#define   MACH64_AA_SC_RIGHTMask              0x1fff0000
#define   MACH64_AA_SC_RIGHTShift                     16
#define MACH64_SC_TOP                         MM(0xAB)
#define   MACH64_AB_SC_TOPMask                0x00007fff
#define   MACH64_AB_SC_TOPShift                        0
#define MACH64_SC_BOTTOM                      MM(0xAC)
#define   MACH64_AC_SC_BOTTOMMask             0x00007fff
#define   MACH64_AC_SC_BOTTOMShift                     0
#define MACH64_SC_TOP_BOTTOM                  MM(0xAD)
#define   MACH64_AD_SC_TOPMask                0x00007fff
#define   MACH64_AD_SC_TOPShift                        0
#define   MACH64_AD_SC_BOTTOMMask             0x7fff0000
#define   MACH64_AD_SC_BOTTOMShift                    16

/* Data Path */
#define MACH64_DP_BKGD_CLR                    MM(0xB0)
#define MACH64_DP_FRGD_CLR                    MM(0xB1)
#define MACH64_DP_WRITE_MASK                  MM(0xB2)
#define MACH64_DP_PIX_WIDTH                   MM(0xB4)
#define   MACH64_B4_DP_DST_PIX_WIDTHMask      0x0000000f
#define   MACH64_B4_DP_DST_PIX_WIDTHShift              0
#define   MACH64_B4_DP_SRC_PIX_WIDTHMask      0x00000f00
#define   MACH64_B4_DP_SRC_PIX_WIDTHShift              8
#define   MACH64_B4_DP_HOST_TRIPLE_EN         0x00002000
#define   MACH64_B4_DP_SRC_AUTONA_FIX_DIS     0x00004000
#define   MACH64_B4_DP_FAST_SRCCOPY_DIS       0x00008000
#define   MACH64_B4_DP_HOST_PIX_WIDTHMask     0x000f0000
#define   MACH64_B4_DP_HOST_PIX_WIDTHShift            16
#define   MACH64_B4_DP_CI4_RGB_INDEXMask      0x00f00000
#define   MACH64_B4_DP_CI4_RGB_INDEXShift             20
#define   MACH64_B4_DP_BYTE_PIX_ORDER         0x01000000
#define   MACH64_B4_DP_CONVERSION_TEMP        0x02000000
#define   MACH64_B4_DP_CI4_RGB_LOW_NIBBLE     0x04000000
#define   MACH64_B4_DP_CI4_RGB_HIGH_NIBBLE    0x08000000
#define   MACH64_B4_DP_SCALE_PIX_WIDTHMask    0xf0000000
#define   MACH64_B4_DP_SCALE_PIX_WIDTHShift           28
#define MACH64_DP_MIX                         MM(0xB5)
#define   MACH64_B5_DP_BKGD_MIXMask           0x0000001f
#define   MACH64_B5_DP_BKGD_MIXShift                   0
#define   MACH64_B5_BKGD_MIX_NOT_D            0x00000000
#define   MACH64_B5_BKGD_MIX_ZERO             0x00000001
#define   MACH64_B5_BKGD_MIX_ONE              0x00000002
#define   MACH64_B5_BKGD_MIX_D                0x00000003
#define   MACH64_B5_BKGD_MIX_NOT_S            0x00000004
#define   MACH64_B5_BKGD_MIX_D_XOR_S          0x00000005
#define   MACH64_B5_BKGD_MIX_NOT_D_XOR_S      0x00000006
#define   MACH64_B5_BKGD_MIX_S                0x00000007
#define   MACH64_B5_BKGD_MIX_NOT_D_OR_NOT_S   0x00000008
#define   MACH64_B5_BKGD_MIX_D_OR_NOT_S       0x00000009
#define   MACH64_B5_BKGD_MIX_NOT_D_OR_S       0x0000000a
#define   MACH64_B5_BKGD_MIX_D_OR_S           0x0000000b
#define   MACH64_B5_BKGD_MIX_D_AND_S          0x0000000c
#define   MACH64_B5_BKGD_MIX_NOT_D_AND_S      0x0000000d
#define   MACH64_B5_BKGD_MIX_D_AND_NOT_S      0x0000000e
#define   MACH64_B5_BKGD_MIX_NOT_D_AND_NOT_S  0x0000000f
#define   MACH64_B5_BKGD_MIX_D_PLUS_S_DIV2    0x00000017
#define   MACH64_B5_DP_FRGD_MIXMask           0x001f0000
#define   MACH64_B5_DP_FRGD_MIXShift                  16
#define   MACH64_B5_FRGD_MIX_NOT_D            0x00000000
#define   MACH64_B5_FRGD_MIX_ZERO             0x00010000
#define   MACH64_B5_FRGD_MIX_ONE              0x00020000
#define   MACH64_B5_FRGD_MIX_D                0x00030000
#define   MACH64_B5_FRGD_MIX_NOT_S            0x00040000
#define   MACH64_B5_FRGD_MIX_D_XOR_S          0x00050000
#define   MACH64_B5_FRGD_MIX_NOT_D_XOR_S      0x00060000
#define   MACH64_B5_FRGD_MIX_S                0x00070000
#define   MACH64_B5_FRGD_MIX_NOT_D_OR_NOT_S   0x00080000
#define   MACH64_B5_FRGD_MIX_D_OR_NOT_S       0x00090000
#define   MACH64_B5_FRGD_MIX_NOT_D_OR_S       0x000a0000
#define   MACH64_B5_FRGD_MIX_D_OR_S           0x000b0000
#define   MACH64_B5_FRGD_MIX_D_AND_S          0x000c0000
#define   MACH64_B5_FRGD_MIX_NOT_D_AND_S      0x000d0000
#define   MACH64_B5_FRGD_MIX_D_AND_NOT_S      0x000e0000
#define   MACH64_B5_FRGD_MIX_NOT_D_AND_NOT_S  0x000f0000
#define   MACH64_B5_FRGD_MIX_D_PLUS_S_DIV2    0x00170000
#define MACH64_DP_SRC                         MM(0xB6)
#define   MACH64_B6_DP_BKGD_SRCMask           0x00000007
#define   MACH64_B6_DP_BKGD_SRCShift                   0
#define   MACH64_B6_BKGD_SRC_BKGD_CLR         0x00000000
#define   MACH64_B6_BKGD_SRC_FRGD_CLR         0x00000001
#define   MACH64_B6_BKGD_SRC_HOST             0x00000002
#define   MACH64_B6_BKGD_SRC_BLIT             0x00000003
#define   MACH64_B6_BKGD_SRC_PATTERN          0x00000004
#define   MACH64_B6_BKGD_SRC_3D               0x00000005
#define   MACH64_B6_DP_FRGD_SRCMask           0x00000700
#define   MACH64_B6_DP_FRGD_SRCShift                   8
#define   MACH64_B6_FRGD_SRC_BKGD_CLR         0x00000000
#define   MACH64_B6_FRGD_SRC_FRGD_CLR         0x00000100
#define   MACH64_B6_FRGD_SRC_HOST             0x00000200
#define   MACH64_B6_FRGD_SRC_BLIT             0x00000300
#define   MACH64_B6_FRGD_SRC_PATTERN          0x00000400
#define   MACH64_B6_FRGD_SRC_3D               0x00000500
#define   MACH64_B6_DP_MONO_SRCMask           0x00030000
#define   MACH64_B6_DP_MONO_SRCShift                  16
#define   MACH64_B6_MONO_SRC_ONE              0x00000000
#define   MACH64_B6_MONO_SRC_PATTERN          0x00010000
#define   MACH64_B6_MONO_SRC_HOST             0x00020000
#define   MACH64_B6_MONO_SRC_BLIT             0x00030000
#define MACH64_DP_FRGD_CLR_MIX                MM(0xB7)
#define   MACH64_B7_DP_FRGD_CLRMask           0x0000ffff
#define   MACH64_B7_DP_FRGD_CLRShift                   0
#define   MACH64_B7_DP_FRGD_MIXMask           0x001f0000
#define   MACH64_B7_DP_FRGD_MIXShift                  16
#define   MACH64_B7_DP_BKGD_MIXMask           0x1f000000
#define   MACH64_B7_DP_BKGD_MIXShift                  24
#define MACH64_DP_FRGD_BKGD_CLR               MM(0xB8)
#define   MACH64_B8_DP_FRGD_CLRMask           0x0000ffff
#define   MACH64_B8_DP_FRGD_CRLShift                   0
#define   MACH64_B8_DP_BKGD_CLRMask           0xffff0000
#define   MACH64_B8_DP_BKGD_CLRShift                  16

/* Draw Engine Destination Trajectory */
#define MACH64_DST_X_Y                        MM(0xBA)
#define   MACH64_BA_DST_XMask                 0x00001fff
#define   MACH64_BA_DST_XShift                         0
#define   MACH64_BA_DST_YMask                 0x1fff0000
#define   MACH64_BA_DST_YShift                        16
#define MACH64_DST_WIDTH_HEIGHT               MM(0xBB)
#define   MACH64_BB_DST_WIDTHMask             0x00001fff
#define   MACH64_BB_DST_WIDTHShift                     0
#define   MACH64_BB_DST_HEIGHTMask            0xffff0000
#define   MACH64_BB_DST_HEIGHTShift                   16

#define MACH64_ACCEL(vis) ((ggi_accel_t *)KGI_ACCEL_PRIV(vis))

#define MACH64_BUFFER_SIZE_ORDER  1
#define MACH64_BUFFER_SIZE        (0x1000 << MACH64_BUFFER_SIZE_ORDER)
#define MACH64_BUFFER_MASK        (MACH64_BUFFER_SIZE - 1)
#define MACH64_BUFFER_SIZE32      (MACH64_BUFFER_SIZE >> 2)
#define MACH64_BUFFER_MASK32      (MACH64_BUFFER_SIZE32 - 1)

#define MACH64_BUFFER_NUM         16

#define MACH64_TOTAL_SIZE         (MACH64_BUFFER_SIZE * MACH64_BUFFER_NUM)
#define MACH64_TOTAL_SIZE32       (MACH64_BUFFER_SIZE32 * MACH64_BUFFER_NUM)

#define MACH64_FLUSH(vis) \
GGI_ACCEL_FLUSH_u32(MACH64_ACCEL(vis), MACH64_BUFFER_SIZE32, MACH64_TOTAL_SIZE32)

#define MACH64_CHECK(vis, n) \
GGI_ACCEL_CHECK_TOTAL_u32(MACH64_ACCEL(vis), n, MACH64_BUFFER_SIZE32, MACH64_TOTAL_SIZE32)

#define MACH64_WRITE(vis, val) \
GGI_ACCEL_WRITE_u32(MACH64_ACCEL(vis), val)

ggifunc_drawhline GGI_kgi_mach64_drawhline;
ggifunc_drawvline GGI_kgi_mach64_drawvline;
ggifunc_drawline  GGI_kgi_mach64_drawline;
ggifunc_drawbox	  GGI_kgi_mach64_drawbox;
ggifunc_copybox   GGI_kgi_mach64_copybox;
ggifunc_gcchanged GGI_kgi_mach64_gcchanged;

#endif
