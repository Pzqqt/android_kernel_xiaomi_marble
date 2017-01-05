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
	uint32_t vdev_id, uint32_t bitmap)
{
	struct wlan_objmgr_vdev *vdev;

	PMO_ENTER();
	if (!psoc) {
		pmo_err("psoc is null");
		goto out;
	}

	vdev = wlan_objmgr_get_vdev_by_id_from_psoc(psoc, vdev_id, WLAN_PMO_ID);
	if (!vdev) {
		pmo_err("vdev is NULL");
		goto out;
	}

	pmo_info("enable wakeup event vdev_id %d wake up event 0x%x",
		vdev_id, bitmap);
	pmo_tgt_enable_wow_wakeup_event(vdev, bitmap);
	wlan_objmgr_vdev_release_ref(vdev, WLAN_PMO_ID);
out:
	PMO_EXIT();

}

void pmo_core_disable_wakeup_event(struct wlan_objmgr_psoc *psoc,
	uint32_t vdev_id, uint32_t bitmap)
{
	struct wlan_objmgr_vdev *vdev;

	PMO_ENTER();
	if (!psoc) {
		pmo_err("psoc is null");
		goto out;
	}

	vdev = wlan_objmgr_get_vdev_by_id_from_psoc(psoc, vdev_id, WLAN_PMO_ID);
	if (!vdev) {
		pmo_err("vdev is NULL");
		goto out;
	}

	pmo_info("Disable wakeup eventvdev_id %d wake up event 0x%x",
		vdev_id, bitmap);
	pmo_tgt_disable_wow_wakeup_event(vdev, bitmap);
	wlan_objmgr_vdev_release_ref(vdev, WLAN_PMO_ID);
out:
	PMO_EXIT();

}

