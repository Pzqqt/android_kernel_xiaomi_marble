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
 * DOC: wmi_unified_event_handler.h
 *
 * Public API file for common WMI event handlers
 */
#ifndef _WMI_UNIFIED_EVENT_HANDLER_H_
#define _WMI_UNIFIED_EVENT_HANDLER_H_

#include "athdefs.h"
#include "osapi_linux.h"
#include "a_types.h"
#include "a_debug.h"
#include "ol_if_athvar.h"
#include "ol_defines.h"
#include "qdf_types.h"
#include "qdf_util.h"
#include "wmi_unified_priv.h"
#include "wmi_unified_param.h"
#include "wlan_objmgr_psoc_obj.h"
#include "target_if.h"

#define MAX_HW_MODE      (2)
#define MAX_MAC_PHY_CAP  (5)
#define MAX_PHY_REG_CAP  (3)

/**
 * struct service_ready_param - service ready structure
 * @wmi_service_bitmap: wmi service bitmap
 * @target_caps: traget capability
 * @hal_reg_cap: hal reg capability
 */
struct service_ready_param {
	uint32_t wmi_service_bitmap[wmi_services_max];
	target_capability_info target_caps;
	TARGET_HAL_REG_CAPABILITIES hal_reg_cap;
};

/**
 * struct ext_service_ready_param - ext service ready structure
 * @service_ext_param: service ext param
 * @hw_mode_caps: hw mode caps
 * @mac_phy_cap: mac phy cap
 * @reg_cap: regulatory capability
 */
struct ext_service_ready_param {
	struct wmi_host_service_ext_param service_ext_param;
	struct wmi_host_hw_mode_caps hw_mode_caps[MAX_HW_MODE];
	struct wmi_host_mac_phy_caps mac_phy_cap[MAX_MAC_PHY_CAP];
	struct WMI_HOST_HAL_REG_CAPABILITIES_EXT reg_cap[MAX_PHY_REG_CAP];
};

/**
 * init_deinit_service_ready_event_handler() - service ready handler
 * @handle: opaqueue pointer to scn
 * @event: pointer to event buffer
 * @event_len: event length
 *
 * WMI common event handler for WMI_SERVICE_READY_EVENTID
 *
 * Return: 0 for success, negative error code for failure
 */
int init_deinit_service_ready_event_handler(ol_scn_t handle, uint8_t *event,
					    uint32_t event_len);

/**
 * init_deinit_service_ext_ready_event_handler() - ext service ready handler
 * @handle: opaqueue pointer to scn
 * @event: pointer to event buffer
 * @event_len: event length
 *
 * WMI common event handler for WMI_SERVICE_READY_EXT_EVENTID
 *
 * Return: 0 for success, negative error code for failure
 */
int init_deinit_service_ext_ready_event_handler(ol_scn_t handle, uint8_t *event,
						uint32_t event_len);

#endif /* _WMI_UNIFIED_EVENT_HANDLER_H_ */
