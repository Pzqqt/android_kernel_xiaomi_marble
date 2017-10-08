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
 * DOC: Define API's for wow pattern addition and deletion in fwr
 */

#include "wlan_pmo_wow.h"
#include "wlan_pmo_tgt_api.h"
#include "wlan_pmo_main.h"
#include "wlan_pmo_obj_mgmt_public_struct.h"
#include <wlan_scan_ucfg_api.h>
#include "wlan_pmo_static_config.h"
#include "wlan_reg_services_api.h"

static inline int pmo_find_wow_ptrn_len(const char *ptrn)
{
	int len = 0;

	while (*ptrn != '\0' && *ptrn != PMO_WOW_INTER_PTRN_TOKENIZER) {
		len++;
		ptrn++;
	}

	return len;
}

QDF_STATUS pmo_core_add_wow_pattern(struct wlan_objmgr_vdev *vdev,
		const char *ptrn)
{
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS pmo_core_del_wow_pattern(struct wlan_objmgr_vdev *vdev,
		const char *ptrn)
{
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS pmo_core_wow_enter(struct wlan_objmgr_vdev *vdev,
		struct pmo_wow_enter_params *wow_enter_param)
{
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS pmo_core_wow_exit(struct wlan_objmgr_vdev *vdev)
{
	return QDF_STATUS_SUCCESS;
}

void pmo_core_enable_wakeup_event(struct wlan_objmgr_psoc *psoc,
	uint32_t vdev_id, uint32_t *bitmap)
{
	QDF_STATUS status;
	struct wlan_objmgr_vdev *vdev;

	PMO_ENTER();
	if (!psoc) {
		pmo_err("psoc is null");
		goto out;
	}

	vdev = pmo_psoc_get_vdev(psoc, vdev_id);
	if (!vdev) {
		pmo_err("vdev is NULL");
		goto out;
	}

	status = pmo_vdev_get_ref(vdev);
	if (QDF_IS_STATUS_ERROR(status))
		goto out;

	pmo_info("enable wakeup event vdev_id %d wake up event 0x%x%x%x%x",
		vdev_id, bitmap[0], bitmap[1], bitmap[2], bitmap[3]);
	pmo_tgt_enable_wow_wakeup_event(vdev, bitmap);

	pmo_vdev_put_ref(vdev);

out:
	PMO_EXIT();
}

void pmo_core_disable_wakeup_event(struct wlan_objmgr_psoc *psoc,
	uint32_t vdev_id, uint32_t *bitmap)
{
	QDF_STATUS status;
	struct wlan_objmgr_vdev *vdev;

	PMO_ENTER();
	if (!psoc) {
		pmo_err("psoc is null");
		goto out;
	}

	vdev = pmo_psoc_get_vdev(psoc, vdev_id);
	if (!vdev) {
		pmo_err("vdev is NULL");
		goto out;
	}

	status = pmo_vdev_get_ref(vdev);
	if (QDF_IS_STATUS_ERROR(status))
		goto out;

	pmo_info("Disable wakeup event vdev_id %d wake up event 0x%x%x%x%x",
		vdev_id, bitmap[0], bitmap[1], bitmap[2], bitmap[3]);
	pmo_tgt_disable_wow_wakeup_event(vdev, bitmap);

	pmo_vdev_put_ref(vdev);

out:
	PMO_EXIT();
}

/**
 * pmo_is_beaconing_vdev_up(): check if a beaconning vdev is up
 * @psoc: objmgr psoc handle
 *
 * Return TRUE if beaconning vdev is up
 */
static
bool pmo_is_beaconing_vdev_up(struct wlan_objmgr_psoc *psoc)
{
	int vdev_id;
	struct wlan_objmgr_vdev *vdev;
	enum tQDF_ADAPTER_MODE vdev_opmode;
	bool is_beaconing;
	QDF_STATUS status;

	/* Iterate through VDEV list */
	for (vdev_id = 0; vdev_id < WLAN_UMAC_PSOC_MAX_VDEVS; vdev_id++) {
		vdev = pmo_psoc_get_vdev(psoc, vdev_id);
		if (!vdev)
			continue;

		status = pmo_vdev_get_ref(vdev);
		if (QDF_IS_STATUS_ERROR(status))
			continue;

		vdev_opmode = pmo_get_vdev_opmode(vdev);
		is_beaconing = pmo_is_vdev_in_beaconning_mode(vdev_opmode) &&
			pmo_is_vdev_up(vdev);

		pmo_vdev_put_ref(vdev);

		if (is_beaconing)
			return true;
	}

	return false;
}

/**
 * pmo_support_wow_for_beaconing: wow query for beaconning
 * @psoc: objmgr psoc handle
 *
 * Need to configure wow to enable beaconning offload when
 * a beaconing vdev is up and beaonning offload is configured.
 *
 * Return: true if we need to enable wow for beaconning offload
 */
static
bool pmo_support_wow_for_beaconing(struct wlan_objmgr_psoc *psoc)
{
	/*
	 * if (WMI_SERVICE_IS_ENABLED(wma->wmi_service_bitmap,
	 *			WMI_SERVICE_BEACON_OFFLOAD))
	 */
	return pmo_is_beaconing_vdev_up(psoc);
}

bool pmo_core_is_wow_applicable(struct wlan_objmgr_psoc *psoc)
{
	int vdev_id;
	struct wlan_objmgr_vdev *vdev;
	bool is_wow_applicable;
	QDF_STATUS status;

	if (!psoc) {
		pmo_err("psoc is null");
		return false;
	}

	if (pmo_support_wow_for_beaconing(psoc)) {
		pmo_debug("one of vdev is in beaconning mode, enabling wow");
		return true;
	}

	if (wlan_reg_is_11d_scan_inprogress(psoc)) {
		pmo_debug("11d scan is in progress, enabling wow");
		return true;
	}

	/* Iterate through VDEV list */
	for (vdev_id = 0; vdev_id < WLAN_UMAC_PSOC_MAX_VDEVS; vdev_id++) {
		vdev = pmo_psoc_get_vdev(psoc, vdev_id);
		if (!vdev)
			continue;

		status = pmo_vdev_get_ref(vdev);
		if (QDF_IS_STATUS_ERROR(status))
			continue;

		if (pmo_core_is_vdev_connected(vdev)) {
			pmo_debug("STA is connected, enabling wow");
			is_wow_applicable = true;
		} else if (ucfg_scan_get_pno_in_progress(vdev)) {
			pmo_debug("NLO is in progress, enabling wow");
			is_wow_applicable = true;
		} else if (pmo_core_is_extscan_in_progress(vdev)) {
			pmo_debug("EXT is in progress, enabling wow");
			is_wow_applicable = true;
		} else if (pmo_core_is_p2plo_in_progress(vdev)) {
			pmo_debug("P2P LO is in progress, enabling wow");
			is_wow_applicable = true;
		} else if (pmo_core_is_lpass_enabled(vdev)) {
			pmo_debug("LPASS is enabled, enabling WoW");
			is_wow_applicable = true;
		} else if (pmo_core_is_nan_enabled(vdev)) {
			pmo_debug("NAN is enabled, enabling WoW");
			is_wow_applicable = true;
		} else if (pmo_core_get_vdev_op_mode(vdev) == QDF_NDI_MODE) {
			pmo_debug("vdev %d is in NAN data mode, enabling wow",
				vdev_id);
			is_wow_applicable = true;
		}

		pmo_vdev_put_ref(vdev);

		if (is_wow_applicable)
			return true;
	}

	pmo_debug("All vdev are in disconnected state\n"
		"and pno/extscan is not in progress, skipping wow");

	return false;
}

void pmo_set_wow_event_bitmap(WOW_WAKE_EVENT_TYPE event,
			      uint32_t wow_bitmap_size,
			      uint32_t *bitmask)
{
	uint32_t bit_idx = 0, idx = 0;

	if (!bitmask || wow_bitmap_size < PMO_WOW_MAX_EVENT_BM_LEN) {
		pmo_err("wow bitmask length shorter than %d",
			 PMO_WOW_MAX_EVENT_BM_LEN);
		return;
	}
	pmo_get_event_bitmap_idx(event, wow_bitmap_size, &bit_idx, &idx);
	bitmask[idx] |= 1 << bit_idx;

	pmo_debug("%s: bitmask updated %x%x%x%x",
		 __func__, bitmask[0], bitmask[1], bitmask[2], bitmask[3]);
}

void pmo_set_sta_wow_bitmask(uint32_t *bitmask, uint32_t wow_bitmap_size)
{

	pmo_set_wow_event_bitmap(WOW_CSA_IE_EVENT,
			     wow_bitmap_size,
			     bitmask);
	pmo_set_wow_event_bitmap(WOW_CLIENT_KICKOUT_EVENT,
			     wow_bitmap_size,
			     bitmask);
	pmo_set_wow_event_bitmap(WOW_PATTERN_MATCH_EVENT,
			     wow_bitmap_size,
			     bitmask);
	pmo_set_wow_event_bitmap(WOW_MAGIC_PKT_RECVD_EVENT,
			     wow_bitmap_size,
			     bitmask);
	pmo_set_wow_event_bitmap(WOW_DEAUTH_RECVD_EVENT,
			     wow_bitmap_size,
			     bitmask);
	pmo_set_wow_event_bitmap(WOW_DISASSOC_RECVD_EVENT,
			     wow_bitmap_size,
			     bitmask);
	pmo_set_wow_event_bitmap(WOW_BMISS_EVENT,
			     wow_bitmap_size,
			     bitmask);
	pmo_set_wow_event_bitmap(WOW_GTK_ERR_EVENT,
			     wow_bitmap_size,
			     bitmask);
	pmo_set_wow_event_bitmap(WOW_BETTER_AP_EVENT,
			     wow_bitmap_size,
			     bitmask);
	pmo_set_wow_event_bitmap(WOW_HTT_EVENT,
			     wow_bitmap_size,
			     bitmask);
	pmo_set_wow_event_bitmap(WOW_RA_MATCH_EVENT,
			     wow_bitmap_size,
			     bitmask);
	pmo_set_wow_event_bitmap(WOW_NLO_DETECTED_EVENT,
			     wow_bitmap_size,
			     bitmask);
	pmo_set_wow_event_bitmap(WOW_EXTSCAN_EVENT,
			     wow_bitmap_size,
			     bitmask);
	pmo_set_wow_event_bitmap(WOW_OEM_RESPONSE_EVENT,
			     wow_bitmap_size,
			     bitmask);
	pmo_set_wow_event_bitmap(WOW_TDLS_CONN_TRACKER_EVENT,
			     wow_bitmap_size,
			     bitmask);
	pmo_set_wow_event_bitmap(WOW_11D_SCAN_EVENT,
				 wow_bitmap_size,
				 bitmask);

}

void pmo_set_sap_wow_bitmask(uint32_t *bitmask, uint32_t wow_bitmap_size)
{

	pmo_set_wow_event_bitmap(WOW_PROBE_REQ_WPS_IE_EVENT,
			     wow_bitmap_size,
			     bitmask);
	pmo_set_wow_event_bitmap(WOW_PATTERN_MATCH_EVENT,
			     wow_bitmap_size,
			     bitmask);
	pmo_set_wow_event_bitmap(WOW_AUTH_REQ_EVENT,
			     wow_bitmap_size,
			     bitmask);
	pmo_set_wow_event_bitmap(WOW_ASSOC_REQ_EVENT,
			     wow_bitmap_size,
			     bitmask);
	pmo_set_wow_event_bitmap(WOW_DEAUTH_RECVD_EVENT,
			     wow_bitmap_size,
			     bitmask);
	pmo_set_wow_event_bitmap(WOW_DISASSOC_RECVD_EVENT,
			     wow_bitmap_size,
			     bitmask);
	pmo_set_wow_event_bitmap(WOW_HTT_EVENT,
			     wow_bitmap_size,
			     bitmask);
}
