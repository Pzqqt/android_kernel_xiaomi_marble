/*
 * Copyright (c) 2011-2018 The Linux Foundation. All rights reserved.
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

/**
 * DOC: wlan_hdd_wext.c
 *
 * Linux Wireless Extensions Implementation
 */

#include <linux/version.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/wireless.h>
#include <mac_trace.h>
#include <wlan_hdd_includes.h>
#include <cds_api.h>
#include "scheduler_api.h"
#include <net/arp.h>
#include <cdp_txrx_cmn.h>
#include <cdp_txrx_stats.h>
#include "sir_params.h"
#include "csr_api.h"
#include "csr_inside_api.h"
#include "sme_rrm_internal.h"
#include <ani_global.h>
#include "dot11f.h"
#include <wlan_hdd_wowl.h>
#include <wlan_hdd_cfg.h>
#include <wlan_hdd_wmm.h>
#include "utils_api.h"
#include "wlan_hdd_p2p.h"
#ifdef FEATURE_WLAN_TDLS
#include "wlan_hdd_tdls.h"
#endif

#include "cds_ieee80211_common.h"
#include "ol_if_athvar.h"
#include "dbglog_host.h"
#include "wma.h"

#include "wlan_hdd_power.h"
#include "qwlan_version.h"
#include "wlan_hdd_host_offload.h"

#include <linux/wireless.h>
#include <net/cfg80211.h>

#include "wlan_hdd_misc.h"

#include "qc_sap_ioctl.h"
#include "sme_api.h"
#include "wma_types.h"
#include "qdf_str.h"
#include "qdf_trace.h"
#include "wlan_hdd_assoc.h"
#include "wlan_hdd_ioctl.h"
#include "wlan_hdd_scan.h"
#include "sme_power_save_api.h"
#include "wlan_policy_mgr_api.h"
#include "wlan_hdd_conc_ut.h"
#include "wlan_hdd_fips.h"
#include "wlan_hdd_tsf.h"
#include "wlan_hdd_ocb.h"
#include "wlan_hdd_napi.h"
#include "cdp_txrx_flow_ctrl_legacy.h"
#include "wlan_hdd_nan_datapath.h"
#include "wlan_hdd_stats.h"
#ifdef WLAN_SUSPEND_RESUME_TEST
#include "wlan_hdd_driver_ops.h"
#include "hif.h"
#include "pld_common.h"
#endif
#include "wlan_hdd_lro.h"
#include "cds_utils.h"
#include "wlan_osif_request_manager.h"
#include "os_if_wifi_pos.h"
#include <cdp_txrx_stats.h>
#include <cds_api.h>
#include "wlan_dsc_test.h"
#include <wlan_osif_priv.h>
#include "wlan_hdd_regulatory.h"
#include "wlan_reg_ucfg_api.h"
#include "wlan_hdd_packet_filter_api.h"
#include "wlan_cp_stats_mc_ucfg_api.h"
#include "wlan_mlme_ucfg_api.h"
#include "cfg_mlme_sta.h"
#include "cfg_ucfg_api.h"

#define HDD_FINISH_ULA_TIME_OUT         800
#define HDD_SET_MCBC_FILTERS_TO_FW      1
#define HDD_DELETE_MCBC_FILTERS_FROM_FW 0

/* Private ioctls and their sub-ioctls */
#define WLAN_PRIV_SET_INT_GET_NONE    (SIOCIWFIRSTPRIV + 0)
#define WE_SET_11D_STATE     1
#define WE_WOWL              2
#define WE_SET_POWER         3
/*
 * <ioctl>
 * setMaxAssoc - Sets the maximum number of associated stations
 *
 * @INPUT: 1 to 32
 *
 * @OUTPUT: None
 *
 * This IOTCL sets the maximum number of associated stations
 *
 * @E.g: iwpriv wlan0 setMaxAssoc <value>
 *
 * Supported Feature: STA
 *
 * Usage: Internal/External
 *
 * </ioctl>
 */
#define WE_SET_MAX_ASSOC     4
/*
 * <ioctl>
 * scan_disable - Disable scan
 *
 * @INPUT: set_value
 *
 * @OUTPUT: None
 *
 * This IOCTL is used to set disable scan
 *
 * @E.g: iwpriv wlan0 scan_disable 1
 *
 * Supported Feature: Scan
 *
 * Usage: Internal/External
 *
 * </ioctl>
 */
#define WE_SET_SCAN_DISABLE  5
/*
 * <ioctl>
 * inactivityTO - sets the timeout value for inactivity data while
 * in power save mode
 *
 * @INPUT: int1…..int255
 *
 * @OUTPUT: None
 *
 * This IOCTL set the timeout value for inactivity data in power save mode
 *
 * @E.g: iwpriv wlan0 inactivityTO 20
 *
 * Supported Feature: STA
 *
 * Usage: Internal/External
 *
 * </ioctl>
 */
#define WE_SET_DATA_INACTIVITY_TO  6
/*
 * <ioctl>
 * setMaxTxPower - Dynamically sets the maximum transmission power
 *
 * @INPUT: Transmission power in dBm
 *
 * @OUTPUT: None
 *
 * This IOCTL dynamically sets the maximum transmission power
 * This setting does not persist over reboots
 *
 * @E.g: iwpriv wlan0 setMaxTxPower <value in db)
 *
 * Supported Feature: STA
 *
 * Usage: Internal/External
 *
 * </ioctl>
 */
#define WE_SET_MAX_TX_POWER  7
/* 8 is unused */
#define WE_SET_TM_LEVEL      9
/*
 * <ioctl>
 * setphymode - Set the phymode dynamically
 *
 * @INPUT: 0 IEEE80211_MODE_AUTO to 22 IEEE80211_MODE_11AGN
 *
 * @OUTPUT: None
 *
 * This IOCTL sets the phymode dynamically
 *
 * @E.g: iwpriv wlan0 setphymode 10
 *
 * Supported Feature: STA
 *
 * Usage: Internal/External
 *
 * </ioctl>
 */
#define WE_SET_PHYMODE       10
/*
 * <ioctl>
 * nss - Set the number of spatial streams
 *
 * @INPUT: int1…..int3
 *
 * @OUTPUT: None
 *
 * This IOCTL sets the number of spatial streams. Supported values are 1 and 2
 *
 * @E.g: iwpriv wlan0 nss 2
 *
 * Supported Feature: STA
 *
 * Usage: Internal/External
 *
 * </ioctl>
 */
#define WE_SET_NSS           11
/*
 * <ioctl>
 * ldpc - Enables or disables LDPC
 *
 * @INPUT: 0 – Disable, 1 - Enable
 *
 * @OUTPUT: None
 *
 * This IOCTL enables or disables LDPC
 *
 * @E.g: iwpriv wlan0 ldpc 1
 *
 * Supported Feature: STA
 *
 * Usage: Internal/External
 *
 * </ioctl>
 */
#define WE_SET_LDPC          12
/*
 * <ioctl>
 * tx_stbc - Enables or disables tx_stbc
 *
 * @INPUT: Int 0 – Disable, 1 - Enable
 *
 * @OUTPUT: None
 *
 * This IOTCL used to enables or disables tx_stbc
 *
 * @E.g: iwpriv wlan0 tx_stbc <value>
 *
 * Supported Feature: STA
 *
 * Usage: Internal/External
 *
 * </ioctl>
 */
#define WE_SET_TX_STBC       13
/*
 * <ioctl>
 * rx_stbc - Set the rx_stbc parameter
 *
 * @INPUT: Int 0 – Disable, 1 - Enable
 *
 * @OUTPUT: None
 *
 * This IOTCL used to set rx_stbc parameter
 *
 * @E.g: iwpriv wlan0 rx_stbc <value>
 *
 * Supported Feature: STA
 *
 * Usage: Internal/External
 *
 * </ioctl>
 */
#define WE_SET_RX_STBC       14
/*
 * <ioctl>
 * shortgi  - Sets the short-guard interval
 *
 * @INPUT: Fixed Rate: 0 - 400ns, 1 - 800ns, 2 - 1600ns, 3 - 3200us
 * Auto Rate: 8 - 400ns, 9 - 800ns, 10 - 1600ns, 11 - 3200us
 *
 * @OUTPUT: None
 *
 * This IOCTL sets the short-guard interval.
 *
 * @E.g: iwpriv wlan0 shortgi <value>
 *
 * Supported Feature: STA
 *
 * Usage: Internal/External
 *
 * </ioctl>
 */
#define WE_SET_SHORT_GI      15
/*
 * <ioctl>
 * enablertscts - enables or disables rts/cts.
 *
 * @INPUT: 1-Enable , 0-Disable
 *
 * @OUTPUT: None
 *
 * This IOCTL enables or disables rts/cts.
 *
 * @E.g: iwpriv wlan0 enablertscts <value>
 *
 * Supported Feature: STA
 *
 * Usage: Internal/External
 *
 * </ioctl>
 */
#define WE_SET_RTSCTS        16
/*
 * <ioctl>
 * chwidth - Set the channel bandwidth
 *
 * @INPUT: 0-20mhz to 3-160mhz
 *
 * @OUTPUT: None
 *
 * This IOTCL used to set the channel bandwidth
 *
 * @E.g: iwpriv wlan0 chwidth 1
 *
 * Supported Feature: STA
 *
 * Usage: Internal/External
 *
 * </ioctl>
 */
#define WE_SET_CHWIDTH       17
#define WE_SET_ANI_EN_DIS    18
#define WE_SET_ANI_POLL_PERIOD    19
#define WE_SET_ANI_LISTEN_PERIOD  20
#define WE_SET_ANI_OFDM_LEVEL     21
#define WE_SET_ANI_CCK_LEVEL      22
/*
 * <ioctl>
 * cwmenable - Enables or disables the dynamic channel bandwidth
 *
 * @INPUT: 0-Disable, 1-Enable
 *
 * @OUTPUT: None
 *
 * This IOTCL used to enables or disables the dynamic channel bandwidth
 *
 * @E.g: iwpriv wlan0 cwmenable <value>
 *
 * Supported Feature: STA
 *
 * Usage: Internal/External
 *
 * </ioctl>
 */
#define WE_SET_DYNAMIC_BW         23
/*
 * <ioctl>
 * txchainmask - This IOCTL sets the current Tx chain mask
 *
 * @INPUT: Mask Value
 *
 * @OUTPUT: None
 *
 * This IOCTL sets the current Tx chain mask
 *
 * @E.g: iwpriv wlan0 txchainmask 1
 *
 * Supported Feature: STA
 *
 * Usage: Internal/External
 *
 * </ioctl>
 */
#define WE_SET_TX_CHAINMASK  24
/*
 * <ioctl>
 * rxchainmask - Sets the current Rx chain mask
 *
 * @INPUT: Mask Value
 *
 * @OUTPUT: None
 *
 * This IOCTL sets the current Rx chain mask. This command is the
 * equivalent to setting in gSetRxChainmask1x1 in WCNSS_qcom_cfg.ini.
 *
 * @E.g: iwpriv wlan0 rxchainmask <value>
 *
 * Supported Feature: STA
 *
 * Usage: Internal/External
 *
 * </ioctl>
 */
#define WE_SET_RX_CHAINMASK  25
/*
 * <ioctl>
 * set11NRates - Fixes the Tx data rate of the 11N mode.
 *
 * @INPUT: 0x1b to 0x8f
 *
 * @OUTPUT: None
 *
 * This IOCTL fixes the Tx data rate of the 11N mode.
 *
 * @E.g: iwpriv wlan0 set11NRates 0x85
 *
 * Supported Feature: STA
 *
 * Usage: Internal/External
 *
 * </ioctl>
 */
#define WE_SET_11N_RATE      26
/*
 * <ioctl>
 * ampdu  - Set the the maximum subframe of ampdu
 *
 * @INPUT: int 1 to int 63
 *
 * @OUTPUT: None
 *
 * This IOCTL sets the maximum subframe of ampdu.
 *
 * @E.g: iwpriv wlan0 ampdu 9
 *
 * Supported Feature: STA
 *
 * Usage: Internal/External
 *
 * </ioctl>
 */
#define WE_SET_AMPDU         27
/*
 * <ioctl>
 * amsdu - Sets the maximum subframe of amsdu.
 *
 * @INPUT: int 1 to int 31
 *
 * @OUTPUT: None
 *
 * This IOCTL sets the maximum subframe of amsdu.
 *
 * @E.g: iwpriv wlan0 amsdu 9
 *
 * Supported Feature: STA
 *
 * Usage: Internal/External
 *
 * </ioctl>
 */
#define WE_SET_AMSDU         28
/*
 * <ioctl>
 * txpow2g - current 2 GHz Tx power setting
 *
 * @INPUT: Tx power in dBm
 *
 * @OUTPUT: None
 *
 * This IOTCL  used to set 2 ghz tx power
 *
 * @E.g: iwpriv wlan0 txpow2g
 *
 * Supported Feature: STA
 *
 * Usage: Internal/External
 *
 * </ioctl>
 */
#define WE_SET_TXPOW_2G      29
/*
 * <ioctl>
 * txpow5g - Current 5 GHz tx power setting
 *
 * @INPUT: Tx power in dBm
 *
 * @OUTPUT: None
 *
 * This IOTCL used to set the 5 ghz txpower
 *
 * @E.g: iwpriv wlan0 txpow5g
 *
 * Supported Feature: STA
 *
 * Usage: Internal/External
 *
 * </ioctl>
 */
#define WE_SET_TXPOW_5G      30
/* Private ioctl for firmware debug log */
#define WE_DBGLOG_LOG_LEVEL             31
#define WE_DBGLOG_VAP_ENABLE            32
#define WE_DBGLOG_VAP_DISABLE           33
#define WE_DBGLOG_MODULE_ENABLE         34
#define WE_DBGLOG_MODULE_DISABLE        35
#define WE_DBGLOG_MOD_LOG_LEVEL         36
#define WE_DBGLOG_TYPE                  37
#define WE_SET_TXRX_FWSTATS             38
/*
 * <ioctl>
 * set11ACRates  - Fixes the Tx data rate of 11AC
 *
 * @INPUT: 0x1 to 0x9
 *
 * @OUTPUT: None
 *
 * This IOCTL fixes the Tx data rate of 11AC.
 *
 * @E.g: iwpriv wlan0 set11ACRates 0x9
 *
 * Supported Feature: STA
 *
 * Usage: Internal/External
 *
 * </ioctl>
 */
#define WE_SET_VHT_RATE                 39
#define WE_DBGLOG_REPORT_ENABLE         40
#define WE_TXRX_FWSTATS_RESET           41
/*
 * <ioctl>
 * setTxMaxPower2G - Set the maximum transmit power for the 2.4-GHz band
 *
 * @INPUT: Transmission power in dBm
 *
 * @OUTPUT: None
 *
 * This IOCTL sets the maximum transmit power for the 2.4-GHz band
 * This setting does not persist over reboots
 *
 * @E.g: iwpriv wlan0 setTxMaxPower2G 10
 *
 * Supported Feature: STA
 *
 * Usage: Internal/External
 *
 * </ioctl>
 */
#define WE_SET_MAX_TX_POWER_2_4   42
/*
 * <ioctl>
 * setTxMaxPower5G - Set the maximum transmit power for the 5-GHz band
 *
 * @INPUT: Transmission power in dBm
 *
 * @OUTPUT: None
 *
 * This IOCTL sets the maximum transmit power for the 5-GHz band
 * This setting does not persist over reboots
 *
 * @E.g: iwpriv wlan0 setTxMaxPower5G 10
 *
 * Supported Feature: STA
 *
 * Usage: Internal/External
 *
 * </ioctl>
 */
#define WE_SET_MAX_TX_POWER_5_0   43
#define WE_SET_PKTLOG                   44
/* Private ioctl for packet power save */
#define  WE_PPS_PAID_MATCH              45
#define  WE_PPS_GID_MATCH               46
#define  WE_PPS_EARLY_TIM_CLEAR         47
#define  WE_PPS_EARLY_DTIM_CLEAR        48
#define  WE_PPS_EOF_PAD_DELIM           49
#define  WE_PPS_MACADDR_MISMATCH        50
#define  WE_PPS_DELIM_CRC_FAIL          51
#define  WE_PPS_GID_NSTS_ZERO           52
/*
 * <ioctl>
 * rssi_chk - Check the rssi
 *
 * @INPUT: One argument as input
 *
 * @OUTPUT: rssi
 *  wlan0	rssi_chk:56
 *
 * This IOTCL used to chek rssi
 *
 * @E.g: iwpriv wlan0 rssi_chk <value>
 *
 * Supported Feature: STA
 *
 * Usage: Internal/External
 *
 * </ioctl>
 */
#define  WE_PPS_RSSI_CHECK              53
/*
 * <ioctl>
 * htsmps - Sets the htsmps
 *
 * @INPUT: Atleast one int argument
 *
 * @OUTPUT: None
 *
 * This IOTCL used to set htsmps
 *
 * @E.g: iwpriv wlan0 htsmps <value>
 *
 * Supported Feature: STA
 *
 * Usage: Internal/External
 *
 * </ioctl>
 */
#define WE_SET_HTSMPS                   55
/* Private ioctl for QPower */
#define WE_SET_QPOWER_MAX_PSPOLL_COUNT            56
#define WE_SET_QPOWER_MAX_TX_BEFORE_WAKE          57
#define WE_SET_QPOWER_SPEC_PSPOLL_WAKE_INTERVAL   58
#define WE_SET_QPOWER_SPEC_MAX_SPEC_NODATA_PSPOLL 59
/* GTX Commands */
/*
 * <ioctl>
 * gtxHTMcs - Set the tx HTM value
 *
 * @INPUT: Atleast one int orgument
 *
 * @OUTPUT: None
 *
 * This IOTCL sets htm tx value
 *
 * @E.g: iwpriv wlan0 gtxHTMcs <value>
 *
 * Supported Feature: STA
 *
 * Usage: Internal/External
 *
 * </ioctl>
 */
#define WE_SET_GTX_HT_MCS               62
/*
 * <ioctl>
 * gtxVHTMcs - Set gtxVHTMcs value
 *
 * @INPUT: Atleast one int argument
 *
 * @OUTPUT: None
 *
 * This IOTCL used to set gtxVHTMcs value
 *
 * @E.g: iwpriv wlan0 gtxVHTMcs <value>
 *
 * Supported Feature: STA
 *
 * Usage: Internal/External
 *
 * </ioctl>
 */
#define WE_SET_GTX_VHT_MCS              63
/*
 * <ioctl>
 * gtxUsrCfg - Host request for GTX mask
 *
 * @INPUT: Atleast one int orgument
 *
 * @OUTPUT: None
 *
 * This IOTCL used send the host request for GTX mask
 *
 * @E.g: iwpriv wlan0 gtxUsrCfg <value>
 *
 * Supported Feature: STA
 *
 * Usage: Internal/External
 *
 * </ioctl>
 */
#define WE_SET_GTX_USRCFG               64
/*
 * <ioctl>
 * gtxThre - Set the tx threshold
 *
 * @INPUT: Atleast one int argument
 *
 * @OUTPUT: None
 *
 * This IOTCL used to set tx threshold
 *
 * @E.g: iwpriv wlan0 gtxThre <value>
 *
 * Supported Feature: STA
 *
 * Usage: Internal/External
 *
 * </ioctl>
 */
#define WE_SET_GTX_THRE                 65
/*
 * <ioctl>
 * gtxMargin  - Set the gtxMargin
 *
 * @INPUT: 1 to 32
 *
 * @OUTPUT: None
 *
 * This IOTCL use dto set gtxMargin
 *
 * @E.g: iwpriv wlan0 gtxMargini <value>
 *
 * Supported Feature: STA
 *
 * Usage: Internal/External
 *
 * </ioctl>
 */
#define WE_SET_GTX_MARGIN               66
/*
 * <ioctl>
 * gtxStep - Set the gtxStep
 *
 * @INPUT: None
 *
 * @OUTPUT: None
 *
 * This IOTCL used to sets gtxStep
 *
 * @E.g: iwpriv wlan0 gtxStep <value>
 *
 * Supported Feature: STA
 *
 * Usage: Internal/External
 *
 * </ioctl>
 */
#define WE_SET_GTX_STEP                 67
/*
 * <ioctl>
 * gtxMinTpc - Sets the gtxMinTpc
 *
 * @INPUT: Atleast one int argument
 *
 * @OUTPUT: None
 *
 * This IOTCL sets the tx MinTpc
 *
 * @E.g: iwpriv wlan0 gtxMinTpc <value>
 *
 * Supported Feature: STA
 *
 * Usage: Internal/External
 *
 * </ioctl>
 */
#define WE_SET_GTX_MINTPC               68
/*
 * <ioctl>
 * gtxBWMask - Sets the BW mask (20/40/80/160 Mhz)
 *
 * @INPUT: Mask value
 *
 * @OUTPUT: None
 *
 * This IOTCL used to set gtxBWMask
 *
 * @E.g: iwpriv wlan0 gtxBWMask <value>
 *
 * Supported Feature: STA
 *
 * Usage: Internal/External
 *
 * </ioctl>
 */

#define WE_SET_GTX_BWMASK               69
/*
 * <ioctl>
 * setMccLatency - Sets the MCC latency value during STA-P2P concurrency
 *
 * @INPUT: set_value
 *
 * @OUTPUT: None
 *
 * This IOCTL is used to set the MCC latency value in milliseconds
 * during STA-P2P concurrency.
 *
 * If 0ms latency is provided, then FW will set to a default.
 * Otherwise, latency must be at least 30ms.
 *
 * @E.g: iwpriv wlan0 setMccLatency 40
 *
 *
 * Supported Feature: Concurrency
 *
 * Usage: Internal/External
 *
 * </ioctl>
 */
#define WE_MCC_CONFIG_LATENCY           70

/*
 * <ioctl>
 * setMccQuota- Set the quota for P2P cases
 *
 * @INPUT: set_value [0,100]
 *
 * @OUTPUT: None
 *
 * This IOCTL is used to set the quota in milliseconds for P2P_GO/STA.
 *
 * Currently used to set time quota for 2 MCC vdevs/adapters using
 * (operating channel, quota) for each mode.
 * The info is provided run time using iwpriv command:
 * iwpriv <wlan0 | p2p0> setMccQuota <quota in ms>.
 * Note: the quota provided in command is for the same mode in cmd.
 * HDD checks if MCC mode is active, gets the second mode and its
 * operating chan.
 * Quota for the 2nd role is calculated as 100 - quota of first mode.
 *
 * @E.g: iwpriv wlan0 setMccQuota 50
 *       iwpriv p2p0 setMccQuota 50
 *
 * Supported Feature: Concurrency
 *
 * Usage: Internal/External
 *
 * </ioctl>
 */
#define WE_MCC_CONFIG_QUOTA             71
/* Private IOCTL for debug connection issues */
#define WE_SET_DEBUG_LOG                72
#ifdef WE_SET_TX_POWER
#undef WE_SET_TX_POWER
#endif

/*
 * <ioctl>
 * setTxPower - Set the current transmit power
 *
 * @INPUT: Transmission power in dBm
 *
 * @OUTPUT: None
 *
 * This IOCTL sets the current transmit power.
 * This setting does not persist over reboots.
 *
 * @E.g: iwpriv wlan0 setTxPower 10
 *
 * Supported Feature: STA
 *
 * Usage: Internal/External
 *
 * </ioctl>
 */
#define WE_SET_TX_POWER                 74
/* Private ioctl for earlyrx power save feature */
#define WE_SET_EARLY_RX_ADJUST_ENABLE         75
#define WE_SET_EARLY_RX_TGT_BMISS_NUM         76
#define WE_SET_EARLY_RX_BMISS_SAMPLE_CYCLE    77
#define WE_SET_EARLY_RX_SLOP_STEP             78
#define WE_SET_EARLY_RX_INIT_SLOP             79
#define WE_SET_EARLY_RX_ADJUST_PAUSE          80
/*
 * <ioctl>
 * setMcRate  - Set the data rate for multicast data
 *
 * @INPUT: 1 to 32
 *
 * @OUTPUT: None
 *
 * This IOCTL sets the data rate for multicast data. Note that this command
 * is allowed only in STA, IBSS, or QCMobileAP mode
 *
 * @E.g: iwpriv wlan0 setMcRate <value>
 *
 * Supported Feature: STA
 *
 * Usage: Internal/External
 *
 * </ioctl>
 */
#define WE_SET_MC_RATE                        81
#define WE_SET_EARLY_RX_DRIFT_SAMPLE          82
/* Private ioctl for packet power save */
/*
 * <ioctl>
 * 5g_ebt - Sets the 5g_ebt
 *
 * @INPUT: <value>
 *
 * @OUTPUT: None
 *
 * This IOTCL used to set 5g_ebt
 *
 * @E.g: iwpriv wlan0 5g_ebt <value>
 *
 * Supported Feature: STA
 *
 * Usage: Internal/External
 *
 * </ioctl>
 */
#define WE_PPS_5G_EBT                         83
/*
 * <ioctl>
 * cts_cbw  - Set CTS channel BW for dynamic BW adjustment
 *
 * @INPUT: 20 t0 160
 *
 * @OUTPUT: None
 *
 * This IOTCL used to set CTS channel BW for dynamic BW adjustment
 *
 * @E.g: iwpriv wlan0 cts_cbw <value>
 *
 * Supported Feature: STA
 *
 * Usage: Internal/External
 *
 * </ioctl>
 */
#define WE_SET_CTS_CBW                        84
#define WE_DUMP_STATS                         85
#define WE_CLEAR_STATS                        86
/* Private sub ioctl for starting/stopping the profiling */
#define WE_START_FW_PROFILE                      87

/*
 * <ioctl>
 * setChanChange - Initiate channel change
 *
 * @INPUT: channel number to switch to.
 *
 * @OUTPUT: None
 *
 * This IOCTL is used to initiate a channel change.
 * If called on STA/CLI interface it will send the
 * ECSA action frame to the connected SAP/GO asking to
 * initiate the ECSA, if supported.
 * If called on SAP/GO interface it will initiate
 * ECSA and ask connected peers to move to new channel.
 *
 * @E.g: iwpriv wlan0 setChanChange <channel>
 * iwpriv wlan0 setChanChange 1
 *
 * Supported Feature: ECSA
 *
 * Usage: Internal/External
 *
 * </ioctl>
 */
#define WE_SET_CHANNEL                        88
#define WE_SET_CONC_SYSTEM_PREF               89

/*
 * <ioctl>
 * set_11ax_rate - set 11ax rates to FW
 *
 * @INPUT: rate code
 *
 * @OUTPUT: None
 *
 * This IOCTL fixes the Tx data rate of 11AX.
 *
 * @E.g: iwpriv wlan0 set_11ax_rate <rate code>
 *
 * Supported Feature: STA/SAP
 *
 * Usage: Internal
 *
 * </ioctl>
 */
#define WE_SET_11AX_RATE                      91

/*
 * <ioctl>
 * enable_dcm - enable Dual Carrier Modulation(DCM)
 *
 * @INPUT: 0/1
 *
 * @OUTPUT: None
 *
 * This IOCTL enables/disables DCM.
 *
 * @E.g: iwpriv wlan0 enable_dcm <0/1>
 *
 * Supported Feature: STA/SAP
 *
 * Usage: Internal
 *
 * </ioctl>
 */
#define WE_SET_DCM                            92

/*
 * <ioctl>
 * range_ext - enable Range extension
 *
 * @INPUT: 0/1
 *
 * @OUTPUT: None
 *
 * This IOCTL enables/disables Range extension.
 *
 * @E.g: iwpriv wlan0 range_ext <1/0>
 *
 * Supported Feature: STA/SAP
 *
 * Usage: Internal
 *
 * </ioctl>
 */
#define WE_SET_RANGE_EXT                      93

/*
 * <ioctl>
 * wow_ito - sets the timeout value for inactivity data while
 * in power save mode during wow
 *
 * @INPUT: int
 *
 * @OUTPUT: None
 *
 * This IOCTL set the timeout value for inactivity data in power save mode
 *
 * @E.g: iwpriv wlan0 wow_ito 20
 *
 * Supported Feature: STA
 *
 * Usage: External
 *
 * </ioctl>
 */
#define WE_SET_WOW_DATA_INACTIVITY_TO    94

/*
 * <ioctl>
 * pdev_reset - reset the pdev
 *
 * @INPUT: Reset command to initiate:
 *    TX_FLUSH = 1
 *    WARM_RESET = 2
 *    COLD_RESET = 3
 *    WARM_RESET_RESTORE_CAL = 4
 *    COLD_RESET_RESTORE_CAL = 5
 *
 * @OUTPUT: None
 *
 * This IOCTL is used to reset the pdev. The primary use is
 * for internal testing. It is not expected that this will
 * be used on a production device.
 *
 * @E.g: iwpriv wlan0 pdev_reset <command>
 * iwpriv wlan0 pdev_reset 1
 *
 * Supported Feature: None
 *
 * Usage: Internal
 *
 * </ioctl>
 */
#define WE_SET_PDEV_RESET    95

/*
 * setModDTIM - Change Modulated DTIM
 *
 * @INPUT: set_value.
 *
 * @OUTPUT: None
 *
 * This IOCTL is used to change modulated DTIM
 * value without WIFI OFF/ON.
 *
 * @E.g: iwpriv wlan0 setModDTIM <value>
 * iwpriv wlan0 setModDTIM 2
 *
 * Supported Feature: N/A
 *
 * Usage: Internal/External
 *
 * </ioctl>
 */
#define WE_SET_MODULATED_DTIM                 96

/* Private ioctls and their sub-ioctls */
#define WLAN_PRIV_SET_NONE_GET_INT    (SIOCIWFIRSTPRIV + 1)
#define WE_GET_11D_STATE     1
#define WE_GET_WLAN_DBG      4
#define WE_GET_MAX_ASSOC     6
/* 7 is unused */
#define WE_GET_SAP_AUTO_CHANNEL_SELECTION 8

/*
 * <ioctl>
 * getconcurrency - Get concurrency mode
 *
 * @INPUT: None
 *
 * @OUTPUT: It shows concurrency value
 * Bit 0:STA   1:SAP     2:P2P_Client  3:P2P_GO
 *     4:FTM   5:IBSS    6:Monitor     7:P2P_Device
 *     8:OCB   9:EPPING  10:QVIT       11:NDI
 *
 * This IOCTL is used to retrieve concurrency mode.
 *
 * @E.g: iwpriv wlan0 getconcurrency
 * wlan0     getconcurrency:5
 * Above value shows STA+P2P_Client
 *
 * Supported Feature: Concurrency
 *
 * Usage: Internal/External
 *
 * </ioctl>
 */
#define WE_GET_CONCURRENCY_MODE 9
/*
 * <ioctl>
 * get_nss - Get the number of spatial STBC streams (NSS)
 *
 * @INPUT: None
 *
 * @OUTPUT: NSS
 *  wlan0     get_nss:2
 *
 * This IOTCL used to get the number of spatial STBC streams
 *
 * @E.g: iwpriv wlan0 get_nss
 *
 * Supported Feature: STA
 *
 * Usage: Internal/External
 *
 * </ioctl>
 */
#define WE_GET_NSS           11
/*
 * <ioctl>
 * get_ldpc - This IOCTL gets the low density parity check (LDPC)
 *
 * @INPUT: None
 *
 * @OUTPUT: ldpc
 *  wlan0     get_ldpc:1
 *
 * This IOTCL used to gets the low density parity check (LDPC)
 *
 * @E.g: iwpriv wlan0 get_ldpc
 *
 * Supported Feature: STA
 *
 * Usage: Internal/External
 *
 * </ioctl>
 */
#define WE_GET_LDPC          12
/*
 * <ioctl>
 * get_tx_stbc - Get the value of the current Tx space time block code (STBC)
 *
 * @INPUT: None
 *
 * @OUTPUT: TXSTBC
 *  wlan0     get_tx_stbc:1
 *
 * This IOTCL get the value of the current Tx space time block code (STBC)
 *
 * @E.g: iwpriv wlan0 get_tx_stbc
 *
 * Supported Feature: STA
 *
 * Usage: Internal/External
 *
 * </ioctl>
 */
#define WE_GET_TX_STBC       13
/*
 * <ioctl>
 * get_rx_stbc - Gets the value of the current Rx STBC
 *
 * @INPUT: None
 *
 * @OUTPUT: Rx STBC
 *  wlan0     get_rx_stbc:1
 *
 * This IOTCL used to get the value of the current Rx STBC
 *
 * @E.g: iwpriv wlan0 get_rx_stbc
 *
 * Supported Feature: STA
 *
 * Usage: Internal/External
 *
 * </ioctl>
 */
#define WE_GET_RX_STBC       14
/*
 * <ioctl>
 * get_shortgi - Get the value of the current short GI setting
 *
 * @INPUT: None
 *
 * @OUTPUT: Enable/disable of shortgi
 *  wlan0     get_shortgi:1
 *
 * This IOCTL gets the value of the current short GI setting
 *
 * @E.g: iwpriv wlan0 get_shortgi
 *
 * Supported Feature: STA
 *
 * Usage: Internal/External
 *
 * </ioctl>
 */
#define WE_GET_SHORT_GI      15
/*
 * <ioctl>
 * get_rtscts - Get the value of the current RTS/CTS setting.
 *
 * @INPUT: None
 *
 * @OUTPUT: Enable/disable of RTS/CTS
 *  wlan0     get_rtscts:33
 *
 * This IOTCL get the value of the current RTS/CTS setting.
 *
 * @E.g: iwpriv wlan0 get_rtscts
 *
 * Supported Feature: STA
 *
 * Usage: Internal/External
 *
 * </ioctl>
 */
#define WE_GET_RTSCTS        16
/*
 * <ioctl>
 * get_chwidth - Get the current channel width setting
 *
 * @INPUT: None
 *
 * @OUTPUT: channel width
 *  wlan0     get_chwidth:0
 *
 * This IOTCL get the current channel width setting.
 *
 * @E.g: iwpriv wlan0 get_chwidth
 *
 * Supported Feature: STA
 *
 * Usage: Internal/External
 *
 * </ioctl>
 */
#define WE_GET_CHWIDTH       17
/*
 * <ioctl>
 * get_anienable - Get the anienable
 *
 * @INPUT: None
 *
 * @OUTPUT:
 *  wlan0     get_anienable:0
 *
 * This IOTCL get the anienable
 *
 * @E.g: iwpriv wlan0 get_anienable
 *
 * Supported Feature: STA
 *
 * Usage: Internal/External
 *
 * </ioctl>
 */
#define WE_GET_ANI_EN_DIS    18
/*
 * <ioctl>
 * get_aniplen  - Get the aniplen
 *
 * @INPUT: None
 *
 * @OUTPUT:
 *  wlan0     get_aniplen:0
 *
 * This IOTCL get the aniplen
 *
 * @E.g: iwpriv wlan0 get_aniplen
 *
 * Supported Feature: STA
 *
 * Usage: Internal/External
 *
 * </ioctl>
 */
#define WE_GET_ANI_POLL_PERIOD    19
/*
 * <ioctl>
 * get_anilislen- Get the anilislen
 *
 * @INPUT: None
 *
 * @OUTPUT:
 *  wlan0     get_anilislen:0
 *
 * This IOTCL used to get anilislen
 *
 * @E.g: iwpriv wlan0 get_anilislen
 *
 * Supported Feature: STA
 *
 * Usage: Internal/External
 *
 * </ioctl>
 */
#define WE_GET_ANI_LISTEN_PERIOD  20
/*
 * <ioctl>
 * get_aniofdmlvl - Get the OFDM level
 *
 * @INPUT: None
 *
 * @OUTPUT: OFDM
 *  wlan0     get_aniofdmlvl:0
 *
 * This IOTCL used to get ofdm level
 *
 * @E.g: iwpriv wlan0 get_aniofdmlvl
 *
 * Supported Feature: STA
 *
 * Usage: Internal/External
 *
 * </ioctl>
 */
#define WE_GET_ANI_OFDM_LEVEL     21
/*
 * <ioctl>
 * get_aniccklvl - Get the cck level
 *
 * @INPUT: None
 *
 * @OUTPUT:
 *  wlan0     get_aniccklvl:0
 *
 * This IOTCL used to get cck level
 *
 * @E.g: iwpriv wlan0 get_aniccklvl
 *
 * Supported Feature: STA
 *
 * Usage: Internal/External
 *
 * </ioctl>
 */
#define WE_GET_ANI_CCK_LEVEL      22
/*
 * <ioctl>
 * get_cwmenable - Get the value of the dynamic channel bandwidth setting
 *
 * @INPUT: None
 *
 * @OUTPUT: Enable/disable dynamic channel bandwidth
 *  wlan0     get_cwmenable:0
 *
 * This IOTCL get the value of the dynamic channel bandwidth setting
 *
 * @E.g: iwpriv wlan0 get_cwmenable
 *
 * Supported Feature: STA
 *
 * Usage: Internal/External
 *
 * </ioctl>
 */
#define WE_GET_DYNAMIC_BW         23
/*
 * <ioctl>
 * get_txchainmask - Get the txchainmask that was set
 *
 * @INPUT: None
 *
 * @OUTPUT: txchainmask
 *  wlan0     get_txchainmask:1
 *
 * This IOCTL gets the txchainmask that was set
 * This command is useful if it was previously set
 *
 * @E.g: iwpriv wlan0 get_txchainmask
 *
 * Supported Feature: STA
 *
 * Usage: Internal/External
 *
 * </ioctl>
 */
#define WE_GET_TX_CHAINMASK  24
/*
 * <ioctl>
 * get_rxchainmask - Get the rxchainmask that was set
 *
 * @INPUT: None
 *
 * @OUTPUT: rxchainmask
 *  wlan0     get_rxchainmask:1
 *
 * This IOCTL gets the rxchainmask that was set
 * This command is useful only if it was previously set.
 *
 * @E.g: iwpriv wlan0 get_rxchainmask
 *
 * Supported Feature: STA
 *
 * Usage: Internal/External
 *
 * </ioctl>
 */
#define WE_GET_RX_CHAINMASK  25
/*
 * <ioctl>
 * get_11nrate - Get the fixed Tx data rate
 *
 * @INPUT: None
 *
 * @OUTPUT: Using this command does not return the same value as set
 *  wlan0     get_11nrate:0
 *
 * This IOCTL gets the fixed Tx data rate
 * This command is useful only if setting the fixed Tx rate.
 *
 * @E.g: iwpriv wlan0 get_11nrate
 *
 * Supported Feature: STA
 *
 * Usage: Internal/External
 *
 * </ioctl>
 */
#define WE_GET_11N_RATE      26
/*
 * <ioctl>
 * get_ampdu - Get the maximum subframe of ampdu
 *
 * @INPUT: None
 *
 * @OUTPUT: Maximum subframe of ampdu
 *  wlan0     get_ampdu:1
 *
 * This IOCTL gets the maximum subframe of ampdu
 * This command is useful only if setting ampdu.
 *
 * @E.g: iwpriv wlan0 get_ampdu
 *
 * Supported Feature: STA
 *
 * Usage: Internal/External
 *
 * </ioctl>
 */
#define WE_GET_AMPDU         27
/*
 * <ioctl>
 * get_amsdu - Get the maximum subframe of amsdu
 *
 * @INPUT: None
 *
 * @OUTPUT: Maximum subframe of amsdu
 *  wlan0     get_amsdu:1
 *
 * This IOCTL gets the maximum subframe of amsdu.
 * This command is useful only if setting amsdu
 *
 * @E.g: iwpriv wlan0 get_amsdu
 *
 * Supported Feature: STA
 *
 * Usage: Internal/External
 *
 * </ioctl>
 */
#define WE_GET_AMSDU         28
/*
 * <ioctl>
 * get_txpow2g - Get the current 2 GHz Tx power setting
 *
 * @INPUT: None
 *
 * @OUTPUT: Tx Power in dbm
 * wlan0     get_txpow2g:0
 *
 * This IOCTL gets the current 2 GHz Tx power setting
 * This command is useful if setting Tx power
 *
 * @E.g: iwpriv wlan0 get_txpow2g
 *
 * Supported Feature: STA
 *
 * Usage: Internal/External
 *
 * </ioctl>
 */
#define WE_GET_TXPOW_2G      29
/*
 * <ioctl>
 * get_txpow5g - Get the current 5 GHz Tx power setting
 *
 * @INPUT: None
 *
 * @OUTPUT: Tx Power in dbm
 * wlan0     get_txpow5g:0
 *
 * This IOCTL gets the current 5 GHz Tx power setting
 * This command is useful if setting Tx power
 *
 * @E.g: iwpriv wlan0 get_txpow5g
 *
 * Supported Feature: STA
 *
 * Usage: Internal/External
 *
 * </ioctl>
 */
#define WE_GET_TXPOW_5G      30
/* 31 is unused */
#define WE_GET_PPS_PAID_MATCH           32
#define WE_GET_PPS_GID_MATCH            33
#define WE_GET_PPS_EARLY_TIM_CLEAR      34
#define WE_GET_PPS_EARLY_DTIM_CLEAR     35
#define WE_GET_PPS_EOF_PAD_DELIM        36
#define WE_GET_PPS_MACADDR_MISMATCH     37
#define WE_GET_PPS_DELIM_CRC_FAIL       38
#define WE_GET_PPS_GID_NSTS_ZERO        39
#define WE_GET_PPS_RSSI_CHECK           40
/* Private ioctl for QPower */
#define WE_GET_QPOWER_MAX_PSPOLL_COUNT            41
#define WE_GET_QPOWER_MAX_TX_BEFORE_WAKE          42
#define WE_GET_QPOWER_SPEC_PSPOLL_WAKE_INTERVAL   43
#define WE_GET_QPOWER_SPEC_MAX_SPEC_NODATA_PSPOLL 44
/* GTX Commands */
/*
 * <ioctl>
 * get_gtxHTMcs - Get the tx HTM
 *
 * @INPUT: None
 *
 * @OUTPUT: HTM
 *  wlan0     get_gtxHTMcs:32896
 *
 * This IOTCL used to get HTM
 *
 * @E.g: iwpriv wlan0 get_gtxHTMcs
 *
 * Supported Feature: STA
 *
 * Usage: Internal/External
 *
 * </ioctl>
 */
#define WE_GET_GTX_HT_MCS               47
/*
 * <ioctl>
 * get_gtxVHTMcs - Get the VHTM
 *
 * @INPUT: None
 *
 * @OUTPUT: VHTM
 *  wlan0     get_gtxVHTMcs:524800
 *
 * This IOTCL used to get the VHTM
 *
 * @E.g: iwpriv wlan0 get_gtxVHTMcs
 *
 * Supported Feature: STA
 *
 * Usage: Internal/External
 *
 * </ioctl>
 */
#define WE_GET_GTX_VHT_MCS              48
/*
 * <ioctl>
 * get_gtxUsrCfg - Get the tx cfg
 *
 * @INPUT: None
 *
 * @OUTPUT: TXCFG
 *  wlan0     get_gtxUsrCfg:32
 *
 * This IOTCL used to get the tx cfg
 *
 * @E.g: iwpriv wlan0 get_gtxUsrCfg
 *
 * Supported Feature: STA
 *
 * Usage: Internal/External
 *
 * </ioctl>
 */
#define WE_GET_GTX_USRCFG               49
/*
 * <ioctl>
 * get_gtxThre - Get the tx threshold
 *
 * @INPUT: None
 *
 * @OUTPUT: Threshold
 *  wlan0     get_gtxThre:3
 *
 * This IOCTL is used to get tx threshold
 *
 * @E.g: iwpriv wlan0 get_gtxThre
 *
 * Supported Feature: STA
 *
 * Usage: Internal/External
 *
 * </ioctl>
 */
#define WE_GET_GTX_THRE                 50
/*
 * <ioctl>
 * get_gtxMargin - Get the tx margin
 *
 * @INPUT: None
 *
 * @OUTPUT: GTXMARGIN
 *  wlan0     get_gtxMargin:2
 *
 * This IOCTL is used to set tx margin
 *
 * @E.g: iwpriv wlan0 get_gtxMargin
 *
 * Supported Feature: STA
 *
 * Usage: Internal/External
 *
 * </ioctl>
 */
#define WE_GET_GTX_MARGIN               51
/*
 * <ioctl>
 * get_gtxStep - Get the tx step
 *
 * @INPUT: None
 *
 * @OUTPUT: GTXSTEP
 * wlan0     get_gtxStep:0
 *
 * This IOCTL is used to get the gtx step
 *
 * @E.g: iwpriv wlan0 get_gtxStep
 *
 * Supported Feature: STA
 *
 * Usage: Internal/External
 *
 * </ioctl>
 */
#define WE_GET_GTX_STEP                 52
/*
 * <ioctl>
 * get_gtxMinTpc - Get the tx miminum tpc
 *
 * @INPUT: None
 *
 * @OUTPUT: TPC
 * wlan0     get_gtxMinTpc:0
 *
 * This IOCTL is used to get tx miminum tpc
 *
 * @E.g: iwpriv wlan0 get_gtxMinTpc
 *
 * Supported Feature: STA
 *
 * Usage: Internal/External
 *
 * </ioctl>
 */
#define WE_GET_GTX_MINTPC               53
/*
 * <ioctl>
 * get_gtxBWMask - Get the tx BW MASK
 *
 * @INPUT: None
 *
 * @OUTPUT: MASK
 * wlan0     get_gtxBWMask:15
 *
 * This IOCTL is used get gtx bw mask
 *
 * @E.g: iwpriv wlan0 get_gtxBWMask
 *
 * Supported Feature: STA
 *
 * Usage: Internal/External
 *
 * </ioctl>
 */
#define WE_GET_GTX_BWMASK               54
#define WE_GET_TEMPERATURE              56
#define WE_CAP_TSF                      58
#define WE_GET_ROAM_SYNCH_DELAY         59

/*
 * <ioctl>
 * get_dcm - Get dcm enablement value
 *
 * @INPUT: None
 *
 * @OUTPUT: 0/1
 * wlan0     get_dcm
 *
 * This IOCTL is used get dcm value
 *
 * Supported Feature: STA/SAP
 *
 * Usage: Internal
 *
 * </ioctl>
 */
#define WE_GET_DCM                      60

/*
 * <ioctl>
 * get_dcm - Get range extension enablement value
 *
 * @INPUT: None
 *
 * @OUTPUT: 0/1
 * wlan0     get_range_ext
 *
 * This IOCTL is used get range_extension value
 *
 * Supported Feature: STA/SAP
 *
 * Usage: Internal
 *
 * </ioctl>
 */
#define WE_GET_RANGE_EXT                61

/* Private ioctls and their sub-ioctls */
#define WLAN_PRIV_SET_INT_GET_INT     (SIOCIWFIRSTPRIV + 2)

/* Private ioctls and their sub-ioctls */
#define WLAN_PRIV_SET_CHAR_GET_NONE   (SIOCIWFIRSTPRIV + 3)
#define WE_WOWL_ADD_PTRN     1
#define WE_WOWL_DEL_PTRN     2
/*
 * <ioctl>
 * neighbor - Send neighbor report request
 *
 * @INPUT: string
 *
 * @OUTPUT: None
 *
 * This IOCTL create a Neighbor report request and send it to peer
 *
 * @E.g: iwpriv wlan0 neighbor "SSID"
 *
 * Supported Feature: 11k
 *
 * Usage: Internal/External
 *
 * </ioctl>
 */
#define WE_NEIGHBOR_REPORT_REQUEST 3
/*
 * <ioctl>
 * set_ap_wps_ie - Set the P2P IE of the probe response
 *
 * @INPUT: string
 *
 * @OUTPUT: None
 *
 * This IOCTL sets the P2P IE of the probe response
 *
 * @E.g: iwpriv wlan0 set_ap_wps_ie abcd
 *
 * Supported Feature: STA
 *
 * Usage: Internal/External
 *
 * </ioctl>
 */
#define WE_SET_AP_WPS_IE     4
#define WE_SET_CONFIG        5

/*
 * <ioctl>
 * unit_test - execute component-level unit tests
 *
 * @INPUT: string - the name of the component to test.
 *	All tests are executed if unspecified
 * @OUTPUT: None
 *
 * Usage: Internal only
 * </ioctl>
 */
#define WE_UNIT_TEST         6

/* Private ioctls and their sub-ioctls */
#define WLAN_PRIV_SET_THREE_INT_GET_NONE   (SIOCIWFIRSTPRIV + 4)
#define WE_SET_WLAN_DBG      1
#define WE_SET_DP_TRACE      2
#define WE_SET_FW_TEST       4

/* Private ioctls and their sub-ioctls */
#define WLAN_PRIV_GET_CHAR_SET_NONE   (SIOCIWFIRSTPRIV + 5)
#define WE_WLAN_VERSION      1
#define WE_GET_STATS         2
/*
 * <ioctl>
 * getConfig - gets the values of all configurations listed in WCNSS
 *
 * @INPUT: None
 *
 * @OUTPUT: Current configuration to the sys log
 *  wlan0	getConfig: WLAN configuration written to system log
 *
 * This IOCTL gets the values of all configurations listed in WCNSS
 *
 * @E.g: iwpriv wlan0 getConfig
 *
 * Supported Feature: STA
 *
 * Usage: Internal/External
 *
 * </ioctl>
 */
#define WE_GET_CFG           3
#define WE_GET_WMM_STATUS    4
/*
 * <ioctl>
 * getChannelList - Get the available channel list while in QCMobileAP
 *
 * @INPUT: None
 *
 * @OUTPUT: Channel list
 * wlan0     getChannelList:36 US 1..165
 *
 * This IOCTL gets the available channel list while in QCMobileAP
 *
 * @E.g: iwpriv wlan0 getChannelList
 *
 * Supported Feature: STA
 *
 * Usage: Internal/External
 *
 * </ioctl>
 */
#define WE_GET_CHANNEL_LIST  5
/*
 * <ioctl>
 * getRSSI - Get the Received Signal Strength Indicator
 *
 * @INPUT: None
 *
 * @OUTPUT: RSSI
 * wlan0     getRSSI:rsssi=-32
 *
 * This IOCTL gets the Received Signal Strength Indicator (RSSI)
 *
 * @E.g: iwpriv wlan0 getRSSI
 *
 * Supported Feature: STA
 *
 * Usage: Internal/External
 *
 * </ioctl>
 */
#define WE_GET_RSSI          6

/*
 * <ioctl>
 * getSuspendStats - Get suspend/resume stats
 *
 * @INPUT: None
 *
 * @OUTPUT: character string containing formatted suspend/resume stats
 *
 * This ioctl is used to get suspend/resume stats formatted for display.
 * Currently it includes suspend/resume counts, wow wake up reasons, and
 * suspend fail reasons.
 *
 * @E.g: iwpriv wlan0 getSuspendStats
 * iwpriv wlan0 getSuspendStats
 *
 * Supported Feature: suspend/resume
 *
 * Usage: Internal
 *
 * </ioctl>
 */
#define WE_GET_SUSPEND_RESUME_STATS 7
#ifdef FEATURE_WLAN_TDLS
/*
 * <ioctl>
 * getTdlsPeers - Get all TDLS peers.
 *
 * @INPUT: None
 *
 * @OUTPUT: Returns the MAC address of all the TDLS peers
 * wlan0     getTdlsPeers:
 * MAC               Id cap up RSSI
 * ---------------------------------
 * 00:0a:f5:0e:bd:18  2   Y  Y  -44
 * 00:0a:f5:bf:0e:12  0   N  N    0
 *
 * This IOCTL is used to get all TDLS peers.
 *
 * @E.g: iwpriv wlan0 getTdlsPeers
 *
 * Supported Feature: TDLS
 *
 * Usage: Internal/External
 *
 * </ioctl>
 */
#define WE_GET_TDLS_PEERS    8
#endif
#ifdef WLAN_FEATURE_11W
/*
 * <ioctl>
 * getPMFInfo - get the PMF info of the connected session
 *
 * @INPUT: None
 *
 * @OUTPUT:
 *  wlan0     getPMFInfo:
 *  BSSID E4:F4:C6:0A:E0:36, Is PMF Assoc? 0
 *  Number of Unprotected Disassocs 0
 *  Number of Unprotected Deauths 0
 *
 * This IOCTL is used to get the PMF stats/status of the current
 * connection.
 *
 * @e.g:iwpriv wlan0 getPMFInfo
 *
 * Supported Feature: PMF
 *
 * Usage: Internal/External
 *
 * </ioctl>
 */
#define WE_GET_11W_INFO      9
#endif
#define WE_GET_STATES        10
/*
 * <ioctl>
 * getIbssSTAs - get ibss sta info
 *
 * @INPUT: None
 *
 * @OUTPUT: Give the MAC of the IBSS STA
 *  wlan0     getIbssSTAs:
 *  1 .8c:fd:f0:01:9c:bf
 *
 * This IOCTL is used to get ibss sta info
 *
 * @E.g: iwpriv wlan0 getIbssSTAs
 *
 * Supported Feature: IBSS
 *
 * Usage: Internal/External
 *
 * </ioctl>
 */
#define WE_GET_IBSS_STA_INFO 11
/*
 * <ioctl>
 * getphymode - Get the current phymode.
 *
 * @INPUT: None
 *
 * @OUTPUT: In phymode
 *  wlan0     getphymode:AUTO MODE
 *
 * This IOCTL used to gets the current phymode.
 *
 * @E.g: iwpriv wlan0 getphymode
 *
 * Supported Feature: STA
 *
 * Usage: Internal/External
 *
 * </ioctl>
 */
#define WE_GET_PHYMODE       12

/*
 * <ioctl>
 * getOemDataCap - Get the oem data caps.
 *
 * @INPUT: None
 *
 * @OUTPUT: oem data capability
 *
 * This IOCTL used to gets the current oem data cap.
 *
 * @E.g: iwpriv wlan0 getOemDataCap
 *
 * Usage: Internal/External
 *
 * </ioctl>
 */
#define WE_GET_OEM_DATA_CAP  13

/*
 * <ioctl>
 * getSNR - Enable SNR Monitoring
 *
 * @INPUT: None
 *
 * @OUTPUT: Signal strength/ratio
 *  wlan0     getSNR:1
 *
 * This IOCTL is used to get ibss sta info
 *
 * @E.g: iwpriv wlan0 getSNR
 *
 * Supported Feature: STA
 *
 * Usage: Internal/External
 *
 * </ioctl>
 */

#define WE_GET_SNR           14
#define WE_LIST_FW_PROFILE      15

/*
 * <ioctl>
 *
 * get_ba_timeout - to get timeout for each AC
 *
 * @INPUT: None
 *
 * @OUTPUT: displays timeout value for each access class
 *
 * @E.g.: iwpriv wlan0 get_ba_timeout
 *
 * Usage: Internal
 *
 * </ioctl>
 */
#define WE_GET_BA_AGEING_TIMEOUT 16

/* Private ioctls and their sub-ioctls */
#define WLAN_PRIV_SET_NONE_GET_NONE   (SIOCIWFIRSTPRIV + 6)

/*
 * <ioctl>
 * reassoc - Trigger STA re-association to the connected AP
 *
 * @INPUT: None
 *
 * @OUTPUT: None
 *
 * This IOCTL is used to trigger STA reassociation to the connected AP.
 *
 * @E.g: iwpriv wlan0 reassoc
 *
 * Supported Feature: Roaming
 *
 * Usage: Internal
 *
 * </ioctl>
 */
#define WE_SET_REASSOC_TRIGGER     8
/*
 * <ioctl>
 * ibssPeerInfoAll - Print the ibss peers's MAC, rate and RSSI
 *
 * @INPUT: None
 *
 * @OUTPUT: print ibss peer in info logs
 *  pPeerInfo->numIBSSPeers = 1
 *  PEER ADDR : 8c:fd:f0:01:9c:bf TxRate: 1 Mbps RSSI: -35
 *
 * This IOCTL is used to rint the ibss peers's MAC, rate and RSSI
 * in info logs
 *
 * @E.g: iwpriv wlan0 ibssPeerInfoAll
 *
 * Supported Feature: IBSS
 *
 * Usage: Internal/External
 *
 * </ioctl>
 */
#define WE_IBSS_GET_PEER_INFO_ALL 10
/* Sub ioctls 11 to 16 are not used */
#define WE_GET_RECOVERY_STAT       17
#define WE_GET_FW_PROFILE_DATA     18
/*
 * <ioctl>
 * stop_obss_scan - Stop obss scan
 *
 * @INPUT: None
 *
 * @OUTPUT: None
 *
 * This IOCTL is used to stop obss scan
 *
 * @E.g: iwpriv wlan0 stop_obss_scan
 *
 * Supported Feature: Scan
 *
 * Usage: Internal/External
 *
 * </ioctl>
 */
#define WE_STOP_OBSS_SCAN          19

/* Private ioctls and their sub-ioctls */
#define WLAN_PRIV_SET_VAR_INT_GET_NONE   (SIOCIWFIRSTPRIV + 7)

#define WE_P2P_NOA_CMD       2
/* subcommands 3 is unused */

#define WE_MAC_PWR_DEBUG_CMD 4

/* subcommand 5 is unused */

/*
 * <ioctl>
 * ibssPeerInfo - Print the ibss peers's MAC, rate and RSSI
 *
 * @INPUT: staid
 *
 * @OUTPUT: print ibss peer corresponding to staid in info logs
 *  PEER ADDR : 8c:fd:f0:01:9c:bf TxRate: 1 Mbps RSSI: -35
 *
 * This IOCTL is used to print the specific ibss peers's MAC,
 * rate and RSSI in info logs
 *
 * @E.g: iwpriv wlan0 ibssPeerInfo <sta_id>
 *  iwpriv wlan0 ibssPeerInfo 0
 *
 * Supported Feature: IBSS
 *
 * Usage: Internal/External
 *
 * </ioctl>
 */
#define WE_IBSS_GET_PEER_INFO   6
#define WE_UNIT_TEST_CMD   7

#define WE_MTRACE_DUMP_CMD    8
#define WE_MTRACE_SELECTIVE_MODULE_LOG_ENABLE_CMD    9


#ifdef WLAN_FEATURE_GPIO_LED_FLASHING
#define WE_LED_FLASHING_PARAM    10
#endif

/*
 * <ioctl>
 * pm_clist - Increments the index value of the concurrent connection list
 * and update with the input parameters provided.
 *
 * @INPUT: Following 8 arguments:
 * @vdev_id: vdev id
 * @tx_streams: TX streams
 * @rx_streams: RX streams
 * @chain_mask: Chain mask
 * @type: vdev_type
 *    AP:1    STA:2    IBSS:3    Monitor:4    NAN:5    OCB:6    NDI:7
 * @sub_type: vdev_subtype
 *    P2P_Device:1    P2P_Client:2     P2P_GO:3
 *    Proxy_STA:4     Mesh:5           Mesh_11s:6
 * @channel: Channel
 * @mac: Mac id
 *
 * @OUTPUT: None
 *
 * This IOCTL is used to increments the index value of the concurrent connection
 * list and update with the input parameters provided.
 *
 * @E.g: iwpriv wlan0 pm_clist vdev_id tx_streams rx_streams chain_mask type
 *                    sub_type channel mac
 * iwpriv wlan0 pm_clist 1 2 2 1 2 3 10 1
 *
 * Supported Feature: DBS
 *
 * Usage: Internal/External
 *
 * </ioctl>
 */
#define WE_POLICY_MANAGER_CLIST_CMD    11

/*
 * <ioctl>
 * pm_dlist - Delete the index from the concurrent connection list that is
 * present in the given vdev_id.
 *
 * @INPUT: delete_all, vdev_id
 * @delete_all: delete all indices
 * @vdev_id: vdev id
 *
 * @OUTPUT: None
 *
 * This IOCTL is used to delete the index from the concurrent connection list
 * that is present in the given vdev_id.
 *
 * @E.g: iwpriv wlan0 pm_dlist delete_all vdev_id
 * iwpriv wlan0 pm_dlist 0 1
 *
 * Supported Feature: DBS
 *
 * Usage: Internal/External
 *
 * </ioctl>
 */
#define WE_POLICY_MANAGER_DLIST_CMD    12

/*
 * <ioctl>
 * pm_dbs - Set dbs capability and system preference
 *
 * @INPUT: dbs, system_pref
 * @dbs: Value of DBS capability to be set
 * @system_pref: System preference
 *     0:PM_THROUGHPUT 1: PM_POWERSAVE 2: PM_LATENCY
 *
 * @OUTPUT: None
 *
 * This IOCTL is used to set dbs capability and system preference.
 *
 * @E.g: iwpriv wlan0 pm_dbs dbs system_pref
 * iwpriv wlan0 pm_dbs 1 0
 *
 * Supported Feature: DBS
 *
 * Usage: Internal/External
 *
 * </ioctl>
 */
#define WE_POLICY_MANAGER_DBS_CMD      13

/*
 * <ioctl>
 * pm_pcl - Set pcl for concurrency mode.
 *
 * @INPUT: policy_mgr_con_mode
 * @policy_mgr_con_mode: concurrency mode for PCL table
 *     0:STA  1:SAP  2:P2P_Client  3:P2P_GO  4:IBSS
 *
 * @OUTPUT: None
 *
 * This IOCTL is used to set pcl for concurrency mode.
 *
 * @E.g: iwpriv wlan0 pm_pcl policy_mgr_con_mode
 * iwpriv wlan0 pm_pcl 0
 *
 * Supported Feature: DBS
 *
 * Usage: Internal/External
 *
 * </ioctl>
 */
#define WE_POLICY_MANAGER_PCL_CMD      14

/*
 * <ioctl>
 * pm_cinfo - Shows the concurrent connection list.
 *
 * @INPUT: None
 *
 * @OUTPUT: None
 *
 * This IOCTL is used to show the concurrent connection list.
 *
 * @E.g: iwpriv wlan0 pm_cinfo
 *
 * Supported Feature: DBS
 *
 * Usage: Internal/External
 *
 * </ioctl>
 */
#define WE_POLICY_MANAGER_CINFO_CMD    15

/*
 * <ioctl>
 * pm_ulist - Updates the index value of the concurrent connection list
 * with the input parameters provided.
 *
 * @INPUT: Following 8 arguments:
 * @vdev_id: vdev id
 * @tx_streams: TX streams
 * @rx_streams: RX streams
 * @chain_mask: Chain mask
 * @type: vdev_type
 *    AP:1    STA:2    IBSS:3    Monitor:4    NAN:5    OCB:6    NDI:7
 * @sub_type: vdev_subtype
 *    P2P_Device:1    P2P_Client:2     P2P_GO:3
 *    Proxy_STA:4     Mesh:5           Mesh_11s:6
 * @channel: Channel
 * @mac: Mac id
 *
 * @OUTPUT: None
 *
 * This IOCTL is used to updates the index value of the concurrent
 * connection list with the input parameters provided.
 *
 * @E.g: iwpriv wlan0 pm_ulist vdev_id tx_streams rx_streams chain_mask type
 *                    sub_type channel mac
 * iwpriv wlan0 pm_ulist 1 2 2 1 2 3 10 1
 *
 * Supported Feature: DBS
 *
 * Usage: Internal/External
 *
 * </ioctl>
 */
#define WE_POLICY_MANAGER_ULIST_CMD    16

/*
 * <ioctl>
 * pm_query_action - Initiate actions needed on current connections as
 * per the channel provided.
 *
 * @INPUT: channel
 * @channel: Channel on which new connection will be.
 *
 * @OUTPUT: None
 *
 * This IOCTL is used to initiate actions needed on current connections
 * as per the channel provided.
 *
 * @E.g: iwpriv wlan0 pm_query_action channel
 * iwpriv wlan0 pm_query_action 6
 *
 * Supported Feature: DBS
 *
 * Usage: Internal/External
 *
 * </ioctl>
 */
#define WE_POLICY_MANAGER_QUERY_ACTION_CMD    17

/*
 * <ioctl>
 * pm_query_allow - Checks for allowed concurrency combination
 *
 * @INPUT: mode, channel, bandwidth
 * @mode:	new connection mode
 *     0:STA  1:SAP  2:P2P_Client  3:P2P_GO  4:IBSS
 * @channel: channel on which new connection is coming up
 * @bandwidth: Bandwidth requested by the connection
 *     0:None    1:5MHz    2:10MHz      3:20MHz
 *     4:40MHz   5:80MHz   6:80+80MHz   7:160MHz
 *
 * @OUTPUT: None
 *
 * This IOCTL is used to checks for allowed concurrency combination.
 *
 * @E.g: iwpriv wlan0 pm_query_allow mode channel bandwidth
 * iwpriv wlan0 pm_query_allow 0 6 4
 *
 * Supported Feature: DBS
 *
 * Usage: Internal/External
 *
 * </ioctl>
 */
#define WE_POLICY_MANAGER_QUERY_ALLOW_CMD    18

/*
 * <ioctl>
 * pm_run_scenario - Create scenario with number of connections provided.
 *
 * @INPUT: num_of_conn
 * @num_of_conn: the number of connections (values: 1~3)
 *
 * @OUTPUT: None
 *
 * This IOCTL is used to create scenario with the number of connections
 * provided.
 *
 * @E.g: iwpriv wlan0 pm_run_scenario num_of_conn
 * iwpriv wlan0 pm_run_scenario 1
 *
 * Supported Feature: DBS
 *
 * Usage: Internal/External
 *
 * </ioctl>
 */
#define WE_POLICY_MANAGER_SCENARIO_CMD 19

/*
 * <ioctl>
 * pm_set_hw_mode - Set hardware for single/dual mac.
 *
 * @INPUT: hw_mode
 *     0:single mac     1:dual mac
 *
 * @OUTPUT: None
 *
 * This IOCTL is used to set hardware for single/dual mac.
 *
 * @E.g: iwpriv wlan0 pm_set_hw_mode hw_mode
 * iwpriv wlan0 pm_set_hw_mode 1
 *
 * Supported Feature: DBS
 *
 * Usage: Internal/External
 *
 * </ioctl>
 */
#define WE_POLICY_SET_HW_MODE_CMD 20

/*
 * <ioctl>
 * ch_avoid - unit test SAP channel avoidance
 *
 * @INPUT: chan avoid ranges
 *
 * @OUTPUT: none
 *
 * This IOCTL is used to fake a channel avoidance event.
 * To test SAP/GO chan switch during chan avoid event process.
 *
 * @E.g: iwpriv wlan0 ch_avoid 2452 2462
 *
 * Supported Feature: SAP chan avoidance.
 *
 * Usage: Internal
 *
 * </ioctl>
 */
#define WE_SET_CHAN_AVOID 21

/*
 * <ioctl>
 * set_scan_cfg - Set dual MAC scan config parameters.
 *
 * @INPUT: dbs, dbs_plus_agile_scan, single_mac_scan_with_dbs
 * @dbs: Value of DBS bit
 * @dbs_plus_agile_scan: Value of DBS plus agile scan bit
 * @single_mac_scan_with_dbs: Value of Single MAC scan with DBS
 *
 * @OUTPUT: None
 *
 * This IOCTL is used to set the dual MAC scan config.
 *
 * @E.g: iwpriv wlan0 set_scan_cfg dbs dbs_plus_agile_scan
 *                    single_mac_scan_with_dbs
 * iwpriv wlan0 set_scan_cfg 1 0 1
 *
 * Supported Feature: DBS
 *
 * Usage: Internal/External
 *
 * </ioctl>
 */
#define WE_SET_DUAL_MAC_SCAN_CONFIG    21

/*
 * <ioctl>
 * set_fw_mode_cfg - Sets the dual mac FW mode config
 *
 * @INPUT: dbs, dfs
 * @dbs: DBS bit
 * @dfs: Agile DFS bit
 *
 * @OUTPUT: None
 *
 * This IOCTL is used to set the dual mac FW mode config.
 *
 * @E.g: iwpriv wlan0 set_fw_mode_cfg dbs dfs
 * iwpriv wlan0 set_fw_mode_cfg 1 1
 *
 * Supported Feature: DBS
 *
 * Usage: Internal/External
 *
 * </ioctl>
 */
#define WE_SET_DUAL_MAC_FW_MODE_CONFIG 22
#define WE_SET_MON_MODE_CHAN 23
/*
 * <ioctl>
 * txrx_stats - TXRX statistics query
 *
 * @INPUT: query category, mac id (default mac id is 0)
 *
 * @OUTPUT: TXRX statistics result
 *
 * This IOCTL is used to get TXRX statistics counters.
 *
 * @E.g: iwpriv wlan0 txrx_stats 21 0
 * iwpriv wlan0 txrx_stats 21 1
 *
 * Usage: Internal
 *
 * </ioctl>
 */
#define WE_SET_TXRX_STATS    24


#ifdef FEATURE_WLAN_TDLS
#undef  MAX_VAR_ARGS
#define MAX_VAR_ARGS         11
#else
#undef  MAX_VAR_ARGS
#define MAX_VAR_ARGS         9
#endif

/*
 * <ioctl>
 * fips_test - Perform a FIPS test
 *
 * @INPUT: Binary representation of the following packed structure
 *
 * @OUTPUT: Binary representation of the following packed structure
 *
 * This IOCTL is used to perform FIPS certification testing
 *
 * @E.g: iwpriv wlan0 fips_test <test vector>
 *
 * iwpriv wlan0 fips_test <tbd>
 *
 * Supported Feature: FIPS
 *
 * Usage: Internal
 *
 * </ioctl>
 */
#define WLAN_PRIV_FIPS_TEST (SIOCIWFIRSTPRIV +  8)

/* Private ioctls (with no sub-ioctls) */
/* note that they must be odd so that they have "get" semantics */
/*
 * <ioctl>
 * addTspec - Add TSPEC for each AC
 *
 * @INPUT: 19 TSPEC params
 *     @[arg0]: handle
 *     @[arg1]: tid
 *     @[arg2]: dir
 *     @[arg3]: psb
 *     @[arg4]: up
 *     @[arg5]: nomMsduSize
 *     @[arg6]: maxMsduSize
 *     @[arg7]: minDataRate
 *     @[arg8]: meanDataRate
 *     @[arg9]: peakDataRate
 *     @[arg10]: maxBurstSize
 *     @[arg11]: minPhyRate
 *     @[arg12]: sba
 *     @[arg13]: minServiceIntv
 *     @[arg14]: suspendIntv
 *     @[arg15]: burstSizeDefn
 *     @[arg16]: ackPolicy
 *     @[arg17]: inactivityPeriod
 *     @[arg18]: maxServiceIntv
 *
 * @OUTPUT: Success/Failure
 *
 * This IOCTL is used to add TSPEC for each AC.
 *
 * @E.g: iwpriv wlan0 addTspec <handle> <tid> <dir> <psb> <up> <nomMsduSize>
 *                             <maxMsduSize> <minDataRate> <meanDataRate>
 *                             <peakDataRate> <maxBurstSize> <minPhyRate>
 *                             <sba> <minServiceIntv> <suspendIntv>
 *                             <burstSizeDefn> <ackPolicy> <inactivityPeriod>
 *                             <maxServiceIntv>
 * iwpriv wlan0 addTspec 7001 6 2 1 6 0x80D0 0x80D0 0x14500 0x14500 0x14500
 *                      0 0x5B8D80 0x2001 20 2000 0 0 0 2000
 * wlan0     addTspec:3
 *
 * Supported Feature: WMM
 *
 * Usage: Internal/External
 *
 * </ioctl>
 */
#define WLAN_PRIV_ADD_TSPEC (SIOCIWFIRSTPRIV +  9)
/*
 * <ioctl>
 * delTspec - Delete TSPEC entry for each AC
 *
 * @INPUT: 1 TSPEC param
 *     @[arg0]: handle
 *
 * @OUTPUT: Success/Failure
 *
 * This IOCTL is used to delete TSPEC entry for each AC.
 *
 * @E.g: iwpriv wlan0 delTspec <handle>
 * iwpriv wlan0 delTspec 7001
 * wlan0     delTspec:16
 *
 * Supported Feature: WMM
 *
 * Usage: Internal/External
 *
 * </ioctl>
 */
#define WLAN_PRIV_DEL_TSPEC (SIOCIWFIRSTPRIV + 11)
/*
 * <ioctl>
 * getTspec - Get TSPEC entry for each AC
 *
 * @INPUT: 1 TSPEC param
 *     @[arg0]: handle
 *
 * @OUTPUT: Success/Failure
 *
 * This IOCTL is used to get TSPEC entry for each AC.
 *
 * @E.g: iwpriv wlan0 getTspec <handle>
 * iwpriv wlan0 getTspec 7001
 * wlan0     delTspec:18
 *
 * Supported Feature: WMM
 *
 * Usage: Internal/External
 *
 * </ioctl>
 */
#define WLAN_PRIV_GET_TSPEC (SIOCIWFIRSTPRIV + 13)

/* (SIOCIWFIRSTPRIV + 10) is currently unused */
/* (SIOCIWFIRSTPRIV + 12) is currently unused */
/* (SIOCIWFIRSTPRIV + 14) is currently unused */
#define WLAN_PRIV_SET_NONE_GET_THREE_INT   (SIOCIWFIRSTPRIV + 15)
#define WE_GET_TSF      1
/* (SIOCIWFIRSTPRIV + 16) is currently unused */
/* (SIOCIWFIRSTPRIV + 17) is currently unused */
/* (SIOCIWFIRSTPRIV + 19) is currently unused */

#define WLAN_PRIV_SET_FTIES             (SIOCIWFIRSTPRIV + 20)

/* Private ioctl for setting the host offload feature */
#define WLAN_PRIV_SET_HOST_OFFLOAD (SIOCIWFIRSTPRIV + 18)

/* Private ioctl to get the statistics */
#define WLAN_GET_WLAN_STATISTICS (SIOCIWFIRSTPRIV + 21)

/* Private ioctl to set the Keep Alive Params */
/*
 * <ioctl>
 * setKeepAlive - Set the keep alive feature
 *
 * @INPUT: 28 bytes of information in the order of packet type, time period
 * host IPv4 address, destination IPv4 address, destination MAC address, bssID
 *
 * @OUTPUT: None
 *
 * This IOCTL sets the keep alive feature to send either NULL
 * or unsolicited ARP response packets
 *
 * @E.g: iwpriv wlan0 setKeepAlive
 *
 * Supported Feature: STA
 *
 * Usage: Internal/External
 *
 * </ioctl>
 */
#define WLAN_SET_KEEPALIVE_PARAMS (SIOCIWFIRSTPRIV + 22)

#ifdef WLAN_FEATURE_PACKET_FILTERING
/* Private ioctl to set the packet filtering params */
#define WLAN_SET_PACKET_FILTER_PARAMS (SIOCIWFIRSTPRIV + 23)
#endif


#ifdef FEATURE_WLAN_SCAN_PNO
/* Private ioctl to get the statistics */
#define WLAN_SET_PNO (SIOCIWFIRSTPRIV + 24)
#endif
/*
 * <ioctl>
 * SETBAND - Set the operational band
 *
 * @INPUT: 0 to Auto, 1 to 5 GHz and 2 to 2.4 GHz
 *
 * @OUTPUT: None
 *
 * This IOCTL Set the operational band If the new band is different from the
 * current operational band, it aborts the pending scan requests, flushes
 * the existing scan results, and then change * the band capability
 *
 * @E.g: iwpriv wlan0 SETBAND <value>
 *
 * Supported Feature: STA
 *
 * Usage: Internal/External
 *
 * </ioctl>
 */
#define WLAN_SET_BAND_CONFIG  (SIOCIWFIRSTPRIV + 25)

#define WLAN_PRIV_SET_MCBC_FILTER   (SIOCIWFIRSTPRIV + 26)
/* (SIOCIWFIRSTPRIV + 27) is currently unused */

/* Private ioctls and their sub-ioctls */
#define WLAN_PRIV_SET_TWO_INT_GET_NONE   (SIOCIWFIRSTPRIV + 28)
#define WE_SET_SMPS_PARAM    1
#define WE_SET_FW_CRASH_INJECT    2
#define WE_DUMP_DP_TRACE_LEVEL    3
/* Private sub ioctl for enabling and setting histogram interval of profiling */
#define WE_ENABLE_FW_PROFILE    4
#define WE_SET_FW_PROFILE_HIST_INTVL    5

/* Private sub-ioctl for initiating WoW suspend without Apps suspend */
#define WE_SET_WLAN_SUSPEND    6
#define WE_SET_WLAN_RESUME    7

/*
 * <ioctl>
 * log_buffer - prints host/target related communication logs via dmesg
 *
 * @INPUT: Log Id, Count
 *
 * Log Id:
 *	0) HTC_CREDIT_HISTORY_LOG
 *	1) COMMAND_LOG,
 *	2) COMMAND_TX_CMP_LOG,
 *	3) MGMT_COMMAND_LOG,
 *	4) MGMT_COMMAND_TX_CMP_LOG,
 *	5) EVENT_LOG,
 *	6) RX_EVENT_LOG,
 *	7) MGMT_EVENT_LOG
 *
 * @OUTPUT: None
 *
 * @E.g:
 * # print up to 10 of the most recent records from HTC Credit History
 *	iwpriv wlan0 log_buffer 0 10
 * # print up to 3 of the most recent records from Event Log
 *	iwpriv wlan0 log_buffer 5 3
 *
 * Supported Feature: WLAN Trace
 *
 * Usage: Internal/External
 *
 * </ioctl>
 */
#define WE_LOG_BUFFER			8

/*
 * <ioctl>
 * set_ba_timeout - sets Block ACK aging timeout value for each Access class
 *
 * @INPUT: Access Class [0:BK, 1:BE, 2:VI, 3:VO], Timeout value
 *
 * @OUTPUT: None
 *
 * @E.g.:
 * # to set duration of 2 seconds for BE
 *	iwpriv wlan0 set_ba_timeout 1 2
 * # to set duration of 3 seconds for VO
 *	iwpriv wlan0 set_ba_timeout 3 3
 *
 * Usage: Internal
 *
 * </ioctl>
 */
#define WE_SET_BA_AGEING_TIMEOUT		9

enum host_target_comm_log {
	HTC_CREDIT_HISTORY_LOG = 0,
	COMMAND_LOG,
	COMMAND_TX_CMP_LOG,
	MGMT_COMMAND_LOG,
	MGMT_COMMAND_TX_CMP_LOG,
	EVENT_LOG,
	RX_EVENT_LOG,
	MGMT_EVENT_LOG
};

/* (SIOCIWFIRSTPRIV + 29) is currently unused */

/* 802.11p IOCTL */
#define WLAN_SET_DOT11P_CHANNEL_SCHED    (SIOCIWFIRSTPRIV + 30)

/*
 * <ioctl>
 * getLinkSpeed - Gets the current link speed in Mbps
 *
 * @INPUT: None
 *
 * @OUTPUT: linkspeed in mbps
 *  wlan0     getLinkSpeed:7
 *
 * This IOCTL is used get the current link speed in Mbps
 *
 * @E.g: iwpriv wlan0 getLinkSpeed
 *
 * Supported Feature: STA
 *
 * Usage: Internal/External
 *
 * </ioctl>
 */
#define WLAN_GET_LINK_SPEED          (SIOCIWFIRSTPRIV + 31)

#define WLAN_STATS_INVALID            0
#define WLAN_STATS_RETRY_CNT          1
#define WLAN_STATS_MUL_RETRY_CNT      2
#define WLAN_STATS_TX_FRM_CNT         3
#define WLAN_STATS_RX_FRM_CNT         4
#define WLAN_STATS_FRM_DUP_CNT        5
#define WLAN_STATS_FAIL_CNT           6
#define WLAN_STATS_RTS_FAIL_CNT       7
#define WLAN_STATS_ACK_FAIL_CNT       8
#define WLAN_STATS_RTS_SUC_CNT        9
#define WLAN_STATS_RX_DISCARD_CNT     10
#define WLAN_STATS_RX_ERROR_CNT       11
#define WLAN_STATS_TX_BYTE_CNT        12

#define WLAN_STATS_RX_BYTE_CNT        13
#define WLAN_STATS_RX_RATE            14
#define WLAN_STATS_TX_RATE            15

#define WLAN_STATS_RX_UC_BYTE_CNT     16
#define WLAN_STATS_RX_MC_BYTE_CNT     17
#define WLAN_STATS_RX_BC_BYTE_CNT     18
#define WLAN_STATS_TX_UC_BYTE_CNT     19
#define WLAN_STATS_TX_MC_BYTE_CNT     20
#define WLAN_STATS_TX_BC_BYTE_CNT     21

#define FILL_TLV(__p, __type, __size, __val, __tlen) do {		\
		if ((__tlen + __size + 2) < WE_MAX_STR_LEN) {		\
			*__p++ = __type;				\
			*__p++ = __size;				\
			memcpy(__p, __val, __size);			\
			__p += __size;					\
			__tlen += __size + 2;				\
		} else {						\
			hdd_err("FILL_TLV Failed!!!");  \
		}							\
	} while (0)

#define VERSION_VALUE_MAX_LEN 32

#define TX_PER_TRACKING_DEFAULT_RATIO             5
#define TX_PER_TRACKING_MAX_RATIO                10
#define TX_PER_TRACKING_DEFAULT_WATERMARK         5

#define WLAN_ADAPTER 0
#define P2P_ADAPTER  1

/**
 * mem_alloc_copy_from_user_helper - copy from user helper
 * @wrqu_data: wireless extensions request data
 * @len: length of @wrqu_data
 *
 * Helper function to allocate buffer and copy user data.
 *
 * Return: On success return a pointer to a kernel buffer containing a
 * copy of the userspace data (with an additional NUL character
 * appended for safety). On failure return %NULL.
 */
void *mem_alloc_copy_from_user_helper(const __user void *wrqu_data, size_t len)
{
	u8 *ptr = NULL;

	/* in order to protect the code, an extra byte is post
	 * appended to the buffer and the null termination is added.
	 * However, when allocating (len+1) byte of memory, we need to
	 * make sure that there is no uint overflow when doing
	 * addition. In theory check len < UINT_MAX protects the uint
	 * overflow. For wlan private ioctl, the buffer size is much
	 * less than UINT_MAX, as a good guess, now, it is assumed
	 * that the private command buffer size is no greater than 4K
	 * (4096 bytes). So we use 4096 as the upper boundary for now.
	 */
	if (len > MAX_USER_COMMAND_SIZE) {
		hdd_err("Invalid length: %zu max: %u",
			 len, MAX_USER_COMMAND_SIZE);
		return NULL;
	}

	ptr = qdf_mem_malloc(len + 1);
	if (NULL == ptr) {
		hdd_err("unable to allocate memory");
		return NULL;
	}

	if (copy_from_user(ptr, wrqu_data, len)) {
		hdd_err("failed to copy data to user buffer");
		qdf_mem_free(ptr);
		return NULL;
	}
	ptr[len] = '\0';
	return ptr;
}

/**
 * hdd_priv_get_data() - Get pointer to ioctl private data
 * @p_priv_data: pointer to iw_point struct to be filled
 * @wrqu: Pointer to IOCTL Data received from userspace
 *
 * Helper function to get compatible struct iw_point passed to ioctl
 *
 * Return - 0 if p_priv_data successfully filled, error otherwise
 */
int hdd_priv_get_data(struct iw_point *p_priv_data, union iwreq_data *wrqu)
{
	if ((NULL == p_priv_data) || (NULL == wrqu))
		return -EINVAL;

#ifdef CONFIG_COMPAT
	if (in_compat_syscall()) {
		struct compat_iw_point *p_compat_priv_data;

		/* Compat task:
		 * typecast to compat structure and copy the members.
		 */
		p_compat_priv_data = (struct compat_iw_point *)&wrqu->data;

		p_priv_data->pointer = compat_ptr(p_compat_priv_data->pointer);
		p_priv_data->length = p_compat_priv_data->length;
		p_priv_data->flags = p_compat_priv_data->flags;
	} else {
#endif /* #ifdef CONFIG_COMPAT */

		/* Non compat task: directly copy the structure. */
		memcpy(p_priv_data, &wrqu->data, sizeof(struct iw_point));

#ifdef CONFIG_COMPAT
	}
#endif /* #ifdef CONFIG_COMPAT */

	return 0;
}

static int hdd_check_wext_control(enum hdd_wext_control wext_control,
				  struct iw_request_info *info)
{
	switch (wext_control) {
	default:
	case hdd_wext_disabled:
		hdd_err("Rejecting disabled ioctl %x", info->cmd);
		return -ENOTSUPP;
	case hdd_wext_deprecated:
		hdd_warn("Using deprecated ioctl %x", info->cmd);
		return 0;
	case hdd_wext_enabled:
		return 0;
	}
}

int hdd_check_private_wext_control(struct hdd_context *hdd_ctx,
				   struct iw_request_info *info)
{
	return hdd_check_wext_control(hdd_ctx->config->private_wext_control,
				      info);
}

/**
 * hdd_wlan_get_stats() - Get txrx stats in SAP mode
 * @adapter: Pointer to the hdd adapter.
 * @length:   Size of the data copied
 * @buffer:   Pointer to char buffer.
 * @buf_len:  Length of the char buffer.
 *
 * This function called when the "iwpriv wlan0 get_stats" command is given.
 * It used to collect the txrx stats when the device is configured in SAP mode.
 *
 * Return - none
 */
void hdd_wlan_get_stats(struct hdd_adapter *adapter, uint16_t *length,
			char *buffer, uint16_t buf_len)
{
	struct hdd_tx_rx_stats *stats = &adapter->hdd_stats.tx_rx_stats;
	uint32_t len = 0;
	uint32_t total_rx_pkt = 0, total_rx_dropped = 0;
	uint32_t total_rx_delv = 0, total_rx_refused = 0;
	int i = 0;

	for (; i < NUM_CPUS; i++) {
		total_rx_pkt += stats->rx_packets[i];
		total_rx_dropped += stats->rx_dropped[i];
		total_rx_delv += stats->rx_delivered[i];
		total_rx_refused += stats->rx_refused[i];
	}

	len = scnprintf(buffer, buf_len,
		"\nTransmit[%lu] - "
		"called %u, dropped %u orphan %u,"
		"\n[dropped]    BK %u, BE %u, VI %u, VO %u"
		"\n[classified] BK %u, BE %u, VI %u, VO %u"
		"\n\nReceive[%lu] - "
		"packets %u, dropped %u, delivered %u, refused %u"
		"\n",
		qdf_system_ticks(),
		stats->tx_called,
		stats->tx_dropped,
		stats->tx_orphaned,

		stats->tx_dropped_ac[SME_AC_BK],
		stats->tx_dropped_ac[SME_AC_BE],
		stats->tx_dropped_ac[SME_AC_VI],
		stats->tx_dropped_ac[SME_AC_VO],

		stats->tx_classified_ac[SME_AC_BK],
		stats->tx_classified_ac[SME_AC_BE],
		stats->tx_classified_ac[SME_AC_VI],
		stats->tx_classified_ac[SME_AC_VO],
		qdf_system_ticks(),
		total_rx_pkt, total_rx_dropped, total_rx_delv, total_rx_refused
		);

	for (i = 0; i < NUM_CPUS; i++) {
		if (stats->rx_packets[i] == 0)
			continue;
		len += scnprintf(buffer + len, buf_len - len,
			"Rx CPU[%d]:"
			"packets %u, dropped %u, delivered %u, refused %u\n",
			i, stats->rx_packets[i], stats->rx_dropped[i],
			stats->rx_delivered[i], stats->rx_refused[i]);
	}

	len += scnprintf(buffer + len, buf_len - len,
		"\nTX_FLOW"
		"\nCurrent status: %s"
		"\ntx-flow timer start count %u"
		"\npause count %u, unpause count %u",
		(stats->is_txflow_paused == true ? "PAUSED" : "UNPAUSED"),
		stats->txflow_timer_cnt,
		stats->txflow_pause_cnt,
		stats->txflow_unpause_cnt);

	len += cdp_stats(cds_get_context(QDF_MODULE_ID_SOC),
		adapter->session_id, &buffer[len], (buf_len - len));
	*length = len + 1;
}

/**
 * wlan_hdd_write_suspend_resume_stats() - Writes suspend/resume stats to buffer
 * @hdd_ctx: The Hdd context owning the stats to be written
 * @buffer: The char buffer to write to
 * @max_len: The maximum number of chars to write
 *
 * This assumes hdd_ctx has already been validated, and buffer is not NULL.
 *
 * Return - length of written content, negative number on error
 */
#ifdef QCA_SUPPORT_CP_STATS
static int wlan_hdd_write_suspend_resume_stats(struct hdd_context *hdd_ctx,
					       char *buffer, uint16_t max_len)
{
	int ret;
	QDF_STATUS status;
	struct suspend_resume_stats *sr_stats;

	sr_stats = &hdd_ctx->suspend_resume_stats;
	ret = scnprintf(buffer, max_len,
			"\n"
			"Suspends: %u\n"
			"Resumes: %u\n"
			"\n"
			"Suspend Fail Reasons\n"
			"\tIPA: %u\n"
			"\tRadar: %u\n"
			"\tRoam: %u\n"
			"\tScan: %u\n"
			"\tInitial Wakeup: %u\n"
			"\n",
			sr_stats->suspends, sr_stats->resumes,
			sr_stats->suspend_fail[SUSPEND_FAIL_IPA],
			sr_stats->suspend_fail[SUSPEND_FAIL_RADAR],
			sr_stats->suspend_fail[SUSPEND_FAIL_ROAM],
			sr_stats->suspend_fail[SUSPEND_FAIL_SCAN],
			sr_stats->suspend_fail[SUSPEND_FAIL_INITIAL_WAKEUP]);

	status = ucfg_mc_cp_stats_write_wow_stats(hdd_ctx->hdd_psoc,
						  &buffer[ret], max_len - ret,
						  &ret);
	if (QDF_IS_STATUS_ERROR(status)) {
		hdd_err("Failed to get WoW stats");
		return qdf_status_to_os_return(status);
	}

	return ret;
}
#else
static int wlan_hdd_write_suspend_resume_stats(struct hdd_context *hdd_ctx,
					       char *buffer, uint16_t max_len)
{
	QDF_STATUS status;
	struct suspend_resume_stats *sr_stats;
	struct sir_wake_lock_stats wow_stats;

	sr_stats = &hdd_ctx->suspend_resume_stats;

	status = wma_get_wakelock_stats(&wow_stats);
	if (QDF_IS_STATUS_ERROR(status)) {
		hdd_err("Failed to get WoW stats");
		return qdf_status_to_os_return(status);
	}

	return scnprintf(buffer, max_len,
			"\n"
			"Suspends: %u\n"
			"Resumes: %u\n"
			"\n"
			"Suspend Fail Reasons\n"
			"\tIPA: %u\n"
			"\tRadar: %u\n"
			"\tRoam: %u\n"
			"\tScan: %u\n"
			"\tInitial Wakeup: %u\n"
			"\n"
			"WoW Wake Reasons\n"
			"\tunicast: %u\n"
			"\tbroadcast: %u\n"
			"\tIPv4 multicast: %u\n"
			"\tIPv6 multicast: %u\n"
			"\tIPv6 multicast RA: %u\n"
			"\tIPv6 multicast NS: %u\n"
			"\tIPv6 multicast NA: %u\n"
			"\tICMPv4: %u\n"
			"\tICMPv6: %u\n"
			"\tRSSI Breach: %u\n"
			"\tLow RSSI: %u\n"
			"\tG-Scan: %u\n"
			"\tPNO Complete: %u\n"
			"\tPNO Match: %u\n",
			sr_stats->suspends,
			sr_stats->resumes,
			sr_stats->suspend_fail[SUSPEND_FAIL_IPA],
			sr_stats->suspend_fail[SUSPEND_FAIL_RADAR],
			sr_stats->suspend_fail[SUSPEND_FAIL_ROAM],
			sr_stats->suspend_fail[SUSPEND_FAIL_SCAN],
			sr_stats->suspend_fail[SUSPEND_FAIL_INITIAL_WAKEUP],
			wow_stats.wow_ucast_wake_up_count,
			wow_stats.wow_bcast_wake_up_count,
			wow_stats.wow_ipv4_mcast_wake_up_count,
			wow_stats.wow_ipv6_mcast_wake_up_count,
			wow_stats.wow_ipv6_mcast_ra_stats,
			wow_stats.wow_ipv6_mcast_ns_stats,
			wow_stats.wow_ipv6_mcast_na_stats,
			wow_stats.wow_icmpv4_count,
			wow_stats.wow_icmpv6_count,
			wow_stats.wow_rssi_breach_wake_up_count,
			wow_stats.wow_low_rssi_wake_up_count,
			wow_stats.wow_gscan_wake_up_count,
			wow_stats.wow_pno_complete_wake_up_count,
			wow_stats.wow_pno_match_wake_up_count);
}
#endif
/**
 * hdd_wlan_list_fw_profile() - Get fw profiling points
 * @length:   Size of the data copied
 * @buffer:   Pointer to char buffer.
 * @buf_len:  Length of the char buffer.
 *
 * This function called when the "iwpriv wlan0 listProfile" command is given.
 * It is used to get the supported profiling points in FW.
 *
 * Return - none
 */
void hdd_wlan_list_fw_profile(uint16_t *length,
			char *buffer, uint16_t buf_len)
{
	uint32_t len = 0;

	len = scnprintf(buffer, buf_len,
		    "PROF_CPU_IDLE: %u\n"
		    "PROF_PPDU_PROC: %u\n"
		    "PROF_PPDU_POST: %u\n"
		    "PROF_HTT_TX_INPUT: %u\n"
		    "PROF_MSDU_ENQ: %u\n"
		    "PROF_PPDU_POST_HAL: %u\n"
		    "PROF_COMPUTE_TX_TIME: %u\n",
		    PROF_CPU_IDLE,
		    PROF_PPDU_PROC,
		    PROF_PPDU_POST,
		    PROF_HTT_TX_INPUT,
		    PROF_MSDU_ENQ,
		    PROF_PPDU_POST_HAL,
		    PROF_COMPUTE_TX_TIME);

	*length = len + 1;
}
/**
 * hdd_display_stats_help() - print statistics help
 *
 * Return: none
 */
static void hdd_display_stats_help(void)
{
	hdd_nofl_info("iwpriv wlan0 dumpStats [option] - dump statistics");
	hdd_nofl_info("iwpriv wlan0 clearStats [option] - clear statistics");
	hdd_nofl_info("options:");
	hdd_nofl_info("  1 -- TXRX PATH statistics");
	hdd_nofl_info("  2 -- TXRX HIST statistics");
	hdd_nofl_info("  3 -- TSO statistics");
	hdd_nofl_info("  4 -- Network queue statistics");
	hdd_nofl_info("  5 -- Flow control statistics");
	hdd_nofl_info("  6 -- Per Layer statistics");
	hdd_nofl_info("  7 -- Copy engine interrupt statistics");
	hdd_nofl_info("  9 -- NAPI statistics");
}

/**
 * hdd_wlan_dump_stats() - display dump Stats
 * @adapter: adapter handle
 * @value: value from user
 *
 * Return: 0 => success, error code on failure
 */
int hdd_wlan_dump_stats(struct hdd_adapter *adapter, int value)
{
	int ret = 0;
	QDF_STATUS status;
	struct hdd_context *hdd_ctx = WLAN_HDD_GET_CTX(adapter);

	switch (value) {
	case CDP_TXRX_HIST_STATS:
		wlan_hdd_display_tx_rx_histogram(hdd_ctx);
		break;

	case CDP_HDD_NETIF_OPER_HISTORY:
		wlan_hdd_display_netif_queue_history
					(hdd_ctx,
					 QDF_STATS_VERBOSITY_LEVEL_HIGH);
		break;

	case CDP_HIF_STATS:
		hdd_display_hif_stats();
		break;

	case CDP_LRO_STATS:
		hdd_lro_display_stats(hdd_ctx);
		break;

	case CDP_NAPI_STATS:
		if (hdd_display_napi_stats()) {
			hdd_err("error displaying napi stats");
			ret = EFAULT;
		}
		break;

	case CDP_DISCONNECT_STATS:
		sme_display_disconnect_stats(hdd_ctx->mac_handle,
					     adapter->session_id);
		break;

	default:
		status = cdp_display_stats(cds_get_context(QDF_MODULE_ID_SOC),
					   value,
					   QDF_STATS_VERBOSITY_LEVEL_HIGH);
		if (status == QDF_STATUS_E_INVAL) {
			hdd_display_stats_help();
			ret = EINVAL;
		}
		break;
	}
	return ret;
}

/**
 * hdd_wlan_get_ibss_peer_info() - Print IBSS peer information
 * @adapter: Adapter upon which the IBSS client is active
 * @staIdx: Station index of the IBSS peer
 *
 * Return: QDF_STATUS_STATUS if the peer was found and displayed,
 * otherwise an appropriate QDF_STATUS_E_* failure code.
 */
static QDF_STATUS hdd_wlan_get_ibss_peer_info(struct hdd_adapter *adapter,
					      uint8_t staIdx)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	mac_handle_t mac_handle = adapter->hdd_ctx->mac_handle;
	struct hdd_station_ctx *pStaCtx = WLAN_HDD_GET_STATION_CTX_PTR(adapter);
	tSirPeerInfoRspParams *pPeerInfo = &pStaCtx->ibss_peer_info;

	INIT_COMPLETION(adapter->ibss_peer_info_comp);
	status = sme_request_ibss_peer_info(mac_handle, adapter,
					    hdd_get_ibss_peer_info_cb,
					    false, staIdx);

	if (QDF_STATUS_SUCCESS == status) {
		unsigned long rc;

		rc = wait_for_completion_timeout
			     (&adapter->ibss_peer_info_comp,
			     msecs_to_jiffies(IBSS_PEER_INFO_REQ_TIMOEUT));
		if (!rc) {
			hdd_err("failed wait on ibss_peer_info_comp");
			return QDF_STATUS_E_FAILURE;
		}

		/** Print the peer info */
		hdd_debug("pPeerInfo->numIBSSPeers = %d ", pPeerInfo->numPeers);
		{
			uint8_t mac_addr[QDF_MAC_ADDR_SIZE];
			uint32_t tx_rate = pPeerInfo->peerInfoParams[0].txRate;

			qdf_mem_copy(mac_addr, pPeerInfo->peerInfoParams[0].
					mac_addr, sizeof(mac_addr));
			hdd_debug("PEER ADDR : %pM TxRate: %d Mbps  RSSI: %d",
				mac_addr, (int)tx_rate,
				(int)pPeerInfo->peerInfoParams[0].rssi);
		}
	} else {
		hdd_warn("Warning: sme_request_ibss_peer_info Request failed");
	}

	return status;
}

/**
 * hdd_wlan_get_ibss_peer_info_all() - Print all IBSS peers
 * @adapter: Adapter upon which the IBSS clients are active
 *
 * Return: QDF_STATUS_STATUS if the peer information was retrieved and
 * displayed, otherwise an appropriate QDF_STATUS_E_* failure code.
 */
static QDF_STATUS hdd_wlan_get_ibss_peer_info_all(struct hdd_adapter *adapter)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	mac_handle_t mac_handle = adapter->hdd_ctx->mac_handle;
	struct hdd_station_ctx *pStaCtx = WLAN_HDD_GET_STATION_CTX_PTR(adapter);
	tSirPeerInfoRspParams *pPeerInfo = &pStaCtx->ibss_peer_info;
	int i;

	INIT_COMPLETION(adapter->ibss_peer_info_comp);
	status = sme_request_ibss_peer_info(mac_handle, adapter,
					    hdd_get_ibss_peer_info_cb,
					    true, 0xFF);

	if (QDF_STATUS_SUCCESS == status) {
		unsigned long rc;

		rc = wait_for_completion_timeout
			     (&adapter->ibss_peer_info_comp,
			     msecs_to_jiffies(IBSS_PEER_INFO_REQ_TIMOEUT));
		if (!rc) {
			hdd_err("failed wait on ibss_peer_info_comp");
			return QDF_STATUS_E_FAILURE;
		}

		/** Print the peer info */
		hdd_debug("pPeerInfo->numIBSSPeers = %d ",
			(int)pPeerInfo->numPeers);
		for (i = 0; i < pPeerInfo->numPeers; i++) {
			uint8_t mac_addr[QDF_MAC_ADDR_SIZE];
			uint32_t tx_rate;

			tx_rate = pPeerInfo->peerInfoParams[i].txRate;
			qdf_mem_copy(mac_addr,
				pPeerInfo->peerInfoParams[i].mac_addr,
				sizeof(mac_addr));

			hdd_debug(" PEER ADDR : %pM TxRate: %d Mbps RSSI: %d",
				mac_addr, (int)tx_rate,
				(int)pPeerInfo->peerInfoParams[i].rssi);
		}
	} else {
		hdd_warn("Warning: sme_request_ibss_peer_info Request failed");
	}

	return status;
}

/**
 * hdd_get_ldpc() - Get adapter LDPC
 * @adapter: adapter being queried
 * @value: where to store the value
 *
 * Return: 0 on success, negative errno on failure
 */
int hdd_get_ldpc(struct hdd_adapter *adapter, int *value)
{
	mac_handle_t mac_handle = adapter->hdd_ctx->mac_handle;
	int ret;

	hdd_enter();
	ret = sme_get_ht_config(mac_handle, adapter->session_id,
				WNI_CFG_HT_CAP_INFO_ADVANCE_CODING);
	if (ret < 0) {
		hdd_err("Failed to get LDPC value");
	} else {
		*value = ret;
		ret = 0;
	}
	return ret;
}

/**
 * hdd_set_ldpc() - Set adapter LDPC
 * @adapter: adapter being modified
 * @value: new LDPC value
 *
 * Return: 0 on success, negative errno on failure
 */
int hdd_set_ldpc(struct hdd_adapter *adapter, int value)
{
	mac_handle_t mac_handle = adapter->hdd_ctx->mac_handle;
	int ret;
	QDF_STATUS status;
	struct hdd_context *hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	struct hdd_config *config = hdd_ctx->config;
	struct mlme_ht_capabilities_info ht_cap_info;

	hdd_debug("%d", value);
	if (value) {
		/* make sure HT capabilities allow this */
		if (!config->enable_rx_ldpc) {
			hdd_err("LDCP not supported");
			return -EINVAL;
		}
	} else if (!config->enable_rx_ldpc) {
		hdd_err("LDCP is already disabled");
		return 0;
	}
	status = ucfg_mlme_get_ht_cap_info(hdd_ctx->hdd_psoc, &ht_cap_info);
	if (QDF_STATUS_SUCCESS != status) {
		hdd_err("Failed to get HT capability info");
		return -EIO;
	}

	ht_cap_info.adv_coding_cap = value;
	status = ucfg_mlme_set_ht_cap_info(hdd_ctx->hdd_psoc, ht_cap_info);
	if (QDF_STATUS_SUCCESS != status) {
		hdd_err("Failed to set HT capability info");
		return -EIO;
	}
	status = sme_cfg_set_int(mac_handle, WNI_CFG_VHT_LDPC_CODING_CAP,
				 value);
	if (QDF_STATUS_SUCCESS != status) {
		hdd_err("Failed to set VHT LDPC capability info");
		return -EIO;
	}

	ret = sme_update_ht_config(mac_handle, adapter->session_id,
				   WNI_CFG_HT_CAP_INFO_ADVANCE_CODING,
				   value);
	if (ret)
		hdd_err("Failed to set LDPC value");
	ret = sme_update_he_ldpc_supp(mac_handle, adapter->session_id, value);
	if (ret)
		hdd_err("Failed to set HE LDPC value");

	return ret;
}

/**
 * hdd_get_tx_stbc() - Get adapter TX STBC
 * @adapter: adapter being queried
 * @value: where to store the value
 *
 * Return: 0 on success, negative errno on failure
 */
int hdd_get_tx_stbc(struct hdd_adapter *adapter, int *value)
{
	mac_handle_t mac_handle = adapter->hdd_ctx->mac_handle;
	int ret;

	hdd_enter();
	ret = sme_get_ht_config(mac_handle, adapter->session_id,
				WNI_CFG_HT_CAP_INFO_TX_STBC);
	if (ret < 0) {
		hdd_err("Failed to get TX STBC value");
	} else {
		*value = ret;
		ret = 0;
	}

	return ret;
}

/**
 * hdd_set_tx_stbc() - Set adapter TX STBC
 * @adapter: adapter being modified
 * @value: new TX STBC value
 *
 * Return: 0 on success, negative errno on failure
 */
int hdd_set_tx_stbc(struct hdd_adapter *adapter, int value)
{
	mac_handle_t mac_handle = adapter->hdd_ctx->mac_handle;
	struct hdd_context *hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	int ret;
	QDF_STATUS status;
	struct mlme_ht_capabilities_info ht_cap_info;

	hdd_debug("%d", value);
	if (value) {
		/* make sure HT capabilities allow this */
		status = ucfg_mlme_get_ht_cap_info(hdd_ctx->hdd_psoc,
						   &ht_cap_info);
		if (QDF_STATUS_SUCCESS != status) {
			hdd_err("Failed to get HT capability info");
			return -EIO;
		}
		if (!ht_cap_info.tx_stbc) {
			hdd_err("TX STBC not supported");
			return -EINVAL;
		}
	}
	ret = sme_update_ht_config(mac_handle, adapter->session_id,
				   WNI_CFG_HT_CAP_INFO_TX_STBC,
				   value);
	if (ret)
		hdd_err("Failed to set TX STBC value");
	ret = sme_update_he_tx_stbc_cap(mac_handle, adapter->session_id, value);
	if (ret)
		hdd_err("Failed to set HE TX STBC value");

	return ret;
}

/**
 * hdd_get_rx_stbc() - Get adapter RX STBC
 * @adapter: adapter being queried
 * @value: where to store the value
 *
 * Return: 0 on success, negative errno on failure
 */
int hdd_get_rx_stbc(struct hdd_adapter *adapter, int *value)
{
	mac_handle_t mac_handle = adapter->hdd_ctx->mac_handle;
	int ret;

	hdd_enter();
	ret = sme_get_ht_config(mac_handle, adapter->session_id,
				WNI_CFG_HT_CAP_INFO_RX_STBC);
	if (ret < 0) {
		hdd_err("Failed to get RX STBC value");
	} else {
		*value = ret;
		ret = 0;
	}

	return ret;
}

/**
 * hdd_set_rx_stbc() - Set adapter RX STBC
 * @adapter: adapter being modified
 * @value: new RX STBC value
 *
 * Return: 0 on success, negative errno on failure
 */
int hdd_set_rx_stbc(struct hdd_adapter *adapter, int value)
{
	mac_handle_t mac_handle = adapter->hdd_ctx->mac_handle;
	struct hdd_context *hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	int ret;
	QDF_STATUS status;
	struct mlme_ht_capabilities_info ht_cap_info;

	hdd_debug("%d", value);
	if (value) {
		/* make sure HT capabilities allow this */
		status = ucfg_mlme_get_ht_cap_info(hdd_ctx->hdd_psoc,
						   &ht_cap_info);
		if (QDF_STATUS_SUCCESS != status) {
			hdd_err("Failed to get HT capability info");
			return -EIO;
		}
		if (!ht_cap_info.rx_stbc) {
			hdd_warn("RX STBC not supported");
			return -EINVAL;
		}
	}
	ret = sme_update_ht_config(mac_handle, adapter->session_id,
				   WNI_CFG_HT_CAP_INFO_RX_STBC,
				   value);
	if (ret)
		hdd_err("Failed to set RX STBC value");

	ret = sme_update_he_rx_stbc_cap(mac_handle, adapter->session_id, value);
	if (ret)
		hdd_err("Failed to set HE RX STBC value");

	return ret;
}

/**
 * iw_get_linkspeed() - Get current link speed ioctl
 * @dev: device upon which the ioctl was received
 * @info: ioctl request information
 * @wrqu: ioctl request data
 * @extra: extra ioctl buffer
 *
 * Return: 0 on success, non-zero on error
 */
static int __iw_get_linkspeed(struct net_device *dev,
			      struct iw_request_info *info,
			      union iwreq_data *wrqu, char *extra)
{
	struct hdd_adapter *adapter = WLAN_HDD_GET_PRIV_PTR(dev);
	char *pLinkSpeed = (char *)extra;
	int len = sizeof(uint32_t) + 1;
	uint32_t link_speed = 0;
	struct hdd_context *hdd_ctx;
	int ret;
	int rc;

	hdd_enter_dev(dev);

	hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	ret = wlan_hdd_validate_context(hdd_ctx);
	if (0 != ret)
		return ret;

	ret = hdd_check_private_wext_control(hdd_ctx, info);
	if (0 != ret)
		return ret;

	ret = wlan_hdd_get_link_speed(adapter, &link_speed);
	if (0 != ret)
		return ret;

	wrqu->data.length = len;
	/* return the linkspeed as a string */
	rc = snprintf(pLinkSpeed, len, "%u", link_speed);
	if ((rc < 0) || (rc >= len)) {
		/* encoding or length error? */
		hdd_err("Unable to encode link speed");
		return -EIO;
	}

	hdd_exit();
	/* a value is being successfully returned */
	return 0;
}

static int iw_get_linkspeed(struct net_device *dev,
			    struct iw_request_info *info,
			    union iwreq_data *wrqu, char *extra)
{
	int ret;

	cds_ssr_protect(__func__);
	ret = __iw_get_linkspeed(dev, info, wrqu, extra);
	cds_ssr_unprotect(__func__);

	return ret;
}

/**
 * wlan_hdd_update_phymode() - handle change in PHY mode
 * @net: device upon which PHY mode change was received
 * @mac_handle: umac handle for the driver
 * @new_phymode: new PHY mode for the device
 * @phddctx: pointer to the HDD context
 *
 * This function is called when the device is set to a new PHY mode.
 * It takes a holistic look at the desired PHY mode along with the
 * configured capabilities of the driver and the reported capabilities
 * of the hardware in order to correctly configure all PHY-related
 * parameters.
 *
 * Return: 0 on success, negative errno value on error
 */
int wlan_hdd_update_phymode(struct net_device *net, mac_handle_t mac_handle,
			    int new_phymode, struct hdd_context *phddctx)
{
#ifdef QCA_HT_2040_COEX
	struct hdd_adapter *adapter = WLAN_HDD_GET_PRIV_PTR(net);
	QDF_STATUS halStatus = QDF_STATUS_E_FAILURE;
#endif
	bool band_24 = false, band_5g = false;
	bool ch_bond24 = false, ch_bond5g = false;
	tSmeConfigParams *sme_config;
	uint32_t chwidth = WNI_CFG_CHANNEL_BONDING_MODE_DISABLE;
	uint32_t vhtchanwidth;
	eCsrPhyMode phymode = -EIO, old_phymode;
	enum hdd_dot11_mode hdd_dot11mode = phddctx->config->dot11Mode;
	enum band_info curr_band = BAND_ALL;
	int retval = 0;

	old_phymode = sme_get_phy_mode(mac_handle);

	if (WNI_CFG_CHANNEL_BONDING_MODE_DISABLE !=
	    sme_get_cb_phy_state_from_cb_ini_value(phddctx->config->
						   nChannelBondingMode24GHz))
		ch_bond24 = true;

	if (WNI_CFG_CHANNEL_BONDING_MODE_DISABLE !=
	    sme_get_cb_phy_state_from_cb_ini_value(phddctx->config->
						   nChannelBondingMode5GHz))
		ch_bond5g = true;

	if (phddctx->config->nBandCapability == BAND_ALL)
		band_24 = band_5g = true;
	else if (phddctx->config->nBandCapability == BAND_2G)
		band_24 = true;
	else if (phddctx->config->nBandCapability == BAND_5G)
		band_5g = true;

	vhtchanwidth = phddctx->config->vhtChannelWidth;
	hdd_debug("ch_bond24=%d ch_bond5g=%d band_24=%d band_5g=%d VHT_ch_width=%u",
		ch_bond24, ch_bond5g, band_24, band_5g, vhtchanwidth);

	switch (new_phymode) {
	case IEEE80211_MODE_AUTO:
		sme_set_phy_mode(mac_handle, eCSR_DOT11_MODE_AUTO);
		if (hdd_reg_set_band(net, WLAN_HDD_UI_BAND_AUTO) == 0) {
			phymode = eCSR_DOT11_MODE_AUTO;
			hdd_dot11mode = eHDD_DOT11_MODE_AUTO;
			chwidth = WNI_CFG_CHANNEL_BONDING_MODE_ENABLE;
			curr_band = BAND_ALL;
			vhtchanwidth = eHT_CHANNEL_WIDTH_80MHZ;
		} else {
			sme_set_phy_mode(mac_handle, old_phymode);
			return -EIO;
		}
		break;
	case IEEE80211_MODE_11A:
		sme_set_phy_mode(mac_handle, eCSR_DOT11_MODE_11a);
		if (hdd_reg_set_band(net, WLAN_HDD_UI_BAND_5_GHZ) == 0) {
			phymode = eCSR_DOT11_MODE_11a;
			hdd_dot11mode = eHDD_DOT11_MODE_11a;
			chwidth = WNI_CFG_CHANNEL_BONDING_MODE_DISABLE;
			curr_band = BAND_5G;
		} else {
			sme_set_phy_mode(mac_handle, old_phymode);
			return -EIO;
		}
		break;
	case IEEE80211_MODE_11B:
		sme_set_phy_mode(mac_handle, eCSR_DOT11_MODE_11b);
		if (hdd_reg_set_band(net, WLAN_HDD_UI_BAND_2_4_GHZ) == 0) {
			phymode = eCSR_DOT11_MODE_11b;
			hdd_dot11mode = eHDD_DOT11_MODE_11b;
			chwidth = WNI_CFG_CHANNEL_BONDING_MODE_DISABLE;
			curr_band = BAND_2G;
		} else {
			sme_set_phy_mode(mac_handle, old_phymode);
			return -EIO;
		}
		break;
	case IEEE80211_MODE_11G:
		sme_set_phy_mode(mac_handle, eCSR_DOT11_MODE_11g);
		if (hdd_reg_set_band(net, WLAN_HDD_UI_BAND_2_4_GHZ) == 0) {
			phymode = eCSR_DOT11_MODE_11g;
			hdd_dot11mode = eHDD_DOT11_MODE_11g;
			chwidth = WNI_CFG_CHANNEL_BONDING_MODE_DISABLE;
			curr_band = BAND_2G;
		} else {
			sme_set_phy_mode(mac_handle, old_phymode);
			return -EIO;
		}
		break;
	/* UMAC doesn't have option to set MODE_11NA/MODE_11NG as phymode
	 * so setting phymode as eCSR_DOT11_MODE_11n and updating the band
	 * and channel bonding in configuration to reflect MODE_11NA/MODE_11NG
	 */
	case IEEE80211_MODE_11NA_HT20:
		sme_set_phy_mode(mac_handle, eCSR_DOT11_MODE_11n);
		if (hdd_reg_set_band(net, WLAN_HDD_UI_BAND_5_GHZ) == 0) {
			phymode = eCSR_DOT11_MODE_11n;
			hdd_dot11mode = eHDD_DOT11_MODE_11n;
			chwidth = WNI_CFG_CHANNEL_BONDING_MODE_DISABLE;
			curr_band = BAND_5G;
		} else {
			sme_set_phy_mode(mac_handle, old_phymode);
			return -EIO;
		}
		break;
	case IEEE80211_MODE_11NA_HT40:
		sme_set_phy_mode(mac_handle, eCSR_DOT11_MODE_11n);
		if (hdd_reg_set_band(net, WLAN_HDD_UI_BAND_5_GHZ) == 0) {
			phymode = eCSR_DOT11_MODE_11n;
			hdd_dot11mode = eHDD_DOT11_MODE_11n;
			chwidth = WNI_CFG_CHANNEL_BONDING_MODE_ENABLE;
			curr_band = BAND_5G;
		} else {
			sme_set_phy_mode(mac_handle, old_phymode);
			return -EIO;
		}
		break;
	case IEEE80211_MODE_11NG_HT20:
		sme_set_phy_mode(mac_handle, eCSR_DOT11_MODE_11n);
		if (hdd_reg_set_band(net, WLAN_HDD_UI_BAND_2_4_GHZ) == 0) {
			phymode = eCSR_DOT11_MODE_11n;
			hdd_dot11mode = eHDD_DOT11_MODE_11n;
			chwidth = WNI_CFG_CHANNEL_BONDING_MODE_DISABLE;
			curr_band = BAND_2G;
		} else {
			sme_set_phy_mode(mac_handle, old_phymode);
			return -EIO;
		}
		break;
	case IEEE80211_MODE_11NG_HT40:
		sme_set_phy_mode(mac_handle, eCSR_DOT11_MODE_11n);
		if (hdd_reg_set_band(net, WLAN_HDD_UI_BAND_2_4_GHZ) == 0) {
			phymode = eCSR_DOT11_MODE_11n;
			hdd_dot11mode = eHDD_DOT11_MODE_11n;
			chwidth = WNI_CFG_CHANNEL_BONDING_MODE_ENABLE;
			curr_band = BAND_2G;
		} else {
			sme_set_phy_mode(mac_handle, old_phymode);
			return -EIO;
		}
		break;
	case IEEE80211_MODE_11AC_VHT20:
	case IEEE80211_MODE_11AC_VHT40:
	case IEEE80211_MODE_11AC_VHT80:
		sme_set_phy_mode(mac_handle, eCSR_DOT11_MODE_11ac);
		phymode = eCSR_DOT11_MODE_11ac;
		hdd_dot11mode = eHDD_DOT11_MODE_11ac;
		chwidth = WNI_CFG_CHANNEL_BONDING_MODE_ENABLE;
		if (band_5g && band_24) {
			curr_band = BAND_ALL;
			break;
		} else if (band_5g) {
			curr_band = BAND_5G;
			break;
		} else if (new_phymode != IEEE80211_MODE_11AC_VHT80) {
			curr_band = BAND_2G;
			break;
		}
		if (hdd_reg_set_band(net, WLAN_HDD_UI_BAND_AUTO) == 0) {
			curr_band = BAND_ALL;
		} else {
			sme_set_phy_mode(mac_handle, old_phymode);
			return -EIO;
		}
		break;
	case IEEE80211_MODE_2G_AUTO:
		sme_set_phy_mode(mac_handle, eCSR_DOT11_MODE_AUTO);
		if (hdd_reg_set_band(net, WLAN_HDD_UI_BAND_2_4_GHZ) == 0) {
			phymode = eCSR_DOT11_MODE_AUTO;
			hdd_dot11mode = eHDD_DOT11_MODE_AUTO;
			chwidth = WNI_CFG_CHANNEL_BONDING_MODE_ENABLE;
			curr_band = BAND_2G;
		} else {
			sme_set_phy_mode(mac_handle, old_phymode);
			return -EIO;
		}
		break;
	case IEEE80211_MODE_5G_AUTO:
		sme_set_phy_mode(mac_handle, eCSR_DOT11_MODE_AUTO);
		if (hdd_reg_set_band(net, WLAN_HDD_UI_BAND_5_GHZ) == 0) {
			phymode = eCSR_DOT11_MODE_AUTO;
			hdd_dot11mode = eHDD_DOT11_MODE_AUTO;
			chwidth = WNI_CFG_CHANNEL_BONDING_MODE_ENABLE;
			vhtchanwidth = eHT_CHANNEL_WIDTH_80MHZ;
			curr_band = BAND_5G;
		} else {
			sme_set_phy_mode(mac_handle, old_phymode);
			return -EIO;
		}
		break;
	case IEEE80211_MODE_11AGN:
		sme_set_phy_mode(mac_handle, eCSR_DOT11_MODE_11n);
		if (hdd_reg_set_band(net, WLAN_HDD_UI_BAND_AUTO) == 0) {
			phymode = eCSR_DOT11_MODE_11n;
			hdd_dot11mode = eHDD_DOT11_MODE_11n;
			chwidth = WNI_CFG_CHANNEL_BONDING_MODE_ENABLE;
			curr_band = BAND_ALL;
		} else {
			sme_set_phy_mode(mac_handle, old_phymode);
			return -EIO;
		}
		break;
	default:
		return -EIO;
	}

	switch (new_phymode) {
	case IEEE80211_MODE_11AC_VHT20:
		chwidth = WNI_CFG_CHANNEL_BONDING_MODE_DISABLE;
		vhtchanwidth = eHT_CHANNEL_WIDTH_20MHZ;
		break;
	case IEEE80211_MODE_11AC_VHT40:
		vhtchanwidth = eHT_CHANNEL_WIDTH_40MHZ;
		break;
	case IEEE80211_MODE_11AC_VHT80:
		vhtchanwidth = eHT_CHANNEL_WIDTH_80MHZ;
		break;
	default:
		vhtchanwidth = phddctx->config->vhtChannelWidth;
		break;
	}

	if (phymode != -EIO) {
		sme_config = qdf_mem_malloc(sizeof(*sme_config));
		if (!sme_config) {
			hdd_err("Failed to allocate memory for sme_config");
			return -ENOMEM;
		}
		qdf_mem_zero(sme_config, sizeof(*sme_config));
		sme_get_config_param(mac_handle, sme_config);
		sme_config->csrConfig.phyMode = phymode;
#ifdef QCA_HT_2040_COEX
		if (phymode == eCSR_DOT11_MODE_11n &&
		    chwidth == WNI_CFG_CHANNEL_BONDING_MODE_DISABLE) {
			sme_config->csrConfig.obssEnabled = false;
			halStatus = sme_set_ht2040_mode(mac_handle,
							adapter->session_id,
							eHT_CHAN_HT20, false);
			if (halStatus == QDF_STATUS_E_FAILURE) {
				hdd_err("Failed to disable OBSS");
				retval = -EIO;
				goto free;
			}
		} else if (phymode == eCSR_DOT11_MODE_11n &&
			   chwidth == WNI_CFG_CHANNEL_BONDING_MODE_ENABLE) {
			sme_config->csrConfig.obssEnabled = true;
			halStatus = sme_set_ht2040_mode(mac_handle,
							adapter->session_id,
							eHT_CHAN_HT20, true);
			if (halStatus == QDF_STATUS_E_FAILURE) {
				hdd_err("Failed to enable OBSS");
				retval = -EIO;
				goto free;
			}
		}
#endif
		sme_config->csrConfig.eBand = curr_band;
		sme_config->csrConfig.bandCapability = curr_band;
		if (curr_band == BAND_2G)
			sme_config->csrConfig.Is11hSupportEnabled = 0;
		else
			sme_config->csrConfig.Is11hSupportEnabled =
				phddctx->config->Is11hSupportEnabled;
		if (curr_band == BAND_2G)
			sme_config->csrConfig.channelBondingMode24GHz = chwidth;
		else if (curr_band == BAND_5G)
			sme_config->csrConfig.channelBondingMode5GHz = chwidth;
		else {
			sme_config->csrConfig.channelBondingMode24GHz = chwidth;
			sme_config->csrConfig.channelBondingMode5GHz = chwidth;
		}
		sme_config->csrConfig.nVhtChannelWidth = vhtchanwidth;
		sme_update_config(mac_handle, sme_config);

		phddctx->config->dot11Mode = hdd_dot11mode;
		phddctx->config->nChannelBondingMode24GHz =
			sme_config->csrConfig.channelBondingMode24GHz;
		phddctx->config->nChannelBondingMode5GHz =
			sme_config->csrConfig.channelBondingMode5GHz;
		phddctx->config->vhtChannelWidth = vhtchanwidth;
		if (hdd_update_config_cfg(phddctx) == false) {
			hdd_err("could not update config_dat");
			retval = -EIO;
			goto free;
		}

		if (band_5g) {
			struct ieee80211_supported_band *band;

			band = phddctx->wiphy->bands[HDD_NL80211_BAND_5GHZ];
			if (phddctx->config->nChannelBondingMode5GHz)
				band->ht_cap.cap |=
					IEEE80211_HT_CAP_SUP_WIDTH_20_40;
			else
				band->ht_cap.cap &=
					~IEEE80211_HT_CAP_SUP_WIDTH_20_40;
		}

		hdd_debug("New_Phymode= %d ch_bonding=%d band=%d VHT_ch_width=%u",
			phymode, chwidth, curr_band, vhtchanwidth);
	}

free:
	if (sme_config)
		qdf_mem_free(sme_config);
	return retval;
}

static int hdd_validate_pdev_reset(int value)
{
	if ((value < 1) || (value > 5)) {
		hdd_warn(" Invalid value %d: Use any one of the below values\n"
			 "    TX_FLUSH = 1\n"
			 "    WARM_RESET = 2\n"
			 "    COLD_RESET = 3\n"
			 "    WARM_RESET_RESTORE_CAL = 4\n"
			 "    COLD_RESET_RESTORE_CAL = 5", value);

		return -EINVAL;
	}

	return 0;
}

static int hdd_handle_pdev_reset(struct hdd_adapter *adapter, int value)
{
	int ret;

	hdd_debug("%d", value);

	ret = hdd_validate_pdev_reset(value);
	if (ret)
		return ret;

	ret = wma_cli_set_command(adapter->session_id,
				  WMI_PDEV_PARAM_PDEV_RESET,
				  value, PDEV_CMD);

	return ret;
}

static int hdd_we_set_ch_width(struct hdd_adapter *adapter, int ch_width)
{
	int errno;
	struct hdd_context *hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	ePhyChanBondState bonding_state;
	uint32_t bonding_mode;
	tSmeConfigParams *sme_config;
	mac_handle_t mac_handle;

	mac_handle = hdd_ctx->mac_handle;
	if (!mac_handle)
		return -EINVAL;

	/* updating channel bonding only on 5Ghz */
	hdd_debug("WMI_VDEV_PARAM_CHWIDTH val %d", ch_width);

	switch (ch_width) {
	case eHT_CHANNEL_WIDTH_20MHZ:
		bonding_mode = WNI_CFG_CHANNEL_BONDING_MODE_DISABLE;
		break;

	case eHT_CHANNEL_WIDTH_40MHZ:
	case eHT_CHANNEL_WIDTH_80MHZ:
		bonding_state = csr_convert_cb_ini_value_to_phy_cb_state(
			hdd_ctx->config->nChannelBondingMode5GHz);

		if (bonding_state == WNI_CFG_CHANNEL_BONDING_MODE_DISABLE)
			return -EINVAL;

		bonding_mode = hdd_ctx->config->nChannelBondingMode5GHz;
		break;

	default:
		hdd_err("Invalid channel width 0->20 1->40 2->80");
		return -EINVAL;
	}

	sme_config = qdf_mem_malloc(sizeof(*sme_config));
	if (!sme_config) {
		hdd_err("failed to allocate memory for sme_config");
		return -ENOMEM;
	}

	errno = wma_cli_set_command(adapter->session_id, WMI_VDEV_PARAM_CHWIDTH,
				    ch_width, VDEV_CMD);
	if (errno)
		goto free_config;

	sme_get_config_param(mac_handle, sme_config);
	sme_config->csrConfig.channelBondingMode5GHz = bonding_mode;
	sme_update_config(mac_handle, sme_config);

free_config:
	qdf_mem_free(sme_config);

	return errno;
}

static int hdd_we_set_11d_state(struct hdd_context *hdd_ctx, int state_11d)
{
	tSmeConfigParams *sme_config;
	bool enable_11d;
	mac_handle_t mac_handle = hdd_ctx->mac_handle;

	if (!mac_handle)
		return -EINVAL;

	switch (state_11d) {
	case ENABLE_11D:
		enable_11d = true;
		break;
	case DISABLE_11D:
		enable_11d = false;
		break;
	default:
		return -EINVAL;
	}

	sme_config = qdf_mem_malloc(sizeof(*sme_config));
	if (!sme_config) {
		hdd_err("failed to allocate memory for sme_config");
		return -ENOMEM;
	}

	sme_get_config_param(mac_handle, sme_config);
	sme_config->csrConfig.Is11dSupportEnabled = enable_11d;
	sme_update_config(mac_handle, sme_config);

	qdf_mem_free(sme_config);

	hdd_debug("11D state=%d", enable_11d);

	return 0;
}

/**
 * iw_setint_getnone() - Generic "set integer" private ioctl handler
 * @dev: device upon which the ioctl was received
 * @info: ioctl request information
 * @wrqu: ioctl request data
 * @extra: ioctl extra data
 *
 * Return: 0 on success, non-zero on error
 */
static int __iw_setint_getnone(struct net_device *dev,
			       struct iw_request_info *info,
			       union iwreq_data *wrqu, char *extra)
{
	struct hdd_adapter *adapter = WLAN_HDD_GET_PRIV_PTR(dev);
	mac_handle_t mac_handle;
	struct hdd_station_ctx *sta_ctx = WLAN_HDD_GET_STATION_CTX_PTR(adapter);
	struct hdd_context *hdd_ctx;
	int *value = (int *)extra;
	int sub_cmd = value[0];
	int set_value = value[1];
	int ret;
	QDF_STATUS status;

	hdd_enter_dev(dev);

	hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	ret = wlan_hdd_validate_context(hdd_ctx);
	if (0 != ret)
		return ret;

	ret = hdd_check_private_wext_control(hdd_ctx, info);
	if (0 != ret)
		return ret;

	mac_handle = hdd_ctx->mac_handle;
	switch (sub_cmd) {
	case WE_SET_11D_STATE:
		ret = hdd_we_set_11d_state(hdd_ctx, set_value);
		break;

	case WE_SET_POWER:
	{
		if (!mac_handle)
			return -EINVAL;

		switch (set_value) {
		case 1:
			/* Enable PowerSave */
			sme_ps_enable_disable(mac_handle, adapter->session_id,
					      SME_PS_ENABLE);
			break;
		case 2:
			/* Disable PowerSave */
			sme_ps_enable_disable(mac_handle, adapter->session_id,
					      SME_PS_DISABLE);
			break;
		case 3:          /* Enable UASPD */
			sme_ps_uapsd_enable(mac_handle, adapter->session_id);
			break;
		case 4:          /* Disable UASPD */
			sme_ps_uapsd_disable(mac_handle, adapter->session_id);
			break;
		default:
			hdd_err("Invalid arg  %d in WE_SET_POWER IOCTL",
				set_value);
			ret = -EINVAL;
			break;
		}
		break;
	}

	case WE_SET_MAX_ASSOC:
	{
		if (!mac_handle)
			return -EINVAL;

		if ((WNI_CFG_ASSOC_STA_LIMIT_STAMIN > set_value) ||
		    (WNI_CFG_ASSOC_STA_LIMIT_STAMAX < set_value)) {
			ret = -EINVAL;
		} else if (sme_cfg_set_int(mac_handle, WNI_CFG_ASSOC_STA_LIMIT,
					set_value)
			   != QDF_STATUS_SUCCESS) {
			hdd_err("WNI_CFG_ASSOC_STA_LIMIT failed");
			ret = -EIO;
		}
		break;
	}

	case WE_SET_DATA_INACTIVITY_TO:
		if (!mac_handle)
			return -EINVAL;

		if ((set_value < CFG_DATA_INACTIVITY_TIMEOUT_MIN) ||
		    (set_value > CFG_DATA_INACTIVITY_TIMEOUT_MAX) ||
		    (sme_cfg_set_int(mac_handle,
				     WNI_CFG_PS_DATA_INACTIVITY_TIMEOUT,
				     set_value) == QDF_STATUS_E_FAILURE)) {
			hdd_err("WNI_CFG_PS_DATA_INACTIVITY_TIMEOUT failed");
			ret = -EINVAL;
		}
		break;
	case WE_SET_WOW_DATA_INACTIVITY_TO:
		if (!mac_handle) {
			ret = -EINVAL;
			break;
		}

		if ((set_value < CFG_WOW_DATA_INACTIVITY_TIMEOUT_MIN) ||
		    (set_value > CFG_WOW_DATA_INACTIVITY_TIMEOUT_MAX) ||
		    (sme_cfg_set_int(mac_handle,
				     WNI_CFG_PS_WOW_DATA_INACTIVITY_TIMEOUT,
				     set_value) == QDF_STATUS_E_FAILURE)) {
			hdd_err("WNI_CFG_PS_WOW_DATA_INACTIVITY_TIMEOUT fail");
			ret = -EINVAL;
		}
		break;
	case WE_SET_MC_RATE:
	{
		if (!mac_handle)
			return -EINVAL;

		ret = wlan_hdd_set_mc_rate(adapter, set_value);
		break;
	}
	case WE_SET_TX_POWER:
	{
		struct qdf_mac_addr bssid;

		if (!mac_handle)
			return -EINVAL;

		qdf_copy_macaddr(&bssid, &sta_ctx->conn_info.bssId);
		if (sme_set_tx_power
			    (mac_handle, adapter->session_id, bssid,
			    adapter->device_mode,
			    set_value) != QDF_STATUS_SUCCESS) {
			hdd_err("Setting tx power failed");
			ret = -EIO;
			break;
		}
		break;
	}
	case WE_SET_MAX_TX_POWER:
	{
		struct qdf_mac_addr bssid;
		struct qdf_mac_addr selfMac;

		if (!mac_handle)
			return -EINVAL;

		hdd_debug("Setting maximum tx power %d dBm",
		       set_value);
		qdf_copy_macaddr(&bssid, &sta_ctx->conn_info.bssId);
		qdf_copy_macaddr(&selfMac, &sta_ctx->conn_info.bssId);

		if (sme_set_max_tx_power(mac_handle, bssid, selfMac, set_value)
		    != QDF_STATUS_SUCCESS) {
			hdd_err("Setting maximum tx power failed");
			ret = -EIO;
			break;
		}

		break;
	}
	case WE_SET_MAX_TX_POWER_2_4:
	{
		hdd_debug("Setting maximum tx power %d dBm for 2.4 GHz band",
			   set_value);
		if (sme_set_max_tx_power_per_band(BAND_2G, set_value) !=
		    QDF_STATUS_SUCCESS) {
			hdd_err("Setting max tx power failed for 2.4 GHz band");
			ret = -EIO;
			break;
		}

		break;
	}
	case WE_SET_MAX_TX_POWER_5_0:
	{
		hdd_debug("Setting maximum tx power %d dBm for 5.0 GHz band",
			   set_value);
		if (sme_set_max_tx_power_per_band(BAND_5G, set_value) !=
		    QDF_STATUS_SUCCESS) {
			hdd_err("Setting max tx power failed for 5.0 GHz band");
			ret = -EIO;
			break;
		}

		break;
	}

	case WE_SET_TM_LEVEL:
	{
		if (!mac_handle)
			return -EINVAL;

		hdd_debug("Set Thermal Mitigation Level %d", set_value);
		(void)sme_set_thermal_level(mac_handle, set_value);
		break;
	}

	case WE_SET_PHYMODE:
		if (!mac_handle)
			return -EINVAL;

		ret = wlan_hdd_update_phymode(dev, mac_handle, set_value,
					      hdd_ctx);
		break;

	case WE_SET_NSS:
	{
		if (!mac_handle)
			return -EINVAL;

		hdd_debug("Set NSS = %d", set_value);
		if ((set_value > 2) || (set_value <= 0)) {
			hdd_err("NSS greater than 2 not supported");
			ret = -EINVAL;
		} else {
			if (QDF_STATUS_SUCCESS !=
				hdd_update_nss(adapter, set_value))
				ret = -EINVAL;
		}
		break;
	}

	case WE_SET_GTX_HT_MCS:
	{
		hdd_debug("WMI_VDEV_PARAM_GTX_HT_MCS %d", set_value);
		ret = wma_cli_set_command(adapter->session_id,
					  WMI_VDEV_PARAM_GTX_HT_MCS,
					  set_value, GTX_CMD);
		break;
	}

	case WE_SET_GTX_VHT_MCS:
	{
		hdd_debug("WMI_VDEV_PARAM_GTX_VHT_MCS %d",
		       set_value);
		ret = wma_cli_set_command(adapter->session_id,
					  WMI_VDEV_PARAM_GTX_VHT_MCS,
					  set_value, GTX_CMD);
		break;
	}

	case WE_SET_GTX_USRCFG:
	{
		hdd_debug("WMI_VDEV_PARAM_GTX_USR_CFG %d",
		       set_value);
		ret = wma_cli_set_command(adapter->session_id,
					  WMI_VDEV_PARAM_GTX_USR_CFG,
					  set_value, GTX_CMD);
		break;
	}

	case WE_SET_GTX_THRE:
	{
		hdd_debug("WMI_VDEV_PARAM_GTX_THRE %d", set_value);
		ret = wma_cli_set_command(adapter->session_id,
					  WMI_VDEV_PARAM_GTX_THRE,
					  set_value, GTX_CMD);
		break;
	}

	case WE_SET_GTX_MARGIN:
	{
		hdd_debug("WMI_VDEV_PARAM_GTX_MARGIN %d", set_value);
		ret = wma_cli_set_command(adapter->session_id,
					  WMI_VDEV_PARAM_GTX_MARGIN,
					  set_value, GTX_CMD);
		break;
	}

	case WE_SET_GTX_STEP:
	{
		hdd_debug("WMI_VDEV_PARAM_GTX_STEP %d", set_value);
		ret = wma_cli_set_command(adapter->session_id,
					  WMI_VDEV_PARAM_GTX_STEP,
					  set_value, GTX_CMD);
		break;
	}

	case WE_SET_GTX_MINTPC:
	{
		hdd_debug("WMI_VDEV_PARAM_GTX_MINTPC %d", set_value);
		ret = wma_cli_set_command(adapter->session_id,
					  WMI_VDEV_PARAM_GTX_MINTPC,
					  set_value, GTX_CMD);
		break;
	}

	case WE_SET_GTX_BWMASK:
	{
		hdd_debug("WMI_VDEV_PARAM_GTX_BWMASK %d", set_value);
		ret = wma_cli_set_command(adapter->session_id,
					  WMI_VDEV_PARAM_GTX_BW_MASK,
					  set_value, GTX_CMD);
		break;
	}

	case WE_SET_LDPC:
	{
		if (!mac_handle)
			return -EINVAL;

		ret = hdd_set_ldpc(adapter, set_value);
		break;
	}

	case WE_SET_TX_STBC:
	{
		if (!mac_handle)
			return -EINVAL;

		ret = hdd_set_tx_stbc(adapter, set_value);
		break;
	}

	case WE_SET_RX_STBC:
	{
		if (!mac_handle)
			return -EINVAL;

		ret = hdd_set_rx_stbc(adapter, set_value);
		break;
	}

	case WE_SET_SHORT_GI:
	{
		if (!mac_handle)
			return -EINVAL;

		hdd_debug("WMI_VDEV_PARAM_SGI val %d", set_value);
		if (set_value & HDD_AUTO_RATE_SGI)
			ret = sme_set_auto_rate_he_sgi(mac_handle,
						       adapter->session_id,
						       set_value);
		else
			ret = sme_update_ht_config(mac_handle,
					adapter->session_id,
					WNI_CFG_HT_CAP_INFO_SHORT_GI_20MHZ,
					set_value);
		if (ret)
			hdd_err("Failed to set ShortGI value %d", set_value);

		break;
	}

	case WE_SET_RTSCTS:
	{
		uint32_t value;

		if (!mac_handle)
			return -EINVAL;

		hdd_debug("WMI_VDEV_PARAM_ENABLE_RTSCTS val 0x%x", set_value);

		if ((set_value & HDD_RTSCTS_EN_MASK) ==
		    HDD_RTSCTS_ENABLE)
			value =
				(WLAN_HDD_GET_CTX(adapter))->config->
				RTSThreshold;
		else if (((set_value & HDD_RTSCTS_EN_MASK) == 0)
			 || ((set_value & HDD_RTSCTS_EN_MASK) ==
			     HDD_CTS_ENABLE)) {
			value = WNI_CFG_RTS_THRESHOLD_STAMAX;
		} else {
			ret = -EIO;
			break;
		}

		ret = wma_cli_set_command(adapter->session_id,
					  WMI_VDEV_PARAM_ENABLE_RTSCTS,
					  set_value, VDEV_CMD);
		if (!ret) {
			if (sme_cfg_set_int(mac_handle, WNI_CFG_RTS_THRESHOLD,
					    value) != QDF_STATUS_SUCCESS) {
				hdd_err("FAILED TO SET RTSCTS");
				ret = -EIO;
				break;
			}
		}

		break;
	}

	case WE_SET_CHWIDTH:
		ret = hdd_we_set_ch_width(adapter, set_value);
		break;

	case WE_SET_ANI_EN_DIS:
	{
		hdd_debug("WMI_PDEV_PARAM_ANI_ENABLE val %d",
		       set_value);
		ret = wma_cli_set_command(adapter->session_id,
					  WMI_PDEV_PARAM_ANI_ENABLE,
					  set_value, PDEV_CMD);
		break;
	}

	case WE_SET_ANI_POLL_PERIOD:
	{
		hdd_debug("WMI_PDEV_PARAM_ANI_POLL_PERIOD val %d",
		       set_value);
		ret = wma_cli_set_command(adapter->session_id,
					  WMI_PDEV_PARAM_ANI_POLL_PERIOD,
					  set_value, PDEV_CMD);
		break;
	}

	case WE_SET_ANI_LISTEN_PERIOD:
	{
		hdd_debug("WMI_PDEV_PARAM_ANI_LISTEN_PERIOD val %d",
		       set_value);
		ret = wma_cli_set_command(adapter->session_id,
					  WMI_PDEV_PARAM_ANI_LISTEN_PERIOD,
					  set_value, PDEV_CMD);
		break;
	}

	case WE_SET_ANI_OFDM_LEVEL:
	{
		hdd_debug("WMI_PDEV_PARAM_ANI_OFDM_LEVEL val %d",
		       set_value);
		ret = wma_cli_set_command(adapter->session_id,
					  WMI_PDEV_PARAM_ANI_OFDM_LEVEL,
					  set_value, PDEV_CMD);
		break;
	}

	case WE_SET_ANI_CCK_LEVEL:
	{
		hdd_debug("WMI_PDEV_PARAM_ANI_CCK_LEVEL val %d",
		       set_value);
		ret = wma_cli_set_command(adapter->session_id,
					  WMI_PDEV_PARAM_ANI_CCK_LEVEL,
					  set_value, PDEV_CMD);
		break;
	}

	case WE_SET_DYNAMIC_BW:
	{
		hdd_debug("WMI_PDEV_PARAM_DYNAMIC_BW val %d",
		       set_value);
		ret = wma_cli_set_command(adapter->session_id,
					  WMI_PDEV_PARAM_DYNAMIC_BW,
					  set_value, PDEV_CMD);
		break;
	}

	case WE_SET_CTS_CBW:
	{
		hdd_debug("WE_SET_CTS_CBW val %d", set_value);
		ret = wma_cli_set_command(adapter->session_id,
					  WMI_PDEV_PARAM_CTS_CBW,
					  set_value, PDEV_CMD);
		break;
	}

	case WE_SET_11N_RATE:
	{
		uint8_t preamble = 0, nss = 0, rix = 0;

		hdd_debug("WMI_VDEV_PARAM_FIXED_RATE val %d",
		       set_value);

		if (set_value != 0xff) {
			rix = RC_2_RATE_IDX(set_value);
			if (set_value & 0x80) {
				preamble = WMI_RATE_PREAMBLE_HT;
				nss = HT_RC_2_STREAMS(set_value) - 1;
			} else {
				nss = 0;
				rix = RC_2_RATE_IDX(set_value);
				if (set_value & 0x10) {
					preamble =
						WMI_RATE_PREAMBLE_CCK;
					if (rix != 0x3)
						/* Enable Short
						 * preamble always for
						 * CCK except 1mbps
						 */
						rix |= 0x4;
				} else {
					preamble =
						WMI_RATE_PREAMBLE_OFDM;
				}
			}
			set_value = hdd_assemble_rate_code(preamble, nss, rix);
		}
		hdd_debug("WMI_VDEV_PARAM_FIXED_RATE val %d rix %d preamble %x nss %d",
			 set_value, rix, preamble, nss);

		ret = wma_cli_set_command(adapter->session_id,
					  WMI_VDEV_PARAM_FIXED_RATE,
					  set_value, VDEV_CMD);
		break;
	}

	case WE_SET_VHT_RATE:
	{
		uint8_t preamble = 0, nss = 0, rix = 0;

		if (set_value != 0xff) {
			rix = RC_2_RATE_IDX_11AC(set_value);
			preamble = WMI_RATE_PREAMBLE_VHT;
			nss = HT_RC_2_STREAMS_11AC(set_value) - 1;

			set_value = hdd_assemble_rate_code(preamble, nss, rix);
		}
		hdd_debug("WMI_VDEV_PARAM_FIXED_RATE val %d rix %d preamble %x nss %d",
			 set_value, rix, preamble, nss);
		ret = wma_cli_set_command(adapter->session_id,
					  WMI_VDEV_PARAM_FIXED_RATE,
					  set_value, VDEV_CMD);
		break;
	}

	case WE_SET_AMPDU:
	{
		hdd_debug("SET AMPDU val %d", set_value);
		ret = wma_cli_set_command(adapter->session_id,
					  GEN_VDEV_PARAM_AMPDU,
					  set_value, GEN_CMD);
		break;
	}

	case WE_SET_AMSDU:
	{
		hdd_debug("SET AMSDU val %d", set_value);
		if (set_value > 1)
			sme_set_amsdu(mac_handle, true);
		else
			sme_set_amsdu(mac_handle, false);

		ret = wma_cli_set_command(adapter->session_id,
					  GEN_VDEV_PARAM_AMSDU,
					  set_value, GEN_CMD);
		/* Update the stored ini value */
		if (!ret)
			hdd_ctx->config->max_amsdu_num = set_value;
		break;
	}

	case WE_SET_TX_CHAINMASK:
	{
		hdd_debug("WMI_PDEV_PARAM_TX_CHAIN_MASK val %d",
		       set_value);
		ret = wma_cli_set_command(adapter->session_id,
					  WMI_PDEV_PARAM_TX_CHAIN_MASK,
					  set_value, PDEV_CMD);
		ret = hdd_set_antenna_mode(adapter, hdd_ctx, set_value);
		break;
	}

	case WE_SET_RX_CHAINMASK:
	{
		hdd_debug("WMI_PDEV_PARAM_RX_CHAIN_MASK val %d",
		       set_value);
		ret = wma_cli_set_command(adapter->session_id,
					  WMI_PDEV_PARAM_RX_CHAIN_MASK,
					  set_value, PDEV_CMD);
		ret = hdd_set_antenna_mode(adapter, hdd_ctx, set_value);
		break;
	}

	case WE_SET_TXPOW_2G:
	{
		hdd_debug("WMI_PDEV_PARAM_TXPOWER_LIMIT2G val %d",
		       set_value);
		ret = wma_cli_set_command(adapter->session_id,
					  WMI_PDEV_PARAM_TXPOWER_LIMIT2G,
					  set_value, PDEV_CMD);
		break;
	}

	case WE_SET_TXPOW_5G:
	{
		hdd_debug("WMI_PDEV_PARAM_TXPOWER_LIMIT5G val %d",
		       set_value);
		ret = wma_cli_set_command(adapter->session_id,
					  WMI_PDEV_PARAM_TXPOWER_LIMIT5G,
					  set_value, PDEV_CMD);
		break;
	}

	/* Firmware debug log */
	case WE_DBGLOG_LOG_LEVEL:
	{
		hdd_debug("WE_DBGLOG_LOG_LEVEL val %d", set_value);
		hdd_ctx->fw_log_settings.dl_loglevel = set_value;
		ret = wma_cli_set_command(adapter->session_id,
					  WMI_DBGLOG_LOG_LEVEL,
					  set_value, DBG_CMD);
		break;
	}

	case WE_DBGLOG_VAP_ENABLE:
	{
		hdd_debug("WE_DBGLOG_VAP_ENABLE val %d", set_value);
		ret = wma_cli_set_command(adapter->session_id,
					  WMI_DBGLOG_VAP_ENABLE,
					  set_value, DBG_CMD);
		break;
	}

	case WE_DBGLOG_VAP_DISABLE:
	{
		hdd_debug("WE_DBGLOG_VAP_DISABLE val %d", set_value);
		ret = wma_cli_set_command(adapter->session_id,
					  WMI_DBGLOG_VAP_DISABLE,
					  set_value, DBG_CMD);
		break;
	}

	case WE_DBGLOG_MODULE_ENABLE:
	{
		hdd_debug("WE_DBGLOG_MODULE_ENABLE val %d",
		       set_value);
		hdd_ctx->fw_log_settings.enable = set_value;
		ret = wma_cli_set_command(adapter->session_id,
					  WMI_DBGLOG_MODULE_ENABLE,
					  set_value, DBG_CMD);
		break;
	}

	case WE_DBGLOG_MODULE_DISABLE:
	{
		hdd_debug("WE_DBGLOG_MODULE_DISABLE val %d",
		       set_value);
		hdd_ctx->fw_log_settings.enable = set_value;
		ret = wma_cli_set_command(adapter->session_id,
					  WMI_DBGLOG_MODULE_DISABLE,
					  set_value, DBG_CMD);
		break;
	}
	case WE_DBGLOG_MOD_LOG_LEVEL:
	{
		hdd_debug("WE_DBGLOG_MOD_LOG_LEVEL val %d",
		       set_value);

		if (hdd_ctx->fw_log_settings.index >= MAX_MOD_LOGLEVEL)
			hdd_ctx->fw_log_settings.index = 0;

		hdd_ctx->fw_log_settings.
		dl_mod_loglevel[hdd_ctx->fw_log_settings.index] =
			set_value;
		hdd_ctx->fw_log_settings.index++;

		ret = wma_cli_set_command(adapter->session_id,
					  WMI_DBGLOG_MOD_LOG_LEVEL,
					  set_value, DBG_CMD);
		break;
	}

	case WE_DBGLOG_TYPE:
	{
		hdd_debug("WE_DBGLOG_TYPE val %d", set_value);
		hdd_ctx->fw_log_settings.dl_type = set_value;
		ret = wma_cli_set_command(adapter->session_id,
					  WMI_DBGLOG_TYPE,
					  set_value, DBG_CMD);
		break;
	}
	case WE_DBGLOG_REPORT_ENABLE:
	{
		hdd_debug("WE_DBGLOG_REPORT_ENABLE val %d",
		       set_value);
		hdd_ctx->fw_log_settings.dl_report = set_value;
		ret = wma_cli_set_command(adapter->session_id,
					  WMI_DBGLOG_REPORT_ENABLE,
					  set_value, DBG_CMD);
		break;
	}

	case WE_SET_TXRX_FWSTATS:
	{
		hdd_debug("WE_SET_TXRX_FWSTATS val %d", set_value);
		ret = wma_cli_set_command(adapter->session_id,
					  WMA_VDEV_TXRX_FWSTATS_ENABLE_CMDID,
					  set_value, VDEV_CMD);
		break;
	}

	case WE_TXRX_FWSTATS_RESET:
	{
		hdd_debug("WE_TXRX_FWSTATS_RESET val %d", set_value);
		ret = wma_cli_set_command(adapter->session_id,
					  WMA_VDEV_TXRX_FWSTATS_RESET_CMDID,
					  set_value, VDEV_CMD);
		break;
	}

	case WE_DUMP_STATS:
	{
		hdd_debug("WE_DUMP_STATS val %d", set_value);
		ret = hdd_wlan_dump_stats(adapter, set_value);
		break;
	}

	case WE_CLEAR_STATS:
	{
		hdd_debug("WE_CLEAR_STATS val %d", set_value);
		switch (set_value) {
		case CDP_HDD_STATS:
			memset(&adapter->stats, 0, sizeof(adapter->stats));
			memset(&adapter->hdd_stats, 0,
					sizeof(adapter->hdd_stats));
			break;
		case CDP_TXRX_HIST_STATS:
			wlan_hdd_clear_tx_rx_histogram(hdd_ctx);
			break;
		case CDP_HDD_NETIF_OPER_HISTORY:
			wlan_hdd_clear_netif_queue_history(hdd_ctx);
			break;
		case CDP_HIF_STATS:
			hdd_clear_hif_stats();
			break;
		case CDP_NAPI_STATS:
			hdd_clear_napi_stats();
			break;
		default:
			cdp_clear_stats(cds_get_context(QDF_MODULE_ID_SOC),
						set_value);
		}
		break;
	}

	case WE_PPS_PAID_MATCH:
	{
		if (adapter->device_mode != QDF_STA_MODE) {
			ret = -EINVAL;
			break;
		}

		hdd_debug("WMI_VDEV_PPS_PAID_MATCH val %d ",
		       set_value);
		ret = wma_cli_set_command(adapter->session_id,
					  WMI_VDEV_PPS_PAID_MATCH,
					  set_value, PPS_CMD);
		break;
	}

	case WE_PPS_GID_MATCH:
	{
		if (adapter->device_mode != QDF_STA_MODE) {
			ret = -EINVAL;
			break;
		}
		hdd_debug("WMI_VDEV_PPS_GID_MATCH val %d ",
		       set_value);
		ret = wma_cli_set_command(adapter->session_id,
					  WMI_VDEV_PPS_GID_MATCH,
					  set_value, PPS_CMD);
		break;
	}

	case WE_PPS_EARLY_TIM_CLEAR:
	{
		if (adapter->device_mode != QDF_STA_MODE) {
			ret = -EINVAL;
			break;
		}
		hdd_debug(" WMI_VDEV_PPS_EARLY_TIM_CLEAR val %d ",
		       set_value);
		ret = wma_cli_set_command(adapter->session_id,
					  WMI_VDEV_PPS_EARLY_TIM_CLEAR,
					  set_value, PPS_CMD);
		break;
	}

	case WE_PPS_EARLY_DTIM_CLEAR:
	{
		if (adapter->device_mode != QDF_STA_MODE) {
			ret = -EINVAL;
			break;
		}
		hdd_debug("WMI_VDEV_PPS_EARLY_DTIM_CLEAR val %d",
		       set_value);
		ret = wma_cli_set_command(adapter->session_id,
					  WMI_VDEV_PPS_EARLY_DTIM_CLEAR,
					  set_value, PPS_CMD);
		break;
	}

	case WE_PPS_EOF_PAD_DELIM:
	{
		if (adapter->device_mode != QDF_STA_MODE) {
			ret = -EINVAL;
			break;
		}
		hdd_debug("WMI_VDEV_PPS_EOF_PAD_DELIM val %d ",
		       set_value);
		ret = wma_cli_set_command(adapter->session_id,
					  WMI_VDEV_PPS_EOF_PAD_DELIM,
					  set_value, PPS_CMD);
		break;
	}

	case WE_PPS_MACADDR_MISMATCH:
	{
		if (adapter->device_mode != QDF_STA_MODE) {
			ret = -EINVAL;
			break;
		}
		hdd_debug("WMI_VDEV_PPS_MACADDR_MISMATCH val %d ",
		       set_value);
		ret = wma_cli_set_command(adapter->session_id,
					  WMI_VDEV_PPS_MACADDR_MISMATCH,
					  set_value, PPS_CMD);
		break;
	}

	case WE_PPS_DELIM_CRC_FAIL:
	{
		if (adapter->device_mode != QDF_STA_MODE) {
			ret = -EINVAL;
			break;
		}
		hdd_debug("WMI_VDEV_PPS_DELIM_CRC_FAIL val %d ",
		       set_value);
		ret = wma_cli_set_command(adapter->session_id,
					  WMI_VDEV_PPS_DELIM_CRC_FAIL,
					  set_value, PPS_CMD);
		break;
	}

	case WE_PPS_GID_NSTS_ZERO:
	{
		if (adapter->device_mode != QDF_STA_MODE) {
			ret = -EINVAL;
			break;
		}
		hdd_debug("WMI_VDEV_PPS_GID_NSTS_ZERO val %d ",
		       set_value);
		ret = wma_cli_set_command(adapter->session_id,
					  WMI_VDEV_PPS_GID_NSTS_ZERO,
					  set_value, PPS_CMD);
		break;
	}

	case WE_PPS_RSSI_CHECK:
	{
		if (adapter->device_mode != QDF_STA_MODE) {
			ret = -EINVAL;
			break;
		}
		hdd_debug("WMI_VDEV_PPS_RSSI_CHECK val %d ",
		       set_value);
		ret = wma_cli_set_command(adapter->session_id,
					  WMI_VDEV_PPS_RSSI_CHECK,
					  set_value, PPS_CMD);
		break;
	}

	case WE_PPS_5G_EBT:
	{
		if (adapter->device_mode != QDF_STA_MODE) {
			ret = -EINVAL;
			break;
		}

		hdd_debug("WMI_VDEV_PPS_5G_EBT val %d", set_value);
		ret = wma_cli_set_command(adapter->session_id,
					  WMI_VDEV_PPS_5G_EBT,
					  set_value, PPS_CMD);
		break;
	}

	case WE_SET_HTSMPS:
	{
		hdd_debug("WE_SET_HTSMPS val %d", set_value);
		ret = wma_cli_set_command(adapter->session_id,
					  WMI_STA_SMPS_FORCE_MODE_CMDID,
					  set_value, VDEV_CMD);
		break;
	}

	case WE_SET_QPOWER_MAX_PSPOLL_COUNT:
	{
		hdd_debug("WE_SET_QPOWER_MAX_PSPOLL_COUNT val %d",
		       set_value);
		ret = wma_cli_set_command(adapter->session_id,
					  WMI_STA_PS_PARAM_QPOWER_PSPOLL_COUNT,
					  set_value, QPOWER_CMD);
		break;
	}

	case WE_SET_QPOWER_MAX_TX_BEFORE_WAKE:
	{
		hdd_debug("WE_SET_QPOWER_MAX_TX_BEFORE_WAKE val %d",
		       set_value);
		ret = wma_cli_set_command(
				adapter->session_id,
				WMI_STA_PS_PARAM_QPOWER_MAX_TX_BEFORE_WAKE,
				set_value, QPOWER_CMD);
		break;
	}

	case WE_SET_QPOWER_SPEC_PSPOLL_WAKE_INTERVAL:
	{
		hdd_debug("WE_SET_QPOWER_SPEC_PSPOLL_WAKE_INTERVAL val %d",
		       set_value);
		ret = wma_cli_set_command(
			adapter->session_id,
			WMI_STA_PS_PARAM_QPOWER_SPEC_PSPOLL_WAKE_INTERVAL,
			set_value, QPOWER_CMD);
		break;
	}

	case WE_SET_QPOWER_SPEC_MAX_SPEC_NODATA_PSPOLL:
	{
		hdd_debug("WE_SET_QPOWER_SPEC_MAX_SPEC_NODATA_PSPOLL val %d",
		       set_value);
		ret = wma_cli_set_command(
			adapter->session_id,
			WMI_STA_PS_PARAM_QPOWER_SPEC_MAX_SPEC_NODATA_PSPOLL,
			set_value, QPOWER_CMD);
		break;
	}

	case WE_MCC_CONFIG_LATENCY:
	{
		wlan_hdd_set_mcc_latency(adapter, set_value);
		break;
	}

	case WE_MCC_CONFIG_QUOTA:
	{
		hdd_debug("iwpriv cmd to set MCC quota with val %dms",
				set_value);
		ret = wlan_hdd_set_mcc_p2p_quota(adapter,
			set_value);
		break;
	}
	case WE_SET_DEBUG_LOG:
	{
		struct hdd_context *hdd_ctx = WLAN_HDD_GET_CTX(adapter);

		if (!mac_handle)
			return -EINVAL;


		hdd_ctx->config->gEnableDebugLog = set_value;
		sme_update_connect_debug(mac_handle, set_value);
		break;
	}
	case WE_SET_EARLY_RX_ADJUST_ENABLE:
	{
		hdd_debug("SET early_rx enable val %d", set_value);
		if ((set_value == 0) || (set_value == 1))
			ret = wma_cli_set_command(
					adapter->session_id,
					WMI_VDEV_PARAM_EARLY_RX_ADJUST_ENABLE,
					set_value, VDEV_CMD);
		else
			ret = -EINVAL;
		break;
	}
	case WE_SET_EARLY_RX_TGT_BMISS_NUM:
	{
		hdd_debug("SET early_rx bmiss val %d", set_value);
		ret = wma_cli_set_command(adapter->session_id,
					  WMI_VDEV_PARAM_EARLY_RX_TGT_BMISS_NUM,
					  set_value, VDEV_CMD);
		break;
	}
	case WE_SET_EARLY_RX_BMISS_SAMPLE_CYCLE:
	{
		hdd_debug("SET early_rx bmiss sample cycle %d",
		       set_value);
		ret = wma_cli_set_command(
				adapter->session_id,
				WMI_VDEV_PARAM_EARLY_RX_BMISS_SAMPLE_CYCLE,
				set_value, VDEV_CMD);
		break;
	}
	case WE_SET_EARLY_RX_SLOP_STEP:
	{
		hdd_debug("SET early_rx bmiss slop step val %d",
		       set_value);
		ret = wma_cli_set_command(adapter->session_id,
					  WMI_VDEV_PARAM_EARLY_RX_SLOP_STEP,
					  set_value, VDEV_CMD);
		break;
	}
	case WE_SET_EARLY_RX_INIT_SLOP:
	{
		hdd_debug("SET early_rx init slop step val %d",
		       set_value);
		ret = wma_cli_set_command(adapter->session_id,
					  WMI_VDEV_PARAM_EARLY_RX_INIT_SLOP,
					  set_value, VDEV_CMD);
		break;
	}
	case WE_SET_EARLY_RX_ADJUST_PAUSE:
	{
		hdd_debug("SET early_rx adjust pause %d", set_value);
		if ((set_value == 0) || (set_value == 1))
			ret = wma_cli_set_command(
					adapter->session_id,
					WMI_VDEV_PARAM_EARLY_RX_ADJUST_PAUSE,
					set_value, VDEV_CMD);
		else
			ret = -EINVAL;
		break;
	}
	case WE_SET_EARLY_RX_DRIFT_SAMPLE:
	{
		hdd_debug("SET early_rx drift sample %d", set_value);
		ret = wma_cli_set_command(adapter->session_id,
					  WMI_VDEV_PARAM_EARLY_RX_DRIFT_SAMPLE,
					  set_value, VDEV_CMD);
		break;
	}
	case WE_SET_SCAN_DISABLE:
	{
		if (!mac_handle)
			return -EINVAL;

		hdd_debug("SET SCAN DISABLE %d", set_value);
		sme_set_scan_disable(mac_handle, set_value);
		break;
	}
	case WE_START_FW_PROFILE:
	{
		hdd_debug("WE_START_FW_PROFILE %d", set_value);
		ret = wma_cli_set_command(adapter->session_id,
					WMI_WLAN_PROFILE_TRIGGER_CMDID,
					set_value, DBG_CMD);
		break;
	}
	case WE_SET_CHANNEL:
	{
		hdd_debug("Set Channel %d Session ID %d mode %d", set_value,
				  adapter->session_id, adapter->device_mode);
		if (!mac_handle)
			return -EINVAL;


		if ((QDF_STA_MODE == adapter->device_mode) ||
		    (QDF_P2P_CLIENT_MODE == adapter->device_mode)) {

			status = sme_ext_change_channel(mac_handle,
							set_value,
							adapter->session_id);
			if (status != QDF_STATUS_SUCCESS) {
				hdd_err("Error in change channel status %d",
					status);
				ret = -EINVAL;
			}
		} else {
			hdd_err("change channel not supported for device mode %d",
				adapter->device_mode);
			ret = -EINVAL;
		}
		break;
	}
	case WE_SET_CONC_SYSTEM_PREF:
	{
		hdd_debug("New preference: %d", set_value);
		if (!((set_value >= CFG_CONC_SYSTEM_PREF_MIN) &&
				(set_value <= CFG_CONC_SYSTEM_PREF_MAX))) {
			hdd_err("Invalid system preference %d", set_value);
			ret = -EINVAL;
			break;
		}

		/* hdd_ctx, hdd_ctx->config are already checked for null */
		hdd_ctx->config->conc_system_pref = set_value;
		break;
	}
	case WE_SET_11AX_RATE:
		ret = hdd_set_11ax_rate(adapter, set_value, NULL);
		break;
	case WE_SET_DCM:
		hdd_debug("Set WMI_VDEV_PARAM_HE_DCM: %d", set_value);
		ret = wma_cli_set_command(adapter->session_id,
					  WMI_VDEV_PARAM_HE_DCM, set_value,
					  VDEV_CMD);
		break;
	case WE_SET_RANGE_EXT:
		hdd_debug("Set WMI_VDEV_PARAM_HE_RANGE_EXT: %d", set_value);
		ret = wma_cli_set_command(adapter->session_id,
					  WMI_VDEV_PARAM_HE_RANGE_EXT,
					  set_value, VDEV_CMD);
		break;
	case WE_SET_PDEV_RESET:
		ret = hdd_handle_pdev_reset(adapter, set_value);
		break;
	case WE_SET_MODULATED_DTIM:
		if ((set_value < CFG_ENABLE_MODULATED_DTIM_MIN) ||
				(set_value > CFG_ENABLE_MODULATED_DTIM_MAX)) {
			hdd_err("Invalid gEnableModuleDTIM value %d",
				set_value);
			return -EINVAL;
		}

		hdd_ctx->config->enableModulatedDTIM = set_value;
		break;
	default:
		hdd_debug("Invalid sub command %d", sub_cmd);
		ret = -EINVAL;
		break;
	}

	hdd_exit();

	return ret;
}

static int iw_setint_getnone(struct net_device *dev,
			     struct iw_request_info *info,
			     union iwreq_data *wrqu,
			     char *extra)
{
	int ret;

	cds_ssr_protect(__func__);
	ret = __iw_setint_getnone(dev, info, wrqu, extra);
	cds_ssr_unprotect(__func__);

	return ret;
}

/**
 * __iw_setnone_get_threeint() - return three value to up layer.
 *
 * @dev: pointer of net_device of this wireless card
 * @info: meta data about Request sent
 * @wrqu: include request info
 * @extra: buf used for in/Output
 *
 * Return: execute result
 */
static int __iw_setnone_get_threeint(struct net_device *dev,
					struct iw_request_info *info,
					union iwreq_data *wrqu, char *extra)
{
	int ret = 0; /* success */
	uint32_t *value = (int *)extra;
	struct hdd_adapter *adapter = WLAN_HDD_GET_PRIV_PTR(dev);
	struct hdd_context *hdd_ctx = WLAN_HDD_GET_CTX(adapter);

	hdd_enter_dev(dev);
	ret = wlan_hdd_validate_context(hdd_ctx);
	if (0 != ret)
		return ret;

	ret = hdd_check_private_wext_control(hdd_ctx, info);
	if (0 != ret)
		return ret;

	hdd_debug("param = %d", value[0]);
	switch (value[0]) {
	case WE_GET_TSF:
		ret = hdd_indicate_tsf(adapter, value, 3);
		break;
	default:
		hdd_err("Invalid IOCTL get_value command %d", value[0]);
		break;
	}
	return ret;
}

/**
 * iw_setnone_get_threeint() - return three value to up layer.
 *
 * @dev: pointer of net_device of this wireless card
 * @info: meta data about Request sent
 * @wrqu: include request info
 * @extra: buf used for in/Output
 *
 * Return: execute result
 */
static int iw_setnone_get_threeint(struct net_device *dev,
					struct iw_request_info *info,
					union iwreq_data *wrqu, char *extra)
{
	int ret;

	cds_ssr_protect(__func__);
	ret = __iw_setnone_get_threeint(dev, info, wrqu, extra);
	cds_ssr_unprotect(__func__);

	return ret;
}

#ifdef WLAN_UNIT_TEST
static int hdd_we_unit_test(struct hdd_context *hdd_ctx, const char *component)
{
	uint32_t errors = 0;
	bool all = !component || !component[0];

	if (all)
		hdd_info("Starting unit tests for all components");
	else
		hdd_info("Starting unit tests for component '%s'", component);

	if (all || qdf_str_eq(component, "dsc"))
		errors += dsc_unit_test();

	/* add future tests here */

	if (errors) {
		hdd_err("Unit tests failed with %u errors", errors);
		return -EPERM;
	}

	hdd_info("Unit tests passed successfully");

	return 0;
}
#else
static int hdd_we_unit_test(struct hdd_context *hdd_ctx, const char *component)
{
	return -EOPNOTSUPP;
}
#endif /* WLAN_UNIT_TEST */

/**
 * iw_setchar_getnone() - Generic "set string" private ioctl handler
 * @dev: device upon which the ioctl was received
 * @info: ioctl request information
 * @wrqu: ioctl request data
 * @extra: ioctl extra data
 *
 * Return: 0 on success, non-zero on error
 */
static int __iw_setchar_getnone(struct net_device *dev,
				struct iw_request_info *info,
				union iwreq_data *wrqu, char *extra)
{
	QDF_STATUS status;
	int sub_cmd;
	int ret;
	char *str_arg = NULL;
	struct hdd_adapter *adapter = (netdev_priv(dev));
	struct hdd_context *hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	struct hdd_config *pConfig = hdd_ctx->config;
	struct iw_point s_priv_data;

	hdd_enter_dev(dev);

	if (!capable(CAP_NET_ADMIN)) {
		hdd_err("permission check failed");
		return -EPERM;
	}

	ret = wlan_hdd_validate_context(hdd_ctx);
	if (0 != ret)
		return ret;

	ret = hdd_check_private_wext_control(hdd_ctx, info);
	if (0 != ret)
		return ret;

	/* helper function to get iwreq_data with compat handling. */
	if (hdd_priv_get_data(&s_priv_data, wrqu))
		return -EINVAL;

	/* make sure all params are correctly passed to function */
	if ((NULL == s_priv_data.pointer) || (0 == s_priv_data.length))
		return -EINVAL;

	sub_cmd = s_priv_data.flags;

	/* ODD number is used for set, copy data using copy_from_user */
	str_arg = mem_alloc_copy_from_user_helper(s_priv_data.pointer,
						  s_priv_data.length);
	if (!str_arg) {
		hdd_err("mem_alloc_copy_from_user_helper fail");
		return -ENOMEM;
	}

	hdd_debug("Received length: %d data: %s",
			s_priv_data.length, str_arg);

	switch (sub_cmd) {
	case WE_WOWL_ADD_PTRN:
		hdd_debug("ADD_PTRN");
		hdd_add_wowl_ptrn(adapter, str_arg);
		break;
	case WE_WOWL_DEL_PTRN:
		hdd_debug("DEL_PTRN");
		hdd_del_wowl_ptrn(adapter, str_arg);
		break;
	case WE_NEIGHBOR_REPORT_REQUEST:
	{
		tRrmNeighborReq neighborReq;
		tRrmNeighborRspCallbackInfo callbackInfo;

		if (pConfig->fRrmEnable) {
			neighborReq.neighbor_report_offload = false;
			neighborReq.no_ssid =
				(s_priv_data.length - 1) ? false : true;
			hdd_debug("Neighbor Request ssid present %d",
				  neighborReq.no_ssid);
			if (!neighborReq.no_ssid) {
				neighborReq.ssid.length =
					(s_priv_data.length - 1) >
					32 ? 32 : (s_priv_data.length - 1);
				qdf_mem_copy(neighborReq.ssid.ssId,
					     str_arg,
					     neighborReq.ssid.length);
			}

			/*
			 * If 11k offload is supported by FW and enabled
			 * in the ini, set the offload to true
			 */
			if (hdd_ctx->config->is_11k_offload_supported &&
			    (hdd_ctx->config->offload_11k_enable_bitmask &
			    OFFLOAD_11K_BITMASK_NEIGHBOR_REPORT_REQUEST)) {
				hdd_debug("Neighbor report offloaded to FW");
				neighborReq.neighbor_report_offload = true;
			}

			callbackInfo.neighborRspCallback = NULL;
			callbackInfo.neighborRspCallbackContext = NULL;
			callbackInfo.timeout = 5000; /* 5 seconds */
			sme_neighbor_report_request(
					hdd_ctx->mac_handle,
					adapter->session_id,
					&neighborReq,
					&callbackInfo);
		} else {
			hdd_err("Ignoring neighbor request as RRM not enabled");
			ret = -EINVAL;
		}
	}
	break;
	case WE_SET_AP_WPS_IE:
		hdd_debug("Received WE_SET_AP_WPS_IE, won't process");
		break;
	case WE_SET_CONFIG:
		status = hdd_execute_global_config_command(hdd_ctx, str_arg);
		if (QDF_IS_STATUS_ERROR(status))
			ret = -EINVAL;

		break;
	case WE_UNIT_TEST:
		ret = hdd_we_unit_test(hdd_ctx, str_arg);
		break;
	default:
	{
		hdd_err("Invalid sub command %d", sub_cmd);
		ret = -EINVAL;
		break;
	}
	}

	qdf_mem_free(str_arg);
	hdd_exit();

	return ret;
}

static int iw_setchar_getnone(struct net_device *dev,
			      struct iw_request_info *info,
			      union iwreq_data *wrqu, char *extra)
{
	int ret;

	cds_ssr_protect(__func__);
	ret = __iw_setchar_getnone(dev, info, wrqu, extra);
	cds_ssr_unprotect(__func__);

	return ret;
}

/**
 * iw_setnone_getint() - Generic "get integer" private ioctl handler
 * @dev: device upon which the ioctl was received
 * @info: ioctl request information
 * @wrqu: ioctl request data
 * @extra: ioctl extra data
 *
 * Return: 0 on success, non-zero on error
 */
static int __iw_setnone_getint(struct net_device *dev,
			       struct iw_request_info *info,
			       union iwreq_data *wrqu, char *extra)
{
	struct hdd_adapter *adapter = WLAN_HDD_GET_PRIV_PTR(dev);
	mac_handle_t mac_handle;
	int *value = (int *)extra;
	int ret;
	tSmeConfigParams *sme_config;
	struct hdd_context *hdd_ctx;

	hdd_enter_dev(dev);

	hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	ret = wlan_hdd_validate_context(hdd_ctx);
	if (0 != ret)
		return ret;

	ret = hdd_check_private_wext_control(hdd_ctx, info);
	if (0 != ret)
		return ret;

	sme_config = qdf_mem_malloc(sizeof(*sme_config));
	if (!sme_config) {
		hdd_err("failed to allocate memory for sme_config");
		return -ENOMEM;
	}

	mac_handle = hdd_ctx->mac_handle;
	switch (value[0]) {
	case WE_GET_11D_STATE:
	{
		sme_get_config_param(mac_handle, sme_config);

		*value = sme_config->csrConfig.Is11dSupportEnabled;

		hdd_debug("11D state=%d!!", *value);

		break;
	}

	case WE_GET_WLAN_DBG:
	{
		qdf_trace_display();
		*value = 0;
		break;
	}
	case WE_GET_MAX_ASSOC:
	{
		if (sme_cfg_get_int(mac_handle, WNI_CFG_ASSOC_STA_LIMIT,
				    (uint32_t *) value) != QDF_STATUS_SUCCESS) {
			hdd_err("WNI_CFG_ASSOC_STA_LIMIT failed");
			ret = -EIO;
		}
		break;
	}

	case WE_GET_CONCURRENCY_MODE:
	{
		*value = policy_mgr_get_concurrency_mode(hdd_ctx->hdd_psoc);

		hdd_debug("concurrency mode=%d", *value);
		break;
	}

	case WE_GET_NSS:
	{
		sme_get_config_param(mac_handle, sme_config);
		*value = (sme_config->csrConfig.enable2x2 == 0) ? 1 : 2;
		if (policy_mgr_is_current_hwmode_dbs(hdd_ctx->hdd_psoc))
			*value = *value - 1;
		hdd_debug("GET_NSS: Current NSS:%d", *value);
		break;
	}

	case WE_GET_GTX_HT_MCS:
	{
		hdd_debug("GET WMI_VDEV_PARAM_GTX_HT_MCS");
		*value = wma_cli_get_command(adapter->session_id,
					     WMI_VDEV_PARAM_GTX_HT_MCS,
					     GTX_CMD);
		break;
	}

	case WE_GET_GTX_VHT_MCS:
	{
		hdd_debug("GET WMI_VDEV_PARAM_GTX_VHT_MCS");
		*value = wma_cli_get_command(adapter->session_id,
					     WMI_VDEV_PARAM_GTX_VHT_MCS,
					     GTX_CMD);
		break;
	}

	case WE_GET_GTX_USRCFG:
	{
		hdd_debug("GET WMI_VDEV_PARAM_GTX_USR_CFG");
		*value = wma_cli_get_command(adapter->session_id,
					     WMI_VDEV_PARAM_GTX_USR_CFG,
					     GTX_CMD);
		break;
	}

	case WE_GET_GTX_THRE:
	{
		hdd_debug("GET WMI_VDEV_PARAM_GTX_THRE");
		*value = wma_cli_get_command(adapter->session_id,
					     WMI_VDEV_PARAM_GTX_THRE,
					     GTX_CMD);
		break;
	}

	case WE_GET_GTX_MARGIN:
	{
		hdd_debug("GET WMI_VDEV_PARAM_GTX_MARGIN");
		*value = wma_cli_get_command(adapter->session_id,
					     WMI_VDEV_PARAM_GTX_MARGIN,
					     GTX_CMD);
		break;
	}

	case WE_GET_GTX_STEP:
	{
		hdd_debug("GET WMI_VDEV_PARAM_GTX_STEP");
		*value = wma_cli_get_command(adapter->session_id,
					     WMI_VDEV_PARAM_GTX_STEP,
					     GTX_CMD);
		break;
	}

	case WE_GET_GTX_MINTPC:
	{
		hdd_debug("GET WMI_VDEV_PARAM_GTX_MINTPC");
		*value = wma_cli_get_command(adapter->session_id,
					     WMI_VDEV_PARAM_GTX_MINTPC,
					     GTX_CMD);
		break;
	}

	case WE_GET_GTX_BWMASK:
	{
		hdd_debug("GET WMI_VDEV_PARAM_GTX_BW_MASK");
		*value = wma_cli_get_command(adapter->session_id,
					     WMI_VDEV_PARAM_GTX_BW_MASK,
					     GTX_CMD);
		break;
	}

	case WE_GET_LDPC:
	{
		ret = hdd_get_ldpc(adapter, value);
		break;
	}

	case WE_GET_TX_STBC:
	{
		ret = hdd_get_tx_stbc(adapter, value);
		break;
	}

	case WE_GET_RX_STBC:
	{
		ret = hdd_get_rx_stbc(adapter, value);
		break;
	}

	case WE_GET_SHORT_GI:
	{
		hdd_debug("GET WMI_VDEV_PARAM_SGI");
		*value = wma_cli_get_command(adapter->session_id,
					     WMI_VDEV_PARAM_SGI,
					     VDEV_CMD);
		break;
	}

	case WE_GET_RTSCTS:
	{
		hdd_debug("GET WMI_VDEV_PARAM_ENABLE_RTSCTS");
		*value = wma_cli_get_command(adapter->session_id,
					     WMI_VDEV_PARAM_ENABLE_RTSCTS,
					     VDEV_CMD);
		break;
	}

	case WE_GET_CHWIDTH:
	{
		hdd_debug("GET WMI_VDEV_PARAM_CHWIDTH");
		*value = wma_cli_get_command(adapter->session_id,
					     WMI_VDEV_PARAM_CHWIDTH,
					     VDEV_CMD);
		break;
	}

	case WE_GET_ANI_EN_DIS:
	{
		hdd_debug("GET WMI_PDEV_PARAM_ANI_ENABLE");
		*value = wma_cli_get_command(adapter->session_id,
					     WMI_PDEV_PARAM_ANI_ENABLE,
					     PDEV_CMD);
		break;
	}

	case WE_GET_ANI_POLL_PERIOD:
	{
		hdd_debug("GET WMI_PDEV_PARAM_ANI_POLL_PERIOD");
		*value = wma_cli_get_command(adapter->session_id,
					     WMI_PDEV_PARAM_ANI_POLL_PERIOD,
					     PDEV_CMD);
		break;
	}

	case WE_GET_ANI_LISTEN_PERIOD:
	{
		hdd_debug("GET WMI_PDEV_PARAM_ANI_LISTEN_PERIOD");
		*value = wma_cli_get_command(adapter->session_id,
					     WMI_PDEV_PARAM_ANI_LISTEN_PERIOD,
					     PDEV_CMD);
		break;
	}

	case WE_GET_ANI_OFDM_LEVEL:
	{
		hdd_debug("GET WMI_PDEV_PARAM_ANI_OFDM_LEVEL");
		*value = wma_cli_get_command(adapter->session_id,
					     WMI_PDEV_PARAM_ANI_OFDM_LEVEL,
					     PDEV_CMD);
		break;
	}

	case WE_GET_ANI_CCK_LEVEL:
	{
		hdd_debug("GET WMI_PDEV_PARAM_ANI_CCK_LEVEL");
		*value = wma_cli_get_command(adapter->session_id,
					     WMI_PDEV_PARAM_ANI_CCK_LEVEL,
					     PDEV_CMD);
		break;
	}

	case WE_GET_DYNAMIC_BW:
	{
		hdd_debug("GET WMI_PDEV_PARAM_ANI_CCK_LEVEL");
		*value = wma_cli_get_command(adapter->session_id,
					     WMI_PDEV_PARAM_DYNAMIC_BW,
					     PDEV_CMD);
		break;
	}

	case WE_GET_11N_RATE:
	{
		hdd_debug("GET WMI_VDEV_PARAM_FIXED_RATE");
		*value = wma_cli_get_command(adapter->session_id,
					     WMI_VDEV_PARAM_FIXED_RATE,
					     VDEV_CMD);
		break;
	}

	case WE_GET_AMPDU:
	{
		hdd_debug("GET AMPDU");
		*value = wma_cli_get_command(adapter->session_id,
					     GEN_VDEV_PARAM_AMPDU,
					     GEN_CMD);
		break;
	}

	case WE_GET_AMSDU:
	{
		hdd_debug("GET AMSDU");
		*value = wma_cli_get_command(adapter->session_id,
					     GEN_VDEV_PARAM_AMSDU,
					     GEN_CMD);
		break;
	}

	case WE_GET_ROAM_SYNCH_DELAY:
	{
		hdd_debug("GET ROAM SYNCH DELAY");
		*value = wma_cli_get_command(adapter->session_id,
					     GEN_VDEV_ROAM_SYNCH_DELAY,
					     GEN_CMD);
		break;
	}

	case WE_GET_TX_CHAINMASK:
	{
		hdd_debug("GET WMI_PDEV_PARAM_TX_CHAIN_MASK");
		*value = wma_cli_get_command(adapter->session_id,
					     WMI_PDEV_PARAM_TX_CHAIN_MASK,
					     PDEV_CMD);
		break;
	}

	case WE_GET_RX_CHAINMASK:
	{
		hdd_debug("GET WMI_PDEV_PARAM_RX_CHAIN_MASK");
		*value = wma_cli_get_command(adapter->session_id,
					     WMI_PDEV_PARAM_RX_CHAIN_MASK,
					     PDEV_CMD);
		break;
	}

	case WE_GET_TXPOW_2G:
	{
		uint32_t txpow2g = 0;

		hdd_debug("GET WMI_PDEV_PARAM_TXPOWER_LIMIT2G");
		*value = wma_cli_get_command(adapter->session_id,
					     WMI_PDEV_PARAM_TXPOWER_LIMIT2G,
					     PDEV_CMD);
		if (QDF_STATUS_SUCCESS !=
		    sme_cfg_get_int(mac_handle, WNI_CFG_CURRENT_TX_POWER_LEVEL,
				    &txpow2g)) {
			return -EIO;
		}
		hdd_debug("2G tx_power %d", txpow2g);
		break;
	}

	case WE_GET_TXPOW_5G:
	{
		uint32_t txpow5g = 0;

		hdd_debug("GET WMI_PDEV_PARAM_TXPOWER_LIMIT5G");
		*value = wma_cli_get_command(adapter->session_id,
					     WMI_PDEV_PARAM_TXPOWER_LIMIT5G,
					     PDEV_CMD);
		if (QDF_STATUS_SUCCESS !=
		    sme_cfg_get_int(mac_handle, WNI_CFG_CURRENT_TX_POWER_LEVEL,
				    &txpow5g)) {
			return -EIO;
		}
		hdd_debug("5G tx_power %d", txpow5g);
		break;
	}

	case WE_GET_PPS_PAID_MATCH:
	{
		hdd_debug("GET WMI_VDEV_PPS_PAID_MATCH");
		*value = wma_cli_get_command(adapter->session_id,
					     WMI_VDEV_PPS_PAID_MATCH,
					     PPS_CMD);
		break;
	}

	case WE_GET_PPS_GID_MATCH:
	{
		hdd_debug("GET WMI_VDEV_PPS_GID_MATCH");
		*value = wma_cli_get_command(adapter->session_id,
					     WMI_VDEV_PPS_GID_MATCH,
					     PPS_CMD);
		break;
	}

	case WE_GET_PPS_EARLY_TIM_CLEAR:
	{
		hdd_debug("GET WMI_VDEV_PPS_EARLY_TIM_CLEAR");
		*value = wma_cli_get_command(adapter->session_id,
					     WMI_VDEV_PPS_EARLY_TIM_CLEAR,
					     PPS_CMD);
		break;
	}

	case WE_GET_PPS_EARLY_DTIM_CLEAR:
	{
		hdd_debug("GET WMI_VDEV_PPS_EARLY_DTIM_CLEAR");
		*value = wma_cli_get_command(adapter->session_id,
					     WMI_VDEV_PPS_EARLY_DTIM_CLEAR,
					     PPS_CMD);
		break;
	}

	case WE_GET_PPS_EOF_PAD_DELIM:
	{
		hdd_debug("GET WMI_VDEV_PPS_EOF_PAD_DELIM");
		*value = wma_cli_get_command(adapter->session_id,
					     WMI_VDEV_PPS_EOF_PAD_DELIM,
					     PPS_CMD);
		break;
	}

	case WE_GET_PPS_MACADDR_MISMATCH:
	{
		hdd_debug("GET WMI_VDEV_PPS_MACADDR_MISMATCH");
		*value = wma_cli_get_command(adapter->session_id,
					     WMI_VDEV_PPS_MACADDR_MISMATCH,
					     PPS_CMD);
		break;
	}

	case WE_GET_PPS_DELIM_CRC_FAIL:
	{
		hdd_debug("GET WMI_VDEV_PPS_DELIM_CRC_FAIL");
		*value = wma_cli_get_command(adapter->session_id,
					     WMI_VDEV_PPS_DELIM_CRC_FAIL,
					     PPS_CMD);
		break;
	}

	case WE_GET_PPS_GID_NSTS_ZERO:
	{
		hdd_debug("GET WMI_VDEV_PPS_GID_NSTS_ZERO");
		*value = wma_cli_get_command(adapter->session_id,
					     WMI_VDEV_PPS_GID_NSTS_ZERO,
					     PPS_CMD);
		break;
	}

	case WE_GET_PPS_RSSI_CHECK:
	{

		hdd_debug("GET WMI_VDEV_PPS_RSSI_CHECK");
		*value = wma_cli_get_command(adapter->session_id,
					     WMI_VDEV_PPS_RSSI_CHECK,
					     PPS_CMD);
		break;
	}

	case WE_GET_QPOWER_MAX_PSPOLL_COUNT:
	{
		hdd_debug("WE_GET_QPOWER_MAX_PSPOLL_COUNT");
		*value = wma_cli_get_command(adapter->session_id,
					     WMI_STA_PS_PARAM_QPOWER_PSPOLL_COUNT,
					     QPOWER_CMD);
		break;
	}

	case WE_GET_QPOWER_MAX_TX_BEFORE_WAKE:
	{
		hdd_debug("WE_GET_QPOWER_MAX_TX_BEFORE_WAKE");
		*value = wma_cli_get_command(adapter->session_id,
					     WMI_STA_PS_PARAM_QPOWER_MAX_TX_BEFORE_WAKE,
					     QPOWER_CMD);
		break;
	}

	case WE_GET_QPOWER_SPEC_PSPOLL_WAKE_INTERVAL:
	{
		hdd_debug("WE_GET_QPOWER_SPEC_PSPOLL_WAKE_INTERVAL");
		*value = wma_cli_get_command(adapter->session_id,
					     WMI_STA_PS_PARAM_QPOWER_SPEC_PSPOLL_WAKE_INTERVAL,
					     QPOWER_CMD);
		break;
	}

	case WE_GET_QPOWER_SPEC_MAX_SPEC_NODATA_PSPOLL:
	{
		hdd_debug("WE_GET_QPOWER_MAX_PSPOLL_COUNT");
		*value = wma_cli_get_command(adapter->session_id,
					     WMI_STA_PS_PARAM_QPOWER_SPEC_MAX_SPEC_NODATA_PSPOLL,
					     QPOWER_CMD);
		break;
	}
	case WE_CAP_TSF:
		ret = hdd_capture_tsf(adapter, (uint32_t *)value, 1);
		break;
	case WE_GET_TEMPERATURE:
	{
		hdd_debug("WE_GET_TEMPERATURE");
		ret = wlan_hdd_get_temperature(adapter, value);
		break;
	}
	case WE_GET_DCM:
		hdd_debug("GET WMI_VDEV_PARAM_HE_DCM");
		*value = wma_cli_get_command(adapter->session_id,
					     WMI_VDEV_PARAM_HE_DCM,
					     VDEV_CMD);
		break;
	case WE_GET_RANGE_EXT:
		hdd_debug("GET WMI_VDEV_PARAM_HE_RANGE_EXT");
		*value = wma_cli_get_command(adapter->session_id,
					     WMI_VDEV_PARAM_HE_RANGE_EXT,
					     VDEV_CMD);
		break;
	default:
	{
		hdd_err("Invalid IOCTL get_value command %d",
		       value[0]);
		break;
	}
	}
	hdd_exit();
	qdf_mem_free(sme_config);
	return ret;
}

static int iw_setnone_getint(struct net_device *dev,
			     struct iw_request_info *info,
			     union iwreq_data *wrqu, char *extra)
{
	int ret;

	cds_ssr_protect(__func__);
	ret = __iw_setnone_getint(dev, info, wrqu, extra);
	cds_ssr_unprotect(__func__);

	return ret;
}

static int hdd_set_fwtest(int argc, int cmd, int value)
{
	struct set_fwtest_params *fw_test;

	/* check for max number of arguments */
	if (argc > (WMA_MAX_NUM_ARGS) ||
	    argc != HDD_FWTEST_PARAMS) {
		hdd_err("Too Many args %d", argc);
		return -EINVAL;
	}
	/*
	 * check if number of arguments are 3 then, check
	 * then set the default value for sounding interval.
	 */
	if (HDD_FWTEST_PARAMS == argc) {
		if (HDD_FWTEST_SU_PARAM_ID == cmd && 0 == value)
			value = HDD_FWTEST_SU_DEFAULT_VALUE;
		if (HDD_FWTEST_MU_PARAM_ID == cmd && 0 == value)
			value = HDD_FWTEST_MU_DEFAULT_VALUE;
	}
	/* check sounding interval value should not exceed to max */
	if (value > HDD_FWTEST_MAX_VALUE) {
		hdd_err("Invalid arguments value should not exceed max: %d",
			value);
		return -EINVAL;
	}
	fw_test = qdf_mem_malloc(sizeof(*fw_test));
	if (NULL == fw_test) {
		hdd_err("qdf_mem_malloc failed for fw_test");
		return -ENOMEM;
	}
	fw_test->arg = cmd;
	fw_test->value = value;
	if (QDF_STATUS_SUCCESS != sme_set_fw_test(fw_test)) {
		qdf_mem_free(fw_test);
		hdd_err("Not able to post FW_TEST_CMD message to WMA");
		return -EINVAL;
	}
	return 0;
}

/**
 * iw_set_three_ints_getnone() - Generic "set 3 params" private ioctl handler
 * @dev: device upon which the ioctl was received
 * @info: ioctl request information
 * @wrqu: ioctl request data
 * @extra: ioctl extra data
 *
 * Return: 0 on success, non-zero on error
 */
static int __iw_set_three_ints_getnone(struct net_device *dev,
				       struct iw_request_info *info,
				       union iwreq_data *wrqu, char *extra)
{
	struct hdd_adapter *adapter = WLAN_HDD_GET_PRIV_PTR(dev);
	int *value = (int *)extra;
	int sub_cmd = value[0];
	int ret;
	struct hdd_context *hdd_ctx = WLAN_HDD_GET_CTX(adapter);

	hdd_enter_dev(dev);

	if (!capable(CAP_NET_ADMIN)) {
		hdd_err("permission check failed");
		return -EPERM;
	}

	ret = wlan_hdd_validate_context(hdd_ctx);
	if (0 != ret)
		return ret;

	ret = hdd_check_private_wext_control(hdd_ctx, info);
	if (0 != ret)
		return ret;

	switch (sub_cmd) {

	case WE_SET_WLAN_DBG:
		qdf_print_set_category_verbose(qdf_get_pidx(), value[1],
					       value[2], value[3]);
		break;
	case WE_SET_DP_TRACE:
		qdf_dp_trace_set_value(value[1], value[2], value[3]);
		break;

	case WE_SET_DUAL_MAC_SCAN_CONFIG:
		hdd_debug("Ioctl to set dual mac scan config");
		if (hdd_ctx->config->dual_mac_feature_disable ==
				DISABLE_DBS_CXN_AND_SCAN) {
			hdd_err("Dual mac feature is disabled from INI");
			return -EPERM;
		}
		hdd_debug("%d %d %d", value[1], value[2], value[3]);
		policy_mgr_set_dual_mac_scan_config(hdd_ctx->hdd_psoc,
			value[1], value[2], value[3]);
		break;
	case WE_SET_FW_TEST:
	{
		ret = hdd_set_fwtest(value[1], value[2], value[3]);
		if (ret) {
			hdd_err("Not able to set fwtest %d", ret);
			return ret;
		}
	}
	break;
	default:
		hdd_err("Invalid IOCTL command %d", sub_cmd);
		break;

	}
	hdd_exit();
	return ret;
}

int iw_set_three_ints_getnone(struct net_device *dev,
			      struct iw_request_info *info,
			      union iwreq_data *wrqu, char *extra)
{
	int ret;

	cds_ssr_protect(__func__);
	ret = __iw_set_three_ints_getnone(dev, info, wrqu, extra);
	cds_ssr_unprotect(__func__);

	return ret;
}

/**
 * hdd_connection_state_string() - Get connection state string
 * @connection_state: enum to be converted to a string
 *
 * Return: the string equivalent of @connection_state
 */
static const char *
hdd_connection_state_string(eConnectionState connection_state)
{
	switch (connection_state) {
		CASE_RETURN_STRING(eConnectionState_NotConnected);
		CASE_RETURN_STRING(eConnectionState_Connecting);
		CASE_RETURN_STRING(eConnectionState_Associated);
		CASE_RETURN_STRING(eConnectionState_IbssDisconnected);
		CASE_RETURN_STRING(eConnectionState_IbssConnected);
		CASE_RETURN_STRING(eConnectionState_Disconnecting);
	default:
		return "UNKNOWN";
	}
}

#if defined(FEATURE_OEM_DATA_SUPPORT)
/**
 * iw_get_oem_data_cap_wrapper() - wrapper function to call legacy or new
 * wifi_pos api to get oem data caps
 * @dev: net device upon which the request was received
 * @info: ioctl request information
 * @wrqu: ioctl request data
 * @extra: ioctl data payload
 *
 * Return: 0 for success, negative errno value on failure
 */
static inline int iw_get_oem_data_cap_wrapper(struct net_device *dev,
					struct iw_request_info *info,
					union iwreq_data *wrqu, char *extra)
{
	return iw_get_oem_data_cap(dev, info, wrqu, extra);
}
#elif defined(WIFI_POS_CONVERGED)
static inline int iw_get_oem_data_cap_wrapper(struct net_device *dev,
					struct iw_request_info *info,
					union iwreq_data *wrqu, char *extra)
{
	struct hdd_adapter *adapter = WLAN_HDD_GET_PRIV_PTR(dev);
	struct hdd_context *hdd_ctx = WLAN_HDD_GET_CTX(adapter);

	return os_if_wifi_pos_populate_caps(hdd_ctx->hdd_psoc,
					(struct wifi_pos_driver_caps *)extra);
}
#else
static inline int iw_get_oem_data_cap_wrapper(struct net_device *dev,
					struct iw_request_info *info,
					union iwreq_data *wrqu, char *extra)
{
	return -ENOTSUPP;
}
#endif

/**
 * iw_get_char_setnone() - Generic "get string" private ioctl handler
 * @dev: device upon which the ioctl was received
 * @info: ioctl request information
 * @wrqu: ioctl request data
 * @extra: ioctl extra data
 *
 * Return: 0 on success, non-zero on error
 */
static int __iw_get_char_setnone(struct net_device *dev,
				 struct iw_request_info *info,
				 union iwreq_data *wrqu, char *extra)
{
	struct hdd_adapter *adapter = WLAN_HDD_GET_PRIV_PTR(dev);
	int sub_cmd = wrqu->data.flags;
	struct hdd_context *hdd_ctx;
	mac_handle_t mac_handle;
	int ret;

	hdd_enter_dev(dev);

	hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	ret = wlan_hdd_validate_context(hdd_ctx);
	if (0 != ret)
		return ret;

	ret = hdd_check_private_wext_control(hdd_ctx, info);
	if (0 != ret)
		return ret;

	mac_handle = hdd_ctx->mac_handle;
	switch (sub_cmd) {
	case WE_WLAN_VERSION:
	{
		wrqu->data.length = hdd_wlan_get_version(hdd_ctx,
							 WE_MAX_STR_LEN, extra);
		break;
	}

	case WE_GET_STATS:
	{
		hdd_wlan_get_stats(adapter, &(wrqu->data.length),
				   extra, WE_MAX_STR_LEN);
		break;
	}

	case WE_GET_SUSPEND_RESUME_STATS:
	{
		ret = wlan_hdd_write_suspend_resume_stats(hdd_ctx, extra,
							  WE_MAX_STR_LEN);
		if (ret >= 0) {
			wrqu->data.length = ret;
			ret = 0;
		}

		break;
	}

	case WE_LIST_FW_PROFILE:
		hdd_wlan_list_fw_profile(&(wrqu->data.length),
					extra, WE_MAX_STR_LEN);
		break;

	/* The case prints the current state of the HDD, SME, CSR, PE,
	 * TL it can be extended for WDI Global State as well.  And
	 * currently it only checks P2P_CLIENT adapter.  P2P_DEVICE
	 * and P2P_GO have not been added as of now.
	 */
	case WE_GET_STATES:
	{
		int buf = 0, len = 0;
		int adapter_num = 0;
		int count = 0, check = 1;

		struct hdd_station_ctx *sta_ctx = NULL;

		struct hdd_context *hdd_ctx = WLAN_HDD_GET_CTX(adapter);
		struct hdd_adapter *useAdapter = NULL;

		/* Print wlan0 or p2p0 states based on the adapter_num
		 * by using the correct adapter
		 */
		while (adapter_num < 2) {
			if (WLAN_ADAPTER == adapter_num) {
				useAdapter = adapter;
				buf =
					scnprintf(extra + len,
						  WE_MAX_STR_LEN - len,
						  "\n\n wlan0 States:-");
				len += buf;
			} else if (P2P_ADAPTER == adapter_num) {
				buf =
					scnprintf(extra + len,
						  WE_MAX_STR_LEN - len,
						  "\n\n p2p0 States:-");
				len += buf;

				if (!hdd_ctx) {
					buf =
						scnprintf(extra + len,
							  WE_MAX_STR_LEN -
							  len,
							  "\n hdd_ctx is NULL");
					len += buf;
					break;
				}

				/* Printing p2p0 states only in the
				 * case when the device is configured
				 * as a p2p_client
				 */
				useAdapter =
					hdd_get_adapter(hdd_ctx,
							QDF_P2P_CLIENT_MODE);
				if (!useAdapter) {
					buf =
						scnprintf(extra + len,
							  WE_MAX_STR_LEN -
							  len,
							  "\n Device not configured as P2P_CLIENT.");
					len += buf;
					break;
				}
			}

			if (!mac_handle) {
				buf = scnprintf(extra + len,
						WE_MAX_STR_LEN - len,
						"\n mac_handle is NULL");
				len += buf;
				break;
			}
			sta_ctx =
				WLAN_HDD_GET_STATION_CTX_PTR(useAdapter);


			buf =
				scnprintf(extra + len, WE_MAX_STR_LEN - len,
					  "\n HDD Conn State - %s "
					  "\n\n SME State:"
					  "\n Neighbour Roam State - %s"
					  "\n CSR State - %s"
					  "\n CSR Substate - %s",
					  hdd_connection_state_string
						  (sta_ctx->conn_info.connState),
					  mac_trace_get_neighbour_roam_state
						  (sme_get_neighbor_roam_state
							  (mac_handle, useAdapter->session_id)),
					  mac_trace_getcsr_roam_state
						  (sme_get_current_roam_state
							  (mac_handle, useAdapter->session_id)),
					  mac_trace_getcsr_roam_sub_state
						  (sme_get_current_roam_sub_state
							  (mac_handle, useAdapter->session_id))
					  );
			len += buf;
			adapter_num++;
		}

		if (mac_handle) {
			/* Printing Lim State starting with global lim states */
			buf =
				scnprintf(extra + len, WE_MAX_STR_LEN - len,
					  "\n\n LIM STATES:-"
					  "\n Global Sme State - %s "
					  "\n Global mlm State - %s " "\n",
					  mac_trace_get_lim_sme_state
						  (sme_get_lim_sme_state(mac_handle)),
					  mac_trace_get_lim_mlm_state
						  (sme_get_lim_sme_state(mac_handle))
					  );
			len += buf;

			while (check < 3 && count < 255) {
				if (sme_is_lim_session_valid(mac_handle, count)) {
					buf =
						scnprintf(extra + len,
							  WE_MAX_STR_LEN -
							  len,
							  "\n Lim Valid Session %d:-"
							  "\n PE Sme State - %s "
							  "\n PE Mlm State - %s "
							  "\n", check,
							  mac_trace_get_lim_sme_state
								  (sme_get_lim_sme_session_state
									  (mac_handle, count)),
							  mac_trace_get_lim_mlm_state
								  (sme_get_lim_mlm_session_state
									  (mac_handle, count))
							  );

					len += buf;
					check++;
				}
				count++;
			}
		}

		wrqu->data.length = strlen(extra) + 1;
		break;
	}

	case WE_GET_CFG:
	{
		hdd_debug("Printing CLD global INI Config");
		hdd_cfg_get_global_config(WLAN_HDD_GET_CTX(adapter),
					  extra,
					  QCSAP_IOCTL_MAX_STR_LEN);
		wrqu->data.length = strlen(extra) + 1;
		break;
	}
	case WE_GET_RSSI:
	{
		int8_t s7Rssi = 0;

		wlan_hdd_get_rssi(adapter, &s7Rssi);
		snprintf(extra, WE_MAX_STR_LEN, "rssi=%d", s7Rssi);
		wrqu->data.length = strlen(extra) + 1;
		break;
	}

	case WE_GET_WMM_STATUS:
	{
		snprintf(extra, WE_MAX_STR_LEN,
			 "\nDir: 0=up, 1=down, 3=both\n"
			 "|------------------------|\n"
			 "|AC | ACM |Admitted| Dir |\n"
			 "|------------------------|\n"
			 "|VO |  %d  |  %3s   |  %d  |\n"
			 "|VI |  %d  |  %3s   |  %d  |\n"
			 "|BE |  %d  |  %3s   |  %d  |\n"
			 "|BK |  %d  |  %3s   |  %d  |\n"
			 "|------------------------|\n",
			 adapter->hdd_wmm_status.
			 wmmAcStatus[SME_AC_VO].wmmAcAccessRequired,
			 adapter->hdd_wmm_status.
			 wmmAcStatus[SME_AC_VO].
			 wmmAcAccessAllowed ? "YES" : "NO",
			 adapter->hdd_wmm_status.
			 wmmAcStatus[SME_AC_VO].wmmAcTspecInfo.
			 ts_info.direction,
			 adapter->hdd_wmm_status.
			 wmmAcStatus[SME_AC_VI].wmmAcAccessRequired,
			 adapter->hdd_wmm_status.
			 wmmAcStatus[SME_AC_VI].
			 wmmAcAccessAllowed ? "YES" : "NO",
			 adapter->hdd_wmm_status.
			 wmmAcStatus[SME_AC_VI].wmmAcTspecInfo.
			 ts_info.direction,
			 adapter->hdd_wmm_status.
			 wmmAcStatus[SME_AC_BE].wmmAcAccessRequired,
			 adapter->hdd_wmm_status.
			 wmmAcStatus[SME_AC_BE].
			 wmmAcAccessAllowed ? "YES" : "NO",
			 adapter->hdd_wmm_status.
			 wmmAcStatus[SME_AC_BE].wmmAcTspecInfo.
			 ts_info.direction,
			 adapter->hdd_wmm_status.
			 wmmAcStatus[SME_AC_BK].wmmAcAccessRequired,
			 adapter->hdd_wmm_status.
			 wmmAcStatus[SME_AC_BK].
			 wmmAcAccessAllowed ? "YES" : "NO",
			 adapter->hdd_wmm_status.
			 wmmAcStatus[SME_AC_BK].wmmAcTspecInfo.
			 ts_info.direction);

		wrqu->data.length = strlen(extra) + 1;
		break;
	}

	case WE_GET_BA_AGEING_TIMEOUT:
	{
		uint8_t ac_cat = 4;
		uint32_t duration[ac_cat], i;
		void *soc = cds_get_context(QDF_MODULE_ID_SOC);

		if (!soc) {
			hdd_err("Invalid SOC handle");
			break;
		}

		for (i = 0; i < ac_cat; i++)
			cdp_get_ba_timeout(soc, i, &duration[i]);

		snprintf(extra, WE_MAX_STR_LEN,
			 "\n|------------------------------|\n"
			 "|AC | BA aging timeout duration |\n"
			 "|--------------------------------|\n"
			 "|VO |  %d        |\n"
			 "|VI |  %d        |\n"
			 "|BE |  %d        |\n"
			 "|BK |  %d        |\n"
			 "|--------------------------------|\n",
			duration[3], duration[2], duration[1], duration[0]);

		wrqu->data.length = strlen(extra) + 1;
		break;
	}

	case WE_GET_CHANNEL_LIST:
	{
		QDF_STATUS status;
		uint8_t i, len;
		char *buf;
		uint8_t ubuf[WNI_CFG_COUNTRY_CODE_LEN];
		uint8_t ubuf_len = WNI_CFG_COUNTRY_CODE_LEN;
		struct channel_list_info channel_list;

		memset(&channel_list, 0, sizeof(channel_list));
		status = iw_get_channel_list(dev, info, wrqu,
						   (char *)&channel_list);
		if (!QDF_IS_STATUS_SUCCESS(status)) {
			hdd_err("GetChannelList Failed!!!");
			return -EINVAL;
		}
		buf = extra;
		/*
		 * Maximum channels = WNI_CFG_VALID_CHANNEL_LIST_LEN.
		 * Maximum buffer needed = 5 * number of channels.
		 * Check ifsufficient buffer is available and then
		 * proceed to fill the buffer.
		 */
		if (WE_MAX_STR_LEN <
		    (5 * WNI_CFG_VALID_CHANNEL_LIST_LEN)) {
			hdd_err("Insufficient Buffer to populate channel list");
			return -EINVAL;
		}
		len = scnprintf(buf, WE_MAX_STR_LEN, "%u ",
				channel_list.num_channels);
		if (QDF_STATUS_SUCCESS == sme_get_country_code(mac_handle,
							       ubuf,
							       &ubuf_len)) {
			/* Printing Country code in getChannelList */
			for (i = 0; i < (ubuf_len - 1); i++)
				len += scnprintf(buf + len,
						WE_MAX_STR_LEN - len,
						"%c", ubuf[i]);
		}
		for (i = 0; i < channel_list.num_channels; i++) {
			len +=
				scnprintf(buf + len, WE_MAX_STR_LEN - len,
					  " %u", channel_list.channels[i]);
		}
		wrqu->data.length = strlen(extra) + 1;

		break;
	}
#ifdef FEATURE_WLAN_TDLS
	case WE_GET_TDLS_PEERS:
	{
		wrqu->data.length =
			wlan_hdd_tdls_get_all_peers(adapter, extra,
						    WE_MAX_STR_LEN) + 1;
		break;
	}
#endif
#ifdef WLAN_FEATURE_11W
	case WE_GET_11W_INFO:
	{
		struct csr_roam_profile *roam_profile =
			hdd_roam_profile(adapter);

		hdd_debug("WE_GET_11W_ENABLED = %d",
		       roam_profile->MFPEnabled);

		snprintf(extra, WE_MAX_STR_LEN,
			 "\n BSSID %02X:%02X:%02X:%02X:%02X:%02X, Is PMF Assoc? %d"
			 "\n Number of Unprotected Disassocs %d"
			 "\n Number of Unprotected Deauths %d",
			 roam_profile->BSSIDs.bssid->bytes[0],
			 roam_profile->BSSIDs.bssid->bytes[1],
			 roam_profile->BSSIDs.bssid->bytes[2],
			 roam_profile->BSSIDs.bssid->bytes[3],
			 roam_profile->BSSIDs.bssid->bytes[4],
			 roam_profile->BSSIDs.bssid->bytes[5],
			 roam_profile->MFPEnabled,
			 adapter->hdd_stats.hdd_pmf_stats.
			 num_unprot_disassoc_rx,
			 adapter->hdd_stats.hdd_pmf_stats.
			 num_unprot_deauth_rx);

		wrqu->data.length = strlen(extra) + 1;
		break;
	}
#endif
	case WE_GET_IBSS_STA_INFO:
	{
		struct hdd_station_ctx *sta_ctx =
			WLAN_HDD_GET_STATION_CTX_PTR(adapter);
		int idx = 0;
		int length = 0, buf = 0;

		for (idx = 0; idx < MAX_PEERS; idx++) {
			if (HDD_WLAN_INVALID_STA_ID !=
					sta_ctx->conn_info.staId[idx]) {
				buf = snprintf
					      ((extra + length),
					      WE_MAX_STR_LEN - length,
					      "\n%d .%02x:%02x:%02x:%02x:%02x:%02x\n",
					      sta_ctx->conn_info.staId[idx],
					      sta_ctx->conn_info.
					      peerMacAddress[idx].bytes[0],
					      sta_ctx->conn_info.
					      peerMacAddress[idx].bytes[1],
					      sta_ctx->conn_info.
					      peerMacAddress[idx].bytes[2],
					      sta_ctx->conn_info.
					      peerMacAddress[idx].bytes[3],
					      sta_ctx->conn_info.
					      peerMacAddress[idx].bytes[4],
					      sta_ctx->conn_info.
					      peerMacAddress[idx].bytes[5]
					      );
				length += buf;
			}
		}
		wrqu->data.length = strlen(extra) + 1;
		break;
	}
	case WE_GET_PHYMODE:
	{
		bool ch_bond24 = false, ch_bond5g = false;
		struct hdd_context *hddctx = WLAN_HDD_GET_CTX(adapter);
		eCsrPhyMode phymode;
		enum band_info currBand;
		tSmeConfigParams *sme_config;

		sme_config = qdf_mem_malloc(sizeof(*sme_config));
		if (!sme_config) {
			hdd_err("Out of memory");
			ret = -ENOMEM;
			break;
		}

		sme_get_config_param(mac_handle, sme_config);
		if (WNI_CFG_CHANNEL_BONDING_MODE_DISABLE !=
		    sme_config->csrConfig.channelBondingMode24GHz)
			ch_bond24 = true;

		if (WNI_CFG_CHANNEL_BONDING_MODE_DISABLE !=
		    sme_config->csrConfig.channelBondingMode5GHz)
			ch_bond5g = true;

		qdf_mem_free(sme_config);

		phymode = sme_get_phy_mode(mac_handle);
		if ((QDF_STATUS_SUCCESS !=
		     sme_get_freq_band(mac_handle, &currBand))) {
			hdd_err("Failed to get current band config");
			return -EIO;
		}

		switch (phymode) {
		case eCSR_DOT11_MODE_AUTO:
			snprintf(extra, WE_MAX_STR_LEN, "AUTO MODE");
			break;
		case eCSR_DOT11_MODE_11n:
		case eCSR_DOT11_MODE_11n_ONLY:
			if (currBand == BAND_2G) {
				if (ch_bond24)
					snprintf(extra, WE_MAX_STR_LEN,
						 "11NGHT40");
				else
					snprintf(extra, WE_MAX_STR_LEN,
						 "11NGHT20");
			} else if (currBand == BAND_5G) {
				if (ch_bond5g)
					snprintf(extra, WE_MAX_STR_LEN,
						 "11NAHT40");
				else
					snprintf(extra, WE_MAX_STR_LEN,
						 "11NAHT20");
			} else {
				snprintf(extra, WE_MAX_STR_LEN, "11N");
			}
			break;
		case eCSR_DOT11_MODE_abg:
			snprintf(extra, WE_MAX_STR_LEN, "11ABG");
			break;
		case eCSR_DOT11_MODE_11a:
			snprintf(extra, WE_MAX_STR_LEN, "11A");
			break;
		case eCSR_DOT11_MODE_11b:
		case eCSR_DOT11_MODE_11b_ONLY:
			snprintf(extra, WE_MAX_STR_LEN, "11B");
			break;
		case eCSR_DOT11_MODE_11g:
		case eCSR_DOT11_MODE_11g_ONLY:
			snprintf(extra, WE_MAX_STR_LEN, "11G");
			break;
		case eCSR_DOT11_MODE_11ac:
		case eCSR_DOT11_MODE_11ac_ONLY:
			if (hddctx->config->vhtChannelWidth ==
			    eHT_CHANNEL_WIDTH_20MHZ)
				snprintf(extra, WE_MAX_STR_LEN,
					 "11ACVHT20");
			else if (hddctx->config->vhtChannelWidth ==
				 eHT_CHANNEL_WIDTH_40MHZ)
				snprintf(extra, WE_MAX_STR_LEN,
					 "11ACVHT40");
			else if (hddctx->config->vhtChannelWidth ==
				 eHT_CHANNEL_WIDTH_80MHZ)
				snprintf(extra, WE_MAX_STR_LEN,
					 "11ACVHT80");
			else if (hddctx->config->vhtChannelWidth ==
				 eHT_CHANNEL_WIDTH_160MHZ)
				snprintf(extra, WE_MAX_STR_LEN,
					 "11ACVHT160");
			break;
		case eCSR_DOT11_MODE_11ax:
		case eCSR_DOT11_MODE_11ax_ONLY:
			/* currently using vhtChannelWidth */
			if (hddctx->config->vhtChannelWidth ==
			    eHT_CHANNEL_WIDTH_20MHZ)
				snprintf(extra, WE_MAX_STR_LEN,
					 "11AX_HE_20");
			else if (hddctx->config->vhtChannelWidth ==
				 eHT_CHANNEL_WIDTH_40MHZ)
				snprintf(extra, WE_MAX_STR_LEN,
					 "11AX_HE_40");
			else if (hddctx->config->vhtChannelWidth ==
				 eHT_CHANNEL_WIDTH_80MHZ)
				snprintf(extra, WE_MAX_STR_LEN,
					 "11AX_HE_80");
			else if (hddctx->config->vhtChannelWidth ==
				 eHT_CHANNEL_WIDTH_160MHZ)
				snprintf(extra, WE_MAX_STR_LEN,
					 "11AX_HE_160");
			break;
		}

		wrqu->data.length = strlen(extra) + 1;
		break;
	}

	case WE_GET_OEM_DATA_CAP:
		return iw_get_oem_data_cap_wrapper(dev, info, wrqu, extra);
	case WE_GET_SNR:
	{
		int8_t s7snr = 0;
		int status = 0;
		struct hdd_context *hdd_ctx;
		struct hdd_station_ctx *sta_ctx;

		hdd_ctx = WLAN_HDD_GET_CTX(adapter);
		status = wlan_hdd_validate_context(hdd_ctx);
		if (status)
			return status;

		sta_ctx = WLAN_HDD_GET_STATION_CTX_PTR(adapter);
		if (0 == hdd_ctx->config->fEnableSNRMonitoring ||
		    eConnectionState_Associated !=
		    sta_ctx->conn_info.connState) {
			hdd_err("getSNR failed: Enable SNR Monitoring-%d, ConnectionState-%d",
			       hdd_ctx->config->fEnableSNRMonitoring,
			       sta_ctx->conn_info.connState);
			return -ENONET;
		}
		wlan_hdd_get_snr(adapter, &s7snr);
		snprintf(extra, WE_MAX_STR_LEN, "snr=%d", s7snr);
		wrqu->data.length = strlen(extra) + 1;
		break;
	}

	default:
		hdd_err("Invalid IOCTL command %d", sub_cmd);
		break;
	}

	hdd_exit();
	return ret;
}

static int iw_get_char_setnone(struct net_device *dev,
			       struct iw_request_info *info,
			       union iwreq_data *wrqu, char *extra)
{
	int ret;

	cds_ssr_protect(__func__);
	ret = __iw_get_char_setnone(dev, info, wrqu, extra);
	cds_ssr_unprotect(__func__);

	return ret;
}

/**
 * iw_setnone_getnone() - Generic "action" private ioctl handler
 * @dev: device upon which the ioctl was received
 * @info: ioctl request information
 * @wrqu: ioctl request data
 * @extra: ioctl extra data
 *
 * Return: 0 on success, non-zero on error
 */
static int __iw_setnone_getnone(struct net_device *dev,
				struct iw_request_info *info,
				union iwreq_data *wrqu, char *extra)
{
	struct hdd_adapter *adapter = WLAN_HDD_GET_PRIV_PTR(dev);
	struct hdd_context *hdd_ctx;
	mac_handle_t mac_handle;
	int ret;
	int sub_cmd;

	hdd_enter_dev(dev);

	hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	ret = wlan_hdd_validate_context(hdd_ctx);
	if (0 != ret)
		return ret;

	ret = hdd_check_private_wext_control(hdd_ctx, info);
	if (0 != ret)
		return ret;

#ifdef CONFIG_COMPAT
	/* this ioctl is a special case where a sub-ioctl is used and both
	 * the number of get and set args is 0.  in this specific case the
	 * logic in iwpriv places the sub_cmd in the data.flags portion of
	 * the iwreq.  unfortunately the location of this field will be
	 * different between 32-bit and 64-bit userspace, and the standard
	 * compat support in the kernel does not handle this case.  so we
	 * need to explicitly handle it here.
	 */
	if (in_compat_syscall()) {
		struct compat_iw_point *compat_iw_point =
			(struct compat_iw_point *)&wrqu->data;
		sub_cmd = compat_iw_point->flags;
	} else {
		sub_cmd = wrqu->data.flags;
	}
#else
	sub_cmd = wrqu->data.flags;
#endif

	mac_handle = hdd_ctx->mac_handle;
	switch (sub_cmd) {

	case WE_GET_RECOVERY_STAT:
		sme_get_recovery_stats(mac_handle);
		break;

	case WE_GET_FW_PROFILE_DATA:
		ret = wma_cli_set_command(adapter->session_id,
				WMI_WLAN_PROFILE_GET_PROFILE_DATA_CMDID,
				0, DBG_CMD);
		break;

	case WE_IBSS_GET_PEER_INFO_ALL:
		hdd_wlan_get_ibss_peer_info_all(adapter);
		break;

	case WE_SET_REASSOC_TRIGGER:
	{
		struct hdd_adapter *adapter = WLAN_HDD_GET_PRIV_PTR(dev);
		tSirMacAddr bssid;
		uint32_t roamId = INVALID_ROAM_ID;
		uint8_t operating_ch =
			adapter->session.station.conn_info.operationChannel;
		tCsrRoamModifyProfileFields modProfileFields;

		sme_get_modify_profile_fields(mac_handle, adapter->session_id,
					      &modProfileFields);
		if (roaming_offload_enabled(hdd_ctx)) {
			qdf_mem_copy(bssid,
				&adapter->session.station.conn_info.bssId,
				sizeof(bssid));
			hdd_wma_send_fastreassoc_cmd(adapter,
						     bssid, operating_ch);
		} else {
			sme_roam_reassoc(mac_handle, adapter->session_id,
					 NULL, modProfileFields, &roamId, 1);
		}
		return 0;
	}

	case WE_STOP_OBSS_SCAN:
		/*
		 * 1.OBSS Scan is mandatory while operating in 2.4GHz
		 * 2.OBSS scan is stopped by Firmware during the disassociation
		 * 3.OBSS stop comamnd is added for debugging purpose
		 */
		if (!mac_handle) {
			hdd_err("mac_handle context is NULL");
			return -EINVAL;
		}
		sme_ht40_stop_obss_scan(mac_handle, adapter->session_id);
		break;

	default:
		hdd_err("unknown ioctl %d", sub_cmd);
		break;
	}
	hdd_exit();
	return ret;
}

static int iw_setnone_getnone(struct net_device *dev,
			      struct iw_request_info *info,
			      union iwreq_data *wrqu, char *extra)
{
	int ret;

	cds_ssr_protect(__func__);
	ret = __iw_setnone_getnone(dev, info, wrqu, extra);
	cds_ssr_unprotect(__func__);

	return ret;
}

#ifdef MPC_UT_FRAMEWORK
static int iw_get_policy_manager_ut_ops(struct hdd_context *hdd_ctx,
			struct hdd_adapter *adapter, int sub_cmd, int *apps_args)
{
	switch (sub_cmd) {
	case WE_POLICY_MANAGER_CLIST_CMD:
	{
		hdd_debug("<iwpriv wlan0 pm_clist> is called");
		if ((apps_args[0] < 0) || (apps_args[1] < 0) ||
		    (apps_args[2] < 0) || (apps_args[3] < 0) ||
		    (apps_args[4] < 0) || (apps_args[5] < 0) ||
		    (apps_args[6] < 0) || (apps_args[7] < 0)) {
			hdd_err("Invalid input params received for the IOCTL");
			return 0;
		}
		policy_mgr_incr_connection_count_utfw(hdd_ctx->hdd_psoc,
			apps_args[0], apps_args[1], apps_args[2], apps_args[3],
			apps_args[4], apps_args[5], apps_args[6], apps_args[7]);
	}
	break;

	case WE_POLICY_MANAGER_DLIST_CMD:
	{
		hdd_debug("<iwpriv wlan0 pm_dlist> is called");
		if ((apps_args[0] < 0) || (apps_args[1] < 0)) {
			hdd_err("Invalid input params received for the IOCTL");
			return 0;
		}
		policy_mgr_decr_connection_count_utfw(hdd_ctx->hdd_psoc,
			apps_args[0], apps_args[1]);
	}
	break;

	case WE_POLICY_MANAGER_ULIST_CMD:
	{
		hdd_debug("<iwpriv wlan0 pm_ulist> is called");
		if ((apps_args[0] < 0) || (apps_args[1] < 0) ||
		    (apps_args[2] < 0) || (apps_args[3] < 0) ||
		    (apps_args[4] < 0) || (apps_args[5] < 0) ||
		    (apps_args[6] < 0) || (apps_args[7] < 0)) {
			hdd_err("Invalid input params received for the IOCTL");
			return 0;
		}
		policy_mgr_update_connection_info_utfw(hdd_ctx->hdd_psoc,
			apps_args[0], apps_args[1], apps_args[2], apps_args[3],
			apps_args[4], apps_args[5], apps_args[6], apps_args[7]);
	}
	break;

	case WE_POLICY_MANAGER_DBS_CMD:
	{
		hdd_debug("<iwpriv wlan0 pm_dbs> is called");
		if (apps_args[0] == 0)
			wma_set_dbs_capability_ut(0);
		else
			wma_set_dbs_capability_ut(1);

		if (apps_args[1] >= PM_THROUGHPUT &&
			apps_args[1] <= PM_LATENCY) {
			pr_info("setting system pref to [%d]\n", apps_args[1]);
			hdd_ctx->config->conc_system_pref = apps_args[1];
		}
	}
	break;

	case WE_POLICY_MANAGER_PCL_CMD:
	{
		uint8_t pcl[QDF_MAX_NUM_CHAN] = {0};
		uint8_t weight_list[QDF_MAX_NUM_CHAN] = {0};
		uint32_t pcl_len = 0, i = 0;

		hdd_debug("<iwpriv wlan0 pm_pcl> is called");

		if (apps_args[0] < 0) {
			hdd_err("Invalid input param received for the IOCTL");
			return 0;
		}
		policy_mgr_get_pcl(hdd_ctx->hdd_psoc, apps_args[0],
				pcl, &pcl_len,
				weight_list, QDF_ARRAY_SIZE(weight_list));
		pr_info("PCL list for role[%d] is {", apps_args[0]);
		for (i = 0 ; i < pcl_len; i++)
			pr_info(" %d, ", pcl[i]);
		pr_info("}--------->\n");
	}
	break;

	case WE_POLICY_SET_HW_MODE_CMD:
	{
		if (apps_args[0] == 0) {
			hdd_err("set hw mode for single mac");
			policy_mgr_pdev_set_hw_mode(hdd_ctx->hdd_psoc,
					adapter->session_id,
					HW_MODE_SS_2x2,
					HW_MODE_80_MHZ,
					HW_MODE_SS_0x0, HW_MODE_BW_NONE,
					HW_MODE_MAC_BAND_NONE,
					HW_MODE_DBS_NONE,
					HW_MODE_AGILE_DFS_NONE,
					HW_MODE_SBS_NONE,
					POLICY_MGR_UPDATE_REASON_UT, PM_NOP);
		} else if (apps_args[0] == 1) {
			hdd_err("set hw mode for dual mac");
			policy_mgr_pdev_set_hw_mode(hdd_ctx->hdd_psoc,
					adapter->session_id,
					HW_MODE_SS_1x1,
					HW_MODE_80_MHZ,
					HW_MODE_SS_1x1, HW_MODE_40_MHZ,
					HW_MODE_MAC_BAND_NONE,
					HW_MODE_DBS,
					HW_MODE_AGILE_DFS_NONE,
					HW_MODE_SBS_NONE,
					POLICY_MGR_UPDATE_REASON_UT, PM_NOP);
		}
	}
	break;

	case WE_POLICY_MANAGER_QUERY_ACTION_CMD:
	{
		hdd_debug("<iwpriv wlan0 pm_query_action> is called");
		if (apps_args[0] < 0) {
			hdd_err("Invalid input params received for the IOCTL");
			return 0;
		}
		policy_mgr_current_connections_update(
			hdd_ctx->hdd_psoc,
			adapter->session_id, apps_args[0],
			POLICY_MGR_UPDATE_REASON_UT);
	}
	break;

	case WE_POLICY_MANAGER_QUERY_ALLOW_CMD:
	{
		bool allow;

		hdd_debug("<iwpriv wlan0 pm_query_allow> is called");
		if ((apps_args[0] < 0) || (apps_args[1] < 0) ||
		    (apps_args[2] < 0)) {
			hdd_err("Invalid input params received for the IOCTL");
			return 0;
		}
		allow = policy_mgr_allow_concurrency(hdd_ctx->hdd_psoc,
				apps_args[0], apps_args[1], apps_args[2]);
		pr_info("allow %d {0 = don't allow, 1 = allow}", allow);
	}
	break;

	case WE_POLICY_MANAGER_SCENARIO_CMD:
	{
		clean_report(hdd_ctx);
		if (apps_args[0] == 1) {
			wlan_hdd_one_connection_scenario(hdd_ctx);
		} else if (apps_args[0] == 2) {
			wlan_hdd_two_connections_scenario(hdd_ctx,
				6, POLICY_MGR_TWO_TWO);
			wlan_hdd_two_connections_scenario(hdd_ctx,
				36, POLICY_MGR_TWO_TWO);
			wlan_hdd_two_connections_scenario(hdd_ctx,
				6, POLICY_MGR_ONE_ONE);
			wlan_hdd_two_connections_scenario(hdd_ctx,
				36, POLICY_MGR_ONE_ONE);
		} else if (apps_args[0] == 3) {
			/* MCC on same band with 2x2 same mac*/
			wlan_hdd_three_connections_scenario(hdd_ctx,
				6, 11, POLICY_MGR_TWO_TWO, 0);
			/* MCC on diff band with 2x2 same mac*/
			wlan_hdd_three_connections_scenario(hdd_ctx,
				6, 36, POLICY_MGR_TWO_TWO, 0);
			/* MCC on diff band with 1x1 diff mac */
			wlan_hdd_three_connections_scenario(hdd_ctx,
				36, 6, POLICY_MGR_ONE_ONE, 0);
			/* MCC on diff band with 1x1 same mac */
			wlan_hdd_three_connections_scenario(hdd_ctx,
				36, 6, POLICY_MGR_ONE_ONE, 1);
			/* SCC on same band with 2x2 same mac */
			wlan_hdd_three_connections_scenario(hdd_ctx,
				36, 36, POLICY_MGR_TWO_TWO, 0);
			/* SCC on same band with 1x1 same mac */
			wlan_hdd_three_connections_scenario(hdd_ctx,
				36, 36, POLICY_MGR_ONE_ONE, 1);
			/* MCC on same band with 2x2 same mac */
			wlan_hdd_three_connections_scenario(hdd_ctx,
				36, 149, POLICY_MGR_TWO_TWO, 0);
			/* MCC on same band with 1x1 same mac */
			wlan_hdd_three_connections_scenario(hdd_ctx,
				36, 149, POLICY_MGR_ONE_ONE, 1);
		}
		print_report(hdd_ctx);
	}
	break;
	}
	return 0;
}
#else
static int iw_get_policy_manager_ut_ops(struct hdd_context *hdd_ctx,
			struct hdd_adapter *adapter, int sub_cmd, int *apps_args)
{
	return 0;
}
#endif

/**
 * hdd_ch_avoid_unit_cmd - unit test ch avoidance
 * @hdd_ctx: hdd_context
 * @num_args: input args number
 * @apps_args: args data ptr
 *
 * This is to inject a ch avoid event to do unit test SAP chan avoidance.
 *
 * Return: void
 */
#if WLAN_DEBUG
static void hdd_ch_avoid_unit_cmd(struct hdd_context *hdd_ctx,
				  int num_args, int *apps_args)
{
	struct ch_avoid_ind_type ch_avoid;
	int cnt = 0, i;

	if (num_args < 2 || num_args > CH_AVOID_MAX_RANGE * 2 ||
	    num_args % 2 != 0)
		return;
	hdd_info("simulate ch avoid num_args %d", num_args);
	for (i = 0; i < num_args && i < CH_AVOID_MAX_RANGE * 2; i++) {
		ch_avoid.avoid_freq_range[cnt].start_freq =
			apps_args[i];
		ch_avoid.avoid_freq_range[cnt].end_freq =
			apps_args[++i];

		hdd_info("simulate ch avoid [%d %d]",
			 ch_avoid.avoid_freq_range[cnt].start_freq,
			 ch_avoid.avoid_freq_range[cnt].end_freq);
		cnt++;
	}
	ch_avoid.ch_avoid_range_cnt = cnt;
	ucfg_reg_unit_simulate_ch_avoid(hdd_ctx->hdd_psoc, &ch_avoid);
}
#else
static void hdd_ch_avoid_unit_cmd(struct hdd_context *hdd_ctx,
				  int num_args, int *apps_args)
{
}
#endif
/**
 * __iw_set_var_ints_getnone - Generic "set many" private ioctl handler
 * @dev: device upon which the ioctl was received
 * @info: ioctl request information
 * @wrqu: ioctl request data
 * @extra: ioctl extra data
 *
 * This is an SSR-protected generic handler for private ioctls which
 * take multiple arguments.  Note that this implementation is also
 * somewhat unique in that it is shared by both STA-mode and SAP-mode
 * interfaces.
 *
 * Return: 0 on success, non-zero on error
 */
static int __iw_set_var_ints_getnone(struct net_device *dev,
				     struct iw_request_info *info,
				     union iwreq_data *wrqu, char *extra)
{
	struct hdd_adapter *adapter = WLAN_HDD_GET_PRIV_PTR(dev);
	mac_handle_t mac_handle;
	struct hdd_station_ctx *sta_ctx = WLAN_HDD_GET_STATION_CTX_PTR(adapter);
	int sub_cmd;
	int *apps_args = (int *) extra;
	struct hdd_context *hdd_ctx;
	int ret, num_args;
	void *soc = NULL;
	struct cdp_pdev *pdev = NULL;
	struct cdp_vdev *vdev = NULL;
	struct cdp_txrx_stats_req req = {0};

	hdd_enter_dev(dev);

	hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	ret = wlan_hdd_validate_context(hdd_ctx);
	if (0 != ret)
		return ret;

	ret = hdd_check_private_wext_control(hdd_ctx, info);
	if (0 != ret)
		return ret;

	mac_handle = hdd_ctx->mac_handle;
	sub_cmd = wrqu->data.flags;
	num_args = wrqu->data.length;

	hdd_debug("Received length %d", wrqu->data.length);

	switch (sub_cmd) {
	case WE_IBSS_GET_PEER_INFO:
	{
		pr_info("Station ID = %d\n", apps_args[0]);
		hdd_wlan_get_ibss_peer_info(adapter, apps_args[0]);
	}
	break;

	case WE_P2P_NOA_CMD:
	{
		struct p2p_app_set_ps p2pNoA;

		if (adapter->device_mode != QDF_P2P_GO_MODE) {
			hdd_err("Setting NoA is not allowed in Device mode %s(%d)",
				hdd_device_mode_to_string(
					adapter->device_mode),
				adapter->device_mode);
			return -EINVAL;
		}

		p2pNoA.opp_ps = apps_args[0];
		p2pNoA.ctWindow = apps_args[1];
		p2pNoA.duration = apps_args[2];
		p2pNoA.interval = apps_args[3];
		p2pNoA.count = apps_args[4];
		p2pNoA.single_noa_duration = apps_args[5];
		p2pNoA.psSelection = apps_args[6];

		hdd_debug("P2P_NOA_ATTR:oppPS %d ctWindow %d duration %d interval %d count %d single noa duration %d PsSelection %x",
			   apps_args[0], apps_args[1], apps_args[2],
			   apps_args[3], apps_args[4],
			   apps_args[5], apps_args[6]);

		hdd_set_p2p_ps(dev, &p2pNoA);

	}
	break;

	case WE_MTRACE_SELECTIVE_MODULE_LOG_ENABLE_CMD:
	{
		hdd_debug("SELECTIVE_MODULE_LOG %d arg1 %d arg2",
			   apps_args[0], apps_args[1]);
		qdf_trace_enable(apps_args[0], apps_args[1]);
	}
	break;

	case WE_MTRACE_DUMP_CMD:
	{
		hdd_debug("MTRACE_DUMP code %d session %d count %d bitmask_of_module %d ",
			   apps_args[0], apps_args[1],
			   apps_args[2], apps_args[3]);
		qdf_trace_dump_all((void *)mac_handle, apps_args[0],
				   apps_args[1], apps_args[2],
				   apps_args[3]);

	}
	break;

	case WE_POLICY_MANAGER_CINFO_CMD:
	{
		struct policy_mgr_conc_connection_info *conn_info;
		uint32_t i = 0, len = 0;

		hdd_info("<iwpriv wlan0 pm_cinfo> is called");
		conn_info = policy_mgr_get_conn_info(&len);
		pr_info("+--------------------------+\n");
		for (i = 0; i < len; i++) {
			pr_info("|table_index[%d]\t\t\n", i);
			pr_info("|\t|vdev_id - %-10d|\n", conn_info->vdev_id);
			pr_info("|\t|chan    - %-10d|\n", conn_info->chan);
			pr_info("|\t|bw      - %-10d|\n", conn_info->bw);
			pr_info("|\t|mode    - %-10d|\n", conn_info->mode);
			pr_info("|\t|mac     - %-10d|\n", conn_info->mac);
			pr_info("|\t|in_use  - %-10d|\n", conn_info->in_use);
			pr_info("+--------------------------+\n");
			conn_info++;
		}
	}
	break;

	case WE_UNIT_TEST_CMD:
	{
		QDF_STATUS status;

		if ((apps_args[0] < WLAN_MODULE_ID_MIN) ||
		    (apps_args[0] >= WLAN_MODULE_ID_MAX)) {
			hdd_err("Invalid MODULE ID %d", apps_args[0]);
			return -EINVAL;
		}
		if ((apps_args[1] >= (WMA_MAX_NUM_ARGS)) ||
		    (apps_args[1] < 0)) {
			hdd_err("Too Many/Few args %d", apps_args[1]);
			return -EINVAL;
		}
		status = sme_send_unit_test_cmd(adapter->session_id,
						apps_args[0],
						apps_args[1],
						&apps_args[2]);
		if (QDF_STATUS_SUCCESS != status) {
			hdd_err("sme_send_unit_test_cmd returned %d", status);
			return -EINVAL;
		}
	}
	break;
#ifdef WLAN_FEATURE_GPIO_LED_FLASHING
	case WE_LED_FLASHING_PARAM:
	{
		int i;

		if (num_args != 4) {
			hdd_err("gpio_control: 4 parameters are required");
			return -EINVAL;
		}
		for (i = 0; i < num_args; i++) {
			if (apps_args[i] >= 0x7fffffff) {
				hdd_err("gpio_control: parameter should be less than 0x7fffffff");
				return -EINVAL;
			}
		}
		sme_set_led_flashing(mac_handle,
				     0, apps_args[0], apps_args[1]);
		sme_set_led_flashing(mac_handle,
				     1, apps_args[2], apps_args[3]);
	}
	break;
#endif
	case WE_SET_PKTLOG:
	{
		int ret;

		if (num_args < 1 || num_args > 2) {
			hdd_err("pktlog: either 1 or 2 parameters are required");
			return -EINVAL;
		}

		ret = hdd_process_pktlog_command(hdd_ctx, apps_args[0],
						   apps_args[1]);
		if (ret)
			return ret;
		break;
	}
	case WE_MAC_PWR_DEBUG_CMD:
	{
		struct sir_mac_pwr_dbg_cmd mac_pwr_dbg_args;
		int i, j;

		if (num_args < 3) {
			hdd_err("number of arguments can't be null %d",
				num_args);
			return -EINVAL;
		}
		if (num_args - 3 != apps_args[2]) {
			hdd_err("arg list of size %d doesn't match num_args %d",
				num_args-3, apps_args[2]);
			return -EINVAL;
		}
		if ((apps_args[1] < WLAN_MODULE_ID_MIN) ||
		    (apps_args[1] >= WLAN_MODULE_ID_MAX)) {
			hdd_err("Invalid MODULE ID %d", apps_args[1]);
			return -EINVAL;
		}
		if (apps_args[2] > (MAX_POWER_DBG_ARGS_SUPPORTED)) {
			hdd_err("Too Many args %d", apps_args[2]);
			return -EINVAL;
		}
		mac_pwr_dbg_args.pdev_id = apps_args[0];
		mac_pwr_dbg_args.module_id = apps_args[1];
		mac_pwr_dbg_args.num_args = apps_args[2];

		for (i = 0, j = 3; i < mac_pwr_dbg_args.num_args; i++, j++)
			mac_pwr_dbg_args.args[i] = apps_args[j];

		if (QDF_STATUS_SUCCESS !=
			sme_process_mac_pwr_dbg_cmd(mac_handle,
						    adapter->session_id,
						    &mac_pwr_dbg_args)) {
			return -EINVAL;
		}
	}
	break;
	case WE_POLICY_MANAGER_CLIST_CMD:
	case WE_POLICY_MANAGER_DLIST_CMD:
	case WE_POLICY_MANAGER_ULIST_CMD:
	case WE_POLICY_MANAGER_DBS_CMD:
	case WE_POLICY_MANAGER_PCL_CMD:
	case WE_POLICY_SET_HW_MODE_CMD:
	case WE_POLICY_MANAGER_QUERY_ACTION_CMD:
	case WE_POLICY_MANAGER_QUERY_ALLOW_CMD:
	case WE_POLICY_MANAGER_SCENARIO_CMD:
	{
		if (!hdd_ctx->config->is_unit_test_framework_enabled) {
			hdd_warn_rl("UT framework is disabled");
			return -EINVAL;
		}
		iw_get_policy_manager_ut_ops(hdd_ctx, adapter,
				sub_cmd, apps_args);
	}
	break;
	case WE_SET_CHAN_AVOID:
	{
		hdd_ch_avoid_unit_cmd(hdd_ctx, num_args, apps_args);
	}
	break;
	case WE_SET_TXRX_STATS:
	{
		ret = cds_get_datapath_handles(&soc, &pdev, &vdev,
					       adapter->session_id);

		if (ret != 0) {
			hdd_err("Invalid handles");
			break;
		}

		req.stats = apps_args[0];
		/* default value of secondary parameter is 0(mac_id) */
		req.mac_id = apps_args[1];

		hdd_debug("WE_SET_TXRX_STATS stats cmd: %d mac_id: %d",
			  req.stats, req.mac_id);
		if (apps_args[0] == CDP_TXRX_STATS_28) {
			if (sta_ctx->conn_info.uIsAuthenticated) {
				hdd_debug("ap mac addr: %pM",
					  (void *)&sta_ctx->conn_info.bssId);
				req.peer_addr =
					(char *)&sta_ctx->conn_info.bssId;
			}
		}
		ret = cdp_txrx_stats_request(soc, vdev, &req);
		break;
	}
	default:
	{
		hdd_err("Invalid IOCTL command %d", sub_cmd);
	}
	break;
	}
	hdd_exit();
	return 0;
}

/**
 * iw_hdd_set_var_ints_getnone() - set var ints getnone callback
 * @dev: pointer to net_device structure
 * @info: pointer to iw_request_info structure
 * @wrqu: pointer to iwreq_data
 * @extra; extra
 *
 * Return: 0 on success, error number otherwise
 *
 */
static int iw_hdd_set_var_ints_getnone(struct net_device *dev,
				       struct iw_request_info *info,
				       union iwreq_data *wrqu, char *extra)
{
	union iwreq_data u_priv_wrqu;
	int apps_args[MAX_VAR_ARGS] = {0};
	int ret, num_args;

	if (!capable(CAP_NET_ADMIN)) {
		hdd_err("permission check failed");
		return -EPERM;
	}

	/* Helper function to get iwreq_data with compat handling. */
	if (hdd_priv_get_data(&u_priv_wrqu.data, wrqu))
		return -EINVAL;

	if (NULL == u_priv_wrqu.data.pointer) {
		hdd_err("NULL data pointer");
		return -EINVAL;
	}

	num_args = u_priv_wrqu.data.length;
	if (num_args > MAX_VAR_ARGS)
		num_args = MAX_VAR_ARGS;

	if (copy_from_user(apps_args, u_priv_wrqu.data.pointer,
			  (sizeof(int)) * num_args)) {
		hdd_err("failed to copy data from user buffer");
		return -EFAULT;
	}

	cds_ssr_protect(__func__);
	ret = __iw_set_var_ints_getnone(dev, info, &u_priv_wrqu,
					(char *)&apps_args);
	cds_ssr_unprotect(__func__);
	return ret;
}

/**
 * iw_set_var_ints_getnone - Generic "set many" private ioctl handler
 * @dev: device upon which the ioctl was received
 * @info: ioctl request information
 * @wrqu: ioctl request data
 * @extra: ioctl extra data
 *
 * This is a generic handler for private ioctls which take multiple
 * arguments.  Note that this implementation is also somewhat unique
 * in that it is shared by both STA-mode and SAP-mode interfaces.
 *
 * Return: 0 on success, non-zero on error
 */
int iw_set_var_ints_getnone(struct net_device *dev,
			    struct iw_request_info *info,
			    union iwreq_data *wrqu, char *extra)
{
	int ret;

	cds_ssr_protect(__func__);
	ret = __iw_set_var_ints_getnone(dev, info, wrqu, extra);
	cds_ssr_unprotect(__func__);
	return ret;
}

/**
 * iw_add_tspec - Add TSpec private ioctl handler
 * @dev: device upon which the ioctl was received
 * @info: ioctl request information
 * @wrqu: ioctl request data
 * @extra: ioctl extra data
 *
 * Return: 0 on success, non-zero on error
 */
static int __iw_add_tspec(struct net_device *dev, struct iw_request_info *info,
			  union iwreq_data *wrqu, char *extra)
{
	struct hdd_adapter *adapter = WLAN_HDD_GET_PRIV_PTR(dev);
	struct hdd_station_ctx *sta_ctx = WLAN_HDD_GET_STATION_CTX_PTR(adapter);
	hdd_wlan_wmm_status_e *pStatus = (hdd_wlan_wmm_status_e *) extra;
	int params[HDD_WLAN_WMM_PARAM_COUNT];
	struct sme_qos_wmmtspecinfo tSpec;
	uint32_t handle;
	struct iw_point s_priv_data;
	struct hdd_context *hdd_ctx;
	int ret;

	hdd_enter_dev(dev);

	hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	ret = wlan_hdd_validate_context(hdd_ctx);
	if (0 != ret)
		return ret;

	ret = hdd_check_private_wext_control(hdd_ctx, info);
	if (0 != ret)
		return ret;

	/* make sure the application is sufficiently priviledged */
	/* note that the kernel will do this for "set" ioctls, but since */
	/* this ioctl wants to return status to user space it must be */
	/* defined as a "get" ioctl */
	if (!capable(CAP_NET_ADMIN))
		return -EPERM;

	/* we must be associated in order to add a tspec */
	if (eConnectionState_Associated != sta_ctx->conn_info.connState) {
		*pStatus = HDD_WLAN_WMM_STATUS_SETUP_FAILED_BAD_PARAM;
		return 0;
	}
	/* since we are defined to be a "get" ioctl, and since the number */
	/* of params exceeds the number of params that wireless extensions */
	/* will pass down in the iwreq_data, we must copy the "set" params. */
	/* We must handle the compat for iwreq_data in 32U/64K environment. */

	/* helper function to get iwreq_data with compat handling. */
	if (hdd_priv_get_data(&s_priv_data, wrqu)) {
		*pStatus = HDD_WLAN_WMM_STATUS_SETUP_FAILED_BAD_PARAM;
		return 0;
	}
	/* make sure all params are correctly passed to function */
	if ((NULL == s_priv_data.pointer) ||
	    (HDD_WLAN_WMM_PARAM_COUNT != s_priv_data.length)) {
		*pStatus = HDD_WLAN_WMM_STATUS_SETUP_FAILED_BAD_PARAM;
		return 0;
	}
	/* from user space ourselves */
	if (copy_from_user(&params, s_priv_data.pointer, sizeof(params))) {
		/* hmmm, can't get them */
		return -EIO;
	}
	/* clear the tspec */
	memset(&tSpec, 0, sizeof(tSpec));

	/* validate the handle */
	handle = params[HDD_WLAN_WMM_PARAM_HANDLE];
	if (HDD_WMM_HANDLE_IMPLICIT == handle) {
		/* that one is reserved */
		*pStatus = HDD_WLAN_WMM_STATUS_SETUP_FAILED_BAD_PARAM;
		return 0;
	}
	/* validate the TID */
	if (params[HDD_WLAN_WMM_PARAM_TID] > 7) {
		/* out of range */
		*pStatus = HDD_WLAN_WMM_STATUS_SETUP_FAILED_BAD_PARAM;
		return 0;
	}
	tSpec.ts_info.tid = params[HDD_WLAN_WMM_PARAM_TID];

	/* validate the direction */
	switch (params[HDD_WLAN_WMM_PARAM_DIRECTION]) {
	case HDD_WLAN_WMM_DIRECTION_UPSTREAM:
		tSpec.ts_info.direction = SME_QOS_WMM_TS_DIR_UPLINK;
		break;

	case HDD_WLAN_WMM_DIRECTION_DOWNSTREAM:
		tSpec.ts_info.direction = SME_QOS_WMM_TS_DIR_DOWNLINK;
		break;

	case HDD_WLAN_WMM_DIRECTION_BIDIRECTIONAL:
		tSpec.ts_info.direction = SME_QOS_WMM_TS_DIR_BOTH;
		break;

	default:
		/* unknown */
		*pStatus = HDD_WLAN_WMM_STATUS_SETUP_FAILED_BAD_PARAM;
		return 0;
	}

	tSpec.ts_info.psb = params[HDD_WLAN_WMM_PARAM_APSD];

	/* validate the user priority */
	if (params[HDD_WLAN_WMM_PARAM_USER_PRIORITY] >= SME_QOS_WMM_UP_MAX) {
		/* out of range */
		*pStatus = HDD_WLAN_WMM_STATUS_SETUP_FAILED_BAD_PARAM;
		return 0;
	}
	tSpec.ts_info.up = params[HDD_WLAN_WMM_PARAM_USER_PRIORITY];
	if (0 > tSpec.ts_info.up || SME_QOS_WMM_UP_MAX < tSpec.ts_info.up) {
		hdd_err("***ts_info.up out of bounds***");
		return 0;
	}

	hdd_debug("TS_INFO PSB %d UP %d !!!",
		  tSpec.ts_info.psb, tSpec.ts_info.up);

	tSpec.nominal_msdu_size = params[HDD_WLAN_WMM_PARAM_NOMINAL_MSDU_SIZE];
	tSpec.maximum_msdu_size = params[HDD_WLAN_WMM_PARAM_MAXIMUM_MSDU_SIZE];
	tSpec.min_data_rate = params[HDD_WLAN_WMM_PARAM_MINIMUM_DATA_RATE];
	tSpec.mean_data_rate = params[HDD_WLAN_WMM_PARAM_MEAN_DATA_RATE];
	tSpec.peak_data_rate = params[HDD_WLAN_WMM_PARAM_PEAK_DATA_RATE];
	tSpec.max_burst_size = params[HDD_WLAN_WMM_PARAM_MAX_BURST_SIZE];
	tSpec.min_phy_rate = params[HDD_WLAN_WMM_PARAM_MINIMUM_PHY_RATE];
	tSpec.surplus_bw_allowance =
		params[HDD_WLAN_WMM_PARAM_SURPLUS_BANDWIDTH_ALLOWANCE];
	tSpec.min_service_interval =
		params[HDD_WLAN_WMM_PARAM_SERVICE_INTERVAL];
	tSpec.max_service_interval =
		params[HDD_WLAN_WMM_PARAM_MAX_SERVICE_INTERVAL];
	tSpec.suspension_interval =
		params[HDD_WLAN_WMM_PARAM_SUSPENSION_INTERVAL];
	tSpec.inactivity_interval =
		params[HDD_WLAN_WMM_PARAM_INACTIVITY_INTERVAL];

	tSpec.ts_info.burst_size_defn =
		params[HDD_WLAN_WMM_PARAM_BURST_SIZE_DEFN];

	/* validate the ts info ack policy */
	switch (params[HDD_WLAN_WMM_PARAM_ACK_POLICY]) {
	case HDD_WLAN_WMM_TS_INFO_ACK_POLICY_NORMAL_ACK:
		tSpec.ts_info.ack_policy = SME_QOS_WMM_TS_ACK_POLICY_NORMAL_ACK;
		break;

	case HDD_WLAN_WMM_TS_INFO_ACK_POLICY_HT_IMMEDIATE_BLOCK_ACK:
		tSpec.ts_info.ack_policy =
			SME_QOS_WMM_TS_ACK_POLICY_HT_IMMEDIATE_BLOCK_ACK;
		break;

	default:
		/* unknown */
		*pStatus = HDD_WLAN_WMM_STATUS_SETUP_FAILED_BAD_PARAM;
		return 0;
	}

	*pStatus = hdd_wmm_addts(adapter, handle, &tSpec);
	hdd_exit();
	return 0;
}

static int iw_add_tspec(struct net_device *dev,
			struct iw_request_info *info,
			union iwreq_data *wrqu, char *extra)
{
	int ret;

	cds_ssr_protect(__func__);
	ret = __iw_add_tspec(dev, info, wrqu, extra);
	cds_ssr_unprotect(__func__);

	return ret;
}

/**
 * iw_del_tspec - Delete TSpec private ioctl handler
 * @dev: device upon which the ioctl was received
 * @info: ioctl request information
 * @wrqu: ioctl request data
 * @extra: ioctl extra data
 *
 * Return: 0 on success, non-zero on error
 */
static int __iw_del_tspec(struct net_device *dev, struct iw_request_info *info,
			  union iwreq_data *wrqu, char *extra)
{
	struct hdd_adapter *adapter = WLAN_HDD_GET_PRIV_PTR(dev);
	struct hdd_context *hdd_ctx;
	int *params = (int *)extra;
	hdd_wlan_wmm_status_e *pStatus = (hdd_wlan_wmm_status_e *) extra;
	uint32_t handle;
	int ret;

	hdd_enter_dev(dev);

	hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	ret = wlan_hdd_validate_context(hdd_ctx);
	if (0 != ret)
		return ret;

	ret = hdd_check_private_wext_control(hdd_ctx, info);
	if (0 != ret)
		return ret;

	/* make sure the application is sufficiently priviledged */
	/* note that the kernel will do this for "set" ioctls, but since */
	/* this ioctl wants to return status to user space it must be */
	/* defined as a "get" ioctl */
	if (!capable(CAP_NET_ADMIN))
		return -EPERM;

	/* although we are defined to be a "get" ioctl, the params we require */
	/* will fit in the iwreq_data, therefore unlike iw_add_tspec() there */
	/* is no need to copy the params from user space */

	/* validate the handle */
	handle = params[HDD_WLAN_WMM_PARAM_HANDLE];
	if (HDD_WMM_HANDLE_IMPLICIT == handle) {
		/* that one is reserved */
		*pStatus = HDD_WLAN_WMM_STATUS_SETUP_FAILED_BAD_PARAM;
		return 0;
	}

	*pStatus = hdd_wmm_delts(adapter, handle);
	hdd_exit();
	return 0;
}

static int iw_del_tspec(struct net_device *dev,
			struct iw_request_info *info,
			union iwreq_data *wrqu, char *extra)
{
	int ret;

	cds_ssr_protect(__func__);
	ret = __iw_del_tspec(dev, info, wrqu, extra);
	cds_ssr_unprotect(__func__);

	return ret;
}

/**
 * iw_get_tspec - Get TSpec private ioctl handler
 * @dev: device upon which the ioctl was received
 * @info: ioctl request information
 * @wrqu: ioctl request data
 * @extra: ioctl extra data
 *
 * Return: 0 on success, non-zero on error
 */
static int __iw_get_tspec(struct net_device *dev, struct iw_request_info *info,
			  union iwreq_data *wrqu, char *extra)
{
	struct hdd_adapter *adapter = WLAN_HDD_GET_PRIV_PTR(dev);
	struct hdd_context *hdd_ctx;
	int *params = (int *)extra;
	hdd_wlan_wmm_status_e *pStatus = (hdd_wlan_wmm_status_e *) extra;
	uint32_t handle;
	int ret;

	hdd_enter_dev(dev);

	hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	ret = wlan_hdd_validate_context(hdd_ctx);
	if (0 != ret)
		return ret;

	ret = hdd_check_private_wext_control(hdd_ctx, info);
	if (0 != ret)
		return ret;

	/* although we are defined to be a "get" ioctl, the params we require */
	/* will fit in the iwreq_data, therefore unlike iw_add_tspec() there */
	/* is no need to copy the params from user space */

	/* validate the handle */
	handle = params[HDD_WLAN_WMM_PARAM_HANDLE];
	if (HDD_WMM_HANDLE_IMPLICIT == handle) {
		/* that one is reserved */
		*pStatus = HDD_WLAN_WMM_STATUS_SETUP_FAILED_BAD_PARAM;
		return 0;
	}

	*pStatus = hdd_wmm_checkts(adapter, handle);
	hdd_exit();
	return 0;
}

static int iw_get_tspec(struct net_device *dev,
			struct iw_request_info *info,
			union iwreq_data *wrqu, char *extra)
{
	int ret;

	cds_ssr_protect(__func__);
	ret = __iw_get_tspec(dev, info, wrqu, extra);
	cds_ssr_unprotect(__func__);

	return ret;
}

/**
 * iw_set_fties - Set FT IEs private ioctl handler
 * @dev: device upon which the ioctl was received
 * @info: ioctl request information
 * @wrqu: ioctl request data
 * @extra: ioctl extra data
 *
 * Each time the supplicant has the auth_request or reassoc request
 * IEs ready they are pushed to the driver. The driver will in turn
 * use it to send out the auth req and reassoc req for 11r FT Assoc.
 *
 * Return: 0 on success, non-zero on error
 */
static int __iw_set_fties(struct net_device *dev, struct iw_request_info *info,
			  union iwreq_data *wrqu, char *extra)
{
	struct hdd_adapter *adapter = WLAN_HDD_GET_PRIV_PTR(dev);
	struct hdd_station_ctx *sta_ctx = WLAN_HDD_GET_STATION_CTX_PTR(adapter);
	struct hdd_context *hdd_ctx;
	int ret;

	hdd_enter_dev(dev);

	hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	ret = wlan_hdd_validate_context(hdd_ctx);
	if (0 != ret)
		return ret;

	ret = hdd_check_private_wext_control(hdd_ctx, info);
	if (0 != ret)
		return ret;

	if (!wrqu->data.length) {
		hdd_err("called with 0 length IEs");
		return -EINVAL;
	}
	if (wrqu->data.pointer == NULL) {
		hdd_err("called with NULL IE");
		return -EINVAL;
	}
	/* Added for debug on reception of Re-assoc Req. */
	if (eConnectionState_Associated != sta_ctx->conn_info.connState) {
		hdd_debug("Called with Ie of length = %d when not associated",
		       wrqu->data.length);
		hdd_debug("Should be Re-assoc Req IEs");
	}
	hdd_debug("called with Ie of length = %d", wrqu->data.length);

	/* Pass the received FT IEs to SME */
	sme_set_ft_ies(hdd_ctx->mac_handle, adapter->session_id,
		       extra, wrqu->data.length);
	hdd_exit();
	return 0;
}

static int iw_set_fties(struct net_device *dev,
			struct iw_request_info *info,
			union iwreq_data *wrqu, char *extra)
{
	int ret;

	cds_ssr_protect(__func__);
	ret = __iw_set_fties(dev, info, wrqu, extra);
	cds_ssr_unprotect(__func__);

	return ret;
}

/**
 * iw_set_dynamic_mcbc_filter() - Set Dynamic MCBC Filter ioctl handler
 * @dev: device upon which the ioctl was received
 * @info: ioctl request information
 * @wrqu: ioctl request data
 * @extra: ioctl extra data
 *
 * This IOCTL is OBSOLETE as of Jan 30, 2017. We are leaving it here for the
 * time being to provide guidance in migrating to standard APIs.
 *
 * Return: 0 on success, non-zero on error
 */
static int iw_set_dynamic_mcbc_filter(struct net_device *dev,
				      struct iw_request_info *info,
				      union iwreq_data *wrqu,
				      char *extra)
{
	hdd_err("\n"
		"setMCBCFilter is obsolete. Use the following instead:\n"
		"Configure multicast filtering via the ‘ip’ command.\n"
		"\tip maddr add 11:22:33:44:55:66 dev wlan0 # allow traffic to address\n"
		"\tip maddr del 11:22:33:44:55:66 dev wlan0 # undo allow\n"
		"Configure broadcast filtering via ini item, 'g_enable_non_arp_bc_hw_filter.'\n"
		"\tg_enable_non_arp_bc_hw_filter=1 # drop all non-ARP broadcast traffic\n"
		"\tg_enable_non_arp_bc_hw_filter=0 # allow all broadcast traffic");
	return -EINVAL;
}

/**
 * iw_set_host_offload - Set host offload ioctl handler
 * @dev: device upon which the ioctl was received
 * @info: ioctl request information
 * @wrqu: ioctl request data
 * @extra: ioctl extra data
 *
 * Return: 0 on success, non-zero on error
 */
static int __iw_set_host_offload(struct net_device *dev,
				 struct iw_request_info *info,
				 union iwreq_data *wrqu, char *extra)
{
	struct hdd_adapter *adapter = WLAN_HDD_GET_PRIV_PTR(dev);
	struct host_offload_req *pRequest = (struct host_offload_req *) extra;
	tSirHostOffloadReq offloadRequest;
	struct hdd_context *hdd_ctx;
	int ret;

	hdd_enter_dev(dev);

	hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	ret = wlan_hdd_validate_context(hdd_ctx);
	if (0 != ret)
		return ret;

	ret = hdd_check_private_wext_control(hdd_ctx, info);
	if (0 != ret)
		return ret;

	if (!hdd_conn_is_connected(WLAN_HDD_GET_STATION_CTX_PTR(adapter))) {
		hdd_err("dev is not in CONNECTED state, ignore!!!");
		return -EINVAL;
	}

	/* Debug display of request components. */
	switch (pRequest->offloadType) {
	case WLAN_IPV4_ARP_REPLY_OFFLOAD:
		hdd_debug("Host offload request: ARP reply");
		switch (pRequest->enableOrDisable) {
		case WLAN_OFFLOAD_DISABLE:
			hdd_debug("   disable");
			break;
		case WLAN_OFFLOAD_ARP_AND_BC_FILTER_ENABLE:
			hdd_debug("   BC Filtering enable");
			/* fallthrough */
		case WLAN_OFFLOAD_ENABLE:
			hdd_debug("   ARP offload enable");
			hdd_debug("   IP address: %d.%d.%d.%d",
			       pRequest->params.hostIpv4Addr[0],
			       pRequest->params.hostIpv4Addr[1],
			       pRequest->params.hostIpv4Addr[2],
			       pRequest->params.hostIpv4Addr[3]);
		}
		break;

	case WLAN_IPV6_NEIGHBOR_DISCOVERY_OFFLOAD:
		hdd_debug("Host offload request: neighbor discovery");
		switch (pRequest->enableOrDisable) {
		case WLAN_OFFLOAD_DISABLE:
			hdd_debug("   disable");
			break;
		case WLAN_OFFLOAD_ENABLE:
			hdd_debug("   enable");
			hdd_debug("   IP address: %x:%x:%x:%x:%x:%x:%x:%x",
			       *(uint16_t *) (pRequest->params.hostIpv6Addr),
			       *(uint16_t *) (pRequest->params.hostIpv6Addr +
					      2),
			       *(uint16_t *) (pRequest->params.hostIpv6Addr +
					      4),
			       *(uint16_t *) (pRequest->params.hostIpv6Addr +
					      6),
			       *(uint16_t *) (pRequest->params.hostIpv6Addr +
					      8),
			       *(uint16_t *) (pRequest->params.hostIpv6Addr +
					      10),
			       *(uint16_t *) (pRequest->params.hostIpv6Addr +
					      12),
			       *(uint16_t *) (pRequest->params.hostIpv6Addr +
					      14));
		}
	}

	qdf_mem_zero(&offloadRequest, sizeof(offloadRequest));
	offloadRequest.offloadType = pRequest->offloadType;
	offloadRequest.enableOrDisable = pRequest->enableOrDisable;
	qdf_mem_copy(&offloadRequest.params, &pRequest->params,
		     sizeof(pRequest->params));
	qdf_mem_copy(&offloadRequest.bssid, &pRequest->bssId.bytes,
		     QDF_MAC_ADDR_SIZE);

	if (QDF_STATUS_SUCCESS !=
	    sme_set_host_offload(hdd_ctx->mac_handle,
				 adapter->session_id, &offloadRequest)) {
		hdd_err("Failure to execute host offload request");
		return -EINVAL;
	}
	hdd_exit();
	return 0;
}

static int iw_set_host_offload(struct net_device *dev,
			       struct iw_request_info *info,
			       union iwreq_data *wrqu, char *extra)
{
	int ret;

	cds_ssr_protect(__func__);
	ret = __iw_set_host_offload(dev, info, wrqu, extra);
	cds_ssr_unprotect(__func__);

	return ret;
}

/**
 * iw_set_keepalive_params - Set keepalive params ioctl handler
 * @dev: device upon which the ioctl was received
 * @info: ioctl request information
 * @wrqu: ioctl request data
 * @extra: ioctl extra data
 *
 * Return: 0 on success, non-zero on error
 */
static int __iw_set_keepalive_params(struct net_device *dev,
				     struct iw_request_info *info,
				     union iwreq_data *wrqu, char *extra)
{
	struct hdd_adapter *adapter = WLAN_HDD_GET_PRIV_PTR(dev);
	tpSirKeepAliveReq request = (tpSirKeepAliveReq) extra;
	struct hdd_context *hdd_ctx;
	int ret;

	hdd_enter_dev(dev);

	hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	ret = wlan_hdd_validate_context(hdd_ctx);
	if (0 != ret)
		return ret;

	ret = hdd_check_private_wext_control(hdd_ctx, info);
	if (0 != ret)
		return ret;

	if (wrqu->data.length != sizeof(*request)) {
		hdd_err("Invalid length %d", wrqu->data.length);
		return -EINVAL;
	}

	if (request->timePeriod > cfg_max(CFG_INFRA_STA_KEEP_ALIVE_PERIOD)) {
		hdd_err("Value of timePeriod %d exceed Max limit %d",
			request->timePeriod,
			cfg_max(CFG_INFRA_STA_KEEP_ALIVE_PERIOD));
		return -EINVAL;
	}

	/* Debug display of request components. */
	hdd_debug("Set Keep Alive Request : TimePeriod %d size %zu",
		request->timePeriod, sizeof(tSirKeepAliveReq));

	switch (request->packetType) {
	case WLAN_KEEP_ALIVE_NULL_PKT:
		hdd_debug("Keep Alive Request: Tx NULL");
		break;

	case WLAN_KEEP_ALIVE_UNSOLICIT_ARP_RSP:
		hdd_debug("Keep Alive Request: Tx UnSolicited ARP RSP");

		hdd_debug("Host IP address: %d.%d.%d.%d",
		       request->hostIpv4Addr[0], request->hostIpv4Addr[1],
		       request->hostIpv4Addr[2], request->hostIpv4Addr[3]);

		hdd_debug("Dest IP address: %d.%d.%d.%d",
		       request->destIpv4Addr[0], request->destIpv4Addr[1],
		       request->destIpv4Addr[2], request->destIpv4Addr[3]);

		hdd_debug("Dest MAC address: "MAC_ADDRESS_STR,
		       MAC_ADDR_ARRAY(request->dest_macaddr.bytes));
		break;
	}

	hdd_debug("Keep alive period  %d", request->timePeriod);

	if (QDF_STATUS_SUCCESS !=
	    sme_set_keep_alive(hdd_ctx->mac_handle,
			       adapter->session_id, request)) {
		hdd_err("Failure to execute Keep Alive");
		return -EINVAL;
	}
	hdd_exit();
	return 0;
}

static int iw_set_keepalive_params(struct net_device *dev,
				   struct iw_request_info *info,
				   union iwreq_data *wrqu,
				   char *extra)
{
	int ret;

	cds_ssr_protect(__func__);
	ret = __iw_set_keepalive_params(dev, info, wrqu, extra);
	cds_ssr_unprotect(__func__);

	return ret;
}

#ifdef WLAN_FEATURE_PACKET_FILTERING
/**
 * validate_packet_filter_params_size() - Validate the size of the params rcvd
 * @priv_data: Pointer to the priv data from user space
 * @request: Pointer to the struct containing the copied data from user space
 *
 * Return: False on invalid length, true otherwise
 */
static bool validate_packet_filter_params_size(struct pkt_filter_cfg *request,
					       uint16_t length)
{
	int max_params_size, rcvd_params_size;

	max_params_size = HDD_MAX_CMP_PER_PACKET_FILTER *
					sizeof(struct pkt_filter_param_cfg);

	if (length < sizeof(struct pkt_filter_cfg) - max_params_size) {
		hdd_err("Less than minimum number of arguments needed");
		return false;
	}

	rcvd_params_size = request->num_params *
					sizeof(struct pkt_filter_param_cfg);

	if (length != sizeof(struct pkt_filter_cfg) -
					max_params_size + rcvd_params_size) {
		hdd_err("Arguments do not match the number of params provided");
		return false;
	}

	return true;
}

/**
 * __iw_set_packet_filter_params() - set packet filter parameters in target
 * @dev: Pointer to netdev
 * @info: Pointer to iw request info
 * @wrqu: Pointer to data
 * @extra: Pointer to extra data
 *
 * Return: 0 on success, non-zero on error
 */
static int __iw_set_packet_filter_params(struct net_device *dev,
				       struct iw_request_info *info,
				       union iwreq_data *wrqu, char *extra)
{
	int ret;
	struct hdd_context *hdd_ctx;
	struct iw_point priv_data;
	struct hdd_adapter *adapter = WLAN_HDD_GET_PRIV_PTR(dev);
	struct pkt_filter_cfg *request = NULL;

	if (!capable(CAP_NET_ADMIN)) {
		hdd_err("permission check failed");
		return -EPERM;
	}

	hdd_enter_dev(dev);

	hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	ret = wlan_hdd_validate_context(hdd_ctx);
	if (0 != ret)
		return ret;

	ret = hdd_check_private_wext_control(hdd_ctx, info);
	if (0 != ret)
		return ret;

	if (hdd_priv_get_data(&priv_data, wrqu)) {
		hdd_err("failed to get priv data");
		return -EINVAL;
	}

	if ((NULL == priv_data.pointer) || (0 == priv_data.length)) {
		hdd_err("invalid priv data %pK or invalid priv data length %d",
			priv_data.pointer, priv_data.length);
		return -EINVAL;
	}

	if (adapter->device_mode != QDF_STA_MODE) {
		hdd_err("Packet filter not supported for this mode :%d",
			adapter->device_mode);
		return -ENOTSUPP;
	}

	if (!hdd_conn_is_connected(WLAN_HDD_GET_STATION_CTX_PTR(adapter))) {
		hdd_err("Packet filter not supported in disconnected state");
		return -ENOTSUPP;
	}

	/* copy data using copy_from_user */
	request = mem_alloc_copy_from_user_helper(priv_data.pointer,
						   priv_data.length);

	if (NULL == request) {
		hdd_err("mem_alloc_copy_from_user_helper fail");
		return -ENOMEM;
	}

	if (!validate_packet_filter_params_size(request, priv_data.length)) {
		hdd_err("Invalid priv data length %d", priv_data.length);
		qdf_mem_free(request);
		return -EINVAL;
	}

	if (request->filter_action == HDD_RCV_FILTER_SET)
		hdd_ctx->user_configured_pkt_filter_rules |=
					1 << request->filter_id;
	else if (request->filter_action == HDD_RCV_FILTER_CLEAR)
		hdd_ctx->user_configured_pkt_filter_rules &=
					~(1 << request->filter_id);

	ret = wlan_hdd_set_filter(hdd_ctx, request, adapter->session_id);

	qdf_mem_free(request);
	hdd_exit();
	return ret;
}

/**
 * iw_set_packet_filter_params() - set packet filter parameters in target
 * @dev: Pointer to netdev
 * @info: Pointer to iw request info
 * @wrqu: Pointer to data
 * @extra: Pointer to extra data
 *
 * Return: 0 on success, non-zero on error
 */
static int iw_set_packet_filter_params(struct net_device *dev,
				       struct iw_request_info *info,
				       union iwreq_data *wrqu, char *extra)
{
	int ret;

	cds_ssr_protect(__func__);
	ret = __iw_set_packet_filter_params(dev, info, wrqu, extra);
	cds_ssr_unprotect(__func__);

	return ret;
}
#endif

#ifdef QCA_SUPPORT_CP_STATS
static int hdd_get_wlan_stats(struct hdd_adapter *adapter)
{
	return wlan_hdd_get_station_stats(adapter);
}
#else /* QCA_SUPPORT_CP_STATS */
struct hdd_statistics_priv {
	tCsrSummaryStatsInfo summary_stats;
	tCsrGlobalClassAStatsInfo class_a_stats;
	tCsrGlobalClassDStatsInfo class_d_stats;
};

/**
 * hdd_statistics_cb() - "Get statistics" callback function
 * @stats: statistics payload
 * @context: opaque context originally passed to SME.  HDD always passes
 *	a cookie for the request context
 *
 * Return: None
 */
static void hdd_statistics_cb(void *stats, void *context)
{
	struct osif_request *request;
	struct hdd_statistics_priv *priv;
	tCsrSummaryStatsInfo *summary_stats;
	tCsrGlobalClassAStatsInfo *class_a_stats;
	tCsrGlobalClassDStatsInfo *class_d_stats;

	request = osif_request_get(context);
	if (!request) {
		hdd_err("Obsolete request");
		return;
	}

	priv = osif_request_priv(request);

	summary_stats = (tCsrSummaryStatsInfo *)stats;
	priv->summary_stats = *summary_stats;

	class_a_stats = (tCsrGlobalClassAStatsInfo *)(summary_stats + 1);
	priv->class_a_stats = *class_a_stats;

	class_d_stats = (tCsrGlobalClassDStatsInfo *)(class_a_stats + 1);
	priv->class_d_stats = *class_d_stats;

	osif_request_complete(request);
	osif_request_put(request);
}

static int hdd_get_wlan_stats(struct hdd_adapter *adapter)
{
	int ret = 0;
	void *cookie;
	QDF_STATUS status;
	struct osif_request *request;
	struct hdd_station_ctx *sta_ctx;
	struct hdd_statistics_priv *priv;
	struct hdd_context *hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	static const struct osif_request_params params = {
		.priv_size = sizeof(*priv),
		.timeout_ms = WLAN_WAIT_TIME_STATS,
	};

	sta_ctx = WLAN_HDD_GET_STATION_CTX_PTR(adapter);
	request = osif_request_alloc(&params);
	if (!request) {
		hdd_warn("request allocation failed");
		return -EINVAL;
	}

	cookie = osif_request_cookie(request);
	status = sme_get_statistics(hdd_ctx->mac_handle, eCSR_HDD,
				    SME_SUMMARY_STATS |
				    SME_GLOBAL_CLASSA_STATS |
				    SME_GLOBAL_CLASSD_STATS,
				    hdd_statistics_cb,
				    sta_ctx->conn_info.staId[0],
				    cookie, adapter->session_id);

	if (QDF_STATUS_SUCCESS != status) {
		hdd_warn("Unable to retrieve SME statistics");
		goto put_request;
	}

	/* request was sent -- wait for the response */
	ret = osif_request_wait_for_response(request);
	if (ret) {
		hdd_err("Failed to wait for statistics, errno %d", ret);
		goto put_request;
	}

	/* update the adapter cache with the fresh results */
	priv = osif_request_priv(request);
	adapter->hdd_stats.summary_stat = priv->summary_stats;
	adapter->hdd_stats.class_a_stat = priv->class_a_stats;
	adapter->hdd_stats.class_d_stat = priv->class_d_stats;

put_request:
	/*
	 * either we never sent a request, we sent a request and
	 * received a response or we sent a request and timed out.
	 * regardless we are done with the request.
	 */
	osif_request_put(request);
	return ret;
}
#endif /* QCA_SUPPORT_CP_STATS */

static int __iw_get_statistics(struct net_device *dev,
			       struct iw_request_info *info,
			       union iwreq_data *wrqu, char *extra)
{
	int ret;
	char *p;
	int tlen;
	struct hdd_station_ctx *sta_ctx;
	tCsrSummaryStatsInfo *summary_stats;
	tCsrGlobalClassAStatsInfo *class_a_stats;
	tCsrGlobalClassDStatsInfo *class_d_stats;
	struct hdd_adapter *adapter = WLAN_HDD_GET_PRIV_PTR(dev);
	struct hdd_context *hdd_ctx = WLAN_HDD_GET_CTX(adapter);

	hdd_enter_dev(dev);

	ret = wlan_hdd_validate_context(hdd_ctx);
	if (0 != ret)
		return ret;

	ret = hdd_check_private_wext_control(hdd_ctx, info);
	if (0 != ret)
		return ret;

	sta_ctx = WLAN_HDD_GET_STATION_CTX_PTR(adapter);
	if (eConnectionState_Associated != sta_ctx->conn_info.connState) {
		wrqu->data.length = 0;
		return 0;
	}

	hdd_get_wlan_stats(adapter);

	summary_stats = &(adapter->hdd_stats.summary_stat);
	class_a_stats = &(adapter->hdd_stats.class_a_stat);
	class_d_stats = &(adapter->hdd_stats.class_d_stat);

	p = extra;
	tlen = 0;

	FILL_TLV(p, WLAN_STATS_RETRY_CNT,
		 sizeof(summary_stats->retry_cnt),
		 &(summary_stats->retry_cnt[0]), tlen);

	FILL_TLV(p, WLAN_STATS_MUL_RETRY_CNT,
		 sizeof(summary_stats->multiple_retry_cnt),
		 &(summary_stats->multiple_retry_cnt[0]), tlen);

	FILL_TLV(p, WLAN_STATS_TX_FRM_CNT,
		 sizeof(summary_stats->tx_frm_cnt),
		 &(summary_stats->tx_frm_cnt[0]), tlen);

	FILL_TLV(p, WLAN_STATS_RX_FRM_CNT,
		 sizeof(summary_stats->rx_frm_cnt),
		 &(summary_stats->rx_frm_cnt), tlen);

	FILL_TLV(p, WLAN_STATS_FRM_DUP_CNT,
		 sizeof(summary_stats->frm_dup_cnt),
		 &(summary_stats->frm_dup_cnt), tlen);

	FILL_TLV(p, WLAN_STATS_FAIL_CNT,
		 sizeof(summary_stats->fail_cnt),
		 &(summary_stats->fail_cnt[0]), tlen);

	FILL_TLV(p, WLAN_STATS_RTS_FAIL_CNT,
		 sizeof(summary_stats->rts_fail_cnt),
		 &(summary_stats->rts_fail_cnt), tlen);

	FILL_TLV(p, WLAN_STATS_ACK_FAIL_CNT,
		 sizeof(summary_stats->ack_fail_cnt),
		 &(summary_stats->ack_fail_cnt), tlen);

	FILL_TLV(p, WLAN_STATS_RTS_SUC_CNT,
		 sizeof(summary_stats->rts_succ_cnt),
		 &(summary_stats->rts_succ_cnt), tlen);

	FILL_TLV(p, WLAN_STATS_RX_DISCARD_CNT,
		 sizeof(summary_stats->rx_discard_cnt),
		 &(summary_stats->rx_discard_cnt), tlen);

	FILL_TLV(p, WLAN_STATS_RX_ERROR_CNT,
		 sizeof(summary_stats->rx_error_cnt),
		 &(summary_stats->rx_error_cnt), tlen);

	FILL_TLV(p, WLAN_STATS_TX_BYTE_CNT,
		 sizeof(class_d_stats->tx_uc_byte_cnt[0]),
		 &(class_d_stats->tx_uc_byte_cnt[0]), tlen);

	FILL_TLV(p, WLAN_STATS_RX_BYTE_CNT,
		 sizeof(class_d_stats->rx_byte_cnt),
		 &(class_d_stats->rx_byte_cnt), tlen);

	FILL_TLV(p, WLAN_STATS_RX_RATE,
		 sizeof(class_d_stats->rx_rate),
		 &(class_d_stats->rx_rate), tlen);

	/* Transmit rate, in units of 500 kbit/sec */
	FILL_TLV(p, WLAN_STATS_TX_RATE,
		 sizeof(class_a_stats->tx_rate),
		 &(class_a_stats->tx_rate), tlen);

	FILL_TLV(p, WLAN_STATS_RX_UC_BYTE_CNT,
		 sizeof(class_d_stats->rx_uc_byte_cnt[0]),
		 &(class_d_stats->rx_uc_byte_cnt[0]), tlen);
	FILL_TLV(p, WLAN_STATS_RX_MC_BYTE_CNT,
		 sizeof(class_d_stats->rx_mc_byte_cnt),
		 &(class_d_stats->rx_mc_byte_cnt), tlen);
	FILL_TLV(p, WLAN_STATS_RX_BC_BYTE_CNT,
		 sizeof(class_d_stats->rx_bc_byte_cnt),
		 &(class_d_stats->rx_bc_byte_cnt), tlen);
	FILL_TLV(p, WLAN_STATS_TX_UC_BYTE_CNT,
		 sizeof(class_d_stats->tx_uc_byte_cnt[0]),
		 &(class_d_stats->tx_uc_byte_cnt[0]), tlen);
	FILL_TLV(p, WLAN_STATS_TX_MC_BYTE_CNT,
		 sizeof(class_d_stats->tx_mc_byte_cnt),
		 &(class_d_stats->tx_mc_byte_cnt), tlen);
	FILL_TLV(p, WLAN_STATS_TX_BC_BYTE_CNT,
		 sizeof(class_d_stats->tx_bc_byte_cnt),
		 &(class_d_stats->tx_bc_byte_cnt), tlen);

	wrqu->data.length = tlen;

	hdd_exit();

	return 0;
}

static int iw_get_statistics(struct net_device *dev,
			     struct iw_request_info *info,
			     union iwreq_data *wrqu, char *extra)
{
	int ret;

	cds_ssr_protect(__func__);
	ret = __iw_get_statistics(dev, info, wrqu, extra);
	cds_ssr_unprotect(__func__);

	return ret;
}

#ifdef FEATURE_WLAN_SCAN_PNO
/*Max Len for PNO notification*/
#define MAX_PNO_NOTIFY_LEN 100
static void found_pref_network_cb(struct wlan_objmgr_vdev *vdev,
	struct scan_event *event, void *args)
{
	struct vdev_osif_priv *osif_priv;
	struct wireless_dev *wdev;
	union iwreq_data wrqu;
	char buf[MAX_PNO_NOTIFY_LEN + 1];

	wlan_vdev_obj_lock(vdev);
	osif_priv = wlan_vdev_get_ospriv(vdev);
	wlan_vdev_obj_unlock(vdev);
	if (!osif_priv) {
		hdd_err("osif_priv is null");
		return;
	}

	wdev = osif_priv->wdev;
	if (!wdev) {
		hdd_err("wdev is null");
		return;
	}

	hdd_debug("A preferred network was found");

	/* create the event */
	qdf_mem_zero(&wrqu, sizeof(wrqu));
	qdf_mem_zero(buf, sizeof(buf));

	snprintf(buf, MAX_PNO_NOTIFY_LEN,
		 "QCOM: Found preferred network:");

	wrqu.data.pointer = buf;
	wrqu.data.length = strlen(buf);

	/* send the event */

	wireless_send_event(wdev->netdev, IWEVCUSTOM, &wrqu, buf);
}

/**
 * __iw_set_pno() - Preferred Network Offload ioctl handler
 * @dev: device upon which the ioctl was received
 * @info: ioctl request information
 * @wrqu: ioctl request data
 * @extra: ioctl extra data
 *
 * This function parses a Preferred Network Offload command
 * Input is string based and expected to be of the form:
 *
 * <enable(1) | disable(0)>
 * when enabling:
 * <number of networks>
 * for each network:
 *    <ssid_len> <ssid> <authentication> <encryption>
 *    <ch_num> <channel_list optional> <bcast_type> <rssi_threshold>
 * <scan_time (seconds)>
 * <scan_repeat_count (0 means indefinite)>
 * <suspend mode>
 *
 * e.g:
 * 1 2 4 test 0 0 3 1 6 11 2 40 5 test2 4 4 6 1 2 3 4 5 6 1 0 5 2 1
 *
 * this translates into:
 * -----------------------------
 * enable PNO
 * 2 networks
 * Network 1:
 *   test - with authentication type 0 and encryption type 0,
 *   search on 3 channels: 1 6 and 11,
 *   SSID bcast type is unknown (directed probe will be sent if
 *   AP not found) and must meet -40dBm RSSI
 * Network 2:
 *   test2 - with authentication type 4 and encryption type 4,
 *   search on 6 channels 1, 2, 3, 4, 5 and 6
 *   bcast type is non-bcast (directed probe will be sent)
 *   and must not meet any RSSI threshold
 *   scan every 5 seconds 2 times
 *   enable on suspend
 */
static int __iw_set_pno(struct net_device *dev,
			struct iw_request_info *info,
			union iwreq_data *wrqu, char *extra)
{
	struct hdd_adapter *adapter = WLAN_HDD_GET_PRIV_PTR(dev);
	struct hdd_context *hdd_ctx;
	uint8_t value;
	struct wlan_objmgr_vdev *vdev;
	struct wlan_objmgr_psoc *psoc;
	int ret = 0;
	int offset;
	char *ptr, *data;
	uint8_t i, j, params;
	QDF_STATUS status;
	size_t len;

	/* request is a large struct, so we make it static to avoid
	 * stack overflow.  This API is only invoked via ioctl, so it
	 * is serialized by the kernel rtnl_lock and hence does not
	 * need to be reentrant
	 */
	static struct pno_scan_req_params req;

	hdd_enter_dev(dev);

	hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	ret = wlan_hdd_validate_context(hdd_ctx);
	if (ret)
		return ret;

	ret = hdd_check_private_wext_control(hdd_ctx, info);
	if (0 != ret)
		return ret;

	vdev = wlan_objmgr_get_vdev_by_macaddr_from_pdev(hdd_ctx->pdev,
							 dev->dev_addr,
							 WLAN_LEGACY_MAC_ID);
	if (!vdev) {
		hdd_err("vdev object is NULL");
		return -EIO;
	}

	/* making sure argument string ends with '\0' */
	len = (wrqu->data.length + 1);
	data = qdf_mem_malloc(len);
	if (!data) {
		hdd_err("fail to allocate memory %zu", len);
		return -EINVAL;
	}
	qdf_mem_copy(data, extra, (len-1));
	ptr = data;

	hdd_debug("PNO data len %d data %s", wrqu->data.length, data);

	if (1 != sscanf(ptr, " %hhu %n", &value, &offset)) {
		hdd_err("PNO enable input is not valid %s", ptr);
		ret = -EINVAL;
		goto exit;
	}

	if (!value) {
		status = ucfg_scan_pno_stop(vdev);
		if (QDF_IS_STATUS_ERROR(status)) {
			hdd_err("Failed to disabled PNO");
			ret = -EINVAL;
		} else {
			hdd_debug("PNO scan disabled");
		}
		goto exit;
	}

	if (ucfg_scan_get_pno_in_progress(vdev)) {
		hdd_debug("pno is already in progress");
		ret = -EBUSY;
		goto exit;
	}

	ptr += offset;

	if (1 != sscanf(ptr, " %hhu %n", &value, &offset)) {
		hdd_err("PNO count input not valid %s", ptr);
		ret = -EINVAL;
		goto exit;
	}
	req.networks_cnt = value;

	hdd_debug("PNO enable networks count %d offset %d",
		 req.networks_cnt, offset);

	if ((0 == req.networks_cnt) ||
	    (req.networks_cnt > SCAN_PNO_MAX_SUPP_NETWORKS)) {
		hdd_err("Network count %d invalid",
			req.networks_cnt);
		ret = -EINVAL;
		goto exit;
	}

	ptr += offset;

	for (i = 0; i < req.networks_cnt; i++) {

		req.networks_list[i].ssid.length = 0;

		params = sscanf(ptr, " %hhu %n",
				  &(req.networks_list[i].ssid.length),
				  &offset);

		if (1 != params) {
			hdd_err("PNO ssid length input is not valid %s", ptr);
			ret = -EINVAL;
			goto exit;
		}

		if ((0 == req.networks_list[i].ssid.length) ||
		    (req.networks_list[i].ssid.length > 32)) {
			hdd_err("SSID Len %d is not correct for network %d",
				  req.networks_list[i].ssid.length, i);
			ret = -EINVAL;
			goto exit;
		}

		/* Advance to SSID */
		ptr += offset;

		memcpy(req.networks_list[i].ssid.ssid, ptr,
		       req.networks_list[i].ssid.length);
		ptr += req.networks_list[i].ssid.length;

		params = sscanf(ptr, " %u %u %hhu %n",
				  &(req.networks_list[i].authentication),
				  &(req.networks_list[i].encryption),
				  &(req.networks_list[i].channel_cnt),
				  &offset);

		if (3 != params) {
			hdd_err("Incorrect cmd %s", ptr);
			ret = -EINVAL;
			goto exit;
		}

		hdd_debug("PNO len %d ssid %.*s auth %d encry %d channel count %d offset %d",
			  req.networks_list[i].ssid.length,
			  req.networks_list[i].ssid.length,
			  req.networks_list[i].ssid.ssid,
			  req.networks_list[i].authentication,
			  req.networks_list[i].encryption,
			  req.networks_list[i].channel_cnt, offset);

		/* Advance to channel list */
		ptr += offset;

		if (SCAN_PNO_MAX_NETW_CHANNELS_EX <
		    req.networks_list[i].channel_cnt) {
			hdd_err("Incorrect number of channels");
			ret = -EINVAL;
			goto exit;
		}

		if (0 != req.networks_list[i].channel_cnt) {
			for (j = 0; j < req.networks_list[i].channel_cnt;
			     j++) {
				if (1 != sscanf(ptr, " %hhu %n", &value,
				   &offset)) {
					hdd_err("PNO network channel is not valid %s",
						  ptr);
					ret = -EINVAL;
					goto exit;
				}
				if (!IS_CHANNEL_VALID(value)) {
					hdd_err("invalid channel: %hhu", value);
					ret = -EINVAL;
					goto exit;
				}
				req.networks_list[i].channels[j] =
					cds_chan_to_freq(value);
				/* Advance to next channel number */
				ptr += offset;
			}
		}

		if (1 != sscanf(ptr, " %u %n",
				&(req.networks_list[i].bc_new_type),
				&offset)) {
			hdd_err("PNO broadcast network type is not valid %s",
				  ptr);
			ret = -EINVAL;
			goto exit;
		}
		if (req.networks_list[i].bc_new_type > 2) {
			hdd_err("invalid bcast nw type: %u",
				req.networks_list[i].bc_new_type);
			ret = -EINVAL;
			goto exit;
		}

		hdd_debug("PNO bcastNetwType %d offset %d",
			  req.networks_list[i].bc_new_type, offset);

		/* Advance to rssi Threshold */
		ptr += offset;
		if (1 != sscanf(ptr, " %d %n",
				&(req.networks_list[i].rssi_thresh),
				&offset)) {
			hdd_err("PNO rssi threshold input is not valid %s",
				  ptr);
			ret = -EINVAL;
			goto exit;
		}
		hdd_debug("PNO rssi %d offset %d",
			  req.networks_list[i].rssi_thresh, offset);
		/* Advance to next network */
		ptr += offset;
	} /* For ucNetworkCount */

	req.fast_scan_period = 0;
	if (sscanf(ptr, " %u %n", &(req.fast_scan_period), &offset) > 0) {
		req.fast_scan_period *= MSEC_PER_SEC;
		ptr += offset;
	}
	if (req.fast_scan_period == 0) {
		hdd_err("invalid fast scan period %u",
			req.fast_scan_period);
			ret = -EINVAL;
			goto exit;
	}

	req.fast_scan_max_cycles = 0;
	if (sscanf(ptr, " %hhu %n", &value,
		   &offset) > 0)
		ptr += offset;
	req.fast_scan_max_cycles = value;

	wlan_pdev_obj_lock(hdd_ctx->pdev);
	psoc = wlan_pdev_get_psoc(hdd_ctx->pdev);
	wlan_pdev_obj_unlock(hdd_ctx->pdev);
	ucfg_scan_register_pno_cb(psoc,
		found_pref_network_cb, NULL);

	ucfg_scan_get_pno_def_params(vdev, &req);
	status = ucfg_scan_pno_start(vdev, &req);
	if (QDF_IS_STATUS_ERROR(status)) {
		hdd_err("Failed to enable PNO");
		ret = -EINVAL;
	}

exit:
	wlan_objmgr_vdev_release_ref(vdev, WLAN_LEGACY_MAC_ID);

	qdf_mem_free(data);
	return ret;
}

static int iw_set_pno(struct net_device *dev,
		      struct iw_request_info *info,
		      union iwreq_data *wrqu, char *extra)
{
	int ret;

	cds_ssr_protect(__func__);
	ret = __iw_set_pno(dev, info, wrqu, extra);
	cds_ssr_unprotect(__func__);

	return ret;
}
#endif /* FEATURE_WLAN_SCAN_PNO */

static int __iw_set_band_config(struct net_device *dev,
				struct iw_request_info *info,
				union iwreq_data *wrqu, char *extra)
{
	struct hdd_adapter *adapter = WLAN_HDD_GET_PRIV_PTR(dev);
	struct hdd_context *hdd_ctx;
	int ret;
	int *value = (int *)extra;

	hdd_enter_dev(dev);

	if (!capable(CAP_NET_ADMIN)) {
		hdd_err("permission check failed");
		return -EPERM;
	}

	hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	ret = hdd_check_private_wext_control(hdd_ctx, info);
	if (0 != ret)
		return ret;

	return hdd_reg_set_band(dev, value[0]);
}

static int iw_set_band_config(struct net_device *dev,
			      struct iw_request_info *info,
			      union iwreq_data *wrqu, char *extra)
{
	int ret;

	cds_ssr_protect(__func__);
	ret = __iw_set_band_config(dev, info, wrqu, extra);
	cds_ssr_unprotect(__func__);

	return ret;
}

static int printk_adapter(void *priv, const char *fmt, ...)
{
	int ret;
	va_list args;

	va_start(args, fmt);
	ret = vprintk(fmt, args);
	ret += printk("\n");
	va_end(args);

	return ret;
}

static void hdd_ioctl_log_buffer(int log_id, uint32_t count)
{
	qdf_abstract_print *print = &printk_adapter;

	switch (log_id) {
	case HTC_CREDIT_HISTORY_LOG:
		cds_print_htc_credit_history(count, print, NULL);
		break;
	case COMMAND_LOG:
		wma_print_wmi_cmd_log(count, print, NULL);
		break;
	case COMMAND_TX_CMP_LOG:
		wma_print_wmi_cmd_tx_cmp_log(count, print, NULL);
		break;
	case MGMT_COMMAND_LOG:
		wma_print_wmi_mgmt_cmd_log(count, print, NULL);
		break;
	case MGMT_COMMAND_TX_CMP_LOG:
		wma_print_wmi_mgmt_cmd_tx_cmp_log(count, print, NULL);
		break;
	case EVENT_LOG:
		wma_print_wmi_event_log(count, print, NULL);
		break;
	case RX_EVENT_LOG:
		wma_print_wmi_rx_event_log(count, print, NULL);
		break;
	case MGMT_EVENT_LOG:
		wma_print_wmi_mgmt_event_log(count, print, NULL);
		break;
	default:
		print(NULL, "Invalid Log Id %d", log_id);
		break;
	}
}

#ifdef CONFIG_WLAN_DEBUG_CRASH_INJECT
int hdd_crash_inject(struct hdd_adapter *adapter, uint32_t v1, uint32_t v2)
{
	struct hdd_context *hdd_ctx;
	int ret;

	hdd_debug("WE_SET_FW_CRASH_INJECT: %d %d",
		  v1, v2);
	pr_err("SSR is triggered by iwpriv CRASH_INJECT: %d %d\n",
	       v1, v2);
	hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	if (!hdd_ctx->config->crash_inject_enabled) {
		hdd_err("Crash Inject ini disabled, Ignore Crash Inject");
		return 0;
	}
	if (v1 == 3) {
		cds_trigger_recovery(QDF_REASON_UNSPECIFIED);
		return 0;
	}
	ret = wma_cli_set2_command(adapter->session_id,
				   GEN_PARAM_CRASH_INJECT,
				   v1, v2, GEN_CMD);
	return ret;
}
#endif

#ifdef CONFIG_DP_TRACE
void hdd_set_dump_dp_trace(uint16_t cmd_type, uint16_t count)
{
	hdd_debug("WE_DUMP_DP_TRACE_LEVEL: %d %d",
		  cmd_type, count);
	if (cmd_type == DUMP_DP_TRACE)
		qdf_dp_trace_dump_all(count, QDF_TRACE_DEFAULT_PDEV_ID);
	else if (cmd_type == ENABLE_DP_TRACE_LIVE_MODE)
		qdf_dp_trace_enable_live_mode();
	else if (cmd_type == CLEAR_DP_TRACE_BUFFER)
		qdf_dp_trace_clear_buffer();
	else if (cmd_type == DISABLE_DP_TRACE_LIVE_MODE)
		qdf_dp_trace_disable_live_mode();
}
#endif

static int __iw_set_two_ints_getnone(struct net_device *dev,
				     struct iw_request_info *info,
				     union iwreq_data *wrqu, char *extra)
{
	struct hdd_adapter *adapter = WLAN_HDD_GET_PRIV_PTR(dev);
	int *value = (int *)extra;
	int sub_cmd = value[0];
	int ret;
	struct hdd_context *hdd_ctx = WLAN_HDD_GET_CTX(adapter);

	hdd_enter_dev(dev);

	ret = wlan_hdd_validate_context(hdd_ctx);
	if (0 != ret)
		return ret;

	ret = hdd_check_private_wext_control(hdd_ctx, info);
	if (0 != ret)
		return ret;

	switch (sub_cmd) {
	case WE_SET_SMPS_PARAM:
		hdd_debug("WE_SET_SMPS_PARAM val %d %d", value[1], value[2]);
		ret = wma_cli_set_command(adapter->session_id,
					  WMI_STA_SMPS_PARAM_CMDID,
					  value[1] << WMA_SMPS_PARAM_VALUE_S
					      | value[2],
					  VDEV_CMD);
		break;
	case WE_SET_FW_CRASH_INJECT:
		ret = hdd_crash_inject(adapter, value[1], value[2]);
		break;
	case WE_ENABLE_FW_PROFILE:
		hdd_err("WE_ENABLE_FW_PROFILE: %d %d",
		       value[1], value[2]);
		ret = wma_cli_set2_command(adapter->session_id,
				 WMI_WLAN_PROFILE_ENABLE_PROFILE_ID_CMDID,
					value[1], value[2], DBG_CMD);
		break;
	case WE_SET_FW_PROFILE_HIST_INTVL:
		hdd_err("WE_SET_FW_PROFILE_HIST_INTVL: %d %d",
		       value[1], value[2]);
		ret = wma_cli_set2_command(adapter->session_id,
					WMI_WLAN_PROFILE_SET_HIST_INTVL_CMDID,
					value[1], value[2], DBG_CMD);
		break;
	case WE_SET_DUAL_MAC_FW_MODE_CONFIG:
		hdd_debug("Ioctl to set dual fw mode config");
		if (hdd_ctx->config->dual_mac_feature_disable ==
				DISABLE_DBS_CXN_AND_SCAN) {
			hdd_err("Dual mac feature is disabled from INI");
			return -EPERM;
		}
		hdd_debug("%d %d", value[1], value[2]);
		policy_mgr_set_dual_mac_fw_mode_config(hdd_ctx->hdd_psoc,
			value[1], value[2]);
		break;
	case WE_DUMP_DP_TRACE_LEVEL:
		hdd_set_dump_dp_trace(value[1], value[2]);
		break;
	case WE_SET_MON_MODE_CHAN:
		ret = wlan_hdd_set_mon_chan(adapter, value[1], value[2]);
		break;
	case WE_SET_WLAN_SUSPEND:
		ret = hdd_wlan_fake_apps_suspend(hdd_ctx->wiphy, dev,
						 value[1], value[2]);
		break;
	case WE_SET_WLAN_RESUME:
		ret = hdd_wlan_fake_apps_resume(hdd_ctx->wiphy, dev);
		break;
	case WE_LOG_BUFFER: {
		int log_id = value[1];
		uint32_t count = value[2] < 0 ? 0 : value[2];

		hdd_ioctl_log_buffer(log_id, count);

		break;
	}
	case WE_SET_BA_AGEING_TIMEOUT:
	{
		void *soc = cds_get_context(QDF_MODULE_ID_SOC);

		if (!soc) {
			hdd_err("Invalid handles");
			break;
		}
		cdp_set_ba_timeout(soc, value[1], value[2]);
		break;
	}
	default:
		hdd_err("Invalid IOCTL command %d", sub_cmd);
		break;
	}

	return ret;
}

static int iw_set_two_ints_getnone(struct net_device *dev,
				   struct iw_request_info *info,
				   union iwreq_data *wrqu, char *extra)
{
	int ret;

	cds_ssr_protect(__func__);
	ret = __iw_set_two_ints_getnone(dev, info, wrqu, extra);
	cds_ssr_unprotect(__func__);

	return ret;
}

/* Define the Wireless Extensions to the Linux Network Device structure */

static const iw_handler we_private[] = {

	[WLAN_PRIV_SET_INT_GET_NONE - SIOCIWFIRSTPRIV] = iw_setint_getnone,
	[WLAN_PRIV_SET_NONE_GET_INT - SIOCIWFIRSTPRIV] = iw_setnone_getint,
	[WLAN_PRIV_SET_CHAR_GET_NONE - SIOCIWFIRSTPRIV] = iw_setchar_getnone,
	[WLAN_PRIV_SET_THREE_INT_GET_NONE - SIOCIWFIRSTPRIV] =
		iw_set_three_ints_getnone,
	[WLAN_PRIV_GET_CHAR_SET_NONE - SIOCIWFIRSTPRIV] = iw_get_char_setnone,
	[WLAN_PRIV_SET_NONE_GET_NONE - SIOCIWFIRSTPRIV] = iw_setnone_getnone,
	[WLAN_PRIV_SET_VAR_INT_GET_NONE - SIOCIWFIRSTPRIV] =
		iw_hdd_set_var_ints_getnone,
	[WLAN_PRIV_SET_NONE_GET_THREE_INT - SIOCIWFIRSTPRIV] =
							iw_setnone_get_threeint,
#ifdef WLAN_FEATURE_FIPS
	[WLAN_PRIV_FIPS_TEST - SIOCIWFIRSTPRIV] = hdd_fips_test,
#endif
	[WLAN_PRIV_ADD_TSPEC - SIOCIWFIRSTPRIV] = iw_add_tspec,
	[WLAN_PRIV_DEL_TSPEC - SIOCIWFIRSTPRIV] = iw_del_tspec,
	[WLAN_PRIV_GET_TSPEC - SIOCIWFIRSTPRIV] = iw_get_tspec,
	[WLAN_PRIV_SET_FTIES - SIOCIWFIRSTPRIV] = iw_set_fties,
	[WLAN_PRIV_SET_HOST_OFFLOAD - SIOCIWFIRSTPRIV] = iw_set_host_offload,
	[WLAN_GET_WLAN_STATISTICS - SIOCIWFIRSTPRIV] = iw_get_statistics,
	[WLAN_SET_KEEPALIVE_PARAMS - SIOCIWFIRSTPRIV] =
		iw_set_keepalive_params,
#ifdef WLAN_FEATURE_PACKET_FILTERING
	[WLAN_SET_PACKET_FILTER_PARAMS - SIOCIWFIRSTPRIV] =
		iw_set_packet_filter_params,
#endif
#ifdef FEATURE_WLAN_SCAN_PNO
	[WLAN_SET_PNO - SIOCIWFIRSTPRIV] = iw_set_pno,
#endif
	[WLAN_SET_BAND_CONFIG - SIOCIWFIRSTPRIV] = iw_set_band_config,
	[WLAN_PRIV_SET_MCBC_FILTER - SIOCIWFIRSTPRIV] =
		iw_set_dynamic_mcbc_filter,
	[WLAN_GET_LINK_SPEED - SIOCIWFIRSTPRIV] = iw_get_linkspeed,
	[WLAN_PRIV_SET_TWO_INT_GET_NONE - SIOCIWFIRSTPRIV] =
		iw_set_two_ints_getnone,
	[WLAN_SET_DOT11P_CHANNEL_SCHED - SIOCIWFIRSTPRIV] =
		iw_set_dot11p_channel_sched,
};

/*Maximum command length can be only 15 */
static const struct iw_priv_args we_private_args[] = {

	/* handlers for main ioctl */
	{WLAN_PRIV_SET_INT_GET_NONE,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 0,
	 ""},

	/* handlers for sub-ioctl */
	{WE_SET_11D_STATE,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 0,
	 "set11Dstate"},

	{WE_WOWL,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 0,
	 "wowl"},

	{WE_SET_POWER,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 0,
	 "setPower"},

	{WE_SET_MAX_ASSOC,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 0,
	 "setMaxAssoc"},

	{WE_SET_SCAN_DISABLE,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 0,
	 "scan_disable"},

	{WE_SET_DATA_INACTIVITY_TO,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 0,
	 "inactivityTO"},

	{WE_SET_WOW_DATA_INACTIVITY_TO,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 0,
	 "wow_ito"},

	{WE_SET_MAX_TX_POWER,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 0,
	 "setMaxTxPower"},

	{WE_SET_TX_POWER,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 0,
	 "setTxPower"},

	{WE_SET_MC_RATE,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 0,
	 "setMcRate"},

	{WE_SET_MAX_TX_POWER_2_4,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 0,
	 "setTxMaxPower2G"},

	{WE_SET_MAX_TX_POWER_5_0,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 0,
	 "setTxMaxPower5G"},

#ifndef REMOVE_PKT_LOG
	{WE_SET_PKTLOG,
	 IW_PRIV_TYPE_INT | MAX_VAR_ARGS,
	 0,
	 "pktlog"},
#endif

	/* SAP has TxMax whereas STA has MaxTx, adding TxMax for STA
	 * as well to keep same syntax as in SAP. Now onwards, STA
	 * will support both
	 */
	{WE_SET_MAX_TX_POWER,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 0,
	 "setTxMaxPower"},

	{WE_SET_TM_LEVEL,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 0,
	 "setTmLevel"},

	{WE_SET_PHYMODE,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 0,
	 "setphymode"},

	{WE_SET_NSS,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 0,
	 "nss"},

	{WE_SET_LDPC,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 0,
	 "ldpc"},

	{WE_SET_TX_STBC,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 0,
	 "tx_stbc"},

	{WE_SET_RX_STBC,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 0,
	 "rx_stbc"},

	{WE_SET_SHORT_GI,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 0,
	 "shortgi"},

	{WE_SET_RTSCTS,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 0,
	 "enablertscts"},

	{WE_SET_CHWIDTH,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 0,
	 "chwidth"},

	{WE_SET_ANI_EN_DIS,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 0,
	 "anienable"},

	{WE_SET_ANI_POLL_PERIOD,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 0,
	 "aniplen"},

	{WE_SET_ANI_LISTEN_PERIOD,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 0,
	 "anilislen"},

	{WE_SET_ANI_OFDM_LEVEL,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 0,
	 "aniofdmlvl"},

	{WE_SET_ANI_CCK_LEVEL,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 0,
	 "aniccklvl"},

	{WE_SET_DYNAMIC_BW,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 0,
	 "cwmenable"},

	{WE_SET_CTS_CBW,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 0,
	 "cts_cbw" },

	{WE_SET_GTX_HT_MCS,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 0,
	 "gtxHTMcs"},

	{WE_SET_GTX_VHT_MCS,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 0,
	 "gtxVHTMcs"},

	{WE_SET_GTX_USRCFG,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 0,
	 "gtxUsrCfg"},

	{WE_SET_GTX_THRE,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 0,
	 "gtxThre"},

	{WE_SET_GTX_MARGIN,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 0,
	 "gtxMargin"},

	{WE_SET_GTX_STEP,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 0,
	 "gtxStep"},

	{WE_SET_GTX_MINTPC,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 0,
	 "gtxMinTpc"},

	{WE_SET_GTX_BWMASK,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 0,
	 "gtxBWMask"},

	{WE_SET_TX_CHAINMASK,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 0,
	 "txchainmask"},

	{WE_SET_RX_CHAINMASK,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 0,
	 "rxchainmask"},

	{WE_SET_11N_RATE,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 0,
	 "set11NRates"},

	{WE_SET_VHT_RATE,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 0,
	 "set11ACRates"},

	{WE_SET_AMPDU,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 0,
	 "ampdu"},

	{WE_SET_AMSDU,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 0,
	 "amsdu"},

	{WE_SET_TXPOW_2G,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 0,
	 "txpow2g"},

	{WE_SET_TXPOW_5G,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 0,
	 "txpow5g"},

#ifdef FEATURE_FW_LOG_PARSING
	/* Sub-cmds DBGLOG specific commands */
	{WE_DBGLOG_LOG_LEVEL,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 0,
	 "dl_loglevel"},

	{WE_DBGLOG_VAP_ENABLE,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 0,
	 "dl_vapon"},

	{WE_DBGLOG_VAP_DISABLE,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 0,
	 "dl_vapoff"},

	{WE_DBGLOG_MODULE_ENABLE,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 0,
	 "dl_modon"},

	{WE_DBGLOG_MODULE_DISABLE,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 0,
	 "dl_modoff"},

	{WE_DBGLOG_MOD_LOG_LEVEL,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 0,
	 "dl_mod_loglevel"},

	{WE_DBGLOG_TYPE,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 0,
	 "dl_type"},
	{WE_DBGLOG_REPORT_ENABLE,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 0,
	 "dl_report"},
#endif /* FEATURE_FW_LOG_PARSING */

	{WE_SET_TXRX_FWSTATS,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 0,
	 "txrx_fw_stats"},

	{WE_SET_TXRX_STATS,
	 IW_PRIV_TYPE_INT | MAX_VAR_ARGS,
	 0,
	 "txrx_stats"},

	{WE_TXRX_FWSTATS_RESET,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 0,
	 "txrx_fw_st_rst"},

	{WE_PPS_PAID_MATCH,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 0, "paid_match"},

	{WE_PPS_GID_MATCH,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 0, "gid_match"},

	{WE_PPS_EARLY_TIM_CLEAR,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 0, "tim_clear"},

	{WE_PPS_EARLY_DTIM_CLEAR,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 0, "dtim_clear"},

	{WE_PPS_EOF_PAD_DELIM,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 0, "eof_delim"},

	{WE_PPS_MACADDR_MISMATCH,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 0, "mac_match"},

	{WE_PPS_DELIM_CRC_FAIL,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 0, "delim_fail"},

	{WE_PPS_GID_NSTS_ZERO,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 0, "nsts_zero"},

	{WE_PPS_RSSI_CHECK,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 0, "rssi_chk"},

	{WE_PPS_5G_EBT,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 0, "5g_ebt"},

	{WE_SET_HTSMPS,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 0, "htsmps"},

	{WE_SET_QPOWER_MAX_PSPOLL_COUNT,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 0, "set_qpspollcnt"},

	{WE_SET_QPOWER_MAX_TX_BEFORE_WAKE,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 0, "set_qtxwake"},

	{WE_SET_QPOWER_SPEC_PSPOLL_WAKE_INTERVAL,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 0, "set_qwakeintv"},

	{WE_SET_QPOWER_SPEC_MAX_SPEC_NODATA_PSPOLL,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 0, "set_qnodatapoll"},

	/* handlers for MCC time quota and latency sub ioctls */
	{WE_MCC_CONFIG_LATENCY,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 0, "setMccLatency"},

	{WE_MCC_CONFIG_QUOTA,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 0, "setMccQuota"},

	{WE_SET_DEBUG_LOG,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 0, "setDbgLvl"},

	/* handlers for early_rx power save */
	{WE_SET_EARLY_RX_ADJUST_ENABLE,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 0, "erx_enable"},

	{WE_SET_EARLY_RX_TGT_BMISS_NUM,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 0, "erx_bmiss_val"},

	{WE_SET_EARLY_RX_BMISS_SAMPLE_CYCLE,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 0, "erx_bmiss_smpl"},

	{WE_SET_EARLY_RX_SLOP_STEP,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 0, "erx_slop_step"},

	{WE_SET_EARLY_RX_INIT_SLOP,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 0, "erx_init_slop"},

	{WE_SET_EARLY_RX_ADJUST_PAUSE,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 0, "erx_adj_pause"},

	{WE_SET_EARLY_RX_DRIFT_SAMPLE,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 0, "erx_dri_sample"},

	{WE_DUMP_STATS,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 0, "dumpStats"},

	{WE_CLEAR_STATS,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 0, "clearStats"},

	{WE_START_FW_PROFILE,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 0, "startProfile"},

	{WE_SET_CHANNEL,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 0, "setChanChange" },

	{WE_SET_CONC_SYSTEM_PREF,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 0, "setConcSysPref" },

	{WE_SET_PDEV_RESET,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 0, "pdev_reset" },

	{WE_SET_MODULATED_DTIM,
	IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	0, "setModDTIM" },

	{WLAN_PRIV_SET_NONE_GET_INT,
	 0,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 ""},

	/* handlers for sub-ioctl */
	{WE_GET_11D_STATE,
	 0,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 "get11Dstate"},

	{WE_GET_WLAN_DBG,
	 0,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 "getwlandbg"},

	{WE_GET_MAX_ASSOC,
	 0,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 "getMaxAssoc"},

	{WE_GET_SAP_AUTO_CHANNEL_SELECTION,
		0, IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
		"getAutoChannel" },

	{WE_GET_CONCURRENCY_MODE,
	 0,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 "getconcurrency"},

	{WE_GET_NSS,
	 0,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 "get_nss"},

	{WE_GET_LDPC,
	 0,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 "get_ldpc"},

	{WE_GET_TX_STBC,
	 0,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 "get_tx_stbc"},

	{WE_GET_RX_STBC,
	 0,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 "get_rx_stbc"},

	{WE_GET_SHORT_GI,
	 0,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 "get_shortgi"},

	{WE_GET_RTSCTS,
	 0,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 "get_rtscts"},

	{WE_GET_CHWIDTH,
	 0,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 "get_chwidth"},

	{WE_GET_ANI_EN_DIS,
	 0,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 "get_anienable"},

	{WE_GET_ANI_POLL_PERIOD,
	 0,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 "get_aniplen"},

	{WE_GET_ANI_LISTEN_PERIOD,
	 0,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 "get_anilislen"},

	{WE_GET_ANI_OFDM_LEVEL,
	 0,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 "get_aniofdmlvl"},

	{WE_GET_ANI_CCK_LEVEL,
	 0,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 "get_aniccklvl"},

	{WE_GET_DYNAMIC_BW,
	 0,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 "get_cwmenable"},

	{WE_GET_GTX_HT_MCS,
	 0,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 "get_gtxHTMcs"},

	{WE_GET_GTX_VHT_MCS,
	 0,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 "get_gtxVHTMcs"},

	{WE_GET_GTX_USRCFG,
	 0,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 "get_gtxUsrCfg"},

	{WE_GET_GTX_THRE,
	 0,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 "get_gtxThre"},

	{WE_GET_GTX_MARGIN,
	 0,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 "get_gtxMargin"},

	{WE_GET_GTX_STEP,
	 0,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 "get_gtxStep"},

	{WE_GET_GTX_MINTPC,
	 0,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 "get_gtxMinTpc"},

	{WE_GET_GTX_BWMASK,
	 0,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 "get_gtxBWMask"},

	{WE_GET_TX_CHAINMASK,
	 0,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 "get_txchainmask"},

	{WE_GET_RX_CHAINMASK,
	 0,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 "get_rxchainmask"},

	{WE_GET_11N_RATE,
	 0,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 "get_11nrate"},

	{WE_GET_AMPDU,
	 0,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 "get_ampdu"},

	{WE_GET_AMSDU,
	 0,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 "get_amsdu"},

	{WE_GET_TXPOW_2G,
	 0,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 "get_txpow2g"},

	{WE_GET_TXPOW_5G,
	 0,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 "get_txpow5g"},

	{WE_GET_PPS_PAID_MATCH,
	 0,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 "get_paid_match"},

	{WE_GET_PPS_GID_MATCH,
	 0,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 "get_gid_match"},

	{WE_GET_PPS_EARLY_TIM_CLEAR,
	 0,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 "get_tim_clear"},

	{WE_GET_PPS_EARLY_DTIM_CLEAR,
	 0,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 "get_dtim_clear"},

	{WE_GET_PPS_EOF_PAD_DELIM,
	 0,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 "get_eof_delim"},

	{WE_GET_PPS_MACADDR_MISMATCH,
	 0,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 "get_mac_match"},

	{WE_GET_PPS_DELIM_CRC_FAIL,
	 0,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 "get_delim_fail"},

	{WE_GET_PPS_GID_NSTS_ZERO,
	 0,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 "get_nsts_zero"},

	{WE_GET_PPS_RSSI_CHECK,
	 0,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 "get_rssi_chk"},

	{WE_GET_QPOWER_MAX_PSPOLL_COUNT,
	 0,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 "get_qpspollcnt"},

	{WE_GET_QPOWER_MAX_TX_BEFORE_WAKE,
	 0,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 "get_qtxwake"},

	{WE_GET_QPOWER_SPEC_PSPOLL_WAKE_INTERVAL,
	 0,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 "get_qwakeintv"},

	{WE_GET_QPOWER_SPEC_MAX_SPEC_NODATA_PSPOLL,
	 0,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 "get_qnodatapoll"},

	{WE_CAP_TSF,
	 0,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 "cap_tsf"},

	{WE_GET_TEMPERATURE,
	 0,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 "get_temp"},
	{WE_GET_DCM,
	 0,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 "get_dcm"},
	{WE_GET_RANGE_EXT,
	 0,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 "get_range_ext"},
	/* handlers for main ioctl */
	{WLAN_PRIV_SET_CHAR_GET_NONE,
	 IW_PRIV_TYPE_CHAR | 512,
	 0,
	 ""},

	/* handlers for sub-ioctl */
	{WE_WOWL_ADD_PTRN,
	 IW_PRIV_TYPE_CHAR | 512,
	 0,
	 "wowlAddPtrn"},

	{WE_WOWL_DEL_PTRN,
	 IW_PRIV_TYPE_CHAR | 512,
	 0,
	 "wowlDelPtrn"},

	/* handlers for sub-ioctl */
	{WE_NEIGHBOR_REPORT_REQUEST,
	 IW_PRIV_TYPE_CHAR | 512,
	 0,
	 "neighbor"},

	{WE_SET_AP_WPS_IE,
	 IW_PRIV_TYPE_CHAR | 512,
	 0,
	 "set_ap_wps_ie"},

	{WE_SET_CONFIG,
	 IW_PRIV_TYPE_CHAR | 512,
	 0,
	 "setConfig"},

#ifdef WLAN_UNIT_TEST
	{WE_UNIT_TEST,
	 IW_PRIV_TYPE_CHAR | 512,
	 0,
	 "unit_test"},
#endif

	/* handlers for main ioctl */
	{WLAN_PRIV_SET_THREE_INT_GET_NONE,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 3,
	 0,
	 ""},

	/* handlers for sub-ioctl */
	{WE_SET_WLAN_DBG,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 3,
	 0,
	 "setwlandbg"},

#ifdef CONFIG_DP_TRACE
	/* handlers for sub-ioctl */
	{WE_SET_DP_TRACE,
	IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 3,
	0,
	"set_dp_trace"},
#endif

	{WE_SET_FW_TEST,
	IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 3,
	0, "fw_test"},

	/* handlers for main ioctl */
	{WLAN_PRIV_SET_NONE_GET_THREE_INT,
	 0,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 3,
	 "" },
	{WE_GET_TSF,
	 0,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 3,
	 "get_tsf" },

	{WE_SET_DUAL_MAC_SCAN_CONFIG,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 3,
	 0,
	 "set_scan_cfg"},

	/* handlers for main ioctl */
	{WLAN_PRIV_GET_CHAR_SET_NONE,
	 0,
	 IW_PRIV_TYPE_CHAR | WE_MAX_STR_LEN,
	 ""},

	/* handlers for sub-ioctl */
	{WE_WLAN_VERSION,
	 0,
	 IW_PRIV_TYPE_CHAR | WE_MAX_STR_LEN,
	 "version"},
	{WE_GET_STATS,
	 0,
	 IW_PRIV_TYPE_CHAR | WE_MAX_STR_LEN,
	 "getStats"},
	{WE_GET_SUSPEND_RESUME_STATS,
	 0,
	 IW_PRIV_TYPE_CHAR | WE_MAX_STR_LEN,
	 "getSuspendStats"},
	{WE_LIST_FW_PROFILE,
	 0,
	 IW_PRIV_TYPE_CHAR | WE_MAX_STR_LEN,
	 "listProfile"},
	{WE_GET_STATES,
	 0,
	 IW_PRIV_TYPE_CHAR | WE_MAX_STR_LEN,
	 "getHostStates"},
	{WE_GET_CFG,
	 0,
	 IW_PRIV_TYPE_CHAR | WE_MAX_STR_LEN,
	 "getConfig"},
	{WE_GET_RSSI,
	 0,
	 IW_PRIV_TYPE_CHAR | WE_MAX_STR_LEN,
	 "getRSSI"},
	{WE_GET_WMM_STATUS,
	 0,
	 IW_PRIV_TYPE_CHAR | WE_MAX_STR_LEN,
	 "getWmmStatus"},
	{
		WE_GET_CHANNEL_LIST,
		0,
		IW_PRIV_TYPE_CHAR | WE_MAX_STR_LEN,
		"getChannelList"
	},
#ifdef FEATURE_WLAN_TDLS
	{
		WE_GET_TDLS_PEERS,
		0,
		IW_PRIV_TYPE_CHAR | WE_MAX_STR_LEN,
		"getTdlsPeers"
	},
#endif
#ifdef WLAN_FEATURE_11W
	{
		WE_GET_11W_INFO,
		0,
		IW_PRIV_TYPE_CHAR | WE_MAX_STR_LEN,
		"getPMFInfo"
	},
#endif
	{
		WE_GET_IBSS_STA_INFO,
		0,
		IW_PRIV_TYPE_CHAR | WE_MAX_STR_LEN,
		"getIbssSTAs"
	},
	{WE_GET_PHYMODE,
	 0,
	 IW_PRIV_TYPE_CHAR | WE_MAX_STR_LEN,
	 "getphymode"},
#if defined(FEATURE_OEM_DATA_SUPPORT) || defined(WIFI_POS_CONVERGED)
	{WE_GET_OEM_DATA_CAP,
	 0,
	 IW_PRIV_TYPE_CHAR | WE_MAX_STR_LEN,
	 "getOemDataCap"},
#endif
	{WE_GET_SNR,
	 0,
	 IW_PRIV_TYPE_CHAR | WE_MAX_STR_LEN,
	 "getSNR"},

	{WE_GET_BA_AGEING_TIMEOUT,
	 0,
	 IW_PRIV_TYPE_CHAR | WE_MAX_STR_LEN,
	 "get_ba_timeout"},

	/* handlers for main ioctl */
	{WLAN_PRIV_SET_NONE_GET_NONE,
	 0,
	 0,
	 ""},

	/* handlers for sub-ioctl */
	{
		WE_IBSS_GET_PEER_INFO_ALL,
		0,
		0,
		"ibssPeerInfoAll"
	},
	{WE_GET_RECOVERY_STAT,
	 0,
	 0,
	 "getRecoverStat"},

	{WE_GET_FW_PROFILE_DATA,
	 0,
	 0,
	 "getProfileData"},

	{WE_SET_REASSOC_TRIGGER,
	0,
	0,
	"reassoc"},

	{WE_STOP_OBSS_SCAN,
	 0,
	 0,
	 "stop_obss_scan"},
	/* handlers for main ioctl */
	{WLAN_PRIV_SET_VAR_INT_GET_NONE,
	 IW_PRIV_TYPE_INT | MAX_VAR_ARGS,
	 0,
	 ""},

	/* handlers for sub-ioctl */
	{WE_IBSS_GET_PEER_INFO,
	 IW_PRIV_TYPE_INT | MAX_VAR_ARGS,
	 0,
	 "ibssPeerInfo"},

#ifdef TRACE_RECORD
	/* handlers for sub-ioctl */
	{WE_MTRACE_SELECTIVE_MODULE_LOG_ENABLE_CMD,
	 IW_PRIV_TYPE_INT | MAX_VAR_ARGS,
	 0,
	 "setdumplog"},

	{WE_MTRACE_DUMP_CMD,
	 IW_PRIV_TYPE_INT | MAX_VAR_ARGS,
	 0,
	 "dumplog"},
#endif

	{WE_POLICY_MANAGER_CINFO_CMD,
	 IW_PRIV_TYPE_INT | MAX_VAR_ARGS,
	 0,
	 "pm_cinfo"},

#ifdef MPC_UT_FRAMEWORK
	{WE_POLICY_MANAGER_CLIST_CMD,
	 IW_PRIV_TYPE_INT | MAX_VAR_ARGS,
	 0,
	 "pm_clist"},

	{WE_POLICY_MANAGER_DLIST_CMD,
	 IW_PRIV_TYPE_INT | MAX_VAR_ARGS,
	 0,
	 "pm_dlist"},

	{WE_POLICY_MANAGER_DBS_CMD,
	 IW_PRIV_TYPE_INT | MAX_VAR_ARGS,
	 0,
	 "pm_dbs"},

	{WE_POLICY_MANAGER_PCL_CMD,
	 IW_PRIV_TYPE_INT | MAX_VAR_ARGS,
	 0,
	 "pm_pcl"},

	{WE_POLICY_MANAGER_ULIST_CMD,
	 IW_PRIV_TYPE_INT | MAX_VAR_ARGS,
	 0,
	 "pm_ulist"},

	{WE_POLICY_MANAGER_QUERY_ACTION_CMD,
	 IW_PRIV_TYPE_INT | MAX_VAR_ARGS,
	 0,
	 "pm_query_action"},

	{WE_POLICY_MANAGER_QUERY_ALLOW_CMD,
	 IW_PRIV_TYPE_INT | MAX_VAR_ARGS,
	 0,
	 "pm_query_allow"},

	{WE_POLICY_MANAGER_SCENARIO_CMD,
	 IW_PRIV_TYPE_INT | MAX_VAR_ARGS,
	 0,
	 "pm_run_scenario"},

	{WE_POLICY_SET_HW_MODE_CMD,
	 IW_PRIV_TYPE_INT | MAX_VAR_ARGS,
	 0,
	 "pm_set_hw_mode"},
#endif
	{
		WE_UNIT_TEST_CMD,
		IW_PRIV_TYPE_INT | MAX_VAR_ARGS,
		0,
		"setUnitTestCmd"
	},
	{
		WE_MAC_PWR_DEBUG_CMD,
		IW_PRIV_TYPE_INT | MAX_VAR_ARGS,
		0,
		"halPwrDebug"
	},

#ifdef WLAN_FEATURE_GPIO_LED_FLASHING
	{WE_LED_FLASHING_PARAM,
	 IW_PRIV_TYPE_INT | MAX_VAR_ARGS,
	 0,
	 "gpio_control"},
#endif
#ifdef WLAN_DEBUG
	{WE_SET_CHAN_AVOID,
	 IW_PRIV_TYPE_INT | MAX_VAR_ARGS,
	 0,
	 "ch_avoid"},
#endif
	/* handlers for main ioctl */
	{WLAN_PRIV_FIPS_TEST,
	 IW_PRIV_TYPE_BYTE | WE_MAX_STR_LEN,
	 IW_PRIV_TYPE_BYTE | WE_MAX_STR_LEN,
	 "fips_test"},

	/* handlers for main ioctl */
	{WLAN_PRIV_ADD_TSPEC,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | HDD_WLAN_WMM_PARAM_COUNT,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 "addTspec"},

	/* handlers for main ioctl */
	{WLAN_PRIV_DEL_TSPEC,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 "delTspec"},

	/* handlers for main ioctl */
	{WLAN_PRIV_GET_TSPEC,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 "getTspec"},

	/* handlers for main ioctl - host offload */
	{
		WLAN_PRIV_SET_HOST_OFFLOAD,
		IW_PRIV_TYPE_BYTE | sizeof(struct host_offload_req),
		0,
		"setHostOffload"
	}
	,

	{
		WLAN_GET_WLAN_STATISTICS,
		0,
		IW_PRIV_TYPE_BYTE | WE_MAX_STR_LEN,
		"getWlanStats"
	}
	,

	{
		WLAN_SET_KEEPALIVE_PARAMS,
		IW_PRIV_TYPE_BYTE | sizeof(tSirKeepAliveReq) |
		IW_PRIV_SIZE_FIXED,
		0,
		"setKeepAlive"
	}
	,
#ifdef WLAN_FEATURE_PACKET_FILTERING
	{
		WLAN_SET_PACKET_FILTER_PARAMS,
		IW_PRIV_TYPE_BYTE |
		sizeof(struct pkt_filter_cfg),
		0,
		"setPktFilter"
	}
	,
#endif
#ifdef FEATURE_WLAN_SCAN_PNO
	{
		WLAN_SET_PNO,
		IW_PRIV_TYPE_CHAR | WE_MAX_STR_LEN,
		0,
		"setpno"
	}
	,
#endif
	{
		WLAN_SET_BAND_CONFIG,
		IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
		0,
		"SETBAND"
	}
	,
	{
		WLAN_PRIV_SET_MCBC_FILTER,
		0,
		0,
		"setMCBCFilter"
	}
	,

	{
		WLAN_GET_LINK_SPEED,
		IW_PRIV_TYPE_CHAR | 18,
		IW_PRIV_TYPE_CHAR | 5, "getLinkSpeed"
	}
	,

	/* handlers for main ioctl */
	{WLAN_PRIV_SET_TWO_INT_GET_NONE,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 2,
	 0,
	 ""}
	,
	{WE_SET_SMPS_PARAM,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 2,
	 0, "set_smps_param"}
	,
	{WLAN_SET_DOT11P_CHANNEL_SCHED,
	 IW_PRIV_TYPE_BYTE | sizeof(struct dot11p_channel_sched),
	 0, "set_dot11p" }
	,
#ifdef CONFIG_WLAN_DEBUG_CRASH_INJECT
	{WE_SET_FW_CRASH_INJECT,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 2,
	 0, "crash_inject"}
	,
#endif
#if defined(WMI_INTERFACE_EVENT_LOGGING) || defined(FEATURE_HTC_CREDIT_HISTORY)
	{WE_LOG_BUFFER,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 2,
	 0, "log_buffer"}
	,
#endif
	{WE_SET_BA_AGEING_TIMEOUT,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 2,
	 0, "set_ba_timeout"}
	,
#ifdef WLAN_SUSPEND_RESUME_TEST
	{WE_SET_WLAN_SUSPEND,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 2,
	 0, "wlan_suspend"}
	,
	{WE_SET_WLAN_RESUME,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 2,
	 0, "wlan_resume"}
	,
#endif
	{WE_ENABLE_FW_PROFILE,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 2,
	 0, "enableProfile"}
	,
	{WE_SET_FW_PROFILE_HIST_INTVL,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 2,
	 0, "set_hist_intvl"}
	,
	{WE_SET_DUAL_MAC_FW_MODE_CONFIG,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 2,
	 0, "set_fw_mode_cfg"}
	,
#ifdef CONFIG_DP_TRACE
	{WE_DUMP_DP_TRACE_LEVEL,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 2,
	 0, "dump_dp_trace"}
	,
#endif
#ifdef FEATURE_MONITOR_MODE_SUPPORT
	{WE_SET_MON_MODE_CHAN,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 2,
	 0, "setMonChan"}
	,
#endif
	{WE_GET_ROAM_SYNCH_DELAY,
	 0,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 "hostroamdelay"}
	,
	{WE_SET_11AX_RATE,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 0,
	 "set_11ax_rate"}
	,
	{WE_SET_DCM,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 0,
	 "enable_dcm"}
	,
	{WE_SET_RANGE_EXT,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 0,
	 "range_ext"}
	,

	{WLAN_PRIV_SET_FTIES,
	 IW_PRIV_TYPE_CHAR | MAX_FTIE_SIZE,
	 0,
	 "set_ft_ies"},
};

const struct iw_handler_def we_handler_def = {
	.num_standard = 0,
	.num_private = QDF_ARRAY_SIZE(we_private),
	.num_private_args = QDF_ARRAY_SIZE(we_private_args),

	.standard = NULL,
	.private = (iw_handler *) we_private,
	.private_args = we_private_args,
	.get_wireless_stats = NULL,
};

void hdd_register_wext(struct net_device *dev)
{
	hdd_enter_dev(dev);

	dev->wireless_handlers = &we_handler_def;

	hdd_exit();
}

void hdd_unregister_wext(struct net_device *dev)
{
	hdd_enter_dev(dev);

	rtnl_lock();
	dev->wireless_handlers = NULL;
	rtnl_unlock();

	hdd_exit();
}
