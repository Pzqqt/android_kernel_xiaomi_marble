/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (c) 2021, The Linux Foundation. All rights reserved.
 */

#ifndef __GSI_HWIO_H__
#define __GSI_HWIO_H__
/**
  @file gsi_hwio.h
  @brief Auto-generated HWIO interface include file.

  This file contains HWIO register definitions for the following modules:
    IPA_0_GSI_TOP_.*

  'Include' filters applied: <none>
  'Exclude' filters applied: RESERVED DUMMY

  Attribute definitions for the HWIO_*_ATTR macros are as follows:
    0x0: Command register
    0x1: Read-Only
    0x2: Write-Only
    0x3: Read/Write
*/

/*----------------------------------------------------------------------------
 * MODULE: GSI
 *--------------------------------------------------------------------------*/

#define GSI_REG_BASE                                                                                     (IPA_0_IPA_WRAPPER_BASE      + 0x00004000)
#define GSI_REG_BASE_PHYS                                                                                (IPA_0_IPA_WRAPPER_BASE_PHYS + 0x00004000)
#define GSI_REG_BASE_OFFS                                                                                0x00004000

#define HWIO_GSI_CFG_ADDR                                                                                (GSI_REG_BASE      + 0x00000000)
#define HWIO_GSI_CFG_PHYS                                                                                (GSI_REG_BASE_PHYS + 0x00000000)
#define HWIO_GSI_CFG_OFFS                                                                                (GSI_REG_BASE_OFFS + 0x00000000)
#define HWIO_GSI_CFG_RMSK                                                                                     0xf3f
#define HWIO_GSI_CFG_ATTR                                                                                       0x3
#define HWIO_GSI_CFG_IN          \
        in_dword_masked(HWIO_GSI_CFG_ADDR, HWIO_GSI_CFG_RMSK)
#define HWIO_GSI_CFG_INM(m)      \
        in_dword_masked(HWIO_GSI_CFG_ADDR, m)
#define HWIO_GSI_CFG_OUT(v)      \
        out_dword(HWIO_GSI_CFG_ADDR,v)
#define HWIO_GSI_CFG_OUTM(m,v) \
        out_dword_masked_ns(HWIO_GSI_CFG_ADDR,m,v,HWIO_GSI_CFG_IN)
#define HWIO_GSI_CFG_SLEEP_CLK_DIV_BMSK                                                                       0xf00
#define HWIO_GSI_CFG_SLEEP_CLK_DIV_SHFT                                                                         0x8
#define HWIO_GSI_CFG_BP_MTRIX_DISABLE_BMSK                                                                     0x20
#define HWIO_GSI_CFG_BP_MTRIX_DISABLE_SHFT                                                                      0x5
#define HWIO_GSI_CFG_GSI_PWR_CLPS_BMSK                                                                         0x10
#define HWIO_GSI_CFG_GSI_PWR_CLPS_SHFT                                                                          0x4
#define HWIO_GSI_CFG_UC_IS_MCS_BMSK                                                                             0x8
#define HWIO_GSI_CFG_UC_IS_MCS_SHFT                                                                             0x3
#define HWIO_GSI_CFG_DOUBLE_MCS_CLK_FREQ_BMSK                                                                   0x4
#define HWIO_GSI_CFG_DOUBLE_MCS_CLK_FREQ_SHFT                                                                   0x2
#define HWIO_GSI_CFG_MCS_ENABLE_BMSK                                                                            0x2
#define HWIO_GSI_CFG_MCS_ENABLE_SHFT                                                                            0x1
#define HWIO_GSI_CFG_GSI_ENABLE_BMSK                                                                            0x1
#define HWIO_GSI_CFG_GSI_ENABLE_SHFT                                                                            0x0

#define HWIO_GSI_MANAGER_MCS_CODE_VER_ADDR                                                               (GSI_REG_BASE      + 0x00000008)
#define HWIO_GSI_MANAGER_MCS_CODE_VER_PHYS                                                               (GSI_REG_BASE_PHYS + 0x00000008)
#define HWIO_GSI_MANAGER_MCS_CODE_VER_OFFS                                                               (GSI_REG_BASE_OFFS + 0x00000008)
#define HWIO_GSI_MANAGER_MCS_CODE_VER_RMSK                                                               0xffffffff
#define HWIO_GSI_MANAGER_MCS_CODE_VER_ATTR                                                                      0x3
#define HWIO_GSI_MANAGER_MCS_CODE_VER_IN          \
        in_dword_masked(HWIO_GSI_MANAGER_MCS_CODE_VER_ADDR, HWIO_GSI_MANAGER_MCS_CODE_VER_RMSK)
#define HWIO_GSI_MANAGER_MCS_CODE_VER_INM(m)      \
        in_dword_masked(HWIO_GSI_MANAGER_MCS_CODE_VER_ADDR, m)
#define HWIO_GSI_MANAGER_MCS_CODE_VER_OUT(v)      \
        out_dword(HWIO_GSI_MANAGER_MCS_CODE_VER_ADDR,v)
#define HWIO_GSI_MANAGER_MCS_CODE_VER_OUTM(m,v) \
        out_dword_masked_ns(HWIO_GSI_MANAGER_MCS_CODE_VER_ADDR,m,v,HWIO_GSI_MANAGER_MCS_CODE_VER_IN)
#define HWIO_GSI_MANAGER_MCS_CODE_VER_VER_BMSK                                                           0xffffffff
#define HWIO_GSI_MANAGER_MCS_CODE_VER_VER_SHFT                                                                  0x0

#define HWIO_GSI_ZEROS_ADDR                                                                              (GSI_REG_BASE      + 0x00000010)
#define HWIO_GSI_ZEROS_PHYS                                                                              (GSI_REG_BASE_PHYS + 0x00000010)
#define HWIO_GSI_ZEROS_OFFS                                                                              (GSI_REG_BASE_OFFS + 0x00000010)
#define HWIO_GSI_ZEROS_RMSK                                                                              0xffffffff
#define HWIO_GSI_ZEROS_ATTR                                                                                     0x1
#define HWIO_GSI_ZEROS_IN          \
        in_dword_masked(HWIO_GSI_ZEROS_ADDR, HWIO_GSI_ZEROS_RMSK)
#define HWIO_GSI_ZEROS_INM(m)      \
        in_dword_masked(HWIO_GSI_ZEROS_ADDR, m)
#define HWIO_GSI_ZEROS_ZEROS_BMSK                                                                        0xffffffff
#define HWIO_GSI_ZEROS_ZEROS_SHFT                                                                               0x0

#define HWIO_GSI_PERIPH_BASE_ADDR_LSB_ADDR                                                               (GSI_REG_BASE      + 0x00000018)
#define HWIO_GSI_PERIPH_BASE_ADDR_LSB_PHYS                                                               (GSI_REG_BASE_PHYS + 0x00000018)
#define HWIO_GSI_PERIPH_BASE_ADDR_LSB_OFFS                                                               (GSI_REG_BASE_OFFS + 0x00000018)
#define HWIO_GSI_PERIPH_BASE_ADDR_LSB_RMSK                                                               0xffffffff
#define HWIO_GSI_PERIPH_BASE_ADDR_LSB_ATTR                                                                      0x3
#define HWIO_GSI_PERIPH_BASE_ADDR_LSB_IN          \
        in_dword_masked(HWIO_GSI_PERIPH_BASE_ADDR_LSB_ADDR, HWIO_GSI_PERIPH_BASE_ADDR_LSB_RMSK)
#define HWIO_GSI_PERIPH_BASE_ADDR_LSB_INM(m)      \
        in_dword_masked(HWIO_GSI_PERIPH_BASE_ADDR_LSB_ADDR, m)
#define HWIO_GSI_PERIPH_BASE_ADDR_LSB_OUT(v)      \
        out_dword(HWIO_GSI_PERIPH_BASE_ADDR_LSB_ADDR,v)
#define HWIO_GSI_PERIPH_BASE_ADDR_LSB_OUTM(m,v) \
        out_dword_masked_ns(HWIO_GSI_PERIPH_BASE_ADDR_LSB_ADDR,m,v,HWIO_GSI_PERIPH_BASE_ADDR_LSB_IN)
#define HWIO_GSI_PERIPH_BASE_ADDR_LSB_BASE_ADDR_BMSK                                                     0xffffffff
#define HWIO_GSI_PERIPH_BASE_ADDR_LSB_BASE_ADDR_SHFT                                                            0x0

#define HWIO_GSI_PERIPH_BASE_ADDR_MSB_ADDR                                                               (GSI_REG_BASE      + 0x0000001c)
#define HWIO_GSI_PERIPH_BASE_ADDR_MSB_PHYS                                                               (GSI_REG_BASE_PHYS + 0x0000001c)
#define HWIO_GSI_PERIPH_BASE_ADDR_MSB_OFFS                                                               (GSI_REG_BASE_OFFS + 0x0000001c)
#define HWIO_GSI_PERIPH_BASE_ADDR_MSB_RMSK                                                               0xffffffff
#define HWIO_GSI_PERIPH_BASE_ADDR_MSB_ATTR                                                                      0x3
#define HWIO_GSI_PERIPH_BASE_ADDR_MSB_IN          \
        in_dword_masked(HWIO_GSI_PERIPH_BASE_ADDR_MSB_ADDR, HWIO_GSI_PERIPH_BASE_ADDR_MSB_RMSK)
#define HWIO_GSI_PERIPH_BASE_ADDR_MSB_INM(m)      \
        in_dword_masked(HWIO_GSI_PERIPH_BASE_ADDR_MSB_ADDR, m)
#define HWIO_GSI_PERIPH_BASE_ADDR_MSB_OUT(v)      \
        out_dword(HWIO_GSI_PERIPH_BASE_ADDR_MSB_ADDR,v)
#define HWIO_GSI_PERIPH_BASE_ADDR_MSB_OUTM(m,v) \
        out_dword_masked_ns(HWIO_GSI_PERIPH_BASE_ADDR_MSB_ADDR,m,v,HWIO_GSI_PERIPH_BASE_ADDR_MSB_IN)
#define HWIO_GSI_PERIPH_BASE_ADDR_MSB_BASE_ADDR_BMSK                                                     0xffffffff
#define HWIO_GSI_PERIPH_BASE_ADDR_MSB_BASE_ADDR_SHFT                                                            0x0

#define HWIO_GSI_CGC_CTRL_ADDR                                                                           (GSI_REG_BASE      + 0x00000020)
#define HWIO_GSI_CGC_CTRL_PHYS                                                                           (GSI_REG_BASE_PHYS + 0x00000020)
#define HWIO_GSI_CGC_CTRL_OFFS                                                                           (GSI_REG_BASE_OFFS + 0x00000020)
#define HWIO_GSI_CGC_CTRL_RMSK                                                                               0xffff
#define HWIO_GSI_CGC_CTRL_ATTR                                                                                  0x3
#define HWIO_GSI_CGC_CTRL_IN          \
        in_dword_masked(HWIO_GSI_CGC_CTRL_ADDR, HWIO_GSI_CGC_CTRL_RMSK)
#define HWIO_GSI_CGC_CTRL_INM(m)      \
        in_dword_masked(HWIO_GSI_CGC_CTRL_ADDR, m)
#define HWIO_GSI_CGC_CTRL_OUT(v)      \
        out_dword(HWIO_GSI_CGC_CTRL_ADDR,v)
#define HWIO_GSI_CGC_CTRL_OUTM(m,v) \
        out_dword_masked_ns(HWIO_GSI_CGC_CTRL_ADDR,m,v,HWIO_GSI_CGC_CTRL_IN)
#define HWIO_GSI_CGC_CTRL_REGION_16_HW_CGC_EN_BMSK                                                           0x8000
#define HWIO_GSI_CGC_CTRL_REGION_16_HW_CGC_EN_SHFT                                                              0xf
#define HWIO_GSI_CGC_CTRL_REGION_15_HW_CGC_EN_BMSK                                                           0x4000
#define HWIO_GSI_CGC_CTRL_REGION_15_HW_CGC_EN_SHFT                                                              0xe
#define HWIO_GSI_CGC_CTRL_REGION_14_HW_CGC_EN_BMSK                                                           0x2000
#define HWIO_GSI_CGC_CTRL_REGION_14_HW_CGC_EN_SHFT                                                              0xd
#define HWIO_GSI_CGC_CTRL_REGION_13_HW_CGC_EN_BMSK                                                           0x1000
#define HWIO_GSI_CGC_CTRL_REGION_13_HW_CGC_EN_SHFT                                                              0xc
#define HWIO_GSI_CGC_CTRL_REGION_12_HW_CGC_EN_BMSK                                                            0x800
#define HWIO_GSI_CGC_CTRL_REGION_12_HW_CGC_EN_SHFT                                                              0xb
#define HWIO_GSI_CGC_CTRL_REGION_11_HW_CGC_EN_BMSK                                                            0x400
#define HWIO_GSI_CGC_CTRL_REGION_11_HW_CGC_EN_SHFT                                                              0xa
#define HWIO_GSI_CGC_CTRL_REGION_10_HW_CGC_EN_BMSK                                                            0x200
#define HWIO_GSI_CGC_CTRL_REGION_10_HW_CGC_EN_SHFT                                                              0x9
#define HWIO_GSI_CGC_CTRL_REGION_9_HW_CGC_EN_BMSK                                                             0x100
#define HWIO_GSI_CGC_CTRL_REGION_9_HW_CGC_EN_SHFT                                                               0x8
#define HWIO_GSI_CGC_CTRL_REGION_8_HW_CGC_EN_BMSK                                                              0x80
#define HWIO_GSI_CGC_CTRL_REGION_8_HW_CGC_EN_SHFT                                                               0x7
#define HWIO_GSI_CGC_CTRL_REGION_7_HW_CGC_EN_BMSK                                                              0x40
#define HWIO_GSI_CGC_CTRL_REGION_7_HW_CGC_EN_SHFT                                                               0x6
#define HWIO_GSI_CGC_CTRL_REGION_6_HW_CGC_EN_BMSK                                                              0x20
#define HWIO_GSI_CGC_CTRL_REGION_6_HW_CGC_EN_SHFT                                                               0x5
#define HWIO_GSI_CGC_CTRL_REGION_5_HW_CGC_EN_BMSK                                                              0x10
#define HWIO_GSI_CGC_CTRL_REGION_5_HW_CGC_EN_SHFT                                                               0x4
#define HWIO_GSI_CGC_CTRL_REGION_4_HW_CGC_EN_BMSK                                                               0x8
#define HWIO_GSI_CGC_CTRL_REGION_4_HW_CGC_EN_SHFT                                                               0x3
#define HWIO_GSI_CGC_CTRL_REGION_3_HW_CGC_EN_BMSK                                                               0x4
#define HWIO_GSI_CGC_CTRL_REGION_3_HW_CGC_EN_SHFT                                                               0x2
#define HWIO_GSI_CGC_CTRL_REGION_2_HW_CGC_EN_BMSK                                                               0x2
#define HWIO_GSI_CGC_CTRL_REGION_2_HW_CGC_EN_SHFT                                                               0x1
#define HWIO_GSI_CGC_CTRL_REGION_1_HW_CGC_EN_BMSK                                                               0x1
#define HWIO_GSI_CGC_CTRL_REGION_1_HW_CGC_EN_SHFT                                                               0x0

#define HWIO_GSI_MOQA_CFG_ADDR                                                                           (GSI_REG_BASE      + 0x00000030)
#define HWIO_GSI_MOQA_CFG_PHYS                                                                           (GSI_REG_BASE_PHYS + 0x00000030)
#define HWIO_GSI_MOQA_CFG_OFFS                                                                           (GSI_REG_BASE_OFFS + 0x00000030)
#define HWIO_GSI_MOQA_CFG_RMSK                                                                             0xffffff
#define HWIO_GSI_MOQA_CFG_ATTR                                                                                  0x3
#define HWIO_GSI_MOQA_CFG_IN          \
        in_dword_masked(HWIO_GSI_MOQA_CFG_ADDR, HWIO_GSI_MOQA_CFG_RMSK)
#define HWIO_GSI_MOQA_CFG_INM(m)      \
        in_dword_masked(HWIO_GSI_MOQA_CFG_ADDR, m)
#define HWIO_GSI_MOQA_CFG_OUT(v)      \
        out_dword(HWIO_GSI_MOQA_CFG_ADDR,v)
#define HWIO_GSI_MOQA_CFG_OUTM(m,v) \
        out_dword_masked_ns(HWIO_GSI_MOQA_CFG_ADDR,m,v,HWIO_GSI_MOQA_CFG_IN)
#define HWIO_GSI_MOQA_CFG_CLIENT_OOWR_BMSK                                                                 0xff0000
#define HWIO_GSI_MOQA_CFG_CLIENT_OOWR_SHFT                                                                     0x10
#define HWIO_GSI_MOQA_CFG_CLIENT_OORD_BMSK                                                                   0xff00
#define HWIO_GSI_MOQA_CFG_CLIENT_OORD_SHFT                                                                      0x8
#define HWIO_GSI_MOQA_CFG_CLIENT_REQ_PRIO_BMSK                                                                 0xff
#define HWIO_GSI_MOQA_CFG_CLIENT_REQ_PRIO_SHFT                                                                  0x0

#define HWIO_GSI_REE_CFG_ADDR                                                                            (GSI_REG_BASE      + 0x00000038)
#define HWIO_GSI_REE_CFG_PHYS                                                                            (GSI_REG_BASE_PHYS + 0x00000038)
#define HWIO_GSI_REE_CFG_OFFS                                                                            (GSI_REG_BASE_OFFS + 0x00000038)
#define HWIO_GSI_REE_CFG_RMSK                                                                                0xff03
#define HWIO_GSI_REE_CFG_ATTR                                                                                   0x3
#define HWIO_GSI_REE_CFG_IN          \
        in_dword_masked(HWIO_GSI_REE_CFG_ADDR, HWIO_GSI_REE_CFG_RMSK)
#define HWIO_GSI_REE_CFG_INM(m)      \
        in_dword_masked(HWIO_GSI_REE_CFG_ADDR, m)
#define HWIO_GSI_REE_CFG_OUT(v)      \
        out_dword(HWIO_GSI_REE_CFG_ADDR,v)
#define HWIO_GSI_REE_CFG_OUTM(m,v) \
        out_dword_masked_ns(HWIO_GSI_REE_CFG_ADDR,m,v,HWIO_GSI_REE_CFG_IN)
#define HWIO_GSI_REE_CFG_MAX_BURST_SIZE_BMSK                                                                 0xff00
#define HWIO_GSI_REE_CFG_MAX_BURST_SIZE_SHFT                                                                    0x8
#define HWIO_GSI_REE_CFG_CHANNEL_EMPTY_INT_ENABLE_BMSK                                                          0x2
#define HWIO_GSI_REE_CFG_CHANNEL_EMPTY_INT_ENABLE_SHFT                                                          0x1
#define HWIO_GSI_REE_CFG_MOVE_TO_ESC_CLR_MODE_TRSH_BMSK                                                         0x1
#define HWIO_GSI_REE_CFG_MOVE_TO_ESC_CLR_MODE_TRSH_SHFT                                                         0x0

#define HWIO_GSI_PERIPH_PENDING_k_ADDR(k)                                                                (GSI_REG_BASE      + 0x00000060 + 0x4 * (k))
#define HWIO_GSI_PERIPH_PENDING_k_PHYS(k)                                                                (GSI_REG_BASE_PHYS + 0x00000060 + 0x4 * (k))
#define HWIO_GSI_PERIPH_PENDING_k_OFFS(k)                                                                (GSI_REG_BASE_OFFS + 0x00000060 + 0x4 * (k))
#define HWIO_GSI_PERIPH_PENDING_k_RMSK                                                                   0xffffffff
#define HWIO_GSI_PERIPH_PENDING_k_MAXk                                                                            1
#define HWIO_GSI_PERIPH_PENDING_k_ATTR                                                                          0x1
#define HWIO_GSI_PERIPH_PENDING_k_INI(k)        \
        in_dword_masked(HWIO_GSI_PERIPH_PENDING_k_ADDR(k), HWIO_GSI_PERIPH_PENDING_k_RMSK)
#define HWIO_GSI_PERIPH_PENDING_k_INMI(k,mask)    \
        in_dword_masked(HWIO_GSI_PERIPH_PENDING_k_ADDR(k), mask)
#define HWIO_GSI_PERIPH_PENDING_k_CHID_BIT_MAP_BMSK                                                      0xffffffff
#define HWIO_GSI_PERIPH_PENDING_k_CHID_BIT_MAP_SHFT                                                             0x0

#define HWIO_GSI_MSI_CACHEATTR_ADDR                                                                      (GSI_REG_BASE      + 0x00000080)
#define HWIO_GSI_MSI_CACHEATTR_PHYS                                                                      (GSI_REG_BASE_PHYS + 0x00000080)
#define HWIO_GSI_MSI_CACHEATTR_OFFS                                                                      (GSI_REG_BASE_OFFS + 0x00000080)
#define HWIO_GSI_MSI_CACHEATTR_RMSK                                                                            0x3f
#define HWIO_GSI_MSI_CACHEATTR_ATTR                                                                             0x3
#define HWIO_GSI_MSI_CACHEATTR_IN          \
        in_dword_masked(HWIO_GSI_MSI_CACHEATTR_ADDR, HWIO_GSI_MSI_CACHEATTR_RMSK)
#define HWIO_GSI_MSI_CACHEATTR_INM(m)      \
        in_dword_masked(HWIO_GSI_MSI_CACHEATTR_ADDR, m)
#define HWIO_GSI_MSI_CACHEATTR_OUT(v)      \
        out_dword(HWIO_GSI_MSI_CACHEATTR_ADDR,v)
#define HWIO_GSI_MSI_CACHEATTR_OUTM(m,v) \
        out_dword_masked_ns(HWIO_GSI_MSI_CACHEATTR_ADDR,m,v,HWIO_GSI_MSI_CACHEATTR_IN)
#define HWIO_GSI_MSI_CACHEATTR_AREQPRIORITY_BMSK                                                               0x30
#define HWIO_GSI_MSI_CACHEATTR_AREQPRIORITY_SHFT                                                                0x4
#define HWIO_GSI_MSI_CACHEATTR_ATRANSIENT_BMSK                                                                  0x8
#define HWIO_GSI_MSI_CACHEATTR_ATRANSIENT_SHFT                                                                  0x3
#define HWIO_GSI_MSI_CACHEATTR_ANOALLOCATE_BMSK                                                                 0x4
#define HWIO_GSI_MSI_CACHEATTR_ANOALLOCATE_SHFT                                                                 0x2
#define HWIO_GSI_MSI_CACHEATTR_AINNERSHARED_BMSK                                                                0x2
#define HWIO_GSI_MSI_CACHEATTR_AINNERSHARED_SHFT                                                                0x1
#define HWIO_GSI_MSI_CACHEATTR_ASHARED_BMSK                                                                     0x1
#define HWIO_GSI_MSI_CACHEATTR_ASHARED_SHFT                                                                     0x0

#define HWIO_GSI_EVENT_CACHEATTR_ADDR                                                                    (GSI_REG_BASE      + 0x00000084)
#define HWIO_GSI_EVENT_CACHEATTR_PHYS                                                                    (GSI_REG_BASE_PHYS + 0x00000084)
#define HWIO_GSI_EVENT_CACHEATTR_OFFS                                                                    (GSI_REG_BASE_OFFS + 0x00000084)
#define HWIO_GSI_EVENT_CACHEATTR_RMSK                                                                          0x3f
#define HWIO_GSI_EVENT_CACHEATTR_ATTR                                                                           0x3
#define HWIO_GSI_EVENT_CACHEATTR_IN          \
        in_dword_masked(HWIO_GSI_EVENT_CACHEATTR_ADDR, HWIO_GSI_EVENT_CACHEATTR_RMSK)
#define HWIO_GSI_EVENT_CACHEATTR_INM(m)      \
        in_dword_masked(HWIO_GSI_EVENT_CACHEATTR_ADDR, m)
#define HWIO_GSI_EVENT_CACHEATTR_OUT(v)      \
        out_dword(HWIO_GSI_EVENT_CACHEATTR_ADDR,v)
#define HWIO_GSI_EVENT_CACHEATTR_OUTM(m,v) \
        out_dword_masked_ns(HWIO_GSI_EVENT_CACHEATTR_ADDR,m,v,HWIO_GSI_EVENT_CACHEATTR_IN)
#define HWIO_GSI_EVENT_CACHEATTR_AREQPRIORITY_BMSK                                                             0x30
#define HWIO_GSI_EVENT_CACHEATTR_AREQPRIORITY_SHFT                                                              0x4
#define HWIO_GSI_EVENT_CACHEATTR_ATRANSIENT_BMSK                                                                0x8
#define HWIO_GSI_EVENT_CACHEATTR_ATRANSIENT_SHFT                                                                0x3
#define HWIO_GSI_EVENT_CACHEATTR_ANOALLOCATE_BMSK                                                               0x4
#define HWIO_GSI_EVENT_CACHEATTR_ANOALLOCATE_SHFT                                                               0x2
#define HWIO_GSI_EVENT_CACHEATTR_AINNERSHARED_BMSK                                                              0x2
#define HWIO_GSI_EVENT_CACHEATTR_AINNERSHARED_SHFT                                                              0x1
#define HWIO_GSI_EVENT_CACHEATTR_ASHARED_BMSK                                                                   0x1
#define HWIO_GSI_EVENT_CACHEATTR_ASHARED_SHFT                                                                   0x0

#define HWIO_GSI_DATA_CACHEATTR_ADDR                                                                     (GSI_REG_BASE      + 0x00000088)
#define HWIO_GSI_DATA_CACHEATTR_PHYS                                                                     (GSI_REG_BASE_PHYS + 0x00000088)
#define HWIO_GSI_DATA_CACHEATTR_OFFS                                                                     (GSI_REG_BASE_OFFS + 0x00000088)
#define HWIO_GSI_DATA_CACHEATTR_RMSK                                                                           0x3f
#define HWIO_GSI_DATA_CACHEATTR_ATTR                                                                            0x3
#define HWIO_GSI_DATA_CACHEATTR_IN          \
        in_dword_masked(HWIO_GSI_DATA_CACHEATTR_ADDR, HWIO_GSI_DATA_CACHEATTR_RMSK)
#define HWIO_GSI_DATA_CACHEATTR_INM(m)      \
        in_dword_masked(HWIO_GSI_DATA_CACHEATTR_ADDR, m)
#define HWIO_GSI_DATA_CACHEATTR_OUT(v)      \
        out_dword(HWIO_GSI_DATA_CACHEATTR_ADDR,v)
#define HWIO_GSI_DATA_CACHEATTR_OUTM(m,v) \
        out_dword_masked_ns(HWIO_GSI_DATA_CACHEATTR_ADDR,m,v,HWIO_GSI_DATA_CACHEATTR_IN)
#define HWIO_GSI_DATA_CACHEATTR_AREQPRIORITY_BMSK                                                              0x30
#define HWIO_GSI_DATA_CACHEATTR_AREQPRIORITY_SHFT                                                               0x4
#define HWIO_GSI_DATA_CACHEATTR_ATRANSIENT_BMSK                                                                 0x8
#define HWIO_GSI_DATA_CACHEATTR_ATRANSIENT_SHFT                                                                 0x3
#define HWIO_GSI_DATA_CACHEATTR_ANOALLOCATE_BMSK                                                                0x4
#define HWIO_GSI_DATA_CACHEATTR_ANOALLOCATE_SHFT                                                                0x2
#define HWIO_GSI_DATA_CACHEATTR_AINNERSHARED_BMSK                                                               0x2
#define HWIO_GSI_DATA_CACHEATTR_AINNERSHARED_SHFT                                                               0x1
#define HWIO_GSI_DATA_CACHEATTR_ASHARED_BMSK                                                                    0x1
#define HWIO_GSI_DATA_CACHEATTR_ASHARED_SHFT                                                                    0x0

#define HWIO_GSI_TRE_CACHEATTR_ADDR                                                                      (GSI_REG_BASE      + 0x00000090)
#define HWIO_GSI_TRE_CACHEATTR_PHYS                                                                      (GSI_REG_BASE_PHYS + 0x00000090)
#define HWIO_GSI_TRE_CACHEATTR_OFFS                                                                      (GSI_REG_BASE_OFFS + 0x00000090)
#define HWIO_GSI_TRE_CACHEATTR_RMSK                                                                            0x3f
#define HWIO_GSI_TRE_CACHEATTR_ATTR                                                                             0x3
#define HWIO_GSI_TRE_CACHEATTR_IN          \
        in_dword_masked(HWIO_GSI_TRE_CACHEATTR_ADDR, HWIO_GSI_TRE_CACHEATTR_RMSK)
#define HWIO_GSI_TRE_CACHEATTR_INM(m)      \
        in_dword_masked(HWIO_GSI_TRE_CACHEATTR_ADDR, m)
#define HWIO_GSI_TRE_CACHEATTR_OUT(v)      \
        out_dword(HWIO_GSI_TRE_CACHEATTR_ADDR,v)
#define HWIO_GSI_TRE_CACHEATTR_OUTM(m,v) \
        out_dword_masked_ns(HWIO_GSI_TRE_CACHEATTR_ADDR,m,v,HWIO_GSI_TRE_CACHEATTR_IN)
#define HWIO_GSI_TRE_CACHEATTR_AREQPRIORITY_BMSK                                                               0x30
#define HWIO_GSI_TRE_CACHEATTR_AREQPRIORITY_SHFT                                                                0x4
#define HWIO_GSI_TRE_CACHEATTR_ATRANSIENT_BMSK                                                                  0x8
#define HWIO_GSI_TRE_CACHEATTR_ATRANSIENT_SHFT                                                                  0x3
#define HWIO_GSI_TRE_CACHEATTR_ANOALLOCATE_BMSK                                                                 0x4
#define HWIO_GSI_TRE_CACHEATTR_ANOALLOCATE_SHFT                                                                 0x2
#define HWIO_GSI_TRE_CACHEATTR_AINNERSHARED_BMSK                                                                0x2
#define HWIO_GSI_TRE_CACHEATTR_AINNERSHARED_SHFT                                                                0x1
#define HWIO_GSI_TRE_CACHEATTR_ASHARED_BMSK                                                                     0x1
#define HWIO_GSI_TRE_CACHEATTR_ASHARED_SHFT                                                                     0x0

#define HWIO_IC_INT_WEIGHT_REE_ADDR                                                                      (GSI_REG_BASE      + 0x00000100)
#define HWIO_IC_INT_WEIGHT_REE_PHYS                                                                      (GSI_REG_BASE_PHYS + 0x00000100)
#define HWIO_IC_INT_WEIGHT_REE_OFFS                                                                      (GSI_REG_BASE_OFFS + 0x00000100)
#define HWIO_IC_INT_WEIGHT_REE_RMSK                                                                           0xfff
#define HWIO_IC_INT_WEIGHT_REE_ATTR                                                                             0x3
#define HWIO_IC_INT_WEIGHT_REE_IN          \
        in_dword_masked(HWIO_IC_INT_WEIGHT_REE_ADDR, HWIO_IC_INT_WEIGHT_REE_RMSK)
#define HWIO_IC_INT_WEIGHT_REE_INM(m)      \
        in_dword_masked(HWIO_IC_INT_WEIGHT_REE_ADDR, m)
#define HWIO_IC_INT_WEIGHT_REE_OUT(v)      \
        out_dword(HWIO_IC_INT_WEIGHT_REE_ADDR,v)
#define HWIO_IC_INT_WEIGHT_REE_OUTM(m,v) \
        out_dword_masked_ns(HWIO_IC_INT_WEIGHT_REE_ADDR,m,v,HWIO_IC_INT_WEIGHT_REE_IN)
#define HWIO_IC_INT_WEIGHT_REE_CH_EMPTY_INT_WEIGHT_BMSK                                                       0xf00
#define HWIO_IC_INT_WEIGHT_REE_CH_EMPTY_INT_WEIGHT_SHFT                                                         0x8
#define HWIO_IC_INT_WEIGHT_REE_NEW_RE_INT_WEIGHT_BMSK                                                          0xf0
#define HWIO_IC_INT_WEIGHT_REE_NEW_RE_INT_WEIGHT_SHFT                                                           0x4
#define HWIO_IC_INT_WEIGHT_REE_STOP_CH_COMP_INT_WEIGHT_BMSK                                                     0xf
#define HWIO_IC_INT_WEIGHT_REE_STOP_CH_COMP_INT_WEIGHT_SHFT                                                     0x0

#define HWIO_IC_INT_WEIGHT_EVT_ENG_ADDR                                                                  (GSI_REG_BASE      + 0x00000104)
#define HWIO_IC_INT_WEIGHT_EVT_ENG_PHYS                                                                  (GSI_REG_BASE_PHYS + 0x00000104)
#define HWIO_IC_INT_WEIGHT_EVT_ENG_OFFS                                                                  (GSI_REG_BASE_OFFS + 0x00000104)
#define HWIO_IC_INT_WEIGHT_EVT_ENG_RMSK                                                                         0xf
#define HWIO_IC_INT_WEIGHT_EVT_ENG_ATTR                                                                         0x3
#define HWIO_IC_INT_WEIGHT_EVT_ENG_IN          \
        in_dword_masked(HWIO_IC_INT_WEIGHT_EVT_ENG_ADDR, HWIO_IC_INT_WEIGHT_EVT_ENG_RMSK)
#define HWIO_IC_INT_WEIGHT_EVT_ENG_INM(m)      \
        in_dword_masked(HWIO_IC_INT_WEIGHT_EVT_ENG_ADDR, m)
#define HWIO_IC_INT_WEIGHT_EVT_ENG_OUT(v)      \
        out_dword(HWIO_IC_INT_WEIGHT_EVT_ENG_ADDR,v)
#define HWIO_IC_INT_WEIGHT_EVT_ENG_OUTM(m,v) \
        out_dword_masked_ns(HWIO_IC_INT_WEIGHT_EVT_ENG_ADDR,m,v,HWIO_IC_INT_WEIGHT_EVT_ENG_IN)
#define HWIO_IC_INT_WEIGHT_EVT_ENG_EVNT_ENG_INT_WEIGHT_BMSK                                                     0xf
#define HWIO_IC_INT_WEIGHT_EVT_ENG_EVNT_ENG_INT_WEIGHT_SHFT                                                     0x0

#define HWIO_IC_INT_WEIGHT_INT_ENG_ADDR                                                                  (GSI_REG_BASE      + 0x00000108)
#define HWIO_IC_INT_WEIGHT_INT_ENG_PHYS                                                                  (GSI_REG_BASE_PHYS + 0x00000108)
#define HWIO_IC_INT_WEIGHT_INT_ENG_OFFS                                                                  (GSI_REG_BASE_OFFS + 0x00000108)
#define HWIO_IC_INT_WEIGHT_INT_ENG_RMSK                                                                         0xf
#define HWIO_IC_INT_WEIGHT_INT_ENG_ATTR                                                                         0x3
#define HWIO_IC_INT_WEIGHT_INT_ENG_IN          \
        in_dword_masked(HWIO_IC_INT_WEIGHT_INT_ENG_ADDR, HWIO_IC_INT_WEIGHT_INT_ENG_RMSK)
#define HWIO_IC_INT_WEIGHT_INT_ENG_INM(m)      \
        in_dword_masked(HWIO_IC_INT_WEIGHT_INT_ENG_ADDR, m)
#define HWIO_IC_INT_WEIGHT_INT_ENG_OUT(v)      \
        out_dword(HWIO_IC_INT_WEIGHT_INT_ENG_ADDR,v)
#define HWIO_IC_INT_WEIGHT_INT_ENG_OUTM(m,v) \
        out_dword_masked_ns(HWIO_IC_INT_WEIGHT_INT_ENG_ADDR,m,v,HWIO_IC_INT_WEIGHT_INT_ENG_IN)
#define HWIO_IC_INT_WEIGHT_INT_ENG_INT_ENG_INT_WEIGHT_BMSK                                                      0xf
#define HWIO_IC_INT_WEIGHT_INT_ENG_INT_ENG_INT_WEIGHT_SHFT                                                      0x0

#define HWIO_IC_INT_WEIGHT_CSR_ADDR                                                                      (GSI_REG_BASE      + 0x0000010c)
#define HWIO_IC_INT_WEIGHT_CSR_PHYS                                                                      (GSI_REG_BASE_PHYS + 0x0000010c)
#define HWIO_IC_INT_WEIGHT_CSR_OFFS                                                                      (GSI_REG_BASE_OFFS + 0x0000010c)
#define HWIO_IC_INT_WEIGHT_CSR_RMSK                                                                            0xff
#define HWIO_IC_INT_WEIGHT_CSR_ATTR                                                                             0x3
#define HWIO_IC_INT_WEIGHT_CSR_IN          \
        in_dword_masked(HWIO_IC_INT_WEIGHT_CSR_ADDR, HWIO_IC_INT_WEIGHT_CSR_RMSK)
#define HWIO_IC_INT_WEIGHT_CSR_INM(m)      \
        in_dword_masked(HWIO_IC_INT_WEIGHT_CSR_ADDR, m)
#define HWIO_IC_INT_WEIGHT_CSR_OUT(v)      \
        out_dword(HWIO_IC_INT_WEIGHT_CSR_ADDR,v)
#define HWIO_IC_INT_WEIGHT_CSR_OUTM(m,v) \
        out_dword_masked_ns(HWIO_IC_INT_WEIGHT_CSR_ADDR,m,v,HWIO_IC_INT_WEIGHT_CSR_IN)
#define HWIO_IC_INT_WEIGHT_CSR_EE_GENERIC_INT_WEIGHT_BMSK                                                      0xf0
#define HWIO_IC_INT_WEIGHT_CSR_EE_GENERIC_INT_WEIGHT_SHFT                                                       0x4
#define HWIO_IC_INT_WEIGHT_CSR_CH_CMD_INT_WEIGHT_BMSK                                                           0xf
#define HWIO_IC_INT_WEIGHT_CSR_CH_CMD_INT_WEIGHT_SHFT                                                           0x0

#define HWIO_IC_INT_WEIGHT_TLV_ENG_ADDR                                                                  (GSI_REG_BASE      + 0x00000110)
#define HWIO_IC_INT_WEIGHT_TLV_ENG_PHYS                                                                  (GSI_REG_BASE_PHYS + 0x00000110)
#define HWIO_IC_INT_WEIGHT_TLV_ENG_OFFS                                                                  (GSI_REG_BASE_OFFS + 0x00000110)
#define HWIO_IC_INT_WEIGHT_TLV_ENG_RMSK                                                                      0xffff
#define HWIO_IC_INT_WEIGHT_TLV_ENG_ATTR                                                                         0x3
#define HWIO_IC_INT_WEIGHT_TLV_ENG_IN          \
        in_dword_masked(HWIO_IC_INT_WEIGHT_TLV_ENG_ADDR, HWIO_IC_INT_WEIGHT_TLV_ENG_RMSK)
#define HWIO_IC_INT_WEIGHT_TLV_ENG_INM(m)      \
        in_dword_masked(HWIO_IC_INT_WEIGHT_TLV_ENG_ADDR, m)
#define HWIO_IC_INT_WEIGHT_TLV_ENG_OUT(v)      \
        out_dword(HWIO_IC_INT_WEIGHT_TLV_ENG_ADDR,v)
#define HWIO_IC_INT_WEIGHT_TLV_ENG_OUTM(m,v) \
        out_dword_masked_ns(HWIO_IC_INT_WEIGHT_TLV_ENG_ADDR,m,v,HWIO_IC_INT_WEIGHT_TLV_ENG_IN)
#define HWIO_IC_INT_WEIGHT_TLV_ENG_CH_NOT_FULL_INT_WEIGHT_BMSK                                               0xf000
#define HWIO_IC_INT_WEIGHT_TLV_ENG_CH_NOT_FULL_INT_WEIGHT_SHFT                                                  0xc
#define HWIO_IC_INT_WEIGHT_TLV_ENG_TLV_2_INT_WEIGHT_BMSK                                                      0xf00
#define HWIO_IC_INT_WEIGHT_TLV_ENG_TLV_2_INT_WEIGHT_SHFT                                                        0x8
#define HWIO_IC_INT_WEIGHT_TLV_ENG_TLV_1_INT_WEIGHT_BMSK                                                       0xf0
#define HWIO_IC_INT_WEIGHT_TLV_ENG_TLV_1_INT_WEIGHT_SHFT                                                        0x4
#define HWIO_IC_INT_WEIGHT_TLV_ENG_TLV_0_INT_WEIGHT_BMSK                                                        0xf
#define HWIO_IC_INT_WEIGHT_TLV_ENG_TLV_0_INT_WEIGHT_SHFT                                                        0x0

#define HWIO_IC_INT_WEIGHT_TIMER_ENG_ADDR                                                                (GSI_REG_BASE      + 0x00000114)
#define HWIO_IC_INT_WEIGHT_TIMER_ENG_PHYS                                                                (GSI_REG_BASE_PHYS + 0x00000114)
#define HWIO_IC_INT_WEIGHT_TIMER_ENG_OFFS                                                                (GSI_REG_BASE_OFFS + 0x00000114)
#define HWIO_IC_INT_WEIGHT_TIMER_ENG_RMSK                                                                       0xf
#define HWIO_IC_INT_WEIGHT_TIMER_ENG_ATTR                                                                       0x3
#define HWIO_IC_INT_WEIGHT_TIMER_ENG_IN          \
        in_dword_masked(HWIO_IC_INT_WEIGHT_TIMER_ENG_ADDR, HWIO_IC_INT_WEIGHT_TIMER_ENG_RMSK)
#define HWIO_IC_INT_WEIGHT_TIMER_ENG_INM(m)      \
        in_dword_masked(HWIO_IC_INT_WEIGHT_TIMER_ENG_ADDR, m)
#define HWIO_IC_INT_WEIGHT_TIMER_ENG_OUT(v)      \
        out_dword(HWIO_IC_INT_WEIGHT_TIMER_ENG_ADDR,v)
#define HWIO_IC_INT_WEIGHT_TIMER_ENG_OUTM(m,v) \
        out_dword_masked_ns(HWIO_IC_INT_WEIGHT_TIMER_ENG_ADDR,m,v,HWIO_IC_INT_WEIGHT_TIMER_ENG_IN)
#define HWIO_IC_INT_WEIGHT_TIMER_ENG_TIMER_INT_WEIGHT_BMSK                                                      0xf
#define HWIO_IC_INT_WEIGHT_TIMER_ENG_TIMER_INT_WEIGHT_SHFT                                                      0x0

#define HWIO_IC_INT_WEIGHT_DB_ENG_ADDR                                                                   (GSI_REG_BASE      + 0x00000118)
#define HWIO_IC_INT_WEIGHT_DB_ENG_PHYS                                                                   (GSI_REG_BASE_PHYS + 0x00000118)
#define HWIO_IC_INT_WEIGHT_DB_ENG_OFFS                                                                   (GSI_REG_BASE_OFFS + 0x00000118)
#define HWIO_IC_INT_WEIGHT_DB_ENG_RMSK                                                                          0xf
#define HWIO_IC_INT_WEIGHT_DB_ENG_ATTR                                                                          0x3
#define HWIO_IC_INT_WEIGHT_DB_ENG_IN          \
        in_dword_masked(HWIO_IC_INT_WEIGHT_DB_ENG_ADDR, HWIO_IC_INT_WEIGHT_DB_ENG_RMSK)
#define HWIO_IC_INT_WEIGHT_DB_ENG_INM(m)      \
        in_dword_masked(HWIO_IC_INT_WEIGHT_DB_ENG_ADDR, m)
#define HWIO_IC_INT_WEIGHT_DB_ENG_OUT(v)      \
        out_dword(HWIO_IC_INT_WEIGHT_DB_ENG_ADDR,v)
#define HWIO_IC_INT_WEIGHT_DB_ENG_OUTM(m,v) \
        out_dword_masked_ns(HWIO_IC_INT_WEIGHT_DB_ENG_ADDR,m,v,HWIO_IC_INT_WEIGHT_DB_ENG_IN)
#define HWIO_IC_INT_WEIGHT_DB_ENG_NEW_DB_INT_WEIGHT_BMSK                                                        0xf
#define HWIO_IC_INT_WEIGHT_DB_ENG_NEW_DB_INT_WEIGHT_SHFT                                                        0x0

#define HWIO_IC_INT_WEIGHT_RD_WR_ENG_ADDR                                                                (GSI_REG_BASE      + 0x0000011c)
#define HWIO_IC_INT_WEIGHT_RD_WR_ENG_PHYS                                                                (GSI_REG_BASE_PHYS + 0x0000011c)
#define HWIO_IC_INT_WEIGHT_RD_WR_ENG_OFFS                                                                (GSI_REG_BASE_OFFS + 0x0000011c)
#define HWIO_IC_INT_WEIGHT_RD_WR_ENG_RMSK                                                                      0xff
#define HWIO_IC_INT_WEIGHT_RD_WR_ENG_ATTR                                                                       0x3
#define HWIO_IC_INT_WEIGHT_RD_WR_ENG_IN          \
        in_dword_masked(HWIO_IC_INT_WEIGHT_RD_WR_ENG_ADDR, HWIO_IC_INT_WEIGHT_RD_WR_ENG_RMSK)
#define HWIO_IC_INT_WEIGHT_RD_WR_ENG_INM(m)      \
        in_dword_masked(HWIO_IC_INT_WEIGHT_RD_WR_ENG_ADDR, m)
#define HWIO_IC_INT_WEIGHT_RD_WR_ENG_OUT(v)      \
        out_dword(HWIO_IC_INT_WEIGHT_RD_WR_ENG_ADDR,v)
#define HWIO_IC_INT_WEIGHT_RD_WR_ENG_OUTM(m,v) \
        out_dword_masked_ns(HWIO_IC_INT_WEIGHT_RD_WR_ENG_ADDR,m,v,HWIO_IC_INT_WEIGHT_RD_WR_ENG_IN)
#define HWIO_IC_INT_WEIGHT_RD_WR_ENG_WRITE_INT_WEIGHT_BMSK                                                     0xf0
#define HWIO_IC_INT_WEIGHT_RD_WR_ENG_WRITE_INT_WEIGHT_SHFT                                                      0x4
#define HWIO_IC_INT_WEIGHT_RD_WR_ENG_READ_INT_WEIGHT_BMSK                                                       0xf
#define HWIO_IC_INT_WEIGHT_RD_WR_ENG_READ_INT_WEIGHT_SHFT                                                       0x0

#define HWIO_IC_INT_WEIGHT_UCONTROLLER_ENG_ADDR                                                          (GSI_REG_BASE      + 0x00000120)
#define HWIO_IC_INT_WEIGHT_UCONTROLLER_ENG_PHYS                                                          (GSI_REG_BASE_PHYS + 0x00000120)
#define HWIO_IC_INT_WEIGHT_UCONTROLLER_ENG_OFFS                                                          (GSI_REG_BASE_OFFS + 0x00000120)
#define HWIO_IC_INT_WEIGHT_UCONTROLLER_ENG_RMSK                                                                 0xf
#define HWIO_IC_INT_WEIGHT_UCONTROLLER_ENG_ATTR                                                                 0x3
#define HWIO_IC_INT_WEIGHT_UCONTROLLER_ENG_IN          \
        in_dword_masked(HWIO_IC_INT_WEIGHT_UCONTROLLER_ENG_ADDR, HWIO_IC_INT_WEIGHT_UCONTROLLER_ENG_RMSK)
#define HWIO_IC_INT_WEIGHT_UCONTROLLER_ENG_INM(m)      \
        in_dword_masked(HWIO_IC_INT_WEIGHT_UCONTROLLER_ENG_ADDR, m)
#define HWIO_IC_INT_WEIGHT_UCONTROLLER_ENG_OUT(v)      \
        out_dword(HWIO_IC_INT_WEIGHT_UCONTROLLER_ENG_ADDR,v)
#define HWIO_IC_INT_WEIGHT_UCONTROLLER_ENG_OUTM(m,v) \
        out_dword_masked_ns(HWIO_IC_INT_WEIGHT_UCONTROLLER_ENG_ADDR,m,v,HWIO_IC_INT_WEIGHT_UCONTROLLER_ENG_IN)
#define HWIO_IC_INT_WEIGHT_UCONTROLLER_ENG_UCONTROLLER_GP_INT_WEIGHT_BMSK                                       0xf
#define HWIO_IC_INT_WEIGHT_UCONTROLLER_ENG_UCONTROLLER_GP_INT_WEIGHT_SHFT                                       0x0

#define HWIO_LOW_LATENCY_ARB_WEIGHT_ADDR                                                                 (GSI_REG_BASE      + 0x00000128)
#define HWIO_LOW_LATENCY_ARB_WEIGHT_PHYS                                                                 (GSI_REG_BASE_PHYS + 0x00000128)
#define HWIO_LOW_LATENCY_ARB_WEIGHT_OFFS                                                                 (GSI_REG_BASE_OFFS + 0x00000128)
#define HWIO_LOW_LATENCY_ARB_WEIGHT_RMSK                                                                    0x13f3f
#define HWIO_LOW_LATENCY_ARB_WEIGHT_ATTR                                                                        0x3
#define HWIO_LOW_LATENCY_ARB_WEIGHT_IN          \
        in_dword_masked(HWIO_LOW_LATENCY_ARB_WEIGHT_ADDR, HWIO_LOW_LATENCY_ARB_WEIGHT_RMSK)
#define HWIO_LOW_LATENCY_ARB_WEIGHT_INM(m)      \
        in_dword_masked(HWIO_LOW_LATENCY_ARB_WEIGHT_ADDR, m)
#define HWIO_LOW_LATENCY_ARB_WEIGHT_OUT(v)      \
        out_dword(HWIO_LOW_LATENCY_ARB_WEIGHT_ADDR,v)
#define HWIO_LOW_LATENCY_ARB_WEIGHT_OUTM(m,v) \
        out_dword_masked_ns(HWIO_LOW_LATENCY_ARB_WEIGHT_ADDR,m,v,HWIO_LOW_LATENCY_ARB_WEIGHT_IN)
#define HWIO_LOW_LATENCY_ARB_WEIGHT_LL_NON_LL_FIX_PRIORITY_BMSK                                             0x10000
#define HWIO_LOW_LATENCY_ARB_WEIGHT_LL_NON_LL_FIX_PRIORITY_SHFT                                                0x10
#define HWIO_LOW_LATENCY_ARB_WEIGHT_NON_LL_WEIGHT_BMSK                                                       0x3f00
#define HWIO_LOW_LATENCY_ARB_WEIGHT_NON_LL_WEIGHT_SHFT                                                          0x8
#define HWIO_LOW_LATENCY_ARB_WEIGHT_LL_WEIGHT_BMSK                                                             0x3f
#define HWIO_LOW_LATENCY_ARB_WEIGHT_LL_WEIGHT_SHFT                                                              0x0

#define HWIO_GSI_MANAGER_EE_QOS_n_ADDR(n)                                                                (GSI_REG_BASE      + 0x00000300 + 0x4 * (n))
#define HWIO_GSI_MANAGER_EE_QOS_n_PHYS(n)                                                                (GSI_REG_BASE_PHYS + 0x00000300 + 0x4 * (n))
#define HWIO_GSI_MANAGER_EE_QOS_n_OFFS(n)                                                                (GSI_REG_BASE_OFFS + 0x00000300 + 0x4 * (n))
#define HWIO_GSI_MANAGER_EE_QOS_n_RMSK                                                                     0xffff03
#define HWIO_GSI_MANAGER_EE_QOS_n_MAXn                                                                            2
#define HWIO_GSI_MANAGER_EE_QOS_n_ATTR                                                                          0x0
#define HWIO_GSI_MANAGER_EE_QOS_n_INI(n)        \
        in_dword_masked(HWIO_GSI_MANAGER_EE_QOS_n_ADDR(n), HWIO_GSI_MANAGER_EE_QOS_n_RMSK)
#define HWIO_GSI_MANAGER_EE_QOS_n_INMI(n,mask)    \
        in_dword_masked(HWIO_GSI_MANAGER_EE_QOS_n_ADDR(n), mask)
#define HWIO_GSI_MANAGER_EE_QOS_n_OUTI(n,val)    \
        out_dword(HWIO_GSI_MANAGER_EE_QOS_n_ADDR(n),val)
#define HWIO_GSI_MANAGER_EE_QOS_n_OUTMI(n,mask,val) \
        out_dword_masked_ns(HWIO_GSI_MANAGER_EE_QOS_n_ADDR(n),mask,val,HWIO_GSI_MANAGER_EE_QOS_n_INI(n))
#define HWIO_GSI_MANAGER_EE_QOS_n_MAX_EV_ALLOC_BMSK                                                        0xff0000
#define HWIO_GSI_MANAGER_EE_QOS_n_MAX_EV_ALLOC_SHFT                                                            0x10
#define HWIO_GSI_MANAGER_EE_QOS_n_MAX_CH_ALLOC_BMSK                                                          0xff00
#define HWIO_GSI_MANAGER_EE_QOS_n_MAX_CH_ALLOC_SHFT                                                             0x8
#define HWIO_GSI_MANAGER_EE_QOS_n_EE_PRIO_BMSK                                                                  0x3
#define HWIO_GSI_MANAGER_EE_QOS_n_EE_PRIO_SHFT                                                                  0x0

#define HWIO_GSI_SHRAM_PTR_CH_CNTXT_BASE_ADDR_ADDR                                                       (GSI_REG_BASE      + 0x00000200)
#define HWIO_GSI_SHRAM_PTR_CH_CNTXT_BASE_ADDR_PHYS                                                       (GSI_REG_BASE_PHYS + 0x00000200)
#define HWIO_GSI_SHRAM_PTR_CH_CNTXT_BASE_ADDR_OFFS                                                       (GSI_REG_BASE_OFFS + 0x00000200)
#define HWIO_GSI_SHRAM_PTR_CH_CNTXT_BASE_ADDR_RMSK                                                           0xffff
#define HWIO_GSI_SHRAM_PTR_CH_CNTXT_BASE_ADDR_ATTR                                                              0x3
#define HWIO_GSI_SHRAM_PTR_CH_CNTXT_BASE_ADDR_IN          \
        in_dword_masked(HWIO_GSI_SHRAM_PTR_CH_CNTXT_BASE_ADDR_ADDR, HWIO_GSI_SHRAM_PTR_CH_CNTXT_BASE_ADDR_RMSK)
#define HWIO_GSI_SHRAM_PTR_CH_CNTXT_BASE_ADDR_INM(m)      \
        in_dword_masked(HWIO_GSI_SHRAM_PTR_CH_CNTXT_BASE_ADDR_ADDR, m)
#define HWIO_GSI_SHRAM_PTR_CH_CNTXT_BASE_ADDR_OUT(v)      \
        out_dword(HWIO_GSI_SHRAM_PTR_CH_CNTXT_BASE_ADDR_ADDR,v)
#define HWIO_GSI_SHRAM_PTR_CH_CNTXT_BASE_ADDR_OUTM(m,v) \
        out_dword_masked_ns(HWIO_GSI_SHRAM_PTR_CH_CNTXT_BASE_ADDR_ADDR,m,v,HWIO_GSI_SHRAM_PTR_CH_CNTXT_BASE_ADDR_IN)
#define HWIO_GSI_SHRAM_PTR_CH_CNTXT_BASE_ADDR_SHRAM_PTR_BMSK                                                 0xffff
#define HWIO_GSI_SHRAM_PTR_CH_CNTXT_BASE_ADDR_SHRAM_PTR_SHFT                                                    0x0

#define HWIO_GSI_SHRAM_PTR_EV_CNTXT_BASE_ADDR_ADDR                                                       (GSI_REG_BASE      + 0x00000204)
#define HWIO_GSI_SHRAM_PTR_EV_CNTXT_BASE_ADDR_PHYS                                                       (GSI_REG_BASE_PHYS + 0x00000204)
#define HWIO_GSI_SHRAM_PTR_EV_CNTXT_BASE_ADDR_OFFS                                                       (GSI_REG_BASE_OFFS + 0x00000204)
#define HWIO_GSI_SHRAM_PTR_EV_CNTXT_BASE_ADDR_RMSK                                                           0xffff
#define HWIO_GSI_SHRAM_PTR_EV_CNTXT_BASE_ADDR_ATTR                                                              0x3
#define HWIO_GSI_SHRAM_PTR_EV_CNTXT_BASE_ADDR_IN          \
        in_dword_masked(HWIO_GSI_SHRAM_PTR_EV_CNTXT_BASE_ADDR_ADDR, HWIO_GSI_SHRAM_PTR_EV_CNTXT_BASE_ADDR_RMSK)
#define HWIO_GSI_SHRAM_PTR_EV_CNTXT_BASE_ADDR_INM(m)      \
        in_dword_masked(HWIO_GSI_SHRAM_PTR_EV_CNTXT_BASE_ADDR_ADDR, m)
#define HWIO_GSI_SHRAM_PTR_EV_CNTXT_BASE_ADDR_OUT(v)      \
        out_dword(HWIO_GSI_SHRAM_PTR_EV_CNTXT_BASE_ADDR_ADDR,v)
#define HWIO_GSI_SHRAM_PTR_EV_CNTXT_BASE_ADDR_OUTM(m,v) \
        out_dword_masked_ns(HWIO_GSI_SHRAM_PTR_EV_CNTXT_BASE_ADDR_ADDR,m,v,HWIO_GSI_SHRAM_PTR_EV_CNTXT_BASE_ADDR_IN)
#define HWIO_GSI_SHRAM_PTR_EV_CNTXT_BASE_ADDR_SHRAM_PTR_BMSK                                                 0xffff
#define HWIO_GSI_SHRAM_PTR_EV_CNTXT_BASE_ADDR_SHRAM_PTR_SHFT                                                    0x0

#define HWIO_GSI_SHRAM_PTR_RE_STORAGE_BASE_ADDR_ADDR                                                     (GSI_REG_BASE      + 0x00000208)
#define HWIO_GSI_SHRAM_PTR_RE_STORAGE_BASE_ADDR_PHYS                                                     (GSI_REG_BASE_PHYS + 0x00000208)
#define HWIO_GSI_SHRAM_PTR_RE_STORAGE_BASE_ADDR_OFFS                                                     (GSI_REG_BASE_OFFS + 0x00000208)
#define HWIO_GSI_SHRAM_PTR_RE_STORAGE_BASE_ADDR_RMSK                                                         0xffff
#define HWIO_GSI_SHRAM_PTR_RE_STORAGE_BASE_ADDR_ATTR                                                            0x3
#define HWIO_GSI_SHRAM_PTR_RE_STORAGE_BASE_ADDR_IN          \
        in_dword_masked(HWIO_GSI_SHRAM_PTR_RE_STORAGE_BASE_ADDR_ADDR, HWIO_GSI_SHRAM_PTR_RE_STORAGE_BASE_ADDR_RMSK)
#define HWIO_GSI_SHRAM_PTR_RE_STORAGE_BASE_ADDR_INM(m)      \
        in_dword_masked(HWIO_GSI_SHRAM_PTR_RE_STORAGE_BASE_ADDR_ADDR, m)
#define HWIO_GSI_SHRAM_PTR_RE_STORAGE_BASE_ADDR_OUT(v)      \
        out_dword(HWIO_GSI_SHRAM_PTR_RE_STORAGE_BASE_ADDR_ADDR,v)
#define HWIO_GSI_SHRAM_PTR_RE_STORAGE_BASE_ADDR_OUTM(m,v) \
        out_dword_masked_ns(HWIO_GSI_SHRAM_PTR_RE_STORAGE_BASE_ADDR_ADDR,m,v,HWIO_GSI_SHRAM_PTR_RE_STORAGE_BASE_ADDR_IN)
#define HWIO_GSI_SHRAM_PTR_RE_STORAGE_BASE_ADDR_SHRAM_PTR_BMSK                                               0xffff
#define HWIO_GSI_SHRAM_PTR_RE_STORAGE_BASE_ADDR_SHRAM_PTR_SHFT                                                  0x0

#define HWIO_GSI_SHRAM_PTR_RE_ESC_BUF_BASE_ADDR_ADDR                                                     (GSI_REG_BASE      + 0x0000020c)
#define HWIO_GSI_SHRAM_PTR_RE_ESC_BUF_BASE_ADDR_PHYS                                                     (GSI_REG_BASE_PHYS + 0x0000020c)
#define HWIO_GSI_SHRAM_PTR_RE_ESC_BUF_BASE_ADDR_OFFS                                                     (GSI_REG_BASE_OFFS + 0x0000020c)
#define HWIO_GSI_SHRAM_PTR_RE_ESC_BUF_BASE_ADDR_RMSK                                                         0xffff
#define HWIO_GSI_SHRAM_PTR_RE_ESC_BUF_BASE_ADDR_ATTR                                                            0x3
#define HWIO_GSI_SHRAM_PTR_RE_ESC_BUF_BASE_ADDR_IN          \
        in_dword_masked(HWIO_GSI_SHRAM_PTR_RE_ESC_BUF_BASE_ADDR_ADDR, HWIO_GSI_SHRAM_PTR_RE_ESC_BUF_BASE_ADDR_RMSK)
#define HWIO_GSI_SHRAM_PTR_RE_ESC_BUF_BASE_ADDR_INM(m)      \
        in_dword_masked(HWIO_GSI_SHRAM_PTR_RE_ESC_BUF_BASE_ADDR_ADDR, m)
#define HWIO_GSI_SHRAM_PTR_RE_ESC_BUF_BASE_ADDR_OUT(v)      \
        out_dword(HWIO_GSI_SHRAM_PTR_RE_ESC_BUF_BASE_ADDR_ADDR,v)
#define HWIO_GSI_SHRAM_PTR_RE_ESC_BUF_BASE_ADDR_OUTM(m,v) \
        out_dword_masked_ns(HWIO_GSI_SHRAM_PTR_RE_ESC_BUF_BASE_ADDR_ADDR,m,v,HWIO_GSI_SHRAM_PTR_RE_ESC_BUF_BASE_ADDR_IN)
#define HWIO_GSI_SHRAM_PTR_RE_ESC_BUF_BASE_ADDR_SHRAM_PTR_BMSK                                               0xffff
#define HWIO_GSI_SHRAM_PTR_RE_ESC_BUF_BASE_ADDR_SHRAM_PTR_SHFT                                                  0x0

#define HWIO_GSI_SHRAM_PTR_EE_SCRACH_BASE_ADDR_ADDR                                                      (GSI_REG_BASE      + 0x00000240)
#define HWIO_GSI_SHRAM_PTR_EE_SCRACH_BASE_ADDR_PHYS                                                      (GSI_REG_BASE_PHYS + 0x00000240)
#define HWIO_GSI_SHRAM_PTR_EE_SCRACH_BASE_ADDR_OFFS                                                      (GSI_REG_BASE_OFFS + 0x00000240)
#define HWIO_GSI_SHRAM_PTR_EE_SCRACH_BASE_ADDR_RMSK                                                          0xffff
#define HWIO_GSI_SHRAM_PTR_EE_SCRACH_BASE_ADDR_ATTR                                                             0x3
#define HWIO_GSI_SHRAM_PTR_EE_SCRACH_BASE_ADDR_IN          \
        in_dword_masked(HWIO_GSI_SHRAM_PTR_EE_SCRACH_BASE_ADDR_ADDR, HWIO_GSI_SHRAM_PTR_EE_SCRACH_BASE_ADDR_RMSK)
#define HWIO_GSI_SHRAM_PTR_EE_SCRACH_BASE_ADDR_INM(m)      \
        in_dword_masked(HWIO_GSI_SHRAM_PTR_EE_SCRACH_BASE_ADDR_ADDR, m)
#define HWIO_GSI_SHRAM_PTR_EE_SCRACH_BASE_ADDR_OUT(v)      \
        out_dword(HWIO_GSI_SHRAM_PTR_EE_SCRACH_BASE_ADDR_ADDR,v)
#define HWIO_GSI_SHRAM_PTR_EE_SCRACH_BASE_ADDR_OUTM(m,v) \
        out_dword_masked_ns(HWIO_GSI_SHRAM_PTR_EE_SCRACH_BASE_ADDR_ADDR,m,v,HWIO_GSI_SHRAM_PTR_EE_SCRACH_BASE_ADDR_IN)
#define HWIO_GSI_SHRAM_PTR_EE_SCRACH_BASE_ADDR_SHRAM_PTR_BMSK                                                0xffff
#define HWIO_GSI_SHRAM_PTR_EE_SCRACH_BASE_ADDR_SHRAM_PTR_SHFT                                                   0x0

#define HWIO_GSI_SHRAM_PTR_FUNC_STACK_BASE_ADDR_ADDR                                                     (GSI_REG_BASE      + 0x00000244)
#define HWIO_GSI_SHRAM_PTR_FUNC_STACK_BASE_ADDR_PHYS                                                     (GSI_REG_BASE_PHYS + 0x00000244)
#define HWIO_GSI_SHRAM_PTR_FUNC_STACK_BASE_ADDR_OFFS                                                     (GSI_REG_BASE_OFFS + 0x00000244)
#define HWIO_GSI_SHRAM_PTR_FUNC_STACK_BASE_ADDR_RMSK                                                         0xffff
#define HWIO_GSI_SHRAM_PTR_FUNC_STACK_BASE_ADDR_ATTR                                                            0x3
#define HWIO_GSI_SHRAM_PTR_FUNC_STACK_BASE_ADDR_IN          \
        in_dword_masked(HWIO_GSI_SHRAM_PTR_FUNC_STACK_BASE_ADDR_ADDR, HWIO_GSI_SHRAM_PTR_FUNC_STACK_BASE_ADDR_RMSK)
#define HWIO_GSI_SHRAM_PTR_FUNC_STACK_BASE_ADDR_INM(m)      \
        in_dword_masked(HWIO_GSI_SHRAM_PTR_FUNC_STACK_BASE_ADDR_ADDR, m)
#define HWIO_GSI_SHRAM_PTR_FUNC_STACK_BASE_ADDR_OUT(v)      \
        out_dword(HWIO_GSI_SHRAM_PTR_FUNC_STACK_BASE_ADDR_ADDR,v)
#define HWIO_GSI_SHRAM_PTR_FUNC_STACK_BASE_ADDR_OUTM(m,v) \
        out_dword_masked_ns(HWIO_GSI_SHRAM_PTR_FUNC_STACK_BASE_ADDR_ADDR,m,v,HWIO_GSI_SHRAM_PTR_FUNC_STACK_BASE_ADDR_IN)
#define HWIO_GSI_SHRAM_PTR_FUNC_STACK_BASE_ADDR_SHRAM_PTR_BMSK                                               0xffff
#define HWIO_GSI_SHRAM_PTR_FUNC_STACK_BASE_ADDR_SHRAM_PTR_SHFT                                                  0x0

#define HWIO_GSI_SHRAM_PTR_MCS_SCRATCH_BASE_ADDR_ADDR                                                    (GSI_REG_BASE      + 0x00000210)
#define HWIO_GSI_SHRAM_PTR_MCS_SCRATCH_BASE_ADDR_PHYS                                                    (GSI_REG_BASE_PHYS + 0x00000210)
#define HWIO_GSI_SHRAM_PTR_MCS_SCRATCH_BASE_ADDR_OFFS                                                    (GSI_REG_BASE_OFFS + 0x00000210)
#define HWIO_GSI_SHRAM_PTR_MCS_SCRATCH_BASE_ADDR_RMSK                                                        0xffff
#define HWIO_GSI_SHRAM_PTR_MCS_SCRATCH_BASE_ADDR_ATTR                                                           0x3
#define HWIO_GSI_SHRAM_PTR_MCS_SCRATCH_BASE_ADDR_IN          \
        in_dword_masked(HWIO_GSI_SHRAM_PTR_MCS_SCRATCH_BASE_ADDR_ADDR, HWIO_GSI_SHRAM_PTR_MCS_SCRATCH_BASE_ADDR_RMSK)
#define HWIO_GSI_SHRAM_PTR_MCS_SCRATCH_BASE_ADDR_INM(m)      \
        in_dword_masked(HWIO_GSI_SHRAM_PTR_MCS_SCRATCH_BASE_ADDR_ADDR, m)
#define HWIO_GSI_SHRAM_PTR_MCS_SCRATCH_BASE_ADDR_OUT(v)      \
        out_dword(HWIO_GSI_SHRAM_PTR_MCS_SCRATCH_BASE_ADDR_ADDR,v)
#define HWIO_GSI_SHRAM_PTR_MCS_SCRATCH_BASE_ADDR_OUTM(m,v) \
        out_dword_masked_ns(HWIO_GSI_SHRAM_PTR_MCS_SCRATCH_BASE_ADDR_ADDR,m,v,HWIO_GSI_SHRAM_PTR_MCS_SCRATCH_BASE_ADDR_IN)
#define HWIO_GSI_SHRAM_PTR_MCS_SCRATCH_BASE_ADDR_SHRAM_PTR_BMSK                                              0xffff
#define HWIO_GSI_SHRAM_PTR_MCS_SCRATCH_BASE_ADDR_SHRAM_PTR_SHFT                                                 0x0

#define HWIO_GSI_SHRAM_PTR_MCS_SCRATCH1_BASE_ADDR_ADDR                                                   (GSI_REG_BASE      + 0x00000214)
#define HWIO_GSI_SHRAM_PTR_MCS_SCRATCH1_BASE_ADDR_PHYS                                                   (GSI_REG_BASE_PHYS + 0x00000214)
#define HWIO_GSI_SHRAM_PTR_MCS_SCRATCH1_BASE_ADDR_OFFS                                                   (GSI_REG_BASE_OFFS + 0x00000214)
#define HWIO_GSI_SHRAM_PTR_MCS_SCRATCH1_BASE_ADDR_RMSK                                                       0xffff
#define HWIO_GSI_SHRAM_PTR_MCS_SCRATCH1_BASE_ADDR_ATTR                                                          0x3
#define HWIO_GSI_SHRAM_PTR_MCS_SCRATCH1_BASE_ADDR_IN          \
        in_dword_masked(HWIO_GSI_SHRAM_PTR_MCS_SCRATCH1_BASE_ADDR_ADDR, HWIO_GSI_SHRAM_PTR_MCS_SCRATCH1_BASE_ADDR_RMSK)
#define HWIO_GSI_SHRAM_PTR_MCS_SCRATCH1_BASE_ADDR_INM(m)      \
        in_dword_masked(HWIO_GSI_SHRAM_PTR_MCS_SCRATCH1_BASE_ADDR_ADDR, m)
#define HWIO_GSI_SHRAM_PTR_MCS_SCRATCH1_BASE_ADDR_OUT(v)      \
        out_dword(HWIO_GSI_SHRAM_PTR_MCS_SCRATCH1_BASE_ADDR_ADDR,v)
#define HWIO_GSI_SHRAM_PTR_MCS_SCRATCH1_BASE_ADDR_OUTM(m,v) \
        out_dword_masked_ns(HWIO_GSI_SHRAM_PTR_MCS_SCRATCH1_BASE_ADDR_ADDR,m,v,HWIO_GSI_SHRAM_PTR_MCS_SCRATCH1_BASE_ADDR_IN)
#define HWIO_GSI_SHRAM_PTR_MCS_SCRATCH1_BASE_ADDR_SHRAM_PTR_BMSK                                             0xffff
#define HWIO_GSI_SHRAM_PTR_MCS_SCRATCH1_BASE_ADDR_SHRAM_PTR_SHFT                                                0x0

#define HWIO_GSI_SHRAM_PTR_MCS_SCRATCH2_BASE_ADDR_ADDR                                                   (GSI_REG_BASE      + 0x00000218)
#define HWIO_GSI_SHRAM_PTR_MCS_SCRATCH2_BASE_ADDR_PHYS                                                   (GSI_REG_BASE_PHYS + 0x00000218)
#define HWIO_GSI_SHRAM_PTR_MCS_SCRATCH2_BASE_ADDR_OFFS                                                   (GSI_REG_BASE_OFFS + 0x00000218)
#define HWIO_GSI_SHRAM_PTR_MCS_SCRATCH2_BASE_ADDR_RMSK                                                       0xffff
#define HWIO_GSI_SHRAM_PTR_MCS_SCRATCH2_BASE_ADDR_ATTR                                                          0x3
#define HWIO_GSI_SHRAM_PTR_MCS_SCRATCH2_BASE_ADDR_IN          \
        in_dword_masked(HWIO_GSI_SHRAM_PTR_MCS_SCRATCH2_BASE_ADDR_ADDR, HWIO_GSI_SHRAM_PTR_MCS_SCRATCH2_BASE_ADDR_RMSK)
#define HWIO_GSI_SHRAM_PTR_MCS_SCRATCH2_BASE_ADDR_INM(m)      \
        in_dword_masked(HWIO_GSI_SHRAM_PTR_MCS_SCRATCH2_BASE_ADDR_ADDR, m)
#define HWIO_GSI_SHRAM_PTR_MCS_SCRATCH2_BASE_ADDR_OUT(v)      \
        out_dword(HWIO_GSI_SHRAM_PTR_MCS_SCRATCH2_BASE_ADDR_ADDR,v)
#define HWIO_GSI_SHRAM_PTR_MCS_SCRATCH2_BASE_ADDR_OUTM(m,v) \
        out_dword_masked_ns(HWIO_GSI_SHRAM_PTR_MCS_SCRATCH2_BASE_ADDR_ADDR,m,v,HWIO_GSI_SHRAM_PTR_MCS_SCRATCH2_BASE_ADDR_IN)
#define HWIO_GSI_SHRAM_PTR_MCS_SCRATCH2_BASE_ADDR_SHRAM_PTR_BMSK                                             0xffff
#define HWIO_GSI_SHRAM_PTR_MCS_SCRATCH2_BASE_ADDR_SHRAM_PTR_SHFT                                                0x0

#define HWIO_GSI_SHRAM_PTR_MCS_SCRATCH3_BASE_ADDR_ADDR                                                   (GSI_REG_BASE      + 0x0000021c)
#define HWIO_GSI_SHRAM_PTR_MCS_SCRATCH3_BASE_ADDR_PHYS                                                   (GSI_REG_BASE_PHYS + 0x0000021c)
#define HWIO_GSI_SHRAM_PTR_MCS_SCRATCH3_BASE_ADDR_OFFS                                                   (GSI_REG_BASE_OFFS + 0x0000021c)
#define HWIO_GSI_SHRAM_PTR_MCS_SCRATCH3_BASE_ADDR_RMSK                                                       0xffff
#define HWIO_GSI_SHRAM_PTR_MCS_SCRATCH3_BASE_ADDR_ATTR                                                          0x3
#define HWIO_GSI_SHRAM_PTR_MCS_SCRATCH3_BASE_ADDR_IN          \
        in_dword_masked(HWIO_GSI_SHRAM_PTR_MCS_SCRATCH3_BASE_ADDR_ADDR, HWIO_GSI_SHRAM_PTR_MCS_SCRATCH3_BASE_ADDR_RMSK)
#define HWIO_GSI_SHRAM_PTR_MCS_SCRATCH3_BASE_ADDR_INM(m)      \
        in_dword_masked(HWIO_GSI_SHRAM_PTR_MCS_SCRATCH3_BASE_ADDR_ADDR, m)
#define HWIO_GSI_SHRAM_PTR_MCS_SCRATCH3_BASE_ADDR_OUT(v)      \
        out_dword(HWIO_GSI_SHRAM_PTR_MCS_SCRATCH3_BASE_ADDR_ADDR,v)
#define HWIO_GSI_SHRAM_PTR_MCS_SCRATCH3_BASE_ADDR_OUTM(m,v) \
        out_dword_masked_ns(HWIO_GSI_SHRAM_PTR_MCS_SCRATCH3_BASE_ADDR_ADDR,m,v,HWIO_GSI_SHRAM_PTR_MCS_SCRATCH3_BASE_ADDR_IN)
#define HWIO_GSI_SHRAM_PTR_MCS_SCRATCH3_BASE_ADDR_SHRAM_PTR_BMSK                                             0xffff
#define HWIO_GSI_SHRAM_PTR_MCS_SCRATCH3_BASE_ADDR_SHRAM_PTR_SHFT                                                0x0

#define HWIO_GSI_SHRAM_PTR_CH_VP_TRANS_TABLE_BASE_ADDR_ADDR                                              (GSI_REG_BASE      + 0x00000254)
#define HWIO_GSI_SHRAM_PTR_CH_VP_TRANS_TABLE_BASE_ADDR_PHYS                                              (GSI_REG_BASE_PHYS + 0x00000254)
#define HWIO_GSI_SHRAM_PTR_CH_VP_TRANS_TABLE_BASE_ADDR_OFFS                                              (GSI_REG_BASE_OFFS + 0x00000254)
#define HWIO_GSI_SHRAM_PTR_CH_VP_TRANS_TABLE_BASE_ADDR_RMSK                                                  0xffff
#define HWIO_GSI_SHRAM_PTR_CH_VP_TRANS_TABLE_BASE_ADDR_ATTR                                                     0x3
#define HWIO_GSI_SHRAM_PTR_CH_VP_TRANS_TABLE_BASE_ADDR_IN          \
        in_dword_masked(HWIO_GSI_SHRAM_PTR_CH_VP_TRANS_TABLE_BASE_ADDR_ADDR, HWIO_GSI_SHRAM_PTR_CH_VP_TRANS_TABLE_BASE_ADDR_RMSK)
#define HWIO_GSI_SHRAM_PTR_CH_VP_TRANS_TABLE_BASE_ADDR_INM(m)      \
        in_dword_masked(HWIO_GSI_SHRAM_PTR_CH_VP_TRANS_TABLE_BASE_ADDR_ADDR, m)
#define HWIO_GSI_SHRAM_PTR_CH_VP_TRANS_TABLE_BASE_ADDR_OUT(v)      \
        out_dword(HWIO_GSI_SHRAM_PTR_CH_VP_TRANS_TABLE_BASE_ADDR_ADDR,v)
#define HWIO_GSI_SHRAM_PTR_CH_VP_TRANS_TABLE_BASE_ADDR_OUTM(m,v) \
        out_dword_masked_ns(HWIO_GSI_SHRAM_PTR_CH_VP_TRANS_TABLE_BASE_ADDR_ADDR,m,v,HWIO_GSI_SHRAM_PTR_CH_VP_TRANS_TABLE_BASE_ADDR_IN)
#define HWIO_GSI_SHRAM_PTR_CH_VP_TRANS_TABLE_BASE_ADDR_SHRAM_PTR_BMSK                                        0xffff
#define HWIO_GSI_SHRAM_PTR_CH_VP_TRANS_TABLE_BASE_ADDR_SHRAM_PTR_SHFT                                           0x0

#define HWIO_GSI_SHRAM_PTR_EV_VP_TRANS_TABLE_BASE_ADDR_ADDR                                              (GSI_REG_BASE      + 0x00000258)
#define HWIO_GSI_SHRAM_PTR_EV_VP_TRANS_TABLE_BASE_ADDR_PHYS                                              (GSI_REG_BASE_PHYS + 0x00000258)
#define HWIO_GSI_SHRAM_PTR_EV_VP_TRANS_TABLE_BASE_ADDR_OFFS                                              (GSI_REG_BASE_OFFS + 0x00000258)
#define HWIO_GSI_SHRAM_PTR_EV_VP_TRANS_TABLE_BASE_ADDR_RMSK                                                  0xffff
#define HWIO_GSI_SHRAM_PTR_EV_VP_TRANS_TABLE_BASE_ADDR_ATTR                                                     0x3
#define HWIO_GSI_SHRAM_PTR_EV_VP_TRANS_TABLE_BASE_ADDR_IN          \
        in_dword_masked(HWIO_GSI_SHRAM_PTR_EV_VP_TRANS_TABLE_BASE_ADDR_ADDR, HWIO_GSI_SHRAM_PTR_EV_VP_TRANS_TABLE_BASE_ADDR_RMSK)
#define HWIO_GSI_SHRAM_PTR_EV_VP_TRANS_TABLE_BASE_ADDR_INM(m)      \
        in_dword_masked(HWIO_GSI_SHRAM_PTR_EV_VP_TRANS_TABLE_BASE_ADDR_ADDR, m)
#define HWIO_GSI_SHRAM_PTR_EV_VP_TRANS_TABLE_BASE_ADDR_OUT(v)      \
        out_dword(HWIO_GSI_SHRAM_PTR_EV_VP_TRANS_TABLE_BASE_ADDR_ADDR,v)
#define HWIO_GSI_SHRAM_PTR_EV_VP_TRANS_TABLE_BASE_ADDR_OUTM(m,v) \
        out_dword_masked_ns(HWIO_GSI_SHRAM_PTR_EV_VP_TRANS_TABLE_BASE_ADDR_ADDR,m,v,HWIO_GSI_SHRAM_PTR_EV_VP_TRANS_TABLE_BASE_ADDR_IN)
#define HWIO_GSI_SHRAM_PTR_EV_VP_TRANS_TABLE_BASE_ADDR_SHRAM_PTR_BMSK                                        0xffff
#define HWIO_GSI_SHRAM_PTR_EV_VP_TRANS_TABLE_BASE_ADDR_SHRAM_PTR_SHFT                                           0x0

#define HWIO_GSI_SHRAM_PTR_USER_INFO_DATA_BASE_ADDR_ADDR                                                 (GSI_REG_BASE      + 0x0000025c)
#define HWIO_GSI_SHRAM_PTR_USER_INFO_DATA_BASE_ADDR_PHYS                                                 (GSI_REG_BASE_PHYS + 0x0000025c)
#define HWIO_GSI_SHRAM_PTR_USER_INFO_DATA_BASE_ADDR_OFFS                                                 (GSI_REG_BASE_OFFS + 0x0000025c)
#define HWIO_GSI_SHRAM_PTR_USER_INFO_DATA_BASE_ADDR_RMSK                                                     0xffff
#define HWIO_GSI_SHRAM_PTR_USER_INFO_DATA_BASE_ADDR_ATTR                                                        0x3
#define HWIO_GSI_SHRAM_PTR_USER_INFO_DATA_BASE_ADDR_IN          \
        in_dword_masked(HWIO_GSI_SHRAM_PTR_USER_INFO_DATA_BASE_ADDR_ADDR, HWIO_GSI_SHRAM_PTR_USER_INFO_DATA_BASE_ADDR_RMSK)
#define HWIO_GSI_SHRAM_PTR_USER_INFO_DATA_BASE_ADDR_INM(m)      \
        in_dword_masked(HWIO_GSI_SHRAM_PTR_USER_INFO_DATA_BASE_ADDR_ADDR, m)
#define HWIO_GSI_SHRAM_PTR_USER_INFO_DATA_BASE_ADDR_OUT(v)      \
        out_dword(HWIO_GSI_SHRAM_PTR_USER_INFO_DATA_BASE_ADDR_ADDR,v)
#define HWIO_GSI_SHRAM_PTR_USER_INFO_DATA_BASE_ADDR_OUTM(m,v) \
        out_dword_masked_ns(HWIO_GSI_SHRAM_PTR_USER_INFO_DATA_BASE_ADDR_ADDR,m,v,HWIO_GSI_SHRAM_PTR_USER_INFO_DATA_BASE_ADDR_IN)
#define HWIO_GSI_SHRAM_PTR_USER_INFO_DATA_BASE_ADDR_SHRAM_PTR_BMSK                                           0xffff
#define HWIO_GSI_SHRAM_PTR_USER_INFO_DATA_BASE_ADDR_SHRAM_PTR_SHFT                                              0x0

#define HWIO_GSI_SHRAM_PTR_EE_CMD_FIFO_BASE_ADDR_ADDR                                                    (GSI_REG_BASE      + 0x00000260)
#define HWIO_GSI_SHRAM_PTR_EE_CMD_FIFO_BASE_ADDR_PHYS                                                    (GSI_REG_BASE_PHYS + 0x00000260)
#define HWIO_GSI_SHRAM_PTR_EE_CMD_FIFO_BASE_ADDR_OFFS                                                    (GSI_REG_BASE_OFFS + 0x00000260)
#define HWIO_GSI_SHRAM_PTR_EE_CMD_FIFO_BASE_ADDR_RMSK                                                        0xffff
#define HWIO_GSI_SHRAM_PTR_EE_CMD_FIFO_BASE_ADDR_ATTR                                                           0x3
#define HWIO_GSI_SHRAM_PTR_EE_CMD_FIFO_BASE_ADDR_IN          \
        in_dword_masked(HWIO_GSI_SHRAM_PTR_EE_CMD_FIFO_BASE_ADDR_ADDR, HWIO_GSI_SHRAM_PTR_EE_CMD_FIFO_BASE_ADDR_RMSK)
#define HWIO_GSI_SHRAM_PTR_EE_CMD_FIFO_BASE_ADDR_INM(m)      \
        in_dword_masked(HWIO_GSI_SHRAM_PTR_EE_CMD_FIFO_BASE_ADDR_ADDR, m)
#define HWIO_GSI_SHRAM_PTR_EE_CMD_FIFO_BASE_ADDR_OUT(v)      \
        out_dword(HWIO_GSI_SHRAM_PTR_EE_CMD_FIFO_BASE_ADDR_ADDR,v)
#define HWIO_GSI_SHRAM_PTR_EE_CMD_FIFO_BASE_ADDR_OUTM(m,v) \
        out_dword_masked_ns(HWIO_GSI_SHRAM_PTR_EE_CMD_FIFO_BASE_ADDR_ADDR,m,v,HWIO_GSI_SHRAM_PTR_EE_CMD_FIFO_BASE_ADDR_IN)
#define HWIO_GSI_SHRAM_PTR_EE_CMD_FIFO_BASE_ADDR_SHRAM_PTR_BMSK                                              0xffff
#define HWIO_GSI_SHRAM_PTR_EE_CMD_FIFO_BASE_ADDR_SHRAM_PTR_SHFT                                                 0x0

#define HWIO_GSI_SHRAM_PTR_CH_CMD_FIFO_BASE_ADDR_ADDR                                                    (GSI_REG_BASE      + 0x00000264)
#define HWIO_GSI_SHRAM_PTR_CH_CMD_FIFO_BASE_ADDR_PHYS                                                    (GSI_REG_BASE_PHYS + 0x00000264)
#define HWIO_GSI_SHRAM_PTR_CH_CMD_FIFO_BASE_ADDR_OFFS                                                    (GSI_REG_BASE_OFFS + 0x00000264)
#define HWIO_GSI_SHRAM_PTR_CH_CMD_FIFO_BASE_ADDR_RMSK                                                        0xffff
#define HWIO_GSI_SHRAM_PTR_CH_CMD_FIFO_BASE_ADDR_ATTR                                                           0x3
#define HWIO_GSI_SHRAM_PTR_CH_CMD_FIFO_BASE_ADDR_IN          \
        in_dword_masked(HWIO_GSI_SHRAM_PTR_CH_CMD_FIFO_BASE_ADDR_ADDR, HWIO_GSI_SHRAM_PTR_CH_CMD_FIFO_BASE_ADDR_RMSK)
#define HWIO_GSI_SHRAM_PTR_CH_CMD_FIFO_BASE_ADDR_INM(m)      \
        in_dword_masked(HWIO_GSI_SHRAM_PTR_CH_CMD_FIFO_BASE_ADDR_ADDR, m)
#define HWIO_GSI_SHRAM_PTR_CH_CMD_FIFO_BASE_ADDR_OUT(v)      \
        out_dword(HWIO_GSI_SHRAM_PTR_CH_CMD_FIFO_BASE_ADDR_ADDR,v)
#define HWIO_GSI_SHRAM_PTR_CH_CMD_FIFO_BASE_ADDR_OUTM(m,v) \
        out_dword_masked_ns(HWIO_GSI_SHRAM_PTR_CH_CMD_FIFO_BASE_ADDR_ADDR,m,v,HWIO_GSI_SHRAM_PTR_CH_CMD_FIFO_BASE_ADDR_IN)
#define HWIO_GSI_SHRAM_PTR_CH_CMD_FIFO_BASE_ADDR_SHRAM_PTR_BMSK                                              0xffff
#define HWIO_GSI_SHRAM_PTR_CH_CMD_FIFO_BASE_ADDR_SHRAM_PTR_SHFT                                                 0x0

#define HWIO_GSI_SHRAM_PTR_EVE_ED_STORAGE_BASE_ADDR_ADDR                                                 (GSI_REG_BASE      + 0x00000268)
#define HWIO_GSI_SHRAM_PTR_EVE_ED_STORAGE_BASE_ADDR_PHYS                                                 (GSI_REG_BASE_PHYS + 0x00000268)
#define HWIO_GSI_SHRAM_PTR_EVE_ED_STORAGE_BASE_ADDR_OFFS                                                 (GSI_REG_BASE_OFFS + 0x00000268)
#define HWIO_GSI_SHRAM_PTR_EVE_ED_STORAGE_BASE_ADDR_RMSK                                                     0xffff
#define HWIO_GSI_SHRAM_PTR_EVE_ED_STORAGE_BASE_ADDR_ATTR                                                        0x3
#define HWIO_GSI_SHRAM_PTR_EVE_ED_STORAGE_BASE_ADDR_IN          \
        in_dword_masked(HWIO_GSI_SHRAM_PTR_EVE_ED_STORAGE_BASE_ADDR_ADDR, HWIO_GSI_SHRAM_PTR_EVE_ED_STORAGE_BASE_ADDR_RMSK)
#define HWIO_GSI_SHRAM_PTR_EVE_ED_STORAGE_BASE_ADDR_INM(m)      \
        in_dword_masked(HWIO_GSI_SHRAM_PTR_EVE_ED_STORAGE_BASE_ADDR_ADDR, m)
#define HWIO_GSI_SHRAM_PTR_EVE_ED_STORAGE_BASE_ADDR_OUT(v)      \
        out_dword(HWIO_GSI_SHRAM_PTR_EVE_ED_STORAGE_BASE_ADDR_ADDR,v)
#define HWIO_GSI_SHRAM_PTR_EVE_ED_STORAGE_BASE_ADDR_OUTM(m,v) \
        out_dword_masked_ns(HWIO_GSI_SHRAM_PTR_EVE_ED_STORAGE_BASE_ADDR_ADDR,m,v,HWIO_GSI_SHRAM_PTR_EVE_ED_STORAGE_BASE_ADDR_IN)
#define HWIO_GSI_SHRAM_PTR_EVE_ED_STORAGE_BASE_ADDR_SHRAM_PTR_BMSK                                           0xffff
#define HWIO_GSI_SHRAM_PTR_EVE_ED_STORAGE_BASE_ADDR_SHRAM_PTR_SHFT                                              0x0

#define HWIO_GSI_IRAM_PTR_CH_CMD_ADDR                                                                    (GSI_REG_BASE      + 0x00000400)
#define HWIO_GSI_IRAM_PTR_CH_CMD_PHYS                                                                    (GSI_REG_BASE_PHYS + 0x00000400)
#define HWIO_GSI_IRAM_PTR_CH_CMD_OFFS                                                                    (GSI_REG_BASE_OFFS + 0x00000400)
#define HWIO_GSI_IRAM_PTR_CH_CMD_RMSK                                                                         0xfff
#define HWIO_GSI_IRAM_PTR_CH_CMD_ATTR                                                                           0x3
#define HWIO_GSI_IRAM_PTR_CH_CMD_IN          \
        in_dword_masked(HWIO_GSI_IRAM_PTR_CH_CMD_ADDR, HWIO_GSI_IRAM_PTR_CH_CMD_RMSK)
#define HWIO_GSI_IRAM_PTR_CH_CMD_INM(m)      \
        in_dword_masked(HWIO_GSI_IRAM_PTR_CH_CMD_ADDR, m)
#define HWIO_GSI_IRAM_PTR_CH_CMD_OUT(v)      \
        out_dword(HWIO_GSI_IRAM_PTR_CH_CMD_ADDR,v)
#define HWIO_GSI_IRAM_PTR_CH_CMD_OUTM(m,v) \
        out_dword_masked_ns(HWIO_GSI_IRAM_PTR_CH_CMD_ADDR,m,v,HWIO_GSI_IRAM_PTR_CH_CMD_IN)
#define HWIO_GSI_IRAM_PTR_CH_CMD_IRAM_PTR_BMSK                                                                0xfff
#define HWIO_GSI_IRAM_PTR_CH_CMD_IRAM_PTR_SHFT                                                                  0x0

#define HWIO_GSI_IRAM_PTR_EE_GENERIC_CMD_ADDR                                                            (GSI_REG_BASE      + 0x00000404)
#define HWIO_GSI_IRAM_PTR_EE_GENERIC_CMD_PHYS                                                            (GSI_REG_BASE_PHYS + 0x00000404)
#define HWIO_GSI_IRAM_PTR_EE_GENERIC_CMD_OFFS                                                            (GSI_REG_BASE_OFFS + 0x00000404)
#define HWIO_GSI_IRAM_PTR_EE_GENERIC_CMD_RMSK                                                                 0xfff
#define HWIO_GSI_IRAM_PTR_EE_GENERIC_CMD_ATTR                                                                   0x3
#define HWIO_GSI_IRAM_PTR_EE_GENERIC_CMD_IN          \
        in_dword_masked(HWIO_GSI_IRAM_PTR_EE_GENERIC_CMD_ADDR, HWIO_GSI_IRAM_PTR_EE_GENERIC_CMD_RMSK)
#define HWIO_GSI_IRAM_PTR_EE_GENERIC_CMD_INM(m)      \
        in_dword_masked(HWIO_GSI_IRAM_PTR_EE_GENERIC_CMD_ADDR, m)
#define HWIO_GSI_IRAM_PTR_EE_GENERIC_CMD_OUT(v)      \
        out_dword(HWIO_GSI_IRAM_PTR_EE_GENERIC_CMD_ADDR,v)
#define HWIO_GSI_IRAM_PTR_EE_GENERIC_CMD_OUTM(m,v) \
        out_dword_masked_ns(HWIO_GSI_IRAM_PTR_EE_GENERIC_CMD_ADDR,m,v,HWIO_GSI_IRAM_PTR_EE_GENERIC_CMD_IN)
#define HWIO_GSI_IRAM_PTR_EE_GENERIC_CMD_IRAM_PTR_BMSK                                                        0xfff
#define HWIO_GSI_IRAM_PTR_EE_GENERIC_CMD_IRAM_PTR_SHFT                                                          0x0

#define HWIO_GSI_IRAM_PTR_TLV_CH_NOT_FULL_ADDR                                                           (GSI_REG_BASE      + 0x00000408)
#define HWIO_GSI_IRAM_PTR_TLV_CH_NOT_FULL_PHYS                                                           (GSI_REG_BASE_PHYS + 0x00000408)
#define HWIO_GSI_IRAM_PTR_TLV_CH_NOT_FULL_OFFS                                                           (GSI_REG_BASE_OFFS + 0x00000408)
#define HWIO_GSI_IRAM_PTR_TLV_CH_NOT_FULL_RMSK                                                                0xfff
#define HWIO_GSI_IRAM_PTR_TLV_CH_NOT_FULL_ATTR                                                                  0x3
#define HWIO_GSI_IRAM_PTR_TLV_CH_NOT_FULL_IN          \
        in_dword_masked(HWIO_GSI_IRAM_PTR_TLV_CH_NOT_FULL_ADDR, HWIO_GSI_IRAM_PTR_TLV_CH_NOT_FULL_RMSK)
#define HWIO_GSI_IRAM_PTR_TLV_CH_NOT_FULL_INM(m)      \
        in_dword_masked(HWIO_GSI_IRAM_PTR_TLV_CH_NOT_FULL_ADDR, m)
#define HWIO_GSI_IRAM_PTR_TLV_CH_NOT_FULL_OUT(v)      \
        out_dword(HWIO_GSI_IRAM_PTR_TLV_CH_NOT_FULL_ADDR,v)
#define HWIO_GSI_IRAM_PTR_TLV_CH_NOT_FULL_OUTM(m,v) \
        out_dword_masked_ns(HWIO_GSI_IRAM_PTR_TLV_CH_NOT_FULL_ADDR,m,v,HWIO_GSI_IRAM_PTR_TLV_CH_NOT_FULL_IN)
#define HWIO_GSI_IRAM_PTR_TLV_CH_NOT_FULL_IRAM_PTR_BMSK                                                       0xfff
#define HWIO_GSI_IRAM_PTR_TLV_CH_NOT_FULL_IRAM_PTR_SHFT                                                         0x0

#define HWIO_GSI_IRAM_PTR_MSI_DB_ADDR                                                                    (GSI_REG_BASE      + 0x00000414)
#define HWIO_GSI_IRAM_PTR_MSI_DB_PHYS                                                                    (GSI_REG_BASE_PHYS + 0x00000414)
#define HWIO_GSI_IRAM_PTR_MSI_DB_OFFS                                                                    (GSI_REG_BASE_OFFS + 0x00000414)
#define HWIO_GSI_IRAM_PTR_MSI_DB_RMSK                                                                         0xfff
#define HWIO_GSI_IRAM_PTR_MSI_DB_ATTR                                                                           0x3
#define HWIO_GSI_IRAM_PTR_MSI_DB_IN          \
        in_dword_masked(HWIO_GSI_IRAM_PTR_MSI_DB_ADDR, HWIO_GSI_IRAM_PTR_MSI_DB_RMSK)
#define HWIO_GSI_IRAM_PTR_MSI_DB_INM(m)      \
        in_dword_masked(HWIO_GSI_IRAM_PTR_MSI_DB_ADDR, m)
#define HWIO_GSI_IRAM_PTR_MSI_DB_OUT(v)      \
        out_dword(HWIO_GSI_IRAM_PTR_MSI_DB_ADDR,v)
#define HWIO_GSI_IRAM_PTR_MSI_DB_OUTM(m,v) \
        out_dword_masked_ns(HWIO_GSI_IRAM_PTR_MSI_DB_ADDR,m,v,HWIO_GSI_IRAM_PTR_MSI_DB_IN)
#define HWIO_GSI_IRAM_PTR_MSI_DB_IRAM_PTR_BMSK                                                                0xfff
#define HWIO_GSI_IRAM_PTR_MSI_DB_IRAM_PTR_SHFT                                                                  0x0

#define HWIO_GSI_IRAM_PTR_CH_DB_ADDR                                                                     (GSI_REG_BASE      + 0x00000418)
#define HWIO_GSI_IRAM_PTR_CH_DB_PHYS                                                                     (GSI_REG_BASE_PHYS + 0x00000418)
#define HWIO_GSI_IRAM_PTR_CH_DB_OFFS                                                                     (GSI_REG_BASE_OFFS + 0x00000418)
#define HWIO_GSI_IRAM_PTR_CH_DB_RMSK                                                                          0xfff
#define HWIO_GSI_IRAM_PTR_CH_DB_ATTR                                                                            0x3
#define HWIO_GSI_IRAM_PTR_CH_DB_IN          \
        in_dword_masked(HWIO_GSI_IRAM_PTR_CH_DB_ADDR, HWIO_GSI_IRAM_PTR_CH_DB_RMSK)
#define HWIO_GSI_IRAM_PTR_CH_DB_INM(m)      \
        in_dword_masked(HWIO_GSI_IRAM_PTR_CH_DB_ADDR, m)
#define HWIO_GSI_IRAM_PTR_CH_DB_OUT(v)      \
        out_dword(HWIO_GSI_IRAM_PTR_CH_DB_ADDR,v)
#define HWIO_GSI_IRAM_PTR_CH_DB_OUTM(m,v) \
        out_dword_masked_ns(HWIO_GSI_IRAM_PTR_CH_DB_ADDR,m,v,HWIO_GSI_IRAM_PTR_CH_DB_IN)
#define HWIO_GSI_IRAM_PTR_CH_DB_IRAM_PTR_BMSK                                                                 0xfff
#define HWIO_GSI_IRAM_PTR_CH_DB_IRAM_PTR_SHFT                                                                   0x0

#define HWIO_GSI_IRAM_PTR_EV_DB_ADDR                                                                     (GSI_REG_BASE      + 0x0000041c)
#define HWIO_GSI_IRAM_PTR_EV_DB_PHYS                                                                     (GSI_REG_BASE_PHYS + 0x0000041c)
#define HWIO_GSI_IRAM_PTR_EV_DB_OFFS                                                                     (GSI_REG_BASE_OFFS + 0x0000041c)
#define HWIO_GSI_IRAM_PTR_EV_DB_RMSK                                                                          0xfff
#define HWIO_GSI_IRAM_PTR_EV_DB_ATTR                                                                            0x3
#define HWIO_GSI_IRAM_PTR_EV_DB_IN          \
        in_dword_masked(HWIO_GSI_IRAM_PTR_EV_DB_ADDR, HWIO_GSI_IRAM_PTR_EV_DB_RMSK)
#define HWIO_GSI_IRAM_PTR_EV_DB_INM(m)      \
        in_dword_masked(HWIO_GSI_IRAM_PTR_EV_DB_ADDR, m)
#define HWIO_GSI_IRAM_PTR_EV_DB_OUT(v)      \
        out_dword(HWIO_GSI_IRAM_PTR_EV_DB_ADDR,v)
#define HWIO_GSI_IRAM_PTR_EV_DB_OUTM(m,v) \
        out_dword_masked_ns(HWIO_GSI_IRAM_PTR_EV_DB_ADDR,m,v,HWIO_GSI_IRAM_PTR_EV_DB_IN)
#define HWIO_GSI_IRAM_PTR_EV_DB_IRAM_PTR_BMSK                                                                 0xfff
#define HWIO_GSI_IRAM_PTR_EV_DB_IRAM_PTR_SHFT                                                                   0x0

#define HWIO_GSI_IRAM_PTR_NEW_RE_ADDR                                                                    (GSI_REG_BASE      + 0x00000420)
#define HWIO_GSI_IRAM_PTR_NEW_RE_PHYS                                                                    (GSI_REG_BASE_PHYS + 0x00000420)
#define HWIO_GSI_IRAM_PTR_NEW_RE_OFFS                                                                    (GSI_REG_BASE_OFFS + 0x00000420)
#define HWIO_GSI_IRAM_PTR_NEW_RE_RMSK                                                                         0xfff
#define HWIO_GSI_IRAM_PTR_NEW_RE_ATTR                                                                           0x3
#define HWIO_GSI_IRAM_PTR_NEW_RE_IN          \
        in_dword_masked(HWIO_GSI_IRAM_PTR_NEW_RE_ADDR, HWIO_GSI_IRAM_PTR_NEW_RE_RMSK)
#define HWIO_GSI_IRAM_PTR_NEW_RE_INM(m)      \
        in_dword_masked(HWIO_GSI_IRAM_PTR_NEW_RE_ADDR, m)
#define HWIO_GSI_IRAM_PTR_NEW_RE_OUT(v)      \
        out_dword(HWIO_GSI_IRAM_PTR_NEW_RE_ADDR,v)
#define HWIO_GSI_IRAM_PTR_NEW_RE_OUTM(m,v) \
        out_dword_masked_ns(HWIO_GSI_IRAM_PTR_NEW_RE_ADDR,m,v,HWIO_GSI_IRAM_PTR_NEW_RE_IN)
#define HWIO_GSI_IRAM_PTR_NEW_RE_IRAM_PTR_BMSK                                                                0xfff
#define HWIO_GSI_IRAM_PTR_NEW_RE_IRAM_PTR_SHFT                                                                  0x0

#define HWIO_GSI_IRAM_PTR_CH_DIS_COMP_ADDR                                                               (GSI_REG_BASE      + 0x00000424)
#define HWIO_GSI_IRAM_PTR_CH_DIS_COMP_PHYS                                                               (GSI_REG_BASE_PHYS + 0x00000424)
#define HWIO_GSI_IRAM_PTR_CH_DIS_COMP_OFFS                                                               (GSI_REG_BASE_OFFS + 0x00000424)
#define HWIO_GSI_IRAM_PTR_CH_DIS_COMP_RMSK                                                                    0xfff
#define HWIO_GSI_IRAM_PTR_CH_DIS_COMP_ATTR                                                                      0x3
#define HWIO_GSI_IRAM_PTR_CH_DIS_COMP_IN          \
        in_dword_masked(HWIO_GSI_IRAM_PTR_CH_DIS_COMP_ADDR, HWIO_GSI_IRAM_PTR_CH_DIS_COMP_RMSK)
#define HWIO_GSI_IRAM_PTR_CH_DIS_COMP_INM(m)      \
        in_dword_masked(HWIO_GSI_IRAM_PTR_CH_DIS_COMP_ADDR, m)
#define HWIO_GSI_IRAM_PTR_CH_DIS_COMP_OUT(v)      \
        out_dword(HWIO_GSI_IRAM_PTR_CH_DIS_COMP_ADDR,v)
#define HWIO_GSI_IRAM_PTR_CH_DIS_COMP_OUTM(m,v) \
        out_dword_masked_ns(HWIO_GSI_IRAM_PTR_CH_DIS_COMP_ADDR,m,v,HWIO_GSI_IRAM_PTR_CH_DIS_COMP_IN)
#define HWIO_GSI_IRAM_PTR_CH_DIS_COMP_IRAM_PTR_BMSK                                                           0xfff
#define HWIO_GSI_IRAM_PTR_CH_DIS_COMP_IRAM_PTR_SHFT                                                             0x0

#define HWIO_GSI_IRAM_PTR_CH_EMPTY_ADDR                                                                  (GSI_REG_BASE      + 0x00000428)
#define HWIO_GSI_IRAM_PTR_CH_EMPTY_PHYS                                                                  (GSI_REG_BASE_PHYS + 0x00000428)
#define HWIO_GSI_IRAM_PTR_CH_EMPTY_OFFS                                                                  (GSI_REG_BASE_OFFS + 0x00000428)
#define HWIO_GSI_IRAM_PTR_CH_EMPTY_RMSK                                                                       0xfff
#define HWIO_GSI_IRAM_PTR_CH_EMPTY_ATTR                                                                         0x3
#define HWIO_GSI_IRAM_PTR_CH_EMPTY_IN          \
        in_dword_masked(HWIO_GSI_IRAM_PTR_CH_EMPTY_ADDR, HWIO_GSI_IRAM_PTR_CH_EMPTY_RMSK)
#define HWIO_GSI_IRAM_PTR_CH_EMPTY_INM(m)      \
        in_dword_masked(HWIO_GSI_IRAM_PTR_CH_EMPTY_ADDR, m)
#define HWIO_GSI_IRAM_PTR_CH_EMPTY_OUT(v)      \
        out_dword(HWIO_GSI_IRAM_PTR_CH_EMPTY_ADDR,v)
#define HWIO_GSI_IRAM_PTR_CH_EMPTY_OUTM(m,v) \
        out_dword_masked_ns(HWIO_GSI_IRAM_PTR_CH_EMPTY_ADDR,m,v,HWIO_GSI_IRAM_PTR_CH_EMPTY_IN)
#define HWIO_GSI_IRAM_PTR_CH_EMPTY_IRAM_PTR_BMSK                                                              0xfff
#define HWIO_GSI_IRAM_PTR_CH_EMPTY_IRAM_PTR_SHFT                                                                0x0

#define HWIO_GSI_IRAM_PTR_EVENT_GEN_COMP_ADDR                                                            (GSI_REG_BASE      + 0x0000042c)
#define HWIO_GSI_IRAM_PTR_EVENT_GEN_COMP_PHYS                                                            (GSI_REG_BASE_PHYS + 0x0000042c)
#define HWIO_GSI_IRAM_PTR_EVENT_GEN_COMP_OFFS                                                            (GSI_REG_BASE_OFFS + 0x0000042c)
#define HWIO_GSI_IRAM_PTR_EVENT_GEN_COMP_RMSK                                                                 0xfff
#define HWIO_GSI_IRAM_PTR_EVENT_GEN_COMP_ATTR                                                                   0x3
#define HWIO_GSI_IRAM_PTR_EVENT_GEN_COMP_IN          \
        in_dword_masked(HWIO_GSI_IRAM_PTR_EVENT_GEN_COMP_ADDR, HWIO_GSI_IRAM_PTR_EVENT_GEN_COMP_RMSK)
#define HWIO_GSI_IRAM_PTR_EVENT_GEN_COMP_INM(m)      \
        in_dword_masked(HWIO_GSI_IRAM_PTR_EVENT_GEN_COMP_ADDR, m)
#define HWIO_GSI_IRAM_PTR_EVENT_GEN_COMP_OUT(v)      \
        out_dword(HWIO_GSI_IRAM_PTR_EVENT_GEN_COMP_ADDR,v)
#define HWIO_GSI_IRAM_PTR_EVENT_GEN_COMP_OUTM(m,v) \
        out_dword_masked_ns(HWIO_GSI_IRAM_PTR_EVENT_GEN_COMP_ADDR,m,v,HWIO_GSI_IRAM_PTR_EVENT_GEN_COMP_IN)
#define HWIO_GSI_IRAM_PTR_EVENT_GEN_COMP_IRAM_PTR_BMSK                                                        0xfff
#define HWIO_GSI_IRAM_PTR_EVENT_GEN_COMP_IRAM_PTR_SHFT                                                          0x0

#define HWIO_GSI_IRAM_PTR_PERIPH_IF_TLV_IN_0_ADDR                                                        (GSI_REG_BASE      + 0x00000430)
#define HWIO_GSI_IRAM_PTR_PERIPH_IF_TLV_IN_0_PHYS                                                        (GSI_REG_BASE_PHYS + 0x00000430)
#define HWIO_GSI_IRAM_PTR_PERIPH_IF_TLV_IN_0_OFFS                                                        (GSI_REG_BASE_OFFS + 0x00000430)
#define HWIO_GSI_IRAM_PTR_PERIPH_IF_TLV_IN_0_RMSK                                                             0xfff
#define HWIO_GSI_IRAM_PTR_PERIPH_IF_TLV_IN_0_ATTR                                                               0x3
#define HWIO_GSI_IRAM_PTR_PERIPH_IF_TLV_IN_0_IN          \
        in_dword_masked(HWIO_GSI_IRAM_PTR_PERIPH_IF_TLV_IN_0_ADDR, HWIO_GSI_IRAM_PTR_PERIPH_IF_TLV_IN_0_RMSK)
#define HWIO_GSI_IRAM_PTR_PERIPH_IF_TLV_IN_0_INM(m)      \
        in_dword_masked(HWIO_GSI_IRAM_PTR_PERIPH_IF_TLV_IN_0_ADDR, m)
#define HWIO_GSI_IRAM_PTR_PERIPH_IF_TLV_IN_0_OUT(v)      \
        out_dword(HWIO_GSI_IRAM_PTR_PERIPH_IF_TLV_IN_0_ADDR,v)
#define HWIO_GSI_IRAM_PTR_PERIPH_IF_TLV_IN_0_OUTM(m,v) \
        out_dword_masked_ns(HWIO_GSI_IRAM_PTR_PERIPH_IF_TLV_IN_0_ADDR,m,v,HWIO_GSI_IRAM_PTR_PERIPH_IF_TLV_IN_0_IN)
#define HWIO_GSI_IRAM_PTR_PERIPH_IF_TLV_IN_0_IRAM_PTR_BMSK                                                    0xfff
#define HWIO_GSI_IRAM_PTR_PERIPH_IF_TLV_IN_0_IRAM_PTR_SHFT                                                      0x0

#define HWIO_GSI_IRAM_PTR_PERIPH_IF_TLV_IN_2_ADDR                                                        (GSI_REG_BASE      + 0x00000434)
#define HWIO_GSI_IRAM_PTR_PERIPH_IF_TLV_IN_2_PHYS                                                        (GSI_REG_BASE_PHYS + 0x00000434)
#define HWIO_GSI_IRAM_PTR_PERIPH_IF_TLV_IN_2_OFFS                                                        (GSI_REG_BASE_OFFS + 0x00000434)
#define HWIO_GSI_IRAM_PTR_PERIPH_IF_TLV_IN_2_RMSK                                                             0xfff
#define HWIO_GSI_IRAM_PTR_PERIPH_IF_TLV_IN_2_ATTR                                                               0x3
#define HWIO_GSI_IRAM_PTR_PERIPH_IF_TLV_IN_2_IN          \
        in_dword_masked(HWIO_GSI_IRAM_PTR_PERIPH_IF_TLV_IN_2_ADDR, HWIO_GSI_IRAM_PTR_PERIPH_IF_TLV_IN_2_RMSK)
#define HWIO_GSI_IRAM_PTR_PERIPH_IF_TLV_IN_2_INM(m)      \
        in_dword_masked(HWIO_GSI_IRAM_PTR_PERIPH_IF_TLV_IN_2_ADDR, m)
#define HWIO_GSI_IRAM_PTR_PERIPH_IF_TLV_IN_2_OUT(v)      \
        out_dword(HWIO_GSI_IRAM_PTR_PERIPH_IF_TLV_IN_2_ADDR,v)
#define HWIO_GSI_IRAM_PTR_PERIPH_IF_TLV_IN_2_OUTM(m,v) \
        out_dword_masked_ns(HWIO_GSI_IRAM_PTR_PERIPH_IF_TLV_IN_2_ADDR,m,v,HWIO_GSI_IRAM_PTR_PERIPH_IF_TLV_IN_2_IN)
#define HWIO_GSI_IRAM_PTR_PERIPH_IF_TLV_IN_2_IRAM_PTR_BMSK                                                    0xfff
#define HWIO_GSI_IRAM_PTR_PERIPH_IF_TLV_IN_2_IRAM_PTR_SHFT                                                      0x0

#define HWIO_GSI_IRAM_PTR_PERIPH_IF_TLV_IN_1_ADDR                                                        (GSI_REG_BASE      + 0x00000438)
#define HWIO_GSI_IRAM_PTR_PERIPH_IF_TLV_IN_1_PHYS                                                        (GSI_REG_BASE_PHYS + 0x00000438)
#define HWIO_GSI_IRAM_PTR_PERIPH_IF_TLV_IN_1_OFFS                                                        (GSI_REG_BASE_OFFS + 0x00000438)
#define HWIO_GSI_IRAM_PTR_PERIPH_IF_TLV_IN_1_RMSK                                                             0xfff
#define HWIO_GSI_IRAM_PTR_PERIPH_IF_TLV_IN_1_ATTR                                                               0x3
#define HWIO_GSI_IRAM_PTR_PERIPH_IF_TLV_IN_1_IN          \
        in_dword_masked(HWIO_GSI_IRAM_PTR_PERIPH_IF_TLV_IN_1_ADDR, HWIO_GSI_IRAM_PTR_PERIPH_IF_TLV_IN_1_RMSK)
#define HWIO_GSI_IRAM_PTR_PERIPH_IF_TLV_IN_1_INM(m)      \
        in_dword_masked(HWIO_GSI_IRAM_PTR_PERIPH_IF_TLV_IN_1_ADDR, m)
#define HWIO_GSI_IRAM_PTR_PERIPH_IF_TLV_IN_1_OUT(v)      \
        out_dword(HWIO_GSI_IRAM_PTR_PERIPH_IF_TLV_IN_1_ADDR,v)
#define HWIO_GSI_IRAM_PTR_PERIPH_IF_TLV_IN_1_OUTM(m,v) \
        out_dword_masked_ns(HWIO_GSI_IRAM_PTR_PERIPH_IF_TLV_IN_1_ADDR,m,v,HWIO_GSI_IRAM_PTR_PERIPH_IF_TLV_IN_1_IN)
#define HWIO_GSI_IRAM_PTR_PERIPH_IF_TLV_IN_1_IRAM_PTR_BMSK                                                    0xfff
#define HWIO_GSI_IRAM_PTR_PERIPH_IF_TLV_IN_1_IRAM_PTR_SHFT                                                      0x0

#define HWIO_GSI_IRAM_PTR_TIMER_EXPIRED_ADDR                                                             (GSI_REG_BASE      + 0x0000043c)
#define HWIO_GSI_IRAM_PTR_TIMER_EXPIRED_PHYS                                                             (GSI_REG_BASE_PHYS + 0x0000043c)
#define HWIO_GSI_IRAM_PTR_TIMER_EXPIRED_OFFS                                                             (GSI_REG_BASE_OFFS + 0x0000043c)
#define HWIO_GSI_IRAM_PTR_TIMER_EXPIRED_RMSK                                                                  0xfff
#define HWIO_GSI_IRAM_PTR_TIMER_EXPIRED_ATTR                                                                    0x3
#define HWIO_GSI_IRAM_PTR_TIMER_EXPIRED_IN          \
        in_dword_masked(HWIO_GSI_IRAM_PTR_TIMER_EXPIRED_ADDR, HWIO_GSI_IRAM_PTR_TIMER_EXPIRED_RMSK)
#define HWIO_GSI_IRAM_PTR_TIMER_EXPIRED_INM(m)      \
        in_dword_masked(HWIO_GSI_IRAM_PTR_TIMER_EXPIRED_ADDR, m)
#define HWIO_GSI_IRAM_PTR_TIMER_EXPIRED_OUT(v)      \
        out_dword(HWIO_GSI_IRAM_PTR_TIMER_EXPIRED_ADDR,v)
#define HWIO_GSI_IRAM_PTR_TIMER_EXPIRED_OUTM(m,v) \
        out_dword_masked_ns(HWIO_GSI_IRAM_PTR_TIMER_EXPIRED_ADDR,m,v,HWIO_GSI_IRAM_PTR_TIMER_EXPIRED_IN)
#define HWIO_GSI_IRAM_PTR_TIMER_EXPIRED_IRAM_PTR_BMSK                                                         0xfff
#define HWIO_GSI_IRAM_PTR_TIMER_EXPIRED_IRAM_PTR_SHFT                                                           0x0

#define HWIO_GSI_IRAM_PTR_WRITE_ENG_COMP_ADDR                                                            (GSI_REG_BASE      + 0x00000440)
#define HWIO_GSI_IRAM_PTR_WRITE_ENG_COMP_PHYS                                                            (GSI_REG_BASE_PHYS + 0x00000440)
#define HWIO_GSI_IRAM_PTR_WRITE_ENG_COMP_OFFS                                                            (GSI_REG_BASE_OFFS + 0x00000440)
#define HWIO_GSI_IRAM_PTR_WRITE_ENG_COMP_RMSK                                                                 0xfff
#define HWIO_GSI_IRAM_PTR_WRITE_ENG_COMP_ATTR                                                                   0x3
#define HWIO_GSI_IRAM_PTR_WRITE_ENG_COMP_IN          \
        in_dword_masked(HWIO_GSI_IRAM_PTR_WRITE_ENG_COMP_ADDR, HWIO_GSI_IRAM_PTR_WRITE_ENG_COMP_RMSK)
#define HWIO_GSI_IRAM_PTR_WRITE_ENG_COMP_INM(m)      \
        in_dword_masked(HWIO_GSI_IRAM_PTR_WRITE_ENG_COMP_ADDR, m)
#define HWIO_GSI_IRAM_PTR_WRITE_ENG_COMP_OUT(v)      \
        out_dword(HWIO_GSI_IRAM_PTR_WRITE_ENG_COMP_ADDR,v)
#define HWIO_GSI_IRAM_PTR_WRITE_ENG_COMP_OUTM(m,v) \
        out_dword_masked_ns(HWIO_GSI_IRAM_PTR_WRITE_ENG_COMP_ADDR,m,v,HWIO_GSI_IRAM_PTR_WRITE_ENG_COMP_IN)
#define HWIO_GSI_IRAM_PTR_WRITE_ENG_COMP_IRAM_PTR_BMSK                                                        0xfff
#define HWIO_GSI_IRAM_PTR_WRITE_ENG_COMP_IRAM_PTR_SHFT                                                          0x0

#define HWIO_GSI_IRAM_PTR_READ_ENG_COMP_ADDR                                                             (GSI_REG_BASE      + 0x00000444)
#define HWIO_GSI_IRAM_PTR_READ_ENG_COMP_PHYS                                                             (GSI_REG_BASE_PHYS + 0x00000444)
#define HWIO_GSI_IRAM_PTR_READ_ENG_COMP_OFFS                                                             (GSI_REG_BASE_OFFS + 0x00000444)
#define HWIO_GSI_IRAM_PTR_READ_ENG_COMP_RMSK                                                                  0xfff
#define HWIO_GSI_IRAM_PTR_READ_ENG_COMP_ATTR                                                                    0x3
#define HWIO_GSI_IRAM_PTR_READ_ENG_COMP_IN          \
        in_dword_masked(HWIO_GSI_IRAM_PTR_READ_ENG_COMP_ADDR, HWIO_GSI_IRAM_PTR_READ_ENG_COMP_RMSK)
#define HWIO_GSI_IRAM_PTR_READ_ENG_COMP_INM(m)      \
        in_dword_masked(HWIO_GSI_IRAM_PTR_READ_ENG_COMP_ADDR, m)
#define HWIO_GSI_IRAM_PTR_READ_ENG_COMP_OUT(v)      \
        out_dword(HWIO_GSI_IRAM_PTR_READ_ENG_COMP_ADDR,v)
#define HWIO_GSI_IRAM_PTR_READ_ENG_COMP_OUTM(m,v) \
        out_dword_masked_ns(HWIO_GSI_IRAM_PTR_READ_ENG_COMP_ADDR,m,v,HWIO_GSI_IRAM_PTR_READ_ENG_COMP_IN)
#define HWIO_GSI_IRAM_PTR_READ_ENG_COMP_IRAM_PTR_BMSK                                                         0xfff
#define HWIO_GSI_IRAM_PTR_READ_ENG_COMP_IRAM_PTR_SHFT                                                           0x0

#define HWIO_GSI_IRAM_PTR_UC_GP_INT_ADDR                                                                 (GSI_REG_BASE      + 0x00000448)
#define HWIO_GSI_IRAM_PTR_UC_GP_INT_PHYS                                                                 (GSI_REG_BASE_PHYS + 0x00000448)
#define HWIO_GSI_IRAM_PTR_UC_GP_INT_OFFS                                                                 (GSI_REG_BASE_OFFS + 0x00000448)
#define HWIO_GSI_IRAM_PTR_UC_GP_INT_RMSK                                                                      0xfff
#define HWIO_GSI_IRAM_PTR_UC_GP_INT_ATTR                                                                        0x3
#define HWIO_GSI_IRAM_PTR_UC_GP_INT_IN          \
        in_dword_masked(HWIO_GSI_IRAM_PTR_UC_GP_INT_ADDR, HWIO_GSI_IRAM_PTR_UC_GP_INT_RMSK)
#define HWIO_GSI_IRAM_PTR_UC_GP_INT_INM(m)      \
        in_dword_masked(HWIO_GSI_IRAM_PTR_UC_GP_INT_ADDR, m)
#define HWIO_GSI_IRAM_PTR_UC_GP_INT_OUT(v)      \
        out_dword(HWIO_GSI_IRAM_PTR_UC_GP_INT_ADDR,v)
#define HWIO_GSI_IRAM_PTR_UC_GP_INT_OUTM(m,v) \
        out_dword_masked_ns(HWIO_GSI_IRAM_PTR_UC_GP_INT_ADDR,m,v,HWIO_GSI_IRAM_PTR_UC_GP_INT_IN)
#define HWIO_GSI_IRAM_PTR_UC_GP_INT_IRAM_PTR_BMSK                                                             0xfff
#define HWIO_GSI_IRAM_PTR_UC_GP_INT_IRAM_PTR_SHFT                                                               0x0

#define HWIO_GSI_IRAM_PTR_INT_MOD_STOPED_ADDR                                                            (GSI_REG_BASE      + 0x0000044c)
#define HWIO_GSI_IRAM_PTR_INT_MOD_STOPED_PHYS                                                            (GSI_REG_BASE_PHYS + 0x0000044c)
#define HWIO_GSI_IRAM_PTR_INT_MOD_STOPED_OFFS                                                            (GSI_REG_BASE_OFFS + 0x0000044c)
#define HWIO_GSI_IRAM_PTR_INT_MOD_STOPED_RMSK                                                                 0xfff
#define HWIO_GSI_IRAM_PTR_INT_MOD_STOPED_ATTR                                                                   0x3
#define HWIO_GSI_IRAM_PTR_INT_MOD_STOPED_IN          \
        in_dword_masked(HWIO_GSI_IRAM_PTR_INT_MOD_STOPED_ADDR, HWIO_GSI_IRAM_PTR_INT_MOD_STOPED_RMSK)
#define HWIO_GSI_IRAM_PTR_INT_MOD_STOPED_INM(m)      \
        in_dword_masked(HWIO_GSI_IRAM_PTR_INT_MOD_STOPED_ADDR, m)
#define HWIO_GSI_IRAM_PTR_INT_MOD_STOPED_OUT(v)      \
        out_dword(HWIO_GSI_IRAM_PTR_INT_MOD_STOPED_ADDR,v)
#define HWIO_GSI_IRAM_PTR_INT_MOD_STOPED_OUTM(m,v) \
        out_dword_masked_ns(HWIO_GSI_IRAM_PTR_INT_MOD_STOPED_ADDR,m,v,HWIO_GSI_IRAM_PTR_INT_MOD_STOPED_IN)
#define HWIO_GSI_IRAM_PTR_INT_MOD_STOPED_IRAM_PTR_BMSK                                                        0xfff
#define HWIO_GSI_IRAM_PTR_INT_MOD_STOPED_IRAM_PTR_SHFT                                                          0x0

#define HWIO_GSI_IRAM_PTR_INT_NOTIFY_MCS_ADDR                                                            (GSI_REG_BASE      + 0x00000470)
#define HWIO_GSI_IRAM_PTR_INT_NOTIFY_MCS_PHYS                                                            (GSI_REG_BASE_PHYS + 0x00000470)
#define HWIO_GSI_IRAM_PTR_INT_NOTIFY_MCS_OFFS                                                            (GSI_REG_BASE_OFFS + 0x00000470)
#define HWIO_GSI_IRAM_PTR_INT_NOTIFY_MCS_RMSK                                                                 0xfff
#define HWIO_GSI_IRAM_PTR_INT_NOTIFY_MCS_ATTR                                                                   0x3
#define HWIO_GSI_IRAM_PTR_INT_NOTIFY_MCS_IN          \
        in_dword_masked(HWIO_GSI_IRAM_PTR_INT_NOTIFY_MCS_ADDR, HWIO_GSI_IRAM_PTR_INT_NOTIFY_MCS_RMSK)
#define HWIO_GSI_IRAM_PTR_INT_NOTIFY_MCS_INM(m)      \
        in_dword_masked(HWIO_GSI_IRAM_PTR_INT_NOTIFY_MCS_ADDR, m)
#define HWIO_GSI_IRAM_PTR_INT_NOTIFY_MCS_OUT(v)      \
        out_dword(HWIO_GSI_IRAM_PTR_INT_NOTIFY_MCS_ADDR,v)
#define HWIO_GSI_IRAM_PTR_INT_NOTIFY_MCS_OUTM(m,v) \
        out_dword_masked_ns(HWIO_GSI_IRAM_PTR_INT_NOTIFY_MCS_ADDR,m,v,HWIO_GSI_IRAM_PTR_INT_NOTIFY_MCS_IN)
#define HWIO_GSI_IRAM_PTR_INT_NOTIFY_MCS_IRAM_PTR_BMSK                                                        0xfff
#define HWIO_GSI_IRAM_PTR_INT_NOTIFY_MCS_IRAM_PTR_SHFT                                                          0x0

#define HWIO_GSI_INST_RAM_n_ADDR(n)                                                                      (GSI_REG_BASE      + 0x000a4000 + 0x4 * (n))
#define HWIO_GSI_INST_RAM_n_PHYS(n)                                                                      (GSI_REG_BASE_PHYS + 0x000a4000 + 0x4 * (n))
#define HWIO_GSI_INST_RAM_n_OFFS(n)                                                                      (GSI_REG_BASE_OFFS + 0x000a4000 + 0x4 * (n))
#define HWIO_GSI_INST_RAM_n_RMSK                                                                         0xffffffff
#define HWIO_GSI_INST_RAM_n_MAXn                                                                               8255
#define HWIO_GSI_INST_RAM_n_ATTR                                                                                0x3
#define HWIO_GSI_INST_RAM_n_INI(n)        \
        in_dword_masked(HWIO_GSI_INST_RAM_n_ADDR(n), HWIO_GSI_INST_RAM_n_RMSK)
#define HWIO_GSI_INST_RAM_n_INMI(n,mask)    \
        in_dword_masked(HWIO_GSI_INST_RAM_n_ADDR(n), mask)
#define HWIO_GSI_INST_RAM_n_OUTI(n,val)    \
        out_dword(HWIO_GSI_INST_RAM_n_ADDR(n),val)
#define HWIO_GSI_INST_RAM_n_OUTMI(n,mask,val) \
        out_dword_masked_ns(HWIO_GSI_INST_RAM_n_ADDR(n),mask,val,HWIO_GSI_INST_RAM_n_INI(n))
#define HWIO_GSI_INST_RAM_n_INST_BYTE_3_BMSK                                                             0xff000000
#define HWIO_GSI_INST_RAM_n_INST_BYTE_3_SHFT                                                                   0x18
#define HWIO_GSI_INST_RAM_n_INST_BYTE_2_BMSK                                                               0xff0000
#define HWIO_GSI_INST_RAM_n_INST_BYTE_2_SHFT                                                                   0x10
#define HWIO_GSI_INST_RAM_n_INST_BYTE_1_BMSK                                                                 0xff00
#define HWIO_GSI_INST_RAM_n_INST_BYTE_1_SHFT                                                                    0x8
#define HWIO_GSI_INST_RAM_n_INST_BYTE_0_BMSK                                                                   0xff
#define HWIO_GSI_INST_RAM_n_INST_BYTE_0_SHFT                                                                    0x0

#define HWIO_GSI_SHRAM_n_ADDR(n)                                                                         (GSI_REG_BASE      + 0x00002000 + 0x4 * (n))
#define HWIO_GSI_SHRAM_n_PHYS(n)                                                                         (GSI_REG_BASE_PHYS + 0x00002000 + 0x4 * (n))
#define HWIO_GSI_SHRAM_n_OFFS(n)                                                                         (GSI_REG_BASE_OFFS + 0x00002000 + 0x4 * (n))
#define HWIO_GSI_SHRAM_n_RMSK                                                                            0xffffffff
#define HWIO_GSI_SHRAM_n_MAXn                                                                                  2047
#define HWIO_GSI_SHRAM_n_ATTR                                                                                   0x3
#define HWIO_GSI_SHRAM_n_INI(n)        \
        in_dword_masked(HWIO_GSI_SHRAM_n_ADDR(n), HWIO_GSI_SHRAM_n_RMSK, HWIO_GSI_SHRAM_n_ATTR)
#define HWIO_GSI_SHRAM_n_INMI(n,mask)    \
        in_dword_masked(HWIO_GSI_SHRAM_n_ADDR(n), mask, HWIO_GSI_SHRAM_n_ATTR)
#define HWIO_GSI_SHRAM_n_OUTI(n,val)    \
        out_dword(HWIO_GSI_SHRAM_n_ADDR(n),val, HWIO_GSI_SHRAM_n_ATTR)
#define HWIO_GSI_SHRAM_n_OUTMI(n,mask,val) \
        out_dword_masked_ns(HWIO_GSI_SHRAM_n_ADDR(n),mask,val,HWIO_GSI_SHRAM_n_INI(n))
#define HWIO_GSI_SHRAM_n_SHRAM_BMSK                                                                      0xffffffff
#define HWIO_GSI_SHRAM_n_SHRAM_SHFT                                                                             0x0

#define HWIO_GSI_MAP_EE_n_CH_k_VP_TABLE_ADDR(n,k)                                                        (GSI_REG_BASE      + 0x00009000 + 0x400 * (n) + 0x4 * (k))
#define HWIO_GSI_MAP_EE_n_CH_k_VP_TABLE_PHYS(n,k)                                                        (GSI_REG_BASE_PHYS + 0x00009000 + 0x400 * (n) + 0x4 * (k))
#define HWIO_GSI_MAP_EE_n_CH_k_VP_TABLE_OFFS(n,k)                                                        (GSI_REG_BASE_OFFS + 0x00009000 + 0x400 * (n) + 0x4 * (k))
#define HWIO_GSI_MAP_EE_n_CH_k_VP_TABLE_RMSK                                                                  0x1ff
#define HWIO_GSI_MAP_EE_n_CH_k_VP_TABLE_MAXn                                                                      2
#define HWIO_GSI_MAP_EE_n_CH_k_VP_TABLE_MAXk                                                                     27
#define HWIO_GSI_MAP_EE_n_CH_k_VP_TABLE_ATTR                                                                    0x3
#define HWIO_GSI_MAP_EE_n_CH_k_VP_TABLE_INI2(n,k)        \
        in_dword_masked(HWIO_GSI_MAP_EE_n_CH_k_VP_TABLE_ADDR(n,k), HWIO_GSI_MAP_EE_n_CH_k_VP_TABLE_RMSK)
#define HWIO_GSI_MAP_EE_n_CH_k_VP_TABLE_INMI2(n,k,mask)    \
        in_dword_masked(HWIO_GSI_MAP_EE_n_CH_k_VP_TABLE_ADDR(n,k), mask)
#define HWIO_GSI_MAP_EE_n_CH_k_VP_TABLE_OUTI2(n,k,val)    \
        out_dword(HWIO_GSI_MAP_EE_n_CH_k_VP_TABLE_ADDR(n,k),val)
#define HWIO_GSI_MAP_EE_n_CH_k_VP_TABLE_OUTMI2(n,k,mask,val) \
        out_dword_masked_ns(HWIO_GSI_MAP_EE_n_CH_k_VP_TABLE_ADDR(n,k),mask,val,HWIO_GSI_MAP_EE_n_CH_k_VP_TABLE_INI2(n,k))
#define HWIO_GSI_MAP_EE_n_CH_k_VP_TABLE_VALID_BMSK                                                            0x100
#define HWIO_GSI_MAP_EE_n_CH_k_VP_TABLE_VALID_SHFT                                                              0x8
#define HWIO_GSI_MAP_EE_n_CH_k_VP_TABLE_PHY_CH_BMSK                                                            0xff
#define HWIO_GSI_MAP_EE_n_CH_k_VP_TABLE_PHY_CH_SHFT                                                             0x0

#define HWIO_GSI_TEST_BUS_SEL_ADDR                                                                       (GSI_REG_BASE      + 0x00001000)
#define HWIO_GSI_TEST_BUS_SEL_PHYS                                                                       (GSI_REG_BASE_PHYS + 0x00001000)
#define HWIO_GSI_TEST_BUS_SEL_OFFS                                                                       (GSI_REG_BASE_OFFS + 0x00001000)
#define HWIO_GSI_TEST_BUS_SEL_RMSK                                                                          0xf00ff
#define HWIO_GSI_TEST_BUS_SEL_ATTR                                                                              0x3
#define HWIO_GSI_TEST_BUS_SEL_IN          \
        in_dword_masked(HWIO_GSI_TEST_BUS_SEL_ADDR, HWIO_GSI_TEST_BUS_SEL_RMSK, HWIO_GSI_TEST_BUS_SEL_ATTR)
#define HWIO_GSI_TEST_BUS_SEL_INM(m)      \
        in_dword_masked(HWIO_GSI_TEST_BUS_SEL_ADDR, m, HWIO_GSI_TEST_BUS_SEL_ATTR)
#define HWIO_GSI_TEST_BUS_SEL_OUT(v)      \
        out_dword(HWIO_GSI_TEST_BUS_SEL_ADDR,v, HWIO_GSI_TEST_BUS_SEL_ATTR)
#define HWIO_GSI_TEST_BUS_SEL_OUTM(m,v) \
        out_dword_masked_ns(HWIO_GSI_TEST_BUS_SEL_ADDR,m,v,HWIO_GSI_TEST_BUS_SEL_IN)
#define HWIO_GSI_TEST_BUS_SEL_GSI_HW_EVENTS_SEL_BMSK                                                        0xf0000
#define HWIO_GSI_TEST_BUS_SEL_GSI_HW_EVENTS_SEL_SHFT                                                           0x10
#define HWIO_GSI_TEST_BUS_SEL_GSI_TESTBUS_SEL_BMSK                                                             0xff
#define HWIO_GSI_TEST_BUS_SEL_GSI_TESTBUS_SEL_SHFT                                                              0x0
#define HWIO_GSI_TEST_BUS_SEL_GSI_TESTBUS_SEL_ZEROS_FVAL                                                        0x0
#define HWIO_GSI_TEST_BUS_SEL_GSI_TESTBUS_SEL_MCS_0_FVAL                                                        0x1
#define HWIO_GSI_TEST_BUS_SEL_GSI_TESTBUS_SEL_MCS_1_FVAL                                                        0x2
#define HWIO_GSI_TEST_BUS_SEL_GSI_TESTBUS_SEL_MCS_2_FVAL                                                        0x3
#define HWIO_GSI_TEST_BUS_SEL_GSI_TESTBUS_SEL_MCS_3_FVAL                                                        0x4
#define HWIO_GSI_TEST_BUS_SEL_GSI_TESTBUS_SEL_MCS_4_FVAL                                                        0x5
#define HWIO_GSI_TEST_BUS_SEL_GSI_TESTBUS_SEL_DB_ENG_FVAL                                                       0x9
#define HWIO_GSI_TEST_BUS_SEL_GSI_TESTBUS_SEL_REE_0_FVAL                                                        0xb
#define HWIO_GSI_TEST_BUS_SEL_GSI_TESTBUS_SEL_REE_1_FVAL                                                        0xc
#define HWIO_GSI_TEST_BUS_SEL_GSI_TESTBUS_SEL_REE_2_FVAL                                                        0xd
#define HWIO_GSI_TEST_BUS_SEL_GSI_TESTBUS_SEL_REE_3_FVAL                                                        0xe
#define HWIO_GSI_TEST_BUS_SEL_GSI_TESTBUS_SEL_REE_4_FVAL                                                        0xf
#define HWIO_GSI_TEST_BUS_SEL_GSI_TESTBUS_SEL_REE_5_FVAL                                                       0x10
#define HWIO_GSI_TEST_BUS_SEL_GSI_TESTBUS_SEL_REE_6_FVAL                                                       0x11
#define HWIO_GSI_TEST_BUS_SEL_GSI_TESTBUS_SEL_REE_7_FVAL                                                       0x12
#define HWIO_GSI_TEST_BUS_SEL_GSI_TESTBUS_SEL_EVE_0_FVAL                                                       0x13
#define HWIO_GSI_TEST_BUS_SEL_GSI_TESTBUS_SEL_EVE_1_FVAL                                                       0x14
#define HWIO_GSI_TEST_BUS_SEL_GSI_TESTBUS_SEL_EVE_2_FVAL                                                       0x15
#define HWIO_GSI_TEST_BUS_SEL_GSI_TESTBUS_SEL_EVE_3_FVAL                                                       0x16
#define HWIO_GSI_TEST_BUS_SEL_GSI_TESTBUS_SEL_EVE_4_FVAL                                                       0x17
#define HWIO_GSI_TEST_BUS_SEL_GSI_TESTBUS_SEL_EVE_5_FVAL                                                       0x18
#define HWIO_GSI_TEST_BUS_SEL_GSI_TESTBUS_SEL_IE_0_FVAL                                                        0x1b
#define HWIO_GSI_TEST_BUS_SEL_GSI_TESTBUS_SEL_IE_1_FVAL                                                        0x1c
#define HWIO_GSI_TEST_BUS_SEL_GSI_TESTBUS_SEL_IE_2_FVAL                                                        0x1d
#define HWIO_GSI_TEST_BUS_SEL_GSI_TESTBUS_SEL_IC_0_FVAL                                                        0x1f
#define HWIO_GSI_TEST_BUS_SEL_GSI_TESTBUS_SEL_IC_1_FVAL                                                        0x20
#define HWIO_GSI_TEST_BUS_SEL_GSI_TESTBUS_SEL_IC_2_FVAL                                                        0x21
#define HWIO_GSI_TEST_BUS_SEL_GSI_TESTBUS_SEL_IC_3_FVAL                                                        0x22
#define HWIO_GSI_TEST_BUS_SEL_GSI_TESTBUS_SEL_IC_4_FVAL                                                        0x23
#define HWIO_GSI_TEST_BUS_SEL_GSI_TESTBUS_SEL_MOQA_0_FVAL                                                      0x27
#define HWIO_GSI_TEST_BUS_SEL_GSI_TESTBUS_SEL_MOQA_1_FVAL                                                      0x28
#define HWIO_GSI_TEST_BUS_SEL_GSI_TESTBUS_SEL_MOQA_2_FVAL                                                      0x29
#define HWIO_GSI_TEST_BUS_SEL_GSI_TESTBUS_SEL_MOQA_3_FVAL                                                      0x2a
#define HWIO_GSI_TEST_BUS_SEL_GSI_TESTBUS_SEL_TMR_0_FVAL                                                       0x2b
#define HWIO_GSI_TEST_BUS_SEL_GSI_TESTBUS_SEL_TMR_1_FVAL                                                       0x2c
#define HWIO_GSI_TEST_BUS_SEL_GSI_TESTBUS_SEL_TMR_2_FVAL                                                       0x2d
#define HWIO_GSI_TEST_BUS_SEL_GSI_TESTBUS_SEL_TMR_3_FVAL                                                       0x2e
#define HWIO_GSI_TEST_BUS_SEL_GSI_TESTBUS_SEL_RD_WR_0_FVAL                                                     0x33
#define HWIO_GSI_TEST_BUS_SEL_GSI_TESTBUS_SEL_RD_WR_1_FVAL                                                     0x34
#define HWIO_GSI_TEST_BUS_SEL_GSI_TESTBUS_SEL_RD_WR_2_FVAL                                                     0x35
#define HWIO_GSI_TEST_BUS_SEL_GSI_TESTBUS_SEL_RD_WR_3_FVAL                                                     0x36
#define HWIO_GSI_TEST_BUS_SEL_GSI_TESTBUS_SEL_CSR_FVAL                                                         0x3a
#define HWIO_GSI_TEST_BUS_SEL_GSI_TESTBUS_SEL_SDMA_0_FVAL                                                      0x3c
#define HWIO_GSI_TEST_BUS_SEL_GSI_TESTBUS_SEL_SMDA_1_FVAL                                                      0x3d
#define HWIO_GSI_TEST_BUS_SEL_GSI_TESTBUS_SEL_CSR_1_FVAL                                                       0x3e
#define HWIO_GSI_TEST_BUS_SEL_GSI_TESTBUS_SEL_CSR_2_FVAL                                                       0x3f
#define HWIO_GSI_TEST_BUS_SEL_GSI_TESTBUS_SEL_MCS_5_FVAL                                                       0x40
#define HWIO_GSI_TEST_BUS_SEL_GSI_TESTBUS_SEL_IC_5_FVAL                                                        0x41
#define HWIO_GSI_TEST_BUS_SEL_GSI_TESTBUS_SEL_CSR_3_FVAL                                                       0x42
#define HWIO_GSI_TEST_BUS_SEL_GSI_TESTBUS_SEL_TLV_0_FVAL                                                       0x43
#define HWIO_GSI_TEST_BUS_SEL_GSI_TESTBUS_SEL_REE_8_FVAL                                                       0x44
#define HWIO_GSI_TEST_BUS_SEL_GSI_TESTBUS_SEL_IE_NOTIFY_FVAL                                                   0x45
#define HWIO_GSI_TEST_BUS_SEL_GSI_TESTBUS_SEL_DB_MSI_FVAL                                                      0x46
#define HWIO_GSI_TEST_BUS_SEL_GSI_TESTBUS_SEL_REE_9_FVAL                                                       0x47

#define HWIO_GSI_TEST_BUS_REG_ADDR                                                                       (GSI_REG_BASE      + 0x00001008)
#define HWIO_GSI_TEST_BUS_REG_PHYS                                                                       (GSI_REG_BASE_PHYS + 0x00001008)
#define HWIO_GSI_TEST_BUS_REG_OFFS                                                                       (GSI_REG_BASE_OFFS + 0x00001008)
#define HWIO_GSI_TEST_BUS_REG_RMSK                                                                       0xffffffff
#define HWIO_GSI_TEST_BUS_REG_ATTR                                                                              0x1
#define HWIO_GSI_TEST_BUS_REG_IN          \
        in_dword_masked(HWIO_GSI_TEST_BUS_REG_ADDR, HWIO_GSI_TEST_BUS_REG_RMSK, HWIO_GSI_TEST_BUS_REG_ATTR)
#define HWIO_GSI_TEST_BUS_REG_INM(m)      \
        in_dword_masked(HWIO_GSI_TEST_BUS_REG_ADDR, m, HWIO_GSI_TEST_BUS_REG_ATTR)
#define HWIO_GSI_TEST_BUS_REG_GSI_TESTBUS_REG_BMSK                                                       0xffffffff
#define HWIO_GSI_TEST_BUS_REG_GSI_TESTBUS_REG_SHFT                                                              0x0

#define HWIO_GSI_DEBUG_BUSY_REG_ADDR                                                                     (GSI_REG_BASE      + 0x00001010)
#define HWIO_GSI_DEBUG_BUSY_REG_PHYS                                                                     (GSI_REG_BASE_PHYS + 0x00001010)
#define HWIO_GSI_DEBUG_BUSY_REG_OFFS                                                                     (GSI_REG_BASE_OFFS + 0x00001010)
#define HWIO_GSI_DEBUG_BUSY_REG_RMSK                                                                         0x1fff
#define HWIO_GSI_DEBUG_BUSY_REG_ATTR                                                                            0x1
#define HWIO_GSI_DEBUG_BUSY_REG_IN          \
        in_dword_masked(HWIO_GSI_DEBUG_BUSY_REG_ADDR, HWIO_GSI_DEBUG_BUSY_REG_RMSK)
#define HWIO_GSI_DEBUG_BUSY_REG_INM(m)      \
        in_dword_masked(HWIO_GSI_DEBUG_BUSY_REG_ADDR, m)
#define HWIO_GSI_DEBUG_BUSY_REG_SDMA_BUSY_BMSK                                                               0x1000
#define HWIO_GSI_DEBUG_BUSY_REG_SDMA_BUSY_SHFT                                                                  0xc
#define HWIO_GSI_DEBUG_BUSY_REG_IC_BUSY_BMSK                                                                  0x800
#define HWIO_GSI_DEBUG_BUSY_REG_IC_BUSY_SHFT                                                                    0xb
#define HWIO_GSI_DEBUG_BUSY_REG_UC_BUSY_BMSK                                                                  0x400
#define HWIO_GSI_DEBUG_BUSY_REG_UC_BUSY_SHFT                                                                    0xa
#define HWIO_GSI_DEBUG_BUSY_REG_DBG_CNT_BUSY_BMSK                                                             0x200
#define HWIO_GSI_DEBUG_BUSY_REG_DBG_CNT_BUSY_SHFT                                                               0x9
#define HWIO_GSI_DEBUG_BUSY_REG_DB_ENG_BUSY_BMSK                                                              0x100
#define HWIO_GSI_DEBUG_BUSY_REG_DB_ENG_BUSY_SHFT                                                                0x8
#define HWIO_GSI_DEBUG_BUSY_REG_REE_PWR_CLPS_BUSY_BMSK                                                         0x80
#define HWIO_GSI_DEBUG_BUSY_REG_REE_PWR_CLPS_BUSY_SHFT                                                          0x7
#define HWIO_GSI_DEBUG_BUSY_REG_INT_ENG_BUSY_BMSK                                                              0x40
#define HWIO_GSI_DEBUG_BUSY_REG_INT_ENG_BUSY_SHFT                                                               0x6
#define HWIO_GSI_DEBUG_BUSY_REG_EV_ENG_BUSY_BMSK                                                               0x20
#define HWIO_GSI_DEBUG_BUSY_REG_EV_ENG_BUSY_SHFT                                                                0x5
#define HWIO_GSI_DEBUG_BUSY_REG_RD_WR_BUSY_BMSK                                                                0x10
#define HWIO_GSI_DEBUG_BUSY_REG_RD_WR_BUSY_SHFT                                                                 0x4
#define HWIO_GSI_DEBUG_BUSY_REG_TIMER_BUSY_BMSK                                                                 0x8
#define HWIO_GSI_DEBUG_BUSY_REG_TIMER_BUSY_SHFT                                                                 0x3
#define HWIO_GSI_DEBUG_BUSY_REG_MCS_BUSY_BMSK                                                                   0x4
#define HWIO_GSI_DEBUG_BUSY_REG_MCS_BUSY_SHFT                                                                   0x2
#define HWIO_GSI_DEBUG_BUSY_REG_REE_BUSY_BMSK                                                                   0x2
#define HWIO_GSI_DEBUG_BUSY_REG_REE_BUSY_SHFT                                                                   0x1
#define HWIO_GSI_DEBUG_BUSY_REG_CSR_BUSY_BMSK                                                                   0x1
#define HWIO_GSI_DEBUG_BUSY_REG_CSR_BUSY_SHFT                                                                   0x0

#define HWIO_GSI_DEBUG_EVENT_PENDING_k_ADDR(k)                                                           (GSI_REG_BASE      + 0x00001a80 + 0x4 * (k))
#define HWIO_GSI_DEBUG_EVENT_PENDING_k_PHYS(k)                                                           (GSI_REG_BASE_PHYS + 0x00001a80 + 0x4 * (k))
#define HWIO_GSI_DEBUG_EVENT_PENDING_k_OFFS(k)                                                           (GSI_REG_BASE_OFFS + 0x00001a80 + 0x4 * (k))
#define HWIO_GSI_DEBUG_EVENT_PENDING_k_RMSK                                                              0xffffffff
#define HWIO_GSI_DEBUG_EVENT_PENDING_k_MAXk                                                                       1
#define HWIO_GSI_DEBUG_EVENT_PENDING_k_ATTR                                                                     0x1
#define HWIO_GSI_DEBUG_EVENT_PENDING_k_INI(k)        \
        in_dword_masked(HWIO_GSI_DEBUG_EVENT_PENDING_k_ADDR(k), HWIO_GSI_DEBUG_EVENT_PENDING_k_RMSK)
#define HWIO_GSI_DEBUG_EVENT_PENDING_k_INMI(k,mask)    \
        in_dword_masked(HWIO_GSI_DEBUG_EVENT_PENDING_k_ADDR(k), mask)
#define HWIO_GSI_DEBUG_EVENT_PENDING_k_CHID_BIT_MAP_BMSK                                                 0xffffffff
#define HWIO_GSI_DEBUG_EVENT_PENDING_k_CHID_BIT_MAP_SHFT                                                        0x0

#define HWIO_GSI_DEBUG_TIMER_PENDING_k_ADDR(k)                                                           (GSI_REG_BASE      + 0x00001aa0 + 0x4 * (k))
#define HWIO_GSI_DEBUG_TIMER_PENDING_k_PHYS(k)                                                           (GSI_REG_BASE_PHYS + 0x00001aa0 + 0x4 * (k))
#define HWIO_GSI_DEBUG_TIMER_PENDING_k_OFFS(k)                                                           (GSI_REG_BASE_OFFS + 0x00001aa0 + 0x4 * (k))
#define HWIO_GSI_DEBUG_TIMER_PENDING_k_RMSK                                                              0xffffffff
#define HWIO_GSI_DEBUG_TIMER_PENDING_k_MAXk                                                                       1
#define HWIO_GSI_DEBUG_TIMER_PENDING_k_ATTR                                                                     0x1
#define HWIO_GSI_DEBUG_TIMER_PENDING_k_INI(k)        \
        in_dword_masked(HWIO_GSI_DEBUG_TIMER_PENDING_k_ADDR(k), HWIO_GSI_DEBUG_TIMER_PENDING_k_RMSK)
#define HWIO_GSI_DEBUG_TIMER_PENDING_k_INMI(k,mask)    \
        in_dword_masked(HWIO_GSI_DEBUG_TIMER_PENDING_k_ADDR(k), mask)
#define HWIO_GSI_DEBUG_TIMER_PENDING_k_CHID_BIT_MAP_BMSK                                                 0xffffffff
#define HWIO_GSI_DEBUG_TIMER_PENDING_k_CHID_BIT_MAP_SHFT                                                        0x0

#define HWIO_GSI_DEBUG_RD_WR_PENDING_k_ADDR(k)                                                           (GSI_REG_BASE      + 0x00001ac0 + 0x4 * (k))
#define HWIO_GSI_DEBUG_RD_WR_PENDING_k_PHYS(k)                                                           (GSI_REG_BASE_PHYS + 0x00001ac0 + 0x4 * (k))
#define HWIO_GSI_DEBUG_RD_WR_PENDING_k_OFFS(k)                                                           (GSI_REG_BASE_OFFS + 0x00001ac0 + 0x4 * (k))
#define HWIO_GSI_DEBUG_RD_WR_PENDING_k_RMSK                                                              0xffffffff
#define HWIO_GSI_DEBUG_RD_WR_PENDING_k_MAXk                                                                       1
#define HWIO_GSI_DEBUG_RD_WR_PENDING_k_ATTR                                                                     0x1
#define HWIO_GSI_DEBUG_RD_WR_PENDING_k_INI(k)        \
        in_dword_masked(HWIO_GSI_DEBUG_RD_WR_PENDING_k_ADDR(k), HWIO_GSI_DEBUG_RD_WR_PENDING_k_RMSK)
#define HWIO_GSI_DEBUG_RD_WR_PENDING_k_INMI(k,mask)    \
        in_dword_masked(HWIO_GSI_DEBUG_RD_WR_PENDING_k_ADDR(k), mask)
#define HWIO_GSI_DEBUG_RD_WR_PENDING_k_CHID_BIT_MAP_BMSK                                                 0xffffffff
#define HWIO_GSI_DEBUG_RD_WR_PENDING_k_CHID_BIT_MAP_SHFT                                                        0x0

#define HWIO_GSI_SPARE_REG_1_ADDR                                                                        (GSI_REG_BASE      + 0x00001030)
#define HWIO_GSI_SPARE_REG_1_PHYS                                                                        (GSI_REG_BASE_PHYS + 0x00001030)
#define HWIO_GSI_SPARE_REG_1_OFFS                                                                        (GSI_REG_BASE_OFFS + 0x00001030)
#define HWIO_GSI_SPARE_REG_1_RMSK                                                                               0x1
#define HWIO_GSI_SPARE_REG_1_ATTR                                                                               0x3
#define HWIO_GSI_SPARE_REG_1_IN          \
        in_dword_masked(HWIO_GSI_SPARE_REG_1_ADDR, HWIO_GSI_SPARE_REG_1_RMSK)
#define HWIO_GSI_SPARE_REG_1_INM(m)      \
        in_dword_masked(HWIO_GSI_SPARE_REG_1_ADDR, m)
#define HWIO_GSI_SPARE_REG_1_OUT(v)      \
        out_dword(HWIO_GSI_SPARE_REG_1_ADDR,v)
#define HWIO_GSI_SPARE_REG_1_OUTM(m,v) \
        out_dword_masked_ns(HWIO_GSI_SPARE_REG_1_ADDR,m,v,HWIO_GSI_SPARE_REG_1_IN)
#define HWIO_GSI_SPARE_REG_1_FIX_IEOB_WRONG_MSK_DISABLE_BMSK                                                    0x1
#define HWIO_GSI_SPARE_REG_1_FIX_IEOB_WRONG_MSK_DISABLE_SHFT                                                    0x0

#define HWIO_GSI_DEBUG_PC_FROM_SW_ADDR                                                                   (GSI_REG_BASE      + 0x00001040)
#define HWIO_GSI_DEBUG_PC_FROM_SW_PHYS                                                                   (GSI_REG_BASE_PHYS + 0x00001040)
#define HWIO_GSI_DEBUG_PC_FROM_SW_OFFS                                                                   (GSI_REG_BASE_OFFS + 0x00001040)
#define HWIO_GSI_DEBUG_PC_FROM_SW_RMSK                                                                        0xfff
#define HWIO_GSI_DEBUG_PC_FROM_SW_ATTR                                                                          0x3
#define HWIO_GSI_DEBUG_PC_FROM_SW_IN          \
        in_dword_masked(HWIO_GSI_DEBUG_PC_FROM_SW_ADDR, HWIO_GSI_DEBUG_PC_FROM_SW_RMSK)
#define HWIO_GSI_DEBUG_PC_FROM_SW_INM(m)      \
        in_dword_masked(HWIO_GSI_DEBUG_PC_FROM_SW_ADDR, m)
#define HWIO_GSI_DEBUG_PC_FROM_SW_OUT(v)      \
        out_dword(HWIO_GSI_DEBUG_PC_FROM_SW_ADDR,v)
#define HWIO_GSI_DEBUG_PC_FROM_SW_OUTM(m,v) \
        out_dword_masked_ns(HWIO_GSI_DEBUG_PC_FROM_SW_ADDR,m,v,HWIO_GSI_DEBUG_PC_FROM_SW_IN)
#define HWIO_GSI_DEBUG_PC_FROM_SW_IRAM_PTR_BMSK                                                               0xfff
#define HWIO_GSI_DEBUG_PC_FROM_SW_IRAM_PTR_SHFT                                                                 0x0

#define HWIO_GSI_DEBUG_SW_STALL_ADDR                                                                     (GSI_REG_BASE      + 0x00001044)
#define HWIO_GSI_DEBUG_SW_STALL_PHYS                                                                     (GSI_REG_BASE_PHYS + 0x00001044)
#define HWIO_GSI_DEBUG_SW_STALL_OFFS                                                                     (GSI_REG_BASE_OFFS + 0x00001044)
#define HWIO_GSI_DEBUG_SW_STALL_RMSK                                                                            0x1
#define HWIO_GSI_DEBUG_SW_STALL_ATTR                                                                            0x3
#define HWIO_GSI_DEBUG_SW_STALL_IN          \
        in_dword_masked(HWIO_GSI_DEBUG_SW_STALL_ADDR, HWIO_GSI_DEBUG_SW_STALL_RMSK)
#define HWIO_GSI_DEBUG_SW_STALL_INM(m)      \
        in_dword_masked(HWIO_GSI_DEBUG_SW_STALL_ADDR, m)
#define HWIO_GSI_DEBUG_SW_STALL_OUT(v)      \
        out_dword(HWIO_GSI_DEBUG_SW_STALL_ADDR,v)
#define HWIO_GSI_DEBUG_SW_STALL_OUTM(m,v) \
        out_dword_masked_ns(HWIO_GSI_DEBUG_SW_STALL_ADDR,m,v,HWIO_GSI_DEBUG_SW_STALL_IN)
#define HWIO_GSI_DEBUG_SW_STALL_MCS_STALL_BMSK                                                                  0x1
#define HWIO_GSI_DEBUG_SW_STALL_MCS_STALL_SHFT                                                                  0x0

#define HWIO_GSI_DEBUG_PC_FOR_DEBUG_ADDR                                                                 (GSI_REG_BASE      + 0x00001048)
#define HWIO_GSI_DEBUG_PC_FOR_DEBUG_PHYS                                                                 (GSI_REG_BASE_PHYS + 0x00001048)
#define HWIO_GSI_DEBUG_PC_FOR_DEBUG_OFFS                                                                 (GSI_REG_BASE_OFFS + 0x00001048)
#define HWIO_GSI_DEBUG_PC_FOR_DEBUG_RMSK                                                                      0xfff
#define HWIO_GSI_DEBUG_PC_FOR_DEBUG_ATTR                                                                        0x1
#define HWIO_GSI_DEBUG_PC_FOR_DEBUG_IN          \
        in_dword_masked(HWIO_GSI_DEBUG_PC_FOR_DEBUG_ADDR, HWIO_GSI_DEBUG_PC_FOR_DEBUG_RMSK)
#define HWIO_GSI_DEBUG_PC_FOR_DEBUG_INM(m)      \
        in_dword_masked(HWIO_GSI_DEBUG_PC_FOR_DEBUG_ADDR, m)
#define HWIO_GSI_DEBUG_PC_FOR_DEBUG_IRAM_PTR_BMSK                                                             0xfff
#define HWIO_GSI_DEBUG_PC_FOR_DEBUG_IRAM_PTR_SHFT                                                               0x0

#define HWIO_GSI_DEBUG_QSB_LOG_SEL_ADDR                                                                  (GSI_REG_BASE      + 0x00001050)
#define HWIO_GSI_DEBUG_QSB_LOG_SEL_PHYS                                                                  (GSI_REG_BASE_PHYS + 0x00001050)
#define HWIO_GSI_DEBUG_QSB_LOG_SEL_OFFS                                                                  (GSI_REG_BASE_OFFS + 0x00001050)
#define HWIO_GSI_DEBUG_QSB_LOG_SEL_RMSK                                                                    0xffff01
#define HWIO_GSI_DEBUG_QSB_LOG_SEL_ATTR                                                                         0x3
#define HWIO_GSI_DEBUG_QSB_LOG_SEL_IN          \
        in_dword_masked(HWIO_GSI_DEBUG_QSB_LOG_SEL_ADDR, HWIO_GSI_DEBUG_QSB_LOG_SEL_RMSK)
#define HWIO_GSI_DEBUG_QSB_LOG_SEL_INM(m)      \
        in_dword_masked(HWIO_GSI_DEBUG_QSB_LOG_SEL_ADDR, m)
#define HWIO_GSI_DEBUG_QSB_LOG_SEL_OUT(v)      \
        out_dword(HWIO_GSI_DEBUG_QSB_LOG_SEL_ADDR,v)
#define HWIO_GSI_DEBUG_QSB_LOG_SEL_OUTM(m,v) \
        out_dword_masked_ns(HWIO_GSI_DEBUG_QSB_LOG_SEL_ADDR,m,v,HWIO_GSI_DEBUG_QSB_LOG_SEL_IN)
#define HWIO_GSI_DEBUG_QSB_LOG_SEL_SEL_MID_BMSK                                                            0xff0000
#define HWIO_GSI_DEBUG_QSB_LOG_SEL_SEL_MID_SHFT                                                                0x10
#define HWIO_GSI_DEBUG_QSB_LOG_SEL_SEL_TID_BMSK                                                              0xff00
#define HWIO_GSI_DEBUG_QSB_LOG_SEL_SEL_TID_SHFT                                                                 0x8
#define HWIO_GSI_DEBUG_QSB_LOG_SEL_SEL_WRITE_BMSK                                                               0x1
#define HWIO_GSI_DEBUG_QSB_LOG_SEL_SEL_WRITE_SHFT                                                               0x0

#define HWIO_GSI_DEBUG_QSB_LOG_CLR_ADDR                                                                  (GSI_REG_BASE      + 0x00001058)
#define HWIO_GSI_DEBUG_QSB_LOG_CLR_PHYS                                                                  (GSI_REG_BASE_PHYS + 0x00001058)
#define HWIO_GSI_DEBUG_QSB_LOG_CLR_OFFS                                                                  (GSI_REG_BASE_OFFS + 0x00001058)
#define HWIO_GSI_DEBUG_QSB_LOG_CLR_RMSK                                                                         0x1
#define HWIO_GSI_DEBUG_QSB_LOG_CLR_ATTR                                                                         0x2
#define HWIO_GSI_DEBUG_QSB_LOG_CLR_OUT(v)      \
        out_dword(HWIO_GSI_DEBUG_QSB_LOG_CLR_ADDR,v)
#define HWIO_GSI_DEBUG_QSB_LOG_CLR_LOG_CLR_BMSK                                                                 0x1
#define HWIO_GSI_DEBUG_QSB_LOG_CLR_LOG_CLR_SHFT                                                                 0x0

#define HWIO_GSI_DEBUG_QSB_LOG_ERR_TRNS_ID_ADDR                                                          (GSI_REG_BASE      + 0x00001060)
#define HWIO_GSI_DEBUG_QSB_LOG_ERR_TRNS_ID_PHYS                                                          (GSI_REG_BASE_PHYS + 0x00001060)
#define HWIO_GSI_DEBUG_QSB_LOG_ERR_TRNS_ID_OFFS                                                          (GSI_REG_BASE_OFFS + 0x00001060)
#define HWIO_GSI_DEBUG_QSB_LOG_ERR_TRNS_ID_RMSK                                                           0x1ffff01
#define HWIO_GSI_DEBUG_QSB_LOG_ERR_TRNS_ID_ATTR                                                                 0x1
#define HWIO_GSI_DEBUG_QSB_LOG_ERR_TRNS_ID_IN          \
        in_dword_masked(HWIO_GSI_DEBUG_QSB_LOG_ERR_TRNS_ID_ADDR, HWIO_GSI_DEBUG_QSB_LOG_ERR_TRNS_ID_RMSK)
#define HWIO_GSI_DEBUG_QSB_LOG_ERR_TRNS_ID_INM(m)      \
        in_dword_masked(HWIO_GSI_DEBUG_QSB_LOG_ERR_TRNS_ID_ADDR, m)
#define HWIO_GSI_DEBUG_QSB_LOG_ERR_TRNS_ID_ERR_SAVED_BMSK                                                 0x1000000
#define HWIO_GSI_DEBUG_QSB_LOG_ERR_TRNS_ID_ERR_SAVED_SHFT                                                      0x18
#define HWIO_GSI_DEBUG_QSB_LOG_ERR_TRNS_ID_ERR_MID_BMSK                                                    0xff0000
#define HWIO_GSI_DEBUG_QSB_LOG_ERR_TRNS_ID_ERR_MID_SHFT                                                        0x10
#define HWIO_GSI_DEBUG_QSB_LOG_ERR_TRNS_ID_ERR_TID_BMSK                                                      0xff00
#define HWIO_GSI_DEBUG_QSB_LOG_ERR_TRNS_ID_ERR_TID_SHFT                                                         0x8
#define HWIO_GSI_DEBUG_QSB_LOG_ERR_TRNS_ID_ERR_WRITE_BMSK                                                       0x1
#define HWIO_GSI_DEBUG_QSB_LOG_ERR_TRNS_ID_ERR_WRITE_SHFT                                                       0x0

#define HWIO_GSI_DEBUG_QSB_LOG_0_ADDR                                                                    (GSI_REG_BASE      + 0x00001064)
#define HWIO_GSI_DEBUG_QSB_LOG_0_PHYS                                                                    (GSI_REG_BASE_PHYS + 0x00001064)
#define HWIO_GSI_DEBUG_QSB_LOG_0_OFFS                                                                    (GSI_REG_BASE_OFFS + 0x00001064)
#define HWIO_GSI_DEBUG_QSB_LOG_0_RMSK                                                                    0xffffffff
#define HWIO_GSI_DEBUG_QSB_LOG_0_ATTR                                                                           0x1
#define HWIO_GSI_DEBUG_QSB_LOG_0_IN          \
        in_dword_masked(HWIO_GSI_DEBUG_QSB_LOG_0_ADDR, HWIO_GSI_DEBUG_QSB_LOG_0_RMSK)
#define HWIO_GSI_DEBUG_QSB_LOG_0_INM(m)      \
        in_dword_masked(HWIO_GSI_DEBUG_QSB_LOG_0_ADDR, m)
#define HWIO_GSI_DEBUG_QSB_LOG_0_ADDR_31_0_BMSK                                                          0xffffffff
#define HWIO_GSI_DEBUG_QSB_LOG_0_ADDR_31_0_SHFT                                                                 0x0

#define HWIO_GSI_DEBUG_QSB_LOG_1_ADDR                                                                    (GSI_REG_BASE      + 0x00001068)
#define HWIO_GSI_DEBUG_QSB_LOG_1_PHYS                                                                    (GSI_REG_BASE_PHYS + 0x00001068)
#define HWIO_GSI_DEBUG_QSB_LOG_1_OFFS                                                                    (GSI_REG_BASE_OFFS + 0x00001068)
#define HWIO_GSI_DEBUG_QSB_LOG_1_RMSK                                                                    0xfff7ffff
#define HWIO_GSI_DEBUG_QSB_LOG_1_ATTR                                                                           0x1
#define HWIO_GSI_DEBUG_QSB_LOG_1_IN          \
        in_dword_masked(HWIO_GSI_DEBUG_QSB_LOG_1_ADDR, HWIO_GSI_DEBUG_QSB_LOG_1_RMSK)
#define HWIO_GSI_DEBUG_QSB_LOG_1_INM(m)      \
        in_dword_masked(HWIO_GSI_DEBUG_QSB_LOG_1_ADDR, m)
#define HWIO_GSI_DEBUG_QSB_LOG_1_AREQPRIORITY_BMSK                                                       0xf0000000
#define HWIO_GSI_DEBUG_QSB_LOG_1_AREQPRIORITY_SHFT                                                             0x1c
#define HWIO_GSI_DEBUG_QSB_LOG_1_ASIZE_BMSK                                                               0xf000000
#define HWIO_GSI_DEBUG_QSB_LOG_1_ASIZE_SHFT                                                                    0x18
#define HWIO_GSI_DEBUG_QSB_LOG_1_ALEN_BMSK                                                                 0xf00000
#define HWIO_GSI_DEBUG_QSB_LOG_1_ALEN_SHFT                                                                     0x14
#define HWIO_GSI_DEBUG_QSB_LOG_1_AOOOWR_BMSK                                                                0x40000
#define HWIO_GSI_DEBUG_QSB_LOG_1_AOOOWR_SHFT                                                                   0x12
#define HWIO_GSI_DEBUG_QSB_LOG_1_AOOORD_BMSK                                                                0x20000
#define HWIO_GSI_DEBUG_QSB_LOG_1_AOOORD_SHFT                                                                   0x11
#define HWIO_GSI_DEBUG_QSB_LOG_1_ATRANSIENT_BMSK                                                            0x10000
#define HWIO_GSI_DEBUG_QSB_LOG_1_ATRANSIENT_SHFT                                                               0x10
#define HWIO_GSI_DEBUG_QSB_LOG_1_ACACHEABLE_BMSK                                                             0x8000
#define HWIO_GSI_DEBUG_QSB_LOG_1_ACACHEABLE_SHFT                                                                0xf
#define HWIO_GSI_DEBUG_QSB_LOG_1_ASHARED_BMSK                                                                0x4000
#define HWIO_GSI_DEBUG_QSB_LOG_1_ASHARED_SHFT                                                                   0xe
#define HWIO_GSI_DEBUG_QSB_LOG_1_ANOALLOCATE_BMSK                                                            0x2000
#define HWIO_GSI_DEBUG_QSB_LOG_1_ANOALLOCATE_SHFT                                                               0xd
#define HWIO_GSI_DEBUG_QSB_LOG_1_AINNERSHARED_BMSK                                                           0x1000
#define HWIO_GSI_DEBUG_QSB_LOG_1_AINNERSHARED_SHFT                                                              0xc
#define HWIO_GSI_DEBUG_QSB_LOG_1_ADDR_43_32_BMSK                                                              0xfff
#define HWIO_GSI_DEBUG_QSB_LOG_1_ADDR_43_32_SHFT                                                                0x0

#define HWIO_GSI_DEBUG_QSB_LOG_2_ADDR                                                                    (GSI_REG_BASE      + 0x0000106c)
#define HWIO_GSI_DEBUG_QSB_LOG_2_PHYS                                                                    (GSI_REG_BASE_PHYS + 0x0000106c)
#define HWIO_GSI_DEBUG_QSB_LOG_2_OFFS                                                                    (GSI_REG_BASE_OFFS + 0x0000106c)
#define HWIO_GSI_DEBUG_QSB_LOG_2_RMSK                                                                        0xffff
#define HWIO_GSI_DEBUG_QSB_LOG_2_ATTR                                                                           0x1
#define HWIO_GSI_DEBUG_QSB_LOG_2_IN          \
        in_dword_masked(HWIO_GSI_DEBUG_QSB_LOG_2_ADDR, HWIO_GSI_DEBUG_QSB_LOG_2_RMSK)
#define HWIO_GSI_DEBUG_QSB_LOG_2_INM(m)      \
        in_dword_masked(HWIO_GSI_DEBUG_QSB_LOG_2_ADDR, m)
#define HWIO_GSI_DEBUG_QSB_LOG_2_AMEMTYPE_BMSK                                                               0xf000
#define HWIO_GSI_DEBUG_QSB_LOG_2_AMEMTYPE_SHFT                                                                  0xc
#define HWIO_GSI_DEBUG_QSB_LOG_2_AMMUSID_BMSK                                                                 0xfff
#define HWIO_GSI_DEBUG_QSB_LOG_2_AMMUSID_SHFT                                                                   0x0

#define HWIO_GSI_DEBUG_QSB_LOG_LAST_MISC_IDn_ADDR(n)                                                     (GSI_REG_BASE      + 0x00001070 + 0x4 * (n))
#define HWIO_GSI_DEBUG_QSB_LOG_LAST_MISC_IDn_PHYS(n)                                                     (GSI_REG_BASE_PHYS + 0x00001070 + 0x4 * (n))
#define HWIO_GSI_DEBUG_QSB_LOG_LAST_MISC_IDn_OFFS(n)                                                     (GSI_REG_BASE_OFFS + 0x00001070 + 0x4 * (n))
#define HWIO_GSI_DEBUG_QSB_LOG_LAST_MISC_IDn_RMSK                                                        0xffffffff
#define HWIO_GSI_DEBUG_QSB_LOG_LAST_MISC_IDn_MAXn                                                                 3
#define HWIO_GSI_DEBUG_QSB_LOG_LAST_MISC_IDn_ATTR                                                               0x1
#define HWIO_GSI_DEBUG_QSB_LOG_LAST_MISC_IDn_INI(n)        \
        in_dword_masked(HWIO_GSI_DEBUG_QSB_LOG_LAST_MISC_IDn_ADDR(n), HWIO_GSI_DEBUG_QSB_LOG_LAST_MISC_IDn_RMSK)
#define HWIO_GSI_DEBUG_QSB_LOG_LAST_MISC_IDn_INMI(n,mask)    \
        in_dword_masked(HWIO_GSI_DEBUG_QSB_LOG_LAST_MISC_IDn_ADDR(n), mask)
#define HWIO_GSI_DEBUG_QSB_LOG_LAST_MISC_IDn_MID_BMSK                                                    0xf8000000
#define HWIO_GSI_DEBUG_QSB_LOG_LAST_MISC_IDn_MID_SHFT                                                          0x1b
#define HWIO_GSI_DEBUG_QSB_LOG_LAST_MISC_IDn_TID_BMSK                                                     0x7c00000
#define HWIO_GSI_DEBUG_QSB_LOG_LAST_MISC_IDn_TID_SHFT                                                          0x16
#define HWIO_GSI_DEBUG_QSB_LOG_LAST_MISC_IDn_WRITE_BMSK                                                    0x200000
#define HWIO_GSI_DEBUG_QSB_LOG_LAST_MISC_IDn_WRITE_SHFT                                                        0x15
#define HWIO_GSI_DEBUG_QSB_LOG_LAST_MISC_IDn_ADDR_20_0_BMSK                                                0x1fffff
#define HWIO_GSI_DEBUG_QSB_LOG_LAST_MISC_IDn_ADDR_20_0_SHFT                                                     0x0

#define HWIO_GSI_DEBUG_SW_RF_n_WRITE_ADDR(n)                                                             (GSI_REG_BASE      + 0x00001080 + 0x4 * (n))
#define HWIO_GSI_DEBUG_SW_RF_n_WRITE_PHYS(n)                                                             (GSI_REG_BASE_PHYS + 0x00001080 + 0x4 * (n))
#define HWIO_GSI_DEBUG_SW_RF_n_WRITE_OFFS(n)                                                             (GSI_REG_BASE_OFFS + 0x00001080 + 0x4 * (n))
#define HWIO_GSI_DEBUG_SW_RF_n_WRITE_RMSK                                                                0xffffffff
#define HWIO_GSI_DEBUG_SW_RF_n_WRITE_MAXn                                                                        31
#define HWIO_GSI_DEBUG_SW_RF_n_WRITE_ATTR                                                                       0x2
#define HWIO_GSI_DEBUG_SW_RF_n_WRITE_OUTI(n,val)    \
        out_dword(HWIO_GSI_DEBUG_SW_RF_n_WRITE_ADDR(n),val)
#define HWIO_GSI_DEBUG_SW_RF_n_WRITE_DATA_IN_BMSK                                                        0xffffffff
#define HWIO_GSI_DEBUG_SW_RF_n_WRITE_DATA_IN_SHFT                                                               0x0

#define HWIO_GSI_DEBUG_SW_RF_n_READ_ADDR(n)                                                              (GSI_REG_BASE      + 0x00001100 + 0x4 * (n))
#define HWIO_GSI_DEBUG_SW_RF_n_READ_PHYS(n)                                                              (GSI_REG_BASE_PHYS + 0x00001100 + 0x4 * (n))
#define HWIO_GSI_DEBUG_SW_RF_n_READ_OFFS(n)                                                              (GSI_REG_BASE_OFFS + 0x00001100 + 0x4 * (n))
#define HWIO_GSI_DEBUG_SW_RF_n_READ_RMSK                                                                 0xffffffff
#define HWIO_GSI_DEBUG_SW_RF_n_READ_MAXn                                                                         31
#define HWIO_GSI_DEBUG_SW_RF_n_READ_ATTR                                                                        0x1
#define HWIO_GSI_DEBUG_SW_RF_n_READ_INI(n)        \
        in_dword_masked(HWIO_GSI_DEBUG_SW_RF_n_READ_ADDR(n), HWIO_GSI_DEBUG_SW_RF_n_READ_RMSK, HWIO_GSI_DEBUG_SW_RF_n_READ_ATTR)
#define HWIO_GSI_DEBUG_SW_RF_n_READ_INMI(n,mask)    \
        in_dword_masked(HWIO_GSI_DEBUG_SW_RF_n_READ_ADDR(n), mask, HWIO_GSI_DEBUG_SW_RF_n_READ_ATTR)
#define HWIO_GSI_DEBUG_SW_RF_n_READ_RF_REG_BMSK                                                          0xffffffff
#define HWIO_GSI_DEBUG_SW_RF_n_READ_RF_REG_SHFT                                                                 0x0

#define HWIO_GSI_DEBUG_COUNTER_CFGn_ADDR(n)                                                              (GSI_REG_BASE      + 0x00001180 + 0x4 * (n))
#define HWIO_GSI_DEBUG_COUNTER_CFGn_PHYS(n)                                                              (GSI_REG_BASE_PHYS + 0x00001180 + 0x4 * (n))
#define HWIO_GSI_DEBUG_COUNTER_CFGn_OFFS(n)                                                              (GSI_REG_BASE_OFFS + 0x00001180 + 0x4 * (n))
#define HWIO_GSI_DEBUG_COUNTER_CFGn_RMSK                                                                   0x1fffff
#define HWIO_GSI_DEBUG_COUNTER_CFGn_MAXn                                                                          7
#define HWIO_GSI_DEBUG_COUNTER_CFGn_ATTR                                                                        0x3
#define HWIO_GSI_DEBUG_COUNTER_CFGn_INI(n)        \
        in_dword_masked(HWIO_GSI_DEBUG_COUNTER_CFGn_ADDR(n), HWIO_GSI_DEBUG_COUNTER_CFGn_RMSK)
#define HWIO_GSI_DEBUG_COUNTER_CFGn_INMI(n,mask)    \
        in_dword_masked(HWIO_GSI_DEBUG_COUNTER_CFGn_ADDR(n), mask)
#define HWIO_GSI_DEBUG_COUNTER_CFGn_OUTI(n,val)    \
        out_dword(HWIO_GSI_DEBUG_COUNTER_CFGn_ADDR(n),val)
#define HWIO_GSI_DEBUG_COUNTER_CFGn_OUTMI(n,mask,val) \
        out_dword_masked_ns(HWIO_GSI_DEBUG_COUNTER_CFGn_ADDR(n),mask,val,HWIO_GSI_DEBUG_COUNTER_CFGn_INI(n))
#define HWIO_GSI_DEBUG_COUNTER_CFGn_CHAIN_BMSK                                                             0x100000
#define HWIO_GSI_DEBUG_COUNTER_CFGn_CHAIN_SHFT                                                                 0x14
#define HWIO_GSI_DEBUG_COUNTER_CFGn_VIRTUAL_CHNL_BMSK                                                       0xff000
#define HWIO_GSI_DEBUG_COUNTER_CFGn_VIRTUAL_CHNL_SHFT                                                           0xc
#define HWIO_GSI_DEBUG_COUNTER_CFGn_EE_BMSK                                                                   0xf00
#define HWIO_GSI_DEBUG_COUNTER_CFGn_EE_SHFT                                                                     0x8
#define HWIO_GSI_DEBUG_COUNTER_CFGn_EVNT_TYPE_BMSK                                                             0xf8
#define HWIO_GSI_DEBUG_COUNTER_CFGn_EVNT_TYPE_SHFT                                                              0x3
#define HWIO_GSI_DEBUG_COUNTER_CFGn_CLR_AT_READ_BMSK                                                            0x4
#define HWIO_GSI_DEBUG_COUNTER_CFGn_CLR_AT_READ_SHFT                                                            0x2
#define HWIO_GSI_DEBUG_COUNTER_CFGn_STOP_AT_WRAP_ARND_BMSK                                                      0x2
#define HWIO_GSI_DEBUG_COUNTER_CFGn_STOP_AT_WRAP_ARND_SHFT                                                      0x1
#define HWIO_GSI_DEBUG_COUNTER_CFGn_ENABLE_BMSK                                                                 0x1
#define HWIO_GSI_DEBUG_COUNTER_CFGn_ENABLE_SHFT                                                                 0x0

#define HWIO_GSI_DEBUG_COUNTERn_ADDR(n)                                                                  (GSI_REG_BASE      + 0x000011a0 + 0x4 * (n))
#define HWIO_GSI_DEBUG_COUNTERn_PHYS(n)                                                                  (GSI_REG_BASE_PHYS + 0x000011a0 + 0x4 * (n))
#define HWIO_GSI_DEBUG_COUNTERn_OFFS(n)                                                                  (GSI_REG_BASE_OFFS + 0x000011a0 + 0x4 * (n))
#define HWIO_GSI_DEBUG_COUNTERn_RMSK                                                                         0xffff
#define HWIO_GSI_DEBUG_COUNTERn_MAXn                                                                              7
#define HWIO_GSI_DEBUG_COUNTERn_ATTR                                                                            0x1
#define HWIO_GSI_DEBUG_COUNTERn_INI(n)        \
        in_dword_masked(HWIO_GSI_DEBUG_COUNTERn_ADDR(n), HWIO_GSI_DEBUG_COUNTERn_RMSK, HWIO_GSI_DEBUG_COUNTERn_ATTR)
#define HWIO_GSI_DEBUG_COUNTERn_INMI(n,mask)    \
        in_dword_masked(HWIO_GSI_DEBUG_COUNTERn_ADDR(n), mask, HWIO_GSI_DEBUG_COUNTERn_ATTR)
#define HWIO_GSI_DEBUG_COUNTERn_COUNTER_VALUE_BMSK                                                           0xffff
#define HWIO_GSI_DEBUG_COUNTERn_COUNTER_VALUE_SHFT                                                              0x0

#define HWIO_GSI_DEBUG_SW_MSK_REG_n_SEC_k_WR_ADDR(n,k)                                                   (GSI_REG_BASE      + 0x000011c0 + 0x4 * (n) + 0x24 * (k))
#define HWIO_GSI_DEBUG_SW_MSK_REG_n_SEC_k_WR_PHYS(n,k)                                                   (GSI_REG_BASE_PHYS + 0x000011c0 + 0x4 * (n) + 0x24 * (k))
#define HWIO_GSI_DEBUG_SW_MSK_REG_n_SEC_k_WR_OFFS(n,k)                                                   (GSI_REG_BASE_OFFS + 0x000011c0 + 0x4 * (n) + 0x24 * (k))
#define HWIO_GSI_DEBUG_SW_MSK_REG_n_SEC_k_WR_RMSK                                                        0xffffffff
#define HWIO_GSI_DEBUG_SW_MSK_REG_n_SEC_k_WR_MAXn                                                                 8
#define HWIO_GSI_DEBUG_SW_MSK_REG_n_SEC_k_WR_MAXk                                                                 1
#define HWIO_GSI_DEBUG_SW_MSK_REG_n_SEC_k_WR_ATTR                                                               0x2
#define HWIO_GSI_DEBUG_SW_MSK_REG_n_SEC_k_WR_OUTI2(n,k,val)    \
        out_dword(HWIO_GSI_DEBUG_SW_MSK_REG_n_SEC_k_WR_ADDR(n,k),val)
#define HWIO_GSI_DEBUG_SW_MSK_REG_n_SEC_k_WR_DATA_IN_BMSK                                                0xffffffff
#define HWIO_GSI_DEBUG_SW_MSK_REG_n_SEC_k_WR_DATA_IN_SHFT                                                       0x0

#define HWIO_GSI_DEBUG_SW_MSK_REG_n_SEC_k_RD_ADDR(n,k)                                                   (GSI_REG_BASE      + 0x000012e0 + 0x4 * (n) + 0x24 * (k))
#define HWIO_GSI_DEBUG_SW_MSK_REG_n_SEC_k_RD_PHYS(n,k)                                                   (GSI_REG_BASE_PHYS + 0x000012e0 + 0x4 * (n) + 0x24 * (k))
#define HWIO_GSI_DEBUG_SW_MSK_REG_n_SEC_k_RD_OFFS(n,k)                                                   (GSI_REG_BASE_OFFS + 0x000012e0 + 0x4 * (n) + 0x24 * (k))
#define HWIO_GSI_DEBUG_SW_MSK_REG_n_SEC_k_RD_RMSK                                                        0xffffffff
#define HWIO_GSI_DEBUG_SW_MSK_REG_n_SEC_k_RD_MAXn                                                                 8
#define HWIO_GSI_DEBUG_SW_MSK_REG_n_SEC_k_RD_MAXk                                                                 1
#define HWIO_GSI_DEBUG_SW_MSK_REG_n_SEC_k_RD_ATTR                                                               0x1
#define HWIO_GSI_DEBUG_SW_MSK_REG_n_SEC_k_RD_INI2(n,k)        \
        in_dword_masked(HWIO_GSI_DEBUG_SW_MSK_REG_n_SEC_k_RD_ADDR(n,k), HWIO_GSI_DEBUG_SW_MSK_REG_n_SEC_k_RD_RMSK)
#define HWIO_GSI_DEBUG_SW_MSK_REG_n_SEC_k_RD_INMI2(n,k,mask)    \
        in_dword_masked(HWIO_GSI_DEBUG_SW_MSK_REG_n_SEC_k_RD_ADDR(n,k), mask)
#define HWIO_GSI_DEBUG_SW_MSK_REG_n_SEC_k_RD_MSK_REG_BMSK                                                0xffffffff
#define HWIO_GSI_DEBUG_SW_MSK_REG_n_SEC_k_RD_MSK_REG_SHFT                                                       0x0

#define HWIO_GSI_DEBUG_EE_n_CH_k_VP_TABLE_ADDR(n,k)                                                      (GSI_REG_BASE      + 0x00001400 + 0x80 * (n) + 0x4 * (k))
#define HWIO_GSI_DEBUG_EE_n_CH_k_VP_TABLE_PHYS(n,k)                                                      (GSI_REG_BASE_PHYS + 0x00001400 + 0x80 * (n) + 0x4 * (k))
#define HWIO_GSI_DEBUG_EE_n_CH_k_VP_TABLE_OFFS(n,k)                                                      (GSI_REG_BASE_OFFS + 0x00001400 + 0x80 * (n) + 0x4 * (k))
#define HWIO_GSI_DEBUG_EE_n_CH_k_VP_TABLE_RMSK                                                                0x1ff
#define HWIO_GSI_DEBUG_EE_n_CH_k_VP_TABLE_MAXn                                                                    3
#define HWIO_GSI_DEBUG_EE_n_CH_k_VP_TABLE_MAXk                                                                   27
#define HWIO_GSI_DEBUG_EE_n_CH_k_VP_TABLE_ATTR                                                                  0x1
#define HWIO_GSI_DEBUG_EE_n_CH_k_VP_TABLE_INI2(n,k)        \
        in_dword_masked(HWIO_GSI_DEBUG_EE_n_CH_k_VP_TABLE_ADDR(n,k), HWIO_GSI_DEBUG_EE_n_CH_k_VP_TABLE_RMSK)
#define HWIO_GSI_DEBUG_EE_n_CH_k_VP_TABLE_INMI2(n,k,mask)    \
        in_dword_masked(HWIO_GSI_DEBUG_EE_n_CH_k_VP_TABLE_ADDR(n,k), mask)
#define HWIO_GSI_DEBUG_EE_n_CH_k_VP_TABLE_VALID_BMSK                                                          0x100
#define HWIO_GSI_DEBUG_EE_n_CH_k_VP_TABLE_VALID_SHFT                                                            0x8
#define HWIO_GSI_DEBUG_EE_n_CH_k_VP_TABLE_PHY_CH_BMSK                                                          0xff
#define HWIO_GSI_DEBUG_EE_n_CH_k_VP_TABLE_PHY_CH_SHFT                                                           0x0

#define HWIO_GSI_DEBUG_EE_n_EV_k_VP_TABLE_ADDR(n,k)                                                      (GSI_REG_BASE      + 0x00001600 + 0x100 * (n) + 0x4 * (k))
#define HWIO_GSI_DEBUG_EE_n_EV_k_VP_TABLE_PHYS(n,k)                                                      (GSI_REG_BASE_PHYS + 0x00001600 + 0x100 * (n) + 0x4 * (k))
#define HWIO_GSI_DEBUG_EE_n_EV_k_VP_TABLE_OFFS(n,k)                                                      (GSI_REG_BASE_OFFS + 0x00001600 + 0x100 * (n) + 0x4 * (k))
#define HWIO_GSI_DEBUG_EE_n_EV_k_VP_TABLE_RMSK                                                                0x1ff
#define HWIO_GSI_DEBUG_EE_n_EV_k_VP_TABLE_MAXn                                                                    3
#define HWIO_GSI_DEBUG_EE_n_EV_k_VP_TABLE_MAXk                                                                   26
#define HWIO_GSI_DEBUG_EE_n_EV_k_VP_TABLE_ATTR                                                                  0x1
#define HWIO_GSI_DEBUG_EE_n_EV_k_VP_TABLE_INI2(n,k)        \
        in_dword_masked(HWIO_GSI_DEBUG_EE_n_EV_k_VP_TABLE_ADDR(n,k), HWIO_GSI_DEBUG_EE_n_EV_k_VP_TABLE_RMSK)
#define HWIO_GSI_DEBUG_EE_n_EV_k_VP_TABLE_INMI2(n,k,mask)    \
        in_dword_masked(HWIO_GSI_DEBUG_EE_n_EV_k_VP_TABLE_ADDR(n,k), mask)
#define HWIO_GSI_DEBUG_EE_n_EV_k_VP_TABLE_VALID_BMSK                                                          0x100
#define HWIO_GSI_DEBUG_EE_n_EV_k_VP_TABLE_VALID_SHFT                                                            0x8
#define HWIO_GSI_DEBUG_EE_n_EV_k_VP_TABLE_PHY_EV_CH_BMSK                                                       0xff
#define HWIO_GSI_DEBUG_EE_n_EV_k_VP_TABLE_PHY_EV_CH_SHFT                                                        0x0

#define HWIO_GSI_DEBUG_REE_PREFETCH_BUF_CH_ID_ADDR                                                       (GSI_REG_BASE      + 0x00001a54)
#define HWIO_GSI_DEBUG_REE_PREFETCH_BUF_CH_ID_PHYS                                                       (GSI_REG_BASE_PHYS + 0x00001a54)
#define HWIO_GSI_DEBUG_REE_PREFETCH_BUF_CH_ID_OFFS                                                       (GSI_REG_BASE_OFFS + 0x00001a54)
#define HWIO_GSI_DEBUG_REE_PREFETCH_BUF_CH_ID_RMSK                                                             0xff
#define HWIO_GSI_DEBUG_REE_PREFETCH_BUF_CH_ID_ATTR                                                              0x3
#define HWIO_GSI_DEBUG_REE_PREFETCH_BUF_CH_ID_IN          \
        in_dword_masked(HWIO_GSI_DEBUG_REE_PREFETCH_BUF_CH_ID_ADDR, HWIO_GSI_DEBUG_REE_PREFETCH_BUF_CH_ID_RMSK)
#define HWIO_GSI_DEBUG_REE_PREFETCH_BUF_CH_ID_INM(m)      \
        in_dword_masked(HWIO_GSI_DEBUG_REE_PREFETCH_BUF_CH_ID_ADDR, m)
#define HWIO_GSI_DEBUG_REE_PREFETCH_BUF_CH_ID_OUT(v)      \
        out_dword(HWIO_GSI_DEBUG_REE_PREFETCH_BUF_CH_ID_ADDR,v)
#define HWIO_GSI_DEBUG_REE_PREFETCH_BUF_CH_ID_OUTM(m,v) \
        out_dword_masked_ns(HWIO_GSI_DEBUG_REE_PREFETCH_BUF_CH_ID_ADDR,m,v,HWIO_GSI_DEBUG_REE_PREFETCH_BUF_CH_ID_IN)
#define HWIO_GSI_DEBUG_REE_PREFETCH_BUF_CH_ID_PREFETCH_BUF_CH_ID_BMSK                                          0xff
#define HWIO_GSI_DEBUG_REE_PREFETCH_BUF_CH_ID_PREFETCH_BUF_CH_ID_SHFT                                           0x0

#define HWIO_GSI_DEBUG_REE_PREFETCH_BUF_STATUS_ADDR                                                      (GSI_REG_BASE      + 0x00001a58)
#define HWIO_GSI_DEBUG_REE_PREFETCH_BUF_STATUS_PHYS                                                      (GSI_REG_BASE_PHYS + 0x00001a58)
#define HWIO_GSI_DEBUG_REE_PREFETCH_BUF_STATUS_OFFS                                                      (GSI_REG_BASE_OFFS + 0x00001a58)
#define HWIO_GSI_DEBUG_REE_PREFETCH_BUF_STATUS_RMSK                                                      0xffffffff
#define HWIO_GSI_DEBUG_REE_PREFETCH_BUF_STATUS_ATTR                                                             0x1
#define HWIO_GSI_DEBUG_REE_PREFETCH_BUF_STATUS_IN          \
        in_dword_masked(HWIO_GSI_DEBUG_REE_PREFETCH_BUF_STATUS_ADDR, HWIO_GSI_DEBUG_REE_PREFETCH_BUF_STATUS_RMSK)
#define HWIO_GSI_DEBUG_REE_PREFETCH_BUF_STATUS_INM(m)      \
        in_dword_masked(HWIO_GSI_DEBUG_REE_PREFETCH_BUF_STATUS_ADDR, m)
#define HWIO_GSI_DEBUG_REE_PREFETCH_BUF_STATUS_PREFETCH_BUF_STATUS_BMSK                                  0xffffffff
#define HWIO_GSI_DEBUG_REE_PREFETCH_BUF_STATUS_PREFETCH_BUF_STATUS_SHFT                                         0x0

#define HWIO_GSI_MCS_PROFILING_BP_CNT_LSB_ADDR                                                           (GSI_REG_BASE      + 0x00001a5c)
#define HWIO_GSI_MCS_PROFILING_BP_CNT_LSB_PHYS                                                           (GSI_REG_BASE_PHYS + 0x00001a5c)
#define HWIO_GSI_MCS_PROFILING_BP_CNT_LSB_OFFS                                                           (GSI_REG_BASE_OFFS + 0x00001a5c)
#define HWIO_GSI_MCS_PROFILING_BP_CNT_LSB_RMSK                                                           0xffffffff
#define HWIO_GSI_MCS_PROFILING_BP_CNT_LSB_ATTR                                                                  0x1
#define HWIO_GSI_MCS_PROFILING_BP_CNT_LSB_IN          \
        in_dword_masked(HWIO_GSI_MCS_PROFILING_BP_CNT_LSB_ADDR, HWIO_GSI_MCS_PROFILING_BP_CNT_LSB_RMSK)
#define HWIO_GSI_MCS_PROFILING_BP_CNT_LSB_INM(m)      \
        in_dword_masked(HWIO_GSI_MCS_PROFILING_BP_CNT_LSB_ADDR, m)
#define HWIO_GSI_MCS_PROFILING_BP_CNT_LSB_BP_CNT_LSB_BMSK                                                0xffffffff
#define HWIO_GSI_MCS_PROFILING_BP_CNT_LSB_BP_CNT_LSB_SHFT                                                       0x0

#define HWIO_GSI_MCS_PROFILING_BP_CNT_MSB_ADDR                                                           (GSI_REG_BASE      + 0x00001a60)
#define HWIO_GSI_MCS_PROFILING_BP_CNT_MSB_PHYS                                                           (GSI_REG_BASE_PHYS + 0x00001a60)
#define HWIO_GSI_MCS_PROFILING_BP_CNT_MSB_OFFS                                                           (GSI_REG_BASE_OFFS + 0x00001a60)
#define HWIO_GSI_MCS_PROFILING_BP_CNT_MSB_RMSK                                                                  0xf
#define HWIO_GSI_MCS_PROFILING_BP_CNT_MSB_ATTR                                                                  0x1
#define HWIO_GSI_MCS_PROFILING_BP_CNT_MSB_IN          \
        in_dword_masked(HWIO_GSI_MCS_PROFILING_BP_CNT_MSB_ADDR, HWIO_GSI_MCS_PROFILING_BP_CNT_MSB_RMSK)
#define HWIO_GSI_MCS_PROFILING_BP_CNT_MSB_INM(m)      \
        in_dword_masked(HWIO_GSI_MCS_PROFILING_BP_CNT_MSB_ADDR, m)
#define HWIO_GSI_MCS_PROFILING_BP_CNT_MSB_BP_CNT_MSB_BMSK                                                       0xf
#define HWIO_GSI_MCS_PROFILING_BP_CNT_MSB_BP_CNT_MSB_SHFT                                                       0x0

#define HWIO_GSI_MCS_PROFILING_BP_AND_PENDING_CNT_LSB_ADDR                                               (GSI_REG_BASE      + 0x00001a64)
#define HWIO_GSI_MCS_PROFILING_BP_AND_PENDING_CNT_LSB_PHYS                                               (GSI_REG_BASE_PHYS + 0x00001a64)
#define HWIO_GSI_MCS_PROFILING_BP_AND_PENDING_CNT_LSB_OFFS                                               (GSI_REG_BASE_OFFS + 0x00001a64)
#define HWIO_GSI_MCS_PROFILING_BP_AND_PENDING_CNT_LSB_RMSK                                               0xffffffff
#define HWIO_GSI_MCS_PROFILING_BP_AND_PENDING_CNT_LSB_ATTR                                                      0x1
#define HWIO_GSI_MCS_PROFILING_BP_AND_PENDING_CNT_LSB_IN          \
        in_dword_masked(HWIO_GSI_MCS_PROFILING_BP_AND_PENDING_CNT_LSB_ADDR, HWIO_GSI_MCS_PROFILING_BP_AND_PENDING_CNT_LSB_RMSK)
#define HWIO_GSI_MCS_PROFILING_BP_AND_PENDING_CNT_LSB_INM(m)      \
        in_dword_masked(HWIO_GSI_MCS_PROFILING_BP_AND_PENDING_CNT_LSB_ADDR, m)
#define HWIO_GSI_MCS_PROFILING_BP_AND_PENDING_CNT_LSB_BP_AND_PENDING_CNT_LSB_BMSK                        0xffffffff
#define HWIO_GSI_MCS_PROFILING_BP_AND_PENDING_CNT_LSB_BP_AND_PENDING_CNT_LSB_SHFT                               0x0

#define HWIO_GSI_MCS_PROFILING_BP_AND_PENDING_CNT_MSB_ADDR                                               (GSI_REG_BASE      + 0x00001a68)
#define HWIO_GSI_MCS_PROFILING_BP_AND_PENDING_CNT_MSB_PHYS                                               (GSI_REG_BASE_PHYS + 0x00001a68)
#define HWIO_GSI_MCS_PROFILING_BP_AND_PENDING_CNT_MSB_OFFS                                               (GSI_REG_BASE_OFFS + 0x00001a68)
#define HWIO_GSI_MCS_PROFILING_BP_AND_PENDING_CNT_MSB_RMSK                                                      0xf
#define HWIO_GSI_MCS_PROFILING_BP_AND_PENDING_CNT_MSB_ATTR                                                      0x1
#define HWIO_GSI_MCS_PROFILING_BP_AND_PENDING_CNT_MSB_IN          \
        in_dword_masked(HWIO_GSI_MCS_PROFILING_BP_AND_PENDING_CNT_MSB_ADDR, HWIO_GSI_MCS_PROFILING_BP_AND_PENDING_CNT_MSB_RMSK)
#define HWIO_GSI_MCS_PROFILING_BP_AND_PENDING_CNT_MSB_INM(m)      \
        in_dword_masked(HWIO_GSI_MCS_PROFILING_BP_AND_PENDING_CNT_MSB_ADDR, m)
#define HWIO_GSI_MCS_PROFILING_BP_AND_PENDING_CNT_MSB_BP_AND_PENDING_CNT_MSB_BMSK                               0xf
#define HWIO_GSI_MCS_PROFILING_BP_AND_PENDING_CNT_MSB_BP_AND_PENDING_CNT_MSB_SHFT                               0x0

#define HWIO_GSI_MCS_PROFILING_MCS_BUSY_CNT_LSB_ADDR                                                     (GSI_REG_BASE      + 0x00001a6c)
#define HWIO_GSI_MCS_PROFILING_MCS_BUSY_CNT_LSB_PHYS                                                     (GSI_REG_BASE_PHYS + 0x00001a6c)
#define HWIO_GSI_MCS_PROFILING_MCS_BUSY_CNT_LSB_OFFS                                                     (GSI_REG_BASE_OFFS + 0x00001a6c)
#define HWIO_GSI_MCS_PROFILING_MCS_BUSY_CNT_LSB_RMSK                                                     0xffffffff
#define HWIO_GSI_MCS_PROFILING_MCS_BUSY_CNT_LSB_ATTR                                                            0x1
#define HWIO_GSI_MCS_PROFILING_MCS_BUSY_CNT_LSB_IN          \
        in_dword_masked(HWIO_GSI_MCS_PROFILING_MCS_BUSY_CNT_LSB_ADDR, HWIO_GSI_MCS_PROFILING_MCS_BUSY_CNT_LSB_RMSK)
#define HWIO_GSI_MCS_PROFILING_MCS_BUSY_CNT_LSB_INM(m)      \
        in_dword_masked(HWIO_GSI_MCS_PROFILING_MCS_BUSY_CNT_LSB_ADDR, m)
#define HWIO_GSI_MCS_PROFILING_MCS_BUSY_CNT_LSB_MCS_BUSY_CNT_LSB_BMSK                                    0xffffffff
#define HWIO_GSI_MCS_PROFILING_MCS_BUSY_CNT_LSB_MCS_BUSY_CNT_LSB_SHFT                                           0x0

#define HWIO_GSI_MCS_PROFILING_MCS_BUSY_CNT_MSB_ADDR                                                     (GSI_REG_BASE      + 0x00001a70)
#define HWIO_GSI_MCS_PROFILING_MCS_BUSY_CNT_MSB_PHYS                                                     (GSI_REG_BASE_PHYS + 0x00001a70)
#define HWIO_GSI_MCS_PROFILING_MCS_BUSY_CNT_MSB_OFFS                                                     (GSI_REG_BASE_OFFS + 0x00001a70)
#define HWIO_GSI_MCS_PROFILING_MCS_BUSY_CNT_MSB_RMSK                                                            0xf
#define HWIO_GSI_MCS_PROFILING_MCS_BUSY_CNT_MSB_ATTR                                                            0x1
#define HWIO_GSI_MCS_PROFILING_MCS_BUSY_CNT_MSB_IN          \
        in_dword_masked(HWIO_GSI_MCS_PROFILING_MCS_BUSY_CNT_MSB_ADDR, HWIO_GSI_MCS_PROFILING_MCS_BUSY_CNT_MSB_RMSK)
#define HWIO_GSI_MCS_PROFILING_MCS_BUSY_CNT_MSB_INM(m)      \
        in_dword_masked(HWIO_GSI_MCS_PROFILING_MCS_BUSY_CNT_MSB_ADDR, m)
#define HWIO_GSI_MCS_PROFILING_MCS_BUSY_CNT_MSB_MCS_BUSY_CNT_MSB_BMSK                                           0xf
#define HWIO_GSI_MCS_PROFILING_MCS_BUSY_CNT_MSB_MCS_BUSY_CNT_MSB_SHFT                                           0x0

#define HWIO_GSI_MCS_PROFILING_MCS_IDLE_CNT_LSB_ADDR                                                     (GSI_REG_BASE      + 0x00001a74)
#define HWIO_GSI_MCS_PROFILING_MCS_IDLE_CNT_LSB_PHYS                                                     (GSI_REG_BASE_PHYS + 0x00001a74)
#define HWIO_GSI_MCS_PROFILING_MCS_IDLE_CNT_LSB_OFFS                                                     (GSI_REG_BASE_OFFS + 0x00001a74)
#define HWIO_GSI_MCS_PROFILING_MCS_IDLE_CNT_LSB_RMSK                                                     0xffffffff
#define HWIO_GSI_MCS_PROFILING_MCS_IDLE_CNT_LSB_ATTR                                                            0x1
#define HWIO_GSI_MCS_PROFILING_MCS_IDLE_CNT_LSB_IN          \
        in_dword_masked(HWIO_GSI_MCS_PROFILING_MCS_IDLE_CNT_LSB_ADDR, HWIO_GSI_MCS_PROFILING_MCS_IDLE_CNT_LSB_RMSK)
#define HWIO_GSI_MCS_PROFILING_MCS_IDLE_CNT_LSB_INM(m)      \
        in_dword_masked(HWIO_GSI_MCS_PROFILING_MCS_IDLE_CNT_LSB_ADDR, m)
#define HWIO_GSI_MCS_PROFILING_MCS_IDLE_CNT_LSB_MCS_IDLE_CNT_LSB_BMSK                                    0xffffffff
#define HWIO_GSI_MCS_PROFILING_MCS_IDLE_CNT_LSB_MCS_IDLE_CNT_LSB_SHFT                                           0x0

#define HWIO_GSI_MCS_PROFILING_MCS_IDLE_CNT_MSB_ADDR                                                     (GSI_REG_BASE      + 0x00001a78)
#define HWIO_GSI_MCS_PROFILING_MCS_IDLE_CNT_MSB_PHYS                                                     (GSI_REG_BASE_PHYS + 0x00001a78)
#define HWIO_GSI_MCS_PROFILING_MCS_IDLE_CNT_MSB_OFFS                                                     (GSI_REG_BASE_OFFS + 0x00001a78)
#define HWIO_GSI_MCS_PROFILING_MCS_IDLE_CNT_MSB_RMSK                                                            0xf
#define HWIO_GSI_MCS_PROFILING_MCS_IDLE_CNT_MSB_ATTR                                                            0x1
#define HWIO_GSI_MCS_PROFILING_MCS_IDLE_CNT_MSB_IN          \
        in_dword_masked(HWIO_GSI_MCS_PROFILING_MCS_IDLE_CNT_MSB_ADDR, HWIO_GSI_MCS_PROFILING_MCS_IDLE_CNT_MSB_RMSK)
#define HWIO_GSI_MCS_PROFILING_MCS_IDLE_CNT_MSB_INM(m)      \
        in_dword_masked(HWIO_GSI_MCS_PROFILING_MCS_IDLE_CNT_MSB_ADDR, m)
#define HWIO_GSI_MCS_PROFILING_MCS_IDLE_CNT_MSB_MCS_IDLE_CNT_MSB_BMSK                                           0xf
#define HWIO_GSI_MCS_PROFILING_MCS_IDLE_CNT_MSB_MCS_IDLE_CNT_MSB_SHFT                                           0x0

#define HWIO_EE_n_GSI_CH_k_CNTXT_0_ADDR(n,k)                                                             (GSI_REG_BASE      + 0x00014000 + 0x12000 * (n) + 0x80 * (k))
#define HWIO_EE_n_GSI_CH_k_CNTXT_0_PHYS(n,k)                                                             (GSI_REG_BASE_PHYS + 0x00014000 + 0x12000 * (n) + 0x80 * (k))
#define HWIO_EE_n_GSI_CH_k_CNTXT_0_OFFS(n,k)                                                             (GSI_REG_BASE_OFFS + 0x00014000 + 0x12000 * (n) + 0x80 * (k))
#define HWIO_EE_n_GSI_CH_k_CNTXT_0_RMSK                                                                  0xffffffff
#define HWIO_EE_n_GSI_CH_k_CNTXT_0_MAXn                                                                           2
#define HWIO_EE_n_GSI_CH_k_CNTXT_0_MAXk                                                                          27
#define HWIO_EE_n_GSI_CH_k_CNTXT_0_ATTR                                                                         0x3
#define HWIO_EE_n_GSI_CH_k_CNTXT_0_INI2(n,k)        \
        in_dword_masked(HWIO_EE_n_GSI_CH_k_CNTXT_0_ADDR(n,k), HWIO_EE_n_GSI_CH_k_CNTXT_0_RMSK)
#define HWIO_EE_n_GSI_CH_k_CNTXT_0_INMI2(n,k,mask)    \
        in_dword_masked(HWIO_EE_n_GSI_CH_k_CNTXT_0_ADDR(n,k), mask)
#define HWIO_EE_n_GSI_CH_k_CNTXT_0_OUTI2(n,k,val)    \
        out_dword(HWIO_EE_n_GSI_CH_k_CNTXT_0_ADDR(n,k),val)
#define HWIO_EE_n_GSI_CH_k_CNTXT_0_OUTMI2(n,k,mask,val) \
        out_dword_masked_ns(HWIO_EE_n_GSI_CH_k_CNTXT_0_ADDR(n,k),mask,val,HWIO_EE_n_GSI_CH_k_CNTXT_0_INI2(n,k))
#define HWIO_EE_n_GSI_CH_k_CNTXT_0_ELEMENT_SIZE_BMSK                                                     0xff000000
#define HWIO_EE_n_GSI_CH_k_CNTXT_0_ELEMENT_SIZE_SHFT                                                           0x18
#define HWIO_EE_n_GSI_CH_k_CNTXT_0_CHSTATE_BMSK                                                            0xf00000
#define HWIO_EE_n_GSI_CH_k_CNTXT_0_CHSTATE_SHFT                                                                0x14
#define HWIO_EE_n_GSI_CH_k_CNTXT_0_CHSTATE_NOT_ALLOCATED_FVAL                                                   0x0
#define HWIO_EE_n_GSI_CH_k_CNTXT_0_CHSTATE_ALLOCATED_FVAL                                                       0x1
#define HWIO_EE_n_GSI_CH_k_CNTXT_0_CHSTATE_STARTED_FVAL                                                         0x2
#define HWIO_EE_n_GSI_CH_k_CNTXT_0_CHSTATE_STOPED_FVAL                                                          0x3
#define HWIO_EE_n_GSI_CH_k_CNTXT_0_CHSTATE_STOP_IN_PROC_FVAL                                                    0x4
#define HWIO_EE_n_GSI_CH_k_CNTXT_0_CHSTATE_ERROR_FVAL                                                           0xf
#define HWIO_EE_n_GSI_CH_k_CNTXT_0_CHID_BMSK                                                                0xff000
#define HWIO_EE_n_GSI_CH_k_CNTXT_0_CHID_SHFT                                                                    0xc
#define HWIO_EE_n_GSI_CH_k_CNTXT_0_EE_BMSK                                                                    0xf00
#define HWIO_EE_n_GSI_CH_k_CNTXT_0_EE_SHFT                                                                      0x8
#define HWIO_EE_n_GSI_CH_k_CNTXT_0_CHTYPE_DIR_BMSK                                                             0x80
#define HWIO_EE_n_GSI_CH_k_CNTXT_0_CHTYPE_DIR_SHFT                                                              0x7
#define HWIO_EE_n_GSI_CH_k_CNTXT_0_CHTYPE_DIR_INBOUND_FVAL                                                      0x0
#define HWIO_EE_n_GSI_CH_k_CNTXT_0_CHTYPE_DIR_OUTBOUND_FVAL                                                     0x1
#define HWIO_EE_n_GSI_CH_k_CNTXT_0_CHTYPE_PROTOCOL_BMSK                                                        0x7f
#define HWIO_EE_n_GSI_CH_k_CNTXT_0_CHTYPE_PROTOCOL_SHFT                                                         0x0
#define HWIO_EE_n_GSI_CH_k_CNTXT_0_CHTYPE_PROTOCOL_MHI_FVAL                                                     0x0
#define HWIO_EE_n_GSI_CH_k_CNTXT_0_CHTYPE_PROTOCOL_XHCI_FVAL                                                    0x1
#define HWIO_EE_n_GSI_CH_k_CNTXT_0_CHTYPE_PROTOCOL_GPI_FVAL                                                     0x2
#define HWIO_EE_n_GSI_CH_k_CNTXT_0_CHTYPE_PROTOCOL_XDCI_FVAL                                                    0x3

#define HWIO_EE_n_GSI_CH_k_CNTXT_1_ADDR(n,k)                                                             (GSI_REG_BASE      + 0x00014004 + 0x12000 * (n) + 0x80 * (k))
#define HWIO_EE_n_GSI_CH_k_CNTXT_1_PHYS(n,k)                                                             (GSI_REG_BASE_PHYS + 0x00014004 + 0x12000 * (n) + 0x80 * (k))
#define HWIO_EE_n_GSI_CH_k_CNTXT_1_OFFS(n,k)                                                             (GSI_REG_BASE_OFFS + 0x00014004 + 0x12000 * (n) + 0x80 * (k))
#define HWIO_EE_n_GSI_CH_k_CNTXT_1_RMSK                                                                  0xffffffff
#define HWIO_EE_n_GSI_CH_k_CNTXT_1_MAXn                                                                           2
#define HWIO_EE_n_GSI_CH_k_CNTXT_1_MAXk                                                                          27
#define HWIO_EE_n_GSI_CH_k_CNTXT_1_ATTR                                                                         0x3
#define HWIO_EE_n_GSI_CH_k_CNTXT_1_INI2(n,k)        \
        in_dword_masked(HWIO_EE_n_GSI_CH_k_CNTXT_1_ADDR(n,k), HWIO_EE_n_GSI_CH_k_CNTXT_1_RMSK)
#define HWIO_EE_n_GSI_CH_k_CNTXT_1_INMI2(n,k,mask)    \
        in_dword_masked(HWIO_EE_n_GSI_CH_k_CNTXT_1_ADDR(n,k), mask)
#define HWIO_EE_n_GSI_CH_k_CNTXT_1_OUTI2(n,k,val)    \
        out_dword(HWIO_EE_n_GSI_CH_k_CNTXT_1_ADDR(n,k),val)
#define HWIO_EE_n_GSI_CH_k_CNTXT_1_OUTMI2(n,k,mask,val) \
        out_dword_masked_ns(HWIO_EE_n_GSI_CH_k_CNTXT_1_ADDR(n,k),mask,val,HWIO_EE_n_GSI_CH_k_CNTXT_1_INI2(n,k))
#define HWIO_EE_n_GSI_CH_k_CNTXT_1_ERINDEX_BMSK                                                          0xff000000
#define HWIO_EE_n_GSI_CH_k_CNTXT_1_ERINDEX_SHFT                                                                0x18
#define HWIO_EE_n_GSI_CH_k_CNTXT_1_R_LENGTH_BMSK                                                           0xffffff
#define HWIO_EE_n_GSI_CH_k_CNTXT_1_R_LENGTH_SHFT                                                                0x0

#define HWIO_EE_n_GSI_CH_k_CNTXT_2_ADDR(n,k)                                                             (GSI_REG_BASE      + 0x00014008 + 0x12000 * (n) + 0x80 * (k))
#define HWIO_EE_n_GSI_CH_k_CNTXT_2_PHYS(n,k)                                                             (GSI_REG_BASE_PHYS + 0x00014008 + 0x12000 * (n) + 0x80 * (k))
#define HWIO_EE_n_GSI_CH_k_CNTXT_2_OFFS(n,k)                                                             (GSI_REG_BASE_OFFS + 0x00014008 + 0x12000 * (n) + 0x80 * (k))
#define HWIO_EE_n_GSI_CH_k_CNTXT_2_RMSK                                                                  0xffffffff
#define HWIO_EE_n_GSI_CH_k_CNTXT_2_MAXn                                                                           2
#define HWIO_EE_n_GSI_CH_k_CNTXT_2_MAXk                                                                          27
#define HWIO_EE_n_GSI_CH_k_CNTXT_2_ATTR                                                                         0x3
#define HWIO_EE_n_GSI_CH_k_CNTXT_2_INI2(n,k)        \
        in_dword_masked(HWIO_EE_n_GSI_CH_k_CNTXT_2_ADDR(n,k), HWIO_EE_n_GSI_CH_k_CNTXT_2_RMSK)
#define HWIO_EE_n_GSI_CH_k_CNTXT_2_INMI2(n,k,mask)    \
        in_dword_masked(HWIO_EE_n_GSI_CH_k_CNTXT_2_ADDR(n,k), mask)
#define HWIO_EE_n_GSI_CH_k_CNTXT_2_OUTI2(n,k,val)    \
        out_dword(HWIO_EE_n_GSI_CH_k_CNTXT_2_ADDR(n,k),val)
#define HWIO_EE_n_GSI_CH_k_CNTXT_2_OUTMI2(n,k,mask,val) \
        out_dword_masked_ns(HWIO_EE_n_GSI_CH_k_CNTXT_2_ADDR(n,k),mask,val,HWIO_EE_n_GSI_CH_k_CNTXT_2_INI2(n,k))
#define HWIO_EE_n_GSI_CH_k_CNTXT_2_R_BASE_ADDR_LSBS_BMSK                                                 0xffffffff
#define HWIO_EE_n_GSI_CH_k_CNTXT_2_R_BASE_ADDR_LSBS_SHFT                                                        0x0

#define HWIO_EE_n_GSI_CH_k_CNTXT_3_ADDR(n,k)                                                             (GSI_REG_BASE      + 0x0001400c + 0x12000 * (n) + 0x80 * (k))
#define HWIO_EE_n_GSI_CH_k_CNTXT_3_PHYS(n,k)                                                             (GSI_REG_BASE_PHYS + 0x0001400c + 0x12000 * (n) + 0x80 * (k))
#define HWIO_EE_n_GSI_CH_k_CNTXT_3_OFFS(n,k)                                                             (GSI_REG_BASE_OFFS + 0x0001400c + 0x12000 * (n) + 0x80 * (k))
#define HWIO_EE_n_GSI_CH_k_CNTXT_3_RMSK                                                                  0xffffffff
#define HWIO_EE_n_GSI_CH_k_CNTXT_3_MAXn                                                                           2
#define HWIO_EE_n_GSI_CH_k_CNTXT_3_MAXk                                                                          27
#define HWIO_EE_n_GSI_CH_k_CNTXT_3_ATTR                                                                         0x3
#define HWIO_EE_n_GSI_CH_k_CNTXT_3_INI2(n,k)        \
        in_dword_masked(HWIO_EE_n_GSI_CH_k_CNTXT_3_ADDR(n,k), HWIO_EE_n_GSI_CH_k_CNTXT_3_RMSK)
#define HWIO_EE_n_GSI_CH_k_CNTXT_3_INMI2(n,k,mask)    \
        in_dword_masked(HWIO_EE_n_GSI_CH_k_CNTXT_3_ADDR(n,k), mask)
#define HWIO_EE_n_GSI_CH_k_CNTXT_3_OUTI2(n,k,val)    \
        out_dword(HWIO_EE_n_GSI_CH_k_CNTXT_3_ADDR(n,k),val)
#define HWIO_EE_n_GSI_CH_k_CNTXT_3_OUTMI2(n,k,mask,val) \
        out_dword_masked_ns(HWIO_EE_n_GSI_CH_k_CNTXT_3_ADDR(n,k),mask,val,HWIO_EE_n_GSI_CH_k_CNTXT_3_INI2(n,k))
#define HWIO_EE_n_GSI_CH_k_CNTXT_3_R_BASE_ADDR_MSBS_BMSK                                                 0xffffffff
#define HWIO_EE_n_GSI_CH_k_CNTXT_3_R_BASE_ADDR_MSBS_SHFT                                                        0x0

#define HWIO_EE_n_GSI_CH_k_CNTXT_4_ADDR(n,k)                                                             (GSI_REG_BASE      + 0x00014010 + 0x12000 * (n) + 0x80 * (k))
#define HWIO_EE_n_GSI_CH_k_CNTXT_4_PHYS(n,k)                                                             (GSI_REG_BASE_PHYS + 0x00014010 + 0x12000 * (n) + 0x80 * (k))
#define HWIO_EE_n_GSI_CH_k_CNTXT_4_OFFS(n,k)                                                             (GSI_REG_BASE_OFFS + 0x00014010 + 0x12000 * (n) + 0x80 * (k))
#define HWIO_EE_n_GSI_CH_k_CNTXT_4_RMSK                                                                  0xffffffff
#define HWIO_EE_n_GSI_CH_k_CNTXT_4_MAXn                                                                           2
#define HWIO_EE_n_GSI_CH_k_CNTXT_4_MAXk                                                                          27
#define HWIO_EE_n_GSI_CH_k_CNTXT_4_ATTR                                                                         0x3
#define HWIO_EE_n_GSI_CH_k_CNTXT_4_INI2(n,k)        \
        in_dword_masked(HWIO_EE_n_GSI_CH_k_CNTXT_4_ADDR(n,k), HWIO_EE_n_GSI_CH_k_CNTXT_4_RMSK)
#define HWIO_EE_n_GSI_CH_k_CNTXT_4_INMI2(n,k,mask)    \
        in_dword_masked(HWIO_EE_n_GSI_CH_k_CNTXT_4_ADDR(n,k), mask)
#define HWIO_EE_n_GSI_CH_k_CNTXT_4_OUTI2(n,k,val)    \
        out_dword(HWIO_EE_n_GSI_CH_k_CNTXT_4_ADDR(n,k),val)
#define HWIO_EE_n_GSI_CH_k_CNTXT_4_OUTMI2(n,k,mask,val) \
        out_dword_masked_ns(HWIO_EE_n_GSI_CH_k_CNTXT_4_ADDR(n,k),mask,val,HWIO_EE_n_GSI_CH_k_CNTXT_4_INI2(n,k))
#define HWIO_EE_n_GSI_CH_k_CNTXT_4_READ_PTR_LSB_BMSK                                                     0xffffffff
#define HWIO_EE_n_GSI_CH_k_CNTXT_4_READ_PTR_LSB_SHFT                                                            0x0

#define HWIO_EE_n_GSI_CH_k_CNTXT_5_ADDR(n,k)                                                             (GSI_REG_BASE      + 0x00014014 + 0x12000 * (n) + 0x80 * (k))
#define HWIO_EE_n_GSI_CH_k_CNTXT_5_PHYS(n,k)                                                             (GSI_REG_BASE_PHYS + 0x00014014 + 0x12000 * (n) + 0x80 * (k))
#define HWIO_EE_n_GSI_CH_k_CNTXT_5_OFFS(n,k)                                                             (GSI_REG_BASE_OFFS + 0x00014014 + 0x12000 * (n) + 0x80 * (k))
#define HWIO_EE_n_GSI_CH_k_CNTXT_5_RMSK                                                                  0xffffffff
#define HWIO_EE_n_GSI_CH_k_CNTXT_5_MAXn                                                                           2
#define HWIO_EE_n_GSI_CH_k_CNTXT_5_MAXk                                                                          27
#define HWIO_EE_n_GSI_CH_k_CNTXT_5_ATTR                                                                         0x1
#define HWIO_EE_n_GSI_CH_k_CNTXT_5_INI2(n,k)        \
        in_dword_masked(HWIO_EE_n_GSI_CH_k_CNTXT_5_ADDR(n,k), HWIO_EE_n_GSI_CH_k_CNTXT_5_RMSK)
#define HWIO_EE_n_GSI_CH_k_CNTXT_5_INMI2(n,k,mask)    \
        in_dword_masked(HWIO_EE_n_GSI_CH_k_CNTXT_5_ADDR(n,k), mask)
#define HWIO_EE_n_GSI_CH_k_CNTXT_5_READ_PTR_MSB_BMSK                                                     0xffffffff
#define HWIO_EE_n_GSI_CH_k_CNTXT_5_READ_PTR_MSB_SHFT                                                            0x0

#define HWIO_EE_n_GSI_CH_k_CNTXT_6_ADDR(n,k)                                                             (GSI_REG_BASE      + 0x00014018 + 0x12000 * (n) + 0x80 * (k))
#define HWIO_EE_n_GSI_CH_k_CNTXT_6_PHYS(n,k)                                                             (GSI_REG_BASE_PHYS + 0x00014018 + 0x12000 * (n) + 0x80 * (k))
#define HWIO_EE_n_GSI_CH_k_CNTXT_6_OFFS(n,k)                                                             (GSI_REG_BASE_OFFS + 0x00014018 + 0x12000 * (n) + 0x80 * (k))
#define HWIO_EE_n_GSI_CH_k_CNTXT_6_RMSK                                                                  0xffffffff
#define HWIO_EE_n_GSI_CH_k_CNTXT_6_MAXn                                                                           2
#define HWIO_EE_n_GSI_CH_k_CNTXT_6_MAXk                                                                          27
#define HWIO_EE_n_GSI_CH_k_CNTXT_6_ATTR                                                                         0x1
#define HWIO_EE_n_GSI_CH_k_CNTXT_6_INI2(n,k)        \
        in_dword_masked(HWIO_EE_n_GSI_CH_k_CNTXT_6_ADDR(n,k), HWIO_EE_n_GSI_CH_k_CNTXT_6_RMSK)
#define HWIO_EE_n_GSI_CH_k_CNTXT_6_INMI2(n,k,mask)    \
        in_dword_masked(HWIO_EE_n_GSI_CH_k_CNTXT_6_ADDR(n,k), mask)
#define HWIO_EE_n_GSI_CH_k_CNTXT_6_WRITE_PTR_LSB_BMSK                                                    0xffffffff
#define HWIO_EE_n_GSI_CH_k_CNTXT_6_WRITE_PTR_LSB_SHFT                                                           0x0

#define HWIO_EE_n_GSI_CH_k_CNTXT_7_ADDR(n,k)                                                             (GSI_REG_BASE      + 0x0001401c + 0x12000 * (n) + 0x80 * (k))
#define HWIO_EE_n_GSI_CH_k_CNTXT_7_PHYS(n,k)                                                             (GSI_REG_BASE_PHYS + 0x0001401c + 0x12000 * (n) + 0x80 * (k))
#define HWIO_EE_n_GSI_CH_k_CNTXT_7_OFFS(n,k)                                                             (GSI_REG_BASE_OFFS + 0x0001401c + 0x12000 * (n) + 0x80 * (k))
#define HWIO_EE_n_GSI_CH_k_CNTXT_7_RMSK                                                                  0xffffffff
#define HWIO_EE_n_GSI_CH_k_CNTXT_7_MAXn                                                                           2
#define HWIO_EE_n_GSI_CH_k_CNTXT_7_MAXk                                                                          27
#define HWIO_EE_n_GSI_CH_k_CNTXT_7_ATTR                                                                         0x1
#define HWIO_EE_n_GSI_CH_k_CNTXT_7_INI2(n,k)        \
        in_dword_masked(HWIO_EE_n_GSI_CH_k_CNTXT_7_ADDR(n,k), HWIO_EE_n_GSI_CH_k_CNTXT_7_RMSK)
#define HWIO_EE_n_GSI_CH_k_CNTXT_7_INMI2(n,k,mask)    \
        in_dword_masked(HWIO_EE_n_GSI_CH_k_CNTXT_7_ADDR(n,k), mask)
#define HWIO_EE_n_GSI_CH_k_CNTXT_7_WRITE_PTR_MSB_BMSK                                                    0xffffffff
#define HWIO_EE_n_GSI_CH_k_CNTXT_7_WRITE_PTR_MSB_SHFT                                                           0x0

#define HWIO_EE_n_GSI_CH_k_CNTXT_8_ADDR(n,k)                                                             (GSI_REG_BASE      + 0x00014020 + 0x12000 * (n) + 0x80 * (k))
#define HWIO_EE_n_GSI_CH_k_CNTXT_8_PHYS(n,k)                                                             (GSI_REG_BASE_PHYS + 0x00014020 + 0x12000 * (n) + 0x80 * (k))
#define HWIO_EE_n_GSI_CH_k_CNTXT_8_OFFS(n,k)                                                             (GSI_REG_BASE_OFFS + 0x00014020 + 0x12000 * (n) + 0x80 * (k))
#define HWIO_EE_n_GSI_CH_k_CNTXT_8_RMSK                                                                  0xffffffff
#define HWIO_EE_n_GSI_CH_k_CNTXT_8_MAXn                                                                           2
#define HWIO_EE_n_GSI_CH_k_CNTXT_8_MAXk                                                                          27
#define HWIO_EE_n_GSI_CH_k_CNTXT_8_ATTR                                                                         0x3
#define HWIO_EE_n_GSI_CH_k_CNTXT_8_INI2(n,k)        \
        in_dword_masked(HWIO_EE_n_GSI_CH_k_CNTXT_8_ADDR(n,k), HWIO_EE_n_GSI_CH_k_CNTXT_8_RMSK)
#define HWIO_EE_n_GSI_CH_k_CNTXT_8_INMI2(n,k,mask)    \
        in_dword_masked(HWIO_EE_n_GSI_CH_k_CNTXT_8_ADDR(n,k), mask)
#define HWIO_EE_n_GSI_CH_k_CNTXT_8_OUTI2(n,k,val)    \
        out_dword(HWIO_EE_n_GSI_CH_k_CNTXT_8_ADDR(n,k),val)
#define HWIO_EE_n_GSI_CH_k_CNTXT_8_OUTMI2(n,k,mask,val) \
        out_dword_masked_ns(HWIO_EE_n_GSI_CH_k_CNTXT_8_ADDR(n,k),mask,val,HWIO_EE_n_GSI_CH_k_CNTXT_8_INI2(n,k))
#define HWIO_EE_n_GSI_CH_k_CNTXT_8_DB_MSI_DATA_BMSK                                                      0xffffffff
#define HWIO_EE_n_GSI_CH_k_CNTXT_8_DB_MSI_DATA_SHFT                                                             0x0

#define HWIO_EE_n_GSI_CH_k_ELEM_SIZE_SHIFT_ADDR(n,k)                                                     (GSI_REG_BASE      + 0x00014024 + 0x12000 * (n) + 0x80 * (k))
#define HWIO_EE_n_GSI_CH_k_ELEM_SIZE_SHIFT_PHYS(n,k)                                                     (GSI_REG_BASE_PHYS + 0x00014024 + 0x12000 * (n) + 0x80 * (k))
#define HWIO_EE_n_GSI_CH_k_ELEM_SIZE_SHIFT_OFFS(n,k)                                                     (GSI_REG_BASE_OFFS + 0x00014024 + 0x12000 * (n) + 0x80 * (k))
#define HWIO_EE_n_GSI_CH_k_ELEM_SIZE_SHIFT_RMSK                                                                 0xf
#define HWIO_EE_n_GSI_CH_k_ELEM_SIZE_SHIFT_MAXn                                                                   2
#define HWIO_EE_n_GSI_CH_k_ELEM_SIZE_SHIFT_MAXk                                                                  27
#define HWIO_EE_n_GSI_CH_k_ELEM_SIZE_SHIFT_ATTR                                                                 0x1
#define HWIO_EE_n_GSI_CH_k_ELEM_SIZE_SHIFT_INI2(n,k)        \
        in_dword_masked(HWIO_EE_n_GSI_CH_k_ELEM_SIZE_SHIFT_ADDR(n,k), HWIO_EE_n_GSI_CH_k_ELEM_SIZE_SHIFT_RMSK)
#define HWIO_EE_n_GSI_CH_k_ELEM_SIZE_SHIFT_INMI2(n,k,mask)    \
        in_dword_masked(HWIO_EE_n_GSI_CH_k_ELEM_SIZE_SHIFT_ADDR(n,k), mask)
#define HWIO_EE_n_GSI_CH_k_ELEM_SIZE_SHIFT_ELEM_SIZE_SHIFT_BMSK                                                 0xf
#define HWIO_EE_n_GSI_CH_k_ELEM_SIZE_SHIFT_ELEM_SIZE_SHIFT_SHFT                                                 0x0
#define HWIO_EE_n_GSI_CH_k_ELEM_SIZE_SHIFT_ELEM_SIZE_SHIFT_TWO_FVAL                                             0x0
#define HWIO_EE_n_GSI_CH_k_ELEM_SIZE_SHIFT_ELEM_SIZE_SHIFT_THREE_FVAL                                           0x1
#define HWIO_EE_n_GSI_CH_k_ELEM_SIZE_SHIFT_ELEM_SIZE_SHIFT_FOUR_FVAL                                            0x2
#define HWIO_EE_n_GSI_CH_k_ELEM_SIZE_SHIFT_ELEM_SIZE_SHIFT_FIVE_FVAL                                            0x3
#define HWIO_EE_n_GSI_CH_k_ELEM_SIZE_SHIFT_ELEM_SIZE_SHIFT_SIX_FVAL                                             0x4

#define HWIO_EE_n_GSI_CH_k_CH_ALMST_EMPTY_THRSHOLD_ADDR(n,k)                                             (GSI_REG_BASE      + 0x00014028 + 0x12000 * (n) + 0x80 * (k))
#define HWIO_EE_n_GSI_CH_k_CH_ALMST_EMPTY_THRSHOLD_PHYS(n,k)                                             (GSI_REG_BASE_PHYS + 0x00014028 + 0x12000 * (n) + 0x80 * (k))
#define HWIO_EE_n_GSI_CH_k_CH_ALMST_EMPTY_THRSHOLD_OFFS(n,k)                                             (GSI_REG_BASE_OFFS + 0x00014028 + 0x12000 * (n) + 0x80 * (k))
#define HWIO_EE_n_GSI_CH_k_CH_ALMST_EMPTY_THRSHOLD_RMSK                                                      0xffff
#define HWIO_EE_n_GSI_CH_k_CH_ALMST_EMPTY_THRSHOLD_MAXn                                                           2
#define HWIO_EE_n_GSI_CH_k_CH_ALMST_EMPTY_THRSHOLD_MAXk                                                          27
#define HWIO_EE_n_GSI_CH_k_CH_ALMST_EMPTY_THRSHOLD_ATTR                                                         0x3
#define HWIO_EE_n_GSI_CH_k_CH_ALMST_EMPTY_THRSHOLD_INI2(n,k)        \
        in_dword_masked(HWIO_EE_n_GSI_CH_k_CH_ALMST_EMPTY_THRSHOLD_ADDR(n,k), HWIO_EE_n_GSI_CH_k_CH_ALMST_EMPTY_THRSHOLD_RMSK)
#define HWIO_EE_n_GSI_CH_k_CH_ALMST_EMPTY_THRSHOLD_INMI2(n,k,mask)    \
        in_dword_masked(HWIO_EE_n_GSI_CH_k_CH_ALMST_EMPTY_THRSHOLD_ADDR(n,k), mask)
#define HWIO_EE_n_GSI_CH_k_CH_ALMST_EMPTY_THRSHOLD_OUTI2(n,k,val)    \
        out_dword(HWIO_EE_n_GSI_CH_k_CH_ALMST_EMPTY_THRSHOLD_ADDR(n,k),val)
#define HWIO_EE_n_GSI_CH_k_CH_ALMST_EMPTY_THRSHOLD_OUTMI2(n,k,mask,val) \
        out_dword_masked_ns(HWIO_EE_n_GSI_CH_k_CH_ALMST_EMPTY_THRSHOLD_ADDR(n,k),mask,val,HWIO_EE_n_GSI_CH_k_CH_ALMST_EMPTY_THRSHOLD_INI2(n,k))
#define HWIO_EE_n_GSI_CH_k_CH_ALMST_EMPTY_THRSHOLD_CH_ALMST_EMPTY_THRSHOLD_BMSK                              0xffff
#define HWIO_EE_n_GSI_CH_k_CH_ALMST_EMPTY_THRSHOLD_CH_ALMST_EMPTY_THRSHOLD_SHFT                                 0x0

#define HWIO_EE_n_GSI_CH_k_RE_FETCH_READ_PTR_ADDR(n,k)                                                   (GSI_REG_BASE      + 0x00014040 + 0x12000 * (n) + 0x80 * (k))
#define HWIO_EE_n_GSI_CH_k_RE_FETCH_READ_PTR_PHYS(n,k)                                                   (GSI_REG_BASE_PHYS + 0x00014040 + 0x12000 * (n) + 0x80 * (k))
#define HWIO_EE_n_GSI_CH_k_RE_FETCH_READ_PTR_OFFS(n,k)                                                   (GSI_REG_BASE_OFFS + 0x00014040 + 0x12000 * (n) + 0x80 * (k))
#define HWIO_EE_n_GSI_CH_k_RE_FETCH_READ_PTR_RMSK                                                          0xffffff
#define HWIO_EE_n_GSI_CH_k_RE_FETCH_READ_PTR_MAXn                                                                 2
#define HWIO_EE_n_GSI_CH_k_RE_FETCH_READ_PTR_MAXk                                                                27
#define HWIO_EE_n_GSI_CH_k_RE_FETCH_READ_PTR_ATTR                                                               0x3
#define HWIO_EE_n_GSI_CH_k_RE_FETCH_READ_PTR_INI2(n,k)        \
        in_dword_masked(HWIO_EE_n_GSI_CH_k_RE_FETCH_READ_PTR_ADDR(n,k), HWIO_EE_n_GSI_CH_k_RE_FETCH_READ_PTR_RMSK)
#define HWIO_EE_n_GSI_CH_k_RE_FETCH_READ_PTR_INMI2(n,k,mask)    \
        in_dword_masked(HWIO_EE_n_GSI_CH_k_RE_FETCH_READ_PTR_ADDR(n,k), mask)
#define HWIO_EE_n_GSI_CH_k_RE_FETCH_READ_PTR_OUTI2(n,k,val)    \
        out_dword(HWIO_EE_n_GSI_CH_k_RE_FETCH_READ_PTR_ADDR(n,k),val)
#define HWIO_EE_n_GSI_CH_k_RE_FETCH_READ_PTR_OUTMI2(n,k,mask,val) \
        out_dword_masked_ns(HWIO_EE_n_GSI_CH_k_RE_FETCH_READ_PTR_ADDR(n,k),mask,val,HWIO_EE_n_GSI_CH_k_RE_FETCH_READ_PTR_INI2(n,k))
#define HWIO_EE_n_GSI_CH_k_RE_FETCH_READ_PTR_READ_PTR_BMSK                                                 0xffffff
#define HWIO_EE_n_GSI_CH_k_RE_FETCH_READ_PTR_READ_PTR_SHFT                                                      0x0

#define HWIO_EE_n_GSI_CH_k_RE_FETCH_WRITE_PTR_ADDR(n,k)                                                  (GSI_REG_BASE      + 0x00014044 + 0x12000 * (n) + 0x80 * (k))
#define HWIO_EE_n_GSI_CH_k_RE_FETCH_WRITE_PTR_PHYS(n,k)                                                  (GSI_REG_BASE_PHYS + 0x00014044 + 0x12000 * (n) + 0x80 * (k))
#define HWIO_EE_n_GSI_CH_k_RE_FETCH_WRITE_PTR_OFFS(n,k)                                                  (GSI_REG_BASE_OFFS + 0x00014044 + 0x12000 * (n) + 0x80 * (k))
#define HWIO_EE_n_GSI_CH_k_RE_FETCH_WRITE_PTR_RMSK                                                         0xffffff
#define HWIO_EE_n_GSI_CH_k_RE_FETCH_WRITE_PTR_MAXn                                                                2
#define HWIO_EE_n_GSI_CH_k_RE_FETCH_WRITE_PTR_MAXk                                                               27
#define HWIO_EE_n_GSI_CH_k_RE_FETCH_WRITE_PTR_ATTR                                                              0x3
#define HWIO_EE_n_GSI_CH_k_RE_FETCH_WRITE_PTR_INI2(n,k)        \
        in_dword_masked(HWIO_EE_n_GSI_CH_k_RE_FETCH_WRITE_PTR_ADDR(n,k), HWIO_EE_n_GSI_CH_k_RE_FETCH_WRITE_PTR_RMSK)
#define HWIO_EE_n_GSI_CH_k_RE_FETCH_WRITE_PTR_INMI2(n,k,mask)    \
        in_dword_masked(HWIO_EE_n_GSI_CH_k_RE_FETCH_WRITE_PTR_ADDR(n,k), mask)
#define HWIO_EE_n_GSI_CH_k_RE_FETCH_WRITE_PTR_OUTI2(n,k,val)    \
        out_dword(HWIO_EE_n_GSI_CH_k_RE_FETCH_WRITE_PTR_ADDR(n,k),val)
#define HWIO_EE_n_GSI_CH_k_RE_FETCH_WRITE_PTR_OUTMI2(n,k,mask,val) \
        out_dword_masked_ns(HWIO_EE_n_GSI_CH_k_RE_FETCH_WRITE_PTR_ADDR(n,k),mask,val,HWIO_EE_n_GSI_CH_k_RE_FETCH_WRITE_PTR_INI2(n,k))
#define HWIO_EE_n_GSI_CH_k_RE_FETCH_WRITE_PTR_RE_INTR_DB_BMSK                                              0xffffff
#define HWIO_EE_n_GSI_CH_k_RE_FETCH_WRITE_PTR_RE_INTR_DB_SHFT                                                   0x0

#define HWIO_EE_n_GSI_CH_k_QOS_ADDR(n,k)                                                                 (GSI_REG_BASE      + 0x00014048 + 0x12000 * (n) + 0x80 * (k))
#define HWIO_EE_n_GSI_CH_k_QOS_PHYS(n,k)                                                                 (GSI_REG_BASE_PHYS + 0x00014048 + 0x12000 * (n) + 0x80 * (k))
#define HWIO_EE_n_GSI_CH_k_QOS_OFFS(n,k)                                                                 (GSI_REG_BASE_OFFS + 0x00014048 + 0x12000 * (n) + 0x80 * (k))
#define HWIO_EE_n_GSI_CH_k_QOS_RMSK                                                                       0x3ff3f0f
#define HWIO_EE_n_GSI_CH_k_QOS_MAXn                                                                               2
#define HWIO_EE_n_GSI_CH_k_QOS_MAXk                                                                              27
#define HWIO_EE_n_GSI_CH_k_QOS_ATTR                                                                             0x3
#define HWIO_EE_n_GSI_CH_k_QOS_INI2(n,k)        \
        in_dword_masked(HWIO_EE_n_GSI_CH_k_QOS_ADDR(n,k), HWIO_EE_n_GSI_CH_k_QOS_RMSK)
#define HWIO_EE_n_GSI_CH_k_QOS_INMI2(n,k,mask)    \
        in_dword_masked(HWIO_EE_n_GSI_CH_k_QOS_ADDR(n,k), mask)
#define HWIO_EE_n_GSI_CH_k_QOS_OUTI2(n,k,val)    \
        out_dword(HWIO_EE_n_GSI_CH_k_QOS_ADDR(n,k),val)
#define HWIO_EE_n_GSI_CH_k_QOS_OUTMI2(n,k,mask,val) \
        out_dword_masked_ns(HWIO_EE_n_GSI_CH_k_QOS_ADDR(n,k),mask,val,HWIO_EE_n_GSI_CH_k_QOS_INI2(n,k))
#define HWIO_EE_n_GSI_CH_k_QOS_LOW_LATENCY_EN_BMSK                                                        0x2000000
#define HWIO_EE_n_GSI_CH_k_QOS_LOW_LATENCY_EN_SHFT                                                             0x19
#define HWIO_EE_n_GSI_CH_k_QOS_DB_IN_BYTES_BMSK                                                           0x1000000
#define HWIO_EE_n_GSI_CH_k_QOS_DB_IN_BYTES_SHFT                                                                0x18
#define HWIO_EE_n_GSI_CH_k_QOS_EMPTY_LVL_THRSHOLD_BMSK                                                     0xff0000
#define HWIO_EE_n_GSI_CH_k_QOS_EMPTY_LVL_THRSHOLD_SHFT                                                         0x10
#define HWIO_EE_n_GSI_CH_k_QOS_PREFETCH_MODE_BMSK                                                            0x3c00
#define HWIO_EE_n_GSI_CH_k_QOS_PREFETCH_MODE_SHFT                                                               0xa
#define HWIO_EE_n_GSI_CH_k_QOS_PREFETCH_MODE_USE_PREFETCH_BUFS_FVAL                                             0x0
#define HWIO_EE_n_GSI_CH_k_QOS_PREFETCH_MODE_ESCAPE_BUF_ONLY_FVAL                                               0x1
#define HWIO_EE_n_GSI_CH_k_QOS_PREFETCH_MODE_SMART_PRE_FETCH_FVAL                                               0x2
#define HWIO_EE_n_GSI_CH_k_QOS_PREFETCH_MODE_FREE_PRE_FETCH_FVAL                                                0x3
#define HWIO_EE_n_GSI_CH_k_QOS_USE_DB_ENG_BMSK                                                                0x200
#define HWIO_EE_n_GSI_CH_k_QOS_USE_DB_ENG_SHFT                                                                  0x9
#define HWIO_EE_n_GSI_CH_k_QOS_MAX_PREFETCH_BMSK                                                              0x100
#define HWIO_EE_n_GSI_CH_k_QOS_MAX_PREFETCH_SHFT                                                                0x8
#define HWIO_EE_n_GSI_CH_k_QOS_MAX_PREFETCH_ONE_PREFETCH_SEG_FVAL                                               0x0
#define HWIO_EE_n_GSI_CH_k_QOS_MAX_PREFETCH_TWO_PREFETCH_SEG_FVAL                                               0x1
#define HWIO_EE_n_GSI_CH_k_QOS_WRR_WEIGHT_BMSK                                                                  0xf
#define HWIO_EE_n_GSI_CH_k_QOS_WRR_WEIGHT_SHFT                                                                  0x0

#define HWIO_EE_n_GSI_CH_k_SCRATCH_0_ADDR(n,k)                                                           (GSI_REG_BASE      + 0x0001404c + 0x12000 * (n) + 0x80 * (k))
#define HWIO_EE_n_GSI_CH_k_SCRATCH_0_PHYS(n,k)                                                           (GSI_REG_BASE_PHYS + 0x0001404c + 0x12000 * (n) + 0x80 * (k))
#define HWIO_EE_n_GSI_CH_k_SCRATCH_0_OFFS(n,k)                                                           (GSI_REG_BASE_OFFS + 0x0001404c + 0x12000 * (n) + 0x80 * (k))
#define HWIO_EE_n_GSI_CH_k_SCRATCH_0_RMSK                                                                0xffffffff
#define HWIO_EE_n_GSI_CH_k_SCRATCH_0_MAXn                                                                         2
#define HWIO_EE_n_GSI_CH_k_SCRATCH_0_MAXk                                                                        27
#define HWIO_EE_n_GSI_CH_k_SCRATCH_0_ATTR                                                                       0x3
#define HWIO_EE_n_GSI_CH_k_SCRATCH_0_INI2(n,k)        \
        in_dword_masked(HWIO_EE_n_GSI_CH_k_SCRATCH_0_ADDR(n,k), HWIO_EE_n_GSI_CH_k_SCRATCH_0_RMSK)
#define HWIO_EE_n_GSI_CH_k_SCRATCH_0_INMI2(n,k,mask)    \
        in_dword_masked(HWIO_EE_n_GSI_CH_k_SCRATCH_0_ADDR(n,k), mask)
#define HWIO_EE_n_GSI_CH_k_SCRATCH_0_OUTI2(n,k,val)    \
        out_dword(HWIO_EE_n_GSI_CH_k_SCRATCH_0_ADDR(n,k),val)
#define HWIO_EE_n_GSI_CH_k_SCRATCH_0_OUTMI2(n,k,mask,val) \
        out_dword_masked_ns(HWIO_EE_n_GSI_CH_k_SCRATCH_0_ADDR(n,k),mask,val,HWIO_EE_n_GSI_CH_k_SCRATCH_0_INI2(n,k))
#define HWIO_EE_n_GSI_CH_k_SCRATCH_0_SCRATCH_BMSK                                                        0xffffffff
#define HWIO_EE_n_GSI_CH_k_SCRATCH_0_SCRATCH_SHFT                                                               0x0

#define HWIO_EE_n_GSI_CH_k_SCRATCH_1_ADDR(n,k)                                                           (GSI_REG_BASE      + 0x00014050 + 0x12000 * (n) + 0x80 * (k))
#define HWIO_EE_n_GSI_CH_k_SCRATCH_1_PHYS(n,k)                                                           (GSI_REG_BASE_PHYS + 0x00014050 + 0x12000 * (n) + 0x80 * (k))
#define HWIO_EE_n_GSI_CH_k_SCRATCH_1_OFFS(n,k)                                                           (GSI_REG_BASE_OFFS + 0x00014050 + 0x12000 * (n) + 0x80 * (k))
#define HWIO_EE_n_GSI_CH_k_SCRATCH_1_RMSK                                                                0xffffffff
#define HWIO_EE_n_GSI_CH_k_SCRATCH_1_MAXn                                                                         2
#define HWIO_EE_n_GSI_CH_k_SCRATCH_1_MAXk                                                                        27
#define HWIO_EE_n_GSI_CH_k_SCRATCH_1_ATTR                                                                       0x3
#define HWIO_EE_n_GSI_CH_k_SCRATCH_1_INI2(n,k)        \
        in_dword_masked(HWIO_EE_n_GSI_CH_k_SCRATCH_1_ADDR(n,k), HWIO_EE_n_GSI_CH_k_SCRATCH_1_RMSK)
#define HWIO_EE_n_GSI_CH_k_SCRATCH_1_INMI2(n,k,mask)    \
        in_dword_masked(HWIO_EE_n_GSI_CH_k_SCRATCH_1_ADDR(n,k), mask)
#define HWIO_EE_n_GSI_CH_k_SCRATCH_1_OUTI2(n,k,val)    \
        out_dword(HWIO_EE_n_GSI_CH_k_SCRATCH_1_ADDR(n,k),val)
#define HWIO_EE_n_GSI_CH_k_SCRATCH_1_OUTMI2(n,k,mask,val) \
        out_dword_masked_ns(HWIO_EE_n_GSI_CH_k_SCRATCH_1_ADDR(n,k),mask,val,HWIO_EE_n_GSI_CH_k_SCRATCH_1_INI2(n,k))
#define HWIO_EE_n_GSI_CH_k_SCRATCH_1_SCRATCH_BMSK                                                        0xffffffff
#define HWIO_EE_n_GSI_CH_k_SCRATCH_1_SCRATCH_SHFT                                                               0x0

#define HWIO_EE_n_GSI_CH_k_SCRATCH_2_ADDR(n,k)                                                           (GSI_REG_BASE      + 0x00014054 + 0x12000 * (n) + 0x80 * (k))
#define HWIO_EE_n_GSI_CH_k_SCRATCH_2_PHYS(n,k)                                                           (GSI_REG_BASE_PHYS + 0x00014054 + 0x12000 * (n) + 0x80 * (k))
#define HWIO_EE_n_GSI_CH_k_SCRATCH_2_OFFS(n,k)                                                           (GSI_REG_BASE_OFFS + 0x00014054 + 0x12000 * (n) + 0x80 * (k))
#define HWIO_EE_n_GSI_CH_k_SCRATCH_2_RMSK                                                                0xffffffff
#define HWIO_EE_n_GSI_CH_k_SCRATCH_2_MAXn                                                                         2
#define HWIO_EE_n_GSI_CH_k_SCRATCH_2_MAXk                                                                        27
#define HWIO_EE_n_GSI_CH_k_SCRATCH_2_ATTR                                                                       0x3
#define HWIO_EE_n_GSI_CH_k_SCRATCH_2_INI2(n,k)        \
        in_dword_masked(HWIO_EE_n_GSI_CH_k_SCRATCH_2_ADDR(n,k), HWIO_EE_n_GSI_CH_k_SCRATCH_2_RMSK)
#define HWIO_EE_n_GSI_CH_k_SCRATCH_2_INMI2(n,k,mask)    \
        in_dword_masked(HWIO_EE_n_GSI_CH_k_SCRATCH_2_ADDR(n,k), mask)
#define HWIO_EE_n_GSI_CH_k_SCRATCH_2_OUTI2(n,k,val)    \
        out_dword(HWIO_EE_n_GSI_CH_k_SCRATCH_2_ADDR(n,k),val)
#define HWIO_EE_n_GSI_CH_k_SCRATCH_2_OUTMI2(n,k,mask,val) \
        out_dword_masked_ns(HWIO_EE_n_GSI_CH_k_SCRATCH_2_ADDR(n,k),mask,val,HWIO_EE_n_GSI_CH_k_SCRATCH_2_INI2(n,k))
#define HWIO_EE_n_GSI_CH_k_SCRATCH_2_SCRATCH_BMSK                                                        0xffffffff
#define HWIO_EE_n_GSI_CH_k_SCRATCH_2_SCRATCH_SHFT                                                               0x0

#define HWIO_EE_n_GSI_CH_k_SCRATCH_3_ADDR(n,k)                                                           (GSI_REG_BASE      + 0x00014058 + 0x12000 * (n) + 0x80 * (k))
#define HWIO_EE_n_GSI_CH_k_SCRATCH_3_PHYS(n,k)                                                           (GSI_REG_BASE_PHYS + 0x00014058 + 0x12000 * (n) + 0x80 * (k))
#define HWIO_EE_n_GSI_CH_k_SCRATCH_3_OFFS(n,k)                                                           (GSI_REG_BASE_OFFS + 0x00014058 + 0x12000 * (n) + 0x80 * (k))
#define HWIO_EE_n_GSI_CH_k_SCRATCH_3_RMSK                                                                0xffffffff
#define HWIO_EE_n_GSI_CH_k_SCRATCH_3_MAXn                                                                         2
#define HWIO_EE_n_GSI_CH_k_SCRATCH_3_MAXk                                                                        27
#define HWIO_EE_n_GSI_CH_k_SCRATCH_3_ATTR                                                                       0x3
#define HWIO_EE_n_GSI_CH_k_SCRATCH_3_INI2(n,k)        \
        in_dword_masked(HWIO_EE_n_GSI_CH_k_SCRATCH_3_ADDR(n,k), HWIO_EE_n_GSI_CH_k_SCRATCH_3_RMSK)
#define HWIO_EE_n_GSI_CH_k_SCRATCH_3_INMI2(n,k,mask)    \
        in_dword_masked(HWIO_EE_n_GSI_CH_k_SCRATCH_3_ADDR(n,k), mask)
#define HWIO_EE_n_GSI_CH_k_SCRATCH_3_OUTI2(n,k,val)    \
        out_dword(HWIO_EE_n_GSI_CH_k_SCRATCH_3_ADDR(n,k),val)
#define HWIO_EE_n_GSI_CH_k_SCRATCH_3_OUTMI2(n,k,mask,val) \
        out_dword_masked_ns(HWIO_EE_n_GSI_CH_k_SCRATCH_3_ADDR(n,k),mask,val,HWIO_EE_n_GSI_CH_k_SCRATCH_3_INI2(n,k))
#define HWIO_EE_n_GSI_CH_k_SCRATCH_3_SCRATCH_BMSK                                                        0xffffffff
#define HWIO_EE_n_GSI_CH_k_SCRATCH_3_SCRATCH_SHFT                                                               0x0

#define HWIO_EE_n_GSI_CH_k_SCRATCH_4_ADDR(n,k)                                                           (GSI_REG_BASE      + 0x0001405c + 0x12000 * (n) + 0x80 * (k))
#define HWIO_EE_n_GSI_CH_k_SCRATCH_4_PHYS(n,k)                                                           (GSI_REG_BASE_PHYS + 0x0001405c + 0x12000 * (n) + 0x80 * (k))
#define HWIO_EE_n_GSI_CH_k_SCRATCH_4_OFFS(n,k)                                                           (GSI_REG_BASE_OFFS + 0x0001405c + 0x12000 * (n) + 0x80 * (k))
#define HWIO_EE_n_GSI_CH_k_SCRATCH_4_RMSK                                                                0xffffffff
#define HWIO_EE_n_GSI_CH_k_SCRATCH_4_MAXn                                                                         2
#define HWIO_EE_n_GSI_CH_k_SCRATCH_4_MAXk                                                                        27
#define HWIO_EE_n_GSI_CH_k_SCRATCH_4_ATTR                                                                       0x3
#define HWIO_EE_n_GSI_CH_k_SCRATCH_4_INI2(n,k)        \
        in_dword_masked(HWIO_EE_n_GSI_CH_k_SCRATCH_4_ADDR(n,k), HWIO_EE_n_GSI_CH_k_SCRATCH_4_RMSK)
#define HWIO_EE_n_GSI_CH_k_SCRATCH_4_INMI2(n,k,mask)    \
        in_dword_masked(HWIO_EE_n_GSI_CH_k_SCRATCH_4_ADDR(n,k), mask)
#define HWIO_EE_n_GSI_CH_k_SCRATCH_4_OUTI2(n,k,val)    \
        out_dword(HWIO_EE_n_GSI_CH_k_SCRATCH_4_ADDR(n,k),val)
#define HWIO_EE_n_GSI_CH_k_SCRATCH_4_OUTMI2(n,k,mask,val) \
        out_dword_masked_ns(HWIO_EE_n_GSI_CH_k_SCRATCH_4_ADDR(n,k),mask,val,HWIO_EE_n_GSI_CH_k_SCRATCH_4_INI2(n,k))
#define HWIO_EE_n_GSI_CH_k_SCRATCH_4_SCRATCH_BMSK                                                        0xffffffff
#define HWIO_EE_n_GSI_CH_k_SCRATCH_4_SCRATCH_SHFT                                                               0x0

#define HWIO_EE_n_GSI_CH_k_SCRATCH_5_ADDR(n,k)                                                           (GSI_REG_BASE      + 0x00014060 + 0x12000 * (n) + 0x80 * (k))
#define HWIO_EE_n_GSI_CH_k_SCRATCH_5_PHYS(n,k)                                                           (GSI_REG_BASE_PHYS + 0x00014060 + 0x12000 * (n) + 0x80 * (k))
#define HWIO_EE_n_GSI_CH_k_SCRATCH_5_OFFS(n,k)                                                           (GSI_REG_BASE_OFFS + 0x00014060 + 0x12000 * (n) + 0x80 * (k))
#define HWIO_EE_n_GSI_CH_k_SCRATCH_5_RMSK                                                                0xffffffff
#define HWIO_EE_n_GSI_CH_k_SCRATCH_5_MAXn                                                                         2
#define HWIO_EE_n_GSI_CH_k_SCRATCH_5_MAXk                                                                        27
#define HWIO_EE_n_GSI_CH_k_SCRATCH_5_ATTR                                                                       0x3
#define HWIO_EE_n_GSI_CH_k_SCRATCH_5_INI2(n,k)        \
        in_dword_masked(HWIO_EE_n_GSI_CH_k_SCRATCH_5_ADDR(n,k), HWIO_EE_n_GSI_CH_k_SCRATCH_5_RMSK)
#define HWIO_EE_n_GSI_CH_k_SCRATCH_5_INMI2(n,k,mask)    \
        in_dword_masked(HWIO_EE_n_GSI_CH_k_SCRATCH_5_ADDR(n,k), mask)
#define HWIO_EE_n_GSI_CH_k_SCRATCH_5_OUTI2(n,k,val)    \
        out_dword(HWIO_EE_n_GSI_CH_k_SCRATCH_5_ADDR(n,k),val)
#define HWIO_EE_n_GSI_CH_k_SCRATCH_5_OUTMI2(n,k,mask,val) \
        out_dword_masked_ns(HWIO_EE_n_GSI_CH_k_SCRATCH_5_ADDR(n,k),mask,val,HWIO_EE_n_GSI_CH_k_SCRATCH_5_INI2(n,k))
#define HWIO_EE_n_GSI_CH_k_SCRATCH_5_SCRATCH_BMSK                                                        0xffffffff
#define HWIO_EE_n_GSI_CH_k_SCRATCH_5_SCRATCH_SHFT                                                               0x0

#define HWIO_EE_n_GSI_CH_k_SCRATCH_6_ADDR(n,k)                                                           (GSI_REG_BASE      + 0x00014064 + 0x12000 * (n) + 0x80 * (k))
#define HWIO_EE_n_GSI_CH_k_SCRATCH_6_PHYS(n,k)                                                           (GSI_REG_BASE_PHYS + 0x00014064 + 0x12000 * (n) + 0x80 * (k))
#define HWIO_EE_n_GSI_CH_k_SCRATCH_6_OFFS(n,k)                                                           (GSI_REG_BASE_OFFS + 0x00014064 + 0x12000 * (n) + 0x80 * (k))
#define HWIO_EE_n_GSI_CH_k_SCRATCH_6_RMSK                                                                0xffffffff
#define HWIO_EE_n_GSI_CH_k_SCRATCH_6_MAXn                                                                         2
#define HWIO_EE_n_GSI_CH_k_SCRATCH_6_MAXk                                                                        27
#define HWIO_EE_n_GSI_CH_k_SCRATCH_6_ATTR                                                                       0x3
#define HWIO_EE_n_GSI_CH_k_SCRATCH_6_INI2(n,k)        \
        in_dword_masked(HWIO_EE_n_GSI_CH_k_SCRATCH_6_ADDR(n,k), HWIO_EE_n_GSI_CH_k_SCRATCH_6_RMSK)
#define HWIO_EE_n_GSI_CH_k_SCRATCH_6_INMI2(n,k,mask)    \
        in_dword_masked(HWIO_EE_n_GSI_CH_k_SCRATCH_6_ADDR(n,k), mask)
#define HWIO_EE_n_GSI_CH_k_SCRATCH_6_OUTI2(n,k,val)    \
        out_dword(HWIO_EE_n_GSI_CH_k_SCRATCH_6_ADDR(n,k),val)
#define HWIO_EE_n_GSI_CH_k_SCRATCH_6_OUTMI2(n,k,mask,val) \
        out_dword_masked_ns(HWIO_EE_n_GSI_CH_k_SCRATCH_6_ADDR(n,k),mask,val,HWIO_EE_n_GSI_CH_k_SCRATCH_6_INI2(n,k))
#define HWIO_EE_n_GSI_CH_k_SCRATCH_6_SCRATCH_BMSK                                                        0xffffffff
#define HWIO_EE_n_GSI_CH_k_SCRATCH_6_SCRATCH_SHFT                                                               0x0

#define HWIO_EE_n_GSI_CH_k_SCRATCH_7_ADDR(n,k)                                                           (GSI_REG_BASE      + 0x00014068 + 0x12000 * (n) + 0x80 * (k))
#define HWIO_EE_n_GSI_CH_k_SCRATCH_7_PHYS(n,k)                                                           (GSI_REG_BASE_PHYS + 0x00014068 + 0x12000 * (n) + 0x80 * (k))
#define HWIO_EE_n_GSI_CH_k_SCRATCH_7_OFFS(n,k)                                                           (GSI_REG_BASE_OFFS + 0x00014068 + 0x12000 * (n) + 0x80 * (k))
#define HWIO_EE_n_GSI_CH_k_SCRATCH_7_RMSK                                                                0xffffffff
#define HWIO_EE_n_GSI_CH_k_SCRATCH_7_MAXn                                                                         2
#define HWIO_EE_n_GSI_CH_k_SCRATCH_7_MAXk                                                                        27
#define HWIO_EE_n_GSI_CH_k_SCRATCH_7_ATTR                                                                       0x3
#define HWIO_EE_n_GSI_CH_k_SCRATCH_7_INI2(n,k)        \
        in_dword_masked(HWIO_EE_n_GSI_CH_k_SCRATCH_7_ADDR(n,k), HWIO_EE_n_GSI_CH_k_SCRATCH_7_RMSK)
#define HWIO_EE_n_GSI_CH_k_SCRATCH_7_INMI2(n,k,mask)    \
        in_dword_masked(HWIO_EE_n_GSI_CH_k_SCRATCH_7_ADDR(n,k), mask)
#define HWIO_EE_n_GSI_CH_k_SCRATCH_7_OUTI2(n,k,val)    \
        out_dword(HWIO_EE_n_GSI_CH_k_SCRATCH_7_ADDR(n,k),val)
#define HWIO_EE_n_GSI_CH_k_SCRATCH_7_OUTMI2(n,k,mask,val) \
        out_dword_masked_ns(HWIO_EE_n_GSI_CH_k_SCRATCH_7_ADDR(n,k),mask,val,HWIO_EE_n_GSI_CH_k_SCRATCH_7_INI2(n,k))
#define HWIO_EE_n_GSI_CH_k_SCRATCH_7_SCRATCH_BMSK                                                        0xffffffff
#define HWIO_EE_n_GSI_CH_k_SCRATCH_7_SCRATCH_SHFT                                                               0x0

#define HWIO_EE_n_GSI_CH_k_SCRATCH_8_ADDR(n,k)                                                           (GSI_REG_BASE      + 0x0001406c + 0x12000 * (n) + 0x80 * (k))
#define HWIO_EE_n_GSI_CH_k_SCRATCH_8_PHYS(n,k)                                                           (GSI_REG_BASE_PHYS + 0x0001406c + 0x12000 * (n) + 0x80 * (k))
#define HWIO_EE_n_GSI_CH_k_SCRATCH_8_OFFS(n,k)                                                           (GSI_REG_BASE_OFFS + 0x0001406c + 0x12000 * (n) + 0x80 * (k))
#define HWIO_EE_n_GSI_CH_k_SCRATCH_8_RMSK                                                                0xffffffff
#define HWIO_EE_n_GSI_CH_k_SCRATCH_8_MAXn                                                                         2
#define HWIO_EE_n_GSI_CH_k_SCRATCH_8_MAXk                                                                        27
#define HWIO_EE_n_GSI_CH_k_SCRATCH_8_ATTR                                                                       0x3
#define HWIO_EE_n_GSI_CH_k_SCRATCH_8_INI2(n,k)        \
        in_dword_masked(HWIO_EE_n_GSI_CH_k_SCRATCH_8_ADDR(n,k), HWIO_EE_n_GSI_CH_k_SCRATCH_8_RMSK)
#define HWIO_EE_n_GSI_CH_k_SCRATCH_8_INMI2(n,k,mask)    \
        in_dword_masked(HWIO_EE_n_GSI_CH_k_SCRATCH_8_ADDR(n,k), mask)
#define HWIO_EE_n_GSI_CH_k_SCRATCH_8_OUTI2(n,k,val)    \
        out_dword(HWIO_EE_n_GSI_CH_k_SCRATCH_8_ADDR(n,k),val)
#define HWIO_EE_n_GSI_CH_k_SCRATCH_8_OUTMI2(n,k,mask,val) \
        out_dword_masked_ns(HWIO_EE_n_GSI_CH_k_SCRATCH_8_ADDR(n,k),mask,val,HWIO_EE_n_GSI_CH_k_SCRATCH_8_INI2(n,k))
#define HWIO_EE_n_GSI_CH_k_SCRATCH_8_SCRATCH_BMSK                                                        0xffffffff
#define HWIO_EE_n_GSI_CH_k_SCRATCH_8_SCRATCH_SHFT                                                               0x0

#define HWIO_EE_n_GSI_CH_k_SCRATCH_9_ADDR(n,k)                                                           (GSI_REG_BASE      + 0x00014070 + 0x12000 * (n) + 0x80 * (k))
#define HWIO_EE_n_GSI_CH_k_SCRATCH_9_PHYS(n,k)                                                           (GSI_REG_BASE_PHYS + 0x00014070 + 0x12000 * (n) + 0x80 * (k))
#define HWIO_EE_n_GSI_CH_k_SCRATCH_9_OFFS(n,k)                                                           (GSI_REG_BASE_OFFS + 0x00014070 + 0x12000 * (n) + 0x80 * (k))
#define HWIO_EE_n_GSI_CH_k_SCRATCH_9_RMSK                                                                0xffffffff
#define HWIO_EE_n_GSI_CH_k_SCRATCH_9_MAXn                                                                         2
#define HWIO_EE_n_GSI_CH_k_SCRATCH_9_MAXk                                                                        27
#define HWIO_EE_n_GSI_CH_k_SCRATCH_9_ATTR                                                                       0x3
#define HWIO_EE_n_GSI_CH_k_SCRATCH_9_INI2(n,k)        \
        in_dword_masked(HWIO_EE_n_GSI_CH_k_SCRATCH_9_ADDR(n,k), HWIO_EE_n_GSI_CH_k_SCRATCH_9_RMSK)
#define HWIO_EE_n_GSI_CH_k_SCRATCH_9_INMI2(n,k,mask)    \
        in_dword_masked(HWIO_EE_n_GSI_CH_k_SCRATCH_9_ADDR(n,k), mask)
#define HWIO_EE_n_GSI_CH_k_SCRATCH_9_OUTI2(n,k,val)    \
        out_dword(HWIO_EE_n_GSI_CH_k_SCRATCH_9_ADDR(n,k),val)
#define HWIO_EE_n_GSI_CH_k_SCRATCH_9_OUTMI2(n,k,mask,val) \
        out_dword_masked_ns(HWIO_EE_n_GSI_CH_k_SCRATCH_9_ADDR(n,k),mask,val,HWIO_EE_n_GSI_CH_k_SCRATCH_9_INI2(n,k))
#define HWIO_EE_n_GSI_CH_k_SCRATCH_9_SCRATCH_BMSK                                                        0xffffffff
#define HWIO_EE_n_GSI_CH_k_SCRATCH_9_SCRATCH_SHFT                                                               0x0

#define HWIO_EE_n_GSI_CH_k_DB_ENG_WRITE_PTR_ADDR(n,k)                                                    (GSI_REG_BASE      + 0x00014074 + 0x12000 * (n) + 0x80 * (k))
#define HWIO_EE_n_GSI_CH_k_DB_ENG_WRITE_PTR_PHYS(n,k)                                                    (GSI_REG_BASE_PHYS + 0x00014074 + 0x12000 * (n) + 0x80 * (k))
#define HWIO_EE_n_GSI_CH_k_DB_ENG_WRITE_PTR_OFFS(n,k)                                                    (GSI_REG_BASE_OFFS + 0x00014074 + 0x12000 * (n) + 0x80 * (k))
#define HWIO_EE_n_GSI_CH_k_DB_ENG_WRITE_PTR_RMSK                                                             0xffff
#define HWIO_EE_n_GSI_CH_k_DB_ENG_WRITE_PTR_MAXn                                                                  2
#define HWIO_EE_n_GSI_CH_k_DB_ENG_WRITE_PTR_MAXk                                                                 27
#define HWIO_EE_n_GSI_CH_k_DB_ENG_WRITE_PTR_ATTR                                                                0x3
#define HWIO_EE_n_GSI_CH_k_DB_ENG_WRITE_PTR_INI2(n,k)        \
        in_dword_masked(HWIO_EE_n_GSI_CH_k_DB_ENG_WRITE_PTR_ADDR(n,k), HWIO_EE_n_GSI_CH_k_DB_ENG_WRITE_PTR_RMSK)
#define HWIO_EE_n_GSI_CH_k_DB_ENG_WRITE_PTR_INMI2(n,k,mask)    \
        in_dword_masked(HWIO_EE_n_GSI_CH_k_DB_ENG_WRITE_PTR_ADDR(n,k), mask)
#define HWIO_EE_n_GSI_CH_k_DB_ENG_WRITE_PTR_OUTI2(n,k,val)    \
        out_dword(HWIO_EE_n_GSI_CH_k_DB_ENG_WRITE_PTR_ADDR(n,k),val)
#define HWIO_EE_n_GSI_CH_k_DB_ENG_WRITE_PTR_OUTMI2(n,k,mask,val) \
        out_dword_masked_ns(HWIO_EE_n_GSI_CH_k_DB_ENG_WRITE_PTR_ADDR(n,k),mask,val,HWIO_EE_n_GSI_CH_k_DB_ENG_WRITE_PTR_INI2(n,k))
#define HWIO_EE_n_GSI_CH_k_DB_ENG_WRITE_PTR_LAST_DB_2_MCS_BMSK                                               0xffff
#define HWIO_EE_n_GSI_CH_k_DB_ENG_WRITE_PTR_LAST_DB_2_MCS_SHFT                                                  0x0

#define HWIO_EE_n_EV_CH_k_CNTXT_0_ADDR(n,k)                                                              (GSI_REG_BASE      + 0x0001c000 + 0x12000 * (n) + 0x80 * (k))
#define HWIO_EE_n_EV_CH_k_CNTXT_0_PHYS(n,k)                                                              (GSI_REG_BASE_PHYS + 0x0001c000 + 0x12000 * (n) + 0x80 * (k))
#define HWIO_EE_n_EV_CH_k_CNTXT_0_OFFS(n,k)                                                              (GSI_REG_BASE_OFFS + 0x0001c000 + 0x12000 * (n) + 0x80 * (k))
#define HWIO_EE_n_EV_CH_k_CNTXT_0_RMSK                                                                   0xffffffff
#define HWIO_EE_n_EV_CH_k_CNTXT_0_MAXn                                                                            2
#define HWIO_EE_n_EV_CH_k_CNTXT_0_MAXk                                                                           26
#define HWIO_EE_n_EV_CH_k_CNTXT_0_ATTR                                                                          0x3
#define HWIO_EE_n_EV_CH_k_CNTXT_0_INI2(n,k)        \
        in_dword_masked(HWIO_EE_n_EV_CH_k_CNTXT_0_ADDR(n,k), HWIO_EE_n_EV_CH_k_CNTXT_0_RMSK)
#define HWIO_EE_n_EV_CH_k_CNTXT_0_INMI2(n,k,mask)    \
        in_dword_masked(HWIO_EE_n_EV_CH_k_CNTXT_0_ADDR(n,k), mask)
#define HWIO_EE_n_EV_CH_k_CNTXT_0_OUTI2(n,k,val)    \
        out_dword(HWIO_EE_n_EV_CH_k_CNTXT_0_ADDR(n,k),val)
#define HWIO_EE_n_EV_CH_k_CNTXT_0_OUTMI2(n,k,mask,val) \
        out_dword_masked_ns(HWIO_EE_n_EV_CH_k_CNTXT_0_ADDR(n,k),mask,val,HWIO_EE_n_EV_CH_k_CNTXT_0_INI2(n,k))
#define HWIO_EE_n_EV_CH_k_CNTXT_0_ELEMENT_SIZE_BMSK                                                      0xff000000
#define HWIO_EE_n_EV_CH_k_CNTXT_0_ELEMENT_SIZE_SHFT                                                            0x18
#define HWIO_EE_n_EV_CH_k_CNTXT_0_CHSTATE_BMSK                                                             0xf00000
#define HWIO_EE_n_EV_CH_k_CNTXT_0_CHSTATE_SHFT                                                                 0x14
#define HWIO_EE_n_EV_CH_k_CNTXT_0_CHSTATE_NOT_ALLOCATED_FVAL                                                    0x0
#define HWIO_EE_n_EV_CH_k_CNTXT_0_CHSTATE_ALLOCATED_FVAL                                                        0x1
#define HWIO_EE_n_EV_CH_k_CNTXT_0_EE_BMSK                                                                   0xf0000
#define HWIO_EE_n_EV_CH_k_CNTXT_0_EE_SHFT                                                                      0x10
#define HWIO_EE_n_EV_CH_k_CNTXT_0_EVCHID_BMSK                                                                0xff00
#define HWIO_EE_n_EV_CH_k_CNTXT_0_EVCHID_SHFT                                                                   0x8
#define HWIO_EE_n_EV_CH_k_CNTXT_0_INTYPE_BMSK                                                                  0x80
#define HWIO_EE_n_EV_CH_k_CNTXT_0_INTYPE_SHFT                                                                   0x7
#define HWIO_EE_n_EV_CH_k_CNTXT_0_INTYPE_MSI_FVAL                                                               0x0
#define HWIO_EE_n_EV_CH_k_CNTXT_0_INTYPE_IRQ_FVAL                                                               0x1
#define HWIO_EE_n_EV_CH_k_CNTXT_0_CHTYPE_BMSK                                                                  0x7f
#define HWIO_EE_n_EV_CH_k_CNTXT_0_CHTYPE_SHFT                                                                   0x0
#define HWIO_EE_n_EV_CH_k_CNTXT_0_CHTYPE_MHI_EV_FVAL                                                            0x0
#define HWIO_EE_n_EV_CH_k_CNTXT_0_CHTYPE_XHCI_EV_FVAL                                                           0x1
#define HWIO_EE_n_EV_CH_k_CNTXT_0_CHTYPE_GPI_EV_FVAL                                                            0x2
#define HWIO_EE_n_EV_CH_k_CNTXT_0_CHTYPE_XDCI_FVAL                                                              0x3

#define HWIO_EE_n_EV_CH_k_CNTXT_1_ADDR(n,k)                                                              (GSI_REG_BASE      + 0x0001c004 + 0x12000 * (n) + 0x80 * (k))
#define HWIO_EE_n_EV_CH_k_CNTXT_1_PHYS(n,k)                                                              (GSI_REG_BASE_PHYS + 0x0001c004 + 0x12000 * (n) + 0x80 * (k))
#define HWIO_EE_n_EV_CH_k_CNTXT_1_OFFS(n,k)                                                              (GSI_REG_BASE_OFFS + 0x0001c004 + 0x12000 * (n) + 0x80 * (k))
#define HWIO_EE_n_EV_CH_k_CNTXT_1_RMSK                                                                     0xffffff
#define HWIO_EE_n_EV_CH_k_CNTXT_1_MAXn                                                                            2
#define HWIO_EE_n_EV_CH_k_CNTXT_1_MAXk                                                                           26
#define HWIO_EE_n_EV_CH_k_CNTXT_1_ATTR                                                                          0x3
#define HWIO_EE_n_EV_CH_k_CNTXT_1_INI2(n,k)        \
        in_dword_masked(HWIO_EE_n_EV_CH_k_CNTXT_1_ADDR(n,k), HWIO_EE_n_EV_CH_k_CNTXT_1_RMSK)
#define HWIO_EE_n_EV_CH_k_CNTXT_1_INMI2(n,k,mask)    \
        in_dword_masked(HWIO_EE_n_EV_CH_k_CNTXT_1_ADDR(n,k), mask)
#define HWIO_EE_n_EV_CH_k_CNTXT_1_OUTI2(n,k,val)    \
        out_dword(HWIO_EE_n_EV_CH_k_CNTXT_1_ADDR(n,k),val)
#define HWIO_EE_n_EV_CH_k_CNTXT_1_OUTMI2(n,k,mask,val) \
        out_dword_masked_ns(HWIO_EE_n_EV_CH_k_CNTXT_1_ADDR(n,k),mask,val,HWIO_EE_n_EV_CH_k_CNTXT_1_INI2(n,k))
#define HWIO_EE_n_EV_CH_k_CNTXT_1_R_LENGTH_BMSK                                                            0xffffff
#define HWIO_EE_n_EV_CH_k_CNTXT_1_R_LENGTH_SHFT                                                                 0x0

#define HWIO_EE_n_EV_CH_k_CNTXT_2_ADDR(n,k)                                                              (GSI_REG_BASE      + 0x0001c008 + 0x12000 * (n) + 0x80 * (k))
#define HWIO_EE_n_EV_CH_k_CNTXT_2_PHYS(n,k)                                                              (GSI_REG_BASE_PHYS + 0x0001c008 + 0x12000 * (n) + 0x80 * (k))
#define HWIO_EE_n_EV_CH_k_CNTXT_2_OFFS(n,k)                                                              (GSI_REG_BASE_OFFS + 0x0001c008 + 0x12000 * (n) + 0x80 * (k))
#define HWIO_EE_n_EV_CH_k_CNTXT_2_RMSK                                                                   0xffffffff
#define HWIO_EE_n_EV_CH_k_CNTXT_2_MAXn                                                                            2
#define HWIO_EE_n_EV_CH_k_CNTXT_2_MAXk                                                                           26
#define HWIO_EE_n_EV_CH_k_CNTXT_2_ATTR                                                                          0x3
#define HWIO_EE_n_EV_CH_k_CNTXT_2_INI2(n,k)        \
        in_dword_masked(HWIO_EE_n_EV_CH_k_CNTXT_2_ADDR(n,k), HWIO_EE_n_EV_CH_k_CNTXT_2_RMSK)
#define HWIO_EE_n_EV_CH_k_CNTXT_2_INMI2(n,k,mask)    \
        in_dword_masked(HWIO_EE_n_EV_CH_k_CNTXT_2_ADDR(n,k), mask)
#define HWIO_EE_n_EV_CH_k_CNTXT_2_OUTI2(n,k,val)    \
        out_dword(HWIO_EE_n_EV_CH_k_CNTXT_2_ADDR(n,k),val)
#define HWIO_EE_n_EV_CH_k_CNTXT_2_OUTMI2(n,k,mask,val) \
        out_dword_masked_ns(HWIO_EE_n_EV_CH_k_CNTXT_2_ADDR(n,k),mask,val,HWIO_EE_n_EV_CH_k_CNTXT_2_INI2(n,k))
#define HWIO_EE_n_EV_CH_k_CNTXT_2_R_BASE_ADDR_LSBS_BMSK                                                  0xffffffff
#define HWIO_EE_n_EV_CH_k_CNTXT_2_R_BASE_ADDR_LSBS_SHFT                                                         0x0

#define HWIO_EE_n_EV_CH_k_CNTXT_3_ADDR(n,k)                                                              (GSI_REG_BASE      + 0x0001c00c + 0x12000 * (n) + 0x80 * (k))
#define HWIO_EE_n_EV_CH_k_CNTXT_3_PHYS(n,k)                                                              (GSI_REG_BASE_PHYS + 0x0001c00c + 0x12000 * (n) + 0x80 * (k))
#define HWIO_EE_n_EV_CH_k_CNTXT_3_OFFS(n,k)                                                              (GSI_REG_BASE_OFFS + 0x0001c00c + 0x12000 * (n) + 0x80 * (k))
#define HWIO_EE_n_EV_CH_k_CNTXT_3_RMSK                                                                   0xffffffff
#define HWIO_EE_n_EV_CH_k_CNTXT_3_MAXn                                                                            2
#define HWIO_EE_n_EV_CH_k_CNTXT_3_MAXk                                                                           26
#define HWIO_EE_n_EV_CH_k_CNTXT_3_ATTR                                                                          0x3
#define HWIO_EE_n_EV_CH_k_CNTXT_3_INI2(n,k)        \
        in_dword_masked(HWIO_EE_n_EV_CH_k_CNTXT_3_ADDR(n,k), HWIO_EE_n_EV_CH_k_CNTXT_3_RMSK)
#define HWIO_EE_n_EV_CH_k_CNTXT_3_INMI2(n,k,mask)    \
        in_dword_masked(HWIO_EE_n_EV_CH_k_CNTXT_3_ADDR(n,k), mask)
#define HWIO_EE_n_EV_CH_k_CNTXT_3_OUTI2(n,k,val)    \
        out_dword(HWIO_EE_n_EV_CH_k_CNTXT_3_ADDR(n,k),val)
#define HWIO_EE_n_EV_CH_k_CNTXT_3_OUTMI2(n,k,mask,val) \
        out_dword_masked_ns(HWIO_EE_n_EV_CH_k_CNTXT_3_ADDR(n,k),mask,val,HWIO_EE_n_EV_CH_k_CNTXT_3_INI2(n,k))
#define HWIO_EE_n_EV_CH_k_CNTXT_3_R_BASE_ADDR_MSBS_BMSK                                                  0xffffffff
#define HWIO_EE_n_EV_CH_k_CNTXT_3_R_BASE_ADDR_MSBS_SHFT                                                         0x0

#define HWIO_EE_n_EV_CH_k_CNTXT_4_ADDR(n,k)                                                              (GSI_REG_BASE      + 0x0001c010 + 0x12000 * (n) + 0x80 * (k))
#define HWIO_EE_n_EV_CH_k_CNTXT_4_PHYS(n,k)                                                              (GSI_REG_BASE_PHYS + 0x0001c010 + 0x12000 * (n) + 0x80 * (k))
#define HWIO_EE_n_EV_CH_k_CNTXT_4_OFFS(n,k)                                                              (GSI_REG_BASE_OFFS + 0x0001c010 + 0x12000 * (n) + 0x80 * (k))
#define HWIO_EE_n_EV_CH_k_CNTXT_4_RMSK                                                                   0xffffffff
#define HWIO_EE_n_EV_CH_k_CNTXT_4_MAXn                                                                            2
#define HWIO_EE_n_EV_CH_k_CNTXT_4_MAXk                                                                           26
#define HWIO_EE_n_EV_CH_k_CNTXT_4_ATTR                                                                          0x3
#define HWIO_EE_n_EV_CH_k_CNTXT_4_INI2(n,k)        \
        in_dword_masked(HWIO_EE_n_EV_CH_k_CNTXT_4_ADDR(n,k), HWIO_EE_n_EV_CH_k_CNTXT_4_RMSK)
#define HWIO_EE_n_EV_CH_k_CNTXT_4_INMI2(n,k,mask)    \
        in_dword_masked(HWIO_EE_n_EV_CH_k_CNTXT_4_ADDR(n,k), mask)
#define HWIO_EE_n_EV_CH_k_CNTXT_4_OUTI2(n,k,val)    \
        out_dword(HWIO_EE_n_EV_CH_k_CNTXT_4_ADDR(n,k),val)
#define HWIO_EE_n_EV_CH_k_CNTXT_4_OUTMI2(n,k,mask,val) \
        out_dword_masked_ns(HWIO_EE_n_EV_CH_k_CNTXT_4_ADDR(n,k),mask,val,HWIO_EE_n_EV_CH_k_CNTXT_4_INI2(n,k))
#define HWIO_EE_n_EV_CH_k_CNTXT_4_READ_PTR_LSB_BMSK                                                      0xffffffff
#define HWIO_EE_n_EV_CH_k_CNTXT_4_READ_PTR_LSB_SHFT                                                             0x0

#define HWIO_EE_n_EV_CH_k_CNTXT_5_ADDR(n,k)                                                              (GSI_REG_BASE      + 0x0001c014 + 0x12000 * (n) + 0x80 * (k))
#define HWIO_EE_n_EV_CH_k_CNTXT_5_PHYS(n,k)                                                              (GSI_REG_BASE_PHYS + 0x0001c014 + 0x12000 * (n) + 0x80 * (k))
#define HWIO_EE_n_EV_CH_k_CNTXT_5_OFFS(n,k)                                                              (GSI_REG_BASE_OFFS + 0x0001c014 + 0x12000 * (n) + 0x80 * (k))
#define HWIO_EE_n_EV_CH_k_CNTXT_5_RMSK                                                                   0xffffffff
#define HWIO_EE_n_EV_CH_k_CNTXT_5_MAXn                                                                            2
#define HWIO_EE_n_EV_CH_k_CNTXT_5_MAXk                                                                           26
#define HWIO_EE_n_EV_CH_k_CNTXT_5_ATTR                                                                          0x1
#define HWIO_EE_n_EV_CH_k_CNTXT_5_INI2(n,k)        \
        in_dword_masked(HWIO_EE_n_EV_CH_k_CNTXT_5_ADDR(n,k), HWIO_EE_n_EV_CH_k_CNTXT_5_RMSK)
#define HWIO_EE_n_EV_CH_k_CNTXT_5_INMI2(n,k,mask)    \
        in_dword_masked(HWIO_EE_n_EV_CH_k_CNTXT_5_ADDR(n,k), mask)
#define HWIO_EE_n_EV_CH_k_CNTXT_5_READ_PTR_MSB_BMSK                                                      0xffffffff
#define HWIO_EE_n_EV_CH_k_CNTXT_5_READ_PTR_MSB_SHFT                                                             0x0

#define HWIO_EE_n_EV_CH_k_CNTXT_6_ADDR(n,k)                                                              (GSI_REG_BASE      + 0x0001c018 + 0x12000 * (n) + 0x80 * (k))
#define HWIO_EE_n_EV_CH_k_CNTXT_6_PHYS(n,k)                                                              (GSI_REG_BASE_PHYS + 0x0001c018 + 0x12000 * (n) + 0x80 * (k))
#define HWIO_EE_n_EV_CH_k_CNTXT_6_OFFS(n,k)                                                              (GSI_REG_BASE_OFFS + 0x0001c018 + 0x12000 * (n) + 0x80 * (k))
#define HWIO_EE_n_EV_CH_k_CNTXT_6_RMSK                                                                   0xffffffff
#define HWIO_EE_n_EV_CH_k_CNTXT_6_MAXn                                                                            2
#define HWIO_EE_n_EV_CH_k_CNTXT_6_MAXk                                                                           26
#define HWIO_EE_n_EV_CH_k_CNTXT_6_ATTR                                                                          0x1
#define HWIO_EE_n_EV_CH_k_CNTXT_6_INI2(n,k)        \
        in_dword_masked(HWIO_EE_n_EV_CH_k_CNTXT_6_ADDR(n,k), HWIO_EE_n_EV_CH_k_CNTXT_6_RMSK)
#define HWIO_EE_n_EV_CH_k_CNTXT_6_INMI2(n,k,mask)    \
        in_dword_masked(HWIO_EE_n_EV_CH_k_CNTXT_6_ADDR(n,k), mask)
#define HWIO_EE_n_EV_CH_k_CNTXT_6_WRITE_PTR_LSB_BMSK                                                     0xffffffff
#define HWIO_EE_n_EV_CH_k_CNTXT_6_WRITE_PTR_LSB_SHFT                                                            0x0

#define HWIO_EE_n_EV_CH_k_CNTXT_7_ADDR(n,k)                                                              (GSI_REG_BASE      + 0x0001c01c + 0x12000 * (n) + 0x80 * (k))
#define HWIO_EE_n_EV_CH_k_CNTXT_7_PHYS(n,k)                                                              (GSI_REG_BASE_PHYS + 0x0001c01c + 0x12000 * (n) + 0x80 * (k))
#define HWIO_EE_n_EV_CH_k_CNTXT_7_OFFS(n,k)                                                              (GSI_REG_BASE_OFFS + 0x0001c01c + 0x12000 * (n) + 0x80 * (k))
#define HWIO_EE_n_EV_CH_k_CNTXT_7_RMSK                                                                   0xffffffff
#define HWIO_EE_n_EV_CH_k_CNTXT_7_MAXn                                                                            2
#define HWIO_EE_n_EV_CH_k_CNTXT_7_MAXk                                                                           26
#define HWIO_EE_n_EV_CH_k_CNTXT_7_ATTR                                                                          0x1
#define HWIO_EE_n_EV_CH_k_CNTXT_7_INI2(n,k)        \
        in_dword_masked(HWIO_EE_n_EV_CH_k_CNTXT_7_ADDR(n,k), HWIO_EE_n_EV_CH_k_CNTXT_7_RMSK)
#define HWIO_EE_n_EV_CH_k_CNTXT_7_INMI2(n,k,mask)    \
        in_dword_masked(HWIO_EE_n_EV_CH_k_CNTXT_7_ADDR(n,k), mask)
#define HWIO_EE_n_EV_CH_k_CNTXT_7_WRITE_PTR_MSB_BMSK                                                     0xffffffff
#define HWIO_EE_n_EV_CH_k_CNTXT_7_WRITE_PTR_MSB_SHFT                                                            0x0

#define HWIO_EE_n_EV_CH_k_CNTXT_8_ADDR(n,k)                                                              (GSI_REG_BASE      + 0x0001c020 + 0x12000 * (n) + 0x80 * (k))
#define HWIO_EE_n_EV_CH_k_CNTXT_8_PHYS(n,k)                                                              (GSI_REG_BASE_PHYS + 0x0001c020 + 0x12000 * (n) + 0x80 * (k))
#define HWIO_EE_n_EV_CH_k_CNTXT_8_OFFS(n,k)                                                              (GSI_REG_BASE_OFFS + 0x0001c020 + 0x12000 * (n) + 0x80 * (k))
#define HWIO_EE_n_EV_CH_k_CNTXT_8_RMSK                                                                   0xffffffff
#define HWIO_EE_n_EV_CH_k_CNTXT_8_MAXn                                                                            2
#define HWIO_EE_n_EV_CH_k_CNTXT_8_MAXk                                                                           26
#define HWIO_EE_n_EV_CH_k_CNTXT_8_ATTR                                                                          0x3
#define HWIO_EE_n_EV_CH_k_CNTXT_8_INI2(n,k)        \
        in_dword_masked(HWIO_EE_n_EV_CH_k_CNTXT_8_ADDR(n,k), HWIO_EE_n_EV_CH_k_CNTXT_8_RMSK)
#define HWIO_EE_n_EV_CH_k_CNTXT_8_INMI2(n,k,mask)    \
        in_dword_masked(HWIO_EE_n_EV_CH_k_CNTXT_8_ADDR(n,k), mask)
#define HWIO_EE_n_EV_CH_k_CNTXT_8_OUTI2(n,k,val)    \
        out_dword(HWIO_EE_n_EV_CH_k_CNTXT_8_ADDR(n,k),val)
#define HWIO_EE_n_EV_CH_k_CNTXT_8_OUTMI2(n,k,mask,val) \
        out_dword_masked_ns(HWIO_EE_n_EV_CH_k_CNTXT_8_ADDR(n,k),mask,val,HWIO_EE_n_EV_CH_k_CNTXT_8_INI2(n,k))
#define HWIO_EE_n_EV_CH_k_CNTXT_8_INT_MOD_CNT_BMSK                                                       0xff000000
#define HWIO_EE_n_EV_CH_k_CNTXT_8_INT_MOD_CNT_SHFT                                                             0x18
#define HWIO_EE_n_EV_CH_k_CNTXT_8_INT_MODC_BMSK                                                            0xff0000
#define HWIO_EE_n_EV_CH_k_CNTXT_8_INT_MODC_SHFT                                                                0x10
#define HWIO_EE_n_EV_CH_k_CNTXT_8_INT_MODT_BMSK                                                              0xffff
#define HWIO_EE_n_EV_CH_k_CNTXT_8_INT_MODT_SHFT                                                                 0x0

#define HWIO_EE_n_EV_CH_k_CNTXT_9_ADDR(n,k)                                                              (GSI_REG_BASE      + 0x0001c024 + 0x12000 * (n) + 0x80 * (k))
#define HWIO_EE_n_EV_CH_k_CNTXT_9_PHYS(n,k)                                                              (GSI_REG_BASE_PHYS + 0x0001c024 + 0x12000 * (n) + 0x80 * (k))
#define HWIO_EE_n_EV_CH_k_CNTXT_9_OFFS(n,k)                                                              (GSI_REG_BASE_OFFS + 0x0001c024 + 0x12000 * (n) + 0x80 * (k))
#define HWIO_EE_n_EV_CH_k_CNTXT_9_RMSK                                                                   0xffffffff
#define HWIO_EE_n_EV_CH_k_CNTXT_9_MAXn                                                                            2
#define HWIO_EE_n_EV_CH_k_CNTXT_9_MAXk                                                                           26
#define HWIO_EE_n_EV_CH_k_CNTXT_9_ATTR                                                                          0x3
#define HWIO_EE_n_EV_CH_k_CNTXT_9_INI2(n,k)        \
        in_dword_masked(HWIO_EE_n_EV_CH_k_CNTXT_9_ADDR(n,k), HWIO_EE_n_EV_CH_k_CNTXT_9_RMSK)
#define HWIO_EE_n_EV_CH_k_CNTXT_9_INMI2(n,k,mask)    \
        in_dword_masked(HWIO_EE_n_EV_CH_k_CNTXT_9_ADDR(n,k), mask)
#define HWIO_EE_n_EV_CH_k_CNTXT_9_OUTI2(n,k,val)    \
        out_dword(HWIO_EE_n_EV_CH_k_CNTXT_9_ADDR(n,k),val)
#define HWIO_EE_n_EV_CH_k_CNTXT_9_OUTMI2(n,k,mask,val) \
        out_dword_masked_ns(HWIO_EE_n_EV_CH_k_CNTXT_9_ADDR(n,k),mask,val,HWIO_EE_n_EV_CH_k_CNTXT_9_INI2(n,k))
#define HWIO_EE_n_EV_CH_k_CNTXT_9_INTVEC_BMSK                                                            0xffffffff
#define HWIO_EE_n_EV_CH_k_CNTXT_9_INTVEC_SHFT                                                                   0x0

#define HWIO_EE_n_EV_CH_k_CNTXT_10_ADDR(n,k)                                                             (GSI_REG_BASE      + 0x0001c028 + 0x12000 * (n) + 0x80 * (k))
#define HWIO_EE_n_EV_CH_k_CNTXT_10_PHYS(n,k)                                                             (GSI_REG_BASE_PHYS + 0x0001c028 + 0x12000 * (n) + 0x80 * (k))
#define HWIO_EE_n_EV_CH_k_CNTXT_10_OFFS(n,k)                                                             (GSI_REG_BASE_OFFS + 0x0001c028 + 0x12000 * (n) + 0x80 * (k))
#define HWIO_EE_n_EV_CH_k_CNTXT_10_RMSK                                                                  0xffffffff
#define HWIO_EE_n_EV_CH_k_CNTXT_10_MAXn                                                                           2
#define HWIO_EE_n_EV_CH_k_CNTXT_10_MAXk                                                                          26
#define HWIO_EE_n_EV_CH_k_CNTXT_10_ATTR                                                                         0x3
#define HWIO_EE_n_EV_CH_k_CNTXT_10_INI2(n,k)        \
        in_dword_masked(HWIO_EE_n_EV_CH_k_CNTXT_10_ADDR(n,k), HWIO_EE_n_EV_CH_k_CNTXT_10_RMSK)
#define HWIO_EE_n_EV_CH_k_CNTXT_10_INMI2(n,k,mask)    \
        in_dword_masked(HWIO_EE_n_EV_CH_k_CNTXT_10_ADDR(n,k), mask)
#define HWIO_EE_n_EV_CH_k_CNTXT_10_OUTI2(n,k,val)    \
        out_dword(HWIO_EE_n_EV_CH_k_CNTXT_10_ADDR(n,k),val)
#define HWIO_EE_n_EV_CH_k_CNTXT_10_OUTMI2(n,k,mask,val) \
        out_dword_masked_ns(HWIO_EE_n_EV_CH_k_CNTXT_10_ADDR(n,k),mask,val,HWIO_EE_n_EV_CH_k_CNTXT_10_INI2(n,k))
#define HWIO_EE_n_EV_CH_k_CNTXT_10_MSI_ADDR_LSB_BMSK                                                     0xffffffff
#define HWIO_EE_n_EV_CH_k_CNTXT_10_MSI_ADDR_LSB_SHFT                                                            0x0

#define HWIO_EE_n_EV_CH_k_CNTXT_11_ADDR(n,k)                                                             (GSI_REG_BASE      + 0x0001c02c + 0x12000 * (n) + 0x80 * (k))
#define HWIO_EE_n_EV_CH_k_CNTXT_11_PHYS(n,k)                                                             (GSI_REG_BASE_PHYS + 0x0001c02c + 0x12000 * (n) + 0x80 * (k))
#define HWIO_EE_n_EV_CH_k_CNTXT_11_OFFS(n,k)                                                             (GSI_REG_BASE_OFFS + 0x0001c02c + 0x12000 * (n) + 0x80 * (k))
#define HWIO_EE_n_EV_CH_k_CNTXT_11_RMSK                                                                  0xffffffff
#define HWIO_EE_n_EV_CH_k_CNTXT_11_MAXn                                                                           2
#define HWIO_EE_n_EV_CH_k_CNTXT_11_MAXk                                                                          26
#define HWIO_EE_n_EV_CH_k_CNTXT_11_ATTR                                                                         0x3
#define HWIO_EE_n_EV_CH_k_CNTXT_11_INI2(n,k)        \
        in_dword_masked(HWIO_EE_n_EV_CH_k_CNTXT_11_ADDR(n,k), HWIO_EE_n_EV_CH_k_CNTXT_11_RMSK)
#define HWIO_EE_n_EV_CH_k_CNTXT_11_INMI2(n,k,mask)    \
        in_dword_masked(HWIO_EE_n_EV_CH_k_CNTXT_11_ADDR(n,k), mask)
#define HWIO_EE_n_EV_CH_k_CNTXT_11_OUTI2(n,k,val)    \
        out_dword(HWIO_EE_n_EV_CH_k_CNTXT_11_ADDR(n,k),val)
#define HWIO_EE_n_EV_CH_k_CNTXT_11_OUTMI2(n,k,mask,val) \
        out_dword_masked_ns(HWIO_EE_n_EV_CH_k_CNTXT_11_ADDR(n,k),mask,val,HWIO_EE_n_EV_CH_k_CNTXT_11_INI2(n,k))
#define HWIO_EE_n_EV_CH_k_CNTXT_11_MSI_ADDR_MSB_BMSK                                                     0xffffffff
#define HWIO_EE_n_EV_CH_k_CNTXT_11_MSI_ADDR_MSB_SHFT                                                            0x0

#define HWIO_EE_n_EV_CH_k_CNTXT_12_ADDR(n,k)                                                             (GSI_REG_BASE      + 0x0001c030 + 0x12000 * (n) + 0x80 * (k))
#define HWIO_EE_n_EV_CH_k_CNTXT_12_PHYS(n,k)                                                             (GSI_REG_BASE_PHYS + 0x0001c030 + 0x12000 * (n) + 0x80 * (k))
#define HWIO_EE_n_EV_CH_k_CNTXT_12_OFFS(n,k)                                                             (GSI_REG_BASE_OFFS + 0x0001c030 + 0x12000 * (n) + 0x80 * (k))
#define HWIO_EE_n_EV_CH_k_CNTXT_12_RMSK                                                                  0xffffffff
#define HWIO_EE_n_EV_CH_k_CNTXT_12_MAXn                                                                           2
#define HWIO_EE_n_EV_CH_k_CNTXT_12_MAXk                                                                          26
#define HWIO_EE_n_EV_CH_k_CNTXT_12_ATTR                                                                         0x3
#define HWIO_EE_n_EV_CH_k_CNTXT_12_INI2(n,k)        \
        in_dword_masked(HWIO_EE_n_EV_CH_k_CNTXT_12_ADDR(n,k), HWIO_EE_n_EV_CH_k_CNTXT_12_RMSK)
#define HWIO_EE_n_EV_CH_k_CNTXT_12_INMI2(n,k,mask)    \
        in_dword_masked(HWIO_EE_n_EV_CH_k_CNTXT_12_ADDR(n,k), mask)
#define HWIO_EE_n_EV_CH_k_CNTXT_12_OUTI2(n,k,val)    \
        out_dword(HWIO_EE_n_EV_CH_k_CNTXT_12_ADDR(n,k),val)
#define HWIO_EE_n_EV_CH_k_CNTXT_12_OUTMI2(n,k,mask,val) \
        out_dword_masked_ns(HWIO_EE_n_EV_CH_k_CNTXT_12_ADDR(n,k),mask,val,HWIO_EE_n_EV_CH_k_CNTXT_12_INI2(n,k))
#define HWIO_EE_n_EV_CH_k_CNTXT_12_RP_UPDATE_ADDR_LSB_BMSK                                               0xffffffff
#define HWIO_EE_n_EV_CH_k_CNTXT_12_RP_UPDATE_ADDR_LSB_SHFT                                                      0x0

#define HWIO_EE_n_EV_CH_k_CNTXT_13_ADDR(n,k)                                                             (GSI_REG_BASE      + 0x0001c034 + 0x12000 * (n) + 0x80 * (k))
#define HWIO_EE_n_EV_CH_k_CNTXT_13_PHYS(n,k)                                                             (GSI_REG_BASE_PHYS + 0x0001c034 + 0x12000 * (n) + 0x80 * (k))
#define HWIO_EE_n_EV_CH_k_CNTXT_13_OFFS(n,k)                                                             (GSI_REG_BASE_OFFS + 0x0001c034 + 0x12000 * (n) + 0x80 * (k))
#define HWIO_EE_n_EV_CH_k_CNTXT_13_RMSK                                                                  0xffffffff
#define HWIO_EE_n_EV_CH_k_CNTXT_13_MAXn                                                                           2
#define HWIO_EE_n_EV_CH_k_CNTXT_13_MAXk                                                                          26
#define HWIO_EE_n_EV_CH_k_CNTXT_13_ATTR                                                                         0x3
#define HWIO_EE_n_EV_CH_k_CNTXT_13_INI2(n,k)        \
        in_dword_masked(HWIO_EE_n_EV_CH_k_CNTXT_13_ADDR(n,k), HWIO_EE_n_EV_CH_k_CNTXT_13_RMSK)
#define HWIO_EE_n_EV_CH_k_CNTXT_13_INMI2(n,k,mask)    \
        in_dword_masked(HWIO_EE_n_EV_CH_k_CNTXT_13_ADDR(n,k), mask)
#define HWIO_EE_n_EV_CH_k_CNTXT_13_OUTI2(n,k,val)    \
        out_dword(HWIO_EE_n_EV_CH_k_CNTXT_13_ADDR(n,k),val)
#define HWIO_EE_n_EV_CH_k_CNTXT_13_OUTMI2(n,k,mask,val) \
        out_dword_masked_ns(HWIO_EE_n_EV_CH_k_CNTXT_13_ADDR(n,k),mask,val,HWIO_EE_n_EV_CH_k_CNTXT_13_INI2(n,k))
#define HWIO_EE_n_EV_CH_k_CNTXT_13_RP_UPDATE_ADDR_MSB_BMSK                                               0xffffffff
#define HWIO_EE_n_EV_CH_k_CNTXT_13_RP_UPDATE_ADDR_MSB_SHFT                                                      0x0

#define HWIO_EE_n_EV_CH_k_ELEM_SIZE_SHIFT_ADDR(n,k)                                                      (GSI_REG_BASE      + 0x0001c038 + 0x12000 * (n) + 0x80 * (k))
#define HWIO_EE_n_EV_CH_k_ELEM_SIZE_SHIFT_PHYS(n,k)                                                      (GSI_REG_BASE_PHYS + 0x0001c038 + 0x12000 * (n) + 0x80 * (k))
#define HWIO_EE_n_EV_CH_k_ELEM_SIZE_SHIFT_OFFS(n,k)                                                      (GSI_REG_BASE_OFFS + 0x0001c038 + 0x12000 * (n) + 0x80 * (k))
#define HWIO_EE_n_EV_CH_k_ELEM_SIZE_SHIFT_RMSK                                                                  0xf
#define HWIO_EE_n_EV_CH_k_ELEM_SIZE_SHIFT_MAXn                                                                    2
#define HWIO_EE_n_EV_CH_k_ELEM_SIZE_SHIFT_MAXk                                                                   26
#define HWIO_EE_n_EV_CH_k_ELEM_SIZE_SHIFT_ATTR                                                                  0x1
#define HWIO_EE_n_EV_CH_k_ELEM_SIZE_SHIFT_INI2(n,k)        \
        in_dword_masked(HWIO_EE_n_EV_CH_k_ELEM_SIZE_SHIFT_ADDR(n,k), HWIO_EE_n_EV_CH_k_ELEM_SIZE_SHIFT_RMSK)
#define HWIO_EE_n_EV_CH_k_ELEM_SIZE_SHIFT_INMI2(n,k,mask)    \
        in_dword_masked(HWIO_EE_n_EV_CH_k_ELEM_SIZE_SHIFT_ADDR(n,k), mask)
#define HWIO_EE_n_EV_CH_k_ELEM_SIZE_SHIFT_ELEM_SIZE_SHIFT_BMSK                                                  0xf
#define HWIO_EE_n_EV_CH_k_ELEM_SIZE_SHIFT_ELEM_SIZE_SHIFT_SHFT                                                  0x0
#define HWIO_EE_n_EV_CH_k_ELEM_SIZE_SHIFT_ELEM_SIZE_SHIFT_TWO_FVAL                                              0x0
#define HWIO_EE_n_EV_CH_k_ELEM_SIZE_SHIFT_ELEM_SIZE_SHIFT_THREE_FVAL                                            0x1
#define HWIO_EE_n_EV_CH_k_ELEM_SIZE_SHIFT_ELEM_SIZE_SHIFT_FOUR_FVAL                                             0x2
#define HWIO_EE_n_EV_CH_k_ELEM_SIZE_SHIFT_ELEM_SIZE_SHIFT_FIVE_FVAL                                             0x3
#define HWIO_EE_n_EV_CH_k_ELEM_SIZE_SHIFT_ELEM_SIZE_SHIFT_SIX_FVAL                                              0x4

#define HWIO_EE_n_EV_CH_k_SCRATCH_0_ADDR(n,k)                                                            (GSI_REG_BASE      + 0x0001c048 + 0x12000 * (n) + 0x80 * (k))
#define HWIO_EE_n_EV_CH_k_SCRATCH_0_PHYS(n,k)                                                            (GSI_REG_BASE_PHYS + 0x0001c048 + 0x12000 * (n) + 0x80 * (k))
#define HWIO_EE_n_EV_CH_k_SCRATCH_0_OFFS(n,k)                                                            (GSI_REG_BASE_OFFS + 0x0001c048 + 0x12000 * (n) + 0x80 * (k))
#define HWIO_EE_n_EV_CH_k_SCRATCH_0_RMSK                                                                 0xffffffff
#define HWIO_EE_n_EV_CH_k_SCRATCH_0_MAXn                                                                          2
#define HWIO_EE_n_EV_CH_k_SCRATCH_0_MAXk                                                                         26
#define HWIO_EE_n_EV_CH_k_SCRATCH_0_ATTR                                                                        0x3
#define HWIO_EE_n_EV_CH_k_SCRATCH_0_INI2(n,k)        \
        in_dword_masked(HWIO_EE_n_EV_CH_k_SCRATCH_0_ADDR(n,k), HWIO_EE_n_EV_CH_k_SCRATCH_0_RMSK)
#define HWIO_EE_n_EV_CH_k_SCRATCH_0_INMI2(n,k,mask)    \
        in_dword_masked(HWIO_EE_n_EV_CH_k_SCRATCH_0_ADDR(n,k), mask)
#define HWIO_EE_n_EV_CH_k_SCRATCH_0_OUTI2(n,k,val)    \
        out_dword(HWIO_EE_n_EV_CH_k_SCRATCH_0_ADDR(n,k),val)
#define HWIO_EE_n_EV_CH_k_SCRATCH_0_OUTMI2(n,k,mask,val) \
        out_dword_masked_ns(HWIO_EE_n_EV_CH_k_SCRATCH_0_ADDR(n,k),mask,val,HWIO_EE_n_EV_CH_k_SCRATCH_0_INI2(n,k))
#define HWIO_EE_n_EV_CH_k_SCRATCH_0_SCRATCH_BMSK                                                         0xffffffff
#define HWIO_EE_n_EV_CH_k_SCRATCH_0_SCRATCH_SHFT                                                                0x0

#define HWIO_EE_n_EV_CH_k_SCRATCH_1_ADDR(n,k)                                                            (GSI_REG_BASE      + 0x0001c04c + 0x12000 * (n) + 0x80 * (k))
#define HWIO_EE_n_EV_CH_k_SCRATCH_1_PHYS(n,k)                                                            (GSI_REG_BASE_PHYS + 0x0001c04c + 0x12000 * (n) + 0x80 * (k))
#define HWIO_EE_n_EV_CH_k_SCRATCH_1_OFFS(n,k)                                                            (GSI_REG_BASE_OFFS + 0x0001c04c + 0x12000 * (n) + 0x80 * (k))
#define HWIO_EE_n_EV_CH_k_SCRATCH_1_RMSK                                                                 0xffffffff
#define HWIO_EE_n_EV_CH_k_SCRATCH_1_MAXn                                                                          2
#define HWIO_EE_n_EV_CH_k_SCRATCH_1_MAXk                                                                         26
#define HWIO_EE_n_EV_CH_k_SCRATCH_1_ATTR                                                                        0x3
#define HWIO_EE_n_EV_CH_k_SCRATCH_1_INI2(n,k)        \
        in_dword_masked(HWIO_EE_n_EV_CH_k_SCRATCH_1_ADDR(n,k), HWIO_EE_n_EV_CH_k_SCRATCH_1_RMSK)
#define HWIO_EE_n_EV_CH_k_SCRATCH_1_INMI2(n,k,mask)    \
        in_dword_masked(HWIO_EE_n_EV_CH_k_SCRATCH_1_ADDR(n,k), mask)
#define HWIO_EE_n_EV_CH_k_SCRATCH_1_OUTI2(n,k,val)    \
        out_dword(HWIO_EE_n_EV_CH_k_SCRATCH_1_ADDR(n,k),val)
#define HWIO_EE_n_EV_CH_k_SCRATCH_1_OUTMI2(n,k,mask,val) \
        out_dword_masked_ns(HWIO_EE_n_EV_CH_k_SCRATCH_1_ADDR(n,k),mask,val,HWIO_EE_n_EV_CH_k_SCRATCH_1_INI2(n,k))
#define HWIO_EE_n_EV_CH_k_SCRATCH_1_SCRATCH_BMSK                                                         0xffffffff
#define HWIO_EE_n_EV_CH_k_SCRATCH_1_SCRATCH_SHFT                                                                0x0

#define HWIO_EE_n_EV_CH_k_SCRATCH_2_ADDR(n,k)                                                            (GSI_REG_BASE      + 0x0001c050 + 0x12000 * (n) + 0x80 * (k))
#define HWIO_EE_n_EV_CH_k_SCRATCH_2_PHYS(n,k)                                                            (GSI_REG_BASE_PHYS + 0x0001c050 + 0x12000 * (n) + 0x80 * (k))
#define HWIO_EE_n_EV_CH_k_SCRATCH_2_OFFS(n,k)                                                            (GSI_REG_BASE_OFFS + 0x0001c050 + 0x12000 * (n) + 0x80 * (k))
#define HWIO_EE_n_EV_CH_k_SCRATCH_2_RMSK                                                                 0xffffffff
#define HWIO_EE_n_EV_CH_k_SCRATCH_2_MAXn                                                                          2
#define HWIO_EE_n_EV_CH_k_SCRATCH_2_MAXk                                                                         26
#define HWIO_EE_n_EV_CH_k_SCRATCH_2_ATTR                                                                        0x3
#define HWIO_EE_n_EV_CH_k_SCRATCH_2_INI2(n,k)        \
        in_dword_masked(HWIO_EE_n_EV_CH_k_SCRATCH_2_ADDR(n,k), HWIO_EE_n_EV_CH_k_SCRATCH_2_RMSK)
#define HWIO_EE_n_EV_CH_k_SCRATCH_2_INMI2(n,k,mask)    \
        in_dword_masked(HWIO_EE_n_EV_CH_k_SCRATCH_2_ADDR(n,k), mask)
#define HWIO_EE_n_EV_CH_k_SCRATCH_2_OUTI2(n,k,val)    \
        out_dword(HWIO_EE_n_EV_CH_k_SCRATCH_2_ADDR(n,k),val)
#define HWIO_EE_n_EV_CH_k_SCRATCH_2_OUTMI2(n,k,mask,val) \
        out_dword_masked_ns(HWIO_EE_n_EV_CH_k_SCRATCH_2_ADDR(n,k),mask,val,HWIO_EE_n_EV_CH_k_SCRATCH_2_INI2(n,k))
#define HWIO_EE_n_EV_CH_k_SCRATCH_2_SCRATCH_BMSK                                                         0xffffffff
#define HWIO_EE_n_EV_CH_k_SCRATCH_2_SCRATCH_SHFT                                                                0x0

#define HWIO_EE_n_GSI_CH_k_DOORBELL_0_ADDR(n,k)                                                          (GSI_REG_BASE      + 0x00024000 + 0x12000 * (n) + 0x8 * (k))
#define HWIO_EE_n_GSI_CH_k_DOORBELL_0_PHYS(n,k)                                                          (GSI_REG_BASE_PHYS + 0x00024000 + 0x12000 * (n) + 0x8 * (k))
#define HWIO_EE_n_GSI_CH_k_DOORBELL_0_OFFS(n,k)                                                          (GSI_REG_BASE_OFFS + 0x00024000 + 0x12000 * (n) + 0x8 * (k))
#define HWIO_EE_n_GSI_CH_k_DOORBELL_0_RMSK                                                               0xffffffff
#define HWIO_EE_n_GSI_CH_k_DOORBELL_0_MAXn                                                                        2
#define HWIO_EE_n_GSI_CH_k_DOORBELL_0_MAXk                                                                       27
#define HWIO_EE_n_GSI_CH_k_DOORBELL_0_ATTR                                                                      0x2
#define HWIO_EE_n_GSI_CH_k_DOORBELL_0_OUTI2(n,k,val)    \
        out_dword(HWIO_EE_n_GSI_CH_k_DOORBELL_0_ADDR(n,k),val)
#define HWIO_EE_n_GSI_CH_k_DOORBELL_0_WRITE_PTR_LSB_BMSK                                                 0xffffffff
#define HWIO_EE_n_GSI_CH_k_DOORBELL_0_WRITE_PTR_LSB_SHFT                                                        0x0

#define HWIO_EE_n_GSI_CH_k_DOORBELL_1_ADDR(n,k)                                                          (GSI_REG_BASE      + 0x00024004 + 0x12000 * (n) + 0x8 * (k))
#define HWIO_EE_n_GSI_CH_k_DOORBELL_1_PHYS(n,k)                                                          (GSI_REG_BASE_PHYS + 0x00024004 + 0x12000 * (n) + 0x8 * (k))
#define HWIO_EE_n_GSI_CH_k_DOORBELL_1_OFFS(n,k)                                                          (GSI_REG_BASE_OFFS + 0x00024004 + 0x12000 * (n) + 0x8 * (k))
#define HWIO_EE_n_GSI_CH_k_DOORBELL_1_RMSK                                                               0xffffffff
#define HWIO_EE_n_GSI_CH_k_DOORBELL_1_MAXn                                                                        2
#define HWIO_EE_n_GSI_CH_k_DOORBELL_1_MAXk                                                                       27
#define HWIO_EE_n_GSI_CH_k_DOORBELL_1_ATTR                                                                      0x2
#define HWIO_EE_n_GSI_CH_k_DOORBELL_1_OUTI2(n,k,val)    \
        out_dword(HWIO_EE_n_GSI_CH_k_DOORBELL_1_ADDR(n,k),val)
#define HWIO_EE_n_GSI_CH_k_DOORBELL_1_WRITE_PTR_MSB_BMSK                                                 0xffffffff
#define HWIO_EE_n_GSI_CH_k_DOORBELL_1_WRITE_PTR_MSB_SHFT                                                        0x0

#define HWIO_EE_n_EV_CH_k_DOORBELL_0_ADDR(n,k)                                                           (GSI_REG_BASE      + 0x00024800 + 0x12000 * (n) + 0x8 * (k))
#define HWIO_EE_n_EV_CH_k_DOORBELL_0_PHYS(n,k)                                                           (GSI_REG_BASE_PHYS + 0x00024800 + 0x12000 * (n) + 0x8 * (k))
#define HWIO_EE_n_EV_CH_k_DOORBELL_0_OFFS(n,k)                                                           (GSI_REG_BASE_OFFS + 0x00024800 + 0x12000 * (n) + 0x8 * (k))
#define HWIO_EE_n_EV_CH_k_DOORBELL_0_RMSK                                                                0xffffffff
#define HWIO_EE_n_EV_CH_k_DOORBELL_0_MAXn                                                                         2
#define HWIO_EE_n_EV_CH_k_DOORBELL_0_MAXk                                                                        26
#define HWIO_EE_n_EV_CH_k_DOORBELL_0_ATTR                                                                       0x2
#define HWIO_EE_n_EV_CH_k_DOORBELL_0_OUTI2(n,k,val)    \
        out_dword(HWIO_EE_n_EV_CH_k_DOORBELL_0_ADDR(n,k),val)
#define HWIO_EE_n_EV_CH_k_DOORBELL_0_WRITE_PTR_LSB_BMSK                                                  0xffffffff
#define HWIO_EE_n_EV_CH_k_DOORBELL_0_WRITE_PTR_LSB_SHFT                                                         0x0

#define HWIO_EE_n_EV_CH_k_DOORBELL_1_ADDR(n,k)                                                           (GSI_REG_BASE      + 0x00024804 + 0x12000 * (n) + 0x8 * (k))
#define HWIO_EE_n_EV_CH_k_DOORBELL_1_PHYS(n,k)                                                           (GSI_REG_BASE_PHYS + 0x00024804 + 0x12000 * (n) + 0x8 * (k))
#define HWIO_EE_n_EV_CH_k_DOORBELL_1_OFFS(n,k)                                                           (GSI_REG_BASE_OFFS + 0x00024804 + 0x12000 * (n) + 0x8 * (k))
#define HWIO_EE_n_EV_CH_k_DOORBELL_1_RMSK                                                                0xffffffff
#define HWIO_EE_n_EV_CH_k_DOORBELL_1_MAXn                                                                         2
#define HWIO_EE_n_EV_CH_k_DOORBELL_1_MAXk                                                                        26
#define HWIO_EE_n_EV_CH_k_DOORBELL_1_ATTR                                                                       0x2
#define HWIO_EE_n_EV_CH_k_DOORBELL_1_OUTI2(n,k,val)    \
        out_dword(HWIO_EE_n_EV_CH_k_DOORBELL_1_ADDR(n,k),val)
#define HWIO_EE_n_EV_CH_k_DOORBELL_1_WRITE_PTR_MSB_BMSK                                                  0xffffffff
#define HWIO_EE_n_EV_CH_k_DOORBELL_1_WRITE_PTR_MSB_SHFT                                                         0x0

#define HWIO_EE_n_GSI_STATUS_ADDR(n)                                                                     (GSI_REG_BASE      + 0x00025000 + 0x12000 * (n))
#define HWIO_EE_n_GSI_STATUS_PHYS(n)                                                                     (GSI_REG_BASE_PHYS + 0x00025000 + 0x12000 * (n))
#define HWIO_EE_n_GSI_STATUS_OFFS(n)                                                                     (GSI_REG_BASE_OFFS + 0x00025000 + 0x12000 * (n))
#define HWIO_EE_n_GSI_STATUS_RMSK                                                                               0x1
#define HWIO_EE_n_GSI_STATUS_MAXn                                                                                 2
#define HWIO_EE_n_GSI_STATUS_ATTR                                                                               0x1
#define HWIO_EE_n_GSI_STATUS_INI(n)        \
        in_dword_masked(HWIO_EE_n_GSI_STATUS_ADDR(n), HWIO_EE_n_GSI_STATUS_RMSK)
#define HWIO_EE_n_GSI_STATUS_INMI(n,mask)    \
        in_dword_masked(HWIO_EE_n_GSI_STATUS_ADDR(n), mask)
#define HWIO_EE_n_GSI_STATUS_ENABLED_BMSK                                                                       0x1
#define HWIO_EE_n_GSI_STATUS_ENABLED_SHFT                                                                       0x0

#define HWIO_EE_n_GSI_CH_CMD_ADDR(n)                                                                     (GSI_REG_BASE      + 0x00025008 + 0x12000 * (n))
#define HWIO_EE_n_GSI_CH_CMD_PHYS(n)                                                                     (GSI_REG_BASE_PHYS + 0x00025008 + 0x12000 * (n))
#define HWIO_EE_n_GSI_CH_CMD_OFFS(n)                                                                     (GSI_REG_BASE_OFFS + 0x00025008 + 0x12000 * (n))
#define HWIO_EE_n_GSI_CH_CMD_RMSK                                                                        0xff0000ff
#define HWIO_EE_n_GSI_CH_CMD_MAXn                                                                                 2
#define HWIO_EE_n_GSI_CH_CMD_ATTR                                                                               0x2
#define HWIO_EE_n_GSI_CH_CMD_OUTI(n,val)    \
        out_dword(HWIO_EE_n_GSI_CH_CMD_ADDR(n),val)
#define HWIO_EE_n_GSI_CH_CMD_OPCODE_BMSK                                                                 0xff000000
#define HWIO_EE_n_GSI_CH_CMD_OPCODE_SHFT                                                                       0x18
#define HWIO_EE_n_GSI_CH_CMD_OPCODE_ALLOCATE_FVAL                                                               0x0
#define HWIO_EE_n_GSI_CH_CMD_OPCODE_START_FVAL                                                                  0x1
#define HWIO_EE_n_GSI_CH_CMD_OPCODE_STOP_FVAL                                                                   0x2
#define HWIO_EE_n_GSI_CH_CMD_OPCODE_RESET_FVAL                                                                  0x9
#define HWIO_EE_n_GSI_CH_CMD_OPCODE_DE_ALLOC_FVAL                                                               0xa
#define HWIO_EE_n_GSI_CH_CMD_OPCODE_DB_STOP_FVAL                                                                0xb
#define HWIO_EE_n_GSI_CH_CMD_CHID_BMSK                                                                         0xff
#define HWIO_EE_n_GSI_CH_CMD_CHID_SHFT                                                                          0x0

#define HWIO_EE_n_EV_CH_CMD_ADDR(n)                                                                      (GSI_REG_BASE      + 0x00025010 + 0x12000 * (n))
#define HWIO_EE_n_EV_CH_CMD_PHYS(n)                                                                      (GSI_REG_BASE_PHYS + 0x00025010 + 0x12000 * (n))
#define HWIO_EE_n_EV_CH_CMD_OFFS(n)                                                                      (GSI_REG_BASE_OFFS + 0x00025010 + 0x12000 * (n))
#define HWIO_EE_n_EV_CH_CMD_RMSK                                                                         0xff0000ff
#define HWIO_EE_n_EV_CH_CMD_MAXn                                                                                  2
#define HWIO_EE_n_EV_CH_CMD_ATTR                                                                                0x2
#define HWIO_EE_n_EV_CH_CMD_OUTI(n,val)    \
        out_dword(HWIO_EE_n_EV_CH_CMD_ADDR(n),val)
#define HWIO_EE_n_EV_CH_CMD_OPCODE_BMSK                                                                  0xff000000
#define HWIO_EE_n_EV_CH_CMD_OPCODE_SHFT                                                                        0x18
#define HWIO_EE_n_EV_CH_CMD_OPCODE_ALLOCATE_FVAL                                                                0x0
#define HWIO_EE_n_EV_CH_CMD_OPCODE_RESET_FVAL                                                                   0x9
#define HWIO_EE_n_EV_CH_CMD_OPCODE_DE_ALLOC_FVAL                                                                0xa
#define HWIO_EE_n_EV_CH_CMD_CHID_BMSK                                                                          0xff
#define HWIO_EE_n_EV_CH_CMD_CHID_SHFT                                                                           0x0

#define HWIO_EE_n_GSI_EE_GENERIC_CMD_ADDR(n)                                                             (GSI_REG_BASE      + 0x00025018 + 0x12000 * (n))
#define HWIO_EE_n_GSI_EE_GENERIC_CMD_PHYS(n)                                                             (GSI_REG_BASE_PHYS + 0x00025018 + 0x12000 * (n))
#define HWIO_EE_n_GSI_EE_GENERIC_CMD_OFFS(n)                                                             (GSI_REG_BASE_OFFS + 0x00025018 + 0x12000 * (n))
#define HWIO_EE_n_GSI_EE_GENERIC_CMD_RMSK                                                                0xffffffff
#define HWIO_EE_n_GSI_EE_GENERIC_CMD_MAXn                                                                         2
#define HWIO_EE_n_GSI_EE_GENERIC_CMD_ATTR                                                                       0x2
#define HWIO_EE_n_GSI_EE_GENERIC_CMD_OUTI(n,val)    \
        out_dword(HWIO_EE_n_GSI_EE_GENERIC_CMD_ADDR(n),val)
#define HWIO_EE_n_GSI_EE_GENERIC_CMD_OPCODE_BMSK                                                         0xffffffff
#define HWIO_EE_n_GSI_EE_GENERIC_CMD_OPCODE_SHFT                                                                0x0

#define HWIO_EE_n_GSI_HW_PARAM_0_ADDR(n)                                                                 (GSI_REG_BASE      + 0x00025038 + 0x12000 * (n))
#define HWIO_EE_n_GSI_HW_PARAM_0_PHYS(n)                                                                 (GSI_REG_BASE_PHYS + 0x00025038 + 0x12000 * (n))
#define HWIO_EE_n_GSI_HW_PARAM_0_OFFS(n)                                                                 (GSI_REG_BASE_OFFS + 0x00025038 + 0x12000 * (n))
#define HWIO_EE_n_GSI_HW_PARAM_0_RMSK                                                                    0xffffffff
#define HWIO_EE_n_GSI_HW_PARAM_0_MAXn                                                                             2
#define HWIO_EE_n_GSI_HW_PARAM_0_ATTR                                                                           0x1
#define HWIO_EE_n_GSI_HW_PARAM_0_INI(n)        \
        in_dword_masked(HWIO_EE_n_GSI_HW_PARAM_0_ADDR(n), HWIO_EE_n_GSI_HW_PARAM_0_RMSK)
#define HWIO_EE_n_GSI_HW_PARAM_0_INMI(n,mask)    \
        in_dword_masked(HWIO_EE_n_GSI_HW_PARAM_0_ADDR(n), mask)
#define HWIO_EE_n_GSI_HW_PARAM_0_USE_AXI_M_BMSK                                                          0x80000000
#define HWIO_EE_n_GSI_HW_PARAM_0_USE_AXI_M_SHFT                                                                0x1f
#define HWIO_EE_n_GSI_HW_PARAM_0_PERIPH_SEC_GRP_BMSK                                                     0x7c000000
#define HWIO_EE_n_GSI_HW_PARAM_0_PERIPH_SEC_GRP_SHFT                                                           0x1a
#define HWIO_EE_n_GSI_HW_PARAM_0_PERIPH_CONF_ADDR_BUS_W_BMSK                                              0x3e00000
#define HWIO_EE_n_GSI_HW_PARAM_0_PERIPH_CONF_ADDR_BUS_W_SHFT                                                   0x15
#define HWIO_EE_n_GSI_HW_PARAM_0_NUM_EES_BMSK                                                              0x1f0000
#define HWIO_EE_n_GSI_HW_PARAM_0_NUM_EES_SHFT                                                                  0x10
#define HWIO_EE_n_GSI_HW_PARAM_0_GSI_CH_NUM_BMSK                                                             0xff00
#define HWIO_EE_n_GSI_HW_PARAM_0_GSI_CH_NUM_SHFT                                                                0x8
#define HWIO_EE_n_GSI_HW_PARAM_0_GSI_EV_CH_NUM_BMSK                                                            0xff
#define HWIO_EE_n_GSI_HW_PARAM_0_GSI_EV_CH_NUM_SHFT                                                             0x0

#define HWIO_EE_n_GSI_HW_PARAM_1_ADDR(n)                                                                 (GSI_REG_BASE      + 0x0002503c + 0x12000 * (n))
#define HWIO_EE_n_GSI_HW_PARAM_1_PHYS(n)                                                                 (GSI_REG_BASE_PHYS + 0x0002503c + 0x12000 * (n))
#define HWIO_EE_n_GSI_HW_PARAM_1_OFFS(n)                                                                 (GSI_REG_BASE_OFFS + 0x0002503c + 0x12000 * (n))
#define HWIO_EE_n_GSI_HW_PARAM_1_RMSK                                                                    0xffffffff
#define HWIO_EE_n_GSI_HW_PARAM_1_MAXn                                                                             2
#define HWIO_EE_n_GSI_HW_PARAM_1_ATTR                                                                           0x1
#define HWIO_EE_n_GSI_HW_PARAM_1_INI(n)        \
        in_dword_masked(HWIO_EE_n_GSI_HW_PARAM_1_ADDR(n), HWIO_EE_n_GSI_HW_PARAM_1_RMSK)
#define HWIO_EE_n_GSI_HW_PARAM_1_INMI(n,mask)    \
        in_dword_masked(HWIO_EE_n_GSI_HW_PARAM_1_ADDR(n), mask)
#define HWIO_EE_n_GSI_HW_PARAM_1_GSI_BLK_INT_ACCESS_REGION_2_EN_BMSK                                     0x80000000
#define HWIO_EE_n_GSI_HW_PARAM_1_GSI_BLK_INT_ACCESS_REGION_2_EN_SHFT                                           0x1f
#define HWIO_EE_n_GSI_HW_PARAM_1_GSI_BLK_INT_ACCESS_REGION_1_EN_BMSK                                     0x40000000
#define HWIO_EE_n_GSI_HW_PARAM_1_GSI_BLK_INT_ACCESS_REGION_1_EN_SHFT                                           0x1e
#define HWIO_EE_n_GSI_HW_PARAM_1_GSI_SIMPLE_RD_WR_BMSK                                                   0x20000000
#define HWIO_EE_n_GSI_HW_PARAM_1_GSI_SIMPLE_RD_WR_SHFT                                                         0x1d
#define HWIO_EE_n_GSI_HW_PARAM_1_GSI_ESCAPE_BUF_ONLY_BMSK                                                0x10000000
#define HWIO_EE_n_GSI_HW_PARAM_1_GSI_ESCAPE_BUF_ONLY_SHFT                                                      0x1c
#define HWIO_EE_n_GSI_HW_PARAM_1_GSI_USE_UC_IF_BMSK                                                       0x8000000
#define HWIO_EE_n_GSI_HW_PARAM_1_GSI_USE_UC_IF_SHFT                                                            0x1b
#define HWIO_EE_n_GSI_HW_PARAM_1_GSI_USE_DB_ENG_BMSK                                                      0x4000000
#define HWIO_EE_n_GSI_HW_PARAM_1_GSI_USE_DB_ENG_SHFT                                                           0x1a
#define HWIO_EE_n_GSI_HW_PARAM_1_GSI_USE_BP_MTRIX_BMSK                                                    0x2000000
#define HWIO_EE_n_GSI_HW_PARAM_1_GSI_USE_BP_MTRIX_SHFT                                                         0x19
#define HWIO_EE_n_GSI_HW_PARAM_1_GSI_NUM_TIMERS_BMSK                                                      0x1f00000
#define HWIO_EE_n_GSI_HW_PARAM_1_GSI_NUM_TIMERS_SHFT                                                           0x14
#define HWIO_EE_n_GSI_HW_PARAM_1_GSI_USE_XPU_BMSK                                                           0x80000
#define HWIO_EE_n_GSI_HW_PARAM_1_GSI_USE_XPU_SHFT                                                              0x13
#define HWIO_EE_n_GSI_HW_PARAM_1_GSI_QRIB_EN_BMSK                                                           0x40000
#define HWIO_EE_n_GSI_HW_PARAM_1_GSI_QRIB_EN_SHFT                                                              0x12
#define HWIO_EE_n_GSI_HW_PARAM_1_GSI_VMIDACR_EN_BMSK                                                        0x20000
#define HWIO_EE_n_GSI_HW_PARAM_1_GSI_VMIDACR_EN_SHFT                                                           0x11
#define HWIO_EE_n_GSI_HW_PARAM_1_GSI_SEC_EN_BMSK                                                            0x10000
#define HWIO_EE_n_GSI_HW_PARAM_1_GSI_SEC_EN_SHFT                                                               0x10
#define HWIO_EE_n_GSI_HW_PARAM_1_GSI_NONSEC_EN_BMSK                                                          0xf000
#define HWIO_EE_n_GSI_HW_PARAM_1_GSI_NONSEC_EN_SHFT                                                             0xc
#define HWIO_EE_n_GSI_HW_PARAM_1_GSI_NUM_QAD_BMSK                                                             0xf00
#define HWIO_EE_n_GSI_HW_PARAM_1_GSI_NUM_QAD_SHFT                                                               0x8
#define HWIO_EE_n_GSI_HW_PARAM_1_GSI_M_DATA_BUS_W_BMSK                                                         0xff
#define HWIO_EE_n_GSI_HW_PARAM_1_GSI_M_DATA_BUS_W_SHFT                                                          0x0

#define HWIO_EE_n_GSI_HW_PARAM_2_ADDR(n)                                                                 (GSI_REG_BASE      + 0x00025040 + 0x12000 * (n))
#define HWIO_EE_n_GSI_HW_PARAM_2_PHYS(n)                                                                 (GSI_REG_BASE_PHYS + 0x00025040 + 0x12000 * (n))
#define HWIO_EE_n_GSI_HW_PARAM_2_OFFS(n)                                                                 (GSI_REG_BASE_OFFS + 0x00025040 + 0x12000 * (n))
#define HWIO_EE_n_GSI_HW_PARAM_2_RMSK                                                                    0xffffffff
#define HWIO_EE_n_GSI_HW_PARAM_2_MAXn                                                                             2
#define HWIO_EE_n_GSI_HW_PARAM_2_ATTR                                                                           0x1
#define HWIO_EE_n_GSI_HW_PARAM_2_INI(n)        \
        in_dword_masked(HWIO_EE_n_GSI_HW_PARAM_2_ADDR(n), HWIO_EE_n_GSI_HW_PARAM_2_RMSK)
#define HWIO_EE_n_GSI_HW_PARAM_2_INMI(n,mask)    \
        in_dword_masked(HWIO_EE_n_GSI_HW_PARAM_2_ADDR(n), mask)
#define HWIO_EE_n_GSI_HW_PARAM_2_GSI_USE_INTER_EE_BMSK                                                   0x80000000
#define HWIO_EE_n_GSI_HW_PARAM_2_GSI_USE_INTER_EE_SHFT                                                         0x1f
#define HWIO_EE_n_GSI_HW_PARAM_2_GSI_USE_RD_WR_ENG_BMSK                                                  0x40000000
#define HWIO_EE_n_GSI_HW_PARAM_2_GSI_USE_RD_WR_ENG_SHFT                                                        0x1e
#define HWIO_EE_n_GSI_HW_PARAM_2_GSI_SDMA_N_IOVEC_BMSK                                                   0x38000000
#define HWIO_EE_n_GSI_HW_PARAM_2_GSI_SDMA_N_IOVEC_SHFT                                                         0x1b
#define HWIO_EE_n_GSI_HW_PARAM_2_GSI_SDMA_MAX_BURST_BMSK                                                  0x7f80000
#define HWIO_EE_n_GSI_HW_PARAM_2_GSI_SDMA_MAX_BURST_SHFT                                                       0x13
#define HWIO_EE_n_GSI_HW_PARAM_2_GSI_SDMA_N_INT_BMSK                                                        0x70000
#define HWIO_EE_n_GSI_HW_PARAM_2_GSI_SDMA_N_INT_SHFT                                                           0x10
#define HWIO_EE_n_GSI_HW_PARAM_2_GSI_USE_SDMA_BMSK                                                           0x8000
#define HWIO_EE_n_GSI_HW_PARAM_2_GSI_USE_SDMA_SHFT                                                              0xf
#define HWIO_EE_n_GSI_HW_PARAM_2_GSI_CH_FULL_LOGIC_BMSK                                                      0x4000
#define HWIO_EE_n_GSI_HW_PARAM_2_GSI_CH_FULL_LOGIC_SHFT                                                         0xe
#define HWIO_EE_n_GSI_HW_PARAM_2_GSI_CH_PEND_TRANSLATE_BMSK                                                  0x2000
#define HWIO_EE_n_GSI_HW_PARAM_2_GSI_CH_PEND_TRANSLATE_SHFT                                                     0xd
#define HWIO_EE_n_GSI_HW_PARAM_2_GSI_IRAM_SIZE_BMSK                                                          0x1f00
#define HWIO_EE_n_GSI_HW_PARAM_2_GSI_IRAM_SIZE_SHFT                                                             0x8
#define HWIO_EE_n_GSI_HW_PARAM_2_GSI_IRAM_SIZE_ONE_KB_FVAL                                                      0x0
#define HWIO_EE_n_GSI_HW_PARAM_2_GSI_IRAM_SIZE_TWO_KB_FVAL                                                      0x1
#define HWIO_EE_n_GSI_HW_PARAM_2_GSI_IRAM_SIZE_TWO_N_HALF_KB_FVAL                                               0x2
#define HWIO_EE_n_GSI_HW_PARAM_2_GSI_IRAM_SIZE_THREE_KB_FVAL                                                    0x3
#define HWIO_EE_n_GSI_HW_PARAM_2_GSI_IRAM_SIZE_THREE_N_HALF_KB_FVAL                                             0x4
#define HWIO_EE_n_GSI_HW_PARAM_2_GSI_IRAM_SIZE_FOUR_KB_FVAL                                                     0x5
#define HWIO_EE_n_GSI_HW_PARAM_2_GSI_NUM_CH_PER_EE_BMSK                                                        0xff
#define HWIO_EE_n_GSI_HW_PARAM_2_GSI_NUM_CH_PER_EE_SHFT                                                         0x0

#define HWIO_EE_n_GSI_SW_VERSION_ADDR(n)                                                                 (GSI_REG_BASE      + 0x00025044 + 0x12000 * (n))
#define HWIO_EE_n_GSI_SW_VERSION_PHYS(n)                                                                 (GSI_REG_BASE_PHYS + 0x00025044 + 0x12000 * (n))
#define HWIO_EE_n_GSI_SW_VERSION_OFFS(n)                                                                 (GSI_REG_BASE_OFFS + 0x00025044 + 0x12000 * (n))
#define HWIO_EE_n_GSI_SW_VERSION_RMSK                                                                    0xffffffff
#define HWIO_EE_n_GSI_SW_VERSION_MAXn                                                                             2
#define HWIO_EE_n_GSI_SW_VERSION_ATTR                                                                           0x1
#define HWIO_EE_n_GSI_SW_VERSION_INI(n)        \
        in_dword_masked(HWIO_EE_n_GSI_SW_VERSION_ADDR(n), HWIO_EE_n_GSI_SW_VERSION_RMSK)
#define HWIO_EE_n_GSI_SW_VERSION_INMI(n,mask)    \
        in_dword_masked(HWIO_EE_n_GSI_SW_VERSION_ADDR(n), mask)
#define HWIO_EE_n_GSI_SW_VERSION_MAJOR_BMSK                                                              0xf0000000
#define HWIO_EE_n_GSI_SW_VERSION_MAJOR_SHFT                                                                    0x1c
#define HWIO_EE_n_GSI_SW_VERSION_MINOR_BMSK                                                               0xfff0000
#define HWIO_EE_n_GSI_SW_VERSION_MINOR_SHFT                                                                    0x10
#define HWIO_EE_n_GSI_SW_VERSION_STEP_BMSK                                                                   0xffff
#define HWIO_EE_n_GSI_SW_VERSION_STEP_SHFT                                                                      0x0

#define HWIO_EE_n_GSI_MCS_CODE_VER_ADDR(n)                                                               (GSI_REG_BASE      + 0x00025048 + 0x12000 * (n))
#define HWIO_EE_n_GSI_MCS_CODE_VER_PHYS(n)                                                               (GSI_REG_BASE_PHYS + 0x00025048 + 0x12000 * (n))
#define HWIO_EE_n_GSI_MCS_CODE_VER_OFFS(n)                                                               (GSI_REG_BASE_OFFS + 0x00025048 + 0x12000 * (n))
#define HWIO_EE_n_GSI_MCS_CODE_VER_RMSK                                                                  0xffffffff
#define HWIO_EE_n_GSI_MCS_CODE_VER_MAXn                                                                           2
#define HWIO_EE_n_GSI_MCS_CODE_VER_ATTR                                                                         0x1
#define HWIO_EE_n_GSI_MCS_CODE_VER_INI(n)        \
        in_dword_masked(HWIO_EE_n_GSI_MCS_CODE_VER_ADDR(n), HWIO_EE_n_GSI_MCS_CODE_VER_RMSK)
#define HWIO_EE_n_GSI_MCS_CODE_VER_INMI(n,mask)    \
        in_dword_masked(HWIO_EE_n_GSI_MCS_CODE_VER_ADDR(n), mask)
#define HWIO_EE_n_GSI_MCS_CODE_VER_VER_BMSK                                                              0xffffffff
#define HWIO_EE_n_GSI_MCS_CODE_VER_VER_SHFT                                                                     0x0

#define HWIO_EE_n_GSI_HW_PARAM_3_ADDR(n)                                                                 (GSI_REG_BASE      + 0x0002504c + 0x12000 * (n))
#define HWIO_EE_n_GSI_HW_PARAM_3_PHYS(n)                                                                 (GSI_REG_BASE_PHYS + 0x0002504c + 0x12000 * (n))
#define HWIO_EE_n_GSI_HW_PARAM_3_OFFS(n)                                                                 (GSI_REG_BASE_OFFS + 0x0002504c + 0x12000 * (n))
#define HWIO_EE_n_GSI_HW_PARAM_3_RMSK                                                                    0x1fffffff
#define HWIO_EE_n_GSI_HW_PARAM_3_MAXn                                                                             2
#define HWIO_EE_n_GSI_HW_PARAM_3_ATTR                                                                           0x1
#define HWIO_EE_n_GSI_HW_PARAM_3_INI(n)        \
        in_dword_masked(HWIO_EE_n_GSI_HW_PARAM_3_ADDR(n), HWIO_EE_n_GSI_HW_PARAM_3_RMSK)
#define HWIO_EE_n_GSI_HW_PARAM_3_INMI(n,mask)    \
        in_dword_masked(HWIO_EE_n_GSI_HW_PARAM_3_ADDR(n), mask)
#define HWIO_EE_n_GSI_HW_PARAM_3_GSI_USE_DB_MSI_MODE_BMSK                                                0x10000000
#define HWIO_EE_n_GSI_HW_PARAM_3_GSI_USE_DB_MSI_MODE_SHFT                                                      0x1c
#define HWIO_EE_n_GSI_HW_PARAM_3_GSI_USE_SLEEP_CLK_DIV_BMSK                                               0x8000000
#define HWIO_EE_n_GSI_HW_PARAM_3_GSI_USE_SLEEP_CLK_DIV_SHFT                                                    0x1b
#define HWIO_EE_n_GSI_HW_PARAM_3_GSI_USE_VIR_CH_IF_BMSK                                                   0x4000000
#define HWIO_EE_n_GSI_HW_PARAM_3_GSI_USE_VIR_CH_IF_SHFT                                                        0x1a
#define HWIO_EE_n_GSI_HW_PARAM_3_GSI_USE_IROM_BMSK                                                        0x2000000
#define HWIO_EE_n_GSI_HW_PARAM_3_GSI_USE_IROM_SHFT                                                             0x19
#define HWIO_EE_n_GSI_HW_PARAM_3_GSI_REE_MAX_BURST_LEN_BMSK                                               0x1f00000
#define HWIO_EE_n_GSI_HW_PARAM_3_GSI_REE_MAX_BURST_LEN_SHFT                                                    0x14
#define HWIO_EE_n_GSI_HW_PARAM_3_GSI_M_ADDR_BUS_W_BMSK                                                      0xff000
#define HWIO_EE_n_GSI_HW_PARAM_3_GSI_M_ADDR_BUS_W_SHFT                                                          0xc
#define HWIO_EE_n_GSI_HW_PARAM_3_GSI_NUM_PREFETCH_BUFS_BMSK                                                   0xf00
#define HWIO_EE_n_GSI_HW_PARAM_3_GSI_NUM_PREFETCH_BUFS_SHFT                                                     0x8
#define HWIO_EE_n_GSI_HW_PARAM_3_GSI_SDMA_MAX_OS_WR_BMSK                                                       0xf0
#define HWIO_EE_n_GSI_HW_PARAM_3_GSI_SDMA_MAX_OS_WR_SHFT                                                        0x4
#define HWIO_EE_n_GSI_HW_PARAM_3_GSI_SDMA_MAX_OS_RD_BMSK                                                        0xf
#define HWIO_EE_n_GSI_HW_PARAM_3_GSI_SDMA_MAX_OS_RD_SHFT                                                        0x0

#define HWIO_EE_n_GSI_HW_PARAM_4_ADDR(n)                                                                 (GSI_REG_BASE      + 0x00025050 + 0x12000 * (n))
#define HWIO_EE_n_GSI_HW_PARAM_4_PHYS(n)                                                                 (GSI_REG_BASE_PHYS + 0x00025050 + 0x12000 * (n))
#define HWIO_EE_n_GSI_HW_PARAM_4_OFFS(n)                                                                 (GSI_REG_BASE_OFFS + 0x00025050 + 0x12000 * (n))
#define HWIO_EE_n_GSI_HW_PARAM_4_RMSK                                                                        0xffff
#define HWIO_EE_n_GSI_HW_PARAM_4_MAXn                                                                             2
#define HWIO_EE_n_GSI_HW_PARAM_4_ATTR                                                                           0x1
#define HWIO_EE_n_GSI_HW_PARAM_4_INI(n)        \
        in_dword_masked(HWIO_EE_n_GSI_HW_PARAM_4_ADDR(n), HWIO_EE_n_GSI_HW_PARAM_4_RMSK)
#define HWIO_EE_n_GSI_HW_PARAM_4_INMI(n,mask)    \
        in_dword_masked(HWIO_EE_n_GSI_HW_PARAM_4_ADDR(n), mask)
#define HWIO_EE_n_GSI_HW_PARAM_4_GSI_IRAM_PROTCOL_CNT_BMSK                                                   0xff00
#define HWIO_EE_n_GSI_HW_PARAM_4_GSI_IRAM_PROTCOL_CNT_SHFT                                                      0x8
#define HWIO_EE_n_GSI_HW_PARAM_4_GSI_NUM_EV_PER_EE_BMSK                                                        0xff
#define HWIO_EE_n_GSI_HW_PARAM_4_GSI_NUM_EV_PER_EE_SHFT                                                         0x0

#define HWIO_EE_n_CNTXT_TYPE_IRQ_ADDR(n)                                                                 (GSI_REG_BASE      + 0x00025080 + 0x12000 * (n))
#define HWIO_EE_n_CNTXT_TYPE_IRQ_PHYS(n)                                                                 (GSI_REG_BASE_PHYS + 0x00025080 + 0x12000 * (n))
#define HWIO_EE_n_CNTXT_TYPE_IRQ_OFFS(n)                                                                 (GSI_REG_BASE_OFFS + 0x00025080 + 0x12000 * (n))
#define HWIO_EE_n_CNTXT_TYPE_IRQ_RMSK                                                                          0x7f
#define HWIO_EE_n_CNTXT_TYPE_IRQ_MAXn                                                                             2
#define HWIO_EE_n_CNTXT_TYPE_IRQ_ATTR                                                                           0x1
#define HWIO_EE_n_CNTXT_TYPE_IRQ_INI(n)        \
        in_dword_masked(HWIO_EE_n_CNTXT_TYPE_IRQ_ADDR(n), HWIO_EE_n_CNTXT_TYPE_IRQ_RMSK)
#define HWIO_EE_n_CNTXT_TYPE_IRQ_INMI(n,mask)    \
        in_dword_masked(HWIO_EE_n_CNTXT_TYPE_IRQ_ADDR(n), mask)
#define HWIO_EE_n_CNTXT_TYPE_IRQ_GENERAL_BMSK                                                                  0x40
#define HWIO_EE_n_CNTXT_TYPE_IRQ_GENERAL_SHFT                                                                   0x6
#define HWIO_EE_n_CNTXT_TYPE_IRQ_INTER_EE_EV_CTRL_BMSK                                                         0x20
#define HWIO_EE_n_CNTXT_TYPE_IRQ_INTER_EE_EV_CTRL_SHFT                                                          0x5
#define HWIO_EE_n_CNTXT_TYPE_IRQ_INTER_EE_CH_CTRL_BMSK                                                         0x10
#define HWIO_EE_n_CNTXT_TYPE_IRQ_INTER_EE_CH_CTRL_SHFT                                                          0x4
#define HWIO_EE_n_CNTXT_TYPE_IRQ_IEOB_BMSK                                                                      0x8
#define HWIO_EE_n_CNTXT_TYPE_IRQ_IEOB_SHFT                                                                      0x3
#define HWIO_EE_n_CNTXT_TYPE_IRQ_GLOB_EE_BMSK                                                                   0x4
#define HWIO_EE_n_CNTXT_TYPE_IRQ_GLOB_EE_SHFT                                                                   0x2
#define HWIO_EE_n_CNTXT_TYPE_IRQ_EV_CTRL_BMSK                                                                   0x2
#define HWIO_EE_n_CNTXT_TYPE_IRQ_EV_CTRL_SHFT                                                                   0x1
#define HWIO_EE_n_CNTXT_TYPE_IRQ_CH_CTRL_BMSK                                                                   0x1
#define HWIO_EE_n_CNTXT_TYPE_IRQ_CH_CTRL_SHFT                                                                   0x0

#define HWIO_EE_n_CNTXT_TYPE_IRQ_MSK_ADDR(n)                                                             (GSI_REG_BASE      + 0x00025088 + 0x12000 * (n))
#define HWIO_EE_n_CNTXT_TYPE_IRQ_MSK_PHYS(n)                                                             (GSI_REG_BASE_PHYS + 0x00025088 + 0x12000 * (n))
#define HWIO_EE_n_CNTXT_TYPE_IRQ_MSK_OFFS(n)                                                             (GSI_REG_BASE_OFFS + 0x00025088 + 0x12000 * (n))
#define HWIO_EE_n_CNTXT_TYPE_IRQ_MSK_RMSK                                                                      0x7f
#define HWIO_EE_n_CNTXT_TYPE_IRQ_MSK_MAXn                                                                         2
#define HWIO_EE_n_CNTXT_TYPE_IRQ_MSK_ATTR                                                                       0x3
#define HWIO_EE_n_CNTXT_TYPE_IRQ_MSK_INI(n)        \
        in_dword_masked(HWIO_EE_n_CNTXT_TYPE_IRQ_MSK_ADDR(n), HWIO_EE_n_CNTXT_TYPE_IRQ_MSK_RMSK)
#define HWIO_EE_n_CNTXT_TYPE_IRQ_MSK_INMI(n,mask)    \
        in_dword_masked(HWIO_EE_n_CNTXT_TYPE_IRQ_MSK_ADDR(n), mask)
#define HWIO_EE_n_CNTXT_TYPE_IRQ_MSK_OUTI(n,val)    \
        out_dword(HWIO_EE_n_CNTXT_TYPE_IRQ_MSK_ADDR(n),val)
#define HWIO_EE_n_CNTXT_TYPE_IRQ_MSK_OUTMI(n,mask,val) \
        out_dword_masked_ns(HWIO_EE_n_CNTXT_TYPE_IRQ_MSK_ADDR(n),mask,val,HWIO_EE_n_CNTXT_TYPE_IRQ_MSK_INI(n))
#define HWIO_EE_n_CNTXT_TYPE_IRQ_MSK_GENERAL_BMSK                                                              0x40
#define HWIO_EE_n_CNTXT_TYPE_IRQ_MSK_GENERAL_SHFT                                                               0x6
#define HWIO_EE_n_CNTXT_TYPE_IRQ_MSK_INTER_EE_EV_CTRL_BMSK                                                     0x20
#define HWIO_EE_n_CNTXT_TYPE_IRQ_MSK_INTER_EE_EV_CTRL_SHFT                                                      0x5
#define HWIO_EE_n_CNTXT_TYPE_IRQ_MSK_INTER_EE_CH_CTRL_BMSK                                                     0x10
#define HWIO_EE_n_CNTXT_TYPE_IRQ_MSK_INTER_EE_CH_CTRL_SHFT                                                      0x4
#define HWIO_EE_n_CNTXT_TYPE_IRQ_MSK_IEOB_BMSK                                                                  0x8
#define HWIO_EE_n_CNTXT_TYPE_IRQ_MSK_IEOB_SHFT                                                                  0x3
#define HWIO_EE_n_CNTXT_TYPE_IRQ_MSK_GLOB_EE_BMSK                                                               0x4
#define HWIO_EE_n_CNTXT_TYPE_IRQ_MSK_GLOB_EE_SHFT                                                               0x2
#define HWIO_EE_n_CNTXT_TYPE_IRQ_MSK_EV_CTRL_BMSK                                                               0x2
#define HWIO_EE_n_CNTXT_TYPE_IRQ_MSK_EV_CTRL_SHFT                                                               0x1
#define HWIO_EE_n_CNTXT_TYPE_IRQ_MSK_CH_CTRL_BMSK                                                               0x1
#define HWIO_EE_n_CNTXT_TYPE_IRQ_MSK_CH_CTRL_SHFT                                                               0x0

#define HWIO_EE_n_CNTXT_SRC_GSI_CH_IRQ_k_ADDR(n,k)                                                       (GSI_REG_BASE      + 0x00025090 + 0x24 * (k) + 0x12000 * (n))
#define HWIO_EE_n_CNTXT_SRC_GSI_CH_IRQ_k_PHYS(n,k)                                                       (GSI_REG_BASE_PHYS + 0x00025090 + 0x24 * (k) + 0x12000 * (n))
#define HWIO_EE_n_CNTXT_SRC_GSI_CH_IRQ_k_OFFS(n,k)                                                       (GSI_REG_BASE_OFFS + 0x00025090 + 0x24 * (k) + 0x12000 * (n))
#define HWIO_EE_n_CNTXT_SRC_GSI_CH_IRQ_k_RMSK                                                            0xffffffff
#define HWIO_EE_n_CNTXT_SRC_GSI_CH_IRQ_k_MAXn                                                                     2
#define HWIO_EE_n_CNTXT_SRC_GSI_CH_IRQ_k_MAXk                                                                     0
#define HWIO_EE_n_CNTXT_SRC_GSI_CH_IRQ_k_ATTR                                                                   0x1
#define HWIO_EE_n_CNTXT_SRC_GSI_CH_IRQ_k_INI2(n,k)        \
        in_dword_masked(HWIO_EE_n_CNTXT_SRC_GSI_CH_IRQ_k_ADDR(n,k), HWIO_EE_n_CNTXT_SRC_GSI_CH_IRQ_k_RMSK)
#define HWIO_EE_n_CNTXT_SRC_GSI_CH_IRQ_k_INMI2(n,k,mask)    \
        in_dword_masked(HWIO_EE_n_CNTXT_SRC_GSI_CH_IRQ_k_ADDR(n,k), mask)
#define HWIO_EE_n_CNTXT_SRC_GSI_CH_IRQ_k_GSI_CH_BIT_MAP_BMSK                                             0xffffffff
#define HWIO_EE_n_CNTXT_SRC_GSI_CH_IRQ_k_GSI_CH_BIT_MAP_SHFT                                                    0x0

#define HWIO_EE_n_CNTXT_SRC_GSI_CH_IRQ_MSK_k_ADDR(n,k)                                                   (GSI_REG_BASE      + 0x00025094 + 0x24 * (k) + 0x12000 * (n))
#define HWIO_EE_n_CNTXT_SRC_GSI_CH_IRQ_MSK_k_PHYS(n,k)                                                   (GSI_REG_BASE_PHYS + 0x00025094 + 0x24 * (k) + 0x12000 * (n))
#define HWIO_EE_n_CNTXT_SRC_GSI_CH_IRQ_MSK_k_OFFS(n,k)                                                   (GSI_REG_BASE_OFFS + 0x00025094 + 0x24 * (k) + 0x12000 * (n))
#define HWIO_EE_n_CNTXT_SRC_GSI_CH_IRQ_MSK_k_RMSK                                                        0xffffffff
#define HWIO_EE_n_CNTXT_SRC_GSI_CH_IRQ_MSK_k_MAXn                                                                 2
#define HWIO_EE_n_CNTXT_SRC_GSI_CH_IRQ_MSK_k_MAXk                                                                 0
#define HWIO_EE_n_CNTXT_SRC_GSI_CH_IRQ_MSK_k_ATTR                                                               0x3
#define HWIO_EE_n_CNTXT_SRC_GSI_CH_IRQ_MSK_k_INI2(n,k)        \
        in_dword_masked(HWIO_EE_n_CNTXT_SRC_GSI_CH_IRQ_MSK_k_ADDR(n,k), HWIO_EE_n_CNTXT_SRC_GSI_CH_IRQ_MSK_k_RMSK)
#define HWIO_EE_n_CNTXT_SRC_GSI_CH_IRQ_MSK_k_INMI2(n,k,mask)    \
        in_dword_masked(HWIO_EE_n_CNTXT_SRC_GSI_CH_IRQ_MSK_k_ADDR(n,k), mask)
#define HWIO_EE_n_CNTXT_SRC_GSI_CH_IRQ_MSK_k_OUTI2(n,k,val)    \
        out_dword(HWIO_EE_n_CNTXT_SRC_GSI_CH_IRQ_MSK_k_ADDR(n,k),val)
#define HWIO_EE_n_CNTXT_SRC_GSI_CH_IRQ_MSK_k_OUTMI2(n,k,mask,val) \
        out_dword_masked_ns(HWIO_EE_n_CNTXT_SRC_GSI_CH_IRQ_MSK_k_ADDR(n,k),mask,val,HWIO_EE_n_CNTXT_SRC_GSI_CH_IRQ_MSK_k_INI2(n,k))
#define HWIO_EE_n_CNTXT_SRC_GSI_CH_IRQ_MSK_k_GSI_CH_BIT_MAP_MSK_BMSK                                     0xffffffff
#define HWIO_EE_n_CNTXT_SRC_GSI_CH_IRQ_MSK_k_GSI_CH_BIT_MAP_MSK_SHFT                                            0x0

#define HWIO_EE_n_CNTXT_SRC_GSI_CH_IRQ_CLR_k_ADDR(n,k)                                                   (GSI_REG_BASE      + 0x00025098 + 0x24 * (k) + 0x12000 * (n))
#define HWIO_EE_n_CNTXT_SRC_GSI_CH_IRQ_CLR_k_PHYS(n,k)                                                   (GSI_REG_BASE_PHYS + 0x00025098 + 0x24 * (k) + 0x12000 * (n))
#define HWIO_EE_n_CNTXT_SRC_GSI_CH_IRQ_CLR_k_OFFS(n,k)                                                   (GSI_REG_BASE_OFFS + 0x00025098 + 0x24 * (k) + 0x12000 * (n))
#define HWIO_EE_n_CNTXT_SRC_GSI_CH_IRQ_CLR_k_RMSK                                                        0xffffffff
#define HWIO_EE_n_CNTXT_SRC_GSI_CH_IRQ_CLR_k_MAXn                                                                 2
#define HWIO_EE_n_CNTXT_SRC_GSI_CH_IRQ_CLR_k_MAXk                                                                 0
#define HWIO_EE_n_CNTXT_SRC_GSI_CH_IRQ_CLR_k_ATTR                                                               0x2
#define HWIO_EE_n_CNTXT_SRC_GSI_CH_IRQ_CLR_k_OUTI2(n,k,val)    \
        out_dword(HWIO_EE_n_CNTXT_SRC_GSI_CH_IRQ_CLR_k_ADDR(n,k),val)
#define HWIO_EE_n_CNTXT_SRC_GSI_CH_IRQ_CLR_k_GSI_CH_BIT_MAP_BMSK                                         0xffffffff
#define HWIO_EE_n_CNTXT_SRC_GSI_CH_IRQ_CLR_k_GSI_CH_BIT_MAP_SHFT                                                0x0

#define HWIO_EE_n_CNTXT_SRC_EV_CH_IRQ_k_ADDR(n,k)                                                        (GSI_REG_BASE      + 0x0002509c + 0x24 * (k) + 0x12000 * (n))
#define HWIO_EE_n_CNTXT_SRC_EV_CH_IRQ_k_PHYS(n,k)                                                        (GSI_REG_BASE_PHYS + 0x0002509c + 0x24 * (k) + 0x12000 * (n))
#define HWIO_EE_n_CNTXT_SRC_EV_CH_IRQ_k_OFFS(n,k)                                                        (GSI_REG_BASE_OFFS + 0x0002509c + 0x24 * (k) + 0x12000 * (n))
#define HWIO_EE_n_CNTXT_SRC_EV_CH_IRQ_k_RMSK                                                             0xffffffff
#define HWIO_EE_n_CNTXT_SRC_EV_CH_IRQ_k_MAXn                                                                      2
#define HWIO_EE_n_CNTXT_SRC_EV_CH_IRQ_k_MAXk                                                                      0
#define HWIO_EE_n_CNTXT_SRC_EV_CH_IRQ_k_ATTR                                                                    0x1
#define HWIO_EE_n_CNTXT_SRC_EV_CH_IRQ_k_INI2(n,k)        \
        in_dword_masked(HWIO_EE_n_CNTXT_SRC_EV_CH_IRQ_k_ADDR(n,k), HWIO_EE_n_CNTXT_SRC_EV_CH_IRQ_k_RMSK)
#define HWIO_EE_n_CNTXT_SRC_EV_CH_IRQ_k_INMI2(n,k,mask)    \
        in_dword_masked(HWIO_EE_n_CNTXT_SRC_EV_CH_IRQ_k_ADDR(n,k), mask)
#define HWIO_EE_n_CNTXT_SRC_EV_CH_IRQ_k_EV_CH_BIT_MAP_BMSK                                               0xffffffff
#define HWIO_EE_n_CNTXT_SRC_EV_CH_IRQ_k_EV_CH_BIT_MAP_SHFT                                                      0x0

#define HWIO_EE_n_CNTXT_SRC_EV_CH_IRQ_MSK_k_ADDR(n,k)                                                    (GSI_REG_BASE      + 0x000250a0 + 0x24 * (k) + 0x12000 * (n))
#define HWIO_EE_n_CNTXT_SRC_EV_CH_IRQ_MSK_k_PHYS(n,k)                                                    (GSI_REG_BASE_PHYS + 0x000250a0 + 0x24 * (k) + 0x12000 * (n))
#define HWIO_EE_n_CNTXT_SRC_EV_CH_IRQ_MSK_k_OFFS(n,k)                                                    (GSI_REG_BASE_OFFS + 0x000250a0 + 0x24 * (k) + 0x12000 * (n))
#define HWIO_EE_n_CNTXT_SRC_EV_CH_IRQ_MSK_k_RMSK                                                         0xffffffff
#define HWIO_EE_n_CNTXT_SRC_EV_CH_IRQ_MSK_k_MAXn                                                                  2
#define HWIO_EE_n_CNTXT_SRC_EV_CH_IRQ_MSK_k_MAXk                                                                  0
#define HWIO_EE_n_CNTXT_SRC_EV_CH_IRQ_MSK_k_ATTR                                                                0x3
#define HWIO_EE_n_CNTXT_SRC_EV_CH_IRQ_MSK_k_INI2(n,k)        \
        in_dword_masked(HWIO_EE_n_CNTXT_SRC_EV_CH_IRQ_MSK_k_ADDR(n,k), HWIO_EE_n_CNTXT_SRC_EV_CH_IRQ_MSK_k_RMSK)
#define HWIO_EE_n_CNTXT_SRC_EV_CH_IRQ_MSK_k_INMI2(n,k,mask)    \
        in_dword_masked(HWIO_EE_n_CNTXT_SRC_EV_CH_IRQ_MSK_k_ADDR(n,k), mask)
#define HWIO_EE_n_CNTXT_SRC_EV_CH_IRQ_MSK_k_OUTI2(n,k,val)    \
        out_dword(HWIO_EE_n_CNTXT_SRC_EV_CH_IRQ_MSK_k_ADDR(n,k),val)
#define HWIO_EE_n_CNTXT_SRC_EV_CH_IRQ_MSK_k_OUTMI2(n,k,mask,val) \
        out_dword_masked_ns(HWIO_EE_n_CNTXT_SRC_EV_CH_IRQ_MSK_k_ADDR(n,k),mask,val,HWIO_EE_n_CNTXT_SRC_EV_CH_IRQ_MSK_k_INI2(n,k))
#define HWIO_EE_n_CNTXT_SRC_EV_CH_IRQ_MSK_k_EV_CH_BIT_MAP_MSK_BMSK                                       0xffffffff
#define HWIO_EE_n_CNTXT_SRC_EV_CH_IRQ_MSK_k_EV_CH_BIT_MAP_MSK_SHFT                                              0x0

#define HWIO_EE_n_CNTXT_SRC_EV_CH_IRQ_CLR_k_ADDR(n,k)                                                    (GSI_REG_BASE      + 0x000250a4 + 0x24 * (k) + 0x12000 * (n))
#define HWIO_EE_n_CNTXT_SRC_EV_CH_IRQ_CLR_k_PHYS(n,k)                                                    (GSI_REG_BASE_PHYS + 0x000250a4 + 0x24 * (k) + 0x12000 * (n))
#define HWIO_EE_n_CNTXT_SRC_EV_CH_IRQ_CLR_k_OFFS(n,k)                                                    (GSI_REG_BASE_OFFS + 0x000250a4 + 0x24 * (k) + 0x12000 * (n))
#define HWIO_EE_n_CNTXT_SRC_EV_CH_IRQ_CLR_k_RMSK                                                         0xffffffff
#define HWIO_EE_n_CNTXT_SRC_EV_CH_IRQ_CLR_k_MAXn                                                                  2
#define HWIO_EE_n_CNTXT_SRC_EV_CH_IRQ_CLR_k_MAXk                                                                  0
#define HWIO_EE_n_CNTXT_SRC_EV_CH_IRQ_CLR_k_ATTR                                                                0x2
#define HWIO_EE_n_CNTXT_SRC_EV_CH_IRQ_CLR_k_OUTI2(n,k,val)    \
        out_dword(HWIO_EE_n_CNTXT_SRC_EV_CH_IRQ_CLR_k_ADDR(n,k),val)
#define HWIO_EE_n_CNTXT_SRC_EV_CH_IRQ_CLR_k_EV_CH_BIT_MAP_BMSK                                           0xffffffff
#define HWIO_EE_n_CNTXT_SRC_EV_CH_IRQ_CLR_k_EV_CH_BIT_MAP_SHFT                                                  0x0

#define HWIO_EE_n_CNTXT_SRC_IEOB_IRQ_k_ADDR(n,k)                                                         (GSI_REG_BASE      + 0x000250a8 + 0x24 * (k) + 0x12000 * (n))
#define HWIO_EE_n_CNTXT_SRC_IEOB_IRQ_k_PHYS(n,k)                                                         (GSI_REG_BASE_PHYS + 0x000250a8 + 0x24 * (k) + 0x12000 * (n))
#define HWIO_EE_n_CNTXT_SRC_IEOB_IRQ_k_OFFS(n,k)                                                         (GSI_REG_BASE_OFFS + 0x000250a8 + 0x24 * (k) + 0x12000 * (n))
#define HWIO_EE_n_CNTXT_SRC_IEOB_IRQ_k_RMSK                                                              0xffffffff
#define HWIO_EE_n_CNTXT_SRC_IEOB_IRQ_k_MAXn                                                                       2
#define HWIO_EE_n_CNTXT_SRC_IEOB_IRQ_k_MAXk                                                                       0
#define HWIO_EE_n_CNTXT_SRC_IEOB_IRQ_k_ATTR                                                                     0x1
#define HWIO_EE_n_CNTXT_SRC_IEOB_IRQ_k_INI2(n,k)        \
        in_dword_masked(HWIO_EE_n_CNTXT_SRC_IEOB_IRQ_k_ADDR(n,k), HWIO_EE_n_CNTXT_SRC_IEOB_IRQ_k_RMSK)
#define HWIO_EE_n_CNTXT_SRC_IEOB_IRQ_k_INMI2(n,k,mask)    \
        in_dword_masked(HWIO_EE_n_CNTXT_SRC_IEOB_IRQ_k_ADDR(n,k), mask)
#define HWIO_EE_n_CNTXT_SRC_IEOB_IRQ_k_EV_CH_BIT_MAP_BMSK                                                0xffffffff
#define HWIO_EE_n_CNTXT_SRC_IEOB_IRQ_k_EV_CH_BIT_MAP_SHFT                                                       0x0

#define HWIO_EE_n_CNTXT_SRC_IEOB_IRQ_MSK_k_ADDR(n,k)                                                     (GSI_REG_BASE      + 0x000250ac + 0x24 * (k) + 0x12000 * (n))
#define HWIO_EE_n_CNTXT_SRC_IEOB_IRQ_MSK_k_PHYS(n,k)                                                     (GSI_REG_BASE_PHYS + 0x000250ac + 0x24 * (k) + 0x12000 * (n))
#define HWIO_EE_n_CNTXT_SRC_IEOB_IRQ_MSK_k_OFFS(n,k)                                                     (GSI_REG_BASE_OFFS + 0x000250ac + 0x24 * (k) + 0x12000 * (n))
#define HWIO_EE_n_CNTXT_SRC_IEOB_IRQ_MSK_k_RMSK                                                          0xffffffff
#define HWIO_EE_n_CNTXT_SRC_IEOB_IRQ_MSK_k_MAXn                                                                   2
#define HWIO_EE_n_CNTXT_SRC_IEOB_IRQ_MSK_k_MAXk                                                                   0
#define HWIO_EE_n_CNTXT_SRC_IEOB_IRQ_MSK_k_ATTR                                                                 0x3
#define HWIO_EE_n_CNTXT_SRC_IEOB_IRQ_MSK_k_INI2(n,k)        \
        in_dword_masked(HWIO_EE_n_CNTXT_SRC_IEOB_IRQ_MSK_k_ADDR(n,k), HWIO_EE_n_CNTXT_SRC_IEOB_IRQ_MSK_k_RMSK)
#define HWIO_EE_n_CNTXT_SRC_IEOB_IRQ_MSK_k_INMI2(n,k,mask)    \
        in_dword_masked(HWIO_EE_n_CNTXT_SRC_IEOB_IRQ_MSK_k_ADDR(n,k), mask)
#define HWIO_EE_n_CNTXT_SRC_IEOB_IRQ_MSK_k_OUTI2(n,k,val)    \
        out_dword(HWIO_EE_n_CNTXT_SRC_IEOB_IRQ_MSK_k_ADDR(n,k),val)
#define HWIO_EE_n_CNTXT_SRC_IEOB_IRQ_MSK_k_OUTMI2(n,k,mask,val) \
        out_dword_masked_ns(HWIO_EE_n_CNTXT_SRC_IEOB_IRQ_MSK_k_ADDR(n,k),mask,val,HWIO_EE_n_CNTXT_SRC_IEOB_IRQ_MSK_k_INI2(n,k))
#define HWIO_EE_n_CNTXT_SRC_IEOB_IRQ_MSK_k_EV_CH_BIT_MAP_MSK_BMSK                                        0xffffffff
#define HWIO_EE_n_CNTXT_SRC_IEOB_IRQ_MSK_k_EV_CH_BIT_MAP_MSK_SHFT                                               0x0

#define HWIO_EE_n_CNTXT_SRC_IEOB_IRQ_CLR_k_ADDR(n,k)                                                     (GSI_REG_BASE      + 0x000250b0 + 0x24 * (k) + 0x12000 * (n))
#define HWIO_EE_n_CNTXT_SRC_IEOB_IRQ_CLR_k_PHYS(n,k)                                                     (GSI_REG_BASE_PHYS + 0x000250b0 + 0x24 * (k) + 0x12000 * (n))
#define HWIO_EE_n_CNTXT_SRC_IEOB_IRQ_CLR_k_OFFS(n,k)                                                     (GSI_REG_BASE_OFFS + 0x000250b0 + 0x24 * (k) + 0x12000 * (n))
#define HWIO_EE_n_CNTXT_SRC_IEOB_IRQ_CLR_k_RMSK                                                          0xffffffff
#define HWIO_EE_n_CNTXT_SRC_IEOB_IRQ_CLR_k_MAXn                                                                   2
#define HWIO_EE_n_CNTXT_SRC_IEOB_IRQ_CLR_k_MAXk                                                                   0
#define HWIO_EE_n_CNTXT_SRC_IEOB_IRQ_CLR_k_ATTR                                                                 0x2
#define HWIO_EE_n_CNTXT_SRC_IEOB_IRQ_CLR_k_OUTI2(n,k,val)    \
        out_dword(HWIO_EE_n_CNTXT_SRC_IEOB_IRQ_CLR_k_ADDR(n,k),val)
#define HWIO_EE_n_CNTXT_SRC_IEOB_IRQ_CLR_k_EV_CH_BIT_MAP_BMSK                                            0xffffffff
#define HWIO_EE_n_CNTXT_SRC_IEOB_IRQ_CLR_k_EV_CH_BIT_MAP_SHFT                                                   0x0

#define HWIO_EE_n_CNTXT_GLOB_IRQ_STTS_ADDR(n)                                                            (GSI_REG_BASE      + 0x00025200 + 0x12000 * (n))
#define HWIO_EE_n_CNTXT_GLOB_IRQ_STTS_PHYS(n)                                                            (GSI_REG_BASE_PHYS + 0x00025200 + 0x12000 * (n))
#define HWIO_EE_n_CNTXT_GLOB_IRQ_STTS_OFFS(n)                                                            (GSI_REG_BASE_OFFS + 0x00025200 + 0x12000 * (n))
#define HWIO_EE_n_CNTXT_GLOB_IRQ_STTS_RMSK                                                                      0xf
#define HWIO_EE_n_CNTXT_GLOB_IRQ_STTS_MAXn                                                                        2
#define HWIO_EE_n_CNTXT_GLOB_IRQ_STTS_ATTR                                                                      0x1
#define HWIO_EE_n_CNTXT_GLOB_IRQ_STTS_INI(n)        \
        in_dword_masked(HWIO_EE_n_CNTXT_GLOB_IRQ_STTS_ADDR(n), HWIO_EE_n_CNTXT_GLOB_IRQ_STTS_RMSK)
#define HWIO_EE_n_CNTXT_GLOB_IRQ_STTS_INMI(n,mask)    \
        in_dword_masked(HWIO_EE_n_CNTXT_GLOB_IRQ_STTS_ADDR(n), mask)
#define HWIO_EE_n_CNTXT_GLOB_IRQ_STTS_GP_INT3_BMSK                                                              0x8
#define HWIO_EE_n_CNTXT_GLOB_IRQ_STTS_GP_INT3_SHFT                                                              0x3
#define HWIO_EE_n_CNTXT_GLOB_IRQ_STTS_GP_INT2_BMSK                                                              0x4
#define HWIO_EE_n_CNTXT_GLOB_IRQ_STTS_GP_INT2_SHFT                                                              0x2
#define HWIO_EE_n_CNTXT_GLOB_IRQ_STTS_GP_INT1_BMSK                                                              0x2
#define HWIO_EE_n_CNTXT_GLOB_IRQ_STTS_GP_INT1_SHFT                                                              0x1
#define HWIO_EE_n_CNTXT_GLOB_IRQ_STTS_ERROR_INT_BMSK                                                            0x1
#define HWIO_EE_n_CNTXT_GLOB_IRQ_STTS_ERROR_INT_SHFT                                                            0x0

#define HWIO_EE_n_CNTXT_GLOB_IRQ_EN_ADDR(n)                                                              (GSI_REG_BASE      + 0x00025204 + 0x12000 * (n))
#define HWIO_EE_n_CNTXT_GLOB_IRQ_EN_PHYS(n)                                                              (GSI_REG_BASE_PHYS + 0x00025204 + 0x12000 * (n))
#define HWIO_EE_n_CNTXT_GLOB_IRQ_EN_OFFS(n)                                                              (GSI_REG_BASE_OFFS + 0x00025204 + 0x12000 * (n))
#define HWIO_EE_n_CNTXT_GLOB_IRQ_EN_RMSK                                                                        0xf
#define HWIO_EE_n_CNTXT_GLOB_IRQ_EN_MAXn                                                                          2
#define HWIO_EE_n_CNTXT_GLOB_IRQ_EN_ATTR                                                                        0x3
#define HWIO_EE_n_CNTXT_GLOB_IRQ_EN_INI(n)        \
        in_dword_masked(HWIO_EE_n_CNTXT_GLOB_IRQ_EN_ADDR(n), HWIO_EE_n_CNTXT_GLOB_IRQ_EN_RMSK)
#define HWIO_EE_n_CNTXT_GLOB_IRQ_EN_INMI(n,mask)    \
        in_dword_masked(HWIO_EE_n_CNTXT_GLOB_IRQ_EN_ADDR(n), mask)
#define HWIO_EE_n_CNTXT_GLOB_IRQ_EN_OUTI(n,val)    \
        out_dword(HWIO_EE_n_CNTXT_GLOB_IRQ_EN_ADDR(n),val)
#define HWIO_EE_n_CNTXT_GLOB_IRQ_EN_OUTMI(n,mask,val) \
        out_dword_masked_ns(HWIO_EE_n_CNTXT_GLOB_IRQ_EN_ADDR(n),mask,val,HWIO_EE_n_CNTXT_GLOB_IRQ_EN_INI(n))
#define HWIO_EE_n_CNTXT_GLOB_IRQ_EN_GP_INT3_BMSK                                                                0x8
#define HWIO_EE_n_CNTXT_GLOB_IRQ_EN_GP_INT3_SHFT                                                                0x3
#define HWIO_EE_n_CNTXT_GLOB_IRQ_EN_GP_INT2_BMSK                                                                0x4
#define HWIO_EE_n_CNTXT_GLOB_IRQ_EN_GP_INT2_SHFT                                                                0x2
#define HWIO_EE_n_CNTXT_GLOB_IRQ_EN_GP_INT1_BMSK                                                                0x2
#define HWIO_EE_n_CNTXT_GLOB_IRQ_EN_GP_INT1_SHFT                                                                0x1
#define HWIO_EE_n_CNTXT_GLOB_IRQ_EN_ERROR_INT_BMSK                                                              0x1
#define HWIO_EE_n_CNTXT_GLOB_IRQ_EN_ERROR_INT_SHFT                                                              0x0

#define HWIO_EE_n_CNTXT_GLOB_IRQ_CLR_ADDR(n)                                                             (GSI_REG_BASE      + 0x00025208 + 0x12000 * (n))
#define HWIO_EE_n_CNTXT_GLOB_IRQ_CLR_PHYS(n)                                                             (GSI_REG_BASE_PHYS + 0x00025208 + 0x12000 * (n))
#define HWIO_EE_n_CNTXT_GLOB_IRQ_CLR_OFFS(n)                                                             (GSI_REG_BASE_OFFS + 0x00025208 + 0x12000 * (n))
#define HWIO_EE_n_CNTXT_GLOB_IRQ_CLR_RMSK                                                                       0xf
#define HWIO_EE_n_CNTXT_GLOB_IRQ_CLR_MAXn                                                                         2
#define HWIO_EE_n_CNTXT_GLOB_IRQ_CLR_ATTR                                                                       0x2
#define HWIO_EE_n_CNTXT_GLOB_IRQ_CLR_OUTI(n,val)    \
        out_dword(HWIO_EE_n_CNTXT_GLOB_IRQ_CLR_ADDR(n),val)
#define HWIO_EE_n_CNTXT_GLOB_IRQ_CLR_GP_INT3_BMSK                                                               0x8
#define HWIO_EE_n_CNTXT_GLOB_IRQ_CLR_GP_INT3_SHFT                                                               0x3
#define HWIO_EE_n_CNTXT_GLOB_IRQ_CLR_GP_INT2_BMSK                                                               0x4
#define HWIO_EE_n_CNTXT_GLOB_IRQ_CLR_GP_INT2_SHFT                                                               0x2
#define HWIO_EE_n_CNTXT_GLOB_IRQ_CLR_GP_INT1_BMSK                                                               0x2
#define HWIO_EE_n_CNTXT_GLOB_IRQ_CLR_GP_INT1_SHFT                                                               0x1
#define HWIO_EE_n_CNTXT_GLOB_IRQ_CLR_ERROR_INT_BMSK                                                             0x1
#define HWIO_EE_n_CNTXT_GLOB_IRQ_CLR_ERROR_INT_SHFT                                                             0x0

#define HWIO_EE_n_CNTXT_GSI_IRQ_STTS_ADDR(n)                                                             (GSI_REG_BASE      + 0x0002520c + 0x12000 * (n))
#define HWIO_EE_n_CNTXT_GSI_IRQ_STTS_PHYS(n)                                                             (GSI_REG_BASE_PHYS + 0x0002520c + 0x12000 * (n))
#define HWIO_EE_n_CNTXT_GSI_IRQ_STTS_OFFS(n)                                                             (GSI_REG_BASE_OFFS + 0x0002520c + 0x12000 * (n))
#define HWIO_EE_n_CNTXT_GSI_IRQ_STTS_RMSK                                                                       0xf
#define HWIO_EE_n_CNTXT_GSI_IRQ_STTS_MAXn                                                                         2
#define HWIO_EE_n_CNTXT_GSI_IRQ_STTS_ATTR                                                                       0x1
#define HWIO_EE_n_CNTXT_GSI_IRQ_STTS_INI(n)        \
        in_dword_masked(HWIO_EE_n_CNTXT_GSI_IRQ_STTS_ADDR(n), HWIO_EE_n_CNTXT_GSI_IRQ_STTS_RMSK)
#define HWIO_EE_n_CNTXT_GSI_IRQ_STTS_INMI(n,mask)    \
        in_dword_masked(HWIO_EE_n_CNTXT_GSI_IRQ_STTS_ADDR(n), mask)
#define HWIO_EE_n_CNTXT_GSI_IRQ_STTS_GSI_MCS_STACK_OVRFLOW_BMSK                                                 0x8
#define HWIO_EE_n_CNTXT_GSI_IRQ_STTS_GSI_MCS_STACK_OVRFLOW_SHFT                                                 0x3
#define HWIO_EE_n_CNTXT_GSI_IRQ_STTS_GSI_CMD_FIFO_OVRFLOW_BMSK                                                  0x4
#define HWIO_EE_n_CNTXT_GSI_IRQ_STTS_GSI_CMD_FIFO_OVRFLOW_SHFT                                                  0x2
#define HWIO_EE_n_CNTXT_GSI_IRQ_STTS_GSI_BUS_ERROR_BMSK                                                         0x2
#define HWIO_EE_n_CNTXT_GSI_IRQ_STTS_GSI_BUS_ERROR_SHFT                                                         0x1
#define HWIO_EE_n_CNTXT_GSI_IRQ_STTS_GSI_BREAK_POINT_BMSK                                                       0x1
#define HWIO_EE_n_CNTXT_GSI_IRQ_STTS_GSI_BREAK_POINT_SHFT                                                       0x0

#define HWIO_EE_n_CNTXT_GSI_IRQ_EN_ADDR(n)                                                               (GSI_REG_BASE      + 0x00025210 + 0x12000 * (n))
#define HWIO_EE_n_CNTXT_GSI_IRQ_EN_PHYS(n)                                                               (GSI_REG_BASE_PHYS + 0x00025210 + 0x12000 * (n))
#define HWIO_EE_n_CNTXT_GSI_IRQ_EN_OFFS(n)                                                               (GSI_REG_BASE_OFFS + 0x00025210 + 0x12000 * (n))
#define HWIO_EE_n_CNTXT_GSI_IRQ_EN_RMSK                                                                         0xf
#define HWIO_EE_n_CNTXT_GSI_IRQ_EN_MAXn                                                                           2
#define HWIO_EE_n_CNTXT_GSI_IRQ_EN_ATTR                                                                         0x3
#define HWIO_EE_n_CNTXT_GSI_IRQ_EN_INI(n)        \
        in_dword_masked(HWIO_EE_n_CNTXT_GSI_IRQ_EN_ADDR(n), HWIO_EE_n_CNTXT_GSI_IRQ_EN_RMSK)
#define HWIO_EE_n_CNTXT_GSI_IRQ_EN_INMI(n,mask)    \
        in_dword_masked(HWIO_EE_n_CNTXT_GSI_IRQ_EN_ADDR(n), mask)
#define HWIO_EE_n_CNTXT_GSI_IRQ_EN_OUTI(n,val)    \
        out_dword(HWIO_EE_n_CNTXT_GSI_IRQ_EN_ADDR(n),val)
#define HWIO_EE_n_CNTXT_GSI_IRQ_EN_OUTMI(n,mask,val) \
        out_dword_masked_ns(HWIO_EE_n_CNTXT_GSI_IRQ_EN_ADDR(n),mask,val,HWIO_EE_n_CNTXT_GSI_IRQ_EN_INI(n))
#define HWIO_EE_n_CNTXT_GSI_IRQ_EN_GSI_MCS_STACK_OVRFLOW_BMSK                                                   0x8
#define HWIO_EE_n_CNTXT_GSI_IRQ_EN_GSI_MCS_STACK_OVRFLOW_SHFT                                                   0x3
#define HWIO_EE_n_CNTXT_GSI_IRQ_EN_GSI_CMD_FIFO_OVRFLOW_BMSK                                                    0x4
#define HWIO_EE_n_CNTXT_GSI_IRQ_EN_GSI_CMD_FIFO_OVRFLOW_SHFT                                                    0x2
#define HWIO_EE_n_CNTXT_GSI_IRQ_EN_GSI_BUS_ERROR_BMSK                                                           0x2
#define HWIO_EE_n_CNTXT_GSI_IRQ_EN_GSI_BUS_ERROR_SHFT                                                           0x1
#define HWIO_EE_n_CNTXT_GSI_IRQ_EN_GSI_BREAK_POINT_BMSK                                                         0x1
#define HWIO_EE_n_CNTXT_GSI_IRQ_EN_GSI_BREAK_POINT_SHFT                                                         0x0

#define HWIO_EE_n_CNTXT_GSI_IRQ_CLR_ADDR(n)                                                              (GSI_REG_BASE      + 0x00025214 + 0x12000 * (n))
#define HWIO_EE_n_CNTXT_GSI_IRQ_CLR_PHYS(n)                                                              (GSI_REG_BASE_PHYS + 0x00025214 + 0x12000 * (n))
#define HWIO_EE_n_CNTXT_GSI_IRQ_CLR_OFFS(n)                                                              (GSI_REG_BASE_OFFS + 0x00025214 + 0x12000 * (n))
#define HWIO_EE_n_CNTXT_GSI_IRQ_CLR_RMSK                                                                        0xf
#define HWIO_EE_n_CNTXT_GSI_IRQ_CLR_MAXn                                                                          2
#define HWIO_EE_n_CNTXT_GSI_IRQ_CLR_ATTR                                                                        0x2
#define HWIO_EE_n_CNTXT_GSI_IRQ_CLR_OUTI(n,val)    \
        out_dword(HWIO_EE_n_CNTXT_GSI_IRQ_CLR_ADDR(n),val)
#define HWIO_EE_n_CNTXT_GSI_IRQ_CLR_GSI_MCS_STACK_OVRFLOW_BMSK                                                  0x8
#define HWIO_EE_n_CNTXT_GSI_IRQ_CLR_GSI_MCS_STACK_OVRFLOW_SHFT                                                  0x3
#define HWIO_EE_n_CNTXT_GSI_IRQ_CLR_GSI_CMD_FIFO_OVRFLOW_BMSK                                                   0x4
#define HWIO_EE_n_CNTXT_GSI_IRQ_CLR_GSI_CMD_FIFO_OVRFLOW_SHFT                                                   0x2
#define HWIO_EE_n_CNTXT_GSI_IRQ_CLR_GSI_BUS_ERROR_BMSK                                                          0x2
#define HWIO_EE_n_CNTXT_GSI_IRQ_CLR_GSI_BUS_ERROR_SHFT                                                          0x1
#define HWIO_EE_n_CNTXT_GSI_IRQ_CLR_GSI_BREAK_POINT_BMSK                                                        0x1
#define HWIO_EE_n_CNTXT_GSI_IRQ_CLR_GSI_BREAK_POINT_SHFT                                                        0x0

#define HWIO_EE_n_CNTXT_INTSET_ADDR(n)                                                                   (GSI_REG_BASE      + 0x00025220 + 0x12000 * (n))
#define HWIO_EE_n_CNTXT_INTSET_PHYS(n)                                                                   (GSI_REG_BASE_PHYS + 0x00025220 + 0x12000 * (n))
#define HWIO_EE_n_CNTXT_INTSET_OFFS(n)                                                                   (GSI_REG_BASE_OFFS + 0x00025220 + 0x12000 * (n))
#define HWIO_EE_n_CNTXT_INTSET_RMSK                                                                             0x1
#define HWIO_EE_n_CNTXT_INTSET_MAXn                                                                               2
#define HWIO_EE_n_CNTXT_INTSET_ATTR                                                                             0x3
#define HWIO_EE_n_CNTXT_INTSET_INI(n)        \
        in_dword_masked(HWIO_EE_n_CNTXT_INTSET_ADDR(n), HWIO_EE_n_CNTXT_INTSET_RMSK)
#define HWIO_EE_n_CNTXT_INTSET_INMI(n,mask)    \
        in_dword_masked(HWIO_EE_n_CNTXT_INTSET_ADDR(n), mask)
#define HWIO_EE_n_CNTXT_INTSET_OUTI(n,val)    \
        out_dword(HWIO_EE_n_CNTXT_INTSET_ADDR(n),val)
#define HWIO_EE_n_CNTXT_INTSET_OUTMI(n,mask,val) \
        out_dword_masked_ns(HWIO_EE_n_CNTXT_INTSET_ADDR(n),mask,val,HWIO_EE_n_CNTXT_INTSET_INI(n))
#define HWIO_EE_n_CNTXT_INTSET_INTYPE_BMSK                                                                      0x1
#define HWIO_EE_n_CNTXT_INTSET_INTYPE_SHFT                                                                      0x0
#define HWIO_EE_n_CNTXT_INTSET_INTYPE_MSI_FVAL                                                                  0x0
#define HWIO_EE_n_CNTXT_INTSET_INTYPE_IRQ_FVAL                                                                  0x1

#define HWIO_EE_n_CNTXT_MSI_BASE_LSB_ADDR(n)                                                             (GSI_REG_BASE      + 0x00025230 + 0x12000 * (n))
#define HWIO_EE_n_CNTXT_MSI_BASE_LSB_PHYS(n)                                                             (GSI_REG_BASE_PHYS + 0x00025230 + 0x12000 * (n))
#define HWIO_EE_n_CNTXT_MSI_BASE_LSB_OFFS(n)                                                             (GSI_REG_BASE_OFFS + 0x00025230 + 0x12000 * (n))
#define HWIO_EE_n_CNTXT_MSI_BASE_LSB_RMSK                                                                0xffffffff
#define HWIO_EE_n_CNTXT_MSI_BASE_LSB_MAXn                                                                         2
#define HWIO_EE_n_CNTXT_MSI_BASE_LSB_ATTR                                                                       0x3
#define HWIO_EE_n_CNTXT_MSI_BASE_LSB_INI(n)        \
        in_dword_masked(HWIO_EE_n_CNTXT_MSI_BASE_LSB_ADDR(n), HWIO_EE_n_CNTXT_MSI_BASE_LSB_RMSK)
#define HWIO_EE_n_CNTXT_MSI_BASE_LSB_INMI(n,mask)    \
        in_dword_masked(HWIO_EE_n_CNTXT_MSI_BASE_LSB_ADDR(n), mask)
#define HWIO_EE_n_CNTXT_MSI_BASE_LSB_OUTI(n,val)    \
        out_dword(HWIO_EE_n_CNTXT_MSI_BASE_LSB_ADDR(n),val)
#define HWIO_EE_n_CNTXT_MSI_BASE_LSB_OUTMI(n,mask,val) \
        out_dword_masked_ns(HWIO_EE_n_CNTXT_MSI_BASE_LSB_ADDR(n),mask,val,HWIO_EE_n_CNTXT_MSI_BASE_LSB_INI(n))
#define HWIO_EE_n_CNTXT_MSI_BASE_LSB_MSI_ADDR_LSB_BMSK                                                   0xffffffff
#define HWIO_EE_n_CNTXT_MSI_BASE_LSB_MSI_ADDR_LSB_SHFT                                                          0x0

#define HWIO_EE_n_CNTXT_MSI_BASE_MSB_ADDR(n)                                                             (GSI_REG_BASE      + 0x00025234 + 0x12000 * (n))
#define HWIO_EE_n_CNTXT_MSI_BASE_MSB_PHYS(n)                                                             (GSI_REG_BASE_PHYS + 0x00025234 + 0x12000 * (n))
#define HWIO_EE_n_CNTXT_MSI_BASE_MSB_OFFS(n)                                                             (GSI_REG_BASE_OFFS + 0x00025234 + 0x12000 * (n))
#define HWIO_EE_n_CNTXT_MSI_BASE_MSB_RMSK                                                                0xffffffff
#define HWIO_EE_n_CNTXT_MSI_BASE_MSB_MAXn                                                                         2
#define HWIO_EE_n_CNTXT_MSI_BASE_MSB_ATTR                                                                       0x3
#define HWIO_EE_n_CNTXT_MSI_BASE_MSB_INI(n)        \
        in_dword_masked(HWIO_EE_n_CNTXT_MSI_BASE_MSB_ADDR(n), HWIO_EE_n_CNTXT_MSI_BASE_MSB_RMSK)
#define HWIO_EE_n_CNTXT_MSI_BASE_MSB_INMI(n,mask)    \
        in_dword_masked(HWIO_EE_n_CNTXT_MSI_BASE_MSB_ADDR(n), mask)
#define HWIO_EE_n_CNTXT_MSI_BASE_MSB_OUTI(n,val)    \
        out_dword(HWIO_EE_n_CNTXT_MSI_BASE_MSB_ADDR(n),val)
#define HWIO_EE_n_CNTXT_MSI_BASE_MSB_OUTMI(n,mask,val) \
        out_dword_masked_ns(HWIO_EE_n_CNTXT_MSI_BASE_MSB_ADDR(n),mask,val,HWIO_EE_n_CNTXT_MSI_BASE_MSB_INI(n))
#define HWIO_EE_n_CNTXT_MSI_BASE_MSB_MSI_ADDR_MSB_BMSK                                                   0xffffffff
#define HWIO_EE_n_CNTXT_MSI_BASE_MSB_MSI_ADDR_MSB_SHFT                                                          0x0

#define HWIO_EE_n_CNTXT_INT_VEC_ADDR(n)                                                                  (GSI_REG_BASE      + 0x00025238 + 0x12000 * (n))
#define HWIO_EE_n_CNTXT_INT_VEC_PHYS(n)                                                                  (GSI_REG_BASE_PHYS + 0x00025238 + 0x12000 * (n))
#define HWIO_EE_n_CNTXT_INT_VEC_OFFS(n)                                                                  (GSI_REG_BASE_OFFS + 0x00025238 + 0x12000 * (n))
#define HWIO_EE_n_CNTXT_INT_VEC_RMSK                                                                     0xffffffff
#define HWIO_EE_n_CNTXT_INT_VEC_MAXn                                                                              2
#define HWIO_EE_n_CNTXT_INT_VEC_ATTR                                                                            0x3
#define HWIO_EE_n_CNTXT_INT_VEC_INI(n)        \
        in_dword_masked(HWIO_EE_n_CNTXT_INT_VEC_ADDR(n), HWIO_EE_n_CNTXT_INT_VEC_RMSK)
#define HWIO_EE_n_CNTXT_INT_VEC_INMI(n,mask)    \
        in_dword_masked(HWIO_EE_n_CNTXT_INT_VEC_ADDR(n), mask)
#define HWIO_EE_n_CNTXT_INT_VEC_OUTI(n,val)    \
        out_dword(HWIO_EE_n_CNTXT_INT_VEC_ADDR(n),val)
#define HWIO_EE_n_CNTXT_INT_VEC_OUTMI(n,mask,val) \
        out_dword_masked_ns(HWIO_EE_n_CNTXT_INT_VEC_ADDR(n),mask,val,HWIO_EE_n_CNTXT_INT_VEC_INI(n))
#define HWIO_EE_n_CNTXT_INT_VEC_INT_VEC_BMSK                                                             0xffffffff
#define HWIO_EE_n_CNTXT_INT_VEC_INT_VEC_SHFT                                                                    0x0

#define HWIO_EE_n_ERROR_LOG_ADDR(n)                                                                      (GSI_REG_BASE      + 0x00025240 + 0x12000 * (n))
#define HWIO_EE_n_ERROR_LOG_PHYS(n)                                                                      (GSI_REG_BASE_PHYS + 0x00025240 + 0x12000 * (n))
#define HWIO_EE_n_ERROR_LOG_OFFS(n)                                                                      (GSI_REG_BASE_OFFS + 0x00025240 + 0x12000 * (n))
#define HWIO_EE_n_ERROR_LOG_RMSK                                                                         0xffffffff
#define HWIO_EE_n_ERROR_LOG_MAXn                                                                                  2
#define HWIO_EE_n_ERROR_LOG_ATTR                                                                                0x3
#define HWIO_EE_n_ERROR_LOG_INI(n)        \
        in_dword_masked(HWIO_EE_n_ERROR_LOG_ADDR(n), HWIO_EE_n_ERROR_LOG_RMSK)
#define HWIO_EE_n_ERROR_LOG_INMI(n,mask)    \
        in_dword_masked(HWIO_EE_n_ERROR_LOG_ADDR(n), mask)
#define HWIO_EE_n_ERROR_LOG_OUTI(n,val)    \
        out_dword(HWIO_EE_n_ERROR_LOG_ADDR(n),val)
#define HWIO_EE_n_ERROR_LOG_OUTMI(n,mask,val) \
        out_dword_masked_ns(HWIO_EE_n_ERROR_LOG_ADDR(n),mask,val,HWIO_EE_n_ERROR_LOG_INI(n))
#define HWIO_EE_n_ERROR_LOG_ERROR_LOG_BMSK                                                               0xffffffff
#define HWIO_EE_n_ERROR_LOG_ERROR_LOG_SHFT                                                                      0x0

#define HWIO_EE_n_ERROR_LOG_CLR_ADDR(n)                                                                  (GSI_REG_BASE      + 0x00025244 + 0x12000 * (n))
#define HWIO_EE_n_ERROR_LOG_CLR_PHYS(n)                                                                  (GSI_REG_BASE_PHYS + 0x00025244 + 0x12000 * (n))
#define HWIO_EE_n_ERROR_LOG_CLR_OFFS(n)                                                                  (GSI_REG_BASE_OFFS + 0x00025244 + 0x12000 * (n))
#define HWIO_EE_n_ERROR_LOG_CLR_RMSK                                                                     0xffffffff
#define HWIO_EE_n_ERROR_LOG_CLR_MAXn                                                                              2
#define HWIO_EE_n_ERROR_LOG_CLR_ATTR                                                                            0x2
#define HWIO_EE_n_ERROR_LOG_CLR_OUTI(n,val)    \
        out_dword(HWIO_EE_n_ERROR_LOG_CLR_ADDR(n),val)
#define HWIO_EE_n_ERROR_LOG_CLR_ERROR_LOG_CLR_BMSK                                                       0xffffffff
#define HWIO_EE_n_ERROR_LOG_CLR_ERROR_LOG_CLR_SHFT                                                              0x0

#define HWIO_EE_n_CNTXT_SCRATCH_0_ADDR(n)                                                                (GSI_REG_BASE      + 0x00025400 + 0x12000 * (n))
#define HWIO_EE_n_CNTXT_SCRATCH_0_PHYS(n)                                                                (GSI_REG_BASE_PHYS + 0x00025400 + 0x12000 * (n))
#define HWIO_EE_n_CNTXT_SCRATCH_0_OFFS(n)                                                                (GSI_REG_BASE_OFFS + 0x00025400 + 0x12000 * (n))
#define HWIO_EE_n_CNTXT_SCRATCH_0_RMSK                                                                   0xffffffff
#define HWIO_EE_n_CNTXT_SCRATCH_0_MAXn                                                                            2
#define HWIO_EE_n_CNTXT_SCRATCH_0_ATTR                                                                          0x3
#define HWIO_EE_n_CNTXT_SCRATCH_0_INI(n)        \
        in_dword_masked(HWIO_EE_n_CNTXT_SCRATCH_0_ADDR(n), HWIO_EE_n_CNTXT_SCRATCH_0_RMSK)
#define HWIO_EE_n_CNTXT_SCRATCH_0_INMI(n,mask)    \
        in_dword_masked(HWIO_EE_n_CNTXT_SCRATCH_0_ADDR(n), mask)
#define HWIO_EE_n_CNTXT_SCRATCH_0_OUTI(n,val)    \
        out_dword(HWIO_EE_n_CNTXT_SCRATCH_0_ADDR(n),val)
#define HWIO_EE_n_CNTXT_SCRATCH_0_OUTMI(n,mask,val) \
        out_dword_masked_ns(HWIO_EE_n_CNTXT_SCRATCH_0_ADDR(n),mask,val,HWIO_EE_n_CNTXT_SCRATCH_0_INI(n))
#define HWIO_EE_n_CNTXT_SCRATCH_0_SCRATCH_BMSK                                                           0xffffffff
#define HWIO_EE_n_CNTXT_SCRATCH_0_SCRATCH_SHFT                                                                  0x0

#define HWIO_EE_n_CNTXT_SCRATCH_1_ADDR(n)                                                                (GSI_REG_BASE      + 0x00025404 + 0x12000 * (n))
#define HWIO_EE_n_CNTXT_SCRATCH_1_PHYS(n)                                                                (GSI_REG_BASE_PHYS + 0x00025404 + 0x12000 * (n))
#define HWIO_EE_n_CNTXT_SCRATCH_1_OFFS(n)                                                                (GSI_REG_BASE_OFFS + 0x00025404 + 0x12000 * (n))
#define HWIO_EE_n_CNTXT_SCRATCH_1_RMSK                                                                   0xffffffff
#define HWIO_EE_n_CNTXT_SCRATCH_1_MAXn                                                                            2
#define HWIO_EE_n_CNTXT_SCRATCH_1_ATTR                                                                          0x3
#define HWIO_EE_n_CNTXT_SCRATCH_1_INI(n)        \
        in_dword_masked(HWIO_EE_n_CNTXT_SCRATCH_1_ADDR(n), HWIO_EE_n_CNTXT_SCRATCH_1_RMSK)
#define HWIO_EE_n_CNTXT_SCRATCH_1_INMI(n,mask)    \
        in_dword_masked(HWIO_EE_n_CNTXT_SCRATCH_1_ADDR(n), mask)
#define HWIO_EE_n_CNTXT_SCRATCH_1_OUTI(n,val)    \
        out_dword(HWIO_EE_n_CNTXT_SCRATCH_1_ADDR(n),val)
#define HWIO_EE_n_CNTXT_SCRATCH_1_OUTMI(n,mask,val) \
        out_dword_masked_ns(HWIO_EE_n_CNTXT_SCRATCH_1_ADDR(n),mask,val,HWIO_EE_n_CNTXT_SCRATCH_1_INI(n))
#define HWIO_EE_n_CNTXT_SCRATCH_1_SCRATCH_BMSK                                                           0xffffffff
#define HWIO_EE_n_CNTXT_SCRATCH_1_SCRATCH_SHFT                                                                  0x0

#define HWIO_GSI_MCS_CFG_ADDR                                                                            (GSI_REG_BASE      + 0x0000b000)
#define HWIO_GSI_MCS_CFG_PHYS                                                                            (GSI_REG_BASE_PHYS + 0x0000b000)
#define HWIO_GSI_MCS_CFG_OFFS                                                                            (GSI_REG_BASE_OFFS + 0x0000b000)
#define HWIO_GSI_MCS_CFG_RMSK                                                                                   0x1
#define HWIO_GSI_MCS_CFG_ATTR                                                                                   0x3
#define HWIO_GSI_MCS_CFG_IN          \
        in_dword_masked(HWIO_GSI_MCS_CFG_ADDR, HWIO_GSI_MCS_CFG_RMSK)
#define HWIO_GSI_MCS_CFG_INM(m)      \
        in_dword_masked(HWIO_GSI_MCS_CFG_ADDR, m)
#define HWIO_GSI_MCS_CFG_OUT(v)      \
        out_dword(HWIO_GSI_MCS_CFG_ADDR,v)
#define HWIO_GSI_MCS_CFG_OUTM(m,v) \
        out_dword_masked_ns(HWIO_GSI_MCS_CFG_ADDR,m,v,HWIO_GSI_MCS_CFG_IN)
#define HWIO_GSI_MCS_CFG_MCS_ENABLE_BMSK                                                                        0x1
#define HWIO_GSI_MCS_CFG_MCS_ENABLE_SHFT                                                                        0x0

#define HWIO_GSI_TZ_FW_AUTH_LOCK_ADDR                                                                    (GSI_REG_BASE      + 0x0000b008)
#define HWIO_GSI_TZ_FW_AUTH_LOCK_PHYS                                                                    (GSI_REG_BASE_PHYS + 0x0000b008)
#define HWIO_GSI_TZ_FW_AUTH_LOCK_OFFS                                                                    (GSI_REG_BASE_OFFS + 0x0000b008)
#define HWIO_GSI_TZ_FW_AUTH_LOCK_RMSK                                                                           0x3
#define HWIO_GSI_TZ_FW_AUTH_LOCK_ATTR                                                                           0x3
#define HWIO_GSI_TZ_FW_AUTH_LOCK_IN          \
        in_dword_masked(HWIO_GSI_TZ_FW_AUTH_LOCK_ADDR, HWIO_GSI_TZ_FW_AUTH_LOCK_RMSK)
#define HWIO_GSI_TZ_FW_AUTH_LOCK_INM(m)      \
        in_dword_masked(HWIO_GSI_TZ_FW_AUTH_LOCK_ADDR, m)
#define HWIO_GSI_TZ_FW_AUTH_LOCK_OUT(v)      \
        out_dword(HWIO_GSI_TZ_FW_AUTH_LOCK_ADDR,v)
#define HWIO_GSI_TZ_FW_AUTH_LOCK_OUTM(m,v) \
        out_dword_masked_ns(HWIO_GSI_TZ_FW_AUTH_LOCK_ADDR,m,v,HWIO_GSI_TZ_FW_AUTH_LOCK_IN)
#define HWIO_GSI_TZ_FW_AUTH_LOCK_DIS_DEBUG_SHRAM_WRITE_BMSK                                                     0x2
#define HWIO_GSI_TZ_FW_AUTH_LOCK_DIS_DEBUG_SHRAM_WRITE_SHFT                                                     0x1
#define HWIO_GSI_TZ_FW_AUTH_LOCK_DIS_IRAM_WRITE_BMSK                                                            0x1
#define HWIO_GSI_TZ_FW_AUTH_LOCK_DIS_IRAM_WRITE_SHFT                                                            0x0

#define HWIO_GSI_MSA_FW_AUTH_LOCK_ADDR                                                                   (GSI_REG_BASE      + 0x0000b010)
#define HWIO_GSI_MSA_FW_AUTH_LOCK_PHYS                                                                   (GSI_REG_BASE_PHYS + 0x0000b010)
#define HWIO_GSI_MSA_FW_AUTH_LOCK_OFFS                                                                   (GSI_REG_BASE_OFFS + 0x0000b010)
#define HWIO_GSI_MSA_FW_AUTH_LOCK_RMSK                                                                          0x3
#define HWIO_GSI_MSA_FW_AUTH_LOCK_ATTR                                                                          0x3
#define HWIO_GSI_MSA_FW_AUTH_LOCK_IN          \
        in_dword_masked(HWIO_GSI_MSA_FW_AUTH_LOCK_ADDR, HWIO_GSI_MSA_FW_AUTH_LOCK_RMSK)
#define HWIO_GSI_MSA_FW_AUTH_LOCK_INM(m)      \
        in_dword_masked(HWIO_GSI_MSA_FW_AUTH_LOCK_ADDR, m)
#define HWIO_GSI_MSA_FW_AUTH_LOCK_OUT(v)      \
        out_dword(HWIO_GSI_MSA_FW_AUTH_LOCK_ADDR,v)
#define HWIO_GSI_MSA_FW_AUTH_LOCK_OUTM(m,v) \
        out_dword_masked_ns(HWIO_GSI_MSA_FW_AUTH_LOCK_ADDR,m,v,HWIO_GSI_MSA_FW_AUTH_LOCK_IN)
#define HWIO_GSI_MSA_FW_AUTH_LOCK_DIS_DEBUG_SHRAM_WRITE_BMSK                                                    0x2
#define HWIO_GSI_MSA_FW_AUTH_LOCK_DIS_DEBUG_SHRAM_WRITE_SHFT                                                    0x1
#define HWIO_GSI_MSA_FW_AUTH_LOCK_DIS_IRAM_WRITE_BMSK                                                           0x1
#define HWIO_GSI_MSA_FW_AUTH_LOCK_DIS_IRAM_WRITE_SHFT                                                           0x0

#define HWIO_GSI_SP_FW_AUTH_LOCK_ADDR                                                                    (GSI_REG_BASE      + 0x0000b018)
#define HWIO_GSI_SP_FW_AUTH_LOCK_PHYS                                                                    (GSI_REG_BASE_PHYS + 0x0000b018)
#define HWIO_GSI_SP_FW_AUTH_LOCK_OFFS                                                                    (GSI_REG_BASE_OFFS + 0x0000b018)
#define HWIO_GSI_SP_FW_AUTH_LOCK_RMSK                                                                           0x3
#define HWIO_GSI_SP_FW_AUTH_LOCK_ATTR                                                                           0x3
#define HWIO_GSI_SP_FW_AUTH_LOCK_IN          \
        in_dword_masked(HWIO_GSI_SP_FW_AUTH_LOCK_ADDR, HWIO_GSI_SP_FW_AUTH_LOCK_RMSK)
#define HWIO_GSI_SP_FW_AUTH_LOCK_INM(m)      \
        in_dword_masked(HWIO_GSI_SP_FW_AUTH_LOCK_ADDR, m)
#define HWIO_GSI_SP_FW_AUTH_LOCK_OUT(v)      \
        out_dword(HWIO_GSI_SP_FW_AUTH_LOCK_ADDR,v)
#define HWIO_GSI_SP_FW_AUTH_LOCK_OUTM(m,v) \
        out_dword_masked_ns(HWIO_GSI_SP_FW_AUTH_LOCK_ADDR,m,v,HWIO_GSI_SP_FW_AUTH_LOCK_IN)
#define HWIO_GSI_SP_FW_AUTH_LOCK_DIS_DEBUG_SHRAM_WRITE_BMSK                                                     0x2
#define HWIO_GSI_SP_FW_AUTH_LOCK_DIS_DEBUG_SHRAM_WRITE_SHFT                                                     0x1
#define HWIO_GSI_SP_FW_AUTH_LOCK_DIS_IRAM_WRITE_BMSK                                                            0x1
#define HWIO_GSI_SP_FW_AUTH_LOCK_DIS_IRAM_WRITE_SHFT                                                            0x0

#define HWIO_INTER_EE_n_ORIGINATOR_EE_ADDR(n)                                                            (GSI_REG_BASE      + 0x0000c000 + 0x1000 * (n))
#define HWIO_INTER_EE_n_ORIGINATOR_EE_PHYS(n)                                                            (GSI_REG_BASE_PHYS + 0x0000c000 + 0x1000 * (n))
#define HWIO_INTER_EE_n_ORIGINATOR_EE_OFFS(n)                                                            (GSI_REG_BASE_OFFS + 0x0000c000 + 0x1000 * (n))
#define HWIO_INTER_EE_n_ORIGINATOR_EE_RMSK                                                                      0xf
#define HWIO_INTER_EE_n_ORIGINATOR_EE_MAXn                                                                        2
#define HWIO_INTER_EE_n_ORIGINATOR_EE_ATTR                                                                      0x3
#define HWIO_INTER_EE_n_ORIGINATOR_EE_INI(n)        \
        in_dword_masked(HWIO_INTER_EE_n_ORIGINATOR_EE_ADDR(n), HWIO_INTER_EE_n_ORIGINATOR_EE_RMSK)
#define HWIO_INTER_EE_n_ORIGINATOR_EE_INMI(n,mask)    \
        in_dword_masked(HWIO_INTER_EE_n_ORIGINATOR_EE_ADDR(n), mask)
#define HWIO_INTER_EE_n_ORIGINATOR_EE_OUTI(n,val)    \
        out_dword(HWIO_INTER_EE_n_ORIGINATOR_EE_ADDR(n),val)
#define HWIO_INTER_EE_n_ORIGINATOR_EE_OUTMI(n,mask,val) \
        out_dword_masked_ns(HWIO_INTER_EE_n_ORIGINATOR_EE_ADDR(n),mask,val,HWIO_INTER_EE_n_ORIGINATOR_EE_INI(n))
#define HWIO_INTER_EE_n_ORIGINATOR_EE_EE_NUMBER_BMSK                                                            0xf
#define HWIO_INTER_EE_n_ORIGINATOR_EE_EE_NUMBER_SHFT                                                            0x0

#define HWIO_INTER_EE_n_GSI_CH_CMD_ADDR(n)                                                               (GSI_REG_BASE      + 0x0000c008 + 0x1000 * (n))
#define HWIO_INTER_EE_n_GSI_CH_CMD_PHYS(n)                                                               (GSI_REG_BASE_PHYS + 0x0000c008 + 0x1000 * (n))
#define HWIO_INTER_EE_n_GSI_CH_CMD_OFFS(n)                                                               (GSI_REG_BASE_OFFS + 0x0000c008 + 0x1000 * (n))
#define HWIO_INTER_EE_n_GSI_CH_CMD_RMSK                                                                  0xff0000ff
#define HWIO_INTER_EE_n_GSI_CH_CMD_MAXn                                                                           2
#define HWIO_INTER_EE_n_GSI_CH_CMD_ATTR                                                                         0x2
#define HWIO_INTER_EE_n_GSI_CH_CMD_OUTI(n,val)    \
        out_dword(HWIO_INTER_EE_n_GSI_CH_CMD_ADDR(n),val)
#define HWIO_INTER_EE_n_GSI_CH_CMD_OPCODE_BMSK                                                           0xff000000
#define HWIO_INTER_EE_n_GSI_CH_CMD_OPCODE_SHFT                                                                 0x18
#define HWIO_INTER_EE_n_GSI_CH_CMD_OPCODE_START_FVAL                                                            0x1
#define HWIO_INTER_EE_n_GSI_CH_CMD_OPCODE_STOP_FVAL                                                             0x2
#define HWIO_INTER_EE_n_GSI_CH_CMD_OPCODE_RESET_FVAL                                                            0x9
#define HWIO_INTER_EE_n_GSI_CH_CMD_OPCODE_DE_ALLOC_FVAL                                                         0xa
#define HWIO_INTER_EE_n_GSI_CH_CMD_OPCODE_DB_STOP_FVAL                                                          0xb
#define HWIO_INTER_EE_n_GSI_CH_CMD_CHID_BMSK                                                                   0xff
#define HWIO_INTER_EE_n_GSI_CH_CMD_CHID_SHFT                                                                    0x0

#define HWIO_INTER_EE_n_EV_CH_CMD_ADDR(n)                                                                (GSI_REG_BASE      + 0x0000c010 + 0x1000 * (n))
#define HWIO_INTER_EE_n_EV_CH_CMD_PHYS(n)                                                                (GSI_REG_BASE_PHYS + 0x0000c010 + 0x1000 * (n))
#define HWIO_INTER_EE_n_EV_CH_CMD_OFFS(n)                                                                (GSI_REG_BASE_OFFS + 0x0000c010 + 0x1000 * (n))
#define HWIO_INTER_EE_n_EV_CH_CMD_RMSK                                                                   0xff0000ff
#define HWIO_INTER_EE_n_EV_CH_CMD_MAXn                                                                            2
#define HWIO_INTER_EE_n_EV_CH_CMD_ATTR                                                                          0x2
#define HWIO_INTER_EE_n_EV_CH_CMD_OUTI(n,val)    \
        out_dword(HWIO_INTER_EE_n_EV_CH_CMD_ADDR(n),val)
#define HWIO_INTER_EE_n_EV_CH_CMD_OPCODE_BMSK                                                            0xff000000
#define HWIO_INTER_EE_n_EV_CH_CMD_OPCODE_SHFT                                                                  0x18
#define HWIO_INTER_EE_n_EV_CH_CMD_OPCODE_RESET_FVAL                                                             0x9
#define HWIO_INTER_EE_n_EV_CH_CMD_OPCODE_DE_ALLOC_FVAL                                                          0xa
#define HWIO_INTER_EE_n_EV_CH_CMD_CHID_BMSK                                                                    0xff
#define HWIO_INTER_EE_n_EV_CH_CMD_CHID_SHFT                                                                     0x0

#define HWIO_INTER_EE_n_SRC_GSI_CH_IRQ_k_ADDR(n,k)                                                       (GSI_REG_BASE      + 0x0000c018 + 0x18 * (k) + 0x1000 * (n))
#define HWIO_INTER_EE_n_SRC_GSI_CH_IRQ_k_PHYS(n,k)                                                       (GSI_REG_BASE_PHYS + 0x0000c018 + 0x18 * (k) + 0x1000 * (n))
#define HWIO_INTER_EE_n_SRC_GSI_CH_IRQ_k_OFFS(n,k)                                                       (GSI_REG_BASE_OFFS + 0x0000c018 + 0x18 * (k) + 0x1000 * (n))
#define HWIO_INTER_EE_n_SRC_GSI_CH_IRQ_k_RMSK                                                            0xffffffff
#define HWIO_INTER_EE_n_SRC_GSI_CH_IRQ_k_MAXn                                                                     2
#define HWIO_INTER_EE_n_SRC_GSI_CH_IRQ_k_MAXk                                                                     0
#define HWIO_INTER_EE_n_SRC_GSI_CH_IRQ_k_ATTR                                                                   0x1
#define HWIO_INTER_EE_n_SRC_GSI_CH_IRQ_k_INI2(n,k)        \
        in_dword_masked(HWIO_INTER_EE_n_SRC_GSI_CH_IRQ_k_ADDR(n,k), HWIO_INTER_EE_n_SRC_GSI_CH_IRQ_k_RMSK)
#define HWIO_INTER_EE_n_SRC_GSI_CH_IRQ_k_INMI2(n,k,mask)    \
        in_dword_masked(HWIO_INTER_EE_n_SRC_GSI_CH_IRQ_k_ADDR(n,k), mask)
#define HWIO_INTER_EE_n_SRC_GSI_CH_IRQ_k_GSI_CH_BIT_MAP_BMSK                                             0xffffffff
#define HWIO_INTER_EE_n_SRC_GSI_CH_IRQ_k_GSI_CH_BIT_MAP_SHFT                                                    0x0

#define HWIO_INTER_EE_n_SRC_GSI_CH_IRQ_MSK_k_ADDR(n,k)                                                   (GSI_REG_BASE      + 0x0000c01c + 0x18 * (k) + 0x1000 * (n))
#define HWIO_INTER_EE_n_SRC_GSI_CH_IRQ_MSK_k_PHYS(n,k)                                                   (GSI_REG_BASE_PHYS + 0x0000c01c + 0x18 * (k) + 0x1000 * (n))
#define HWIO_INTER_EE_n_SRC_GSI_CH_IRQ_MSK_k_OFFS(n,k)                                                   (GSI_REG_BASE_OFFS + 0x0000c01c + 0x18 * (k) + 0x1000 * (n))
#define HWIO_INTER_EE_n_SRC_GSI_CH_IRQ_MSK_k_RMSK                                                        0xffffffff
#define HWIO_INTER_EE_n_SRC_GSI_CH_IRQ_MSK_k_MAXn                                                                 2
#define HWIO_INTER_EE_n_SRC_GSI_CH_IRQ_MSK_k_MAXk                                                                 0
#define HWIO_INTER_EE_n_SRC_GSI_CH_IRQ_MSK_k_ATTR                                                               0x3
#define HWIO_INTER_EE_n_SRC_GSI_CH_IRQ_MSK_k_INI2(n,k)        \
        in_dword_masked(HWIO_INTER_EE_n_SRC_GSI_CH_IRQ_MSK_k_ADDR(n,k), HWIO_INTER_EE_n_SRC_GSI_CH_IRQ_MSK_k_RMSK)
#define HWIO_INTER_EE_n_SRC_GSI_CH_IRQ_MSK_k_INMI2(n,k,mask)    \
        in_dword_masked(HWIO_INTER_EE_n_SRC_GSI_CH_IRQ_MSK_k_ADDR(n,k), mask)
#define HWIO_INTER_EE_n_SRC_GSI_CH_IRQ_MSK_k_OUTI2(n,k,val)    \
        out_dword(HWIO_INTER_EE_n_SRC_GSI_CH_IRQ_MSK_k_ADDR(n,k),val)
#define HWIO_INTER_EE_n_SRC_GSI_CH_IRQ_MSK_k_OUTMI2(n,k,mask,val) \
        out_dword_masked_ns(HWIO_INTER_EE_n_SRC_GSI_CH_IRQ_MSK_k_ADDR(n,k),mask,val,HWIO_INTER_EE_n_SRC_GSI_CH_IRQ_MSK_k_INI2(n,k))
#define HWIO_INTER_EE_n_SRC_GSI_CH_IRQ_MSK_k_GSI_CH_BIT_MAP_MSK_BMSK                                     0xffffffff
#define HWIO_INTER_EE_n_SRC_GSI_CH_IRQ_MSK_k_GSI_CH_BIT_MAP_MSK_SHFT                                            0x0

#define HWIO_INTER_EE_n_SRC_GSI_CH_IRQ_CLR_k_ADDR(n,k)                                                   (GSI_REG_BASE      + 0x0000c020 + 0x18 * (k) + 0x1000 * (n))
#define HWIO_INTER_EE_n_SRC_GSI_CH_IRQ_CLR_k_PHYS(n,k)                                                   (GSI_REG_BASE_PHYS + 0x0000c020 + 0x18 * (k) + 0x1000 * (n))
#define HWIO_INTER_EE_n_SRC_GSI_CH_IRQ_CLR_k_OFFS(n,k)                                                   (GSI_REG_BASE_OFFS + 0x0000c020 + 0x18 * (k) + 0x1000 * (n))
#define HWIO_INTER_EE_n_SRC_GSI_CH_IRQ_CLR_k_RMSK                                                        0xffffffff
#define HWIO_INTER_EE_n_SRC_GSI_CH_IRQ_CLR_k_MAXn                                                                 2
#define HWIO_INTER_EE_n_SRC_GSI_CH_IRQ_CLR_k_MAXk                                                                 0
#define HWIO_INTER_EE_n_SRC_GSI_CH_IRQ_CLR_k_ATTR                                                               0x2
#define HWIO_INTER_EE_n_SRC_GSI_CH_IRQ_CLR_k_OUTI2(n,k,val)    \
        out_dword(HWIO_INTER_EE_n_SRC_GSI_CH_IRQ_CLR_k_ADDR(n,k),val)
#define HWIO_INTER_EE_n_SRC_GSI_CH_IRQ_CLR_k_GSI_CH_BIT_MAP_BMSK                                         0xffffffff
#define HWIO_INTER_EE_n_SRC_GSI_CH_IRQ_CLR_k_GSI_CH_BIT_MAP_SHFT                                                0x0

#define HWIO_INTER_EE_n_SRC_EV_CH_IRQ_k_ADDR(n,k)                                                        (GSI_REG_BASE      + 0x0000c024 + 0x18 * (k) + 0x1000 * (n))
#define HWIO_INTER_EE_n_SRC_EV_CH_IRQ_k_PHYS(n,k)                                                        (GSI_REG_BASE_PHYS + 0x0000c024 + 0x18 * (k) + 0x1000 * (n))
#define HWIO_INTER_EE_n_SRC_EV_CH_IRQ_k_OFFS(n,k)                                                        (GSI_REG_BASE_OFFS + 0x0000c024 + 0x18 * (k) + 0x1000 * (n))
#define HWIO_INTER_EE_n_SRC_EV_CH_IRQ_k_RMSK                                                             0xffffffff
#define HWIO_INTER_EE_n_SRC_EV_CH_IRQ_k_MAXn                                                                      2
#define HWIO_INTER_EE_n_SRC_EV_CH_IRQ_k_MAXk                                                                      0
#define HWIO_INTER_EE_n_SRC_EV_CH_IRQ_k_ATTR                                                                    0x1
#define HWIO_INTER_EE_n_SRC_EV_CH_IRQ_k_INI2(n,k)        \
        in_dword_masked(HWIO_INTER_EE_n_SRC_EV_CH_IRQ_k_ADDR(n,k), HWIO_INTER_EE_n_SRC_EV_CH_IRQ_k_RMSK)
#define HWIO_INTER_EE_n_SRC_EV_CH_IRQ_k_INMI2(n,k,mask)    \
        in_dword_masked(HWIO_INTER_EE_n_SRC_EV_CH_IRQ_k_ADDR(n,k), mask)
#define HWIO_INTER_EE_n_SRC_EV_CH_IRQ_k_EV_CH_BIT_MAP_BMSK                                               0xffffffff
#define HWIO_INTER_EE_n_SRC_EV_CH_IRQ_k_EV_CH_BIT_MAP_SHFT                                                      0x0

#define HWIO_INTER_EE_n_SRC_EV_CH_IRQ_MSK_k_ADDR(n,k)                                                    (GSI_REG_BASE      + 0x0000c028 + 0x18 * (k) + 0x1000 * (n))
#define HWIO_INTER_EE_n_SRC_EV_CH_IRQ_MSK_k_PHYS(n,k)                                                    (GSI_REG_BASE_PHYS + 0x0000c028 + 0x18 * (k) + 0x1000 * (n))
#define HWIO_INTER_EE_n_SRC_EV_CH_IRQ_MSK_k_OFFS(n,k)                                                    (GSI_REG_BASE_OFFS + 0x0000c028 + 0x18 * (k) + 0x1000 * (n))
#define HWIO_INTER_EE_n_SRC_EV_CH_IRQ_MSK_k_RMSK                                                         0xffffffff
#define HWIO_INTER_EE_n_SRC_EV_CH_IRQ_MSK_k_MAXn                                                                  2
#define HWIO_INTER_EE_n_SRC_EV_CH_IRQ_MSK_k_MAXk                                                                  0
#define HWIO_INTER_EE_n_SRC_EV_CH_IRQ_MSK_k_ATTR                                                                0x3
#define HWIO_INTER_EE_n_SRC_EV_CH_IRQ_MSK_k_INI2(n,k)        \
        in_dword_masked(HWIO_INTER_EE_n_SRC_EV_CH_IRQ_MSK_k_ADDR(n,k), HWIO_INTER_EE_n_SRC_EV_CH_IRQ_MSK_k_RMSK)
#define HWIO_INTER_EE_n_SRC_EV_CH_IRQ_MSK_k_INMI2(n,k,mask)    \
        in_dword_masked(HWIO_INTER_EE_n_SRC_EV_CH_IRQ_MSK_k_ADDR(n,k), mask)
#define HWIO_INTER_EE_n_SRC_EV_CH_IRQ_MSK_k_OUTI2(n,k,val)    \
        out_dword(HWIO_INTER_EE_n_SRC_EV_CH_IRQ_MSK_k_ADDR(n,k),val)
#define HWIO_INTER_EE_n_SRC_EV_CH_IRQ_MSK_k_OUTMI2(n,k,mask,val) \
        out_dword_masked_ns(HWIO_INTER_EE_n_SRC_EV_CH_IRQ_MSK_k_ADDR(n,k),mask,val,HWIO_INTER_EE_n_SRC_EV_CH_IRQ_MSK_k_INI2(n,k))
#define HWIO_INTER_EE_n_SRC_EV_CH_IRQ_MSK_k_EV_CH_BIT_MAP_MSK_BMSK                                       0xffffffff
#define HWIO_INTER_EE_n_SRC_EV_CH_IRQ_MSK_k_EV_CH_BIT_MAP_MSK_SHFT                                              0x0

#define HWIO_INTER_EE_n_SRC_EV_CH_IRQ_CLR_k_ADDR(n,k)                                                    (GSI_REG_BASE      + 0x0000c02c + 0x18 * (k) + 0x1000 * (n))
#define HWIO_INTER_EE_n_SRC_EV_CH_IRQ_CLR_k_PHYS(n,k)                                                    (GSI_REG_BASE_PHYS + 0x0000c02c + 0x18 * (k) + 0x1000 * (n))
#define HWIO_INTER_EE_n_SRC_EV_CH_IRQ_CLR_k_OFFS(n,k)                                                    (GSI_REG_BASE_OFFS + 0x0000c02c + 0x18 * (k) + 0x1000 * (n))
#define HWIO_INTER_EE_n_SRC_EV_CH_IRQ_CLR_k_RMSK                                                         0xffffffff
#define HWIO_INTER_EE_n_SRC_EV_CH_IRQ_CLR_k_MAXn                                                                  2
#define HWIO_INTER_EE_n_SRC_EV_CH_IRQ_CLR_k_MAXk                                                                  0
#define HWIO_INTER_EE_n_SRC_EV_CH_IRQ_CLR_k_ATTR                                                                0x2
#define HWIO_INTER_EE_n_SRC_EV_CH_IRQ_CLR_k_OUTI2(n,k,val)    \
        out_dword(HWIO_INTER_EE_n_SRC_EV_CH_IRQ_CLR_k_ADDR(n,k),val)
#define HWIO_INTER_EE_n_SRC_EV_CH_IRQ_CLR_k_EV_CH_BIT_MAP_BMSK                                           0xffffffff
#define HWIO_INTER_EE_n_SRC_EV_CH_IRQ_CLR_k_EV_CH_BIT_MAP_SHFT                                                  0x0

/*----------------------------------------------------------------------------
 * MODULE: IPA_0_GSI_TOP_XPU3
 *--------------------------------------------------------------------------*/

#define IPA_0_GSI_TOP_XPU3_REG_BASE                                          (IPA_0_IPA_WRAPPER_BASE      + 0x00000000)
#define IPA_0_GSI_TOP_XPU3_REG_BASE_PHYS                                     (IPA_0_IPA_WRAPPER_BASE_PHYS + 0x00000000)
#define IPA_0_GSI_TOP_XPU3_REG_BASE_OFFS                                     0x00000000

#define HWIO_IPA_0_GSI_TOP_XPU3_GCR0_ADDR                                    (IPA_0_GSI_TOP_XPU3_REG_BASE      + 0x00000000)
#define HWIO_IPA_0_GSI_TOP_XPU3_GCR0_PHYS                                    (IPA_0_GSI_TOP_XPU3_REG_BASE_PHYS + 0x00000000)
#define HWIO_IPA_0_GSI_TOP_XPU3_GCR0_OFFS                                    (IPA_0_GSI_TOP_XPU3_REG_BASE_OFFS + 0x00000000)
#define HWIO_IPA_0_GSI_TOP_XPU3_GCR0_RMSK                                           0x3
#define HWIO_IPA_0_GSI_TOP_XPU3_GCR0_ATTR                                           0x3
#define HWIO_IPA_0_GSI_TOP_XPU3_GCR0_IN          \
        in_dword_masked(HWIO_IPA_0_GSI_TOP_XPU3_GCR0_ADDR, HWIO_IPA_0_GSI_TOP_XPU3_GCR0_RMSK)
#define HWIO_IPA_0_GSI_TOP_XPU3_GCR0_INM(m)      \
        in_dword_masked(HWIO_IPA_0_GSI_TOP_XPU3_GCR0_ADDR, m)
#define HWIO_IPA_0_GSI_TOP_XPU3_GCR0_OUT(v)      \
        out_dword(HWIO_IPA_0_GSI_TOP_XPU3_GCR0_ADDR,v)
#define HWIO_IPA_0_GSI_TOP_XPU3_GCR0_OUTM(m,v) \
        out_dword_masked_ns(HWIO_IPA_0_GSI_TOP_XPU3_GCR0_ADDR,m,v,HWIO_IPA_0_GSI_TOP_XPU3_GCR0_IN)
#define HWIO_IPA_0_GSI_TOP_XPU3_GCR0_AALOG_MODE_DIS_BMSK                            0x2
#define HWIO_IPA_0_GSI_TOP_XPU3_GCR0_AALOG_MODE_DIS_SHFT                            0x1
#define HWIO_IPA_0_GSI_TOP_XPU3_GCR0_AADEN_BMSK                                     0x1
#define HWIO_IPA_0_GSI_TOP_XPU3_GCR0_AADEN_SHFT                                     0x0

#define HWIO_IPA_0_GSI_TOP_XPU3_SCR0_ADDR                                    (IPA_0_GSI_TOP_XPU3_REG_BASE      + 0x00000008)
#define HWIO_IPA_0_GSI_TOP_XPU3_SCR0_PHYS                                    (IPA_0_GSI_TOP_XPU3_REG_BASE_PHYS + 0x00000008)
#define HWIO_IPA_0_GSI_TOP_XPU3_SCR0_OFFS                                    (IPA_0_GSI_TOP_XPU3_REG_BASE_OFFS + 0x00000008)
#define HWIO_IPA_0_GSI_TOP_XPU3_SCR0_RMSK                                         0x10f
#define HWIO_IPA_0_GSI_TOP_XPU3_SCR0_ATTR                                           0x3
#define HWIO_IPA_0_GSI_TOP_XPU3_SCR0_IN          \
        in_dword_masked(HWIO_IPA_0_GSI_TOP_XPU3_SCR0_ADDR, HWIO_IPA_0_GSI_TOP_XPU3_SCR0_RMSK)
#define HWIO_IPA_0_GSI_TOP_XPU3_SCR0_INM(m)      \
        in_dword_masked(HWIO_IPA_0_GSI_TOP_XPU3_SCR0_ADDR, m)
#define HWIO_IPA_0_GSI_TOP_XPU3_SCR0_OUT(v)      \
        out_dword(HWIO_IPA_0_GSI_TOP_XPU3_SCR0_ADDR,v)
#define HWIO_IPA_0_GSI_TOP_XPU3_SCR0_OUTM(m,v) \
        out_dword_masked_ns(HWIO_IPA_0_GSI_TOP_XPU3_SCR0_ADDR,m,v,HWIO_IPA_0_GSI_TOP_XPU3_SCR0_IN)
#define HWIO_IPA_0_GSI_TOP_XPU3_SCR0_DYNAMIC_CLK_EN_BMSK                          0x100
#define HWIO_IPA_0_GSI_TOP_XPU3_SCR0_DYNAMIC_CLK_EN_SHFT                            0x8
#define HWIO_IPA_0_GSI_TOP_XPU3_SCR0_SCLEIE_BMSK                                    0x8
#define HWIO_IPA_0_GSI_TOP_XPU3_SCR0_SCLEIE_SHFT                                    0x3
#define HWIO_IPA_0_GSI_TOP_XPU3_SCR0_SCFGEIE_BMSK                                   0x4
#define HWIO_IPA_0_GSI_TOP_XPU3_SCR0_SCFGEIE_SHFT                                   0x2
#define HWIO_IPA_0_GSI_TOP_XPU3_SCR0_SCLERE_BMSK                                    0x2
#define HWIO_IPA_0_GSI_TOP_XPU3_SCR0_SCLERE_SHFT                                    0x1
#define HWIO_IPA_0_GSI_TOP_XPU3_SCR0_SCFGERE_BMSK                                   0x1
#define HWIO_IPA_0_GSI_TOP_XPU3_SCR0_SCFGERE_SHFT                                   0x0

#define HWIO_IPA_0_GSI_TOP_XPU3_CR0_ADDR                                     (IPA_0_GSI_TOP_XPU3_REG_BASE      + 0x00000010)
#define HWIO_IPA_0_GSI_TOP_XPU3_CR0_PHYS                                     (IPA_0_GSI_TOP_XPU3_REG_BASE_PHYS + 0x00000010)
#define HWIO_IPA_0_GSI_TOP_XPU3_CR0_OFFS                                     (IPA_0_GSI_TOP_XPU3_REG_BASE_OFFS + 0x00000010)
#define HWIO_IPA_0_GSI_TOP_XPU3_CR0_RMSK                                          0x18f
#define HWIO_IPA_0_GSI_TOP_XPU3_CR0_ATTR                                            0x3
#define HWIO_IPA_0_GSI_TOP_XPU3_CR0_IN          \
        in_dword_masked(HWIO_IPA_0_GSI_TOP_XPU3_CR0_ADDR, HWIO_IPA_0_GSI_TOP_XPU3_CR0_RMSK)
#define HWIO_IPA_0_GSI_TOP_XPU3_CR0_INM(m)      \
        in_dword_masked(HWIO_IPA_0_GSI_TOP_XPU3_CR0_ADDR, m)
#define HWIO_IPA_0_GSI_TOP_XPU3_CR0_OUT(v)      \
        out_dword(HWIO_IPA_0_GSI_TOP_XPU3_CR0_ADDR,v)
#define HWIO_IPA_0_GSI_TOP_XPU3_CR0_OUTM(m,v) \
        out_dword_masked_ns(HWIO_IPA_0_GSI_TOP_XPU3_CR0_ADDR,m,v,HWIO_IPA_0_GSI_TOP_XPU3_CR0_IN)
#define HWIO_IPA_0_GSI_TOP_XPU3_CR0_DYNAMIC_CLK_EN_BMSK                           0x100
#define HWIO_IPA_0_GSI_TOP_XPU3_CR0_DYNAMIC_CLK_EN_SHFT                             0x8
#define HWIO_IPA_0_GSI_TOP_XPU3_CR0_VMIDEN_BMSK                                    0x80
#define HWIO_IPA_0_GSI_TOP_XPU3_CR0_VMIDEN_SHFT                                     0x7
#define HWIO_IPA_0_GSI_TOP_XPU3_CR0_CLEIE_BMSK                                      0x8
#define HWIO_IPA_0_GSI_TOP_XPU3_CR0_CLEIE_SHFT                                      0x3
#define HWIO_IPA_0_GSI_TOP_XPU3_CR0_CFGEIE_BMSK                                     0x4
#define HWIO_IPA_0_GSI_TOP_XPU3_CR0_CFGEIE_SHFT                                     0x2
#define HWIO_IPA_0_GSI_TOP_XPU3_CR0_CLERE_BMSK                                      0x2
#define HWIO_IPA_0_GSI_TOP_XPU3_CR0_CLERE_SHFT                                      0x1
#define HWIO_IPA_0_GSI_TOP_XPU3_CR0_CFGERE_BMSK                                     0x1
#define HWIO_IPA_0_GSI_TOP_XPU3_CR0_CFGERE_SHFT                                     0x0

#define HWIO_IPA_0_GSI_TOP_XPU3_RPU_ACR0_ADDR                                (IPA_0_GSI_TOP_XPU3_REG_BASE      + 0x00000020)
#define HWIO_IPA_0_GSI_TOP_XPU3_RPU_ACR0_PHYS                                (IPA_0_GSI_TOP_XPU3_REG_BASE_PHYS + 0x00000020)
#define HWIO_IPA_0_GSI_TOP_XPU3_RPU_ACR0_OFFS                                (IPA_0_GSI_TOP_XPU3_REG_BASE_OFFS + 0x00000020)
#define HWIO_IPA_0_GSI_TOP_XPU3_RPU_ACR0_RMSK                                0xffffffff
#define HWIO_IPA_0_GSI_TOP_XPU3_RPU_ACR0_ATTR                                       0x3
#define HWIO_IPA_0_GSI_TOP_XPU3_RPU_ACR0_IN          \
        in_dword_masked(HWIO_IPA_0_GSI_TOP_XPU3_RPU_ACR0_ADDR, HWIO_IPA_0_GSI_TOP_XPU3_RPU_ACR0_RMSK)
#define HWIO_IPA_0_GSI_TOP_XPU3_RPU_ACR0_INM(m)      \
        in_dword_masked(HWIO_IPA_0_GSI_TOP_XPU3_RPU_ACR0_ADDR, m)
#define HWIO_IPA_0_GSI_TOP_XPU3_RPU_ACR0_OUT(v)      \
        out_dword(HWIO_IPA_0_GSI_TOP_XPU3_RPU_ACR0_ADDR,v)
#define HWIO_IPA_0_GSI_TOP_XPU3_RPU_ACR0_OUTM(m,v) \
        out_dword_masked_ns(HWIO_IPA_0_GSI_TOP_XPU3_RPU_ACR0_ADDR,m,v,HWIO_IPA_0_GSI_TOP_XPU3_RPU_ACR0_IN)
#define HWIO_IPA_0_GSI_TOP_XPU3_RPU_ACR0_SUVMID_BMSK                         0xffffffff
#define HWIO_IPA_0_GSI_TOP_XPU3_RPU_ACR0_SUVMID_SHFT                                0x0

#define HWIO_IPA_0_GSI_TOP_XPU3_QAD0_GCR0_ADDR                               (IPA_0_GSI_TOP_XPU3_REG_BASE      + 0x00000080)
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD0_GCR0_PHYS                               (IPA_0_GSI_TOP_XPU3_REG_BASE_PHYS + 0x00000080)
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD0_GCR0_OFFS                               (IPA_0_GSI_TOP_XPU3_REG_BASE_OFFS + 0x00000080)
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD0_GCR0_RMSK                                      0x3
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD0_GCR0_ATTR                                      0x3
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD0_GCR0_IN          \
        in_dword_masked(HWIO_IPA_0_GSI_TOP_XPU3_QAD0_GCR0_ADDR, HWIO_IPA_0_GSI_TOP_XPU3_QAD0_GCR0_RMSK)
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD0_GCR0_INM(m)      \
        in_dword_masked(HWIO_IPA_0_GSI_TOP_XPU3_QAD0_GCR0_ADDR, m)
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD0_GCR0_OUT(v)      \
        out_dword(HWIO_IPA_0_GSI_TOP_XPU3_QAD0_GCR0_ADDR,v)
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD0_GCR0_OUTM(m,v) \
        out_dword_masked_ns(HWIO_IPA_0_GSI_TOP_XPU3_QAD0_GCR0_ADDR,m,v,HWIO_IPA_0_GSI_TOP_XPU3_QAD0_GCR0_IN)
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD0_GCR0_QAD0LOG_MODE_DIS_BMSK                     0x2
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD0_GCR0_QAD0LOG_MODE_DIS_SHFT                     0x1
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD0_GCR0_QAD0DEN_BMSK                              0x1
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD0_GCR0_QAD0DEN_SHFT                              0x0

#define HWIO_IPA_0_GSI_TOP_XPU3_QAD0_CR0_ADDR                                (IPA_0_GSI_TOP_XPU3_REG_BASE      + 0x00000090)
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD0_CR0_PHYS                                (IPA_0_GSI_TOP_XPU3_REG_BASE_PHYS + 0x00000090)
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD0_CR0_OFFS                                (IPA_0_GSI_TOP_XPU3_REG_BASE_OFFS + 0x00000090)
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD0_CR0_RMSK                                     0x10f
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD0_CR0_ATTR                                       0x3
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD0_CR0_IN          \
        in_dword_masked(HWIO_IPA_0_GSI_TOP_XPU3_QAD0_CR0_ADDR, HWIO_IPA_0_GSI_TOP_XPU3_QAD0_CR0_RMSK)
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD0_CR0_INM(m)      \
        in_dword_masked(HWIO_IPA_0_GSI_TOP_XPU3_QAD0_CR0_ADDR, m)
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD0_CR0_OUT(v)      \
        out_dword(HWIO_IPA_0_GSI_TOP_XPU3_QAD0_CR0_ADDR,v)
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD0_CR0_OUTM(m,v) \
        out_dword_masked_ns(HWIO_IPA_0_GSI_TOP_XPU3_QAD0_CR0_ADDR,m,v,HWIO_IPA_0_GSI_TOP_XPU3_QAD0_CR0_IN)
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD0_CR0_DYNAMIC_CLK_EN_BMSK                      0x100
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD0_CR0_DYNAMIC_CLK_EN_SHFT                        0x8
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD0_CR0_CLEIE_BMSK                                 0x8
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD0_CR0_CLEIE_SHFT                                 0x3
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD0_CR0_CFGEIE_BMSK                                0x4
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD0_CR0_CFGEIE_SHFT                                0x2
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD0_CR0_CLERE_BMSK                                 0x2
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD0_CR0_CLERE_SHFT                                 0x1
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD0_CR0_CFGERE_BMSK                                0x1
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD0_CR0_CFGERE_SHFT                                0x0

#define HWIO_IPA_0_GSI_TOP_XPU3_QAD1_GCR0_ADDR                               (IPA_0_GSI_TOP_XPU3_REG_BASE      + 0x00000100)
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD1_GCR0_PHYS                               (IPA_0_GSI_TOP_XPU3_REG_BASE_PHYS + 0x00000100)
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD1_GCR0_OFFS                               (IPA_0_GSI_TOP_XPU3_REG_BASE_OFFS + 0x00000100)
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD1_GCR0_RMSK                                      0x3
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD1_GCR0_ATTR                                      0x3
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD1_GCR0_IN          \
        in_dword_masked(HWIO_IPA_0_GSI_TOP_XPU3_QAD1_GCR0_ADDR, HWIO_IPA_0_GSI_TOP_XPU3_QAD1_GCR0_RMSK)
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD1_GCR0_INM(m)      \
        in_dword_masked(HWIO_IPA_0_GSI_TOP_XPU3_QAD1_GCR0_ADDR, m)
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD1_GCR0_OUT(v)      \
        out_dword(HWIO_IPA_0_GSI_TOP_XPU3_QAD1_GCR0_ADDR,v)
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD1_GCR0_OUTM(m,v) \
        out_dword_masked_ns(HWIO_IPA_0_GSI_TOP_XPU3_QAD1_GCR0_ADDR,m,v,HWIO_IPA_0_GSI_TOP_XPU3_QAD1_GCR0_IN)
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD1_GCR0_QAD1LOG_MODE_DIS_BMSK                     0x2
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD1_GCR0_QAD1LOG_MODE_DIS_SHFT                     0x1
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD1_GCR0_QAD1DEN_BMSK                              0x1
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD1_GCR0_QAD1DEN_SHFT                              0x0

#define HWIO_IPA_0_GSI_TOP_XPU3_QAD1_CR0_ADDR                                (IPA_0_GSI_TOP_XPU3_REG_BASE      + 0x00000110)
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD1_CR0_PHYS                                (IPA_0_GSI_TOP_XPU3_REG_BASE_PHYS + 0x00000110)
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD1_CR0_OFFS                                (IPA_0_GSI_TOP_XPU3_REG_BASE_OFFS + 0x00000110)
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD1_CR0_RMSK                                     0x10f
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD1_CR0_ATTR                                       0x3
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD1_CR0_IN          \
        in_dword_masked(HWIO_IPA_0_GSI_TOP_XPU3_QAD1_CR0_ADDR, HWIO_IPA_0_GSI_TOP_XPU3_QAD1_CR0_RMSK)
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD1_CR0_INM(m)      \
        in_dword_masked(HWIO_IPA_0_GSI_TOP_XPU3_QAD1_CR0_ADDR, m)
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD1_CR0_OUT(v)      \
        out_dword(HWIO_IPA_0_GSI_TOP_XPU3_QAD1_CR0_ADDR,v)
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD1_CR0_OUTM(m,v) \
        out_dword_masked_ns(HWIO_IPA_0_GSI_TOP_XPU3_QAD1_CR0_ADDR,m,v,HWIO_IPA_0_GSI_TOP_XPU3_QAD1_CR0_IN)
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD1_CR0_DYNAMIC_CLK_EN_BMSK                      0x100
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD1_CR0_DYNAMIC_CLK_EN_SHFT                        0x8
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD1_CR0_CLEIE_BMSK                                 0x8
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD1_CR0_CLEIE_SHFT                                 0x3
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD1_CR0_CFGEIE_BMSK                                0x4
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD1_CR0_CFGEIE_SHFT                                0x2
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD1_CR0_CLERE_BMSK                                 0x2
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD1_CR0_CLERE_SHFT                                 0x1
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD1_CR0_CFGERE_BMSK                                0x1
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD1_CR0_CFGERE_SHFT                                0x0

#define HWIO_IPA_0_GSI_TOP_XPU3_IDR3_ADDR                                    (IPA_0_GSI_TOP_XPU3_REG_BASE      + 0x000003ec)
#define HWIO_IPA_0_GSI_TOP_XPU3_IDR3_PHYS                                    (IPA_0_GSI_TOP_XPU3_REG_BASE_PHYS + 0x000003ec)
#define HWIO_IPA_0_GSI_TOP_XPU3_IDR3_OFFS                                    (IPA_0_GSI_TOP_XPU3_REG_BASE_OFFS + 0x000003ec)
#define HWIO_IPA_0_GSI_TOP_XPU3_IDR3_RMSK                                         0x3ff
#define HWIO_IPA_0_GSI_TOP_XPU3_IDR3_ATTR                                           0x1
#define HWIO_IPA_0_GSI_TOP_XPU3_IDR3_IN          \
        in_dword_masked(HWIO_IPA_0_GSI_TOP_XPU3_IDR3_ADDR, HWIO_IPA_0_GSI_TOP_XPU3_IDR3_RMSK)
#define HWIO_IPA_0_GSI_TOP_XPU3_IDR3_INM(m)      \
        in_dword_masked(HWIO_IPA_0_GSI_TOP_XPU3_IDR3_ADDR, m)
#define HWIO_IPA_0_GSI_TOP_XPU3_IDR3_PT_BMSK                                      0x200
#define HWIO_IPA_0_GSI_TOP_XPU3_IDR3_PT_SHFT                                        0x9
#define HWIO_IPA_0_GSI_TOP_XPU3_IDR3_MV_BMSK                                      0x100
#define HWIO_IPA_0_GSI_TOP_XPU3_IDR3_MV_SHFT                                        0x8
#define HWIO_IPA_0_GSI_TOP_XPU3_IDR3_NVMID_BMSK                                    0xff
#define HWIO_IPA_0_GSI_TOP_XPU3_IDR3_NVMID_SHFT                                     0x0

#define HWIO_IPA_0_GSI_TOP_XPU3_IDR2_ADDR                                    (IPA_0_GSI_TOP_XPU3_REG_BASE      + 0x000003f0)
#define HWIO_IPA_0_GSI_TOP_XPU3_IDR2_PHYS                                    (IPA_0_GSI_TOP_XPU3_REG_BASE_PHYS + 0x000003f0)
#define HWIO_IPA_0_GSI_TOP_XPU3_IDR2_OFFS                                    (IPA_0_GSI_TOP_XPU3_REG_BASE_OFFS + 0x000003f0)
#define HWIO_IPA_0_GSI_TOP_XPU3_IDR2_RMSK                                    0xffffff0f
#define HWIO_IPA_0_GSI_TOP_XPU3_IDR2_ATTR                                           0x1
#define HWIO_IPA_0_GSI_TOP_XPU3_IDR2_IN          \
        in_dword_masked(HWIO_IPA_0_GSI_TOP_XPU3_IDR2_ADDR, HWIO_IPA_0_GSI_TOP_XPU3_IDR2_RMSK)
#define HWIO_IPA_0_GSI_TOP_XPU3_IDR2_INM(m)      \
        in_dword_masked(HWIO_IPA_0_GSI_TOP_XPU3_IDR2_ADDR, m)
#define HWIO_IPA_0_GSI_TOP_XPU3_IDR2_NONSEC_EN_BMSK                          0xff000000
#define HWIO_IPA_0_GSI_TOP_XPU3_IDR2_NONSEC_EN_SHFT                                0x18
#define HWIO_IPA_0_GSI_TOP_XPU3_IDR2_SEC_EN_BMSK                               0xff0000
#define HWIO_IPA_0_GSI_TOP_XPU3_IDR2_SEC_EN_SHFT                                   0x10
#define HWIO_IPA_0_GSI_TOP_XPU3_IDR2_VMIDACR_EN_BMSK                             0xff00
#define HWIO_IPA_0_GSI_TOP_XPU3_IDR2_VMIDACR_EN_SHFT                                0x8
#define HWIO_IPA_0_GSI_TOP_XPU3_IDR2_NUM_QAD_BMSK                                   0xf
#define HWIO_IPA_0_GSI_TOP_XPU3_IDR2_NUM_QAD_SHFT                                   0x0

#define HWIO_IPA_0_GSI_TOP_XPU3_IDR1_ADDR                                    (IPA_0_GSI_TOP_XPU3_REG_BASE      + 0x000003f4)
#define HWIO_IPA_0_GSI_TOP_XPU3_IDR1_PHYS                                    (IPA_0_GSI_TOP_XPU3_REG_BASE_PHYS + 0x000003f4)
#define HWIO_IPA_0_GSI_TOP_XPU3_IDR1_OFFS                                    (IPA_0_GSI_TOP_XPU3_REG_BASE_OFFS + 0x000003f4)
#define HWIO_IPA_0_GSI_TOP_XPU3_IDR1_RMSK                                    0x3f3f0000
#define HWIO_IPA_0_GSI_TOP_XPU3_IDR1_ATTR                                           0x1
#define HWIO_IPA_0_GSI_TOP_XPU3_IDR1_IN          \
        in_dword_masked(HWIO_IPA_0_GSI_TOP_XPU3_IDR1_ADDR, HWIO_IPA_0_GSI_TOP_XPU3_IDR1_RMSK)
#define HWIO_IPA_0_GSI_TOP_XPU3_IDR1_INM(m)      \
        in_dword_masked(HWIO_IPA_0_GSI_TOP_XPU3_IDR1_ADDR, m)
#define HWIO_IPA_0_GSI_TOP_XPU3_IDR1_CLIENT_ADDR_WIDTH_BMSK                  0x3f000000
#define HWIO_IPA_0_GSI_TOP_XPU3_IDR1_CLIENT_ADDR_WIDTH_SHFT                        0x18
#define HWIO_IPA_0_GSI_TOP_XPU3_IDR1_CONFIG_ADDR_WIDTH_BMSK                    0x3f0000
#define HWIO_IPA_0_GSI_TOP_XPU3_IDR1_CONFIG_ADDR_WIDTH_SHFT                        0x10

#define HWIO_IPA_0_GSI_TOP_XPU3_IDR0_ADDR                                    (IPA_0_GSI_TOP_XPU3_REG_BASE      + 0x000003f8)
#define HWIO_IPA_0_GSI_TOP_XPU3_IDR0_PHYS                                    (IPA_0_GSI_TOP_XPU3_REG_BASE_PHYS + 0x000003f8)
#define HWIO_IPA_0_GSI_TOP_XPU3_IDR0_OFFS                                    (IPA_0_GSI_TOP_XPU3_REG_BASE_OFFS + 0x000003f8)
#define HWIO_IPA_0_GSI_TOP_XPU3_IDR0_RMSK                                     0x3ff0023
#define HWIO_IPA_0_GSI_TOP_XPU3_IDR0_ATTR                                           0x1
#define HWIO_IPA_0_GSI_TOP_XPU3_IDR0_IN          \
        in_dword_masked(HWIO_IPA_0_GSI_TOP_XPU3_IDR0_ADDR, HWIO_IPA_0_GSI_TOP_XPU3_IDR0_RMSK)
#define HWIO_IPA_0_GSI_TOP_XPU3_IDR0_INM(m)      \
        in_dword_masked(HWIO_IPA_0_GSI_TOP_XPU3_IDR0_ADDR, m)
#define HWIO_IPA_0_GSI_TOP_XPU3_IDR0_NRG_BMSK                                 0x3ff0000
#define HWIO_IPA_0_GSI_TOP_XPU3_IDR0_NRG_SHFT                                      0x10
#define HWIO_IPA_0_GSI_TOP_XPU3_IDR0_CLIENTREQ_HALT_ACK_HW_EN_BMSK                 0x20
#define HWIO_IPA_0_GSI_TOP_XPU3_IDR0_CLIENTREQ_HALT_ACK_HW_EN_SHFT                  0x5
#define HWIO_IPA_0_GSI_TOP_XPU3_IDR0_XPUTYPE_BMSK                                   0x3
#define HWIO_IPA_0_GSI_TOP_XPU3_IDR0_XPUTYPE_SHFT                                   0x0

#define HWIO_IPA_0_GSI_TOP_XPU3_REV_ADDR                                     (IPA_0_GSI_TOP_XPU3_REG_BASE      + 0x000003fc)
#define HWIO_IPA_0_GSI_TOP_XPU3_REV_PHYS                                     (IPA_0_GSI_TOP_XPU3_REG_BASE_PHYS + 0x000003fc)
#define HWIO_IPA_0_GSI_TOP_XPU3_REV_OFFS                                     (IPA_0_GSI_TOP_XPU3_REG_BASE_OFFS + 0x000003fc)
#define HWIO_IPA_0_GSI_TOP_XPU3_REV_RMSK                                     0xffffffff
#define HWIO_IPA_0_GSI_TOP_XPU3_REV_ATTR                                            0x1
#define HWIO_IPA_0_GSI_TOP_XPU3_REV_IN          \
        in_dword_masked(HWIO_IPA_0_GSI_TOP_XPU3_REV_ADDR, HWIO_IPA_0_GSI_TOP_XPU3_REV_RMSK)
#define HWIO_IPA_0_GSI_TOP_XPU3_REV_INM(m)      \
        in_dword_masked(HWIO_IPA_0_GSI_TOP_XPU3_REV_ADDR, m)
#define HWIO_IPA_0_GSI_TOP_XPU3_REV_MAJOR_BMSK                               0xf0000000
#define HWIO_IPA_0_GSI_TOP_XPU3_REV_MAJOR_SHFT                                     0x1c
#define HWIO_IPA_0_GSI_TOP_XPU3_REV_MINOR_BMSK                                0xfff0000
#define HWIO_IPA_0_GSI_TOP_XPU3_REV_MINOR_SHFT                                     0x10
#define HWIO_IPA_0_GSI_TOP_XPU3_REV_STEP_BMSK                                    0xffff
#define HWIO_IPA_0_GSI_TOP_XPU3_REV_STEP_SHFT                                       0x0

#define HWIO_IPA_0_GSI_TOP_XPU3_LOG_MODE_DIS_ADDR                            (IPA_0_GSI_TOP_XPU3_REG_BASE      + 0x00000400)
#define HWIO_IPA_0_GSI_TOP_XPU3_LOG_MODE_DIS_PHYS                            (IPA_0_GSI_TOP_XPU3_REG_BASE_PHYS + 0x00000400)
#define HWIO_IPA_0_GSI_TOP_XPU3_LOG_MODE_DIS_OFFS                            (IPA_0_GSI_TOP_XPU3_REG_BASE_OFFS + 0x00000400)
#define HWIO_IPA_0_GSI_TOP_XPU3_LOG_MODE_DIS_RMSK                                   0x1
#define HWIO_IPA_0_GSI_TOP_XPU3_LOG_MODE_DIS_ATTR                                   0x3
#define HWIO_IPA_0_GSI_TOP_XPU3_LOG_MODE_DIS_IN          \
        in_dword_masked(HWIO_IPA_0_GSI_TOP_XPU3_LOG_MODE_DIS_ADDR, HWIO_IPA_0_GSI_TOP_XPU3_LOG_MODE_DIS_RMSK)
#define HWIO_IPA_0_GSI_TOP_XPU3_LOG_MODE_DIS_INM(m)      \
        in_dword_masked(HWIO_IPA_0_GSI_TOP_XPU3_LOG_MODE_DIS_ADDR, m)
#define HWIO_IPA_0_GSI_TOP_XPU3_LOG_MODE_DIS_OUT(v)      \
        out_dword(HWIO_IPA_0_GSI_TOP_XPU3_LOG_MODE_DIS_ADDR,v)
#define HWIO_IPA_0_GSI_TOP_XPU3_LOG_MODE_DIS_OUTM(m,v) \
        out_dword_masked_ns(HWIO_IPA_0_GSI_TOP_XPU3_LOG_MODE_DIS_ADDR,m,v,HWIO_IPA_0_GSI_TOP_XPU3_LOG_MODE_DIS_IN)
#define HWIO_IPA_0_GSI_TOP_XPU3_LOG_MODE_DIS_LOG_MODE_DIS_BMSK                      0x1
#define HWIO_IPA_0_GSI_TOP_XPU3_LOG_MODE_DIS_LOG_MODE_DIS_SHFT                      0x0

#define HWIO_IPA_0_GSI_TOP_XPU3_RGN_FREESTATUSr_ADDR(r)                      (IPA_0_GSI_TOP_XPU3_REG_BASE      + 0x00000500 + 0x4 * (r))
#define HWIO_IPA_0_GSI_TOP_XPU3_RGN_FREESTATUSr_PHYS(r)                      (IPA_0_GSI_TOP_XPU3_REG_BASE_PHYS + 0x00000500 + 0x4 * (r))
#define HWIO_IPA_0_GSI_TOP_XPU3_RGN_FREESTATUSr_OFFS(r)                      (IPA_0_GSI_TOP_XPU3_REG_BASE_OFFS + 0x00000500 + 0x4 * (r))
#define HWIO_IPA_0_GSI_TOP_XPU3_RGN_FREESTATUSr_RMSK                           0x1fffff
#define HWIO_IPA_0_GSI_TOP_XPU3_RGN_FREESTATUSr_MAXr                                  0
#define HWIO_IPA_0_GSI_TOP_XPU3_RGN_FREESTATUSr_ATTR                                0x1
#define HWIO_IPA_0_GSI_TOP_XPU3_RGN_FREESTATUSr_INI(r)        \
        in_dword_masked(HWIO_IPA_0_GSI_TOP_XPU3_RGN_FREESTATUSr_ADDR(r), HWIO_IPA_0_GSI_TOP_XPU3_RGN_FREESTATUSr_RMSK)
#define HWIO_IPA_0_GSI_TOP_XPU3_RGN_FREESTATUSr_INMI(r,mask)    \
        in_dword_masked(HWIO_IPA_0_GSI_TOP_XPU3_RGN_FREESTATUSr_ADDR(r), mask)
#define HWIO_IPA_0_GSI_TOP_XPU3_RGN_FREESTATUSr_RGFREESTATUS_BMSK              0x1fffff
#define HWIO_IPA_0_GSI_TOP_XPU3_RGN_FREESTATUSr_RGFREESTATUS_SHFT                   0x0

#define HWIO_IPA_0_GSI_TOP_XPU3_SEAR0_ADDR                                   (IPA_0_GSI_TOP_XPU3_REG_BASE      + 0x00000800)
#define HWIO_IPA_0_GSI_TOP_XPU3_SEAR0_PHYS                                   (IPA_0_GSI_TOP_XPU3_REG_BASE_PHYS + 0x00000800)
#define HWIO_IPA_0_GSI_TOP_XPU3_SEAR0_OFFS                                   (IPA_0_GSI_TOP_XPU3_REG_BASE_OFFS + 0x00000800)
#define HWIO_IPA_0_GSI_TOP_XPU3_SEAR0_RMSK                                   0xffffffff
#define HWIO_IPA_0_GSI_TOP_XPU3_SEAR0_ATTR                                          0x1
#define HWIO_IPA_0_GSI_TOP_XPU3_SEAR0_IN          \
        in_dword_masked(HWIO_IPA_0_GSI_TOP_XPU3_SEAR0_ADDR, HWIO_IPA_0_GSI_TOP_XPU3_SEAR0_RMSK)
#define HWIO_IPA_0_GSI_TOP_XPU3_SEAR0_INM(m)      \
        in_dword_masked(HWIO_IPA_0_GSI_TOP_XPU3_SEAR0_ADDR, m)
#define HWIO_IPA_0_GSI_TOP_XPU3_SEAR0_ADDR_31_0_BMSK                         0xffffffff
#define HWIO_IPA_0_GSI_TOP_XPU3_SEAR0_ADDR_31_0_SHFT                                0x0

#define HWIO_IPA_0_GSI_TOP_XPU3_SESR_ADDR                                    (IPA_0_GSI_TOP_XPU3_REG_BASE      + 0x00000808)
#define HWIO_IPA_0_GSI_TOP_XPU3_SESR_PHYS                                    (IPA_0_GSI_TOP_XPU3_REG_BASE_PHYS + 0x00000808)
#define HWIO_IPA_0_GSI_TOP_XPU3_SESR_OFFS                                    (IPA_0_GSI_TOP_XPU3_REG_BASE_OFFS + 0x00000808)
#define HWIO_IPA_0_GSI_TOP_XPU3_SESR_RMSK                                           0xf
#define HWIO_IPA_0_GSI_TOP_XPU3_SESR_ATTR                                           0x3
#define HWIO_IPA_0_GSI_TOP_XPU3_SESR_IN          \
        in_dword_masked(HWIO_IPA_0_GSI_TOP_XPU3_SESR_ADDR, HWIO_IPA_0_GSI_TOP_XPU3_SESR_RMSK)
#define HWIO_IPA_0_GSI_TOP_XPU3_SESR_INM(m)      \
        in_dword_masked(HWIO_IPA_0_GSI_TOP_XPU3_SESR_ADDR, m)
#define HWIO_IPA_0_GSI_TOP_XPU3_SESR_OUT(v)      \
        out_dword(HWIO_IPA_0_GSI_TOP_XPU3_SESR_ADDR,v)
#define HWIO_IPA_0_GSI_TOP_XPU3_SESR_OUTM(m,v) \
        out_dword_masked_ns(HWIO_IPA_0_GSI_TOP_XPU3_SESR_ADDR,m,v,HWIO_IPA_0_GSI_TOP_XPU3_SESR_IN)
#define HWIO_IPA_0_GSI_TOP_XPU3_SESR_CLMULTI_BMSK                                   0x8
#define HWIO_IPA_0_GSI_TOP_XPU3_SESR_CLMULTI_SHFT                                   0x3
#define HWIO_IPA_0_GSI_TOP_XPU3_SESR_CFGMULTI_BMSK                                  0x4
#define HWIO_IPA_0_GSI_TOP_XPU3_SESR_CFGMULTI_SHFT                                  0x2
#define HWIO_IPA_0_GSI_TOP_XPU3_SESR_CLIENT_BMSK                                    0x2
#define HWIO_IPA_0_GSI_TOP_XPU3_SESR_CLIENT_SHFT                                    0x1
#define HWIO_IPA_0_GSI_TOP_XPU3_SESR_CFG_BMSK                                       0x1
#define HWIO_IPA_0_GSI_TOP_XPU3_SESR_CFG_SHFT                                       0x0

#define HWIO_IPA_0_GSI_TOP_XPU3_SESRRESTORE_ADDR                             (IPA_0_GSI_TOP_XPU3_REG_BASE      + 0x0000080c)
#define HWIO_IPA_0_GSI_TOP_XPU3_SESRRESTORE_PHYS                             (IPA_0_GSI_TOP_XPU3_REG_BASE_PHYS + 0x0000080c)
#define HWIO_IPA_0_GSI_TOP_XPU3_SESRRESTORE_OFFS                             (IPA_0_GSI_TOP_XPU3_REG_BASE_OFFS + 0x0000080c)
#define HWIO_IPA_0_GSI_TOP_XPU3_SESRRESTORE_RMSK                                    0xf
#define HWIO_IPA_0_GSI_TOP_XPU3_SESRRESTORE_ATTR                                    0x3
#define HWIO_IPA_0_GSI_TOP_XPU3_SESRRESTORE_IN          \
        in_dword_masked(HWIO_IPA_0_GSI_TOP_XPU3_SESRRESTORE_ADDR, HWIO_IPA_0_GSI_TOP_XPU3_SESRRESTORE_RMSK)
#define HWIO_IPA_0_GSI_TOP_XPU3_SESRRESTORE_INM(m)      \
        in_dword_masked(HWIO_IPA_0_GSI_TOP_XPU3_SESRRESTORE_ADDR, m)
#define HWIO_IPA_0_GSI_TOP_XPU3_SESRRESTORE_OUT(v)      \
        out_dword(HWIO_IPA_0_GSI_TOP_XPU3_SESRRESTORE_ADDR,v)
#define HWIO_IPA_0_GSI_TOP_XPU3_SESRRESTORE_OUTM(m,v) \
        out_dword_masked_ns(HWIO_IPA_0_GSI_TOP_XPU3_SESRRESTORE_ADDR,m,v,HWIO_IPA_0_GSI_TOP_XPU3_SESRRESTORE_IN)
#define HWIO_IPA_0_GSI_TOP_XPU3_SESRRESTORE_CLMULTI_BMSK                            0x8
#define HWIO_IPA_0_GSI_TOP_XPU3_SESRRESTORE_CLMULTI_SHFT                            0x3
#define HWIO_IPA_0_GSI_TOP_XPU3_SESRRESTORE_CFGMULTI_BMSK                           0x4
#define HWIO_IPA_0_GSI_TOP_XPU3_SESRRESTORE_CFGMULTI_SHFT                           0x2
#define HWIO_IPA_0_GSI_TOP_XPU3_SESRRESTORE_CLIENT_BMSK                             0x2
#define HWIO_IPA_0_GSI_TOP_XPU3_SESRRESTORE_CLIENT_SHFT                             0x1
#define HWIO_IPA_0_GSI_TOP_XPU3_SESRRESTORE_CFG_BMSK                                0x1
#define HWIO_IPA_0_GSI_TOP_XPU3_SESRRESTORE_CFG_SHFT                                0x0

#define HWIO_IPA_0_GSI_TOP_XPU3_SESYNR0_ADDR                                 (IPA_0_GSI_TOP_XPU3_REG_BASE      + 0x00000810)
#define HWIO_IPA_0_GSI_TOP_XPU3_SESYNR0_PHYS                                 (IPA_0_GSI_TOP_XPU3_REG_BASE_PHYS + 0x00000810)
#define HWIO_IPA_0_GSI_TOP_XPU3_SESYNR0_OFFS                                 (IPA_0_GSI_TOP_XPU3_REG_BASE_OFFS + 0x00000810)
#define HWIO_IPA_0_GSI_TOP_XPU3_SESYNR0_RMSK                                 0x67ffff0f
#define HWIO_IPA_0_GSI_TOP_XPU3_SESYNR0_ATTR                                        0x1
#define HWIO_IPA_0_GSI_TOP_XPU3_SESYNR0_IN          \
        in_dword_masked(HWIO_IPA_0_GSI_TOP_XPU3_SESYNR0_ADDR, HWIO_IPA_0_GSI_TOP_XPU3_SESYNR0_RMSK)
#define HWIO_IPA_0_GSI_TOP_XPU3_SESYNR0_INM(m)      \
        in_dword_masked(HWIO_IPA_0_GSI_TOP_XPU3_SESYNR0_ADDR, m)
#define HWIO_IPA_0_GSI_TOP_XPU3_SESYNR0_AC_BMSK                              0x40000000
#define HWIO_IPA_0_GSI_TOP_XPU3_SESYNR0_AC_SHFT                                    0x1e
#define HWIO_IPA_0_GSI_TOP_XPU3_SESYNR0_BURSTLEN_BMSK                        0x20000000
#define HWIO_IPA_0_GSI_TOP_XPU3_SESYNR0_BURSTLEN_SHFT                              0x1d
#define HWIO_IPA_0_GSI_TOP_XPU3_SESYNR0_ASIZE_BMSK                            0x7000000
#define HWIO_IPA_0_GSI_TOP_XPU3_SESYNR0_ASIZE_SHFT                                 0x18
#define HWIO_IPA_0_GSI_TOP_XPU3_SESYNR0_ALEN_BMSK                              0xff0000
#define HWIO_IPA_0_GSI_TOP_XPU3_SESYNR0_ALEN_SHFT                                  0x10
#define HWIO_IPA_0_GSI_TOP_XPU3_SESYNR0_QAD_BMSK                                 0xff00
#define HWIO_IPA_0_GSI_TOP_XPU3_SESYNR0_QAD_SHFT                                    0x8
#define HWIO_IPA_0_GSI_TOP_XPU3_SESYNR0_XPRIV_BMSK                                  0x8
#define HWIO_IPA_0_GSI_TOP_XPU3_SESYNR0_XPRIV_SHFT                                  0x3
#define HWIO_IPA_0_GSI_TOP_XPU3_SESYNR0_XINST_BMSK                                  0x4
#define HWIO_IPA_0_GSI_TOP_XPU3_SESYNR0_XINST_SHFT                                  0x2
#define HWIO_IPA_0_GSI_TOP_XPU3_SESYNR0_AWRITE_BMSK                                 0x2
#define HWIO_IPA_0_GSI_TOP_XPU3_SESYNR0_AWRITE_SHFT                                 0x1
#define HWIO_IPA_0_GSI_TOP_XPU3_SESYNR0_XPROTNS_BMSK                                0x1
#define HWIO_IPA_0_GSI_TOP_XPU3_SESYNR0_XPROTNS_SHFT                                0x0

#define HWIO_IPA_0_GSI_TOP_XPU3_SESYNR1_ADDR                                 (IPA_0_GSI_TOP_XPU3_REG_BASE      + 0x00000814)
#define HWIO_IPA_0_GSI_TOP_XPU3_SESYNR1_PHYS                                 (IPA_0_GSI_TOP_XPU3_REG_BASE_PHYS + 0x00000814)
#define HWIO_IPA_0_GSI_TOP_XPU3_SESYNR1_OFFS                                 (IPA_0_GSI_TOP_XPU3_REG_BASE_OFFS + 0x00000814)
#define HWIO_IPA_0_GSI_TOP_XPU3_SESYNR1_RMSK                                 0xffffffff
#define HWIO_IPA_0_GSI_TOP_XPU3_SESYNR1_ATTR                                        0x1
#define HWIO_IPA_0_GSI_TOP_XPU3_SESYNR1_IN          \
        in_dword_masked(HWIO_IPA_0_GSI_TOP_XPU3_SESYNR1_ADDR, HWIO_IPA_0_GSI_TOP_XPU3_SESYNR1_RMSK)
#define HWIO_IPA_0_GSI_TOP_XPU3_SESYNR1_INM(m)      \
        in_dword_masked(HWIO_IPA_0_GSI_TOP_XPU3_SESYNR1_ADDR, m)
#define HWIO_IPA_0_GSI_TOP_XPU3_SESYNR1_TID_BMSK                             0xff000000
#define HWIO_IPA_0_GSI_TOP_XPU3_SESYNR1_TID_SHFT                                   0x18
#define HWIO_IPA_0_GSI_TOP_XPU3_SESYNR1_VMID_BMSK                              0xff0000
#define HWIO_IPA_0_GSI_TOP_XPU3_SESYNR1_VMID_SHFT                                  0x10
#define HWIO_IPA_0_GSI_TOP_XPU3_SESYNR1_BID_BMSK                                 0xe000
#define HWIO_IPA_0_GSI_TOP_XPU3_SESYNR1_BID_SHFT                                    0xd
#define HWIO_IPA_0_GSI_TOP_XPU3_SESYNR1_PID_BMSK                                 0x1f00
#define HWIO_IPA_0_GSI_TOP_XPU3_SESYNR1_PID_SHFT                                    0x8
#define HWIO_IPA_0_GSI_TOP_XPU3_SESYNR1_MID_BMSK                                   0xff
#define HWIO_IPA_0_GSI_TOP_XPU3_SESYNR1_MID_SHFT                                    0x0

#define HWIO_IPA_0_GSI_TOP_XPU3_SESYNR2_ADDR                                 (IPA_0_GSI_TOP_XPU3_REG_BASE      + 0x00000818)
#define HWIO_IPA_0_GSI_TOP_XPU3_SESYNR2_PHYS                                 (IPA_0_GSI_TOP_XPU3_REG_BASE_PHYS + 0x00000818)
#define HWIO_IPA_0_GSI_TOP_XPU3_SESYNR2_OFFS                                 (IPA_0_GSI_TOP_XPU3_REG_BASE_OFFS + 0x00000818)
#define HWIO_IPA_0_GSI_TOP_XPU3_SESYNR2_RMSK                                 0xffffff87
#define HWIO_IPA_0_GSI_TOP_XPU3_SESYNR2_ATTR                                        0x1
#define HWIO_IPA_0_GSI_TOP_XPU3_SESYNR2_IN          \
        in_dword_masked(HWIO_IPA_0_GSI_TOP_XPU3_SESYNR2_ADDR, HWIO_IPA_0_GSI_TOP_XPU3_SESYNR2_RMSK)
#define HWIO_IPA_0_GSI_TOP_XPU3_SESYNR2_INM(m)      \
        in_dword_masked(HWIO_IPA_0_GSI_TOP_XPU3_SESYNR2_ADDR, m)
#define HWIO_IPA_0_GSI_TOP_XPU3_SESYNR2_BAR_BMSK                             0xc0000000
#define HWIO_IPA_0_GSI_TOP_XPU3_SESYNR2_BAR_SHFT                                   0x1e
#define HWIO_IPA_0_GSI_TOP_XPU3_SESYNR2_BURST_BMSK                           0x20000000
#define HWIO_IPA_0_GSI_TOP_XPU3_SESYNR2_BURST_SHFT                                 0x1d
#define HWIO_IPA_0_GSI_TOP_XPU3_SESYNR2_CACHEABLE_BMSK                       0x10000000
#define HWIO_IPA_0_GSI_TOP_XPU3_SESYNR2_CACHEABLE_SHFT                             0x1c
#define HWIO_IPA_0_GSI_TOP_XPU3_SESYNR2_DEVICE_BMSK                           0x8000000
#define HWIO_IPA_0_GSI_TOP_XPU3_SESYNR2_DEVICE_SHFT                                0x1b
#define HWIO_IPA_0_GSI_TOP_XPU3_SESYNR2_DEVICE_TYPE_BMSK                      0x6000000
#define HWIO_IPA_0_GSI_TOP_XPU3_SESYNR2_DEVICE_TYPE_SHFT                           0x19
#define HWIO_IPA_0_GSI_TOP_XPU3_SESYNR2_EARLYWRRESP_BMSK                      0x1000000
#define HWIO_IPA_0_GSI_TOP_XPU3_SESYNR2_EARLYWRRESP_SHFT                           0x18
#define HWIO_IPA_0_GSI_TOP_XPU3_SESYNR2_ERROR_BMSK                             0x800000
#define HWIO_IPA_0_GSI_TOP_XPU3_SESYNR2_ERROR_SHFT                                 0x17
#define HWIO_IPA_0_GSI_TOP_XPU3_SESYNR2_EXCLUSIVE_BMSK                         0x400000
#define HWIO_IPA_0_GSI_TOP_XPU3_SESYNR2_EXCLUSIVE_SHFT                             0x16
#define HWIO_IPA_0_GSI_TOP_XPU3_SESYNR2_FULL_BMSK                              0x200000
#define HWIO_IPA_0_GSI_TOP_XPU3_SESYNR2_FULL_SHFT                                  0x15
#define HWIO_IPA_0_GSI_TOP_XPU3_SESYNR2_SHARED_BMSK                            0x100000
#define HWIO_IPA_0_GSI_TOP_XPU3_SESYNR2_SHARED_SHFT                                0x14
#define HWIO_IPA_0_GSI_TOP_XPU3_SESYNR2_WRITETHROUGH_BMSK                       0x80000
#define HWIO_IPA_0_GSI_TOP_XPU3_SESYNR2_WRITETHROUGH_SHFT                          0x13
#define HWIO_IPA_0_GSI_TOP_XPU3_SESYNR2_INNERNOALLOCATE_BMSK                    0x40000
#define HWIO_IPA_0_GSI_TOP_XPU3_SESYNR2_INNERNOALLOCATE_SHFT                       0x12
#define HWIO_IPA_0_GSI_TOP_XPU3_SESYNR2_INNERCACHEABLE_BMSK                     0x20000
#define HWIO_IPA_0_GSI_TOP_XPU3_SESYNR2_INNERCACHEABLE_SHFT                        0x11
#define HWIO_IPA_0_GSI_TOP_XPU3_SESYNR2_INNERSHARED_BMSK                        0x10000
#define HWIO_IPA_0_GSI_TOP_XPU3_SESYNR2_INNERSHARED_SHFT                           0x10
#define HWIO_IPA_0_GSI_TOP_XPU3_SESYNR2_INNERTRANSIENT_BMSK                      0x8000
#define HWIO_IPA_0_GSI_TOP_XPU3_SESYNR2_INNERTRANSIENT_SHFT                         0xf
#define HWIO_IPA_0_GSI_TOP_XPU3_SESYNR2_INNERWRITETHROUGH_BMSK                   0x4000
#define HWIO_IPA_0_GSI_TOP_XPU3_SESYNR2_INNERWRITETHROUGH_SHFT                      0xe
#define HWIO_IPA_0_GSI_TOP_XPU3_SESYNR2_PORTMREL_BMSK                            0x2000
#define HWIO_IPA_0_GSI_TOP_XPU3_SESYNR2_PORTMREL_SHFT                               0xd
#define HWIO_IPA_0_GSI_TOP_XPU3_SESYNR2_ORDEREDRD_BMSK                           0x1000
#define HWIO_IPA_0_GSI_TOP_XPU3_SESYNR2_ORDEREDRD_SHFT                              0xc
#define HWIO_IPA_0_GSI_TOP_XPU3_SESYNR2_ORDEREDWR_BMSK                            0x800
#define HWIO_IPA_0_GSI_TOP_XPU3_SESYNR2_ORDEREDWR_SHFT                              0xb
#define HWIO_IPA_0_GSI_TOP_XPU3_SESYNR2_OOORD_BMSK                                0x400
#define HWIO_IPA_0_GSI_TOP_XPU3_SESYNR2_OOORD_SHFT                                  0xa
#define HWIO_IPA_0_GSI_TOP_XPU3_SESYNR2_OOOWR_BMSK                                0x200
#define HWIO_IPA_0_GSI_TOP_XPU3_SESYNR2_OOOWR_SHFT                                  0x9
#define HWIO_IPA_0_GSI_TOP_XPU3_SESYNR2_NOALLOCATE_BMSK                           0x100
#define HWIO_IPA_0_GSI_TOP_XPU3_SESYNR2_NOALLOCATE_SHFT                             0x8
#define HWIO_IPA_0_GSI_TOP_XPU3_SESYNR2_TRANSIENT_BMSK                             0x80
#define HWIO_IPA_0_GSI_TOP_XPU3_SESYNR2_TRANSIENT_SHFT                              0x7
#define HWIO_IPA_0_GSI_TOP_XPU3_SESYNR2_MEMTYPE_BMSK                                0x7
#define HWIO_IPA_0_GSI_TOP_XPU3_SESYNR2_MEMTYPE_SHFT                                0x0

#define HWIO_IPA_0_GSI_TOP_XPU3_SEAR1_ADDR                                   (IPA_0_GSI_TOP_XPU3_REG_BASE      + 0x00000804)
#define HWIO_IPA_0_GSI_TOP_XPU3_SEAR1_PHYS                                   (IPA_0_GSI_TOP_XPU3_REG_BASE_PHYS + 0x00000804)
#define HWIO_IPA_0_GSI_TOP_XPU3_SEAR1_OFFS                                   (IPA_0_GSI_TOP_XPU3_REG_BASE_OFFS + 0x00000804)
#define HWIO_IPA_0_GSI_TOP_XPU3_SEAR1_RMSK                                   0xffffffff
#define HWIO_IPA_0_GSI_TOP_XPU3_SEAR1_ATTR                                          0x1
#define HWIO_IPA_0_GSI_TOP_XPU3_SEAR1_IN          \
        in_dword_masked(HWIO_IPA_0_GSI_TOP_XPU3_SEAR1_ADDR, HWIO_IPA_0_GSI_TOP_XPU3_SEAR1_RMSK)
#define HWIO_IPA_0_GSI_TOP_XPU3_SEAR1_INM(m)      \
        in_dword_masked(HWIO_IPA_0_GSI_TOP_XPU3_SEAR1_ADDR, m)
#define HWIO_IPA_0_GSI_TOP_XPU3_SEAR1_ADDR_63_32_BMSK                        0xffffffff
#define HWIO_IPA_0_GSI_TOP_XPU3_SEAR1_ADDR_63_32_SHFT                               0x0

#define HWIO_IPA_0_GSI_TOP_XPU3_EAR0_ADDR                                    (IPA_0_GSI_TOP_XPU3_REG_BASE      + 0x00000880)
#define HWIO_IPA_0_GSI_TOP_XPU3_EAR0_PHYS                                    (IPA_0_GSI_TOP_XPU3_REG_BASE_PHYS + 0x00000880)
#define HWIO_IPA_0_GSI_TOP_XPU3_EAR0_OFFS                                    (IPA_0_GSI_TOP_XPU3_REG_BASE_OFFS + 0x00000880)
#define HWIO_IPA_0_GSI_TOP_XPU3_EAR0_RMSK                                    0xffffffff
#define HWIO_IPA_0_GSI_TOP_XPU3_EAR0_ATTR                                           0x1
#define HWIO_IPA_0_GSI_TOP_XPU3_EAR0_IN          \
        in_dword_masked(HWIO_IPA_0_GSI_TOP_XPU3_EAR0_ADDR, HWIO_IPA_0_GSI_TOP_XPU3_EAR0_RMSK)
#define HWIO_IPA_0_GSI_TOP_XPU3_EAR0_INM(m)      \
        in_dword_masked(HWIO_IPA_0_GSI_TOP_XPU3_EAR0_ADDR, m)
#define HWIO_IPA_0_GSI_TOP_XPU3_EAR0_ADDR_31_0_BMSK                          0xffffffff
#define HWIO_IPA_0_GSI_TOP_XPU3_EAR0_ADDR_31_0_SHFT                                 0x0

#define HWIO_IPA_0_GSI_TOP_XPU3_ESR_ADDR                                     (IPA_0_GSI_TOP_XPU3_REG_BASE      + 0x00000888)
#define HWIO_IPA_0_GSI_TOP_XPU3_ESR_PHYS                                     (IPA_0_GSI_TOP_XPU3_REG_BASE_PHYS + 0x00000888)
#define HWIO_IPA_0_GSI_TOP_XPU3_ESR_OFFS                                     (IPA_0_GSI_TOP_XPU3_REG_BASE_OFFS + 0x00000888)
#define HWIO_IPA_0_GSI_TOP_XPU3_ESR_RMSK                                            0xf
#define HWIO_IPA_0_GSI_TOP_XPU3_ESR_ATTR                                            0x3
#define HWIO_IPA_0_GSI_TOP_XPU3_ESR_IN          \
        in_dword_masked(HWIO_IPA_0_GSI_TOP_XPU3_ESR_ADDR, HWIO_IPA_0_GSI_TOP_XPU3_ESR_RMSK)
#define HWIO_IPA_0_GSI_TOP_XPU3_ESR_INM(m)      \
        in_dword_masked(HWIO_IPA_0_GSI_TOP_XPU3_ESR_ADDR, m)
#define HWIO_IPA_0_GSI_TOP_XPU3_ESR_OUT(v)      \
        out_dword(HWIO_IPA_0_GSI_TOP_XPU3_ESR_ADDR,v)
#define HWIO_IPA_0_GSI_TOP_XPU3_ESR_OUTM(m,v) \
        out_dword_masked_ns(HWIO_IPA_0_GSI_TOP_XPU3_ESR_ADDR,m,v,HWIO_IPA_0_GSI_TOP_XPU3_ESR_IN)
#define HWIO_IPA_0_GSI_TOP_XPU3_ESR_CLMULTI_BMSK                                    0x8
#define HWIO_IPA_0_GSI_TOP_XPU3_ESR_CLMULTI_SHFT                                    0x3
#define HWIO_IPA_0_GSI_TOP_XPU3_ESR_CFGMULTI_BMSK                                   0x4
#define HWIO_IPA_0_GSI_TOP_XPU3_ESR_CFGMULTI_SHFT                                   0x2
#define HWIO_IPA_0_GSI_TOP_XPU3_ESR_CLIENT_BMSK                                     0x2
#define HWIO_IPA_0_GSI_TOP_XPU3_ESR_CLIENT_SHFT                                     0x1
#define HWIO_IPA_0_GSI_TOP_XPU3_ESR_CFG_BMSK                                        0x1
#define HWIO_IPA_0_GSI_TOP_XPU3_ESR_CFG_SHFT                                        0x0

#define HWIO_IPA_0_GSI_TOP_XPU3_ESRRESTORE_ADDR                              (IPA_0_GSI_TOP_XPU3_REG_BASE      + 0x0000088c)
#define HWIO_IPA_0_GSI_TOP_XPU3_ESRRESTORE_PHYS                              (IPA_0_GSI_TOP_XPU3_REG_BASE_PHYS + 0x0000088c)
#define HWIO_IPA_0_GSI_TOP_XPU3_ESRRESTORE_OFFS                              (IPA_0_GSI_TOP_XPU3_REG_BASE_OFFS + 0x0000088c)
#define HWIO_IPA_0_GSI_TOP_XPU3_ESRRESTORE_RMSK                                     0xf
#define HWIO_IPA_0_GSI_TOP_XPU3_ESRRESTORE_ATTR                                     0x3
#define HWIO_IPA_0_GSI_TOP_XPU3_ESRRESTORE_IN          \
        in_dword_masked(HWIO_IPA_0_GSI_TOP_XPU3_ESRRESTORE_ADDR, HWIO_IPA_0_GSI_TOP_XPU3_ESRRESTORE_RMSK)
#define HWIO_IPA_0_GSI_TOP_XPU3_ESRRESTORE_INM(m)      \
        in_dword_masked(HWIO_IPA_0_GSI_TOP_XPU3_ESRRESTORE_ADDR, m)
#define HWIO_IPA_0_GSI_TOP_XPU3_ESRRESTORE_OUT(v)      \
        out_dword(HWIO_IPA_0_GSI_TOP_XPU3_ESRRESTORE_ADDR,v)
#define HWIO_IPA_0_GSI_TOP_XPU3_ESRRESTORE_OUTM(m,v) \
        out_dword_masked_ns(HWIO_IPA_0_GSI_TOP_XPU3_ESRRESTORE_ADDR,m,v,HWIO_IPA_0_GSI_TOP_XPU3_ESRRESTORE_IN)
#define HWIO_IPA_0_GSI_TOP_XPU3_ESRRESTORE_CLMULTI_BMSK                             0x8
#define HWIO_IPA_0_GSI_TOP_XPU3_ESRRESTORE_CLMULTI_SHFT                             0x3
#define HWIO_IPA_0_GSI_TOP_XPU3_ESRRESTORE_CFGMULTI_BMSK                            0x4
#define HWIO_IPA_0_GSI_TOP_XPU3_ESRRESTORE_CFGMULTI_SHFT                            0x2
#define HWIO_IPA_0_GSI_TOP_XPU3_ESRRESTORE_CLIENT_BMSK                              0x2
#define HWIO_IPA_0_GSI_TOP_XPU3_ESRRESTORE_CLIENT_SHFT                              0x1
#define HWIO_IPA_0_GSI_TOP_XPU3_ESRRESTORE_CFG_BMSK                                 0x1
#define HWIO_IPA_0_GSI_TOP_XPU3_ESRRESTORE_CFG_SHFT                                 0x0

#define HWIO_IPA_0_GSI_TOP_XPU3_ESYNR0_ADDR                                  (IPA_0_GSI_TOP_XPU3_REG_BASE      + 0x00000890)
#define HWIO_IPA_0_GSI_TOP_XPU3_ESYNR0_PHYS                                  (IPA_0_GSI_TOP_XPU3_REG_BASE_PHYS + 0x00000890)
#define HWIO_IPA_0_GSI_TOP_XPU3_ESYNR0_OFFS                                  (IPA_0_GSI_TOP_XPU3_REG_BASE_OFFS + 0x00000890)
#define HWIO_IPA_0_GSI_TOP_XPU3_ESYNR0_RMSK                                  0x67ffff0f
#define HWIO_IPA_0_GSI_TOP_XPU3_ESYNR0_ATTR                                         0x1
#define HWIO_IPA_0_GSI_TOP_XPU3_ESYNR0_IN          \
        in_dword_masked(HWIO_IPA_0_GSI_TOP_XPU3_ESYNR0_ADDR, HWIO_IPA_0_GSI_TOP_XPU3_ESYNR0_RMSK)
#define HWIO_IPA_0_GSI_TOP_XPU3_ESYNR0_INM(m)      \
        in_dword_masked(HWIO_IPA_0_GSI_TOP_XPU3_ESYNR0_ADDR, m)
#define HWIO_IPA_0_GSI_TOP_XPU3_ESYNR0_AC_BMSK                               0x40000000
#define HWIO_IPA_0_GSI_TOP_XPU3_ESYNR0_AC_SHFT                                     0x1e
#define HWIO_IPA_0_GSI_TOP_XPU3_ESYNR0_BURSTLEN_BMSK                         0x20000000
#define HWIO_IPA_0_GSI_TOP_XPU3_ESYNR0_BURSTLEN_SHFT                               0x1d
#define HWIO_IPA_0_GSI_TOP_XPU3_ESYNR0_ASIZE_BMSK                             0x7000000
#define HWIO_IPA_0_GSI_TOP_XPU3_ESYNR0_ASIZE_SHFT                                  0x18
#define HWIO_IPA_0_GSI_TOP_XPU3_ESYNR0_ALEN_BMSK                               0xff0000
#define HWIO_IPA_0_GSI_TOP_XPU3_ESYNR0_ALEN_SHFT                                   0x10
#define HWIO_IPA_0_GSI_TOP_XPU3_ESYNR0_QAD_BMSK                                  0xff00
#define HWIO_IPA_0_GSI_TOP_XPU3_ESYNR0_QAD_SHFT                                     0x8
#define HWIO_IPA_0_GSI_TOP_XPU3_ESYNR0_XPRIV_BMSK                                   0x8
#define HWIO_IPA_0_GSI_TOP_XPU3_ESYNR0_XPRIV_SHFT                                   0x3
#define HWIO_IPA_0_GSI_TOP_XPU3_ESYNR0_XINST_BMSK                                   0x4
#define HWIO_IPA_0_GSI_TOP_XPU3_ESYNR0_XINST_SHFT                                   0x2
#define HWIO_IPA_0_GSI_TOP_XPU3_ESYNR0_AWRITE_BMSK                                  0x2
#define HWIO_IPA_0_GSI_TOP_XPU3_ESYNR0_AWRITE_SHFT                                  0x1
#define HWIO_IPA_0_GSI_TOP_XPU3_ESYNR0_XPROTNS_BMSK                                 0x1
#define HWIO_IPA_0_GSI_TOP_XPU3_ESYNR0_XPROTNS_SHFT                                 0x0

#define HWIO_IPA_0_GSI_TOP_XPU3_ESYNR1_ADDR                                  (IPA_0_GSI_TOP_XPU3_REG_BASE      + 0x00000894)
#define HWIO_IPA_0_GSI_TOP_XPU3_ESYNR1_PHYS                                  (IPA_0_GSI_TOP_XPU3_REG_BASE_PHYS + 0x00000894)
#define HWIO_IPA_0_GSI_TOP_XPU3_ESYNR1_OFFS                                  (IPA_0_GSI_TOP_XPU3_REG_BASE_OFFS + 0x00000894)
#define HWIO_IPA_0_GSI_TOP_XPU3_ESYNR1_RMSK                                  0xffffffff
#define HWIO_IPA_0_GSI_TOP_XPU3_ESYNR1_ATTR                                         0x1
#define HWIO_IPA_0_GSI_TOP_XPU3_ESYNR1_IN          \
        in_dword_masked(HWIO_IPA_0_GSI_TOP_XPU3_ESYNR1_ADDR, HWIO_IPA_0_GSI_TOP_XPU3_ESYNR1_RMSK)
#define HWIO_IPA_0_GSI_TOP_XPU3_ESYNR1_INM(m)      \
        in_dword_masked(HWIO_IPA_0_GSI_TOP_XPU3_ESYNR1_ADDR, m)
#define HWIO_IPA_0_GSI_TOP_XPU3_ESYNR1_TID_BMSK                              0xff000000
#define HWIO_IPA_0_GSI_TOP_XPU3_ESYNR1_TID_SHFT                                    0x18
#define HWIO_IPA_0_GSI_TOP_XPU3_ESYNR1_VMID_BMSK                               0xff0000
#define HWIO_IPA_0_GSI_TOP_XPU3_ESYNR1_VMID_SHFT                                   0x10
#define HWIO_IPA_0_GSI_TOP_XPU3_ESYNR1_BID_BMSK                                  0xe000
#define HWIO_IPA_0_GSI_TOP_XPU3_ESYNR1_BID_SHFT                                     0xd
#define HWIO_IPA_0_GSI_TOP_XPU3_ESYNR1_PID_BMSK                                  0x1f00
#define HWIO_IPA_0_GSI_TOP_XPU3_ESYNR1_PID_SHFT                                     0x8
#define HWIO_IPA_0_GSI_TOP_XPU3_ESYNR1_MID_BMSK                                    0xff
#define HWIO_IPA_0_GSI_TOP_XPU3_ESYNR1_MID_SHFT                                     0x0

#define HWIO_IPA_0_GSI_TOP_XPU3_ESYNR2_ADDR                                  (IPA_0_GSI_TOP_XPU3_REG_BASE      + 0x00000898)
#define HWIO_IPA_0_GSI_TOP_XPU3_ESYNR2_PHYS                                  (IPA_0_GSI_TOP_XPU3_REG_BASE_PHYS + 0x00000898)
#define HWIO_IPA_0_GSI_TOP_XPU3_ESYNR2_OFFS                                  (IPA_0_GSI_TOP_XPU3_REG_BASE_OFFS + 0x00000898)
#define HWIO_IPA_0_GSI_TOP_XPU3_ESYNR2_RMSK                                  0xffffff87
#define HWIO_IPA_0_GSI_TOP_XPU3_ESYNR2_ATTR                                         0x1
#define HWIO_IPA_0_GSI_TOP_XPU3_ESYNR2_IN          \
        in_dword_masked(HWIO_IPA_0_GSI_TOP_XPU3_ESYNR2_ADDR, HWIO_IPA_0_GSI_TOP_XPU3_ESYNR2_RMSK)
#define HWIO_IPA_0_GSI_TOP_XPU3_ESYNR2_INM(m)      \
        in_dword_masked(HWIO_IPA_0_GSI_TOP_XPU3_ESYNR2_ADDR, m)
#define HWIO_IPA_0_GSI_TOP_XPU3_ESYNR2_BAR_BMSK                              0xc0000000
#define HWIO_IPA_0_GSI_TOP_XPU3_ESYNR2_BAR_SHFT                                    0x1e
#define HWIO_IPA_0_GSI_TOP_XPU3_ESYNR2_BURST_BMSK                            0x20000000
#define HWIO_IPA_0_GSI_TOP_XPU3_ESYNR2_BURST_SHFT                                  0x1d
#define HWIO_IPA_0_GSI_TOP_XPU3_ESYNR2_CACHEABLE_BMSK                        0x10000000
#define HWIO_IPA_0_GSI_TOP_XPU3_ESYNR2_CACHEABLE_SHFT                              0x1c
#define HWIO_IPA_0_GSI_TOP_XPU3_ESYNR2_DEVICE_BMSK                            0x8000000
#define HWIO_IPA_0_GSI_TOP_XPU3_ESYNR2_DEVICE_SHFT                                 0x1b
#define HWIO_IPA_0_GSI_TOP_XPU3_ESYNR2_DEVICE_TYPE_BMSK                       0x6000000
#define HWIO_IPA_0_GSI_TOP_XPU3_ESYNR2_DEVICE_TYPE_SHFT                            0x19
#define HWIO_IPA_0_GSI_TOP_XPU3_ESYNR2_EARLYWRRESP_BMSK                       0x1000000
#define HWIO_IPA_0_GSI_TOP_XPU3_ESYNR2_EARLYWRRESP_SHFT                            0x18
#define HWIO_IPA_0_GSI_TOP_XPU3_ESYNR2_ERROR_BMSK                              0x800000
#define HWIO_IPA_0_GSI_TOP_XPU3_ESYNR2_ERROR_SHFT                                  0x17
#define HWIO_IPA_0_GSI_TOP_XPU3_ESYNR2_EXCLUSIVE_BMSK                          0x400000
#define HWIO_IPA_0_GSI_TOP_XPU3_ESYNR2_EXCLUSIVE_SHFT                              0x16
#define HWIO_IPA_0_GSI_TOP_XPU3_ESYNR2_FULL_BMSK                               0x200000
#define HWIO_IPA_0_GSI_TOP_XPU3_ESYNR2_FULL_SHFT                                   0x15
#define HWIO_IPA_0_GSI_TOP_XPU3_ESYNR2_SHARED_BMSK                             0x100000
#define HWIO_IPA_0_GSI_TOP_XPU3_ESYNR2_SHARED_SHFT                                 0x14
#define HWIO_IPA_0_GSI_TOP_XPU3_ESYNR2_WRITETHROUGH_BMSK                        0x80000
#define HWIO_IPA_0_GSI_TOP_XPU3_ESYNR2_WRITETHROUGH_SHFT                           0x13
#define HWIO_IPA_0_GSI_TOP_XPU3_ESYNR2_INNERNOALLOCATE_BMSK                     0x40000
#define HWIO_IPA_0_GSI_TOP_XPU3_ESYNR2_INNERNOALLOCATE_SHFT                        0x12
#define HWIO_IPA_0_GSI_TOP_XPU3_ESYNR2_INNERCACHEABLE_BMSK                      0x20000
#define HWIO_IPA_0_GSI_TOP_XPU3_ESYNR2_INNERCACHEABLE_SHFT                         0x11
#define HWIO_IPA_0_GSI_TOP_XPU3_ESYNR2_INNERSHARED_BMSK                         0x10000
#define HWIO_IPA_0_GSI_TOP_XPU3_ESYNR2_INNERSHARED_SHFT                            0x10
#define HWIO_IPA_0_GSI_TOP_XPU3_ESYNR2_INNERTRANSIENT_BMSK                       0x8000
#define HWIO_IPA_0_GSI_TOP_XPU3_ESYNR2_INNERTRANSIENT_SHFT                          0xf
#define HWIO_IPA_0_GSI_TOP_XPU3_ESYNR2_INNERWRITETHROUGH_BMSK                    0x4000
#define HWIO_IPA_0_GSI_TOP_XPU3_ESYNR2_INNERWRITETHROUGH_SHFT                       0xe
#define HWIO_IPA_0_GSI_TOP_XPU3_ESYNR2_PORTMREL_BMSK                             0x2000
#define HWIO_IPA_0_GSI_TOP_XPU3_ESYNR2_PORTMREL_SHFT                                0xd
#define HWIO_IPA_0_GSI_TOP_XPU3_ESYNR2_ORDEREDRD_BMSK                            0x1000
#define HWIO_IPA_0_GSI_TOP_XPU3_ESYNR2_ORDEREDRD_SHFT                               0xc
#define HWIO_IPA_0_GSI_TOP_XPU3_ESYNR2_ORDEREDWR_BMSK                             0x800
#define HWIO_IPA_0_GSI_TOP_XPU3_ESYNR2_ORDEREDWR_SHFT                               0xb
#define HWIO_IPA_0_GSI_TOP_XPU3_ESYNR2_OOORD_BMSK                                 0x400
#define HWIO_IPA_0_GSI_TOP_XPU3_ESYNR2_OOORD_SHFT                                   0xa
#define HWIO_IPA_0_GSI_TOP_XPU3_ESYNR2_OOOWR_BMSK                                 0x200
#define HWIO_IPA_0_GSI_TOP_XPU3_ESYNR2_OOOWR_SHFT                                   0x9
#define HWIO_IPA_0_GSI_TOP_XPU3_ESYNR2_NOALLOCATE_BMSK                            0x100
#define HWIO_IPA_0_GSI_TOP_XPU3_ESYNR2_NOALLOCATE_SHFT                              0x8
#define HWIO_IPA_0_GSI_TOP_XPU3_ESYNR2_TRANSIENT_BMSK                              0x80
#define HWIO_IPA_0_GSI_TOP_XPU3_ESYNR2_TRANSIENT_SHFT                               0x7
#define HWIO_IPA_0_GSI_TOP_XPU3_ESYNR2_MEMTYPE_BMSK                                 0x7
#define HWIO_IPA_0_GSI_TOP_XPU3_ESYNR2_MEMTYPE_SHFT                                 0x0

#define HWIO_IPA_0_GSI_TOP_XPU3_EAR1_ADDR                                    (IPA_0_GSI_TOP_XPU3_REG_BASE      + 0x00000884)
#define HWIO_IPA_0_GSI_TOP_XPU3_EAR1_PHYS                                    (IPA_0_GSI_TOP_XPU3_REG_BASE_PHYS + 0x00000884)
#define HWIO_IPA_0_GSI_TOP_XPU3_EAR1_OFFS                                    (IPA_0_GSI_TOP_XPU3_REG_BASE_OFFS + 0x00000884)
#define HWIO_IPA_0_GSI_TOP_XPU3_EAR1_RMSK                                    0xffffffff
#define HWIO_IPA_0_GSI_TOP_XPU3_EAR1_ATTR                                           0x1
#define HWIO_IPA_0_GSI_TOP_XPU3_EAR1_IN          \
        in_dword_masked(HWIO_IPA_0_GSI_TOP_XPU3_EAR1_ADDR, HWIO_IPA_0_GSI_TOP_XPU3_EAR1_RMSK)
#define HWIO_IPA_0_GSI_TOP_XPU3_EAR1_INM(m)      \
        in_dword_masked(HWIO_IPA_0_GSI_TOP_XPU3_EAR1_ADDR, m)
#define HWIO_IPA_0_GSI_TOP_XPU3_EAR1_ADDR_63_32_BMSK                         0xffffffff
#define HWIO_IPA_0_GSI_TOP_XPU3_EAR1_ADDR_63_32_SHFT                                0x0

#define HWIO_IPA_0_GSI_TOP_XPU3_QAD0_EAR0_ADDR                               (IPA_0_GSI_TOP_XPU3_REG_BASE      + 0x00000880)
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD0_EAR0_PHYS                               (IPA_0_GSI_TOP_XPU3_REG_BASE_PHYS + 0x00000880)
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD0_EAR0_OFFS                               (IPA_0_GSI_TOP_XPU3_REG_BASE_OFFS + 0x00000880)
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD0_EAR0_RMSK                               0xffffffff
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD0_EAR0_ATTR                                      0x1
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD0_EAR0_IN          \
        in_dword_masked(HWIO_IPA_0_GSI_TOP_XPU3_QAD0_EAR0_ADDR, HWIO_IPA_0_GSI_TOP_XPU3_QAD0_EAR0_RMSK)
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD0_EAR0_INM(m)      \
        in_dword_masked(HWIO_IPA_0_GSI_TOP_XPU3_QAD0_EAR0_ADDR, m)
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD0_EAR0_ADDR_31_0_BMSK                     0xffffffff
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD0_EAR0_ADDR_31_0_SHFT                            0x0

#define HWIO_IPA_0_GSI_TOP_XPU3_QAD0_ESR_ADDR                                (IPA_0_GSI_TOP_XPU3_REG_BASE      + 0x00000888)
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD0_ESR_PHYS                                (IPA_0_GSI_TOP_XPU3_REG_BASE_PHYS + 0x00000888)
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD0_ESR_OFFS                                (IPA_0_GSI_TOP_XPU3_REG_BASE_OFFS + 0x00000888)
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD0_ESR_RMSK                                       0xf
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD0_ESR_ATTR                                       0x3
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD0_ESR_IN          \
        in_dword_masked(HWIO_IPA_0_GSI_TOP_XPU3_QAD0_ESR_ADDR, HWIO_IPA_0_GSI_TOP_XPU3_QAD0_ESR_RMSK)
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD0_ESR_INM(m)      \
        in_dword_masked(HWIO_IPA_0_GSI_TOP_XPU3_QAD0_ESR_ADDR, m)
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD0_ESR_OUT(v)      \
        out_dword(HWIO_IPA_0_GSI_TOP_XPU3_QAD0_ESR_ADDR,v)
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD0_ESR_OUTM(m,v) \
        out_dword_masked_ns(HWIO_IPA_0_GSI_TOP_XPU3_QAD0_ESR_ADDR,m,v,HWIO_IPA_0_GSI_TOP_XPU3_QAD0_ESR_IN)
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD0_ESR_CLMULTI_BMSK                               0x8
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD0_ESR_CLMULTI_SHFT                               0x3
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD0_ESR_CFGMULTI_BMSK                              0x4
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD0_ESR_CFGMULTI_SHFT                              0x2
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD0_ESR_CLIENT_BMSK                                0x2
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD0_ESR_CLIENT_SHFT                                0x1
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD0_ESR_CFG_BMSK                                   0x1
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD0_ESR_CFG_SHFT                                   0x0

#define HWIO_IPA_0_GSI_TOP_XPU3_QAD0_ESRRESTORE_ADDR                         (IPA_0_GSI_TOP_XPU3_REG_BASE      + 0x0000088c)
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD0_ESRRESTORE_PHYS                         (IPA_0_GSI_TOP_XPU3_REG_BASE_PHYS + 0x0000088c)
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD0_ESRRESTORE_OFFS                         (IPA_0_GSI_TOP_XPU3_REG_BASE_OFFS + 0x0000088c)
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD0_ESRRESTORE_RMSK                                0xf
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD0_ESRRESTORE_ATTR                                0x3
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD0_ESRRESTORE_IN          \
        in_dword_masked(HWIO_IPA_0_GSI_TOP_XPU3_QAD0_ESRRESTORE_ADDR, HWIO_IPA_0_GSI_TOP_XPU3_QAD0_ESRRESTORE_RMSK)
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD0_ESRRESTORE_INM(m)      \
        in_dword_masked(HWIO_IPA_0_GSI_TOP_XPU3_QAD0_ESRRESTORE_ADDR, m)
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD0_ESRRESTORE_OUT(v)      \
        out_dword(HWIO_IPA_0_GSI_TOP_XPU3_QAD0_ESRRESTORE_ADDR,v)
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD0_ESRRESTORE_OUTM(m,v) \
        out_dword_masked_ns(HWIO_IPA_0_GSI_TOP_XPU3_QAD0_ESRRESTORE_ADDR,m,v,HWIO_IPA_0_GSI_TOP_XPU3_QAD0_ESRRESTORE_IN)
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD0_ESRRESTORE_CLMULTI_BMSK                        0x8
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD0_ESRRESTORE_CLMULTI_SHFT                        0x3
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD0_ESRRESTORE_CFGMULTI_BMSK                       0x4
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD0_ESRRESTORE_CFGMULTI_SHFT                       0x2
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD0_ESRRESTORE_CLIENT_BMSK                         0x2
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD0_ESRRESTORE_CLIENT_SHFT                         0x1
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD0_ESRRESTORE_CFG_BMSK                            0x1
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD0_ESRRESTORE_CFG_SHFT                            0x0

#define HWIO_IPA_0_GSI_TOP_XPU3_QAD0_ESYNR0_ADDR                             (IPA_0_GSI_TOP_XPU3_REG_BASE      + 0x00000890)
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD0_ESYNR0_PHYS                             (IPA_0_GSI_TOP_XPU3_REG_BASE_PHYS + 0x00000890)
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD0_ESYNR0_OFFS                             (IPA_0_GSI_TOP_XPU3_REG_BASE_OFFS + 0x00000890)
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD0_ESYNR0_RMSK                             0x67ffff0f
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD0_ESYNR0_ATTR                                    0x1
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD0_ESYNR0_IN          \
        in_dword_masked(HWIO_IPA_0_GSI_TOP_XPU3_QAD0_ESYNR0_ADDR, HWIO_IPA_0_GSI_TOP_XPU3_QAD0_ESYNR0_RMSK)
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD0_ESYNR0_INM(m)      \
        in_dword_masked(HWIO_IPA_0_GSI_TOP_XPU3_QAD0_ESYNR0_ADDR, m)
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD0_ESYNR0_AC_BMSK                          0x40000000
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD0_ESYNR0_AC_SHFT                                0x1e
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD0_ESYNR0_BURSTLEN_BMSK                    0x20000000
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD0_ESYNR0_BURSTLEN_SHFT                          0x1d
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD0_ESYNR0_ASIZE_BMSK                        0x7000000
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD0_ESYNR0_ASIZE_SHFT                             0x18
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD0_ESYNR0_ALEN_BMSK                          0xff0000
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD0_ESYNR0_ALEN_SHFT                              0x10
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD0_ESYNR0_QAD_BMSK                             0xff00
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD0_ESYNR0_QAD_SHFT                                0x8
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD0_ESYNR0_XPRIV_BMSK                              0x8
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD0_ESYNR0_XPRIV_SHFT                              0x3
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD0_ESYNR0_XINST_BMSK                              0x4
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD0_ESYNR0_XINST_SHFT                              0x2
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD0_ESYNR0_AWRITE_BMSK                             0x2
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD0_ESYNR0_AWRITE_SHFT                             0x1
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD0_ESYNR0_XPROTNS_BMSK                            0x1
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD0_ESYNR0_XPROTNS_SHFT                            0x0

#define HWIO_IPA_0_GSI_TOP_XPU3_QAD0_ESYNR1_ADDR                             (IPA_0_GSI_TOP_XPU3_REG_BASE      + 0x00000894)
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD0_ESYNR1_PHYS                             (IPA_0_GSI_TOP_XPU3_REG_BASE_PHYS + 0x00000894)
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD0_ESYNR1_OFFS                             (IPA_0_GSI_TOP_XPU3_REG_BASE_OFFS + 0x00000894)
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD0_ESYNR1_RMSK                             0xffffffff
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD0_ESYNR1_ATTR                                    0x1
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD0_ESYNR1_IN          \
        in_dword_masked(HWIO_IPA_0_GSI_TOP_XPU3_QAD0_ESYNR1_ADDR, HWIO_IPA_0_GSI_TOP_XPU3_QAD0_ESYNR1_RMSK)
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD0_ESYNR1_INM(m)      \
        in_dword_masked(HWIO_IPA_0_GSI_TOP_XPU3_QAD0_ESYNR1_ADDR, m)
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD0_ESYNR1_TID_BMSK                         0xff000000
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD0_ESYNR1_TID_SHFT                               0x18
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD0_ESYNR1_VMID_BMSK                          0xff0000
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD0_ESYNR1_VMID_SHFT                              0x10
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD0_ESYNR1_BID_BMSK                             0xe000
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD0_ESYNR1_BID_SHFT                                0xd
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD0_ESYNR1_PID_BMSK                             0x1f00
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD0_ESYNR1_PID_SHFT                                0x8
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD0_ESYNR1_MID_BMSK                               0xff
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD0_ESYNR1_MID_SHFT                                0x0

#define HWIO_IPA_0_GSI_TOP_XPU3_QAD0_ESYNR2_ADDR                             (IPA_0_GSI_TOP_XPU3_REG_BASE      + 0x00000898)
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD0_ESYNR2_PHYS                             (IPA_0_GSI_TOP_XPU3_REG_BASE_PHYS + 0x00000898)
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD0_ESYNR2_OFFS                             (IPA_0_GSI_TOP_XPU3_REG_BASE_OFFS + 0x00000898)
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD0_ESYNR2_RMSK                             0xffffff87
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD0_ESYNR2_ATTR                                    0x1
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD0_ESYNR2_IN          \
        in_dword_masked(HWIO_IPA_0_GSI_TOP_XPU3_QAD0_ESYNR2_ADDR, HWIO_IPA_0_GSI_TOP_XPU3_QAD0_ESYNR2_RMSK)
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD0_ESYNR2_INM(m)      \
        in_dword_masked(HWIO_IPA_0_GSI_TOP_XPU3_QAD0_ESYNR2_ADDR, m)
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD0_ESYNR2_BAR_BMSK                         0xc0000000
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD0_ESYNR2_BAR_SHFT                               0x1e
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD0_ESYNR2_BURST_BMSK                       0x20000000
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD0_ESYNR2_BURST_SHFT                             0x1d
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD0_ESYNR2_CACHEABLE_BMSK                   0x10000000
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD0_ESYNR2_CACHEABLE_SHFT                         0x1c
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD0_ESYNR2_DEVICE_BMSK                       0x8000000
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD0_ESYNR2_DEVICE_SHFT                            0x1b
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD0_ESYNR2_DEVICE_TYPE_BMSK                  0x6000000
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD0_ESYNR2_DEVICE_TYPE_SHFT                       0x19
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD0_ESYNR2_EARLYWRRESP_BMSK                  0x1000000
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD0_ESYNR2_EARLYWRRESP_SHFT                       0x18
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD0_ESYNR2_ERROR_BMSK                         0x800000
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD0_ESYNR2_ERROR_SHFT                             0x17
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD0_ESYNR2_EXCLUSIVE_BMSK                     0x400000
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD0_ESYNR2_EXCLUSIVE_SHFT                         0x16
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD0_ESYNR2_FULL_BMSK                          0x200000
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD0_ESYNR2_FULL_SHFT                              0x15
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD0_ESYNR2_SHARED_BMSK                        0x100000
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD0_ESYNR2_SHARED_SHFT                            0x14
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD0_ESYNR2_WRITETHROUGH_BMSK                   0x80000
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD0_ESYNR2_WRITETHROUGH_SHFT                      0x13
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD0_ESYNR2_INNERNOALLOCATE_BMSK                0x40000
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD0_ESYNR2_INNERNOALLOCATE_SHFT                   0x12
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD0_ESYNR2_INNERCACHEABLE_BMSK                 0x20000
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD0_ESYNR2_INNERCACHEABLE_SHFT                    0x11
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD0_ESYNR2_INNERSHARED_BMSK                    0x10000
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD0_ESYNR2_INNERSHARED_SHFT                       0x10
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD0_ESYNR2_INNERTRANSIENT_BMSK                  0x8000
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD0_ESYNR2_INNERTRANSIENT_SHFT                     0xf
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD0_ESYNR2_INNERWRITETHROUGH_BMSK               0x4000
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD0_ESYNR2_INNERWRITETHROUGH_SHFT                  0xe
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD0_ESYNR2_PORTMREL_BMSK                        0x2000
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD0_ESYNR2_PORTMREL_SHFT                           0xd
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD0_ESYNR2_ORDEREDRD_BMSK                       0x1000
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD0_ESYNR2_ORDEREDRD_SHFT                          0xc
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD0_ESYNR2_ORDEREDWR_BMSK                        0x800
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD0_ESYNR2_ORDEREDWR_SHFT                          0xb
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD0_ESYNR2_OOORD_BMSK                            0x400
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD0_ESYNR2_OOORD_SHFT                              0xa
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD0_ESYNR2_OOOWR_BMSK                            0x200
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD0_ESYNR2_OOOWR_SHFT                              0x9
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD0_ESYNR2_NOALLOCATE_BMSK                       0x100
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD0_ESYNR2_NOALLOCATE_SHFT                         0x8
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD0_ESYNR2_TRANSIENT_BMSK                         0x80
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD0_ESYNR2_TRANSIENT_SHFT                          0x7
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD0_ESYNR2_MEMTYPE_BMSK                            0x7
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD0_ESYNR2_MEMTYPE_SHFT                            0x0

#define HWIO_IPA_0_GSI_TOP_XPU3_QAD0_EAR1_ADDR                               (IPA_0_GSI_TOP_XPU3_REG_BASE      + 0x00000884)
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD0_EAR1_PHYS                               (IPA_0_GSI_TOP_XPU3_REG_BASE_PHYS + 0x00000884)
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD0_EAR1_OFFS                               (IPA_0_GSI_TOP_XPU3_REG_BASE_OFFS + 0x00000884)
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD0_EAR1_RMSK                               0xffffffff
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD0_EAR1_ATTR                                      0x1
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD0_EAR1_IN          \
        in_dword_masked(HWIO_IPA_0_GSI_TOP_XPU3_QAD0_EAR1_ADDR, HWIO_IPA_0_GSI_TOP_XPU3_QAD0_EAR1_RMSK)
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD0_EAR1_INM(m)      \
        in_dword_masked(HWIO_IPA_0_GSI_TOP_XPU3_QAD0_EAR1_ADDR, m)
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD0_EAR1_ADDR_63_32_BMSK                    0xffffffff
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD0_EAR1_ADDR_63_32_SHFT                           0x0

#define HWIO_IPA_0_GSI_TOP_XPU3_QAD1_EAR0_ADDR                               (IPA_0_GSI_TOP_XPU3_REG_BASE      + 0x00000880)
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD1_EAR0_PHYS                               (IPA_0_GSI_TOP_XPU3_REG_BASE_PHYS + 0x00000880)
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD1_EAR0_OFFS                               (IPA_0_GSI_TOP_XPU3_REG_BASE_OFFS + 0x00000880)
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD1_EAR0_RMSK                               0xffffffff
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD1_EAR0_ATTR                                      0x1
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD1_EAR0_IN          \
        in_dword_masked(HWIO_IPA_0_GSI_TOP_XPU3_QAD1_EAR0_ADDR, HWIO_IPA_0_GSI_TOP_XPU3_QAD1_EAR0_RMSK)
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD1_EAR0_INM(m)      \
        in_dword_masked(HWIO_IPA_0_GSI_TOP_XPU3_QAD1_EAR0_ADDR, m)
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD1_EAR0_ADDR_31_0_BMSK                     0xffffffff
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD1_EAR0_ADDR_31_0_SHFT                            0x0

#define HWIO_IPA_0_GSI_TOP_XPU3_QAD1_ESR_ADDR                                (IPA_0_GSI_TOP_XPU3_REG_BASE      + 0x00000888)
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD1_ESR_PHYS                                (IPA_0_GSI_TOP_XPU3_REG_BASE_PHYS + 0x00000888)
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD1_ESR_OFFS                                (IPA_0_GSI_TOP_XPU3_REG_BASE_OFFS + 0x00000888)
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD1_ESR_RMSK                                       0xf
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD1_ESR_ATTR                                       0x3
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD1_ESR_IN          \
        in_dword_masked(HWIO_IPA_0_GSI_TOP_XPU3_QAD1_ESR_ADDR, HWIO_IPA_0_GSI_TOP_XPU3_QAD1_ESR_RMSK)
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD1_ESR_INM(m)      \
        in_dword_masked(HWIO_IPA_0_GSI_TOP_XPU3_QAD1_ESR_ADDR, m)
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD1_ESR_OUT(v)      \
        out_dword(HWIO_IPA_0_GSI_TOP_XPU3_QAD1_ESR_ADDR,v)
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD1_ESR_OUTM(m,v) \
        out_dword_masked_ns(HWIO_IPA_0_GSI_TOP_XPU3_QAD1_ESR_ADDR,m,v,HWIO_IPA_0_GSI_TOP_XPU3_QAD1_ESR_IN)
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD1_ESR_CLMULTI_BMSK                               0x8
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD1_ESR_CLMULTI_SHFT                               0x3
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD1_ESR_CFGMULTI_BMSK                              0x4
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD1_ESR_CFGMULTI_SHFT                              0x2
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD1_ESR_CLIENT_BMSK                                0x2
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD1_ESR_CLIENT_SHFT                                0x1
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD1_ESR_CFG_BMSK                                   0x1
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD1_ESR_CFG_SHFT                                   0x0

#define HWIO_IPA_0_GSI_TOP_XPU3_QAD1_ESRRESTORE_ADDR                         (IPA_0_GSI_TOP_XPU3_REG_BASE      + 0x0000088c)
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD1_ESRRESTORE_PHYS                         (IPA_0_GSI_TOP_XPU3_REG_BASE_PHYS + 0x0000088c)
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD1_ESRRESTORE_OFFS                         (IPA_0_GSI_TOP_XPU3_REG_BASE_OFFS + 0x0000088c)
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD1_ESRRESTORE_RMSK                                0xf
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD1_ESRRESTORE_ATTR                                0x3
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD1_ESRRESTORE_IN          \
        in_dword_masked(HWIO_IPA_0_GSI_TOP_XPU3_QAD1_ESRRESTORE_ADDR, HWIO_IPA_0_GSI_TOP_XPU3_QAD1_ESRRESTORE_RMSK)
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD1_ESRRESTORE_INM(m)      \
        in_dword_masked(HWIO_IPA_0_GSI_TOP_XPU3_QAD1_ESRRESTORE_ADDR, m)
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD1_ESRRESTORE_OUT(v)      \
        out_dword(HWIO_IPA_0_GSI_TOP_XPU3_QAD1_ESRRESTORE_ADDR,v)
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD1_ESRRESTORE_OUTM(m,v) \
        out_dword_masked_ns(HWIO_IPA_0_GSI_TOP_XPU3_QAD1_ESRRESTORE_ADDR,m,v,HWIO_IPA_0_GSI_TOP_XPU3_QAD1_ESRRESTORE_IN)
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD1_ESRRESTORE_CLMULTI_BMSK                        0x8
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD1_ESRRESTORE_CLMULTI_SHFT                        0x3
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD1_ESRRESTORE_CFGMULTI_BMSK                       0x4
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD1_ESRRESTORE_CFGMULTI_SHFT                       0x2
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD1_ESRRESTORE_CLIENT_BMSK                         0x2
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD1_ESRRESTORE_CLIENT_SHFT                         0x1
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD1_ESRRESTORE_CFG_BMSK                            0x1
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD1_ESRRESTORE_CFG_SHFT                            0x0

#define HWIO_IPA_0_GSI_TOP_XPU3_QAD1_ESYNR0_ADDR                             (IPA_0_GSI_TOP_XPU3_REG_BASE      + 0x00000890)
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD1_ESYNR0_PHYS                             (IPA_0_GSI_TOP_XPU3_REG_BASE_PHYS + 0x00000890)
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD1_ESYNR0_OFFS                             (IPA_0_GSI_TOP_XPU3_REG_BASE_OFFS + 0x00000890)
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD1_ESYNR0_RMSK                             0x67ffff0f
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD1_ESYNR0_ATTR                                    0x1
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD1_ESYNR0_IN          \
        in_dword_masked(HWIO_IPA_0_GSI_TOP_XPU3_QAD1_ESYNR0_ADDR, HWIO_IPA_0_GSI_TOP_XPU3_QAD1_ESYNR0_RMSK)
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD1_ESYNR0_INM(m)      \
        in_dword_masked(HWIO_IPA_0_GSI_TOP_XPU3_QAD1_ESYNR0_ADDR, m)
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD1_ESYNR0_AC_BMSK                          0x40000000
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD1_ESYNR0_AC_SHFT                                0x1e
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD1_ESYNR0_BURSTLEN_BMSK                    0x20000000
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD1_ESYNR0_BURSTLEN_SHFT                          0x1d
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD1_ESYNR0_ASIZE_BMSK                        0x7000000
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD1_ESYNR0_ASIZE_SHFT                             0x18
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD1_ESYNR0_ALEN_BMSK                          0xff0000
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD1_ESYNR0_ALEN_SHFT                              0x10
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD1_ESYNR0_QAD_BMSK                             0xff00
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD1_ESYNR0_QAD_SHFT                                0x8
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD1_ESYNR0_XPRIV_BMSK                              0x8
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD1_ESYNR0_XPRIV_SHFT                              0x3
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD1_ESYNR0_XINST_BMSK                              0x4
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD1_ESYNR0_XINST_SHFT                              0x2
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD1_ESYNR0_AWRITE_BMSK                             0x2
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD1_ESYNR0_AWRITE_SHFT                             0x1
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD1_ESYNR0_XPROTNS_BMSK                            0x1
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD1_ESYNR0_XPROTNS_SHFT                            0x0

#define HWIO_IPA_0_GSI_TOP_XPU3_QAD1_ESYNR1_ADDR                             (IPA_0_GSI_TOP_XPU3_REG_BASE      + 0x00000894)
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD1_ESYNR1_PHYS                             (IPA_0_GSI_TOP_XPU3_REG_BASE_PHYS + 0x00000894)
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD1_ESYNR1_OFFS                             (IPA_0_GSI_TOP_XPU3_REG_BASE_OFFS + 0x00000894)
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD1_ESYNR1_RMSK                             0xffffffff
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD1_ESYNR1_ATTR                                    0x1
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD1_ESYNR1_IN          \
        in_dword_masked(HWIO_IPA_0_GSI_TOP_XPU3_QAD1_ESYNR1_ADDR, HWIO_IPA_0_GSI_TOP_XPU3_QAD1_ESYNR1_RMSK)
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD1_ESYNR1_INM(m)      \
        in_dword_masked(HWIO_IPA_0_GSI_TOP_XPU3_QAD1_ESYNR1_ADDR, m)
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD1_ESYNR1_TID_BMSK                         0xff000000
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD1_ESYNR1_TID_SHFT                               0x18
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD1_ESYNR1_VMID_BMSK                          0xff0000
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD1_ESYNR1_VMID_SHFT                              0x10
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD1_ESYNR1_BID_BMSK                             0xe000
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD1_ESYNR1_BID_SHFT                                0xd
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD1_ESYNR1_PID_BMSK                             0x1f00
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD1_ESYNR1_PID_SHFT                                0x8
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD1_ESYNR1_MID_BMSK                               0xff
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD1_ESYNR1_MID_SHFT                                0x0

#define HWIO_IPA_0_GSI_TOP_XPU3_QAD1_ESYNR2_ADDR                             (IPA_0_GSI_TOP_XPU3_REG_BASE      + 0x00000898)
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD1_ESYNR2_PHYS                             (IPA_0_GSI_TOP_XPU3_REG_BASE_PHYS + 0x00000898)
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD1_ESYNR2_OFFS                             (IPA_0_GSI_TOP_XPU3_REG_BASE_OFFS + 0x00000898)
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD1_ESYNR2_RMSK                             0xffffff87
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD1_ESYNR2_ATTR                                    0x1
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD1_ESYNR2_IN          \
        in_dword_masked(HWIO_IPA_0_GSI_TOP_XPU3_QAD1_ESYNR2_ADDR, HWIO_IPA_0_GSI_TOP_XPU3_QAD1_ESYNR2_RMSK)
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD1_ESYNR2_INM(m)      \
        in_dword_masked(HWIO_IPA_0_GSI_TOP_XPU3_QAD1_ESYNR2_ADDR, m)
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD1_ESYNR2_BAR_BMSK                         0xc0000000
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD1_ESYNR2_BAR_SHFT                               0x1e
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD1_ESYNR2_BURST_BMSK                       0x20000000
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD1_ESYNR2_BURST_SHFT                             0x1d
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD1_ESYNR2_CACHEABLE_BMSK                   0x10000000
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD1_ESYNR2_CACHEABLE_SHFT                         0x1c
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD1_ESYNR2_DEVICE_BMSK                       0x8000000
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD1_ESYNR2_DEVICE_SHFT                            0x1b
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD1_ESYNR2_DEVICE_TYPE_BMSK                  0x6000000
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD1_ESYNR2_DEVICE_TYPE_SHFT                       0x19
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD1_ESYNR2_EARLYWRRESP_BMSK                  0x1000000
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD1_ESYNR2_EARLYWRRESP_SHFT                       0x18
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD1_ESYNR2_ERROR_BMSK                         0x800000
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD1_ESYNR2_ERROR_SHFT                             0x17
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD1_ESYNR2_EXCLUSIVE_BMSK                     0x400000
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD1_ESYNR2_EXCLUSIVE_SHFT                         0x16
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD1_ESYNR2_FULL_BMSK                          0x200000
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD1_ESYNR2_FULL_SHFT                              0x15
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD1_ESYNR2_SHARED_BMSK                        0x100000
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD1_ESYNR2_SHARED_SHFT                            0x14
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD1_ESYNR2_WRITETHROUGH_BMSK                   0x80000
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD1_ESYNR2_WRITETHROUGH_SHFT                      0x13
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD1_ESYNR2_INNERNOALLOCATE_BMSK                0x40000
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD1_ESYNR2_INNERNOALLOCATE_SHFT                   0x12
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD1_ESYNR2_INNERCACHEABLE_BMSK                 0x20000
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD1_ESYNR2_INNERCACHEABLE_SHFT                    0x11
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD1_ESYNR2_INNERSHARED_BMSK                    0x10000
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD1_ESYNR2_INNERSHARED_SHFT                       0x10
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD1_ESYNR2_INNERTRANSIENT_BMSK                  0x8000
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD1_ESYNR2_INNERTRANSIENT_SHFT                     0xf
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD1_ESYNR2_INNERWRITETHROUGH_BMSK               0x4000
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD1_ESYNR2_INNERWRITETHROUGH_SHFT                  0xe
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD1_ESYNR2_PORTMREL_BMSK                        0x2000
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD1_ESYNR2_PORTMREL_SHFT                           0xd
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD1_ESYNR2_ORDEREDRD_BMSK                       0x1000
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD1_ESYNR2_ORDEREDRD_SHFT                          0xc
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD1_ESYNR2_ORDEREDWR_BMSK                        0x800
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD1_ESYNR2_ORDEREDWR_SHFT                          0xb
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD1_ESYNR2_OOORD_BMSK                            0x400
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD1_ESYNR2_OOORD_SHFT                              0xa
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD1_ESYNR2_OOOWR_BMSK                            0x200
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD1_ESYNR2_OOOWR_SHFT                              0x9
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD1_ESYNR2_NOALLOCATE_BMSK                       0x100
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD1_ESYNR2_NOALLOCATE_SHFT                         0x8
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD1_ESYNR2_TRANSIENT_BMSK                         0x80
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD1_ESYNR2_TRANSIENT_SHFT                          0x7
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD1_ESYNR2_MEMTYPE_BMSK                            0x7
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD1_ESYNR2_MEMTYPE_SHFT                            0x0

#define HWIO_IPA_0_GSI_TOP_XPU3_QAD1_EAR1_ADDR                               (IPA_0_GSI_TOP_XPU3_REG_BASE      + 0x00000884)
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD1_EAR1_PHYS                               (IPA_0_GSI_TOP_XPU3_REG_BASE_PHYS + 0x00000884)
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD1_EAR1_OFFS                               (IPA_0_GSI_TOP_XPU3_REG_BASE_OFFS + 0x00000884)
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD1_EAR1_RMSK                               0xffffffff
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD1_EAR1_ATTR                                      0x1
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD1_EAR1_IN          \
        in_dword_masked(HWIO_IPA_0_GSI_TOP_XPU3_QAD1_EAR1_ADDR, HWIO_IPA_0_GSI_TOP_XPU3_QAD1_EAR1_RMSK)
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD1_EAR1_INM(m)      \
        in_dword_masked(HWIO_IPA_0_GSI_TOP_XPU3_QAD1_EAR1_ADDR, m)
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD1_EAR1_ADDR_63_32_BMSK                    0xffffffff
#define HWIO_IPA_0_GSI_TOP_XPU3_QAD1_EAR1_ADDR_63_32_SHFT                           0x0

#define HWIO_IPA_0_GSI_TOP_XPU3_RGN_OWNERSTATUSr_ADDR(r)                     (IPA_0_GSI_TOP_XPU3_REG_BASE      + 0x00000900 + 0x4 * (r))
#define HWIO_IPA_0_GSI_TOP_XPU3_RGN_OWNERSTATUSr_PHYS(r)                     (IPA_0_GSI_TOP_XPU3_REG_BASE_PHYS + 0x00000900 + 0x4 * (r))
#define HWIO_IPA_0_GSI_TOP_XPU3_RGN_OWNERSTATUSr_OFFS(r)                     (IPA_0_GSI_TOP_XPU3_REG_BASE_OFFS + 0x00000900 + 0x4 * (r))
#define HWIO_IPA_0_GSI_TOP_XPU3_RGN_OWNERSTATUSr_RMSK                          0x1fffff
#define HWIO_IPA_0_GSI_TOP_XPU3_RGN_OWNERSTATUSr_MAXr                                 0
#define HWIO_IPA_0_GSI_TOP_XPU3_RGN_OWNERSTATUSr_ATTR                               0x1
#define HWIO_IPA_0_GSI_TOP_XPU3_RGN_OWNERSTATUSr_INI(r)        \
        in_dword_masked(HWIO_IPA_0_GSI_TOP_XPU3_RGN_OWNERSTATUSr_ADDR(r), HWIO_IPA_0_GSI_TOP_XPU3_RGN_OWNERSTATUSr_RMSK)
#define HWIO_IPA_0_GSI_TOP_XPU3_RGN_OWNERSTATUSr_INMI(r,mask)    \
        in_dword_masked(HWIO_IPA_0_GSI_TOP_XPU3_RGN_OWNERSTATUSr_ADDR(r), mask)
#define HWIO_IPA_0_GSI_TOP_XPU3_RGN_OWNERSTATUSr_RGOWNERSTATUS_BMSK            0x1fffff
#define HWIO_IPA_0_GSI_TOP_XPU3_RGN_OWNERSTATUSr_RGOWNERSTATUS_SHFT                 0x0

#define HWIO_IPA_0_GSI_TOP_XPU3_RGn_GCR0_ADDR(n)                             (IPA_0_GSI_TOP_XPU3_REG_BASE      + 0x00001000 + 0x80 * (n))
#define HWIO_IPA_0_GSI_TOP_XPU3_RGn_GCR0_PHYS(n)                             (IPA_0_GSI_TOP_XPU3_REG_BASE_PHYS + 0x00001000 + 0x80 * (n))
#define HWIO_IPA_0_GSI_TOP_XPU3_RGn_GCR0_OFFS(n)                             (IPA_0_GSI_TOP_XPU3_REG_BASE_OFFS + 0x00001000 + 0x80 * (n))
#define HWIO_IPA_0_GSI_TOP_XPU3_RGn_GCR0_RMSK                                     0x107
#define HWIO_IPA_0_GSI_TOP_XPU3_RGn_GCR0_MAXn                                        20
#define HWIO_IPA_0_GSI_TOP_XPU3_RGn_GCR0_ATTR                                       0x3
#define HWIO_IPA_0_GSI_TOP_XPU3_RGn_GCR0_INI(n)        \
        in_dword_masked(HWIO_IPA_0_GSI_TOP_XPU3_RGn_GCR0_ADDR(n), HWIO_IPA_0_GSI_TOP_XPU3_RGn_GCR0_RMSK)
#define HWIO_IPA_0_GSI_TOP_XPU3_RGn_GCR0_INMI(n,mask)    \
        in_dword_masked(HWIO_IPA_0_GSI_TOP_XPU3_RGn_GCR0_ADDR(n), mask)
#define HWIO_IPA_0_GSI_TOP_XPU3_RGn_GCR0_OUTI(n,val)    \
        out_dword(HWIO_IPA_0_GSI_TOP_XPU3_RGn_GCR0_ADDR(n),val)
#define HWIO_IPA_0_GSI_TOP_XPU3_RGn_GCR0_OUTMI(n,mask,val) \
        out_dword_masked_ns(HWIO_IPA_0_GSI_TOP_XPU3_RGn_GCR0_ADDR(n),mask,val,HWIO_IPA_0_GSI_TOP_XPU3_RGn_GCR0_INI(n))
#define HWIO_IPA_0_GSI_TOP_XPU3_RGn_GCR0_RG_SEC_APPS_BMSK                         0x100
#define HWIO_IPA_0_GSI_TOP_XPU3_RGn_GCR0_RG_SEC_APPS_SHFT                           0x8
#define HWIO_IPA_0_GSI_TOP_XPU3_RGn_GCR0_RG_OWNER_BMSK                              0x7
#define HWIO_IPA_0_GSI_TOP_XPU3_RGn_GCR0_RG_OWNER_SHFT                              0x0

#define HWIO_IPA_0_GSI_TOP_XPU3_RGn_GCR3_ADDR(n)                             (IPA_0_GSI_TOP_XPU3_REG_BASE      + 0x0000100c + 0x80 * (n))
#define HWIO_IPA_0_GSI_TOP_XPU3_RGn_GCR3_PHYS(n)                             (IPA_0_GSI_TOP_XPU3_REG_BASE_PHYS + 0x0000100c + 0x80 * (n))
#define HWIO_IPA_0_GSI_TOP_XPU3_RGn_GCR3_OFFS(n)                             (IPA_0_GSI_TOP_XPU3_REG_BASE_OFFS + 0x0000100c + 0x80 * (n))
#define HWIO_IPA_0_GSI_TOP_XPU3_RGn_GCR3_RMSK                                       0x1
#define HWIO_IPA_0_GSI_TOP_XPU3_RGn_GCR3_MAXn                                        20
#define HWIO_IPA_0_GSI_TOP_XPU3_RGn_GCR3_ATTR                                       0x3
#define HWIO_IPA_0_GSI_TOP_XPU3_RGn_GCR3_INI(n)        \
        in_dword_masked(HWIO_IPA_0_GSI_TOP_XPU3_RGn_GCR3_ADDR(n), HWIO_IPA_0_GSI_TOP_XPU3_RGn_GCR3_RMSK)
#define HWIO_IPA_0_GSI_TOP_XPU3_RGn_GCR3_INMI(n,mask)    \
        in_dword_masked(HWIO_IPA_0_GSI_TOP_XPU3_RGn_GCR3_ADDR(n), mask)
#define HWIO_IPA_0_GSI_TOP_XPU3_RGn_GCR3_OUTI(n,val)    \
        out_dword(HWIO_IPA_0_GSI_TOP_XPU3_RGn_GCR3_ADDR(n),val)
#define HWIO_IPA_0_GSI_TOP_XPU3_RGn_GCR3_OUTMI(n,mask,val) \
        out_dword_masked_ns(HWIO_IPA_0_GSI_TOP_XPU3_RGn_GCR3_ADDR(n),mask,val,HWIO_IPA_0_GSI_TOP_XPU3_RGn_GCR3_INI(n))
#define HWIO_IPA_0_GSI_TOP_XPU3_RGn_GCR3_SECURE_ACCESS_LOCK_BMSK                    0x1
#define HWIO_IPA_0_GSI_TOP_XPU3_RGn_GCR3_SECURE_ACCESS_LOCK_SHFT                    0x0

#define HWIO_IPA_0_GSI_TOP_XPU3_RGn_CR0_ADDR(n)                              (IPA_0_GSI_TOP_XPU3_REG_BASE      + 0x00001010 + 0x80 * (n))
#define HWIO_IPA_0_GSI_TOP_XPU3_RGn_CR0_PHYS(n)                              (IPA_0_GSI_TOP_XPU3_REG_BASE_PHYS + 0x00001010 + 0x80 * (n))
#define HWIO_IPA_0_GSI_TOP_XPU3_RGn_CR0_OFFS(n)                              (IPA_0_GSI_TOP_XPU3_REG_BASE_OFFS + 0x00001010 + 0x80 * (n))
#define HWIO_IPA_0_GSI_TOP_XPU3_RGn_CR0_RMSK                                        0x1
#define HWIO_IPA_0_GSI_TOP_XPU3_RGn_CR0_MAXn                                         20
#define HWIO_IPA_0_GSI_TOP_XPU3_RGn_CR0_ATTR                                        0x3
#define HWIO_IPA_0_GSI_TOP_XPU3_RGn_CR0_INI(n)        \
        in_dword_masked(HWIO_IPA_0_GSI_TOP_XPU3_RGn_CR0_ADDR(n), HWIO_IPA_0_GSI_TOP_XPU3_RGn_CR0_RMSK)
#define HWIO_IPA_0_GSI_TOP_XPU3_RGn_CR0_INMI(n,mask)    \
        in_dword_masked(HWIO_IPA_0_GSI_TOP_XPU3_RGn_CR0_ADDR(n), mask)
#define HWIO_IPA_0_GSI_TOP_XPU3_RGn_CR0_OUTI(n,val)    \
        out_dword(HWIO_IPA_0_GSI_TOP_XPU3_RGn_CR0_ADDR(n),val)
#define HWIO_IPA_0_GSI_TOP_XPU3_RGn_CR0_OUTMI(n,mask,val) \
        out_dword_masked_ns(HWIO_IPA_0_GSI_TOP_XPU3_RGn_CR0_ADDR(n),mask,val,HWIO_IPA_0_GSI_TOP_XPU3_RGn_CR0_INI(n))
#define HWIO_IPA_0_GSI_TOP_XPU3_RGn_CR0_RGSCLRDEN_APPS_BMSK                         0x1
#define HWIO_IPA_0_GSI_TOP_XPU3_RGn_CR0_RGSCLRDEN_APPS_SHFT                         0x0

#define HWIO_IPA_0_GSI_TOP_XPU3_RGn_CR1_ADDR(n)                              (IPA_0_GSI_TOP_XPU3_REG_BASE      + 0x00001014 + 0x80 * (n))
#define HWIO_IPA_0_GSI_TOP_XPU3_RGn_CR1_PHYS(n)                              (IPA_0_GSI_TOP_XPU3_REG_BASE_PHYS + 0x00001014 + 0x80 * (n))
#define HWIO_IPA_0_GSI_TOP_XPU3_RGn_CR1_OFFS(n)                              (IPA_0_GSI_TOP_XPU3_REG_BASE_OFFS + 0x00001014 + 0x80 * (n))
#define HWIO_IPA_0_GSI_TOP_XPU3_RGn_CR1_RMSK                                        0x7
#define HWIO_IPA_0_GSI_TOP_XPU3_RGn_CR1_MAXn                                         20
#define HWIO_IPA_0_GSI_TOP_XPU3_RGn_CR1_ATTR                                        0x3
#define HWIO_IPA_0_GSI_TOP_XPU3_RGn_CR1_INI(n)        \
        in_dword_masked(HWIO_IPA_0_GSI_TOP_XPU3_RGn_CR1_ADDR(n), HWIO_IPA_0_GSI_TOP_XPU3_RGn_CR1_RMSK)
#define HWIO_IPA_0_GSI_TOP_XPU3_RGn_CR1_INMI(n,mask)    \
        in_dword_masked(HWIO_IPA_0_GSI_TOP_XPU3_RGn_CR1_ADDR(n), mask)
#define HWIO_IPA_0_GSI_TOP_XPU3_RGn_CR1_OUTI(n,val)    \
        out_dword(HWIO_IPA_0_GSI_TOP_XPU3_RGn_CR1_ADDR(n),val)
#define HWIO_IPA_0_GSI_TOP_XPU3_RGn_CR1_OUTMI(n,mask,val) \
        out_dword_masked_ns(HWIO_IPA_0_GSI_TOP_XPU3_RGn_CR1_ADDR(n),mask,val,HWIO_IPA_0_GSI_TOP_XPU3_RGn_CR1_INI(n))
#define HWIO_IPA_0_GSI_TOP_XPU3_RGn_CR1_RGCLRDEN_BMSK                               0x7
#define HWIO_IPA_0_GSI_TOP_XPU3_RGn_CR1_RGCLRDEN_SHFT                               0x0

#define HWIO_IPA_0_GSI_TOP_XPU3_RGn_CR2_ADDR(n)                              (IPA_0_GSI_TOP_XPU3_REG_BASE      + 0x00001018 + 0x80 * (n))
#define HWIO_IPA_0_GSI_TOP_XPU3_RGn_CR2_PHYS(n)                              (IPA_0_GSI_TOP_XPU3_REG_BASE_PHYS + 0x00001018 + 0x80 * (n))
#define HWIO_IPA_0_GSI_TOP_XPU3_RGn_CR2_OFFS(n)                              (IPA_0_GSI_TOP_XPU3_REG_BASE_OFFS + 0x00001018 + 0x80 * (n))
#define HWIO_IPA_0_GSI_TOP_XPU3_RGn_CR2_RMSK                                        0x1
#define HWIO_IPA_0_GSI_TOP_XPU3_RGn_CR2_MAXn                                         20
#define HWIO_IPA_0_GSI_TOP_XPU3_RGn_CR2_ATTR                                        0x3
#define HWIO_IPA_0_GSI_TOP_XPU3_RGn_CR2_INI(n)        \
        in_dword_masked(HWIO_IPA_0_GSI_TOP_XPU3_RGn_CR2_ADDR(n), HWIO_IPA_0_GSI_TOP_XPU3_RGn_CR2_RMSK)
#define HWIO_IPA_0_GSI_TOP_XPU3_RGn_CR2_INMI(n,mask)    \
        in_dword_masked(HWIO_IPA_0_GSI_TOP_XPU3_RGn_CR2_ADDR(n), mask)
#define HWIO_IPA_0_GSI_TOP_XPU3_RGn_CR2_OUTI(n,val)    \
        out_dword(HWIO_IPA_0_GSI_TOP_XPU3_RGn_CR2_ADDR(n),val)
#define HWIO_IPA_0_GSI_TOP_XPU3_RGn_CR2_OUTMI(n,mask,val) \
        out_dword_masked_ns(HWIO_IPA_0_GSI_TOP_XPU3_RGn_CR2_ADDR(n),mask,val,HWIO_IPA_0_GSI_TOP_XPU3_RGn_CR2_INI(n))
#define HWIO_IPA_0_GSI_TOP_XPU3_RGn_CR2_RGSCLWREN_APPS_BMSK                         0x1
#define HWIO_IPA_0_GSI_TOP_XPU3_RGn_CR2_RGSCLWREN_APPS_SHFT                         0x0

#define HWIO_IPA_0_GSI_TOP_XPU3_RGn_CR3_ADDR(n)                              (IPA_0_GSI_TOP_XPU3_REG_BASE      + 0x0000101c + 0x80 * (n))
#define HWIO_IPA_0_GSI_TOP_XPU3_RGn_CR3_PHYS(n)                              (IPA_0_GSI_TOP_XPU3_REG_BASE_PHYS + 0x0000101c + 0x80 * (n))
#define HWIO_IPA_0_GSI_TOP_XPU3_RGn_CR3_OFFS(n)                              (IPA_0_GSI_TOP_XPU3_REG_BASE_OFFS + 0x0000101c + 0x80 * (n))
#define HWIO_IPA_0_GSI_TOP_XPU3_RGn_CR3_RMSK                                        0x7
#define HWIO_IPA_0_GSI_TOP_XPU3_RGn_CR3_MAXn                                         20
#define HWIO_IPA_0_GSI_TOP_XPU3_RGn_CR3_ATTR                                        0x3
#define HWIO_IPA_0_GSI_TOP_XPU3_RGn_CR3_INI(n)        \
        in_dword_masked(HWIO_IPA_0_GSI_TOP_XPU3_RGn_CR3_ADDR(n), HWIO_IPA_0_GSI_TOP_XPU3_RGn_CR3_RMSK)
#define HWIO_IPA_0_GSI_TOP_XPU3_RGn_CR3_INMI(n,mask)    \
        in_dword_masked(HWIO_IPA_0_GSI_TOP_XPU3_RGn_CR3_ADDR(n), mask)
#define HWIO_IPA_0_GSI_TOP_XPU3_RGn_CR3_OUTI(n,val)    \
        out_dword(HWIO_IPA_0_GSI_TOP_XPU3_RGn_CR3_ADDR(n),val)
#define HWIO_IPA_0_GSI_TOP_XPU3_RGn_CR3_OUTMI(n,mask,val) \
        out_dword_masked_ns(HWIO_IPA_0_GSI_TOP_XPU3_RGn_CR3_ADDR(n),mask,val,HWIO_IPA_0_GSI_TOP_XPU3_RGn_CR3_INI(n))
#define HWIO_IPA_0_GSI_TOP_XPU3_RGn_CR3_RGCLWREN_BMSK                               0x7
#define HWIO_IPA_0_GSI_TOP_XPU3_RGn_CR3_RGCLWREN_SHFT                               0x0

#define HWIO_IPA_0_GSI_TOP_XPU3_RGn_RACR_ADDR(n)                             (IPA_0_GSI_TOP_XPU3_REG_BASE      + 0x00001040 + 0x80 * (n))
#define HWIO_IPA_0_GSI_TOP_XPU3_RGn_RACR_PHYS(n)                             (IPA_0_GSI_TOP_XPU3_REG_BASE_PHYS + 0x00001040 + 0x80 * (n))
#define HWIO_IPA_0_GSI_TOP_XPU3_RGn_RACR_OFFS(n)                             (IPA_0_GSI_TOP_XPU3_REG_BASE_OFFS + 0x00001040 + 0x80 * (n))
#define HWIO_IPA_0_GSI_TOP_XPU3_RGn_RACR_RMSK                                0xffffffff
#define HWIO_IPA_0_GSI_TOP_XPU3_RGn_RACR_MAXn                                        20
#define HWIO_IPA_0_GSI_TOP_XPU3_RGn_RACR_ATTR                                       0x3
#define HWIO_IPA_0_GSI_TOP_XPU3_RGn_RACR_INI(n)        \
        in_dword_masked(HWIO_IPA_0_GSI_TOP_XPU3_RGn_RACR_ADDR(n), HWIO_IPA_0_GSI_TOP_XPU3_RGn_RACR_RMSK)
#define HWIO_IPA_0_GSI_TOP_XPU3_RGn_RACR_INMI(n,mask)    \
        in_dword_masked(HWIO_IPA_0_GSI_TOP_XPU3_RGn_RACR_ADDR(n), mask)
#define HWIO_IPA_0_GSI_TOP_XPU3_RGn_RACR_OUTI(n,val)    \
        out_dword(HWIO_IPA_0_GSI_TOP_XPU3_RGn_RACR_ADDR(n),val)
#define HWIO_IPA_0_GSI_TOP_XPU3_RGn_RACR_OUTMI(n,mask,val) \
        out_dword_masked_ns(HWIO_IPA_0_GSI_TOP_XPU3_RGn_RACR_ADDR(n),mask,val,HWIO_IPA_0_GSI_TOP_XPU3_RGn_RACR_INI(n))
#define HWIO_IPA_0_GSI_TOP_XPU3_RGn_RACR_RE_BMSK                             0xffffffff
#define HWIO_IPA_0_GSI_TOP_XPU3_RGn_RACR_RE_SHFT                                    0x0

#define HWIO_IPA_0_GSI_TOP_XPU3_RGn_WACR_ADDR(n)                             (IPA_0_GSI_TOP_XPU3_REG_BASE      + 0x00001060 + 0x80 * (n))
#define HWIO_IPA_0_GSI_TOP_XPU3_RGn_WACR_PHYS(n)                             (IPA_0_GSI_TOP_XPU3_REG_BASE_PHYS + 0x00001060 + 0x80 * (n))
#define HWIO_IPA_0_GSI_TOP_XPU3_RGn_WACR_OFFS(n)                             (IPA_0_GSI_TOP_XPU3_REG_BASE_OFFS + 0x00001060 + 0x80 * (n))
#define HWIO_IPA_0_GSI_TOP_XPU3_RGn_WACR_RMSK                                0xffffffff
#define HWIO_IPA_0_GSI_TOP_XPU3_RGn_WACR_MAXn                                        20
#define HWIO_IPA_0_GSI_TOP_XPU3_RGn_WACR_ATTR                                       0x3
#define HWIO_IPA_0_GSI_TOP_XPU3_RGn_WACR_INI(n)        \
        in_dword_masked(HWIO_IPA_0_GSI_TOP_XPU3_RGn_WACR_ADDR(n), HWIO_IPA_0_GSI_TOP_XPU3_RGn_WACR_RMSK)
#define HWIO_IPA_0_GSI_TOP_XPU3_RGn_WACR_INMI(n,mask)    \
        in_dword_masked(HWIO_IPA_0_GSI_TOP_XPU3_RGn_WACR_ADDR(n), mask)
#define HWIO_IPA_0_GSI_TOP_XPU3_RGn_WACR_OUTI(n,val)    \
        out_dword(HWIO_IPA_0_GSI_TOP_XPU3_RGn_WACR_ADDR(n),val)
#define HWIO_IPA_0_GSI_TOP_XPU3_RGn_WACR_OUTMI(n,mask,val) \
        out_dword_masked_ns(HWIO_IPA_0_GSI_TOP_XPU3_RGn_WACR_ADDR(n),mask,val,HWIO_IPA_0_GSI_TOP_XPU3_RGn_WACR_INI(n))
#define HWIO_IPA_0_GSI_TOP_XPU3_RGn_WACR_WE_BMSK                             0xffffffff
#define HWIO_IPA_0_GSI_TOP_XPU3_RGn_WACR_WE_SHFT                                    0x0


#endif /* __GSI_HWIO_H__ */
