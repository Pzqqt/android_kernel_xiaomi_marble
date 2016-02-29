/*
 * Copyright (c) 2014-2016 The Linux Foundation. All rights reserved.
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
#include "cds_reg_service.h"
#include "qdf_trace.h"
#include "sme_api.h"
#include "cds_api.h"
#include "cds_reg_service.h"
#include "cds_regdomain.h"

const struct chan_map chan_mapping[NUM_CHANNELS] = {
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

	[CHAN_ENUM_170] = {5852, 170},
	[CHAN_ENUM_171] = {5855, 171},
	[CHAN_ENUM_172] = {5860, 172},
	[CHAN_ENUM_173] = {5865, 173},
	[CHAN_ENUM_174] = {5870, 174},
	[CHAN_ENUM_175] = {5875, 175},
	[CHAN_ENUM_176] = {5880, 176},
	[CHAN_ENUM_177] = {5885, 177},
	[CHAN_ENUM_178] = {5890, 178},
	[CHAN_ENUM_179] = {5895, 179},
	[CHAN_ENUM_180] = {5900, 180},
	[CHAN_ENUM_181] = {5905, 181},
	[CHAN_ENUM_182] = {5910, 182},
	[CHAN_ENUM_183] = {5915, 183},
	[CHAN_ENUM_184] = {5920, 184},
};

struct regulatory_channel reg_channels[NUM_CHANNELS];
uint8_t default_country[CDS_COUNTRY_CODE_LEN + 1];
uint8_t dfs_region;

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
		for (i = 0; i <= CHAN_ENUM_14; i++) {
			if (reg_channels[i].state) {
				base_channels[count].chan_num =
					chan_mapping[i].chan_num;
				base_channels[count++].power =
					reg_channels[i].pwr_limit;
			}
		}
		for (i = CHAN_ENUM_36; i <= CHAN_ENUM_184; i++) {
			if (reg_channels[i].state) {
				base_channels[count].chan_num =
					chan_mapping[i].chan_num;
				base_channels[count++].power =
					reg_channels[i].pwr_limit;
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
static enum channel_enum cds_get_channel_enum(uint32_t chan_num)
{
	uint32_t loop;

	for (loop = 0; loop <= CHAN_ENUM_184; loop++)
		if (chan_mapping[loop].chan_num == chan_num)
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


/**
 * cds_get_bonded_channel_state() - get the bonded channel state
 * @channel_num: channel number
 *
 * Return: channel state
 */
enum channel_state cds_get_bonded_channel_state(uint32_t chan_num,
					   enum phy_ch_width ch_width)
{
	enum channel_enum chan_enum;
	bool bw_enabled = false;

	chan_enum = cds_get_channel_enum(chan_num);
	if (INVALID_CHANNEL == chan_enum)
		return CHANNEL_STATE_INVALID;

	if (reg_channels[chan_enum].state) {
		if (CH_WIDTH_5MHZ == ch_width)
			bw_enabled = 1;
		else if (CH_WIDTH_10MHZ == ch_width)
			bw_enabled = !(reg_channels[chan_enum].flags &
				       IEEE80211_CHAN_NO_10MHZ);
		else if (CH_WIDTH_20MHZ == ch_width)
			bw_enabled = !(reg_channels[chan_enum].flags &
				       IEEE80211_CHAN_NO_20MHZ);
		else if (CH_WIDTH_40MHZ == ch_width)
			bw_enabled = !(reg_channels[chan_enum].flags &
				       IEEE80211_CHAN_NO_HT40);
		else if (CH_WIDTH_80MHZ == ch_width)
			bw_enabled = !(reg_channels[chan_enum].flags &
				       IEEE80211_CHAN_NO_80MHZ);
		else if (CH_WIDTH_160MHZ == ch_width)
			bw_enabled = !(reg_channels[chan_enum].flags &
				       IEEE80211_CHAN_NO_160MHZ);
	}

	if (bw_enabled)
		return reg_channels[chan_enum].state;
	else
		return CHANNEL_STATE_DISABLE;
}

/**
 * cds_get_max_channel_bw() - get the max channel bandwidth
 * @channel_num: channel number
 *
 * Return: channel_width
 */
enum phy_ch_width cds_get_max_channel_bw(uint32_t chan_num)
{
	enum channel_enum chan_enum;
	enum phy_ch_width chan_bw = CH_WIDTH_INVALID;

	chan_enum = cds_get_channel_enum(chan_num);

	if ((INVALID_CHANNEL != chan_enum) &&
	    (CHANNEL_STATE_DISABLE != reg_channels[chan_enum].state)) {

		if (!(reg_channels[chan_enum].flags &
		      IEEE80211_CHAN_NO_160MHZ))
			chan_bw = CH_WIDTH_160MHZ;
		else if (!(reg_channels[chan_enum].flags &
			   IEEE80211_CHAN_NO_80MHZ))
			chan_bw = CH_WIDTH_80MHZ;
		else if (!(reg_channels[chan_enum].flags &
			   IEEE80211_CHAN_NO_HT40))
			chan_bw = CH_WIDTH_40MHZ;
		else if (!(reg_channels[chan_enum].flags &
			   IEEE80211_CHAN_NO_20MHZ))
			chan_bw = CH_WIDTH_20MHZ;
		else if (!(reg_channels[chan_enum].flags &
			   IEEE80211_CHAN_NO_10MHZ))
			chan_bw = CH_WIDTH_10MHZ;
		else
			chan_bw = CH_WIDTH_5MHZ;
	}
	return chan_bw;
}

/**
 * cds_get_dfs_region() - get the dfs_region
 * @dfs_region: the dfs_region to return
 *
 * Return: QDF_STATUS
 */
QDF_STATUS cds_get_dfs_region(uint8_t *dfs_reg)
{
	*dfs_reg = dfs_region;

	return QDF_STATUS_SUCCESS;
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
		return 1;

	return 0;
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
 * cds_put_dfs_region() - save dfs region
 * @dfs_reg: dfs region
 *
 * Return: QDF_STATUS
 */
QDF_STATUS cds_put_dfs_region(uint8_t dfs_reg)
{
	dfs_region = dfs_reg;

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
 * cds_set_ch_params() - set channel parameters
 * @ch: channel
 * @phy_mode: physical mode
 * @ch_param: channel parameters will be returned
 *
 * Return: None
 */
void cds_set_ch_params(uint8_t ch, uint32_t phy_mode,
		       struct ch_params_s *ch_params)
{
	tHalHandle *hal_ctx = cds_get_context(QDF_MODULE_ID_PE);
	if (!hal_ctx) {
		QDF_TRACE(QDF_MODULE_ID_QDF, QDF_TRACE_LEVEL_ERROR,
			("Invalid hal_ctx pointer"));
		return;
	}
	/*
	 * TODO: remove SME call and move the SME set channel
	 * param functionality to CDS.
	 */
	sme_set_ch_params(hal_ctx, phy_mode, ch, 0, ch_params);
}
