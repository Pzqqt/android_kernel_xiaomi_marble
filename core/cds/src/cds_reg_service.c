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

const struct chan_map chan_mapping[NUM_RF_CHANNELS] = {
	{2412, 1},
	{2417, 2},
	{2422, 3},
	{2427, 4},
	{2432, 5},
	{2437, 6},
	{2442, 7},
	{2447, 8},
	{2452, 9},
	{2457, 10},
	{2462, 11},
	{2467, 12},
	{2472, 13},
	{2484, 14},
	{5180, 36},
	{5200, 40},
	{5220, 44},
	{5240, 48},
	{5260, 52},
	{5280, 56},
	{5300, 60},
	{5320, 64},
	{5500, 100},
	{5520, 104},
	{5540, 108},
	{5560, 112},
	{5580, 116},
	{5600, 120},
	{5620, 124},
	{5640, 128},
	{5660, 132},
	{5680, 136},
	{5700, 140},
	{5720, 144},
	{5745, 149},
	{5765, 153},
	{5785, 157},
	{5805, 161},
	{5825, 165},
	{5852, 170},
	{5855, 171},
	{5860, 172},
	{5865, 173},
	{5870, 174},
	{5875, 175},
	{5880, 176},
	{5885, 177},
	{5890, 178},
	{5895, 179},
	{5900, 180},
	{5905, 181},
	{5910, 182},
	{5915, 183},
	{5920, 184},
	{2422, 3},
	{2427, 4},
	{2432, 5},
	{2437, 6},
	{2442, 7},
	{2447, 8},
	{2452, 9},
	{2457, 10},
	{2462, 11},
	{5190, 38},
	{5210, 42},
	{5230, 46},
	{5250, 50},
	{5270, 54},
	{5290, 58},
	{5310, 62},
	{5510, 102},
	{5530, 106},
	{5550, 110},
	{5570, 114},
	{5590, 118},
	{5610, 122},
	{5630, 126},
	{5650, 130},
	{5670, 134},
	{5690, 138},
	{5710, 142},
	{5755, 151},
	{5775, 155},
	{5795, 159},
	{5815, 163},
};

struct regulatory_channel reg_channels[NUM_RF_CHANNELS];
uint8_t default_country[CDS_COUNTRY_CODE_LEN + 1];
uint8_t dfs_region;

/**
 * cds_get_channel_list_with_power() - retrieve channel list with power
 * @base_channels: base channels
 * @num_base_channels: number of base channels
 * @channels_40mhz: 40 MHz channels
 * @num_40mhz_channels: number of 40 Mhz channels
 *
 * Return: QDF_STATUS_SUCCESS
 */
QDF_STATUS cds_get_channel_list_with_power(struct channel_power
					   *base_channels,
					   uint8_t *num_base_channels,
					   struct channel_power
					   *channels_40mhz,
					   uint8_t *num_40mhz_channels)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	int i, count;

	if (base_channels && num_base_channels) {
		count = 0;
		for (i = 0; i <= RF_CHAN_14; i++) {
			if (reg_channels[i].state) {
				base_channels[count].chan_num =
					chan_mapping[i].chan_num;
				base_channels[count++].power =
					reg_channels[i].pwr_limit;
			}
		}
		for (i = RF_CHAN_36; i <= RF_CHAN_184; i++) {
			if (reg_channels[i].state) {
				base_channels[count].chan_num =
					chan_mapping[i].chan_num;
				base_channels[count++].power =
					reg_channels[i].pwr_limit;
			}
		}
		*num_base_channels = count;
	}

	if (channels_40mhz && num_40mhz_channels) {
		count = 0;

		for (i = RF_CHAN_BOND_3; i <= RF_CHAN_BOND_11; i++) {
			if (reg_channels[i].state) {
				channels_40mhz[count].chan_num =
					chan_mapping[i].chan_num;
				channels_40mhz[count++].power =
					reg_channels[i].pwr_limit;
			}
		}

		for (i = RF_CHAN_BOND_38; i <= RF_CHAN_BOND_163; i++) {
			if (reg_channels[i].state) {
				channels_40mhz[count].chan_num =
					chan_mapping[i].chan_num;
				channels_40mhz[count++].power =
					reg_channels[i].pwr_limit;
			}
		}
		*num_40mhz_channels = count;
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

	for (loop = 0; loop <= RF_CHAN_184; loop++)
		if (chan_mapping[loop].chan_num == chan_num)
			return loop;

	QDF_TRACE(QDF_MODULE_ID_QDF, QDF_TRACE_LEVEL_ERROR,
		  "invalid channel number %d", chan_num);

	return INVALID_RF_CHANNEL;
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
	if (INVALID_RF_CHANNEL == chan_enum)
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
					   enum channel_width ch_width)
{
	enum channel_enum chan_enum;
	bool bw_enabled = false;

	chan_enum = cds_get_channel_enum(chan_num);
	if (INVALID_RF_CHANNEL == chan_enum)
		return CHANNEL_STATE_INVALID;

	if (reg_channels[chan_enum].state) {
		if (CHAN_WIDTH_5MHZ == ch_width)
			bw_enabled = 1;
		else if (CHAN_WIDTH_10MHZ == ch_width)
			bw_enabled = !(reg_channels[chan_enum].flags &
				       IEEE80211_CHAN_NO_10MHZ);
		else if (CHAN_WIDTH_20MHZ == ch_width)
			bw_enabled = !(reg_channels[chan_enum].flags &
				       IEEE80211_CHAN_NO_20MHZ);
		else if (CHAN_WIDTH_40MHZ == ch_width)
			bw_enabled = !(reg_channels[chan_enum].flags &
				       IEEE80211_CHAN_NO_HT40);
		else if (CHAN_WIDTH_80MHZ == ch_width)
			bw_enabled = !(reg_channels[chan_enum].flags &
				       IEEE80211_CHAN_NO_80MHZ);
		else if (CHAN_WIDTH_160MHZ == ch_width)
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
enum channel_width cds_get_max_channel_bw(uint32_t chan_num)
{
	enum channel_enum chan_enum;
	enum channel_width chan_bw = CHAN_WIDTH_0MHZ;

	chan_enum = cds_get_channel_enum(chan_num);

	if ((INVALID_RF_CHANNEL != chan_enum) &&
	    (CHANNEL_STATE_DISABLE != reg_channels[chan_enum].state)) {

		if (!(reg_channels[chan_enum].flags &
		      IEEE80211_CHAN_NO_160MHZ))
			chan_bw = CHAN_WIDTH_160MHZ;
		else if (!(reg_channels[chan_enum].flags &
			   IEEE80211_CHAN_NO_80MHZ))
			chan_bw = CHAN_WIDTH_80MHZ;
		else if (!(reg_channels[chan_enum].flags &
			   IEEE80211_CHAN_NO_HT40))
			chan_bw = CHAN_WIDTH_40MHZ;
		else if (!(reg_channels[chan_enum].flags &
			   IEEE80211_CHAN_NO_20MHZ))
			chan_bw = CHAN_WIDTH_20MHZ;
		else if (!(reg_channels[chan_enum].flags &
			   IEEE80211_CHAN_NO_10MHZ))
			chan_bw = CHAN_WIDTH_10MHZ;
		else
			chan_bw = CHAN_WIDTH_5MHZ;
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
