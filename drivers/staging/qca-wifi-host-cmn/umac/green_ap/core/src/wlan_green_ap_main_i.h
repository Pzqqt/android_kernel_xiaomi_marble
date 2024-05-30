/*
 * Copyright (c) 2017-2019 The Linux Foundation. All rights reserved.
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
 * DOC: This file has main green ap structures.
 */

#ifndef _WLAN_GREEN_AP_MAIN_I_H_
#define _WLAN_GREEN_AP_MAIN_I_H_

#include <wlan_objmgr_cmn.h>
#include <wlan_objmgr_pdev_obj.h>
#include <wlan_green_ap_api.h>
#include <qdf_types.h>
#include <qdf_status.h>
#include <qdf_timer.h>
#include "wlan_utility.h"
#include <qdf_module.h>

#define WLAN_GREEN_AP_PS_ON_TIME        (0)
#define WLAN_GREEN_AP_PS_TRANS_TIME     (20)

#define green_ap_alert(params...) \
	QDF_TRACE_FATAL(QDF_MODULE_ID_GREEN_AP, params)
#define green_ap_err(params...) \
	QDF_TRACE_ERROR(QDF_MODULE_ID_GREEN_AP, params)
#define green_ap_warn(params...) \
	QDF_TRACE_WARN(QDF_MODULE_ID_GREEN_AP, params)
#define green_ap_notice(params...) \
	QDF_TRACE_INFO(QDF_MODULE_ID_GREEN_AP, params)
#define green_ap_info(params...) \
	QDF_TRACE_INFO(QDF_MODULE_ID_GREEN_AP, params)
#define green_ap_debug(params...) \
	QDF_TRACE_DEBUG(QDF_MODULE_ID_GREEN_AP, params)

#define greenap_nofl_alert(params...) \
	QDF_TRACE_FATAL_NO_FL(QDF_MODULE_ID_GREEN_AP, params)
#define greenap_nofl_err(params...) \
	QDF_TRACE_ERROR_NO_FL(QDF_MODULE_ID_GREEN_AP, params)
#define greenap_nofl_warn(params...) \
	QDF_TRACE_WARN_NO_FL(QDF_MODULE_ID_GREEN_AP, params)
#define greenap_nofl_info(params...) \
	QDF_TRACE_INFO_NO_FL(QDF_MODULE_ID_GREEN_AP, params)
#define greenap_nofl_debug(params...) \
	QDF_TRACE_DEBUG_NO_FL(QDF_MODULE_ID_GREEN_AP, params)

#define WLAN_GREEN_AP_PS_DISABLE 0
#define WLAN_GREEN_AP_PS_ENABLE 1
#define WLAN_GREEN_AP_PS_SUSPEND 2
/**
 * enum wlan_green_ap_ps_state - PS states
 * @WLAN_GREEN_AP_PS_IDLE_STATE - Idle
 * @WLAN_GREEN_AP_PS_OFF_STATE  - Off
 * @WLAN_GREEN_AP_PS_WAIT_STATE - Wait
 * @WLAN_GREEN_AP_PS_ON_STATE   - On
 */
enum wlan_green_ap_ps_state {
	WLAN_GREEN_AP_PS_IDLE_STATE = 1,
	WLAN_GREEN_AP_PS_OFF_STATE,
	WLAN_GREEN_AP_PS_WAIT_STATE,
	WLAN_GREEN_AP_PS_ON_STATE,
};

/**
 * enum wlan_green_ap_ps_event   - PS event
 * @WLAN_GREEN_AP_PS_START_EVENT - Start
 * @WLAN_GREEN_AP_PS_STOP_EVENT  - Stop
 * @WLAN_GREEN_AP_ADD_STA_EVENT  - Sta assoc
 * @WLAN_GREEN_AP_DEL_STA_EVENT  - Sta disassoc
 * @WLAN_GREEN_AP_ADD_MULTISTREAM_STA_EVENT - Multistream sta assoc
 * @WLAN_GREEN_AP_DEL_MULTISTREAM_STA_EVENT - Multistream sta disassoc
 * @WLAN_GREEN_AP_PS_ON_EVENT    - PS on
 * @WLAN_GREEN_AP_PS_OFF_EVENT   - PS off
 */
enum wlan_green_ap_ps_event {
	WLAN_GREEN_AP_PS_START_EVENT = 1,
	WLAN_GREEN_AP_PS_STOP_EVENT,
	WLAN_GREEN_AP_ADD_STA_EVENT,
	WLAN_GREEN_AP_DEL_STA_EVENT,
	WLAN_GREEN_AP_ADD_MULTISTREAM_STA_EVENT,
	WLAN_GREEN_AP_DEL_MULTISTREAM_STA_EVENT,
	WLAN_GREEN_AP_PS_ON_EVENT,
	WLAN_GREEN_AP_PS_WAIT_EVENT,
};

/**
 * struct wlan_pdev_green_ap_ctx - green ap context
 * @pdev - Pdev pointer
 * @ps_enable  - Enable PS
 * @ps_mode - No sta or Multistream sta mode
 * @ps_on_time - PS on time, once enabled
 * @ps_trans_time - PS transition time
 * @num_nodes - Number of nodes associated to radio
 * @num_nodes_multistream - Multistream nodes associated to radio
 * @ps_state - PS state
 * @ps_event - PS event
 * @ps_timer - Timer
 * @lock: green ap spinlock
 * @egap_params - Enhanced green ap params
 */
struct wlan_pdev_green_ap_ctx {
	struct wlan_objmgr_pdev *pdev;
	uint8_t ps_enable;
	uint8_t ps_mode;
	uint8_t ps_on_time;
	uint32_t ps_trans_time;
	uint32_t num_nodes;
	uint32_t num_nodes_multistream;
	enum wlan_green_ap_ps_state ps_state;
	enum wlan_green_ap_ps_event ps_event;
	qdf_timer_t ps_timer;
	qdf_spinlock_t lock;
	struct wlan_green_ap_egap_params egap_params;
	bool dbg_enable;
};

/**
 * wlan_psoc_get_green_ap_tx_ops() - Obtain green ap tx ops from green ap ctx
 * @green_ap_ctx: green ap context
 *
 * @Return: green ap tx ops pointer
 */
struct wlan_lmac_if_green_ap_tx_ops *
wlan_psoc_get_green_ap_tx_ops(struct wlan_pdev_green_ap_ctx *green_ap_ctx);

/**
 * wlan_is_egap_enabled() - Get Enhance Green AP feature status
 * @green_ap_ctx: green ap context
 *
 * Return: true if firmware, feature_flag and ini are all egap enabled
 */
bool wlan_is_egap_enabled(struct wlan_pdev_green_ap_ctx *green_ap_ctx);

/**
 * wlan_green_ap_state_mc() - Green ap state machine
 * @green_ap_ctx: green ap context
 * @event: ps event
 *
 * @Return: Success or Failure
 */
QDF_STATUS wlan_green_ap_state_mc(struct wlan_pdev_green_ap_ctx *green_ap_ctx,
				  enum wlan_green_ap_ps_event event);

/**
 * wlan_green_ap_timer_fn() - Green ap timer callback
 * @pdev: pdev pointer
 *
 * @Return: None
 */
void wlan_green_ap_timer_fn(void *pdev);

/**
 * wlan_green_ap_check_mode() - Check for mode
 * @pdev: pdev pointer
 * @object:  vdev object
 * @arg: flag to be set
 *
 * Callback to check if all modes on radio are configured as AP
 *
 * @Return: None
 */
void wlan_green_ap_check_mode(struct wlan_objmgr_pdev *pdev,
		void *object,
		void *arg);
#endif  /* _WLAN_GREEN_AP_MAIN_I_H_ */
