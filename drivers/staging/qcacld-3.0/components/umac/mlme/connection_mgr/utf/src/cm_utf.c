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
 * DOC: Implements CM UTF
 */

#ifdef FEATURE_CM_UTF_ENABLE
#include <cm_utf.h>
#include <wlan_cm_utf.h>
#include <include/wlan_mlme_cmn.h>

void cm_utf_set_mlme_ops(struct mlme_ext_ops *ext_ops)
{
	ext_ops->mlme_cm_ext_connect_start_ind_cb = NULL;
	ext_ops->mlme_cm_ext_bss_select_ind_cb = NULL;
	ext_ops->mlme_cm_ext_bss_peer_create_req_cb =
					wlan_cm_utf_bss_peer_create_req;
	ext_ops->mlme_cm_ext_connect_req_cb = wlan_cm_utf_connect_req;
	ext_ops->mlme_cm_ext_connect_complete_ind_cb = NULL;
	ext_ops->mlme_cm_ext_disconnect_start_ind_cb = NULL;
	ext_ops->mlme_cm_ext_disconnect_req_cb = wlan_cm_utf_disconnect_req;
	ext_ops->mlme_cm_ext_bss_peer_delete_req_cb =
					wlan_cm_utf_bss_peer_delete_req;
	ext_ops->mlme_cm_ext_disconnect_complete_ind_cb = NULL;
	ext_ops->mlme_cm_ext_vdev_down_req_cb = wlan_cm_utf_vdev_down;
}

QDF_STATUS cm_utf_register_os_if_cb(void)
{
	return osif_cm_utf_register_cb();
}

void cm_utf_attach(struct wlan_objmgr_vdev *vdev)
{
	if (QDF_IS_STATUS_ERROR(wlan_cm_utf_attach(vdev)))
		mlme_err("utf attach failed");
	else
		mlme_debug("utf attach success");
}

void cm_utf_detach(struct wlan_objmgr_vdev *vdev)
{
	wlan_cm_utf_detach(vdev);
}
#endif
