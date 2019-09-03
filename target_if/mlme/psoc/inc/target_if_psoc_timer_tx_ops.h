
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

/**
 * DOC: target_if_psoc_timer_tx_ops.h
 *
 * This file provide declaration for APIs registered through lmac Tx Ops
 */

#ifndef __TARGET_IF_PSOC_TIMER_TX_OPS_H__
#define __TARGET_IF_PSOC_TIMER_TX_OPS_H__

/**
 * target_if_psoc_vdev_rsp_timer_inuse() - API to check if the response timer
 * for vdev is inuse
 * @psoc: Psoc object
 * @vdev_id: Vdev object id
 *
 * Return: QDF_STATUS_E_ALREADY in case the timer is inuse or QDF_STATUS_SUCCESS
 */
QDF_STATUS target_if_psoc_vdev_rsp_timer_inuse(struct wlan_objmgr_psoc *psoc,
					       uint8_t vdev_id);

/**
 * target_if_flush_psoc_vdev_timers() - API to flush target_if response timers
 * for vdev
 * @psoc: pointer to psoc object
 *
 * This API is used to flush target_if response timer. This API used while
 * wlan driver shutdown.
 *
 * Return: none
 */
void target_if_flush_psoc_vdev_timers(struct wlan_objmgr_psoc *psoc);

/**
 * target_if_psoc_vdev_rsp_timer_init() - API to initialize response timers for
 * vdev from psoc
 * @psoc: pointer to psoc object
 * @vdev_id: vdev id for which response timer to be retrieved
 *
 * This API is used to initialize vdev response timer for vdev-id.
 *
 * Return: QDF_STATUS
 */
QDF_STATUS target_if_psoc_vdev_rsp_timer_init(struct wlan_objmgr_psoc *psoc,
					      uint8_t vdev_id);

/**
 * target_if_psoc_vdev_rsp_timer_deinit() - API to de-initialize response timers
 * for vdev from psoc
 * @psoc: pointer to psoc object
 * @vdev_id: vdev id for which response timer to be retrieved
 *
 * This API is used to de-initialize vdev response timer from vdev-id.
 *
 * Return: none
 */
void target_if_psoc_vdev_rsp_timer_deinit(struct wlan_objmgr_psoc *psoc,
					  uint8_t vdev_id);

/**
 * target_if_vdev_mgr_rsp_timer_mod() - API to modify time of response timers
 * for vdev from psoc
 * @psoc: pointer to psoc object
 * @vdev_id: vdev id for which response timer to be retrieved
 * @mseconds:  milli seconds
 *
 * This API is used to modify vdev response timer for vdev-id.
 *
 * Return: none
 */
QDF_STATUS target_if_vdev_mgr_rsp_timer_mod(
					struct wlan_objmgr_psoc *psoc,
					uint8_t vdev_id,
					int mseconds);

#endif
