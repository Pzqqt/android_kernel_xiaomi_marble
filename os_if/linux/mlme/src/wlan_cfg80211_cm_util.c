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

/**
 * osif_cm_connect_complete_cb() - Connect complete callback
 * @vdev: vdev pointer
 * @cm_conn_rsp: connect response
 *
 * Return: None
 */
static void
osif_cm_connect_complete_cb(struct wlan_objmgr_vdev *vdev,
			    struct wlan_cm_connect_rsp *cm_conn_rsp)
{
}

/**
 * osif_cm_failed_candidate_cb() - Callback to indicate failed candidate
 * @vdev: vdev pointer
 * @cm_conn_rsp: connect response
 *
 * Return: None
 */
static void
osif_cm_failed_candidate_cb(struct wlan_objmgr_vdev *vdev,
			    struct wlan_cm_connect_rsp *cm_conn_rsp)
{
}

/**
 * osif_cm_update_conn_id_and_src_cb() - Callback to update connect id and
 * source of the connect request
 * @vdev: vdev pointer
 * @Source: Source of the connect req
 * @conn_id: Connect id
 *
 * Return: None
 */
static void
osif_cm_update_conn_id_and_src_cb(struct wlan_objmgr_vdev *vdev,
				  enum wlan_cm_source source, uint64_t conn_id)
{
}

/**
 * osif_cm_disconnect_complete_cb() - Disconnect done callback
 * @vdev: vdev pointer
 * @cm_disconnect_rsp: Disconnect response
 *
 * Return: None
 */

static void
osif_cm_disconnect_complete_cb(struct wlan_objmgr_vdev *vdev,
			       struct wlan_cm_discon_rsp *cm_disconnect_rsp)
{
}

/**
 * osif_cm_disconnect_start_cb() - Disconnect start callback
 * @vdev: vdev pointer
 *
 * This callback indicates os_if that disconnection is started
 * so that os_if can stop all the activity on this connection
 *
 * Return: None
 */
static void
osif_cm_disconnect_start_cb(struct wlan_objmgr_vdev *vdev)
{
}

static struct mlme_cm_ops cm_ops = {
	.mlme_cm_connect_complete_cb = osif_cm_connect_complete_cb,
	.mlme_cm_failed_candidate_cb = osif_cm_failed_candidate_cb,
	.mlme_cm_update_conn_id_and_src_cb = osif_cm_update_conn_id_and_src_cb,
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

