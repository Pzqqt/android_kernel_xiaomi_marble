/*
 * Copyright (c) 2017-2021 The Linux Foundation. All rights reserved.
 * Copyright (c) 2021-2022 Qualcomm Innovation Center, Inc. All rights reserved.
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
#define REG_MAX_20M_SUB_CH   8
#ifdef CONFIG_AFC_SUPPORT
#define MIN_AFC_BW 2
#define MAX_AFC_BW 160
#endif

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

#ifdef CONFIG_REG_CLIENT
/**
 * reg_save_reg_rules_to_pdev() - Save psoc reg-rules to pdev.
 * @pdev_priv_obj: Pointer to regdb pdev private object.
 */
void reg_save_reg_rules_to_pdev(
		struct reg_rule_info *psoc_reg_rules,
		struct wlan_regulatory_pdev_priv_obj *pdev_priv_obj);
#else
static inline void
reg_save_reg_rules_to_pdev(struct reg_rule_info *psoc_reg_rules,
			   struct wlan_regulatory_pdev_priv_obj *pdev_priv_obj)
{
}
#endif

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

#ifdef CONFIG_BAND_6GHZ
/**
 * reg_process_master_chan_list_ext() - Compute master channel extended list
 * based on the regulatory rules.
 * @reg_info: Pointer to regulatory info
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
reg_process_master_chan_list_ext(struct cur_regulatory_info *reg_info);

/**
 * reg_get_6g_ap_master_chan_list() - Get  an ap  master channel list depending
 * on * ap power type
 * @ap_pwr_type: Power type (LPI/VLP/SP)
 * @chan_list: Pointer to the channel list. The output channel list
 *
 * Return: QDF_STATUS
 */
QDF_STATUS reg_get_6g_ap_master_chan_list(struct wlan_objmgr_pdev *pdev,
					  enum reg_6g_ap_type ap_pwr_type,
					  struct regulatory_channel *chan_list);

#ifdef CONFIG_REG_CLIENT
/**
 * reg_get_power_string() - get power string from power enum type
 * @power_type: power type enum value
 *
 * Return: power type string
 */
const char *reg_get_power_string(enum reg_6g_ap_type power_type);
#endif

#ifdef CONFIG_AFC_SUPPORT
/**
 * reg_process_afc_event() - Process the afc event and compute the 6G AFC
 * channel list based on the frequency range and channel frequency indices set.
 * @reg_info: Pointer to regulatory info
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
reg_process_afc_event(struct afc_regulatory_info *afc_info);
#endif

#else /* CONFIG_BAND_6GHZ */
static inline QDF_STATUS
reg_get_6g_ap_master_chan_list(struct wlan_objmgr_pdev *pdev,
			       enum reg_6g_ap_type ap_pwr_type,
			       struct regulatory_channel *chan_list)
{
	return QDF_STATUS_E_FAILURE;
}
#endif /* CONFIG_BAND_6GHZ */
/**
 * reg_process_master_chan_list() - Compute master channel list based on the
 * regulatory rules.
 * @reg_info: Pointer to regulatory info
 *
 * Return: QDF_STATUS
 */
QDF_STATUS reg_process_master_chan_list(struct cur_regulatory_info *reg_info);

/**
 * reg_get_current_chan_list() - provide the pdev current channel list
 * @pdev: pdev pointer
 * @chan_list: channel list pointer
 *
 * Return: QDF_STATUS
 */
QDF_STATUS reg_get_current_chan_list(struct wlan_objmgr_pdev *pdev,
				     struct regulatory_channel *chan_list);

#if defined(CONFIG_AFC_SUPPORT) && defined(CONFIG_BAND_6GHZ)
/**
 * reg_get_6g_afc_chan_list() - provide the pdev afc channel list
 * @pdev: pdev pointer
 * @chan_list: channel list pointer
 *
 * Return: QDF_STATUS
 */
QDF_STATUS reg_get_6g_afc_chan_list(struct wlan_objmgr_pdev *pdev,
				    struct regulatory_channel *chan_list);

/**
 * reg_get_6g_afc_mas_chan_list() - provide the pdev afc master channel list
 * @pdev: pdev pointer
 * @chan_list: channel list pointer
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
reg_get_6g_afc_mas_chan_list(struct wlan_objmgr_pdev *pdev,
			     struct regulatory_channel *chan_list);

/**
 * reg_psd_2_eirp() - Calculate EIRP from PSD and bandwidth
 * channel list
 * @pdev: pdev pointer
 * @psd: Power Spectral Density in dBm/MHz
 * @ch_bw: Bandwdith of a channel in MHz (20/40/80/160/320 etc)
 * @eirp:  EIRP power  in dBm
 *
 * Return: QDF_STATUS
 */
QDF_STATUS reg_psd_2_eirp(struct wlan_objmgr_pdev *pdev,
			  int16_t psd,
			  uint16_t ch_bw,
			  int16_t *eirp);
#endif

#ifdef CONFIG_REG_CLIENT
/**
 * reg_get_secondary_current_chan_list() - provide the pdev secondary current
 * channel list
 * @pdev: pdev pointer
 * @chan_list: channel list pointer
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
reg_get_secondary_current_chan_list(struct wlan_objmgr_pdev *pdev,
				    struct regulatory_channel *chan_list);
#endif
#endif
