/*
 * Copyright (c) 2021 Qualcomm Innovation Center, Inc. All rights reserved.
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
 * DOC: osif_vdev_mgr_util.c
 *
 * This header file maintains definitaions of osif APIs corresponding to vdev
 * manager.
 */

#include <include/wlan_mlme_cmn.h>
#include "osif_vdev_mgr_util.h"

static struct osif_vdev_mgr_ops *osif_vdev_mgr_legacy_ops;

#ifdef WLAN_FEATURE_DYNAMIC_MAC_ADDR_UPDATE
static QDF_STATUS osif_vdev_mgr_set_mac_addr_response(uint8_t vdev_id,
						      uint8_t resp_status)
{
	if (osif_vdev_mgr_legacy_ops &&
	    osif_vdev_mgr_legacy_ops->osif_vdev_mgr_set_mac_addr_response)
		osif_vdev_mgr_legacy_ops->osif_vdev_mgr_set_mac_addr_response(
						vdev_id, resp_status);

	return QDF_STATUS_SUCCESS;
}
#endif

static struct mlme_vdev_mgr_ops vdev_mgr_ops = {
#ifdef WLAN_FEATURE_DYNAMIC_MAC_ADDR_UPDATE
	.mlme_vdev_mgr_set_mac_addr_response =
					osif_vdev_mgr_set_mac_addr_response
#endif
};

/**
 * osif_vdev_mgr_get_global_ops() - Get vdev manager global ops
 *
 * Return: Connection manager global ops
 */
static struct mlme_vdev_mgr_ops *osif_vdev_mgr_get_global_ops(void)
{
	return &vdev_mgr_ops;
}

QDF_STATUS osif_vdev_mgr_register_cb(void)
{
	mlme_set_osif_vdev_mgr_cb(osif_vdev_mgr_get_global_ops);

	return QDF_STATUS_SUCCESS;
}

void osif_vdev_mgr_set_legacy_cb(struct osif_vdev_mgr_ops *osif_legacy_ops)
{
	osif_vdev_mgr_legacy_ops = osif_legacy_ops;
}

void osif_vdev_mgr_reset_legacy_cb(void)
{
	osif_vdev_mgr_legacy_ops = NULL;
}
