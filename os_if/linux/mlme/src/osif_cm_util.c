/*
 * Copyright (c) 2012-2015, 2020 The Linux Foundation. All rights reserved.
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
 * DOC: osif_cm_util.c
 *
 * This file maintains definitaions of connect, disconnect, roam
 * common apis.
 */
#include <include/wlan_mlme_cmn.h>
#include "osif_cm_util.h"
#include "wlan_osif_priv.h"
#include "wlan_cfg80211.h"
#include "osif_cm_rsp.h"

const char *
osif_cm_qca_reason_to_str(enum qca_disconnect_reason_codes reason)
{
	switch (reason) {
	CASE_RETURN_STRING(QCA_DISCONNECT_REASON_INTERNAL_ROAM_FAILURE);
	CASE_RETURN_STRING(QCA_DISCONNECT_REASON_EXTERNAL_ROAM_FAILURE);
	CASE_RETURN_STRING(QCA_DISCONNECT_REASON_GATEWAY_REACHABILITY_FAILURE);
	CASE_RETURN_STRING(QCA_DISCONNECT_REASON_UNSUPPORTED_CHANNEL_CSA);
	CASE_RETURN_STRING(QCA_DISCONNECT_REASON_OPER_CHANNEL_DISABLED_INDOOR);
	CASE_RETURN_STRING(QCA_DISCONNECT_REASON_OPER_CHANNEL_USER_DISABLED);
	CASE_RETURN_STRING(QCA_DISCONNECT_REASON_DEVICE_RECOVERY);
	CASE_RETURN_STRING(QCA_DISCONNECT_REASON_KEY_TIMEOUT);
	CASE_RETURN_STRING(QCA_DISCONNECT_REASON_OPER_CHANNEL_BAND_CHANGE);
	CASE_RETURN_STRING(QCA_DISCONNECT_REASON_IFACE_DOWN);
	CASE_RETURN_STRING(QCA_DISCONNECT_REASON_PEER_XRETRY_FAIL);
	CASE_RETURN_STRING(QCA_DISCONNECT_REASON_PEER_INACTIVITY);
	CASE_RETURN_STRING(QCA_DISCONNECT_REASON_SA_QUERY_TIMEOUT);
	CASE_RETURN_STRING(QCA_DISCONNECT_REASON_BEACON_MISS_FAILURE);
	CASE_RETURN_STRING(QCA_DISCONNECT_REASON_CHANNEL_SWITCH_FAILURE);
	CASE_RETURN_STRING(QCA_DISCONNECT_REASON_USER_TRIGGERED);
	case QCA_DISCONNECT_REASON_UNSPECIFIED:
		return "";
	default:
		return "Unknown";
	}
}

enum qca_disconnect_reason_codes
osif_cm_mac_to_qca_reason(enum wlan_reason_code internal_reason)
{
	enum qca_disconnect_reason_codes reason =
					QCA_DISCONNECT_REASON_UNSPECIFIED;

	if (internal_reason < REASON_PROP_START)
		return reason;

	switch (internal_reason) {
	case REASON_HOST_TRIGGERED_ROAM_FAILURE:
		reason = QCA_DISCONNECT_REASON_INTERNAL_ROAM_FAILURE;
		break;
	case REASON_FW_TRIGGERED_ROAM_FAILURE:
		reason = QCA_DISCONNECT_REASON_EXTERNAL_ROAM_FAILURE;
		break;
	case REASON_GATEWAY_REACHABILITY_FAILURE:
		reason =
		QCA_DISCONNECT_REASON_GATEWAY_REACHABILITY_FAILURE;
		break;
	case REASON_UNSUPPORTED_CHANNEL_CSA:
		reason = QCA_DISCONNECT_REASON_UNSUPPORTED_CHANNEL_CSA;
		break;
	case REASON_OPER_CHANNEL_DISABLED_INDOOR:
		reason =
		QCA_DISCONNECT_REASON_OPER_CHANNEL_DISABLED_INDOOR;
		break;
	case REASON_OPER_CHANNEL_USER_DISABLED:
		reason =
		QCA_DISCONNECT_REASON_OPER_CHANNEL_USER_DISABLED;
		break;
	case REASON_DEVICE_RECOVERY:
		reason = QCA_DISCONNECT_REASON_DEVICE_RECOVERY;
		break;
	case REASON_KEY_TIMEOUT:
		reason = QCA_DISCONNECT_REASON_KEY_TIMEOUT;
		break;
	case REASON_OPER_CHANNEL_BAND_CHANGE:
		reason = QCA_DISCONNECT_REASON_OPER_CHANNEL_BAND_CHANGE;
		break;
	case REASON_IFACE_DOWN:
		reason = QCA_DISCONNECT_REASON_IFACE_DOWN;
		break;
	case REASON_PEER_XRETRY_FAIL:
		reason = QCA_DISCONNECT_REASON_PEER_XRETRY_FAIL;
		break;
	case REASON_PEER_INACTIVITY:
		reason = QCA_DISCONNECT_REASON_PEER_INACTIVITY;
		break;
	case REASON_SA_QUERY_TIMEOUT:
		reason = QCA_DISCONNECT_REASON_SA_QUERY_TIMEOUT;
		break;
	case REASON_CHANNEL_SWITCH_FAILED:
		reason = QCA_DISCONNECT_REASON_CHANNEL_SWITCH_FAILURE;
		break;
	case REASON_BEACON_MISSED:
		reason = QCA_DISCONNECT_REASON_BEACON_MISS_FAILURE;
		break;
	case REASON_USER_TRIGGERED_ROAM_FAILURE:
		reason = QCA_DISCONNECT_REASON_USER_TRIGGERED;
		break;
	default:
		osif_debug("No QCA reason code for mac reason: %u",
			   internal_reason);
		/* Unspecified reason by default */
	}

	return reason;
}

void osif_cm_reset_id_and_src_no_lock(struct vdev_osif_priv *osif_priv)
{
	osif_priv->cm_info.last_id = CM_ID_INVALID;
	osif_priv->cm_info.last_source = CM_SOURCE_INVALID;
}

QDF_STATUS osif_cm_reset_id_and_src(struct wlan_objmgr_vdev *vdev)
{
	struct vdev_osif_priv *osif_priv = wlan_vdev_get_ospriv(vdev);

	if (!osif_priv) {
		osif_err("Invalid vdev osif priv");
		return QDF_STATUS_E_INVAL;
	}
	qdf_spinlock_acquire(&osif_priv->cm_info.cmd_id_lock);
	osif_cm_reset_id_and_src_no_lock(osif_priv);
	qdf_spinlock_release(&osif_priv->cm_info.cmd_id_lock);

	return QDF_STATUS_SUCCESS;
}

/**
 * osif_cm_connect_complete_cb() - Connect complete callback
 * @vdev: vdev pointer
 * @rsp: connect response
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS
osif_cm_connect_complete_cb(struct wlan_objmgr_vdev *vdev,
			    struct wlan_cm_connect_rsp *rsp)
{
	return osif_connect_handler(vdev, rsp);
}

/**
 * osif_cm_failed_candidate_cb() - Callback to indicate failed candidate
 * @vdev: vdev pointer
 * @rsp: connect response
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS
osif_cm_failed_candidate_cb(struct wlan_objmgr_vdev *vdev,
			    struct wlan_cm_connect_rsp *rsp)
{
	return osif_failed_candidate_handler(vdev, rsp);
}

/**
 * osif_cm_update_id_and_src_cb() - Callback to update id and
 * source of the connect/disconnect request
 * @vdev: vdev pointer
 * @Source: Source of the connect req
 * @id: Connect/disconnect id
 *
 * Context: Any context. Takes and releases cmd id spinlock
 * Return: QDF_STATUS
 */
static QDF_STATUS
osif_cm_update_id_and_src_cb(struct wlan_objmgr_vdev *vdev,
			     enum wlan_cm_source source, wlan_cm_id cm_id)
{
	struct vdev_osif_priv *osif_priv = wlan_vdev_get_ospriv(vdev);

	if (!osif_priv) {
		osif_err("Invalid vdev osif priv");
		return QDF_STATUS_E_INVAL;
	}

	qdf_spinlock_acquire(&osif_priv->cm_info.cmd_id_lock);
	osif_priv->cm_info.last_id = cm_id;
	osif_priv->cm_info.last_source = source;
	qdf_spinlock_release(&osif_priv->cm_info.cmd_id_lock);

	return QDF_STATUS_SUCCESS;
}

/**
 * osif_cm_disconnect_complete_cb() - Disconnect done callback
 * @vdev: vdev pointer
 * @disconnect_rsp: Disconnect response
 *
 * Context: Any context
 * Return: QDF_STATUS
 */

static QDF_STATUS
osif_cm_disconnect_complete_cb(struct wlan_objmgr_vdev *vdev,
			       struct wlan_cm_discon_rsp *rsp)
{
	return osif_disconnect_handler(vdev, rsp);
}

/**
 * osif_cm_disconnect_start_cb() - Disconnect start callback
 * @vdev: vdev pointer
 *
 * This callback indicates os_if that disconnection is started
 * so that os_if can stop all the activity on this connection
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS
osif_cm_disconnect_start_cb(struct wlan_objmgr_vdev *vdev)
{
	return QDF_STATUS_SUCCESS;
}

static struct mlme_cm_ops cm_ops = {
	.mlme_cm_connect_complete_cb = osif_cm_connect_complete_cb,
	.mlme_cm_failed_candidate_cb = osif_cm_failed_candidate_cb,
	.mlme_cm_update_id_and_src_cb = osif_cm_update_id_and_src_cb,
	.mlme_cm_disconnect_complete_cb = osif_cm_disconnect_complete_cb,
	.mlme_cm_disconnect_start_cb = osif_cm_disconnect_start_cb,
};

/**
 * osif_cm_get_global_ops() - Get connection manager global ops
 *
 * Return: Connection manager global ops
 */
static struct mlme_cm_ops *osif_cm_get_global_ops(void)
{
	return &cm_ops;
}

QDF_STATUS osif_cm_register_cb(void)
{
	mlme_set_osif_cm_cb(osif_cm_get_global_ops);

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS osif_cm_osif_priv_init(struct wlan_objmgr_vdev *vdev)
{
	struct vdev_osif_priv *osif_priv = wlan_vdev_get_ospriv(vdev);
	enum QDF_OPMODE mode = wlan_vdev_mlme_get_opmode(vdev);

	if (mode != QDF_STA_MODE && mode != QDF_P2P_CLIENT_MODE)
		return QDF_STATUS_SUCCESS;

	if (!osif_priv) {
		osif_err("Invalid vdev osif priv");
		return QDF_STATUS_E_INVAL;
	}

	qdf_spinlock_create(&osif_priv->cm_info.cmd_id_lock);

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS osif_cm_osif_priv_deinit(struct wlan_objmgr_vdev *vdev)
{
	struct vdev_osif_priv *osif_priv = wlan_vdev_get_ospriv(vdev);
	enum QDF_OPMODE mode = wlan_vdev_mlme_get_opmode(vdev);

	if (mode != QDF_STA_MODE && mode != QDF_P2P_CLIENT_MODE)
		return QDF_STATUS_SUCCESS;

	if (!osif_priv) {
		osif_err("Invalid vdev osif priv");
		return QDF_STATUS_E_INVAL;
	}

	qdf_spinlock_destroy(&osif_priv->cm_info.cmd_id_lock);

	return QDF_STATUS_SUCCESS;
}
