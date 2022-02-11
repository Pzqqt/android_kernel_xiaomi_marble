/*
 * Copyright (c) 2017-2021 The Linux Foundation. All rights reserved.
 * Copyright (c) 2022 Qualcomm Innovation Center, Inc. All rights reserved.
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
 * DOC: reg_priv_objs.h
 * This file contains regulatory component private data structures.
 */

#ifndef __REG_PRIV_OBJS_H
#define __REG_PRIV_OBJS_H

#include <wlan_scan_public_structs.h>
#ifdef CONFIG_AFC_SUPPORT
#include "reg_services_common.h"
#endif
#include <wlan_objmgr_psoc_obj.h>

#define reg_alert(params...) \
	QDF_TRACE_FATAL(QDF_MODULE_ID_REGULATORY, params)
#define reg_err(params...) \
	QDF_TRACE_ERROR(QDF_MODULE_ID_REGULATORY, params)
#define reg_err_rl(params...) \
	QDF_TRACE_ERROR_RL(QDF_MODULE_ID_REGULATORY, params)
#define reg_warn(params...) \
	QDF_TRACE_WARN(QDF_MODULE_ID_REGULATORY, params)
#define reg_notice(params...) \
	QDF_TRACE_INFO(QDF_MODULE_ID_REGULATORY, params)
#define reg_info(params...) \
	QDF_TRACE_INFO(QDF_MODULE_ID_REGULATORY, params)
#define reg_debug(params...) \
	QDF_TRACE_DEBUG(QDF_MODULE_ID_REGULATORY, params)
#define reg_debug_rl(params...) \
	QDF_TRACE_DEBUG_RL(QDF_MODULE_ID_REGULATORY, params)

#define reg_nofl_alert(params...) \
	QDF_TRACE_FATAL_NO_FL(QDF_MODULE_ID_REGULATORY, params)
#define reg_nofl_err(params...) \
	QDF_TRACE_ERROR_NO_FL(QDF_MODULE_ID_REGULATORY, params)
#define reg_nofl_warn(params...) \
	QDF_TRACE_WARN_NO_FL(QDF_MODULE_ID_REGULATORY, params)
#define reg_nofl_info(params...) \
	QDF_TRACE_INFO_NO_FL(QDF_MODULE_ID_REGULATORY, params)
#define reg_nofl_debug(params...) \
	QDF_TRACE_DEBUG_NO_FL(QDF_MODULE_ID_REGULATORY, params)

/**
 * typedef reg_chan_change_callback() - Regulatory channel change callback
 * @psoc: Pointer to psoc
 * @pdev: Pointer to pdev
 * @chan_list: Pointer to regulatory channel list
 * @avoid_freq_ind: Pointer to avoid frequencies
 * @arg: list of arguments
 */
typedef void (*reg_chan_change_callback)(
		struct wlan_objmgr_psoc *psoc,
		struct wlan_objmgr_pdev *pdev,
		struct regulatory_channel *chan_list,
		struct avoid_freq_ind_data *avoid_freq_ind,
		void *arg);

/**
 * struct chan_change_cbk_entry - Channel change callback entry
 * @cbk: Callback
 * @arg: Arguments
 */
struct chan_change_cbk_entry {
	reg_chan_change_callback cbk;
	void *arg;
};

/**
 * struct wlan_regulatory_psoc_priv_obj - wlan regulatory psoc private object
 * @mas_chan_params: master channel parameters list
 * @chan_list_recvd: whether channel list has been received
 * @new_user_ctry_pending: In this array, element[phy_id] is true if any user
 *	country update is pending for pdev (phy_id), used in case of MCL.
 * @new_init_ctry_pending: In this array, element[phy_id] is true if any user
 *	country update is pending for pdev (phy_id), used in case of WIN.
 * @new_11d_ctry_pending: In this array, element[phy_id] is true if any 11d
 *	country update is pending for pdev (phy_id).
 * @world_country_pending: In this array, element[phy_id] is true if any world
 *	country update is pending for pdev (phy_id).
 * @band_capability: bitmap of bands enabled, using enum reg_wifi_band as the
 *	bit position value
 * @ignore_fw_reg_offload_ind: Ignore FW reg offload indication
 * @six_ghz_supported: whether 6ghz is supported
 * @five_dot_nine_ghz_supported: whether 5.9ghz is supported
 *	(service bit WMI_SERVICE_5_DOT_9GHZ_SUPPORT)
 * @enable_5dot9_ghz_chan_in_master_mode: 5.9 GHz channel support in
 *	master mode (ini fcc_5dot9_ghz_chan_in_master_mode)
 * @retain_nol_across_regdmn_update: Retain the NOL list across the regdomain
 *	changes.
 * @domain_code_6g_ap: domain code for 6G AP
 * @domain_code_6g_client: domain code for 6G client
 * @is_ext_tpc_supported: Whether FW supports new WMI command for TPC
 * @is_lower_6g_edge_ch_supported: whether lower 6ghz edge channel 5935MHz is
 * supported
 * @is_upper_6g_edge_ch_disabled: whether upper 6ghz edge channel 7115MHz is
 * disabled
 * @ch_avoid_ext_ind: whether need to update extended channel frequency list
 * @avoid_freq_ext_list: the extended avoid channel frequency list
 * @coex_unsafe_chan_nb_user_prefer: Honor coex unsafe chan cmd from firmware or
 * userspace
 * @coex_unsafe_chan_reg_disable: To disable reg channels for received coex
 * unsafe channels list
 */
struct wlan_regulatory_psoc_priv_obj {
	struct mas_chan_params mas_chan_params[PSOC_MAX_PHY_REG_CAP];
	bool chan_list_recvd[PSOC_MAX_PHY_REG_CAP];
	bool offload_enabled;
	bool six_ghz_supported;
	bool five_dot_nine_ghz_supported;
	uint8_t num_phy;
	char cur_country[REG_ALPHA2_LEN + 1];
	char def_country[REG_ALPHA2_LEN + 1];
	uint16_t def_country_code;
	uint16_t def_region_domain;
	enum country_src cc_src;
	struct wlan_objmgr_psoc *psoc_ptr;
	bool new_user_ctry_pending[PSOC_MAX_PHY_REG_CAP];
	bool new_init_ctry_pending[PSOC_MAX_PHY_REG_CAP];
	bool new_11d_ctry_pending[PSOC_MAX_PHY_REG_CAP];
	bool world_country_pending[PSOC_MAX_PHY_REG_CAP];
	bool dfs_enabled;
	uint32_t band_capability;
	bool indoor_chan_enabled;
	bool ignore_fw_reg_offload_ind;
	bool enable_11d_supp_original;
	bool enable_11d_supp;
	bool is_11d_offloaded;
	uint8_t vdev_id_for_11d_scan;
	uint8_t master_vdev_cnt;
	uint8_t vdev_cnt_11d;
	uint32_t scan_11d_interval;
#ifdef HOST_11D_SCAN
	bool is_host_11d_inited;
	wlan_scan_requester scan_req_id;
	uint32_t scan_id;
	qdf_mc_timer_t timer;
#endif
	uint8_t vdev_ids_11d[MAX_STA_VDEV_CNT];
	bool user_ctry_priority;
	bool user_ctry_set;
	struct chan_change_cbk_entry cbk_list[REG_MAX_CHAN_CHANGE_CBKS];
	uint8_t num_chan_change_cbks;
	uint8_t ch_avoid_ind;
	struct unsafe_ch_list unsafe_chan_list;
	struct ch_avoid_ind_type avoid_freq_list;
	enum restart_beaconing_on_ch_avoid_rule restart_beaconing;
	struct wlan_psoc_host_hal_reg_capabilities_ext
			reg_cap[PSOC_MAX_PHY_REG_CAP];
	bool force_ssc_disable_indoor_channel;
	uint8_t enable_srd_chan_in_master_mode;
	bool enable_11d_in_world_mode;
	bool enable_5dot9_ghz_chan_in_master_mode;
	qdf_spinlock_t cbk_list_lock;
	bool retain_nol_across_regdmn_update;
#ifdef CONFIG_BAND_6GHZ
	uint8_t domain_code_6g_ap[REG_CURRENT_MAX_AP_TYPE];
	uint8_t domain_code_6g_client[REG_CURRENT_MAX_AP_TYPE][REG_MAX_CLIENT_TYPE];
#endif
	bool is_ext_tpc_supported;
#if defined(CONFIG_BAND_6GHZ)
	bool is_lower_6g_edge_ch_supported;
	bool is_upper_6g_edge_ch_disabled;
#endif
#ifdef FEATURE_WLAN_CH_AVOID_EXT
	bool ch_avoid_ext_ind;
	struct ch_avoid_ind_type avoid_freq_ext_list;
	bool coex_unsafe_chan_nb_user_prefer;
	bool coex_unsafe_chan_reg_disable;
#endif
};

/**
 * struct wlan_regulatory_pdev_priv_obj - wlan regulatory pdev private object
 * @cur_chan_list: current channel list, includes 6G channels
 * @secondary_cur_chan_list: secondary current channel list, for concurrency
 * situations
 * @mas_chan_list: master channel list
 * from the firmware.
 * @is_6g_channel_list_populated: indicates the channel lists are populated
 * @mas_chan_list_6g_ap: master channel list for 6G AP, includes all power types
 * @mas_chan_list_6g_client: master channel list for 6G client, includes
 *	all power types
 * @band_capability: bitmap of bands enabled, using enum reg_wifi_band as the
 *	bit position value
 * @reg_6g_superid: 6Ghz super domain id
 * @pdev_opened: whether pdev has been opened by application
 * @reg_cur_6g_ap_pwr_type: 6G AP type ie VLP/SP/LPI.
 * @reg_cur_6g_client_mobility_type: 6G client type ie Default/Subordinate.
 * @reg_rnr_tpe_usable: Indicates whether RNR IE is applicable for current reg
 * domain.
 * @reg_unspecified_ap_usable: Indicates if the AP type mentioned is not part of
 * 802.11 standard.
 * @max_phymode: The maximum phymode supported by the device and regulatory.
 * @max_chwidth: The maximum bandwidth corresponding to the maximum phymode.
 * @avoid_chan_ext_list: the extended avoid frequency list.
 * @afc_cb_lock: The spinlock to synchronize afc callbacks
 * @afc_cb_obj: The object containing the callback function and opaque argument
 * @afc_request_id: The last AFC request id received from FW/halphy
 * @is_6g_afc_power_event_received: indicates if the AFC power event is
 * received
 * @is_6g_afc_expiry_event_received: indicates if the AFC exipiry event is
 * received
 * @afc_chan_list: Intersection of AFC master and Standard power channel list
 * @mas_chan_list_6g_afc: AFC master channel list constructed from the AFC
 * server response.
 * @power_info: pointer to AFC power information received from the AFC event
 * sent by the target
 * @is_reg_noaction_on_afc_pwr_evt: indicates whether regulatory needs to
 * take action when AFC Power event is received
 */
struct wlan_regulatory_pdev_priv_obj {
	struct regulatory_channel cur_chan_list[NUM_CHANNELS];
#ifdef CONFIG_REG_CLIENT
	struct regulatory_channel secondary_cur_chan_list[NUM_CHANNELS];
#endif
	struct regulatory_channel mas_chan_list[NUM_CHANNELS];
#ifdef CONFIG_BAND_6GHZ
	bool is_6g_channel_list_populated;
	struct regulatory_channel mas_chan_list_6g_ap[REG_CURRENT_MAX_AP_TYPE][NUM_6GHZ_CHANNELS];
	struct regulatory_channel mas_chan_list_6g_client[REG_CURRENT_MAX_AP_TYPE][REG_MAX_CLIENT_TYPE][NUM_6GHZ_CHANNELS];
#endif
#ifdef DISABLE_CHANNEL_LIST
	struct regulatory_channel cache_disable_chan_list[NUM_CHANNELS];
	uint32_t num_cache_channels;
	bool disable_cached_channels;
#endif
	char default_country[REG_ALPHA2_LEN + 1];
	uint16_t def_region_domain;
	uint16_t def_country_code;
	char current_country[REG_ALPHA2_LEN + 1];
	uint16_t reg_dmn_pair;
	uint16_t reg_6g_superid;
	uint16_t ctry_code;
#ifdef DISABLE_UNII_SHARED_BANDS
	uint8_t unii_5g_bitmap;
#endif
	enum dfs_reg dfs_region;
	uint32_t phybitmap;
	struct wlan_objmgr_pdev *pdev_ptr;
	qdf_freq_t range_2g_low;
	qdf_freq_t range_2g_high;
	qdf_freq_t range_5g_low;
	qdf_freq_t range_5g_high;
	bool dfs_enabled;
	bool set_fcc_channel;
	uint32_t band_capability;
	bool indoor_chan_enabled;
	bool en_chan_144;
	uint64_t wireless_modes;
	struct ch_avoid_ind_type freq_avoid_list;
	bool force_ssc_disable_indoor_channel;
	bool sap_state;
	struct reg_rule_info reg_rules;
	qdf_spinlock_t reg_rules_lock;
	bool chan_list_recvd;
	bool pdev_opened;
#if defined(CONFIG_BAND_6GHZ)
	enum reg_6g_ap_type reg_cur_6g_ap_pwr_type;
	enum reg_6g_client_type reg_cur_6g_client_mobility_type;
	bool reg_rnr_tpe_usable;
	bool reg_unspecified_ap_usable;
#endif
#ifdef CONFIG_HOST_FIND_CHAN
	enum reg_phymode max_phymode;
	enum phy_ch_width max_chwidth;
#endif
#ifdef FEATURE_WLAN_CH_AVOID_EXT
	avoid_ch_ext_list avoid_chan_ext_list;
#endif
#ifdef CONFIG_AFC_SUPPORT
	qdf_spinlock_t afc_cb_lock;
	struct afc_cb_handler afc_cb_obj;
	uint64_t afc_request_id;
	bool is_6g_afc_power_event_received;
	bool is_6g_afc_expiry_event_received;
	struct regulatory_channel afc_chan_list[NUM_6GHZ_CHANNELS];
	struct regulatory_channel mas_chan_list_6g_afc[NUM_6GHZ_CHANNELS];
	struct reg_fw_afc_power_event *power_info;
	bool is_reg_noaction_on_afc_pwr_evt;
#endif
};

/**
 * reg_get_psoc_obj() - Provides the reg component object pointer
 * @psoc: pointer to psoc object.
 *
 * Return: reg component object pointer
 */
struct wlan_regulatory_psoc_priv_obj *reg_get_psoc_obj(
		struct wlan_objmgr_psoc *psoc);

/**
 * reg_get_pdev_obj() - Provides the reg component object pointer
 * @psoc: pointer to psoc object.
 *
 * Return: reg component object pointer
 */
struct wlan_regulatory_pdev_priv_obj *reg_get_pdev_obj(
		struct wlan_objmgr_pdev *pdev);

/**
 * wlan_regulatory_psoc_obj_created_notification() - PSOC obj create callback
 * @psoc: PSOC object
 * @arg_list: Variable argument list
 *
 * This callback is registered with object manager during initialization to
 * get notified when the object is created.
 *
 * Return: Success or Failure
 */
QDF_STATUS wlan_regulatory_psoc_obj_created_notification(
		struct wlan_objmgr_psoc *psoc, void *arg_list);

/**
 * wlan_regulatory_psoc_obj_destroyed_notification() - PSOC obj delete callback
 * @psoc: PSOC object
 * @arg_list: Variable argument list
 *
 * This callback is registered with object manager during initialization to
 * get notified when the object is deleted.
 *
 * Return: Success or Failure
 */
QDF_STATUS wlan_regulatory_psoc_obj_destroyed_notification(
	struct wlan_objmgr_psoc *psoc, void *arg_list);

/**
 * wlan_regulatory_pdev_obj_created_notification() - PDEV obj create callback
 * @pdev: pdev object
 * @arg_list: Variable argument list
 *
 * This callback is registered with object manager during initialization to
 * get notified when the pdev object is created.
 *
 * Return: Success or Failure
 */
QDF_STATUS wlan_regulatory_pdev_obj_created_notification(
	struct wlan_objmgr_pdev *pdev, void *arg_list);

/**
 * wlan_regulatory_pdev_obj_destroyed_notification() - PDEV obj destroy callback
 * @pdev: pdev object
 * @arg_list: Variable argument list
 *
 * This callback is registered with object manager during initialization to
 * get notified when the pdev object is destroyed.
 *
 * Return: Success or Failure
 */
QDF_STATUS wlan_regulatory_pdev_obj_destroyed_notification(
		struct wlan_objmgr_pdev *pdev, void *arg_list);

#ifdef CONFIG_AFC_SUPPORT
/**
 * reg_free_afc_pwr_info() - Free the AFC power information object
 * @pdev_priv_obj: Pointer to pdev_priv_obj
 *
 * Return: void
 */
void
reg_free_afc_pwr_info(struct wlan_regulatory_pdev_priv_obj *pdev_priv_obj);
#else
static inline void
reg_free_afc_pwr_info(struct wlan_regulatory_pdev_priv_obj *pdev_priv_obj)
{
}
#endif
#endif
