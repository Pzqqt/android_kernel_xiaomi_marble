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

/**
 * mlo_ap_link_start_rsp_notify - Notifies that the link is completed
 *
 * @vdev: pointer to vdev
 *
 * Return: none
 */
void mlo_ap_link_start_rsp_notify(struct wlan_objmgr_vdev *vdev);

/**
 * mlo_is_ap_vdev_up_allowed - Checks if the AP can be started
 *
 * Return: true if vdev is allowed to come up, false otherwise
 */
bool mlo_is_ap_vdev_up_allowed(struct wlan_objmgr_vdev *vdev);

/**
 * mlo_ap_link_down_notify - Currently does nothing
 *
 * @vdev: pointer to vdev
 *
 * Return: none
 */
void mlo_ap_link_down_notify(struct wlan_objmgr_vdev *vdev);

/**
 * mlo_is_mld_ap - Check if MLD associated with the vdev is an AP
 *
 * @vdev: pointer to vdev
 *
 * Return: true if MLD is an AP, false otherwise
 */
bool mlo_is_mld_ap(struct wlan_objmgr_vdev *vdev);

#endif
