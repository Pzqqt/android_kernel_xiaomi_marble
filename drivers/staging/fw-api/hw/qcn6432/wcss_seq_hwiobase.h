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
#ifndef __WCSS_SEQ_HWIOBASE_H__
#define __WCSS_SEQ_HWIOBASE_H__
/*----------------------------------------------------------------------------
 * BASE: WCSS_CFGBUS
 *--------------------------------------------------------------------------*/

#define WCSS_CFGBUS_BASE                                            0x00008000
#define WCSS_CFGBUS_BASE_SIZE                                       0x00004000
#define WCSS_CFGBUS_BASE_PHYS                                       0x00008000

/*----------------------------------------------------------------------------
 * BASE: UMAC_NOC
 *--------------------------------------------------------------------------*/

#define UMAC_NOC_BASE                                               0x00140000
#define UMAC_NOC_BASE_SIZE                                          0x00004200
#define UMAC_NOC_BASE_PHYS                                          0x00140000

/*----------------------------------------------------------------------------
 * BASE: PHYA0
 *--------------------------------------------------------------------------*/

#define PHYA0_BASE                                                  0x00300000
#define PHYA0_BASE_SIZE                                             0x00300000
#define PHYA0_BASE_PHYS                                             0x00300000

/*----------------------------------------------------------------------------
 * BASE: DMAC
 *--------------------------------------------------------------------------*/

#define DMAC_BASE                                                   0x00900000
#define DMAC_BASE_SIZE                                              0x00080000
#define DMAC_BASE_PHYS                                              0x00900000

/*----------------------------------------------------------------------------
 * BASE: UMAC
 *--------------------------------------------------------------------------*/

#define UMAC_BASE                                                   0x00a00000
#define UMAC_BASE_SIZE                                              0x0004d000
#define UMAC_BASE_PHYS                                              0x00a00000

/*----------------------------------------------------------------------------
 * BASE: PMAC0
 *--------------------------------------------------------------------------*/

#define PMAC0_BASE                                                  0x00a80000
#define PMAC0_BASE_SIZE                                             0x00040000
#define PMAC0_BASE_PHYS                                             0x00a80000

/*----------------------------------------------------------------------------
 * BASE: MAC_WSIB
 *--------------------------------------------------------------------------*/

#define MAC_WSIB_BASE                                               0x00b3c000
#define MAC_WSIB_BASE_SIZE                                          0x00004000
#define MAC_WSIB_BASE_PHYS                                          0x00b3c000

/*----------------------------------------------------------------------------
 * BASE: CXC
 *--------------------------------------------------------------------------*/

#define CXC_BASE                                                    0x00b40000
#define CXC_BASE_SIZE                                               0x00010000
#define CXC_BASE_PHYS                                               0x00b40000

/*----------------------------------------------------------------------------
 * BASE: WFSS_PMM
 *--------------------------------------------------------------------------*/

#define WFSS_PMM_BASE                                               0x00b50000
#define WFSS_PMM_BASE_SIZE                                          0x00002401
#define WFSS_PMM_BASE_PHYS                                          0x00b50000

/*----------------------------------------------------------------------------
 * BASE: WFSS_CC
 *--------------------------------------------------------------------------*/

#define WFSS_CC_BASE                                                0x00b60000
#define WFSS_CC_BASE_SIZE                                           0x00008000
#define WFSS_CC_BASE_PHYS                                           0x00b60000

/*----------------------------------------------------------------------------
 * BASE: WCMN_CORE
 *--------------------------------------------------------------------------*/

#define WCMN_CORE_BASE                                              0x00b68000
#define WCMN_CORE_BASE_SIZE                                         0x000008a9
#define WCMN_CORE_BASE_PHYS                                         0x00b68000

/*----------------------------------------------------------------------------
 * BASE: WIFI_CFGBUS_APB_TSLV
 *--------------------------------------------------------------------------*/

#define WIFI_CFGBUS_APB_TSLV_BASE                                   0x00b6b000
#define WIFI_CFGBUS_APB_TSLV_BASE_SIZE                              0x00001000
#define WIFI_CFGBUS_APB_TSLV_BASE_PHYS                              0x00b6b000

/*----------------------------------------------------------------------------
 * BASE: WFSS_CFGBUS
 *--------------------------------------------------------------------------*/

#define WFSS_CFGBUS_BASE                                            0x00b6c000
#define WFSS_CFGBUS_BASE_SIZE                                       0x000000a0
#define WFSS_CFGBUS_BASE_PHYS                                       0x00b6c000

/*----------------------------------------------------------------------------
 * BASE: WIFI_CFGBUS_AHB_TSLV
 *--------------------------------------------------------------------------*/

#define WIFI_CFGBUS_AHB_TSLV_BASE                                   0x00b6d000
#define WIFI_CFGBUS_AHB_TSLV_BASE_SIZE                              0x00001000
#define WIFI_CFGBUS_AHB_TSLV_BASE_PHYS                              0x00b6d000

/*----------------------------------------------------------------------------
 * BASE: UMAC_ACMT
 *--------------------------------------------------------------------------*/

#define UMAC_ACMT_BASE                                              0x00b6e000
#define UMAC_ACMT_BASE_SIZE                                         0x00001000
#define UMAC_ACMT_BASE_PHYS                                         0x00b6e000

/*----------------------------------------------------------------------------
 * BASE: WCSS_CC
 *--------------------------------------------------------------------------*/

#define WCSS_CC_BASE                                                0x00b80000
#define WCSS_CC_BASE_SIZE                                           0x00010000
#define WCSS_CC_BASE_PHYS                                           0x00b80000

/*----------------------------------------------------------------------------
 * BASE: PMM_TOP
 *--------------------------------------------------------------------------*/

#define PMM_TOP_BASE                                                0x00b90000
#define PMM_TOP_BASE_SIZE                                           0x00010000
#define PMM_TOP_BASE_PHYS                                           0x00b90000

/*----------------------------------------------------------------------------
 * BASE: WCSS_TOP_CMN
 *--------------------------------------------------------------------------*/

#define WCSS_TOP_CMN_BASE                                           0x00ba0000
#define WCSS_TOP_CMN_BASE_SIZE                                      0x00004000
#define WCSS_TOP_CMN_BASE_PHYS                                      0x00ba0000

/*----------------------------------------------------------------------------
 * BASE: WCSS_IE
 *--------------------------------------------------------------------------*/

#define WCSS_IE_BASE                                                0x00ba4000
#define WCSS_IE_BASE_SIZE                                           0x00001000
#define WCSS_IE_BASE_PHYS                                           0x00ba4000

/*----------------------------------------------------------------------------
 * BASE: MSIP
 *--------------------------------------------------------------------------*/

#define MSIP_BASE                                                   0x00bb0000
#define MSIP_BASE_SIZE                                              0x00010000
#define MSIP_BASE_PHYS                                              0x00bb0000

/*----------------------------------------------------------------------------
 * BASE: DBG
 *--------------------------------------------------------------------------*/

#define DBG_BASE                                                    0x01000000
#define DBG_BASE_SIZE                                               0x00100000
#define DBG_BASE_PHYS                                               0x01000000


#endif /* __WCSS_SEQ_HWIOBASE_H__ */
