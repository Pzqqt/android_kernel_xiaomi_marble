/*
 * Copyright (c) 2020 The Linux Foundation. All rights reserved.
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
 * DOC : target_if_cfr_adrastea.c
 *
 * Target interface of CFR for Adrastea implementation
 *
 */

#include <cdp_txrx_ctrl.h>
#include <qdf_nbuf.h>
#include "target_if_cfr.h"
#include "init_deinit_lmac.h"
#include <wlan_objmgr_psoc_obj.h>
#include "wlan_cfr_utils_api.h"
#include "target_if_cfr_adrastea.h"
#include "cfg_ucfg_api.h"
#include "cfr_cfg.h"
#include <target_if.h>

#ifdef WLAN_CFR_ADRASTEA
QDF_STATUS cfr_adrastea_init_pdev(struct wlan_objmgr_psoc *psoc,
				  struct wlan_objmgr_pdev *pdev)
{
	struct pdev_cfr *cfr_pdev;
	struct psoc_cfr *cfr_psoc;
	struct wmi_unified *wmi_handle = NULL;
	struct target_psoc_info *tgt_hdl;
	struct tgt_info *info;
	bool cfr_capable;
	int num_mem_chunks, idx;
	u32 *read_index;

	if (!psoc || !pdev) {
		cfr_err("null pdev or psoc");
		return QDF_STATUS_E_FAILURE;
	}

	cfr_pdev = wlan_objmgr_pdev_get_comp_private_obj(
					pdev, WLAN_UMAC_COMP_CFR);
	if (!cfr_pdev) {
		cfr_err("null pdev cfr");
		return QDF_STATUS_E_FAILURE;
	}

	cfr_psoc = wlan_objmgr_psoc_get_comp_private_obj(
					psoc, WLAN_UMAC_COMP_CFR);

	if (!cfr_psoc) {
		cfr_err("null psoc cfr");
		return QDF_STATUS_E_FAILURE;
	}

	if (wlan_cfr_is_feature_disabled(pdev)) {
		cfr_pdev->is_cfr_capable = 0;
		cfr_psoc->is_cfr_capable = 0;
		cfr_info("cfr disabled");
		return QDF_STATUS_SUCCESS;
	}

	wmi_handle = lmac_get_pdev_wmi_handle(pdev);
	if (!wmi_handle) {
		cfr_err("null wmi handle");
		return QDF_STATUS_E_FAILURE;
	}

	cfr_capable = wmi_service_enabled(wmi_handle,
					  wmi_service_cfr_capture_support);
	cfr_pdev->is_cfr_capable = cfr_capable;
	cfr_psoc->is_cfr_capable = cfr_capable;
	cfr_pdev->chip_type = CFR_CAPTURE_RADIO_ADRASTEA;
	cfr_pdev->cfr_max_sta_count = MAX_CFR_ENABLED_CLIENTS;
	cfr_pdev->subbuf_size = STREAMFS_MAX_SUBBUF_ADRASTEA;
	cfr_pdev->num_subbufs = STREAMFS_NUM_BUF_SUBBUF_ADRASTEA;
	cfr_pdev->pdev_obj = pdev;
	cfr_psoc->psoc_obj = psoc;

	if (!cfr_capable) {
		cfr_err("FW doesn't support CFR");
		return QDF_STATUS_SUCCESS;
	}

	tgt_hdl = wlan_psoc_get_tgt_if_handle(psoc);

	info = &(tgt_hdl->info);
	num_mem_chunks = info->num_mem_chunks;

	/*
	 * Copy the host mem chunk info allocated during init
	 * for CFR capture in cfr pdev
	 */
	for (idx = 0; idx < num_mem_chunks; idx++) {
		if (info->mem_chunks[idx].req_id ==
					CFR_CAPTURE_HOST_MEM_REQ_ID) {
			cfr_pdev->cfr_mem_chunk.vaddr =
						info->mem_chunks[idx].vaddr;
			cfr_pdev->cfr_mem_chunk.paddr =
						info->mem_chunks[idx].paddr;
			cfr_pdev->cfr_mem_chunk.req_id =
						info->mem_chunks[idx].req_id;
			cfr_pdev->cfr_mem_chunk.len = info->mem_chunks[idx].len;

			/* Initialize the read index with default value */
			read_index = (u32 *)info->mem_chunks[idx].vaddr;
			(*read_index) = CFR_HOST_MEM_READ_INDEX_DEFAULT;
			break;
		}
	}

	if (idx == num_mem_chunks) {
		cfr_err("Host mem chunks for CFR req id not allocated\n");
		cfr_pdev->is_cfr_capable = 0;
		cfr_psoc->is_cfr_capable = 0;
		return QDF_STATUS_SUCCESS;
	}

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS cfr_adrastea_deinit_pdev(struct wlan_objmgr_psoc *psoc,
				    struct wlan_objmgr_pdev *pdev)
{
	struct pdev_cfr *pcfr;

	if (!psoc || !pdev) {
		cfr_err("null pdev or psoc");
		return QDF_STATUS_E_FAILURE;
	}

	pcfr = wlan_objmgr_pdev_get_comp_private_obj(
					pdev, WLAN_UMAC_COMP_CFR);
	if (!pcfr) {
		cfr_err("null pdev cfr");
		return QDF_STATUS_E_FAILURE;
	}

	if (!pcfr->is_cfr_capable) {
		cfr_info("cfr disabled or FW not support");
		return QDF_STATUS_SUCCESS;
	}

	qdf_mem_zero(&pcfr->cfr_mem_chunk,
		     sizeof(struct cfr_wmi_host_mem_chunk));

	return QDF_STATUS_SUCCESS;
}
#endif
