/*
 * Copyright (c) 2017-2021 The Linux Foundation. All rights reserved.
 * Copyright (c) 2021-2024 Qualcomm Innovation Center, Inc. All rights reserved.
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

#include <reg_services_public_struct.h>

/**
 * wlan_reg_max_5ghz_ch_num() - Get maximum 5GHz channel number
 *
 * Return: Maximum 5GHz channel number
 */
#define WLAN_REG_MAX_5GHZ_CH_NUM wlan_reg_max_5ghz_ch_num()
uint8_t wlan_reg_max_5ghz_ch_num(void);

#ifdef CONFIG_CHAN_FREQ_API
/**
 * wlan_reg_min_24ghz_chan_freq() - Get minimum 2.4GHz channel frequency
 *
 * Return: Minimum 2.4GHz channel frequency
 */
#define WLAN_REG_MIN_24GHZ_CHAN_FREQ wlan_reg_min_24ghz_chan_freq()
qdf_freq_t wlan_reg_min_24ghz_chan_freq(void);

/**
 * wlan_reg_max_24ghz_chan_freq() - Get maximum 2.4GHz channel frequency
 *
 * Return: Maximum 2.4GHz channel frequency
 */
#define WLAN_REG_MAX_24GHZ_CHAN_FREQ wlan_reg_max_24ghz_chan_freq()
qdf_freq_t wlan_reg_max_24ghz_chan_freq(void);

/**
 * wlan_reg_min_5ghz_chan_freq() - Get minimum 5GHz channel frequency
 *
 * Return: Minimum 5GHz channel frequency
 */
#define WLAN_REG_MIN_5GHZ_CHAN_FREQ wlan_reg_min_5ghz_chan_freq()
qdf_freq_t wlan_reg_min_5ghz_chan_freq(void);

/**
 * wlan_reg_max_5ghz_chan_freq() - Get maximum 5GHz channel frequency
 *
 * Return: Maximum 5GHz channel frequency
 */
#define WLAN_REG_MAX_5GHZ_CHAN_FREQ wlan_reg_max_5ghz_chan_freq()
qdf_freq_t wlan_reg_max_5ghz_chan_freq(void);
#endif /* CONFIG_CHAN_FREQ_API */

/**
 * wlan_reg_is_24ghz_ch_freq() - Check if the given channel frequency is 2.4GHz
 * @freq: Channel frequency
 *
 * Return: true if channel frequency is 2.4GHz, else false
 */
#define WLAN_REG_IS_24GHZ_CH_FREQ(freq) wlan_reg_is_24ghz_ch_freq(freq)
bool wlan_reg_is_24ghz_ch_freq(qdf_freq_t freq);

/**
 * wlan_reg_is_5ghz_ch_freq() - Check if the given channel frequency is 5GHz
 * @freq: Channel frequency
 *
 * Return: true if channel frequency is 5GHz, else false
 */
#define WLAN_REG_IS_5GHZ_CH_FREQ(freq) wlan_reg_is_5ghz_ch_freq(freq)
bool wlan_reg_is_5ghz_ch_freq(qdf_freq_t freq);

/**
 * wlan_reg_is_range_overlap_2g() - Check if the given low_freq and high_freq
 * is in the 2G range.
 *
 * @low_freq - Low frequency.
 * @high_freq - High frequency.
 *
 * Return: Return true if given low_freq and high_freq overlaps 2G range,
 * else false.
 */
bool wlan_reg_is_range_overlap_2g(qdf_freq_t low_freq, qdf_freq_t high_freq);

/**
 * wlan_reg_is_range_overlap_5g() - Check if the given low_freq and high_freq
 * is in the 5G range.
 *
 * @low_freq - Low frequency.
 * @high_freq - High frequency.
 *
 * Return: Return true if given low_freq and high_freq overlaps 5G range,
 * else false.
 */
bool wlan_reg_is_range_overlap_5g(qdf_freq_t low_freq, qdf_freq_t high_freq);

/**
 * wlan_reg_is_freq_indoor() - Check if a frequency is indoor.
 * @pdev: Pointer to pdev.
 * @freq: Channel frequency.
 *
 * Return: Return true if a frequency is indoor, else false.
 */
bool wlan_reg_is_freq_indoor(struct wlan_objmgr_pdev *pdev, qdf_freq_t freq);

/**
 * wlan_reg_get_next_lower_bandwidth() - Get next lower bandwdith
 * @ch_width: channel bandwdith
 *
 * Return: Return next lower bandwidth of input channel bandwidth
 */
enum phy_ch_width
wlan_reg_get_next_lower_bandwidth(enum phy_ch_width ch_width);

#ifdef CONFIG_REG_CLIENT
/**
 * wlan_reg_is_freq_indoor_in_secondary_list() - Check if the input frequency is
 * an indoor frequency in the secondary list
 * @pdev: Pointer to pdev.
 * @freq: Channel frequency.
 *
 * Return: Return true if a frequency is indoor, else false.
 */
bool wlan_reg_is_freq_indoor_in_secondary_list(struct wlan_objmgr_pdev *pdev,
					       qdf_freq_t freq);
#endif

#ifdef CONFIG_BAND_6GHZ
/**
 * wlan_reg_is_6ghz_chan_freq() - Check if the given channel frequency is 6GHz
 * @freq: Channel frequency
 *
 * Return: true if channel frequency is 6GHz, else false
 */
#define WLAN_REG_IS_6GHZ_CHAN_FREQ(freq) wlan_reg_is_6ghz_chan_freq(freq)
bool wlan_reg_is_6ghz_chan_freq(uint16_t freq);

#ifdef CONFIG_6G_FREQ_OVERLAP
/**
 * wlan_reg_is_range_only6g() - Check if the given low_freq and high_freq
 * is in the 6G range.
 * @low_freq - Low frequency.
 * @high_freq - High frequency.
 *
 * Return: Return true if given low_freq and high_freq overlaps 6G range,
 * else false.
 */
bool wlan_reg_is_range_only6g(qdf_freq_t low_freq, qdf_freq_t high_freq);

/**
 * wlan_reg_is_range_overlap_6g() - Check if the given low_freq and high_freq
 * is in the 6G range.
 *
 * @low_freq - Low frequency.
 * @high_freq - High frequency.
 *
 * Return: Return true if given low_freq and high_freq overlaps 6G range,
 * else false.
 */
bool wlan_reg_is_range_overlap_6g(qdf_freq_t low_freq, qdf_freq_t high_freq);
#else
static inline bool wlan_reg_is_range_only6g(qdf_freq_t low_freq,
					    qdf_freq_t high_freq)
{
	return false;
}

static inline bool wlan_reg_is_range_overlap_6g(qdf_freq_t low_freq,
						qdf_freq_t high_freq)
{
	return false;
}
#endif

/**
 * wlan_reg_get_6g_ap_master_chan_list() - provide  the appropriate ap master
 * channel list
 * @pdev: pdev pointer
 * @ap_pwr_type: The ap power type (LPI/VLP/SP)
 * @chan_list: channel list pointer
 *
 * Return: QDF_STATUS
 */
QDF_STATUS wlan_reg_get_6g_ap_master_chan_list(
					struct wlan_objmgr_pdev *pdev,
					enum reg_6g_ap_type ap_pwr_type,
					struct regulatory_channel *chan_list);

#ifdef CONFIG_REG_CLIENT
/**
 * wlan_reg_get_power_string () - wlan reg get power type string
 * @power_type: power type enum
 *
 * Return: power type string
 */
const char *wlan_reg_get_power_string(enum reg_6g_ap_type power_type);
#endif

/**
 * wlan_reg_is_6ghz_psc_chan_freq() - Check if the given 6GHz channel frequency
 * is preferred scanning channel frequency.
 * @freq: Channel frequency
 *
 * Return: true if given 6GHz channel frequency is preferred scanning channel
 * frequency, else false
 */
#define WLAN_REG_IS_6GHZ_PSC_CHAN_FREQ(freq) \
	wlan_reg_is_6ghz_psc_chan_freq(freq)
bool wlan_reg_is_6ghz_psc_chan_freq(uint16_t freq);

/**
 * wlan_reg_min_6ghz_chan_freq() - Get minimum 6GHz channel center frequency
 *
 * Return: Minimum 6GHz channel center frequency
 */
#define WLAN_REG_MIN_6GHZ_CHAN_FREQ wlan_reg_min_6ghz_chan_freq()
uint16_t wlan_reg_min_6ghz_chan_freq(void);

/**
 * wlan_reg_max_6ghz_chan_freq() - Get maximum 6GHz channel center frequency
 *
 * Return: Maximum 6GHz channel center frequency
 */
#define WLAN_REG_MAX_6GHZ_CHAN_FREQ wlan_reg_max_6ghz_chan_freq()
uint16_t wlan_reg_max_6ghz_chan_freq(void);

/**
 * wlan_reg_is_6g_freq_indoor() - Check if a 6GHz frequency is indoor.
 * @pdev: Pointer to pdev.
 * @freq: Channel frequency.
 *
 * Return: Return true if a 6GHz frequency is indoor, else false.
 */
#define WLAN_REG_IS_6GHZ_FREQ_INDOOR(pdev, freq) \
					wlan_reg_is_6g_freq_indoor(pdev, freq)
bool wlan_reg_is_6g_freq_indoor(struct wlan_objmgr_pdev *pdev, qdf_freq_t freq);

/**
 * wlan_reg_get_max_txpower_for_6g_tpe() - Get max txpower for 6G TPE IE.
 * @pdev: Pointer to pdev.
 * @freq: Channel frequency.
 * @bw: Channel bandwidth.
 * @reg_ap: Regulatory 6G AP type.
 * @reg_client: Regulatory client type.
 * @is_psd: True if txpower is needed in PSD format, and false if needed in EIRP
 * format.
 * @tx_power: Pointer to tx-power.
 *
 * Return: Return QDF_STATUS_SUCCESS, if tx_power is filled for 6G TPE IE
 * else return QDF_STATUS_E_FAILURE.
 */
QDF_STATUS
wlan_reg_get_max_txpower_for_6g_tpe(struct wlan_objmgr_pdev *pdev,
				    qdf_freq_t freq, uint8_t bw,
				    enum reg_6g_ap_type reg_ap,
				    enum reg_6g_client_type reg_client,
				    bool is_psd,
				    uint8_t *tx_power);

#else

#define WLAN_REG_IS_6GHZ_CHAN_FREQ(freq) (false)
static inline bool wlan_reg_is_6ghz_chan_freq(uint16_t freq)
{
	return false;
}

static inline bool wlan_reg_is_range_only6g(qdf_freq_t low_freq,
					    qdf_freq_t high_freq)
{
	return false;
}

#define WLAN_REG_IS_6GHZ_PSC_CHAN_FREQ(freq) (false)
static inline bool wlan_reg_is_6ghz_psc_chan_freq(uint16_t freq)
{
	return false;
}

#define WLAN_REG_MIN_6GHZ_CHAN_FREQ (false)
static inline uint16_t wlan_reg_min_6ghz_chan_freq(void)
{
	return 0;
}

#define WLAN_REG_MAX_6GHZ_CHAN_FREQ (false)
static inline uint16_t wlan_reg_max_6ghz_chan_freq(void)
{
	return 0;
}

#define WLAN_REG_IS_6GHZ_FREQ_INDOOR(pdev, freq) (false)
static inline bool
wlan_reg_is_6g_freq_indoor(struct wlan_objmgr_pdev *pdev, qdf_freq_t freq)
{
	return false;
}

static inline bool wlan_reg_is_range_overlap_6g(qdf_freq_t low_freq,
						qdf_freq_t high_freq)
{
	return false;
}

static inline QDF_STATUS
wlan_reg_get_max_txpower_for_6g_tpe(struct wlan_objmgr_pdev *pdev,
				    qdf_freq_t freq, uint8_t bw,
				    enum reg_6g_ap_type reg_ap,
				    enum reg_6g_client_type reg_client,
				    bool is_psd,
				    uint8_t *tx_power)
{
	return QDF_STATUS_E_FAILURE;
}

static inline QDF_STATUS
wlan_reg_get_6g_ap_master_chan_list(struct wlan_objmgr_pdev *pdev,
				    enum reg_6g_ap_type ap_pwr_type,
				    struct regulatory_channel *chan_list)
{
	return QDF_STATUS_E_FAILURE;
}

static inline
const char *wlan_reg_get_power_string(enum reg_6g_ap_type power_type)
{
	return "INVALID";
}
#endif /* CONFIG_BAND_6GHZ */

/**
 * wlan_reg_get_band_channel_list() - Get channel list based on the band_mask
 * @pdev: pdev ptr
 * @band_mask: Input bitmap with band set
 * @channel_list: Pointer to Channel List
 *
 * Get the given channel list and number of channels from the current channel
 * list based on input band bitmap.
 *
 * Return: Number of channels, else 0 to indicate error
 */
uint16_t
wlan_reg_get_band_channel_list(struct wlan_objmgr_pdev *pdev,
			       uint8_t band_mask,
			       struct regulatory_channel *channel_list);

#ifdef CONFIG_REG_CLIENT
/**
 * wlan_reg_get_secondary_band_channel_list() - Get secondary channel list for
 * SAP based on the band_mask
 * @pdev: pdev ptr
 * @band_mask: Input bitmap with band set
 * @channel_list: Pointer to Channel List
 *
 * Get the given channel list and number of channels from the secondary current
 * channel list based on input band bitmap.
 *
 * Return: Number of channels, else 0 to indicate error
 */
uint16_t
wlan_reg_get_secondary_band_channel_list(struct wlan_objmgr_pdev *pdev,
					 uint8_t band_mask,
					 struct regulatory_channel
					 *channel_list);
#endif

/**
 * wlan_reg_chan_band_to_freq - Return channel frequency based on the channel
 * number and band.
 * @pdev: pdev ptr
 * @chan: Channel Number
 * @band_mask: Bitmap for bands
 *
 * Return: Return channel frequency or return 0, if the channel is disabled or
 * if the input channel number or band_mask is invalid. Composite bands are
 * supported only for 2.4Ghz and 5Ghz bands. For other bands the following
 * priority is given: 1) 6Ghz 2) 5Ghz 3) 2.4Ghz.
 */
qdf_freq_t wlan_reg_chan_band_to_freq(struct wlan_objmgr_pdev *pdev,
				      uint8_t chan,
				      uint8_t band_mask);

#ifdef CONFIG_49GHZ_CHAN
/**
 * wlan_reg_is_49ghz_freq() - Check if the given channel frequency is 4.9GHz
 * @freq: Channel frequency
 *
 * Return: true if channel frequency is 4.9GHz, else false
 */
#define WLAN_REG_IS_49GHZ_FREQ(freq) wlan_reg_is_49ghz_freq(freq)
bool wlan_reg_is_49ghz_freq(qdf_freq_t freq);

#else

#define WLAN_REG_IS_49GHZ_FREQ(freq) (false)
static inline bool wlan_reg_is_49ghz_freq(qdf_freq_t freq)
{
	return false;
}
#endif /* CONFIG_49GHZ_CHAN */

/**
 * wlan_reg_ch_num() - Get channel number from channel enum
 * @ch_enum: Channel enum
 *
 * Return: channel number
 */
#define WLAN_REG_CH_NUM(ch_enum) wlan_reg_ch_num(ch_enum)
uint8_t wlan_reg_ch_num(uint32_t ch_enum);

/**
 * wlan_reg_ch_to_freq() - Get channel frequency from channel enum
 * @ch_enum: Channel enum
 *
 * Return: channel frequency
 */
#define WLAN_REG_CH_TO_FREQ(ch_enum) wlan_reg_ch_to_freq(ch_enum)
qdf_freq_t wlan_reg_ch_to_freq(uint32_t ch_enum);

/**
 * wlan_reg_read_default_country() - Read the default country for the regdomain
 * @country: pointer to the country code.
 *
 * Return: QDF_STATUS
 */
QDF_STATUS wlan_reg_read_default_country(struct wlan_objmgr_psoc *psoc,
				   uint8_t *country);

/**
 * wlan_reg_get_ctry_idx_max_bw_from_country_code() - Get the max 5G
 * bandwidth from country code
 * @cc : Country Code
 * @max_bw_5g : Max 5G bandwidth supported by the country
 *
 * Return : QDF_STATUS
 */

QDF_STATUS wlan_reg_get_max_5g_bw_from_country_code(uint16_t cc,
						    uint16_t *max_bw_5g);

/**
 * wlan_reg_get_max_5g_bw_from_regdomain() - Get the max 5G bandwidth
 * supported by the regdomain
 * @orig_regdmn : Regdomain Pair value
 * @max_bw_5g : Max 5G bandwidth supported by the country
 *
 * Return : QDF_STATUS
 */
QDF_STATUS wlan_reg_get_max_5g_bw_from_regdomain(uint16_t regdmn,
						 uint16_t *max_bw_5g);

/**
 * wlan_reg_get_fcc_constraint() - Check FCC constraint on given frequency
 * @pdev: physical dev to get
 * @freq: frequency to be checked
 *
 * Return: If FCC constraint is on applied given frequency return true
 *	   else return false.
 */
bool wlan_reg_get_fcc_constraint(struct wlan_objmgr_pdev *pdev, uint32_t freq);

#ifdef CONFIG_REG_CLIENT
/**
 * wlan_reg_read_current_country() - Read the current country for the regdomain
 * @country: pointer to the country code.
 *
 * Return: QDF_STATUS
 */
QDF_STATUS wlan_reg_read_current_country(struct wlan_objmgr_psoc *psoc,
				   uint8_t *country);

#ifdef CONFIG_REG_CLIENT
/**
 * wlan_reg_get_best_6g_power_type() - Return best power type for 6GHz
 * connection
 * @psoc: pointer to psoc
 * @pdev: pointer to pdev
 * @pwr_type_6g: pointer to 6G power type
 * @ap_pwr_type: AP's power type for 6G as advertised in HE ops IEa
 * @chan_freq: Connection channel frequency
 * Return: QDF_STATUS
 */
QDF_STATUS
wlan_reg_get_best_6g_power_type(struct wlan_objmgr_psoc *psoc,
				struct wlan_objmgr_pdev *pdev,
				enum reg_6g_ap_type *pwr_type_6g,
				enum reg_6g_ap_type ap_pwr_type,
				uint32_t chan_freq);
#endif

#ifdef CONFIG_CHAN_FREQ_API
/**
 * wlan_reg_is_etsi13_srd_chan_for_freq () - Checks if the ch is ETSI13 srd ch
 * or not
 * @pdev: pdev ptr
 * @freq: channel center frequency
 *
 * Return: true or false
 */
bool wlan_reg_is_etsi13_srd_chan_for_freq(struct wlan_objmgr_pdev *pdev,
					  qdf_freq_t freq);
#endif /*CONFIG_CHAN_FREQ_API*/

/**
 * wlan_reg_is_etsi13_regdmn() - Checks if current reg domain is ETSI13 or not
 * @pdev: pdev ptr
 *
 * Return: true or false
 */
bool wlan_reg_is_etsi13_regdmn(struct wlan_objmgr_pdev *pdev);

/**
 * wlan_reg_is_etsi13_srd_chan_allowed_master_mode() - Checks if regdmn is
 * ETSI13 and SRD channels are allowed in master mode or not.
 *
 * @pdev: pdev ptr
 *
 * Return: true or false
 */
bool wlan_reg_is_etsi13_srd_chan_allowed_master_mode(struct wlan_objmgr_pdev
						     *pdev);
#endif

/**
 * wlan_reg_is_world() - reg is world mode
 * @country: The country information
 *
 * Return: true or false
 */
bool wlan_reg_is_world(uint8_t *country);

/**
 * wlan_reg_get_dfs_region () - Get the current dfs region
 * @dfs_reg: pointer to dfs region
 *
 * Return: Status
 */
QDF_STATUS wlan_reg_get_dfs_region(struct wlan_objmgr_pdev *pdev,
			     enum dfs_reg *dfs_reg);

/**
 * wlan_reg_is_chan_disabled_and_not_nol() - In the regulatory channel list, a
 * channel may be disabled by the regulatory/device or by radar. Radar is
 * temporary and a radar disabled channel does not mean that the channel is
 * permanently disabled. The API checks if the channel is disabled, but not due
 * to radar.
 * @chan - Regulatory channel object
 *
 * Return - True,  the channel is disabled, but not due to radar, else false.
 */
bool wlan_reg_is_chan_disabled_and_not_nol(struct regulatory_channel *chan);

/**
 * wlan_reg_get_current_chan_list() - provide the pdev current channel list
 * @pdev: pdev pointer
 * @chan_list: channel list pointer
 *
 * Return: QDF_STATUS
 */
QDF_STATUS wlan_reg_get_current_chan_list(struct wlan_objmgr_pdev *pdev,
		struct regulatory_channel *chan_list);

#ifdef CONFIG_REG_CLIENT
/**
 * wlan_reg_get_secondary_current_chan_list() - provide the pdev secondary
 * current channel list
 * @pdev: pdev pointer
 * @chan_list: channel list pointer
 *
 * Return: QDF_STATUS
 */
QDF_STATUS wlan_reg_get_secondary_current_chan_list(
					struct wlan_objmgr_pdev *pdev,
					struct regulatory_channel *chan_list);
#endif

#if defined(CONFIG_AFC_SUPPORT) && defined(CONFIG_BAND_6GHZ)
/**
 * wlan_reg_get_6g_afc_chan_list() - provide the pdev afc channel list
 * @pdev: pdev pointer
 * @chan_list: channel list pointer
 *
 * Return: QDF_STATUS
 */
QDF_STATUS wlan_reg_get_6g_afc_chan_list(struct wlan_objmgr_pdev *pdev,
					 struct regulatory_channel *chan_list);

/**
 * wlan_reg_get_6g_afc_mas_chan_list() - provide the pdev afc master channel
 * list
 * @pdev: pdev pointer
 * @chan_list: channel list pointer
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
wlan_reg_get_6g_afc_mas_chan_list(struct wlan_objmgr_pdev *pdev,
				  struct regulatory_channel *chan_list);

/**
 * wlan_reg_psd_2_eirp() - Calculate EIRP from PSD and bandwidth
 * channel list
 * @pdev: pdev pointer
 * @psd: Power Spectral Density in dBm/MHz
 * @ch_bw: Bandwidth of a channel in MHz (20/40/80/160/320 etc)
 * @eirp:  EIRP power  in dBm
 *
 * Return: QDF_STATUS
 */
QDF_STATUS wlan_reg_psd_2_eirp(struct wlan_objmgr_pdev *pdev,
			       int16_t psd,
			       uint16_t ch_bw,
			       int16_t *eirp);

/**
 * wlan_reg_is_afc_power_event_received() - Checks if AFC power event is
 * received from the FW.
 *
 * @pdev: pdev ptr
 *
 * Return: true if AFC power event is received from the FW or false otherwise
 */
bool wlan_reg_is_afc_power_event_received(struct wlan_objmgr_pdev *pdev);

/**
 * wlan_reg_get_afc_req_id() - Get the AFC request ID
 * @pdev: pdev pointer
 * @req_id: Pointer to request id
 *
 * Return: QDF_STATUS
 */
QDF_STATUS wlan_reg_get_afc_req_id(struct wlan_objmgr_pdev *pdev,
				   uint64_t *req_id);

/**
 * wlan_reg_is_afc_expiry_event_received() - Checks if AFC power event is
 * received from the FW.
 *
 * @pdev: pdev ptr
 *
 * Return: true if AFC exipry event is received from the FW or false otherwise
 */
bool wlan_reg_is_afc_expiry_event_received(struct wlan_objmgr_pdev *pdev);

/**
 * wlan_reg_is_noaction_on_afc_pwr_evt() - Checks whether driver needs to
 * take action for AFC action or the response should be handled by the
 * user application.
 *
 * @pdev: pdev ptr
 *
 * Return: true if driver need not take action for AFC resp, false otherwise.
 */
bool
wlan_reg_is_noaction_on_afc_pwr_evt(struct wlan_objmgr_pdev *pdev);
#else
static inline bool
wlan_reg_is_afc_power_event_received(struct wlan_objmgr_pdev *pdev)
{
	return false;
}

static inline QDF_STATUS
wlan_reg_get_6g_afc_chan_list(struct wlan_objmgr_pdev *pdev,
			      struct regulatory_channel *chan_list)
{
	return QDF_STATUS_E_NOSUPPORT;
}
#endif

/**
 * wlan_reg_get_bonded_channel_state_for_freq() - Get bonded channel freq state
 * @freq: channel frequency
 * @bw: channel band width
 * @sec_freq: secondary frequency
 *
 * Return: channel state
 */
enum channel_state
wlan_reg_get_bonded_channel_state_for_freq(struct wlan_objmgr_pdev *pdev,
					   qdf_freq_t freq,
					   enum phy_ch_width bw,
					   qdf_freq_t sec_freq);

/**
 * wlan_reg_set_dfs_region() - set the dfs region
 * @pdev: pdev ptr
 * @dfs_reg: dfs region
 *
 * Return: void
 */
void wlan_reg_set_dfs_region(struct wlan_objmgr_pdev *pdev,
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
 * wlan_reg_get_opclass_from_freq_width() - Get operating class from frequency
 * @country: Country code.
 * @freq: Channel center frequency.
 * @ch_width: Channel width.
 * @behav_limit: Behaviour limit.
 *
 * Return: Error code.
 */
uint8_t wlan_reg_get_opclass_from_freq_width(uint8_t *country,
					     qdf_freq_t freq,
					     uint16_t ch_width,
					     uint16_t behav_limit);

/**
 * wlan_reg_get_band_cap_from_op_class() - Return band capability bitmap
 * @country: Pointer to Country code.
 * @num_of_opclass: Number of Operating class.
 * @opclass: Pointer to opclass.
 *
 * Return supported band bitmap based on the input operating class list
 * provided.
 *
 * Return: Return supported band capability
 */
uint8_t wlan_reg_get_band_cap_from_op_class(const uint8_t *country,
					    uint8_t num_of_opclass,
					    const uint8_t *opclass);

/**
 * wlan_reg_dmn_print_channels_in_opclass() - Print channels in op-class
 * @country: country alpha2
 * @opclass: oplcass
 *
 * Return: void
 */
void wlan_reg_dmn_print_channels_in_opclass(uint8_t *country,
					    uint8_t opclass);


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
 * wlan_reg_dmn_get_chanwidth_from_opclass_auto() - get channel width from
 * operating class. If opclass not found then search in global opclass.
 * @country: country alpha2
 * @channel: channel number
 * @opclass: operating class
 *
 * Return: int
 */
uint16_t wlan_reg_dmn_get_chanwidth_from_opclass_auto(uint8_t *country,
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
 * wlan_reg_get_opclass_details() - Get details about the current opclass table.
 * @pdev: Pointer to pdev.
 * @reg_ap_cap: Pointer to reg_ap_cap.
 * @n_opclasses: Pointer to number of opclasses.
 * @max_supp_op_class: Maximum number of operating classes supported.
 * @global_tbl_lookup: Whether to lookup global op class tbl.
 *
 * Return: QDF_STATUS_SUCCESS if success, else return QDF_STATUS_FAILURE.
 */
QDF_STATUS
wlan_reg_get_opclass_details(struct wlan_objmgr_pdev *pdev,
			     struct regdmn_ap_cap_opclass_t *reg_ap_cap,
			     uint8_t *n_opclasses,
			     uint8_t max_supp_op_class,
			     bool global_tbl_lookup);

/**
 * wlan_reg_get_cc_and_src () - get country code and src
 * @psoc: psoc ptr
 * @alpha2: country code alpha2
 *
 * Return: country_src
 */
enum country_src wlan_reg_get_cc_and_src(struct wlan_objmgr_psoc *psoc,
					 uint8_t *alpha);

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
 * regulatory_pdev_open() - Open regulatory component
 * @pdev: Pointer to pdev structure
 *
 * Return: Success or Failure
 */
QDF_STATUS regulatory_pdev_open(struct wlan_objmgr_pdev *pdev);

/**
 * regulatory_pdev_close() - Close regulatory component
 * @pdev: Pointer to pdev structure.
 *
 * Return: Success or Failure
 */
QDF_STATUS regulatory_pdev_close(struct wlan_objmgr_pdev *pdev);

/**
 * wlan_reg_freq_to_chan () - convert channel freq to channel number
 * @pdev: The physical dev to set current country for
 * @freq: frequency
 *
 * Return: true or false
 */
uint8_t wlan_reg_freq_to_chan(struct wlan_objmgr_pdev *pdev,
			      qdf_freq_t freq);

/**
 * wlan_reg_legacy_chan_to_freq () - convert chan to freq, for 2G and 5G
 * @chan: channel number
 *
 * Return: frequency
 */
qdf_freq_t wlan_reg_legacy_chan_to_freq(struct wlan_objmgr_pdev *pdev,
					uint8_t chan);

/**
 * wlan_reg_is_us() - reg is us country
 * @country: The country information
 *
 * Return: true or false
 */
bool wlan_reg_is_us(uint8_t *country);

/**
 * wlan_reg_is_etsi() - reg is a country in EU
 * @country: The country information
 *
 * Return: true or false
 */
bool wlan_reg_is_etsi(uint8_t *country);


/**
 * wlan_reg_ctry_support_vlp() - Country supports VLP or not
 * @country: The country information
 *
 * Return: true or false
 */
bool wlan_reg_ctry_support_vlp(uint8_t *country);

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
 * wlan_reg_set_11d_country() - Set the 11d regulatory country
 * @pdev: The physical dev to set current country for
 * @country: The country information to configure
 *
 * Return: QDF_STATUS
 */
QDF_STATUS wlan_reg_set_11d_country(struct wlan_objmgr_pdev *pdev,
				    uint8_t *country);

/**
 * wlan_reg_register_chan_change_callback () - add chan change cbk
 * @psoc: psoc ptr
 * @cbk: callback
 * @arg: argument
 *
 * Return: true or false
 */
void wlan_reg_register_chan_change_callback(struct wlan_objmgr_psoc *psoc,
					    void *cbk, void *arg);

/**
 * wlan_reg_unregister_chan_change_callback () - remove chan change cbk
 * @psoc: psoc ptr
 * @cbk:callback
 *
 * Return: true or false
 */
void wlan_reg_unregister_chan_change_callback(struct wlan_objmgr_psoc *psoc,
					      void *cbk);

/**
 * wlan_reg_is_11d_offloaded() - 11d offloaded supported
 * @psoc: psoc ptr
 *
 * Return: bool
 */
bool wlan_reg_is_11d_offloaded(struct wlan_objmgr_psoc *psoc);

/**
 * wlan_reg_11d_enabled_on_host() - 11d enabled don host
 * @psoc: psoc ptr
 *
 * Return: bool
 */
bool wlan_reg_11d_enabled_on_host(struct wlan_objmgr_psoc *psoc);

/**
 * wlan_reg_get_chip_mode() - get supported chip mode
 * @pdev: pdev pointer
 * @chip_mode: chip mode
 *
 * Return: QDF STATUS
 */
QDF_STATUS wlan_reg_get_chip_mode(struct wlan_objmgr_pdev *pdev,
		uint64_t *chip_mode);

/**
 * wlan_reg_is_11d_scan_inprogress() - checks 11d scan status
 * @psoc: psoc ptr
 *
 * Return: bool
 */
bool wlan_reg_is_11d_scan_inprogress(struct wlan_objmgr_psoc *psoc);
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
		qdf_freq_t *low_2g,
		qdf_freq_t *high_2g,
		qdf_freq_t *low_5g,
		qdf_freq_t *high_5g);
/**
 * wlan_reg_get_tx_ops () - get regulatory tx ops
 * @psoc: psoc ptr
 *
 */
struct wlan_lmac_if_reg_tx_ops *
wlan_reg_get_tx_ops(struct wlan_objmgr_psoc *psoc);

/**
 * wlan_reg_get_curr_regdomain() - Get current regdomain in use
 * @pdev: pdev pointer
 * @cur_regdmn: Current regdomain info
 *
 * Return: QDF status
 */
QDF_STATUS wlan_reg_get_curr_regdomain(struct wlan_objmgr_pdev *pdev,
		struct cur_regdmn_info *cur_regdmn);

/**
 * wlan_reg_is_regdmn_en302502_applicable() - Find if ETSI EN302_502 radar
 * pattern is applicable in the current regulatory domain.
 * @pdev:    Pdev ptr.
 *
 * Return: Boolean.
 * True:  If EN302_502 is applicable.
 * False: otherwise.
 */
bool wlan_reg_is_regdmn_en302502_applicable(struct wlan_objmgr_pdev *pdev);

/**
 * wlan_reg_modify_pdev_chan_range() - Compute current channel list for the
 * modified channel range in the regcap.
 * @pdev: pointer to wlan_objmgr_pdev.
 *
 * Return : QDF_STATUS
 */
QDF_STATUS wlan_reg_modify_pdev_chan_range(struct wlan_objmgr_pdev *pdev);

/**
 * wlan_reg_get_phybitmap() - Get phybitmap from regulatory pdev_priv_obj
 * @pdev: pdev pointer
 * @phybitmap: pointer to phybitmap
 *
 * Return: QDF STATUS
 */
QDF_STATUS wlan_reg_get_phybitmap(struct wlan_objmgr_pdev *pdev,
				  uint16_t *phybitmap);

/**
 * wlan_reg_update_pdev_wireless_modes() - Update the wireless_modes in the
 * pdev_priv_obj with the input wireless_modes
 * @pdev: pointer to wlan_objmgr_pdev.
 * @wireless_modes: Wireless modes.
 *
 * Return : QDF_STATUS
 */
QDF_STATUS wlan_reg_update_pdev_wireless_modes(struct wlan_objmgr_pdev *pdev,
					       uint32_t wireless_modes);
/**
 * wlan_reg_disable_chan_coex() - Disable Coexisting channels based on the input
 * bitmask
 * @pdev: pointer to wlan_objmgr_pdev.
 * unii_5g_bitmap: UNII 5G bitmap.
 *
 * Return : QDF_STATUS
 */
#ifdef DISABLE_UNII_SHARED_BANDS
QDF_STATUS wlan_reg_disable_chan_coex(struct wlan_objmgr_pdev *pdev,
				      uint8_t unii_5g_bitmap);
#else
static inline QDF_STATUS
wlan_reg_disable_chan_coex(struct wlan_objmgr_pdev *pdev,
			   uint8_t unii_5g_bitmap)
{
	return QDF_STATUS_SUCCESS;
}
#endif

#ifdef WLAN_FEATURE_GET_USABLE_CHAN_LIST
/**
 * wlan_reg_get_usable_channel() - Get usable channels
 * @pdev: Pointer to pdev
 * @req_msg: Request msg
 * @res_msg: Response msg
 * @count: no of usable channels
 *
 * Return: qdf status
 */
QDF_STATUS
wlan_reg_get_usable_channel(struct wlan_objmgr_pdev *pdev,
			    struct get_usable_chan_req_params req_msg,
			    struct get_usable_chan_res_params *res_msg,
			    uint32_t *count);
#endif

#ifdef CONFIG_CHAN_FREQ_API
/**
 * wlan_reg_is_same_band_freqs() - Check if two channel frequencies
 * have same band
 * @freq1: Frequency 1
 * @freq2: Frequency 2
 *
 * Return: true if both the channel frequency has the same band.
 */
#define WLAN_REG_IS_SAME_BAND_FREQS(freq1, freq2) \
	wlan_reg_is_same_band_freqs(freq1, freq2)
bool wlan_reg_is_same_band_freqs(qdf_freq_t freq1, qdf_freq_t freq2);

/**
 * wlan_reg_get_chan_enum_for_freq() - Get channel enum for given channel center
 * frequency
 * @freq: Channel center frequency
 *
 * Return: Channel enum
 */
enum channel_enum wlan_reg_get_chan_enum_for_freq(qdf_freq_t freq);

/**
 * wlan_reg_is_freq_present_in_cur_chan_list() - Check if channel is present
 * in the current channel list
 * @pdev: pdev pointer
 * @freq: Channel center frequency
 *
 * Return: true if channel is present in current channel list
 */
bool wlan_reg_is_freq_present_in_cur_chan_list(struct wlan_objmgr_pdev *pdev,
					       qdf_freq_t freq);

/**
 * wlan_reg_update_nol_history_ch_for_freq() - Set nol-history flag for the
 * channels in the list.
 *
 * @pdev: Pdev ptr
 * @ch_list: Input channel list.
 * @num_ch: Number of channels.
 * @nol_history_ch: Nol history value.
 *
 * Return: void
 */
void wlan_reg_update_nol_history_ch_for_freq(struct wlan_objmgr_pdev *pdev,
					     uint16_t *ch_list,
					     uint8_t num_ch,
					     bool nol_history_ch);
/**
 * wlan_reg_is_frequency_valid_5g_sbs() Check if the given frequency is 5G SBS.
 * @curfreq: current channel center frequency
 * @newfreq:new channel center frequency
 *
 * Return: true if the given frequency is a valid 5G SBS
 */
#define WLAN_REG_IS_FREQUENCY_VALID_5G_SBS(curfreq, newfreq) \
	wlan_reg_is_frequency_valid_5g_sbs(curfreq, newfreq)
bool wlan_reg_is_frequency_valid_5g_sbs(qdf_freq_t curfreq, qdf_freq_t newfreq);

/**
 * wlan_reg_chan_has_dfs_attribute_for_freq() - check channel has dfs
 * attribute flag
 * @freq: channel center frequency.
 *
 * This API get chan initial dfs attribute from regdomain
 *
 * Return: true if chan is dfs, otherwise false
 */
bool
wlan_reg_chan_has_dfs_attribute_for_freq(struct wlan_objmgr_pdev *pdev,
					 qdf_freq_t freq);

/**
 * wlan_reg_get_channel_list_with_power_for_freq() - Provide the channel list
 * with power
 * @ch_list: pointer to the channel list.
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
wlan_reg_get_channel_list_with_power_for_freq(struct wlan_objmgr_pdev *pdev,
					      struct channel_power *ch_list,
					      uint8_t *num_chan);

/**
 * wlan_reg_get_5g_bonded_channel_state_for_freq() - Get 5G bonded channel state
 * @pdev: The physical dev to program country code or regdomain
 * @freq: channel frequency.
 * @bw: channel band width
 *
 * Return: channel state
 */
enum channel_state
wlan_reg_get_5g_bonded_channel_state_for_freq(struct wlan_objmgr_pdev *pdev,
					      qdf_freq_t freq,
					      enum phy_ch_width bw);
/**
 * wlan_reg_get_2g_bonded_channel_state_for_freq() - Get 2G bonded channel state
 * @pdev: The physical dev to program country code or regdomain
 * @freq: channel center frequency.
 * @sec_ch_freq: Secondary channel center frequency.
 * @bw: channel band width
 *
 * Return: channel state
 */
enum channel_state
wlan_reg_get_2g_bonded_channel_state_for_freq(struct wlan_objmgr_pdev *pdev,
					      qdf_freq_t freq,
					      qdf_freq_t sec_ch_freq,
					      enum phy_ch_width bw);

/**
 * wlan_reg_get_channel_state_for_freq() - Get channel state from regulatory
 * @pdev: Pointer to pdev
 * @freq: channel center frequency.
 *
 * Return: channel state
 */
enum channel_state
wlan_reg_get_channel_state_for_freq(struct wlan_objmgr_pdev *pdev,
				    qdf_freq_t freq);

#ifdef CONFIG_REG_CLIENT
/**
 * wlan_reg_get_channel_state_from_secondary_list_for_freq() - Get channel state
 * from secondary regulatory current channel list
 * @pdev: Pointer to pdev
 * @freq: channel center frequency.
 *
 * Return: channel state
 */
enum channel_state wlan_reg_get_channel_state_from_secondary_list_for_freq(
						struct wlan_objmgr_pdev *pdev,
						qdf_freq_t freq);
#endif

/**
 * wlan_reg_fill_channel_list() - Fills the reg_channel_list (list of channels)
 * @pdev: Pointer to struct wlan_objmgr_pdev.
 * @freq: Center frequency of the primary channel in MHz
 * @sec_ch_2g_freq: Secondary channel center frequency.
 * @ch_width: Channel width of type 'enum phy_ch_width'.
 * @band_center_320: Center frequency of 320MHZ channel.
 * @chan_list: Pointer to struct reg_channel_list to be filled (Output param).
 *
 * Return: None
 */
#ifdef WLAN_FEATURE_11BE
void wlan_reg_fill_channel_list(struct wlan_objmgr_pdev *pdev,
				qdf_freq_t freq,
				qdf_freq_t sec_ch_2g_freq,
				enum phy_ch_width ch_width,
				qdf_freq_t band_center_320,
				struct reg_channel_list *chan_list);

/**
 * wlan_reg_is_punc_bitmap_valid() - is puncture bitmap valid or not
 * @bw: Input channel width.
 * @puncture_bitmap Input puncture bitmap.
 *
 * Return: true if given puncture bitmap is valid
 */
bool wlan_reg_is_punc_bitmap_valid(enum phy_ch_width bw,
				   uint16_t puncture_bitmap);

/**
 * wlan_reg_set_create_punc_bitmap() - set is_create_punc_bitmap of ch_params
 * @ch_params: ch_params to set
 * @is_create_punc_bitmap: is create punc bitmap
 *
 * Return: NULL
 */
void wlan_reg_set_create_punc_bitmap(struct ch_params *ch_params,
				     bool is_create_punc_bitmap);
#else
static inline void wlan_reg_set_create_punc_bitmap(struct ch_params *ch_params,
						   bool is_create_punc_bitmap)
{
}
#endif

/**
 * wlan_reg_set_channel_params_for_freq() - Sets channel parameteres for
 * given bandwidth
 * @pdev: The physical dev to program country code or regdomain
 * @freq: channel center frequency.
 * @sec_ch_2g_freq: Secondary channel center frequency.
 * @ch_params: pointer to the channel parameters.
 *
 * Return: None
 */
void wlan_reg_set_channel_params_for_freq(struct wlan_objmgr_pdev *pdev,
					  qdf_freq_t freq,
					  qdf_freq_t sec_ch_2g_freq,
					  struct ch_params *ch_params);

/**
 * wlan_reg_get_channel_cfreq_reg_power_for_freq() - Provide the channel
 * regulatory power
 * @freq: channel center frequency
 *
 * Return: int
 */
uint8_t wlan_reg_get_channel_reg_power_for_freq(struct wlan_objmgr_pdev *pdev,
						qdf_freq_t freq);

/**
 * wlan_reg_get_bonded_chan_entry() - Fetch the bonded channel pointer given a
 * frequency and channel width.
 * @freq: Input frequency in MHz.
 * @chwidth: Input channel width of enum phy_ch_width.
 *
 * Return: A valid bonded channel pointer if found, else NULL.
 */
const struct bonded_channel_freq *
wlan_reg_get_bonded_chan_entry(qdf_freq_t freq, enum phy_ch_width chwidth);

/**
 * wlan_reg_update_nol_ch_for_freq () - set nol channel
 * @pdev: pdev ptr
 * @chan_freq_list: channel list to be returned
 * @num_ch: number of channels
 * @nol_ch: nol flag
 *
 * Return: void
 */
void wlan_reg_update_nol_ch_for_freq(struct wlan_objmgr_pdev *pdev,
				     uint16_t *chan_freq_list,
				     uint8_t num_ch,
				     bool nol_ch);

/**
 * wlan_reg_is_dfs_freq() - Checks the channel state for DFS
 * @freq: Channel center frequency
 *
 * Return: true or false
 */
bool wlan_reg_is_dfs_for_freq(struct wlan_objmgr_pdev *pdev, qdf_freq_t freq);

/**
 * wlan_reg_is_dsrc_freq() - Checks if the channel is dsrc channel or not
 * @freq: Channel center frequency
 *
 * Return: true or false
 */
bool wlan_reg_is_dsrc_freq(qdf_freq_t freq);

/**
 * wlan_reg_is_passive_or_disable_for_freq() - Checks chan state for passive
 * and disabled
 * @pdev: pdev ptr
 * @freq: Channel center frequency
 *
 * Return: true or false
 */
bool wlan_reg_is_passive_or_disable_for_freq(struct wlan_objmgr_pdev *pdev,
					     qdf_freq_t freq);

/**
 * wlan_reg_is_disable_for_freq() - Checks chan state for disabled
 * @pdev: pdev ptr
 * @freq: Channel center frequency
 *
 * Return: true or false
 */
bool wlan_reg_is_disable_for_freq(struct wlan_objmgr_pdev *pdev,
				  qdf_freq_t freq);

#ifdef CONFIG_REG_CLIENT
/**
 * wlan_reg_is_disable_in_secondary_list_for_freq() - Checks in the secondary
 * channel list to see if chan state is disabled
 * @pdev: pdev ptr
 * @freq: Channel center frequency
 *
 * Return: true or false
 */
bool wlan_reg_is_disable_in_secondary_list_for_freq(
						struct wlan_objmgr_pdev *pdev,
						qdf_freq_t freq);

/**
 * wlan_reg_is_enable_in_secondary_list_for_freq() - Checks in the secondary
 * channel list to see if chan state is enabled
 * @pdev: pdev ptr
 * @freq: Channel center frequency
 *
 * Return: true or false
 */
bool wlan_reg_is_enable_in_secondary_list_for_freq(
						struct wlan_objmgr_pdev *pdev,
						qdf_freq_t freq);

/**
 * wlan_reg_is_dfs_in_secondary_list_for_freq() - hecks the channel state for
 * DFS from the secondary channel list
 * @pdev: pdev ptr
 * @freq: Channel center frequency
 *
 * Return: true or false
 */
bool wlan_reg_is_dfs_in_secondary_list_for_freq(struct wlan_objmgr_pdev *pdev,
						qdf_freq_t freq);
#endif

/**
 * wlan_reg_is_passive_for_freq() - Check the channel flags to see if the
 * passive flag is set
 * @pdev: pdev ptr
 * @freq: Channel center frequency
 *
 * Return: true or false
 */
bool wlan_reg_is_passive_for_freq(struct wlan_objmgr_pdev *pdev,
				  qdf_freq_t freq);

/**
 * wlan_reg_freq_to_band() - Get band from channel number
 * @freq:Channel frequency in MHz
 *
 * Return: wifi band
 */
enum reg_wifi_band wlan_reg_freq_to_band(qdf_freq_t freq);

/**
 * wlan_reg_min_chan_freq() - Minimum channel frequency supported
 *
 * Return: frequency
 */
qdf_freq_t wlan_reg_min_chan_freq(void);

/**
 * wlan_reg_max_chan_freq() - Return max. frequency
 *
 * Return: frequency
 */
qdf_freq_t wlan_reg_max_chan_freq(void);

/**
 * wlan_reg_freq_width_to_chan_op_class() -Get op class from freq
 * @pdev: pdev ptr
 * @freq: channel frequency
 * @chan_width: channel width
 * @global_tbl_lookup: whether to look up global table
 * @behav_limit: behavior limit
 * @op_class: operating class
 * @chan_num: channel number
 *
 * Return: void
 */
void wlan_reg_freq_width_to_chan_op_class(struct wlan_objmgr_pdev *pdev,
					  qdf_freq_t freq,
					  uint16_t chan_width,
					  bool global_tbl_lookup,
					  uint16_t behav_limit,
					  uint8_t *op_class,
					  uint8_t *chan_num);

/**
 * wlan_reg_freq_width_to_chan_op_class_auto() - convert frequency to
 * operating class,channel
 * @pdev: pdev pointer
 * @freq: channel frequency in mhz
 * @chan_width: channel width
 * @global_tbl_lookup: whether to lookup global op class tbl
 * @behav_limit: behavior limit
 * @op_class: operating class
 * @chan_num: channel number
 *
 * Return: Void.
 */
void wlan_reg_freq_width_to_chan_op_class_auto(struct wlan_objmgr_pdev *pdev,
					       qdf_freq_t freq,
					       uint16_t chan_width,
					       bool global_tbl_lookup,
					       uint16_t behav_limit,
					       uint8_t *op_class,
					       uint8_t *chan_num);

/**
 * wlan_reg_freq_to_chan_and_op_class() - Converts freq to oper class
 * @pdev: pdev ptr
 * @freq: channel frequency
 * @global_tbl_lookup: whether to look up global table
 * @behav_limit: behavior limit
 * @op_class: operating class
 * @chan_num: channel number
 *
 * Return: void
 */
void wlan_reg_freq_to_chan_op_class(struct wlan_objmgr_pdev *pdev,
				    qdf_freq_t freq,
				    bool global_tbl_lookup,
				    uint16_t behav_limit,
				    uint8_t *op_class,
				    uint8_t *chan_num);

/**
 * wlan_reg_is_freq_in_country_opclass() - checks frequency in (ctry, op class)
 *                                         pair
 * @pdev: pdev ptr
 * @country: country information
 * @op_class: operating class
 * @chan_freq: channel frequency
 *
 * Return: bool
 */
bool wlan_reg_is_freq_in_country_opclass(struct wlan_objmgr_pdev *pdev,
					 const uint8_t country[3],
					 uint8_t op_class,
					 qdf_freq_t chan_freq);

/**
 * wlan_reg_get_5g_bonded_channel_and_state_for_freq()- Return the channel
 * state for a 5G or 6G channel frequency based on the channel width and
 * bonded channel.
 * @pdev: Pointer to pdev.
 * @freq: Channel center frequency.
 * @bw Channel Width.
 * @bonded_chan_ptr_ptr: Pointer to bonded_channel_freq.
 *
 * Return: Channel State
 */
enum channel_state
wlan_reg_get_5g_bonded_channel_and_state_for_freq(struct wlan_objmgr_pdev *pdev,
						  uint16_t freq,
						  enum phy_ch_width bw,
						  const
						  struct bonded_channel_freq
						  **bonded_chan_ptr_ptr);
#endif /*CONFIG_CHAN_FREQ_API */

/**
 * wlan_reg_get_op_class_width() - Get operating class chan width
 * @pdev: pdev ptr
 * @freq: channel frequency
 * @global_tbl_lookup: whether to look up global table
 * @op_class: operating class
 * @chan_num: channel number
 *
 * Return: channel width of op class
 */
uint16_t wlan_reg_get_op_class_width(struct wlan_objmgr_pdev *pdev,
				     uint8_t op_class,
				     bool global_tbl_lookup);

/**
 * wlan_reg_is_5ghz_op_class() - Check if the input opclass is a 5GHz opclass.
 * @country: Country code.
 * @op_class: Operating class.
 *
 * Return: Return true if input the opclass is a 5GHz opclass,
 * else return false.
 */
bool wlan_reg_is_5ghz_op_class(const uint8_t *country, uint8_t op_class);

/**
 * wlan_reg_is_2ghz_op_class() - Check if the input opclass is a 2.4GHz opclass.
 * @country: Country code.
 * @op_class: Operating class.
 *
 * Return: Return true if input the opclass is a 2.4GHz opclass,
 * else return false.
 */
bool wlan_reg_is_2ghz_op_class(const uint8_t *country, uint8_t op_class);

/**
 * wlan_reg_is_6ghz_op_class() - Whether 6ghz oper class
 * @pdev: pdev ptr
 * @op_class: operating class
 *
 * Return: bool
 */
bool wlan_reg_is_6ghz_op_class(struct wlan_objmgr_pdev *pdev,
			       uint8_t op_class);

#ifdef CONFIG_REG_CLIENT
/**
 * wlan_reg_is_6ghz_supported() - Whether 6ghz is supported
 * @psoc: psoc ptr
 *
 * Return: bool
 */
bool wlan_reg_is_6ghz_supported(struct wlan_objmgr_psoc *psoc);
#endif

#ifdef HOST_OPCLASS_EXT
/**
 * wlan_reg_country_chan_opclass_to_freq() - Convert channel number to
 * frequency based on country code and op class
 * @pdev: pdev object.
 * @country: country code.
 * @chan: IEEE Channel Number.
 * @op_class: Opclass.
 * @strict: flag to find channel from matched operating class code.
 *
 * Look up (channel, operating class) pair in country operating class tables
 * and return the channel frequency.
 * If not found and "strict" flag is false, try to get frequency (Mhz) by
 * channel number only.
 *
 * Return: Channel center frequency else return 0.
 */
qdf_freq_t
wlan_reg_country_chan_opclass_to_freq(struct wlan_objmgr_pdev *pdev,
				      const uint8_t country[3],
				      uint8_t chan, uint8_t op_class,
				      bool strict);
#endif

/**
 * reg_chan_opclass_to_freq() - Convert channel number and opclass to frequency
 * @chan: IEEE Channel Number.
 * @op_class: Opclass.
 * @global_tbl_lookup: Global table lookup.
 *
 * Return: Channel center frequency else return 0.
 */
uint16_t wlan_reg_chan_opclass_to_freq(uint8_t chan,
				       uint8_t op_class,
				       bool global_tbl_lookup);

/**
 * wlan_reg_chan_opclass_to_freq_auto() - Convert channel number and opclass to
 * frequency
 * @chan: IEEE channel number
 * @op_class: Operating class of channel
 * @global_tbl_lookup: Flag to determine if global table has to be looked up
 *
 * Return: Channel center frequency if valid, else zero
 */

qdf_freq_t wlan_reg_chan_opclass_to_freq_auto(uint8_t chan, uint8_t op_class,
					      bool global_tbl_lookup);

#ifdef CHECK_REG_PHYMODE
/**
 * wlan_reg_get_max_phymode() - Find the best possible phymode given a
 * phymode, a frequency, and per-country regulations
 * @pdev: pdev pointer
 * @phy_in: phymode that the user requested
 * @freq: current operating center frequency
 *
 * Return: maximum phymode allowed in current country that is <= phy_in
 */
enum reg_phymode wlan_reg_get_max_phymode(struct wlan_objmgr_pdev *pdev,
					  enum reg_phymode phy_in,
					  qdf_freq_t freq);
#else
static inline enum reg_phymode
wlan_reg_get_max_phymode(struct wlan_objmgr_pdev *pdev,
			 enum reg_phymode phy_in,
			 qdf_freq_t freq)
{
	return REG_PHYMODE_INVALID;
}
#endif /* CHECK_REG_PHYMODE */

#ifdef CONFIG_REG_CLIENT
/**
 * wlan_reg_band_bitmap_to_band_info() - Convert the band_bitmap to a
 *	band_info enum
 * @band_bitmap: bitmap on top of reg_wifi_band of bands enabled
 *
 * Return: BAND_ALL if both 2G and 5G band is enabled
 *	BAND_2G if 2G is enabled but 5G isn't
 *	BAND_5G if 5G is enabled but 2G isn't
 */
enum band_info wlan_reg_band_bitmap_to_band_info(uint32_t band_bitmap);

/**
 * wlan_reg_modify_indoor_concurrency() - Update the indoor concurrency list
 * in regulatory pdev context
 *
 * @pdev: pointer to pdev
 * @vdev_id: vdev id
 * @freq: frequency
 * @width: channel width
 * @add: add or delete entry
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
wlan_reg_modify_indoor_concurrency(struct wlan_objmgr_pdev *pdev,
				   uint8_t vdev_id, uint32_t freq,
				   enum phy_ch_width width, bool add);

/**
 * wlan_reg_recompute_current_chan_list() - Recompute the current channel list
 * based on the regulatory change
 *
 * @psoc: pointer to psoc
 * @pdev: pointer to pdev
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
wlan_reg_recompute_current_chan_list(struct wlan_objmgr_psoc *psoc,
				     struct wlan_objmgr_pdev *pdev);
#endif

#if defined(CONFIG_BAND_6GHZ)
/**
 * wlan_reg_get_cur_6g_ap_pwr_type() - Get the current 6G regulatory AP power
 * type.
 * @pdev: Pointer to PDEV object.
 * @reg_cur_6g_ap_pwr_type: The current regulatory 6G AP power type ie.
 * LPI/SP/VLP.
 *
 * Return: QDF_STATUS.
 */
QDF_STATUS
wlan_reg_get_cur_6g_ap_pwr_type(struct wlan_objmgr_pdev *pdev,
				enum reg_6g_ap_type *reg_cur_6g_ap_pwr_type);

/**
 * wlan_reg_get_cur_6g_client_type() - Get the current 6G regulatory client
 * type.
 * @pdev: Pointer to PDEV object.
 * @reg_cur_6g_client_mobility_type: The current regulatory 6G client type ie.
 * default/subordinate.
 *
 * Return: QDF_STATUS.
 */
QDF_STATUS
wlan_reg_get_cur_6g_client_type(struct wlan_objmgr_pdev *pdev,
				enum reg_6g_client_type
				*reg_cur_6g_client_mobility_type);
/**
 * wlan_reg_get_rnr_tpe_usable() - Tells if RNR IE is applicable for current
 * domain.
 * @pdev: Pointer to PDEV object.
 * @reg_rnr_tpe_usable: Pointer to hold the bool value, true if RNR IE is
 * applicable, else false.
 *
 * Return: QDF_STATUS.
 */
QDF_STATUS wlan_reg_get_rnr_tpe_usable(struct wlan_objmgr_pdev *pdev,
				       bool *reg_rnr_tpe_usable);

/**
 * wlan_reg_get_unspecified_ap_usable() - Tells if AP type unspecified by 802.11
 * can be used or not.
 * @pdev: Pointer to PDEV object.
 * @reg_unspecified_ap_usable: Pointer to hold the bool value, true if
 * unspecified AP types can be used in the IE, else false.
 *
 * Return: QDF_STATUS.
 */
QDF_STATUS wlan_reg_get_unspecified_ap_usable(struct wlan_objmgr_pdev *pdev,
					      bool *reg_unspecified_ap_usable);

/**
 * wlan_reg_is_6g_psd_power() - Checks if given freq is PSD power
 *
 * @pdev: pdev ptr
 * @freq: channel frequency
 *
 * Return: true if channel is PSD power or false otherwise
 */
bool wlan_reg_is_6g_psd_power(struct wlan_objmgr_pdev *pdev);

/**
 * wlan_reg_get_6g_chan_ap_power() - Finds the AP TX power for the given channel
 *	frequency
 *
 * @pdev: pdev ptr
 * @chan_freq: channel frequency
 * @is_psd: is channel PSD or not
 * @tx_power: transmit power to fill for chan_freq
 * @eirp_psd_power: EIRP power, will only be filled if is_psd is true
 *
 * Return: QDF_STATUS
 */
QDF_STATUS wlan_reg_get_6g_chan_ap_power(struct wlan_objmgr_pdev *pdev,
					 qdf_freq_t chan_freq, bool *is_psd,
					 uint16_t *tx_power,
					 uint16_t *eirp_psd_power);

/**
 * wlan_reg_get_client_power_for_connecting_ap() - Find the channel information
 *	when device is operating as a client
 *
 * @pdev: pdev ptr
 * @ap_type: type of AP that device is connected to
 * @chan_freq: channel frequency
 * @is_psd: is channel PSD or not
 * @tx_power: transmit power to fill for chan_freq
 * @eirp_psd_power: EIRP power, will only be filled if is_psd is true
 *
 * This function is meant to be called to find the channel frequency power
 * information for a client when the device is operating as a client. It will
 * fill in the parameters tx_power and eirp_psd_power. eirp_psd_power will
 * only be filled if the channel is PSD.
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
wlan_reg_get_client_power_for_connecting_ap(struct wlan_objmgr_pdev *pdev,
					    enum reg_6g_ap_type ap_type,
					    qdf_freq_t chan_freq,
					    bool is_psd, uint16_t *tx_power,
					    uint16_t *eirp_psd_power);

/**
 * wlan_reg_get_client_power_for_6ghz_ap() - Find the channel information when
 *	device is operating as a 6GHz AP
 *
 * @pdev: pdev ptr
 * @client_type: type of client that is connected to our AP
 * @chan_freq: channel frequency
 * @is_psd: is channel PSD or not
 * @tx_power: transmit power to fill for chan_freq
 * @eirp_psd_power: EIRP power, will only be filled if is_psd is true
 *
 * This function is meant to be called to find the channel frequency power
 * information for a client when the device is operating as an AP. It will fill
 * in the parameter is_psd, tx_power, and eirp_psd_power. eirp_psd_power will
 * only be filled if the channel is PSD.
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
wlan_reg_get_client_power_for_6ghz_ap(struct wlan_objmgr_pdev *pdev,
				      enum reg_6g_client_type client_type,
				      qdf_freq_t chan_freq,
				      bool *is_psd, uint16_t *tx_power,
				      uint16_t *eirp_psd_power);

/**
 * wlan_reg_decide_6g_ap_pwr_type() - Decide which power mode AP should operate
 * in
 *
 * @pdev: pdev ptr
 *
 * Return: AP power type
 */
enum reg_6g_ap_type
wlan_reg_decide_6g_ap_pwr_type(struct wlan_objmgr_pdev *pdev);

/**
 * wlan_reg_set_ap_pwr_and_update_chan_list() - Set the AP power mode and
 * recompute the current channel list
 *
 * @pdev: pdev ptr
 * @ap_pwr_type: the AP power type to update to
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
wlan_reg_set_ap_pwr_and_update_chan_list(struct wlan_objmgr_pdev *pdev,
					 enum reg_6g_ap_type ap_pwr_type);
#else
static inline QDF_STATUS
wlan_reg_get_cur_6g_ap_pwr_type(struct wlan_objmgr_pdev *pdev,
				enum reg_6g_ap_type *reg_cur_6g_ap_pwr_type)
{
	*reg_cur_6g_ap_pwr_type = REG_CURRENT_MAX_AP_TYPE;
	return QDF_STATUS_E_NOSUPPORT;
}

static inline QDF_STATUS
wlan_reg_get_cur_6g_client_type(struct wlan_objmgr_pdev *pdev,
				enum reg_6g_client_type
				*reg_cur_6g_client_mobility_type)
{
	*reg_cur_6g_client_mobility_type = REG_SUBORDINATE_CLIENT;
	return QDF_STATUS_E_NOSUPPORT;
}

static inline
QDF_STATUS wlan_reg_get_rnr_tpe_usable(struct wlan_objmgr_pdev *pdev,
				       bool *reg_rnr_tpe_usable)
{
	*reg_rnr_tpe_usable = false;
	return QDF_STATUS_E_NOSUPPORT;
}

static inline
QDF_STATUS wlan_reg_get_unspecified_ap_usable(struct wlan_objmgr_pdev *pdev,
					      bool *reg_unspecified_ap_usable)
{
	*reg_unspecified_ap_usable = false;
	return QDF_STATUS_E_NOSUPPORT;
}

static inline
bool wlan_reg_is_6g_psd_power(struct wlan_objmgr_pdev *pdev)
{
	return false;
}

static inline
QDF_STATUS wlan_reg_get_6g_chan_ap_power(struct wlan_objmgr_pdev *pdev,
					 qdf_freq_t chan_freq, bool *is_psd,
					 uint16_t *tx_power,
					 uint16_t *eirp_psd_power)
{
	*is_psd = false;
	*tx_power = 0;
	*eirp_psd_power = 0;
	return QDF_STATUS_E_NOSUPPORT;
}

static inline QDF_STATUS
wlan_reg_get_client_power_for_connecting_ap(struct wlan_objmgr_pdev *pdev,
					    enum reg_6g_ap_type ap_type,
					    qdf_freq_t chan_freq,
					    bool is_psd, uint16_t *tx_power,
					    uint16_t *eirp_psd_power)
{
	*tx_power = 0;
	*eirp_psd_power = 0;
	return QDF_STATUS_E_NOSUPPORT;
}

static inline QDF_STATUS
wlan_reg_get_client_power_for_6ghz_ap(struct wlan_objmgr_pdev *pdev,
				      enum reg_6g_client_type client_type,
				      qdf_freq_t chan_freq,
				      bool *is_psd, uint16_t *tx_power,
				      uint16_t *eirp_psd_power)
{
	*is_psd = false;
	*tx_power = 0;
	*eirp_psd_power = 0;
	return QDF_STATUS_E_NOSUPPORT;
}

static inline enum reg_6g_ap_type
wlan_reg_decide_6g_ap_pwr_type(struct wlan_objmgr_pdev *pdev)
{
	return REG_INDOOR_AP;
}

static inline QDF_STATUS
wlan_reg_set_ap_pwr_and_update_chan_list(struct wlan_objmgr_pdev *pdev,
					 enum reg_6g_ap_type ap_pwr_type)
{
	return QDF_STATUS_E_NOSUPPORT;
}
#endif

/**
 * wlan_reg_is_ext_tpc_supported() - Checks if FW supports new WMI cmd for TPC
 *
 * @psoc: psoc ptr
 *
 * Return: true if FW supports new command or false otherwise
 */
bool wlan_reg_is_ext_tpc_supported(struct wlan_objmgr_psoc *psoc);

/**
 * wlan_reg_is_chwidth_supported() - Check if given channel width is supported
 * on a given pdev
 * @pdev: pdev pointer
 * @ch_width: channel width.
 * @is_supported: whether the channel width is supported
 *
 * Return QDF_STATUS_SUCCESS of operation
 */
QDF_STATUS wlan_reg_is_chwidth_supported(struct wlan_objmgr_pdev *pdev,
					 enum phy_ch_width ch_width,
					 bool *is_supported);
#endif
