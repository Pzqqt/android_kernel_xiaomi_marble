/*
 * Copyright (c) 2017-2021 The Linux Foundation. All rights reserved.
 * Copyright (c) 2021 Qualcomm Innovation Center, Inc. All rights reserved.
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
 * DOC: reg_utils.h
 * This file provides prototypes for setting and getting regulatory variables.
 */

#ifndef __REG_UTILS_H_
#define __REG_UTILS_H_

#include <wlan_lmac_if_def.h>

#ifdef WLAN_FEATURE_DSRC
#define REG_DSRC_START_FREQ channel_map[MIN_DSRC_CHANNEL].center_freq
#define REG_DSRC_END_FREQ   channel_map[MAX_DSRC_CHANNEL].center_freq
#endif

#define REG_ETSI13_SRD_START_FREQ 5745
#define REG_ETSI13_SRD_END_FREQ   5865

/**
 * reg_is_world_ctry_code() - Check if the given country code is WORLD regdomain
 * @ctry_code: Country code value.
 *
 * Return: If country code is WORLD regdomain return true else false
 */
bool reg_is_world_ctry_code(uint16_t ctry_code);

#if defined(CONFIG_REG_CLIENT) && defined(CONFIG_CHAN_FREQ_API)
/**
 * reg_chan_has_dfs_attribute_for_freq() - check channel frequency has dfs
 * attribue or not
 * @freq: channel frequency.
 *
 * This API gets initial dfs attribute flag of the channel frequency from
 * regdomain
 *
 * Return: true if channel frequency is dfs, otherwise false
 */
bool reg_chan_has_dfs_attribute_for_freq(struct wlan_objmgr_pdev *pdev,
					 qdf_freq_t freq);
/**
 * reg_is_passive_or_disable_for_freq() - Check if the given channel is
 * passive or disabled.
 * @pdev: Pointer to physical dev
 * @chan: Channel frequency
 *
 * Return: true if channel frequency is passive or disabled, else false.
 */
bool reg_is_passive_or_disable_for_freq(struct wlan_objmgr_pdev *pdev,
					qdf_freq_t freq);
#else
static inline bool
reg_chan_has_dfs_attribute_for_freq(struct wlan_objmgr_pdev *pdev,
				    qdf_freq_t freq)
{
	return false;
}

static inline bool
reg_is_passive_or_disable_for_freq(struct wlan_objmgr_pdev *pdev,
				   qdf_freq_t freq)
{
	return false;
}
#endif /* defined(CONFIG_REG_CLIENT) && defined(CONFIG_CHAN_FREQ_API) */

#ifdef DISABLE_CHANNEL_LIST
/**
 * reg_disable_cached_channels() - Disable cached channels
 * @pdev: The physical dev to cache the channels for
 */
QDF_STATUS reg_disable_cached_channels(struct wlan_objmgr_pdev *pdev);
/**
 * reg_restore_cached_channels() - Restore disabled cached channels
 * @pdev: The physical dev to cache the channels for
 */
QDF_STATUS reg_restore_cached_channels(struct wlan_objmgr_pdev *pdev);
#else
static inline
QDF_STATUS reg_restore_cached_channels(struct wlan_objmgr_pdev *pdev)
{
	return QDF_STATUS_SUCCESS;
}

static inline
QDF_STATUS reg_disable_cached_channels(struct wlan_objmgr_pdev *pdev)
{
	return QDF_STATUS_SUCCESS;
}
#endif /* DISABLE_CHANNEL_LIST */

#if defined(DISABLE_CHANNEL_LIST) && defined(CONFIG_CHAN_FREQ_API)
/**
 * reg_cache_channel_freq_state() - Cache the current state of the channels
 * based on the channel center frequency
 * @pdev: The physical dev to cache the channels for
 * @channel_list: List of the channels for which states needs to be cached
 * @num_channels: Number of channels in the list
 *
 */
QDF_STATUS reg_cache_channel_freq_state(struct wlan_objmgr_pdev *pdev,
					uint32_t *channel_list,
					uint32_t num_channels);
#else
static inline
QDF_STATUS reg_cache_channel_freq_state(struct wlan_objmgr_pdev *pdev,
					uint32_t *channel_list,
					uint32_t num_channels)
{
	return QDF_STATUS_SUCCESS;
}
#endif /* defined(DISABLE_CHANNEL_LIST) && defined(CONFIG_CHAN_FREQ_API) */

#ifdef CONFIG_REG_CLIENT
/**
 * reg_set_band() - Sets the band information for the PDEV
 * @pdev: The physical dev to set the band for
 * @band_bitmap: The set band parameters to configure for the physical device
 *
 * Return: QDF_STATUS
 */
QDF_STATUS reg_set_band(struct wlan_objmgr_pdev *pdev, uint32_t band_bitmap);

/**
 * reg_get_band() - Get the band information for the PDEV
 * @pdev: The physical dev to get the band for
 * @band_bitmap: The band parameters of the physical device
 *
 * Return: QDF_STATUS
 */
QDF_STATUS reg_get_band(struct wlan_objmgr_pdev *pdev, uint32_t *band_bitmap);

/**
 * reg_set_fcc_constraint() - Apply fcc constraints on channels 12/13
 * @pdev: The physical dev to set the band for
 *
 * This function reduces the transmit power on channels 12 and 13, to comply
 * with FCC regulations in the USA.
 *
 * Return: QDF_STATUS
 */
QDF_STATUS reg_set_fcc_constraint(struct wlan_objmgr_pdev *pdev,
				  bool fcc_constraint);

/**
 * reg_get_fcc_constraint() - Check FCC constraint on given frequency
 * @pdev: physical dev to get
 * @freq: frequency to be checked
 *
 * Return: If FCC constraint is applied on given frequency return true
 *	   else return false.
 */
bool reg_get_fcc_constraint(struct wlan_objmgr_pdev *pdev, uint32_t freq);

/**
 * reg_read_current_country() - Get the current regulatory country
 * @psoc: The physical SoC to get current country from
 * @country_code: the buffer to populate the country code into
 *
 * Return: QDF_STATUS
 */
QDF_STATUS reg_read_current_country(struct wlan_objmgr_psoc *psoc,
				    uint8_t *country_code);

/**
 * reg_set_default_country() - Set the default regulatory country
 * @psoc: The physical SoC to set default country for
 * @req: The country information to configure
 *
 * Return: QDF_STATUS
 */
QDF_STATUS reg_set_default_country(struct wlan_objmgr_psoc *psoc,
				   uint8_t *country);

/**
 * reg_is_world_alpha2 - is reg world mode
 * @alpha2: country code pointer
 *
 * Return: true or false
 */
bool reg_is_world_alpha2(uint8_t *alpha2);

/**
 * reg_is_us_alpha2 - is US country code
 * @alpha2: country code pointer
 *
 * Return: true or false
 */
bool reg_is_us_alpha2(uint8_t *alpha2);

/**
 * reg_is_etsi_alpha2 - is country code in EU
 * @alpha2: country code pointer
 *
 * Return: true or false
 */
bool reg_is_etsi_alpha2(uint8_t *alpha2);

/**
 * reg_set_country() - Set the current regulatory country
 * @pdev: pdev device for country information
 * @country: country value
 *
 * Return: QDF_STATUS
 */
QDF_STATUS reg_set_country(struct wlan_objmgr_pdev *pdev, uint8_t *country);

/**
 * reg_reset_country() - Reset the regulatory country to default
 * @psoc: The physical SoC to reset country for
 *
 * Return: QDF_STATUS
 */
QDF_STATUS reg_reset_country(struct wlan_objmgr_psoc *psoc);

/**
 * reg_get_domain_from_country_code() - Get regdomain from country code
 * @reg_domain_ptr: Pointer to save regdomain
 * @country_alpha2: country string
 * @source: Country code source
 *
 * Return: QDF_STATUS
 */
QDF_STATUS reg_get_domain_from_country_code(v_REGDOMAIN_t *reg_domain_ptr,
					    const uint8_t *country_alpha2,
					    enum country_src source);

#ifdef CONFIG_REG_CLIENT
/**
 * reg_get_6g_power_type_for_ctry() - Return power type for 6G based on cntry IE
 * @ap_ctry: ptr to country string in country IE
 * @sta_ctry: ptr to sta programmed country
 * @pwr_type_6g: ptr to 6G power type
 * @ctry_code_match: Check for country IE and sta country code match
 * @ap_pwr_type: AP's power type as advertised in HE ops IE
 * Return: QDF_STATUS
 */
QDF_STATUS
reg_get_6g_power_type_for_ctry(struct wlan_objmgr_psoc *psoc,
			       uint8_t *ap_ctry, uint8_t *sta_ctry,
			       enum reg_6g_ap_type *pwr_type_6g,
			       bool *ctry_code_match,
			       enum reg_6g_ap_type ap_pwr_type);
#endif

/**
 * reg_set_config_vars () - set configration variables
 * @psoc: psoc ptr
 * @config_vars: configuration struct
 *
 * Return: QDF_STATUS
 */
QDF_STATUS reg_set_config_vars(struct wlan_objmgr_psoc *psoc,
			       struct reg_config_vars config_vars);

/**
 * reg_program_mas_chan_list() - Program the master channel list
 * @psoc: Pointer to psoc structure
 * @reg_channels: Pointer to reg channels
 * @alpha2: country string
 * @dfs_region: DFS region
 */
void reg_program_mas_chan_list(struct wlan_objmgr_psoc *psoc,
			       struct regulatory_channel *reg_channels,
			       uint8_t *alpha2, enum dfs_reg dfs_region);

/**
 * reg_get_regd_rules() - provides the reg domain rules info
 * @pdev: pdev pointer
 * @reg_rules: regulatory rules
 *
 * Return: QDF_STATUS
 */
QDF_STATUS reg_get_regd_rules(struct wlan_objmgr_pdev *pdev,
			      struct reg_rule_info *reg_rules);

/**
 * reg_get_cc_and_src() - Get country string and country source
 * @psoc: Pointer to psoc
 * @alpha2: Pointer to save country string
 *
 * Return: country_src
 */
enum country_src reg_get_cc_and_src(struct wlan_objmgr_psoc *psoc,
				    uint8_t *alpha2);

/**
 * reg_reset_ctry_pending_hints() - Reset all country pending hints
 * @soc_reg: regulatory private object
 *
 * Return: None
 */
void
reg_reset_ctry_pending_hints(struct wlan_regulatory_psoc_priv_obj *soc_reg);

/**
 * reg_set_curr_country() - Set current country update
 * @soc_reg: regulatory private object
 * @regulat_info: regulatory info from firmware
 * @tx_ops: send operations for regulatory component
 *
 * During SSR or restart of wlan modules after interface change timer phase,
 * this function is used to send the recent user/11d country code to firmware.
 *
 * Return: QDF_STATUS_SUCCESS if correct country is configured
 * else return failure
 * error code.
 */
QDF_STATUS reg_set_curr_country(
		struct wlan_regulatory_psoc_priv_obj *soc_reg,
		struct cur_regulatory_info *regulat_info,
		struct wlan_lmac_if_reg_tx_ops *tx_ops);

/**
 * reg_ignore_default_country() - Ignore default country update
 * @soc_reg: regulatory private object
 * @regulat_info: regulatory info from firmware
 *
 * During SSR or restart of wlan modules after interface change timer phase,
 * this function is used to ignore default country code from firmware.
 *
 * Return: If default country needs to be ignored return true else false.
 */
bool reg_ignore_default_country(struct wlan_regulatory_psoc_priv_obj *soc_reg,
				struct cur_regulatory_info *regulat_info);

#ifdef CONFIG_BAND_6GHZ
/**
 * reg_decide_6g_ap_pwr_type() - Decide which power mode AP should operate in
 *
 * @pdev: pdev ptr
 *
 * Return: AP power type
 */
enum reg_6g_ap_type reg_decide_6g_ap_pwr_type(struct wlan_objmgr_pdev *pdev);
#else
static inline enum reg_6g_ap_type
reg_decide_6g_ap_pwr_type(struct wlan_objmgr_pdev *pdev)
{
	return REG_CURRENT_MAX_AP_TYPE;
}
#endif /* CONFIG_BAND_6GHZ */
#else
static inline QDF_STATUS reg_read_current_country(struct wlan_objmgr_psoc *psoc,
						  uint8_t *country_code)
{
	return QDF_STATUS_SUCCESS;
}

static inline bool reg_is_world_alpha2(uint8_t *alpha2)
{
	return false;
}

static inline bool reg_is_us_alpha2(uint8_t *alpha2)
{
	return false;
}

static inline bool reg_is_etsi_alpha2(uint8_t *alpha2)
{
	return false;
}

static inline QDF_STATUS reg_set_country(struct wlan_objmgr_pdev *pdev,
					 uint8_t *country)
{
	return QDF_STATUS_SUCCESS;
}

static inline QDF_STATUS reg_reset_country(struct wlan_objmgr_psoc *psoc)
{
	return QDF_STATUS_SUCCESS;
}

static inline QDF_STATUS reg_get_domain_from_country_code(
	v_REGDOMAIN_t *reg_domain_ptr, const uint8_t *country_alpha2,
	enum country_src source)
{
	return QDF_STATUS_SUCCESS;
}

static inline QDF_STATUS reg_set_config_vars(struct wlan_objmgr_psoc *psoc,
					     struct reg_config_vars config_vars)
{
	return QDF_STATUS_SUCCESS;
}

static inline void reg_program_mas_chan_list(
	struct wlan_objmgr_psoc *psoc,
	struct regulatory_channel *reg_channels,
	uint8_t *alpha2, enum dfs_reg dfs_region)
{
}

static inline QDF_STATUS reg_get_regd_rules(struct wlan_objmgr_pdev *pdev,
					    struct reg_rule_info *reg_rules)
{
	return QDF_STATUS_SUCCESS;
}

static inline enum country_src reg_get_cc_and_src(struct wlan_objmgr_psoc *psoc,
						  uint8_t *alpha2)
{
		return SOURCE_UNKNOWN;
}

static inline void
reg_reset_ctry_pending_hints(struct wlan_regulatory_psoc_priv_obj *soc_reg)
{
}

static inline QDF_STATUS reg_set_curr_country(
		struct wlan_regulatory_psoc_priv_obj *soc_reg,
		struct cur_regulatory_info *regulat_info,
		struct wlan_lmac_if_reg_tx_ops *tx_ops)
{
	return QDF_STATUS_SUCCESS;
}

static inline
bool reg_ignore_default_country(struct wlan_regulatory_psoc_priv_obj *soc_reg,
				struct cur_regulatory_info *regulat_info)
{
	return false;
}

static inline
QDF_STATUS reg_set_fcc_constraint(struct wlan_objmgr_pdev *pdev,
				  bool fcc_constraint)
{
	return QDF_STATUS_SUCCESS;
}

static inline
bool reg_get_fcc_constraint(struct wlan_objmgr_pdev *pdev, uint32_t freq)
{
	return false;
}

static inline enum reg_6g_ap_type
reg_decide_6g_ap_pwr_type(struct wlan_objmgr_pdev *pdev)
{
	return REG_CURRENT_MAX_AP_TYPE;
}
#endif /* CONFIG_REG_CLIENT */

#if defined(WLAN_FEATURE_DSRC) && defined(CONFIG_REG_CLIENT)
/**
 * reg_is_dsrc_freq () - Checks the channel frequency is DSRC or not
 * @freq: Channel center frequency
 * @pdev: pdev ptr
 *
 * Return: true or false
 */
#ifdef CONFIG_CHAN_FREQ_API
bool reg_is_dsrc_freq(qdf_freq_t freq);
#endif /* CONFIG_CHAN_FREQ_API*/

static inline bool reg_is_etsi13_regdmn(struct wlan_objmgr_pdev *pdev)
{
	return false;
}

/**
 * reg_is_etsi13_srd_chan_for_freq() - Checks the channel for ETSI13 srd ch
 * frequency or not
 * @freq: Channel center frequency
 * @pdev: pdev ptr
 *
 * Return: true or false
 */
static inline bool
reg_is_etsi13_srd_chan_for_freq(struct wlan_objmgr_pdev *pdev, uint16_t freq)
{
	return false;
}

static inline bool
reg_is_etsi13_srd_chan_allowed_master_mode(struct wlan_objmgr_pdev *pdev)
{
	return true;
}
#elif defined(CONFIG_REG_CLIENT)
static inline bool reg_is_dsrc_freq(qdf_freq_t freq)
{
	return false;
}

#ifdef CONFIG_CHAN_FREQ_API
bool reg_is_etsi13_srd_chan_for_freq(struct wlan_objmgr_pdev
				     *pdev, uint16_t freq);
#endif /*CONFIG_CHAN_FREQ_API */

/**
 * reg_is_etsi13_regdmn () - Checks if the current reg domain is ETSI13 or not
 * @pdev: pdev ptr
 *
 * Return: true or false
 */
bool reg_is_etsi13_regdmn(struct wlan_objmgr_pdev *pdev);

/**
 * reg_is_etsi13_srd_chan_allowed_master_mode() - Checks if regdmn is ETSI13
 * and SRD channels are allowed in master mode or not.
 *
 * @pdev: pdev ptr
 *
 * Return: true or false
 */
bool reg_is_etsi13_srd_chan_allowed_master_mode(struct wlan_objmgr_pdev *pdev);
#else
static inline bool reg_is_dsrc_freq(qdf_freq_t freq)
{
	return false;
}

static inline
bool reg_is_etsi13_srd_chan_for_freq(struct wlan_objmgr_pdev *pdev,
				     uint16_t freq)
{
	return false;
}

static inline bool reg_is_etsi13_regdmn(struct wlan_objmgr_pdev *pdev)
{
	return false;
}

static inline bool
reg_is_etsi13_srd_chan_allowed_master_mode(struct wlan_objmgr_pdev *pdev)
{
	return false;
}

#endif

#endif
