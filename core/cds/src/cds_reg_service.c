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

/*============================================================================
   FILE:         cds_reg_service.c
   OVERVIEW:     This source file contains definitions for CDS regulatory APIs
   DEPENDENCIES: None
   ============================================================================*/

#include <net/cfg80211.h>
#include "qdf_types.h"
#include "qdf_trace.h"
#include "cds_api.h"
#include "cds_reg_service.h"
#include "cds_regdomain.h"
#include "cds_ieee80211_common_i.h"
#include "sme_api.h"
#include "wlan_hdd_main.h"

/**
 * struct bonded_chan
 * @start_ch: start channel
 * @end_ch: end channel
 */
struct bonded_chan {
	uint16_t start_ch;
	uint16_t end_ch;
};

static const struct bonded_chan bonded_chan_40mhz_array[] = {
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

static const struct bonded_chan bonded_chan_80mhz_array[] = {
	{36, 48},
	{52, 64},
	{100, 112},
	{116, 128},
	{132, 144},
	{149, 161}
};

static const struct bonded_chan bonded_chan_160mhz_array[] = {
	{36, 64},
	{100, 128}
};

static const enum phy_ch_width next_lower_bw[] = {
	[CH_WIDTH_80P80MHZ] = CH_WIDTH_160MHZ,
	[CH_WIDTH_160MHZ] = CH_WIDTH_80MHZ,
	[CH_WIDTH_80MHZ] = CH_WIDTH_40MHZ,
	[CH_WIDTH_40MHZ] = CH_WIDTH_20MHZ,
	[CH_WIDTH_20MHZ] = CH_WIDTH_10MHZ,
	[CH_WIDTH_10MHZ] = CH_WIDTH_5MHZ,
	[CH_WIDTH_5MHZ] = CH_WIDTH_INVALID
};

struct regulatory_channel reg_channels[NUM_CHANNELS];
static uint8_t default_country[CDS_COUNTRY_CODE_LEN + 1];

/**
 * cds_get_channel_list_with_power() - retrieve channel list with power
 * @base_channels: base channels
 * @num_base_channels: number of base channels
 *
 * Return: QDF_STATUS_SUCCESS
 */
QDF_STATUS cds_get_channel_list_with_power(struct channel_power
					   *base_channels,
					   uint8_t *num_base_channels)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	int i, count;

	if (base_channels && num_base_channels) {
		count = 0;
		for (i = 0; i < NUM_CHANNELS; i++) {
			if (reg_channels[i].state) {
				base_channels[count].chan_num =
					channel_map[i].chan_num;
				base_channels[count++].tx_power =
					reg_channels[i].tx_power;
			}
		}
		*num_base_channels = count;
	}

	return status;
}

/**
 * cds_read_default_country() - set the default country
 * @def_ctry: default country
 *
 * Return: QDF_STATUS
 */
QDF_STATUS cds_read_default_country(uint8_t *def_ctry)
{
	memcpy(def_ctry,
	       default_country,
	       CDS_COUNTRY_CODE_LEN + 1);

	QDF_TRACE(QDF_MODULE_ID_QDF, QDF_TRACE_LEVEL_INFO,
		  "default country is %c%c\n",
		  def_ctry[0],
		  def_ctry[1]);

	return QDF_STATUS_SUCCESS;
}

/**
 * cds_get_channel_enum() - get the channel enumeration
 * @chan_num: channel number
 *
 * Return: enum for the channel
 */
enum channel_enum cds_get_channel_enum(uint32_t chan_num)
{
	uint32_t loop;

	for (loop = 0; loop < NUM_CHANNELS; loop++)
		if (channel_map[loop].chan_num == chan_num)
			return loop;

	QDF_TRACE(QDF_MODULE_ID_QDF, QDF_TRACE_LEVEL_ERROR,
		  "invalid channel number %d", chan_num);

	return INVALID_CHANNEL;
}


/**
 * cds_get_channel_state() - get the channel state
 * @chan_num: channel number
 *
 * Return: channel state
 */
enum channel_state cds_get_channel_state(uint32_t chan_num)
{
	enum channel_enum chan_enum;

	chan_enum = cds_get_channel_enum(chan_num);
	if (INVALID_CHANNEL == chan_enum)
		return CHANNEL_STATE_INVALID;
	else
		return reg_channels[chan_enum].state;
}

int8_t cds_get_channel_reg_power(uint32_t chan_num)
{
	enum channel_enum chan_enum;

	chan_enum = cds_get_channel_enum(chan_num);
	if (chan_enum == INVALID_CHANNEL)
		return CHANNEL_STATE_INVALID;
	else
		return reg_channels[chan_enum].tx_power;
}

uint32_t cds_get_channel_flags(uint32_t chan_num)
{
	enum channel_enum chan_enum;

	chan_enum = cds_get_channel_enum(chan_num);
	if (chan_enum == INVALID_CHANNEL)
		return CHANNEL_STATE_INVALID;
	else
		return reg_channels[chan_enum].chan_flags;
}

uint32_t cds_get_vendor_reg_flags(uint32_t chan, uint16_t bandwidth,
				bool is_ht_enabled, bool is_vht_enabled,
				uint8_t sub_20_channel_width)
{
	uint32_t flags = 0;
	enum channel_state state;
	struct ch_params ch_params;

	state = cds_get_channel_state(chan);
	if (state == CHANNEL_STATE_INVALID)
		return flags;
	if (state == CHANNEL_STATE_DFS) {
		flags |= IEEE80211_CHAN_PASSIVE;
		flags |= IEEE80211_CHAN_DFS;
	}
	if (state == CHANNEL_STATE_DISABLE)
		flags |= IEEE80211_CHAN_BLOCKED;

	if (CDS_IS_CHANNEL_24GHZ(chan)) {
		if ((bandwidth == CH_WIDTH_80P80MHZ) ||
			(bandwidth == CH_WIDTH_160MHZ) ||
			(bandwidth == CH_WIDTH_80MHZ)) {
			bandwidth = CH_WIDTH_40MHZ;
		}
	}

	switch (bandwidth) {

	case CH_WIDTH_80P80MHZ:
		if (cds_get_5g_bonded_channel_state(chan,
				  bandwidth) != CHANNEL_STATE_INVALID) {
			if (is_vht_enabled)
				flags |= IEEE80211_CHAN_VHT80_80;
		}
		bandwidth = CH_WIDTH_160MHZ;
		/* FALLTHROUGH */
	case CH_WIDTH_160MHZ:
		if (cds_get_5g_bonded_channel_state(chan,
				  bandwidth) != CHANNEL_STATE_INVALID) {
			if (is_vht_enabled)
				flags |= IEEE80211_CHAN_VHT160;
		}
		bandwidth = CH_WIDTH_80MHZ;
		/* FALLTHROUGH */
	case CH_WIDTH_80MHZ:
		if (cds_get_5g_bonded_channel_state(chan,
				  bandwidth) != CHANNEL_STATE_INVALID) {
			if (is_vht_enabled)
				flags |= IEEE80211_CHAN_VHT80;
		}
		bandwidth = CH_WIDTH_40MHZ;
		/* FALLTHROUGH */
	case CH_WIDTH_40MHZ:
		qdf_mem_zero(&ch_params, sizeof(ch_params));
		ch_params.ch_width = bandwidth;
		cds_set_channel_params(chan, 0, &ch_params);

		if (cds_get_bonded_channel_state(chan, bandwidth,
		    ch_params.sec_ch_offset) != CHANNEL_STATE_INVALID) {
			if (ch_params.sec_ch_offset ==
			    PHY_DOUBLE_CHANNEL_LOW_PRIMARY) {
				flags |= IEEE80211_CHAN_HT40PLUS;
				if (is_vht_enabled)
					flags |= IEEE80211_CHAN_VHT40PLUS;
			} else if (ch_params.sec_ch_offset ==
				   PHY_DOUBLE_CHANNEL_HIGH_PRIMARY) {
				flags |= IEEE80211_CHAN_HT40MINUS;
				if (is_vht_enabled)
					flags |= IEEE80211_CHAN_VHT40MINUS;
			}
		}

		bandwidth = CH_WIDTH_20MHZ;
		/* FALLTHROUGH */
	case CH_WIDTH_20MHZ:
		if (is_vht_enabled)
			flags |= IEEE80211_CHAN_VHT20;
		if (is_ht_enabled)
			flags |= IEEE80211_CHAN_HT20;
		bandwidth = CH_WIDTH_10MHZ;
		/* FALLTHROUGH */
	case CH_WIDTH_10MHZ:
		if ((cds_get_bonded_channel_state(chan,
			bandwidth, 0) != CHANNEL_STATE_INVALID) &&
			(sub_20_channel_width ==
				WLAN_SUB_20_CH_WIDTH_10))
			flags |= IEEE80211_CHAN_HALF;
		/* FALLTHROUGH */
	case CH_WIDTH_5MHZ:
		if ((cds_get_bonded_channel_state(chan,
			bandwidth, 0) != CHANNEL_STATE_INVALID) &&
			(sub_20_channel_width ==
				WLAN_SUB_20_CH_WIDTH_5))
			flags |= IEEE80211_CHAN_QUARTER;
		break;
	default:
	QDF_TRACE(QDF_MODULE_ID_QDF, QDF_TRACE_LEVEL_INFO,
		"invalid channel width value %d", bandwidth);
	}

	return flags;
}

uint32_t cds_get_channel_freq(uint32_t chan_num)
{
	enum channel_enum chan_enum;

	chan_enum = cds_get_channel_enum(chan_num);
	if (chan_enum == INVALID_CHANNEL)
		return CHANNEL_STATE_INVALID;
	else
		return channel_map[chan_enum].center_freq;
}

/**
 * cds_search_5g_bonded_chan_array() - get ptr to bonded channel
 * @oper_ch: operating channel number
 * @bonded_chan_ar: bonded channel array
 * @bonded_chan_ptr_ptr: bonded channel ptr ptr
 *
 * Return: bonded channel state
 */
static enum channel_state cds_search_5g_bonded_chan_array(uint32_t oper_chan,
							const struct bonded_chan
							bonded_chan_ar[],
							uint16_t array_size,
							const struct bonded_chan
							**bonded_chan_ptr_ptr)
{
	int i;
	uint8_t chan_num;
	const struct bonded_chan *bonded_chan_ptr = NULL;
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
		temp_chan_state = cds_get_channel_state(chan_num);
		if (temp_chan_state < chan_state)
			chan_state = temp_chan_state;
		chan_num = chan_num + 4;
	}

	return chan_state;
}

/**
 * cds_search_5g_bonded_channel() - get the 5G bonded channel state
 * @chan_num: channel number
 * @ch_width: channel width
 * @bonded_chan_ptr_ptr: bonded channel ptr ptr
 *
 * Return: channel state
 */
static enum channel_state cds_search_5g_bonded_channel(uint32_t chan_num,
						enum phy_ch_width ch_width,
						const struct bonded_chan
						**bonded_chan_ptr_ptr)
{

	if (CH_WIDTH_80P80MHZ == ch_width)
		return cds_search_5g_bonded_chan_array(chan_num,
						       bonded_chan_80mhz_array,
						       QDF_ARRAY_SIZE(bonded_chan_80mhz_array),
						       bonded_chan_ptr_ptr);
	else if (CH_WIDTH_160MHZ == ch_width)
		return cds_search_5g_bonded_chan_array(chan_num,
						       bonded_chan_160mhz_array,
						       QDF_ARRAY_SIZE(bonded_chan_160mhz_array),
						       bonded_chan_ptr_ptr);
	else if (CH_WIDTH_80MHZ == ch_width)
		return cds_search_5g_bonded_chan_array(chan_num,
						       bonded_chan_80mhz_array,
						       QDF_ARRAY_SIZE(bonded_chan_80mhz_array),
						       bonded_chan_ptr_ptr);
	else if (CH_WIDTH_40MHZ == ch_width)
		return cds_search_5g_bonded_chan_array(chan_num,
						       bonded_chan_40mhz_array,
						       QDF_ARRAY_SIZE(bonded_chan_40mhz_array),
						       bonded_chan_ptr_ptr);
	else
		return cds_get_channel_state(chan_num);
}

enum channel_state cds_get_bonded_channel_state(uint16_t oper_ch,
						enum phy_ch_width ch_width,
						uint16_t sec_ch)
{
	if (CDS_IS_CHANNEL_5GHZ(oper_ch))
		return cds_get_5g_bonded_channel_state(oper_ch,
							ch_width);
	else
		return cds_get_2g_bonded_channel_state(oper_ch,
							ch_width, sec_ch);
}

/**
 * cds_get_2g_bonded_channel_state() - get the 2G bonded channel state
 * @oper_ch: operating channel
 * @ch_width: channel width
 * @sec_ch: secondary channel
 *
 * Return: channel state
 */
enum channel_state cds_get_2g_bonded_channel_state(uint16_t oper_ch,
						  enum phy_ch_width ch_width,
						  uint16_t sec_ch)
{
	enum channel_enum chan_enum;
	enum channel_state chan_state;
	bool bw_enabled = false;
	enum channel_state chan_state2 = CHANNEL_STATE_INVALID;

	if (CH_WIDTH_40MHZ < ch_width)
		return CHANNEL_STATE_INVALID;

	if (CH_WIDTH_40MHZ == ch_width) {
		if ((sec_ch + 4 != oper_ch) &&
		    (oper_ch + 4 != sec_ch))
			return CHANNEL_STATE_INVALID;
		chan_state2 = cds_get_channel_state(sec_ch);
		if (CHANNEL_STATE_INVALID == chan_state2)
			return chan_state2;
	}

	chan_state = cds_get_channel_state(oper_ch);
	if (chan_state2 < chan_state)
		chan_state = chan_state2;

	if ((CHANNEL_STATE_INVALID == chan_state) ||
	    (CHANNEL_STATE_DISABLE == chan_state))
		return chan_state;

	chan_enum = cds_get_channel_enum(oper_ch);
	if (CH_WIDTH_5MHZ == ch_width)
		bw_enabled = true;
	else if (CH_WIDTH_10MHZ == ch_width)
		bw_enabled = !(reg_channels[chan_enum].chan_flags &
			       IEEE80211_CHAN_NO_10MHZ);
	else if (CH_WIDTH_20MHZ == ch_width)
		bw_enabled = !(reg_channels[chan_enum].chan_flags &
			       IEEE80211_CHAN_NO_20MHZ);
	else if (CH_WIDTH_40MHZ == ch_width)
		bw_enabled = !((reg_channels[chan_enum].chan_flags &
				IEEE80211_CHAN_NO_HT40) == IEEE80211_CHAN_NO_HT40);

	if (bw_enabled)
		return chan_state;
	else
		return CHANNEL_STATE_DISABLE;
}

/**
 * cds_get_5g_bonded_channel_state() - get the 5G bonded channel state
 * @chan_num: channel number
 * @ch_width: channel width
 *
 * Return: channel state
 */
enum channel_state cds_get_5g_bonded_channel_state(
	uint16_t chan_num,
	enum phy_ch_width ch_width)
{
	enum channel_enum chan_enum;
	enum channel_state chan_state;
	bool bw_enabled = false;
	const struct bonded_chan *bonded_chan_ptr = NULL;

	if (CH_WIDTH_80P80MHZ < ch_width)
		return CHANNEL_STATE_INVALID;

	chan_state = cds_search_5g_bonded_channel(chan_num, ch_width,
						  &bonded_chan_ptr);

	if ((CHANNEL_STATE_INVALID == chan_state) ||
	    (CHANNEL_STATE_DISABLE == chan_state))
		return chan_state;

	chan_enum = cds_get_channel_enum(chan_num);
	if (CH_WIDTH_5MHZ == ch_width)
		bw_enabled = true;
	else if (CH_WIDTH_10MHZ == ch_width)
		bw_enabled = !(reg_channels[chan_enum].chan_flags &
			       IEEE80211_CHAN_NO_10MHZ);
	else if (CH_WIDTH_20MHZ == ch_width)
		bw_enabled = !(reg_channels[chan_enum].chan_flags &
			       IEEE80211_CHAN_NO_20MHZ);
	else if (CH_WIDTH_40MHZ == ch_width)
		bw_enabled = !((reg_channels[chan_enum].chan_flags &
				IEEE80211_CHAN_NO_HT40) == IEEE80211_CHAN_NO_HT40);
	else if (CH_WIDTH_80MHZ == ch_width)
		bw_enabled = !(reg_channels[chan_enum].chan_flags &
			       IEEE80211_CHAN_NO_80MHZ);
	else if (CH_WIDTH_160MHZ == ch_width)
		bw_enabled = !(reg_channels[chan_enum].chan_flags &
			       IEEE80211_CHAN_NO_160MHZ);
	else if (CH_WIDTH_80P80MHZ == ch_width)
		bw_enabled = !(reg_channels[chan_enum].chan_flags &
			       IEEE80211_CHAN_NO_80MHZ);

	if (bw_enabled)
		return chan_state;
	else
		return CHANNEL_STATE_DISABLE;
}

/**
 * cds_combine_channel_states() - combine channel states
 * @chan_state1: channel state1
 * @chan_state2: channel state2
 *
 * Return: enum channel_state
 */
 static enum channel_state cds_combine_channel_states(
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
 * cds_set_5g_channel_params() - set the 5G bonded channel parameters
 * @oper_ch: opetrating channel
 * @ch_params: channel parameters
 *
 * Return: void
 */
static void cds_set_5g_channel_params(uint16_t oper_ch,
				      struct ch_params *ch_params)
{
	enum channel_state chan_state = CHANNEL_STATE_ENABLE;
	enum channel_state chan_state2 = CHANNEL_STATE_ENABLE;
	const struct bonded_chan *bonded_chan_ptr = NULL;
	const struct bonded_chan *bonded_chan_ptr2 = NULL;

	if (CH_WIDTH_MAX <= ch_params->ch_width) {
		if (0 != ch_params->center_freq_seg1)
			ch_params->ch_width = CH_WIDTH_80P80MHZ;
		else
			ch_params->ch_width = CH_WIDTH_160MHZ;
	}

	while (ch_params->ch_width != CH_WIDTH_INVALID) {
		bonded_chan_ptr = NULL;
		bonded_chan_ptr2 = NULL;
		chan_state = cds_search_5g_bonded_channel(oper_ch,
							  ch_params->ch_width,
							  &bonded_chan_ptr);

		chan_state = cds_get_5g_bonded_channel_state(oper_ch,
							  ch_params->ch_width);

		if (CH_WIDTH_80P80MHZ == ch_params->ch_width) {
			chan_state2 = cds_get_5g_bonded_channel_state(
				ch_params->center_freq_seg1 - 2,
				CH_WIDTH_80MHZ);

			chan_state = cds_combine_channel_states(chan_state,
								chan_state2);
		}

		if ((CHANNEL_STATE_ENABLE == chan_state) ||
		    (CHANNEL_STATE_DFS == chan_state)) {
			if (CH_WIDTH_20MHZ >= ch_params->ch_width) {
				ch_params->sec_ch_offset
					= PHY_SINGLE_CHANNEL_CENTERED;
				ch_params->center_freq_seg0 = oper_ch;
				break;
			} else if (CH_WIDTH_40MHZ <= ch_params->ch_width) {
				cds_search_5g_bonded_chan_array(oper_ch,
							bonded_chan_40mhz_array,
					QDF_ARRAY_SIZE(bonded_chan_40mhz_array),
							     &bonded_chan_ptr2);
				if (bonded_chan_ptr && bonded_chan_ptr2) {
					if (oper_ch ==
					    bonded_chan_ptr2->start_ch)
						ch_params->sec_ch_offset =
						PHY_DOUBLE_CHANNEL_LOW_PRIMARY;
					else
						ch_params->sec_ch_offset =
						PHY_DOUBLE_CHANNEL_HIGH_PRIMARY;

					ch_params->center_freq_seg0 =
						(bonded_chan_ptr->start_ch +
						 bonded_chan_ptr->end_ch)/2;
					break;
				}
			}
		}
		ch_params->ch_width = next_lower_bw[ch_params->ch_width];
	}
	if (CH_WIDTH_160MHZ == ch_params->ch_width) {
		ch_params->center_freq_seg1 = ch_params->center_freq_seg0;
		chan_state = cds_search_5g_bonded_channel(oper_ch,
							  CH_WIDTH_80MHZ,
							  &bonded_chan_ptr);
		if (bonded_chan_ptr)
			ch_params->center_freq_seg0 =
				(bonded_chan_ptr->start_ch +
				 bonded_chan_ptr->end_ch)/2;
	}
	QDF_TRACE(QDF_MODULE_ID_QDF, QDF_TRACE_LEVEL_DEBUG,
			"ch: %d ch_wd: %d freq0: %d freq1: %d", oper_ch,
			ch_params->ch_width, ch_params->center_freq_seg0,
			ch_params->center_freq_seg1);
}

/**
 * cds_set_2g_channel_params() - set the 2.4G bonded channel parameters
 * @oper_ch: operating channel
 * @ch_params: channel parameters
 * @sec_ch_2g: 2.4G secondary channel
 *
 * Return: void
 */
static void cds_set_2g_channel_params(uint16_t oper_ch,
			       struct ch_params *ch_params,
			       uint16_t sec_ch_2g)
{
	enum channel_state chan_state = CHANNEL_STATE_ENABLE;

	if (CH_WIDTH_MAX <= ch_params->ch_width)
		ch_params->ch_width = CH_WIDTH_40MHZ;

	if ((cds_bw_value(ch_params->ch_width) > 20) && !sec_ch_2g) {
		if (oper_ch >= 1 && oper_ch <= 5)
			sec_ch_2g = oper_ch + 4;
		else if (oper_ch >= 6 && oper_ch <= 13)
			sec_ch_2g = oper_ch - 4;
	}

	while (ch_params->ch_width != CH_WIDTH_INVALID) {
		chan_state = cds_get_2g_bonded_channel_state(oper_ch,
							    ch_params->ch_width,
							    sec_ch_2g);
		if (CHANNEL_STATE_ENABLE == chan_state) {
			if (CH_WIDTH_40MHZ == ch_params->ch_width) {
				if (oper_ch < sec_ch_2g)
					ch_params->sec_ch_offset =
						PHY_DOUBLE_CHANNEL_LOW_PRIMARY;
				else
					ch_params->sec_ch_offset =
						PHY_DOUBLE_CHANNEL_HIGH_PRIMARY;
				ch_params->center_freq_seg0 =
					(oper_ch + sec_ch_2g)/2;
			} else
				ch_params->sec_ch_offset =
					PHY_SINGLE_CHANNEL_CENTERED;
			break;
		}

		ch_params->ch_width = next_lower_bw[ch_params->ch_width];
	}
}

/**
 * cds_set_channel_params() - set the bonded channel parameters
 * @oper_ch: operating channel
 * @sec_ch_2g: 2.4G secondary channel
 * @ch_params: chanel parameters
 *
 * Return: void
 */
void cds_set_channel_params(uint16_t oper_ch, uint16_t sec_ch_2g,
			    struct ch_params *ch_params)
{
	if (CDS_IS_CHANNEL_5GHZ(oper_ch))
		cds_set_5g_channel_params(oper_ch, ch_params);
	else if  (CDS_IS_CHANNEL_24GHZ(oper_ch))
		cds_set_2g_channel_params(oper_ch, ch_params, sec_ch_2g);
}

/**
 * cds_get_reg_domain_from_country_code() - get the regulatory domain
 * @reg_domain_ptr: ptr to store regulatory domain
 *
 * Return: QDF_STATUS_SUCCESS on success
 *         QDF_STATUS_E_FAULT on error
 *         QDF_STATUS_E_EMPTY country table empty
 */
QDF_STATUS cds_get_reg_domain_from_country_code(v_REGDOMAIN_t *reg_domain_ptr,
						const uint8_t *country_alpha2,
						enum country_src source)
{
	if (NULL == reg_domain_ptr) {
		QDF_TRACE(QDF_MODULE_ID_QDF, QDF_TRACE_LEVEL_ERROR,
			  "Invalid reg domain pointer");
		return QDF_STATUS_E_FAULT;
	}

	*reg_domain_ptr = 0;

	if (SOURCE_QUERY == source)
		return QDF_STATUS_SUCCESS;

	if (NULL == country_alpha2) {
		QDF_TRACE(QDF_MODULE_ID_QDF, QDF_TRACE_LEVEL_ERROR,
			  "Country code array is NULL");
		return QDF_STATUS_E_FAULT;
	}

	if (cds_is_driver_recovering()) {
		QDF_TRACE(QDF_MODULE_ID_QDF, QDF_TRACE_LEVEL_ERROR,
			  "SSR in progress, return");
		return QDF_STATUS_SUCCESS;
	}

	if (SOURCE_11D == source || SOURCE_USERSPACE == source)
		regulatory_hint_user(country_alpha2,
				     NL80211_USER_REG_HINT_USER);

	return QDF_STATUS_SUCCESS;
}

/**
 * cds_is_fcc_regdomian() - is the regdomain FCC
 *
 * Return: true on FCC regdomain, false otherwise
 */
bool cds_is_fcc_regdomain(void)
{
	v_REGDOMAIN_t domainId;
	hdd_context_t *hdd_ctx;

	hdd_ctx = cds_get_context(QDF_MODULE_ID_HDD);
	if (wlan_hdd_validate_context(hdd_ctx))
		return false;
	cds_get_reg_domain_from_country_code(&domainId,
			hdd_ctx->reg.alpha2, SOURCE_QUERY);
	if (REGDOMAIN_FCC == domainId)
		return true;
	return false;
}

/*
 * cds_is_dsrc_channel() - is the channel DSRC
 * @center_freq: center freq of the channel
 *
 * Return: true if dsrc channel
 *         false otherwise
 */
bool cds_is_dsrc_channel(uint16_t center_freq)
{
	if (center_freq >= 5852 &&
	    center_freq <= 5920)
		return true;

	return false;
}

/**
 * cds_set_reg_domain() - set regulatory domain
 * @client_ctxt: client context
 * @reg_domain: regulatory domain
 *
 * Return: QDF_STATUS
 */
QDF_STATUS cds_set_reg_domain(void *client_ctxt, v_REGDOMAIN_t reg_domain)
{
	if (reg_domain >= REGDOMAIN_COUNT) {
		QDF_TRACE(QDF_MODULE_ID_QDF_DEVICE, QDF_TRACE_LEVEL_ERROR,
			  "CDS set reg domain, invalid REG domain ID %d",
			  reg_domain);
		return QDF_STATUS_E_INVAL;
	}

	return QDF_STATUS_SUCCESS;
}

/**
 * cds_put_default_country() - save the default country
 * @dfs_country: default country
 *
 * Return: QDF_STATUS
 */
QDF_STATUS cds_put_default_country(uint8_t *def_country)
{
	default_country[0] = def_country[0];
	default_country[1] = def_country[1];

	return QDF_STATUS_SUCCESS;
}

/**
 * cds_bw_value() - give bandwidth value
 * bw: bandwidth enum
 *
 * Return: uint16_t
 */
uint16_t cds_bw_value(enum phy_ch_width bw)
{
	switch (bw) {
	case CH_WIDTH_20MHZ:
		return 20;
	case CH_WIDTH_40MHZ:
		return 40;
	case CH_WIDTH_80MHZ:
		return 80;
	case CH_WIDTH_160MHZ:
		return 160;
	case CH_WIDTH_80P80MHZ:
		return 160;
	case CH_WIDTH_INVALID:
		return 0;
	case CH_WIDTH_5MHZ:
		return 10;
	case CH_WIDTH_10MHZ:
		return 5;
	case CH_WIDTH_MAX:
		return 160;
	default:
		return 0;
	}
}

