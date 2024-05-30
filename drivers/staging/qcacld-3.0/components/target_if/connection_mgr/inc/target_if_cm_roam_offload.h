/*
 * Copyright (c) 2020, The Linux Foundation. All rights reserved.
 * Copyright (c) 2022 Qualcomm Innovation Center, Inc. All rights reserved.
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
 * DOC: This file contains definitions for target_if roaming offload.
 */

#ifndef TARGET_IF_CM_ROAM_OFFLOAD_H__
#define TARGET_IF_CM_ROAM_OFFLOAD_H__

#include "wlan_cm_roam_public_struct.h"

/**
 * target_if_cm_roam_register_tx_ops  - Target IF API to register roam
 * related tx op.
 * @tx_ops: Pointer to tx ops fp struct
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
target_if_cm_roam_register_tx_ops(struct wlan_cm_roam_tx_ops *tx_ops);

#ifdef WLAN_FEATURE_11BE_MLO
/**
 * target_if_stop_rso_stop_timer() - Stop the RSO_STOP timer
 * @roam_event: Roam event data
 *
 * This stops the RSO stop timer in below cases,
 * 1. If the reason is RSO_STATUS and notif is CM_ROAM_NOTIF_SCAN_MODE_SUCCESS
 * 2. If wait started already and received HO_FAIL event
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
target_if_stop_rso_stop_timer(struct roam_offload_roam_event *roam_event);
#else
static inline QDF_STATUS
target_if_stop_rso_stop_timer(struct roam_offload_roam_event *roam_event)
{
	roam_event->rso_timer_stopped = false;
	return QDF_STATUS_E_NOSUPPORT;
}
#endif

#ifdef WLAN_FEATURE_ROAM_OFFLOAD
/**
 * target_if_cm_send_rso_stop_failure_rsp() - Send RSO_STOP failure rsp to CM
 * @psoc: psoc object
 * @vdev_id: vdev_id on which RSO stop is issued
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
target_if_cm_send_rso_stop_failure_rsp(struct wlan_objmgr_psoc *psoc,
				       uint8_t vdev_id);
#else
static inline QDF_STATUS
target_if_cm_send_rso_stop_failure_rsp(struct wlan_objmgr_psoc *psoc,
				       uint8_t vdev_id)
{
	return QDF_STATUS_E_NOSUPPORT;
}
#endif
#endif
