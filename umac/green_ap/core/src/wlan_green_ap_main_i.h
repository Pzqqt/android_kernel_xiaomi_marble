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
 * DOC: This file has main green ap structures.
 */

#ifndef _WLAN_GREEN_AP_MAIN_I_H_
#define _WLAN_GREEN_AP_MAIN_I_H_

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
 * @WLAN_GREEN_AP_PS_ON_EVENT    - PS on
 * @WLAN_GREEN_AP_PS_OFF_EVENT   - PS off
 */
enum wlan_green_ap_ps_event {
	WLAN_GREEN_AP_PS_START_EVENT = 1,
	WLAN_GREEN_AP_PS_STOP_EVENT,
	WLAN_GREEN_AP_ADD_STA_EVENT,
	WLAN_GREEN_AP_DEL_STA_EVENT,
	WLAN_GREEN_AP_PS_ON_EVENT,
	WLAN_GREEN_AP_PS_WAIT_EVENT,
};

/**
 * struct wlan_pdev_green_ap_ctx - green ap context
 * @pdev - Pdev pointer
 * @green_ap_ps_enable  - Enable PS
 * @green_ap_ps_on_time - PS on time, once enabled
 * @green_ap_ps_trans_time - PS transition time
 * @green_ap_num_node - Number of nodes associated to radio
 * @ps_state - PS state
 * @ps_event - PS event
 * @green_ap_ps_timer - Timer
 * @green_ap_egap_support - Enhanced green ap support
 */
struct wlan_pdev_green_ap_ctx {
	struct wlan_objmgr_pdev *pdev;
	uint8_t green_ap_ps_enable;
	uint8_t green_ap_ps_on_time;
	uint32_t green_ap_ps_trans_time;
	uint32_t green_ap_num_nodes;
	enum wlan_green_ap_ps_state ps_state;
	enum wlan_green_ap_ps_event ps_event;
	qdf_timer_t green_ap_ps_timer;
	bool green_ap_egap_support;
};

/**
 * wlan_green_ap_state_mc() - Green ap state machine
 * @pdev: pdev pointer
 * @event: ps event
 *
 * @Return: None
 */
void wlan_green_ap_state_mc(struct wlan_objmgr_pdev *pdev,
			   enum wlan_green_ap_ps_event event);

/**
 * wlan_green_ap_ps_event_state_update() - Update PS state and event
 * @pdev: pdev pointer
 * @state: ps state
 * @event: ps event
 *
 * @Return: None
 */
void wlan_green_ap_ps_event_state_update(struct wlan_objmgr_pdev *pdev,
			 enum wlan_green_ap_ps_state state,
			 enum wlan_green_ap_ps_event event);

/**
 * wlan_green_ap_timer_fn() - Green ap timer callback
 * @pdev: pdev pointer
 *
 * @Return: status
 */
int wlan_green_ap_timer_fn(struct wlan_objmgr_pdev *pdev);
#endif  /* _WLAN_GREEN_AP_MAIN_I_H_ */
