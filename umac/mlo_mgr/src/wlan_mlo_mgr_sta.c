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
 * DOC: contains MLO manager STA related api's
 */

#include <wlan_cmn.h>
#include "wlan_cm_public_struct.h"

QDF_STATUS mlo_connect(struct wlan_objmgr_vdev *vdev,
		       struct wlan_cm_connect_req *req)
{
/* WIN Specific API*/
/* Command differ logic will be added here */
	return QDF_STATUS_SUCCESS;
}

void mlo_sta_link_up_notify(struct wlan_objmgr_vdev *vdev, uint8_t *mlo_ie)
{
/* Create the secondary interface.
 * call mlo_peer_create();
 * Check if we need to, then issue the connection on second link
 * Send keys if the last link
 */
}

/* STA disconnect */
QDF_STATUS mlo_disconnect(struct wlan_objmgr_vdev *vdev,
			  enum wlan_cm_source source,
			  enum wlan_reason_code reason_code,
			  struct qdf_mac_addr *bssid)
{
/* This API will be pass through if MLO manager/11be is disabled */
	return QDF_STATUS_SUCCESS;
}

void mlo_sta_link_down_notify(struct wlan_objmgr_vdev *vdev)
{
}

bool mlo_is_mld_sta(struct wlan_objmgr_vdev *vdev)
{
	return false;
}
