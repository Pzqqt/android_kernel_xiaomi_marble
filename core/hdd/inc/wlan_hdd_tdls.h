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

#ifndef __HDD_TDLS_H
#define __HDD_TDLS_H
/**
 * DOC: wlan_hdd_tdls.h
 * WLAN Host Device Driver TDLS include file
 */

struct hdd_context;

/**
 * enum tdls_concerned_external_events - External events that affect TDLS
 * @P2P_ROC_START: P2P remain on channel starts
 * @P2P_ROC_END: P2P remain on channel ends
 */
enum tdls_concerned_external_events {
	P2P_ROC_START,
	P2P_ROC_END,
};

#ifdef FEATURE_WLAN_TDLS

/* Bit mask flag for tdls_option to FW */
#define ENA_TDLS_OFFCHAN      (1 << 0)  /* TDLS Off Channel support */
#define ENA_TDLS_BUFFER_STA   (1 << 1)  /* TDLS Buffer STA support */
#define ENA_TDLS_SLEEP_STA    (1 << 2)  /* TDLS Sleep STA support */
/**
 * struct hdd_tdls_config_params - tdls config params
 *
 * @tdls: tdls
 * @tx_period_t: tx period
 * @tx_packet_n: tx packets number
 * @discovery_tries_n: discovery tries
 * @idle_timeout_t: idle traffic time out value
 * @idle_packet_n: idle packet number
 * @rssi_trigger_threshold: rssi trigger threshold
 * @rssi_teardown_threshold: rssi tear down threshold
 * @rssi_delta: rssi delta
 */
struct hdd_tdls_config_params {
	uint32_t tdls;
	uint32_t tx_period_t;
	uint32_t tx_packet_n;
	uint32_t discovery_tries_n;
	uint32_t idle_timeout_t;
	uint32_t idle_packet_n;
	int32_t rssi_trigger_threshold;
	int32_t rssi_teardown_threshold;
	int32_t rssi_delta;
};

typedef int (*cfg80211_exttdls_callback)(const uint8_t *mac,
					 uint32_t opclass,
					 uint32_t channel,
					 uint32_t state,
					 int32_t reason, void *ctx);

/**
 * struct tdlsInfo_t - tdls info
 *
 * @vdev_id: vdev id
 * @tdls_state: tdls state
 * @notification_interval_ms: notification interval in ms
 * @tx_discovery_threshold: tx discovery threshold
 * @tx_teardown_threshold: tx teardown threshold
 * @rssi_teardown_threshold: rx teardown threshold
 * @rssi_delta: rssi delta
 * @tdls_options: tdls options
 * @peer_traffic_ind_window: peer traffic indication window
 * @peer_traffic_response_timeout: peer traffic response timeout
 * @puapsd_mask: puapsd mask
 * @puapsd_inactivity_time: puapsd inactivity time
 * @puapsd_rx_frame_threshold: puapsd rx frame threshold
 * @teardown_notification_ms: tdls teardown notification interval
 * @tdls_peer_kickout_threshold: tdls packets threshold
 *    for peer kickout operation
 */
typedef struct {
	uint32_t vdev_id;
	uint32_t tdls_state;
	uint32_t notification_interval_ms;
	uint32_t tx_discovery_threshold;
	uint32_t tx_teardown_threshold;
	int32_t rssi_teardown_threshold;
	int32_t rssi_delta;
	uint32_t tdls_options;
	uint32_t peer_traffic_ind_window;
	uint32_t peer_traffic_response_timeout;
	uint32_t puapsd_mask;
	uint32_t puapsd_inactivity_time;
	uint32_t puapsd_rx_frame_threshold;
	uint32_t teardown_notification_ms;
	uint32_t tdls_peer_kickout_threshold;
} tdlsInfo_t;

int wlan_hdd_tdls_set_params(struct net_device *dev,
			     struct hdd_tdls_config_params *config);

int wlan_hdd_tdls_get_all_peers(struct hdd_adapter *adapter, char *buf,
				int buflen);

int wlan_hdd_tdls_extctrl_deconfig_peer(struct hdd_adapter *adapter,
					const uint8_t *peer);
int wlan_hdd_tdls_extctrl_config_peer(struct hdd_adapter *adapter,
				      const uint8_t *peer,
				      cfg80211_exttdls_callback callback,
				      uint32_t chan,
				      uint32_t max_latency,
				      uint32_t op_class,
				      uint32_t min_bandwidth);

int wlan_hdd_cfg80211_exttdls_enable(struct wiphy *wiphy,
				     struct wireless_dev *wdev,
				     const void *data,
				     int data_len);

int wlan_hdd_cfg80211_exttdls_disable(struct wiphy *wiphy,
				      struct wireless_dev *wdev,
				      const void *data,
				      int data_len);

int wlan_hdd_cfg80211_exttdls_get_status(struct wiphy *wiphy,
					 struct wireless_dev *wdev,
					 const void *data,
					 int data_len);

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 16, 0))
int wlan_hdd_cfg80211_tdls_oper(struct wiphy *wiphy,
				struct net_device *dev,
				const uint8_t *peer,
				enum nl80211_tdls_operation oper);
#else
int wlan_hdd_cfg80211_tdls_oper(struct wiphy *wiphy,
				struct net_device *dev,
				uint8_t *peer,
				enum nl80211_tdls_operation oper);
#endif

#ifdef TDLS_MGMT_VERSION2
int wlan_hdd_cfg80211_tdls_mgmt(struct wiphy *wiphy,
				struct net_device *dev, u8 *peer,
				u8 action_code, u8 dialog_token,
				u16 status_code, u32 peer_capability,
				const u8 *buf, size_t len);
#else
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 17, 0))
int wlan_hdd_cfg80211_tdls_mgmt(struct wiphy *wiphy,
				struct net_device *dev, const uint8_t *peer,
				uint8_t action_code, uint8_t dialog_token,
				uint16_t status_code, uint32_t peer_capability,
				bool initiator, const uint8_t *buf,
				size_t len);
#elif (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 16, 0))
int wlan_hdd_cfg80211_tdls_mgmt(struct wiphy *wiphy,
				struct net_device *dev, const uint8_t *peer,
				uint8_t action_code, uint8_t dialog_token,
				uint16_t status_code, uint32_t peer_capability,
				const uint8_t *buf, size_t len);
#elif (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 15, 0))
int wlan_hdd_cfg80211_tdls_mgmt(struct wiphy *wiphy,
				struct net_device *dev, uint8_t *peer,
				uint8_t action_code, uint8_t dialog_token,
				uint16_t status_code, uint32_t peer_capability,
				const uint8_t *buf, size_t len);
#else
int wlan_hdd_cfg80211_tdls_mgmt(struct wiphy *wiphy,
				struct net_device *dev, uint8_t *peer,
				uint8_t action_code, uint8_t dialog_token,
				uint16_t status_code, const uint8_t *buf,
				size_t len);
#endif
#endif

int hdd_set_tdls_offchannel(struct hdd_context *hdd_ctx, int offchannel);
int hdd_set_tdls_secoffchanneloffset(struct hdd_context *hdd_ctx,
				     int offchanoffset);
int hdd_set_tdls_offchannelmode(struct hdd_adapter *adapter, int offchanmode);
int hdd_set_tdls_scan_type(struct hdd_context *hdd_ctx, int val);
int wlan_hdd_tdls_antenna_switch(struct hdd_context *hdd_ctx,
				 struct hdd_adapter *adapter,
				 uint32_t mode);

/**
 * wlan_hdd_cfg80211_configure_tdls_mode() - configure tdls mode
 * @wiphy:   pointer to wireless wiphy structure.
 * @wdev:    pointer to wireless_dev structure.
 * @data:    Pointer to the data to be passed via vendor interface
 * @data_len:Length of the data to be passed
 *
 * Return:   Return the Success or Failure code.
 */
int wlan_hdd_cfg80211_configure_tdls_mode(struct wiphy *wiphy,
					struct wireless_dev *wdev,
					const void *data,
					int data_len);

QDF_STATUS hdd_tdls_register_peer(void *userdata, uint32_t vdev_id,
				  const uint8_t *mac, uint16_t sta_id,
				  uint8_t qos);

QDF_STATUS hdd_tdls_deregister_peer(void *userdata, uint32_t vdev_id,
				    uint8_t sta_id);

#else

static inline int wlan_hdd_tdls_antenna_switch(struct hdd_context *hdd_ctx,
					       struct hdd_adapter *adapter,
					       uint32_t mode)
{
	return 0;
}

static inline int wlan_hdd_cfg80211_configure_tdls_mode(struct wiphy *wiphy,
					struct wireless_dev *wdev,
					const void *data,
					int data_len)
{
	return 0;
}

static inline void
hdd_tdls_notify_p2p_roc(struct hdd_context *hdd_ctx,
			enum tdls_concerned_external_events event)
{
}

static inline
QDF_STATUS hdd_tdls_register_peer(void *userdata, uint32_t vdev_id,
				  const uint8_t *mac, uint16_t sta_id,
				  uint8_t qos);
{
}

static inline
QDF_STATUS hdd_tdls_deregister_peer(void *userdata, uint32_t vdev_id,
				    uint8_t sta_id)
{
}
#endif /* End of FEATURE_WLAN_TDLS */
#endif /* __HDD_TDLS_H */
