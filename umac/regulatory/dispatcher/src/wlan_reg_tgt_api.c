/*
 * Copyright (c) 2017-2020 The Linux Foundation. All rights reserved.
 *
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
 * @file wlan_req_tgt_api.c
 * @brief contains regulatory target interface definations
 */

#include <qdf_status.h>
#include <qdf_types.h>
#include <wlan_cmn.h>
#include <reg_services_public_struct.h>
#include <wlan_reg_tgt_api.h>
#include <wlan_objmgr_psoc_obj.h>
#include <../../core/src/reg_priv_objs.h>
#include <../../core/src/reg_utils.h>
#include <../../core/src/reg_services_common.h>
#include <../../core/src/reg_lte.h>
#include <../../core/src/reg_build_chan_list.h>
#include <../../core/src/reg_offload_11d_scan.h>

/**
 * tgt_process_master_chan_list() - process master channel list
 * @reg_info: regulatory info
 *
 * Return: QDF_STATUS
 */
QDF_STATUS tgt_reg_process_master_chan_list(struct cur_regulatory_info
					    *reg_info)
{
	struct wlan_regulatory_psoc_priv_obj *soc_reg;
	struct wlan_objmgr_psoc *psoc;

	psoc = reg_info->psoc;
	soc_reg = reg_get_psoc_obj(psoc);

	if (!IS_VALID_PSOC_REG_OBJ(soc_reg)) {
		reg_err("psoc reg component is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	if (!soc_reg->offload_enabled)
		return QDF_STATUS_SUCCESS;

	return reg_process_master_chan_list(reg_info);
}

QDF_STATUS tgt_reg_process_11d_new_country(struct wlan_objmgr_psoc *psoc,
		struct reg_11d_new_country *reg_11d_new_cc)
{
	return reg_save_new_11d_country(psoc, reg_11d_new_cc->alpha2);
}

QDF_STATUS tgt_reg_set_regdb_offloaded(struct wlan_objmgr_psoc *psoc,
		bool val)
{
	return reg_set_regdb_offloaded(psoc, val);
}

QDF_STATUS tgt_reg_set_11d_offloaded(struct wlan_objmgr_psoc *psoc,
		bool val)
{
	return reg_set_11d_offloaded(psoc, val);
}

QDF_STATUS tgt_reg_process_ch_avoid_event(struct wlan_objmgr_psoc *psoc,
		struct ch_avoid_ind_type *ch_avoid_evnt)
{
	return reg_process_ch_avoid_event(psoc, ch_avoid_evnt);
}

bool tgt_reg_ignore_fw_reg_offload_ind(struct wlan_objmgr_psoc *psoc)
{
	return reg_get_ignore_fw_reg_offload_ind(psoc);
}

QDF_STATUS tgt_reg_set_6ghz_supported(struct wlan_objmgr_psoc *psoc,
				      bool val)
{
	return reg_set_6ghz_supported(psoc, val);
}

QDF_STATUS tgt_reg_set_5dot9_ghz_supported(struct wlan_objmgr_psoc *psoc,
					   bool val)
{
	return reg_set_5dot9_ghz_supported(psoc, val);
}
