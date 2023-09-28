/*
 * Copyright (c) 2017-2018 The Linux Foundation. All rights reserved.
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

#ifndef _WLAN_GREEN_AP_UCFG_API_H_
#define _WLAN_GREEN_AP_UCFG_API_H_

#include <wlan_objmgr_cmn.h>
#include <wlan_objmgr_pdev_obj.h>
#include <qdf_status.h>
#include "wlan_utility.h"

/**
 * ucfg_green_ap_enable_egap() - Enable enhanced green ap
 * @pdev: pdev pointer
 *
 * Return: Success or Failure
 */
QDF_STATUS ucfg_green_ap_enable_egap(struct wlan_objmgr_pdev *pdev);

/**
 * ucfg_green_ap_set_ps_config() - Set ps value
 * @pdev: pdev pointer
 * @value - value to be set
 *
 * Return: Success or Failure
 */
QDF_STATUS ucfg_green_ap_set_ps_config(struct wlan_objmgr_pdev *pdev,
				       uint8_t value);
/**
 * ucfg_green_ap_get_ps_config() - Check if ps is enabled or not
 * @pdev: pdev pointer
 * @ps_enable: pointer to ps enable config value
 *
 * Return: Success or Failure
 */
QDF_STATUS ucfg_green_ap_get_ps_config(struct wlan_objmgr_pdev *pdev,
				       uint8_t *ps_enable);

/**
 * ucfg_green_ap_set_transition_time() - Set transition time
 * @pdev: pdev pointer
 * @val: transition time
 *
 * This API sets custom transition time
 *
 * Return: Success or Failure
 */
QDF_STATUS ucfg_green_ap_set_transition_time(struct wlan_objmgr_pdev *pdev,
					     uint32_t val);

/**
 * ucfg_green_ap_get_transition_time() - Get transition time
 * @pdev: pdev pointer
 * @ps_trans_time: pointer to transition time
 *
 * This API gets transition time
 *
 * Return: Success or Failure
 */
QDF_STATUS ucfg_green_ap_get_transition_time(struct wlan_objmgr_pdev *pdev,
					     uint32_t *ps_trans_time);

/**
 * ucfg_green_ap_config() - Config green AP
 * @pdev: pdev pointer
 *
 * Return: Success or Failure
 */
QDF_STATUS ucfg_green_ap_config(struct wlan_objmgr_pdev *pdev, uint8_t val);

/**
 * ucfg_green_ap_enable_debug_prints() - Enable debugs
 * @pdev: pdev pointer
 *
 * Return: None
 */
void ucfg_green_ap_enable_debug_prints(struct wlan_objmgr_pdev *pdev,
					uint32_t val);

/**
 * ucfg_green_ap_get_debug_prints() - Check if debug enabled
 * @pdev: pdev pointer
 *
 * Return: Debug value
 */
bool ucfg_green_ap_get_debug_prints(struct wlan_objmgr_pdev *pdev);
#endif /* _WLAN_GREEN_AP_UCFG_API_H_ */
