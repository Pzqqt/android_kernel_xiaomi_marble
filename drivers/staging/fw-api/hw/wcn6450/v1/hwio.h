
/*
 * Copyright (c) 2023 Qualcomm Innovation Center, Inc. All rights reserved.
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



#ifndef __HWIO_H__
#define __HWIO_H__

#define HOST_SOC_A_WFSS_CE0_WFSS_CE_0_REG_REG_BASE                                                         (HOST_SOC_WFSS_CE_REG_TOP_BASE      + 0x00000000)
#define HOST_SOC_A_WFSS_CE0_WFSS_CE_0_REG_REG_BASE_SIZE                                                    0x1000
#define HOST_SOC_A_WFSS_CE0_WFSS_CE_0_REG_REG_BASE_USED                                                    0x8c

#define HWIO_HOST_SOC_A_WFSS_CE0_SR_BA_LOW_ADDR                                                            (HOST_SOC_A_WFSS_CE0_WFSS_CE_0_REG_REG_BASE      + 0x0)
#define HWIO_HOST_SOC_A_WFSS_CE0_SR_BA_LOW_RMSK                                                            0xffffffff
#define HWIO_HOST_SOC_A_WFSS_CE0_SR_BA_LOW_IN                    \
                in_dword(HWIO_HOST_SOC_A_WFSS_CE0_SR_BA_LOW_ADDR)
#define HWIO_HOST_SOC_A_WFSS_CE0_SR_BA_LOW_INM(m)            \
                in_dword_masked(HWIO_HOST_SOC_A_WFSS_CE0_SR_BA_LOW_ADDR, m)
#define HWIO_HOST_SOC_A_WFSS_CE0_SR_BA_LOW_OUT(v)            \
                out_dword(HWIO_HOST_SOC_A_WFSS_CE0_SR_BA_LOW_ADDR,v)
#define HWIO_HOST_SOC_A_WFSS_CE0_SR_BA_LOW_OUTM(m,v) \
                out_dword_masked_ns(HWIO_HOST_SOC_A_WFSS_CE0_SR_BA_LOW_ADDR,m,v,HWIO_HOST_SOC_A_WFSS_CE0_SR_BA_LOW_IN)
#define HWIO_HOST_SOC_A_WFSS_CE0_SR_BA_LOW_BASE_ADDR_LOW_BMSK                                              0xffffffff
#define HWIO_HOST_SOC_A_WFSS_CE0_SR_BA_LOW_BASE_ADDR_LOW_SHFT                                                       0

#define HWIO_HOST_SOC_A_WFSS_CE0_SR_BA_HIGH_ADDR                                                           (HOST_SOC_A_WFSS_CE0_WFSS_CE_0_REG_REG_BASE      + 0x4)
#define HWIO_HOST_SOC_A_WFSS_CE0_SR_BA_HIGH_RMSK                                                                 0xff
#define HWIO_HOST_SOC_A_WFSS_CE0_SR_BA_HIGH_IN                    \
                in_dword(HWIO_HOST_SOC_A_WFSS_CE0_SR_BA_HIGH_ADDR)
#define HWIO_HOST_SOC_A_WFSS_CE0_SR_BA_HIGH_INM(m)            \
                in_dword_masked(HWIO_HOST_SOC_A_WFSS_CE0_SR_BA_HIGH_ADDR, m)
#define HWIO_HOST_SOC_A_WFSS_CE0_SR_BA_HIGH_OUT(v)            \
                out_dword(HWIO_HOST_SOC_A_WFSS_CE0_SR_BA_HIGH_ADDR,v)
#define HWIO_HOST_SOC_A_WFSS_CE0_SR_BA_HIGH_OUTM(m,v) \
                out_dword_masked_ns(HWIO_HOST_SOC_A_WFSS_CE0_SR_BA_HIGH_ADDR,m,v,HWIO_HOST_SOC_A_WFSS_CE0_SR_BA_HIGH_IN)
#define HWIO_HOST_SOC_A_WFSS_CE0_SR_BA_HIGH_BASE_ADDR_HIGH_BMSK                                                  0xff
#define HWIO_HOST_SOC_A_WFSS_CE0_SR_BA_HIGH_BASE_ADDR_HIGH_SHFT                                                     0

#define HWIO_HOST_SOC_A_WFSS_CE0_SR_SIZE_ADDR                                                              (HOST_SOC_A_WFSS_CE0_WFSS_CE_0_REG_REG_BASE      + 0x8)
#define HWIO_HOST_SOC_A_WFSS_CE0_SR_SIZE_RMSK                                                              0xffffffff
#define HWIO_HOST_SOC_A_WFSS_CE0_SR_SIZE_IN                    \
                in_dword(HWIO_HOST_SOC_A_WFSS_CE0_SR_SIZE_ADDR)
#define HWIO_HOST_SOC_A_WFSS_CE0_SR_SIZE_INM(m)            \
                in_dword_masked(HWIO_HOST_SOC_A_WFSS_CE0_SR_SIZE_ADDR, m)
#define HWIO_HOST_SOC_A_WFSS_CE0_SR_SIZE_OUT(v)            \
                out_dword(HWIO_HOST_SOC_A_WFSS_CE0_SR_SIZE_ADDR,v)
#define HWIO_HOST_SOC_A_WFSS_CE0_SR_SIZE_OUTM(m,v) \
                out_dword_masked_ns(HWIO_HOST_SOC_A_WFSS_CE0_SR_SIZE_ADDR,m,v,HWIO_HOST_SOC_A_WFSS_CE0_SR_SIZE_IN)
#define HWIO_HOST_SOC_A_WFSS_CE0_SR_SIZE_START_OFFSET_BMSK                                                 0xffff0000
#define HWIO_HOST_SOC_A_WFSS_CE0_SR_SIZE_START_OFFSET_SHFT                                                         16
#define HWIO_HOST_SOC_A_WFSS_CE0_SR_SIZE_SIZE_BMSK                                                             0xffff
#define HWIO_HOST_SOC_A_WFSS_CE0_SR_SIZE_SIZE_SHFT                                                                  0

#define HWIO_HOST_SOC_A_WFSS_CE0_DR_BA_LOW_ADDR                                                            (HOST_SOC_A_WFSS_CE0_WFSS_CE_0_REG_REG_BASE      + 0xc)
#define HWIO_HOST_SOC_A_WFSS_CE0_DR_BA_LOW_RMSK                                                            0xffffffff
#define HWIO_HOST_SOC_A_WFSS_CE0_DR_BA_LOW_IN                    \
                in_dword(HWIO_HOST_SOC_A_WFSS_CE0_DR_BA_LOW_ADDR)
#define HWIO_HOST_SOC_A_WFSS_CE0_DR_BA_LOW_INM(m)            \
                in_dword_masked(HWIO_HOST_SOC_A_WFSS_CE0_DR_BA_LOW_ADDR, m)
#define HWIO_HOST_SOC_A_WFSS_CE0_DR_BA_LOW_OUT(v)            \
                out_dword(HWIO_HOST_SOC_A_WFSS_CE0_DR_BA_LOW_ADDR,v)
#define HWIO_HOST_SOC_A_WFSS_CE0_DR_BA_LOW_OUTM(m,v) \
                out_dword_masked_ns(HWIO_HOST_SOC_A_WFSS_CE0_DR_BA_LOW_ADDR,m,v,HWIO_HOST_SOC_A_WFSS_CE0_DR_BA_LOW_IN)
#define HWIO_HOST_SOC_A_WFSS_CE0_DR_BA_LOW_BASE_ADDR_LOW_BMSK                                              0xffffffff
#define HWIO_HOST_SOC_A_WFSS_CE0_DR_BA_LOW_BASE_ADDR_LOW_SHFT                                                       0

#define HWIO_HOST_SOC_A_WFSS_CE0_DR_BA_HIGH_ADDR                                                           (HOST_SOC_A_WFSS_CE0_WFSS_CE_0_REG_REG_BASE      + 0x10)
#define HWIO_HOST_SOC_A_WFSS_CE0_DR_BA_HIGH_RMSK                                                                0x3ff
#define HWIO_HOST_SOC_A_WFSS_CE0_DR_BA_HIGH_IN                    \
                in_dword(HWIO_HOST_SOC_A_WFSS_CE0_DR_BA_HIGH_ADDR)
#define HWIO_HOST_SOC_A_WFSS_CE0_DR_BA_HIGH_INM(m)            \
                in_dword_masked(HWIO_HOST_SOC_A_WFSS_CE0_DR_BA_HIGH_ADDR, m)
#define HWIO_HOST_SOC_A_WFSS_CE0_DR_BA_HIGH_OUT(v)            \
                out_dword(HWIO_HOST_SOC_A_WFSS_CE0_DR_BA_HIGH_ADDR,v)
#define HWIO_HOST_SOC_A_WFSS_CE0_DR_BA_HIGH_OUTM(m,v) \
                out_dword_masked_ns(HWIO_HOST_SOC_A_WFSS_CE0_DR_BA_HIGH_ADDR,m,v,HWIO_HOST_SOC_A_WFSS_CE0_DR_BA_HIGH_IN)
#define HWIO_HOST_SOC_A_WFSS_CE0_DR_BA_HIGH_DESC_SKIP_DWORD_BMSK                                                0x300
#define HWIO_HOST_SOC_A_WFSS_CE0_DR_BA_HIGH_DESC_SKIP_DWORD_SHFT                                                    8
#define HWIO_HOST_SOC_A_WFSS_CE0_DR_BA_HIGH_BASE_ADDR_HIGH_BMSK                                                  0xff
#define HWIO_HOST_SOC_A_WFSS_CE0_DR_BA_HIGH_BASE_ADDR_HIGH_SHFT                                                     0

#define HWIO_HOST_SOC_A_WFSS_CE0_DR_SIZE_ADDR                                                              (HOST_SOC_A_WFSS_CE0_WFSS_CE_0_REG_REG_BASE      + 0x14)
#define HWIO_HOST_SOC_A_WFSS_CE0_DR_SIZE_RMSK                                                              0xffffffff
#define HWIO_HOST_SOC_A_WFSS_CE0_DR_SIZE_IN                    \
                in_dword(HWIO_HOST_SOC_A_WFSS_CE0_DR_SIZE_ADDR)
#define HWIO_HOST_SOC_A_WFSS_CE0_DR_SIZE_INM(m)            \
                in_dword_masked(HWIO_HOST_SOC_A_WFSS_CE0_DR_SIZE_ADDR, m)
#define HWIO_HOST_SOC_A_WFSS_CE0_DR_SIZE_OUT(v)            \
                out_dword(HWIO_HOST_SOC_A_WFSS_CE0_DR_SIZE_ADDR,v)
#define HWIO_HOST_SOC_A_WFSS_CE0_DR_SIZE_OUTM(m,v) \
                out_dword_masked_ns(HWIO_HOST_SOC_A_WFSS_CE0_DR_SIZE_ADDR,m,v,HWIO_HOST_SOC_A_WFSS_CE0_DR_SIZE_IN)
#define HWIO_HOST_SOC_A_WFSS_CE0_DR_SIZE_START_OFFSET_BMSK                                                 0xffff0000
#define HWIO_HOST_SOC_A_WFSS_CE0_DR_SIZE_START_OFFSET_SHFT                                                         16
#define HWIO_HOST_SOC_A_WFSS_CE0_DR_SIZE_SIZE_BMSK                                                             0xffff
#define HWIO_HOST_SOC_A_WFSS_CE0_DR_SIZE_SIZE_SHFT                                                                  0

#define HWIO_HOST_SOC_A_WFSS_CE0_CE_CTRL1_ADDR                                                             (HOST_SOC_A_WFSS_CE0_WFSS_CE_0_REG_REG_BASE      + 0x18)
#define HWIO_HOST_SOC_A_WFSS_CE0_CE_CTRL1_RMSK                                                              0x1ffffff
#define HWIO_HOST_SOC_A_WFSS_CE0_CE_CTRL1_IN                    \
                in_dword(HWIO_HOST_SOC_A_WFSS_CE0_CE_CTRL1_ADDR)
#define HWIO_HOST_SOC_A_WFSS_CE0_CE_CTRL1_INM(m)            \
                in_dword_masked(HWIO_HOST_SOC_A_WFSS_CE0_CE_CTRL1_ADDR, m)
#define HWIO_HOST_SOC_A_WFSS_CE0_CE_CTRL1_OUT(v)            \
                out_dword(HWIO_HOST_SOC_A_WFSS_CE0_CE_CTRL1_ADDR,v)
#define HWIO_HOST_SOC_A_WFSS_CE0_CE_CTRL1_OUTM(m,v) \
                out_dword_masked_ns(HWIO_HOST_SOC_A_WFSS_CE0_CE_CTRL1_ADDR,m,v,HWIO_HOST_SOC_A_WFSS_CE0_CE_CTRL1_IN)
#define HWIO_HOST_SOC_A_WFSS_CE0_CE_CTRL1_TARGET_MSI_EN_BMSK                                                0x1000000
#define HWIO_HOST_SOC_A_WFSS_CE0_CE_CTRL1_TARGET_MSI_EN_SHFT                                                       24
#define HWIO_HOST_SOC_A_WFSS_CE0_CE_CTRL1_FW_EN_BMSK                                                         0x800000
#define HWIO_HOST_SOC_A_WFSS_CE0_CE_CTRL1_FW_EN_SHFT                                                               23
#define HWIO_HOST_SOC_A_WFSS_CE0_CE_CTRL1_SECURITY_DEST_BMSK                                                 0x400000
#define HWIO_HOST_SOC_A_WFSS_CE0_CE_CTRL1_SECURITY_DEST_SHFT                                                       22
#define HWIO_HOST_SOC_A_WFSS_CE0_CE_CTRL1_SECURITY_SRC_BMSK                                                  0x200000
#define HWIO_HOST_SOC_A_WFSS_CE0_CE_CTRL1_SECURITY_SRC_SHFT                                                        21
#define HWIO_HOST_SOC_A_WFSS_CE0_CE_CTRL1_PREFETCH_EN_BMSK                                                   0x100000
#define HWIO_HOST_SOC_A_WFSS_CE0_CE_CTRL1_PREFETCH_EN_SHFT                                                         20
#define HWIO_HOST_SOC_A_WFSS_CE0_CE_CTRL1_IDX_UPD_EN_BMSK                                                     0x80000
#define HWIO_HOST_SOC_A_WFSS_CE0_CE_CTRL1_IDX_UPD_EN_SHFT                                                          19
#define HWIO_HOST_SOC_A_WFSS_CE0_CE_CTRL1_DST_RING_BYTE_SWAP_EN_BMSK                                          0x40000
#define HWIO_HOST_SOC_A_WFSS_CE0_CE_CTRL1_DST_RING_BYTE_SWAP_EN_SHFT                                               18
#define HWIO_HOST_SOC_A_WFSS_CE0_CE_CTRL1_SRC_RING_BYTE_SWAP_EN_BMSK                                          0x20000
#define HWIO_HOST_SOC_A_WFSS_CE0_CE_CTRL1_SRC_RING_BYTE_SWAP_EN_SHFT                                               17
#define HWIO_HOST_SOC_A_WFSS_CE0_CE_CTRL1_HOST_MSI_EN_BMSK                                                    0x10000
#define HWIO_HOST_SOC_A_WFSS_CE0_CE_CTRL1_HOST_MSI_EN_SHFT                                                         16
#define HWIO_HOST_SOC_A_WFSS_CE0_CE_CTRL1_DEST_MAX_LENGTH_BMSK                                                 0xffff
#define HWIO_HOST_SOC_A_WFSS_CE0_CE_CTRL1_DEST_MAX_LENGTH_SHFT                                                      0

#define HWIO_HOST_SOC_A_WFSS_CE0_CE_CTRL2_ADDR                                                             (HOST_SOC_A_WFSS_CE0_WFSS_CE_0_REG_REG_BASE      + 0x1c)
#define HWIO_HOST_SOC_A_WFSS_CE0_CE_CTRL2_RMSK                                                                    0xf
#define HWIO_HOST_SOC_A_WFSS_CE0_CE_CTRL2_IN                    \
                in_dword(HWIO_HOST_SOC_A_WFSS_CE0_CE_CTRL2_ADDR)
#define HWIO_HOST_SOC_A_WFSS_CE0_CE_CTRL2_INM(m)            \
                in_dword_masked(HWIO_HOST_SOC_A_WFSS_CE0_CE_CTRL2_ADDR, m)
#define HWIO_HOST_SOC_A_WFSS_CE0_CE_CTRL2_OUT(v)            \
                out_dword(HWIO_HOST_SOC_A_WFSS_CE0_CE_CTRL2_ADDR,v)
#define HWIO_HOST_SOC_A_WFSS_CE0_CE_CTRL2_OUTM(m,v) \
                out_dword_masked_ns(HWIO_HOST_SOC_A_WFSS_CE0_CE_CTRL2_ADDR,m,v,HWIO_HOST_SOC_A_WFSS_CE0_CE_CTRL2_IN)
#define HWIO_HOST_SOC_A_WFSS_CE0_CE_CTRL2_DST_AXI_MAX_LEN_BMSK                                                    0xc
#define HWIO_HOST_SOC_A_WFSS_CE0_CE_CTRL2_DST_AXI_MAX_LEN_SHFT                                                      2
#define HWIO_HOST_SOC_A_WFSS_CE0_CE_CTRL2_SRC_AXI_MAX_LEN_BMSK                                                    0x3
#define HWIO_HOST_SOC_A_WFSS_CE0_CE_CTRL2_SRC_AXI_MAX_LEN_SHFT                                                      0

#define HWIO_HOST_SOC_A_WFSS_CE0_CE_CMD_ADDR                                                               (HOST_SOC_A_WFSS_CE0_WFSS_CE_0_REG_REG_BASE      + 0x20)
#define HWIO_HOST_SOC_A_WFSS_CE0_CE_CMD_RMSK                                                                      0xf
#define HWIO_HOST_SOC_A_WFSS_CE0_CE_CMD_IN                    \
                in_dword(HWIO_HOST_SOC_A_WFSS_CE0_CE_CMD_ADDR)
#define HWIO_HOST_SOC_A_WFSS_CE0_CE_CMD_INM(m)            \
                in_dword_masked(HWIO_HOST_SOC_A_WFSS_CE0_CE_CMD_ADDR, m)
#define HWIO_HOST_SOC_A_WFSS_CE0_CE_CMD_OUT(v)            \
                out_dword(HWIO_HOST_SOC_A_WFSS_CE0_CE_CMD_ADDR,v)
#define HWIO_HOST_SOC_A_WFSS_CE0_CE_CMD_OUTM(m,v) \
                out_dword_masked_ns(HWIO_HOST_SOC_A_WFSS_CE0_CE_CMD_ADDR,m,v,HWIO_HOST_SOC_A_WFSS_CE0_CE_CMD_IN)
#define HWIO_HOST_SOC_A_WFSS_CE0_CE_CMD_HALT_STATUS_BMSK                                                          0x8
#define HWIO_HOST_SOC_A_WFSS_CE0_CE_CMD_HALT_STATUS_SHFT                                                            3
#define HWIO_HOST_SOC_A_WFSS_CE0_CE_CMD_DST_FLUSH_BMSK                                                            0x4
#define HWIO_HOST_SOC_A_WFSS_CE0_CE_CMD_DST_FLUSH_SHFT                                                              2
#define HWIO_HOST_SOC_A_WFSS_CE0_CE_CMD_SRC_FLUSH_BMSK                                                            0x2
#define HWIO_HOST_SOC_A_WFSS_CE0_CE_CMD_SRC_FLUSH_SHFT                                                              1
#define HWIO_HOST_SOC_A_WFSS_CE0_CE_CMD_HALT_BMSK                                                                 0x1
#define HWIO_HOST_SOC_A_WFSS_CE0_CE_CMD_HALT_SHFT                                                                   0

#define HWIO_HOST_SOC_A_WFSS_CE0_TARGET_IE_ADDR                                                            (HOST_SOC_A_WFSS_CE0_WFSS_CE_0_REG_REG_BASE      + 0x24)
#define HWIO_HOST_SOC_A_WFSS_CE0_TARGET_IE_RMSK                                                                  0x7f
#define HWIO_HOST_SOC_A_WFSS_CE0_TARGET_IE_IN                    \
                in_dword(HWIO_HOST_SOC_A_WFSS_CE0_TARGET_IE_ADDR)
#define HWIO_HOST_SOC_A_WFSS_CE0_TARGET_IE_INM(m)            \
                in_dword_masked(HWIO_HOST_SOC_A_WFSS_CE0_TARGET_IE_ADDR, m)
#define HWIO_HOST_SOC_A_WFSS_CE0_TARGET_IE_OUT(v)            \
                out_dword(HWIO_HOST_SOC_A_WFSS_CE0_TARGET_IE_ADDR,v)
#define HWIO_HOST_SOC_A_WFSS_CE0_TARGET_IE_OUTM(m,v) \
                out_dword_masked_ns(HWIO_HOST_SOC_A_WFSS_CE0_TARGET_IE_ADDR,m,v,HWIO_HOST_SOC_A_WFSS_CE0_TARGET_IE_IN)
#define HWIO_HOST_SOC_A_WFSS_CE0_TARGET_IE_DST_TIMER_BATCH_BMSK                                                  0x40
#define HWIO_HOST_SOC_A_WFSS_CE0_TARGET_IE_DST_TIMER_BATCH_SHFT                                                     6
#define HWIO_HOST_SOC_A_WFSS_CE0_TARGET_IE_SRC_TIMER_BATCH_BMSK                                                  0x20
#define HWIO_HOST_SOC_A_WFSS_CE0_TARGET_IE_SRC_TIMER_BATCH_SHFT                                                     5
#define HWIO_HOST_SOC_A_WFSS_CE0_TARGET_IE_DST_RING_LOW_WATERMARK_BMSK                                           0x10
#define HWIO_HOST_SOC_A_WFSS_CE0_TARGET_IE_DST_RING_LOW_WATERMARK_SHFT                                              4
#define HWIO_HOST_SOC_A_WFSS_CE0_TARGET_IE_DST_RING_HIGH_WATERMARK_BMSK                                           0x8
#define HWIO_HOST_SOC_A_WFSS_CE0_TARGET_IE_DST_RING_HIGH_WATERMARK_SHFT                                             3
#define HWIO_HOST_SOC_A_WFSS_CE0_TARGET_IE_SRC_RING_LOW_WATERMARK_BMSK                                            0x4
#define HWIO_HOST_SOC_A_WFSS_CE0_TARGET_IE_SRC_RING_LOW_WATERMARK_SHFT                                              2
#define HWIO_HOST_SOC_A_WFSS_CE0_TARGET_IE_SRC_RING_HIGH_WATERMARK_BMSK                                           0x2
#define HWIO_HOST_SOC_A_WFSS_CE0_TARGET_IE_SRC_RING_HIGH_WATERMARK_SHFT                                             1
#define HWIO_HOST_SOC_A_WFSS_CE0_TARGET_IE_COPY_COMPLETE_BMSK                                                     0x1
#define HWIO_HOST_SOC_A_WFSS_CE0_TARGET_IE_COPY_COMPLETE_SHFT                                                       0

#define HWIO_HOST_SOC_A_WFSS_CE0_TARGET_IS_ADDR                                                            (HOST_SOC_A_WFSS_CE0_WFSS_CE_0_REG_REG_BASE      + 0x28)
#define HWIO_HOST_SOC_A_WFSS_CE0_TARGET_IS_RMSK                                                                  0x7f
#define HWIO_HOST_SOC_A_WFSS_CE0_TARGET_IS_IN                    \
                in_dword(HWIO_HOST_SOC_A_WFSS_CE0_TARGET_IS_ADDR)
#define HWIO_HOST_SOC_A_WFSS_CE0_TARGET_IS_INM(m)            \
                in_dword_masked(HWIO_HOST_SOC_A_WFSS_CE0_TARGET_IS_ADDR, m)
#define HWIO_HOST_SOC_A_WFSS_CE0_TARGET_IS_OUT(v)            \
                out_dword(HWIO_HOST_SOC_A_WFSS_CE0_TARGET_IS_ADDR,v)
#define HWIO_HOST_SOC_A_WFSS_CE0_TARGET_IS_OUTM(m,v) \
                out_dword_masked_ns(HWIO_HOST_SOC_A_WFSS_CE0_TARGET_IS_ADDR,m,v,HWIO_HOST_SOC_A_WFSS_CE0_TARGET_IS_IN)
#define HWIO_HOST_SOC_A_WFSS_CE0_TARGET_IS_DST_TIMER_BATCH_BMSK                                                  0x40
#define HWIO_HOST_SOC_A_WFSS_CE0_TARGET_IS_DST_TIMER_BATCH_SHFT                                                     6
#define HWIO_HOST_SOC_A_WFSS_CE0_TARGET_IS_SRC_TIMER_BATCH_BMSK                                                  0x20
#define HWIO_HOST_SOC_A_WFSS_CE0_TARGET_IS_SRC_TIMER_BATCH_SHFT                                                     5
#define HWIO_HOST_SOC_A_WFSS_CE0_TARGET_IS_DST_RING_LOW_WATERMARK_BMSK                                           0x10
#define HWIO_HOST_SOC_A_WFSS_CE0_TARGET_IS_DST_RING_LOW_WATERMARK_SHFT                                              4
#define HWIO_HOST_SOC_A_WFSS_CE0_TARGET_IS_DST_RING_HIGH_WATERMARK_BMSK                                           0x8
#define HWIO_HOST_SOC_A_WFSS_CE0_TARGET_IS_DST_RING_HIGH_WATERMARK_SHFT                                             3
#define HWIO_HOST_SOC_A_WFSS_CE0_TARGET_IS_SRC_RING_LOW_WATERMARK_BMSK                                            0x4
#define HWIO_HOST_SOC_A_WFSS_CE0_TARGET_IS_SRC_RING_LOW_WATERMARK_SHFT                                              2
#define HWIO_HOST_SOC_A_WFSS_CE0_TARGET_IS_SRC_RING_HIGH_WATERMARK_BMSK                                           0x2
#define HWIO_HOST_SOC_A_WFSS_CE0_TARGET_IS_SRC_RING_HIGH_WATERMARK_SHFT                                             1
#define HWIO_HOST_SOC_A_WFSS_CE0_TARGET_IS_COPY_COMPLETE_BMSK                                                     0x1
#define HWIO_HOST_SOC_A_WFSS_CE0_TARGET_IS_COPY_COMPLETE_SHFT                                                       0

#define HWIO_HOST_SOC_A_WFSS_CE0_HOST_IE_ADDR                                                              (HOST_SOC_A_WFSS_CE0_WFSS_CE_0_REG_REG_BASE      + 0x2c)
#define HWIO_HOST_SOC_A_WFSS_CE0_HOST_IE_RMSK                                                                    0x7f
#define HWIO_HOST_SOC_A_WFSS_CE0_HOST_IE_IN                    \
                in_dword(HWIO_HOST_SOC_A_WFSS_CE0_HOST_IE_ADDR)
#define HWIO_HOST_SOC_A_WFSS_CE0_HOST_IE_INM(m)            \
                in_dword_masked(HWIO_HOST_SOC_A_WFSS_CE0_HOST_IE_ADDR, m)
#define HWIO_HOST_SOC_A_WFSS_CE0_HOST_IE_OUT(v)            \
                out_dword(HWIO_HOST_SOC_A_WFSS_CE0_HOST_IE_ADDR,v)
#define HWIO_HOST_SOC_A_WFSS_CE0_HOST_IE_OUTM(m,v) \
                out_dword_masked_ns(HWIO_HOST_SOC_A_WFSS_CE0_HOST_IE_ADDR,m,v,HWIO_HOST_SOC_A_WFSS_CE0_HOST_IE_IN)
#define HWIO_HOST_SOC_A_WFSS_CE0_HOST_IE_DST_TIMER_BATCH_BMSK                                                    0x40
#define HWIO_HOST_SOC_A_WFSS_CE0_HOST_IE_DST_TIMER_BATCH_SHFT                                                       6
#define HWIO_HOST_SOC_A_WFSS_CE0_HOST_IE_SRC_TIMER_BATCH_BMSK                                                    0x20
#define HWIO_HOST_SOC_A_WFSS_CE0_HOST_IE_SRC_TIMER_BATCH_SHFT                                                       5
#define HWIO_HOST_SOC_A_WFSS_CE0_HOST_IE_DST_RING_LOW_WATERMARK_BMSK                                             0x10
#define HWIO_HOST_SOC_A_WFSS_CE0_HOST_IE_DST_RING_LOW_WATERMARK_SHFT                                                4
#define HWIO_HOST_SOC_A_WFSS_CE0_HOST_IE_DST_RING_HIGH_WATERMARK_BMSK                                             0x8
#define HWIO_HOST_SOC_A_WFSS_CE0_HOST_IE_DST_RING_HIGH_WATERMARK_SHFT                                               3
#define HWIO_HOST_SOC_A_WFSS_CE0_HOST_IE_SRC_RING_LOW_WATERMARK_BMSK                                              0x4
#define HWIO_HOST_SOC_A_WFSS_CE0_HOST_IE_SRC_RING_LOW_WATERMARK_SHFT                                                2
#define HWIO_HOST_SOC_A_WFSS_CE0_HOST_IE_SRC_RING_HIGH_WATERMARK_BMSK                                             0x2
#define HWIO_HOST_SOC_A_WFSS_CE0_HOST_IE_SRC_RING_HIGH_WATERMARK_SHFT                                               1
#define HWIO_HOST_SOC_A_WFSS_CE0_HOST_IE_COPY_COMPLETE_BMSK                                                       0x1
#define HWIO_HOST_SOC_A_WFSS_CE0_HOST_IE_COPY_COMPLETE_SHFT                                                         0

#define HWIO_HOST_SOC_A_WFSS_CE0_HOST_IS_ADDR                                                              (HOST_SOC_A_WFSS_CE0_WFSS_CE_0_REG_REG_BASE      + 0x30)
#define HWIO_HOST_SOC_A_WFSS_CE0_HOST_IS_RMSK                                                                    0x7f
#define HWIO_HOST_SOC_A_WFSS_CE0_HOST_IS_IN                    \
                in_dword(HWIO_HOST_SOC_A_WFSS_CE0_HOST_IS_ADDR)
#define HWIO_HOST_SOC_A_WFSS_CE0_HOST_IS_INM(m)            \
                in_dword_masked(HWIO_HOST_SOC_A_WFSS_CE0_HOST_IS_ADDR, m)
#define HWIO_HOST_SOC_A_WFSS_CE0_HOST_IS_OUT(v)            \
                out_dword(HWIO_HOST_SOC_A_WFSS_CE0_HOST_IS_ADDR,v)
#define HWIO_HOST_SOC_A_WFSS_CE0_HOST_IS_OUTM(m,v) \
                out_dword_masked_ns(HWIO_HOST_SOC_A_WFSS_CE0_HOST_IS_ADDR,m,v,HWIO_HOST_SOC_A_WFSS_CE0_HOST_IS_IN)
#define HWIO_HOST_SOC_A_WFSS_CE0_HOST_IS_DST_TIMER_BATCH_BMSK                                                    0x40
#define HWIO_HOST_SOC_A_WFSS_CE0_HOST_IS_DST_TIMER_BATCH_SHFT                                                       6
#define HWIO_HOST_SOC_A_WFSS_CE0_HOST_IS_SRC_TIMER_BATCH_BMSK                                                    0x20
#define HWIO_HOST_SOC_A_WFSS_CE0_HOST_IS_SRC_TIMER_BATCH_SHFT                                                       5
#define HWIO_HOST_SOC_A_WFSS_CE0_HOST_IS_DST_RING_LOW_WATERMARK_BMSK                                             0x10
#define HWIO_HOST_SOC_A_WFSS_CE0_HOST_IS_DST_RING_LOW_WATERMARK_SHFT                                                4
#define HWIO_HOST_SOC_A_WFSS_CE0_HOST_IS_DST_RING_HIGH_WATERMARK_BMSK                                             0x8
#define HWIO_HOST_SOC_A_WFSS_CE0_HOST_IS_DST_RING_HIGH_WATERMARK_SHFT                                               3
#define HWIO_HOST_SOC_A_WFSS_CE0_HOST_IS_SRC_RING_LOW_WATERMARK_BMSK                                              0x4
#define HWIO_HOST_SOC_A_WFSS_CE0_HOST_IS_SRC_RING_LOW_WATERMARK_SHFT                                                2
#define HWIO_HOST_SOC_A_WFSS_CE0_HOST_IS_SRC_RING_HIGH_WATERMARK_BMSK                                             0x2
#define HWIO_HOST_SOC_A_WFSS_CE0_HOST_IS_SRC_RING_HIGH_WATERMARK_SHFT                                               1
#define HWIO_HOST_SOC_A_WFSS_CE0_HOST_IS_COPY_COMPLETE_BMSK                                                       0x1
#define HWIO_HOST_SOC_A_WFSS_CE0_HOST_IS_COPY_COMPLETE_SHFT                                                         0

#define HWIO_HOST_SOC_A_WFSS_CE0_MISC_IE_ADDR                                                              (HOST_SOC_A_WFSS_CE0_WFSS_CE_0_REG_REG_BASE      + 0x34)
#define HWIO_HOST_SOC_A_WFSS_CE0_MISC_IE_RMSK                                                                 0xfffff
#define HWIO_HOST_SOC_A_WFSS_CE0_MISC_IE_IN                    \
                in_dword(HWIO_HOST_SOC_A_WFSS_CE0_MISC_IE_ADDR)
#define HWIO_HOST_SOC_A_WFSS_CE0_MISC_IE_INM(m)            \
                in_dword_masked(HWIO_HOST_SOC_A_WFSS_CE0_MISC_IE_ADDR, m)
#define HWIO_HOST_SOC_A_WFSS_CE0_MISC_IE_OUT(v)            \
                out_dword(HWIO_HOST_SOC_A_WFSS_CE0_MISC_IE_ADDR,v)
#define HWIO_HOST_SOC_A_WFSS_CE0_MISC_IE_OUTM(m,v) \
                out_dword_masked_ns(HWIO_HOST_SOC_A_WFSS_CE0_MISC_IE_ADDR,m,v,HWIO_HOST_SOC_A_WFSS_CE0_MISC_IE_IN)
#define HWIO_HOST_SOC_A_WFSS_CE0_MISC_IE_PARSER_INT_BMSK                                                      0xfc000
#define HWIO_HOST_SOC_A_WFSS_CE0_MISC_IE_PARSER_INT_SHFT                                                           14
#define HWIO_HOST_SOC_A_WFSS_CE0_MISC_IE_REG_INVAL_ADDR_RD_BMSK                                                0x2000
#define HWIO_HOST_SOC_A_WFSS_CE0_MISC_IE_REG_INVAL_ADDR_RD_SHFT                                                    13
#define HWIO_HOST_SOC_A_WFSS_CE0_MISC_IE_REG_INVAL_ADDR_WR_BMSK                                                0x1000
#define HWIO_HOST_SOC_A_WFSS_CE0_MISC_IE_REG_INVAL_ADDR_WR_SHFT                                                    12
#define HWIO_HOST_SOC_A_WFSS_CE0_MISC_IE_REG_RO_WR_BMSK                                                         0x800
#define HWIO_HOST_SOC_A_WFSS_CE0_MISC_IE_REG_RO_WR_SHFT                                                            11
#define HWIO_HOST_SOC_A_WFSS_CE0_MISC_IE_AXI_TIMEOUT_ERR_BMSK                                                   0x400
#define HWIO_HOST_SOC_A_WFSS_CE0_MISC_IE_AXI_TIMEOUT_ERR_SHFT                                                      10
#define HWIO_HOST_SOC_A_WFSS_CE0_MISC_IE_AXI_ERR_BMSK                                                           0x200
#define HWIO_HOST_SOC_A_WFSS_CE0_MISC_IE_AXI_ERR_SHFT                                                               9
#define HWIO_HOST_SOC_A_WFSS_CE0_MISC_IE_SRC_LEN_ERR_BMSK                                                       0x100
#define HWIO_HOST_SOC_A_WFSS_CE0_MISC_IE_SRC_LEN_ERR_SHFT                                                           8
#define HWIO_HOST_SOC_A_WFSS_CE0_MISC_IE_DST_MAX_LEN_VIO_BMSK                                                    0x80
#define HWIO_HOST_SOC_A_WFSS_CE0_MISC_IE_DST_MAX_LEN_VIO_SHFT                                                       7
#define HWIO_HOST_SOC_A_WFSS_CE0_MISC_IE_DST_RING_OVERFLOW_BMSK                                                  0x40
#define HWIO_HOST_SOC_A_WFSS_CE0_MISC_IE_DST_RING_OVERFLOW_SHFT                                                     6
#define HWIO_HOST_SOC_A_WFSS_CE0_MISC_IE_SRC_RING_OVERFLOW_BMSK                                                  0x20
#define HWIO_HOST_SOC_A_WFSS_CE0_MISC_IE_SRC_RING_OVERFLOW_SHFT                                                     5
#define HWIO_HOST_SOC_A_WFSS_CE0_MISC_IE_DST_RING_LOW_WATERMARK_BMSK                                             0x10
#define HWIO_HOST_SOC_A_WFSS_CE0_MISC_IE_DST_RING_LOW_WATERMARK_SHFT                                                4
#define HWIO_HOST_SOC_A_WFSS_CE0_MISC_IE_DST_RING_HIGH_WATERMARK_BMSK                                             0x8
#define HWIO_HOST_SOC_A_WFSS_CE0_MISC_IE_DST_RING_HIGH_WATERMARK_SHFT                                               3
#define HWIO_HOST_SOC_A_WFSS_CE0_MISC_IE_SRC_RING_LOW_WATERMARK_BMSK                                              0x4
#define HWIO_HOST_SOC_A_WFSS_CE0_MISC_IE_SRC_RING_LOW_WATERMARK_SHFT                                                2
#define HWIO_HOST_SOC_A_WFSS_CE0_MISC_IE_SRC_RING_HIGH_WATERMARK_BMSK                                             0x2
#define HWIO_HOST_SOC_A_WFSS_CE0_MISC_IE_SRC_RING_HIGH_WATERMARK_SHFT                                               1
#define HWIO_HOST_SOC_A_WFSS_CE0_MISC_IE_COPY_COMPLETE_BMSK                                                       0x1
#define HWIO_HOST_SOC_A_WFSS_CE0_MISC_IE_COPY_COMPLETE_SHFT                                                         0

#define HWIO_HOST_SOC_A_WFSS_CE0_MISC_IS_ADDR                                                              (HOST_SOC_A_WFSS_CE0_WFSS_CE_0_REG_REG_BASE      + 0x38)
#define HWIO_HOST_SOC_A_WFSS_CE0_MISC_IS_RMSK                                                                 0xfffff
#define HWIO_HOST_SOC_A_WFSS_CE0_MISC_IS_IN                    \
                in_dword(HWIO_HOST_SOC_A_WFSS_CE0_MISC_IS_ADDR)
#define HWIO_HOST_SOC_A_WFSS_CE0_MISC_IS_INM(m)            \
                in_dword_masked(HWIO_HOST_SOC_A_WFSS_CE0_MISC_IS_ADDR, m)
#define HWIO_HOST_SOC_A_WFSS_CE0_MISC_IS_OUT(v)            \
                out_dword(HWIO_HOST_SOC_A_WFSS_CE0_MISC_IS_ADDR,v)
#define HWIO_HOST_SOC_A_WFSS_CE0_MISC_IS_OUTM(m,v) \
                out_dword_masked_ns(HWIO_HOST_SOC_A_WFSS_CE0_MISC_IS_ADDR,m,v,HWIO_HOST_SOC_A_WFSS_CE0_MISC_IS_IN)
#define HWIO_HOST_SOC_A_WFSS_CE0_MISC_IS_PARSER_INT_BMSK                                                      0xfc000
#define HWIO_HOST_SOC_A_WFSS_CE0_MISC_IS_PARSER_INT_SHFT                                                           14
#define HWIO_HOST_SOC_A_WFSS_CE0_MISC_IS_REG_INVAL_ADDR_RD_BMSK                                                0x2000
#define HWIO_HOST_SOC_A_WFSS_CE0_MISC_IS_REG_INVAL_ADDR_RD_SHFT                                                    13
#define HWIO_HOST_SOC_A_WFSS_CE0_MISC_IS_REG_INVAL_ADDR_WR_BMSK                                                0x1000
#define HWIO_HOST_SOC_A_WFSS_CE0_MISC_IS_REG_INVAL_ADDR_WR_SHFT                                                    12
#define HWIO_HOST_SOC_A_WFSS_CE0_MISC_IS_REG_INVAL_RO_WR_BMSK                                                   0x800
#define HWIO_HOST_SOC_A_WFSS_CE0_MISC_IS_REG_INVAL_RO_WR_SHFT                                                      11
#define HWIO_HOST_SOC_A_WFSS_CE0_MISC_IS_AXI_TIMEOUT_ERR_BMSK                                                   0x400
#define HWIO_HOST_SOC_A_WFSS_CE0_MISC_IS_AXI_TIMEOUT_ERR_SHFT                                                      10
#define HWIO_HOST_SOC_A_WFSS_CE0_MISC_IS_AXI_BUS_ERR_BMSK                                                       0x200
#define HWIO_HOST_SOC_A_WFSS_CE0_MISC_IS_AXI_BUS_ERR_SHFT                                                           9
#define HWIO_HOST_SOC_A_WFSS_CE0_MISC_IS_SRC_LEN_ERR_BMSK                                                       0x100
#define HWIO_HOST_SOC_A_WFSS_CE0_MISC_IS_SRC_LEN_ERR_SHFT                                                           8
#define HWIO_HOST_SOC_A_WFSS_CE0_MISC_IS_DST_MAX_LEN_VIO_BMSK                                                    0x80
#define HWIO_HOST_SOC_A_WFSS_CE0_MISC_IS_DST_MAX_LEN_VIO_SHFT                                                       7
#define HWIO_HOST_SOC_A_WFSS_CE0_MISC_IS_DST_RING_OVERFLOW_BMSK                                                  0x40
#define HWIO_HOST_SOC_A_WFSS_CE0_MISC_IS_DST_RING_OVERFLOW_SHFT                                                     6
#define HWIO_HOST_SOC_A_WFSS_CE0_MISC_IS_SRC_RING_OVERFLOW_BMSK                                                  0x20
#define HWIO_HOST_SOC_A_WFSS_CE0_MISC_IS_SRC_RING_OVERFLOW_SHFT                                                     5
#define HWIO_HOST_SOC_A_WFSS_CE0_MISC_IS_DST_RING_LOW_WATERMARK_BMSK                                             0x10
#define HWIO_HOST_SOC_A_WFSS_CE0_MISC_IS_DST_RING_LOW_WATERMARK_SHFT                                                4
#define HWIO_HOST_SOC_A_WFSS_CE0_MISC_IS_DST_RING_HIGH_WATERMARK_BMSK                                             0x8
#define HWIO_HOST_SOC_A_WFSS_CE0_MISC_IS_DST_RING_HIGH_WATERMARK_SHFT                                               3
#define HWIO_HOST_SOC_A_WFSS_CE0_MISC_IS_SRC_RING_LOW_WATERMARK_BMSK                                              0x4
#define HWIO_HOST_SOC_A_WFSS_CE0_MISC_IS_SRC_RING_LOW_WATERMARK_SHFT                                                2
#define HWIO_HOST_SOC_A_WFSS_CE0_MISC_IS_SRC_RING_HIGH_WATERMARK_BMSK                                             0x2
#define HWIO_HOST_SOC_A_WFSS_CE0_MISC_IS_SRC_RING_HIGH_WATERMARK_SHFT                                               1
#define HWIO_HOST_SOC_A_WFSS_CE0_MISC_IS_COPY_COMPLETE_BMSK                                                       0x1
#define HWIO_HOST_SOC_A_WFSS_CE0_MISC_IS_COPY_COMPLETE_SHFT                                                         0

#define HWIO_HOST_SOC_A_WFSS_CE0_SRC_WR_INDEX_ADDR                                                         (HOST_SOC_A_WFSS_CE0_WFSS_CE_0_REG_REG_BASE      + 0x3c)
#define HWIO_HOST_SOC_A_WFSS_CE0_SRC_WR_INDEX_RMSK                                                             0xffff
#define HWIO_HOST_SOC_A_WFSS_CE0_SRC_WR_INDEX_IN                    \
                in_dword(HWIO_HOST_SOC_A_WFSS_CE0_SRC_WR_INDEX_ADDR)
#define HWIO_HOST_SOC_A_WFSS_CE0_SRC_WR_INDEX_INM(m)            \
                in_dword_masked(HWIO_HOST_SOC_A_WFSS_CE0_SRC_WR_INDEX_ADDR, m)
#define HWIO_HOST_SOC_A_WFSS_CE0_SRC_WR_INDEX_OUT(v)            \
                out_dword(HWIO_HOST_SOC_A_WFSS_CE0_SRC_WR_INDEX_ADDR,v)
#define HWIO_HOST_SOC_A_WFSS_CE0_SRC_WR_INDEX_OUTM(m,v) \
                out_dword_masked_ns(HWIO_HOST_SOC_A_WFSS_CE0_SRC_WR_INDEX_ADDR,m,v,HWIO_HOST_SOC_A_WFSS_CE0_SRC_WR_INDEX_IN)
#define HWIO_HOST_SOC_A_WFSS_CE0_SRC_WR_INDEX_SRC_WR_INDEX_BMSK                                                0xffff
#define HWIO_HOST_SOC_A_WFSS_CE0_SRC_WR_INDEX_SRC_WR_INDEX_SHFT                                                     0

#define HWIO_HOST_SOC_A_WFSS_CE0_DST_WR_INDEX_ADDR                                                         (HOST_SOC_A_WFSS_CE0_WFSS_CE_0_REG_REG_BASE      + 0x40)
#define HWIO_HOST_SOC_A_WFSS_CE0_DST_WR_INDEX_RMSK                                                             0xffff
#define HWIO_HOST_SOC_A_WFSS_CE0_DST_WR_INDEX_IN                    \
                in_dword(HWIO_HOST_SOC_A_WFSS_CE0_DST_WR_INDEX_ADDR)
#define HWIO_HOST_SOC_A_WFSS_CE0_DST_WR_INDEX_INM(m)            \
                in_dword_masked(HWIO_HOST_SOC_A_WFSS_CE0_DST_WR_INDEX_ADDR, m)
#define HWIO_HOST_SOC_A_WFSS_CE0_DST_WR_INDEX_OUT(v)            \
                out_dword(HWIO_HOST_SOC_A_WFSS_CE0_DST_WR_INDEX_ADDR,v)
#define HWIO_HOST_SOC_A_WFSS_CE0_DST_WR_INDEX_OUTM(m,v) \
                out_dword_masked_ns(HWIO_HOST_SOC_A_WFSS_CE0_DST_WR_INDEX_ADDR,m,v,HWIO_HOST_SOC_A_WFSS_CE0_DST_WR_INDEX_IN)
#define HWIO_HOST_SOC_A_WFSS_CE0_DST_WR_INDEX_DST_WR_INDEX_BMSK                                                0xffff
#define HWIO_HOST_SOC_A_WFSS_CE0_DST_WR_INDEX_DST_WR_INDEX_SHFT                                                     0

#define HWIO_HOST_SOC_A_WFSS_CE0_CURRENT_SRRI_ADDR                                                         (HOST_SOC_A_WFSS_CE0_WFSS_CE_0_REG_REG_BASE      + 0x44)
#define HWIO_HOST_SOC_A_WFSS_CE0_CURRENT_SRRI_RMSK                                                             0xffff
#define HWIO_HOST_SOC_A_WFSS_CE0_CURRENT_SRRI_IN                    \
                in_dword(HWIO_HOST_SOC_A_WFSS_CE0_CURRENT_SRRI_ADDR)
#define HWIO_HOST_SOC_A_WFSS_CE0_CURRENT_SRRI_INM(m)            \
                in_dword_masked(HWIO_HOST_SOC_A_WFSS_CE0_CURRENT_SRRI_ADDR, m)
#define HWIO_HOST_SOC_A_WFSS_CE0_CURRENT_SRRI_CURRENT_SRRI_BMSK                                                0xffff
#define HWIO_HOST_SOC_A_WFSS_CE0_CURRENT_SRRI_CURRENT_SRRI_SHFT                                                     0

#define HWIO_HOST_SOC_A_WFSS_CE0_CURRENT_DRRI_ADDR                                                         (HOST_SOC_A_WFSS_CE0_WFSS_CE_0_REG_REG_BASE      + 0x48)
#define HWIO_HOST_SOC_A_WFSS_CE0_CURRENT_DRRI_RMSK                                                             0xffff
#define HWIO_HOST_SOC_A_WFSS_CE0_CURRENT_DRRI_IN                    \
                in_dword(HWIO_HOST_SOC_A_WFSS_CE0_CURRENT_DRRI_ADDR)
#define HWIO_HOST_SOC_A_WFSS_CE0_CURRENT_DRRI_INM(m)            \
                in_dword_masked(HWIO_HOST_SOC_A_WFSS_CE0_CURRENT_DRRI_ADDR, m)
#define HWIO_HOST_SOC_A_WFSS_CE0_CURRENT_DRRI_CURRENT_DRRI_BMSK                                                0xffff
#define HWIO_HOST_SOC_A_WFSS_CE0_CURRENT_DRRI_CURRENT_DRRI_SHFT                                                     0

#define HWIO_HOST_SOC_A_WFSS_CE0_SRC_WATERMARK_ADDR                                                        (HOST_SOC_A_WFSS_CE0_WFSS_CE_0_REG_REG_BASE      + 0x4c)
#define HWIO_HOST_SOC_A_WFSS_CE0_SRC_WATERMARK_RMSK                                                        0xffffffff
#define HWIO_HOST_SOC_A_WFSS_CE0_SRC_WATERMARK_IN                    \
                in_dword(HWIO_HOST_SOC_A_WFSS_CE0_SRC_WATERMARK_ADDR)
#define HWIO_HOST_SOC_A_WFSS_CE0_SRC_WATERMARK_INM(m)            \
                in_dword_masked(HWIO_HOST_SOC_A_WFSS_CE0_SRC_WATERMARK_ADDR, m)
#define HWIO_HOST_SOC_A_WFSS_CE0_SRC_WATERMARK_OUT(v)            \
                out_dword(HWIO_HOST_SOC_A_WFSS_CE0_SRC_WATERMARK_ADDR,v)
#define HWIO_HOST_SOC_A_WFSS_CE0_SRC_WATERMARK_OUTM(m,v) \
                out_dword_masked_ns(HWIO_HOST_SOC_A_WFSS_CE0_SRC_WATERMARK_ADDR,m,v,HWIO_HOST_SOC_A_WFSS_CE0_SRC_WATERMARK_IN)
#define HWIO_HOST_SOC_A_WFSS_CE0_SRC_WATERMARK_SR_LOW_WATER_MARK_THRESOLD_BMSK                             0xffff0000
#define HWIO_HOST_SOC_A_WFSS_CE0_SRC_WATERMARK_SR_LOW_WATER_MARK_THRESOLD_SHFT                                     16
#define HWIO_HOST_SOC_A_WFSS_CE0_SRC_WATERMARK_SR_HIGH_WATER_MARK_THRESHOLD_BMSK                               0xffff
#define HWIO_HOST_SOC_A_WFSS_CE0_SRC_WATERMARK_SR_HIGH_WATER_MARK_THRESHOLD_SHFT                                    0

#define HWIO_HOST_SOC_A_WFSS_CE0_DST_WATERMARK_ADDR                                                        (HOST_SOC_A_WFSS_CE0_WFSS_CE_0_REG_REG_BASE      + 0x50)
#define HWIO_HOST_SOC_A_WFSS_CE0_DST_WATERMARK_RMSK                                                        0xffffffff
#define HWIO_HOST_SOC_A_WFSS_CE0_DST_WATERMARK_IN                    \
                in_dword(HWIO_HOST_SOC_A_WFSS_CE0_DST_WATERMARK_ADDR)
#define HWIO_HOST_SOC_A_WFSS_CE0_DST_WATERMARK_INM(m)            \
                in_dword_masked(HWIO_HOST_SOC_A_WFSS_CE0_DST_WATERMARK_ADDR, m)
#define HWIO_HOST_SOC_A_WFSS_CE0_DST_WATERMARK_OUT(v)            \
                out_dword(HWIO_HOST_SOC_A_WFSS_CE0_DST_WATERMARK_ADDR,v)
#define HWIO_HOST_SOC_A_WFSS_CE0_DST_WATERMARK_OUTM(m,v) \
                out_dword_masked_ns(HWIO_HOST_SOC_A_WFSS_CE0_DST_WATERMARK_ADDR,m,v,HWIO_HOST_SOC_A_WFSS_CE0_DST_WATERMARK_IN)
#define HWIO_HOST_SOC_A_WFSS_CE0_DST_WATERMARK_DR_LOW_WATER_MARK_THRESHOLD_BMSK                            0xffff0000
#define HWIO_HOST_SOC_A_WFSS_CE0_DST_WATERMARK_DR_LOW_WATER_MARK_THRESHOLD_SHFT                                    16
#define HWIO_HOST_SOC_A_WFSS_CE0_DST_WATERMARK_DR_HIGH_WATER_MARK_THRESHOLD_BMSK                               0xffff
#define HWIO_HOST_SOC_A_WFSS_CE0_DST_WATERMARK_DR_HIGH_WATER_MARK_THRESHOLD_SHFT                                    0

#define HWIO_HOST_SOC_A_WFSS_CE0_CE_CHANNEL_PRIORITY_ADDR                                                  (HOST_SOC_A_WFSS_CE0_WFSS_CE_0_REG_REG_BASE      + 0x54)
#define HWIO_HOST_SOC_A_WFSS_CE0_CE_CHANNEL_PRIORITY_RMSK                                                         0x7
#define HWIO_HOST_SOC_A_WFSS_CE0_CE_CHANNEL_PRIORITY_IN                    \
                in_dword(HWIO_HOST_SOC_A_WFSS_CE0_CE_CHANNEL_PRIORITY_ADDR)
#define HWIO_HOST_SOC_A_WFSS_CE0_CE_CHANNEL_PRIORITY_INM(m)            \
                in_dword_masked(HWIO_HOST_SOC_A_WFSS_CE0_CE_CHANNEL_PRIORITY_ADDR, m)
#define HWIO_HOST_SOC_A_WFSS_CE0_CE_CHANNEL_PRIORITY_OUT(v)            \
                out_dword(HWIO_HOST_SOC_A_WFSS_CE0_CE_CHANNEL_PRIORITY_ADDR,v)
#define HWIO_HOST_SOC_A_WFSS_CE0_CE_CHANNEL_PRIORITY_OUTM(m,v) \
                out_dword_masked_ns(HWIO_HOST_SOC_A_WFSS_CE0_CE_CHANNEL_PRIORITY_ADDR,m,v,HWIO_HOST_SOC_A_WFSS_CE0_CE_CHANNEL_PRIORITY_IN)
#define HWIO_HOST_SOC_A_WFSS_CE0_CE_CHANNEL_PRIORITY_CE_DST_OVERFLOW_MASK_BMSK                                    0x4
#define HWIO_HOST_SOC_A_WFSS_CE0_CE_CHANNEL_PRIORITY_CE_DST_OVERFLOW_MASK_SHFT                                      2
#define HWIO_HOST_SOC_A_WFSS_CE0_CE_CHANNEL_PRIORITY_CE_SRC_OVERFLOW_MASK_BMSK                                    0x2
#define HWIO_HOST_SOC_A_WFSS_CE0_CE_CHANNEL_PRIORITY_CE_SRC_OVERFLOW_MASK_SHFT                                      1
#define HWIO_HOST_SOC_A_WFSS_CE0_CE_CHANNEL_PRIORITY_CE_PRIORITY_BMSK                                             0x1
#define HWIO_HOST_SOC_A_WFSS_CE0_CE_CHANNEL_PRIORITY_CE_PRIORITY_SHFT                                               0

#define HWIO_HOST_SOC_A_WFSS_CE0_HOST_MSI_ADDR_LOW_ADDR                                                    (HOST_SOC_A_WFSS_CE0_WFSS_CE_0_REG_REG_BASE      + 0x58)
#define HWIO_HOST_SOC_A_WFSS_CE0_HOST_MSI_ADDR_LOW_RMSK                                                    0xffffffff
#define HWIO_HOST_SOC_A_WFSS_CE0_HOST_MSI_ADDR_LOW_IN                    \
                in_dword(HWIO_HOST_SOC_A_WFSS_CE0_HOST_MSI_ADDR_LOW_ADDR)
#define HWIO_HOST_SOC_A_WFSS_CE0_HOST_MSI_ADDR_LOW_INM(m)            \
                in_dword_masked(HWIO_HOST_SOC_A_WFSS_CE0_HOST_MSI_ADDR_LOW_ADDR, m)
#define HWIO_HOST_SOC_A_WFSS_CE0_HOST_MSI_ADDR_LOW_OUT(v)            \
                out_dword(HWIO_HOST_SOC_A_WFSS_CE0_HOST_MSI_ADDR_LOW_ADDR,v)
#define HWIO_HOST_SOC_A_WFSS_CE0_HOST_MSI_ADDR_LOW_OUTM(m,v) \
                out_dword_masked_ns(HWIO_HOST_SOC_A_WFSS_CE0_HOST_MSI_ADDR_LOW_ADDR,m,v,HWIO_HOST_SOC_A_WFSS_CE0_HOST_MSI_ADDR_LOW_IN)
#define HWIO_HOST_SOC_A_WFSS_CE0_HOST_MSI_ADDR_LOW_MSI_ADDR_LOW_BMSK                                       0xffffffff
#define HWIO_HOST_SOC_A_WFSS_CE0_HOST_MSI_ADDR_LOW_MSI_ADDR_LOW_SHFT                                                0

#define HWIO_HOST_SOC_A_WFSS_CE0_HOST_MSI_ADDR_HIGH_ADDR                                                   (HOST_SOC_A_WFSS_CE0_WFSS_CE_0_REG_REG_BASE      + 0x5c)
#define HWIO_HOST_SOC_A_WFSS_CE0_HOST_MSI_ADDR_HIGH_RMSK                                                         0xff
#define HWIO_HOST_SOC_A_WFSS_CE0_HOST_MSI_ADDR_HIGH_IN                    \
                in_dword(HWIO_HOST_SOC_A_WFSS_CE0_HOST_MSI_ADDR_HIGH_ADDR)
#define HWIO_HOST_SOC_A_WFSS_CE0_HOST_MSI_ADDR_HIGH_INM(m)            \
                in_dword_masked(HWIO_HOST_SOC_A_WFSS_CE0_HOST_MSI_ADDR_HIGH_ADDR, m)
#define HWIO_HOST_SOC_A_WFSS_CE0_HOST_MSI_ADDR_HIGH_OUT(v)            \
                out_dword(HWIO_HOST_SOC_A_WFSS_CE0_HOST_MSI_ADDR_HIGH_ADDR,v)
#define HWIO_HOST_SOC_A_WFSS_CE0_HOST_MSI_ADDR_HIGH_OUTM(m,v) \
                out_dword_masked_ns(HWIO_HOST_SOC_A_WFSS_CE0_HOST_MSI_ADDR_HIGH_ADDR,m,v,HWIO_HOST_SOC_A_WFSS_CE0_HOST_MSI_ADDR_HIGH_IN)
#define HWIO_HOST_SOC_A_WFSS_CE0_HOST_MSI_ADDR_HIGH_MSI_ADDR_HIGH_BMSK                                           0xff
#define HWIO_HOST_SOC_A_WFSS_CE0_HOST_MSI_ADDR_HIGH_MSI_ADDR_HIGH_SHFT                                              0

#define HWIO_HOST_SOC_A_WFSS_CE0_HOST_MSI_DATA_ADDR                                                        (HOST_SOC_A_WFSS_CE0_WFSS_CE_0_REG_REG_BASE      + 0x60)
#define HWIO_HOST_SOC_A_WFSS_CE0_HOST_MSI_DATA_RMSK                                                        0xffffffff
#define HWIO_HOST_SOC_A_WFSS_CE0_HOST_MSI_DATA_IN                    \
                in_dword(HWIO_HOST_SOC_A_WFSS_CE0_HOST_MSI_DATA_ADDR)
#define HWIO_HOST_SOC_A_WFSS_CE0_HOST_MSI_DATA_INM(m)            \
                in_dword_masked(HWIO_HOST_SOC_A_WFSS_CE0_HOST_MSI_DATA_ADDR, m)
#define HWIO_HOST_SOC_A_WFSS_CE0_HOST_MSI_DATA_OUT(v)            \
                out_dword(HWIO_HOST_SOC_A_WFSS_CE0_HOST_MSI_DATA_ADDR,v)
#define HWIO_HOST_SOC_A_WFSS_CE0_HOST_MSI_DATA_OUTM(m,v) \
                out_dword_masked_ns(HWIO_HOST_SOC_A_WFSS_CE0_HOST_MSI_DATA_ADDR,m,v,HWIO_HOST_SOC_A_WFSS_CE0_HOST_MSI_DATA_IN)
#define HWIO_HOST_SOC_A_WFSS_CE0_HOST_MSI_DATA_MSI_DATA_BMSK                                               0xffffffff
#define HWIO_HOST_SOC_A_WFSS_CE0_HOST_MSI_DATA_MSI_DATA_SHFT                                                        0

#define HWIO_HOST_SOC_A_WFSS_CE0_CE_OBFF_TIMEOUT_ADDR                                                      (HOST_SOC_A_WFSS_CE0_WFSS_CE_0_REG_REG_BASE      + 0x64)
#define HWIO_HOST_SOC_A_WFSS_CE0_CE_OBFF_TIMEOUT_RMSK                                                          0xffff
#define HWIO_HOST_SOC_A_WFSS_CE0_CE_OBFF_TIMEOUT_IN                    \
                in_dword(HWIO_HOST_SOC_A_WFSS_CE0_CE_OBFF_TIMEOUT_ADDR)
#define HWIO_HOST_SOC_A_WFSS_CE0_CE_OBFF_TIMEOUT_INM(m)            \
                in_dword_masked(HWIO_HOST_SOC_A_WFSS_CE0_CE_OBFF_TIMEOUT_ADDR, m)
#define HWIO_HOST_SOC_A_WFSS_CE0_CE_OBFF_TIMEOUT_OUT(v)            \
                out_dword(HWIO_HOST_SOC_A_WFSS_CE0_CE_OBFF_TIMEOUT_ADDR,v)
#define HWIO_HOST_SOC_A_WFSS_CE0_CE_OBFF_TIMEOUT_OUTM(m,v) \
                out_dword_masked_ns(HWIO_HOST_SOC_A_WFSS_CE0_CE_OBFF_TIMEOUT_ADDR,m,v,HWIO_HOST_SOC_A_WFSS_CE0_CE_OBFF_TIMEOUT_IN)
#define HWIO_HOST_SOC_A_WFSS_CE0_CE_OBFF_TIMEOUT_OBFF_TOLERANCE_TIMEOUT_BMSK                                   0xffff
#define HWIO_HOST_SOC_A_WFSS_CE0_CE_OBFF_TIMEOUT_OBFF_TOLERANCE_TIMEOUT_SHFT                                        0

#define HWIO_HOST_SOC_A_WFSS_CE0_CE_CHANNEL_OBFF_CFG_ADDR                                                  (HOST_SOC_A_WFSS_CE0_WFSS_CE_0_REG_REG_BASE      + 0x68)
#define HWIO_HOST_SOC_A_WFSS_CE0_CE_CHANNEL_OBFF_CFG_RMSK                                                         0x3
#define HWIO_HOST_SOC_A_WFSS_CE0_CE_CHANNEL_OBFF_CFG_IN                    \
                in_dword(HWIO_HOST_SOC_A_WFSS_CE0_CE_CHANNEL_OBFF_CFG_ADDR)
#define HWIO_HOST_SOC_A_WFSS_CE0_CE_CHANNEL_OBFF_CFG_INM(m)            \
                in_dword_masked(HWIO_HOST_SOC_A_WFSS_CE0_CE_CHANNEL_OBFF_CFG_ADDR, m)
#define HWIO_HOST_SOC_A_WFSS_CE0_CE_CHANNEL_OBFF_CFG_OUT(v)            \
                out_dword(HWIO_HOST_SOC_A_WFSS_CE0_CE_CHANNEL_OBFF_CFG_ADDR,v)
#define HWIO_HOST_SOC_A_WFSS_CE0_CE_CHANNEL_OBFF_CFG_OUTM(m,v) \
                out_dword_masked_ns(HWIO_HOST_SOC_A_WFSS_CE0_CE_CHANNEL_OBFF_CFG_ADDR,m,v,HWIO_HOST_SOC_A_WFSS_CE0_CE_CHANNEL_OBFF_CFG_IN)
#define HWIO_HOST_SOC_A_WFSS_CE0_CE_CHANNEL_OBFF_CFG_OBFF_CE_FLUSH_REQ_BMSK                                       0x2
#define HWIO_HOST_SOC_A_WFSS_CE0_CE_CHANNEL_OBFF_CFG_OBFF_CE_FLUSH_REQ_SHFT                                         1
#define HWIO_HOST_SOC_A_WFSS_CE0_CE_CHANNEL_OBFF_CFG_OBFF_CE_AUTO_PRIORITY_DISABLE_BMSK                           0x1
#define HWIO_HOST_SOC_A_WFSS_CE0_CE_CHANNEL_OBFF_CFG_OBFF_CE_AUTO_PRIORITY_DISABLE_SHFT                             0

#define HWIO_HOST_SOC_A_WFSS_CE0_CE_WATCHDOG_ADDR                                                          (HOST_SOC_A_WFSS_CE0_WFSS_CE_0_REG_REG_BASE      + 0x6c)
#define HWIO_HOST_SOC_A_WFSS_CE0_CE_WATCHDOG_RMSK                                                          0xffffffff
#define HWIO_HOST_SOC_A_WFSS_CE0_CE_WATCHDOG_IN                    \
                in_dword(HWIO_HOST_SOC_A_WFSS_CE0_CE_WATCHDOG_ADDR)
#define HWIO_HOST_SOC_A_WFSS_CE0_CE_WATCHDOG_INM(m)            \
                in_dword_masked(HWIO_HOST_SOC_A_WFSS_CE0_CE_WATCHDOG_ADDR, m)
#define HWIO_HOST_SOC_A_WFSS_CE0_CE_WATCHDOG_OUT(v)            \
                out_dword(HWIO_HOST_SOC_A_WFSS_CE0_CE_WATCHDOG_ADDR,v)
#define HWIO_HOST_SOC_A_WFSS_CE0_CE_WATCHDOG_OUTM(m,v) \
                out_dword_masked_ns(HWIO_HOST_SOC_A_WFSS_CE0_CE_WATCHDOG_ADDR,m,v,HWIO_HOST_SOC_A_WFSS_CE0_CE_WATCHDOG_IN)
#define HWIO_HOST_SOC_A_WFSS_CE0_CE_WATCHDOG_STATUS_BMSK                                                   0xffff0000
#define HWIO_HOST_SOC_A_WFSS_CE0_CE_WATCHDOG_STATUS_SHFT                                                           16
#define HWIO_HOST_SOC_A_WFSS_CE0_CE_WATCHDOG_LIMIT_BMSK                                                        0xffff
#define HWIO_HOST_SOC_A_WFSS_CE0_CE_WATCHDOG_LIMIT_SHFT                                                             0

#define HWIO_HOST_SOC_A_WFSS_CE0_CE_CSM_DBG_ADDR                                                           (HOST_SOC_A_WFSS_CE0_WFSS_CE_0_REG_REG_BASE      + 0x70)
#define HWIO_HOST_SOC_A_WFSS_CE0_CE_CSM_DBG_RMSK                                                           0xffffffff
#define HWIO_HOST_SOC_A_WFSS_CE0_CE_CSM_DBG_IN                    \
                in_dword(HWIO_HOST_SOC_A_WFSS_CE0_CE_CSM_DBG_ADDR)
#define HWIO_HOST_SOC_A_WFSS_CE0_CE_CSM_DBG_INM(m)            \
                in_dword_masked(HWIO_HOST_SOC_A_WFSS_CE0_CE_CSM_DBG_ADDR, m)
#define HWIO_HOST_SOC_A_WFSS_CE0_CE_CSM_DBG_VALUE_BMSK                                                     0xffffffff
#define HWIO_HOST_SOC_A_WFSS_CE0_CE_CSM_DBG_VALUE_SHFT                                                              0

#define HWIO_HOST_SOC_A_WFSS_CE0_CE_SRC_BATCH_TIMER_INT_SETUP_ADDR                                         (HOST_SOC_A_WFSS_CE0_WFSS_CE_0_REG_REG_BASE      + 0x74)
#define HWIO_HOST_SOC_A_WFSS_CE0_CE_SRC_BATCH_TIMER_INT_SETUP_RMSK                                         0xffffffff
#define HWIO_HOST_SOC_A_WFSS_CE0_CE_SRC_BATCH_TIMER_INT_SETUP_IN                    \
                in_dword(HWIO_HOST_SOC_A_WFSS_CE0_CE_SRC_BATCH_TIMER_INT_SETUP_ADDR)
#define HWIO_HOST_SOC_A_WFSS_CE0_CE_SRC_BATCH_TIMER_INT_SETUP_INM(m)            \
                in_dword_masked(HWIO_HOST_SOC_A_WFSS_CE0_CE_SRC_BATCH_TIMER_INT_SETUP_ADDR, m)
#define HWIO_HOST_SOC_A_WFSS_CE0_CE_SRC_BATCH_TIMER_INT_SETUP_OUT(v)            \
                out_dword(HWIO_HOST_SOC_A_WFSS_CE0_CE_SRC_BATCH_TIMER_INT_SETUP_ADDR,v)
#define HWIO_HOST_SOC_A_WFSS_CE0_CE_SRC_BATCH_TIMER_INT_SETUP_OUTM(m,v) \
                out_dword_masked_ns(HWIO_HOST_SOC_A_WFSS_CE0_CE_SRC_BATCH_TIMER_INT_SETUP_ADDR,m,v,HWIO_HOST_SOC_A_WFSS_CE0_CE_SRC_BATCH_TIMER_INT_SETUP_IN)
#define HWIO_HOST_SOC_A_WFSS_CE0_CE_SRC_BATCH_TIMER_INT_SETUP_INTERRUPT_TIMER_THRESHOLD_BMSK               0xffff0000
#define HWIO_HOST_SOC_A_WFSS_CE0_CE_SRC_BATCH_TIMER_INT_SETUP_INTERRUPT_TIMER_THRESHOLD_SHFT                       16
#define HWIO_HOST_SOC_A_WFSS_CE0_CE_SRC_BATCH_TIMER_INT_SETUP_SW_INTERRUPT_MODE_BMSK                           0x8000
#define HWIO_HOST_SOC_A_WFSS_CE0_CE_SRC_BATCH_TIMER_INT_SETUP_SW_INTERRUPT_MODE_SHFT                               15
#define HWIO_HOST_SOC_A_WFSS_CE0_CE_SRC_BATCH_TIMER_INT_SETUP_BATCH_COUNTER_THRESHOLD_BMSK                     0x7fff
#define HWIO_HOST_SOC_A_WFSS_CE0_CE_SRC_BATCH_TIMER_INT_SETUP_BATCH_COUNTER_THRESHOLD_SHFT                          0

#define HWIO_HOST_SOC_A_WFSS_CE0_CE_SRC_BATCH_TIMER_INT_STATUS_ADDR                                        (HOST_SOC_A_WFSS_CE0_WFSS_CE_0_REG_REG_BASE      + 0x78)
#define HWIO_HOST_SOC_A_WFSS_CE0_CE_SRC_BATCH_TIMER_INT_STATUS_RMSK                                        0xffffffff
#define HWIO_HOST_SOC_A_WFSS_CE0_CE_SRC_BATCH_TIMER_INT_STATUS_IN                    \
                in_dword(HWIO_HOST_SOC_A_WFSS_CE0_CE_SRC_BATCH_TIMER_INT_STATUS_ADDR)
#define HWIO_HOST_SOC_A_WFSS_CE0_CE_SRC_BATCH_TIMER_INT_STATUS_INM(m)            \
                in_dword_masked(HWIO_HOST_SOC_A_WFSS_CE0_CE_SRC_BATCH_TIMER_INT_STATUS_ADDR, m)
#define HWIO_HOST_SOC_A_WFSS_CE0_CE_SRC_BATCH_TIMER_INT_STATUS_CURRENT_INTERRUPT_TIMER_VALUE_BMSK          0xffff0000
#define HWIO_HOST_SOC_A_WFSS_CE0_CE_SRC_BATCH_TIMER_INT_STATUS_CURRENT_INTERRUPT_TIMER_VALUE_SHFT                  16
#define HWIO_HOST_SOC_A_WFSS_CE0_CE_SRC_BATCH_TIMER_INT_STATUS_CURRENT_SW_INT_WIRE_VALUE_BMSK                  0x8000
#define HWIO_HOST_SOC_A_WFSS_CE0_CE_SRC_BATCH_TIMER_INT_STATUS_CURRENT_SW_INT_WIRE_VALUE_SHFT                      15
#define HWIO_HOST_SOC_A_WFSS_CE0_CE_SRC_BATCH_TIMER_INT_STATUS_INTERNAL_BATCH_COUNTER_VALUE_BMSK               0x7fff
#define HWIO_HOST_SOC_A_WFSS_CE0_CE_SRC_BATCH_TIMER_INT_STATUS_INTERNAL_BATCH_COUNTER_VALUE_SHFT                    0

#define HWIO_HOST_SOC_A_WFSS_CE0_CE_DST_BATCH_TIMER_INT_SETUP_ADDR                                         (HOST_SOC_A_WFSS_CE0_WFSS_CE_0_REG_REG_BASE      + 0x7c)
#define HWIO_HOST_SOC_A_WFSS_CE0_CE_DST_BATCH_TIMER_INT_SETUP_RMSK                                         0xffffffff
#define HWIO_HOST_SOC_A_WFSS_CE0_CE_DST_BATCH_TIMER_INT_SETUP_IN                    \
                in_dword(HWIO_HOST_SOC_A_WFSS_CE0_CE_DST_BATCH_TIMER_INT_SETUP_ADDR)
#define HWIO_HOST_SOC_A_WFSS_CE0_CE_DST_BATCH_TIMER_INT_SETUP_INM(m)            \
                in_dword_masked(HWIO_HOST_SOC_A_WFSS_CE0_CE_DST_BATCH_TIMER_INT_SETUP_ADDR, m)
#define HWIO_HOST_SOC_A_WFSS_CE0_CE_DST_BATCH_TIMER_INT_SETUP_OUT(v)            \
                out_dword(HWIO_HOST_SOC_A_WFSS_CE0_CE_DST_BATCH_TIMER_INT_SETUP_ADDR,v)
#define HWIO_HOST_SOC_A_WFSS_CE0_CE_DST_BATCH_TIMER_INT_SETUP_OUTM(m,v) \
                out_dword_masked_ns(HWIO_HOST_SOC_A_WFSS_CE0_CE_DST_BATCH_TIMER_INT_SETUP_ADDR,m,v,HWIO_HOST_SOC_A_WFSS_CE0_CE_DST_BATCH_TIMER_INT_SETUP_IN)
#define HWIO_HOST_SOC_A_WFSS_CE0_CE_DST_BATCH_TIMER_INT_SETUP_INTERRUPT_TIMER_THRESHOLD_BMSK               0xffff0000
#define HWIO_HOST_SOC_A_WFSS_CE0_CE_DST_BATCH_TIMER_INT_SETUP_INTERRUPT_TIMER_THRESHOLD_SHFT                       16
#define HWIO_HOST_SOC_A_WFSS_CE0_CE_DST_BATCH_TIMER_INT_SETUP_SW_INTERRUPT_MODE_BMSK                           0x8000
#define HWIO_HOST_SOC_A_WFSS_CE0_CE_DST_BATCH_TIMER_INT_SETUP_SW_INTERRUPT_MODE_SHFT                               15
#define HWIO_HOST_SOC_A_WFSS_CE0_CE_DST_BATCH_TIMER_INT_SETUP_BATCH_COUNTER_THRESHOLD_BMSK                     0x7fff
#define HWIO_HOST_SOC_A_WFSS_CE0_CE_DST_BATCH_TIMER_INT_SETUP_BATCH_COUNTER_THRESHOLD_SHFT                          0

#define HWIO_HOST_SOC_A_WFSS_CE0_CE_DST_BATCH_TIMER_INT_STATUS_ADDR                                        (HOST_SOC_A_WFSS_CE0_WFSS_CE_0_REG_REG_BASE      + 0x80)
#define HWIO_HOST_SOC_A_WFSS_CE0_CE_DST_BATCH_TIMER_INT_STATUS_RMSK                                        0xffffffff
#define HWIO_HOST_SOC_A_WFSS_CE0_CE_DST_BATCH_TIMER_INT_STATUS_IN                    \
                in_dword(HWIO_HOST_SOC_A_WFSS_CE0_CE_DST_BATCH_TIMER_INT_STATUS_ADDR)
#define HWIO_HOST_SOC_A_WFSS_CE0_CE_DST_BATCH_TIMER_INT_STATUS_INM(m)            \
                in_dword_masked(HWIO_HOST_SOC_A_WFSS_CE0_CE_DST_BATCH_TIMER_INT_STATUS_ADDR, m)
#define HWIO_HOST_SOC_A_WFSS_CE0_CE_DST_BATCH_TIMER_INT_STATUS_CURRENT_INTERRUPT_TIMER_VALUE_BMSK          0xffff0000
#define HWIO_HOST_SOC_A_WFSS_CE0_CE_DST_BATCH_TIMER_INT_STATUS_CURRENT_INTERRUPT_TIMER_VALUE_SHFT                  16
#define HWIO_HOST_SOC_A_WFSS_CE0_CE_DST_BATCH_TIMER_INT_STATUS_CURRENT_SW_INT_WIRE_VALUE_BMSK                  0x8000
#define HWIO_HOST_SOC_A_WFSS_CE0_CE_DST_BATCH_TIMER_INT_STATUS_CURRENT_SW_INT_WIRE_VALUE_SHFT                      15
#define HWIO_HOST_SOC_A_WFSS_CE0_CE_DST_BATCH_TIMER_INT_STATUS_INTERNAL_BATCH_COUNTER_VALUE_BMSK               0x7fff
#define HWIO_HOST_SOC_A_WFSS_CE0_CE_DST_BATCH_TIMER_INT_STATUS_INTERNAL_BATCH_COUNTER_VALUE_SHFT                    0

#define HWIO_HOST_SOC_A_WFSS_CE0_TARGET_MSI_ADDR_LOW_ADDR                                                  (HOST_SOC_A_WFSS_CE0_WFSS_CE_0_REG_REG_BASE      + 0x84)
#define HWIO_HOST_SOC_A_WFSS_CE0_TARGET_MSI_ADDR_LOW_RMSK                                                  0xffffffff
#define HWIO_HOST_SOC_A_WFSS_CE0_TARGET_MSI_ADDR_LOW_IN                    \
                in_dword(HWIO_HOST_SOC_A_WFSS_CE0_TARGET_MSI_ADDR_LOW_ADDR)
#define HWIO_HOST_SOC_A_WFSS_CE0_TARGET_MSI_ADDR_LOW_INM(m)            \
                in_dword_masked(HWIO_HOST_SOC_A_WFSS_CE0_TARGET_MSI_ADDR_LOW_ADDR, m)
#define HWIO_HOST_SOC_A_WFSS_CE0_TARGET_MSI_ADDR_LOW_OUT(v)            \
                out_dword(HWIO_HOST_SOC_A_WFSS_CE0_TARGET_MSI_ADDR_LOW_ADDR,v)
#define HWIO_HOST_SOC_A_WFSS_CE0_TARGET_MSI_ADDR_LOW_OUTM(m,v) \
                out_dword_masked_ns(HWIO_HOST_SOC_A_WFSS_CE0_TARGET_MSI_ADDR_LOW_ADDR,m,v,HWIO_HOST_SOC_A_WFSS_CE0_TARGET_MSI_ADDR_LOW_IN)
#define HWIO_HOST_SOC_A_WFSS_CE0_TARGET_MSI_ADDR_LOW_MSI_ADDR_LOW_BMSK                                     0xffffffff
#define HWIO_HOST_SOC_A_WFSS_CE0_TARGET_MSI_ADDR_LOW_MSI_ADDR_LOW_SHFT                                              0

#define HWIO_HOST_SOC_A_WFSS_CE0_TARGET_MSI_ADDR_HIGH_ADDR                                                 (HOST_SOC_A_WFSS_CE0_WFSS_CE_0_REG_REG_BASE      + 0x88)
#define HWIO_HOST_SOC_A_WFSS_CE0_TARGET_MSI_ADDR_HIGH_RMSK                                                       0xff
#define HWIO_HOST_SOC_A_WFSS_CE0_TARGET_MSI_ADDR_HIGH_IN                    \
                in_dword(HWIO_HOST_SOC_A_WFSS_CE0_TARGET_MSI_ADDR_HIGH_ADDR)
#define HWIO_HOST_SOC_A_WFSS_CE0_TARGET_MSI_ADDR_HIGH_INM(m)            \
                in_dword_masked(HWIO_HOST_SOC_A_WFSS_CE0_TARGET_MSI_ADDR_HIGH_ADDR, m)
#define HWIO_HOST_SOC_A_WFSS_CE0_TARGET_MSI_ADDR_HIGH_OUT(v)            \
                out_dword(HWIO_HOST_SOC_A_WFSS_CE0_TARGET_MSI_ADDR_HIGH_ADDR,v)
#define HWIO_HOST_SOC_A_WFSS_CE0_TARGET_MSI_ADDR_HIGH_OUTM(m,v) \
                out_dword_masked_ns(HWIO_HOST_SOC_A_WFSS_CE0_TARGET_MSI_ADDR_HIGH_ADDR,m,v,HWIO_HOST_SOC_A_WFSS_CE0_TARGET_MSI_ADDR_HIGH_IN)
#define HWIO_HOST_SOC_A_WFSS_CE0_TARGET_MSI_ADDR_HIGH_MSI_ADDR_HIGH_BMSK                                         0xff
#define HWIO_HOST_SOC_A_WFSS_CE0_TARGET_MSI_ADDR_HIGH_MSI_ADDR_HIGH_SHFT                                            0

#define HWIO_HOST_SOC_A_WFSS_CE0_TARGET_MSI_DATA_ADDR                                                      (HOST_SOC_A_WFSS_CE0_WFSS_CE_0_REG_REG_BASE      + 0x8c)
#define HWIO_HOST_SOC_A_WFSS_CE0_TARGET_MSI_DATA_RMSK                                                      0xffffffff
#define HWIO_HOST_SOC_A_WFSS_CE0_TARGET_MSI_DATA_IN                    \
                in_dword(HWIO_HOST_SOC_A_WFSS_CE0_TARGET_MSI_DATA_ADDR)
#define HWIO_HOST_SOC_A_WFSS_CE0_TARGET_MSI_DATA_INM(m)            \
                in_dword_masked(HWIO_HOST_SOC_A_WFSS_CE0_TARGET_MSI_DATA_ADDR, m)
#define HWIO_HOST_SOC_A_WFSS_CE0_TARGET_MSI_DATA_OUT(v)            \
                out_dword(HWIO_HOST_SOC_A_WFSS_CE0_TARGET_MSI_DATA_ADDR,v)
#define HWIO_HOST_SOC_A_WFSS_CE0_TARGET_MSI_DATA_OUTM(m,v) \
                out_dword_masked_ns(HWIO_HOST_SOC_A_WFSS_CE0_TARGET_MSI_DATA_ADDR,m,v,HWIO_HOST_SOC_A_WFSS_CE0_TARGET_MSI_DATA_IN)
#define HWIO_HOST_SOC_A_WFSS_CE0_TARGET_MSI_DATA_MSI_DATA_BMSK                                             0xffffffff
#define HWIO_HOST_SOC_A_WFSS_CE0_TARGET_MSI_DATA_MSI_DATA_SHFT                                                      0

#define HOST_SOC_A_WFSS_CE1_WFSS_CE_1_REG_REG_BASE                                                         (HOST_SOC_WFSS_CE_REG_TOP_BASE      + 0x00001000)
#define HOST_SOC_A_WFSS_CE1_WFSS_CE_1_REG_REG_BASE_SIZE                                                    0x1000
#define HOST_SOC_A_WFSS_CE1_WFSS_CE_1_REG_REG_BASE_USED                                                    0x8c

#define HWIO_HOST_SOC_A_WFSS_CE1_SR_BA_LOW_ADDR                                                            (HOST_SOC_A_WFSS_CE1_WFSS_CE_1_REG_REG_BASE      + 0x0)
#define HWIO_HOST_SOC_A_WFSS_CE1_SR_BA_LOW_RMSK                                                            0xffffffff
#define HWIO_HOST_SOC_A_WFSS_CE1_SR_BA_LOW_IN                    \
                in_dword(HWIO_HOST_SOC_A_WFSS_CE1_SR_BA_LOW_ADDR)
#define HWIO_HOST_SOC_A_WFSS_CE1_SR_BA_LOW_INM(m)            \
                in_dword_masked(HWIO_HOST_SOC_A_WFSS_CE1_SR_BA_LOW_ADDR, m)
#define HWIO_HOST_SOC_A_WFSS_CE1_SR_BA_LOW_OUT(v)            \
                out_dword(HWIO_HOST_SOC_A_WFSS_CE1_SR_BA_LOW_ADDR,v)
#define HWIO_HOST_SOC_A_WFSS_CE1_SR_BA_LOW_OUTM(m,v) \
                out_dword_masked_ns(HWIO_HOST_SOC_A_WFSS_CE1_SR_BA_LOW_ADDR,m,v,HWIO_HOST_SOC_A_WFSS_CE1_SR_BA_LOW_IN)
#define HWIO_HOST_SOC_A_WFSS_CE1_SR_BA_LOW_BASE_ADDR_LOW_BMSK                                              0xffffffff
#define HWIO_HOST_SOC_A_WFSS_CE1_SR_BA_LOW_BASE_ADDR_LOW_SHFT                                                       0

#define HWIO_HOST_SOC_A_WFSS_CE1_SR_BA_HIGH_ADDR                                                           (HOST_SOC_A_WFSS_CE1_WFSS_CE_1_REG_REG_BASE      + 0x4)
#define HWIO_HOST_SOC_A_WFSS_CE1_SR_BA_HIGH_RMSK                                                                 0xff
#define HWIO_HOST_SOC_A_WFSS_CE1_SR_BA_HIGH_IN                    \
                in_dword(HWIO_HOST_SOC_A_WFSS_CE1_SR_BA_HIGH_ADDR)
#define HWIO_HOST_SOC_A_WFSS_CE1_SR_BA_HIGH_INM(m)            \
                in_dword_masked(HWIO_HOST_SOC_A_WFSS_CE1_SR_BA_HIGH_ADDR, m)
#define HWIO_HOST_SOC_A_WFSS_CE1_SR_BA_HIGH_OUT(v)            \
                out_dword(HWIO_HOST_SOC_A_WFSS_CE1_SR_BA_HIGH_ADDR,v)
#define HWIO_HOST_SOC_A_WFSS_CE1_SR_BA_HIGH_OUTM(m,v) \
                out_dword_masked_ns(HWIO_HOST_SOC_A_WFSS_CE1_SR_BA_HIGH_ADDR,m,v,HWIO_HOST_SOC_A_WFSS_CE1_SR_BA_HIGH_IN)
#define HWIO_HOST_SOC_A_WFSS_CE1_SR_BA_HIGH_BASE_ADDR_HIGH_BMSK                                                  0xff
#define HWIO_HOST_SOC_A_WFSS_CE1_SR_BA_HIGH_BASE_ADDR_HIGH_SHFT                                                     0

#define HWIO_HOST_SOC_A_WFSS_CE1_SR_SIZE_ADDR                                                              (HOST_SOC_A_WFSS_CE1_WFSS_CE_1_REG_REG_BASE      + 0x8)
#define HWIO_HOST_SOC_A_WFSS_CE1_SR_SIZE_RMSK                                                              0xffffffff
#define HWIO_HOST_SOC_A_WFSS_CE1_SR_SIZE_IN                    \
                in_dword(HWIO_HOST_SOC_A_WFSS_CE1_SR_SIZE_ADDR)
#define HWIO_HOST_SOC_A_WFSS_CE1_SR_SIZE_INM(m)            \
                in_dword_masked(HWIO_HOST_SOC_A_WFSS_CE1_SR_SIZE_ADDR, m)
#define HWIO_HOST_SOC_A_WFSS_CE1_SR_SIZE_OUT(v)            \
                out_dword(HWIO_HOST_SOC_A_WFSS_CE1_SR_SIZE_ADDR,v)
#define HWIO_HOST_SOC_A_WFSS_CE1_SR_SIZE_OUTM(m,v) \
                out_dword_masked_ns(HWIO_HOST_SOC_A_WFSS_CE1_SR_SIZE_ADDR,m,v,HWIO_HOST_SOC_A_WFSS_CE1_SR_SIZE_IN)
#define HWIO_HOST_SOC_A_WFSS_CE1_SR_SIZE_START_OFFSET_BMSK                                                 0xffff0000
#define HWIO_HOST_SOC_A_WFSS_CE1_SR_SIZE_START_OFFSET_SHFT                                                         16
#define HWIO_HOST_SOC_A_WFSS_CE1_SR_SIZE_SIZE_BMSK                                                             0xffff
#define HWIO_HOST_SOC_A_WFSS_CE1_SR_SIZE_SIZE_SHFT                                                                  0

#define HWIO_HOST_SOC_A_WFSS_CE1_DR_BA_LOW_ADDR                                                            (HOST_SOC_A_WFSS_CE1_WFSS_CE_1_REG_REG_BASE      + 0xc)
#define HWIO_HOST_SOC_A_WFSS_CE1_DR_BA_LOW_RMSK                                                            0xffffffff
#define HWIO_HOST_SOC_A_WFSS_CE1_DR_BA_LOW_IN                    \
                in_dword(HWIO_HOST_SOC_A_WFSS_CE1_DR_BA_LOW_ADDR)
#define HWIO_HOST_SOC_A_WFSS_CE1_DR_BA_LOW_INM(m)            \
                in_dword_masked(HWIO_HOST_SOC_A_WFSS_CE1_DR_BA_LOW_ADDR, m)
#define HWIO_HOST_SOC_A_WFSS_CE1_DR_BA_LOW_OUT(v)            \
                out_dword(HWIO_HOST_SOC_A_WFSS_CE1_DR_BA_LOW_ADDR,v)
#define HWIO_HOST_SOC_A_WFSS_CE1_DR_BA_LOW_OUTM(m,v) \
                out_dword_masked_ns(HWIO_HOST_SOC_A_WFSS_CE1_DR_BA_LOW_ADDR,m,v,HWIO_HOST_SOC_A_WFSS_CE1_DR_BA_LOW_IN)
#define HWIO_HOST_SOC_A_WFSS_CE1_DR_BA_LOW_BASE_ADDR_LOW_BMSK                                              0xffffffff
#define HWIO_HOST_SOC_A_WFSS_CE1_DR_BA_LOW_BASE_ADDR_LOW_SHFT                                                       0

#define HWIO_HOST_SOC_A_WFSS_CE1_DR_BA_HIGH_ADDR                                                           (HOST_SOC_A_WFSS_CE1_WFSS_CE_1_REG_REG_BASE      + 0x10)
#define HWIO_HOST_SOC_A_WFSS_CE1_DR_BA_HIGH_RMSK                                                                0x3ff
#define HWIO_HOST_SOC_A_WFSS_CE1_DR_BA_HIGH_IN                    \
                in_dword(HWIO_HOST_SOC_A_WFSS_CE1_DR_BA_HIGH_ADDR)
#define HWIO_HOST_SOC_A_WFSS_CE1_DR_BA_HIGH_INM(m)            \
                in_dword_masked(HWIO_HOST_SOC_A_WFSS_CE1_DR_BA_HIGH_ADDR, m)
#define HWIO_HOST_SOC_A_WFSS_CE1_DR_BA_HIGH_OUT(v)            \
                out_dword(HWIO_HOST_SOC_A_WFSS_CE1_DR_BA_HIGH_ADDR,v)
#define HWIO_HOST_SOC_A_WFSS_CE1_DR_BA_HIGH_OUTM(m,v) \
                out_dword_masked_ns(HWIO_HOST_SOC_A_WFSS_CE1_DR_BA_HIGH_ADDR,m,v,HWIO_HOST_SOC_A_WFSS_CE1_DR_BA_HIGH_IN)
#define HWIO_HOST_SOC_A_WFSS_CE1_DR_BA_HIGH_DESC_SKIP_DWORD_BMSK                                                0x300
#define HWIO_HOST_SOC_A_WFSS_CE1_DR_BA_HIGH_DESC_SKIP_DWORD_SHFT                                                    8
#define HWIO_HOST_SOC_A_WFSS_CE1_DR_BA_HIGH_BASE_ADDR_HIGH_BMSK                                                  0xff
#define HWIO_HOST_SOC_A_WFSS_CE1_DR_BA_HIGH_BASE_ADDR_HIGH_SHFT                                                     0

#define HWIO_HOST_SOC_A_WFSS_CE1_DR_SIZE_ADDR                                                              (HOST_SOC_A_WFSS_CE1_WFSS_CE_1_REG_REG_BASE      + 0x14)
#define HWIO_HOST_SOC_A_WFSS_CE1_DR_SIZE_RMSK                                                              0xffffffff
#define HWIO_HOST_SOC_A_WFSS_CE1_DR_SIZE_IN                    \
                in_dword(HWIO_HOST_SOC_A_WFSS_CE1_DR_SIZE_ADDR)
#define HWIO_HOST_SOC_A_WFSS_CE1_DR_SIZE_INM(m)            \
                in_dword_masked(HWIO_HOST_SOC_A_WFSS_CE1_DR_SIZE_ADDR, m)
#define HWIO_HOST_SOC_A_WFSS_CE1_DR_SIZE_OUT(v)            \
                out_dword(HWIO_HOST_SOC_A_WFSS_CE1_DR_SIZE_ADDR,v)
#define HWIO_HOST_SOC_A_WFSS_CE1_DR_SIZE_OUTM(m,v) \
                out_dword_masked_ns(HWIO_HOST_SOC_A_WFSS_CE1_DR_SIZE_ADDR,m,v,HWIO_HOST_SOC_A_WFSS_CE1_DR_SIZE_IN)
#define HWIO_HOST_SOC_A_WFSS_CE1_DR_SIZE_START_OFFSET_BMSK                                                 0xffff0000
#define HWIO_HOST_SOC_A_WFSS_CE1_DR_SIZE_START_OFFSET_SHFT                                                         16
#define HWIO_HOST_SOC_A_WFSS_CE1_DR_SIZE_SIZE_BMSK                                                             0xffff
#define HWIO_HOST_SOC_A_WFSS_CE1_DR_SIZE_SIZE_SHFT                                                                  0

#define HWIO_HOST_SOC_A_WFSS_CE1_CE_CTRL1_ADDR                                                             (HOST_SOC_A_WFSS_CE1_WFSS_CE_1_REG_REG_BASE      + 0x18)
#define HWIO_HOST_SOC_A_WFSS_CE1_CE_CTRL1_RMSK                                                              0x1ffffff
#define HWIO_HOST_SOC_A_WFSS_CE1_CE_CTRL1_IN                    \
                in_dword(HWIO_HOST_SOC_A_WFSS_CE1_CE_CTRL1_ADDR)
#define HWIO_HOST_SOC_A_WFSS_CE1_CE_CTRL1_INM(m)            \
                in_dword_masked(HWIO_HOST_SOC_A_WFSS_CE1_CE_CTRL1_ADDR, m)
#define HWIO_HOST_SOC_A_WFSS_CE1_CE_CTRL1_OUT(v)            \
                out_dword(HWIO_HOST_SOC_A_WFSS_CE1_CE_CTRL1_ADDR,v)
#define HWIO_HOST_SOC_A_WFSS_CE1_CE_CTRL1_OUTM(m,v) \
                out_dword_masked_ns(HWIO_HOST_SOC_A_WFSS_CE1_CE_CTRL1_ADDR,m,v,HWIO_HOST_SOC_A_WFSS_CE1_CE_CTRL1_IN)
#define HWIO_HOST_SOC_A_WFSS_CE1_CE_CTRL1_TARGET_MSI_EN_BMSK                                                0x1000000
#define HWIO_HOST_SOC_A_WFSS_CE1_CE_CTRL1_TARGET_MSI_EN_SHFT                                                       24
#define HWIO_HOST_SOC_A_WFSS_CE1_CE_CTRL1_FW_EN_BMSK                                                         0x800000
#define HWIO_HOST_SOC_A_WFSS_CE1_CE_CTRL1_FW_EN_SHFT                                                               23
#define HWIO_HOST_SOC_A_WFSS_CE1_CE_CTRL1_SECURITY_DEST_BMSK                                                 0x400000
#define HWIO_HOST_SOC_A_WFSS_CE1_CE_CTRL1_SECURITY_DEST_SHFT                                                       22
#define HWIO_HOST_SOC_A_WFSS_CE1_CE_CTRL1_SECURITY_SRC_BMSK                                                  0x200000
#define HWIO_HOST_SOC_A_WFSS_CE1_CE_CTRL1_SECURITY_SRC_SHFT                                                        21
#define HWIO_HOST_SOC_A_WFSS_CE1_CE_CTRL1_PREFETCH_EN_BMSK                                                   0x100000
#define HWIO_HOST_SOC_A_WFSS_CE1_CE_CTRL1_PREFETCH_EN_SHFT                                                         20
#define HWIO_HOST_SOC_A_WFSS_CE1_CE_CTRL1_IDX_UPD_EN_BMSK                                                     0x80000
#define HWIO_HOST_SOC_A_WFSS_CE1_CE_CTRL1_IDX_UPD_EN_SHFT                                                          19
#define HWIO_HOST_SOC_A_WFSS_CE1_CE_CTRL1_DST_RING_BYTE_SWAP_EN_BMSK                                          0x40000
#define HWIO_HOST_SOC_A_WFSS_CE1_CE_CTRL1_DST_RING_BYTE_SWAP_EN_SHFT                                               18
#define HWIO_HOST_SOC_A_WFSS_CE1_CE_CTRL1_SRC_RING_BYTE_SWAP_EN_BMSK                                          0x20000
#define HWIO_HOST_SOC_A_WFSS_CE1_CE_CTRL1_SRC_RING_BYTE_SWAP_EN_SHFT                                               17
#define HWIO_HOST_SOC_A_WFSS_CE1_CE_CTRL1_HOST_MSI_EN_BMSK                                                    0x10000
#define HWIO_HOST_SOC_A_WFSS_CE1_CE_CTRL1_HOST_MSI_EN_SHFT                                                         16
#define HWIO_HOST_SOC_A_WFSS_CE1_CE_CTRL1_DEST_MAX_LENGTH_BMSK                                                 0xffff
#define HWIO_HOST_SOC_A_WFSS_CE1_CE_CTRL1_DEST_MAX_LENGTH_SHFT                                                      0

#define HWIO_HOST_SOC_A_WFSS_CE1_CE_CTRL2_ADDR                                                             (HOST_SOC_A_WFSS_CE1_WFSS_CE_1_REG_REG_BASE      + 0x1c)
#define HWIO_HOST_SOC_A_WFSS_CE1_CE_CTRL2_RMSK                                                                    0xf
#define HWIO_HOST_SOC_A_WFSS_CE1_CE_CTRL2_IN                    \
                in_dword(HWIO_HOST_SOC_A_WFSS_CE1_CE_CTRL2_ADDR)
#define HWIO_HOST_SOC_A_WFSS_CE1_CE_CTRL2_INM(m)            \
                in_dword_masked(HWIO_HOST_SOC_A_WFSS_CE1_CE_CTRL2_ADDR, m)
#define HWIO_HOST_SOC_A_WFSS_CE1_CE_CTRL2_OUT(v)            \
                out_dword(HWIO_HOST_SOC_A_WFSS_CE1_CE_CTRL2_ADDR,v)
#define HWIO_HOST_SOC_A_WFSS_CE1_CE_CTRL2_OUTM(m,v) \
                out_dword_masked_ns(HWIO_HOST_SOC_A_WFSS_CE1_CE_CTRL2_ADDR,m,v,HWIO_HOST_SOC_A_WFSS_CE1_CE_CTRL2_IN)
#define HWIO_HOST_SOC_A_WFSS_CE1_CE_CTRL2_DST_AXI_MAX_LEN_BMSK                                                    0xc
#define HWIO_HOST_SOC_A_WFSS_CE1_CE_CTRL2_DST_AXI_MAX_LEN_SHFT                                                      2
#define HWIO_HOST_SOC_A_WFSS_CE1_CE_CTRL2_SRC_AXI_MAX_LEN_BMSK                                                    0x3
#define HWIO_HOST_SOC_A_WFSS_CE1_CE_CTRL2_SRC_AXI_MAX_LEN_SHFT                                                      0

#define HWIO_HOST_SOC_A_WFSS_CE1_CE_CMD_ADDR                                                               (HOST_SOC_A_WFSS_CE1_WFSS_CE_1_REG_REG_BASE      + 0x20)
#define HWIO_HOST_SOC_A_WFSS_CE1_CE_CMD_RMSK                                                                      0xf
#define HWIO_HOST_SOC_A_WFSS_CE1_CE_CMD_IN                    \
                in_dword(HWIO_HOST_SOC_A_WFSS_CE1_CE_CMD_ADDR)
#define HWIO_HOST_SOC_A_WFSS_CE1_CE_CMD_INM(m)            \
                in_dword_masked(HWIO_HOST_SOC_A_WFSS_CE1_CE_CMD_ADDR, m)
#define HWIO_HOST_SOC_A_WFSS_CE1_CE_CMD_OUT(v)            \
                out_dword(HWIO_HOST_SOC_A_WFSS_CE1_CE_CMD_ADDR,v)
#define HWIO_HOST_SOC_A_WFSS_CE1_CE_CMD_OUTM(m,v) \
                out_dword_masked_ns(HWIO_HOST_SOC_A_WFSS_CE1_CE_CMD_ADDR,m,v,HWIO_HOST_SOC_A_WFSS_CE1_CE_CMD_IN)
#define HWIO_HOST_SOC_A_WFSS_CE1_CE_CMD_HALT_STATUS_BMSK                                                          0x8
#define HWIO_HOST_SOC_A_WFSS_CE1_CE_CMD_HALT_STATUS_SHFT                                                            3
#define HWIO_HOST_SOC_A_WFSS_CE1_CE_CMD_DST_FLUSH_BMSK                                                            0x4
#define HWIO_HOST_SOC_A_WFSS_CE1_CE_CMD_DST_FLUSH_SHFT                                                              2
#define HWIO_HOST_SOC_A_WFSS_CE1_CE_CMD_SRC_FLUSH_BMSK                                                            0x2
#define HWIO_HOST_SOC_A_WFSS_CE1_CE_CMD_SRC_FLUSH_SHFT                                                              1
#define HWIO_HOST_SOC_A_WFSS_CE1_CE_CMD_HALT_BMSK                                                                 0x1
#define HWIO_HOST_SOC_A_WFSS_CE1_CE_CMD_HALT_SHFT                                                                   0

#define HWIO_HOST_SOC_A_WFSS_CE1_TARGET_IE_ADDR                                                            (HOST_SOC_A_WFSS_CE1_WFSS_CE_1_REG_REG_BASE      + 0x24)
#define HWIO_HOST_SOC_A_WFSS_CE1_TARGET_IE_RMSK                                                                  0x7f
#define HWIO_HOST_SOC_A_WFSS_CE1_TARGET_IE_IN                    \
                in_dword(HWIO_HOST_SOC_A_WFSS_CE1_TARGET_IE_ADDR)
#define HWIO_HOST_SOC_A_WFSS_CE1_TARGET_IE_INM(m)            \
                in_dword_masked(HWIO_HOST_SOC_A_WFSS_CE1_TARGET_IE_ADDR, m)
#define HWIO_HOST_SOC_A_WFSS_CE1_TARGET_IE_OUT(v)            \
                out_dword(HWIO_HOST_SOC_A_WFSS_CE1_TARGET_IE_ADDR,v)
#define HWIO_HOST_SOC_A_WFSS_CE1_TARGET_IE_OUTM(m,v) \
                out_dword_masked_ns(HWIO_HOST_SOC_A_WFSS_CE1_TARGET_IE_ADDR,m,v,HWIO_HOST_SOC_A_WFSS_CE1_TARGET_IE_IN)
#define HWIO_HOST_SOC_A_WFSS_CE1_TARGET_IE_DST_TIMER_BATCH_BMSK                                                  0x40
#define HWIO_HOST_SOC_A_WFSS_CE1_TARGET_IE_DST_TIMER_BATCH_SHFT                                                     6
#define HWIO_HOST_SOC_A_WFSS_CE1_TARGET_IE_SRC_TIMER_BATCH_BMSK                                                  0x20
#define HWIO_HOST_SOC_A_WFSS_CE1_TARGET_IE_SRC_TIMER_BATCH_SHFT                                                     5
#define HWIO_HOST_SOC_A_WFSS_CE1_TARGET_IE_DST_RING_LOW_WATERMARK_BMSK                                           0x10
#define HWIO_HOST_SOC_A_WFSS_CE1_TARGET_IE_DST_RING_LOW_WATERMARK_SHFT                                              4
#define HWIO_HOST_SOC_A_WFSS_CE1_TARGET_IE_DST_RING_HIGH_WATERMARK_BMSK                                           0x8
#define HWIO_HOST_SOC_A_WFSS_CE1_TARGET_IE_DST_RING_HIGH_WATERMARK_SHFT                                             3
#define HWIO_HOST_SOC_A_WFSS_CE1_TARGET_IE_SRC_RING_LOW_WATERMARK_BMSK                                            0x4
#define HWIO_HOST_SOC_A_WFSS_CE1_TARGET_IE_SRC_RING_LOW_WATERMARK_SHFT                                              2
#define HWIO_HOST_SOC_A_WFSS_CE1_TARGET_IE_SRC_RING_HIGH_WATERMARK_BMSK                                           0x2
#define HWIO_HOST_SOC_A_WFSS_CE1_TARGET_IE_SRC_RING_HIGH_WATERMARK_SHFT                                             1
#define HWIO_HOST_SOC_A_WFSS_CE1_TARGET_IE_COPY_COMPLETE_BMSK                                                     0x1
#define HWIO_HOST_SOC_A_WFSS_CE1_TARGET_IE_COPY_COMPLETE_SHFT                                                       0

#define HWIO_HOST_SOC_A_WFSS_CE1_TARGET_IS_ADDR                                                            (HOST_SOC_A_WFSS_CE1_WFSS_CE_1_REG_REG_BASE      + 0x28)
#define HWIO_HOST_SOC_A_WFSS_CE1_TARGET_IS_RMSK                                                                  0x7f
#define HWIO_HOST_SOC_A_WFSS_CE1_TARGET_IS_IN                    \
                in_dword(HWIO_HOST_SOC_A_WFSS_CE1_TARGET_IS_ADDR)
#define HWIO_HOST_SOC_A_WFSS_CE1_TARGET_IS_INM(m)            \
                in_dword_masked(HWIO_HOST_SOC_A_WFSS_CE1_TARGET_IS_ADDR, m)
#define HWIO_HOST_SOC_A_WFSS_CE1_TARGET_IS_OUT(v)            \
                out_dword(HWIO_HOST_SOC_A_WFSS_CE1_TARGET_IS_ADDR,v)
#define HWIO_HOST_SOC_A_WFSS_CE1_TARGET_IS_OUTM(m,v) \
                out_dword_masked_ns(HWIO_HOST_SOC_A_WFSS_CE1_TARGET_IS_ADDR,m,v,HWIO_HOST_SOC_A_WFSS_CE1_TARGET_IS_IN)
#define HWIO_HOST_SOC_A_WFSS_CE1_TARGET_IS_DST_TIMER_BATCH_BMSK                                                  0x40
#define HWIO_HOST_SOC_A_WFSS_CE1_TARGET_IS_DST_TIMER_BATCH_SHFT                                                     6
#define HWIO_HOST_SOC_A_WFSS_CE1_TARGET_IS_SRC_TIMER_BATCH_BMSK                                                  0x20
#define HWIO_HOST_SOC_A_WFSS_CE1_TARGET_IS_SRC_TIMER_BATCH_SHFT                                                     5
#define HWIO_HOST_SOC_A_WFSS_CE1_TARGET_IS_DST_RING_LOW_WATERMARK_BMSK                                           0x10
#define HWIO_HOST_SOC_A_WFSS_CE1_TARGET_IS_DST_RING_LOW_WATERMARK_SHFT                                              4
#define HWIO_HOST_SOC_A_WFSS_CE1_TARGET_IS_DST_RING_HIGH_WATERMARK_BMSK                                           0x8
#define HWIO_HOST_SOC_A_WFSS_CE1_TARGET_IS_DST_RING_HIGH_WATERMARK_SHFT                                             3
#define HWIO_HOST_SOC_A_WFSS_CE1_TARGET_IS_SRC_RING_LOW_WATERMARK_BMSK                                            0x4
#define HWIO_HOST_SOC_A_WFSS_CE1_TARGET_IS_SRC_RING_LOW_WATERMARK_SHFT                                              2
#define HWIO_HOST_SOC_A_WFSS_CE1_TARGET_IS_SRC_RING_HIGH_WATERMARK_BMSK                                           0x2
#define HWIO_HOST_SOC_A_WFSS_CE1_TARGET_IS_SRC_RING_HIGH_WATERMARK_SHFT                                             1
#define HWIO_HOST_SOC_A_WFSS_CE1_TARGET_IS_COPY_COMPLETE_BMSK                                                     0x1
#define HWIO_HOST_SOC_A_WFSS_CE1_TARGET_IS_COPY_COMPLETE_SHFT                                                       0

#define HWIO_HOST_SOC_A_WFSS_CE1_HOST_IE_ADDR                                                              (HOST_SOC_A_WFSS_CE1_WFSS_CE_1_REG_REG_BASE      + 0x2c)
#define HWIO_HOST_SOC_A_WFSS_CE1_HOST_IE_RMSK                                                                    0x7f
#define HWIO_HOST_SOC_A_WFSS_CE1_HOST_IE_IN                    \
                in_dword(HWIO_HOST_SOC_A_WFSS_CE1_HOST_IE_ADDR)
#define HWIO_HOST_SOC_A_WFSS_CE1_HOST_IE_INM(m)            \
                in_dword_masked(HWIO_HOST_SOC_A_WFSS_CE1_HOST_IE_ADDR, m)
#define HWIO_HOST_SOC_A_WFSS_CE1_HOST_IE_OUT(v)            \
                out_dword(HWIO_HOST_SOC_A_WFSS_CE1_HOST_IE_ADDR,v)
#define HWIO_HOST_SOC_A_WFSS_CE1_HOST_IE_OUTM(m,v) \
                out_dword_masked_ns(HWIO_HOST_SOC_A_WFSS_CE1_HOST_IE_ADDR,m,v,HWIO_HOST_SOC_A_WFSS_CE1_HOST_IE_IN)
#define HWIO_HOST_SOC_A_WFSS_CE1_HOST_IE_DST_TIMER_BATCH_BMSK                                                    0x40
#define HWIO_HOST_SOC_A_WFSS_CE1_HOST_IE_DST_TIMER_BATCH_SHFT                                                       6
#define HWIO_HOST_SOC_A_WFSS_CE1_HOST_IE_SRC_TIMER_BATCH_BMSK                                                    0x20
#define HWIO_HOST_SOC_A_WFSS_CE1_HOST_IE_SRC_TIMER_BATCH_SHFT                                                       5
#define HWIO_HOST_SOC_A_WFSS_CE1_HOST_IE_DST_RING_LOW_WATERMARK_BMSK                                             0x10
#define HWIO_HOST_SOC_A_WFSS_CE1_HOST_IE_DST_RING_LOW_WATERMARK_SHFT                                                4
#define HWIO_HOST_SOC_A_WFSS_CE1_HOST_IE_DST_RING_HIGH_WATERMARK_BMSK                                             0x8
#define HWIO_HOST_SOC_A_WFSS_CE1_HOST_IE_DST_RING_HIGH_WATERMARK_SHFT                                               3
#define HWIO_HOST_SOC_A_WFSS_CE1_HOST_IE_SRC_RING_LOW_WATERMARK_BMSK                                              0x4
#define HWIO_HOST_SOC_A_WFSS_CE1_HOST_IE_SRC_RING_LOW_WATERMARK_SHFT                                                2
#define HWIO_HOST_SOC_A_WFSS_CE1_HOST_IE_SRC_RING_HIGH_WATERMARK_BMSK                                             0x2
#define HWIO_HOST_SOC_A_WFSS_CE1_HOST_IE_SRC_RING_HIGH_WATERMARK_SHFT                                               1
#define HWIO_HOST_SOC_A_WFSS_CE1_HOST_IE_COPY_COMPLETE_BMSK                                                       0x1
#define HWIO_HOST_SOC_A_WFSS_CE1_HOST_IE_COPY_COMPLETE_SHFT                                                         0

#define HWIO_HOST_SOC_A_WFSS_CE1_HOST_IS_ADDR                                                              (HOST_SOC_A_WFSS_CE1_WFSS_CE_1_REG_REG_BASE      + 0x30)
#define HWIO_HOST_SOC_A_WFSS_CE1_HOST_IS_RMSK                                                                    0x7f
#define HWIO_HOST_SOC_A_WFSS_CE1_HOST_IS_IN                    \
                in_dword(HWIO_HOST_SOC_A_WFSS_CE1_HOST_IS_ADDR)
#define HWIO_HOST_SOC_A_WFSS_CE1_HOST_IS_INM(m)            \
                in_dword_masked(HWIO_HOST_SOC_A_WFSS_CE1_HOST_IS_ADDR, m)
#define HWIO_HOST_SOC_A_WFSS_CE1_HOST_IS_OUT(v)            \
                out_dword(HWIO_HOST_SOC_A_WFSS_CE1_HOST_IS_ADDR,v)
#define HWIO_HOST_SOC_A_WFSS_CE1_HOST_IS_OUTM(m,v) \
                out_dword_masked_ns(HWIO_HOST_SOC_A_WFSS_CE1_HOST_IS_ADDR,m,v,HWIO_HOST_SOC_A_WFSS_CE1_HOST_IS_IN)
#define HWIO_HOST_SOC_A_WFSS_CE1_HOST_IS_DST_TIMER_BATCH_BMSK                                                    0x40
#define HWIO_HOST_SOC_A_WFSS_CE1_HOST_IS_DST_TIMER_BATCH_SHFT                                                       6
#define HWIO_HOST_SOC_A_WFSS_CE1_HOST_IS_SRC_TIMER_BATCH_BMSK                                                    0x20
#define HWIO_HOST_SOC_A_WFSS_CE1_HOST_IS_SRC_TIMER_BATCH_SHFT                                                       5
#define HWIO_HOST_SOC_A_WFSS_CE1_HOST_IS_DST_RING_LOW_WATERMARK_BMSK                                             0x10
#define HWIO_HOST_SOC_A_WFSS_CE1_HOST_IS_DST_RING_LOW_WATERMARK_SHFT                                                4
#define HWIO_HOST_SOC_A_WFSS_CE1_HOST_IS_DST_RING_HIGH_WATERMARK_BMSK                                             0x8
#define HWIO_HOST_SOC_A_WFSS_CE1_HOST_IS_DST_RING_HIGH_WATERMARK_SHFT                                               3
#define HWIO_HOST_SOC_A_WFSS_CE1_HOST_IS_SRC_RING_LOW_WATERMARK_BMSK                                              0x4
#define HWIO_HOST_SOC_A_WFSS_CE1_HOST_IS_SRC_RING_LOW_WATERMARK_SHFT                                                2
#define HWIO_HOST_SOC_A_WFSS_CE1_HOST_IS_SRC_RING_HIGH_WATERMARK_BMSK                                             0x2
#define HWIO_HOST_SOC_A_WFSS_CE1_HOST_IS_SRC_RING_HIGH_WATERMARK_SHFT                                               1
#define HWIO_HOST_SOC_A_WFSS_CE1_HOST_IS_COPY_COMPLETE_BMSK                                                       0x1
#define HWIO_HOST_SOC_A_WFSS_CE1_HOST_IS_COPY_COMPLETE_SHFT                                                         0

#define HWIO_HOST_SOC_A_WFSS_CE1_MISC_IE_ADDR                                                              (HOST_SOC_A_WFSS_CE1_WFSS_CE_1_REG_REG_BASE      + 0x34)
#define HWIO_HOST_SOC_A_WFSS_CE1_MISC_IE_RMSK                                                                 0xfffff
#define HWIO_HOST_SOC_A_WFSS_CE1_MISC_IE_IN                    \
                in_dword(HWIO_HOST_SOC_A_WFSS_CE1_MISC_IE_ADDR)
#define HWIO_HOST_SOC_A_WFSS_CE1_MISC_IE_INM(m)            \
                in_dword_masked(HWIO_HOST_SOC_A_WFSS_CE1_MISC_IE_ADDR, m)
#define HWIO_HOST_SOC_A_WFSS_CE1_MISC_IE_OUT(v)            \
                out_dword(HWIO_HOST_SOC_A_WFSS_CE1_MISC_IE_ADDR,v)
#define HWIO_HOST_SOC_A_WFSS_CE1_MISC_IE_OUTM(m,v) \
                out_dword_masked_ns(HWIO_HOST_SOC_A_WFSS_CE1_MISC_IE_ADDR,m,v,HWIO_HOST_SOC_A_WFSS_CE1_MISC_IE_IN)
#define HWIO_HOST_SOC_A_WFSS_CE1_MISC_IE_PARSER_INT_BMSK                                                      0xfc000
#define HWIO_HOST_SOC_A_WFSS_CE1_MISC_IE_PARSER_INT_SHFT                                                           14
#define HWIO_HOST_SOC_A_WFSS_CE1_MISC_IE_REG_INVAL_ADDR_RD_BMSK                                                0x2000
#define HWIO_HOST_SOC_A_WFSS_CE1_MISC_IE_REG_INVAL_ADDR_RD_SHFT                                                    13
#define HWIO_HOST_SOC_A_WFSS_CE1_MISC_IE_REG_INVAL_ADDR_WR_BMSK                                                0x1000
#define HWIO_HOST_SOC_A_WFSS_CE1_MISC_IE_REG_INVAL_ADDR_WR_SHFT                                                    12
#define HWIO_HOST_SOC_A_WFSS_CE1_MISC_IE_REG_RO_WR_BMSK                                                         0x800
#define HWIO_HOST_SOC_A_WFSS_CE1_MISC_IE_REG_RO_WR_SHFT                                                            11
#define HWIO_HOST_SOC_A_WFSS_CE1_MISC_IE_AXI_TIMEOUT_ERR_BMSK                                                   0x400
#define HWIO_HOST_SOC_A_WFSS_CE1_MISC_IE_AXI_TIMEOUT_ERR_SHFT                                                      10
#define HWIO_HOST_SOC_A_WFSS_CE1_MISC_IE_AXI_ERR_BMSK                                                           0x200
#define HWIO_HOST_SOC_A_WFSS_CE1_MISC_IE_AXI_ERR_SHFT                                                               9
#define HWIO_HOST_SOC_A_WFSS_CE1_MISC_IE_SRC_LEN_ERR_BMSK                                                       0x100
#define HWIO_HOST_SOC_A_WFSS_CE1_MISC_IE_SRC_LEN_ERR_SHFT                                                           8
#define HWIO_HOST_SOC_A_WFSS_CE1_MISC_IE_DST_MAX_LEN_VIO_BMSK                                                    0x80
#define HWIO_HOST_SOC_A_WFSS_CE1_MISC_IE_DST_MAX_LEN_VIO_SHFT                                                       7
#define HWIO_HOST_SOC_A_WFSS_CE1_MISC_IE_DST_RING_OVERFLOW_BMSK                                                  0x40
#define HWIO_HOST_SOC_A_WFSS_CE1_MISC_IE_DST_RING_OVERFLOW_SHFT                                                     6
#define HWIO_HOST_SOC_A_WFSS_CE1_MISC_IE_SRC_RING_OVERFLOW_BMSK                                                  0x20
#define HWIO_HOST_SOC_A_WFSS_CE1_MISC_IE_SRC_RING_OVERFLOW_SHFT                                                     5
#define HWIO_HOST_SOC_A_WFSS_CE1_MISC_IE_DST_RING_LOW_WATERMARK_BMSK                                             0x10
#define HWIO_HOST_SOC_A_WFSS_CE1_MISC_IE_DST_RING_LOW_WATERMARK_SHFT                                                4
#define HWIO_HOST_SOC_A_WFSS_CE1_MISC_IE_DST_RING_HIGH_WATERMARK_BMSK                                             0x8
#define HWIO_HOST_SOC_A_WFSS_CE1_MISC_IE_DST_RING_HIGH_WATERMARK_SHFT                                               3
#define HWIO_HOST_SOC_A_WFSS_CE1_MISC_IE_SRC_RING_LOW_WATERMARK_BMSK                                              0x4
#define HWIO_HOST_SOC_A_WFSS_CE1_MISC_IE_SRC_RING_LOW_WATERMARK_SHFT                                                2
#define HWIO_HOST_SOC_A_WFSS_CE1_MISC_IE_SRC_RING_HIGH_WATERMARK_BMSK                                             0x2
#define HWIO_HOST_SOC_A_WFSS_CE1_MISC_IE_SRC_RING_HIGH_WATERMARK_SHFT                                               1
#define HWIO_HOST_SOC_A_WFSS_CE1_MISC_IE_COPY_COMPLETE_BMSK                                                       0x1
#define HWIO_HOST_SOC_A_WFSS_CE1_MISC_IE_COPY_COMPLETE_SHFT                                                         0

#define HWIO_HOST_SOC_A_WFSS_CE1_MISC_IS_ADDR                                                              (HOST_SOC_A_WFSS_CE1_WFSS_CE_1_REG_REG_BASE      + 0x38)
#define HWIO_HOST_SOC_A_WFSS_CE1_MISC_IS_RMSK                                                                 0xfffff
#define HWIO_HOST_SOC_A_WFSS_CE1_MISC_IS_IN                    \
                in_dword(HWIO_HOST_SOC_A_WFSS_CE1_MISC_IS_ADDR)
#define HWIO_HOST_SOC_A_WFSS_CE1_MISC_IS_INM(m)            \
                in_dword_masked(HWIO_HOST_SOC_A_WFSS_CE1_MISC_IS_ADDR, m)
#define HWIO_HOST_SOC_A_WFSS_CE1_MISC_IS_OUT(v)            \
                out_dword(HWIO_HOST_SOC_A_WFSS_CE1_MISC_IS_ADDR,v)
#define HWIO_HOST_SOC_A_WFSS_CE1_MISC_IS_OUTM(m,v) \
                out_dword_masked_ns(HWIO_HOST_SOC_A_WFSS_CE1_MISC_IS_ADDR,m,v,HWIO_HOST_SOC_A_WFSS_CE1_MISC_IS_IN)
#define HWIO_HOST_SOC_A_WFSS_CE1_MISC_IS_PARSER_INT_BMSK                                                      0xfc000
#define HWIO_HOST_SOC_A_WFSS_CE1_MISC_IS_PARSER_INT_SHFT                                                           14
#define HWIO_HOST_SOC_A_WFSS_CE1_MISC_IS_REG_INVAL_ADDR_RD_BMSK                                                0x2000
#define HWIO_HOST_SOC_A_WFSS_CE1_MISC_IS_REG_INVAL_ADDR_RD_SHFT                                                    13
#define HWIO_HOST_SOC_A_WFSS_CE1_MISC_IS_REG_INVAL_ADDR_WR_BMSK                                                0x1000
#define HWIO_HOST_SOC_A_WFSS_CE1_MISC_IS_REG_INVAL_ADDR_WR_SHFT                                                    12
#define HWIO_HOST_SOC_A_WFSS_CE1_MISC_IS_REG_INVAL_RO_WR_BMSK                                                   0x800
#define HWIO_HOST_SOC_A_WFSS_CE1_MISC_IS_REG_INVAL_RO_WR_SHFT                                                      11
#define HWIO_HOST_SOC_A_WFSS_CE1_MISC_IS_AXI_TIMEOUT_ERR_BMSK                                                   0x400
#define HWIO_HOST_SOC_A_WFSS_CE1_MISC_IS_AXI_TIMEOUT_ERR_SHFT                                                      10
#define HWIO_HOST_SOC_A_WFSS_CE1_MISC_IS_AXI_BUS_ERR_BMSK                                                       0x200
#define HWIO_HOST_SOC_A_WFSS_CE1_MISC_IS_AXI_BUS_ERR_SHFT                                                           9
#define HWIO_HOST_SOC_A_WFSS_CE1_MISC_IS_SRC_LEN_ERR_BMSK                                                       0x100
#define HWIO_HOST_SOC_A_WFSS_CE1_MISC_IS_SRC_LEN_ERR_SHFT                                                           8
#define HWIO_HOST_SOC_A_WFSS_CE1_MISC_IS_DST_MAX_LEN_VIO_BMSK                                                    0x80
#define HWIO_HOST_SOC_A_WFSS_CE1_MISC_IS_DST_MAX_LEN_VIO_SHFT                                                       7
#define HWIO_HOST_SOC_A_WFSS_CE1_MISC_IS_DST_RING_OVERFLOW_BMSK                                                  0x40
#define HWIO_HOST_SOC_A_WFSS_CE1_MISC_IS_DST_RING_OVERFLOW_SHFT                                                     6
#define HWIO_HOST_SOC_A_WFSS_CE1_MISC_IS_SRC_RING_OVERFLOW_BMSK                                                  0x20
#define HWIO_HOST_SOC_A_WFSS_CE1_MISC_IS_SRC_RING_OVERFLOW_SHFT                                                     5
#define HWIO_HOST_SOC_A_WFSS_CE1_MISC_IS_DST_RING_LOW_WATERMARK_BMSK                                             0x10
#define HWIO_HOST_SOC_A_WFSS_CE1_MISC_IS_DST_RING_LOW_WATERMARK_SHFT                                                4
#define HWIO_HOST_SOC_A_WFSS_CE1_MISC_IS_DST_RING_HIGH_WATERMARK_BMSK                                             0x8
#define HWIO_HOST_SOC_A_WFSS_CE1_MISC_IS_DST_RING_HIGH_WATERMARK_SHFT                                               3
#define HWIO_HOST_SOC_A_WFSS_CE1_MISC_IS_SRC_RING_LOW_WATERMARK_BMSK                                              0x4
#define HWIO_HOST_SOC_A_WFSS_CE1_MISC_IS_SRC_RING_LOW_WATERMARK_SHFT                                                2
#define HWIO_HOST_SOC_A_WFSS_CE1_MISC_IS_SRC_RING_HIGH_WATERMARK_BMSK                                             0x2
#define HWIO_HOST_SOC_A_WFSS_CE1_MISC_IS_SRC_RING_HIGH_WATERMARK_SHFT                                               1
#define HWIO_HOST_SOC_A_WFSS_CE1_MISC_IS_COPY_COMPLETE_BMSK                                                       0x1
#define HWIO_HOST_SOC_A_WFSS_CE1_MISC_IS_COPY_COMPLETE_SHFT                                                         0

#define HWIO_HOST_SOC_A_WFSS_CE1_SRC_WR_INDEX_ADDR                                                         (HOST_SOC_A_WFSS_CE1_WFSS_CE_1_REG_REG_BASE      + 0x3c)
#define HWIO_HOST_SOC_A_WFSS_CE1_SRC_WR_INDEX_RMSK                                                             0xffff
#define HWIO_HOST_SOC_A_WFSS_CE1_SRC_WR_INDEX_IN                    \
                in_dword(HWIO_HOST_SOC_A_WFSS_CE1_SRC_WR_INDEX_ADDR)
#define HWIO_HOST_SOC_A_WFSS_CE1_SRC_WR_INDEX_INM(m)            \
                in_dword_masked(HWIO_HOST_SOC_A_WFSS_CE1_SRC_WR_INDEX_ADDR, m)
#define HWIO_HOST_SOC_A_WFSS_CE1_SRC_WR_INDEX_OUT(v)            \
                out_dword(HWIO_HOST_SOC_A_WFSS_CE1_SRC_WR_INDEX_ADDR,v)
#define HWIO_HOST_SOC_A_WFSS_CE1_SRC_WR_INDEX_OUTM(m,v) \
                out_dword_masked_ns(HWIO_HOST_SOC_A_WFSS_CE1_SRC_WR_INDEX_ADDR,m,v,HWIO_HOST_SOC_A_WFSS_CE1_SRC_WR_INDEX_IN)
#define HWIO_HOST_SOC_A_WFSS_CE1_SRC_WR_INDEX_SRC_WR_INDEX_BMSK                                                0xffff
#define HWIO_HOST_SOC_A_WFSS_CE1_SRC_WR_INDEX_SRC_WR_INDEX_SHFT                                                     0

#define HWIO_HOST_SOC_A_WFSS_CE1_DST_WR_INDEX_ADDR                                                         (HOST_SOC_A_WFSS_CE1_WFSS_CE_1_REG_REG_BASE      + 0x40)
#define HWIO_HOST_SOC_A_WFSS_CE1_DST_WR_INDEX_RMSK                                                             0xffff
#define HWIO_HOST_SOC_A_WFSS_CE1_DST_WR_INDEX_IN                    \
                in_dword(HWIO_HOST_SOC_A_WFSS_CE1_DST_WR_INDEX_ADDR)
#define HWIO_HOST_SOC_A_WFSS_CE1_DST_WR_INDEX_INM(m)            \
                in_dword_masked(HWIO_HOST_SOC_A_WFSS_CE1_DST_WR_INDEX_ADDR, m)
#define HWIO_HOST_SOC_A_WFSS_CE1_DST_WR_INDEX_OUT(v)            \
                out_dword(HWIO_HOST_SOC_A_WFSS_CE1_DST_WR_INDEX_ADDR,v)
#define HWIO_HOST_SOC_A_WFSS_CE1_DST_WR_INDEX_OUTM(m,v) \
                out_dword_masked_ns(HWIO_HOST_SOC_A_WFSS_CE1_DST_WR_INDEX_ADDR,m,v,HWIO_HOST_SOC_A_WFSS_CE1_DST_WR_INDEX_IN)
#define HWIO_HOST_SOC_A_WFSS_CE1_DST_WR_INDEX_DST_WR_INDEX_BMSK                                                0xffff
#define HWIO_HOST_SOC_A_WFSS_CE1_DST_WR_INDEX_DST_WR_INDEX_SHFT                                                     0

#define HWIO_HOST_SOC_A_WFSS_CE1_CURRENT_SRRI_ADDR                                                         (HOST_SOC_A_WFSS_CE1_WFSS_CE_1_REG_REG_BASE      + 0x44)
#define HWIO_HOST_SOC_A_WFSS_CE1_CURRENT_SRRI_RMSK                                                             0xffff
#define HWIO_HOST_SOC_A_WFSS_CE1_CURRENT_SRRI_IN                    \
                in_dword(HWIO_HOST_SOC_A_WFSS_CE1_CURRENT_SRRI_ADDR)
#define HWIO_HOST_SOC_A_WFSS_CE1_CURRENT_SRRI_INM(m)            \
                in_dword_masked(HWIO_HOST_SOC_A_WFSS_CE1_CURRENT_SRRI_ADDR, m)
#define HWIO_HOST_SOC_A_WFSS_CE1_CURRENT_SRRI_CURRENT_SRRI_BMSK                                                0xffff
#define HWIO_HOST_SOC_A_WFSS_CE1_CURRENT_SRRI_CURRENT_SRRI_SHFT                                                     0

#define HWIO_HOST_SOC_A_WFSS_CE1_CURRENT_DRRI_ADDR                                                         (HOST_SOC_A_WFSS_CE1_WFSS_CE_1_REG_REG_BASE      + 0x48)
#define HWIO_HOST_SOC_A_WFSS_CE1_CURRENT_DRRI_RMSK                                                             0xffff
#define HWIO_HOST_SOC_A_WFSS_CE1_CURRENT_DRRI_IN                    \
                in_dword(HWIO_HOST_SOC_A_WFSS_CE1_CURRENT_DRRI_ADDR)
#define HWIO_HOST_SOC_A_WFSS_CE1_CURRENT_DRRI_INM(m)            \
                in_dword_masked(HWIO_HOST_SOC_A_WFSS_CE1_CURRENT_DRRI_ADDR, m)
#define HWIO_HOST_SOC_A_WFSS_CE1_CURRENT_DRRI_CURRENT_DRRI_BMSK                                                0xffff
#define HWIO_HOST_SOC_A_WFSS_CE1_CURRENT_DRRI_CURRENT_DRRI_SHFT                                                     0

#define HWIO_HOST_SOC_A_WFSS_CE1_SRC_WATERMARK_ADDR                                                        (HOST_SOC_A_WFSS_CE1_WFSS_CE_1_REG_REG_BASE      + 0x4c)
#define HWIO_HOST_SOC_A_WFSS_CE1_SRC_WATERMARK_RMSK                                                        0xffffffff
#define HWIO_HOST_SOC_A_WFSS_CE1_SRC_WATERMARK_IN                    \
                in_dword(HWIO_HOST_SOC_A_WFSS_CE1_SRC_WATERMARK_ADDR)
#define HWIO_HOST_SOC_A_WFSS_CE1_SRC_WATERMARK_INM(m)            \
                in_dword_masked(HWIO_HOST_SOC_A_WFSS_CE1_SRC_WATERMARK_ADDR, m)
#define HWIO_HOST_SOC_A_WFSS_CE1_SRC_WATERMARK_OUT(v)            \
                out_dword(HWIO_HOST_SOC_A_WFSS_CE1_SRC_WATERMARK_ADDR,v)
#define HWIO_HOST_SOC_A_WFSS_CE1_SRC_WATERMARK_OUTM(m,v) \
                out_dword_masked_ns(HWIO_HOST_SOC_A_WFSS_CE1_SRC_WATERMARK_ADDR,m,v,HWIO_HOST_SOC_A_WFSS_CE1_SRC_WATERMARK_IN)
#define HWIO_HOST_SOC_A_WFSS_CE1_SRC_WATERMARK_SR_LOW_WATER_MARK_THRESOLD_BMSK                             0xffff0000
#define HWIO_HOST_SOC_A_WFSS_CE1_SRC_WATERMARK_SR_LOW_WATER_MARK_THRESOLD_SHFT                                     16
#define HWIO_HOST_SOC_A_WFSS_CE1_SRC_WATERMARK_SR_HIGH_WATER_MARK_THRESHOLD_BMSK                               0xffff
#define HWIO_HOST_SOC_A_WFSS_CE1_SRC_WATERMARK_SR_HIGH_WATER_MARK_THRESHOLD_SHFT                                    0

#define HWIO_HOST_SOC_A_WFSS_CE1_DST_WATERMARK_ADDR                                                        (HOST_SOC_A_WFSS_CE1_WFSS_CE_1_REG_REG_BASE      + 0x50)
#define HWIO_HOST_SOC_A_WFSS_CE1_DST_WATERMARK_RMSK                                                        0xffffffff
#define HWIO_HOST_SOC_A_WFSS_CE1_DST_WATERMARK_IN                    \
                in_dword(HWIO_HOST_SOC_A_WFSS_CE1_DST_WATERMARK_ADDR)
#define HWIO_HOST_SOC_A_WFSS_CE1_DST_WATERMARK_INM(m)            \
                in_dword_masked(HWIO_HOST_SOC_A_WFSS_CE1_DST_WATERMARK_ADDR, m)
#define HWIO_HOST_SOC_A_WFSS_CE1_DST_WATERMARK_OUT(v)            \
                out_dword(HWIO_HOST_SOC_A_WFSS_CE1_DST_WATERMARK_ADDR,v)
#define HWIO_HOST_SOC_A_WFSS_CE1_DST_WATERMARK_OUTM(m,v) \
                out_dword_masked_ns(HWIO_HOST_SOC_A_WFSS_CE1_DST_WATERMARK_ADDR,m,v,HWIO_HOST_SOC_A_WFSS_CE1_DST_WATERMARK_IN)
#define HWIO_HOST_SOC_A_WFSS_CE1_DST_WATERMARK_DR_LOW_WATER_MARK_THRESHOLD_BMSK                            0xffff0000
#define HWIO_HOST_SOC_A_WFSS_CE1_DST_WATERMARK_DR_LOW_WATER_MARK_THRESHOLD_SHFT                                    16
#define HWIO_HOST_SOC_A_WFSS_CE1_DST_WATERMARK_DR_HIGH_WATER_MARK_THRESHOLD_BMSK                               0xffff
#define HWIO_HOST_SOC_A_WFSS_CE1_DST_WATERMARK_DR_HIGH_WATER_MARK_THRESHOLD_SHFT                                    0

#define HWIO_HOST_SOC_A_WFSS_CE1_CE_CHANNEL_PRIORITY_ADDR                                                  (HOST_SOC_A_WFSS_CE1_WFSS_CE_1_REG_REG_BASE      + 0x54)
#define HWIO_HOST_SOC_A_WFSS_CE1_CE_CHANNEL_PRIORITY_RMSK                                                         0x7
#define HWIO_HOST_SOC_A_WFSS_CE1_CE_CHANNEL_PRIORITY_IN                    \
                in_dword(HWIO_HOST_SOC_A_WFSS_CE1_CE_CHANNEL_PRIORITY_ADDR)
#define HWIO_HOST_SOC_A_WFSS_CE1_CE_CHANNEL_PRIORITY_INM(m)            \
                in_dword_masked(HWIO_HOST_SOC_A_WFSS_CE1_CE_CHANNEL_PRIORITY_ADDR, m)
#define HWIO_HOST_SOC_A_WFSS_CE1_CE_CHANNEL_PRIORITY_OUT(v)            \
                out_dword(HWIO_HOST_SOC_A_WFSS_CE1_CE_CHANNEL_PRIORITY_ADDR,v)
#define HWIO_HOST_SOC_A_WFSS_CE1_CE_CHANNEL_PRIORITY_OUTM(m,v) \
                out_dword_masked_ns(HWIO_HOST_SOC_A_WFSS_CE1_CE_CHANNEL_PRIORITY_ADDR,m,v,HWIO_HOST_SOC_A_WFSS_CE1_CE_CHANNEL_PRIORITY_IN)
#define HWIO_HOST_SOC_A_WFSS_CE1_CE_CHANNEL_PRIORITY_CE_DST_OVERFLOW_MASK_BMSK                                    0x4
#define HWIO_HOST_SOC_A_WFSS_CE1_CE_CHANNEL_PRIORITY_CE_DST_OVERFLOW_MASK_SHFT                                      2
#define HWIO_HOST_SOC_A_WFSS_CE1_CE_CHANNEL_PRIORITY_CE_SRC_OVERFLOW_MASK_BMSK                                    0x2
#define HWIO_HOST_SOC_A_WFSS_CE1_CE_CHANNEL_PRIORITY_CE_SRC_OVERFLOW_MASK_SHFT                                      1
#define HWIO_HOST_SOC_A_WFSS_CE1_CE_CHANNEL_PRIORITY_CE_PRIORITY_BMSK                                             0x1
#define HWIO_HOST_SOC_A_WFSS_CE1_CE_CHANNEL_PRIORITY_CE_PRIORITY_SHFT                                               0

#define HWIO_HOST_SOC_A_WFSS_CE1_HOST_MSI_ADDR_LOW_ADDR                                                    (HOST_SOC_A_WFSS_CE1_WFSS_CE_1_REG_REG_BASE      + 0x58)
#define HWIO_HOST_SOC_A_WFSS_CE1_HOST_MSI_ADDR_LOW_RMSK                                                    0xffffffff
#define HWIO_HOST_SOC_A_WFSS_CE1_HOST_MSI_ADDR_LOW_IN                    \
                in_dword(HWIO_HOST_SOC_A_WFSS_CE1_HOST_MSI_ADDR_LOW_ADDR)
#define HWIO_HOST_SOC_A_WFSS_CE1_HOST_MSI_ADDR_LOW_INM(m)            \
                in_dword_masked(HWIO_HOST_SOC_A_WFSS_CE1_HOST_MSI_ADDR_LOW_ADDR, m)
#define HWIO_HOST_SOC_A_WFSS_CE1_HOST_MSI_ADDR_LOW_OUT(v)            \
                out_dword(HWIO_HOST_SOC_A_WFSS_CE1_HOST_MSI_ADDR_LOW_ADDR,v)
#define HWIO_HOST_SOC_A_WFSS_CE1_HOST_MSI_ADDR_LOW_OUTM(m,v) \
                out_dword_masked_ns(HWIO_HOST_SOC_A_WFSS_CE1_HOST_MSI_ADDR_LOW_ADDR,m,v,HWIO_HOST_SOC_A_WFSS_CE1_HOST_MSI_ADDR_LOW_IN)
#define HWIO_HOST_SOC_A_WFSS_CE1_HOST_MSI_ADDR_LOW_MSI_ADDR_LOW_BMSK                                       0xffffffff
#define HWIO_HOST_SOC_A_WFSS_CE1_HOST_MSI_ADDR_LOW_MSI_ADDR_LOW_SHFT                                                0

#define HWIO_HOST_SOC_A_WFSS_CE1_HOST_MSI_ADDR_HIGH_ADDR                                                   (HOST_SOC_A_WFSS_CE1_WFSS_CE_1_REG_REG_BASE      + 0x5c)
#define HWIO_HOST_SOC_A_WFSS_CE1_HOST_MSI_ADDR_HIGH_RMSK                                                         0xff
#define HWIO_HOST_SOC_A_WFSS_CE1_HOST_MSI_ADDR_HIGH_IN                    \
                in_dword(HWIO_HOST_SOC_A_WFSS_CE1_HOST_MSI_ADDR_HIGH_ADDR)
#define HWIO_HOST_SOC_A_WFSS_CE1_HOST_MSI_ADDR_HIGH_INM(m)            \
                in_dword_masked(HWIO_HOST_SOC_A_WFSS_CE1_HOST_MSI_ADDR_HIGH_ADDR, m)
#define HWIO_HOST_SOC_A_WFSS_CE1_HOST_MSI_ADDR_HIGH_OUT(v)            \
                out_dword(HWIO_HOST_SOC_A_WFSS_CE1_HOST_MSI_ADDR_HIGH_ADDR,v)
#define HWIO_HOST_SOC_A_WFSS_CE1_HOST_MSI_ADDR_HIGH_OUTM(m,v) \
                out_dword_masked_ns(HWIO_HOST_SOC_A_WFSS_CE1_HOST_MSI_ADDR_HIGH_ADDR,m,v,HWIO_HOST_SOC_A_WFSS_CE1_HOST_MSI_ADDR_HIGH_IN)
#define HWIO_HOST_SOC_A_WFSS_CE1_HOST_MSI_ADDR_HIGH_MSI_ADDR_HIGH_BMSK                                           0xff
#define HWIO_HOST_SOC_A_WFSS_CE1_HOST_MSI_ADDR_HIGH_MSI_ADDR_HIGH_SHFT                                              0

#define HWIO_HOST_SOC_A_WFSS_CE1_HOST_MSI_DATA_ADDR                                                        (HOST_SOC_A_WFSS_CE1_WFSS_CE_1_REG_REG_BASE      + 0x60)
#define HWIO_HOST_SOC_A_WFSS_CE1_HOST_MSI_DATA_RMSK                                                        0xffffffff
#define HWIO_HOST_SOC_A_WFSS_CE1_HOST_MSI_DATA_IN                    \
                in_dword(HWIO_HOST_SOC_A_WFSS_CE1_HOST_MSI_DATA_ADDR)
#define HWIO_HOST_SOC_A_WFSS_CE1_HOST_MSI_DATA_INM(m)            \
                in_dword_masked(HWIO_HOST_SOC_A_WFSS_CE1_HOST_MSI_DATA_ADDR, m)
#define HWIO_HOST_SOC_A_WFSS_CE1_HOST_MSI_DATA_OUT(v)            \
                out_dword(HWIO_HOST_SOC_A_WFSS_CE1_HOST_MSI_DATA_ADDR,v)
#define HWIO_HOST_SOC_A_WFSS_CE1_HOST_MSI_DATA_OUTM(m,v) \
                out_dword_masked_ns(HWIO_HOST_SOC_A_WFSS_CE1_HOST_MSI_DATA_ADDR,m,v,HWIO_HOST_SOC_A_WFSS_CE1_HOST_MSI_DATA_IN)
#define HWIO_HOST_SOC_A_WFSS_CE1_HOST_MSI_DATA_MSI_DATA_BMSK                                               0xffffffff
#define HWIO_HOST_SOC_A_WFSS_CE1_HOST_MSI_DATA_MSI_DATA_SHFT                                                        0

#define HWIO_HOST_SOC_A_WFSS_CE1_CE_OBFF_TIMEOUT_ADDR                                                      (HOST_SOC_A_WFSS_CE1_WFSS_CE_1_REG_REG_BASE      + 0x64)
#define HWIO_HOST_SOC_A_WFSS_CE1_CE_OBFF_TIMEOUT_RMSK                                                          0xffff
#define HWIO_HOST_SOC_A_WFSS_CE1_CE_OBFF_TIMEOUT_IN                    \
                in_dword(HWIO_HOST_SOC_A_WFSS_CE1_CE_OBFF_TIMEOUT_ADDR)
#define HWIO_HOST_SOC_A_WFSS_CE1_CE_OBFF_TIMEOUT_INM(m)            \
                in_dword_masked(HWIO_HOST_SOC_A_WFSS_CE1_CE_OBFF_TIMEOUT_ADDR, m)
#define HWIO_HOST_SOC_A_WFSS_CE1_CE_OBFF_TIMEOUT_OUT(v)            \
                out_dword(HWIO_HOST_SOC_A_WFSS_CE1_CE_OBFF_TIMEOUT_ADDR,v)
#define HWIO_HOST_SOC_A_WFSS_CE1_CE_OBFF_TIMEOUT_OUTM(m,v) \
                out_dword_masked_ns(HWIO_HOST_SOC_A_WFSS_CE1_CE_OBFF_TIMEOUT_ADDR,m,v,HWIO_HOST_SOC_A_WFSS_CE1_CE_OBFF_TIMEOUT_IN)
#define HWIO_HOST_SOC_A_WFSS_CE1_CE_OBFF_TIMEOUT_OBFF_TOLERANCE_TIMEOUT_BMSK                                   0xffff
#define HWIO_HOST_SOC_A_WFSS_CE1_CE_OBFF_TIMEOUT_OBFF_TOLERANCE_TIMEOUT_SHFT                                        0

#define HWIO_HOST_SOC_A_WFSS_CE1_CE_CHANNEL_OBFF_CFG_ADDR                                                  (HOST_SOC_A_WFSS_CE1_WFSS_CE_1_REG_REG_BASE      + 0x68)
#define HWIO_HOST_SOC_A_WFSS_CE1_CE_CHANNEL_OBFF_CFG_RMSK                                                         0x3
#define HWIO_HOST_SOC_A_WFSS_CE1_CE_CHANNEL_OBFF_CFG_IN                    \
                in_dword(HWIO_HOST_SOC_A_WFSS_CE1_CE_CHANNEL_OBFF_CFG_ADDR)
#define HWIO_HOST_SOC_A_WFSS_CE1_CE_CHANNEL_OBFF_CFG_INM(m)            \
                in_dword_masked(HWIO_HOST_SOC_A_WFSS_CE1_CE_CHANNEL_OBFF_CFG_ADDR, m)
#define HWIO_HOST_SOC_A_WFSS_CE1_CE_CHANNEL_OBFF_CFG_OUT(v)            \
                out_dword(HWIO_HOST_SOC_A_WFSS_CE1_CE_CHANNEL_OBFF_CFG_ADDR,v)
#define HWIO_HOST_SOC_A_WFSS_CE1_CE_CHANNEL_OBFF_CFG_OUTM(m,v) \
                out_dword_masked_ns(HWIO_HOST_SOC_A_WFSS_CE1_CE_CHANNEL_OBFF_CFG_ADDR,m,v,HWIO_HOST_SOC_A_WFSS_CE1_CE_CHANNEL_OBFF_CFG_IN)
#define HWIO_HOST_SOC_A_WFSS_CE1_CE_CHANNEL_OBFF_CFG_OBFF_CE_FLUSH_REQ_BMSK                                       0x2
#define HWIO_HOST_SOC_A_WFSS_CE1_CE_CHANNEL_OBFF_CFG_OBFF_CE_FLUSH_REQ_SHFT                                         1
#define HWIO_HOST_SOC_A_WFSS_CE1_CE_CHANNEL_OBFF_CFG_OBFF_CE_AUTO_PRIORITY_DISABLE_BMSK                           0x1
#define HWIO_HOST_SOC_A_WFSS_CE1_CE_CHANNEL_OBFF_CFG_OBFF_CE_AUTO_PRIORITY_DISABLE_SHFT                             0

#define HWIO_HOST_SOC_A_WFSS_CE1_CE_WATCHDOG_ADDR                                                          (HOST_SOC_A_WFSS_CE1_WFSS_CE_1_REG_REG_BASE      + 0x6c)
#define HWIO_HOST_SOC_A_WFSS_CE1_CE_WATCHDOG_RMSK                                                          0xffffffff
#define HWIO_HOST_SOC_A_WFSS_CE1_CE_WATCHDOG_IN                    \
                in_dword(HWIO_HOST_SOC_A_WFSS_CE1_CE_WATCHDOG_ADDR)
#define HWIO_HOST_SOC_A_WFSS_CE1_CE_WATCHDOG_INM(m)            \
                in_dword_masked(HWIO_HOST_SOC_A_WFSS_CE1_CE_WATCHDOG_ADDR, m)
#define HWIO_HOST_SOC_A_WFSS_CE1_CE_WATCHDOG_OUT(v)            \
                out_dword(HWIO_HOST_SOC_A_WFSS_CE1_CE_WATCHDOG_ADDR,v)
#define HWIO_HOST_SOC_A_WFSS_CE1_CE_WATCHDOG_OUTM(m,v) \
                out_dword_masked_ns(HWIO_HOST_SOC_A_WFSS_CE1_CE_WATCHDOG_ADDR,m,v,HWIO_HOST_SOC_A_WFSS_CE1_CE_WATCHDOG_IN)
#define HWIO_HOST_SOC_A_WFSS_CE1_CE_WATCHDOG_STATUS_BMSK                                                   0xffff0000
#define HWIO_HOST_SOC_A_WFSS_CE1_CE_WATCHDOG_STATUS_SHFT                                                           16
#define HWIO_HOST_SOC_A_WFSS_CE1_CE_WATCHDOG_LIMIT_BMSK                                                        0xffff
#define HWIO_HOST_SOC_A_WFSS_CE1_CE_WATCHDOG_LIMIT_SHFT                                                             0

#define HWIO_HOST_SOC_A_WFSS_CE1_CE_CSM_DBG_ADDR                                                           (HOST_SOC_A_WFSS_CE1_WFSS_CE_1_REG_REG_BASE      + 0x70)
#define HWIO_HOST_SOC_A_WFSS_CE1_CE_CSM_DBG_RMSK                                                           0xffffffff
#define HWIO_HOST_SOC_A_WFSS_CE1_CE_CSM_DBG_IN                    \
                in_dword(HWIO_HOST_SOC_A_WFSS_CE1_CE_CSM_DBG_ADDR)
#define HWIO_HOST_SOC_A_WFSS_CE1_CE_CSM_DBG_INM(m)            \
                in_dword_masked(HWIO_HOST_SOC_A_WFSS_CE1_CE_CSM_DBG_ADDR, m)
#define HWIO_HOST_SOC_A_WFSS_CE1_CE_CSM_DBG_VALUE_BMSK                                                     0xffffffff
#define HWIO_HOST_SOC_A_WFSS_CE1_CE_CSM_DBG_VALUE_SHFT                                                              0

#define HWIO_HOST_SOC_A_WFSS_CE1_CE_SRC_BATCH_TIMER_INT_SETUP_ADDR                                         (HOST_SOC_A_WFSS_CE1_WFSS_CE_1_REG_REG_BASE      + 0x74)
#define HWIO_HOST_SOC_A_WFSS_CE1_CE_SRC_BATCH_TIMER_INT_SETUP_RMSK                                         0xffffffff
#define HWIO_HOST_SOC_A_WFSS_CE1_CE_SRC_BATCH_TIMER_INT_SETUP_IN                    \
                in_dword(HWIO_HOST_SOC_A_WFSS_CE1_CE_SRC_BATCH_TIMER_INT_SETUP_ADDR)
#define HWIO_HOST_SOC_A_WFSS_CE1_CE_SRC_BATCH_TIMER_INT_SETUP_INM(m)            \
                in_dword_masked(HWIO_HOST_SOC_A_WFSS_CE1_CE_SRC_BATCH_TIMER_INT_SETUP_ADDR, m)
#define HWIO_HOST_SOC_A_WFSS_CE1_CE_SRC_BATCH_TIMER_INT_SETUP_OUT(v)            \
                out_dword(HWIO_HOST_SOC_A_WFSS_CE1_CE_SRC_BATCH_TIMER_INT_SETUP_ADDR,v)
#define HWIO_HOST_SOC_A_WFSS_CE1_CE_SRC_BATCH_TIMER_INT_SETUP_OUTM(m,v) \
                out_dword_masked_ns(HWIO_HOST_SOC_A_WFSS_CE1_CE_SRC_BATCH_TIMER_INT_SETUP_ADDR,m,v,HWIO_HOST_SOC_A_WFSS_CE1_CE_SRC_BATCH_TIMER_INT_SETUP_IN)
#define HWIO_HOST_SOC_A_WFSS_CE1_CE_SRC_BATCH_TIMER_INT_SETUP_INTERRUPT_TIMER_THRESHOLD_BMSK               0xffff0000
#define HWIO_HOST_SOC_A_WFSS_CE1_CE_SRC_BATCH_TIMER_INT_SETUP_INTERRUPT_TIMER_THRESHOLD_SHFT                       16
#define HWIO_HOST_SOC_A_WFSS_CE1_CE_SRC_BATCH_TIMER_INT_SETUP_SW_INTERRUPT_MODE_BMSK                           0x8000
#define HWIO_HOST_SOC_A_WFSS_CE1_CE_SRC_BATCH_TIMER_INT_SETUP_SW_INTERRUPT_MODE_SHFT                               15
#define HWIO_HOST_SOC_A_WFSS_CE1_CE_SRC_BATCH_TIMER_INT_SETUP_BATCH_COUNTER_THRESHOLD_BMSK                     0x7fff
#define HWIO_HOST_SOC_A_WFSS_CE1_CE_SRC_BATCH_TIMER_INT_SETUP_BATCH_COUNTER_THRESHOLD_SHFT                          0

#define HWIO_HOST_SOC_A_WFSS_CE1_CE_SRC_BATCH_TIMER_INT_STATUS_ADDR                                        (HOST_SOC_A_WFSS_CE1_WFSS_CE_1_REG_REG_BASE      + 0x78)
#define HWIO_HOST_SOC_A_WFSS_CE1_CE_SRC_BATCH_TIMER_INT_STATUS_RMSK                                        0xffffffff
#define HWIO_HOST_SOC_A_WFSS_CE1_CE_SRC_BATCH_TIMER_INT_STATUS_IN                    \
                in_dword(HWIO_HOST_SOC_A_WFSS_CE1_CE_SRC_BATCH_TIMER_INT_STATUS_ADDR)
#define HWIO_HOST_SOC_A_WFSS_CE1_CE_SRC_BATCH_TIMER_INT_STATUS_INM(m)            \
                in_dword_masked(HWIO_HOST_SOC_A_WFSS_CE1_CE_SRC_BATCH_TIMER_INT_STATUS_ADDR, m)
#define HWIO_HOST_SOC_A_WFSS_CE1_CE_SRC_BATCH_TIMER_INT_STATUS_CURRENT_INTERRUPT_TIMER_VALUE_BMSK          0xffff0000
#define HWIO_HOST_SOC_A_WFSS_CE1_CE_SRC_BATCH_TIMER_INT_STATUS_CURRENT_INTERRUPT_TIMER_VALUE_SHFT                  16
#define HWIO_HOST_SOC_A_WFSS_CE1_CE_SRC_BATCH_TIMER_INT_STATUS_CURRENT_SW_INT_WIRE_VALUE_BMSK                  0x8000
#define HWIO_HOST_SOC_A_WFSS_CE1_CE_SRC_BATCH_TIMER_INT_STATUS_CURRENT_SW_INT_WIRE_VALUE_SHFT                      15
#define HWIO_HOST_SOC_A_WFSS_CE1_CE_SRC_BATCH_TIMER_INT_STATUS_INTERNAL_BATCH_COUNTER_VALUE_BMSK               0x7fff
#define HWIO_HOST_SOC_A_WFSS_CE1_CE_SRC_BATCH_TIMER_INT_STATUS_INTERNAL_BATCH_COUNTER_VALUE_SHFT                    0

#define HWIO_HOST_SOC_A_WFSS_CE1_CE_DST_BATCH_TIMER_INT_SETUP_ADDR                                         (HOST_SOC_A_WFSS_CE1_WFSS_CE_1_REG_REG_BASE      + 0x7c)
#define HWIO_HOST_SOC_A_WFSS_CE1_CE_DST_BATCH_TIMER_INT_SETUP_RMSK                                         0xffffffff
#define HWIO_HOST_SOC_A_WFSS_CE1_CE_DST_BATCH_TIMER_INT_SETUP_IN                    \
                in_dword(HWIO_HOST_SOC_A_WFSS_CE1_CE_DST_BATCH_TIMER_INT_SETUP_ADDR)
#define HWIO_HOST_SOC_A_WFSS_CE1_CE_DST_BATCH_TIMER_INT_SETUP_INM(m)            \
                in_dword_masked(HWIO_HOST_SOC_A_WFSS_CE1_CE_DST_BATCH_TIMER_INT_SETUP_ADDR, m)
#define HWIO_HOST_SOC_A_WFSS_CE1_CE_DST_BATCH_TIMER_INT_SETUP_OUT(v)            \
                out_dword(HWIO_HOST_SOC_A_WFSS_CE1_CE_DST_BATCH_TIMER_INT_SETUP_ADDR,v)
#define HWIO_HOST_SOC_A_WFSS_CE1_CE_DST_BATCH_TIMER_INT_SETUP_OUTM(m,v) \
                out_dword_masked_ns(HWIO_HOST_SOC_A_WFSS_CE1_CE_DST_BATCH_TIMER_INT_SETUP_ADDR,m,v,HWIO_HOST_SOC_A_WFSS_CE1_CE_DST_BATCH_TIMER_INT_SETUP_IN)
#define HWIO_HOST_SOC_A_WFSS_CE1_CE_DST_BATCH_TIMER_INT_SETUP_INTERRUPT_TIMER_THRESHOLD_BMSK               0xffff0000
#define HWIO_HOST_SOC_A_WFSS_CE1_CE_DST_BATCH_TIMER_INT_SETUP_INTERRUPT_TIMER_THRESHOLD_SHFT                       16
#define HWIO_HOST_SOC_A_WFSS_CE1_CE_DST_BATCH_TIMER_INT_SETUP_SW_INTERRUPT_MODE_BMSK                           0x8000
#define HWIO_HOST_SOC_A_WFSS_CE1_CE_DST_BATCH_TIMER_INT_SETUP_SW_INTERRUPT_MODE_SHFT                               15
#define HWIO_HOST_SOC_A_WFSS_CE1_CE_DST_BATCH_TIMER_INT_SETUP_BATCH_COUNTER_THRESHOLD_BMSK                     0x7fff
#define HWIO_HOST_SOC_A_WFSS_CE1_CE_DST_BATCH_TIMER_INT_SETUP_BATCH_COUNTER_THRESHOLD_SHFT                          0

#define HWIO_HOST_SOC_A_WFSS_CE1_CE_DST_BATCH_TIMER_INT_STATUS_ADDR                                        (HOST_SOC_A_WFSS_CE1_WFSS_CE_1_REG_REG_BASE      + 0x80)
#define HWIO_HOST_SOC_A_WFSS_CE1_CE_DST_BATCH_TIMER_INT_STATUS_RMSK                                        0xffffffff
#define HWIO_HOST_SOC_A_WFSS_CE1_CE_DST_BATCH_TIMER_INT_STATUS_IN                    \
                in_dword(HWIO_HOST_SOC_A_WFSS_CE1_CE_DST_BATCH_TIMER_INT_STATUS_ADDR)
#define HWIO_HOST_SOC_A_WFSS_CE1_CE_DST_BATCH_TIMER_INT_STATUS_INM(m)            \
                in_dword_masked(HWIO_HOST_SOC_A_WFSS_CE1_CE_DST_BATCH_TIMER_INT_STATUS_ADDR, m)
#define HWIO_HOST_SOC_A_WFSS_CE1_CE_DST_BATCH_TIMER_INT_STATUS_CURRENT_INTERRUPT_TIMER_VALUE_BMSK          0xffff0000
#define HWIO_HOST_SOC_A_WFSS_CE1_CE_DST_BATCH_TIMER_INT_STATUS_CURRENT_INTERRUPT_TIMER_VALUE_SHFT                  16
#define HWIO_HOST_SOC_A_WFSS_CE1_CE_DST_BATCH_TIMER_INT_STATUS_CURRENT_SW_INT_WIRE_VALUE_BMSK                  0x8000
#define HWIO_HOST_SOC_A_WFSS_CE1_CE_DST_BATCH_TIMER_INT_STATUS_CURRENT_SW_INT_WIRE_VALUE_SHFT                      15
#define HWIO_HOST_SOC_A_WFSS_CE1_CE_DST_BATCH_TIMER_INT_STATUS_INTERNAL_BATCH_COUNTER_VALUE_BMSK               0x7fff
#define HWIO_HOST_SOC_A_WFSS_CE1_CE_DST_BATCH_TIMER_INT_STATUS_INTERNAL_BATCH_COUNTER_VALUE_SHFT                    0

#define HWIO_HOST_SOC_A_WFSS_CE1_TARGET_MSI_ADDR_LOW_ADDR                                                  (HOST_SOC_A_WFSS_CE1_WFSS_CE_1_REG_REG_BASE      + 0x84)
#define HWIO_HOST_SOC_A_WFSS_CE1_TARGET_MSI_ADDR_LOW_RMSK                                                  0xffffffff
#define HWIO_HOST_SOC_A_WFSS_CE1_TARGET_MSI_ADDR_LOW_IN                    \
                in_dword(HWIO_HOST_SOC_A_WFSS_CE1_TARGET_MSI_ADDR_LOW_ADDR)
#define HWIO_HOST_SOC_A_WFSS_CE1_TARGET_MSI_ADDR_LOW_INM(m)            \
                in_dword_masked(HWIO_HOST_SOC_A_WFSS_CE1_TARGET_MSI_ADDR_LOW_ADDR, m)
#define HWIO_HOST_SOC_A_WFSS_CE1_TARGET_MSI_ADDR_LOW_OUT(v)            \
                out_dword(HWIO_HOST_SOC_A_WFSS_CE1_TARGET_MSI_ADDR_LOW_ADDR,v)
#define HWIO_HOST_SOC_A_WFSS_CE1_TARGET_MSI_ADDR_LOW_OUTM(m,v) \
                out_dword_masked_ns(HWIO_HOST_SOC_A_WFSS_CE1_TARGET_MSI_ADDR_LOW_ADDR,m,v,HWIO_HOST_SOC_A_WFSS_CE1_TARGET_MSI_ADDR_LOW_IN)
#define HWIO_HOST_SOC_A_WFSS_CE1_TARGET_MSI_ADDR_LOW_MSI_ADDR_LOW_BMSK                                     0xffffffff
#define HWIO_HOST_SOC_A_WFSS_CE1_TARGET_MSI_ADDR_LOW_MSI_ADDR_LOW_SHFT                                              0

#define HWIO_HOST_SOC_A_WFSS_CE1_TARGET_MSI_ADDR_HIGH_ADDR                                                 (HOST_SOC_A_WFSS_CE1_WFSS_CE_1_REG_REG_BASE      + 0x88)
#define HWIO_HOST_SOC_A_WFSS_CE1_TARGET_MSI_ADDR_HIGH_RMSK                                                       0xff
#define HWIO_HOST_SOC_A_WFSS_CE1_TARGET_MSI_ADDR_HIGH_IN                    \
                in_dword(HWIO_HOST_SOC_A_WFSS_CE1_TARGET_MSI_ADDR_HIGH_ADDR)
#define HWIO_HOST_SOC_A_WFSS_CE1_TARGET_MSI_ADDR_HIGH_INM(m)            \
                in_dword_masked(HWIO_HOST_SOC_A_WFSS_CE1_TARGET_MSI_ADDR_HIGH_ADDR, m)
#define HWIO_HOST_SOC_A_WFSS_CE1_TARGET_MSI_ADDR_HIGH_OUT(v)            \
                out_dword(HWIO_HOST_SOC_A_WFSS_CE1_TARGET_MSI_ADDR_HIGH_ADDR,v)
#define HWIO_HOST_SOC_A_WFSS_CE1_TARGET_MSI_ADDR_HIGH_OUTM(m,v) \
                out_dword_masked_ns(HWIO_HOST_SOC_A_WFSS_CE1_TARGET_MSI_ADDR_HIGH_ADDR,m,v,HWIO_HOST_SOC_A_WFSS_CE1_TARGET_MSI_ADDR_HIGH_IN)
#define HWIO_HOST_SOC_A_WFSS_CE1_TARGET_MSI_ADDR_HIGH_MSI_ADDR_HIGH_BMSK                                         0xff
#define HWIO_HOST_SOC_A_WFSS_CE1_TARGET_MSI_ADDR_HIGH_MSI_ADDR_HIGH_SHFT                                            0

#define HWIO_HOST_SOC_A_WFSS_CE1_TARGET_MSI_DATA_ADDR                                                      (HOST_SOC_A_WFSS_CE1_WFSS_CE_1_REG_REG_BASE      + 0x8c)
#define HWIO_HOST_SOC_A_WFSS_CE1_TARGET_MSI_DATA_RMSK                                                      0xffffffff
#define HWIO_HOST_SOC_A_WFSS_CE1_TARGET_MSI_DATA_IN                    \
                in_dword(HWIO_HOST_SOC_A_WFSS_CE1_TARGET_MSI_DATA_ADDR)
#define HWIO_HOST_SOC_A_WFSS_CE1_TARGET_MSI_DATA_INM(m)            \
                in_dword_masked(HWIO_HOST_SOC_A_WFSS_CE1_TARGET_MSI_DATA_ADDR, m)
#define HWIO_HOST_SOC_A_WFSS_CE1_TARGET_MSI_DATA_OUT(v)            \
                out_dword(HWIO_HOST_SOC_A_WFSS_CE1_TARGET_MSI_DATA_ADDR,v)
#define HWIO_HOST_SOC_A_WFSS_CE1_TARGET_MSI_DATA_OUTM(m,v) \
                out_dword_masked_ns(HWIO_HOST_SOC_A_WFSS_CE1_TARGET_MSI_DATA_ADDR,m,v,HWIO_HOST_SOC_A_WFSS_CE1_TARGET_MSI_DATA_IN)
#define HWIO_HOST_SOC_A_WFSS_CE1_TARGET_MSI_DATA_MSI_DATA_BMSK                                             0xffffffff
#define HWIO_HOST_SOC_A_WFSS_CE1_TARGET_MSI_DATA_MSI_DATA_SHFT                                                      0

#define HOST_SOC_A_WFSS_CE_COMMON_WRAPPER_REG_BASE                                                  (HOST_SOC_WFSS_CE_REG_TOP_BASE      + 0x00010000)
#define HOST_SOC_A_WFSS_CE_COMMON_WRAPPER_REG_BASE_SIZE                                             0x1000
#define HOST_SOC_A_WFSS_CE_COMMON_WRAPPER_REG_BASE_USED                                             0x10

#define HWIO_HOST_SOC_A_WFSS_CE_COMMON_WRAPPER_CE_WRAPPER_HOST_INTERRUPT_SUMMARY_ADDR               (HOST_SOC_A_WFSS_CE_COMMON_WRAPPER_REG_BASE      + 0x0)
#define HWIO_HOST_SOC_A_WFSS_CE_COMMON_WRAPPER_CE_WRAPPER_HOST_INTERRUPT_SUMMARY_RMSK                 0xfff000
#define HWIO_HOST_SOC_A_WFSS_CE_COMMON_WRAPPER_CE_WRAPPER_HOST_INTERRUPT_SUMMARY_IN                    \
                in_dword(HWIO_HOST_SOC_A_WFSS_CE_COMMON_WRAPPER_CE_WRAPPER_HOST_INTERRUPT_SUMMARY_ADDR)
#define HWIO_HOST_SOC_A_WFSS_CE_COMMON_WRAPPER_CE_WRAPPER_HOST_INTERRUPT_SUMMARY_INM(m)            \
                in_dword_masked(HWIO_HOST_SOC_A_WFSS_CE_COMMON_WRAPPER_CE_WRAPPER_HOST_INTERRUPT_SUMMARY_ADDR, m)
#define HWIO_HOST_SOC_A_WFSS_CE_COMMON_WRAPPER_CE_WRAPPER_HOST_INTERRUPT_SUMMARY_HOST_BMSK            0xfff000
#define HWIO_HOST_SOC_A_WFSS_CE_COMMON_WRAPPER_CE_WRAPPER_HOST_INTERRUPT_SUMMARY_HOST_SHFT                  12

#define HWIO_HOST_SOC_A_WFSS_CE_COMMON_WRAPPER_CE_WRAPPER_INDEX_BASE_LOW_ADDR                       (HOST_SOC_A_WFSS_CE_COMMON_WRAPPER_REG_BASE      + 0x4)
#define HWIO_HOST_SOC_A_WFSS_CE_COMMON_WRAPPER_CE_WRAPPER_INDEX_BASE_LOW_RMSK                       0xffffffff
#define HWIO_HOST_SOC_A_WFSS_CE_COMMON_WRAPPER_CE_WRAPPER_INDEX_BASE_LOW_IN                    \
                in_dword(HWIO_HOST_SOC_A_WFSS_CE_COMMON_WRAPPER_CE_WRAPPER_INDEX_BASE_LOW_ADDR)
#define HWIO_HOST_SOC_A_WFSS_CE_COMMON_WRAPPER_CE_WRAPPER_INDEX_BASE_LOW_INM(m)            \
                in_dword_masked(HWIO_HOST_SOC_A_WFSS_CE_COMMON_WRAPPER_CE_WRAPPER_INDEX_BASE_LOW_ADDR, m)
#define HWIO_HOST_SOC_A_WFSS_CE_COMMON_WRAPPER_CE_WRAPPER_INDEX_BASE_LOW_OUT(v)            \
                out_dword(HWIO_HOST_SOC_A_WFSS_CE_COMMON_WRAPPER_CE_WRAPPER_INDEX_BASE_LOW_ADDR,v)
#define HWIO_HOST_SOC_A_WFSS_CE_COMMON_WRAPPER_CE_WRAPPER_INDEX_BASE_LOW_OUTM(m,v) \
                out_dword_masked_ns(HWIO_HOST_SOC_A_WFSS_CE_COMMON_WRAPPER_CE_WRAPPER_INDEX_BASE_LOW_ADDR,m,v,HWIO_HOST_SOC_A_WFSS_CE_COMMON_WRAPPER_CE_WRAPPER_INDEX_BASE_LOW_IN)
#define HWIO_HOST_SOC_A_WFSS_CE_COMMON_WRAPPER_CE_WRAPPER_INDEX_BASE_LOW_VAL_BMSK                   0xffffffff
#define HWIO_HOST_SOC_A_WFSS_CE_COMMON_WRAPPER_CE_WRAPPER_INDEX_BASE_LOW_VAL_SHFT                            0

#define HWIO_HOST_SOC_A_WFSS_CE_COMMON_WRAPPER_CE_WRAPPER_INDEX_BASE_HIGH_ADDR                      (HOST_SOC_A_WFSS_CE_COMMON_WRAPPER_REG_BASE      + 0x8)
#define HWIO_HOST_SOC_A_WFSS_CE_COMMON_WRAPPER_CE_WRAPPER_INDEX_BASE_HIGH_RMSK                            0xff
#define HWIO_HOST_SOC_A_WFSS_CE_COMMON_WRAPPER_CE_WRAPPER_INDEX_BASE_HIGH_IN                    \
                in_dword(HWIO_HOST_SOC_A_WFSS_CE_COMMON_WRAPPER_CE_WRAPPER_INDEX_BASE_HIGH_ADDR)
#define HWIO_HOST_SOC_A_WFSS_CE_COMMON_WRAPPER_CE_WRAPPER_INDEX_BASE_HIGH_INM(m)            \
                in_dword_masked(HWIO_HOST_SOC_A_WFSS_CE_COMMON_WRAPPER_CE_WRAPPER_INDEX_BASE_HIGH_ADDR, m)
#define HWIO_HOST_SOC_A_WFSS_CE_COMMON_WRAPPER_CE_WRAPPER_INDEX_BASE_HIGH_OUT(v)            \
                out_dword(HWIO_HOST_SOC_A_WFSS_CE_COMMON_WRAPPER_CE_WRAPPER_INDEX_BASE_HIGH_ADDR,v)
#define HWIO_HOST_SOC_A_WFSS_CE_COMMON_WRAPPER_CE_WRAPPER_INDEX_BASE_HIGH_OUTM(m,v) \
                out_dword_masked_ns(HWIO_HOST_SOC_A_WFSS_CE_COMMON_WRAPPER_CE_WRAPPER_INDEX_BASE_HIGH_ADDR,m,v,HWIO_HOST_SOC_A_WFSS_CE_COMMON_WRAPPER_CE_WRAPPER_INDEX_BASE_HIGH_IN)
#define HWIO_HOST_SOC_A_WFSS_CE_COMMON_WRAPPER_CE_WRAPPER_INDEX_BASE_HIGH_VAL_BMSK                        0xff
#define HWIO_HOST_SOC_A_WFSS_CE_COMMON_WRAPPER_CE_WRAPPER_INDEX_BASE_HIGH_VAL_SHFT                           0

#define HWIO_HOST_SOC_A_WFSS_CE_COMMON_WRAPPER_CE_WRAPPER_FW_INDEX_BASE_LOW_ADDR                    (HOST_SOC_A_WFSS_CE_COMMON_WRAPPER_REG_BASE      + 0xc)
#define HWIO_HOST_SOC_A_WFSS_CE_COMMON_WRAPPER_CE_WRAPPER_FW_INDEX_BASE_LOW_RMSK                    0xffffffff
#define HWIO_HOST_SOC_A_WFSS_CE_COMMON_WRAPPER_CE_WRAPPER_FW_INDEX_BASE_LOW_IN                    \
                in_dword(HWIO_HOST_SOC_A_WFSS_CE_COMMON_WRAPPER_CE_WRAPPER_FW_INDEX_BASE_LOW_ADDR)
#define HWIO_HOST_SOC_A_WFSS_CE_COMMON_WRAPPER_CE_WRAPPER_FW_INDEX_BASE_LOW_INM(m)            \
                in_dword_masked(HWIO_HOST_SOC_A_WFSS_CE_COMMON_WRAPPER_CE_WRAPPER_FW_INDEX_BASE_LOW_ADDR, m)
#define HWIO_HOST_SOC_A_WFSS_CE_COMMON_WRAPPER_CE_WRAPPER_FW_INDEX_BASE_LOW_OUT(v)            \
                out_dword(HWIO_HOST_SOC_A_WFSS_CE_COMMON_WRAPPER_CE_WRAPPER_FW_INDEX_BASE_LOW_ADDR,v)
#define HWIO_HOST_SOC_A_WFSS_CE_COMMON_WRAPPER_CE_WRAPPER_FW_INDEX_BASE_LOW_OUTM(m,v) \
                out_dword_masked_ns(HWIO_HOST_SOC_A_WFSS_CE_COMMON_WRAPPER_CE_WRAPPER_FW_INDEX_BASE_LOW_ADDR,m,v,HWIO_HOST_SOC_A_WFSS_CE_COMMON_WRAPPER_CE_WRAPPER_FW_INDEX_BASE_LOW_IN)
#define HWIO_HOST_SOC_A_WFSS_CE_COMMON_WRAPPER_CE_WRAPPER_FW_INDEX_BASE_LOW_VAL_BMSK                0xffffffff
#define HWIO_HOST_SOC_A_WFSS_CE_COMMON_WRAPPER_CE_WRAPPER_FW_INDEX_BASE_LOW_VAL_SHFT                         0

#define HWIO_HOST_SOC_A_WFSS_CE_COMMON_WRAPPER_CE_WRAPPER_FW_INDEX_BASE_HIGH_ADDR                   (HOST_SOC_A_WFSS_CE_COMMON_WRAPPER_REG_BASE      + 0x10)
#define HWIO_HOST_SOC_A_WFSS_CE_COMMON_WRAPPER_CE_WRAPPER_FW_INDEX_BASE_HIGH_RMSK                         0xff
#define HWIO_HOST_SOC_A_WFSS_CE_COMMON_WRAPPER_CE_WRAPPER_FW_INDEX_BASE_HIGH_IN                    \
                in_dword(HWIO_HOST_SOC_A_WFSS_CE_COMMON_WRAPPER_CE_WRAPPER_FW_INDEX_BASE_HIGH_ADDR)
#define HWIO_HOST_SOC_A_WFSS_CE_COMMON_WRAPPER_CE_WRAPPER_FW_INDEX_BASE_HIGH_INM(m)            \
                in_dword_masked(HWIO_HOST_SOC_A_WFSS_CE_COMMON_WRAPPER_CE_WRAPPER_FW_INDEX_BASE_HIGH_ADDR, m)
#define HWIO_HOST_SOC_A_WFSS_CE_COMMON_WRAPPER_CE_WRAPPER_FW_INDEX_BASE_HIGH_OUT(v)            \
                out_dword(HWIO_HOST_SOC_A_WFSS_CE_COMMON_WRAPPER_CE_WRAPPER_FW_INDEX_BASE_HIGH_ADDR,v)
#define HWIO_HOST_SOC_A_WFSS_CE_COMMON_WRAPPER_CE_WRAPPER_FW_INDEX_BASE_HIGH_OUTM(m,v) \
                out_dword_masked_ns(HWIO_HOST_SOC_A_WFSS_CE_COMMON_WRAPPER_CE_WRAPPER_FW_INDEX_BASE_HIGH_ADDR,m,v,HWIO_HOST_SOC_A_WFSS_CE_COMMON_WRAPPER_CE_WRAPPER_FW_INDEX_BASE_HIGH_IN)
#define HWIO_HOST_SOC_A_WFSS_CE_COMMON_WRAPPER_CE_WRAPPER_FW_INDEX_BASE_HIGH_VAL_BMSK                     0xff
#define HWIO_HOST_SOC_A_WFSS_CE_COMMON_WRAPPER_CE_WRAPPER_FW_INDEX_BASE_HIGH_VAL_SHFT                        0

#endif
