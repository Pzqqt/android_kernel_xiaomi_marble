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

#ifndef __WCSS_SEQ_BASE_H__
#define __WCSS_SEQ_BASE_H__

#ifdef SCALE_INCLUDES
	#include "HALhwio.h"
#else
	#include "msmhwio.h"
#endif


///////////////////////////////////////////////////////////////////////////////////////////////
// Instance Relative Offsets from Block wcss
///////////////////////////////////////////////////////////////////////////////////////////////

#define SEQ_WCSS_UMAC_NOC_OFFSET                                     0x00140000
#define SEQ_WCSS_PHYA_OFFSET                                         0x00300000
#define SEQ_WCSS_PHYA_WFAX_PCSS_PDMEM_REG_MAP_OFFSET                 0x00300000
#define SEQ_WCSS_PHYA_WFAX_PCSS_REG_MAP_OFFSET                       0x00380000
#define SEQ_WCSS_PHYA_WFAX_PCSS_DMAC0_REG_MAP_OFFSET                 0x00380400
#define SEQ_WCSS_PHYA_WFAX_PCSS_DMAC1_REG_MAP_OFFSET                 0x00380800
#define SEQ_WCSS_PHYA_WFAX_PCSS_DMAC2_REG_MAP_OFFSET                 0x00380c00
#define SEQ_WCSS_PHYA_WFAX_PCSS_DMAC3_REG_MAP_OFFSET                 0x00381000
#define SEQ_WCSS_PHYA_WFAX_PCSS_DMAC4_REG_MAP_OFFSET                 0x00381400
#define SEQ_WCSS_PHYA_WFAX_PCSS_DUAL_TIMER_REG_MAP_OFFSET            0x00381800
#define SEQ_WCSS_PHYA_WFAX_PCSS_WATCHDOG_REG_MAP_OFFSET              0x00381c00
#define SEQ_WCSS_PHYA_WFAX_PCSS_XDMAC5_REG_MAP_OFFSET                0x00382c00
#define SEQ_WCSS_PHYA_WFAX_PCSS_XDMAC6_REG_MAP_OFFSET                0x00383000
#define SEQ_WCSS_PHYA_WFAX_NOC_REG_MAP_OFFSET                        0x00388000
#define SEQ_WCSS_PHYA_WFAX_TXFD_REG_MAP_OFFSET                       0x00390000
#define SEQ_WCSS_PHYA_WFAX_RXTD_REG_MAP_OFFSET                       0x003a0000
#define SEQ_WCSS_PHYA_WFAX_TXTD_REG_MAP_OFFSET                       0x003b0000
#define SEQ_WCSS_PHYA_WFAX_TXBF_REG_MAP_OFFSET                       0x003c0000
#define SEQ_WCSS_PHYA_WFAX_DEMFRONT_0_REG_MAP_OFFSET                 0x00400000
#define SEQ_WCSS_PHYA_WFAX_PHYRF_REG_MAP_OFFSET                      0x00480000
#define SEQ_WCSS_PHYA_WFAX_ROBE_REG_MAP_OFFSET                       0x004b0000
#define SEQ_WCSS_PHYA_WFAX_DEMFRONT_1_REG_MAP_OFFSET                 0x00500000
#define SEQ_WCSS_PHYA_WFAX_IRON2G_REG_MAP_OFFSET                     0x005c0000
#define SEQ_WCSS_PHYA_WFAX_IRON2G_REG_MAP_RFA_CMN_OFFSET             0x005d4000
#define SEQ_WCSS_PHYA_WFAX_IRON2G_REG_MAP_RFA_CMN_AON_OFFSET         0x005d4000
#define SEQ_WCSS_PHYA_WFAX_IRON2G_REG_MAP_RFA_CMN_AON_XFEM_OFFSET    0x005d4240
#define SEQ_WCSS_PHYA_WFAX_IRON2G_REG_MAP_RFA_CMN_AON_COEX_OFFSET    0x005d42c0
#define SEQ_WCSS_PHYA_WFAX_IRON2G_REG_MAP_RFA_CMN_RFFE_M_OFFSET      0x005d4300
#define SEQ_WCSS_PHYA_WFAX_IRON2G_REG_MAP_RFA_CMN_RFA_SHD_OTP_OFFSET 0x005d4400
#define SEQ_WCSS_PHYA_WFAX_IRON2G_REG_MAP_RFA_CMN_RFA_OTP_OFFSET     0x005d4480
#define SEQ_WCSS_PHYA_WFAX_IRON2G_REG_MAP_RFA_CMN_CLKGEN_OFFSET      0x005d4800
#define SEQ_WCSS_PHYA_WFAX_IRON2G_REG_MAP_RFA_CMN_WL_SYNTH0_BS_OFFSET 0x005d6000
#define SEQ_WCSS_PHYA_WFAX_IRON2G_REG_MAP_RFA_CMN_WL_SYNTH0_CLBS_OFFSET 0x005d6040
#define SEQ_WCSS_PHYA_WFAX_IRON2G_REG_MAP_RFA_CMN_WL_SYNTH0_BIST_OFFSET 0x005d6100
#define SEQ_WCSS_PHYA_WFAX_IRON2G_REG_MAP_RFA_CMN_WL_SYNTH0_PC_OFFSET 0x005d6140
#define SEQ_WCSS_PHYA_WFAX_IRON2G_REG_MAP_RFA_CMN_WL_SYNTH0_KVCO_OFFSET 0x005d6180
#define SEQ_WCSS_PHYA_WFAX_IRON2G_REG_MAP_RFA_CMN_WL_SYNTH0_AC_OFFSET 0x005d61c0
#define SEQ_WCSS_PHYA_WFAX_IRON2G_REG_MAP_RFA_CMN_WL_SYNTH0_LO_OFFSET 0x005d6280
#define SEQ_WCSS_PHYA_WFAX_IRON2G_REG_MAP_RFA_CMN_HLS_WL_REGFILE_OFFSET 0x005d7c00
#define SEQ_WCSS_PHYA_WFAX_IRON2G_REG_MAP_RFA_PMU_OFFSET             0x005da000
#define SEQ_WCSS_PHYA_WFAX_IRON2G_REG_MAP_RFA_PMU_PMU_OFFSET         0x005da000
#define SEQ_WCSS_PHYA_WFAX_IRON2G_REG_MAP_RFA_WL_OFFSET              0x005e0000
#define SEQ_WCSS_PHYA_WFAX_IRON2G_REG_MAP_RFA_WL_WL_MC_CH0_OFFSET    0x005e0000
#define SEQ_WCSS_PHYA_WFAX_IRON2G_REG_MAP_RFA_WL_WL_RXBB_CH0_OFFSET  0x005e0400
#define SEQ_WCSS_PHYA_WFAX_IRON2G_REG_MAP_RFA_WL_WL_TXBB_CH0_OFFSET  0x005e0800
#define SEQ_WCSS_PHYA_WFAX_IRON2G_REG_MAP_RFA_WL_WL_RXFE_CH0_OFFSET  0x005e1000
#define SEQ_WCSS_PHYA_WFAX_IRON2G_REG_MAP_RFA_WL_WL_TXFE_CH0_OFFSET  0x005e1300
#define SEQ_WCSS_PHYA_WFAX_IRON2G_REG_MAP_RFA_WL_WL_LO_PAL_CH0_OFFSET 0x005e1600
#define SEQ_WCSS_PHYA_WFAX_IRON2G_REG_MAP_RFA_WL_WL_LO_CH0_OFFSET    0x005e1640
#define SEQ_WCSS_PHYA_WFAX_IRON2G_REG_MAP_RFA_WL_WL_TPC_CH0_OFFSET   0x005e2000
#define SEQ_WCSS_PHYA_WFAX_IRON2G_REG_MAP_RFA_WL_WL_MEM_CH0_OFFSET   0x005e4000
#define SEQ_WCSS_PHYA_WFAX_IRON2G_REG_MAP_RFA_WL_WL_MC_CH1_OFFSET    0x005e8000
#define SEQ_WCSS_PHYA_WFAX_IRON2G_REG_MAP_RFA_WL_WL_RXBB_CH1_OFFSET  0x005e8400
#define SEQ_WCSS_PHYA_WFAX_IRON2G_REG_MAP_RFA_WL_WL_TXBB_CH1_OFFSET  0x005e8800
#define SEQ_WCSS_PHYA_WFAX_IRON2G_REG_MAP_RFA_WL_WL_RXFE_CH1_OFFSET  0x005e9000
#define SEQ_WCSS_PHYA_WFAX_IRON2G_REG_MAP_RFA_WL_WL_TXFE_CH1_OFFSET  0x005e9300
#define SEQ_WCSS_PHYA_WFAX_IRON2G_REG_MAP_RFA_WL_WL_LO_PAL_CH1_OFFSET 0x005e9600
#define SEQ_WCSS_PHYA_WFAX_IRON2G_REG_MAP_RFA_WL_WL_LO_CH1_OFFSET    0x005e9640
#define SEQ_WCSS_PHYA_WFAX_IRON2G_REG_MAP_RFA_WL_WL_TPC_CH1_OFFSET   0x005ea000
#define SEQ_WCSS_PHYA_WFAX_IRON2G_REG_MAP_RFA_WL_WL_MEM_CH1_OFFSET   0x005ec000
#define SEQ_WCSS_UMAC_OFFSET                                         0x00a00000
#define SEQ_WCSS_UMAC_CXC_TOP_REG_OFFSET                             0x00a20000
#define SEQ_WCSS_UMAC_CXC_TOP_REG_CXC_BMH_REG_OFFSET                 0x00a20000
#define SEQ_WCSS_UMAC_CXC_TOP_REG_CXC_LCMH_REG_OFFSET                0x00a22000
#define SEQ_WCSS_UMAC_CXC_TOP_REG_CXC_MCIBASIC_REG_OFFSET            0x00a24000
#define SEQ_WCSS_UMAC_CXC_TOP_REG_CXC_LMH_REG_OFFSET                 0x00a26000
#define SEQ_WCSS_UMAC_CXC_TOP_REG_CXC_SMH_REG_OFFSET                 0x00a28000
#define SEQ_WCSS_UMAC_CXC_TOP_REG_CXC_PMH_REG_OFFSET                 0x00a2a000
#define SEQ_WCSS_UMAC_MAC_TRACER_REG_OFFSET                          0x00a30000
#define SEQ_WCSS_UMAC_WBM_REG_OFFSET                                 0x00a34000
#define SEQ_WCSS_UMAC_REO_REG_OFFSET                                 0x00a38000
#define SEQ_WCSS_UMAC_TQM_REG_OFFSET                                 0x00a3c000
#define SEQ_WCSS_UMAC_MAC_UMCMN_REG_OFFSET                           0x00a40000
#define SEQ_WCSS_UMAC_MAC_TCL_REG_OFFSET                             0x00a44000
#define SEQ_WCSS_UMAC_MAC_CMN_PARSER_REG_OFFSET                      0x00a47000
#define SEQ_WCSS_WMAC0_OFFSET                                        0x00a80000
#define SEQ_WCSS_WMAC0_MAC_PDG_REG_OFFSET                            0x00a80000
#define SEQ_WCSS_WMAC0_MAC_TXDMA_REG_OFFSET                          0x00a83000
#define SEQ_WCSS_WMAC0_MAC_RXDMA_REG_OFFSET                          0x00a86000
#define SEQ_WCSS_WMAC0_MAC_MCMN_REG_OFFSET                           0x00a89000
#define SEQ_WCSS_WMAC0_MAC_RXPCU_REG_OFFSET                          0x00a8c000
#define SEQ_WCSS_WMAC0_MAC_TXPCU_REG_OFFSET                          0x00a8f000
#define SEQ_WCSS_WMAC0_MAC_AMPI_REG_OFFSET                           0x00a92000
#define SEQ_WCSS_WMAC0_MAC_RXOLE_REG_OFFSET                          0x00a95000
#define SEQ_WCSS_WMAC0_MAC_RXOLE_PARSER_REG_OFFSET                   0x00a98000
#define SEQ_WCSS_WMAC0_MAC_CCE_REG_OFFSET                            0x00a9b000
#define SEQ_WCSS_WMAC0_MAC_TXOLE_REG_OFFSET                          0x00a9e000
#define SEQ_WCSS_WMAC0_MAC_TXOLE_PARSER_REG_OFFSET                   0x00aa1000
#define SEQ_WCSS_WMAC0_MAC_RRI_REG_OFFSET                            0x00aa4000
#define SEQ_WCSS_WMAC0_MAC_CRYPTO_REG_OFFSET                         0x00aa7000
#define SEQ_WCSS_WMAC0_MAC_HWSCH_REG_OFFSET                          0x00aaa000
#define SEQ_WCSS_WMAC0_MAC_MXI_REG_OFFSET                            0x00ab0000
#define SEQ_WCSS_WMAC0_MAC_SFM_REG_OFFSET                            0x00ab3000
#define SEQ_WCSS_WMAC0_MAC_RXDMA1_REG_OFFSET                         0x00ab6000
#define SEQ_WCSS_APB_TSLV_OFFSET                                     0x00b40000
#define SEQ_WCSS_TOP_CMN_OFFSET                                      0x00b50000
#define SEQ_WCSS_WCMN_CORE_OFFSET                                    0x00b58000
#define SEQ_WCSS_WFSS_PMM_OFFSET                                     0x00b60000
#define SEQ_WCSS_PMM_TOP_OFFSET                                      0x00b70000
#define SEQ_WCSS_WL_MSIP_OFFSET                                      0x00b80000
#define SEQ_WCSS_WL_MSIP_RBIST_TX_CH0_OFFSET                         0x00b80000
#define SEQ_WCSS_WL_MSIP_WL_DAC_CH0_OFFSET                           0x00b80080
#define SEQ_WCSS_WL_MSIP_WL_DAC_DIG_CORRECTION_CH0_OFFSET            0x00b800c0
#define SEQ_WCSS_WL_MSIP_WL_DAC_MISC_CH0_OFFSET                      0x00b80340
#define SEQ_WCSS_WL_MSIP_WL_DAC_BBCLKGEN_CH0_OFFSET                  0x00b803c4
#define SEQ_WCSS_WL_MSIP_WL_ADC_CH0_OFFSET                           0x00b80400
#define SEQ_WCSS_WL_MSIP_WL_ADC_POSTPROC_I_EVEN_CH0_OFFSET           0x00b80800
#define SEQ_WCSS_WL_MSIP_WL_ADC_POSTPROC_I_ODD_CH0_OFFSET            0x00b80840
#define SEQ_WCSS_WL_MSIP_WL_ADC_POSTPROC_Q_EVEN_CH0_OFFSET           0x00b80880
#define SEQ_WCSS_WL_MSIP_WL_ADC_POSTPROC_Q_ODD_CH0_OFFSET            0x00b808c0
#define SEQ_WCSS_WL_MSIP_WL_ADC_POSTPROC_RO_CH0_OFFSET               0x00b80900
#define SEQ_WCSS_WL_MSIP_WL_ADC_BBCLKGEN_CH0_OFFSET                  0x00b8099c
#define SEQ_WCSS_WL_MSIP_RBIST_TX_CH1_OFFSET                         0x00b81000
#define SEQ_WCSS_WL_MSIP_WL_DAC_CH1_OFFSET                           0x00b81080
#define SEQ_WCSS_WL_MSIP_WL_DAC_DIG_CORRECTION_CH1_OFFSET            0x00b810c0
#define SEQ_WCSS_WL_MSIP_WL_DAC_MISC_CH1_OFFSET                      0x00b81340
#define SEQ_WCSS_WL_MSIP_WL_DAC_BBCLKGEN_CH1_OFFSET                  0x00b813c4
#define SEQ_WCSS_WL_MSIP_WL_ADC_CH1_OFFSET                           0x00b81400
#define SEQ_WCSS_WL_MSIP_WL_ADC_POSTPROC_I_EVEN_CH1_OFFSET           0x00b81800
#define SEQ_WCSS_WL_MSIP_WL_ADC_POSTPROC_I_ODD_CH1_OFFSET            0x00b81840
#define SEQ_WCSS_WL_MSIP_WL_ADC_POSTPROC_Q_EVEN_CH1_OFFSET           0x00b81880
#define SEQ_WCSS_WL_MSIP_WL_ADC_POSTPROC_Q_ODD_CH1_OFFSET            0x00b818c0
#define SEQ_WCSS_WL_MSIP_WL_ADC_POSTPROC_RO_CH1_OFFSET               0x00b81900
#define SEQ_WCSS_WL_MSIP_WL_ADC_BBCLKGEN_CH1_OFFSET                  0x00b8199c
#define SEQ_WCSS_WL_MSIP_MSIP_TMUX_OFFSET                            0x00b8d000
#define SEQ_WCSS_WL_MSIP_MSIP_OTP_OFFSET                             0x00b8d080
#define SEQ_WCSS_WL_MSIP_MSIP_LDO_CTRL_OFFSET                        0x00b8d0b4
#define SEQ_WCSS_WL_MSIP_MSIP_CLKGEN_OFFSET                          0x00b8d100
#define SEQ_WCSS_WL_MSIP_MSIP_BIAS_OFFSET                            0x00b8e000
#define SEQ_WCSS_WL_MSIP_BBPLL_OFFSET                                0x00b8f000
#define SEQ_WCSS_WL_MSIP_WL_TOP_CLKGEN_OFFSET                        0x00b8f100
#define SEQ_WCSS_WL_MSIP_MSIP_DRM_REG_OFFSET                         0x00b8fc00
#define SEQ_WCSS_DBG_OFFSET                                          0x00b90000
#define SEQ_WCSS_DBG_WCSS_DBG_DAPROM_OFFSET                          0x00b90000
#define SEQ_WCSS_DBG_CSR_WCSS_DBG_CSR_OFFSET                         0x00b91000
#define SEQ_WCSS_DBG_TSGEN_CXTSGEN_OFFSET                            0x00b92000
#define SEQ_WCSS_DBG_CTIDBG_QC_CTI_32T_8CH_OFFSET                    0x00b94000
#define SEQ_WCSS_DBG_CTINOC_QC_CTI_8T_8CH_OFFSET                     0x00b95000
#define SEQ_WCSS_DBG_CTIIRQ_QC_CTI_32T_8CH_OFFSET                    0x00b96000
#define SEQ_WCSS_DBG_EVENT_MACEVENT_OFFSET                           0x00bb0000
#define SEQ_WCSS_DBG_EVENTFUN_CXATBFUNNEL_32W8SP_OFFSET              0x00bb1000
#define SEQ_WCSS_DBG_TLV_MACTLV_OFFSET                               0x00bb2000
#define SEQ_WCSS_DBG_TLVFUN_CXATBFUNNEL_32W8SP_OFFSET                0x00bb3000
#define SEQ_WCSS_DBG_TBUS_MACTBUS_OFFSET                             0x00bb4000
#define SEQ_WCSS_DBG_TBUSFUN_CXATBFUNNEL_32W8SP_OFFSET               0x00bb5000
#define SEQ_WCSS_DBG_CTIMAC_QC_CTI_12T_8CH_OFFSET                    0x00bb6000
#define SEQ_WCSS_DBG_WCSS_DBG_TSTMP_INJCTR_OFFSET                    0x00bb8000
#define SEQ_WCSS_DBG_TPDM_OFFSET                                     0x00bb9000
#define SEQ_WCSS_DBG_TPDM_TPDM_ATB64_CMB40_DSB256_CSBE6C04F7_SUB_OFFSET 0x00bb9280
#define SEQ_WCSS_DBG_TPDM_TPDM_ATB64_CMB40_DSB256_CSBE6C04F7_GPR_OFFSET 0x00bb9000
#define SEQ_WCSS_DBG_TPDA_OFFSET                                     0x00bba000
#define SEQ_WCSS_DBG_CXATBFUNNEL_128W8SP_OFFSET                      0x00bbb000
#define SEQ_WCSS_DBG_TMC_CXTMC_F128W32K_OFFSET                       0x00bbc000
#define SEQ_WCSS_DBG_OUTFUN_CXATBFUNNEL_128W2SP_OFFSET               0x00bbe000
#define SEQ_WCSS_DBG_PHYFUN_CXATBFUNNEL_128W2SP_OFFSET               0x00bbf000
#define SEQ_WCSS_DBG_OUTDMUX_ATB_DEMUX_OFFSET                        0x00bc0000
#define SEQ_WCSS_DBG_TRCCNTRS_OFFSET                                 0x00bc1000
#define SEQ_WCSS_DBG_TLV_TPDM_ATB128_CMB64_OFFSET                    0x00bc2000
#define SEQ_WCSS_DBG_TLV_TPDM_ATB128_CMB64_TPDM_ATB128_CMB64_SUB_OFFSET 0x00bc2280
#define SEQ_WCSS_DBG_TLV_TPDM_ATB128_CMB64_TPDM_ATB128_CMB64_GPR_OFFSET 0x00bc2000
#define SEQ_WCSS_DBG_MISC_TPDM_ATB128_CMB64_OFFSET                   0x00bc3000
#define SEQ_WCSS_DBG_MISC_TPDM_ATB128_CMB64_TPDM_ATB128_CMB64_SUB_OFFSET 0x00bc3280
#define SEQ_WCSS_DBG_MISC_TPDM_ATB128_CMB64_TPDM_ATB128_CMB64_GPR_OFFSET 0x00bc3000
#define SEQ_WCSS_DBG_QC_TGU_APCLK_CSAE4EA8E3_OFFSET                  0x00bc4000
#define SEQ_WCSS_DBG_CTITGU_QC_CTI_4T_8CH_OFFSET                     0x00bc5000
#define SEQ_WCSS_DBG_PHYADMUX_ATB_DEMUX_OFFSET                       0x00bc6000
#define SEQ_WCSS_DBG_MISCFUN_CXATBFUNNEL_64W8SP_OFFSET               0x00bc8000
#define SEQ_WCSS_DBG_UNOC_UMAC_NOC_OFFSET                            0x00bd0000
#define SEQ_WCSS_DBG_PHYA_PHYA_DBG_OFFSET                            0x00be0000
#define SEQ_WCSS_DBG_PHYA_PHYA_DBG_PHYA_NOC_OFFSET                   0x00be0000
#define SEQ_WCSS_DBG_PHYA_PHYA_DBG_FUN_CXATBFUNNEL_64W8SP_OFFSET     0x00be4000
#define SEQ_WCSS_DBG_PHYA_PHYA_DBG_CTI_QC_CTI_10T_8CH_OFFSET         0x00be5000
#define SEQ_WCSS_DBG_PHYA_PHYA_DBG_TRC_PHYTRC_CTRL_OFFSET            0x00be6000
#define SEQ_WCSS_DBG_PHYA_PHYA_DBG_ITM_OFFSET                        0x00be8000
#define SEQ_WCSS_DBG_PHYA_PHYA_DBG_DWT_OFFSET                        0x00be9000
#define SEQ_WCSS_DBG_PHYA_PHYA_DBG_FPB_OFFSET                        0x00bea000
#define SEQ_WCSS_DBG_PHYA_PHYA_DBG_SCS_OFFSET                        0x00beb000
#define SEQ_WCSS_DBG_PHYA_PHYA_DBG_M3_ETM_OFFSET                     0x00bec000
#define SEQ_WCSS_DBG_PHYA_PHYA_DBG_M3CTI_QC_CTI_8T_8CH_OFFSET        0x00bed000
#define SEQ_WCSS_DBG_PHYA_PHYA_DBG_CPU0_M3_AHB_AP_OFFSET             0x00bee000
#define SEQ_WCSS_DBG_BUS_TIMEOUT_OFFSET                              0x00c31000
#define SEQ_WCSS_RET_AHB_OFFSET                                      0x00c90000
#define SEQ_WCSS_WAHB_TSLV_OFFSET                                    0x00ca0000
#define SEQ_WCSS_CC_OFFSET                                           0x00cb0000
#define SEQ_WCSS_UMAC_ACMT_OFFSET                                    0x00cc0000


///////////////////////////////////////////////////////////////////////////////////////////////
// Instance Relative Offsets from Block wfax_top
///////////////////////////////////////////////////////////////////////////////////////////////

#define SEQ_WFAX_TOP_WFAX_PCSS_PDMEM_REG_MAP_OFFSET                  0x00000000
#define SEQ_WFAX_TOP_WFAX_PCSS_REG_MAP_OFFSET                        0x00080000
#define SEQ_WFAX_TOP_WFAX_PCSS_DMAC0_REG_MAP_OFFSET                  0x00080400
#define SEQ_WFAX_TOP_WFAX_PCSS_DMAC1_REG_MAP_OFFSET                  0x00080800
#define SEQ_WFAX_TOP_WFAX_PCSS_DMAC2_REG_MAP_OFFSET                  0x00080c00
#define SEQ_WFAX_TOP_WFAX_PCSS_DMAC3_REG_MAP_OFFSET                  0x00081000
#define SEQ_WFAX_TOP_WFAX_PCSS_DMAC4_REG_MAP_OFFSET                  0x00081400
#define SEQ_WFAX_TOP_WFAX_PCSS_DUAL_TIMER_REG_MAP_OFFSET             0x00081800
#define SEQ_WFAX_TOP_WFAX_PCSS_WATCHDOG_REG_MAP_OFFSET               0x00081c00
#define SEQ_WFAX_TOP_WFAX_PCSS_XDMAC5_REG_MAP_OFFSET                 0x00082c00
#define SEQ_WFAX_TOP_WFAX_PCSS_XDMAC6_REG_MAP_OFFSET                 0x00083000
#define SEQ_WFAX_TOP_WFAX_NOC_REG_MAP_OFFSET                         0x00088000
#define SEQ_WFAX_TOP_WFAX_TXFD_REG_MAP_OFFSET                        0x00090000
#define SEQ_WFAX_TOP_WFAX_RXTD_REG_MAP_OFFSET                        0x000a0000
#define SEQ_WFAX_TOP_WFAX_TXTD_REG_MAP_OFFSET                        0x000b0000
#define SEQ_WFAX_TOP_WFAX_TXBF_REG_MAP_OFFSET                        0x000c0000
#define SEQ_WFAX_TOP_WFAX_DEMFRONT_0_REG_MAP_OFFSET                  0x00100000
#define SEQ_WFAX_TOP_WFAX_PHYRF_REG_MAP_OFFSET                       0x00180000
#define SEQ_WFAX_TOP_WFAX_ROBE_REG_MAP_OFFSET                        0x001b0000
#define SEQ_WFAX_TOP_WFAX_DEMFRONT_1_REG_MAP_OFFSET                  0x00200000
#define SEQ_WFAX_TOP_WFAX_IRON2G_REG_MAP_OFFSET                      0x002c0000
#define SEQ_WFAX_TOP_WFAX_IRON2G_REG_MAP_RFA_CMN_OFFSET              0x002d4000
#define SEQ_WFAX_TOP_WFAX_IRON2G_REG_MAP_RFA_CMN_AON_OFFSET          0x002d4000
#define SEQ_WFAX_TOP_WFAX_IRON2G_REG_MAP_RFA_CMN_AON_XFEM_OFFSET     0x002d4240
#define SEQ_WFAX_TOP_WFAX_IRON2G_REG_MAP_RFA_CMN_AON_COEX_OFFSET     0x002d42c0
#define SEQ_WFAX_TOP_WFAX_IRON2G_REG_MAP_RFA_CMN_RFFE_M_OFFSET       0x002d4300
#define SEQ_WFAX_TOP_WFAX_IRON2G_REG_MAP_RFA_CMN_RFA_SHD_OTP_OFFSET  0x002d4400
#define SEQ_WFAX_TOP_WFAX_IRON2G_REG_MAP_RFA_CMN_RFA_OTP_OFFSET      0x002d4480
#define SEQ_WFAX_TOP_WFAX_IRON2G_REG_MAP_RFA_CMN_CLKGEN_OFFSET       0x002d4800
#define SEQ_WFAX_TOP_WFAX_IRON2G_REG_MAP_RFA_CMN_WL_SYNTH0_BS_OFFSET 0x002d6000
#define SEQ_WFAX_TOP_WFAX_IRON2G_REG_MAP_RFA_CMN_WL_SYNTH0_CLBS_OFFSET 0x002d6040
#define SEQ_WFAX_TOP_WFAX_IRON2G_REG_MAP_RFA_CMN_WL_SYNTH0_BIST_OFFSET 0x002d6100
#define SEQ_WFAX_TOP_WFAX_IRON2G_REG_MAP_RFA_CMN_WL_SYNTH0_PC_OFFSET 0x002d6140
#define SEQ_WFAX_TOP_WFAX_IRON2G_REG_MAP_RFA_CMN_WL_SYNTH0_KVCO_OFFSET 0x002d6180
#define SEQ_WFAX_TOP_WFAX_IRON2G_REG_MAP_RFA_CMN_WL_SYNTH0_AC_OFFSET 0x002d61c0
#define SEQ_WFAX_TOP_WFAX_IRON2G_REG_MAP_RFA_CMN_WL_SYNTH0_LO_OFFSET 0x002d6280
#define SEQ_WFAX_TOP_WFAX_IRON2G_REG_MAP_RFA_CMN_HLS_WL_REGFILE_OFFSET 0x002d7c00
#define SEQ_WFAX_TOP_WFAX_IRON2G_REG_MAP_RFA_PMU_OFFSET              0x002da000
#define SEQ_WFAX_TOP_WFAX_IRON2G_REG_MAP_RFA_PMU_PMU_OFFSET          0x002da000
#define SEQ_WFAX_TOP_WFAX_IRON2G_REG_MAP_RFA_WL_OFFSET               0x002e0000
#define SEQ_WFAX_TOP_WFAX_IRON2G_REG_MAP_RFA_WL_WL_MC_CH0_OFFSET     0x002e0000
#define SEQ_WFAX_TOP_WFAX_IRON2G_REG_MAP_RFA_WL_WL_RXBB_CH0_OFFSET   0x002e0400
#define SEQ_WFAX_TOP_WFAX_IRON2G_REG_MAP_RFA_WL_WL_TXBB_CH0_OFFSET   0x002e0800
#define SEQ_WFAX_TOP_WFAX_IRON2G_REG_MAP_RFA_WL_WL_RXFE_CH0_OFFSET   0x002e1000
#define SEQ_WFAX_TOP_WFAX_IRON2G_REG_MAP_RFA_WL_WL_TXFE_CH0_OFFSET   0x002e1300
#define SEQ_WFAX_TOP_WFAX_IRON2G_REG_MAP_RFA_WL_WL_LO_PAL_CH0_OFFSET 0x002e1600
#define SEQ_WFAX_TOP_WFAX_IRON2G_REG_MAP_RFA_WL_WL_LO_CH0_OFFSET     0x002e1640
#define SEQ_WFAX_TOP_WFAX_IRON2G_REG_MAP_RFA_WL_WL_TPC_CH0_OFFSET    0x002e2000
#define SEQ_WFAX_TOP_WFAX_IRON2G_REG_MAP_RFA_WL_WL_MEM_CH0_OFFSET    0x002e4000
#define SEQ_WFAX_TOP_WFAX_IRON2G_REG_MAP_RFA_WL_WL_MC_CH1_OFFSET     0x002e8000
#define SEQ_WFAX_TOP_WFAX_IRON2G_REG_MAP_RFA_WL_WL_RXBB_CH1_OFFSET   0x002e8400
#define SEQ_WFAX_TOP_WFAX_IRON2G_REG_MAP_RFA_WL_WL_TXBB_CH1_OFFSET   0x002e8800
#define SEQ_WFAX_TOP_WFAX_IRON2G_REG_MAP_RFA_WL_WL_RXFE_CH1_OFFSET   0x002e9000
#define SEQ_WFAX_TOP_WFAX_IRON2G_REG_MAP_RFA_WL_WL_TXFE_CH1_OFFSET   0x002e9300
#define SEQ_WFAX_TOP_WFAX_IRON2G_REG_MAP_RFA_WL_WL_LO_PAL_CH1_OFFSET 0x002e9600
#define SEQ_WFAX_TOP_WFAX_IRON2G_REG_MAP_RFA_WL_WL_LO_CH1_OFFSET     0x002e9640
#define SEQ_WFAX_TOP_WFAX_IRON2G_REG_MAP_RFA_WL_WL_TPC_CH1_OFFSET    0x002ea000
#define SEQ_WFAX_TOP_WFAX_IRON2G_REG_MAP_RFA_WL_WL_MEM_CH1_OFFSET    0x002ec000


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


///////////////////////////////////////////////////////////////////////////////////////////////
// Instance Relative Offsets from Block umac_top_reg
///////////////////////////////////////////////////////////////////////////////////////////////

#define SEQ_UMAC_TOP_REG_CXC_TOP_REG_OFFSET                          0x00020000
#define SEQ_UMAC_TOP_REG_CXC_TOP_REG_CXC_BMH_REG_OFFSET              0x00020000
#define SEQ_UMAC_TOP_REG_CXC_TOP_REG_CXC_LCMH_REG_OFFSET             0x00022000
#define SEQ_UMAC_TOP_REG_CXC_TOP_REG_CXC_MCIBASIC_REG_OFFSET         0x00024000
#define SEQ_UMAC_TOP_REG_CXC_TOP_REG_CXC_LMH_REG_OFFSET              0x00026000
#define SEQ_UMAC_TOP_REG_CXC_TOP_REG_CXC_SMH_REG_OFFSET              0x00028000
#define SEQ_UMAC_TOP_REG_CXC_TOP_REG_CXC_PMH_REG_OFFSET              0x0002a000
#define SEQ_UMAC_TOP_REG_MAC_TRACER_REG_OFFSET                       0x00030000
#define SEQ_UMAC_TOP_REG_WBM_REG_OFFSET                              0x00034000
#define SEQ_UMAC_TOP_REG_REO_REG_OFFSET                              0x00038000
#define SEQ_UMAC_TOP_REG_TQM_REG_OFFSET                              0x0003c000
#define SEQ_UMAC_TOP_REG_MAC_UMCMN_REG_OFFSET                        0x00040000
#define SEQ_UMAC_TOP_REG_MAC_TCL_REG_OFFSET                          0x00044000
#define SEQ_UMAC_TOP_REG_MAC_CMN_PARSER_REG_OFFSET                   0x00047000


///////////////////////////////////////////////////////////////////////////////////////////////
// Instance Relative Offsets from Block cxc_top_reg
///////////////////////////////////////////////////////////////////////////////////////////////

#define SEQ_CXC_TOP_REG_CXC_BMH_REG_OFFSET                           0x00000000
#define SEQ_CXC_TOP_REG_CXC_LCMH_REG_OFFSET                          0x00002000
#define SEQ_CXC_TOP_REG_CXC_MCIBASIC_REG_OFFSET                      0x00004000
#define SEQ_CXC_TOP_REG_CXC_LMH_REG_OFFSET                           0x00006000
#define SEQ_CXC_TOP_REG_CXC_SMH_REG_OFFSET                           0x00008000
#define SEQ_CXC_TOP_REG_CXC_PMH_REG_OFFSET                           0x0000a000


///////////////////////////////////////////////////////////////////////////////////////////////
// Instance Relative Offsets from Block wmac_top_reg
///////////////////////////////////////////////////////////////////////////////////////////////

#define SEQ_WMAC_TOP_REG_MAC_PDG_REG_OFFSET                          0x00000000
#define SEQ_WMAC_TOP_REG_MAC_TXDMA_REG_OFFSET                        0x00003000
#define SEQ_WMAC_TOP_REG_MAC_RXDMA_REG_OFFSET                        0x00006000
#define SEQ_WMAC_TOP_REG_MAC_MCMN_REG_OFFSET                         0x00009000
#define SEQ_WMAC_TOP_REG_MAC_RXPCU_REG_OFFSET                        0x0000c000
#define SEQ_WMAC_TOP_REG_MAC_TXPCU_REG_OFFSET                        0x0000f000
#define SEQ_WMAC_TOP_REG_MAC_AMPI_REG_OFFSET                         0x00012000
#define SEQ_WMAC_TOP_REG_MAC_RXOLE_REG_OFFSET                        0x00015000
#define SEQ_WMAC_TOP_REG_MAC_RXOLE_PARSER_REG_OFFSET                 0x00018000
#define SEQ_WMAC_TOP_REG_MAC_CCE_REG_OFFSET                          0x0001b000
#define SEQ_WMAC_TOP_REG_MAC_TXOLE_REG_OFFSET                        0x0001e000
#define SEQ_WMAC_TOP_REG_MAC_TXOLE_PARSER_REG_OFFSET                 0x00021000
#define SEQ_WMAC_TOP_REG_MAC_RRI_REG_OFFSET                          0x00024000
#define SEQ_WMAC_TOP_REG_MAC_CRYPTO_REG_OFFSET                       0x00027000
#define SEQ_WMAC_TOP_REG_MAC_HWSCH_REG_OFFSET                        0x0002a000
#define SEQ_WMAC_TOP_REG_MAC_MXI_REG_OFFSET                          0x00030000
#define SEQ_WMAC_TOP_REG_MAC_SFM_REG_OFFSET                          0x00033000
#define SEQ_WMAC_TOP_REG_MAC_RXDMA1_REG_OFFSET                       0x00036000


///////////////////////////////////////////////////////////////////////////////////////////////
// Instance Relative Offsets from Block msip
///////////////////////////////////////////////////////////////////////////////////////////////

#define SEQ_MSIP_RBIST_TX_CH0_OFFSET                                 0x00000000
#define SEQ_MSIP_WL_DAC_CH0_OFFSET                                   0x00000080
#define SEQ_MSIP_WL_DAC_DIG_CORRECTION_CH0_OFFSET                    0x000000c0
#define SEQ_MSIP_WL_DAC_MISC_CH0_OFFSET                              0x00000340
#define SEQ_MSIP_WL_DAC_BBCLKGEN_CH0_OFFSET                          0x000003c4
#define SEQ_MSIP_WL_ADC_CH0_OFFSET                                   0x00000400
#define SEQ_MSIP_WL_ADC_POSTPROC_I_EVEN_CH0_OFFSET                   0x00000800
#define SEQ_MSIP_WL_ADC_POSTPROC_I_ODD_CH0_OFFSET                    0x00000840
#define SEQ_MSIP_WL_ADC_POSTPROC_Q_EVEN_CH0_OFFSET                   0x00000880
#define SEQ_MSIP_WL_ADC_POSTPROC_Q_ODD_CH0_OFFSET                    0x000008c0
#define SEQ_MSIP_WL_ADC_POSTPROC_RO_CH0_OFFSET                       0x00000900
#define SEQ_MSIP_WL_ADC_BBCLKGEN_CH0_OFFSET                          0x0000099c
#define SEQ_MSIP_RBIST_TX_CH1_OFFSET                                 0x00001000
#define SEQ_MSIP_WL_DAC_CH1_OFFSET                                   0x00001080
#define SEQ_MSIP_WL_DAC_DIG_CORRECTION_CH1_OFFSET                    0x000010c0
#define SEQ_MSIP_WL_DAC_MISC_CH1_OFFSET                              0x00001340
#define SEQ_MSIP_WL_DAC_BBCLKGEN_CH1_OFFSET                          0x000013c4
#define SEQ_MSIP_WL_ADC_CH1_OFFSET                                   0x00001400
#define SEQ_MSIP_WL_ADC_POSTPROC_I_EVEN_CH1_OFFSET                   0x00001800
#define SEQ_MSIP_WL_ADC_POSTPROC_I_ODD_CH1_OFFSET                    0x00001840
#define SEQ_MSIP_WL_ADC_POSTPROC_Q_EVEN_CH1_OFFSET                   0x00001880
#define SEQ_MSIP_WL_ADC_POSTPROC_Q_ODD_CH1_OFFSET                    0x000018c0
#define SEQ_MSIP_WL_ADC_POSTPROC_RO_CH1_OFFSET                       0x00001900
#define SEQ_MSIP_WL_ADC_BBCLKGEN_CH1_OFFSET                          0x0000199c
#define SEQ_MSIP_MSIP_TMUX_OFFSET                                    0x0000d000
#define SEQ_MSIP_MSIP_OTP_OFFSET                                     0x0000d080
#define SEQ_MSIP_MSIP_LDO_CTRL_OFFSET                                0x0000d0b4
#define SEQ_MSIP_MSIP_CLKGEN_OFFSET                                  0x0000d100
#define SEQ_MSIP_MSIP_BIAS_OFFSET                                    0x0000e000
#define SEQ_MSIP_BBPLL_OFFSET                                        0x0000f000
#define SEQ_MSIP_WL_TOP_CLKGEN_OFFSET                                0x0000f100
#define SEQ_MSIP_MSIP_DRM_REG_OFFSET                                 0x0000fc00


///////////////////////////////////////////////////////////////////////////////////////////////
// Instance Relative Offsets from Block wcssdbg
///////////////////////////////////////////////////////////////////////////////////////////////

#define SEQ_WCSSDBG_WCSS_DBG_DAPROM_OFFSET                           0x00000000
#define SEQ_WCSSDBG_CSR_WCSS_DBG_CSR_OFFSET                          0x00001000
#define SEQ_WCSSDBG_TSGEN_CXTSGEN_OFFSET                             0x00002000
#define SEQ_WCSSDBG_CTIDBG_QC_CTI_32T_8CH_OFFSET                     0x00004000
#define SEQ_WCSSDBG_CTINOC_QC_CTI_8T_8CH_OFFSET                      0x00005000
#define SEQ_WCSSDBG_CTIIRQ_QC_CTI_32T_8CH_OFFSET                     0x00006000
#define SEQ_WCSSDBG_EVENT_MACEVENT_OFFSET                            0x00020000
#define SEQ_WCSSDBG_EVENTFUN_CXATBFUNNEL_32W8SP_OFFSET               0x00021000
#define SEQ_WCSSDBG_TLV_MACTLV_OFFSET                                0x00022000
#define SEQ_WCSSDBG_TLVFUN_CXATBFUNNEL_32W8SP_OFFSET                 0x00023000
#define SEQ_WCSSDBG_TBUS_MACTBUS_OFFSET                              0x00024000
#define SEQ_WCSSDBG_TBUSFUN_CXATBFUNNEL_32W8SP_OFFSET                0x00025000
#define SEQ_WCSSDBG_CTIMAC_QC_CTI_12T_8CH_OFFSET                     0x00026000
#define SEQ_WCSSDBG_WCSS_DBG_TSTMP_INJCTR_OFFSET                     0x00028000
#define SEQ_WCSSDBG_TPDM_OFFSET                                      0x00029000
#define SEQ_WCSSDBG_TPDM_TPDM_ATB64_CMB40_DSB256_CSBE6C04F7_SUB_OFFSET 0x00029280
#define SEQ_WCSSDBG_TPDM_TPDM_ATB64_CMB40_DSB256_CSBE6C04F7_GPR_OFFSET 0x00029000
#define SEQ_WCSSDBG_TPDA_OFFSET                                      0x0002a000
#define SEQ_WCSSDBG_CXATBFUNNEL_128W8SP_OFFSET                       0x0002b000
#define SEQ_WCSSDBG_TMC_CXTMC_F128W32K_OFFSET                        0x0002c000
#define SEQ_WCSSDBG_OUTFUN_CXATBFUNNEL_128W2SP_OFFSET                0x0002e000
#define SEQ_WCSSDBG_PHYFUN_CXATBFUNNEL_128W2SP_OFFSET                0x0002f000
#define SEQ_WCSSDBG_OUTDMUX_ATB_DEMUX_OFFSET                         0x00030000
#define SEQ_WCSSDBG_TRCCNTRS_OFFSET                                  0x00031000
#define SEQ_WCSSDBG_TLV_TPDM_ATB128_CMB64_OFFSET                     0x00032000
#define SEQ_WCSSDBG_TLV_TPDM_ATB128_CMB64_TPDM_ATB128_CMB64_SUB_OFFSET 0x00032280
#define SEQ_WCSSDBG_TLV_TPDM_ATB128_CMB64_TPDM_ATB128_CMB64_GPR_OFFSET 0x00032000
#define SEQ_WCSSDBG_MISC_TPDM_ATB128_CMB64_OFFSET                    0x00033000
#define SEQ_WCSSDBG_MISC_TPDM_ATB128_CMB64_TPDM_ATB128_CMB64_SUB_OFFSET 0x00033280
#define SEQ_WCSSDBG_MISC_TPDM_ATB128_CMB64_TPDM_ATB128_CMB64_GPR_OFFSET 0x00033000
#define SEQ_WCSSDBG_QC_TGU_APCLK_CSAE4EA8E3_OFFSET                   0x00034000
#define SEQ_WCSSDBG_CTITGU_QC_CTI_4T_8CH_OFFSET                      0x00035000
#define SEQ_WCSSDBG_PHYADMUX_ATB_DEMUX_OFFSET                        0x00036000
#define SEQ_WCSSDBG_MISCFUN_CXATBFUNNEL_64W8SP_OFFSET                0x00038000
#define SEQ_WCSSDBG_UNOC_UMAC_NOC_OFFSET                             0x00040000
#define SEQ_WCSSDBG_PHYA_PHYA_DBG_OFFSET                             0x00050000
#define SEQ_WCSSDBG_PHYA_PHYA_DBG_PHYA_NOC_OFFSET                    0x00050000
#define SEQ_WCSSDBG_PHYA_PHYA_DBG_FUN_CXATBFUNNEL_64W8SP_OFFSET      0x00054000
#define SEQ_WCSSDBG_PHYA_PHYA_DBG_CTI_QC_CTI_10T_8CH_OFFSET          0x00055000
#define SEQ_WCSSDBG_PHYA_PHYA_DBG_TRC_PHYTRC_CTRL_OFFSET             0x00056000
#define SEQ_WCSSDBG_PHYA_PHYA_DBG_ITM_OFFSET                         0x00058000
#define SEQ_WCSSDBG_PHYA_PHYA_DBG_DWT_OFFSET                         0x00059000
#define SEQ_WCSSDBG_PHYA_PHYA_DBG_FPB_OFFSET                         0x0005a000
#define SEQ_WCSSDBG_PHYA_PHYA_DBG_SCS_OFFSET                         0x0005b000
#define SEQ_WCSSDBG_PHYA_PHYA_DBG_M3_ETM_OFFSET                      0x0005c000
#define SEQ_WCSSDBG_PHYA_PHYA_DBG_M3CTI_QC_CTI_8T_8CH_OFFSET         0x0005d000
#define SEQ_WCSSDBG_PHYA_PHYA_DBG_CPU0_M3_AHB_AP_OFFSET              0x0005e000
#define SEQ_WCSSDBG_BUS_TIMEOUT_OFFSET                               0x000a1000


///////////////////////////////////////////////////////////////////////////////////////////////
// Instance Relative Offsets from Block tpdm_atb64_cmb40_dsb256_csbe6c04f7
///////////////////////////////////////////////////////////////////////////////////////////////

#define SEQ_TPDM_ATB64_CMB40_DSB256_CSBE6C04F7_TPDM_ATB64_CMB40_DSB256_CSBE6C04F7_SUB_OFFSET 0x00000280
#define SEQ_TPDM_ATB64_CMB40_DSB256_CSBE6C04F7_TPDM_ATB64_CMB40_DSB256_CSBE6C04F7_GPR_OFFSET 0x00000000


///////////////////////////////////////////////////////////////////////////////////////////////
// Instance Relative Offsets from Block tpdm_atb128_cmb64
///////////////////////////////////////////////////////////////////////////////////////////////

#define SEQ_TPDM_ATB128_CMB64_TPDM_ATB128_CMB64_SUB_OFFSET           0x00000280
#define SEQ_TPDM_ATB128_CMB64_TPDM_ATB128_CMB64_GPR_OFFSET           0x00000000


///////////////////////////////////////////////////////////////////////////////////////////////
// Instance Relative Offsets from Block phya_dbg
///////////////////////////////////////////////////////////////////////////////////////////////

#define SEQ_PHYA_DBG_PHYA_NOC_OFFSET                                 0x00000000
#define SEQ_PHYA_DBG_FUN_CXATBFUNNEL_64W8SP_OFFSET                   0x00004000
#define SEQ_PHYA_DBG_CTI_QC_CTI_10T_8CH_OFFSET                       0x00005000
#define SEQ_PHYA_DBG_TRC_PHYTRC_CTRL_OFFSET                          0x00006000
#define SEQ_PHYA_DBG_ITM_OFFSET                                      0x00008000
#define SEQ_PHYA_DBG_DWT_OFFSET                                      0x00009000
#define SEQ_PHYA_DBG_FPB_OFFSET                                      0x0000a000
#define SEQ_PHYA_DBG_SCS_OFFSET                                      0x0000b000
#define SEQ_PHYA_DBG_M3_ETM_OFFSET                                   0x0000c000
#define SEQ_PHYA_DBG_M3CTI_QC_CTI_8T_8CH_OFFSET                      0x0000d000
#define SEQ_PHYA_DBG_CPU0_M3_AHB_AP_OFFSET                           0x0000e000


#endif

