/*
 * Copyright (c) 2024 Qualcomm Innovation Center, Inc. All rights reserved.
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
#ifndef MSMHWIOBASE_H
#define MSMHWIOBASE_H

#define HOST_WCSS_WCSS_BASE                                                0x0
#define HOST_WCSS_WCSS_BASE_SIZE                                    0x00d00000
#define HOST_WCSS_WCSS_BASE_PHYS                                    0x00000000

#define HOST_TLMM_BASE                                               0x1a00000
#define HOST_TLMM_BASE_SIZE                                         0x00100000
#define HOST_TLMM_BASE_PHYS                                         0x01a00000

#define HOST_CORE_TOP_CSR_BASE                                       0x1b00000
#define HOST_CORE_TOP_CSR_BASE_SIZE                                 0x0003f000
#define HOST_CORE_TOP_CSR_BASE_PHYS                                 0x01b00000

#define HOST_SOC_WFSS_CE_REG_BASE                                    0x1b80000
#define HOST_SOC_WFSS_CE_REG_BASE_SIZE                              0x0001c000
#define HOST_SOC_WFSS_CE_REG_BASE_PHYS                              0x01b80000

#define HOST_WL_TLMM_BASE                                            0x1bc0000
#define HOST_WL_TLMM_BASE_SIZE                                      0x00020000
#define HOST_WL_TLMM_BASE_PHYS                                      0x01bc0000

#define HOST_TSENS_SROT_BASE                                         0x1bf0000
#define HOST_TSENS_SROT_BASE_SIZE                                   0x00001000
#define HOST_TSENS_SROT_BASE_PHYS                                   0x01bf0000

#define HOST_TSENS_TM_BASE                                           0x1bf1000
#define HOST_TSENS_TM_BASE_SIZE                                     0x00001000
#define HOST_TSENS_TM_BASE_PHYS                                     0x01bf1000

#define HOST_QDSS_QDSS_BASE                                          0x1c00000
#define HOST_QDSS_QDSS_BASE_SIZE                                    0x00080000
#define HOST_QDSS_QDSS_BASE_PHYS                                    0x01c00000

#define HOST_QDSS_WRAPPER_TOP_BASE                                   0x1c80000
#define HOST_QDSS_WRAPPER_TOP_BASE_SIZE                             0x0007fffd
#define HOST_QDSS_WRAPPER_TOP_BASE_PHYS                             0x01c80000

#define HOST_QDSS_APB_WCSS_DBG_DEC_QDSS_APB_WCSSDBG_BASE             0x1d00000
#define HOST_QDSS_APB_WCSS_DBG_DEC_QDSS_APB_WCSSDBG_BASE_SIZE       0x00100000
#define HOST_QDSS_APB_WCSS_DBG_DEC_QDSS_APB_WCSSDBG_BASE_PHYS       0x01d00000

#define HOST_PCIE_PCIE_TOP_WRAPPER_BASE                              0x1e00000
#define HOST_PCIE_PCIE_TOP_WRAPPER_BASE_SIZE                        0x00020000
#define HOST_PCIE_PCIE_TOP_WRAPPER_BASE_PHYS                        0x01e00000

#define HOST_SECURITY_CONTROL_WLAN_BASE                              0x1e20000
#define HOST_SECURITY_CONTROL_WLAN_BASE_SIZE                        0x00008000
#define HOST_SECURITY_CONTROL_WLAN_BASE_PHYS                        0x01e20000

#define HOST_EDPD_CAL_ACC_BASE                                       0x1e28000
#define HOST_EDPD_CAL_ACC_BASE_SIZE                                 0x00003000
#define HOST_EDPD_CAL_ACC_BASE_PHYS                                 0x01e28000

#define HOST_CPR_CX_CPR3_BASE                                        0x1e30000
#define HOST_CPR_CX_CPR3_BASE_SIZE                                  0x00004000
#define HOST_CPR_CX_CPR3_BASE_PHYS                                  0x01e30000

#define HOST_CPR_MX_CPR3_BASE                                        0x1e34000
#define HOST_CPR_MX_CPR3_BASE_SIZE                                  0x00004000
#define HOST_CPR_MX_CPR3_BASE_PHYS                                  0x01e34000

#define HOST_HZ_AUXSSAUXSS_SWI_BASE                                  0x1e38000
#define HOST_HZ_AUXSSAUXSS_SWI_BASE_SIZE                            0x00007000
#define HOST_HZ_AUXSSAUXSS_SWI_BASE_PHYS                            0x01e38000

#define HOST_GCC_GCC_BASE                                            0x1e40000
#define HOST_GCC_GCC_BASE_SIZE                                      0x0000048c
#define HOST_GCC_GCC_BASE_PHYS                                      0x01e40000

#define HOST_PCNOC_0_BUS_TIMEOUT_BASE                                0x1e60000
#define HOST_PCNOC_0_BUS_TIMEOUT_BASE_SIZE                          0x00001000
#define HOST_PCNOC_0_BUS_TIMEOUT_BASE_PHYS                          0x01e60000

#define HOST_PCNOC_1_BUS_TIMEOUT_BASE                                0x1e61000
#define HOST_PCNOC_1_BUS_TIMEOUT_BASE_SIZE                          0x00001000
#define HOST_PCNOC_1_BUS_TIMEOUT_BASE_PHYS                          0x01e61000

#define HOST_PCNOC_2_BUS_TIMEOUT_BASE                                0x1e62000
#define HOST_PCNOC_2_BUS_TIMEOUT_BASE_SIZE                          0x00001000
#define HOST_PCNOC_2_BUS_TIMEOUT_BASE_PHYS                          0x01e62000

#define HOST_PCNOC_3_BUS_TIMEOUT_BASE                                0x1e63000
#define HOST_PCNOC_3_BUS_TIMEOUT_BASE_SIZE                          0x00001000
#define HOST_PCNOC_3_BUS_TIMEOUT_BASE_PHYS                          0x01e63000

#define HOST_PCNOC_4_BUS_TIMEOUT_BASE                                0x1e64000
#define HOST_PCNOC_4_BUS_TIMEOUT_BASE_SIZE                          0x00001000
#define HOST_PCNOC_4_BUS_TIMEOUT_BASE_PHYS                          0x01e64000

#define HOST_RRI_PREFETCH_REG_BASE                                   0x1e70000
#define HOST_RRI_PREFETCH_REG_BASE_SIZE                             0x00010000
#define HOST_RRI_PREFETCH_REG_BASE_PHYS                             0x01e70000

#define HOST_SYSTEM_NOC_BASE                                         0x1e80000
#define HOST_SYSTEM_NOC_BASE_SIZE                                   0x0000a000
#define HOST_SYSTEM_NOC_BASE_PHYS                                   0x01e80000

#define HOST_PC_NOC_BASE                                             0x1f00000
#define HOST_PC_NOC_BASE_SIZE                                       0x00003880
#define HOST_PC_NOC_BASE_PHYS                                       0x01f00000

#define HOST_WLAON_WL_AON_REG_BASE                                   0x1f80000
#define HOST_WLAON_WL_AON_REG_BASE_SIZE                             0x000007f0
#define HOST_WLAON_WL_AON_REG_BASE_PHYS                             0x01f80000

#define HOST_SYSPM_SYSPM_REG_BASE                                    0x1f82000
#define HOST_SYSPM_SYSPM_REG_BASE_SIZE                              0x00001000
#define HOST_SYSPM_SYSPM_REG_BASE_PHYS                              0x01f82000

#define HOST_PMU_WLAN_PMU_TOP_BASE                                   0x1f88000
#define HOST_PMU_WLAN_PMU_TOP_BASE_SIZE                             0x00000400
#define HOST_PMU_WLAN_PMU_TOP_BASE_PHYS                             0x01f88000

#define HOST_PMU_NOC_BASE                                            0x1f8a000
#define HOST_PMU_NOC_BASE_SIZE                                      0x00000080
#define HOST_PMU_NOC_BASE_PHYS                                      0x01f8a000

#define HOST_SYSTEM_IRAM                                             0x1400000
#define HOST_SYSTEM_IRAM_SIZE                                       0x00025000
#define HOST_SYSTEM_IRAM_PHYS                                       0x01400000

#define HOST_PCIE_ATU_REGION                                         0x4000000
#define HOST_PCIE_ATU_REGION_SIZE                                   0x40000000
#define HOST_PCIE_ATU_REGION_PHYS                                   0x04000000

#endif
