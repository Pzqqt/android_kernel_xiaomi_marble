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
 * DOC: wlan_reg_ucfg_api.h
 * This file provides prototypes of the regulatory component user
 * config interface routines
 */

#ifndef __WLAN_REG_UCFG_API_H
#define __WLAN_REG_UCFG_API_H

typedef QDF_STATUS (*reg_event_cb)(void *status_struct);

/**
 * ucfg_reg_set_band() - Sets the band information for the PDEV
 * @pdev: The physical pdev to set the band for
 * @band: The set band parameter to configure for the physical device
 *
 * Return: QDF_STATUS
 */
QDF_STATUS ucfg_reg_set_band(struct wlan_objmgr_pdev *pdev,
			     enum band_info band);

/**
 * ucfg_reg_get_band() - Gets the band information for the PDEV
 * @pdev: The physical pdev to get the band for
 * @band: The band parameter of the physical device
 *
 * Return: QDF_STATUS
 */
QDF_STATUS ucfg_reg_get_band(struct wlan_objmgr_pdev *pdev,
			     enum band_info *band);

/**
 * ucfg_reg_notify_sap_event() - Notify regulatory domain for sap event
 * @pdev: The physical dev to set the band for
 * @sap_state: true for sap start else false
 *
 * Return: QDF_STATUS
 */
QDF_STATUS ucfg_reg_notify_sap_event(struct wlan_objmgr_pdev *pdev,
			bool sap_state);

/**
 * ucfg_reg_cache_channel_state() - Cache the current state of the channles
 * @pdev: The physical dev to cache the channels for
 * @channel_list: List of the channels for which states needs to be cached
 * @num_channels: Number of channels in the list
 *
 * Return: QDF_STATUS
 */
#if defined(DISABLE_CHANNEL_LIST) && defined(CONFIG_CHAN_NUM_API)
void ucfg_reg_cache_channel_state(struct wlan_objmgr_pdev *pdev,
				  uint32_t *channel_list,
				  uint32_t num_channels);
#else
static inline
void ucfg_reg_cache_channel_state(struct wlan_objmgr_pdev *pdev,
				  uint32_t *channel_list,
				  uint32_t num_channels)
{
}
#endif /* CONFIG_CHAN_NUM_API */

/**
 * ucfg_reg_cache_channel_freq_state() - Cache the current state of the
 * channels based on the channel center frequency.
 * @pdev: Pointer to pdev.
 * @channel_list: List of the channels for which states need to be cached.
 * @num_channels: Number of channels in the list.
 *
 * Return: QDF_STATUS
 */
#if defined(DISABLE_CHANNEL_LIST) && defined(CONFIG_CHAN_FREQ_API)
void ucfg_reg_cache_channel_freq_state(struct wlan_objmgr_pdev *pdev,
				       uint32_t *channel_list,
				       uint32_t num_channels);
#else
static inline
void ucfg_reg_cache_channel_freq_state(struct wlan_objmgr_pdev *pdev,
				       uint32_t *channel_list,
				       uint32_t num_channels)
{
}
#endif /* CONFIG_CHAN_FREQ_API */

/**
 * ucfg_reg_restore_cached_channels() - Cache the current state of the channles
 * @pdev: The physical dev to cache the channels for
 *
 * Return: QDF_STATUS
 */
#ifdef DISABLE_CHANNEL_LIST
void ucfg_reg_restore_cached_channels(struct wlan_objmgr_pdev *pdev);
#else
static inline
void ucfg_reg_restore_cached_channels(struct wlan_objmgr_pdev *pdev)
{
}
#endif

/**
 * ucfg_reg_set_fcc_constraint() - apply fcc constraints on channels 12/13
 * @pdev: The physical pdev to reduce tx power for
 *
 * This function adjusts the transmit power on channels 12 and 13, to comply
 * with FCC regulations in the USA.
 *
 * Return: QDF_STATUS
 */
QDF_STATUS ucfg_reg_set_fcc_constraint(struct wlan_objmgr_pdev *pdev,
		bool fcc_constraint);

/**
 * ucfg_reg_get_default_country() - Get the default regulatory country
 * @psoc: The physical SoC to get default country from
 * @country_code: the buffer to populate the country code into
 *
 * Return: QDF_STATUS
 */
QDF_STATUS ucfg_reg_get_default_country(struct wlan_objmgr_psoc *psoc,
					       uint8_t *country_code);

/**
 * ucfg_reg_get_current_country() - Get the current regulatory country
 * @psoc: The physical SoC to get current country from
 * @country_code: the buffer to populate the country code into
 *
 * Return: QDF_STATUS
 */
QDF_STATUS ucfg_reg_get_current_country(struct wlan_objmgr_psoc *psoc,
					       uint8_t *country_code);
/**
 * ucfg_reg_set_default_country() - Set the default regulatory country
 * @psoc: The physical SoC to set default country for
 * @country_code: The country information to configure
 *
 * Return: QDF_STATUS
 */
QDF_STATUS ucfg_reg_set_default_country(struct wlan_objmgr_psoc *psoc,
					       uint8_t *country_code);

/**
 * ucfg_reg_set_country() - Set the current regulatory country
 * @pdev: The physical dev to set current country for
 * @country_code: The country information to configure
 *
 * Return: QDF_STATUS
 */
QDF_STATUS ucfg_reg_set_country(struct wlan_objmgr_pdev *dev,
				uint8_t *country_code);

/**
 * ucfg_reg_reset_country() - Reset the regulatory country to default
 * @psoc: The physical SoC to reset country for
 *
 * Return: QDF_STATUS
 */
QDF_STATUS ucfg_reg_reset_country(struct wlan_objmgr_psoc *psoc);

/**
 * ucfg_reg_get_curr_band() - Get the current band capability
 * @pdev: The physical dev to get default country from
 * @band: buffer to populate the band into
 *
 * Return: QDF_STATUS
 */
QDF_STATUS ucfg_reg_get_curr_band(struct wlan_objmgr_pdev *pdev,
		enum band_info *band);
/**
 * ucfg_reg_enable_dfs_channels() - Enable the use of DFS channels
 * @pdev: The physical dev to enable DFS channels for
 *
 * Return: QDF_STATUS
 */
QDF_STATUS ucfg_reg_enable_dfs_channels(struct wlan_objmgr_pdev *pdev,
		bool dfs_enable);

QDF_STATUS ucfg_reg_register_event_handler(uint8_t vdev_id, reg_event_cb cb,
		void *arg);
QDF_STATUS ucfg_reg_unregister_event_handler(uint8_t vdev_id, reg_event_cb cb,
		void *arg);
QDF_STATUS ucfg_reg_init_handler(uint8_t pdev_id);

QDF_STATUS ucfg_reg_program_default_cc(struct wlan_objmgr_pdev *pdev,
				       uint16_t regdmn);

/**
 * ucfg_reg_program_cc() - Program user country code or regdomain
 * @pdev: The physical dev to program country code or regdomain
 * @rd: User country code or regdomain
 *
 * Return: QDF_STATUS
 */
QDF_STATUS ucfg_reg_program_cc(struct wlan_objmgr_pdev *pdev,
			       struct cc_regdmn_s *rd);

/**
 * ucfg_reg_get_current_cc() - get current country code or regdomain
 * @pdev: The physical dev to program country code or regdomain
 * @rd: Pointer to country code or regdomain
 *
 * Return: QDF_STATUS
 */
QDF_STATUS ucfg_reg_get_current_cc(struct wlan_objmgr_pdev *pdev,
				   struct cc_regdmn_s *rd);

/**
 * ucfg_reg_set_config_vars () - Set the config vars in reg component
 * @psoc: psoc ptr
 * @config_vars: config variables structure
 *
 * Return: QDF_STATUS
 */
QDF_STATUS ucfg_reg_set_config_vars(struct wlan_objmgr_psoc *psoc,
				    struct reg_config_vars config_vars);

/**
 * ucfg_reg_get_current_chan_list () - get current channel list
 * @pdev: pdev ptr
 * @chan_list: channel list
 *
 * Return: QDF_STATUS
 */
QDF_STATUS ucfg_reg_get_current_chan_list(struct wlan_objmgr_pdev *pdev,
				    struct regulatory_channel *chan_list);

/**
 * ucfg_reg_modify_chan_144() - Enable/Disable channel 144
 * @pdev: pdev pointer
 * @enable_chan_144: flag to disable/enable channel 144
 *
 * Return: Success or Failure
 */
QDF_STATUS ucfg_reg_modify_chan_144(struct wlan_objmgr_pdev *pdev,
				    bool enable_ch_144);

/**
 * ucfg_reg_get_en_chan_144() - get en_chan_144 flag value
 * @pdev: pdev pointer
 *
 * Return: en_chan_144 flag value
 */
bool ucfg_reg_get_en_chan_144(struct wlan_objmgr_pdev *pdev);

/**
 * ucfg_reg_is_regdb_offloaded () - is regulatory database offloaded
 * @psoc: psoc ptr
 *
 * Return: bool
 */
bool ucfg_reg_is_regdb_offloaded(struct wlan_objmgr_psoc *psoc);

/**
 * ucfg_reg_program_mas_chan_list () - program master channel list
 * @psoc: psoc ptr
 * @reg_channels: regulatory channels
 * @alpha2: country code
 * @dfs_region: dfs region
 *
 * Return: void
 */
void ucfg_reg_program_mas_chan_list(struct wlan_objmgr_psoc *psoc,
				    struct regulatory_channel *reg_channels,
				    uint8_t *alpha2,
				    enum dfs_reg dfs_region);

/**
 * ucfg_reg_get_regd_rules() - provides the reg domain rules info pointer
 * @pdev: pdev ptr
 * @reg_rules: regulatory rules
 *
 * Return: QDF_STATUS
 */
QDF_STATUS ucfg_reg_get_regd_rules(struct wlan_objmgr_pdev *pdev,
				   struct reg_rule_info *reg_rules);

/**
 * ucfg_reg_register_chan_change_callback () - add chan change cbk
 * @psoc: psoc ptr
 * @cbk: callback
 * @arg: argument
 *
 * Return: void
 */
void ucfg_reg_register_chan_change_callback(struct wlan_objmgr_psoc *psoc,
					    void *cbk, void *arg);

/**
 * ucfg_reg_unregister_chan_change_callback () - remove chan change cbk
 * @psoc: psoc ptr
 * @cbk: callback
 *
 * Return: void
 */
void ucfg_reg_unregister_chan_change_callback(struct wlan_objmgr_psoc *psoc,
					      void *cbk);

/**
 * ucfg_reg_get_cc_and_src () - get country code and src
 * @psoc: psoc ptr
 * @alpha2: country code alpha2
 *
 * Return: void
 */
enum country_src ucfg_reg_get_cc_and_src(struct wlan_objmgr_psoc *psoc,
					 uint8_t *alpha2);

/**
 * ucfg_reg_unit_simulate_ch_avoid () - fake a ch avoid event
 * @psoc: psoc ptr
 * @ch_avoid: ch_avoid_ind_type ranges
 *
 * This function inject a ch_avoid event for unit test sap chan switch.
 *
 * Return: void
 */
void ucfg_reg_unit_simulate_ch_avoid(struct wlan_objmgr_psoc *psoc,
	struct ch_avoid_ind_type *ch_avoid);

/**
 * ucfg_reg_11d_vdev_delete_update() - update vdev delete to regulatory
 * @vdev: vdev ptr
 *
 * Return: QDF_STATUS
 */
QDF_STATUS ucfg_reg_11d_vdev_delete_update(struct wlan_objmgr_vdev *vdev);

/**
 * ucfg_reg_11d_vdev_created_update() - update vdev create to regulatory
 * @vdev: vdev ptr
 *
 * Return: QDF_STATUS
 */
QDF_STATUS ucfg_reg_11d_vdev_created_update(struct wlan_objmgr_vdev *vdev);

/**
 * ucfg_reg_get_hal_reg_cap() - return hal reg cap
 * @psoc: psoc ptr
 *
 * Return: ptr to  wlan_psoc_host_hal_reg_capabilities_ext
 */
struct wlan_psoc_host_hal_reg_capabilities_ext *ucfg_reg_get_hal_reg_cap(
				struct wlan_objmgr_psoc *psoc);

/**
 * ucfg_reg_set_hal_reg_cap() - update hal reg cap
 * @psoc: psoc ptr
 * @reg_cap: Regulatory cap array
 * @phy_cnt: Number of phy
 *
 * Return: QDF_STATUS
 */
QDF_STATUS ucfg_reg_set_hal_reg_cap(struct wlan_objmgr_psoc *psoc,
			struct wlan_psoc_host_hal_reg_capabilities_ext *reg_cap,
			uint16_t phy_cnt);

/**
 * ucfg_set_ignore_fw_reg_offload_ind() - API to set ignore regdb offload ind
 * @psoc: psoc ptr
 *
 * Return: QDF_STATUS
 */
QDF_STATUS ucfg_set_ignore_fw_reg_offload_ind(struct wlan_objmgr_psoc *psoc);

/**
 * ucfg_reg_get_unii_5g_bitmap() - get unii_5g_bitmap value
 * @pdev: pdev pointer
 * @bitmap: Pointer to retrieve unii_5g_bitmap of enum reg_unii_band.
 *
 * Return: QDF_STATUS
 */
#ifdef DISABLE_UNII_SHARED_BANDS
QDF_STATUS
ucfg_reg_get_unii_5g_bitmap(struct wlan_objmgr_pdev *pdev, uint8_t *bitmap);
#else
static inline QDF_STATUS
ucfg_reg_get_unii_5g_bitmap(struct wlan_objmgr_pdev *pdev, uint8_t *bitmap)
{
	*bitmap = 0;
	return QDF_STATUS_SUCCESS;
}
#endif

#endif
