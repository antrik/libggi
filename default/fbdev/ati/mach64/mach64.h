/*
 * ATI Mach64 Register Definitions
 *
 * Copyright (C) 1997 Michael AK Tesch
 *  written with much help from Jon Howell
 *
 * Updated for 3D RAGE PRO and 3D RAGE Mobility by Geert Uytterhoeven
 *	
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version
 * 2 of the License, or (at your option) any later version.
 */

/*
 * most of the rest of this file comes from ATI sample code
 */
#ifndef REGMACH64_H
#define REGMACH64_H

/* NON-GUI MEMORY MAPPED Registers - expressed in BYTE offsets */

/* Accelerator CRTC */
#define CRTC_H_TOTAL_DISP	0x0000U	/* Dword offset 0_00 */
#define CRTC2_H_TOTAL_DISP	0x0000U	/* Dword offset 0_00 */
#define CRTC_H_SYNC_STRT_WID	0x0004U	/* Dword offset 0_01 */
#define CRTC2_H_SYNC_STRT_WID	0x0004U	/* Dword offset 0_01 */
#define CRTC_H_SYNC_STRT	0x0004U
#define CRTC2_H_SYNC_STRT	0x0004U
#define CRTC_H_SYNC_DLY		0x0005U
#define CRTC2_H_SYNC_DLY	0x0005U
#define CRTC_H_SYNC_WID		0x0006U
#define CRTC2_H_SYNC_WID	0x0006U
#define CRTC_V_TOTAL_DISP	0x0008U	/* Dword offset 0_02 */
#define CRTC2_V_TOTAL_DISP	0x0008U	/* Dword offset 0_02 */
#define CRTC_V_TOTAL		0x0008U
#define CRTC2_V_TOTAL		0x0008U
#define CRTC_V_DISP		0x000AU
#define CRTC2_V_DISP		0x000AU
#define CRTC_V_SYNC_STRT_WID	0x000CU	/* Dword offset 0_03 */
#define CRTC2_V_SYNC_STRT_WID	0x000CU	/* Dword offset 0_03 */
#define CRTC_V_SYNC_STRT	0x000CU
#define CRTC2_V_SYNC_STRT	0x000CU
#define CRTC_V_SYNC_WID		0x000EU
#define CRTC2_V_SYNC_WID	0x000EU
#define CRTC_VLINE_CRNT_VLINE	0x0010U	/* Dword offset 0_04 */
#define CRTC2_VLINE_CRNT_VLINE	0x0010U	/* Dword offset 0_04 */
#define CRTC_OFF_PITCH		0x0014U	/* Dword offset 0_05 */
#define CRTC_OFFSET		0x0014U
#define CRTC_PITCH		0x0016U
#define CRTC_INT_CNTL		0x0018U	/* Dword offset 0_06 */
#define CRTC_GEN_CNTL		0x001CU	/* Dword offset 0_07 */
#define CRTC_PIX_WIDTH		0x001DU
#define CRTC_FIFO		0x001EU
#define CRTC_EXT_DISP		0x001FU

/* Memory Buffer Control */
#define DSP_CONFIG		0x0020U	/* Dword offset 0_08 */
#define PM_DSP_CONFIG		0x0020U	/* Dword offset 0_08 (Mobility Only) */
#define DSP_ON_OFF		0x0024U	/* Dword offset 0_09 */
#define PM_DSP_ON_OFF		0x0024U	/* Dword offset 0_09 (Mobility Only) */
#define TIMER_CONFIG		0x0028U	/* Dword offset 0_0A */
#define MEM_BUF_CNTL		0x002CU	/* Dword offset 0_0B */
#define MEM_ADDR_CONFIG		0x0034U	/* Dword offset 0_0D */

/* Accelerator CRTC */
#define CRT_TRAP		0x0038U	/* Dword offset 0_0E */

#define I2C_CNTL_0		0x003CU	/* Dword offset 0_0F */

/* Overscan */
#define OVR_CLR			0x0040U	/* Dword offset 0_10 */
#define OVR2_CLR		0x0040U	/* Dword offset 0_10 */
#define OVR_WID_LEFT_RIGHT	0x0044U	/* Dword offset 0_11 */
#define OVR2_WID_LEFT_RIGHT	0x0044U	/* Dword offset 0_11 */
#define OVR_WID_TOP_BOTTOM	0x0048U	/* Dword offset 0_12 */
#define OVR2_WID_TOP_BOTTOM	0x0048U	/* Dword offset 0_12 */

/* Memory Buffer Control */
#define VGA_DSP_CONFIG		0x004CU	/* Dword offset 0_13 */
#define PM_VGA_DSP_CONFIG	0x004CU	/* Dword offset 0_13 (Mobility Only) */
#define VGA_DSP_ON_OFF		0x0050U	/* Dword offset 0_14 */
#define PM_VGA_DSP_ON_OFF	0x0050U	/* Dword offset 0_14 (Mobility Only) */
#define DSP2_CONFIG		0x0054U	/* Dword offset 0_15 */
#define PM_DSP2_CONFIG		0x0054U	/* Dword offset 0_15 (Mobility Only) */
#define DSP2_ON_OFF		0x0058U	/* Dword offset 0_16 */
#define PM_DSP2_ON_OFF		0x0058U	/* Dword offset 0_16 (Mobility Only) */

/* Accelerator CRTC */
#define CRTC2_OFF_PITCH		0x005CU	/* Dword offset 0_17 */

/* Hardware Cursor */
#define CUR_CLR0		0x0060U	/* Dword offset 0_18 */
#define CUR2_CLR0		0x0060U	/* Dword offset 0_18 */
#define CUR_CLR1		0x0064U	/* Dword offset 0_19 */
#define CUR2_CLR1		0x0064U	/* Dword offset 0_19 */
#define CUR_OFFSET		0x0068U	/* Dword offset 0_1A */
#define CUR2_OFFSET		0x0068U	/* Dword offset 0_1A */
#define CUR_HORZ_VERT_POSN	0x006CU	/* Dword offset 0_1B */
#define CUR2_HORZ_VERT_POSN	0x006CU	/* Dword offset 0_1B */
#define CUR_HORZ_VERT_OFF	0x0070U	/* Dword offset 0_1C */
#define CUR2_HORZ_VERT_OFF	0x0070U	/* Dword offset 0_1C */

#define CONFIG_PANEL_LG		0x0074U	/* Dword offset 0_1D */

/* General I/O Control */
#define GP_IO			0x0078U	/* Dword offset 0_1E */

/* Test and Debug */
#define HW_DEBUG		0x007CU	/* Dword offset 0_1F */

/* Scratch Pad and Test */
#define SCRATCH_REG0		0x0080U	/* Dword offset 0_20 */
#define SCRATCH_REG1		0x0084U	/* Dword offset 0_21 */
#define SCRATCH_REG2		0x0088U	/* Dword offset 0_22 */
#define SCRATCH_REG3		0x008CU	/* Dword offset 0_23 */

/* Clock Control */
#define CLOCK_CNTL		0x0090U	/* Dword offset 0_24 */
#define CLOCK_SEL_CNTL		0x0090U	/* Dword offset 0_24 */

/* Configuration */
#define CONFIG_STAT1		0x0094U	/* Dword offset 0_25 */
#define CONFIG_STAT2		0x0098U	/* Dword offset 0_26 */

/* Bus Control */
#define BUS_CNTL		0x00A0U	/* Dword offset 0_28 */

#define LCD_INDEX		0x00A4U	/* Dword offset 0_29 */
#define LCD_DATA		0x00A8U	/* Dword offset 0_2A */

/* Memory Control */
#define EXT_MEM_CNTL		0x00ACU	/* Dword offset 0_2B */
#define MEM_CNTL		0x00B0U	/* Dword offset 0_2C */
#define MEM_VGA_WP_SEL		0x00B4U	/* Dword offset 0_2D */
#define MEM_VGA_RP_SEL		0x00B8U	/* Dword offset 0_2E */

#define I2C_CNTL_1		0x00BCU	/* Dword offset 0_2F */

/* DAC Control */
#define DAC_REGS		0x00C0U	/* Dword offset 0_30 */
#define DAC_W_INDEX		0x00C0U	/* Dword offset 0_30 */
#define DAC_DATA		0x00C1U	/* Dword offset 0_30 */
#define DAC_MASK		0x00C2U	/* Dword offset 0_30 */
#define DAC_R_INDEX		0x00C3U	/* Dword offset 0_30 */
#define DAC_CNTL		0x00C4U	/* Dword offset 0_31 */

#define EXT_DAC_REGS		0x00C8U	/* Dword offset 0_32 */

/* Test and Debug */
#define GEN_TEST_CNTL		0x00D0U	/* Dword offset 0_34 */

/* Custom Macros */
#define CUSTOM_MACRO_CNTL	0x00D4U	/* Dword offset 0_35 */

#define LCD_GEN_CNTL_LG		0x00D4U	/* Dword offset 0_35 */

#define POWER_MANAGEMENT_LG	0x00D8U	/* Dword offset 0_36 (LG) */

/* Configuration */
#define CONFIG_CNTL		0x00DCU	/* Dword offset 0_37 (CT, ET, VT) */
#define CONFIG_CHIP_ID		0x00E0U	/* Dword offset 0_38 */
#define CONFIG_STAT0		0x00E4U	/* Dword offset 0_39 */

/* Test and Debug */
#define CRC_SIG			0x00E8U	/* Dword offset 0_3A */
#define CRC2_SIG		0x00E8U	/* Dword offset 0_3A */


/* GUI MEMORY MAPPED Registers */

/* Draw Engine Destination Trajectory */
#define DST_OFF_PITCH		0x0100U	/* Dword offset 0_40 */
#define DST_X			0x0104U	/* Dword offset 0_41 */
#define DST_Y			0x0108U	/* Dword offset 0_42 */
#define DST_Y_X			0x010CU	/* Dword offset 0_43 */
#define DST_WIDTH		0x0110U	/* Dword offset 0_44 */
#define DST_HEIGHT		0x0114U	/* Dword offset 0_45 */
#define DST_HEIGHT_WIDTH	0x0118U	/* Dword offset 0_46 */
#define DST_X_WIDTH		0x011CU	/* Dword offset 0_47 */
#define DST_BRES_LNTH		0x0120U	/* Dword offset 0_48 */
#define DST_BRES_ERR		0x0124U	/* Dword offset 0_49 */
#define DST_BRES_INC		0x0128U	/* Dword offset 0_4A */
#define DST_BRES_DEC		0x012CU	/* Dword offset 0_4B */
#define DST_CNTL		0x0130U	/* Dword offset 0_4C */
#define DST_Y_X__ALIAS__	0x0134U	/* Dword offset 0_4D */
#define TRAIL_BRES_ERR		0x0138U	/* Dword offset 0_4E */
#define TRAIL_BRES_INC		0x013CU	/* Dword offset 0_4F */
#define TRAIL_BRES_DEC		0x0140U	/* Dword offset 0_50 */
#define LEAD_BRES_LNTH		0x0144U	/* Dword offset 0_51 */
#define Z_OFF_PITCH		0x0148U	/* Dword offset 0_52 */
#define Z_CNTL			0x014CU	/* Dword offset 0_53 */
#define ALPHA_TST_CNTL		0x0150U	/* Dword offset 0_54 */
#define SECONDARY_STW_EXP	0x0158U	/* Dword offset 0_56 */
#define SECONDARY_S_X_INC	0x015CU	/* Dword offset 0_57 */
#define SECONDARY_S_Y_INC	0x0160U	/* Dword offset 0_58 */
#define SECONDARY_S_START	0x0164U	/* Dword offset 0_59 */
#define SECONDARY_W_X_INC	0x0168U	/* Dword offset 0_5A */
#define SECONDARY_W_Y_INC	0x016CU	/* Dword offset 0_5B */
#define SECONDARY_W_START	0x0170U	/* Dword offset 0_5C */
#define SECONDARY_T_X_INC	0x0174U	/* Dword offset 0_5D */
#define SECONDARY_T_Y_INC	0x0178U	/* Dword offset 0_5E */
#define SECONDARY_T_START	0x017CU	/* Dword offset 0_5F */

/* Draw Engine Source Trajectory */
#define SRC_OFF_PITCH		0x0180U	/* Dword offset 0_60 */
#define SRC_X			0x0184U	/* Dword offset 0_61 */
#define SRC_Y			0x0188U	/* Dword offset 0_62 */
#define SRC_Y_X			0x018CU	/* Dword offset 0_63 */
#define SRC_WIDTH1		0x0190U	/* Dword offset 0_64 */
#define SRC_HEIGHT1		0x0194U	/* Dword offset 0_65 */
#define SRC_HEIGHT1_WIDTH1	0x0198U	/* Dword offset 0_66 */
#define SRC_X_START		0x019CU	/* Dword offset 0_67 */
#define SRC_Y_START		0x01A0U	/* Dword offset 0_68 */
#define SRC_Y_X_START		0x01A4U	/* Dword offset 0_69 */
#define SRC_WIDTH2		0x01A8U	/* Dword offset 0_6A */
#define SRC_HEIGHT2		0x01ACU	/* Dword offset 0_6B */
#define SRC_HEIGHT2_WIDTH2	0x01B0U	/* Dword offset 0_6C */
#define SRC_CNTL		0x01B4U	/* Dword offset 0_6D */

#define SCALE_OFF		0x01C0U	/* Dword offset 0_70 */
#define SECONDARY_SCALE_OFF	0x01C4U	/* Dword offset 0_71 */

#define TEX_0_OFF		0x01C0U	/* Dword offset 0_70 */
#define TEX_1_OFF		0x01C4U	/* Dword offset 0_71 */
#define TEX_2_OFF		0x01C8U	/* Dword offset 0_72 */
#define TEX_3_OFF		0x01CCU	/* Dword offset 0_73 */
#define TEX_4_OFF		0x01D0U	/* Dword offset 0_74 */
#define TEX_5_OFF		0x01D4U	/* Dword offset 0_75 */
#define TEX_6_OFF		0x01D8U	/* Dword offset 0_76 */
#define TEX_7_OFF		0x01DCU	/* Dword offset 0_77 */

#define SCALE_WIDTH		0x01DCU	/* Dword offset 0_77 */
#define SCALE_HEIGHT		0x01E0U	/* Dword offset 0_78 */

#define TEX_8_OFF		0x01E0U	/* Dword offset 0_78 */
#define TEX_9_OFF		0x01E4U	/* Dword offset 0_79 */
#define TEX_10_OFF		0x01E8U	/* Dword offset 0_7A */
#define S_Y_INC			0x01ECU	/* Dword offset 0_7B */

#define SCALE_PITCH		0x01ECU	/* Dword offset 0_7B */
#define SCALE_X_INC		0x01F0U	/* Dword offset 0_7C */

#define RED_X_INC		0x01F0U	/* Dword offset 0_7C */
#define GREEN_X_INC		0x01F4U	/* Dword offset 0_7D */

#define SCALE_Y_INC		0x01F4U	/* Dword offset 0_7D */
#define SCALE_VACC		0x01F8U	/* Dword offset 0_7E */
#define SCALE_3D_CNTL		0x01FCU	/* Dword offset 0_7F */

/* Host Data */
#define HOST_DATA0		0x0200U	/* Dword offset 0_80 */
#define HOST_DATA1		0x0204U	/* Dword offset 0_81 */
#define HOST_DATA2		0x0208U	/* Dword offset 0_82 */
#define HOST_DATA3		0x020CU	/* Dword offset 0_83 */
#define HOST_DATA4		0x0210U	/* Dword offset 0_84 */
#define HOST_DATA5		0x0214U	/* Dword offset 0_85 */
#define HOST_DATA6		0x0218U	/* Dword offset 0_86 */
#define HOST_DATA7		0x021CU	/* Dword offset 0_87 */
#define HOST_DATA8		0x0220U	/* Dword offset 0_88 */
#define HOST_DATA9		0x0224U	/* Dword offset 0_89 */
#define HOST_DATAA		0x0228U	/* Dword offset 0_8A */
#define HOST_DATAB		0x022CU	/* Dword offset 0_8B */
#define HOST_DATAC		0x0230U	/* Dword offset 0_8C */
#define HOST_DATAD		0x0234U	/* Dword offset 0_8D */
#define HOST_DATAE		0x0238U	/* Dword offset 0_8E */
#define HOST_DATAF		0x023CU	/* Dword offset 0_8F */
#define HOST_CNTL		0x0240U	/* Dword offset 0_90 */

/* GUI Bus Mastering */
#define BM_HOSTDATA		0x0244U	/* Dword offset 0_91 */
#define BM_ADDR			0x0248U	/* Dword offset 0_92 */
#define BM_DATA			0x0248U	/* Dword offset 0_92 */
#define BM_GUI_TABLE_CMD	0x024CU	/* Dword offset 0_93 */

/* Pattern */
#define PAT_REG0		0x0280U	/* Dword offset 0_A0 */
#define PAT_REG1		0x0284U	/* Dword offset 0_A1 */
#define PAT_CNTL		0x0288U	/* Dword offset 0_A2 */

/* Scissors */
#define SC_LEFT			0x02A0U	/* Dword offset 0_A8 */
#define SC_RIGHT		0x02A4U	/* Dword offset 0_A9 */
#define SC_LEFT_RIGHT		0x02A8U	/* Dword offset 0_AA */
#define SC_TOP			0x02ACU	/* Dword offset 0_AB */
#define SC_BOTTOM		0x02B0U	/* Dword offset 0_AC */
#define SC_TOP_BOTTOM		0x02B4U	/* Dword offset 0_AD */

/* Data Path */
#define USR1_DST_OFF_PITCH	0x02B8U	/* Dword offset 0_AE */
#define USR2_DST_OFF_PITCH	0x02BCU	/* Dword offset 0_AF */
#define DP_BKGD_CLR		0x02C0U	/* Dword offset 0_B0 */
#define DP_FOG_CLR		0x02C4U	/* Dword offset 0_B1 */
#define DP_FRGD_CLR		0x02C4U	/* Dword offset 0_B1 */
#define DP_WRITE_MASK		0x02C8U	/* Dword offset 0_B2 */
#define DP_CHAIN_MASK		0x02CCU	/* Dword offset 0_B3 */
#define DP_PIX_WIDTH		0x02D0U	/* Dword offset 0_B4 */
#define DP_MIX			0x02D4U	/* Dword offset 0_B5 */
#define DP_SRC			0x02D8U	/* Dword offset 0_B6 */
#define DP_FRGD_CLR_MIX		0x02DCU	/* Dword offset 0_B7 */
#define DP_FRGD_BKGD_CLR	0x02E0U	/* Dword offset 0_B8 */

/* Draw Engine Destination Trajectory */
#define DST_X_Y			0x02E8U	/* Dword offset 0_BA */
#define DST_WIDTH_HEIGHT	0x02ECU	/* Dword offset 0_BB */

/* Data Path */
#define USR_DST_PICTH		0x02F0U	/* Dword offset 0_BC */
#define DP_SET_GUI_ENGINE2	0x02F8U	/* Dword offset 0_BE */
#define DP_SET_GUI_ENGINE	0x02FCU	/* Dword offset 0_BF */

/* Color Compare */
#define CLR_CMP_CLR		0x0300U	/* Dword offset 0_C0 */
#define CLR_CMP_MASK		0x0304U	/* Dword offset 0_C1 */
#define CLR_CMP_CNTL		0x0308U	/* Dword offset 0_C2 */

/* Command FIFO */
#define FIFO_STAT		0x0310U	/* Dword offset 0_C4 */

#define CONTEXT_MASK		0x0320U	/* Dword offset 0_C8 */
#define CONTEXT_LOAD_CNTL	0x032CU	/* Dword offset 0_CB */

/* Engine Control */
#define GUI_TRAJ_CNTL		0x0330U	/* Dword offset 0_CC */

/* Engine Status/FIFO */
#define GUI_STAT		0x0338U	/* Dword offset 0_CE */

#define TEX_PALETTE_INDEX	0x0340U	/* Dword offset 0_D0 */
#define STW_EXP			0x0344U	/* Dword offset 0_D1 */
#define LOG_MAX_INC		0x0348U	/* Dword offset 0_D2 */
#define S_X_INC			0x034CU	/* Dword offset 0_D3 */
#define S_Y_INC__ALIAS__	0x0350U	/* Dword offset 0_D4 */

#define SCALE_PITCH__ALIAS__	0x0350U	/* Dword offset 0_D4 */

#define S_START			0x0354U	/* Dword offset 0_D5 */
#define W_X_INC			0x0358U	/* Dword offset 0_D6 */
#define W_Y_INC			0x035CU	/* Dword offset 0_D7 */
#define W_START			0x0360U	/* Dword offset 0_D8 */
#define T_X_INC			0x0364U	/* Dword offset 0_D9 */
#define T_Y_INC			0x0368U	/* Dword offset 0_DA */

#define SECONDARY_SCALE_PITCH	0x0368U	/* Dword offset 0_DA */

#define T_START			0x036CU	/* Dword offset 0_DB */
#define TEX_SIZE_PITCH		0x0370U	/* Dword offset 0_DC */
#define TEX_CNTL		0x0374U	/* Dword offset 0_DD */
#define SECONDARY_TEX_OFFSET	0x0378U	/* Dword offset 0_DE */
#define TEX_PALETTE		0x037CU	/* Dword offset 0_DF */

#define SCALE_PITCH_BOTH	0x0380U	/* Dword offset 0_E0 */
#define SECONDARY_SCALE_OFF_ACC	0x0384U	/* Dword offset 0_E1 */
#define SCALE_OFF_ACC		0x0388U	/* Dword offset 0_E2 */
#define SCALE_DST_Y_X		0x038CU	/* Dword offset 0_E3 */

/* Draw Engine Destination Trajectory */
#define COMPOSITE_SHADOW_ID	0x0398U	/* Dword offset 0_E6 */

#define SECONDARY_SCALE_X_INC	0x039CU	/* Dword offset 0_E7 */

#define SPECULAR_RED_X_INC	0x039CU	/* Dword offset 0_E7 */
#define SPECULAR_RED_Y_INC	0x03A0U	/* Dword offset 0_E8 */
#define SPECULAR_RED_START	0x03A4U	/* Dword offset 0_E9 */

#define SECONDARY_SCALE_HACC	0x03A4U	/* Dword offset 0_E9 */

#define SPECULAR_GREEN_X_INC	0x03A8U	/* Dword offset 0_EA */
#define SPECULAR_GREEN_Y_INC	0x03ACU	/* Dword offset 0_EB */
#define SPECULAR_GREEN_START	0x03B0U	/* Dword offset 0_EC */
#define SPECULAR_BLUE_X_INC	0x03B4U	/* Dword offset 0_ED */
#define SPECULAR_BLUE_Y_INC	0x03B8U	/* Dword offset 0_EE */
#define SPECULAR_BLUE_START	0x03BCU	/* Dword offset 0_EF */

#define SCALE_X_INC__ALIAS__	0x03C0U	/* Dword offset 0_F0 */

#define RED_X_INC__ALIAS__	0x03C0U	/* Dword offset 0_F0 */
#define RED_Y_INC		0x03C4U	/* Dword offset 0_F1 */
#define RED_START		0x03C8U	/* Dword offset 0_F2 */

#define SCALE_HACC		0x03C8U	/* Dword offset 0_F2 */
#define SCALE_Y_INC__ALIAS__	0x03CCU	/* Dword offset 0_F3 */

#define GREEN_X_INC__ALIAS__	0x03CCU	/* Dword offset 0_F3 */
#define GREEN_Y_INC		0x03D0U	/* Dword offset 0_F4 */

#define SECONDARY_SCALE_Y_INC	0x03D0U	/* Dword offset 0_F4 */
#define SECONDARY_SCALE_VACC	0x03D4U	/* Dword offset 0_F5 */

#define GREEN_START		0x03D4U	/* Dword offset 0_F5 */
#define BLUE_X_INC		0x03D8U	/* Dword offset 0_F6 */
#define BLUE_Y_INC		0x03DCU	/* Dword offset 0_F7 */
#define BLUE_START		0x03E0U	/* Dword offset 0_F8 */
#define Z_X_INC			0x03E4U	/* Dword offset 0_F9 */
#define Z_Y_INC			0x03E8U	/* Dword offset 0_FA */
#define Z_START			0x03ECU	/* Dword offset 0_FB */
#define ALPHA_X_INC		0x03F0U	/* Dword offset 0_FC */
#define FOG_X_INC		0x03F0U	/* Dword offset 0_FC */
#define ALPHA_Y_INC		0x03F4U	/* Dword offset 0_FD */
#define FOG_Y_INC		0x03F4U	/* Dword offset 0_FD */
#define ALPHA_START		0x03F8U	/* Dword offset 0_FE */
#define FOG_START		0x03F8U	/* Dword offset 0_FE */

#define OVERLAY_Y_X_START		0x0400U	/* Dword offset 1_00 */
#define OVERLAY_Y_X_END			0x0404U	/* Dword offset 1_01 */
#define OVERLAY_VIDEO_KEY_CLR		0x0408U	/* Dword offset 1_02 */
#define OVERLAY_VIDEO_KEY_MSK		0x040CU	/* Dword offset 1_03 */
#define OVERLAY_GRAPHICS_KEY_CLR	0x0410U	/* Dword offset 1_04 */
#define OVERLAY_GRAPHICS_KEY_MSK	0x0414U	/* Dword offset 1_05 */
#define OVERLAY_KEY_CNTL		0x0418U	/* Dword offset 1_06 */

#define OVERLAY_SCALE_INC	0x0420U	/* Dword offset 1_08 */
#define OVERLAY_SCALE_CNTL	0x0424U	/* Dword offset 1_09 */
#define SCALER_HEIGHT_WIDTH	0x0428U	/* Dword offset 1_0A */
#define SCALER_TEST		0x042CU	/* Dword offset 1_0B */
#define SCALER_BUF0_OFFSET	0x0434U	/* Dword offset 1_0D */
#define SCALER_BUF1_OFFSET	0x0438U	/* Dword offset 1_0E */
#define SCALE_BUF_PITCH		0x043CU	/* Dword offset 1_0F */

#define CAPTURE_START_END	0x0440U	/* Dword offset 1_10 */
#define CAPTURE_X_WIDTH		0x0444U	/* Dword offset 1_11 */
#define VIDEO_FORMAT		0x0448U	/* Dword offset 1_12 */
#define VBI_START_END		0x044CU	/* Dword offset 1_13 */
#define CAPTURE_CONFIG		0x0450U	/* Dword offset 1_14 */
#define TRIG_CNTL		0x0454U	/* Dword offset 1_15 */

#define OVERLAY_EXCLUSIVE_HORZ	0x0458U	/* Dword offset 1_16 */
#define OVERLAY_EXCLUSIVE_VERT	0x045CU	/* Dword offset 1_17 */

#define VAL_WIDTH		0x0460U	/* Dword offset 1_18 */
#define CAPTURE_DEBUG		0x0464U	/* Dword offset 1_19 */
#define VIDEO_SYNC_TEST		0x0468U	/* Dword offset 1_1A */

/* GenLocking */
#define SNAPSHOT_VH_COUNTS	0x0470U	/* Dword offset 1_1C */
#define SNAPSHOT_F_COUNT	0x0474U	/* Dword offset 1_1D */
#define N_VIF_COUNT		0x0478U	/* Dword offset 1_1E */
#define SNAPSHOT_VIF_COUNT	0x047CU	/* Dword offset 1_1F */

#define CAPTURE_BUF0_OFFSET	0x0480U	/* Dword offset 1_20 */
#define CAPTURE_BUF1_OFFSET	0x0484U	/* Dword offset 1_21 */
#define CAPTURE_BUF_PITCH	0x0488U	/* Dword offset 1_22 */

/* GenLocking */
#define SNAPSHOT2_VH_COUNTS	0x04B0U	/* Dword offset 1_2C */
#define SNAPSHOT2_F_COUNT	0x04B4U	/* Dword offset 1_2D */
#define N_VIF2_COUNT		0x04B8U	/* Dword offset 1_2E */
#define SNAPSHOT2_VIF_COUNT	0x04BCU	/* Dword offset 1_2F */

#define MPP_CONFIG		0x04C0U	/* Dword offset 1_30 */
#define MPP_STROBE_SEQ		0x04C4U	/* Dword offset 1_31 */
#define MPP_ADDR		0x04C8U	/* Dword offset 1_32 */
#define MPP_DATA		0x04CCU	/* Dword offset 1_33 */
#define TVO_CNTL		0x0500U	/* Dword offset 1_40 */

/* Test and Debug */
#define CRT_HORZ_VERT_LOAD	0x0544U	/* Dword offset 1_51 */

/* AGP */
#define AGP_BASE		0x0548U	/* Dword offset 1_52 */
#define AGP_CNTL		0x054CU	/* Dword offset 1_53 */

#define SCALER_COLOUR_CNTL	0x0550U	/* Dword offset 1_54 */
#define SCALER_H_COEFF0		0x0554U	/* Dword offset 1_55 */
#define SCALER_H_COEFF1		0x0558U	/* Dword offset 1_56 */
#define SCALER_H_COEFF2		0x055CU	/* Dword offset 1_57 */
#define SCALER_H_COEFF3		0x0560U	/* Dword offset 1_58 */
#define SCALER_H_COEFF4		0x0564U	/* Dword offset 1_59 */

/* Command FIFO */
#define GUI_CMDFIFO_DEBUG	0x0570U	/* Dword offset 1_5C */
#define GUI_CMDFIFO_DATA	0x0574U	/* Dword offset 1_5D */
#define GUI_CNTL		0x0578U	/* Dword offset 1_5E */

/* Bus Mastering */
#define BM_FRAME_BUF_OFFSET	0x0580U	/* Dword offset 1_60 */
#define BM_SYSTEM_MEM_ADDR	0x0584U	/* Dword offset 1_61 */
#define BM_COMMAND		0x0588U	/* Dword offset 1_62 */
#define BM_STATUS		0x058CU	/* Dword offset 1_63 */
#define BM_GUI_TABLE		0x05B8U	/* Dword offset 1_6E */
#define BM_SYSTEM_TABLE		0x05BCU	/* Dword offset 1_6F */

#define SCALER_BUF0_OFFSET_U	0x05D4U	/* Dword offset 1_75 */
#define SCALER_BUF0_OFFSET_V	0x05D8U	/* Dword offset 1_76 */
#define SCALER_BUF1_OFFSET_U	0x05DCU	/* Dword offset 1_77 */
#define SCALER_BUF1_OFFSET_V	0x05E0U	/* Dword offset 1_78 */

/* Setup Engine */
#define VERTEX_1_S		0x0640U	/* Dword offset 1_90 */
#define VERTEX_1_T		0x0644U	/* Dword offset 1_91 */
#define VERTEX_1_W		0x0648U	/* Dword offset 1_92 */
#define VERTEX_1_SPEC_ARGB	0x064CU	/* Dword offset 1_93 */
#define VERTEX_1_Z		0x0650U	/* Dword offset 1_94 */
#define VERTEX_1_ARGB		0x0654U	/* Dword offset 1_95 */
#define VERTEX_1_X_Y		0x0658U	/* Dword offset 1_96 */
#define ONE_OVER_AREA		0x065CU	/* Dword offset 1_97 */
#define VERTEX_2_S		0x0660U	/* Dword offset 1_98 */
#define VERTEX_2_T		0x0664U	/* Dword offset 1_99 */
#define VERTEX_2_W		0x0668U	/* Dword offset 1_9A */
#define VERTEX_2_SPEC_ARGB	0x066CU	/* Dword offset 1_9B */
#define VERTEX_2_Z		0x0670U	/* Dword offset 1_9C */
#define VERTEX_2_ARGB		0x0674U	/* Dword offset 1_9D */
#define VERTEX_2_X_Y		0x0678U	/* Dword offset 1_9E */
#define ONE_OVER_AREA		0x065CU	/* Dword offset 1_9F */
#define VERTEX_3_S		0x0680U	/* Dword offset 1_A0 */
#define VERTEX_3_T		0x0684U	/* Dword offset 1_A1 */
#define VERTEX_3_W		0x0688U	/* Dword offset 1_A2 */
#define VERTEX_3_SPEC_ARGB	0x068CU	/* Dword offset 1_A3 */
#define VERTEX_3_Z		0x0690U	/* Dword offset 1_A4 */
#define VERTEX_3_ARGB		0x0694U	/* Dword offset 1_A5 */
#define VERTEX_3_X_Y		0x0698U	/* Dword offset 1_A6 */
#define ONE_OVER_AREA		0x065CU	/* Dword offset 1_A7 */
#define VERTEX_1_S		0x0640U	/* Dword offset 1_AB */
#define VERTEX_1_T		0x0644U	/* Dword offset 1_AC */
#define VERTEX_1_W		0x0648U	/* Dword offset 1_AD */
#define VERTEX_2_S		0x0660U	/* Dword offset 1_AE */
#define VERTEX_2_T		0x0664U	/* Dword offset 1_AF */
#define VERTEX_2_W		0x0668U	/* Dword offset 1_B0 */
#define VERTEX_3_SECONDARY_S	0x06C0U	/* Dword offset 1_B0 */
#define VERTEX_3_S		0x0680U	/* Dword offset 1_B1 */
#define VERTEX_3_SECONDARY_T	0x06C4U	/* Dword offset 1_B1 */
#define VERTEX_3_T		0x0684U	/* Dword offset 1_B2 */
#define VERTEX_3_SECONDARY_W	0x06C8U	/* Dword offset 1_B2 */
#define VERTEX_3_W		0x0688U	/* Dword offset 1_B3 */
#define VERTEX_1_SPEC_ARGB	0x064CU	/* Dword offset 1_B4 */
#define VERTEX_2_SPEC_ARGB	0x066CU	/* Dword offset 1_B5 */
#define VERTEX_3_SPEC_ARGB	0x068CU	/* Dword offset 1_B6 */
#define VERTEX_1_Z		0x0650U	/* Dword offset 1_B7 */
#define VERTEX_2_Z		0x0670U	/* Dword offset 1_B8 */
#define VERTEX_3_Z		0x0690U	/* Dword offset 1_B9 */
#define VERTEX_1_ARGB		0x0654U	/* Dword offset 1_BA */
#define VERTEX_2_ARGB		0x0674U	/* Dword offset 1_BB */
#define VERTEX_3_ARGB		0x0694U	/* Dword offset 1_BC */
#define VERTEX_1_X_Y		0x0658U	/* Dword offset 1_BD */
#define VERTEX_2_X_Y		0x0678U	/* Dword offset 1_BE */
#define VERTEX_3_X_Y		0x0698U	/* Dword offset 1_BF */
#define ONE_OVER_AREA_UC	0x0700U	/* Dword offset 1_C0 */
#define SETUP_CNTL		0x0704U	/* Dword offset 1_C1 */
#define VERTEX_1_SECONDARY_S	0x0728U	/* Dword offset 1_CA */
#define VERTEX_1_SECONDARY_T	0x072CU	/* Dword offset 1_CB */
#define VERTEX_1_SECONDARY_W	0x0730U	/* Dword offset 1_CC */
#define VERTEX_2_SECONDARY_S	0x0734U	/* Dword offset 1_CD */
#define VERTEX_2_SECONDARY_T	0x0738U	/* Dword offset 1_CE */
#define VERTEX_2_SECONDARY_W	0x073CU	/* Dword offset 1_CF */


#define GTC_3D_RESET_DELAY	3000U	/* 3D engine reset delay in microseconds */

/* CRTC control values (mostly CRTC_GEN_CNTL) */

#define CRTC_H_SYNC_NEG		0x00200000U
#define CRTC_V_SYNC_NEG		0x00200000U

#define CRTC_DBL_SCAN_EN	0x00000001U
#define CRTC_INTERLACE_EN	0x00000002U
#define CRTC_HSYNC_DIS		0x00000004U
#define CRTC_VSYNC_DIS		0x00000008U
#define CRTC_CSYNC_EN		0x00000010U
#define CRTC_PIX_BY_2_EN	0x00000020U	/* unused on RAGE */
#define CRTC_DISPLAY_DIS	0x00000040U
#define CRTC_VGA_XOVERSCAN	0x00000040U

#define CRTC_PIX_WIDTH_MASK	0x00000700U
#define CRTC_PIX_WIDTH_4BPP	0x00000100U
#define CRTC_PIX_WIDTH_8BPP	0x00000200U
#define CRTC_PIX_WIDTH_15BPP	0x00000300U
#define CRTC_PIX_WIDTH_16BPP	0x00000400U
#define CRTC_PIX_WIDTH_24BPP	0x00000500U
#define CRTC_PIX_WIDTH_32BPP	0x00000600U

#define CRTC_BYTE_PIX_ORDER	0x00000800U
#define CRTC_PIX_ORDER_MSN_LSN	0x00000000U
#define CRTC_PIX_ORDER_LSN_MSN	0x00000800U

#define CRTC_FIFO_LWM		0x000f0000U

#define VGA_128KAP_PAGING	0x00100000U
#define VFC_SYNC_TRISTATE	0x00200000U
#define CRTC_LOCK_REGS		0x00400000U
#define CRTC_SYNC_TRISTATE	0x00800000U

#define CRTC_EXT_DISP_EN	0x01000000U
#define CRTC_ENABLE		0x02000000U
#define CRTC_DISP_REQ_ENB	0x04000000U
#define VGA_ATI_LINEAR		0x08000000U
#define CRTC_VSYNC_FALL_EDGE	0x10000000U
#define VGA_TEXT_132		0x20000000U
#define VGA_XCRT_CNT_EN		0x40000000U
#define VGA_CUR_B_TEST		0x80000000U

#define CRTC_CRNT_VLINE		0x07f00000U
#define CRTC_VBLANK		0x00000001U


/* DAC control values */

#define DAC_EXT_SEL_RS2		0x01U
#define DAC_EXT_SEL_RS3		0x02U
#define DAC_8BIT_EN		0x00000100U
#define DAC_PIX_DLY_MASK	0x00000600U
#define DAC_PIX_DLY_0NS		0x00000000U
#define DAC_PIX_DLY_2NS		0x00000200U
#define DAC_PIX_DLY_4NS		0x00000400U
#define DAC_BLANK_ADJ_MASK	0x00001800U
#define DAC_BLANK_ADJ_0		0x00000000U
#define DAC_BLANK_ADJ_1		0x00000800U
#define DAC_BLANK_ADJ_2		0x00001000U


/* Mix control values */

#define MIX_NOT_DST		0x0000U
#define MIX_0			0x0001U
#define MIX_1			0x0002U
#define MIX_DST			0x0003U
#define MIX_NOT_SRC		0x0004U
#define MIX_XOR			0x0005U
#define MIX_XNOR		0x0006U
#define MIX_SRC			0x0007U
#define MIX_NAND		0x0008U
#define MIX_NOT_SRC_OR_DST	0x0009U
#define MIX_SRC_OR_NOT_DST	0x000aU
#define MIX_OR			0x000bU
#define MIX_AND			0x000cU
#define MIX_SRC_AND_NOT_DST	0x000dU
#define MIX_NOT_SRC_AND_DST	0x000eU
#define MIX_NOR			0x000fU

/* Maximum engine dimensions */
#define ENGINE_MIN_X		0
#define ENGINE_MIN_Y		0
#define ENGINE_MAX_X		4095
#define ENGINE_MAX_Y		16383

/* Mach64 engine bit constants - these are typically ORed together */

/* BUS_CNTL register constants */
#define BUS_FIFO_ERR_ACK	0x00200000U
#define BUS_HOST_ERR_ACK	0x00800000U

/* GEN_TEST_CNTL register constants */
#define GEN_OVR_OUTPUT_EN	0x20U
#define HWCURSOR_ENABLE		0x80U
#define GUI_ENGINE_ENABLE	0x100U
#define BLOCK_WRITE_ENABLE	0x200U

/* DSP_CONFIG register constants */
#define DSP_XCLKS_PER_QW	0x00003fffU
#define DSP_LOOP_LATENCY	0x000f0000U
#define DSP_PRECISION		0x00700000U

/* DSP_ON_OFF register constants */
#define DSP_OFF			0x000007ffU
#define DSP_ON			0x07ff0000U

/* CLOCK_CNTL register constants */
#define CLOCK_SEL		0x0fU
#define CLOCK_DIV		0x30U
#define CLOCK_DIV1		0x00U
#define CLOCK_DIV2		0x10U
#define CLOCK_DIV4		0x20U
#define CLOCK_STROBE		0x40U
#define PLL_WR_EN		0x02U

/* PLL register indices */
#define MPLL_CNTL		0x00U
#define VPLL_CNTL		0x01U
#define PLL_REF_DIV		0x02U
#define PLL_GEN_CNTL		0x03U
#define MCLK_FB_DIV		0x04U
#define PLL_VCLK_CNTL		0x05U
#define VCLK_POST_DIV		0x06U
#define VCLK0_FB_DIV		0x07U
#define VCLK1_FB_DIV		0x08U
#define VCLK2_FB_DIV		0x09U
#define VCLK3_FB_DIV		0x0AU
#define PLL_EXT_CNTL		0x0BU
#define DLL_CNTL		0x0CU
#define DLL1_CNTL		0x0CU
#define VFC_CNTL		0x0DU
#define PLL_TEST_CNTL		0x0EU
#define PLL_TEST_COUNT		0x0FU
#define LVDS_CNTL0		0x10U
#define LVDS_CNTL1		0x11U
#define AGP1_CNTL		0x12U
#define AGP2_CNTL		0x13U
#define DLL2_CNTL		0x14U
#define SCLK_FB_DIV		0x15U
#define SPLL_CNTL1		0x16U
#define SPLL_CNTL2		0x17U
#define APLL_STRAPS		0x18U
#define EXT_VPLL_CNTL		0x19U
#define EXT_VPLL_REF_DIV	0x1AU
#define EXT_VPLL_FB_DIV		0x1BU
#define EXT_VPLL_MSB		0x1CU
#define HTOTAL_CNTL		0x1DU
#define BYTE_CLK_CNTL		0x1EU
#define TV_PLL_CNTL1		0x1FU
#define TV_PLL_CNTL2		0x20U
#define TV_PLL_CNTL		0x21U
#define EXT_TV_PLL		0x22U
#define V2PLL_CNTL		0x23U
#define PLL_V2CLK_CNTL		0x24U
#define EXT_V2PLL_REF_DIV	0x25U
#define EXT_V2PLL_FB_DIV	0x26U
#define EXT_V2PLL_MSB		0x27U
#define HTOTAL2_CNTL		0x28U
#define PLL_YCLK_CNTL		0x29U
#define PM_DYN_CLK_CNTL		0x2AU

/* Fields in PLL registers */
#define PLL_PC_GAIN		0x07U
#define PLL_VC_GAIN		0x18U
#define PLL_DUTY_CYC		0xE0U
#define PLL_OVERRIDE		0x01U
#define PLL_MCLK_RST		0x02U
#define OSC_EN			0x04U
#define EXT_CLK_EN		0x08U
#define MCLK_SRC_SEL		0x70U
#define EXT_CLK_CNTL		0x80U
#define VCLK_SRC_SEL		0x03U
#define PLL_VCLK_RST		0x04U
#define VCLK_INVERT		0x08U
#define VCLK0_POST		0x03U
#define VCLK1_POST		0x0CU
#define VCLK2_POST		0x30U
#define VCLK3_POST		0xC0U

/* CONFIG_CNTL register constants */
#define APERTURE_4M_ENABLE	1
#define APERTURE_8M_ENABLE	2
#define VGA_APERTURE_ENABLE	4

/* CONFIG_STAT0 register constants (GX, CX) */
#define CFG_BUS_TYPE		0x00000007U
#define CFG_MEM_TYPE		0x00000038U
#define CFG_INIT_DAC_TYPE	0x00000e00U

/* CONFIG_STAT0 register constants (CT, ET, VT) */
#define CFG_MEM_TYPE_xT		0x00000007U

#define ISA			0
#define EISA			1
#define LOCAL_BUS		6
#define PCI			7

/* Memory types for GX, CX */
#define DRAMx4			0
#define VRAMx16			1
#define VRAMx16ssr		2
#define DRAMx16			3
#define GraphicsDRAMx16		4
#define EnhancedVRAMx16		5
#define EnhancedVRAMx16ssr	6

/* Memory types for CT, ET, VT, GT */
#define DRAM			1
#define EDO			2
#define PSEUDO_EDO		3
#define SDRAM			4
#define SGRAM			5
#define WRAM			6

#define DAC_INTERNAL		0x00U
#define DAC_IBMRGB514		0x01U
#define DAC_ATI68875		0x02U
#define DAC_TVP3026_A		0x72U
#define DAC_BT476		0x03U
#define DAC_BT481		0x04U
#define DAC_ATT20C491		0x14U
#define DAC_SC15026		0x24U
#define DAC_MU9C1880		0x34U
#define DAC_IMSG174		0x44U
#define DAC_ATI68860_B		0x05U
#define DAC_ATI68860_C		0x15U
#define DAC_TVP3026_B		0x75U
#define DAC_STG1700		0x06U
#define DAC_ATT498		0x16U
#define DAC_STG1702		0x07U
#define DAC_SC15021		0x17U
#define DAC_ATT21C498		0x27U
#define DAC_STG1703		0x37U
#define DAC_CH8398		0x47U
#define DAC_ATT20C408		0x57U

#define CLK_ATI18818_0		0
#define CLK_ATI18818_1		1
#define CLK_STG1703		2
#define CLK_CH8398		3
#define CLK_INTERNAL		4
#define CLK_ATT20C408		5
#define CLK_IBMRGB514		6

/* MEM_CNTL register constants */
#define MEM_SIZE_ALIAS		0x00000007U
#define MEM_SIZE_512K		0x00000000U
#define MEM_SIZE_1M		0x00000001U
#define MEM_SIZE_2M		0x00000002U
#define MEM_SIZE_4M		0x00000003U
#define MEM_SIZE_6M		0x00000004U
#define MEM_SIZE_8M		0x00000005U
#define MEM_SIZE_ALIAS_GTB	0x0000000FU
#define MEM_SIZE_2M_GTB		0x00000003U
#define MEM_SIZE_4M_GTB		0x00000007U
#define MEM_SIZE_6M_GTB		0x00000009U
#define MEM_SIZE_8M_GTB		0x0000000BU
#define MEM_BNDRY		0x00030000U
#define MEM_BNDRY_0K		0x00000000U
#define MEM_BNDRY_256K		0x00010000U
#define MEM_BNDRY_512K		0x00020000U
#define MEM_BNDRY_1M		0x00030000U
#define MEM_BNDRY_EN		0x00040000U

/* ATI PCI constants */
#define PCI_ATI_VENDOR_ID	0x1002U


/* CONFIG_CHIP_ID register constants */
#define CFG_CHIP_TYPE		0x0000FFFFU
#define CFG_CHIP_CLASS		0x00FF0000U
#define CFG_CHIP_REV		0xFF000000U
#define CFG_CHIP_MAJOR		0x07000000U
#define CFG_CHIP_FND_ID		0x38000000U
#define CFG_CHIP_MINOR		0xC0000000U


/* Chip IDs read from CONFIG_CHIP_ID */

/* mach64GX family */
#define GX_CHIP_ID	0xD7U	/* mach64GX (ATI888GX00) */
#define CX_CHIP_ID	0x57U	/* mach64CX (ATI888CX00) */

#define GX_PCI_ID	0x4758U	/* mach64GX (ATI888GX00) */
#define CX_PCI_ID	0x4358U	/* mach64CX (ATI888CX00) */

/* mach64CT family */
#define CT_CHIP_ID	0x4354U	/* mach64CT (ATI264CT) */
#define ET_CHIP_ID	0x4554U	/* mach64ET (ATI264ET) */

/* mach64CT family / mach64VT class */
#define VT_CHIP_ID	0x5654U	/* mach64VT (ATI264VT) */
#define VU_CHIP_ID	0x5655U	/* mach64VTB (ATI264VTB) */
#define VV_CHIP_ID	0x5656U	/* mach64VT4 (ATI264VT4) */

/* mach64CT family / mach64GT (3D RAGE) class */
#define LB_CHIP_ID	0x4c42U	/* RAGE LT PRO, AGP */
#define LD_CHIP_ID	0x4c44U	/* RAGE LT PRO */
#define LG_CHIP_ID	0x4c47U	/* RAGE LT */
#define LI_CHIP_ID	0x4c49U	/* RAGE LT PRO */
#define LP_CHIP_ID	0x4c50U	/* RAGE LT PRO */
#define LT_CHIP_ID	0x4c54U	/* RAGE LT */
#define XL_CHIP_ID	0x4752U	/* RAGE (XL) */
#define GT_CHIP_ID	0x4754U	/* RAGE (GT) */
#define GU_CHIP_ID	0x4755U	/* RAGE II/II+ (GTB) */
#define GV_CHIP_ID	0x4756U	/* RAGE IIC, PCI */
#define GW_CHIP_ID	0x4757U	/* RAGE IIC, AGP */
#define GZ_CHIP_ID	0x475aU	/* RAGE IIC, AGP */
#define GB_CHIP_ID	0x4742U	/* RAGE PRO, BGA, AGP 1x and 2x */
#define GD_CHIP_ID	0x4744U	/* RAGE PRO, BGA, AGP 1x only */
#define GI_CHIP_ID	0x4749U	/* RAGE PRO, BGA, PCI33 only */
#define GP_CHIP_ID	0x4750U	/* RAGE PRO, PQFP, PCI33, full 3D */
#define GQ_CHIP_ID	0x4751U	/* RAGE PRO, PQFP, PCI33, limited 3D */
#define LM_CHIP_ID	0x4c4dU	/* RAGE Mobility PCI */
#define LN_CHIP_ID	0x4c4eU	/* RAGE Mobility AGP */


/* Mach64 major ASIC revisions */
#define MACH64_ASIC_NEC_VT_A3		0x08U
#define MACH64_ASIC_NEC_VT_A4		0x48U
#define MACH64_ASIC_SGS_VT_A4		0x40U
#define MACH64_ASIC_SGS_VT_B1S1		0x01U
#define MACH64_ASIC_SGS_GT_B1S1		0x01U
#define MACH64_ASIC_SGS_GT_B1S2		0x41U
#define MACH64_ASIC_UMC_GT_B2U1		0x1aU
#define MACH64_ASIC_UMC_GT_B2U2		0x5aU
#define MACH64_ASIC_UMC_VT_B2U3		0x9aU
#define MACH64_ASIC_UMC_GT_B2U3		0x9aU
#define MACH64_ASIC_UMC_R3B_D_P_A1	0x1bU
#define MACH64_ASIC_UMC_R3B_D_P_A2	0x5bU
#define MACH64_ASIC_UMC_R3B_D_P_A3	0x1cU
#define MACH64_ASIC_UMC_R3B_D_P_A4	0x5cU

/* Mach64 foundries */
#define MACH64_FND_SGS		0
#define MACH64_FND_NEC		1
#define MACH64_FND_UMC		3

/* Mach64 chip types */
#define MACH64_UNKNOWN		0
#define MACH64_GX		1
#define MACH64_CX		2
#define MACH64_CT		3
#define MACH64_ET		4
#define MACH64_VT		5
#define MACH64_GT		6

/* DST_CNTL register constants */
#define DST_X_RIGHT_TO_LEFT	0
#define DST_X_LEFT_TO_RIGHT	1
#define DST_Y_BOTTOM_TO_TOP	0
#define DST_Y_TOP_TO_BOTTOM	2
#define DST_X_MAJOR		0
#define DST_Y_MAJOR		4
#define DST_X_TILE		8
#define DST_Y_TILE		0x10U
#define DST_LAST_PEL		0x20U
#define DST_POLYGON_ENABLE	0x40U
#define DST_24_ROTATION_ENABLE	0x80U

/* SRC_CNTL register constants */
#define SRC_PATTERN_ENABLE		1
#define SRC_ROTATION_ENABLE		2
#define SRC_LINEAR_ENABLE		4
#define SRC_BYTE_ALIGN			8
#define SRC_LINE_X_RIGHT_TO_LEFT	0
#define SRC_LINE_X_LEFT_TO_RIGHT	0x10

/* HOST_CNTL register constants */
#define HOST_BYTE_ALIGN		1

/* GUI_TRAJ_CNTL register constants */
#define PAT_MONO_8x8_ENABLE	0x01000000U
#define PAT_CLR_4x2_ENABLE	0x02000000U
#define PAT_CLR_8x1_ENABLE	0x04000000U

/* DP_CHAIN_MASK register constants */
#define DP_CHAIN_4BPP		0x8888U
#define DP_CHAIN_7BPP		0xD2D2U
#define DP_CHAIN_8BPP		0x8080U
#define DP_CHAIN_8BPP_RGB	0x9292U
#define DP_CHAIN_15BPP		0x4210U
#define DP_CHAIN_16BPP		0x8410U
#define DP_CHAIN_24BPP		0x8080U
#define DP_CHAIN_32BPP		0x8080U

/* DP_PIX_WIDTH register constants */
#define DST_1BPP		0
#define DST_4BPP		1
#define DST_8BPP		2
#define DST_15BPP		3
#define DST_16BPP		4
#define DST_32BPP		6
#define SRC_1BPP		0
#define SRC_4BPP		0x100U
#define SRC_8BPP		0x200U
#define SRC_15BPP		0x300U
#define SRC_16BPP		0x400U
#define SRC_32BPP		0x600U
#define HOST_TRIPLE_EN		0x2000U
#define HOST_1BPP		0
#define HOST_4BPP		0x10000U
#define HOST_8BPP		0x20000U
#define HOST_15BPP		0x30000U
#define HOST_16BPP		0x40000U
#define HOST_32BPP		0x60000U
#define BYTE_ORDER_MSB_TO_LSB	0
#define BYTE_ORDER_LSB_TO_MSB	0x1000000U

/* DP_MIX register constants */
#define BKGD_MIX_NOT_D			0
#define BKGD_MIX_ZERO			1
#define BKGD_MIX_ONE			2
#define BKGD_MIX_D			3
#define BKGD_MIX_NOT_S			4
#define BKGD_MIX_D_XOR_S		5
#define BKGD_MIX_NOT_D_XOR_S		6
#define BKGD_MIX_S			7
#define BKGD_MIX_NOT_D_OR_NOT_S		8
#define BKGD_MIX_D_OR_NOT_S		9
#define BKGD_MIX_NOT_D_OR_S		10
#define BKGD_MIX_D_OR_S			11
#define BKGD_MIX_D_AND_S		12
#define BKGD_MIX_NOT_D_AND_S		13
#define BKGD_MIX_D_AND_NOT_S		14
#define BKGD_MIX_NOT_D_AND_NOT_S	15
#define BKGD_MIX_D_PLUS_S_DIV2		0x17U
#define FRGD_MIX_NOT_D			0
#define FRGD_MIX_ZERO			0x10000U
#define FRGD_MIX_ONE			0x20000U
#define FRGD_MIX_D			0x30000U
#define FRGD_MIX_NOT_S			0x40000U
#define FRGD_MIX_D_XOR_S		0x50000U
#define FRGD_MIX_NOT_D_XOR_S		0x60000U
#define FRGD_MIX_S			0x70000U
#define FRGD_MIX_NOT_D_OR_NOT_S		0x80000U
#define FRGD_MIX_D_OR_NOT_S		0x90000U
#define FRGD_MIX_NOT_D_OR_S		0xa0000U
#define FRGD_MIX_D_OR_S			0xb0000U
#define FRGD_MIX_D_AND_S		0xc0000U
#define FRGD_MIX_NOT_D_AND_S		0xd0000U
#define FRGD_MIX_D_AND_NOT_S		0xe0000U
#define FRGD_MIX_NOT_D_AND_NOT_S	0xf0000U
#define FRGD_MIX_D_PLUS_S_DIV2		0x170000U

/* DP_SRC register constants */
#define BKGD_SRC_BKGD_CLR	0
#define BKGD_SRC_FRGD_CLR	1
#define BKGD_SRC_HOST		2
#define BKGD_SRC_BLIT		3
#define BKGD_SRC_PATTERN	4
#define FRGD_SRC_BKGD_CLR	0
#define FRGD_SRC_FRGD_CLR	0x100U
#define FRGD_SRC_HOST		0x200U
#define FRGD_SRC_BLIT		0x300U
#define FRGD_SRC_PATTERN	0x400U
#define MONO_SRC_ONE		0
#define MONO_SRC_PATTERN	0x10000U
#define MONO_SRC_HOST		0x20000U
#define MONO_SRC_BLIT		0x30000U

/* CLR_CMP_CNTL register constants */
#define COMPARE_FALSE		0
#define COMPARE_TRUE		1
#define COMPARE_NOT_EQUAL	4
#define COMPARE_EQUAL		5
#define COMPARE_DESTINATION	0
#define COMPARE_SOURCE		0x1000000U

/* FIFO_STAT register constants */
#define FIFO_ERR		0x80000000

/* CONTEXT_LOAD_CNTL constants */
#define CONTEXT_NO_LOAD			0
#define CONTEXT_LOAD			0x10000U
#define CONTEXT_LOAD_AND_DO_FILL	0x20000U
#define CONTEXT_LOAD_AND_DO_LINE	0x30000U
#define CONTEXT_EXECUTE			0
#define CONTEXT_CMD_DISABLE		0x80000000U

/* GUI_STAT register constants */
#define ENGINE_IDLE		0
#define ENGINE_BUSY		1
#define SCISSOR_LEFT_FLAG	0x10U
#define SCISSOR_RIGHT_FLAG	0x20U
#define SCISSOR_TOP_FLAG	0x40U
#define SCISSOR_BOTTOM_FLAG	0x80U

/* ATI VGA Extended Regsiters */
#define sioATIEXT		0x1ceU
#define bioATIEXT		0x3ceU

#define ATI2E			0xaeU
#define ATI32			0xb2U
#define ATI36			0xb6U

/* VGA Graphics Controller Registers */
#define VGAGRA			0x3ceU
#define GRA06			0x06U

/* VGA Seququencer Registers */
#define VGASEQ			0x3c4U
#define SEQ02			0x02U
#define SEQ04			0x04U

#define MACH64_MAX_X		ENGINE_MAX_X
#define MACH64_MAX_Y		ENGINE_MAX_Y

#define INC_X			0x0020U
#define INC_Y			0x0080U

#define RGB16_555		0x0000U
#define RGB16_565		0x0040U
#define RGB16_655		0x0080U
#define RGB16_664		0x00c0U

#define POLY_TEXT_TYPE		0x0001U
#define IMAGE_TEXT_TYPE		0x0002U
#define TEXT_TYPE_8_BIT		0x0004U
#define TEXT_TYPE_16_BIT	0x0008U
#define POLY_TEXT_TYPE_8	(POLY_TEXT_TYPE | TEXT_TYPE_8_BIT)
#define IMAGE_TEXT_TYPE_8	(IMAGE_TEXT_TYPE | TEXT_TYPE_8_BIT)
#define POLY_TEXT_TYPE_16	(POLY_TEXT_TYPE | TEXT_TYPE_16_BIT)
#define IMAGE_TEXT_TYPE_16	(IMAGE_TEXT_TYPE | TEXT_TYPE_16_BIT)

#define MACH64_NUM_CLOCKS	16
#define MACH64_NUM_FREQS	50

/* Power Management register constants (LT & LT Pro) */
#define PWR_MGT_ON		0x00000001U
#define PWR_MGT_MODE_MASK	0x00000006U
#define AUTO_PWR_UP		0x00000008U
#define USE_F32KHZ		0x00000400U
#define TRISTATE_MEM_EN		0x00000800U
#define SELF_REFRESH		0x00000080U
#define PWR_BLON		0x02000000U
#define STANDBY_NOW		0x10000000U
#define SUSPEND_NOW		0x20000000U
#define PWR_MGT_STATUS_MASK	0xC0000000U
#define PWR_MGT_STATUS_SUSPEND	0x80000000U

/* PM Mode constants  */
#define PWR_MGT_MODE_PIN	0x00000000U
#define PWR_MGT_MODE_REG	0x00000002U
#define PWR_MGT_MODE_TIMER	0x00000004U
#define PWR_MGT_MODE_PCI	0x00000006U

/* LCD registers (LT Pro) */

/* LCD Index register */
#define LCD_INDEX_MASK		0x0000003FU
#define LCD_DISPLAY_DIS		0x00000100U
#define LCD_SRC_SEL		0x00000200U
#define CRTC2_DISPLAY_DIS	0x00000400U

/* LCD register indices */
#define CONFIG_PANEL		0x00U
#define LCD_GEN_CTRL		0x01U
#define DSTN_CONTROL		0x02U
#define HFB_PITCH_ADDR		0x03U
#define HORZ_STRETCHING		0x04U
#define VERT_STRETCHING		0x05U
#define EXT_VERT_STRETCH	0x06U
#define LT_GIO			0x07U
#define POWER_MANAGEMENT	0x08U
#define ZVGPIO			0x09U
#define ICON_CLR0		0x0AU
#define ICON_CLR1		0x0BU
#define ICON_OFFSET		0x0CU
#define ICON_HORZ_VERT_POSN	0x0DU
#define ICON_HORZ_VERT_OFF	0x0EU
#define ICON2_CLR0		0x0FU
#define ICON2_CLR1		0x10U
#define ICON2_OFFSET		0x11U
#define ICON2_HORZ_VERT_POSN	0x12U
#define ICON2_HORZ_VERT_OFF	0x13U
#define LCD_MISC_CNTL		0x14U
#define APC_CNTL		0x1CU
#define POWER_MANAGEMENT_2	0x1DU
#define ALPHA_BLENDING		0x25U
#define PORTRAIT_GEN_CNTL	0x26U
#define APC_CTRL_IO		0x27U
#define TEST_IO			0x28U
#define TEST_OUTPUTS		0x29U
#define DP1_MEM_ACCESS		0x2AU
#define DP0_MEM_ACCESS		0x2BU
#define DP0_DEBUG_A		0x2CU
#define DP0_DEBUG_B		0x2DU
#define DP1_DEBUG_A		0x2EU
#define DP1_DEBUG_B		0x2FU
#define DPCTRL_DEBUG_A		0x30U
#define DPCTRL_DEBUG_B		0x31U
#define MEMBLK_DEBUG		0x32U
#define APC_LUT_AB		0x33U
#define APC_LUT_CD		0x34U
#define APC_LUT_EF		0x35U
#define APC_LUT_GH		0x36U
#define APC_LUT_IJ		0x37U
#define APC_LUT_KL		0x38U
#define APC_LUT_MN		0x39U
#define APC_LUT_OP		0x3AU

/* Values in LCD_GEN_CTRL */
#define CRT_ON                          0x00000001ul
#define LCD_ON                          0x00000002ul
#define HORZ_DIVBY2_EN                  0x00000004ul
#define DONT_DS_ICON                    0x00000008ul
#define LOCK_8DOT                       0x00000010ul
#define ICON_ENABLE                     0x00000020ul
#define DONT_SHADOW_VPAR                0x00000040ul
#define V2CLK_PM_EN                     0x00000080ul
#define RST_FM                          0x00000100ul
#define DISABLE_PCLK_RESET              0x00000200ul    /* XC/XL */
#define DIS_HOR_CRT_DIVBY2              0x00000400ul
#define SCLK_SEL                        0x00000800ul
#define SCLK_DELAY                      0x0000f000ul
#define TVCLK_PM_EN                     0x00010000ul
#define VCLK_DAC_PM_EN                  0x00020000ul
#define VCLK_LCD_OFF                    0x00040000ul
#define SELECT_WAIT_4MS                 0x00080000ul
#define XTALIN_PM_EN                    0x00080000ul    /* XC/XL */
#define V2CLK_DAC_PM_EN                 0x00100000ul
#define LVDS_EN                         0x00200000ul
#define LVDS_PLL_EN                     0x00400000ul
#define LVDS_PLL_RESET                  0x00800000ul
#define LVDS_RESERVED_BITS              0x07000000ul
#define CRTC_RW_SELECT                  0x08000000ul    /* LTPro */
#define USE_SHADOWED_VEND               0x10000000ul
#define USE_SHADOWED_ROWCUR             0x20000000ul
#define SHADOW_EN                       0x40000000ul
#define SHADOW_RW_EN                    0x80000000ul

/* Values in HORZ_STRETCHING */
#define HORZ_STRETCH_BLEND              0x00000ffful
#define HORZ_STRETCH_RATIO              0x0000fffful
#define HORZ_STRETCH_LOOP               0x00070000ul
#define HORZ_STRETCH_LOOP09                     0x00000000ul
#define HORZ_STRETCH_LOOP11                     0x00010000ul
#define HORZ_STRETCH_LOOP12                     0x00020000ul
#define HORZ_STRETCH_LOOP14                     0x00030000ul
#define HORZ_STRETCH_LOOP15                     0x00040000ul
/*      ?                                       0x00050000ul */
/*      ?                                       0x00060000ul */
/*      ?                                       0x00070000ul */
/*      ?                               0x00080000ul */
#define HORZ_PANEL_SIZE                 0x0ff00000ul    /* XC/XL */
/*      ?                               0x10000000ul */
#define AUTO_HORZ_RATIO                 0x20000000ul    /* XC/XL */
#define HORZ_STRETCH_MODE               0x40000000ul
#define HORZ_STRETCH_EN                 0x80000000ul

/* Values in VERT_STRETCHING */
#define VERT_STRETCH_RATIO0             0x000003fful
#define VERT_STRETCH_RATIO1             0x000ffc00ul
#define VERT_STRETCH_RATIO2             0x3ff00000ul
#define VERT_STRETCH_USE0               0x40000000ul
#define VERT_STRETCH_EN                 0x80000000ul

/* Values in EXT_VERT_STRETCH */
#define AUTO_VERT_RATIO                 0x00400000ul
#define VERT_STRETCH_MODE		0x00000400ul

/* Values in LCD_MISC_CNTL */
#define BIAS_MOD_LEVEL_MASK	0x0000ff00U
#define BIAS_MOD_LEVEL_SHIFT	8
#define BLMOD_EN		0x00010000U
#define BIASMOD_EN		0x00020000U

#endif /* REGMACH64_H */
