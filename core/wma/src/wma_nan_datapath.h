/*
 * Copyright (c) 2016 The Linux Foundation. All rights reserved.
 *
 * Previously licensed under the ISC license by Qualcomm Atheros, Inc.
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
 * DOC: wma_nan_datapath.h
 *
 * WMA NAN Data path API specification
 */

#ifndef __WMA_NAN_DATAPATH_H
#define __WMA_NAN_DATAPATH_H

#include "wma.h"
#include "sir_api.h"
#include "sme_nan_datapath.h"

#ifdef WLAN_FEATURE_NAN_DATAPATH
#define WMA_IS_VDEV_IN_NDI_MODE(intf, vdev_id) \
				(WMI_VDEV_TYPE_NDI == intf[vdev_id].type)
/**
 * wma_update_hdd_cfg_ndp() - Update target device NAN datapath capability
 * @wma_handle: pointer to WMA context
 * @tgt_cfg: Pointer to target configuration data structure
 *
 * Return: none
 */
static inline void wma_update_hdd_cfg_ndp(tp_wma_handle wma_handle,
					struct wma_tgt_cfg *tgt_cfg)
{
	tgt_cfg->nan_datapath_enabled = wma_handle->nan_datapath_enabled;
}
QDF_STATUS wma_handle_ndp_responder_req(tp_wma_handle wma_handle,
					struct ndp_responder_req *req_params);
void wma_delete_all_nan_remote_peers(tp_wma_handle wma,
					uint32_t vdev_id);

void wma_ndp_register_all_event_handlers(tp_wma_handle wma_handle);
void wma_ndp_unregister_all_event_handlers(tp_wma_handle wma_handle);
void wma_ndp_add_wow_wakeup_event(tp_wma_handle wma_handle,
						uint8_t vdev_id);
void wma_ndp_wow_event_callback(void *handle, void *event,
				uint32_t len, uint32_t event_id);
void wma_add_bss_ndi_mode(tp_wma_handle wma, tpAddBssParams add_bss);
void wma_add_sta_ndi_mode(tp_wma_handle wma, tpAddStaParams add_sta);
QDF_STATUS wma_handle_ndp_initiator_req(tp_wma_handle wma_handle, void *req);
QDF_STATUS wma_handle_ndp_end_req(tp_wma_handle wma_handle, void *req);
void wma_delete_sta_req_ndi_mode(tp_wma_handle wma,
					tpDeleteStaParams del_sta);
uint32_t wma_ndp_get_eventid_from_tlvtag(uint32_t tag);
#else
#define WMA_IS_VDEV_IN_NDI_MODE(intf, vdev_id) (false)
static inline void wma_update_hdd_cfg_ndp(tp_wma_handle wma_handle,
					struct wma_tgt_cfg *tgt_cfg)
{
	return;
}
static inline void wma_add_bss_ndi_mode(tp_wma_handle wma,
					tpAddBssParams add_bss)
{
	return;
}
static inline void wma_delete_all_nan_remote_peers(tp_wma_handle wma,
							uint32_t vdev_id)
{
}
static inline void wma_ndp_register_all_event_handlers(
					tp_wma_handle wma_handle) {}
static inline void wma_ndp_unregister_all_event_handlers(
					tp_wma_handle wma_handle) {}
static inline void wma_ndp_add_wow_wakeup_event(tp_wma_handle wma_handle,
						uint8_t vdev_id) {}
static inline void wma_ndp_wow_event_callback(void *handle, void *event,
					uint32_t len, uint32_t event_id) {}
static inline void wma_add_sta_ndi_mode(tp_wma_handle wma,
					tpAddStaParams add_sta) {}
static inline QDF_STATUS wma_handle_ndp_initiator_req(tp_wma_handle wma_handle,
						      void *req)
{
	return QDF_STATUS_SUCCESS;
}
static inline QDF_STATUS wma_handle_ndp_responder_req(tp_wma_handle wma_handle,
					void *req_params)
{
	return QDF_STATUS_SUCCESS;
}

static inline QDF_STATUS wma_handle_ndp_end_req(tp_wma_handle wma_handle,
						void *req)
{
	return QDF_STATUS_SUCCESS;
}
static inline void wma_delete_sta_req_ndi_mode(tp_wma_handle wma,
					tpDeleteStaParams del_sta)
{
}

static inline uint32_t wma_ndp_get_eventid_from_tlvtag(uint32_t tag)
{
	return 0;
}
#endif /* WLAN_FEATURE_NAN_DATAPATH */
#endif /* __WMA_NAN_DATAPATH_H */
