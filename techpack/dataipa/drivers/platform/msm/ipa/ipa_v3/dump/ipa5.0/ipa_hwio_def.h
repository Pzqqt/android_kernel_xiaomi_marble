/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (c) 2021, The Linux Foundation. All rights reserved.
 */

#ifndef __IPA_HWIO_DEF_H__
#define __IPA_HWIO_DEF_H__
/**
  @file ipa_hwio.h
  @brief Auto-generated HWIO interface include file.

  This file contains HWIO register definitions for the following modules:
    IPA.*

  'Include' filters applied: <none>
  'Exclude' filters applied: RESERVED DUMMY
*/

/*----------------------------------------------------------------------------
 * MODULE: IPA_UC_IPA_UC
 *--------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
 * MODULE: IPA_UC_IPA_UC_RAM
 *--------------------------------------------------------------------------*/

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_UC_IRAM_START
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_uc_iram_start_s
{
  u32 data : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_uc_iram_start_u
{
  struct ipa_hwio_def_ipa_uc_iram_start_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_UC_DRAM_START
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_uc_dram_start_s
{
  u32 data : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_uc_dram_start_u
{
  struct ipa_hwio_def_ipa_uc_dram_start_s def;
  u32 value;
};

/*----------------------------------------------------------------------------
 * MODULE: IPA_UC_IPA_UC_PER
 *--------------------------------------------------------------------------*/

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_UC_STATUS
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_uc_status_s
{
  u32 sleepdeep : 1;
  u32 sleep : 1;
  u32 lockup : 1;
  u32 uc_enable : 1;
  u32 reserved0 : 28;
};

/* Union definition of register */
union ipa_hwio_def_ipa_uc_status_u
{
  struct ipa_hwio_def_ipa_uc_status_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_UC_CONTROL
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_uc_control_s
{
  u32 reserved0 : 1;
  u32 uc_dsmode : 1;
  u32 qmb_snoc_bypass_dis : 1;
  u32 uc_clock_gating_dis : 1;
  u32 mbox_dis : 8;
  u32 reserved1 : 12;
  u32 warmboot_dis : 1;
  u32 reserved2 : 2;
  u32 uc_ram_rd_cli_cache_dis : 1;
  u32 reserved3 : 4;
};

/* Union definition of register */
union ipa_hwio_def_ipa_uc_control_u
{
  struct ipa_hwio_def_ipa_uc_control_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_UC_SYS_BUS_ATTRIB
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_uc_sys_bus_attrib_s
{
  u32 memtype : 3;
  u32 reserved0 : 1;
  u32 noallocate : 1;
  u32 reserved1 : 3;
  u32 innershared : 1;
  u32 reserved2 : 3;
  u32 shared : 1;
  u32 reserved3 : 19;
};

/* Union definition of register */
union ipa_hwio_def_ipa_uc_sys_bus_attrib_u
{
  struct ipa_hwio_def_ipa_uc_sys_bus_attrib_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_UC_PEND_IRQ
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_uc_pend_irq_s
{
  u32 pend_irq : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_uc_pend_irq_u
{
  struct ipa_hwio_def_ipa_uc_pend_irq_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_UC_TRACE_BUFFER
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_uc_trace_buffer_s
{
  u32 trace_buffer : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_uc_trace_buffer_u
{
  struct ipa_hwio_def_ipa_uc_trace_buffer_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_UC_PC
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_uc_pc_s
{
  u32 pc : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_uc_pc_u
{
  struct ipa_hwio_def_ipa_uc_pc_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_UC_VUIC_INT_ADDRESS_LSB
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_uc_vuic_int_address_lsb_s
{
  u32 addrress : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_uc_vuic_int_address_lsb_u
{
  struct ipa_hwio_def_ipa_uc_vuic_int_address_lsb_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_UC_VUIC_INT_ADDRESS_MSB
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_uc_vuic_int_address_msb_s
{
  u32 addrress : 9;
  u32 reserved0 : 23;
};

/* Union definition of register */
union ipa_hwio_def_ipa_uc_vuic_int_address_msb_u
{
  struct ipa_hwio_def_ipa_uc_vuic_int_address_msb_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_UC_QMB_SYS_ADDR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_uc_qmb_sys_addr_s
{
  u32 addr : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_uc_qmb_sys_addr_u
{
  struct ipa_hwio_def_ipa_uc_qmb_sys_addr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_UC_QMB_SYS_ADDR_MSB
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_uc_qmb_sys_addr_msb_s
{
  u32 addr_msb : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_uc_qmb_sys_addr_msb_u
{
  struct ipa_hwio_def_ipa_uc_qmb_sys_addr_msb_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_UC_QMB_LOCAL_ADDR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_uc_qmb_local_addr_s
{
  u32 addr : 18;
  u32 reserved0 : 14;
};

/* Union definition of register */
union ipa_hwio_def_ipa_uc_qmb_local_addr_u
{
  struct ipa_hwio_def_ipa_uc_qmb_local_addr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_UC_QMB_LENGTH
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_uc_qmb_length_s
{
  u32 length : 16;
  u32 reserved0 : 16;
};

/* Union definition of register */
union ipa_hwio_def_ipa_uc_qmb_length_u
{
  struct ipa_hwio_def_ipa_uc_qmb_length_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_UC_QMB_TRIGGER
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_uc_qmb_trigger_s
{
  u32 rsv : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_uc_qmb_trigger_u
{
  struct ipa_hwio_def_ipa_uc_qmb_trigger_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_UC_QMB_COMMAND_ATTR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_uc_qmb_command_attr_s
{
  u32 direction : 1;
  u32 inorder : 1;
  u32 wait_for_response_mode : 1;
  u32 sync : 1;
  u32 interrupt_on_completion : 1;
  u32 queue_number : 1;
  u32 reserved0 : 10;
  u32 user : 11;
  u32 reserved1 : 5;
};

/* Union definition of register */
union ipa_hwio_def_ipa_uc_qmb_command_attr_u
{
  struct ipa_hwio_def_ipa_uc_qmb_command_attr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_UC_QMB_COMMAND_UCTAG
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_uc_qmb_command_uctag_s
{
  u32 uctag : 18;
  u32 reserved0 : 14;
};

/* Union definition of register */
union ipa_hwio_def_ipa_uc_qmb_command_uctag_u
{
  struct ipa_hwio_def_ipa_uc_qmb_command_uctag_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_UC_QMB_COMPLETED_FIFO_n
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_uc_qmb_completed_fifo_n_s
{
  u32 uctag : 18;
  u32 fifo_size : 4;
  u32 fifo_cnt : 4;
  u32 error : 1;
  u32 reserved0 : 3;
  u32 empty : 1;
  u32 full : 1;
};

/* Union definition of register */
union ipa_hwio_def_ipa_uc_qmb_completed_fifo_n_u
{
  struct ipa_hwio_def_ipa_uc_qmb_completed_fifo_n_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_UC_QMB_COMPLETED_FIFO_PEEK_n
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_uc_qmb_completed_fifo_peek_n_s
{
  u32 uctag : 18;
  u32 fifo_size : 4;
  u32 fifo_cnt : 4;
  u32 error : 1;
  u32 reserved0 : 3;
  u32 empty : 1;
  u32 full : 1;
};

/* Union definition of register */
union ipa_hwio_def_ipa_uc_qmb_completed_fifo_peek_n_u
{
  struct ipa_hwio_def_ipa_uc_qmb_completed_fifo_peek_n_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_UC_QMB_CMD_FIFO_STATUS_n
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_uc_qmb_cmd_fifo_status_n_s
{
  u32 fifo_size : 4;
  u32 fifo_cnt : 4;
  u32 reserved0 : 8;
  u32 empty : 1;
  u32 full : 1;
  u32 reserved1 : 14;
};

/* Union definition of register */
union ipa_hwio_def_ipa_uc_qmb_cmd_fifo_status_n_u
{
  struct ipa_hwio_def_ipa_uc_qmb_cmd_fifo_status_n_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_UC_QMB_SYNC_STATUS
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_uc_qmb_sync_status_s
{
  u32 error_queue_0 : 1;
  u32 reserved0 : 15;
  u32 error_queue_1 : 1;
  u32 reserved1 : 15;
};

/* Union definition of register */
union ipa_hwio_def_ipa_uc_qmb_sync_status_u
{
  struct ipa_hwio_def_ipa_uc_qmb_sync_status_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_UC_QMB_BUS_ATTRIB
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_uc_qmb_bus_attrib_s
{
  u32 memtype : 3;
  u32 reserved0 : 1;
  u32 noallocate : 1;
  u32 reserved1 : 3;
  u32 innershared : 1;
  u32 reserved2 : 3;
  u32 shared : 1;
  u32 reserved3 : 19;
};

/* Union definition of register */
union ipa_hwio_def_ipa_uc_qmb_bus_attrib_u
{
  struct ipa_hwio_def_ipa_uc_qmb_bus_attrib_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_UC_QMB_OUTSTANDING_CFG
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_uc_qmb_outstanding_cfg_s
{
  u32 max_ot_overall : 8;
  u32 max_ot_rd : 8;
  u32 max_ot_wr : 8;
  u32 reserved0 : 8;
};

/* Union definition of register */
union ipa_hwio_def_ipa_uc_qmb_outstanding_cfg_u
{
  struct ipa_hwio_def_ipa_uc_qmb_outstanding_cfg_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_UC_QMB_OUTSTANDING_STATUS
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_uc_qmb_outstanding_status_s
{
  u32 current_ot_overall : 8;
  u32 current_ot_rd : 8;
  u32 current_ot_wr : 8;
  u32 reserved0 : 8;
};

/* Union definition of register */
union ipa_hwio_def_ipa_uc_qmb_outstanding_status_u
{
  struct ipa_hwio_def_ipa_uc_qmb_outstanding_status_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_UC_QMB_COMP_FIFO_INT_EN
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_uc_qmb_comp_fifo_int_en_s
{
  u32 comp_fifo_0_not_empty : 1;
  u32 comp_fifo_0_full : 1;
  u32 comp_fifo_0_ioc_cmd : 1;
  u32 reserved0 : 13;
  u32 comp_fifo_1_not_empty : 1;
  u32 comp_fifo_1_full : 1;
  u32 comp_fifo_1_ioc_cmd : 1;
  u32 reserved1 : 13;
};

/* Union definition of register */
union ipa_hwio_def_ipa_uc_qmb_comp_fifo_int_en_u
{
  struct ipa_hwio_def_ipa_uc_qmb_comp_fifo_int_en_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_UC_QMB_COMP_FIFO_INT_CLR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_uc_qmb_comp_fifo_int_clr_s
{
  u32 comp_fifo_0_not_empty : 1;
  u32 comp_fifo_0_full : 1;
  u32 comp_fifo_0_ioc_cmd : 1;
  u32 reserved0 : 13;
  u32 comp_fifo_1_not_empty : 1;
  u32 comp_fifo_1_full : 1;
  u32 comp_fifo_1_ioc_cmd : 1;
  u32 reserved1 : 13;
};

/* Union definition of register */
union ipa_hwio_def_ipa_uc_qmb_comp_fifo_int_clr_u
{
  struct ipa_hwio_def_ipa_uc_qmb_comp_fifo_int_clr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_UC_QMB_COMP_FIFO_INT_STTS
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_uc_qmb_comp_fifo_int_stts_s
{
  u32 comp_fifo_0_not_empty : 1;
  u32 comp_fifo_0_full : 1;
  u32 comp_fifo_0_ioc_cmd : 1;
  u32 reserved0 : 13;
  u32 comp_fifo_1_not_empty : 1;
  u32 comp_fifo_1_full : 1;
  u32 comp_fifo_1_ioc_cmd : 1;
  u32 reserved1 : 13;
};

/* Union definition of register */
union ipa_hwio_def_ipa_uc_qmb_comp_fifo_int_stts_u
{
  struct ipa_hwio_def_ipa_uc_qmb_comp_fifo_int_stts_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_UC_QMB_SYNC_COMPLETE_INT_EN
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_uc_qmb_sync_complete_int_en_s
{
  u32 sync_completed_0 : 1;
  u32 sync_completed_1 : 1;
  u32 reserved0 : 30;
};

/* Union definition of register */
union ipa_hwio_def_ipa_uc_qmb_sync_complete_int_en_u
{
  struct ipa_hwio_def_ipa_uc_qmb_sync_complete_int_en_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_UC_QMB_SYNC_COMPLETE_INT_CLR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_uc_qmb_sync_complete_int_clr_s
{
  u32 sync_completed_0 : 1;
  u32 sync_completed_1 : 1;
  u32 reserved0 : 30;
};

/* Union definition of register */
union ipa_hwio_def_ipa_uc_qmb_sync_complete_int_clr_u
{
  struct ipa_hwio_def_ipa_uc_qmb_sync_complete_int_clr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_UC_QMB_SYNC_COMPLETE_INT_STTS
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_uc_qmb_sync_complete_int_stts_s
{
  u32 sync_completed_0 : 1;
  u32 sync_completed_1 : 1;
  u32 reserved0 : 30;
};

/* Union definition of register */
union ipa_hwio_def_ipa_uc_qmb_sync_complete_int_stts_u
{
  struct ipa_hwio_def_ipa_uc_qmb_sync_complete_int_stts_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_UC_MBOX_INT_STTS_n
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_uc_mbox_int_stts_n_s
{
  u32 irq_status : 16;
  u32 reserved0 : 16;
};

/* Union definition of register */
union ipa_hwio_def_ipa_uc_mbox_int_stts_n_u
{
  struct ipa_hwio_def_ipa_uc_mbox_int_stts_n_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_UC_MBOX_INT_EN_n
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_uc_mbox_int_en_n_s
{
  u32 irq_en : 16;
  u32 reserved0 : 16;
};

/* Union definition of register */
union ipa_hwio_def_ipa_uc_mbox_int_en_n_u
{
  struct ipa_hwio_def_ipa_uc_mbox_int_en_n_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_UC_MBOX_INT_CLR_n
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_uc_mbox_int_clr_n_s
{
  u32 irq_clr : 16;
  u32 reserved0 : 16;
};

/* Union definition of register */
union ipa_hwio_def_ipa_uc_mbox_int_clr_n_u
{
  struct ipa_hwio_def_ipa_uc_mbox_int_clr_n_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_UC_IPA_INT_STTS_n
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_uc_ipa_int_stts_n_s
{
  u32 irq_status : 4;
  u32 reserved0 : 28;
};

/* Union definition of register */
union ipa_hwio_def_ipa_uc_ipa_int_stts_n_u
{
  struct ipa_hwio_def_ipa_uc_ipa_int_stts_n_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_UC_IPA_INT_EN_n
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_uc_ipa_int_en_n_s
{
  u32 irq_en : 4;
  u32 reserved0 : 28;
};

/* Union definition of register */
union ipa_hwio_def_ipa_uc_ipa_int_en_n_u
{
  struct ipa_hwio_def_ipa_uc_ipa_int_en_n_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_UC_IPA_INT_CLR_n
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_uc_ipa_int_clr_n_s
{
  u32 irq_clr : 4;
  u32 reserved0 : 28;
};

/* Union definition of register */
union ipa_hwio_def_ipa_uc_ipa_int_clr_n_u
{
  struct ipa_hwio_def_ipa_uc_ipa_int_clr_n_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_UC_HWEV_INT_STTS
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_uc_hwev_int_stts_s
{
  u32 irq_status : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_uc_hwev_int_stts_u
{
  struct ipa_hwio_def_ipa_uc_hwev_int_stts_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_UC_HWEV_INT_EN
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_uc_hwev_int_en_s
{
  u32 irq_en : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_uc_hwev_int_en_u
{
  struct ipa_hwio_def_ipa_uc_hwev_int_en_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_UC_HWEV_INT_CLR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_uc_hwev_int_clr_s
{
  u32 irq_clr : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_uc_hwev_int_clr_u
{
  struct ipa_hwio_def_ipa_uc_hwev_int_clr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_UC_SWEV_INT_STTS
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_uc_swev_int_stts_s
{
  u32 irq_status : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_uc_swev_int_stts_u
{
  struct ipa_hwio_def_ipa_uc_swev_int_stts_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_UC_SWEV_INT_EN
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_uc_swev_int_en_s
{
  u32 irq_en : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_uc_swev_int_en_u
{
  struct ipa_hwio_def_ipa_uc_swev_int_en_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_UC_SWEV_INT_CLR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_uc_swev_int_clr_s
{
  u32 irq_clr : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_uc_swev_int_clr_u
{
  struct ipa_hwio_def_ipa_uc_swev_int_clr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_UC_VUIC_INT_STTS
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_uc_vuic_int_stts_s
{
  u32 irq_status : 1;
  u32 reserved0 : 31;
};

/* Union definition of register */
union ipa_hwio_def_ipa_uc_vuic_int_stts_u
{
  struct ipa_hwio_def_ipa_uc_vuic_int_stts_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_UC_VUIC_INT_CLR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_uc_vuic_int_clr_s
{
  u32 irq_clr : 1;
  u32 reserved0 : 31;
};

/* Union definition of register */
union ipa_hwio_def_ipa_uc_vuic_int_clr_u
{
  struct ipa_hwio_def_ipa_uc_vuic_int_clr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_UC_TIMER_CTRL_n
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_uc_timer_ctrl_n_s
{
  u32 count : 16;
  u32 event_sel : 7;
  u32 reserved0 : 1;
  u32 retrig : 1;
  u32 reserved1 : 5;
  u32 gran_sel : 2;
};

/* Union definition of register */
union ipa_hwio_def_ipa_uc_timer_ctrl_n_u
{
  struct ipa_hwio_def_ipa_uc_timer_ctrl_n_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_UC_TIMER_STATUS_n
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_uc_timer_status_n_s
{
  u32 count : 16;
  u32 reserved0 : 8;
  u32 active : 1;
  u32 reserved1 : 7;
};

/* Union definition of register */
union ipa_hwio_def_ipa_uc_timer_status_n_u
{
  struct ipa_hwio_def_ipa_uc_timer_status_n_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_UC_EVENTS
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_uc_events_s
{
  u32 events : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_uc_events_u
{
  struct ipa_hwio_def_ipa_uc_events_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_UC_VUIC_BUS_ADDR_TRANSLATE_EN
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_uc_vuic_bus_addr_translate_en_s
{
  u32 qmb_addr_translate : 1;
  u32 direct_addr_translate : 1;
  u32 reserved0 : 30;
};

/* Union definition of register */
union ipa_hwio_def_ipa_uc_vuic_bus_addr_translate_en_u
{
  struct ipa_hwio_def_ipa_uc_vuic_bus_addr_translate_en_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_UC_SYS_ADDR_MSB
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_uc_sys_addr_msb_s
{
  u32 sys_addr_msb : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_uc_sys_addr_msb_u
{
  struct ipa_hwio_def_ipa_uc_sys_addr_msb_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_UC_PC_RESTORE_WR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_uc_pc_restore_wr_s
{
  u32 set_ipa_pc_ack : 1;
  u32 clear_ipa_pc_ack : 1;
  u32 set_ipa_restore_ack : 1;
  u32 clear_ipa_restore_ack : 1;
  u32 reserved0 : 28;
};

/* Union definition of register */
union ipa_hwio_def_ipa_uc_pc_restore_wr_u
{
  struct ipa_hwio_def_ipa_uc_pc_restore_wr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_UC_PC_RESTORE_RD
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_uc_pc_restore_rd_s
{
  u32 ipa_pc_req : 1;
  u32 ipa_pc_ack : 1;
  u32 ipa_restore_req : 1;
  u32 ipa_restore_ack : 1;
  u32 reserved0 : 28;
};

/* Union definition of register */
union ipa_hwio_def_ipa_uc_pc_restore_rd_u
{
  struct ipa_hwio_def_ipa_uc_pc_restore_rd_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_UC_CNT_GLOBAL
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_uc_cnt_global_s
{
  u32 count_en : 1;
  u32 count_cgc_open : 1;
  u32 reserved0 : 29;
  u32 clear_all : 1;
};

/* Union definition of register */
union ipa_hwio_def_ipa_uc_cnt_global_u
{
  struct ipa_hwio_def_ipa_uc_cnt_global_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_UC_CNT_CTL
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_uc_cnt_ctl_s
{
  u32 cycle_cnt_en : 1;
  u32 reserved0 : 1;
  u32 cycle_cnt_clr : 1;
  u32 reserved1 : 1;
  u32 idle_cnt_en : 1;
  u32 reserved2 : 1;
  u32 idle_cnt_clr : 1;
  u32 reserved3 : 1;
  u32 inst_cnt_en : 1;
  u32 inst_clr_after_rd : 1;
  u32 inst_cnt_clr : 1;
  u32 reserved4 : 1;
  u32 vuic_rd_cnt_en : 1;
  u32 vuic_wr_cnt_en : 1;
  u32 vuic_clr_after_rd : 1;
  u32 vuic_cnt_clr : 1;
  u32 dram_rd_cnt_en : 1;
  u32 dram_wr_cnt_en : 1;
  u32 dram_clr_after_rd : 1;
  u32 dram_cnt_clr : 1;
  u32 reserved5 : 12;
};

/* Union definition of register */
union ipa_hwio_def_ipa_uc_cnt_ctl_u
{
  struct ipa_hwio_def_ipa_uc_cnt_ctl_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_UC_CNT_CLK_CYCLE
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_uc_cnt_clk_cycle_s
{
  u32 counter : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_uc_cnt_clk_cycle_u
{
  struct ipa_hwio_def_ipa_uc_cnt_clk_cycle_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_UC_CNT_CLK_CYCLE_MSB
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_uc_cnt_clk_cycle_msb_s
{
  u32 counter : 8;
  u32 reserved0 : 24;
};

/* Union definition of register */
union ipa_hwio_def_ipa_uc_cnt_clk_cycle_msb_u
{
  struct ipa_hwio_def_ipa_uc_cnt_clk_cycle_msb_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_UC_CNT_IDLE
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_uc_cnt_idle_s
{
  u32 counter : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_uc_cnt_idle_u
{
  struct ipa_hwio_def_ipa_uc_cnt_idle_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_UC_CNT_IDLE_MSB
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_uc_cnt_idle_msb_s
{
  u32 counter : 8;
  u32 reserved0 : 24;
};

/* Union definition of register */
union ipa_hwio_def_ipa_uc_cnt_idle_msb_u
{
  struct ipa_hwio_def_ipa_uc_cnt_idle_msb_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_UC_CNT_INST
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_uc_cnt_inst_s
{
  u32 counter : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_uc_cnt_inst_u
{
  struct ipa_hwio_def_ipa_uc_cnt_inst_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_UC_CNT_DRAM
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_uc_cnt_dram_s
{
  u32 counter : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_uc_cnt_dram_u
{
  struct ipa_hwio_def_ipa_uc_cnt_dram_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_UC_CNT_VUIC
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_uc_cnt_vuic_s
{
  u32 counter : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_uc_cnt_vuic_u
{
  struct ipa_hwio_def_ipa_uc_cnt_vuic_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_UC_SPARE
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_uc_spare_s
{
  u32 spare : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_uc_spare_u
{
  struct ipa_hwio_def_ipa_uc_spare_s def;
  u32 value;
};

/*----------------------------------------------------------------------------
 * MODULE: IPA_UC_IPA_UC_MBOX
 *--------------------------------------------------------------------------*/

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_UC_MAILBOX_m_n
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_uc_mailbox_m_n_s
{
  u32 data : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_uc_mailbox_m_n_u
{
  struct ipa_hwio_def_ipa_uc_mailbox_m_n_s def;
  u32 value;
};

/*----------------------------------------------------------------------------
 * MODULE: IPA_RAM
 *--------------------------------------------------------------------------*/

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_SW_AREA_RAM_DIRECT_ACCESS_n
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_sw_area_ram_direct_access_n_s
{
  u32 data_word : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_sw_area_ram_direct_access_n_u
{
  struct ipa_hwio_def_ipa_sw_area_ram_direct_access_n_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_HW_AREA_RAM_DIRECT_ACCESS_n
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_hw_area_ram_direct_access_n_s
{
  u32 data_word : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_hw_area_ram_direct_access_n_u
{
  struct ipa_hwio_def_ipa_hw_area_ram_direct_access_n_s def;
  u32 value;
};

/*----------------------------------------------------------------------------
 * MODULE: IPA_EE
 *--------------------------------------------------------------------------*/

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_IRQ_STTS_EE_n
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_irq_stts_ee_n_s
{
  u32 bad_snoc_access_irq : 1;
  u32 reserved0 : 1;
  u32 uc_irq_0 : 1;
  u32 uc_irq_1 : 1;
  u32 uc_irq_2 : 1;
  u32 uc_irq_3 : 1;
  u32 uc_in_q_not_empty_irq : 1;
  u32 uc_rx_cmd_q_not_full_irq : 1;
  u32 proc_to_uc_ack_q_not_empty_irq : 1;
  u32 rx_err_irq : 1;
  u32 deaggr_err_irq : 1;
  u32 tx_err_irq : 1;
  u32 step_mode_irq : 1;
  u32 proc_err_irq : 1;
  u32 tx_suspend_irq : 1;
  u32 tx_holb_drop_irq : 1;
  u32 bam_gsi_idle_irq : 1;
  u32 pipe_yellow_marker_below_irq : 1;
  u32 pipe_red_marker_below_irq : 1;
  u32 pipe_yellow_marker_above_irq : 1;
  u32 pipe_red_marker_above_irq : 1;
  u32 ucp_irq : 1;
  u32 reserved1 : 1;
  u32 gsi_ee_irq : 1;
  u32 gsi_ipa_if_tlv_rcvd_irq : 1;
  u32 gsi_uc_irq : 1;
  u32 tlv_len_min_dsm_irq : 1;
  u32 drbip_pkt_exceed_max_size_irq : 1;
  u32 drbip_data_sctr_cfg_error_irq : 1;
  u32 drbip_imm_cmd_no_flsh_hzrd_irq : 1;
  u32 reserved2 : 2;
};

/* Union definition of register */
union ipa_hwio_def_ipa_irq_stts_ee_n_u
{
  struct ipa_hwio_def_ipa_irq_stts_ee_n_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_IRQ_EN_EE_n
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_irq_en_ee_n_s
{
  u32 bad_snoc_access_irq_en : 1;
  u32 reserved0 : 1;
  u32 uc_irq_0_irq_en : 1;
  u32 uc_irq_1_irq_en : 1;
  u32 uc_irq_2_irq_en : 1;
  u32 uc_irq_3_irq_en : 1;
  u32 uc_in_q_not_empty_irq_en : 1;
  u32 uc_rx_cmd_q_not_full_irq_en : 1;
  u32 proc_to_uc_ack_q_not_empty_irq_en : 1;
  u32 rx_err_irq_en : 1;
  u32 deaggr_err_irq_en : 1;
  u32 tx_err_irq_en : 1;
  u32 step_mode_irq_en : 1;
  u32 proc_err_irq_en : 1;
  u32 tx_suspend_irq_en : 1;
  u32 tx_holb_drop_irq_en : 1;
  u32 bam_gsi_idle_irq_en : 1;
  u32 pipe_yellow_marker_below_irq_en : 1;
  u32 pipe_red_marker_below_irq_en : 1;
  u32 pipe_yellow_marker_above_irq_en : 1;
  u32 pipe_red_marker_above_irq_en : 1;
  u32 ucp_irq_en : 1;
  u32 reserved1 : 1;
  u32 gsi_ee_irq_en : 1;
  u32 gsi_ipa_if_tlv_rcvd_irq_en : 1;
  u32 gsi_uc_irq_en : 1;
  u32 tlv_len_min_dsm_irq_en : 1;
  u32 drbip_pkt_exceed_max_size_irq_en : 1;
  u32 drbip_data_sctr_cfg_error_irq_en : 1;
  u32 drbip_imm_cmd_no_flsh_hzrd_irq_en : 1;
  u32 reserved2 : 2;
};

/* Union definition of register */
union ipa_hwio_def_ipa_irq_en_ee_n_u
{
  struct ipa_hwio_def_ipa_irq_en_ee_n_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_IRQ_CLR_EE_n
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_irq_clr_ee_n_s
{
  u32 bad_snoc_access_irq_clr : 1;
  u32 reserved0 : 1;
  u32 uc_irq_0_clr : 1;
  u32 uc_irq_1_clr : 1;
  u32 uc_irq_2_clr : 1;
  u32 uc_irq_3_clr : 1;
  u32 uc_in_q_not_empty_irq_clr : 1;
  u32 uc_rx_cmd_q_not_full_irq_clr : 1;
  u32 proc_to_uc_ack_q_not_empty_irq_clr : 1;
  u32 rx_err_irq_clr : 1;
  u32 deaggr_err_irq_clr : 1;
  u32 tx_err_irq_clr : 1;
  u32 step_mode_irq_clr : 1;
  u32 proc_err_irq_clr : 1;
  u32 tx_suspend_irq_clr : 1;
  u32 tx_holb_drop_irq_clr : 1;
  u32 bam_gsi_idle_irq_clr : 1;
  u32 pipe_yellow_marker_below_irq_clr : 1;
  u32 pipe_red_marker_below_irq_clr : 1;
  u32 pipe_yellow_marker_above_irq_clr : 1;
  u32 pipe_red_marker_above_irq_clr : 1;
  u32 ucp_irq_clr : 1;
  u32 reserved1 : 1;
  u32 gsi_ee_irq_clr : 1;
  u32 gsi_ipa_if_tlv_rcvd_irq_clr : 1;
  u32 gsi_uc_irq_clr : 1;
  u32 tlv_len_min_dsm_irq_clr : 1;
  u32 drbip_pkt_exceed_max_size_irq_clr : 1;
  u32 drbip_data_sctr_cfg_error_irq_clr : 1;
  u32 drbip_imm_cmd_no_flsh_hzrd_irq_clr : 1;
  u32 reserved2 : 2;
};

/* Union definition of register */
union ipa_hwio_def_ipa_irq_clr_ee_n_u
{
  struct ipa_hwio_def_ipa_irq_clr_ee_n_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_SNOC_FEC_EE_n
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_snoc_fec_ee_n_s
{
  u32 client : 8;
  u32 noc_port : 1;
  u32 noc_master : 3;
  u32 tid : 5;
  u32 reserved0 : 11;
  u32 valid : 1;
  u32 clear : 1;
  u32 reserved1 : 1;
  u32 direction : 1;
};

/* Union definition of register */
union ipa_hwio_def_ipa_snoc_fec_ee_n_u
{
  struct ipa_hwio_def_ipa_snoc_fec_ee_n_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_IRQ_EE_UC_n
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_irq_ee_uc_n_s
{
  u32 intr : 1;
  u32 reserved0 : 31;
};

/* Union definition of register */
union ipa_hwio_def_ipa_irq_ee_uc_n_u
{
  struct ipa_hwio_def_ipa_irq_ee_uc_n_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_FEC_ADDR_EE_n
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_fec_addr_ee_n_s
{
  u32 addr : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_fec_addr_ee_n_u
{
  struct ipa_hwio_def_ipa_fec_addr_ee_n_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_FEC_ADDR_MSB_EE_n
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_fec_addr_msb_ee_n_s
{
  u32 addr : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_fec_addr_msb_ee_n_u
{
  struct ipa_hwio_def_ipa_fec_addr_msb_ee_n_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_FEC_ATTR_EE_n
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_fec_attr_ee_n_s
{
  u32 opcode : 6;
  u32 error_info : 26;
};

/* Union definition of register */
union ipa_hwio_def_ipa_fec_attr_ee_n_u
{
  struct ipa_hwio_def_ipa_fec_attr_ee_n_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_DRBIP_FEC_INFO_EE_n
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_drbip_fec_info_ee_n_s
{
  u32 error_code : 4;
  u32 src_grp : 4;
  u32 src_pipe : 8;
  u32 required_data_sectors : 8;
  u32 avail_data_sectors : 8;
};

/* Union definition of register */
union ipa_hwio_def_ipa_drbip_fec_info_ee_n_u
{
  struct ipa_hwio_def_ipa_drbip_fec_info_ee_n_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_DRBIP_FEC_INFO_EXT_EE_n
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_drbip_fec_info_ext_ee_n_s
{
  u32 size : 16;
  u32 opocode : 8;
  u32 reserved0 : 8;
};

/* Union definition of register */
union ipa_hwio_def_ipa_drbip_fec_info_ext_ee_n_u
{
  struct ipa_hwio_def_ipa_drbip_fec_info_ext_ee_n_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_SUSPEND_IRQ_INFO_EE_n_REG_k
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_suspend_irq_info_ee_n_reg_k_s
{
  u32 endpoints : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_suspend_irq_info_ee_n_reg_k_u
{
  struct ipa_hwio_def_ipa_suspend_irq_info_ee_n_reg_k_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_SUSPEND_IRQ_EN_EE_n_REG_k
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_suspend_irq_en_ee_n_reg_k_s
{
  u32 endpoints : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_suspend_irq_en_ee_n_reg_k_u
{
  struct ipa_hwio_def_ipa_suspend_irq_en_ee_n_reg_k_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_SUSPEND_IRQ_CLR_EE_n_REG_k
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_suspend_irq_clr_ee_n_reg_k_s
{
  u32 endpoints : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_suspend_irq_clr_ee_n_reg_k_u
{
  struct ipa_hwio_def_ipa_suspend_irq_clr_ee_n_reg_k_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_HOLB_DROP_IRQ_INFO_EE_n_REG_k
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_holb_drop_irq_info_ee_n_reg_k_s
{
  u32 endpoints : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_holb_drop_irq_info_ee_n_reg_k_u
{
  struct ipa_hwio_def_ipa_holb_drop_irq_info_ee_n_reg_k_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_HOLB_DROP_IRQ_EN_EE_n_REG_k
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_holb_drop_irq_en_ee_n_reg_k_s
{
  u32 endpoints : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_holb_drop_irq_en_ee_n_reg_k_u
{
  struct ipa_hwio_def_ipa_holb_drop_irq_en_ee_n_reg_k_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_HOLB_DROP_IRQ_CLR_EE_n_REG_k
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_holb_drop_irq_clr_ee_n_reg_k_s
{
  u32 endpoints : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_holb_drop_irq_clr_ee_n_reg_k_u
{
  struct ipa_hwio_def_ipa_holb_drop_irq_clr_ee_n_reg_k_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_MODEM_BEARER_INIT_VALUES_0
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_modem_bearer_init_values_0_s
{
  u32 modem_bearer_init_l2_hdr_size : 8;
  u32 reserved0 : 4;
  u32 modem_bearer_init_cphr_algorithm : 4;
  u32 modem_bearer_init_cphr_key_indx : 5;
  u32 reserved1 : 3;
  u32 modem_bearer_init_bearer : 8;
};

/* Union definition of register */
union ipa_hwio_def_ipa_modem_bearer_init_values_0_u
{
  struct ipa_hwio_def_ipa_modem_bearer_init_values_0_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_MODEM_BEARER_INIT_VALUES_1
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_modem_bearer_init_values_1_s
{
  u32 modem_bearer_init_cphr_ofst_keystrm : 16;
  u32 modem_bearer_init_cphr_ofst_start : 14;
  u32 modem_bearer_init_direction : 1;
  u32 modem_bearer_init_bearer_sel : 1;
};

/* Union definition of register */
union ipa_hwio_def_ipa_modem_bearer_init_values_1_u
{
  struct ipa_hwio_def_ipa_modem_bearer_init_values_1_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_MODEM_BEARER_INIT_VALUES_2
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_modem_bearer_init_values_2_s
{
  u32 modem_bearer_init_ip_algorithm : 4;
  u32 modem_bearer_init_ip_key_indx : 5;
  u32 reserved0 : 3;
  u32 modem_bearer_init_ip_maci_size : 2;
  u32 reserved1 : 18;
};

/* Union definition of register */
union ipa_hwio_def_ipa_modem_bearer_init_values_2_u
{
  struct ipa_hwio_def_ipa_modem_bearer_init_values_2_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_MODEM_BEARER_CONFIG_VALUES_0
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_modem_bearer_config_values_0_s
{
  u32 modem_bearer_config_count_f : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_modem_bearer_config_values_0_u
{
  struct ipa_hwio_def_ipa_modem_bearer_config_values_0_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_MODEM_BEARER_CONFIG_VALUES_1
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_modem_bearer_config_values_1_s
{
  u32 modem_bearer_config_size_f : 16;
  u32 reserved0 : 16;
};

/* Union definition of register */
union ipa_hwio_def_ipa_modem_bearer_config_values_1_u
{
  struct ipa_hwio_def_ipa_modem_bearer_config_values_1_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_SECURED_PIPES_n
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_secured_pipes_n_s
{
  u32 endpoints : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_secured_pipes_n_u
{
  struct ipa_hwio_def_ipa_secured_pipes_n_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_MODEM_BEARER_INIT_VALUES_CFG
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_modem_bearer_init_values_cfg_s
{
  u32 bearer_context_index_sel : 2;
  u32 reserved0 : 30;
};

/* Union definition of register */
union ipa_hwio_def_ipa_modem_bearer_init_values_cfg_u
{
  struct ipa_hwio_def_ipa_modem_bearer_init_values_cfg_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_UC_REGS_INSIDE_IPA__CONTROL
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_uc_regs_inside_ipa__control_s
{
  u32 uc_enable : 1;
  u32 reserved0 : 31;
};

/* Union definition of register */
union ipa_hwio_def_ipa_uc_regs_inside_ipa__control_u
{
  struct ipa_hwio_def_ipa_uc_regs_inside_ipa__control_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_UC_REGS_INSIDE_IPA__NMI
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_uc_regs_inside_ipa__nmi_s
{
  u32 pulse : 1;
  u32 reserved0 : 31;
};

/* Union definition of register */
union ipa_hwio_def_ipa_uc_regs_inside_ipa__nmi_u
{
  struct ipa_hwio_def_ipa_uc_regs_inside_ipa__nmi_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_DRBIP_CFG
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_drbip_cfg_s
{
  u32 operation_mode : 1;
  u32 reserved0 : 31;
};

/* Union definition of register */
union ipa_hwio_def_ipa_drbip_cfg_u
{
  struct ipa_hwio_def_ipa_drbip_cfg_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_SET_UC_IRQ_EE_n
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_set_uc_irq_ee_n_s
{
  u32 set_uc_irq_0 : 1;
  u32 set_uc_irq_1 : 1;
  u32 set_uc_irq_2 : 1;
  u32 set_uc_irq_3 : 1;
  u32 reserved0 : 28;
};

/* Union definition of register */
union ipa_hwio_def_ipa_set_uc_irq_ee_n_u
{
  struct ipa_hwio_def_ipa_set_uc_irq_ee_n_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_SET_UC_IRQ_ALL_EES
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_set_uc_irq_all_ees_s
{
  u32 set_uc_irq_0 : 1;
  u32 set_uc_irq_1 : 1;
  u32 set_uc_irq_2 : 1;
  u32 set_uc_irq_3 : 1;
  u32 reserved0 : 28;
};

/* Union definition of register */
union ipa_hwio_def_ipa_set_uc_irq_all_ees_u
{
  struct ipa_hwio_def_ipa_set_uc_irq_all_ees_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_UCP_RESUME
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_ucp_resume_s
{
  u32 reserved0 : 1;
  u32 next_round_en : 1;
  u32 dest_pipe_override : 1;
  u32 reserved1 : 1;
  u32 ip_checksum_fix_en : 1;
  u32 tport_checksum_fix_en : 1;
  u32 reserved2 : 2;
  u32 dest_pipe_value : 8;
  u32 exception : 1;
  u32 reserved3 : 2;
  u32 next_pkt_parser_dis : 1;
  u32 metadata_override : 1;
  u32 reserved4 : 11;
};

/* Union definition of register */
union ipa_hwio_def_ipa_ucp_resume_u
{
  struct ipa_hwio_def_ipa_ucp_resume_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_UCP_RESUME_METADATA
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_ucp_resume_metadata_s
{
  u32 metadata : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_ucp_resume_metadata_u
{
  struct ipa_hwio_def_ipa_ucp_resume_metadata_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_PROC_UCP_CFG
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_proc_ucp_cfg_s
{
  u32 ipa_ucp_irq_sw_events_uc_mux_en : 1;
  u32 reserved0 : 31;
};

/* Union definition of register */
union ipa_hwio_def_ipa_proc_ucp_cfg_u
{
  struct ipa_hwio_def_ipa_proc_ucp_cfg_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_UC_PKT_PROCESS_BASE_ADDR_0
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_uc_pkt_process_base_addr_0_s
{
  u32 ipa_uc_pkt_process_context_base : 18;
  u32 reserved0 : 14;
};

/* Union definition of register */
union ipa_hwio_def_ipa_uc_pkt_process_base_addr_0_u
{
  struct ipa_hwio_def_ipa_uc_pkt_process_base_addr_0_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_UC_PKT_PROCESS_BASE_ADDR_1
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_uc_pkt_process_base_addr_1_s
{
  u32 ipa_uc_pkt_process_pkt_base : 18;
  u32 reserved0 : 14;
};

/* Union definition of register */
union ipa_hwio_def_ipa_uc_pkt_process_base_addr_1_u
{
  struct ipa_hwio_def_ipa_uc_pkt_process_base_addr_1_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_UC_PKT_PROCESS_BASE_ADDR_2
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_uc_pkt_process_base_addr_2_s
{
  u32 ipa_uc_pkt_process_hdr_base : 18;
  u32 reserved0 : 14;
};

/* Union definition of register */
union ipa_hwio_def_ipa_uc_pkt_process_base_addr_2_u
{
  struct ipa_hwio_def_ipa_uc_pkt_process_base_addr_2_s def;
  u32 value;
};

/*----------------------------------------------------------------------------
 * MODULE: IPA_DEBUG
 *--------------------------------------------------------------------------*/

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_RSRC_MNGR_SW_ACCESS_ALLOC_CFG
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_rsrc_mngr_sw_access_alloc_cfg_s
{
  u32 alloc_rsrc_type : 3;
  u32 reserved0 : 1;
  u32 alloc_rsrc_grp : 3;
  u32 reserved1 : 1;
  u32 alloc_rsrc_id_curr : 6;
  u32 reserved2 : 2;
  u32 alloc_list_id : 6;
  u32 reserved3 : 2;
  u32 alloc_hold : 1;
  u32 alloc_reserved : 1;
  u32 alloc_list_type : 2;
  u32 reserved4 : 4;
};

/* Union definition of register */
union ipa_hwio_def_ipa_rsrc_mngr_sw_access_alloc_cfg_u
{
  struct ipa_hwio_def_ipa_rsrc_mngr_sw_access_alloc_cfg_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_RSRC_MNGR_SW_ACCESS_SRCH_CFG
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_rsrc_mngr_sw_access_srch_cfg_s
{
  u32 srch_rsrc_type : 3;
  u32 reserved0 : 1;
  u32 srch_rsrc_cnt : 7;
  u32 reserved1 : 1;
  u32 srch_list_id : 6;
  u32 srch_list_type : 2;
  u32 reserved2 : 12;
};

/* Union definition of register */
union ipa_hwio_def_ipa_rsrc_mngr_sw_access_srch_cfg_u
{
  struct ipa_hwio_def_ipa_rsrc_mngr_sw_access_srch_cfg_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_RSRC_MNGR_SW_ACCESS_REL_CFG
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_rsrc_mngr_sw_access_rel_cfg_s
{
  u32 rel_rsrc_type : 3;
  u32 reserved0 : 1;
  u32 rel_rsrc_grp : 3;
  u32 reserved1 : 1;
  u32 rel_rsrc_id : 6;
  u32 reserved2 : 2;
  u32 rel_list_id : 6;
  u32 rel_list_type : 2;
  u32 reserved3 : 8;
};

/* Union definition of register */
union ipa_hwio_def_ipa_rsrc_mngr_sw_access_rel_cfg_u
{
  struct ipa_hwio_def_ipa_rsrc_mngr_sw_access_rel_cfg_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_RSRC_MNGR_SW_ACCESS_RSRV_CFG
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_rsrc_mngr_sw_access_rsrv_cfg_s
{
  u32 rsrv_rsrc_type : 3;
  u32 reserved0 : 1;
  u32 rsrv_rsrc_grp : 3;
  u32 reserved1 : 1;
  u32 rsrv_rsrc_amount : 6;
  u32 reserved2 : 18;
};

/* Union definition of register */
union ipa_hwio_def_ipa_rsrc_mngr_sw_access_rsrv_cfg_u
{
  struct ipa_hwio_def_ipa_rsrc_mngr_sw_access_rsrv_cfg_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_RSRC_MNGR_SW_ACCESS_CMD
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_rsrc_mngr_sw_access_cmd_s
{
  u32 alloc_valid : 1;
  u32 srch_valid : 1;
  u32 rel_valid : 1;
  u32 rsrv_valid : 1;
  u32 reserved0 : 28;
};

/* Union definition of register */
union ipa_hwio_def_ipa_rsrc_mngr_sw_access_cmd_u
{
  struct ipa_hwio_def_ipa_rsrc_mngr_sw_access_cmd_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_RSRC_MNGR_SW_ACCESS_STATUS
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_rsrc_mngr_sw_access_status_s
{
  u32 alloc_ready : 1;
  u32 srch_ready : 1;
  u32 rel_ready : 1;
  u32 rsrv_ready : 1;
  u32 alloc_rsrc_id_next : 6;
  u32 reserved0 : 2;
  u32 srch_rsrc_id_next : 6;
  u32 reserved1 : 14;
};

/* Union definition of register */
union ipa_hwio_def_ipa_rsrc_mngr_sw_access_status_u
{
  struct ipa_hwio_def_ipa_rsrc_mngr_sw_access_status_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_RSRC_MNGR_DB_CFG
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_rsrc_mngr_db_cfg_s
{
  u32 rsrc_grp_sel : 3;
  u32 reserved0 : 1;
  u32 rsrc_type_sel : 3;
  u32 reserved1 : 1;
  u32 rsrc_id_sel : 6;
  u32 reserved2 : 18;
};

/* Union definition of register */
union ipa_hwio_def_ipa_rsrc_mngr_db_cfg_u
{
  struct ipa_hwio_def_ipa_rsrc_mngr_db_cfg_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_RSRC_MNGR_DB_RSRC_READ
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_rsrc_mngr_db_rsrc_read_s
{
  u32 rsrc_occupied : 1;
  u32 rsrc_next_valid : 1;
  u32 reserved0 : 2;
  u32 rsrc_next_index : 6;
  u32 reserved1 : 22;
};

/* Union definition of register */
union ipa_hwio_def_ipa_rsrc_mngr_db_rsrc_read_u
{
  struct ipa_hwio_def_ipa_rsrc_mngr_db_rsrc_read_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_RSRC_MNGR_DB_LIST_READ
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_rsrc_mngr_db_list_read_s
{
  u32 rsrc_list_valid : 1;
  u32 rsrc_list_hold : 1;
  u32 reserved0 : 2;
  u32 rsrc_list_head_rsrc : 6;
  u32 reserved1 : 2;
  u32 rsrc_list_head_cnt : 7;
  u32 reserved2 : 1;
  u32 rsrc_list_entry_cnt : 7;
  u32 reserved3 : 5;
};

/* Union definition of register */
union ipa_hwio_def_ipa_rsrc_mngr_db_list_read_u
{
  struct ipa_hwio_def_ipa_rsrc_mngr_db_list_read_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_RSRC_MNGR_CONTEXTS
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_rsrc_mngr_contexts_s
{
  u32 rsrc_occupied_contexts_bitmap : 16;
  u32 reserved0 : 16;
};

/* Union definition of register */
union ipa_hwio_def_ipa_rsrc_mngr_contexts_u
{
  struct ipa_hwio_def_ipa_rsrc_mngr_contexts_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_BRESP_DB_CFG
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_bresp_db_cfg_s
{
  u32 sel_entry : 3;
  u32 sel_pipe : 8;
  u32 reserved0 : 21;
};

/* Union definition of register */
union ipa_hwio_def_ipa_bresp_db_cfg_u
{
  struct ipa_hwio_def_ipa_bresp_db_cfg_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_BRESP_DB_DATA
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_bresp_db_data_s
{
  u32 data : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_bresp_db_data_u
{
  struct ipa_hwio_def_ipa_bresp_db_data_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_SNOC_MONITORING_CFG
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_snoc_monitoring_cfg_s
{
  u32 enable : 1;
  u32 reserved0 : 31;
};

/* Union definition of register */
union ipa_hwio_def_ipa_snoc_monitoring_cfg_u
{
  struct ipa_hwio_def_ipa_snoc_monitoring_cfg_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_PCIE_SNOC_MONITOR_CNT
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_pcie_snoc_monitor_cnt_s
{
  u32 ar_value : 5;
  u32 reserved0 : 1;
  u32 aw_value : 5;
  u32 reserved1 : 1;
  u32 r_value : 5;
  u32 reserved2 : 1;
  u32 w_value : 5;
  u32 reserved3 : 1;
  u32 b_value : 5;
  u32 reserved4 : 3;
};

/* Union definition of register */
union ipa_hwio_def_ipa_pcie_snoc_monitor_cnt_u
{
  struct ipa_hwio_def_ipa_pcie_snoc_monitor_cnt_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_DDR_SNOC_MONITOR_CNT
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_ddr_snoc_monitor_cnt_s
{
  u32 ar_value : 5;
  u32 reserved0 : 1;
  u32 aw_value : 5;
  u32 reserved1 : 1;
  u32 r_value : 5;
  u32 reserved2 : 1;
  u32 w_value : 5;
  u32 reserved3 : 1;
  u32 b_value : 5;
  u32 reserved4 : 3;
};

/* Union definition of register */
union ipa_hwio_def_ipa_ddr_snoc_monitor_cnt_u
{
  struct ipa_hwio_def_ipa_ddr_snoc_monitor_cnt_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_GSI_SNOC_MONITOR_CNT
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_gsi_snoc_monitor_cnt_s
{
  u32 ar_value : 5;
  u32 reserved0 : 1;
  u32 aw_value : 5;
  u32 reserved1 : 1;
  u32 r_value : 5;
  u32 reserved2 : 1;
  u32 w_value : 5;
  u32 reserved3 : 1;
  u32 b_value : 5;
  u32 reserved4 : 3;
};

/* Union definition of register */
union ipa_hwio_def_ipa_gsi_snoc_monitor_cnt_u
{
  struct ipa_hwio_def_ipa_gsi_snoc_monitor_cnt_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_DEBUG_DATA
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_debug_data_s
{
  u32 debug_data : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_debug_data_u
{
  struct ipa_hwio_def_ipa_debug_data_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_TESTBUS_SEL
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_testbus_sel_s
{
  u32 testbus_en : 1;
  u32 reserved0 : 3;
  u32 external_block_select : 8;
  u32 internal_block_select : 8;
  u32 reserved1 : 12;
};

/* Union definition of register */
union ipa_hwio_def_ipa_testbus_sel_u
{
  struct ipa_hwio_def_ipa_testbus_sel_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_STEP_MODE_BREAKPOINTS
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_step_mode_breakpoints_s
{
  u32 hw_en : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_step_mode_breakpoints_u
{
  struct ipa_hwio_def_ipa_step_mode_breakpoints_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_STEP_MODE_STATUS
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_step_mode_status_s
{
  u32 hw_en : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_step_mode_status_u
{
  struct ipa_hwio_def_ipa_step_mode_status_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_STEP_MODE_GO
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_step_mode_go_s
{
  u32 hw_en : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_step_mode_go_u
{
  struct ipa_hwio_def_ipa_step_mode_go_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_HW_EVENTS_CFG
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_hw_events_cfg_s
{
  u32 hw_events_select : 4;
  u32 rx_events_pipe_select : 8;
  u32 reserved0 : 20;
};

/* Union definition of register */
union ipa_hwio_def_ipa_hw_events_cfg_u
{
  struct ipa_hwio_def_ipa_hw_events_cfg_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_LOG
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_log_s
{
  u32 reserved0 : 1;
  u32 log_en : 1;
  u32 reserved1 : 2;
  u32 log_pipe : 8;
  u32 log_length : 8;
  u32 log_reduction_en : 1;
  u32 log_dpl_l2_remove_en : 1;
  u32 reserved2 : 10;
};

/* Union definition of register */
union ipa_hwio_def_ipa_log_u
{
  struct ipa_hwio_def_ipa_log_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_LOG_BUF_HW_CMD_ADDR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_log_buf_hw_cmd_addr_s
{
  u32 start_addr : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_log_buf_hw_cmd_addr_u
{
  struct ipa_hwio_def_ipa_log_buf_hw_cmd_addr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_LOG_BUF_HW_CMD_ADDR_MSB
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_log_buf_hw_cmd_addr_msb_s
{
  u32 start_addr : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_log_buf_hw_cmd_addr_msb_u
{
  struct ipa_hwio_def_ipa_log_buf_hw_cmd_addr_msb_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_LOG_BUF_HW_CMD_WRITE_PTR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_log_buf_hw_cmd_write_ptr_s
{
  u32 writr_addr : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_log_buf_hw_cmd_write_ptr_u
{
  struct ipa_hwio_def_ipa_log_buf_hw_cmd_write_ptr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_LOG_BUF_HW_CMD_WRITE_PTR_MSB
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_log_buf_hw_cmd_write_ptr_msb_s
{
  u32 writr_addr : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_log_buf_hw_cmd_write_ptr_msb_u
{
  struct ipa_hwio_def_ipa_log_buf_hw_cmd_write_ptr_msb_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_LOG_BUF_HW_CMD_CFG
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_log_buf_hw_cmd_cfg_s
{
  u32 size : 16;
  u32 enable : 1;
  u32 skip_ddr_dma : 1;
  u32 tpdm_enable : 1;
  u32 reserved0 : 13;
};

/* Union definition of register */
union ipa_hwio_def_ipa_log_buf_hw_cmd_cfg_u
{
  struct ipa_hwio_def_ipa_log_buf_hw_cmd_cfg_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_LOG_BUF_HW_CMD_RAM_PTR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_log_buf_hw_cmd_ram_ptr_s
{
  u32 read_ptr : 14;
  u32 reserved0 : 2;
  u32 write_ptr : 14;
  u32 full : 1;
  u32 skip_ddr_wrap_happened : 1;
};

/* Union definition of register */
union ipa_hwio_def_ipa_log_buf_hw_cmd_ram_ptr_u
{
  struct ipa_hwio_def_ipa_log_buf_hw_cmd_ram_ptr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_LOG_BUF_HW_CMD_NOC_MASTER_SEL
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_log_buf_hw_cmd_noc_master_sel_s
{
  u32 noc_port_sel : 1;
  u32 qmb_rd_en : 1;
  u32 qmb_wr_en : 1;
  u32 gsi_rd_en : 1;
  u32 gsi_wr_en : 1;
  u32 uc_rd_en : 1;
  u32 uc_wr_en : 1;
  u32 qmb_resp_en : 1;
  u32 gsi_resp_en : 1;
  u32 uc_resp_en : 1;
  u32 reserved0 : 22;
};

/* Union definition of register */
union ipa_hwio_def_ipa_log_buf_hw_cmd_noc_master_sel_u
{
  struct ipa_hwio_def_ipa_log_buf_hw_cmd_noc_master_sel_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_STEP_MODE_HFETCHER_ADDR_LSB
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_step_mode_hfetcher_addr_lsb_s
{
  u32 addr_lsb : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_step_mode_hfetcher_addr_lsb_u
{
  struct ipa_hwio_def_ipa_step_mode_hfetcher_addr_lsb_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_STEP_MODE_HFETCHER_ADDR_MSB
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_step_mode_hfetcher_addr_msb_s
{
  u32 addr_msb : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_step_mode_hfetcher_addr_msb_u
{
  struct ipa_hwio_def_ipa_step_mode_hfetcher_addr_msb_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_STEP_MODE_HFETCHER_ADDR_RESULT
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_step_mode_hfetcher_addr_result_s
{
  u32 ctx_id_f : 4;
  u32 src_id_f : 8;
  u32 src_pipe_f : 8;
  u32 opcode_f : 2;
  u32 type_f : 1;
  u32 reserved0 : 9;
};

/* Union definition of register */
union ipa_hwio_def_ipa_step_mode_hfetcher_addr_result_u
{
  struct ipa_hwio_def_ipa_step_mode_hfetcher_addr_result_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_STEP_MODE_HSEQ_BREAKPOINT
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_step_mode_hseq_breakpoint_s
{
  u32 ctx_id_f : 4;
  u32 src_id_f : 8;
  u32 src_pipe_f : 8;
  u32 opcode_f : 2;
  u32 type_f : 1;
  u32 acl_id_f : 6;
  u32 reserved0 : 3;
};

/* Union definition of register */
union ipa_hwio_def_ipa_step_mode_hseq_breakpoint_u
{
  struct ipa_hwio_def_ipa_step_mode_hseq_breakpoint_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_STEP_MODE_HSEQ_BREAKPOINT_1
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_step_mode_hseq_breakpoint_1_s
{
  u32 ctx_id_v : 1;
  u32 src_id_v : 1;
  u32 src_pipe_v : 1;
  u32 opcode_v : 1;
  u32 type_v : 1;
  u32 acl_id_v : 1;
  u32 reserved0 : 26;
};

/* Union definition of register */
union ipa_hwio_def_ipa_step_mode_hseq_breakpoint_1_u
{
  struct ipa_hwio_def_ipa_step_mode_hseq_breakpoint_1_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_STEP_MODE_HSEQ_STATUS
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_step_mode_hseq_status_s
{
  u32 ctx_id_f : 4;
  u32 src_id_f : 8;
  u32 src_pipe_f : 8;
  u32 opcode_f : 2;
  u32 type_f : 1;
  u32 acl_id_f : 6;
  u32 reserved0 : 3;
};

/* Union definition of register */
union ipa_hwio_def_ipa_step_mode_hseq_status_u
{
  struct ipa_hwio_def_ipa_step_mode_hseq_status_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_STEP_MODE_DSEQ_BREAKPOINT
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_step_mode_dseq_breakpoint_s
{
  u32 ctx_id_f : 4;
  u32 src_id_f : 8;
  u32 src_pipe_f : 8;
  u32 opcode_f : 2;
  u32 type_f : 1;
  u32 acl_id_f : 6;
  u32 reserved0 : 3;
};

/* Union definition of register */
union ipa_hwio_def_ipa_step_mode_dseq_breakpoint_u
{
  struct ipa_hwio_def_ipa_step_mode_dseq_breakpoint_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_STEP_MODE_DSEQ_BREAKPOINT_1
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_step_mode_dseq_breakpoint_1_s
{
  u32 ctx_id_v : 1;
  u32 src_id_v : 1;
  u32 src_pipe_v : 1;
  u32 opcode_v : 1;
  u32 type_v : 1;
  u32 acl_id_v : 1;
  u32 reserved0 : 26;
};

/* Union definition of register */
union ipa_hwio_def_ipa_step_mode_dseq_breakpoint_1_u
{
  struct ipa_hwio_def_ipa_step_mode_dseq_breakpoint_1_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_STEP_MODE_DSEQ_STATUS
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_step_mode_dseq_status_s
{
  u32 ctx_id_f : 4;
  u32 src_id_f : 8;
  u32 src_pipe_f : 8;
  u32 opcode_f : 2;
  u32 type_f : 1;
  u32 acl_id_f : 6;
  u32 reserved0 : 3;
};

/* Union definition of register */
union ipa_hwio_def_ipa_step_mode_dseq_status_u
{
  struct ipa_hwio_def_ipa_step_mode_dseq_status_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_RX_ACKQ_CMD
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_rx_ackq_cmd_s
{
  u32 write_cmd : 1;
  u32 pop_cmd : 1;
  u32 release_rd_cmd : 1;
  u32 release_wr_cmd : 1;
  u32 reserved0 : 28;
};

/* Union definition of register */
union ipa_hwio_def_ipa_rx_ackq_cmd_u
{
  struct ipa_hwio_def_ipa_rx_ackq_cmd_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_RX_ACKQ_CFG
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_rx_ackq_cfg_s
{
  u32 block_rd_req : 1;
  u32 block_wr : 1;
  u32 reserved0 : 30;
};

/* Union definition of register */
union ipa_hwio_def_ipa_rx_ackq_cfg_u
{
  struct ipa_hwio_def_ipa_rx_ackq_cfg_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_RX_ACKQ_DATA_WR_0
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_rx_ackq_data_wr_0_s
{
  u32 ack_value1 : 16;
  u32 ack_value2 : 8;
  u32 ack_value1_type : 1;
  u32 reserved0 : 7;
};

/* Union definition of register */
union ipa_hwio_def_ipa_rx_ackq_data_wr_0_u
{
  struct ipa_hwio_def_ipa_rx_ackq_data_wr_0_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_RX_ACKQ_DATA_RD_0
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_rx_ackq_data_rd_0_s
{
  u32 ack_value1 : 16;
  u32 ack_value2 : 8;
  u32 ack_value1_type : 1;
  u32 reserved0 : 7;
};

/* Union definition of register */
union ipa_hwio_def_ipa_rx_ackq_data_rd_0_u
{
  struct ipa_hwio_def_ipa_rx_ackq_data_rd_0_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_RX_ACKQ_STATUS
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_rx_ackq_status_s
{
  u32 status : 1;
  u32 ackq_empty : 1;
  u32 ackq_full : 1;
  u32 reserved0 : 1;
  u32 ackq_count : 4;
  u32 ackq_depth : 4;
  u32 block_rd_ack : 1;
  u32 reserved1 : 19;
};

/* Union definition of register */
union ipa_hwio_def_ipa_rx_ackq_status_u
{
  struct ipa_hwio_def_ipa_rx_ackq_status_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_UC_ACKQ_CMD
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_uc_ackq_cmd_s
{
  u32 write_cmd : 1;
  u32 pop_cmd : 1;
  u32 release_rd_cmd : 1;
  u32 release_wr_cmd : 1;
  u32 reserved0 : 28;
};

/* Union definition of register */
union ipa_hwio_def_ipa_uc_ackq_cmd_u
{
  struct ipa_hwio_def_ipa_uc_ackq_cmd_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_UC_ACKQ_CFG
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_uc_ackq_cfg_s
{
  u32 block_rd : 1;
  u32 block_wr : 1;
  u32 reserved0 : 30;
};

/* Union definition of register */
union ipa_hwio_def_ipa_uc_ackq_cfg_u
{
  struct ipa_hwio_def_ipa_uc_ackq_cfg_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_UC_ACKQ_DATA_WR_0
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_uc_ackq_data_wr_0_s
{
  u32 ack_value1 : 16;
  u32 ack_value2 : 8;
  u32 ack_value1_type : 1;
  u32 reserved0 : 7;
};

/* Union definition of register */
union ipa_hwio_def_ipa_uc_ackq_data_wr_0_u
{
  struct ipa_hwio_def_ipa_uc_ackq_data_wr_0_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_UC_ACKQ_DATA_RD_0
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_uc_ackq_data_rd_0_s
{
  u32 ack_value1 : 16;
  u32 ack_value2 : 8;
  u32 ack_value1_type : 1;
  u32 reserved0 : 7;
};

/* Union definition of register */
union ipa_hwio_def_ipa_uc_ackq_data_rd_0_u
{
  struct ipa_hwio_def_ipa_uc_ackq_data_rd_0_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_UC_ACKQ_STATUS
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_uc_ackq_status_s
{
  u32 status : 1;
  u32 ackq_empty : 1;
  u32 ackq_full : 1;
  u32 reserved0 : 1;
  u32 ackq_count : 5;
  u32 reserved1 : 3;
  u32 ackq_depth : 5;
  u32 reserved2 : 15;
};

/* Union definition of register */
union ipa_hwio_def_ipa_uc_ackq_status_u
{
  struct ipa_hwio_def_ipa_uc_ackq_status_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_RX_SPLT_CMDQ_CMD_n
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_rx_splt_cmdq_cmd_n_s
{
  u32 write_cmd : 1;
  u32 pop_cmd : 1;
  u32 release_rd_cmd : 1;
  u32 release_wr_cmd : 1;
  u32 release_rd_pkt : 1;
  u32 release_wr_pkt : 1;
  u32 release_rd_pkt_enhanced : 1;
  u32 reserved0 : 25;
};

/* Union definition of register */
union ipa_hwio_def_ipa_rx_splt_cmdq_cmd_n_u
{
  struct ipa_hwio_def_ipa_rx_splt_cmdq_cmd_n_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_RX_SPLT_CMDQ_CFG_n
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_rx_splt_cmdq_cfg_n_s
{
  u32 block_rd : 1;
  u32 block_wr : 1;
  u32 reserved0 : 30;
};

/* Union definition of register */
union ipa_hwio_def_ipa_rx_splt_cmdq_cfg_n_u
{
  struct ipa_hwio_def_ipa_rx_splt_cmdq_cfg_n_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_RX_SPLT_CMDQ_DATA_WR_0_n
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_rx_splt_cmdq_data_wr_0_n_s
{
  u32 cmdq_packet_len_f : 16;
  u32 cmdq_src_len_f : 16;
};

/* Union definition of register */
union ipa_hwio_def_ipa_rx_splt_cmdq_data_wr_0_n_u
{
  struct ipa_hwio_def_ipa_rx_splt_cmdq_data_wr_0_n_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_RX_SPLT_CMDQ_DATA_WR_1_n
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_rx_splt_cmdq_data_wr_1_n_s
{
  u32 cmdq_src_pipe_f : 8;
  u32 cmdq_order_f : 2;
  u32 cmdq_flags_f : 6;
  u32 cmdq_opcode_f : 8;
  u32 cmdq_metadata_f : 8;
};

/* Union definition of register */
union ipa_hwio_def_ipa_rx_splt_cmdq_data_wr_1_n_u
{
  struct ipa_hwio_def_ipa_rx_splt_cmdq_data_wr_1_n_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_RX_SPLT_CMDQ_DATA_WR_2_n
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_rx_splt_cmdq_data_wr_2_n_s
{
  u32 cmdq_addr_lsb_f : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_rx_splt_cmdq_data_wr_2_n_u
{
  struct ipa_hwio_def_ipa_rx_splt_cmdq_data_wr_2_n_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_RX_SPLT_CMDQ_DATA_WR_3_n
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_rx_splt_cmdq_data_wr_3_n_s
{
  u32 cmdq_addr_msb_f : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_rx_splt_cmdq_data_wr_3_n_u
{
  struct ipa_hwio_def_ipa_rx_splt_cmdq_data_wr_3_n_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_RX_SPLT_CMDQ_DATA_RD_0_n
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_rx_splt_cmdq_data_rd_0_n_s
{
  u32 cmdq_packet_len_f : 16;
  u32 cmdq_src_len_f : 16;
};

/* Union definition of register */
union ipa_hwio_def_ipa_rx_splt_cmdq_data_rd_0_n_u
{
  struct ipa_hwio_def_ipa_rx_splt_cmdq_data_rd_0_n_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_RX_SPLT_CMDQ_DATA_RD_1_n
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_rx_splt_cmdq_data_rd_1_n_s
{
  u32 cmdq_src_pipe_f : 8;
  u32 cmdq_order_f : 2;
  u32 cmdq_flags_f : 6;
  u32 cmdq_opcode_f : 8;
  u32 cmdq_metadata_f : 8;
};

/* Union definition of register */
union ipa_hwio_def_ipa_rx_splt_cmdq_data_rd_1_n_u
{
  struct ipa_hwio_def_ipa_rx_splt_cmdq_data_rd_1_n_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_RX_SPLT_CMDQ_DATA_RD_2_n
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_rx_splt_cmdq_data_rd_2_n_s
{
  u32 cmdq_addr_lsb_f : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_rx_splt_cmdq_data_rd_2_n_u
{
  struct ipa_hwio_def_ipa_rx_splt_cmdq_data_rd_2_n_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_RX_SPLT_CMDQ_DATA_RD_3_n
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_rx_splt_cmdq_data_rd_3_n_s
{
  u32 cmdq_addr_msb_f : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_rx_splt_cmdq_data_rd_3_n_u
{
  struct ipa_hwio_def_ipa_rx_splt_cmdq_data_rd_3_n_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_RX_SPLT_CMDQ_STATUS_n
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_rx_splt_cmdq_status_n_s
{
  u32 status : 1;
  u32 cmdq_empty : 1;
  u32 cmdq_full : 1;
  u32 cmdq_count : 2;
  u32 cmdq_depth : 2;
  u32 reserved0 : 25;
};

/* Union definition of register */
union ipa_hwio_def_ipa_rx_splt_cmdq_status_n_u
{
  struct ipa_hwio_def_ipa_rx_splt_cmdq_status_n_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_TX_COMMANDER_CMDQ_CMD
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_tx_commander_cmdq_cmd_s
{
  u32 write_cmd : 1;
  u32 pop_cmd : 1;
  u32 release_wr_cmd : 1;
  u32 reserved0 : 1;
  u32 release_wr_pkt : 1;
  u32 reserved1 : 27;
};

/* Union definition of register */
union ipa_hwio_def_ipa_tx_commander_cmdq_cmd_u
{
  struct ipa_hwio_def_ipa_tx_commander_cmdq_cmd_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_TX_COMMANDER_CMDQ_CFG
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_tx_commander_cmdq_cfg_s
{
  u32 block_wr : 1;
  u32 reserved0 : 3;
  u32 tx_select : 1;
  u32 reserved1 : 27;
};

/* Union definition of register */
union ipa_hwio_def_ipa_tx_commander_cmdq_cfg_u
{
  struct ipa_hwio_def_ipa_tx_commander_cmdq_cfg_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_TX_COMMANDER_CMDQ_DATA_WR_0
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_tx_commander_cmdq_data_wr_0_s
{
  u32 cmdq_packet_len_f : 16;
  u32 cmdq_dest_len_f : 16;
};

/* Union definition of register */
union ipa_hwio_def_ipa_tx_commander_cmdq_data_wr_0_u
{
  struct ipa_hwio_def_ipa_tx_commander_cmdq_data_wr_0_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_TX_COMMANDER_CMDQ_DATA_WR_1
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_tx_commander_cmdq_data_wr_1_s
{
  u32 cmdq_dest_pipe_f : 8;
  u32 cmdq_order_f : 2;
  u32 cmdq_flags_f : 6;
  u32 cmdq_rsrc_type_f : 8;
  u32 cmdq_rsrc_arg_f : 8;
};

/* Union definition of register */
union ipa_hwio_def_ipa_tx_commander_cmdq_data_wr_1_u
{
  struct ipa_hwio_def_ipa_tx_commander_cmdq_data_wr_1_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_TX_COMMANDER_CMDQ_DATA_WR_2
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_tx_commander_cmdq_data_wr_2_s
{
  u32 cmdq_addr_f : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_tx_commander_cmdq_data_wr_2_u
{
  struct ipa_hwio_def_ipa_tx_commander_cmdq_data_wr_2_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_TX_COMMANDER_CMDQ_DATA_RD_0
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_tx_commander_cmdq_data_rd_0_s
{
  u32 cmdq_packet_len_f : 16;
  u32 cmdq_dest_len_f : 16;
};

/* Union definition of register */
union ipa_hwio_def_ipa_tx_commander_cmdq_data_rd_0_u
{
  struct ipa_hwio_def_ipa_tx_commander_cmdq_data_rd_0_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_TX_COMMANDER_CMDQ_DATA_RD_1
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_tx_commander_cmdq_data_rd_1_s
{
  u32 cmdq_dest_pipe_f : 8;
  u32 cmdq_order_f : 2;
  u32 cmdq_flags_f : 6;
  u32 cmdq_rsrc_type_f : 8;
  u32 cmdq_rsrc_arg_f : 8;
};

/* Union definition of register */
union ipa_hwio_def_ipa_tx_commander_cmdq_data_rd_1_u
{
  struct ipa_hwio_def_ipa_tx_commander_cmdq_data_rd_1_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_TX_COMMANDER_CMDQ_DATA_RD_2
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_tx_commander_cmdq_data_rd_2_s
{
  u32 cmdq_addr_f : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_tx_commander_cmdq_data_rd_2_u
{
  struct ipa_hwio_def_ipa_tx_commander_cmdq_data_rd_2_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_TX_COMMANDER_CMDQ_STATUS
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_tx_commander_cmdq_status_s
{
  u32 status : 1;
  u32 cmdq_empty : 1;
  u32 cmdq_full : 1;
  u32 reserved0 : 29;
};

/* Union definition of register */
union ipa_hwio_def_ipa_tx_commander_cmdq_status_u
{
  struct ipa_hwio_def_ipa_tx_commander_cmdq_status_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_RX_HPS_CMDQ_CMD
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_rx_hps_cmdq_cmd_s
{
  u32 write_cmd : 1;
  u32 pop_cmd : 1;
  u32 cmd_client : 3;
  u32 rd_req : 1;
  u32 reserved0 : 26;
};

/* Union definition of register */
union ipa_hwio_def_ipa_rx_hps_cmdq_cmd_u
{
  struct ipa_hwio_def_ipa_rx_hps_cmdq_cmd_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_RX_HPS_CMDQ_RELEASE_WR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_rx_hps_cmdq_release_wr_s
{
  u32 release_wr_cmd : 6;
  u32 reserved0 : 26;
};

/* Union definition of register */
union ipa_hwio_def_ipa_rx_hps_cmdq_release_wr_u
{
  struct ipa_hwio_def_ipa_rx_hps_cmdq_release_wr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_RX_HPS_CMDQ_RELEASE_RD
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_rx_hps_cmdq_release_rd_s
{
  u32 release_rd_cmd : 6;
  u32 reserved0 : 26;
};

/* Union definition of register */
union ipa_hwio_def_ipa_rx_hps_cmdq_release_rd_u
{
  struct ipa_hwio_def_ipa_rx_hps_cmdq_release_rd_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_RX_HPS_CMDQ_CFG_WR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_rx_hps_cmdq_cfg_wr_s
{
  u32 block_wr : 6;
  u32 reserved0 : 26;
};

/* Union definition of register */
union ipa_hwio_def_ipa_rx_hps_cmdq_cfg_wr_u
{
  struct ipa_hwio_def_ipa_rx_hps_cmdq_cfg_wr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_RX_HPS_CMDQ_CFG_RD
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_rx_hps_cmdq_cfg_rd_s
{
  u32 block_rd : 6;
  u32 reserved0 : 26;
};

/* Union definition of register */
union ipa_hwio_def_ipa_rx_hps_cmdq_cfg_rd_u
{
  struct ipa_hwio_def_ipa_rx_hps_cmdq_cfg_rd_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_RX_HPS_CMDQ_DATA_WR_0
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_rx_hps_cmdq_data_wr_0_s
{
  u32 cmdq_packet_len_f : 16;
  u32 cmdq_dest_len_f : 16;
};

/* Union definition of register */
union ipa_hwio_def_ipa_rx_hps_cmdq_data_wr_0_u
{
  struct ipa_hwio_def_ipa_rx_hps_cmdq_data_wr_0_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_RX_HPS_CMDQ_DATA_WR_1
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_rx_hps_cmdq_data_wr_1_s
{
  u32 cmdq_src_pipe_f : 8;
  u32 cmdq_order_f : 2;
  u32 cmdq_flags_f : 6;
  u32 cmdq_opcode_f : 8;
  u32 cmdq_metadata_f : 8;
};

/* Union definition of register */
union ipa_hwio_def_ipa_rx_hps_cmdq_data_wr_1_u
{
  struct ipa_hwio_def_ipa_rx_hps_cmdq_data_wr_1_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_RX_HPS_CMDQ_DATA_WR_2
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_rx_hps_cmdq_data_wr_2_s
{
  u32 cmdq_addr_lsb_f : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_rx_hps_cmdq_data_wr_2_u
{
  struct ipa_hwio_def_ipa_rx_hps_cmdq_data_wr_2_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_RX_HPS_CMDQ_DATA_WR_3
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_rx_hps_cmdq_data_wr_3_s
{
  u32 cmdq_addr_msb_f : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_rx_hps_cmdq_data_wr_3_u
{
  struct ipa_hwio_def_ipa_rx_hps_cmdq_data_wr_3_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_RX_HPS_CMDQ_DATA_RD_0
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_rx_hps_cmdq_data_rd_0_s
{
  u32 cmdq_packet_len_f : 16;
  u32 cmdq_dest_len_f : 16;
};

/* Union definition of register */
union ipa_hwio_def_ipa_rx_hps_cmdq_data_rd_0_u
{
  struct ipa_hwio_def_ipa_rx_hps_cmdq_data_rd_0_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_RX_HPS_CMDQ_DATA_RD_1
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_rx_hps_cmdq_data_rd_1_s
{
  u32 cmdq_src_pipe_f : 8;
  u32 cmdq_order_f : 2;
  u32 cmdq_flags_f : 6;
  u32 cmdq_opcode_f : 8;
  u32 cmdq_metadata_f : 8;
};

/* Union definition of register */
union ipa_hwio_def_ipa_rx_hps_cmdq_data_rd_1_u
{
  struct ipa_hwio_def_ipa_rx_hps_cmdq_data_rd_1_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_RX_HPS_CMDQ_DATA_RD_2
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_rx_hps_cmdq_data_rd_2_s
{
  u32 cmdq_addr_lsb_f : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_rx_hps_cmdq_data_rd_2_u
{
  struct ipa_hwio_def_ipa_rx_hps_cmdq_data_rd_2_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_RX_HPS_CMDQ_DATA_RD_3
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_rx_hps_cmdq_data_rd_3_s
{
  u32 cmdq_addr_msb_f : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_rx_hps_cmdq_data_rd_3_u
{
  struct ipa_hwio_def_ipa_rx_hps_cmdq_data_rd_3_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_RX_HPS_CMDQ_STATUS
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_rx_hps_cmdq_status_s
{
  u32 status : 1;
  u32 cmdq_full : 1;
  u32 cmdq_depth : 7;
  u32 reserved0 : 23;
};

/* Union definition of register */
union ipa_hwio_def_ipa_rx_hps_cmdq_status_u
{
  struct ipa_hwio_def_ipa_rx_hps_cmdq_status_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_RX_HPS_CMDQ_STATUS_EMPTY
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_rx_hps_cmdq_status_empty_s
{
  u32 cmdq_empty : 6;
  u32 reserved0 : 26;
};

/* Union definition of register */
union ipa_hwio_def_ipa_rx_hps_cmdq_status_empty_u
{
  struct ipa_hwio_def_ipa_rx_hps_cmdq_status_empty_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_RX_HPS_SNP
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_rx_hps_snp_s
{
  u32 snp_last : 1;
  u32 snp_write : 1;
  u32 snp_valid : 1;
  u32 snp_next_is_valid : 1;
  u32 snp_next : 4;
  u32 snp_head : 4;
  u32 snp_addr : 4;
  u32 reserved0 : 16;
};

/* Union definition of register */
union ipa_hwio_def_ipa_rx_hps_snp_u
{
  struct ipa_hwio_def_ipa_rx_hps_snp_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_RX_HPS_CMDQ_COUNT
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_rx_hps_cmdq_count_s
{
  u32 fifo_count : 7;
  u32 reserved0 : 25;
};

/* Union definition of register */
union ipa_hwio_def_ipa_rx_hps_cmdq_count_u
{
  struct ipa_hwio_def_ipa_rx_hps_cmdq_count_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_RX_HPS_CLIENTS_MIN_DEPTH_0
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_rx_hps_clients_min_depth_0_s
{
  u32 client_0_min_depth : 4;
  u32 reserved0 : 4;
  u32 client_1_min_depth : 4;
  u32 reserved1 : 4;
  u32 client_2_min_depth : 4;
  u32 reserved2 : 4;
  u32 client_3_min_depth : 4;
  u32 client_4_min_depth : 4;
};

/* Union definition of register */
union ipa_hwio_def_ipa_rx_hps_clients_min_depth_0_u
{
  struct ipa_hwio_def_ipa_rx_hps_clients_min_depth_0_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_RX_HPS_CLIENTS_MIN_DEPTH_1
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_rx_hps_clients_min_depth_1_s
{
  u32 client_5_min_depth : 4;
  u32 reserved0 : 4;
  u32 client_6_min_depth : 4;
  u32 reserved1 : 4;
  u32 client_7_min_depth : 4;
  u32 reserved2 : 4;
  u32 client_8_min_depth : 4;
  u32 client_9_min_depth : 4;
};

/* Union definition of register */
union ipa_hwio_def_ipa_rx_hps_clients_min_depth_1_u
{
  struct ipa_hwio_def_ipa_rx_hps_clients_min_depth_1_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_RX_HPS_CLIENTS_MAX_DEPTH_0
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_rx_hps_clients_max_depth_0_s
{
  u32 client_0_max_depth : 4;
  u32 reserved0 : 4;
  u32 client_1_max_depth : 4;
  u32 reserved1 : 4;
  u32 client_2_max_depth : 4;
  u32 reserved2 : 4;
  u32 client_3_max_depth : 4;
  u32 client_4_max_depth : 4;
};

/* Union definition of register */
union ipa_hwio_def_ipa_rx_hps_clients_max_depth_0_u
{
  struct ipa_hwio_def_ipa_rx_hps_clients_max_depth_0_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_RX_HPS_CLIENTS_MAX_DEPTH_1
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_rx_hps_clients_max_depth_1_s
{
  u32 client_5_max_depth : 4;
  u32 reserved0 : 4;
  u32 client_6_max_depth : 4;
  u32 reserved1 : 4;
  u32 client_7_max_depth : 4;
  u32 reserved2 : 4;
  u32 client_8_max_depth : 4;
  u32 client_9_max_depth : 4;
};

/* Union definition of register */
union ipa_hwio_def_ipa_rx_hps_clients_max_depth_1_u
{
  struct ipa_hwio_def_ipa_rx_hps_clients_max_depth_1_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_HPS_DPS_CMDQ_CMD
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_hps_dps_cmdq_cmd_s
{
  u32 write_cmd : 1;
  u32 pop_cmd : 1;
  u32 rd_req : 1;
  u32 reserved0 : 1;
  u32 cmd_client : 8;
  u32 reserved1 : 20;
};

/* Union definition of register */
union ipa_hwio_def_ipa_hps_dps_cmdq_cmd_u
{
  struct ipa_hwio_def_ipa_hps_dps_cmdq_cmd_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_HPS_DPS_CMDQ_DATA_WR_0
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_hps_dps_cmdq_data_wr_0_s
{
  u32 cmdq_ctx_id_f : 4;
  u32 cmdq_src_id_f : 8;
  u32 cmdq_src_pipe_f : 8;
  u32 cmdq_opcode_f : 2;
  u32 cmdq_type_f : 1;
  u32 cmdq_virt_cod_f : 1;
  u32 reserved0 : 8;
};

/* Union definition of register */
union ipa_hwio_def_ipa_hps_dps_cmdq_data_wr_0_u
{
  struct ipa_hwio_def_ipa_hps_dps_cmdq_data_wr_0_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_HPS_DPS_CMDQ_DATA_RD_0
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_hps_dps_cmdq_data_rd_0_s
{
  u32 cmdq_ctx_id_f : 4;
  u32 cmdq_src_id_f : 8;
  u32 cmdq_src_pipe_f : 8;
  u32 cmdq_opcode_f : 2;
  u32 cmdq_type_f : 1;
  u32 cmdq_virt_cod_f : 1;
  u32 reserved0 : 8;
};

/* Union definition of register */
union ipa_hwio_def_ipa_hps_dps_cmdq_data_rd_0_u
{
  struct ipa_hwio_def_ipa_hps_dps_cmdq_data_rd_0_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_HPS_DPS_CMDQ_STATUS
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_hps_dps_cmdq_status_s
{
  u32 status : 1;
  u32 cmdq_full : 1;
  u32 reserved0 : 2;
  u32 cmdq_depth : 8;
  u32 reserved1 : 20;
};

/* Union definition of register */
union ipa_hwio_def_ipa_hps_dps_cmdq_status_u
{
  struct ipa_hwio_def_ipa_hps_dps_cmdq_status_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_HPS_DPS_SNP
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_hps_dps_snp_s
{
  u32 snp_last : 1;
  u32 snp_write : 1;
  u32 snp_valid : 1;
  u32 snp_next_is_valid : 1;
  u32 snp_next : 8;
  u32 snp_head : 8;
  u32 snp_addr : 8;
  u32 reserved0 : 4;
};

/* Union definition of register */
union ipa_hwio_def_ipa_hps_dps_snp_u
{
  struct ipa_hwio_def_ipa_hps_dps_snp_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_HPS_DPS_CMDQ_COUNT
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_hps_dps_cmdq_count_s
{
  u32 fifo_count : 8;
  u32 reserved0 : 24;
};

/* Union definition of register */
union ipa_hwio_def_ipa_hps_dps_cmdq_count_u
{
  struct ipa_hwio_def_ipa_hps_dps_cmdq_count_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_HPS_DPS_CMDQ_RELEASE_WR_n
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_hps_dps_cmdq_release_wr_n_s
{
  u32 release_wr_cmd : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_hps_dps_cmdq_release_wr_n_u
{
  struct ipa_hwio_def_ipa_hps_dps_cmdq_release_wr_n_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_HPS_DPS_CMDQ_RELEASE_RD_n
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_hps_dps_cmdq_release_rd_n_s
{
  u32 release_rd_cmd : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_hps_dps_cmdq_release_rd_n_u
{
  struct ipa_hwio_def_ipa_hps_dps_cmdq_release_rd_n_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_HPS_DPS_CMDQ_CFG_WR_n
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_hps_dps_cmdq_cfg_wr_n_s
{
  u32 block_wr : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_hps_dps_cmdq_cfg_wr_n_u
{
  struct ipa_hwio_def_ipa_hps_dps_cmdq_cfg_wr_n_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_HPS_DPS_CMDQ_CFG_RD_n
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_hps_dps_cmdq_cfg_rd_n_s
{
  u32 block_rd : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_hps_dps_cmdq_cfg_rd_n_u
{
  struct ipa_hwio_def_ipa_hps_dps_cmdq_cfg_rd_n_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_HPS_DPS_CMDQ_STATUS_EMPTY_n
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_hps_dps_cmdq_status_empty_n_s
{
  u32 cmdq_empty : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_hps_dps_cmdq_status_empty_n_u
{
  struct ipa_hwio_def_ipa_hps_dps_cmdq_status_empty_n_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_DPS_TX_CMDQ_CMD
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_dps_tx_cmdq_cmd_s
{
  u32 write_cmd : 1;
  u32 pop_cmd : 1;
  u32 rd_req : 1;
  u32 cmd_client : 4;
  u32 reserved0 : 25;
};

/* Union definition of register */
union ipa_hwio_def_ipa_dps_tx_cmdq_cmd_u
{
  struct ipa_hwio_def_ipa_dps_tx_cmdq_cmd_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_DPS_TX_CMDQ_RELEASE_WR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_dps_tx_cmdq_release_wr_s
{
  u32 release_wr_cmd : 12;
  u32 reserved0 : 20;
};

/* Union definition of register */
union ipa_hwio_def_ipa_dps_tx_cmdq_release_wr_u
{
  struct ipa_hwio_def_ipa_dps_tx_cmdq_release_wr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_DPS_TX_CMDQ_RELEASE_RD
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_dps_tx_cmdq_release_rd_s
{
  u32 release_rd_cmd : 12;
  u32 reserved0 : 20;
};

/* Union definition of register */
union ipa_hwio_def_ipa_dps_tx_cmdq_release_rd_u
{
  struct ipa_hwio_def_ipa_dps_tx_cmdq_release_rd_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_DPS_TX_CMDQ_CFG_WR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_dps_tx_cmdq_cfg_wr_s
{
  u32 block_wr : 12;
  u32 reserved0 : 20;
};

/* Union definition of register */
union ipa_hwio_def_ipa_dps_tx_cmdq_cfg_wr_u
{
  struct ipa_hwio_def_ipa_dps_tx_cmdq_cfg_wr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_DPS_TX_CMDQ_CFG_RD
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_dps_tx_cmdq_cfg_rd_s
{
  u32 block_rd : 12;
  u32 reserved0 : 20;
};

/* Union definition of register */
union ipa_hwio_def_ipa_dps_tx_cmdq_cfg_rd_u
{
  struct ipa_hwio_def_ipa_dps_tx_cmdq_cfg_rd_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_DPS_TX_CMDQ_DATA_WR_0
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_dps_tx_cmdq_data_wr_0_s
{
  u32 cmdq_ctx_id_f : 4;
  u32 cmdq_src_id_f : 8;
  u32 cmdq_src_pipe_f : 8;
  u32 cmdq_opcode_f : 2;
  u32 cmdq_type_f : 1;
  u32 cmdq_virt_cod_f : 1;
  u32 seg_valid_f : 1;
  u32 seg_ctx_id_f : 2;
  u32 reserved0 : 5;
};

/* Union definition of register */
union ipa_hwio_def_ipa_dps_tx_cmdq_data_wr_0_u
{
  struct ipa_hwio_def_ipa_dps_tx_cmdq_data_wr_0_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_DPS_TX_CMDQ_DATA_RD_0
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_dps_tx_cmdq_data_rd_0_s
{
  u32 cmdq_ctx_id_f : 4;
  u32 cmdq_src_id_f : 8;
  u32 cmdq_src_pipe_f : 8;
  u32 cmdq_opcode_f : 2;
  u32 cmdq_type_f : 1;
  u32 cmdq_virt_cod_f : 1;
  u32 seg_valid_f : 1;
  u32 seg_ctx_id_f : 2;
  u32 reserved0 : 5;
};

/* Union definition of register */
union ipa_hwio_def_ipa_dps_tx_cmdq_data_rd_0_u
{
  struct ipa_hwio_def_ipa_dps_tx_cmdq_data_rd_0_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_DPS_TX_CMDQ_STATUS
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_dps_tx_cmdq_status_s
{
  u32 status : 1;
  u32 cmdq_full : 1;
  u32 reserved0 : 2;
  u32 cmdq_depth : 8;
  u32 reserved1 : 20;
};

/* Union definition of register */
union ipa_hwio_def_ipa_dps_tx_cmdq_status_u
{
  struct ipa_hwio_def_ipa_dps_tx_cmdq_status_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_DPS_TX_CMDQ_STATUS_EMPTY
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_dps_tx_cmdq_status_empty_s
{
  u32 cmdq_empty : 12;
  u32 reserved0 : 20;
};

/* Union definition of register */
union ipa_hwio_def_ipa_dps_tx_cmdq_status_empty_u
{
  struct ipa_hwio_def_ipa_dps_tx_cmdq_status_empty_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_DPS_TX_SNP
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_dps_tx_snp_s
{
  u32 snp_last : 1;
  u32 snp_write : 1;
  u32 snp_valid : 1;
  u32 snp_next_is_valid : 1;
  u32 snp_next : 8;
  u32 snp_head : 8;
  u32 snp_addr : 8;
  u32 reserved0 : 4;
};

/* Union definition of register */
union ipa_hwio_def_ipa_dps_tx_snp_u
{
  struct ipa_hwio_def_ipa_dps_tx_snp_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_DPS_TX_CMDQ_COUNT
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_dps_tx_cmdq_count_s
{
  u32 fifo_count : 7;
  u32 reserved0 : 25;
};

/* Union definition of register */
union ipa_hwio_def_ipa_dps_tx_cmdq_count_u
{
  struct ipa_hwio_def_ipa_dps_tx_cmdq_count_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_LOG_BUF_HW_SNIF_EL_EN
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_log_buf_hw_snif_el_en_s
{
  u32 bitmap : 3;
  u32 reserved0 : 29;
};

/* Union definition of register */
union ipa_hwio_def_ipa_log_buf_hw_snif_el_en_u
{
  struct ipa_hwio_def_ipa_log_buf_hw_snif_el_en_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_LOG_BUF_HW_SNIF_EL_WR_N_RD_SEL
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_log_buf_hw_snif_el_wr_n_rd_sel_s
{
  u32 bitmap : 3;
  u32 reserved0 : 29;
};

/* Union definition of register */
union ipa_hwio_def_ipa_log_buf_hw_snif_el_wr_n_rd_sel_u
{
  struct ipa_hwio_def_ipa_log_buf_hw_snif_el_wr_n_rd_sel_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_LOG_BUF_HW_SNIF_EL_CLI_MUX
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_log_buf_hw_snif_el_cli_mux_s
{
  u32 all_cli_mux_concat : 15;
  u32 reserved0 : 17;
};

/* Union definition of register */
union ipa_hwio_def_ipa_log_buf_hw_snif_el_cli_mux_u
{
  struct ipa_hwio_def_ipa_log_buf_hw_snif_el_cli_mux_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_LOG_BUF_HW_SNIF_EL_COMP_VAL_0_CLI_n
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_log_buf_hw_snif_el_comp_val_0_cli_n_s
{
  u32 value : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_log_buf_hw_snif_el_comp_val_0_cli_n_u
{
  struct ipa_hwio_def_ipa_log_buf_hw_snif_el_comp_val_0_cli_n_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_LOG_BUF_HW_SNIF_EL_COMP_VAL_1_CLI_n
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_log_buf_hw_snif_el_comp_val_1_cli_n_s
{
  u32 value : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_log_buf_hw_snif_el_comp_val_1_cli_n_u
{
  struct ipa_hwio_def_ipa_log_buf_hw_snif_el_comp_val_1_cli_n_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_LOG_BUF_HW_SNIF_EL_COMP_VAL_2_CLI_n
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_log_buf_hw_snif_el_comp_val_2_cli_n_s
{
  u32 value : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_log_buf_hw_snif_el_comp_val_2_cli_n_u
{
  struct ipa_hwio_def_ipa_log_buf_hw_snif_el_comp_val_2_cli_n_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_LOG_BUF_HW_SNIF_EL_COMP_VAL_3_CLI_n
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_log_buf_hw_snif_el_comp_val_3_cli_n_s
{
  u32 value : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_log_buf_hw_snif_el_comp_val_3_cli_n_u
{
  struct ipa_hwio_def_ipa_log_buf_hw_snif_el_comp_val_3_cli_n_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_LOG_BUF_HW_SNIF_EL_MASK_VAL_0_CLI_n
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_log_buf_hw_snif_el_mask_val_0_cli_n_s
{
  u32 value : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_log_buf_hw_snif_el_mask_val_0_cli_n_u
{
  struct ipa_hwio_def_ipa_log_buf_hw_snif_el_mask_val_0_cli_n_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_LOG_BUF_HW_SNIF_EL_MASK_VAL_1_CLI_n
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_log_buf_hw_snif_el_mask_val_1_cli_n_s
{
  u32 value : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_log_buf_hw_snif_el_mask_val_1_cli_n_u
{
  struct ipa_hwio_def_ipa_log_buf_hw_snif_el_mask_val_1_cli_n_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_LOG_BUF_HW_SNIF_EL_MASK_VAL_2_CLI_n
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_log_buf_hw_snif_el_mask_val_2_cli_n_s
{
  u32 value : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_log_buf_hw_snif_el_mask_val_2_cli_n_u
{
  struct ipa_hwio_def_ipa_log_buf_hw_snif_el_mask_val_2_cli_n_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_LOG_BUF_HW_SNIF_EL_MASK_VAL_3_CLI_n
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_log_buf_hw_snif_el_mask_val_3_cli_n_s
{
  u32 value : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_log_buf_hw_snif_el_mask_val_3_cli_n_u
{
  struct ipa_hwio_def_ipa_log_buf_hw_snif_el_mask_val_3_cli_n_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_LOG_BUF_HW_SNIF_LEGACY_RX
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_log_buf_hw_snif_legacy_rx_s
{
  u32 src_group_sel : 3;
  u32 reserved0 : 29;
};

/* Union definition of register */
union ipa_hwio_def_ipa_log_buf_hw_snif_legacy_rx_u
{
  struct ipa_hwio_def_ipa_log_buf_hw_snif_legacy_rx_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_ACKMNGR_CMDQ_CMD
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_ackmngr_cmdq_cmd_s
{
  u32 write_cmd : 1;
  u32 pop_cmd : 1;
  u32 cmd_client : 8;
  u32 rd_req : 1;
  u32 reserved0 : 21;
};

/* Union definition of register */
union ipa_hwio_def_ipa_ackmngr_cmdq_cmd_u
{
  struct ipa_hwio_def_ipa_ackmngr_cmdq_cmd_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_ACKMNGR_CMDQ_DATA_RD
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_ackmngr_cmdq_data_rd_s
{
  u32 cmdq_src_id : 8;
  u32 cmdq_length : 16;
  u32 cmdq_origin : 1;
  u32 cmdq_sent : 1;
  u32 cmdq_src_id_valid : 1;
  u32 cmdq_error : 1;
  u32 reserved0 : 4;
};

/* Union definition of register */
union ipa_hwio_def_ipa_ackmngr_cmdq_data_rd_u
{
  struct ipa_hwio_def_ipa_ackmngr_cmdq_data_rd_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_ACKMNGR_CMDQ_STATUS
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_ackmngr_cmdq_status_s
{
  u32 status : 1;
  u32 cmdq_full : 1;
  u32 cmdq_depth : 7;
  u32 reserved0 : 23;
};

/* Union definition of register */
union ipa_hwio_def_ipa_ackmngr_cmdq_status_u
{
  struct ipa_hwio_def_ipa_ackmngr_cmdq_status_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_ACKMNGR_CMDQ_STATUS_EMPTY_n
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_ackmngr_cmdq_status_empty_n_s
{
  u32 cmdq_empty : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_ackmngr_cmdq_status_empty_n_u
{
  struct ipa_hwio_def_ipa_ackmngr_cmdq_status_empty_n_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_ACKMNGR_CMDQ_COUNT
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_ackmngr_cmdq_count_s
{
  u32 fifo_count : 7;
  u32 reserved0 : 25;
};

/* Union definition of register */
union ipa_hwio_def_ipa_ackmngr_cmdq_count_u
{
  struct ipa_hwio_def_ipa_ackmngr_cmdq_count_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_GSI_FIFO_STATUS_CTRL
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_gsi_fifo_status_ctrl_s
{
  u32 ipa_gsi_fifo_status_port_sel : 5;
  u32 ipa_gsi_fifo_status_en : 1;
  u32 reserved0 : 26;
};

/* Union definition of register */
union ipa_hwio_def_ipa_gsi_fifo_status_ctrl_u
{
  struct ipa_hwio_def_ipa_gsi_fifo_status_ctrl_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_GSI_TLV_FIFO_STATUS
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_gsi_tlv_fifo_status_s
{
  u32 fifo_wr_ptr : 8;
  u32 fifo_rd_ptr : 8;
  u32 fifo_rd_pub_ptr : 8;
  u32 fifo_empty : 1;
  u32 fifo_empty_pub : 1;
  u32 fifo_almost_full : 1;
  u32 fifo_full : 1;
  u32 fifo_almost_full_pub : 1;
  u32 fifo_full_pub : 1;
  u32 fifo_head_is_bubble : 1;
  u32 reserved0 : 1;
};

/* Union definition of register */
union ipa_hwio_def_ipa_gsi_tlv_fifo_status_u
{
  struct ipa_hwio_def_ipa_gsi_tlv_fifo_status_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_GSI_AOS_FIFO_STATUS
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_gsi_aos_fifo_status_s
{
  u32 fifo_wr_ptr : 8;
  u32 fifo_rd_ptr : 8;
  u32 fifo_rd_pub_ptr : 8;
  u32 fifo_empty : 1;
  u32 fifo_empty_pub : 1;
  u32 fifo_almost_full : 1;
  u32 fifo_full : 1;
  u32 fifo_almost_full_pub : 1;
  u32 fifo_full_pub : 1;
  u32 fifo_head_is_bubble : 1;
  u32 reserved0 : 1;
};

/* Union definition of register */
union ipa_hwio_def_ipa_gsi_aos_fifo_status_u
{
  struct ipa_hwio_def_ipa_gsi_aos_fifo_status_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_ENDP_GSI_CONS_BYTES_TLV
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_endp_gsi_cons_bytes_tlv_s
{
  u32 cons_bytes : 16;
  u32 reserved0 : 16;
};

/* Union definition of register */
union ipa_hwio_def_ipa_endp_gsi_cons_bytes_tlv_u
{
  struct ipa_hwio_def_ipa_endp_gsi_cons_bytes_tlv_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_ENDP_GSI_CONS_BYTES_AOS
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_endp_gsi_cons_bytes_aos_s
{
  u32 cons_bytes : 16;
  u32 reserved0 : 16;
};

/* Union definition of register */
union ipa_hwio_def_ipa_endp_gsi_cons_bytes_aos_u
{
  struct ipa_hwio_def_ipa_endp_gsi_cons_bytes_aos_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_LOG_BUF_HW_GEN_RAM_OFFSET
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_log_buf_hw_gen_ram_offset_s
{
  u32 ram_region_baddr : 19;
  u32 reserved0 : 1;
  u32 ram_region_size : 4;
  u32 reserved1 : 7;
  u32 enable : 1;
};

/* Union definition of register */
union ipa_hwio_def_ipa_log_buf_hw_gen_ram_offset_u
{
  struct ipa_hwio_def_ipa_log_buf_hw_gen_ram_offset_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_UC_RX_HND_CMDQ_CMD
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_uc_rx_hnd_cmdq_cmd_s
{
  u32 write_cmd : 1;
  u32 pop_cmd : 1;
  u32 release_rd_cmd : 1;
  u32 release_wr_cmd : 1;
  u32 release_rd_pkt : 1;
  u32 release_wr_pkt : 1;
  u32 release_rd_pkt_enhanced : 1;
  u32 reserved0 : 25;
};

/* Union definition of register */
union ipa_hwio_def_ipa_uc_rx_hnd_cmdq_cmd_u
{
  struct ipa_hwio_def_ipa_uc_rx_hnd_cmdq_cmd_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_UC_RX_HND_CMDQ_CFG
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_uc_rx_hnd_cmdq_cfg_s
{
  u32 block_rd : 1;
  u32 block_wr : 1;
  u32 reserved0 : 30;
};

/* Union definition of register */
union ipa_hwio_def_ipa_uc_rx_hnd_cmdq_cfg_u
{
  struct ipa_hwio_def_ipa_uc_rx_hnd_cmdq_cfg_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_UC_RX_HND_CMDQ_DATA_WR_0
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_uc_rx_hnd_cmdq_data_wr_0_s
{
  u32 cmdq_packet_len_f : 16;
  u32 cmdq_src_len_f : 16;
};

/* Union definition of register */
union ipa_hwio_def_ipa_uc_rx_hnd_cmdq_data_wr_0_u
{
  struct ipa_hwio_def_ipa_uc_rx_hnd_cmdq_data_wr_0_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_UC_RX_HND_CMDQ_DATA_WR_1
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_uc_rx_hnd_cmdq_data_wr_1_s
{
  u32 cmdq_src_pipe_f : 8;
  u32 cmdq_order_f : 2;
  u32 cmdq_flags_f : 6;
  u32 cmdq_opcode_f : 8;
  u32 cmdq_metadata_f : 8;
};

/* Union definition of register */
union ipa_hwio_def_ipa_uc_rx_hnd_cmdq_data_wr_1_u
{
  struct ipa_hwio_def_ipa_uc_rx_hnd_cmdq_data_wr_1_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_UC_RX_HND_CMDQ_DATA_WR_2
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_uc_rx_hnd_cmdq_data_wr_2_s
{
  u32 cmdq_addr_lsb_f : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_uc_rx_hnd_cmdq_data_wr_2_u
{
  struct ipa_hwio_def_ipa_uc_rx_hnd_cmdq_data_wr_2_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_UC_RX_HND_CMDQ_DATA_WR_3
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_uc_rx_hnd_cmdq_data_wr_3_s
{
  u32 cmdq_addr_msb_f : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_uc_rx_hnd_cmdq_data_wr_3_u
{
  struct ipa_hwio_def_ipa_uc_rx_hnd_cmdq_data_wr_3_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_UC_RX_HND_CMDQ_DATA_RD_0
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_uc_rx_hnd_cmdq_data_rd_0_s
{
  u32 cmdq_packet_len_f : 16;
  u32 cmdq_src_len_f : 16;
};

/* Union definition of register */
union ipa_hwio_def_ipa_uc_rx_hnd_cmdq_data_rd_0_u
{
  struct ipa_hwio_def_ipa_uc_rx_hnd_cmdq_data_rd_0_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_UC_RX_HND_CMDQ_DATA_RD_1
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_uc_rx_hnd_cmdq_data_rd_1_s
{
  u32 cmdq_src_pipe_f : 8;
  u32 cmdq_order_f : 2;
  u32 cmdq_flags_f : 6;
  u32 cmdq_opcode_f : 8;
  u32 cmdq_metadata_f : 8;
};

/* Union definition of register */
union ipa_hwio_def_ipa_uc_rx_hnd_cmdq_data_rd_1_u
{
  struct ipa_hwio_def_ipa_uc_rx_hnd_cmdq_data_rd_1_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_UC_RX_HND_CMDQ_DATA_RD_2
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_uc_rx_hnd_cmdq_data_rd_2_s
{
  u32 cmdq_addr_lsb_f : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_uc_rx_hnd_cmdq_data_rd_2_u
{
  struct ipa_hwio_def_ipa_uc_rx_hnd_cmdq_data_rd_2_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_UC_RX_HND_CMDQ_DATA_RD_3
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_uc_rx_hnd_cmdq_data_rd_3_s
{
  u32 cmdq_addr_msb_f : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_uc_rx_hnd_cmdq_data_rd_3_u
{
  struct ipa_hwio_def_ipa_uc_rx_hnd_cmdq_data_rd_3_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_UC_RX_HND_CMDQ_STATUS
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_uc_rx_hnd_cmdq_status_s
{
  u32 status : 1;
  u32 cmdq_empty : 1;
  u32 cmdq_full : 1;
  u32 cmdq_count : 4;
  u32 cmdq_depth : 4;
  u32 reserved0 : 21;
};

/* Union definition of register */
union ipa_hwio_def_ipa_uc_rx_hnd_cmdq_status_u
{
  struct ipa_hwio_def_ipa_uc_rx_hnd_cmdq_status_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_RAM_HW_FIRST
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_ram_hw_first_s
{
  u32 address : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_ram_hw_first_u
{
  struct ipa_hwio_def_ipa_ram_hw_first_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_RAM_HW_LAST
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_ram_hw_last_s
{
  u32 address : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_ram_hw_last_u
{
  struct ipa_hwio_def_ipa_ram_hw_last_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_RAM_FRAG_FRST_BASE_ADDR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_ram_frag_frst_base_addr_s
{
  u32 address : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_ram_frag_frst_base_addr_u
{
  struct ipa_hwio_def_ipa_ram_frag_frst_base_addr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_RAM_FRAG_SCND_BASE_ADDR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_ram_frag_scnd_base_addr_s
{
  u32 address : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_ram_frag_scnd_base_addr_u
{
  struct ipa_hwio_def_ipa_ram_frag_scnd_base_addr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_RAM_GSI_TLV_BASE_ADDR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_ram_gsi_tlv_base_addr_s
{
  u32 address : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_ram_gsi_tlv_base_addr_u
{
  struct ipa_hwio_def_ipa_ram_gsi_tlv_base_addr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_RAM_DCPH_KEYS_FIRST
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_ram_dcph_keys_first_s
{
  u32 address : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_ram_dcph_keys_first_u
{
  struct ipa_hwio_def_ipa_ram_dcph_keys_first_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_RAM_DCPH_KEYS_LAST
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_ram_dcph_keys_last_s
{
  u32 address : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_ram_dcph_keys_last_u
{
  struct ipa_hwio_def_ipa_ram_dcph_keys_last_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_DPS_SEQUENCER_FIRST
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_dps_sequencer_first_s
{
  u32 address : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_dps_sequencer_first_u
{
  struct ipa_hwio_def_ipa_dps_sequencer_first_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_DPS_SEQUENCER_LAST
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_dps_sequencer_last_s
{
  u32 address : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_dps_sequencer_last_u
{
  struct ipa_hwio_def_ipa_dps_sequencer_last_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_HPS_SEQUENCER_FIRST
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_hps_sequencer_first_s
{
  u32 address : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_hps_sequencer_first_u
{
  struct ipa_hwio_def_ipa_hps_sequencer_first_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_HPS_SEQUENCER_LAST
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_hps_sequencer_last_s
{
  u32 address : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_hps_sequencer_last_u
{
  struct ipa_hwio_def_ipa_hps_sequencer_last_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_RAM_PKT_CTX_BASE_ADDR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_ram_pkt_ctx_base_addr_s
{
  u32 address : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_ram_pkt_ctx_base_addr_u
{
  struct ipa_hwio_def_ipa_ram_pkt_ctx_base_addr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_RAM_SW_AREA_BASE_ADDR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_ram_sw_area_base_addr_s
{
  u32 address : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_ram_sw_area_base_addr_u
{
  struct ipa_hwio_def_ipa_ram_sw_area_base_addr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_RAM_HDRI_TYPE1_BASE_ADDR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_ram_hdri_type1_base_addr_s
{
  u32 address : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_ram_hdri_type1_base_addr_u
{
  struct ipa_hwio_def_ipa_ram_hdri_type1_base_addr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_RAM_AGGR_NLO_COUNTERS_BASE_ADDR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_ram_aggr_nlo_counters_base_addr_s
{
  u32 address : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_ram_aggr_nlo_counters_base_addr_u
{
  struct ipa_hwio_def_ipa_ram_aggr_nlo_counters_base_addr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_RAM_NLO_VP_CACHE_BASE_ADDR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_ram_nlo_vp_cache_base_addr_s
{
  u32 address : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_ram_nlo_vp_cache_base_addr_u
{
  struct ipa_hwio_def_ipa_ram_nlo_vp_cache_base_addr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_RAM_COAL_VP_CACHE_BASE_ADDR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_ram_coal_vp_cache_base_addr_s
{
  u32 address : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_ram_coal_vp_cache_base_addr_u
{
  struct ipa_hwio_def_ipa_ram_coal_vp_cache_base_addr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_RAM_COAL_VP_FIFO_BASE_ADDR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_ram_coal_vp_fifo_base_addr_s
{
  u32 address : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_ram_coal_vp_fifo_base_addr_u
{
  struct ipa_hwio_def_ipa_ram_coal_vp_fifo_base_addr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_RAM_AGGR_BASE_ADDR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_ram_aggr_base_addr_s
{
  u32 address : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_ram_aggr_base_addr_u
{
  struct ipa_hwio_def_ipa_ram_aggr_base_addr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_RAM_TX_COUNTERS_BASE_ADDR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_ram_tx_counters_base_addr_s
{
  u32 address : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_ram_tx_counters_base_addr_u
{
  struct ipa_hwio_def_ipa_ram_tx_counters_base_addr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_RAM_DPL_FIFO_BASE_ADDR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_ram_dpl_fifo_base_addr_s
{
  u32 address : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_ram_dpl_fifo_base_addr_u
{
  struct ipa_hwio_def_ipa_ram_dpl_fifo_base_addr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_RAM_COAL_MASTER_VP_CTX_BASE_ADDR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_ram_coal_master_vp_ctx_base_addr_s
{
  u32 address : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_ram_coal_master_vp_ctx_base_addr_u
{
  struct ipa_hwio_def_ipa_ram_coal_master_vp_ctx_base_addr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_RAM_COAL_MASTER_VP_AGGR_BASE_ADDR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_ram_coal_master_vp_aggr_base_addr_s
{
  u32 address : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_ram_coal_master_vp_aggr_base_addr_u
{
  struct ipa_hwio_def_ipa_ram_coal_master_vp_aggr_base_addr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_RAM_COAL_SLAVE_VP_CTX_BASE_ADDR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_ram_coal_slave_vp_ctx_base_addr_s
{
  u32 address : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_ram_coal_slave_vp_ctx_base_addr_u
{
  struct ipa_hwio_def_ipa_ram_coal_slave_vp_ctx_base_addr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_RAM_UL_NLO_AGGR_BASE_ADDR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_ram_ul_nlo_aggr_base_addr_s
{
  u32 address : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_ram_ul_nlo_aggr_base_addr_u
{
  struct ipa_hwio_def_ipa_ram_ul_nlo_aggr_base_addr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_RAM_UC_IRAM_ADDR_BASE_ADDR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_ram_uc_iram_addr_base_addr_s
{
  u32 address : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_ram_uc_iram_addr_base_addr_u
{
  struct ipa_hwio_def_ipa_ram_uc_iram_addr_base_addr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_RAM_SNIFFER_HW_BASE_ADDR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_ram_sniffer_hw_base_addr_s
{
  u32 address : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_ram_sniffer_hw_base_addr_u
{
  struct ipa_hwio_def_ipa_ram_sniffer_hw_base_addr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_RAM_FILTER_ROUTER_CACHE_BASE_ADDR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_ram_filter_router_cache_base_addr_s
{
  u32 address : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_ram_filter_router_cache_base_addr_u
{
  struct ipa_hwio_def_ipa_ram_filter_router_cache_base_addr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_SPARE_REG_1
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_spare_reg_1_s
{
  u32 spare_bits : 16;
  u32 reserved0 : 16;
};

/* Union definition of register */
union ipa_hwio_def_ipa_spare_reg_1_u
{
  struct ipa_hwio_def_ipa_spare_reg_1_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_HPS_UC2SEQ_PUSH
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_hps_uc2seq_push_s
{
  u32 src_pipe : 8;
  u32 src_flags : 2;
  u32 src_id : 8;
  u32 ctx_id : 4;
  u32 reserved0 : 8;
  u32 virt_opcode : 1;
  u32 type : 1;
};

/* Union definition of register */
union ipa_hwio_def_ipa_hps_uc2seq_push_u
{
  struct ipa_hwio_def_ipa_hps_uc2seq_push_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_HPS_UC2SEQ_STATUS
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_hps_uc2seq_status_s
{
  u32 fill_level : 4;
  u32 reserved0 : 28;
};

/* Union definition of register */
union ipa_hwio_def_ipa_hps_uc2seq_status_u
{
  struct ipa_hwio_def_ipa_hps_uc2seq_status_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_HPS_SEQ2UC_RD
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_hps_seq2uc_rd_s
{
  u32 src_pipe : 8;
  u32 src_flags : 2;
  u32 src_id : 8;
  u32 ctx_id : 4;
  u32 reserved0 : 9;
  u32 type : 1;
};

/* Union definition of register */
union ipa_hwio_def_ipa_hps_seq2uc_rd_u
{
  struct ipa_hwio_def_ipa_hps_seq2uc_rd_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_HPS_SEQ2UC_STATUS
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_hps_seq2uc_status_s
{
  u32 fill_level : 4;
  u32 reserved0 : 28;
};

/* Union definition of register */
union ipa_hwio_def_ipa_hps_seq2uc_status_u
{
  struct ipa_hwio_def_ipa_hps_seq2uc_status_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_HPS_SEQ2UC_CMD
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_hps_seq2uc_cmd_s
{
  u32 pop : 1;
  u32 reserved0 : 31;
};

/* Union definition of register */
union ipa_hwio_def_ipa_hps_seq2uc_cmd_u
{
  struct ipa_hwio_def_ipa_hps_seq2uc_cmd_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_DPS_UC2SEQ_PUSH
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_dps_uc2seq_push_s
{
  u32 src_pipe : 8;
  u32 src_flags : 2;
  u32 src_id : 8;
  u32 ctx_id : 4;
  u32 dest_pipe : 8;
  u32 reserved0 : 1;
  u32 type : 1;
};

/* Union definition of register */
union ipa_hwio_def_ipa_dps_uc2seq_push_u
{
  struct ipa_hwio_def_ipa_dps_uc2seq_push_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_DPS_UC2SEQ_STATUS
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_dps_uc2seq_status_s
{
  u32 fill_level : 4;
  u32 reserved0 : 28;
};

/* Union definition of register */
union ipa_hwio_def_ipa_dps_uc2seq_status_u
{
  struct ipa_hwio_def_ipa_dps_uc2seq_status_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_DPS_SEQ2UC_RD
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_dps_seq2uc_rd_s
{
  u32 src_pipe : 8;
  u32 src_flags : 2;
  u32 src_id : 8;
  u32 ctx_id : 4;
  u32 dest_pipe : 8;
  u32 reserved0 : 1;
  u32 type : 1;
};

/* Union definition of register */
union ipa_hwio_def_ipa_dps_seq2uc_rd_u
{
  struct ipa_hwio_def_ipa_dps_seq2uc_rd_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_DPS_SEQ2UC_STATUS
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_dps_seq2uc_status_s
{
  u32 fill_level : 4;
  u32 reserved0 : 28;
};

/* Union definition of register */
union ipa_hwio_def_ipa_dps_seq2uc_status_u
{
  struct ipa_hwio_def_ipa_dps_seq2uc_status_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_DPS_SEQ2UC_CMD
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_dps_seq2uc_cmd_s
{
  u32 pop : 1;
  u32 reserved0 : 31;
};

/* Union definition of register */
union ipa_hwio_def_ipa_dps_seq2uc_cmd_u
{
  struct ipa_hwio_def_ipa_dps_seq2uc_cmd_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_NTF_TX_CMDQ_CMD
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_ntf_tx_cmdq_cmd_s
{
  u32 write_cmd : 1;
  u32 pop_cmd : 1;
  u32 rd_req : 1;
  u32 reserved0 : 1;
  u32 cmd_client : 8;
  u32 reserved1 : 20;
};

/* Union definition of register */
union ipa_hwio_def_ipa_ntf_tx_cmdq_cmd_u
{
  struct ipa_hwio_def_ipa_ntf_tx_cmdq_cmd_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_NTF_TX_CMDQ_DATA_WR_0
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_ntf_tx_cmdq_data_wr_0_s
{
  u32 cmdq_ctx_id_f : 4;
  u32 cmdq_src_id_f : 8;
  u32 cmdq_src_pipe_f : 8;
  u32 cmdq_opcode_f : 2;
  u32 cmdq_type_f : 1;
  u32 cmdq_virt_cod_f : 1;
  u32 seg_valid_f : 1;
  u32 seg_ctx_id_f : 2;
  u32 reserved0 : 5;
};

/* Union definition of register */
union ipa_hwio_def_ipa_ntf_tx_cmdq_data_wr_0_u
{
  struct ipa_hwio_def_ipa_ntf_tx_cmdq_data_wr_0_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_NTF_TX_CMDQ_DATA_RD_0
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_ntf_tx_cmdq_data_rd_0_s
{
  u32 cmdq_ctx_id_f : 4;
  u32 cmdq_src_id_f : 8;
  u32 cmdq_src_pipe_f : 8;
  u32 cmdq_opcode_f : 2;
  u32 cmdq_type_f : 1;
  u32 cmdq_virt_cod_f : 1;
  u32 seg_valid_f : 1;
  u32 seg_ctx_id_f : 2;
  u32 reserved0 : 5;
};

/* Union definition of register */
union ipa_hwio_def_ipa_ntf_tx_cmdq_data_rd_0_u
{
  struct ipa_hwio_def_ipa_ntf_tx_cmdq_data_rd_0_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_NTF_TX_CMDQ_STATUS
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_ntf_tx_cmdq_status_s
{
  u32 status : 1;
  u32 cmdq_full : 1;
  u32 cmdq_depth : 7;
  u32 reserved0 : 23;
};

/* Union definition of register */
union ipa_hwio_def_ipa_ntf_tx_cmdq_status_u
{
  struct ipa_hwio_def_ipa_ntf_tx_cmdq_status_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_NTF_TX_SNP
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_ntf_tx_snp_s
{
  u32 snp_last : 1;
  u32 snp_write : 1;
  u32 snp_valid : 1;
  u32 snp_next_is_valid : 1;
  u32 snp_next : 8;
  u32 snp_head : 8;
  u32 snp_addr : 8;
  u32 reserved0 : 4;
};

/* Union definition of register */
union ipa_hwio_def_ipa_ntf_tx_snp_u
{
  struct ipa_hwio_def_ipa_ntf_tx_snp_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_NTF_TX_CMDQ_COUNT
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_ntf_tx_cmdq_count_s
{
  u32 fifo_count : 7;
  u32 reserved0 : 25;
};

/* Union definition of register */
union ipa_hwio_def_ipa_ntf_tx_cmdq_count_u
{
  struct ipa_hwio_def_ipa_ntf_tx_cmdq_count_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_PROD_ACKMNGR_CMDQ_CMD
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_prod_ackmngr_cmdq_cmd_s
{
  u32 write_cmd : 1;
  u32 pop_cmd : 1;
  u32 cmd_client : 8;
  u32 rd_req : 1;
  u32 reserved0 : 21;
};

/* Union definition of register */
union ipa_hwio_def_ipa_prod_ackmngr_cmdq_cmd_u
{
  struct ipa_hwio_def_ipa_prod_ackmngr_cmdq_cmd_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_PROD_ACKMNGR_CMDQ_DATA_RD
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_prod_ackmngr_cmdq_data_rd_s
{
  u32 cmdq_src_id : 8;
  u32 cmdq_length : 16;
  u32 cmdq_origin : 1;
  u32 cmdq_sent : 1;
  u32 cmdq_src_id_valid : 1;
  u32 cmdq_userdata : 5;
};

/* Union definition of register */
union ipa_hwio_def_ipa_prod_ackmngr_cmdq_data_rd_u
{
  struct ipa_hwio_def_ipa_prod_ackmngr_cmdq_data_rd_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_PROD_ACKMNGR_CMDQ_DATA_RD_1
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_prod_ackmngr_cmdq_data_rd_1_s
{
  u32 cmdq_fnr_aggr_fc : 1;
  u32 reserved0 : 31;
};

/* Union definition of register */
union ipa_hwio_def_ipa_prod_ackmngr_cmdq_data_rd_1_u
{
  struct ipa_hwio_def_ipa_prod_ackmngr_cmdq_data_rd_1_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_PROD_ACKMNGR_CMDQ_STATUS_EMPTY_n
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_prod_ackmngr_cmdq_status_empty_n_s
{
  u32 cmdq_empty : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_prod_ackmngr_cmdq_status_empty_n_u
{
  struct ipa_hwio_def_ipa_prod_ackmngr_cmdq_status_empty_n_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_PROD_ACKMNGR_CMDQ_STATUS
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_prod_ackmngr_cmdq_status_s
{
  u32 status : 1;
  u32 cmdq_full : 1;
  u32 cmdq_depth : 7;
  u32 reserved0 : 23;
};

/* Union definition of register */
union ipa_hwio_def_ipa_prod_ackmngr_cmdq_status_u
{
  struct ipa_hwio_def_ipa_prod_ackmngr_cmdq_status_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_PROD_ACKMNGR_CMDQ_COUNT
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_prod_ackmngr_cmdq_count_s
{
  u32 fifo_count : 7;
  u32 reserved0 : 25;
};

/* Union definition of register */
union ipa_hwio_def_ipa_prod_ackmngr_cmdq_count_u
{
  struct ipa_hwio_def_ipa_prod_ackmngr_cmdq_count_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_ACKMNGR_SW_ACCESS_ACKINJ_CFG
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_ackmngr_sw_access_ackinj_cfg_s
{
  u32 reserved0 : 5;
  u32 ackinj_src_id_valid : 1;
  u32 ackinj_origin : 1;
  u32 ackinj_sent : 1;
  u32 ackinj_src_id : 8;
  u32 ackinj_length : 16;
};

/* Union definition of register */
union ipa_hwio_def_ipa_ackmngr_sw_access_ackinj_cfg_u
{
  struct ipa_hwio_def_ipa_ackmngr_sw_access_ackinj_cfg_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_ACKMNGR_SW_ACCESS_ACKINJ_PIPE
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_ackmngr_sw_access_ackinj_pipe_s
{
  u32 cons_ackinj_src_pipe : 8;
  u32 prod_ackinj_src_pipe : 8;
  u32 reserved0 : 16;
};

/* Union definition of register */
union ipa_hwio_def_ipa_ackmngr_sw_access_ackinj_pipe_u
{
  struct ipa_hwio_def_ipa_ackmngr_sw_access_ackinj_pipe_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_ACKMNGR_SW_ACCESS_ACKUPD_CFG
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_ackmngr_sw_access_ackupd_cfg_s
{
  u32 ackupd_src_pipe : 8;
  u32 ackupd_src_id : 8;
  u32 ackupd_error : 1;
  u32 reserved0 : 15;
};

/* Union definition of register */
union ipa_hwio_def_ipa_ackmngr_sw_access_ackupd_cfg_u
{
  struct ipa_hwio_def_ipa_ackmngr_sw_access_ackupd_cfg_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_ACKMNGR_SW_ACCESS_CMD
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_ackmngr_sw_access_cmd_s
{
  u32 ackinj_valid : 1;
  u32 ackupd_valid : 1;
  u32 reserved0 : 30;
};

/* Union definition of register */
union ipa_hwio_def_ipa_ackmngr_sw_access_cmd_u
{
  struct ipa_hwio_def_ipa_ackmngr_sw_access_cmd_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_ACKMNGR_SW_ACCESS_STATUS
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_ackmngr_sw_access_status_s
{
  u32 ackinj_ready : 1;
  u32 ackupd_ready : 1;
  u32 reserved0 : 30;
};

/* Union definition of register */
union ipa_hwio_def_ipa_ackmngr_sw_access_status_u
{
  struct ipa_hwio_def_ipa_ackmngr_sw_access_status_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_PROD_ACKMNGR_SW_ACCESS_ACKINJ_CFG
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_prod_ackmngr_sw_access_ackinj_cfg_s
{
  u32 reserved0 : 5;
  u32 ackinj_src_id_valid : 1;
  u32 ackinj_origin : 1;
  u32 ackinj_sent : 1;
  u32 ackinj_src_id : 8;
  u32 ackinj_length : 16;
};

/* Union definition of register */
union ipa_hwio_def_ipa_prod_ackmngr_sw_access_ackinj_cfg_u
{
  struct ipa_hwio_def_ipa_prod_ackmngr_sw_access_ackinj_cfg_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_PROD_ACKMNGR_SW_ACCESS_ACKUPD_CFG
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_prod_ackmngr_sw_access_ackupd_cfg_s
{
  u32 ackupd_src_pipe : 8;
  u32 ackupd_src_id : 8;
  u32 reserved0 : 16;
};

/* Union definition of register */
union ipa_hwio_def_ipa_prod_ackmngr_sw_access_ackupd_cfg_u
{
  struct ipa_hwio_def_ipa_prod_ackmngr_sw_access_ackupd_cfg_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_PROD_ACKMNGR_SW_ACCESS_CMD
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_prod_ackmngr_sw_access_cmd_s
{
  u32 ackinj_valid : 1;
  u32 ackupd_valid : 1;
  u32 reserved0 : 30;
};

/* Union definition of register */
union ipa_hwio_def_ipa_prod_ackmngr_sw_access_cmd_u
{
  struct ipa_hwio_def_ipa_prod_ackmngr_sw_access_cmd_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_PROD_ACKMNGR_SW_ACCESS_STATUS
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_prod_ackmngr_sw_access_status_s
{
  u32 ackinj_ready : 1;
  u32 ackupd_ready : 1;
  u32 reserved0 : 30;
};

/* Union definition of register */
union ipa_hwio_def_ipa_prod_ackmngr_sw_access_status_u
{
  struct ipa_hwio_def_ipa_prod_ackmngr_sw_access_status_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_PROD_ACKMNGR_SW_ACCESS_ACKINJ_CFG1
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_prod_ackmngr_sw_access_ackinj_cfg1_s
{
  u32 ackinj_userdata : 6;
  u32 reserved0 : 26;
};

/* Union definition of register */
union ipa_hwio_def_ipa_prod_ackmngr_sw_access_ackinj_cfg1_u
{
  struct ipa_hwio_def_ipa_prod_ackmngr_sw_access_ackinj_cfg1_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_NTF_TX_CMDQ_RELEASE_WR_n
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_ntf_tx_cmdq_release_wr_n_s
{
  u32 release_wr_cmd : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_ntf_tx_cmdq_release_wr_n_u
{
  struct ipa_hwio_def_ipa_ntf_tx_cmdq_release_wr_n_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_NTF_TX_CMDQ_RELEASE_RD_n
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_ntf_tx_cmdq_release_rd_n_s
{
  u32 release_rd_cmd : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_ntf_tx_cmdq_release_rd_n_u
{
  struct ipa_hwio_def_ipa_ntf_tx_cmdq_release_rd_n_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_NTF_TX_CMDQ_CFG_WR_n
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_ntf_tx_cmdq_cfg_wr_n_s
{
  u32 block_wr : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_ntf_tx_cmdq_cfg_wr_n_u
{
  struct ipa_hwio_def_ipa_ntf_tx_cmdq_cfg_wr_n_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_NTF_TX_CMDQ_CFG_RD_n
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_ntf_tx_cmdq_cfg_rd_n_s
{
  u32 block_rd : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_ntf_tx_cmdq_cfg_rd_n_u
{
  struct ipa_hwio_def_ipa_ntf_tx_cmdq_cfg_rd_n_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_NTF_TX_CMDQ_STATUS_EMPTY_n
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_ntf_tx_cmdq_status_empty_n_s
{
  u32 cmdq_empty : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_ntf_tx_cmdq_status_empty_n_u
{
  struct ipa_hwio_def_ipa_ntf_tx_cmdq_status_empty_n_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_BASE_ADDR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_base_addr_s
{
  u32 zero : 21;
  u32 base : 11;
};

/* Union definition of register */
union ipa_hwio_def_ipa_base_addr_u
{
  struct ipa_hwio_def_ipa_base_addr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_BASE_ADDR_MSB
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_base_addr_msb_s
{
  u32 base_msb : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_base_addr_msb_u
{
  struct ipa_hwio_def_ipa_base_addr_msb_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_ENDP_GSI_CFG1_n
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_endp_gsi_cfg1_n_s
{
  u32 reserved0 : 16;
  u32 endp_en : 1;
  u32 reserved1 : 14;
  u32 init_endp : 1;
};

/* Union definition of register */
union ipa_hwio_def_ipa_endp_gsi_cfg1_n_u
{
  struct ipa_hwio_def_ipa_endp_gsi_cfg1_n_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_GSI_IPA_IF_TLV_OUT_GENERATOR_1
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_gsi_ipa_if_tlv_out_generator_1_s
{
  u32 gen_tlv_out_addr_lsb : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_gsi_ipa_if_tlv_out_generator_1_u
{
  struct ipa_hwio_def_ipa_gsi_ipa_if_tlv_out_generator_1_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_GSI_IPA_IF_TLV_OUT_GENERATOR_2
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_gsi_ipa_if_tlv_out_generator_2_s
{
  u32 gen_tlv_out_addr_msb : 8;
  u32 gen_tlv_out_length : 16;
  u32 gen_tlv_out_routine : 4;
  u32 gen_tlv_out_ee : 4;
};

/* Union definition of register */
union ipa_hwio_def_ipa_gsi_ipa_if_tlv_out_generator_2_u
{
  struct ipa_hwio_def_ipa_gsi_ipa_if_tlv_out_generator_2_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_GSI_IPA_IF_TLV_OUT_GENERATOR_3
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_gsi_ipa_if_tlv_out_generator_3_s
{
  u32 gen_tlv_out_chid : 8;
  u32 gen_tlv_out_type : 4;
  u32 gen_tlv_out_direction : 1;
  u32 gen_tlv_out_top_addr_bit : 1;
  u32 reserved0 : 2;
  u32 gen_tlv_out_chain : 1;
  u32 gen_tlv_out_user_data : 15;
};

/* Union definition of register */
union ipa_hwio_def_ipa_gsi_ipa_if_tlv_out_generator_3_u
{
  struct ipa_hwio_def_ipa_gsi_ipa_if_tlv_out_generator_3_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_GSI_IPA_IF_TLV_OUT_GENERATOR_CTRL
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_gsi_ipa_if_tlv_out_generator_ctrl_s
{
  u32 gen_tlv_out_rdy : 1;
  u32 reserved0 : 3;
  u32 gen_tlv_out_status : 4;
  u32 reserved1 : 8;
  u32 gen_tlv_out_activate : 1;
  u32 reserved2 : 3;
  u32 gen_tlv_out_en : 1;
  u32 reserved3 : 11;
};

/* Union definition of register */
union ipa_hwio_def_ipa_gsi_ipa_if_tlv_out_generator_ctrl_u
{
  struct ipa_hwio_def_ipa_gsi_ipa_if_tlv_out_generator_ctrl_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_GSI_IPA_IF_TLV_IN_RDY
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_gsi_ipa_if_tlv_in_rdy_s
{
  u32 gen_tlv_in_rdy : 1;
  u32 reserved0 : 31;
};

/* Union definition of register */
union ipa_hwio_def_ipa_gsi_ipa_if_tlv_in_rdy_u
{
  struct ipa_hwio_def_ipa_gsi_ipa_if_tlv_in_rdy_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_GSI_IPA_IF_TLV_IN_DATA_1
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_gsi_ipa_if_tlv_in_data_1_s
{
  u32 gen_tlv_in_user_data : 16;
  u32 gen_tlv_in_length : 16;
};

/* Union definition of register */
union ipa_hwio_def_ipa_gsi_ipa_if_tlv_in_data_1_u
{
  struct ipa_hwio_def_ipa_gsi_ipa_if_tlv_in_data_1_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_GSI_IPA_IF_TLV_IN_DATA_2
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_gsi_ipa_if_tlv_in_data_2_s
{
  u32 gen_tlv_in_eot : 1;
  u32 reserved0 : 3;
  u32 gen_tlv_in_ee : 4;
  u32 gen_tlv_in_chid : 8;
  u32 gen_tlv_in_status : 4;
  u32 reserved1 : 8;
  u32 gen_tlv_in_routine : 4;
};

/* Union definition of register */
union ipa_hwio_def_ipa_gsi_ipa_if_tlv_in_data_2_u
{
  struct ipa_hwio_def_ipa_gsi_ipa_if_tlv_in_data_2_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_ENDP_GSI_CFG_TLV_n
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_endp_gsi_cfg_tlv_n_s
{
  u32 fifo_base_addr : 16;
  u32 fifo_size : 8;
  u32 reserved0 : 8;
};

/* Union definition of register */
union ipa_hwio_def_ipa_endp_gsi_cfg_tlv_n_u
{
  struct ipa_hwio_def_ipa_endp_gsi_cfg_tlv_n_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_ENDP_GSI_CFG_AOS_n
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_endp_gsi_cfg_aos_n_s
{
  u32 fifo_base_addr : 16;
  u32 fifo_size : 8;
  u32 reserved0 : 8;
};

/* Union definition of register */
union ipa_hwio_def_ipa_endp_gsi_cfg_aos_n_u
{
  struct ipa_hwio_def_ipa_endp_gsi_cfg_aos_n_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_COAL_VP_AOS_FIFO_n
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_coal_vp_aos_fifo_n_s
{
  u32 fifo_base_addr : 16;
  u32 fifo_size : 8;
  u32 reserved0 : 8;
};

/* Union definition of register */
union ipa_hwio_def_ipa_coal_vp_aos_fifo_n_u
{
  struct ipa_hwio_def_ipa_coal_vp_aos_fifo_n_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_QMB_DEBUG_CTRL
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_qmb_debug_ctrl_s
{
  u32 ram_slaveway_access_protection_disable : 1;
  u32 reserved0 : 31;
};

/* Union definition of register */
union ipa_hwio_def_ipa_qmb_debug_ctrl_u
{
  struct ipa_hwio_def_ipa_qmb_debug_ctrl_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_CTXH_CTRL
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_ctxh_ctrl_s
{
  u32 ctxh_lock_id : 4;
  u32 reserved0 : 25;
  u32 ctxh_wr_block_on_noc_err : 1;
  u32 ctxh_lock_active : 1;
  u32 ctxh_lock : 1;
};

/* Union definition of register */
union ipa_hwio_def_ipa_ctxh_ctrl_u
{
  struct ipa_hwio_def_ipa_ctxh_ctrl_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_CTX_ID_m_CTX_NUM_n
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_ctx_id_m_ctx_num_n_s
{
  u32 ipa_ctxh_data : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_ctx_id_m_ctx_num_n_u
{
  struct ipa_hwio_def_ipa_ctx_id_m_ctx_num_n_s def;
  u32 value;
};

/*----------------------------------------------------------------------------
 * MODULE: IPA_CFG
 *--------------------------------------------------------------------------*/

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_FLAVOR_0
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_flavor_0_s
{
  u32 ipa_pipes : 8;
  u32 ipa_cons_pipes : 8;
  u32 ipa_prod_pipes : 8;
  u32 ipa_prod_lowest : 8;
};

/* Union definition of register */
union ipa_hwio_def_ipa_flavor_0_u
{
  struct ipa_hwio_def_ipa_flavor_0_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_FLAVOR_1
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_flavor_1_s
{
  u32 ctx_n : 6;
  u32 reserved0 : 2;
  u32 mbim_deagg_en : 1;
  u32 ucp_en : 1;
  u32 d_dcph_2_en : 1;
  u32 d_dcph_en : 1;
  u32 h_dcph_en : 1;
  u32 reserved1 : 1;
  u32 filter_router_cache_gen : 1;
  u32 nat_acl_en : 1;
  u32 vmidmt_en : 1;
  u32 uc_en : 1;
  u32 cpr_en : 1;
  u32 dpl_en : 1;
  u32 qmb0_slaveway_en : 1;
  u32 qmb1_slaveway_en : 1;
  u32 qmb1_en : 1;
  u32 dual_tx_en : 1;
  u32 rx_uc_handler_en : 1;
  u32 gsi_slaveway_en : 1;
  u32 pcie_path_en : 1;
  u32 d_dcph_engine_num : 2;
  u32 reserved2 : 3;
};

/* Union definition of register */
union ipa_hwio_def_ipa_flavor_1_u
{
  struct ipa_hwio_def_ipa_flavor_1_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_FLAVOR_2
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_flavor_2_s
{
  u32 qmb0_outst_wr : 6;
  u32 reserved0 : 2;
  u32 qmb0_outst_rd : 6;
  u32 reserved1 : 2;
  u32 qmb1_outst_wr : 6;
  u32 reserved2 : 2;
  u32 qmb1_outst_rd : 6;
  u32 reserved3 : 2;
};

/* Union definition of register */
union ipa_hwio_def_ipa_flavor_2_u
{
  struct ipa_hwio_def_ipa_flavor_2_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_FLAVOR_3
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_flavor_3_s
{
  u32 rsrc_grp_src_num_wout_uc : 4;
  u32 rsrc_grp_src_num_uc : 4;
  u32 rsrc_grp_dst_num_wo_uc_n_drbip : 4;
  u32 rsrc_grp_dst_num_uc : 4;
  u32 pkt_ctx_size : 8;
  u32 rsrc_grp_dst_num_drbip : 4;
  u32 reserved0 : 4;
};

/* Union definition of register */
union ipa_hwio_def_ipa_flavor_3_u
{
  struct ipa_hwio_def_ipa_flavor_3_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_FLAVOR_4
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_flavor_4_s
{
  u32 generic_agg_pipes : 8;
  u32 generic_deagg_pipes : 8;
  u32 bearer_init_ctx_num : 4;
  u32 mbim_agg_pipes : 4;
  u32 reserved0 : 4;
  u32 frag_tables_num : 2;
  u32 reserved1 : 2;
};

/* Union definition of register */
union ipa_hwio_def_ipa_flavor_4_u
{
  struct ipa_hwio_def_ipa_flavor_4_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_FLAVOR_5
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_flavor_5_s
{
  u32 consumer_ack_mngr_db_depth : 6;
  u32 reserved0 : 2;
  u32 producer_ack_mngr_db_depth : 6;
  u32 reserved1 : 2;
  u32 ipa_num_ees : 4;
  u32 gsi_num_ees : 4;
  u32 rx_hps_cmdq_q_depth : 6;
  u32 reserved2 : 2;
};

/* Union definition of register */
union ipa_hwio_def_ipa_flavor_5_u
{
  struct ipa_hwio_def_ipa_flavor_5_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_FLAVOR_6
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_flavor_6_s
{
  u32 hps_dmar_num : 4;
  u32 dps_dmar_num : 4;
  u32 data_descriptor_lists : 6;
  u32 reserved0 : 2;
  u32 data_descriptor_buffers : 8;
  u32 data_sectors : 6;
  u32 reserved1 : 2;
};

/* Union definition of register */
union ipa_hwio_def_ipa_flavor_6_u
{
  struct ipa_hwio_def_ipa_flavor_6_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_FLAVOR_7
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_flavor_7_s
{
  u32 tlv_entry_num : 10;
  u32 reserved0 : 6;
  u32 aos_entry_num : 10;
  u32 coal_vp_num : 4;
  u32 reserved1 : 2;
};

/* Union definition of register */
union ipa_hwio_def_ipa_flavor_7_u
{
  struct ipa_hwio_def_ipa_flavor_7_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_FLAVOR_8
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_flavor_8_s
{
  u32 multi_drbip_dmar_engine_num : 4;
  u32 multi_drbip_dcph_engine_num : 4;
  u32 reserved0 : 24;
};

/* Union definition of register */
union ipa_hwio_def_ipa_flavor_8_u
{
  struct ipa_hwio_def_ipa_flavor_8_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_COMP_HW_VERSION
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_comp_hw_version_s
{
  u32 step : 16;
  u32 minor : 12;
  u32 major : 4;
};

/* Union definition of register */
union ipa_hwio_def_ipa_comp_hw_version_u
{
  struct ipa_hwio_def_ipa_comp_hw_version_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_VERSION
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_version_s
{
  u32 ipa_r_rev : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_version_u
{
  struct ipa_hwio_def_ipa_version_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_COMP_CFG
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_comp_cfg_s
{
  u32 ram_arb_priority_client_samp_fix_disable : 1;
  u32 gsi_snoc_bypass_dis : 1;
  u32 gen_qmb_0_snoc_bypass_dis : 1;
  u32 gen_qmb_1_snoc_bypass_dis : 1;
  u32 reserved0 : 1;
  u32 ipa_qmb_select_by_address_cons_en : 1;
  u32 ipa_qmb_select_by_address_prod_en : 1;
  u32 gsi_multi_inorder_rd_dis : 1;
  u32 gsi_multi_inorder_wr_dis : 1;
  u32 gen_qmb_0_multi_inorder_rd_dis : 1;
  u32 gen_qmb_1_multi_inorder_rd_dis : 1;
  u32 gen_qmb_0_multi_inorder_wr_dis : 1;
  u32 gen_qmb_1_multi_inorder_wr_dis : 1;
  u32 gen_qmb_0_snoc_cnoc_loop_protection_disable : 1;
  u32 gsi_snoc_cnoc_loop_protection_disable : 1;
  u32 gsi_multi_axi_masters_dis : 1;
  u32 ipa_qmb_select_by_address_global_en : 1;
  u32 ipa_full_flush_wait_rsc_closure_en : 1;
  u32 reserved1 : 1;
  u32 qmb_ram_rd_cache_disable : 1;
  u32 genqmb_aooowr : 1;
  u32 gsi_if_out_of_buf_stop_reset_mask_enable : 1;
  u32 ipa_atomic_fetcher_arb_lock_dis : 6;
  u32 reserved2 : 2;
  u32 gen_qmb_1_dynamic_asize : 1;
  u32 gen_qmb_0_dynamic_asize : 1;
};

/* Union definition of register */
union ipa_hwio_def_ipa_comp_cfg_u
{
  struct ipa_hwio_def_ipa_comp_cfg_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_CLKON_CFG_1
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_clkon_cfg_1_s
{
  u32 cgc_open_ipa_core_clk_phase : 1;
  u32 reserved0 : 31;
};

/* Union definition of register */
union ipa_hwio_def_ipa_clkon_cfg_1_u
{
  struct ipa_hwio_def_ipa_clkon_cfg_1_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_CLKON_CFG
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_clkon_cfg_s
{
  u32 cgc_open_rx : 1;
  u32 cgc_open_proc : 1;
  u32 cgc_open_tx_wrapper : 1;
  u32 cgc_open_misc : 1;
  u32 cgc_open_ram_arb : 1;
  u32 cgc_open_ftch_hps : 1;
  u32 cgc_open_ftch_dps : 1;
  u32 cgc_open_hps : 1;
  u32 cgc_open_dps : 1;
  u32 cgc_open_rx_hps_cmdqs : 1;
  u32 cgc_open_hps_dps_cmdqs : 1;
  u32 cgc_open_dps_tx_cmdqs : 1;
  u32 cgc_open_rsrc_mngr : 1;
  u32 cgc_open_ctx_handler : 1;
  u32 cgc_open_ack_mngr : 1;
  u32 cgc_open_d_dcph : 1;
  u32 cgc_open_h_dcph : 1;
  u32 reserved0 : 1;
  u32 cgc_open_ntf_tx_cmdqs : 1;
  u32 cgc_open_tx_0 : 1;
  u32 cgc_open_tx_1 : 1;
  u32 cgc_open_fnr : 1;
  u32 cgc_open_qsb2axi_cmdq_l : 1;
  u32 cgc_open_aggr_wrapper : 1;
  u32 cgc_open_ram_slaveway : 1;
  u32 cgc_open_qmb : 1;
  u32 cgc_open_weight_arb : 1;
  u32 cgc_open_gsi_if : 1;
  u32 cgc_open_global : 1;
  u32 cgc_open_global_2x_clk : 1;
  u32 cgc_open_dpl_fifo : 1;
  u32 cgc_open_drbip : 1;
};

/* Union definition of register */
union ipa_hwio_def_ipa_clkon_cfg_u
{
  struct ipa_hwio_def_ipa_clkon_cfg_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_ROUTE
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_route_s
{
  u32 route_def_pipe : 8;
  u32 route_frag_def_pipe : 8;
  u32 route_def_hdr_ofst : 10;
  u32 route_def_hdr_table : 1;
  u32 route_def_retain_hdr : 1;
  u32 route_dis : 1;
  u32 reserved0 : 3;
};

/* Union definition of register */
union ipa_hwio_def_ipa_route_u
{
  struct ipa_hwio_def_ipa_route_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_MASTER_PRIORITY
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_master_priority_s
{
  u32 qmb_0_rd : 2;
  u32 qmb_1_rd : 2;
  u32 reserved0 : 28;
};

/* Union definition of register */
union ipa_hwio_def_ipa_master_priority_u
{
  struct ipa_hwio_def_ipa_master_priority_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_SHARED_MEM_SIZE
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_shared_mem_size_s
{
  u32 shared_mem_size : 16;
  u32 shared_mem_baddr : 16;
};

/* Union definition of register */
union ipa_hwio_def_ipa_shared_mem_size_u
{
  struct ipa_hwio_def_ipa_shared_mem_size_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_NAT_TIMER
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_nat_timer_s
{
  u32 nat_timer : 24;
  u32 reserved0 : 8;
};

/* Union definition of register */
union ipa_hwio_def_ipa_nat_timer_u
{
  struct ipa_hwio_def_ipa_nat_timer_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_TAG_TIMER
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_tag_timer_s
{
  u32 tag_timer : 24;
  u32 reserved0 : 8;
};

/* Union definition of register */
union ipa_hwio_def_ipa_tag_timer_u
{
  struct ipa_hwio_def_ipa_tag_timer_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_FRAG_RULES_CLR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_frag_rules_clr_s
{
  u32 clr : 1;
  u32 reserved0 : 31;
};

/* Union definition of register */
union ipa_hwio_def_ipa_frag_rules_clr_u
{
  struct ipa_hwio_def_ipa_frag_rules_clr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_PROC_IPH_CFG
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_proc_iph_cfg_s
{
  u32 reserved0 : 8;
  u32 iph_pkt_parser_protocol_stop_enable : 1;
  u32 iph_pkt_parser_protocol_stop_hop : 1;
  u32 iph_pkt_parser_protocol_stop_dest : 1;
  u32 iph_pkt_parser_ihl_to_2nd_frag_en : 1;
  u32 reserved1 : 4;
  u32 iph_pkt_parser_protocol_stop_value : 8;
  u32 d_dcph_multi_engine_disable : 1;
  u32 reserved2 : 7;
};

/* Union definition of register */
union ipa_hwio_def_ipa_proc_iph_cfg_u
{
  struct ipa_hwio_def_ipa_proc_iph_cfg_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_QSB_MAX_WRITES
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_qsb_max_writes_s
{
  u32 gen_qmb_0_max_writes : 4;
  u32 gen_qmb_1_max_writes : 4;
  u32 reserved0 : 24;
};

/* Union definition of register */
union ipa_hwio_def_ipa_qsb_max_writes_u
{
  struct ipa_hwio_def_ipa_qsb_max_writes_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_QSB_MAX_READS
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_qsb_max_reads_s
{
  u32 gen_qmb_0_max_reads : 4;
  u32 gen_qmb_1_max_reads : 4;
  u32 reserved0 : 8;
  u32 gen_qmb_0_max_read_beats : 8;
  u32 gen_qmb_1_max_read_beats : 8;
};

/* Union definition of register */
union ipa_hwio_def_ipa_qsb_max_reads_u
{
  struct ipa_hwio_def_ipa_qsb_max_reads_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_QSB_OUTSTANDING_COUNTER
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_qsb_outstanding_counter_s
{
  u32 gen_qmb_0_reads_cnt : 5;
  u32 reserved0 : 3;
  u32 gen_qmb_1_reads_cnt : 5;
  u32 reserved1 : 3;
  u32 gen_qmb_0_writes_cnt : 5;
  u32 reserved2 : 3;
  u32 gen_qmb_1_writes_cnt : 5;
  u32 reserved3 : 3;
};

/* Union definition of register */
union ipa_hwio_def_ipa_qsb_outstanding_counter_u
{
  struct ipa_hwio_def_ipa_qsb_outstanding_counter_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_QSB_OUTSTANDING_BEATS_COUNTER
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_qsb_outstanding_beats_counter_s
{
  u32 gen_qmb_0_read_beats_cnt : 8;
  u32 gen_qmb_1_read_beats_cnt : 8;
  u32 reserved0 : 16;
};

/* Union definition of register */
union ipa_hwio_def_ipa_qsb_outstanding_beats_counter_u
{
  struct ipa_hwio_def_ipa_qsb_outstanding_beats_counter_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_DPL_TIMER_LSB
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_dpl_timer_lsb_s
{
  u32 tod_lsb : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_dpl_timer_lsb_u
{
  struct ipa_hwio_def_ipa_dpl_timer_lsb_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_DPL_TIMER_MSB
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_dpl_timer_msb_s
{
  u32 tod_msb : 16;
  u32 reserved0 : 11;
  u32 gran_sel : 4;
  u32 timer_en : 1;
};

/* Union definition of register */
union ipa_hwio_def_ipa_dpl_timer_msb_u
{
  struct ipa_hwio_def_ipa_dpl_timer_msb_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_STATE_RX_ACTIVE_n
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_state_rx_active_n_s
{
  u32 endpoints : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_state_rx_active_n_u
{
  struct ipa_hwio_def_ipa_state_rx_active_n_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_STATE_TX_WRAPPER
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_state_tx_wrapper_s
{
  u32 tx0_idle : 1;
  u32 tx1_idle : 1;
  u32 ipa_prod_ackmngr_db_empty : 1;
  u32 ipa_prod_ackmngr_state_idle : 1;
  u32 ipa_prod_bresp_empty : 1;
  u32 reserved0 : 13;
  u32 coal_slave_idle : 1;
  u32 coal_slave_ctx_idle : 1;
  u32 reserved1 : 8;
  u32 coal_slave_open_frame : 4;
};

/* Union definition of register */
union ipa_hwio_def_ipa_state_tx_wrapper_u
{
  struct ipa_hwio_def_ipa_state_tx_wrapper_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_STATE_TX0
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_state_tx0_s
{
  u32 flopped_arbit_type : 3;
  u32 arbit_type : 3;
  u32 pa_idle : 1;
  u32 pa_ctx_idle : 1;
  u32 pa_rst_idle : 1;
  u32 pa_pub_cnt_empty : 1;
  u32 tx_cmd_main_idle : 1;
  u32 tx_cmd_trnseq_idle : 1;
  u32 tx_cmd_snif_idle : 1;
  u32 tx_cmd_bresp_aloc_idle : 1;
  u32 tx_cmd_bresp_inj_idle : 1;
  u32 ar_idle : 1;
  u32 dmaw_idle : 1;
  u32 dmaw_last_outsd_idle : 1;
  u32 pf_idle : 1;
  u32 pf_empty : 1;
  u32 aligner_empty : 1;
  u32 holb_idle : 1;
  u32 holb_mask_idle : 1;
  u32 rsrcrel_idle : 1;
  u32 suspend_empty : 1;
  u32 cs_snif_idle : 1;
  u32 suspend_req_empty : 1;
  u32 reserved0 : 5;
};

/* Union definition of register */
union ipa_hwio_def_ipa_state_tx0_u
{
  struct ipa_hwio_def_ipa_state_tx0_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_STATE_TX1
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_state_tx1_s
{
  u32 flopped_arbit_type : 3;
  u32 arbit_type : 3;
  u32 pa_idle : 1;
  u32 pa_ctx_idle : 1;
  u32 pa_rst_idle : 1;
  u32 pa_pub_cnt_empty : 1;
  u32 tx_cmd_main_idle : 1;
  u32 tx_cmd_trnseq_idle : 1;
  u32 tx_cmd_snif_idle : 1;
  u32 tx_cmd_bresp_aloc_idle : 1;
  u32 tx_cmd_bresp_inj_idle : 1;
  u32 ar_idle : 1;
  u32 dmaw_idle : 1;
  u32 dmaw_last_outsd_idle : 1;
  u32 pf_idle : 1;
  u32 pf_empty : 1;
  u32 aligner_empty : 1;
  u32 holb_idle : 1;
  u32 holb_mask_idle : 1;
  u32 rsrcrel_idle : 1;
  u32 suspend_empty : 1;
  u32 cs_snif_idle : 1;
  u32 suspend_req_empty : 1;
  u32 reserved0 : 5;
};

/* Union definition of register */
union ipa_hwio_def_ipa_state_tx1_u
{
  struct ipa_hwio_def_ipa_state_tx1_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_STATE_TX0_MISC
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_state_tx0_misc_s
{
  u32 ipa_mbim_pkt_fms_idle : 1;
  u32 mbim_direct_dma : 1;
  u32 trnseq_force_valid : 1;
  u32 pkt_drop_cnt_idle : 1;
  u32 nlo_direct_dma : 1;
  u32 coal_direct_dma : 1;
  u32 last_cmd_pipe : 8;
  u32 reserved0 : 18;
};

/* Union definition of register */
union ipa_hwio_def_ipa_state_tx0_misc_u
{
  struct ipa_hwio_def_ipa_state_tx0_misc_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_STATE_TX1_MISC
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_state_tx1_misc_s
{
  u32 ipa_mbim_pkt_fms_idle : 1;
  u32 mbim_direct_dma : 1;
  u32 trnseq_force_valid : 1;
  u32 pkt_drop_cnt_idle : 1;
  u32 nlo_direct_dma : 1;
  u32 coal_direct_dma : 1;
  u32 last_cmd_pipe : 8;
  u32 reserved0 : 18;
};

/* Union definition of register */
union ipa_hwio_def_ipa_state_tx1_misc_u
{
  struct ipa_hwio_def_ipa_state_tx1_misc_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_STATE_FETCHER
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_state_fetcher_s
{
  u32 ipa_hps_ftch_state_idle : 1;
  u32 ipa_hps_ftch_alloc_state_idle : 1;
  u32 ipa_hps_ftch_pkt_state_idle : 1;
  u32 ipa_hps_ftch_imm_state_idle : 1;
  u32 ipa_hps_ftch_cmplt_state_idle : 1;
  u32 ipa_hps_dmar_state_idle : 7;
  u32 ipa_hps_dmar_slot_state_idle : 7;
  u32 ipa_hps_imm_cmd_exec_state_idle : 1;
  u32 reserved0 : 12;
};

/* Union definition of register */
union ipa_hwio_def_ipa_state_fetcher_u
{
  struct ipa_hwio_def_ipa_state_fetcher_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_STATE_FETCHER_MASK_0
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_state_fetcher_mask_0_s
{
  u32 mask_queue_dmar_uses_queue : 8;
  u32 mask_queue_imm_exec : 8;
  u32 mask_queue_no_resources_context : 8;
  u32 mask_queue_no_resources_hps_dmar : 8;
};

/* Union definition of register */
union ipa_hwio_def_ipa_state_fetcher_mask_0_u
{
  struct ipa_hwio_def_ipa_state_fetcher_mask_0_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_STATE_DFETCHER
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_state_dfetcher_s
{
  u32 ipa_dps_ftch_pkt_state_idle : 1;
  u32 ipa_dps_ftch_cmplt_state_idle : 1;
  u32 reserved0 : 2;
  u32 ipa_dps_dmar_state_idle : 7;
  u32 reserved1 : 5;
  u32 ipa_dps_dmar_slot_state_idle : 7;
  u32 reserved2 : 9;
};

/* Union definition of register */
union ipa_hwio_def_ipa_state_dfetcher_u
{
  struct ipa_hwio_def_ipa_state_dfetcher_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_STATE_ACL
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_state_acl_s
{
  u32 ipa_hps_h_dcph_empty : 1;
  u32 ipa_hps_h_dcph_active : 1;
  u32 ipa_hps_pkt_parser_empty : 1;
  u32 ipa_hps_pkt_parser_active : 1;
  u32 ipa_hps_filter_nat_empty : 1;
  u32 ipa_hps_filter_nat_active : 1;
  u32 ipa_hps_router_empty : 1;
  u32 ipa_hps_router_active : 1;
  u32 ipa_hps_hdri_empty : 1;
  u32 ipa_hps_hdri_active : 1;
  u32 ipa_hps_ucp_empty : 1;
  u32 ipa_hps_ucp_active : 1;
  u32 ipa_hps_enqueuer_empty : 1;
  u32 ipa_hps_enqueuer_active : 1;
  u32 ipa_dps_d_dcph_empty : 1;
  u32 ipa_dps_d_dcph_active : 1;
  u32 reserved0 : 2;
  u32 ipa_dps_dispatcher_empty : 1;
  u32 ipa_dps_dispatcher_active : 1;
  u32 ipa_dps_d_dcph_2_empty : 1;
  u32 ipa_dps_d_dcph_2_active : 1;
  u32 ipa_hps_sequencer_idle : 1;
  u32 ipa_dps_sequencer_idle : 1;
  u32 ipa_dps_d_dcph_2nd_empty : 1;
  u32 ipa_dps_d_dcph_2nd_active : 1;
  u32 ipa_hps_coal_master_empty : 1;
  u32 ipa_hps_coal_master_active : 1;
  u32 ipa_hps_multi_drbip_empty : 1;
  u32 ipa_hps_multi_drbip_active : 1;
  u32 reserved1 : 2;
};

/* Union definition of register */
union ipa_hwio_def_ipa_state_acl_u
{
  struct ipa_hwio_def_ipa_state_acl_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_STATE
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_state_s
{
  u32 rx_wait : 1;
  u32 rx_idle : 1;
  u32 tx_idle : 1;
  u32 dpl_fifo_idle : 1;
  u32 bam_gsi_idle : 1;
  u32 ipa_status_sniffer_idle : 1;
  u32 ipa_noc_idle : 1;
  u32 aggr_idle : 1;
  u32 mbim_aggr_idle : 1;
  u32 ipa_rsrc_mngr_db_empty : 1;
  u32 ipa_rsrc_state_idle : 1;
  u32 ipa_ackmngr_db_empty : 1;
  u32 ipa_ackmngr_state_idle : 1;
  u32 ipa_tx_ackq_full : 1;
  u32 ipa_prod_ackmngr_db_empty : 1;
  u32 ipa_prod_ackmngr_state_idle : 1;
  u32 ipa_prod_bresp_idle : 1;
  u32 ipa_full_idle : 1;
  u32 ipa_ntf_tx_empty : 1;
  u32 ipa_tx_ackq_empty : 1;
  u32 ipa_uc_ackq_empty : 1;
  u32 ipa_rx_ackq_empty : 1;
  u32 ipa_tx_commander_cmdq_empty : 1;
  u32 ipa_rx_splt_cmdq_empty : 5;
  u32 ipa_rx_hps_empty : 1;
  u32 ipa_hps_dps_empty : 1;
  u32 ipa_dps_tx_empty : 1;
  u32 ipa_uc_rx_hnd_cmdq_empty : 1;
};

/* Union definition of register */
union ipa_hwio_def_ipa_state_u
{
  struct ipa_hwio_def_ipa_state_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_STATE_GSI_AOS
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_state_gsi_aos_s
{
  u32 ipa_gsi_aos_fsm_idle : 1;
  u32 ipa_gsi_aos_nlo_fsm_idle : 1;
  u32 reserved0 : 30;
};

/* Union definition of register */
union ipa_hwio_def_ipa_state_gsi_aos_u
{
  struct ipa_hwio_def_ipa_state_gsi_aos_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_STATE_GSI_IF
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_state_gsi_if_s
{
  u32 ipa_gsi_prod_fsm_tx_0 : 4;
  u32 ipa_gsi_prod_fsm_tx_1 : 4;
  u32 ipa_gsi_toggle_fsm_idle : 1;
  u32 reserved0 : 7;
  u32 ipa_gsi_skip_fsm : 2;
  u32 reserved1 : 14;
};

/* Union definition of register */
union ipa_hwio_def_ipa_state_gsi_if_u
{
  struct ipa_hwio_def_ipa_state_gsi_if_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_STATE_GSI_IF_CONS
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_state_gsi_if_cons_s
{
  u32 state_idle : 1;
  u32 cache_vld : 7;
  u32 reserved0 : 24;
};

/* Union definition of register */
union ipa_hwio_def_ipa_state_gsi_if_cons_u
{
  struct ipa_hwio_def_ipa_state_gsi_if_cons_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_STATE_FETCHER_MASK_1
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_state_fetcher_mask_1_s
{
  u32 mask_queue_no_resources_ack_entry : 8;
  u32 mask_queue_arb_lock : 8;
  u32 mask_queue_step_mode : 8;
  u32 mask_queue_no_space_dpl_fifo : 8;
};

/* Union definition of register */
union ipa_hwio_def_ipa_state_fetcher_mask_1_u
{
  struct ipa_hwio_def_ipa_state_fetcher_mask_1_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_STATE_FETCHER_MASK_2
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_state_fetcher_mask_2_s
{
  u32 mask_queue_drbip_no_data_sectors : 8;
  u32 mask_queue_drbip_pkt_exceed_max_size : 8;
  u32 reserved0 : 16;
};

/* Union definition of register */
union ipa_hwio_def_ipa_state_fetcher_mask_2_u
{
  struct ipa_hwio_def_ipa_state_fetcher_mask_2_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_STATE_DPL_FIFO
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_state_dpl_fifo_s
{
  u32 pop_fsm_state : 3;
  u32 reserved0 : 29;
};

/* Union definition of register */
union ipa_hwio_def_ipa_state_dpl_fifo_u
{
  struct ipa_hwio_def_ipa_state_dpl_fifo_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_STATE_COAL_MASTER
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_state_coal_master_s
{
  u32 vp_vld : 4;
  u32 main_fsm_state : 4;
  u32 find_open_fsm_state : 4;
  u32 hash_calc_fsm_state : 4;
  u32 check_fit_fsm_state : 4;
  u32 init_vp_fsm_state : 4;
  u32 lru_vp : 4;
  u32 vp_timer_expired : 4;
};

/* Union definition of register */
union ipa_hwio_def_ipa_state_coal_master_u
{
  struct ipa_hwio_def_ipa_state_coal_master_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_STATE_COAL_MASTER_1
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_state_coal_master_1_s
{
  u32 init_vp_wr_ctx_line : 6;
  u32 init_vp_rd_pkt_line : 6;
  u32 init_vp_fsm_state : 4;
  u32 check_fit_rd_ctx_line : 6;
  u32 check_fit_fsm_state : 4;
  u32 arbiter_state : 4;
  u32 reserved0 : 2;
};

/* Union definition of register */
union ipa_hwio_def_ipa_state_coal_master_1_u
{
  struct ipa_hwio_def_ipa_state_coal_master_1_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_STATE_NLO_AGGR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_state_nlo_aggr_s
{
  u32 nlo_aggr_state : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_state_nlo_aggr_u
{
  struct ipa_hwio_def_ipa_state_nlo_aggr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_STATE_CTXH
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_state_ctxh_s
{
  u32 ipa_ctxh_rd_idle : 1;
  u32 ipa_ctxh_wr_idle : 1;
  u32 reserved0 : 30;
};

/* Union definition of register */
union ipa_hwio_def_ipa_state_ctxh_u
{
  struct ipa_hwio_def_ipa_state_ctxh_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_STATE_UC_QMB
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_state_uc_qmb_s
{
  u32 ctrl_fsm_state_queue_0 : 2;
  u32 ot_table_empty_queue_0 : 1;
  u32 ot_table_full_queue_0 : 1;
  u32 comp_fifo_empty_queue_0 : 1;
  u32 comp_fifo_full_queue_0 : 1;
  u32 cmd_fifo_empty_queue_0 : 1;
  u32 cmd_fifo_full_queue_0 : 1;
  u32 queue_0_idle : 1;
  u32 reserved0 : 7;
  u32 ctrl_fsm_state_queue_1 : 2;
  u32 ot_table_empty_queue_1 : 1;
  u32 ot_table_full_queue_1 : 1;
  u32 comp_fifo_empty_queue_1 : 1;
  u32 comp_fifo_full_queue_1 : 1;
  u32 cmd_fifo_empty_queue_1 : 1;
  u32 cmd_fifo_full_queue_1 : 1;
  u32 queue_1_idle : 1;
  u32 reserved1 : 7;
};

/* Union definition of register */
union ipa_hwio_def_ipa_state_uc_qmb_u
{
  struct ipa_hwio_def_ipa_state_uc_qmb_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_STATE_DRBIP
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_state_drbip_s
{
  u32 drbip_dmar_idle : 3;
  u32 reserved0 : 5;
  u32 drbip_dcph_idle : 1;
  u32 reserved1 : 7;
  u32 drbip_pkt_idle : 4;
  u32 reserved2 : 12;
};

/* Union definition of register */
union ipa_hwio_def_ipa_state_drbip_u
{
  struct ipa_hwio_def_ipa_state_drbip_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_STATE_AGGR_ACTIVE_n
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_state_aggr_active_n_s
{
  u32 endpoints : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_state_aggr_active_n_u
{
  struct ipa_hwio_def_ipa_state_aggr_active_n_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_STATE_GSI_TLV_FIFO_EMPTY_n
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_state_gsi_tlv_fifo_empty_n_s
{
  u32 pipe_fifo_empty : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_state_gsi_tlv_fifo_empty_n_u
{
  struct ipa_hwio_def_ipa_state_gsi_tlv_fifo_empty_n_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_STATE_GSI_AOS_FIFO_EMPTY_n
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_state_gsi_aos_fifo_empty_n_s
{
  u32 pipe_fifo_empty : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_state_gsi_aos_fifo_empty_n_u
{
  struct ipa_hwio_def_ipa_state_gsi_aos_fifo_empty_n_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_STATE_DRBIP_DROP_STATE_n
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_state_drbip_drop_state_n_s
{
  u32 consumer_pipe_drop_state : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_state_drbip_drop_state_n_u
{
  struct ipa_hwio_def_ipa_state_drbip_drop_state_n_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_STATE_DFETCHER_MASK_0_n
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_state_dfetcher_mask_0_n_s
{
  u32 mask_queue_dst_grp_dmar_outstanding : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_state_dfetcher_mask_0_n_u
{
  struct ipa_hwio_def_ipa_state_dfetcher_mask_0_n_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_STATE_DFETCHER_MASK_1_n
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_state_dfetcher_mask_1_n_s
{
  u32 mask_queue_no_resources_data_sectors : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_state_dfetcher_mask_1_n_u
{
  struct ipa_hwio_def_ipa_state_dfetcher_mask_1_n_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_STATE_DFETCHER_MASK_2_n
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_state_dfetcher_mask_2_n_s
{
  u32 mask_queue_no_resources_dps_dmar : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_state_dfetcher_mask_2_n_u
{
  struct ipa_hwio_def_ipa_state_dfetcher_mask_2_n_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_STATE_DFETCHER_MASK_3_n
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_state_dfetcher_mask_3_n_s
{
  u32 mask_queue_no_resources_seg_ctx : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_state_dfetcher_mask_3_n_u
{
  struct ipa_hwio_def_ipa_state_dfetcher_mask_3_n_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_BAM_ACTIVATED_PORTS_n
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_bam_activated_ports_n_s
{
  u32 endpoints : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_bam_activated_ports_n_u
{
  struct ipa_hwio_def_ipa_bam_activated_ports_n_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_YELLOW_MARKER_BELOW_n
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_yellow_marker_below_n_s
{
  u32 endpoints : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_yellow_marker_below_n_u
{
  struct ipa_hwio_def_ipa_yellow_marker_below_n_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_YELLOW_MARKER_BELOW_EN_n
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_yellow_marker_below_en_n_s
{
  u32 endpoints : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_yellow_marker_below_en_n_u
{
  struct ipa_hwio_def_ipa_yellow_marker_below_en_n_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_YELLOW_MARKER_BELOW_CLR_n
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_yellow_marker_below_clr_n_s
{
  u32 endpoints : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_yellow_marker_below_clr_n_u
{
  struct ipa_hwio_def_ipa_yellow_marker_below_clr_n_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_RED_MARKER_BELOW_n
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_red_marker_below_n_s
{
  u32 endpoints : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_red_marker_below_n_u
{
  struct ipa_hwio_def_ipa_red_marker_below_n_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_RED_MARKER_BELOW_EN_n
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_red_marker_below_en_n_s
{
  u32 endpoints : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_red_marker_below_en_n_u
{
  struct ipa_hwio_def_ipa_red_marker_below_en_n_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_RED_MARKER_BELOW_CLR_n
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_red_marker_below_clr_n_s
{
  u32 endpoints : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_red_marker_below_clr_n_u
{
  struct ipa_hwio_def_ipa_red_marker_below_clr_n_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_YELLOW_MARKER_SHADOW_n
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_yellow_marker_shadow_n_s
{
  u32 endpoints : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_yellow_marker_shadow_n_u
{
  struct ipa_hwio_def_ipa_yellow_marker_shadow_n_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_RED_MARKER_SHADOW_n
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_red_marker_shadow_n_s
{
  u32 endpoints : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_red_marker_shadow_n_u
{
  struct ipa_hwio_def_ipa_red_marker_shadow_n_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_YELLOW_MARKER_ABOVE_n
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_yellow_marker_above_n_s
{
  u32 endpoints : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_yellow_marker_above_n_u
{
  struct ipa_hwio_def_ipa_yellow_marker_above_n_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_YELLOW_MARKER_ABOVE_EN_n
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_yellow_marker_above_en_n_s
{
  u32 endpoints : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_yellow_marker_above_en_n_u
{
  struct ipa_hwio_def_ipa_yellow_marker_above_en_n_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_YELLOW_MARKER_ABOVE_CLR_n
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_yellow_marker_above_clr_n_s
{
  u32 endpoints : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_yellow_marker_above_clr_n_u
{
  struct ipa_hwio_def_ipa_yellow_marker_above_clr_n_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_RED_MARKER_ABOVE_n
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_red_marker_above_n_s
{
  u32 endpoints : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_red_marker_above_n_u
{
  struct ipa_hwio_def_ipa_red_marker_above_n_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_RED_MARKER_ABOVE_EN_n
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_red_marker_above_en_n_s
{
  u32 endpoints : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_red_marker_above_en_n_u
{
  struct ipa_hwio_def_ipa_red_marker_above_en_n_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_RED_MARKER_ABOVE_CLR_n
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_red_marker_above_clr_n_s
{
  u32 endpoints : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_red_marker_above_clr_n_u
{
  struct ipa_hwio_def_ipa_red_marker_above_clr_n_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_FILT_ROUT_CACHE_CFG
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_filt_rout_cache_cfg_s
{
  u32 ipa_router_cache_en : 1;
  u32 reserved0 : 3;
  u32 ipa_filter_cache_en : 1;
  u32 reserved1 : 3;
  u32 cache_low_priority_hashable_hit_disable : 1;
  u32 reserved2 : 7;
  u32 cache_lru_eviction_threshold : 16;
};

/* Union definition of register */
union ipa_hwio_def_ipa_filt_rout_cache_cfg_u
{
  struct ipa_hwio_def_ipa_filt_rout_cache_cfg_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_FILT_ROUT_CACHE_REDUCE_CFG
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_filt_rout_cache_reduce_cfg_s
{
  u32 ipa_router_cache_reduce_en : 1;
  u32 reserved0 : 3;
  u32 ipa_filter_cache_reduce_en : 1;
  u32 reserved1 : 3;
  u32 ipa_router_cache_reduce_level : 8;
  u32 ipa_filter_cache_reduce_level : 8;
  u32 reserved2 : 8;
};

/* Union definition of register */
union ipa_hwio_def_ipa_filt_rout_cache_reduce_cfg_u
{
  struct ipa_hwio_def_ipa_filt_rout_cache_reduce_cfg_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_FILT_ROUT_CACHE_FLUSH
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_filt_rout_cache_flush_s
{
  u32 ipa_router_cache_flush : 1;
  u32 reserved0 : 3;
  u32 ipa_filter_cache_flush : 1;
  u32 reserved1 : 27;
};

/* Union definition of register */
union ipa_hwio_def_ipa_filt_rout_cache_flush_u
{
  struct ipa_hwio_def_ipa_filt_rout_cache_flush_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_FILT_ROUT_CFG
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_filt_rout_cfg_s
{
  u32 router_prefetch_en : 1;
  u32 reserved0 : 3;
  u32 filter_prefetch_en : 1;
  u32 reserved1 : 3;
  u32 filt_rout_data_cache_en : 1;
  u32 reserved2 : 23;
};

/* Union definition of register */
union ipa_hwio_def_ipa_filt_rout_cfg_u
{
  struct ipa_hwio_def_ipa_filt_rout_cfg_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_IPV4_FILTER_INIT_VALUES
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_ipv4_filter_init_values_s
{
  u32 ip_v4_filter_init_hashed_addr : 16;
  u32 ip_v4_filter_init_non_hashed_addr : 16;
};

/* Union definition of register */
union ipa_hwio_def_ipa_ipv4_filter_init_values_u
{
  struct ipa_hwio_def_ipa_ipv4_filter_init_values_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_IPV6_FILTER_INIT_VALUES
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_ipv6_filter_init_values_s
{
  u32 ip_v6_filter_init_hashed_addr : 16;
  u32 ip_v6_filter_init_non_hashed_addr : 16;
};

/* Union definition of register */
union ipa_hwio_def_ipa_ipv6_filter_init_values_u
{
  struct ipa_hwio_def_ipa_ipv6_filter_init_values_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_IPV4_NAT_INIT_VALUES_0
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_ipv4_nat_init_values_0_s
{
  u32 ip_v4_nat_init_rules_addr : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_ipv4_nat_init_values_0_u
{
  struct ipa_hwio_def_ipa_ipv4_nat_init_values_0_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_IPV4_NAT_INIT_VALUES_0_MSB
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_ipv4_nat_init_values_0_msb_s
{
  u32 ip_v4_nat_init_rules_addr : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_ipv4_nat_init_values_0_msb_u
{
  struct ipa_hwio_def_ipa_ipv4_nat_init_values_0_msb_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_IPV4_NAT_INIT_VALUES_1
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_ipv4_nat_init_values_1_s
{
  u32 ip_v4_nat_init_exp_rules_addr : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_ipv4_nat_init_values_1_u
{
  struct ipa_hwio_def_ipa_ipv4_nat_init_values_1_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_IPV4_NAT_INIT_VALUES_1_MSB
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_ipv4_nat_init_values_1_msb_s
{
  u32 ip_v4_nat_init_exp_rules_addr : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_ipv4_nat_init_values_1_msb_u
{
  struct ipa_hwio_def_ipa_ipv4_nat_init_values_1_msb_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_IPV4_NAT_INIT_VALUES_2
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_ipv4_nat_init_values_2_s
{
  u32 ip_v4_nat_init_index_table_addr : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_ipv4_nat_init_values_2_u
{
  struct ipa_hwio_def_ipa_ipv4_nat_init_values_2_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_IPV4_NAT_INIT_VALUES_2_MSB
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_ipv4_nat_init_values_2_msb_s
{
  u32 ip_v4_nat_init_index_table_addr : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_ipv4_nat_init_values_2_msb_u
{
  struct ipa_hwio_def_ipa_ipv4_nat_init_values_2_msb_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_IPV4_NAT_INIT_VALUES_3
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_ipv4_nat_init_values_3_s
{
  u32 ip_v4_nat_init_index_table_exp_addr : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_ipv4_nat_init_values_3_u
{
  struct ipa_hwio_def_ipa_ipv4_nat_init_values_3_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_IPV4_NAT_INIT_VALUES_3_MSB
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_ipv4_nat_init_values_3_msb_s
{
  u32 ip_v4_nat_init_index_table_exp_addr : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_ipv4_nat_init_values_3_msb_u
{
  struct ipa_hwio_def_ipa_ipv4_nat_init_values_3_msb_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_IPV4_NAT_INIT_VALUES_4
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_ipv4_nat_init_values_4_s
{
  u32 ip_v4_nat_init_table_index : 3;
  u32 reserved0 : 1;
  u32 ip_v4_nat_init_rules_addr_type : 1;
  u32 ip_v4_nat_init_exp_rules_addr_type : 1;
  u32 ip_v4_nat_init_index_table_addr_type : 1;
  u32 ip_v4_nat_init_index_table_exp_addr_type : 1;
  u32 ip_v4_nat_init_size_base_tables : 12;
  u32 ip_v4_nat_init_size_exp_tables : 10;
  u32 reserved1 : 2;
};

/* Union definition of register */
union ipa_hwio_def_ipa_ipv4_nat_init_values_4_u
{
  struct ipa_hwio_def_ipa_ipv4_nat_init_values_4_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_IPV4_NAT_INIT_VALUES_5
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_ipv4_nat_init_values_5_s
{
  u32 ip_v4_nat_init_pdn_config_table_addr : 20;
  u32 reserved0 : 12;
};

/* Union definition of register */
union ipa_hwio_def_ipa_ipv4_nat_init_values_5_u
{
  struct ipa_hwio_def_ipa_ipv4_nat_init_values_5_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_IPV4_ROUTE_INIT_VALUES
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_ipv4_route_init_values_s
{
  u32 ip_v4_route_init_hashed_addr : 16;
  u32 ip_v4_route_init_non_hashed_addr : 16;
};

/* Union definition of register */
union ipa_hwio_def_ipa_ipv4_route_init_values_u
{
  struct ipa_hwio_def_ipa_ipv4_route_init_values_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_IPV6_ROUTE_INIT_VALUES
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_ipv6_route_init_values_s
{
  u32 ip_v6_route_init_hashed_addr : 16;
  u32 ip_v6_route_init_non_hashed_addr : 16;
};

/* Union definition of register */
union ipa_hwio_def_ipa_ipv6_route_init_values_u
{
  struct ipa_hwio_def_ipa_ipv6_route_init_values_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_IPV6_CONN_TRACK_INIT_VALUES_0
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_ipv6_conn_track_init_values_0_s
{
  u32 ip_v6_conn_track_init_table_addr : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_ipv6_conn_track_init_values_0_u
{
  struct ipa_hwio_def_ipa_ipv6_conn_track_init_values_0_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_IPV6_CONN_TRACK_INIT_VALUES_0_MSB
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_ipv6_conn_track_init_values_0_msb_s
{
  u32 ip_v6_conn_track_init_table_addr : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_ipv6_conn_track_init_values_0_msb_u
{
  struct ipa_hwio_def_ipa_ipv6_conn_track_init_values_0_msb_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_IPV6_CONN_TRACK_INIT_VALUES_1
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_ipv6_conn_track_init_values_1_s
{
  u32 ip_v6_conn_track_init_exp_table_addr : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_ipv6_conn_track_init_values_1_u
{
  struct ipa_hwio_def_ipa_ipv6_conn_track_init_values_1_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_IPV6_CONN_TRACK_INIT_VALUES_1_MSB
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_ipv6_conn_track_init_values_1_msb_s
{
  u32 ip_v6_conn_track_init_exp_table_addr : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_ipv6_conn_track_init_values_1_msb_u
{
  struct ipa_hwio_def_ipa_ipv6_conn_track_init_values_1_msb_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_IPV6_CONN_TRACK_INIT_VALUES_2
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_ipv6_conn_track_init_values_2_s
{
  u32 ip_v6_conn_track_init_table_index : 3;
  u32 reserved0 : 1;
  u32 ip_v6_conn_track_init_table_addr_type : 1;
  u32 ip_v6_conn_track_init_exp_table_addr_type : 1;
  u32 reserved1 : 2;
  u32 ip_v6_conn_track_init_size_base_tables : 12;
  u32 ip_v6_conn_track_init_size_exp_tables : 10;
  u32 reserved2 : 2;
};

/* Union definition of register */
union ipa_hwio_def_ipa_ipv6_conn_track_init_values_2_u
{
  struct ipa_hwio_def_ipa_ipv6_conn_track_init_values_2_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_HDR_INIT_LOCAL_VALUES
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_hdr_init_local_values_s
{
  u32 reserved0 : 12;
  u32 hdr_init_local_hdr_addr : 16;
  u32 reserved1 : 4;
};

/* Union definition of register */
union ipa_hwio_def_ipa_hdr_init_local_values_u
{
  struct ipa_hwio_def_ipa_hdr_init_local_values_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_HDR_INIT_SYSTEM_VALUES
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_hdr_init_system_values_s
{
  u32 hdr_init_system_hdr_table_addr : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_hdr_init_system_values_u
{
  struct ipa_hwio_def_ipa_hdr_init_system_values_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_HDR_INIT_SYSTEM_VALUES_MSB
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_hdr_init_system_values_msb_s
{
  u32 hdr_init_system_hdr_table_addr : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_hdr_init_system_values_msb_u
{
  struct ipa_hwio_def_ipa_hdr_init_system_values_msb_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_IMM_CMD_ACCESS_PIPE_VALUES
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_imm_cmd_access_pipe_values_s
{
  u32 imm_cmd_filter_router_pipe : 8;
  u32 imm_cmd_nat_pipe : 8;
  u32 imm_cmd_conn_track_pipe : 8;
  u32 imm_cmd_hdri_pipe : 8;
};

/* Union definition of register */
union ipa_hwio_def_ipa_imm_cmd_access_pipe_values_u
{
  struct ipa_hwio_def_ipa_imm_cmd_access_pipe_values_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_IMM_CMD_ACCESS_PIPE_VALUES_1
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_imm_cmd_access_pipe_values_1_s
{
  u32 imm_cmd_gen_pipe : 8;
  u32 reserved0 : 24;
};

/* Union definition of register */
union ipa_hwio_def_ipa_imm_cmd_access_pipe_values_1_u
{
  struct ipa_hwio_def_ipa_imm_cmd_access_pipe_values_1_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_FRAG_VALUES
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_frag_values_s
{
  u32 ipa_frag_ram_last_addr : 16;
  u32 reserved0 : 8;
  u32 ipa_frag_fairness_cnt : 4;
  u32 reserved1 : 4;
};

/* Union definition of register */
union ipa_hwio_def_ipa_frag_values_u
{
  struct ipa_hwio_def_ipa_frag_values_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_SYS_PKT_PROC_CNTXT_BASE
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_sys_pkt_proc_cntxt_base_s
{
  u32 zero : 3;
  u32 addr : 29;
};

/* Union definition of register */
union ipa_hwio_def_ipa_sys_pkt_proc_cntxt_base_u
{
  struct ipa_hwio_def_ipa_sys_pkt_proc_cntxt_base_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_SYS_PKT_PROC_CNTXT_BASE_MSB
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_sys_pkt_proc_cntxt_base_msb_s
{
  u32 addr : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_sys_pkt_proc_cntxt_base_msb_u
{
  struct ipa_hwio_def_ipa_sys_pkt_proc_cntxt_base_msb_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_LOCAL_PKT_PROC_CNTXT_BASE
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_local_pkt_proc_cntxt_base_s
{
  u32 zero : 3;
  u32 addr : 15;
  u32 reserved0 : 14;
};

/* Union definition of register */
union ipa_hwio_def_ipa_local_pkt_proc_cntxt_base_u
{
  struct ipa_hwio_def_ipa_local_pkt_proc_cntxt_base_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_SCND_FRAG_VALUES
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_scnd_frag_values_s
{
  u32 ipa_scnd_frag_ram_last_addr : 16;
  u32 reserved0 : 8;
  u32 ipa_scnd_frag_fairness_cnt : 4;
  u32 reserved1 : 4;
};

/* Union definition of register */
union ipa_hwio_def_ipa_scnd_frag_values_u
{
  struct ipa_hwio_def_ipa_scnd_frag_values_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_AOS_CFG
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_aos_cfg_s
{
  u32 ipa_aos_tx_rx_priority : 1;
  u32 reserved0 : 31;
};

/* Union definition of register */
union ipa_hwio_def_ipa_aos_cfg_u
{
  struct ipa_hwio_def_ipa_aos_cfg_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_TX_CFG
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_tx_cfg_s
{
  u32 reserved0 : 2;
  u32 prefetch_almost_empty_size_tx0 : 4;
  u32 dmaw_scnd_outsd_pred_threshold : 4;
  u32 dmaw_scnd_outsd_pred_en : 1;
  u32 dmaw_max_beats_256_dis : 1;
  u32 pa_mask_en : 1;
  u32 prefetch_almost_empty_size_tx1 : 4;
  u32 dual_tx_enable : 1;
  u32 sspnd_pa_no_start_state : 1;
  u32 reserved1 : 1;
  u32 holb_sticky_drop_en : 1;
  u32 reserved2 : 11;
};

/* Union definition of register */
union ipa_hwio_def_ipa_tx_cfg_u
{
  struct ipa_hwio_def_ipa_tx_cfg_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_NAT_UC_EXTERNAL_CFG
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_nat_uc_external_cfg_s
{
  u32 ipa_nat_uc_external_table_addr_lsb : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_nat_uc_external_cfg_u
{
  struct ipa_hwio_def_ipa_nat_uc_external_cfg_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_NAT_UC_LOCAL_CFG
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_nat_uc_local_cfg_s
{
  u32 ipa_nat_uc_local_table_addr_lsb : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_nat_uc_local_cfg_u
{
  struct ipa_hwio_def_ipa_nat_uc_local_cfg_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_NAT_UC_SHARED_CFG
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_nat_uc_shared_cfg_s
{
  u32 ipa_nat_uc_external_table_addr_msb : 16;
  u32 ipa_nat_uc_local_table_addr_msb : 16;
};

/* Union definition of register */
union ipa_hwio_def_ipa_nat_uc_shared_cfg_u
{
  struct ipa_hwio_def_ipa_nat_uc_shared_cfg_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_RAM_INTLV_CFG
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_ram_intlv_cfg_s
{
  u32 ipa_ram_intlv_cfg : 16;
  u32 reserved0 : 16;
};

/* Union definition of register */
union ipa_hwio_def_ipa_ram_intlv_cfg_u
{
  struct ipa_hwio_def_ipa_ram_intlv_cfg_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_CONN_TRACK_UC_EXTERNAL_CFG
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_conn_track_uc_external_cfg_s
{
  u32 ipa_conn_track_uc_external_table_addr_lsb : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_conn_track_uc_external_cfg_u
{
  struct ipa_hwio_def_ipa_conn_track_uc_external_cfg_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_CONN_TRACK_UC_LOCAL_CFG
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_conn_track_uc_local_cfg_s
{
  u32 ipa_conn_track_uc_local_table_addr_lsb : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_conn_track_uc_local_cfg_u
{
  struct ipa_hwio_def_ipa_conn_track_uc_local_cfg_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_CONN_TRACK_UC_SHARED_CFG
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_conn_track_uc_shared_cfg_s
{
  u32 ipa_conn_track_uc_external_table_addr_msb : 16;
  u32 ipa_conn_track_uc_local_table_addr_msb : 16;
};

/* Union definition of register */
union ipa_hwio_def_ipa_conn_track_uc_shared_cfg_u
{
  struct ipa_hwio_def_ipa_conn_track_uc_shared_cfg_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_IDLE_INDICATION_CFG
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_idle_indication_cfg_s
{
  u32 enter_idle_debounce_thresh : 16;
  u32 idle_indication_enable : 1;
  u32 reserved0 : 15;
};

/* Union definition of register */
union ipa_hwio_def_ipa_idle_indication_cfg_u
{
  struct ipa_hwio_def_ipa_idle_indication_cfg_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_QTIME_TIMESTAMP_CFG
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_qtime_timestamp_cfg_s
{
  u32 dpl_timestamp_lsb : 5;
  u32 reserved0 : 2;
  u32 dpl_timestamp_sel : 1;
  u32 tag_timestamp_lsb : 5;
  u32 reserved1 : 3;
  u32 nat_timestamp_lsb : 5;
  u32 reserved2 : 11;
};

/* Union definition of register */
union ipa_hwio_def_ipa_qtime_timestamp_cfg_u
{
  struct ipa_hwio_def_ipa_qtime_timestamp_cfg_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_TIMERS_XO_CLK_DIV_CFG
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_timers_xo_clk_div_cfg_s
{
  u32 value : 9;
  u32 reserved0 : 22;
  u32 enable : 1;
};

/* Union definition of register */
union ipa_hwio_def_ipa_timers_xo_clk_div_cfg_u
{
  struct ipa_hwio_def_ipa_timers_xo_clk_div_cfg_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_TIMERS_PULSE_GRAN_CFG
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_timers_pulse_gran_cfg_s
{
  u32 gran_0 : 3;
  u32 gran_1 : 3;
  u32 gran_2 : 3;
  u32 gran_3 : 3;
  u32 reserved0 : 20;
};

/* Union definition of register */
union ipa_hwio_def_ipa_timers_pulse_gran_cfg_u
{
  struct ipa_hwio_def_ipa_timers_pulse_gran_cfg_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_QTIME_SMP
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_qtime_smp_s
{
  u32 pulse : 1;
  u32 reserved0 : 31;
};

/* Union definition of register */
union ipa_hwio_def_ipa_qtime_smp_u
{
  struct ipa_hwio_def_ipa_qtime_smp_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_QTIME_LSB
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_qtime_lsb_s
{
  u32 value : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_qtime_lsb_u
{
  struct ipa_hwio_def_ipa_qtime_lsb_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_QTIME_MSB
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_qtime_msb_s
{
  u32 value : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_qtime_msb_u
{
  struct ipa_hwio_def_ipa_qtime_msb_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_SRC_RSRC_AMOUNT_REDUCE_EN
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_src_rsrc_amount_reduce_en_s
{
  u32 ipa_src_rsrc_amount_reduce_en : 8;
  u32 reserved0 : 24;
};

/* Union definition of register */
union ipa_hwio_def_ipa_src_rsrc_amount_reduce_en_u
{
  struct ipa_hwio_def_ipa_src_rsrc_amount_reduce_en_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_SRC_RSRC_AMOUNT_REDUCE_VALUES_0
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_src_rsrc_amount_reduce_values_0_s
{
  u32 ipa_src_rsrc_amount_reduce_value_rsrc_type_0 : 6;
  u32 reserved0 : 2;
  u32 ipa_src_rsrc_amount_reduce_value_rsrc_type_1 : 6;
  u32 reserved1 : 2;
  u32 ipa_src_rsrc_amount_reduce_value_rsrc_type_2 : 6;
  u32 reserved2 : 2;
  u32 ipa_src_rsrc_amount_reduce_value_rsrc_type_3 : 6;
  u32 reserved3 : 2;
};

/* Union definition of register */
union ipa_hwio_def_ipa_src_rsrc_amount_reduce_values_0_u
{
  struct ipa_hwio_def_ipa_src_rsrc_amount_reduce_values_0_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_SRC_RSRC_AMOUNT_REDUCE_VALUES_1
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_src_rsrc_amount_reduce_values_1_s
{
  u32 ipa_src_rsrc_amount_reduce_value_rsrc_type_4 : 6;
  u32 reserved0 : 26;
};

/* Union definition of register */
union ipa_hwio_def_ipa_src_rsrc_amount_reduce_values_1_u
{
  struct ipa_hwio_def_ipa_src_rsrc_amount_reduce_values_1_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_DST_RSRC_AMOUNT_REDUCE_EN
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_dst_rsrc_amount_reduce_en_s
{
  u32 ipa_dst_rsrc_amount_reduce_en : 4;
  u32 reserved0 : 28;
};

/* Union definition of register */
union ipa_hwio_def_ipa_dst_rsrc_amount_reduce_en_u
{
  struct ipa_hwio_def_ipa_dst_rsrc_amount_reduce_en_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_DST_RSRC_AMOUNT_REDUCE_VALUES_0
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_dst_rsrc_amount_reduce_values_0_s
{
  u32 ipa_dst_rsrc_amount_reduce_value_rsrc_type_0 : 6;
  u32 reserved0 : 2;
  u32 ipa_dst_rsrc_amount_reduce_value_rsrc_type_1 : 6;
  u32 reserved1 : 18;
};

/* Union definition of register */
union ipa_hwio_def_ipa_dst_rsrc_amount_reduce_values_0_u
{
  struct ipa_hwio_def_ipa_dst_rsrc_amount_reduce_values_0_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_ATOMIC_LOCK_CFG
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_atomic_lock_cfg_s
{
  u32 groups_to_mask : 6;
  u32 reserved0 : 26;
};

/* Union definition of register */
union ipa_hwio_def_ipa_atomic_lock_cfg_u
{
  struct ipa_hwio_def_ipa_atomic_lock_cfg_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_GENERIC_RAM_ARBITER_PRIORITY
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_generic_ram_arbiter_priority_s
{
  u32 rd_priority_valid : 1;
  u32 wr_priority_valid : 1;
  u32 reserved0 : 2;
  u32 rd_priority_index : 8;
  u32 wr_priority_index : 8;
  u32 reserved1 : 12;
};

/* Union definition of register */
union ipa_hwio_def_ipa_generic_ram_arbiter_priority_u
{
  struct ipa_hwio_def_ipa_generic_ram_arbiter_priority_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_SRC_RSRC_GRP_01_RSRC_TYPE_n
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_src_rsrc_grp_01_rsrc_type_n_s
{
  u32 src_rsrc_grp_0_min_limit : 6;
  u32 reserved0 : 2;
  u32 src_rsrc_grp_0_max_limit : 6;
  u32 reserved1 : 2;
  u32 src_rsrc_grp_1_min_limit : 6;
  u32 reserved2 : 2;
  u32 src_rsrc_grp_1_max_limit : 6;
  u32 reserved3 : 2;
};

/* Union definition of register */
union ipa_hwio_def_ipa_src_rsrc_grp_01_rsrc_type_n_u
{
  struct ipa_hwio_def_ipa_src_rsrc_grp_01_rsrc_type_n_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_SRC_RSRC_GRP_23_RSRC_TYPE_n
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_src_rsrc_grp_23_rsrc_type_n_s
{
  u32 src_rsrc_grp_2_min_limit : 6;
  u32 reserved0 : 2;
  u32 src_rsrc_grp_2_max_limit : 6;
  u32 reserved1 : 2;
  u32 src_rsrc_grp_3_min_limit : 6;
  u32 reserved2 : 2;
  u32 src_rsrc_grp_3_max_limit : 6;
  u32 reserved3 : 2;
};

/* Union definition of register */
union ipa_hwio_def_ipa_src_rsrc_grp_23_rsrc_type_n_u
{
  struct ipa_hwio_def_ipa_src_rsrc_grp_23_rsrc_type_n_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_SRC_RSRC_GRP_45_RSRC_TYPE_n
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_src_rsrc_grp_45_rsrc_type_n_s
{
  u32 src_rsrc_grp_4_min_limit : 6;
  u32 reserved0 : 2;
  u32 src_rsrc_grp_4_max_limit : 6;
  u32 reserved1 : 2;
  u32 src_rsrc_grp_5_min_limit : 6;
  u32 reserved2 : 2;
  u32 src_rsrc_grp_5_max_limit : 6;
  u32 reserved3 : 2;
};

/* Union definition of register */
union ipa_hwio_def_ipa_src_rsrc_grp_45_rsrc_type_n_u
{
  struct ipa_hwio_def_ipa_src_rsrc_grp_45_rsrc_type_n_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_SRC_RSRC_GRP_67_RSRC_TYPE_n
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_src_rsrc_grp_67_rsrc_type_n_s
{
  u32 src_rsrc_grp_6_min_limit : 6;
  u32 reserved0 : 2;
  u32 src_rsrc_grp_6_max_limit : 6;
  u32 reserved1 : 2;
  u32 src_rsrc_grp_7_min_limit : 6;
  u32 reserved2 : 2;
  u32 src_rsrc_grp_7_max_limit : 6;
  u32 reserved3 : 2;
};

/* Union definition of register */
union ipa_hwio_def_ipa_src_rsrc_grp_67_rsrc_type_n_u
{
  struct ipa_hwio_def_ipa_src_rsrc_grp_67_rsrc_type_n_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_SRC_RSRC_GRP_0123_RSRC_TYPE_CNT_n
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_src_rsrc_grp_0123_rsrc_type_cnt_n_s
{
  u32 src_rsrc_grp_0_cnt : 6;
  u32 reserved0 : 2;
  u32 src_rsrc_grp_1_cnt : 6;
  u32 reserved1 : 2;
  u32 src_rsrc_grp_2_cnt : 6;
  u32 reserved2 : 2;
  u32 src_rsrc_grp_3_cnt : 6;
  u32 reserved3 : 2;
};

/* Union definition of register */
union ipa_hwio_def_ipa_src_rsrc_grp_0123_rsrc_type_cnt_n_u
{
  struct ipa_hwio_def_ipa_src_rsrc_grp_0123_rsrc_type_cnt_n_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_SRC_RSRC_GRP_4567_RSRC_TYPE_CNT_n
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_src_rsrc_grp_4567_rsrc_type_cnt_n_s
{
  u32 src_rsrc_grp_4_cnt : 6;
  u32 reserved0 : 2;
  u32 src_rsrc_grp_5_cnt : 6;
  u32 reserved1 : 18;
};

/* Union definition of register */
union ipa_hwio_def_ipa_src_rsrc_grp_4567_rsrc_type_cnt_n_u
{
  struct ipa_hwio_def_ipa_src_rsrc_grp_4567_rsrc_type_cnt_n_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_SRC_RSRC_TYPE_AMOUNT_n
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_src_rsrc_type_amount_n_s
{
  u32 src_rsrc_type_amount : 6;
  u32 reserved0 : 26;
};

/* Union definition of register */
union ipa_hwio_def_ipa_src_rsrc_type_amount_n_u
{
  struct ipa_hwio_def_ipa_src_rsrc_type_amount_n_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_DST_RSRC_GRP_01_RSRC_TYPE_n
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_dst_rsrc_grp_01_rsrc_type_n_s
{
  u32 dst_rsrc_grp_0_min_limit : 6;
  u32 reserved0 : 2;
  u32 dst_rsrc_grp_0_max_limit : 6;
  u32 reserved1 : 2;
  u32 dst_rsrc_grp_1_min_limit : 6;
  u32 reserved2 : 2;
  u32 dst_rsrc_grp_1_max_limit : 6;
  u32 reserved3 : 2;
};

/* Union definition of register */
union ipa_hwio_def_ipa_dst_rsrc_grp_01_rsrc_type_n_u
{
  struct ipa_hwio_def_ipa_dst_rsrc_grp_01_rsrc_type_n_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_DST_RSRC_GRP_23_RSRC_TYPE_n
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_dst_rsrc_grp_23_rsrc_type_n_s
{
  u32 dst_rsrc_grp_2_min_limit : 6;
  u32 reserved0 : 2;
  u32 dst_rsrc_grp_2_max_limit : 6;
  u32 reserved1 : 2;
  u32 dst_rsrc_grp_3_min_limit : 6;
  u32 reserved2 : 2;
  u32 dst_rsrc_grp_3_max_limit : 6;
  u32 reserved3 : 2;
};

/* Union definition of register */
union ipa_hwio_def_ipa_dst_rsrc_grp_23_rsrc_type_n_u
{
  struct ipa_hwio_def_ipa_dst_rsrc_grp_23_rsrc_type_n_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_DST_RSRC_GRP_45_RSRC_TYPE_n
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_dst_rsrc_grp_45_rsrc_type_n_s
{
  u32 dst_rsrc_grp_4_min_limit : 6;
  u32 reserved0 : 2;
  u32 dst_rsrc_grp_4_max_limit : 6;
  u32 reserved1 : 2;
  u32 dst_rsrc_grp_5_min_limit : 6;
  u32 reserved2 : 2;
  u32 dst_rsrc_grp_5_max_limit : 6;
  u32 reserved3 : 2;
};

/* Union definition of register */
union ipa_hwio_def_ipa_dst_rsrc_grp_45_rsrc_type_n_u
{
  struct ipa_hwio_def_ipa_dst_rsrc_grp_45_rsrc_type_n_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_DST_RSRC_GRP_67_RSRC_TYPE_n
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_dst_rsrc_grp_67_rsrc_type_n_s
{
  u32 dst_rsrc_grp_6_min_limit : 6;
  u32 reserved0 : 2;
  u32 dst_rsrc_grp_6_max_limit : 6;
  u32 reserved1 : 18;
};

/* Union definition of register */
union ipa_hwio_def_ipa_dst_rsrc_grp_67_rsrc_type_n_u
{
  struct ipa_hwio_def_ipa_dst_rsrc_grp_67_rsrc_type_n_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_DST_RSRC_GRP_0123_RSRC_TYPE_CNT_n
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_dst_rsrc_grp_0123_rsrc_type_cnt_n_s
{
  u32 dst_rsrc_grp_0_cnt : 6;
  u32 reserved0 : 2;
  u32 dst_rsrc_grp_1_cnt : 6;
  u32 reserved1 : 2;
  u32 dst_rsrc_grp_2_cnt : 6;
  u32 reserved2 : 2;
  u32 dst_rsrc_grp_3_cnt : 6;
  u32 reserved3 : 2;
};

/* Union definition of register */
union ipa_hwio_def_ipa_dst_rsrc_grp_0123_rsrc_type_cnt_n_u
{
  struct ipa_hwio_def_ipa_dst_rsrc_grp_0123_rsrc_type_cnt_n_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_DST_RSRC_GRP_4567_RSRC_TYPE_CNT_n
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_dst_rsrc_grp_4567_rsrc_type_cnt_n_s
{
  u32 dst_rsrc_grp_4_cnt : 8;
  u32 dst_rsrc_grp_5_cnt : 8;
  u32 dst_rsrc_grp_6_cnt : 8;
  u32 reserved0 : 8;
};

/* Union definition of register */
union ipa_hwio_def_ipa_dst_rsrc_grp_4567_rsrc_type_cnt_n_u
{
  struct ipa_hwio_def_ipa_dst_rsrc_grp_4567_rsrc_type_cnt_n_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_DST_RSRC_TYPE_AMOUNT_n
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_dst_rsrc_type_amount_n_s
{
  u32 dst_rsrc_type_amount : 6;
  u32 reserved0 : 26;
};

/* Union definition of register */
union ipa_hwio_def_ipa_dst_rsrc_type_amount_n_u
{
  struct ipa_hwio_def_ipa_dst_rsrc_type_amount_n_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_RX_CFG
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_rx_cfg_s
{
  u32 cmdq_split_not_wait_data_desc_prior_hdr_push : 1;
  u32 rx_cmdq_splitter_cmdq_pending_mux_disable : 1;
  u32 reserved0 : 30;
};

/* Union definition of register */
union ipa_hwio_def_ipa_rx_cfg_u
{
  struct ipa_hwio_def_ipa_rx_cfg_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_RSRC_GRP_CFG
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_rsrc_grp_cfg_s
{
  u32 src_grp_special_valid : 1;
  u32 reserved0 : 3;
  u32 src_grp_special_index : 3;
  u32 reserved1 : 1;
  u32 dst_pipe_special_valid : 1;
  u32 reserved2 : 3;
  u32 dst_pipe_special_index : 8;
  u32 dst_grp_special_valid : 1;
  u32 reserved3 : 3;
  u32 dst_grp_special_index : 6;
  u32 reserved4 : 2;
};

/* Union definition of register */
union ipa_hwio_def_ipa_rsrc_grp_cfg_u
{
  struct ipa_hwio_def_ipa_rsrc_grp_cfg_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_RSRC_GRP_CFG_EXT
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_rsrc_grp_cfg_ext_s
{
  u32 src_grp_2nd_priority_special_valid : 1;
  u32 reserved0 : 3;
  u32 src_grp_2nd_priority_special_index : 3;
  u32 reserved1 : 25;
};

/* Union definition of register */
union ipa_hwio_def_ipa_rsrc_grp_cfg_ext_u
{
  struct ipa_hwio_def_ipa_rsrc_grp_cfg_ext_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_AXI_CFG
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_axi_cfg_s
{
  u32 relaxed_ordering_gsi_rd : 1;
  u32 relaxed_ordering_gsi_wr : 1;
  u32 relaxed_ordering_ipa_rd : 1;
  u32 relaxed_ordering_ipa_wr : 1;
  u32 reserved0 : 28;
};

/* Union definition of register */
union ipa_hwio_def_ipa_axi_cfg_u
{
  struct ipa_hwio_def_ipa_axi_cfg_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_AGGR_FORCE_CLOSE_n
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_aggr_force_close_n_s
{
  u32 aggr_force_close_pipe_bitmap : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_aggr_force_close_n_u
{
  struct ipa_hwio_def_ipa_aggr_force_close_n_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_STAT_QUOTA_BASE_n
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_stat_quota_base_n_s
{
  u32 base_addr_offset : 3;
  u32 base_addr : 16;
  u32 reserved0 : 13;
};

/* Union definition of register */
union ipa_hwio_def_ipa_stat_quota_base_n_u
{
  struct ipa_hwio_def_ipa_stat_quota_base_n_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_STAT_TETHERING_BASE_n
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_stat_tethering_base_n_s
{
  u32 base_addr_offset : 3;
  u32 base_addr : 16;
  u32 reserved0 : 13;
};

/* Union definition of register */
union ipa_hwio_def_ipa_stat_tethering_base_n_u
{
  struct ipa_hwio_def_ipa_stat_tethering_base_n_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_STAT_DROP_CNT_BASE_n
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_stat_drop_cnt_base_n_s
{
  u32 base_addr_offset : 3;
  u32 base_addr : 16;
  u32 reserved0 : 13;
};

/* Union definition of register */
union ipa_hwio_def_ipa_stat_drop_cnt_base_n_u
{
  struct ipa_hwio_def_ipa_stat_drop_cnt_base_n_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_STAT_FILTER_IPV4_BASE
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_stat_filter_ipv4_base_s
{
  u32 base_addr_offset : 3;
  u32 base_addr : 16;
  u32 reserved0 : 13;
};

/* Union definition of register */
union ipa_hwio_def_ipa_stat_filter_ipv4_base_u
{
  struct ipa_hwio_def_ipa_stat_filter_ipv4_base_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_STAT_FILTER_IPV6_BASE
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_stat_filter_ipv6_base_s
{
  u32 base_addr_offset : 3;
  u32 base_addr : 16;
  u32 reserved0 : 13;
};

/* Union definition of register */
union ipa_hwio_def_ipa_stat_filter_ipv6_base_u
{
  struct ipa_hwio_def_ipa_stat_filter_ipv6_base_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_STAT_ROUTER_IPV4_BASE
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_stat_router_ipv4_base_s
{
  u32 base_addr_offset : 3;
  u32 base_addr : 16;
  u32 reserved0 : 13;
};

/* Union definition of register */
union ipa_hwio_def_ipa_stat_router_ipv4_base_u
{
  struct ipa_hwio_def_ipa_stat_router_ipv4_base_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_STAT_ROUTER_IPV6_BASE
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_stat_router_ipv6_base_s
{
  u32 base_addr_offset : 3;
  u32 base_addr : 16;
  u32 reserved0 : 13;
};

/* Union definition of register */
union ipa_hwio_def_ipa_stat_router_ipv6_base_u
{
  struct ipa_hwio_def_ipa_stat_router_ipv6_base_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_STAT_QUOTA_MASK_EE_n_REG_k
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_stat_quota_mask_ee_n_reg_k_s
{
  u32 pipe_mask : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_stat_quota_mask_ee_n_reg_k_u
{
  struct ipa_hwio_def_ipa_stat_quota_mask_ee_n_reg_k_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_STAT_TETHERING_MASK_EE_n_REG_k
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_stat_tethering_mask_ee_n_reg_k_s
{
  u32 pipe_mask : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_stat_tethering_mask_ee_n_reg_k_u
{
  struct ipa_hwio_def_ipa_stat_tethering_mask_ee_n_reg_k_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_STAT_DROP_CNT_MASK_EE_n_REG_k
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_stat_drop_cnt_mask_ee_n_reg_k_s
{
  u32 pipe_mask : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_stat_drop_cnt_mask_ee_n_reg_k_u
{
  struct ipa_hwio_def_ipa_stat_drop_cnt_mask_ee_n_reg_k_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_NLO_PP_CFG1
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_nlo_pp_cfg1_s
{
  u32 nlo_ack_pp : 8;
  u32 nlo_data_pp : 8;
  u32 nlo_status_pp : 8;
  u32 nlo_ack_max_vp : 8;
};

/* Union definition of register */
union ipa_hwio_def_ipa_nlo_pp_cfg1_u
{
  struct ipa_hwio_def_ipa_nlo_pp_cfg1_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_NLO_PP_CFG2
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_nlo_pp_cfg2_s
{
  u32 nlo_ack_close_padd : 8;
  u32 nlo_data_close_padd : 8;
  u32 nlo_ack_buffer_mode : 1;
  u32 nlo_data_buffer_mode : 1;
  u32 nlo_status_buffer_mode : 1;
  u32 reserved0 : 13;
};

/* Union definition of register */
union ipa_hwio_def_ipa_nlo_pp_cfg2_u
{
  struct ipa_hwio_def_ipa_nlo_pp_cfg2_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_NLO_MIN_DSM_CFG
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_nlo_min_dsm_cfg_s
{
  u32 nlo_ack_min_dsm_len : 16;
  u32 nlo_data_min_dsm_len : 16;
};

/* Union definition of register */
union ipa_hwio_def_ipa_nlo_min_dsm_cfg_u
{
  struct ipa_hwio_def_ipa_nlo_min_dsm_cfg_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_NLO_VP_AGGR_CFG_LSB_n
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_nlo_vp_aggr_cfg_lsb_n_s
{
  u32 vp_pkt_limit : 6;
  u32 vp_time_limit : 5;
  u32 vp_byte_limit : 6;
  u32 vp_hard_byte_limit_en : 1;
  u32 vp_aggr_gran_sel : 1;
  u32 reserved0 : 13;
};

/* Union definition of register */
union ipa_hwio_def_ipa_nlo_vp_aggr_cfg_lsb_n_u
{
  struct ipa_hwio_def_ipa_nlo_vp_aggr_cfg_lsb_n_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_NLO_VP_LIMIT_CFG_n
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_nlo_vp_limit_cfg_n_s
{
  u32 lower_size : 16;
  u32 upper_size : 16;
};

/* Union definition of register */
union ipa_hwio_def_ipa_nlo_vp_limit_cfg_n_u
{
  struct ipa_hwio_def_ipa_nlo_vp_limit_cfg_n_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_NLO_VP_FLUSH_REQ
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_nlo_vp_flush_req_s
{
  u32 vp_flush_pp_indx : 8;
  u32 reserved0 : 8;
  u32 vp_flush_vp_indx : 8;
  u32 reserved1 : 7;
  u32 vp_flush_req : 1;
};

/* Union definition of register */
union ipa_hwio_def_ipa_nlo_vp_flush_req_u
{
  struct ipa_hwio_def_ipa_nlo_vp_flush_req_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_NLO_VP_FLUSH_COOKIE
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_nlo_vp_flush_cookie_s
{
  u32 vp_flush_cookie : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_nlo_vp_flush_cookie_u
{
  struct ipa_hwio_def_ipa_nlo_vp_flush_cookie_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_NLO_VP_FLUSH_ACK
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_nlo_vp_flush_ack_s
{
  u32 vp_flush_ack : 1;
  u32 reserved0 : 31;
};

/* Union definition of register */
union ipa_hwio_def_ipa_nlo_vp_flush_ack_u
{
  struct ipa_hwio_def_ipa_nlo_vp_flush_ack_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_NLO_VP_DSM_OPEN
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_nlo_vp_dsm_open_s
{
  u32 vp_dsm_open : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_nlo_vp_dsm_open_u
{
  struct ipa_hwio_def_ipa_nlo_vp_dsm_open_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_NLO_VP_QBAP_OPEN
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_nlo_vp_qbap_open_s
{
  u32 vp_qbap_open : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_nlo_vp_qbap_open_u
{
  struct ipa_hwio_def_ipa_nlo_vp_qbap_open_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_COAL_MASTER_CFG
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_coal_master_cfg_s
{
  u32 coal_force_to_default : 1;
  u32 coal_enhanced_ipv4_id_en : 1;
  u32 reserved0 : 30;
};

/* Union definition of register */
union ipa_hwio_def_ipa_coal_master_cfg_u
{
  struct ipa_hwio_def_ipa_coal_master_cfg_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_COAL_EVICT_LRU
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_coal_evict_lru_s
{
  u32 coal_eviction_en : 1;
  u32 coal_vp_lru_thrshld : 5;
  u32 reserved0 : 26;
};

/* Union definition of register */
union ipa_hwio_def_ipa_coal_evict_lru_u
{
  struct ipa_hwio_def_ipa_coal_evict_lru_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_COAL_QMAP_CFG
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_coal_qmap_cfg_s
{
  u32 mux_id_byte_sel : 2;
  u32 reserved0 : 30;
};

/* Union definition of register */
union ipa_hwio_def_ipa_coal_qmap_cfg_u
{
  struct ipa_hwio_def_ipa_coal_qmap_cfg_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_SNIFFER_QMB_SEL
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_sniffer_qmb_sel_s
{
  u32 snif_qmb_sel : 1;
  u32 reserved0 : 31;
};

/* Union definition of register */
union ipa_hwio_def_ipa_sniffer_qmb_sel_u
{
  struct ipa_hwio_def_ipa_sniffer_qmb_sel_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_ULSO_CFG_IP_ID_MAX_VALUE_n
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_ulso_cfg_ip_id_max_value_n_s
{
  u32 ip_id_max_value : 16;
  u32 reserved0 : 16;
};

/* Union definition of register */
union ipa_hwio_def_ipa_ulso_cfg_ip_id_max_value_n_u
{
  struct ipa_hwio_def_ipa_ulso_cfg_ip_id_max_value_n_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_ULSO_CFG_IP_ID_MIN_VALUE_n
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_ulso_cfg_ip_id_min_value_n_s
{
  u32 ip_id_min_value : 16;
  u32 reserved0 : 16;
};

/* Union definition of register */
union ipa_hwio_def_ipa_ulso_cfg_ip_id_min_value_n_u
{
  struct ipa_hwio_def_ipa_ulso_cfg_ip_id_min_value_n_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_ENDP_INIT_CTRL_n
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_endp_init_ctrl_n_s
{
  u32 reserved0 : 1;
  u32 endp_delay : 1;
  u32 reserved1 : 30;
};

/* Union definition of register */
union ipa_hwio_def_ipa_endp_init_ctrl_n_u
{
  struct ipa_hwio_def_ipa_endp_init_ctrl_n_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_ENDP_INIT_CTRL_SCND_n
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_endp_init_ctrl_scnd_n_s
{
  u32 reserved0 : 1;
  u32 endp_delay : 1;
  u32 reserved1 : 30;
};

/* Union definition of register */
union ipa_hwio_def_ipa_endp_init_ctrl_scnd_n_u
{
  struct ipa_hwio_def_ipa_endp_init_ctrl_scnd_n_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_ENDP_INIT_CFG_n
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_endp_init_cfg_n_s
{
  u32 frag_offload_en : 1;
  u32 cs_offload_en : 2;
  u32 cs_metadata_hdr_offset : 4;
  u32 reserved0 : 1;
  u32 gen_qmb_master_sel : 1;
  u32 reserved1 : 23;
};

/* Union definition of register */
union ipa_hwio_def_ipa_endp_init_cfg_n_u
{
  struct ipa_hwio_def_ipa_endp_init_cfg_n_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_ENDP_INIT_NAT_n
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_endp_init_nat_n_s
{
  u32 nat_en : 2;
  u32 reserved0 : 30;
};

/* Union definition of register */
union ipa_hwio_def_ipa_endp_init_nat_n_u
{
  struct ipa_hwio_def_ipa_endp_init_nat_n_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_ENDP_INIT_HDR_n
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_endp_init_hdr_n_s
{
  u32 hdr_len : 6;
  u32 hdr_ofst_metadata_valid : 1;
  u32 hdr_ofst_metadata : 6;
  u32 hdr_additional_const_len : 6;
  u32 hdr_ofst_pkt_size_valid : 1;
  u32 hdr_ofst_pkt_size : 6;
  u32 reserved0 : 1;
  u32 hdr_len_inc_deagg_hdr : 1;
  u32 hdr_len_msb : 2;
  u32 hdr_ofst_metadata_msb : 2;
};

/* Union definition of register */
union ipa_hwio_def_ipa_endp_init_hdr_n_u
{
  struct ipa_hwio_def_ipa_endp_init_hdr_n_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_ENDP_INIT_HDR_EXT_n
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_endp_init_hdr_ext_n_s
{
  u32 hdr_endianess : 1;
  u32 hdr_total_len_or_pad_valid : 1;
  u32 hdr_total_len_or_pad : 1;
  u32 hdr_payload_len_inc_padding : 1;
  u32 hdr_total_len_or_pad_offset : 6;
  u32 hdr_pad_to_alignment : 4;
  u32 reserved0 : 2;
  u32 hdr_total_len_or_pad_offset_msb : 2;
  u32 hdr_ofst_pkt_size_msb : 2;
  u32 hdr_additional_const_len_msb : 2;
  u32 hdr_bytes_to_remove_valid : 1;
  u32 reserved1 : 1;
  u32 hdr_bytes_to_remove : 8;
};

/* Union definition of register */
union ipa_hwio_def_ipa_endp_init_hdr_ext_n_u
{
  struct ipa_hwio_def_ipa_endp_init_hdr_ext_n_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_ENDP_INIT_HDR_METADATA_MASK_n
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_endp_init_hdr_metadata_mask_n_s
{
  u32 metadata_mask : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_endp_init_hdr_metadata_mask_n_u
{
  struct ipa_hwio_def_ipa_endp_init_hdr_metadata_mask_n_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_ENDP_INIT_HDR_METADATA_n
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_endp_init_hdr_metadata_n_s
{
  u32 metadata : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_endp_init_hdr_metadata_n_u
{
  struct ipa_hwio_def_ipa_endp_init_hdr_metadata_n_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_ENDP_INIT_MODE_n
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_endp_init_mode_n_s
{
  u32 mode : 3;
  u32 bearer_cntx_enable : 1;
  u32 dest_pipe_index : 8;
  u32 byte_threshold : 16;
  u32 pipe_replicate_en : 1;
  u32 pad_en : 1;
  u32 drbip_acl_enable : 1;
  u32 reserved0 : 1;
};

/* Union definition of register */
union ipa_hwio_def_ipa_endp_init_mode_n_u
{
  struct ipa_hwio_def_ipa_endp_init_mode_n_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_ENDP_INIT_AGGR_n
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_endp_init_aggr_n_s
{
  u32 aggr_en : 2;
  u32 aggr_type : 3;
  u32 aggr_byte_limit : 6;
  u32 reserved0 : 1;
  u32 aggr_time_limit : 5;
  u32 aggr_pkt_limit : 6;
  u32 aggr_sw_eof_active : 1;
  u32 aggr_force_close : 1;
  u32 reserved1 : 1;
  u32 aggr_hard_byte_limit_enable : 1;
  u32 aggr_gran_sel : 1;
  u32 reserved2 : 4;
};

/* Union definition of register */
union ipa_hwio_def_ipa_endp_init_aggr_n_u
{
  struct ipa_hwio_def_ipa_endp_init_aggr_n_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_ENDP_INIT_HOL_BLOCK_EN_n
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_endp_init_hol_block_en_n_s
{
  u32 en : 1;
  u32 reserved0 : 31;
};

/* Union definition of register */
union ipa_hwio_def_ipa_endp_init_hol_block_en_n_u
{
  struct ipa_hwio_def_ipa_endp_init_hol_block_en_n_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_ENDP_INIT_HOL_BLOCK_TIMER_n
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_endp_init_hol_block_timer_n_s
{
  u32 time_limit : 5;
  u32 reserved0 : 3;
  u32 gran_sel : 2;
  u32 reserved1 : 22;
};

/* Union definition of register */
union ipa_hwio_def_ipa_endp_init_hol_block_timer_n_u
{
  struct ipa_hwio_def_ipa_endp_init_hol_block_timer_n_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_ENDP_INIT_DEAGGR_n
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_endp_init_deaggr_n_s
{
  u32 deaggr_hdr_len : 6;
  u32 syspipe_err_detection : 1;
  u32 packet_offset_valid : 1;
  u32 packet_offset_location : 6;
  u32 ignore_min_pkt_err : 1;
  u32 reserved0 : 1;
  u32 max_packet_len : 16;
};

/* Union definition of register */
union ipa_hwio_def_ipa_endp_init_deaggr_n_u
{
  struct ipa_hwio_def_ipa_endp_init_deaggr_n_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_ENDP_INIT_RSRC_GRP_n
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_endp_init_rsrc_grp_n_s
{
  u32 rsrc_grp : 3;
  u32 reserved0 : 29;
};

/* Union definition of register */
union ipa_hwio_def_ipa_endp_init_rsrc_grp_n_u
{
  struct ipa_hwio_def_ipa_endp_init_rsrc_grp_n_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_ENDP_INIT_SEQ_n
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_endp_init_seq_n_s
{
  u32 hps_seq_type : 5;
  u32 reserved0 : 3;
  u32 dps_seq_type : 5;
  u32 reserved1 : 19;
};

/* Union definition of register */
union ipa_hwio_def_ipa_endp_init_seq_n_u
{
  struct ipa_hwio_def_ipa_endp_init_seq_n_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_ENDP_STATUS_n
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_endp_status_n_s
{
  u32 status_en : 1;
  u32 status_endp : 8;
  u32 status_pkt_supress : 1;
  u32 reserved0 : 22;
};

/* Union definition of register */
union ipa_hwio_def_ipa_endp_status_n_u
{
  struct ipa_hwio_def_ipa_endp_status_n_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_ENDP_SRC_ID_WRITE_n
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_endp_src_id_write_n_s
{
  u32 src_id_write_value : 8;
  u32 reserved0 : 24;
};

/* Union definition of register */
union ipa_hwio_def_ipa_endp_src_id_write_n_u
{
  struct ipa_hwio_def_ipa_endp_src_id_write_n_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_ENDP_SRC_ID_READ_n
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_endp_src_id_read_n_s
{
  u32 src_id_read_value : 8;
  u32 reserved0 : 24;
};

/* Union definition of register */
union ipa_hwio_def_ipa_endp_src_id_read_n_u
{
  struct ipa_hwio_def_ipa_endp_src_id_read_n_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_ENDP_INIT_CONN_TRACK_n
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_endp_init_conn_track_n_s
{
  u32 conn_track_en : 1;
  u32 reserved0 : 31;
};

/* Union definition of register */
union ipa_hwio_def_ipa_endp_init_conn_track_n_u
{
  struct ipa_hwio_def_ipa_endp_init_conn_track_n_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_ENDP_INIT_DRBIP_CFG_n
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_endp_init_drbip_cfg_n_s
{
  u32 data_sectors_for_imm_cmd : 6;
  u32 reserved0 : 26;
};

/* Union definition of register */
union ipa_hwio_def_ipa_endp_init_drbip_cfg_n_u
{
  struct ipa_hwio_def_ipa_endp_init_drbip_cfg_n_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_FILTER_CACHE_CFG_n
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_filter_cache_cfg_n_s
{
  u32 filter_cache_msk_src_id : 1;
  u32 filter_cache_msk_src_ip_add : 1;
  u32 filter_cache_msk_dst_ip_add : 1;
  u32 filter_cache_msk_src_port : 1;
  u32 filter_cache_msk_dst_port : 1;
  u32 filter_cache_msk_protocol : 1;
  u32 filter_cache_msk_metadata : 1;
  u32 reserved0 : 25;
};

/* Union definition of register */
union ipa_hwio_def_ipa_filter_cache_cfg_n_u
{
  struct ipa_hwio_def_ipa_filter_cache_cfg_n_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_ROUTER_CACHE_CFG_n
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_router_cache_cfg_n_s
{
  u32 router_cache_msk_src_id : 1;
  u32 router_cache_msk_src_ip_add : 1;
  u32 router_cache_msk_dst_ip_add : 1;
  u32 router_cache_msk_src_port : 1;
  u32 router_cache_msk_dst_port : 1;
  u32 router_cache_msk_protocol : 1;
  u32 router_cache_msk_metadata : 1;
  u32 reserved0 : 25;
};

/* Union definition of register */
union ipa_hwio_def_ipa_router_cache_cfg_n_u
{
  struct ipa_hwio_def_ipa_router_cache_cfg_n_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_ENDP_YELLOW_RED_MARKER_CFG_n
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_endp_yellow_red_marker_cfg_n_s
{
  u32 reserved0 : 10;
  u32 ipa_yellow_marker_cfg : 6;
  u32 reserved1 : 10;
  u32 ipa_red_marker_cfg : 6;
};

/* Union definition of register */
union ipa_hwio_def_ipa_endp_yellow_red_marker_cfg_n_u
{
  struct ipa_hwio_def_ipa_endp_yellow_red_marker_cfg_n_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_ENDP_INIT_CTRL_STATUS_n
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_endp_init_ctrl_status_n_s
{
  u32 endp_suspend_status : 1;
  u32 endp_delay_status : 1;
  u32 reserved0 : 30;
};

/* Union definition of register */
union ipa_hwio_def_ipa_endp_init_ctrl_status_n_u
{
  struct ipa_hwio_def_ipa_endp_init_ctrl_status_n_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_ENDP_INIT_PROD_CFG_n
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_endp_init_prod_cfg_n_s
{
  u32 tx_sel : 1;
  u32 reserved0 : 31;
};

/* Union definition of register */
union ipa_hwio_def_ipa_endp_init_prod_cfg_n_u
{
  struct ipa_hwio_def_ipa_endp_init_prod_cfg_n_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_ENDP_INIT_ULSO_CFG_n
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_endp_init_ulso_cfg_n_s
{
  u32 ipv4_id_min_max_val_index : 2;
  u32 reserved0 : 30;
};

/* Union definition of register */
union ipa_hwio_def_ipa_endp_init_ulso_cfg_n_u
{
  struct ipa_hwio_def_ipa_endp_init_ulso_cfg_n_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_ENDP_INIT_UCP_CFG_n
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_endp_init_ucp_cfg_n_s
{
  u32 ucp_command_id : 16;
  u32 ucp_trigger_en : 1;
  u32 reserved0 : 15;
};

/* Union definition of register */
union ipa_hwio_def_ipa_endp_init_ucp_cfg_n_u
{
  struct ipa_hwio_def_ipa_endp_init_ucp_cfg_n_s def;
  u32 value;
};

/*----------------------------------------------------------------------------
 * MODULE: IPA_VMIDMT
 *--------------------------------------------------------------------------*/

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_VMIDMT_SCR0
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_vmidmt_scr0_s
{
  u32 clientpd : 1;
  u32 reserved0 : 1;
  u32 gfie : 1;
  u32 reserved1 : 1;
  u32 gcfgere : 1;
  u32 gcfgfie : 1;
  u32 transientcfg : 2;
  u32 stalld : 1;
  u32 gse : 1;
  u32 usfcfg : 1;
  u32 reserved2 : 5;
  u32 memattr : 3;
  u32 reserved3 : 1;
  u32 mtcfg : 1;
  u32 smcfcfg : 1;
  u32 shcfg : 2;
  u32 racfg : 2;
  u32 wacfg : 2;
  u32 nscfg : 2;
  u32 reserved4 : 2;
};

/* Union definition of register */
union ipa_hwio_def_ipa_vmidmt_scr0_u
{
  struct ipa_hwio_def_ipa_vmidmt_scr0_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_VMIDMT_SCR1
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_vmidmt_scr1_s
{
  u32 reserved0 : 8;
  u32 nsnumsmrgo : 6;
  u32 reserved1 : 10;
  u32 gasrae : 1;
  u32 reserved2 : 7;
};

/* Union definition of register */
union ipa_hwio_def_ipa_vmidmt_scr1_u
{
  struct ipa_hwio_def_ipa_vmidmt_scr1_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_VMIDMT_SCR2
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_vmidmt_scr2_s
{
  u32 bpvmid : 5;
  u32 reserved0 : 27;
};

/* Union definition of register */
union ipa_hwio_def_ipa_vmidmt_scr2_u
{
  struct ipa_hwio_def_ipa_vmidmt_scr2_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_VMIDMT_SACR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_vmidmt_sacr_s
{
  u32 bpreqpriority : 2;
  u32 reserved0 : 2;
  u32 bpreqprioritycfg : 1;
  u32 reserved1 : 23;
  u32 bprcosh : 1;
  u32 bprcish : 1;
  u32 bprcnsh : 1;
  u32 reserved2 : 1;
};

/* Union definition of register */
union ipa_hwio_def_ipa_vmidmt_sacr_u
{
  struct ipa_hwio_def_ipa_vmidmt_sacr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_VMIDMT_SIDR0
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_vmidmt_sidr0_s
{
  u32 numsmrg : 8;
  u32 reserved0 : 1;
  u32 numsidb : 4;
  u32 reserved1 : 14;
  u32 sms : 1;
  u32 reserved2 : 3;
  u32 ses : 1;
};

/* Union definition of register */
union ipa_hwio_def_ipa_vmidmt_sidr0_u
{
  struct ipa_hwio_def_ipa_vmidmt_sidr0_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_VMIDMT_SIDR1
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_vmidmt_sidr1_s
{
  u32 reserved0 : 8;
  u32 numssdndx : 4;
  u32 ssdtp : 1;
  u32 reserved1 : 2;
  u32 smcd : 1;
  u32 reserved2 : 16;
};

/* Union definition of register */
union ipa_hwio_def_ipa_vmidmt_sidr1_u
{
  struct ipa_hwio_def_ipa_vmidmt_sidr1_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_VMIDMT_SIDR2
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_vmidmt_sidr2_s
{
  u32 ias : 4;
  u32 oas : 4;
  u32 reserved0 : 24;
};

/* Union definition of register */
union ipa_hwio_def_ipa_vmidmt_sidr2_u
{
  struct ipa_hwio_def_ipa_vmidmt_sidr2_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_VMIDMT_SIDR4
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_vmidmt_sidr4_s
{
  u32 step : 16;
  u32 minor : 12;
  u32 major : 4;
};

/* Union definition of register */
union ipa_hwio_def_ipa_vmidmt_sidr4_u
{
  struct ipa_hwio_def_ipa_vmidmt_sidr4_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_VMIDMT_SIDR5
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_vmidmt_sidr5_s
{
  u32 nvmid : 8;
  u32 qribe : 1;
  u32 msae : 1;
  u32 reserved0 : 6;
  u32 nummsdrb : 8;
  u32 reserved1 : 8;
};

/* Union definition of register */
union ipa_hwio_def_ipa_vmidmt_sidr5_u
{
  struct ipa_hwio_def_ipa_vmidmt_sidr5_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_VMIDMT_SIDR7
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_vmidmt_sidr7_s
{
  u32 minor : 4;
  u32 major : 4;
  u32 reserved0 : 24;
};

/* Union definition of register */
union ipa_hwio_def_ipa_vmidmt_sidr7_u
{
  struct ipa_hwio_def_ipa_vmidmt_sidr7_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_VMIDMT_SGFAR0
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_vmidmt_sgfar0_s
{
  u32 sgfea0 : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_vmidmt_sgfar0_u
{
  struct ipa_hwio_def_ipa_vmidmt_sgfar0_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_VMIDMT_SGFAR1
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_vmidmt_sgfar1_s
{
  u32 sgfea1 : 8;
  u32 reserved0 : 24;
};

/* Union definition of register */
union ipa_hwio_def_ipa_vmidmt_sgfar1_u
{
  struct ipa_hwio_def_ipa_vmidmt_sgfar1_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_VMIDMT_SGFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_vmidmt_sgfsr_s
{
  u32 reserved0 : 1;
  u32 usf : 1;
  u32 smcf : 1;
  u32 reserved1 : 2;
  u32 caf : 1;
  u32 reserved2 : 24;
  u32 multi_cfg : 1;
  u32 multi_client : 1;
};

/* Union definition of register */
union ipa_hwio_def_ipa_vmidmt_sgfsr_u
{
  struct ipa_hwio_def_ipa_vmidmt_sgfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_VMIDMT_SGFSRRESTORE
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_vmidmt_sgfsrrestore_s
{
  u32 reserved0 : 1;
  u32 usf : 1;
  u32 smcf : 1;
  u32 reserved1 : 2;
  u32 caf : 1;
  u32 reserved2 : 24;
  u32 multi_cfg : 1;
  u32 multi_client : 1;
};

/* Union definition of register */
union ipa_hwio_def_ipa_vmidmt_sgfsrrestore_u
{
  struct ipa_hwio_def_ipa_vmidmt_sgfsrrestore_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_VMIDMT_SGFSYNDR0
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_vmidmt_sgfsyndr0_s
{
  u32 reserved0 : 1;
  u32 wnr : 1;
  u32 reserved1 : 2;
  u32 nsstate : 1;
  u32 nsattr : 1;
  u32 reserved2 : 2;
  u32 mssselfauth : 1;
  u32 reserved3 : 23;
};

/* Union definition of register */
union ipa_hwio_def_ipa_vmidmt_sgfsyndr0_u
{
  struct ipa_hwio_def_ipa_vmidmt_sgfsyndr0_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_VMIDMT_SGFSYNDR1
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_vmidmt_sgfsyndr1_s
{
  u32 streamindex : 8;
  u32 reserved0 : 8;
  u32 ssdindex : 8;
  u32 msdindex : 7;
  u32 reserved1 : 1;
};

/* Union definition of register */
union ipa_hwio_def_ipa_vmidmt_sgfsyndr1_u
{
  struct ipa_hwio_def_ipa_vmidmt_sgfsyndr1_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_VMIDMT_SGFSYNDR2
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_vmidmt_sgfsyndr2_s
{
  u32 amid : 8;
  u32 apid : 5;
  u32 abid : 3;
  u32 avmid : 5;
  u32 reserved0 : 3;
  u32 atid : 5;
  u32 reserved1 : 3;
};

/* Union definition of register */
union ipa_hwio_def_ipa_vmidmt_sgfsyndr2_u
{
  struct ipa_hwio_def_ipa_vmidmt_sgfsyndr2_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_VMIDMT_VMIDMTSCR0
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_vmidmt_vmidmtscr0_s
{
  u32 clkonoffe : 1;
  u32 reserved0 : 31;
};

/* Union definition of register */
union ipa_hwio_def_ipa_vmidmt_vmidmtscr0_u
{
  struct ipa_hwio_def_ipa_vmidmt_vmidmtscr0_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_VMIDMT_CR0
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_vmidmt_cr0_s
{
  u32 clientpd : 1;
  u32 reserved0 : 1;
  u32 gfie : 1;
  u32 reserved1 : 1;
  u32 gcfgere : 1;
  u32 gcfgfie : 1;
  u32 transientcfg : 2;
  u32 stalld : 1;
  u32 gse : 1;
  u32 usfcfg : 1;
  u32 vmidpne : 1;
  u32 reserved2 : 4;
  u32 memattr : 3;
  u32 reserved3 : 1;
  u32 mtcfg : 1;
  u32 smcfcfg : 1;
  u32 shcfg : 2;
  u32 racfg : 2;
  u32 wacfg : 2;
  u32 reserved4 : 4;
};

/* Union definition of register */
union ipa_hwio_def_ipa_vmidmt_cr0_u
{
  struct ipa_hwio_def_ipa_vmidmt_cr0_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_VMIDMT_CR2
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_vmidmt_cr2_s
{
  u32 bpvmid : 5;
  u32 reserved0 : 27;
};

/* Union definition of register */
union ipa_hwio_def_ipa_vmidmt_cr2_u
{
  struct ipa_hwio_def_ipa_vmidmt_cr2_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_VMIDMT_ACR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_vmidmt_acr_s
{
  u32 bpreqpriority : 2;
  u32 reserved0 : 2;
  u32 bpreqprioritycfg : 1;
  u32 reserved1 : 23;
  u32 bprcosh : 1;
  u32 bprcish : 1;
  u32 bprcnsh : 1;
  u32 reserved2 : 1;
};

/* Union definition of register */
union ipa_hwio_def_ipa_vmidmt_acr_u
{
  struct ipa_hwio_def_ipa_vmidmt_acr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_VMIDMT_IDR0
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_vmidmt_idr0_s
{
  u32 numsmrg : 8;
  u32 reserved0 : 1;
  u32 numsidb : 4;
  u32 reserved1 : 14;
  u32 sms : 1;
  u32 reserved2 : 4;
};

/* Union definition of register */
union ipa_hwio_def_ipa_vmidmt_idr0_u
{
  struct ipa_hwio_def_ipa_vmidmt_idr0_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_VMIDMT_IDR1
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_vmidmt_idr1_s
{
  u32 reserved0 : 8;
  u32 numssdndx : 4;
  u32 ssdtp : 1;
  u32 reserved1 : 2;
  u32 smcd : 1;
  u32 reserved2 : 16;
};

/* Union definition of register */
union ipa_hwio_def_ipa_vmidmt_idr1_u
{
  struct ipa_hwio_def_ipa_vmidmt_idr1_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_VMIDMT_IDR2
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_vmidmt_idr2_s
{
  u32 ias : 4;
  u32 oas : 4;
  u32 reserved0 : 24;
};

/* Union definition of register */
union ipa_hwio_def_ipa_vmidmt_idr2_u
{
  struct ipa_hwio_def_ipa_vmidmt_idr2_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_VMIDMT_IDR4
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_vmidmt_idr4_s
{
  u32 step : 16;
  u32 minor : 12;
  u32 major : 4;
};

/* Union definition of register */
union ipa_hwio_def_ipa_vmidmt_idr4_u
{
  struct ipa_hwio_def_ipa_vmidmt_idr4_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_VMIDMT_IDR5
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_vmidmt_idr5_s
{
  u32 nvmid : 8;
  u32 qribe : 1;
  u32 msae : 1;
  u32 reserved0 : 6;
  u32 nummsdrb : 8;
  u32 reserved1 : 8;
};

/* Union definition of register */
union ipa_hwio_def_ipa_vmidmt_idr5_u
{
  struct ipa_hwio_def_ipa_vmidmt_idr5_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_VMIDMT_IDR7
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_vmidmt_idr7_s
{
  u32 minor : 4;
  u32 major : 4;
  u32 reserved0 : 24;
};

/* Union definition of register */
union ipa_hwio_def_ipa_vmidmt_idr7_u
{
  struct ipa_hwio_def_ipa_vmidmt_idr7_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_VMIDMT_GFAR0
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_vmidmt_gfar0_s
{
  u32 gfea0 : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_vmidmt_gfar0_u
{
  struct ipa_hwio_def_ipa_vmidmt_gfar0_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_VMIDMT_GFAR1
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_vmidmt_gfar1_s
{
  u32 gfea1 : 8;
  u32 reserved0 : 24;
};

/* Union definition of register */
union ipa_hwio_def_ipa_vmidmt_gfar1_u
{
  struct ipa_hwio_def_ipa_vmidmt_gfar1_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_VMIDMT_GFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_vmidmt_gfsr_s
{
  u32 reserved0 : 1;
  u32 usf : 1;
  u32 smcf : 1;
  u32 reserved1 : 2;
  u32 caf : 1;
  u32 reserved2 : 1;
  u32 pf : 1;
  u32 reserved3 : 22;
  u32 multi_cfg : 1;
  u32 multi_client : 1;
};

/* Union definition of register */
union ipa_hwio_def_ipa_vmidmt_gfsr_u
{
  struct ipa_hwio_def_ipa_vmidmt_gfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_VMIDMT_GFSRRESTORE
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_vmidmt_gfsrrestore_s
{
  u32 reserved0 : 1;
  u32 usf : 1;
  u32 smcf : 1;
  u32 reserved1 : 2;
  u32 caf : 1;
  u32 reserved2 : 1;
  u32 pf : 1;
  u32 reserved3 : 22;
  u32 multi_cfg : 1;
  u32 multi_client : 1;
};

/* Union definition of register */
union ipa_hwio_def_ipa_vmidmt_gfsrrestore_u
{
  struct ipa_hwio_def_ipa_vmidmt_gfsrrestore_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_VMIDMT_GFSYNDR0
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_vmidmt_gfsyndr0_s
{
  u32 reserved0 : 1;
  u32 wnr : 1;
  u32 reserved1 : 2;
  u32 nsstate : 1;
  u32 nsattr : 1;
  u32 reserved2 : 2;
  u32 mssselfauth : 1;
  u32 reserved3 : 23;
};

/* Union definition of register */
union ipa_hwio_def_ipa_vmidmt_gfsyndr0_u
{
  struct ipa_hwio_def_ipa_vmidmt_gfsyndr0_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_VMIDMT_GFSYNDR1
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_vmidmt_gfsyndr1_s
{
  u32 streamindex : 8;
  u32 reserved0 : 8;
  u32 ssdindex : 8;
  u32 msdindex : 7;
  u32 reserved1 : 1;
};

/* Union definition of register */
union ipa_hwio_def_ipa_vmidmt_gfsyndr1_u
{
  struct ipa_hwio_def_ipa_vmidmt_gfsyndr1_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_VMIDMT_GFSYNDR2
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_vmidmt_gfsyndr2_s
{
  u32 amid : 8;
  u32 apid : 5;
  u32 abid : 3;
  u32 avmid : 5;
  u32 reserved0 : 3;
  u32 atid : 5;
  u32 reserved1 : 3;
};

/* Union definition of register */
union ipa_hwio_def_ipa_vmidmt_gfsyndr2_u
{
  struct ipa_hwio_def_ipa_vmidmt_gfsyndr2_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_VMIDMT_VMIDMTCR0
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_vmidmt_vmidmtcr0_s
{
  u32 clkonoffe : 1;
  u32 reserved0 : 31;
};

/* Union definition of register */
union ipa_hwio_def_ipa_vmidmt_vmidmtcr0_u
{
  struct ipa_hwio_def_ipa_vmidmt_vmidmtcr0_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_VMIDMT_VMIDMTACR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_vmidmt_vmidmtacr_s
{
  u32 rwe : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_vmidmt_vmidmtacr_u
{
  struct ipa_hwio_def_ipa_vmidmt_vmidmtacr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_VMIDMT_NSCR0
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_vmidmt_nscr0_s
{
  u32 clientpd : 1;
  u32 reserved0 : 1;
  u32 gfie : 1;
  u32 reserved1 : 1;
  u32 gcfgere : 1;
  u32 gcfgfie : 1;
  u32 transientcfg : 2;
  u32 stalld : 1;
  u32 gse : 1;
  u32 usfcfg : 1;
  u32 vmidpne : 1;
  u32 reserved2 : 4;
  u32 memattr : 3;
  u32 reserved3 : 1;
  u32 mtcfg : 1;
  u32 smcfcfg : 1;
  u32 shcfg : 2;
  u32 racfg : 2;
  u32 wacfg : 2;
  u32 reserved4 : 4;
};

/* Union definition of register */
union ipa_hwio_def_ipa_vmidmt_nscr0_u
{
  struct ipa_hwio_def_ipa_vmidmt_nscr0_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_VMIDMT_NSCR2
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_vmidmt_nscr2_s
{
  u32 bpvmid : 5;
  u32 reserved0 : 27;
};

/* Union definition of register */
union ipa_hwio_def_ipa_vmidmt_nscr2_u
{
  struct ipa_hwio_def_ipa_vmidmt_nscr2_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_VMIDMT_NSACR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_vmidmt_nsacr_s
{
  u32 bpreqpriority : 2;
  u32 reserved0 : 2;
  u32 bpreqprioritycfg : 1;
  u32 reserved1 : 23;
  u32 bprcosh : 1;
  u32 bprcish : 1;
  u32 bprcnsh : 1;
  u32 reserved2 : 1;
};

/* Union definition of register */
union ipa_hwio_def_ipa_vmidmt_nsacr_u
{
  struct ipa_hwio_def_ipa_vmidmt_nsacr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_VMIDMT_NSGFAR0
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_vmidmt_nsgfar0_s
{
  u32 gfea0 : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_vmidmt_nsgfar0_u
{
  struct ipa_hwio_def_ipa_vmidmt_nsgfar0_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_VMIDMT_NSGFAR1
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_vmidmt_nsgfar1_s
{
  u32 gfea1 : 8;
  u32 reserved0 : 24;
};

/* Union definition of register */
union ipa_hwio_def_ipa_vmidmt_nsgfar1_u
{
  struct ipa_hwio_def_ipa_vmidmt_nsgfar1_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_VMIDMT_NSGFSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_vmidmt_nsgfsr_s
{
  u32 reserved0 : 1;
  u32 usf : 1;
  u32 smcf : 1;
  u32 reserved1 : 2;
  u32 caf : 1;
  u32 reserved2 : 1;
  u32 pf : 1;
  u32 reserved3 : 22;
  u32 multi_cfg : 1;
  u32 multi_client : 1;
};

/* Union definition of register */
union ipa_hwio_def_ipa_vmidmt_nsgfsr_u
{
  struct ipa_hwio_def_ipa_vmidmt_nsgfsr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_VMIDMT_NSGFSRRESTORE
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_vmidmt_nsgfsrrestore_s
{
  u32 reserved0 : 1;
  u32 usf : 1;
  u32 smcf : 1;
  u32 reserved1 : 2;
  u32 caf : 1;
  u32 reserved2 : 1;
  u32 pf : 1;
  u32 reserved3 : 22;
  u32 multi_cfg : 1;
  u32 multi_client : 1;
};

/* Union definition of register */
union ipa_hwio_def_ipa_vmidmt_nsgfsrrestore_u
{
  struct ipa_hwio_def_ipa_vmidmt_nsgfsrrestore_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_VMIDMT_NSGFSYNDR0
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_vmidmt_nsgfsyndr0_s
{
  u32 reserved0 : 1;
  u32 wnr : 1;
  u32 reserved1 : 2;
  u32 nsstate : 1;
  u32 nsattr : 1;
  u32 reserved2 : 2;
  u32 mssselfauth : 1;
  u32 reserved3 : 23;
};

/* Union definition of register */
union ipa_hwio_def_ipa_vmidmt_nsgfsyndr0_u
{
  struct ipa_hwio_def_ipa_vmidmt_nsgfsyndr0_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_VMIDMT_NSGFSYNDR1
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_vmidmt_nsgfsyndr1_s
{
  u32 streamindex : 8;
  u32 reserved0 : 8;
  u32 ssdindex : 8;
  u32 msdindex : 7;
  u32 reserved1 : 1;
};

/* Union definition of register */
union ipa_hwio_def_ipa_vmidmt_nsgfsyndr1_u
{
  struct ipa_hwio_def_ipa_vmidmt_nsgfsyndr1_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_VMIDMT_NSGFSYNDR2
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_vmidmt_nsgfsyndr2_s
{
  u32 amid : 8;
  u32 apid : 5;
  u32 abid : 3;
  u32 avmid : 5;
  u32 reserved0 : 3;
  u32 atid : 5;
  u32 reserved1 : 3;
};

/* Union definition of register */
union ipa_hwio_def_ipa_vmidmt_nsgfsyndr2_u
{
  struct ipa_hwio_def_ipa_vmidmt_nsgfsyndr2_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_VMIDMT_NSVMIDMTCR0
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_vmidmt_nsvmidmtcr0_s
{
  u32 clkonoffe : 1;
  u32 reserved0 : 31;
};

/* Union definition of register */
union ipa_hwio_def_ipa_vmidmt_nsvmidmtcr0_u
{
  struct ipa_hwio_def_ipa_vmidmt_nsvmidmtcr0_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_VMIDMT_SSDR0
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_vmidmt_ssdr0_s
{
  u32 rwe : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_vmidmt_ssdr0_u
{
  struct ipa_hwio_def_ipa_vmidmt_ssdr0_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_VMIDMT_SSDR1
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_vmidmt_ssdr1_s
{
  u32 rwe : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_vmidmt_ssdr1_u
{
  struct ipa_hwio_def_ipa_vmidmt_ssdr1_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_VMIDMT_SSDR2
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_vmidmt_ssdr2_s
{
  u32 rwe : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_vmidmt_ssdr2_u
{
  struct ipa_hwio_def_ipa_vmidmt_ssdr2_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_VMIDMT_SSDR3
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_vmidmt_ssdr3_s
{
  u32 rwe : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_vmidmt_ssdr3_u
{
  struct ipa_hwio_def_ipa_vmidmt_ssdr3_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_VMIDMT_MSDR0
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_vmidmt_msdr0_s
{
  u32 rwe : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_vmidmt_msdr0_u
{
  struct ipa_hwio_def_ipa_vmidmt_msdr0_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_VMIDMT_MSDR1
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_vmidmt_msdr1_s
{
  u32 rwe : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_vmidmt_msdr1_u
{
  struct ipa_hwio_def_ipa_vmidmt_msdr1_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_VMIDMT_MSDR2
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_vmidmt_msdr2_s
{
  u32 rwe : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_vmidmt_msdr2_u
{
  struct ipa_hwio_def_ipa_vmidmt_msdr2_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_VMIDMT_MSDR3
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_vmidmt_msdr3_s
{
  u32 rwe : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_vmidmt_msdr3_u
{
  struct ipa_hwio_def_ipa_vmidmt_msdr3_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_VMIDMT_MCR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_vmidmt_mcr_s
{
  u32 bpsmsacfg : 1;
  u32 bpmsacfg : 1;
  u32 clkonoffe : 1;
  u32 reserved0 : 29;
};

/* Union definition of register */
union ipa_hwio_def_ipa_vmidmt_mcr_u
{
  struct ipa_hwio_def_ipa_vmidmt_mcr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_VMIDMT_S2VRn
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_vmidmt_s2vrn_s
{
  u32 vmid : 5;
  u32 reserved0 : 3;
  u32 shcfg : 2;
  u32 reserved1 : 1;
  u32 mtcfg : 1;
  u32 memattr : 3;
  u32 reserved2 : 1;
  u32 type : 2;
  u32 nscfg : 2;
  u32 racfg : 2;
  u32 wacfg : 2;
  u32 reserved3 : 4;
  u32 transientcfg : 2;
  u32 reserved4 : 2;
};

/* Union definition of register */
union ipa_hwio_def_ipa_vmidmt_s2vrn_u
{
  struct ipa_hwio_def_ipa_vmidmt_s2vrn_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_VMIDMT_AS2VRn
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_vmidmt_as2vrn_s
{
  u32 reqpriority : 2;
  u32 reserved0 : 2;
  u32 reqprioritycfg : 1;
  u32 reserved1 : 23;
  u32 rcosh : 1;
  u32 rcish : 1;
  u32 rcnsh : 1;
  u32 reserved2 : 1;
};

/* Union definition of register */
union ipa_hwio_def_ipa_vmidmt_as2vrn_u
{
  struct ipa_hwio_def_ipa_vmidmt_as2vrn_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_VMIDMT_SMRn
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_vmidmt_smrn_s
{
  u32 id : 8;
  u32 reserved0 : 8;
  u32 mask : 8;
  u32 reserved1 : 7;
  u32 valid : 1;
};

/* Union definition of register */
union ipa_hwio_def_ipa_vmidmt_smrn_u
{
  struct ipa_hwio_def_ipa_vmidmt_smrn_s def;
  u32 value;
};

/*----------------------------------------------------------------------------
 * MODULE: IPA_0_GSI_TOP
 *--------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
 * MODULE: IPA_GSI_TOP_GSI
 *--------------------------------------------------------------------------*/

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_GSI_TOP_GSI_CFG
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_gsi_top_gsi_cfg_s
{
  u32 gsi_enable : 1;
  u32 mcs_enable : 1;
  u32 double_mcs_clk_freq : 1;
  u32 uc_is_mcs : 1;
  u32 gsi_pwr_clps : 1;
  u32 bp_mtrix_disable : 1;
  u32 reserved0 : 2;
  u32 sleep_clk_div : 4;
  u32 reserved1 : 20;
};

/* Union definition of register */
union ipa_hwio_def_ipa_gsi_top_gsi_cfg_u
{
  struct ipa_hwio_def_ipa_gsi_top_gsi_cfg_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_GSI_TOP_GSI_MANAGER_MCS_CODE_VER
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_gsi_top_gsi_manager_mcs_code_ver_s
{
  u32 ver : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_gsi_top_gsi_manager_mcs_code_ver_u
{
  struct ipa_hwio_def_ipa_gsi_top_gsi_manager_mcs_code_ver_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_GSI_TOP_GSI_ZEROS
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_gsi_top_gsi_zeros_s
{
  u32 zeros : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_gsi_top_gsi_zeros_u
{
  struct ipa_hwio_def_ipa_gsi_top_gsi_zeros_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_GSI_TOP_GSI_PERIPH_BASE_ADDR_LSB
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_gsi_top_gsi_periph_base_addr_lsb_s
{
  u32 base_addr : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_gsi_top_gsi_periph_base_addr_lsb_u
{
  struct ipa_hwio_def_ipa_gsi_top_gsi_periph_base_addr_lsb_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_GSI_TOP_GSI_PERIPH_BASE_ADDR_MSB
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_gsi_top_gsi_periph_base_addr_msb_s
{
  u32 base_addr : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_gsi_top_gsi_periph_base_addr_msb_u
{
  struct ipa_hwio_def_ipa_gsi_top_gsi_periph_base_addr_msb_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_GSI_TOP_GSI_CGC_CTRL
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_gsi_top_gsi_cgc_ctrl_s
{
  u32 region_1_hw_cgc_en : 1;
  u32 region_2_hw_cgc_en : 1;
  u32 region_3_hw_cgc_en : 1;
  u32 region_4_hw_cgc_en : 1;
  u32 region_5_hw_cgc_en : 1;
  u32 region_6_hw_cgc_en : 1;
  u32 region_7_hw_cgc_en : 1;
  u32 region_8_hw_cgc_en : 1;
  u32 region_9_hw_cgc_en : 1;
  u32 region_10_hw_cgc_en : 1;
  u32 region_11_hw_cgc_en : 1;
  u32 region_12_hw_cgc_en : 1;
  u32 region_13_hw_cgc_en : 1;
  u32 region_14_hw_cgc_en : 1;
  u32 region_15_hw_cgc_en : 1;
  u32 region_16_hw_cgc_en : 1;
  u32 reserved0 : 16;
};

/* Union definition of register */
union ipa_hwio_def_ipa_gsi_top_gsi_cgc_ctrl_u
{
  struct ipa_hwio_def_ipa_gsi_top_gsi_cgc_ctrl_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_GSI_TOP_GSI_MOQA_CFG
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_gsi_top_gsi_moqa_cfg_s
{
  u32 client_req_prio : 8;
  u32 client_oord : 8;
  u32 client_oowr : 8;
  u32 reserved0 : 8;
};

/* Union definition of register */
union ipa_hwio_def_ipa_gsi_top_gsi_moqa_cfg_u
{
  struct ipa_hwio_def_ipa_gsi_top_gsi_moqa_cfg_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_GSI_TOP_GSI_REE_CFG
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_gsi_top_gsi_ree_cfg_s
{
  u32 move_to_esc_clr_mode_trsh : 1;
  u32 channel_empty_int_enable : 1;
  u32 reserved0 : 6;
  u32 max_burst_size : 8;
  u32 reserved1 : 16;
};

/* Union definition of register */
union ipa_hwio_def_ipa_gsi_top_gsi_ree_cfg_u
{
  struct ipa_hwio_def_ipa_gsi_top_gsi_ree_cfg_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_GSI_TOP_GSI_PERIPH_PENDING_k
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_gsi_top_gsi_periph_pending_k_s
{
  u32 chid_bit_map : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_gsi_top_gsi_periph_pending_k_u
{
  struct ipa_hwio_def_ipa_gsi_top_gsi_periph_pending_k_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_GSI_TOP_GSI_MSI_CACHEATTR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_gsi_top_gsi_msi_cacheattr_s
{
  u32 ashared : 1;
  u32 ainnershared : 1;
  u32 anoallocate : 1;
  u32 atransient : 1;
  u32 areqpriority : 2;
  u32 reserved0 : 26;
};

/* Union definition of register */
union ipa_hwio_def_ipa_gsi_top_gsi_msi_cacheattr_u
{
  struct ipa_hwio_def_ipa_gsi_top_gsi_msi_cacheattr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_GSI_TOP_GSI_EVENT_CACHEATTR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_gsi_top_gsi_event_cacheattr_s
{
  u32 ashared : 1;
  u32 ainnershared : 1;
  u32 anoallocate : 1;
  u32 atransient : 1;
  u32 areqpriority : 2;
  u32 reserved0 : 26;
};

/* Union definition of register */
union ipa_hwio_def_ipa_gsi_top_gsi_event_cacheattr_u
{
  struct ipa_hwio_def_ipa_gsi_top_gsi_event_cacheattr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_GSI_TOP_GSI_DATA_CACHEATTR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_gsi_top_gsi_data_cacheattr_s
{
  u32 ashared : 1;
  u32 ainnershared : 1;
  u32 anoallocate : 1;
  u32 atransient : 1;
  u32 areqpriority : 2;
  u32 reserved0 : 26;
};

/* Union definition of register */
union ipa_hwio_def_ipa_gsi_top_gsi_data_cacheattr_u
{
  struct ipa_hwio_def_ipa_gsi_top_gsi_data_cacheattr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_GSI_TOP_GSI_TRE_CACHEATTR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_gsi_top_gsi_tre_cacheattr_s
{
  u32 ashared : 1;
  u32 ainnershared : 1;
  u32 anoallocate : 1;
  u32 atransient : 1;
  u32 areqpriority : 2;
  u32 reserved0 : 26;
};

/* Union definition of register */
union ipa_hwio_def_ipa_gsi_top_gsi_tre_cacheattr_u
{
  struct ipa_hwio_def_ipa_gsi_top_gsi_tre_cacheattr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_GSI_TOP_IC_INT_WEIGHT_REE
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_gsi_top_ic_int_weight_ree_s
{
  u32 stop_ch_comp_int_weight : 4;
  u32 new_re_int_weight : 4;
  u32 ch_empty_int_weight : 4;
  u32 reserved0 : 20;
};

/* Union definition of register */
union ipa_hwio_def_ipa_gsi_top_ic_int_weight_ree_u
{
  struct ipa_hwio_def_ipa_gsi_top_ic_int_weight_ree_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_GSI_TOP_IC_INT_WEIGHT_EVT_ENG
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_gsi_top_ic_int_weight_evt_eng_s
{
  u32 evnt_eng_int_weight : 4;
  u32 reserved0 : 28;
};

/* Union definition of register */
union ipa_hwio_def_ipa_gsi_top_ic_int_weight_evt_eng_u
{
  struct ipa_hwio_def_ipa_gsi_top_ic_int_weight_evt_eng_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_GSI_TOP_IC_INT_WEIGHT_INT_ENG
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_gsi_top_ic_int_weight_int_eng_s
{
  u32 int_eng_int_weight : 4;
  u32 reserved0 : 28;
};

/* Union definition of register */
union ipa_hwio_def_ipa_gsi_top_ic_int_weight_int_eng_u
{
  struct ipa_hwio_def_ipa_gsi_top_ic_int_weight_int_eng_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_GSI_TOP_IC_INT_WEIGHT_CSR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_gsi_top_ic_int_weight_csr_s
{
  u32 ch_cmd_int_weight : 4;
  u32 ee_generic_int_weight : 4;
  u32 reserved0 : 24;
};

/* Union definition of register */
union ipa_hwio_def_ipa_gsi_top_ic_int_weight_csr_u
{
  struct ipa_hwio_def_ipa_gsi_top_ic_int_weight_csr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_GSI_TOP_IC_INT_WEIGHT_TLV_ENG
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_gsi_top_ic_int_weight_tlv_eng_s
{
  u32 tlv_0_int_weight : 4;
  u32 tlv_1_int_weight : 4;
  u32 tlv_2_int_weight : 4;
  u32 ch_not_full_int_weight : 4;
  u32 reserved0 : 16;
};

/* Union definition of register */
union ipa_hwio_def_ipa_gsi_top_ic_int_weight_tlv_eng_u
{
  struct ipa_hwio_def_ipa_gsi_top_ic_int_weight_tlv_eng_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_GSI_TOP_IC_INT_WEIGHT_TIMER_ENG
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_gsi_top_ic_int_weight_timer_eng_s
{
  u32 timer_int_weight : 4;
  u32 reserved0 : 28;
};

/* Union definition of register */
union ipa_hwio_def_ipa_gsi_top_ic_int_weight_timer_eng_u
{
  struct ipa_hwio_def_ipa_gsi_top_ic_int_weight_timer_eng_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_GSI_TOP_IC_INT_WEIGHT_DB_ENG
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_gsi_top_ic_int_weight_db_eng_s
{
  u32 new_db_int_weight : 4;
  u32 reserved0 : 28;
};

/* Union definition of register */
union ipa_hwio_def_ipa_gsi_top_ic_int_weight_db_eng_u
{
  struct ipa_hwio_def_ipa_gsi_top_ic_int_weight_db_eng_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_GSI_TOP_IC_INT_WEIGHT_RD_WR_ENG
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_gsi_top_ic_int_weight_rd_wr_eng_s
{
  u32 read_int_weight : 4;
  u32 write_int_weight : 4;
  u32 reserved0 : 24;
};

/* Union definition of register */
union ipa_hwio_def_ipa_gsi_top_ic_int_weight_rd_wr_eng_u
{
  struct ipa_hwio_def_ipa_gsi_top_ic_int_weight_rd_wr_eng_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_GSI_TOP_IC_INT_WEIGHT_UCONTROLLER_ENG
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_gsi_top_ic_int_weight_ucontroller_eng_s
{
  u32 ucontroller_gp_int_weight : 4;
  u32 reserved0 : 28;
};

/* Union definition of register */
union ipa_hwio_def_ipa_gsi_top_ic_int_weight_ucontroller_eng_u
{
  struct ipa_hwio_def_ipa_gsi_top_ic_int_weight_ucontroller_eng_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_GSI_TOP_LOW_LATENCY_ARB_WEIGHT
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_gsi_top_low_latency_arb_weight_s
{
  u32 ll_weight : 6;
  u32 reserved0 : 2;
  u32 non_ll_weight : 6;
  u32 reserved1 : 2;
  u32 ll_non_ll_fix_priority : 1;
  u32 reserved2 : 15;
};

/* Union definition of register */
union ipa_hwio_def_ipa_gsi_top_low_latency_arb_weight_u
{
  struct ipa_hwio_def_ipa_gsi_top_low_latency_arb_weight_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_GSI_TOP_GSI_MANAGER_EE_QOS_n
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_gsi_top_gsi_manager_ee_qos_n_s
{
  u32 ee_prio : 2;
  u32 reserved0 : 6;
  u32 max_ch_alloc : 8;
  u32 max_ev_alloc : 8;
  u32 reserved1 : 8;
};

/* Union definition of register */
union ipa_hwio_def_ipa_gsi_top_gsi_manager_ee_qos_n_u
{
  struct ipa_hwio_def_ipa_gsi_top_gsi_manager_ee_qos_n_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_GSI_TOP_GSI_SHRAM_PTR_CH_CNTXT_BASE_ADDR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_gsi_top_gsi_shram_ptr_ch_cntxt_base_addr_s
{
  u32 shram_ptr : 16;
  u32 reserved0 : 16;
};

/* Union definition of register */
union ipa_hwio_def_ipa_gsi_top_gsi_shram_ptr_ch_cntxt_base_addr_u
{
  struct ipa_hwio_def_ipa_gsi_top_gsi_shram_ptr_ch_cntxt_base_addr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_GSI_TOP_GSI_SHRAM_PTR_EV_CNTXT_BASE_ADDR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_gsi_top_gsi_shram_ptr_ev_cntxt_base_addr_s
{
  u32 shram_ptr : 16;
  u32 reserved0 : 16;
};

/* Union definition of register */
union ipa_hwio_def_ipa_gsi_top_gsi_shram_ptr_ev_cntxt_base_addr_u
{
  struct ipa_hwio_def_ipa_gsi_top_gsi_shram_ptr_ev_cntxt_base_addr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_GSI_TOP_GSI_SHRAM_PTR_RE_STORAGE_BASE_ADDR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_gsi_top_gsi_shram_ptr_re_storage_base_addr_s
{
  u32 shram_ptr : 16;
  u32 reserved0 : 16;
};

/* Union definition of register */
union ipa_hwio_def_ipa_gsi_top_gsi_shram_ptr_re_storage_base_addr_u
{
  struct ipa_hwio_def_ipa_gsi_top_gsi_shram_ptr_re_storage_base_addr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_GSI_TOP_GSI_SHRAM_PTR_RE_ESC_BUF_BASE_ADDR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_gsi_top_gsi_shram_ptr_re_esc_buf_base_addr_s
{
  u32 shram_ptr : 16;
  u32 reserved0 : 16;
};

/* Union definition of register */
union ipa_hwio_def_ipa_gsi_top_gsi_shram_ptr_re_esc_buf_base_addr_u
{
  struct ipa_hwio_def_ipa_gsi_top_gsi_shram_ptr_re_esc_buf_base_addr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_GSI_TOP_GSI_SHRAM_PTR_EE_SCRACH_BASE_ADDR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_gsi_top_gsi_shram_ptr_ee_scrach_base_addr_s
{
  u32 shram_ptr : 16;
  u32 reserved0 : 16;
};

/* Union definition of register */
union ipa_hwio_def_ipa_gsi_top_gsi_shram_ptr_ee_scrach_base_addr_u
{
  struct ipa_hwio_def_ipa_gsi_top_gsi_shram_ptr_ee_scrach_base_addr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_GSI_TOP_GSI_SHRAM_PTR_FUNC_STACK_BASE_ADDR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_gsi_top_gsi_shram_ptr_func_stack_base_addr_s
{
  u32 shram_ptr : 16;
  u32 reserved0 : 16;
};

/* Union definition of register */
union ipa_hwio_def_ipa_gsi_top_gsi_shram_ptr_func_stack_base_addr_u
{
  struct ipa_hwio_def_ipa_gsi_top_gsi_shram_ptr_func_stack_base_addr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_GSI_TOP_GSI_SHRAM_PTR_MCS_SCRATCH_BASE_ADDR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_gsi_top_gsi_shram_ptr_mcs_scratch_base_addr_s
{
  u32 shram_ptr : 16;
  u32 reserved0 : 16;
};

/* Union definition of register */
union ipa_hwio_def_ipa_gsi_top_gsi_shram_ptr_mcs_scratch_base_addr_u
{
  struct ipa_hwio_def_ipa_gsi_top_gsi_shram_ptr_mcs_scratch_base_addr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_GSI_TOP_GSI_SHRAM_PTR_MCS_SCRATCH1_BASE_ADDR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_gsi_top_gsi_shram_ptr_mcs_scratch1_base_addr_s
{
  u32 shram_ptr : 16;
  u32 reserved0 : 16;
};

/* Union definition of register */
union ipa_hwio_def_ipa_gsi_top_gsi_shram_ptr_mcs_scratch1_base_addr_u
{
  struct ipa_hwio_def_ipa_gsi_top_gsi_shram_ptr_mcs_scratch1_base_addr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_GSI_TOP_GSI_SHRAM_PTR_MCS_SCRATCH2_BASE_ADDR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_gsi_top_gsi_shram_ptr_mcs_scratch2_base_addr_s
{
  u32 shram_ptr : 16;
  u32 reserved0 : 16;
};

/* Union definition of register */
union ipa_hwio_def_ipa_gsi_top_gsi_shram_ptr_mcs_scratch2_base_addr_u
{
  struct ipa_hwio_def_ipa_gsi_top_gsi_shram_ptr_mcs_scratch2_base_addr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_GSI_TOP_GSI_SHRAM_PTR_MCS_SCRATCH3_BASE_ADDR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_gsi_top_gsi_shram_ptr_mcs_scratch3_base_addr_s
{
  u32 shram_ptr : 16;
  u32 reserved0 : 16;
};

/* Union definition of register */
union ipa_hwio_def_ipa_gsi_top_gsi_shram_ptr_mcs_scratch3_base_addr_u
{
  struct ipa_hwio_def_ipa_gsi_top_gsi_shram_ptr_mcs_scratch3_base_addr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_GSI_TOP_GSI_SHRAM_PTR_CH_VP_TRANS_TABLE_BASE_ADDR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_gsi_top_gsi_shram_ptr_ch_vp_trans_table_base_addr_s
{
  u32 shram_ptr : 16;
  u32 reserved0 : 16;
};

/* Union definition of register */
union ipa_hwio_def_ipa_gsi_top_gsi_shram_ptr_ch_vp_trans_table_base_addr_u
{
  struct ipa_hwio_def_ipa_gsi_top_gsi_shram_ptr_ch_vp_trans_table_base_addr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_GSI_TOP_GSI_SHRAM_PTR_EV_VP_TRANS_TABLE_BASE_ADDR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_gsi_top_gsi_shram_ptr_ev_vp_trans_table_base_addr_s
{
  u32 shram_ptr : 16;
  u32 reserved0 : 16;
};

/* Union definition of register */
union ipa_hwio_def_ipa_gsi_top_gsi_shram_ptr_ev_vp_trans_table_base_addr_u
{
  struct ipa_hwio_def_ipa_gsi_top_gsi_shram_ptr_ev_vp_trans_table_base_addr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_GSI_TOP_GSI_SHRAM_PTR_USER_INFO_DATA_BASE_ADDR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_gsi_top_gsi_shram_ptr_user_info_data_base_addr_s
{
  u32 shram_ptr : 16;
  u32 reserved0 : 16;
};

/* Union definition of register */
union ipa_hwio_def_ipa_gsi_top_gsi_shram_ptr_user_info_data_base_addr_u
{
  struct ipa_hwio_def_ipa_gsi_top_gsi_shram_ptr_user_info_data_base_addr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_GSI_TOP_GSI_SHRAM_PTR_EE_CMD_FIFO_BASE_ADDR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_gsi_top_gsi_shram_ptr_ee_cmd_fifo_base_addr_s
{
  u32 shram_ptr : 16;
  u32 reserved0 : 16;
};

/* Union definition of register */
union ipa_hwio_def_ipa_gsi_top_gsi_shram_ptr_ee_cmd_fifo_base_addr_u
{
  struct ipa_hwio_def_ipa_gsi_top_gsi_shram_ptr_ee_cmd_fifo_base_addr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_GSI_TOP_GSI_SHRAM_PTR_CH_CMD_FIFO_BASE_ADDR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_gsi_top_gsi_shram_ptr_ch_cmd_fifo_base_addr_s
{
  u32 shram_ptr : 16;
  u32 reserved0 : 16;
};

/* Union definition of register */
union ipa_hwio_def_ipa_gsi_top_gsi_shram_ptr_ch_cmd_fifo_base_addr_u
{
  struct ipa_hwio_def_ipa_gsi_top_gsi_shram_ptr_ch_cmd_fifo_base_addr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_GSI_TOP_GSI_SHRAM_PTR_EVE_ED_STORAGE_BASE_ADDR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_gsi_top_gsi_shram_ptr_eve_ed_storage_base_addr_s
{
  u32 shram_ptr : 16;
  u32 reserved0 : 16;
};

/* Union definition of register */
union ipa_hwio_def_ipa_gsi_top_gsi_shram_ptr_eve_ed_storage_base_addr_u
{
  struct ipa_hwio_def_ipa_gsi_top_gsi_shram_ptr_eve_ed_storage_base_addr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_GSI_TOP_GSI_IRAM_PTR_CH_CMD
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_gsi_top_gsi_iram_ptr_ch_cmd_s
{
  u32 iram_ptr : 12;
  u32 reserved0 : 20;
};

/* Union definition of register */
union ipa_hwio_def_ipa_gsi_top_gsi_iram_ptr_ch_cmd_u
{
  struct ipa_hwio_def_ipa_gsi_top_gsi_iram_ptr_ch_cmd_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_GSI_TOP_GSI_IRAM_PTR_EE_GENERIC_CMD
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_gsi_top_gsi_iram_ptr_ee_generic_cmd_s
{
  u32 iram_ptr : 12;
  u32 reserved0 : 20;
};

/* Union definition of register */
union ipa_hwio_def_ipa_gsi_top_gsi_iram_ptr_ee_generic_cmd_u
{
  struct ipa_hwio_def_ipa_gsi_top_gsi_iram_ptr_ee_generic_cmd_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_GSI_TOP_GSI_IRAM_PTR_TLV_CH_NOT_FULL
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_gsi_top_gsi_iram_ptr_tlv_ch_not_full_s
{
  u32 iram_ptr : 12;
  u32 reserved0 : 20;
};

/* Union definition of register */
union ipa_hwio_def_ipa_gsi_top_gsi_iram_ptr_tlv_ch_not_full_u
{
  struct ipa_hwio_def_ipa_gsi_top_gsi_iram_ptr_tlv_ch_not_full_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_GSI_TOP_GSI_IRAM_PTR_MSI_DB
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_gsi_top_gsi_iram_ptr_msi_db_s
{
  u32 iram_ptr : 12;
  u32 reserved0 : 20;
};

/* Union definition of register */
union ipa_hwio_def_ipa_gsi_top_gsi_iram_ptr_msi_db_u
{
  struct ipa_hwio_def_ipa_gsi_top_gsi_iram_ptr_msi_db_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_GSI_TOP_GSI_IRAM_PTR_CH_DB
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_gsi_top_gsi_iram_ptr_ch_db_s
{
  u32 iram_ptr : 12;
  u32 reserved0 : 20;
};

/* Union definition of register */
union ipa_hwio_def_ipa_gsi_top_gsi_iram_ptr_ch_db_u
{
  struct ipa_hwio_def_ipa_gsi_top_gsi_iram_ptr_ch_db_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_GSI_TOP_GSI_IRAM_PTR_EV_DB
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_gsi_top_gsi_iram_ptr_ev_db_s
{
  u32 iram_ptr : 12;
  u32 reserved0 : 20;
};

/* Union definition of register */
union ipa_hwio_def_ipa_gsi_top_gsi_iram_ptr_ev_db_u
{
  struct ipa_hwio_def_ipa_gsi_top_gsi_iram_ptr_ev_db_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_GSI_TOP_GSI_IRAM_PTR_NEW_RE
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_gsi_top_gsi_iram_ptr_new_re_s
{
  u32 iram_ptr : 12;
  u32 reserved0 : 20;
};

/* Union definition of register */
union ipa_hwio_def_ipa_gsi_top_gsi_iram_ptr_new_re_u
{
  struct ipa_hwio_def_ipa_gsi_top_gsi_iram_ptr_new_re_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_GSI_TOP_GSI_IRAM_PTR_CH_DIS_COMP
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_gsi_top_gsi_iram_ptr_ch_dis_comp_s
{
  u32 iram_ptr : 12;
  u32 reserved0 : 20;
};

/* Union definition of register */
union ipa_hwio_def_ipa_gsi_top_gsi_iram_ptr_ch_dis_comp_u
{
  struct ipa_hwio_def_ipa_gsi_top_gsi_iram_ptr_ch_dis_comp_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_GSI_TOP_GSI_IRAM_PTR_CH_EMPTY
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_gsi_top_gsi_iram_ptr_ch_empty_s
{
  u32 iram_ptr : 12;
  u32 reserved0 : 20;
};

/* Union definition of register */
union ipa_hwio_def_ipa_gsi_top_gsi_iram_ptr_ch_empty_u
{
  struct ipa_hwio_def_ipa_gsi_top_gsi_iram_ptr_ch_empty_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_GSI_TOP_GSI_IRAM_PTR_EVENT_GEN_COMP
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_gsi_top_gsi_iram_ptr_event_gen_comp_s
{
  u32 iram_ptr : 12;
  u32 reserved0 : 20;
};

/* Union definition of register */
union ipa_hwio_def_ipa_gsi_top_gsi_iram_ptr_event_gen_comp_u
{
  struct ipa_hwio_def_ipa_gsi_top_gsi_iram_ptr_event_gen_comp_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_GSI_TOP_GSI_IRAM_PTR_PERIPH_IF_TLV_IN_0
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_gsi_top_gsi_iram_ptr_periph_if_tlv_in_0_s
{
  u32 iram_ptr : 12;
  u32 reserved0 : 20;
};

/* Union definition of register */
union ipa_hwio_def_ipa_gsi_top_gsi_iram_ptr_periph_if_tlv_in_0_u
{
  struct ipa_hwio_def_ipa_gsi_top_gsi_iram_ptr_periph_if_tlv_in_0_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_GSI_TOP_GSI_IRAM_PTR_PERIPH_IF_TLV_IN_2
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_gsi_top_gsi_iram_ptr_periph_if_tlv_in_2_s
{
  u32 iram_ptr : 12;
  u32 reserved0 : 20;
};

/* Union definition of register */
union ipa_hwio_def_ipa_gsi_top_gsi_iram_ptr_periph_if_tlv_in_2_u
{
  struct ipa_hwio_def_ipa_gsi_top_gsi_iram_ptr_periph_if_tlv_in_2_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_GSI_TOP_GSI_IRAM_PTR_PERIPH_IF_TLV_IN_1
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_gsi_top_gsi_iram_ptr_periph_if_tlv_in_1_s
{
  u32 iram_ptr : 12;
  u32 reserved0 : 20;
};

/* Union definition of register */
union ipa_hwio_def_ipa_gsi_top_gsi_iram_ptr_periph_if_tlv_in_1_u
{
  struct ipa_hwio_def_ipa_gsi_top_gsi_iram_ptr_periph_if_tlv_in_1_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_GSI_TOP_GSI_IRAM_PTR_TIMER_EXPIRED
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_gsi_top_gsi_iram_ptr_timer_expired_s
{
  u32 iram_ptr : 12;
  u32 reserved0 : 20;
};

/* Union definition of register */
union ipa_hwio_def_ipa_gsi_top_gsi_iram_ptr_timer_expired_u
{
  struct ipa_hwio_def_ipa_gsi_top_gsi_iram_ptr_timer_expired_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_GSI_TOP_GSI_IRAM_PTR_WRITE_ENG_COMP
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_gsi_top_gsi_iram_ptr_write_eng_comp_s
{
  u32 iram_ptr : 12;
  u32 reserved0 : 20;
};

/* Union definition of register */
union ipa_hwio_def_ipa_gsi_top_gsi_iram_ptr_write_eng_comp_u
{
  struct ipa_hwio_def_ipa_gsi_top_gsi_iram_ptr_write_eng_comp_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_GSI_TOP_GSI_IRAM_PTR_READ_ENG_COMP
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_gsi_top_gsi_iram_ptr_read_eng_comp_s
{
  u32 iram_ptr : 12;
  u32 reserved0 : 20;
};

/* Union definition of register */
union ipa_hwio_def_ipa_gsi_top_gsi_iram_ptr_read_eng_comp_u
{
  struct ipa_hwio_def_ipa_gsi_top_gsi_iram_ptr_read_eng_comp_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_GSI_TOP_GSI_IRAM_PTR_UC_GP_INT
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_gsi_top_gsi_iram_ptr_uc_gp_int_s
{
  u32 iram_ptr : 12;
  u32 reserved0 : 20;
};

/* Union definition of register */
union ipa_hwio_def_ipa_gsi_top_gsi_iram_ptr_uc_gp_int_u
{
  struct ipa_hwio_def_ipa_gsi_top_gsi_iram_ptr_uc_gp_int_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_GSI_TOP_GSI_IRAM_PTR_INT_MOD_STOPED
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_gsi_top_gsi_iram_ptr_int_mod_stoped_s
{
  u32 iram_ptr : 12;
  u32 reserved0 : 20;
};

/* Union definition of register */
union ipa_hwio_def_ipa_gsi_top_gsi_iram_ptr_int_mod_stoped_u
{
  struct ipa_hwio_def_ipa_gsi_top_gsi_iram_ptr_int_mod_stoped_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_GSI_TOP_GSI_IRAM_PTR_INT_NOTIFY_MCS
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_gsi_top_gsi_iram_ptr_int_notify_mcs_s
{
  u32 iram_ptr : 12;
  u32 reserved0 : 20;
};

/* Union definition of register */
union ipa_hwio_def_ipa_gsi_top_gsi_iram_ptr_int_notify_mcs_u
{
  struct ipa_hwio_def_ipa_gsi_top_gsi_iram_ptr_int_notify_mcs_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_GSI_TOP_GSI_INST_RAM_n
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_gsi_top_gsi_inst_ram_n_s
{
  u32 inst_byte_0 : 8;
  u32 inst_byte_1 : 8;
  u32 inst_byte_2 : 8;
  u32 inst_byte_3 : 8;
};

/* Union definition of register */
union ipa_hwio_def_ipa_gsi_top_gsi_inst_ram_n_u
{
  struct ipa_hwio_def_ipa_gsi_top_gsi_inst_ram_n_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_GSI_TOP_GSI_SHRAM_n
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_gsi_top_gsi_shram_n_s
{
  u32 shram : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_gsi_top_gsi_shram_n_u
{
  struct ipa_hwio_def_ipa_gsi_top_gsi_shram_n_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_GSI_TOP_GSI_MAP_EE_n_CH_k_VP_TABLE
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_gsi_top_gsi_map_ee_n_ch_k_vp_table_s
{
  u32 phy_ch : 8;
  u32 valid : 1;
  u32 reserved0 : 23;
};

/* Union definition of register */
union ipa_hwio_def_ipa_gsi_top_gsi_map_ee_n_ch_k_vp_table_u
{
  struct ipa_hwio_def_ipa_gsi_top_gsi_map_ee_n_ch_k_vp_table_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_GSI_TOP_GSI_TEST_BUS_SEL
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_gsi_top_gsi_test_bus_sel_s
{
  u32 gsi_testbus_sel : 8;
  u32 reserved0 : 8;
  u32 gsi_hw_events_sel : 4;
  u32 reserved1 : 12;
};

/* Union definition of register */
union ipa_hwio_def_ipa_gsi_top_gsi_test_bus_sel_u
{
  struct ipa_hwio_def_ipa_gsi_top_gsi_test_bus_sel_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_GSI_TOP_GSI_TEST_BUS_REG
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_gsi_top_gsi_test_bus_reg_s
{
  u32 gsi_testbus_reg : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_gsi_top_gsi_test_bus_reg_u
{
  struct ipa_hwio_def_ipa_gsi_top_gsi_test_bus_reg_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_GSI_TOP_GSI_DEBUG_BUSY_REG
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_gsi_top_gsi_debug_busy_reg_s
{
  u32 csr_busy : 1;
  u32 ree_busy : 1;
  u32 mcs_busy : 1;
  u32 timer_busy : 1;
  u32 rd_wr_busy : 1;
  u32 ev_eng_busy : 1;
  u32 int_eng_busy : 1;
  u32 ree_pwr_clps_busy : 1;
  u32 db_eng_busy : 1;
  u32 dbg_cnt_busy : 1;
  u32 uc_busy : 1;
  u32 ic_busy : 1;
  u32 sdma_busy : 1;
  u32 reserved0 : 19;
};

/* Union definition of register */
union ipa_hwio_def_ipa_gsi_top_gsi_debug_busy_reg_u
{
  struct ipa_hwio_def_ipa_gsi_top_gsi_debug_busy_reg_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_GSI_TOP_GSI_DEBUG_EVENT_PENDING_k
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_gsi_top_gsi_debug_event_pending_k_s
{
  u32 chid_bit_map : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_gsi_top_gsi_debug_event_pending_k_u
{
  struct ipa_hwio_def_ipa_gsi_top_gsi_debug_event_pending_k_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_GSI_TOP_GSI_DEBUG_TIMER_PENDING_k
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_gsi_top_gsi_debug_timer_pending_k_s
{
  u32 chid_bit_map : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_gsi_top_gsi_debug_timer_pending_k_u
{
  struct ipa_hwio_def_ipa_gsi_top_gsi_debug_timer_pending_k_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_GSI_TOP_GSI_DEBUG_RD_WR_PENDING_k
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_gsi_top_gsi_debug_rd_wr_pending_k_s
{
  u32 chid_bit_map : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_gsi_top_gsi_debug_rd_wr_pending_k_u
{
  struct ipa_hwio_def_ipa_gsi_top_gsi_debug_rd_wr_pending_k_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_GSI_TOP_GSI_SPARE_REG_1
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_gsi_top_gsi_spare_reg_1_s
{
  u32 fix_ieob_wrong_msk_disable : 1;
  u32 reserved0 : 31;
};

/* Union definition of register */
union ipa_hwio_def_ipa_gsi_top_gsi_spare_reg_1_u
{
  struct ipa_hwio_def_ipa_gsi_top_gsi_spare_reg_1_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_GSI_TOP_GSI_DEBUG_PC_FROM_SW
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_gsi_top_gsi_debug_pc_from_sw_s
{
  u32 iram_ptr : 12;
  u32 reserved0 : 20;
};

/* Union definition of register */
union ipa_hwio_def_ipa_gsi_top_gsi_debug_pc_from_sw_u
{
  struct ipa_hwio_def_ipa_gsi_top_gsi_debug_pc_from_sw_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_GSI_TOP_GSI_DEBUG_SW_STALL
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_gsi_top_gsi_debug_sw_stall_s
{
  u32 mcs_stall : 1;
  u32 reserved0 : 31;
};

/* Union definition of register */
union ipa_hwio_def_ipa_gsi_top_gsi_debug_sw_stall_u
{
  struct ipa_hwio_def_ipa_gsi_top_gsi_debug_sw_stall_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_GSI_TOP_GSI_DEBUG_PC_FOR_DEBUG
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_gsi_top_gsi_debug_pc_for_debug_s
{
  u32 iram_ptr : 12;
  u32 reserved0 : 20;
};

/* Union definition of register */
union ipa_hwio_def_ipa_gsi_top_gsi_debug_pc_for_debug_u
{
  struct ipa_hwio_def_ipa_gsi_top_gsi_debug_pc_for_debug_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_GSI_TOP_GSI_DEBUG_QSB_LOG_SEL
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_gsi_top_gsi_debug_qsb_log_sel_s
{
  u32 sel_write : 1;
  u32 reserved0 : 7;
  u32 sel_tid : 8;
  u32 sel_mid : 8;
  u32 reserved1 : 8;
};

/* Union definition of register */
union ipa_hwio_def_ipa_gsi_top_gsi_debug_qsb_log_sel_u
{
  struct ipa_hwio_def_ipa_gsi_top_gsi_debug_qsb_log_sel_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_GSI_TOP_GSI_DEBUG_QSB_LOG_CLR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_gsi_top_gsi_debug_qsb_log_clr_s
{
  u32 log_clr : 1;
  u32 reserved0 : 31;
};

/* Union definition of register */
union ipa_hwio_def_ipa_gsi_top_gsi_debug_qsb_log_clr_u
{
  struct ipa_hwio_def_ipa_gsi_top_gsi_debug_qsb_log_clr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_GSI_TOP_GSI_DEBUG_QSB_LOG_ERR_TRNS_ID
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_gsi_top_gsi_debug_qsb_log_err_trns_id_s
{
  u32 err_write : 1;
  u32 reserved0 : 7;
  u32 err_tid : 8;
  u32 err_mid : 8;
  u32 err_saved : 1;
  u32 reserved1 : 7;
};

/* Union definition of register */
union ipa_hwio_def_ipa_gsi_top_gsi_debug_qsb_log_err_trns_id_u
{
  struct ipa_hwio_def_ipa_gsi_top_gsi_debug_qsb_log_err_trns_id_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_GSI_TOP_GSI_DEBUG_QSB_LOG_0
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_gsi_top_gsi_debug_qsb_log_0_s
{
  u32 addr_31_0 : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_gsi_top_gsi_debug_qsb_log_0_u
{
  struct ipa_hwio_def_ipa_gsi_top_gsi_debug_qsb_log_0_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_GSI_TOP_GSI_DEBUG_QSB_LOG_1
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_gsi_top_gsi_debug_qsb_log_1_s
{
  u32 addr_43_32 : 12;
  u32 ainnershared : 1;
  u32 anoallocate : 1;
  u32 ashared : 1;
  u32 acacheable : 1;
  u32 atransient : 1;
  u32 aooord : 1;
  u32 aooowr : 1;
  u32 reserved0 : 1;
  u32 alen : 4;
  u32 asize : 4;
  u32 areqpriority : 4;
};

/* Union definition of register */
union ipa_hwio_def_ipa_gsi_top_gsi_debug_qsb_log_1_u
{
  struct ipa_hwio_def_ipa_gsi_top_gsi_debug_qsb_log_1_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_GSI_TOP_GSI_DEBUG_QSB_LOG_2
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_gsi_top_gsi_debug_qsb_log_2_s
{
  u32 ammusid : 12;
  u32 amemtype : 4;
  u32 reserved0 : 16;
};

/* Union definition of register */
union ipa_hwio_def_ipa_gsi_top_gsi_debug_qsb_log_2_u
{
  struct ipa_hwio_def_ipa_gsi_top_gsi_debug_qsb_log_2_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_GSI_TOP_GSI_DEBUG_QSB_LOG_LAST_MISC_IDn
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_gsi_top_gsi_debug_qsb_log_last_misc_idn_s
{
  u32 addr_20_0 : 21;
  u32 write : 1;
  u32 tid : 5;
  u32 mid : 5;
};

/* Union definition of register */
union ipa_hwio_def_ipa_gsi_top_gsi_debug_qsb_log_last_misc_idn_u
{
  struct ipa_hwio_def_ipa_gsi_top_gsi_debug_qsb_log_last_misc_idn_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_GSI_TOP_GSI_DEBUG_SW_RF_n_WRITE
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_gsi_top_gsi_debug_sw_rf_n_write_s
{
  u32 data_in : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_gsi_top_gsi_debug_sw_rf_n_write_u
{
  struct ipa_hwio_def_ipa_gsi_top_gsi_debug_sw_rf_n_write_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_GSI_TOP_GSI_DEBUG_SW_RF_n_READ
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_gsi_top_gsi_debug_sw_rf_n_read_s
{
  u32 rf_reg : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_gsi_top_gsi_debug_sw_rf_n_read_u
{
  struct ipa_hwio_def_ipa_gsi_top_gsi_debug_sw_rf_n_read_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_GSI_TOP_GSI_DEBUG_COUNTER_CFGn
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_gsi_top_gsi_debug_counter_cfgn_s
{
  u32 enable : 1;
  u32 stop_at_wrap_arnd : 1;
  u32 clr_at_read : 1;
  u32 evnt_type : 5;
  u32 ee : 4;
  u32 virtual_chnl : 8;
  u32 chain : 1;
  u32 reserved0 : 11;
};

/* Union definition of register */
union ipa_hwio_def_ipa_gsi_top_gsi_debug_counter_cfgn_u
{
  struct ipa_hwio_def_ipa_gsi_top_gsi_debug_counter_cfgn_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_GSI_TOP_GSI_DEBUG_COUNTERn
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_gsi_top_gsi_debug_countern_s
{
  u32 counter_value : 16;
  u32 reserved0 : 16;
};

/* Union definition of register */
union ipa_hwio_def_ipa_gsi_top_gsi_debug_countern_u
{
  struct ipa_hwio_def_ipa_gsi_top_gsi_debug_countern_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_GSI_TOP_GSI_DEBUG_SW_MSK_REG_n_SEC_k_WR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_gsi_top_gsi_debug_sw_msk_reg_n_sec_k_wr_s
{
  u32 data_in : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_gsi_top_gsi_debug_sw_msk_reg_n_sec_k_wr_u
{
  struct ipa_hwio_def_ipa_gsi_top_gsi_debug_sw_msk_reg_n_sec_k_wr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_GSI_TOP_GSI_DEBUG_SW_MSK_REG_n_SEC_k_RD
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_gsi_top_gsi_debug_sw_msk_reg_n_sec_k_rd_s
{
  u32 msk_reg : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_gsi_top_gsi_debug_sw_msk_reg_n_sec_k_rd_u
{
  struct ipa_hwio_def_ipa_gsi_top_gsi_debug_sw_msk_reg_n_sec_k_rd_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_GSI_TOP_GSI_DEBUG_EE_n_CH_k_VP_TABLE
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_gsi_top_gsi_debug_ee_n_ch_k_vp_table_s
{
  u32 phy_ch : 8;
  u32 valid : 1;
  u32 reserved0 : 23;
};

/* Union definition of register */
union ipa_hwio_def_ipa_gsi_top_gsi_debug_ee_n_ch_k_vp_table_u
{
  struct ipa_hwio_def_ipa_gsi_top_gsi_debug_ee_n_ch_k_vp_table_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_GSI_TOP_GSI_DEBUG_EE_n_EV_k_VP_TABLE
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_gsi_top_gsi_debug_ee_n_ev_k_vp_table_s
{
  u32 phy_ev_ch : 8;
  u32 valid : 1;
  u32 reserved0 : 23;
};

/* Union definition of register */
union ipa_hwio_def_ipa_gsi_top_gsi_debug_ee_n_ev_k_vp_table_u
{
  struct ipa_hwio_def_ipa_gsi_top_gsi_debug_ee_n_ev_k_vp_table_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_GSI_TOP_GSI_DEBUG_REE_PREFETCH_BUF_CH_ID
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_gsi_top_gsi_debug_ree_prefetch_buf_ch_id_s
{
  u32 prefetch_buf_ch_id : 8;
  u32 reserved0 : 24;
};

/* Union definition of register */
union ipa_hwio_def_ipa_gsi_top_gsi_debug_ree_prefetch_buf_ch_id_u
{
  struct ipa_hwio_def_ipa_gsi_top_gsi_debug_ree_prefetch_buf_ch_id_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_GSI_TOP_GSI_DEBUG_REE_PREFETCH_BUF_STATUS
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_gsi_top_gsi_debug_ree_prefetch_buf_status_s
{
  u32 prefetch_buf_status : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_gsi_top_gsi_debug_ree_prefetch_buf_status_u
{
  struct ipa_hwio_def_ipa_gsi_top_gsi_debug_ree_prefetch_buf_status_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_GSI_TOP_GSI_MCS_PROFILING_BP_CNT_LSB
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_gsi_top_gsi_mcs_profiling_bp_cnt_lsb_s
{
  u32 bp_cnt_lsb : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_gsi_top_gsi_mcs_profiling_bp_cnt_lsb_u
{
  struct ipa_hwio_def_ipa_gsi_top_gsi_mcs_profiling_bp_cnt_lsb_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_GSI_TOP_GSI_MCS_PROFILING_BP_CNT_MSB
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_gsi_top_gsi_mcs_profiling_bp_cnt_msb_s
{
  u32 bp_cnt_msb : 4;
  u32 reserved0 : 28;
};

/* Union definition of register */
union ipa_hwio_def_ipa_gsi_top_gsi_mcs_profiling_bp_cnt_msb_u
{
  struct ipa_hwio_def_ipa_gsi_top_gsi_mcs_profiling_bp_cnt_msb_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_GSI_TOP_GSI_MCS_PROFILING_BP_AND_PENDING_CNT_LSB
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_gsi_top_gsi_mcs_profiling_bp_and_pending_cnt_lsb_s
{
  u32 bp_and_pending_cnt_lsb : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_gsi_top_gsi_mcs_profiling_bp_and_pending_cnt_lsb_u
{
  struct ipa_hwio_def_ipa_gsi_top_gsi_mcs_profiling_bp_and_pending_cnt_lsb_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_GSI_TOP_GSI_MCS_PROFILING_BP_AND_PENDING_CNT_MSB
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_gsi_top_gsi_mcs_profiling_bp_and_pending_cnt_msb_s
{
  u32 bp_and_pending_cnt_msb : 4;
  u32 reserved0 : 28;
};

/* Union definition of register */
union ipa_hwio_def_ipa_gsi_top_gsi_mcs_profiling_bp_and_pending_cnt_msb_u
{
  struct ipa_hwio_def_ipa_gsi_top_gsi_mcs_profiling_bp_and_pending_cnt_msb_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_GSI_TOP_GSI_MCS_PROFILING_MCS_BUSY_CNT_LSB
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_gsi_top_gsi_mcs_profiling_mcs_busy_cnt_lsb_s
{
  u32 mcs_busy_cnt_lsb : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_gsi_top_gsi_mcs_profiling_mcs_busy_cnt_lsb_u
{
  struct ipa_hwio_def_ipa_gsi_top_gsi_mcs_profiling_mcs_busy_cnt_lsb_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_GSI_TOP_GSI_MCS_PROFILING_MCS_BUSY_CNT_MSB
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_gsi_top_gsi_mcs_profiling_mcs_busy_cnt_msb_s
{
  u32 mcs_busy_cnt_msb : 4;
  u32 reserved0 : 28;
};

/* Union definition of register */
union ipa_hwio_def_ipa_gsi_top_gsi_mcs_profiling_mcs_busy_cnt_msb_u
{
  struct ipa_hwio_def_ipa_gsi_top_gsi_mcs_profiling_mcs_busy_cnt_msb_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_GSI_TOP_GSI_MCS_PROFILING_MCS_IDLE_CNT_LSB
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_gsi_top_gsi_mcs_profiling_mcs_idle_cnt_lsb_s
{
  u32 mcs_idle_cnt_lsb : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_gsi_top_gsi_mcs_profiling_mcs_idle_cnt_lsb_u
{
  struct ipa_hwio_def_ipa_gsi_top_gsi_mcs_profiling_mcs_idle_cnt_lsb_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_GSI_TOP_GSI_MCS_PROFILING_MCS_IDLE_CNT_MSB
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_gsi_top_gsi_mcs_profiling_mcs_idle_cnt_msb_s
{
  u32 mcs_idle_cnt_msb : 4;
  u32 reserved0 : 28;
};

/* Union definition of register */
union ipa_hwio_def_ipa_gsi_top_gsi_mcs_profiling_mcs_idle_cnt_msb_u
{
  struct ipa_hwio_def_ipa_gsi_top_gsi_mcs_profiling_mcs_idle_cnt_msb_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_GSI_TOP_EE_n_GSI_CH_k_CNTXT_0
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_gsi_top_ee_n_gsi_ch_k_cntxt_0_s
{
  u32 chtype_protocol : 7;
  u32 chtype_dir : 1;
  u32 ee : 4;
  u32 chid : 8;
  u32 chstate : 4;
  u32 element_size : 8;
};

/* Union definition of register */
union ipa_hwio_def_ipa_gsi_top_ee_n_gsi_ch_k_cntxt_0_u
{
  struct ipa_hwio_def_ipa_gsi_top_ee_n_gsi_ch_k_cntxt_0_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_GSI_TOP_EE_n_GSI_CH_k_CNTXT_1
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_gsi_top_ee_n_gsi_ch_k_cntxt_1_s
{
  u32 r_length : 24;
  u32 erindex : 8;
};

/* Union definition of register */
union ipa_hwio_def_ipa_gsi_top_ee_n_gsi_ch_k_cntxt_1_u
{
  struct ipa_hwio_def_ipa_gsi_top_ee_n_gsi_ch_k_cntxt_1_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_GSI_TOP_EE_n_GSI_CH_k_CNTXT_2
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_gsi_top_ee_n_gsi_ch_k_cntxt_2_s
{
  u32 r_base_addr_lsbs : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_gsi_top_ee_n_gsi_ch_k_cntxt_2_u
{
  struct ipa_hwio_def_ipa_gsi_top_ee_n_gsi_ch_k_cntxt_2_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_GSI_TOP_EE_n_GSI_CH_k_CNTXT_3
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_gsi_top_ee_n_gsi_ch_k_cntxt_3_s
{
  u32 r_base_addr_msbs : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_gsi_top_ee_n_gsi_ch_k_cntxt_3_u
{
  struct ipa_hwio_def_ipa_gsi_top_ee_n_gsi_ch_k_cntxt_3_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_GSI_TOP_EE_n_GSI_CH_k_CNTXT_4
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_gsi_top_ee_n_gsi_ch_k_cntxt_4_s
{
  u32 read_ptr_lsb : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_gsi_top_ee_n_gsi_ch_k_cntxt_4_u
{
  struct ipa_hwio_def_ipa_gsi_top_ee_n_gsi_ch_k_cntxt_4_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_GSI_TOP_EE_n_GSI_CH_k_CNTXT_5
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_gsi_top_ee_n_gsi_ch_k_cntxt_5_s
{
  u32 read_ptr_msb : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_gsi_top_ee_n_gsi_ch_k_cntxt_5_u
{
  struct ipa_hwio_def_ipa_gsi_top_ee_n_gsi_ch_k_cntxt_5_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_GSI_TOP_EE_n_GSI_CH_k_CNTXT_6
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_gsi_top_ee_n_gsi_ch_k_cntxt_6_s
{
  u32 write_ptr_lsb : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_gsi_top_ee_n_gsi_ch_k_cntxt_6_u
{
  struct ipa_hwio_def_ipa_gsi_top_ee_n_gsi_ch_k_cntxt_6_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_GSI_TOP_EE_n_GSI_CH_k_CNTXT_7
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_gsi_top_ee_n_gsi_ch_k_cntxt_7_s
{
  u32 write_ptr_msb : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_gsi_top_ee_n_gsi_ch_k_cntxt_7_u
{
  struct ipa_hwio_def_ipa_gsi_top_ee_n_gsi_ch_k_cntxt_7_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_GSI_TOP_EE_n_GSI_CH_k_CNTXT_8
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_gsi_top_ee_n_gsi_ch_k_cntxt_8_s
{
  u32 db_msi_data : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_gsi_top_ee_n_gsi_ch_k_cntxt_8_u
{
  struct ipa_hwio_def_ipa_gsi_top_ee_n_gsi_ch_k_cntxt_8_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_GSI_TOP_EE_n_GSI_CH_k_ELEM_SIZE_SHIFT
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_gsi_top_ee_n_gsi_ch_k_elem_size_shift_s
{
  u32 elem_size_shift : 4;
  u32 reserved0 : 28;
};

/* Union definition of register */
union ipa_hwio_def_ipa_gsi_top_ee_n_gsi_ch_k_elem_size_shift_u
{
  struct ipa_hwio_def_ipa_gsi_top_ee_n_gsi_ch_k_elem_size_shift_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_GSI_TOP_EE_n_GSI_CH_k_CH_ALMST_EMPTY_THRSHOLD
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_gsi_top_ee_n_gsi_ch_k_ch_almst_empty_thrshold_s
{
  u32 ch_almst_empty_thrshold : 16;
  u32 reserved0 : 16;
};

/* Union definition of register */
union ipa_hwio_def_ipa_gsi_top_ee_n_gsi_ch_k_ch_almst_empty_thrshold_u
{
  struct ipa_hwio_def_ipa_gsi_top_ee_n_gsi_ch_k_ch_almst_empty_thrshold_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_GSI_TOP_EE_n_GSI_CH_k_RE_FETCH_READ_PTR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_gsi_top_ee_n_gsi_ch_k_re_fetch_read_ptr_s
{
  u32 read_ptr : 24;
  u32 reserved0 : 8;
};

/* Union definition of register */
union ipa_hwio_def_ipa_gsi_top_ee_n_gsi_ch_k_re_fetch_read_ptr_u
{
  struct ipa_hwio_def_ipa_gsi_top_ee_n_gsi_ch_k_re_fetch_read_ptr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_GSI_TOP_EE_n_GSI_CH_k_RE_FETCH_WRITE_PTR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_gsi_top_ee_n_gsi_ch_k_re_fetch_write_ptr_s
{
  u32 re_intr_db : 24;
  u32 reserved0 : 8;
};

/* Union definition of register */
union ipa_hwio_def_ipa_gsi_top_ee_n_gsi_ch_k_re_fetch_write_ptr_u
{
  struct ipa_hwio_def_ipa_gsi_top_ee_n_gsi_ch_k_re_fetch_write_ptr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_GSI_TOP_EE_n_GSI_CH_k_QOS
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_gsi_top_ee_n_gsi_ch_k_qos_s
{
  u32 wrr_weight : 4;
  u32 reserved0 : 4;
  u32 max_prefetch : 1;
  u32 use_db_eng : 1;
  u32 prefetch_mode : 4;
  u32 reserved1 : 2;
  u32 empty_lvl_thrshold : 8;
  u32 db_in_bytes : 1;
  u32 low_latency_en : 1;
  u32 reserved2 : 6;
};

/* Union definition of register */
union ipa_hwio_def_ipa_gsi_top_ee_n_gsi_ch_k_qos_u
{
  struct ipa_hwio_def_ipa_gsi_top_ee_n_gsi_ch_k_qos_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_GSI_TOP_EE_n_GSI_CH_k_SCRATCH_0
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_gsi_top_ee_n_gsi_ch_k_scratch_0_s
{
  u32 scratch : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_gsi_top_ee_n_gsi_ch_k_scratch_0_u
{
  struct ipa_hwio_def_ipa_gsi_top_ee_n_gsi_ch_k_scratch_0_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_GSI_TOP_EE_n_GSI_CH_k_SCRATCH_1
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_gsi_top_ee_n_gsi_ch_k_scratch_1_s
{
  u32 scratch : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_gsi_top_ee_n_gsi_ch_k_scratch_1_u
{
  struct ipa_hwio_def_ipa_gsi_top_ee_n_gsi_ch_k_scratch_1_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_GSI_TOP_EE_n_GSI_CH_k_SCRATCH_2
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_gsi_top_ee_n_gsi_ch_k_scratch_2_s
{
  u32 scratch : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_gsi_top_ee_n_gsi_ch_k_scratch_2_u
{
  struct ipa_hwio_def_ipa_gsi_top_ee_n_gsi_ch_k_scratch_2_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_GSI_TOP_EE_n_GSI_CH_k_SCRATCH_3
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_gsi_top_ee_n_gsi_ch_k_scratch_3_s
{
  u32 scratch : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_gsi_top_ee_n_gsi_ch_k_scratch_3_u
{
  struct ipa_hwio_def_ipa_gsi_top_ee_n_gsi_ch_k_scratch_3_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_GSI_TOP_EE_n_GSI_CH_k_SCRATCH_4
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_gsi_top_ee_n_gsi_ch_k_scratch_4_s
{
  u32 scratch : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_gsi_top_ee_n_gsi_ch_k_scratch_4_u
{
  struct ipa_hwio_def_ipa_gsi_top_ee_n_gsi_ch_k_scratch_4_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_GSI_TOP_EE_n_GSI_CH_k_SCRATCH_5
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_gsi_top_ee_n_gsi_ch_k_scratch_5_s
{
  u32 scratch : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_gsi_top_ee_n_gsi_ch_k_scratch_5_u
{
  struct ipa_hwio_def_ipa_gsi_top_ee_n_gsi_ch_k_scratch_5_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_GSI_TOP_EE_n_GSI_CH_k_SCRATCH_6
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_gsi_top_ee_n_gsi_ch_k_scratch_6_s
{
  u32 scratch : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_gsi_top_ee_n_gsi_ch_k_scratch_6_u
{
  struct ipa_hwio_def_ipa_gsi_top_ee_n_gsi_ch_k_scratch_6_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_GSI_TOP_EE_n_GSI_CH_k_SCRATCH_7
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_gsi_top_ee_n_gsi_ch_k_scratch_7_s
{
  u32 scratch : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_gsi_top_ee_n_gsi_ch_k_scratch_7_u
{
  struct ipa_hwio_def_ipa_gsi_top_ee_n_gsi_ch_k_scratch_7_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_GSI_TOP_EE_n_GSI_CH_k_SCRATCH_8
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_gsi_top_ee_n_gsi_ch_k_scratch_8_s
{
  u32 scratch : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_gsi_top_ee_n_gsi_ch_k_scratch_8_u
{
  struct ipa_hwio_def_ipa_gsi_top_ee_n_gsi_ch_k_scratch_8_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_GSI_TOP_EE_n_GSI_CH_k_SCRATCH_9
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_gsi_top_ee_n_gsi_ch_k_scratch_9_s
{
  u32 scratch : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_gsi_top_ee_n_gsi_ch_k_scratch_9_u
{
  struct ipa_hwio_def_ipa_gsi_top_ee_n_gsi_ch_k_scratch_9_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_GSI_TOP_EE_n_GSI_CH_k_DB_ENG_WRITE_PTR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_gsi_top_ee_n_gsi_ch_k_db_eng_write_ptr_s
{
  u32 last_db_2_mcs : 16;
  u32 reserved0 : 16;
};

/* Union definition of register */
union ipa_hwio_def_ipa_gsi_top_ee_n_gsi_ch_k_db_eng_write_ptr_u
{
  struct ipa_hwio_def_ipa_gsi_top_ee_n_gsi_ch_k_db_eng_write_ptr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_GSI_TOP_EE_n_EV_CH_k_CNTXT_0
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_gsi_top_ee_n_ev_ch_k_cntxt_0_s
{
  u32 chtype : 7;
  u32 intype : 1;
  u32 evchid : 8;
  u32 ee : 4;
  u32 chstate : 4;
  u32 element_size : 8;
};

/* Union definition of register */
union ipa_hwio_def_ipa_gsi_top_ee_n_ev_ch_k_cntxt_0_u
{
  struct ipa_hwio_def_ipa_gsi_top_ee_n_ev_ch_k_cntxt_0_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_GSI_TOP_EE_n_EV_CH_k_CNTXT_1
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_gsi_top_ee_n_ev_ch_k_cntxt_1_s
{
  u32 r_length : 24;
  u32 reserved0 : 8;
};

/* Union definition of register */
union ipa_hwio_def_ipa_gsi_top_ee_n_ev_ch_k_cntxt_1_u
{
  struct ipa_hwio_def_ipa_gsi_top_ee_n_ev_ch_k_cntxt_1_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_GSI_TOP_EE_n_EV_CH_k_CNTXT_2
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_gsi_top_ee_n_ev_ch_k_cntxt_2_s
{
  u32 r_base_addr_lsbs : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_gsi_top_ee_n_ev_ch_k_cntxt_2_u
{
  struct ipa_hwio_def_ipa_gsi_top_ee_n_ev_ch_k_cntxt_2_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_GSI_TOP_EE_n_EV_CH_k_CNTXT_3
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_gsi_top_ee_n_ev_ch_k_cntxt_3_s
{
  u32 r_base_addr_msbs : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_gsi_top_ee_n_ev_ch_k_cntxt_3_u
{
  struct ipa_hwio_def_ipa_gsi_top_ee_n_ev_ch_k_cntxt_3_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_GSI_TOP_EE_n_EV_CH_k_CNTXT_4
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_gsi_top_ee_n_ev_ch_k_cntxt_4_s
{
  u32 read_ptr_lsb : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_gsi_top_ee_n_ev_ch_k_cntxt_4_u
{
  struct ipa_hwio_def_ipa_gsi_top_ee_n_ev_ch_k_cntxt_4_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_GSI_TOP_EE_n_EV_CH_k_CNTXT_5
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_gsi_top_ee_n_ev_ch_k_cntxt_5_s
{
  u32 read_ptr_msb : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_gsi_top_ee_n_ev_ch_k_cntxt_5_u
{
  struct ipa_hwio_def_ipa_gsi_top_ee_n_ev_ch_k_cntxt_5_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_GSI_TOP_EE_n_EV_CH_k_CNTXT_6
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_gsi_top_ee_n_ev_ch_k_cntxt_6_s
{
  u32 write_ptr_lsb : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_gsi_top_ee_n_ev_ch_k_cntxt_6_u
{
  struct ipa_hwio_def_ipa_gsi_top_ee_n_ev_ch_k_cntxt_6_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_GSI_TOP_EE_n_EV_CH_k_CNTXT_7
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_gsi_top_ee_n_ev_ch_k_cntxt_7_s
{
  u32 write_ptr_msb : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_gsi_top_ee_n_ev_ch_k_cntxt_7_u
{
  struct ipa_hwio_def_ipa_gsi_top_ee_n_ev_ch_k_cntxt_7_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_GSI_TOP_EE_n_EV_CH_k_CNTXT_8
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_gsi_top_ee_n_ev_ch_k_cntxt_8_s
{
  u32 int_modt : 16;
  u32 int_modc : 8;
  u32 int_mod_cnt : 8;
};

/* Union definition of register */
union ipa_hwio_def_ipa_gsi_top_ee_n_ev_ch_k_cntxt_8_u
{
  struct ipa_hwio_def_ipa_gsi_top_ee_n_ev_ch_k_cntxt_8_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_GSI_TOP_EE_n_EV_CH_k_CNTXT_9
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_gsi_top_ee_n_ev_ch_k_cntxt_9_s
{
  u32 intvec : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_gsi_top_ee_n_ev_ch_k_cntxt_9_u
{
  struct ipa_hwio_def_ipa_gsi_top_ee_n_ev_ch_k_cntxt_9_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_GSI_TOP_EE_n_EV_CH_k_CNTXT_10
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_gsi_top_ee_n_ev_ch_k_cntxt_10_s
{
  u32 msi_addr_lsb : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_gsi_top_ee_n_ev_ch_k_cntxt_10_u
{
  struct ipa_hwio_def_ipa_gsi_top_ee_n_ev_ch_k_cntxt_10_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_GSI_TOP_EE_n_EV_CH_k_CNTXT_11
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_gsi_top_ee_n_ev_ch_k_cntxt_11_s
{
  u32 msi_addr_msb : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_gsi_top_ee_n_ev_ch_k_cntxt_11_u
{
  struct ipa_hwio_def_ipa_gsi_top_ee_n_ev_ch_k_cntxt_11_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_GSI_TOP_EE_n_EV_CH_k_CNTXT_12
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_gsi_top_ee_n_ev_ch_k_cntxt_12_s
{
  u32 rp_update_addr_lsb : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_gsi_top_ee_n_ev_ch_k_cntxt_12_u
{
  struct ipa_hwio_def_ipa_gsi_top_ee_n_ev_ch_k_cntxt_12_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_GSI_TOP_EE_n_EV_CH_k_CNTXT_13
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_gsi_top_ee_n_ev_ch_k_cntxt_13_s
{
  u32 rp_update_addr_msb : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_gsi_top_ee_n_ev_ch_k_cntxt_13_u
{
  struct ipa_hwio_def_ipa_gsi_top_ee_n_ev_ch_k_cntxt_13_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_GSI_TOP_EE_n_EV_CH_k_ELEM_SIZE_SHIFT
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_gsi_top_ee_n_ev_ch_k_elem_size_shift_s
{
  u32 elem_size_shift : 4;
  u32 reserved0 : 28;
};

/* Union definition of register */
union ipa_hwio_def_ipa_gsi_top_ee_n_ev_ch_k_elem_size_shift_u
{
  struct ipa_hwio_def_ipa_gsi_top_ee_n_ev_ch_k_elem_size_shift_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_GSI_TOP_EE_n_EV_CH_k_SCRATCH_0
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_gsi_top_ee_n_ev_ch_k_scratch_0_s
{
  u32 scratch : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_gsi_top_ee_n_ev_ch_k_scratch_0_u
{
  struct ipa_hwio_def_ipa_gsi_top_ee_n_ev_ch_k_scratch_0_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_GSI_TOP_EE_n_EV_CH_k_SCRATCH_1
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_gsi_top_ee_n_ev_ch_k_scratch_1_s
{
  u32 scratch : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_gsi_top_ee_n_ev_ch_k_scratch_1_u
{
  struct ipa_hwio_def_ipa_gsi_top_ee_n_ev_ch_k_scratch_1_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_GSI_TOP_EE_n_EV_CH_k_SCRATCH_2
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_gsi_top_ee_n_ev_ch_k_scratch_2_s
{
  u32 scratch : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_gsi_top_ee_n_ev_ch_k_scratch_2_u
{
  struct ipa_hwio_def_ipa_gsi_top_ee_n_ev_ch_k_scratch_2_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_GSI_TOP_EE_n_GSI_CH_k_DOORBELL_0
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_gsi_top_ee_n_gsi_ch_k_doorbell_0_s
{
  u32 write_ptr_lsb : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_gsi_top_ee_n_gsi_ch_k_doorbell_0_u
{
  struct ipa_hwio_def_ipa_gsi_top_ee_n_gsi_ch_k_doorbell_0_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_GSI_TOP_EE_n_GSI_CH_k_DOORBELL_1
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_gsi_top_ee_n_gsi_ch_k_doorbell_1_s
{
  u32 write_ptr_msb : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_gsi_top_ee_n_gsi_ch_k_doorbell_1_u
{
  struct ipa_hwio_def_ipa_gsi_top_ee_n_gsi_ch_k_doorbell_1_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_GSI_TOP_EE_n_EV_CH_k_DOORBELL_0
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_gsi_top_ee_n_ev_ch_k_doorbell_0_s
{
  u32 write_ptr_lsb : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_gsi_top_ee_n_ev_ch_k_doorbell_0_u
{
  struct ipa_hwio_def_ipa_gsi_top_ee_n_ev_ch_k_doorbell_0_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_GSI_TOP_EE_n_EV_CH_k_DOORBELL_1
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_gsi_top_ee_n_ev_ch_k_doorbell_1_s
{
  u32 write_ptr_msb : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_gsi_top_ee_n_ev_ch_k_doorbell_1_u
{
  struct ipa_hwio_def_ipa_gsi_top_ee_n_ev_ch_k_doorbell_1_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_GSI_TOP_EE_n_GSI_STATUS
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_gsi_top_ee_n_gsi_status_s
{
  u32 enabled : 1;
  u32 reserved0 : 31;
};

/* Union definition of register */
union ipa_hwio_def_ipa_gsi_top_ee_n_gsi_status_u
{
  struct ipa_hwio_def_ipa_gsi_top_ee_n_gsi_status_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_GSI_TOP_EE_n_GSI_CH_CMD
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_gsi_top_ee_n_gsi_ch_cmd_s
{
  u32 chid : 8;
  u32 reserved0 : 16;
  u32 opcode : 8;
};

/* Union definition of register */
union ipa_hwio_def_ipa_gsi_top_ee_n_gsi_ch_cmd_u
{
  struct ipa_hwio_def_ipa_gsi_top_ee_n_gsi_ch_cmd_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_GSI_TOP_EE_n_EV_CH_CMD
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_gsi_top_ee_n_ev_ch_cmd_s
{
  u32 chid : 8;
  u32 reserved0 : 16;
  u32 opcode : 8;
};

/* Union definition of register */
union ipa_hwio_def_ipa_gsi_top_ee_n_ev_ch_cmd_u
{
  struct ipa_hwio_def_ipa_gsi_top_ee_n_ev_ch_cmd_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_GSI_TOP_EE_n_GSI_EE_GENERIC_CMD
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_gsi_top_ee_n_gsi_ee_generic_cmd_s
{
  u32 opcode : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_gsi_top_ee_n_gsi_ee_generic_cmd_u
{
  struct ipa_hwio_def_ipa_gsi_top_ee_n_gsi_ee_generic_cmd_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_GSI_TOP_EE_n_GSI_HW_PARAM_0
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_gsi_top_ee_n_gsi_hw_param_0_s
{
  u32 gsi_ev_ch_num : 8;
  u32 gsi_ch_num : 8;
  u32 num_ees : 5;
  u32 periph_conf_addr_bus_w : 5;
  u32 periph_sec_grp : 5;
  u32 use_axi_m : 1;
};

/* Union definition of register */
union ipa_hwio_def_ipa_gsi_top_ee_n_gsi_hw_param_0_u
{
  struct ipa_hwio_def_ipa_gsi_top_ee_n_gsi_hw_param_0_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_GSI_TOP_EE_n_GSI_HW_PARAM_1
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_gsi_top_ee_n_gsi_hw_param_1_s
{
  u32 gsi_m_data_bus_w : 8;
  u32 gsi_num_qad : 4;
  u32 gsi_nonsec_en : 4;
  u32 gsi_sec_en : 1;
  u32 gsi_vmidacr_en : 1;
  u32 gsi_qrib_en : 1;
  u32 gsi_use_xpu : 1;
  u32 gsi_num_timers : 5;
  u32 gsi_use_bp_mtrix : 1;
  u32 gsi_use_db_eng : 1;
  u32 gsi_use_uc_if : 1;
  u32 gsi_escape_buf_only : 1;
  u32 gsi_simple_rd_wr : 1;
  u32 gsi_blk_int_access_region_1_en : 1;
  u32 gsi_blk_int_access_region_2_en : 1;
};

/* Union definition of register */
union ipa_hwio_def_ipa_gsi_top_ee_n_gsi_hw_param_1_u
{
  struct ipa_hwio_def_ipa_gsi_top_ee_n_gsi_hw_param_1_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_GSI_TOP_EE_n_GSI_HW_PARAM_2
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_gsi_top_ee_n_gsi_hw_param_2_s
{
  u32 gsi_num_ch_per_ee : 8;
  u32 gsi_iram_size : 5;
  u32 gsi_ch_pend_translate : 1;
  u32 gsi_ch_full_logic : 1;
  u32 gsi_use_sdma : 1;
  u32 gsi_sdma_n_int : 3;
  u32 gsi_sdma_max_burst : 8;
  u32 gsi_sdma_n_iovec : 3;
  u32 gsi_use_rd_wr_eng : 1;
  u32 gsi_use_inter_ee : 1;
};

/* Union definition of register */
union ipa_hwio_def_ipa_gsi_top_ee_n_gsi_hw_param_2_u
{
  struct ipa_hwio_def_ipa_gsi_top_ee_n_gsi_hw_param_2_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_GSI_TOP_EE_n_GSI_SW_VERSION
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_gsi_top_ee_n_gsi_sw_version_s
{
  u32 step : 16;
  u32 minor : 12;
  u32 major : 4;
};

/* Union definition of register */
union ipa_hwio_def_ipa_gsi_top_ee_n_gsi_sw_version_u
{
  struct ipa_hwio_def_ipa_gsi_top_ee_n_gsi_sw_version_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_GSI_TOP_EE_n_GSI_MCS_CODE_VER
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_gsi_top_ee_n_gsi_mcs_code_ver_s
{
  u32 ver : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_gsi_top_ee_n_gsi_mcs_code_ver_u
{
  struct ipa_hwio_def_ipa_gsi_top_ee_n_gsi_mcs_code_ver_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_GSI_TOP_EE_n_GSI_HW_PARAM_3
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_gsi_top_ee_n_gsi_hw_param_3_s
{
  u32 gsi_sdma_max_os_rd : 4;
  u32 gsi_sdma_max_os_wr : 4;
  u32 gsi_num_prefetch_bufs : 4;
  u32 gsi_m_addr_bus_w : 8;
  u32 gsi_ree_max_burst_len : 5;
  u32 gsi_use_irom : 1;
  u32 gsi_use_vir_ch_if : 1;
  u32 gsi_use_sleep_clk_div : 1;
  u32 gsi_use_db_msi_mode : 1;
  u32 reserved0 : 3;
};

/* Union definition of register */
union ipa_hwio_def_ipa_gsi_top_ee_n_gsi_hw_param_3_u
{
  struct ipa_hwio_def_ipa_gsi_top_ee_n_gsi_hw_param_3_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_GSI_TOP_EE_n_GSI_HW_PARAM_4
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_gsi_top_ee_n_gsi_hw_param_4_s
{
  u32 gsi_num_ev_per_ee : 8;
  u32 gsi_iram_protcol_cnt : 8;
  u32 reserved0 : 16;
};

/* Union definition of register */
union ipa_hwio_def_ipa_gsi_top_ee_n_gsi_hw_param_4_u
{
  struct ipa_hwio_def_ipa_gsi_top_ee_n_gsi_hw_param_4_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_GSI_TOP_EE_n_CNTXT_TYPE_IRQ
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_gsi_top_ee_n_cntxt_type_irq_s
{
  u32 ch_ctrl : 1;
  u32 ev_ctrl : 1;
  u32 glob_ee : 1;
  u32 ieob : 1;
  u32 inter_ee_ch_ctrl : 1;
  u32 inter_ee_ev_ctrl : 1;
  u32 general : 1;
  u32 reserved0 : 25;
};

/* Union definition of register */
union ipa_hwio_def_ipa_gsi_top_ee_n_cntxt_type_irq_u
{
  struct ipa_hwio_def_ipa_gsi_top_ee_n_cntxt_type_irq_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_GSI_TOP_EE_n_CNTXT_TYPE_IRQ_MSK
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_gsi_top_ee_n_cntxt_type_irq_msk_s
{
  u32 ch_ctrl : 1;
  u32 ev_ctrl : 1;
  u32 glob_ee : 1;
  u32 ieob : 1;
  u32 inter_ee_ch_ctrl : 1;
  u32 inter_ee_ev_ctrl : 1;
  u32 general : 1;
  u32 reserved0 : 25;
};

/* Union definition of register */
union ipa_hwio_def_ipa_gsi_top_ee_n_cntxt_type_irq_msk_u
{
  struct ipa_hwio_def_ipa_gsi_top_ee_n_cntxt_type_irq_msk_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_GSI_TOP_EE_n_CNTXT_SRC_GSI_CH_IRQ_k
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_gsi_top_ee_n_cntxt_src_gsi_ch_irq_k_s
{
  u32 gsi_ch_bit_map : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_gsi_top_ee_n_cntxt_src_gsi_ch_irq_k_u
{
  struct ipa_hwio_def_ipa_gsi_top_ee_n_cntxt_src_gsi_ch_irq_k_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_GSI_TOP_EE_n_CNTXT_SRC_GSI_CH_IRQ_MSK_k
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_gsi_top_ee_n_cntxt_src_gsi_ch_irq_msk_k_s
{
  u32 gsi_ch_bit_map_msk : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_gsi_top_ee_n_cntxt_src_gsi_ch_irq_msk_k_u
{
  struct ipa_hwio_def_ipa_gsi_top_ee_n_cntxt_src_gsi_ch_irq_msk_k_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_GSI_TOP_EE_n_CNTXT_SRC_GSI_CH_IRQ_CLR_k
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_gsi_top_ee_n_cntxt_src_gsi_ch_irq_clr_k_s
{
  u32 gsi_ch_bit_map : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_gsi_top_ee_n_cntxt_src_gsi_ch_irq_clr_k_u
{
  struct ipa_hwio_def_ipa_gsi_top_ee_n_cntxt_src_gsi_ch_irq_clr_k_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_GSI_TOP_EE_n_CNTXT_SRC_EV_CH_IRQ_k
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_gsi_top_ee_n_cntxt_src_ev_ch_irq_k_s
{
  u32 ev_ch_bit_map : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_gsi_top_ee_n_cntxt_src_ev_ch_irq_k_u
{
  struct ipa_hwio_def_ipa_gsi_top_ee_n_cntxt_src_ev_ch_irq_k_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_GSI_TOP_EE_n_CNTXT_SRC_EV_CH_IRQ_MSK_k
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_gsi_top_ee_n_cntxt_src_ev_ch_irq_msk_k_s
{
  u32 ev_ch_bit_map_msk : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_gsi_top_ee_n_cntxt_src_ev_ch_irq_msk_k_u
{
  struct ipa_hwio_def_ipa_gsi_top_ee_n_cntxt_src_ev_ch_irq_msk_k_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_GSI_TOP_EE_n_CNTXT_SRC_EV_CH_IRQ_CLR_k
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_gsi_top_ee_n_cntxt_src_ev_ch_irq_clr_k_s
{
  u32 ev_ch_bit_map : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_gsi_top_ee_n_cntxt_src_ev_ch_irq_clr_k_u
{
  struct ipa_hwio_def_ipa_gsi_top_ee_n_cntxt_src_ev_ch_irq_clr_k_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_GSI_TOP_EE_n_CNTXT_SRC_IEOB_IRQ_k
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_gsi_top_ee_n_cntxt_src_ieob_irq_k_s
{
  u32 ev_ch_bit_map : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_gsi_top_ee_n_cntxt_src_ieob_irq_k_u
{
  struct ipa_hwio_def_ipa_gsi_top_ee_n_cntxt_src_ieob_irq_k_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_GSI_TOP_EE_n_CNTXT_SRC_IEOB_IRQ_MSK_k
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_gsi_top_ee_n_cntxt_src_ieob_irq_msk_k_s
{
  u32 ev_ch_bit_map_msk : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_gsi_top_ee_n_cntxt_src_ieob_irq_msk_k_u
{
  struct ipa_hwio_def_ipa_gsi_top_ee_n_cntxt_src_ieob_irq_msk_k_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_GSI_TOP_EE_n_CNTXT_SRC_IEOB_IRQ_CLR_k
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_gsi_top_ee_n_cntxt_src_ieob_irq_clr_k_s
{
  u32 ev_ch_bit_map : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_gsi_top_ee_n_cntxt_src_ieob_irq_clr_k_u
{
  struct ipa_hwio_def_ipa_gsi_top_ee_n_cntxt_src_ieob_irq_clr_k_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_GSI_TOP_EE_n_CNTXT_GLOB_IRQ_STTS
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_gsi_top_ee_n_cntxt_glob_irq_stts_s
{
  u32 error_int : 1;
  u32 gp_int1 : 1;
  u32 gp_int2 : 1;
  u32 gp_int3 : 1;
  u32 reserved0 : 28;
};

/* Union definition of register */
union ipa_hwio_def_ipa_gsi_top_ee_n_cntxt_glob_irq_stts_u
{
  struct ipa_hwio_def_ipa_gsi_top_ee_n_cntxt_glob_irq_stts_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_GSI_TOP_EE_n_CNTXT_GLOB_IRQ_EN
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_gsi_top_ee_n_cntxt_glob_irq_en_s
{
  u32 error_int : 1;
  u32 gp_int1 : 1;
  u32 gp_int2 : 1;
  u32 gp_int3 : 1;
  u32 reserved0 : 28;
};

/* Union definition of register */
union ipa_hwio_def_ipa_gsi_top_ee_n_cntxt_glob_irq_en_u
{
  struct ipa_hwio_def_ipa_gsi_top_ee_n_cntxt_glob_irq_en_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_GSI_TOP_EE_n_CNTXT_GLOB_IRQ_CLR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_gsi_top_ee_n_cntxt_glob_irq_clr_s
{
  u32 error_int : 1;
  u32 gp_int1 : 1;
  u32 gp_int2 : 1;
  u32 gp_int3 : 1;
  u32 reserved0 : 28;
};

/* Union definition of register */
union ipa_hwio_def_ipa_gsi_top_ee_n_cntxt_glob_irq_clr_u
{
  struct ipa_hwio_def_ipa_gsi_top_ee_n_cntxt_glob_irq_clr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_GSI_TOP_EE_n_CNTXT_GSI_IRQ_STTS
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_gsi_top_ee_n_cntxt_gsi_irq_stts_s
{
  u32 gsi_break_point : 1;
  u32 gsi_bus_error : 1;
  u32 gsi_cmd_fifo_ovrflow : 1;
  u32 gsi_mcs_stack_ovrflow : 1;
  u32 reserved0 : 28;
};

/* Union definition of register */
union ipa_hwio_def_ipa_gsi_top_ee_n_cntxt_gsi_irq_stts_u
{
  struct ipa_hwio_def_ipa_gsi_top_ee_n_cntxt_gsi_irq_stts_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_GSI_TOP_EE_n_CNTXT_GSI_IRQ_EN
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_gsi_top_ee_n_cntxt_gsi_irq_en_s
{
  u32 gsi_break_point : 1;
  u32 gsi_bus_error : 1;
  u32 gsi_cmd_fifo_ovrflow : 1;
  u32 gsi_mcs_stack_ovrflow : 1;
  u32 reserved0 : 28;
};

/* Union definition of register */
union ipa_hwio_def_ipa_gsi_top_ee_n_cntxt_gsi_irq_en_u
{
  struct ipa_hwio_def_ipa_gsi_top_ee_n_cntxt_gsi_irq_en_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_GSI_TOP_EE_n_CNTXT_GSI_IRQ_CLR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_gsi_top_ee_n_cntxt_gsi_irq_clr_s
{
  u32 gsi_break_point : 1;
  u32 gsi_bus_error : 1;
  u32 gsi_cmd_fifo_ovrflow : 1;
  u32 gsi_mcs_stack_ovrflow : 1;
  u32 reserved0 : 28;
};

/* Union definition of register */
union ipa_hwio_def_ipa_gsi_top_ee_n_cntxt_gsi_irq_clr_u
{
  struct ipa_hwio_def_ipa_gsi_top_ee_n_cntxt_gsi_irq_clr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_GSI_TOP_EE_n_CNTXT_INTSET
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_gsi_top_ee_n_cntxt_intset_s
{
  u32 intype : 1;
  u32 reserved0 : 31;
};

/* Union definition of register */
union ipa_hwio_def_ipa_gsi_top_ee_n_cntxt_intset_u
{
  struct ipa_hwio_def_ipa_gsi_top_ee_n_cntxt_intset_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_GSI_TOP_EE_n_CNTXT_MSI_BASE_LSB
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_gsi_top_ee_n_cntxt_msi_base_lsb_s
{
  u32 msi_addr_lsb : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_gsi_top_ee_n_cntxt_msi_base_lsb_u
{
  struct ipa_hwio_def_ipa_gsi_top_ee_n_cntxt_msi_base_lsb_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_GSI_TOP_EE_n_CNTXT_MSI_BASE_MSB
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_gsi_top_ee_n_cntxt_msi_base_msb_s
{
  u32 msi_addr_msb : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_gsi_top_ee_n_cntxt_msi_base_msb_u
{
  struct ipa_hwio_def_ipa_gsi_top_ee_n_cntxt_msi_base_msb_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_GSI_TOP_EE_n_CNTXT_INT_VEC
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_gsi_top_ee_n_cntxt_int_vec_s
{
  u32 int_vec : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_gsi_top_ee_n_cntxt_int_vec_u
{
  struct ipa_hwio_def_ipa_gsi_top_ee_n_cntxt_int_vec_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_GSI_TOP_EE_n_ERROR_LOG
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_gsi_top_ee_n_error_log_s
{
  u32 error_log : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_gsi_top_ee_n_error_log_u
{
  struct ipa_hwio_def_ipa_gsi_top_ee_n_error_log_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_GSI_TOP_EE_n_ERROR_LOG_CLR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_gsi_top_ee_n_error_log_clr_s
{
  u32 error_log_clr : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_gsi_top_ee_n_error_log_clr_u
{
  struct ipa_hwio_def_ipa_gsi_top_ee_n_error_log_clr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_GSI_TOP_EE_n_CNTXT_SCRATCH_0
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_gsi_top_ee_n_cntxt_scratch_0_s
{
  u32 scratch : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_gsi_top_ee_n_cntxt_scratch_0_u
{
  struct ipa_hwio_def_ipa_gsi_top_ee_n_cntxt_scratch_0_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_GSI_TOP_EE_n_CNTXT_SCRATCH_1
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_gsi_top_ee_n_cntxt_scratch_1_s
{
  u32 scratch : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_gsi_top_ee_n_cntxt_scratch_1_u
{
  struct ipa_hwio_def_ipa_gsi_top_ee_n_cntxt_scratch_1_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_GSI_TOP_GSI_MCS_CFG
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_gsi_top_gsi_mcs_cfg_s
{
  u32 mcs_enable : 1;
  u32 reserved0 : 31;
};

/* Union definition of register */
union ipa_hwio_def_ipa_gsi_top_gsi_mcs_cfg_u
{
  struct ipa_hwio_def_ipa_gsi_top_gsi_mcs_cfg_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_GSI_TOP_GSI_TZ_FW_AUTH_LOCK
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_gsi_top_gsi_tz_fw_auth_lock_s
{
  u32 dis_iram_write : 1;
  u32 dis_debug_shram_write : 1;
  u32 reserved0 : 30;
};

/* Union definition of register */
union ipa_hwio_def_ipa_gsi_top_gsi_tz_fw_auth_lock_u
{
  struct ipa_hwio_def_ipa_gsi_top_gsi_tz_fw_auth_lock_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_GSI_TOP_GSI_MSA_FW_AUTH_LOCK
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_gsi_top_gsi_msa_fw_auth_lock_s
{
  u32 dis_iram_write : 1;
  u32 dis_debug_shram_write : 1;
  u32 reserved0 : 30;
};

/* Union definition of register */
union ipa_hwio_def_ipa_gsi_top_gsi_msa_fw_auth_lock_u
{
  struct ipa_hwio_def_ipa_gsi_top_gsi_msa_fw_auth_lock_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_GSI_TOP_GSI_SP_FW_AUTH_LOCK
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_gsi_top_gsi_sp_fw_auth_lock_s
{
  u32 dis_iram_write : 1;
  u32 dis_debug_shram_write : 1;
  u32 reserved0 : 30;
};

/* Union definition of register */
union ipa_hwio_def_ipa_gsi_top_gsi_sp_fw_auth_lock_u
{
  struct ipa_hwio_def_ipa_gsi_top_gsi_sp_fw_auth_lock_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_GSI_TOP_INTER_EE_n_ORIGINATOR_EE
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_gsi_top_inter_ee_n_originator_ee_s
{
  u32 ee_number : 4;
  u32 reserved0 : 28;
};

/* Union definition of register */
union ipa_hwio_def_ipa_gsi_top_inter_ee_n_originator_ee_u
{
  struct ipa_hwio_def_ipa_gsi_top_inter_ee_n_originator_ee_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_GSI_TOP_INTER_EE_n_GSI_CH_CMD
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_gsi_top_inter_ee_n_gsi_ch_cmd_s
{
  u32 chid : 8;
  u32 reserved0 : 16;
  u32 opcode : 8;
};

/* Union definition of register */
union ipa_hwio_def_ipa_gsi_top_inter_ee_n_gsi_ch_cmd_u
{
  struct ipa_hwio_def_ipa_gsi_top_inter_ee_n_gsi_ch_cmd_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_GSI_TOP_INTER_EE_n_EV_CH_CMD
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_gsi_top_inter_ee_n_ev_ch_cmd_s
{
  u32 chid : 8;
  u32 reserved0 : 16;
  u32 opcode : 8;
};

/* Union definition of register */
union ipa_hwio_def_ipa_gsi_top_inter_ee_n_ev_ch_cmd_u
{
  struct ipa_hwio_def_ipa_gsi_top_inter_ee_n_ev_ch_cmd_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_GSI_TOP_INTER_EE_n_SRC_GSI_CH_IRQ_k
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_gsi_top_inter_ee_n_src_gsi_ch_irq_k_s
{
  u32 gsi_ch_bit_map : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_gsi_top_inter_ee_n_src_gsi_ch_irq_k_u
{
  struct ipa_hwio_def_ipa_gsi_top_inter_ee_n_src_gsi_ch_irq_k_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_GSI_TOP_INTER_EE_n_SRC_GSI_CH_IRQ_MSK_k
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_gsi_top_inter_ee_n_src_gsi_ch_irq_msk_k_s
{
  u32 gsi_ch_bit_map_msk : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_gsi_top_inter_ee_n_src_gsi_ch_irq_msk_k_u
{
  struct ipa_hwio_def_ipa_gsi_top_inter_ee_n_src_gsi_ch_irq_msk_k_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_GSI_TOP_INTER_EE_n_SRC_GSI_CH_IRQ_CLR_k
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_gsi_top_inter_ee_n_src_gsi_ch_irq_clr_k_s
{
  u32 gsi_ch_bit_map : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_gsi_top_inter_ee_n_src_gsi_ch_irq_clr_k_u
{
  struct ipa_hwio_def_ipa_gsi_top_inter_ee_n_src_gsi_ch_irq_clr_k_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_GSI_TOP_INTER_EE_n_SRC_EV_CH_IRQ_k
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_gsi_top_inter_ee_n_src_ev_ch_irq_k_s
{
  u32 ev_ch_bit_map : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_gsi_top_inter_ee_n_src_ev_ch_irq_k_u
{
  struct ipa_hwio_def_ipa_gsi_top_inter_ee_n_src_ev_ch_irq_k_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_GSI_TOP_INTER_EE_n_SRC_EV_CH_IRQ_MSK_k
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_gsi_top_inter_ee_n_src_ev_ch_irq_msk_k_s
{
  u32 ev_ch_bit_map_msk : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_gsi_top_inter_ee_n_src_ev_ch_irq_msk_k_u
{
  struct ipa_hwio_def_ipa_gsi_top_inter_ee_n_src_ev_ch_irq_msk_k_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_GSI_TOP_INTER_EE_n_SRC_EV_CH_IRQ_CLR_k
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_gsi_top_inter_ee_n_src_ev_ch_irq_clr_k_s
{
  u32 ev_ch_bit_map : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_gsi_top_inter_ee_n_src_ev_ch_irq_clr_k_u
{
  struct ipa_hwio_def_ipa_gsi_top_inter_ee_n_src_ev_ch_irq_clr_k_s def;
  u32 value;
};

/*----------------------------------------------------------------------------
 * MODULE: IPA_GSI_TOP_XPU3
 *--------------------------------------------------------------------------*/

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_GSI_TOP_XPU3_GCR0
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_gsi_top_xpu3_gcr0_s
{
  u32 aaden : 1;
  u32 aalog_mode_dis : 1;
  u32 reserved0 : 30;
};

/* Union definition of register */
union ipa_hwio_def_ipa_gsi_top_xpu3_gcr0_u
{
  struct ipa_hwio_def_ipa_gsi_top_xpu3_gcr0_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_GSI_TOP_XPU3_SCR0
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_gsi_top_xpu3_scr0_s
{
  u32 scfgere : 1;
  u32 sclere : 1;
  u32 scfgeie : 1;
  u32 scleie : 1;
  u32 reserved0 : 4;
  u32 dynamic_clk_en : 1;
  u32 reserved1 : 23;
};

/* Union definition of register */
union ipa_hwio_def_ipa_gsi_top_xpu3_scr0_u
{
  struct ipa_hwio_def_ipa_gsi_top_xpu3_scr0_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_GSI_TOP_XPU3_CR0
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_gsi_top_xpu3_cr0_s
{
  u32 cfgere : 1;
  u32 clere : 1;
  u32 cfgeie : 1;
  u32 cleie : 1;
  u32 reserved0 : 3;
  u32 vmiden : 1;
  u32 dynamic_clk_en : 1;
  u32 reserved1 : 23;
};

/* Union definition of register */
union ipa_hwio_def_ipa_gsi_top_xpu3_cr0_u
{
  struct ipa_hwio_def_ipa_gsi_top_xpu3_cr0_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_GSI_TOP_XPU3_RPU_ACR0
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_gsi_top_xpu3_rpu_acr0_s
{
  u32 suvmid : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_gsi_top_xpu3_rpu_acr0_u
{
  struct ipa_hwio_def_ipa_gsi_top_xpu3_rpu_acr0_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_GSI_TOP_XPU3_QAD0_GCR0
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_gsi_top_xpu3_qad0_gcr0_s
{
  u32 qad0den : 1;
  u32 qad0log_mode_dis : 1;
  u32 reserved0 : 30;
};

/* Union definition of register */
union ipa_hwio_def_ipa_gsi_top_xpu3_qad0_gcr0_u
{
  struct ipa_hwio_def_ipa_gsi_top_xpu3_qad0_gcr0_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_GSI_TOP_XPU3_QAD0_CR0
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_gsi_top_xpu3_qad0_cr0_s
{
  u32 cfgere : 1;
  u32 clere : 1;
  u32 cfgeie : 1;
  u32 cleie : 1;
  u32 reserved0 : 4;
  u32 dynamic_clk_en : 1;
  u32 reserved1 : 23;
};

/* Union definition of register */
union ipa_hwio_def_ipa_gsi_top_xpu3_qad0_cr0_u
{
  struct ipa_hwio_def_ipa_gsi_top_xpu3_qad0_cr0_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_GSI_TOP_XPU3_QAD1_GCR0
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_gsi_top_xpu3_qad1_gcr0_s
{
  u32 qad1den : 1;
  u32 qad1log_mode_dis : 1;
  u32 reserved0 : 30;
};

/* Union definition of register */
union ipa_hwio_def_ipa_gsi_top_xpu3_qad1_gcr0_u
{
  struct ipa_hwio_def_ipa_gsi_top_xpu3_qad1_gcr0_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_GSI_TOP_XPU3_QAD1_CR0
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_gsi_top_xpu3_qad1_cr0_s
{
  u32 cfgere : 1;
  u32 clere : 1;
  u32 cfgeie : 1;
  u32 cleie : 1;
  u32 reserved0 : 4;
  u32 dynamic_clk_en : 1;
  u32 reserved1 : 23;
};

/* Union definition of register */
union ipa_hwio_def_ipa_gsi_top_xpu3_qad1_cr0_u
{
  struct ipa_hwio_def_ipa_gsi_top_xpu3_qad1_cr0_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_GSI_TOP_XPU3_IDR3
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_gsi_top_xpu3_idr3_s
{
  u32 nvmid : 8;
  u32 mv : 1;
  u32 pt : 1;
  u32 reserved0 : 22;
};

/* Union definition of register */
union ipa_hwio_def_ipa_gsi_top_xpu3_idr3_u
{
  struct ipa_hwio_def_ipa_gsi_top_xpu3_idr3_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_GSI_TOP_XPU3_IDR2
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_gsi_top_xpu3_idr2_s
{
  u32 num_qad : 4;
  u32 reserved0 : 4;
  u32 vmidacr_en : 8;
  u32 sec_en : 8;
  u32 nonsec_en : 8;
};

/* Union definition of register */
union ipa_hwio_def_ipa_gsi_top_xpu3_idr2_u
{
  struct ipa_hwio_def_ipa_gsi_top_xpu3_idr2_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_GSI_TOP_XPU3_IDR1
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_gsi_top_xpu3_idr1_s
{
  u32 reserved0 : 16;
  u32 config_addr_width : 6;
  u32 reserved1 : 2;
  u32 client_addr_width : 6;
  u32 reserved2 : 2;
};

/* Union definition of register */
union ipa_hwio_def_ipa_gsi_top_xpu3_idr1_u
{
  struct ipa_hwio_def_ipa_gsi_top_xpu3_idr1_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_GSI_TOP_XPU3_IDR0
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_gsi_top_xpu3_idr0_s
{
  u32 xputype : 2;
  u32 reserved0 : 3;
  u32 clientreq_halt_ack_hw_en : 1;
  u32 reserved1 : 10;
  u32 nrg : 10;
  u32 reserved2 : 6;
};

/* Union definition of register */
union ipa_hwio_def_ipa_gsi_top_xpu3_idr0_u
{
  struct ipa_hwio_def_ipa_gsi_top_xpu3_idr0_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_GSI_TOP_XPU3_REV
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_gsi_top_xpu3_rev_s
{
  u32 step : 16;
  u32 minor : 12;
  u32 major : 4;
};

/* Union definition of register */
union ipa_hwio_def_ipa_gsi_top_xpu3_rev_u
{
  struct ipa_hwio_def_ipa_gsi_top_xpu3_rev_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_GSI_TOP_XPU3_LOG_MODE_DIS
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_gsi_top_xpu3_log_mode_dis_s
{
  u32 log_mode_dis : 1;
  u32 reserved0 : 31;
};

/* Union definition of register */
union ipa_hwio_def_ipa_gsi_top_xpu3_log_mode_dis_u
{
  struct ipa_hwio_def_ipa_gsi_top_xpu3_log_mode_dis_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_GSI_TOP_XPU3_RGN_FREESTATUSr
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_gsi_top_xpu3_rgn_freestatusr_s
{
  u32 rgfreestatus : 21;
  u32 reserved0 : 11;
};

/* Union definition of register */
union ipa_hwio_def_ipa_gsi_top_xpu3_rgn_freestatusr_u
{
  struct ipa_hwio_def_ipa_gsi_top_xpu3_rgn_freestatusr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_GSI_TOP_XPU3_SEAR0
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_gsi_top_xpu3_sear0_s
{
  u32 addr_31_0 : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_gsi_top_xpu3_sear0_u
{
  struct ipa_hwio_def_ipa_gsi_top_xpu3_sear0_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_GSI_TOP_XPU3_SESR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_gsi_top_xpu3_sesr_s
{
  u32 cfg : 1;
  u32 client : 1;
  u32 cfgmulti : 1;
  u32 clmulti : 1;
  u32 reserved0 : 28;
};

/* Union definition of register */
union ipa_hwio_def_ipa_gsi_top_xpu3_sesr_u
{
  struct ipa_hwio_def_ipa_gsi_top_xpu3_sesr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_GSI_TOP_XPU3_SESRRESTORE
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_gsi_top_xpu3_sesrrestore_s
{
  u32 cfg : 1;
  u32 client : 1;
  u32 cfgmulti : 1;
  u32 clmulti : 1;
  u32 reserved0 : 28;
};

/* Union definition of register */
union ipa_hwio_def_ipa_gsi_top_xpu3_sesrrestore_u
{
  struct ipa_hwio_def_ipa_gsi_top_xpu3_sesrrestore_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_GSI_TOP_XPU3_SESYNR0
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_gsi_top_xpu3_sesynr0_s
{
  u32 xprotns : 1;
  u32 awrite : 1;
  u32 xinst : 1;
  u32 xpriv : 1;
  u32 reserved0 : 4;
  u32 qad : 8;
  u32 alen : 8;
  u32 asize : 3;
  u32 reserved1 : 2;
  u32 burstlen : 1;
  u32 ac : 1;
  u32 reserved2 : 1;
};

/* Union definition of register */
union ipa_hwio_def_ipa_gsi_top_xpu3_sesynr0_u
{
  struct ipa_hwio_def_ipa_gsi_top_xpu3_sesynr0_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_GSI_TOP_XPU3_SESYNR1
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_gsi_top_xpu3_sesynr1_s
{
  u32 mid : 8;
  u32 pid : 5;
  u32 bid : 3;
  u32 vmid : 8;
  u32 tid : 8;
};

/* Union definition of register */
union ipa_hwio_def_ipa_gsi_top_xpu3_sesynr1_u
{
  struct ipa_hwio_def_ipa_gsi_top_xpu3_sesynr1_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_GSI_TOP_XPU3_SESYNR2
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_gsi_top_xpu3_sesynr2_s
{
  u32 memtype : 3;
  u32 reserved0 : 4;
  u32 transient : 1;
  u32 noallocate : 1;
  u32 ooowr : 1;
  u32 ooord : 1;
  u32 orderedwr : 1;
  u32 orderedrd : 1;
  u32 portmrel : 1;
  u32 innerwritethrough : 1;
  u32 innertransient : 1;
  u32 innershared : 1;
  u32 innercacheable : 1;
  u32 innernoallocate : 1;
  u32 writethrough : 1;
  u32 shared : 1;
  u32 full : 1;
  u32 exclusive : 1;
  u32 error : 1;
  u32 earlywrresp : 1;
  u32 device_type : 2;
  u32 device : 1;
  u32 cacheable : 1;
  u32 burst : 1;
  u32 bar : 2;
};

/* Union definition of register */
union ipa_hwio_def_ipa_gsi_top_xpu3_sesynr2_u
{
  struct ipa_hwio_def_ipa_gsi_top_xpu3_sesynr2_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_GSI_TOP_XPU3_SEAR1
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_gsi_top_xpu3_sear1_s
{
  u32 addr_63_32 : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_gsi_top_xpu3_sear1_u
{
  struct ipa_hwio_def_ipa_gsi_top_xpu3_sear1_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_GSI_TOP_XPU3_EAR0
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_gsi_top_xpu3_ear0_s
{
  u32 addr_31_0 : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_gsi_top_xpu3_ear0_u
{
  struct ipa_hwio_def_ipa_gsi_top_xpu3_ear0_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_GSI_TOP_XPU3_ESR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_gsi_top_xpu3_esr_s
{
  u32 cfg : 1;
  u32 client : 1;
  u32 cfgmulti : 1;
  u32 clmulti : 1;
  u32 reserved0 : 28;
};

/* Union definition of register */
union ipa_hwio_def_ipa_gsi_top_xpu3_esr_u
{
  struct ipa_hwio_def_ipa_gsi_top_xpu3_esr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_GSI_TOP_XPU3_ESRRESTORE
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_gsi_top_xpu3_esrrestore_s
{
  u32 cfg : 1;
  u32 client : 1;
  u32 cfgmulti : 1;
  u32 clmulti : 1;
  u32 reserved0 : 28;
};

/* Union definition of register */
union ipa_hwio_def_ipa_gsi_top_xpu3_esrrestore_u
{
  struct ipa_hwio_def_ipa_gsi_top_xpu3_esrrestore_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_GSI_TOP_XPU3_ESYNR0
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_gsi_top_xpu3_esynr0_s
{
  u32 xprotns : 1;
  u32 awrite : 1;
  u32 xinst : 1;
  u32 xpriv : 1;
  u32 reserved0 : 4;
  u32 qad : 8;
  u32 alen : 8;
  u32 asize : 3;
  u32 reserved1 : 2;
  u32 burstlen : 1;
  u32 ac : 1;
  u32 reserved2 : 1;
};

/* Union definition of register */
union ipa_hwio_def_ipa_gsi_top_xpu3_esynr0_u
{
  struct ipa_hwio_def_ipa_gsi_top_xpu3_esynr0_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_GSI_TOP_XPU3_ESYNR1
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_gsi_top_xpu3_esynr1_s
{
  u32 mid : 8;
  u32 pid : 5;
  u32 bid : 3;
  u32 vmid : 8;
  u32 tid : 8;
};

/* Union definition of register */
union ipa_hwio_def_ipa_gsi_top_xpu3_esynr1_u
{
  struct ipa_hwio_def_ipa_gsi_top_xpu3_esynr1_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_GSI_TOP_XPU3_ESYNR2
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_gsi_top_xpu3_esynr2_s
{
  u32 memtype : 3;
  u32 reserved0 : 4;
  u32 transient : 1;
  u32 noallocate : 1;
  u32 ooowr : 1;
  u32 ooord : 1;
  u32 orderedwr : 1;
  u32 orderedrd : 1;
  u32 portmrel : 1;
  u32 innerwritethrough : 1;
  u32 innertransient : 1;
  u32 innershared : 1;
  u32 innercacheable : 1;
  u32 innernoallocate : 1;
  u32 writethrough : 1;
  u32 shared : 1;
  u32 full : 1;
  u32 exclusive : 1;
  u32 error : 1;
  u32 earlywrresp : 1;
  u32 device_type : 2;
  u32 device : 1;
  u32 cacheable : 1;
  u32 burst : 1;
  u32 bar : 2;
};

/* Union definition of register */
union ipa_hwio_def_ipa_gsi_top_xpu3_esynr2_u
{
  struct ipa_hwio_def_ipa_gsi_top_xpu3_esynr2_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_GSI_TOP_XPU3_EAR1
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_gsi_top_xpu3_ear1_s
{
  u32 addr_63_32 : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_gsi_top_xpu3_ear1_u
{
  struct ipa_hwio_def_ipa_gsi_top_xpu3_ear1_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_GSI_TOP_XPU3_QAD0_EAR0
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_gsi_top_xpu3_qad0_ear0_s
{
  u32 addr_31_0 : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_gsi_top_xpu3_qad0_ear0_u
{
  struct ipa_hwio_def_ipa_gsi_top_xpu3_qad0_ear0_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_GSI_TOP_XPU3_QAD0_ESR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_gsi_top_xpu3_qad0_esr_s
{
  u32 cfg : 1;
  u32 client : 1;
  u32 cfgmulti : 1;
  u32 clmulti : 1;
  u32 reserved0 : 28;
};

/* Union definition of register */
union ipa_hwio_def_ipa_gsi_top_xpu3_qad0_esr_u
{
  struct ipa_hwio_def_ipa_gsi_top_xpu3_qad0_esr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_GSI_TOP_XPU3_QAD0_ESRRESTORE
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_gsi_top_xpu3_qad0_esrrestore_s
{
  u32 cfg : 1;
  u32 client : 1;
  u32 cfgmulti : 1;
  u32 clmulti : 1;
  u32 reserved0 : 28;
};

/* Union definition of register */
union ipa_hwio_def_ipa_gsi_top_xpu3_qad0_esrrestore_u
{
  struct ipa_hwio_def_ipa_gsi_top_xpu3_qad0_esrrestore_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_GSI_TOP_XPU3_QAD0_ESYNR0
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_gsi_top_xpu3_qad0_esynr0_s
{
  u32 xprotns : 1;
  u32 awrite : 1;
  u32 xinst : 1;
  u32 xpriv : 1;
  u32 reserved0 : 4;
  u32 qad : 8;
  u32 alen : 8;
  u32 asize : 3;
  u32 reserved1 : 2;
  u32 burstlen : 1;
  u32 ac : 1;
  u32 reserved2 : 1;
};

/* Union definition of register */
union ipa_hwio_def_ipa_gsi_top_xpu3_qad0_esynr0_u
{
  struct ipa_hwio_def_ipa_gsi_top_xpu3_qad0_esynr0_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_GSI_TOP_XPU3_QAD0_ESYNR1
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_gsi_top_xpu3_qad0_esynr1_s
{
  u32 mid : 8;
  u32 pid : 5;
  u32 bid : 3;
  u32 vmid : 8;
  u32 tid : 8;
};

/* Union definition of register */
union ipa_hwio_def_ipa_gsi_top_xpu3_qad0_esynr1_u
{
  struct ipa_hwio_def_ipa_gsi_top_xpu3_qad0_esynr1_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_GSI_TOP_XPU3_QAD0_ESYNR2
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_gsi_top_xpu3_qad0_esynr2_s
{
  u32 memtype : 3;
  u32 reserved0 : 4;
  u32 transient : 1;
  u32 noallocate : 1;
  u32 ooowr : 1;
  u32 ooord : 1;
  u32 orderedwr : 1;
  u32 orderedrd : 1;
  u32 portmrel : 1;
  u32 innerwritethrough : 1;
  u32 innertransient : 1;
  u32 innershared : 1;
  u32 innercacheable : 1;
  u32 innernoallocate : 1;
  u32 writethrough : 1;
  u32 shared : 1;
  u32 full : 1;
  u32 exclusive : 1;
  u32 error : 1;
  u32 earlywrresp : 1;
  u32 device_type : 2;
  u32 device : 1;
  u32 cacheable : 1;
  u32 burst : 1;
  u32 bar : 2;
};

/* Union definition of register */
union ipa_hwio_def_ipa_gsi_top_xpu3_qad0_esynr2_u
{
  struct ipa_hwio_def_ipa_gsi_top_xpu3_qad0_esynr2_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_GSI_TOP_XPU3_QAD0_EAR1
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_gsi_top_xpu3_qad0_ear1_s
{
  u32 addr_63_32 : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_gsi_top_xpu3_qad0_ear1_u
{
  struct ipa_hwio_def_ipa_gsi_top_xpu3_qad0_ear1_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_GSI_TOP_XPU3_QAD1_EAR0
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_gsi_top_xpu3_qad1_ear0_s
{
  u32 addr_31_0 : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_gsi_top_xpu3_qad1_ear0_u
{
  struct ipa_hwio_def_ipa_gsi_top_xpu3_qad1_ear0_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_GSI_TOP_XPU3_QAD1_ESR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_gsi_top_xpu3_qad1_esr_s
{
  u32 cfg : 1;
  u32 client : 1;
  u32 cfgmulti : 1;
  u32 clmulti : 1;
  u32 reserved0 : 28;
};

/* Union definition of register */
union ipa_hwio_def_ipa_gsi_top_xpu3_qad1_esr_u
{
  struct ipa_hwio_def_ipa_gsi_top_xpu3_qad1_esr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_GSI_TOP_XPU3_QAD1_ESRRESTORE
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_gsi_top_xpu3_qad1_esrrestore_s
{
  u32 cfg : 1;
  u32 client : 1;
  u32 cfgmulti : 1;
  u32 clmulti : 1;
  u32 reserved0 : 28;
};

/* Union definition of register */
union ipa_hwio_def_ipa_gsi_top_xpu3_qad1_esrrestore_u
{
  struct ipa_hwio_def_ipa_gsi_top_xpu3_qad1_esrrestore_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_GSI_TOP_XPU3_QAD1_ESYNR0
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_gsi_top_xpu3_qad1_esynr0_s
{
  u32 xprotns : 1;
  u32 awrite : 1;
  u32 xinst : 1;
  u32 xpriv : 1;
  u32 reserved0 : 4;
  u32 qad : 8;
  u32 alen : 8;
  u32 asize : 3;
  u32 reserved1 : 2;
  u32 burstlen : 1;
  u32 ac : 1;
  u32 reserved2 : 1;
};

/* Union definition of register */
union ipa_hwio_def_ipa_gsi_top_xpu3_qad1_esynr0_u
{
  struct ipa_hwio_def_ipa_gsi_top_xpu3_qad1_esynr0_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_GSI_TOP_XPU3_QAD1_ESYNR1
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_gsi_top_xpu3_qad1_esynr1_s
{
  u32 mid : 8;
  u32 pid : 5;
  u32 bid : 3;
  u32 vmid : 8;
  u32 tid : 8;
};

/* Union definition of register */
union ipa_hwio_def_ipa_gsi_top_xpu3_qad1_esynr1_u
{
  struct ipa_hwio_def_ipa_gsi_top_xpu3_qad1_esynr1_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_GSI_TOP_XPU3_QAD1_ESYNR2
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_gsi_top_xpu3_qad1_esynr2_s
{
  u32 memtype : 3;
  u32 reserved0 : 4;
  u32 transient : 1;
  u32 noallocate : 1;
  u32 ooowr : 1;
  u32 ooord : 1;
  u32 orderedwr : 1;
  u32 orderedrd : 1;
  u32 portmrel : 1;
  u32 innerwritethrough : 1;
  u32 innertransient : 1;
  u32 innershared : 1;
  u32 innercacheable : 1;
  u32 innernoallocate : 1;
  u32 writethrough : 1;
  u32 shared : 1;
  u32 full : 1;
  u32 exclusive : 1;
  u32 error : 1;
  u32 earlywrresp : 1;
  u32 device_type : 2;
  u32 device : 1;
  u32 cacheable : 1;
  u32 burst : 1;
  u32 bar : 2;
};

/* Union definition of register */
union ipa_hwio_def_ipa_gsi_top_xpu3_qad1_esynr2_u
{
  struct ipa_hwio_def_ipa_gsi_top_xpu3_qad1_esynr2_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_GSI_TOP_XPU3_QAD1_EAR1
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_gsi_top_xpu3_qad1_ear1_s
{
  u32 addr_63_32 : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_gsi_top_xpu3_qad1_ear1_u
{
  struct ipa_hwio_def_ipa_gsi_top_xpu3_qad1_ear1_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_GSI_TOP_XPU3_RGN_OWNERSTATUSr
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_gsi_top_xpu3_rgn_ownerstatusr_s
{
  u32 rgownerstatus : 21;
  u32 reserved0 : 11;
};

/* Union definition of register */
union ipa_hwio_def_ipa_gsi_top_xpu3_rgn_ownerstatusr_u
{
  struct ipa_hwio_def_ipa_gsi_top_xpu3_rgn_ownerstatusr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_GSI_TOP_XPU3_RGn_GCR0
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_gsi_top_xpu3_rgn_gcr0_s
{
  u32 rg_owner : 3;
  u32 reserved0 : 5;
  u32 rg_sec_apps : 1;
  u32 reserved1 : 23;
};

/* Union definition of register */
union ipa_hwio_def_ipa_gsi_top_xpu3_rgn_gcr0_u
{
  struct ipa_hwio_def_ipa_gsi_top_xpu3_rgn_gcr0_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_GSI_TOP_XPU3_RGn_GCR3
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_gsi_top_xpu3_rgn_gcr3_s
{
  u32 secure_access_lock : 1;
  u32 reserved0 : 31;
};

/* Union definition of register */
union ipa_hwio_def_ipa_gsi_top_xpu3_rgn_gcr3_u
{
  struct ipa_hwio_def_ipa_gsi_top_xpu3_rgn_gcr3_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_GSI_TOP_XPU3_RGn_CR0
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_gsi_top_xpu3_rgn_cr0_s
{
  u32 rgsclrden_apps : 1;
  u32 reserved0 : 31;
};

/* Union definition of register */
union ipa_hwio_def_ipa_gsi_top_xpu3_rgn_cr0_u
{
  struct ipa_hwio_def_ipa_gsi_top_xpu3_rgn_cr0_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_GSI_TOP_XPU3_RGn_CR1
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_gsi_top_xpu3_rgn_cr1_s
{
  u32 rgclrden : 3;
  u32 reserved0 : 29;
};

/* Union definition of register */
union ipa_hwio_def_ipa_gsi_top_xpu3_rgn_cr1_u
{
  struct ipa_hwio_def_ipa_gsi_top_xpu3_rgn_cr1_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_GSI_TOP_XPU3_RGn_CR2
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_gsi_top_xpu3_rgn_cr2_s
{
  u32 rgsclwren_apps : 1;
  u32 reserved0 : 31;
};

/* Union definition of register */
union ipa_hwio_def_ipa_gsi_top_xpu3_rgn_cr2_u
{
  struct ipa_hwio_def_ipa_gsi_top_xpu3_rgn_cr2_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_GSI_TOP_XPU3_RGn_CR3
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_gsi_top_xpu3_rgn_cr3_s
{
  u32 rgclwren : 3;
  u32 reserved0 : 29;
};

/* Union definition of register */
union ipa_hwio_def_ipa_gsi_top_xpu3_rgn_cr3_u
{
  struct ipa_hwio_def_ipa_gsi_top_xpu3_rgn_cr3_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_GSI_TOP_XPU3_RGn_RACR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_gsi_top_xpu3_rgn_racr_s
{
  u32 re : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_gsi_top_xpu3_rgn_racr_u
{
  struct ipa_hwio_def_ipa_gsi_top_xpu3_rgn_racr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_GSI_TOP_XPU3_RGn_WACR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_gsi_top_xpu3_rgn_wacr_s
{
  u32 we : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_gsi_top_xpu3_rgn_wacr_u
{
  struct ipa_hwio_def_ipa_gsi_top_xpu3_rgn_wacr_s def;
  u32 value;
};

/*----------------------------------------------------------------------------
 * MODULE: IPA_MS_MPU_CFG_SNOC_IPA_MS_MPU_CFG
 *--------------------------------------------------------------------------*/

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_MS_MPU_CFG_XPU3_GCR0
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_ms_mpu_cfg_xpu3_gcr0_s
{
  u32 aaden : 1;
  u32 aalog_mode_dis : 1;
  u32 reserved0 : 30;
};

/* Union definition of register */
union ipa_hwio_def_ipa_ms_mpu_cfg_xpu3_gcr0_u
{
  struct ipa_hwio_def_ipa_ms_mpu_cfg_xpu3_gcr0_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_MS_MPU_CFG_XPU3_SCR0
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_ms_mpu_cfg_xpu3_scr0_s
{
  u32 scfgere : 1;
  u32 sclere : 1;
  u32 scfgeie : 1;
  u32 scleie : 1;
  u32 reserved0 : 4;
  u32 dynamic_clk_en : 1;
  u32 reserved1 : 23;
};

/* Union definition of register */
union ipa_hwio_def_ipa_ms_mpu_cfg_xpu3_scr0_u
{
  struct ipa_hwio_def_ipa_ms_mpu_cfg_xpu3_scr0_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_MS_MPU_CFG_XPU3_CR0
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_ms_mpu_cfg_xpu3_cr0_s
{
  u32 cfgere : 1;
  u32 clere : 1;
  u32 cfgeie : 1;
  u32 cleie : 1;
  u32 reserved0 : 4;
  u32 dynamic_clk_en : 1;
  u32 reserved1 : 23;
};

/* Union definition of register */
union ipa_hwio_def_ipa_ms_mpu_cfg_xpu3_cr0_u
{
  struct ipa_hwio_def_ipa_ms_mpu_cfg_xpu3_cr0_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_MS_MPU_CFG_XPU3_QAD0_GCR0
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_ms_mpu_cfg_xpu3_qad0_gcr0_s
{
  u32 qad0den : 1;
  u32 qad0log_mode_dis : 1;
  u32 reserved0 : 30;
};

/* Union definition of register */
union ipa_hwio_def_ipa_ms_mpu_cfg_xpu3_qad0_gcr0_u
{
  struct ipa_hwio_def_ipa_ms_mpu_cfg_xpu3_qad0_gcr0_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_MS_MPU_CFG_XPU3_QAD0_CR0
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_ms_mpu_cfg_xpu3_qad0_cr0_s
{
  u32 cfgere : 1;
  u32 clere : 1;
  u32 cfgeie : 1;
  u32 cleie : 1;
  u32 reserved0 : 4;
  u32 dynamic_clk_en : 1;
  u32 reserved1 : 23;
};

/* Union definition of register */
union ipa_hwio_def_ipa_ms_mpu_cfg_xpu3_qad0_cr0_u
{
  struct ipa_hwio_def_ipa_ms_mpu_cfg_xpu3_qad0_cr0_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_MS_MPU_CFG_XPU3_QAD1_GCR0
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_ms_mpu_cfg_xpu3_qad1_gcr0_s
{
  u32 qad1den : 1;
  u32 qad1log_mode_dis : 1;
  u32 reserved0 : 30;
};

/* Union definition of register */
union ipa_hwio_def_ipa_ms_mpu_cfg_xpu3_qad1_gcr0_u
{
  struct ipa_hwio_def_ipa_ms_mpu_cfg_xpu3_qad1_gcr0_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_MS_MPU_CFG_XPU3_QAD1_CR0
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_ms_mpu_cfg_xpu3_qad1_cr0_s
{
  u32 cfgere : 1;
  u32 clere : 1;
  u32 cfgeie : 1;
  u32 cleie : 1;
  u32 reserved0 : 4;
  u32 dynamic_clk_en : 1;
  u32 reserved1 : 23;
};

/* Union definition of register */
union ipa_hwio_def_ipa_ms_mpu_cfg_xpu3_qad1_cr0_u
{
  struct ipa_hwio_def_ipa_ms_mpu_cfg_xpu3_qad1_cr0_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_MS_MPU_CFG_XPU3_UMR_GCR0
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_ms_mpu_cfg_xpu3_umr_gcr0_s
{
  u32 umr_owner : 3;
  u32 reserved0 : 5;
  u32 umr_sec_apps : 1;
  u32 reserved1 : 23;
};

/* Union definition of register */
union ipa_hwio_def_ipa_ms_mpu_cfg_xpu3_umr_gcr0_u
{
  struct ipa_hwio_def_ipa_ms_mpu_cfg_xpu3_umr_gcr0_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_MS_MPU_CFG_XPU3_UMR_GCR3
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_ms_mpu_cfg_xpu3_umr_gcr3_s
{
  u32 umr_secure_access_lock : 1;
  u32 reserved0 : 31;
};

/* Union definition of register */
union ipa_hwio_def_ipa_ms_mpu_cfg_xpu3_umr_gcr3_u
{
  struct ipa_hwio_def_ipa_ms_mpu_cfg_xpu3_umr_gcr3_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_MS_MPU_CFG_XPU3_UMR_CR0
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_ms_mpu_cfg_xpu3_umr_cr0_s
{
  u32 umrsclrden_apps : 1;
  u32 reserved0 : 31;
};

/* Union definition of register */
union ipa_hwio_def_ipa_ms_mpu_cfg_xpu3_umr_cr0_u
{
  struct ipa_hwio_def_ipa_ms_mpu_cfg_xpu3_umr_cr0_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_MS_MPU_CFG_XPU3_UMR_CR1
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_ms_mpu_cfg_xpu3_umr_cr1_s
{
  u32 umrclrden : 3;
  u32 arm_qc_approach : 1;
  u32 reserved0 : 28;
};

/* Union definition of register */
union ipa_hwio_def_ipa_ms_mpu_cfg_xpu3_umr_cr1_u
{
  struct ipa_hwio_def_ipa_ms_mpu_cfg_xpu3_umr_cr1_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_MS_MPU_CFG_XPU3_UMR_CR2
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_ms_mpu_cfg_xpu3_umr_cr2_s
{
  u32 umrsclwren_apps : 1;
  u32 reserved0 : 31;
};

/* Union definition of register */
union ipa_hwio_def_ipa_ms_mpu_cfg_xpu3_umr_cr2_u
{
  struct ipa_hwio_def_ipa_ms_mpu_cfg_xpu3_umr_cr2_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_MS_MPU_CFG_XPU3_UMR_CR3
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_ms_mpu_cfg_xpu3_umr_cr3_s
{
  u32 umrclwren : 3;
  u32 reserved0 : 29;
};

/* Union definition of register */
union ipa_hwio_def_ipa_ms_mpu_cfg_xpu3_umr_cr3_u
{
  struct ipa_hwio_def_ipa_ms_mpu_cfg_xpu3_umr_cr3_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_MS_MPU_CFG_XPU3_IDR3
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_ms_mpu_cfg_xpu3_idr3_s
{
  u32 nvmid : 8;
  u32 mv : 1;
  u32 pt : 1;
  u32 reserved0 : 22;
};

/* Union definition of register */
union ipa_hwio_def_ipa_ms_mpu_cfg_xpu3_idr3_u
{
  struct ipa_hwio_def_ipa_ms_mpu_cfg_xpu3_idr3_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_MS_MPU_CFG_XPU3_IDR2
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_ms_mpu_cfg_xpu3_idr2_s
{
  u32 num_qad : 4;
  u32 reserved0 : 4;
  u32 vmidacr_en : 8;
  u32 sec_en : 8;
  u32 nonsec_en : 8;
};

/* Union definition of register */
union ipa_hwio_def_ipa_ms_mpu_cfg_xpu3_idr2_u
{
  struct ipa_hwio_def_ipa_ms_mpu_cfg_xpu3_idr2_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_MS_MPU_CFG_XPU3_IDR1
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_ms_mpu_cfg_xpu3_idr1_s
{
  u32 lsb : 6;
  u32 reserved0 : 2;
  u32 msb_mpu : 6;
  u32 reserved1 : 2;
  u32 config_addr_width : 6;
  u32 reserved2 : 2;
  u32 client_addr_width : 6;
  u32 reserved3 : 2;
};

/* Union definition of register */
union ipa_hwio_def_ipa_ms_mpu_cfg_xpu3_idr1_u
{
  struct ipa_hwio_def_ipa_ms_mpu_cfg_xpu3_idr1_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_MS_MPU_CFG_XPU3_IDR0
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_ms_mpu_cfg_xpu3_idr0_s
{
  u32 xputype : 2;
  u32 reserved0 : 2;
  u32 xpu_client_pipeline_en : 1;
  u32 clientreq_halt_ack_hw_en : 1;
  u32 bled : 1;
  u32 reserved1 : 9;
  u32 nrg : 10;
  u32 reserved2 : 6;
};

/* Union definition of register */
union ipa_hwio_def_ipa_ms_mpu_cfg_xpu3_idr0_u
{
  struct ipa_hwio_def_ipa_ms_mpu_cfg_xpu3_idr0_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_MS_MPU_CFG_XPU3_REV
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_ms_mpu_cfg_xpu3_rev_s
{
  u32 step : 16;
  u32 minor : 12;
  u32 major : 4;
};

/* Union definition of register */
union ipa_hwio_def_ipa_ms_mpu_cfg_xpu3_rev_u
{
  struct ipa_hwio_def_ipa_ms_mpu_cfg_xpu3_rev_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_MS_MPU_CFG_XPU3_LOG_MODE_DIS
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_ms_mpu_cfg_xpu3_log_mode_dis_s
{
  u32 log_mode_dis : 1;
  u32 reserved0 : 31;
};

/* Union definition of register */
union ipa_hwio_def_ipa_ms_mpu_cfg_xpu3_log_mode_dis_u
{
  struct ipa_hwio_def_ipa_ms_mpu_cfg_xpu3_log_mode_dis_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_MS_MPU_CFG_XPU3_RGN_FREESTATUSr
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_ms_mpu_cfg_xpu3_rgn_freestatusr_s
{
  u32 rgfreestatus : 10;
  u32 reserved0 : 22;
};

/* Union definition of register */
union ipa_hwio_def_ipa_ms_mpu_cfg_xpu3_rgn_freestatusr_u
{
  struct ipa_hwio_def_ipa_ms_mpu_cfg_xpu3_rgn_freestatusr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_MS_MPU_CFG_XPU3_SEAR0
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_ms_mpu_cfg_xpu3_sear0_s
{
  u32 addr_31_0 : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_ms_mpu_cfg_xpu3_sear0_u
{
  struct ipa_hwio_def_ipa_ms_mpu_cfg_xpu3_sear0_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_MS_MPU_CFG_XPU3_SESR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_ms_mpu_cfg_xpu3_sesr_s
{
  u32 cfg : 1;
  u32 client : 1;
  u32 cfgmulti : 1;
  u32 clmulti : 1;
  u32 reserved0 : 28;
};

/* Union definition of register */
union ipa_hwio_def_ipa_ms_mpu_cfg_xpu3_sesr_u
{
  struct ipa_hwio_def_ipa_ms_mpu_cfg_xpu3_sesr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_MS_MPU_CFG_XPU3_SESRRESTORE
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_ms_mpu_cfg_xpu3_sesrrestore_s
{
  u32 cfg : 1;
  u32 client : 1;
  u32 cfgmulti : 1;
  u32 clmulti : 1;
  u32 reserved0 : 28;
};

/* Union definition of register */
union ipa_hwio_def_ipa_ms_mpu_cfg_xpu3_sesrrestore_u
{
  struct ipa_hwio_def_ipa_ms_mpu_cfg_xpu3_sesrrestore_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_MS_MPU_CFG_XPU3_SESYNR0
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_ms_mpu_cfg_xpu3_sesynr0_s
{
  u32 xprotns : 1;
  u32 awrite : 1;
  u32 xinst : 1;
  u32 xpriv : 1;
  u32 reserved0 : 4;
  u32 qad : 8;
  u32 alen : 8;
  u32 asize : 3;
  u32 reserved1 : 2;
  u32 burstlen : 1;
  u32 ac : 1;
  u32 reserved2 : 1;
};

/* Union definition of register */
union ipa_hwio_def_ipa_ms_mpu_cfg_xpu3_sesynr0_u
{
  struct ipa_hwio_def_ipa_ms_mpu_cfg_xpu3_sesynr0_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_MS_MPU_CFG_XPU3_SESYNR1
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_ms_mpu_cfg_xpu3_sesynr1_s
{
  u32 mid : 8;
  u32 pid : 5;
  u32 bid : 3;
  u32 vmid : 8;
  u32 tid : 8;
};

/* Union definition of register */
union ipa_hwio_def_ipa_ms_mpu_cfg_xpu3_sesynr1_u
{
  struct ipa_hwio_def_ipa_ms_mpu_cfg_xpu3_sesynr1_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_MS_MPU_CFG_XPU3_SESYNR2
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_ms_mpu_cfg_xpu3_sesynr2_s
{
  u32 memtype : 3;
  u32 reserved0 : 4;
  u32 transient : 1;
  u32 noallocate : 1;
  u32 ooowr : 1;
  u32 ooord : 1;
  u32 orderedwr : 1;
  u32 orderedrd : 1;
  u32 portmrel : 1;
  u32 innerwritethrough : 1;
  u32 innertransient : 1;
  u32 innershared : 1;
  u32 innercacheable : 1;
  u32 innernoallocate : 1;
  u32 writethrough : 1;
  u32 shared : 1;
  u32 full : 1;
  u32 exclusive : 1;
  u32 error : 1;
  u32 earlywrresp : 1;
  u32 device_type : 2;
  u32 device : 1;
  u32 cacheable : 1;
  u32 burst : 1;
  u32 bar : 2;
};

/* Union definition of register */
union ipa_hwio_def_ipa_ms_mpu_cfg_xpu3_sesynr2_u
{
  struct ipa_hwio_def_ipa_ms_mpu_cfg_xpu3_sesynr2_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_MS_MPU_CFG_XPU3_SEAR1
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_ms_mpu_cfg_xpu3_sear1_s
{
  u32 addr_63_32 : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_ms_mpu_cfg_xpu3_sear1_u
{
  struct ipa_hwio_def_ipa_ms_mpu_cfg_xpu3_sear1_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_MS_MPU_CFG_XPU3_SESYNR3
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_ms_mpu_cfg_xpu3_sesynr3_s
{
  u32 nonsec_ad_rg_match : 8;
  u32 sec_ad_rg_match : 8;
  u32 reserved0 : 16;
};

/* Union definition of register */
union ipa_hwio_def_ipa_ms_mpu_cfg_xpu3_sesynr3_u
{
  struct ipa_hwio_def_ipa_ms_mpu_cfg_xpu3_sesynr3_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_MS_MPU_CFG_XPU3_SESYNR4
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_ms_mpu_cfg_xpu3_sesynr4_s
{
  u32 auattr : 16;
  u32 acgranuletrans : 1;
  u32 asid : 5;
  u32 acacheoptype : 4;
  u32 reserved0 : 6;
};

/* Union definition of register */
union ipa_hwio_def_ipa_ms_mpu_cfg_xpu3_sesynr4_u
{
  struct ipa_hwio_def_ipa_ms_mpu_cfg_xpu3_sesynr4_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_MS_MPU_CFG_XPU3_RGN_START0_SSHADOW
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_ms_mpu_cfg_xpu3_rgn_start0_sshadow_s
{
  u32 reserved0 : 12;
  u32 addr_31_0 : 20;
};

/* Union definition of register */
union ipa_hwio_def_ipa_ms_mpu_cfg_xpu3_rgn_start0_sshadow_u
{
  struct ipa_hwio_def_ipa_ms_mpu_cfg_xpu3_rgn_start0_sshadow_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_MS_MPU_CFG_XPU3_RGN_END0_SSHADOW
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_ms_mpu_cfg_xpu3_rgn_end0_sshadow_s
{
  u32 reserved0 : 12;
  u32 addr_31_0 : 20;
};

/* Union definition of register */
union ipa_hwio_def_ipa_ms_mpu_cfg_xpu3_rgn_end0_sshadow_u
{
  struct ipa_hwio_def_ipa_ms_mpu_cfg_xpu3_rgn_end0_sshadow_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_MS_MPU_CFG_XPU3_EAR0
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_ms_mpu_cfg_xpu3_ear0_s
{
  u32 addr_31_0 : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_ms_mpu_cfg_xpu3_ear0_u
{
  struct ipa_hwio_def_ipa_ms_mpu_cfg_xpu3_ear0_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_MS_MPU_CFG_XPU3_ESR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_ms_mpu_cfg_xpu3_esr_s
{
  u32 cfg : 1;
  u32 client : 1;
  u32 cfgmulti : 1;
  u32 clmulti : 1;
  u32 reserved0 : 28;
};

/* Union definition of register */
union ipa_hwio_def_ipa_ms_mpu_cfg_xpu3_esr_u
{
  struct ipa_hwio_def_ipa_ms_mpu_cfg_xpu3_esr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_MS_MPU_CFG_XPU3_ESRRESTORE
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_ms_mpu_cfg_xpu3_esrrestore_s
{
  u32 cfg : 1;
  u32 client : 1;
  u32 cfgmulti : 1;
  u32 clmulti : 1;
  u32 reserved0 : 28;
};

/* Union definition of register */
union ipa_hwio_def_ipa_ms_mpu_cfg_xpu3_esrrestore_u
{
  struct ipa_hwio_def_ipa_ms_mpu_cfg_xpu3_esrrestore_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_MS_MPU_CFG_XPU3_ESYNR0
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_ms_mpu_cfg_xpu3_esynr0_s
{
  u32 xprotns : 1;
  u32 awrite : 1;
  u32 xinst : 1;
  u32 xpriv : 1;
  u32 reserved0 : 4;
  u32 qad : 8;
  u32 alen : 8;
  u32 asize : 3;
  u32 reserved1 : 2;
  u32 burstlen : 1;
  u32 ac : 1;
  u32 reserved2 : 1;
};

/* Union definition of register */
union ipa_hwio_def_ipa_ms_mpu_cfg_xpu3_esynr0_u
{
  struct ipa_hwio_def_ipa_ms_mpu_cfg_xpu3_esynr0_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_MS_MPU_CFG_XPU3_ESYNR1
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_ms_mpu_cfg_xpu3_esynr1_s
{
  u32 mid : 8;
  u32 pid : 5;
  u32 bid : 3;
  u32 vmid : 8;
  u32 tid : 8;
};

/* Union definition of register */
union ipa_hwio_def_ipa_ms_mpu_cfg_xpu3_esynr1_u
{
  struct ipa_hwio_def_ipa_ms_mpu_cfg_xpu3_esynr1_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_MS_MPU_CFG_XPU3_ESYNR2
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_ms_mpu_cfg_xpu3_esynr2_s
{
  u32 memtype : 3;
  u32 reserved0 : 4;
  u32 transient : 1;
  u32 noallocate : 1;
  u32 ooowr : 1;
  u32 ooord : 1;
  u32 orderedwr : 1;
  u32 orderedrd : 1;
  u32 portmrel : 1;
  u32 innerwritethrough : 1;
  u32 innertransient : 1;
  u32 innershared : 1;
  u32 innercacheable : 1;
  u32 innernoallocate : 1;
  u32 writethrough : 1;
  u32 shared : 1;
  u32 full : 1;
  u32 exclusive : 1;
  u32 error : 1;
  u32 earlywrresp : 1;
  u32 device_type : 2;
  u32 device : 1;
  u32 cacheable : 1;
  u32 burst : 1;
  u32 bar : 2;
};

/* Union definition of register */
union ipa_hwio_def_ipa_ms_mpu_cfg_xpu3_esynr2_u
{
  struct ipa_hwio_def_ipa_ms_mpu_cfg_xpu3_esynr2_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_MS_MPU_CFG_XPU3_EAR1
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_ms_mpu_cfg_xpu3_ear1_s
{
  u32 addr_63_32 : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_ms_mpu_cfg_xpu3_ear1_u
{
  struct ipa_hwio_def_ipa_ms_mpu_cfg_xpu3_ear1_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_MS_MPU_CFG_XPU3_ESYNR3
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_ms_mpu_cfg_xpu3_esynr3_s
{
  u32 nonsec_ad_rg_match : 8;
  u32 sec_ad_rg_match : 8;
  u32 reserved0 : 16;
};

/* Union definition of register */
union ipa_hwio_def_ipa_ms_mpu_cfg_xpu3_esynr3_u
{
  struct ipa_hwio_def_ipa_ms_mpu_cfg_xpu3_esynr3_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_MS_MPU_CFG_XPU3_ESYNR4
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_ms_mpu_cfg_xpu3_esynr4_s
{
  u32 auattr : 16;
  u32 acgranuletrans : 1;
  u32 asid : 5;
  u32 acacheoptype : 4;
  u32 reserved0 : 6;
};

/* Union definition of register */
union ipa_hwio_def_ipa_ms_mpu_cfg_xpu3_esynr4_u
{
  struct ipa_hwio_def_ipa_ms_mpu_cfg_xpu3_esynr4_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_MS_MPU_CFG_XPU3_RGN_START0_SHADOW
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_ms_mpu_cfg_xpu3_rgn_start0_shadow_s
{
  u32 reserved0 : 12;
  u32 addr_31_0 : 20;
};

/* Union definition of register */
union ipa_hwio_def_ipa_ms_mpu_cfg_xpu3_rgn_start0_shadow_u
{
  struct ipa_hwio_def_ipa_ms_mpu_cfg_xpu3_rgn_start0_shadow_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_MS_MPU_CFG_XPU3_RGN_END0_SHADOW
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_ms_mpu_cfg_xpu3_rgn_end0_shadow_s
{
  u32 reserved0 : 12;
  u32 addr_31_0 : 20;
};

/* Union definition of register */
union ipa_hwio_def_ipa_ms_mpu_cfg_xpu3_rgn_end0_shadow_u
{
  struct ipa_hwio_def_ipa_ms_mpu_cfg_xpu3_rgn_end0_shadow_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_MS_MPU_CFG_XPU3_QAD0_EAR0
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_ms_mpu_cfg_xpu3_qad0_ear0_s
{
  u32 addr_31_0 : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_ms_mpu_cfg_xpu3_qad0_ear0_u
{
  struct ipa_hwio_def_ipa_ms_mpu_cfg_xpu3_qad0_ear0_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_MS_MPU_CFG_XPU3_QAD0_ESR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_ms_mpu_cfg_xpu3_qad0_esr_s
{
  u32 cfg : 1;
  u32 client : 1;
  u32 cfgmulti : 1;
  u32 clmulti : 1;
  u32 reserved0 : 28;
};

/* Union definition of register */
union ipa_hwio_def_ipa_ms_mpu_cfg_xpu3_qad0_esr_u
{
  struct ipa_hwio_def_ipa_ms_mpu_cfg_xpu3_qad0_esr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_MS_MPU_CFG_XPU3_QAD0_ESRRESTORE
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_ms_mpu_cfg_xpu3_qad0_esrrestore_s
{
  u32 cfg : 1;
  u32 client : 1;
  u32 cfgmulti : 1;
  u32 clmulti : 1;
  u32 reserved0 : 28;
};

/* Union definition of register */
union ipa_hwio_def_ipa_ms_mpu_cfg_xpu3_qad0_esrrestore_u
{
  struct ipa_hwio_def_ipa_ms_mpu_cfg_xpu3_qad0_esrrestore_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_MS_MPU_CFG_XPU3_QAD0_ESYNR0
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_ms_mpu_cfg_xpu3_qad0_esynr0_s
{
  u32 xprotns : 1;
  u32 awrite : 1;
  u32 xinst : 1;
  u32 xpriv : 1;
  u32 reserved0 : 4;
  u32 qad : 8;
  u32 alen : 8;
  u32 asize : 3;
  u32 reserved1 : 2;
  u32 burstlen : 1;
  u32 ac : 1;
  u32 reserved2 : 1;
};

/* Union definition of register */
union ipa_hwio_def_ipa_ms_mpu_cfg_xpu3_qad0_esynr0_u
{
  struct ipa_hwio_def_ipa_ms_mpu_cfg_xpu3_qad0_esynr0_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_MS_MPU_CFG_XPU3_QAD0_ESYNR1
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_ms_mpu_cfg_xpu3_qad0_esynr1_s
{
  u32 mid : 8;
  u32 pid : 5;
  u32 bid : 3;
  u32 vmid : 8;
  u32 tid : 8;
};

/* Union definition of register */
union ipa_hwio_def_ipa_ms_mpu_cfg_xpu3_qad0_esynr1_u
{
  struct ipa_hwio_def_ipa_ms_mpu_cfg_xpu3_qad0_esynr1_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_MS_MPU_CFG_XPU3_QAD0_ESYNR2
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_ms_mpu_cfg_xpu3_qad0_esynr2_s
{
  u32 memtype : 3;
  u32 reserved0 : 4;
  u32 transient : 1;
  u32 noallocate : 1;
  u32 ooowr : 1;
  u32 ooord : 1;
  u32 orderedwr : 1;
  u32 orderedrd : 1;
  u32 portmrel : 1;
  u32 innerwritethrough : 1;
  u32 innertransient : 1;
  u32 innershared : 1;
  u32 innercacheable : 1;
  u32 innernoallocate : 1;
  u32 writethrough : 1;
  u32 shared : 1;
  u32 full : 1;
  u32 exclusive : 1;
  u32 error : 1;
  u32 earlywrresp : 1;
  u32 device_type : 2;
  u32 device : 1;
  u32 cacheable : 1;
  u32 burst : 1;
  u32 bar : 2;
};

/* Union definition of register */
union ipa_hwio_def_ipa_ms_mpu_cfg_xpu3_qad0_esynr2_u
{
  struct ipa_hwio_def_ipa_ms_mpu_cfg_xpu3_qad0_esynr2_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_MS_MPU_CFG_XPU3_QAD0_EAR1
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_ms_mpu_cfg_xpu3_qad0_ear1_s
{
  u32 addr_63_32 : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_ms_mpu_cfg_xpu3_qad0_ear1_u
{
  struct ipa_hwio_def_ipa_ms_mpu_cfg_xpu3_qad0_ear1_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_MS_MPU_CFG_XPU3_QAD0_ESYNR3
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_ms_mpu_cfg_xpu3_qad0_esynr3_s
{
  u32 nonsec_ad_rg_match : 8;
  u32 sec_ad_rg_match : 8;
  u32 reserved0 : 16;
};

/* Union definition of register */
union ipa_hwio_def_ipa_ms_mpu_cfg_xpu3_qad0_esynr3_u
{
  struct ipa_hwio_def_ipa_ms_mpu_cfg_xpu3_qad0_esynr3_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_MS_MPU_CFG_XPU3_QAD0_ESYNR4
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_ms_mpu_cfg_xpu3_qad0_esynr4_s
{
  u32 auattr : 16;
  u32 acgranuletrans : 1;
  u32 asid : 5;
  u32 acacheoptype : 4;
  u32 reserved0 : 6;
};

/* Union definition of register */
union ipa_hwio_def_ipa_ms_mpu_cfg_xpu3_qad0_esynr4_u
{
  struct ipa_hwio_def_ipa_ms_mpu_cfg_xpu3_qad0_esynr4_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_MS_MPU_CFG_XPU3_QAD0_RGN_START0_SHADOW
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_ms_mpu_cfg_xpu3_qad0_rgn_start0_shadow_s
{
  u32 reserved0 : 12;
  u32 addr_31_0 : 20;
};

/* Union definition of register */
union ipa_hwio_def_ipa_ms_mpu_cfg_xpu3_qad0_rgn_start0_shadow_u
{
  struct ipa_hwio_def_ipa_ms_mpu_cfg_xpu3_qad0_rgn_start0_shadow_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_MS_MPU_CFG_XPU3_QAD0_RGN_END0_SHADOW
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_ms_mpu_cfg_xpu3_qad0_rgn_end0_shadow_s
{
  u32 reserved0 : 12;
  u32 addr_31_0 : 20;
};

/* Union definition of register */
union ipa_hwio_def_ipa_ms_mpu_cfg_xpu3_qad0_rgn_end0_shadow_u
{
  struct ipa_hwio_def_ipa_ms_mpu_cfg_xpu3_qad0_rgn_end0_shadow_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_MS_MPU_CFG_XPU3_QAD1_EAR0
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_ms_mpu_cfg_xpu3_qad1_ear0_s
{
  u32 addr_31_0 : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_ms_mpu_cfg_xpu3_qad1_ear0_u
{
  struct ipa_hwio_def_ipa_ms_mpu_cfg_xpu3_qad1_ear0_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_MS_MPU_CFG_XPU3_QAD1_ESR
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_ms_mpu_cfg_xpu3_qad1_esr_s
{
  u32 cfg : 1;
  u32 client : 1;
  u32 cfgmulti : 1;
  u32 clmulti : 1;
  u32 reserved0 : 28;
};

/* Union definition of register */
union ipa_hwio_def_ipa_ms_mpu_cfg_xpu3_qad1_esr_u
{
  struct ipa_hwio_def_ipa_ms_mpu_cfg_xpu3_qad1_esr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_MS_MPU_CFG_XPU3_QAD1_ESRRESTORE
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_ms_mpu_cfg_xpu3_qad1_esrrestore_s
{
  u32 cfg : 1;
  u32 client : 1;
  u32 cfgmulti : 1;
  u32 clmulti : 1;
  u32 reserved0 : 28;
};

/* Union definition of register */
union ipa_hwio_def_ipa_ms_mpu_cfg_xpu3_qad1_esrrestore_u
{
  struct ipa_hwio_def_ipa_ms_mpu_cfg_xpu3_qad1_esrrestore_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_MS_MPU_CFG_XPU3_QAD1_ESYNR0
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_ms_mpu_cfg_xpu3_qad1_esynr0_s
{
  u32 xprotns : 1;
  u32 awrite : 1;
  u32 xinst : 1;
  u32 xpriv : 1;
  u32 reserved0 : 4;
  u32 qad : 8;
  u32 alen : 8;
  u32 asize : 3;
  u32 reserved1 : 2;
  u32 burstlen : 1;
  u32 ac : 1;
  u32 reserved2 : 1;
};

/* Union definition of register */
union ipa_hwio_def_ipa_ms_mpu_cfg_xpu3_qad1_esynr0_u
{
  struct ipa_hwio_def_ipa_ms_mpu_cfg_xpu3_qad1_esynr0_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_MS_MPU_CFG_XPU3_QAD1_ESYNR1
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_ms_mpu_cfg_xpu3_qad1_esynr1_s
{
  u32 mid : 8;
  u32 pid : 5;
  u32 bid : 3;
  u32 vmid : 8;
  u32 tid : 8;
};

/* Union definition of register */
union ipa_hwio_def_ipa_ms_mpu_cfg_xpu3_qad1_esynr1_u
{
  struct ipa_hwio_def_ipa_ms_mpu_cfg_xpu3_qad1_esynr1_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_MS_MPU_CFG_XPU3_QAD1_ESYNR2
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_ms_mpu_cfg_xpu3_qad1_esynr2_s
{
  u32 memtype : 3;
  u32 reserved0 : 4;
  u32 transient : 1;
  u32 noallocate : 1;
  u32 ooowr : 1;
  u32 ooord : 1;
  u32 orderedwr : 1;
  u32 orderedrd : 1;
  u32 portmrel : 1;
  u32 innerwritethrough : 1;
  u32 innertransient : 1;
  u32 innershared : 1;
  u32 innercacheable : 1;
  u32 innernoallocate : 1;
  u32 writethrough : 1;
  u32 shared : 1;
  u32 full : 1;
  u32 exclusive : 1;
  u32 error : 1;
  u32 earlywrresp : 1;
  u32 device_type : 2;
  u32 device : 1;
  u32 cacheable : 1;
  u32 burst : 1;
  u32 bar : 2;
};

/* Union definition of register */
union ipa_hwio_def_ipa_ms_mpu_cfg_xpu3_qad1_esynr2_u
{
  struct ipa_hwio_def_ipa_ms_mpu_cfg_xpu3_qad1_esynr2_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_MS_MPU_CFG_XPU3_QAD1_EAR1
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_ms_mpu_cfg_xpu3_qad1_ear1_s
{
  u32 addr_63_32 : 32;
};

/* Union definition of register */
union ipa_hwio_def_ipa_ms_mpu_cfg_xpu3_qad1_ear1_u
{
  struct ipa_hwio_def_ipa_ms_mpu_cfg_xpu3_qad1_ear1_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_MS_MPU_CFG_XPU3_QAD1_ESYNR3
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_ms_mpu_cfg_xpu3_qad1_esynr3_s
{
  u32 nonsec_ad_rg_match : 8;
  u32 sec_ad_rg_match : 8;
  u32 reserved0 : 16;
};

/* Union definition of register */
union ipa_hwio_def_ipa_ms_mpu_cfg_xpu3_qad1_esynr3_u
{
  struct ipa_hwio_def_ipa_ms_mpu_cfg_xpu3_qad1_esynr3_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_MS_MPU_CFG_XPU3_QAD1_ESYNR4
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_ms_mpu_cfg_xpu3_qad1_esynr4_s
{
  u32 auattr : 16;
  u32 acgranuletrans : 1;
  u32 asid : 5;
  u32 acacheoptype : 4;
  u32 reserved0 : 6;
};

/* Union definition of register */
union ipa_hwio_def_ipa_ms_mpu_cfg_xpu3_qad1_esynr4_u
{
  struct ipa_hwio_def_ipa_ms_mpu_cfg_xpu3_qad1_esynr4_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_MS_MPU_CFG_XPU3_QAD1_RGN_START0_SHADOW
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_ms_mpu_cfg_xpu3_qad1_rgn_start0_shadow_s
{
  u32 reserved0 : 12;
  u32 addr_31_0 : 20;
};

/* Union definition of register */
union ipa_hwio_def_ipa_ms_mpu_cfg_xpu3_qad1_rgn_start0_shadow_u
{
  struct ipa_hwio_def_ipa_ms_mpu_cfg_xpu3_qad1_rgn_start0_shadow_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_MS_MPU_CFG_XPU3_QAD1_RGN_END0_SHADOW
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_ms_mpu_cfg_xpu3_qad1_rgn_end0_shadow_s
{
  u32 reserved0 : 12;
  u32 addr_31_0 : 20;
};

/* Union definition of register */
union ipa_hwio_def_ipa_ms_mpu_cfg_xpu3_qad1_rgn_end0_shadow_u
{
  struct ipa_hwio_def_ipa_ms_mpu_cfg_xpu3_qad1_rgn_end0_shadow_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_MS_MPU_CFG_XPU3_RGN_OWNERSTATUSr
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_ms_mpu_cfg_xpu3_rgn_ownerstatusr_s
{
  u32 rgownerstatus : 10;
  u32 reserved0 : 22;
};

/* Union definition of register */
union ipa_hwio_def_ipa_ms_mpu_cfg_xpu3_rgn_ownerstatusr_u
{
  struct ipa_hwio_def_ipa_ms_mpu_cfg_xpu3_rgn_ownerstatusr_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_MS_MPU_CFG_XPU3_RGn_GCR0
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_ms_mpu_cfg_xpu3_rgn_gcr0_s
{
  u32 rg_owner : 3;
  u32 reserved0 : 5;
  u32 rg_sec_apps : 1;
  u32 reserved1 : 23;
};

/* Union definition of register */
union ipa_hwio_def_ipa_ms_mpu_cfg_xpu3_rgn_gcr0_u
{
  struct ipa_hwio_def_ipa_ms_mpu_cfg_xpu3_rgn_gcr0_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_MS_MPU_CFG_XPU3_RGn_GCR1
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_ms_mpu_cfg_xpu3_rgn_gcr1_s
{
  u32 reserved0 : 31;
  u32 pd : 1;
};

/* Union definition of register */
union ipa_hwio_def_ipa_ms_mpu_cfg_xpu3_rgn_gcr1_u
{
  struct ipa_hwio_def_ipa_ms_mpu_cfg_xpu3_rgn_gcr1_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_MS_MPU_CFG_XPU3_RGn_GCR2
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_ms_mpu_cfg_xpu3_rgn_gcr2_s
{
  u32 csrc : 1;
  u32 asrc : 1;
  u32 reserved0 : 30;
};

/* Union definition of register */
union ipa_hwio_def_ipa_ms_mpu_cfg_xpu3_rgn_gcr2_u
{
  struct ipa_hwio_def_ipa_ms_mpu_cfg_xpu3_rgn_gcr2_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_MS_MPU_CFG_XPU3_RGn_GCR3
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_ms_mpu_cfg_xpu3_rgn_gcr3_s
{
  u32 secure_access_lock : 1;
  u32 reserved0 : 31;
};

/* Union definition of register */
union ipa_hwio_def_ipa_ms_mpu_cfg_xpu3_rgn_gcr3_u
{
  struct ipa_hwio_def_ipa_ms_mpu_cfg_xpu3_rgn_gcr3_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_MS_MPU_CFG_XPU3_RGn_CR0
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_ms_mpu_cfg_xpu3_rgn_cr0_s
{
  u32 rgsclrden_apps : 1;
  u32 reserved0 : 31;
};

/* Union definition of register */
union ipa_hwio_def_ipa_ms_mpu_cfg_xpu3_rgn_cr0_u
{
  struct ipa_hwio_def_ipa_ms_mpu_cfg_xpu3_rgn_cr0_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_MS_MPU_CFG_XPU3_RGn_CR1
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_ms_mpu_cfg_xpu3_rgn_cr1_s
{
  u32 rgclrden : 3;
  u32 reserved0 : 29;
};

/* Union definition of register */
union ipa_hwio_def_ipa_ms_mpu_cfg_xpu3_rgn_cr1_u
{
  struct ipa_hwio_def_ipa_ms_mpu_cfg_xpu3_rgn_cr1_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_MS_MPU_CFG_XPU3_RGn_CR2
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_ms_mpu_cfg_xpu3_rgn_cr2_s
{
  u32 rgsclwren_apps : 1;
  u32 reserved0 : 31;
};

/* Union definition of register */
union ipa_hwio_def_ipa_ms_mpu_cfg_xpu3_rgn_cr2_u
{
  struct ipa_hwio_def_ipa_ms_mpu_cfg_xpu3_rgn_cr2_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_MS_MPU_CFG_XPU3_RGn_CR3
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_ms_mpu_cfg_xpu3_rgn_cr3_s
{
  u32 rgclwren : 3;
  u32 reserved0 : 29;
};

/* Union definition of register */
union ipa_hwio_def_ipa_ms_mpu_cfg_xpu3_rgn_cr3_u
{
  struct ipa_hwio_def_ipa_ms_mpu_cfg_xpu3_rgn_cr3_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_MS_MPU_CFG_XPU3_RGn_START0
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_ms_mpu_cfg_xpu3_rgn_start0_s
{
  u32 reserved0 : 12;
  u32 addr_31_0 : 20;
};

/* Union definition of register */
union ipa_hwio_def_ipa_ms_mpu_cfg_xpu3_rgn_start0_u
{
  struct ipa_hwio_def_ipa_ms_mpu_cfg_xpu3_rgn_start0_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_MS_MPU_CFG_XPU3_RGn_END0
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_ms_mpu_cfg_xpu3_rgn_end0_s
{
  u32 reserved0 : 12;
  u32 addr_31_0 : 20;
};

/* Union definition of register */
union ipa_hwio_def_ipa_ms_mpu_cfg_xpu3_rgn_end0_u
{
  struct ipa_hwio_def_ipa_ms_mpu_cfg_xpu3_rgn_end0_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of register: IPA_0_IPA_RSRC_GRP_CFG_EXT
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_ipa_0_ipa_rsrc_grp_cfg_ext_s
{
  u32 src_grp_2nd_priority_special_valid : 1;
  u32 reserved0 : 3;
  u32 src_grp_2nd_priority_special_index : 3;
  u32 reserved1 : 25;
};

/* Union definition of register */
union ipa_hwio_def_ipa_0_ipa_rsrc_grp_cfg_ext_u
{
  struct ipa_hwio_def_ipa_0_ipa_rsrc_grp_cfg_ext_s def;
  u32 value;
};

/*===========================================================================*/
/*!
  @brief Bit Field definition of fc_stats
*/
/*===========================================================================*/
/* Structure definition of register */
struct ipa_hwio_def_fc_stats_state_s
{
  u32 reserved0 : 16;
  u32 flow_control : 1;
  u32 flow_control_primary : 1;
  u32 flow_control_secondary : 1;
  u32 pending_flow_control : 1;
  u32 reserved1 : 12;
};

/* Union definition of register */
union ipa_hwio_def_fc_stats_state_u
{
  struct ipa_hwio_def_fc_stats_state_s def;
  u32 value;
};

#endif /* __IPA_HWIO_DEF_H__ */
