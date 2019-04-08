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
 * DOC: reg_build_chan_list.h
 * This file provides prototypes of the regulatory component to build master
 * and current channel list.
 */

#ifndef __REG_BUILD_CHAN_LIST_H__
#define __REG_BUILD_CHAN_LIST_H__

#define CHAN_12_CENT_FREQ 2467
#define CHAN_13_CENT_FREQ 2472

/**
 * reg_reset_reg_rules() - provides the reg domain rules info
 * @reg_rules: reg rules pointer
 *
 * Return: None
 */
void reg_reset_reg_rules(struct reg_rule_info *reg_rules);

/**
 * reg_init_pdev_mas_chan_list() - Initialize pdev master channel list
 * @pdev_priv_obj: Pointer to regdb pdev private object.
 * @mas_chan_params: Master channel params.
 */
void reg_init_pdev_mas_chan_list(
		struct wlan_regulatory_pdev_priv_obj *pdev_priv_obj,
		struct mas_chan_params *mas_chan_params);

/**
 * reg_save_reg_rules_to_pdev() - Save psoc reg-rules to pdev.
 * @pdev_priv_obj: Pointer to regdb pdev private object.
 */
void reg_save_reg_rules_to_pdev(
		struct reg_rule_info *psoc_reg_rules,
		struct wlan_regulatory_pdev_priv_obj *pdev_priv_obj);

/**
 * reg_compute_pdev_current_chan_list() - Compute pdev current channel list.
 * @pdev_priv_obj: Pointer to regdb pdev private object.
 */
void reg_compute_pdev_current_chan_list(
		struct wlan_regulatory_pdev_priv_obj *pdev_priv_obj);

/**
 * reg_propagate_mas_chan_list_to_pdev() - Propagate master channel list to pdev
 * @psoc: Pointer to psoc object.
 * @object: Void pointer to pdev object.
 * @arg: Pointer to direction.
 */
void reg_propagate_mas_chan_list_to_pdev(struct wlan_objmgr_psoc *psoc,
					 void *object, void *arg);

/**
 * reg_process_master_chan_list() - Compute master channel list based on the
 * regulatory rules.
 * @reg_info: Pointer to regulatory info
 *
 * Return: QDF_STATUS
 */
QDF_STATUS reg_process_master_chan_list(struct cur_regulatory_info *reg_info);

QDF_STATUS reg_get_current_chan_list(struct wlan_objmgr_pdev *pdev,
				     struct regulatory_channel *chan_list);

/**
 * reg_update_nol_history_ch() - Set nol-history flag for the channels in the
 * list.
 *
 * @pdev: Pdev ptr.
 * @ch_list: Input channel list.
 * @num_ch: Number of channels.
 * @nol_history_ch: NOL-History flag.
 *
 * Return: void
 */
void reg_update_nol_history_ch(struct wlan_objmgr_pdev *pdev,
			       uint8_t *chan_list,
			       uint8_t num_chan,
			       bool nol_history_chan);

#endif
