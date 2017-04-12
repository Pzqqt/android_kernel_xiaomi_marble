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
#include "target_if_scan.h"

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
