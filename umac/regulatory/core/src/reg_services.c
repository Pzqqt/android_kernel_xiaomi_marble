/*
 * Copyright (c) 2014-2017 The Linux Foundation. All rights reserved.
 *
 * Previously licensed under the ISC license by Qualcomm Atheros, Inc.
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

/*
 * This file was originally distributed by Qualcomm Atheros, Inc.
 * under proprietary terms before Copyright ownership was assigned
 * to the Linux Foundation.
 */

/**
 * DOC: reg_services.c
 * This file defines regulatory component service functions
 */

#include "qdf_status.h"
#include "qdf_types.h"
#include "wlan_objmgr_cmn.h"
#include "wlan_objmgr_global_obj.h"
#include "wlan_objmgr_psoc_obj.h"
#include "wlan_objmgr_pdev_obj.h"
#include "scheduler_api.h"
#include "reg_db.h"
#include "reg_services.h"
#include "reg_priv.h"

const struct bonded_channel bonded_chan_40mhz_list[] = {
	{36, 40},
	{44, 48},
	{52, 56},
	{60, 64},
	{100, 104},
	{108, 112},
	{116, 120},
	{124, 128},
	{132, 136},
	{140, 144},
	{149, 153},
	{157, 161}
};

const struct bonded_channel bonded_chan_80mhz_list[] = {
	{36, 48},
	{52, 64},
	{100, 112},
	{116, 128},
	{132, 144},
	{149, 161}
};

const struct bonded_channel bonded_chan_160mhz_list[] = {
	{36, 64},
	{100, 128}
};

const enum phy_ch_width get_next_lower_bw[] = {
	[CH_WIDTH_80P80MHZ] = CH_WIDTH_160MHZ,
	[CH_WIDTH_160MHZ] = CH_WIDTH_80MHZ,
	[CH_WIDTH_80MHZ] = CH_WIDTH_40MHZ,
	[CH_WIDTH_40MHZ] = CH_WIDTH_20MHZ,
	[CH_WIDTH_20MHZ] = CH_WIDTH_10MHZ,
	[CH_WIDTH_10MHZ] = CH_WIDTH_5MHZ,
	[CH_WIDTH_5MHZ] = CH_WIDTH_INVALID
};

const struct chan_map channel_map[NUM_CHANNELS] = {
	[CHAN_ENUM_1] = {2412, 1},
	[CHAN_ENUM_2] = {2417, 2},
	[CHAN_ENUM_3] = {2422, 3},
	[CHAN_ENUM_4] = {2427, 4},
	[CHAN_ENUM_5] = {2432, 5},
	[CHAN_ENUM_6] = {2437, 6},
	[CHAN_ENUM_7] = {2442, 7},
	[CHAN_ENUM_8] = {2447, 8},
	[CHAN_ENUM_9] = {2452, 9},
	[CHAN_ENUM_10] = {2457, 10},
	[CHAN_ENUM_11] = {2462, 11},
	[CHAN_ENUM_12] = {2467, 12},
	[CHAN_ENUM_13] = {2472, 13},
	[CHAN_ENUM_14] = {2484, 14},

	[CHAN_ENUM_36] = {5180, 36},
	[CHAN_ENUM_40] = {5200, 40},
	[CHAN_ENUM_44] = {5220, 44},
	[CHAN_ENUM_48] = {5240, 48},
	[CHAN_ENUM_52] = {5260, 52},
	[CHAN_ENUM_56] = {5280, 56},
	[CHAN_ENUM_60] = {5300, 60},
	[CHAN_ENUM_64] = {5320, 64},

	[CHAN_ENUM_100] = {5500, 100},
	[CHAN_ENUM_104] = {5520, 104},
	[CHAN_ENUM_108] = {5540, 108},
	[CHAN_ENUM_112] = {5560, 112},
	[CHAN_ENUM_116] = {5580, 116},
	[CHAN_ENUM_120] = {5600, 120},
	[CHAN_ENUM_124] = {5620, 124},
	[CHAN_ENUM_128] = {5640, 128},
	[CHAN_ENUM_132] = {5660, 132},
	[CHAN_ENUM_136] = {5680, 136},
	[CHAN_ENUM_140] = {5700, 140},
	[CHAN_ENUM_144] = {5720, 144},

	[CHAN_ENUM_149] = {5745, 149},
	[CHAN_ENUM_153] = {5765, 153},
	[CHAN_ENUM_157] = {5785, 157},
	[CHAN_ENUM_161] = {5805, 161},
	[CHAN_ENUM_165] = {5825, 165},

	[CHAN_ENUM_183] = {4915, 183},
	[CHAN_ENUM_184] = {4920, 184},
	[CHAN_ENUM_185] = {4925, 185},
	[CHAN_ENUM_187] = {4935, 187},
	[CHAN_ENUM_188] = {4940, 188},
	[CHAN_ENUM_189] = {4945, 189},
	[CHAN_ENUM_192] = {4960, 192},
	[CHAN_ENUM_196] = {4980, 196},
};

/**
 * reg_get_channel_list_with_power() - Provides the channel list with power
 * @ch_list: pointer to the channel list.
 *
 * Return: QDF_STATUS
 */
QDF_STATUS reg_get_channel_list_with_power(struct wlan_objmgr_psoc *psoc,
					   struct channel_power *ch_list,
					   uint8_t *num_chan)
{
	int i, count;
	struct regulatory_channel *reg_channels;
	struct wlan_regulatory_psoc_priv_obj *soc_reg;

	if (!num_chan || !ch_list) {
		reg_err("chan_list or num_ch is NULL");
		return QDF_STATUS_E_FAILURE;
	}
	soc_reg = (struct wlan_regulatory_psoc_priv_obj *)
		wlan_objmgr_psoc_get_comp_private_obj(psoc,
				WLAN_UMAC_COMP_REGULATORY);

	if (NULL == soc_reg) {
		reg_err("soc_reg is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	/* set the current channel list */
	reg_channels = &soc_reg->current_ch_list[0];
	for (i = 0, count = 0; i < NUM_CHANNELS; i++) {
		if (reg_channels[i].state) {
			ch_list[count].chan_num =
				channel_map[i].chan_num;
			ch_list[count++].tx_power =
				reg_channels[i].tx_power;
		}
	}
	*num_chan = count;

	return QDF_STATUS_SUCCESS;
}
/**
 * reg_read_default_country() - Read the default country for the regdomain
 * @country: pointer to the country code.
 *
 * Return: QDF_STATUS
 */
void reg_read_default_country(struct wlan_objmgr_psoc *psoc, uint8_t *country)
{
	struct wlan_regulatory_psoc_priv_obj *soc_reg;

	soc_reg = (struct wlan_regulatory_psoc_priv_obj *)
		wlan_objmgr_psoc_get_comp_private_obj(psoc,
				WLAN_UMAC_COMP_REGULATORY);

	if (NULL == soc_reg) {
		reg_err("soc_reg is NULL");
		return;
	}

	if (NULL == country) {
		reg_err("country is NULL");
		return;
	}

	qdf_mem_copy(country, soc_reg->default_country,
			sizeof(soc_reg->default_country));
}

static enum channel_enum get_reg_ch_idx(uint32_t chan_num)
{
	uint32_t indx;

	for (indx = 0; indx < NUM_CHANNELS; indx++)
		if (channel_map[indx].chan_num == chan_num)
			return indx;

	QDF_TRACE(QDF_MODULE_ID_QDF, QDF_TRACE_LEVEL_ERROR,
		  "invalid channel number %d", chan_num);

	return INVALID_CHANNEL;
}
/**
 * reg_get_channel_state() - Get channel state from regulatory
 * @ch: channel number.
 *
 * Return: channel state
 */
enum channel_state reg_get_channel_state(struct wlan_objmgr_psoc *psoc,
		uint32_t ch)
{
	enum channel_enum ch_idx;
	struct wlan_regulatory_psoc_priv_obj *soc_reg;

	ch_idx = get_reg_ch_idx(ch);

	if (INVALID_CHANNEL == ch_idx)
		return CHANNEL_STATE_INVALID;

	soc_reg = (struct wlan_regulatory_psoc_priv_obj *)
		wlan_objmgr_psoc_get_comp_private_obj(psoc,
				WLAN_UMAC_COMP_REGULATORY);

	if (NULL == soc_reg) {
		reg_err("soc_reg is NULL");
		return CHANNEL_STATE_INVALID;
	}

	return soc_reg->current_ch_list[ch_idx].state;
}

/**
 * reg_get_5g_bonded_chan_array() - get ptr to bonded channel
 * @oper_ch: operating channel number
 * @bonded_chan_ar: bonded channel array
 * @bonded_chan_ptr_ptr: bonded channel ptr ptr
 *
 * Return: bonded channel state
 */
static enum channel_state reg_get_5g_bonded_chan_array(
		struct wlan_objmgr_psoc *psoc,
		uint32_t oper_chan,
		const struct bonded_channel bonded_chan_ar[],
		uint16_t array_size,
		const struct bonded_channel **bonded_chan_ptr_ptr)
{
	int i;
	uint8_t chan_num;
	const struct bonded_channel *bonded_chan_ptr = NULL;
	enum channel_state chan_state = CHANNEL_STATE_INVALID;
	enum channel_state temp_chan_state;

	for (i = 0; i < array_size; i++) {
		if ((oper_chan >= bonded_chan_ar[i].start_ch) &&
		    (oper_chan <= bonded_chan_ar[i].end_ch)) {
			bonded_chan_ptr =  &(bonded_chan_ar[i]);
			break;
		}
	}

	if (NULL == bonded_chan_ptr)
		return chan_state;

	*bonded_chan_ptr_ptr = bonded_chan_ptr;
	chan_num =  bonded_chan_ptr->start_ch;
	while (chan_num <= bonded_chan_ptr->end_ch) {
		temp_chan_state = reg_get_channel_state(psoc, chan_num);
		if (temp_chan_state < chan_state)
			chan_state = temp_chan_state;
		chan_num = chan_num + 4;
	}

	return chan_state;
}

/**
 * reg_get_5g_bonded_channel() - get the 5G bonded channel state
 * @chan_num: channel number
 * @ch_width: channel width
 * @bonded_chan_ptr_ptr: bonded channel ptr ptr
 *
 * Return: channel state
 */
static enum channel_state reg_get_5g_bonded_channel(
		struct wlan_objmgr_psoc *psoc, uint32_t chan_num,
		enum phy_ch_width ch_width,
		const struct bonded_channel **bonded_chan_ptr_ptr)
{
	if (CH_WIDTH_80P80MHZ == ch_width)
		return reg_get_5g_bonded_chan_array(psoc, chan_num,
				bonded_chan_80mhz_list,
				QDF_ARRAY_SIZE(bonded_chan_80mhz_list),
				bonded_chan_ptr_ptr);
	else if (CH_WIDTH_160MHZ == ch_width)
		return reg_get_5g_bonded_chan_array(psoc, chan_num,
				bonded_chan_160mhz_list,
				QDF_ARRAY_SIZE(bonded_chan_160mhz_list),
				bonded_chan_ptr_ptr);
	else if (CH_WIDTH_80MHZ == ch_width)
		return reg_get_5g_bonded_chan_array(psoc, chan_num,
				bonded_chan_80mhz_list,
				QDF_ARRAY_SIZE(bonded_chan_80mhz_list),
				bonded_chan_ptr_ptr);
	else if (CH_WIDTH_40MHZ == ch_width)
		return reg_get_5g_bonded_chan_array(psoc, chan_num,
				bonded_chan_40mhz_list,
				QDF_ARRAY_SIZE(bonded_chan_40mhz_list),
				bonded_chan_ptr_ptr);
	else
		return reg_get_channel_state(psoc, chan_num);
}
/**
 * reg_get_5g_bonded_channel_state() - Get channel state for 5G bonded channel
 * @ch: channel number.
 * @bw: channel band width
 *
 * Return: channel state
 */
enum channel_state reg_get_5g_bonded_channel_state(
		struct wlan_objmgr_psoc *psoc,
		uint8_t ch, enum phy_ch_width bw)
{
	enum channel_enum ch_indx;
	enum channel_state chan_state;
	struct regulatory_channel *reg_channels;
	struct wlan_regulatory_psoc_priv_obj *soc_reg;
	bool bw_enabled = false;
	const struct bonded_channel *bonded_chan_ptr = NULL;

	if (CH_WIDTH_80P80MHZ < bw) {
		reg_err("bw passed is not good");
		return CHANNEL_STATE_INVALID;
	}

	chan_state = reg_get_5g_bonded_channel(psoc, ch, bw, &bonded_chan_ptr);

	if ((CHANNEL_STATE_INVALID == chan_state) ||
			(CHANNEL_STATE_DISABLE == chan_state))
		return chan_state;

	soc_reg = (struct wlan_regulatory_psoc_priv_obj *)
		wlan_objmgr_psoc_get_comp_private_obj(psoc,
				WLAN_UMAC_COMP_REGULATORY);

	if (NULL == soc_reg) {
		reg_err("soc_reg is NULL");
		return CHANNEL_STATE_INVALID;
	}

	reg_channels = &soc_reg->current_ch_list[0];
	ch_indx = get_reg_ch_idx(ch);
	if (CH_WIDTH_5MHZ == bw)
		bw_enabled = true;
	else if (CH_WIDTH_10MHZ == bw)
		bw_enabled = !(reg_channels[ch_indx].chan_flags &
				REGULATORY_CHAN_NO_10MHZ);
	else if (CH_WIDTH_20MHZ == bw)
		bw_enabled = !(reg_channels[ch_indx].chan_flags &
				REGULATORY_CHAN_NO_20MHZ);
	else if (CH_WIDTH_40MHZ == bw)
		bw_enabled = !((reg_channels[ch_indx].chan_flags &
			REGULATORY_CHAN_NO_HT40) == REGULATORY_CHAN_NO_HT40);
	else if (CH_WIDTH_80MHZ == bw)
		bw_enabled = !(reg_channels[ch_indx].chan_flags &
				REGULATORY_CHAN_NO_80MHZ);
	else if (CH_WIDTH_160MHZ == bw)
		bw_enabled = !(reg_channels[ch_indx].chan_flags &
				REGULATORY_CHAN_NO_160MHZ);
	else if (CH_WIDTH_80P80MHZ == bw)
		bw_enabled = !(reg_channels[ch_indx].chan_flags &
				REGULATORY_CHAN_NO_80MHZ);

	if (bw_enabled)
		return chan_state;
	else
		return CHANNEL_STATE_DISABLE;
}

/**
 * reg_get_2g_bonded_channel_state() - Get channel state for 2G bonded channel
 * @ch: channel number.
 * @bw: channel band width
 *
 * Return: channel state
 */
enum channel_state reg_get_2g_bonded_channel_state(
		struct wlan_objmgr_psoc *psoc,
		uint8_t oper_ch, uint8_t sec_ch,
		enum phy_ch_width bw)
{
	enum channel_enum chan_idx;
	enum channel_state chan_state;
	struct regulatory_channel *reg_channels;
	struct wlan_regulatory_psoc_priv_obj *soc_reg;
	bool bw_enabled = false;
	enum channel_state chan_state2 = CHANNEL_STATE_INVALID;

	if (CH_WIDTH_40MHZ < bw)
		return CHANNEL_STATE_INVALID;

	if (CH_WIDTH_40MHZ == bw) {
		if ((sec_ch + 4 != oper_ch) &&
		    (oper_ch + 4 != sec_ch))
			return CHANNEL_STATE_INVALID;
		chan_state2 = reg_get_channel_state(psoc, sec_ch);
		if (CHANNEL_STATE_INVALID == chan_state2)
			return chan_state2;
	}

	soc_reg = (struct wlan_regulatory_psoc_priv_obj *)
		wlan_objmgr_psoc_get_comp_private_obj(psoc,
				WLAN_UMAC_COMP_REGULATORY);

	if (NULL == soc_reg) {
		reg_err("soc_reg is NULL");
		return CHANNEL_STATE_INVALID;
	}

	reg_channels = &soc_reg->current_ch_list[0];

	chan_state = reg_get_channel_state(psoc, oper_ch);
	if (chan_state2 < chan_state)
		chan_state = chan_state2;

	if ((CHANNEL_STATE_INVALID == chan_state) ||
	    (CHANNEL_STATE_DISABLE == chan_state))
		return chan_state;

	chan_idx = get_reg_ch_idx(oper_ch);
	if (CH_WIDTH_5MHZ == bw)
		bw_enabled = true;
	else if (CH_WIDTH_10MHZ == bw)
		bw_enabled = !(reg_channels[chan_idx].chan_flags &
			       REGULATORY_CHAN_NO_10MHZ);
	else if (CH_WIDTH_20MHZ == bw)
		bw_enabled = !(reg_channels[chan_idx].chan_flags &
			       REGULATORY_CHAN_NO_20MHZ);
	else if (CH_WIDTH_40MHZ == bw)
		bw_enabled = !((reg_channels[chan_idx].chan_flags &
				REGULATORY_CHAN_NO_HT40) ==
				REGULATORY_CHAN_NO_HT40);

	if (bw_enabled)
		return chan_state;
	else
		return CHANNEL_STATE_DISABLE;

	return CHANNEL_STATE_ENABLE;
}

static enum channel_state reg_combine_channel_states(
	enum channel_state chan_state1,
	enum channel_state chan_state2)
{
	if ((CHANNEL_STATE_INVALID == chan_state1) ||
	    (CHANNEL_STATE_INVALID == chan_state2))
		return CHANNEL_STATE_INVALID;
	else
		return min(chan_state1, chan_state2);
}
/**
 * reg_set_channel_params () - Sets channel parameteres for given bandwidth
 * @ch: channel number.
 * @ch_params: pointer to the channel parameters.
 *
 * Return: None
 */
void reg_set_channel_params(struct wlan_objmgr_psoc *psoc,
		uint8_t ch, struct ch_params *ch_params)
{
	/*
	 * Set channel parameters like center frequency for a bonded channel
	 * state. Also return the maximum bandwidth supported by the channel.
	 */
	enum phy_ch_width next_lower_bw;
	enum channel_state chan_state = CHANNEL_STATE_ENABLE;
	enum channel_state chan_state2 = CHANNEL_STATE_ENABLE;
	const struct bonded_channel *bonded_chan_ptr = NULL;
	const struct bonded_channel *bonded_chan_ptr2 = NULL;

	if (NULL == ch_params) {
		reg_err("ch_params is NULL");
		return;
	}

	if (CH_WIDTH_MAX <= ch_params->ch_width) {
		if (0 != ch_params->center_freq_seg1)
			ch_params->ch_width = CH_WIDTH_80P80MHZ;
		else
			ch_params->ch_width = CH_WIDTH_160MHZ;
	}
	next_lower_bw = ch_params->ch_width;
	while (ch_params->ch_width != CH_WIDTH_INVALID) {
		ch_params->ch_width = next_lower_bw;
		next_lower_bw = get_next_lower_bw[ch_params->ch_width];
		bonded_chan_ptr = NULL;
		bonded_chan_ptr2 = NULL;
		chan_state = reg_get_5g_bonded_channel(psoc, ch,
				ch_params->ch_width, &bonded_chan_ptr);

		chan_state = reg_get_5g_bonded_channel_state(psoc, ch,
				ch_params->ch_width);

		if (CH_WIDTH_80P80MHZ == ch_params->ch_width) {
			chan_state2 = reg_get_5g_bonded_channel_state(psoc,
				ch_params->center_freq_seg1 - 2,
				CH_WIDTH_80MHZ);

			chan_state = reg_combine_channel_states(chan_state,
					chan_state2);
		}

		if ((CHANNEL_STATE_ENABLE != chan_state) &&
		    (CHANNEL_STATE_DFS != chan_state))
			continue;
		if (CH_WIDTH_20MHZ >= ch_params->ch_width) {
			ch_params->sec_ch_offset = NO_SEC_CH;
			ch_params->center_freq_seg0 = ch;
			break;
		} else if (CH_WIDTH_40MHZ <= ch_params->ch_width) {
			reg_get_5g_bonded_chan_array(psoc, ch,
					bonded_chan_40mhz_list,
					QDF_ARRAY_SIZE(bonded_chan_40mhz_list),
					&bonded_chan_ptr2);
			if (!bonded_chan_ptr || !bonded_chan_ptr2)
				continue;
			if (ch == bonded_chan_ptr2->start_ch)
				ch_params->sec_ch_offset = LOW_PRIMARY_CH;
			else
				ch_params->sec_ch_offset = HIGH_PRIMARY_CH;

			ch_params->center_freq_seg0 =
				(bonded_chan_ptr->start_ch +
				 bonded_chan_ptr->end_ch)/2;
			break;
		}
	}
	if (CH_WIDTH_160MHZ == ch_params->ch_width) {
		ch_params->center_freq_seg1 = ch_params->center_freq_seg0;
		chan_state = reg_get_5g_bonded_channel(psoc, ch,
				CH_WIDTH_80MHZ, &bonded_chan_ptr);
		if (bonded_chan_ptr)
			ch_params->center_freq_seg0 =
				(bonded_chan_ptr->start_ch +
				 bonded_chan_ptr->end_ch)/2;
	}
	reg_info("ch %d ch_wd %d freq0 %d freq1 %d", ch,
			ch_params->ch_width, ch_params->center_freq_seg0,
			ch_params->center_freq_seg1);
}

/**
 * reg_get_dfs_region () - Get the current dfs region
 * @dfs_reg: pointer to dfs region
 *
 * Return: None
 */
void reg_get_dfs_region(struct wlan_objmgr_psoc *psoc,
		enum dfs_region *dfs_reg)
{
	/*
	 * Get the current dfs region
	 */
}

/**
 * reg_is_dfs_ch () - Checks the channel state for DFS
 * @ch: channel
 *
 * Return: true or false
 */
bool reg_is_dfs_ch(struct wlan_objmgr_psoc *psoc, uint8_t ch)
{
	enum channel_state ch_state;

	ch_state = reg_get_channel_state(psoc, ch);

	return ch_state == CHANNEL_STATE_DFS;
}
