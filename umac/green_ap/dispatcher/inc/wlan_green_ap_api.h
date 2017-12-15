/*
 * Copyright (c) 2017 The Linux Foundation. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for
 * any purpose with or without fee is hereby granted, provided that the
 * above copyright notice and this permission notice appear in all
 * copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL
 * WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE
 * AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR
 * PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

/**
 * DOC: Contains green ap north bound interface definitions
 */

#ifndef _WLAN_GREEN_AP_API_H_
#define _WLAN_GREEN_AP_API_H_

#include <wlan_objmgr_pdev_obj.h>

/**
 * wlan_green_ap_init() - initialize green ap component
 *
 * Return: Success or Failure
 */
QDF_STATUS wlan_green_ap_init(void);

/**
 * wlan_green_ap_deinit() - De-initialize green ap component
 *
 * Return: Success or Failure
 */
QDF_STATUS wlan_green_ap_deinit(void);

/**
 * wlan_green_ap_start() - Start green ap
 * @pdev: pdev pointer
 *
 * Call this function when the first vdev comes up
 *
 * Return: Success or Failure
 */
QDF_STATUS wlan_green_ap_start(struct wlan_objmgr_pdev *pdev);

/**
 * wlan_green_ap_stop() - Stop green ap
 * @pdev: pdev pointer
 *
 * Call this function when the last vdev goes down
 *
 * Return: Success or Failure
 */
QDF_STATUS wlan_green_ap_stop(struct wlan_objmgr_pdev *pdev);

/**
 * wlan_green_ap_add_sta() - On association
 * @pdev: pdev pointer
 *
 * Call this function when new node is associated
 *
 * Return: Success or Failure
 */
QDF_STATUS wlan_green_ap_add_sta(struct wlan_objmgr_pdev *pdev);

/**
 * wlan_green_ap_del_sta() - On disassociation
 * @pdev: pdev pointer
 *
 * Call this function when new node is associated
 *
 * Return: Success or Failure
 */
QDF_STATUS wlan_green_ap_del_sta(struct wlan_objmgr_pdev *pdev);

#endif /* _WLAN_GREEN_AP_API_H_ */
