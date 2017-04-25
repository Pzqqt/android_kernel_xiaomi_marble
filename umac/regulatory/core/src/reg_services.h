/*
 * Copyright (c) 2017 The Linux Foundation. All rights reserved.
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
 * DOC: reg_services.h
 * This file provides prototypes of the regulatory component
 * service functions
 */

#ifndef __REG_SERVICES_H_
#define __REG_SERVICES_H_

#include <qdf_types.h>
#include <qdf_status.h>
#include <wlan_objmgr_cmn.h>
#include <wlan_objmgr_global_obj.h>
#include <wlan_objmgr_psoc_obj.h>
#include <wlan_objmgr_pdev_obj.h>
#include "reg_db.h"
#include <reg_services_public_struct.h>

#define REG_MIN_24GHZ_CH_NUM channel_map[MIN_24GHZ_CHANNEL].chan_num
#define REG_MAX_24GHZ_CH_NUM channel_map[MAX_24GHZ_CHANNEL].chan_num
#define REG_MIN_5GHZ_CH_NUM channel_map[MIN_5GHZ_CHANNEL].chan_num
#define REG_MAX_5GHZ_CH_NUM channel_map[MAX_5GHZ_CHANNEL].chan_num
#define REG_MIN_11P_CH_NUM channel_map[MIN_11P_CHANNEL].chan_num
#define REG_MAX_11P_CH_NUM channel_map[MAX_11P_CHANNEL].chan_num

#define REG_IS_24GHZ_CH(chan_num) \
	((chan_num >= REG_MIN_24GHZ_CH_NUM) &&	\
	 (chan_num <= REG_MAX_24GHZ_CH_NUM))

#define REG_IS_5GHZ_CH(chan_num) \
	((chan_num >= REG_MIN_5GHZ_CH_NUM) &&	\
	 (chan_num <= REG_MAX_5GHZ_CH_NUM))

#define REG_IS_11P_CH(chan_num) \
	((chan_num >= REG_MIN_11P_CH_NUM) &&	\
	 (chan_num <= REG_MAX_11P_CH_NUM))

#define REG_CH_NUM(ch_enum) channel_map[ch_enum].chan_num
#define REG_CH_TO_FREQ(ch_enum) channel_map[ch_enum].center_freq

#define REG_IS_CHANNEL_VALID_5G_SBS(curchan, newchan)	\
	(curchan > newchan ?				\
	 REG_CH_TO_FREQ(reg_get_chan_enum(curchan))	\
	 - REG_CH_TO_FREQ(reg_get_chan_enum(newchan))	\
	 > REG_SBS_SEPARATION_THRESHOLD :		\
	 REG_CH_TO_FREQ(reg_get_chan_enum(newchan))	\
	 - REG_CH_TO_FREQ(reg_get_chan_enum(curchan))	\
	 > REG_SBS_SEPARATION_THRESHOLD)


extern const struct chan_map channel_map[NUM_CHANNELS];

enum channel_enum reg_get_chan_enum(uint32_t chan_num);

QDF_STATUS reg_get_channel_list_with_power(struct wlan_objmgr_pdev *pdev,
					   struct channel_power *ch_list,
					   uint8_t *num_chan);

QDF_STATUS reg_read_default_country(struct wlan_objmgr_psoc *psoc,
				    uint8_t *country);

enum channel_state reg_get_channel_state(struct wlan_objmgr_pdev *pdev,
					 uint32_t ch);

enum channel_state reg_get_5g_bonded_channel_state(struct wlan_objmgr_pdev
						   *pdev,
						   uint8_t ch,
						   enum phy_ch_width bw);

enum channel_state reg_get_2g_bonded_channel_state(struct wlan_objmgr_pdev
						   *pdev,
						   uint8_t oper_ch,
						   uint8_t sec_ch,
						   enum phy_ch_width bw);

void reg_set_channel_params(struct wlan_objmgr_pdev *pdev,
			    uint8_t ch, uint8_t sec_ch_2g,
			    struct ch_params *ch_params);

void reg_get_dfs_region(struct wlan_objmgr_psoc *psoc,
			enum dfs_reg *dfs_reg);

uint32_t reg_get_channel_reg_power(struct wlan_objmgr_pdev *pdev,
				   uint32_t chan_num);

uint32_t reg_get_channel_freq(struct wlan_objmgr_pdev *pdev,
			      uint32_t chan_num);

QDF_STATUS reg_read_default_country(struct wlan_objmgr_psoc *psoc,
				    uint8_t *country);

void reg_set_default_country(struct wlan_objmgr_psoc *psoc, uint8_t *country);

uint16_t reg_get_bw_value(enum phy_ch_width bw);

void reg_set_dfs_region(struct wlan_objmgr_psoc *psoc,
			enum dfs_reg dfs_reg);

QDF_STATUS reg_get_domain_from_country_code(v_REGDOMAIN_t *reg_domain_ptr,
					    const uint8_t *country_alpha2,
					    enum country_src source);

enum band_info reg_chan_to_band(uint32_t chan_num);

uint16_t reg_dmn_get_chanwidth_from_opclass(uint8_t *country,
					    uint8_t channel,
					    uint8_t opclass);

uint16_t reg_dmn_get_opclass_from_channel(uint8_t *country,
					  uint8_t channel,
					  uint8_t offset);

uint16_t reg_dmn_set_curr_opclasses(uint8_t num_classes, uint8_t *class);

uint16_t reg_dmn_get_curr_opclasses(uint8_t *num_classes, uint8_t *class);


QDF_STATUS reg_process_master_chan_list(struct cur_regulatory_info *reg_info);

QDF_STATUS wlan_regulatory_psoc_obj_created_notification(
					    struct wlan_objmgr_psoc *psoc,
					    void *arg_list);

QDF_STATUS  wlan_regulatory_psoc_obj_destroyed_notification(
					    struct wlan_objmgr_psoc *psoc,
					    void *arg_list);

QDF_STATUS wlan_regulatory_pdev_obj_created_notification(
					    struct wlan_objmgr_pdev *pdev,
					    void *arg_list);

QDF_STATUS  wlan_regulatory_pdev_obj_destroyed_notification(
					    struct wlan_objmgr_pdev *pdev,
					    void *arg_list);

static inline struct wlan_lmac_if_reg_tx_ops *
get_reg_psoc_tx_ops(struct wlan_objmgr_psoc *psoc)
{
	return &((psoc->soc_cb.tx_ops.reg_ops));
}

QDF_STATUS reg_get_current_chan_list(struct wlan_objmgr_pdev *pdev,
				     struct regulatory_channel
				     *chan_list);

void reg_update_nol_ch(struct wlan_objmgr_pdev *pdev, uint8_t *ch_list,
		       uint8_t num_ch, bool nol_ch);

/**
 * reg_is_dfs_ch () - Checks the channel state for DFS
 * @chan: channel
 * @pdev: pdev ptr
 *
 * Return: true or false
 */
bool reg_is_dfs_ch(struct wlan_objmgr_pdev *pdev, uint32_t chan);

bool reg_is_passive_or_disable_ch(struct wlan_objmgr_pdev *pdev,
				  uint32_t chan);

bool reg_is_disable_ch(struct wlan_objmgr_pdev *pdev, uint32_t chan);

uint32_t reg_freq_to_chan(struct wlan_objmgr_pdev *pdev, uint32_t freq);

uint32_t reg_chan_to_freq(struct wlan_objmgr_pdev *pdev, uint32_t chan_num);

/**
 * reg_set_config_vars () - set configration variables
 * @psoc: psoc ptr
 * @config_vars: configuration struct
 *
 * Return: QDF_STATUS
 */
QDF_STATUS reg_set_config_vars(struct wlan_objmgr_psoc *psoc,
			       struct reg_config_vars config_vars);

bool reg_is_regdb_offloaded(struct wlan_objmgr_psoc *psoc);

void reg_program_mas_chan_list(struct wlan_objmgr_psoc *psoc,
			       struct regulatory_channel *reg_channels,
			       uint8_t *alpha2,
			       enum dfs_reg dfs_region);
#endif
