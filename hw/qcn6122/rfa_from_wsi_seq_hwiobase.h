/*
 * Copyright (c) 2020, The Linux Foundation. All rights reserved.
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

#ifndef __RFA_FROM_WSI_SEQ_BASE_H__
#define __RFA_FROM_WSI_SEQ_BASE_H__

#ifdef SCALE_INCLUDES
	#include "HALhwio.h"
#else
	#include "msmhwio.h"
#endif


///////////////////////////////////////////////////////////////////////////////////////////////
// Instance Relative Offsets from Block rfa_from_wsi
///////////////////////////////////////////////////////////////////////////////////////////////

#define SEQ_RFA_FROM_WSI_RFA_CMN_OFFSET                              0x00014000
#define SEQ_RFA_FROM_WSI_RFA_CMN_AON_OFFSET                          0x00014000
#define SEQ_RFA_FROM_WSI_RFA_CMN_AON_XFEM_OFFSET                     0x00014240
#define SEQ_RFA_FROM_WSI_RFA_CMN_AON_COEX_OFFSET                     0x000142c0
#define SEQ_RFA_FROM_WSI_RFA_CMN_RFFE_M_OFFSET                       0x00014300
#define SEQ_RFA_FROM_WSI_RFA_CMN_RFA_SHD_OTP_OFFSET                  0x00014400
#define SEQ_RFA_FROM_WSI_RFA_CMN_RFA_OTP_OFFSET                      0x00014480
#define SEQ_RFA_FROM_WSI_RFA_CMN_CLKGEN_OFFSET                       0x00014800
#define SEQ_RFA_FROM_WSI_RFA_CMN_WL_SYNTH0_BS_OFFSET                 0x00016000
#define SEQ_RFA_FROM_WSI_RFA_CMN_WL_SYNTH0_CLBS_OFFSET               0x00016040
#define SEQ_RFA_FROM_WSI_RFA_CMN_WL_SYNTH0_BIST_OFFSET               0x00016100
#define SEQ_RFA_FROM_WSI_RFA_CMN_WL_SYNTH0_PC_OFFSET                 0x00016140
#define SEQ_RFA_FROM_WSI_RFA_CMN_WL_SYNTH0_KVCO_OFFSET               0x00016180
#define SEQ_RFA_FROM_WSI_RFA_CMN_WL_SYNTH0_AC_OFFSET                 0x000161c0
#define SEQ_RFA_FROM_WSI_RFA_CMN_WL_SYNTH0_LO_OFFSET                 0x00016280
#define SEQ_RFA_FROM_WSI_RFA_CMN_HLS_WL_REGFILE_OFFSET               0x00017c00
#define SEQ_RFA_FROM_WSI_RFA_PMU_OFFSET                              0x0001a000
#define SEQ_RFA_FROM_WSI_RFA_PMU_PMU_OFFSET                          0x0001a000
#define SEQ_RFA_FROM_WSI_RFA_WL_OFFSET                               0x00020000
#define SEQ_RFA_FROM_WSI_RFA_WL_WL_MC_CH0_OFFSET                     0x00020000
#define SEQ_RFA_FROM_WSI_RFA_WL_WL_RXBB_CH0_OFFSET                   0x00020400
#define SEQ_RFA_FROM_WSI_RFA_WL_WL_TXBB_CH0_OFFSET                   0x00020800
#define SEQ_RFA_FROM_WSI_RFA_WL_WL_RXFE_CH0_OFFSET                   0x00021000
#define SEQ_RFA_FROM_WSI_RFA_WL_WL_TXFE_CH0_OFFSET                   0x00021300
#define SEQ_RFA_FROM_WSI_RFA_WL_WL_LO_PAL_CH0_OFFSET                 0x00021600
#define SEQ_RFA_FROM_WSI_RFA_WL_WL_LO_CH0_OFFSET                     0x00021640
#define SEQ_RFA_FROM_WSI_RFA_WL_WL_TPC_CH0_OFFSET                    0x00022000
#define SEQ_RFA_FROM_WSI_RFA_WL_WL_MEM_CH0_OFFSET                    0x00024000
#define SEQ_RFA_FROM_WSI_RFA_WL_WL_MC_CH1_OFFSET                     0x00028000
#define SEQ_RFA_FROM_WSI_RFA_WL_WL_RXBB_CH1_OFFSET                   0x00028400
#define SEQ_RFA_FROM_WSI_RFA_WL_WL_TXBB_CH1_OFFSET                   0x00028800
#define SEQ_RFA_FROM_WSI_RFA_WL_WL_RXFE_CH1_OFFSET                   0x00029000
#define SEQ_RFA_FROM_WSI_RFA_WL_WL_TXFE_CH1_OFFSET                   0x00029300
#define SEQ_RFA_FROM_WSI_RFA_WL_WL_LO_PAL_CH1_OFFSET                 0x00029600
#define SEQ_RFA_FROM_WSI_RFA_WL_WL_LO_CH1_OFFSET                     0x00029640
#define SEQ_RFA_FROM_WSI_RFA_WL_WL_TPC_CH1_OFFSET                    0x0002a000
#define SEQ_RFA_FROM_WSI_RFA_WL_WL_MEM_CH1_OFFSET                    0x0002c000


///////////////////////////////////////////////////////////////////////////////////////////////
// Instance Relative Offsets from Block rfa_cmn
///////////////////////////////////////////////////////////////////////////////////////////////

#define SEQ_RFA_CMN_AON_OFFSET                                       0x00000000
#define SEQ_RFA_CMN_AON_XFEM_OFFSET                                  0x00000240
#define SEQ_RFA_CMN_AON_COEX_OFFSET                                  0x000002c0
#define SEQ_RFA_CMN_RFFE_M_OFFSET                                    0x00000300
#define SEQ_RFA_CMN_RFA_SHD_OTP_OFFSET                               0x00000400
#define SEQ_RFA_CMN_RFA_OTP_OFFSET                                   0x00000480
#define SEQ_RFA_CMN_CLKGEN_OFFSET                                    0x00000800
#define SEQ_RFA_CMN_WL_SYNTH0_BS_OFFSET                              0x00002000
#define SEQ_RFA_CMN_WL_SYNTH0_CLBS_OFFSET                            0x00002040
#define SEQ_RFA_CMN_WL_SYNTH0_BIST_OFFSET                            0x00002100
#define SEQ_RFA_CMN_WL_SYNTH0_PC_OFFSET                              0x00002140
#define SEQ_RFA_CMN_WL_SYNTH0_KVCO_OFFSET                            0x00002180
#define SEQ_RFA_CMN_WL_SYNTH0_AC_OFFSET                              0x000021c0
#define SEQ_RFA_CMN_WL_SYNTH0_LO_OFFSET                              0x00002280
#define SEQ_RFA_CMN_HLS_WL_REGFILE_OFFSET                            0x00003c00


///////////////////////////////////////////////////////////////////////////////////////////////
// Instance Relative Offsets from Block rfa_pmu
///////////////////////////////////////////////////////////////////////////////////////////////

#define SEQ_RFA_PMU_PMU_OFFSET                                       0x00000000


///////////////////////////////////////////////////////////////////////////////////////////////
// Instance Relative Offsets from Block rfa_wl
///////////////////////////////////////////////////////////////////////////////////////////////

#define SEQ_RFA_WL_WL_MC_CH0_OFFSET                                  0x00000000
#define SEQ_RFA_WL_WL_RXBB_CH0_OFFSET                                0x00000400
#define SEQ_RFA_WL_WL_TXBB_CH0_OFFSET                                0x00000800
#define SEQ_RFA_WL_WL_RXFE_CH0_OFFSET                                0x00001000
#define SEQ_RFA_WL_WL_TXFE_CH0_OFFSET                                0x00001300
#define SEQ_RFA_WL_WL_LO_PAL_CH0_OFFSET                              0x00001600
#define SEQ_RFA_WL_WL_LO_CH0_OFFSET                                  0x00001640
#define SEQ_RFA_WL_WL_TPC_CH0_OFFSET                                 0x00002000
#define SEQ_RFA_WL_WL_MEM_CH0_OFFSET                                 0x00004000
#define SEQ_RFA_WL_WL_MC_CH1_OFFSET                                  0x00008000
#define SEQ_RFA_WL_WL_RXBB_CH1_OFFSET                                0x00008400
#define SEQ_RFA_WL_WL_TXBB_CH1_OFFSET                                0x00008800
#define SEQ_RFA_WL_WL_RXFE_CH1_OFFSET                                0x00009000
#define SEQ_RFA_WL_WL_TXFE_CH1_OFFSET                                0x00009300
#define SEQ_RFA_WL_WL_LO_PAL_CH1_OFFSET                              0x00009600
#define SEQ_RFA_WL_WL_LO_CH1_OFFSET                                  0x00009640
#define SEQ_RFA_WL_WL_TPC_CH1_OFFSET                                 0x0000a000
#define SEQ_RFA_WL_WL_MEM_CH1_OFFSET                                 0x0000c000


#endif

