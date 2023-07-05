
/*
 * Copyright (c) 2021 Qualcomm Innovation Center, Inc. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for
 * any purpose with or without fee is hereby granted, provided that the
 * above copyright notice and this permission notice appear in all
 * copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL
 * WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE
 * AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR
 * PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */


#ifndef __WCSS_SEQ_HWIOREG_UMAC_H__
#define __WCSS_SEQ_HWIOREG_UMAC_H__




#include "seq_hwio.h"
#include "wcss_seq_hwiobase.h"
#ifdef SCALE_INCLUDES
#include "HALhwio.h"
#else
#include "msmhwio.h"
#endif

#define REO_REG_REG_BASE                                                                                   (UMAC_BASE            + 0x00038000)
#define REO_REG_REG_BASE_SIZE                                                                              0x4000
#define REO_REG_REG_BASE_USED                                                                              0x309c
#define REO_REG_REG_BASE_PHYS                                                                              (UMAC_BASE_PHYS + 0x00038000)
#define REO_REG_REG_BASE_OFFS                                                                              0x00038000
#define MAC_TCL_REG_REG_BASE                                                                                (UMAC_BASE            + 0x00044000)
#define WBM_REG_REG_BASE                                                                                        (UMAC_BASE            + 0x00034000)
#define HWIO_TCL_R0_DSCP_TID_MAP_n_RMSK                                                                     0xffffffff

#define HWIO_WBM_R0_IDLE_LIST_CONTROL_SCATTER_BUFFER_SIZE_BMSK                                                       0x7fc
#define HWIO_WBM_R0_IDLE_LIST_CONTROL_SCATTER_BUFFER_SIZE_SHFT                                                           2

#define HWIO_WBM_R0_IDLE_LIST_CONTROL_LINK_DESC_IDLE_LIST_MODE_BMSK                                                    0x2
#define HWIO_WBM_R0_IDLE_LIST_CONTROL_LINK_DESC_IDLE_LIST_MODE_SHFT                                                      1

#define HWIO_TCL_R0_CONS_RING_CMN_CTRL_REG_DSCP_TID_MAP_PROGRAM_EN_BMSK                                       0x800000
#define HWIO_TCL_R0_CONS_RING_CMN_CTRL_REG_DSCP_TID_MAP_PROGRAM_EN_SHFT                                             23
#define HWIO_WBM_R0_IDLE_LIST_SIZE_SCATTER_RING_SIZE_OF_IDLE_LINK_DESC_LIST_BMSK                                0xffff0000
#define HWIO_WBM_R0_IDLE_LIST_SIZE_SCATTER_RING_SIZE_OF_IDLE_LINK_DESC_LIST_SHFT                                        16
#define HWIO_WBM_R0_IDLE_LIST_SIZE_SCATTER_RING_SIZE_OF_IDLE_BUF_LIST_BMSK                                          0xffff
#define HWIO_WBM_R0_IDLE_LIST_SIZE_SCATTER_RING_SIZE_OF_IDLE_BUF_LIST_SHFT                                               0

#define HWIO_TCL_R0_SW2TCL1_RING_MSI1_BASE_MSB_ADDR_BMSK                                                          0xff
#define HWIO_TCL_R0_SW2TCL1_RING_MSI1_BASE_MSB_ADDR_SHFT                                                             0

#define HWIO_WBM_R0_IDLE_LIST_SIZE_ADDR(x)                                                                      ((x) + 0xc4)
#define HWIO_TCL_R0_SW2TCL1_RING_MSI1_BASE_MSB_ADDR_SHFT                                                             0

#define HWIO_TCL_R0_SW2TCL1_RING_MSI1_BASE_MSB_MSI1_ENABLE_BMSK                                                  0x100
#define HWIO_TCL_R0_SW2TCL1_RING_MSI1_BASE_MSB_MSI1_ENABLE_SHFT                                                      8
#define HWIO_WBM_R0_SCATTERED_LINK_DESC_LIST_BASE_LSB_ADDR(x)                                                   ((x) + 0xd0)
#define HWIO_TCL_R0_SW_CONFIG_BANK_n_ADDR(base, n)                                                           ((base) + 0X88 + (0x4*(n)))
#define HWIO_TCL_R0_SW_CONFIG_BANK_n_PHYS(base, n)                                                           ((base) + 0X88 + (0x4*(n)))
#define HWIO_TCL_R0_SW_CONFIG_BANK_n_OFFS(n)                                                                (0X88 + (0x4*(n)))
#define HWIO_TCL_R0_SW_CONFIG_BANK_n_RMSK                                                                      0x7ffff
#define HWIO_TCL_R0_SW_CONFIG_BANK_n_MAXn                                                                            7
#define HWIO_TCL_R0_SW_CONFIG_BANK_n_POR                                                                    0x00000038
#define HWIO_TCL_R0_SW_CONFIG_BANK_n_POR_RMSK                                                               0xffffffff
#define HWIO_TCL_R0_SW_CONFIG_BANK_n_ATTR                                                                                0x3
#define HWIO_TCL_R0_SW_CONFIG_BANK_n_INI(base, n)                \
                in_dword_masked(HWIO_TCL_R0_SW_CONFIG_BANK_n_ADDR(base, n), HWIO_TCL_R0_SW_CONFIG_BANK_n_RMSK)
#define HWIO_TCL_R0_SW_CONFIG_BANK_n_INMI(base, n, mask)        \
                in_dword_masked(HWIO_TCL_R0_SW_CONFIG_BANK_n_ADDR(base, n), mask)
#define HWIO_TCL_R0_SW_CONFIG_BANK_n_OUTI(base, n, val)        \
                out_dword(HWIO_TCL_R0_SW_CONFIG_BANK_n_ADDR(base, n), val)
#define HWIO_TCL_R0_SW_CONFIG_BANK_n_OUTMI(base, n, mask, val) \
                out_dword_masked_ns(HWIO_TCL_R0_SW_CONFIG_BANK_n_ADDR(base, n), mask, val, HWIO_TCL_R0_SW_CONFIG_BANK_n_INI(base, n))
#define HWIO_TCL_R0_SW_CONFIG_BANK_n_MCAST_PACKET_CTRL_BMSK                                                    0x60000
#define HWIO_TCL_R0_SW_CONFIG_BANK_n_MCAST_PACKET_CTRL_SHFT                                                         17
#define HWIO_TCL_R0_SW_CONFIG_BANK_n_PMAC_ID_BMSK                                                              0x18000
#define HWIO_TCL_R0_SW_CONFIG_BANK_n_PMAC_ID_SHFT                                                                   15
#define HWIO_TCL_R0_SW_CONFIG_BANK_n_VDEV_ID_CHECK_EN_BMSK                                                      0x4000
#define HWIO_TCL_R0_SW_CONFIG_BANK_n_VDEV_ID_CHECK_EN_SHFT                                                          14
#define HWIO_TCL_R0_SW_CONFIG_BANK_n_MESH_ENABLE_BMSK                                                           0x3000
#define HWIO_TCL_R0_SW_CONFIG_BANK_n_MESH_ENABLE_SHFT                                                               12
#define HWIO_TCL_R0_SW_CONFIG_BANK_n_ADDRY_EN_BMSK                                                               0x800
#define HWIO_TCL_R0_SW_CONFIG_BANK_n_ADDRY_EN_SHFT                                                                  11
#define HWIO_TCL_R0_SW_CONFIG_BANK_n_ADDRX_EN_BMSK                                                               0x400
#define HWIO_TCL_R0_SW_CONFIG_BANK_n_ADDRX_EN_SHFT                                                                  10
#define HWIO_TCL_R0_SW_CONFIG_BANK_n_INDEX_LOOKUP_ENABLE_BMSK                                                    0x200
#define HWIO_TCL_R0_SW_CONFIG_BANK_n_INDEX_LOOKUP_ENABLE_SHFT                                                        9
#define HWIO_TCL_R0_SW_CONFIG_BANK_n_LINK_META_SWAP_BMSK                                                         0x100
#define HWIO_TCL_R0_SW_CONFIG_BANK_n_LINK_META_SWAP_SHFT                                                             8
#define HWIO_TCL_R0_SW_CONFIG_BANK_n_SRC_BUFFER_SWAP_BMSK                                                         0x80
#define HWIO_TCL_R0_SW_CONFIG_BANK_n_SRC_BUFFER_SWAP_SHFT                                                            7
#define HWIO_TCL_R0_SW_CONFIG_BANK_n_ENCRYPT_TYPE_BMSK                                                            0x78
#define HWIO_TCL_R0_SW_CONFIG_BANK_n_ENCRYPT_TYPE_SHFT                                                               3
#define HWIO_TCL_R0_SW_CONFIG_BANK_n_ENCAP_TYPE_BMSK                                                               0x6
#define HWIO_TCL_R0_SW_CONFIG_BANK_n_ENCAP_TYPE_SHFT                                                                 1
#define HWIO_TCL_R0_SW_CONFIG_BANK_n_EPD_BMSK                                                                      0x1
#define HWIO_TCL_R0_SW_CONFIG_BANK_n_EPD_SHFT                                                                        0
#define HWIO_TCL_R0_CONS_RING_CMN_CTRL_REG_DSCP_TID_MAP_PROGRAM_EN_SHFT                                             23
#define HWIO_WBM_R0_SCATTERED_LINK_DESC_LIST_BASE_MSB_ADDRESS_MATCH_TAG_BMSK                                    0xffffff00
#define HWIO_WBM_R0_SCATTERED_LINK_DESC_LIST_BASE_MSB_ADDRESS_MATCH_TAG_SHFT                                             8
#define HWIO_WBM_R0_SCATTERED_LINK_DESC_LIST_BASE_MSB_BASE_ADDRESS_39_32_BMSK                                         0xff
#define HWIO_WBM_R0_SCATTERED_LINK_DESC_LIST_BASE_MSB_BASE_ADDRESS_39_32_SHFT                                            0

#define HWIO_WBM_R0_SCATTERED_LINK_DESC_PTR_HEAD_INFO_IX0_ADDR(x)                                               ((x) + 0xe0)
#define HWIO_WBM_R0_SCATTERED_LINK_DESC_LIST_BASE_MSB_ADDR(x)                                                   ((x) + 0xd4)

#define HWIO_TCL_R0_SW2TCL1_RING_ID_ENTRY_SIZE_BMSK                                                               0xff
#define HWIO_TCL_R0_SW2TCL1_RING_ID_ENTRY_SIZE_SHFT                                                                  0

#define HWIO_TCL_R0_SW2TCL1_RING_CONSUMER_INT_SETUP_IX0_INTERRUPT_TIMER_THRESHOLD_BMSK                      0xffff0000
#define HWIO_TCL_R0_SW2TCL1_RING_CONSUMER_INT_SETUP_IX0_INTERRUPT_TIMER_THRESHOLD_SHFT                              16
#define HWIO_TCL_R0_SW2TCL1_RING_CONSUMER_INT_SETUP_IX0_BATCH_COUNTER_THRESHOLD_BMSK                            0x7fff
#define HWIO_TCL_R0_SW2TCL1_RING_CONSUMER_INT_SETUP_IX0_BATCH_COUNTER_THRESHOLD_SHFT                                 0

#define HWIO_WBM_R0_SCATTERED_LINK_DESC_PTR_TAIL_INFO_IX0_ADDR(x)                                               ((x) + 0xf0)
#define HWIO_TCL_R0_SW2TCL1_RING_MISC_RING_ID_DISABLE_BMSK                                                         0x1
#define HWIO_TCL_R0_SW2TCL1_RING_MISC_RING_ID_DISABLE_SHFT                                                           0
#define HWIO_WBM_R0_SCATTERED_LINK_DESC_PTR_TAIL_INFO_IX1_BUFFER_ADDRESS_39_32_BMSK                                   0xff
#define HWIO_WBM_R0_SCATTERED_LINK_DESC_PTR_TAIL_INFO_IX1_BUFFER_ADDRESS_39_32_SHFT                                      0

#define HWIO_WBM_R0_SCATTERED_LINK_DESC_PTR_TAIL_INFO_IX1_TAIL_POINTER_OFFSET_BMSK                                0x1fff00
#define HWIO_WBM_R0_SCATTERED_LINK_DESC_PTR_TAIL_INFO_IX1_TAIL_POINTER_OFFSET_SHFT                                       8

#define HWIO_TCL_R0_SW2TCL1_RING_MISC_DATA_TLV_SWAP_BIT_BMSK                                                      0x20
#define HWIO_TCL_R0_SW2TCL1_RING_MISC_DATA_TLV_SWAP_BIT_SHFT                                                         5

#define HWIO_WBM_R0_WBM_IDLE_LINK_RING_MISC_RING_ID_DISABLE_BMSK                                                       0x1
#define HWIO_WBM_R0_WBM_IDLE_LINK_RING_MISC_RING_ID_DISABLE_SHFT                                                         0
#define HWIO_WBM_R0_SCATTERED_LINK_DESC_PTR_HP_ADDR(x)                                                          ((x) + 0xfc)
#define HWIO_TCL_R0_SW2TCL1_RING_CONSUMER_INT_SETUP_IX1_LOW_THRESHOLD_BMSK                                      0xffff
#define HWIO_TCL_R0_SW2TCL1_RING_CONSUMER_INT_SETUP_IX1_LOW_THRESHOLD_SHFT                                           0

#define HWIO_WBM_R0_SCATTERED_LINK_DESC_PTR_TAIL_INFO_IX0_ADDR(x)                                               ((x) + 0xf0)
#define HWIO_WBM_R0_SCATTERED_LINK_DESC_PTR_TAIL_INFO_IX1_ADDR(x)                                               ((x) + 0xf4)
#define HWIO_WBM_R0_SCATTERED_LINK_DESC_PTR_HEAD_INFO_IX1_HEAD_POINTER_OFFSET_BMSK                                0x1fff00
#define HWIO_WBM_R0_SCATTERED_LINK_DESC_PTR_HEAD_INFO_IX1_HEAD_POINTER_OFFSET_SHFT                                       8
#define HWIO_WBM_R0_SCATTERED_LINK_DESC_PTR_HEAD_INFO_IX1_HEAD_POINTER_OFFSET_SHFT                                       8
#define HWIO_WBM_R0_SCATTERED_LINK_DESC_PTR_HEAD_INFO_IX1_ADDR(x)                                               ((x) + 0xe4)
#define HWIO_WBM_R0_SCATTERED_LINK_DESC_PTR_HEAD_INFO_IX1_BUFFER_ADDRESS_39_32_BMSK                                   0xff
#define HWIO_WBM_R0_SCATTERED_LINK_DESC_PTR_HEAD_INFO_IX1_BUFFER_ADDRESS_39_32_SHFT                                      0

#define HWIO_WBM_R0_WBM_IDLE_LINK_RING_MISC_ADDR(x)                                                             ((x) + 0xb6c)

#define HWIO_TCL_R0_SW2TCL1_RING_MISC_HOST_FW_SWAP_BIT_BMSK                                                       0x10
#define HWIO_TCL_R0_SW2TCL1_RING_MISC_HOST_FW_SWAP_BIT_SHFT                                                          4
#define HWIO_REO_R0_DESTINATION_RING_CTRL_IX_0_DEST_RING_MAPPING_7_BMSK                                    0xf0000000
#define HWIO_REO_R0_DESTINATION_RING_CTRL_IX_0_DEST_RING_MAPPING_7_SHFT                                            28
#define HWIO_REO_R0_DESTINATION_RING_CTRL_IX_0_DEST_RING_MAPPING_6_BMSK                                     0xf000000
#define HWIO_REO_R0_DESTINATION_RING_CTRL_IX_0_DEST_RING_MAPPING_6_SHFT                                            24
#define HWIO_REO_R0_DESTINATION_RING_CTRL_IX_0_DEST_RING_MAPPING_5_BMSK                                      0xf00000
#define HWIO_REO_R0_DESTINATION_RING_CTRL_IX_0_DEST_RING_MAPPING_5_SHFT                                            20
#define HWIO_REO_R0_DESTINATION_RING_CTRL_IX_0_DEST_RING_MAPPING_4_BMSK                                       0xf0000
#define HWIO_REO_R0_DESTINATION_RING_CTRL_IX_0_DEST_RING_MAPPING_4_SHFT                                            16
#define HWIO_REO_R0_DESTINATION_RING_CTRL_IX_0_DEST_RING_MAPPING_3_BMSK                                        0xf000
#define HWIO_REO_R0_DESTINATION_RING_CTRL_IX_0_DEST_RING_MAPPING_3_SHFT                                            12
#define HWIO_REO_R0_DESTINATION_RING_CTRL_IX_0_DEST_RING_MAPPING_2_BMSK                                         0xf00
#define HWIO_REO_R0_DESTINATION_RING_CTRL_IX_0_DEST_RING_MAPPING_2_SHFT                                             8
#define HWIO_REO_R0_DESTINATION_RING_CTRL_IX_0_DEST_RING_MAPPING_1_BMSK                                          0xf0
#define HWIO_REO_R0_DESTINATION_RING_CTRL_IX_0_DEST_RING_MAPPING_1_SHFT                                             4
#define HWIO_REO_R0_DESTINATION_RING_CTRL_IX_0_DEST_RING_MAPPING_0_BMSK                                           0xf
#define HWIO_REO_R0_DESTINATION_RING_CTRL_IX_0_DEST_RING_MAPPING_0_SHFT                                             0

#define HWIO_REO_R0_DESTINATION_RING_CTRL_IX_2_DEST_RING_MAPPING_23_BMSK                                   0xf0000000
#define HWIO_REO_R0_DESTINATION_RING_CTRL_IX_2_DEST_RING_MAPPING_23_SHFT                                           28
#define HWIO_REO_R0_DESTINATION_RING_CTRL_IX_2_DEST_RING_MAPPING_22_BMSK                                    0xf000000
#define HWIO_REO_R0_DESTINATION_RING_CTRL_IX_2_DEST_RING_MAPPING_22_SHFT                                           24
#define HWIO_REO_R0_DESTINATION_RING_CTRL_IX_2_DEST_RING_MAPPING_21_BMSK                                     0xf00000
#define HWIO_REO_R0_DESTINATION_RING_CTRL_IX_2_DEST_RING_MAPPING_21_SHFT                                           20
#define HWIO_REO_R0_DESTINATION_RING_CTRL_IX_2_DEST_RING_MAPPING_20_BMSK                                      0xf0000
#define HWIO_REO_R0_DESTINATION_RING_CTRL_IX_2_DEST_RING_MAPPING_20_SHFT                                           16
#define HWIO_REO_R0_DESTINATION_RING_CTRL_IX_2_DEST_RING_MAPPING_19_BMSK                                       0xf000
#define HWIO_REO_R0_DESTINATION_RING_CTRL_IX_2_DEST_RING_MAPPING_19_SHFT                                           12
#define HWIO_REO_R0_DESTINATION_RING_CTRL_IX_2_DEST_RING_MAPPING_18_BMSK                                        0xf00
#define HWIO_REO_R0_DESTINATION_RING_CTRL_IX_2_DEST_RING_MAPPING_18_SHFT                                            8
#define HWIO_REO_R0_DESTINATION_RING_CTRL_IX_2_DEST_RING_MAPPING_17_BMSK                                         0xf0
#define HWIO_REO_R0_DESTINATION_RING_CTRL_IX_2_DEST_RING_MAPPING_17_SHFT                                            4
#define HWIO_REO_R0_DESTINATION_RING_CTRL_IX_2_DEST_RING_MAPPING_16_BMSK                                          0xf
#define HWIO_REO_R0_DESTINATION_RING_CTRL_IX_2_DEST_RING_MAPPING_16_SHFT                                            0

#define HWIO_TCL_R0_SW2TCL1_RING_MISC_MSI_SWAP_BIT_BMSK                                                            0x8
#define HWIO_TCL_R0_SW2TCL1_RING_MISC_MSI_SWAP_BIT_SHFT                                                              3

#define HWIO_TCL_R0_SW2TCL1_RING_MISC_LOOPCNT_DISABLE_BMSK                                                         0x2
#define HWIO_TCL_R0_SW2TCL1_RING_MISC_LOOPCNT_DISABLE_SHFT                                                           1

#define HWIO_REO_R0_SW_COOKIE_CFG1_SW_COOKIE_CONVERT_GLOBAL_ENABLE_BMSK                                      0x100000
#define HWIO_REO_R0_SW_COOKIE_CFG1_SW_COOKIE_CONVERT_GLOBAL_ENABLE_SHFT                                            20
#define HWIO_REO_R0_SW_COOKIE_CFG1_SW_COOKIE_CONVERT_ENABLE_BMSK                                              0x80000
#define HWIO_REO_R0_SW_COOKIE_CFG1_SW_COOKIE_CONVERT_ENABLE_SHFT                                                   19
#define HWIO_REO_R0_SW_COOKIE_CFG1_PAGE_ALIGNMENT_BMSK                                                        0x40000
#define HWIO_REO_R0_SW_COOKIE_CFG1_PAGE_ALIGNMENT_SHFT                                                             18
#define HWIO_REO_R0_SW_COOKIE_CFG1_COOKIE_OFFSET_MSB_BMSK                                                     0x3e000
#define HWIO_REO_R0_SW_COOKIE_CFG1_COOKIE_OFFSET_MSB_SHFT                                                          13
#define HWIO_REO_R0_SW_COOKIE_CFG1_COOKIE_PAGE_MSB_BMSK                                                        0x1f00
#define HWIO_REO_R0_SW_COOKIE_CFG1_COOKIE_PAGE_MSB_SHFT                                                             8
#define HWIO_REO_R0_SW_COOKIE_CFG1_CMEM_LUT_BASE_ADDR_39_32_BMSK                                                 0xff
#define HWIO_REO_R0_SW_COOKIE_CFG1_CMEM_LUT_BASE_ADDR_39_32_SHFT                                                    0
#define HWIO_WBM_R0_SW_COOKIE_CFG1_PAGE_ALIGNMENT_BMSK                                                             0x40000
#define HWIO_WBM_R0_SW_COOKIE_CFG1_PAGE_ALIGNMENT_SHFT                                                                  18
#define HWIO_WBM_R0_SW_COOKIE_CFG1_COOKIE_OFFSET_MSB_BMSK                                                          0x3e000
#define HWIO_WBM_R0_SW_COOKIE_CFG1_COOKIE_OFFSET_MSB_SHFT                                                               13
#define HWIO_WBM_R0_SW_COOKIE_CFG1_CMEM_LUT_BASE_ADDR_39_32_BMSK                                                      0xff
#define HWIO_WBM_R0_SW_COOKIE_CFG1_CMEM_LUT_BASE_ADDR_39_32_SHFT                                                         0
#define HWIO_WBM_R0_SW_COOKIE_CFG1_COOKIE_PAGE_MSB_BMSK                                                             0x1f00
#define HWIO_WBM_R0_SW_COOKIE_CFG1_COOKIE_PAGE_MSB_SHFT                                                                  8

#define HWIO_WBM_R0_SW_COOKIE_CONVERT_CFG_WBM_COOKIE_CONV_GLOBAL_ENABLE_BMSK                                         0x100
#define HWIO_WBM_R0_SW_COOKIE_CONVERT_CFG_WBM_COOKIE_CONV_GLOBAL_ENABLE_SHFT                                             8
#define HWIO_WBM_R0_SW_COOKIE_CONVERT_CFG_WBM2SW6_COOKIE_CONVERSION_EN_BMSK                                           0x80
#define HWIO_WBM_R0_SW_COOKIE_CONVERT_CFG_WBM2SW6_COOKIE_CONVERSION_EN_SHFT                                              7
#define HWIO_WBM_R0_SW_COOKIE_CONVERT_CFG_WBM2SW5_COOKIE_CONVERSION_EN_BMSK                                           0x40
#define HWIO_WBM_R0_SW_COOKIE_CONVERT_CFG_WBM2SW5_COOKIE_CONVERSION_EN_SHFT                                              6
#define HWIO_WBM_R0_SW_COOKIE_CONVERT_CFG_WBM2SW4_COOKIE_CONVERSION_EN_BMSK                                           0x20
#define HWIO_WBM_R0_SW_COOKIE_CONVERT_CFG_WBM2SW4_COOKIE_CONVERSION_EN_SHFT                                              5
#define HWIO_WBM_R0_SW_COOKIE_CONVERT_CFG_WBM2SW3_COOKIE_CONVERSION_EN_BMSK                                           0x10
#define HWIO_WBM_R0_SW_COOKIE_CONVERT_CFG_WBM2SW3_COOKIE_CONVERSION_EN_SHFT                                              4
#define HWIO_WBM_R0_SW_COOKIE_CONVERT_CFG_WBM2SW2_COOKIE_CONVERSION_EN_BMSK                                            0x8
#define HWIO_WBM_R0_SW_COOKIE_CONVERT_CFG_WBM2SW2_COOKIE_CONVERSION_EN_SHFT                                              3
#define HWIO_WBM_R0_SW_COOKIE_CONVERT_CFG_WBM2SW1_COOKIE_CONVERSION_EN_BMSK                                            0x4
#define HWIO_WBM_R0_SW_COOKIE_CONVERT_CFG_WBM2SW1_COOKIE_CONVERSION_EN_SHFT                                              2
#define HWIO_WBM_R0_SW_COOKIE_CONVERT_CFG_WBM2SW0_COOKIE_CONVERSION_EN_BMSK                                            0x2
#define HWIO_WBM_R0_SW_COOKIE_CONVERT_CFG_WBM2SW0_COOKIE_CONVERSION_EN_SHFT                                              1
#define HWIO_WBM_R0_SW_COOKIE_CONVERT_CFG_WBM2FW_COOKIE_CONVERSION_EN_BMSK                                             0x1
#define HWIO_WBM_R0_SW_COOKIE_CONVERT_CFG_WBM2FW_COOKIE_CONVERSION_EN_SHFT                                               0


#define HWIO_TCL_R0_SW2TCL1_RING_BASE_LSB_ADDR(x)                                                           ((x) + 0x878)
#define HWIO_REO_R0_REO2SW1_RING_MSI1_BASE_MSB_ADDR(x)                                                     ((x) + 0x3f0)
#define HWIO_REO_R0_REO2SW1_RING_HP_ADDR_MSB_ADDR(x)                                                       ((x) + 0x3bc)
#define HWIO_WBM_R0_SW_COOKIE_CFG0_ADDR(x)                                                                      ((x) + 0x34)
#define HWIO_WBM_R0_SW_COOKIE_CONVERT_CFG_ADDR(x)                                                               ((x) + 0x74)
#define HWIO_REO_R0_REO2SW1_RING_MSI1_DATA_ADDR(x)                                                         ((x) + 0x3f4)
#define HWIO_REO_R0_REO2SW1_RING_BASE_MSB_ADDR(x)                                                          ((x) + 0x3a8)
#define HWIO_REO_R0_REO2SW1_RING_PRODUCER_INT_SETUP_ADDR(x)                                                ((x) + 0x3c8)
#define HWIO_TCL_R0_SW2TCL1_RING_MISC_ADDR(x)                                                               ((x) + 0x888)
#define HWIO_TCL_R0_SW2TCL1_RING_TP_ADDR_LSB_ADDR(x)                                                        ((x) + 0x894)
#define HWIO_TCL_R0_SW2TCL1_RING_TP_ADDR_MSB_ADDR(x)                                                        ((x) + 0x898)
#define HWIO_TCL_R0_SW2TCL1_RING_MSI1_BASE_LSB_ADDR(x)                                                      ((x) + 0x8c0)
#define HWIO_TCL_R0_SW2TCL1_RING_MSI1_BASE_MSB_ADDR(x)                                                      ((x) + 0x8c4)
#define HWIO_TCL_R0_SW2TCL1_RING_MSI1_DATA_ADDR(x)                                                          ((x) + 0x8c8)
#define HWIO_TCL_R0_SW2TCL1_RING_BASE_MSB_ADDR(x)                                                           ((x) + 0x87c)
#define HWIO_TCL_R0_SW2TCL1_RING_CONSUMER_INT_SETUP_IX0_ADDR(x)                                             ((x) + 0x8a8)
#define HWIO_TCL_R0_SW2TCL1_RING_CONSUMER_INT_SETUP_IX1_ADDR(x)                                             ((x) + 0x8ac)

#define HWIO_TCL_R2_SW2TCL1_RING_HP_ADDR(x)                                                                 ((x) + 0x2000)
#define HWIO_TCL_R0_SW2TCL1_RING_ID_ADDR(x)                                                                 ((x) + 0x880)
#define HWIO_TCL_R2_SW2TCL1_RING_TP_ADDR(x)                                                                 ((x) + 0x2004)
#define HWIO_WBM_R0_SW_COOKIE_CFG1_ADDR(x)                                                                      ((x) + 0x38)
#define HWIO_REO_R0_REO2SW1_RING_HP_ADDR_LSB_ADDR(x)                                                       ((x) + 0x3b8)
#define HWIO_REO_R0_REO2SW1_RING_ID_ADDR(x)                                                                ((x) + 0x3ac)

#define HWIO_REO_R0_REO2SW1_RING_BASE_LSB_ADDR(x)                                                          ((x) + 0x3a4)
#define HWIO_REO_R0_REO2SW1_RING_MSI1_BASE_LSB_ADDR(x)                                                     ((x) + 0x3ec)

#define HWIO_REO_R0_REO2SW1_RING_MISC_ADDR(x)                                                              ((x) + 0x3b4)
#define HWIO_REO_R2_REO2SW1_RING_TP_ADDR(x)                                                                ((x) + 0x303c)
#define HWIO_REO_R2_REO2SW1_RING_HP_ADDR(x)                                                                ((x) + 0x3038)
#define HWIO_REO_R0_SW_COOKIE_CFG1_ADDR(x)                                                                 ((x) + 0x54)
#define HWIO_REO_R0_SW_COOKIE_CFG0_ADDR(x)                                                                 ((x) + 0x50)
#define HWIO_WBM_R0_MISC_CONTROL_ADDR(x)                                                                        ((x) + 0x60)
#define HWIO_TCL_R0_SW2TCL1_RING_ID_ENTRY_SIZE_SHFT                                                                  0
#define HWIO_WBM_R0_SCATTERED_LINK_DESC_PTR_HEAD_INFO_IX0_ADDR(x)                                               ((x) + 0xe0)
#define HWIO_TCL_R0_SW2TCL1_RING_BASE_MSB_RING_SIZE_BMSK                                                     0xfffff00
#define HWIO_TCL_R0_SW2TCL1_RING_BASE_MSB_RING_SIZE_SHFT                                                             8
#define HWIO_TCL_R0_SW2TCL1_RING_BASE_MSB_RING_BASE_ADDR_MSB_BMSK                                                 0xff
#define HWIO_TCL_R0_SW2TCL1_RING_BASE_MSB_RING_BASE_ADDR_MSB_SHFT                                                    0
#define HWIO_WBM_R0_IDLE_LIST_CONTROL_ADDR(x)                                                                   ((x) + 0xc0)
#define HWIO_TCL_R0_DSCP_TID_MAP_n_ADDR(base, n)                                                             ((base) + 0X1F8 + (0x4*(n)))
#define HWIO_REO_R0_GENERAL_ENABLE_ADDR(x)                                                                 ((x) + 0x0)
#define HWIO_REO_R0_GENERAL_ENABLE_PHYS(x)                                                                 ((x) + 0x0)
#define HWIO_REO_R0_AGING_THRESHOLD_IX_0_ADDR(x)                                                           ((x) + 0x994)
#define HWIO_REO_R0_AGING_THRESHOLD_IX_1_ADDR(x)                                                           ((x) + 0x998)
#define HWIO_TCL_R0_SW_CONFIG_BANK_n_ADDR(base, n)                                                           ((base) + 0X88 + (0x4*(n)))
#define HWIO_REO_R0_AGING_THRESHOLD_IX_2_ADDR(x)                                                           ((x) + 0x99c)
#define HWIO_REO_R0_AGING_THRESHOLD_IX_3_ADDR(x)                                                           ((x) + 0x9a0)
#define HWIO_REO_R0_DESTINATION_RING_CTRL_IX_2_ADDR(x)                                                     ((x) + 0xc)
#define HWIO_TCL_R0_CONS_RING_CMN_CTRL_REG_ADDR(x)                                                          ((x) + 0x20)
#define HWIO_REO_R0_DESTINATION_RING_CTRL_IX_3_ADDR(x)                                                     ((x) + 0x10)
#define HWIO_TCL_R0_PCP_TID_MAP_ADDR(x)                                                                     ((x) + 0x678)
#define HWIO_TCL_R0_TID_MAP_PRTY_ADDR(x)                                                                    ((x) + 0x6a0)
#define HWIO_REO_R0_GENERAL_ENABLE_AGING_FLUSH_ENABLE_BMSK                                                        0x8
#define HWIO_REO_R0_GENERAL_ENABLE_AGING_LIST_ENABLE_SHFT                                                           2
#define HWIO_REO_R0_GENERAL_ENABLE_AGING_LIST_ENABLE_BMSK                                                         0x4
#define HWIO_REO_R0_GENERAL_ENABLE_AGING_FLUSH_ENABLE_SHFT                                                          3
#define HWIO_REO_R0_MISC_CTL_ADDR(x)                                                                       ((x) + 0xa08)
#define HWIO_REO_R0_MISC_CTL_BAR_DEST_RING_BMSK                                                             0x1e00000
#define HWIO_REO_R0_MISC_CTL_BAR_DEST_RING_SHFT                                                                    21
#define HWIO_REO_R0_MISC_CTL_FRAGMENT_DEST_RING_BMSK                                                         0x1e0000
#define HWIO_REO_R0_MISC_CTL_FRAGMENT_DEST_RING_SHFT                                                               17

#define HWIO_REO_R0_ERROR_DESTINATION_MAPPING_IX_0_ERROR_DESTINATION_RING_7_BMSK                           0xf0000000
#define HWIO_REO_R0_ERROR_DESTINATION_MAPPING_IX_0_ERROR_DESTINATION_RING_7_SHFT                                   28
#define HWIO_REO_R0_ERROR_DESTINATION_MAPPING_IX_0_ERROR_DESTINATION_RING_6_BMSK                            0xf000000
#define HWIO_REO_R0_ERROR_DESTINATION_MAPPING_IX_0_ERROR_DESTINATION_RING_6_SHFT                                   24
#define HWIO_REO_R0_ERROR_DESTINATION_MAPPING_IX_0_ERROR_DESTINATION_RING_5_BMSK                             0xf00000
#define HWIO_REO_R0_ERROR_DESTINATION_MAPPING_IX_0_ERROR_DESTINATION_RING_5_SHFT                                   20
#define HWIO_REO_R0_ERROR_DESTINATION_MAPPING_IX_0_ERROR_DESTINATION_RING_4_BMSK                              0xf0000
#define HWIO_REO_R0_ERROR_DESTINATION_MAPPING_IX_0_ERROR_DESTINATION_RING_4_SHFT                                   16
#define HWIO_REO_R0_ERROR_DESTINATION_MAPPING_IX_0_ERROR_DESTINATION_RING_3_BMSK                               0xf000
#define HWIO_REO_R0_ERROR_DESTINATION_MAPPING_IX_0_ERROR_DESTINATION_RING_3_SHFT                                   12
#define HWIO_REO_R0_ERROR_DESTINATION_MAPPING_IX_0_ERROR_DESTINATION_RING_2_BMSK                                0xf00
#define HWIO_REO_R0_ERROR_DESTINATION_MAPPING_IX_0_ERROR_DESTINATION_RING_2_SHFT                                    8
#define HWIO_REO_R0_ERROR_DESTINATION_MAPPING_IX_0_ERROR_DESTINATION_RING_1_BMSK                                 0xf0
#define HWIO_REO_R0_ERROR_DESTINATION_MAPPING_IX_0_ERROR_DESTINATION_RING_1_SHFT                                    4
#define HWIO_REO_R0_ERROR_DESTINATION_MAPPING_IX_0_ERROR_DESTINATION_RING_0_BMSK                                  0xf
#define HWIO_REO_R0_ERROR_DESTINATION_MAPPING_IX_0_ERROR_DESTINATION_RING_0_SHFT                                    0

#define HWIO_REO_R0_ERROR_DESTINATION_MAPPING_IX_1_ERROR_DESTINATION_RING_14_SHFT                                  24
#define HWIO_REO_R0_ERROR_DESTINATION_MAPPING_IX_1_ERROR_DESTINATION_RING_13_BMSK                            0xf00000
#define HWIO_REO_R0_ERROR_DESTINATION_MAPPING_IX_1_ERROR_DESTINATION_RING_13_SHFT                                  20
#define HWIO_REO_R0_ERROR_DESTINATION_MAPPING_IX_1_ERROR_DESTINATION_RING_12_BMSK                             0xf0000
#define HWIO_REO_R0_ERROR_DESTINATION_MAPPING_IX_1_ERROR_DESTINATION_RING_12_SHFT                                  16
#define HWIO_REO_R0_ERROR_DESTINATION_MAPPING_IX_1_ERROR_DESTINATION_RING_11_BMSK                              0xf000
#define HWIO_REO_R0_ERROR_DESTINATION_MAPPING_IX_1_ERROR_DESTINATION_RING_11_SHFT                                  12
#define HWIO_REO_R0_ERROR_DESTINATION_MAPPING_IX_1_ERROR_DESTINATION_RING_10_BMSK                               0xf00
#define HWIO_REO_R0_ERROR_DESTINATION_MAPPING_IX_1_ERROR_DESTINATION_RING_10_SHFT                                   8
#define HWIO_REO_R0_ERROR_DESTINATION_MAPPING_IX_1_ERROR_DESTINATION_RING_9_BMSK                                 0xf0
#define HWIO_REO_R0_ERROR_DESTINATION_MAPPING_IX_1_ERROR_DESTINATION_RING_9_SHFT                                    4
#define HWIO_REO_R0_ERROR_DESTINATION_MAPPING_IX_1_ERROR_DESTINATION_RING_8_BMSK                                  0xf
#define HWIO_REO_R0_ERROR_DESTINATION_MAPPING_IX_1_ERROR_DESTINATION_RING_8_SHFT                                    0

#define HWIO_REO_R0_DESTINATION_RING_CTRL_IX_3_DEST_RING_MAPPING_31_BMSK                                   0xf0000000
#define HWIO_REO_R0_DESTINATION_RING_CTRL_IX_3_DEST_RING_MAPPING_31_SHFT                                           28
#define HWIO_REO_R0_DESTINATION_RING_CTRL_IX_3_DEST_RING_MAPPING_30_BMSK                                    0xf000000
#define HWIO_REO_R0_DESTINATION_RING_CTRL_IX_3_DEST_RING_MAPPING_30_SHFT                                           24
#define HWIO_REO_R0_DESTINATION_RING_CTRL_IX_3_DEST_RING_MAPPING_29_BMSK                                     0xf00000
#define HWIO_REO_R0_DESTINATION_RING_CTRL_IX_3_DEST_RING_MAPPING_29_SHFT                                           20
#define HWIO_REO_R0_DESTINATION_RING_CTRL_IX_3_DEST_RING_MAPPING_28_BMSK                                      0xf0000
#define HWIO_REO_R0_DESTINATION_RING_CTRL_IX_3_DEST_RING_MAPPING_28_SHFT                                           16
#define HWIO_REO_R0_DESTINATION_RING_CTRL_IX_3_DEST_RING_MAPPING_27_BMSK                                       0xf000
#define HWIO_REO_R0_DESTINATION_RING_CTRL_IX_3_DEST_RING_MAPPING_27_SHFT                                           12
#define HWIO_REO_R0_DESTINATION_RING_CTRL_IX_3_DEST_RING_MAPPING_26_BMSK                                        0xf00
#define HWIO_REO_R0_DESTINATION_RING_CTRL_IX_3_DEST_RING_MAPPING_26_SHFT                                            8
#define HWIO_REO_R0_DESTINATION_RING_CTRL_IX_3_DEST_RING_MAPPING_25_BMSK                                         0xf0
#define HWIO_REO_R0_DESTINATION_RING_CTRL_IX_3_DEST_RING_MAPPING_25_SHFT                                            4
#define HWIO_REO_R0_DESTINATION_RING_CTRL_IX_3_DEST_RING_MAPPING_24_BMSK                                          0xf
#define HWIO_REO_R0_DESTINATION_RING_CTRL_IX_3_DEST_RING_MAPPING_24_SHFT                                            0

#define HWIO_REO_R0_REO2SW1_RING_BASE_MSB_RING_SIZE_BMSK                                                    0xfffff00
#define HWIO_REO_R0_REO2SW1_RING_BASE_MSB_RING_SIZE_SHFT                                                            8
#define HWIO_REO_R0_REO2SW1_RING_BASE_MSB_RING_BASE_ADDR_MSB_BMSK                                                0xff
#define HWIO_REO_R0_REO2SW1_RING_BASE_MSB_RING_BASE_ADDR_MSB_SHFT                                                   0
#define HWIO_REO_R0_REO2SW0_RING_BASE_MSB_RING_SIZE_BMSK                                                    0xfffff00
#define HWIO_REO_R0_REO2SW0_RING_BASE_MSB_RING_SIZE_SHFT                                                            8
#define HWIO_REO_R0_SW2REO_RING_BASE_LSB_ADDR(x)                                                           ((x) + 0x1d4)
#define HWIO_REO_R2_SW2REO_RING_HP_ADDR(x)                                                                 ((x) + 0x3018)
#define HWIO_REO_R0_SW2REO_RING_BASE_MSB_RING_SIZE_BMSK                                                      0xffff00
#define HWIO_REO_R0_SW2REO_RING_BASE_MSB_RING_SIZE_SHFT                                                             8
#define HWIO_REO_R0_REO_CMD_RING_BASE_LSB_ADDR(x)                                                          ((x) + 0x160)
#define HWIO_REO_R2_REO_CMD_RING_HP_ADDR(x)                                                                ((x) + 0x3010)
#define HWIO_REO_R0_REO_CMD_RING_BASE_MSB_RING_SIZE_BMSK                                                     0xffff00
#define HWIO_REO_R0_REO_CMD_RING_BASE_MSB_RING_SIZE_SHFT                                                            8
#define HWIO_REO_R0_REO_STATUS_RING_BASE_LSB_ADDR(x)                                                       ((x) + 0x914)
#define HWIO_REO_R2_REO_STATUS_RING_HP_ADDR(x)                                                             ((x) + 0x3098)
#define HWIO_REO_R0_REO_STATUS_RING_BASE_MSB_RING_SIZE_BMSK                                                  0xffff00
#define HWIO_REO_R0_REO_STATUS_RING_BASE_MSB_RING_SIZE_SHFT                                                         8
#define HWIO_TCL_R0_SW2TCL2_RING_BASE_LSB_ADDR(x)                                                           ((x) + 0x8ec)
#define HWIO_TCL_R2_SW2TCL2_RING_HP_ADDR(x)                                                                 ((x) + 0x2008)
#define HWIO_TCL_R0_SW2TCL_CREDIT_RING_BASE_LSB_ADDR(x)                                                     ((x) + 0xabc)
#define HWIO_TCL_R2_SW2TCL_CREDIT_RING_HP_ADDR(x)                                                           ((x) + 0x2028)
#define HWIO_TCL_R0_SW2TCL_CREDIT_RING_BASE_MSB_RING_SIZE_BMSK                                               0xfffff00
#define HWIO_TCL_R0_SW2TCL_CREDIT_RING_BASE_MSB_RING_SIZE_SHFT                                                       8
#define HWIO_TCL_R0_TCL_STATUS1_RING_BASE_LSB_ADDR(x)                                                       ((x) + 0xc8c)
#define HWIO_TCL_R2_TCL_STATUS1_RING_HP_ADDR(x)                                                             ((x) + 0x2048)
#define HWIO_TCL_R0_TCL_STATUS1_RING_BASE_MSB_RING_SIZE_BMSK                                                  0xffff00
#define HWIO_TCL_R0_TCL_STATUS1_RING_BASE_MSB_RING_SIZE_SHFT                                                         8
#define HWIO_WBM_R0_WBM_IDLE_LINK_RING_BASE_LSB_ADDR(x)                                                         ((x) + 0xb5c)
#define HWIO_WBM_R2_WBM_IDLE_LINK_RING_HP_ADDR(x)                                                               ((x) + 0x30b8)
#define HWIO_WBM_R0_WBM_IDLE_LINK_RING_BASE_MSB_RING_SIZE_BMSK                                                   0xfffff00
#define HWIO_WBM_R0_WBM_IDLE_LINK_RING_BASE_MSB_RING_SIZE_SHFT                                                           8
#define HWIO_WBM_R0_SW_RELEASE_RING_BASE_LSB_ADDR(x)                                                            ((x) + 0x268)
#define HWIO_WBM_R2_SW_RELEASE_RING_HP_ADDR(x)                                                                  ((x) + 0x3018)
#define HWIO_WBM_R0_SW_RELEASE_RING_BASE_MSB_RING_SIZE_BMSK                                                       0xffff00
#define HWIO_WBM_R0_SW_RELEASE_RING_BASE_MSB_RING_SIZE_SHFT                                                              8
#define HWIO_WBM_R0_WBM2SW0_RELEASE_RING_BASE_LSB_ADDR(x)                                                       ((x) + 0xc20)
#define HWIO_WBM_R2_WBM2SW0_RELEASE_RING_HP_ADDR(x)                                                             ((x) + 0x30c8)
#define HWIO_WBM_R0_WBM2SW1_RELEASE_RING_BASE_LSB_ADDR(x)                                                       ((x) + 0xc94)
#define HWIO_WBM_R2_WBM2SW1_RELEASE_RING_HP_ADDR(x)                                                             ((x) + 0x30d0)
#define HWIO_WBM_R0_WBM2SW0_RELEASE_RING_BASE_MSB_RING_SIZE_BMSK                                                 0xfffff00
#define HWIO_WBM_R0_WBM2SW0_RELEASE_RING_BASE_MSB_RING_SIZE_SHFT                                                         8
#define HWIO_REO_R0_REO2SW1_RING_MSI2_BASE_LSB_ADDR(x)                                                     ((x) + 0x3fc)
#define HWIO_REO_R0_REO2SW1_RING_MSI2_BASE_MSB_ADDR(x)                                                     ((x) + 0x400)
#define HWIO_REO_R0_REO2SW1_RING_MSI2_DATA_ADDR(x)                                                         ((x) + 0x404)
#define HWIO_REO_R0_REO2SW1_RING_PRODUCER_INT2_SETUP_ADDR(x)                                               ((x) + 0x3f8)

#define HWIO_REO_R0_REO2SW0_RING_BASE_LSB_ADDR(x)                                                          ((x) + 0x744)
#define HWIO_REO_R0_REO2SW2_RING_BASE_LSB_ADDR(x)                                                          ((x) + 0x418)
#define HWIO_REO_R2_REO2SW0_RING_HP_ADDR(x)                                                                ((x) + 0x3078)
#define HWIO_REO_R2_REO2SW2_RING_HP_ADDR(x)                                                                ((x) + 0x3040)
#define HWIO_REO_R0_ERROR_DESTINATION_MAPPING_IX_0_ADDR(x)                                                 ((x) + 0x28)
#define HWIO_REO_R0_ERROR_DESTINATION_MAPPING_IX_1_ADDR(x)                                                 ((x) + 0x2c)
#define HWIO_REO_R0_PN_IN_DEST_ADDR(x)                                                                     ((x) + 0x4c)
#define HWIO_TCL_R0_PCP_TID_MAP_RMSK                                                                          0xffffff
#define HWIO_TCL_R0_TID_MAP_PRTY_RMSK                                                                             0xef
#define HWIO_TCL_R0_PCP_TID_MAP_PCP_7_BMSK                                                                    0xe00000
#define HWIO_TCL_R0_PCP_TID_MAP_PCP_7_SHFT                                                                          21
#define HWIO_TCL_R0_PCP_TID_MAP_PCP_6_BMSK                                                                    0x1c0000
#define HWIO_TCL_R0_PCP_TID_MAP_PCP_6_SHFT                                                                          18
#define HWIO_TCL_R0_PCP_TID_MAP_PCP_5_BMSK                                                                     0x38000
#define HWIO_TCL_R0_PCP_TID_MAP_PCP_5_SHFT                                                                          15
#define HWIO_TCL_R0_PCP_TID_MAP_PCP_4_BMSK                                                                      0x7000
#define HWIO_TCL_R0_PCP_TID_MAP_PCP_4_SHFT                                                                          12
#define HWIO_TCL_R0_PCP_TID_MAP_PCP_3_BMSK                                                                       0xe00
#define HWIO_TCL_R0_PCP_TID_MAP_PCP_3_SHFT                                                                           9
#define HWIO_TCL_R0_PCP_TID_MAP_PCP_2_BMSK                                                                       0x1c0
#define HWIO_TCL_R0_PCP_TID_MAP_PCP_2_SHFT                                                                           6
#define HWIO_TCL_R0_PCP_TID_MAP_PCP_1_BMSK                                                                        0x38
#define HWIO_TCL_R0_PCP_TID_MAP_PCP_1_SHFT                                                                           3
#define HWIO_TCL_R0_PCP_TID_MAP_PCP_0_BMSK                                                                         0x7
#define HWIO_TCL_R0_PCP_TID_MAP_PCP_0_SHFT                                                                           0

#define MAC_TCL_REG_REG_BASE                                                                                (UMAC_BASE            + 0x00044000)
#define MAC_TCL_REG_REG_BASE_SIZE                                                                           0x3000
#define MAC_TCL_REG_REG_BASE_USED                                                                           0x205c
#define MAC_TCL_REG_REG_BASE_PHYS                                                                           (UMAC_BASE_PHYS + 0x00044000)
#define MAC_TCL_REG_REG_BASE_OFFS                                                                           0x00044000


#endif
