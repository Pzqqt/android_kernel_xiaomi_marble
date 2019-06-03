/*
 * Copyright (c) 2019, The Linux Foundation. All rights reserved.
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

#ifndef __MSMHWIOBASE_H__
#define __MSMHWIOBASE_H__
/*
===========================================================================
*/
/**
  @file msmhwiobase.h
  @brief Auto-generated HWIO base include file.
*/
/*
  ===========================================================================


  ===========================================================================
*/

/*----------------------------------------------------------------------------
 * BASE: WCSS_WCSS
 *--------------------------------------------------------------------------*/

#define WCSS_WCSS_BASE                                              0x00000000
#define WCSS_WCSS_BASE_SIZE                                         0x01000000
#define WCSS_WCSS_BASE_PHYS                                         0x00000000

/*----------------------------------------------------------------------------
 * BASE: BOOT_ROM_SIZE
 *--------------------------------------------------------------------------*/

#define BOOT_ROM_SIZE_BASE                                          0x00100000
#define BOOT_ROM_SIZE_BASE_SIZE                                     0x100000000
#define BOOT_ROM_SIZE_BASE_PHYS                                     0x00100000

/*----------------------------------------------------------------------------
 * BASE: QDSS_STM_SIZE
 *--------------------------------------------------------------------------*/

#define QDSS_STM_SIZE_BASE                                          0x00100000
#define QDSS_STM_SIZE_BASE_SIZE                                     0x100000000
#define QDSS_STM_SIZE_BASE_PHYS                                     0x00100000

/*----------------------------------------------------------------------------
 * BASE: SYSTEM_IRAM_SIZE
 *--------------------------------------------------------------------------*/

#define SYSTEM_IRAM_SIZE_BASE                                       0x00400000
#define SYSTEM_IRAM_SIZE_BASE_SIZE                                  0x100000000
#define SYSTEM_IRAM_SIZE_BASE_PHYS                                  0x00400000

/*----------------------------------------------------------------------------
 * BASE: BOOT_ROM_START_ADDRESS
 *--------------------------------------------------------------------------*/

#define BOOT_ROM_START_ADDRESS_BASE                                 0x00800000
#define BOOT_ROM_START_ADDRESS_BASE_SIZE                            0x100000000
#define BOOT_ROM_START_ADDRESS_BASE_PHYS                            0x00800000

/*----------------------------------------------------------------------------
 * BASE: BOOT_ROM_END_ADDRESS
 *--------------------------------------------------------------------------*/

#define BOOT_ROM_END_ADDRESS_BASE                                   0x008fffff
#define BOOT_ROM_END_ADDRESS_BASE_SIZE                              0x100000000
#define BOOT_ROM_END_ADDRESS_BASE_PHYS                              0x008fffff

/*----------------------------------------------------------------------------
 * BASE: QDSS_STM
 *--------------------------------------------------------------------------*/

#define QDSS_STM_BASE                                               0x00900000
#define QDSS_STM_BASE_SIZE                                          0x100000000
#define QDSS_STM_BASE_PHYS                                          0x00900000

/*----------------------------------------------------------------------------
 * BASE: QDSS_STM_END
 *--------------------------------------------------------------------------*/

#define QDSS_STM_END_BASE                                           0x009fffff
#define QDSS_STM_END_BASE_SIZE                                      0x100000000
#define QDSS_STM_END_BASE_PHYS                                      0x009fffff

/*----------------------------------------------------------------------------
 * BASE: SYSTEM_IRAM_START_ADDRESS
 *--------------------------------------------------------------------------*/

#define SYSTEM_IRAM_START_ADDRESS_BASE                              0x01400000
#define SYSTEM_IRAM_START_ADDRESS_BASE_SIZE                         0x100000000
#define SYSTEM_IRAM_START_ADDRESS_BASE_PHYS                         0x01400000

/*----------------------------------------------------------------------------
 * BASE: SYSTEM_IRAM_END_ADDRESS
 *--------------------------------------------------------------------------*/

#define SYSTEM_IRAM_END_ADDRESS_BASE                                0x017fffff
#define SYSTEM_IRAM_END_ADDRESS_BASE_SIZE                           0x100000000
#define SYSTEM_IRAM_END_ADDRESS_BASE_PHYS                           0x017fffff

/*----------------------------------------------------------------------------
 * BASE: TLMM
 *--------------------------------------------------------------------------*/

#define TLMM_BASE                                                   0x01800000
#define TLMM_BASE_SIZE                                              0x00300000
#define TLMM_BASE_PHYS                                              0x01800000

/*----------------------------------------------------------------------------
 * BASE: CORE_TOP_CSR
 *--------------------------------------------------------------------------*/

#define CORE_TOP_CSR_BASE                                           0x01b00000
#define CORE_TOP_CSR_BASE_SIZE                                      0x00040000
#define CORE_TOP_CSR_BASE_PHYS                                      0x01b00000

/*----------------------------------------------------------------------------
 * BASE: BLSP1_BLSP
 *--------------------------------------------------------------------------*/

#define BLSP1_BLSP_BASE                                             0x01b40000
#define BLSP1_BLSP_BASE_SIZE                                        0x00040000
#define BLSP1_BLSP_BASE_PHYS                                        0x01b40000

/*----------------------------------------------------------------------------
 * BASE: CE_WFSS_CE_REG
 *--------------------------------------------------------------------------*/

#define CE_WFSS_CE_REG_BASE                                         0x01b80000
#define CE_WFSS_CE_REG_BASE_SIZE                                    0x0001c000
#define CE_WFSS_CE_REG_BASE_PHYS                                    0x01b80000

/*----------------------------------------------------------------------------
 * BASE: MEMSS_CSR
 *--------------------------------------------------------------------------*/

#define MEMSS_CSR_BASE                                              0x01bc0000
#define MEMSS_CSR_BASE_SIZE                                         0x0000001c
#define MEMSS_CSR_BASE_PHYS                                         0x01bc0000

/*----------------------------------------------------------------------------
 * BASE: TSENS_SROT
 *--------------------------------------------------------------------------*/

#define TSENS_SROT_BASE                                             0x01bf0000
#define TSENS_SROT_BASE_SIZE                                        0x00001000
#define TSENS_SROT_BASE_PHYS                                        0x01bf0000

/*----------------------------------------------------------------------------
 * BASE: TSENS_TM
 *--------------------------------------------------------------------------*/

#define TSENS_TM_BASE                                               0x01bf1000
#define TSENS_TM_BASE_SIZE                                          0x00001000
#define TSENS_TM_BASE_PHYS                                          0x01bf1000

/*----------------------------------------------------------------------------
 * BASE: QDSS_APB_DEC_QDSS_APB
 *--------------------------------------------------------------------------*/

#define QDSS_APB_DEC_QDSS_APB_BASE                                  0x01c00000
#define QDSS_APB_DEC_QDSS_APB_BASE_SIZE                             0x00080000
#define QDSS_APB_DEC_QDSS_APB_BASE_PHYS                             0x01c00000

/*----------------------------------------------------------------------------
 * BASE: QDSS_APB_WCSS_DBG_DEC_QDSS_APB_WCSSDBG
 *--------------------------------------------------------------------------*/

#define QDSS_APB_WCSS_DBG_DEC_QDSS_APB_WCSSDBG_BASE                 0x01c80000
#define QDSS_APB_WCSS_DBG_DEC_QDSS_APB_WCSSDBG_BASE_SIZE            0x00080000
#define QDSS_APB_WCSS_DBG_DEC_QDSS_APB_WCSSDBG_BASE_PHYS            0x01c80000

/*----------------------------------------------------------------------------
 * BASE: QDSS_WRAPPER_TOP
 *--------------------------------------------------------------------------*/

#define QDSS_WRAPPER_TOP_BASE                                       0x01d00000
#define QDSS_WRAPPER_TOP_BASE_SIZE                                  0x0007fffd
#define QDSS_WRAPPER_TOP_BASE_PHYS                                  0x01d00000

/*----------------------------------------------------------------------------
 * BASE: PCIE_PCIE_TOP_WRAPPER
 *--------------------------------------------------------------------------*/

#define PCIE_PCIE_TOP_WRAPPER_BASE                                  0x01e00000
#define PCIE_PCIE_TOP_WRAPPER_BASE_SIZE                             0x00020000
#define PCIE_PCIE_TOP_WRAPPER_BASE_PHYS                             0x01e00000

/*----------------------------------------------------------------------------
 * BASE: SECURITY_CONTROL_WLAN
 *--------------------------------------------------------------------------*/

#define SECURITY_CONTROL_WLAN_BASE                                  0x01e20000
#define SECURITY_CONTROL_WLAN_BASE_SIZE                             0x00008000
#define SECURITY_CONTROL_WLAN_BASE_PHYS                             0x01e20000

/*----------------------------------------------------------------------------
 * BASE: EDPD_EDPD_CAL_ACC
 *--------------------------------------------------------------------------*/

#define EDPD_EDPD_CAL_ACC_BASE                                      0x01e28000
#define EDPD_EDPD_CAL_ACC_BASE_SIZE                                 0x00004000
#define EDPD_EDPD_CAL_ACC_BASE_PHYS                                 0x01e28000

/*----------------------------------------------------------------------------
 * BASE: CPR_CX_CPR3
 *--------------------------------------------------------------------------*/

#define CPR_CX_CPR3_BASE                                            0x01e30000
#define CPR_CX_CPR3_BASE_SIZE                                       0x00004000
#define CPR_CX_CPR3_BASE_PHYS                                       0x01e30000

/*----------------------------------------------------------------------------
 * BASE: CPR_MX_CPR3
 *--------------------------------------------------------------------------*/

#define CPR_MX_CPR3_BASE                                            0x01e34000
#define CPR_MX_CPR3_BASE_SIZE                                       0x00004000
#define CPR_MX_CPR3_BASE_PHYS                                       0x01e34000

/*----------------------------------------------------------------------------
 * BASE: GCC_GCC
 *--------------------------------------------------------------------------*/

#define GCC_GCC_BASE                                                0x01e40000
#define GCC_GCC_BASE_SIZE                                           0x00001000
#define GCC_GCC_BASE_PHYS                                           0x01e40000

/*----------------------------------------------------------------------------
 * BASE: PRNG_PRNG_TOP
 *--------------------------------------------------------------------------*/

#define PRNG_PRNG_TOP_BASE                                          0x01e50000
#define PRNG_PRNG_TOP_BASE_SIZE                                     0x00010000
#define PRNG_PRNG_TOP_BASE_PHYS                                     0x01e50000

/*----------------------------------------------------------------------------
 * BASE: PCNOC_0_BUS_TIMEOUT
 *--------------------------------------------------------------------------*/

#define PCNOC_0_BUS_TIMEOUT_BASE                                    0x01e60000
#define PCNOC_0_BUS_TIMEOUT_BASE_SIZE                               0x00001000
#define PCNOC_0_BUS_TIMEOUT_BASE_PHYS                               0x01e60000

/*----------------------------------------------------------------------------
 * BASE: PCNOC_1_BUS_TIMEOUT
 *--------------------------------------------------------------------------*/

#define PCNOC_1_BUS_TIMEOUT_BASE                                    0x01e61000
#define PCNOC_1_BUS_TIMEOUT_BASE_SIZE                               0x00001000
#define PCNOC_1_BUS_TIMEOUT_BASE_PHYS                               0x01e61000

/*----------------------------------------------------------------------------
 * BASE: PCNOC_2_BUS_TIMEOUT
 *--------------------------------------------------------------------------*/

#define PCNOC_2_BUS_TIMEOUT_BASE                                    0x01e62000
#define PCNOC_2_BUS_TIMEOUT_BASE_SIZE                               0x00001000
#define PCNOC_2_BUS_TIMEOUT_BASE_PHYS                               0x01e62000

/*----------------------------------------------------------------------------
 * BASE: PCNOC_3_BUS_TIMEOUT
 *--------------------------------------------------------------------------*/

#define PCNOC_3_BUS_TIMEOUT_BASE                                    0x01e63000
#define PCNOC_3_BUS_TIMEOUT_BASE_SIZE                               0x00001000
#define PCNOC_3_BUS_TIMEOUT_BASE_PHYS                               0x01e63000

/*----------------------------------------------------------------------------
 * BASE: SYSTEM_NOC
 *--------------------------------------------------------------------------*/

#define SYSTEM_NOC_BASE                                             0x01e80000
#define SYSTEM_NOC_BASE_SIZE                                        0x0000a000
#define SYSTEM_NOC_BASE_PHYS                                        0x01e80000

/*----------------------------------------------------------------------------
 * BASE: PC_NOC
 *--------------------------------------------------------------------------*/

#define PC_NOC_BASE                                                 0x01f00000
#define PC_NOC_BASE_SIZE                                            0x00004200
#define PC_NOC_BASE_PHYS                                            0x01f00000

/*----------------------------------------------------------------------------
 * BASE: WLAON_WL_AON_REG
 *--------------------------------------------------------------------------*/

#define WLAON_WL_AON_REG_BASE                                       0x01f80000
#define WLAON_WL_AON_REG_BASE_SIZE                                  0x00000708
#define WLAON_WL_AON_REG_BASE_PHYS                                  0x01f80000

/*----------------------------------------------------------------------------
 * BASE: SYSPM_SYSPM_REG
 *--------------------------------------------------------------------------*/

#define SYSPM_SYSPM_REG_BASE                                        0x01f82000
#define SYSPM_SYSPM_REG_BASE_SIZE                                   0x00001000
#define SYSPM_SYSPM_REG_BASE_PHYS                                   0x01f82000

/*----------------------------------------------------------------------------
 * BASE: PMU_WLAN_PMU
 *--------------------------------------------------------------------------*/

#define PMU_WLAN_PMU_BASE                                           0x01f88000
#define PMU_WLAN_PMU_BASE_SIZE                                      0x000000d4
#define PMU_WLAN_PMU_BASE_PHYS                                      0x01f88000

/*----------------------------------------------------------------------------
 * BASE: PMU_NOC
 *--------------------------------------------------------------------------*/

#define PMU_NOC_BASE                                                0x01f8a000
#define PMU_NOC_BASE_SIZE                                           0x00000080
#define PMU_NOC_BASE_PHYS                                           0x01f8a000

/*----------------------------------------------------------------------------
 * BASE: BT_SEC_REG_SECURITY_CONTROL_BT
 *--------------------------------------------------------------------------*/

#define BT_SEC_REG_SECURITY_CONTROL_BT_BASE                         0x01f90000
#define BT_SEC_REG_SECURITY_CONTROL_BT_BASE_SIZE                    0x00008000
#define BT_SEC_REG_SECURITY_CONTROL_BT_BASE_PHYS                    0x01f90000

/*----------------------------------------------------------------------------
 * BASE: PCIE_ATU_REGION
 *--------------------------------------------------------------------------*/

#define PCIE_ATU_REGION_BASE                                        0x04000000
#define PCIE_ATU_REGION_BASE_SIZE                                   0x100000000
#define PCIE_ATU_REGION_BASE_PHYS                                   0x04000000

/*----------------------------------------------------------------------------
 * BASE: PCIE_ATU_REGION_SIZE
 *--------------------------------------------------------------------------*/

#define PCIE_ATU_REGION_SIZE_BASE                                   0x40000000
#define PCIE_ATU_REGION_SIZE_BASE_SIZE                              0x100000000
#define PCIE_ATU_REGION_SIZE_BASE_PHYS                              0x40000000

/*----------------------------------------------------------------------------
 * BASE: PCIE_ATU_REGION_END
 *--------------------------------------------------------------------------*/

#define PCIE_ATU_REGION_END_BASE                                    0x43ffffff
#define PCIE_ATU_REGION_END_BASE_SIZE                               0x100000000
#define PCIE_ATU_REGION_END_BASE_PHYS                               0x43ffffff

/*----------------------------------------------------------------------------
 * BASE: MEM_SS_RAM_START_ADDRESS
 *--------------------------------------------------------------------------*/

#define MEM_SS_RAM_START_ADDRESS_BASE                               0x1400000
#define MEM_SS_RAM_START_ADDRESS_BASE_SIZE                          0x100000000
#define MEM_SS_RAM_START_ADDRESS_BASE_PHYS                          0x1400000

/*----------------------------------------------------------------------------
 * BASE: MEM_SS_RAM_SIZE
 *--------------------------------------------------------------------------*/
#define MEM_SS_RAM_SIZE_BASE                                        0x003a0000
#define MEM_SS_RAM_SIZE_BASE_SIZE                                   0x100000000
#define MEM_SS_RAM_SIZE_BASE_PHYS                                   0x003a0000

/*----------------------------------------------------------------------------
 * BASE: MEM_SS_RAM_END_ADDRESS
 *--------------------------------------------------------------------------*/

#define MEM_SS_RAM_END_ADDRESS_BASE                                 0x0179ffff
#define MEM_SS_RAM_END_ADDRESS_BASE_SIZE                            0x100000000
#define MEM_SS_RAM_END_ADDRESS_BASE_PHYS                            0x0179ffff

/*----------------------------------------------------------------------------
 * BASE: MEM_SS_ROM_START_ADDRESS
 *--------------------------------------------------------------------------*/

#define MEM_SS_ROM_START_ADDRESS_BASE                               0x00800000
#define MEM_SS_ROM_START_ADDRESS_BASE_SIZE                          0x100000000
#define MEM_SS_ROM_START_ADDRESS_BASE_PHYS                          0x00800000

/*----------------------------------------------------------------------------
 * BASE: MEM_SS_ROM_END_ADDRESS
 *--------------------------------------------------------------------------*/

#define MEM_SS_ROM_END_ADDRESS_BASE                                 0x008bffff
#define MEM_SS_ROM_END_ADDRESS_BASE_SIZE                            0x100000000
#define MEM_SS_ROM_END_ADDRESS_BASE_PHYS                            0x008bffff

/*----------------------------------------------------------------------------
 * BASE: MEM_SS_ROM_SIZE
 *--------------------------------------------------------------------------*/

#define MEM_SS_ROM_SIZE_BASE                                        0x000c0000
#define MEM_SS_ROM_SIZE_BASE_SIZE                                   0x100000000
#define MEM_SS_ROM_SIZE_BASE_PHYS                                   0x000c0000

/*----------------------------------------------------------------------------
 * BASE: QDSP6V67SS_WLAN
 *--------------------------------------------------------------------------*/

#define QDSP6V67SS_WLAN_BASE                                        0x00000000
#define QDSP6V67SS_WLAN_BASE_SIZE                                   0x01000000
#define QDSP6V67SS_WLAN_BASE_PHYS                                   0x00000000

#endif /* __MSMHWIOBASE_H__ */
