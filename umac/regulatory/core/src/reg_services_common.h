/*
 * Copyright (c) 2017-2021 The Linux Foundation. All rights reserved.
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
 * DOC: reg_services.h
 * This file provides prototypes of the regulatory component
 * service functions
 */

#ifndef __REG_SERVICES_COMMON_H_
#define __REG_SERVICES_COMMON_H_

#ifdef CONFIG_AFC_SUPPORT
#include <wlan_reg_afc.h>
#endif

#define IS_VALID_PSOC_REG_OBJ(psoc_priv_obj) (psoc_priv_obj)
#define IS_VALID_PDEV_REG_OBJ(pdev_priv_obj) (pdev_priv_obj)
#define FREQ_TO_CHAN_SCALE     5
/* The distance between the 80Mhz center and the nearest 20Mhz channel */
#define NEAREST_20MHZ_CHAN_FREQ_OFFSET     10
#define NUM_20_MHZ_CHAN_IN_80_MHZ_CHAN     4
#define NUM_20_MHZ_CHAN_IN_160_MHZ_CHAN    8

#define REG_MAX_5GHZ_CH_NUM channel_map[MAX_5GHZ_CHANNEL].chan_num

#define REG_MIN_24GHZ_CH_FREQ channel_map[MIN_24GHZ_CHANNEL].center_freq
#define REG_MAX_24GHZ_CH_FREQ channel_map[MAX_24GHZ_CHANNEL].center_freq

#define REG_IS_24GHZ_CH_FREQ(freq) \
	(((freq) >= REG_MIN_24GHZ_CH_FREQ) &&   \
	((freq) <= REG_MAX_24GHZ_CH_FREQ))

#ifdef CONFIG_CHAN_FREQ_API
#define REG_MIN_5GHZ_CH_FREQ channel_map[MIN_5GHZ_CHANNEL].center_freq
#define REG_MAX_5GHZ_CH_FREQ channel_map[MAX_5GHZ_CHANNEL].center_freq
#endif /* CONFIG_CHAN_FREQ_API */

#ifdef CONFIG_49GHZ_CHAN
#define REG_MIN_49GHZ_CH_FREQ channel_map[MIN_49GHZ_CHANNEL].center_freq
#define REG_MAX_49GHZ_CH_FREQ channel_map[MAX_49GHZ_CHANNEL].center_freq
#else
#define REG_MIN_49GHZ_CH_FREQ 0
#define REG_MAX_49GHZ_CH_FREQ 0
#endif /* CONFIG_49GHZ_CHAN */

#define REG_IS_49GHZ_FREQ(freq) \
	(((freq) >= REG_MIN_49GHZ_CH_FREQ) &&   \
	((freq) <= REG_MAX_49GHZ_CH_FREQ))


#define REG_IS_5GHZ_FREQ(freq) \
	(((freq) >= channel_map[MIN_5GHZ_CHANNEL].center_freq) &&	\
	 ((freq) <= channel_map[MAX_5GHZ_CHANNEL].center_freq))

/*
 * It should be 2.5 MHz actually but since we are using integer use 2
 * instead, which does not create any problem in the start edge calculation.
 */
#define HALF_5MHZ_BW     2
#define HALF_20MHZ_BW    10
#define HALF_40MHZ_BW    20
#define HALF_80MHZ_BW    40
#define HALF_160MHZ_BW   80

#define TWO_GIG_STARTING_EDGE_FREQ (channel_map_global[MIN_24GHZ_CHANNEL]. \
				  center_freq - HALF_20MHZ_BW)
#define TWO_GIG_ENDING_EDGE_FREQ   (channel_map_global[MAX_24GHZ_CHANNEL]. \
				  center_freq + HALF_20MHZ_BW)
#ifdef CONFIG_49GHZ_CHAN
#define FIVE_GIG_STARTING_EDGE_FREQ (channel_map_global[MIN_49GHZ_CHANNEL]. \
				  center_freq - HALF_5MHZ_BW)
#else
#define FIVE_GIG_STARTING_EDGE_FREQ (channel_map_global[MIN_5GHZ_CHANNEL]. \
				  center_freq - HALF_20MHZ_BW)
#endif /* CONFIG_49GHZ_CHAN */
#define FIVE_GIG_ENDING_EDGE_FREQ   (channel_map_global[MAX_5GHZ_CHANNEL]. \
				  center_freq + HALF_20MHZ_BW)

#ifdef CONFIG_BAND_6GHZ
#define SIX_GIG_STARTING_EDGE_FREQ  (channel_map_global[MIN_6GHZ_CHANNEL]. \
				  center_freq - HALF_20MHZ_BW)
#define SIX_GIG_ENDING_EDGE_FREQ    (channel_map_global[MAX_6GHZ_CHANNEL]. \
				  center_freq + HALF_20MHZ_BW)

#define FREQ_LEFT_SHIFT         55
#define SIX_GHZ_NON_ORPHAN_START_FREQ \
	(channel_map_global[MIN_6GHZ_NON_ORPHAN_CHANNEL].center_freq  - 5)
#define CHAN_FREQ_5935          5935
#define NUM_80MHZ_BAND_IN_6G    16
#define NUM_PSC_FREQ            15
#define PSC_BAND_MHZ (FREQ_TO_CHAN_SCALE * NUM_80MHZ_BAND_IN_6G)
#define REG_MIN_6GHZ_CHAN_FREQ channel_map[MIN_6GHZ_CHANNEL].center_freq
#define REG_MAX_6GHZ_CHAN_FREQ channel_map[MAX_6GHZ_CHANNEL].center_freq
#else
#define FREQ_LEFT_SHIFT         0
#define SIX_GHZ_NON_ORPHAN_START_FREQ       0
#define CHAN_FREQ_5935          0
#define NUM_80MHZ_BAND_IN_6G    0
#define NUM_PSC_FREQ            0
#define PSC_BAND_MHZ (FREQ_TO_CHAN_SCALE * NUM_80MHZ_BAND_IN_6G)
#define REG_MIN_6GHZ_CHAN_FREQ  0
#define REG_MAX_6GHZ_CHAN_FREQ  0
#endif /*CONFIG_BAND_6GHZ*/

#define REG_CH_NUM(ch_enum) channel_map[ch_enum].chan_num
#define REG_CH_TO_FREQ(ch_enum) channel_map[ch_enum].center_freq

/* EEPROM setting is a country code */
#define    COUNTRY_ERD_FLAG     0x8000
#define MIN_6GHZ_OPER_CLASS 131
#define MAX_6GHZ_OPER_CLASS 136

#ifdef CONFIG_AFC_SUPPORT
#define DEFAULT_REQ_ID 11235813
/* default minimum power in dBm units */
#define DEFAULT_MIN_POWER    (-10)
#define DEFAULT_NUM_FREQS       1

/* Have the entire 6Ghz band as single range */
#define DEFAULT_LOW_6GFREQ    5925
#define DEFAULT_HIGH_6GFREQ   7125
#endif

#define SIXG_CHAN_2           2
#ifdef CONFIG_BAND_6GHZ
#define CHAN_ENUM_SIXG_2      CHAN_ENUM_5935
#else
#define CHAN_ENUM_SIXG_2      INVALID_CHANNEL
#endif

extern const struct chan_map *channel_map;
extern const struct chan_map channel_map_us[];
extern const struct chan_map channel_map_eu[];
extern const struct chan_map channel_map_jp[];
extern const struct chan_map channel_map_china[];
extern const struct chan_map channel_map_global[];

#ifdef WLAN_FEATURE_11BE
#define ALL_SCHANS_PUNC 0x0000 /* all subchannels punctured */
#endif

#ifdef CONFIG_AFC_SUPPORT
/**
 * struct afc_cb_handler - defines structure for afc request received  event
 * handler call back function and argument
 * @func: handler function pointer
 * @arg: argument to handler function
 */
struct afc_cb_handler {
	afc_req_rx_evt_handler func;
	void *arg;
};

/**
 * reg_init_freq_range() - Initialize a freq_range object
 * @left: The left frequency range
 * @right: The right frequency range
 *
 * Return: The initialized freq_range object
 */
struct freq_range
reg_init_freq_range(qdf_freq_t left, qdf_freq_t right);
#endif
/**
 * get_next_lower_bandwidth() - Get next lower bandwidth
 * @ch_width: Channel width
 *
 * Return: Channel width
 */
enum phy_ch_width get_next_lower_bandwidth(enum phy_ch_width ch_width);

/**
 * reg_read_default_country() - Get the default regulatory country
 * @psoc: The physical SoC to get default country from
 * @country_code: the buffer to populate the country code into
 *
 * Return: QDF_STATUS
 */
QDF_STATUS reg_read_default_country(struct wlan_objmgr_psoc *psoc,
				    uint8_t *country_code);

/**
 * reg_get_ctry_idx_max_bw_from_country_code() - Get the max 5G bandwidth
 * from country code
 * @cc : Country Code
 * @max_bw_5g : Max 5G bandwidth supported by the country
 *
 * Return : QDF_STATUS
 */

QDF_STATUS reg_get_max_5g_bw_from_country_code(uint16_t cc,
					       uint16_t *max_bw_5g);

/**
 * reg_get_max_5g_bw_from_regdomain() - Get the max 5G bandwidth
 * supported by the regdomain
 * @orig_regdmn : Regdomain pair value
 * @max_bw_5g : Max 5G bandwidth supported by the country
 *
 * Return : QDF_STATUS
 */

QDF_STATUS reg_get_max_5g_bw_from_regdomain(uint16_t regdmn,
					    uint16_t *max_bw_5g);

/**
 * reg_get_current_dfs_region () - Get the current dfs region
 * @pdev: Pointer to pdev
 * @dfs_reg: pointer to dfs region
 *
 * Return: None
 */
void reg_get_current_dfs_region(struct wlan_objmgr_pdev *pdev,
				enum dfs_reg *dfs_reg);

/**
 * reg_get_bw_value() - give bandwidth value
 * bw: bandwidth enum
 *
 * Return: uint16_t
 */
uint16_t reg_get_bw_value(enum phy_ch_width bw);

/**
 * reg_set_dfs_region () - Set the current dfs region
 * @pdev: Pointer to pdev
 * @dfs_reg: pointer to dfs region
 *
 * Return: None
 */
void reg_set_dfs_region(struct wlan_objmgr_pdev *pdev,
			enum dfs_reg dfs_reg);

/**
 * reg_program_chan_list() - Set user country code and populate the channel list
 * @pdev: Pointer to pdev
 * @rd: Pointer to cc_regdmn_s structure
 *
 * Return: QDF_STATUS
 */
QDF_STATUS reg_program_chan_list(struct wlan_objmgr_pdev *pdev,
				 struct cc_regdmn_s *rd);

/**
 * reg_freq_to_chan() - Get channel number from frequency.
 * @pdev: Pointer to pdev
 * @freq: Channel frequency
 *
 * Return: Channel number if success, otherwise 0
 */
uint8_t reg_freq_to_chan(struct wlan_objmgr_pdev *pdev, qdf_freq_t freq);

/**
 * reg_legacy_chan_to_freq() - Get freq from chan noumber, for 2G and 5G
 * @pdev: Pointer to pdev
 * @chan_num: Channel number
 *
 * Return: Channel frequency if success, otherwise 0
 */
uint16_t reg_legacy_chan_to_freq(struct wlan_objmgr_pdev *pdev,
				 uint8_t chan_num);

/**
 * reg_program_default_cc() - Program default country code
 * @pdev: Pdev pointer
 * @regdmn: Regdomain value
 *
 * Return: QDF_STATUS
 */
QDF_STATUS reg_program_default_cc(struct wlan_objmgr_pdev *pdev,
				  uint16_t regdmn);

/**
 * reg_get_current_cc() - Get current country code
 * @pdev: Pdev pointer
 * @regdmn: Pointer to get current country values
 *
 * Return: QDF_STATUS
 */
QDF_STATUS reg_get_current_cc(struct wlan_objmgr_pdev *pdev,
			      struct cc_regdmn_s *rd);

/**
 * reg_set_regdb_offloaded() - set/clear regulatory offloaded flag
 *
 * @psoc: psoc pointer
 * Return: Success or Failure
 */
QDF_STATUS reg_set_regdb_offloaded(struct wlan_objmgr_psoc *psoc, bool val);

/**
 * reg_get_curr_regdomain() - Get current regdomain in use
 * @pdev: pdev pointer
 * @cur_regdmn: Current regdomain info
 *
 * Return: QDF status
 */
QDF_STATUS reg_get_curr_regdomain(struct wlan_objmgr_pdev *pdev,
				  struct cur_regdmn_info *cur_regdmn);

/**
 * reg_modify_chan_144() - Enable/Disable channel 144
 * @pdev: pdev pointer
 * @en_chan_144: flag to disable/enable channel 144
 *
 * Return: Success or Failure
 */
QDF_STATUS reg_modify_chan_144(struct wlan_objmgr_pdev *pdev, bool en_chan_144);

/**
 * reg_get_en_chan_144() - get en_chan_144 flag value
 * @pdev: pdev pointer
 *
 * Return: en_chan_144 flag value
 */
bool reg_get_en_chan_144(struct wlan_objmgr_pdev *pdev);

/**
 * reg_get_hal_reg_cap() - Get HAL REG capabilities
 * @psoc: psoc for country information
 *
 * Return: hal reg cap pointer
 */
struct wlan_psoc_host_hal_reg_capabilities_ext *reg_get_hal_reg_cap(
		struct wlan_objmgr_psoc *psoc);

/**
 * reg_set_hal_reg_cap() - Set HAL REG capabilities
 * @psoc: psoc for country information
 * @reg_cap: Regulatory caps pointer
 * @phy_cnt: number of phy
 *
 * Return: hal reg cap pointer
 */
QDF_STATUS reg_set_hal_reg_cap(
		struct wlan_objmgr_psoc *psoc,
		struct wlan_psoc_host_hal_reg_capabilities_ext *reg_cap,
		uint16_t phy_cnt);

/**
 * reg_update_hal_reg_cap() - Update HAL REG capabilities
 * @psoc: psoc pointer
 * @wireless_modes: 11AX wireless modes
 * @phy_id: phy id
 *
 * Return: QDF_STATUS
 */
QDF_STATUS reg_update_hal_reg_cap(struct wlan_objmgr_psoc *psoc,
				  uint64_t wireless_modes, uint8_t phy_id);

/**
 * reg_chan_in_range() - Check if the given channel is in pdev's channel range
 * @chan_list: Pointer to regulatory channel list.
 * @low_freq_2g: Low frequency 2G.
 * @high_freq_2g: High frequency 2G.
 * @low_freq_5g: Low frequency 5G.
 * @high_freq_5g: High frequency 5G.
 * @ch_enum: Channel enum.
 *
 * Return: true if ch_enum is with in pdev's channel range, else false.
 */
bool reg_chan_in_range(struct regulatory_channel *chan_list,
		       qdf_freq_t low_freq_2g, qdf_freq_t high_freq_2g,
		       qdf_freq_t low_freq_5g, qdf_freq_t high_freq_5g,
		       enum channel_enum ch_enum);

/**
 * reg_init_channel_map() - Initialize the channel list based on the dfs region.
 * @dfs_region: Dfs region
 */
void reg_init_channel_map(enum dfs_reg dfs_region);

/**
 * reg_get_psoc_tx_ops() - Get regdb tx ops
 * @psoc: Pointer to psoc structure
 */
struct wlan_lmac_if_reg_tx_ops *reg_get_psoc_tx_ops(
	struct wlan_objmgr_psoc *psoc);

/**
 * reg_is_24ghz_ch_freq() - Check if the given channel frequency is 2.4GHz
 * @freq: Channel frequency
 *
 * Return: true if channel frequency is 2.4GHz, else false
 */
bool reg_is_24ghz_ch_freq(uint32_t freq);

/**
 * reg_is_5ghz_ch_freq() - Check if the given channel frequency is 5GHz
 * @freq: Channel frequency
 *
 * Return: true if channel frequency is 5GHz, else false
 */
bool reg_is_5ghz_ch_freq(uint32_t freq);

/**
 * reg_is_range_overlap_2g() - Check if the given low_freq and high_freq
 * is in the 2G range.
 *
 * @low_freq - Low frequency.
 * @high_freq - High frequency.
 *
 * Return: Return true if given low_freq and high_freq overlaps 2G range,
 * else false.
 */
bool reg_is_range_overlap_2g(qdf_freq_t low_freq, qdf_freq_t high_freq);

/**
 * reg_is_range_overlap_5g() - Check if the given low_freq and high_freq
 * is in the 5G range.
 *
 * @low_freq - Low frequency.
 * @high_freq - High frequency.
 *
 * Return: Return true if given low_freq and high_freq overlaps 5G range,
 * else false.
 */
bool reg_is_range_overlap_5g(qdf_freq_t low_freq, qdf_freq_t high_freq);

/**
 * reg_is_freq_indoor() - Check if the input frequency is an indoor frequency.
 * @pdev: Pointer to pdev.
 * @freq: Channel frequency.
 *
 * Return: Return true if the input frequency is indoor, else false.
 */
bool reg_is_freq_indoor(struct wlan_objmgr_pdev *pdev, qdf_freq_t freq);

#ifdef CONFIG_REG_CLIENT
/**
 * reg_is_freq_indoor_in_secondary_list() - Check if the input frequency is
 * an indoor frequency in the secondary channel list
 * @pdev: Pointer to pdev.
 * @freq: Channel frequency.
 *
 * Return: Return true if the input frequency is indoor, else false.
 */
bool reg_is_freq_indoor_in_secondary_list(struct wlan_objmgr_pdev *pdev,
					  qdf_freq_t freq);
#endif

#ifdef CONFIG_BAND_6GHZ
/**
 * reg_is_6ghz_chan_freq() - Check if the given channel frequency is 6GHz
 * @freq: Channel frequency
 *
 * Return: true if channel frequency is 6GHz, else false
 */
bool reg_is_6ghz_chan_freq(uint16_t freq);

#ifdef CONFIG_6G_FREQ_OVERLAP
/**
 * reg_is_range_only6g() - Check if the given low_freq and high_freq is only in
 * the 6G range.
 *
 * @low_freq - Low frequency.
 * @high_freq - High frequency.
 *
 * Return: Return true if given low_freq and high_freq overlaps only the 6G
 * range, else false.
 */
bool reg_is_range_only6g(qdf_freq_t low_freq, qdf_freq_t high_freq);

/**
 * reg_is_range_overlap_6g() - Check if the given low_freq and high_freq
 * is in the 6G range.
 *
 * @low_freq - Low frequency.
 * @high_freq - High frequency.
 *
 * Return: Return true if given low_freq and high_freq overlaps 6G range,
 * else false.
 */
bool reg_is_range_overlap_6g(qdf_freq_t low_freq, qdf_freq_t high_freq);
#endif

/**
 * REG_IS_6GHZ_FREQ() - Check if the given channel frequency is 6GHz
 * @freq: Channel frequency
 *
 * Return: true if channel frequency is 6GHz, else false
 */
static inline bool REG_IS_6GHZ_FREQ(uint16_t freq)
{
	return ((freq >= REG_MIN_6GHZ_CHAN_FREQ) &&
		(freq <= REG_MAX_6GHZ_CHAN_FREQ));
}

/**
 * reg_is_6ghz_psc_chan_freq() - Check if the given 6GHz channel frequency is
 * preferred scanning channel frequency.
 * @freq: Channel frequency
 *
 * Return: true if given 6GHz channel frequency is preferred scanning channel
 * frequency, else false
 */
bool reg_is_6ghz_psc_chan_freq(uint16_t freq);

/**
 * reg_is_6g_freq_indoor() - Check if a 6GHz frequency is indoor.
 * @pdev: Pointer to pdev.
 * @freq: Channel frequency.
 *
 * Return: Return true if a 6GHz frequency is indoor, else false.
 */
bool reg_is_6g_freq_indoor(struct wlan_objmgr_pdev *pdev, qdf_freq_t freq);

/**
 * reg_get_max_txpower_for_6g_tpe() - Get max txpower for 6G TPE IE.
 * @pdev: Pointer to pdev.
 * @freq: Channel frequency.
 * @bw: Channel bandwidth.
 * @reg_ap: Regulatory 6G AP type.
 * @reg_client: Regulatory 6G client type.
 * @is_psd: True if txpower is needed in PSD format, and false if needed in EIRP
 * format.
 * @tx_power: Pointer to tx-power.
 *
 * Return: Return QDF_STATUS_SUCCESS, if tx_power is filled for 6G TPE IE
 * else return QDF_STATUS_E_FAILURE.
 */
QDF_STATUS reg_get_max_txpower_for_6g_tpe(struct wlan_objmgr_pdev *pdev,
					  qdf_freq_t freq, uint8_t bw,
					  enum reg_6g_ap_type reg_ap,
					  enum reg_6g_client_type reg_client,
					  bool is_psd,
					  uint8_t *tx_power);

/**
 * reg_min_6ghz_chan_freq() - Get minimum 6GHz channel center frequency
 *
 * Return: Minimum 6GHz channel center frequency
 */
uint16_t reg_min_6ghz_chan_freq(void);

/**
 * reg_max_6ghz_chan_freq() - Get maximum 6GHz channel center frequency
 *
 * Return: Maximum 6GHz channel center frequency
 */
uint16_t reg_max_6ghz_chan_freq(void);
#else
static inline bool reg_is_6ghz_chan_freq(uint16_t freq)
{
	return false;
}

static inline bool
reg_is_6g_freq_indoor(struct wlan_objmgr_pdev *pdev, qdf_freq_t freq)
{
	return false;
}

static inline QDF_STATUS
reg_get_max_txpower_for_6g_tpe(struct wlan_objmgr_pdev *pdev,
			       qdf_freq_t freq, uint8_t bw,
			       enum reg_6g_ap_type reg_ap,
			       enum reg_6g_client_type reg_client,
			       bool is_psd,
			       uint8_t *tx_power)
{
	return QDF_STATUS_E_FAILURE;
}

#ifdef CONFIG_6G_FREQ_OVERLAP
static inline bool reg_is_range_overlap_6g(qdf_freq_t low_freq,
					   qdf_freq_t high_freq)
{
	return false;
}

static inline bool reg_is_range_only6g(qdf_freq_t low_freq,
				       qdf_freq_t high_freq)
{
	return false;
}
#endif

static inline bool REG_IS_6GHZ_FREQ(uint16_t freq)
{
	return false;
}

static inline bool reg_is_6ghz_psc_chan_freq(uint16_t freq)
{
	return false;
}

static inline uint16_t reg_min_6ghz_chan_freq(void)
{
	return 0;
}

static inline uint16_t reg_max_6ghz_chan_freq(void)
{
	return 0;
}
#endif /* CONFIG_BAND_6GHZ */

/**
 * reg_get_band_channel_list() - Caller function to
 * reg_get_band_from_cur_chan_list with primary current channel list
 * @pdev: pdev ptr
 * @band_mask: Input bitmap with band set
 * @channel_list: Pointer to Channel List
 *
 * Caller function to reg_get_band_from_cur_chan_listto get the primary channel
 * list and number of channels (for non-beaconing entities).
 *
 * Return: Number of channels, else 0 to indicate error
 */
uint16_t reg_get_band_channel_list(struct wlan_objmgr_pdev *pdev,
				   uint8_t band_mask,
				   struct regulatory_channel *channel_list);

#ifdef CONFIG_REG_CLIENT
/**
 * reg_get_secondary_band_channel_list() - Caller function to
 * reg_get_band_from_cur_chan_list with secondary current channel list
 * @pdev: pdev ptr
 * @band_mask: Input bitmap with band set
 * @channel_list: Pointer to Channel List
 *
 * Caller function to reg_get_band_from_cur_chan_list to get the secondary
 * channel list and number of channels (for beaconing entities).
 *
 * Return: Number of channels, else 0 to indicate error
 */
uint16_t reg_get_secondary_band_channel_list(struct wlan_objmgr_pdev *pdev,
					     uint8_t band_mask,
					     struct regulatory_channel
					     *channel_list);
#endif

/**
 * reg_chan_band_to_freq - Return channel frequency based on the channel number
 * and band.
 * @pdev: pdev ptr
 * @chan: Channel Number
 * @band_mask: Bitmap for bands
 *
 * Return: Return channel frequency or return 0, if the channel is disabled or
 * if the input channel number or band_mask is invalid. Composite bands are
 * supported only for 2.4Ghz and 5Ghz bands. For other bands the following
 * priority is given: 1) 6Ghz 2) 5Ghz 3) 2.4Ghz.
 */
qdf_freq_t reg_chan_band_to_freq(struct wlan_objmgr_pdev *pdev,
				 uint8_t chan,
				 uint8_t band_mask);

/**
 * reg_is_49ghz_freq() - Check if the given channel frequency is 4.9GHz
 * @freq: Channel frequency
 *
 * Return: true if channel frequency is 4.9GHz, else false
 */
bool reg_is_49ghz_freq(qdf_freq_t freq);

/**
 * reg_ch_num() - Get channel number from channel enum
 * @ch_enum: Channel enum
 *
 * Return: channel number
 */
qdf_freq_t reg_ch_num(uint32_t ch_enum);

/**
 * reg_ch_to_freq() - Get channel frequency from channel enum
 * @ch_enum: Channel enum
 *
 * Return: channel frequency
 */
qdf_freq_t reg_ch_to_freq(uint32_t ch_enum);

/**
 * reg_max_5ghz_ch_num() - Get maximum 5GHz channel number
 *
 * Return: Maximum 5GHz channel number
 */
uint8_t reg_max_5ghz_ch_num(void);

#ifdef CONFIG_CHAN_FREQ_API
/**
 * reg_min_24ghz_chan_freq() - Get minimum 2.4GHz channel frequency
 *
 * Return: Minimum 2.4GHz channel frequency
 */
qdf_freq_t reg_min_24ghz_chan_freq(void);

/**
 * reg_max_24ghz_chan_freq() - Get maximum 2.4GHz channel frequency
 *
 * Return: Maximum 2.4GHz channel frequency
 */
qdf_freq_t reg_max_24ghz_chan_freq(void);

/**
 * reg_min_5ghz_chan_freq() - Get minimum 5GHz channel frequency
 *
 * Return: Minimum 5GHz channel frequency
 */
qdf_freq_t reg_min_5ghz_chan_freq(void);

/**
 * reg_max_5ghz_chan_freq() - Get maximum 5GHz channel frequency
 *
 * Return: Maximum 5GHz channel frequency
 */
qdf_freq_t reg_max_5ghz_chan_freq(void);
#endif /* CONFIG_CHAN_FREQ_API */

/**
 * reg_enable_dfs_channels() - Enable the use of DFS channels
 * @pdev: The physical dev to enable/disable DFS channels for
 *
 * Return: QDF_STATUS
 */
QDF_STATUS reg_enable_dfs_channels(struct wlan_objmgr_pdev *pdev, bool enable);

/**
 * reg_is_regdmn_en302502_applicable() - Find if ETSI EN302_502 radar pattern
 * is applicable in current regulatory domain.
 * @pdev: Pdev object pointer.
 *
 * Return: True if en302_502 is applicable, else false.
 */
bool reg_is_regdmn_en302502_applicable(struct wlan_objmgr_pdev *pdev);

/**
 * reg_modify_pdev_chan_range() - Compute current channel list
 * in accordance with the modified reg caps.
 * @pdev: The physical dev for which channel list must be built.
 *
 * Return: QDF_STATUS
 */
QDF_STATUS reg_modify_pdev_chan_range(struct wlan_objmgr_pdev *pdev);

/**
 * reg_update_pdev_wireless_modes() - Update the wireless_modes in the
 * pdev_priv_obj with the input wireless_modes
 * @pdev: pointer to wlan_objmgr_pdev.
 * @wireless_modes: Wireless modes.
 *
 * Return : QDF_STATUS
 */
QDF_STATUS reg_update_pdev_wireless_modes(struct wlan_objmgr_pdev *pdev,
					  uint32_t wireless_modes);

/**
 * reg_get_phybitmap() - Get phybitmap from regulatory pdev_priv_obj
 * @pdev: pdev pointer
 * @phybitmap: pointer to phybitmap
 *
 * Return: QDF STATUS
 */
QDF_STATUS reg_get_phybitmap(struct wlan_objmgr_pdev *pdev,
			     uint16_t *phybitmap);
#ifdef DISABLE_UNII_SHARED_BANDS
/**
 * reg_disable_chan_coex() - Disable Coexisting channels based on the input
 * bitmask.
 * @pdev: pointer to wlan_objmgr_pdev.
 * unii_5g_bitmap: UNII 5G bitmap.
 *
 * Return : QDF_STATUS
 */
QDF_STATUS reg_disable_chan_coex(struct wlan_objmgr_pdev *pdev,
				 uint8_t unii_5g_bitmap);
#endif

#ifdef CONFIG_CHAN_FREQ_API
/**
 * reg_is_freq_present_in_cur_chan_list() - Check the input frequency
 * @pdev: Pointer to pdev
 * @freq: Channel center frequency in MHz
 *
 * Check if the input channel center frequency is present in the current
 * channel list
 *
 * Return: Return true if channel center frequency is present in the current
 * channel list, else return false.
 */
bool
reg_is_freq_present_in_cur_chan_list(struct wlan_objmgr_pdev *pdev,
				     qdf_freq_t freq);

/**
 * reg_get_chan_enum_for_freq() - Get channel enum for given channel frequency
 * @freq: Channel Frequency
 *
 * Return: Channel enum
 */
enum channel_enum reg_get_chan_enum_for_freq(qdf_freq_t freq);

/**
 * reg_get_channel_list_with_power_for_freq() - Provides the channel list with
 * power
 * @pdev: Pointer to pdev
 * @ch_list: Pointer to the channel list.
 * @num_chan: Pointer to save number of channels
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
reg_get_channel_list_with_power_for_freq(struct wlan_objmgr_pdev *pdev,
					 struct channel_power *ch_list,
					 uint8_t *num_chan);

/**
 * reg_get_channel_state_for_freq() - Get channel state from regulatory
 * @pdev: Pointer to pdev
 * @freq: channel center frequency.
 *
 * Return: channel state
 */
enum channel_state reg_get_channel_state_for_freq(struct wlan_objmgr_pdev *pdev,
						  qdf_freq_t freq);

#ifdef CONFIG_REG_CLIENT
/**
 * reg_get_channel_state_from_secondary_list_for_freq() - Get channel state
 * from secondary regulatory current channel list
 * @pdev: Pointer to pdev
 * @freq: channel center frequency.
 *
 * Return: channel state
 */
enum channel_state reg_get_channel_state_from_secondary_list_for_freq(
						struct wlan_objmgr_pdev *pdev,
						qdf_freq_t freq);
#endif

/**
 * reg_get_5g_bonded_channel_state_for_freq() - Get channel state for
 * 5G bonded channel using the channel frequency
 * @pdev: Pointer to pdev
 * @freq: channel center frequency.
 * @bw: channel band width
 *
 * Return: channel state
 */
enum channel_state
reg_get_5g_bonded_channel_state_for_freq(struct wlan_objmgr_pdev *pdev,
					 qdf_freq_t freq,
					 enum phy_ch_width bw);

/**
 * reg_get_2g_bonded_channel_state_for_freq() - Get channel state for 2G
 * bonded channel
 * @freq: channel center frequency.
 * @pdev: Pointer to pdev
 * @oper_ch_freq: Primary channel center frequency
 * @sec_ch_freq: Secondary channel center frequency
 * @bw: channel band width
 *
 * Return: channel state
 */
enum channel_state
reg_get_2g_bonded_channel_state_for_freq(struct wlan_objmgr_pdev *pdev,
					 qdf_freq_t oper_ch_freq,
					 qdf_freq_t sec_ch_freq,
					 enum phy_ch_width bw);

/**
 * reg_set_channel_params_for_freq () - Sets channel parameteres for given
 * bandwidth
 * @pdev: Pointer to pdev
 * @freq: Channel center frequency.
 * @sec_ch_2g_freq: Secondary 2G channel frequency
 * @ch_params: pointer to the channel parameters.
 *
 * Return: None
 */
void reg_set_channel_params_for_freq(struct wlan_objmgr_pdev *pdev,
				     qdf_freq_t freq,
				     qdf_freq_t sec_ch_2g_freq,
				     struct ch_params *ch_params);

/**
 * reg_fill_channel_list() - Fills an array of ch_params (list of
 * channels) for the given channel width and primary freq.
 * If 320 band_center is given, ch_params corresponding to the
 * given band_center is filled.
 *
 * @pdev: Pointer to pdev
 * @freq: Center frequency of the primary channel in MHz
 * @sec_ch_2g_freq: Secondary 2G channel frequency in MHZ
 * @ch_width: Input channel width.
 * @band_center: Center frequency of the 320MHZ channel.
 * @chan_list: Pointer to struct reg_channel_list to be filled (Output).
 * The caller is supposed to provide enough storage for the elements
 * in the list.
 *
 * Return: None
 */
#ifdef WLAN_FEATURE_11BE
void
reg_fill_channel_list(struct wlan_objmgr_pdev *pdev,
		      qdf_freq_t freq,
		      qdf_freq_t sec_ch_2g_freq,
		      enum phy_ch_width ch_width,
		      qdf_freq_t band_center_320,
		      struct reg_channel_list *chan_list);
#endif
/**
 * reg_get_channel_reg_power_for_freq() - Get the txpower for the given channel
 * @pdev: Pointer to pdev
 * @freq: Channel frequency
 *
 * Return: txpower
 */
uint8_t reg_get_channel_reg_power_for_freq(struct wlan_objmgr_pdev *pdev,
					   qdf_freq_t freq);

/**
 * reg_update_nol_ch_for_freq () - Updates NOL channels in current channel list
 * @pdev: pointer to pdev object
 * @chan_freq_list: pointer to NOL channel list
 * @num_ch: No.of channels in list
 * @update_nol: set/reset the NOL status
 *
 * Return: None
 */
void reg_update_nol_ch_for_freq(struct wlan_objmgr_pdev *pdev,
				uint16_t *chan_freq_list,
				uint8_t num_chan,
				bool nol_chan);
/**
 * reg_is_dfs_for_freq () - Checks the channel state for DFS
 * @pdev: pdev ptr
 * @freq: Channel center frequency
 *
 * Return: true or false
 */
bool reg_is_dfs_for_freq(struct wlan_objmgr_pdev *pdev, qdf_freq_t freq);

#ifdef CONFIG_REG_CLIENT
/**
 * reg_is_dfs_in_secondary_list_for_freq() - Checks the channel state for DFS
 * from the secondary channel list
 * @pdev: pdev ptr
 * @freq: Channel center frequency
 *
 * Return: true or false
 */
bool reg_is_dfs_in_secondary_list_for_freq(struct wlan_objmgr_pdev *pdev,
					   qdf_freq_t freq);
#endif

/**
 * reg_chan_freq_is_49ghz() - Check if the input channel center frequency is
 * 4.9GHz
 * @pdev: Pdev pointer
 * @chan_num: Input channel center frequency
 *
 * Return: true if the frequency is 4.9GHz else false.
 */
bool reg_chan_freq_is_49ghz(qdf_freq_t freq);

/**
 * reg_update_nol_history_ch_for_freq() - Set nol-history flag for the channels
 * in the list.
 * @pdev: Pdev ptr.
 * @chan_list: Input channel freqeuncy list.
 * @num_ch: Number of channels.
 * @nol_history_ch: NOL-History flag.
 *
 * Return: void
 */
void reg_update_nol_history_ch_for_freq(struct wlan_objmgr_pdev *pdev,
					uint16_t *chan_list,
					uint8_t num_chan,
					bool nol_history_chan);

/**
 * reg_is_same_5g_band_freqs() - Check if given channel center
 * frequencies have same band
 * @freq1: Channel Center Frequency 1
 * @freq2: Channel Center Frequency 2
 *
 * Return: true if both the frequencies has the same band.
 */
bool reg_is_same_band_freqs(qdf_freq_t freq1, qdf_freq_t freq2);

/**
 * reg_is_frequency_valid_5g_sbs() Check if the given frequency is 5G SBS.
 * @curfreq: current channel frequency
 * @newfreq: new channel center frequency
 *
 * Return: true if the given center frequency is a valid 5G SBS
 */
bool reg_is_frequency_valid_5g_sbs(qdf_freq_t curfreq, qdf_freq_t newfreq);

/**
 * reg_freq_to_band() - Get band from channel frequency
 * @chan_num: channel frequency
 *
 * Return: wifi band
 */
enum reg_wifi_band reg_freq_to_band(qdf_freq_t freq);

/**
 * reg_min_chan_freq() - minimum channel frequency supported
 *
 * Return: channel frequency
 */
qdf_freq_t reg_min_chan_freq(void);

/**
 * reg_max_chan_freq() - maximum channel frequency supported
 *
 * Return: channel frequency
 */
qdf_freq_t reg_max_chan_freq(void);

/**
 * reg_get_5g_bonded_channel_for_freq()- Return the channel state for a
 * 5G or 6G channel frequency based on the channel width and bonded channel
 * @pdev: Pointer to pdev.
 * @freq: Channel center frequency.
 * @ch_width: Channel Width.
 * @bonded_chan_ptr_ptr: Pointer to bonded_channel_freq.
 *
 * Return: Channel State
 */
enum channel_state
reg_get_5g_bonded_channel_for_freq(struct wlan_objmgr_pdev *pdev,
				   uint16_t freq,
				   enum phy_ch_width ch_width,
				   const struct bonded_channel_freq
				   **bonded_chan_ptr_ptr);

/**
 * reg_is_disable_for_freq() - Check if the given channel frequency in
 * disable state
 * @pdev: Pointer to pdev
 * @freq: Channel frequency
 *
 * Return: True if channel state is disabled, else false
 */
bool reg_is_disable_for_freq(struct wlan_objmgr_pdev *pdev, qdf_freq_t freq);

#ifdef CONFIG_REG_CLIENT
/**
 * reg_is_disable_in_secondary_list_for_freq() - Check if the given channel
 * frequency is in disable state
 * @pdev: Pointer to pdev
 * @freq: Channel frequency
 *
 * Return: True if channel state is disabled, else false
 */
bool reg_is_disable_in_secondary_list_for_freq(struct wlan_objmgr_pdev *pdev,
					       qdf_freq_t freq);
#endif

/**
 * reg_is_passive_for_freq() - Check if the given channel frequency is in
 * passive state
 * @pdev: Pointer to pdev
 * @freq: Channel frequency
 *
 * Return: True if channel state is passive, else false
 */
bool reg_is_passive_for_freq(struct wlan_objmgr_pdev *pdev, qdf_freq_t freq);
#endif /* CONFIG_CHAN_FREQ_API */

/**
 * reg_get_max_tx_power() - Get maximum tx power from the current channel list
 * @pdev: Pointer to pdev
 *
 * Return: return the value of the maximum tx power in the current channel list
 *
 */
uint8_t reg_get_max_tx_power(struct wlan_objmgr_pdev *pdev);

/**
 * reg_set_ignore_fw_reg_offload_ind() - Set if regdb offload indication
 * needs to be ignored
 * @psoc: Pointer to psoc
 *
 * Return: QDF_STATUS
 */
QDF_STATUS reg_set_ignore_fw_reg_offload_ind(struct wlan_objmgr_psoc *psoc);

/**
 * reg_get_ignore_fw_reg_offload_ind() - Check whether regdb offload indication
 * needs to be ignored
 *
 * @psoc: Pointer to psoc
 */
bool reg_get_ignore_fw_reg_offload_ind(struct wlan_objmgr_psoc *psoc);

/**
 * reg_set_6ghz_supported() - Set if 6ghz is supported
 *
 * @psoc: Pointer to psoc
 * @val: value
 */
QDF_STATUS reg_set_6ghz_supported(struct wlan_objmgr_psoc *psoc,
				  bool val);

/**
 * reg_set_5dot9_ghz_supported() - Set if 5.9ghz is supported
 *
 * @psoc: Pointer to psoc
 * @val: value
 */
QDF_STATUS reg_set_5dot9_ghz_supported(struct wlan_objmgr_psoc *psoc,
				       bool val);

/**
 * reg_is_6ghz_op_class() - Check whether 6ghz oper class
 *
 * @pdev: Pointer to pdev
 * @op_class: oper class
 */
bool reg_is_6ghz_op_class(struct wlan_objmgr_pdev *pdev,
			  uint8_t op_class);

#ifdef CONFIG_REG_CLIENT
/**
 * reg_is_6ghz_supported() - Whether 6ghz is supported
 *
 * @psoc: pointer to psoc
 */
bool reg_is_6ghz_supported(struct wlan_objmgr_psoc *psoc);
#endif

/**
 * reg_is_5dot9_ghz_supported() - Whether 5.9ghz is supported
 *
 * @psoc: pointer to psoc
 */
bool reg_is_5dot9_ghz_supported(struct wlan_objmgr_psoc *psoc);

/**
 * reg_is_fcc_regdmn () - Checks if the current reg domain is FCC3/FCC8/FCC15/
 * FCC16 or not
 * @pdev: pdev ptr
 *
 * Return: true or false
 */
bool reg_is_fcc_regdmn(struct wlan_objmgr_pdev *pdev);

/**
 * reg_is_5dot9_ghz_freq () - Checks if the frequency is 5.9 GHz freq or not
 * @freq: frequency
 * @pdev: pdev ptr
 *
 * Return: true or false
 */
bool reg_is_5dot9_ghz_freq(struct wlan_objmgr_pdev *pdev, qdf_freq_t freq);

/**
 * reg_is_5dot9_ghz_chan_allowed_master_mode () - Checks if 5.9 GHz channels
 * are allowed in master mode or not.
 *
 * @pdev: pdev ptr
 *
 * Return: true or false
 */
bool reg_is_5dot9_ghz_chan_allowed_master_mode(struct wlan_objmgr_pdev *pdev);

/**
 * reg_get_unii_5g_bitmap() - get unii_5g_bitmap value
 * @pdev: pdev pointer
 * @bitmap: Pointer to retrieve the unii_5g_bitmap of enum reg_unii_band
 *
 * Return: QDF_STATUS
 */
#ifdef DISABLE_UNII_SHARED_BANDS
QDF_STATUS
reg_get_unii_5g_bitmap(struct wlan_objmgr_pdev *pdev, uint8_t *bitmap);
#endif

#ifdef CHECK_REG_PHYMODE
/**
 * reg_get_max_phymode() - Recursively find the best possible phymode given a
 * phymode, a frequency, and per-country regulations
 * @pdev: pdev pointer
 * @phy_in: phymode that the user requested
 * @freq: current operating center frequency
 *
 * Return: maximum phymode allowed in current country that is <= phy_in
 */
enum reg_phymode reg_get_max_phymode(struct wlan_objmgr_pdev *pdev,
				     enum reg_phymode phy_in,
				     qdf_freq_t freq);
#else
static inline enum reg_phymode
reg_get_max_phymode(struct wlan_objmgr_pdev *pdev,
		    enum reg_phymode phy_in,
		    qdf_freq_t freq)
{
	return REG_PHYMODE_INVALID;
}
#endif /* CHECK_REG_PHYMODE */

#ifdef CONFIG_REG_CLIENT
/**
 * reg_band_bitmap_to_band_info() - Convert the band_bitmap to a band_info enum.
 *	Since band_info enum only has combinations for 2G and 5G, 6G is not
 *	considered in this function.
 * @band_bitmap: bitmap on top of reg_wifi_band of bands enabled
 *
 * Return: BAND_ALL if both 2G and 5G band is enabled
 *	BAND_2G if 2G is enabled but 5G isn't
 *	BAND_5G if 5G is enabled but 2G isn't
 */
enum band_info reg_band_bitmap_to_band_info(uint32_t band_bitmap);
#endif

#if defined(CONFIG_BAND_6GHZ)
/**
 * reg_set_cur_6g_ap_pwr_type() - Set the current 6G regulatory AP power type.
 * @pdev: Pointer to PDEV object.
 * @reg_6g_ap_type: Regulatory 6G AP type ie VLPI/LPI/SP.
 *
 * Return: QDF_STATUS_E_INVAL if unable to set and QDF_STATUS_SUCCESS is set.
 */
QDF_STATUS
reg_set_cur_6g_ap_pwr_type(struct wlan_objmgr_pdev *pdev,
			   enum reg_6g_ap_type reg_cur_6g_ap_pwr_type);

/**
 * reg_get_cur_6g_ap_pwr_type() - Get the current 6G regulatory AP power type.
 * @reg_6g_ap_pwr_type: The current regulatory 6G AP type ie VLPI/LPI/SP.
 * subordinate.
 *
 * Return: QDF_STATUS.
 */
QDF_STATUS
reg_get_cur_6g_ap_pwr_type(struct wlan_objmgr_pdev *pdev,
			   enum reg_6g_ap_type *reg_cur_6g_ap_pwr_type);

#ifdef CONFIG_AFC_SUPPORT
/**
 * reg_afc_start() - Start the AFC request from regulatory. This finally
 *                   sends the request to registered callbacks
 * @pdev: Pointer to pdev
 * @req_id: The AFC request ID
 *
 * Return: QDF_STATUS
 */
QDF_STATUS reg_afc_start(struct wlan_objmgr_pdev *pdev, uint64_t req_id);

/**
 * reg_get_partial_afc_req_info() - Get the AFC partial request information
 * @pdev: Pointer to pdev
 * @afc_req: Address of AFC request pointer
 *
 * NOTE:- The memory for AFC request is allocated by the function must be
 *        freed by the caller.
 * Return: QDF_STATUS
 */
QDF_STATUS
reg_get_partial_afc_req_info(struct wlan_objmgr_pdev *pdev,
			     struct wlan_afc_host_partial_request **afc_req);

/**
 * reg_print_partial_afc_req_info() - Print the AFC partial request
 *                                    information
 * @pdev: Pointer to pdev
 * @afc_req: Pointer to AFC request
 *
 * Return: Void
 */
void
reg_print_partial_afc_req_info(struct wlan_objmgr_pdev *pdev,
			       struct wlan_afc_host_partial_request *afc_req);

/**
 * reg_register_afc_req_rx_callback () - add AFC request received callback
 * @pdev: Pointer to pdev
 * @cbf: Pointer to callback handler
 * @arg: Pointer to opaque argument
 *
 * Return: QDF_STATUS
 */
QDF_STATUS reg_register_afc_req_rx_callback(struct wlan_objmgr_pdev *pdev,
					    afc_req_rx_evt_handler cbf,
					    void *arg);

/**
 * reg_unregister_afc_req_rx_callback () - remove AFC request received
 * callback
 * @pdev: Pointer to pdev
 * @cbf: Pointer to callback handler
 *
 * Return: QDF_STATUS
 */
QDF_STATUS reg_unregister_afc_req_rx_callback(struct wlan_objmgr_pdev *pdev,
					      afc_req_rx_evt_handler cbf);
#endif

/**
 * reg_get_cur_6g_client_type() - Get the current 6G regulatory client Type.
 * @pdev: Pointer to PDEV object.
 * @reg_cur_6g_client_mobility_type: The current regulatory 6G client type ie.
 * default/subordinate.
 *
 * Return: QDF_STATUS.
 */
QDF_STATUS
reg_get_cur_6g_client_type(struct wlan_objmgr_pdev *pdev,
			   enum reg_6g_client_type
			   *reg_cur_6g_client_mobility_type);

/**
 * reg_get_rnr_tpe_usable() - Tells if RNR IE is applicable for current domain.
 * @pdev: Pointer to PDEV object.
 * @reg_rnr_tpe_usable: Pointer to hold the bool value, true if RNR IE is
 * applicable, else false.
 *
 * Return: QDF_STATUS.
 */
QDF_STATUS reg_get_rnr_tpe_usable(struct wlan_objmgr_pdev *pdev,
				  bool *reg_rnr_tpe_usable);

/**
 * reg_get_unspecified_ap_usable() - Tells if AP type unspecified by 802.11 can
 * be used or not.
 * @pdev: Pointer to PDEV object.
 * @reg_unspecified_ap_usable: Pointer to hold the bool value, true if
 * unspecified AP types can be used in the IE, else false.
 *
 * Return: QDF_STATUS.
 */
QDF_STATUS reg_get_unspecified_ap_usable(struct wlan_objmgr_pdev *pdev,
					 bool *reg_unspecified_ap_usable);

/**
 * reg_is_6g_psd_power() - Checks if pdev has PSD power
 *
 * @pdev: pdev ptr
 *
 * Return: true if PSD power or false otherwise
 */
bool reg_is_6g_psd_power(struct wlan_objmgr_pdev *pdev);

/**
 * reg_get_6g_chan_ap_power() - Finds the TX power for the given channel
 *	frequency, taking the AP's current power level into account
 *
 * @pdev: pdev ptr
 * @chan_freq: channel frequency
 * @is_psd: is channel PSD or not
 * @tx_power: transmit power to fill for chan_freq
 * @eirp_psd_power: EIRP PSD power, will only be filled if is_psd is true
 *
 * Return: QDF_STATUS
 */
QDF_STATUS reg_get_6g_chan_ap_power(struct wlan_objmgr_pdev *pdev,
				    qdf_freq_t chan_freq, bool *is_psd,
				    uint16_t *tx_power,
				    uint16_t *eirp_psd_power);

/**
 * reg_get_client_power_for_connecting_ap() - Find the channel information when
 *	device is operating as a client
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
 * fill in the parameter is_psd, tx_power, and eirp_psd_power. eirp_psd_power
 * will only be filled if the channel is PSD.
 *
 * Return: QDF_STATUS
 */
QDF_STATUS reg_get_client_power_for_connecting_ap(struct wlan_objmgr_pdev *pdev,
						  enum reg_6g_ap_type ap_type,
						  qdf_freq_t chan_freq,
						  bool *is_psd,
						  uint16_t *tx_power,
						  uint16_t *eirp_psd_power);

/**
 * reg_get_client_power_for_6ghz_ap() - Find the channel information when
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
QDF_STATUS reg_get_client_power_for_6ghz_ap(struct wlan_objmgr_pdev *pdev,
					    enum reg_6g_client_type client_type,
					    qdf_freq_t chan_freq,
					    bool *is_psd, uint16_t *tx_power,
					    uint16_t *eirp_psd_power);

/**
 * reg_set_ap_pwr_and_update_chan_list() - Set the AP power mode and recompute
 * the current channel list
 *
 * @pdev: pdev ptr
 * @ap_pwr_type: the AP power type to update to
 *
 * Return: QDF_STATUS
 */
QDF_STATUS reg_set_ap_pwr_and_update_chan_list(struct wlan_objmgr_pdev *pdev,
					       enum reg_6g_ap_type ap_pwr_type);

/**
 * reg_get_6g_chan_psd_eirp_power() - For a given frequency, get the max PSD
 * from the mas_chan_list
 * @freq: Channel frequency
 * @mas_chan_list: Pointer to mas_chan_list
 * @reg_psd: Pointer to reg_psd
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
reg_get_6g_chan_psd_eirp_power(qdf_freq_t freq,
			       struct regulatory_channel *mas_chan_list,
			       uint16_t *reg_psd);

/**
 * reg_find_txpower_from_6g_list() - For a given frequency, get the max EIRP
 * from the mas_chan_list
 * @freq: Channel frequency
 * @mas_chan_list: Pointer to mas_chan_list
 * @reg_eirp: Pointer to reg_eirp
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
reg_find_txpower_from_6g_list(qdf_freq_t freq,
			      struct regulatory_channel *chan_list,
			      uint16_t *reg_eirp);
#else
static inline QDF_STATUS
reg_set_cur_6g_ap_pwr_type(struct wlan_objmgr_pdev *pdev,
			   enum reg_6g_ap_type reg_cur_6g_ap_pwr_type)
{
	return QDF_STATUS_E_NOSUPPORT;
}

static inline QDF_STATUS
reg_get_cur_6g_ap_pwr_type(struct wlan_objmgr_pdev *pdev,
			   enum reg_6g_ap_type *reg_cur_6g_ap_pwr_type)
{
	*reg_cur_6g_ap_pwr_type = REG_MAX_AP_TYPE;
	return QDF_STATUS_E_NOSUPPORT;
}

static inline QDF_STATUS
reg_get_cur_6g_client_type(struct wlan_objmgr_pdev *pdev,
			   enum reg_6g_client_type
			   *reg_cur_6g_client_mobility_type)
{
	*reg_cur_6g_client_mobility_type = REG_SUBORDINATE_CLIENT;
	return QDF_STATUS_E_NOSUPPORT;
}

static inline
QDF_STATUS reg_get_rnr_tpe_usable(struct wlan_objmgr_pdev *pdev,
				  bool *reg_rnr_tpe_usable)
{
	*reg_rnr_tpe_usable = false;
	return QDF_STATUS_E_NOSUPPORT;
}

static inline
QDF_STATUS reg_get_unspecified_ap_usable(struct wlan_objmgr_pdev *pdev,
					 bool *reg_unspecified_ap_usable)
{
	*reg_unspecified_ap_usable = false;
	return QDF_STATUS_E_NOSUPPORT;
}

static inline
bool reg_is_6g_psd_power(struct wlan_objmgr_pdev *pdev)
{
	return false;
}

static inline
QDF_STATUS reg_get_6g_chan_ap_power(struct wlan_objmgr_pdev *pdev,
				    qdf_freq_t chan_freq, bool *is_psd,
				    uint16_t *tx_power,
				    uint16_t *eirp_psd_power)
{
	*is_psd = false;
	*eirp_psd_power = 0;
	*tx_power = 0;
	return QDF_STATUS_E_NOSUPPORT;
}

static inline
QDF_STATUS reg_get_client_power_for_connecting_ap(struct wlan_objmgr_pdev *pdev,
						  enum reg_6g_ap_type ap_type,
						  qdf_freq_t chan_freq,
						  bool *is_psd,
						  uint16_t *tx_power,
						  uint16_t *eirp_psd_power)
{
	*is_psd = false;
	*tx_power = 0;
	*eirp_psd_power = 0;
	return QDF_STATUS_E_NOSUPPORT;
}

static inline
QDF_STATUS reg_get_client_power_for_6ghz_ap(struct wlan_objmgr_pdev *pdev,
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

static inline
QDF_STATUS reg_set_ap_pwr_and_update_chan_list(struct wlan_objmgr_pdev *pdev,
					       enum reg_6g_ap_type ap_pwr_type)
{
	return QDF_STATUS_E_NOSUPPORT;
}

static inline QDF_STATUS
reg_get_6g_chan_psd_eirp_power(qdf_freq_t freq,
			       struct regulatory_channel *mas_chan_list,
			       uint16_t *eirp_psd_power)
{
	*eirp_psd_power = 0;
	return QDF_STATUS_E_NOSUPPORT;
}

static inline QDF_STATUS
reg_find_txpower_from_6g_list(qdf_freq_t freq,
			      struct regulatory_channel *chan_list,
			      uint16_t *reg_eirp)
{
	*reg_eirp = 0;
	return QDF_STATUS_E_NOSUPPORT;
}
#endif

#ifdef CONFIG_HOST_FIND_CHAN
/**
 * reg_update_max_phymode_chwidth_for_pdev() - Update the maximum phymode
 * and the corresponding chwidth for the pdev.
 * @pdev: Pointer to PDEV object.
 *
 */
void reg_update_max_phymode_chwidth_for_pdev(struct wlan_objmgr_pdev *pdev);

/**
 * reg_modify_chan_list_for_max_chwidth() - Update the maximum bandwidth for
 * each channel in the current channel list.
 * @pdev: Pointer to PDEV object.
 * @cur_chan_list: Pointer to the pdev current channel list.
 *
 * In countries like DK, the channel 144 is not supported by the regulatory.
 * When we get the regulatory rules, the entire UNII-2E's max bandwidth is set
 * to 160MHz but this is only true for channel 100 to 128. Channels 132 and
 * and 136 will have maximum bandwidth of 40MHz and channel 140 will have a
 * max bandwidth value of 20MHz (since 144 is not available).
 * These values in the current channel list are not updated based on the
 * bonded channels and hence will have an incorrect value for particular
 * channels.
 * Use this API to update the maximum bandwidth based on the device
 * capabilities and the availability of adjacent channels.
 */
void
reg_modify_chan_list_for_max_chwidth(struct wlan_objmgr_pdev *pdev,
				     struct regulatory_channel *cur_chan_list);

#else
static inline void
reg_update_max_phymode_chwidth_for_pdev(struct wlan_objmgr_pdev *pdev)
{
}

static inline void
reg_modify_chan_list_for_max_chwidth(struct wlan_objmgr_pdev *pdev,
				     struct regulatory_channel *cur_chan_list)
{
}

#endif /* CONFIG_HOST_FIND_CHAN */

/**
 * reg_is_phymode_unallowed() - Check if requested phymode is unallowed
 * @phy_in: phymode that the user requested
 * @phymode_bitmap: bitmap of unallowed phymodes for specific country
 *
 * Return: true if phymode is not allowed, else false
 */
bool reg_is_phymode_unallowed(enum reg_phymode phy_in, uint32_t phymode_bitmap);

/*
 * reg_is_regdb_offloaded() - is regdb offloaded
 * @psoc: Pointer to psoc object
 *
 * Return: true if regdb is offloaded, else false
 */
bool reg_is_regdb_offloaded(struct wlan_objmgr_psoc *psoc);

/**
 * reg_set_ext_tpc_supported() - Set if FW supports new WMI command for TPC
 * @psoc: Pointer to psoc
 * @val: value
 *
 * Return: QDF_STATUS
 */
QDF_STATUS reg_set_ext_tpc_supported(struct wlan_objmgr_psoc *psoc,
				     bool val);

/**
 * reg_is_ext_tpc_supported() - Whether FW supports new WMI command for TPC
 *
 * @psoc: pointer to psoc
 *
 * Return: true if FW supports the new TPC command, else false
 */
bool reg_is_ext_tpc_supported(struct wlan_objmgr_psoc *psoc);

/**
 * reg_get_bonded_chan_entry() - Fetch the bonded channel pointer given a
 * frequency and channel width.
 * @freq: Input frequency.
 * @chwidth: Input channel width.
 *
 * Return: A valid bonded channel pointer if found, else NULL.
 */
const struct bonded_channel_freq *
reg_get_bonded_chan_entry(qdf_freq_t freq, enum phy_ch_width chwidth);

/**
 * reg_set_2g_channel_params_for_freq() - set the 2.4G bonded channel parameters
 * @oper_freq: operating channel
 * @ch_params: channel parameters
 * @sec_ch_2g_freq: 2.4G secondary channel
 *
 * Return: void
 */
void reg_set_2g_channel_params_for_freq(struct wlan_objmgr_pdev *pdev,
					uint16_t oper_freq,
					struct ch_params *ch_params,
					uint16_t sec_ch_2g_freq);

/**
 * reg_combine_channel_states() - Get minimum of channel state1 and state2
 * @chan_state1: Channel state1
 * @chan_state2: Channel state2
 *
 * Return: Channel state
 */
enum channel_state reg_combine_channel_states(enum channel_state chan_state1,
					      enum channel_state chan_state2);

#if defined(CONFIG_BAND_6GHZ)
/**
 * reg_set_lower_6g_edge_ch_supp() - Set if lower 6ghz edge channel is
 * supported by FW
 *
 * @psoc: Pointer to psoc
 * @val: value
 */
QDF_STATUS reg_set_lower_6g_edge_ch_supp(struct wlan_objmgr_psoc *psoc,
					 bool val);

/**
 * reg_set_disable_upper_6g_edge_ch_supp() - Set if upper 6ghz edge channel is
 * disabled by FW
 *
 * @psoc: Pointer to psoc
 * @val: value
 */
QDF_STATUS
reg_set_disable_upper_6g_edge_ch_supp(struct wlan_objmgr_psoc *psoc,
				      bool val);

/**
 * reg_is_lower_6g_edge_ch_supp() - Check whether 6GHz lower edge channel
 * (5935 MHz) is supported.
 * @psoc: pointer to psoc
 *
 * Return: true if edge channels are supported, else false
 */
bool reg_is_lower_6g_edge_ch_supp(struct wlan_objmgr_psoc *psoc);

/**
 * reg_is_upper_6g_edge_ch_disabled() - Check whether 6GHz upper edge
 * channel (7115 MHz) is disabled.
 * @psoc: pointer to psoc
 *
 * Return: true if edge channels are supported, else false
 */
bool reg_is_upper_6g_edge_ch_disabled(struct wlan_objmgr_psoc *psoc);
#endif

#ifdef FEATURE_WLAN_CH_AVOID_EXT
/**
 * reg_process_ch_avoid_ext_event() - Process channel avoid extended event
 * @psoc: psoc for country information
 * @ch_avoid_event: channel avoid extended event buffer
 *
 * Return: QDF_STATUS
 */

QDF_STATUS
reg_process_ch_avoid_ext_event(struct wlan_objmgr_psoc *psoc,
			       struct ch_avoid_ind_type *ch_avoid_event);
#else
static inline QDF_STATUS
reg_process_ch_avoid_ext_event(struct wlan_objmgr_psoc *psoc,
			       struct ch_avoid_ind_type *ch_avoid_event)
{
	return QDF_STATUS_SUCCESS;
}
#endif

#if defined(CONFIG_AFC_SUPPORT) && defined(CONFIG_BAND_6GHZ)
/**
 * reg_send_afc_cmd() - Send AFC cmd to the FW
 * @pdev: pdev ptr
 * @afc_ind_obj: Pointer to hold AFC indication
 *
 * Return: QDF_STATUS_SUCCESS if the WMI command is sent or QDF_STATUS_E_FAILURE
 * otherwise
 */
QDF_STATUS reg_send_afc_cmd(struct wlan_objmgr_pdev *pdev,
			    struct reg_afc_resp_rx_ind_info *afc_ind_obj);

/**
 * reg_is_afc_power_event_received() - Checks if AFC power event is
 * received from the FW.
 *
 * @pdev: pdev ptr
 *
 * Return: true if AFC power event is received from the FW or false otherwise
 */
bool reg_is_afc_power_event_received(struct wlan_objmgr_pdev *pdev);

/**
 * reg_get_afc_req_id() - Get the AFC request ID
 * @pdev: pdev pointer
 * @req_id: Pointer to request id
 *
 * Return: QDF_STATUS
 */
QDF_STATUS reg_get_afc_req_id(struct wlan_objmgr_pdev *pdev, uint64_t *req_id);

/**
 * reg_is_afc_expiry_event_received() - Checks if AFC power event is
 * received from the FW.
 *
 * @pdev: pdev ptr
 *
 * Return: true if AFC expiry event is received from the FW or false otherwise
 */
bool reg_is_afc_expiry_event_received(struct wlan_objmgr_pdev *pdev);

/**
 * reg_is_noaction_on_afc_pwr_evt() - Checks if the regulatory module
 * needs to take action when AFC power event is received.
 *
 * @pdev: pdev ptr
 *
 * Return: true if regulatory should not take any action or false otherwise
 */
bool reg_is_noaction_on_afc_pwr_evt(struct wlan_objmgr_pdev *pdev);

/**
 * reg_dmn_set_afc_req_id() - Set the request ID in the AFC partial request
 *                            object
 * @afc_req: pointer to AFC partial request
 * @req_id: AFC request ID
 *
 * Return: Void
 */
void reg_dmn_set_afc_req_id(struct wlan_afc_host_partial_request *afc_req,
			    uint64_t req_id);
#endif

/**
 * reg_is_chwidth_supported() - Check if given channel width is supported
 * on a given pdev
 * @pdev: pdev pointer
 * @ch_width: channel width.
 * @is_supported: whether the channel width is supported
 *
 * Return QDF_STATUS_SUCCESS of operation
 */
QDF_STATUS reg_is_chwidth_supported(struct wlan_objmgr_pdev *pdev,
				    enum phy_ch_width ch_width,
				    bool *is_supported);
#endif
