/*
 * Copyright (c) 2014-2018 The Linux Foundation. All rights reserved.
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
 * DOC: wlan_hdd_regulatory.c
 *
 * hdd regulatory implementation
 */

#include "qdf_types.h"
#include "qdf_trace.h"
#include "wlan_hdd_main.h"
#include <wlan_osif_priv.h>
#include "wlan_hdd_regulatory.h"
#include <wlan_reg_ucfg_api.h>
#include "cds_regdomain.h"
#include "pld_common.h"

#define REG_RULE_2412_2462    REG_RULE(2412-10, 2462+10, 40, 0, 20, 0)

#define REG_RULE_2467_2472    REG_RULE(2467-10, 2472+10, 40, 0, 20, \
		NL80211_RRF_PASSIVE_SCAN | NL80211_RRF_NO_IBSS)

#define REG_RULE_2484         REG_RULE(2484-10, 2484+10, 20, 0, 20, \
		NL80211_RRF_PASSIVE_SCAN | NL80211_RRF_NO_IBSS | \
				       NL80211_RRF_NO_OFDM)

#define REG_RULE_5180_5320    REG_RULE(5180-10, 5320+10, 160, 0, 20, \
		NL80211_RRF_PASSIVE_SCAN | NL80211_RRF_NO_IBSS)

#define REG_RULE_5500_5720    REG_RULE(5500-10, 5720+10, 160, 0, 20, \
		NL80211_RRF_PASSIVE_SCAN | NL80211_RRF_NO_IBSS)

#define REG_RULE_5745_5925    REG_RULE(5745-10, 5925+10, 80, 0, 20, \
		NL80211_RRF_PASSIVE_SCAN | NL80211_RRF_NO_IBSS)

static bool init_by_driver;
static bool init_by_reg_core;

struct regulatory_channel reg_channels[NUM_CHANNELS];

static const struct ieee80211_regdomain
hdd_world_regrules_60_61_62 = {
	.n_reg_rules = 6,
	.alpha2 =  "00",
	.reg_rules = {
		REG_RULE_2412_2462,
		REG_RULE_2467_2472,
		REG_RULE_2484,
		REG_RULE_5180_5320,
		REG_RULE_5500_5720,
		REG_RULE_5745_5925,
	}
};

static const struct ieee80211_regdomain
hdd_world_regrules_63_65 = {
	.n_reg_rules = 4,
	.alpha2 =  "00",
	.reg_rules = {
		REG_RULE_2412_2462,
		REG_RULE_2467_2472,
		REG_RULE_5180_5320,
		REG_RULE_5745_5925,
	}
};

static const struct ieee80211_regdomain
hdd_world_regrules_64 = {
	.n_reg_rules = 3,
	.alpha2 =  "00",
	.reg_rules = {
		REG_RULE_2412_2462,
		REG_RULE_5180_5320,
		REG_RULE_5745_5925,
	}
};

static const struct ieee80211_regdomain
hdd_world_regrules_66_69 = {
	.n_reg_rules = 4,
	.alpha2 =  "00",
	.reg_rules = {
		REG_RULE_2412_2462,
		REG_RULE_5180_5320,
		REG_RULE_5500_5720,
		REG_RULE_5745_5925,
	}
};

static const struct ieee80211_regdomain
hdd_world_regrules_67_68_6A_6C = {
	.n_reg_rules = 5,
	.alpha2 =  "00",
	.reg_rules = {
		REG_RULE_2412_2462,
		REG_RULE_2467_2472,
		REG_RULE_5180_5320,
		REG_RULE_5500_5720,
		REG_RULE_5745_5925,
	}
};

/**
 * hdd_get_world_regrules() - get the appropriate world regrules
 * @reg: regulatory data
 *
 * Return: regulatory rules ptr
 */
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 4, 0))
static const struct ieee80211_regdomain *hdd_get_world_regrules(
	struct regulatory *reg)
{
	struct reg_dmn_pair *regpair =
		(struct reg_dmn_pair *)reg->regpair;

	switch (regpair->reg_dmn_pair) {
	case 0x60:
	case 0x61:
	case 0x62:
		return &hdd_world_regrules_60_61_62;
	case 0x63:
	case 0x65:
		return &hdd_world_regrules_63_65;
	case 0x64:
		return &hdd_world_regrules_64;
	case 0x66:
	case 0x69:
		return &hdd_world_regrules_66_69;
	case 0x67:
	case 0x68:
	case 0x6A:
	case 0x6C:
		return &hdd_world_regrules_67_68_6A_6C;
	default:
		hdd_warn("invalid world mode in BDF");
		return &hdd_world_regrules_60_61_62;
	}
}

/**
 * hdd_is_world_regdomain() - whether world regdomain
 * @reg_domain: integer regulatory domain
 *
 * Return: bool
 */
static bool hdd_is_world_regdomain(uint32_t reg_domain)
{
	uint32_t temp_regd = reg_domain & ~WORLD_ROAMING_FLAG;

	return ((temp_regd & CTRY_FLAG) != CTRY_FLAG) &&
		((temp_regd & WORLD_ROAMING_MASK) ==
		 WORLD_ROAMING_PREFIX);
}

/**
 * hdd_update_regulatory_info() - update regulatory info
 * @hdd_ctx: hdd context
 *
 * Return: Error Code
 */
static int hdd_update_regulatory_info(struct hdd_context *hdd_ctx)
{
	uint32_t country_code;

	country_code = cds_get_country_from_alpha2(hdd_ctx->reg.alpha2);

	hdd_ctx->reg.reg_domain = CTRY_FLAG;
	hdd_ctx->reg.reg_domain |= country_code;

	return cds_fill_some_regulatory_info(&hdd_ctx->reg);

}
#endif

/**
 * hdd_reset_global_reg_params - Reset global static reg params
 *
 * This function is helpful in static driver to reset
 * the global params.
 *
 * Return: void
 */
void hdd_reset_global_reg_params(void)
{
	init_by_driver = false;
	init_by_reg_core = false;
}

static void reg_program_config_vars(struct hdd_context *hdd_ctx,
				    struct reg_config_vars *config_vars)
{
	config_vars->enable_11d_support = hdd_ctx->config->Is11dSupportEnabled;
	config_vars->scan_11d_interval = hdd_ctx->config->scan_11d_interval;
	config_vars->userspace_ctry_priority =
		hdd_ctx->config->fSupplicantCountryCodeHasPriority;
	config_vars->dfs_enabled = hdd_ctx->config->enableDFSChnlScan;
	config_vars->indoor_chan_enabled =
		hdd_ctx->config->indoor_channel_support;
	config_vars->band_capability = hdd_ctx->config->nBandCapability;
	config_vars->restart_beaconing = hdd_ctx->config->
		restart_beaconing_on_chan_avoid_event;
}


/**
 * hdd_regulatory_wiphy_init() - regulatory wiphy init
 * @hdd_ctx: hdd context
 * @reg: regulatory data
 * @wiphy: wiphy structure
 *
 * Return: void
 */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 4, 0))
#elif (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 14, 0)) || defined(WITH_BACKPORTS)
static void hdd_regulatory_wiphy_init(struct hdd_context *hdd_ctx,
				     struct regulatory *reg,
				     struct wiphy *wiphy)
{
	const struct ieee80211_regdomain *reg_rules;
	int chan_num;
	struct ieee80211_channel *chan;

	if (hdd_is_world_regdomain(reg->reg_domain)) {
		reg_rules = hdd_get_world_regrules(reg);
		wiphy->regulatory_flags |= REGULATORY_CUSTOM_REG;
	} else if (hdd_ctx->config->fRegChangeDefCountry) {
		wiphy->regulatory_flags |= REGULATORY_CUSTOM_REG;
		reg_rules = &hdd_world_regrules_60_61_62;
	} else {
		wiphy->regulatory_flags |= REGULATORY_STRICT_REG;
		reg_rules = &hdd_world_regrules_60_61_62;
	}

	/*
	 * save the original driver regulatory flags
	 */
	hdd_ctx->reg.reg_flags = wiphy->regulatory_flags;
	wiphy_apply_custom_regulatory(wiphy, reg_rules);

	/*
	 * disable 2.4 Ghz channels that dont have 20 mhz bw
	 */
	for (chan_num = 0;
	     chan_num < wiphy->bands[HDD_NL80211_BAND_2GHZ]->n_channels;
	     chan_num++) {
		chan = &(wiphy->bands[HDD_NL80211_BAND_2GHZ]->channels[chan_num]);
		if (chan->flags & IEEE80211_CHAN_NO_20MHZ)
			chan->flags |= IEEE80211_CHAN_DISABLED;
	}

	/*
	 * restore the driver regulatory flags since
	 * wiphy_apply_custom_regulatory may have
	 * changed them
	 */
	wiphy->regulatory_flags = hdd_ctx->reg.reg_flags;

}
#else
static void hdd_regulatory_wiphy_init(struct hdd_context *hdd_ctx,
				     struct regulatory *reg,
				     struct wiphy *wiphy)
{
	const struct ieee80211_regdomain *reg_rules;

	if (hdd_is_world_regdomain(reg->reg_domain)) {
		reg_rules = hdd_get_world_regrules(reg);
		wiphy->flags |= WIPHY_FLAG_CUSTOM_REGULATORY;
	} else if (hdd_ctx->config->fRegChangeDefCountry) {
		wiphy->flags |= WIPHY_FLAG_CUSTOM_REGULATORY;
		reg_rules = &hdd_world_regrules_60_61_62;
	} else {
		wiphy->flags |= WIPHY_FLAG_STRICT_REGULATORY;
		reg_rules = &hdd_world_regrules_60_61_62;
	}

	/*
	 * save the original driver regulatory flags
	 */
	hdd_ctx->reg.reg_flags = wiphy->flags;
	wiphy_apply_custom_regulatory(wiphy, reg_rules);

	/*
	 * restore the driver regulatory flags since
	 * wiphy_apply_custom_regulatory may have
	 * changed them
	 */
	wiphy->flags = hdd_ctx->reg.reg_flags;

}
#endif

/**
 * is_wiphy_custom_regulatory() - is custom regulatory defined
 * @wiphy: wiphy
 *
 * Return: int
 */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 4, 0))
#elif (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 14, 0)) || defined(WITH_BACKPORTS)
static int is_wiphy_custom_regulatory(struct wiphy *wiphy)
{

	return wiphy->regulatory_flags & REGULATORY_CUSTOM_REG;
}
#else
static int is_wiphy_custom_regulatory(struct wiphy *wiphy)
{
	return wiphy->flags & WIPHY_FLAG_CUSTOM_REGULATORY;
}
#endif

/**
 * hdd_modify_wiphy() - modify wiphy
 * @wiphy: wiphy
 * @chan: channel structure
 *
 * Return: void
 */
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 4, 0))
static void hdd_modify_wiphy(struct wiphy  *wiphy,
			     struct ieee80211_channel *chan)
{
	const struct ieee80211_reg_rule *reg_rule;

	if (is_wiphy_custom_regulatory(wiphy)) {
		reg_rule = freq_reg_info(wiphy, MHZ_TO_KHZ(chan->center_freq));
		if (!IS_ERR(reg_rule)) {
			chan->flags &= ~IEEE80211_CHAN_DISABLED;

			if (!(reg_rule->flags & NL80211_RRF_DFS)) {
				hdd_debug("Remove dfs restriction for %u",
					chan->center_freq);
				chan->flags &= ~IEEE80211_CHAN_RADAR;
			}

			if (!(reg_rule->flags & NL80211_RRF_PASSIVE_SCAN)) {
				hdd_debug("Remove passive restriction for %u",
					chan->center_freq);
				chan->flags &= ~IEEE80211_CHAN_PASSIVE_SCAN;
			}

			if (!(reg_rule->flags & NL80211_RRF_NO_IBSS)) {
				hdd_debug("Remove no ibss restriction for %u",
					chan->center_freq);
				chan->flags &= ~IEEE80211_CHAN_NO_IBSS;
			}

			chan->max_power =
				MBM_TO_DBM(reg_rule->power_rule.max_eirp);
		}
	}
}
#endif

/**
 * hdd_set_dfs_region() - set the dfs_region
 * @dfs_region: the dfs_region to set
 *
 * Return: void
 */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 4, 0))
#elif (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 14, 0)) || defined(WITH_BACKPORTS)
static void hdd_set_dfs_region(struct hdd_context *hdd_ctx,
			       enum dfs_reg dfs_reg)
{
	wlan_reg_set_dfs_region(hdd_ctx->hdd_pdev, dfs_reg);
}
#endif

/**
 * hdd_process_regulatory_data() - process regulatory data
 * @hdd_ctx: hdd context
 * @wiphy: wiphy
 * @reset: whether to reset channel data
 *
 * Return: void
 */
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 4, 0))
static void hdd_process_regulatory_data(struct hdd_context *hdd_ctx,
					struct wiphy *wiphy,
					bool reset)
{
	int band_num;
	int chan_num;
	enum channel_enum chan_enum = CHAN_ENUM_1;
	struct ieee80211_channel *wiphy_chan, *wiphy_chan_144 = NULL;
	struct regulatory_channel *cds_chan;
	uint8_t band_capability;

	band_capability = hdd_ctx->curr_band;

	for (band_num = 0; band_num < HDD_NUM_NL80211_BANDS; band_num++) {

		if (wiphy->bands[band_num] == NULL)
			continue;

		for (chan_num = 0;
		     chan_num < wiphy->bands[band_num]->n_channels &&
		     chan_enum < NUM_CHANNELS;
		     chan_num++) {
			wiphy_chan =
				&(wiphy->bands[band_num]->channels[chan_num]);
			cds_chan = &(reg_channels[chan_enum]);
			cds_chan->chan_flags = 0;
			if (CHAN_ENUM_144 == chan_enum)
				wiphy_chan_144 = wiphy_chan;

			chan_enum++;

			if (!reset)
				hdd_modify_wiphy(wiphy, wiphy_chan);

			if (wiphy_chan->flags & IEEE80211_CHAN_DISABLED) {
				cds_chan->state = CHANNEL_STATE_DISABLE;
				cds_chan->chan_flags |=
					REGULATORY_CHAN_DISABLED;
			} else if (wiphy_chan->flags &
				    (IEEE80211_CHAN_RADAR |
				     IEEE80211_CHAN_PASSIVE_SCAN)) {
				cds_chan->state = CHANNEL_STATE_DFS;
				if (wiphy_chan->flags & IEEE80211_CHAN_RADAR)
					cds_chan->chan_flags |=
						REGULATORY_CHAN_RADAR;
				if (wiphy_chan->flags &
				    IEEE80211_CHAN_PASSIVE_SCAN)
					cds_chan->chan_flags |=
						REGULATORY_CHAN_NO_IR;
			} else if (wiphy_chan->flags &
				     IEEE80211_CHAN_INDOOR_ONLY) {
				cds_chan->chan_flags |=
					REGULATORY_CHAN_INDOOR_ONLY;
				if (hdd_ctx->config->indoor_channel_support
				    == false) {
					cds_chan->state = CHANNEL_STATE_DFS;
					wiphy_chan->flags |=
						IEEE80211_CHAN_PASSIVE_SCAN;
					cds_chan->chan_flags |=
						REGULATORY_CHAN_NO_IR;
				} else
					cds_chan->state = CHANNEL_STATE_ENABLE;
			} else
				cds_chan->state = CHANNEL_STATE_ENABLE;
			cds_chan->tx_power = wiphy_chan->max_power;
			if (wiphy_chan->flags & IEEE80211_CHAN_NO_10MHZ)
				cds_chan->max_bw = 5;
			else if (wiphy_chan->flags & IEEE80211_CHAN_NO_20MHZ)
				cds_chan->max_bw = 10;
			/*
			 * IEEE80211_CHAN_NO_HT40  is defined as 0x30 in kernel
			 * 4th BIT representing IEEE80211_CHAN_NO_HT40PLUS
			 * 5th BIT representing IEEE80211_CHAN_NO_HT40MINUS
			 *
			 * In order to claim no 40Mhz support value of
			 * wiphy_chan->flags needs to be 0x30.
			 * 0x20 and 0x10 values shows that either HT40+ or
			 * HT40- is not supported based on BIT set but they
			 * can support 40Mhz Operation.
			 */
			else if ((wiphy_chan->flags & IEEE80211_CHAN_NO_HT40) ==
					IEEE80211_CHAN_NO_HT40)
				cds_chan->max_bw = 20;
			else if (wiphy_chan->flags & IEEE80211_CHAN_NO_80MHZ)
				cds_chan->max_bw = 40;
			else if (wiphy_chan->flags & IEEE80211_CHAN_NO_160MHZ)
				cds_chan->max_bw = 80;
			else
				cds_chan->max_bw = 160;
		}
	}

	if (0 == (hdd_ctx->reg.eeprom_rd_ext &
		  (1 << WMI_REG_EXT_FCC_CH_144))) {
		cds_chan = &(reg_channels[CHAN_ENUM_144]);
		cds_chan->state = CHANNEL_STATE_DISABLE;
		if (NULL != wiphy_chan_144)
			wiphy_chan_144->flags |= IEEE80211_CHAN_DISABLED;
	}

	wlan_hdd_cfg80211_update_band(hdd_ctx, wiphy, band_capability);
}

/**
 * hdd_regulatory_init_no_offload() - regulatory init
 * @hdd_ctx: hdd context
 * @wiphy: wiphy
 *
 * Return: int
 */
static int hdd_regulatory_init_no_offload(struct hdd_context *hdd_ctx,
					  struct wiphy *wiphy)
{
	int ret_val;
	struct regulatory *reg_info;
	enum dfs_reg dfs_reg;
	struct reg_config_vars config_vars;

	reg_info = &hdd_ctx->reg;

	ret_val = cds_fill_some_regulatory_info(reg_info);
	if (ret_val) {
		hdd_err("incorrect BDF regulatory data");
		return ret_val;
	}

	hdd_set_dfs_region(hdd_ctx, DFS_FCC_REG);

	hdd_regulatory_wiphy_init(hdd_ctx, reg_info, wiphy);

	hdd_process_regulatory_data(hdd_ctx, wiphy, true);

	reg_info->cc_src = SOURCE_DRIVER;

	ucfg_reg_set_default_country(hdd_ctx->hdd_psoc, reg_info->alpha2);

	cds_fill_and_send_ctl_to_fw(reg_info);

	wlan_reg_get_dfs_region(hdd_ctx->hdd_pdev, &dfs_reg);

	reg_program_config_vars(hdd_ctx, &config_vars);
	ucfg_reg_set_config_vars(hdd_ctx->hdd_psoc, config_vars);
	ucfg_reg_program_mas_chan_list(hdd_ctx->hdd_psoc,
				       reg_channels,
				       hdd_ctx->reg.alpha2,
				       dfs_reg);

	return 0;
}
#endif

/**
 * hdd_program_country_code() - process channel information from country code
 * @hdd_ctx: hddc context
 *
 * Return: void
 */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 4, 0))
void hdd_program_country_code(struct hdd_context *hdd_ctx)
{
}
#else
void hdd_program_country_code(struct hdd_context *hdd_ctx)
{
	struct wiphy *wiphy = hdd_ctx->wiphy;
	uint8_t *country_alpha2 = hdd_ctx->reg.alpha2;

	if (!init_by_reg_core && !init_by_driver) {
		init_by_driver = true;
		if (('0' != country_alpha2[0]) ||
		    ('0' != country_alpha2[1]))
			regulatory_hint(wiphy, country_alpha2);
	}
}
#endif

int hdd_reg_set_country(struct hdd_context *hdd_ctx, char *country_code)
{
	int err;
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	/* validation */
	err = wlan_hdd_validate_context(hdd_ctx);
	if (err)
		return err;

	if (!country_code) {
		hdd_err("country_code is null");
		return -EINVAL;
	}

	status = ucfg_reg_set_country(hdd_ctx->hdd_pdev, country_code);

	if (QDF_IS_STATUS_ERROR(status))
		hdd_err("Failed to set country");

	return qdf_status_to_os_return(status);
}

/**
 * hdd_restore_custom_reg_settings() - restore custom reg settings
 * @wiphy: wiphy structure
 * @country_alpha2: alpha2 of the country
 * @reset: whether wiphy is reset
 *
 * Return: void
 */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 4, 0))
#elif (LINUX_VERSION_CODE > KERNEL_VERSION(3, 14, 0)) || defined(WITH_BACKPORTS)
static void hdd_restore_custom_reg_settings(struct wiphy *wiphy,
					    uint8_t *country_alpha2,
					    bool *reset)
{
}
#else
static void hdd_restore_custom_reg_settings(struct wiphy *wiphy,
					    uint8_t *country_alpha2,
					    bool *reset)
{
	struct ieee80211_supported_band *sband;
	enum nl80211_band band;
	struct ieee80211_channel *chan;
	int i;

	if ((country_alpha2[0] == '0') &&
	    (country_alpha2[1] == '0') &&
	    (wiphy->flags & WIPHY_FLAG_CUSTOM_REGULATORY)) {

		for (band = 0; band < HDD_NUM_NL80211_BANDS; band++) {
			sband = wiphy->bands[band];
			if (!sband)
				continue;
			for (i = 0; i < sband->n_channels; i++) {
				chan = &sband->channels[i];
				chan->flags = chan->orig_flags;
				chan->max_antenna_gain = chan->orig_mag;
				chan->max_power = chan->orig_mpwr;
			}
		}
		*reset = true;
	}
}
#endif

/**
 * hdd_restore_reg_flags() - restore regulatory flags
 * @flags: regulatory flags
 *
 * Return: void
 */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 4, 0))
#elif (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 14, 0)) || defined(WITH_BACKPORTS)
static void hdd_restore_reg_flags(struct wiphy *wiphy, uint32_t flags)
{
	wiphy->regulatory_flags = flags;
}
#else
static void hdd_restore_reg_flags(struct wiphy *wiphy, uint32_t flags)
{
	wiphy->flags = flags;
}
#endif

/**
 * hdd_reg_notifier() - regulatory notifier
 * @wiphy: wiphy
 * @request: regulatory request
 *
 * Return: void
 */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 4, 0))
void hdd_reg_notifier(struct wiphy *wiphy,
		      struct regulatory_request *request)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	struct hdd_context *hdd_ctx = wiphy_priv(wiphy);

	hdd_debug("country: %c%c, initiator %d, dfs_region: %d",
		  request->alpha2[0],
		  request->alpha2[1],
		  request->initiator,
		  request->dfs_region);

	switch (request->initiator) {
	case NL80211_REGDOM_SET_BY_CORE:
		if (pld_get_driver_load_cnt(hdd_ctx->parent_dev) == 0) {
			hdd_debug("ignore the 1st time notifier from CORE");
			return;
		}
		/* fall through to set country */
	case NL80211_REGDOM_SET_BY_USER:
		status = ucfg_reg_set_country(hdd_ctx->hdd_pdev,
					      request->alpha2);
		break;
	case NL80211_REGDOM_SET_BY_COUNTRY_IE:
	case NL80211_REGDOM_SET_BY_DRIVER:
	default:
		break;
	}

	if (QDF_IS_STATUS_ERROR(status))
		hdd_err("Failed to set country");
}
#else
void hdd_reg_notifier(struct wiphy *wiphy,
		      struct regulatory_request *request)
{
	struct hdd_context *hdd_ctx = wiphy_priv(wiphy);
	bool reset = false;
	enum dfs_reg dfs_reg;
	struct reg_config_vars config_vars;
	int ret_val;

	hdd_debug("country: %c%c, initiator %d, dfs_region: %d",
		  request->alpha2[0],
		  request->alpha2[1],
		  request->initiator,
		  request->dfs_region);

	if (NULL == hdd_ctx) {
		hdd_err("invalid hdd_ctx pointer");
		return;
	}

	if (cds_is_driver_unloading() || cds_is_driver_recovering() ||
	    cds_is_driver_in_bad_state()) {
		hdd_err("%s: unloading or ssr in progress, ignore",
			__func__);
		return;
	}

	if (hdd_ctx->driver_status == DRIVER_MODULES_CLOSED) {
		hdd_err("Driver module is closed; dropping request");
		return;
	}

	if (hdd_ctx->is_wiphy_suspended == true) {
		hdd_err("%s: system/cfg80211 is already suspend", __func__);
		return;
	}

	if (('K' == request->alpha2[0]) &&
	    ('R' == request->alpha2[1]))
		request->dfs_region = (enum nl80211_dfs_regions) DFS_KR_REG;

	if (('C' == request->alpha2[0]) &&
	    ('N' == request->alpha2[1]))
		request->dfs_region = (enum nl80211_dfs_regions) DFS_CN_REG;

	/* first check if this callback is in response to the driver callback */

	switch (request->initiator) {
	case NL80211_REGDOM_SET_BY_DRIVER:
	case NL80211_REGDOM_SET_BY_CORE:
	case NL80211_REGDOM_SET_BY_USER:

		if ((false == init_by_driver) &&
		    (false == init_by_reg_core)) {

			if (NL80211_REGDOM_SET_BY_CORE == request->initiator)
				return;
			init_by_reg_core = true;
		}

		if ((NL80211_REGDOM_SET_BY_DRIVER == request->initiator) &&
		    (true == init_by_driver)) {

			/*
			 * restore the driver regulatory flags since
			 * regulatory_hint may have
			 * changed them
			 */
			hdd_restore_reg_flags(wiphy, hdd_ctx->reg.reg_flags);
		}

		if (NL80211_REGDOM_SET_BY_CORE == request->initiator) {
			hdd_ctx->reg.cc_src = SOURCE_CORE;
			if (is_wiphy_custom_regulatory(wiphy))
				reset = true;
		} else if (NL80211_REGDOM_SET_BY_DRIVER == request->initiator) {
			hdd_ctx->reg.cc_src = SOURCE_DRIVER;
			sme_set_cc_src(hdd_ctx->hHal, SOURCE_DRIVER);
		} else {
			hdd_ctx->reg.cc_src = SOURCE_USERSPACE;
			hdd_restore_custom_reg_settings(wiphy,
							request->alpha2,
							&reset);
		}

		hdd_ctx->reg.alpha2[0] = request->alpha2[0];
		hdd_ctx->reg.alpha2[1] = request->alpha2[1];

		ret_val = hdd_update_regulatory_info(hdd_ctx);
		if (ret_val) {
			hdd_err("invalid reg info, do not process");
			return;
		}

		hdd_process_regulatory_data(hdd_ctx, wiphy, reset);

		sme_generic_change_country_code(hdd_ctx->hHal,
						hdd_ctx->reg.alpha2);

		cds_fill_and_send_ctl_to_fw(&hdd_ctx->reg);

		hdd_set_dfs_region(hdd_ctx, request->dfs_region);
		wlan_reg_get_dfs_region(hdd_ctx->hdd_pdev, &dfs_reg);

		reg_program_config_vars(hdd_ctx, &config_vars);
		ucfg_reg_set_config_vars(hdd_ctx->hdd_psoc, config_vars);
		ucfg_reg_program_mas_chan_list(hdd_ctx->hdd_psoc,
					       reg_channels,
					       hdd_ctx->reg.alpha2,
					       dfs_reg);
		break;

	default:
		break;
	}
}
#endif

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 4, 0))
static void fill_wiphy_channel(struct ieee80211_channel *wiphy_chan,
			       struct regulatory_channel *cur_chan)
{

	wiphy_chan->flags = 0;
	wiphy_chan->max_power = cur_chan->tx_power;

	if (cur_chan->chan_flags & REGULATORY_CHAN_DISABLED)
		wiphy_chan->flags  |= IEEE80211_CHAN_DISABLED;
	if (cur_chan->chan_flags & REGULATORY_CHAN_NO_IR)
		wiphy_chan->flags  |= IEEE80211_CHAN_NO_IR;
	if (cur_chan->chan_flags & REGULATORY_CHAN_RADAR)
		wiphy_chan->flags  |= IEEE80211_CHAN_RADAR;
	if (cur_chan->chan_flags & REGULATORY_CHAN_NO_OFDM)
		wiphy_chan->flags  |= IEEE80211_CHAN_NO_OFDM;
	if (cur_chan->chan_flags & REGULATORY_CHAN_INDOOR_ONLY)
		wiphy_chan->flags  |= IEEE80211_CHAN_INDOOR_ONLY;

	if (cur_chan->max_bw < 10)
		wiphy_chan->flags |= IEEE80211_CHAN_NO_10MHZ;
	if (cur_chan->max_bw < 20)
		wiphy_chan->flags |= IEEE80211_CHAN_NO_20MHZ;
	if (cur_chan->max_bw < 40)
		wiphy_chan->flags |= IEEE80211_CHAN_NO_HT40;
	if (cur_chan->max_bw < 80)
		wiphy_chan->flags |= IEEE80211_CHAN_NO_80MHZ;
	if (cur_chan->max_bw < 160)
		wiphy_chan->flags |= IEEE80211_CHAN_NO_160MHZ;
}


static void fill_wiphy_band_channels(struct wiphy *wiphy,
				     struct regulatory_channel *cur_chan_list,
				     uint8_t band_id)
{
	uint32_t wiphy_num_chan, wiphy_index;
	uint32_t chan_cnt;
	struct ieee80211_channel *wiphy_chan;

	if (wiphy->bands[band_id] == NULL)
		return;

	wiphy_num_chan = wiphy->bands[band_id]->n_channels;
	wiphy_chan = wiphy->bands[band_id]->channels;

	for (wiphy_index = 0; wiphy_index < wiphy_num_chan; wiphy_index++) {
		for (chan_cnt = 0; chan_cnt < NUM_CHANNELS; chan_cnt++) {
			if (wiphy_chan[wiphy_index].center_freq ==
			    cur_chan_list[chan_cnt].center_freq) {
				fill_wiphy_channel(&(wiphy_chan[wiphy_index]),
						   &(cur_chan_list[chan_cnt]));
				break;
			}
		}
	}
}

#ifdef FEATURE_WLAN_CH_AVOID
/**
 * hdd_ch_avoid_ind() - Avoid notified channels from FW handler
 * @adapter:	HDD adapter pointer
 * @indParam:	Channel avoid notification parameter
 *
 * Avoid channel notification from FW handler.
 * FW will send un-safe channel list to avoid over wrapping.
 * hostapd should not use notified channel
 *
 * Return: None
 */
void hdd_ch_avoid_ind(struct hdd_context *hdd_ctxt,
		struct unsafe_ch_list *unsafe_chan_list,
		struct ch_avoid_ind_type *avoid_freq_list)
{
	uint16_t *local_unsafe_list;
	uint16_t local_unsafe_list_count;

	/* Basic sanity */
	if (!hdd_ctxt) {
		hdd_err("Invalid arguments");
		return;
	}

	mutex_lock(&hdd_ctxt->avoid_freq_lock);
	qdf_mem_copy(&hdd_ctxt->coex_avoid_freq_list, avoid_freq_list,
			sizeof(struct ch_avoid_ind_type));
	mutex_unlock(&hdd_ctxt->avoid_freq_lock);

	if (hdd_clone_local_unsafe_chan(hdd_ctxt,
					&local_unsafe_list,
					&local_unsafe_list_count) != 0) {
		hdd_err("failed to clone cur unsafe chan list");
		return;
	}

	/* clear existing unsafe channel cache */
	hdd_ctxt->unsafe_channel_count = 0;
	qdf_mem_zero(hdd_ctxt->unsafe_channel_list,
					sizeof(hdd_ctxt->unsafe_channel_list));

	hdd_ctxt->unsafe_channel_count = unsafe_chan_list->ch_cnt;

	qdf_mem_copy(hdd_ctxt->unsafe_channel_list, unsafe_chan_list->ch_list,
					sizeof(hdd_ctxt->unsafe_channel_list));
	hdd_debug("number of unsafe channels is %d ",
	       hdd_ctxt->unsafe_channel_count);

	if (pld_set_wlan_unsafe_channel(hdd_ctxt->parent_dev,
					hdd_ctxt->unsafe_channel_list,
				hdd_ctxt->unsafe_channel_count)) {
		hdd_err("Failed to set unsafe channel");

		/* clear existing unsafe channel cache */
		hdd_ctxt->unsafe_channel_count = 0;
		qdf_mem_zero(hdd_ctxt->unsafe_channel_list,
			sizeof(hdd_ctxt->unsafe_channel_list));
		qdf_mem_free(local_unsafe_list);
		return;
	}

	mutex_lock(&hdd_ctxt->avoid_freq_lock);
	if (hdd_ctxt->dnbs_avoid_freq_list.ch_avoid_range_cnt)
		if (wlan_hdd_merge_avoid_freqs(avoid_freq_list,
					&hdd_ctxt->dnbs_avoid_freq_list)) {
			mutex_unlock(&hdd_ctxt->avoid_freq_lock);
			hdd_debug("unable to merge avoid freqs");
			qdf_mem_free(local_unsafe_list);
			return;
	}
	mutex_unlock(&hdd_ctxt->avoid_freq_lock);
	/*
	 * first update the unsafe channel list to the platform driver and
	 * send the avoid freq event to the application
	 */
	wlan_hdd_send_avoid_freq_event(hdd_ctxt, avoid_freq_list);

	if (!hdd_ctxt->unsafe_channel_count) {
		hdd_debug("no unsafe channels - not restarting SAP");
		qdf_mem_free(local_unsafe_list);
		return;
	}
	if (hdd_local_unsafe_channel_updated(hdd_ctxt,
					    local_unsafe_list,
					    local_unsafe_list_count))
		hdd_unsafe_channel_restart_sap(hdd_ctxt);
	qdf_mem_free(local_unsafe_list);

}
#endif

static void hdd_regulatory_dyn_cbk(struct wlan_objmgr_psoc *psoc,
				   struct wlan_objmgr_pdev *pdev,
				   struct regulatory_channel *chan_list,
				   struct avoid_freq_ind_data *avoid_freq_ind,
				   void *arg)
{
	struct wiphy *wiphy;
	struct pdev_osif_priv *pdev_priv;
	struct hdd_context *hdd_ctx;
	enum country_src cc_src;
	uint8_t alpha2[REG_ALPHA2_LEN + 1];

	pdev_priv = wlan_pdev_get_ospriv(pdev);
	wiphy = pdev_priv->wiphy;
	hdd_ctx = wiphy_priv(wiphy);

	hdd_debug("process channel list update from regulatory");

	fill_wiphy_band_channels(wiphy, chan_list, NL80211_BAND_2GHZ);
	fill_wiphy_band_channels(wiphy, chan_list, NL80211_BAND_5GHZ);

	cc_src = ucfg_reg_get_cc_and_src(hdd_ctx->hdd_psoc, alpha2);
	qdf_mem_copy(hdd_ctx->reg.alpha2, alpha2, REG_ALPHA2_LEN + 1);
	sme_set_cc_src(hdd_ctx->hHal, cc_src);

	sme_generic_change_country_code(hdd_ctx->hHal,
					hdd_ctx->reg.alpha2);
	if (avoid_freq_ind)
		hdd_ch_avoid_ind(hdd_ctx, &avoid_freq_ind->chan_list,
				&avoid_freq_ind->freq_list);
}

int hdd_regulatory_init(struct hdd_context *hdd_ctx, struct wiphy *wiphy)
{
	bool offload_enabled;
	struct reg_config_vars config_vars;
	struct regulatory_channel cur_chan_list[NUM_CHANNELS];
	enum country_src cc_src;
	uint8_t alpha2[REG_ALPHA2_LEN + 1];

	reg_program_config_vars(hdd_ctx, &config_vars);
	ucfg_reg_set_config_vars(hdd_ctx->hdd_psoc, config_vars);

	ucfg_reg_register_chan_change_callback(hdd_ctx->hdd_psoc,
					       hdd_regulatory_dyn_cbk,
					       NULL);

	wiphy->regulatory_flags |= REGULATORY_WIPHY_SELF_MANAGED;
	wiphy->reg_notifier = hdd_reg_notifier;
	offload_enabled = ucfg_reg_is_regdb_offloaded(hdd_ctx->hdd_psoc);
	if (offload_enabled) {
		hdd_ctx->reg_offload = true;
		ucfg_reg_get_current_chan_list(hdd_ctx->hdd_pdev,
					       cur_chan_list);
		fill_wiphy_band_channels(wiphy, cur_chan_list,
					 NL80211_BAND_2GHZ);
		fill_wiphy_band_channels(wiphy, cur_chan_list,
					 NL80211_BAND_5GHZ);

		cc_src = ucfg_reg_get_cc_and_src(hdd_ctx->hdd_psoc, alpha2);
		qdf_mem_copy(hdd_ctx->reg.alpha2, alpha2, REG_ALPHA2_LEN + 1);
		sme_set_cc_src(hdd_ctx->hHal, cc_src);
	} else {
		hdd_ctx->reg_offload = false;
		ucfg_reg_program_default_cc(hdd_ctx->hdd_pdev,
					    hdd_ctx->reg.reg_domain);
	}

	return 0;
}

#elif (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 14, 0))
int hdd_regulatory_init(struct hdd_context *hdd_ctx, struct wiphy *wiphy)
{
	hdd_ctx->reg_offload = false;
	wiphy->reg_notifier = hdd_reg_notifier;
	wiphy->regulatory_flags |= REGULATORY_DISABLE_BEACON_HINTS;
	wiphy->regulatory_flags |= REGULATORY_COUNTRY_IE_IGNORE;
	hdd_regulatory_init_no_offload(hdd_ctx, wiphy);

	return 0;
}

#else
int hdd_regulatory_init(struct hdd_context *hdd_ctx, struct wiphy *wiphy)
{
	hdd_ctx->reg_offload = false;
	wiphy->reg_notifier = hdd_reg_notifier;
	wiphy->flags |= WIPHY_FLAG_DISABLE_BEACON_HINTS;
	wiphy->country_ie_pref |= NL80211_COUNTRY_IE_IGNORE_CORE;
	hdd_regulatory_init_no_offload(hdd_ctx, wiphy);

	return 0;
}
#endif
