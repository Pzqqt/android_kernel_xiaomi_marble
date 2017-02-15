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

#if !defined __CDS_REG_SERVICE_H
#define __CDS_REG_SERVICE_H

/**=========================================================================

   \file  cds_reg_service.h

   \brief Connectivity driver services (CDS): Non-Volatile storage API

   ========================================================================*/

#include "qdf_status.h"
#include <reg_services_public_struct.h>
#include <wlan_reg_services_api.h>

#define CDS_COUNTRY_CODE_LEN  2
#define CDS_MAC_ADDRESS_LEN 6
#define CDS_SBS_SEPARATION_THRESHOLD 100
#define HT40PLUS_2G_FCC_CH_END       7
#define HT40PLUS_2G_EURJAP_CH_END    9
#define HT40MINUS_2G_CH_START        5
#define HT40MINUS_2G_CH_END          13


#define CDS_CHANNEL_STATE(chan_enum) reg_channels[chan_enum].state
#define CDS_CHANNEL_NUM(chan_enum) channel_map[chan_enum].chan_num
#define CDS_CHANNEL_FREQ(chan_enum) channel_map[chan_enum].center_freq
#define CDS_IS_DFS_CH(chan_num) (cds_get_channel_state((chan_num)) == \
				CHANNEL_STATE_DFS)

#define CDS_IS_PASSIVE_OR_DISABLE_CH(chan_num) \
	(cds_get_channel_state(chan_num) != CHANNEL_STATE_ENABLE)

#define CDS_MIN_24GHZ_CHANNEL_NUMBER channel_map[MIN_24GHZ_CHANNEL].chan_num
#define CDS_MAX_24GHZ_CHANNEL_NUMBER channel_map[MAX_24GHZ_CHANNEL].chan_num
#define CDS_MIN_5GHZ_CHANNEL_NUMBER channel_map[MIN_5GHZ_CHANNEL].chan_num
#define CDS_MAX_5GHZ_CHANNEL_NUMBER channel_map[MAX_5GHZ_CHANNEL].chan_num

#define CDS_IS_CHANNEL_5GHZ(chan_num) \
	((chan_num >= CDS_MIN_5GHZ_CHANNEL_NUMBER) && \
	 (chan_num <= CDS_MAX_5GHZ_CHANNEL_NUMBER))

#define CDS_IS_CHANNEL_24GHZ(chan_num) \
	((chan_num >= CDS_MIN_24GHZ_CHANNEL_NUMBER) && \
	 (chan_num <= CDS_MAX_24GHZ_CHANNEL_NUMBER))

#define CDS_IS_SAME_BAND_CHANNELS(chan_num1, chan_num2) \
	(chan_num1 && chan_num2 && \
	(CDS_IS_CHANNEL_5GHZ(chan_num1) == CDS_IS_CHANNEL_5GHZ(chan_num2)))

#define CDS_IS_CHANNEL_VALID_5G_SBS(curchan, newchan) \
	(curchan > newchan ? \
	CDS_CHANNEL_FREQ(cds_get_channel_enum(curchan)) \
	- CDS_CHANNEL_FREQ(cds_get_channel_enum(newchan)) \
	 > CDS_SBS_SEPARATION_THRESHOLD : \
	CDS_CHANNEL_FREQ(cds_get_channel_enum(newchan)) \
	- CDS_CHANNEL_FREQ(cds_get_channel_enum(curchan)) \
	 > CDS_SBS_SEPARATION_THRESHOLD)

extern const struct chan_map channel_map[NUM_CHANNELS];

extern struct regulatory_channel reg_channels[NUM_CHANNELS];

QDF_STATUS cds_get_reg_domain_from_country_code(v_REGDOMAIN_t *pRegDomain,
						const uint8_t *country_alpha2,
						enum country_src source);

/**
 * cds_is_fcc_regdomian() - is the regdomain FCC
 *
 * Return: true on FCC regdomain, false otherwise
 */
bool cds_is_fcc_regdomain(void);

QDF_STATUS cds_read_default_country(uint8_t *default_country);

QDF_STATUS cds_get_channel_list_with_power(struct channel_power
					   *base_channels,
					   uint8_t *num_base_channels);

enum channel_enum cds_get_channel_enum(uint32_t chan_num);

enum channel_state cds_get_channel_state(uint32_t chan_num);

/**
 * cds_get_channel_reg_power() - get max power based on regulatory
 * @chan_num: channel number
 *
 * Return: tx power
 */
int8_t cds_get_channel_reg_power(uint32_t chan_num);

/**
 * cds_get_channel_flags() - This API returns regulatory channel flags
 * @chan_num: channel number
 *
 * Return: channel flags
 */
uint32_t cds_get_channel_flags(uint32_t chan_num);

/**
 * cds_get_vendor_reg_flags() - This API returns vendor specific regulatory
 * channel flags
 * @chan_num: channel number
 *
 * Return: channel flags
 */
uint32_t cds_get_vendor_reg_flags(uint32_t chan, uint16_t bandwidth,
				bool is_ht_enabled, bool is_vht_enabled,
				uint8_t is_sub_20_channel_width);

/**
 * cds_get_channel_freq() - This API returns frequency for channel
 * @chan_num: channel number
 *
 * Return: frequency
 */
uint32_t cds_get_channel_freq(uint32_t chan_num);

bool cds_is_dsrc_channel(uint16_t center_freq);
enum channel_state cds_get_5g_bonded_channel_state(uint16_t chan_num,
						 enum phy_ch_width chan_width);

enum channel_state cds_get_2g_bonded_channel_state(uint16_t chan_num,
						   enum phy_ch_width chan_width,
						   uint16_t sec_ch);
/**
 * cds_get_2g_bonded_channel_state() - get the channel bonded channel state
 * @oper_ch: operating channel
 * @ch_width: channel width
 * @sec_ch: secondary channel
 *
 * Return: channel state
 */
enum channel_state cds_get_bonded_channel_state(uint16_t oper_ch,
						enum phy_ch_width ch_width,
						uint16_t sec_ch);
void cds_set_channel_params(uint16_t oper_ch, uint16_t ht_offset_2g,
			    struct ch_params *ch_params);

QDF_STATUS cds_set_reg_domain(void *client_ctxt, v_REGDOMAIN_t reg_domain);
QDF_STATUS cds_put_default_country(uint8_t *def_country);
uint16_t cds_bw_value(enum phy_ch_width bw);
uint8_t cds_skip_dfs_and_2g(uint32_t rf_channel);

#endif /* __CDS_REG_SERVICE_H */
