
/*
 * Copyright (c) 2022 Qualcomm Innovation Center, Inc. All rights reserved.
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

#ifndef __MSMHWIOBASE_H__
#define __MSMHWIOBASE_H__

#define WCSS_WCSS_BASE                                              0x00000000
#define WCSS_WCSS_BASE_SIZE                                         0x01000000
#define WCSS_WCSS_BASE_PHYS                                         0x00000000

#define QDSS_STM_SIZE_BASE                                          0x00100000
#define QDSS_STM_SIZE_BASE_SIZE                                     0x100000000
#define QDSS_STM_SIZE_BASE_PHYS                                     0x00100000

#define BOOT_ROM_SIZE_BASE                                          0x00200000
#define BOOT_ROM_SIZE_BASE_SIZE                                     0x100000000
#define BOOT_ROM_SIZE_BASE_PHYS                                     0x00200000

#define SYSTEM_IRAM_SIZE_BASE                                       0x00400000
#define SYSTEM_IRAM_SIZE_BASE_SIZE                                  0x100000000
#define SYSTEM_IRAM_SIZE_BASE_PHYS                                  0x00400000

#define BOOT_ROM_START_ADDRESS_BASE                                 0x01200000
#define BOOT_ROM_START_ADDRESS_BASE_SIZE                            0x100000000
#define BOOT_ROM_START_ADDRESS_BASE_PHYS                            0x01200000

#define BOOT_ROM_END_ADDRESS_BASE                                   0x013fffff
#define BOOT_ROM_END_ADDRESS_BASE_SIZE                              0x100000000
#define BOOT_ROM_END_ADDRESS_BASE_PHYS                              0x013fffff

#define SYSTEM_IRAM_START_ADDRESS_BASE                              0x01400000
#define SYSTEM_IRAM_START_ADDRESS_BASE_SIZE                         0x100000000
#define SYSTEM_IRAM_START_ADDRESS_BASE_PHYS                         0x01400000

#define SYSTEM_IRAM_END_ADDRESS_BASE                                0x017fffff
#define SYSTEM_IRAM_END_ADDRESS_BASE_SIZE                           0x100000000
#define SYSTEM_IRAM_END_ADDRESS_BASE_PHYS                           0x017fffff

#define QDSS_STM_BASE                                               0x01800000
#define QDSS_STM_BASE_SIZE                                          0x100000000
#define QDSS_STM_BASE_PHYS                                          0x01800000

#define QDSS_STM_END_BASE                                           0x018fffff
#define QDSS_STM_END_BASE_SIZE                                      0x100000000
#define QDSS_STM_END_BASE_PHYS                                      0x018fffff

#define TLMM_BASE                                                   0x01900000
#define TLMM_BASE_SIZE                                              0x00200000
#define TLMM_BASE_PHYS                                              0x01900000

#define CORE_TOP_CSR_BASE                                           0x01b00000
#define CORE_TOP_CSR_BASE_SIZE                                      0x00040000
#define CORE_TOP_CSR_BASE_PHYS                                      0x01b00000

#define BLSP1_BLSP_BASE                                             0x01b40000
#define BLSP1_BLSP_BASE_SIZE                                        0x00040000
#define BLSP1_BLSP_BASE_PHYS                                        0x01b40000

#define SOC_WFSS_CE_REG_BASE                                        0x01b80000
#define SOC_WFSS_CE_REG_BASE_SIZE                                   0x0001c000
#define SOC_WFSS_CE_REG_BASE_PHYS                                   0x01b80000

#define WL_TLMM_BASE                                                0x01bc0000
#define WL_TLMM_BASE_SIZE                                           0x00020000
#define WL_TLMM_BASE_PHYS                                           0x01bc0000

#define MEMSS_CSR_BASE                                              0x01be0000
#define MEMSS_CSR_BASE_SIZE                                         0x0000001c
#define MEMSS_CSR_BASE_PHYS                                         0x01be0000

#define TSENS_SROT_BASE                                             0x01bf0000
#define TSENS_SROT_BASE_SIZE                                        0x00001000
#define TSENS_SROT_BASE_PHYS                                        0x01bf0000

#define TSENS_TM_BASE                                               0x01bf1000
#define TSENS_TM_BASE_SIZE                                          0x00001000
#define TSENS_TM_BASE_PHYS                                          0x01bf1000

#define QDSS_APB_DEC_QDSS_APB_BASE                                  0x01c00000
#define QDSS_APB_DEC_QDSS_APB_BASE_SIZE                             0x00080000
#define QDSS_APB_DEC_QDSS_APB_BASE_PHYS                             0x01c00000

#define QDSS_WRAPPER_TOP_BASE                                       0x01c80000
#define QDSS_WRAPPER_TOP_BASE_SIZE                                  0x0007fffd
#define QDSS_WRAPPER_TOP_BASE_PHYS                                  0x01c80000

#define QDSS_APB_WCSS_DBG_DEC_QDSS_APB_WCSSDBG_BASE                 0x01d00000
#define QDSS_APB_WCSS_DBG_DEC_QDSS_APB_WCSSDBG_BASE_SIZE            0x00100000
#define QDSS_APB_WCSS_DBG_DEC_QDSS_APB_WCSSDBG_BASE_PHYS            0x01d00000

#define PCIE_PCIE_TOP_WRAPPER_BASE                                  0x01e00000
#define PCIE_PCIE_TOP_WRAPPER_BASE_SIZE                             0x00020000
#define PCIE_PCIE_TOP_WRAPPER_BASE_PHYS                             0x01e00000

#define SECURITY_CONTROL_WLAN_BASE                                  0x01e20000
#define SECURITY_CONTROL_WLAN_BASE_SIZE                             0x00008000
#define SECURITY_CONTROL_WLAN_BASE_PHYS                             0x01e20000

#define EDPD_CAL_ACC_BASE                                           0x01e28000
#define EDPD_CAL_ACC_BASE_SIZE                                      0x00003000
#define EDPD_CAL_ACC_BASE_PHYS                                      0x01e28000

#define CPR_CX_CPR3_BASE                                            0x01e30000
#define CPR_CX_CPR3_BASE_SIZE                                       0x00004000
#define CPR_CX_CPR3_BASE_PHYS                                       0x01e30000

#define CPR_MX_CPR3_BASE                                            0x01e34000
#define CPR_MX_CPR3_BASE_SIZE                                       0x00004000
#define CPR_MX_CPR3_BASE_PHYS                                       0x01e34000

#define GCC_GCC_BASE                                                0x01e40000
#define GCC_GCC_BASE_SIZE                                           0x000003e8
#define GCC_GCC_BASE_PHYS                                           0x01e40000

#define PRNG_PRNG_TOP_BASE                                          0x01e50000
#define PRNG_PRNG_TOP_BASE_SIZE                                     0x00010000
#define PRNG_PRNG_TOP_BASE_PHYS                                     0x01e50000

#define PCNOC_0_BUS_TIMEOUT_BASE                                    0x01e60000
#define PCNOC_0_BUS_TIMEOUT_BASE_SIZE                               0x00001000
#define PCNOC_0_BUS_TIMEOUT_BASE_PHYS                               0x01e60000

#define PCNOC_1_BUS_TIMEOUT_BASE                                    0x01e61000
#define PCNOC_1_BUS_TIMEOUT_BASE_SIZE                               0x00001000
#define PCNOC_1_BUS_TIMEOUT_BASE_PHYS                               0x01e61000

#define PCNOC_2_BUS_TIMEOUT_BASE                                    0x01e62000
#define PCNOC_2_BUS_TIMEOUT_BASE_SIZE                               0x00001000
#define PCNOC_2_BUS_TIMEOUT_BASE_PHYS                               0x01e62000

#define PCNOC_3_BUS_TIMEOUT_BASE                                    0x01e63000
#define PCNOC_3_BUS_TIMEOUT_BASE_SIZE                               0x00001000
#define PCNOC_3_BUS_TIMEOUT_BASE_PHYS                               0x01e63000

#define RRI_PREFETCH_REG_BASE                                       0x01e70000
#define RRI_PREFETCH_REG_BASE_SIZE                                  0x00010000
#define RRI_PREFETCH_REG_BASE_PHYS                                  0x01e70000

#define SYSTEM_NOC_BASE                                             0x01e80000
#define SYSTEM_NOC_BASE_SIZE                                        0x0000a000
#define SYSTEM_NOC_BASE_PHYS                                        0x01e80000

#define PC_NOC_BASE                                                 0x01f00000
#define PC_NOC_BASE_SIZE                                            0x00003880
#define PC_NOC_BASE_PHYS                                            0x01f00000

#define WLAON_WL_AON_REG_BASE                                       0x01f80000
#define WLAON_WL_AON_REG_BASE_SIZE                                  0x000007c8
#define WLAON_WL_AON_REG_BASE_PHYS                                  0x01f80000

#define SYSPM_SYSPM_REG_BASE                                        0x01f82000
#define SYSPM_SYSPM_REG_BASE_SIZE                                   0x00001000
#define SYSPM_SYSPM_REG_BASE_PHYS                                   0x01f82000

#define PMU_WLAN_PMU_TOP_BASE                                       0x01f88000
#define PMU_WLAN_PMU_TOP_BASE_SIZE                                  0x00000340
#define PMU_WLAN_PMU_TOP_BASE_PHYS                                  0x01f88000

#define PMU_NOC_BASE                                                0x01f8a000
#define PMU_NOC_BASE_SIZE                                           0x00000080
#define PMU_NOC_BASE_PHYS                                           0x01f8a000

#define PCIE_ATU_REGION_BASE                                        0x04000000
#define PCIE_ATU_REGION_BASE_SIZE                                   0x100000000
#define PCIE_ATU_REGION_BASE_PHYS                                   0x04000000

#define PCIE_ATU_REGION_SIZE_BASE                                   0x40000000
#define PCIE_ATU_REGION_SIZE_BASE_SIZE                              0x100000000
#define PCIE_ATU_REGION_SIZE_BASE_PHYS                              0x40000000

#define PCIE_ATU_REGION_END_BASE                                    0x43ffffff
#define PCIE_ATU_REGION_END_BASE_SIZE                               0x100000000
#define PCIE_ATU_REGION_END_BASE_PHYS                               0x43ffffff

#endif
