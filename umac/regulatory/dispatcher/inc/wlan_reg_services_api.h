/*
 * Copyright (c) 2017-2019 The Linux Foundation. All rights reserved.
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

#ifdef CONFIG_CHAN_NUM_API
/**
 * wlan_reg_min_24ghz_ch_num() - Get minimum 2.4GHz channel number
 *
 * Return: Minimum 2.4GHz channel number
 */
#define WLAN_REG_MIN_24GHZ_CH_NUM wlan_reg_min_24ghz_ch_num()
uint8_t wlan_reg_min_24ghz_ch_num(void);

/**
 * wlan_reg_max_24ghz_ch_num() - Get maximum 2.4GHz channel number
 *
 * Return: Maximum 2.4GHz channel number
 */
#define WLAN_REG_MAX_24GHZ_CH_NUM wlan_reg_max_24ghz_ch_num()
uint8_t wlan_reg_max_24ghz_ch_num(void);

/**
 * wlan_reg_min_5ghz_ch_num() - Get minimum 5GHz channel number
 *
 * Return: Minimum 5GHz channel number
 */
#define WLAN_REG_MIN_5GHZ_CH_NUM wlan_reg_min_5ghz_ch_num()
uint8_t wlan_reg_min_5ghz_ch_num(void);

/**
 * wlan_reg_max_5ghz_ch_num() - Get maximum 5GHz channel number
 *
 * Return: Maximum 5GHz channel number
 */
#define WLAN_REG_MAX_5GHZ_CH_NUM wlan_reg_max_5ghz_ch_num()
uint8_t wlan_reg_max_5ghz_ch_num(void);
#endif /* CONFIG_CHAN_NUM_API */

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

#ifdef CONFIG_CHAN_NUM_API
/**
 * wlan_reg_is_24ghz_ch() - Check if the given channel number is 2.4GHz
 * @chan: Channel number
 *
 * Return: true if channel number is 2.4GHz, else false
 */
#define WLAN_REG_IS_24GHZ_CH(chan) wlan_reg_is_24ghz_ch(chan)
bool wlan_reg_is_24ghz_ch(uint8_t chan);

/**
 * wlan_reg_is_5ghz_ch() - Check if the given channel number is 5GHz
 * @chan: Channel number
 *
 * Return: true if channel number is 5GHz, else false
 */
#define WLAN_REG_IS_5GHZ_CH(chan) wlan_reg_is_5ghz_ch(chan)
bool wlan_reg_is_5ghz_ch(uint8_t chan);
#endif /* CONFIG_CHAN_NUM_API */

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

#ifdef CONFIG_BAND_6GHZ
/**
 * wlan_reg_is_6ghz_chan_freq() - Check if the given channel frequency is 6GHz
 * @freq: Channel frequency
 *
 * Return: true if channel frequency is 6GHz, else false
 */
#define WLAN_REG_IS_6GHZ_CHAN_FREQ(freq) wlan_reg_is_6ghz_chan_freq(freq)
bool wlan_reg_is_6ghz_chan_freq(uint16_t freq);

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

#else

#define WLAN_REG_IS_6GHZ_CHAN_FREQ(freq) (false)
static inline bool wlan_reg_is_6ghz_chan_freq(uint16_t freq)
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

/**
 * wlan_reg_is_49ghz_freq() - Check if the given channel frequency is 4.9GHz
 * @freq: Channel frequency
 *
 * Return: true if channel frequency is 4.9GHz, else false
 */
#define WLAN_REG_IS_49GHZ_FREQ(freq) wlan_reg_is_49ghz_freq(freq)
bool wlan_reg_is_49ghz_freq(qdf_freq_t freq);

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

#ifdef CONFIG_CHAN_NUM_API
/**
 * wlan_reg_is_same_band_channels() - Check if given channel numbers have same
 * band
 * @chan_num1: Channel number1
 * @chan_num2: Channel number2
 *
 * Return: true if both the channels has the same band.
 */
#define WLAN_REG_IS_SAME_BAND_CHANNELS(chan_num1, chan_num2) \
	wlan_reg_is_same_band_channels(chan_num1, chan_num2)
bool wlan_reg_is_same_band_channels(uint8_t chan_num1, uint8_t chan_num2);

/**
 * wlan_reg_is_channel_valid_5g_sbs() Check if the given channel is 5G SBS.
 * @curchan: current channel
 * @newchan:new channel
 *
 * Return: true if the given channel is a valid 5G SBS
 */
#define WLAN_REG_IS_CHANNEL_VALID_5G_SBS(curchan, newchan) \
	wlan_reg_is_channel_valid_5g_sbs(curchan, newchan)
bool wlan_reg_is_channel_valid_5g_sbs(uint8_t curchan, uint8_t newchan);
#endif /* CONFIG_CHAN_NUM_API */


#ifdef CONFIG_CHAN_NUM_API
/**
 * wlan_reg_chan_to_band() - Get band from channel number
 * @chan_num: channel number
 *
 * Return: band info
 */
#define WLAN_REG_CHAN_TO_BAND(chan_num)  wlan_reg_chan_to_band(chan_num)
enum band_info wlan_reg_chan_to_band(uint8_t chan_num);

/**
 * wlan_reg_get_channel_list_with_power() - Provide the channel list with power
 * @ch_list: pointer to the channel list.
 *
 * Return: QDF_STATUS
 */
QDF_STATUS wlan_reg_get_channel_list_with_power(struct wlan_objmgr_pdev *pdev,
						struct channel_power *ch_list,
						uint8_t *num_chan);
#endif /* CONFIG_CHAN_NUM_API */
/**
 * wlan_reg_read_default_country() - Read the default country for the regdomain
 * @country: pointer to the country code.
 *
 * Return: QDF_STATUS
 */
QDF_STATUS wlan_reg_read_default_country(struct wlan_objmgr_psoc *psoc,
				   uint8_t *country);

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

#ifdef CONFIG_CHAN_NUM_API
/**
 * wlan_reg_chan_has_dfs_attribute() - check channel has dfs attribute flag
 * @ch: channel number.
 *
 * This API get chan initial dfs attribute from regdomain
 *
 * Return: true if chan is dfs, otherwise false
 */
bool
wlan_reg_chan_has_dfs_attribute(struct wlan_objmgr_pdev *pdev, uint8_t ch);

/**
 * wlan_reg_is_etsi13_srd_chan () - Checks if the ch is ETSI13 srd ch or not
 * @pdev: pdev ptr
 * @chan_num: channel
 *
 * Return: true or false
 */
bool wlan_reg_is_etsi13_srd_chan(struct wlan_objmgr_pdev *pdev,
				 uint8_t chan_num);
#endif /* CONFIG_CHAN_NUM_API */

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

#ifdef CONFIG_CHAN_NUM_API
/**
 * wlan_reg_get_chan_enum() - Get channel enum for given channel number
 * @chan_num: Channel number
 *
 * Return: Channel enum
 */
enum channel_enum wlan_reg_get_chan_enum(uint8_t chan_num);

/**
 * wlan_reg_get_channel_state() - Get channel state from regulatory
 * @ch: channel number.
 *
 * Return: channel state
 */
enum channel_state wlan_reg_get_channel_state(struct wlan_objmgr_pdev *pdev,
					      uint8_t ch);

/**
 * wlan_reg_get_5g_bonded_channel_state() - Get 5G bonded channel state
 * @pdev: The physical dev to program country code or regdomain
 * @ch: channel number.
 * @bw: channel band width
 *
 * Return: channel state
 */
enum channel_state wlan_reg_get_5g_bonded_channel_state(
		struct wlan_objmgr_pdev *pdev, uint8_t ch,
		enum phy_ch_width bw);

/**
 * wlan_reg_get_2g_bonded_channel_state() - Get 2G bonded channel state
 * @pdev: The physical dev to program country code or regdomain
 * @ch: channel number.
 * @sec_ch: Secondary channel.
 * @bw: channel band width
 *
 * Return: channel state
 */
enum channel_state wlan_reg_get_2g_bonded_channel_state(
		struct wlan_objmgr_pdev *pdev, uint8_t ch,
		uint8_t sec_ch, enum phy_ch_width bw);

/**
 * wlan_reg_set_channel_params () - Sets channel parameteres for given bandwidth
 * @pdev: The physical dev to program country code or regdomain
 * @ch: channel number.
 * @sec_ch_2g: Secondary channel.
 * @ch_params: pointer to the channel parameters.
 *
 * Return: None
 */
void wlan_reg_set_channel_params(struct wlan_objmgr_pdev *pdev, uint8_t ch,
				 uint8_t sec_ch_2g,
				 struct ch_params *ch_params);
#endif /* CONFIG_CHAN_NUM_API */
/**
 * wlan_reg_get_dfs_region () - Get the current dfs region
 * @dfs_reg: pointer to dfs region
 *
 * Return: Status
 */
QDF_STATUS wlan_reg_get_dfs_region(struct wlan_objmgr_pdev *pdev,
			     enum dfs_reg *dfs_reg);

#ifdef CONFIG_CHAN_NUM_API
/**
 * wlan_reg_get_channel_reg_power() - Provide the channel regulatory power
 * @chan_num: channel number
 *
 * Return: int
 */
uint32_t wlan_reg_get_channel_reg_power(struct wlan_objmgr_pdev *pdev,
					uint8_t chan_num);

/**
 * wlan_reg_get_channel_freq() - provide the channel center freq
 * @chan_num: channel number
 *
 * Return: int
 */
qdf_freq_t wlan_reg_get_channel_freq(struct wlan_objmgr_pdev *pdev,
				     uint8_t chan_num);
#endif /* CONFIG_CHAN_NUM_API */

/**
 * wlan_reg_get_current_chan_list() - provide the pdev current channel list
 * @pdev: pdev pointer
 * @chan_list: channel list pointer
 *
 * Return: QDF_STATUS
 */
QDF_STATUS wlan_reg_get_current_chan_list(struct wlan_objmgr_pdev *pdev,
		struct regulatory_channel *chan_list);

#ifdef CONFIG_CHAN_NUM_API
/**
 * wlan_reg_get_bonded_channel_state() - get bonded channel state
 * @pdev: pdev ptr
 * @ch: chennal number
 * @bw: chennal number
 * @sec_ch: secondary channel
 *
 * Return: enum channel_state
 */
enum channel_state wlan_reg_get_bonded_channel_state(
	struct wlan_objmgr_pdev *pdev, uint8_t ch,
	enum phy_ch_width bw, uint8_t sec_ch);
#endif /* CONFIG_CHAN_NUM_API */

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

#ifdef CONFIG_CHAN_NUM_API
/**
 * wlan_reg_update_nol_ch () - set nol channel
 * @pdev: pdev ptr
 * @ch_list: channel list to be returned
 * @num_ch: number of channels
 * @nol_ch: nol flag
 *
 * Return: void
 */
void wlan_reg_update_nol_ch(struct wlan_objmgr_pdev *pdev,
			    uint8_t *ch_list,
			    uint8_t num_ch,
			    bool nol_ch);

/**
 * wlan_reg_is_dfs_ch () - Checks the channel state for DFS
 * @pdev: pdev ptr
 * @chan: channel
 *
 * Return: true or false
 */
bool wlan_reg_is_dfs_ch(struct wlan_objmgr_pdev *pdev, uint8_t chan);

/**
 * wlan_reg_is_dsrc_chan () - Checks if the channel is dsrc channel or not
 * @pdev: pdev ptr
 * @chan_num: channel
 *
 * Return: true or false
 */
bool wlan_reg_is_dsrc_chan(struct wlan_objmgr_pdev *pdev, uint8_t chan_num);

/**
 * wlan_reg_is_passive_or_disable_ch () - Checks chan state for passive
 * and disabled
 * @pdev: pdev ptr
 * @chan: channel
 *
 * Return: true or false
 */
bool wlan_reg_is_passive_or_disable_ch(struct wlan_objmgr_pdev *pdev,
				       uint8_t chan);

/**
 * wlan_reg_is_disable_ch () - Checks chan state for disabled
 * @pdev: pdev ptr
 * @chan: channel
 *
 * Return: true or false
 */
bool wlan_reg_is_disable_ch(struct wlan_objmgr_pdev *pdev, uint8_t chan);
#endif /* CONFIG_CHAN_NUM_API */

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
 * wlan_reg_chan_to_freq () - convert channel number to frequency
 * @chan: channel number
 *
 * Return: true or false
 */
qdf_freq_t wlan_reg_chan_to_freq(struct wlan_objmgr_pdev *pdev,
				 uint8_t chan);

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

#ifdef CONFIG_CHAN_NUM_API
/**
 * wlan_reg_chan_is_49ghz() - Check if the input channel number is 4.9GHz
 * @pdev: Pdev pointer
 * @chan_num: Input channel number
 *
 * Return: true if the channel is 4.9GHz else false.
 */
bool wlan_reg_chan_is_49ghz(struct wlan_objmgr_pdev *pdev,
		uint8_t chan_num);
#endif /* CONFIG_CHAN_NUM_API */

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
		uint32_t *chip_mode);

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

#ifdef CONFIG_CHAN_NUM_API
/**
 * wlan_reg_update_nol_history_ch() - Set nol-history flag for the channels in
 * the list.
 *
 * @pdev: Pdev ptr
 * @ch_list: Input channel list.
 * @num_ch: Number of channels.
 * @nol_history_ch: Nol history value.
 *
 * Return: void
 */
void wlan_reg_update_nol_history_ch(struct wlan_objmgr_pdev *pdev,
				    uint8_t *ch_list,
				    uint8_t num_ch,
				    bool nol_history_ch);
#endif /* CONFIG_CHAN_NUM_API */
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

/**
 * wlan_reg_chan_to_band() - Get band from channel number
 * @chan_num: channel number
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
 * wlan_reg_is_6ghz_op_class() - Whether 6ghz oper class
 * @pdev: pdev ptr
 * @op_class: operating class
 *
 * Return: bool
 */
bool wlan_reg_is_6ghz_op_class(struct wlan_objmgr_pdev *pdev,
			       uint8_t op_class);

/**
 * wlan_reg_is_6ghz_supported() - Whether 6ghz is supported
 * @pdev: pdev ptr
 *
 * Return: bool
 */
bool wlan_reg_is_6ghz_supported(struct wlan_objmgr_pdev *pdev);

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
#endif
