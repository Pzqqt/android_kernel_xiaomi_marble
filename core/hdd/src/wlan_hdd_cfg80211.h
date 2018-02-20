/*
 * Copyright (c) 2012-2018 The Linux Foundation. All rights reserved.
 *
 * Previously licensed under the ISC license by Qualcomm Atheros, Inc.
 *
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

/*
 * This file was originally distributed by Qualcomm Atheros, Inc.
 * under proprietary terms before Copyright ownership was assigned
 * to the Linux Foundation.
 */

/**
 * DOC: wlan_hdd_cfg80211.h
 *
 * WLAN host device driver cfg80211 functions declaration
 */

#if !defined(HDD_CFG80211_H__)
#define HDD_CFG80211_H__

#include <wlan_cfg80211_scan.h>
#include <wlan_cfg80211.h>
#include <wlan_cfg80211_tdls.h>

struct hdd_context;

/* value for initial part of frames and number of bytes to be compared */
#define GAS_INITIAL_REQ "\x04\x0a"
#define GAS_INITIAL_REQ_SIZE 2

#define GAS_INITIAL_RSP "\x04\x0b"
#define GAS_INITIAL_RSP_SIZE 2

#define GAS_COMEBACK_REQ "\x04\x0c"
#define GAS_COMEBACK_REQ_SIZE 2

#define GAS_COMEBACK_RSP "\x04\x0d"
#define GAS_COMEBACK_RSP_SIZE 2

#define P2P_PUBLIC_ACTION_FRAME "\x04\x09\x50\x6f\x9a\x09"
#define P2P_PUBLIC_ACTION_FRAME_SIZE 6

#define P2P_ACTION_FRAME "\x7f\x50\x6f\x9a\x09"
#define P2P_ACTION_FRAME_SIZE 5

#define SA_QUERY_FRAME_REQ "\x08\x00"
#define SA_QUERY_FRAME_REQ_SIZE 2

#define SA_QUERY_FRAME_RSP "\x08\x01"
#define SA_QUERY_FRAME_RSP_SIZE 2

#define HDD_P2P_WILDCARD_SSID "DIRECT-"
#define HDD_P2P_WILDCARD_SSID_LEN 7

#define WNM_BSS_ACTION_FRAME "\x0a\x07"
#define WNM_BSS_ACTION_FRAME_SIZE 2

#define WNM_NOTIFICATION_FRAME "\x0a\x1a"
#define WNM_NOTIFICATION_FRAME_SIZE 2

#define WPA_OUI_TYPE   "\x00\x50\xf2\x01"
#define BLACKLIST_OUI_TYPE   "\x00\x50\x00\x00"
#define WHITELIST_OUI_TYPE   "\x00\x50\x00\x01"
#define WPA_OUI_TYPE_SIZE  4
#define WMM_OUI_TYPE   "\x00\x50\xf2\x02\x01"
#define WMM_OUI_TYPE_SIZE  5

#define VENDOR1_AP_OUI_TYPE "\x00\xE0\x4C"
#define VENDOR1_AP_OUI_TYPE_SIZE 3

#define WLAN_BSS_MEMBERSHIP_SELECTOR_VHT_PHY 126
#define WLAN_BSS_MEMBERSHIP_SELECTOR_HT_PHY 127
#define BASIC_RATE_MASK   0x80
#define RATE_MASK         0x7f

#ifndef NL80211_AUTHTYPE_FILS_SK
#define NL80211_AUTHTYPE_FILS_SK 5
#endif
#ifndef NL80211_AUTHTYPE_FILS_SK_PFS
#define NL80211_AUTHTYPE_FILS_SK_PFS 6
#endif
#ifndef NL80211_AUTHTYPE_FILS_PK
#define NL80211_AUTHTYPE_FILS_PK 7
#endif
#ifndef WLAN_AKM_SUITE_FILS_SHA256
#define WLAN_AKM_SUITE_FILS_SHA256 0x000FAC0E
#endif
#ifndef WLAN_AKM_SUITE_FILS_SHA384
#define WLAN_AKM_SUITE_FILS_SHA384 0x000FAC0F
#endif
#ifndef WLAN_AKM_SUITE_FT_FILS_SHA256
#define WLAN_AKM_SUITE_FT_FILS_SHA256 0x000FAC10
#endif
#ifndef WLAN_AKM_SUITE_FT_FILS_SHA384
#define WLAN_AKM_SUITE_FT_FILS_SHA384 0x000FAC11
#endif
#ifndef WLAN_AKM_SUITE_DPP_RSN
#define WLAN_AKM_SUITE_DPP_RSN 0x506f9a02
#endif

#define WLAN_AKM_SUITE_OWE 0x000FAC12
#define WLAN_AKM_SUITE_EAP_SHA256 0x000FAC0B
#define WLAN_AKM_SUITE_EAP_SHA384 0x000FAC0C


#ifndef WLAN_AKM_SUITE_SAE
#define WLAN_AKM_SUITE_SAE 0x000FAC08
#endif

#ifdef FEATURE_WLAN_TDLS
#define WLAN_IS_TDLS_SETUP_ACTION(action) \
	((SIR_MAC_TDLS_SETUP_REQ <= action) && \
	(SIR_MAC_TDLS_SETUP_CNF >= action))
#if !defined(TDLS_MGMT_VERSION2)
#define TDLS_MGMT_VERSION2 0
#endif

#endif

#ifdef WLAN_FEATURE_LINK_LAYER_STATS
void wlan_hdd_clear_link_layer_stats(struct hdd_adapter *adapter);
#else
static inline void wlan_hdd_clear_link_layer_stats(struct hdd_adapter *adapter) {}
#endif

#define MAX_CHANNEL (NUM_24GHZ_CHANNELS + NUM_5GHZ_CHANNELS)
#define MAX_SCAN_SSID 10

#define IS_CHANNEL_VALID(channel) ((channel >= 0 && channel < 15) \
			|| (channel >= 36 && channel <= 184))

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 12, 0)) \
	|| defined(BACKPORTED_CHANNEL_SWITCH_PRESENT)
#define CHANNEL_SWITCH_SUPPORTED
#endif

#if defined(CFG80211_DEL_STA_V2) || (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 19, 0)) || defined(WITH_BACKPORTS)
#define USE_CFG80211_DEL_STA_V2
#endif

#define OL_TXRX_INVALID_TDLS_PEER_ID 0xff

/**
 * enum eDFS_CAC_STATUS: CAC status
 *
 * @DFS_CAC_NEVER_DONE: CAC never done
 * @DFS_CAC_IN_PROGRESS: CAC is in progress
 * @DFS_CAC_IN_PROGRESS: CAC already done
 */
typedef enum {
	DFS_CAC_NEVER_DONE,
	DFS_CAC_IN_PROGRESS,
	DFS_CAC_ALREADY_DONE,
} eDFS_CAC_STATUS;

#define MAX_REQUEST_ID			0xFFFFFFFF

/* Feature defines */
#define WIFI_FEATURE_INFRA              0x0001  /* Basic infrastructure mode */
#define WIFI_FEATURE_INFRA_5G           0x0002  /* Support for 5 GHz Band */
#define WIFI_FEATURE_HOTSPOT            0x0004  /* Support for GAS/ANQP */
#define WIFI_FEATURE_P2P                0x0008  /* Wifi-Direct */
#define WIFI_FEATURE_SOFT_AP            0x0010  /* Soft AP */
#define WIFI_FEATURE_EXTSCAN            0x0020  /* Extended Scan APIs */
#define WIFI_FEATURE_NAN                0x0040  /* Neighbor Awareness
						 * Networking
						 */
#define WIFI_FEATURE_D2D_RTT		0x0080  /* Device-to-device RTT */
#define WIFI_FEATURE_D2AP_RTT           0x0100  /* Device-to-AP RTT */
#define WIFI_FEATURE_BATCH_SCAN         0x0200  /* Batched Scan (legacy) */
#define WIFI_FEATURE_PNO                0x0400  /* Preferred network offload */
#define WIFI_FEATURE_ADDITIONAL_STA     0x0800  /* Support for two STAs */
#define WIFI_FEATURE_TDLS               0x1000  /* Tunnel directed link
						 * setup
						 */
#define WIFI_FEATURE_TDLS_OFFCHANNEL	0x2000  /* Support for TDLS off
						 * channel
						 */
#define WIFI_FEATURE_EPR                0x4000  /* Enhanced power reporting */
#define WIFI_FEATURE_AP_STA             0x8000  /* Support for AP STA
						 * Concurrency
						 */
#define WIFI_FEATURE_LINK_LAYER_STATS   0x10000  /* Link layer stats */
#define WIFI_FEATURE_LOGGER             0x20000  /* WiFi Logger */
#define WIFI_FEATURE_HAL_EPNO           0x40000  /* WiFi PNO enhanced */
#define WIFI_FEATURE_RSSI_MONITOR       0x80000  /* RSSI Monitor */
#define WIFI_FEATURE_MKEEP_ALIVE        0x100000  /* WiFi mkeep_alive */
#define WIFI_FEATURE_CONFIG_NDO         0x200000  /* ND offload configure */
#define WIFI_FEATURE_TX_TRANSMIT_POWER  0x400000  /* Tx transmit power levels */
#define WIFI_FEATURE_CONTROL_ROAMING    0x800000  /* Enable/Disable roaming */
#define WIFI_FEATURE_IE_WHITELIST       0x1000000 /* Support Probe IE white listing */
#define WIFI_FEATURE_SCAN_RAND          0x2000000 /* Support MAC & Probe Sequence Number randomization */

/* Support Tx Power Limit setting */
#define WIFI_FEATURE_SET_TX_POWER_LIMIT 0x4000000

/* Add more features here */
#define WIFI_TDLS_SUPPORT			BIT(0)
#define WIFI_TDLS_EXTERNAL_CONTROL_SUPPORT	BIT(1)
#define WIIF_TDLS_OFFCHANNEL_SUPPORT		BIT(2)

#define CFG_NON_AGG_RETRY_MAX                  (31)
#define CFG_AGG_RETRY_MAX                      (31)
#define CFG_MGMT_RETRY_MAX                     (31)
#define CFG_CTRL_RETRY_MAX                     (31)
#define CFG_PROPAGATION_DELAY_MAX              (63)
#define CFG_PROPAGATION_DELAY_BASE             (64)
#define CFG_AGG_RETRY_MIN                      (5)

struct cfg80211_bss *
wlan_hdd_cfg80211_update_bss_db(struct hdd_adapter *adapter,
				struct csr_roam_info *roam_info);

int wlan_hdd_cfg80211_pmksa_candidate_notify(struct hdd_adapter *adapter,
					struct csr_roam_info *roam_info,
					int index, bool preauth);

#ifdef FEATURE_WLAN_LFR_METRICS
QDF_STATUS
wlan_hdd_cfg80211_roam_metrics_preauth(struct hdd_adapter *adapter,
				       struct csr_roam_info *roam_info);

QDF_STATUS
wlan_hdd_cfg80211_roam_metrics_preauth_status(struct hdd_adapter *adapter,
					      struct csr_roam_info *roam_info,
					      bool preauth_status);

QDF_STATUS
wlan_hdd_cfg80211_roam_metrics_handover(struct hdd_adapter *adapter,
					struct csr_roam_info *roam_info);
#endif

#ifdef FEATURE_WLAN_WAPI
void wlan_hdd_cfg80211_set_key_wapi(struct hdd_adapter *adapter,
				    uint8_t key_index,
				    const uint8_t *mac_addr, const uint8_t *key,
				    int key_Len);
#endif
struct hdd_context *hdd_cfg80211_wiphy_alloc(int priv_size);

int wlan_hdd_cfg80211_tdls_scan(struct wiphy *wiphy,
				struct cfg80211_scan_request *request,
				uint8_t source);

int wlan_hdd_cfg80211_scan(struct wiphy *wiphy,
			   struct cfg80211_scan_request *request);

int wlan_hdd_cfg80211_init(struct device *dev,
			   struct wiphy *wiphy, struct hdd_config *pCfg);

void wlan_hdd_cfg80211_deinit(struct wiphy *wiphy);

void wlan_hdd_update_wiphy(struct hdd_context *hdd_ctx);

void wlan_hdd_update_11n_mode(struct hdd_config *cfg);

int wlan_hdd_cfg80211_register(struct wiphy *wiphy);

/**
 * wlan_hdd_cfg80211_register_frames() - register frame types and callbacks
 * with the PE.
 * @adapter: pointer to adapter
 *
 * This function is used by HDD to register frame types which are interested
 * by supplicant, callbacks for rx frame indication and ack.
 *
 * Return: 0 on success and non zero value on failure
 */
int wlan_hdd_cfg80211_register_frames(struct hdd_adapter *adapter);

void wlan_hdd_cfg80211_deregister_frames(struct hdd_adapter *adapter);

void hdd_reg_notifier(struct wiphy *wiphy,
				 struct regulatory_request *request);

extern void hdd_conn_set_connection_state(struct hdd_adapter *adapter,
					  eConnectionState connState);
QDF_STATUS wlan_hdd_validate_operation_channel(struct hdd_adapter *adapter,
					       int channel);
#ifdef FEATURE_WLAN_TDLS
int wlan_hdd_cfg80211_send_tdls_discover_req(struct wiphy *wiphy,
					     struct net_device *dev, u8 *peer);
#endif

void *wlan_hdd_change_country_code_cb(void *adapter);
void hdd_select_cbmode(struct hdd_adapter *adapter, uint8_t operationChannel,
		       struct ch_params *ch_params);

/**
 * wlan_hdd_is_ap_supports_immediate_power_save() - to find certain vendor APs
 *				which do not support immediate power-save.
 * @ies: beacon IE of the AP which STA is connecting/connected to
 * @length: beacon IE length only
 *
 * This API takes the IE of connected/connecting AP and determines that
 * whether it has specific vendor OUI. If it finds then it will return false to
 * notify that AP doesn't support immediate power-save.
 *
 * Return: true or false based on findings
 */
bool wlan_hdd_is_ap_supports_immediate_power_save(uint8_t *ies, int length);
void wlan_hdd_del_station(struct hdd_adapter *adapter);

#if defined(USE_CFG80211_DEL_STA_V2)
int wlan_hdd_cfg80211_del_station(struct wiphy *wiphy,
				  struct net_device *dev,
				  struct station_del_parameters *param);
#else
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 16, 0))
int wlan_hdd_cfg80211_del_station(struct wiphy *wiphy,
				  struct net_device *dev,
				  const uint8_t *mac);
#else
int wlan_hdd_cfg80211_del_station(struct wiphy *wiphy,
				  struct net_device *dev,
				  uint8_t *mac);
#endif
#endif /* USE_CFG80211_DEL_STA_V2 */

int wlan_hdd_send_avoid_freq_event(struct hdd_context *hdd_ctx,
				   struct ch_avoid_ind_type *avoid_freq_list);

/**
 * wlan_hdd_send_hang_reason_event() - Send hang reason to the userspace
 * @hdd_ctx: Pointer to hdd context
 * @reason: cds recovery reason
 *
 * Return: 0 on success or failure reason
 */
int wlan_hdd_send_hang_reason_event(struct hdd_context *hdd_ctx,
				    uint32_t reason);

int wlan_hdd_send_avoid_freq_for_dnbs(struct hdd_context *hdd_ctx,
				      uint8_t op_chan);

#ifdef FEATURE_WLAN_EXTSCAN
void wlan_hdd_cfg80211_extscan_callback(void *ctx,
					const uint16_t evType, void *pMsg);
#endif /* FEATURE_WLAN_EXTSCAN */

/**
 * wlan_hdd_rso_cmd_status_cb() - HDD callback to read RSO command status
 * @ctx: void pointer to hdd context
 * @rso_status: rso command status
 *
 * This callback function is invoked by firmware to update
 * the RSO(ROAM SCAN OFFLOAD) command status.
 *
 * Return: None
 */
void wlan_hdd_rso_cmd_status_cb(void *ctx, struct rso_cmd_status *rso_status);

void hdd_rssi_threshold_breached(void *hddctx,
				 struct rssi_breach_event *data);

struct cfg80211_bss *
wlan_hdd_cfg80211_update_bss_list(struct hdd_adapter *adapter,
				  tSirMacAddr bssid);

void wlan_hdd_cfg80211_acs_ch_select_evt(struct hdd_adapter *adapter);

#ifdef WLAN_FEATURE_ROAM_OFFLOAD
int wlan_hdd_send_roam_auth_event(struct hdd_adapter *adapter, uint8_t *bssid,
		uint8_t *req_rsn_ie, uint32_t req_rsn_length, uint8_t
		*rsp_rsn_ie, uint32_t rsp_rsn_length, struct csr_roam_info
		*roam_info_ptr);
#else
static inline int wlan_hdd_send_roam_auth_event(struct hdd_adapter *adapter,
		uint8_t *bssid, uint8_t *req_rsn_ie, uint32_t req_rsn_length,
		uint8_t *rsp_rsn_ie, uint32_t rsp_rsn_length,
		struct csr_roam_info *roam_info_ptr)
{
	return 0;
}
#endif

int wlan_hdd_cfg80211_update_apies(struct hdd_adapter *adapter);
int wlan_hdd_request_pre_cac(uint8_t channel);
int wlan_hdd_sap_cfg_dfs_override(struct hdd_adapter *adapter);

enum policy_mgr_con_mode wlan_hdd_convert_nl_iftype_to_hdd_type(
					enum nl80211_iftype type);

int wlan_hdd_enable_dfs_chan_scan(struct hdd_context *hdd_ctx,
				  bool enable_dfs_channels);

int wlan_hdd_cfg80211_update_band(struct hdd_context *hdd_ctx,
				  struct wiphy *wiphy,
				  enum band_info eBand);

/**
 * wlan_hdd_try_disconnect() - try disconnnect from previous connection
 * @adapter: Pointer to adapter
 *
 * This function is used to disconnect from previous connection
 *
 * Return: 0 for success, non-zero for failure
 */
int wlan_hdd_try_disconnect(struct hdd_adapter *adapter);

#if defined(CFG80211_DISCONNECTED_V2) || \
(LINUX_VERSION_CODE >= KERNEL_VERSION(4, 2, 0))
static inline void wlan_hdd_cfg80211_indicate_disconnect(struct net_device *dev,
							bool locally_generated,
							int reason)
{
	cfg80211_disconnected(dev, reason, NULL, 0,
				locally_generated, GFP_KERNEL);
}
#else
static inline void wlan_hdd_cfg80211_indicate_disconnect(struct net_device *dev,
							bool locally_generated,
							int reason)
{
	cfg80211_disconnected(dev, reason, NULL, 0,
				GFP_KERNEL);
}
#endif

/**
 * wlan_hdd_inform_bss_frame() - inform bss details to NL80211
 * @adapter: Pointer to adapter
 * @bss_desc: Pointer to bss descriptor
 *
 * This function is used to inform the BSS details to nl80211 interface.
 *
 * Return: struct cfg80211_bss pointer
 */
struct cfg80211_bss *
wlan_hdd_inform_bss_frame(struct hdd_adapter *adapter,
				     struct bss_description *bss_desc);

/**
 * wlan_hdd_change_hw_mode_for_given_chnl() - change HW mode for given channel
 * @adapter: pointer to adapter
 * @channel: given channel number
 * @reason: reason for HW mode change is needed
 *
 * This API decides and sets hardware mode to DBS based on given channel.
 * For example, some of the platforms require DBS hardware mode to operate
 * in 2.4G channel
 *
 * Return: 0 for success and non-zero for failure
 */
int wlan_hdd_change_hw_mode_for_given_chnl(struct hdd_adapter *adapter,
				uint8_t channel,
				enum policy_mgr_conn_update_reason reason);

/**
 * hdd_rate_info_bw: an HDD internal rate bandwidth representation
 * @HDD_RATE_BW_5: 5MHz
 * @HDD_RATE_BW_10: 10MHz
 * @HDD_RATE_BW_20: 20MHz
 * @HDD_RATE_BW_40: 40MHz
 * @HDD_RATE_BW_80: 80MHz
 * @HDD_RATE_BW_160: 160 MHz
 */
enum hdd_rate_info_bw {
	HDD_RATE_BW_5,
	HDD_RATE_BW_10,
	HDD_RATE_BW_20,
	HDD_RATE_BW_40,
	HDD_RATE_BW_80,
	HDD_RATE_BW_160,
};

/**
 * hdd_set_rate_bw(): Set the bandwidth for the given rate_info
 * @info: The rate info for which the bandwidth should be set
 * @hdd_bw: HDD representation of a rate info bandwidth
 */
void hdd_set_rate_bw(struct rate_info *info, enum hdd_rate_info_bw hdd_bw);

/**
 * hdd_lost_link_info_cb() - callback function to get lost link information
 * @context: HDD context
 * @lost_link_info: lost link information
 *
 * Return: none
 */
void hdd_lost_link_info_cb(void *context,
			struct sir_lost_link_info *lost_link_info);
/*
 * hdd_get_sap_operating_band:  Get current operating channel
 * for sap.
 * @hdd_ctx: hdd context
 *
 * Return : Corresponding band for SAP operating channel
 */
uint8_t hdd_get_sap_operating_band(struct hdd_context *hdd_ctx);

/**
 * wlan_hdd_try_disconnect() - try disconnnect from previous connection
 * @adapter: Pointer to adapter
 *
 * This function is used to disconnect from previous connection
 *
 * Return: 0 for success, non-zero for failure
 */
int wlan_hdd_try_disconnect(struct hdd_adapter *adapter);

/**
 * wlan_hdd_disconnect() - hdd disconnect api
 * @adapter: Pointer to adapter
 * @reason: Disconnect reason code
 *
 * This function is used to issue a disconnect request to SME
 *
 * Return: 0 for success, non-zero for failure
 */
int wlan_hdd_disconnect(struct hdd_adapter *adapter, u16 reason);

/**
 * hdd_update_cca_info_cb() - stores congestion value in station context
 * @context : HDD context
 * @congestion : congestion
 * @vdev_id : vdev id
 *
 * Return: None
 */
void hdd_update_cca_info_cb(void *context, uint32_t congestion,
			uint32_t vdev_id);

/**
 * wlan_hdd_get_adjacent_chan(): Gets next/previous channel
 * to the channel passed.
 * @chan: Channel
 * @upper: If "true" then next channel is returned or else
 * previous channel is returned.
 *
 * This function returns the next/previous adjacent-channel to
 * the channel passed. If "upper = true" then next channel is
 * returned else previous is returned.
 */
int wlan_hdd_get_adjacent_chan(uint8_t chan, bool upper);

/**
 * wlan_hdd_merge_avoid_freqs(): Merge two tHddAvoidFreqList
 * @destFreqList: Destination list in which merged frequency
 * list will be available.
 * @srcFreqList: Source frequency list.
 *
 * Merges two avoid_frequency lists
 */
int wlan_hdd_merge_avoid_freqs(struct ch_avoid_ind_type *destFreqList,
		struct ch_avoid_ind_type *srcFreqList);


/**
 * hdd_bt_activity_cb() - callback function to receive bt activity
 * @context: HDD context
 * @bt_activity: specifies the kind of bt activity
 *
 * Return: none
 */
void hdd_bt_activity_cb(void *context, uint32_t bt_activity);

/**
 * wlan_hdd_save_gtk_offload_params() - Save gtk offload parameters in STA
 *                                      context for offload operations.
 * @adapter: Adapter context
 * @kck_ptr: KCK buffer pointer
 * @kek_ptr: KEK buffer pointer
 * @kek_len: KEK length
 * @replay_ctr: Pointer to 64 bit long replay counter
 * @big_endian: true if replay_ctr is in big endian format
 *
 * Return: None
 */
void wlan_hdd_save_gtk_offload_params(struct hdd_adapter *adapter,
					     uint8_t *kck_ptr,
					     uint8_t *kek_ptr,
					     uint32_t kek_len,
					     uint8_t *replay_ctr,
					     bool big_endian);
#endif
