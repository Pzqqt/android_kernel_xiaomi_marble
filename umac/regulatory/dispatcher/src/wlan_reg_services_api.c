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
 * @file wlan_reg_services_api.c
 * @brief contains regulatory service functions
 */

#include "qdf_types.h"
#include "qdf_status.h"
#include "wlan_objmgr_cmn.h"
#include "wlan_objmgr_global_obj.h"
#include "wlan_objmgr_psoc_obj.h"
#include "wlan_objmgr_pdev_obj.h"
#include "scheduler_api.h"
#include "reg_db.h"
#include "reg_services.h"
#include "wlan_reg_services_api.h"

/**
 * wlan_reg_get_channel_list_with_power() - Provide the channel list with power
 * @ch_list: pointer to the channel list.
 *
 * Return: QDF_STATUS
 */
QDF_STATUS wlan_reg_get_channel_list_with_power(struct wlan_objmgr_psoc *psoc,
						struct channel_power *ch_list,
						uint8_t *num_chan)
{
	/*
	 * Update the channel list with channel information with power.
	 */
	return reg_get_channel_list_with_power(psoc, ch_list, num_chan);
}

/**
 * wlan_reg_read_default_country() - Read the default country for the regdomain
 * @country: pointer to the country code.
 *
 * Return: None
 */
void wlan_reg_read_default_country(struct wlan_objmgr_psoc *psoc,
		uint8_t *country)
{
	/*
	 * Get the default country information
	 */
	reg_read_default_country(psoc, country);
}

/**
 * wlan_reg_get_channel_state() - Get channel state from regulatory
 * @ch: channel number.
 *
 * Return: channel state
 */
enum channel_state wlan_reg_get_channel_state(struct wlan_objmgr_psoc *psoc,
		uint32_t ch)
{
	/*
	 * Get channel state from regulatory
	 */
	return reg_get_channel_state(psoc, ch);
}

/**
 * wlan_reg_get_5g_bonded_channel_state() - Get 5G bonded channel state
 * @ch: channel number.
 * @bw: channel band width
 *
 * Return: channel state
 */
enum channel_state wlan_reg_get_5g_bonded_channel_state(
		struct wlan_objmgr_psoc *psoc, uint8_t ch,
		enum phy_ch_width bw)
{
	/*
	 * Get channel state from regulatory
	 */
	return reg_get_5g_bonded_channel_state(psoc, ch, bw);
}

/**
 * wlan_reg_get_2g_bonded_channel_state() - Get 2G bonded channel state
 * @ch: channel number.
 * @bw: channel band width
 *
 * Return: channel state
 */
enum channel_state wlan_reg_get_2g_bonded_channel_state(
		struct wlan_objmgr_psoc *psoc, uint8_t ch, uint8_t sec_ch,
		enum phy_ch_width bw)
{
	/*
	 * Get channel state from regulatory
	 */
	return reg_get_2g_bonded_channel_state(psoc, ch, sec_ch, bw);
}

/**
 * wlan_reg_set_channel_params() - Sets channel parameteres for given bandwidth
 * @ch: channel number.
 * @ch_params: pointer to the channel parameters.
 *
 * Return: None
 */
void wlan_reg_set_channel_params(struct wlan_objmgr_psoc *psoc, uint8_t ch,
		struct ch_params *ch_params)
{
	/*
	 * Set channel parameters like center frequency for a bonded channel
	 * state. Also return the maximum bandwidth supported by the channel.
	 */
	reg_set_channel_params(psoc, ch, ch_params);
}

/**
 * wlan_reg_get_dfs_region () - Get the current dfs region
 * @dfs_reg: pointer to dfs region
 *
 * Return: None
 */
void wlan_reg_get_dfs_region(struct wlan_objmgr_psoc *psoc,
		enum dfs_region *dfs_reg)
{
	/*
	 * Get the current dfs region
	 */
	reg_get_dfs_region(psoc, dfs_reg);
}

/**
 * wlan_reg_is_dfs_ch () - Checks the channel state for DFS
 * @ch: channel
 *
 * Return: true or false
 */
bool wlan_reg_is_dfs_ch(struct wlan_objmgr_psoc *psoc, uint8_t ch)
{
	/*
	 * Get the current dfs region
	 */
	return reg_is_dfs_ch(psoc, ch);
}
