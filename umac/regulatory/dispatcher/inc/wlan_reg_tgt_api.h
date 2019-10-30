/*
 * Copyright (c) 2017-2019 The Linux Foundation. All rights reserved.
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

QDF_STATUS tgt_reg_process_master_chan_list(struct cur_regulatory_info
					    *reg_info);

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

#endif
