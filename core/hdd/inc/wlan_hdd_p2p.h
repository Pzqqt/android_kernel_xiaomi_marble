/*
 * Copyright (c) 2012-2017 The Linux Foundation. All rights reserved.
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

#ifndef __P2P_H
#define __P2P_H

/**
 * DOC: wlan_hdd_p2p.h
 *
 * Linux HDD P2P include file
 */
#define ACTION_FRAME_TX_TIMEOUT 2000
#define WAIT_CANCEL_REM_CHAN    1000
#define WAIT_REM_CHAN_READY     1000
#define WAIT_CHANGE_CHANNEL_FOR_OFFCHANNEL_TX 3000
#define COMPLETE_EVENT_PROPOGATE_TIME 10

#ifdef QCA_WIFI_3_0_EMU
#define ACTION_FRAME_DEFAULT_WAIT 500
#else
#define ACTION_FRAME_DEFAULT_WAIT 200
#endif

#define WLAN_HDD_GET_TYPE_FRM_FC(__fc__)         (((__fc__) & 0x0F) >> 2)
#define WLAN_HDD_GET_SUBTYPE_FRM_FC(__fc__)      (((__fc__) & 0xF0) >> 4)
#define WLAN_HDD_80211_FRM_DA_OFFSET             4
#define P2P_WILDCARD_SSID_LEN                    7
#define P2P_WILDCARD_SSID                        "DIRECT-"
#define WLAN_HDD_80211_PEER_ADDR_OFFSET (WLAN_HDD_80211_FRM_DA_OFFSET + \
					 MAC_ADDR_LEN)


#ifdef QCA_WIFI_3_0_EMU
#define P2P_ROC_DURATION_MULTIPLIER_GO_PRESENT   2
#define P2P_ROC_DURATION_MULTIPLIER_GO_ABSENT    3
#else
#define P2P_ROC_DURATION_MULTIPLIER_GO_PRESENT   2
#define P2P_ROC_DURATION_MULTIPLIER_GO_ABSENT    5
#endif

#define HDD_P2P_MAX_ROC_DURATION 1000
#define MAX_ROC_REQ_QUEUE_ENTRY 10

#define P2P_POWER_SAVE_TYPE_OPPORTUNISTIC        (1 << 0)
#define P2P_POWER_SAVE_TYPE_PERIODIC_NOA         (1 << 1)
#define P2P_POWER_SAVE_TYPE_SINGLE_NOA           (1 << 2)

#define ACTION_FRAME_RSP_WAIT 500
#define ACTION_FRAME_ACK_WAIT 300

#ifdef WLAN_FEATURE_P2P_DEBUG
enum p2p_connection_status {
	P2P_NOT_ACTIVE,
	P2P_GO_NEG_PROCESS,
	P2P_GO_NEG_COMPLETED,
	P2P_CLIENT_CONNECTING_STATE_1,
	P2P_GO_COMPLETED_STATE,
	P2P_CLIENT_CONNECTED_STATE_1,
	P2P_CLIENT_DISCONNECTED_STATE,
	P2P_CLIENT_CONNECTING_STATE_2,
	P2P_CLIENT_COMPLETED_STATE
};

extern enum p2p_connection_status global_p2p_connection_status;
#endif

struct p2p_app_set_ps {
	uint8_t opp_ps;
	uint32_t ctWindow;
	uint8_t count;
	uint32_t duration;
	uint32_t interval;
	uint32_t single_noa_duration;
	uint8_t psSelection;
};

int wlan_hdd_cfg80211_remain_on_channel(struct wiphy *wiphy,
					struct wireless_dev *wdev,
					struct ieee80211_channel *chan,
					unsigned int duration, u64 *cookie);

int wlan_hdd_cfg80211_cancel_remain_on_channel(struct wiphy *wiphy,
					       struct wireless_dev *wdev,
					       u64 cookie);

int wlan_hdd_cfg80211_mgmt_tx_cancel_wait(struct wiphy *wiphy,
					  struct wireless_dev *wdev,
					  u64 cookie);

int hdd_set_p2p_ps(struct net_device *dev, void *msgData);
int hdd_set_p2p_opps(struct net_device *dev, uint8_t *command);
int hdd_set_p2p_noa(struct net_device *dev, uint8_t *command);

void __hdd_indicate_mgmt_frame(struct hdd_adapter *adapter,
			     uint32_t nFrameLength, uint8_t *pbFrames,
			     uint8_t frameType, uint32_t rxChan, int8_t rxRssi);

void hdd_remain_chan_ready_handler(struct hdd_adapter *adapter,
	uint32_t scan_id);
void hdd_send_action_cnf(struct hdd_adapter *adapter, bool actionSendSuccess);
int wlan_hdd_check_remain_on_channel(struct hdd_adapter *adapter);
void hdd_send_action_cnf_cb(uint32_t session_id, bool status);
void wlan_hdd_cancel_existing_remain_on_channel(struct hdd_adapter *adapter);

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 14, 0))
int wlan_hdd_mgmt_tx(struct wiphy *wiphy, struct wireless_dev *wdev,
		     struct cfg80211_mgmt_tx_params *params, u64 *cookie);
#else
int wlan_hdd_mgmt_tx(struct wiphy *wiphy, struct wireless_dev *wdev,
		     struct ieee80211_channel *chan, bool offchan,
		     unsigned int wait,
		     const u8 *buf, size_t len, bool no_cck,
		     bool dont_wait_for_ack, u64 *cookie);
#endif

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 12, 0)
struct wireless_dev *wlan_hdd_add_virtual_intf(struct wiphy *wiphy,
					       const char *name,
					       unsigned char name_assign_type,
					       enum nl80211_iftype type,
					       struct vif_params *params);
#elif (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 1, 0)) || defined(WITH_BACKPORTS)
struct wireless_dev *wlan_hdd_add_virtual_intf(struct wiphy *wiphy,
					       const char *name,
					       unsigned char name_assign_type,
					       enum nl80211_iftype type,
					       u32 *flags,
					       struct vif_params *params);
#else
struct wireless_dev *wlan_hdd_add_virtual_intf(struct wiphy *wiphy,
					       const char *name,
					       enum nl80211_iftype type,
					       u32 *flags,
					       struct vif_params *params);

#endif

int wlan_hdd_del_virtual_intf(struct wiphy *wiphy, struct wireless_dev *wdev);
int __wlan_hdd_del_virtual_intf(struct wiphy *wiphy, struct wireless_dev *wdev);


void wlan_hdd_cleanup_remain_on_channel_ctx(struct hdd_adapter *adapter);

void wlan_hdd_roc_request_dequeue(struct work_struct *work);

/**
 * wlan_hdd_set_power_save() - hdd set power save
 * @adapter:    adapter context
 * @pnoa:       pointer to noa parameters
 *
 * This function sets power save parameters.
 *
 * Return: 0 - success
 *    others - failure
 */
int wlan_hdd_set_power_save(struct hdd_adapter *adapter,
	tpP2pPsConfig pnoa);

/**
 * wlan_hdd_listen_offload_start() - hdd set listen offload start
 * @adapter:  adapter context
 * @params:   listen offload parameters
 *
 * This function sets listen offload start parameters.
 *
 * Return: 0 - success
 *    others - failure
 */
int wlan_hdd_listen_offload_start(struct hdd_adapter *adapter,
	struct sir_p2p_lo_start *params);

/**
 * wlan_hdd_listen_offload_stop() - hdd set listen offload stop
 * @adapter:  adapter context
 *
 * This function sets listen offload stop parameters.
 *
 * Return: 0 - success
 *    others - failure
 */
int wlan_hdd_listen_offload_stop(struct hdd_adapter *adapter);

/**
 * wlan_hdd_set_mas() - Function to set MAS value to FW
 * @adapter:            Pointer to HDD adapter
 * @mas_value:          0-Disable, 1-Enable MAS
 *
 * This function passes down the value of MAS to FW
 *
 * Return: Configuration message posting status, SUCCESS or Fail
 *
 */
int32_t wlan_hdd_set_mas(struct hdd_adapter *adapter, uint8_t mas_value);

/**
 * wlan_hdd_set_mcc_p2p_quota() - Function to set quota for P2P
 * to FW
 * @adapter:            Pointer to HDD adapter
 * @set_value:          Quota value for the interface
 *
 * This function is used to set the quota for P2P cases
 *
 * Return: Configuration message posting status, SUCCESS or Fail
 *
 */
int wlan_hdd_set_mcc_p2p_quota(struct hdd_adapter *adapter,
			       uint32_t set_value);

/**
 * wlan_hdd_go_set_mcc_p2p_quota() - Function to set quota for
 * P2P GO to FW
 * @hostapd_adapter:    Pointer to HDD adapter
 * @set_value:          Quota value for the interface
 *
 * This function is used to set the quota for P2P GO cases
 *
 * Return: Configuration message posting status, SUCCESS or Fail
 *
 */
int wlan_hdd_go_set_mcc_p2p_quota(struct hdd_adapter *hostapd_adapter,
				  uint32_t set_value);
/**
 * wlan_hdd_set_mcc_latency() - Set MCC latency to FW
 * @adapter: Pointer to HDD adapter
 * @set_value: Latency value
 *
 * Sets the MCC latency value during STA-P2P concurrency
 *
 * Return: None
 */
void wlan_hdd_set_mcc_latency(struct hdd_adapter *adapter, int set_value);
#endif /* __P2P_H */
