/*
 * Copyright (c) 2014-2017, 2019 The Linux Foundation. All rights reserved.
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

#if !defined(__HOST_DIAG_CORE_LOG_H)
#define __HOST_DIAG_CORE_LOG_H

/**=========================================================================

   \file  host_diag_core_log.h

   \brief WLAN UTIL host DIAG logs

   Definitions for WLAN UTIL host diag events

   ========================================================================*/

/* $Header$ */

/*--------------------------------------------------------------------------
   Include Files
   ------------------------------------------------------------------------*/
#include "qdf_types.h"
#include "i_host_diag_core_log.h"

/*--------------------------------------------------------------------------
   Preprocessor definitions and constants
   ------------------------------------------------------------------------*/
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*--------------------------------------------------------------------------
   Preprocessor definitions and constants
   ------------------------------------------------------------------------*/
#define HOST_LOG_MAX_NUM_SSID                                (21)
#define HOST_LOG_MAX_NUM_BSSID                               (21)
#define HOST_LOG_MAX_SSID_SIZE                               (32)
#define HOST_LOG_MAX_BSSID_SIZE                              (6)
#define HOST_LOG_MAX_NUM_CHANNEL                             (64)
#define HOST_LOG_MAX_NUM_HO_CANDIDATE_APS                    (20)
#define HOST_LOG_MAX_WOW_PTRN_SIZE                           (128)
#define HOST_LOG_MAX_WOW_PTRN_MASK_SIZE                      (16)
#define VOS_LOG_PKT_LOG_SIZE                                 (2048)
#define HOST_LOG_PKT_LOG_THRESHOLD                           40960
#define HOST_LOG_MAX_COLD_BOOT_CAL_DATA_SIZE                 (2048)

/* Version to be updated whenever format of vos_log_pktlog_info changes */
#define VERSION_LOG_WLAN_PKT_LOG_INFO_C                     1
/* Version to be updated whenever format of host_log_cold_boot_cal_data_type
 * changes
 */
#define VERSION_LOG_WLAN_COLD_BOOT_CAL_DATA_C               1

/*---------------------------------------------------------------------------
   This packet contains the scan results of the recent scan operation
   LOG_WLAN_SCAN_C                                 0x1496
   ---------------------------------------------------------------------------*/
typedef struct {
	log_hdr_type hdr;
	uint8_t eventId;
	uint8_t numSsid;
	uint8_t ssid[HOST_LOG_MAX_NUM_SSID][HOST_LOG_MAX_SSID_SIZE];
	uint8_t bssid[HOST_LOG_MAX_NUM_BSSID][HOST_LOG_MAX_BSSID_SIZE];
	uint8_t totalSsid;
	uint8_t minChnTime;
	uint8_t maxChnTime;
	uint16_t timeBetweenBgScan;
	uint8_t BSSMode;
	uint8_t numChannel;
	uint8_t channels[HOST_LOG_MAX_NUM_CHANNEL];
	uint16_t status;
} host_log_scan_pkt_type;

/*---------------------------------------------------------------------------
   This packet contains the information related to IBSS connection setup
   LOG_WLAN_IBSS_C                                 0x1497
   ---------------------------------------------------------------------------*/
typedef struct {
	log_hdr_type hdr;
	uint8_t eventId;
	uint8_t channelSetting;
	struct qdf_mac_addr bssid;
	struct qdf_mac_addr peer_macaddr;
	uint8_t ssid[HOST_LOG_MAX_SSID_SIZE];
	uint8_t operatingChannel;
	uint8_t beaconInterval;
	uint8_t status;
} host_log_ibss_pkt_type;

/*---------------------------------------------------------------------------
   This packet contains the information related to 802.11D
   LOG_WLAN_80211D_C                               0x1498
   ---------------------------------------------------------------------------*/
typedef struct {
	log_hdr_type hdr;
	uint8_t eventId;
	uint8_t numChannel;
	uint8_t Channels[HOST_LOG_MAX_NUM_CHANNEL];
	uint8_t TxPwr[HOST_LOG_MAX_NUM_CHANNEL];
	uint8_t countryCode[3];
	uint8_t supportMultipleDomain;
} host_log_802_11d_pkt_type;

/*---------------------------------------------------------------------------
   This is a log packet which contains below handoff information:
   - Current AP + RSSI (if already associated)
   - Candidate AP + RSSI (before association and when the list is updated)
   - For each BSSID in candidate list, provide RSSI, QoS and security compatibility
   LOG_WLAN_HANDOFF_C                              0x1499
   ---------------------------------------------------------------------------*/
typedef struct {
	uint8_t ssid[9];
	uint8_t bssid[HOST_LOG_MAX_BSSID_SIZE];
	uint8_t channel_id;
	uint32_t qos_score;
	uint32_t sec_score;
	uint32_t rssi_score;
	uint32_t overall_score;
	uint32_t tx_per;                /* represented as a %      */
	uint32_t rx_per;                /* represented as a %      */

} host_log_ho_ap_info;

typedef struct {
	log_hdr_type hdr;
	uint32_t num_aps;
	host_log_ho_ap_info current_ap_info;
	host_log_ho_ap_info
		candidate_ap_info[HOST_LOG_MAX_NUM_HO_CANDIDATE_APS];
} host_log_ho_pkt_type;

/*---------------------------------------------------------------------------
   This packet contains the information related to the EDCA parameters
   advertised by the AP
   LOG_WLAN_QOS_EDCA_C                             0x149A
   ---------------------------------------------------------------------------*/
typedef struct {
	log_hdr_type hdr;
	uint8_t aci_be;
	uint8_t cw_be;
	uint16_t txoplimit_be;
	uint8_t aci_bk;
	uint8_t cw_bk;
	uint16_t txoplimit_bk;
	uint8_t aci_vi;
	uint8_t cw_vi;
	uint16_t txoplimit_vi;
	uint8_t aci_vo;
	uint8_t cw_vo;
	uint16_t txoplimit_vo;
} host_log_qos_edca_pkt_type;

/*---------------------------------------------------------------------------
   This packet contains the total number of beacon received value
   LOG_WLAN_BEACON_UPDATE_C                        0x149B
   ---------------------------------------------------------------------------*/
typedef struct {
	log_hdr_type hdr;
	uint32_t bcn_rx_cnt;
} host_log_beacon_update_pkt_type;

/*---------------------------------------------------------------------------
   This packet contains the information related to a WoW patern value when set
   LOG_WLAN_POWERSAVE_WOW_ADD_PTRN_C               0x149C
   ---------------------------------------------------------------------------*/
typedef struct {
	log_hdr_type hdr;
	uint8_t pattern_id;
	uint8_t pattern_byte_offset;
	uint8_t pattern_size;
	uint8_t pattern[HOST_LOG_MAX_WOW_PTRN_SIZE];
	uint8_t pattern_mask_size;
	uint8_t pattern_mask[HOST_LOG_MAX_WOW_PTRN_MASK_SIZE];
} host_log_powersave_wow_add_ptrn_pkt_type;

/*---------------------------------------------------------------------------
   This packet contains the Tspec info negotiated with the AP for the
   specific AC
   LOG_WLAN_QOS_TSPEC_C                            0x14A2
   ---------------------------------------------------------------------------*/
typedef struct {
	log_hdr_type hdr;
	uint8_t tsinfo[3];
	uint16_t nominal_msdu_size;
	uint16_t maximum_msdu_size;
	uint32_t min_service_interval;
	uint32_t max_service_interval;
	uint32_t inactivity_interval;
	uint32_t suspension_interval;
	uint32_t svc_start_time;
	uint32_t min_data_rate;
	uint32_t mean_data_rate;
	uint32_t peak_data_rate;
	uint32_t max_burst_size;
	uint32_t delay_bound;
	uint32_t min_phy_rate;
	uint16_t surplus_bw_allowance;
	uint16_t medium_time;
} host_log_qos_tspec_pkt_type;

/*---------------------------------------------------------------------------
   This packet contains data information when stall detected
   LOG_TRSP_DATA_STALL_C                           0x1801
   ---------------------------------------------------------------------------*/

typedef struct {
	char channelName[4];
	uint32_t numDesc;
	uint32_t numFreeDesc;
	uint32_t numRsvdDesc;
	uint32_t headDescOrder;
	uint32_t tailDescOrder;
	uint32_t ctrlRegVal;
	uint32_t statRegVal;
	uint32_t numValDesc;
	uint32_t numInvalDesc;
} host_log_data_stall_channel_type;

typedef struct {
	log_hdr_type hdr;
	uint32_t PowerState;
	uint32_t numFreeBd;
	host_log_data_stall_channel_type dxeChannelInfo[4];
} host_log_data_stall_type;

/*---------------------------------------------------------------------------
   This packet contains the rssi value from BSS descriptor
   LOG_WLAN_RSSI_UPDATE_C                          0x1354
   ---------------------------------------------------------------------------*/
typedef struct {
	log_hdr_type hdr;
	int8_t rssi;
} host_log_rssi_pkt_type;

/**
 * struct host_log_pktlog_info - Packet log info
 * @log_hdr: Log header
 * @buf_len: Length of the buffer that follows
 * @buf:     Buffer containing the packet log info
 *
 * Structure containing the packet log information
 * LOG_WLAN_PKT_LOG_INFO_C          0x18E0
 */
struct host_log_pktlog_info {
	log_hdr_type log_hdr;
	uint32_t version;
	uint32_t seq_no;
	uint32_t buf_len;
	uint8_t buf[];
};

/**
 * struct host_log_cold_boot_cal_data_type - Cold boot cal log info
 * @hdr: Log header
 * @version: version
 * @flags: Flag to indicate if more data follows
 * @cb_cal_data_len: Length of the cal data
 * @cb_cal_data: Cold boot cal data
 *
 * Structure containing the cold boot calibration data
 * log information
 * LOG_WLAN_COLD_BOOT_CAL_DATA_C          0x1A18
 */
struct host_log_cold_boot_cal_data_type {
	log_hdr_type hdr;
	uint32_t version;
	uint32_t flags;
	uint32_t cb_cal_data_len;
	uint8_t cb_cal_data[HOST_LOG_MAX_COLD_BOOT_CAL_DATA_SIZE];
};

/*-------------------------------------------------------------------------
   Function declarations and documenation
   ------------------------------------------------------------------------*/

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* __HOST_DIAG_CORE_LOG_H */
