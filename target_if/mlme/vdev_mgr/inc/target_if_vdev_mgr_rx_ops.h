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
 * DOC: target_if_vdev_mgr_rx_ops.h
 *
 * This file provides declarations for APIs registered for wmi events
 */

#ifndef __TARGET_IF_VDEV_MGR_RX_OPS_H__
#define __TARGET_IF_VDEV_MGR_RX_OPS_H__

#include <wmi_unified_param.h>
#include <wlan_objmgr_psoc_obj.h>
#include <wlan_lmac_if_def.h>

/**
 * target_if_vdev_mgr_is_driver_unloading: API to driver unload status
 *
 * Return: TRUE or FALSE
 */
static inline bool target_if_vdev_mgr_is_driver_unloading(void)
{
	return false;
}

/**
 * target_if_vdev_mgr_is_panic_on_bug: API to get panic on bug
 *
 * Return: TRUE or FALSE
 */
static inline bool target_if_vdev_mgr_is_panic_on_bug(void)
{
#ifdef PANIC_ON_BUG
	return true;
#else
	return false;
#endif
}

/**
 * target_if_vdev_mgr_get_rx_ops() - get rx ops
 * @psoc: pointer to psoc object
 *
 * Return: pointer to rx ops
 */
static inline struct wlan_lmac_if_mlme_rx_ops *
target_if_vdev_mgr_get_rx_ops(struct wlan_objmgr_psoc *psoc)
{
	return &psoc->soc_cb.rx_ops.mops;
}

/**
 * target_if_vdev_mgr_rsp_timer_mgmt_cb() - function to handle response timer
 * @arg: pointer to argument
 *
 * Callback timer triggered when response timer expires which pass
 * vdev as argument
 *
 * Return: status of operation.
 */
void target_if_vdev_mgr_rsp_timer_mgmt_cb(void *arg);

/**
 * target_if_vdev_mgr_wmi_event_register() - function to handle register
 * events from WMI
 * @psoc: pointer to psoc object
 *
 * Return: QDF_STATUS_SUCCESS on success, QDF_STATUS_E_** on error
 */
QDF_STATUS target_if_vdev_mgr_wmi_event_register(
					struct wlan_objmgr_psoc *psoc);

/**
 * target_if_vdev_mgr_wmi_event_unregister() - function to handle unregister
 * events from WMI
 * @psoc: pointer to psoc object
 *
 * Return: QDF_STATUS_SUCCESS on success, QDF_STATUS_E_** on error
 */
QDF_STATUS target_if_vdev_mgr_wmi_event_unregister(
					struct wlan_objmgr_psoc *psoc);

#endif /* __TARGET_IF_VDEV_MGR_RX_OPS_H__ */
