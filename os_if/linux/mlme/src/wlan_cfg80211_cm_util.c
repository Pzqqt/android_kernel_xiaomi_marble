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
 * DOC: wlan_cfg80211_cm_util.c
 *
 * This file maintains definitaions of connect, disconnect, roam
 * common apis.
 */
#include <include/wlan_mlme_cmn.h>
#include "wlan_cfg80211_cm_util.h"
#include "wlan_osif_priv.h"
#include "wlan_cfg80211.h"
#include "wlan_cfg80211_cm_rsp.h"

void osif_cm_reset_id_and_src_no_lock(struct vdev_osif_priv *osif_priv)
{
	osif_priv->last_cmd_info.last_id = CM_ID_INVALID;
	osif_priv->last_cmd_info.last_source = CM_SOURCE_INVALID;
}

QDF_STATUS osif_cm_reset_id_and_src(struct wlan_objmgr_vdev *vdev)
{
	struct vdev_osif_priv *osif_priv = wlan_vdev_get_ospriv(vdev);

	if (!osif_priv) {
		osif_err("Invalid vdev osif priv");
		return QDF_STATUS_E_INVAL;
	}
	qdf_spinlock_acquire(&osif_priv->last_cmd_info.cmd_id_lock);
	osif_cm_reset_id_and_src_no_lock(osif_priv);
	qdf_spinlock_release(&osif_priv->last_cmd_info.cmd_id_lock);

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
 * @cm_conn_rsp: connect response
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS
osif_cm_failed_candidate_cb(struct wlan_objmgr_vdev *vdev,
			    struct wlan_cm_connect_rsp *cm_conn_rsp)
{
	return QDF_STATUS_SUCCESS;
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

	qdf_spinlock_acquire(&osif_priv->last_cmd_info.cmd_id_lock);
	osif_priv->last_cmd_info.last_id = cm_id;
	osif_priv->last_cmd_info.last_source = source;
	qdf_spinlock_release(&osif_priv->last_cmd_info.cmd_id_lock);

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

