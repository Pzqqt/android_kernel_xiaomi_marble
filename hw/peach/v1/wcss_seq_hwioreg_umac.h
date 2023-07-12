/*
 * Copyright (c) 2023 Qualcomm Innovation Center, Inc. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
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

#define HWIO_UMAC_MXI_R0_MXI_NULL_REMAP_CFG_REG_ADDR(x)                                                     ((x) + 0xa0)
#define HWIO_UMAC_MXI_R0_MXI_NULL_REMAP_CFG_REG_PHYS(x)                                                     ((x) + 0xa0)
#define HWIO_UMAC_MXI_R0_MXI_NULL_REMAP_CFG_REG_OFFS                                                        (0xa0)
#define HWIO_UMAC_MXI_R0_MXI_NULL_REMAP_CFG_REG_RMSK                                                               0x7
#define HWIO_UMAC_MXI_R0_MXI_NULL_REMAP_CFG_REG_POR                                                         0x00000000
#define HWIO_UMAC_MXI_R0_MXI_NULL_REMAP_CFG_REG_POR_RMSK                                                    0xffffffff
#define HWIO_UMAC_MXI_R0_MXI_NULL_REMAP_CFG_REG_ATTR                                                                     0x3
#define HWIO_UMAC_MXI_R0_MXI_NULL_REMAP_CFG_REG_IN(x)            \
                in_dword(HWIO_UMAC_MXI_R0_MXI_NULL_REMAP_CFG_REG_ADDR(x))
#define HWIO_UMAC_MXI_R0_MXI_NULL_REMAP_CFG_REG_INM(x, m)            \
                in_dword_masked(HWIO_UMAC_MXI_R0_MXI_NULL_REMAP_CFG_REG_ADDR(x), m)
#define HWIO_UMAC_MXI_R0_MXI_NULL_REMAP_CFG_REG_OUT(x, v)            \
                out_dword(HWIO_UMAC_MXI_R0_MXI_NULL_REMAP_CFG_REG_ADDR(x),v)
#define HWIO_UMAC_MXI_R0_MXI_NULL_REMAP_CFG_REG_OUTM(x,m,v) \
                out_dword_masked_ns(HWIO_UMAC_MXI_R0_MXI_NULL_REMAP_CFG_REG_ADDR(x),m,v,HWIO_UMAC_MXI_R0_MXI_NULL_REMAP_CFG_REG_IN(x))
#define HWIO_UMAC_MXI_R0_MXI_NULL_REMAP_CFG_REG_SIZE_OF_NULL_REMAP_BMSK                                            0x7
#define HWIO_UMAC_MXI_R0_MXI_NULL_REMAP_CFG_REG_SIZE_OF_NULL_REMAP_SHFT                                              0

#define HWIO_UMAC_MXI_R0_WMAC_GXI_GXI_SS_UP_TIMEOUT_INT_CTRL_ADDR(x)                                        ((x) + 0xa4)
#define HWIO_UMAC_MXI_R0_WMAC_GXI_GXI_SS_UP_TIMEOUT_INT_CTRL_PHYS(x)                                        ((x) + 0xa4)
#define HWIO_UMAC_MXI_R0_WMAC_GXI_GXI_SS_UP_TIMEOUT_INT_CTRL_OFFS                                           (0xa4)
#define HWIO_UMAC_MXI_R0_WMAC_GXI_GXI_SS_UP_TIMEOUT_INT_CTRL_RMSK                                            0x1ffffff
#define HWIO_UMAC_MXI_R0_WMAC_GXI_GXI_SS_UP_TIMEOUT_INT_CTRL_POR                                            0x00001ffe
#define HWIO_UMAC_MXI_R0_WMAC_GXI_GXI_SS_UP_TIMEOUT_INT_CTRL_POR_RMSK                                       0xffffffff
#define HWIO_UMAC_MXI_R0_WMAC_GXI_GXI_SS_UP_TIMEOUT_INT_CTRL_ATTR                                                        0x3
#define HWIO_UMAC_MXI_R0_WMAC_GXI_GXI_SS_UP_TIMEOUT_INT_CTRL_IN(x)            \
                in_dword(HWIO_UMAC_MXI_R0_WMAC_GXI_GXI_SS_UP_TIMEOUT_INT_CTRL_ADDR(x))
#define HWIO_UMAC_MXI_R0_WMAC_GXI_GXI_SS_UP_TIMEOUT_INT_CTRL_INM(x, m)            \
                in_dword_masked(HWIO_UMAC_MXI_R0_WMAC_GXI_GXI_SS_UP_TIMEOUT_INT_CTRL_ADDR(x), m)
#define HWIO_UMAC_MXI_R0_WMAC_GXI_GXI_SS_UP_TIMEOUT_INT_CTRL_OUT(x, v)            \
                out_dword(HWIO_UMAC_MXI_R0_WMAC_GXI_GXI_SS_UP_TIMEOUT_INT_CTRL_ADDR(x),v)
#define HWIO_UMAC_MXI_R0_WMAC_GXI_GXI_SS_UP_TIMEOUT_INT_CTRL_OUTM(x,m,v) \
                out_dword_masked_ns(HWIO_UMAC_MXI_R0_WMAC_GXI_GXI_SS_UP_TIMEOUT_INT_CTRL_ADDR(x),m,v,HWIO_UMAC_MXI_R0_WMAC_GXI_GXI_SS_UP_TIMEOUT_INT_CTRL_IN(x))
#define HWIO_UMAC_MXI_R0_WMAC_GXI_GXI_SS_UP_TIMEOUT_INT_CTRL_GXI_SS_UP_TIMEOUT_STATS_BMSK                    0x1ffe000
#define HWIO_UMAC_MXI_R0_WMAC_GXI_GXI_SS_UP_TIMEOUT_INT_CTRL_GXI_SS_UP_TIMEOUT_STATS_SHFT                           13
#define HWIO_UMAC_MXI_R0_WMAC_GXI_GXI_SS_UP_TIMEOUT_INT_CTRL_GXI_SS_UP_TIMEOUT_LIMIT_BMSK                       0x1ffe
#define HWIO_UMAC_MXI_R0_WMAC_GXI_GXI_SS_UP_TIMEOUT_INT_CTRL_GXI_SS_UP_TIMEOUT_LIMIT_SHFT                            1
#define HWIO_UMAC_MXI_R0_WMAC_GXI_GXI_SS_UP_TIMEOUT_INT_CTRL_GXI_SS_UP_TIMEOUT_INT_BMSK                            0x1
#define HWIO_UMAC_MXI_R0_WMAC_GXI_GXI_SS_UP_TIMEOUT_INT_CTRL_GXI_SS_UP_TIMEOUT_INT_SHFT                              0

#define HWIO_UMAC_MXI_R0_WMAC_GXI_GXI_ERR_INTS_GXI_RD_ZERO_ADDR_ERR_INT_BMSK                                    0x1000
#define HWIO_UMAC_MXI_R0_WMAC_GXI_GXI_ERR_INTS_GXI_RD_ZERO_ADDR_ERR_INT_SHFT                                        12
#define HWIO_UMAC_MXI_R0_WMAC_GXI_GXI_ERR_INTS_GXI_RD_ZERO_SIZE_ERR_INT_BMSK                                     0x800
#define HWIO_UMAC_MXI_R0_WMAC_GXI_GXI_ERR_INTS_GXI_RD_ZERO_SIZE_ERR_INT_SHFT                                        11
#define HWIO_UMAC_MXI_R0_WMAC_GXI_GXI_ERR_INTS_GXI_WR_ZERO_ADDR_ERR_INT_BMSK                                     0x400
#define HWIO_UMAC_MXI_R0_WMAC_GXI_GXI_ERR_INTS_GXI_WR_ZERO_ADDR_ERR_INT_SHFT                                        10
#define HWIO_UMAC_MXI_R0_WMAC_GXI_GXI_ERR_INTS_GXI_WR_ZERO_SIZE_ERR_INT_BMSK                                     0x200
#define HWIO_UMAC_MXI_R0_WMAC_GXI_GXI_ERR_INTS_GXI_WR_ZERO_SIZE_ERR_INT_SHFT                                         9
#define HWIO_UMAC_MXI_R0_WMAC_GXI_GXI_ZERO_ERR_STATS_ADDR(x)                                                ((x) + 0xd8)
#define HWIO_UMAC_MXI_R0_WMAC_GXI_GXI_ZERO_ERR_STATS_PHYS(x)                                                ((x) + 0xd8)
#define HWIO_UMAC_MXI_R0_WMAC_GXI_GXI_ZERO_ERR_STATS_OFFS                                                   (0xd8)
#define HWIO_UMAC_MXI_R0_WMAC_GXI_GXI_ZERO_ERR_STATS_RMSK                                                   0xffffffff
#define HWIO_UMAC_MXI_R0_WMAC_GXI_GXI_ZERO_ERR_STATS_POR                                                    0x00000000
#define HWIO_UMAC_MXI_R0_WMAC_GXI_GXI_ZERO_ERR_STATS_POR_RMSK                                               0xffffffff
#define HWIO_UMAC_MXI_R0_WMAC_GXI_GXI_ZERO_ERR_STATS_ATTR                                                                0x1
#define HWIO_UMAC_MXI_R0_WMAC_GXI_GXI_ZERO_ERR_STATS_IN(x)            \
                in_dword(HWIO_UMAC_MXI_R0_WMAC_GXI_GXI_ZERO_ERR_STATS_ADDR(x))
#define HWIO_UMAC_MXI_R0_WMAC_GXI_GXI_ZERO_ERR_STATS_INM(x, m)            \
                in_dword_masked(HWIO_UMAC_MXI_R0_WMAC_GXI_GXI_ZERO_ERR_STATS_ADDR(x), m)
#define HWIO_UMAC_MXI_R0_WMAC_GXI_GXI_ZERO_ERR_STATS_RD_ZERO_ADDR_PORT_BMSK                                 0xff000000
#define HWIO_UMAC_MXI_R0_WMAC_GXI_GXI_ZERO_ERR_STATS_RD_ZERO_ADDR_PORT_SHFT                                         24
#define HWIO_UMAC_MXI_R0_WMAC_GXI_GXI_ZERO_ERR_STATS_RD_ZERO_SIZE_PORT_BMSK                                   0xff0000
#define HWIO_UMAC_MXI_R0_WMAC_GXI_GXI_ZERO_ERR_STATS_RD_ZERO_SIZE_PORT_SHFT                                         16
#define HWIO_UMAC_MXI_R0_WMAC_GXI_GXI_ZERO_ERR_STATS_WR_ZERO_ADDR_PORT_BMSK                                     0xff00
#define HWIO_UMAC_MXI_R0_WMAC_GXI_GXI_ZERO_ERR_STATS_WR_ZERO_ADDR_PORT_SHFT                                          8
#define HWIO_UMAC_MXI_R0_WMAC_GXI_GXI_ZERO_ERR_STATS_WR_ZERO_SIZE_PORT_BMSK                                       0xff
#define HWIO_UMAC_MXI_R0_WMAC_GXI_GXI_ZERO_ERR_STATS_WR_ZERO_SIZE_PORT_SHFT                                          0

#define HWIO_UMAC_MXI_R0_MXI_TRACKING_TRANSACTION_TIME_n_ADDR(base,n)                                       ((base) + 0X1A4 + (0x4*(n)))
#define HWIO_UMAC_MXI_R0_MXI_TRACKING_TRANSACTION_TIME_n_PHYS(base,n)                                       ((base) + 0X1A4 + (0x4*(n)))
#define HWIO_UMAC_MXI_R0_MXI_TRACKING_TRANSACTION_TIME_n_OFFS(n)                                            (0X1A4 + (0x4*(n)))
#define HWIO_UMAC_MXI_R0_MXI_TRACKING_TRANSACTION_TIME_n_RMSK                                                    0xfff
#define HWIO_UMAC_MXI_R0_MXI_TRACKING_TRANSACTION_TIME_n_MAXn                                                        3
#define HWIO_UMAC_MXI_R0_MXI_TRACKING_TRANSACTION_TIME_n_POR                                                0x00000000
#define HWIO_UMAC_MXI_R0_MXI_TRACKING_TRANSACTION_TIME_n_POR_RMSK                                           0xffffffff
#define HWIO_UMAC_MXI_R0_MXI_TRACKING_TRANSACTION_TIME_n_ATTR                                                            0x1
#define HWIO_UMAC_MXI_R0_MXI_TRACKING_TRANSACTION_TIME_n_INI(base,n)                \
                in_dword_masked(HWIO_UMAC_MXI_R0_MXI_TRACKING_TRANSACTION_TIME_n_ADDR(base,n), HWIO_UMAC_MXI_R0_MXI_TRACKING_TRANSACTION_TIME_n_RMSK)
#define HWIO_UMAC_MXI_R0_MXI_TRACKING_TRANSACTION_TIME_n_INMI(base,n,mask)        \
                in_dword_masked(HWIO_UMAC_MXI_R0_MXI_TRACKING_TRANSACTION_TIME_n_ADDR(base,n), mask)
#define HWIO_UMAC_MXI_R0_MXI_TRACKING_TRANSACTION_TIME_n_TRANSACTION_TIME_BMSK                                   0xfff
#define HWIO_UMAC_MXI_R0_MXI_TRACKING_TRANSACTION_TIME_n_TRANSACTION_TIME_SHFT                                       0

#define HWIO_UMAC_MXI_R0_GXI_TXN_HALT_ACK_CONFIG_ADDR(x)                                                    ((x) + 0x1c4)
#define HWIO_UMAC_MXI_R0_GXI_TXN_HALT_ACK_CONFIG_PHYS(x)                                                    ((x) + 0x1c4)
#define HWIO_UMAC_MXI_R0_GXI_TXN_HALT_ACK_CONFIG_OFFS                                                       (0x1c4)
#define HWIO_UMAC_MXI_R0_GXI_TXN_HALT_ACK_CONFIG_RMSK                                                              0x3
#define HWIO_UMAC_MXI_R0_GXI_TXN_HALT_ACK_CONFIG_POR                                                        0x00000000
#define HWIO_UMAC_MXI_R0_GXI_TXN_HALT_ACK_CONFIG_POR_RMSK                                                   0xffffffff
#define HWIO_UMAC_MXI_R0_GXI_TXN_HALT_ACK_CONFIG_ATTR                                                                    0x3
#define HWIO_UMAC_MXI_R0_GXI_TXN_HALT_ACK_CONFIG_IN(x)            \
                in_dword(HWIO_UMAC_MXI_R0_GXI_TXN_HALT_ACK_CONFIG_ADDR(x))
#define HWIO_UMAC_MXI_R0_GXI_TXN_HALT_ACK_CONFIG_INM(x, m)            \
                in_dword_masked(HWIO_UMAC_MXI_R0_GXI_TXN_HALT_ACK_CONFIG_ADDR(x), m)
#define HWIO_UMAC_MXI_R0_GXI_TXN_HALT_ACK_CONFIG_OUT(x, v)            \
                out_dword(HWIO_UMAC_MXI_R0_GXI_TXN_HALT_ACK_CONFIG_ADDR(x),v)
#define HWIO_UMAC_MXI_R0_GXI_TXN_HALT_ACK_CONFIG_OUTM(x,m,v) \
                out_dword_masked_ns(HWIO_UMAC_MXI_R0_GXI_TXN_HALT_ACK_CONFIG_ADDR(x),m,v,HWIO_UMAC_MXI_R0_GXI_TXN_HALT_ACK_CONFIG_IN(x))
#define HWIO_UMAC_MXI_R0_GXI_TXN_HALT_ACK_CONFIG_TXN_HALT_ACK_BMSK                                                 0x2
#define HWIO_UMAC_MXI_R0_GXI_TXN_HALT_ACK_CONFIG_TXN_HALT_ACK_SHFT                                                   1
#define HWIO_UMAC_MXI_R0_GXI_TXN_HALT_ACK_CONFIG_TXN_HALT_EN_BMSK                                                  0x1
#define HWIO_UMAC_MXI_R0_GXI_TXN_HALT_ACK_CONFIG_TXN_HALT_EN_SHFT                                                    0

#define HWIO_UMAC_MXI_R1_MXI_TESTBUS_CAPTURE_n_ADDR(base,n)                                                 ((base) + 0X508 + (0x4*(n)))
#define HWIO_UMAC_MXI_R1_MXI_TESTBUS_CAPTURE_n_PHYS(base,n)                                                 ((base) + 0X508 + (0x4*(n)))
#define HWIO_UMAC_MXI_R1_MXI_TESTBUS_CAPTURE_n_OFFS(n)                                                      (0X508 + (0x4*(n)))
#define HWIO_UMAC_MXI_R1_MXI_TESTBUS_CAPTURE_n_RMSK                                                         0xffffffff
#define HWIO_UMAC_MXI_R1_MXI_TESTBUS_CAPTURE_n_MAXn                                                                 63
#define HWIO_UMAC_MXI_R1_MXI_TESTBUS_CAPTURE_n_POR                                                          0x00000000
#define HWIO_UMAC_MXI_R1_MXI_TESTBUS_CAPTURE_n_POR_RMSK                                                     0xffffffff
#define HWIO_UMAC_MXI_R1_MXI_TESTBUS_CAPTURE_n_ATTR                                                                      0x1
#define HWIO_UMAC_MXI_R1_MXI_TESTBUS_CAPTURE_n_INI(base,n)                \
                in_dword_masked(HWIO_UMAC_MXI_R1_MXI_TESTBUS_CAPTURE_n_ADDR(base,n), HWIO_UMAC_MXI_R1_MXI_TESTBUS_CAPTURE_n_RMSK)
#define HWIO_UMAC_MXI_R1_MXI_TESTBUS_CAPTURE_n_INMI(base,n,mask)        \
                in_dword_masked(HWIO_UMAC_MXI_R1_MXI_TESTBUS_CAPTURE_n_ADDR(base,n), mask)
#define HWIO_UMAC_MXI_R1_MXI_TESTBUS_CAPTURE_n_DATA_BMSK                                                    0xffffffff
#define HWIO_UMAC_MXI_R1_MXI_TESTBUS_CAPTURE_n_DATA_SHFT                                                             0

#define WBM_REG_REG_BASE                                                                                        (UMAC_BASE      + 0x00034000)
#define HWIO_WBM_R0_SW_COOKIE_CFG0_ADDR(x)                                                                      ((x) + 0x40)
#define HWIO_WBM_R0_SW_COOKIE_CFG1_ADDR(x)                                                                      ((x) + 0x44)
#define HWIO_WBM_R0_SW_COOKIE_CFG1_PAGE_ALIGNMENT_BMSK                                                             0x40000
#define HWIO_WBM_R0_SW_COOKIE_CFG1_PAGE_ALIGNMENT_SHFT                                                                  18
#define HWIO_WBM_R0_SW_COOKIE_CFG1_COOKIE_OFFSET_MSB_BMSK                                                          0x3e000
#define HWIO_WBM_R0_SW_COOKIE_CFG1_COOKIE_OFFSET_MSB_SHFT                                                               13
#define HWIO_WBM_R0_SW_COOKIE_CFG1_COOKIE_PAGE_MSB_BMSK                                                             0x1f00
#define HWIO_WBM_R0_SW_COOKIE_CFG1_COOKIE_PAGE_MSB_SHFT                                                                  8
#define HWIO_WBM_R0_SW_COOKIE_CFG1_CMEM_LUT_BASE_ADDR_39_32_BMSK                                                      0xff
#define HWIO_WBM_R0_SW_COOKIE_CFG1_CMEM_LUT_BASE_ADDR_39_32_SHFT                                                         0

#define HWIO_WBM_R0_MISC_CONTROL_ADDR(x)                                                                        ((x) + 0x7c)
#define HWIO_WBM_R0_WBM_CFG_2_ADDR(x)                                                                           ((x) + 0x90)
#define HWIO_WBM_R0_WBM_CFG_2_PHYS(x)                                                                           ((x) + 0x90)
#define HWIO_WBM_R0_WBM_CFG_2_OFFS                                                                              (0x90)
#define HWIO_WBM_R0_WBM_CFG_2_RMSK                                                                                    0x4b
#define HWIO_WBM_R0_WBM_CFG_2_POR                                                                               0x00000040
#define HWIO_WBM_R0_WBM_CFG_2_POR_RMSK                                                                          0xffffffff
#define HWIO_WBM_R0_WBM_CFG_2_ATTR                                                                                           0x3
#define HWIO_WBM_R0_WBM_CFG_2_IN(x)            \
                in_dword(HWIO_WBM_R0_WBM_CFG_2_ADDR(x))
#define HWIO_WBM_R0_WBM_CFG_2_INM(x, m)            \
                in_dword_masked(HWIO_WBM_R0_WBM_CFG_2_ADDR(x), m)
#define HWIO_WBM_R0_WBM_CFG_2_OUT(x, v)            \
                out_dword(HWIO_WBM_R0_WBM_CFG_2_ADDR(x),v)
#define HWIO_WBM_R0_WBM_CFG_2_OUTM(x,m,v) \
                out_dword_masked_ns(HWIO_WBM_R0_WBM_CFG_2_ADDR(x),m,v,HWIO_WBM_R0_WBM_CFG_2_IN(x))
#define HWIO_WBM_R0_WBM_CFG_2_COOKIE_DEBUG_SEL_BMSK                                                                   0x40
#define HWIO_WBM_R0_WBM_CFG_2_COOKIE_DEBUG_SEL_SHFT                                                                      6
#define HWIO_WBM_R0_WBM_CFG_2_COOKIE_CONV_INDICATION_EN_BMSK                                                           0x8
#define HWIO_WBM_R0_WBM_CFG_2_COOKIE_CONV_INDICATION_EN_SHFT                                                             3
#define HWIO_WBM_R0_WBM_CFG_2_ERROR_PATH_COOKIE_CONV_EN_BMSK                                                           0x2
#define HWIO_WBM_R0_WBM_CFG_2_ERROR_PATH_COOKIE_CONV_EN_SHFT                                                             1
#define HWIO_WBM_R0_WBM_CFG_2_RELEASE_PATH_COOKIE_CONV_EN_BMSK                                                         0x1
#define HWIO_WBM_R0_WBM_CFG_2_RELEASE_PATH_COOKIE_CONV_EN_SHFT                                                           0

#define HWIO_WBM_R0_SW_COOKIE_CONVERT_CFG_ADDR(x)                                                               ((x) + 0x94)
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

#define HWIO_WBM_R0_IDLE_LIST_CONTROL_ADDR(x)                                                                   ((x) + 0x240)
#define HWIO_WBM_R0_IDLE_LIST_CONTROL_SCATTER_BUFFER_SIZE_BMSK                                                       0x7fc
#define HWIO_WBM_R0_IDLE_LIST_CONTROL_SCATTER_BUFFER_SIZE_SHFT                                                           2
#define HWIO_WBM_R0_IDLE_LIST_CONTROL_LINK_DESC_IDLE_LIST_MODE_BMSK                                                    0x2
#define HWIO_WBM_R0_IDLE_LIST_CONTROL_LINK_DESC_IDLE_LIST_MODE_SHFT                                                      1
#define HWIO_WBM_R0_IDLE_LIST_SIZE_ADDR(x)                                                                      ((x) + 0x244)
#define HWIO_WBM_R0_IDLE_LIST_SIZE_SCATTER_RING_SIZE_OF_IDLE_LINK_DESC_LIST_BMSK                                0xffff0000
#define HWIO_WBM_R0_IDLE_LIST_SIZE_SCATTER_RING_SIZE_OF_IDLE_LINK_DESC_LIST_SHFT                                        16
#define HWIO_WBM_R0_IDLE_LIST_SIZE_SCATTER_RING_SIZE_OF_IDLE_BUF_LIST_BMSK                                          0xffff
#define HWIO_WBM_R0_IDLE_LIST_SIZE_SCATTER_RING_SIZE_OF_IDLE_BUF_LIST_SHFT                                               0

#define HWIO_WBM_R0_SCATTERED_LINK_DESC_LIST_BASE_LSB_ADDR(x)                                                   ((x) + 0x250)
#define HWIO_WBM_R0_SCATTERED_LINK_DESC_LIST_BASE_MSB_ADDR(x)                                                   ((x) + 0x254)
#define HWIO_WBM_R0_SCATTERED_LINK_DESC_LIST_BASE_MSB_ADDRESS_MATCH_TAG_BMSK                                    0xffffff00
#define HWIO_WBM_R0_SCATTERED_LINK_DESC_LIST_BASE_MSB_ADDRESS_MATCH_TAG_SHFT                                             8
#define HWIO_WBM_R0_SCATTERED_LINK_DESC_LIST_BASE_MSB_BASE_ADDRESS_39_32_BMSK                                         0xff
#define HWIO_WBM_R0_SCATTERED_LINK_DESC_LIST_BASE_MSB_BASE_ADDRESS_39_32_SHFT                                            0

#define HWIO_WBM_R0_SCATTERED_LINK_DESC_PTR_HEAD_INFO_IX0_ADDR(x)                                               ((x) + 0x260)
#define HWIO_WBM_R0_SCATTERED_LINK_DESC_PTR_HEAD_INFO_IX1_ADDR(x)                                               ((x) + 0x264)
#define HWIO_WBM_R0_SCATTERED_LINK_DESC_PTR_HEAD_INFO_IX1_HEAD_POINTER_OFFSET_BMSK                                0x1fff00
#define HWIO_WBM_R0_SCATTERED_LINK_DESC_PTR_HEAD_INFO_IX1_HEAD_POINTER_OFFSET_SHFT                                       8
#define HWIO_WBM_R0_SCATTERED_LINK_DESC_PTR_HEAD_INFO_IX1_BUFFER_ADDRESS_39_32_BMSK                                   0xff
#define HWIO_WBM_R0_SCATTERED_LINK_DESC_PTR_HEAD_INFO_IX1_BUFFER_ADDRESS_39_32_SHFT                                      0

#define HWIO_WBM_R0_SCATTERED_LINK_DESC_PTR_TAIL_INFO_IX0_ADDR(x)                                               ((x) + 0x270)
#define HWIO_WBM_R0_SCATTERED_LINK_DESC_PTR_TAIL_INFO_IX1_ADDR(x)                                               ((x) + 0x274)
#define HWIO_WBM_R0_SCATTERED_LINK_DESC_PTR_TAIL_INFO_IX1_TAIL_POINTER_OFFSET_BMSK                                0x1fff00
#define HWIO_WBM_R0_SCATTERED_LINK_DESC_PTR_TAIL_INFO_IX1_TAIL_POINTER_OFFSET_SHFT                                       8
#define HWIO_WBM_R0_SCATTERED_LINK_DESC_PTR_TAIL_INFO_IX1_BUFFER_ADDRESS_39_32_BMSK                                   0xff
#define HWIO_WBM_R0_SCATTERED_LINK_DESC_PTR_TAIL_INFO_IX1_BUFFER_ADDRESS_39_32_SHFT                                      0

#define HWIO_WBM_R0_SCATTERED_LINK_DESC_PTR_HP_ADDR(x)                                                          ((x) + 0x27c)
#define HWIO_WBM_R0_TQM_RELEASE_RING_MISC_TRANSACTION_TYPE_BMSK                                                   0x400000
#define HWIO_WBM_R0_TQM_RELEASE_RING_MISC_TRANSACTION_TYPE_SHFT                                                         22
#define HWIO_WBM_R0_REO_RELEASE_RING_MISC_TRANSACTION_TYPE_BMSK                                                   0x400000
#define HWIO_WBM_R0_REO_RELEASE_RING_MISC_TRANSACTION_TYPE_SHFT                                                         22
#define HWIO_WBM_R0_SW_RELEASE_RING_BASE_LSB_ADDR(x)                                                            ((x) + 0x37c)
#define HWIO_WBM_R0_SW_RELEASE_RING_BASE_MSB_RING_SIZE_BMSK                                                       0xffff00
#define HWIO_WBM_R0_SW_RELEASE_RING_BASE_MSB_RING_SIZE_SHFT                                                              8
#define HWIO_WBM_R0_SW_RELEASE_RING_MISC_TRANSACTION_TYPE_BMSK                                                    0x400000
#define HWIO_WBM_R0_SW_RELEASE_RING_MISC_TRANSACTION_TYPE_SHFT                                                          22
#define HWIO_WBM_R0_PPE_RELEASE_RING_MISC_TRANSACTION_TYPE_BMSK                                                   0x400000
#define HWIO_WBM_R0_PPE_RELEASE_RING_MISC_TRANSACTION_TYPE_SHFT                                                         22
#define HWIO_WBM_R0_FW_RELEASE_RING_MISC_TRANSACTION_TYPE_BMSK                                                    0x400000
#define HWIO_WBM_R0_FW_RELEASE_RING_MISC_TRANSACTION_TYPE_SHFT                                                          22
#define HWIO_WBM_R0_RXDMA0_RELEASE_RING_MISC_TRANSACTION_TYPE_BMSK                                                0x400000
#define HWIO_WBM_R0_RXDMA0_RELEASE_RING_MISC_TRANSACTION_TYPE_SHFT                                                      22
#define HWIO_WBM_R0_WBM2TQM_LINK_RING_MISC_TRANSACTION_TYPE_BMSK                                                 0x8000000
#define HWIO_WBM_R0_WBM2TQM_LINK_RING_MISC_TRANSACTION_TYPE_SHFT                                                        27
#define HWIO_WBM_R0_WBM2REO_LINK_RING_MISC_TRANSACTION_TYPE_BMSK                                                 0x8000000
#define HWIO_WBM_R0_WBM2REO_LINK_RING_MISC_TRANSACTION_TYPE_SHFT                                                        27
#define HWIO_WBM_R0_WBM2SW_LINK_RING_MISC_TRANSACTION_TYPE_BMSK                                                  0x8000000
#define HWIO_WBM_R0_WBM2SW_LINK_RING_MISC_TRANSACTION_TYPE_SHFT                                                         27
#define HWIO_WBM_R0_WBM2FW_LINK_RING_MISC_TRANSACTION_TYPE_BMSK                                                  0x8000000
#define HWIO_WBM_R0_WBM2FW_LINK_RING_MISC_TRANSACTION_TYPE_SHFT                                                         27
#define HWIO_WBM_R0_WBM2RXDMA0_LINK_RING_MISC_TRANSACTION_TYPE_BMSK                                              0x8000000
#define HWIO_WBM_R0_WBM2RXDMA0_LINK_RING_MISC_TRANSACTION_TYPE_SHFT                                                     27
#define HWIO_WBM_R0_WBM_IDLE_LINK_RING_BASE_LSB_ADDR(x)                                                         ((x) + 0xd3c)
#define HWIO_WBM_R0_WBM_IDLE_LINK_RING_BASE_MSB_RING_SIZE_BMSK                                                   0xfffff00
#define HWIO_WBM_R0_WBM_IDLE_LINK_RING_BASE_MSB_RING_SIZE_SHFT                                                           8
#define HWIO_WBM_R0_WBM_IDLE_LINK_RING_MISC_ADDR(x)                                                             ((x) + 0xd4c)
#define HWIO_WBM_R0_WBM_IDLE_LINK_RING_MISC_RING_ID_DISABLE_BMSK                                                       0x1
#define HWIO_WBM_R0_WBM_IDLE_LINK_RING_MISC_RING_ID_DISABLE_SHFT                                                         0

#define HWIO_WBM_R0_WBM2FW_RELEASE_RING_MISC_TRANSACTION_TYPE_BMSK                                               0x8000000
#define HWIO_WBM_R0_WBM2FW_RELEASE_RING_MISC_TRANSACTION_TYPE_SHFT                                                      27
#define HWIO_WBM_R0_WBM2SW0_RELEASE_RING_BASE_LSB_ADDR(x)                                                       ((x) + 0xe08)
#define HWIO_WBM_R0_WBM2SW0_RELEASE_RING_BASE_MSB_RING_SIZE_BMSK                                                 0xfffff00
#define HWIO_WBM_R0_WBM2SW0_RELEASE_RING_BASE_MSB_RING_SIZE_SHFT                                                         8
#define HWIO_WBM_R0_WBM2SW0_RELEASE_RING_MISC_TRANSACTION_TYPE_BMSK                                              0x8000000
#define HWIO_WBM_R0_WBM2SW0_RELEASE_RING_MISC_TRANSACTION_TYPE_SHFT                                                     27
#define HWIO_WBM_R0_WBM2SW1_RELEASE_RING_BASE_LSB_ADDR(x)                                                       ((x) + 0xe80)
#define HWIO_WBM_R0_WBM2SW1_RELEASE_RING_MISC_TRANSACTION_TYPE_BMSK                                              0x8000000
#define HWIO_WBM_R0_WBM2SW1_RELEASE_RING_MISC_TRANSACTION_TYPE_SHFT                                                     27
#define HWIO_WBM_R0_WBM2SW2_RELEASE_RING_MISC_TRANSACTION_TYPE_BMSK                                              0x8000000
#define HWIO_WBM_R0_WBM2SW2_RELEASE_RING_MISC_TRANSACTION_TYPE_SHFT                                                     27
#define HWIO_WBM_R0_WBM2SW3_RELEASE_RING_MISC_TRANSACTION_TYPE_BMSK                                              0x8000000
#define HWIO_WBM_R0_WBM2SW3_RELEASE_RING_MISC_TRANSACTION_TYPE_SHFT                                                     27
#define HWIO_WBM_R0_WBM2SW4_RELEASE_RING_MISC_TRANSACTION_TYPE_BMSK                                              0x8000000
#define HWIO_WBM_R0_WBM2SW4_RELEASE_RING_MISC_TRANSACTION_TYPE_SHFT                                                     27
#define HWIO_WBM_R0_WBM2SW5_RELEASE_RING_MISC_TRANSACTION_TYPE_BMSK                                              0x8000000
#define HWIO_WBM_R0_WBM2SW5_RELEASE_RING_MISC_TRANSACTION_TYPE_SHFT                                                     27
#define HWIO_WBM_R0_WBM2SW6_RELEASE_RING_MISC_TRANSACTION_TYPE_BMSK                                              0x8000000
#define HWIO_WBM_R0_WBM2SW6_RELEASE_RING_MISC_TRANSACTION_TYPE_SHFT                                                     27
#define HWIO_WBM_R0_WBM_ERROR_RELEASE_RING_MISC_TRANSACTION_TYPE_BMSK                                            0x8000000
#define HWIO_WBM_R0_WBM_ERROR_RELEASE_RING_MISC_TRANSACTION_TYPE_SHFT                                                   27
#define HWIO_WBM_R0_MULTI_SRNG_CONS_RING_MISC_COMMON_ADDR(x)                                                    ((x) + 0x1408)
#define HWIO_WBM_R0_MULTI_SRNG_CONS_RING_MISC_COMMON_PHYS(x)                                                    ((x) + 0x1408)
#define HWIO_WBM_R0_MULTI_SRNG_CONS_RING_MISC_COMMON_OFFS                                                       (0x1408)
#define HWIO_WBM_R0_MULTI_SRNG_CONS_RING_MISC_COMMON_RMSK                                                         0x1fffff
#define HWIO_WBM_R0_MULTI_SRNG_CONS_RING_MISC_COMMON_POR                                                        0x00001000
#define HWIO_WBM_R0_MULTI_SRNG_CONS_RING_MISC_COMMON_POR_RMSK                                                   0xffffffff
#define HWIO_WBM_R0_MULTI_SRNG_CONS_RING_MISC_COMMON_ATTR                                                                    0x3
#define HWIO_WBM_R0_MULTI_SRNG_CONS_RING_MISC_COMMON_IN(x)            \
                in_dword(HWIO_WBM_R0_MULTI_SRNG_CONS_RING_MISC_COMMON_ADDR(x))
#define HWIO_WBM_R0_MULTI_SRNG_CONS_RING_MISC_COMMON_INM(x, m)            \
                in_dword_masked(HWIO_WBM_R0_MULTI_SRNG_CONS_RING_MISC_COMMON_ADDR(x), m)
#define HWIO_WBM_R0_MULTI_SRNG_CONS_RING_MISC_COMMON_OUT(x, v)            \
                out_dword(HWIO_WBM_R0_MULTI_SRNG_CONS_RING_MISC_COMMON_ADDR(x),v)
#define HWIO_WBM_R0_MULTI_SRNG_CONS_RING_MISC_COMMON_OUTM(x,m,v) \
                out_dword_masked_ns(HWIO_WBM_R0_MULTI_SRNG_CONS_RING_MISC_COMMON_ADDR(x),m,v,HWIO_WBM_R0_MULTI_SRNG_CONS_RING_MISC_COMMON_IN(x))
#define HWIO_WBM_R0_MULTI_SRNG_CONS_RING_MISC_COMMON_MISC_CONTROL_BMSK                                            0x1fe000
#define HWIO_WBM_R0_MULTI_SRNG_CONS_RING_MISC_COMMON_MISC_CONTROL_SHFT                                                  13
#define HWIO_WBM_R0_MULTI_SRNG_CONS_RING_MISC_COMMON_SRNG_IS_IDLE_BMSK                                              0x1000
#define HWIO_WBM_R0_MULTI_SRNG_CONS_RING_MISC_COMMON_SRNG_IS_IDLE_SHFT                                                  12
#define HWIO_WBM_R0_MULTI_SRNG_CONS_RING_MISC_COMMON_SRNG_SM1_STATE2_BMSK                                            0xc00
#define HWIO_WBM_R0_MULTI_SRNG_CONS_RING_MISC_COMMON_SRNG_SM1_STATE2_SHFT                                               10
#define HWIO_WBM_R0_MULTI_SRNG_CONS_RING_MISC_COMMON_SRNG_SM1_STATE1_BMSK                                            0x3c0
#define HWIO_WBM_R0_MULTI_SRNG_CONS_RING_MISC_COMMON_SRNG_SM1_STATE1_SHFT                                                6
#define HWIO_WBM_R0_MULTI_SRNG_CONS_RING_MISC_COMMON_SRNG_SM0_STATE2_BMSK                                             0x30
#define HWIO_WBM_R0_MULTI_SRNG_CONS_RING_MISC_COMMON_SRNG_SM0_STATE2_SHFT                                                4
#define HWIO_WBM_R0_MULTI_SRNG_CONS_RING_MISC_COMMON_SRNG_SM0_STATE1_BMSK                                              0xf
#define HWIO_WBM_R0_MULTI_SRNG_CONS_RING_MISC_COMMON_SRNG_SM0_STATE1_SHFT                                                0

#define HWIO_WBM_R0_MULTI_SRNG_CONS_RING_MISC_COMMON_EXT_ADDR(x)                                                ((x) + 0x140c)
#define HWIO_WBM_R0_MULTI_SRNG_CONS_RING_MISC_COMMON_EXT_PHYS(x)                                                ((x) + 0x140c)
#define HWIO_WBM_R0_MULTI_SRNG_CONS_RING_MISC_COMMON_EXT_OFFS                                                   (0x140c)
#define HWIO_WBM_R0_MULTI_SRNG_CONS_RING_MISC_COMMON_EXT_RMSK                                                     0xffffff
#define HWIO_WBM_R0_MULTI_SRNG_CONS_RING_MISC_COMMON_EXT_POR                                                    0x00000fff
#define HWIO_WBM_R0_MULTI_SRNG_CONS_RING_MISC_COMMON_EXT_POR_RMSK                                               0xffffffff
#define HWIO_WBM_R0_MULTI_SRNG_CONS_RING_MISC_COMMON_EXT_ATTR                                                                0x3
#define HWIO_WBM_R0_MULTI_SRNG_CONS_RING_MISC_COMMON_EXT_IN(x)            \
                in_dword(HWIO_WBM_R0_MULTI_SRNG_CONS_RING_MISC_COMMON_EXT_ADDR(x))
#define HWIO_WBM_R0_MULTI_SRNG_CONS_RING_MISC_COMMON_EXT_INM(x, m)            \
                in_dword_masked(HWIO_WBM_R0_MULTI_SRNG_CONS_RING_MISC_COMMON_EXT_ADDR(x), m)
#define HWIO_WBM_R0_MULTI_SRNG_CONS_RING_MISC_COMMON_EXT_OUT(x, v)            \
                out_dword(HWIO_WBM_R0_MULTI_SRNG_CONS_RING_MISC_COMMON_EXT_ADDR(x),v)
#define HWIO_WBM_R0_MULTI_SRNG_CONS_RING_MISC_COMMON_EXT_OUTM(x,m,v) \
                out_dword_masked_ns(HWIO_WBM_R0_MULTI_SRNG_CONS_RING_MISC_COMMON_EXT_ADDR(x),m,v,HWIO_WBM_R0_MULTI_SRNG_CONS_RING_MISC_COMMON_EXT_IN(x))
#define HWIO_WBM_R0_MULTI_SRNG_CONS_RING_MISC_COMMON_EXT_RING_WATCHDOG_TIMER_STATUS_BMSK                          0xfff000
#define HWIO_WBM_R0_MULTI_SRNG_CONS_RING_MISC_COMMON_EXT_RING_WATCHDOG_TIMER_STATUS_SHFT                                12
#define HWIO_WBM_R0_MULTI_SRNG_CONS_RING_MISC_COMMON_EXT_WATCHDOG_TIMEOUT_VALUE_BMSK                                 0xfff
#define HWIO_WBM_R0_MULTI_SRNG_CONS_RING_MISC_COMMON_EXT_WATCHDOG_TIMEOUT_VALUE_SHFT                                     0

#define HWIO_WBM_R0_MULTI_SRNG_PROD_RING_MISC_COMMON_ADDR(x)                                                    ((x) + 0x1410)
#define HWIO_WBM_R0_MULTI_SRNG_PROD_RING_MISC_COMMON_PHYS(x)                                                    ((x) + 0x1410)
#define HWIO_WBM_R0_MULTI_SRNG_PROD_RING_MISC_COMMON_OFFS                                                       (0x1410)
#define HWIO_WBM_R0_MULTI_SRNG_PROD_RING_MISC_COMMON_RMSK                                                         0x1fffff
#define HWIO_WBM_R0_MULTI_SRNG_PROD_RING_MISC_COMMON_POR                                                        0x00001000
#define HWIO_WBM_R0_MULTI_SRNG_PROD_RING_MISC_COMMON_POR_RMSK                                                   0xffffffff
#define HWIO_WBM_R0_MULTI_SRNG_PROD_RING_MISC_COMMON_ATTR                                                                    0x3
#define HWIO_WBM_R0_MULTI_SRNG_PROD_RING_MISC_COMMON_IN(x)            \
                in_dword(HWIO_WBM_R0_MULTI_SRNG_PROD_RING_MISC_COMMON_ADDR(x))
#define HWIO_WBM_R0_MULTI_SRNG_PROD_RING_MISC_COMMON_INM(x, m)            \
                in_dword_masked(HWIO_WBM_R0_MULTI_SRNG_PROD_RING_MISC_COMMON_ADDR(x), m)
#define HWIO_WBM_R0_MULTI_SRNG_PROD_RING_MISC_COMMON_OUT(x, v)            \
                out_dword(HWIO_WBM_R0_MULTI_SRNG_PROD_RING_MISC_COMMON_ADDR(x),v)
#define HWIO_WBM_R0_MULTI_SRNG_PROD_RING_MISC_COMMON_OUTM(x,m,v) \
                out_dword_masked_ns(HWIO_WBM_R0_MULTI_SRNG_PROD_RING_MISC_COMMON_ADDR(x),m,v,HWIO_WBM_R0_MULTI_SRNG_PROD_RING_MISC_COMMON_IN(x))
#define HWIO_WBM_R0_MULTI_SRNG_PROD_RING_MISC_COMMON_MISC_CONTROL_BMSK                                            0x1fe000
#define HWIO_WBM_R0_MULTI_SRNG_PROD_RING_MISC_COMMON_MISC_CONTROL_SHFT                                                  13
#define HWIO_WBM_R0_MULTI_SRNG_PROD_RING_MISC_COMMON_SRNG_IS_IDLE_BMSK                                              0x1000
#define HWIO_WBM_R0_MULTI_SRNG_PROD_RING_MISC_COMMON_SRNG_IS_IDLE_SHFT                                                  12
#define HWIO_WBM_R0_MULTI_SRNG_PROD_RING_MISC_COMMON_SRNG_SM1_STATE2_BMSK                                            0xc00
#define HWIO_WBM_R0_MULTI_SRNG_PROD_RING_MISC_COMMON_SRNG_SM1_STATE2_SHFT                                               10
#define HWIO_WBM_R0_MULTI_SRNG_PROD_RING_MISC_COMMON_SRNG_SM1_STATE1_BMSK                                            0x3c0
#define HWIO_WBM_R0_MULTI_SRNG_PROD_RING_MISC_COMMON_SRNG_SM1_STATE1_SHFT                                                6
#define HWIO_WBM_R0_MULTI_SRNG_PROD_RING_MISC_COMMON_SRNG_SM0_STATE2_BMSK                                             0x30
#define HWIO_WBM_R0_MULTI_SRNG_PROD_RING_MISC_COMMON_SRNG_SM0_STATE2_SHFT                                                4
#define HWIO_WBM_R0_MULTI_SRNG_PROD_RING_MISC_COMMON_SRNG_SM0_STATE1_BMSK                                              0xf
#define HWIO_WBM_R0_MULTI_SRNG_PROD_RING_MISC_COMMON_SRNG_SM0_STATE1_SHFT                                                0

#define HWIO_WBM_R0_MULTI_SRNG_PROD_RING_MISC_COMMON_EXT_ADDR(x)                                                ((x) + 0x1414)
#define HWIO_WBM_R0_MULTI_SRNG_PROD_RING_MISC_COMMON_EXT_PHYS(x)                                                ((x) + 0x1414)
#define HWIO_WBM_R0_MULTI_SRNG_PROD_RING_MISC_COMMON_EXT_OFFS                                                   (0x1414)
#define HWIO_WBM_R0_MULTI_SRNG_PROD_RING_MISC_COMMON_EXT_RMSK                                                     0xffffff
#define HWIO_WBM_R0_MULTI_SRNG_PROD_RING_MISC_COMMON_EXT_POR                                                    0x00000fff
#define HWIO_WBM_R0_MULTI_SRNG_PROD_RING_MISC_COMMON_EXT_POR_RMSK                                               0xffffffff
#define HWIO_WBM_R0_MULTI_SRNG_PROD_RING_MISC_COMMON_EXT_ATTR                                                                0x3
#define HWIO_WBM_R0_MULTI_SRNG_PROD_RING_MISC_COMMON_EXT_IN(x)            \
                in_dword(HWIO_WBM_R0_MULTI_SRNG_PROD_RING_MISC_COMMON_EXT_ADDR(x))
#define HWIO_WBM_R0_MULTI_SRNG_PROD_RING_MISC_COMMON_EXT_INM(x, m)            \
                in_dword_masked(HWIO_WBM_R0_MULTI_SRNG_PROD_RING_MISC_COMMON_EXT_ADDR(x), m)
#define HWIO_WBM_R0_MULTI_SRNG_PROD_RING_MISC_COMMON_EXT_OUT(x, v)            \
                out_dword(HWIO_WBM_R0_MULTI_SRNG_PROD_RING_MISC_COMMON_EXT_ADDR(x),v)
#define HWIO_WBM_R0_MULTI_SRNG_PROD_RING_MISC_COMMON_EXT_OUTM(x,m,v) \
                out_dword_masked_ns(HWIO_WBM_R0_MULTI_SRNG_PROD_RING_MISC_COMMON_EXT_ADDR(x),m,v,HWIO_WBM_R0_MULTI_SRNG_PROD_RING_MISC_COMMON_EXT_IN(x))
#define HWIO_WBM_R0_MULTI_SRNG_PROD_RING_MISC_COMMON_EXT_RING_WATCHDOG_TIMER_STATUS_BMSK                          0xfff000
#define HWIO_WBM_R0_MULTI_SRNG_PROD_RING_MISC_COMMON_EXT_RING_WATCHDOG_TIMER_STATUS_SHFT                                12
#define HWIO_WBM_R0_MULTI_SRNG_PROD_RING_MISC_COMMON_EXT_WATCHDOG_TIMEOUT_VALUE_BMSK                                 0xfff
#define HWIO_WBM_R0_MULTI_SRNG_PROD_RING_MISC_COMMON_EXT_WATCHDOG_TIMEOUT_VALUE_SHFT                                     0

#define HWIO_WBM_R0_LPM_FW_CTRL_ADDR(x)                                                                         ((x) + 0x1418)
#define HWIO_WBM_R0_LPM_FW_CTRL_PHYS(x)                                                                         ((x) + 0x1418)
#define HWIO_WBM_R0_LPM_FW_CTRL_OFFS                                                                            (0x1418)
#define HWIO_WBM_R0_LPM_FW_CTRL_RMSK                                                                                  0x3f
#define HWIO_WBM_R0_LPM_FW_CTRL_POR                                                                             0x00000000
#define HWIO_WBM_R0_LPM_FW_CTRL_POR_RMSK                                                                        0xffffffff
#define HWIO_WBM_R0_LPM_FW_CTRL_ATTR                                                                                         0x3
#define HWIO_WBM_R0_LPM_FW_CTRL_IN(x)            \
                in_dword(HWIO_WBM_R0_LPM_FW_CTRL_ADDR(x))
#define HWIO_WBM_R0_LPM_FW_CTRL_INM(x, m)            \
                in_dword_masked(HWIO_WBM_R0_LPM_FW_CTRL_ADDR(x), m)
#define HWIO_WBM_R0_LPM_FW_CTRL_OUT(x, v)            \
                out_dword(HWIO_WBM_R0_LPM_FW_CTRL_ADDR(x),v)
#define HWIO_WBM_R0_LPM_FW_CTRL_OUTM(x,m,v) \
                out_dword_masked_ns(HWIO_WBM_R0_LPM_FW_CTRL_ADDR(x),m,v,HWIO_WBM_R0_LPM_FW_CTRL_IN(x))
#define HWIO_WBM_R0_LPM_FW_CTRL_SLEEP_REQ_ACK_OVR_SRNG_P_BMSK                                                         0x20
#define HWIO_WBM_R0_LPM_FW_CTRL_SLEEP_REQ_ACK_OVR_SRNG_P_SHFT                                                            5
#define HWIO_WBM_R0_LPM_FW_CTRL_SLEEP_REQ_ACK_OVR_SRNG_C_BMSK                                                         0x10
#define HWIO_WBM_R0_LPM_FW_CTRL_SLEEP_REQ_ACK_OVR_SRNG_C_SHFT                                                            4
#define HWIO_WBM_R0_LPM_FW_CTRL_SLEEP_REQ_ACK_OVR_MULTI_SRNG_P_BMSK                                                    0x8
#define HWIO_WBM_R0_LPM_FW_CTRL_SLEEP_REQ_ACK_OVR_MULTI_SRNG_P_SHFT                                                      3
#define HWIO_WBM_R0_LPM_FW_CTRL_SLEEP_REQ_ACK_OVR_MULTI_SRNG_C_BMSK                                                    0x4
#define HWIO_WBM_R0_LPM_FW_CTRL_SLEEP_REQ_ACK_OVR_MULTI_SRNG_C_SHFT                                                      2
#define HWIO_WBM_R0_LPM_FW_CTRL_SLEEP_REQ_ACK_OVR_TOP_BMSK                                                             0x2
#define HWIO_WBM_R0_LPM_FW_CTRL_SLEEP_REQ_ACK_OVR_TOP_SHFT                                                               1
#define HWIO_WBM_R0_LPM_FW_CTRL_SLEEP_REQ_BMSK                                                                         0x1
#define HWIO_WBM_R0_LPM_FW_CTRL_SLEEP_REQ_SHFT                                                                           0

#define HWIO_WBM_R0_IDLE_SEQ_FIFO_CTRL_ADDR(x)                                                                  ((x) + 0x141c)
#define HWIO_WBM_R0_IDLE_SEQ_FIFO_CTRL_PHYS(x)                                                                  ((x) + 0x141c)
#define HWIO_WBM_R0_IDLE_SEQ_FIFO_CTRL_OFFS                                                                     (0x141c)
#define HWIO_WBM_R0_IDLE_SEQ_FIFO_CTRL_RMSK                                                                     0xffffffff
#define HWIO_WBM_R0_IDLE_SEQ_FIFO_CTRL_POR                                                                      0x00000000
#define HWIO_WBM_R0_IDLE_SEQ_FIFO_CTRL_POR_RMSK                                                                 0xffffffff
#define HWIO_WBM_R0_IDLE_SEQ_FIFO_CTRL_ATTR                                                                                  0x3
#define HWIO_WBM_R0_IDLE_SEQ_FIFO_CTRL_IN(x)            \
                in_dword(HWIO_WBM_R0_IDLE_SEQ_FIFO_CTRL_ADDR(x))
#define HWIO_WBM_R0_IDLE_SEQ_FIFO_CTRL_INM(x, m)            \
                in_dword_masked(HWIO_WBM_R0_IDLE_SEQ_FIFO_CTRL_ADDR(x), m)
#define HWIO_WBM_R0_IDLE_SEQ_FIFO_CTRL_OUT(x, v)            \
                out_dword(HWIO_WBM_R0_IDLE_SEQ_FIFO_CTRL_ADDR(x),v)
#define HWIO_WBM_R0_IDLE_SEQ_FIFO_CTRL_OUTM(x,m,v) \
                out_dword_masked_ns(HWIO_WBM_R0_IDLE_SEQ_FIFO_CTRL_ADDR(x),m,v,HWIO_WBM_R0_IDLE_SEQ_FIFO_CTRL_IN(x))
#define HWIO_WBM_R0_IDLE_SEQ_FIFO_CTRL_MISC_SPARE_BMSK                                                          0xffe00000
#define HWIO_WBM_R0_IDLE_SEQ_FIFO_CTRL_MISC_SPARE_SHFT                                                                  21
#define HWIO_WBM_R0_IDLE_SEQ_FIFO_CTRL_CMD_UD_CNT_BMSK                                                            0x1f0000
#define HWIO_WBM_R0_IDLE_SEQ_FIFO_CTRL_CMD_UD_CNT_SHFT                                                                  16
#define HWIO_WBM_R0_IDLE_SEQ_FIFO_CTRL_UD_CNT_BMSK                                                                  0xf800
#define HWIO_WBM_R0_IDLE_SEQ_FIFO_CTRL_UD_CNT_SHFT                                                                      11
#define HWIO_WBM_R0_IDLE_SEQ_FIFO_CTRL_WR_PTR_BMSK                                                                   0x7c0
#define HWIO_WBM_R0_IDLE_SEQ_FIFO_CTRL_WR_PTR_SHFT                                                                       6
#define HWIO_WBM_R0_IDLE_SEQ_FIFO_CTRL_RD_PTR_BMSK                                                                    0x3e
#define HWIO_WBM_R0_IDLE_SEQ_FIFO_CTRL_RD_PTR_SHFT                                                                       1
#define HWIO_WBM_R0_IDLE_SEQ_FIFO_CTRL_LOAD_BMSK                                                                       0x1
#define HWIO_WBM_R0_IDLE_SEQ_FIFO_CTRL_LOAD_SHFT                                                                         0

#define HWIO_WBM_R1_LPM_REQ_HANDLER_STATUS_ADDR(x)                                                              ((x) + 0x2030)
#define HWIO_WBM_R1_LPM_REQ_HANDLER_STATUS_PHYS(x)                                                              ((x) + 0x2030)
#define HWIO_WBM_R1_LPM_REQ_HANDLER_STATUS_OFFS                                                                 (0x2030)
#define HWIO_WBM_R1_LPM_REQ_HANDLER_STATUS_RMSK                                                                      0x3ff
#define HWIO_WBM_R1_LPM_REQ_HANDLER_STATUS_POR                                                                  0x00000000
#define HWIO_WBM_R1_LPM_REQ_HANDLER_STATUS_POR_RMSK                                                             0xffffffff
#define HWIO_WBM_R1_LPM_REQ_HANDLER_STATUS_ATTR                                                                              0x1
#define HWIO_WBM_R1_LPM_REQ_HANDLER_STATUS_IN(x)            \
                in_dword(HWIO_WBM_R1_LPM_REQ_HANDLER_STATUS_ADDR(x))
#define HWIO_WBM_R1_LPM_REQ_HANDLER_STATUS_INM(x, m)            \
                in_dword_masked(HWIO_WBM_R1_LPM_REQ_HANDLER_STATUS_ADDR(x), m)
#define HWIO_WBM_R1_LPM_REQ_HANDLER_STATUS_SLEEP_REQ_ACK_SRNG_P_BMSK                                                 0x200
#define HWIO_WBM_R1_LPM_REQ_HANDLER_STATUS_SLEEP_REQ_ACK_SRNG_P_SHFT                                                     9
#define HWIO_WBM_R1_LPM_REQ_HANDLER_STATUS_SLEEP_REQ_ACK_SRNG_C_BMSK                                                 0x100
#define HWIO_WBM_R1_LPM_REQ_HANDLER_STATUS_SLEEP_REQ_ACK_SRNG_C_SHFT                                                     8
#define HWIO_WBM_R1_LPM_REQ_HANDLER_STATUS_SLEEP_REQ_ACK_MULTI_SRNG_P_BMSK                                            0x80
#define HWIO_WBM_R1_LPM_REQ_HANDLER_STATUS_SLEEP_REQ_ACK_MULTI_SRNG_P_SHFT                                               7
#define HWIO_WBM_R1_LPM_REQ_HANDLER_STATUS_SLEEP_REQ_ACK_MULTI_SRNG_C_BMSK                                            0x40
#define HWIO_WBM_R1_LPM_REQ_HANDLER_STATUS_SLEEP_REQ_ACK_MULTI_SRNG_C_SHFT                                               6
#define HWIO_WBM_R1_LPM_REQ_HANDLER_STATUS_SLEEP_REQ_ACK_TOP_BMSK                                                     0x20
#define HWIO_WBM_R1_LPM_REQ_HANDLER_STATUS_SLEEP_REQ_ACK_TOP_SHFT                                                        5
#define HWIO_WBM_R1_LPM_REQ_HANDLER_STATUS_SLEEP_REQ_ACK_TO_LPM_BMSK                                                  0x10
#define HWIO_WBM_R1_LPM_REQ_HANDLER_STATUS_SLEEP_REQ_ACK_TO_LPM_SHFT                                                     4
#define HWIO_WBM_R1_LPM_REQ_HANDLER_STATUS_SLEEP_REQ_FROM_LPM_BMSK                                                     0x8
#define HWIO_WBM_R1_LPM_REQ_HANDLER_STATUS_SLEEP_REQ_FROM_LPM_SHFT                                                       3
#define HWIO_WBM_R1_LPM_REQ_HANDLER_STATUS_SM_STATE_BMSK                                                               0x7
#define HWIO_WBM_R1_LPM_REQ_HANDLER_STATUS_SM_STATE_SHFT                                                                 0

#define HWIO_WBM_R1_TESTBUS_CAPTURE_n_ADDR(base,n)                                                              ((base) + 0X2034 + (0x4*(n)))
#define HWIO_WBM_R1_TESTBUS_CAPTURE_n_PHYS(base,n)                                                              ((base) + 0X2034 + (0x4*(n)))
#define HWIO_WBM_R1_TESTBUS_CAPTURE_n_OFFS(n)                                                                   (0X2034 + (0x4*(n)))
#define HWIO_WBM_R1_TESTBUS_CAPTURE_n_RMSK                                                                      0xffffffff
#define HWIO_WBM_R1_TESTBUS_CAPTURE_n_MAXn                                                                             255
#define HWIO_WBM_R1_TESTBUS_CAPTURE_n_POR                                                                       0x00000000
#define HWIO_WBM_R1_TESTBUS_CAPTURE_n_POR_RMSK                                                                  0xffffffff
#define HWIO_WBM_R1_TESTBUS_CAPTURE_n_ATTR                                                                                   0x1
#define HWIO_WBM_R1_TESTBUS_CAPTURE_n_INI(base,n)                \
                in_dword_masked(HWIO_WBM_R1_TESTBUS_CAPTURE_n_ADDR(base,n), HWIO_WBM_R1_TESTBUS_CAPTURE_n_RMSK)
#define HWIO_WBM_R1_TESTBUS_CAPTURE_n_INMI(base,n,mask)        \
                in_dword_masked(HWIO_WBM_R1_TESTBUS_CAPTURE_n_ADDR(base,n), mask)
#define HWIO_WBM_R1_TESTBUS_CAPTURE_n_DATA_BMSK                                                                 0xffffffff
#define HWIO_WBM_R1_TESTBUS_CAPTURE_n_DATA_SHFT                                                                          0

#define HWIO_WBM_R2_SW_RELEASE_RING_HP_ADDR(x)                                                                  ((x) + 0x3010)
#define HWIO_WBM_R2_WBM_IDLE_LINK_RING_HP_ADDR(x)                                                               ((x) + 0x30b8)
#define HWIO_WBM_R2_WBM2SW0_RELEASE_RING_HP_ADDR(x)                                                             ((x) + 0x30c8)
#define HWIO_WBM_R2_WBM2SW1_RELEASE_RING_HP_ADDR(x)                                                             ((x) + 0x30d0)
#define REO_REG_REG_BASE                                                                                               (UMAC_BASE      + 0x00038000)
#define HWIO_REO_R0_GENERAL_ENABLE_ADDR(x)                                                                             ((x) + 0x0)
#define HWIO_REO_R0_GENERAL_ENABLE_AGING_FLUSH_ENABLE_BMSK                                                                    0x8
#define HWIO_REO_R0_GENERAL_ENABLE_AGING_FLUSH_ENABLE_SHFT                                                                      3
#define HWIO_REO_R0_GENERAL_ENABLE_AGING_LIST_ENABLE_BMSK                                                                     0x4
#define HWIO_REO_R0_GENERAL_ENABLE_AGING_LIST_ENABLE_SHFT                                                                       2
#define HWIO_REO_R0_DESTINATION_RING_CTRL_IX_0_DEST_RING_MAPPING_7_SHFT                                                        28
#define HWIO_REO_R0_DESTINATION_RING_CTRL_IX_0_DEST_RING_MAPPING_6_SHFT                                                        24
#define HWIO_REO_R0_DESTINATION_RING_CTRL_IX_0_DEST_RING_MAPPING_5_SHFT                                                        20
#define HWIO_REO_R0_DESTINATION_RING_CTRL_IX_0_DEST_RING_MAPPING_4_SHFT                                                        16
#define HWIO_REO_R0_DESTINATION_RING_CTRL_IX_0_DEST_RING_MAPPING_3_SHFT                                                        12
#define HWIO_REO_R0_DESTINATION_RING_CTRL_IX_0_DEST_RING_MAPPING_2_SHFT                                                         8
#define HWIO_REO_R0_DESTINATION_RING_CTRL_IX_0_DEST_RING_MAPPING_1_SHFT                                                         4
#define HWIO_REO_R0_DESTINATION_RING_CTRL_IX_0_DEST_RING_MAPPING_0_SHFT                                                         0

#define HWIO_REO_R0_DESTINATION_RING_CTRL_IX_2_ADDR(x)                                                                 ((x) + 0xc)
#define HWIO_REO_R0_DESTINATION_RING_CTRL_IX_2_DEST_RING_MAPPING_23_SHFT                                                       28
#define HWIO_REO_R0_DESTINATION_RING_CTRL_IX_2_DEST_RING_MAPPING_22_SHFT                                                       24
#define HWIO_REO_R0_DESTINATION_RING_CTRL_IX_2_DEST_RING_MAPPING_21_SHFT                                                       20
#define HWIO_REO_R0_DESTINATION_RING_CTRL_IX_2_DEST_RING_MAPPING_20_SHFT                                                       16
#define HWIO_REO_R0_DESTINATION_RING_CTRL_IX_2_DEST_RING_MAPPING_19_SHFT                                                       12
#define HWIO_REO_R0_DESTINATION_RING_CTRL_IX_2_DEST_RING_MAPPING_18_SHFT                                                        8
#define HWIO_REO_R0_DESTINATION_RING_CTRL_IX_2_DEST_RING_MAPPING_17_SHFT                                                        4
#define HWIO_REO_R0_DESTINATION_RING_CTRL_IX_2_DEST_RING_MAPPING_16_SHFT                                                        0

#define HWIO_REO_R0_DESTINATION_RING_CTRL_IX_3_ADDR(x)                                                                 ((x) + 0x10)
#define HWIO_REO_R0_DESTINATION_RING_CTRL_IX_3_DEST_RING_MAPPING_31_SHFT                                                       28
#define HWIO_REO_R0_DESTINATION_RING_CTRL_IX_3_DEST_RING_MAPPING_30_SHFT                                                       24
#define HWIO_REO_R0_DESTINATION_RING_CTRL_IX_3_DEST_RING_MAPPING_29_SHFT                                                       20
#define HWIO_REO_R0_DESTINATION_RING_CTRL_IX_3_DEST_RING_MAPPING_28_SHFT                                                       16
#define HWIO_REO_R0_DESTINATION_RING_CTRL_IX_3_DEST_RING_MAPPING_27_SHFT                                                       12
#define HWIO_REO_R0_DESTINATION_RING_CTRL_IX_3_DEST_RING_MAPPING_26_SHFT                                                        8
#define HWIO_REO_R0_DESTINATION_RING_CTRL_IX_3_DEST_RING_MAPPING_25_SHFT                                                        4
#define HWIO_REO_R0_DESTINATION_RING_CTRL_IX_3_DEST_RING_MAPPING_24_SHFT                                                        0

#define HWIO_REO_R0_DESTINATION_RING_CTRL_TID_BASED_IX_0_ADDR(x)                                                       ((x) + 0x14)
#define HWIO_REO_R0_DESTINATION_RING_CTRL_TID_BASED_IX_0_PHYS(x)                                                       ((x) + 0x14)
#define HWIO_REO_R0_DESTINATION_RING_CTRL_TID_BASED_IX_0_OFFS                                                          (0x14)
#define HWIO_REO_R0_DESTINATION_RING_CTRL_TID_BASED_IX_0_RMSK                                                          0xffffffff
#define HWIO_REO_R0_DESTINATION_RING_CTRL_TID_BASED_IX_0_POR                                                           0x76543210
#define HWIO_REO_R0_DESTINATION_RING_CTRL_TID_BASED_IX_0_POR_RMSK                                                      0xffffffff
#define HWIO_REO_R0_DESTINATION_RING_CTRL_TID_BASED_IX_0_ATTR                                                                       0x3
#define HWIO_REO_R0_DESTINATION_RING_CTRL_TID_BASED_IX_0_IN(x)            \
                in_dword(HWIO_REO_R0_DESTINATION_RING_CTRL_TID_BASED_IX_0_ADDR(x))
#define HWIO_REO_R0_DESTINATION_RING_CTRL_TID_BASED_IX_0_INM(x, m)            \
                in_dword_masked(HWIO_REO_R0_DESTINATION_RING_CTRL_TID_BASED_IX_0_ADDR(x), m)
#define HWIO_REO_R0_DESTINATION_RING_CTRL_TID_BASED_IX_0_OUT(x, v)            \
                out_dword(HWIO_REO_R0_DESTINATION_RING_CTRL_TID_BASED_IX_0_ADDR(x),v)
#define HWIO_REO_R0_DESTINATION_RING_CTRL_TID_BASED_IX_0_OUTM(x,m,v) \
                out_dword_masked_ns(HWIO_REO_R0_DESTINATION_RING_CTRL_TID_BASED_IX_0_ADDR(x),m,v,HWIO_REO_R0_DESTINATION_RING_CTRL_TID_BASED_IX_0_IN(x))
#define HWIO_REO_R0_DESTINATION_RING_CTRL_TID_BASED_IX_0_DEST_RING_MAPPING_TID_BASED_7_BMSK                            0xf0000000
#define HWIO_REO_R0_DESTINATION_RING_CTRL_TID_BASED_IX_0_DEST_RING_MAPPING_TID_BASED_7_SHFT                                    28
#define HWIO_REO_R0_DESTINATION_RING_CTRL_TID_BASED_IX_0_DEST_RING_MAPPING_TID_BASED_6_BMSK                             0xf000000
#define HWIO_REO_R0_DESTINATION_RING_CTRL_TID_BASED_IX_0_DEST_RING_MAPPING_TID_BASED_6_SHFT                                    24
#define HWIO_REO_R0_DESTINATION_RING_CTRL_TID_BASED_IX_0_DEST_RING_MAPPING_TID_BASED_5_BMSK                              0xf00000
#define HWIO_REO_R0_DESTINATION_RING_CTRL_TID_BASED_IX_0_DEST_RING_MAPPING_TID_BASED_5_SHFT                                    20
#define HWIO_REO_R0_DESTINATION_RING_CTRL_TID_BASED_IX_0_DEST_RING_MAPPING_TID_BASED_4_BMSK                               0xf0000
#define HWIO_REO_R0_DESTINATION_RING_CTRL_TID_BASED_IX_0_DEST_RING_MAPPING_TID_BASED_4_SHFT                                    16
#define HWIO_REO_R0_DESTINATION_RING_CTRL_TID_BASED_IX_0_DEST_RING_MAPPING_TID_BASED_3_BMSK                                0xf000
#define HWIO_REO_R0_DESTINATION_RING_CTRL_TID_BASED_IX_0_DEST_RING_MAPPING_TID_BASED_3_SHFT                                    12
#define HWIO_REO_R0_DESTINATION_RING_CTRL_TID_BASED_IX_0_DEST_RING_MAPPING_TID_BASED_2_BMSK                                 0xf00
#define HWIO_REO_R0_DESTINATION_RING_CTRL_TID_BASED_IX_0_DEST_RING_MAPPING_TID_BASED_2_SHFT                                     8
#define HWIO_REO_R0_DESTINATION_RING_CTRL_TID_BASED_IX_0_DEST_RING_MAPPING_TID_BASED_1_BMSK                                  0xf0
#define HWIO_REO_R0_DESTINATION_RING_CTRL_TID_BASED_IX_0_DEST_RING_MAPPING_TID_BASED_1_SHFT                                     4
#define HWIO_REO_R0_DESTINATION_RING_CTRL_TID_BASED_IX_0_DEST_RING_MAPPING_TID_BASED_0_BMSK                                   0xf
#define HWIO_REO_R0_DESTINATION_RING_CTRL_TID_BASED_IX_0_DEST_RING_MAPPING_TID_BASED_0_SHFT                                     0

#define HWIO_REO_R0_DESTINATION_RING_CTRL_TID_BASED_IX_1_ADDR(x)                                                       ((x) + 0x18)
#define HWIO_REO_R0_DESTINATION_RING_CTRL_TID_BASED_IX_1_PHYS(x)                                                       ((x) + 0x18)
#define HWIO_REO_R0_DESTINATION_RING_CTRL_TID_BASED_IX_1_OFFS                                                          (0x18)
#define HWIO_REO_R0_DESTINATION_RING_CTRL_TID_BASED_IX_1_RMSK                                                          0xffffffff
#define HWIO_REO_R0_DESTINATION_RING_CTRL_TID_BASED_IX_1_POR                                                           0x66666a98
#define HWIO_REO_R0_DESTINATION_RING_CTRL_TID_BASED_IX_1_POR_RMSK                                                      0xffffffff
#define HWIO_REO_R0_DESTINATION_RING_CTRL_TID_BASED_IX_1_ATTR                                                                       0x3
#define HWIO_REO_R0_DESTINATION_RING_CTRL_TID_BASED_IX_1_IN(x)            \
                in_dword(HWIO_REO_R0_DESTINATION_RING_CTRL_TID_BASED_IX_1_ADDR(x))
#define HWIO_REO_R0_DESTINATION_RING_CTRL_TID_BASED_IX_1_INM(x, m)            \
                in_dword_masked(HWIO_REO_R0_DESTINATION_RING_CTRL_TID_BASED_IX_1_ADDR(x), m)
#define HWIO_REO_R0_DESTINATION_RING_CTRL_TID_BASED_IX_1_OUT(x, v)            \
                out_dword(HWIO_REO_R0_DESTINATION_RING_CTRL_TID_BASED_IX_1_ADDR(x),v)
#define HWIO_REO_R0_DESTINATION_RING_CTRL_TID_BASED_IX_1_OUTM(x,m,v) \
                out_dword_masked_ns(HWIO_REO_R0_DESTINATION_RING_CTRL_TID_BASED_IX_1_ADDR(x),m,v,HWIO_REO_R0_DESTINATION_RING_CTRL_TID_BASED_IX_1_IN(x))
#define HWIO_REO_R0_DESTINATION_RING_CTRL_TID_BASED_IX_1_DEST_RING_MAPPING_TID_BASED_15_BMSK                           0xf0000000
#define HWIO_REO_R0_DESTINATION_RING_CTRL_TID_BASED_IX_1_DEST_RING_MAPPING_TID_BASED_15_SHFT                                   28
#define HWIO_REO_R0_DESTINATION_RING_CTRL_TID_BASED_IX_1_DEST_RING_MAPPING_TID_BASED_14_BMSK                            0xf000000
#define HWIO_REO_R0_DESTINATION_RING_CTRL_TID_BASED_IX_1_DEST_RING_MAPPING_TID_BASED_14_SHFT                                   24
#define HWIO_REO_R0_DESTINATION_RING_CTRL_TID_BASED_IX_1_DEST_RING_MAPPING_TID_BASED_13_BMSK                             0xf00000
#define HWIO_REO_R0_DESTINATION_RING_CTRL_TID_BASED_IX_1_DEST_RING_MAPPING_TID_BASED_13_SHFT                                   20
#define HWIO_REO_R0_DESTINATION_RING_CTRL_TID_BASED_IX_1_DEST_RING_MAPPING_TID_BASED_12_BMSK                              0xf0000
#define HWIO_REO_R0_DESTINATION_RING_CTRL_TID_BASED_IX_1_DEST_RING_MAPPING_TID_BASED_12_SHFT                                   16
#define HWIO_REO_R0_DESTINATION_RING_CTRL_TID_BASED_IX_1_DEST_RING_MAPPING_TID_BASED_11_BMSK                               0xf000
#define HWIO_REO_R0_DESTINATION_RING_CTRL_TID_BASED_IX_1_DEST_RING_MAPPING_TID_BASED_11_SHFT                                   12
#define HWIO_REO_R0_DESTINATION_RING_CTRL_TID_BASED_IX_1_DEST_RING_MAPPING_TID_BASED_10_BMSK                                0xf00
#define HWIO_REO_R0_DESTINATION_RING_CTRL_TID_BASED_IX_1_DEST_RING_MAPPING_TID_BASED_10_SHFT                                    8
#define HWIO_REO_R0_DESTINATION_RING_CTRL_TID_BASED_IX_1_DEST_RING_MAPPING_TID_BASED_9_BMSK                                  0xf0
#define HWIO_REO_R0_DESTINATION_RING_CTRL_TID_BASED_IX_1_DEST_RING_MAPPING_TID_BASED_9_SHFT                                     4
#define HWIO_REO_R0_DESTINATION_RING_CTRL_TID_BASED_IX_1_DEST_RING_MAPPING_TID_BASED_8_BMSK                                   0xf
#define HWIO_REO_R0_DESTINATION_RING_CTRL_TID_BASED_IX_1_DEST_RING_MAPPING_TID_BASED_8_SHFT                                     0

#define HWIO_REO_R0_DESTINATION_RING_CTRL_TID_BASED_IX_2_ADDR(x)                                                       ((x) + 0x1c)
#define HWIO_REO_R0_DESTINATION_RING_CTRL_TID_BASED_IX_2_PHYS(x)                                                       ((x) + 0x1c)
#define HWIO_REO_R0_DESTINATION_RING_CTRL_TID_BASED_IX_2_OFFS                                                          (0x1c)
#define HWIO_REO_R0_DESTINATION_RING_CTRL_TID_BASED_IX_2_RMSK                                                          0xffffffff
#define HWIO_REO_R0_DESTINATION_RING_CTRL_TID_BASED_IX_2_POR                                                           0x66666666
#define HWIO_REO_R0_DESTINATION_RING_CTRL_TID_BASED_IX_2_POR_RMSK                                                      0xffffffff
#define HWIO_REO_R0_DESTINATION_RING_CTRL_TID_BASED_IX_2_ATTR                                                                       0x3
#define HWIO_REO_R0_DESTINATION_RING_CTRL_TID_BASED_IX_2_IN(x)            \
                in_dword(HWIO_REO_R0_DESTINATION_RING_CTRL_TID_BASED_IX_2_ADDR(x))
#define HWIO_REO_R0_DESTINATION_RING_CTRL_TID_BASED_IX_2_INM(x, m)            \
                in_dword_masked(HWIO_REO_R0_DESTINATION_RING_CTRL_TID_BASED_IX_2_ADDR(x), m)
#define HWIO_REO_R0_DESTINATION_RING_CTRL_TID_BASED_IX_2_OUT(x, v)            \
                out_dword(HWIO_REO_R0_DESTINATION_RING_CTRL_TID_BASED_IX_2_ADDR(x),v)
#define HWIO_REO_R0_DESTINATION_RING_CTRL_TID_BASED_IX_2_OUTM(x,m,v) \
                out_dword_masked_ns(HWIO_REO_R0_DESTINATION_RING_CTRL_TID_BASED_IX_2_ADDR(x),m,v,HWIO_REO_R0_DESTINATION_RING_CTRL_TID_BASED_IX_2_IN(x))
#define HWIO_REO_R0_DESTINATION_RING_CTRL_TID_BASED_IX_2_DEST_RING_MAPPING_TID_BASED_23_BMSK                           0xf0000000
#define HWIO_REO_R0_DESTINATION_RING_CTRL_TID_BASED_IX_2_DEST_RING_MAPPING_TID_BASED_23_SHFT                                   28
#define HWIO_REO_R0_DESTINATION_RING_CTRL_TID_BASED_IX_2_DEST_RING_MAPPING_TID_BASED_22_BMSK                            0xf000000
#define HWIO_REO_R0_DESTINATION_RING_CTRL_TID_BASED_IX_2_DEST_RING_MAPPING_TID_BASED_22_SHFT                                   24
#define HWIO_REO_R0_DESTINATION_RING_CTRL_TID_BASED_IX_2_DEST_RING_MAPPING_TID_BASED_21_BMSK                             0xf00000
#define HWIO_REO_R0_DESTINATION_RING_CTRL_TID_BASED_IX_2_DEST_RING_MAPPING_TID_BASED_21_SHFT                                   20
#define HWIO_REO_R0_DESTINATION_RING_CTRL_TID_BASED_IX_2_DEST_RING_MAPPING_TID_BASED_20_BMSK                              0xf0000
#define HWIO_REO_R0_DESTINATION_RING_CTRL_TID_BASED_IX_2_DEST_RING_MAPPING_TID_BASED_20_SHFT                                   16
#define HWIO_REO_R0_DESTINATION_RING_CTRL_TID_BASED_IX_2_DEST_RING_MAPPING_TID_BASED_19_BMSK                               0xf000
#define HWIO_REO_R0_DESTINATION_RING_CTRL_TID_BASED_IX_2_DEST_RING_MAPPING_TID_BASED_19_SHFT                                   12
#define HWIO_REO_R0_DESTINATION_RING_CTRL_TID_BASED_IX_2_DEST_RING_MAPPING_TID_BASED_18_BMSK                                0xf00
#define HWIO_REO_R0_DESTINATION_RING_CTRL_TID_BASED_IX_2_DEST_RING_MAPPING_TID_BASED_18_SHFT                                    8
#define HWIO_REO_R0_DESTINATION_RING_CTRL_TID_BASED_IX_2_DEST_RING_MAPPING_TID_BASED_17_BMSK                                 0xf0
#define HWIO_REO_R0_DESTINATION_RING_CTRL_TID_BASED_IX_2_DEST_RING_MAPPING_TID_BASED_17_SHFT                                    4
#define HWIO_REO_R0_DESTINATION_RING_CTRL_TID_BASED_IX_2_DEST_RING_MAPPING_TID_BASED_16_BMSK                                  0xf
#define HWIO_REO_R0_DESTINATION_RING_CTRL_TID_BASED_IX_2_DEST_RING_MAPPING_TID_BASED_16_SHFT                                    0

#define HWIO_REO_R0_DESTINATION_RING_CTRL_TID_BASED_IX_3_ADDR(x)                                                       ((x) + 0x20)
#define HWIO_REO_R0_DESTINATION_RING_CTRL_TID_BASED_IX_3_PHYS(x)                                                       ((x) + 0x20)
#define HWIO_REO_R0_DESTINATION_RING_CTRL_TID_BASED_IX_3_OFFS                                                          (0x20)
#define HWIO_REO_R0_DESTINATION_RING_CTRL_TID_BASED_IX_3_RMSK                                                          0xffffffff
#define HWIO_REO_R0_DESTINATION_RING_CTRL_TID_BASED_IX_3_POR                                                           0x66666666
#define HWIO_REO_R0_DESTINATION_RING_CTRL_TID_BASED_IX_3_POR_RMSK                                                      0xffffffff
#define HWIO_REO_R0_DESTINATION_RING_CTRL_TID_BASED_IX_3_ATTR                                                                       0x3
#define HWIO_REO_R0_DESTINATION_RING_CTRL_TID_BASED_IX_3_IN(x)            \
                in_dword(HWIO_REO_R0_DESTINATION_RING_CTRL_TID_BASED_IX_3_ADDR(x))
#define HWIO_REO_R0_DESTINATION_RING_CTRL_TID_BASED_IX_3_INM(x, m)            \
                in_dword_masked(HWIO_REO_R0_DESTINATION_RING_CTRL_TID_BASED_IX_3_ADDR(x), m)
#define HWIO_REO_R0_DESTINATION_RING_CTRL_TID_BASED_IX_3_OUT(x, v)            \
                out_dword(HWIO_REO_R0_DESTINATION_RING_CTRL_TID_BASED_IX_3_ADDR(x),v)
#define HWIO_REO_R0_DESTINATION_RING_CTRL_TID_BASED_IX_3_OUTM(x,m,v) \
                out_dword_masked_ns(HWIO_REO_R0_DESTINATION_RING_CTRL_TID_BASED_IX_3_ADDR(x),m,v,HWIO_REO_R0_DESTINATION_RING_CTRL_TID_BASED_IX_3_IN(x))
#define HWIO_REO_R0_DESTINATION_RING_CTRL_TID_BASED_IX_3_DEST_RING_MAPPING_TID_BASED_31_BMSK                           0xf0000000
#define HWIO_REO_R0_DESTINATION_RING_CTRL_TID_BASED_IX_3_DEST_RING_MAPPING_TID_BASED_31_SHFT                                   28
#define HWIO_REO_R0_DESTINATION_RING_CTRL_TID_BASED_IX_3_DEST_RING_MAPPING_TID_BASED_30_BMSK                            0xf000000
#define HWIO_REO_R0_DESTINATION_RING_CTRL_TID_BASED_IX_3_DEST_RING_MAPPING_TID_BASED_30_SHFT                                   24
#define HWIO_REO_R0_DESTINATION_RING_CTRL_TID_BASED_IX_3_DEST_RING_MAPPING_TID_BASED_29_BMSK                             0xf00000
#define HWIO_REO_R0_DESTINATION_RING_CTRL_TID_BASED_IX_3_DEST_RING_MAPPING_TID_BASED_29_SHFT                                   20
#define HWIO_REO_R0_DESTINATION_RING_CTRL_TID_BASED_IX_3_DEST_RING_MAPPING_TID_BASED_28_BMSK                              0xf0000
#define HWIO_REO_R0_DESTINATION_RING_CTRL_TID_BASED_IX_3_DEST_RING_MAPPING_TID_BASED_28_SHFT                                   16
#define HWIO_REO_R0_DESTINATION_RING_CTRL_TID_BASED_IX_3_DEST_RING_MAPPING_TID_BASED_27_BMSK                               0xf000
#define HWIO_REO_R0_DESTINATION_RING_CTRL_TID_BASED_IX_3_DEST_RING_MAPPING_TID_BASED_27_SHFT                                   12
#define HWIO_REO_R0_DESTINATION_RING_CTRL_TID_BASED_IX_3_DEST_RING_MAPPING_TID_BASED_26_BMSK                                0xf00
#define HWIO_REO_R0_DESTINATION_RING_CTRL_TID_BASED_IX_3_DEST_RING_MAPPING_TID_BASED_26_SHFT                                    8
#define HWIO_REO_R0_DESTINATION_RING_CTRL_TID_BASED_IX_3_DEST_RING_MAPPING_TID_BASED_25_BMSK                                 0xf0
#define HWIO_REO_R0_DESTINATION_RING_CTRL_TID_BASED_IX_3_DEST_RING_MAPPING_TID_BASED_25_SHFT                                    4
#define HWIO_REO_R0_DESTINATION_RING_CTRL_TID_BASED_IX_3_DEST_RING_MAPPING_TID_BASED_24_BMSK                                  0xf
#define HWIO_REO_R0_DESTINATION_RING_CTRL_TID_BASED_IX_3_DEST_RING_MAPPING_TID_BASED_24_SHFT                                    0

#define HWIO_REO_R0_ERROR_DESTINATION_MAPPING_IX_0_ADDR(x)                                                             ((x) + 0x38)
#define HWIO_REO_R0_ERROR_DESTINATION_MAPPING_IX_0_ERROR_DESTINATION_RING_7_BMSK                                       0xf0000000
#define HWIO_REO_R0_ERROR_DESTINATION_MAPPING_IX_0_ERROR_DESTINATION_RING_7_SHFT                                               28
#define HWIO_REO_R0_ERROR_DESTINATION_MAPPING_IX_0_ERROR_DESTINATION_RING_6_BMSK                                        0xf000000
#define HWIO_REO_R0_ERROR_DESTINATION_MAPPING_IX_0_ERROR_DESTINATION_RING_6_SHFT                                               24
#define HWIO_REO_R0_ERROR_DESTINATION_MAPPING_IX_0_ERROR_DESTINATION_RING_5_BMSK                                         0xf00000
#define HWIO_REO_R0_ERROR_DESTINATION_MAPPING_IX_0_ERROR_DESTINATION_RING_5_SHFT                                               20
#define HWIO_REO_R0_ERROR_DESTINATION_MAPPING_IX_0_ERROR_DESTINATION_RING_4_BMSK                                          0xf0000
#define HWIO_REO_R0_ERROR_DESTINATION_MAPPING_IX_0_ERROR_DESTINATION_RING_4_SHFT                                               16
#define HWIO_REO_R0_ERROR_DESTINATION_MAPPING_IX_0_ERROR_DESTINATION_RING_3_BMSK                                           0xf000
#define HWIO_REO_R0_ERROR_DESTINATION_MAPPING_IX_0_ERROR_DESTINATION_RING_3_SHFT                                               12
#define HWIO_REO_R0_ERROR_DESTINATION_MAPPING_IX_0_ERROR_DESTINATION_RING_2_BMSK                                            0xf00
#define HWIO_REO_R0_ERROR_DESTINATION_MAPPING_IX_0_ERROR_DESTINATION_RING_2_SHFT                                                8
#define HWIO_REO_R0_ERROR_DESTINATION_MAPPING_IX_0_ERROR_DESTINATION_RING_1_BMSK                                             0xf0
#define HWIO_REO_R0_ERROR_DESTINATION_MAPPING_IX_0_ERROR_DESTINATION_RING_1_SHFT                                                4
#define HWIO_REO_R0_ERROR_DESTINATION_MAPPING_IX_0_ERROR_DESTINATION_RING_0_BMSK                                              0xf
#define HWIO_REO_R0_ERROR_DESTINATION_MAPPING_IX_0_ERROR_DESTINATION_RING_0_SHFT                                                0

#define HWIO_REO_R0_ERROR_DESTINATION_MAPPING_IX_1_ADDR(x)                                                             ((x) + 0x3c)
#define HWIO_REO_R0_ERROR_DESTINATION_MAPPING_IX_1_ERROR_DESTINATION_RING_14_SHFT                                              24
#define HWIO_REO_R0_ERROR_DESTINATION_MAPPING_IX_1_ERROR_DESTINATION_RING_13_SHFT                                              20
#define HWIO_REO_R0_ERROR_DESTINATION_MAPPING_IX_1_ERROR_DESTINATION_RING_12_SHFT                                              16
#define HWIO_REO_R0_ERROR_DESTINATION_MAPPING_IX_1_ERROR_DESTINATION_RING_11_SHFT                                              12
#define HWIO_REO_R0_ERROR_DESTINATION_MAPPING_IX_1_ERROR_DESTINATION_RING_10_SHFT                                               8
#define HWIO_REO_R0_ERROR_DESTINATION_MAPPING_IX_1_ERROR_DESTINATION_RING_9_SHFT                                                4
#define HWIO_REO_R0_ERROR_DESTINATION_MAPPING_IX_1_ERROR_DESTINATION_RING_8_SHFT                                                0

#define HWIO_REO_R0_ERROR_DESTINATION_MAPPING_TID_BASED_IX_0_ADDR(x)                                                   ((x) + 0x40)
#define HWIO_REO_R0_ERROR_DESTINATION_MAPPING_TID_BASED_IX_0_PHYS(x)                                                   ((x) + 0x40)
#define HWIO_REO_R0_ERROR_DESTINATION_MAPPING_TID_BASED_IX_0_OFFS                                                      (0x40)
#define HWIO_REO_R0_ERROR_DESTINATION_MAPPING_TID_BASED_IX_0_RMSK                                                      0xffffffff
#define HWIO_REO_R0_ERROR_DESTINATION_MAPPING_TID_BASED_IX_0_POR                                                       0x55555555
#define HWIO_REO_R0_ERROR_DESTINATION_MAPPING_TID_BASED_IX_0_POR_RMSK                                                  0xffffffff
#define HWIO_REO_R0_ERROR_DESTINATION_MAPPING_TID_BASED_IX_0_ATTR                                                                   0x3
#define HWIO_REO_R0_ERROR_DESTINATION_MAPPING_TID_BASED_IX_0_IN(x)            \
                in_dword(HWIO_REO_R0_ERROR_DESTINATION_MAPPING_TID_BASED_IX_0_ADDR(x))
#define HWIO_REO_R0_ERROR_DESTINATION_MAPPING_TID_BASED_IX_0_INM(x, m)            \
                in_dword_masked(HWIO_REO_R0_ERROR_DESTINATION_MAPPING_TID_BASED_IX_0_ADDR(x), m)
#define HWIO_REO_R0_ERROR_DESTINATION_MAPPING_TID_BASED_IX_0_OUT(x, v)            \
                out_dword(HWIO_REO_R0_ERROR_DESTINATION_MAPPING_TID_BASED_IX_0_ADDR(x),v)
#define HWIO_REO_R0_ERROR_DESTINATION_MAPPING_TID_BASED_IX_0_OUTM(x,m,v) \
                out_dword_masked_ns(HWIO_REO_R0_ERROR_DESTINATION_MAPPING_TID_BASED_IX_0_ADDR(x),m,v,HWIO_REO_R0_ERROR_DESTINATION_MAPPING_TID_BASED_IX_0_IN(x))
#define HWIO_REO_R0_ERROR_DESTINATION_MAPPING_TID_BASED_IX_0_ERROR_DESTINATION_RING_TID_BASED_7_BMSK                   0xf0000000
#define HWIO_REO_R0_ERROR_DESTINATION_MAPPING_TID_BASED_IX_0_ERROR_DESTINATION_RING_TID_BASED_7_SHFT                           28
#define HWIO_REO_R0_ERROR_DESTINATION_MAPPING_TID_BASED_IX_0_ERROR_DESTINATION_RING_TID_BASED_6_BMSK                    0xf000000
#define HWIO_REO_R0_ERROR_DESTINATION_MAPPING_TID_BASED_IX_0_ERROR_DESTINATION_RING_TID_BASED_6_SHFT                           24
#define HWIO_REO_R0_ERROR_DESTINATION_MAPPING_TID_BASED_IX_0_ERROR_DESTINATION_RING_TID_BASED_5_BMSK                     0xf00000
#define HWIO_REO_R0_ERROR_DESTINATION_MAPPING_TID_BASED_IX_0_ERROR_DESTINATION_RING_TID_BASED_5_SHFT                           20
#define HWIO_REO_R0_ERROR_DESTINATION_MAPPING_TID_BASED_IX_0_ERROR_DESTINATION_RING_TID_BASED_4_BMSK                      0xf0000
#define HWIO_REO_R0_ERROR_DESTINATION_MAPPING_TID_BASED_IX_0_ERROR_DESTINATION_RING_TID_BASED_4_SHFT                           16
#define HWIO_REO_R0_ERROR_DESTINATION_MAPPING_TID_BASED_IX_0_ERROR_DESTINATION_RING_TID_BASED_3_BMSK                       0xf000
#define HWIO_REO_R0_ERROR_DESTINATION_MAPPING_TID_BASED_IX_0_ERROR_DESTINATION_RING_TID_BASED_3_SHFT                           12
#define HWIO_REO_R0_ERROR_DESTINATION_MAPPING_TID_BASED_IX_0_ERROR_DESTINATION_RING_TID_BASED_2_BMSK                        0xf00
#define HWIO_REO_R0_ERROR_DESTINATION_MAPPING_TID_BASED_IX_0_ERROR_DESTINATION_RING_TID_BASED_2_SHFT                            8
#define HWIO_REO_R0_ERROR_DESTINATION_MAPPING_TID_BASED_IX_0_ERROR_DESTINATION_RING_TID_BASED_1_BMSK                         0xf0
#define HWIO_REO_R0_ERROR_DESTINATION_MAPPING_TID_BASED_IX_0_ERROR_DESTINATION_RING_TID_BASED_1_SHFT                            4
#define HWIO_REO_R0_ERROR_DESTINATION_MAPPING_TID_BASED_IX_0_ERROR_DESTINATION_RING_TID_BASED_0_BMSK                          0xf
#define HWIO_REO_R0_ERROR_DESTINATION_MAPPING_TID_BASED_IX_0_ERROR_DESTINATION_RING_TID_BASED_0_SHFT                            0

#define HWIO_REO_R0_ERROR_DESTINATION_MAPPING_TID_BASED_IX_1_ADDR(x)                                                   ((x) + 0x44)
#define HWIO_REO_R0_ERROR_DESTINATION_MAPPING_TID_BASED_IX_1_PHYS(x)                                                   ((x) + 0x44)
#define HWIO_REO_R0_ERROR_DESTINATION_MAPPING_TID_BASED_IX_1_OFFS                                                      (0x44)
#define HWIO_REO_R0_ERROR_DESTINATION_MAPPING_TID_BASED_IX_1_RMSK                                                      0xffffffff
#define HWIO_REO_R0_ERROR_DESTINATION_MAPPING_TID_BASED_IX_1_POR                                                       0x55555555
#define HWIO_REO_R0_ERROR_DESTINATION_MAPPING_TID_BASED_IX_1_POR_RMSK                                                  0xffffffff
#define HWIO_REO_R0_ERROR_DESTINATION_MAPPING_TID_BASED_IX_1_ATTR                                                                   0x3
#define HWIO_REO_R0_ERROR_DESTINATION_MAPPING_TID_BASED_IX_1_IN(x)            \
                in_dword(HWIO_REO_R0_ERROR_DESTINATION_MAPPING_TID_BASED_IX_1_ADDR(x))
#define HWIO_REO_R0_ERROR_DESTINATION_MAPPING_TID_BASED_IX_1_INM(x, m)            \
                in_dword_masked(HWIO_REO_R0_ERROR_DESTINATION_MAPPING_TID_BASED_IX_1_ADDR(x), m)
#define HWIO_REO_R0_ERROR_DESTINATION_MAPPING_TID_BASED_IX_1_OUT(x, v)            \
                out_dword(HWIO_REO_R0_ERROR_DESTINATION_MAPPING_TID_BASED_IX_1_ADDR(x),v)
#define HWIO_REO_R0_ERROR_DESTINATION_MAPPING_TID_BASED_IX_1_OUTM(x,m,v) \
                out_dword_masked_ns(HWIO_REO_R0_ERROR_DESTINATION_MAPPING_TID_BASED_IX_1_ADDR(x),m,v,HWIO_REO_R0_ERROR_DESTINATION_MAPPING_TID_BASED_IX_1_IN(x))
#define HWIO_REO_R0_ERROR_DESTINATION_MAPPING_TID_BASED_IX_1_ERROR_DESTINATION_RING_TID_BASED_OTHER_BMSK               0xf0000000
#define HWIO_REO_R0_ERROR_DESTINATION_MAPPING_TID_BASED_IX_1_ERROR_DESTINATION_RING_TID_BASED_OTHER_SHFT                       28
#define HWIO_REO_R0_ERROR_DESTINATION_MAPPING_TID_BASED_IX_1_ERROR_DESTINATION_RING_TID_BASED_14_BMSK                   0xf000000
#define HWIO_REO_R0_ERROR_DESTINATION_MAPPING_TID_BASED_IX_1_ERROR_DESTINATION_RING_TID_BASED_14_SHFT                          24
#define HWIO_REO_R0_ERROR_DESTINATION_MAPPING_TID_BASED_IX_1_ERROR_DESTINATION_RING_TID_BASED_13_BMSK                    0xf00000
#define HWIO_REO_R0_ERROR_DESTINATION_MAPPING_TID_BASED_IX_1_ERROR_DESTINATION_RING_TID_BASED_13_SHFT                          20
#define HWIO_REO_R0_ERROR_DESTINATION_MAPPING_TID_BASED_IX_1_ERROR_DESTINATION_RING_TID_BASED_12_BMSK                     0xf0000
#define HWIO_REO_R0_ERROR_DESTINATION_MAPPING_TID_BASED_IX_1_ERROR_DESTINATION_RING_TID_BASED_12_SHFT                          16
#define HWIO_REO_R0_ERROR_DESTINATION_MAPPING_TID_BASED_IX_1_ERROR_DESTINATION_RING_TID_BASED_11_BMSK                      0xf000
#define HWIO_REO_R0_ERROR_DESTINATION_MAPPING_TID_BASED_IX_1_ERROR_DESTINATION_RING_TID_BASED_11_SHFT                          12
#define HWIO_REO_R0_ERROR_DESTINATION_MAPPING_TID_BASED_IX_1_ERROR_DESTINATION_RING_TID_BASED_10_BMSK                       0xf00
#define HWIO_REO_R0_ERROR_DESTINATION_MAPPING_TID_BASED_IX_1_ERROR_DESTINATION_RING_TID_BASED_10_SHFT                           8
#define HWIO_REO_R0_ERROR_DESTINATION_MAPPING_TID_BASED_IX_1_ERROR_DESTINATION_RING_TID_BASED_9_BMSK                         0xf0
#define HWIO_REO_R0_ERROR_DESTINATION_MAPPING_TID_BASED_IX_1_ERROR_DESTINATION_RING_TID_BASED_9_SHFT                            4
#define HWIO_REO_R0_ERROR_DESTINATION_MAPPING_TID_BASED_IX_1_ERROR_DESTINATION_RING_TID_BASED_8_BMSK                          0xf
#define HWIO_REO_R0_ERROR_DESTINATION_MAPPING_TID_BASED_IX_1_ERROR_DESTINATION_RING_TID_BASED_8_SHFT                            0

#define HWIO_REO_R0_RDI_CTRL_SEL_WITH_TID_ADDR(x)                                                                      ((x) + 0x48)
#define HWIO_REO_R0_RDI_CTRL_SEL_WITH_TID_PHYS(x)                                                                      ((x) + 0x48)
#define HWIO_REO_R0_RDI_CTRL_SEL_WITH_TID_OFFS                                                                         (0x48)
#define HWIO_REO_R0_RDI_CTRL_SEL_WITH_TID_RMSK                                                                            0x1ffff
#define HWIO_REO_R0_RDI_CTRL_SEL_WITH_TID_POR                                                                          0x00000000
#define HWIO_REO_R0_RDI_CTRL_SEL_WITH_TID_POR_RMSK                                                                     0xffffffff
#define HWIO_REO_R0_RDI_CTRL_SEL_WITH_TID_ATTR                                                                                      0x3
#define HWIO_REO_R0_RDI_CTRL_SEL_WITH_TID_IN(x)            \
                in_dword(HWIO_REO_R0_RDI_CTRL_SEL_WITH_TID_ADDR(x))
#define HWIO_REO_R0_RDI_CTRL_SEL_WITH_TID_INM(x, m)            \
                in_dword_masked(HWIO_REO_R0_RDI_CTRL_SEL_WITH_TID_ADDR(x), m)
#define HWIO_REO_R0_RDI_CTRL_SEL_WITH_TID_OUT(x, v)            \
                out_dword(HWIO_REO_R0_RDI_CTRL_SEL_WITH_TID_ADDR(x),v)
#define HWIO_REO_R0_RDI_CTRL_SEL_WITH_TID_OUTM(x,m,v) \
                out_dword_masked_ns(HWIO_REO_R0_RDI_CTRL_SEL_WITH_TID_ADDR(x),m,v,HWIO_REO_R0_RDI_CTRL_SEL_WITH_TID_IN(x))
#define HWIO_REO_R0_RDI_CTRL_SEL_WITH_TID_TID_CTRL_BMSK                                                                   0x1ffff
#define HWIO_REO_R0_RDI_CTRL_SEL_WITH_TID_TID_CTRL_SHFT                                                                         0

#define HWIO_REO_R0_PN_IN_DEST_ADDR(x)                                                                                 ((x) + 0x68)
#define HWIO_REO_R0_SW_COOKIE_CFG0_ADDR(x)                                                                             ((x) + 0x6c)
#define HWIO_REO_R0_SW_COOKIE_CFG1_ADDR(x)                                                                             ((x) + 0x70)
#define HWIO_REO_R0_SW_COOKIE_CFG1_SW_COOKIE_CONVERT_GLOBAL_ENABLE_BMSK                                                  0x100000
#define HWIO_REO_R0_SW_COOKIE_CFG1_SW_COOKIE_CONVERT_GLOBAL_ENABLE_SHFT                                                        20
#define HWIO_REO_R0_SW_COOKIE_CFG1_SW_COOKIE_CONVERT_ENABLE_BMSK                                                          0x80000
#define HWIO_REO_R0_SW_COOKIE_CFG1_SW_COOKIE_CONVERT_ENABLE_SHFT                                                               19
#define HWIO_REO_R0_SW_COOKIE_CFG1_PAGE_ALIGNMENT_BMSK                                                                    0x40000
#define HWIO_REO_R0_SW_COOKIE_CFG1_PAGE_ALIGNMENT_SHFT                                                                         18
#define HWIO_REO_R0_SW_COOKIE_CFG1_COOKIE_OFFSET_MSB_BMSK                                                                 0x3e000
#define HWIO_REO_R0_SW_COOKIE_CFG1_COOKIE_OFFSET_MSB_SHFT                                                                      13
#define HWIO_REO_R0_SW_COOKIE_CFG1_COOKIE_PAGE_MSB_BMSK                                                                    0x1f00
#define HWIO_REO_R0_SW_COOKIE_CFG1_COOKIE_PAGE_MSB_SHFT                                                                         8
#define HWIO_REO_R0_SW_COOKIE_CFG1_CMEM_LUT_BASE_ADDR_39_32_BMSK                                                             0xff
#define HWIO_REO_R0_SW_COOKIE_CFG1_CMEM_LUT_BASE_ADDR_39_32_SHFT                                                                0

#define HWIO_REO_R0_RXDMA2REO0_RING_MISC_TRANSACTION_TYPE_BMSK                                                           0x400000
#define HWIO_REO_R0_RXDMA2REO0_RING_MISC_TRANSACTION_TYPE_SHFT                                                                 22
#define HWIO_REO_R0_WBM2REO_LINK_RING_MISC_TRANSACTION_TYPE_BMSK                                                         0x400000
#define HWIO_REO_R0_WBM2REO_LINK_RING_MISC_TRANSACTION_TYPE_SHFT                                                               22
#define HWIO_REO_R0_REO_CMD_RING_BASE_LSB_ADDR(x)                                                                      ((x) + 0x2a8)
#define HWIO_REO_R0_REO_CMD_RING_BASE_MSB_RING_SIZE_BMSK                                                                 0xffff00
#define HWIO_REO_R0_REO_CMD_RING_BASE_MSB_RING_SIZE_SHFT                                                                        8
#define HWIO_REO_R0_REO_CMD_RING_MISC_TRANSACTION_TYPE_BMSK                                                              0x400000
#define HWIO_REO_R0_REO_CMD_RING_MISC_TRANSACTION_TYPE_SHFT                                                                    22
#define HWIO_REO_R0_SW2REO_RING_BASE_LSB_ADDR(x)                                                                       ((x) + 0x320)
#define HWIO_REO_R0_SW2REO_RING_BASE_MSB_RING_SIZE_BMSK                                                                  0xffff00
#define HWIO_REO_R0_SW2REO_RING_BASE_MSB_RING_SIZE_SHFT                                                                         8
#define HWIO_REO_R0_SW2REO_RING_MISC_TRANSACTION_TYPE_BMSK                                                               0x400000
#define HWIO_REO_R0_SW2REO_RING_MISC_TRANSACTION_TYPE_SHFT                                                                     22
#define HWIO_REO_R0_SW2REO1_RING_MISC_TRANSACTION_TYPE_BMSK                                                              0x400000
#define HWIO_REO_R0_SW2REO1_RING_MISC_TRANSACTION_TYPE_SHFT                                                                    22
#define HWIO_REO_R0_REO2SW1_RING_BASE_LSB_ADDR(x)                                                                      ((x) + 0x500)
#define HWIO_REO_R0_REO2SW1_RING_BASE_MSB_ADDR(x)                                                                      ((x) + 0x504)
#define HWIO_REO_R0_REO2SW1_RING_BASE_MSB_RING_SIZE_BMSK                                                                0xfffff00
#define HWIO_REO_R0_REO2SW1_RING_BASE_MSB_RING_SIZE_SHFT                                                                        8
#define HWIO_REO_R0_REO2SW1_RING_ID_ADDR(x)                                                                            ((x) + 0x508)
#define HWIO_REO_R0_REO2SW1_RING_MISC_ADDR(x)                                                                          ((x) + 0x510)
#define HWIO_REO_R0_REO2SW1_RING_MISC_TRANSACTION_TYPE_BMSK                                                             0x8000000
#define HWIO_REO_R0_REO2SW1_RING_MISC_TRANSACTION_TYPE_SHFT                                                                    27
#define HWIO_REO_R0_REO2SW1_RING_HP_ADDR_LSB_ADDR(x)                                                                   ((x) + 0x514)
#define HWIO_REO_R0_REO2SW1_RING_HP_ADDR_MSB_ADDR(x)                                                                   ((x) + 0x518)
#define HWIO_REO_R0_REO2SW1_RING_PRODUCER_INT_SETUP_ADDR(x)                                                            ((x) + 0x524)
#define HWIO_REO_R0_REO2SW1_RING_MSI1_BASE_LSB_ADDR(x)                                                                 ((x) + 0x548)
#define HWIO_REO_R0_REO2SW1_RING_MSI1_BASE_MSB_ADDR(x)                                                                 ((x) + 0x54c)
#define HWIO_REO_R0_REO2SW1_RING_MSI1_DATA_ADDR(x)                                                                     ((x) + 0x550)
#define HWIO_REO_R0_REO2SW1_RING_MISC_1_ADDR(x)                                                                        ((x) + 0x574)
#define HWIO_REO_R0_REO2SW1_RING_MISC_1_TIME_THRESHOLD_TO_UPDATE_BMSK                                                  0xffff0000
#define HWIO_REO_R0_REO2SW1_RING_MISC_1_TIME_THRESHOLD_TO_UPDATE_SHFT                                                          16
#define HWIO_REO_R0_REO2SW1_RING_MISC_1_NUM_THRESHOLD_TO_UPDATE_BMSK                                                         0x3f
#define HWIO_REO_R0_REO2SW1_RING_MISC_1_NUM_THRESHOLD_TO_UPDATE_SHFT                                                            0
#define HWIO_REO_R0_REO2SW1_RING_PRODUCER_INT2_SETUP_ADDR(x)                                                           ((x) + 0x554)
#define HWIO_REO_R0_REO2SW1_RING_MSI2_BASE_LSB_ADDR(x)                                                                 ((x) + 0x558)
#define HWIO_REO_R0_REO2SW1_RING_MSI2_BASE_MSB_ADDR(x)                                                                 ((x) + 0x55c)
#define HWIO_REO_R0_REO2SW1_RING_MSI2_DATA_ADDR(x)                                                                     ((x) + 0x560)
#define HWIO_REO_R0_REO2SW2_RING_BASE_LSB_ADDR(x)                                                                      ((x) + 0x578)
#define HWIO_REO_R0_REO2SW2_RING_MISC_TRANSACTION_TYPE_BMSK                                                             0x8000000
#define HWIO_REO_R0_REO2SW2_RING_MISC_TRANSACTION_TYPE_SHFT                                                                    27
#define HWIO_REO_R0_REO2SW3_RING_MISC_TRANSACTION_TYPE_BMSK                                                             0x8000000
#define HWIO_REO_R0_REO2SW3_RING_MISC_TRANSACTION_TYPE_SHFT                                                                    27
#define HWIO_REO_R0_REO2SW4_RING_MISC_TRANSACTION_TYPE_BMSK                                                             0x8000000
#define HWIO_REO_R0_REO2SW4_RING_MISC_TRANSACTION_TYPE_SHFT                                                                    27
#define HWIO_REO_R0_REO2SW5_RING_MISC_TRANSACTION_TYPE_BMSK                                                             0x8000000
#define HWIO_REO_R0_REO2SW5_RING_MISC_TRANSACTION_TYPE_SHFT                                                                    27
#define HWIO_REO_R0_REO2SW6_RING_MISC_TRANSACTION_TYPE_BMSK                                                             0x8000000
#define HWIO_REO_R0_REO2SW6_RING_MISC_TRANSACTION_TYPE_SHFT                                                                    27
#define HWIO_REO_R0_REO2SW7_RING_MISC_TRANSACTION_TYPE_BMSK                                                             0x8000000
#define HWIO_REO_R0_REO2SW7_RING_MISC_TRANSACTION_TYPE_SHFT                                                                    27
#define HWIO_REO_R0_REO2SW8_RING_MISC_TRANSACTION_TYPE_BMSK                                                             0x8000000
#define HWIO_REO_R0_REO2SW8_RING_MISC_TRANSACTION_TYPE_SHFT                                                                    27
#define HWIO_REO_R0_REO2SW0_RING_BASE_LSB_ADDR(x)                                                                      ((x) + 0x8c0)
#define HWIO_REO_R0_REO2SW0_RING_BASE_MSB_RING_SIZE_BMSK                                                                0xfffff00
#define HWIO_REO_R0_REO2SW0_RING_BASE_MSB_RING_SIZE_SHFT                                                                        8
#define HWIO_REO_R0_REO2SW0_RING_MISC_TRANSACTION_TYPE_BMSK                                                             0x8000000
#define HWIO_REO_R0_REO2SW0_RING_MISC_TRANSACTION_TYPE_SHFT                                                                    27
#define HWIO_REO_R0_REO2FW_RING_MISC_TRANSACTION_TYPE_BMSK                                                              0x8000000
#define HWIO_REO_R0_REO2FW_RING_MISC_TRANSACTION_TYPE_SHFT                                                                     27
#define HWIO_REO_R0_REO_RELEASE_RING_MISC_TRANSACTION_TYPE_BMSK                                                         0x8000000
#define HWIO_REO_R0_REO_RELEASE_RING_MISC_TRANSACTION_TYPE_SHFT                                                                27
#define HWIO_REO_R0_REO_STATUS_RING_BASE_LSB_ADDR(x)                                                                   ((x) + 0xaa0)
#define HWIO_REO_R0_REO_STATUS_RING_BASE_MSB_RING_SIZE_BMSK                                                              0xffff00
#define HWIO_REO_R0_REO_STATUS_RING_BASE_MSB_RING_SIZE_SHFT                                                                     8
#define HWIO_REO_R0_REO_STATUS_RING_MISC_TRANSACTION_TYPE_BMSK                                                          0x8000000
#define HWIO_REO_R0_REO_STATUS_RING_MISC_TRANSACTION_TYPE_SHFT                                                                 27
#define HWIO_REO_R0_MISC_CFG_ADDR(x)                                                                                   ((x) + 0xb24)
#define HWIO_REO_R0_MISC_CFG_PHYS(x)                                                                                   ((x) + 0xb24)
#define HWIO_REO_R0_MISC_CFG_OFFS                                                                                      (0xb24)
#define HWIO_REO_R0_MISC_CFG_RMSK                                                                                             0x1
#define HWIO_REO_R0_MISC_CFG_POR                                                                                       0x00000000
#define HWIO_REO_R0_MISC_CFG_POR_RMSK                                                                                  0xffffffff
#define HWIO_REO_R0_MISC_CFG_ATTR                                                                                                   0x3
#define HWIO_REO_R0_MISC_CFG_IN(x)            \
                in_dword(HWIO_REO_R0_MISC_CFG_ADDR(x))
#define HWIO_REO_R0_MISC_CFG_INM(x, m)            \
                in_dword_masked(HWIO_REO_R0_MISC_CFG_ADDR(x), m)
#define HWIO_REO_R0_MISC_CFG_OUT(x, v)            \
                out_dword(HWIO_REO_R0_MISC_CFG_ADDR(x),v)
#define HWIO_REO_R0_MISC_CFG_OUTM(x,m,v) \
                out_dword_masked_ns(HWIO_REO_R0_MISC_CFG_ADDR(x),m,v,HWIO_REO_R0_MISC_CFG_IN(x))
#define HWIO_REO_R0_MISC_CFG_CREDIT_BASED_MECH_EN_BMSK                                                                        0x1
#define HWIO_REO_R0_MISC_CFG_CREDIT_BASED_MECH_EN_SHFT                                                                          0

#define HWIO_REO_R0_MSDU_BUF_COUNT_CFG_ADDR(x)                                                                         ((x) + 0xb28)
#define HWIO_REO_R0_MSDU_BUF_COUNT_CFG_PHYS(x)                                                                         ((x) + 0xb28)
#define HWIO_REO_R0_MSDU_BUF_COUNT_CFG_OFFS                                                                            (0xb28)
#define HWIO_REO_R0_MSDU_BUF_COUNT_CFG_RMSK                                                                                 0x1ff
#define HWIO_REO_R0_MSDU_BUF_COUNT_CFG_POR                                                                             0x0000002d
#define HWIO_REO_R0_MSDU_BUF_COUNT_CFG_POR_RMSK                                                                        0xffffffff
#define HWIO_REO_R0_MSDU_BUF_COUNT_CFG_ATTR                                                                                         0x3
#define HWIO_REO_R0_MSDU_BUF_COUNT_CFG_IN(x)            \
                in_dword(HWIO_REO_R0_MSDU_BUF_COUNT_CFG_ADDR(x))
#define HWIO_REO_R0_MSDU_BUF_COUNT_CFG_INM(x, m)            \
                in_dword_masked(HWIO_REO_R0_MSDU_BUF_COUNT_CFG_ADDR(x), m)
#define HWIO_REO_R0_MSDU_BUF_COUNT_CFG_OUT(x, v)            \
                out_dword(HWIO_REO_R0_MSDU_BUF_COUNT_CFG_ADDR(x),v)
#define HWIO_REO_R0_MSDU_BUF_COUNT_CFG_OUTM(x,m,v) \
                out_dword_masked_ns(HWIO_REO_R0_MSDU_BUF_COUNT_CFG_ADDR(x),m,v,HWIO_REO_R0_MSDU_BUF_COUNT_CFG_IN(x))
#define HWIO_REO_R0_MSDU_BUF_COUNT_CFG_THRESHOLD_BUF_COUNT_BMSK                                                             0x1fe
#define HWIO_REO_R0_MSDU_BUF_COUNT_CFG_THRESHOLD_BUF_COUNT_SHFT                                                                 1
#define HWIO_REO_R0_MSDU_BUF_COUNT_CFG_DROP_EN_BMSK                                                                           0x1
#define HWIO_REO_R0_MSDU_BUF_COUNT_CFG_DROP_EN_SHFT                                                                             0

#define HWIO_REO_R0_AGING_THRESHOLD_IX_0_ADDR(x)                                                                       ((x) + 0xb2c)
#define HWIO_REO_R0_AGING_THRESHOLD_IX_1_ADDR(x)                                                                       ((x) + 0xb30)
#define HWIO_REO_R0_AGING_THRESHOLD_IX_2_ADDR(x)                                                                       ((x) + 0xb34)
#define HWIO_REO_R0_AGING_THRESHOLD_IX_3_ADDR(x)                                                                       ((x) + 0xb38)
#define HWIO_REO_R0_MISC_CTL_ADDR(x)                                                                                   ((x) + 0xba0)
#define HWIO_REO_R0_MISC_CTL_BAR_DEST_RING_BMSK                                                                         0x1e00000
#define HWIO_REO_R0_MISC_CTL_BAR_DEST_RING_SHFT                                                                                21
#define HWIO_REO_R0_MISC_CTL_FRAGMENT_DEST_RING_BMSK                                                                     0x1e0000
#define HWIO_REO_R0_MISC_CTL_FRAGMENT_DEST_RING_SHFT                                                                           17
#define HWIO_REO_R0_REO_MULTI_SRNG_CONS_RING_MISC_COMMON_ADDR(x)                                                       ((x) + 0xd78)
#define HWIO_REO_R0_REO_MULTI_SRNG_CONS_RING_MISC_COMMON_PHYS(x)                                                       ((x) + 0xd78)
#define HWIO_REO_R0_REO_MULTI_SRNG_CONS_RING_MISC_COMMON_OFFS                                                          (0xd78)
#define HWIO_REO_R0_REO_MULTI_SRNG_CONS_RING_MISC_COMMON_RMSK                                                            0x1fffff
#define HWIO_REO_R0_REO_MULTI_SRNG_CONS_RING_MISC_COMMON_POR                                                           0x00001000
#define HWIO_REO_R0_REO_MULTI_SRNG_CONS_RING_MISC_COMMON_POR_RMSK                                                      0xffffffff
#define HWIO_REO_R0_REO_MULTI_SRNG_CONS_RING_MISC_COMMON_ATTR                                                                       0x3
#define HWIO_REO_R0_REO_MULTI_SRNG_CONS_RING_MISC_COMMON_IN(x)            \
                in_dword(HWIO_REO_R0_REO_MULTI_SRNG_CONS_RING_MISC_COMMON_ADDR(x))
#define HWIO_REO_R0_REO_MULTI_SRNG_CONS_RING_MISC_COMMON_INM(x, m)            \
                in_dword_masked(HWIO_REO_R0_REO_MULTI_SRNG_CONS_RING_MISC_COMMON_ADDR(x), m)
#define HWIO_REO_R0_REO_MULTI_SRNG_CONS_RING_MISC_COMMON_OUT(x, v)            \
                out_dword(HWIO_REO_R0_REO_MULTI_SRNG_CONS_RING_MISC_COMMON_ADDR(x),v)
#define HWIO_REO_R0_REO_MULTI_SRNG_CONS_RING_MISC_COMMON_OUTM(x,m,v) \
                out_dword_masked_ns(HWIO_REO_R0_REO_MULTI_SRNG_CONS_RING_MISC_COMMON_ADDR(x),m,v,HWIO_REO_R0_REO_MULTI_SRNG_CONS_RING_MISC_COMMON_IN(x))
#define HWIO_REO_R0_REO_MULTI_SRNG_CONS_RING_MISC_COMMON_MISC_CONTROL_BMSK                                               0x1fe000
#define HWIO_REO_R0_REO_MULTI_SRNG_CONS_RING_MISC_COMMON_MISC_CONTROL_SHFT                                                     13
#define HWIO_REO_R0_REO_MULTI_SRNG_CONS_RING_MISC_COMMON_SRNG_IS_IDLE_BMSK                                                 0x1000
#define HWIO_REO_R0_REO_MULTI_SRNG_CONS_RING_MISC_COMMON_SRNG_IS_IDLE_SHFT                                                     12
#define HWIO_REO_R0_REO_MULTI_SRNG_CONS_RING_MISC_COMMON_SRNG_SM1_STATE2_BMSK                                               0xc00
#define HWIO_REO_R0_REO_MULTI_SRNG_CONS_RING_MISC_COMMON_SRNG_SM1_STATE2_SHFT                                                  10
#define HWIO_REO_R0_REO_MULTI_SRNG_CONS_RING_MISC_COMMON_SRNG_SM1_STATE1_BMSK                                               0x3c0
#define HWIO_REO_R0_REO_MULTI_SRNG_CONS_RING_MISC_COMMON_SRNG_SM1_STATE1_SHFT                                                   6
#define HWIO_REO_R0_REO_MULTI_SRNG_CONS_RING_MISC_COMMON_SRNG_SM0_STATE2_BMSK                                                0x30
#define HWIO_REO_R0_REO_MULTI_SRNG_CONS_RING_MISC_COMMON_SRNG_SM0_STATE2_SHFT                                                   4
#define HWIO_REO_R0_REO_MULTI_SRNG_CONS_RING_MISC_COMMON_SRNG_SM0_STATE1_BMSK                                                 0xf
#define HWIO_REO_R0_REO_MULTI_SRNG_CONS_RING_MISC_COMMON_SRNG_SM0_STATE1_SHFT                                                   0

#define HWIO_REO_R0_REO_MULTI_SRNG_CONS_RING_MISC_COMMON_EXT_ADDR(x)                                                   ((x) + 0xd7c)
#define HWIO_REO_R0_REO_MULTI_SRNG_CONS_RING_MISC_COMMON_EXT_PHYS(x)                                                   ((x) + 0xd7c)
#define HWIO_REO_R0_REO_MULTI_SRNG_CONS_RING_MISC_COMMON_EXT_OFFS                                                      (0xd7c)
#define HWIO_REO_R0_REO_MULTI_SRNG_CONS_RING_MISC_COMMON_EXT_RMSK                                                        0xffffff
#define HWIO_REO_R0_REO_MULTI_SRNG_CONS_RING_MISC_COMMON_EXT_POR                                                       0x00000fff
#define HWIO_REO_R0_REO_MULTI_SRNG_CONS_RING_MISC_COMMON_EXT_POR_RMSK                                                  0xffffffff
#define HWIO_REO_R0_REO_MULTI_SRNG_CONS_RING_MISC_COMMON_EXT_ATTR                                                                   0x3
#define HWIO_REO_R0_REO_MULTI_SRNG_CONS_RING_MISC_COMMON_EXT_IN(x)            \
                in_dword(HWIO_REO_R0_REO_MULTI_SRNG_CONS_RING_MISC_COMMON_EXT_ADDR(x))
#define HWIO_REO_R0_REO_MULTI_SRNG_CONS_RING_MISC_COMMON_EXT_INM(x, m)            \
                in_dword_masked(HWIO_REO_R0_REO_MULTI_SRNG_CONS_RING_MISC_COMMON_EXT_ADDR(x), m)
#define HWIO_REO_R0_REO_MULTI_SRNG_CONS_RING_MISC_COMMON_EXT_OUT(x, v)            \
                out_dword(HWIO_REO_R0_REO_MULTI_SRNG_CONS_RING_MISC_COMMON_EXT_ADDR(x),v)
#define HWIO_REO_R0_REO_MULTI_SRNG_CONS_RING_MISC_COMMON_EXT_OUTM(x,m,v) \
                out_dword_masked_ns(HWIO_REO_R0_REO_MULTI_SRNG_CONS_RING_MISC_COMMON_EXT_ADDR(x),m,v,HWIO_REO_R0_REO_MULTI_SRNG_CONS_RING_MISC_COMMON_EXT_IN(x))
#define HWIO_REO_R0_REO_MULTI_SRNG_CONS_RING_MISC_COMMON_EXT_RING_WATCHDOG_TIMER_STATUS_BMSK                             0xfff000
#define HWIO_REO_R0_REO_MULTI_SRNG_CONS_RING_MISC_COMMON_EXT_RING_WATCHDOG_TIMER_STATUS_SHFT                                   12
#define HWIO_REO_R0_REO_MULTI_SRNG_CONS_RING_MISC_COMMON_EXT_WATCHDOG_TIMEOUT_VALUE_BMSK                                    0xfff
#define HWIO_REO_R0_REO_MULTI_SRNG_CONS_RING_MISC_COMMON_EXT_WATCHDOG_TIMEOUT_VALUE_SHFT                                        0

#define HWIO_REO_R0_REO_MULTI_SRNG_PROD_RING_MISC_COMMON_ADDR(x)                                                       ((x) + 0xd80)
#define HWIO_REO_R0_REO_MULTI_SRNG_PROD_RING_MISC_COMMON_PHYS(x)                                                       ((x) + 0xd80)
#define HWIO_REO_R0_REO_MULTI_SRNG_PROD_RING_MISC_COMMON_OFFS                                                          (0xd80)
#define HWIO_REO_R0_REO_MULTI_SRNG_PROD_RING_MISC_COMMON_RMSK                                                            0x1fffff
#define HWIO_REO_R0_REO_MULTI_SRNG_PROD_RING_MISC_COMMON_POR                                                           0x00001000
#define HWIO_REO_R0_REO_MULTI_SRNG_PROD_RING_MISC_COMMON_POR_RMSK                                                      0xffffffff
#define HWIO_REO_R0_REO_MULTI_SRNG_PROD_RING_MISC_COMMON_ATTR                                                                       0x3
#define HWIO_REO_R0_REO_MULTI_SRNG_PROD_RING_MISC_COMMON_IN(x)            \
                in_dword(HWIO_REO_R0_REO_MULTI_SRNG_PROD_RING_MISC_COMMON_ADDR(x))
#define HWIO_REO_R0_REO_MULTI_SRNG_PROD_RING_MISC_COMMON_INM(x, m)            \
                in_dword_masked(HWIO_REO_R0_REO_MULTI_SRNG_PROD_RING_MISC_COMMON_ADDR(x), m)
#define HWIO_REO_R0_REO_MULTI_SRNG_PROD_RING_MISC_COMMON_OUT(x, v)            \
                out_dword(HWIO_REO_R0_REO_MULTI_SRNG_PROD_RING_MISC_COMMON_ADDR(x),v)
#define HWIO_REO_R0_REO_MULTI_SRNG_PROD_RING_MISC_COMMON_OUTM(x,m,v) \
                out_dword_masked_ns(HWIO_REO_R0_REO_MULTI_SRNG_PROD_RING_MISC_COMMON_ADDR(x),m,v,HWIO_REO_R0_REO_MULTI_SRNG_PROD_RING_MISC_COMMON_IN(x))
#define HWIO_REO_R0_REO_MULTI_SRNG_PROD_RING_MISC_COMMON_MISC_CONTROL_BMSK                                               0x1fe000
#define HWIO_REO_R0_REO_MULTI_SRNG_PROD_RING_MISC_COMMON_MISC_CONTROL_SHFT                                                     13
#define HWIO_REO_R0_REO_MULTI_SRNG_PROD_RING_MISC_COMMON_SRNG_IS_IDLE_BMSK                                                 0x1000
#define HWIO_REO_R0_REO_MULTI_SRNG_PROD_RING_MISC_COMMON_SRNG_IS_IDLE_SHFT                                                     12
#define HWIO_REO_R0_REO_MULTI_SRNG_PROD_RING_MISC_COMMON_SRNG_SM1_STATE2_BMSK                                               0xc00
#define HWIO_REO_R0_REO_MULTI_SRNG_PROD_RING_MISC_COMMON_SRNG_SM1_STATE2_SHFT                                                  10
#define HWIO_REO_R0_REO_MULTI_SRNG_PROD_RING_MISC_COMMON_SRNG_SM1_STATE1_BMSK                                               0x3c0
#define HWIO_REO_R0_REO_MULTI_SRNG_PROD_RING_MISC_COMMON_SRNG_SM1_STATE1_SHFT                                                   6
#define HWIO_REO_R0_REO_MULTI_SRNG_PROD_RING_MISC_COMMON_SRNG_SM0_STATE2_BMSK                                                0x30
#define HWIO_REO_R0_REO_MULTI_SRNG_PROD_RING_MISC_COMMON_SRNG_SM0_STATE2_SHFT                                                   4
#define HWIO_REO_R0_REO_MULTI_SRNG_PROD_RING_MISC_COMMON_SRNG_SM0_STATE1_BMSK                                                 0xf
#define HWIO_REO_R0_REO_MULTI_SRNG_PROD_RING_MISC_COMMON_SRNG_SM0_STATE1_SHFT                                                   0

#define HWIO_REO_R0_REO_MULTI_SRNG_PROD_RING_MISC_COMMON_EXT_ADDR(x)                                                   ((x) + 0xd84)
#define HWIO_REO_R0_REO_MULTI_SRNG_PROD_RING_MISC_COMMON_EXT_PHYS(x)                                                   ((x) + 0xd84)
#define HWIO_REO_R0_REO_MULTI_SRNG_PROD_RING_MISC_COMMON_EXT_OFFS                                                      (0xd84)
#define HWIO_REO_R0_REO_MULTI_SRNG_PROD_RING_MISC_COMMON_EXT_RMSK                                                        0xffffff
#define HWIO_REO_R0_REO_MULTI_SRNG_PROD_RING_MISC_COMMON_EXT_POR                                                       0x00000fff
#define HWIO_REO_R0_REO_MULTI_SRNG_PROD_RING_MISC_COMMON_EXT_POR_RMSK                                                  0xffffffff
#define HWIO_REO_R0_REO_MULTI_SRNG_PROD_RING_MISC_COMMON_EXT_ATTR                                                                   0x3
#define HWIO_REO_R0_REO_MULTI_SRNG_PROD_RING_MISC_COMMON_EXT_IN(x)            \
                in_dword(HWIO_REO_R0_REO_MULTI_SRNG_PROD_RING_MISC_COMMON_EXT_ADDR(x))
#define HWIO_REO_R0_REO_MULTI_SRNG_PROD_RING_MISC_COMMON_EXT_INM(x, m)            \
                in_dword_masked(HWIO_REO_R0_REO_MULTI_SRNG_PROD_RING_MISC_COMMON_EXT_ADDR(x), m)
#define HWIO_REO_R0_REO_MULTI_SRNG_PROD_RING_MISC_COMMON_EXT_OUT(x, v)            \
                out_dword(HWIO_REO_R0_REO_MULTI_SRNG_PROD_RING_MISC_COMMON_EXT_ADDR(x),v)
#define HWIO_REO_R0_REO_MULTI_SRNG_PROD_RING_MISC_COMMON_EXT_OUTM(x,m,v) \
                out_dword_masked_ns(HWIO_REO_R0_REO_MULTI_SRNG_PROD_RING_MISC_COMMON_EXT_ADDR(x),m,v,HWIO_REO_R0_REO_MULTI_SRNG_PROD_RING_MISC_COMMON_EXT_IN(x))
#define HWIO_REO_R0_REO_MULTI_SRNG_PROD_RING_MISC_COMMON_EXT_RING_WATCHDOG_TIMER_STATUS_BMSK                             0xfff000
#define HWIO_REO_R0_REO_MULTI_SRNG_PROD_RING_MISC_COMMON_EXT_RING_WATCHDOG_TIMER_STATUS_SHFT                                   12
#define HWIO_REO_R0_REO_MULTI_SRNG_PROD_RING_MISC_COMMON_EXT_WATCHDOG_TIMEOUT_VALUE_BMSK                                    0xfff
#define HWIO_REO_R0_REO_MULTI_SRNG_PROD_RING_MISC_COMMON_EXT_WATCHDOG_TIMEOUT_VALUE_SHFT                                        0

#define HWIO_REO_R0_CREDIT_ADDR(x)                                                                                     ((x) + 0xd88)
#define HWIO_REO_R0_CREDIT_PHYS(x)                                                                                     ((x) + 0xd88)
#define HWIO_REO_R0_CREDIT_OFFS                                                                                        (0xd88)
#define HWIO_REO_R0_CREDIT_RMSK                                                                                        0xffffffff
#define HWIO_REO_R0_CREDIT_POR                                                                                         0x00000000
#define HWIO_REO_R0_CREDIT_POR_RMSK                                                                                    0xffffffff
#define HWIO_REO_R0_CREDIT_ATTR                                                                                                     0x3
#define HWIO_REO_R0_CREDIT_IN(x)            \
                in_dword(HWIO_REO_R0_CREDIT_ADDR(x))
#define HWIO_REO_R0_CREDIT_INM(x, m)            \
                in_dword_masked(HWIO_REO_R0_CREDIT_ADDR(x), m)
#define HWIO_REO_R0_CREDIT_OUT(x, v)            \
                out_dword(HWIO_REO_R0_CREDIT_ADDR(x),v)
#define HWIO_REO_R0_CREDIT_OUTM(x,m,v) \
                out_dword_masked_ns(HWIO_REO_R0_CREDIT_ADDR(x),m,v,HWIO_REO_R0_CREDIT_IN(x))
#define HWIO_REO_R0_CREDIT_VAL_BMSK                                                                                    0xffffffff
#define HWIO_REO_R0_CREDIT_VAL_SHFT                                                                                             0

#define HWIO_REO_R0_CREDIT_AVAIL_RING_MASK_ADDR(x)                                                                     ((x) + 0xd8c)
#define HWIO_REO_R0_CREDIT_AVAIL_RING_MASK_PHYS(x)                                                                     ((x) + 0xd8c)
#define HWIO_REO_R0_CREDIT_AVAIL_RING_MASK_OFFS                                                                        (0xd8c)
#define HWIO_REO_R0_CREDIT_AVAIL_RING_MASK_RMSK                                                                               0x7
#define HWIO_REO_R0_CREDIT_AVAIL_RING_MASK_POR                                                                         0x00000002
#define HWIO_REO_R0_CREDIT_AVAIL_RING_MASK_POR_RMSK                                                                    0xffffffff
#define HWIO_REO_R0_CREDIT_AVAIL_RING_MASK_ATTR                                                                                     0x3
#define HWIO_REO_R0_CREDIT_AVAIL_RING_MASK_IN(x)            \
                in_dword(HWIO_REO_R0_CREDIT_AVAIL_RING_MASK_ADDR(x))
#define HWIO_REO_R0_CREDIT_AVAIL_RING_MASK_INM(x, m)            \
                in_dword_masked(HWIO_REO_R0_CREDIT_AVAIL_RING_MASK_ADDR(x), m)
#define HWIO_REO_R0_CREDIT_AVAIL_RING_MASK_OUT(x, v)            \
                out_dword(HWIO_REO_R0_CREDIT_AVAIL_RING_MASK_ADDR(x),v)
#define HWIO_REO_R0_CREDIT_AVAIL_RING_MASK_OUTM(x,m,v) \
                out_dword_masked_ns(HWIO_REO_R0_CREDIT_AVAIL_RING_MASK_ADDR(x),m,v,HWIO_REO_R0_CREDIT_AVAIL_RING_MASK_IN(x))
#define HWIO_REO_R0_CREDIT_AVAIL_RING_MASK_ENABLE_BMSK                                                                        0x7
#define HWIO_REO_R0_CREDIT_AVAIL_RING_MASK_ENABLE_SHFT                                                                          0

#define HWIO_REO_R0_CREDIT_COUNTER_STATUS_ADDR(x)                                                                      ((x) + 0xd90)
#define HWIO_REO_R0_CREDIT_COUNTER_STATUS_PHYS(x)                                                                      ((x) + 0xd90)
#define HWIO_REO_R0_CREDIT_COUNTER_STATUS_OFFS                                                                         (0xd90)
#define HWIO_REO_R0_CREDIT_COUNTER_STATUS_RMSK                                                                         0xffffffff
#define HWIO_REO_R0_CREDIT_COUNTER_STATUS_POR                                                                          0x00000000
#define HWIO_REO_R0_CREDIT_COUNTER_STATUS_POR_RMSK                                                                     0xffffffff
#define HWIO_REO_R0_CREDIT_COUNTER_STATUS_ATTR                                                                                      0x1
#define HWIO_REO_R0_CREDIT_COUNTER_STATUS_IN(x)            \
                in_dword(HWIO_REO_R0_CREDIT_COUNTER_STATUS_ADDR(x))
#define HWIO_REO_R0_CREDIT_COUNTER_STATUS_INM(x, m)            \
                in_dword_masked(HWIO_REO_R0_CREDIT_COUNTER_STATUS_ADDR(x), m)
#define HWIO_REO_R0_CREDIT_COUNTER_STATUS_VAL_BMSK                                                                     0xffffffff
#define HWIO_REO_R0_CREDIT_COUNTER_STATUS_VAL_SHFT                                                                              0

#define HWIO_REO_R0_CLK_GATE_CTRL_REO_CLKGATE_DISABLE_TESTBUS_CAPTURE_BMSK                                                 0x2000
#define HWIO_REO_R0_CLK_GATE_CTRL_REO_CLKGATE_DISABLE_TESTBUS_CAPTURE_SHFT                                                     13
#define HWIO_REO_R0_LPM_FW_CTRL_ADDR(x)                                                                                ((x) + 0xdbc)
#define HWIO_REO_R0_LPM_FW_CTRL_PHYS(x)                                                                                ((x) + 0xdbc)
#define HWIO_REO_R0_LPM_FW_CTRL_OFFS                                                                                   (0xdbc)
#define HWIO_REO_R0_LPM_FW_CTRL_RMSK                                                                                          0x7
#define HWIO_REO_R0_LPM_FW_CTRL_POR                                                                                    0x00000000
#define HWIO_REO_R0_LPM_FW_CTRL_POR_RMSK                                                                               0xffffffff
#define HWIO_REO_R0_LPM_FW_CTRL_ATTR                                                                                                0x3
#define HWIO_REO_R0_LPM_FW_CTRL_IN(x)            \
                in_dword(HWIO_REO_R0_LPM_FW_CTRL_ADDR(x))
#define HWIO_REO_R0_LPM_FW_CTRL_INM(x, m)            \
                in_dword_masked(HWIO_REO_R0_LPM_FW_CTRL_ADDR(x), m)
#define HWIO_REO_R0_LPM_FW_CTRL_OUT(x, v)            \
                out_dword(HWIO_REO_R0_LPM_FW_CTRL_ADDR(x),v)
#define HWIO_REO_R0_LPM_FW_CTRL_OUTM(x,m,v) \
                out_dword_masked_ns(HWIO_REO_R0_LPM_FW_CTRL_ADDR(x),m,v,HWIO_REO_R0_LPM_FW_CTRL_IN(x))
#define HWIO_REO_R0_LPM_FW_CTRL_SLEEP_REQ_ACK_OVR_MULTI_SRNG_P_BMSK                                                           0x4
#define HWIO_REO_R0_LPM_FW_CTRL_SLEEP_REQ_ACK_OVR_MULTI_SRNG_P_SHFT                                                             2
#define HWIO_REO_R0_LPM_FW_CTRL_SLEEP_REQ_ACK_OVR_MULTI_SRNG_C_BMSK                                                           0x2
#define HWIO_REO_R0_LPM_FW_CTRL_SLEEP_REQ_ACK_OVR_MULTI_SRNG_C_SHFT                                                             1
#define HWIO_REO_R0_LPM_FW_CTRL_SLEEP_REQ_BMSK                                                                                0x1
#define HWIO_REO_R0_LPM_FW_CTRL_SLEEP_REQ_SHFT                                                                                  0

#define HWIO_REO_R1_CACHE_CTL_DEBUG_OWNER_CHECK_ADDR_HIGH_ADDR(x)                                                      ((x) + 0x2054)
#define HWIO_REO_R1_CACHE_CTL_DEBUG_OWNER_CHECK_ADDR_HIGH_PHYS(x)                                                      ((x) + 0x2054)
#define HWIO_REO_R1_CACHE_CTL_DEBUG_OWNER_CHECK_ADDR_HIGH_OFFS                                                         (0x2054)
#define HWIO_REO_R1_CACHE_CTL_DEBUG_OWNER_CHECK_ADDR_HIGH_RMSK                                                               0xff
#define HWIO_REO_R1_CACHE_CTL_DEBUG_OWNER_CHECK_ADDR_HIGH_POR                                                          0x00000000
#define HWIO_REO_R1_CACHE_CTL_DEBUG_OWNER_CHECK_ADDR_HIGH_POR_RMSK                                                     0xffffffff
#define HWIO_REO_R1_CACHE_CTL_DEBUG_OWNER_CHECK_ADDR_HIGH_ATTR                                                                      0x1
#define HWIO_REO_R1_CACHE_CTL_DEBUG_OWNER_CHECK_ADDR_HIGH_IN(x)            \
                in_dword(HWIO_REO_R1_CACHE_CTL_DEBUG_OWNER_CHECK_ADDR_HIGH_ADDR(x))
#define HWIO_REO_R1_CACHE_CTL_DEBUG_OWNER_CHECK_ADDR_HIGH_INM(x, m)            \
                in_dword_masked(HWIO_REO_R1_CACHE_CTL_DEBUG_OWNER_CHECK_ADDR_HIGH_ADDR(x), m)
#define HWIO_REO_R1_CACHE_CTL_DEBUG_OWNER_CHECK_ADDR_HIGH_ADDR_39_32_BMSK                                                    0xff
#define HWIO_REO_R1_CACHE_CTL_DEBUG_OWNER_CHECK_ADDR_HIGH_ADDR_39_32_SHFT                                                       0

#define HWIO_REO_R1_CACHE_CTL_DEBUG_OWNER_CHECK_ADDR_LOW_ADDR(x)                                                       ((x) + 0x2058)
#define HWIO_REO_R1_CACHE_CTL_DEBUG_OWNER_CHECK_ADDR_LOW_PHYS(x)                                                       ((x) + 0x2058)
#define HWIO_REO_R1_CACHE_CTL_DEBUG_OWNER_CHECK_ADDR_LOW_OFFS                                                          (0x2058)
#define HWIO_REO_R1_CACHE_CTL_DEBUG_OWNER_CHECK_ADDR_LOW_RMSK                                                          0xffffffff
#define HWIO_REO_R1_CACHE_CTL_DEBUG_OWNER_CHECK_ADDR_LOW_POR                                                           0x00000000
#define HWIO_REO_R1_CACHE_CTL_DEBUG_OWNER_CHECK_ADDR_LOW_POR_RMSK                                                      0xffffffff
#define HWIO_REO_R1_CACHE_CTL_DEBUG_OWNER_CHECK_ADDR_LOW_ATTR                                                                       0x1
#define HWIO_REO_R1_CACHE_CTL_DEBUG_OWNER_CHECK_ADDR_LOW_IN(x)            \
                in_dword(HWIO_REO_R1_CACHE_CTL_DEBUG_OWNER_CHECK_ADDR_LOW_ADDR(x))
#define HWIO_REO_R1_CACHE_CTL_DEBUG_OWNER_CHECK_ADDR_LOW_INM(x, m)            \
                in_dword_masked(HWIO_REO_R1_CACHE_CTL_DEBUG_OWNER_CHECK_ADDR_LOW_ADDR(x), m)
#define HWIO_REO_R1_CACHE_CTL_DEBUG_OWNER_CHECK_ADDR_LOW_ADDR_31_0_BMSK                                                0xffffffff
#define HWIO_REO_R1_CACHE_CTL_DEBUG_OWNER_CHECK_ADDR_LOW_ADDR_31_0_SHFT                                                         0

#define HWIO_REO_R1_CACHE_CTL_DEBUG_CACHE_SNAPSHOT_ADDR_HIGH_ADDR(x)                                                   ((x) + 0x205c)
#define HWIO_REO_R1_CACHE_CTL_DEBUG_CACHE_SNAPSHOT_ADDR_HIGH_PHYS(x)                                                   ((x) + 0x205c)
#define HWIO_REO_R1_CACHE_CTL_DEBUG_CACHE_SNAPSHOT_ADDR_HIGH_OFFS                                                      (0x205c)
#define HWIO_REO_R1_CACHE_CTL_DEBUG_CACHE_SNAPSHOT_ADDR_HIGH_RMSK                                                            0xff
#define HWIO_REO_R1_CACHE_CTL_DEBUG_CACHE_SNAPSHOT_ADDR_HIGH_POR                                                       0x00000000
#define HWIO_REO_R1_CACHE_CTL_DEBUG_CACHE_SNAPSHOT_ADDR_HIGH_POR_RMSK                                                  0xffffffff
#define HWIO_REO_R1_CACHE_CTL_DEBUG_CACHE_SNAPSHOT_ADDR_HIGH_ATTR                                                                   0x2
#define HWIO_REO_R1_CACHE_CTL_DEBUG_CACHE_SNAPSHOT_ADDR_HIGH_OUT(x, v)            \
                out_dword(HWIO_REO_R1_CACHE_CTL_DEBUG_CACHE_SNAPSHOT_ADDR_HIGH_ADDR(x),v)
#define HWIO_REO_R1_CACHE_CTL_DEBUG_CACHE_SNAPSHOT_ADDR_HIGH_ADDR_39_32_BMSK                                                 0xff
#define HWIO_REO_R1_CACHE_CTL_DEBUG_CACHE_SNAPSHOT_ADDR_HIGH_ADDR_39_32_SHFT                                                    0

#define HWIO_REO_R1_CACHE_CTL_DEBUG_CACHE_SNAPSHOT_ADDR_LOW_ADDR(x)                                                    ((x) + 0x2060)
#define HWIO_REO_R1_CACHE_CTL_DEBUG_CACHE_SNAPSHOT_ADDR_LOW_PHYS(x)                                                    ((x) + 0x2060)
#define HWIO_REO_R1_CACHE_CTL_DEBUG_CACHE_SNAPSHOT_ADDR_LOW_OFFS                                                       (0x2060)
#define HWIO_REO_R1_CACHE_CTL_DEBUG_CACHE_SNAPSHOT_ADDR_LOW_RMSK                                                       0xffffffff
#define HWIO_REO_R1_CACHE_CTL_DEBUG_CACHE_SNAPSHOT_ADDR_LOW_POR                                                        0x00000000
#define HWIO_REO_R1_CACHE_CTL_DEBUG_CACHE_SNAPSHOT_ADDR_LOW_POR_RMSK                                                   0xffffffff
#define HWIO_REO_R1_CACHE_CTL_DEBUG_CACHE_SNAPSHOT_ADDR_LOW_ATTR                                                                    0x2
#define HWIO_REO_R1_CACHE_CTL_DEBUG_CACHE_SNAPSHOT_ADDR_LOW_OUT(x, v)            \
                out_dword(HWIO_REO_R1_CACHE_CTL_DEBUG_CACHE_SNAPSHOT_ADDR_LOW_ADDR(x),v)
#define HWIO_REO_R1_CACHE_CTL_DEBUG_CACHE_SNAPSHOT_ADDR_LOW_ADDR_31_0_BMSK                                             0xffffffff
#define HWIO_REO_R1_CACHE_CTL_DEBUG_CACHE_SNAPSHOT_ADDR_LOW_ADDR_31_0_SHFT                                                      0

#define HWIO_REO_R1_CACHE_CTL_DEBUG_CACHE_SNAPSHOT_CAP_CONTROL_ADDR(x)                                                 ((x) + 0x2064)
#define HWIO_REO_R1_CACHE_CTL_DEBUG_CACHE_SNAPSHOT_CAP_CONTROL_PHYS(x)                                                 ((x) + 0x2064)
#define HWIO_REO_R1_CACHE_CTL_DEBUG_CACHE_SNAPSHOT_CAP_CONTROL_OFFS                                                    (0x2064)
#define HWIO_REO_R1_CACHE_CTL_DEBUG_CACHE_SNAPSHOT_CAP_CONTROL_RMSK                                                    0xffffffff
#define HWIO_REO_R1_CACHE_CTL_DEBUG_CACHE_SNAPSHOT_CAP_CONTROL_POR                                                     0x00000000
#define HWIO_REO_R1_CACHE_CTL_DEBUG_CACHE_SNAPSHOT_CAP_CONTROL_POR_RMSK                                                0xffffffff
#define HWIO_REO_R1_CACHE_CTL_DEBUG_CACHE_SNAPSHOT_CAP_CONTROL_ATTR                                                                 0x3
#define HWIO_REO_R1_CACHE_CTL_DEBUG_CACHE_SNAPSHOT_CAP_CONTROL_IN(x)            \
                in_dword(HWIO_REO_R1_CACHE_CTL_DEBUG_CACHE_SNAPSHOT_CAP_CONTROL_ADDR(x))
#define HWIO_REO_R1_CACHE_CTL_DEBUG_CACHE_SNAPSHOT_CAP_CONTROL_INM(x, m)            \
                in_dword_masked(HWIO_REO_R1_CACHE_CTL_DEBUG_CACHE_SNAPSHOT_CAP_CONTROL_ADDR(x), m)
#define HWIO_REO_R1_CACHE_CTL_DEBUG_CACHE_SNAPSHOT_CAP_CONTROL_OUT(x, v)            \
                out_dword(HWIO_REO_R1_CACHE_CTL_DEBUG_CACHE_SNAPSHOT_CAP_CONTROL_ADDR(x),v)
#define HWIO_REO_R1_CACHE_CTL_DEBUG_CACHE_SNAPSHOT_CAP_CONTROL_OUTM(x,m,v) \
                out_dword_masked_ns(HWIO_REO_R1_CACHE_CTL_DEBUG_CACHE_SNAPSHOT_CAP_CONTROL_ADDR(x),m,v,HWIO_REO_R1_CACHE_CTL_DEBUG_CACHE_SNAPSHOT_CAP_CONTROL_IN(x))
#define HWIO_REO_R1_CACHE_CTL_DEBUG_CACHE_SNAPSHOT_CAP_CONTROL_MISC_CTRL_BMSK                                          0xffff0000
#define HWIO_REO_R1_CACHE_CTL_DEBUG_CACHE_SNAPSHOT_CAP_CONTROL_MISC_CTRL_SHFT                                                  16
#define HWIO_REO_R1_CACHE_CTL_DEBUG_CACHE_SNAPSHOT_CAP_CONTROL_ARMED_CAPTURE_TRIGGER_BMSK                                  0xfff0
#define HWIO_REO_R1_CACHE_CTL_DEBUG_CACHE_SNAPSHOT_CAP_CONTROL_ARMED_CAPTURE_TRIGGER_SHFT                                       4
#define HWIO_REO_R1_CACHE_CTL_DEBUG_CACHE_SNAPSHOT_CAP_CONTROL_CAPTURE_ARMED_DONE_BMSK                                        0x8
#define HWIO_REO_R1_CACHE_CTL_DEBUG_CACHE_SNAPSHOT_CAP_CONTROL_CAPTURE_ARMED_DONE_SHFT                                          3
#define HWIO_REO_R1_CACHE_CTL_DEBUG_CACHE_SNAPSHOT_CAP_CONTROL_CAPTURE_ARMED_BMSK                                             0x4
#define HWIO_REO_R1_CACHE_CTL_DEBUG_CACHE_SNAPSHOT_CAP_CONTROL_CAPTURE_ARMED_SHFT                                               2
#define HWIO_REO_R1_CACHE_CTL_DEBUG_CACHE_SNAPSHOT_CAP_CONTROL_CAPTURE_NOW_DONE_STATUS_BMSK                                   0x2
#define HWIO_REO_R1_CACHE_CTL_DEBUG_CACHE_SNAPSHOT_CAP_CONTROL_CAPTURE_NOW_DONE_STATUS_SHFT                                     1
#define HWIO_REO_R1_CACHE_CTL_DEBUG_CACHE_SNAPSHOT_CAP_CONTROL_CAPTURE_NOW_BMSK                                               0x1
#define HWIO_REO_R1_CACHE_CTL_DEBUG_CACHE_SNAPSHOT_CAP_CONTROL_CAPTURE_NOW_SHFT                                                 0

#define HWIO_REO_R1_MISC_DEBUG_STATUS_ADDR(x)                                                                          ((x) + 0x20c0)
#define HWIO_REO_R1_MISC_DEBUG_STATUS_PHYS(x)                                                                          ((x) + 0x20c0)
#define HWIO_REO_R1_MISC_DEBUG_STATUS_OFFS                                                                             (0x20c0)
#define HWIO_REO_R1_MISC_DEBUG_STATUS_RMSK                                                                                   0x3f
#define HWIO_REO_R1_MISC_DEBUG_STATUS_POR                                                                              0x00000000
#define HWIO_REO_R1_MISC_DEBUG_STATUS_POR_RMSK                                                                         0xffffffff
#define HWIO_REO_R1_MISC_DEBUG_STATUS_ATTR                                                                                          0x1
#define HWIO_REO_R1_MISC_DEBUG_STATUS_IN(x)            \
                in_dword(HWIO_REO_R1_MISC_DEBUG_STATUS_ADDR(x))
#define HWIO_REO_R1_MISC_DEBUG_STATUS_INM(x, m)            \
                in_dword_masked(HWIO_REO_R1_MISC_DEBUG_STATUS_ADDR(x), m)
#define HWIO_REO_R1_MISC_DEBUG_STATUS_BUF_COUNT_EXCEEDED_FLAG_2_BMSK                                                         0x20
#define HWIO_REO_R1_MISC_DEBUG_STATUS_BUF_COUNT_EXCEEDED_FLAG_2_SHFT                                                            5
#define HWIO_REO_R1_MISC_DEBUG_STATUS_TMP_FIFO_FULL_2_BMSK                                                                   0x10
#define HWIO_REO_R1_MISC_DEBUG_STATUS_TMP_FIFO_FULL_2_SHFT                                                                      4
#define HWIO_REO_R1_MISC_DEBUG_STATUS_TMP_CMD_FIFO_FULL_2_BMSK                                                                0x8
#define HWIO_REO_R1_MISC_DEBUG_STATUS_TMP_CMD_FIFO_FULL_2_SHFT                                                                  3
#define HWIO_REO_R1_MISC_DEBUG_STATUS_BUF_COUNT_EXCEEDED_FLAG_BMSK                                                            0x4
#define HWIO_REO_R1_MISC_DEBUG_STATUS_BUF_COUNT_EXCEEDED_FLAG_SHFT                                                              2
#define HWIO_REO_R1_MISC_DEBUG_STATUS_TMP_FIFO_FULL_BMSK                                                                      0x2
#define HWIO_REO_R1_MISC_DEBUG_STATUS_TMP_FIFO_FULL_SHFT                                                                        1
#define HWIO_REO_R1_MISC_DEBUG_STATUS_TMP_CMD_FIFO_FULL_BMSK                                                                  0x1
#define HWIO_REO_R1_MISC_DEBUG_STATUS_TMP_CMD_FIFO_FULL_SHFT                                                                    0

#define HWIO_REO_R1_DEBUG_COUNTER_MSDU_BUF_COUNT_EXCEEDED_ADDR(x)                                                      ((x) + 0x20c4)
#define HWIO_REO_R1_DEBUG_COUNTER_MSDU_BUF_COUNT_EXCEEDED_PHYS(x)                                                      ((x) + 0x20c4)
#define HWIO_REO_R1_DEBUG_COUNTER_MSDU_BUF_COUNT_EXCEEDED_OFFS                                                         (0x20c4)
#define HWIO_REO_R1_DEBUG_COUNTER_MSDU_BUF_COUNT_EXCEEDED_RMSK                                                         0xffffffff
#define HWIO_REO_R1_DEBUG_COUNTER_MSDU_BUF_COUNT_EXCEEDED_POR                                                          0x00000000
#define HWIO_REO_R1_DEBUG_COUNTER_MSDU_BUF_COUNT_EXCEEDED_POR_RMSK                                                     0xffffffff
#define HWIO_REO_R1_DEBUG_COUNTER_MSDU_BUF_COUNT_EXCEEDED_ATTR                                                                      0x3
#define HWIO_REO_R1_DEBUG_COUNTER_MSDU_BUF_COUNT_EXCEEDED_IN(x)            \
                in_dword(HWIO_REO_R1_DEBUG_COUNTER_MSDU_BUF_COUNT_EXCEEDED_ADDR(x))
#define HWIO_REO_R1_DEBUG_COUNTER_MSDU_BUF_COUNT_EXCEEDED_INM(x, m)            \
                in_dword_masked(HWIO_REO_R1_DEBUG_COUNTER_MSDU_BUF_COUNT_EXCEEDED_ADDR(x), m)
#define HWIO_REO_R1_DEBUG_COUNTER_MSDU_BUF_COUNT_EXCEEDED_OUT(x, v)            \
                out_dword(HWIO_REO_R1_DEBUG_COUNTER_MSDU_BUF_COUNT_EXCEEDED_ADDR(x),v)
#define HWIO_REO_R1_DEBUG_COUNTER_MSDU_BUF_COUNT_EXCEEDED_OUTM(x,m,v) \
                out_dword_masked_ns(HWIO_REO_R1_DEBUG_COUNTER_MSDU_BUF_COUNT_EXCEEDED_ADDR(x),m,v,HWIO_REO_R1_DEBUG_COUNTER_MSDU_BUF_COUNT_EXCEEDED_IN(x))
#define HWIO_REO_R1_DEBUG_COUNTER_MSDU_BUF_COUNT_EXCEEDED_COUNT_BMSK                                                   0xffffffff
#define HWIO_REO_R1_DEBUG_COUNTER_MSDU_BUF_COUNT_EXCEEDED_COUNT_SHFT                                                            0

#define HWIO_REO_R1_LPM_REQ_HANDLER_STATUS_ADDR(x)                                                                     ((x) + 0x20cc)
#define HWIO_REO_R1_LPM_REQ_HANDLER_STATUS_PHYS(x)                                                                     ((x) + 0x20cc)
#define HWIO_REO_R1_LPM_REQ_HANDLER_STATUS_OFFS                                                                        (0x20cc)
#define HWIO_REO_R1_LPM_REQ_HANDLER_STATUS_RMSK                                                                              0x7f
#define HWIO_REO_R1_LPM_REQ_HANDLER_STATUS_POR                                                                         0x00000000
#define HWIO_REO_R1_LPM_REQ_HANDLER_STATUS_POR_RMSK                                                                    0xffffffff
#define HWIO_REO_R1_LPM_REQ_HANDLER_STATUS_ATTR                                                                                     0x1
#define HWIO_REO_R1_LPM_REQ_HANDLER_STATUS_IN(x)            \
                in_dword(HWIO_REO_R1_LPM_REQ_HANDLER_STATUS_ADDR(x))
#define HWIO_REO_R1_LPM_REQ_HANDLER_STATUS_INM(x, m)            \
                in_dword_masked(HWIO_REO_R1_LPM_REQ_HANDLER_STATUS_ADDR(x), m)
#define HWIO_REO_R1_LPM_REQ_HANDLER_STATUS_SLEEP_REQ_ACK_MULTI_SRNG_P_BMSK                                                   0x40
#define HWIO_REO_R1_LPM_REQ_HANDLER_STATUS_SLEEP_REQ_ACK_MULTI_SRNG_P_SHFT                                                      6
#define HWIO_REO_R1_LPM_REQ_HANDLER_STATUS_SLEEP_REQ_ACK_MULTI_SRNG_C_BMSK                                                   0x20
#define HWIO_REO_R1_LPM_REQ_HANDLER_STATUS_SLEEP_REQ_ACK_MULTI_SRNG_C_SHFT                                                      5
#define HWIO_REO_R1_LPM_REQ_HANDLER_STATUS_SLEEP_REQ_ACK_TO_LPM_BMSK                                                         0x10
#define HWIO_REO_R1_LPM_REQ_HANDLER_STATUS_SLEEP_REQ_ACK_TO_LPM_SHFT                                                            4
#define HWIO_REO_R1_LPM_REQ_HANDLER_STATUS_SLEEP_REQ_FROM_LPM_BMSK                                                            0x8
#define HWIO_REO_R1_LPM_REQ_HANDLER_STATUS_SLEEP_REQ_FROM_LPM_SHFT                                                              3
#define HWIO_REO_R1_LPM_REQ_HANDLER_STATUS_SM_STATE_BMSK                                                                      0x7
#define HWIO_REO_R1_LPM_REQ_HANDLER_STATUS_SM_STATE_SHFT                                                                        0

#define HWIO_REO_R1_TESTBUS_CAPTURE_n_ADDR(base,n)                                                                     ((base) + 0X20D0 + (0x4*(n)))
#define HWIO_REO_R1_TESTBUS_CAPTURE_n_PHYS(base,n)                                                                     ((base) + 0X20D0 + (0x4*(n)))
#define HWIO_REO_R1_TESTBUS_CAPTURE_n_OFFS(n)                                                                          (0X20D0 + (0x4*(n)))
#define HWIO_REO_R1_TESTBUS_CAPTURE_n_RMSK                                                                             0xffffffff
#define HWIO_REO_R1_TESTBUS_CAPTURE_n_MAXn                                                                                    255
#define HWIO_REO_R1_TESTBUS_CAPTURE_n_POR                                                                              0x00000000
#define HWIO_REO_R1_TESTBUS_CAPTURE_n_POR_RMSK                                                                         0xffffffff
#define HWIO_REO_R1_TESTBUS_CAPTURE_n_ATTR                                                                                          0x1
#define HWIO_REO_R1_TESTBUS_CAPTURE_n_INI(base,n)                \
                in_dword_masked(HWIO_REO_R1_TESTBUS_CAPTURE_n_ADDR(base,n), HWIO_REO_R1_TESTBUS_CAPTURE_n_RMSK)
#define HWIO_REO_R1_TESTBUS_CAPTURE_n_INMI(base,n,mask)        \
                in_dword_masked(HWIO_REO_R1_TESTBUS_CAPTURE_n_ADDR(base,n), mask)
#define HWIO_REO_R1_TESTBUS_CAPTURE_n_DATA_BMSK                                                                        0xffffffff
#define HWIO_REO_R1_TESTBUS_CAPTURE_n_DATA_SHFT                                                                                 0

#define HWIO_REO_R2_REO_CMD_RING_HP_ADDR(x)                                                                            ((x) + 0x3020)
#define HWIO_REO_R2_SW2REO_RING_HP_ADDR(x)                                                                             ((x) + 0x3028)
#define HWIO_REO_R2_REO2SW1_RING_HP_ADDR(x)                                                                            ((x) + 0x3048)
#define HWIO_REO_R2_REO2SW1_RING_TP_ADDR(x)                                                                            ((x) + 0x304c)
#define HWIO_REO_R2_REO2SW2_RING_HP_ADDR(x)                                                                            ((x) + 0x3050)
#define HWIO_REO_R2_REO2SW0_RING_HP_ADDR(x)                                                                            ((x) + 0x3088)
#define HWIO_REO_R2_REO_STATUS_RING_HP_ADDR(x)                                                                         ((x) + 0x30a8)
#define HWIO_TQM_R0_PAUSE_CONTROL_ENABLE_HW_WRITE_CMD_BMSK                                                             0x80
#define HWIO_TQM_R0_PAUSE_CONTROL_ENABLE_HW_WRITE_CMD_SHFT                                                                7
#define HWIO_TQM_R0_PAUSE_CONTROL_ENABLE_HW_UPDATE_TX_MPDU_CNT_BMSK                                                    0x40
#define HWIO_TQM_R0_PAUSE_CONTROL_ENABLE_HW_UPDATE_TX_MPDU_CNT_SHFT                                                       6
#define HWIO_TQM_R0_PAUSE_CONTROL_ENABLE_HW_GEN_MPDU_LEN_LIST_BMSK                                                     0x20
#define HWIO_TQM_R0_PAUSE_CONTROL_ENABLE_HW_GEN_MPDU_LEN_LIST_SHFT                                                        5
#define HWIO_TQM_R0_PAUSE_CONTROL_ENABLE_HW_REMOVE_MPDU_BMSK                                                           0x10
#define HWIO_TQM_R0_PAUSE_CONTROL_ENABLE_HW_REMOVE_MPDU_SHFT                                                              4
#define HWIO_TQM_R0_PAUSE_CONTROL_ENABLE_HW_ACKED_1K_MPDU_BMSK                                                          0x8
#define HWIO_TQM_R0_PAUSE_CONTROL_ENABLE_HW_ACKED_1K_MPDU_SHFT                                                            3
#define HWIO_TQM_R0_TCL2TQM_RING_MISC_TRANSACTION_TYPE_BMSK                                                        0x400000
#define HWIO_TQM_R0_TCL2TQM_RING_MISC_TRANSACTION_TYPE_SHFT                                                              22
#define HWIO_TQM_R0_FW2TQM_RING_MISC_TRANSACTION_TYPE_BMSK                                                         0x400000
#define HWIO_TQM_R0_FW2TQM_RING_MISC_TRANSACTION_TYPE_SHFT                                                               22
#define HWIO_TQM_R0_SW_CMD_RING_MISC_TRANSACTION_TYPE_BMSK                                                         0x400000
#define HWIO_TQM_R0_SW_CMD_RING_MISC_TRANSACTION_TYPE_SHFT                                                               22
#define HWIO_TQM_R0_SW_CMD1_RING_MISC_TRANSACTION_TYPE_BMSK                                                        0x400000
#define HWIO_TQM_R0_SW_CMD1_RING_MISC_TRANSACTION_TYPE_SHFT                                                              22
#define HWIO_TQM_R0_WBM2TQM_LINK_RING_MISC_TRANSACTION_TYPE_BMSK                                                   0x400000
#define HWIO_TQM_R0_WBM2TQM_LINK_RING_MISC_TRANSACTION_TYPE_SHFT                                                         22
#define HWIO_TQM_R0_TQM_RELEASE_RING_MISC_TRANSACTION_TYPE_BMSK                                                   0x8000000
#define HWIO_TQM_R0_TQM_RELEASE_RING_MISC_TRANSACTION_TYPE_SHFT                                                          27
#define HWIO_TQM_R0_TQM_STATUS_RING_MISC_TRANSACTION_TYPE_BMSK                                                    0x8000000
#define HWIO_TQM_R0_TQM_STATUS_RING_MISC_TRANSACTION_TYPE_SHFT                                                           27
#define HWIO_TQM_R0_TQM_STATUS1_RING_MISC_TRANSACTION_TYPE_BMSK                                                   0x8000000
#define HWIO_TQM_R0_TQM_STATUS1_RING_MISC_TRANSACTION_TYPE_SHFT                                                          27
#define HWIO_TQM_R0_TQM_MULTI_SRNG_PROD_RING_MISC_COMMON_ADDR(x)                                                 ((x) + 0x3f4)
#define HWIO_TQM_R0_TQM_MULTI_SRNG_PROD_RING_MISC_COMMON_PHYS(x)                                                 ((x) + 0x3f4)
#define HWIO_TQM_R0_TQM_MULTI_SRNG_PROD_RING_MISC_COMMON_OFFS                                                    (0x3f4)
#define HWIO_TQM_R0_TQM_MULTI_SRNG_PROD_RING_MISC_COMMON_RMSK                                                      0x1fffff
#define HWIO_TQM_R0_TQM_MULTI_SRNG_PROD_RING_MISC_COMMON_POR                                                     0x00001000
#define HWIO_TQM_R0_TQM_MULTI_SRNG_PROD_RING_MISC_COMMON_POR_RMSK                                                0xffffffff
#define HWIO_TQM_R0_TQM_MULTI_SRNG_PROD_RING_MISC_COMMON_ATTR                                                                 0x3
#define HWIO_TQM_R0_TQM_MULTI_SRNG_PROD_RING_MISC_COMMON_IN(x)            \
                in_dword(HWIO_TQM_R0_TQM_MULTI_SRNG_PROD_RING_MISC_COMMON_ADDR(x))
#define HWIO_TQM_R0_TQM_MULTI_SRNG_PROD_RING_MISC_COMMON_INM(x, m)            \
                in_dword_masked(HWIO_TQM_R0_TQM_MULTI_SRNG_PROD_RING_MISC_COMMON_ADDR(x), m)
#define HWIO_TQM_R0_TQM_MULTI_SRNG_PROD_RING_MISC_COMMON_OUT(x, v)            \
                out_dword(HWIO_TQM_R0_TQM_MULTI_SRNG_PROD_RING_MISC_COMMON_ADDR(x),v)
#define HWIO_TQM_R0_TQM_MULTI_SRNG_PROD_RING_MISC_COMMON_OUTM(x,m,v) \
                out_dword_masked_ns(HWIO_TQM_R0_TQM_MULTI_SRNG_PROD_RING_MISC_COMMON_ADDR(x),m,v,HWIO_TQM_R0_TQM_MULTI_SRNG_PROD_RING_MISC_COMMON_IN(x))
#define HWIO_TQM_R0_TQM_MULTI_SRNG_PROD_RING_MISC_COMMON_MISC_CONTROL_BMSK                                         0x1fe000
#define HWIO_TQM_R0_TQM_MULTI_SRNG_PROD_RING_MISC_COMMON_MISC_CONTROL_SHFT                                               13
#define HWIO_TQM_R0_TQM_MULTI_SRNG_PROD_RING_MISC_COMMON_SRNG_IS_IDLE_BMSK                                           0x1000
#define HWIO_TQM_R0_TQM_MULTI_SRNG_PROD_RING_MISC_COMMON_SRNG_IS_IDLE_SHFT                                               12
#define HWIO_TQM_R0_TQM_MULTI_SRNG_PROD_RING_MISC_COMMON_SRNG_SM1_STATE2_BMSK                                         0xc00
#define HWIO_TQM_R0_TQM_MULTI_SRNG_PROD_RING_MISC_COMMON_SRNG_SM1_STATE2_SHFT                                            10
#define HWIO_TQM_R0_TQM_MULTI_SRNG_PROD_RING_MISC_COMMON_SRNG_SM1_STATE1_BMSK                                         0x3c0
#define HWIO_TQM_R0_TQM_MULTI_SRNG_PROD_RING_MISC_COMMON_SRNG_SM1_STATE1_SHFT                                             6
#define HWIO_TQM_R0_TQM_MULTI_SRNG_PROD_RING_MISC_COMMON_SRNG_SM0_STATE2_BMSK                                          0x30
#define HWIO_TQM_R0_TQM_MULTI_SRNG_PROD_RING_MISC_COMMON_SRNG_SM0_STATE2_SHFT                                             4
#define HWIO_TQM_R0_TQM_MULTI_SRNG_PROD_RING_MISC_COMMON_SRNG_SM0_STATE1_BMSK                                           0xf
#define HWIO_TQM_R0_TQM_MULTI_SRNG_PROD_RING_MISC_COMMON_SRNG_SM0_STATE1_SHFT                                             0

#define HWIO_TQM_R0_TQM_MULTI_SRNG_PROD_RING_MISC_COMMON_EXT_ADDR(x)                                             ((x) + 0x3f8)
#define HWIO_TQM_R0_TQM_MULTI_SRNG_PROD_RING_MISC_COMMON_EXT_PHYS(x)                                             ((x) + 0x3f8)
#define HWIO_TQM_R0_TQM_MULTI_SRNG_PROD_RING_MISC_COMMON_EXT_OFFS                                                (0x3f8)
#define HWIO_TQM_R0_TQM_MULTI_SRNG_PROD_RING_MISC_COMMON_EXT_RMSK                                                  0xffffff
#define HWIO_TQM_R0_TQM_MULTI_SRNG_PROD_RING_MISC_COMMON_EXT_POR                                                 0x00000fff
#define HWIO_TQM_R0_TQM_MULTI_SRNG_PROD_RING_MISC_COMMON_EXT_POR_RMSK                                            0xffffffff
#define HWIO_TQM_R0_TQM_MULTI_SRNG_PROD_RING_MISC_COMMON_EXT_ATTR                                                             0x3
#define HWIO_TQM_R0_TQM_MULTI_SRNG_PROD_RING_MISC_COMMON_EXT_IN(x)            \
                in_dword(HWIO_TQM_R0_TQM_MULTI_SRNG_PROD_RING_MISC_COMMON_EXT_ADDR(x))
#define HWIO_TQM_R0_TQM_MULTI_SRNG_PROD_RING_MISC_COMMON_EXT_INM(x, m)            \
                in_dword_masked(HWIO_TQM_R0_TQM_MULTI_SRNG_PROD_RING_MISC_COMMON_EXT_ADDR(x), m)
#define HWIO_TQM_R0_TQM_MULTI_SRNG_PROD_RING_MISC_COMMON_EXT_OUT(x, v)            \
                out_dword(HWIO_TQM_R0_TQM_MULTI_SRNG_PROD_RING_MISC_COMMON_EXT_ADDR(x),v)
#define HWIO_TQM_R0_TQM_MULTI_SRNG_PROD_RING_MISC_COMMON_EXT_OUTM(x,m,v) \
                out_dword_masked_ns(HWIO_TQM_R0_TQM_MULTI_SRNG_PROD_RING_MISC_COMMON_EXT_ADDR(x),m,v,HWIO_TQM_R0_TQM_MULTI_SRNG_PROD_RING_MISC_COMMON_EXT_IN(x))
#define HWIO_TQM_R0_TQM_MULTI_SRNG_PROD_RING_MISC_COMMON_EXT_RING_WATCHDOG_TIMER_STATUS_BMSK                       0xfff000
#define HWIO_TQM_R0_TQM_MULTI_SRNG_PROD_RING_MISC_COMMON_EXT_RING_WATCHDOG_TIMER_STATUS_SHFT                             12
#define HWIO_TQM_R0_TQM_MULTI_SRNG_PROD_RING_MISC_COMMON_EXT_WATCHDOG_TIMEOUT_VALUE_BMSK                              0xfff
#define HWIO_TQM_R0_TQM_MULTI_SRNG_PROD_RING_MISC_COMMON_EXT_WATCHDOG_TIMEOUT_VALUE_SHFT                                  0

#define HWIO_TQM_R0_TQM_MULTI_SRNG_CONS_RING_MISC_COMMON_ADDR(x)                                                 ((x) + 0x3fc)
#define HWIO_TQM_R0_TQM_MULTI_SRNG_CONS_RING_MISC_COMMON_PHYS(x)                                                 ((x) + 0x3fc)
#define HWIO_TQM_R0_TQM_MULTI_SRNG_CONS_RING_MISC_COMMON_OFFS                                                    (0x3fc)
#define HWIO_TQM_R0_TQM_MULTI_SRNG_CONS_RING_MISC_COMMON_RMSK                                                      0x1fffff
#define HWIO_TQM_R0_TQM_MULTI_SRNG_CONS_RING_MISC_COMMON_POR                                                     0x00001000
#define HWIO_TQM_R0_TQM_MULTI_SRNG_CONS_RING_MISC_COMMON_POR_RMSK                                                0xffffffff
#define HWIO_TQM_R0_TQM_MULTI_SRNG_CONS_RING_MISC_COMMON_ATTR                                                                 0x3
#define HWIO_TQM_R0_TQM_MULTI_SRNG_CONS_RING_MISC_COMMON_IN(x)            \
                in_dword(HWIO_TQM_R0_TQM_MULTI_SRNG_CONS_RING_MISC_COMMON_ADDR(x))
#define HWIO_TQM_R0_TQM_MULTI_SRNG_CONS_RING_MISC_COMMON_INM(x, m)            \
                in_dword_masked(HWIO_TQM_R0_TQM_MULTI_SRNG_CONS_RING_MISC_COMMON_ADDR(x), m)
#define HWIO_TQM_R0_TQM_MULTI_SRNG_CONS_RING_MISC_COMMON_OUT(x, v)            \
                out_dword(HWIO_TQM_R0_TQM_MULTI_SRNG_CONS_RING_MISC_COMMON_ADDR(x),v)
#define HWIO_TQM_R0_TQM_MULTI_SRNG_CONS_RING_MISC_COMMON_OUTM(x,m,v) \
                out_dword_masked_ns(HWIO_TQM_R0_TQM_MULTI_SRNG_CONS_RING_MISC_COMMON_ADDR(x),m,v,HWIO_TQM_R0_TQM_MULTI_SRNG_CONS_RING_MISC_COMMON_IN(x))
#define HWIO_TQM_R0_TQM_MULTI_SRNG_CONS_RING_MISC_COMMON_MISC_CONTROL_BMSK                                         0x1fe000
#define HWIO_TQM_R0_TQM_MULTI_SRNG_CONS_RING_MISC_COMMON_MISC_CONTROL_SHFT                                               13
#define HWIO_TQM_R0_TQM_MULTI_SRNG_CONS_RING_MISC_COMMON_SRNG_IS_IDLE_BMSK                                           0x1000
#define HWIO_TQM_R0_TQM_MULTI_SRNG_CONS_RING_MISC_COMMON_SRNG_IS_IDLE_SHFT                                               12
#define HWIO_TQM_R0_TQM_MULTI_SRNG_CONS_RING_MISC_COMMON_SRNG_SM1_STATE2_BMSK                                         0xc00
#define HWIO_TQM_R0_TQM_MULTI_SRNG_CONS_RING_MISC_COMMON_SRNG_SM1_STATE2_SHFT                                            10
#define HWIO_TQM_R0_TQM_MULTI_SRNG_CONS_RING_MISC_COMMON_SRNG_SM1_STATE1_BMSK                                         0x3c0
#define HWIO_TQM_R0_TQM_MULTI_SRNG_CONS_RING_MISC_COMMON_SRNG_SM1_STATE1_SHFT                                             6
#define HWIO_TQM_R0_TQM_MULTI_SRNG_CONS_RING_MISC_COMMON_SRNG_SM0_STATE2_BMSK                                          0x30
#define HWIO_TQM_R0_TQM_MULTI_SRNG_CONS_RING_MISC_COMMON_SRNG_SM0_STATE2_SHFT                                             4
#define HWIO_TQM_R0_TQM_MULTI_SRNG_CONS_RING_MISC_COMMON_SRNG_SM0_STATE1_BMSK                                           0xf
#define HWIO_TQM_R0_TQM_MULTI_SRNG_CONS_RING_MISC_COMMON_SRNG_SM0_STATE1_SHFT                                             0

#define HWIO_TQM_R0_TQM_MULTI_SRNG_CONS_RING_MISC_COMMON_EXT_ADDR(x)                                             ((x) + 0x400)
#define HWIO_TQM_R0_TQM_MULTI_SRNG_CONS_RING_MISC_COMMON_EXT_PHYS(x)                                             ((x) + 0x400)
#define HWIO_TQM_R0_TQM_MULTI_SRNG_CONS_RING_MISC_COMMON_EXT_OFFS                                                (0x400)
#define HWIO_TQM_R0_TQM_MULTI_SRNG_CONS_RING_MISC_COMMON_EXT_RMSK                                                  0xffffff
#define HWIO_TQM_R0_TQM_MULTI_SRNG_CONS_RING_MISC_COMMON_EXT_POR                                                 0x00000fff
#define HWIO_TQM_R0_TQM_MULTI_SRNG_CONS_RING_MISC_COMMON_EXT_POR_RMSK                                            0xffffffff
#define HWIO_TQM_R0_TQM_MULTI_SRNG_CONS_RING_MISC_COMMON_EXT_ATTR                                                             0x3
#define HWIO_TQM_R0_TQM_MULTI_SRNG_CONS_RING_MISC_COMMON_EXT_IN(x)            \
                in_dword(HWIO_TQM_R0_TQM_MULTI_SRNG_CONS_RING_MISC_COMMON_EXT_ADDR(x))
#define HWIO_TQM_R0_TQM_MULTI_SRNG_CONS_RING_MISC_COMMON_EXT_INM(x, m)            \
                in_dword_masked(HWIO_TQM_R0_TQM_MULTI_SRNG_CONS_RING_MISC_COMMON_EXT_ADDR(x), m)
#define HWIO_TQM_R0_TQM_MULTI_SRNG_CONS_RING_MISC_COMMON_EXT_OUT(x, v)            \
                out_dword(HWIO_TQM_R0_TQM_MULTI_SRNG_CONS_RING_MISC_COMMON_EXT_ADDR(x),v)
#define HWIO_TQM_R0_TQM_MULTI_SRNG_CONS_RING_MISC_COMMON_EXT_OUTM(x,m,v) \
                out_dword_masked_ns(HWIO_TQM_R0_TQM_MULTI_SRNG_CONS_RING_MISC_COMMON_EXT_ADDR(x),m,v,HWIO_TQM_R0_TQM_MULTI_SRNG_CONS_RING_MISC_COMMON_EXT_IN(x))
#define HWIO_TQM_R0_TQM_MULTI_SRNG_CONS_RING_MISC_COMMON_EXT_RING_WATCHDOG_TIMER_STATUS_BMSK                       0xfff000
#define HWIO_TQM_R0_TQM_MULTI_SRNG_CONS_RING_MISC_COMMON_EXT_RING_WATCHDOG_TIMER_STATUS_SHFT                             12
#define HWIO_TQM_R0_TQM_MULTI_SRNG_CONS_RING_MISC_COMMON_EXT_WATCHDOG_TIMEOUT_VALUE_BMSK                              0xfff
#define HWIO_TQM_R0_TQM_MULTI_SRNG_CONS_RING_MISC_COMMON_EXT_WATCHDOG_TIMEOUT_VALUE_SHFT                                  0

#define HWIO_TQM_R0_MISC_CFG_ENABLE_ROUTING_CHECKS_BMSK                                                          0x80000000
#define HWIO_TQM_R0_MISC_CFG_ENABLE_ROUTING_CHECKS_SHFT                                                                  31
#define HWIO_TQM_R0_MISC_CFG_1_ENABLE_ONE_SHOT_DELAY_BMSK                                                            0x8000
#define HWIO_TQM_R0_MISC_CFG_1_ENABLE_ONE_SHOT_DELAY_SHFT                                                                15
#define HWIO_TQM_R0_MISC_CFG_1_ENABLE_DELAYED_PROCESSING_BMSK                                                        0x4000
#define HWIO_TQM_R0_MISC_CFG_1_ENABLE_DELAYED_PROCESSING_SHFT                                                            14
#define HWIO_TQM_R0_MISC_CFG_1_BYPASS_MLO_FILTER_BMSK                                                                0x2000
#define HWIO_TQM_R0_MISC_CFG_1_BYPASS_MLO_FILTER_SHFT                                                                    13
#define HWIO_TQM_R0_MISC_CFG_1_BYPASS_NON_MLO_FILTER_BMSK                                                            0x1000
#define HWIO_TQM_R0_MISC_CFG_1_BYPASS_NON_MLO_FILTER_SHFT                                                                12
#define HWIO_TQM_R0_MISC_CFG_1_DISABLE_MLO_PDG_UPDATE_TX_COUNT_CMD_BMSK                                               0x800
#define HWIO_TQM_R0_MISC_CFG_1_DISABLE_MLO_PDG_UPDATE_TX_COUNT_CMD_SHFT                                                  11
#define HWIO_TQM_R0_MISC_CFG_1_DISABLE_MLO_OWNER_BASED_ACK_PROCESS_BMSK                                               0x400
#define HWIO_TQM_R0_MISC_CFG_1_DISABLE_MLO_OWNER_BASED_ACK_PROCESS_SHFT                                                  10
#define HWIO_TQM_R0_LPM_FW_CTRL_ADDR(x)                                                                          ((x) + 0x480)
#define HWIO_TQM_R0_LPM_FW_CTRL_PHYS(x)                                                                          ((x) + 0x480)
#define HWIO_TQM_R0_LPM_FW_CTRL_OFFS                                                                             (0x480)
#define HWIO_TQM_R0_LPM_FW_CTRL_RMSK                                                                                    0xf
#define HWIO_TQM_R0_LPM_FW_CTRL_POR                                                                              0x00000000
#define HWIO_TQM_R0_LPM_FW_CTRL_POR_RMSK                                                                         0xffffffff
#define HWIO_TQM_R0_LPM_FW_CTRL_ATTR                                                                                          0x3
#define HWIO_TQM_R0_LPM_FW_CTRL_IN(x)            \
                in_dword(HWIO_TQM_R0_LPM_FW_CTRL_ADDR(x))
#define HWIO_TQM_R0_LPM_FW_CTRL_INM(x, m)            \
                in_dword_masked(HWIO_TQM_R0_LPM_FW_CTRL_ADDR(x), m)
#define HWIO_TQM_R0_LPM_FW_CTRL_OUT(x, v)            \
                out_dword(HWIO_TQM_R0_LPM_FW_CTRL_ADDR(x),v)
#define HWIO_TQM_R0_LPM_FW_CTRL_OUTM(x,m,v) \
                out_dword_masked_ns(HWIO_TQM_R0_LPM_FW_CTRL_ADDR(x),m,v,HWIO_TQM_R0_LPM_FW_CTRL_IN(x))
#define HWIO_TQM_R0_LPM_FW_CTRL_SLEEP_REQ_ACK_OVR_MULTI_SRNG_P_BMSK                                                     0x8
#define HWIO_TQM_R0_LPM_FW_CTRL_SLEEP_REQ_ACK_OVR_MULTI_SRNG_P_SHFT                                                       3
#define HWIO_TQM_R0_LPM_FW_CTRL_SLEEP_REQ_ACK_OVR_MULTI_SRNG_C_BMSK                                                     0x4
#define HWIO_TQM_R0_LPM_FW_CTRL_SLEEP_REQ_ACK_OVR_MULTI_SRNG_C_SHFT                                                       2
#define HWIO_TQM_R0_LPM_FW_CTRL_SLEEP_REQ_ACK_OVR_TOP_BMSK                                                              0x2
#define HWIO_TQM_R0_LPM_FW_CTRL_SLEEP_REQ_ACK_OVR_TOP_SHFT                                                                1
#define HWIO_TQM_R0_LPM_FW_CTRL_SLEEP_REQ_BMSK                                                                          0x1
#define HWIO_TQM_R0_LPM_FW_CTRL_SLEEP_REQ_SHFT                                                                            0

#define HWIO_TQM_R0_CLKGATE_CTRL_2_ADDR(x)                                                                       ((x) + 0x484)
#define HWIO_TQM_R0_CLKGATE_CTRL_2_PHYS(x)                                                                       ((x) + 0x484)
#define HWIO_TQM_R0_CLKGATE_CTRL_2_OFFS                                                                          (0x484)
#define HWIO_TQM_R0_CLKGATE_CTRL_2_RMSK                                                                                 0x3
#define HWIO_TQM_R0_CLKGATE_CTRL_2_POR                                                                           0x00000000
#define HWIO_TQM_R0_CLKGATE_CTRL_2_POR_RMSK                                                                      0xffffffff
#define HWIO_TQM_R0_CLKGATE_CTRL_2_ATTR                                                                                       0x3
#define HWIO_TQM_R0_CLKGATE_CTRL_2_IN(x)            \
                in_dword(HWIO_TQM_R0_CLKGATE_CTRL_2_ADDR(x))
#define HWIO_TQM_R0_CLKGATE_CTRL_2_INM(x, m)            \
                in_dword_masked(HWIO_TQM_R0_CLKGATE_CTRL_2_ADDR(x), m)
#define HWIO_TQM_R0_CLKGATE_CTRL_2_OUT(x, v)            \
                out_dword(HWIO_TQM_R0_CLKGATE_CTRL_2_ADDR(x),v)
#define HWIO_TQM_R0_CLKGATE_CTRL_2_OUTM(x,m,v) \
                out_dword_masked_ns(HWIO_TQM_R0_CLKGATE_CTRL_2_ADDR(x),m,v,HWIO_TQM_R0_CLKGATE_CTRL_2_IN(x))
#define HWIO_TQM_R0_CLKGATE_CTRL_2_CLKGATE_DISABLE_TESTBUS_APB_CAPTURE_BMSK                                             0x2
#define HWIO_TQM_R0_CLKGATE_CTRL_2_CLKGATE_DISABLE_TESTBUS_APB_CAPTURE_SHFT                                               1
#define HWIO_TQM_R0_CLKGATE_CTRL_2_CLKGATE_DISABLE_LPM_HANDLER_BMSK                                                     0x1
#define HWIO_TQM_R0_CLKGATE_CTRL_2_CLKGATE_DISABLE_LPM_HANDLER_SHFT                                                       0

#define HWIO_TQM_R0_CLKGATE_CTRL_TQM_MULTI_SRNG_DISABLE_BMSK                                                     0x20000000
#define HWIO_TQM_R0_CLKGATE_CTRL_TQM_MULTI_SRNG_DISABLE_SHFT                                                             29
#define HWIO_TQM_R0_DROPPED_MSDU_COUNT_VDEV_ID_MISMATCH_ADDR(x)                                                  ((x) + 0x508)
#define HWIO_TQM_R0_DROPPED_MSDU_COUNT_VDEV_ID_MISMATCH_PHYS(x)                                                  ((x) + 0x508)
#define HWIO_TQM_R0_DROPPED_MSDU_COUNT_VDEV_ID_MISMATCH_OFFS                                                     (0x508)
#define HWIO_TQM_R0_DROPPED_MSDU_COUNT_VDEV_ID_MISMATCH_RMSK                                                     0xffffffff
#define HWIO_TQM_R0_DROPPED_MSDU_COUNT_VDEV_ID_MISMATCH_POR                                                      0x00000000
#define HWIO_TQM_R0_DROPPED_MSDU_COUNT_VDEV_ID_MISMATCH_POR_RMSK                                                 0xffffffff
#define HWIO_TQM_R0_DROPPED_MSDU_COUNT_VDEV_ID_MISMATCH_ATTR                                                                  0x3
#define HWIO_TQM_R0_DROPPED_MSDU_COUNT_VDEV_ID_MISMATCH_IN(x)            \
                in_dword(HWIO_TQM_R0_DROPPED_MSDU_COUNT_VDEV_ID_MISMATCH_ADDR(x))
#define HWIO_TQM_R0_DROPPED_MSDU_COUNT_VDEV_ID_MISMATCH_INM(x, m)            \
                in_dword_masked(HWIO_TQM_R0_DROPPED_MSDU_COUNT_VDEV_ID_MISMATCH_ADDR(x), m)
#define HWIO_TQM_R0_DROPPED_MSDU_COUNT_VDEV_ID_MISMATCH_OUT(x, v)            \
                out_dword(HWIO_TQM_R0_DROPPED_MSDU_COUNT_VDEV_ID_MISMATCH_ADDR(x),v)
#define HWIO_TQM_R0_DROPPED_MSDU_COUNT_VDEV_ID_MISMATCH_OUTM(x,m,v) \
                out_dword_masked_ns(HWIO_TQM_R0_DROPPED_MSDU_COUNT_VDEV_ID_MISMATCH_ADDR(x),m,v,HWIO_TQM_R0_DROPPED_MSDU_COUNT_VDEV_ID_MISMATCH_IN(x))
#define HWIO_TQM_R0_DROPPED_MSDU_COUNT_VDEV_ID_MISMATCH_VALUE_BMSK                                               0xffffffff
#define HWIO_TQM_R0_DROPPED_MSDU_COUNT_VDEV_ID_MISMATCH_VALUE_SHFT                                                        0

#define HWIO_TQM_R0_DROPPED_MSDU_BYTE_COUNT_VDEV_ID_MISMATCH_0_ADDR(x)                                           ((x) + 0x50c)
#define HWIO_TQM_R0_DROPPED_MSDU_BYTE_COUNT_VDEV_ID_MISMATCH_0_PHYS(x)                                           ((x) + 0x50c)
#define HWIO_TQM_R0_DROPPED_MSDU_BYTE_COUNT_VDEV_ID_MISMATCH_0_OFFS                                              (0x50c)
#define HWIO_TQM_R0_DROPPED_MSDU_BYTE_COUNT_VDEV_ID_MISMATCH_0_RMSK                                              0xffffffff
#define HWIO_TQM_R0_DROPPED_MSDU_BYTE_COUNT_VDEV_ID_MISMATCH_0_POR                                               0x00000000
#define HWIO_TQM_R0_DROPPED_MSDU_BYTE_COUNT_VDEV_ID_MISMATCH_0_POR_RMSK                                          0xffffffff
#define HWIO_TQM_R0_DROPPED_MSDU_BYTE_COUNT_VDEV_ID_MISMATCH_0_ATTR                                                           0x3
#define HWIO_TQM_R0_DROPPED_MSDU_BYTE_COUNT_VDEV_ID_MISMATCH_0_IN(x)            \
                in_dword(HWIO_TQM_R0_DROPPED_MSDU_BYTE_COUNT_VDEV_ID_MISMATCH_0_ADDR(x))
#define HWIO_TQM_R0_DROPPED_MSDU_BYTE_COUNT_VDEV_ID_MISMATCH_0_INM(x, m)            \
                in_dword_masked(HWIO_TQM_R0_DROPPED_MSDU_BYTE_COUNT_VDEV_ID_MISMATCH_0_ADDR(x), m)
#define HWIO_TQM_R0_DROPPED_MSDU_BYTE_COUNT_VDEV_ID_MISMATCH_0_OUT(x, v)            \
                out_dword(HWIO_TQM_R0_DROPPED_MSDU_BYTE_COUNT_VDEV_ID_MISMATCH_0_ADDR(x),v)
#define HWIO_TQM_R0_DROPPED_MSDU_BYTE_COUNT_VDEV_ID_MISMATCH_0_OUTM(x,m,v) \
                out_dword_masked_ns(HWIO_TQM_R0_DROPPED_MSDU_BYTE_COUNT_VDEV_ID_MISMATCH_0_ADDR(x),m,v,HWIO_TQM_R0_DROPPED_MSDU_BYTE_COUNT_VDEV_ID_MISMATCH_0_IN(x))
#define HWIO_TQM_R0_DROPPED_MSDU_BYTE_COUNT_VDEV_ID_MISMATCH_0_VALUE_BMSK                                        0xffffffff
#define HWIO_TQM_R0_DROPPED_MSDU_BYTE_COUNT_VDEV_ID_MISMATCH_0_VALUE_SHFT                                                 0

#define HWIO_TQM_R0_DROPPED_MSDU_BYTE_COUNT_VDEV_ID_MISMATCH_1_ADDR(x)                                           ((x) + 0x510)
#define HWIO_TQM_R0_DROPPED_MSDU_BYTE_COUNT_VDEV_ID_MISMATCH_1_PHYS(x)                                           ((x) + 0x510)
#define HWIO_TQM_R0_DROPPED_MSDU_BYTE_COUNT_VDEV_ID_MISMATCH_1_OFFS                                              (0x510)
#define HWIO_TQM_R0_DROPPED_MSDU_BYTE_COUNT_VDEV_ID_MISMATCH_1_RMSK                                              0xffffffff
#define HWIO_TQM_R0_DROPPED_MSDU_BYTE_COUNT_VDEV_ID_MISMATCH_1_POR                                               0x00000000
#define HWIO_TQM_R0_DROPPED_MSDU_BYTE_COUNT_VDEV_ID_MISMATCH_1_POR_RMSK                                          0xffffffff
#define HWIO_TQM_R0_DROPPED_MSDU_BYTE_COUNT_VDEV_ID_MISMATCH_1_ATTR                                                           0x3
#define HWIO_TQM_R0_DROPPED_MSDU_BYTE_COUNT_VDEV_ID_MISMATCH_1_IN(x)            \
                in_dword(HWIO_TQM_R0_DROPPED_MSDU_BYTE_COUNT_VDEV_ID_MISMATCH_1_ADDR(x))
#define HWIO_TQM_R0_DROPPED_MSDU_BYTE_COUNT_VDEV_ID_MISMATCH_1_INM(x, m)            \
                in_dword_masked(HWIO_TQM_R0_DROPPED_MSDU_BYTE_COUNT_VDEV_ID_MISMATCH_1_ADDR(x), m)
#define HWIO_TQM_R0_DROPPED_MSDU_BYTE_COUNT_VDEV_ID_MISMATCH_1_OUT(x, v)            \
                out_dword(HWIO_TQM_R0_DROPPED_MSDU_BYTE_COUNT_VDEV_ID_MISMATCH_1_ADDR(x),v)
#define HWIO_TQM_R0_DROPPED_MSDU_BYTE_COUNT_VDEV_ID_MISMATCH_1_OUTM(x,m,v) \
                out_dword_masked_ns(HWIO_TQM_R0_DROPPED_MSDU_BYTE_COUNT_VDEV_ID_MISMATCH_1_ADDR(x),m,v,HWIO_TQM_R0_DROPPED_MSDU_BYTE_COUNT_VDEV_ID_MISMATCH_1_IN(x))
#define HWIO_TQM_R0_DROPPED_MSDU_BYTE_COUNT_VDEV_ID_MISMATCH_1_VALUE_BMSK                                        0xffffffff
#define HWIO_TQM_R0_DROPPED_MSDU_BYTE_COUNT_VDEV_ID_MISMATCH_1_VALUE_SHFT                                                 0

#define HWIO_TQM_R0_WATCHDOG_SRNG_ADDR(x)                                                                        ((x) + 0x51c)
#define HWIO_TQM_R0_WATCHDOG_SRNG_PHYS(x)                                                                        ((x) + 0x51c)
#define HWIO_TQM_R0_WATCHDOG_SRNG_OFFS                                                                           (0x51c)
#define HWIO_TQM_R0_WATCHDOG_SRNG_RMSK                                                                                0xfff
#define HWIO_TQM_R0_WATCHDOG_SRNG_POR                                                                            0x00000710
#define HWIO_TQM_R0_WATCHDOG_SRNG_POR_RMSK                                                                       0xffffffff
#define HWIO_TQM_R0_WATCHDOG_SRNG_ATTR                                                                                        0x3
#define HWIO_TQM_R0_WATCHDOG_SRNG_IN(x)            \
                in_dword(HWIO_TQM_R0_WATCHDOG_SRNG_ADDR(x))
#define HWIO_TQM_R0_WATCHDOG_SRNG_INM(x, m)            \
                in_dword_masked(HWIO_TQM_R0_WATCHDOG_SRNG_ADDR(x), m)
#define HWIO_TQM_R0_WATCHDOG_SRNG_OUT(x, v)            \
                out_dword(HWIO_TQM_R0_WATCHDOG_SRNG_ADDR(x),v)
#define HWIO_TQM_R0_WATCHDOG_SRNG_OUTM(x,m,v) \
                out_dword_masked_ns(HWIO_TQM_R0_WATCHDOG_SRNG_ADDR(x),m,v,HWIO_TQM_R0_WATCHDOG_SRNG_IN(x))
#define HWIO_TQM_R0_WATCHDOG_SRNG_LIMIT_BMSK                                                                          0xfff
#define HWIO_TQM_R0_WATCHDOG_SRNG_LIMIT_SHFT                                                                              0

#define HWIO_TQM_R1_CACHE_CTL_DEBUG_OWNER_CHECK_ADDR_HIGH_ADDR(x)                                                ((x) + 0x204c)
#define HWIO_TQM_R1_CACHE_CTL_DEBUG_OWNER_CHECK_ADDR_HIGH_PHYS(x)                                                ((x) + 0x204c)
#define HWIO_TQM_R1_CACHE_CTL_DEBUG_OWNER_CHECK_ADDR_HIGH_OFFS                                                   (0x204c)
#define HWIO_TQM_R1_CACHE_CTL_DEBUG_OWNER_CHECK_ADDR_HIGH_RMSK                                                         0xff
#define HWIO_TQM_R1_CACHE_CTL_DEBUG_OWNER_CHECK_ADDR_HIGH_POR                                                    0x00000000
#define HWIO_TQM_R1_CACHE_CTL_DEBUG_OWNER_CHECK_ADDR_HIGH_POR_RMSK                                               0xffffffff
#define HWIO_TQM_R1_CACHE_CTL_DEBUG_OWNER_CHECK_ADDR_HIGH_ATTR                                                                0x1
#define HWIO_TQM_R1_CACHE_CTL_DEBUG_OWNER_CHECK_ADDR_HIGH_IN(x)            \
                in_dword(HWIO_TQM_R1_CACHE_CTL_DEBUG_OWNER_CHECK_ADDR_HIGH_ADDR(x))
#define HWIO_TQM_R1_CACHE_CTL_DEBUG_OWNER_CHECK_ADDR_HIGH_INM(x, m)            \
                in_dword_masked(HWIO_TQM_R1_CACHE_CTL_DEBUG_OWNER_CHECK_ADDR_HIGH_ADDR(x), m)
#define HWIO_TQM_R1_CACHE_CTL_DEBUG_OWNER_CHECK_ADDR_HIGH_ADDR_39_32_BMSK                                              0xff
#define HWIO_TQM_R1_CACHE_CTL_DEBUG_OWNER_CHECK_ADDR_HIGH_ADDR_39_32_SHFT                                                 0

#define HWIO_TQM_R1_CACHE_CTL_DEBUG_OWNER_CHECK_ADDR_LOW_ADDR(x)                                                 ((x) + 0x2050)
#define HWIO_TQM_R1_CACHE_CTL_DEBUG_OWNER_CHECK_ADDR_LOW_PHYS(x)                                                 ((x) + 0x2050)
#define HWIO_TQM_R1_CACHE_CTL_DEBUG_OWNER_CHECK_ADDR_LOW_OFFS                                                    (0x2050)
#define HWIO_TQM_R1_CACHE_CTL_DEBUG_OWNER_CHECK_ADDR_LOW_RMSK                                                    0xffffffff
#define HWIO_TQM_R1_CACHE_CTL_DEBUG_OWNER_CHECK_ADDR_LOW_POR                                                     0x00000000
#define HWIO_TQM_R1_CACHE_CTL_DEBUG_OWNER_CHECK_ADDR_LOW_POR_RMSK                                                0xffffffff
#define HWIO_TQM_R1_CACHE_CTL_DEBUG_OWNER_CHECK_ADDR_LOW_ATTR                                                                 0x1
#define HWIO_TQM_R1_CACHE_CTL_DEBUG_OWNER_CHECK_ADDR_LOW_IN(x)            \
                in_dword(HWIO_TQM_R1_CACHE_CTL_DEBUG_OWNER_CHECK_ADDR_LOW_ADDR(x))
#define HWIO_TQM_R1_CACHE_CTL_DEBUG_OWNER_CHECK_ADDR_LOW_INM(x, m)            \
                in_dword_masked(HWIO_TQM_R1_CACHE_CTL_DEBUG_OWNER_CHECK_ADDR_LOW_ADDR(x), m)
#define HWIO_TQM_R1_CACHE_CTL_DEBUG_OWNER_CHECK_ADDR_LOW_ADDR_31_0_BMSK                                          0xffffffff
#define HWIO_TQM_R1_CACHE_CTL_DEBUG_OWNER_CHECK_ADDR_LOW_ADDR_31_0_SHFT                                                   0

#define HWIO_TQM_R1_CACHE_CTL_DEBUG_CACHE_SNAPSHOT_ADDR_HIGH_ADDR(x)                                             ((x) + 0x2054)
#define HWIO_TQM_R1_CACHE_CTL_DEBUG_CACHE_SNAPSHOT_ADDR_HIGH_PHYS(x)                                             ((x) + 0x2054)
#define HWIO_TQM_R1_CACHE_CTL_DEBUG_CACHE_SNAPSHOT_ADDR_HIGH_OFFS                                                (0x2054)
#define HWIO_TQM_R1_CACHE_CTL_DEBUG_CACHE_SNAPSHOT_ADDR_HIGH_RMSK                                                      0xff
#define HWIO_TQM_R1_CACHE_CTL_DEBUG_CACHE_SNAPSHOT_ADDR_HIGH_POR                                                 0x00000000
#define HWIO_TQM_R1_CACHE_CTL_DEBUG_CACHE_SNAPSHOT_ADDR_HIGH_POR_RMSK                                            0xffffffff
#define HWIO_TQM_R1_CACHE_CTL_DEBUG_CACHE_SNAPSHOT_ADDR_HIGH_ATTR                                                             0x2
#define HWIO_TQM_R1_CACHE_CTL_DEBUG_CACHE_SNAPSHOT_ADDR_HIGH_OUT(x, v)            \
                out_dword(HWIO_TQM_R1_CACHE_CTL_DEBUG_CACHE_SNAPSHOT_ADDR_HIGH_ADDR(x),v)
#define HWIO_TQM_R1_CACHE_CTL_DEBUG_CACHE_SNAPSHOT_ADDR_HIGH_ADDR_39_32_BMSK                                           0xff
#define HWIO_TQM_R1_CACHE_CTL_DEBUG_CACHE_SNAPSHOT_ADDR_HIGH_ADDR_39_32_SHFT                                              0

#define HWIO_TQM_R1_CACHE_CTL_DEBUG_CACHE_SNAPSHOT_ADDR_LOW_ADDR(x)                                              ((x) + 0x2058)
#define HWIO_TQM_R1_CACHE_CTL_DEBUG_CACHE_SNAPSHOT_ADDR_LOW_PHYS(x)                                              ((x) + 0x2058)
#define HWIO_TQM_R1_CACHE_CTL_DEBUG_CACHE_SNAPSHOT_ADDR_LOW_OFFS                                                 (0x2058)
#define HWIO_TQM_R1_CACHE_CTL_DEBUG_CACHE_SNAPSHOT_ADDR_LOW_RMSK                                                 0xffffffff
#define HWIO_TQM_R1_CACHE_CTL_DEBUG_CACHE_SNAPSHOT_ADDR_LOW_POR                                                  0x00000000
#define HWIO_TQM_R1_CACHE_CTL_DEBUG_CACHE_SNAPSHOT_ADDR_LOW_POR_RMSK                                             0xffffffff
#define HWIO_TQM_R1_CACHE_CTL_DEBUG_CACHE_SNAPSHOT_ADDR_LOW_ATTR                                                              0x2
#define HWIO_TQM_R1_CACHE_CTL_DEBUG_CACHE_SNAPSHOT_ADDR_LOW_OUT(x, v)            \
                out_dword(HWIO_TQM_R1_CACHE_CTL_DEBUG_CACHE_SNAPSHOT_ADDR_LOW_ADDR(x),v)
#define HWIO_TQM_R1_CACHE_CTL_DEBUG_CACHE_SNAPSHOT_ADDR_LOW_ADDR_31_0_BMSK                                       0xffffffff
#define HWIO_TQM_R1_CACHE_CTL_DEBUG_CACHE_SNAPSHOT_ADDR_LOW_ADDR_31_0_SHFT                                                0

#define HWIO_TQM_R1_CACHE_CTL_DEBUG_CACHE_SNAPSHOT_CAP_CONTROL_ADDR(x)                                           ((x) + 0x205c)
#define HWIO_TQM_R1_CACHE_CTL_DEBUG_CACHE_SNAPSHOT_CAP_CONTROL_PHYS(x)                                           ((x) + 0x205c)
#define HWIO_TQM_R1_CACHE_CTL_DEBUG_CACHE_SNAPSHOT_CAP_CONTROL_OFFS                                              (0x205c)
#define HWIO_TQM_R1_CACHE_CTL_DEBUG_CACHE_SNAPSHOT_CAP_CONTROL_RMSK                                              0xffffffff
#define HWIO_TQM_R1_CACHE_CTL_DEBUG_CACHE_SNAPSHOT_CAP_CONTROL_POR                                               0x00000000
#define HWIO_TQM_R1_CACHE_CTL_DEBUG_CACHE_SNAPSHOT_CAP_CONTROL_POR_RMSK                                          0xffffffff
#define HWIO_TQM_R1_CACHE_CTL_DEBUG_CACHE_SNAPSHOT_CAP_CONTROL_ATTR                                                           0x3
#define HWIO_TQM_R1_CACHE_CTL_DEBUG_CACHE_SNAPSHOT_CAP_CONTROL_IN(x)            \
                in_dword(HWIO_TQM_R1_CACHE_CTL_DEBUG_CACHE_SNAPSHOT_CAP_CONTROL_ADDR(x))
#define HWIO_TQM_R1_CACHE_CTL_DEBUG_CACHE_SNAPSHOT_CAP_CONTROL_INM(x, m)            \
                in_dword_masked(HWIO_TQM_R1_CACHE_CTL_DEBUG_CACHE_SNAPSHOT_CAP_CONTROL_ADDR(x), m)
#define HWIO_TQM_R1_CACHE_CTL_DEBUG_CACHE_SNAPSHOT_CAP_CONTROL_OUT(x, v)            \
                out_dword(HWIO_TQM_R1_CACHE_CTL_DEBUG_CACHE_SNAPSHOT_CAP_CONTROL_ADDR(x),v)
#define HWIO_TQM_R1_CACHE_CTL_DEBUG_CACHE_SNAPSHOT_CAP_CONTROL_OUTM(x,m,v) \
                out_dword_masked_ns(HWIO_TQM_R1_CACHE_CTL_DEBUG_CACHE_SNAPSHOT_CAP_CONTROL_ADDR(x),m,v,HWIO_TQM_R1_CACHE_CTL_DEBUG_CACHE_SNAPSHOT_CAP_CONTROL_IN(x))
#define HWIO_TQM_R1_CACHE_CTL_DEBUG_CACHE_SNAPSHOT_CAP_CONTROL_MISC_CTRL_BMSK                                    0xffff0000
#define HWIO_TQM_R1_CACHE_CTL_DEBUG_CACHE_SNAPSHOT_CAP_CONTROL_MISC_CTRL_SHFT                                            16
#define HWIO_TQM_R1_CACHE_CTL_DEBUG_CACHE_SNAPSHOT_CAP_CONTROL_ARMED_CAPTURE_TRIGGER_BMSK                            0xfff0
#define HWIO_TQM_R1_CACHE_CTL_DEBUG_CACHE_SNAPSHOT_CAP_CONTROL_ARMED_CAPTURE_TRIGGER_SHFT                                 4
#define HWIO_TQM_R1_CACHE_CTL_DEBUG_CACHE_SNAPSHOT_CAP_CONTROL_CAPTURE_ARMED_DONE_BMSK                                  0x8
#define HWIO_TQM_R1_CACHE_CTL_DEBUG_CACHE_SNAPSHOT_CAP_CONTROL_CAPTURE_ARMED_DONE_SHFT                                    3
#define HWIO_TQM_R1_CACHE_CTL_DEBUG_CACHE_SNAPSHOT_CAP_CONTROL_CAPTURE_ARMED_BMSK                                       0x4
#define HWIO_TQM_R1_CACHE_CTL_DEBUG_CACHE_SNAPSHOT_CAP_CONTROL_CAPTURE_ARMED_SHFT                                         2
#define HWIO_TQM_R1_CACHE_CTL_DEBUG_CACHE_SNAPSHOT_CAP_CONTROL_CAPTURE_NOW_DONE_STATUS_BMSK                             0x2
#define HWIO_TQM_R1_CACHE_CTL_DEBUG_CACHE_SNAPSHOT_CAP_CONTROL_CAPTURE_NOW_DONE_STATUS_SHFT                               1
#define HWIO_TQM_R1_CACHE_CTL_DEBUG_CACHE_SNAPSHOT_CAP_CONTROL_CAPTURE_NOW_BMSK                                         0x1
#define HWIO_TQM_R1_CACHE_CTL_DEBUG_CACHE_SNAPSHOT_CAP_CONTROL_CAPTURE_NOW_SHFT                                           0

#define HWIO_TQM_R1_SW_CMD_PROCESSING_ADDR(x)                                                                    ((x) + 0x2060)
#define HWIO_TQM_R1_SW_CMD_PROCESSING_PHYS(x)                                                                    ((x) + 0x2060)
#define HWIO_TQM_R1_SW_CMD_PROCESSING_OFFS                                                                       (0x2060)
#define HWIO_TQM_R1_SW_CMD_PROCESSING_RMSK                                                                          0x1ffff
#define HWIO_TQM_R1_SW_CMD_PROCESSING_POR                                                                        0x00000000
#define HWIO_TQM_R1_SW_CMD_PROCESSING_POR_RMSK                                                                   0xffffffff
#define HWIO_TQM_R1_SW_CMD_PROCESSING_ATTR                                                                                    0x3
#define HWIO_TQM_R1_SW_CMD_PROCESSING_IN(x)            \
                in_dword(HWIO_TQM_R1_SW_CMD_PROCESSING_ADDR(x))
#define HWIO_TQM_R1_SW_CMD_PROCESSING_INM(x, m)            \
                in_dword_masked(HWIO_TQM_R1_SW_CMD_PROCESSING_ADDR(x), m)
#define HWIO_TQM_R1_SW_CMD_PROCESSING_OUT(x, v)            \
                out_dword(HWIO_TQM_R1_SW_CMD_PROCESSING_ADDR(x),v)
#define HWIO_TQM_R1_SW_CMD_PROCESSING_OUTM(x,m,v) \
                out_dword_masked_ns(HWIO_TQM_R1_SW_CMD_PROCESSING_ADDR(x),m,v,HWIO_TQM_R1_SW_CMD_PROCESSING_IN(x))
#define HWIO_TQM_R1_SW_CMD_PROCESSING_DELAY_VALUE_BMSK                                                              0x1ffff
#define HWIO_TQM_R1_SW_CMD_PROCESSING_DELAY_VALUE_SHFT                                                                    0

#define HWIO_TQM_R1_ENT_CMD_PROCESSING_ADDR(x)                                                                   ((x) + 0x2064)
#define HWIO_TQM_R1_ENT_CMD_PROCESSING_PHYS(x)                                                                   ((x) + 0x2064)
#define HWIO_TQM_R1_ENT_CMD_PROCESSING_OFFS                                                                      (0x2064)
#define HWIO_TQM_R1_ENT_CMD_PROCESSING_RMSK                                                                         0x1ffff
#define HWIO_TQM_R1_ENT_CMD_PROCESSING_POR                                                                       0x00000000
#define HWIO_TQM_R1_ENT_CMD_PROCESSING_POR_RMSK                                                                  0xffffffff
#define HWIO_TQM_R1_ENT_CMD_PROCESSING_ATTR                                                                                   0x3
#define HWIO_TQM_R1_ENT_CMD_PROCESSING_IN(x)            \
                in_dword(HWIO_TQM_R1_ENT_CMD_PROCESSING_ADDR(x))
#define HWIO_TQM_R1_ENT_CMD_PROCESSING_INM(x, m)            \
                in_dword_masked(HWIO_TQM_R1_ENT_CMD_PROCESSING_ADDR(x), m)
#define HWIO_TQM_R1_ENT_CMD_PROCESSING_OUT(x, v)            \
                out_dword(HWIO_TQM_R1_ENT_CMD_PROCESSING_ADDR(x),v)
#define HWIO_TQM_R1_ENT_CMD_PROCESSING_OUTM(x,m,v) \
                out_dword_masked_ns(HWIO_TQM_R1_ENT_CMD_PROCESSING_ADDR(x),m,v,HWIO_TQM_R1_ENT_CMD_PROCESSING_IN(x))
#define HWIO_TQM_R1_ENT_CMD_PROCESSING_DELAY_VALUE_BMSK                                                             0x1ffff
#define HWIO_TQM_R1_ENT_CMD_PROCESSING_DELAY_VALUE_SHFT                                                                   0

#define HWIO_TQM_R1_HW_CMD_PROCESSING_ADDR(x)                                                                    ((x) + 0x2068)
#define HWIO_TQM_R1_HW_CMD_PROCESSING_PHYS(x)                                                                    ((x) + 0x2068)
#define HWIO_TQM_R1_HW_CMD_PROCESSING_OFFS                                                                       (0x2068)
#define HWIO_TQM_R1_HW_CMD_PROCESSING_RMSK                                                                          0x1ffff
#define HWIO_TQM_R1_HW_CMD_PROCESSING_POR                                                                        0x00000000
#define HWIO_TQM_R1_HW_CMD_PROCESSING_POR_RMSK                                                                   0xffffffff
#define HWIO_TQM_R1_HW_CMD_PROCESSING_ATTR                                                                                    0x3
#define HWIO_TQM_R1_HW_CMD_PROCESSING_IN(x)            \
                in_dword(HWIO_TQM_R1_HW_CMD_PROCESSING_ADDR(x))
#define HWIO_TQM_R1_HW_CMD_PROCESSING_INM(x, m)            \
                in_dword_masked(HWIO_TQM_R1_HW_CMD_PROCESSING_ADDR(x), m)
#define HWIO_TQM_R1_HW_CMD_PROCESSING_OUT(x, v)            \
                out_dword(HWIO_TQM_R1_HW_CMD_PROCESSING_ADDR(x),v)
#define HWIO_TQM_R1_HW_CMD_PROCESSING_OUTM(x,m,v) \
                out_dword_masked_ns(HWIO_TQM_R1_HW_CMD_PROCESSING_ADDR(x),m,v,HWIO_TQM_R1_HW_CMD_PROCESSING_IN(x))
#define HWIO_TQM_R1_HW_CMD_PROCESSING_DELAY_VALUE_BMSK                                                              0x1ffff
#define HWIO_TQM_R1_HW_CMD_PROCESSING_DELAY_VALUE_SHFT                                                                    0

#define HWIO_TQM_R1_DELAY_PROCESSING_STATUS_ADDR(x)                                                              ((x) + 0x206c)
#define HWIO_TQM_R1_DELAY_PROCESSING_STATUS_PHYS(x)                                                              ((x) + 0x206c)
#define HWIO_TQM_R1_DELAY_PROCESSING_STATUS_OFFS                                                                 (0x206c)
#define HWIO_TQM_R1_DELAY_PROCESSING_STATUS_RMSK                                                                        0xf
#define HWIO_TQM_R1_DELAY_PROCESSING_STATUS_POR                                                                  0x00000000
#define HWIO_TQM_R1_DELAY_PROCESSING_STATUS_POR_RMSK                                                             0xffffffff
#define HWIO_TQM_R1_DELAY_PROCESSING_STATUS_ATTR                                                                              0x1
#define HWIO_TQM_R1_DELAY_PROCESSING_STATUS_IN(x)            \
                in_dword(HWIO_TQM_R1_DELAY_PROCESSING_STATUS_ADDR(x))
#define HWIO_TQM_R1_DELAY_PROCESSING_STATUS_INM(x, m)            \
                in_dword_masked(HWIO_TQM_R1_DELAY_PROCESSING_STATUS_ADDR(x), m)
#define HWIO_TQM_R1_DELAY_PROCESSING_STATUS_ENT_ONE_SHOT_DELAY_DONE_BMSK                                                0x8
#define HWIO_TQM_R1_DELAY_PROCESSING_STATUS_ENT_ONE_SHOT_DELAY_DONE_SHFT                                                  3
#define HWIO_TQM_R1_DELAY_PROCESSING_STATUS_HW_ONE_SHOT_DELAY_DONE_BMSK                                                 0x4
#define HWIO_TQM_R1_DELAY_PROCESSING_STATUS_HW_ONE_SHOT_DELAY_DONE_SHFT                                                   2
#define HWIO_TQM_R1_DELAY_PROCESSING_STATUS_SW_ONE_SHOT_DELAY_DONE_BMSK                                                 0x2
#define HWIO_TQM_R1_DELAY_PROCESSING_STATUS_SW_ONE_SHOT_DELAY_DONE_SHFT                                                   1
#define HWIO_TQM_R1_DELAY_PROCESSING_STATUS_ENT_DELAY_PROCESSING_DONE_BMSK                                              0x1
#define HWIO_TQM_R1_DELAY_PROCESSING_STATUS_ENT_DELAY_PROCESSING_DONE_SHFT                                                0

#define HWIO_TQM_R1_TOTAL_TX_MPDU_COUNT_ADDR(x)                                                                  ((x) + 0x2070)
#define HWIO_TQM_R1_TOTAL_TX_MPDU_COUNT_PHYS(x)                                                                  ((x) + 0x2070)
#define HWIO_TQM_R1_TOTAL_TX_MPDU_COUNT_OFFS                                                                     (0x2070)
#define HWIO_TQM_R1_TOTAL_TX_MPDU_COUNT_RMSK                                                                     0xffffffff
#define HWIO_TQM_R1_TOTAL_TX_MPDU_COUNT_POR                                                                      0x00000000
#define HWIO_TQM_R1_TOTAL_TX_MPDU_COUNT_POR_RMSK                                                                 0xffffffff
#define HWIO_TQM_R1_TOTAL_TX_MPDU_COUNT_ATTR                                                                                  0x1
#define HWIO_TQM_R1_TOTAL_TX_MPDU_COUNT_IN(x)            \
                in_dword(HWIO_TQM_R1_TOTAL_TX_MPDU_COUNT_ADDR(x))
#define HWIO_TQM_R1_TOTAL_TX_MPDU_COUNT_INM(x, m)            \
                in_dword_masked(HWIO_TQM_R1_TOTAL_TX_MPDU_COUNT_ADDR(x), m)
#define HWIO_TQM_R1_TOTAL_TX_MPDU_COUNT_VALUE_BMSK                                                               0xffffffff
#define HWIO_TQM_R1_TOTAL_TX_MPDU_COUNT_VALUE_SHFT                                                                        0

#define HWIO_TQM_R1_TOTAL_ACKED_MPDU_COUNT_ADDR(x)                                                               ((x) + 0x2074)
#define HWIO_TQM_R1_TOTAL_ACKED_MPDU_COUNT_PHYS(x)                                                               ((x) + 0x2074)
#define HWIO_TQM_R1_TOTAL_ACKED_MPDU_COUNT_OFFS                                                                  (0x2074)
#define HWIO_TQM_R1_TOTAL_ACKED_MPDU_COUNT_RMSK                                                                  0xffffffff
#define HWIO_TQM_R1_TOTAL_ACKED_MPDU_COUNT_POR                                                                   0x00000000
#define HWIO_TQM_R1_TOTAL_ACKED_MPDU_COUNT_POR_RMSK                                                              0xffffffff
#define HWIO_TQM_R1_TOTAL_ACKED_MPDU_COUNT_ATTR                                                                               0x1
#define HWIO_TQM_R1_TOTAL_ACKED_MPDU_COUNT_IN(x)            \
                in_dword(HWIO_TQM_R1_TOTAL_ACKED_MPDU_COUNT_ADDR(x))
#define HWIO_TQM_R1_TOTAL_ACKED_MPDU_COUNT_INM(x, m)            \
                in_dword_masked(HWIO_TQM_R1_TOTAL_ACKED_MPDU_COUNT_ADDR(x), m)
#define HWIO_TQM_R1_TOTAL_ACKED_MPDU_COUNT_VALUE_BMSK                                                            0xffffffff
#define HWIO_TQM_R1_TOTAL_ACKED_MPDU_COUNT_VALUE_SHFT                                                                     0

#define HWIO_TQM_R1_LPM_REQ_HANDLER_STATUS_ADDR(x)                                                               ((x) + 0x2078)
#define HWIO_TQM_R1_LPM_REQ_HANDLER_STATUS_PHYS(x)                                                               ((x) + 0x2078)
#define HWIO_TQM_R1_LPM_REQ_HANDLER_STATUS_OFFS                                                                  (0x2078)
#define HWIO_TQM_R1_LPM_REQ_HANDLER_STATUS_RMSK                                                                        0xff
#define HWIO_TQM_R1_LPM_REQ_HANDLER_STATUS_POR                                                                   0x00000000
#define HWIO_TQM_R1_LPM_REQ_HANDLER_STATUS_POR_RMSK                                                              0xffffffff
#define HWIO_TQM_R1_LPM_REQ_HANDLER_STATUS_ATTR                                                                               0x1
#define HWIO_TQM_R1_LPM_REQ_HANDLER_STATUS_IN(x)            \
                in_dword(HWIO_TQM_R1_LPM_REQ_HANDLER_STATUS_ADDR(x))
#define HWIO_TQM_R1_LPM_REQ_HANDLER_STATUS_INM(x, m)            \
                in_dword_masked(HWIO_TQM_R1_LPM_REQ_HANDLER_STATUS_ADDR(x), m)
#define HWIO_TQM_R1_LPM_REQ_HANDLER_STATUS_SLEEP_REQ_ACK_MULTI_SRNG_P_BMSK                                             0x80
#define HWIO_TQM_R1_LPM_REQ_HANDLER_STATUS_SLEEP_REQ_ACK_MULTI_SRNG_P_SHFT                                                7
#define HWIO_TQM_R1_LPM_REQ_HANDLER_STATUS_SLEEP_REQ_ACK_MULTI_SRNG_C_BMSK                                             0x40
#define HWIO_TQM_R1_LPM_REQ_HANDLER_STATUS_SLEEP_REQ_ACK_MULTI_SRNG_C_SHFT                                                6
#define HWIO_TQM_R1_LPM_REQ_HANDLER_STATUS_SLEEP_REQ_ACK_TOP_BMSK                                                      0x20
#define HWIO_TQM_R1_LPM_REQ_HANDLER_STATUS_SLEEP_REQ_ACK_TOP_SHFT                                                         5
#define HWIO_TQM_R1_LPM_REQ_HANDLER_STATUS_SLEEP_REQ_ACK_TO_LPM_BMSK                                                   0x10
#define HWIO_TQM_R1_LPM_REQ_HANDLER_STATUS_SLEEP_REQ_ACK_TO_LPM_SHFT                                                      4
#define HWIO_TQM_R1_LPM_REQ_HANDLER_STATUS_SLEEP_REQ_FROM_LPM_BMSK                                                      0x8
#define HWIO_TQM_R1_LPM_REQ_HANDLER_STATUS_SLEEP_REQ_FROM_LPM_SHFT                                                        3
#define HWIO_TQM_R1_LPM_REQ_HANDLER_STATUS_SM_STATE_BMSK                                                                0x7
#define HWIO_TQM_R1_LPM_REQ_HANDLER_STATUS_SM_STATE_SHFT                                                                  0

#define HWIO_TQM_R1_TESTBUS_CAPTURE_n_ADDR(base,n)                                                               ((base) + 0X2114 + (0x4*(n)))
#define HWIO_TQM_R1_TESTBUS_CAPTURE_n_PHYS(base,n)                                                               ((base) + 0X2114 + (0x4*(n)))
#define HWIO_TQM_R1_TESTBUS_CAPTURE_n_OFFS(n)                                                                    (0X2114 + (0x4*(n)))
#define HWIO_TQM_R1_TESTBUS_CAPTURE_n_RMSK                                                                       0xffffffff
#define HWIO_TQM_R1_TESTBUS_CAPTURE_n_MAXn                                                                              127
#define HWIO_TQM_R1_TESTBUS_CAPTURE_n_POR                                                                        0x00000000
#define HWIO_TQM_R1_TESTBUS_CAPTURE_n_POR_RMSK                                                                   0xffffffff
#define HWIO_TQM_R1_TESTBUS_CAPTURE_n_ATTR                                                                                    0x1
#define HWIO_TQM_R1_TESTBUS_CAPTURE_n_INI(base,n)                \
                in_dword_masked(HWIO_TQM_R1_TESTBUS_CAPTURE_n_ADDR(base,n), HWIO_TQM_R1_TESTBUS_CAPTURE_n_RMSK)
#define HWIO_TQM_R1_TESTBUS_CAPTURE_n_INMI(base,n,mask)        \
                in_dword_masked(HWIO_TQM_R1_TESTBUS_CAPTURE_n_ADDR(base,n), mask)
#define HWIO_TQM_R1_TESTBUS_CAPTURE_n_DATA_BMSK                                                                  0xffffffff
#define HWIO_TQM_R1_TESTBUS_CAPTURE_n_DATA_SHFT                                                                           0

#define HWIO_UMCMN_R0_CLK_GATE_DISABLE_LPM_HANDLER_BMSK                                                        0x80
#define HWIO_UMCMN_R0_CLK_GATE_DISABLE_LPM_HANDLER_SHFT                                                           7
#define HWIO_UMCMN_R0_ISR_S6_REO_MULTI_PRODUCER_SRNG_WDG_ERR_BMSK                                          0x800000
#define HWIO_UMCMN_R0_ISR_S6_REO_MULTI_PRODUCER_SRNG_WDG_ERR_SHFT                                                23
#define HWIO_UMCMN_R0_ISR_S6_REO_MULTI_CONSUMER_SRNG_WDG_ERR_BMSK                                          0x400000
#define HWIO_UMCMN_R0_ISR_S6_REO_MULTI_CONSUMER_SRNG_WDG_ERR_SHFT                                                22
#define HWIO_UMCMN_R0_ISR_S11_TCL_MULTI_PRODUCER_SRNG_WDG_ERR_BMSK                                        0x8000000
#define HWIO_UMCMN_R0_ISR_S11_TCL_MULTI_PRODUCER_SRNG_WDG_ERR_SHFT                                               27
#define HWIO_UMCMN_R0_ISR_S11_TCL_MULTI_CONSUMER_SRNG_WDG_ERR_BMSK                                        0x4000000
#define HWIO_UMCMN_R0_ISR_S11_TCL_MULTI_CONSUMER_SRNG_WDG_ERR_SHFT                                               26
#define HWIO_UMCMN_R0_ISR_S13_TQM_MULTI_PRODUCER_SRNG_WDG_ERR_BMSK                                          0x80000
#define HWIO_UMCMN_R0_ISR_S13_TQM_MULTI_PRODUCER_SRNG_WDG_ERR_SHFT                                               19
#define HWIO_UMCMN_R0_ISR_S13_TQM_MULTI_CONSUMER_SRNG_WDG_ERR_BMSK                                          0x40000
#define HWIO_UMCMN_R0_ISR_S13_TQM_MULTI_CONSUMER_SRNG_WDG_ERR_SHFT                                               18
#define HWIO_UMCMN_R0_ISR_S16_MXI_RD_ZERO_ADDR_ERR_BMSK                                                       0x100
#define HWIO_UMCMN_R0_ISR_S16_MXI_RD_ZERO_ADDR_ERR_SHFT                                                           8
#define HWIO_UMCMN_R0_ISR_S16_MXI_RD_ZERO_SIZE_ERR_BMSK                                                        0x80
#define HWIO_UMCMN_R0_ISR_S16_MXI_RD_ZERO_SIZE_ERR_SHFT                                                           7
#define HWIO_UMCMN_R0_ISR_S16_MXI_WR_ZERO_ADDR_ERR_BMSK                                                        0x40
#define HWIO_UMCMN_R0_ISR_S16_MXI_WR_ZERO_ADDR_ERR_SHFT                                                           6
#define HWIO_UMCMN_R0_ISR_S16_MXI_WR_ZERO_SIZE_ERR_BMSK                                                        0x20
#define HWIO_UMCMN_R0_ISR_S16_MXI_WR_ZERO_SIZE_ERR_SHFT                                                           5
#define HWIO_UMCMN_R0_ISR_S17_WBM_MULTI_PRODUCER_SRNG_WDG_ERR_BMSK                                          0x20000
#define HWIO_UMCMN_R0_ISR_S17_WBM_MULTI_PRODUCER_SRNG_WDG_ERR_SHFT                                               17
#define HWIO_UMCMN_R0_ISR_S17_WBM_MULTI_CONSUMER_SRNG_WDG_ERR_BMSK                                          0x10000
#define HWIO_UMCMN_R0_ISR_S17_WBM_MULTI_CONSUMER_SRNG_WDG_ERR_SHFT                                               16
#define HWIO_UMCMN_R0_IMR_S6_REO_MULTI_PRODUCER_SRNG_WDG_ERR_BMSK                                          0x800000
#define HWIO_UMCMN_R0_IMR_S6_REO_MULTI_PRODUCER_SRNG_WDG_ERR_SHFT                                                23
#define HWIO_UMCMN_R0_IMR_S6_REO_MULTI_CONSUMER_SRNG_WDG_ERR_BMSK                                          0x400000
#define HWIO_UMCMN_R0_IMR_S6_REO_MULTI_CONSUMER_SRNG_WDG_ERR_SHFT                                                22
#define HWIO_UMCMN_R0_IMR_S11_TCL_MULTI_PRODUCER_SRNG_WDG_ERR_BMSK                                        0x8000000
#define HWIO_UMCMN_R0_IMR_S11_TCL_MULTI_PRODUCER_SRNG_WDG_ERR_SHFT                                               27
#define HWIO_UMCMN_R0_IMR_S11_TCL_MULTI_CONSUMER_SRNG_WDG_ERR_BMSK                                        0x4000000
#define HWIO_UMCMN_R0_IMR_S11_TCL_MULTI_CONSUMER_SRNG_WDG_ERR_SHFT                                               26
#define HWIO_UMCMN_R0_IMR_S13_TQM_MULTI_PRODUCER_SRNG_WDG_ERR_BMSK                                          0x80000
#define HWIO_UMCMN_R0_IMR_S13_TQM_MULTI_PRODUCER_SRNG_WDG_ERR_SHFT                                               19
#define HWIO_UMCMN_R0_IMR_S13_TQM_MULTI_CONSUMER_SRNG_WDG_ERR_BMSK                                          0x40000
#define HWIO_UMCMN_R0_IMR_S13_TQM_MULTI_CONSUMER_SRNG_WDG_ERR_SHFT                                               18
#define HWIO_UMCMN_R0_IMR_S16_MXI_RD_ZERO_ADDR_ERR_BMSK                                                       0x100
#define HWIO_UMCMN_R0_IMR_S16_MXI_RD_ZERO_ADDR_ERR_SHFT                                                           8
#define HWIO_UMCMN_R0_IMR_S16_MXI_RD_ZERO_SIZE_ERR_BMSK                                                        0x80
#define HWIO_UMCMN_R0_IMR_S16_MXI_RD_ZERO_SIZE_ERR_SHFT                                                           7
#define HWIO_UMCMN_R0_IMR_S16_MXI_WR_ZERO_ADDR_ERR_BMSK                                                        0x40
#define HWIO_UMCMN_R0_IMR_S16_MXI_WR_ZERO_ADDR_ERR_SHFT                                                           6
#define HWIO_UMCMN_R0_IMR_S16_MXI_WR_ZERO_SIZE_ERR_BMSK                                                        0x20
#define HWIO_UMCMN_R0_IMR_S16_MXI_WR_ZERO_SIZE_ERR_SHFT                                                           5
#define HWIO_UMCMN_R0_IMR_S17_WBM_MULTI_PRODUCER_SRNG_WDG_ERR_BMSK                                          0x20000
#define HWIO_UMCMN_R0_IMR_S17_WBM_MULTI_PRODUCER_SRNG_WDG_ERR_SHFT                                               17
#define HWIO_UMCMN_R0_IMR_S17_WBM_MULTI_CONSUMER_SRNG_WDG_ERR_BMSK                                          0x10000
#define HWIO_UMCMN_R0_IMR_S17_WBM_MULTI_CONSUMER_SRNG_WDG_ERR_SHFT                                               16
#define HWIO_UMCMN_R0_ASYNC_ISYNC_FIFO_SOFTRESET_AND_CLK_UMAC_BMSK                                             0x40
#define HWIO_UMCMN_R0_ASYNC_ISYNC_FIFO_SOFTRESET_AND_CLK_UMAC_SHFT                                                6
#define HWIO_UMCMN_R0_TX_TIMESTAMP_RESOLUTION_SELECT_ADDR(x)                                             ((x) + 0x168)
#define HWIO_UMCMN_R0_TX_TIMESTAMP_RESOLUTION_SELECT_PHYS(x)                                             ((x) + 0x168)
#define HWIO_UMCMN_R0_TX_TIMESTAMP_RESOLUTION_SELECT_OFFS                                                (0x168)
#define HWIO_UMCMN_R0_TX_TIMESTAMP_RESOLUTION_SELECT_RMSK                                                       0xf
#define HWIO_UMCMN_R0_TX_TIMESTAMP_RESOLUTION_SELECT_POR                                                 0x0000000a
#define HWIO_UMCMN_R0_TX_TIMESTAMP_RESOLUTION_SELECT_POR_RMSK                                            0xffffffff
#define HWIO_UMCMN_R0_TX_TIMESTAMP_RESOLUTION_SELECT_ATTR                                                             0x3
#define HWIO_UMCMN_R0_TX_TIMESTAMP_RESOLUTION_SELECT_IN(x)            \
                in_dword(HWIO_UMCMN_R0_TX_TIMESTAMP_RESOLUTION_SELECT_ADDR(x))
#define HWIO_UMCMN_R0_TX_TIMESTAMP_RESOLUTION_SELECT_INM(x, m)            \
                in_dword_masked(HWIO_UMCMN_R0_TX_TIMESTAMP_RESOLUTION_SELECT_ADDR(x), m)
#define HWIO_UMCMN_R0_TX_TIMESTAMP_RESOLUTION_SELECT_OUT(x, v)            \
                out_dword(HWIO_UMCMN_R0_TX_TIMESTAMP_RESOLUTION_SELECT_ADDR(x),v)
#define HWIO_UMCMN_R0_TX_TIMESTAMP_RESOLUTION_SELECT_OUTM(x,m,v) \
                out_dword_masked_ns(HWIO_UMCMN_R0_TX_TIMESTAMP_RESOLUTION_SELECT_ADDR(x),m,v,HWIO_UMCMN_R0_TX_TIMESTAMP_RESOLUTION_SELECT_IN(x))
#define HWIO_UMCMN_R0_TX_TIMESTAMP_RESOLUTION_SELECT_VALUE_BMSK                                                 0xf
#define HWIO_UMCMN_R0_TX_TIMESTAMP_RESOLUTION_SELECT_VALUE_SHFT                                                   0

#define HWIO_UMCMN_R0_LPM_FW_CTRL_ADDR(x)                                                                ((x) + 0x16c)
#define HWIO_UMCMN_R0_LPM_FW_CTRL_PHYS(x)                                                                ((x) + 0x16c)
#define HWIO_UMCMN_R0_LPM_FW_CTRL_OFFS                                                                   (0x16c)
#define HWIO_UMCMN_R0_LPM_FW_CTRL_RMSK                                                                         0x1f
#define HWIO_UMCMN_R0_LPM_FW_CTRL_POR                                                                    0x00000000
#define HWIO_UMCMN_R0_LPM_FW_CTRL_POR_RMSK                                                               0xffffffff
#define HWIO_UMCMN_R0_LPM_FW_CTRL_ATTR                                                                                0x3
#define HWIO_UMCMN_R0_LPM_FW_CTRL_IN(x)            \
                in_dword(HWIO_UMCMN_R0_LPM_FW_CTRL_ADDR(x))
#define HWIO_UMCMN_R0_LPM_FW_CTRL_INM(x, m)            \
                in_dword_masked(HWIO_UMCMN_R0_LPM_FW_CTRL_ADDR(x), m)
#define HWIO_UMCMN_R0_LPM_FW_CTRL_OUT(x, v)            \
                out_dword(HWIO_UMCMN_R0_LPM_FW_CTRL_ADDR(x),v)
#define HWIO_UMCMN_R0_LPM_FW_CTRL_OUTM(x,m,v) \
                out_dword_masked_ns(HWIO_UMCMN_R0_LPM_FW_CTRL_ADDR(x),m,v,HWIO_UMCMN_R0_LPM_FW_CTRL_IN(x))
#define HWIO_UMCMN_R0_LPM_FW_CTRL_SLEEP_REQ_ACK_OVR_REO_BMSK                                                   0x10
#define HWIO_UMCMN_R0_LPM_FW_CTRL_SLEEP_REQ_ACK_OVR_REO_SHFT                                                      4
#define HWIO_UMCMN_R0_LPM_FW_CTRL_SLEEP_REQ_ACK_OVR_WBM_BMSK                                                    0x8
#define HWIO_UMCMN_R0_LPM_FW_CTRL_SLEEP_REQ_ACK_OVR_WBM_SHFT                                                      3
#define HWIO_UMCMN_R0_LPM_FW_CTRL_SLEEP_REQ_ACK_OVR_TQM_BMSK                                                    0x4
#define HWIO_UMCMN_R0_LPM_FW_CTRL_SLEEP_REQ_ACK_OVR_TQM_SHFT                                                      2
#define HWIO_UMCMN_R0_LPM_FW_CTRL_SLEEP_REQ_ACK_OVR_TCL_BMSK                                                    0x2
#define HWIO_UMCMN_R0_LPM_FW_CTRL_SLEEP_REQ_ACK_OVR_TCL_SHFT                                                      1
#define HWIO_UMCMN_R0_LPM_FW_CTRL_SLEEP_REQ_BMSK                                                                0x1
#define HWIO_UMCMN_R0_LPM_FW_CTRL_SLEEP_REQ_SHFT                                                                  0

#define HWIO_UMCMN_R0_LINK_ID_ADDR(x)                                                                    ((x) + 0x170)
#define HWIO_UMCMN_R0_LINK_ID_PHYS(x)                                                                    ((x) + 0x170)
#define HWIO_UMCMN_R0_LINK_ID_OFFS                                                                       (0x170)
#define HWIO_UMCMN_R0_LINK_ID_RMSK                                                                           0xffff
#define HWIO_UMCMN_R0_LINK_ID_POR                                                                        0x000052c8
#define HWIO_UMCMN_R0_LINK_ID_POR_RMSK                                                                   0xffffffff
#define HWIO_UMCMN_R0_LINK_ID_ATTR                                                                                    0x3
#define HWIO_UMCMN_R0_LINK_ID_IN(x)            \
                in_dword(HWIO_UMCMN_R0_LINK_ID_ADDR(x))
#define HWIO_UMCMN_R0_LINK_ID_INM(x, m)            \
                in_dword_masked(HWIO_UMCMN_R0_LINK_ID_ADDR(x), m)
#define HWIO_UMCMN_R0_LINK_ID_OUT(x, v)            \
                out_dword(HWIO_UMCMN_R0_LINK_ID_ADDR(x),v)
#define HWIO_UMCMN_R0_LINK_ID_OUTM(x,m,v) \
                out_dword_masked_ns(HWIO_UMCMN_R0_LINK_ID_ADDR(x),m,v,HWIO_UMCMN_R0_LINK_ID_IN(x))
#define HWIO_UMCMN_R0_LINK_ID_WLAN1_LINK_EN_BMSK                                                               0x80
#define HWIO_UMCMN_R0_LINK_ID_WLAN1_LINK_EN_SHFT                                                                  7
#define HWIO_UMCMN_R0_LINK_ID_WLAN0_LINK_EN_BMSK                                                               0x40
#define HWIO_UMCMN_R0_LINK_ID_WLAN0_LINK_EN_SHFT                                                                  6
#define HWIO_UMCMN_R0_LINK_ID_WLAN1_LINK_ID_BMSK                                                               0x38
#define HWIO_UMCMN_R0_LINK_ID_WLAN1_LINK_ID_SHFT                                                                  3
#define HWIO_UMCMN_R0_LINK_ID_WLAN0_LINK_ID_BMSK                                                                0x7
#define HWIO_UMCMN_R0_LINK_ID_WLAN0_LINK_ID_SHFT                                                                  0

#define HWIO_UMCMN_R0_TRC_CTRL_2_EVENT_BLK_MASK_UMXI_BMSK                                                    0x4000
#define HWIO_UMCMN_R0_TRC_CTRL_2_EVENT_BLK_MASK_UMXI_SHFT                                                        14
#define HWIO_UMCMN_R0_TRC_CTRL_2_EVENT_BLK_MASK_RESERVED_BMSK                                                0x2000
#define HWIO_UMCMN_R0_TRC_CTRL_2_EVENT_BLK_MASK_RESERVED_SHFT                                                    13
#define HWIO_UMCMN_R0_TRC_CTRL_2_EVENT_BLK_MASK_WBM1_BMSK                                                    0x1000
#define HWIO_UMCMN_R0_TRC_CTRL_2_EVENT_BLK_MASK_WBM1_SHFT                                                        12
#define HWIO_UMCMN_R0_TRC_CTRL_2_EVENT_BLK_MASK_TQM1_BMSK                                                     0x800
#define HWIO_UMCMN_R0_TRC_CTRL_2_EVENT_BLK_MASK_TQM1_SHFT                                                        11
#define HWIO_UMCMN_R0_TRC_CTRL_2_EVENT_BLK_MASK_REO1_BMSK                                                     0x400
#define HWIO_UMCMN_R0_TRC_CTRL_2_EVENT_BLK_MASK_REO1_SHFT                                                        10
#define HWIO_UMCMN_R0_TRC_CTRL_2_EVENT_BLK_MASK_TCL1_BMSK                                                     0x200
#define HWIO_UMCMN_R0_TRC_CTRL_2_EVENT_BLK_MASK_TCL1_SHFT                                                         9
#define HWIO_UMCMN_R0_TRC_CTRL_2_EVENT_BLK_MASK_CXC1_BMSK                                                     0x100
#define HWIO_UMCMN_R0_TRC_CTRL_2_EVENT_BLK_MASK_CXC1_SHFT                                                         8
#define HWIO_UMCMN_R0_TRC_CTRL_2_EVENT_BLK_MASK_REO_BMSK                                                       0x80
#define HWIO_UMCMN_R0_TRC_CTRL_2_EVENT_BLK_MASK_REO_SHFT                                                          7
#define HWIO_UMCMN_R0_TRC_CTRL_2_EVENT_BLK_MASK_TCL_BMSK                                                       0x40
#define HWIO_UMCMN_R0_TRC_CTRL_2_EVENT_BLK_MASK_TCL_SHFT                                                          6
#define HWIO_UMCMN_R0_TRC_CTRL_2_EVENT_BLK_MASK_WBM_BMSK                                                       0x20
#define HWIO_UMCMN_R0_TRC_CTRL_2_EVENT_BLK_MASK_WBM_SHFT                                                          5
#define HWIO_UMCMN_R0_TRC_CTRL_2_EVENT_BLK_MASK_TQM_BMSK                                                       0x10
#define HWIO_UMCMN_R0_TRC_CTRL_2_EVENT_BLK_MASK_TQM_SHFT                                                          4
#define HWIO_UMCMN_R0_TRC_CTRL_2_EVENT_BLK_MASK_SW_BMSK                                                         0x8
#define HWIO_UMCMN_R0_TRC_CTRL_2_EVENT_BLK_MASK_SW_SHFT                                                           3
#define HWIO_UMCMN_R0_TRC_CTRL_2_EVENT_BLK_MASK_CXC_BMSK                                                        0x4
#define HWIO_UMCMN_R0_TRC_CTRL_2_EVENT_BLK_MASK_CXC_SHFT                                                          2
#define HWIO_UMCMN_R0_TRC_CTRL_2_EVENT_BLK_MASK_CE_BMSK                                                         0x2
#define HWIO_UMCMN_R0_TRC_CTRL_2_EVENT_BLK_MASK_CE_SHFT                                                           1
#define HWIO_UMCMN_R0_TRC_CTRL_2_EVENT_BLK_MASK_ECD_BMSK                                                        0x1
#define HWIO_UMCMN_R0_TRC_CTRL_2_EVENT_BLK_MASK_ECD_SHFT                                                          0

#define HWIO_UMCMN_R0_EVENT_MODE_SELECT_ADDR(x)                                                          ((x) + 0x184)
#define HWIO_UMCMN_R0_EVENT_MODE_SELECT_PHYS(x)                                                          ((x) + 0x184)
#define HWIO_UMCMN_R0_EVENT_MODE_SELECT_OFFS                                                             (0x184)
#define HWIO_UMCMN_R0_EVENT_MODE_SELECT_RMSK                                                                    0x1
#define HWIO_UMCMN_R0_EVENT_MODE_SELECT_POR                                                              0x00000000
#define HWIO_UMCMN_R0_EVENT_MODE_SELECT_POR_RMSK                                                         0xffffffff
#define HWIO_UMCMN_R0_EVENT_MODE_SELECT_ATTR                                                                          0x3
#define HWIO_UMCMN_R0_EVENT_MODE_SELECT_IN(x)            \
                in_dword(HWIO_UMCMN_R0_EVENT_MODE_SELECT_ADDR(x))
#define HWIO_UMCMN_R0_EVENT_MODE_SELECT_INM(x, m)            \
                in_dword_masked(HWIO_UMCMN_R0_EVENT_MODE_SELECT_ADDR(x), m)
#define HWIO_UMCMN_R0_EVENT_MODE_SELECT_OUT(x, v)            \
                out_dword(HWIO_UMCMN_R0_EVENT_MODE_SELECT_ADDR(x),v)
#define HWIO_UMCMN_R0_EVENT_MODE_SELECT_OUTM(x,m,v) \
                out_dword_masked_ns(HWIO_UMCMN_R0_EVENT_MODE_SELECT_ADDR(x),m,v,HWIO_UMCMN_R0_EVENT_MODE_SELECT_IN(x))
#define HWIO_UMCMN_R0_EVENT_MODE_SELECT_VAL_BMSK                                                                0x1
#define HWIO_UMCMN_R0_EVENT_MODE_SELECT_VAL_SHFT                                                                  0

#define HWIO_UMCMN_R0_SIMULATED_WCSS_EVENT_BPGEN_CFG_ADDR(x)                                             ((x) + 0x188)
#define HWIO_UMCMN_R0_SIMULATED_WCSS_EVENT_BPGEN_CFG_PHYS(x)                                             ((x) + 0x188)
#define HWIO_UMCMN_R0_SIMULATED_WCSS_EVENT_BPGEN_CFG_OFFS                                                (0x188)
#define HWIO_UMCMN_R0_SIMULATED_WCSS_EVENT_BPGEN_CFG_RMSK                                                   0x1ffff
#define HWIO_UMCMN_R0_SIMULATED_WCSS_EVENT_BPGEN_CFG_POR                                                 0x00000000
#define HWIO_UMCMN_R0_SIMULATED_WCSS_EVENT_BPGEN_CFG_POR_RMSK                                            0xffffffff
#define HWIO_UMCMN_R0_SIMULATED_WCSS_EVENT_BPGEN_CFG_ATTR                                                             0x3
#define HWIO_UMCMN_R0_SIMULATED_WCSS_EVENT_BPGEN_CFG_IN(x)            \
                in_dword(HWIO_UMCMN_R0_SIMULATED_WCSS_EVENT_BPGEN_CFG_ADDR(x))
#define HWIO_UMCMN_R0_SIMULATED_WCSS_EVENT_BPGEN_CFG_INM(x, m)            \
                in_dword_masked(HWIO_UMCMN_R0_SIMULATED_WCSS_EVENT_BPGEN_CFG_ADDR(x), m)
#define HWIO_UMCMN_R0_SIMULATED_WCSS_EVENT_BPGEN_CFG_OUT(x, v)            \
                out_dword(HWIO_UMCMN_R0_SIMULATED_WCSS_EVENT_BPGEN_CFG_ADDR(x),v)
#define HWIO_UMCMN_R0_SIMULATED_WCSS_EVENT_BPGEN_CFG_OUTM(x,m,v) \
                out_dword_masked_ns(HWIO_UMCMN_R0_SIMULATED_WCSS_EVENT_BPGEN_CFG_ADDR(x),m,v,HWIO_UMCMN_R0_SIMULATED_WCSS_EVENT_BPGEN_CFG_IN(x))
#define HWIO_UMCMN_R0_SIMULATED_WCSS_EVENT_BPGEN_CFG_BACK_TO_BACK_BACKPRESSURE_COUNT_BMSK                   0x1fe00
#define HWIO_UMCMN_R0_SIMULATED_WCSS_EVENT_BPGEN_CFG_BACK_TO_BACK_BACKPRESSURE_COUNT_SHFT                         9
#define HWIO_UMCMN_R0_SIMULATED_WCSS_EVENT_BPGEN_CFG_BACK_TO_BACK_ALLOWED_VALID_COUNT_BMSK                    0x1fe
#define HWIO_UMCMN_R0_SIMULATED_WCSS_EVENT_BPGEN_CFG_BACK_TO_BACK_ALLOWED_VALID_COUNT_SHFT                        1
#define HWIO_UMCMN_R0_SIMULATED_WCSS_EVENT_BPGEN_CFG_SIMULATED_BACKPRESSURE_ENABLE_BMSK                         0x1
#define HWIO_UMCMN_R0_SIMULATED_WCSS_EVENT_BPGEN_CFG_SIMULATED_BACKPRESSURE_ENABLE_SHFT                           0

#define HWIO_UMCMN_R1_LPM_REQ_HANDLER_STATUS_ADDR(x)                                                     ((x) + 0x2010)
#define HWIO_UMCMN_R1_LPM_REQ_HANDLER_STATUS_PHYS(x)                                                     ((x) + 0x2010)
#define HWIO_UMCMN_R1_LPM_REQ_HANDLER_STATUS_OFFS                                                        (0x2010)
#define HWIO_UMCMN_R1_LPM_REQ_HANDLER_STATUS_RMSK                                                             0x1ff
#define HWIO_UMCMN_R1_LPM_REQ_HANDLER_STATUS_POR                                                         0x00000000
#define HWIO_UMCMN_R1_LPM_REQ_HANDLER_STATUS_POR_RMSK                                                    0xffffffff
#define HWIO_UMCMN_R1_LPM_REQ_HANDLER_STATUS_ATTR                                                                     0x1
#define HWIO_UMCMN_R1_LPM_REQ_HANDLER_STATUS_IN(x)            \
                in_dword(HWIO_UMCMN_R1_LPM_REQ_HANDLER_STATUS_ADDR(x))
#define HWIO_UMCMN_R1_LPM_REQ_HANDLER_STATUS_INM(x, m)            \
                in_dword_masked(HWIO_UMCMN_R1_LPM_REQ_HANDLER_STATUS_ADDR(x), m)
#define HWIO_UMCMN_R1_LPM_REQ_HANDLER_STATUS_SLEEP_REQ_ACK_REO_BMSK                                           0x100
#define HWIO_UMCMN_R1_LPM_REQ_HANDLER_STATUS_SLEEP_REQ_ACK_REO_SHFT                                               8
#define HWIO_UMCMN_R1_LPM_REQ_HANDLER_STATUS_SLEEP_REQ_ACK_WBM_BMSK                                            0x80
#define HWIO_UMCMN_R1_LPM_REQ_HANDLER_STATUS_SLEEP_REQ_ACK_WBM_SHFT                                               7
#define HWIO_UMCMN_R1_LPM_REQ_HANDLER_STATUS_SLEEP_REQ_ACK_TQM_BMSK                                            0x40
#define HWIO_UMCMN_R1_LPM_REQ_HANDLER_STATUS_SLEEP_REQ_ACK_TQM_SHFT                                               6
#define HWIO_UMCMN_R1_LPM_REQ_HANDLER_STATUS_SLEEP_REQ_ACK_TCL_BMSK                                            0x20
#define HWIO_UMCMN_R1_LPM_REQ_HANDLER_STATUS_SLEEP_REQ_ACK_TCL_SHFT                                               5
#define HWIO_UMCMN_R1_LPM_REQ_HANDLER_STATUS_SLEEP_REQ_ACK_TO_LPM_BMSK                                         0x10
#define HWIO_UMCMN_R1_LPM_REQ_HANDLER_STATUS_SLEEP_REQ_ACK_TO_LPM_SHFT                                            4
#define HWIO_UMCMN_R1_LPM_REQ_HANDLER_STATUS_SLEEP_REQ_FROM_LPM_BMSK                                            0x8
#define HWIO_UMCMN_R1_LPM_REQ_HANDLER_STATUS_SLEEP_REQ_FROM_LPM_SHFT                                              3
#define HWIO_UMCMN_R1_LPM_REQ_HANDLER_STATUS_SM_STATE_BMSK                                                      0x7
#define HWIO_UMCMN_R1_LPM_REQ_HANDLER_STATUS_SM_STATE_SHFT                                                        0

#define HWIO_UMCMN_R1_RETENTION_SPARE_REGISTER_n_ADDR(base,n)                                            ((base) + 0X2014 + (0x4*(n)))
#define HWIO_UMCMN_R1_RETENTION_SPARE_REGISTER_n_PHYS(base,n)                                            ((base) + 0X2014 + (0x4*(n)))
#define HWIO_UMCMN_R1_RETENTION_SPARE_REGISTER_n_OFFS(n)                                                 (0X2014 + (0x4*(n)))
#define HWIO_UMCMN_R1_RETENTION_SPARE_REGISTER_n_RMSK                                                    0xffffffff
#define HWIO_UMCMN_R1_RETENTION_SPARE_REGISTER_n_MAXn                                                             7
#define HWIO_UMCMN_R1_RETENTION_SPARE_REGISTER_n_POR                                                     0x00000000
#define HWIO_UMCMN_R1_RETENTION_SPARE_REGISTER_n_POR_RMSK                                                0xffffffff
#define HWIO_UMCMN_R1_RETENTION_SPARE_REGISTER_n_ATTR                                                                 0x3
#define HWIO_UMCMN_R1_RETENTION_SPARE_REGISTER_n_INI(base,n)                \
                in_dword_masked(HWIO_UMCMN_R1_RETENTION_SPARE_REGISTER_n_ADDR(base,n), HWIO_UMCMN_R1_RETENTION_SPARE_REGISTER_n_RMSK)
#define HWIO_UMCMN_R1_RETENTION_SPARE_REGISTER_n_INMI(base,n,mask)        \
                in_dword_masked(HWIO_UMCMN_R1_RETENTION_SPARE_REGISTER_n_ADDR(base,n), mask)
#define HWIO_UMCMN_R1_RETENTION_SPARE_REGISTER_n_OUTI(base,n,val)        \
                out_dword(HWIO_UMCMN_R1_RETENTION_SPARE_REGISTER_n_ADDR(base,n),val)
#define HWIO_UMCMN_R1_RETENTION_SPARE_REGISTER_n_OUTMI(base,n,mask,val) \
                out_dword_masked_ns(HWIO_UMCMN_R1_RETENTION_SPARE_REGISTER_n_ADDR(base,n),mask,val,HWIO_UMCMN_R1_RETENTION_SPARE_REGISTER_n_INI(base,n))
#define HWIO_UMCMN_R1_RETENTION_SPARE_REGISTER_n_VAL_BMSK                                                0xffffffff
#define HWIO_UMCMN_R1_RETENTION_SPARE_REGISTER_n_VAL_SHFT                                                         0

#define MAC_TCL_REG_REG_BASE                                                                                (UMAC_BASE      + 0x00044000)
#define HWIO_TCL_R0_CONS_RING_CMN_CTRL_REG_ADDR(x)                                                          ((x) + 0x20)
#define HWIO_TCL_R0_CONS_RING_CMN_CTRL_REG_DSCP_TID_MAP_PROGRAM_EN_BMSK                                       0x800000
#define HWIO_TCL_R0_CONS_RING_CMN_CTRL_REG_DSCP_TID_MAP_PROGRAM_EN_SHFT                                             23
#define HWIO_TCL_R0_CMN_CONFIG_VDEV_ID_MISMATCH_DROP_REASON_EN_BMSK                                          0x8000000
#define HWIO_TCL_R0_CMN_CONFIG_VDEV_ID_MISMATCH_DROP_REASON_EN_SHFT                                                 27
#define HWIO_TCL_R0_SW_CONFIG_BANK_n_ADDR(base,n)                                                           ((base) + 0X8C + (0x4*(n)))
#define HWIO_TCL_R0_SW_CONFIG_BANK_n_DSCP_TID_TABLE_NUM_SHFT                                                        17
#define HWIO_TCL_R0_SW_CONFIG_BANK_n_PMAC_ID_SHFT                                                                   15
#define HWIO_TCL_R0_SW_CONFIG_BANK_n_VDEV_ID_CHECK_EN_SHFT                                                          14
#define HWIO_TCL_R0_SW_CONFIG_BANK_n_MESH_ENABLE_SHFT                                                               12
#define HWIO_TCL_R0_SW_CONFIG_BANK_n_ADDRY_EN_SHFT                                                                  11
#define HWIO_TCL_R0_SW_CONFIG_BANK_n_ADDRX_EN_SHFT                                                                  10
#define HWIO_TCL_R0_SW_CONFIG_BANK_n_INDEX_LOOKUP_ENABLE_SHFT                                                        9
#define HWIO_TCL_R0_SW_CONFIG_BANK_n_LINK_META_SWAP_SHFT                                                             8
#define HWIO_TCL_R0_SW_CONFIG_BANK_n_SRC_BUFFER_SWAP_SHFT                                                            7
#define HWIO_TCL_R0_SW_CONFIG_BANK_n_ENCRYPT_TYPE_SHFT                                                               3
#define HWIO_TCL_R0_SW_CONFIG_BANK_n_ENCAP_TYPE_SHFT                                                                 1
#define HWIO_TCL_R0_SW_CONFIG_BANK_n_EPD_SHFT                                                                        0

#define HWIO_TCL_R0_DSCP_TID_MAP_n_ADDR(base,n)                                                             ((base) + 0X240 + (0x4*(n)))
#define HWIO_TCL_R0_DSCP_TID_MAP_n_RMSK                                                                     0xffffffff
#define HWIO_TCL_R0_PCP_TID_MAP_ADDR(x)                                                                     ((x) + 0x6c0)
#define HWIO_TCL_R0_PCP_TID_MAP_RMSK                                                                          0xffffff
#define HWIO_TCL_R0_PCP_TID_MAP_PCP_7_SHFT                                                                          21
#define HWIO_TCL_R0_PCP_TID_MAP_PCP_6_SHFT                                                                          18
#define HWIO_TCL_R0_PCP_TID_MAP_PCP_5_SHFT                                                                          15
#define HWIO_TCL_R0_PCP_TID_MAP_PCP_4_SHFT                                                                          12
#define HWIO_TCL_R0_PCP_TID_MAP_PCP_3_SHFT                                                                           9
#define HWIO_TCL_R0_PCP_TID_MAP_PCP_2_SHFT                                                                           6
#define HWIO_TCL_R0_PCP_TID_MAP_PCP_1_SHFT                                                                           3
#define HWIO_TCL_R0_TID_MAP_PRTY_ADDR(x)                                                                    ((x) + 0x6e8)
#define HWIO_TCL_R0_TID_MAP_PRTY_RMSK                                                                             0xef
#define HWIO_TCL_R0_LCE_RULE_n_MATCH_MCAST_AND_L3_TYPE_BMSK                                                   0x800000
#define HWIO_TCL_R0_LCE_RULE_n_MATCH_MCAST_AND_L3_TYPE_SHFT                                                         23
#define HWIO_TCL_R0_CLKGATE_DISABLE1_LPM_HANDLER_BMSK                                                             0x10
#define HWIO_TCL_R0_CLKGATE_DISABLE1_LPM_HANDLER_SHFT                                                                4
#define HWIO_TCL_R0_CLKGATE_DISABLE1_TESTBUS_CAPTURE_BMSK                                                          0x8
#define HWIO_TCL_R0_CLKGATE_DISABLE1_TESTBUS_CAPTURE_SHFT                                                            3
#define HWIO_TCL_R0_CLKGATE_DISABLE2_ADDR(x)                                                                ((x) + 0x8b4)
#define HWIO_TCL_R0_CLKGATE_DISABLE2_PHYS(x)                                                                ((x) + 0x8b4)
#define HWIO_TCL_R0_CLKGATE_DISABLE2_OFFS                                                                   (0x8b4)
#define HWIO_TCL_R0_CLKGATE_DISABLE2_RMSK                                                                          0x3
#define HWIO_TCL_R0_CLKGATE_DISABLE2_POR                                                                    0x00000000
#define HWIO_TCL_R0_CLKGATE_DISABLE2_POR_RMSK                                                               0xffffffff
#define HWIO_TCL_R0_CLKGATE_DISABLE2_ATTR                                                                                0x3
#define HWIO_TCL_R0_CLKGATE_DISABLE2_IN(x)            \
                in_dword(HWIO_TCL_R0_CLKGATE_DISABLE2_ADDR(x))
#define HWIO_TCL_R0_CLKGATE_DISABLE2_INM(x, m)            \
                in_dword_masked(HWIO_TCL_R0_CLKGATE_DISABLE2_ADDR(x), m)
#define HWIO_TCL_R0_CLKGATE_DISABLE2_OUT(x, v)            \
                out_dword(HWIO_TCL_R0_CLKGATE_DISABLE2_ADDR(x),v)
#define HWIO_TCL_R0_CLKGATE_DISABLE2_OUTM(x,m,v) \
                out_dword_masked_ns(HWIO_TCL_R0_CLKGATE_DISABLE2_ADDR(x),m,v,HWIO_TCL_R0_CLKGATE_DISABLE2_IN(x))
#define HWIO_TCL_R0_CLKGATE_DISABLE2_MULTI_SRNG_P_BMSK                                                             0x2
#define HWIO_TCL_R0_CLKGATE_DISABLE2_MULTI_SRNG_P_SHFT                                                               1
#define HWIO_TCL_R0_CLKGATE_DISABLE2_MULTI_SRNG_C_BMSK                                                             0x1
#define HWIO_TCL_R0_CLKGATE_DISABLE2_MULTI_SRNG_C_SHFT                                                               0

#define HWIO_TCL_R0_LPM_FW_CTRL_ADDR(x)                                                                     ((x) + 0x914)
#define HWIO_TCL_R0_LPM_FW_CTRL_PHYS(x)                                                                     ((x) + 0x914)
#define HWIO_TCL_R0_LPM_FW_CTRL_OFFS                                                                        (0x914)
#define HWIO_TCL_R0_LPM_FW_CTRL_RMSK                                                                               0x7
#define HWIO_TCL_R0_LPM_FW_CTRL_POR                                                                         0x00000000
#define HWIO_TCL_R0_LPM_FW_CTRL_POR_RMSK                                                                    0xffffffff
#define HWIO_TCL_R0_LPM_FW_CTRL_ATTR                                                                                     0x3
#define HWIO_TCL_R0_LPM_FW_CTRL_IN(x)            \
                in_dword(HWIO_TCL_R0_LPM_FW_CTRL_ADDR(x))
#define HWIO_TCL_R0_LPM_FW_CTRL_INM(x, m)            \
                in_dword_masked(HWIO_TCL_R0_LPM_FW_CTRL_ADDR(x), m)
#define HWIO_TCL_R0_LPM_FW_CTRL_OUT(x, v)            \
                out_dword(HWIO_TCL_R0_LPM_FW_CTRL_ADDR(x),v)
#define HWIO_TCL_R0_LPM_FW_CTRL_OUTM(x,m,v) \
                out_dword_masked_ns(HWIO_TCL_R0_LPM_FW_CTRL_ADDR(x),m,v,HWIO_TCL_R0_LPM_FW_CTRL_IN(x))
#define HWIO_TCL_R0_LPM_FW_CTRL_SLEEP_REQ_ACK_OVR_MULTI_SRNG_P_BMSK                                                0x4
#define HWIO_TCL_R0_LPM_FW_CTRL_SLEEP_REQ_ACK_OVR_MULTI_SRNG_P_SHFT                                                  2
#define HWIO_TCL_R0_LPM_FW_CTRL_SLEEP_REQ_ACK_OVR_MULTI_SRNG_C_BMSK                                                0x2
#define HWIO_TCL_R0_LPM_FW_CTRL_SLEEP_REQ_ACK_OVR_MULTI_SRNG_C_SHFT                                                  1
#define HWIO_TCL_R0_LPM_FW_CTRL_SLEEP_REQ_BMSK                                                                     0x1
#define HWIO_TCL_R0_LPM_FW_CTRL_SLEEP_REQ_SHFT                                                                       0

#define HWIO_TCL_R0_SW2TCL1_RING_BASE_LSB_ADDR(x)                                                           ((x) + 0x918)
#define HWIO_TCL_R0_SW2TCL1_RING_BASE_MSB_ADDR(x)                                                           ((x) + 0x91c)
#define HWIO_TCL_R0_SW2TCL1_RING_BASE_MSB_RING_SIZE_BMSK                                                     0xfffff00
#define HWIO_TCL_R0_SW2TCL1_RING_BASE_MSB_RING_SIZE_SHFT                                                             8
#define HWIO_TCL_R0_SW2TCL1_RING_BASE_MSB_RING_BASE_ADDR_MSB_BMSK                                                 0xff
#define HWIO_TCL_R0_SW2TCL1_RING_BASE_MSB_RING_BASE_ADDR_MSB_SHFT                                                    0

#define HWIO_TCL_R0_SW2TCL1_RING_ID_ADDR(x)                                                                 ((x) + 0x920)
#define HWIO_TCL_R0_SW2TCL1_RING_ID_ENTRY_SIZE_BMSK                                                               0xff
#define HWIO_TCL_R0_SW2TCL1_RING_ID_ENTRY_SIZE_SHFT                                                                  0

#define HWIO_TCL_R0_SW2TCL1_RING_MISC_ADDR(x)                                                               ((x) + 0x928)
#define HWIO_TCL_R0_SW2TCL1_RING_MISC_TRANSACTION_TYPE_BMSK                                                   0x400000
#define HWIO_TCL_R0_SW2TCL1_RING_MISC_TRANSACTION_TYPE_SHFT                                                         22
#define HWIO_TCL_R0_SW2TCL1_RING_MISC_DATA_TLV_SWAP_BIT_BMSK                                                      0x20
#define HWIO_TCL_R0_SW2TCL1_RING_MISC_DATA_TLV_SWAP_BIT_SHFT                                                         5
#define HWIO_TCL_R0_SW2TCL1_RING_MISC_HOST_FW_SWAP_BIT_BMSK                                                       0x10
#define HWIO_TCL_R0_SW2TCL1_RING_MISC_HOST_FW_SWAP_BIT_SHFT                                                          4
#define HWIO_TCL_R0_SW2TCL1_RING_MISC_MSI_SWAP_BIT_BMSK                                                            0x8
#define HWIO_TCL_R0_SW2TCL1_RING_MISC_MSI_SWAP_BIT_SHFT                                                              3
#define HWIO_TCL_R0_SW2TCL1_RING_MISC_LOOPCNT_DISABLE_BMSK                                                         0x2
#define HWIO_TCL_R0_SW2TCL1_RING_MISC_LOOPCNT_DISABLE_SHFT                                                           1
#define HWIO_TCL_R0_SW2TCL1_RING_MISC_RING_ID_DISABLE_BMSK                                                         0x1
#define HWIO_TCL_R0_SW2TCL1_RING_MISC_RING_ID_DISABLE_SHFT                                                           0

#define HWIO_TCL_R0_SW2TCL1_RING_TP_ADDR_LSB_ADDR(x)                                                        ((x) + 0x934)
#define HWIO_TCL_R0_SW2TCL1_RING_TP_ADDR_MSB_ADDR(x)                                                        ((x) + 0x938)
#define HWIO_TCL_R0_SW2TCL1_RING_CONSUMER_INT_SETUP_IX0_ADDR(x)                                             ((x) + 0x948)
#define HWIO_TCL_R0_SW2TCL1_RING_CONSUMER_INT_SETUP_IX0_INTERRUPT_TIMER_THRESHOLD_BMSK                      0xffff0000
#define HWIO_TCL_R0_SW2TCL1_RING_CONSUMER_INT_SETUP_IX0_INTERRUPT_TIMER_THRESHOLD_SHFT                              16
#define HWIO_TCL_R0_SW2TCL1_RING_CONSUMER_INT_SETUP_IX0_BATCH_COUNTER_THRESHOLD_BMSK                            0x7fff
#define HWIO_TCL_R0_SW2TCL1_RING_CONSUMER_INT_SETUP_IX0_BATCH_COUNTER_THRESHOLD_SHFT                                 0

#define HWIO_TCL_R0_SW2TCL1_RING_CONSUMER_INT_SETUP_IX1_ADDR(x)                                             ((x) + 0x94c)
#define HWIO_TCL_R0_SW2TCL1_RING_CONSUMER_INT_SETUP_IX1_LOW_THRESHOLD_BMSK                                      0xffff
#define HWIO_TCL_R0_SW2TCL1_RING_CONSUMER_INT_SETUP_IX1_LOW_THRESHOLD_SHFT                                           0

#define HWIO_TCL_R0_SW2TCL1_RING_MSI1_BASE_LSB_ADDR(x)                                                      ((x) + 0x960)
#define HWIO_TCL_R0_SW2TCL1_RING_MSI1_BASE_MSB_ADDR(x)                                                      ((x) + 0x964)
#define HWIO_TCL_R0_SW2TCL1_RING_MSI1_BASE_MSB_MSI1_ENABLE_BMSK                                                  0x100
#define HWIO_TCL_R0_SW2TCL1_RING_MSI1_BASE_MSB_MSI1_ENABLE_SHFT                                                      8
#define HWIO_TCL_R0_SW2TCL1_RING_MSI1_BASE_MSB_ADDR_BMSK                                                          0xff
#define HWIO_TCL_R0_SW2TCL1_RING_MSI1_BASE_MSB_ADDR_SHFT                                                             0

#define HWIO_TCL_R0_SW2TCL1_RING_MSI1_DATA_ADDR(x)                                                          ((x) + 0x968)
#define HWIO_TCL_R0_SW2TCL2_RING_BASE_LSB_ADDR(x)                                                           ((x) + 0x990)
#define HWIO_TCL_R0_SW2TCL2_RING_MISC_TRANSACTION_TYPE_BMSK                                                   0x400000
#define HWIO_TCL_R0_SW2TCL2_RING_MISC_TRANSACTION_TYPE_SHFT                                                         22
#define HWIO_TCL_R0_SW2TCL3_RING_MISC_TRANSACTION_TYPE_BMSK                                                   0x400000
#define HWIO_TCL_R0_SW2TCL3_RING_MISC_TRANSACTION_TYPE_SHFT                                                         22
#define HWIO_TCL_R0_SW2TCL4_RING_MISC_TRANSACTION_TYPE_BMSK                                                   0x400000
#define HWIO_TCL_R0_SW2TCL4_RING_MISC_TRANSACTION_TYPE_SHFT                                                         22
#define HWIO_TCL_R0_SW2TCL5_RING_MISC_TRANSACTION_TYPE_BMSK                                                   0x400000
#define HWIO_TCL_R0_SW2TCL5_RING_MISC_TRANSACTION_TYPE_SHFT                                                         22
#define HWIO_TCL_R0_SW2TCL_CREDIT_RING_BASE_LSB_ADDR(x)                                                     ((x) + 0xb70)
#define HWIO_TCL_R0_SW2TCL_CREDIT_RING_BASE_MSB_RING_SIZE_BMSK                                               0xfffff00
#define HWIO_TCL_R0_SW2TCL_CREDIT_RING_BASE_MSB_RING_SIZE_SHFT                                                       8
#define HWIO_TCL_R0_SW2TCL_CREDIT_RING_MISC_TRANSACTION_TYPE_BMSK                                             0x400000
#define HWIO_TCL_R0_SW2TCL_CREDIT_RING_MISC_TRANSACTION_TYPE_SHFT                                                   22
#define HWIO_TCL_R0_FW2TCL1_RING_MISC_TRANSACTION_TYPE_BMSK                                                   0x400000
#define HWIO_TCL_R0_FW2TCL1_RING_MISC_TRANSACTION_TYPE_SHFT                                                         22
#define HWIO_TCL_R0_TCL2TQM_RING_MISC_TRANSACTION_TYPE_BMSK                                                  0x8000000
#define HWIO_TCL_R0_TCL2TQM_RING_MISC_TRANSACTION_TYPE_SHFT                                                         27
#define HWIO_TCL_R0_TCL_STATUS1_RING_BASE_LSB_ADDR(x)                                                       ((x) + 0xd50)
#define HWIO_TCL_R0_TCL_STATUS1_RING_BASE_MSB_RING_SIZE_BMSK                                                  0xffff00
#define HWIO_TCL_R0_TCL_STATUS1_RING_BASE_MSB_RING_SIZE_SHFT                                                         8
#define HWIO_TCL_R0_TCL_STATUS1_RING_MISC_TRANSACTION_TYPE_BMSK                                              0x8000000
#define HWIO_TCL_R0_TCL_STATUS1_RING_MISC_TRANSACTION_TYPE_SHFT                                                     27
#define HWIO_TCL_R0_TCL2FW_RING_MISC_TRANSACTION_TYPE_BMSK                                                   0x8000000
#define HWIO_TCL_R0_TCL2FW_RING_MISC_TRANSACTION_TYPE_SHFT                                                          27
#define HWIO_TCL_R0_MULTI_SRNG_CONS_RING_MISC_COMMON_ADDR(x)                                                ((x) + 0xedc)
#define HWIO_TCL_R0_MULTI_SRNG_CONS_RING_MISC_COMMON_PHYS(x)                                                ((x) + 0xedc)
#define HWIO_TCL_R0_MULTI_SRNG_CONS_RING_MISC_COMMON_OFFS                                                   (0xedc)
#define HWIO_TCL_R0_MULTI_SRNG_CONS_RING_MISC_COMMON_RMSK                                                     0x1fffff
#define HWIO_TCL_R0_MULTI_SRNG_CONS_RING_MISC_COMMON_POR                                                    0x00001000
#define HWIO_TCL_R0_MULTI_SRNG_CONS_RING_MISC_COMMON_POR_RMSK                                               0xffffffff
#define HWIO_TCL_R0_MULTI_SRNG_CONS_RING_MISC_COMMON_ATTR                                                                0x3
#define HWIO_TCL_R0_MULTI_SRNG_CONS_RING_MISC_COMMON_IN(x)            \
                in_dword(HWIO_TCL_R0_MULTI_SRNG_CONS_RING_MISC_COMMON_ADDR(x))
#define HWIO_TCL_R0_MULTI_SRNG_CONS_RING_MISC_COMMON_INM(x, m)            \
                in_dword_masked(HWIO_TCL_R0_MULTI_SRNG_CONS_RING_MISC_COMMON_ADDR(x), m)
#define HWIO_TCL_R0_MULTI_SRNG_CONS_RING_MISC_COMMON_OUT(x, v)            \
                out_dword(HWIO_TCL_R0_MULTI_SRNG_CONS_RING_MISC_COMMON_ADDR(x),v)
#define HWIO_TCL_R0_MULTI_SRNG_CONS_RING_MISC_COMMON_OUTM(x,m,v) \
                out_dword_masked_ns(HWIO_TCL_R0_MULTI_SRNG_CONS_RING_MISC_COMMON_ADDR(x),m,v,HWIO_TCL_R0_MULTI_SRNG_CONS_RING_MISC_COMMON_IN(x))
#define HWIO_TCL_R0_MULTI_SRNG_CONS_RING_MISC_COMMON_MISC_CONTROL_BMSK                                        0x1fe000
#define HWIO_TCL_R0_MULTI_SRNG_CONS_RING_MISC_COMMON_MISC_CONTROL_SHFT                                              13
#define HWIO_TCL_R0_MULTI_SRNG_CONS_RING_MISC_COMMON_SRNG_IS_IDLE_BMSK                                          0x1000
#define HWIO_TCL_R0_MULTI_SRNG_CONS_RING_MISC_COMMON_SRNG_IS_IDLE_SHFT                                              12
#define HWIO_TCL_R0_MULTI_SRNG_CONS_RING_MISC_COMMON_SRNG_SM1_STATE2_BMSK                                        0xc00
#define HWIO_TCL_R0_MULTI_SRNG_CONS_RING_MISC_COMMON_SRNG_SM1_STATE2_SHFT                                           10
#define HWIO_TCL_R0_MULTI_SRNG_CONS_RING_MISC_COMMON_SRNG_SM1_STATE1_BMSK                                        0x3c0
#define HWIO_TCL_R0_MULTI_SRNG_CONS_RING_MISC_COMMON_SRNG_SM1_STATE1_SHFT                                            6
#define HWIO_TCL_R0_MULTI_SRNG_CONS_RING_MISC_COMMON_SRNG_SM0_STATE2_BMSK                                         0x30
#define HWIO_TCL_R0_MULTI_SRNG_CONS_RING_MISC_COMMON_SRNG_SM0_STATE2_SHFT                                            4
#define HWIO_TCL_R0_MULTI_SRNG_CONS_RING_MISC_COMMON_SRNG_SM0_STATE1_BMSK                                          0xf
#define HWIO_TCL_R0_MULTI_SRNG_CONS_RING_MISC_COMMON_SRNG_SM0_STATE1_SHFT                                            0

#define HWIO_TCL_R0_MULTI_SRNG_CONS_RING_MISC_COMMON_EXT_ADDR(x)                                            ((x) + 0xee0)
#define HWIO_TCL_R0_MULTI_SRNG_CONS_RING_MISC_COMMON_EXT_PHYS(x)                                            ((x) + 0xee0)
#define HWIO_TCL_R0_MULTI_SRNG_CONS_RING_MISC_COMMON_EXT_OFFS                                               (0xee0)
#define HWIO_TCL_R0_MULTI_SRNG_CONS_RING_MISC_COMMON_EXT_RMSK                                                 0xffffff
#define HWIO_TCL_R0_MULTI_SRNG_CONS_RING_MISC_COMMON_EXT_POR                                                0x00000fff
#define HWIO_TCL_R0_MULTI_SRNG_CONS_RING_MISC_COMMON_EXT_POR_RMSK                                           0xffffffff
#define HWIO_TCL_R0_MULTI_SRNG_CONS_RING_MISC_COMMON_EXT_ATTR                                                            0x3
#define HWIO_TCL_R0_MULTI_SRNG_CONS_RING_MISC_COMMON_EXT_IN(x)            \
                in_dword(HWIO_TCL_R0_MULTI_SRNG_CONS_RING_MISC_COMMON_EXT_ADDR(x))
#define HWIO_TCL_R0_MULTI_SRNG_CONS_RING_MISC_COMMON_EXT_INM(x, m)            \
                in_dword_masked(HWIO_TCL_R0_MULTI_SRNG_CONS_RING_MISC_COMMON_EXT_ADDR(x), m)
#define HWIO_TCL_R0_MULTI_SRNG_CONS_RING_MISC_COMMON_EXT_OUT(x, v)            \
                out_dword(HWIO_TCL_R0_MULTI_SRNG_CONS_RING_MISC_COMMON_EXT_ADDR(x),v)
#define HWIO_TCL_R0_MULTI_SRNG_CONS_RING_MISC_COMMON_EXT_OUTM(x,m,v) \
                out_dword_masked_ns(HWIO_TCL_R0_MULTI_SRNG_CONS_RING_MISC_COMMON_EXT_ADDR(x),m,v,HWIO_TCL_R0_MULTI_SRNG_CONS_RING_MISC_COMMON_EXT_IN(x))
#define HWIO_TCL_R0_MULTI_SRNG_CONS_RING_MISC_COMMON_EXT_RING_WATCHDOG_TIMER_STATUS_BMSK                      0xfff000
#define HWIO_TCL_R0_MULTI_SRNG_CONS_RING_MISC_COMMON_EXT_RING_WATCHDOG_TIMER_STATUS_SHFT                            12
#define HWIO_TCL_R0_MULTI_SRNG_CONS_RING_MISC_COMMON_EXT_WATCHDOG_TIMEOUT_VALUE_BMSK                             0xfff
#define HWIO_TCL_R0_MULTI_SRNG_CONS_RING_MISC_COMMON_EXT_WATCHDOG_TIMEOUT_VALUE_SHFT                                 0

#define HWIO_TCL_R0_MULTI_SRNG_PROD_RING_MISC_COMMON_ADDR(x)                                                ((x) + 0xee4)
#define HWIO_TCL_R0_MULTI_SRNG_PROD_RING_MISC_COMMON_PHYS(x)                                                ((x) + 0xee4)
#define HWIO_TCL_R0_MULTI_SRNG_PROD_RING_MISC_COMMON_OFFS                                                   (0xee4)
#define HWIO_TCL_R0_MULTI_SRNG_PROD_RING_MISC_COMMON_RMSK                                                     0x1fffff
#define HWIO_TCL_R0_MULTI_SRNG_PROD_RING_MISC_COMMON_POR                                                    0x00001000
#define HWIO_TCL_R0_MULTI_SRNG_PROD_RING_MISC_COMMON_POR_RMSK                                               0xffffffff
#define HWIO_TCL_R0_MULTI_SRNG_PROD_RING_MISC_COMMON_ATTR                                                                0x3
#define HWIO_TCL_R0_MULTI_SRNG_PROD_RING_MISC_COMMON_IN(x)            \
                in_dword(HWIO_TCL_R0_MULTI_SRNG_PROD_RING_MISC_COMMON_ADDR(x))
#define HWIO_TCL_R0_MULTI_SRNG_PROD_RING_MISC_COMMON_INM(x, m)            \
                in_dword_masked(HWIO_TCL_R0_MULTI_SRNG_PROD_RING_MISC_COMMON_ADDR(x), m)
#define HWIO_TCL_R0_MULTI_SRNG_PROD_RING_MISC_COMMON_OUT(x, v)            \
                out_dword(HWIO_TCL_R0_MULTI_SRNG_PROD_RING_MISC_COMMON_ADDR(x),v)
#define HWIO_TCL_R0_MULTI_SRNG_PROD_RING_MISC_COMMON_OUTM(x,m,v) \
                out_dword_masked_ns(HWIO_TCL_R0_MULTI_SRNG_PROD_RING_MISC_COMMON_ADDR(x),m,v,HWIO_TCL_R0_MULTI_SRNG_PROD_RING_MISC_COMMON_IN(x))
#define HWIO_TCL_R0_MULTI_SRNG_PROD_RING_MISC_COMMON_MISC_CONTROL_BMSK                                        0x1fe000
#define HWIO_TCL_R0_MULTI_SRNG_PROD_RING_MISC_COMMON_MISC_CONTROL_SHFT                                              13
#define HWIO_TCL_R0_MULTI_SRNG_PROD_RING_MISC_COMMON_SRNG_IS_IDLE_BMSK                                          0x1000
#define HWIO_TCL_R0_MULTI_SRNG_PROD_RING_MISC_COMMON_SRNG_IS_IDLE_SHFT                                              12
#define HWIO_TCL_R0_MULTI_SRNG_PROD_RING_MISC_COMMON_SRNG_SM1_STATE2_BMSK                                        0xc00
#define HWIO_TCL_R0_MULTI_SRNG_PROD_RING_MISC_COMMON_SRNG_SM1_STATE2_SHFT                                           10
#define HWIO_TCL_R0_MULTI_SRNG_PROD_RING_MISC_COMMON_SRNG_SM1_STATE1_BMSK                                        0x3c0
#define HWIO_TCL_R0_MULTI_SRNG_PROD_RING_MISC_COMMON_SRNG_SM1_STATE1_SHFT                                            6
#define HWIO_TCL_R0_MULTI_SRNG_PROD_RING_MISC_COMMON_SRNG_SM0_STATE2_BMSK                                         0x30
#define HWIO_TCL_R0_MULTI_SRNG_PROD_RING_MISC_COMMON_SRNG_SM0_STATE2_SHFT                                            4
#define HWIO_TCL_R0_MULTI_SRNG_PROD_RING_MISC_COMMON_SRNG_SM0_STATE1_BMSK                                          0xf
#define HWIO_TCL_R0_MULTI_SRNG_PROD_RING_MISC_COMMON_SRNG_SM0_STATE1_SHFT                                            0

#define HWIO_TCL_R0_MULTI_SRNG_PROD_RING_MISC_COMMON_EXT_ADDR(x)                                            ((x) + 0xee8)
#define HWIO_TCL_R0_MULTI_SRNG_PROD_RING_MISC_COMMON_EXT_PHYS(x)                                            ((x) + 0xee8)
#define HWIO_TCL_R0_MULTI_SRNG_PROD_RING_MISC_COMMON_EXT_OFFS                                               (0xee8)
#define HWIO_TCL_R0_MULTI_SRNG_PROD_RING_MISC_COMMON_EXT_RMSK                                                 0xffffff
#define HWIO_TCL_R0_MULTI_SRNG_PROD_RING_MISC_COMMON_EXT_POR                                                0x00000fff
#define HWIO_TCL_R0_MULTI_SRNG_PROD_RING_MISC_COMMON_EXT_POR_RMSK                                           0xffffffff
#define HWIO_TCL_R0_MULTI_SRNG_PROD_RING_MISC_COMMON_EXT_ATTR                                                            0x3
#define HWIO_TCL_R0_MULTI_SRNG_PROD_RING_MISC_COMMON_EXT_IN(x)            \
                in_dword(HWIO_TCL_R0_MULTI_SRNG_PROD_RING_MISC_COMMON_EXT_ADDR(x))
#define HWIO_TCL_R0_MULTI_SRNG_PROD_RING_MISC_COMMON_EXT_INM(x, m)            \
                in_dword_masked(HWIO_TCL_R0_MULTI_SRNG_PROD_RING_MISC_COMMON_EXT_ADDR(x), m)
#define HWIO_TCL_R0_MULTI_SRNG_PROD_RING_MISC_COMMON_EXT_OUT(x, v)            \
                out_dword(HWIO_TCL_R0_MULTI_SRNG_PROD_RING_MISC_COMMON_EXT_ADDR(x),v)
#define HWIO_TCL_R0_MULTI_SRNG_PROD_RING_MISC_COMMON_EXT_OUTM(x,m,v) \
                out_dword_masked_ns(HWIO_TCL_R0_MULTI_SRNG_PROD_RING_MISC_COMMON_EXT_ADDR(x),m,v,HWIO_TCL_R0_MULTI_SRNG_PROD_RING_MISC_COMMON_EXT_IN(x))
#define HWIO_TCL_R0_MULTI_SRNG_PROD_RING_MISC_COMMON_EXT_RING_WATCHDOG_TIMER_STATUS_BMSK                      0xfff000
#define HWIO_TCL_R0_MULTI_SRNG_PROD_RING_MISC_COMMON_EXT_RING_WATCHDOG_TIMER_STATUS_SHFT                            12
#define HWIO_TCL_R0_MULTI_SRNG_PROD_RING_MISC_COMMON_EXT_WATCHDOG_TIMEOUT_VALUE_BMSK                             0xfff
#define HWIO_TCL_R0_MULTI_SRNG_PROD_RING_MISC_COMMON_EXT_WATCHDOG_TIMEOUT_VALUE_SHFT                                 0

#define HWIO_TCL_R1_LPM_REQ_HANDLER_STATUS_ADDR(x)                                                          ((x) + 0x1000)
#define HWIO_TCL_R1_LPM_REQ_HANDLER_STATUS_PHYS(x)                                                          ((x) + 0x1000)
#define HWIO_TCL_R1_LPM_REQ_HANDLER_STATUS_OFFS                                                             (0x1000)
#define HWIO_TCL_R1_LPM_REQ_HANDLER_STATUS_RMSK                                                                   0x7f
#define HWIO_TCL_R1_LPM_REQ_HANDLER_STATUS_POR                                                              0x00000000
#define HWIO_TCL_R1_LPM_REQ_HANDLER_STATUS_POR_RMSK                                                         0xffffffff
#define HWIO_TCL_R1_LPM_REQ_HANDLER_STATUS_ATTR                                                                          0x1
#define HWIO_TCL_R1_LPM_REQ_HANDLER_STATUS_IN(x)            \
                in_dword(HWIO_TCL_R1_LPM_REQ_HANDLER_STATUS_ADDR(x))
#define HWIO_TCL_R1_LPM_REQ_HANDLER_STATUS_INM(x, m)            \
                in_dword_masked(HWIO_TCL_R1_LPM_REQ_HANDLER_STATUS_ADDR(x), m)
#define HWIO_TCL_R1_LPM_REQ_HANDLER_STATUS_SLEEP_REQ_ACK_MULTI_SRNG_P_BMSK                                        0x40
#define HWIO_TCL_R1_LPM_REQ_HANDLER_STATUS_SLEEP_REQ_ACK_MULTI_SRNG_P_SHFT                                           6
#define HWIO_TCL_R1_LPM_REQ_HANDLER_STATUS_SLEEP_REQ_ACK_MULTI_SRNG_C_BMSK                                        0x20
#define HWIO_TCL_R1_LPM_REQ_HANDLER_STATUS_SLEEP_REQ_ACK_MULTI_SRNG_C_SHFT                                           5
#define HWIO_TCL_R1_LPM_REQ_HANDLER_STATUS_SLEEP_REQ_ACK_TO_LPM_BMSK                                              0x10
#define HWIO_TCL_R1_LPM_REQ_HANDLER_STATUS_SLEEP_REQ_ACK_TO_LPM_SHFT                                                 4
#define HWIO_TCL_R1_LPM_REQ_HANDLER_STATUS_SLEEP_REQ_FROM_LPM_BMSK                                                 0x8
#define HWIO_TCL_R1_LPM_REQ_HANDLER_STATUS_SLEEP_REQ_FROM_LPM_SHFT                                                   3
#define HWIO_TCL_R1_LPM_REQ_HANDLER_STATUS_SM_STATE_BMSK                                                           0x7
#define HWIO_TCL_R1_LPM_REQ_HANDLER_STATUS_SM_STATE_SHFT                                                             0

#define HWIO_TCL_R1_STATUS_TCL_PEER_FETCH_CTRL_IDLE_BMSK                                                       0x20000
#define HWIO_TCL_R1_STATUS_TCL_PEER_FETCH_CTRL_IDLE_SHFT                                                            17
#define HWIO_TCL_R1_WDOG_STATUS_TCL_PEER_FETCH_CTRL_IDLE_BMSK                                                  0x20000
#define HWIO_TCL_R1_WDOG_STATUS_TCL_PEER_FETCH_CTRL_IDLE_SHFT                                                       17
#define HWIO_TCL_R1_TESTBUS_CTRL_ADDR(x)                                                                    ((x) + 0x1030)
#define HWIO_TCL_R1_TESTBUS_CTRL_PHYS(x)                                                                    ((x) + 0x1030)
#define HWIO_TCL_R1_TESTBUS_CTRL_OFFS                                                                       (0x1030)
#define HWIO_TCL_R1_TESTBUS_CTRL_RMSK                                                                            0x1ff
#define HWIO_TCL_R1_TESTBUS_CTRL_POR                                                                        0x00000000
#define HWIO_TCL_R1_TESTBUS_CTRL_POR_RMSK                                                                   0xffffffff
#define HWIO_TCL_R1_TESTBUS_CTRL_ATTR                                                                                    0x3
#define HWIO_TCL_R1_TESTBUS_CTRL_IN(x)            \
                in_dword(HWIO_TCL_R1_TESTBUS_CTRL_ADDR(x))
#define HWIO_TCL_R1_TESTBUS_CTRL_INM(x, m)            \
                in_dword_masked(HWIO_TCL_R1_TESTBUS_CTRL_ADDR(x), m)
#define HWIO_TCL_R1_TESTBUS_CTRL_OUT(x, v)            \
                out_dword(HWIO_TCL_R1_TESTBUS_CTRL_ADDR(x),v)
#define HWIO_TCL_R1_TESTBUS_CTRL_OUTM(x,m,v) \
                out_dword_masked_ns(HWIO_TCL_R1_TESTBUS_CTRL_ADDR(x),m,v,HWIO_TCL_R1_TESTBUS_CTRL_IN(x))
#define HWIO_TCL_R1_TESTBUS_CTRL_HW_ERROR_INTERRUPT_TESTBUS_OVERWRITE_BMSK                                       0x100
#define HWIO_TCL_R1_TESTBUS_CTRL_HW_ERROR_INTERRUPT_TESTBUS_OVERWRITE_SHFT                                           8
#define HWIO_TCL_R1_TESTBUS_CTRL_BLOCK_SELECT_BMSK                                                                0xc0
#define HWIO_TCL_R1_TESTBUS_CTRL_BLOCK_SELECT_SHFT                                                                   6
#define HWIO_TCL_R1_TESTBUS_CTRL_SUBBLOCK_SELECT_BMSK                                                             0x3f
#define HWIO_TCL_R1_TESTBUS_CTRL_SUBBLOCK_SELECT_SHFT                                                                0

#define HWIO_TCL_R1_TESTBUS_CAPTURE_n_ADDR(base,n)                                                          ((base) + 0X1034 + (0x4*(n)))
#define HWIO_TCL_R1_TESTBUS_CAPTURE_n_PHYS(base,n)                                                          ((base) + 0X1034 + (0x4*(n)))
#define HWIO_TCL_R1_TESTBUS_CAPTURE_n_OFFS(n)                                                               (0X1034 + (0x4*(n)))
#define HWIO_TCL_R1_TESTBUS_CAPTURE_n_RMSK                                                                  0xffffffff
#define HWIO_TCL_R1_TESTBUS_CAPTURE_n_MAXn                                                                         511
#define HWIO_TCL_R1_TESTBUS_CAPTURE_n_POR                                                                   0x00000000
#define HWIO_TCL_R1_TESTBUS_CAPTURE_n_POR_RMSK                                                              0xffffffff
#define HWIO_TCL_R1_TESTBUS_CAPTURE_n_ATTR                                                                               0x1
#define HWIO_TCL_R1_TESTBUS_CAPTURE_n_INI(base,n)                \
                in_dword_masked(HWIO_TCL_R1_TESTBUS_CAPTURE_n_ADDR(base,n), HWIO_TCL_R1_TESTBUS_CAPTURE_n_RMSK)
#define HWIO_TCL_R1_TESTBUS_CAPTURE_n_INMI(base,n,mask)        \
                in_dword_masked(HWIO_TCL_R1_TESTBUS_CAPTURE_n_ADDR(base,n), mask)
#define HWIO_TCL_R1_TESTBUS_CAPTURE_n_DATA_BMSK                                                             0xffffffff
#define HWIO_TCL_R1_TESTBUS_CAPTURE_n_DATA_SHFT                                                                      0

#define HWIO_TCL_R2_SW2TCL1_RING_HP_ADDR(x)                                                                 ((x) + 0x2000)
#define HWIO_TCL_R2_SW2TCL1_RING_TP_ADDR(x)                                                                 ((x) + 0x2004)
#define HWIO_TCL_R2_SW2TCL2_RING_HP_ADDR(x)                                                                 ((x) + 0x2008)
#define HWIO_TCL_R2_SW2TCL_CREDIT_RING_HP_ADDR(x)                                                           ((x) + 0x2028)
#define HWIO_TCL_R2_TCL_STATUS1_RING_HP_ADDR(x)                                                             ((x) + 0x2048)
#define HWIO_UMAC_NOC_SIDEBANDMANAGER_CMN_SBM_SENSEIN0_LOW_COEX_APB2AXI_NOPX_BMSK                                   0x80000
#define HWIO_UMAC_NOC_SIDEBANDMANAGER_CMN_SBM_SENSEIN0_LOW_COEX_APB2AXI_NOPX_SHFT                                        19
#define HWIO_UMAC_NOC_SIDEBANDMANAGER_CMN_SBM_SENSEIN0_LOW_COEX_APB2AXI_XWSLVERR_BMSK                               0x40000
#define HWIO_UMAC_NOC_SIDEBANDMANAGER_CMN_SBM_SENSEIN0_LOW_COEX_APB2AXI_XWSLVERR_SHFT                                    18
#define HWIO_UMAC_NOC_SIDEBANDMANAGER_CMN_SBM_SENSEIN0_LOW_COEX_APB2AXI_XWDECERR_BMSK                               0x20000
#define HWIO_UMAC_NOC_SIDEBANDMANAGER_CMN_SBM_SENSEIN0_LOW_COEX_APB2AXI_XWDECERR_SHFT                                    17
#define HWIO_UMAC_NOC_QNS4M_PHY_QOSGEN_MAINCTL_LOW_QOSDISABLE_BMSK                                                0x1000000
#define HWIO_UMAC_NOC_QNS4M_PHY_QOSGEN_MAINCTL_LOW_QOSDISABLE_SHFT                                                       24
#define HWIO_UMAC_NOC_XM_UMXI_QOSGEN_MAINCTL_LOW_QOSDISABLE_BMSK                                                  0x1000000
#define HWIO_UMAC_NOC_XM_UMXI_QOSGEN_MAINCTL_LOW_QOSDISABLE_SHFT                                                         24
#define HWIO_UMAC_NOC_XM_DMAC_QOSGEN_MAINCTL_LOW_QOSDISABLE_BMSK                                                  0x1000000
#define HWIO_UMAC_NOC_XM_DMAC_QOSGEN_MAINCTL_LOW_QOSDISABLE_SHFT                                                         24
#define HWIO_UMAC_NOC_XM_PMAC0_QOSGEN_MAINCTL_LOW_QOSDISABLE_BMSK                                                 0x1000000
#define HWIO_UMAC_NOC_XM_PMAC0_QOSGEN_MAINCTL_LOW_QOSDISABLE_SHFT                                                        24
#define HWIO_UMAC_NOC_XM_PMAC1_QOSGEN_MAINCTL_LOW_QOSDISABLE_BMSK                                                 0x1000000
#define HWIO_UMAC_NOC_XM_PMAC1_QOSGEN_MAINCTL_LOW_QOSDISABLE_SHFT                                                        24
#endif
