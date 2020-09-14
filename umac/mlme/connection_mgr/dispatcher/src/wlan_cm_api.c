/*
 * Copyright (c) 2012-2015, 2020, The Linux Foundation. All rights reserved.
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
 * DOC: wlan_cm_api.c
 *
 * This file maintains definitaions public apis.
 */

#include <wlan_cm_api.h>
#include "connection_mgr/core/src/wlan_cm_main_api.h"

QDF_STATUS wlan_cm_start_connect(struct wlan_objmgr_vdev *vdev,
				 struct wlan_cm_connect_req *req)
{
	return cm_connect_start_req(vdev, req);
}

QDF_STATUS wlan_cm_start_disconnect(struct wlan_objmgr_vdev *vdev,
				    struct wlan_cm_disconnect_req *req)
{
	return cm_disconnect_start_req(vdev, req);
}

QDF_STATUS wlan_cm_bss_peer_create_rsp(struct wlan_objmgr_vdev *vdev,
				       QDF_STATUS status,
				       struct qdf_mac_addr *peer_mac)
{
	return cm_bss_peer_create_rsp(vdev, status, peer_mac);
}

QDF_STATUS wlan_cm_connect_rsp(struct wlan_objmgr_vdev *vdev,
			       struct wlan_cm_connect_rsp *cm_conn_rsp)
{
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS wlan_cm_bss_peer_delete_ind(struct wlan_objmgr_vdev *vdev,
				       struct qdf_mac_addr *peer_mac)
{
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS wlan_cm_bss_peer_delete_rsp(struct wlan_objmgr_vdev *vdev,
				       uint32_t status)
{
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS wlan_cm_disconnect_rsp(struct wlan_objmgr_vdev *vdev,
				  struct wlan_cm_discon_rsp cm_discon_rsp)
{
	return QDF_STATUS_SUCCESS;
}

#ifdef WLAN_POLICY_MGR_ENABLE
void wlan_cm_hw_mode_change_resp(struct wlan_objmgr_pdev *pdev, uint8_t vdev_id,
				 wlan_cm_id cm_id, QDF_STATUS status)
{
	cm_hw_mode_change_resp(pdev, vdev_id, cm_id, status);
}
#endif /* ifdef POLICY_MGR_ENABLE */
