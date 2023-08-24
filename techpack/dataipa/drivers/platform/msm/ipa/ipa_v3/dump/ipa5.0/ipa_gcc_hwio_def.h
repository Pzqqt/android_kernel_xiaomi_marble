/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (c) 2021, The Linux Foundation. All rights reserved.
 */

#ifndef __IPA_GCC_HWIO_DEF_H__
#define __IPA_GCC_HWIO_DEF_H__
/**
  @file ipa_gcc_hwio.h
  @brief Auto-generated HWIO interface include file.

  This file contains HWIO register definitions for the following modules:
    GCC_CLK_CTL_REG.*

  'Include' filters applied: <none>
  'Exclude' filters applied: RESERVED DUMMY
*/

/*----------------------------------------------------------------------------
 * MODULE: GCC_CLK_CTL_REG
 *--------------------------------------------------------------------------*/

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_SYSTEM_NOC_BCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_system_noc_bcr_s
{
  u32 blk_ares : 1;
  u32 reserved0 : 31;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_system_noc_bcr_u
{
  struct ipa_gcc_hwio_def_gcc_system_noc_bcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_SYS_NOC_AXI_CBCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_sys_noc_axi_cbcr_s
{
  u32 clk_enable : 1;
  u32 hw_ctl : 1;
  u32 clk_ares : 1;
  u32 sw_only_en : 1;
  u32 reserved0 : 16;
  u32 ignore_rpmh_clk_dis : 1;
  u32 reserved1 : 1;
  u32 clk_dis : 1;
  u32 ignore_all_clk_dis : 1;
  u32 ignore_all_ares : 1;
  u32 reserved2 : 6;
  u32 clk_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_sys_noc_axi_cbcr_u
{
  struct ipa_gcc_hwio_def_gcc_sys_noc_axi_cbcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_SYS_NOC_HS_AXI_CBCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_sys_noc_hs_axi_cbcr_s
{
  u32 clk_enable : 1;
  u32 hw_ctl : 1;
  u32 clk_ares : 1;
  u32 sw_only_en : 1;
  u32 reserved0 : 16;
  u32 ignore_rpmh_clk_dis : 1;
  u32 reserved1 : 1;
  u32 clk_dis : 1;
  u32 ignore_all_clk_dis : 1;
  u32 ignore_all_ares : 1;
  u32 reserved2 : 6;
  u32 clk_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_sys_noc_hs_axi_cbcr_u
{
  struct ipa_gcc_hwio_def_gcc_sys_noc_hs_axi_cbcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_SYS_NOC_QDSS_STM_AXI_CBCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_sys_noc_qdss_stm_axi_cbcr_s
{
  u32 clk_enable : 1;
  u32 hw_ctl : 1;
  u32 clk_ares : 1;
  u32 sw_only_en : 1;
  u32 reserved0 : 16;
  u32 ignore_rpmh_clk_dis : 1;
  u32 reserved1 : 1;
  u32 clk_dis : 1;
  u32 ignore_all_clk_dis : 1;
  u32 ignore_all_ares : 1;
  u32 reserved2 : 6;
  u32 clk_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_sys_noc_qdss_stm_axi_cbcr_u
{
  struct ipa_gcc_hwio_def_gcc_sys_noc_qdss_stm_axi_cbcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_SYS_NOC_CPUSS_AHB_CBCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_sys_noc_cpuss_ahb_cbcr_s
{
  u32 reserved0 : 1;
  u32 hw_ctl : 1;
  u32 clk_ares : 1;
  u32 sw_only_en : 1;
  u32 reserved1 : 18;
  u32 clk_dis : 1;
  u32 ignore_all_clk_dis : 1;
  u32 ignore_all_ares : 1;
  u32 reserved2 : 6;
  u32 clk_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_sys_noc_cpuss_ahb_cbcr_u
{
  struct ipa_gcc_hwio_def_gcc_sys_noc_cpuss_ahb_cbcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_SYS_NOC_AHB_CFG_CBCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_sys_noc_ahb_cfg_cbcr_s
{
  u32 clk_enable : 1;
  u32 hw_ctl : 1;
  u32 clk_ares : 1;
  u32 sw_only_en : 1;
  u32 reserved0 : 16;
  u32 ignore_rpmh_clk_dis : 1;
  u32 reserved1 : 1;
  u32 clk_dis : 1;
  u32 ignore_all_clk_dis : 1;
  u32 ignore_all_ares : 1;
  u32 reserved2 : 6;
  u32 clk_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_sys_noc_ahb_cfg_cbcr_u
{
  struct ipa_gcc_hwio_def_gcc_sys_noc_ahb_cfg_cbcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_SYS_NOC_IPA_CBCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_sys_noc_ipa_cbcr_s
{
  u32 clk_enable : 1;
  u32 reserved0 : 1;
  u32 clk_ares : 1;
  u32 reserved1 : 17;
  u32 ignore_rpmh_clk_dis : 1;
  u32 reserved2 : 1;
  u32 clk_dis : 1;
  u32 ignore_all_clk_dis : 1;
  u32 ignore_all_ares : 1;
  u32 reserved3 : 6;
  u32 clk_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_sys_noc_ipa_cbcr_u
{
  struct ipa_gcc_hwio_def_gcc_sys_noc_ipa_cbcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_SYS_NOC_AT_CBCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_sys_noc_at_cbcr_s
{
  u32 clk_enable : 1;
  u32 hw_ctl : 1;
  u32 clk_ares : 1;
  u32 sw_only_en : 1;
  u32 reserved0 : 16;
  u32 ignore_rpmh_clk_dis : 1;
  u32 reserved1 : 1;
  u32 clk_dis : 1;
  u32 ignore_all_clk_dis : 1;
  u32 ignore_all_ares : 1;
  u32 reserved2 : 6;
  u32 clk_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_sys_noc_at_cbcr_u
{
  struct ipa_gcc_hwio_def_gcc_sys_noc_at_cbcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_SYS_NOC_CMD_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_sys_noc_cmd_dfsr_s
{
  u32 dfs_en : 1;
  u32 curr_perf_state : 4;
  u32 hw_clk_control : 1;
  u32 dfs_fsm_state : 3;
  u32 perf_state_update_status : 1;
  u32 sw_override : 1;
  u32 sw_perf_state : 4;
  u32 rcg_sw_ctrl : 4;
  u32 reserved0 : 13;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_sys_noc_cmd_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_sys_noc_cmd_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_SYS_NOC_SYS_NOC_PERF0_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_sys_noc_sys_noc_perf0_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 21;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_sys_noc_sys_noc_perf0_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_sys_noc_sys_noc_perf0_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_SYS_NOC_SYS_NOC_PERF1_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_sys_noc_sys_noc_perf1_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 21;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_sys_noc_sys_noc_perf1_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_sys_noc_sys_noc_perf1_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_SYS_NOC_SYS_NOC_PERF2_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_sys_noc_sys_noc_perf2_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 21;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_sys_noc_sys_noc_perf2_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_sys_noc_sys_noc_perf2_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_SYS_NOC_SYS_NOC_PERF3_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_sys_noc_sys_noc_perf3_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 21;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_sys_noc_sys_noc_perf3_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_sys_noc_sys_noc_perf3_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_SYS_NOC_SYS_NOC_PERF4_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_sys_noc_sys_noc_perf4_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 21;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_sys_noc_sys_noc_perf4_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_sys_noc_sys_noc_perf4_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_SYS_NOC_SYS_NOC_PERF5_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_sys_noc_sys_noc_perf5_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 21;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_sys_noc_sys_noc_perf5_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_sys_noc_sys_noc_perf5_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_SYS_NOC_SYS_NOC_PERF6_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_sys_noc_sys_noc_perf6_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 21;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_sys_noc_sys_noc_perf6_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_sys_noc_sys_noc_perf6_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_SYS_NOC_SYS_NOC_PERF7_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_sys_noc_sys_noc_perf7_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 21;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_sys_noc_sys_noc_perf7_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_sys_noc_sys_noc_perf7_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_SYS_NOC_SYS_NOC_PERF8_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_sys_noc_sys_noc_perf8_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 21;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_sys_noc_sys_noc_perf8_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_sys_noc_sys_noc_perf8_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_SYS_NOC_SYS_NOC_PERF9_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_sys_noc_sys_noc_perf9_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 21;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_sys_noc_sys_noc_perf9_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_sys_noc_sys_noc_perf9_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_SYS_NOC_SYS_NOC_PERF10_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_sys_noc_sys_noc_perf10_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 21;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_sys_noc_sys_noc_perf10_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_sys_noc_sys_noc_perf10_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_SYS_NOC_SYS_NOC_PERF11_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_sys_noc_sys_noc_perf11_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 21;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_sys_noc_sys_noc_perf11_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_sys_noc_sys_noc_perf11_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_SYS_NOC_SYS_NOC_PERF12_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_sys_noc_sys_noc_perf12_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 21;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_sys_noc_sys_noc_perf12_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_sys_noc_sys_noc_perf12_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_SYS_NOC_SYS_NOC_PERF13_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_sys_noc_sys_noc_perf13_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 21;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_sys_noc_sys_noc_perf13_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_sys_noc_sys_noc_perf13_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_SYS_NOC_SYS_NOC_PERF14_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_sys_noc_sys_noc_perf14_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 21;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_sys_noc_sys_noc_perf14_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_sys_noc_sys_noc_perf14_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_SYS_NOC_SYS_NOC_PERF15_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_sys_noc_sys_noc_perf15_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 21;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_sys_noc_sys_noc_perf15_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_sys_noc_sys_noc_perf15_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_SYS_NOC_CMD_RCGR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_sys_noc_cmd_rcgr_s
{
  u32 update : 1;
  u32 root_en : 1;
  u32 reserved0 : 2;
  u32 dirty_cfg_rcgr : 1;
  u32 reserved1 : 26;
  u32 root_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_sys_noc_cmd_rcgr_u
{
  struct ipa_gcc_hwio_def_gcc_sys_noc_cmd_rcgr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_SYS_NOC_CFG_RCGR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_sys_noc_cfg_rcgr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 5;
  u32 rcglite_disable : 1;
  u32 reserved2 : 3;
  u32 hw_clk_control : 1;
  u32 reserved3 : 11;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_sys_noc_cfg_rcgr_u
{
  struct ipa_gcc_hwio_def_gcc_sys_noc_cfg_rcgr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_SYS_NOC_DCD_CDIV_DCDR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_sys_noc_dcd_cdiv_dcdr_s
{
  u32 dcd_enable : 1;
  u32 reserved0 : 31;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_sys_noc_dcd_cdiv_dcdr_u
{
  struct ipa_gcc_hwio_def_gcc_sys_noc_dcd_cdiv_dcdr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_SYS_NOC_SYS_NOC_HS_PERF0_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_sys_noc_sys_noc_hs_perf0_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 21;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_sys_noc_sys_noc_hs_perf0_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_sys_noc_sys_noc_hs_perf0_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_SYS_NOC_SYS_NOC_HS_PERF1_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_sys_noc_sys_noc_hs_perf1_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 21;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_sys_noc_sys_noc_hs_perf1_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_sys_noc_sys_noc_hs_perf1_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_SYS_NOC_SYS_NOC_HS_PERF2_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_sys_noc_sys_noc_hs_perf2_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 21;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_sys_noc_sys_noc_hs_perf2_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_sys_noc_sys_noc_hs_perf2_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_SYS_NOC_SYS_NOC_HS_PERF3_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_sys_noc_sys_noc_hs_perf3_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 21;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_sys_noc_sys_noc_hs_perf3_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_sys_noc_sys_noc_hs_perf3_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_SYS_NOC_SYS_NOC_HS_PERF4_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_sys_noc_sys_noc_hs_perf4_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 21;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_sys_noc_sys_noc_hs_perf4_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_sys_noc_sys_noc_hs_perf4_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_SYS_NOC_SYS_NOC_HS_PERF5_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_sys_noc_sys_noc_hs_perf5_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 21;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_sys_noc_sys_noc_hs_perf5_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_sys_noc_sys_noc_hs_perf5_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_SYS_NOC_SYS_NOC_HS_PERF6_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_sys_noc_sys_noc_hs_perf6_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 21;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_sys_noc_sys_noc_hs_perf6_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_sys_noc_sys_noc_hs_perf6_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_SYS_NOC_SYS_NOC_HS_PERF7_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_sys_noc_sys_noc_hs_perf7_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 21;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_sys_noc_sys_noc_hs_perf7_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_sys_noc_sys_noc_hs_perf7_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_SYS_NOC_SYS_NOC_HS_PERF8_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_sys_noc_sys_noc_hs_perf8_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 21;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_sys_noc_sys_noc_hs_perf8_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_sys_noc_sys_noc_hs_perf8_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_SYS_NOC_SYS_NOC_HS_PERF9_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_sys_noc_sys_noc_hs_perf9_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 21;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_sys_noc_sys_noc_hs_perf9_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_sys_noc_sys_noc_hs_perf9_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_SYS_NOC_SYS_NOC_HS_PERF10_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_sys_noc_sys_noc_hs_perf10_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 21;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_sys_noc_sys_noc_hs_perf10_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_sys_noc_sys_noc_hs_perf10_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_SYS_NOC_SYS_NOC_HS_PERF11_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_sys_noc_sys_noc_hs_perf11_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 21;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_sys_noc_sys_noc_hs_perf11_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_sys_noc_sys_noc_hs_perf11_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_SYS_NOC_SYS_NOC_HS_PERF12_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_sys_noc_sys_noc_hs_perf12_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 21;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_sys_noc_sys_noc_hs_perf12_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_sys_noc_sys_noc_hs_perf12_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_SYS_NOC_SYS_NOC_HS_PERF13_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_sys_noc_sys_noc_hs_perf13_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 21;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_sys_noc_sys_noc_hs_perf13_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_sys_noc_sys_noc_hs_perf13_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_SYS_NOC_SYS_NOC_HS_PERF14_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_sys_noc_sys_noc_hs_perf14_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 21;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_sys_noc_sys_noc_hs_perf14_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_sys_noc_sys_noc_hs_perf14_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_SYS_NOC_SYS_NOC_HS_PERF15_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_sys_noc_sys_noc_hs_perf15_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 21;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_sys_noc_sys_noc_hs_perf15_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_sys_noc_sys_noc_hs_perf15_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_SYS_NOC_HS_CMD_RCGR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_sys_noc_hs_cmd_rcgr_s
{
  u32 update : 1;
  u32 root_en : 1;
  u32 reserved0 : 2;
  u32 dirty_cfg_rcgr : 1;
  u32 reserved1 : 26;
  u32 root_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_sys_noc_hs_cmd_rcgr_u
{
  struct ipa_gcc_hwio_def_gcc_sys_noc_hs_cmd_rcgr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_SYS_NOC_HS_CFG_RCGR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_sys_noc_hs_cfg_rcgr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 5;
  u32 rcglite_disable : 1;
  u32 reserved2 : 3;
  u32 hw_clk_control : 1;
  u32 reserved3 : 11;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_sys_noc_hs_cfg_rcgr_u
{
  struct ipa_gcc_hwio_def_gcc_sys_noc_hs_cfg_rcgr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_SYS_NOC_HS_DCD_CDIV_DCDR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_sys_noc_hs_dcd_cdiv_dcdr_s
{
  u32 dcd_enable : 1;
  u32 reserved0 : 31;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_sys_noc_hs_dcd_cdiv_dcdr_u
{
  struct ipa_gcc_hwio_def_gcc_sys_noc_hs_dcd_cdiv_dcdr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_SNOC_QOSGEN_EXTREF_CBCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_snoc_qosgen_extref_cbcr_s
{
  u32 clk_enable : 1;
  u32 reserved0 : 1;
  u32 clk_ares : 1;
  u32 reserved1 : 19;
  u32 clk_dis : 1;
  u32 ignore_all_clk_dis : 1;
  u32 ignore_all_ares : 1;
  u32 reserved2 : 6;
  u32 clk_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_snoc_qosgen_extref_cbcr_u
{
  struct ipa_gcc_hwio_def_gcc_snoc_qosgen_extref_cbcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_PCNOC_BCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_pcnoc_bcr_s
{
  u32 blk_ares : 1;
  u32 reserved0 : 31;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_pcnoc_bcr_u
{
  struct ipa_gcc_hwio_def_gcc_pcnoc_bcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_CFG_AHB_CBCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_cfg_ahb_cbcr_s
{
  u32 clk_enable : 1;
  u32 hw_ctl : 1;
  u32 clk_ares : 1;
  u32 sw_only_en : 1;
  u32 reserved0 : 16;
  u32 ignore_rpmh_clk_dis : 1;
  u32 reserved1 : 1;
  u32 clk_dis : 1;
  u32 ignore_all_clk_dis : 1;
  u32 ignore_all_ares : 1;
  u32 reserved2 : 6;
  u32 clk_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_cfg_ahb_cbcr_u
{
  struct ipa_gcc_hwio_def_gcc_cfg_ahb_cbcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_NOC_DCD_XO_CBCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_noc_dcd_xo_cbcr_s
{
  u32 clk_enable : 1;
  u32 reserved0 : 1;
  u32 clk_ares : 1;
  u32 reserved1 : 19;
  u32 clk_dis : 1;
  u32 ignore_all_clk_dis : 1;
  u32 ignore_all_ares : 1;
  u32 reserved2 : 6;
  u32 clk_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_noc_dcd_xo_cbcr_u
{
  struct ipa_gcc_hwio_def_gcc_noc_dcd_xo_cbcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_PCNOC_SPMI_VGIS_CBCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_pcnoc_spmi_vgis_cbcr_s
{
  u32 clk_enable : 1;
  u32 reserved0 : 1;
  u32 clk_ares : 1;
  u32 reserved1 : 19;
  u32 clk_dis : 1;
  u32 ignore_all_clk_dis : 1;
  u32 ignore_all_ares : 1;
  u32 reserved2 : 6;
  u32 clk_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_pcnoc_spmi_vgis_cbcr_u
{
  struct ipa_gcc_hwio_def_gcc_pcnoc_spmi_vgis_cbcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_CNOC_CMD_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_cnoc_cmd_dfsr_s
{
  u32 dfs_en : 1;
  u32 curr_perf_state : 4;
  u32 hw_clk_control : 1;
  u32 dfs_fsm_state : 3;
  u32 perf_state_update_status : 1;
  u32 sw_override : 1;
  u32 sw_perf_state : 4;
  u32 rcg_sw_ctrl : 1;
  u32 reserved0 : 16;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_cnoc_cmd_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_cnoc_cmd_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_CNOC_PCNOC_PERF0_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_cnoc_pcnoc_perf0_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 21;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_cnoc_pcnoc_perf0_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_cnoc_pcnoc_perf0_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_CNOC_PCNOC_PERF1_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_cnoc_pcnoc_perf1_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 21;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_cnoc_pcnoc_perf1_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_cnoc_pcnoc_perf1_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_CNOC_PCNOC_PERF2_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_cnoc_pcnoc_perf2_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 21;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_cnoc_pcnoc_perf2_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_cnoc_pcnoc_perf2_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_CNOC_PCNOC_PERF3_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_cnoc_pcnoc_perf3_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 21;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_cnoc_pcnoc_perf3_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_cnoc_pcnoc_perf3_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_CNOC_PCNOC_PERF4_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_cnoc_pcnoc_perf4_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 21;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_cnoc_pcnoc_perf4_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_cnoc_pcnoc_perf4_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_CNOC_PCNOC_PERF5_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_cnoc_pcnoc_perf5_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 21;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_cnoc_pcnoc_perf5_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_cnoc_pcnoc_perf5_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_CNOC_PCNOC_PERF6_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_cnoc_pcnoc_perf6_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 21;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_cnoc_pcnoc_perf6_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_cnoc_pcnoc_perf6_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_CNOC_PCNOC_PERF7_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_cnoc_pcnoc_perf7_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 21;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_cnoc_pcnoc_perf7_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_cnoc_pcnoc_perf7_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_CNOC_PCNOC_PERF8_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_cnoc_pcnoc_perf8_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 21;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_cnoc_pcnoc_perf8_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_cnoc_pcnoc_perf8_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_CNOC_PCNOC_PERF9_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_cnoc_pcnoc_perf9_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 21;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_cnoc_pcnoc_perf9_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_cnoc_pcnoc_perf9_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_CNOC_PCNOC_PERF10_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_cnoc_pcnoc_perf10_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 21;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_cnoc_pcnoc_perf10_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_cnoc_pcnoc_perf10_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_CNOC_PCNOC_PERF11_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_cnoc_pcnoc_perf11_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 21;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_cnoc_pcnoc_perf11_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_cnoc_pcnoc_perf11_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_CNOC_PCNOC_PERF12_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_cnoc_pcnoc_perf12_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 21;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_cnoc_pcnoc_perf12_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_cnoc_pcnoc_perf12_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_CNOC_PCNOC_PERF13_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_cnoc_pcnoc_perf13_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 21;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_cnoc_pcnoc_perf13_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_cnoc_pcnoc_perf13_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_CNOC_PCNOC_PERF14_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_cnoc_pcnoc_perf14_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 21;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_cnoc_pcnoc_perf14_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_cnoc_pcnoc_perf14_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_CNOC_PCNOC_PERF15_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_cnoc_pcnoc_perf15_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 21;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_cnoc_pcnoc_perf15_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_cnoc_pcnoc_perf15_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_PCNOC_CMD_RCGR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_pcnoc_cmd_rcgr_s
{
  u32 update : 1;
  u32 root_en : 1;
  u32 reserved0 : 2;
  u32 dirty_cfg_rcgr : 1;
  u32 reserved1 : 26;
  u32 root_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_pcnoc_cmd_rcgr_u
{
  struct ipa_gcc_hwio_def_gcc_pcnoc_cmd_rcgr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_PCNOC_CFG_RCGR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_pcnoc_cfg_rcgr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 5;
  u32 rcglite_disable : 1;
  u32 reserved2 : 3;
  u32 hw_clk_control : 1;
  u32 reserved3 : 11;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_pcnoc_cfg_rcgr_u
{
  struct ipa_gcc_hwio_def_gcc_pcnoc_cfg_rcgr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_PCNOC_DCD_CDIV_DCDR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_pcnoc_dcd_cdiv_dcdr_s
{
  u32 dcd_enable : 1;
  u32 reserved0 : 31;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_pcnoc_dcd_cdiv_dcdr_u
{
  struct ipa_gcc_hwio_def_gcc_pcnoc_dcd_cdiv_dcdr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_TIC_CBCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_tic_cbcr_s
{
  u32 clk_enable : 1;
  u32 hw_ctl : 1;
  u32 clk_ares : 1;
  u32 sw_only_en : 1;
  u32 reserved0 : 18;
  u32 clk_dis : 1;
  u32 ignore_all_clk_dis : 1;
  u32 ignore_all_ares : 1;
  u32 reserved1 : 6;
  u32 clk_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_tic_cbcr_u
{
  struct ipa_gcc_hwio_def_gcc_tic_cbcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_TIC_CFG_AHB_CBCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_tic_cfg_ahb_cbcr_s
{
  u32 clk_enable : 1;
  u32 hw_ctl : 1;
  u32 clk_ares : 1;
  u32 sw_only_en : 1;
  u32 sleep : 4;
  u32 wakeup : 4;
  u32 force_mem_periph_off : 1;
  u32 force_mem_periph_on : 1;
  u32 force_mem_core_on : 1;
  u32 reserved0 : 5;
  u32 ignore_rpmh_clk_dis : 1;
  u32 reserved1 : 1;
  u32 clk_dis : 1;
  u32 ignore_all_clk_dis : 1;
  u32 ignore_all_ares : 1;
  u32 reserved2 : 6;
  u32 clk_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_tic_cfg_ahb_cbcr_u
{
  struct ipa_gcc_hwio_def_gcc_tic_cfg_ahb_cbcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_TIC_CFG_AHB_SREGR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_tic_cfg_ahb_sregr_s
{
  u32 reserved0 : 1;
  u32 sw_clk_en_slp_stg : 1;
  u32 sw_clk_en_sel_slp_stg : 1;
  u32 sw_ctrl_pwr_down : 1;
  u32 sw_rst_slp_stg : 1;
  u32 sw_rst_sel_slp_stg : 1;
  u32 force_clk_on : 1;
  u32 mem_cph_enable : 1;
  u32 sw_div_ratio_slp_stg_clk : 2;
  u32 mem_periph_on_ack : 1;
  u32 mem_core_on_ack : 1;
  u32 sw_sm_pscbc_seq_in_override : 1;
  u32 mem_cph_rst_sw_override : 1;
  u32 pscbc_slp_stg_mode_csr : 1;
  u32 ignore_gdsc_pwr_dwn_csr : 1;
  u32 sreg_pscbc_spare_ctrl_in : 8;
  u32 sreg_pscbc_spare_ctrl_out : 8;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_tic_cfg_ahb_sregr_u
{
  struct ipa_gcc_hwio_def_gcc_tic_cfg_ahb_sregr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_IMEM_BCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_imem_bcr_s
{
  u32 blk_ares : 1;
  u32 reserved0 : 31;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_imem_bcr_u
{
  struct ipa_gcc_hwio_def_gcc_imem_bcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_IMEM_AXI_CBCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_imem_axi_cbcr_s
{
  u32 reserved0 : 2;
  u32 clk_ares : 1;
  u32 reserved1 : 1;
  u32 sleep : 4;
  u32 wakeup : 4;
  u32 force_mem_periph_off : 1;
  u32 force_mem_periph_on : 1;
  u32 force_mem_core_on : 1;
  u32 reserved2 : 5;
  u32 ignore_rpmh_clk_dis : 1;
  u32 reserved3 : 1;
  u32 clk_dis : 1;
  u32 ignore_all_clk_dis : 1;
  u32 ignore_all_ares : 1;
  u32 reserved4 : 6;
  u32 clk_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_imem_axi_cbcr_u
{
  struct ipa_gcc_hwio_def_gcc_imem_axi_cbcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_IMEM_AXI_SREGR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_imem_axi_sregr_s
{
  u32 reserved0 : 1;
  u32 sw_clk_en_slp_stg : 1;
  u32 sw_clk_en_sel_slp_stg : 1;
  u32 sw_ctrl_pwr_down : 1;
  u32 sw_rst_slp_stg : 1;
  u32 sw_rst_sel_slp_stg : 1;
  u32 force_clk_on : 1;
  u32 mem_cph_enable : 1;
  u32 sw_div_ratio_slp_stg_clk : 2;
  u32 mem_periph_on_ack : 1;
  u32 mem_core_on_ack : 1;
  u32 sw_sm_pscbc_seq_in_override : 1;
  u32 mem_cph_rst_sw_override : 1;
  u32 pscbc_slp_stg_mode_csr : 1;
  u32 ignore_gdsc_pwr_dwn_csr : 1;
  u32 sreg_pscbc_spare_ctrl_in : 8;
  u32 sreg_pscbc_spare_ctrl_out : 8;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_imem_axi_sregr_u
{
  struct ipa_gcc_hwio_def_gcc_imem_axi_sregr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_IMEM_CFG_AHB_CBCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_imem_cfg_ahb_cbcr_s
{
  u32 clk_enable : 1;
  u32 reserved0 : 1;
  u32 clk_ares : 1;
  u32 reserved1 : 17;
  u32 ignore_rpmh_clk_dis : 1;
  u32 reserved2 : 1;
  u32 clk_dis : 1;
  u32 ignore_all_clk_dis : 1;
  u32 ignore_all_ares : 1;
  u32 reserved3 : 6;
  u32 clk_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_imem_cfg_ahb_cbcr_u
{
  struct ipa_gcc_hwio_def_gcc_imem_cfg_ahb_cbcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_MMU_BCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_mmu_bcr_s
{
  u32 blk_ares : 1;
  u32 reserved0 : 31;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_mmu_bcr_u
{
  struct ipa_gcc_hwio_def_gcc_mmu_bcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_SYS_NOC_TCU_CBCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_sys_noc_tcu_cbcr_s
{
  u32 clk_enable : 1;
  u32 hw_ctl : 1;
  u32 clk_ares : 1;
  u32 sw_only_en : 1;
  u32 reserved0 : 16;
  u32 ignore_rpmh_clk_dis : 1;
  u32 reserved1 : 1;
  u32 clk_dis : 1;
  u32 ignore_all_clk_dis : 1;
  u32 ignore_all_ares : 1;
  u32 reserved2 : 6;
  u32 clk_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_sys_noc_tcu_cbcr_u
{
  struct ipa_gcc_hwio_def_gcc_sys_noc_tcu_cbcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_MMU_TCU_CBCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_mmu_tcu_cbcr_s
{
  u32 clk_enable : 1;
  u32 hw_ctl : 1;
  u32 clk_ares : 1;
  u32 sw_only_en : 1;
  u32 sleep : 4;
  u32 wakeup : 4;
  u32 force_mem_periph_off : 1;
  u32 force_mem_periph_on : 1;
  u32 force_mem_core_on : 1;
  u32 reserved0 : 5;
  u32 ignore_rpmh_clk_dis : 1;
  u32 reserved1 : 1;
  u32 clk_dis : 1;
  u32 ignore_all_clk_dis : 1;
  u32 ignore_all_ares : 1;
  u32 reserved2 : 6;
  u32 clk_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_mmu_tcu_cbcr_u
{
  struct ipa_gcc_hwio_def_gcc_mmu_tcu_cbcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_MMU_TCU_SREGR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_mmu_tcu_sregr_s
{
  u32 reserved0 : 1;
  u32 sw_clk_en_slp_stg : 1;
  u32 sw_clk_en_sel_slp_stg : 1;
  u32 sw_ctrl_pwr_down : 1;
  u32 sw_rst_slp_stg : 1;
  u32 sw_rst_sel_slp_stg : 1;
  u32 force_clk_on : 1;
  u32 mem_cph_enable : 1;
  u32 sw_div_ratio_slp_stg_clk : 2;
  u32 mem_periph_on_ack : 1;
  u32 mem_core_on_ack : 1;
  u32 sw_sm_pscbc_seq_in_override : 1;
  u32 mem_cph_rst_sw_override : 1;
  u32 pscbc_slp_stg_mode_csr : 1;
  u32 ignore_gdsc_pwr_dwn_csr : 1;
  u32 sreg_pscbc_spare_ctrl_in : 8;
  u32 sreg_pscbc_spare_ctrl_out : 8;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_mmu_tcu_sregr_u
{
  struct ipa_gcc_hwio_def_gcc_mmu_tcu_sregr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_SHUB_CMD_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_shub_cmd_dfsr_s
{
  u32 dfs_en : 1;
  u32 curr_perf_state : 4;
  u32 hw_clk_control : 1;
  u32 dfs_fsm_state : 3;
  u32 perf_state_update_status : 1;
  u32 sw_override : 1;
  u32 sw_perf_state : 4;
  u32 rcg_sw_ctrl : 5;
  u32 reserved0 : 12;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_shub_cmd_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_shub_cmd_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_SHUB_MMU_TCU_PERF0_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_shub_mmu_tcu_perf0_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 21;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_shub_mmu_tcu_perf0_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_shub_mmu_tcu_perf0_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_SHUB_MMU_TCU_PERF1_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_shub_mmu_tcu_perf1_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 21;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_shub_mmu_tcu_perf1_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_shub_mmu_tcu_perf1_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_SHUB_MMU_TCU_PERF2_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_shub_mmu_tcu_perf2_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 21;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_shub_mmu_tcu_perf2_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_shub_mmu_tcu_perf2_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_SHUB_MMU_TCU_PERF3_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_shub_mmu_tcu_perf3_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 21;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_shub_mmu_tcu_perf3_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_shub_mmu_tcu_perf3_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_SHUB_MMU_TCU_PERF4_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_shub_mmu_tcu_perf4_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 21;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_shub_mmu_tcu_perf4_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_shub_mmu_tcu_perf4_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_SHUB_MMU_TCU_PERF5_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_shub_mmu_tcu_perf5_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 21;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_shub_mmu_tcu_perf5_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_shub_mmu_tcu_perf5_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_SHUB_MMU_TCU_PERF6_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_shub_mmu_tcu_perf6_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 21;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_shub_mmu_tcu_perf6_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_shub_mmu_tcu_perf6_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_SHUB_MMU_TCU_PERF7_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_shub_mmu_tcu_perf7_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 21;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_shub_mmu_tcu_perf7_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_shub_mmu_tcu_perf7_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_SHUB_MMU_TCU_PERF8_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_shub_mmu_tcu_perf8_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 21;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_shub_mmu_tcu_perf8_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_shub_mmu_tcu_perf8_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_SHUB_MMU_TCU_PERF9_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_shub_mmu_tcu_perf9_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 21;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_shub_mmu_tcu_perf9_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_shub_mmu_tcu_perf9_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_SHUB_MMU_TCU_PERF10_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_shub_mmu_tcu_perf10_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 21;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_shub_mmu_tcu_perf10_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_shub_mmu_tcu_perf10_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_SHUB_MMU_TCU_PERF11_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_shub_mmu_tcu_perf11_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 21;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_shub_mmu_tcu_perf11_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_shub_mmu_tcu_perf11_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_SHUB_MMU_TCU_PERF12_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_shub_mmu_tcu_perf12_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 21;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_shub_mmu_tcu_perf12_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_shub_mmu_tcu_perf12_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_SHUB_MMU_TCU_PERF13_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_shub_mmu_tcu_perf13_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 21;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_shub_mmu_tcu_perf13_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_shub_mmu_tcu_perf13_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_SHUB_MMU_TCU_PERF14_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_shub_mmu_tcu_perf14_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 21;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_shub_mmu_tcu_perf14_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_shub_mmu_tcu_perf14_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_SHUB_MMU_TCU_PERF15_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_shub_mmu_tcu_perf15_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 21;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_shub_mmu_tcu_perf15_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_shub_mmu_tcu_perf15_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_MMU_TCU_CMD_RCGR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_mmu_tcu_cmd_rcgr_s
{
  u32 update : 1;
  u32 root_en : 1;
  u32 reserved0 : 2;
  u32 dirty_cfg_rcgr : 1;
  u32 reserved1 : 26;
  u32 root_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_mmu_tcu_cmd_rcgr_u
{
  struct ipa_gcc_hwio_def_gcc_mmu_tcu_cmd_rcgr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_MMU_TCU_CFG_RCGR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_mmu_tcu_cfg_rcgr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 5;
  u32 rcglite_disable : 1;
  u32 reserved2 : 3;
  u32 hw_clk_control : 1;
  u32 reserved3 : 11;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_mmu_tcu_cfg_rcgr_u
{
  struct ipa_gcc_hwio_def_gcc_mmu_tcu_cfg_rcgr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_MMU_TCU_DCD_CDIV_DCDR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_mmu_tcu_dcd_cdiv_dcdr_s
{
  u32 dcd_enable : 1;
  u32 reserved0 : 31;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_mmu_tcu_dcd_cdiv_dcdr_u
{
  struct ipa_gcc_hwio_def_gcc_mmu_tcu_dcd_cdiv_dcdr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_ANOC_TBU_BCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_anoc_tbu_bcr_s
{
  u32 blk_ares : 1;
  u32 reserved0 : 31;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_anoc_tbu_bcr_u
{
  struct ipa_gcc_hwio_def_gcc_anoc_tbu_bcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_AGGRE_NOC_TBU1_CBCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_aggre_noc_tbu1_cbcr_s
{
  u32 clk_enable : 1;
  u32 hw_ctl : 1;
  u32 clk_ares : 1;
  u32 sw_only_en : 1;
  u32 sleep : 4;
  u32 wakeup : 4;
  u32 force_mem_periph_off : 1;
  u32 force_mem_periph_on : 1;
  u32 force_mem_core_on : 1;
  u32 reserved0 : 5;
  u32 ignore_rpmh_clk_dis : 1;
  u32 reserved1 : 1;
  u32 clk_dis : 1;
  u32 ignore_all_clk_dis : 1;
  u32 ignore_all_ares : 1;
  u32 reserved2 : 6;
  u32 clk_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_aggre_noc_tbu1_cbcr_u
{
  struct ipa_gcc_hwio_def_gcc_aggre_noc_tbu1_cbcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_AGGRE_NOC_TBU1_SREGR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_aggre_noc_tbu1_sregr_s
{
  u32 reserved0 : 1;
  u32 sw_clk_en_slp_stg : 1;
  u32 sw_clk_en_sel_slp_stg : 1;
  u32 sw_ctrl_pwr_down : 1;
  u32 sw_rst_slp_stg : 1;
  u32 sw_rst_sel_slp_stg : 1;
  u32 force_clk_on : 1;
  u32 mem_cph_enable : 1;
  u32 sw_div_ratio_slp_stg_clk : 2;
  u32 mem_periph_on_ack : 1;
  u32 mem_core_on_ack : 1;
  u32 sw_sm_pscbc_seq_in_override : 1;
  u32 mem_cph_rst_sw_override : 1;
  u32 pscbc_slp_stg_mode_csr : 1;
  u32 ignore_gdsc_pwr_dwn_csr : 1;
  u32 sreg_pscbc_spare_ctrl_in : 8;
  u32 sreg_pscbc_spare_ctrl_out : 8;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_aggre_noc_tbu1_sregr_u
{
  struct ipa_gcc_hwio_def_gcc_aggre_noc_tbu1_sregr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_AGGRE_NOC_TBU2_CBCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_aggre_noc_tbu2_cbcr_s
{
  u32 clk_enable : 1;
  u32 hw_ctl : 1;
  u32 clk_ares : 1;
  u32 sw_only_en : 1;
  u32 sleep : 4;
  u32 wakeup : 4;
  u32 force_mem_periph_off : 1;
  u32 force_mem_periph_on : 1;
  u32 force_mem_core_on : 1;
  u32 reserved0 : 5;
  u32 ignore_rpmh_clk_dis : 1;
  u32 reserved1 : 1;
  u32 clk_dis : 1;
  u32 ignore_all_clk_dis : 1;
  u32 ignore_all_ares : 1;
  u32 reserved2 : 6;
  u32 clk_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_aggre_noc_tbu2_cbcr_u
{
  struct ipa_gcc_hwio_def_gcc_aggre_noc_tbu2_cbcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_AGGRE_NOC_TBU2_SREGR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_aggre_noc_tbu2_sregr_s
{
  u32 reserved0 : 1;
  u32 sw_clk_en_slp_stg : 1;
  u32 sw_clk_en_sel_slp_stg : 1;
  u32 sw_ctrl_pwr_down : 1;
  u32 sw_rst_slp_stg : 1;
  u32 sw_rst_sel_slp_stg : 1;
  u32 force_clk_on : 1;
  u32 mem_cph_enable : 1;
  u32 sw_div_ratio_slp_stg_clk : 2;
  u32 mem_periph_on_ack : 1;
  u32 mem_core_on_ack : 1;
  u32 sw_sm_pscbc_seq_in_override : 1;
  u32 mem_cph_rst_sw_override : 1;
  u32 pscbc_slp_stg_mode_csr : 1;
  u32 ignore_gdsc_pwr_dwn_csr : 1;
  u32 sreg_pscbc_spare_ctrl_in : 8;
  u32 sreg_pscbc_spare_ctrl_out : 8;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_aggre_noc_tbu2_sregr_u
{
  struct ipa_gcc_hwio_def_gcc_aggre_noc_tbu2_sregr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_QDSS_BCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_qdss_bcr_s
{
  u32 blk_ares : 1;
  u32 reserved0 : 31;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_qdss_bcr_u
{
  struct ipa_gcc_hwio_def_gcc_qdss_bcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_QDSS_DAP_AHB_CBCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_qdss_dap_ahb_cbcr_s
{
  u32 clk_enable : 1;
  u32 hw_ctl : 1;
  u32 clk_ares : 1;
  u32 sw_only_en : 1;
  u32 reserved0 : 16;
  u32 ignore_rpmh_clk_dis : 1;
  u32 reserved1 : 1;
  u32 clk_dis : 1;
  u32 ignore_all_clk_dis : 1;
  u32 ignore_all_ares : 1;
  u32 reserved2 : 6;
  u32 clk_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_qdss_dap_ahb_cbcr_u
{
  struct ipa_gcc_hwio_def_gcc_qdss_dap_ahb_cbcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_QDSS_CFG_AHB_CBCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_qdss_cfg_ahb_cbcr_s
{
  u32 reserved0 : 1;
  u32 hw_ctl : 1;
  u32 clk_ares : 1;
  u32 sw_only_en : 1;
  u32 reserved1 : 16;
  u32 ignore_rpmh_clk_dis : 1;
  u32 reserved2 : 1;
  u32 clk_dis : 1;
  u32 ignore_all_clk_dis : 1;
  u32 ignore_all_ares : 1;
  u32 reserved3 : 6;
  u32 clk_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_qdss_cfg_ahb_cbcr_u
{
  struct ipa_gcc_hwio_def_gcc_qdss_cfg_ahb_cbcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_QDSS_AT_CBCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_qdss_at_cbcr_s
{
  u32 clk_enable : 1;
  u32 hw_ctl : 1;
  u32 clk_ares : 1;
  u32 sw_only_en : 1;
  u32 reserved0 : 16;
  u32 ignore_rpmh_clk_dis : 1;
  u32 reserved1 : 1;
  u32 clk_dis : 1;
  u32 ignore_all_clk_dis : 1;
  u32 ignore_all_ares : 1;
  u32 reserved2 : 6;
  u32 clk_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_qdss_at_cbcr_u
{
  struct ipa_gcc_hwio_def_gcc_qdss_at_cbcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_QDSS_ETR_USB_CBCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_qdss_etr_usb_cbcr_s
{
  u32 clk_enable : 1;
  u32 hw_ctl : 1;
  u32 clk_ares : 1;
  u32 sw_only_en : 1;
  u32 reserved0 : 16;
  u32 ignore_rpmh_clk_dis : 1;
  u32 reserved1 : 1;
  u32 clk_dis : 1;
  u32 ignore_all_clk_dis : 1;
  u32 ignore_all_ares : 1;
  u32 reserved2 : 6;
  u32 clk_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_qdss_etr_usb_cbcr_u
{
  struct ipa_gcc_hwio_def_gcc_qdss_etr_usb_cbcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_QDSS_STM_CBCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_qdss_stm_cbcr_s
{
  u32 clk_enable : 1;
  u32 hw_ctl : 1;
  u32 clk_ares : 1;
  u32 sw_only_en : 1;
  u32 reserved0 : 16;
  u32 ignore_rpmh_clk_dis : 1;
  u32 reserved1 : 1;
  u32 clk_dis : 1;
  u32 ignore_all_clk_dis : 1;
  u32 ignore_all_ares : 1;
  u32 reserved2 : 6;
  u32 clk_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_qdss_stm_cbcr_u
{
  struct ipa_gcc_hwio_def_gcc_qdss_stm_cbcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_QDSS_TRACECLKIN_CBCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_qdss_traceclkin_cbcr_s
{
  u32 clk_enable : 1;
  u32 hw_ctl : 1;
  u32 clk_ares : 1;
  u32 sw_only_en : 1;
  u32 reserved0 : 16;
  u32 ignore_rpmh_clk_dis : 1;
  u32 reserved1 : 1;
  u32 clk_dis : 1;
  u32 ignore_all_clk_dis : 1;
  u32 ignore_all_ares : 1;
  u32 reserved2 : 6;
  u32 clk_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_qdss_traceclkin_cbcr_u
{
  struct ipa_gcc_hwio_def_gcc_qdss_traceclkin_cbcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_QDSS_TSCTR_CBCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_qdss_tsctr_cbcr_s
{
  u32 clk_enable : 1;
  u32 hw_ctl : 1;
  u32 clk_ares : 1;
  u32 sw_only_en : 1;
  u32 reserved0 : 18;
  u32 clk_dis : 1;
  u32 ignore_all_clk_dis : 1;
  u32 ignore_all_ares : 1;
  u32 reserved1 : 6;
  u32 clk_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_qdss_tsctr_cbcr_u
{
  struct ipa_gcc_hwio_def_gcc_qdss_tsctr_cbcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_QDSS_TRIG_CBCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_qdss_trig_cbcr_s
{
  u32 clk_enable : 1;
  u32 hw_ctl : 1;
  u32 clk_ares : 1;
  u32 sw_only_en : 1;
  u32 reserved0 : 16;
  u32 ignore_rpmh_clk_dis : 1;
  u32 reserved1 : 1;
  u32 clk_dis : 1;
  u32 ignore_all_clk_dis : 1;
  u32 ignore_all_ares : 1;
  u32 reserved2 : 6;
  u32 clk_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_qdss_trig_cbcr_u
{
  struct ipa_gcc_hwio_def_gcc_qdss_trig_cbcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_QDSS_DAP_CBCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_qdss_dap_cbcr_s
{
  u32 clk_enable : 1;
  u32 hw_ctl : 1;
  u32 clk_ares : 1;
  u32 sw_only_en : 1;
  u32 reserved0 : 18;
  u32 clk_dis : 1;
  u32 ignore_all_clk_dis : 1;
  u32 ignore_all_ares : 1;
  u32 reserved1 : 6;
  u32 clk_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_qdss_dap_cbcr_u
{
  struct ipa_gcc_hwio_def_gcc_qdss_dap_cbcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_APB_CBCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_apb_cbcr_s
{
  u32 clk_enable : 1;
  u32 hw_ctl : 1;
  u32 clk_ares : 1;
  u32 sw_only_en : 1;
  u32 reserved0 : 18;
  u32 clk_dis : 1;
  u32 ignore_all_clk_dis : 1;
  u32 ignore_all_ares : 1;
  u32 reserved1 : 6;
  u32 clk_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_apb_cbcr_u
{
  struct ipa_gcc_hwio_def_gcc_apb_cbcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_QDSS_XO_CBCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_qdss_xo_cbcr_s
{
  u32 clk_enable : 1;
  u32 hw_ctl : 1;
  u32 clk_ares : 1;
  u32 sw_only_en : 1;
  u32 reserved0 : 18;
  u32 clk_dis : 1;
  u32 ignore_all_clk_dis : 1;
  u32 ignore_all_ares : 1;
  u32 reserved1 : 6;
  u32 clk_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_qdss_xo_cbcr_u
{
  struct ipa_gcc_hwio_def_gcc_qdss_xo_cbcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_SYS_NOC_QDSS_STM_PERF0_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_sys_noc_qdss_stm_perf0_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 21;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_sys_noc_qdss_stm_perf0_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_sys_noc_qdss_stm_perf0_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_SYS_NOC_QDSS_STM_PERF1_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_sys_noc_qdss_stm_perf1_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 21;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_sys_noc_qdss_stm_perf1_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_sys_noc_qdss_stm_perf1_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_SYS_NOC_QDSS_STM_PERF2_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_sys_noc_qdss_stm_perf2_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 21;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_sys_noc_qdss_stm_perf2_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_sys_noc_qdss_stm_perf2_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_SYS_NOC_QDSS_STM_PERF3_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_sys_noc_qdss_stm_perf3_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 21;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_sys_noc_qdss_stm_perf3_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_sys_noc_qdss_stm_perf3_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_SYS_NOC_QDSS_STM_PERF4_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_sys_noc_qdss_stm_perf4_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 21;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_sys_noc_qdss_stm_perf4_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_sys_noc_qdss_stm_perf4_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_SYS_NOC_QDSS_STM_PERF5_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_sys_noc_qdss_stm_perf5_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 21;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_sys_noc_qdss_stm_perf5_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_sys_noc_qdss_stm_perf5_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_SYS_NOC_QDSS_STM_PERF6_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_sys_noc_qdss_stm_perf6_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 21;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_sys_noc_qdss_stm_perf6_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_sys_noc_qdss_stm_perf6_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_SYS_NOC_QDSS_STM_PERF7_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_sys_noc_qdss_stm_perf7_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 21;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_sys_noc_qdss_stm_perf7_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_sys_noc_qdss_stm_perf7_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_SYS_NOC_QDSS_STM_PERF8_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_sys_noc_qdss_stm_perf8_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 21;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_sys_noc_qdss_stm_perf8_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_sys_noc_qdss_stm_perf8_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_SYS_NOC_QDSS_STM_PERF9_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_sys_noc_qdss_stm_perf9_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 21;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_sys_noc_qdss_stm_perf9_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_sys_noc_qdss_stm_perf9_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_SYS_NOC_QDSS_STM_PERF10_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_sys_noc_qdss_stm_perf10_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 21;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_sys_noc_qdss_stm_perf10_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_sys_noc_qdss_stm_perf10_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_SYS_NOC_QDSS_STM_PERF11_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_sys_noc_qdss_stm_perf11_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 21;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_sys_noc_qdss_stm_perf11_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_sys_noc_qdss_stm_perf11_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_SYS_NOC_QDSS_STM_PERF12_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_sys_noc_qdss_stm_perf12_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 21;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_sys_noc_qdss_stm_perf12_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_sys_noc_qdss_stm_perf12_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_SYS_NOC_QDSS_STM_PERF13_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_sys_noc_qdss_stm_perf13_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 21;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_sys_noc_qdss_stm_perf13_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_sys_noc_qdss_stm_perf13_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_SYS_NOC_QDSS_STM_PERF14_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_sys_noc_qdss_stm_perf14_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 21;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_sys_noc_qdss_stm_perf14_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_sys_noc_qdss_stm_perf14_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_SYS_NOC_QDSS_STM_PERF15_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_sys_noc_qdss_stm_perf15_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 21;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_sys_noc_qdss_stm_perf15_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_sys_noc_qdss_stm_perf15_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_QDSS_STM_CMD_RCGR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_qdss_stm_cmd_rcgr_s
{
  u32 update : 1;
  u32 root_en : 1;
  u32 reserved0 : 2;
  u32 dirty_cfg_rcgr : 1;
  u32 reserved1 : 26;
  u32 root_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_qdss_stm_cmd_rcgr_u
{
  struct ipa_gcc_hwio_def_gcc_qdss_stm_cmd_rcgr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_QDSS_STM_CFG_RCGR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_qdss_stm_cfg_rcgr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 5;
  u32 rcglite_disable : 1;
  u32 reserved2 : 3;
  u32 hw_clk_control : 1;
  u32 reserved3 : 11;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_qdss_stm_cfg_rcgr_u
{
  struct ipa_gcc_hwio_def_gcc_qdss_stm_cfg_rcgr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_SHUB_QDSS_TRACECLKIN_PERF0_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_shub_qdss_traceclkin_perf0_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 21;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_shub_qdss_traceclkin_perf0_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_shub_qdss_traceclkin_perf0_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_SHUB_QDSS_TRACECLKIN_PERF1_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_shub_qdss_traceclkin_perf1_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 21;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_shub_qdss_traceclkin_perf1_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_shub_qdss_traceclkin_perf1_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_SHUB_QDSS_TRACECLKIN_PERF2_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_shub_qdss_traceclkin_perf2_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 21;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_shub_qdss_traceclkin_perf2_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_shub_qdss_traceclkin_perf2_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_SHUB_QDSS_TRACECLKIN_PERF3_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_shub_qdss_traceclkin_perf3_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 21;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_shub_qdss_traceclkin_perf3_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_shub_qdss_traceclkin_perf3_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_SHUB_QDSS_TRACECLKIN_PERF4_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_shub_qdss_traceclkin_perf4_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 21;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_shub_qdss_traceclkin_perf4_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_shub_qdss_traceclkin_perf4_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_SHUB_QDSS_TRACECLKIN_PERF5_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_shub_qdss_traceclkin_perf5_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 21;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_shub_qdss_traceclkin_perf5_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_shub_qdss_traceclkin_perf5_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_SHUB_QDSS_TRACECLKIN_PERF6_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_shub_qdss_traceclkin_perf6_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 21;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_shub_qdss_traceclkin_perf6_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_shub_qdss_traceclkin_perf6_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_SHUB_QDSS_TRACECLKIN_PERF7_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_shub_qdss_traceclkin_perf7_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 21;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_shub_qdss_traceclkin_perf7_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_shub_qdss_traceclkin_perf7_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_SHUB_QDSS_TRACECLKIN_PERF8_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_shub_qdss_traceclkin_perf8_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 21;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_shub_qdss_traceclkin_perf8_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_shub_qdss_traceclkin_perf8_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_SHUB_QDSS_TRACECLKIN_PERF9_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_shub_qdss_traceclkin_perf9_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 21;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_shub_qdss_traceclkin_perf9_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_shub_qdss_traceclkin_perf9_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_SHUB_QDSS_TRACECLKIN_PERF10_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_shub_qdss_traceclkin_perf10_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 21;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_shub_qdss_traceclkin_perf10_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_shub_qdss_traceclkin_perf10_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_SHUB_QDSS_TRACECLKIN_PERF11_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_shub_qdss_traceclkin_perf11_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 21;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_shub_qdss_traceclkin_perf11_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_shub_qdss_traceclkin_perf11_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_SHUB_QDSS_TRACECLKIN_PERF12_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_shub_qdss_traceclkin_perf12_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 21;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_shub_qdss_traceclkin_perf12_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_shub_qdss_traceclkin_perf12_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_SHUB_QDSS_TRACECLKIN_PERF13_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_shub_qdss_traceclkin_perf13_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 21;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_shub_qdss_traceclkin_perf13_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_shub_qdss_traceclkin_perf13_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_SHUB_QDSS_TRACECLKIN_PERF14_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_shub_qdss_traceclkin_perf14_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 21;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_shub_qdss_traceclkin_perf14_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_shub_qdss_traceclkin_perf14_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_SHUB_QDSS_TRACECLKIN_PERF15_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_shub_qdss_traceclkin_perf15_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 21;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_shub_qdss_traceclkin_perf15_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_shub_qdss_traceclkin_perf15_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_QDSS_TRACECLKIN_CMD_RCGR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_qdss_traceclkin_cmd_rcgr_s
{
  u32 update : 1;
  u32 root_en : 1;
  u32 reserved0 : 2;
  u32 dirty_cfg_rcgr : 1;
  u32 reserved1 : 26;
  u32 root_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_qdss_traceclkin_cmd_rcgr_u
{
  struct ipa_gcc_hwio_def_gcc_qdss_traceclkin_cmd_rcgr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_QDSS_TRACECLKIN_CFG_RCGR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_qdss_traceclkin_cfg_rcgr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 5;
  u32 rcglite_disable : 1;
  u32 reserved2 : 3;
  u32 hw_clk_control : 1;
  u32 reserved3 : 11;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_qdss_traceclkin_cfg_rcgr_u
{
  struct ipa_gcc_hwio_def_gcc_qdss_traceclkin_cfg_rcgr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_QDSS_APB_TSCTR_CMD_RCGR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_qdss_apb_tsctr_cmd_rcgr_s
{
  u32 update : 1;
  u32 root_en : 1;
  u32 reserved0 : 2;
  u32 dirty_cfg_rcgr : 1;
  u32 reserved1 : 26;
  u32 root_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_qdss_apb_tsctr_cmd_rcgr_u
{
  struct ipa_gcc_hwio_def_gcc_qdss_apb_tsctr_cmd_rcgr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_QDSS_APB_TSCTR_CFG_RCGR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_qdss_apb_tsctr_cfg_rcgr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 5;
  u32 rcglite_disable : 1;
  u32 reserved2 : 3;
  u32 hw_clk_control : 1;
  u32 reserved3 : 11;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_qdss_apb_tsctr_cfg_rcgr_u
{
  struct ipa_gcc_hwio_def_gcc_qdss_apb_tsctr_cfg_rcgr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_SYS_NOC_QDSS_TRIG_PERF0_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_sys_noc_qdss_trig_perf0_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 21;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_sys_noc_qdss_trig_perf0_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_sys_noc_qdss_trig_perf0_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_SYS_NOC_QDSS_TRIG_PERF1_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_sys_noc_qdss_trig_perf1_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 21;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_sys_noc_qdss_trig_perf1_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_sys_noc_qdss_trig_perf1_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_SYS_NOC_QDSS_TRIG_PERF2_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_sys_noc_qdss_trig_perf2_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 21;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_sys_noc_qdss_trig_perf2_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_sys_noc_qdss_trig_perf2_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_SYS_NOC_QDSS_TRIG_PERF3_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_sys_noc_qdss_trig_perf3_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 21;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_sys_noc_qdss_trig_perf3_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_sys_noc_qdss_trig_perf3_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_SYS_NOC_QDSS_TRIG_PERF4_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_sys_noc_qdss_trig_perf4_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 21;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_sys_noc_qdss_trig_perf4_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_sys_noc_qdss_trig_perf4_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_SYS_NOC_QDSS_TRIG_PERF5_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_sys_noc_qdss_trig_perf5_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 21;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_sys_noc_qdss_trig_perf5_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_sys_noc_qdss_trig_perf5_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_SYS_NOC_QDSS_TRIG_PERF6_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_sys_noc_qdss_trig_perf6_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 21;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_sys_noc_qdss_trig_perf6_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_sys_noc_qdss_trig_perf6_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_SYS_NOC_QDSS_TRIG_PERF7_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_sys_noc_qdss_trig_perf7_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 21;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_sys_noc_qdss_trig_perf7_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_sys_noc_qdss_trig_perf7_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_SYS_NOC_QDSS_TRIG_PERF8_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_sys_noc_qdss_trig_perf8_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 21;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_sys_noc_qdss_trig_perf8_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_sys_noc_qdss_trig_perf8_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_SYS_NOC_QDSS_TRIG_PERF9_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_sys_noc_qdss_trig_perf9_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 21;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_sys_noc_qdss_trig_perf9_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_sys_noc_qdss_trig_perf9_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_SYS_NOC_QDSS_TRIG_PERF10_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_sys_noc_qdss_trig_perf10_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 21;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_sys_noc_qdss_trig_perf10_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_sys_noc_qdss_trig_perf10_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_SYS_NOC_QDSS_TRIG_PERF11_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_sys_noc_qdss_trig_perf11_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 21;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_sys_noc_qdss_trig_perf11_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_sys_noc_qdss_trig_perf11_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_SYS_NOC_QDSS_TRIG_PERF12_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_sys_noc_qdss_trig_perf12_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 21;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_sys_noc_qdss_trig_perf12_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_sys_noc_qdss_trig_perf12_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_SYS_NOC_QDSS_TRIG_PERF13_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_sys_noc_qdss_trig_perf13_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 21;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_sys_noc_qdss_trig_perf13_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_sys_noc_qdss_trig_perf13_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_SYS_NOC_QDSS_TRIG_PERF14_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_sys_noc_qdss_trig_perf14_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 21;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_sys_noc_qdss_trig_perf14_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_sys_noc_qdss_trig_perf14_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_SYS_NOC_QDSS_TRIG_PERF15_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_sys_noc_qdss_trig_perf15_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 21;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_sys_noc_qdss_trig_perf15_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_sys_noc_qdss_trig_perf15_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_QDSS_TRIG_CMD_RCGR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_qdss_trig_cmd_rcgr_s
{
  u32 update : 1;
  u32 root_en : 1;
  u32 reserved0 : 2;
  u32 dirty_cfg_rcgr : 1;
  u32 reserved1 : 26;
  u32 root_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_qdss_trig_cmd_rcgr_u
{
  struct ipa_gcc_hwio_def_gcc_qdss_trig_cmd_rcgr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_QDSS_TRIG_CFG_RCGR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_qdss_trig_cfg_rcgr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 5;
  u32 rcglite_disable : 1;
  u32 reserved2 : 3;
  u32 hw_clk_control : 1;
  u32 reserved3 : 11;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_qdss_trig_cfg_rcgr_u
{
  struct ipa_gcc_hwio_def_gcc_qdss_trig_cfg_rcgr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_SHUB_QDSS_AT_PERF0_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_shub_qdss_at_perf0_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 21;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_shub_qdss_at_perf0_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_shub_qdss_at_perf0_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_SHUB_QDSS_AT_PERF1_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_shub_qdss_at_perf1_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 21;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_shub_qdss_at_perf1_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_shub_qdss_at_perf1_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_SHUB_QDSS_AT_PERF2_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_shub_qdss_at_perf2_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 21;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_shub_qdss_at_perf2_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_shub_qdss_at_perf2_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_SHUB_QDSS_AT_PERF3_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_shub_qdss_at_perf3_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 21;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_shub_qdss_at_perf3_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_shub_qdss_at_perf3_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_SHUB_QDSS_AT_PERF4_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_shub_qdss_at_perf4_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 21;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_shub_qdss_at_perf4_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_shub_qdss_at_perf4_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_SHUB_QDSS_AT_PERF5_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_shub_qdss_at_perf5_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 21;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_shub_qdss_at_perf5_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_shub_qdss_at_perf5_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_SHUB_QDSS_AT_PERF6_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_shub_qdss_at_perf6_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 21;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_shub_qdss_at_perf6_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_shub_qdss_at_perf6_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_SHUB_QDSS_AT_PERF7_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_shub_qdss_at_perf7_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 21;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_shub_qdss_at_perf7_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_shub_qdss_at_perf7_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_SHUB_QDSS_AT_PERF8_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_shub_qdss_at_perf8_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 21;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_shub_qdss_at_perf8_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_shub_qdss_at_perf8_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_SHUB_QDSS_AT_PERF9_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_shub_qdss_at_perf9_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 21;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_shub_qdss_at_perf9_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_shub_qdss_at_perf9_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_SHUB_QDSS_AT_PERF10_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_shub_qdss_at_perf10_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 21;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_shub_qdss_at_perf10_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_shub_qdss_at_perf10_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_SHUB_QDSS_AT_PERF11_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_shub_qdss_at_perf11_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 21;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_shub_qdss_at_perf11_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_shub_qdss_at_perf11_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_SHUB_QDSS_AT_PERF12_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_shub_qdss_at_perf12_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 21;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_shub_qdss_at_perf12_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_shub_qdss_at_perf12_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_SHUB_QDSS_AT_PERF13_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_shub_qdss_at_perf13_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 21;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_shub_qdss_at_perf13_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_shub_qdss_at_perf13_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_SHUB_QDSS_AT_PERF14_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_shub_qdss_at_perf14_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 21;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_shub_qdss_at_perf14_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_shub_qdss_at_perf14_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_SHUB_QDSS_AT_PERF15_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_shub_qdss_at_perf15_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 21;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_shub_qdss_at_perf15_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_shub_qdss_at_perf15_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_QDSS_AT_CMD_RCGR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_qdss_at_cmd_rcgr_s
{
  u32 update : 1;
  u32 root_en : 1;
  u32 reserved0 : 2;
  u32 dirty_cfg_rcgr : 1;
  u32 reserved1 : 26;
  u32 root_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_qdss_at_cmd_rcgr_u
{
  struct ipa_gcc_hwio_def_gcc_qdss_at_cmd_rcgr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_QDSS_AT_CFG_RCGR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_qdss_at_cfg_rcgr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 5;
  u32 rcglite_disable : 1;
  u32 reserved2 : 3;
  u32 hw_clk_control : 1;
  u32 reserved3 : 11;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_qdss_at_cfg_rcgr_u
{
  struct ipa_gcc_hwio_def_gcc_qdss_at_cfg_rcgr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_USB30_BCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_usb30_bcr_s
{
  u32 blk_ares : 1;
  u32 reserved0 : 31;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_usb30_bcr_u
{
  struct ipa_gcc_hwio_def_gcc_usb30_bcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_USB30_GDSCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_usb30_gdscr_s
{
  u32 sw_collapse : 1;
  u32 hw_control : 1;
  u32 sw_override : 1;
  u32 pd_ares : 1;
  u32 clk_disable : 1;
  u32 clamp_io : 1;
  u32 en_few : 1;
  u32 en_rest : 1;
  u32 retain : 1;
  u32 save : 1;
  u32 restore : 1;
  u32 retain_ff_enable : 1;
  u32 clk_dis_wait : 4;
  u32 en_few_wait : 4;
  u32 en_rest_wait : 4;
  u32 reserved0 : 3;
  u32 gdsc_state : 4;
  u32 pwr_on : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_usb30_gdscr_u
{
  struct ipa_gcc_hwio_def_gcc_usb30_gdscr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_USB30_CFG_GDSCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_usb30_cfg_gdscr_s
{
  u32 disable_clk_software_override : 1;
  u32 clamp_io_software_override : 1;
  u32 save_restore_software_override : 1;
  u32 unclamp_io_software_override : 1;
  u32 gdsc_pscbc_pwr_dwn_sw : 1;
  u32 gdsc_phase_reset_delay_count_sw : 2;
  u32 gdsc_phase_reset_en_sw : 1;
  u32 gdsc_mem_core_force_in_sw : 1;
  u32 gdsc_mem_peri_force_in_sw : 1;
  u32 gdsc_handshake_dis : 1;
  u32 software_control_override : 4;
  u32 gdsc_power_down_complete : 1;
  u32 gdsc_power_up_complete : 1;
  u32 gdsc_enf_ack_status : 1;
  u32 gdsc_enr_ack_status : 1;
  u32 gdsc_mem_pwr_ack_status : 1;
  u32 gdsc_cfg_fsm_state_status : 4;
  u32 gdsc_pwr_up_start : 1;
  u32 gdsc_pwr_dwn_start : 1;
  u32 reserved0 : 6;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_usb30_cfg_gdscr_u
{
  struct ipa_gcc_hwio_def_gcc_usb30_cfg_gdscr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_USB30_CFG2_GDSCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_usb30_cfg2_gdscr_s
{
  u32 mem_pwr_dwn_timeout : 4;
  u32 dly_assert_clamp_mem : 4;
  u32 dly_deassert_clamp_mem : 4;
  u32 dly_mem_pwr_up : 4;
  u32 gdsc_clamp_mem_sw : 1;
  u32 gdsc_pwrdwn_enable_ack_override : 1;
  u32 gdsc_mem_pwrup_ack_override : 1;
  u32 reserved0 : 13;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_usb30_cfg2_gdscr_u
{
  struct ipa_gcc_hwio_def_gcc_usb30_cfg2_gdscr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_USB30_CFG3_GDSCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_usb30_cfg3_gdscr_s
{
  u32 gdsc_spare_ctrl_out : 8;
  u32 gdsc_spare_ctrl_in : 8;
  u32 gdsc_accu_red_sw_override : 1;
  u32 gdsc_accu_red_shifter_start_sw : 1;
  u32 gdsc_accu_red_shifter_clk_en_sw : 1;
  u32 gdsc_accu_red_shifter_done_override : 1;
  u32 gdsc_accu_red_timer_en_sw : 1;
  u32 dly_accu_red_shifter_done : 4;
  u32 gdsc_accu_red_enable : 1;
  u32 gdsc_accu_red_shifter_done_status : 1;
  u32 reserved0 : 5;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_usb30_cfg3_gdscr_u
{
  struct ipa_gcc_hwio_def_gcc_usb30_cfg3_gdscr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_USB30_CFG4_GDSCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_usb30_cfg4_gdscr_s
{
  u32 dly_retainff : 4;
  u32 dly_clampio : 4;
  u32 dly_deassertares : 4;
  u32 dly_noretainff : 4;
  u32 dly_restoreff : 4;
  u32 dly_unclampio : 4;
  u32 reserved0 : 8;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_usb30_cfg4_gdscr_u
{
  struct ipa_gcc_hwio_def_gcc_usb30_cfg4_gdscr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_USB30_MASTER_CBCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_usb30_master_cbcr_s
{
  u32 clk_enable : 1;
  u32 reserved0 : 1;
  u32 clk_ares : 1;
  u32 reserved1 : 1;
  u32 sleep : 4;
  u32 wakeup : 4;
  u32 force_mem_periph_off : 1;
  u32 force_mem_periph_on : 1;
  u32 force_mem_core_on : 1;
  u32 reserved2 : 7;
  u32 clk_dis : 1;
  u32 ignore_all_clk_dis : 1;
  u32 ignore_all_ares : 1;
  u32 reserved3 : 6;
  u32 clk_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_usb30_master_cbcr_u
{
  struct ipa_gcc_hwio_def_gcc_usb30_master_cbcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_USB30_MASTER_SREGR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_usb30_master_sregr_s
{
  u32 reserved0 : 1;
  u32 sw_clk_en_slp_stg : 1;
  u32 sw_clk_en_sel_slp_stg : 1;
  u32 sw_ctrl_pwr_down : 1;
  u32 sw_rst_slp_stg : 1;
  u32 sw_rst_sel_slp_stg : 1;
  u32 force_clk_on : 1;
  u32 mem_cph_enable : 1;
  u32 sw_div_ratio_slp_stg_clk : 2;
  u32 mem_periph_on_ack : 1;
  u32 mem_core_on_ack : 1;
  u32 sw_sm_pscbc_seq_in_override : 1;
  u32 mem_cph_rst_sw_override : 1;
  u32 pscbc_slp_stg_mode_csr : 1;
  u32 ignore_gdsc_pwr_dwn_csr : 1;
  u32 sreg_pscbc_spare_ctrl_in : 8;
  u32 sreg_pscbc_spare_ctrl_out : 8;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_usb30_master_sregr_u
{
  struct ipa_gcc_hwio_def_gcc_usb30_master_sregr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_USB30_MSTR_AXI_CBCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_usb30_mstr_axi_cbcr_s
{
  u32 clk_enable : 1;
  u32 reserved0 : 1;
  u32 clk_ares : 1;
  u32 reserved1 : 17;
  u32 ignore_rpmh_clk_dis : 1;
  u32 reserved2 : 1;
  u32 clk_dis : 1;
  u32 ignore_all_clk_dis : 1;
  u32 ignore_all_ares : 1;
  u32 reserved3 : 6;
  u32 clk_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_usb30_mstr_axi_cbcr_u
{
  struct ipa_gcc_hwio_def_gcc_usb30_mstr_axi_cbcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_USB30_SLV_AHB_CBCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_usb30_slv_ahb_cbcr_s
{
  u32 clk_enable : 1;
  u32 reserved0 : 1;
  u32 clk_ares : 1;
  u32 reserved1 : 17;
  u32 ignore_rpmh_clk_dis : 1;
  u32 reserved2 : 1;
  u32 clk_dis : 1;
  u32 ignore_all_clk_dis : 1;
  u32 ignore_all_ares : 1;
  u32 reserved3 : 6;
  u32 clk_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_usb30_slv_ahb_cbcr_u
{
  struct ipa_gcc_hwio_def_gcc_usb30_slv_ahb_cbcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_USB30_SLEEP_CBCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_usb30_sleep_cbcr_s
{
  u32 clk_enable : 1;
  u32 reserved0 : 1;
  u32 clk_ares : 1;
  u32 reserved1 : 19;
  u32 clk_dis : 1;
  u32 ignore_all_clk_dis : 1;
  u32 ignore_all_ares : 1;
  u32 reserved2 : 6;
  u32 clk_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_usb30_sleep_cbcr_u
{
  struct ipa_gcc_hwio_def_gcc_usb30_sleep_cbcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_USB30_MOCK_UTMI_CBCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_usb30_mock_utmi_cbcr_s
{
  u32 clk_enable : 1;
  u32 reserved0 : 1;
  u32 clk_ares : 1;
  u32 reserved1 : 19;
  u32 clk_dis : 1;
  u32 ignore_all_clk_dis : 1;
  u32 ignore_all_ares : 1;
  u32 reserved2 : 6;
  u32 clk_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_usb30_mock_utmi_cbcr_u
{
  struct ipa_gcc_hwio_def_gcc_usb30_mock_utmi_cbcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_USB30_MASTER_CMD_RCGR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_usb30_master_cmd_rcgr_s
{
  u32 update : 1;
  u32 root_en : 1;
  u32 reserved0 : 2;
  u32 dirty_cfg_rcgr : 1;
  u32 dirty_m : 1;
  u32 dirty_n : 1;
  u32 dirty_d : 1;
  u32 reserved1 : 23;
  u32 root_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_usb30_master_cmd_rcgr_u
{
  struct ipa_gcc_hwio_def_gcc_usb30_master_cmd_rcgr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_USB30_MASTER_CFG_RCGR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_usb30_master_cfg_rcgr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 1;
  u32 mode : 2;
  u32 reserved2 : 6;
  u32 hw_clk_control : 1;
  u32 reserved3 : 11;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_usb30_master_cfg_rcgr_u
{
  struct ipa_gcc_hwio_def_gcc_usb30_master_cfg_rcgr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_USB30_MASTER_M
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_usb30_master_m_s
{
  u32 m : 8;
  u32 reserved0 : 24;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_usb30_master_m_u
{
  struct ipa_gcc_hwio_def_gcc_usb30_master_m_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_USB30_MASTER_N
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_usb30_master_n_s
{
  u32 not_n_minus_m : 8;
  u32 reserved0 : 24;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_usb30_master_n_u
{
  struct ipa_gcc_hwio_def_gcc_usb30_master_n_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_USB30_MASTER_D
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_usb30_master_d_s
{
  u32 not_2d : 8;
  u32 reserved0 : 24;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_usb30_master_d_u
{
  struct ipa_gcc_hwio_def_gcc_usb30_master_d_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_USB30_MOCK_UTMI_CMD_RCGR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_usb30_mock_utmi_cmd_rcgr_s
{
  u32 update : 1;
  u32 root_en : 1;
  u32 reserved0 : 2;
  u32 dirty_cfg_rcgr : 1;
  u32 reserved1 : 26;
  u32 root_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_usb30_mock_utmi_cmd_rcgr_u
{
  struct ipa_gcc_hwio_def_gcc_usb30_mock_utmi_cmd_rcgr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_USB30_MOCK_UTMI_CFG_RCGR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_usb30_mock_utmi_cfg_rcgr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 5;
  u32 rcglite_disable : 1;
  u32 reserved2 : 3;
  u32 hw_clk_control : 1;
  u32 reserved3 : 11;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_usb30_mock_utmi_cfg_rcgr_u
{
  struct ipa_gcc_hwio_def_gcc_usb30_mock_utmi_cfg_rcgr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_USB30_MOCK_UTMI_POSTDIV_CDIVR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_usb30_mock_utmi_postdiv_cdivr_s
{
  u32 clk_div : 4;
  u32 reserved0 : 28;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_usb30_mock_utmi_postdiv_cdivr_u
{
  struct ipa_gcc_hwio_def_gcc_usb30_mock_utmi_postdiv_cdivr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_USB3_PHY_AUX_CBCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_usb3_phy_aux_cbcr_s
{
  u32 clk_enable : 1;
  u32 reserved0 : 1;
  u32 clk_ares : 1;
  u32 reserved1 : 19;
  u32 clk_dis : 1;
  u32 ignore_all_clk_dis : 1;
  u32 ignore_all_ares : 1;
  u32 reserved2 : 6;
  u32 clk_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_usb3_phy_aux_cbcr_u
{
  struct ipa_gcc_hwio_def_gcc_usb3_phy_aux_cbcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_USB3_PHY_PIPE_CBCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_usb3_phy_pipe_cbcr_s
{
  u32 clk_enable : 1;
  u32 hw_ctl : 1;
  u32 clk_ares : 1;
  u32 sw_only_en : 1;
  u32 reserved0 : 18;
  u32 clk_dis : 1;
  u32 ignore_all_clk_dis : 1;
  u32 ignore_all_ares : 1;
  u32 reserved1 : 6;
  u32 clk_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_usb3_phy_pipe_cbcr_u
{
  struct ipa_gcc_hwio_def_gcc_usb3_phy_pipe_cbcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_USB3_PHY_AUX_CMD_RCGR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_usb3_phy_aux_cmd_rcgr_s
{
  u32 update : 1;
  u32 root_en : 1;
  u32 reserved0 : 2;
  u32 dirty_cfg_rcgr : 1;
  u32 dirty_m : 1;
  u32 dirty_n : 1;
  u32 dirty_d : 1;
  u32 reserved1 : 23;
  u32 root_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_usb3_phy_aux_cmd_rcgr_u
{
  struct ipa_gcc_hwio_def_gcc_usb3_phy_aux_cmd_rcgr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_USB3_PHY_AUX_CFG_RCGR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_usb3_phy_aux_cfg_rcgr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 1;
  u32 mode : 2;
  u32 reserved2 : 6;
  u32 hw_clk_control : 1;
  u32 reserved3 : 11;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_usb3_phy_aux_cfg_rcgr_u
{
  struct ipa_gcc_hwio_def_gcc_usb3_phy_aux_cfg_rcgr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_USB3_PHY_AUX_M
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_usb3_phy_aux_m_s
{
  u32 m : 16;
  u32 reserved0 : 16;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_usb3_phy_aux_m_u
{
  struct ipa_gcc_hwio_def_gcc_usb3_phy_aux_m_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_USB3_PHY_AUX_N
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_usb3_phy_aux_n_s
{
  u32 not_n_minus_m : 16;
  u32 reserved0 : 16;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_usb3_phy_aux_n_u
{
  struct ipa_gcc_hwio_def_gcc_usb3_phy_aux_n_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_USB3_PHY_AUX_D
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_usb3_phy_aux_d_s
{
  u32 not_2d : 16;
  u32 reserved0 : 16;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_usb3_phy_aux_d_u
{
  struct ipa_gcc_hwio_def_gcc_usb3_phy_aux_d_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_USB3_PHY_BCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_usb3_phy_bcr_s
{
  u32 blk_ares : 1;
  u32 reserved0 : 31;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_usb3_phy_bcr_u
{
  struct ipa_gcc_hwio_def_gcc_usb3_phy_bcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_USB3PHY_PHY_BCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_usb3phy_phy_bcr_s
{
  u32 blk_ares : 1;
  u32 reserved0 : 31;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_usb3phy_phy_bcr_u
{
  struct ipa_gcc_hwio_def_gcc_usb3phy_phy_bcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_QUSB2PHY_BCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_qusb2phy_bcr_s
{
  u32 blk_ares : 1;
  u32 reserved0 : 31;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_qusb2phy_bcr_u
{
  struct ipa_gcc_hwio_def_gcc_qusb2phy_bcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_USB_PHY_CFG_AHB2PHY_BCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_usb_phy_cfg_ahb2phy_bcr_s
{
  u32 blk_ares : 1;
  u32 reserved0 : 31;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_usb_phy_cfg_ahb2phy_bcr_u
{
  struct ipa_gcc_hwio_def_gcc_usb_phy_cfg_ahb2phy_bcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_USB_PHY_CFG_AHB2PHY_CBCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_usb_phy_cfg_ahb2phy_cbcr_s
{
  u32 clk_enable : 1;
  u32 hw_ctl : 1;
  u32 clk_ares : 1;
  u32 sw_only_en : 1;
  u32 reserved0 : 16;
  u32 ignore_rpmh_clk_dis : 1;
  u32 reserved1 : 1;
  u32 clk_dis : 1;
  u32 ignore_all_clk_dis : 1;
  u32 ignore_all_ares : 1;
  u32 reserved2 : 6;
  u32 clk_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_usb_phy_cfg_ahb2phy_cbcr_u
{
  struct ipa_gcc_hwio_def_gcc_usb_phy_cfg_ahb2phy_cbcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_SDCC1_BCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_sdcc1_bcr_s
{
  u32 blk_ares : 1;
  u32 reserved0 : 31;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_sdcc1_bcr_u
{
  struct ipa_gcc_hwio_def_gcc_sdcc1_bcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_SDCC1_APPS_CBCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_sdcc1_apps_cbcr_s
{
  u32 clk_enable : 1;
  u32 reserved0 : 1;
  u32 clk_ares : 1;
  u32 reserved1 : 1;
  u32 sleep : 4;
  u32 wakeup : 4;
  u32 force_mem_periph_off : 1;
  u32 force_mem_periph_on : 1;
  u32 force_mem_core_on : 1;
  u32 reserved2 : 7;
  u32 clk_dis : 1;
  u32 ignore_all_clk_dis : 1;
  u32 ignore_all_ares : 1;
  u32 reserved3 : 6;
  u32 clk_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_sdcc1_apps_cbcr_u
{
  struct ipa_gcc_hwio_def_gcc_sdcc1_apps_cbcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_SDCC1_APPS_SREGR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_sdcc1_apps_sregr_s
{
  u32 reserved0 : 1;
  u32 sw_clk_en_slp_stg : 1;
  u32 sw_clk_en_sel_slp_stg : 1;
  u32 sw_ctrl_pwr_down : 1;
  u32 sw_rst_slp_stg : 1;
  u32 sw_rst_sel_slp_stg : 1;
  u32 force_clk_on : 1;
  u32 mem_cph_enable : 1;
  u32 sw_div_ratio_slp_stg_clk : 2;
  u32 mem_periph_on_ack : 1;
  u32 mem_core_on_ack : 1;
  u32 sw_sm_pscbc_seq_in_override : 1;
  u32 mem_cph_rst_sw_override : 1;
  u32 pscbc_slp_stg_mode_csr : 1;
  u32 ignore_gdsc_pwr_dwn_csr : 1;
  u32 sreg_pscbc_spare_ctrl_in : 8;
  u32 sreg_pscbc_spare_ctrl_out : 8;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_sdcc1_apps_sregr_u
{
  struct ipa_gcc_hwio_def_gcc_sdcc1_apps_sregr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_SDCC1_AHB_CBCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_sdcc1_ahb_cbcr_s
{
  u32 clk_enable : 1;
  u32 reserved0 : 1;
  u32 clk_ares : 1;
  u32 reserved1 : 17;
  u32 ignore_rpmh_clk_dis : 1;
  u32 reserved2 : 1;
  u32 clk_dis : 1;
  u32 ignore_all_clk_dis : 1;
  u32 ignore_all_ares : 1;
  u32 reserved3 : 6;
  u32 clk_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_sdcc1_ahb_cbcr_u
{
  struct ipa_gcc_hwio_def_gcc_sdcc1_ahb_cbcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_SDCC1_APPS_CMD_RCGR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_sdcc1_apps_cmd_rcgr_s
{
  u32 update : 1;
  u32 root_en : 1;
  u32 reserved0 : 2;
  u32 dirty_cfg_rcgr : 1;
  u32 dirty_m : 1;
  u32 dirty_n : 1;
  u32 dirty_d : 1;
  u32 reserved1 : 23;
  u32 root_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_sdcc1_apps_cmd_rcgr_u
{
  struct ipa_gcc_hwio_def_gcc_sdcc1_apps_cmd_rcgr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_SDCC1_APPS_CFG_RCGR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_sdcc1_apps_cfg_rcgr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 1;
  u32 mode : 2;
  u32 reserved2 : 6;
  u32 hw_clk_control : 1;
  u32 reserved3 : 11;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_sdcc1_apps_cfg_rcgr_u
{
  struct ipa_gcc_hwio_def_gcc_sdcc1_apps_cfg_rcgr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_SDCC1_APPS_M
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_sdcc1_apps_m_s
{
  u32 m : 8;
  u32 reserved0 : 24;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_sdcc1_apps_m_u
{
  struct ipa_gcc_hwio_def_gcc_sdcc1_apps_m_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_SDCC1_APPS_N
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_sdcc1_apps_n_s
{
  u32 not_n_minus_m : 8;
  u32 reserved0 : 24;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_sdcc1_apps_n_u
{
  struct ipa_gcc_hwio_def_gcc_sdcc1_apps_n_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_SDCC1_APPS_D
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_sdcc1_apps_d_s
{
  u32 not_2d : 8;
  u32 reserved0 : 24;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_sdcc1_apps_d_u
{
  struct ipa_gcc_hwio_def_gcc_sdcc1_apps_d_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_BLSP1_BCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_blsp1_bcr_s
{
  u32 blk_ares : 1;
  u32 reserved0 : 31;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_blsp1_bcr_u
{
  struct ipa_gcc_hwio_def_gcc_blsp1_bcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_BLSP1_AHB_CBCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_blsp1_ahb_cbcr_s
{
  u32 reserved0 : 2;
  u32 clk_ares : 1;
  u32 reserved1 : 1;
  u32 sleep : 4;
  u32 wakeup : 4;
  u32 force_mem_periph_off : 1;
  u32 force_mem_periph_on : 1;
  u32 force_mem_core_on : 1;
  u32 reserved2 : 5;
  u32 ignore_rpmh_clk_dis : 1;
  u32 reserved3 : 1;
  u32 clk_dis : 1;
  u32 ignore_all_clk_dis : 1;
  u32 ignore_all_ares : 1;
  u32 reserved4 : 6;
  u32 clk_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_blsp1_ahb_cbcr_u
{
  struct ipa_gcc_hwio_def_gcc_blsp1_ahb_cbcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_BLSP1_AHB_SREGR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_blsp1_ahb_sregr_s
{
  u32 reserved0 : 1;
  u32 sw_clk_en_slp_stg : 1;
  u32 sw_clk_en_sel_slp_stg : 1;
  u32 sw_ctrl_pwr_down : 1;
  u32 sw_rst_slp_stg : 1;
  u32 sw_rst_sel_slp_stg : 1;
  u32 force_clk_on : 1;
  u32 mem_cph_enable : 1;
  u32 sw_div_ratio_slp_stg_clk : 2;
  u32 mem_periph_on_ack : 1;
  u32 mem_core_on_ack : 1;
  u32 sw_sm_pscbc_seq_in_override : 1;
  u32 mem_cph_rst_sw_override : 1;
  u32 pscbc_slp_stg_mode_csr : 1;
  u32 ignore_gdsc_pwr_dwn_csr : 1;
  u32 sreg_pscbc_spare_ctrl_in : 8;
  u32 sreg_pscbc_spare_ctrl_out : 8;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_blsp1_ahb_sregr_u
{
  struct ipa_gcc_hwio_def_gcc_blsp1_ahb_sregr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_BLSP1_SLEEP_CBCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_blsp1_sleep_cbcr_s
{
  u32 reserved0 : 2;
  u32 clk_ares : 1;
  u32 reserved1 : 19;
  u32 clk_dis : 1;
  u32 ignore_all_clk_dis : 1;
  u32 ignore_all_ares : 1;
  u32 reserved2 : 6;
  u32 clk_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_blsp1_sleep_cbcr_u
{
  struct ipa_gcc_hwio_def_gcc_blsp1_sleep_cbcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_BLSP_UART_SIM_CMD_RCGR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_blsp_uart_sim_cmd_rcgr_s
{
  u32 update : 1;
  u32 root_en : 1;
  u32 reserved0 : 2;
  u32 dirty_cfg_rcgr : 1;
  u32 dirty_m : 1;
  u32 dirty_n : 1;
  u32 dirty_d : 1;
  u32 reserved1 : 23;
  u32 root_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_blsp_uart_sim_cmd_rcgr_u
{
  struct ipa_gcc_hwio_def_gcc_blsp_uart_sim_cmd_rcgr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_BLSP_UART_SIM_CFG_RCGR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_blsp_uart_sim_cfg_rcgr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 1;
  u32 mode : 2;
  u32 reserved2 : 6;
  u32 hw_clk_control : 1;
  u32 reserved3 : 11;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_blsp_uart_sim_cfg_rcgr_u
{
  struct ipa_gcc_hwio_def_gcc_blsp_uart_sim_cfg_rcgr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_BLSP_UART_SIM_M
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_blsp_uart_sim_m_s
{
  u32 m : 8;
  u32 reserved0 : 24;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_blsp_uart_sim_m_u
{
  struct ipa_gcc_hwio_def_gcc_blsp_uart_sim_m_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_BLSP_UART_SIM_N
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_blsp_uart_sim_n_s
{
  u32 not_n_minus_m : 8;
  u32 reserved0 : 24;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_blsp_uart_sim_n_u
{
  struct ipa_gcc_hwio_def_gcc_blsp_uart_sim_n_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_BLSP_UART_SIM_D
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_blsp_uart_sim_d_s
{
  u32 not_2d : 8;
  u32 reserved0 : 24;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_blsp_uart_sim_d_u
{
  struct ipa_gcc_hwio_def_gcc_blsp_uart_sim_d_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_BLSP1_QUP1_BCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_blsp1_qup1_bcr_s
{
  u32 blk_ares : 1;
  u32 reserved0 : 31;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_blsp1_qup1_bcr_u
{
  struct ipa_gcc_hwio_def_gcc_blsp1_qup1_bcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_BLSP1_QUP1_SPI_APPS_CBCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_blsp1_qup1_spi_apps_cbcr_s
{
  u32 clk_enable : 1;
  u32 reserved0 : 1;
  u32 clk_ares : 1;
  u32 reserved1 : 19;
  u32 clk_dis : 1;
  u32 ignore_all_clk_dis : 1;
  u32 ignore_all_ares : 1;
  u32 reserved2 : 6;
  u32 clk_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_blsp1_qup1_spi_apps_cbcr_u
{
  struct ipa_gcc_hwio_def_gcc_blsp1_qup1_spi_apps_cbcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_BLSP1_QUP1_I2C_APPS_CBCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_blsp1_qup1_i2c_apps_cbcr_s
{
  u32 clk_enable : 1;
  u32 reserved0 : 1;
  u32 clk_ares : 1;
  u32 reserved1 : 19;
  u32 clk_dis : 1;
  u32 ignore_all_clk_dis : 1;
  u32 ignore_all_ares : 1;
  u32 reserved2 : 6;
  u32 clk_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_blsp1_qup1_i2c_apps_cbcr_u
{
  struct ipa_gcc_hwio_def_gcc_blsp1_qup1_i2c_apps_cbcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_BLSP1_QUP1_SPI_APPS_CMD_RCGR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_blsp1_qup1_spi_apps_cmd_rcgr_s
{
  u32 update : 1;
  u32 root_en : 1;
  u32 reserved0 : 2;
  u32 dirty_cfg_rcgr : 1;
  u32 dirty_m : 1;
  u32 dirty_n : 1;
  u32 dirty_d : 1;
  u32 reserved1 : 23;
  u32 root_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_blsp1_qup1_spi_apps_cmd_rcgr_u
{
  struct ipa_gcc_hwio_def_gcc_blsp1_qup1_spi_apps_cmd_rcgr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_BLSP1_QUP1_SPI_APPS_CFG_RCGR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_blsp1_qup1_spi_apps_cfg_rcgr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 1;
  u32 mode : 2;
  u32 reserved2 : 6;
  u32 hw_clk_control : 1;
  u32 reserved3 : 11;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_blsp1_qup1_spi_apps_cfg_rcgr_u
{
  struct ipa_gcc_hwio_def_gcc_blsp1_qup1_spi_apps_cfg_rcgr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_BLSP1_QUP1_SPI_APPS_M
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_blsp1_qup1_spi_apps_m_s
{
  u32 m : 8;
  u32 reserved0 : 24;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_blsp1_qup1_spi_apps_m_u
{
  struct ipa_gcc_hwio_def_gcc_blsp1_qup1_spi_apps_m_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_BLSP1_QUP1_SPI_APPS_N
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_blsp1_qup1_spi_apps_n_s
{
  u32 not_n_minus_m : 8;
  u32 reserved0 : 24;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_blsp1_qup1_spi_apps_n_u
{
  struct ipa_gcc_hwio_def_gcc_blsp1_qup1_spi_apps_n_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_BLSP1_QUP1_SPI_APPS_D
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_blsp1_qup1_spi_apps_d_s
{
  u32 not_2d : 8;
  u32 reserved0 : 24;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_blsp1_qup1_spi_apps_d_u
{
  struct ipa_gcc_hwio_def_gcc_blsp1_qup1_spi_apps_d_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_BLSP1_QUP1_I2C_APPS_CMD_RCGR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_blsp1_qup1_i2c_apps_cmd_rcgr_s
{
  u32 update : 1;
  u32 root_en : 1;
  u32 reserved0 : 2;
  u32 dirty_cfg_rcgr : 1;
  u32 dirty_m : 1;
  u32 dirty_n : 1;
  u32 dirty_d : 1;
  u32 reserved1 : 23;
  u32 root_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_blsp1_qup1_i2c_apps_cmd_rcgr_u
{
  struct ipa_gcc_hwio_def_gcc_blsp1_qup1_i2c_apps_cmd_rcgr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_BLSP1_QUP1_I2C_APPS_CFG_RCGR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_blsp1_qup1_i2c_apps_cfg_rcgr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 1;
  u32 mode : 2;
  u32 reserved2 : 6;
  u32 hw_clk_control : 1;
  u32 reserved3 : 11;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_blsp1_qup1_i2c_apps_cfg_rcgr_u
{
  struct ipa_gcc_hwio_def_gcc_blsp1_qup1_i2c_apps_cfg_rcgr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_BLSP1_QUP1_I2C_APPS_M
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_blsp1_qup1_i2c_apps_m_s
{
  u32 m : 8;
  u32 reserved0 : 24;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_blsp1_qup1_i2c_apps_m_u
{
  struct ipa_gcc_hwio_def_gcc_blsp1_qup1_i2c_apps_m_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_BLSP1_QUP1_I2C_APPS_N
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_blsp1_qup1_i2c_apps_n_s
{
  u32 not_n_minus_m : 8;
  u32 reserved0 : 24;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_blsp1_qup1_i2c_apps_n_u
{
  struct ipa_gcc_hwio_def_gcc_blsp1_qup1_i2c_apps_n_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_BLSP1_QUP1_I2C_APPS_D
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_blsp1_qup1_i2c_apps_d_s
{
  u32 not_2d : 8;
  u32 reserved0 : 24;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_blsp1_qup1_i2c_apps_d_u
{
  struct ipa_gcc_hwio_def_gcc_blsp1_qup1_i2c_apps_d_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_BLSP1_UART1_BCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_blsp1_uart1_bcr_s
{
  u32 blk_ares : 1;
  u32 reserved0 : 31;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_blsp1_uart1_bcr_u
{
  struct ipa_gcc_hwio_def_gcc_blsp1_uart1_bcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_BLSP1_UART1_APPS_CBCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_blsp1_uart1_apps_cbcr_s
{
  u32 clk_enable : 1;
  u32 reserved0 : 1;
  u32 clk_ares : 1;
  u32 reserved1 : 19;
  u32 clk_dis : 1;
  u32 ignore_all_clk_dis : 1;
  u32 ignore_all_ares : 1;
  u32 reserved2 : 6;
  u32 clk_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_blsp1_uart1_apps_cbcr_u
{
  struct ipa_gcc_hwio_def_gcc_blsp1_uart1_apps_cbcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_BLSP1_UART1_SIM_CBCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_blsp1_uart1_sim_cbcr_s
{
  u32 clk_enable : 1;
  u32 reserved0 : 1;
  u32 clk_ares : 1;
  u32 reserved1 : 19;
  u32 clk_dis : 1;
  u32 ignore_all_clk_dis : 1;
  u32 ignore_all_ares : 1;
  u32 reserved2 : 6;
  u32 clk_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_blsp1_uart1_sim_cbcr_u
{
  struct ipa_gcc_hwio_def_gcc_blsp1_uart1_sim_cbcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_BLSP1_UART1_APPS_CMD_RCGR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_blsp1_uart1_apps_cmd_rcgr_s
{
  u32 update : 1;
  u32 root_en : 1;
  u32 reserved0 : 2;
  u32 dirty_cfg_rcgr : 1;
  u32 dirty_m : 1;
  u32 dirty_n : 1;
  u32 dirty_d : 1;
  u32 reserved1 : 23;
  u32 root_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_blsp1_uart1_apps_cmd_rcgr_u
{
  struct ipa_gcc_hwio_def_gcc_blsp1_uart1_apps_cmd_rcgr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_BLSP1_UART1_APPS_CFG_RCGR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_blsp1_uart1_apps_cfg_rcgr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 1;
  u32 mode : 2;
  u32 reserved2 : 6;
  u32 hw_clk_control : 1;
  u32 reserved3 : 11;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_blsp1_uart1_apps_cfg_rcgr_u
{
  struct ipa_gcc_hwio_def_gcc_blsp1_uart1_apps_cfg_rcgr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_BLSP1_UART1_APPS_M
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_blsp1_uart1_apps_m_s
{
  u32 m : 16;
  u32 reserved0 : 16;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_blsp1_uart1_apps_m_u
{
  struct ipa_gcc_hwio_def_gcc_blsp1_uart1_apps_m_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_BLSP1_UART1_APPS_N
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_blsp1_uart1_apps_n_s
{
  u32 not_n_minus_m : 16;
  u32 reserved0 : 16;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_blsp1_uart1_apps_n_u
{
  struct ipa_gcc_hwio_def_gcc_blsp1_uart1_apps_n_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_BLSP1_UART1_APPS_D
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_blsp1_uart1_apps_d_s
{
  u32 not_2d : 16;
  u32 reserved0 : 16;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_blsp1_uart1_apps_d_u
{
  struct ipa_gcc_hwio_def_gcc_blsp1_uart1_apps_d_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_BLSP1_QUP2_BCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_blsp1_qup2_bcr_s
{
  u32 blk_ares : 1;
  u32 reserved0 : 31;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_blsp1_qup2_bcr_u
{
  struct ipa_gcc_hwio_def_gcc_blsp1_qup2_bcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_BLSP1_QUP2_SPI_APPS_CBCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_blsp1_qup2_spi_apps_cbcr_s
{
  u32 clk_enable : 1;
  u32 reserved0 : 1;
  u32 clk_ares : 1;
  u32 reserved1 : 19;
  u32 clk_dis : 1;
  u32 ignore_all_clk_dis : 1;
  u32 ignore_all_ares : 1;
  u32 reserved2 : 6;
  u32 clk_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_blsp1_qup2_spi_apps_cbcr_u
{
  struct ipa_gcc_hwio_def_gcc_blsp1_qup2_spi_apps_cbcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_BLSP1_QUP2_I2C_APPS_CBCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_blsp1_qup2_i2c_apps_cbcr_s
{
  u32 clk_enable : 1;
  u32 reserved0 : 1;
  u32 clk_ares : 1;
  u32 reserved1 : 19;
  u32 clk_dis : 1;
  u32 ignore_all_clk_dis : 1;
  u32 ignore_all_ares : 1;
  u32 reserved2 : 6;
  u32 clk_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_blsp1_qup2_i2c_apps_cbcr_u
{
  struct ipa_gcc_hwio_def_gcc_blsp1_qup2_i2c_apps_cbcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_BLSP1_QUP2_SPI_APPS_CMD_RCGR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_blsp1_qup2_spi_apps_cmd_rcgr_s
{
  u32 update : 1;
  u32 root_en : 1;
  u32 reserved0 : 2;
  u32 dirty_cfg_rcgr : 1;
  u32 dirty_m : 1;
  u32 dirty_n : 1;
  u32 dirty_d : 1;
  u32 reserved1 : 23;
  u32 root_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_blsp1_qup2_spi_apps_cmd_rcgr_u
{
  struct ipa_gcc_hwio_def_gcc_blsp1_qup2_spi_apps_cmd_rcgr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_BLSP1_QUP2_SPI_APPS_CFG_RCGR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_blsp1_qup2_spi_apps_cfg_rcgr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 1;
  u32 mode : 2;
  u32 reserved2 : 6;
  u32 hw_clk_control : 1;
  u32 reserved3 : 11;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_blsp1_qup2_spi_apps_cfg_rcgr_u
{
  struct ipa_gcc_hwio_def_gcc_blsp1_qup2_spi_apps_cfg_rcgr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_BLSP1_QUP2_SPI_APPS_M
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_blsp1_qup2_spi_apps_m_s
{
  u32 m : 8;
  u32 reserved0 : 24;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_blsp1_qup2_spi_apps_m_u
{
  struct ipa_gcc_hwio_def_gcc_blsp1_qup2_spi_apps_m_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_BLSP1_QUP2_SPI_APPS_N
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_blsp1_qup2_spi_apps_n_s
{
  u32 not_n_minus_m : 8;
  u32 reserved0 : 24;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_blsp1_qup2_spi_apps_n_u
{
  struct ipa_gcc_hwio_def_gcc_blsp1_qup2_spi_apps_n_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_BLSP1_QUP2_SPI_APPS_D
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_blsp1_qup2_spi_apps_d_s
{
  u32 not_2d : 8;
  u32 reserved0 : 24;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_blsp1_qup2_spi_apps_d_u
{
  struct ipa_gcc_hwio_def_gcc_blsp1_qup2_spi_apps_d_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_BLSP1_QUP2_I2C_APPS_CMD_RCGR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_blsp1_qup2_i2c_apps_cmd_rcgr_s
{
  u32 update : 1;
  u32 root_en : 1;
  u32 reserved0 : 2;
  u32 dirty_cfg_rcgr : 1;
  u32 dirty_m : 1;
  u32 dirty_n : 1;
  u32 dirty_d : 1;
  u32 reserved1 : 23;
  u32 root_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_blsp1_qup2_i2c_apps_cmd_rcgr_u
{
  struct ipa_gcc_hwio_def_gcc_blsp1_qup2_i2c_apps_cmd_rcgr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_BLSP1_QUP2_I2C_APPS_CFG_RCGR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_blsp1_qup2_i2c_apps_cfg_rcgr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 1;
  u32 mode : 2;
  u32 reserved2 : 6;
  u32 hw_clk_control : 1;
  u32 reserved3 : 11;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_blsp1_qup2_i2c_apps_cfg_rcgr_u
{
  struct ipa_gcc_hwio_def_gcc_blsp1_qup2_i2c_apps_cfg_rcgr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_BLSP1_QUP2_I2C_APPS_M
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_blsp1_qup2_i2c_apps_m_s
{
  u32 m : 8;
  u32 reserved0 : 24;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_blsp1_qup2_i2c_apps_m_u
{
  struct ipa_gcc_hwio_def_gcc_blsp1_qup2_i2c_apps_m_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_BLSP1_QUP2_I2C_APPS_N
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_blsp1_qup2_i2c_apps_n_s
{
  u32 not_n_minus_m : 8;
  u32 reserved0 : 24;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_blsp1_qup2_i2c_apps_n_u
{
  struct ipa_gcc_hwio_def_gcc_blsp1_qup2_i2c_apps_n_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_BLSP1_QUP2_I2C_APPS_D
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_blsp1_qup2_i2c_apps_d_s
{
  u32 not_2d : 8;
  u32 reserved0 : 24;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_blsp1_qup2_i2c_apps_d_u
{
  struct ipa_gcc_hwio_def_gcc_blsp1_qup2_i2c_apps_d_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_BLSP1_UART2_BCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_blsp1_uart2_bcr_s
{
  u32 blk_ares : 1;
  u32 reserved0 : 31;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_blsp1_uart2_bcr_u
{
  struct ipa_gcc_hwio_def_gcc_blsp1_uart2_bcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_BLSP1_UART2_APPS_CBCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_blsp1_uart2_apps_cbcr_s
{
  u32 clk_enable : 1;
  u32 reserved0 : 1;
  u32 clk_ares : 1;
  u32 reserved1 : 19;
  u32 clk_dis : 1;
  u32 ignore_all_clk_dis : 1;
  u32 ignore_all_ares : 1;
  u32 reserved2 : 6;
  u32 clk_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_blsp1_uart2_apps_cbcr_u
{
  struct ipa_gcc_hwio_def_gcc_blsp1_uart2_apps_cbcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_BLSP1_UART2_SIM_CBCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_blsp1_uart2_sim_cbcr_s
{
  u32 clk_enable : 1;
  u32 reserved0 : 1;
  u32 clk_ares : 1;
  u32 reserved1 : 19;
  u32 clk_dis : 1;
  u32 ignore_all_clk_dis : 1;
  u32 ignore_all_ares : 1;
  u32 reserved2 : 6;
  u32 clk_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_blsp1_uart2_sim_cbcr_u
{
  struct ipa_gcc_hwio_def_gcc_blsp1_uart2_sim_cbcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_BLSP1_UART2_APPS_CMD_RCGR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_blsp1_uart2_apps_cmd_rcgr_s
{
  u32 update : 1;
  u32 root_en : 1;
  u32 reserved0 : 2;
  u32 dirty_cfg_rcgr : 1;
  u32 dirty_m : 1;
  u32 dirty_n : 1;
  u32 dirty_d : 1;
  u32 reserved1 : 23;
  u32 root_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_blsp1_uart2_apps_cmd_rcgr_u
{
  struct ipa_gcc_hwio_def_gcc_blsp1_uart2_apps_cmd_rcgr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_BLSP1_UART2_APPS_CFG_RCGR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_blsp1_uart2_apps_cfg_rcgr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 1;
  u32 mode : 2;
  u32 reserved2 : 6;
  u32 hw_clk_control : 1;
  u32 reserved3 : 11;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_blsp1_uart2_apps_cfg_rcgr_u
{
  struct ipa_gcc_hwio_def_gcc_blsp1_uart2_apps_cfg_rcgr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_BLSP1_UART2_APPS_M
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_blsp1_uart2_apps_m_s
{
  u32 m : 16;
  u32 reserved0 : 16;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_blsp1_uart2_apps_m_u
{
  struct ipa_gcc_hwio_def_gcc_blsp1_uart2_apps_m_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_BLSP1_UART2_APPS_N
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_blsp1_uart2_apps_n_s
{
  u32 not_n_minus_m : 16;
  u32 reserved0 : 16;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_blsp1_uart2_apps_n_u
{
  struct ipa_gcc_hwio_def_gcc_blsp1_uart2_apps_n_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_BLSP1_UART2_APPS_D
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_blsp1_uart2_apps_d_s
{
  u32 not_2d : 16;
  u32 reserved0 : 16;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_blsp1_uart2_apps_d_u
{
  struct ipa_gcc_hwio_def_gcc_blsp1_uart2_apps_d_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_BLSP1_QUP3_BCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_blsp1_qup3_bcr_s
{
  u32 blk_ares : 1;
  u32 reserved0 : 31;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_blsp1_qup3_bcr_u
{
  struct ipa_gcc_hwio_def_gcc_blsp1_qup3_bcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_BLSP1_QUP3_SPI_APPS_CBCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_blsp1_qup3_spi_apps_cbcr_s
{
  u32 clk_enable : 1;
  u32 reserved0 : 1;
  u32 clk_ares : 1;
  u32 reserved1 : 19;
  u32 clk_dis : 1;
  u32 ignore_all_clk_dis : 1;
  u32 ignore_all_ares : 1;
  u32 reserved2 : 6;
  u32 clk_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_blsp1_qup3_spi_apps_cbcr_u
{
  struct ipa_gcc_hwio_def_gcc_blsp1_qup3_spi_apps_cbcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_BLSP1_QUP3_I2C_APPS_CBCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_blsp1_qup3_i2c_apps_cbcr_s
{
  u32 clk_enable : 1;
  u32 reserved0 : 1;
  u32 clk_ares : 1;
  u32 reserved1 : 19;
  u32 clk_dis : 1;
  u32 ignore_all_clk_dis : 1;
  u32 ignore_all_ares : 1;
  u32 reserved2 : 6;
  u32 clk_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_blsp1_qup3_i2c_apps_cbcr_u
{
  struct ipa_gcc_hwio_def_gcc_blsp1_qup3_i2c_apps_cbcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_BLSP1_QUP3_SPI_APPS_CMD_RCGR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_blsp1_qup3_spi_apps_cmd_rcgr_s
{
  u32 update : 1;
  u32 root_en : 1;
  u32 reserved0 : 2;
  u32 dirty_cfg_rcgr : 1;
  u32 dirty_m : 1;
  u32 dirty_n : 1;
  u32 dirty_d : 1;
  u32 reserved1 : 23;
  u32 root_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_blsp1_qup3_spi_apps_cmd_rcgr_u
{
  struct ipa_gcc_hwio_def_gcc_blsp1_qup3_spi_apps_cmd_rcgr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_BLSP1_QUP3_SPI_APPS_CFG_RCGR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_blsp1_qup3_spi_apps_cfg_rcgr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 1;
  u32 mode : 2;
  u32 reserved2 : 6;
  u32 hw_clk_control : 1;
  u32 reserved3 : 11;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_blsp1_qup3_spi_apps_cfg_rcgr_u
{
  struct ipa_gcc_hwio_def_gcc_blsp1_qup3_spi_apps_cfg_rcgr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_BLSP1_QUP3_SPI_APPS_M
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_blsp1_qup3_spi_apps_m_s
{
  u32 m : 8;
  u32 reserved0 : 24;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_blsp1_qup3_spi_apps_m_u
{
  struct ipa_gcc_hwio_def_gcc_blsp1_qup3_spi_apps_m_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_BLSP1_QUP3_SPI_APPS_N
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_blsp1_qup3_spi_apps_n_s
{
  u32 not_n_minus_m : 8;
  u32 reserved0 : 24;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_blsp1_qup3_spi_apps_n_u
{
  struct ipa_gcc_hwio_def_gcc_blsp1_qup3_spi_apps_n_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_BLSP1_QUP3_SPI_APPS_D
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_blsp1_qup3_spi_apps_d_s
{
  u32 not_2d : 8;
  u32 reserved0 : 24;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_blsp1_qup3_spi_apps_d_u
{
  struct ipa_gcc_hwio_def_gcc_blsp1_qup3_spi_apps_d_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_BLSP1_QUP3_I2C_APPS_CMD_RCGR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_blsp1_qup3_i2c_apps_cmd_rcgr_s
{
  u32 update : 1;
  u32 root_en : 1;
  u32 reserved0 : 2;
  u32 dirty_cfg_rcgr : 1;
  u32 dirty_m : 1;
  u32 dirty_n : 1;
  u32 dirty_d : 1;
  u32 reserved1 : 23;
  u32 root_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_blsp1_qup3_i2c_apps_cmd_rcgr_u
{
  struct ipa_gcc_hwio_def_gcc_blsp1_qup3_i2c_apps_cmd_rcgr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_BLSP1_QUP3_I2C_APPS_CFG_RCGR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_blsp1_qup3_i2c_apps_cfg_rcgr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 1;
  u32 mode : 2;
  u32 reserved2 : 6;
  u32 hw_clk_control : 1;
  u32 reserved3 : 11;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_blsp1_qup3_i2c_apps_cfg_rcgr_u
{
  struct ipa_gcc_hwio_def_gcc_blsp1_qup3_i2c_apps_cfg_rcgr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_BLSP1_QUP3_I2C_APPS_M
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_blsp1_qup3_i2c_apps_m_s
{
  u32 m : 8;
  u32 reserved0 : 24;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_blsp1_qup3_i2c_apps_m_u
{
  struct ipa_gcc_hwio_def_gcc_blsp1_qup3_i2c_apps_m_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_BLSP1_QUP3_I2C_APPS_N
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_blsp1_qup3_i2c_apps_n_s
{
  u32 not_n_minus_m : 8;
  u32 reserved0 : 24;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_blsp1_qup3_i2c_apps_n_u
{
  struct ipa_gcc_hwio_def_gcc_blsp1_qup3_i2c_apps_n_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_BLSP1_QUP3_I2C_APPS_D
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_blsp1_qup3_i2c_apps_d_s
{
  u32 not_2d : 8;
  u32 reserved0 : 24;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_blsp1_qup3_i2c_apps_d_u
{
  struct ipa_gcc_hwio_def_gcc_blsp1_qup3_i2c_apps_d_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_BLSP1_UART3_BCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_blsp1_uart3_bcr_s
{
  u32 blk_ares : 1;
  u32 reserved0 : 31;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_blsp1_uart3_bcr_u
{
  struct ipa_gcc_hwio_def_gcc_blsp1_uart3_bcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_BLSP1_UART3_APPS_CBCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_blsp1_uart3_apps_cbcr_s
{
  u32 clk_enable : 1;
  u32 reserved0 : 1;
  u32 clk_ares : 1;
  u32 reserved1 : 19;
  u32 clk_dis : 1;
  u32 ignore_all_clk_dis : 1;
  u32 ignore_all_ares : 1;
  u32 reserved2 : 6;
  u32 clk_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_blsp1_uart3_apps_cbcr_u
{
  struct ipa_gcc_hwio_def_gcc_blsp1_uart3_apps_cbcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_BLSP1_UART3_SIM_CBCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_blsp1_uart3_sim_cbcr_s
{
  u32 clk_enable : 1;
  u32 reserved0 : 1;
  u32 clk_ares : 1;
  u32 reserved1 : 19;
  u32 clk_dis : 1;
  u32 ignore_all_clk_dis : 1;
  u32 ignore_all_ares : 1;
  u32 reserved2 : 6;
  u32 clk_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_blsp1_uart3_sim_cbcr_u
{
  struct ipa_gcc_hwio_def_gcc_blsp1_uart3_sim_cbcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_BLSP1_UART3_APPS_CMD_RCGR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_blsp1_uart3_apps_cmd_rcgr_s
{
  u32 update : 1;
  u32 root_en : 1;
  u32 reserved0 : 2;
  u32 dirty_cfg_rcgr : 1;
  u32 dirty_m : 1;
  u32 dirty_n : 1;
  u32 dirty_d : 1;
  u32 reserved1 : 23;
  u32 root_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_blsp1_uart3_apps_cmd_rcgr_u
{
  struct ipa_gcc_hwio_def_gcc_blsp1_uart3_apps_cmd_rcgr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_BLSP1_UART3_APPS_CFG_RCGR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_blsp1_uart3_apps_cfg_rcgr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 1;
  u32 mode : 2;
  u32 reserved2 : 6;
  u32 hw_clk_control : 1;
  u32 reserved3 : 11;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_blsp1_uart3_apps_cfg_rcgr_u
{
  struct ipa_gcc_hwio_def_gcc_blsp1_uart3_apps_cfg_rcgr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_BLSP1_UART3_APPS_M
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_blsp1_uart3_apps_m_s
{
  u32 m : 16;
  u32 reserved0 : 16;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_blsp1_uart3_apps_m_u
{
  struct ipa_gcc_hwio_def_gcc_blsp1_uart3_apps_m_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_BLSP1_UART3_APPS_N
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_blsp1_uart3_apps_n_s
{
  u32 not_n_minus_m : 16;
  u32 reserved0 : 16;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_blsp1_uart3_apps_n_u
{
  struct ipa_gcc_hwio_def_gcc_blsp1_uart3_apps_n_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_BLSP1_UART3_APPS_D
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_blsp1_uart3_apps_d_s
{
  u32 not_2d : 16;
  u32 reserved0 : 16;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_blsp1_uart3_apps_d_u
{
  struct ipa_gcc_hwio_def_gcc_blsp1_uart3_apps_d_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_BLSP1_QUP4_BCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_blsp1_qup4_bcr_s
{
  u32 blk_ares : 1;
  u32 reserved0 : 31;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_blsp1_qup4_bcr_u
{
  struct ipa_gcc_hwio_def_gcc_blsp1_qup4_bcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_BLSP1_QUP4_SPI_APPS_CBCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_blsp1_qup4_spi_apps_cbcr_s
{
  u32 clk_enable : 1;
  u32 reserved0 : 1;
  u32 clk_ares : 1;
  u32 reserved1 : 19;
  u32 clk_dis : 1;
  u32 ignore_all_clk_dis : 1;
  u32 ignore_all_ares : 1;
  u32 reserved2 : 6;
  u32 clk_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_blsp1_qup4_spi_apps_cbcr_u
{
  struct ipa_gcc_hwio_def_gcc_blsp1_qup4_spi_apps_cbcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_BLSP1_QUP4_I2C_APPS_CBCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_blsp1_qup4_i2c_apps_cbcr_s
{
  u32 clk_enable : 1;
  u32 reserved0 : 1;
  u32 clk_ares : 1;
  u32 reserved1 : 19;
  u32 clk_dis : 1;
  u32 ignore_all_clk_dis : 1;
  u32 ignore_all_ares : 1;
  u32 reserved2 : 6;
  u32 clk_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_blsp1_qup4_i2c_apps_cbcr_u
{
  struct ipa_gcc_hwio_def_gcc_blsp1_qup4_i2c_apps_cbcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_BLSP1_QUP4_SPI_APPS_CMD_RCGR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_blsp1_qup4_spi_apps_cmd_rcgr_s
{
  u32 update : 1;
  u32 root_en : 1;
  u32 reserved0 : 2;
  u32 dirty_cfg_rcgr : 1;
  u32 dirty_m : 1;
  u32 dirty_n : 1;
  u32 dirty_d : 1;
  u32 reserved1 : 23;
  u32 root_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_blsp1_qup4_spi_apps_cmd_rcgr_u
{
  struct ipa_gcc_hwio_def_gcc_blsp1_qup4_spi_apps_cmd_rcgr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_BLSP1_QUP4_SPI_APPS_CFG_RCGR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_blsp1_qup4_spi_apps_cfg_rcgr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 1;
  u32 mode : 2;
  u32 reserved2 : 6;
  u32 hw_clk_control : 1;
  u32 reserved3 : 11;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_blsp1_qup4_spi_apps_cfg_rcgr_u
{
  struct ipa_gcc_hwio_def_gcc_blsp1_qup4_spi_apps_cfg_rcgr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_BLSP1_QUP4_SPI_APPS_M
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_blsp1_qup4_spi_apps_m_s
{
  u32 m : 8;
  u32 reserved0 : 24;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_blsp1_qup4_spi_apps_m_u
{
  struct ipa_gcc_hwio_def_gcc_blsp1_qup4_spi_apps_m_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_BLSP1_QUP4_SPI_APPS_N
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_blsp1_qup4_spi_apps_n_s
{
  u32 not_n_minus_m : 8;
  u32 reserved0 : 24;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_blsp1_qup4_spi_apps_n_u
{
  struct ipa_gcc_hwio_def_gcc_blsp1_qup4_spi_apps_n_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_BLSP1_QUP4_SPI_APPS_D
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_blsp1_qup4_spi_apps_d_s
{
  u32 not_2d : 8;
  u32 reserved0 : 24;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_blsp1_qup4_spi_apps_d_u
{
  struct ipa_gcc_hwio_def_gcc_blsp1_qup4_spi_apps_d_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_BLSP1_QUP4_I2C_APPS_CMD_RCGR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_blsp1_qup4_i2c_apps_cmd_rcgr_s
{
  u32 update : 1;
  u32 root_en : 1;
  u32 reserved0 : 2;
  u32 dirty_cfg_rcgr : 1;
  u32 dirty_m : 1;
  u32 dirty_n : 1;
  u32 dirty_d : 1;
  u32 reserved1 : 23;
  u32 root_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_blsp1_qup4_i2c_apps_cmd_rcgr_u
{
  struct ipa_gcc_hwio_def_gcc_blsp1_qup4_i2c_apps_cmd_rcgr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_BLSP1_QUP4_I2C_APPS_CFG_RCGR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_blsp1_qup4_i2c_apps_cfg_rcgr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 1;
  u32 mode : 2;
  u32 reserved2 : 6;
  u32 hw_clk_control : 1;
  u32 reserved3 : 11;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_blsp1_qup4_i2c_apps_cfg_rcgr_u
{
  struct ipa_gcc_hwio_def_gcc_blsp1_qup4_i2c_apps_cfg_rcgr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_BLSP1_QUP4_I2C_APPS_M
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_blsp1_qup4_i2c_apps_m_s
{
  u32 m : 8;
  u32 reserved0 : 24;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_blsp1_qup4_i2c_apps_m_u
{
  struct ipa_gcc_hwio_def_gcc_blsp1_qup4_i2c_apps_m_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_BLSP1_QUP4_I2C_APPS_N
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_blsp1_qup4_i2c_apps_n_s
{
  u32 not_n_minus_m : 8;
  u32 reserved0 : 24;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_blsp1_qup4_i2c_apps_n_u
{
  struct ipa_gcc_hwio_def_gcc_blsp1_qup4_i2c_apps_n_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_BLSP1_QUP4_I2C_APPS_D
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_blsp1_qup4_i2c_apps_d_s
{
  u32 not_2d : 8;
  u32 reserved0 : 24;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_blsp1_qup4_i2c_apps_d_u
{
  struct ipa_gcc_hwio_def_gcc_blsp1_qup4_i2c_apps_d_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_BLSP1_UART4_BCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_blsp1_uart4_bcr_s
{
  u32 blk_ares : 1;
  u32 reserved0 : 31;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_blsp1_uart4_bcr_u
{
  struct ipa_gcc_hwio_def_gcc_blsp1_uart4_bcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_BLSP1_UART4_APPS_CBCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_blsp1_uart4_apps_cbcr_s
{
  u32 clk_enable : 1;
  u32 reserved0 : 1;
  u32 clk_ares : 1;
  u32 reserved1 : 19;
  u32 clk_dis : 1;
  u32 ignore_all_clk_dis : 1;
  u32 ignore_all_ares : 1;
  u32 reserved2 : 6;
  u32 clk_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_blsp1_uart4_apps_cbcr_u
{
  struct ipa_gcc_hwio_def_gcc_blsp1_uart4_apps_cbcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_BLSP1_UART4_SIM_CBCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_blsp1_uart4_sim_cbcr_s
{
  u32 clk_enable : 1;
  u32 reserved0 : 1;
  u32 clk_ares : 1;
  u32 reserved1 : 19;
  u32 clk_dis : 1;
  u32 ignore_all_clk_dis : 1;
  u32 ignore_all_ares : 1;
  u32 reserved2 : 6;
  u32 clk_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_blsp1_uart4_sim_cbcr_u
{
  struct ipa_gcc_hwio_def_gcc_blsp1_uart4_sim_cbcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_BLSP1_UART4_APPS_CMD_RCGR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_blsp1_uart4_apps_cmd_rcgr_s
{
  u32 update : 1;
  u32 root_en : 1;
  u32 reserved0 : 2;
  u32 dirty_cfg_rcgr : 1;
  u32 dirty_m : 1;
  u32 dirty_n : 1;
  u32 dirty_d : 1;
  u32 reserved1 : 23;
  u32 root_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_blsp1_uart4_apps_cmd_rcgr_u
{
  struct ipa_gcc_hwio_def_gcc_blsp1_uart4_apps_cmd_rcgr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_BLSP1_UART4_APPS_CFG_RCGR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_blsp1_uart4_apps_cfg_rcgr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 1;
  u32 mode : 2;
  u32 reserved2 : 6;
  u32 hw_clk_control : 1;
  u32 reserved3 : 11;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_blsp1_uart4_apps_cfg_rcgr_u
{
  struct ipa_gcc_hwio_def_gcc_blsp1_uart4_apps_cfg_rcgr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_BLSP1_UART4_APPS_M
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_blsp1_uart4_apps_m_s
{
  u32 m : 16;
  u32 reserved0 : 16;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_blsp1_uart4_apps_m_u
{
  struct ipa_gcc_hwio_def_gcc_blsp1_uart4_apps_m_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_BLSP1_UART4_APPS_N
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_blsp1_uart4_apps_n_s
{
  u32 not_n_minus_m : 16;
  u32 reserved0 : 16;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_blsp1_uart4_apps_n_u
{
  struct ipa_gcc_hwio_def_gcc_blsp1_uart4_apps_n_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_BLSP1_UART4_APPS_D
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_blsp1_uart4_apps_d_s
{
  u32 not_2d : 16;
  u32 reserved0 : 16;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_blsp1_uart4_apps_d_u
{
  struct ipa_gcc_hwio_def_gcc_blsp1_uart4_apps_d_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_PDM_BCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_pdm_bcr_s
{
  u32 blk_ares : 1;
  u32 reserved0 : 31;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_pdm_bcr_u
{
  struct ipa_gcc_hwio_def_gcc_pdm_bcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_PDM_AHB_CBCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_pdm_ahb_cbcr_s
{
  u32 clk_enable : 1;
  u32 hw_ctl : 1;
  u32 clk_ares : 1;
  u32 sw_only_en : 1;
  u32 reserved0 : 16;
  u32 ignore_rpmh_clk_dis : 1;
  u32 reserved1 : 1;
  u32 clk_dis : 1;
  u32 ignore_all_clk_dis : 1;
  u32 ignore_all_ares : 1;
  u32 reserved2 : 6;
  u32 clk_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_pdm_ahb_cbcr_u
{
  struct ipa_gcc_hwio_def_gcc_pdm_ahb_cbcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_PDM_XO4_CBCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_pdm_xo4_cbcr_s
{
  u32 clk_enable : 1;
  u32 reserved0 : 1;
  u32 clk_ares : 1;
  u32 reserved1 : 19;
  u32 clk_dis : 1;
  u32 ignore_all_clk_dis : 1;
  u32 ignore_all_ares : 1;
  u32 reserved2 : 6;
  u32 clk_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_pdm_xo4_cbcr_u
{
  struct ipa_gcc_hwio_def_gcc_pdm_xo4_cbcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_PDM2_CBCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_pdm2_cbcr_s
{
  u32 clk_enable : 1;
  u32 reserved0 : 1;
  u32 clk_ares : 1;
  u32 reserved1 : 19;
  u32 clk_dis : 1;
  u32 ignore_all_clk_dis : 1;
  u32 ignore_all_ares : 1;
  u32 reserved2 : 6;
  u32 clk_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_pdm2_cbcr_u
{
  struct ipa_gcc_hwio_def_gcc_pdm2_cbcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_PDM2_CMD_RCGR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_pdm2_cmd_rcgr_s
{
  u32 update : 1;
  u32 root_en : 1;
  u32 reserved0 : 2;
  u32 dirty_cfg_rcgr : 1;
  u32 reserved1 : 26;
  u32 root_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_pdm2_cmd_rcgr_u
{
  struct ipa_gcc_hwio_def_gcc_pdm2_cmd_rcgr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_PDM2_CFG_RCGR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_pdm2_cfg_rcgr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 5;
  u32 rcglite_disable : 1;
  u32 reserved2 : 3;
  u32 hw_clk_control : 1;
  u32 reserved3 : 11;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_pdm2_cfg_rcgr_u
{
  struct ipa_gcc_hwio_def_gcc_pdm2_cfg_rcgr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_PDM_XO4_CDIVR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_pdm_xo4_cdivr_s
{
  u32 clk_div : 4;
  u32 reserved0 : 28;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_pdm_xo4_cdivr_u
{
  struct ipa_gcc_hwio_def_gcc_pdm_xo4_cdivr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_PRNG_BCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_prng_bcr_s
{
  u32 blk_ares : 1;
  u32 reserved0 : 31;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_prng_bcr_u
{
  struct ipa_gcc_hwio_def_gcc_prng_bcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_PRNG_AHB_CBCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_prng_ahb_cbcr_s
{
  u32 reserved0 : 2;
  u32 clk_ares : 1;
  u32 reserved1 : 17;
  u32 ignore_rpmh_clk_dis : 1;
  u32 reserved2 : 1;
  u32 clk_dis : 1;
  u32 ignore_all_clk_dis : 1;
  u32 ignore_all_ares : 1;
  u32 reserved3 : 6;
  u32 clk_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_prng_ahb_cbcr_u
{
  struct ipa_gcc_hwio_def_gcc_prng_ahb_cbcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_TCSR_BCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_tcsr_bcr_s
{
  u32 blk_ares : 1;
  u32 reserved0 : 31;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_tcsr_bcr_u
{
  struct ipa_gcc_hwio_def_gcc_tcsr_bcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_TCSR_AHB_CBCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_tcsr_ahb_cbcr_s
{
  u32 reserved0 : 1;
  u32 hw_ctl : 1;
  u32 clk_ares : 1;
  u32 sw_only_en : 1;
  u32 reserved1 : 16;
  u32 ignore_rpmh_clk_dis : 1;
  u32 reserved2 : 1;
  u32 clk_dis : 1;
  u32 ignore_all_clk_dis : 1;
  u32 ignore_all_ares : 1;
  u32 reserved3 : 6;
  u32 clk_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_tcsr_ahb_cbcr_u
{
  struct ipa_gcc_hwio_def_gcc_tcsr_ahb_cbcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_TCSR_ACC_SERIAL_CBCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_tcsr_acc_serial_cbcr_s
{
  u32 clk_enable : 1;
  u32 hw_ctl : 1;
  u32 clk_ares : 1;
  u32 sw_only_en : 1;
  u32 reserved0 : 18;
  u32 clk_dis : 1;
  u32 ignore_all_clk_dis : 1;
  u32 ignore_all_ares : 1;
  u32 reserved1 : 6;
  u32 clk_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_tcsr_acc_serial_cbcr_u
{
  struct ipa_gcc_hwio_def_gcc_tcsr_acc_serial_cbcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_BOOT_ROM_BCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_boot_rom_bcr_s
{
  u32 blk_ares : 1;
  u32 reserved0 : 31;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_boot_rom_bcr_u
{
  struct ipa_gcc_hwio_def_gcc_boot_rom_bcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_BOOT_ROM_AHB_CBCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_boot_rom_ahb_cbcr_s
{
  u32 reserved0 : 1;
  u32 hw_ctl : 1;
  u32 clk_ares : 1;
  u32 sw_only_en : 1;
  u32 sleep : 4;
  u32 wakeup : 4;
  u32 force_mem_periph_off : 1;
  u32 force_mem_periph_on : 1;
  u32 force_mem_core_on : 1;
  u32 reserved1 : 5;
  u32 ignore_rpmh_clk_dis : 1;
  u32 reserved2 : 1;
  u32 clk_dis : 1;
  u32 ignore_all_clk_dis : 1;
  u32 ignore_all_ares : 1;
  u32 reserved3 : 6;
  u32 clk_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_boot_rom_ahb_cbcr_u
{
  struct ipa_gcc_hwio_def_gcc_boot_rom_ahb_cbcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_BOOT_ROM_AHB_SREGR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_boot_rom_ahb_sregr_s
{
  u32 reserved0 : 1;
  u32 sw_clk_en_slp_stg : 1;
  u32 sw_clk_en_sel_slp_stg : 1;
  u32 sw_ctrl_pwr_down : 1;
  u32 sw_rst_slp_stg : 1;
  u32 sw_rst_sel_slp_stg : 1;
  u32 force_clk_on : 1;
  u32 mem_cph_enable : 1;
  u32 sw_div_ratio_slp_stg_clk : 2;
  u32 mem_periph_on_ack : 1;
  u32 mem_core_on_ack : 1;
  u32 sw_sm_pscbc_seq_in_override : 1;
  u32 mem_cph_rst_sw_override : 1;
  u32 pscbc_slp_stg_mode_csr : 1;
  u32 ignore_gdsc_pwr_dwn_csr : 1;
  u32 sreg_pscbc_spare_ctrl_in : 8;
  u32 sreg_pscbc_spare_ctrl_out : 8;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_boot_rom_ahb_sregr_u
{
  struct ipa_gcc_hwio_def_gcc_boot_rom_ahb_sregr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_TLMM_BCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_tlmm_bcr_s
{
  u32 blk_ares : 1;
  u32 reserved0 : 31;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_tlmm_bcr_u
{
  struct ipa_gcc_hwio_def_gcc_tlmm_bcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_TLMM_AHB_CBCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_tlmm_ahb_cbcr_s
{
  u32 reserved0 : 2;
  u32 clk_ares : 1;
  u32 reserved1 : 17;
  u32 ignore_rpmh_clk_dis : 1;
  u32 reserved2 : 1;
  u32 clk_dis : 1;
  u32 ignore_all_clk_dis : 1;
  u32 ignore_all_ares : 1;
  u32 reserved3 : 6;
  u32 clk_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_tlmm_ahb_cbcr_u
{
  struct ipa_gcc_hwio_def_gcc_tlmm_ahb_cbcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_TLMM_CBCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_tlmm_cbcr_s
{
  u32 reserved0 : 2;
  u32 clk_ares : 1;
  u32 reserved1 : 17;
  u32 ignore_rpmh_clk_dis : 1;
  u32 reserved2 : 1;
  u32 clk_dis : 1;
  u32 ignore_all_clk_dis : 1;
  u32 ignore_all_ares : 1;
  u32 reserved3 : 6;
  u32 clk_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_tlmm_cbcr_u
{
  struct ipa_gcc_hwio_def_gcc_tlmm_cbcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_AOSS_BCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_aoss_bcr_s
{
  u32 blk_ares : 1;
  u32 reserved0 : 31;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_aoss_bcr_u
{
  struct ipa_gcc_hwio_def_gcc_aoss_bcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_AOSS_CFG_AHB_CBCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_aoss_cfg_ahb_cbcr_s
{
  u32 clk_enable : 1;
  u32 reserved0 : 1;
  u32 clk_ares : 1;
  u32 reserved1 : 17;
  u32 ignore_rpmh_clk_dis : 1;
  u32 reserved2 : 1;
  u32 clk_dis : 1;
  u32 ignore_all_clk_dis : 1;
  u32 ignore_all_ares : 1;
  u32 reserved3 : 6;
  u32 clk_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_aoss_cfg_ahb_cbcr_u
{
  struct ipa_gcc_hwio_def_gcc_aoss_cfg_ahb_cbcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_AOSS_AT_CBCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_aoss_at_cbcr_s
{
  u32 clk_enable : 1;
  u32 hw_ctl : 1;
  u32 clk_ares : 1;
  u32 sw_only_en : 1;
  u32 reserved0 : 16;
  u32 ignore_rpmh_clk_dis : 1;
  u32 reserved1 : 1;
  u32 clk_dis : 1;
  u32 ignore_all_clk_dis : 1;
  u32 ignore_all_ares : 1;
  u32 reserved2 : 6;
  u32 clk_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_aoss_at_cbcr_u
{
  struct ipa_gcc_hwio_def_gcc_aoss_at_cbcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_SEC_CTRL_BCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_sec_ctrl_bcr_s
{
  u32 blk_ares : 1;
  u32 reserved0 : 31;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_sec_ctrl_bcr_u
{
  struct ipa_gcc_hwio_def_gcc_sec_ctrl_bcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_SEC_CTRL_ACC_CBCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_sec_ctrl_acc_cbcr_s
{
  u32 clk_enable : 1;
  u32 reserved0 : 1;
  u32 clk_ares : 1;
  u32 reserved1 : 19;
  u32 clk_dis : 1;
  u32 ignore_all_clk_dis : 1;
  u32 ignore_all_ares : 1;
  u32 reserved2 : 6;
  u32 clk_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_sec_ctrl_acc_cbcr_u
{
  struct ipa_gcc_hwio_def_gcc_sec_ctrl_acc_cbcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_SEC_CTRL_AHB_CBCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_sec_ctrl_ahb_cbcr_s
{
  u32 clk_enable : 1;
  u32 hw_ctl : 1;
  u32 clk_ares : 1;
  u32 sw_only_en : 1;
  u32 reserved0 : 16;
  u32 ignore_rpmh_clk_dis : 1;
  u32 reserved1 : 1;
  u32 clk_dis : 1;
  u32 ignore_all_clk_dis : 1;
  u32 ignore_all_ares : 1;
  u32 reserved2 : 6;
  u32 clk_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_sec_ctrl_ahb_cbcr_u
{
  struct ipa_gcc_hwio_def_gcc_sec_ctrl_ahb_cbcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_SEC_CTRL_CBCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_sec_ctrl_cbcr_s
{
  u32 clk_enable : 1;
  u32 reserved0 : 1;
  u32 clk_ares : 1;
  u32 reserved1 : 1;
  u32 sleep : 4;
  u32 wakeup : 4;
  u32 force_mem_periph_off : 1;
  u32 force_mem_periph_on : 1;
  u32 force_mem_core_on : 1;
  u32 reserved2 : 7;
  u32 clk_dis : 1;
  u32 ignore_all_clk_dis : 1;
  u32 ignore_all_ares : 1;
  u32 reserved3 : 6;
  u32 clk_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_sec_ctrl_cbcr_u
{
  struct ipa_gcc_hwio_def_gcc_sec_ctrl_cbcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_SEC_CTRL_SREGR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_sec_ctrl_sregr_s
{
  u32 reserved0 : 1;
  u32 sw_clk_en_slp_stg : 1;
  u32 sw_clk_en_sel_slp_stg : 1;
  u32 sw_ctrl_pwr_down : 1;
  u32 sw_rst_slp_stg : 1;
  u32 sw_rst_sel_slp_stg : 1;
  u32 force_clk_on : 1;
  u32 mem_cph_enable : 1;
  u32 sw_div_ratio_slp_stg_clk : 2;
  u32 mem_periph_on_ack : 1;
  u32 mem_core_on_ack : 1;
  u32 sw_sm_pscbc_seq_in_override : 1;
  u32 mem_cph_rst_sw_override : 1;
  u32 pscbc_slp_stg_mode_csr : 1;
  u32 ignore_gdsc_pwr_dwn_csr : 1;
  u32 sreg_pscbc_spare_ctrl_in : 8;
  u32 sreg_pscbc_spare_ctrl_out : 8;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_sec_ctrl_sregr_u
{
  struct ipa_gcc_hwio_def_gcc_sec_ctrl_sregr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_SEC_CTRL_SENSE_CBCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_sec_ctrl_sense_cbcr_s
{
  u32 clk_enable : 1;
  u32 reserved0 : 1;
  u32 clk_ares : 1;
  u32 reserved1 : 19;
  u32 clk_dis : 1;
  u32 ignore_all_clk_dis : 1;
  u32 ignore_all_ares : 1;
  u32 reserved2 : 6;
  u32 clk_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_sec_ctrl_sense_cbcr_u
{
  struct ipa_gcc_hwio_def_gcc_sec_ctrl_sense_cbcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_SEC_CTRL_BOOT_ROM_PATCH_CBCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_sec_ctrl_boot_rom_patch_cbcr_s
{
  u32 clk_enable : 1;
  u32 reserved0 : 1;
  u32 clk_ares : 1;
  u32 reserved1 : 17;
  u32 ignore_rpmh_clk_dis : 1;
  u32 reserved2 : 1;
  u32 clk_dis : 1;
  u32 ignore_all_clk_dis : 1;
  u32 ignore_all_ares : 1;
  u32 reserved3 : 6;
  u32 clk_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_sec_ctrl_boot_rom_patch_cbcr_u
{
  struct ipa_gcc_hwio_def_gcc_sec_ctrl_boot_rom_patch_cbcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_ACC_CMD_RCGR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_acc_cmd_rcgr_s
{
  u32 update : 1;
  u32 root_en : 1;
  u32 reserved0 : 2;
  u32 dirty_cfg_rcgr : 1;
  u32 reserved1 : 26;
  u32 root_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_acc_cmd_rcgr_u
{
  struct ipa_gcc_hwio_def_gcc_acc_cmd_rcgr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_ACC_CFG_RCGR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_acc_cfg_rcgr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 5;
  u32 rcglite_disable : 1;
  u32 reserved2 : 3;
  u32 hw_clk_control : 1;
  u32 reserved3 : 11;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_acc_cfg_rcgr_u
{
  struct ipa_gcc_hwio_def_gcc_acc_cfg_rcgr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_SEC_CTRL_CMD_RCGR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_sec_ctrl_cmd_rcgr_s
{
  u32 update : 1;
  u32 root_en : 1;
  u32 reserved0 : 2;
  u32 dirty_cfg_rcgr : 1;
  u32 reserved1 : 26;
  u32 root_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_sec_ctrl_cmd_rcgr_u
{
  struct ipa_gcc_hwio_def_gcc_sec_ctrl_cmd_rcgr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_SEC_CTRL_CFG_RCGR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_sec_ctrl_cfg_rcgr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 5;
  u32 rcglite_disable : 1;
  u32 reserved2 : 3;
  u32 hw_clk_control : 1;
  u32 reserved3 : 11;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_sec_ctrl_cfg_rcgr_u
{
  struct ipa_gcc_hwio_def_gcc_sec_ctrl_cfg_rcgr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_SPDM_BCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_spdm_bcr_s
{
  u32 blk_ares : 1;
  u32 reserved0 : 31;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_spdm_bcr_u
{
  struct ipa_gcc_hwio_def_gcc_spdm_bcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_SPDM_CFG_AHB_CBCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_spdm_cfg_ahb_cbcr_s
{
  u32 clk_enable : 1;
  u32 reserved0 : 1;
  u32 clk_ares : 1;
  u32 reserved1 : 17;
  u32 ignore_rpmh_clk_dis : 1;
  u32 reserved2 : 1;
  u32 clk_dis : 1;
  u32 ignore_all_clk_dis : 1;
  u32 ignore_all_ares : 1;
  u32 reserved3 : 6;
  u32 clk_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_spdm_cfg_ahb_cbcr_u
{
  struct ipa_gcc_hwio_def_gcc_spdm_cfg_ahb_cbcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_SPDM_MSTR_AHB_CBCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_spdm_mstr_ahb_cbcr_s
{
  u32 clk_enable : 1;
  u32 reserved0 : 1;
  u32 clk_ares : 1;
  u32 reserved1 : 17;
  u32 ignore_rpmh_clk_dis : 1;
  u32 reserved2 : 1;
  u32 clk_dis : 1;
  u32 ignore_all_clk_dis : 1;
  u32 ignore_all_ares : 1;
  u32 reserved3 : 6;
  u32 clk_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_spdm_mstr_ahb_cbcr_u
{
  struct ipa_gcc_hwio_def_gcc_spdm_mstr_ahb_cbcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_SPDM_FF_CBCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_spdm_ff_cbcr_s
{
  u32 clk_enable : 1;
  u32 reserved0 : 1;
  u32 clk_ares : 1;
  u32 reserved1 : 19;
  u32 clk_dis : 1;
  u32 ignore_all_clk_dis : 1;
  u32 ignore_all_ares : 1;
  u32 reserved2 : 6;
  u32 clk_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_spdm_ff_cbcr_u
{
  struct ipa_gcc_hwio_def_gcc_spdm_ff_cbcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_SPDM_MEMNOC_CY_CBCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_spdm_memnoc_cy_cbcr_s
{
  u32 clk_enable : 1;
  u32 reserved0 : 1;
  u32 clk_ares : 1;
  u32 reserved1 : 17;
  u32 ignore_rpmh_clk_dis : 1;
  u32 reserved2 : 1;
  u32 clk_dis : 1;
  u32 ignore_all_clk_dis : 1;
  u32 ignore_all_ares : 1;
  u32 reserved3 : 6;
  u32 clk_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_spdm_memnoc_cy_cbcr_u
{
  struct ipa_gcc_hwio_def_gcc_spdm_memnoc_cy_cbcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_SPDM_SNOC_CY_CBCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_spdm_snoc_cy_cbcr_s
{
  u32 clk_enable : 1;
  u32 reserved0 : 1;
  u32 clk_ares : 1;
  u32 reserved1 : 17;
  u32 ignore_rpmh_clk_dis : 1;
  u32 reserved2 : 1;
  u32 clk_dis : 1;
  u32 ignore_all_clk_dis : 1;
  u32 ignore_all_ares : 1;
  u32 reserved3 : 6;
  u32 clk_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_spdm_snoc_cy_cbcr_u
{
  struct ipa_gcc_hwio_def_gcc_spdm_snoc_cy_cbcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_SPDM_DEBUG_CY_CBCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_spdm_debug_cy_cbcr_s
{
  u32 clk_enable : 1;
  u32 reserved0 : 1;
  u32 clk_ares : 1;
  u32 reserved1 : 19;
  u32 clk_dis : 1;
  u32 ignore_all_clk_dis : 1;
  u32 ignore_all_ares : 1;
  u32 reserved2 : 6;
  u32 clk_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_spdm_debug_cy_cbcr_u
{
  struct ipa_gcc_hwio_def_gcc_spdm_debug_cy_cbcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_ULTAUDIO_AHBFABRIC_EFABRIC_SPDM_CBCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_ultaudio_ahbfabric_efabric_spdm_cbcr_s
{
  u32 clk_enable : 1;
  u32 reserved0 : 1;
  u32 clk_ares : 1;
  u32 reserved1 : 19;
  u32 clk_dis : 1;
  u32 ignore_all_clk_dis : 1;
  u32 ignore_all_ares : 1;
  u32 reserved2 : 6;
  u32 clk_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_ultaudio_ahbfabric_efabric_spdm_cbcr_u
{
  struct ipa_gcc_hwio_def_gcc_ultaudio_ahbfabric_efabric_spdm_cbcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_SPDM_PNOC_CY_CBCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_spdm_pnoc_cy_cbcr_s
{
  u32 clk_enable : 1;
  u32 reserved0 : 1;
  u32 clk_ares : 1;
  u32 reserved1 : 17;
  u32 ignore_rpmh_clk_dis : 1;
  u32 reserved2 : 1;
  u32 clk_dis : 1;
  u32 ignore_all_clk_dis : 1;
  u32 ignore_all_ares : 1;
  u32 reserved3 : 6;
  u32 clk_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_spdm_pnoc_cy_cbcr_u
{
  struct ipa_gcc_hwio_def_gcc_spdm_pnoc_cy_cbcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_SPDM_MEMNOC_CY_CDIVR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_spdm_memnoc_cy_cdivr_s
{
  u32 clk_div : 4;
  u32 reserved0 : 28;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_spdm_memnoc_cy_cdivr_u
{
  struct ipa_gcc_hwio_def_gcc_spdm_memnoc_cy_cdivr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_SPDM_SNOC_CY_CDIVR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_spdm_snoc_cy_cdivr_s
{
  u32 clk_div : 4;
  u32 reserved0 : 28;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_spdm_snoc_cy_cdivr_u
{
  struct ipa_gcc_hwio_def_gcc_spdm_snoc_cy_cdivr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_SPDM_DEBUG_CY_CDIVR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_spdm_debug_cy_cdivr_s
{
  u32 clk_div : 4;
  u32 reserved0 : 28;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_spdm_debug_cy_cdivr_u
{
  struct ipa_gcc_hwio_def_gcc_spdm_debug_cy_cdivr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_CE1_BCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_ce1_bcr_s
{
  u32 blk_ares : 1;
  u32 reserved0 : 31;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_ce1_bcr_u
{
  struct ipa_gcc_hwio_def_gcc_ce1_bcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_CE1_CBCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_ce1_cbcr_s
{
  u32 reserved0 : 2;
  u32 clk_ares : 1;
  u32 reserved1 : 1;
  u32 sleep : 4;
  u32 wakeup : 4;
  u32 force_mem_periph_off : 1;
  u32 force_mem_periph_on : 1;
  u32 force_mem_core_on : 1;
  u32 reserved2 : 5;
  u32 ignore_rpmh_clk_dis : 1;
  u32 reserved3 : 1;
  u32 clk_dis : 1;
  u32 ignore_all_clk_dis : 1;
  u32 ignore_all_ares : 1;
  u32 reserved4 : 6;
  u32 clk_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_ce1_cbcr_u
{
  struct ipa_gcc_hwio_def_gcc_ce1_cbcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_CE1_SREGR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_ce1_sregr_s
{
  u32 reserved0 : 1;
  u32 sw_clk_en_slp_stg : 1;
  u32 sw_clk_en_sel_slp_stg : 1;
  u32 sw_ctrl_pwr_down : 1;
  u32 sw_rst_slp_stg : 1;
  u32 sw_rst_sel_slp_stg : 1;
  u32 force_clk_on : 1;
  u32 mem_cph_enable : 1;
  u32 sw_div_ratio_slp_stg_clk : 2;
  u32 mem_periph_on_ack : 1;
  u32 mem_core_on_ack : 1;
  u32 sw_sm_pscbc_seq_in_override : 1;
  u32 mem_cph_rst_sw_override : 1;
  u32 pscbc_slp_stg_mode_csr : 1;
  u32 ignore_gdsc_pwr_dwn_csr : 1;
  u32 sreg_pscbc_spare_ctrl_in : 8;
  u32 sreg_pscbc_spare_ctrl_out : 8;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_ce1_sregr_u
{
  struct ipa_gcc_hwio_def_gcc_ce1_sregr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_CE1_AXI_CBCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_ce1_axi_cbcr_s
{
  u32 reserved0 : 2;
  u32 clk_ares : 1;
  u32 reserved1 : 17;
  u32 ignore_rpmh_clk_dis : 1;
  u32 reserved2 : 1;
  u32 clk_dis : 1;
  u32 ignore_all_clk_dis : 1;
  u32 ignore_all_ares : 1;
  u32 reserved3 : 6;
  u32 clk_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_ce1_axi_cbcr_u
{
  struct ipa_gcc_hwio_def_gcc_ce1_axi_cbcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_CE1_AHB_CBCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_ce1_ahb_cbcr_s
{
  u32 reserved0 : 1;
  u32 hw_ctl : 1;
  u32 clk_ares : 1;
  u32 sw_only_en : 1;
  u32 reserved1 : 16;
  u32 ignore_rpmh_clk_dis : 1;
  u32 reserved2 : 1;
  u32 clk_dis : 1;
  u32 ignore_all_clk_dis : 1;
  u32 ignore_all_ares : 1;
  u32 reserved3 : 6;
  u32 clk_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_ce1_ahb_cbcr_u
{
  struct ipa_gcc_hwio_def_gcc_ce1_ahb_cbcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_CE_CMD_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_ce_cmd_dfsr_s
{
  u32 dfs_en : 1;
  u32 curr_perf_state : 4;
  u32 hw_clk_control : 1;
  u32 dfs_fsm_state : 3;
  u32 perf_state_update_status : 1;
  u32 sw_override : 1;
  u32 sw_perf_state : 4;
  u32 rcg_sw_ctrl : 1;
  u32 reserved0 : 16;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_ce_cmd_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_ce_cmd_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_CE_CE1_PERF0_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_ce_ce1_perf0_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 21;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_ce_ce1_perf0_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_ce_ce1_perf0_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_CE_CE1_PERF1_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_ce_ce1_perf1_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 21;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_ce_ce1_perf1_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_ce_ce1_perf1_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_CE_CE1_PERF2_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_ce_ce1_perf2_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 21;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_ce_ce1_perf2_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_ce_ce1_perf2_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_CE_CE1_PERF3_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_ce_ce1_perf3_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 21;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_ce_ce1_perf3_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_ce_ce1_perf3_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_CE_CE1_PERF4_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_ce_ce1_perf4_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 21;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_ce_ce1_perf4_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_ce_ce1_perf4_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_CE_CE1_PERF5_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_ce_ce1_perf5_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 21;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_ce_ce1_perf5_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_ce_ce1_perf5_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_CE_CE1_PERF6_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_ce_ce1_perf6_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 21;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_ce_ce1_perf6_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_ce_ce1_perf6_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_CE_CE1_PERF7_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_ce_ce1_perf7_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 21;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_ce_ce1_perf7_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_ce_ce1_perf7_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_CE_CE1_PERF8_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_ce_ce1_perf8_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 21;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_ce_ce1_perf8_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_ce_ce1_perf8_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_CE_CE1_PERF9_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_ce_ce1_perf9_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 21;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_ce_ce1_perf9_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_ce_ce1_perf9_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_CE_CE1_PERF10_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_ce_ce1_perf10_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 21;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_ce_ce1_perf10_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_ce_ce1_perf10_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_CE_CE1_PERF11_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_ce_ce1_perf11_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 21;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_ce_ce1_perf11_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_ce_ce1_perf11_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_CE_CE1_PERF12_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_ce_ce1_perf12_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 21;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_ce_ce1_perf12_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_ce_ce1_perf12_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_CE_CE1_PERF13_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_ce_ce1_perf13_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 21;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_ce_ce1_perf13_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_ce_ce1_perf13_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_CE_CE1_PERF14_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_ce_ce1_perf14_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 21;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_ce_ce1_perf14_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_ce_ce1_perf14_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_CE_CE1_PERF15_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_ce_ce1_perf15_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 21;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_ce_ce1_perf15_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_ce_ce1_perf15_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_CE1_CMD_RCGR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_ce1_cmd_rcgr_s
{
  u32 update : 1;
  u32 root_en : 1;
  u32 reserved0 : 2;
  u32 dirty_cfg_rcgr : 1;
  u32 reserved1 : 26;
  u32 root_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_ce1_cmd_rcgr_u
{
  struct ipa_gcc_hwio_def_gcc_ce1_cmd_rcgr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_CE1_CFG_RCGR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_ce1_cfg_rcgr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 5;
  u32 rcglite_disable : 1;
  u32 reserved2 : 3;
  u32 hw_clk_control : 1;
  u32 reserved3 : 11;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_ce1_cfg_rcgr_u
{
  struct ipa_gcc_hwio_def_gcc_ce1_cfg_rcgr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_AHB_CBCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_ahb_cbcr_s
{
  u32 clk_enable : 1;
  u32 reserved0 : 1;
  u32 clk_ares : 1;
  u32 reserved1 : 17;
  u32 ignore_rpmh_clk_dis : 1;
  u32 reserved2 : 1;
  u32 clk_dis : 1;
  u32 ignore_all_clk_dis : 1;
  u32 ignore_all_ares : 1;
  u32 reserved3 : 6;
  u32 clk_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_ahb_cbcr_u
{
  struct ipa_gcc_hwio_def_gcc_ahb_cbcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_AHB_PCIE_LINK_CBCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_ahb_pcie_link_cbcr_s
{
  u32 clk_enable : 1;
  u32 reserved0 : 1;
  u32 clk_ares : 1;
  u32 reserved1 : 17;
  u32 ignore_rpmh_clk_dis : 1;
  u32 reserved2 : 1;
  u32 clk_dis : 1;
  u32 ignore_all_clk_dis : 1;
  u32 ignore_all_ares : 1;
  u32 reserved3 : 6;
  u32 clk_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_ahb_pcie_link_cbcr_u
{
  struct ipa_gcc_hwio_def_gcc_ahb_pcie_link_cbcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_XO_PCIE_LINK_CBCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_xo_pcie_link_cbcr_s
{
  u32 clk_enable : 1;
  u32 hw_ctl : 1;
  u32 clk_ares : 1;
  u32 sw_only_en : 1;
  u32 reserved0 : 18;
  u32 clk_dis : 1;
  u32 ignore_all_clk_dis : 1;
  u32 ignore_all_ares : 1;
  u32 reserved1 : 6;
  u32 clk_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_xo_pcie_link_cbcr_u
{
  struct ipa_gcc_hwio_def_gcc_xo_pcie_link_cbcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_XO_CBCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_xo_cbcr_s
{
  u32 clk_enable : 1;
  u32 reserved0 : 1;
  u32 clk_ares : 1;
  u32 reserved1 : 19;
  u32 clk_dis : 1;
  u32 ignore_all_clk_dis : 1;
  u32 ignore_all_ares : 1;
  u32 reserved2 : 6;
  u32 clk_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_xo_cbcr_u
{
  struct ipa_gcc_hwio_def_gcc_xo_cbcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_XO_DIV4_CBCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_xo_div4_cbcr_s
{
  u32 clk_enable : 1;
  u32 reserved0 : 1;
  u32 clk_ares : 1;
  u32 reserved1 : 19;
  u32 clk_dis : 1;
  u32 ignore_all_clk_dis : 1;
  u32 ignore_all_ares : 1;
  u32 reserved2 : 6;
  u32 clk_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_xo_div4_cbcr_u
{
  struct ipa_gcc_hwio_def_gcc_xo_div4_cbcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_SLEEP_CBCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_sleep_cbcr_s
{
  u32 clk_enable : 1;
  u32 reserved0 : 1;
  u32 clk_ares : 1;
  u32 reserved1 : 19;
  u32 clk_dis : 1;
  u32 ignore_all_clk_dis : 1;
  u32 ignore_all_ares : 1;
  u32 reserved2 : 6;
  u32 clk_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_sleep_cbcr_u
{
  struct ipa_gcc_hwio_def_gcc_sleep_cbcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_XO_DIV4_CDIVR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_xo_div4_cdivr_s
{
  u32 clk_div : 4;
  u32 reserved0 : 28;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_xo_div4_cdivr_u
{
  struct ipa_gcc_hwio_def_gcc_xo_div4_cdivr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_SLEEP_CMD_RCGR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_sleep_cmd_rcgr_s
{
  u32 update : 1;
  u32 root_en : 1;
  u32 reserved0 : 2;
  u32 dirty_cfg_rcgr : 1;
  u32 reserved1 : 26;
  u32 root_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_sleep_cmd_rcgr_u
{
  struct ipa_gcc_hwio_def_gcc_sleep_cmd_rcgr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_SLEEP_CFG_RCGR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_sleep_cfg_rcgr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 5;
  u32 rcglite_disable : 1;
  u32 reserved2 : 3;
  u32 hw_clk_control : 1;
  u32 reserved3 : 11;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_sleep_cfg_rcgr_u
{
  struct ipa_gcc_hwio_def_gcc_sleep_cfg_rcgr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_XO_CMD_RCGR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_xo_cmd_rcgr_s
{
  u32 update : 1;
  u32 root_en : 1;
  u32 reserved0 : 2;
  u32 dirty_cfg_rcgr : 1;
  u32 reserved1 : 26;
  u32 root_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_xo_cmd_rcgr_u
{
  struct ipa_gcc_hwio_def_gcc_xo_cmd_rcgr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_XO_CFG_RCGR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_xo_cfg_rcgr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 5;
  u32 rcglite_disable : 1;
  u32 reserved2 : 3;
  u32 hw_clk_control : 1;
  u32 reserved3 : 11;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_xo_cfg_rcgr_u
{
  struct ipa_gcc_hwio_def_gcc_xo_cfg_rcgr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_DDRSS_BCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_ddrss_bcr_s
{
  u32 blk_ares : 1;
  u32 reserved0 : 31;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_ddrss_bcr_u
{
  struct ipa_gcc_hwio_def_gcc_ddrss_bcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_DDRSS_GDSCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_ddrss_gdscr_s
{
  u32 sw_collapse : 1;
  u32 hw_control : 1;
  u32 sw_override : 1;
  u32 pd_ares : 1;
  u32 clk_disable : 1;
  u32 clamp_io : 1;
  u32 en_few : 1;
  u32 en_rest : 1;
  u32 retain : 1;
  u32 save : 1;
  u32 restore : 1;
  u32 retain_ff_enable : 1;
  u32 clk_dis_wait : 4;
  u32 en_few_wait : 4;
  u32 en_rest_wait : 4;
  u32 reserved0 : 3;
  u32 gdsc_state : 4;
  u32 pwr_on : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_ddrss_gdscr_u
{
  struct ipa_gcc_hwio_def_gcc_ddrss_gdscr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_DDRSS_CFG_GDSCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_ddrss_cfg_gdscr_s
{
  u32 disable_clk_software_override : 1;
  u32 clamp_io_software_override : 1;
  u32 save_restore_software_override : 1;
  u32 unclamp_io_software_override : 1;
  u32 gdsc_pscbc_pwr_dwn_sw : 1;
  u32 gdsc_phase_reset_delay_count_sw : 2;
  u32 gdsc_phase_reset_en_sw : 1;
  u32 gdsc_mem_core_force_in_sw : 1;
  u32 gdsc_mem_peri_force_in_sw : 1;
  u32 gdsc_handshake_dis : 1;
  u32 software_control_override : 4;
  u32 gdsc_power_down_complete : 1;
  u32 gdsc_power_up_complete : 1;
  u32 gdsc_enf_ack_status : 1;
  u32 gdsc_enr_ack_status : 1;
  u32 gdsc_mem_pwr_ack_status : 1;
  u32 gdsc_cfg_fsm_state_status : 4;
  u32 gdsc_pwr_up_start : 1;
  u32 gdsc_pwr_dwn_start : 1;
  u32 reserved0 : 6;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_ddrss_cfg_gdscr_u
{
  struct ipa_gcc_hwio_def_gcc_ddrss_cfg_gdscr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_DDRSS_CFG2_GDSCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_ddrss_cfg2_gdscr_s
{
  u32 mem_pwr_dwn_timeout : 4;
  u32 dly_assert_clamp_mem : 4;
  u32 dly_deassert_clamp_mem : 4;
  u32 dly_mem_pwr_up : 4;
  u32 gdsc_clamp_mem_sw : 1;
  u32 gdsc_pwrdwn_enable_ack_override : 1;
  u32 gdsc_mem_pwrup_ack_override : 1;
  u32 reserved0 : 13;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_ddrss_cfg2_gdscr_u
{
  struct ipa_gcc_hwio_def_gcc_ddrss_cfg2_gdscr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_DDRSS_CFG3_GDSCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_ddrss_cfg3_gdscr_s
{
  u32 gdsc_spare_ctrl_out : 8;
  u32 gdsc_spare_ctrl_in : 8;
  u32 gdsc_accu_red_sw_override : 1;
  u32 gdsc_accu_red_shifter_start_sw : 1;
  u32 gdsc_accu_red_shifter_clk_en_sw : 1;
  u32 gdsc_accu_red_shifter_done_override : 1;
  u32 gdsc_accu_red_timer_en_sw : 1;
  u32 dly_accu_red_shifter_done : 4;
  u32 gdsc_accu_red_enable : 1;
  u32 gdsc_accu_red_shifter_done_status : 1;
  u32 reserved0 : 5;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_ddrss_cfg3_gdscr_u
{
  struct ipa_gcc_hwio_def_gcc_ddrss_cfg3_gdscr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_DDRSS_CFG4_GDSCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_ddrss_cfg4_gdscr_s
{
  u32 dly_retainff : 4;
  u32 dly_clampio : 4;
  u32 dly_deassertares : 4;
  u32 dly_noretainff : 4;
  u32 dly_restoreff : 4;
  u32 dly_unclampio : 4;
  u32 reserved0 : 8;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_ddrss_cfg4_gdscr_u
{
  struct ipa_gcc_hwio_def_gcc_ddrss_cfg4_gdscr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_DDRSS_TCU_CBCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_ddrss_tcu_cbcr_s
{
  u32 clk_enable : 1;
  u32 hw_ctl : 1;
  u32 clk_ares : 1;
  u32 sw_only_en : 1;
  u32 reserved0 : 16;
  u32 ignore_rpmh_clk_dis : 1;
  u32 reserved1 : 1;
  u32 clk_dis : 1;
  u32 ignore_all_clk_dis : 1;
  u32 ignore_all_ares : 1;
  u32 reserved2 : 6;
  u32 clk_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_ddrss_tcu_cbcr_u
{
  struct ipa_gcc_hwio_def_gcc_ddrss_tcu_cbcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_DDRSS_SYS_NOC_AXI_CBCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_ddrss_sys_noc_axi_cbcr_s
{
  u32 clk_enable : 1;
  u32 hw_ctl : 1;
  u32 clk_ares : 1;
  u32 sw_only_en : 1;
  u32 reserved0 : 16;
  u32 ignore_rpmh_clk_dis : 1;
  u32 reserved1 : 1;
  u32 clk_dis : 1;
  u32 ignore_all_clk_dis : 1;
  u32 ignore_all_ares : 1;
  u32 reserved2 : 6;
  u32 clk_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_ddrss_sys_noc_axi_cbcr_u
{
  struct ipa_gcc_hwio_def_gcc_ddrss_sys_noc_axi_cbcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_DDRSS_SYS_NOC_HS_AXI_CBCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_ddrss_sys_noc_hs_axi_cbcr_s
{
  u32 clk_enable : 1;
  u32 hw_ctl : 1;
  u32 clk_ares : 1;
  u32 sw_only_en : 1;
  u32 reserved0 : 16;
  u32 ignore_rpmh_clk_dis : 1;
  u32 reserved1 : 1;
  u32 clk_dis : 1;
  u32 ignore_all_clk_dis : 1;
  u32 ignore_all_ares : 1;
  u32 reserved2 : 6;
  u32 clk_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_ddrss_sys_noc_hs_axi_cbcr_u
{
  struct ipa_gcc_hwio_def_gcc_ddrss_sys_noc_hs_axi_cbcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_DDRSS_XO_CBCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_ddrss_xo_cbcr_s
{
  u32 clk_enable : 1;
  u32 hw_ctl : 1;
  u32 clk_ares : 1;
  u32 sw_only_en : 1;
  u32 reserved0 : 18;
  u32 clk_dis : 1;
  u32 ignore_all_clk_dis : 1;
  u32 ignore_all_ares : 1;
  u32 reserved1 : 6;
  u32 clk_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_ddrss_xo_cbcr_u
{
  struct ipa_gcc_hwio_def_gcc_ddrss_xo_cbcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_DDRSS_CFG_AHB_CBCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_ddrss_cfg_ahb_cbcr_s
{
  u32 clk_enable : 1;
  u32 hw_ctl : 1;
  u32 clk_ares : 1;
  u32 sw_only_en : 1;
  u32 reserved0 : 16;
  u32 ignore_rpmh_clk_dis : 1;
  u32 reserved1 : 1;
  u32 clk_dis : 1;
  u32 ignore_all_clk_dis : 1;
  u32 ignore_all_ares : 1;
  u32 reserved2 : 6;
  u32 clk_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_ddrss_cfg_ahb_cbcr_u
{
  struct ipa_gcc_hwio_def_gcc_ddrss_cfg_ahb_cbcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_DDRSS_SLEEP_CBCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_ddrss_sleep_cbcr_s
{
  u32 clk_enable : 1;
  u32 hw_ctl : 1;
  u32 clk_ares : 1;
  u32 sw_only_en : 1;
  u32 reserved0 : 18;
  u32 clk_dis : 1;
  u32 ignore_all_clk_dis : 1;
  u32 ignore_all_ares : 1;
  u32 reserved1 : 6;
  u32 clk_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_ddrss_sleep_cbcr_u
{
  struct ipa_gcc_hwio_def_gcc_ddrss_sleep_cbcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_MEMNOC_CBCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_memnoc_cbcr_s
{
  u32 clk_enable : 1;
  u32 hw_ctl : 1;
  u32 clk_ares : 1;
  u32 sw_only_en : 1;
  u32 reserved0 : 16;
  u32 ignore_rpmh_clk_dis : 1;
  u32 reserved1 : 1;
  u32 clk_dis : 1;
  u32 ignore_all_clk_dis : 1;
  u32 ignore_all_ares : 1;
  u32 reserved2 : 6;
  u32 clk_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_memnoc_cbcr_u
{
  struct ipa_gcc_hwio_def_gcc_memnoc_cbcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_DDRSS_AT_CBCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_ddrss_at_cbcr_s
{
  u32 clk_enable : 1;
  u32 hw_ctl : 1;
  u32 clk_ares : 1;
  u32 sw_only_en : 1;
  u32 reserved0 : 16;
  u32 ignore_rpmh_clk_dis : 1;
  u32 reserved1 : 1;
  u32 clk_dis : 1;
  u32 ignore_all_clk_dis : 1;
  u32 ignore_all_ares : 1;
  u32 reserved2 : 6;
  u32 clk_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_ddrss_at_cbcr_u
{
  struct ipa_gcc_hwio_def_gcc_ddrss_at_cbcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_DDRSS_MSS_MCDMA_CBCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_ddrss_mss_mcdma_cbcr_s
{
  u32 clk_enable : 1;
  u32 hw_ctl : 1;
  u32 clk_ares : 1;
  u32 sw_only_en : 1;
  u32 reserved0 : 16;
  u32 ignore_rpmh_clk_dis : 1;
  u32 reserved1 : 1;
  u32 clk_dis : 1;
  u32 ignore_all_clk_dis : 1;
  u32 ignore_all_ares : 1;
  u32 reserved2 : 6;
  u32 clk_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_ddrss_mss_mcdma_cbcr_u
{
  struct ipa_gcc_hwio_def_gcc_ddrss_mss_mcdma_cbcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_SHUB_MEMNOC_PERF0_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_shub_memnoc_perf0_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 21;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_shub_memnoc_perf0_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_shub_memnoc_perf0_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_SHUB_MEMNOC_PERF1_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_shub_memnoc_perf1_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 21;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_shub_memnoc_perf1_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_shub_memnoc_perf1_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_SHUB_MEMNOC_PERF2_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_shub_memnoc_perf2_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 21;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_shub_memnoc_perf2_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_shub_memnoc_perf2_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_SHUB_MEMNOC_PERF3_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_shub_memnoc_perf3_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 21;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_shub_memnoc_perf3_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_shub_memnoc_perf3_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_SHUB_MEMNOC_PERF4_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_shub_memnoc_perf4_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 21;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_shub_memnoc_perf4_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_shub_memnoc_perf4_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_SHUB_MEMNOC_PERF5_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_shub_memnoc_perf5_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 21;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_shub_memnoc_perf5_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_shub_memnoc_perf5_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_SHUB_MEMNOC_PERF6_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_shub_memnoc_perf6_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 21;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_shub_memnoc_perf6_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_shub_memnoc_perf6_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_SHUB_MEMNOC_PERF7_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_shub_memnoc_perf7_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 21;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_shub_memnoc_perf7_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_shub_memnoc_perf7_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_SHUB_MEMNOC_PERF8_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_shub_memnoc_perf8_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 21;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_shub_memnoc_perf8_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_shub_memnoc_perf8_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_SHUB_MEMNOC_PERF9_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_shub_memnoc_perf9_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 21;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_shub_memnoc_perf9_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_shub_memnoc_perf9_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_SHUB_MEMNOC_PERF10_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_shub_memnoc_perf10_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 21;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_shub_memnoc_perf10_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_shub_memnoc_perf10_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_SHUB_MEMNOC_PERF11_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_shub_memnoc_perf11_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 21;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_shub_memnoc_perf11_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_shub_memnoc_perf11_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_SHUB_MEMNOC_PERF12_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_shub_memnoc_perf12_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 21;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_shub_memnoc_perf12_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_shub_memnoc_perf12_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_SHUB_MEMNOC_PERF13_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_shub_memnoc_perf13_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 21;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_shub_memnoc_perf13_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_shub_memnoc_perf13_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_SHUB_MEMNOC_PERF14_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_shub_memnoc_perf14_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 21;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_shub_memnoc_perf14_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_shub_memnoc_perf14_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_SHUB_MEMNOC_PERF15_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_shub_memnoc_perf15_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 21;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_shub_memnoc_perf15_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_shub_memnoc_perf15_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_MEMNOC_CMD_RCGR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_memnoc_cmd_rcgr_s
{
  u32 update : 1;
  u32 root_en : 1;
  u32 reserved0 : 2;
  u32 dirty_cfg_rcgr : 1;
  u32 reserved1 : 26;
  u32 root_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_memnoc_cmd_rcgr_u
{
  struct ipa_gcc_hwio_def_gcc_memnoc_cmd_rcgr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_MEMNOC_CFG_RCGR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_memnoc_cfg_rcgr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 5;
  u32 rcglite_disable : 1;
  u32 reserved2 : 3;
  u32 hw_clk_control : 1;
  u32 reserved3 : 11;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_memnoc_cfg_rcgr_u
{
  struct ipa_gcc_hwio_def_gcc_memnoc_cfg_rcgr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_SHRM_CMD_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_shrm_cmd_dfsr_s
{
  u32 dfs_en : 1;
  u32 curr_perf_state : 4;
  u32 hw_clk_control : 1;
  u32 dfs_fsm_state : 3;
  u32 perf_state_update_status : 1;
  u32 sw_override : 1;
  u32 sw_perf_state : 4;
  u32 rcg_sw_ctrl : 1;
  u32 reserved0 : 16;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_shrm_cmd_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_shrm_cmd_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_SHRM_SHRM_PERF0_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_shrm_shrm_perf0_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 21;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_shrm_shrm_perf0_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_shrm_shrm_perf0_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_SHRM_SHRM_PERF1_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_shrm_shrm_perf1_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 21;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_shrm_shrm_perf1_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_shrm_shrm_perf1_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_SHRM_SHRM_PERF2_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_shrm_shrm_perf2_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 21;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_shrm_shrm_perf2_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_shrm_shrm_perf2_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_SHRM_SHRM_PERF3_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_shrm_shrm_perf3_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 21;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_shrm_shrm_perf3_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_shrm_shrm_perf3_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_SHRM_SHRM_PERF4_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_shrm_shrm_perf4_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 21;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_shrm_shrm_perf4_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_shrm_shrm_perf4_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_SHRM_SHRM_PERF5_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_shrm_shrm_perf5_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 21;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_shrm_shrm_perf5_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_shrm_shrm_perf5_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_SHRM_SHRM_PERF6_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_shrm_shrm_perf6_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 21;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_shrm_shrm_perf6_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_shrm_shrm_perf6_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_SHRM_SHRM_PERF7_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_shrm_shrm_perf7_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 21;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_shrm_shrm_perf7_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_shrm_shrm_perf7_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_SHRM_SHRM_PERF8_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_shrm_shrm_perf8_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 21;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_shrm_shrm_perf8_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_shrm_shrm_perf8_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_SHRM_SHRM_PERF9_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_shrm_shrm_perf9_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 21;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_shrm_shrm_perf9_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_shrm_shrm_perf9_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_SHRM_SHRM_PERF10_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_shrm_shrm_perf10_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 21;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_shrm_shrm_perf10_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_shrm_shrm_perf10_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_SHRM_SHRM_PERF11_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_shrm_shrm_perf11_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 21;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_shrm_shrm_perf11_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_shrm_shrm_perf11_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_SHRM_SHRM_PERF12_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_shrm_shrm_perf12_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 21;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_shrm_shrm_perf12_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_shrm_shrm_perf12_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_SHRM_SHRM_PERF13_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_shrm_shrm_perf13_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 21;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_shrm_shrm_perf13_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_shrm_shrm_perf13_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_SHRM_SHRM_PERF14_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_shrm_shrm_perf14_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 21;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_shrm_shrm_perf14_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_shrm_shrm_perf14_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_SHRM_SHRM_PERF15_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_shrm_shrm_perf15_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 21;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_shrm_shrm_perf15_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_shrm_shrm_perf15_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_SHRM_CMD_RCGR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_shrm_cmd_rcgr_s
{
  u32 update : 1;
  u32 root_en : 1;
  u32 reserved0 : 2;
  u32 dirty_cfg_rcgr : 1;
  u32 reserved1 : 26;
  u32 root_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_shrm_cmd_rcgr_u
{
  struct ipa_gcc_hwio_def_gcc_shrm_cmd_rcgr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_SHRM_CFG_RCGR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_shrm_cfg_rcgr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 5;
  u32 rcglite_disable : 1;
  u32 reserved2 : 3;
  u32 hw_clk_control : 1;
  u32 reserved3 : 11;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_shrm_cfg_rcgr_u
{
  struct ipa_gcc_hwio_def_gcc_shrm_cfg_rcgr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_SHRM_DCD_CDIV_DCDR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_shrm_dcd_cdiv_dcdr_s
{
  u32 dcd_enable : 1;
  u32 reserved0 : 31;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_shrm_dcd_cdiv_dcdr_u
{
  struct ipa_gcc_hwio_def_gcc_shrm_dcd_cdiv_dcdr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_MEMNOC_DCD_CDIV_DCDR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_memnoc_dcd_cdiv_dcdr_s
{
  u32 dcd_enable : 1;
  u32 reserved0 : 31;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_memnoc_dcd_cdiv_dcdr_u
{
  struct ipa_gcc_hwio_def_gcc_memnoc_dcd_cdiv_dcdr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_DDR_I_HCLK_CBCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_ddr_i_hclk_cbcr_s
{
  u32 clk_enable : 1;
  u32 hw_ctl : 1;
  u32 clk_ares : 1;
  u32 sw_only_en : 1;
  u32 reserved0 : 18;
  u32 clk_dis : 1;
  u32 ignore_all_clk_dis : 1;
  u32 ignore_all_ares : 1;
  u32 reserved1 : 6;
  u32 clk_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_ddr_i_hclk_cbcr_u
{
  struct ipa_gcc_hwio_def_gcc_ddr_i_hclk_cbcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_DDRMC_CH0_CBCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_ddrmc_ch0_cbcr_s
{
  u32 clk_enable : 1;
  u32 hw_ctl : 1;
  u32 clk_ares : 1;
  u32 sw_only_en : 1;
  u32 reserved0 : 16;
  u32 ignore_rpmh_clk_dis : 1;
  u32 reserved1 : 1;
  u32 clk_dis : 1;
  u32 ignore_all_clk_dis : 1;
  u32 ignore_all_ares : 1;
  u32 reserved2 : 6;
  u32 clk_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_ddrmc_ch0_cbcr_u
{
  struct ipa_gcc_hwio_def_gcc_ddrmc_ch0_cbcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_DDRMC_CH1_CBCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_ddrmc_ch1_cbcr_s
{
  u32 clk_enable : 1;
  u32 hw_ctl : 1;
  u32 clk_ares : 1;
  u32 sw_only_en : 1;
  u32 reserved0 : 16;
  u32 ignore_rpmh_clk_dis : 1;
  u32 reserved1 : 1;
  u32 clk_dis : 1;
  u32 ignore_all_clk_dis : 1;
  u32 ignore_all_ares : 1;
  u32 reserved2 : 6;
  u32 clk_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_ddrmc_ch1_cbcr_u
{
  struct ipa_gcc_hwio_def_gcc_ddrmc_ch1_cbcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_DDRMC_CH0_CMD_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_ddrmc_ch0_cmd_dfsr_s
{
  u32 dfs_en : 1;
  u32 curr_perf_state : 4;
  u32 hw_clk_control : 1;
  u32 dfs_fsm_state : 3;
  u32 perf_state_update_status : 1;
  u32 sw_override : 1;
  u32 sw_perf_state : 4;
  u32 rcg_sw_ctrl : 1;
  u32 reserved0 : 16;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_ddrmc_ch0_cmd_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_ddrmc_ch0_cmd_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_DDRMC_CH0_DDRMC_CH0_ROOT_PERF0_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_ddrmc_ch0_ddrmc_ch0_root_perf0_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 21;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_ddrmc_ch0_ddrmc_ch0_root_perf0_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_ddrmc_ch0_ddrmc_ch0_root_perf0_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_DDRMC_CH0_DDRMC_CH0_ROOT_PERF1_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_ddrmc_ch0_ddrmc_ch0_root_perf1_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 21;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_ddrmc_ch0_ddrmc_ch0_root_perf1_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_ddrmc_ch0_ddrmc_ch0_root_perf1_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_DDRMC_CH0_DDRMC_CH0_ROOT_PERF2_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_ddrmc_ch0_ddrmc_ch0_root_perf2_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 21;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_ddrmc_ch0_ddrmc_ch0_root_perf2_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_ddrmc_ch0_ddrmc_ch0_root_perf2_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_DDRMC_CH0_DDRMC_CH0_ROOT_PERF3_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_ddrmc_ch0_ddrmc_ch0_root_perf3_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 21;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_ddrmc_ch0_ddrmc_ch0_root_perf3_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_ddrmc_ch0_ddrmc_ch0_root_perf3_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_DDRMC_CH0_DDRMC_CH0_ROOT_PERF4_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_ddrmc_ch0_ddrmc_ch0_root_perf4_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 21;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_ddrmc_ch0_ddrmc_ch0_root_perf4_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_ddrmc_ch0_ddrmc_ch0_root_perf4_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_DDRMC_CH0_DDRMC_CH0_ROOT_PERF5_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_ddrmc_ch0_ddrmc_ch0_root_perf5_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 21;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_ddrmc_ch0_ddrmc_ch0_root_perf5_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_ddrmc_ch0_ddrmc_ch0_root_perf5_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_DDRMC_CH0_DDRMC_CH0_ROOT_PERF6_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_ddrmc_ch0_ddrmc_ch0_root_perf6_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 21;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_ddrmc_ch0_ddrmc_ch0_root_perf6_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_ddrmc_ch0_ddrmc_ch0_root_perf6_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_DDRMC_CH0_DDRMC_CH0_ROOT_PERF7_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_ddrmc_ch0_ddrmc_ch0_root_perf7_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 21;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_ddrmc_ch0_ddrmc_ch0_root_perf7_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_ddrmc_ch0_ddrmc_ch0_root_perf7_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_DDRMC_CH0_DDRMC_CH0_ROOT_PERF8_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_ddrmc_ch0_ddrmc_ch0_root_perf8_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 21;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_ddrmc_ch0_ddrmc_ch0_root_perf8_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_ddrmc_ch0_ddrmc_ch0_root_perf8_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_DDRMC_CH0_DDRMC_CH0_ROOT_PERF9_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_ddrmc_ch0_ddrmc_ch0_root_perf9_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 21;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_ddrmc_ch0_ddrmc_ch0_root_perf9_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_ddrmc_ch0_ddrmc_ch0_root_perf9_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_DDRMC_CH0_DDRMC_CH0_ROOT_PERF10_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_ddrmc_ch0_ddrmc_ch0_root_perf10_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 21;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_ddrmc_ch0_ddrmc_ch0_root_perf10_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_ddrmc_ch0_ddrmc_ch0_root_perf10_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_DDRMC_CH0_DDRMC_CH0_ROOT_PERF11_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_ddrmc_ch0_ddrmc_ch0_root_perf11_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 21;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_ddrmc_ch0_ddrmc_ch0_root_perf11_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_ddrmc_ch0_ddrmc_ch0_root_perf11_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_DDRMC_CH0_DDRMC_CH0_ROOT_PERF12_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_ddrmc_ch0_ddrmc_ch0_root_perf12_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 21;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_ddrmc_ch0_ddrmc_ch0_root_perf12_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_ddrmc_ch0_ddrmc_ch0_root_perf12_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_DDRMC_CH0_DDRMC_CH0_ROOT_PERF13_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_ddrmc_ch0_ddrmc_ch0_root_perf13_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 21;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_ddrmc_ch0_ddrmc_ch0_root_perf13_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_ddrmc_ch0_ddrmc_ch0_root_perf13_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_DDRMC_CH0_DDRMC_CH0_ROOT_PERF14_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_ddrmc_ch0_ddrmc_ch0_root_perf14_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 21;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_ddrmc_ch0_ddrmc_ch0_root_perf14_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_ddrmc_ch0_ddrmc_ch0_root_perf14_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_DDRMC_CH0_DDRMC_CH0_ROOT_PERF15_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_ddrmc_ch0_ddrmc_ch0_root_perf15_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 21;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_ddrmc_ch0_ddrmc_ch0_root_perf15_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_ddrmc_ch0_ddrmc_ch0_root_perf15_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_DDRMC_CH0_ROOT_CMD_RCGR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_ddrmc_ch0_root_cmd_rcgr_s
{
  u32 update : 1;
  u32 root_en : 1;
  u32 reserved0 : 2;
  u32 dirty_cfg_rcgr : 1;
  u32 reserved1 : 26;
  u32 root_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_ddrmc_ch0_root_cmd_rcgr_u
{
  struct ipa_gcc_hwio_def_gcc_ddrmc_ch0_root_cmd_rcgr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_DDRMC_CH0_ROOT_CFG_RCGR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_ddrmc_ch0_root_cfg_rcgr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 5;
  u32 rcglite_disable : 1;
  u32 reserved2 : 3;
  u32 hw_clk_control : 1;
  u32 reserved3 : 11;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_ddrmc_ch0_root_cfg_rcgr_u
{
  struct ipa_gcc_hwio_def_gcc_ddrmc_ch0_root_cfg_rcgr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_DDRMC_CH0_ROOT_DCD_CDIV_DCDR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_ddrmc_ch0_root_dcd_cdiv_dcdr_s
{
  u32 dcd_enable : 1;
  u32 reserved0 : 31;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_ddrmc_ch0_root_dcd_cdiv_dcdr_u
{
  struct ipa_gcc_hwio_def_gcc_ddrmc_ch0_root_dcd_cdiv_dcdr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_DDRMC_CH1_CMD_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_ddrmc_ch1_cmd_dfsr_s
{
  u32 dfs_en : 1;
  u32 curr_perf_state : 4;
  u32 hw_clk_control : 1;
  u32 dfs_fsm_state : 3;
  u32 perf_state_update_status : 1;
  u32 sw_override : 1;
  u32 sw_perf_state : 4;
  u32 rcg_sw_ctrl : 1;
  u32 reserved0 : 16;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_ddrmc_ch1_cmd_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_ddrmc_ch1_cmd_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_DDRMC_CH1_DDRMC_CH1_ROOT_PERF0_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_ddrmc_ch1_ddrmc_ch1_root_perf0_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 21;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_ddrmc_ch1_ddrmc_ch1_root_perf0_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_ddrmc_ch1_ddrmc_ch1_root_perf0_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_DDRMC_CH1_DDRMC_CH1_ROOT_PERF1_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_ddrmc_ch1_ddrmc_ch1_root_perf1_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 21;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_ddrmc_ch1_ddrmc_ch1_root_perf1_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_ddrmc_ch1_ddrmc_ch1_root_perf1_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_DDRMC_CH1_DDRMC_CH1_ROOT_PERF2_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_ddrmc_ch1_ddrmc_ch1_root_perf2_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 21;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_ddrmc_ch1_ddrmc_ch1_root_perf2_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_ddrmc_ch1_ddrmc_ch1_root_perf2_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_DDRMC_CH1_DDRMC_CH1_ROOT_PERF3_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_ddrmc_ch1_ddrmc_ch1_root_perf3_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 21;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_ddrmc_ch1_ddrmc_ch1_root_perf3_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_ddrmc_ch1_ddrmc_ch1_root_perf3_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_DDRMC_CH1_DDRMC_CH1_ROOT_PERF4_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_ddrmc_ch1_ddrmc_ch1_root_perf4_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 21;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_ddrmc_ch1_ddrmc_ch1_root_perf4_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_ddrmc_ch1_ddrmc_ch1_root_perf4_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_DDRMC_CH1_DDRMC_CH1_ROOT_PERF5_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_ddrmc_ch1_ddrmc_ch1_root_perf5_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 21;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_ddrmc_ch1_ddrmc_ch1_root_perf5_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_ddrmc_ch1_ddrmc_ch1_root_perf5_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_DDRMC_CH1_DDRMC_CH1_ROOT_PERF6_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_ddrmc_ch1_ddrmc_ch1_root_perf6_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 21;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_ddrmc_ch1_ddrmc_ch1_root_perf6_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_ddrmc_ch1_ddrmc_ch1_root_perf6_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_DDRMC_CH1_DDRMC_CH1_ROOT_PERF7_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_ddrmc_ch1_ddrmc_ch1_root_perf7_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 21;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_ddrmc_ch1_ddrmc_ch1_root_perf7_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_ddrmc_ch1_ddrmc_ch1_root_perf7_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_DDRMC_CH1_DDRMC_CH1_ROOT_PERF8_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_ddrmc_ch1_ddrmc_ch1_root_perf8_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 21;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_ddrmc_ch1_ddrmc_ch1_root_perf8_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_ddrmc_ch1_ddrmc_ch1_root_perf8_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_DDRMC_CH1_DDRMC_CH1_ROOT_PERF9_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_ddrmc_ch1_ddrmc_ch1_root_perf9_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 21;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_ddrmc_ch1_ddrmc_ch1_root_perf9_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_ddrmc_ch1_ddrmc_ch1_root_perf9_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_DDRMC_CH1_DDRMC_CH1_ROOT_PERF10_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_ddrmc_ch1_ddrmc_ch1_root_perf10_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 21;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_ddrmc_ch1_ddrmc_ch1_root_perf10_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_ddrmc_ch1_ddrmc_ch1_root_perf10_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_DDRMC_CH1_DDRMC_CH1_ROOT_PERF11_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_ddrmc_ch1_ddrmc_ch1_root_perf11_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 21;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_ddrmc_ch1_ddrmc_ch1_root_perf11_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_ddrmc_ch1_ddrmc_ch1_root_perf11_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_DDRMC_CH1_DDRMC_CH1_ROOT_PERF12_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_ddrmc_ch1_ddrmc_ch1_root_perf12_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 21;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_ddrmc_ch1_ddrmc_ch1_root_perf12_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_ddrmc_ch1_ddrmc_ch1_root_perf12_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_DDRMC_CH1_DDRMC_CH1_ROOT_PERF13_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_ddrmc_ch1_ddrmc_ch1_root_perf13_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 21;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_ddrmc_ch1_ddrmc_ch1_root_perf13_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_ddrmc_ch1_ddrmc_ch1_root_perf13_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_DDRMC_CH1_DDRMC_CH1_ROOT_PERF14_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_ddrmc_ch1_ddrmc_ch1_root_perf14_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 21;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_ddrmc_ch1_ddrmc_ch1_root_perf14_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_ddrmc_ch1_ddrmc_ch1_root_perf14_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_DDRMC_CH1_DDRMC_CH1_ROOT_PERF15_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_ddrmc_ch1_ddrmc_ch1_root_perf15_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 21;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_ddrmc_ch1_ddrmc_ch1_root_perf15_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_ddrmc_ch1_ddrmc_ch1_root_perf15_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_DDRMC_CH1_ROOT_CMD_RCGR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_ddrmc_ch1_root_cmd_rcgr_s
{
  u32 update : 1;
  u32 root_en : 1;
  u32 reserved0 : 2;
  u32 dirty_cfg_rcgr : 1;
  u32 reserved1 : 26;
  u32 root_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_ddrmc_ch1_root_cmd_rcgr_u
{
  struct ipa_gcc_hwio_def_gcc_ddrmc_ch1_root_cmd_rcgr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_DDRMC_CH1_ROOT_CFG_RCGR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_ddrmc_ch1_root_cfg_rcgr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 5;
  u32 rcglite_disable : 1;
  u32 reserved2 : 3;
  u32 hw_clk_control : 1;
  u32 reserved3 : 11;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_ddrmc_ch1_root_cfg_rcgr_u
{
  struct ipa_gcc_hwio_def_gcc_ddrmc_ch1_root_cfg_rcgr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_DDRMC_CH1_ROOT_DCD_CDIV_DCDR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_ddrmc_ch1_root_dcd_cdiv_dcdr_s
{
  u32 dcd_enable : 1;
  u32 reserved0 : 31;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_ddrmc_ch1_root_dcd_cdiv_dcdr_u
{
  struct ipa_gcc_hwio_def_gcc_ddrmc_ch1_root_dcd_cdiv_dcdr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_CPUSS_AHB_CBCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_cpuss_ahb_cbcr_s
{
  u32 reserved0 : 1;
  u32 hw_ctl : 1;
  u32 clk_ares : 1;
  u32 sw_only_en : 1;
  u32 reserved1 : 18;
  u32 clk_dis : 1;
  u32 ignore_all_clk_dis : 1;
  u32 ignore_all_ares : 1;
  u32 reserved2 : 6;
  u32 clk_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_cpuss_ahb_cbcr_u
{
  struct ipa_gcc_hwio_def_gcc_cpuss_ahb_cbcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_CPUSS_GNOC_CBCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_cpuss_gnoc_cbcr_s
{
  u32 reserved0 : 1;
  u32 hw_ctl : 1;
  u32 clk_ares : 1;
  u32 sw_only_en : 1;
  u32 reserved1 : 16;
  u32 ignore_rpmh_clk_dis : 1;
  u32 reserved2 : 1;
  u32 clk_dis : 1;
  u32 ignore_all_clk_dis : 1;
  u32 ignore_all_ares : 1;
  u32 reserved3 : 6;
  u32 clk_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_cpuss_gnoc_cbcr_u
{
  struct ipa_gcc_hwio_def_gcc_cpuss_gnoc_cbcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_CPUSS_AT_CBCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_cpuss_at_cbcr_s
{
  u32 clk_enable : 1;
  u32 hw_ctl : 1;
  u32 clk_ares : 1;
  u32 sw_only_en : 1;
  u32 reserved0 : 16;
  u32 ignore_rpmh_clk_dis : 1;
  u32 reserved1 : 1;
  u32 clk_dis : 1;
  u32 ignore_all_clk_dis : 1;
  u32 ignore_all_ares : 1;
  u32 reserved2 : 6;
  u32 clk_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_cpuss_at_cbcr_u
{
  struct ipa_gcc_hwio_def_gcc_cpuss_at_cbcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_CPUSS_AHB_CMD_RCGR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_cpuss_ahb_cmd_rcgr_s
{
  u32 update : 1;
  u32 root_en : 1;
  u32 reserved0 : 2;
  u32 dirty_cfg_rcgr : 1;
  u32 reserved1 : 26;
  u32 root_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_cpuss_ahb_cmd_rcgr_u
{
  struct ipa_gcc_hwio_def_gcc_cpuss_ahb_cmd_rcgr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_CPUSS_AHB_CFG_RCGR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_cpuss_ahb_cfg_rcgr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 5;
  u32 rcglite_disable : 1;
  u32 reserved2 : 3;
  u32 hw_clk_control : 1;
  u32 reserved3 : 11;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_cpuss_ahb_cfg_rcgr_u
{
  struct ipa_gcc_hwio_def_gcc_cpuss_ahb_cfg_rcgr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_CPUSS_AHB_POSTDIV_CDIVR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_cpuss_ahb_postdiv_cdivr_s
{
  u32 clk_div : 4;
  u32 reserved0 : 28;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_cpuss_ahb_postdiv_cdivr_u
{
  struct ipa_gcc_hwio_def_gcc_cpuss_ahb_postdiv_cdivr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_CPUSS_GPLL0_CMD_RCGR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_cpuss_gpll0_cmd_rcgr_s
{
  u32 update : 1;
  u32 root_en : 1;
  u32 reserved0 : 2;
  u32 dirty_cfg_rcgr : 1;
  u32 reserved1 : 26;
  u32 root_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_cpuss_gpll0_cmd_rcgr_u
{
  struct ipa_gcc_hwio_def_gcc_cpuss_gpll0_cmd_rcgr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_CPUSS_GPLL0_CFG_RCGR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_cpuss_gpll0_cfg_rcgr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 5;
  u32 rcglite_disable : 1;
  u32 reserved2 : 3;
  u32 hw_clk_control : 1;
  u32 reserved3 : 11;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_cpuss_gpll0_cfg_rcgr_u
{
  struct ipa_gcc_hwio_def_gcc_cpuss_gpll0_cfg_rcgr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_APSS_QDSS_TSCTR_CBCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_apss_qdss_tsctr_cbcr_s
{
  u32 clk_enable : 1;
  u32 hw_ctl : 1;
  u32 clk_ares : 1;
  u32 sw_only_en : 1;
  u32 reserved0 : 18;
  u32 clk_dis : 1;
  u32 ignore_all_clk_dis : 1;
  u32 ignore_all_ares : 1;
  u32 reserved1 : 6;
  u32 clk_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_apss_qdss_tsctr_cbcr_u
{
  struct ipa_gcc_hwio_def_gcc_apss_qdss_tsctr_cbcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_APSS_QDSS_APB_CBCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_apss_qdss_apb_cbcr_s
{
  u32 clk_enable : 1;
  u32 hw_ctl : 1;
  u32 clk_ares : 1;
  u32 sw_only_en : 1;
  u32 reserved0 : 18;
  u32 clk_dis : 1;
  u32 ignore_all_clk_dis : 1;
  u32 ignore_all_ares : 1;
  u32 reserved1 : 6;
  u32 clk_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_apss_qdss_apb_cbcr_u
{
  struct ipa_gcc_hwio_def_gcc_apss_qdss_apb_cbcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_NOC_BUS_TIMEOUT_EXTREF_BCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_noc_bus_timeout_extref_bcr_s
{
  u32 blk_ares : 1;
  u32 reserved0 : 31;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_noc_bus_timeout_extref_bcr_u
{
  struct ipa_gcc_hwio_def_gcc_noc_bus_timeout_extref_bcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_NOC_BUS_TIMEOUT_EXTREF_CBCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_noc_bus_timeout_extref_cbcr_s
{
  u32 clk_enable : 1;
  u32 reserved0 : 1;
  u32 clk_ares : 1;
  u32 reserved1 : 19;
  u32 clk_dis : 1;
  u32 ignore_all_clk_dis : 1;
  u32 ignore_all_ares : 1;
  u32 reserved2 : 6;
  u32 clk_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_noc_bus_timeout_extref_cbcr_u
{
  struct ipa_gcc_hwio_def_gcc_noc_bus_timeout_extref_cbcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_NOC_BUS_TIMEOUT_EXTREF_CDIVR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_noc_bus_timeout_extref_cdivr_s
{
  u32 clk_div : 4;
  u32 reserved0 : 28;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_noc_bus_timeout_extref_cdivr_u
{
  struct ipa_gcc_hwio_def_gcc_noc_bus_timeout_extref_cdivr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_NOC_BUS_TIMEOUT_EXTREF_DIV1024_CDIVR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_noc_bus_timeout_extref_div1024_cdivr_s
{
  u32 clk_div : 9;
  u32 reserved0 : 23;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_noc_bus_timeout_extref_div1024_cdivr_u
{
  struct ipa_gcc_hwio_def_gcc_noc_bus_timeout_extref_div1024_cdivr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_APB2JTAG_BCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_apb2jtag_bcr_s
{
  u32 blk_ares : 1;
  u32 reserved0 : 31;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_apb2jtag_bcr_u
{
  struct ipa_gcc_hwio_def_gcc_apb2jtag_bcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RBCPR_CX_BCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rbcpr_cx_bcr_s
{
  u32 blk_ares : 1;
  u32 reserved0 : 31;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rbcpr_cx_bcr_u
{
  struct ipa_gcc_hwio_def_gcc_rbcpr_cx_bcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RBCPR_CX_CBCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rbcpr_cx_cbcr_s
{
  u32 clk_enable : 1;
  u32 reserved0 : 1;
  u32 clk_ares : 1;
  u32 reserved1 : 19;
  u32 clk_dis : 1;
  u32 ignore_all_clk_dis : 1;
  u32 ignore_all_ares : 1;
  u32 reserved2 : 6;
  u32 clk_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rbcpr_cx_cbcr_u
{
  struct ipa_gcc_hwio_def_gcc_rbcpr_cx_cbcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RBCPR_CX_AHB_CBCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rbcpr_cx_ahb_cbcr_s
{
  u32 clk_enable : 1;
  u32 reserved0 : 1;
  u32 clk_ares : 1;
  u32 reserved1 : 17;
  u32 ignore_rpmh_clk_dis : 1;
  u32 reserved2 : 1;
  u32 clk_dis : 1;
  u32 ignore_all_clk_dis : 1;
  u32 ignore_all_ares : 1;
  u32 reserved3 : 6;
  u32 clk_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rbcpr_cx_ahb_cbcr_u
{
  struct ipa_gcc_hwio_def_gcc_rbcpr_cx_ahb_cbcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RBCPR_CX_CMD_RCGR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rbcpr_cx_cmd_rcgr_s
{
  u32 update : 1;
  u32 root_en : 1;
  u32 reserved0 : 2;
  u32 dirty_cfg_rcgr : 1;
  u32 reserved1 : 26;
  u32 root_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rbcpr_cx_cmd_rcgr_u
{
  struct ipa_gcc_hwio_def_gcc_rbcpr_cx_cmd_rcgr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RBCPR_CX_CFG_RCGR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rbcpr_cx_cfg_rcgr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 5;
  u32 rcglite_disable : 1;
  u32 reserved2 : 3;
  u32 hw_clk_control : 1;
  u32 reserved3 : 11;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rbcpr_cx_cfg_rcgr_u
{
  struct ipa_gcc_hwio_def_gcc_rbcpr_cx_cfg_rcgr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RBCPR_MX_BCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rbcpr_mx_bcr_s
{
  u32 blk_ares : 1;
  u32 reserved0 : 31;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rbcpr_mx_bcr_u
{
  struct ipa_gcc_hwio_def_gcc_rbcpr_mx_bcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RBCPR_MX_CBCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rbcpr_mx_cbcr_s
{
  u32 clk_enable : 1;
  u32 reserved0 : 1;
  u32 clk_ares : 1;
  u32 reserved1 : 19;
  u32 clk_dis : 1;
  u32 ignore_all_clk_dis : 1;
  u32 ignore_all_ares : 1;
  u32 reserved2 : 6;
  u32 clk_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rbcpr_mx_cbcr_u
{
  struct ipa_gcc_hwio_def_gcc_rbcpr_mx_cbcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RBCPR_MX_AHB_CBCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rbcpr_mx_ahb_cbcr_s
{
  u32 clk_enable : 1;
  u32 reserved0 : 1;
  u32 clk_ares : 1;
  u32 reserved1 : 17;
  u32 ignore_rpmh_clk_dis : 1;
  u32 reserved2 : 1;
  u32 clk_dis : 1;
  u32 ignore_all_clk_dis : 1;
  u32 ignore_all_ares : 1;
  u32 reserved3 : 6;
  u32 clk_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rbcpr_mx_ahb_cbcr_u
{
  struct ipa_gcc_hwio_def_gcc_rbcpr_mx_ahb_cbcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RBCPR_MX_CMD_RCGR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rbcpr_mx_cmd_rcgr_s
{
  u32 update : 1;
  u32 root_en : 1;
  u32 reserved0 : 2;
  u32 dirty_cfg_rcgr : 1;
  u32 reserved1 : 26;
  u32 root_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rbcpr_mx_cmd_rcgr_u
{
  struct ipa_gcc_hwio_def_gcc_rbcpr_mx_cmd_rcgr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RBCPR_MX_CFG_RCGR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rbcpr_mx_cfg_rcgr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 5;
  u32 rcglite_disable : 1;
  u32 reserved2 : 3;
  u32 hw_clk_control : 1;
  u32 reserved3 : 11;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rbcpr_mx_cfg_rcgr_u
{
  struct ipa_gcc_hwio_def_gcc_rbcpr_mx_cfg_rcgr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RBCPR_MXC_BCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rbcpr_mxc_bcr_s
{
  u32 blk_ares : 1;
  u32 reserved0 : 31;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rbcpr_mxc_bcr_u
{
  struct ipa_gcc_hwio_def_gcc_rbcpr_mxc_bcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RBCPR_MXC_CBCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rbcpr_mxc_cbcr_s
{
  u32 clk_enable : 1;
  u32 reserved0 : 1;
  u32 clk_ares : 1;
  u32 reserved1 : 19;
  u32 clk_dis : 1;
  u32 ignore_all_clk_dis : 1;
  u32 ignore_all_ares : 1;
  u32 reserved2 : 6;
  u32 clk_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rbcpr_mxc_cbcr_u
{
  struct ipa_gcc_hwio_def_gcc_rbcpr_mxc_cbcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RBCPR_MXC_AHB_CBCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rbcpr_mxc_ahb_cbcr_s
{
  u32 clk_enable : 1;
  u32 reserved0 : 1;
  u32 clk_ares : 1;
  u32 reserved1 : 17;
  u32 ignore_rpmh_clk_dis : 1;
  u32 reserved2 : 1;
  u32 clk_dis : 1;
  u32 ignore_all_clk_dis : 1;
  u32 ignore_all_ares : 1;
  u32 reserved3 : 6;
  u32 clk_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rbcpr_mxc_ahb_cbcr_u
{
  struct ipa_gcc_hwio_def_gcc_rbcpr_mxc_ahb_cbcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RBCPR_MXC_CMD_RCGR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rbcpr_mxc_cmd_rcgr_s
{
  u32 update : 1;
  u32 root_en : 1;
  u32 reserved0 : 2;
  u32 dirty_cfg_rcgr : 1;
  u32 reserved1 : 26;
  u32 root_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rbcpr_mxc_cmd_rcgr_u
{
  struct ipa_gcc_hwio_def_gcc_rbcpr_mxc_cmd_rcgr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RBCPR_MXC_CFG_RCGR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rbcpr_mxc_cfg_rcgr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 5;
  u32 rcglite_disable : 1;
  u32 reserved2 : 3;
  u32 hw_clk_control : 1;
  u32 reserved3 : 11;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rbcpr_mxc_cfg_rcgr_u
{
  struct ipa_gcc_hwio_def_gcc_rbcpr_mxc_cfg_rcgr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_DEBUG_DIV_CDIVR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_debug_div_cdivr_s
{
  u32 clk_div : 4;
  u32 reserved0 : 28;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_debug_div_cdivr_u
{
  struct ipa_gcc_hwio_def_gcc_debug_div_cdivr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_DEBUG_CBCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_debug_cbcr_s
{
  u32 clk_enable : 1;
  u32 reserved0 : 1;
  u32 clk_ares : 1;
  u32 reserved1 : 19;
  u32 clk_dis : 1;
  u32 ignore_all_clk_dis : 1;
  u32 ignore_all_ares : 1;
  u32 reserved2 : 6;
  u32 clk_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_debug_cbcr_u
{
  struct ipa_gcc_hwio_def_gcc_debug_cbcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_GP1_CBCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_gp1_cbcr_s
{
  u32 clk_enable : 1;
  u32 reserved0 : 1;
  u32 clk_ares : 1;
  u32 reserved1 : 19;
  u32 clk_dis : 1;
  u32 ignore_all_clk_dis : 1;
  u32 ignore_all_ares : 1;
  u32 reserved2 : 6;
  u32 clk_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_gp1_cbcr_u
{
  struct ipa_gcc_hwio_def_gcc_gp1_cbcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_GP1_CMD_RCGR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_gp1_cmd_rcgr_s
{
  u32 update : 1;
  u32 root_en : 1;
  u32 reserved0 : 2;
  u32 dirty_cfg_rcgr : 1;
  u32 dirty_m : 1;
  u32 dirty_n : 1;
  u32 dirty_d : 1;
  u32 reserved1 : 23;
  u32 root_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_gp1_cmd_rcgr_u
{
  struct ipa_gcc_hwio_def_gcc_gp1_cmd_rcgr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_GP1_CFG_RCGR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_gp1_cfg_rcgr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 1;
  u32 mode : 2;
  u32 reserved2 : 6;
  u32 hw_clk_control : 1;
  u32 reserved3 : 11;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_gp1_cfg_rcgr_u
{
  struct ipa_gcc_hwio_def_gcc_gp1_cfg_rcgr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_GP1_M
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_gp1_m_s
{
  u32 m : 16;
  u32 reserved0 : 16;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_gp1_m_u
{
  struct ipa_gcc_hwio_def_gcc_gp1_m_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_GP1_N
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_gp1_n_s
{
  u32 not_n_minus_m : 16;
  u32 reserved0 : 16;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_gp1_n_u
{
  struct ipa_gcc_hwio_def_gcc_gp1_n_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_GP1_D
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_gp1_d_s
{
  u32 not_2d : 16;
  u32 reserved0 : 16;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_gp1_d_u
{
  struct ipa_gcc_hwio_def_gcc_gp1_d_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_GP2_CBCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_gp2_cbcr_s
{
  u32 clk_enable : 1;
  u32 reserved0 : 1;
  u32 clk_ares : 1;
  u32 reserved1 : 19;
  u32 clk_dis : 1;
  u32 ignore_all_clk_dis : 1;
  u32 ignore_all_ares : 1;
  u32 reserved2 : 6;
  u32 clk_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_gp2_cbcr_u
{
  struct ipa_gcc_hwio_def_gcc_gp2_cbcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_GP2_CMD_RCGR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_gp2_cmd_rcgr_s
{
  u32 update : 1;
  u32 root_en : 1;
  u32 reserved0 : 2;
  u32 dirty_cfg_rcgr : 1;
  u32 dirty_m : 1;
  u32 dirty_n : 1;
  u32 dirty_d : 1;
  u32 reserved1 : 23;
  u32 root_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_gp2_cmd_rcgr_u
{
  struct ipa_gcc_hwio_def_gcc_gp2_cmd_rcgr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_GP2_CFG_RCGR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_gp2_cfg_rcgr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 1;
  u32 mode : 2;
  u32 reserved2 : 6;
  u32 hw_clk_control : 1;
  u32 reserved3 : 11;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_gp2_cfg_rcgr_u
{
  struct ipa_gcc_hwio_def_gcc_gp2_cfg_rcgr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_GP2_M
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_gp2_m_s
{
  u32 m : 16;
  u32 reserved0 : 16;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_gp2_m_u
{
  struct ipa_gcc_hwio_def_gcc_gp2_m_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_GP2_N
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_gp2_n_s
{
  u32 not_n_minus_m : 16;
  u32 reserved0 : 16;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_gp2_n_u
{
  struct ipa_gcc_hwio_def_gcc_gp2_n_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_GP2_D
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_gp2_d_s
{
  u32 not_2d : 16;
  u32 reserved0 : 16;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_gp2_d_u
{
  struct ipa_gcc_hwio_def_gcc_gp2_d_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_GP3_CBCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_gp3_cbcr_s
{
  u32 clk_enable : 1;
  u32 reserved0 : 1;
  u32 clk_ares : 1;
  u32 reserved1 : 19;
  u32 clk_dis : 1;
  u32 ignore_all_clk_dis : 1;
  u32 ignore_all_ares : 1;
  u32 reserved2 : 6;
  u32 clk_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_gp3_cbcr_u
{
  struct ipa_gcc_hwio_def_gcc_gp3_cbcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_GP3_CMD_RCGR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_gp3_cmd_rcgr_s
{
  u32 update : 1;
  u32 root_en : 1;
  u32 reserved0 : 2;
  u32 dirty_cfg_rcgr : 1;
  u32 dirty_m : 1;
  u32 dirty_n : 1;
  u32 dirty_d : 1;
  u32 reserved1 : 23;
  u32 root_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_gp3_cmd_rcgr_u
{
  struct ipa_gcc_hwio_def_gcc_gp3_cmd_rcgr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_GP3_CFG_RCGR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_gp3_cfg_rcgr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 1;
  u32 mode : 2;
  u32 reserved2 : 6;
  u32 hw_clk_control : 1;
  u32 reserved3 : 11;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_gp3_cfg_rcgr_u
{
  struct ipa_gcc_hwio_def_gcc_gp3_cfg_rcgr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_GP3_M
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_gp3_m_s
{
  u32 m : 16;
  u32 reserved0 : 16;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_gp3_m_u
{
  struct ipa_gcc_hwio_def_gcc_gp3_m_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_GP3_N
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_gp3_n_s
{
  u32 not_n_minus_m : 16;
  u32 reserved0 : 16;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_gp3_n_u
{
  struct ipa_gcc_hwio_def_gcc_gp3_n_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_GP3_D
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_gp3_d_s
{
  u32 not_2d : 16;
  u32 reserved0 : 16;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_gp3_d_u
{
  struct ipa_gcc_hwio_def_gcc_gp3_d_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_AUDIO_CORE_BCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_audio_core_bcr_s
{
  u32 blk_ares : 1;
  u32 reserved0 : 31;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_audio_core_bcr_u
{
  struct ipa_gcc_hwio_def_gcc_audio_core_bcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_ULTAUDIO_PCNOC_MPORT_CBCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_ultaudio_pcnoc_mport_cbcr_s
{
  u32 clk_enable : 1;
  u32 reserved0 : 1;
  u32 clk_ares : 1;
  u32 reserved1 : 17;
  u32 ignore_rpmh_clk_dis : 1;
  u32 reserved2 : 1;
  u32 clk_dis : 1;
  u32 ignore_all_clk_dis : 1;
  u32 ignore_all_ares : 1;
  u32 reserved3 : 6;
  u32 clk_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_ultaudio_pcnoc_mport_cbcr_u
{
  struct ipa_gcc_hwio_def_gcc_ultaudio_pcnoc_mport_cbcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_ULTAUDIO_PCNOC_SWAY_CBCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_ultaudio_pcnoc_sway_cbcr_s
{
  u32 reserved0 : 1;
  u32 hw_ctl : 1;
  u32 clk_ares : 1;
  u32 sw_only_en : 1;
  u32 reserved1 : 16;
  u32 ignore_rpmh_clk_dis : 1;
  u32 reserved2 : 1;
  u32 clk_dis : 1;
  u32 ignore_all_clk_dis : 1;
  u32 ignore_all_ares : 1;
  u32 reserved3 : 6;
  u32 clk_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_ultaudio_pcnoc_sway_cbcr_u
{
  struct ipa_gcc_hwio_def_gcc_ultaudio_pcnoc_sway_cbcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_AUDIO_AHB_BUS_BCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_audio_ahb_bus_bcr_s
{
  u32 blk_ares : 1;
  u32 reserved0 : 31;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_audio_ahb_bus_bcr_u
{
  struct ipa_gcc_hwio_def_gcc_audio_ahb_bus_bcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_ULTAUDIO_AHBFABRIC_IXFABRIC_CBCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_ultaudio_ahbfabric_ixfabric_cbcr_s
{
  u32 reserved0 : 2;
  u32 clk_ares : 1;
  u32 reserved1 : 19;
  u32 clk_dis : 1;
  u32 ignore_all_clk_dis : 1;
  u32 ignore_all_ares : 1;
  u32 reserved2 : 6;
  u32 clk_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_ultaudio_ahbfabric_ixfabric_cbcr_u
{
  struct ipa_gcc_hwio_def_gcc_ultaudio_ahbfabric_ixfabric_cbcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_ULTAUDIO_AHBFABRIC_IXFABRIC_LPM_CBCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_ultaudio_ahbfabric_ixfabric_lpm_cbcr_s
{
  u32 clk_enable : 1;
  u32 reserved0 : 1;
  u32 clk_ares : 1;
  u32 reserved1 : 1;
  u32 sleep : 4;
  u32 wakeup : 4;
  u32 force_mem_periph_off : 1;
  u32 force_mem_periph_on : 1;
  u32 force_mem_core_on : 1;
  u32 reserved2 : 7;
  u32 clk_dis : 1;
  u32 ignore_all_clk_dis : 1;
  u32 ignore_all_ares : 1;
  u32 reserved3 : 6;
  u32 clk_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_ultaudio_ahbfabric_ixfabric_lpm_cbcr_u
{
  struct ipa_gcc_hwio_def_gcc_ultaudio_ahbfabric_ixfabric_lpm_cbcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_ULTAUDIO_AHBFABRIC_IXFABRIC_LPM_SREGR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_ultaudio_ahbfabric_ixfabric_lpm_sregr_s
{
  u32 reserved0 : 1;
  u32 sw_clk_en_slp_stg : 1;
  u32 sw_clk_en_sel_slp_stg : 1;
  u32 sw_ctrl_pwr_down : 1;
  u32 sw_rst_slp_stg : 1;
  u32 sw_rst_sel_slp_stg : 1;
  u32 force_clk_on : 1;
  u32 mem_cph_enable : 1;
  u32 sw_div_ratio_slp_stg_clk : 2;
  u32 mem_periph_on_ack : 1;
  u32 mem_core_on_ack : 1;
  u32 sw_sm_pscbc_seq_in_override : 1;
  u32 mem_cph_rst_sw_override : 1;
  u32 pscbc_slp_stg_mode_csr : 1;
  u32 ignore_gdsc_pwr_dwn_csr : 1;
  u32 sreg_pscbc_spare_ctrl_in : 8;
  u32 sreg_pscbc_spare_ctrl_out : 8;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_ultaudio_ahbfabric_ixfabric_lpm_sregr_u
{
  struct ipa_gcc_hwio_def_gcc_ultaudio_ahbfabric_ixfabric_lpm_sregr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_ULTAUDIO_LPAIF_SLIMBUS_BAM_CBCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_ultaudio_lpaif_slimbus_bam_cbcr_s
{
  u32 clk_enable : 1;
  u32 reserved0 : 1;
  u32 clk_ares : 1;
  u32 reserved1 : 1;
  u32 sleep : 4;
  u32 wakeup : 4;
  u32 force_mem_periph_off : 1;
  u32 force_mem_periph_on : 1;
  u32 force_mem_core_on : 1;
  u32 reserved2 : 7;
  u32 clk_dis : 1;
  u32 ignore_all_clk_dis : 1;
  u32 ignore_all_ares : 1;
  u32 reserved3 : 6;
  u32 clk_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_ultaudio_lpaif_slimbus_bam_cbcr_u
{
  struct ipa_gcc_hwio_def_gcc_ultaudio_lpaif_slimbus_bam_cbcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_ULTAUDIO_LPAIF_SLIMBUS_BAM_SREGR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_ultaudio_lpaif_slimbus_bam_sregr_s
{
  u32 reserved0 : 1;
  u32 sw_clk_en_slp_stg : 1;
  u32 sw_clk_en_sel_slp_stg : 1;
  u32 sw_ctrl_pwr_down : 1;
  u32 sw_rst_slp_stg : 1;
  u32 sw_rst_sel_slp_stg : 1;
  u32 force_clk_on : 1;
  u32 mem_cph_enable : 1;
  u32 sw_div_ratio_slp_stg_clk : 2;
  u32 mem_periph_on_ack : 1;
  u32 mem_core_on_ack : 1;
  u32 sw_sm_pscbc_seq_in_override : 1;
  u32 mem_cph_rst_sw_override : 1;
  u32 pscbc_slp_stg_mode_csr : 1;
  u32 ignore_gdsc_pwr_dwn_csr : 1;
  u32 sreg_pscbc_spare_ctrl_in : 8;
  u32 sreg_pscbc_spare_ctrl_out : 8;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_ultaudio_lpaif_slimbus_bam_sregr_u
{
  struct ipa_gcc_hwio_def_gcc_ultaudio_lpaif_slimbus_bam_sregr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_ULTAUDIO_AHBFABRIC_CMD_RCGR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_ultaudio_ahbfabric_cmd_rcgr_s
{
  u32 update : 1;
  u32 root_en : 1;
  u32 reserved0 : 2;
  u32 dirty_cfg_rcgr : 1;
  u32 dirty_m : 1;
  u32 dirty_n : 1;
  u32 dirty_d : 1;
  u32 reserved1 : 23;
  u32 root_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_ultaudio_ahbfabric_cmd_rcgr_u
{
  struct ipa_gcc_hwio_def_gcc_ultaudio_ahbfabric_cmd_rcgr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_ULTAUDIO_AHBFABRIC_CFG_RCGR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_ultaudio_ahbfabric_cfg_rcgr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 1;
  u32 mode : 2;
  u32 reserved2 : 6;
  u32 hw_clk_control : 1;
  u32 reserved3 : 11;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_ultaudio_ahbfabric_cfg_rcgr_u
{
  struct ipa_gcc_hwio_def_gcc_ultaudio_ahbfabric_cfg_rcgr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_ULTAUDIO_AHBFABRIC_M
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_ultaudio_ahbfabric_m_s
{
  u32 m : 8;
  u32 reserved0 : 24;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_ultaudio_ahbfabric_m_u
{
  struct ipa_gcc_hwio_def_gcc_ultaudio_ahbfabric_m_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_ULTAUDIO_AHBFABRIC_N
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_ultaudio_ahbfabric_n_s
{
  u32 not_n_minus_m : 8;
  u32 reserved0 : 24;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_ultaudio_ahbfabric_n_u
{
  struct ipa_gcc_hwio_def_gcc_ultaudio_ahbfabric_n_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_ULTAUDIO_AHBFABRIC_D
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_ultaudio_ahbfabric_d_s
{
  u32 not_2d : 8;
  u32 reserved0 : 24;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_ultaudio_ahbfabric_d_u
{
  struct ipa_gcc_hwio_def_gcc_ultaudio_ahbfabric_d_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_ULTAUDIO_PRI_I2S_BCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_ultaudio_pri_i2s_bcr_s
{
  u32 blk_ares : 1;
  u32 reserved0 : 31;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_ultaudio_pri_i2s_bcr_u
{
  struct ipa_gcc_hwio_def_gcc_ultaudio_pri_i2s_bcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_ULTAUDIO_LPAIF_PRI_I2S_CBCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_ultaudio_lpaif_pri_i2s_cbcr_s
{
  u32 clk_enable : 1;
  u32 reserved0 : 1;
  u32 clk_ares : 1;
  u32 reserved1 : 19;
  u32 clk_dis : 1;
  u32 ignore_all_clk_dis : 1;
  u32 ignore_all_ares : 1;
  u32 reserved2 : 6;
  u32 clk_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_ultaudio_lpaif_pri_i2s_cbcr_u
{
  struct ipa_gcc_hwio_def_gcc_ultaudio_lpaif_pri_i2s_cbcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_ULTAUDIO_LPAIF_PRI_I2S_CMD_RCGR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_ultaudio_lpaif_pri_i2s_cmd_rcgr_s
{
  u32 update : 1;
  u32 root_en : 1;
  u32 reserved0 : 2;
  u32 dirty_cfg_rcgr : 1;
  u32 dirty_m : 1;
  u32 dirty_n : 1;
  u32 dirty_d : 1;
  u32 reserved1 : 23;
  u32 root_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_ultaudio_lpaif_pri_i2s_cmd_rcgr_u
{
  struct ipa_gcc_hwio_def_gcc_ultaudio_lpaif_pri_i2s_cmd_rcgr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_ULTAUDIO_LPAIF_PRI_I2S_CFG_RCGR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_ultaudio_lpaif_pri_i2s_cfg_rcgr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 1;
  u32 mode : 2;
  u32 reserved2 : 6;
  u32 hw_clk_control : 1;
  u32 reserved3 : 11;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_ultaudio_lpaif_pri_i2s_cfg_rcgr_u
{
  struct ipa_gcc_hwio_def_gcc_ultaudio_lpaif_pri_i2s_cfg_rcgr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_ULTAUDIO_LPAIF_PRI_I2S_M
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_ultaudio_lpaif_pri_i2s_m_s
{
  u32 m : 8;
  u32 reserved0 : 24;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_ultaudio_lpaif_pri_i2s_m_u
{
  struct ipa_gcc_hwio_def_gcc_ultaudio_lpaif_pri_i2s_m_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_ULTAUDIO_LPAIF_PRI_I2S_N
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_ultaudio_lpaif_pri_i2s_n_s
{
  u32 not_n_minus_m : 8;
  u32 reserved0 : 24;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_ultaudio_lpaif_pri_i2s_n_u
{
  struct ipa_gcc_hwio_def_gcc_ultaudio_lpaif_pri_i2s_n_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_ULTAUDIO_LPAIF_PRI_I2S_D
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_ultaudio_lpaif_pri_i2s_d_s
{
  u32 not_2d : 8;
  u32 reserved0 : 24;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_ultaudio_lpaif_pri_i2s_d_u
{
  struct ipa_gcc_hwio_def_gcc_ultaudio_lpaif_pri_i2s_d_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_ULTAUDIO_SEC_I2S_BCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_ultaudio_sec_i2s_bcr_s
{
  u32 blk_ares : 1;
  u32 reserved0 : 31;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_ultaudio_sec_i2s_bcr_u
{
  struct ipa_gcc_hwio_def_gcc_ultaudio_sec_i2s_bcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_ULTAUDIO_LPAIF_SEC_I2S_CBCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_ultaudio_lpaif_sec_i2s_cbcr_s
{
  u32 clk_enable : 1;
  u32 reserved0 : 1;
  u32 clk_ares : 1;
  u32 reserved1 : 19;
  u32 clk_dis : 1;
  u32 ignore_all_clk_dis : 1;
  u32 ignore_all_ares : 1;
  u32 reserved2 : 6;
  u32 clk_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_ultaudio_lpaif_sec_i2s_cbcr_u
{
  struct ipa_gcc_hwio_def_gcc_ultaudio_lpaif_sec_i2s_cbcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_ULTAUDIO_LPAIF_SEC_I2S_CMD_RCGR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_ultaudio_lpaif_sec_i2s_cmd_rcgr_s
{
  u32 update : 1;
  u32 root_en : 1;
  u32 reserved0 : 2;
  u32 dirty_cfg_rcgr : 1;
  u32 dirty_m : 1;
  u32 dirty_n : 1;
  u32 dirty_d : 1;
  u32 reserved1 : 23;
  u32 root_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_ultaudio_lpaif_sec_i2s_cmd_rcgr_u
{
  struct ipa_gcc_hwio_def_gcc_ultaudio_lpaif_sec_i2s_cmd_rcgr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_ULTAUDIO_LPAIF_SEC_I2S_CFG_RCGR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_ultaudio_lpaif_sec_i2s_cfg_rcgr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 1;
  u32 mode : 2;
  u32 reserved2 : 6;
  u32 hw_clk_control : 1;
  u32 reserved3 : 11;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_ultaudio_lpaif_sec_i2s_cfg_rcgr_u
{
  struct ipa_gcc_hwio_def_gcc_ultaudio_lpaif_sec_i2s_cfg_rcgr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_ULTAUDIO_LPAIF_SEC_I2S_M
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_ultaudio_lpaif_sec_i2s_m_s
{
  u32 m : 8;
  u32 reserved0 : 24;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_ultaudio_lpaif_sec_i2s_m_u
{
  struct ipa_gcc_hwio_def_gcc_ultaudio_lpaif_sec_i2s_m_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_ULTAUDIO_LPAIF_SEC_I2S_N
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_ultaudio_lpaif_sec_i2s_n_s
{
  u32 not_n_minus_m : 8;
  u32 reserved0 : 24;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_ultaudio_lpaif_sec_i2s_n_u
{
  struct ipa_gcc_hwio_def_gcc_ultaudio_lpaif_sec_i2s_n_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_ULTAUDIO_LPAIF_SEC_I2S_D
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_ultaudio_lpaif_sec_i2s_d_s
{
  u32 not_2d : 8;
  u32 reserved0 : 24;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_ultaudio_lpaif_sec_i2s_d_u
{
  struct ipa_gcc_hwio_def_gcc_ultaudio_lpaif_sec_i2s_d_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_ULTAUDIO_AUX_I2S_BCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_ultaudio_aux_i2s_bcr_s
{
  u32 blk_ares : 1;
  u32 reserved0 : 31;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_ultaudio_aux_i2s_bcr_u
{
  struct ipa_gcc_hwio_def_gcc_ultaudio_aux_i2s_bcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_ULTAUDIO_LPAIF_AUX_I2S_CBCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_ultaudio_lpaif_aux_i2s_cbcr_s
{
  u32 clk_enable : 1;
  u32 reserved0 : 1;
  u32 clk_ares : 1;
  u32 reserved1 : 19;
  u32 clk_dis : 1;
  u32 ignore_all_clk_dis : 1;
  u32 ignore_all_ares : 1;
  u32 reserved2 : 6;
  u32 clk_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_ultaudio_lpaif_aux_i2s_cbcr_u
{
  struct ipa_gcc_hwio_def_gcc_ultaudio_lpaif_aux_i2s_cbcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_ULTAUDIO_LPAIF_AUX_I2S_CMD_RCGR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_ultaudio_lpaif_aux_i2s_cmd_rcgr_s
{
  u32 update : 1;
  u32 root_en : 1;
  u32 reserved0 : 2;
  u32 dirty_cfg_rcgr : 1;
  u32 dirty_m : 1;
  u32 dirty_n : 1;
  u32 dirty_d : 1;
  u32 reserved1 : 23;
  u32 root_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_ultaudio_lpaif_aux_i2s_cmd_rcgr_u
{
  struct ipa_gcc_hwio_def_gcc_ultaudio_lpaif_aux_i2s_cmd_rcgr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_ULTAUDIO_LPAIF_AUX_I2S_CFG_RCGR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_ultaudio_lpaif_aux_i2s_cfg_rcgr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 1;
  u32 mode : 2;
  u32 reserved2 : 6;
  u32 hw_clk_control : 1;
  u32 reserved3 : 11;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_ultaudio_lpaif_aux_i2s_cfg_rcgr_u
{
  struct ipa_gcc_hwio_def_gcc_ultaudio_lpaif_aux_i2s_cfg_rcgr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_ULTAUDIO_LPAIF_AUX_I2S_M
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_ultaudio_lpaif_aux_i2s_m_s
{
  u32 m : 8;
  u32 reserved0 : 24;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_ultaudio_lpaif_aux_i2s_m_u
{
  struct ipa_gcc_hwio_def_gcc_ultaudio_lpaif_aux_i2s_m_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_ULTAUDIO_LPAIF_AUX_I2S_N
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_ultaudio_lpaif_aux_i2s_n_s
{
  u32 not_n_minus_m : 8;
  u32 reserved0 : 24;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_ultaudio_lpaif_aux_i2s_n_u
{
  struct ipa_gcc_hwio_def_gcc_ultaudio_lpaif_aux_i2s_n_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_ULTAUDIO_LPAIF_AUX_I2S_D
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_ultaudio_lpaif_aux_i2s_d_s
{
  u32 not_2d : 8;
  u32 reserved0 : 24;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_ultaudio_lpaif_aux_i2s_d_u
{
  struct ipa_gcc_hwio_def_gcc_ultaudio_lpaif_aux_i2s_d_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_AUDIO_CXO_BCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_audio_cxo_bcr_s
{
  u32 blk_ares : 1;
  u32 reserved0 : 31;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_audio_cxo_bcr_u
{
  struct ipa_gcc_hwio_def_gcc_audio_cxo_bcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_ULTAUDIO_AVSYNC_XO_CBCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_ultaudio_avsync_xo_cbcr_s
{
  u32 clk_enable : 1;
  u32 reserved0 : 1;
  u32 clk_ares : 1;
  u32 reserved1 : 19;
  u32 clk_dis : 1;
  u32 ignore_all_clk_dis : 1;
  u32 ignore_all_ares : 1;
  u32 reserved2 : 6;
  u32 clk_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_ultaudio_avsync_xo_cbcr_u
{
  struct ipa_gcc_hwio_def_gcc_ultaudio_avsync_xo_cbcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_ULTAUDIO_XO_CMD_RCGR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_ultaudio_xo_cmd_rcgr_s
{
  u32 update : 1;
  u32 root_en : 1;
  u32 reserved0 : 2;
  u32 dirty_cfg_rcgr : 1;
  u32 dirty_m : 1;
  u32 dirty_n : 1;
  u32 dirty_d : 1;
  u32 reserved1 : 23;
  u32 root_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_ultaudio_xo_cmd_rcgr_u
{
  struct ipa_gcc_hwio_def_gcc_ultaudio_xo_cmd_rcgr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_ULTAUDIO_XO_CFG_RCGR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_ultaudio_xo_cfg_rcgr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 1;
  u32 mode : 2;
  u32 reserved2 : 6;
  u32 hw_clk_control : 1;
  u32 reserved3 : 11;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_ultaudio_xo_cfg_rcgr_u
{
  struct ipa_gcc_hwio_def_gcc_ultaudio_xo_cfg_rcgr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_ULTAUDIO_XO_M
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_ultaudio_xo_m_s
{
  u32 m : 8;
  u32 reserved0 : 24;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_ultaudio_xo_m_u
{
  struct ipa_gcc_hwio_def_gcc_ultaudio_xo_m_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_ULTAUDIO_XO_N
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_ultaudio_xo_n_s
{
  u32 not_n_minus_m : 8;
  u32 reserved0 : 24;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_ultaudio_xo_n_u
{
  struct ipa_gcc_hwio_def_gcc_ultaudio_xo_n_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_ULTAUDIO_XO_D
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_ultaudio_xo_d_s
{
  u32 not_2d : 8;
  u32 reserved0 : 24;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_ultaudio_xo_d_u
{
  struct ipa_gcc_hwio_def_gcc_ultaudio_xo_d_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_ULTAUDIO_EXT_I2S_BCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_ultaudio_ext_i2s_bcr_s
{
  u32 blk_ares : 1;
  u32 reserved0 : 31;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_ultaudio_ext_i2s_bcr_u
{
  struct ipa_gcc_hwio_def_gcc_ultaudio_ext_i2s_bcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_ULTAUDIO_LPAIF_EXT_I2S_CBCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_ultaudio_lpaif_ext_i2s_cbcr_s
{
  u32 clk_enable : 1;
  u32 reserved0 : 1;
  u32 clk_ares : 1;
  u32 reserved1 : 19;
  u32 clk_dis : 1;
  u32 ignore_all_clk_dis : 1;
  u32 ignore_all_ares : 1;
  u32 reserved2 : 6;
  u32 clk_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_ultaudio_lpaif_ext_i2s_cbcr_u
{
  struct ipa_gcc_hwio_def_gcc_ultaudio_lpaif_ext_i2s_cbcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_ULTAUDIO_LPAIF_EXT_I2S_CMD_RCGR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_ultaudio_lpaif_ext_i2s_cmd_rcgr_s
{
  u32 update : 1;
  u32 root_en : 1;
  u32 reserved0 : 2;
  u32 dirty_cfg_rcgr : 1;
  u32 dirty_m : 1;
  u32 dirty_n : 1;
  u32 dirty_d : 1;
  u32 reserved1 : 23;
  u32 root_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_ultaudio_lpaif_ext_i2s_cmd_rcgr_u
{
  struct ipa_gcc_hwio_def_gcc_ultaudio_lpaif_ext_i2s_cmd_rcgr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_ULTAUDIO_LPAIF_EXT_I2S_CFG_RCGR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_ultaudio_lpaif_ext_i2s_cfg_rcgr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 1;
  u32 mode : 2;
  u32 reserved2 : 6;
  u32 hw_clk_control : 1;
  u32 reserved3 : 11;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_ultaudio_lpaif_ext_i2s_cfg_rcgr_u
{
  struct ipa_gcc_hwio_def_gcc_ultaudio_lpaif_ext_i2s_cfg_rcgr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_ULTAUDIO_LPAIF_EXT_I2S_M
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_ultaudio_lpaif_ext_i2s_m_s
{
  u32 m : 8;
  u32 reserved0 : 24;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_ultaudio_lpaif_ext_i2s_m_u
{
  struct ipa_gcc_hwio_def_gcc_ultaudio_lpaif_ext_i2s_m_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_ULTAUDIO_LPAIF_EXT_I2S_N
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_ultaudio_lpaif_ext_i2s_n_s
{
  u32 not_n_minus_m : 8;
  u32 reserved0 : 24;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_ultaudio_lpaif_ext_i2s_n_u
{
  struct ipa_gcc_hwio_def_gcc_ultaudio_lpaif_ext_i2s_n_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_ULTAUDIO_LPAIF_EXT_I2S_D
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_ultaudio_lpaif_ext_i2s_d_s
{
  u32 not_2d : 8;
  u32 reserved0 : 24;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_ultaudio_lpaif_ext_i2s_d_u
{
  struct ipa_gcc_hwio_def_gcc_ultaudio_lpaif_ext_i2s_d_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_SLIMBUS_BCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_slimbus_bcr_s
{
  u32 blk_ares : 1;
  u32 reserved0 : 31;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_slimbus_bcr_u
{
  struct ipa_gcc_hwio_def_gcc_slimbus_bcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_ULTAUDIO_LPAIF_SLIMBUS_CORE_CBCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_ultaudio_lpaif_slimbus_core_cbcr_s
{
  u32 clk_enable : 1;
  u32 reserved0 : 1;
  u32 clk_ares : 1;
  u32 reserved1 : 19;
  u32 clk_dis : 1;
  u32 ignore_all_clk_dis : 1;
  u32 ignore_all_ares : 1;
  u32 reserved2 : 6;
  u32 clk_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_ultaudio_lpaif_slimbus_core_cbcr_u
{
  struct ipa_gcc_hwio_def_gcc_ultaudio_lpaif_slimbus_core_cbcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_ULTAUDIO_LPAIF_SLIMBUS_CORE_CMD_RCGR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_ultaudio_lpaif_slimbus_core_cmd_rcgr_s
{
  u32 update : 1;
  u32 root_en : 1;
  u32 reserved0 : 2;
  u32 dirty_cfg_rcgr : 1;
  u32 dirty_m : 1;
  u32 dirty_n : 1;
  u32 dirty_d : 1;
  u32 reserved1 : 23;
  u32 root_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_ultaudio_lpaif_slimbus_core_cmd_rcgr_u
{
  struct ipa_gcc_hwio_def_gcc_ultaudio_lpaif_slimbus_core_cmd_rcgr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_ULTAUDIO_LPAIF_SLIMBUS_CORE_CFG_RCGR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_ultaudio_lpaif_slimbus_core_cfg_rcgr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 1;
  u32 mode : 2;
  u32 reserved2 : 6;
  u32 hw_clk_control : 1;
  u32 reserved3 : 11;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_ultaudio_lpaif_slimbus_core_cfg_rcgr_u
{
  struct ipa_gcc_hwio_def_gcc_ultaudio_lpaif_slimbus_core_cfg_rcgr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_ULTAUDIO_LPAIF_SLIMBUS_CORE_M
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_ultaudio_lpaif_slimbus_core_m_s
{
  u32 m : 8;
  u32 reserved0 : 24;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_ultaudio_lpaif_slimbus_core_m_u
{
  struct ipa_gcc_hwio_def_gcc_ultaudio_lpaif_slimbus_core_m_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_ULTAUDIO_LPAIF_SLIMBUS_CORE_N
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_ultaudio_lpaif_slimbus_core_n_s
{
  u32 not_n_minus_m : 8;
  u32 reserved0 : 24;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_ultaudio_lpaif_slimbus_core_n_u
{
  struct ipa_gcc_hwio_def_gcc_ultaudio_lpaif_slimbus_core_n_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_ULTAUDIO_LPAIF_SLIMBUS_CORE_D
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_ultaudio_lpaif_slimbus_core_d_s
{
  u32 not_2d : 8;
  u32 reserved0 : 24;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_ultaudio_lpaif_slimbus_core_d_u
{
  struct ipa_gcc_hwio_def_gcc_ultaudio_lpaif_slimbus_core_d_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_ULTAUDIO_PCM_BCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_ultaudio_pcm_bcr_s
{
  u32 blk_ares : 1;
  u32 reserved0 : 31;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_ultaudio_pcm_bcr_u
{
  struct ipa_gcc_hwio_def_gcc_ultaudio_pcm_bcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_ULTAUDIO_LPAIF_PCM_DATAOE_CBCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_ultaudio_lpaif_pcm_dataoe_cbcr_s
{
  u32 clk_enable : 1;
  u32 reserved0 : 1;
  u32 clk_ares : 1;
  u32 reserved1 : 19;
  u32 clk_dis : 1;
  u32 ignore_all_clk_dis : 1;
  u32 ignore_all_ares : 1;
  u32 reserved2 : 6;
  u32 clk_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_ultaudio_lpaif_pcm_dataoe_cbcr_u
{
  struct ipa_gcc_hwio_def_gcc_ultaudio_lpaif_pcm_dataoe_cbcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_ULTAUDIO_LPAIF_PCM_DATAOE_CMD_RCGR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_ultaudio_lpaif_pcm_dataoe_cmd_rcgr_s
{
  u32 update : 1;
  u32 root_en : 1;
  u32 reserved0 : 2;
  u32 dirty_cfg_rcgr : 1;
  u32 dirty_m : 1;
  u32 dirty_n : 1;
  u32 dirty_d : 1;
  u32 reserved1 : 23;
  u32 root_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_ultaudio_lpaif_pcm_dataoe_cmd_rcgr_u
{
  struct ipa_gcc_hwio_def_gcc_ultaudio_lpaif_pcm_dataoe_cmd_rcgr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_ULTAUDIO_LPAIF_PCM_DATAOE_CFG_RCGR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_ultaudio_lpaif_pcm_dataoe_cfg_rcgr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 1;
  u32 mode : 2;
  u32 reserved2 : 6;
  u32 hw_clk_control : 1;
  u32 reserved3 : 11;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_ultaudio_lpaif_pcm_dataoe_cfg_rcgr_u
{
  struct ipa_gcc_hwio_def_gcc_ultaudio_lpaif_pcm_dataoe_cfg_rcgr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_ULTAUDIO_LPAIF_PCM_DATAOE_M
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_ultaudio_lpaif_pcm_dataoe_m_s
{
  u32 m : 8;
  u32 reserved0 : 24;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_ultaudio_lpaif_pcm_dataoe_m_u
{
  struct ipa_gcc_hwio_def_gcc_ultaudio_lpaif_pcm_dataoe_m_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_ULTAUDIO_LPAIF_PCM_DATAOE_N
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_ultaudio_lpaif_pcm_dataoe_n_s
{
  u32 not_n_minus_m : 8;
  u32 reserved0 : 24;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_ultaudio_lpaif_pcm_dataoe_n_u
{
  struct ipa_gcc_hwio_def_gcc_ultaudio_lpaif_pcm_dataoe_n_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_ULTAUDIO_LPAIF_PCM_DATAOE_D
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_ultaudio_lpaif_pcm_dataoe_d_s
{
  u32 not_2d : 8;
  u32 reserved0 : 24;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_ultaudio_lpaif_pcm_dataoe_d_u
{
  struct ipa_gcc_hwio_def_gcc_ultaudio_lpaif_pcm_dataoe_d_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_ULTAUDIO_LPAIF_AUX_PCM_DATAOE_CBCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_ultaudio_lpaif_aux_pcm_dataoe_cbcr_s
{
  u32 clk_enable : 1;
  u32 reserved0 : 1;
  u32 clk_ares : 1;
  u32 reserved1 : 19;
  u32 clk_dis : 1;
  u32 ignore_all_clk_dis : 1;
  u32 ignore_all_ares : 1;
  u32 reserved2 : 6;
  u32 clk_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_ultaudio_lpaif_aux_pcm_dataoe_cbcr_u
{
  struct ipa_gcc_hwio_def_gcc_ultaudio_lpaif_aux_pcm_dataoe_cbcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_ULTAUDIO_LPAIF_AUX_PCM_DATAOE_CMD_RCGR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_ultaudio_lpaif_aux_pcm_dataoe_cmd_rcgr_s
{
  u32 update : 1;
  u32 root_en : 1;
  u32 reserved0 : 2;
  u32 dirty_cfg_rcgr : 1;
  u32 dirty_m : 1;
  u32 dirty_n : 1;
  u32 dirty_d : 1;
  u32 reserved1 : 23;
  u32 root_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_ultaudio_lpaif_aux_pcm_dataoe_cmd_rcgr_u
{
  struct ipa_gcc_hwio_def_gcc_ultaudio_lpaif_aux_pcm_dataoe_cmd_rcgr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_ULTAUDIO_LPAIF_AUX_PCM_DATAOE_CFG_RCGR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_ultaudio_lpaif_aux_pcm_dataoe_cfg_rcgr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 1;
  u32 mode : 2;
  u32 reserved2 : 6;
  u32 hw_clk_control : 1;
  u32 reserved3 : 11;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_ultaudio_lpaif_aux_pcm_dataoe_cfg_rcgr_u
{
  struct ipa_gcc_hwio_def_gcc_ultaudio_lpaif_aux_pcm_dataoe_cfg_rcgr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_ULTAUDIO_LPAIF_AUX_PCM_DATAOE_M
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_ultaudio_lpaif_aux_pcm_dataoe_m_s
{
  u32 m : 8;
  u32 reserved0 : 24;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_ultaudio_lpaif_aux_pcm_dataoe_m_u
{
  struct ipa_gcc_hwio_def_gcc_ultaudio_lpaif_aux_pcm_dataoe_m_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_ULTAUDIO_LPAIF_AUX_PCM_DATAOE_N
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_ultaudio_lpaif_aux_pcm_dataoe_n_s
{
  u32 not_n_minus_m : 8;
  u32 reserved0 : 24;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_ultaudio_lpaif_aux_pcm_dataoe_n_u
{
  struct ipa_gcc_hwio_def_gcc_ultaudio_lpaif_aux_pcm_dataoe_n_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_ULTAUDIO_LPAIF_AUX_PCM_DATAOE_D
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_ultaudio_lpaif_aux_pcm_dataoe_d_s
{
  u32 not_2d : 8;
  u32 reserved0 : 24;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_ultaudio_lpaif_aux_pcm_dataoe_d_u
{
  struct ipa_gcc_hwio_def_gcc_ultaudio_lpaif_aux_pcm_dataoe_d_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_PCIE_BCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_pcie_bcr_s
{
  u32 blk_ares : 1;
  u32 reserved0 : 31;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_pcie_bcr_u
{
  struct ipa_gcc_hwio_def_gcc_pcie_bcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_PCIE_GDSCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_pcie_gdscr_s
{
  u32 sw_collapse : 1;
  u32 hw_control : 1;
  u32 sw_override : 1;
  u32 pd_ares : 1;
  u32 clk_disable : 1;
  u32 clamp_io : 1;
  u32 en_few : 1;
  u32 en_rest : 1;
  u32 retain : 1;
  u32 save : 1;
  u32 restore : 1;
  u32 retain_ff_enable : 1;
  u32 clk_dis_wait : 4;
  u32 en_few_wait : 4;
  u32 en_rest_wait : 4;
  u32 reserved0 : 3;
  u32 gdsc_state : 4;
  u32 pwr_on : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_pcie_gdscr_u
{
  struct ipa_gcc_hwio_def_gcc_pcie_gdscr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_PCIE_CFG_GDSCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_pcie_cfg_gdscr_s
{
  u32 disable_clk_software_override : 1;
  u32 clamp_io_software_override : 1;
  u32 save_restore_software_override : 1;
  u32 unclamp_io_software_override : 1;
  u32 gdsc_pscbc_pwr_dwn_sw : 1;
  u32 gdsc_phase_reset_delay_count_sw : 2;
  u32 gdsc_phase_reset_en_sw : 1;
  u32 gdsc_mem_core_force_in_sw : 1;
  u32 gdsc_mem_peri_force_in_sw : 1;
  u32 gdsc_handshake_dis : 1;
  u32 software_control_override : 4;
  u32 gdsc_power_down_complete : 1;
  u32 gdsc_power_up_complete : 1;
  u32 gdsc_enf_ack_status : 1;
  u32 gdsc_enr_ack_status : 1;
  u32 gdsc_mem_pwr_ack_status : 1;
  u32 gdsc_cfg_fsm_state_status : 4;
  u32 gdsc_pwr_up_start : 1;
  u32 gdsc_pwr_dwn_start : 1;
  u32 reserved0 : 6;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_pcie_cfg_gdscr_u
{
  struct ipa_gcc_hwio_def_gcc_pcie_cfg_gdscr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_PCIE_CFG2_GDSCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_pcie_cfg2_gdscr_s
{
  u32 mem_pwr_dwn_timeout : 4;
  u32 dly_assert_clamp_mem : 4;
  u32 dly_deassert_clamp_mem : 4;
  u32 dly_mem_pwr_up : 4;
  u32 gdsc_clamp_mem_sw : 1;
  u32 gdsc_pwrdwn_enable_ack_override : 1;
  u32 gdsc_mem_pwrup_ack_override : 1;
  u32 reserved0 : 13;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_pcie_cfg2_gdscr_u
{
  struct ipa_gcc_hwio_def_gcc_pcie_cfg2_gdscr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_PCIE_CFG3_GDSCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_pcie_cfg3_gdscr_s
{
  u32 gdsc_spare_ctrl_out : 8;
  u32 gdsc_spare_ctrl_in : 8;
  u32 gdsc_accu_red_sw_override : 1;
  u32 gdsc_accu_red_shifter_start_sw : 1;
  u32 gdsc_accu_red_shifter_clk_en_sw : 1;
  u32 gdsc_accu_red_shifter_done_override : 1;
  u32 gdsc_accu_red_timer_en_sw : 1;
  u32 dly_accu_red_shifter_done : 4;
  u32 gdsc_accu_red_enable : 1;
  u32 gdsc_accu_red_shifter_done_status : 1;
  u32 reserved0 : 5;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_pcie_cfg3_gdscr_u
{
  struct ipa_gcc_hwio_def_gcc_pcie_cfg3_gdscr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_PCIE_CFG4_GDSCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_pcie_cfg4_gdscr_s
{
  u32 dly_retainff : 4;
  u32 dly_clampio : 4;
  u32 dly_deassertares : 4;
  u32 dly_noretainff : 4;
  u32 dly_restoreff : 4;
  u32 dly_unclampio : 4;
  u32 reserved0 : 8;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_pcie_cfg4_gdscr_u
{
  struct ipa_gcc_hwio_def_gcc_pcie_cfg4_gdscr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_PCIE_SLV_Q2A_AXI_CBCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_pcie_slv_q2a_axi_cbcr_s
{
  u32 reserved0 : 1;
  u32 hw_ctl : 1;
  u32 clk_ares : 1;
  u32 sw_only_en : 1;
  u32 reserved1 : 16;
  u32 ignore_rpmh_clk_dis : 1;
  u32 reserved2 : 1;
  u32 clk_dis : 1;
  u32 ignore_all_clk_dis : 1;
  u32 ignore_all_ares : 1;
  u32 reserved3 : 6;
  u32 clk_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_pcie_slv_q2a_axi_cbcr_u
{
  struct ipa_gcc_hwio_def_gcc_pcie_slv_q2a_axi_cbcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_PCIE_SLV_AXI_CBCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_pcie_slv_axi_cbcr_s
{
  u32 reserved0 : 1;
  u32 hw_ctl : 1;
  u32 clk_ares : 1;
  u32 sw_only_en : 1;
  u32 sleep : 4;
  u32 wakeup : 4;
  u32 force_mem_periph_off : 1;
  u32 force_mem_periph_on : 1;
  u32 force_mem_core_on : 1;
  u32 reserved1 : 5;
  u32 ignore_rpmh_clk_dis : 1;
  u32 reserved2 : 1;
  u32 clk_dis : 1;
  u32 ignore_all_clk_dis : 1;
  u32 ignore_all_ares : 1;
  u32 reserved3 : 6;
  u32 clk_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_pcie_slv_axi_cbcr_u
{
  struct ipa_gcc_hwio_def_gcc_pcie_slv_axi_cbcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_PCIE_SLV_AXI_SREGR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_pcie_slv_axi_sregr_s
{
  u32 reserved0 : 1;
  u32 sw_clk_en_slp_stg : 1;
  u32 sw_clk_en_sel_slp_stg : 1;
  u32 sw_ctrl_pwr_down : 1;
  u32 sw_rst_slp_stg : 1;
  u32 sw_rst_sel_slp_stg : 1;
  u32 force_clk_on : 1;
  u32 mem_cph_enable : 1;
  u32 sw_div_ratio_slp_stg_clk : 2;
  u32 mem_periph_on_ack : 1;
  u32 mem_core_on_ack : 1;
  u32 sw_sm_pscbc_seq_in_override : 1;
  u32 mem_cph_rst_sw_override : 1;
  u32 pscbc_slp_stg_mode_csr : 1;
  u32 ignore_gdsc_pwr_dwn_csr : 1;
  u32 sreg_pscbc_spare_ctrl_in : 8;
  u32 sreg_pscbc_spare_ctrl_out : 8;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_pcie_slv_axi_sregr_u
{
  struct ipa_gcc_hwio_def_gcc_pcie_slv_axi_sregr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_PCIE_MSTR_AXI_CBCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_pcie_mstr_axi_cbcr_s
{
  u32 reserved0 : 1;
  u32 hw_ctl : 1;
  u32 clk_ares : 1;
  u32 sw_only_en : 1;
  u32 sleep : 4;
  u32 wakeup : 4;
  u32 force_mem_periph_off : 1;
  u32 force_mem_periph_on : 1;
  u32 force_mem_core_on : 1;
  u32 reserved1 : 5;
  u32 ignore_rpmh_clk_dis : 1;
  u32 reserved2 : 1;
  u32 clk_dis : 1;
  u32 ignore_all_clk_dis : 1;
  u32 ignore_all_ares : 1;
  u32 reserved3 : 6;
  u32 clk_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_pcie_mstr_axi_cbcr_u
{
  struct ipa_gcc_hwio_def_gcc_pcie_mstr_axi_cbcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_PCIE_MSTR_AXI_SREGR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_pcie_mstr_axi_sregr_s
{
  u32 reserved0 : 1;
  u32 sw_clk_en_slp_stg : 1;
  u32 sw_clk_en_sel_slp_stg : 1;
  u32 sw_ctrl_pwr_down : 1;
  u32 sw_rst_slp_stg : 1;
  u32 sw_rst_sel_slp_stg : 1;
  u32 force_clk_on : 1;
  u32 mem_cph_enable : 1;
  u32 sw_div_ratio_slp_stg_clk : 2;
  u32 mem_periph_on_ack : 1;
  u32 mem_core_on_ack : 1;
  u32 sw_sm_pscbc_seq_in_override : 1;
  u32 mem_cph_rst_sw_override : 1;
  u32 pscbc_slp_stg_mode_csr : 1;
  u32 ignore_gdsc_pwr_dwn_csr : 1;
  u32 sreg_pscbc_spare_ctrl_in : 8;
  u32 sreg_pscbc_spare_ctrl_out : 8;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_pcie_mstr_axi_sregr_u
{
  struct ipa_gcc_hwio_def_gcc_pcie_mstr_axi_sregr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_PCIE_CFG_AHB_CBCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_pcie_cfg_ahb_cbcr_s
{
  u32 reserved0 : 1;
  u32 hw_ctl : 1;
  u32 clk_ares : 1;
  u32 sw_only_en : 1;
  u32 reserved1 : 16;
  u32 ignore_rpmh_clk_dis : 1;
  u32 reserved2 : 1;
  u32 clk_dis : 1;
  u32 ignore_all_clk_dis : 1;
  u32 ignore_all_ares : 1;
  u32 reserved3 : 6;
  u32 clk_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_pcie_cfg_ahb_cbcr_u
{
  struct ipa_gcc_hwio_def_gcc_pcie_cfg_ahb_cbcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_PCIE_RCHNG_PHY_CBCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_pcie_rchng_phy_cbcr_s
{
  u32 reserved0 : 1;
  u32 hw_ctl : 1;
  u32 clk_ares : 1;
  u32 sw_only_en : 1;
  u32 reserved1 : 18;
  u32 clk_dis : 1;
  u32 ignore_all_clk_dis : 1;
  u32 ignore_all_ares : 1;
  u32 reserved2 : 6;
  u32 clk_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_pcie_rchng_phy_cbcr_u
{
  struct ipa_gcc_hwio_def_gcc_pcie_rchng_phy_cbcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_PCIE_AUX_CBCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_pcie_aux_cbcr_s
{
  u32 reserved0 : 1;
  u32 hw_ctl : 1;
  u32 clk_ares : 1;
  u32 sw_only_en : 1;
  u32 reserved1 : 18;
  u32 clk_dis : 1;
  u32 ignore_all_clk_dis : 1;
  u32 ignore_all_ares : 1;
  u32 reserved2 : 6;
  u32 clk_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_pcie_aux_cbcr_u
{
  struct ipa_gcc_hwio_def_gcc_pcie_aux_cbcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_PCIE_SLEEP_CBCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_pcie_sleep_cbcr_s
{
  u32 reserved0 : 1;
  u32 hw_ctl : 1;
  u32 clk_ares : 1;
  u32 sw_only_en : 1;
  u32 reserved1 : 18;
  u32 clk_dis : 1;
  u32 ignore_all_clk_dis : 1;
  u32 ignore_all_ares : 1;
  u32 reserved2 : 6;
  u32 clk_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_pcie_sleep_cbcr_u
{
  struct ipa_gcc_hwio_def_gcc_pcie_sleep_cbcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_PCIE_PIPE_CBCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_pcie_pipe_cbcr_s
{
  u32 reserved0 : 1;
  u32 hw_ctl : 1;
  u32 clk_ares : 1;
  u32 sw_only_en : 1;
  u32 sleep : 4;
  u32 wakeup : 4;
  u32 force_mem_periph_off : 1;
  u32 force_mem_periph_on : 1;
  u32 force_mem_core_on : 1;
  u32 reserved1 : 7;
  u32 clk_dis : 1;
  u32 ignore_all_clk_dis : 1;
  u32 ignore_all_ares : 1;
  u32 reserved2 : 6;
  u32 clk_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_pcie_pipe_cbcr_u
{
  struct ipa_gcc_hwio_def_gcc_pcie_pipe_cbcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_PCIE_PIPE_SREGR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_pcie_pipe_sregr_s
{
  u32 reserved0 : 1;
  u32 sw_clk_en_slp_stg : 1;
  u32 sw_clk_en_sel_slp_stg : 1;
  u32 sw_ctrl_pwr_down : 1;
  u32 sw_rst_slp_stg : 1;
  u32 sw_rst_sel_slp_stg : 1;
  u32 force_clk_on : 1;
  u32 mem_cph_enable : 1;
  u32 sw_div_ratio_slp_stg_clk : 2;
  u32 mem_periph_on_ack : 1;
  u32 mem_core_on_ack : 1;
  u32 sw_sm_pscbc_seq_in_override : 1;
  u32 mem_cph_rst_sw_override : 1;
  u32 pscbc_slp_stg_mode_csr : 1;
  u32 ignore_gdsc_pwr_dwn_csr : 1;
  u32 sreg_pscbc_spare_ctrl_in : 8;
  u32 sreg_pscbc_spare_ctrl_out : 8;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_pcie_pipe_sregr_u
{
  struct ipa_gcc_hwio_def_gcc_pcie_pipe_sregr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_PCIE_AUX_PHY_CMD_RCGR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_pcie_aux_phy_cmd_rcgr_s
{
  u32 update : 1;
  u32 root_en : 1;
  u32 reserved0 : 2;
  u32 dirty_cfg_rcgr : 1;
  u32 dirty_m : 1;
  u32 dirty_n : 1;
  u32 dirty_d : 1;
  u32 reserved1 : 23;
  u32 root_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_pcie_aux_phy_cmd_rcgr_u
{
  struct ipa_gcc_hwio_def_gcc_pcie_aux_phy_cmd_rcgr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_PCIE_AUX_PHY_CFG_RCGR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_pcie_aux_phy_cfg_rcgr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 1;
  u32 mode : 2;
  u32 reserved2 : 6;
  u32 hw_clk_control : 1;
  u32 reserved3 : 11;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_pcie_aux_phy_cfg_rcgr_u
{
  struct ipa_gcc_hwio_def_gcc_pcie_aux_phy_cfg_rcgr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_PCIE_AUX_PHY_M
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_pcie_aux_phy_m_s
{
  u32 m : 16;
  u32 reserved0 : 16;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_pcie_aux_phy_m_u
{
  struct ipa_gcc_hwio_def_gcc_pcie_aux_phy_m_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_PCIE_AUX_PHY_N
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_pcie_aux_phy_n_s
{
  u32 not_n_minus_m : 16;
  u32 reserved0 : 16;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_pcie_aux_phy_n_u
{
  struct ipa_gcc_hwio_def_gcc_pcie_aux_phy_n_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_PCIE_AUX_PHY_D
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_pcie_aux_phy_d_s
{
  u32 not_2d : 16;
  u32 reserved0 : 16;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_pcie_aux_phy_d_u
{
  struct ipa_gcc_hwio_def_gcc_pcie_aux_phy_d_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_PCIE_RCHNG_PHY_CMD_RCGR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_pcie_rchng_phy_cmd_rcgr_s
{
  u32 update : 1;
  u32 root_en : 1;
  u32 reserved0 : 2;
  u32 dirty_cfg_rcgr : 1;
  u32 reserved1 : 26;
  u32 root_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_pcie_rchng_phy_cmd_rcgr_u
{
  struct ipa_gcc_hwio_def_gcc_pcie_rchng_phy_cmd_rcgr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_PCIE_RCHNG_PHY_CFG_RCGR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_pcie_rchng_phy_cfg_rcgr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 5;
  u32 rcglite_disable : 1;
  u32 reserved2 : 3;
  u32 hw_clk_control : 1;
  u32 reserved3 : 11;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_pcie_rchng_phy_cfg_rcgr_u
{
  struct ipa_gcc_hwio_def_gcc_pcie_rchng_phy_cfg_rcgr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_PCIE_PHY_BCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_pcie_phy_bcr_s
{
  u32 blk_ares : 1;
  u32 reserved0 : 31;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_pcie_phy_bcr_u
{
  struct ipa_gcc_hwio_def_gcc_pcie_phy_bcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_VS_BCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_vs_bcr_s
{
  u32 blk_ares : 1;
  u32 reserved0 : 31;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_vs_bcr_u
{
  struct ipa_gcc_hwio_def_gcc_vs_bcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_VDDCX_VS_CBCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_vddcx_vs_cbcr_s
{
  u32 clk_enable : 1;
  u32 hw_ctl : 1;
  u32 clk_ares : 1;
  u32 sw_only_en : 1;
  u32 reserved0 : 18;
  u32 clk_dis : 1;
  u32 ignore_all_clk_dis : 1;
  u32 ignore_all_ares : 1;
  u32 reserved1 : 6;
  u32 clk_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_vddcx_vs_cbcr_u
{
  struct ipa_gcc_hwio_def_gcc_vddcx_vs_cbcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_VDDMX_VS_CBCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_vddmx_vs_cbcr_s
{
  u32 clk_enable : 1;
  u32 hw_ctl : 1;
  u32 clk_ares : 1;
  u32 sw_only_en : 1;
  u32 reserved0 : 18;
  u32 clk_dis : 1;
  u32 ignore_all_clk_dis : 1;
  u32 ignore_all_ares : 1;
  u32 reserved1 : 6;
  u32 clk_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_vddmx_vs_cbcr_u
{
  struct ipa_gcc_hwio_def_gcc_vddmx_vs_cbcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_VDDA_VS_CBCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_vdda_vs_cbcr_s
{
  u32 clk_enable : 1;
  u32 hw_ctl : 1;
  u32 clk_ares : 1;
  u32 sw_only_en : 1;
  u32 reserved0 : 18;
  u32 clk_dis : 1;
  u32 ignore_all_clk_dis : 1;
  u32 ignore_all_ares : 1;
  u32 reserved1 : 6;
  u32 clk_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_vdda_vs_cbcr_u
{
  struct ipa_gcc_hwio_def_gcc_vdda_vs_cbcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_VDDMXC_VS_CBCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_vddmxc_vs_cbcr_s
{
  u32 clk_enable : 1;
  u32 hw_ctl : 1;
  u32 clk_ares : 1;
  u32 sw_only_en : 1;
  u32 reserved0 : 18;
  u32 clk_dis : 1;
  u32 ignore_all_clk_dis : 1;
  u32 ignore_all_ares : 1;
  u32 reserved1 : 6;
  u32 clk_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_vddmxc_vs_cbcr_u
{
  struct ipa_gcc_hwio_def_gcc_vddmxc_vs_cbcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_VS_CTRL_CBCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_vs_ctrl_cbcr_s
{
  u32 clk_enable : 1;
  u32 hw_ctl : 1;
  u32 clk_ares : 1;
  u32 sw_only_en : 1;
  u32 reserved0 : 18;
  u32 clk_dis : 1;
  u32 ignore_all_clk_dis : 1;
  u32 ignore_all_ares : 1;
  u32 reserved1 : 6;
  u32 clk_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_vs_ctrl_cbcr_u
{
  struct ipa_gcc_hwio_def_gcc_vs_ctrl_cbcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_VS_CTRL_AHB_CBCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_vs_ctrl_ahb_cbcr_s
{
  u32 clk_enable : 1;
  u32 hw_ctl : 1;
  u32 clk_ares : 1;
  u32 sw_only_en : 1;
  u32 reserved0 : 16;
  u32 ignore_rpmh_clk_dis : 1;
  u32 reserved1 : 1;
  u32 clk_dis : 1;
  u32 ignore_all_clk_dis : 1;
  u32 ignore_all_ares : 1;
  u32 reserved2 : 6;
  u32 clk_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_vs_ctrl_ahb_cbcr_u
{
  struct ipa_gcc_hwio_def_gcc_vs_ctrl_ahb_cbcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_VSENSOR_CMD_RCGR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_vsensor_cmd_rcgr_s
{
  u32 update : 1;
  u32 root_en : 1;
  u32 reserved0 : 2;
  u32 dirty_cfg_rcgr : 1;
  u32 reserved1 : 26;
  u32 root_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_vsensor_cmd_rcgr_u
{
  struct ipa_gcc_hwio_def_gcc_vsensor_cmd_rcgr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_VSENSOR_CFG_RCGR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_vsensor_cfg_rcgr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 5;
  u32 rcglite_disable : 1;
  u32 reserved2 : 3;
  u32 hw_clk_control : 1;
  u32 reserved3 : 11;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_vsensor_cfg_rcgr_u
{
  struct ipa_gcc_hwio_def_gcc_vsensor_cfg_rcgr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_VS_CTRL_CMD_RCGR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_vs_ctrl_cmd_rcgr_s
{
  u32 update : 1;
  u32 root_en : 1;
  u32 reserved0 : 2;
  u32 dirty_cfg_rcgr : 1;
  u32 reserved1 : 26;
  u32 root_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_vs_ctrl_cmd_rcgr_u
{
  struct ipa_gcc_hwio_def_gcc_vs_ctrl_cmd_rcgr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_VS_CTRL_CFG_RCGR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_vs_ctrl_cfg_rcgr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 5;
  u32 rcglite_disable : 1;
  u32 reserved2 : 3;
  u32 hw_clk_control : 1;
  u32 reserved3 : 11;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_vs_ctrl_cfg_rcgr_u
{
  struct ipa_gcc_hwio_def_gcc_vs_ctrl_cfg_rcgr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_MSS_VS_CBCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_mss_vs_cbcr_s
{
  u32 clk_enable : 1;
  u32 hw_ctl : 1;
  u32 clk_ares : 1;
  u32 sw_only_en : 1;
  u32 reserved0 : 18;
  u32 clk_dis : 1;
  u32 ignore_all_clk_dis : 1;
  u32 ignore_all_ares : 1;
  u32 reserved1 : 6;
  u32 clk_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_mss_vs_cbcr_u
{
  struct ipa_gcc_hwio_def_gcc_mss_vs_cbcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_DCC_BCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_dcc_bcr_s
{
  u32 blk_ares : 1;
  u32 reserved0 : 31;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_dcc_bcr_u
{
  struct ipa_gcc_hwio_def_gcc_dcc_bcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_DCC_AHB_CBCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_dcc_ahb_cbcr_s
{
  u32 clk_enable : 1;
  u32 hw_ctl : 1;
  u32 clk_ares : 1;
  u32 sw_only_en : 1;
  u32 sleep : 4;
  u32 wakeup : 4;
  u32 force_mem_periph_off : 1;
  u32 force_mem_periph_on : 1;
  u32 force_mem_core_on : 1;
  u32 reserved0 : 5;
  u32 ignore_rpmh_clk_dis : 1;
  u32 reserved1 : 1;
  u32 clk_dis : 1;
  u32 ignore_all_clk_dis : 1;
  u32 ignore_all_ares : 1;
  u32 reserved2 : 6;
  u32 clk_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_dcc_ahb_cbcr_u
{
  struct ipa_gcc_hwio_def_gcc_dcc_ahb_cbcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_DCC_AHB_SREGR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_dcc_ahb_sregr_s
{
  u32 reserved0 : 1;
  u32 sw_clk_en_slp_stg : 1;
  u32 sw_clk_en_sel_slp_stg : 1;
  u32 sw_ctrl_pwr_down : 1;
  u32 sw_rst_slp_stg : 1;
  u32 sw_rst_sel_slp_stg : 1;
  u32 force_clk_on : 1;
  u32 mem_cph_enable : 1;
  u32 sw_div_ratio_slp_stg_clk : 2;
  u32 mem_periph_on_ack : 1;
  u32 mem_core_on_ack : 1;
  u32 sw_sm_pscbc_seq_in_override : 1;
  u32 mem_cph_rst_sw_override : 1;
  u32 pscbc_slp_stg_mode_csr : 1;
  u32 ignore_gdsc_pwr_dwn_csr : 1;
  u32 sreg_pscbc_spare_ctrl_in : 8;
  u32 sreg_pscbc_spare_ctrl_out : 8;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_dcc_ahb_sregr_u
{
  struct ipa_gcc_hwio_def_gcc_dcc_ahb_sregr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_IPA_BCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_ipa_bcr_s
{
  u32 blk_ares : 1;
  u32 reserved0 : 31;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_ipa_bcr_u
{
  struct ipa_gcc_hwio_def_gcc_ipa_bcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_IPA_GDSCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_ipa_gdscr_s
{
  u32 sw_collapse : 1;
  u32 hw_control : 1;
  u32 sw_override : 1;
  u32 pd_ares : 1;
  u32 clk_disable : 1;
  u32 clamp_io : 1;
  u32 en_few : 1;
  u32 en_rest : 1;
  u32 retain : 1;
  u32 save : 1;
  u32 restore : 1;
  u32 retain_ff_enable : 1;
  u32 clk_dis_wait : 4;
  u32 en_few_wait : 4;
  u32 en_rest_wait : 4;
  u32 reserved0 : 3;
  u32 gdsc_state : 4;
  u32 pwr_on : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_ipa_gdscr_u
{
  struct ipa_gcc_hwio_def_gcc_ipa_gdscr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_IPA_CFG_GDSCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_ipa_cfg_gdscr_s
{
  u32 disable_clk_software_override : 1;
  u32 clamp_io_software_override : 1;
  u32 save_restore_software_override : 1;
  u32 unclamp_io_software_override : 1;
  u32 gdsc_pscbc_pwr_dwn_sw : 1;
  u32 gdsc_phase_reset_delay_count_sw : 2;
  u32 gdsc_phase_reset_en_sw : 1;
  u32 gdsc_mem_core_force_in_sw : 1;
  u32 gdsc_mem_peri_force_in_sw : 1;
  u32 gdsc_handshake_dis : 1;
  u32 software_control_override : 4;
  u32 gdsc_power_down_complete : 1;
  u32 gdsc_power_up_complete : 1;
  u32 gdsc_enf_ack_status : 1;
  u32 gdsc_enr_ack_status : 1;
  u32 gdsc_mem_pwr_ack_status : 1;
  u32 gdsc_cfg_fsm_state_status : 4;
  u32 gdsc_pwr_up_start : 1;
  u32 gdsc_pwr_dwn_start : 1;
  u32 reserved0 : 6;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_ipa_cfg_gdscr_u
{
  struct ipa_gcc_hwio_def_gcc_ipa_cfg_gdscr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_IPA_CFG2_GDSCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_ipa_cfg2_gdscr_s
{
  u32 mem_pwr_dwn_timeout : 4;
  u32 dly_assert_clamp_mem : 4;
  u32 dly_deassert_clamp_mem : 4;
  u32 dly_mem_pwr_up : 4;
  u32 gdsc_clamp_mem_sw : 1;
  u32 gdsc_pwrdwn_enable_ack_override : 1;
  u32 gdsc_mem_pwrup_ack_override : 1;
  u32 reserved0 : 13;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_ipa_cfg2_gdscr_u
{
  struct ipa_gcc_hwio_def_gcc_ipa_cfg2_gdscr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_IPA_CFG3_GDSCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_ipa_cfg3_gdscr_s
{
  u32 gdsc_spare_ctrl_out : 8;
  u32 gdsc_spare_ctrl_in : 8;
  u32 gdsc_accu_red_sw_override : 1;
  u32 gdsc_accu_red_shifter_start_sw : 1;
  u32 gdsc_accu_red_shifter_clk_en_sw : 1;
  u32 gdsc_accu_red_shifter_done_override : 1;
  u32 gdsc_accu_red_timer_en_sw : 1;
  u32 dly_accu_red_shifter_done : 4;
  u32 gdsc_accu_red_enable : 1;
  u32 gdsc_accu_red_shifter_done_status : 1;
  u32 reserved0 : 5;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_ipa_cfg3_gdscr_u
{
  struct ipa_gcc_hwio_def_gcc_ipa_cfg3_gdscr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_IPA_CFG4_GDSCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_ipa_cfg4_gdscr_s
{
  u32 dly_retainff : 4;
  u32 dly_clampio : 4;
  u32 dly_deassertares : 4;
  u32 dly_noretainff : 4;
  u32 dly_restoreff : 4;
  u32 dly_unclampio : 4;
  u32 reserved0 : 8;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_ipa_cfg4_gdscr_u
{
  struct ipa_gcc_hwio_def_gcc_ipa_cfg4_gdscr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_IPA_2X_CBCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_ipa_2x_cbcr_s
{
  u32 clk_enable : 1;
  u32 reserved0 : 1;
  u32 clk_ares : 1;
  u32 reserved1 : 1;
  u32 sleep : 4;
  u32 wakeup : 4;
  u32 force_mem_periph_off : 1;
  u32 force_mem_periph_on : 1;
  u32 force_mem_core_on : 1;
  u32 reserved2 : 5;
  u32 ignore_rpmh_clk_dis : 1;
  u32 reserved3 : 1;
  u32 clk_dis : 1;
  u32 ignore_all_clk_dis : 1;
  u32 ignore_all_ares : 1;
  u32 reserved4 : 6;
  u32 clk_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_ipa_2x_cbcr_u
{
  struct ipa_gcc_hwio_def_gcc_ipa_2x_cbcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_IPA_2X_SREGR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_ipa_2x_sregr_s
{
  u32 reserved0 : 1;
  u32 sw_clk_en_slp_stg : 1;
  u32 sw_clk_en_sel_slp_stg : 1;
  u32 sw_ctrl_pwr_down : 1;
  u32 sw_rst_slp_stg : 1;
  u32 sw_rst_sel_slp_stg : 1;
  u32 force_clk_on : 1;
  u32 mem_cph_enable : 1;
  u32 sw_div_ratio_slp_stg_clk : 2;
  u32 mem_periph_on_ack : 1;
  u32 mem_core_on_ack : 1;
  u32 sw_sm_pscbc_seq_in_override : 1;
  u32 mem_cph_rst_sw_override : 1;
  u32 pscbc_slp_stg_mode_csr : 1;
  u32 ignore_gdsc_pwr_dwn_csr : 1;
  u32 sreg_pscbc_spare_ctrl_in : 8;
  u32 sreg_pscbc_spare_ctrl_out : 8;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_ipa_2x_sregr_u
{
  struct ipa_gcc_hwio_def_gcc_ipa_2x_sregr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_IPA_CBCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_ipa_cbcr_s
{
  u32 clk_enable : 1;
  u32 reserved0 : 1;
  u32 clk_ares : 1;
  u32 reserved1 : 1;
  u32 sleep : 4;
  u32 wakeup : 4;
  u32 force_mem_periph_off : 1;
  u32 force_mem_periph_on : 1;
  u32 force_mem_core_on : 1;
  u32 reserved2 : 5;
  u32 ignore_rpmh_clk_dis : 1;
  u32 reserved3 : 1;
  u32 clk_dis : 1;
  u32 ignore_all_clk_dis : 1;
  u32 ignore_all_ares : 1;
  u32 reserved4 : 6;
  u32 clk_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_ipa_cbcr_u
{
  struct ipa_gcc_hwio_def_gcc_ipa_cbcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_IPA_SREGR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_ipa_sregr_s
{
  u32 reserved0 : 1;
  u32 sw_clk_en_slp_stg : 1;
  u32 sw_clk_en_sel_slp_stg : 1;
  u32 sw_ctrl_pwr_down : 1;
  u32 sw_rst_slp_stg : 1;
  u32 sw_rst_sel_slp_stg : 1;
  u32 force_clk_on : 1;
  u32 mem_cph_enable : 1;
  u32 sw_div_ratio_slp_stg_clk : 2;
  u32 mem_periph_on_ack : 1;
  u32 mem_core_on_ack : 1;
  u32 sw_sm_pscbc_seq_in_override : 1;
  u32 mem_cph_rst_sw_override : 1;
  u32 pscbc_slp_stg_mode_csr : 1;
  u32 ignore_gdsc_pwr_dwn_csr : 1;
  u32 sreg_pscbc_spare_ctrl_in : 8;
  u32 sreg_pscbc_spare_ctrl_out : 8;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_ipa_sregr_u
{
  struct ipa_gcc_hwio_def_gcc_ipa_sregr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_IPA_AHB_CBCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_ipa_ahb_cbcr_s
{
  u32 clk_enable : 1;
  u32 hw_ctl : 1;
  u32 clk_ares : 1;
  u32 sw_only_en : 1;
  u32 reserved0 : 16;
  u32 ignore_rpmh_clk_dis : 1;
  u32 reserved1 : 1;
  u32 clk_dis : 1;
  u32 ignore_all_clk_dis : 1;
  u32 ignore_all_ares : 1;
  u32 reserved2 : 6;
  u32 clk_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_ipa_ahb_cbcr_u
{
  struct ipa_gcc_hwio_def_gcc_ipa_ahb_cbcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_IPA_XO_CBCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_ipa_xo_cbcr_s
{
  u32 clk_enable : 1;
  u32 reserved0 : 1;
  u32 clk_ares : 1;
  u32 reserved1 : 19;
  u32 clk_dis : 1;
  u32 ignore_all_clk_dis : 1;
  u32 ignore_all_ares : 1;
  u32 reserved2 : 6;
  u32 clk_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_ipa_xo_cbcr_u
{
  struct ipa_gcc_hwio_def_gcc_ipa_xo_cbcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_IPA_2X_CDIV_DCD_DCDR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_ipa_2x_cdiv_dcd_dcdr_s
{
  u32 dcd_enable : 1;
  u32 reserved0 : 31;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_ipa_2x_cdiv_dcd_dcdr_u
{
  struct ipa_gcc_hwio_def_gcc_ipa_2x_cdiv_dcd_dcdr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_IPA_CMD_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_ipa_cmd_dfsr_s
{
  u32 dfs_en : 1;
  u32 curr_perf_state : 4;
  u32 hw_clk_control : 1;
  u32 dfs_fsm_state : 3;
  u32 perf_state_update_status : 1;
  u32 sw_override : 1;
  u32 sw_perf_state : 4;
  u32 rcg_sw_ctrl : 1;
  u32 reserved0 : 16;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_ipa_cmd_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_ipa_cmd_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_IPA_IPA_2X_PERF0_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_ipa_ipa_2x_perf0_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 1;
  u32 mode : 2;
  u32 reserved2 : 18;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_ipa_ipa_2x_perf0_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_ipa_ipa_2x_perf0_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_IPA_IPA_2X_PERF1_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_ipa_ipa_2x_perf1_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 1;
  u32 mode : 2;
  u32 reserved2 : 18;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_ipa_ipa_2x_perf1_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_ipa_ipa_2x_perf1_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_IPA_IPA_2X_PERF2_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_ipa_ipa_2x_perf2_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 1;
  u32 mode : 2;
  u32 reserved2 : 18;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_ipa_ipa_2x_perf2_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_ipa_ipa_2x_perf2_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_IPA_IPA_2X_PERF3_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_ipa_ipa_2x_perf3_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 1;
  u32 mode : 2;
  u32 reserved2 : 18;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_ipa_ipa_2x_perf3_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_ipa_ipa_2x_perf3_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_IPA_IPA_2X_PERF4_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_ipa_ipa_2x_perf4_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 1;
  u32 mode : 2;
  u32 reserved2 : 18;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_ipa_ipa_2x_perf4_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_ipa_ipa_2x_perf4_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_IPA_IPA_2X_PERF5_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_ipa_ipa_2x_perf5_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 1;
  u32 mode : 2;
  u32 reserved2 : 18;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_ipa_ipa_2x_perf5_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_ipa_ipa_2x_perf5_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_IPA_IPA_2X_PERF6_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_ipa_ipa_2x_perf6_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 1;
  u32 mode : 2;
  u32 reserved2 : 18;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_ipa_ipa_2x_perf6_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_ipa_ipa_2x_perf6_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_IPA_IPA_2X_PERF7_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_ipa_ipa_2x_perf7_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 1;
  u32 mode : 2;
  u32 reserved2 : 18;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_ipa_ipa_2x_perf7_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_ipa_ipa_2x_perf7_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_IPA_IPA_2X_PERF8_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_ipa_ipa_2x_perf8_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 1;
  u32 mode : 2;
  u32 reserved2 : 18;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_ipa_ipa_2x_perf8_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_ipa_ipa_2x_perf8_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_IPA_IPA_2X_PERF9_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_ipa_ipa_2x_perf9_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 1;
  u32 mode : 2;
  u32 reserved2 : 18;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_ipa_ipa_2x_perf9_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_ipa_ipa_2x_perf9_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_IPA_IPA_2X_PERF10_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_ipa_ipa_2x_perf10_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 1;
  u32 mode : 2;
  u32 reserved2 : 18;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_ipa_ipa_2x_perf10_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_ipa_ipa_2x_perf10_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_IPA_IPA_2X_PERF11_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_ipa_ipa_2x_perf11_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 1;
  u32 mode : 2;
  u32 reserved2 : 18;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_ipa_ipa_2x_perf11_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_ipa_ipa_2x_perf11_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_IPA_IPA_2X_PERF12_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_ipa_ipa_2x_perf12_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 1;
  u32 mode : 2;
  u32 reserved2 : 18;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_ipa_ipa_2x_perf12_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_ipa_ipa_2x_perf12_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_IPA_IPA_2X_PERF13_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_ipa_ipa_2x_perf13_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 1;
  u32 mode : 2;
  u32 reserved2 : 18;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_ipa_ipa_2x_perf13_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_ipa_ipa_2x_perf13_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_IPA_IPA_2X_PERF14_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_ipa_ipa_2x_perf14_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 1;
  u32 mode : 2;
  u32 reserved2 : 18;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_ipa_ipa_2x_perf14_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_ipa_ipa_2x_perf14_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_IPA_IPA_2X_PERF15_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_ipa_ipa_2x_perf15_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 1;
  u32 mode : 2;
  u32 reserved2 : 18;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_ipa_ipa_2x_perf15_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_ipa_ipa_2x_perf15_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_IPA_IPA_2X_PERF0_M_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_ipa_ipa_2x_perf0_m_dfsr_s
{
  u32 m : 8;
  u32 reserved0 : 24;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_ipa_ipa_2x_perf0_m_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_ipa_ipa_2x_perf0_m_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_IPA_IPA_2X_PERF1_M_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_ipa_ipa_2x_perf1_m_dfsr_s
{
  u32 m : 8;
  u32 reserved0 : 24;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_ipa_ipa_2x_perf1_m_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_ipa_ipa_2x_perf1_m_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_IPA_IPA_2X_PERF2_M_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_ipa_ipa_2x_perf2_m_dfsr_s
{
  u32 m : 8;
  u32 reserved0 : 24;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_ipa_ipa_2x_perf2_m_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_ipa_ipa_2x_perf2_m_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_IPA_IPA_2X_PERF3_M_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_ipa_ipa_2x_perf3_m_dfsr_s
{
  u32 m : 8;
  u32 reserved0 : 24;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_ipa_ipa_2x_perf3_m_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_ipa_ipa_2x_perf3_m_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_IPA_IPA_2X_PERF4_M_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_ipa_ipa_2x_perf4_m_dfsr_s
{
  u32 m : 8;
  u32 reserved0 : 24;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_ipa_ipa_2x_perf4_m_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_ipa_ipa_2x_perf4_m_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_IPA_IPA_2X_PERF5_M_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_ipa_ipa_2x_perf5_m_dfsr_s
{
  u32 m : 8;
  u32 reserved0 : 24;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_ipa_ipa_2x_perf5_m_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_ipa_ipa_2x_perf5_m_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_IPA_IPA_2X_PERF6_M_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_ipa_ipa_2x_perf6_m_dfsr_s
{
  u32 m : 8;
  u32 reserved0 : 24;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_ipa_ipa_2x_perf6_m_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_ipa_ipa_2x_perf6_m_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_IPA_IPA_2X_PERF7_M_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_ipa_ipa_2x_perf7_m_dfsr_s
{
  u32 m : 8;
  u32 reserved0 : 24;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_ipa_ipa_2x_perf7_m_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_ipa_ipa_2x_perf7_m_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_IPA_IPA_2X_PERF8_M_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_ipa_ipa_2x_perf8_m_dfsr_s
{
  u32 m : 8;
  u32 reserved0 : 24;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_ipa_ipa_2x_perf8_m_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_ipa_ipa_2x_perf8_m_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_IPA_IPA_2X_PERF9_M_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_ipa_ipa_2x_perf9_m_dfsr_s
{
  u32 m : 8;
  u32 reserved0 : 24;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_ipa_ipa_2x_perf9_m_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_ipa_ipa_2x_perf9_m_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_IPA_IPA_2X_PERF10_M_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_ipa_ipa_2x_perf10_m_dfsr_s
{
  u32 m : 8;
  u32 reserved0 : 24;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_ipa_ipa_2x_perf10_m_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_ipa_ipa_2x_perf10_m_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_IPA_IPA_2X_PERF11_M_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_ipa_ipa_2x_perf11_m_dfsr_s
{
  u32 m : 8;
  u32 reserved0 : 24;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_ipa_ipa_2x_perf11_m_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_ipa_ipa_2x_perf11_m_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_IPA_IPA_2X_PERF12_M_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_ipa_ipa_2x_perf12_m_dfsr_s
{
  u32 m : 8;
  u32 reserved0 : 24;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_ipa_ipa_2x_perf12_m_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_ipa_ipa_2x_perf12_m_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_IPA_IPA_2X_PERF13_M_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_ipa_ipa_2x_perf13_m_dfsr_s
{
  u32 m : 8;
  u32 reserved0 : 24;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_ipa_ipa_2x_perf13_m_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_ipa_ipa_2x_perf13_m_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_IPA_IPA_2X_PERF14_M_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_ipa_ipa_2x_perf14_m_dfsr_s
{
  u32 m : 8;
  u32 reserved0 : 24;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_ipa_ipa_2x_perf14_m_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_ipa_ipa_2x_perf14_m_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_IPA_IPA_2X_PERF15_M_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_ipa_ipa_2x_perf15_m_dfsr_s
{
  u32 m : 8;
  u32 reserved0 : 24;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_ipa_ipa_2x_perf15_m_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_ipa_ipa_2x_perf15_m_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_IPA_IPA_2X_PERF0_N_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_ipa_ipa_2x_perf0_n_dfsr_s
{
  u32 not_n_minus_m : 8;
  u32 reserved0 : 24;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_ipa_ipa_2x_perf0_n_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_ipa_ipa_2x_perf0_n_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_IPA_IPA_2X_PERF1_N_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_ipa_ipa_2x_perf1_n_dfsr_s
{
  u32 not_n_minus_m : 8;
  u32 reserved0 : 24;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_ipa_ipa_2x_perf1_n_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_ipa_ipa_2x_perf1_n_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_IPA_IPA_2X_PERF2_N_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_ipa_ipa_2x_perf2_n_dfsr_s
{
  u32 not_n_minus_m : 8;
  u32 reserved0 : 24;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_ipa_ipa_2x_perf2_n_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_ipa_ipa_2x_perf2_n_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_IPA_IPA_2X_PERF3_N_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_ipa_ipa_2x_perf3_n_dfsr_s
{
  u32 not_n_minus_m : 8;
  u32 reserved0 : 24;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_ipa_ipa_2x_perf3_n_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_ipa_ipa_2x_perf3_n_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_IPA_IPA_2X_PERF4_N_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_ipa_ipa_2x_perf4_n_dfsr_s
{
  u32 not_n_minus_m : 8;
  u32 reserved0 : 24;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_ipa_ipa_2x_perf4_n_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_ipa_ipa_2x_perf4_n_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_IPA_IPA_2X_PERF5_N_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_ipa_ipa_2x_perf5_n_dfsr_s
{
  u32 not_n_minus_m : 8;
  u32 reserved0 : 24;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_ipa_ipa_2x_perf5_n_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_ipa_ipa_2x_perf5_n_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_IPA_IPA_2X_PERF6_N_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_ipa_ipa_2x_perf6_n_dfsr_s
{
  u32 not_n_minus_m : 8;
  u32 reserved0 : 24;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_ipa_ipa_2x_perf6_n_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_ipa_ipa_2x_perf6_n_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_IPA_IPA_2X_PERF7_N_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_ipa_ipa_2x_perf7_n_dfsr_s
{
  u32 not_n_minus_m : 8;
  u32 reserved0 : 24;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_ipa_ipa_2x_perf7_n_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_ipa_ipa_2x_perf7_n_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_IPA_IPA_2X_PERF8_N_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_ipa_ipa_2x_perf8_n_dfsr_s
{
  u32 not_n_minus_m : 8;
  u32 reserved0 : 24;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_ipa_ipa_2x_perf8_n_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_ipa_ipa_2x_perf8_n_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_IPA_IPA_2X_PERF9_N_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_ipa_ipa_2x_perf9_n_dfsr_s
{
  u32 not_n_minus_m : 8;
  u32 reserved0 : 24;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_ipa_ipa_2x_perf9_n_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_ipa_ipa_2x_perf9_n_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_IPA_IPA_2X_PERF10_N_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_ipa_ipa_2x_perf10_n_dfsr_s
{
  u32 not_n_minus_m : 8;
  u32 reserved0 : 24;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_ipa_ipa_2x_perf10_n_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_ipa_ipa_2x_perf10_n_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_IPA_IPA_2X_PERF11_N_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_ipa_ipa_2x_perf11_n_dfsr_s
{
  u32 not_n_minus_m : 8;
  u32 reserved0 : 24;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_ipa_ipa_2x_perf11_n_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_ipa_ipa_2x_perf11_n_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_IPA_IPA_2X_PERF12_N_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_ipa_ipa_2x_perf12_n_dfsr_s
{
  u32 not_n_minus_m : 8;
  u32 reserved0 : 24;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_ipa_ipa_2x_perf12_n_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_ipa_ipa_2x_perf12_n_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_IPA_IPA_2X_PERF13_N_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_ipa_ipa_2x_perf13_n_dfsr_s
{
  u32 not_n_minus_m : 8;
  u32 reserved0 : 24;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_ipa_ipa_2x_perf13_n_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_ipa_ipa_2x_perf13_n_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_IPA_IPA_2X_PERF14_N_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_ipa_ipa_2x_perf14_n_dfsr_s
{
  u32 not_n_minus_m : 8;
  u32 reserved0 : 24;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_ipa_ipa_2x_perf14_n_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_ipa_ipa_2x_perf14_n_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_IPA_IPA_2X_PERF15_N_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_ipa_ipa_2x_perf15_n_dfsr_s
{
  u32 not_n_minus_m : 8;
  u32 reserved0 : 24;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_ipa_ipa_2x_perf15_n_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_ipa_ipa_2x_perf15_n_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_IPA_IPA_2X_PERF0_D_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_ipa_ipa_2x_perf0_d_dfsr_s
{
  u32 not_2d : 8;
  u32 reserved0 : 24;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_ipa_ipa_2x_perf0_d_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_ipa_ipa_2x_perf0_d_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_IPA_IPA_2X_PERF1_D_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_ipa_ipa_2x_perf1_d_dfsr_s
{
  u32 not_2d : 8;
  u32 reserved0 : 24;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_ipa_ipa_2x_perf1_d_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_ipa_ipa_2x_perf1_d_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_IPA_IPA_2X_PERF2_D_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_ipa_ipa_2x_perf2_d_dfsr_s
{
  u32 not_2d : 8;
  u32 reserved0 : 24;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_ipa_ipa_2x_perf2_d_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_ipa_ipa_2x_perf2_d_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_IPA_IPA_2X_PERF3_D_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_ipa_ipa_2x_perf3_d_dfsr_s
{
  u32 not_2d : 8;
  u32 reserved0 : 24;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_ipa_ipa_2x_perf3_d_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_ipa_ipa_2x_perf3_d_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_IPA_IPA_2X_PERF4_D_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_ipa_ipa_2x_perf4_d_dfsr_s
{
  u32 not_2d : 8;
  u32 reserved0 : 24;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_ipa_ipa_2x_perf4_d_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_ipa_ipa_2x_perf4_d_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_IPA_IPA_2X_PERF5_D_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_ipa_ipa_2x_perf5_d_dfsr_s
{
  u32 not_2d : 8;
  u32 reserved0 : 24;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_ipa_ipa_2x_perf5_d_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_ipa_ipa_2x_perf5_d_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_IPA_IPA_2X_PERF6_D_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_ipa_ipa_2x_perf6_d_dfsr_s
{
  u32 not_2d : 8;
  u32 reserved0 : 24;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_ipa_ipa_2x_perf6_d_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_ipa_ipa_2x_perf6_d_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_IPA_IPA_2X_PERF7_D_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_ipa_ipa_2x_perf7_d_dfsr_s
{
  u32 not_2d : 8;
  u32 reserved0 : 24;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_ipa_ipa_2x_perf7_d_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_ipa_ipa_2x_perf7_d_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_IPA_IPA_2X_PERF8_D_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_ipa_ipa_2x_perf8_d_dfsr_s
{
  u32 not_2d : 8;
  u32 reserved0 : 24;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_ipa_ipa_2x_perf8_d_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_ipa_ipa_2x_perf8_d_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_IPA_IPA_2X_PERF9_D_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_ipa_ipa_2x_perf9_d_dfsr_s
{
  u32 not_2d : 8;
  u32 reserved0 : 24;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_ipa_ipa_2x_perf9_d_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_ipa_ipa_2x_perf9_d_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_IPA_IPA_2X_PERF10_D_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_ipa_ipa_2x_perf10_d_dfsr_s
{
  u32 not_2d : 8;
  u32 reserved0 : 24;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_ipa_ipa_2x_perf10_d_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_ipa_ipa_2x_perf10_d_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_IPA_IPA_2X_PERF11_D_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_ipa_ipa_2x_perf11_d_dfsr_s
{
  u32 not_2d : 8;
  u32 reserved0 : 24;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_ipa_ipa_2x_perf11_d_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_ipa_ipa_2x_perf11_d_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_IPA_IPA_2X_PERF12_D_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_ipa_ipa_2x_perf12_d_dfsr_s
{
  u32 not_2d : 8;
  u32 reserved0 : 24;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_ipa_ipa_2x_perf12_d_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_ipa_ipa_2x_perf12_d_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_IPA_IPA_2X_PERF13_D_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_ipa_ipa_2x_perf13_d_dfsr_s
{
  u32 not_2d : 8;
  u32 reserved0 : 24;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_ipa_ipa_2x_perf13_d_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_ipa_ipa_2x_perf13_d_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_IPA_IPA_2X_PERF14_D_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_ipa_ipa_2x_perf14_d_dfsr_s
{
  u32 not_2d : 8;
  u32 reserved0 : 24;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_ipa_ipa_2x_perf14_d_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_ipa_ipa_2x_perf14_d_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_IPA_IPA_2X_PERF15_D_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_ipa_ipa_2x_perf15_d_dfsr_s
{
  u32 not_2d : 8;
  u32 reserved0 : 24;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_ipa_ipa_2x_perf15_d_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_ipa_ipa_2x_perf15_d_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_IPA_2X_CMD_RCGR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_ipa_2x_cmd_rcgr_s
{
  u32 update : 1;
  u32 root_en : 1;
  u32 reserved0 : 2;
  u32 dirty_cfg_rcgr : 1;
  u32 dirty_m : 1;
  u32 dirty_n : 1;
  u32 dirty_d : 1;
  u32 reserved1 : 23;
  u32 root_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_ipa_2x_cmd_rcgr_u
{
  struct ipa_gcc_hwio_def_gcc_ipa_2x_cmd_rcgr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_IPA_2X_CFG_RCGR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_ipa_2x_cfg_rcgr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 1;
  u32 mode : 2;
  u32 reserved2 : 6;
  u32 hw_clk_control : 1;
  u32 reserved3 : 11;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_ipa_2x_cfg_rcgr_u
{
  struct ipa_gcc_hwio_def_gcc_ipa_2x_cfg_rcgr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_IPA_2X_M
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_ipa_2x_m_s
{
  u32 m : 8;
  u32 reserved0 : 24;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_ipa_2x_m_u
{
  struct ipa_gcc_hwio_def_gcc_ipa_2x_m_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_IPA_2X_N
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_ipa_2x_n_s
{
  u32 not_n_minus_m : 8;
  u32 reserved0 : 24;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_ipa_2x_n_u
{
  struct ipa_gcc_hwio_def_gcc_ipa_2x_n_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_IPA_2X_D
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_ipa_2x_d_s
{
  u32 not_2d : 8;
  u32 reserved0 : 24;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_ipa_2x_d_u
{
  struct ipa_gcc_hwio_def_gcc_ipa_2x_d_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_IPA_CDIVR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_ipa_cdivr_s
{
  u32 clk_div : 4;
  u32 reserved0 : 28;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_ipa_cdivr_u
{
  struct ipa_gcc_hwio_def_gcc_ipa_cdivr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_QPIC_BCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_qpic_bcr_s
{
  u32 blk_ares : 1;
  u32 reserved0 : 31;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_qpic_bcr_u
{
  struct ipa_gcc_hwio_def_gcc_qpic_bcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_QPIC_CBCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_qpic_cbcr_s
{
  u32 clk_enable : 1;
  u32 reserved0 : 1;
  u32 clk_ares : 1;
  u32 reserved1 : 1;
  u32 sleep : 4;
  u32 wakeup : 4;
  u32 force_mem_periph_off : 1;
  u32 force_mem_periph_on : 1;
  u32 force_mem_core_on : 1;
  u32 reserved2 : 5;
  u32 ignore_rpmh_clk_dis : 1;
  u32 reserved3 : 1;
  u32 clk_dis : 1;
  u32 ignore_all_clk_dis : 1;
  u32 ignore_all_ares : 1;
  u32 reserved4 : 6;
  u32 clk_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_qpic_cbcr_u
{
  struct ipa_gcc_hwio_def_gcc_qpic_cbcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_QPIC_SREGR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_qpic_sregr_s
{
  u32 reserved0 : 1;
  u32 sw_clk_en_slp_stg : 1;
  u32 sw_clk_en_sel_slp_stg : 1;
  u32 sw_ctrl_pwr_down : 1;
  u32 sw_rst_slp_stg : 1;
  u32 sw_rst_sel_slp_stg : 1;
  u32 force_clk_on : 1;
  u32 mem_cph_enable : 1;
  u32 sw_div_ratio_slp_stg_clk : 2;
  u32 mem_periph_on_ack : 1;
  u32 mem_core_on_ack : 1;
  u32 sw_sm_pscbc_seq_in_override : 1;
  u32 mem_cph_rst_sw_override : 1;
  u32 pscbc_slp_stg_mode_csr : 1;
  u32 ignore_gdsc_pwr_dwn_csr : 1;
  u32 sreg_pscbc_spare_ctrl_in : 8;
  u32 sreg_pscbc_spare_ctrl_out : 8;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_qpic_sregr_u
{
  struct ipa_gcc_hwio_def_gcc_qpic_sregr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_QPIC_AHB_CBCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_qpic_ahb_cbcr_s
{
  u32 clk_enable : 1;
  u32 reserved0 : 1;
  u32 clk_ares : 1;
  u32 reserved1 : 17;
  u32 ignore_rpmh_clk_dis : 1;
  u32 reserved2 : 1;
  u32 clk_dis : 1;
  u32 ignore_all_clk_dis : 1;
  u32 ignore_all_ares : 1;
  u32 reserved3 : 6;
  u32 clk_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_qpic_ahb_cbcr_u
{
  struct ipa_gcc_hwio_def_gcc_qpic_ahb_cbcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_QPIC_SYSTEM_CBCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_qpic_system_cbcr_s
{
  u32 clk_enable : 1;
  u32 reserved0 : 1;
  u32 clk_ares : 1;
  u32 reserved1 : 19;
  u32 clk_dis : 1;
  u32 ignore_all_clk_dis : 1;
  u32 ignore_all_ares : 1;
  u32 reserved2 : 6;
  u32 clk_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_qpic_system_cbcr_u
{
  struct ipa_gcc_hwio_def_gcc_qpic_system_cbcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_QPIC_CMD_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_qpic_cmd_dfsr_s
{
  u32 dfs_en : 1;
  u32 curr_perf_state : 4;
  u32 hw_clk_control : 1;
  u32 dfs_fsm_state : 3;
  u32 perf_state_update_status : 1;
  u32 sw_override : 1;
  u32 sw_perf_state : 4;
  u32 rcg_sw_ctrl : 1;
  u32 reserved0 : 16;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_qpic_cmd_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_qpic_cmd_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_QPIC_QPIC_PERF0_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_qpic_qpic_perf0_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 1;
  u32 mode : 2;
  u32 reserved2 : 18;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_qpic_qpic_perf0_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_qpic_qpic_perf0_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_QPIC_QPIC_PERF1_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_qpic_qpic_perf1_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 1;
  u32 mode : 2;
  u32 reserved2 : 18;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_qpic_qpic_perf1_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_qpic_qpic_perf1_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_QPIC_QPIC_PERF2_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_qpic_qpic_perf2_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 1;
  u32 mode : 2;
  u32 reserved2 : 18;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_qpic_qpic_perf2_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_qpic_qpic_perf2_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_QPIC_QPIC_PERF3_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_qpic_qpic_perf3_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 1;
  u32 mode : 2;
  u32 reserved2 : 18;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_qpic_qpic_perf3_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_qpic_qpic_perf3_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_QPIC_QPIC_PERF4_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_qpic_qpic_perf4_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 1;
  u32 mode : 2;
  u32 reserved2 : 18;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_qpic_qpic_perf4_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_qpic_qpic_perf4_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_QPIC_QPIC_PERF5_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_qpic_qpic_perf5_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 1;
  u32 mode : 2;
  u32 reserved2 : 18;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_qpic_qpic_perf5_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_qpic_qpic_perf5_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_QPIC_QPIC_PERF6_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_qpic_qpic_perf6_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 1;
  u32 mode : 2;
  u32 reserved2 : 18;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_qpic_qpic_perf6_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_qpic_qpic_perf6_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_QPIC_QPIC_PERF7_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_qpic_qpic_perf7_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 1;
  u32 mode : 2;
  u32 reserved2 : 18;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_qpic_qpic_perf7_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_qpic_qpic_perf7_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_QPIC_QPIC_PERF8_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_qpic_qpic_perf8_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 1;
  u32 mode : 2;
  u32 reserved2 : 18;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_qpic_qpic_perf8_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_qpic_qpic_perf8_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_QPIC_QPIC_PERF9_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_qpic_qpic_perf9_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 1;
  u32 mode : 2;
  u32 reserved2 : 18;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_qpic_qpic_perf9_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_qpic_qpic_perf9_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_QPIC_QPIC_PERF10_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_qpic_qpic_perf10_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 1;
  u32 mode : 2;
  u32 reserved2 : 18;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_qpic_qpic_perf10_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_qpic_qpic_perf10_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_QPIC_QPIC_PERF11_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_qpic_qpic_perf11_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 1;
  u32 mode : 2;
  u32 reserved2 : 18;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_qpic_qpic_perf11_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_qpic_qpic_perf11_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_QPIC_QPIC_PERF12_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_qpic_qpic_perf12_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 1;
  u32 mode : 2;
  u32 reserved2 : 18;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_qpic_qpic_perf12_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_qpic_qpic_perf12_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_QPIC_QPIC_PERF13_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_qpic_qpic_perf13_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 1;
  u32 mode : 2;
  u32 reserved2 : 18;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_qpic_qpic_perf13_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_qpic_qpic_perf13_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_QPIC_QPIC_PERF14_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_qpic_qpic_perf14_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 1;
  u32 mode : 2;
  u32 reserved2 : 18;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_qpic_qpic_perf14_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_qpic_qpic_perf14_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_QPIC_QPIC_PERF15_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_qpic_qpic_perf15_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 1;
  u32 mode : 2;
  u32 reserved2 : 18;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_qpic_qpic_perf15_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_qpic_qpic_perf15_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_QPIC_QPIC_PERF0_M_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_qpic_qpic_perf0_m_dfsr_s
{
  u32 m : 8;
  u32 reserved0 : 24;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_qpic_qpic_perf0_m_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_qpic_qpic_perf0_m_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_QPIC_QPIC_PERF1_M_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_qpic_qpic_perf1_m_dfsr_s
{
  u32 m : 8;
  u32 reserved0 : 24;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_qpic_qpic_perf1_m_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_qpic_qpic_perf1_m_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_QPIC_QPIC_PERF2_M_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_qpic_qpic_perf2_m_dfsr_s
{
  u32 m : 8;
  u32 reserved0 : 24;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_qpic_qpic_perf2_m_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_qpic_qpic_perf2_m_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_QPIC_QPIC_PERF3_M_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_qpic_qpic_perf3_m_dfsr_s
{
  u32 m : 8;
  u32 reserved0 : 24;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_qpic_qpic_perf3_m_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_qpic_qpic_perf3_m_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_QPIC_QPIC_PERF4_M_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_qpic_qpic_perf4_m_dfsr_s
{
  u32 m : 8;
  u32 reserved0 : 24;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_qpic_qpic_perf4_m_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_qpic_qpic_perf4_m_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_QPIC_QPIC_PERF5_M_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_qpic_qpic_perf5_m_dfsr_s
{
  u32 m : 8;
  u32 reserved0 : 24;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_qpic_qpic_perf5_m_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_qpic_qpic_perf5_m_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_QPIC_QPIC_PERF6_M_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_qpic_qpic_perf6_m_dfsr_s
{
  u32 m : 8;
  u32 reserved0 : 24;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_qpic_qpic_perf6_m_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_qpic_qpic_perf6_m_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_QPIC_QPIC_PERF7_M_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_qpic_qpic_perf7_m_dfsr_s
{
  u32 m : 8;
  u32 reserved0 : 24;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_qpic_qpic_perf7_m_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_qpic_qpic_perf7_m_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_QPIC_QPIC_PERF8_M_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_qpic_qpic_perf8_m_dfsr_s
{
  u32 m : 8;
  u32 reserved0 : 24;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_qpic_qpic_perf8_m_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_qpic_qpic_perf8_m_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_QPIC_QPIC_PERF9_M_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_qpic_qpic_perf9_m_dfsr_s
{
  u32 m : 8;
  u32 reserved0 : 24;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_qpic_qpic_perf9_m_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_qpic_qpic_perf9_m_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_QPIC_QPIC_PERF10_M_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_qpic_qpic_perf10_m_dfsr_s
{
  u32 m : 8;
  u32 reserved0 : 24;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_qpic_qpic_perf10_m_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_qpic_qpic_perf10_m_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_QPIC_QPIC_PERF11_M_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_qpic_qpic_perf11_m_dfsr_s
{
  u32 m : 8;
  u32 reserved0 : 24;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_qpic_qpic_perf11_m_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_qpic_qpic_perf11_m_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_QPIC_QPIC_PERF12_M_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_qpic_qpic_perf12_m_dfsr_s
{
  u32 m : 8;
  u32 reserved0 : 24;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_qpic_qpic_perf12_m_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_qpic_qpic_perf12_m_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_QPIC_QPIC_PERF13_M_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_qpic_qpic_perf13_m_dfsr_s
{
  u32 m : 8;
  u32 reserved0 : 24;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_qpic_qpic_perf13_m_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_qpic_qpic_perf13_m_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_QPIC_QPIC_PERF14_M_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_qpic_qpic_perf14_m_dfsr_s
{
  u32 m : 8;
  u32 reserved0 : 24;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_qpic_qpic_perf14_m_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_qpic_qpic_perf14_m_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_QPIC_QPIC_PERF15_M_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_qpic_qpic_perf15_m_dfsr_s
{
  u32 m : 8;
  u32 reserved0 : 24;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_qpic_qpic_perf15_m_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_qpic_qpic_perf15_m_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_QPIC_QPIC_PERF0_N_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_qpic_qpic_perf0_n_dfsr_s
{
  u32 not_n_minus_m : 8;
  u32 reserved0 : 24;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_qpic_qpic_perf0_n_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_qpic_qpic_perf0_n_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_QPIC_QPIC_PERF1_N_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_qpic_qpic_perf1_n_dfsr_s
{
  u32 not_n_minus_m : 8;
  u32 reserved0 : 24;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_qpic_qpic_perf1_n_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_qpic_qpic_perf1_n_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_QPIC_QPIC_PERF2_N_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_qpic_qpic_perf2_n_dfsr_s
{
  u32 not_n_minus_m : 8;
  u32 reserved0 : 24;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_qpic_qpic_perf2_n_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_qpic_qpic_perf2_n_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_QPIC_QPIC_PERF3_N_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_qpic_qpic_perf3_n_dfsr_s
{
  u32 not_n_minus_m : 8;
  u32 reserved0 : 24;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_qpic_qpic_perf3_n_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_qpic_qpic_perf3_n_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_QPIC_QPIC_PERF4_N_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_qpic_qpic_perf4_n_dfsr_s
{
  u32 not_n_minus_m : 8;
  u32 reserved0 : 24;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_qpic_qpic_perf4_n_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_qpic_qpic_perf4_n_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_QPIC_QPIC_PERF5_N_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_qpic_qpic_perf5_n_dfsr_s
{
  u32 not_n_minus_m : 8;
  u32 reserved0 : 24;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_qpic_qpic_perf5_n_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_qpic_qpic_perf5_n_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_QPIC_QPIC_PERF6_N_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_qpic_qpic_perf6_n_dfsr_s
{
  u32 not_n_minus_m : 8;
  u32 reserved0 : 24;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_qpic_qpic_perf6_n_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_qpic_qpic_perf6_n_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_QPIC_QPIC_PERF7_N_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_qpic_qpic_perf7_n_dfsr_s
{
  u32 not_n_minus_m : 8;
  u32 reserved0 : 24;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_qpic_qpic_perf7_n_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_qpic_qpic_perf7_n_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_QPIC_QPIC_PERF8_N_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_qpic_qpic_perf8_n_dfsr_s
{
  u32 not_n_minus_m : 8;
  u32 reserved0 : 24;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_qpic_qpic_perf8_n_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_qpic_qpic_perf8_n_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_QPIC_QPIC_PERF9_N_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_qpic_qpic_perf9_n_dfsr_s
{
  u32 not_n_minus_m : 8;
  u32 reserved0 : 24;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_qpic_qpic_perf9_n_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_qpic_qpic_perf9_n_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_QPIC_QPIC_PERF10_N_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_qpic_qpic_perf10_n_dfsr_s
{
  u32 not_n_minus_m : 8;
  u32 reserved0 : 24;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_qpic_qpic_perf10_n_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_qpic_qpic_perf10_n_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_QPIC_QPIC_PERF11_N_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_qpic_qpic_perf11_n_dfsr_s
{
  u32 not_n_minus_m : 8;
  u32 reserved0 : 24;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_qpic_qpic_perf11_n_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_qpic_qpic_perf11_n_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_QPIC_QPIC_PERF12_N_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_qpic_qpic_perf12_n_dfsr_s
{
  u32 not_n_minus_m : 8;
  u32 reserved0 : 24;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_qpic_qpic_perf12_n_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_qpic_qpic_perf12_n_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_QPIC_QPIC_PERF13_N_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_qpic_qpic_perf13_n_dfsr_s
{
  u32 not_n_minus_m : 8;
  u32 reserved0 : 24;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_qpic_qpic_perf13_n_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_qpic_qpic_perf13_n_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_QPIC_QPIC_PERF14_N_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_qpic_qpic_perf14_n_dfsr_s
{
  u32 not_n_minus_m : 8;
  u32 reserved0 : 24;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_qpic_qpic_perf14_n_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_qpic_qpic_perf14_n_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_QPIC_QPIC_PERF15_N_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_qpic_qpic_perf15_n_dfsr_s
{
  u32 not_n_minus_m : 8;
  u32 reserved0 : 24;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_qpic_qpic_perf15_n_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_qpic_qpic_perf15_n_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_QPIC_QPIC_PERF0_D_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_qpic_qpic_perf0_d_dfsr_s
{
  u32 not_2d : 8;
  u32 reserved0 : 24;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_qpic_qpic_perf0_d_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_qpic_qpic_perf0_d_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_QPIC_QPIC_PERF1_D_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_qpic_qpic_perf1_d_dfsr_s
{
  u32 not_2d : 8;
  u32 reserved0 : 24;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_qpic_qpic_perf1_d_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_qpic_qpic_perf1_d_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_QPIC_QPIC_PERF2_D_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_qpic_qpic_perf2_d_dfsr_s
{
  u32 not_2d : 8;
  u32 reserved0 : 24;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_qpic_qpic_perf2_d_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_qpic_qpic_perf2_d_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_QPIC_QPIC_PERF3_D_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_qpic_qpic_perf3_d_dfsr_s
{
  u32 not_2d : 8;
  u32 reserved0 : 24;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_qpic_qpic_perf3_d_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_qpic_qpic_perf3_d_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_QPIC_QPIC_PERF4_D_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_qpic_qpic_perf4_d_dfsr_s
{
  u32 not_2d : 8;
  u32 reserved0 : 24;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_qpic_qpic_perf4_d_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_qpic_qpic_perf4_d_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_QPIC_QPIC_PERF5_D_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_qpic_qpic_perf5_d_dfsr_s
{
  u32 not_2d : 8;
  u32 reserved0 : 24;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_qpic_qpic_perf5_d_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_qpic_qpic_perf5_d_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_QPIC_QPIC_PERF6_D_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_qpic_qpic_perf6_d_dfsr_s
{
  u32 not_2d : 8;
  u32 reserved0 : 24;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_qpic_qpic_perf6_d_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_qpic_qpic_perf6_d_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_QPIC_QPIC_PERF7_D_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_qpic_qpic_perf7_d_dfsr_s
{
  u32 not_2d : 8;
  u32 reserved0 : 24;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_qpic_qpic_perf7_d_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_qpic_qpic_perf7_d_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_QPIC_QPIC_PERF8_D_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_qpic_qpic_perf8_d_dfsr_s
{
  u32 not_2d : 8;
  u32 reserved0 : 24;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_qpic_qpic_perf8_d_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_qpic_qpic_perf8_d_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_QPIC_QPIC_PERF9_D_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_qpic_qpic_perf9_d_dfsr_s
{
  u32 not_2d : 8;
  u32 reserved0 : 24;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_qpic_qpic_perf9_d_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_qpic_qpic_perf9_d_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_QPIC_QPIC_PERF10_D_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_qpic_qpic_perf10_d_dfsr_s
{
  u32 not_2d : 8;
  u32 reserved0 : 24;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_qpic_qpic_perf10_d_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_qpic_qpic_perf10_d_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_QPIC_QPIC_PERF11_D_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_qpic_qpic_perf11_d_dfsr_s
{
  u32 not_2d : 8;
  u32 reserved0 : 24;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_qpic_qpic_perf11_d_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_qpic_qpic_perf11_d_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_QPIC_QPIC_PERF12_D_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_qpic_qpic_perf12_d_dfsr_s
{
  u32 not_2d : 8;
  u32 reserved0 : 24;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_qpic_qpic_perf12_d_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_qpic_qpic_perf12_d_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_QPIC_QPIC_PERF13_D_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_qpic_qpic_perf13_d_dfsr_s
{
  u32 not_2d : 8;
  u32 reserved0 : 24;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_qpic_qpic_perf13_d_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_qpic_qpic_perf13_d_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_QPIC_QPIC_PERF14_D_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_qpic_qpic_perf14_d_dfsr_s
{
  u32 not_2d : 8;
  u32 reserved0 : 24;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_qpic_qpic_perf14_d_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_qpic_qpic_perf14_d_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_QPIC_QPIC_PERF15_D_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_qpic_qpic_perf15_d_dfsr_s
{
  u32 not_2d : 8;
  u32 reserved0 : 24;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_qpic_qpic_perf15_d_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_qpic_qpic_perf15_d_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_QPIC_CMD_RCGR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_qpic_cmd_rcgr_s
{
  u32 update : 1;
  u32 root_en : 1;
  u32 reserved0 : 2;
  u32 dirty_cfg_rcgr : 1;
  u32 dirty_m : 1;
  u32 dirty_n : 1;
  u32 dirty_d : 1;
  u32 reserved1 : 23;
  u32 root_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_qpic_cmd_rcgr_u
{
  struct ipa_gcc_hwio_def_gcc_qpic_cmd_rcgr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_QPIC_CFG_RCGR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_qpic_cfg_rcgr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 1;
  u32 mode : 2;
  u32 reserved2 : 6;
  u32 hw_clk_control : 1;
  u32 reserved3 : 11;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_qpic_cfg_rcgr_u
{
  struct ipa_gcc_hwio_def_gcc_qpic_cfg_rcgr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_QPIC_M
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_qpic_m_s
{
  u32 m : 8;
  u32 reserved0 : 24;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_qpic_m_u
{
  struct ipa_gcc_hwio_def_gcc_qpic_m_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_QPIC_N
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_qpic_n_s
{
  u32 not_n_minus_m : 8;
  u32 reserved0 : 24;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_qpic_n_u
{
  struct ipa_gcc_hwio_def_gcc_qpic_n_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_QPIC_D
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_qpic_d_s
{
  u32 not_2d : 8;
  u32 reserved0 : 24;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_qpic_d_u
{
  struct ipa_gcc_hwio_def_gcc_qpic_d_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_SPMI_FETCHER_BCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_spmi_fetcher_bcr_s
{
  u32 blk_ares : 1;
  u32 reserved0 : 31;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_spmi_fetcher_bcr_u
{
  struct ipa_gcc_hwio_def_gcc_spmi_fetcher_bcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_SPMI_FETCHER_CBCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_spmi_fetcher_cbcr_s
{
  u32 clk_enable : 1;
  u32 reserved0 : 1;
  u32 clk_ares : 1;
  u32 reserved1 : 19;
  u32 clk_dis : 1;
  u32 ignore_all_clk_dis : 1;
  u32 ignore_all_ares : 1;
  u32 reserved2 : 6;
  u32 clk_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_spmi_fetcher_cbcr_u
{
  struct ipa_gcc_hwio_def_gcc_spmi_fetcher_cbcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_SPMI_FETCHER_AHB_CBCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_spmi_fetcher_ahb_cbcr_s
{
  u32 clk_enable : 1;
  u32 reserved0 : 1;
  u32 clk_ares : 1;
  u32 reserved1 : 17;
  u32 ignore_rpmh_clk_dis : 1;
  u32 reserved2 : 1;
  u32 clk_dis : 1;
  u32 ignore_all_clk_dis : 1;
  u32 ignore_all_ares : 1;
  u32 reserved3 : 6;
  u32 clk_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_spmi_fetcher_ahb_cbcr_u
{
  struct ipa_gcc_hwio_def_gcc_spmi_fetcher_ahb_cbcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_SPMI_FETCHER_CMD_RCGR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_spmi_fetcher_cmd_rcgr_s
{
  u32 update : 1;
  u32 root_en : 1;
  u32 reserved0 : 2;
  u32 dirty_cfg_rcgr : 1;
  u32 reserved1 : 26;
  u32 root_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_spmi_fetcher_cmd_rcgr_u
{
  struct ipa_gcc_hwio_def_gcc_spmi_fetcher_cmd_rcgr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_SPMI_FETCHER_CFG_RCGR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_spmi_fetcher_cfg_rcgr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 5;
  u32 rcglite_disable : 1;
  u32 reserved2 : 3;
  u32 hw_clk_control : 1;
  u32 reserved3 : 11;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_spmi_fetcher_cfg_rcgr_u
{
  struct ipa_gcc_hwio_def_gcc_spmi_fetcher_cfg_rcgr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_MSS_CFG_AHB_CBCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_mss_cfg_ahb_cbcr_s
{
  u32 clk_enable : 1;
  u32 hw_ctl : 1;
  u32 clk_ares : 1;
  u32 sw_only_en : 1;
  u32 reserved0 : 16;
  u32 ignore_rpmh_clk_dis : 1;
  u32 reserved1 : 1;
  u32 clk_dis : 1;
  u32 ignore_all_clk_dis : 1;
  u32 ignore_all_ares : 1;
  u32 reserved2 : 6;
  u32 clk_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_mss_cfg_ahb_cbcr_u
{
  struct ipa_gcc_hwio_def_gcc_mss_cfg_ahb_cbcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_MSS_OFFLINE_AXI_CBCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_mss_offline_axi_cbcr_s
{
  u32 clk_enable : 1;
  u32 hw_ctl : 1;
  u32 clk_ares : 1;
  u32 sw_only_en : 1;
  u32 reserved0 : 16;
  u32 ignore_rpmh_clk_dis : 1;
  u32 reserved1 : 1;
  u32 clk_dis : 1;
  u32 ignore_all_clk_dis : 1;
  u32 ignore_all_ares : 1;
  u32 reserved2 : 6;
  u32 clk_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_mss_offline_axi_cbcr_u
{
  struct ipa_gcc_hwio_def_gcc_mss_offline_axi_cbcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_MSS_CE_AXI_CBCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_mss_ce_axi_cbcr_s
{
  u32 clk_enable : 1;
  u32 hw_ctl : 1;
  u32 clk_ares : 1;
  u32 sw_only_en : 1;
  u32 reserved0 : 16;
  u32 ignore_rpmh_clk_dis : 1;
  u32 reserved1 : 1;
  u32 clk_dis : 1;
  u32 ignore_all_clk_dis : 1;
  u32 ignore_all_ares : 1;
  u32 reserved2 : 6;
  u32 clk_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_mss_ce_axi_cbcr_u
{
  struct ipa_gcc_hwio_def_gcc_mss_ce_axi_cbcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_MSS_TRIG_CBCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_mss_trig_cbcr_s
{
  u32 clk_enable : 1;
  u32 hw_ctl : 1;
  u32 clk_ares : 1;
  u32 sw_only_en : 1;
  u32 reserved0 : 16;
  u32 ignore_rpmh_clk_dis : 1;
  u32 reserved1 : 1;
  u32 clk_dis : 1;
  u32 ignore_all_clk_dis : 1;
  u32 ignore_all_ares : 1;
  u32 reserved2 : 6;
  u32 clk_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_mss_trig_cbcr_u
{
  struct ipa_gcc_hwio_def_gcc_mss_trig_cbcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_MSS_AT_CBCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_mss_at_cbcr_s
{
  u32 clk_enable : 1;
  u32 hw_ctl : 1;
  u32 clk_ares : 1;
  u32 sw_only_en : 1;
  u32 reserved0 : 16;
  u32 ignore_rpmh_clk_dis : 1;
  u32 reserved1 : 1;
  u32 clk_dis : 1;
  u32 ignore_all_clk_dis : 1;
  u32 ignore_all_ares : 1;
  u32 reserved2 : 6;
  u32 clk_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_mss_at_cbcr_u
{
  struct ipa_gcc_hwio_def_gcc_mss_at_cbcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_MSS_PLL0_MAIN_DIV_CDIVR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_mss_pll0_main_div_cdivr_s
{
  u32 clk_div : 4;
  u32 reserved0 : 28;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_mss_pll0_main_div_cdivr_u
{
  struct ipa_gcc_hwio_def_gcc_mss_pll0_main_div_cdivr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_SHUB_MSS_MCDMA_MEMNOC_PERF0_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_shub_mss_mcdma_memnoc_perf0_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 21;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_shub_mss_mcdma_memnoc_perf0_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_shub_mss_mcdma_memnoc_perf0_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_SHUB_MSS_MCDMA_MEMNOC_PERF1_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_shub_mss_mcdma_memnoc_perf1_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 21;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_shub_mss_mcdma_memnoc_perf1_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_shub_mss_mcdma_memnoc_perf1_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_SHUB_MSS_MCDMA_MEMNOC_PERF2_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_shub_mss_mcdma_memnoc_perf2_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 21;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_shub_mss_mcdma_memnoc_perf2_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_shub_mss_mcdma_memnoc_perf2_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_SHUB_MSS_MCDMA_MEMNOC_PERF3_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_shub_mss_mcdma_memnoc_perf3_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 21;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_shub_mss_mcdma_memnoc_perf3_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_shub_mss_mcdma_memnoc_perf3_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_SHUB_MSS_MCDMA_MEMNOC_PERF4_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_shub_mss_mcdma_memnoc_perf4_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 21;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_shub_mss_mcdma_memnoc_perf4_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_shub_mss_mcdma_memnoc_perf4_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_SHUB_MSS_MCDMA_MEMNOC_PERF5_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_shub_mss_mcdma_memnoc_perf5_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 21;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_shub_mss_mcdma_memnoc_perf5_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_shub_mss_mcdma_memnoc_perf5_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_SHUB_MSS_MCDMA_MEMNOC_PERF6_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_shub_mss_mcdma_memnoc_perf6_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 21;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_shub_mss_mcdma_memnoc_perf6_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_shub_mss_mcdma_memnoc_perf6_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_SHUB_MSS_MCDMA_MEMNOC_PERF7_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_shub_mss_mcdma_memnoc_perf7_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 21;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_shub_mss_mcdma_memnoc_perf7_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_shub_mss_mcdma_memnoc_perf7_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_SHUB_MSS_MCDMA_MEMNOC_PERF8_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_shub_mss_mcdma_memnoc_perf8_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 21;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_shub_mss_mcdma_memnoc_perf8_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_shub_mss_mcdma_memnoc_perf8_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_SHUB_MSS_MCDMA_MEMNOC_PERF9_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_shub_mss_mcdma_memnoc_perf9_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 21;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_shub_mss_mcdma_memnoc_perf9_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_shub_mss_mcdma_memnoc_perf9_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_SHUB_MSS_MCDMA_MEMNOC_PERF10_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_shub_mss_mcdma_memnoc_perf10_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 21;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_shub_mss_mcdma_memnoc_perf10_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_shub_mss_mcdma_memnoc_perf10_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_SHUB_MSS_MCDMA_MEMNOC_PERF11_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_shub_mss_mcdma_memnoc_perf11_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 21;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_shub_mss_mcdma_memnoc_perf11_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_shub_mss_mcdma_memnoc_perf11_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_SHUB_MSS_MCDMA_MEMNOC_PERF12_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_shub_mss_mcdma_memnoc_perf12_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 21;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_shub_mss_mcdma_memnoc_perf12_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_shub_mss_mcdma_memnoc_perf12_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_SHUB_MSS_MCDMA_MEMNOC_PERF13_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_shub_mss_mcdma_memnoc_perf13_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 21;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_shub_mss_mcdma_memnoc_perf13_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_shub_mss_mcdma_memnoc_perf13_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_SHUB_MSS_MCDMA_MEMNOC_PERF14_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_shub_mss_mcdma_memnoc_perf14_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 21;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_shub_mss_mcdma_memnoc_perf14_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_shub_mss_mcdma_memnoc_perf14_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_SHUB_MSS_MCDMA_MEMNOC_PERF15_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_shub_mss_mcdma_memnoc_perf15_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 21;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_shub_mss_mcdma_memnoc_perf15_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_shub_mss_mcdma_memnoc_perf15_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_MSS_MCDMA_MEMNOC_CMD_RCGR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_mss_mcdma_memnoc_cmd_rcgr_s
{
  u32 update : 1;
  u32 root_en : 1;
  u32 reserved0 : 2;
  u32 dirty_cfg_rcgr : 1;
  u32 reserved1 : 26;
  u32 root_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_mss_mcdma_memnoc_cmd_rcgr_u
{
  struct ipa_gcc_hwio_def_gcc_mss_mcdma_memnoc_cmd_rcgr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_MSS_MCDMA_MEMNOC_CFG_RCGR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_mss_mcdma_memnoc_cfg_rcgr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 5;
  u32 rcglite_disable : 1;
  u32 reserved2 : 3;
  u32 hw_clk_control : 1;
  u32 reserved3 : 11;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_mss_mcdma_memnoc_cfg_rcgr_u
{
  struct ipa_gcc_hwio_def_gcc_mss_mcdma_memnoc_cfg_rcgr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_MSS_SNOC_AXI_CBCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_mss_snoc_axi_cbcr_s
{
  u32 clk_enable : 1;
  u32 reserved0 : 1;
  u32 clk_ares : 1;
  u32 reserved1 : 17;
  u32 ignore_rpmh_clk_dis : 1;
  u32 reserved2 : 1;
  u32 clk_dis : 1;
  u32 ignore_all_clk_dis : 1;
  u32 ignore_all_ares : 1;
  u32 reserved3 : 6;
  u32 clk_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_mss_snoc_axi_cbcr_u
{
  struct ipa_gcc_hwio_def_gcc_mss_snoc_axi_cbcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_MSS_Q6VQ6_AXIM1_CBCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_mss_q6vq6_axim1_cbcr_s
{
  u32 clk_enable : 1;
  u32 hw_ctl : 1;
  u32 clk_ares : 1;
  u32 sw_only_en : 1;
  u32 reserved0 : 16;
  u32 ignore_rpmh_clk_dis : 1;
  u32 reserved1 : 1;
  u32 clk_dis : 1;
  u32 ignore_all_clk_dis : 1;
  u32 ignore_all_ares : 1;
  u32 reserved2 : 6;
  u32 clk_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_mss_q6vq6_axim1_cbcr_u
{
  struct ipa_gcc_hwio_def_gcc_mss_q6vq6_axim1_cbcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_QREFS_VBG_CAL_BCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_qrefs_vbg_cal_bcr_s
{
  u32 blk_ares : 1;
  u32 reserved0 : 31;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_qrefs_vbg_cal_bcr_u
{
  struct ipa_gcc_hwio_def_gcc_qrefs_vbg_cal_bcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_QREFS_VBG_CAL_CBCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_qrefs_vbg_cal_cbcr_s
{
  u32 clk_enable : 1;
  u32 reserved0 : 1;
  u32 clk_ares : 1;
  u32 reserved1 : 19;
  u32 clk_dis : 1;
  u32 ignore_all_clk_dis : 1;
  u32 ignore_all_ares : 1;
  u32 reserved2 : 6;
  u32 clk_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_qrefs_vbg_cal_cbcr_u
{
  struct ipa_gcc_hwio_def_gcc_qrefs_vbg_cal_cbcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_NAV_BCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_nav_bcr_s
{
  u32 blk_ares : 1;
  u32 reserved0 : 31;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_nav_bcr_u
{
  struct ipa_gcc_hwio_def_gcc_nav_bcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_NAV_SNOC_AXI_CBCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_nav_snoc_axi_cbcr_s
{
  u32 clk_enable : 1;
  u32 reserved0 : 1;
  u32 clk_ares : 1;
  u32 reserved1 : 17;
  u32 ignore_rpmh_clk_dis : 1;
  u32 reserved2 : 1;
  u32 clk_dis : 1;
  u32 ignore_all_clk_dis : 1;
  u32 ignore_all_ares : 1;
  u32 reserved3 : 6;
  u32 clk_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_nav_snoc_axi_cbcr_u
{
  struct ipa_gcc_hwio_def_gcc_nav_snoc_axi_cbcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_GPLL4_OUT_EVEN_DIV_CDIVR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_gpll4_out_even_div_cdivr_s
{
  u32 clk_div : 4;
  u32 reserved0 : 28;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_gpll4_out_even_div_cdivr_u
{
  struct ipa_gcc_hwio_def_gcc_gpll4_out_even_div_cdivr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_CM_PHY_REFGEN1_BCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_cm_phy_refgen1_bcr_s
{
  u32 blk_ares : 1;
  u32 reserved0 : 31;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_cm_phy_refgen1_bcr_u
{
  struct ipa_gcc_hwio_def_gcc_cm_phy_refgen1_bcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_CM_PHY_REFGEN1_CBCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_cm_phy_refgen1_cbcr_s
{
  u32 clk_enable : 1;
  u32 hw_ctl : 1;
  u32 clk_ares : 1;
  u32 sw_only_en : 1;
  u32 reserved0 : 18;
  u32 clk_dis : 1;
  u32 ignore_all_clk_dis : 1;
  u32 ignore_all_ares : 1;
  u32 reserved1 : 6;
  u32 clk_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_cm_phy_refgen1_cbcr_u
{
  struct ipa_gcc_hwio_def_gcc_cm_phy_refgen1_cbcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_ECC_BCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_ecc_bcr_s
{
  u32 blk_ares : 1;
  u32 reserved0 : 31;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_ecc_bcr_u
{
  struct ipa_gcc_hwio_def_gcc_ecc_bcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_ECC_CBCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_ecc_cbcr_s
{
  u32 clk_enable : 1;
  u32 hw_ctl : 1;
  u32 clk_ares : 1;
  u32 sw_only_en : 1;
  u32 reserved0 : 16;
  u32 ignore_rpmh_clk_dis : 1;
  u32 reserved1 : 1;
  u32 clk_dis : 1;
  u32 ignore_all_clk_dis : 1;
  u32 ignore_all_ares : 1;
  u32 reserved2 : 6;
  u32 clk_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_ecc_cbcr_u
{
  struct ipa_gcc_hwio_def_gcc_ecc_cbcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_ECC_CORE_CBCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_ecc_core_cbcr_s
{
  u32 clk_enable : 1;
  u32 reserved0 : 1;
  u32 clk_ares : 1;
  u32 reserved1 : 1;
  u32 sleep : 4;
  u32 wakeup : 4;
  u32 force_mem_periph_off : 1;
  u32 force_mem_periph_on : 1;
  u32 force_mem_core_on : 1;
  u32 reserved2 : 5;
  u32 ignore_rpmh_clk_dis : 1;
  u32 reserved3 : 1;
  u32 clk_dis : 1;
  u32 ignore_all_clk_dis : 1;
  u32 ignore_all_ares : 1;
  u32 reserved4 : 6;
  u32 clk_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_ecc_core_cbcr_u
{
  struct ipa_gcc_hwio_def_gcc_ecc_core_cbcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_ECC_CORE_SREGR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_ecc_core_sregr_s
{
  u32 reserved0 : 1;
  u32 sw_clk_en_slp_stg : 1;
  u32 sw_clk_en_sel_slp_stg : 1;
  u32 sw_ctrl_pwr_down : 1;
  u32 sw_rst_slp_stg : 1;
  u32 sw_rst_sel_slp_stg : 1;
  u32 force_clk_on : 1;
  u32 mem_cph_enable : 1;
  u32 sw_div_ratio_slp_stg_clk : 2;
  u32 mem_periph_on_ack : 1;
  u32 mem_core_on_ack : 1;
  u32 sw_sm_pscbc_seq_in_override : 1;
  u32 mem_cph_rst_sw_override : 1;
  u32 pscbc_slp_stg_mode_csr : 1;
  u32 ignore_gdsc_pwr_dwn_csr : 1;
  u32 sreg_pscbc_spare_ctrl_in : 8;
  u32 sreg_pscbc_spare_ctrl_out : 8;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_ecc_core_sregr_u
{
  struct ipa_gcc_hwio_def_gcc_ecc_core_sregr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_PKA_CMD_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_pka_cmd_dfsr_s
{
  u32 dfs_en : 1;
  u32 curr_perf_state : 4;
  u32 hw_clk_control : 1;
  u32 dfs_fsm_state : 3;
  u32 perf_state_update_status : 1;
  u32 sw_override : 1;
  u32 sw_perf_state : 4;
  u32 rcg_sw_ctrl : 1;
  u32 reserved0 : 16;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_pka_cmd_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_pka_cmd_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_PKA_ECC_PERF0_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_pka_ecc_perf0_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 21;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_pka_ecc_perf0_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_pka_ecc_perf0_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_PKA_ECC_PERF1_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_pka_ecc_perf1_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 21;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_pka_ecc_perf1_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_pka_ecc_perf1_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_PKA_ECC_PERF2_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_pka_ecc_perf2_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 21;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_pka_ecc_perf2_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_pka_ecc_perf2_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_PKA_ECC_PERF3_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_pka_ecc_perf3_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 21;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_pka_ecc_perf3_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_pka_ecc_perf3_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_PKA_ECC_PERF4_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_pka_ecc_perf4_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 21;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_pka_ecc_perf4_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_pka_ecc_perf4_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_PKA_ECC_PERF5_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_pka_ecc_perf5_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 21;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_pka_ecc_perf5_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_pka_ecc_perf5_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_PKA_ECC_PERF6_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_pka_ecc_perf6_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 21;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_pka_ecc_perf6_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_pka_ecc_perf6_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_PKA_ECC_PERF7_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_pka_ecc_perf7_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 21;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_pka_ecc_perf7_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_pka_ecc_perf7_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_PKA_ECC_PERF8_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_pka_ecc_perf8_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 21;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_pka_ecc_perf8_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_pka_ecc_perf8_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_PKA_ECC_PERF9_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_pka_ecc_perf9_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 21;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_pka_ecc_perf9_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_pka_ecc_perf9_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_PKA_ECC_PERF10_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_pka_ecc_perf10_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 21;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_pka_ecc_perf10_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_pka_ecc_perf10_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_PKA_ECC_PERF11_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_pka_ecc_perf11_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 21;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_pka_ecc_perf11_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_pka_ecc_perf11_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_PKA_ECC_PERF12_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_pka_ecc_perf12_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 21;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_pka_ecc_perf12_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_pka_ecc_perf12_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_PKA_ECC_PERF13_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_pka_ecc_perf13_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 21;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_pka_ecc_perf13_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_pka_ecc_perf13_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_PKA_ECC_PERF14_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_pka_ecc_perf14_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 21;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_pka_ecc_perf14_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_pka_ecc_perf14_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_PKA_ECC_PERF15_DFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_pka_ecc_perf15_dfsr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 21;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_pka_ecc_perf15_dfsr_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_pka_ecc_perf15_dfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_ECC_CMD_RCGR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_ecc_cmd_rcgr_s
{
  u32 update : 1;
  u32 root_en : 1;
  u32 reserved0 : 2;
  u32 dirty_cfg_rcgr : 1;
  u32 reserved1 : 26;
  u32 root_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_ecc_cmd_rcgr_u
{
  struct ipa_gcc_hwio_def_gcc_ecc_cmd_rcgr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_ECC_CFG_RCGR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_ecc_cfg_rcgr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 5;
  u32 rcglite_disable : 1;
  u32 reserved2 : 3;
  u32 hw_clk_control : 1;
  u32 reserved3 : 11;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_ecc_cfg_rcgr_u
{
  struct ipa_gcc_hwio_def_gcc_ecc_cfg_rcgr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_QM_BCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_qm_bcr_s
{
  u32 blk_ares : 1;
  u32 reserved0 : 31;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_qm_bcr_u
{
  struct ipa_gcc_hwio_def_gcc_qm_bcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_QM_CFG_AHB_CBCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_qm_cfg_ahb_cbcr_s
{
  u32 clk_enable : 1;
  u32 reserved0 : 1;
  u32 clk_ares : 1;
  u32 reserved1 : 17;
  u32 ignore_rpmh_clk_dis : 1;
  u32 reserved2 : 1;
  u32 clk_dis : 1;
  u32 ignore_all_clk_dis : 1;
  u32 ignore_all_ares : 1;
  u32 reserved3 : 6;
  u32 clk_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_qm_cfg_ahb_cbcr_u
{
  struct ipa_gcc_hwio_def_gcc_qm_cfg_ahb_cbcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_QM_CORE_CBCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_qm_core_cbcr_s
{
  u32 clk_enable : 1;
  u32 reserved0 : 1;
  u32 clk_ares : 1;
  u32 reserved1 : 19;
  u32 clk_dis : 1;
  u32 ignore_all_clk_dis : 1;
  u32 ignore_all_ares : 1;
  u32 reserved2 : 6;
  u32 clk_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_qm_core_cbcr_u
{
  struct ipa_gcc_hwio_def_gcc_qm_core_cbcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_QM_CMD_RCGR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_qm_cmd_rcgr_s
{
  u32 update : 1;
  u32 root_en : 1;
  u32 reserved0 : 2;
  u32 dirty_cfg_rcgr : 1;
  u32 reserved1 : 26;
  u32 root_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_qm_cmd_rcgr_u
{
  struct ipa_gcc_hwio_def_gcc_qm_cmd_rcgr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_QM_CFG_RCGR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_qm_cfg_rcgr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 5;
  u32 rcglite_disable : 1;
  u32 reserved2 : 3;
  u32 hw_clk_control : 1;
  u32 reserved3 : 11;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_qm_cfg_rcgr_u
{
  struct ipa_gcc_hwio_def_gcc_qm_cfg_rcgr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_MSS_CE_NAV_BRIDGE_BCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_mss_ce_nav_bridge_bcr_s
{
  u32 blk_ares : 1;
  u32 reserved0 : 31;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_mss_ce_nav_bridge_bcr_u
{
  struct ipa_gcc_hwio_def_gcc_mss_ce_nav_bridge_bcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_MSS_CE_NAV_BRIDGE_AXI_CBCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_mss_ce_nav_bridge_axi_cbcr_s
{
  u32 clk_enable : 1;
  u32 reserved0 : 1;
  u32 clk_ares : 1;
  u32 reserved1 : 17;
  u32 ignore_rpmh_clk_dis : 1;
  u32 reserved2 : 1;
  u32 clk_dis : 1;
  u32 ignore_all_clk_dis : 1;
  u32 ignore_all_ares : 1;
  u32 reserved3 : 6;
  u32 clk_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_mss_ce_nav_bridge_axi_cbcr_u
{
  struct ipa_gcc_hwio_def_gcc_mss_ce_nav_bridge_axi_cbcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_SPMI_VGIS_BCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_spmi_vgis_bcr_s
{
  u32 blk_ares : 1;
  u32 reserved0 : 31;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_spmi_vgis_bcr_u
{
  struct ipa_gcc_hwio_def_gcc_spmi_vgis_bcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_SPMI_VGIS_CBCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_spmi_vgis_cbcr_s
{
  u32 clk_enable : 1;
  u32 hw_ctl : 1;
  u32 clk_ares : 1;
  u32 sw_only_en : 1;
  u32 reserved0 : 18;
  u32 clk_dis : 1;
  u32 ignore_all_clk_dis : 1;
  u32 ignore_all_ares : 1;
  u32 reserved1 : 6;
  u32 clk_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_spmi_vgis_cbcr_u
{
  struct ipa_gcc_hwio_def_gcc_spmi_vgis_cbcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_SPMI_VGIS_CMD_RCGR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_spmi_vgis_cmd_rcgr_s
{
  u32 update : 1;
  u32 root_en : 1;
  u32 reserved0 : 2;
  u32 dirty_cfg_rcgr : 1;
  u32 reserved1 : 26;
  u32 root_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_spmi_vgis_cmd_rcgr_u
{
  struct ipa_gcc_hwio_def_gcc_spmi_vgis_cmd_rcgr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_SPMI_VGIS_CFG_RCGR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_spmi_vgis_cfg_rcgr_s
{
  u32 src_div : 5;
  u32 reserved0 : 3;
  u32 src_sel : 3;
  u32 reserved1 : 5;
  u32 rcglite_disable : 1;
  u32 reserved2 : 3;
  u32 hw_clk_control : 1;
  u32 reserved3 : 11;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_spmi_vgis_cfg_rcgr_u
{
  struct ipa_gcc_hwio_def_gcc_spmi_vgis_cfg_rcgr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_MISC_BCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_misc_bcr_s
{
  u32 blk_ares : 1;
  u32 reserved0 : 31;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_misc_bcr_u
{
  struct ipa_gcc_hwio_def_gcc_misc_bcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_PCIE_LINK_DOWN_BCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_pcie_link_down_bcr_s
{
  u32 blk_ares : 1;
  u32 reserved0 : 31;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_pcie_link_down_bcr_u
{
  struct ipa_gcc_hwio_def_gcc_pcie_link_down_bcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_PCIE_PHY_CFG_AHB_BCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_pcie_phy_cfg_ahb_bcr_s
{
  u32 blk_ares : 1;
  u32 reserved0 : 31;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_pcie_phy_cfg_ahb_bcr_u
{
  struct ipa_gcc_hwio_def_gcc_pcie_phy_cfg_ahb_bcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_PCIE_PHY_COM_BCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_pcie_phy_com_bcr_s
{
  u32 blk_ares : 1;
  u32 reserved0 : 31;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_pcie_phy_com_bcr_u
{
  struct ipa_gcc_hwio_def_gcc_pcie_phy_com_bcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_PCIE_NOCSR_COM_PHY_BCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_pcie_nocsr_com_phy_bcr_s
{
  u32 blk_ares : 1;
  u32 reserved0 : 31;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_pcie_nocsr_com_phy_bcr_u
{
  struct ipa_gcc_hwio_def_gcc_pcie_nocsr_com_phy_bcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_PCIE_PHY_NOCSR_COM_PHY_BCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_pcie_phy_nocsr_com_phy_bcr_s
{
  u32 blk_ares : 1;
  u32 reserved0 : 31;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_pcie_phy_nocsr_com_phy_bcr_u
{
  struct ipa_gcc_hwio_def_gcc_pcie_phy_nocsr_com_phy_bcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_GPLL4_OUT_EVEN_PWRGRP1_CLKGEN_ACGC_ACGCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_gpll4_out_even_pwrgrp1_clkgen_acgc_acgcr_s
{
  u32 clk_enable : 1;
  u32 reserved0 : 30;
  u32 clk_on : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_gpll4_out_even_pwrgrp1_clkgen_acgc_acgcr_u
{
  struct ipa_gcc_hwio_def_gcc_gpll4_out_even_pwrgrp1_clkgen_acgc_acgcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_GPLL4_OUT_EVEN_PWRGRP2_CLKGEN_ACGC_ACGCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_gpll4_out_even_pwrgrp2_clkgen_acgc_acgcr_s
{
  u32 clk_enable : 1;
  u32 reserved0 : 30;
  u32 clk_on : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_gpll4_out_even_pwrgrp2_clkgen_acgc_acgcr_u
{
  struct ipa_gcc_hwio_def_gcc_gpll4_out_even_pwrgrp2_clkgen_acgc_acgcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_GPLL1_OUT_EVEN_PWRGRP2_CLKGEN_ACGC_ACGCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_gpll1_out_even_pwrgrp2_clkgen_acgc_acgcr_s
{
  u32 clk_enable : 1;
  u32 reserved0 : 30;
  u32 clk_on : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_gpll1_out_even_pwrgrp2_clkgen_acgc_acgcr_u
{
  struct ipa_gcc_hwio_def_gcc_gpll1_out_even_pwrgrp2_clkgen_acgc_acgcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_GPLL1_OUT_MAIN_PWRGRP1_CLKGEN_ACGC_ACGCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_gpll1_out_main_pwrgrp1_clkgen_acgc_acgcr_s
{
  u32 clk_enable : 1;
  u32 reserved0 : 30;
  u32 clk_on : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_gpll1_out_main_pwrgrp1_clkgen_acgc_acgcr_u
{
  struct ipa_gcc_hwio_def_gcc_gpll1_out_main_pwrgrp1_clkgen_acgc_acgcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_GPLL5_OUT_MAIN_PWRGRP1_CLKGEN_ACGC_ACGCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_gpll5_out_main_pwrgrp1_clkgen_acgc_acgcr_s
{
  u32 clk_enable : 1;
  u32 reserved0 : 30;
  u32 clk_on : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_gpll5_out_main_pwrgrp1_clkgen_acgc_acgcr_u
{
  struct ipa_gcc_hwio_def_gcc_gpll5_out_main_pwrgrp1_clkgen_acgc_acgcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_GPLL5_OUT_MAIN_PWRGRP2_CLKGEN_ACGC_ACGCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_gpll5_out_main_pwrgrp2_clkgen_acgc_acgcr_s
{
  u32 clk_enable : 1;
  u32 reserved0 : 30;
  u32 clk_on : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_gpll5_out_main_pwrgrp2_clkgen_acgc_acgcr_u
{
  struct ipa_gcc_hwio_def_gcc_gpll5_out_main_pwrgrp2_clkgen_acgc_acgcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_GPLL0_OUT_MAIN_PWRGRP1_CLKGEN_ACGC_ACGCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_gpll0_out_main_pwrgrp1_clkgen_acgc_acgcr_s
{
  u32 clk_enable : 1;
  u32 reserved0 : 30;
  u32 clk_on : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_gpll0_out_main_pwrgrp1_clkgen_acgc_acgcr_u
{
  struct ipa_gcc_hwio_def_gcc_gpll0_out_main_pwrgrp1_clkgen_acgc_acgcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_GPLL0_OUT_MAIN_PWRGRP2_CLKGEN_ACGC_ACGCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_gpll0_out_main_pwrgrp2_clkgen_acgc_acgcr_s
{
  u32 clk_enable : 1;
  u32 reserved0 : 30;
  u32 clk_on : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_gpll0_out_main_pwrgrp2_clkgen_acgc_acgcr_u
{
  struct ipa_gcc_hwio_def_gcc_gpll0_out_main_pwrgrp2_clkgen_acgc_acgcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_GPLL0_OUT_MAIN_PWRGRP3_CLKGEN_ACGC_ACGCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_gpll0_out_main_pwrgrp3_clkgen_acgc_acgcr_s
{
  u32 clk_enable : 1;
  u32 reserved0 : 30;
  u32 clk_on : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_gpll0_out_main_pwrgrp3_clkgen_acgc_acgcr_u
{
  struct ipa_gcc_hwio_def_gcc_gpll0_out_main_pwrgrp3_clkgen_acgc_acgcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_GPLL0_OUT_MAIN_PWRGRP4_CLKGEN_ACGC_ACGCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_gpll0_out_main_pwrgrp4_clkgen_acgc_acgcr_s
{
  u32 clk_enable : 1;
  u32 reserved0 : 30;
  u32 clk_on : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_gpll0_out_main_pwrgrp4_clkgen_acgc_acgcr_u
{
  struct ipa_gcc_hwio_def_gcc_gpll0_out_main_pwrgrp4_clkgen_acgc_acgcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_GPLL0_OUT_MAIN_PWRGRP5_CLKGEN_ACGC_ACGCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_gpll0_out_main_pwrgrp5_clkgen_acgc_acgcr_s
{
  u32 clk_enable : 1;
  u32 reserved0 : 30;
  u32 clk_on : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_gpll0_out_main_pwrgrp5_clkgen_acgc_acgcr_u
{
  struct ipa_gcc_hwio_def_gcc_gpll0_out_main_pwrgrp5_clkgen_acgc_acgcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_GPLL0_OUT_MAIN_PWRGRP6_CLKGEN_ACGC_ACGCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_gpll0_out_main_pwrgrp6_clkgen_acgc_acgcr_s
{
  u32 clk_enable : 1;
  u32 reserved0 : 30;
  u32 clk_on : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_gpll0_out_main_pwrgrp6_clkgen_acgc_acgcr_u
{
  struct ipa_gcc_hwio_def_gcc_gpll0_out_main_pwrgrp6_clkgen_acgc_acgcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_GPLL0_OUT_MAIN_PWRGRP7_CLKGEN_ACGC_ACGCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_gpll0_out_main_pwrgrp7_clkgen_acgc_acgcr_s
{
  u32 clk_enable : 1;
  u32 reserved0 : 30;
  u32 clk_on : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_gpll0_out_main_pwrgrp7_clkgen_acgc_acgcr_u
{
  struct ipa_gcc_hwio_def_gcc_gpll0_out_main_pwrgrp7_clkgen_acgc_acgcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_GPLL0_OUT_MAIN_PWRGRP8_CLKGEN_ACGC_ACGCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_gpll0_out_main_pwrgrp8_clkgen_acgc_acgcr_s
{
  u32 clk_enable : 1;
  u32 reserved0 : 30;
  u32 clk_on : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_gpll0_out_main_pwrgrp8_clkgen_acgc_acgcr_u
{
  struct ipa_gcc_hwio_def_gcc_gpll0_out_main_pwrgrp8_clkgen_acgc_acgcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_GPLL0_OUT_MAIN_PWRGRP9_CLKGEN_ACGC_ACGCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_gpll0_out_main_pwrgrp9_clkgen_acgc_acgcr_s
{
  u32 clk_enable : 1;
  u32 reserved0 : 30;
  u32 clk_on : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_gpll0_out_main_pwrgrp9_clkgen_acgc_acgcr_u
{
  struct ipa_gcc_hwio_def_gcc_gpll0_out_main_pwrgrp9_clkgen_acgc_acgcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_GPLL0_OUT_MAIN_PWRGRP10_CLKGEN_ACGC_ACGCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_gpll0_out_main_pwrgrp10_clkgen_acgc_acgcr_s
{
  u32 clk_enable : 1;
  u32 reserved0 : 30;
  u32 clk_on : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_gpll0_out_main_pwrgrp10_clkgen_acgc_acgcr_u
{
  struct ipa_gcc_hwio_def_gcc_gpll0_out_main_pwrgrp10_clkgen_acgc_acgcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_GPLL0_OUT_MAIN_PWRGRP11_CLKGEN_ACGC_ACGCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_gpll0_out_main_pwrgrp11_clkgen_acgc_acgcr_s
{
  u32 clk_enable : 1;
  u32 reserved0 : 30;
  u32 clk_on : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_gpll0_out_main_pwrgrp11_clkgen_acgc_acgcr_u
{
  struct ipa_gcc_hwio_def_gcc_gpll0_out_main_pwrgrp11_clkgen_acgc_acgcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_GPLL0_OUT_MAIN_PWRGRP12_CLKGEN_ACGC_ACGCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_gpll0_out_main_pwrgrp12_clkgen_acgc_acgcr_s
{
  u32 clk_enable : 1;
  u32 reserved0 : 30;
  u32 clk_on : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_gpll0_out_main_pwrgrp12_clkgen_acgc_acgcr_u
{
  struct ipa_gcc_hwio_def_gcc_gpll0_out_main_pwrgrp12_clkgen_acgc_acgcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_GPLL0_OUT_EVEN_PWRGRP15_CLKGEN_ACGC_ACGCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_gpll0_out_even_pwrgrp15_clkgen_acgc_acgcr_s
{
  u32 clk_enable : 1;
  u32 reserved0 : 30;
  u32 clk_on : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_gpll0_out_even_pwrgrp15_clkgen_acgc_acgcr_u
{
  struct ipa_gcc_hwio_def_gcc_gpll0_out_even_pwrgrp15_clkgen_acgc_acgcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_NAV_MBIST_ACGCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_nav_mbist_acgcr_s
{
  u32 clk_enable : 1;
  u32 reserved0 : 30;
  u32 clk_on : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_nav_mbist_acgcr_u
{
  struct ipa_gcc_hwio_def_gcc_nav_mbist_acgcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_USB3_PHY_PIPE_MUXR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_usb3_phy_pipe_muxr_s
{
  u32 mux_sel : 2;
  u32 reserved0 : 30;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_usb3_phy_pipe_muxr_u
{
  struct ipa_gcc_hwio_def_gcc_usb3_phy_pipe_muxr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_JBIST_REF_CLK_MUXR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_jbist_ref_clk_muxr_s
{
  u32 mux_sel : 2;
  u32 reserved0 : 30;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_jbist_ref_clk_muxr_u
{
  struct ipa_gcc_hwio_def_gcc_jbist_ref_clk_muxr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_PCIE_PIPE_MUXR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_pcie_pipe_muxr_s
{
  u32 mux_sel : 2;
  u32 reserved0 : 30;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_pcie_pipe_muxr_u
{
  struct ipa_gcc_hwio_def_gcc_pcie_pipe_muxr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_PCIE_AUX_MUXR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_pcie_aux_muxr_s
{
  u32 mux_sel : 2;
  u32 reserved0 : 30;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_pcie_aux_muxr_u
{
  struct ipa_gcc_hwio_def_gcc_pcie_aux_muxr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_PCIE_MBIST_MUXR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_pcie_mbist_muxr_s
{
  u32 mux_sel : 2;
  u32 reserved0 : 30;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_pcie_mbist_muxr_u
{
  struct ipa_gcc_hwio_def_gcc_pcie_mbist_muxr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_NAV_MBIST_MUXR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_nav_mbist_muxr_s
{
  u32 mux_sel : 1;
  u32 reserved0 : 31;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_nav_mbist_muxr_u
{
  struct ipa_gcc_hwio_def_gcc_nav_mbist_muxr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_MSS_Q6SS_BOOT_GPLL0_MUXR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_mss_q6ss_boot_gpll0_muxr_s
{
  u32 mux_sel : 1;
  u32 reserved0 : 31;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_mss_q6ss_boot_gpll0_muxr_u
{
  struct ipa_gcc_hwio_def_gcc_mss_q6ss_boot_gpll0_muxr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_AUDIO_PLL_REF_MUXR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_audio_pll_ref_muxr_s
{
  u32 mux_sel : 2;
  u32 reserved0 : 30;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_audio_pll_ref_muxr_u
{
  struct ipa_gcc_hwio_def_gcc_audio_pll_ref_muxr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_IPA_AHB_MISC_CBCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_ipa_ahb_misc_cbcr_s
{
  u32 reserved0 : 1;
  u32 hw_ctl : 1;
  u32 reserved1 : 30;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_ipa_ahb_misc_cbcr_u
{
  struct ipa_gcc_hwio_def_gcc_ipa_ahb_misc_cbcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_TCSR_PCIE_BCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_tcsr_pcie_bcr_s
{
  u32 blk_ares : 1;
  u32 reserved0 : 31;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_tcsr_pcie_bcr_u
{
  struct ipa_gcc_hwio_def_gcc_tcsr_pcie_bcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_GPLL4_PLL_TEST_SE_OVRD
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_gpll4_pll_test_se_ovrd_s
{
  u32 ovrd : 1;
  u32 reserved0 : 31;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_gpll4_pll_test_se_ovrd_u
{
  struct ipa_gcc_hwio_def_gcc_gpll4_pll_test_se_ovrd_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_ACC_MISC
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_acc_misc_s
{
  u32 jtag_acc_src_sel_en : 1;
  u32 reserved0 : 31;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_acc_misc_u
{
  struct ipa_gcc_hwio_def_gcc_acc_misc_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_CPUSS_AHB_MISC
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_cpuss_ahb_misc_s
{
  u32 cpuss_ahb_clk_auto_scale_dis : 1;
  u32 reserved0 : 3;
  u32 cpuss_ahb_clk_auto_scale_div : 4;
  u32 reserved1 : 24;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_cpuss_ahb_misc_u
{
  struct ipa_gcc_hwio_def_gcc_cpuss_ahb_misc_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_USB_30_MISC
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_usb_30_misc_s
{
  u32 blk_ares_all : 1;
  u32 reserved0 : 31;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_usb_30_misc_u
{
  struct ipa_gcc_hwio_def_gcc_usb_30_misc_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPM_GPLL_ENA_VOTE
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpm_gpll_ena_vote_s
{
  u32 gpll0 : 1;
  u32 gpll1 : 1;
  u32 gpll2 : 1;
  u32 gpll3 : 1;
  u32 gpll4 : 1;
  u32 gpll5 : 1;
  u32 gpll6 : 1;
  u32 reserved0 : 25;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpm_gpll_ena_vote_u
{
  struct ipa_gcc_hwio_def_gcc_rpm_gpll_ena_vote_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPM_GPLL_SLEEP_ENA_VOTE
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpm_gpll_sleep_ena_vote_s
{
  u32 gpll0 : 1;
  u32 gpll1 : 1;
  u32 gpll2 : 1;
  u32 gpll3 : 1;
  u32 gpll4 : 1;
  u32 gpll5 : 1;
  u32 gpll6 : 1;
  u32 reserved0 : 25;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpm_gpll_sleep_ena_vote_u
{
  struct ipa_gcc_hwio_def_gcc_rpm_gpll_sleep_ena_vote_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPM_CLOCK_BRANCH_ENA_VOTE
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpm_clock_branch_ena_vote_s
{
  u32 sys_noc_cpuss_ahb_clk_ena : 1;
  u32 tcsr_ahb_clk_ena : 1;
  u32 qdss_cfg_ahb_clk_ena : 1;
  u32 ce1_ahb_clk_ena : 1;
  u32 ce1_axi_clk_ena : 1;
  u32 ce1_clk_ena : 1;
  u32 tlmm_clk_ena : 1;
  u32 ultaudio_pcnoc_sway_clk_ena : 1;
  u32 ultaudio_ahbfabric_ixfabric_clk_ena : 1;
  u32 reserved0 : 1;
  u32 boot_rom_ahb_clk_ena : 1;
  u32 reserved1 : 1;
  u32 tlmm_ahb_clk_ena : 1;
  u32 prng_ahb_clk_ena : 1;
  u32 blsp1_ahb_clk_ena : 1;
  u32 blsp1_sleep_clk_ena : 1;
  u32 reserved2 : 1;
  u32 mss_gpll0_div_clk_src_ena : 1;
  u32 reserved3 : 3;
  u32 cpuss_ahb_clk_ena : 1;
  u32 cpuss_gnoc_clk_ena : 1;
  u32 reserved4 : 1;
  u32 imem_axi_clk_ena : 1;
  u32 reserved5 : 7;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpm_clock_branch_ena_vote_u
{
  struct ipa_gcc_hwio_def_gcc_rpm_clock_branch_ena_vote_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPM_CLOCK_SLEEP_ENA_VOTE
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpm_clock_sleep_ena_vote_s
{
  u32 sys_noc_cpuss_ahb_clk_sleep_ena : 1;
  u32 tcsr_ahb_clk_sleep_ena : 1;
  u32 qdss_cfg_ahb_clk_sleep_ena : 1;
  u32 ce1_ahb_clk_sleep_ena : 1;
  u32 ce1_axi_clk_sleep_ena : 1;
  u32 ce1_clk_sleep_ena : 1;
  u32 tlmm_clk_sleep_ena : 1;
  u32 ultaudio_pcnoc_sway_clk_sleep_ena : 1;
  u32 ultaudio_ahbfabric_ixfabric_clk_sleep_ena : 1;
  u32 reserved0 : 1;
  u32 boot_rom_ahb_clk_sleep_ena : 1;
  u32 reserved1 : 1;
  u32 tlmm_ahb_clk_sleep_ena : 1;
  u32 prng_ahb_clk_sleep_ena : 1;
  u32 blsp1_ahb_clk_sleep_ena : 1;
  u32 blsp1_sleep_clk_sleep_ena : 1;
  u32 reserved2 : 1;
  u32 mss_gpll0_div_clk_src_sleep_ena : 1;
  u32 reserved3 : 3;
  u32 cpuss_ahb_clk_sleep_ena : 1;
  u32 cpuss_gnoc_clk_sleep_ena : 1;
  u32 reserved4 : 1;
  u32 imem_axi_clk_sleep_ena : 1;
  u32 reserved5 : 7;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpm_clock_sleep_ena_vote_u
{
  struct ipa_gcc_hwio_def_gcc_rpm_clock_sleep_ena_vote_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPM_CLOCK_BRANCH_ENA_VOTE_1
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpm_clock_branch_ena_vote_1_s
{
  u32 pcie_slv_axi_clk_ena : 1;
  u32 pcie_mstr_axi_clk_ena : 1;
  u32 pcie_cfg_ahb_clk_ena : 1;
  u32 pcie_aux_clk_ena : 1;
  u32 pcie_pipe_clk_ena : 1;
  u32 pcie_slv_q2a_axi_clk_ena : 1;
  u32 pcie_sleep_clk_ena : 1;
  u32 pcie_rchng_phy_clk_ena : 1;
  u32 reserved0 : 24;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpm_clock_branch_ena_vote_1_u
{
  struct ipa_gcc_hwio_def_gcc_rpm_clock_branch_ena_vote_1_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPM_CLOCK_SLEEP_ENA_VOTE_1
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpm_clock_sleep_ena_vote_1_s
{
  u32 pcie_slv_axi_clk_sleep_ena : 1;
  u32 pcie_mstr_axi_clk_sleep_ena : 1;
  u32 pcie_cfg_ahb_clk_sleep_ena : 1;
  u32 pcie_aux_clk_sleep_ena : 1;
  u32 pcie_pipe_clk_sleep_ena : 1;
  u32 pcie_slv_q2a_axi_clk_sleep_ena : 1;
  u32 pcie_sleep_clk_sleep_ena : 1;
  u32 pcie_rchng_phy_clk_sleep_ena : 1;
  u32 reserved0 : 24;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpm_clock_sleep_ena_vote_1_u
{
  struct ipa_gcc_hwio_def_gcc_rpm_clock_sleep_ena_vote_1_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_APCS_GPLL_ENA_VOTE
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_apcs_gpll_ena_vote_s
{
  u32 gpll0 : 1;
  u32 gpll1 : 1;
  u32 gpll2 : 1;
  u32 gpll3 : 1;
  u32 gpll4 : 1;
  u32 gpll5 : 1;
  u32 gpll6 : 1;
  u32 reserved0 : 25;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_apcs_gpll_ena_vote_u
{
  struct ipa_gcc_hwio_def_gcc_apcs_gpll_ena_vote_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_APCS_GPLL_SLEEP_ENA_VOTE
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_apcs_gpll_sleep_ena_vote_s
{
  u32 gpll0 : 1;
  u32 gpll1 : 1;
  u32 gpll2 : 1;
  u32 gpll3 : 1;
  u32 gpll4 : 1;
  u32 gpll5 : 1;
  u32 gpll6 : 1;
  u32 reserved0 : 25;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_apcs_gpll_sleep_ena_vote_u
{
  struct ipa_gcc_hwio_def_gcc_apcs_gpll_sleep_ena_vote_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_APCS_CLOCK_BRANCH_ENA_VOTE
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_apcs_clock_branch_ena_vote_s
{
  u32 sys_noc_cpuss_ahb_clk_ena : 1;
  u32 tcsr_ahb_clk_ena : 1;
  u32 qdss_cfg_ahb_clk_ena : 1;
  u32 ce1_ahb_clk_ena : 1;
  u32 ce1_axi_clk_ena : 1;
  u32 ce1_clk_ena : 1;
  u32 tlmm_clk_ena : 1;
  u32 ultaudio_pcnoc_sway_clk_ena : 1;
  u32 ultaudio_ahbfabric_ixfabric_clk_ena : 1;
  u32 reserved0 : 1;
  u32 boot_rom_ahb_clk_ena : 1;
  u32 reserved1 : 1;
  u32 tlmm_ahb_clk_ena : 1;
  u32 prng_ahb_clk_ena : 1;
  u32 blsp1_ahb_clk_ena : 1;
  u32 blsp1_sleep_clk_ena : 1;
  u32 reserved2 : 1;
  u32 mss_gpll0_div_clk_src_ena : 1;
  u32 reserved3 : 3;
  u32 cpuss_ahb_clk_ena : 1;
  u32 cpuss_gnoc_clk_ena : 1;
  u32 reserved4 : 1;
  u32 imem_axi_clk_ena : 1;
  u32 reserved5 : 7;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_apcs_clock_branch_ena_vote_u
{
  struct ipa_gcc_hwio_def_gcc_apcs_clock_branch_ena_vote_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_APCS_CLOCK_SLEEP_ENA_VOTE
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_apcs_clock_sleep_ena_vote_s
{
  u32 sys_noc_cpuss_ahb_clk_sleep_ena : 1;
  u32 tcsr_ahb_clk_sleep_ena : 1;
  u32 qdss_cfg_ahb_clk_sleep_ena : 1;
  u32 ce1_ahb_clk_sleep_ena : 1;
  u32 ce1_axi_clk_sleep_ena : 1;
  u32 ce1_clk_sleep_ena : 1;
  u32 tlmm_clk_sleep_ena : 1;
  u32 ultaudio_pcnoc_sway_clk_sleep_ena : 1;
  u32 ultaudio_ahbfabric_ixfabric_clk_sleep_ena : 1;
  u32 reserved0 : 1;
  u32 boot_rom_ahb_clk_sleep_ena : 1;
  u32 reserved1 : 1;
  u32 tlmm_ahb_clk_sleep_ena : 1;
  u32 prng_ahb_clk_sleep_ena : 1;
  u32 blsp1_ahb_clk_sleep_ena : 1;
  u32 blsp1_sleep_clk_sleep_ena : 1;
  u32 reserved2 : 1;
  u32 mss_gpll0_div_clk_src_sleep_ena : 1;
  u32 reserved3 : 3;
  u32 cpuss_ahb_clk_sleep_ena : 1;
  u32 cpuss_gnoc_clk_sleep_ena : 1;
  u32 reserved4 : 1;
  u32 imem_axi_clk_sleep_ena : 1;
  u32 reserved5 : 7;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_apcs_clock_sleep_ena_vote_u
{
  struct ipa_gcc_hwio_def_gcc_apcs_clock_sleep_ena_vote_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_APCS_CLOCK_BRANCH_ENA_VOTE_1
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_apcs_clock_branch_ena_vote_1_s
{
  u32 pcie_slv_axi_clk_ena : 1;
  u32 pcie_mstr_axi_clk_ena : 1;
  u32 pcie_cfg_ahb_clk_ena : 1;
  u32 pcie_aux_clk_ena : 1;
  u32 pcie_pipe_clk_ena : 1;
  u32 pcie_slv_q2a_axi_clk_ena : 1;
  u32 pcie_sleep_clk_ena : 1;
  u32 pcie_rchng_phy_clk_ena : 1;
  u32 reserved0 : 24;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_apcs_clock_branch_ena_vote_1_u
{
  struct ipa_gcc_hwio_def_gcc_apcs_clock_branch_ena_vote_1_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_APCS_CLOCK_SLEEP_ENA_VOTE_1
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_apcs_clock_sleep_ena_vote_1_s
{
  u32 pcie_slv_axi_clk_sleep_ena : 1;
  u32 pcie_mstr_axi_clk_sleep_ena : 1;
  u32 pcie_cfg_ahb_clk_sleep_ena : 1;
  u32 pcie_aux_clk_sleep_ena : 1;
  u32 pcie_pipe_clk_sleep_ena : 1;
  u32 pcie_slv_q2a_axi_clk_sleep_ena : 1;
  u32 pcie_sleep_clk_sleep_ena : 1;
  u32 pcie_rchng_phy_clk_sleep_ena : 1;
  u32 reserved0 : 24;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_apcs_clock_sleep_ena_vote_1_u
{
  struct ipa_gcc_hwio_def_gcc_apcs_clock_sleep_ena_vote_1_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_APCS_TZ_GPLL_ENA_VOTE
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_apcs_tz_gpll_ena_vote_s
{
  u32 gpll0 : 1;
  u32 gpll1 : 1;
  u32 gpll2 : 1;
  u32 gpll3 : 1;
  u32 gpll4 : 1;
  u32 gpll5 : 1;
  u32 gpll6 : 1;
  u32 reserved0 : 25;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_apcs_tz_gpll_ena_vote_u
{
  struct ipa_gcc_hwio_def_gcc_apcs_tz_gpll_ena_vote_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_APCS_TZ_GPLL_SLEEP_ENA_VOTE
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_apcs_tz_gpll_sleep_ena_vote_s
{
  u32 gpll0 : 1;
  u32 gpll1 : 1;
  u32 gpll2 : 1;
  u32 gpll3 : 1;
  u32 gpll4 : 1;
  u32 gpll5 : 1;
  u32 gpll6 : 1;
  u32 reserved0 : 25;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_apcs_tz_gpll_sleep_ena_vote_u
{
  struct ipa_gcc_hwio_def_gcc_apcs_tz_gpll_sleep_ena_vote_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_APCS_TZ_CLOCK_BRANCH_ENA_VOTE
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_apcs_tz_clock_branch_ena_vote_s
{
  u32 sys_noc_cpuss_ahb_clk_ena : 1;
  u32 tcsr_ahb_clk_ena : 1;
  u32 qdss_cfg_ahb_clk_ena : 1;
  u32 ce1_ahb_clk_ena : 1;
  u32 ce1_axi_clk_ena : 1;
  u32 ce1_clk_ena : 1;
  u32 tlmm_clk_ena : 1;
  u32 ultaudio_pcnoc_sway_clk_ena : 1;
  u32 ultaudio_ahbfabric_ixfabric_clk_ena : 1;
  u32 reserved0 : 1;
  u32 boot_rom_ahb_clk_ena : 1;
  u32 reserved1 : 1;
  u32 tlmm_ahb_clk_ena : 1;
  u32 prng_ahb_clk_ena : 1;
  u32 blsp1_ahb_clk_ena : 1;
  u32 blsp1_sleep_clk_ena : 1;
  u32 reserved2 : 1;
  u32 mss_gpll0_div_clk_src_ena : 1;
  u32 reserved3 : 3;
  u32 cpuss_ahb_clk_ena : 1;
  u32 cpuss_gnoc_clk_ena : 1;
  u32 reserved4 : 1;
  u32 imem_axi_clk_ena : 1;
  u32 reserved5 : 7;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_apcs_tz_clock_branch_ena_vote_u
{
  struct ipa_gcc_hwio_def_gcc_apcs_tz_clock_branch_ena_vote_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_APCS_TZ_CLOCK_SLEEP_ENA_VOTE
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_apcs_tz_clock_sleep_ena_vote_s
{
  u32 sys_noc_cpuss_ahb_clk_sleep_ena : 1;
  u32 tcsr_ahb_clk_sleep_ena : 1;
  u32 qdss_cfg_ahb_clk_sleep_ena : 1;
  u32 ce1_ahb_clk_sleep_ena : 1;
  u32 ce1_axi_clk_sleep_ena : 1;
  u32 ce1_clk_sleep_ena : 1;
  u32 tlmm_clk_sleep_ena : 1;
  u32 ultaudio_pcnoc_sway_clk_sleep_ena : 1;
  u32 ultaudio_ahbfabric_ixfabric_clk_sleep_ena : 1;
  u32 reserved0 : 1;
  u32 boot_rom_ahb_clk_sleep_ena : 1;
  u32 reserved1 : 1;
  u32 tlmm_ahb_clk_sleep_ena : 1;
  u32 prng_ahb_clk_sleep_ena : 1;
  u32 blsp1_ahb_clk_sleep_ena : 1;
  u32 blsp1_sleep_clk_sleep_ena : 1;
  u32 reserved2 : 1;
  u32 mss_gpll0_div_clk_src_sleep_ena : 1;
  u32 reserved3 : 3;
  u32 cpuss_ahb_clk_sleep_ena : 1;
  u32 cpuss_gnoc_clk_sleep_ena : 1;
  u32 reserved4 : 1;
  u32 imem_axi_clk_sleep_ena : 1;
  u32 reserved5 : 7;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_apcs_tz_clock_sleep_ena_vote_u
{
  struct ipa_gcc_hwio_def_gcc_apcs_tz_clock_sleep_ena_vote_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_APCS_TZ_CLOCK_BRANCH_ENA_VOTE_1
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_apcs_tz_clock_branch_ena_vote_1_s
{
  u32 pcie_slv_axi_clk_ena : 1;
  u32 pcie_mstr_axi_clk_ena : 1;
  u32 pcie_cfg_ahb_clk_ena : 1;
  u32 pcie_aux_clk_ena : 1;
  u32 pcie_pipe_clk_ena : 1;
  u32 pcie_slv_q2a_axi_clk_ena : 1;
  u32 pcie_sleep_clk_ena : 1;
  u32 pcie_rchng_phy_clk_ena : 1;
  u32 reserved0 : 24;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_apcs_tz_clock_branch_ena_vote_1_u
{
  struct ipa_gcc_hwio_def_gcc_apcs_tz_clock_branch_ena_vote_1_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_APCS_TZ_CLOCK_SLEEP_ENA_VOTE_1
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_apcs_tz_clock_sleep_ena_vote_1_s
{
  u32 pcie_slv_axi_clk_sleep_ena : 1;
  u32 pcie_mstr_axi_clk_sleep_ena : 1;
  u32 pcie_cfg_ahb_clk_sleep_ena : 1;
  u32 pcie_aux_clk_sleep_ena : 1;
  u32 pcie_pipe_clk_sleep_ena : 1;
  u32 pcie_slv_q2a_axi_clk_sleep_ena : 1;
  u32 pcie_sleep_clk_sleep_ena : 1;
  u32 pcie_rchng_phy_clk_sleep_ena : 1;
  u32 reserved0 : 24;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_apcs_tz_clock_sleep_ena_vote_1_u
{
  struct ipa_gcc_hwio_def_gcc_apcs_tz_clock_sleep_ena_vote_1_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_HYP_GPLL_ENA_VOTE
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_hyp_gpll_ena_vote_s
{
  u32 gpll0 : 1;
  u32 gpll1 : 1;
  u32 gpll2 : 1;
  u32 gpll3 : 1;
  u32 gpll4 : 1;
  u32 gpll5 : 1;
  u32 gpll6 : 1;
  u32 reserved0 : 25;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_hyp_gpll_ena_vote_u
{
  struct ipa_gcc_hwio_def_gcc_hyp_gpll_ena_vote_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_HYP_GPLL_SLEEP_ENA_VOTE
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_hyp_gpll_sleep_ena_vote_s
{
  u32 gpll0 : 1;
  u32 gpll1 : 1;
  u32 gpll2 : 1;
  u32 gpll3 : 1;
  u32 gpll4 : 1;
  u32 gpll5 : 1;
  u32 gpll6 : 1;
  u32 reserved0 : 25;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_hyp_gpll_sleep_ena_vote_u
{
  struct ipa_gcc_hwio_def_gcc_hyp_gpll_sleep_ena_vote_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_HYP_CLOCK_BRANCH_ENA_VOTE
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_hyp_clock_branch_ena_vote_s
{
  u32 sys_noc_cpuss_ahb_clk_ena : 1;
  u32 tcsr_ahb_clk_ena : 1;
  u32 qdss_cfg_ahb_clk_ena : 1;
  u32 ce1_ahb_clk_ena : 1;
  u32 ce1_axi_clk_ena : 1;
  u32 ce1_clk_ena : 1;
  u32 tlmm_clk_ena : 1;
  u32 ultaudio_pcnoc_sway_clk_ena : 1;
  u32 ultaudio_ahbfabric_ixfabric_clk_ena : 1;
  u32 reserved0 : 1;
  u32 boot_rom_ahb_clk_ena : 1;
  u32 reserved1 : 1;
  u32 tlmm_ahb_clk_ena : 1;
  u32 prng_ahb_clk_ena : 1;
  u32 blsp1_ahb_clk_ena : 1;
  u32 blsp1_sleep_clk_ena : 1;
  u32 reserved2 : 1;
  u32 mss_gpll0_div_clk_src_ena : 1;
  u32 reserved3 : 3;
  u32 cpuss_ahb_clk_ena : 1;
  u32 cpuss_gnoc_clk_ena : 1;
  u32 reserved4 : 1;
  u32 imem_axi_clk_ena : 1;
  u32 reserved5 : 7;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_hyp_clock_branch_ena_vote_u
{
  struct ipa_gcc_hwio_def_gcc_hyp_clock_branch_ena_vote_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_HYP_CLOCK_SLEEP_ENA_VOTE
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_hyp_clock_sleep_ena_vote_s
{
  u32 sys_noc_cpuss_ahb_clk_sleep_ena : 1;
  u32 tcsr_ahb_clk_sleep_ena : 1;
  u32 qdss_cfg_ahb_clk_sleep_ena : 1;
  u32 ce1_ahb_clk_sleep_ena : 1;
  u32 ce1_axi_clk_sleep_ena : 1;
  u32 ce1_clk_sleep_ena : 1;
  u32 tlmm_clk_sleep_ena : 1;
  u32 ultaudio_pcnoc_sway_clk_sleep_ena : 1;
  u32 ultaudio_ahbfabric_ixfabric_clk_sleep_ena : 1;
  u32 reserved0 : 1;
  u32 boot_rom_ahb_clk_sleep_ena : 1;
  u32 reserved1 : 1;
  u32 tlmm_ahb_clk_sleep_ena : 1;
  u32 prng_ahb_clk_sleep_ena : 1;
  u32 blsp1_ahb_clk_sleep_ena : 1;
  u32 blsp1_sleep_clk_sleep_ena : 1;
  u32 reserved2 : 1;
  u32 mss_gpll0_div_clk_src_sleep_ena : 1;
  u32 reserved3 : 3;
  u32 cpuss_ahb_clk_sleep_ena : 1;
  u32 cpuss_gnoc_clk_sleep_ena : 1;
  u32 reserved4 : 1;
  u32 imem_axi_clk_sleep_ena : 1;
  u32 reserved5 : 7;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_hyp_clock_sleep_ena_vote_u
{
  struct ipa_gcc_hwio_def_gcc_hyp_clock_sleep_ena_vote_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_HYP_CLOCK_BRANCH_ENA_VOTE_1
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_hyp_clock_branch_ena_vote_1_s
{
  u32 pcie_slv_axi_clk_ena : 1;
  u32 pcie_mstr_axi_clk_ena : 1;
  u32 pcie_cfg_ahb_clk_ena : 1;
  u32 pcie_aux_clk_ena : 1;
  u32 pcie_pipe_clk_ena : 1;
  u32 pcie_slv_q2a_axi_clk_ena : 1;
  u32 pcie_sleep_clk_ena : 1;
  u32 pcie_rchng_phy_clk_ena : 1;
  u32 reserved0 : 24;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_hyp_clock_branch_ena_vote_1_u
{
  struct ipa_gcc_hwio_def_gcc_hyp_clock_branch_ena_vote_1_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_HYP_CLOCK_SLEEP_ENA_VOTE_1
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_hyp_clock_sleep_ena_vote_1_s
{
  u32 pcie_slv_axi_clk_sleep_ena : 1;
  u32 pcie_mstr_axi_clk_sleep_ena : 1;
  u32 pcie_cfg_ahb_clk_sleep_ena : 1;
  u32 pcie_aux_clk_sleep_ena : 1;
  u32 pcie_pipe_clk_sleep_ena : 1;
  u32 pcie_slv_q2a_axi_clk_sleep_ena : 1;
  u32 pcie_sleep_clk_sleep_ena : 1;
  u32 pcie_rchng_phy_clk_sleep_ena : 1;
  u32 reserved0 : 24;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_hyp_clock_sleep_ena_vote_1_u
{
  struct ipa_gcc_hwio_def_gcc_hyp_clock_sleep_ena_vote_1_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_SPARE_GPLL_ENA_VOTE
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_spare_gpll_ena_vote_s
{
  u32 gpll0 : 1;
  u32 gpll1 : 1;
  u32 gpll2 : 1;
  u32 gpll3 : 1;
  u32 gpll4 : 1;
  u32 gpll5 : 1;
  u32 gpll6 : 1;
  u32 reserved0 : 25;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_spare_gpll_ena_vote_u
{
  struct ipa_gcc_hwio_def_gcc_spare_gpll_ena_vote_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_SPARE_GPLL_SLEEP_ENA_VOTE
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_spare_gpll_sleep_ena_vote_s
{
  u32 gpll0 : 1;
  u32 gpll1 : 1;
  u32 gpll2 : 1;
  u32 gpll3 : 1;
  u32 gpll4 : 1;
  u32 gpll5 : 1;
  u32 gpll6 : 1;
  u32 reserved0 : 25;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_spare_gpll_sleep_ena_vote_u
{
  struct ipa_gcc_hwio_def_gcc_spare_gpll_sleep_ena_vote_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_SPARE_CLOCK_BRANCH_ENA_VOTE
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_spare_clock_branch_ena_vote_s
{
  u32 sys_noc_cpuss_ahb_clk_ena : 1;
  u32 tcsr_ahb_clk_ena : 1;
  u32 qdss_cfg_ahb_clk_ena : 1;
  u32 ce1_ahb_clk_ena : 1;
  u32 ce1_axi_clk_ena : 1;
  u32 ce1_clk_ena : 1;
  u32 tlmm_clk_ena : 1;
  u32 ultaudio_pcnoc_sway_clk_ena : 1;
  u32 ultaudio_ahbfabric_ixfabric_clk_ena : 1;
  u32 reserved0 : 1;
  u32 boot_rom_ahb_clk_ena : 1;
  u32 reserved1 : 1;
  u32 tlmm_ahb_clk_ena : 1;
  u32 prng_ahb_clk_ena : 1;
  u32 blsp1_ahb_clk_ena : 1;
  u32 blsp1_sleep_clk_ena : 1;
  u32 reserved2 : 1;
  u32 mss_gpll0_div_clk_src_ena : 1;
  u32 reserved3 : 3;
  u32 cpuss_ahb_clk_ena : 1;
  u32 cpuss_gnoc_clk_ena : 1;
  u32 reserved4 : 1;
  u32 imem_axi_clk_ena : 1;
  u32 reserved5 : 7;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_spare_clock_branch_ena_vote_u
{
  struct ipa_gcc_hwio_def_gcc_spare_clock_branch_ena_vote_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_SPARE_CLOCK_SLEEP_ENA_VOTE
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_spare_clock_sleep_ena_vote_s
{
  u32 sys_noc_cpuss_ahb_clk_sleep_ena : 1;
  u32 tcsr_ahb_clk_sleep_ena : 1;
  u32 qdss_cfg_ahb_clk_sleep_ena : 1;
  u32 ce1_ahb_clk_sleep_ena : 1;
  u32 ce1_axi_clk_sleep_ena : 1;
  u32 ce1_clk_sleep_ena : 1;
  u32 tlmm_clk_sleep_ena : 1;
  u32 ultaudio_pcnoc_sway_clk_sleep_ena : 1;
  u32 ultaudio_ahbfabric_ixfabric_clk_sleep_ena : 1;
  u32 reserved0 : 1;
  u32 boot_rom_ahb_clk_sleep_ena : 1;
  u32 reserved1 : 1;
  u32 tlmm_ahb_clk_sleep_ena : 1;
  u32 prng_ahb_clk_sleep_ena : 1;
  u32 blsp1_ahb_clk_sleep_ena : 1;
  u32 blsp1_sleep_clk_sleep_ena : 1;
  u32 reserved2 : 1;
  u32 mss_gpll0_div_clk_src_sleep_ena : 1;
  u32 reserved3 : 3;
  u32 cpuss_ahb_clk_sleep_ena : 1;
  u32 cpuss_gnoc_clk_sleep_ena : 1;
  u32 reserved4 : 1;
  u32 imem_axi_clk_sleep_ena : 1;
  u32 reserved5 : 7;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_spare_clock_sleep_ena_vote_u
{
  struct ipa_gcc_hwio_def_gcc_spare_clock_sleep_ena_vote_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_SPARE_CLOCK_BRANCH_ENA_VOTE_1
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_spare_clock_branch_ena_vote_1_s
{
  u32 pcie_slv_axi_clk_ena : 1;
  u32 pcie_mstr_axi_clk_ena : 1;
  u32 pcie_cfg_ahb_clk_ena : 1;
  u32 pcie_aux_clk_ena : 1;
  u32 pcie_pipe_clk_ena : 1;
  u32 pcie_slv_q2a_axi_clk_ena : 1;
  u32 pcie_sleep_clk_ena : 1;
  u32 pcie_rchng_phy_clk_ena : 1;
  u32 reserved0 : 24;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_spare_clock_branch_ena_vote_1_u
{
  struct ipa_gcc_hwio_def_gcc_spare_clock_branch_ena_vote_1_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_SPARE_CLOCK_SLEEP_ENA_VOTE_1
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_spare_clock_sleep_ena_vote_1_s
{
  u32 pcie_slv_axi_clk_sleep_ena : 1;
  u32 pcie_mstr_axi_clk_sleep_ena : 1;
  u32 pcie_cfg_ahb_clk_sleep_ena : 1;
  u32 pcie_aux_clk_sleep_ena : 1;
  u32 pcie_pipe_clk_sleep_ena : 1;
  u32 pcie_slv_q2a_axi_clk_sleep_ena : 1;
  u32 pcie_sleep_clk_sleep_ena : 1;
  u32 pcie_rchng_phy_clk_sleep_ena : 1;
  u32 reserved0 : 24;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_spare_clock_sleep_ena_vote_1_u
{
  struct ipa_gcc_hwio_def_gcc_spare_clock_sleep_ena_vote_1_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_SPARE1_GPLL_ENA_VOTE
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_spare1_gpll_ena_vote_s
{
  u32 gpll0 : 1;
  u32 gpll1 : 1;
  u32 gpll2 : 1;
  u32 gpll3 : 1;
  u32 gpll4 : 1;
  u32 gpll5 : 1;
  u32 gpll6 : 1;
  u32 reserved0 : 25;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_spare1_gpll_ena_vote_u
{
  struct ipa_gcc_hwio_def_gcc_spare1_gpll_ena_vote_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_SPARE1_GPLL_SLEEP_ENA_VOTE
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_spare1_gpll_sleep_ena_vote_s
{
  u32 gpll0 : 1;
  u32 gpll1 : 1;
  u32 gpll2 : 1;
  u32 gpll3 : 1;
  u32 gpll4 : 1;
  u32 gpll5 : 1;
  u32 gpll6 : 1;
  u32 reserved0 : 25;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_spare1_gpll_sleep_ena_vote_u
{
  struct ipa_gcc_hwio_def_gcc_spare1_gpll_sleep_ena_vote_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_SPARE1_CLOCK_BRANCH_ENA_VOTE
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_spare1_clock_branch_ena_vote_s
{
  u32 sys_noc_cpuss_ahb_clk_ena : 1;
  u32 tcsr_ahb_clk_ena : 1;
  u32 qdss_cfg_ahb_clk_ena : 1;
  u32 ce1_ahb_clk_ena : 1;
  u32 ce1_axi_clk_ena : 1;
  u32 ce1_clk_ena : 1;
  u32 tlmm_clk_ena : 1;
  u32 ultaudio_pcnoc_sway_clk_ena : 1;
  u32 ultaudio_ahbfabric_ixfabric_clk_ena : 1;
  u32 reserved0 : 1;
  u32 boot_rom_ahb_clk_ena : 1;
  u32 reserved1 : 1;
  u32 tlmm_ahb_clk_ena : 1;
  u32 prng_ahb_clk_ena : 1;
  u32 blsp1_ahb_clk_ena : 1;
  u32 blsp1_sleep_clk_ena : 1;
  u32 reserved2 : 1;
  u32 mss_gpll0_div_clk_src_ena : 1;
  u32 reserved3 : 3;
  u32 cpuss_ahb_clk_ena : 1;
  u32 cpuss_gnoc_clk_ena : 1;
  u32 reserved4 : 1;
  u32 imem_axi_clk_ena : 1;
  u32 reserved5 : 7;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_spare1_clock_branch_ena_vote_u
{
  struct ipa_gcc_hwio_def_gcc_spare1_clock_branch_ena_vote_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_SPARE1_CLOCK_SLEEP_ENA_VOTE
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_spare1_clock_sleep_ena_vote_s
{
  u32 sys_noc_cpuss_ahb_clk_sleep_ena : 1;
  u32 tcsr_ahb_clk_sleep_ena : 1;
  u32 qdss_cfg_ahb_clk_sleep_ena : 1;
  u32 ce1_ahb_clk_sleep_ena : 1;
  u32 ce1_axi_clk_sleep_ena : 1;
  u32 ce1_clk_sleep_ena : 1;
  u32 tlmm_clk_sleep_ena : 1;
  u32 ultaudio_pcnoc_sway_clk_sleep_ena : 1;
  u32 ultaudio_ahbfabric_ixfabric_clk_sleep_ena : 1;
  u32 reserved0 : 1;
  u32 boot_rom_ahb_clk_sleep_ena : 1;
  u32 reserved1 : 1;
  u32 tlmm_ahb_clk_sleep_ena : 1;
  u32 prng_ahb_clk_sleep_ena : 1;
  u32 blsp1_ahb_clk_sleep_ena : 1;
  u32 blsp1_sleep_clk_sleep_ena : 1;
  u32 reserved2 : 1;
  u32 mss_gpll0_div_clk_src_sleep_ena : 1;
  u32 reserved3 : 3;
  u32 cpuss_ahb_clk_sleep_ena : 1;
  u32 cpuss_gnoc_clk_sleep_ena : 1;
  u32 reserved4 : 1;
  u32 imem_axi_clk_sleep_ena : 1;
  u32 reserved5 : 7;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_spare1_clock_sleep_ena_vote_u
{
  struct ipa_gcc_hwio_def_gcc_spare1_clock_sleep_ena_vote_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_SPARE1_CLOCK_BRANCH_ENA_VOTE_1
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_spare1_clock_branch_ena_vote_1_s
{
  u32 pcie_slv_axi_clk_ena : 1;
  u32 pcie_mstr_axi_clk_ena : 1;
  u32 pcie_cfg_ahb_clk_ena : 1;
  u32 pcie_aux_clk_ena : 1;
  u32 pcie_pipe_clk_ena : 1;
  u32 pcie_slv_q2a_axi_clk_ena : 1;
  u32 pcie_sleep_clk_ena : 1;
  u32 pcie_rchng_phy_clk_ena : 1;
  u32 reserved0 : 24;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_spare1_clock_branch_ena_vote_1_u
{
  struct ipa_gcc_hwio_def_gcc_spare1_clock_branch_ena_vote_1_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_SPARE1_CLOCK_SLEEP_ENA_VOTE_1
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_spare1_clock_sleep_ena_vote_1_s
{
  u32 pcie_slv_axi_clk_sleep_ena : 1;
  u32 pcie_mstr_axi_clk_sleep_ena : 1;
  u32 pcie_cfg_ahb_clk_sleep_ena : 1;
  u32 pcie_aux_clk_sleep_ena : 1;
  u32 pcie_pipe_clk_sleep_ena : 1;
  u32 pcie_slv_q2a_axi_clk_sleep_ena : 1;
  u32 pcie_sleep_clk_sleep_ena : 1;
  u32 pcie_rchng_phy_clk_sleep_ena : 1;
  u32 reserved0 : 24;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_spare1_clock_sleep_ena_vote_1_u
{
  struct ipa_gcc_hwio_def_gcc_spare1_clock_sleep_ena_vote_1_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_SPARE2_GPLL_ENA_VOTE
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_spare2_gpll_ena_vote_s
{
  u32 gpll0 : 1;
  u32 gpll1 : 1;
  u32 gpll2 : 1;
  u32 gpll3 : 1;
  u32 gpll4 : 1;
  u32 gpll5 : 1;
  u32 gpll6 : 1;
  u32 reserved0 : 25;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_spare2_gpll_ena_vote_u
{
  struct ipa_gcc_hwio_def_gcc_spare2_gpll_ena_vote_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_SPARE2_GPLL_SLEEP_ENA_VOTE
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_spare2_gpll_sleep_ena_vote_s
{
  u32 gpll0 : 1;
  u32 gpll1 : 1;
  u32 gpll2 : 1;
  u32 gpll3 : 1;
  u32 gpll4 : 1;
  u32 gpll5 : 1;
  u32 gpll6 : 1;
  u32 reserved0 : 25;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_spare2_gpll_sleep_ena_vote_u
{
  struct ipa_gcc_hwio_def_gcc_spare2_gpll_sleep_ena_vote_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_SPARE2_CLOCK_BRANCH_ENA_VOTE
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_spare2_clock_branch_ena_vote_s
{
  u32 sys_noc_cpuss_ahb_clk_ena : 1;
  u32 tcsr_ahb_clk_ena : 1;
  u32 qdss_cfg_ahb_clk_ena : 1;
  u32 ce1_ahb_clk_ena : 1;
  u32 ce1_axi_clk_ena : 1;
  u32 ce1_clk_ena : 1;
  u32 tlmm_clk_ena : 1;
  u32 ultaudio_pcnoc_sway_clk_ena : 1;
  u32 ultaudio_ahbfabric_ixfabric_clk_ena : 1;
  u32 reserved0 : 1;
  u32 boot_rom_ahb_clk_ena : 1;
  u32 reserved1 : 1;
  u32 tlmm_ahb_clk_ena : 1;
  u32 prng_ahb_clk_ena : 1;
  u32 blsp1_ahb_clk_ena : 1;
  u32 blsp1_sleep_clk_ena : 1;
  u32 reserved2 : 1;
  u32 mss_gpll0_div_clk_src_ena : 1;
  u32 reserved3 : 3;
  u32 cpuss_ahb_clk_ena : 1;
  u32 cpuss_gnoc_clk_ena : 1;
  u32 reserved4 : 1;
  u32 imem_axi_clk_ena : 1;
  u32 reserved5 : 7;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_spare2_clock_branch_ena_vote_u
{
  struct ipa_gcc_hwio_def_gcc_spare2_clock_branch_ena_vote_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_SPARE2_CLOCK_SLEEP_ENA_VOTE
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_spare2_clock_sleep_ena_vote_s
{
  u32 sys_noc_cpuss_ahb_clk_sleep_ena : 1;
  u32 tcsr_ahb_clk_sleep_ena : 1;
  u32 qdss_cfg_ahb_clk_sleep_ena : 1;
  u32 ce1_ahb_clk_sleep_ena : 1;
  u32 ce1_axi_clk_sleep_ena : 1;
  u32 ce1_clk_sleep_ena : 1;
  u32 tlmm_clk_sleep_ena : 1;
  u32 ultaudio_pcnoc_sway_clk_sleep_ena : 1;
  u32 ultaudio_ahbfabric_ixfabric_clk_sleep_ena : 1;
  u32 reserved0 : 1;
  u32 boot_rom_ahb_clk_sleep_ena : 1;
  u32 reserved1 : 1;
  u32 tlmm_ahb_clk_sleep_ena : 1;
  u32 prng_ahb_clk_sleep_ena : 1;
  u32 blsp1_ahb_clk_sleep_ena : 1;
  u32 blsp1_sleep_clk_sleep_ena : 1;
  u32 reserved2 : 1;
  u32 mss_gpll0_div_clk_src_sleep_ena : 1;
  u32 reserved3 : 3;
  u32 cpuss_ahb_clk_sleep_ena : 1;
  u32 cpuss_gnoc_clk_sleep_ena : 1;
  u32 reserved4 : 1;
  u32 imem_axi_clk_sleep_ena : 1;
  u32 reserved5 : 7;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_spare2_clock_sleep_ena_vote_u
{
  struct ipa_gcc_hwio_def_gcc_spare2_clock_sleep_ena_vote_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_SPARE2_CLOCK_BRANCH_ENA_VOTE_1
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_spare2_clock_branch_ena_vote_1_s
{
  u32 pcie_slv_axi_clk_ena : 1;
  u32 pcie_mstr_axi_clk_ena : 1;
  u32 pcie_cfg_ahb_clk_ena : 1;
  u32 pcie_aux_clk_ena : 1;
  u32 pcie_pipe_clk_ena : 1;
  u32 pcie_slv_q2a_axi_clk_ena : 1;
  u32 pcie_sleep_clk_ena : 1;
  u32 pcie_rchng_phy_clk_ena : 1;
  u32 reserved0 : 24;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_spare2_clock_branch_ena_vote_1_u
{
  struct ipa_gcc_hwio_def_gcc_spare2_clock_branch_ena_vote_1_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_SPARE2_CLOCK_SLEEP_ENA_VOTE_1
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_spare2_clock_sleep_ena_vote_1_s
{
  u32 pcie_slv_axi_clk_sleep_ena : 1;
  u32 pcie_mstr_axi_clk_sleep_ena : 1;
  u32 pcie_cfg_ahb_clk_sleep_ena : 1;
  u32 pcie_aux_clk_sleep_ena : 1;
  u32 pcie_pipe_clk_sleep_ena : 1;
  u32 pcie_slv_q2a_axi_clk_sleep_ena : 1;
  u32 pcie_sleep_clk_sleep_ena : 1;
  u32 pcie_rchng_phy_clk_sleep_ena : 1;
  u32 reserved0 : 24;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_spare2_clock_sleep_ena_vote_1_u
{
  struct ipa_gcc_hwio_def_gcc_spare2_clock_sleep_ena_vote_1_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_MSS_Q6_GPLL_ENA_VOTE
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_mss_q6_gpll_ena_vote_s
{
  u32 gpll0 : 1;
  u32 gpll1 : 1;
  u32 gpll2 : 1;
  u32 gpll3 : 1;
  u32 gpll4 : 1;
  u32 gpll5 : 1;
  u32 gpll6 : 1;
  u32 reserved0 : 25;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_mss_q6_gpll_ena_vote_u
{
  struct ipa_gcc_hwio_def_gcc_mss_q6_gpll_ena_vote_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_MSS_Q6_GPLL_SLEEP_ENA_VOTE
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_mss_q6_gpll_sleep_ena_vote_s
{
  u32 gpll0 : 1;
  u32 gpll1 : 1;
  u32 gpll2 : 1;
  u32 gpll3 : 1;
  u32 gpll4 : 1;
  u32 gpll5 : 1;
  u32 gpll6 : 1;
  u32 reserved0 : 25;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_mss_q6_gpll_sleep_ena_vote_u
{
  struct ipa_gcc_hwio_def_gcc_mss_q6_gpll_sleep_ena_vote_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_MSS_Q6_CLOCK_BRANCH_ENA_VOTE
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_mss_q6_clock_branch_ena_vote_s
{
  u32 sys_noc_cpuss_ahb_clk_ena : 1;
  u32 tcsr_ahb_clk_ena : 1;
  u32 qdss_cfg_ahb_clk_ena : 1;
  u32 ce1_ahb_clk_ena : 1;
  u32 ce1_axi_clk_ena : 1;
  u32 ce1_clk_ena : 1;
  u32 tlmm_clk_ena : 1;
  u32 ultaudio_pcnoc_sway_clk_ena : 1;
  u32 ultaudio_ahbfabric_ixfabric_clk_ena : 1;
  u32 reserved0 : 1;
  u32 boot_rom_ahb_clk_ena : 1;
  u32 reserved1 : 1;
  u32 tlmm_ahb_clk_ena : 1;
  u32 prng_ahb_clk_ena : 1;
  u32 blsp1_ahb_clk_ena : 1;
  u32 blsp1_sleep_clk_ena : 1;
  u32 reserved2 : 1;
  u32 mss_gpll0_div_clk_src_ena : 1;
  u32 reserved3 : 3;
  u32 cpuss_ahb_clk_ena : 1;
  u32 cpuss_gnoc_clk_ena : 1;
  u32 reserved4 : 1;
  u32 imem_axi_clk_ena : 1;
  u32 reserved5 : 7;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_mss_q6_clock_branch_ena_vote_u
{
  struct ipa_gcc_hwio_def_gcc_mss_q6_clock_branch_ena_vote_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_MSS_Q6_CLOCK_SLEEP_ENA_VOTE
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_mss_q6_clock_sleep_ena_vote_s
{
  u32 sys_noc_cpuss_ahb_clk_sleep_ena : 1;
  u32 tcsr_ahb_clk_sleep_ena : 1;
  u32 qdss_cfg_ahb_clk_sleep_ena : 1;
  u32 ce1_ahb_clk_sleep_ena : 1;
  u32 ce1_axi_clk_sleep_ena : 1;
  u32 ce1_clk_sleep_ena : 1;
  u32 tlmm_clk_sleep_ena : 1;
  u32 ultaudio_pcnoc_sway_clk_sleep_ena : 1;
  u32 ultaudio_ahbfabric_ixfabric_clk_sleep_ena : 1;
  u32 reserved0 : 1;
  u32 boot_rom_ahb_clk_sleep_ena : 1;
  u32 reserved1 : 1;
  u32 tlmm_ahb_clk_sleep_ena : 1;
  u32 prng_ahb_clk_sleep_ena : 1;
  u32 blsp1_ahb_clk_sleep_ena : 1;
  u32 blsp1_sleep_clk_sleep_ena : 1;
  u32 reserved2 : 1;
  u32 mss_gpll0_div_clk_src_sleep_ena : 1;
  u32 reserved3 : 3;
  u32 cpuss_ahb_clk_sleep_ena : 1;
  u32 cpuss_gnoc_clk_sleep_ena : 1;
  u32 reserved4 : 1;
  u32 imem_axi_clk_sleep_ena : 1;
  u32 reserved5 : 7;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_mss_q6_clock_sleep_ena_vote_u
{
  struct ipa_gcc_hwio_def_gcc_mss_q6_clock_sleep_ena_vote_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_MSS_Q6_CLOCK_BRANCH_ENA_VOTE_1
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_mss_q6_clock_branch_ena_vote_1_s
{
  u32 pcie_slv_axi_clk_ena : 1;
  u32 pcie_mstr_axi_clk_ena : 1;
  u32 pcie_cfg_ahb_clk_ena : 1;
  u32 pcie_aux_clk_ena : 1;
  u32 pcie_pipe_clk_ena : 1;
  u32 pcie_slv_q2a_axi_clk_ena : 1;
  u32 pcie_sleep_clk_ena : 1;
  u32 pcie_rchng_phy_clk_ena : 1;
  u32 reserved0 : 24;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_mss_q6_clock_branch_ena_vote_1_u
{
  struct ipa_gcc_hwio_def_gcc_mss_q6_clock_branch_ena_vote_1_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_MSS_Q6_CLOCK_SLEEP_ENA_VOTE_1
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_mss_q6_clock_sleep_ena_vote_1_s
{
  u32 pcie_slv_axi_clk_sleep_ena : 1;
  u32 pcie_mstr_axi_clk_sleep_ena : 1;
  u32 pcie_cfg_ahb_clk_sleep_ena : 1;
  u32 pcie_aux_clk_sleep_ena : 1;
  u32 pcie_pipe_clk_sleep_ena : 1;
  u32 pcie_slv_q2a_axi_clk_sleep_ena : 1;
  u32 pcie_sleep_clk_sleep_ena : 1;
  u32 pcie_rchng_phy_clk_sleep_ena : 1;
  u32 reserved0 : 24;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_mss_q6_clock_sleep_ena_vote_1_u
{
  struct ipa_gcc_hwio_def_gcc_mss_q6_clock_sleep_ena_vote_1_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_PCIE_MISC_RESET
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_pcie_misc_reset_s
{
  u32 pcie_slv_axi_bcr_blk_ares : 1;
  u32 pcie_mstr_axi_bcr_blk_ares : 1;
  u32 pcie_cfg_ahb_bcr_blk_ares : 1;
  u32 pcie_aux_bcr_blk_ares : 1;
  u32 pcie_pipe_bcr_blk_ares : 1;
  u32 pcie_mstr_axi_sticky_bcr_blk_ares : 1;
  u32 pcie_core_sticky_bcr_blk_ares : 1;
  u32 pcie_slv_axi_sticky_bcr_blk_ares : 1;
  u32 pcie_sleep_bcr_blk_ares : 1;
  u32 pcie_slv_axi_q2a_bcr_blk_ares : 1;
  u32 pcie_rchng_phy_bcr_blk_ares : 1;
  u32 pcie_cfg_ahb_bridge2mx_bcr_blk_ares : 1;
  u32 pcie_mstr_axi_bridge2mx_bcr_blk_ares : 1;
  u32 pcie_slv_axi_q2a_bridge2mx_bcr_blk_ares : 1;
  u32 reserved0 : 18;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_pcie_misc_reset_u
{
  struct ipa_gcc_hwio_def_gcc_pcie_misc_reset_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_DEBUG_CLK_CTL
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_debug_clk_ctl_s
{
  u32 mux_sel : 10;
  u32 plltest_de_sel : 1;
  u32 reserved0 : 3;
  u32 pll_lock_det_mux_sel : 5;
  u32 debug_bus_sel : 4;
  u32 reserved1 : 9;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_debug_clk_ctl_u
{
  struct ipa_gcc_hwio_def_gcc_debug_clk_ctl_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_CLOCK_FRQ_MEASURE_CTL
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_clock_frq_measure_ctl_s
{
  u32 xo_div4_term_cnt : 20;
  u32 cnt_en : 1;
  u32 clr_cnt : 1;
  u32 reserved0 : 10;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_clock_frq_measure_ctl_u
{
  struct ipa_gcc_hwio_def_gcc_clock_frq_measure_ctl_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_CLOCK_FRQ_MEASURE_STATUS
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_clock_frq_measure_status_s
{
  u32 measure_cnt : 25;
  u32 xo_div4_cnt_done : 1;
  u32 reserved0 : 6;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_clock_frq_measure_status_u
{
  struct ipa_gcc_hwio_def_gcc_clock_frq_measure_status_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_PLLTEST_PAD_CFG
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_plltest_pad_cfg_s
{
  u32 out_sel : 5;
  u32 reserve_bits10_5 : 6;
  u32 hdrive : 3;
  u32 hihys_en : 1;
  u32 core_ie : 1;
  u32 reserve_bit16 : 1;
  u32 core_oe : 1;
  u32 reserve_bit18 : 1;
  u32 core_pll_en : 1;
  u32 reserve_bits23_20 : 4;
  u32 core_pll_b : 2;
  u32 reserved0 : 6;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_plltest_pad_cfg_u
{
  struct ipa_gcc_hwio_def_gcc_plltest_pad_cfg_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_SYS_NOC_INTERFACE_FSM
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_sys_noc_interface_fsm_s
{
  u32 fsm_state : 5;
  u32 reserved0 : 27;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_sys_noc_interface_fsm_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_sys_noc_interface_fsm_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_CNOC_INTERFACE_FSM
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_cnoc_interface_fsm_s
{
  u32 fsm_state : 5;
  u32 reserved0 : 27;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_cnoc_interface_fsm_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_cnoc_interface_fsm_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_SHUB_INTERFACE_FSM
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_shub_interface_fsm_s
{
  u32 fsm_state : 5;
  u32 reserved0 : 27;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_shub_interface_fsm_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_shub_interface_fsm_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_CE_INTERFACE_FSM
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_ce_interface_fsm_s
{
  u32 fsm_state : 5;
  u32 reserved0 : 27;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_ce_interface_fsm_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_ce_interface_fsm_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_SHRM_INTERFACE_FSM
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_shrm_interface_fsm_s
{
  u32 fsm_state : 5;
  u32 reserved0 : 27;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_shrm_interface_fsm_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_shrm_interface_fsm_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_IPA_INTERFACE_FSM
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_ipa_interface_fsm_s
{
  u32 fsm_state : 5;
  u32 reserved0 : 27;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_ipa_interface_fsm_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_ipa_interface_fsm_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_QPIC_INTERFACE_FSM
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_qpic_interface_fsm_s
{
  u32 fsm_state : 5;
  u32 reserved0 : 27;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_qpic_interface_fsm_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_qpic_interface_fsm_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_PKA_INTERFACE_FSM
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_pka_interface_fsm_s
{
  u32 fsm_state : 5;
  u32 reserved0 : 27;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_pka_interface_fsm_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_pka_interface_fsm_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_DDRMC_INTERFACE_FSM
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_ddrmc_interface_fsm_s
{
  u32 fsm_state : 5;
  u32 reserved0 : 27;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_ddrmc_interface_fsm_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_ddrmc_interface_fsm_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_USB_BOOT_CLOCK_CTL
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_usb_boot_clock_ctl_s
{
  u32 clk_enable : 1;
  u32 reserved0 : 31;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_usb_boot_clock_ctl_u
{
  struct ipa_gcc_hwio_def_gcc_usb_boot_clock_ctl_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_PCIE_BOOT_CLOCK_CTL
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_pcie_boot_clock_ctl_s
{
  u32 clk_enable : 1;
  u32 reserved0 : 31;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_pcie_boot_clock_ctl_u
{
  struct ipa_gcc_hwio_def_gcc_pcie_boot_clock_ctl_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_TIC_MODE_APCS_BOOT
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_tic_mode_apcs_boot_s
{
  u32 apcs_boot_in_tic_mode : 1;
  u32 reserved0 : 31;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_tic_mode_apcs_boot_u
{
  struct ipa_gcc_hwio_def_gcc_tic_mode_apcs_boot_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_IPA_GDSC_OVRD
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_ipa_gdsc_ovrd_s
{
  u32 retain_ff_enable : 1;
  u32 sw_override : 1;
  u32 reserved0 : 30;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_ipa_gdsc_ovrd_u
{
  struct ipa_gcc_hwio_def_gcc_ipa_gdsc_ovrd_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_USB30_PRIM_GDSC_OVRD
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_usb30_prim_gdsc_ovrd_s
{
  u32 retain_ff_enable : 1;
  u32 sw_override : 1;
  u32 reserved0 : 30;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_usb30_prim_gdsc_ovrd_u
{
  struct ipa_gcc_hwio_def_gcc_usb30_prim_gdsc_ovrd_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_PCIE_0_GDSC_OVRD
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_pcie_0_gdsc_ovrd_s
{
  u32 retain_ff_enable : 1;
  u32 sw_override : 1;
  u32 reserved0 : 30;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_pcie_0_gdsc_ovrd_u
{
  struct ipa_gcc_hwio_def_gcc_pcie_0_gdsc_ovrd_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_DDRSS_GDSC_OVRD
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_ddrss_gdsc_ovrd_s
{
  u32 retain_ff_enable : 1;
  u32 sw_override : 1;
  u32 reserved0 : 30;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_ddrss_gdsc_ovrd_u
{
  struct ipa_gcc_hwio_def_gcc_ddrss_gdsc_ovrd_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_GDS_HW_CTRL_SPARE
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_gds_hw_ctrl_spare_s
{
  u32 spare : 8;
  u32 reserved0 : 24;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_gds_hw_ctrl_spare_u
{
  struct ipa_gcc_hwio_def_gcc_gds_hw_ctrl_spare_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_ARC_CLK_DIS_ACK_OVRD
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_arc_clk_dis_ack_ovrd_s
{
  u32 gcc_mx_clk_dis_ack_ovrd : 1;
  u32 nav_mx_clk_dis_ack_ovrd : 1;
  u32 apss_mx_clk_dis_ack_ovrd : 1;
  u32 mss_mx_clk_dis_ack_ovrd : 1;
  u32 ddr_phy_mx_clk_dis_ack_ovrd : 1;
  u32 reserved0 : 11;
  u32 gcc_cx_clk_dis_ack_ovrd : 1;
  u32 nav_cx_clk_dis_ack_ovrd : 1;
  u32 apss_cx_clk_dis_ack_ovrd : 1;
  u32 mss_cx_clk_dis_ack_ovrd : 1;
  u32 ddr_phy_cx_clk_dis_ack_ovrd : 1;
  u32 reserved1 : 11;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_arc_clk_dis_ack_ovrd_u
{
  struct ipa_gcc_hwio_def_gcc_arc_clk_dis_ack_ovrd_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_SPARE0_REG
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_spare0_reg_s
{
  u32 spare_bits : 32;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_spare0_reg_u
{
  struct ipa_gcc_hwio_def_gcc_spare0_reg_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_SPARE1_REG
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_spare1_reg_s
{
  u32 spare_bits : 32;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_spare1_reg_u
{
  struct ipa_gcc_hwio_def_gcc_spare1_reg_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_SPARE2_REG
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_spare2_reg_s
{
  u32 spare_bits : 32;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_spare2_reg_u
{
  struct ipa_gcc_hwio_def_gcc_spare2_reg_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_SPARE3_REG
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_spare3_reg_s
{
  u32 spare_bits : 32;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_spare3_reg_u
{
  struct ipa_gcc_hwio_def_gcc_spare3_reg_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RAW_SLEEP_CLK_CTRL
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_raw_sleep_clk_ctrl_s
{
  u32 gating_disable : 1;
  u32 reserved0 : 31;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_raw_sleep_clk_ctrl_u
{
  struct ipa_gcc_hwio_def_gcc_raw_sleep_clk_ctrl_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_TZ_VOTE_AGGRE_NOC_MMU_TBU1_CLK
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_tz_vote_aggre_noc_mmu_tbu1_clk_s
{
  u32 clk_enable : 1;
  u32 reserved0 : 30;
  u32 clk_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_tz_vote_aggre_noc_mmu_tbu1_clk_u
{
  struct ipa_gcc_hwio_def_gcc_tz_vote_aggre_noc_mmu_tbu1_clk_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_TZ_VOTE_AGGRE_NOC_MMU_TBU2_CLK
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_tz_vote_aggre_noc_mmu_tbu2_clk_s
{
  u32 clk_enable : 1;
  u32 reserved0 : 30;
  u32 clk_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_tz_vote_aggre_noc_mmu_tbu2_clk_u
{
  struct ipa_gcc_hwio_def_gcc_tz_vote_aggre_noc_mmu_tbu2_clk_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_TZ_VOTE_ALL_SMMU_MMU_CLK
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_tz_vote_all_smmu_mmu_clk_s
{
  u32 clk_enable : 1;
  u32 reserved0 : 30;
  u32 clk_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_tz_vote_all_smmu_mmu_clk_u
{
  struct ipa_gcc_hwio_def_gcc_tz_vote_all_smmu_mmu_clk_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_TZ_VOTE_MMU_TCU_CLK
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_tz_vote_mmu_tcu_clk_s
{
  u32 clk_enable : 1;
  u32 reserved0 : 30;
  u32 clk_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_tz_vote_mmu_tcu_clk_u
{
  struct ipa_gcc_hwio_def_gcc_tz_vote_mmu_tcu_clk_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_HYP_VOTE_AGGRE_NOC_MMU_TBU1_CLK
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_hyp_vote_aggre_noc_mmu_tbu1_clk_s
{
  u32 clk_enable : 1;
  u32 reserved0 : 30;
  u32 clk_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_hyp_vote_aggre_noc_mmu_tbu1_clk_u
{
  struct ipa_gcc_hwio_def_gcc_hyp_vote_aggre_noc_mmu_tbu1_clk_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_HYP_VOTE_AGGRE_NOC_MMU_TBU2_CLK
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_hyp_vote_aggre_noc_mmu_tbu2_clk_s
{
  u32 clk_enable : 1;
  u32 reserved0 : 30;
  u32 clk_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_hyp_vote_aggre_noc_mmu_tbu2_clk_u
{
  struct ipa_gcc_hwio_def_gcc_hyp_vote_aggre_noc_mmu_tbu2_clk_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_HYP_VOTE_ALL_SMMU_MMU_CLK
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_hyp_vote_all_smmu_mmu_clk_s
{
  u32 clk_enable : 1;
  u32 reserved0 : 30;
  u32 clk_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_hyp_vote_all_smmu_mmu_clk_u
{
  struct ipa_gcc_hwio_def_gcc_hyp_vote_all_smmu_mmu_clk_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_HYP_VOTE_MMU_TCU_CLK
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_hyp_vote_mmu_tcu_clk_s
{
  u32 clk_enable : 1;
  u32 reserved0 : 30;
  u32 clk_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_hyp_vote_mmu_tcu_clk_u
{
  struct ipa_gcc_hwio_def_gcc_hyp_vote_mmu_tcu_clk_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_HLOS1_VOTE_AGGRE_NOC_MMU_TBU1_CLK
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_hlos1_vote_aggre_noc_mmu_tbu1_clk_s
{
  u32 clk_enable : 1;
  u32 reserved0 : 30;
  u32 clk_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_hlos1_vote_aggre_noc_mmu_tbu1_clk_u
{
  struct ipa_gcc_hwio_def_gcc_hlos1_vote_aggre_noc_mmu_tbu1_clk_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_HLOS1_VOTE_AGGRE_NOC_MMU_TBU2_CLK
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_hlos1_vote_aggre_noc_mmu_tbu2_clk_s
{
  u32 clk_enable : 1;
  u32 reserved0 : 30;
  u32 clk_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_hlos1_vote_aggre_noc_mmu_tbu2_clk_u
{
  struct ipa_gcc_hwio_def_gcc_hlos1_vote_aggre_noc_mmu_tbu2_clk_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_HLOS1_VOTE_ALL_SMMU_MMU_CLK
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_hlos1_vote_all_smmu_mmu_clk_s
{
  u32 clk_enable : 1;
  u32 reserved0 : 30;
  u32 clk_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_hlos1_vote_all_smmu_mmu_clk_u
{
  struct ipa_gcc_hwio_def_gcc_hlos1_vote_all_smmu_mmu_clk_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_HLOS1_VOTE_MMU_TCU_CLK
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_hlos1_vote_mmu_tcu_clk_s
{
  u32 clk_enable : 1;
  u32 reserved0 : 30;
  u32 clk_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_hlos1_vote_mmu_tcu_clk_u
{
  struct ipa_gcc_hwio_def_gcc_hlos1_vote_mmu_tcu_clk_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_HLOS2_VOTE_AGGRE_NOC_MMU_TBU1_CLK
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_hlos2_vote_aggre_noc_mmu_tbu1_clk_s
{
  u32 clk_enable : 1;
  u32 reserved0 : 30;
  u32 clk_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_hlos2_vote_aggre_noc_mmu_tbu1_clk_u
{
  struct ipa_gcc_hwio_def_gcc_hlos2_vote_aggre_noc_mmu_tbu1_clk_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_HLOS2_VOTE_AGGRE_NOC_MMU_TBU2_CLK
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_hlos2_vote_aggre_noc_mmu_tbu2_clk_s
{
  u32 clk_enable : 1;
  u32 reserved0 : 30;
  u32 clk_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_hlos2_vote_aggre_noc_mmu_tbu2_clk_u
{
  struct ipa_gcc_hwio_def_gcc_hlos2_vote_aggre_noc_mmu_tbu2_clk_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_HLOS2_VOTE_ALL_SMMU_MMU_CLK
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_hlos2_vote_all_smmu_mmu_clk_s
{
  u32 clk_enable : 1;
  u32 reserved0 : 30;
  u32 clk_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_hlos2_vote_all_smmu_mmu_clk_u
{
  struct ipa_gcc_hwio_def_gcc_hlos2_vote_all_smmu_mmu_clk_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_HLOS2_VOTE_MMU_TCU_CLK
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_hlos2_vote_mmu_tcu_clk_s
{
  u32 clk_enable : 1;
  u32 reserved0 : 30;
  u32 clk_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_hlos2_vote_mmu_tcu_clk_u
{
  struct ipa_gcc_hwio_def_gcc_hlos2_vote_mmu_tcu_clk_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_USB3_PRIM_CLKREF_EN
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_usb3_prim_clkref_en_s
{
  u32 usb3_enable : 1;
  u32 reserved0 : 30;
  u32 usb3_status : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_usb3_prim_clkref_en_u
{
  struct ipa_gcc_hwio_def_gcc_usb3_prim_clkref_en_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_PCIE_0_CLKREF_EN
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_pcie_0_clkref_en_s
{
  u32 pcie_enable : 1;
  u32 reserved0 : 30;
  u32 pcie_status : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_pcie_0_clkref_en_u
{
  struct ipa_gcc_hwio_def_gcc_pcie_0_clkref_en_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RX1_USB2_CLKREF_EN
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rx1_usb2_clkref_en_s
{
  u32 rx1_usb2_enable : 1;
  u32 cref_enable : 1;
  u32 reserved0 : 29;
  u32 rx1_usb2_status : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rx1_usb2_clkref_en_u
{
  struct ipa_gcc_hwio_def_gcc_rx1_usb2_clkref_en_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RX2_QLINK_CLKREF_EN
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rx2_qlink_clkref_en_s
{
  u32 rx2_qlink_enable : 1;
  u32 rxtap0_enable : 1;
  u32 reserved0 : 29;
  u32 rx2_qlink_status : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rx2_qlink_clkref_en_u
{
  struct ipa_gcc_hwio_def_gcc_rx2_qlink_clkref_en_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RX3_MODEM_CLKREF_EN
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rx3_modem_clkref_en_s
{
  u32 rx3_modem_enable : 1;
  u32 rxtap1_enable : 1;
  u32 reserved0 : 29;
  u32 rx3_modem_status : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rx3_modem_clkref_en_u
{
  struct ipa_gcc_hwio_def_gcc_rx3_modem_clkref_en_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_CXO_TX1_CLKREF_EN1
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_cxo_tx1_clkref_en1_s
{
  u32 cxo_tx1_enable : 1;
  u32 reserved0 : 30;
  u32 cxo_tx1_status : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_cxo_tx1_clkref_en1_u
{
  struct ipa_gcc_hwio_def_gcc_cxo_tx1_clkref_en1_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_CLKREF_SPARE
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_clkref_spare_s
{
  u32 spare : 8;
  u32 reserved0 : 24;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_clkref_spare_u
{
  struct ipa_gcc_hwio_def_gcc_clkref_spare_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_CXO_REFGEN_BIAS_SEL
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_cxo_refgen_bias_sel_s
{
  u32 sel_refgen : 1;
  u32 reserved0 : 30;
  u32 sel_refgen_status : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_cxo_refgen_bias_sel_u
{
  struct ipa_gcc_hwio_def_gcc_cxo_refgen_bias_sel_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_MSS_VOTE_AGGRE_NOC_MMU_TBU1_CLK
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_mss_vote_aggre_noc_mmu_tbu1_clk_s
{
  u32 clk_enable : 1;
  u32 reserved0 : 30;
  u32 clk_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_mss_vote_aggre_noc_mmu_tbu1_clk_u
{
  struct ipa_gcc_hwio_def_gcc_mss_vote_aggre_noc_mmu_tbu1_clk_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_MSS_VOTE_AGGRE_NOC_MMU_TBU2_CLK
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_mss_vote_aggre_noc_mmu_tbu2_clk_s
{
  u32 clk_enable : 1;
  u32 reserved0 : 30;
  u32 clk_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_mss_vote_aggre_noc_mmu_tbu2_clk_u
{
  struct ipa_gcc_hwio_def_gcc_mss_vote_aggre_noc_mmu_tbu2_clk_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_MSS_VOTE_ALL_SMMU_MMU_CLK
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_mss_vote_all_smmu_mmu_clk_s
{
  u32 clk_enable : 1;
  u32 reserved0 : 30;
  u32 clk_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_mss_vote_all_smmu_mmu_clk_u
{
  struct ipa_gcc_hwio_def_gcc_mss_vote_all_smmu_mmu_clk_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_MSS_VOTE_MMU_TCU_CLK
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_mss_vote_mmu_tcu_clk_s
{
  u32 clk_enable : 1;
  u32 reserved0 : 30;
  u32 clk_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_mss_vote_mmu_tcu_clk_u
{
  struct ipa_gcc_hwio_def_gcc_mss_vote_mmu_tcu_clk_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPM_VOTE_QDSS_APB_CLK
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpm_vote_qdss_apb_clk_s
{
  u32 clk_enable : 1;
  u32 reserved0 : 30;
  u32 clk_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpm_vote_qdss_apb_clk_u
{
  struct ipa_gcc_hwio_def_gcc_rpm_vote_qdss_apb_clk_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_MSS_Q6_VOTE_QDSS_APB_CLK
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_mss_q6_vote_qdss_apb_clk_s
{
  u32 clk_enable : 1;
  u32 reserved0 : 30;
  u32 clk_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_mss_q6_vote_qdss_apb_clk_u
{
  struct ipa_gcc_hwio_def_gcc_mss_q6_vote_qdss_apb_clk_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_APCS_VOTE_QDSS_APB_CLK
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_apcs_vote_qdss_apb_clk_s
{
  u32 clk_enable : 1;
  u32 reserved0 : 30;
  u32 clk_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_apcs_vote_qdss_apb_clk_u
{
  struct ipa_gcc_hwio_def_gcc_apcs_vote_qdss_apb_clk_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_APCS_TZ_VOTE_QDSS_APB_CLK
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_apcs_tz_vote_qdss_apb_clk_s
{
  u32 clk_enable : 1;
  u32 reserved0 : 30;
  u32 clk_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_apcs_tz_vote_qdss_apb_clk_u
{
  struct ipa_gcc_hwio_def_gcc_apcs_tz_vote_qdss_apb_clk_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_HYP_VOTE_QDSS_APB_CLK
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_hyp_vote_qdss_apb_clk_s
{
  u32 clk_enable : 1;
  u32 reserved0 : 30;
  u32 clk_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_hyp_vote_qdss_apb_clk_u
{
  struct ipa_gcc_hwio_def_gcc_hyp_vote_qdss_apb_clk_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_SPARE1_VOTE_QDSS_APB_CLK
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_spare1_vote_qdss_apb_clk_s
{
  u32 clk_enable : 1;
  u32 reserved0 : 30;
  u32 clk_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_spare1_vote_qdss_apb_clk_u
{
  struct ipa_gcc_hwio_def_gcc_spare1_vote_qdss_apb_clk_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_SPARE2_VOTE_QDSS_APB_CLK
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_spare2_vote_qdss_apb_clk_s
{
  u32 clk_enable : 1;
  u32 reserved0 : 30;
  u32 clk_off : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_spare2_vote_qdss_apb_clk_u
{
  struct ipa_gcc_hwio_def_gcc_spare2_vote_qdss_apb_clk_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_JBIST_MODE
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_jbist_mode_s
{
  u32 sleep_n : 1;
  u32 reset_n : 1;
  u32 jbist_test : 1;
  u32 start_meas : 1;
  u32 reserve_bits31_4 : 28;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_jbist_mode_u
{
  struct ipa_gcc_hwio_def_gcc_jbist_mode_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_JBIST_CONFIG_CTL
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_jbist_config_ctl_s
{
  u32 jbist_config_ctl : 32;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_jbist_config_ctl_u
{
  struct ipa_gcc_hwio_def_gcc_jbist_config_ctl_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_JBIST_USER_CTL
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_jbist_user_ctl_s
{
  u32 jbist_user_ctl : 32;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_jbist_user_ctl_u
{
  struct ipa_gcc_hwio_def_gcc_jbist_user_ctl_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_JBIST_USER_CTL_U
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_jbist_user_ctl_u_s
{
  u32 jbist_user_ctl_u : 32;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_jbist_user_ctl_u_u
{
  struct ipa_gcc_hwio_def_gcc_jbist_user_ctl_u_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_JBIST_TEST_CTL
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_jbist_test_ctl_s
{
  u32 jbist_test_ctl : 32;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_jbist_test_ctl_u
{
  struct ipa_gcc_hwio_def_gcc_jbist_test_ctl_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_JBIST_STATUS
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_jbist_status_s
{
  u32 jbist_status : 32;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_jbist_status_u
{
  struct ipa_gcc_hwio_def_gcc_jbist_status_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_JBIST_MEAS_DONE
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_jbist_meas_done_s
{
  u32 jbist_data_stream_rdy : 1;
  u32 reserve_bits31_1 : 31;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_jbist_meas_done_u
{
  struct ipa_gcc_hwio_def_gcc_jbist_meas_done_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_JBIST_MISC
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_jbist_misc_s
{
  u32 clk_ext_sel : 2;
  u32 reserved0 : 30;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_jbist_misc_u
{
  struct ipa_gcc_hwio_def_gcc_jbist_misc_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_GLOBAL_EN
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_global_en_s
{
  u32 east_enable : 1;
  u32 west_enable : 1;
  u32 north_enable : 1;
  u32 south_enable : 1;
  u32 center_enable : 1;
  u32 peripherals_enable : 1;
  u32 rest_enable : 1;
  u32 mem_enable_0 : 1;
  u32 mem_enable_1 : 1;
  u32 mem_enable_2 : 1;
  u32 mem_enable_3 : 1;
  u32 mem_enable_4 : 1;
  u32 mem_enable_5 : 1;
  u32 mem_enable_6 : 1;
  u32 mem_enable_7 : 1;
  u32 spare_enable : 17;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_global_en_u
{
  struct ipa_gcc_hwio_def_gcc_global_en_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_USB3_LPC_GPLL0_ACGCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_usb3_lpc_gpll0_acgcr_s
{
  u32 reserved0 : 31;
  u32 clk_on : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_usb3_lpc_gpll0_acgcr_u
{
  struct ipa_gcc_hwio_def_gcc_usb3_lpc_gpll0_acgcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_USB3_LPC_GPLL4_ACGCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_usb3_lpc_gpll4_acgcr_s
{
  u32 reserved0 : 31;
  u32 clk_on : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_usb3_lpc_gpll4_acgcr_u
{
  struct ipa_gcc_hwio_def_gcc_usb3_lpc_gpll4_acgcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_CPUSS_GPLL1_ACGCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_cpuss_gpll1_acgcr_s
{
  u32 reserved0 : 31;
  u32 clk_on : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_cpuss_gpll1_acgcr_u
{
  struct ipa_gcc_hwio_def_gcc_cpuss_gpll1_acgcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_CPUSS_GPLL4_ACGCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_cpuss_gpll4_acgcr_s
{
  u32 reserved0 : 31;
  u32 clk_on : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_cpuss_gpll4_acgcr_u
{
  struct ipa_gcc_hwio_def_gcc_cpuss_gpll4_acgcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_CPUSS_GPLL5_ACGCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_cpuss_gpll5_acgcr_s
{
  u32 reserved0 : 31;
  u32 clk_on : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_cpuss_gpll5_acgcr_u
{
  struct ipa_gcc_hwio_def_gcc_cpuss_gpll5_acgcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_MSS_GPLL0_DIV_ACGCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_mss_gpll0_div_acgcr_s
{
  u32 reserved0 : 31;
  u32 clk_on : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_mss_gpll0_div_acgcr_u
{
  struct ipa_gcc_hwio_def_gcc_mss_gpll0_div_acgcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_PLL_MISC
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_pll_misc_s
{
  u32 hw_triggered_stby_dis : 1;
  u32 reserved0 : 31;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_pll_misc_u
{
  struct ipa_gcc_hwio_def_gcc_pll_misc_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_PLL_MISC1
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_pll_misc1_s
{
  u32 pll0_pll_active_mux : 1;
  u32 pll1_pll_active_mux : 1;
  u32 pll2_pll_active_mux : 1;
  u32 pll3_pll_active_mux : 1;
  u32 pll4_pll_active_mux : 1;
  u32 pll5_pll_active_mux : 1;
  u32 reserved0 : 26;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_pll_misc1_u
{
  struct ipa_gcc_hwio_def_gcc_pll_misc1_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_PCIE_PERST_HANDSHAKE
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_pcie_perst_handshake_s
{
  u32 timer_enable : 1;
  u32 reserved0 : 28;
  u32 fsm_status : 2;
  u32 timeout_status : 1;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_pcie_perst_handshake_u
{
  struct ipa_gcc_hwio_def_gcc_pcie_perst_handshake_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_PCIE_PERST_HANDSHAKE_TIMER
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_pcie_perst_handshake_timer_s
{
  u32 timer_val : 32;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_pcie_perst_handshake_timer_u
{
  struct ipa_gcc_hwio_def_gcc_pcie_perst_handshake_timer_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_SYS_NOC_PERF0_ENA_VOTE
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_sys_noc_perf0_ena_vote_s
{
  u32 gpll0 : 1;
  u32 gpll1 : 1;
  u32 gpll2_3 : 1;
  u32 gpll4 : 1;
  u32 gpll5 : 1;
  u32 reserved0 : 27;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_sys_noc_perf0_ena_vote_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_sys_noc_perf0_ena_vote_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_SYS_NOC_PERF1_ENA_VOTE
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_sys_noc_perf1_ena_vote_s
{
  u32 gpll0 : 1;
  u32 gpll1 : 1;
  u32 gpll2_3 : 1;
  u32 gpll4 : 1;
  u32 gpll5 : 1;
  u32 reserved0 : 27;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_sys_noc_perf1_ena_vote_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_sys_noc_perf1_ena_vote_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_SYS_NOC_PERF2_ENA_VOTE
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_sys_noc_perf2_ena_vote_s
{
  u32 gpll0 : 1;
  u32 gpll1 : 1;
  u32 gpll2_3 : 1;
  u32 gpll4 : 1;
  u32 gpll5 : 1;
  u32 reserved0 : 27;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_sys_noc_perf2_ena_vote_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_sys_noc_perf2_ena_vote_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_SYS_NOC_PERF3_ENA_VOTE
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_sys_noc_perf3_ena_vote_s
{
  u32 gpll0 : 1;
  u32 gpll1 : 1;
  u32 gpll2_3 : 1;
  u32 gpll4 : 1;
  u32 gpll5 : 1;
  u32 reserved0 : 27;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_sys_noc_perf3_ena_vote_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_sys_noc_perf3_ena_vote_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_SYS_NOC_PERF4_ENA_VOTE
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_sys_noc_perf4_ena_vote_s
{
  u32 gpll0 : 1;
  u32 gpll1 : 1;
  u32 gpll2_3 : 1;
  u32 gpll4 : 1;
  u32 gpll5 : 1;
  u32 reserved0 : 27;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_sys_noc_perf4_ena_vote_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_sys_noc_perf4_ena_vote_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_SYS_NOC_PERF5_ENA_VOTE
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_sys_noc_perf5_ena_vote_s
{
  u32 gpll0 : 1;
  u32 gpll1 : 1;
  u32 gpll2_3 : 1;
  u32 gpll4 : 1;
  u32 gpll5 : 1;
  u32 reserved0 : 27;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_sys_noc_perf5_ena_vote_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_sys_noc_perf5_ena_vote_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_SYS_NOC_PERF6_ENA_VOTE
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_sys_noc_perf6_ena_vote_s
{
  u32 gpll0 : 1;
  u32 gpll1 : 1;
  u32 gpll2_3 : 1;
  u32 gpll4 : 1;
  u32 gpll5 : 1;
  u32 reserved0 : 27;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_sys_noc_perf6_ena_vote_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_sys_noc_perf6_ena_vote_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_SYS_NOC_PERF7_ENA_VOTE
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_sys_noc_perf7_ena_vote_s
{
  u32 gpll0 : 1;
  u32 gpll1 : 1;
  u32 gpll2_3 : 1;
  u32 gpll4 : 1;
  u32 gpll5 : 1;
  u32 reserved0 : 27;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_sys_noc_perf7_ena_vote_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_sys_noc_perf7_ena_vote_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_SYS_NOC_PERF8_ENA_VOTE
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_sys_noc_perf8_ena_vote_s
{
  u32 gpll0 : 1;
  u32 gpll1 : 1;
  u32 gpll2_3 : 1;
  u32 gpll4 : 1;
  u32 gpll5 : 1;
  u32 reserved0 : 27;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_sys_noc_perf8_ena_vote_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_sys_noc_perf8_ena_vote_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_SYS_NOC_PERF9_ENA_VOTE
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_sys_noc_perf9_ena_vote_s
{
  u32 gpll0 : 1;
  u32 gpll1 : 1;
  u32 gpll2_3 : 1;
  u32 gpll4 : 1;
  u32 gpll5 : 1;
  u32 reserved0 : 27;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_sys_noc_perf9_ena_vote_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_sys_noc_perf9_ena_vote_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_SYS_NOC_PERF10_ENA_VOTE
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_sys_noc_perf10_ena_vote_s
{
  u32 gpll0 : 1;
  u32 gpll1 : 1;
  u32 gpll2_3 : 1;
  u32 gpll4 : 1;
  u32 gpll5 : 1;
  u32 reserved0 : 27;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_sys_noc_perf10_ena_vote_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_sys_noc_perf10_ena_vote_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_SYS_NOC_PERF11_ENA_VOTE
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_sys_noc_perf11_ena_vote_s
{
  u32 gpll0 : 1;
  u32 gpll1 : 1;
  u32 gpll2_3 : 1;
  u32 gpll4 : 1;
  u32 gpll5 : 1;
  u32 reserved0 : 27;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_sys_noc_perf11_ena_vote_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_sys_noc_perf11_ena_vote_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_SYS_NOC_PERF12_ENA_VOTE
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_sys_noc_perf12_ena_vote_s
{
  u32 gpll0 : 1;
  u32 gpll1 : 1;
  u32 gpll2_3 : 1;
  u32 gpll4 : 1;
  u32 gpll5 : 1;
  u32 reserved0 : 27;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_sys_noc_perf12_ena_vote_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_sys_noc_perf12_ena_vote_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_SYS_NOC_PERF13_ENA_VOTE
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_sys_noc_perf13_ena_vote_s
{
  u32 gpll0 : 1;
  u32 gpll1 : 1;
  u32 gpll2_3 : 1;
  u32 gpll4 : 1;
  u32 gpll5 : 1;
  u32 reserved0 : 27;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_sys_noc_perf13_ena_vote_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_sys_noc_perf13_ena_vote_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_SYS_NOC_PERF14_ENA_VOTE
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_sys_noc_perf14_ena_vote_s
{
  u32 gpll0 : 1;
  u32 gpll1 : 1;
  u32 gpll2_3 : 1;
  u32 gpll4 : 1;
  u32 gpll5 : 1;
  u32 reserved0 : 27;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_sys_noc_perf14_ena_vote_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_sys_noc_perf14_ena_vote_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_SYS_NOC_PERF15_ENA_VOTE
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_sys_noc_perf15_ena_vote_s
{
  u32 gpll0 : 1;
  u32 gpll1 : 1;
  u32 gpll2_3 : 1;
  u32 gpll4 : 1;
  u32 gpll5 : 1;
  u32 reserved0 : 27;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_sys_noc_perf15_ena_vote_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_sys_noc_perf15_ena_vote_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_CNOC_PERF0_ENA_VOTE
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_cnoc_perf0_ena_vote_s
{
  u32 gpll0 : 1;
  u32 gpll1 : 1;
  u32 gpll2_3 : 1;
  u32 gpll4 : 1;
  u32 gpll5 : 1;
  u32 reserved0 : 27;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_cnoc_perf0_ena_vote_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_cnoc_perf0_ena_vote_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_CNOC_PERF1_ENA_VOTE
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_cnoc_perf1_ena_vote_s
{
  u32 gpll0 : 1;
  u32 gpll1 : 1;
  u32 gpll2_3 : 1;
  u32 gpll4 : 1;
  u32 gpll5 : 1;
  u32 reserved0 : 27;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_cnoc_perf1_ena_vote_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_cnoc_perf1_ena_vote_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_CNOC_PERF2_ENA_VOTE
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_cnoc_perf2_ena_vote_s
{
  u32 gpll0 : 1;
  u32 gpll1 : 1;
  u32 gpll2_3 : 1;
  u32 gpll4 : 1;
  u32 gpll5 : 1;
  u32 reserved0 : 27;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_cnoc_perf2_ena_vote_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_cnoc_perf2_ena_vote_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_CNOC_PERF3_ENA_VOTE
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_cnoc_perf3_ena_vote_s
{
  u32 gpll0 : 1;
  u32 gpll1 : 1;
  u32 gpll2_3 : 1;
  u32 gpll4 : 1;
  u32 gpll5 : 1;
  u32 reserved0 : 27;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_cnoc_perf3_ena_vote_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_cnoc_perf3_ena_vote_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_CNOC_PERF4_ENA_VOTE
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_cnoc_perf4_ena_vote_s
{
  u32 gpll0 : 1;
  u32 gpll1 : 1;
  u32 gpll2_3 : 1;
  u32 gpll4 : 1;
  u32 gpll5 : 1;
  u32 reserved0 : 27;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_cnoc_perf4_ena_vote_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_cnoc_perf4_ena_vote_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_CNOC_PERF5_ENA_VOTE
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_cnoc_perf5_ena_vote_s
{
  u32 gpll0 : 1;
  u32 gpll1 : 1;
  u32 gpll2_3 : 1;
  u32 gpll4 : 1;
  u32 gpll5 : 1;
  u32 reserved0 : 27;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_cnoc_perf5_ena_vote_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_cnoc_perf5_ena_vote_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_CNOC_PERF6_ENA_VOTE
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_cnoc_perf6_ena_vote_s
{
  u32 gpll0 : 1;
  u32 gpll1 : 1;
  u32 gpll2_3 : 1;
  u32 gpll4 : 1;
  u32 gpll5 : 1;
  u32 reserved0 : 27;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_cnoc_perf6_ena_vote_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_cnoc_perf6_ena_vote_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_CNOC_PERF7_ENA_VOTE
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_cnoc_perf7_ena_vote_s
{
  u32 gpll0 : 1;
  u32 gpll1 : 1;
  u32 gpll2_3 : 1;
  u32 gpll4 : 1;
  u32 gpll5 : 1;
  u32 reserved0 : 27;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_cnoc_perf7_ena_vote_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_cnoc_perf7_ena_vote_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_CNOC_PERF8_ENA_VOTE
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_cnoc_perf8_ena_vote_s
{
  u32 gpll0 : 1;
  u32 gpll1 : 1;
  u32 gpll2_3 : 1;
  u32 gpll4 : 1;
  u32 gpll5 : 1;
  u32 reserved0 : 27;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_cnoc_perf8_ena_vote_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_cnoc_perf8_ena_vote_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_CNOC_PERF9_ENA_VOTE
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_cnoc_perf9_ena_vote_s
{
  u32 gpll0 : 1;
  u32 gpll1 : 1;
  u32 gpll2_3 : 1;
  u32 gpll4 : 1;
  u32 gpll5 : 1;
  u32 reserved0 : 27;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_cnoc_perf9_ena_vote_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_cnoc_perf9_ena_vote_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_CNOC_PERF10_ENA_VOTE
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_cnoc_perf10_ena_vote_s
{
  u32 gpll0 : 1;
  u32 gpll1 : 1;
  u32 gpll2_3 : 1;
  u32 gpll4 : 1;
  u32 gpll5 : 1;
  u32 reserved0 : 27;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_cnoc_perf10_ena_vote_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_cnoc_perf10_ena_vote_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_CNOC_PERF11_ENA_VOTE
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_cnoc_perf11_ena_vote_s
{
  u32 gpll0 : 1;
  u32 gpll1 : 1;
  u32 gpll2_3 : 1;
  u32 gpll4 : 1;
  u32 gpll5 : 1;
  u32 reserved0 : 27;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_cnoc_perf11_ena_vote_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_cnoc_perf11_ena_vote_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_CNOC_PERF12_ENA_VOTE
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_cnoc_perf12_ena_vote_s
{
  u32 gpll0 : 1;
  u32 gpll1 : 1;
  u32 gpll2_3 : 1;
  u32 gpll4 : 1;
  u32 gpll5 : 1;
  u32 reserved0 : 27;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_cnoc_perf12_ena_vote_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_cnoc_perf12_ena_vote_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_CNOC_PERF13_ENA_VOTE
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_cnoc_perf13_ena_vote_s
{
  u32 gpll0 : 1;
  u32 gpll1 : 1;
  u32 gpll2_3 : 1;
  u32 gpll4 : 1;
  u32 gpll5 : 1;
  u32 reserved0 : 27;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_cnoc_perf13_ena_vote_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_cnoc_perf13_ena_vote_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_CNOC_PERF14_ENA_VOTE
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_cnoc_perf14_ena_vote_s
{
  u32 gpll0 : 1;
  u32 gpll1 : 1;
  u32 gpll2_3 : 1;
  u32 gpll4 : 1;
  u32 gpll5 : 1;
  u32 reserved0 : 27;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_cnoc_perf14_ena_vote_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_cnoc_perf14_ena_vote_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_CNOC_PERF15_ENA_VOTE
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_cnoc_perf15_ena_vote_s
{
  u32 gpll0 : 1;
  u32 gpll1 : 1;
  u32 gpll2_3 : 1;
  u32 gpll4 : 1;
  u32 gpll5 : 1;
  u32 reserved0 : 27;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_cnoc_perf15_ena_vote_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_cnoc_perf15_ena_vote_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_IPA_PERF0_ENA_VOTE
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_ipa_perf0_ena_vote_s
{
  u32 gpll0 : 1;
  u32 gpll1 : 1;
  u32 gpll2_3 : 1;
  u32 gpll4 : 1;
  u32 gpll5 : 1;
  u32 reserved0 : 27;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_ipa_perf0_ena_vote_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_ipa_perf0_ena_vote_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_IPA_PERF1_ENA_VOTE
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_ipa_perf1_ena_vote_s
{
  u32 gpll0 : 1;
  u32 gpll1 : 1;
  u32 gpll2_3 : 1;
  u32 gpll4 : 1;
  u32 gpll5 : 1;
  u32 reserved0 : 27;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_ipa_perf1_ena_vote_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_ipa_perf1_ena_vote_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_IPA_PERF2_ENA_VOTE
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_ipa_perf2_ena_vote_s
{
  u32 gpll0 : 1;
  u32 gpll1 : 1;
  u32 gpll2_3 : 1;
  u32 gpll4 : 1;
  u32 gpll5 : 1;
  u32 reserved0 : 27;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_ipa_perf2_ena_vote_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_ipa_perf2_ena_vote_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_IPA_PERF3_ENA_VOTE
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_ipa_perf3_ena_vote_s
{
  u32 gpll0 : 1;
  u32 gpll1 : 1;
  u32 gpll2_3 : 1;
  u32 gpll4 : 1;
  u32 gpll5 : 1;
  u32 reserved0 : 27;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_ipa_perf3_ena_vote_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_ipa_perf3_ena_vote_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_IPA_PERF4_ENA_VOTE
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_ipa_perf4_ena_vote_s
{
  u32 gpll0 : 1;
  u32 gpll1 : 1;
  u32 gpll2_3 : 1;
  u32 gpll4 : 1;
  u32 gpll5 : 1;
  u32 reserved0 : 27;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_ipa_perf4_ena_vote_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_ipa_perf4_ena_vote_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_IPA_PERF5_ENA_VOTE
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_ipa_perf5_ena_vote_s
{
  u32 gpll0 : 1;
  u32 gpll1 : 1;
  u32 gpll2_3 : 1;
  u32 gpll4 : 1;
  u32 gpll5 : 1;
  u32 reserved0 : 27;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_ipa_perf5_ena_vote_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_ipa_perf5_ena_vote_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_IPA_PERF6_ENA_VOTE
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_ipa_perf6_ena_vote_s
{
  u32 gpll0 : 1;
  u32 gpll1 : 1;
  u32 gpll2_3 : 1;
  u32 gpll4 : 1;
  u32 gpll5 : 1;
  u32 reserved0 : 27;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_ipa_perf6_ena_vote_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_ipa_perf6_ena_vote_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_IPA_PERF7_ENA_VOTE
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_ipa_perf7_ena_vote_s
{
  u32 gpll0 : 1;
  u32 gpll1 : 1;
  u32 gpll2_3 : 1;
  u32 gpll4 : 1;
  u32 gpll5 : 1;
  u32 reserved0 : 27;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_ipa_perf7_ena_vote_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_ipa_perf7_ena_vote_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_IPA_PERF8_ENA_VOTE
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_ipa_perf8_ena_vote_s
{
  u32 gpll0 : 1;
  u32 gpll1 : 1;
  u32 gpll2_3 : 1;
  u32 gpll4 : 1;
  u32 gpll5 : 1;
  u32 reserved0 : 27;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_ipa_perf8_ena_vote_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_ipa_perf8_ena_vote_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_IPA_PERF9_ENA_VOTE
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_ipa_perf9_ena_vote_s
{
  u32 gpll0 : 1;
  u32 gpll1 : 1;
  u32 gpll2_3 : 1;
  u32 gpll4 : 1;
  u32 gpll5 : 1;
  u32 reserved0 : 27;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_ipa_perf9_ena_vote_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_ipa_perf9_ena_vote_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_IPA_PERF10_ENA_VOTE
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_ipa_perf10_ena_vote_s
{
  u32 gpll0 : 1;
  u32 gpll1 : 1;
  u32 gpll2_3 : 1;
  u32 gpll4 : 1;
  u32 gpll5 : 1;
  u32 reserved0 : 27;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_ipa_perf10_ena_vote_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_ipa_perf10_ena_vote_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_IPA_PERF11_ENA_VOTE
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_ipa_perf11_ena_vote_s
{
  u32 gpll0 : 1;
  u32 gpll1 : 1;
  u32 gpll2_3 : 1;
  u32 gpll4 : 1;
  u32 gpll5 : 1;
  u32 reserved0 : 27;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_ipa_perf11_ena_vote_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_ipa_perf11_ena_vote_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_IPA_PERF12_ENA_VOTE
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_ipa_perf12_ena_vote_s
{
  u32 gpll0 : 1;
  u32 gpll1 : 1;
  u32 gpll2_3 : 1;
  u32 gpll4 : 1;
  u32 gpll5 : 1;
  u32 reserved0 : 27;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_ipa_perf12_ena_vote_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_ipa_perf12_ena_vote_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_IPA_PERF13_ENA_VOTE
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_ipa_perf13_ena_vote_s
{
  u32 gpll0 : 1;
  u32 gpll1 : 1;
  u32 gpll2_3 : 1;
  u32 gpll4 : 1;
  u32 gpll5 : 1;
  u32 reserved0 : 27;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_ipa_perf13_ena_vote_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_ipa_perf13_ena_vote_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_IPA_PERF14_ENA_VOTE
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_ipa_perf14_ena_vote_s
{
  u32 gpll0 : 1;
  u32 gpll1 : 1;
  u32 gpll2_3 : 1;
  u32 gpll4 : 1;
  u32 gpll5 : 1;
  u32 reserved0 : 27;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_ipa_perf14_ena_vote_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_ipa_perf14_ena_vote_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_IPA_PERF15_ENA_VOTE
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_ipa_perf15_ena_vote_s
{
  u32 gpll0 : 1;
  u32 gpll1 : 1;
  u32 gpll2_3 : 1;
  u32 gpll4 : 1;
  u32 gpll5 : 1;
  u32 reserved0 : 27;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_ipa_perf15_ena_vote_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_ipa_perf15_ena_vote_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_QPIC_PERF0_ENA_VOTE
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_qpic_perf0_ena_vote_s
{
  u32 gpll0 : 1;
  u32 gpll1 : 1;
  u32 gpll2_3 : 1;
  u32 gpll4 : 1;
  u32 gpll5 : 1;
  u32 reserved0 : 27;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_qpic_perf0_ena_vote_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_qpic_perf0_ena_vote_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_QPIC_PERF1_ENA_VOTE
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_qpic_perf1_ena_vote_s
{
  u32 gpll0 : 1;
  u32 gpll1 : 1;
  u32 gpll2_3 : 1;
  u32 gpll4 : 1;
  u32 gpll5 : 1;
  u32 reserved0 : 27;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_qpic_perf1_ena_vote_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_qpic_perf1_ena_vote_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_QPIC_PERF2_ENA_VOTE
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_qpic_perf2_ena_vote_s
{
  u32 gpll0 : 1;
  u32 gpll1 : 1;
  u32 gpll2_3 : 1;
  u32 gpll4 : 1;
  u32 gpll5 : 1;
  u32 reserved0 : 27;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_qpic_perf2_ena_vote_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_qpic_perf2_ena_vote_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_QPIC_PERF3_ENA_VOTE
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_qpic_perf3_ena_vote_s
{
  u32 gpll0 : 1;
  u32 gpll1 : 1;
  u32 gpll2_3 : 1;
  u32 gpll4 : 1;
  u32 gpll5 : 1;
  u32 reserved0 : 27;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_qpic_perf3_ena_vote_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_qpic_perf3_ena_vote_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_QPIC_PERF4_ENA_VOTE
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_qpic_perf4_ena_vote_s
{
  u32 gpll0 : 1;
  u32 gpll1 : 1;
  u32 gpll2_3 : 1;
  u32 gpll4 : 1;
  u32 gpll5 : 1;
  u32 reserved0 : 27;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_qpic_perf4_ena_vote_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_qpic_perf4_ena_vote_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_QPIC_PERF5_ENA_VOTE
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_qpic_perf5_ena_vote_s
{
  u32 gpll0 : 1;
  u32 gpll1 : 1;
  u32 gpll2_3 : 1;
  u32 gpll4 : 1;
  u32 gpll5 : 1;
  u32 reserved0 : 27;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_qpic_perf5_ena_vote_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_qpic_perf5_ena_vote_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_QPIC_PERF6_ENA_VOTE
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_qpic_perf6_ena_vote_s
{
  u32 gpll0 : 1;
  u32 gpll1 : 1;
  u32 gpll2_3 : 1;
  u32 gpll4 : 1;
  u32 gpll5 : 1;
  u32 reserved0 : 27;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_qpic_perf6_ena_vote_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_qpic_perf6_ena_vote_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_QPIC_PERF7_ENA_VOTE
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_qpic_perf7_ena_vote_s
{
  u32 gpll0 : 1;
  u32 gpll1 : 1;
  u32 gpll2_3 : 1;
  u32 gpll4 : 1;
  u32 gpll5 : 1;
  u32 reserved0 : 27;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_qpic_perf7_ena_vote_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_qpic_perf7_ena_vote_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_QPIC_PERF8_ENA_VOTE
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_qpic_perf8_ena_vote_s
{
  u32 gpll0 : 1;
  u32 gpll1 : 1;
  u32 gpll2_3 : 1;
  u32 gpll4 : 1;
  u32 gpll5 : 1;
  u32 reserved0 : 27;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_qpic_perf8_ena_vote_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_qpic_perf8_ena_vote_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_QPIC_PERF9_ENA_VOTE
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_qpic_perf9_ena_vote_s
{
  u32 gpll0 : 1;
  u32 gpll1 : 1;
  u32 gpll2_3 : 1;
  u32 gpll4 : 1;
  u32 gpll5 : 1;
  u32 reserved0 : 27;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_qpic_perf9_ena_vote_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_qpic_perf9_ena_vote_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_QPIC_PERF10_ENA_VOTE
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_qpic_perf10_ena_vote_s
{
  u32 gpll0 : 1;
  u32 gpll1 : 1;
  u32 gpll2_3 : 1;
  u32 gpll4 : 1;
  u32 gpll5 : 1;
  u32 reserved0 : 27;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_qpic_perf10_ena_vote_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_qpic_perf10_ena_vote_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_QPIC_PERF11_ENA_VOTE
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_qpic_perf11_ena_vote_s
{
  u32 gpll0 : 1;
  u32 gpll1 : 1;
  u32 gpll2_3 : 1;
  u32 gpll4 : 1;
  u32 gpll5 : 1;
  u32 reserved0 : 27;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_qpic_perf11_ena_vote_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_qpic_perf11_ena_vote_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_QPIC_PERF12_ENA_VOTE
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_qpic_perf12_ena_vote_s
{
  u32 gpll0 : 1;
  u32 gpll1 : 1;
  u32 gpll2_3 : 1;
  u32 gpll4 : 1;
  u32 gpll5 : 1;
  u32 reserved0 : 27;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_qpic_perf12_ena_vote_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_qpic_perf12_ena_vote_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_QPIC_PERF13_ENA_VOTE
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_qpic_perf13_ena_vote_s
{
  u32 gpll0 : 1;
  u32 gpll1 : 1;
  u32 gpll2_3 : 1;
  u32 gpll4 : 1;
  u32 gpll5 : 1;
  u32 reserved0 : 27;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_qpic_perf13_ena_vote_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_qpic_perf13_ena_vote_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_QPIC_PERF14_ENA_VOTE
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_qpic_perf14_ena_vote_s
{
  u32 gpll0 : 1;
  u32 gpll1 : 1;
  u32 gpll2_3 : 1;
  u32 gpll4 : 1;
  u32 gpll5 : 1;
  u32 reserved0 : 27;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_qpic_perf14_ena_vote_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_qpic_perf14_ena_vote_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_QPIC_PERF15_ENA_VOTE
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_qpic_perf15_ena_vote_s
{
  u32 gpll0 : 1;
  u32 gpll1 : 1;
  u32 gpll2_3 : 1;
  u32 gpll4 : 1;
  u32 gpll5 : 1;
  u32 reserved0 : 27;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_qpic_perf15_ena_vote_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_qpic_perf15_ena_vote_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_PKA_PERF0_ENA_VOTE
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_pka_perf0_ena_vote_s
{
  u32 gpll0 : 1;
  u32 gpll1 : 1;
  u32 gpll2_3 : 1;
  u32 gpll4 : 1;
  u32 gpll5 : 1;
  u32 reserved0 : 27;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_pka_perf0_ena_vote_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_pka_perf0_ena_vote_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_PKA_PERF1_ENA_VOTE
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_pka_perf1_ena_vote_s
{
  u32 gpll0 : 1;
  u32 gpll1 : 1;
  u32 gpll2_3 : 1;
  u32 gpll4 : 1;
  u32 gpll5 : 1;
  u32 reserved0 : 27;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_pka_perf1_ena_vote_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_pka_perf1_ena_vote_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_PKA_PERF2_ENA_VOTE
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_pka_perf2_ena_vote_s
{
  u32 gpll0 : 1;
  u32 gpll1 : 1;
  u32 gpll2_3 : 1;
  u32 gpll4 : 1;
  u32 gpll5 : 1;
  u32 reserved0 : 27;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_pka_perf2_ena_vote_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_pka_perf2_ena_vote_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_PKA_PERF3_ENA_VOTE
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_pka_perf3_ena_vote_s
{
  u32 gpll0 : 1;
  u32 gpll1 : 1;
  u32 gpll2_3 : 1;
  u32 gpll4 : 1;
  u32 gpll5 : 1;
  u32 reserved0 : 27;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_pka_perf3_ena_vote_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_pka_perf3_ena_vote_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_PKA_PERF4_ENA_VOTE
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_pka_perf4_ena_vote_s
{
  u32 gpll0 : 1;
  u32 gpll1 : 1;
  u32 gpll2_3 : 1;
  u32 gpll4 : 1;
  u32 gpll5 : 1;
  u32 reserved0 : 27;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_pka_perf4_ena_vote_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_pka_perf4_ena_vote_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_PKA_PERF5_ENA_VOTE
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_pka_perf5_ena_vote_s
{
  u32 gpll0 : 1;
  u32 gpll1 : 1;
  u32 gpll2_3 : 1;
  u32 gpll4 : 1;
  u32 gpll5 : 1;
  u32 reserved0 : 27;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_pka_perf5_ena_vote_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_pka_perf5_ena_vote_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_PKA_PERF6_ENA_VOTE
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_pka_perf6_ena_vote_s
{
  u32 gpll0 : 1;
  u32 gpll1 : 1;
  u32 gpll2_3 : 1;
  u32 gpll4 : 1;
  u32 gpll5 : 1;
  u32 reserved0 : 27;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_pka_perf6_ena_vote_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_pka_perf6_ena_vote_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_PKA_PERF7_ENA_VOTE
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_pka_perf7_ena_vote_s
{
  u32 gpll0 : 1;
  u32 gpll1 : 1;
  u32 gpll2_3 : 1;
  u32 gpll4 : 1;
  u32 gpll5 : 1;
  u32 reserved0 : 27;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_pka_perf7_ena_vote_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_pka_perf7_ena_vote_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_PKA_PERF8_ENA_VOTE
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_pka_perf8_ena_vote_s
{
  u32 gpll0 : 1;
  u32 gpll1 : 1;
  u32 gpll2_3 : 1;
  u32 gpll4 : 1;
  u32 gpll5 : 1;
  u32 reserved0 : 27;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_pka_perf8_ena_vote_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_pka_perf8_ena_vote_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_PKA_PERF9_ENA_VOTE
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_pka_perf9_ena_vote_s
{
  u32 gpll0 : 1;
  u32 gpll1 : 1;
  u32 gpll2_3 : 1;
  u32 gpll4 : 1;
  u32 gpll5 : 1;
  u32 reserved0 : 27;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_pka_perf9_ena_vote_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_pka_perf9_ena_vote_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_PKA_PERF10_ENA_VOTE
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_pka_perf10_ena_vote_s
{
  u32 gpll0 : 1;
  u32 gpll1 : 1;
  u32 gpll2_3 : 1;
  u32 gpll4 : 1;
  u32 gpll5 : 1;
  u32 reserved0 : 27;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_pka_perf10_ena_vote_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_pka_perf10_ena_vote_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_PKA_PERF11_ENA_VOTE
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_pka_perf11_ena_vote_s
{
  u32 gpll0 : 1;
  u32 gpll1 : 1;
  u32 gpll2_3 : 1;
  u32 gpll4 : 1;
  u32 gpll5 : 1;
  u32 reserved0 : 27;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_pka_perf11_ena_vote_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_pka_perf11_ena_vote_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_PKA_PERF12_ENA_VOTE
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_pka_perf12_ena_vote_s
{
  u32 gpll0 : 1;
  u32 gpll1 : 1;
  u32 gpll2_3 : 1;
  u32 gpll4 : 1;
  u32 gpll5 : 1;
  u32 reserved0 : 27;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_pka_perf12_ena_vote_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_pka_perf12_ena_vote_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_PKA_PERF13_ENA_VOTE
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_pka_perf13_ena_vote_s
{
  u32 gpll0 : 1;
  u32 gpll1 : 1;
  u32 gpll2_3 : 1;
  u32 gpll4 : 1;
  u32 gpll5 : 1;
  u32 reserved0 : 27;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_pka_perf13_ena_vote_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_pka_perf13_ena_vote_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_PKA_PERF14_ENA_VOTE
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_pka_perf14_ena_vote_s
{
  u32 gpll0 : 1;
  u32 gpll1 : 1;
  u32 gpll2_3 : 1;
  u32 gpll4 : 1;
  u32 gpll5 : 1;
  u32 reserved0 : 27;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_pka_perf14_ena_vote_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_pka_perf14_ena_vote_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_PKA_PERF15_ENA_VOTE
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_pka_perf15_ena_vote_s
{
  u32 gpll0 : 1;
  u32 gpll1 : 1;
  u32 gpll2_3 : 1;
  u32 gpll4 : 1;
  u32 gpll5 : 1;
  u32 reserved0 : 27;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_pka_perf15_ena_vote_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_pka_perf15_ena_vote_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_CE_PERF0_ENA_VOTE
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_ce_perf0_ena_vote_s
{
  u32 gpll0 : 1;
  u32 gpll1 : 1;
  u32 gpll2_3 : 1;
  u32 gpll4 : 1;
  u32 gpll5 : 1;
  u32 reserved0 : 27;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_ce_perf0_ena_vote_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_ce_perf0_ena_vote_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_CE_PERF1_ENA_VOTE
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_ce_perf1_ena_vote_s
{
  u32 gpll0 : 1;
  u32 gpll1 : 1;
  u32 gpll2_3 : 1;
  u32 gpll4 : 1;
  u32 gpll5 : 1;
  u32 reserved0 : 27;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_ce_perf1_ena_vote_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_ce_perf1_ena_vote_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_CE_PERF2_ENA_VOTE
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_ce_perf2_ena_vote_s
{
  u32 gpll0 : 1;
  u32 gpll1 : 1;
  u32 gpll2_3 : 1;
  u32 gpll4 : 1;
  u32 gpll5 : 1;
  u32 reserved0 : 27;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_ce_perf2_ena_vote_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_ce_perf2_ena_vote_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_CE_PERF3_ENA_VOTE
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_ce_perf3_ena_vote_s
{
  u32 gpll0 : 1;
  u32 gpll1 : 1;
  u32 gpll2_3 : 1;
  u32 gpll4 : 1;
  u32 gpll5 : 1;
  u32 reserved0 : 27;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_ce_perf3_ena_vote_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_ce_perf3_ena_vote_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_CE_PERF4_ENA_VOTE
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_ce_perf4_ena_vote_s
{
  u32 gpll0 : 1;
  u32 gpll1 : 1;
  u32 gpll2_3 : 1;
  u32 gpll4 : 1;
  u32 gpll5 : 1;
  u32 reserved0 : 27;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_ce_perf4_ena_vote_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_ce_perf4_ena_vote_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_CE_PERF5_ENA_VOTE
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_ce_perf5_ena_vote_s
{
  u32 gpll0 : 1;
  u32 gpll1 : 1;
  u32 gpll2_3 : 1;
  u32 gpll4 : 1;
  u32 gpll5 : 1;
  u32 reserved0 : 27;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_ce_perf5_ena_vote_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_ce_perf5_ena_vote_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_CE_PERF6_ENA_VOTE
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_ce_perf6_ena_vote_s
{
  u32 gpll0 : 1;
  u32 gpll1 : 1;
  u32 gpll2_3 : 1;
  u32 gpll4 : 1;
  u32 gpll5 : 1;
  u32 reserved0 : 27;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_ce_perf6_ena_vote_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_ce_perf6_ena_vote_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_CE_PERF7_ENA_VOTE
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_ce_perf7_ena_vote_s
{
  u32 gpll0 : 1;
  u32 gpll1 : 1;
  u32 gpll2_3 : 1;
  u32 gpll4 : 1;
  u32 gpll5 : 1;
  u32 reserved0 : 27;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_ce_perf7_ena_vote_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_ce_perf7_ena_vote_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_CE_PERF8_ENA_VOTE
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_ce_perf8_ena_vote_s
{
  u32 gpll0 : 1;
  u32 gpll1 : 1;
  u32 gpll2_3 : 1;
  u32 gpll4 : 1;
  u32 gpll5 : 1;
  u32 reserved0 : 27;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_ce_perf8_ena_vote_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_ce_perf8_ena_vote_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_CE_PERF9_ENA_VOTE
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_ce_perf9_ena_vote_s
{
  u32 gpll0 : 1;
  u32 gpll1 : 1;
  u32 gpll2_3 : 1;
  u32 gpll4 : 1;
  u32 gpll5 : 1;
  u32 reserved0 : 27;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_ce_perf9_ena_vote_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_ce_perf9_ena_vote_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_CE_PERF10_ENA_VOTE
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_ce_perf10_ena_vote_s
{
  u32 gpll0 : 1;
  u32 gpll1 : 1;
  u32 gpll2_3 : 1;
  u32 gpll4 : 1;
  u32 gpll5 : 1;
  u32 reserved0 : 27;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_ce_perf10_ena_vote_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_ce_perf10_ena_vote_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_CE_PERF11_ENA_VOTE
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_ce_perf11_ena_vote_s
{
  u32 gpll0 : 1;
  u32 gpll1 : 1;
  u32 gpll2_3 : 1;
  u32 gpll4 : 1;
  u32 gpll5 : 1;
  u32 reserved0 : 27;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_ce_perf11_ena_vote_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_ce_perf11_ena_vote_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_CE_PERF12_ENA_VOTE
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_ce_perf12_ena_vote_s
{
  u32 gpll0 : 1;
  u32 gpll1 : 1;
  u32 gpll2_3 : 1;
  u32 gpll4 : 1;
  u32 gpll5 : 1;
  u32 reserved0 : 27;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_ce_perf12_ena_vote_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_ce_perf12_ena_vote_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_CE_PERF13_ENA_VOTE
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_ce_perf13_ena_vote_s
{
  u32 gpll0 : 1;
  u32 gpll1 : 1;
  u32 gpll2_3 : 1;
  u32 gpll4 : 1;
  u32 gpll5 : 1;
  u32 reserved0 : 27;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_ce_perf13_ena_vote_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_ce_perf13_ena_vote_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_CE_PERF14_ENA_VOTE
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_ce_perf14_ena_vote_s
{
  u32 gpll0 : 1;
  u32 gpll1 : 1;
  u32 gpll2_3 : 1;
  u32 gpll4 : 1;
  u32 gpll5 : 1;
  u32 reserved0 : 27;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_ce_perf14_ena_vote_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_ce_perf14_ena_vote_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_CE_PERF15_ENA_VOTE
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_ce_perf15_ena_vote_s
{
  u32 gpll0 : 1;
  u32 gpll1 : 1;
  u32 gpll2_3 : 1;
  u32 gpll4 : 1;
  u32 gpll5 : 1;
  u32 reserved0 : 27;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_ce_perf15_ena_vote_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_ce_perf15_ena_vote_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_SHUB_PERF0_ENA_VOTE
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_shub_perf0_ena_vote_s
{
  u32 gpll0 : 1;
  u32 gpll1 : 1;
  u32 gpll2_3 : 1;
  u32 gpll4 : 1;
  u32 gpll5 : 1;
  u32 reserved0 : 27;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_shub_perf0_ena_vote_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_shub_perf0_ena_vote_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_SHUB_PERF1_ENA_VOTE
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_shub_perf1_ena_vote_s
{
  u32 gpll0 : 1;
  u32 gpll1 : 1;
  u32 gpll2_3 : 1;
  u32 gpll4 : 1;
  u32 gpll5 : 1;
  u32 reserved0 : 27;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_shub_perf1_ena_vote_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_shub_perf1_ena_vote_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_SHUB_PERF2_ENA_VOTE
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_shub_perf2_ena_vote_s
{
  u32 gpll0 : 1;
  u32 gpll1 : 1;
  u32 gpll2_3 : 1;
  u32 gpll4 : 1;
  u32 gpll5 : 1;
  u32 reserved0 : 27;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_shub_perf2_ena_vote_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_shub_perf2_ena_vote_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_SHUB_PERF3_ENA_VOTE
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_shub_perf3_ena_vote_s
{
  u32 gpll0 : 1;
  u32 gpll1 : 1;
  u32 gpll2_3 : 1;
  u32 gpll4 : 1;
  u32 gpll5 : 1;
  u32 reserved0 : 27;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_shub_perf3_ena_vote_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_shub_perf3_ena_vote_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_SHUB_PERF4_ENA_VOTE
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_shub_perf4_ena_vote_s
{
  u32 gpll0 : 1;
  u32 gpll1 : 1;
  u32 gpll2_3 : 1;
  u32 gpll4 : 1;
  u32 gpll5 : 1;
  u32 reserved0 : 27;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_shub_perf4_ena_vote_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_shub_perf4_ena_vote_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_SHUB_PERF5_ENA_VOTE
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_shub_perf5_ena_vote_s
{
  u32 gpll0 : 1;
  u32 gpll1 : 1;
  u32 gpll2_3 : 1;
  u32 gpll4 : 1;
  u32 gpll5 : 1;
  u32 reserved0 : 27;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_shub_perf5_ena_vote_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_shub_perf5_ena_vote_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_SHUB_PERF6_ENA_VOTE
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_shub_perf6_ena_vote_s
{
  u32 gpll0 : 1;
  u32 gpll1 : 1;
  u32 gpll2_3 : 1;
  u32 gpll4 : 1;
  u32 gpll5 : 1;
  u32 reserved0 : 27;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_shub_perf6_ena_vote_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_shub_perf6_ena_vote_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_SHUB_PERF7_ENA_VOTE
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_shub_perf7_ena_vote_s
{
  u32 gpll0 : 1;
  u32 gpll1 : 1;
  u32 gpll2_3 : 1;
  u32 gpll4 : 1;
  u32 gpll5 : 1;
  u32 reserved0 : 27;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_shub_perf7_ena_vote_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_shub_perf7_ena_vote_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_SHUB_PERF8_ENA_VOTE
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_shub_perf8_ena_vote_s
{
  u32 gpll0 : 1;
  u32 gpll1 : 1;
  u32 gpll2_3 : 1;
  u32 gpll4 : 1;
  u32 gpll5 : 1;
  u32 reserved0 : 27;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_shub_perf8_ena_vote_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_shub_perf8_ena_vote_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_SHUB_PERF9_ENA_VOTE
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_shub_perf9_ena_vote_s
{
  u32 gpll0 : 1;
  u32 gpll1 : 1;
  u32 gpll2_3 : 1;
  u32 gpll4 : 1;
  u32 gpll5 : 1;
  u32 reserved0 : 27;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_shub_perf9_ena_vote_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_shub_perf9_ena_vote_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_SHUB_PERF10_ENA_VOTE
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_shub_perf10_ena_vote_s
{
  u32 gpll0 : 1;
  u32 gpll1 : 1;
  u32 gpll2_3 : 1;
  u32 gpll4 : 1;
  u32 gpll5 : 1;
  u32 reserved0 : 27;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_shub_perf10_ena_vote_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_shub_perf10_ena_vote_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_SHUB_PERF11_ENA_VOTE
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_shub_perf11_ena_vote_s
{
  u32 gpll0 : 1;
  u32 gpll1 : 1;
  u32 gpll2_3 : 1;
  u32 gpll4 : 1;
  u32 gpll5 : 1;
  u32 reserved0 : 27;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_shub_perf11_ena_vote_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_shub_perf11_ena_vote_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_SHUB_PERF12_ENA_VOTE
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_shub_perf12_ena_vote_s
{
  u32 gpll0 : 1;
  u32 gpll1 : 1;
  u32 gpll2_3 : 1;
  u32 gpll4 : 1;
  u32 gpll5 : 1;
  u32 reserved0 : 27;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_shub_perf12_ena_vote_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_shub_perf12_ena_vote_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_SHUB_PERF13_ENA_VOTE
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_shub_perf13_ena_vote_s
{
  u32 gpll0 : 1;
  u32 gpll1 : 1;
  u32 gpll2_3 : 1;
  u32 gpll4 : 1;
  u32 gpll5 : 1;
  u32 reserved0 : 27;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_shub_perf13_ena_vote_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_shub_perf13_ena_vote_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_SHUB_PERF14_ENA_VOTE
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_shub_perf14_ena_vote_s
{
  u32 gpll0 : 1;
  u32 gpll1 : 1;
  u32 gpll2_3 : 1;
  u32 gpll4 : 1;
  u32 gpll5 : 1;
  u32 reserved0 : 27;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_shub_perf14_ena_vote_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_shub_perf14_ena_vote_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_SHUB_PERF15_ENA_VOTE
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_shub_perf15_ena_vote_s
{
  u32 gpll0 : 1;
  u32 gpll1 : 1;
  u32 gpll2_3 : 1;
  u32 gpll4 : 1;
  u32 gpll5 : 1;
  u32 reserved0 : 27;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_shub_perf15_ena_vote_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_shub_perf15_ena_vote_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_SHRM_PERF0_ENA_VOTE
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_shrm_perf0_ena_vote_s
{
  u32 gpll0 : 1;
  u32 gpll1 : 1;
  u32 gpll2_3 : 1;
  u32 gpll4 : 1;
  u32 gpll5 : 1;
  u32 reserved0 : 27;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_shrm_perf0_ena_vote_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_shrm_perf0_ena_vote_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_SHRM_PERF1_ENA_VOTE
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_shrm_perf1_ena_vote_s
{
  u32 gpll0 : 1;
  u32 gpll1 : 1;
  u32 gpll2_3 : 1;
  u32 gpll4 : 1;
  u32 gpll5 : 1;
  u32 reserved0 : 27;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_shrm_perf1_ena_vote_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_shrm_perf1_ena_vote_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_SHRM_PERF2_ENA_VOTE
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_shrm_perf2_ena_vote_s
{
  u32 gpll0 : 1;
  u32 gpll1 : 1;
  u32 gpll2_3 : 1;
  u32 gpll4 : 1;
  u32 gpll5 : 1;
  u32 reserved0 : 27;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_shrm_perf2_ena_vote_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_shrm_perf2_ena_vote_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_SHRM_PERF3_ENA_VOTE
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_shrm_perf3_ena_vote_s
{
  u32 gpll0 : 1;
  u32 gpll1 : 1;
  u32 gpll2_3 : 1;
  u32 gpll4 : 1;
  u32 gpll5 : 1;
  u32 reserved0 : 27;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_shrm_perf3_ena_vote_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_shrm_perf3_ena_vote_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_SHRM_PERF4_ENA_VOTE
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_shrm_perf4_ena_vote_s
{
  u32 gpll0 : 1;
  u32 gpll1 : 1;
  u32 gpll2_3 : 1;
  u32 gpll4 : 1;
  u32 gpll5 : 1;
  u32 reserved0 : 27;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_shrm_perf4_ena_vote_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_shrm_perf4_ena_vote_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_SHRM_PERF5_ENA_VOTE
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_shrm_perf5_ena_vote_s
{
  u32 gpll0 : 1;
  u32 gpll1 : 1;
  u32 gpll2_3 : 1;
  u32 gpll4 : 1;
  u32 gpll5 : 1;
  u32 reserved0 : 27;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_shrm_perf5_ena_vote_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_shrm_perf5_ena_vote_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_SHRM_PERF6_ENA_VOTE
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_shrm_perf6_ena_vote_s
{
  u32 gpll0 : 1;
  u32 gpll1 : 1;
  u32 gpll2_3 : 1;
  u32 gpll4 : 1;
  u32 gpll5 : 1;
  u32 reserved0 : 27;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_shrm_perf6_ena_vote_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_shrm_perf6_ena_vote_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_SHRM_PERF7_ENA_VOTE
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_shrm_perf7_ena_vote_s
{
  u32 gpll0 : 1;
  u32 gpll1 : 1;
  u32 gpll2_3 : 1;
  u32 gpll4 : 1;
  u32 gpll5 : 1;
  u32 reserved0 : 27;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_shrm_perf7_ena_vote_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_shrm_perf7_ena_vote_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_SHRM_PERF8_ENA_VOTE
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_shrm_perf8_ena_vote_s
{
  u32 gpll0 : 1;
  u32 gpll1 : 1;
  u32 gpll2_3 : 1;
  u32 gpll4 : 1;
  u32 gpll5 : 1;
  u32 reserved0 : 27;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_shrm_perf8_ena_vote_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_shrm_perf8_ena_vote_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_SHRM_PERF9_ENA_VOTE
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_shrm_perf9_ena_vote_s
{
  u32 gpll0 : 1;
  u32 gpll1 : 1;
  u32 gpll2_3 : 1;
  u32 gpll4 : 1;
  u32 gpll5 : 1;
  u32 reserved0 : 27;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_shrm_perf9_ena_vote_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_shrm_perf9_ena_vote_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_SHRM_PERF10_ENA_VOTE
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_shrm_perf10_ena_vote_s
{
  u32 gpll0 : 1;
  u32 gpll1 : 1;
  u32 gpll2_3 : 1;
  u32 gpll4 : 1;
  u32 gpll5 : 1;
  u32 reserved0 : 27;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_shrm_perf10_ena_vote_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_shrm_perf10_ena_vote_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_SHRM_PERF11_ENA_VOTE
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_shrm_perf11_ena_vote_s
{
  u32 gpll0 : 1;
  u32 gpll1 : 1;
  u32 gpll2_3 : 1;
  u32 gpll4 : 1;
  u32 gpll5 : 1;
  u32 reserved0 : 27;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_shrm_perf11_ena_vote_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_shrm_perf11_ena_vote_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_SHRM_PERF12_ENA_VOTE
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_shrm_perf12_ena_vote_s
{
  u32 gpll0 : 1;
  u32 gpll1 : 1;
  u32 gpll2_3 : 1;
  u32 gpll4 : 1;
  u32 gpll5 : 1;
  u32 reserved0 : 27;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_shrm_perf12_ena_vote_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_shrm_perf12_ena_vote_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_SHRM_PERF13_ENA_VOTE
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_shrm_perf13_ena_vote_s
{
  u32 gpll0 : 1;
  u32 gpll1 : 1;
  u32 gpll2_3 : 1;
  u32 gpll4 : 1;
  u32 gpll5 : 1;
  u32 reserved0 : 27;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_shrm_perf13_ena_vote_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_shrm_perf13_ena_vote_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_SHRM_PERF14_ENA_VOTE
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_shrm_perf14_ena_vote_s
{
  u32 gpll0 : 1;
  u32 gpll1 : 1;
  u32 gpll2_3 : 1;
  u32 gpll4 : 1;
  u32 gpll5 : 1;
  u32 reserved0 : 27;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_shrm_perf14_ena_vote_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_shrm_perf14_ena_vote_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_SHRM_PERF15_ENA_VOTE
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_shrm_perf15_ena_vote_s
{
  u32 gpll0 : 1;
  u32 gpll1 : 1;
  u32 gpll2_3 : 1;
  u32 gpll4 : 1;
  u32 gpll5 : 1;
  u32 reserved0 : 27;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_shrm_perf15_ena_vote_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_shrm_perf15_ena_vote_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_DDRMC_PERF0_ENA_VOTE
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_ddrmc_perf0_ena_vote_s
{
  u32 gpll0 : 1;
  u32 gpll1 : 1;
  u32 gpll2_3 : 1;
  u32 gpll4 : 1;
  u32 gpll5 : 1;
  u32 gcc_mode : 1;
  u32 reserved0 : 26;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_ddrmc_perf0_ena_vote_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_ddrmc_perf0_ena_vote_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_DDRMC_PERF1_ENA_VOTE
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_ddrmc_perf1_ena_vote_s
{
  u32 gpll0 : 1;
  u32 gpll1 : 1;
  u32 gpll2_3 : 1;
  u32 gpll4 : 1;
  u32 gpll5 : 1;
  u32 gcc_mode : 1;
  u32 reserved0 : 26;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_ddrmc_perf1_ena_vote_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_ddrmc_perf1_ena_vote_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_DDRMC_PERF2_ENA_VOTE
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_ddrmc_perf2_ena_vote_s
{
  u32 gpll0 : 1;
  u32 gpll1 : 1;
  u32 gpll2_3 : 1;
  u32 gpll4 : 1;
  u32 gpll5 : 1;
  u32 gcc_mode : 1;
  u32 reserved0 : 26;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_ddrmc_perf2_ena_vote_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_ddrmc_perf2_ena_vote_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_DDRMC_PERF3_ENA_VOTE
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_ddrmc_perf3_ena_vote_s
{
  u32 gpll0 : 1;
  u32 gpll1 : 1;
  u32 gpll2_3 : 1;
  u32 gpll4 : 1;
  u32 gpll5 : 1;
  u32 gcc_mode : 1;
  u32 reserved0 : 26;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_ddrmc_perf3_ena_vote_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_ddrmc_perf3_ena_vote_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_DDRMC_PERF4_ENA_VOTE
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_ddrmc_perf4_ena_vote_s
{
  u32 gpll0 : 1;
  u32 gpll1 : 1;
  u32 gpll2_3 : 1;
  u32 gpll4 : 1;
  u32 gpll5 : 1;
  u32 gcc_mode : 1;
  u32 reserved0 : 26;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_ddrmc_perf4_ena_vote_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_ddrmc_perf4_ena_vote_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_DDRMC_PERF5_ENA_VOTE
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_ddrmc_perf5_ena_vote_s
{
  u32 gpll0 : 1;
  u32 gpll1 : 1;
  u32 gpll2_3 : 1;
  u32 gpll4 : 1;
  u32 gpll5 : 1;
  u32 gcc_mode : 1;
  u32 reserved0 : 26;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_ddrmc_perf5_ena_vote_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_ddrmc_perf5_ena_vote_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_DDRMC_PERF6_ENA_VOTE
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_ddrmc_perf6_ena_vote_s
{
  u32 gpll0 : 1;
  u32 gpll1 : 1;
  u32 gpll2_3 : 1;
  u32 gpll4 : 1;
  u32 gpll5 : 1;
  u32 gcc_mode : 1;
  u32 reserved0 : 26;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_ddrmc_perf6_ena_vote_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_ddrmc_perf6_ena_vote_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_DDRMC_PERF7_ENA_VOTE
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_ddrmc_perf7_ena_vote_s
{
  u32 gpll0 : 1;
  u32 gpll1 : 1;
  u32 gpll2_3 : 1;
  u32 gpll4 : 1;
  u32 gpll5 : 1;
  u32 gcc_mode : 1;
  u32 reserved0 : 26;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_ddrmc_perf7_ena_vote_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_ddrmc_perf7_ena_vote_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_DDRMC_PERF8_ENA_VOTE
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_ddrmc_perf8_ena_vote_s
{
  u32 gpll0 : 1;
  u32 gpll1 : 1;
  u32 gpll2_3 : 1;
  u32 gpll4 : 1;
  u32 gpll5 : 1;
  u32 gcc_mode : 1;
  u32 reserved0 : 26;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_ddrmc_perf8_ena_vote_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_ddrmc_perf8_ena_vote_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_DDRMC_PERF9_ENA_VOTE
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_ddrmc_perf9_ena_vote_s
{
  u32 gpll0 : 1;
  u32 gpll1 : 1;
  u32 gpll2_3 : 1;
  u32 gpll4 : 1;
  u32 gpll5 : 1;
  u32 gcc_mode : 1;
  u32 reserved0 : 26;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_ddrmc_perf9_ena_vote_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_ddrmc_perf9_ena_vote_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_DDRMC_PERF10_ENA_VOTE
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_ddrmc_perf10_ena_vote_s
{
  u32 gpll0 : 1;
  u32 gpll1 : 1;
  u32 gpll2_3 : 1;
  u32 gpll4 : 1;
  u32 gpll5 : 1;
  u32 gcc_mode : 1;
  u32 reserved0 : 26;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_ddrmc_perf10_ena_vote_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_ddrmc_perf10_ena_vote_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_DDRMC_PERF11_ENA_VOTE
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_ddrmc_perf11_ena_vote_s
{
  u32 gpll0 : 1;
  u32 gpll1 : 1;
  u32 gpll2_3 : 1;
  u32 gpll4 : 1;
  u32 gpll5 : 1;
  u32 gcc_mode : 1;
  u32 reserved0 : 26;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_ddrmc_perf11_ena_vote_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_ddrmc_perf11_ena_vote_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_DDRMC_PERF12_ENA_VOTE
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_ddrmc_perf12_ena_vote_s
{
  u32 gpll0 : 1;
  u32 gpll1 : 1;
  u32 gpll2_3 : 1;
  u32 gpll4 : 1;
  u32 gpll5 : 1;
  u32 gcc_mode : 1;
  u32 reserved0 : 26;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_ddrmc_perf12_ena_vote_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_ddrmc_perf12_ena_vote_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_DDRMC_PERF13_ENA_VOTE
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_ddrmc_perf13_ena_vote_s
{
  u32 gpll0 : 1;
  u32 gpll1 : 1;
  u32 gpll2_3 : 1;
  u32 gpll4 : 1;
  u32 gpll5 : 1;
  u32 gcc_mode : 1;
  u32 reserved0 : 26;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_ddrmc_perf13_ena_vote_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_ddrmc_perf13_ena_vote_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_DDRMC_PERF14_ENA_VOTE
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_ddrmc_perf14_ena_vote_s
{
  u32 gpll0 : 1;
  u32 gpll1 : 1;
  u32 gpll2_3 : 1;
  u32 gpll4 : 1;
  u32 gpll5 : 1;
  u32 gcc_mode : 1;
  u32 reserved0 : 26;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_ddrmc_perf14_ena_vote_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_ddrmc_perf14_ena_vote_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_DDRMC_PERF15_ENA_VOTE
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_ddrmc_perf15_ena_vote_s
{
  u32 gpll0 : 1;
  u32 gpll1 : 1;
  u32 gpll2_3 : 1;
  u32 gpll4 : 1;
  u32 gpll5 : 1;
  u32 gcc_mode : 1;
  u32 reserved0 : 26;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_ddrmc_perf15_ena_vote_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_ddrmc_perf15_ena_vote_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_DDRMC_PERF0_GPLL2_3_L_VAL
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_ddrmc_perf0_gpll2_3_l_val_s
{
  u32 pll_l : 16;
  u32 reserved0 : 16;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_ddrmc_perf0_gpll2_3_l_val_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_ddrmc_perf0_gpll2_3_l_val_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_DDRMC_PERF1_GPLL2_3_L_VAL
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_ddrmc_perf1_gpll2_3_l_val_s
{
  u32 pll_l : 16;
  u32 reserved0 : 16;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_ddrmc_perf1_gpll2_3_l_val_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_ddrmc_perf1_gpll2_3_l_val_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_DDRMC_PERF2_GPLL2_3_L_VAL
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_ddrmc_perf2_gpll2_3_l_val_s
{
  u32 pll_l : 16;
  u32 reserved0 : 16;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_ddrmc_perf2_gpll2_3_l_val_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_ddrmc_perf2_gpll2_3_l_val_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_DDRMC_PERF3_GPLL2_3_L_VAL
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_ddrmc_perf3_gpll2_3_l_val_s
{
  u32 pll_l : 16;
  u32 reserved0 : 16;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_ddrmc_perf3_gpll2_3_l_val_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_ddrmc_perf3_gpll2_3_l_val_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_DDRMC_PERF4_GPLL2_3_L_VAL
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_ddrmc_perf4_gpll2_3_l_val_s
{
  u32 pll_l : 16;
  u32 reserved0 : 16;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_ddrmc_perf4_gpll2_3_l_val_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_ddrmc_perf4_gpll2_3_l_val_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_DDRMC_PERF5_GPLL2_3_L_VAL
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_ddrmc_perf5_gpll2_3_l_val_s
{
  u32 pll_l : 16;
  u32 reserved0 : 16;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_ddrmc_perf5_gpll2_3_l_val_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_ddrmc_perf5_gpll2_3_l_val_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_DDRMC_PERF6_GPLL2_3_L_VAL
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_ddrmc_perf6_gpll2_3_l_val_s
{
  u32 pll_l : 16;
  u32 reserved0 : 16;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_ddrmc_perf6_gpll2_3_l_val_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_ddrmc_perf6_gpll2_3_l_val_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_DDRMC_PERF7_GPLL2_3_L_VAL
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_ddrmc_perf7_gpll2_3_l_val_s
{
  u32 pll_l : 16;
  u32 reserved0 : 16;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_ddrmc_perf7_gpll2_3_l_val_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_ddrmc_perf7_gpll2_3_l_val_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_DDRMC_PERF8_GPLL2_3_L_VAL
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_ddrmc_perf8_gpll2_3_l_val_s
{
  u32 pll_l : 16;
  u32 reserved0 : 16;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_ddrmc_perf8_gpll2_3_l_val_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_ddrmc_perf8_gpll2_3_l_val_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_DDRMC_PERF9_GPLL2_3_L_VAL
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_ddrmc_perf9_gpll2_3_l_val_s
{
  u32 pll_l : 16;
  u32 reserved0 : 16;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_ddrmc_perf9_gpll2_3_l_val_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_ddrmc_perf9_gpll2_3_l_val_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_DDRMC_PERF10_GPLL2_3_L_VAL
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_ddrmc_perf10_gpll2_3_l_val_s
{
  u32 pll_l : 16;
  u32 reserved0 : 16;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_ddrmc_perf10_gpll2_3_l_val_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_ddrmc_perf10_gpll2_3_l_val_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_DDRMC_PERF11_GPLL2_3_L_VAL
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_ddrmc_perf11_gpll2_3_l_val_s
{
  u32 pll_l : 16;
  u32 reserved0 : 16;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_ddrmc_perf11_gpll2_3_l_val_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_ddrmc_perf11_gpll2_3_l_val_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_DDRMC_PERF12_GPLL2_3_L_VAL
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_ddrmc_perf12_gpll2_3_l_val_s
{
  u32 pll_l : 16;
  u32 reserved0 : 16;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_ddrmc_perf12_gpll2_3_l_val_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_ddrmc_perf12_gpll2_3_l_val_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_DDRMC_PERF13_GPLL2_3_L_VAL
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_ddrmc_perf13_gpll2_3_l_val_s
{
  u32 pll_l : 16;
  u32 reserved0 : 16;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_ddrmc_perf13_gpll2_3_l_val_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_ddrmc_perf13_gpll2_3_l_val_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_DDRMC_PERF14_GPLL2_3_L_VAL
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_ddrmc_perf14_gpll2_3_l_val_s
{
  u32 pll_l : 16;
  u32 reserved0 : 16;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_ddrmc_perf14_gpll2_3_l_val_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_ddrmc_perf14_gpll2_3_l_val_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_DDRMC_PERF15_GPLL2_3_L_VAL
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_ddrmc_perf15_gpll2_3_l_val_s
{
  u32 pll_l : 16;
  u32 reserved0 : 16;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_ddrmc_perf15_gpll2_3_l_val_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_ddrmc_perf15_gpll2_3_l_val_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_DDRMC_PERF0_GPLL2_3_FRAC_VAL
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_ddrmc_perf0_gpll2_3_frac_val_s
{
  u32 pll_frac_val : 16;
  u32 reserved0 : 16;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_ddrmc_perf0_gpll2_3_frac_val_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_ddrmc_perf0_gpll2_3_frac_val_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_DDRMC_PERF1_GPLL2_3_FRAC_VAL
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_ddrmc_perf1_gpll2_3_frac_val_s
{
  u32 pll_frac_val : 16;
  u32 reserved0 : 16;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_ddrmc_perf1_gpll2_3_frac_val_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_ddrmc_perf1_gpll2_3_frac_val_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_DDRMC_PERF2_GPLL2_3_FRAC_VAL
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_ddrmc_perf2_gpll2_3_frac_val_s
{
  u32 pll_frac_val : 16;
  u32 reserved0 : 16;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_ddrmc_perf2_gpll2_3_frac_val_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_ddrmc_perf2_gpll2_3_frac_val_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_DDRMC_PERF3_GPLL2_3_FRAC_VAL
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_ddrmc_perf3_gpll2_3_frac_val_s
{
  u32 pll_frac_val : 16;
  u32 reserved0 : 16;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_ddrmc_perf3_gpll2_3_frac_val_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_ddrmc_perf3_gpll2_3_frac_val_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_DDRMC_PERF4_GPLL2_3_FRAC_VAL
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_ddrmc_perf4_gpll2_3_frac_val_s
{
  u32 pll_frac_val : 16;
  u32 reserved0 : 16;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_ddrmc_perf4_gpll2_3_frac_val_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_ddrmc_perf4_gpll2_3_frac_val_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_DDRMC_PERF5_GPLL2_3_FRAC_VAL
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_ddrmc_perf5_gpll2_3_frac_val_s
{
  u32 pll_frac_val : 16;
  u32 reserved0 : 16;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_ddrmc_perf5_gpll2_3_frac_val_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_ddrmc_perf5_gpll2_3_frac_val_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_DDRMC_PERF6_GPLL2_3_FRAC_VAL
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_ddrmc_perf6_gpll2_3_frac_val_s
{
  u32 pll_frac_val : 16;
  u32 reserved0 : 16;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_ddrmc_perf6_gpll2_3_frac_val_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_ddrmc_perf6_gpll2_3_frac_val_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_DDRMC_PERF7_GPLL2_3_FRAC_VAL
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_ddrmc_perf7_gpll2_3_frac_val_s
{
  u32 pll_frac_val : 16;
  u32 reserved0 : 16;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_ddrmc_perf7_gpll2_3_frac_val_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_ddrmc_perf7_gpll2_3_frac_val_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_DDRMC_PERF8_GPLL2_3_FRAC_VAL
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_ddrmc_perf8_gpll2_3_frac_val_s
{
  u32 pll_frac_val : 16;
  u32 reserved0 : 16;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_ddrmc_perf8_gpll2_3_frac_val_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_ddrmc_perf8_gpll2_3_frac_val_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_DDRMC_PERF9_GPLL2_3_FRAC_VAL
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_ddrmc_perf9_gpll2_3_frac_val_s
{
  u32 pll_frac_val : 16;
  u32 reserved0 : 16;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_ddrmc_perf9_gpll2_3_frac_val_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_ddrmc_perf9_gpll2_3_frac_val_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_DDRMC_PERF10_GPLL2_3_FRAC_VAL
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_ddrmc_perf10_gpll2_3_frac_val_s
{
  u32 pll_frac_val : 16;
  u32 reserved0 : 16;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_ddrmc_perf10_gpll2_3_frac_val_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_ddrmc_perf10_gpll2_3_frac_val_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_DDRMC_PERF11_GPLL2_3_FRAC_VAL
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_ddrmc_perf11_gpll2_3_frac_val_s
{
  u32 pll_frac_val : 16;
  u32 reserved0 : 16;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_ddrmc_perf11_gpll2_3_frac_val_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_ddrmc_perf11_gpll2_3_frac_val_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_DDRMC_PERF12_GPLL2_3_FRAC_VAL
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_ddrmc_perf12_gpll2_3_frac_val_s
{
  u32 pll_frac_val : 16;
  u32 reserved0 : 16;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_ddrmc_perf12_gpll2_3_frac_val_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_ddrmc_perf12_gpll2_3_frac_val_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_DDRMC_PERF13_GPLL2_3_FRAC_VAL
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_ddrmc_perf13_gpll2_3_frac_val_s
{
  u32 pll_frac_val : 16;
  u32 reserved0 : 16;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_ddrmc_perf13_gpll2_3_frac_val_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_ddrmc_perf13_gpll2_3_frac_val_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_DDRMC_PERF14_GPLL2_3_FRAC_VAL
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_ddrmc_perf14_gpll2_3_frac_val_s
{
  u32 pll_frac_val : 16;
  u32 reserved0 : 16;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_ddrmc_perf14_gpll2_3_frac_val_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_ddrmc_perf14_gpll2_3_frac_val_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_DDRMC_PERF15_GPLL2_3_FRAC_VAL
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_ddrmc_perf15_gpll2_3_frac_val_s
{
  u32 pll_frac_val : 16;
  u32 reserved0 : 16;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_ddrmc_perf15_gpll2_3_frac_val_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_ddrmc_perf15_gpll2_3_frac_val_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_DDRMC_FAKE_SWITCH_DEBUG
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_ddrmc_fake_switch_debug_s
{
  u32 pll_toggle_en : 1;
  u32 rcg_toggle_en : 1;
  u32 reserved0 : 30;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_ddrmc_fake_switch_debug_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_ddrmc_fake_switch_debug_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: GCC_RPMH_DDRMC_SWITCH_STATUS
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_gcc_hwio_def_gcc_rpmh_ddrmc_switch_status_s
{
  u32 rpmh_ddrmc_clock_switch_fsm_state : 5;
  u32 rpmh_ddrmc_clock_switch_fsm_pll_toggle_fsm_state : 1;
  u32 rpmh_ddrmc_clock_switch_fsm_rcg_toggle_fsm_state : 1;
  u32 reserved0 : 25;
};

/* Union definition of register */
union ipa_gcc_hwio_def_gcc_rpmh_ddrmc_switch_status_u
{
  struct ipa_gcc_hwio_def_gcc_rpmh_ddrmc_switch_status_s def;
  u32 value;
};


#endif /* __IPA_GCC_HWIO_DEF_H__ */
