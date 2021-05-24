/*
 * Copyright (c) 2017-2021 The Linux Foundation. All rights reserved.
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
 * DOC: wlan_reg_tgt_api.h
 * This file provides prototypes of the regulatory component target
 * interface routines
 */

#ifndef __WLAN_REG_TGT_API_H
#define __WLAN_REG_TGT_API_H

/**
 * tgt_process_master_chan_list() - process master channel list
 * @reg_info: regulatory info
 *
 * Return: QDF_STATUS
 */
QDF_STATUS tgt_reg_process_master_chan_list(struct cur_regulatory_info
					    *reg_info);

#ifdef CONFIG_BAND_6GHZ
/**
 * tgt_reg_process_master_chan_list_ext() - process master ext channel list
 * @reg_info: regulatory info
 *
 * Return: QDF_STATUS
 */
QDF_STATUS tgt_reg_process_master_chan_list_ext(struct cur_regulatory_info
						*reg_info);

#ifdef CONFIG_AFC_SUPPORT
/**
 * tgt_reg_process_afc_event() - process the AFC event
 * @afc_info: AFC regulatory info
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
tgt_reg_process_afc_event(struct afc_regulatory_info *afc_info);
#endif
#endif

/**
 * tgt_reg_process_11d_new_country() - process new 11d country event
 * @psoc: pointer to psoc
 * @reg_11d_new_cc: new 11d country pointer
 *
 * Return: QDF_STATUS
 */
QDF_STATUS tgt_reg_process_11d_new_country(struct wlan_objmgr_psoc *psoc,
		struct reg_11d_new_country *reg_11d_new_cc);

/**
 * tgt_reg_set_regdb_offloaded() - set/clear regulatory offloaded flag
 *
 * @psoc: psoc pointer
 * Return: Success or Failure
 */
QDF_STATUS tgt_reg_set_regdb_offloaded(struct wlan_objmgr_psoc *psoc,
		bool val);

/**
 * tgt_reg_set_11d_offloaded() - set/clear 11d offloaded flag
 *
 * @psoc: psoc pointer
 * Return: Success or Failure
 */
QDF_STATUS tgt_reg_set_11d_offloaded(struct wlan_objmgr_psoc *psoc,
		bool val);
/**
 * tgt_reg_process_ch_avoid_event() - process new ch avoid event
 * @psoc: pointer to psoc
 * @ch_avoid_evnt: channel avoid event
 *
 * Return: QDF_STATUS
 */
QDF_STATUS tgt_reg_process_ch_avoid_event(struct wlan_objmgr_psoc *psoc,
		struct ch_avoid_ind_type *ch_avoid_evnt);

/**
 * tgt_reg_ignore_fw_reg_offload_ind() - Check whether regdb offload indication
 * from FW needs to be ignored.
 * @psoc: Pointer to psoc
 */
bool tgt_reg_ignore_fw_reg_offload_ind(struct wlan_objmgr_psoc *psoc);

/**
 * tgt_reg_set_6ghz_supported() - Whether 6ghz is supported by the chip
 * @psoc: Pointer to psoc
 * @val: value
 */
QDF_STATUS tgt_reg_set_6ghz_supported(struct wlan_objmgr_psoc *psoc,
				      bool val);

/**
 * tgt_reg_set_5dot9_ghz_supported() - Whether 5.9ghz is supported by the chip
 * @psoc: Pointer to psoc
 * @val: value
 */
QDF_STATUS tgt_reg_set_5dot9_ghz_supported(struct wlan_objmgr_psoc *psoc,
					   bool val);

/**
 * tgt_reg_set_ext_tpc_supported() - Whether FW supports new WMI cmd for TPC
 * @psoc: Pointer to psoc
 * @val: value
 *
 * Return: QDF_STATUS
 */
QDF_STATUS tgt_reg_set_ext_tpc_supported(struct wlan_objmgr_psoc *psoc,
					 bool val);

#if defined(CONFIG_BAND_6GHZ)
/**
 * tgt_reg_set_lower_6g_edge_ch_supp() - Assign the value set by FW for lower
 * 6ghz edge channel (5935 MHz) support
 * @psoc: Pointer to psoc
 * @val: value
 */
QDF_STATUS tgt_reg_set_lower_6g_edge_ch_supp(struct wlan_objmgr_psoc *psoc,
					     bool val);

/**
 * tgt_reg_set_disable_upper_6g_edge_ch_supp() - Assign the value set by FW
 * for upper 6G edge channel {7115MHz) disablement
 * @psoc: Pointer to psoc
 * @val: value
 */
QDF_STATUS
tgt_reg_set_disable_upper_6g_edge_ch_supp(struct wlan_objmgr_psoc *psoc,
					  bool val);
#else
static inline
QDF_STATUS tgt_reg_set_lower_6g_edge_ch_supp(struct wlan_objmgr_psoc *psoc,
					     bool val)
{
	return QDF_STATUS_E_FAILURE;
}

static inline QDF_STATUS
tgt_reg_set_disable_upper_6g_edge_ch_supp(struct wlan_objmgr_psoc *psoc,
					  bool val)

{
	return QDF_STATUS_E_FAILURE;
}
#endif
#endif
