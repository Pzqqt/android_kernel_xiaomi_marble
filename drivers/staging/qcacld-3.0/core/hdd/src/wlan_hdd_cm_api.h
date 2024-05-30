/*
 * Copyright (c) 2012-2021, The Linux Foundation. All rights reserved.
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

/**
 * DOC: wlan_hdd_cm_api.h
 *
 * WLAN host device driver connect/disconnect functions declaration
 */

#ifndef __WLAN_HDD_CM_API_H
#define __WLAN_HDD_CM_API_H

#include <net/cfg80211.h>
#include "wlan_cm_public_struct.h"
#include "osif_cm_util.h"
#include "wlan_cm_roam_ucfg_api.h"

/**
 * wlan_hdd_cm_connect() - cfg80211 connect api
 * @wiphy: Pointer to wiphy
 * @dev: Pointer to network device
 * @req: Pointer to cfg80211 connect request
 *
 * This function is used to issue connect request to connection manager
 *
 * Context: Any context.
 * Return: 0 for success, non-zero for failure
 */
int wlan_hdd_cm_connect(struct wiphy *wiphy,
			struct net_device *ndev,
			struct cfg80211_connect_params *req);

/**
 * wlan_hdd_cm_issue_disconnect() - initiate disconnect from osif
 * @adapter: Pointer to adapter
 * @reason: Disconnect reason code
 * @sync: true if wait for disconnect to complete is required. for the
 *        supplicant initiated disconnect or during vdev delete/change interface
 *        sync should be true.
 *
 * This function is used to issue disconnect request to conection manager
 *
 * Return: QDF_STATUS
 */
QDF_STATUS wlan_hdd_cm_issue_disconnect(struct hdd_adapter *adapter,
					enum wlan_reason_code reason,
					bool sync);

/**
 * wlan_hdd_cm_disconnect() - cfg80211 disconnect api
 * @wiphy: Pointer to wiphy
 * @dev: Pointer to network device
 * @reason: Disconnect reason code
 *
 * This function is used to issue disconnect request to conection manager
 *
 * Return: 0 for success, non-zero for failure
 */
int wlan_hdd_cm_disconnect(struct wiphy *wiphy,
			   struct net_device *dev, u16 reason);

QDF_STATUS hdd_cm_disconnect_complete(struct wlan_objmgr_vdev *vdev,
				      struct wlan_cm_discon_rsp *rsp,
				      enum osif_cb_type type);

QDF_STATUS hdd_cm_netif_queue_control(struct wlan_objmgr_vdev *vdev,
				      enum netif_action_type action,
				      enum netif_reason_type reason);

QDF_STATUS hdd_cm_connect_complete(struct wlan_objmgr_vdev *vdev,
				   struct wlan_cm_connect_resp *rsp,
				   enum osif_cb_type type);
/**
 * hdd_cm_napi_serialize_control() - NAPI serialize hdd cb
 * @action: serialize or de-serialize NAPI activities
 *
 * This function is for napi serialize
 *
 * Return: qdf status
 */
QDF_STATUS hdd_cm_napi_serialize_control(bool action);

#ifdef WLAN_BOOST_CPU_FREQ_IN_ROAM
/**
 * hdd_cm_perfd_set_cpufreq() - API to set CPU min freq
 * @action: set or reset the CPU freq
 *
 * This function sets/resets the CPU min frequency
 * by sending netlink msg to cnss-daemon, which will
 * communicate to perf daemon to set/reset CPU freq.
 *
 * Return: qdf status
 */

QDF_STATUS hdd_cm_perfd_set_cpufreq(bool action);
#else
static inline
QDF_STATUS hdd_cm_perfd_set_cpufreq(bool action)
{
	return QDF_STATUS_SUCCESS;
}
#endif

#ifdef WLAN_FEATURE_FILS_SK
/**
 * hdd_cm_save_gtk() - save gtk api
 * @vdev: Pointer to vdev
 * @rsp: Pointer to connect rsp
 *
 * This function is used to save gtk in legacy mode
 *
 * Return: QDF_STATUS
 */
QDF_STATUS hdd_cm_save_gtk(struct wlan_objmgr_vdev *vdev,
			   struct wlan_cm_connect_resp *rsp);

/**
 * hdd_cm_set_hlp_data() - api to set hlp data for dhcp
 * @dev: pointer to net device
 * @vdev: Pointer to vdev
 * @rsp: Pointer to connect rsp
 *
 * This function is used to set hlp data for dhcp in legacy mode
 *
 * Return: QDF_STATUS
 */
QDF_STATUS hdd_cm_set_hlp_data(struct net_device *dev,
			       struct wlan_objmgr_vdev *vdev,
			       struct wlan_cm_connect_resp *rsp);
#endif

#ifdef WLAN_FEATURE_PREAUTH_ENABLE
/**
 * hdd_cm_ft_preauth_complete() - send fast transition event
 * @vdev: Pointer to vdev
 * @rsp: Pointer to preauth rsp
 *
 * This function is used to send fast transition event in legacy mode
 *
 * Return: QDF_STATUS
 */
QDF_STATUS hdd_cm_ft_preauth_complete(struct wlan_objmgr_vdev *vdev,
				      struct wlan_preauth_rsp *rsp);

#ifdef FEATURE_WLAN_ESE
/**
 * hdd_cm_cckm_preauth_complete() - send cckm preauth indication to
 * the supplicant via wireless custom event
 * @vdev: Pointer to vdev
 * @rsp: Pointer to preauth rsp
 *
 * This function is used to send cckm preauth indication to
 * the supplicant via wireless custom event in legacy mode
 *
 * Return: QDF_STATUS
 */
QDF_STATUS hdd_cm_cckm_preauth_complete(struct wlan_objmgr_vdev *vdev,
					struct wlan_preauth_rsp *rsp);
#endif
#endif

#ifdef WLAN_FEATURE_MSCS
/**
 * reset_mscs_params() - Reset mscs parameters
 * @adapter: pointer to adapter structure
 *
 * Reset mscs parameters whils disconnection
 *
 * Return: None
 */
void reset_mscs_params(struct hdd_adapter *adapter);
#else
static inline
void reset_mscs_params(struct hdd_adapter *adapter)
{
	return;
}
#endif

/**
 * hdd_handle_disassociation_event() - Handle disassociation event
 * @adapter: Pointer to adapter
 * @peer_macaddr: Pointer to peer mac address
 *
 * Return: None
 */
void hdd_handle_disassociation_event(struct hdd_adapter *adapter,
				     struct qdf_mac_addr *peer_macaddr);

/**
 * __hdd_cm_disconnect_handler_pre_user_update() - Handle disconnect indication
 * before updating to user space
 * @adapter: Pointer to adapter
 *
 * Return: None
 */
void __hdd_cm_disconnect_handler_pre_user_update(struct hdd_adapter *adapter);

/**
 * __hdd_cm_disconnect_handler_post_user_update() - Handle disconnect indication
 * after updating to user space
 * @adapter: Pointer to adapter
 * @vdev: vdev ptr
 *
 * Return: None
 */
void
__hdd_cm_disconnect_handler_post_user_update(struct hdd_adapter *adapter,
					     struct wlan_objmgr_vdev *vdev);

/**
 * hdd_cm_set_peer_authenticate() - set peer as authenticated
 * @adapter: pointer to adapter
 * @bssid: bssid of the connection
 * @is_auth_required: is upper layer authenticatoin required
 *
 * Return: QDF_STATUS enumeration
 */
void hdd_cm_set_peer_authenticate(struct hdd_adapter *adapter,
				  struct qdf_mac_addr *bssid,
				  bool is_auth_required);

/**
 * hdd_cm_update_rssi_snr_by_bssid() - update rsi and snr into adapter
 * @adapter: Pointer to adapter
 *
 * Return: None
 */
void hdd_cm_update_rssi_snr_by_bssid(struct hdd_adapter *adapter);

/**
 *  hdd_cm_handle_assoc_event() - Send disassociation indication to oem
 * app
 * @vdev: Pointer to adapter
 * @peer_mac: Pointer to peer mac address
 *
 * Return: None
 */
void hdd_cm_handle_assoc_event(struct wlan_objmgr_vdev *vdev,
			       uint8_t *peer_mac);

/**
 * hdd_cm_netif_queue_enable() - Enable the network queue for a
 *			      particular adapter.
 * @adapter: pointer to the adapter structure
 *
 * This function schedules a work to update the netdev features
 * and enable the network queue if the feature "disable checksum/tso
 * for legacy connections" is enabled via INI. If not, it will
 * retain the existing behavior by just enabling the network queues.
 *
 * Returns: none
 */
void hdd_cm_netif_queue_enable(struct hdd_adapter *adapter);

/**
 * hdd_cm_clear_pmf_stats() - Clear pmf stats
 * @adapter: pointer to the adapter structure
 *
 * Returns: None
 */
void hdd_cm_clear_pmf_stats(struct hdd_adapter *adapter);

/**
 * hdd_cm_save_connect_status() - Save connect status
 * @adapter: pointer to the adapter structure
 * @reason_code: IEE80211 wlan status code
 *
 * Returns: None
 */
void hdd_cm_save_connect_status(struct hdd_adapter *adapter,
				uint32_t reason_code);

/**
 * hdd_cm_is_vdev_associated() - Checks if vdev is associated or not
 * @adapter: pointer to the adapter structure
 *
 * Returns: True if vdev is associated else false
 */
bool hdd_cm_is_vdev_associated(struct hdd_adapter *adapter);

/**
 * hdd_cm_is_vdev_connected() - Checks if vdev is connected or not
 * @adapter: pointer to the adapter structure
 *
 * Returns: True if vdev is connected else false
 */
bool hdd_cm_is_vdev_connected(struct hdd_adapter *adapter);

/**
 * hdd_cm_is_connecting() - Function to check connection in progress
 * @adapter: pointer to the adapter structure
 *
 * Return: true if connecting, false otherwise
 */
bool hdd_cm_is_connecting(struct hdd_adapter *adapter);

/**
 * hdd_cm_is_disconnected() - Function to check if vdev is disconnected or not
 * @adapter: pointer to the adapter structure
 *
 * Return: true if disconnected, false otherwise
 */
bool hdd_cm_is_disconnected(struct hdd_adapter *adapter);

/**
 * hdd_cm_is_disconnecting() - Function to check disconnection in progress
 * @adapter: pointer to the adapter structure
 *
 * Return: true if disconnecting, false otherwise
 */
bool hdd_cm_is_disconnecting(struct hdd_adapter *adapter);

/**
 * hdd_cm_is_vdev_roaming() - Function to check roaming in progress
 * @adapter: pointer to the adapter structure
 *
 * Return: true if roaming, false otherwise
 */
bool hdd_cm_is_vdev_roaming(struct hdd_adapter *adapter);

#endif /* __WLAN_HDD_CM_API_H */
