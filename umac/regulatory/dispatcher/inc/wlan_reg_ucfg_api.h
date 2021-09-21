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
 * DOC: wlan_reg_ucfg_api.h
 * This file provides prototypes of the regulatory component user
 * config interface routines
 */

#ifndef __WLAN_REG_UCFG_API_H
#define __WLAN_REG_UCFG_API_H

#ifdef CONFIG_AFC_SUPPORT
#include <wlan_reg_afc.h>
#endif
#include <reg_services_public_struct.h>

typedef QDF_STATUS (*reg_event_cb)(void *status_struct);

/**
 * ucfg_reg_set_band() - Sets the band information for the PDEV
 * @pdev: The physical pdev to set the band for
 * @band_bitmap: The band bitmap parameter (over reg_wifi_band) to configure
 *	for the physical device
 *
 * Return: QDF_STATUS
 */
QDF_STATUS ucfg_reg_set_band(struct wlan_objmgr_pdev *pdev,
			     uint32_t band_bitmap);

/**
 * ucfg_reg_get_band() - Gets the band information for the PDEV
 * @pdev: The physical pdev to get the band for
 * @band_bitmap: The band parameter of the physical device
 *
 * Return: QDF_STATUS
 */
QDF_STATUS ucfg_reg_get_band(struct wlan_objmgr_pdev *pdev,
			     uint32_t *band_bitmap);

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

#ifdef DISABLE_CHANNEL_LIST
/**
 * ucfg_reg_disable_cached_channels() - Disable cached channels
 * @pdev: The physical dev to cache the channels for
 *
 * Return: Void
 */
void ucfg_reg_disable_cached_channels(struct wlan_objmgr_pdev *pdev);

/**
 * ucfg_reg_restore_cached_channels() - Restore disabled cached channels
 * @pdev: The physical dev to cache the channels for
 *
 * Return: Void
 */
void ucfg_reg_restore_cached_channels(struct wlan_objmgr_pdev *pdev);
#else
static inline
void ucfg_reg_disable_cached_channels(struct wlan_objmgr_pdev *pdev)
{
}

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

#ifdef CONFIG_AFC_SUPPORT
/**
 * ucfg_reg_register_afc_req_rx_callback () - add AFC request received callback
 * @pdev: Pointer to pdev
 * @cbf: Pointer to callback function
 * @arg: Pointer to opaque argument
 *
 * Return: QDF_STATUS
 */
QDF_STATUS ucfg_reg_register_afc_req_rx_callback(struct wlan_objmgr_pdev *pdev,
						 afc_req_rx_evt_handler cbf,
						 void *arg);

/**
 * ucfg_reg_unregister_afc_req_rx_callback () - remove AFC request received
 * callback
 * @pdev: Pointer to pdev
 * @cbf: Pointer to callback function
 *
 * Return: QDF_STATUS
 */
QDF_STATUS ucfg_reg_unregister_afc_req_rx_callback(struct wlan_objmgr_pdev *pdev,
						   afc_req_rx_evt_handler cbf);

/**
 * ucfg_reg_get_partial_afc_req_info() - Get the the frequency ranges and
 * opclass + channel ranges. This is partial because in the AFC request there
 * are a few more parameters: Longitude, Latitude a few other information
 * @pdev: Pointer to PDEV object.
 * @afc_req: Address of AFC request pointer.
 * @req_id: AFC request ID.
 *
 * Return: QDF_STATUS_E_INVAL if unable to set and QDF_STATUS_SUCCESS is set.
 */
QDF_STATUS ucfg_reg_get_partial_afc_req_info(
		struct wlan_objmgr_pdev *pdev,
		struct wlan_afc_host_partial_request **afc_req,
		uint64_t req_id);
#endif

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
 * ucfg_reg_ch_avoid () - Send channel avoid cmd to regulatory
 * @psoc: psoc ptr
 * @ch_avoid: ch_avoid_ind_type ranges
 *
 * This function send channel avoid cmd to regulatory from os_if/upper layer
 *
 * Return: void
 */
void ucfg_reg_ch_avoid(struct wlan_objmgr_psoc *psoc,
		       struct ch_avoid_ind_type *ch_avoid);

#ifdef FEATURE_WLAN_CH_AVOID_EXT
/**
 * ucfg_reg_ch_avoid_ext () - Send channel avoid extend cmd to regulatory
 * @psoc: psoc ptr
 * @ch_avoid: ch_avoid_ind_type ranges
 *
 * This function send channel avoid extend cmd to regulatory from
 * os_if/upper layer
 *
 * Return: void
 */
void ucfg_reg_ch_avoid_ext(struct wlan_objmgr_psoc *psoc,
			   struct ch_avoid_ind_type *ch_avoid);
#endif

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
 * ucfg_reg_update_hal_reg_cap() - update hal reg cap
 * @psoc: psoc ptr
 * @wireless_modes: 11AX wireless modes
 * @phy_id: phy id
 *
 * Return: QDF_STATUS
 */
QDF_STATUS ucfg_reg_update_hal_reg_cap(struct wlan_objmgr_psoc *psoc,
				       uint64_t wireless_modes, uint8_t phy_id);

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

#if defined(CONFIG_BAND_6GHZ)
/**
 * ucfg_reg_get_cur_6g_ap_pwr_type() - Get the current 6G regulatory AP power
 * type.
 * @pdev: Pointer to PDEV object.
 * @reg_6g_ap_pwr_type: The current regulatory 6G AP type ie VLPI/LPI/SP.
 *
 * Return: QDF_STATUS.
 */
QDF_STATUS
ucfg_reg_get_cur_6g_ap_pwr_type(struct wlan_objmgr_pdev *pdev,
				enum reg_6g_ap_type *reg_cur_6g_ap_pwr_type);

/**
 * ucfg_reg_set_cur_6g_ap_pwr_type() - Set the current 6G regulatory AP power
 * type.
 * @pdev: Pointer to PDEV object.
 * @reg_6g_ap_pwr_type: Regulatory 6G AP type ie VLPI/LPI/SP.
 *
 * Return: QDF_STATUS_E_INVAL if unable to set and QDF_STATUS_SUCCESS is set.
 */
QDF_STATUS
ucfg_reg_set_cur_6g_ap_pwr_type(struct wlan_objmgr_pdev *pdev,
				enum reg_6g_ap_type reg_cur_6g_ap_type);
#else
static inline QDF_STATUS
ucfg_reg_get_cur_6g_ap_pwr_type(struct wlan_objmgr_pdev *pdev,
				enum reg_6g_ap_type *reg_cur_6g_ap_pwr_type)
{
	*reg_cur_6g_ap_pwr_type = REG_INDOOR_AP;
	return QDF_STATUS_E_NOSUPPORT;
}

static inline QDF_STATUS
ucfg_reg_set_cur_6g_ap_pwr_type(struct wlan_objmgr_pdev *pdev,
				enum reg_6g_ap_type reg_cur_6g_ap_type)
{
	return QDF_STATUS_E_NOSUPPORT;
}
#endif

#if defined(CONFIG_AFC_SUPPORT) && defined(CONFIG_BAND_6GHZ)
/**
 * ucfg_reg_send_afc_resp_rx_ind() - Send AFC response received indication to
 * the FW.
 * @pdev: pdev ptr
 * @afc_ind_obj: Pointer to hold AFC indication
 *
 * Return: QDF_STATUS_SUCCESS if the WMI command is sent or QDF_STATUS_E_FAILURE
 * otherwise
 */
QDF_STATUS
ucfg_reg_send_afc_resp_rx_ind(struct wlan_objmgr_pdev *pdev,
			      struct reg_afc_resp_rx_ind_info *afc_ind_obj);

/**
 * ucfg_reg_afc_start() - Start the AFC request from regulatory. This finally
 *                   sends the request to registered callbacks
 * @pdev: Pointer to pdev
 * @req_id: The AFC request ID
 *
 * Return: QDF_STATUS
 */
QDF_STATUS ucfg_reg_afc_start(struct wlan_objmgr_pdev *pdev, uint64_t req_id);
#endif
#endif
