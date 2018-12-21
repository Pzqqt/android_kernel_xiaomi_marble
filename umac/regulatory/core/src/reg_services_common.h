/*
 * Copyright (c) 2017-2019 The Linux Foundation. All rights reserved.
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

#define IS_VALID_PSOC_REG_OBJ(psoc_priv_obj) (psoc_priv_obj)
#define IS_VALID_PDEV_REG_OBJ(pdev_priv_obj) (pdev_priv_obj)

#define REG_MIN_24GHZ_CH_NUM channel_map[MIN_24GHZ_CHANNEL].chan_num
#define REG_MAX_24GHZ_CH_NUM channel_map[MAX_24GHZ_CHANNEL].chan_num
#define REG_MIN_5GHZ_CH_NUM channel_map[MIN_5GHZ_CHANNEL].chan_num
#define REG_MAX_5GHZ_CH_NUM channel_map[MAX_5GHZ_CHANNEL].chan_num

#define REG_IS_24GHZ_CH(chan_num) \
	(((chan_num) >= REG_MIN_24GHZ_CH_NUM) &&	\
	 ((chan_num) <= REG_MAX_24GHZ_CH_NUM))

#define REG_MIN_24GHZ_CH_FREQ channel_map[MIN_24GHZ_CHANNEL].center_freq
#define REG_MAX_24GHZ_CH_FREQ channel_map[MAX_24GHZ_CHANNEL].center_freq

#define REG_IS_24GHZ_CH_FREQ(freq) \
	(((freq) >= REG_MIN_24GHZ_CH_FREQ) &&   \
	((freq) <= REG_MAX_24GHZ_CH_FREQ))

#ifndef CONFIG_LEGACY_CHAN_ENUM
#define REG_MIN_49GHZ_CH_FREQ channel_map[MIN_49GHZ_CHANNEL].center_freq
#define REG_MAX_49GHZ_CH_FREQ channel_map[MAX_49GHZ_CHANNEL].center_freq

#define REG_IS_49GHZ_FREQ(freq) \
	(((freq) >= REG_MIN_49GHZ_CH_FREQ) &&   \
	((freq) <= REG_MAX_49GHZ_CH_FREQ))
#endif

#define REG_IS_5GHZ_CH(chan_num) \
	(((chan_num) >= REG_MIN_5GHZ_CH_NUM) &&	\
	 ((chan_num) <= REG_MAX_5GHZ_CH_NUM))

#define REG_IS_5GHZ_FREQ(freq) \
	(((freq) >= channel_map[MIN_5GHZ_CHANNEL].center_freq) &&	\
	 ((freq) <= channel_map[MAX_5GHZ_CHANNEL].center_freq))

#define REG_CH_NUM(ch_enum) channel_map[ch_enum].chan_num
#define REG_CH_TO_FREQ(ch_enum) channel_map[ch_enum].center_freq

/* EEPROM setting is a country code */
#define    COUNTRY_ERD_FLAG     0x8000

extern const struct chan_map *channel_map;

/**
 * reg_get_chan_enum() - Get channel enum for given channel number
 * @chan_num: Channel number
 *
 * Return: Channel enum
 */
enum channel_enum reg_get_chan_enum(uint32_t chan_num);

/**
 * reg_get_channel_list_with_power() - Provides the channel list with power
 * @pdev: Pointer to pdev
 * @ch_list: Pointer to the channel list.
 * @num_chan: Pointer to save number of channels
 *
 * Return: QDF_STATUS
 */
QDF_STATUS reg_get_channel_list_with_power(struct wlan_objmgr_pdev *pdev,
					   struct channel_power *ch_list,
					   uint8_t *num_chan);

/**
 * reg_get_channel_state() - Get channel state from regulatory
 * @pdev: Pointer to pdev
 * @ch: channel number.
 *
 * Return: channel state
 */
enum channel_state reg_get_channel_state(struct wlan_objmgr_pdev *pdev,
					 uint32_t ch);

/**
 * reg_get_5g_bonded_channel_state() - Get channel state for 5G bonded channel
 * @pdev: Pointer to pdev
 * @ch: channel number.
 * @bw: channel band width
 *
 * Return: channel state
 */
enum channel_state reg_get_5g_bonded_channel_state(
		struct wlan_objmgr_pdev *pdev, uint8_t ch,
		enum phy_ch_width bw);

/**
 * reg_get_2g_bonded_channel_state() - Get channel state for 2G bonded channel
 * @ch: channel number.
 * @pdev: Pointer to pdev
 * @oper_ch: Primary channel number
 * @sec_ch: Secondary channel number
 * @bw: channel band width
 *
 * Return: channel state
 */
enum channel_state reg_get_2g_bonded_channel_state(
		struct wlan_objmgr_pdev *pdev, uint8_t oper_ch, uint8_t sec_ch,
		enum phy_ch_width bw);

/**
 * reg_set_channel_params () - Sets channel parameteres for given bandwidth
 * @pdev: Pointer to pdev
 * @ch: channel number.
 * @sec_ch_2g: Secondary 2G channel
 * @ch_params: pointer to the channel parameters.
 *
 * Return: None
 */
void reg_set_channel_params(struct wlan_objmgr_pdev *pdev,
			    uint8_t ch, uint8_t sec_ch_2g,
			    struct ch_params *ch_params);

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
 * reg_get_current_dfs_region () - Get the current dfs region
 * @pdev: Pointer to pdev
 * @dfs_reg: pointer to dfs region
 *
 * Return: None
 */
void reg_get_current_dfs_region(struct wlan_objmgr_pdev *pdev,
				enum dfs_reg *dfs_reg);

/**
 * reg_get_channel_reg_power() - Get the txpower for the given channel
 * @pdev: Pointer to pdev
 * @chan_num: Channel number
 *
 * Return: txpower
 */
uint32_t reg_get_channel_reg_power(struct wlan_objmgr_pdev *pdev,
				   uint32_t chan_num);

/**
 * reg_get_channel_freq() - Get the channel frequency
 * @pdev: Pointer to pdev
 * @chan_num: Channel number
 *
 * Return: frequency
 */
uint32_t reg_get_channel_freq(struct wlan_objmgr_pdev *pdev,
			      uint32_t chan_num);

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
 * reg_chan_to_band() - Get band from channel number
 * @chan_num: channel number
 *
 * Return: band info
 */
enum band_info reg_chan_to_band(uint32_t chan_num);

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
 * reg_update_nol_ch () - Updates NOL channels in current channel list
 * @pdev: pointer to pdev object
 * @ch_list: pointer to NOL channel list
 * @num_ch: No.of channels in list
 * @update_nol: set/reset the NOL status
 *
 * Return: None
 */
void reg_update_nol_ch(struct wlan_objmgr_pdev *pdev, uint8_t *chan_list,
		       uint8_t num_chan, bool nol_chan);

/**
 * reg_is_dfs_ch () - Checks the channel state for DFS
 * @pdev: pdev ptr
 * @chan: channel
 *
 * Return: true or false
 */
bool reg_is_dfs_ch(struct wlan_objmgr_pdev *pdev, uint32_t chan);

/**
 * reg_freq_to_chan() - Get channel number from frequency.
 * @pdev: Pointer to pdev
 * @freq: Channel frequency
 *
 * Return: Channel number
 */
uint32_t reg_freq_to_chan(struct wlan_objmgr_pdev *pdev, uint32_t freq);

/**
 * reg_chan_to_freq() - Get frequency from channel number
 * @pdev: Pointer to pdev
 * @chan_num: Channel number
 *
 * Return: Channel frequency
 */
uint32_t reg_chan_to_freq(struct wlan_objmgr_pdev *pdev, uint32_t chan_num);

/**
 * reg_chan_is_49ghz() - Check if the input channel number is 4.9GHz
 * @pdev: Pdev pointer
 * @chan_num: Input channel number
 *
 * Return: true if the channel is 4.9GHz else false.
 */
bool reg_chan_is_49ghz(struct wlan_objmgr_pdev *pdev, uint8_t chan_num);

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
 * reg_get_curr_band() - Get current band
 * @pdev: Pdev pointer
 * @band: Pointer to save the current band
 *
 * Return: QDF_STATUS
 */
QDF_STATUS reg_get_curr_band(struct wlan_objmgr_pdev *pdev,
			     enum band_info *band);

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
		       uint32_t low_freq_2g, uint32_t high_freq_2g,
		       uint32_t low_freq_5g, uint32_t high_freq_5g,
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
 * reg_update_nol_history_ch() - Set nol-history flag for the channels in the
 * list.
 * @pdev: Pdev ptr.
 * @ch_list: Input channel list.
 * @num_ch: Number of channels.
 * @nol_history_ch: NOL-History flag.
 *
 * Return: void
 */
void reg_update_nol_history_ch(struct wlan_objmgr_pdev *pdev,
			       uint8_t *chan_list,
			       uint8_t num_chan,
			       bool nol_history_chan);

/**
 * reg_is_24ghz_ch() - Check if the given channel number is 2.4GHz
 * @chan: Channel number
 *
 * Return: true if channel number is 2.4GHz, else false
 */
bool reg_is_24ghz_ch(uint32_t chan);

/**
 * reg_is_5ghz_ch() - Check if the given channel number is 5GHz
 * @chan: Channel number
 *
 * Return: true if channel number is 5GHz, else false
 */
bool reg_is_5ghz_ch(uint32_t chan);

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
bool reg_is_5ghz_ch_freq(uint32_t chan);

#ifndef CONFIG_LEGACY_CHAN_ENUM
/**
 * reg_is_49ghz_freq() - Check if the given channel frequency is 4.9GHz
 * @freq: Channel frequency
 *
 * Return: true if channel frequency is 4.9GHz, else false
 */
bool reg_is_49ghz_freq(uint32_t freq);
#endif

/**
 * reg_ch_num() - Get channel number from channel enum
 * @ch_enum: Channel enum
 *
 * Return: channel number
 */
uint32_t reg_ch_num(uint32_t ch_enum);

/**
 * reg_ch_to_freq() - Get channel frequency from channel enum
 * @ch_enum: Channel enum
 *
 * Return: channel frequency
 */
uint32_t reg_ch_to_freq(uint32_t ch_enum);

/**
 * reg_is_same_band_channels() - Check if given channel numbers have same band
 * @chan_num1: Channel number1
 * @chan_num2: Channel number2
 *
 * Return: true if both the channels has the same band.
 */
bool reg_is_same_band_channels(uint32_t chan_num1, uint32_t chan_num2);

/**
 * reg_is_channel_valid_5g_sbs() Check if the given channel is 5G SBS.
 * @curchan: current channel
 * @newchan:new channel
 *
 * Return: true if the given channel is a valid 5G SBS
 */
bool reg_is_channel_valid_5g_sbs(uint32_t curchan, uint32_t newchan);

/**
 * reg_min_24ghz_ch_num() - Get minimum 2.4GHz channel number
 *
 * Return: Minimum 2.4GHz channel number
 */
uint32_t reg_min_24ghz_ch_num(void);

/**
 * reg_max_24ghz_ch_num() - Get maximum 2.4GHz channel number
 *
 * Return: Maximum 2.4GHz channel number
 */
uint32_t reg_max_24ghz_ch_num(void);

/**
 * reg_min_5ghz_ch_num() - Get minimum 5GHz channel number
 *
 * Return: Minimum 5GHz channel number
 */
uint32_t reg_min_5ghz_ch_num(void);

/**
 * reg_max_5ghz_ch_num() - Get maximum 5GHz channel number
 *
 * Return: Maximum 5GHz channel number
 */
uint32_t reg_max_5ghz_ch_num(void);
#endif
