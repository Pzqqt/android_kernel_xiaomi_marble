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

QDF_STATUS wlan_cm_disconnect(struct wlan_objmgr_vdev *vdev,
			      enum wlan_cm_source source,
			      enum wlan_reason_code reason_code,
			      struct qdf_mac_addr *bssid)
{
	struct wlan_cm_disconnect_req req = {0};

	req.vdev_id = wlan_vdev_get_id(vdev);
	req.source = source;
	req.reason_code = reason_code;
	if (bssid)
		qdf_copy_macaddr(&req.bssid, bssid);

	return cm_disconnect_start_req(vdev, &req);
}

QDF_STATUS wlan_cm_disconnect_sync(struct wlan_objmgr_vdev *vdev,
				   enum wlan_cm_source source,
				   enum wlan_reason_code reason_code)
{
	struct wlan_cm_disconnect_req req = {0};

	req.vdev_id = wlan_vdev_get_id(vdev);
	req.source = source;
	req.reason_code = reason_code;

	return cm_disconnect_start_req_sync(vdev, &req);
}

QDF_STATUS wlan_cm_bss_select_ind_rsp(struct wlan_objmgr_vdev *vdev,
				      QDF_STATUS status)
{
	return cm_bss_select_ind_rsp(vdev, status);
}

QDF_STATUS wlan_cm_bss_peer_create_rsp(struct wlan_objmgr_vdev *vdev,
				       QDF_STATUS status,
				       struct qdf_mac_addr *peer_mac)
{
	return cm_bss_peer_create_rsp(vdev, status, peer_mac);
}

QDF_STATUS wlan_cm_connect_rsp(struct wlan_objmgr_vdev *vdev,
			       struct wlan_cm_connect_resp *resp)
{
	return cm_connect_rsp(vdev, resp);
}

QDF_STATUS wlan_cm_bss_peer_delete_ind(struct wlan_objmgr_vdev *vdev,
				       struct qdf_mac_addr *peer_mac)
{
	return cm_bss_peer_delete_req(vdev, peer_mac);
}

QDF_STATUS wlan_cm_bss_peer_delete_rsp(struct wlan_objmgr_vdev *vdev,
				       uint32_t status)
{
	return cm_vdev_down_req(vdev, status);
}

QDF_STATUS wlan_cm_disconnect_rsp(struct wlan_objmgr_vdev *vdev,
				  struct wlan_cm_discon_rsp *resp)
{
	return cm_disconnect_rsp(vdev, resp);
}

void wlan_cm_set_max_connect_attempts(struct wlan_objmgr_vdev *vdev,
				      uint8_t max_connect_attempts)
{
	cm_set_max_connect_attempts(vdev, max_connect_attempts);
}

bool wlan_cm_is_vdev_connecting(struct wlan_objmgr_vdev *vdev)
{
	return cm_is_vdev_connecting(vdev);
}

bool wlan_cm_is_vdev_connected(struct wlan_objmgr_vdev *vdev)
{
	return cm_is_vdev_connected(vdev);
}

bool wlan_cm_is_vdev_disconnecting(struct wlan_objmgr_vdev *vdev)
{
	return cm_is_vdev_disconnecting(vdev);
}

bool wlan_cm_is_vdev_disconnected(struct wlan_objmgr_vdev *vdev)
{
	return cm_is_vdev_disconnected(vdev);
}

bool wlan_cm_is_vdev_roaming(struct wlan_objmgr_vdev *vdev)
{
	return cm_is_vdev_roaming(vdev);
}

const char *wlan_cm_reason_code_to_str(enum wlan_reason_code reason)
{
	if (reason > REASON_PROP_START)
		return "";

	switch (reason) {
	CASE_RETURN_STRING(REASON_UNSPEC_FAILURE);
	CASE_RETURN_STRING(REASON_PREV_AUTH_NOT_VALID);
	CASE_RETURN_STRING(REASON_DEAUTH_NETWORK_LEAVING);
	CASE_RETURN_STRING(REASON_DISASSOC_DUE_TO_INACTIVITY);
	CASE_RETURN_STRING(REASON_DISASSOC_AP_BUSY);
	CASE_RETURN_STRING(REASON_CLASS2_FRAME_FROM_NON_AUTH_STA);
	CASE_RETURN_STRING(REASON_CLASS3_FRAME_FROM_NON_ASSOC_STA);
	CASE_RETURN_STRING(REASON_DISASSOC_NETWORK_LEAVING);
	CASE_RETURN_STRING(REASON_STA_NOT_AUTHENTICATED);
	CASE_RETURN_STRING(REASON_BAD_PWR_CAPABILITY);
	CASE_RETURN_STRING(REASON_BAD_SUPPORTED_CHANNELS);
	CASE_RETURN_STRING(REASON_DISASSOC_BSS_TRANSITION);
	CASE_RETURN_STRING(REASON_INVALID_IE);
	CASE_RETURN_STRING(REASON_MIC_FAILURE);
	CASE_RETURN_STRING(REASON_4WAY_HANDSHAKE_TIMEOUT);
	CASE_RETURN_STRING(REASON_GROUP_KEY_UPDATE_TIMEOUT);
	CASE_RETURN_STRING(REASON_IN_4WAY_DIFFERS);
	CASE_RETURN_STRING(REASON_INVALID_GROUP_CIPHER);
	CASE_RETURN_STRING(REASON_INVALID_PAIRWISE_CIPHER);
	CASE_RETURN_STRING(REASON_INVALID_AKMP);
	CASE_RETURN_STRING(REASON_UNSUPPORTED_RSNE_VER);
	CASE_RETURN_STRING(REASON_INVALID_RSNE_CAPABILITIES);
	CASE_RETURN_STRING(REASON_1X_AUTH_FAILURE);
	CASE_RETURN_STRING(REASON_CIPHER_SUITE_REJECTED);
	CASE_RETURN_STRING(REASON_TDLS_PEER_UNREACHABLE);
	CASE_RETURN_STRING(REASON_TDLS_UNSPEC);
	CASE_RETURN_STRING(REASON_DISASSOC_SSP_REQUESTED);
	CASE_RETURN_STRING(REASON_NO_SSP_ROAMING_AGREEMENT);
	CASE_RETURN_STRING(REASON_BAD_CIPHER_OR_AKM);
	CASE_RETURN_STRING(REASON_LOCATION_NOT_AUTHORIZED);
	CASE_RETURN_STRING(REASON_SERVICE_CHANGE_PRECLUDES_TS);
	CASE_RETURN_STRING(REASON_QOS_UNSPECIFIED);
	CASE_RETURN_STRING(REASON_NO_BANDWIDTH);
	CASE_RETURN_STRING(REASON_XS_UNACKED_FRAMES);
	CASE_RETURN_STRING(REASON_EXCEEDED_TXOP);
	CASE_RETURN_STRING(REASON_STA_LEAVING);
	CASE_RETURN_STRING(REASON_END_TS_BA_DLS);
	CASE_RETURN_STRING(REASON_UNKNOWN_TS_BA);
	CASE_RETURN_STRING(REASON_TIMEDOUT);
	CASE_RETURN_STRING(REASON_PEERKEY_MISMATCH);
	CASE_RETURN_STRING(REASON_AUTHORIZED_ACCESS_LIMIT_REACHED);
	CASE_RETURN_STRING(REASON_EXTERNAL_SERVICE_REQUIREMENTS);
	CASE_RETURN_STRING(REASON_INVALID_FT_ACTION_FRAME_COUNT);
	CASE_RETURN_STRING(REASON_INVALID_PMKID);
	CASE_RETURN_STRING(REASON_INVALID_MDE);
	CASE_RETURN_STRING(REASON_INVALID_FTE);
	CASE_RETURN_STRING(REASON_MESH_PEERING_CANCELLED);
	CASE_RETURN_STRING(REASON_MESH_MAX_PEERS);
	CASE_RETURN_STRING(REASON_MESH_CONFIG_POLICY_VIOLATION);
	CASE_RETURN_STRING(REASON_MESH_CLOSE_RCVD);
	CASE_RETURN_STRING(REASON_MESH_MAX_RETRIES);
	CASE_RETURN_STRING(REASON_MESH_CONFIRM_TIMEOUT);
	CASE_RETURN_STRING(REASON_MESH_INVALID_GTK);
	CASE_RETURN_STRING(REASON_MESH_INCONSISTENT_PARAMS);
	CASE_RETURN_STRING(REASON_MESH_INVALID_SECURITY_CAP);
	CASE_RETURN_STRING(REASON_MESH_PATH_ERROR_NO_PROXY_INFO);
	CASE_RETURN_STRING(REASON_MESH_PATH_ERROR_NO_FORWARDING_INFO);
	CASE_RETURN_STRING(REASON_MESH_PATH_ERROR_DEST_UNREACHABLE);
	CASE_RETURN_STRING(REASON_MAC_ADDRESS_ALREADY_EXISTS_IN_MBSS);
	CASE_RETURN_STRING(REASON_MESH_CHANNEL_SWITCH_REGULATORY_REQ);
	CASE_RETURN_STRING(REASON_MESH_CHANNEL_SWITCH_UNSPECIFIED);
	CASE_RETURN_STRING(REASON_POOR_RSSI_CONDITIONS);
	default:
		return "Unknown";
	}
}

#ifdef WLAN_POLICY_MGR_ENABLE
void wlan_cm_hw_mode_change_resp(struct wlan_objmgr_pdev *pdev, uint8_t vdev_id,
				 wlan_cm_id cm_id, QDF_STATUS status)
{
	cm_hw_mode_change_resp(pdev, vdev_id, cm_id, status);
}
#endif /* ifdef POLICY_MGR_ENABLE */
