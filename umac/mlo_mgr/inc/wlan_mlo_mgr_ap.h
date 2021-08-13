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
 * DOC: contains MLO manager public file containing AP functionality
 */
#ifndef _WLAN_MLO_MGR_AP_H_
#define _WLAN_MLO_MGR_AP_H_

#include <wlan_mlo_mgr_cmn.h>
#include <wlan_mlo_mgr_public_structs.h>

/**
 * mlo_ap_vdev_attach() - update vdev obj and vdev count to
 *                         wlan_mlo_dev_context
 * @vdev: vdev pointer
 * @link_id: link id
 * @vdev_count: number of vdev in the mlo
 *
 * Return: true if succeeds
 */
bool mlo_ap_vdev_attach(struct wlan_objmgr_vdev *vdev,
			uint8_t link_id,
			uint16_t vdev_count);

/**
 * mlo_ap_get_vdev_list() - get mlo vdev list
 * @vdev: vdev pointer
 * @vdev_count: vdev count
 * @wlan_vdev_list: vdev list
 *
 * Return: None
 */
void mlo_ap_get_vdev_list(struct wlan_objmgr_vdev *vdev,
			  uint16_t *vdev_count,
			  struct wlan_objmgr_vdev **wlan_vdev_list);

/**
 * mlo_ap_link_sync_wait_notify() - notify the mlo manager, once vdev
 *                                  enters WLAN_VDEV_SS_MLO_SYNC_WAIT
 * @vdev: vdev pointer
 *
 * Return: None
 */
void mlo_ap_link_sync_wait_notify(struct wlan_objmgr_vdev *vdev);

/**
 * mlo_ap_link_start_rsp_notify - Notify that the link start is completed
 *
 * @vdev: pointer to vdev
 *
 * Return: none
 */
void mlo_ap_link_start_rsp_notify(struct wlan_objmgr_vdev *vdev);

/**
 * mlo_ap_vdev_detach() - notify the mlo manager to detach given vdev
 * @vdev: vdev pointer
 *
 * Return: None
 */
void mlo_ap_vdev_detach(struct wlan_objmgr_vdev *vdev);

/**
 * mlo_ap_link_down_cmpl_notify() - notify the mlo manager, once vdev
 *                                  is down completely
 * @vdev: vdev pointer
 *
 * Return: None
 */
void mlo_ap_link_down_cmpl_notify(struct wlan_objmgr_vdev *vdev);
#endif
