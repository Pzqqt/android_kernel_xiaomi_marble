/*
 * Copyright (c) 2018 The Linux Foundation. All rights reserved.
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
 * DOC: define internal APIs related to the fwol component
 */

#include "wlan_fw_offload_main.h"
#include "cfg_ucfg_api.h"

struct wlan_fwol_psoc_obj *fwol_get_psoc_obj(struct wlan_objmgr_psoc *psoc)
{
	return wlan_objmgr_psoc_get_comp_private_obj(psoc,
						     WLAN_UMAC_COMP_FWOL);
}

QDF_STATUS fwol_cfg_on_psoc_enable(struct wlan_objmgr_psoc *psoc)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	struct wlan_fwol_psoc_obj *fwol_obj;
	struct wlan_fwol_cfg *fwol_cfg;

	fwol_obj = fwol_get_psoc_obj(psoc);
	if (!fwol_obj) {
		fwol_err("Failed to get FWOL Obj");
		return QDF_STATUS_E_FAILURE;
	}

	fwol_cfg = &fwol_obj->cfg;
	/* Populate the CFG and INI here using CFG_GET */

	return status;
}

QDF_STATUS fwol_cfg_on_psoc_disable(struct wlan_objmgr_psoc *psoc)
{
	/* Clear the CFG structure */
	return QDF_STATUS_SUCCESS;
}
