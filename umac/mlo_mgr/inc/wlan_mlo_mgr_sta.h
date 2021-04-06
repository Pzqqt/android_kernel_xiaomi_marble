/*
 * Copyright (c) 2021, The Linux Foundation. All rights reserved.
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

/*
 * DOC: contains MLO manager public file containing STA functionality
 */
#ifndef _WLAN_MLO_MGR_STA_H_
#define _WLAN_MLO_MGR_STA_H_

/**
 * mlo_connect - Start the connection process
 *
 * @vdev: pointer to vdev
 * @req: connection request
 *
 * Return: QDF_STATUS
 */
QDF_STATUS mlo_connect(struct wlan_objmgr_vdev *vdev,
		       struct wlan_cm_connect_req *req);

/**
 * mlo_sta_link_up_notify - Called by connection manager to notify the
 * STA link up is complete
 *
 * @vdev: pointer to vdev
 * @mlo_ie: MLO information element
 *
 * Connection manager will notify the MLO manager when the link has started
 * and MLO manager will start the subsequent connections, if necessary
 *
 * Return: none
 */
void mlo_sta_link_up_notify(struct wlan_objmgr_vdev *vdev, uint8_t *mlo_ie);

/**
 * mlo_disconnect - Start the disconnection process
 *
 * @vdev: pointer to vdev
 * @source: source of the request (can be connect or disconnect request)
 * @reason: reason for disconnect
 * @bssid: BSSID
 *
 * Return: QDF_STATUS
 */
QDF_STATUS mlo_disconnect(struct wlan_objmgr_vdev *vdev,
			  enum wlan_cm_source source,
			  enum wlan_reason_code reason_code,
			  struct qdf_mac_addr *bssid);

/**
 * mlo_sta_link_down_notify - Notifies that STA link has gone down
 *
 * @vdev: pointer to vdev
 *
 * Return: none
 */
void mlo_sta_link_down_notify(struct wlan_objmgr_vdev *vdev);

/**
 * mlo_is_mld_sta - Check if MLD associated with the vdev is a station
 *
 * @vdev: pointer to vdev
 *
 * Return: true if MLD is a station, false otherwise
 */
bool mlo_is_mld_sta(struct wlan_objmgr_vdev *vdev);

#endif
