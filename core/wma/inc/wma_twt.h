/*
 * Copyright (c) 2019 The Linux Foundation. All rights reserved.
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

#ifndef __WMA_TWT_H
#define __WMA_TWT_H

#include "wma.h"
#include "wmi_unified_twt_param.h"

#ifdef WLAN_SUPPORT_TWT
/**
 * wma_send_twt_enable_cmd() - Send TWT Enable command to firmware
 * @pdev_id: pdev id
 * @congestion_timeout: Timeout value for the TWT congestion timer
 * @bcast_val: broadcast twt support
 *
 * Return: None
 */
void wma_send_twt_enable_cmd(uint32_t pdev_id,
			     uint32_t congestion_timeout,
			     bool bcast_val);

/**
 * wma_set_twt_peer_caps() - Fill the peer TWT capabilities
 * @params: STA context params which will store the capabilities
 * @cmd: Command in which the capabilities should be populated
 *
 * Return: None
 */
void wma_set_twt_peer_caps(tpAddStaParams params,
			   struct peer_assoc_params *cmd);

/**
 * wma_twt_process_add_dialog() - Process twt add dialog command
 * @params: add dialog configuration param
 *
 * Return: QDF_STATUS
 */
QDF_STATUS wma_twt_process_add_dialog(struct wmi_twt_add_dialog_param *params);

/**
 * wma_twt_process_del_dialog() - Process del dialog command
 * @params: del dialog configuration param
 *
 * Return: QDF_STATUS
 */
QDF_STATUS wma_twt_process_del_dialog(struct wmi_twt_del_dialog_param *params);

#else
static inline void wma_send_twt_enable_cmd(uint32_t pdev_id,
					   uint32_t congestion_timeout,
					   bool bcast_val)
{
	WMA_LOGD(FL("TWT not supported as WLAN_SUPPORT_TWT is disabled"));
}

static inline void wma_set_twt_peer_caps(tpAddStaParams params,
					 struct peer_assoc_params *cmd)
{
}

static inline QDF_STATUS wma_twt_process_add_dialog(
		struct wmi_twt_add_dialog_param *params)
{
	WMA_LOGD(FL("TWT not supported as WLAN_SUPPORT_TWT is disabled"));

	return QDF_STATUS_E_INVAL;
}

static inline QDF_STATUS wma_twt_process_del_dialog(
		struct wmi_twt_del_dialog_param *params)
{
	WMA_LOGD(FL("TWT not supported as WLAN_SUPPORT_TWT is disabled"));

	return QDF_STATUS_E_INVAL;
}
#endif

#endif /* __WMA_HE_H */
