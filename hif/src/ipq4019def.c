/*
 * Copyright (c) 2015-2016 The Linux Foundation. All rights reserved.
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

#if defined(IPQ4019_HEADERS_DEF)
#define AR900B 1

#define WLAN_HEADERS 1
#include "common_drv.h"
#include "IPQ4019/soc_addrs.h"
#include "IPQ4019/extra/hw/apb_map.h"
#ifdef WLAN_HEADERS
#include "IPQ4019/extra/hw/wifi_top_reg_map.h"
#include "IPQ4019/hw/rtc_soc_reg.h"
#endif
#include "IPQ4019/hw/ce_wrapper_reg_csr.h"

#include "IPQ4019/extra/hw/soc_core_reg.h"
#include "IPQ4019/extra/hw/ce_reg_csr.h"
#include <IPQ4019/hw/interface/rx_location_info.h>
#include <IPQ4019/hw/interface/rx_pkt_end.h>
#include <IPQ4019/hw/interface/rx_phy_ppdu_end.h>
#include <IPQ4019/hw/interface/rx_timing_offset.h>
#include <IPQ4019/hw/interface/rx_location_info.h>
#include <IPQ4019/hw/tlv/rx_ppdu_start.h>
#include <IPQ4019/hw/tlv/rx_ppdu_end.h>
#include <IPQ4019/hw/tlv/rx_mpdu_start.h>
#include <IPQ4019/hw/tlv/rx_mpdu_end.h>
#include <IPQ4019/hw/tlv/rx_msdu_start.h>
#include <IPQ4019/hw/tlv/rx_msdu_end.h>
#include <IPQ4019/hw/tlv/rx_attention.h>
#include <IPQ4019/hw/tlv/rx_frag_info.h>
#include <IPQ4019/hw/datastruct/msdu_link_ext.h>

/* Base address is defined in pcie_local_reg.h. Macros which access the
 * registers include the base address in their definition.
 */

#define FW_EVENT_PENDING_ADDRESS (WIFICMN_SCRATCH_3_ADDRESS)
#define DRAM_BASE_ADDRESS TARG_DRAM_START

/* Backwards compatibility -- TBDXXX */

#define MISSING 0

#define WLAN_SYSTEM_SLEEP_DISABLE_LSB           WIFI_SYSTEM_SLEEP_DISABLE_LSB
#define WLAN_SYSTEM_SLEEP_DISABLE_MASK          WIFI_SYSTEM_SLEEP_DISABLE_MASK
#define WLAN_RESET_CONTROL_COLD_RST_MASK WIFI_RESET_CONTROL_MAC_COLD_RST_MASK
#define WLAN_RESET_CONTROL_WARM_RST_MASK WIFI_RESET_CONTROL_MAC_WARM_RST_MASK
#define SOC_CLOCK_CONTROL_OFFSET                SOC_CLOCK_CONTROL_ADDRESS
#define SOC_RESET_CONTROL_OFFSET                SOC_RESET_CONTROL_ADDRESS
#define CPU_CLOCK_OFFSET                        SOC_CPU_CLOCK_ADDRESS
#define SOC_LPO_CAL_OFFSET                      SOC_LPO_CAL_ADDRESS
#define SOC_RESET_CONTROL_CE_RST_MASK WIFI_RESET_CONTROL_CE_RESET_MASK
#define WLAN_SYSTEM_SLEEP_OFFSET                WIFI_SYSTEM_SLEEP_ADDRESS
#define WLAN_RESET_CONTROL_OFFSET               WIFI_RESET_CONTROL_ADDRESS
#define CLOCK_CONTROL_OFFSET                    SOC_CLOCK_CONTROL_OFFSET
#define CLOCK_CONTROL_SI0_CLK_MASK SOC_CLOCK_CONTROL_SI0_CLK_MASK
#define RESET_CONTROL_SI0_RST_MASK SOC_RESET_CONTROL_SI0_RST_MASK
#define GPIO_BASE_ADDRESS                       WLAN_GPIO_BASE_ADDRESS
#define GPIO_PIN0_OFFSET                        MISSING
#define GPIO_PIN1_OFFSET                        MISSING
#define GPIO_PIN0_CONFIG_MASK                   MISSING
#define GPIO_PIN1_CONFIG_MASK                   MISSING
#define SCRATCH_BASE_ADDRESS                    SOC_CORE_BASE_ADDRESS
#define LOCAL_SCRATCH_OFFSET                    0x18
#define GPIO_PIN10_OFFSET                       WLAN_GPIO_PIN10_ADDRESS
#define GPIO_PIN11_OFFSET                       WLAN_GPIO_PIN11_ADDRESS
#define GPIO_PIN12_OFFSET                       WLAN_GPIO_PIN12_ADDRESS
#define GPIO_PIN13_OFFSET                       WLAN_GPIO_PIN13_ADDRESS
/*TBD:dakota Check if these can be removed for dakota */
#define CPU_CLOCK_STANDARD_LSB                  SOC_CPU_CLOCK_STANDARD_LSB
#define CPU_CLOCK_STANDARD_MASK                 SOC_CPU_CLOCK_STANDARD_MASK
#define LPO_CAL_ENABLE_LSB                      SOC_LPO_CAL_ENABLE_LSB
#define LPO_CAL_ENABLE_MASK                     SOC_LPO_CAL_ENABLE_MASK
#define ANALOG_INTF_BASE_ADDRESS                WLAN_ANALOG_INTF_BASE_ADDRESS
#define MBOX_BASE_ADDRESS                       MISSING
#define INT_STATUS_ENABLE_ERROR_LSB             MISSING
#define INT_STATUS_ENABLE_ERROR_MASK            MISSING
#define INT_STATUS_ENABLE_CPU_LSB               MISSING
#define INT_STATUS_ENABLE_CPU_MASK              MISSING
#define INT_STATUS_ENABLE_COUNTER_LSB           MISSING
#define INT_STATUS_ENABLE_COUNTER_MASK          MISSING
#define INT_STATUS_ENABLE_MBOX_DATA_LSB         MISSING
#define INT_STATUS_ENABLE_MBOX_DATA_MASK        MISSING
#define ERROR_STATUS_ENABLE_RX_UNDERFLOW_LSB    MISSING
#define ERROR_STATUS_ENABLE_RX_UNDERFLOW_MASK   MISSING
#define ERROR_STATUS_ENABLE_TX_OVERFLOW_LSB     MISSING
#define ERROR_STATUS_ENABLE_TX_OVERFLOW_MASK    MISSING
#define COUNTER_INT_STATUS_ENABLE_BIT_LSB       MISSING
#define COUNTER_INT_STATUS_ENABLE_BIT_MASK      MISSING
#define INT_STATUS_ENABLE_ADDRESS               MISSING
#define CPU_INT_STATUS_ENABLE_BIT_LSB           MISSING
#define CPU_INT_STATUS_ENABLE_BIT_MASK          MISSING
#define HOST_INT_STATUS_ADDRESS                 MISSING
#define CPU_INT_STATUS_ADDRESS                  MISSING
#define ERROR_INT_STATUS_ADDRESS                MISSING
#define ERROR_INT_STATUS_WAKEUP_MASK            MISSING
#define ERROR_INT_STATUS_WAKEUP_LSB             MISSING
#define ERROR_INT_STATUS_RX_UNDERFLOW_MASK      MISSING
#define ERROR_INT_STATUS_RX_UNDERFLOW_LSB       MISSING
#define ERROR_INT_STATUS_TX_OVERFLOW_MASK       MISSING
#define ERROR_INT_STATUS_TX_OVERFLOW_LSB        MISSING
#define COUNT_DEC_ADDRESS                       MISSING
#define HOST_INT_STATUS_CPU_MASK                MISSING
#define HOST_INT_STATUS_CPU_LSB                 MISSING
#define HOST_INT_STATUS_ERROR_MASK              MISSING
#define HOST_INT_STATUS_ERROR_LSB               MISSING
#define HOST_INT_STATUS_COUNTER_MASK            MISSING
#define HOST_INT_STATUS_COUNTER_LSB             MISSING
#define RX_LOOKAHEAD_VALID_ADDRESS              MISSING
#define WINDOW_DATA_ADDRESS                     MISSING
#define WINDOW_READ_ADDR_ADDRESS                MISSING
#define WINDOW_WRITE_ADDR_ADDRESS               MISSING
/* MAC Descriptor */
#define RX_PPDU_END_ANTENNA_OFFSET_DWORD (RX_PPDU_END_25_RX_ANTENNA_OFFSET >> 2)
/* GPIO Register */
#define GPIO_ENABLE_W1TS_LOW_ADDRESS            MISSING
#define GPIO_PIN0_CONFIG_LSB                    MISSING
#define GPIO_PIN0_PAD_PULL_LSB                  MISSING
#define GPIO_PIN0_PAD_PULL_MASK                 MISSING
/* SI reg */
#define SI_CONFIG_ERR_INT_MASK                  MISSING
#define SI_CONFIG_ERR_INT_LSB                   MISSING
/* CE descriptor */
#define CE_SRC_DESC_SIZE_DWORD         2
#define CE_DEST_DESC_SIZE_DWORD        2
#define CE_SRC_DESC_SRC_PTR_OFFSET_DWORD    0
#define CE_SRC_DESC_INFO_OFFSET_DWORD       1
#define CE_DEST_DESC_DEST_PTR_OFFSET_DWORD  0
#define CE_DEST_DESC_INFO_OFFSET_DWORD      1
#if _BYTE_ORDER == _BIG_ENDIAN
#define CE_SRC_DESC_INFO_NBYTES_MASK               0xFFFF0000
#define CE_SRC_DESC_INFO_NBYTES_SHIFT              16
#define CE_SRC_DESC_INFO_GATHER_MASK               0x00008000
#define CE_SRC_DESC_INFO_GATHER_SHIFT              15
#define CE_SRC_DESC_INFO_BYTE_SWAP_MASK            0x00004000
#define CE_SRC_DESC_INFO_BYTE_SWAP_SHIFT           14
#define CE_SRC_DESC_INFO_HOST_INT_DISABLE_MASK     0x00002000
#define CE_SRC_DESC_INFO_HOST_INT_DISABLE_SHIFT    13
#define CE_SRC_DESC_INFO_TARGET_INT_DISABLE_MASK   0x00001000
#define CE_SRC_DESC_INFO_TARGET_INT_DISABLE_SHIFT  12
#define CE_SRC_DESC_INFO_META_DATA_MASK            0x00000FFF
#define CE_SRC_DESC_INFO_META_DATA_SHIFT           0
#else
#define CE_SRC_DESC_INFO_NBYTES_MASK               0x0000FFFF
#define CE_SRC_DESC_INFO_NBYTES_SHIFT              0
#define CE_SRC_DESC_INFO_GATHER_MASK               0x00010000
#define CE_SRC_DESC_INFO_GATHER_SHIFT              16
#define CE_SRC_DESC_INFO_BYTE_SWAP_MASK            0x00020000
#define CE_SRC_DESC_INFO_BYTE_SWAP_SHIFT           17
#define CE_SRC_DESC_INFO_HOST_INT_DISABLE_MASK     0x00040000
#define CE_SRC_DESC_INFO_HOST_INT_DISABLE_SHIFT    18
#define CE_SRC_DESC_INFO_TARGET_INT_DISABLE_MASK   0x00080000
#define CE_SRC_DESC_INFO_TARGET_INT_DISABLE_SHIFT  19
#define CE_SRC_DESC_INFO_META_DATA_MASK            0xFFF00000
#define CE_SRC_DESC_INFO_META_DATA_SHIFT           20
#endif
#if _BYTE_ORDER == _BIG_ENDIAN
#define CE_DEST_DESC_INFO_NBYTES_MASK              0xFFFF0000
#define CE_DEST_DESC_INFO_NBYTES_SHIFT             16
#define CE_DEST_DESC_INFO_GATHER_MASK              0x00008000
#define CE_DEST_DESC_INFO_GATHER_SHIFT             15
#define CE_DEST_DESC_INFO_BYTE_SWAP_MASK           0x00004000
#define CE_DEST_DESC_INFO_BYTE_SWAP_SHIFT          14
#define CE_DEST_DESC_INFO_HOST_INT_DISABLE_MASK    0x00002000
#define CE_DEST_DESC_INFO_HOST_INT_DISABLE_SHIFT   13
#define CE_DEST_DESC_INFO_TARGET_INT_DISABLE_MASK  0x00001000
#define CE_DEST_DESC_INFO_TARGET_INT_DISABLE_SHIFT 12
#define CE_DEST_DESC_INFO_META_DATA_MASK           0x00000FFF
#define CE_DEST_DESC_INFO_META_DATA_SHIFT          0
#else
#define CE_DEST_DESC_INFO_NBYTES_MASK              0x0000FFFF
#define CE_DEST_DESC_INFO_NBYTES_SHIFT             0
#define CE_DEST_DESC_INFO_GATHER_MASK              0x00010000
#define CE_DEST_DESC_INFO_GATHER_SHIFT             16
#define CE_DEST_DESC_INFO_BYTE_SWAP_MASK           0x00020000
#define CE_DEST_DESC_INFO_BYTE_SWAP_SHIFT          17
#define CE_DEST_DESC_INFO_HOST_INT_DISABLE_MASK    0x00040000
#define CE_DEST_DESC_INFO_HOST_INT_DISABLE_SHIFT   18
#define CE_DEST_DESC_INFO_TARGET_INT_DISABLE_MASK  0x00080000
#define CE_DEST_DESC_INFO_TARGET_INT_DISABLE_SHIFT 19
#define CE_DEST_DESC_INFO_META_DATA_MASK           0xFFF00000
#define CE_DEST_DESC_INFO_META_DATA_SHIFT          20
#endif

#define MY_TARGET_DEF IPQ4019_TARGETdef
#define MY_HOST_DEF IPQ4019_HOSTdef
#define MY_CEREG_DEF IPQ4019_CE_TARGETdef
#define MY_TARGET_BOARD_DATA_SZ IPQ4019_BOARD_DATA_SZ
#define MY_TARGET_BOARD_EXT_DATA_SZ IPQ4019_BOARD_EXT_DATA_SZ
#include "targetdef.h"
#include "hostdef.h"
#else
#include "common_drv.h"
#include "targetdef.h"
#include "hostdef.h"
struct targetdef_s *IPQ4019_TARGETdef;
struct hostdef_s *IPQ4019_HOSTdef;
#endif /* IPQ4019_HEADERS_DEF */
