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
 * DOC: wlan_reg_services_api.h
 * This file provides prototypes of the routines needed for the
 * external components to utilize the services provided by the
 * regulatory component.
 */

#ifndef __WLAN_REG_SERVICES_API_H
#define __WLAN_REG_SERVICES_API_H

#include "../../core/src/reg_services.h"
#include <reg_services_public_struct.h>


#define WLAN_REG_MIN_24GHZ_CH_NUM REG_MIN_24GHZ_CH_NUM
#define WLAN_REG_MAX_24GHZ_CH_NUM REG_MAX_24GHZ_CH_NUM
#define WLAN_REG_MIN_5GHZ_CH_NUM REG_MIN_5GHZ_CH_NUM
#define WLAN_REG_MAX_5GHZ_CH_NUM REG_MAX_5GHZ_CH_NUM
#define WLAN_REG_MIN_11P_CH_NUM REG_MIN_11P_CH_NUM
#define WLAN_REG_MAX_11P_CH_NUM REG_MAX_11P_CH_NUM

#define WLAN_REG_IS_24GHZ_CH(chan) REG_IS_24GHZ_CH(chan)
#define WLAN_REG_IS_5GHZ_CH(chan) REG_IS_5GHZ_CH(chan)
#define WLAN_REG_IS_11P_CH(chan) REG_IS_11P_CH(chan)

#define WLAN_REG_CH_NUM(ch_enum) REG_CH_NUM(ch_enum)
#define WLAN_REG_CH_TO_FREQ(ch_enum) REG_CH_TO_FREQ(ch_enum)

#define WLAN_REG_IS_SAME_BAND_CHANNELS(chan_num1, chan_num2) \
	(chan_num1 && chan_num2 &&					\
	(WLAN_REG_IS_5GHZ_CH(chan_num1) == WLAN_REG_IS_5GHZ_CH(chan_num2)))


#define WLAN_REG_IS_CHANNEL_VALID_5G_SBS(curchan, newchan) \
	(curchan > newchan ?				   \
	 REG_CH_TO_FREQ(reg_get_chan_enum(curchan))	   \
	 - REG_CH_TO_FREQ(reg_get_chan_enum(newchan))	   \
	 > REG_SBS_SEPARATION_THRESHOLD :		   \
	 REG_CH_TO_FREQ(reg_get_chan_enum(newchan))	   \
	 - REG_CH_TO_FREQ(reg_get_chan_enum(curchan))	   \
	 > REG_SBS_SEPARATION_THRESHOLD)

#define WLAN_REG_INVALID_CHANNEL_ID
#define WLAN_REG_GET_24_END_CHAN_NUM 14

#define WLAN_REG_CHAN_TO_BAND(chan_num)  reg_chan_to_band(chan_num)

/**
 * wlan_reg_get_channel_list_with_power() - Provide the channel list with power
 * @ch_list: pointer to the channel list.
 *
 * Return: QDF_STATUS
 */
QDF_STATUS wlan_reg_get_channel_list_with_power(struct wlan_objmgr_pdev *pdev,
						struct channel_power *ch_list,
						uint8_t *num_chan);

/**
 * wlan_reg_read_default_country() - Read the default country for the regdomain
 * @country: pointer to the country code.
 *
 * Return: QDF_STATUS
 */
QDF_STATUS wlan_reg_read_default_country(struct wlan_objmgr_psoc *psoc,
				   uint8_t *country);
/**
 * wlan_reg_get_channel_state() - Get channel state from regulatory
 * @ch: channel number.
 *
 * Return: channel state
 */
enum channel_state wlan_reg_get_channel_state(struct wlan_objmgr_pdev *pdev,
					      uint32_t ch);

/**
 * wlan_reg_get_5g_bonded_channel_state() - Get 5G bonded channel state
 * @ch: channel number.
 * @bw: channel band width
 *
 * Return: channel state
 */
enum channel_state wlan_reg_get_5g_bonded_channel_state(
		struct wlan_objmgr_pdev *pdev, uint8_t ch,
		enum phy_ch_width bw);

/**
 * wlan_reg_get_2g_bonded_channel_state() - Get 2G bonded channel state
 * @ch: channel number.
 * @bw: channel band width
 *
 * Return: channel state
 */
enum channel_state wlan_reg_get_2g_bonded_channel_state(
		struct wlan_objmgr_pdev *pdev, uint8_t ch,
		uint8_t sec_ch, enum phy_ch_width bw);

/**
 * wlan_reg_set_channel_params () - Sets channel parameteres for given bandwidth
 * @ch: channel number.
 * @ch_params: pointer to the channel parameters.
 *
 * Return: None
 */
void wlan_reg_set_channel_params(struct wlan_objmgr_pdev *pdev, uint8_t ch,
				 uint8_t sec_ch_2g,
				 struct ch_params *ch_params);

/**
 * wlan_reg_get_dfs_region () - Get the current dfs region
 * @dfs_reg: pointer to dfs region
 *
 * Return: None
 */
void wlan_reg_get_dfs_region(struct wlan_objmgr_psoc *psoc,
			     enum dfs_reg *dfs_reg);

/**
 * wlan_reg_get_channel_reg_power() - Provide the channel regulatory power
 * @chan_num: chennal number
 *
 * Return: int
 */
uint32_t wlan_reg_get_channel_reg_power(struct wlan_objmgr_pdev *pdev,
					uint32_t chan_num);

/**
 * wlan_reg_get_channel_freq() - provide the channel center freq
 * @chan_num: chennal number
 *
 * Return: int
 */
uint32_t wlan_reg_get_channel_freq(struct wlan_objmgr_pdev *pdev,
				   uint32_t chan_num);

/**
 * wlan_reg_get_bonded_channel_state() - get bonded channel state
 * @pdev: pdev ptr
 * @ch: chennal number
 * @bw: chennal number
 * @sec_ch: secondary channel
 *
 * Return: enum channel_state
 */
enum channel_state wlan_reg_get_bonded_channel_state(
	struct wlan_objmgr_pdev *pdev, uint8_t ch,
	enum phy_ch_width bw, uint8_t sec_ch);

/**
 * wlan_reg_set_dfs_region() - set the dfs region
 * @psoc: psoc ptr
 * @dfs_reg: dfs region
 *
 * Return: void
 */
void wlan_reg_set_dfs_region(struct wlan_objmgr_psoc *psoc,
			     enum dfs_reg dfs_reg);

/**
 * wlan_reg_get_bw_value() - provide the channel center freq
 * @chan_num: chennal number
 *
 * Return: int
 */
uint16_t wlan_reg_get_bw_value(enum phy_ch_width bw);

/**
 * wlan_reg_get_domain_from_country_code() - provide the channel center freq
 * @reg_domain_ptr: regulatory domain ptr
 * @country_alpha2: country alpha2
 * @source: alpha2 source
 *
 * Return: int
 */
QDF_STATUS wlan_reg_get_domain_from_country_code(v_REGDOMAIN_t *reg_domain_ptr,
						 const uint8_t *country_alpha2,
						 enum country_src source);

/**
 * wlan_reg_dmn_get_opclass_from_channel() - provide the channel center freq
 * @country: country alpha2
 * @channel: channel number
 * @offset: offset
 *
 * Return: int
 */
uint16_t wlan_reg_dmn_get_opclass_from_channel(uint8_t *country,
					       uint8_t channel,
					       uint8_t offset);

/**
 * wlan_reg_dmn_get_chanwidth_from_opclass() - get channel width from
 *                                             operating class
 * @country: country alpha2
 * @channel: channel number
 * @opclass: operating class
 *
 * Return: int
 */
uint16_t wlan_reg_dmn_get_chanwidth_from_opclass(uint8_t *country,
						 uint8_t channel,
						 uint8_t opclass);
/**
 * wlan_reg_dmn_set_curr_opclasses() - set operating class
 * @num_classes: number of classes
 * @class: operating class
 *
 * Return: int
 */
uint16_t wlan_reg_dmn_set_curr_opclasses(uint8_t num_classes,
					 uint8_t *class);

/**
 * wlan_reg_dmn_get_curr_opclasses() - get current oper classes
 * @num_classes: number of classes
 * @class: operating class
 *
 * Return: int
 */
uint16_t wlan_reg_dmn_get_curr_opclasses(uint8_t *num_classes,
					 uint8_t *class);


/**
 * wlan_regulatory_init() - init regulatory component
 *
 * Return: Success or Failure
 */
QDF_STATUS wlan_regulatory_init(void);

/**
 * wlan_regulatory_deinit() - deinit regulatory component
 *
 * Return: Success or Failure
 */
QDF_STATUS wlan_regulatory_deinit(void);

/**
 * regulatory_psoc_open() - open regulatory component
 *
 * Return: Success or Failure
 */
QDF_STATUS regulatory_psoc_open(struct wlan_objmgr_psoc *psoc);


/**
 * regulatory_psoc_close() - close regulatory component
 *
 * Return: Success or Failure
 */
QDF_STATUS regulatory_psoc_close(struct wlan_objmgr_psoc *psoc);

/**
 * wlan_reg_update_nol_ch () - set nol channel
 * @pdev: pdev ptr
 * @ch_list: channel list to be returned
 * @num_ch: number of channels
 * @nol_ch: nol flag
 *
 * Return: void
 */
void wlan_reg_update_nol_ch(struct wlan_objmgr_pdev *pdev,
			    uint8_t *ch_list,
			    uint8_t num_ch,
			    bool nol_ch);

/**
 * wlan_reg_is_dfs_ch () - Checks the channel state for DFS
 * @pdev: pdev ptr
 * @chan: channel
 *
 * Return: true or false
 */
bool wlan_reg_is_dfs_ch(struct wlan_objmgr_pdev *pdev, uint32_t chan);

/**
 * wlan_reg_is_passive_or_disable_ch () - Checks chan state for passive
 * and disabled
 * @pdev: pdev ptr
 * @chan: channel
 *
 * Return: true or false
 */
bool wlan_reg_is_passive_or_disable_ch(struct wlan_objmgr_pdev *pdev,
				       uint32_t chan);

/**
 * wlan_reg_is_disable_ch () - Checks chan state for disabled
 * @pdev: pdev ptr
 * @chan: channel
 *
 * Return: true or false
 */
bool wlan_reg_is_disable_ch(struct wlan_objmgr_pdev *pdev, uint32_t chan);

/**
 * wlan_reg_freq_to_chan () - convert channel freq to channel number
 * @freq: frequency
 *
 * Return: true or false
 */
uint32_t wlan_reg_freq_to_chan(struct wlan_objmgr_pdev *pdev,
			       uint32_t freq);

/**
 * wlan_reg_chan_to_freq () - convert channel number to frequency
 * @chan: channel number
 *
 * Return: true or false
 */
uint32_t wlan_reg_chan_to_freq(struct wlan_objmgr_pdev *pdev,
			       uint32_t chan);

/**
 * wlan_reg_set_country() - Set the current regulatory country
 * @pdev: The physical dev to set current country for
 * @country: The country information to configure
 *
 * Return: QDF_STATUS
 */
QDF_STATUS wlan_reg_set_country(struct wlan_objmgr_pdev *pdev,
				       uint8_t *country);

/**
 * wlan_reg_register_chan_change_callback () - add chan change cbk
 * @psoc: channel number
 * @cbk: callback
 * @arg: argument
 *
 * Return: true or false
 */
void wlan_reg_register_chan_change_callback(struct wlan_objmgr_psoc *psoc,
					    reg_chan_change_callback cbk,
					    void *arg);

/**
 * wlan_reg_unregister_chan_change_callback () - remove chan change cbk
 * @psoc: channel number
 * @cbk: callback
 *
 * Return: true or false
 */
void wlan_reg_unregister_chan_change_callback(struct wlan_objmgr_psoc *psoc,
					      reg_chan_change_callback cbk);

/**
 * wlan_reg_get_chip_mode() - get supported chip mode
 * @pdev: pdev pointer
 * @chip_mode: chip mode
 *
 * Return: QDF STATUS
 */
QDF_STATUS wlan_reg_get_chip_mode(struct wlan_objmgr_pdev *pdev,
		uint32_t *chip_mode);

/**
 * wlan_reg_get_freq_range() - Get 2GHz and 5GHz frequency range
 * @pdev: pdev pointer
 * @low_2g: low 2GHz frequency range
 * @high_2g: high 2GHz frequency range
 * @low_5g: low 5GHz frequency range
 * @high_5g: high 5GHz frequency range
 *
 * Return: QDF status
 */
QDF_STATUS wlan_reg_get_freq_range(struct wlan_objmgr_pdev *pdev,
		uint32_t *low_2g,
		uint32_t *high_2g,
		uint32_t *low_5g,
		uint32_t *high_5g);
#endif
