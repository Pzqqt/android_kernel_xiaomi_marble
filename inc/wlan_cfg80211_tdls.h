/*
 * Copyright (c) 2017 The Linux Foundation. All rights reserved.
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
 * DOC: declares driver functions interfacing with linux kernel
 */

#ifndef _WLAN_CFG80211_TDLS_H_
#define _WLAN_CFG80211_TDLS_H_

#include <linux/version.h>
#include <linux/netdevice.h>
#include <linux/completion.h>
#include <net/cfg80211.h>
#include <qca_vendor.h>
#include <wlan_tdls_public_structs.h>
#include <qdf_list.h>
#include <qdf_types.h>
#include <wlan_tdls_ucfg_api.h>

/**
 * struct osif_tdls_vdev - OS tdls vdev private structure
 * @tdls_add_peer_comp: Completion to add tdls peer
 * @tdls_del_peer_comp: Completion to delete tdls peer
 * @tdls_mgmt_comp: Completion to send tdls mgmt packets
 * @tdls_link_establish_req_comp: Completion to establish link, sync to
 * send establish params to firmware, not used today.
 * @tdls_add_peer_status: Peer status after add peer
 * @mgmt_tx_completion_status: Tdls mgmt frames TX completion status code
 */
struct osif_tdls_vdev {
	struct completion tdls_add_peer_comp;
	struct completion tdls_del_peer_comp;
	struct completion tdls_mgmt_comp;
	struct completion tdls_link_establish_req_comp;
	QDF_STATUS tdls_add_peer_status;
	uint32_t mgmt_tx_completion_status;
};

/**
 * wlan_cfg80211_tdls_priv_init() - API to initialize tdls os private
 * @osif_priv: vdev os private
 *
 * API to initialize tdls os private
 *
 * Return: QDF_STATUS
 */
QDF_STATUS wlan_cfg80211_tdls_priv_init(struct vdev_osif_priv *osif_priv);

/**
 * wlan_cfg80211_tdls_priv_deinit() - API to deinitialize tdls os private
 * @osif_priv: vdev os private
 *
 * API to deinitialize tdls os private
 *
 * Return: None
 */
void wlan_cfg80211_tdls_priv_deinit(struct vdev_osif_priv *osif_priv);

/**
 * wlan_cfg80211_tdls_add_peer() - process cfg80211 add TDLS peer request
 * @pdev: pdev object
 * @dev: Pointer to net device
 * @mac: MAC address for TDLS peer
 *
 * Return: 0 for success; negative errno otherwise
 */
int wlan_cfg80211_tdls_add_peer(struct wlan_objmgr_pdev *pdev,
				struct net_device *dev, const uint8_t *mac);

/**
 * wlan_cfg80211_tdls_update_peer() - process cfg80211 update TDLS peer request
 * @pdev: pdev object
 * @dev: Pointer to net device
 * @mac: MAC address for TDLS peer
 * @params: Pointer to station parameters
 *
 * Return: 0 for success; negative errno otherwise
 */
int wlan_cfg80211_tdls_update_peer(struct wlan_objmgr_pdev *pdev,
				   struct net_device *dev,
				   const uint8_t *mac,
				   struct station_parameters *params);
/**
 * wlan_cfg80211_tdls_oper() - process cfg80211 operation on an TDLS peer
 * @pdev: pdev object
 * @dev: net device
 * @peer: MAC address of the TDLS peer
 * @oper: cfg80211 TDLS operation
 *
 * Return: 0 on success; negative errno otherwise
 */
int wlan_cfg80211_tdls_oper(struct wlan_objmgr_pdev *pdev,
			    struct net_device *dev,
			    const uint8_t *peer,
			    enum nl80211_tdls_operation oper);

/**
 * wlan_cfg80211_tdls_event_callback() - callback for tdls module
 * @userdata: user data
 * @type: request callback type
 * @param: passed parameter
 *
 * This is used by TDLS to sync with os interface
 *
 * Return: None
 */
void wlan_cfg80211_tdls_event_callback(void *userdata,
				       enum tdls_event_type type,
				       struct tdls_osif_indication *param);
#endif
