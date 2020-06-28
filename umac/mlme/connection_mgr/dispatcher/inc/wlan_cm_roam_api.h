/*
 * Copyright (c) 2020, The Linux Foundation. All rights reserved.
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

/*
 * DOC: wlan_cm_roam_api.h
 *
 * Implementation for the Common Roaming interfaces.
 */

#ifndef WLAN_CM_ROAM_API_H__
#define WLAN_CM_ROAM_API_H__

#include "wlan_mlme_dbg.h"
#include "wlan_cm_roam_public_srtuct.h"
#include "wlan_mlme_main.h"
#include "wlan_mlme_api.h"

#ifdef WLAN_FEATURE_ROAM_OFFLOAD
/**
 * wlan_cm_roam_extract_btm_response() - Extract BTM rsp stats
 * @wmi:       wmi handle
 * @evt_buf:   Pointer to the event buffer
 * @dst:       Pointer to destination structure to fill data
 * @idx:       TLV id
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
wlan_cm_roam_extract_btm_response(wmi_unified_t wmi, void *evt_buf,
				  struct roam_btm_response_data *dst,
				  uint8_t idx);

/**
 * wlan_cm_roam_extract_roam_initial_info() - Extract Roam Initial stats
 * @wmi:       wmi handle
 * @evt_buf:   Pointer to the event buffer
 * @dst:       Pointer to destination structure to fill data
 * @idx:       TLV id
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
wlan_cm_roam_extract_roam_initial_info(wmi_unified_t wmi, void *evt_buf,
				       struct roam_initial_data *dst,
				       uint8_t idx);

/**
 * wlan_cm_roam_activate_pcl_per_vdev  - Set the PCL command to be sent per
 * vdev instead of pdev.
 * @psoc: PSOC pointer
 * @vdev_id: VDEV id
 * @pcl_per_vdev: Activate vdev PCL type. 1- VDEV PCL, 0- PDEV PCL
 *
 * pcl_per_vdev will be set when:
 *  STA + STA is connected in DBS mode and roaming init is done on
 *  the 2nd STA.
 *
 * pcl_per_vdev will be false when only 1 sta connection exists or
 * when 2nd sta gets disconnected
 *
 * Return: None
 */
void wlan_cm_roam_activate_pcl_per_vdev(struct wlan_objmgr_psoc *psoc,
					uint8_t vdev_id,
					bool pcl_per_vdev);

/**
 * wlan_cm_roam_is_pcl_per_vdev_active  - API to know if the pcl command needs to be
 * sent per vdev or not
 * @psoc: PSOC pointer
 * @vdev_id: VDEV id
 *
 * Return: PCL level
 */
bool wlan_cm_roam_is_pcl_per_vdev_active(struct wlan_objmgr_psoc *psoc,
					 uint8_t vdev_id);

/**
 * wlan_cm_dual_sta_is_freq_allowed  - This API is used to check if the
 * provided frequency is allowed for the 2nd STA vdev for connection.
 * @psoc:   Pointer to PSOC object
 * @freq:   Frequency in the given frequency list for the STA that is about to
 * connect
 * @opmode: Operational mode
 *
 * This API will be called while filling scan filter channels during connection.
 *
 * Return: True if this channel is allowed for connection when dual sta roaming
 * is enabled
 */
bool
wlan_cm_dual_sta_is_freq_allowed(struct wlan_objmgr_psoc *psoc, uint32_t freq,
				 enum QDF_OPMODE opmode);

/**
 * wlan_cm_dual_sta_roam_update_connect_channels  - Fill the allowed channels
 * for connection of the 2nd STA based on the 1st STA connected band if dual
 * sta roaming is enabled.
 * @psoc:   Pointer to PSOC object
 * @filter: Pointer to scan filter
 *
 * Return: None
 */
void
wlan_cm_dual_sta_roam_update_connect_channels(struct wlan_objmgr_psoc *psoc,
					      struct scan_filter *filter);
#else
static inline
void wlan_cm_roam_activate_pcl_per_vdev(struct wlan_objmgr_psoc *psoc,
					uint8_t vdev_id,
					bool pcl_per_vdev)
{}

static inline
bool wlan_cm_roam_is_pcl_per_vdev_active(struct wlan_objmgr_psoc *psoc,
					 uint8_t vdev_id)
{
	return false;
}

static inline bool
wlan_cm_dual_sta_is_freq_allowed(struct wlan_objmgr_psoc *psoc, uint32_t freq,
				 enum QDF_OPMODE opmode)
{
	return true;
}

static inline void
wlan_cm_dual_sta_roam_update_connect_channels(struct wlan_objmgr_psoc *psoc,
					      struct scan_filter *filter)
{}

static inline QDF_STATUS
wlan_cm_roam_extract_btm_response(wmi_unified_t wmi, void *evt_buf,
				  struct roam_btm_response_data *dst,
				  uint8_t idx)
{
	return true;
}

static inline QDF_STATUS
wlan_cm_roam_extract_roam_initial_info(wmi_unified_t wmi, void *evt_buf,
				       struct roam_initial_data *dst,
				       uint8_t idx)
{
	return true;
}

#endif  /* FEATURE_ROAM_OFFLOAD */
#endif  /* WLAN_CM_ROAM_API_H__ */
