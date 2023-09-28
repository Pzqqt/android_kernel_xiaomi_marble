/*
 * Copyright (c) 2020-2021, The Linux Foundation. All rights reserved.
 * Copyright (c) 2021-2022 Qualcomm Innovation Center, Inc. All rights reserved.
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

/**
 * DOC: This file contains definitions for target_if
 * roaming events.
 */

#ifndef TARGET_IF_CM_ROAM_EVENT_H__
#define TARGET_IF_CM_ROAM_EVENT_H__

#include "qdf_types.h"
#include "wlan_objmgr_psoc_obj.h"
#include "wlan_objmgr_pdev_obj.h"
#include "wlan_objmgr_vdev_obj.h"
#include "wlan_cm_roam_public_struct.h"
#include <target_if.h>

#ifdef WLAN_FEATURE_ROAM_OFFLOAD
/**
 * target_if_cm_get_roam_rx_ops() - Get CM roam rx ops registered
 * @psoc: pointer to psoc object
 *
 * Return: roam rx ops of connection mgr
 */
struct wlan_cm_roam_rx_ops *
target_if_cm_get_roam_rx_ops(struct wlan_objmgr_psoc *psoc);

/**
 * target_if_cm_roam_sync_event() - Target IF handler for roam sync events
 * @scn: target handle
 * @event: event buffer
 * @len: event buffer length
 *
 * Return: int for success or error code
 */
int target_if_cm_roam_sync_event(ol_scn_t scn, uint8_t *event,
				 uint32_t len);

/**
 * target_if_cm_roam_sync_frame_event() - Target IF handler for
 * roam sync frame events
 * @scn: target handle
 * @event: event buffer
 * @len: event buffer length
 *
 * Return: int for success or error code
 */
int
target_if_cm_roam_sync_frame_event(ol_scn_t scn,
				   uint8_t *event,
				   uint32_t len);

/**
 * target_if_cm_roam_event() - Target IF handler for roam events
 * @scn: target handle
 * @event: event buffer
 * @len: event buffer length
 *
 * Return: int for success or error code
 */
int target_if_cm_roam_event(ol_scn_t scn, uint8_t *event, uint32_t len);

/**
 * target_if_cm_roam_stats_event() - Target IF handler for roam stats event
 * @scn: target handle
 * @event: event buffer
 * @len: event buffer length
 *
 * Return: int for success or error code
 */
int
target_if_cm_roam_stats_event(ol_scn_t scn, uint8_t *event, uint32_t len);

/**
 * target_if_cm_roam_auth_offload_event - auth roam offload event handler
 * @scn: target handle
 * @event: event buffer
 * @len: event buffer length
 *
 * Return: int for success or error code
 */
int
target_if_cm_roam_auth_offload_event(ol_scn_t scn, uint8_t *event,
				     uint32_t len);

/**
 * target_if_roam_offload_register_events() - register roam events
 * @psoc: pointer to psoc object
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
target_if_roam_offload_register_events(struct wlan_objmgr_psoc *psoc);

/**
 * target_if_cm_roam_vdev_disconnect_event_handler - vdev disconnect evt handler
 * @scn: target handle
 * @event: event buffer
 * @len: event buffer length
 *
 * Return: int for success or error code
 */
int
target_if_cm_roam_vdev_disconnect_event_handler(ol_scn_t scn, uint8_t *event,
						uint32_t len);

/**
 * target_if_cm_roam_scan_chan_list_event_handler - roam scan ch evt handler
 * @scn: target handle
 * @event: event buffer
 * @len: event buffer length
 *
 * Return: int for success or error code
 */
int
target_if_cm_roam_scan_chan_list_event_handler(ol_scn_t scn, uint8_t *event,
					       uint32_t len);

/**
 * target_if_pmkid_request_event_handler - pmkid request event handler
 * @scn: target handle
 * @event: event buffer
 * @len: event buffer length
 *
 * Return: int for success or error code
 */
int
target_if_pmkid_request_event_handler(ol_scn_t scn, uint8_t *event,
				      uint32_t len);

/**
 * target_if_cm_roam_register_rx_ops  - Target IF API to register roam
 * related rx op.
 * @rx_ops: Pointer to rx ops fp struct
 *
 * Return: none
 */
void
target_if_cm_roam_register_rx_ops(struct wlan_cm_roam_rx_ops *rx_ops);

/**
 * target_if_roam_frame_event_handler - Target IF API to receive
 * Beacon/probe for the roaming candidate.
 * @scn: target handle
 * @event: event buffer
 * @len: event buffer length
 *
 * Return: int for success or error code
 */
int
target_if_roam_frame_event_handler(ol_scn_t scn, uint8_t *event,
				   uint32_t len);
#else /* WLAN_FEATURE_ROAM_OFFLOAD */
static inline
void
target_if_cm_roam_register_rx_ops(struct wlan_cm_roam_rx_ops *rx_ops)
{
}

static inline
QDF_STATUS
target_if_roam_offload_register_events(struct wlan_objmgr_psoc *psoc)
{
	return QDF_STATUS_E_NOSUPPORT;
}

static inline int
target_if_cm_roam_event(ol_scn_t scn, uint8_t *event, uint32_t len)
{
	return 0;
}

static inline int
target_if_cm_roam_vdev_disconnect_event_handler(ol_scn_t scn, uint8_t *event,
						uint32_t len)
{
	return 0;
}

static inline int
target_if_cm_roam_scan_chan_list_event_handler(ol_scn_t scn, uint8_t *event,
					       uint32_t len)
{
	return 0;
}

static inline int
target_if_pmkid_request_event_handler(ol_scn_t scn, uint8_t *event,
				      uint32_t len)
{
	return 0;
}

static inline int
target_if_roam_frame_event_handler(ol_scn_t scn, uint8_t *event,
				   uint32_t len)
{
	return 0;
}
#endif /* WLAN_FEATURE_ROAM_OFFLOAD */
#endif
