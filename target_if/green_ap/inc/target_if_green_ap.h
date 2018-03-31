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
 * DOC: offload lmac interface APIs for green ap
 */
#ifndef __TARGET_IF_GREEN_AP_H__
#define __TARGET_IF_GREEN_AP_H__

#include <wlan_objmgr_cmn.h>
#include <wlan_objmgr_pdev_obj.h>
#include <qdf_status.h>
#include <wlan_lmac_if_def.h>

struct wlan_green_ap_egap_params;

/**
 * target_if_register_green_ap_tx_ops() - lmac handler to register
 *   green ap tx_ops callback functions
 * @tx_ops: wlan_lmac_if_tx_ops object
 *
 * Return: QDF_STATUS in case of success
 */
QDF_STATUS target_if_register_green_ap_tx_ops(
		struct wlan_lmac_if_tx_ops *tx_ops);

/**
 * target_if_green_ap_register_egap_event_handler() - registers enhanced
 *                                  green ap event handler
 * @pdev: objmgr pdev
 *
 * Return: QDF_STATUS in case of success
 */
QDF_STATUS target_if_green_ap_register_egap_event_handler(
			struct wlan_objmgr_pdev *pdev);

/**
 * target_if_green_ap_enable_egap() - enable enhanced green ap
 * @pdev: pdev pointer
 * @egap_params: enhanced green ap params
 *
 * @Return: QDF_STATUS_SUCCESS in case of success
 */
QDF_STATUS target_if_green_ap_enable_egap(
		struct wlan_objmgr_pdev *pdev,
		struct wlan_green_ap_egap_params *egap_params);

/**
 * target_if_green_ap_set_ps_on_off() - Green AP PS toggle
 * @pdev: pdev pointer
 * @value: Value to send PS on/off to FW
 * @pdev_id: pdev id
 *
 * @Return: QDF_STATUS_SUCCESS in case of success
 */
QDF_STATUS target_if_green_ap_set_ps_on_off(struct wlan_objmgr_pdev *pdev,
					    bool value, uint8_t pdev_id);

/**
 * target_if_green_ap_get_current_channel() - Get current channel
 * @pdev: pdev pointer
 *
 * @Return: current channel freq
 */
uint16_t target_if_green_ap_get_current_channel(struct wlan_objmgr_pdev *pdev);

/**
 * target_if_green_ap_get_current_channel_flags() - Get current channel flags
 * @pdev: pdev pointer
 *
 * @Return: current channel flags
 */
uint64_t target_if_green_ap_get_current_channel_flags(
				struct wlan_objmgr_pdev *pdev);

/**
 * target_if_green_ap_reset_dev() - Reset dev
 * @pdev: pdev pointer
 *
 * @Return:  QDF_STATUS_SUCCESS if device resetted
 */
QDF_STATUS target_if_green_ap_reset_dev(struct wlan_objmgr_pdev *pdev);

#endif
