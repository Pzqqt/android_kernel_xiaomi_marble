/*
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

/*
 * DOC: contains MLO manager roaming related functionality
 */
#include <wlan_cmn.h>
#include <wlan_cm_public_struct.h>
#include <wlan_cm_roam_public_struct.h>
#include "wlan_mlo_mgr_cmn.h"
#include "wlan_mlo_mgr_main.h"
#include "wlan_mlo_mgr_roam.h"
#include "wlan_mlo_mgr_public_structs.h"
#include "wlan_mlo_mgr_sta.h"
#include <../../core/src/wlan_cm_roam_i.h>
#include "wlan_cm_roam_api.h"

QDF_STATUS mlo_fw_roam_sync_req(struct wlan_objmgr_psoc *psoc, uint8_t vdev_id,
				void *event, uint32_t event_data_len)
{
	struct roam_offload_synch_ind *sync_ind;

	sync_ind = (struct roam_offload_synch_ind *)event;
	if (!sync_ind)
		return QDF_STATUS_E_FAILURE;

	return QDF_STATUS_SUCCESS;
}

void
mlo_fw_ho_fail_req(struct wlan_objmgr_psoc *psoc,
		   uint8_t vdev_id, struct qdf_mac_addr bssid)
{
}

QDF_STATUS
mlo_get_sta_link_mac_addr(uint8_t vdev_id,
			  struct roam_offload_synch_ind *sync_ind,
			  struct qdf_mac_addr *link_mac_addr)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	if (!sync_ind || !sync_ind->num_setup_links)
		return QDF_STATUS_E_FAILURE;

	return status;
}

bool is_multi_link_roam(struct roam_offload_synch_ind *sync_ind)
{
	if (!sync_ind)
		return false;

	if (sync_ind->num_setup_links)
		return true;

	return false;
}

QDF_STATUS mlo_enable_rso(struct wlan_objmgr_pdev *pdev,
			  struct wlan_objmgr_vdev *vdev)
{
	struct wlan_objmgr_vdev *assoc_vdev;

	if (wlan_vdev_mlme_is_mlo_link_vdev(vdev)) {
		assoc_vdev = wlan_mlo_get_assoc_link_vdev(vdev);
		if (!assoc_vdev) {
			mlo_err("Assoc vdev is null");
			return QDF_STATUS_E_NULL_VALUE;
		}

		cm_roam_start_init_on_connect(pdev,
					      wlan_vdev_get_id(assoc_vdev));
	}

	return QDF_STATUS_SUCCESS;
}

