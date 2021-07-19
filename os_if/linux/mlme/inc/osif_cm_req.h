/*
 * Copyright (c) 2012-2015,2020-2021 The Linux Foundation. All rights reserved.
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
 * DOC: osif_cm_req.h
 *
 * This header file maintains declarations of connect, disconnect, roam
 * request apis.
 */

#ifndef __OSIF_CM_REQ_H
#define __OSIF_CM_REQ_H

#include "qdf_status.h"
#include "wlan_cm_public_struct.h"
#include <net/cfg80211.h>
#include "wlan_objmgr_vdev_obj.h"

/**
 * struct osif_connect_params - extra connect params
 * @scan_ie: default scan ie to be used in connect scan and unicast probe req
 * during connect
 * @force_rsne_override: force the arbitrary rsne received in connect req to be
 * used with out validation, used for the scenarios where the device is used
 * as a testbed device with special functionality and not recommended
 * for production.
 * @dot11mode_filter: dot11mode filter used to restrict connection to
 * 11n/11ac/11ax.
 * @sae_pwe: SAE mechanism for PWE derivation
 *           0 = hunting-and-pecking loop only
 *           1 = hash-to-element only
 *           2 = both hunting-and-pecking loop and hash-to-element enabled
 * @prev_bssid: Previous bssid in case of roam scenario
 */
struct osif_connect_params {
	struct element_info scan_ie;
	bool force_rsne_override;
	enum dot11_mode_filter dot11mode_filter;
	uint8_t sae_pwe;
	struct qdf_mac_addr prev_bssid;
};

#if defined(WLAN_FEATURE_FILS_SK) && \
	(defined(CFG80211_FILS_SK_OFFLOAD_SUPPORT) || \
		 (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 12, 0)))
/**
 * osif_cm_get_fils_auth_type() - get fils auth type
 * @auth: nl auth type
 *
 * Return: enum wlan_fils_auth_type
 */
enum wlan_fils_auth_type
osif_cm_get_fils_auth_type(enum nl80211_auth_type auth);
#endif

/**
 * osif_cm_connect() - Connect start request
 * @dev: net dev
 * @vdev: vdev pointer
 * @req: connect req
 * @params: connect params
 *
 * Return: int
 */
int osif_cm_connect(struct net_device *dev, struct wlan_objmgr_vdev *vdev,
		    const struct cfg80211_connect_params *req,
		    const struct osif_connect_params *params);

/**
 * osif_cm_disconnect() - Disconnect start request
 * @dev: net dev
 * @vdev: vdev pointer
 * @reason: disconnect reason
 *
 * Return: int
 */
int osif_cm_disconnect(struct net_device *dev, struct wlan_objmgr_vdev *vdev,
		       uint16_t reason);

/**
 * osif_cm_disconnect_sync() - Disconnect vdev and wait for it to complete
 * @vdev: vdev pointer
 * @reason: disconnect reason
 *
 * Return: int
 */
int osif_cm_disconnect_sync(struct wlan_objmgr_vdev *vdev, uint16_t reason);

#endif /* __OSIF_CM_REQ_H */
