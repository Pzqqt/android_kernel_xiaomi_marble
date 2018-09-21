/*
 * Copyright (c) 2018 The Linux Foundation. All rights reserved.
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
 * DOC: declare UCFG APIs exposed by the mlme component
 */

#ifndef _WLAN_MLME_UCFG_API_H_
#define _WLAN_MLME_UCFG_API_H_

#include <wlan_mlme_public_struct.h>
#include <wlan_objmgr_psoc_obj.h>
#include <wlan_objmgr_global_obj.h>
#include <wlan_cmn.h>
#include <wlan_mlme_api.h>

/**
 * ucfg_mlme_init() - initialize mlme_ctx context.
 *
 * This function initializes the mlme context.
 *
 * Return: QDF_STATUS_SUCCESS - in case of success else return error
 */
QDF_STATUS ucfg_mlme_init(void);

/**
 * ucfg_mlme_deinit() - De initialize mlme_ctx context.
 *
 * This function De initializes mlme contex.
 *
 * Return: QDF_STATUS_SUCCESS - in case of success else return error
 */
QDF_STATUS ucfg_mlme_deinit(void);

/**
 * ucfg_mlme_psoc_open() - MLME component Open
 * @psoc: pointer to psoc object
 *
 * Open the MLME component and initialize the MLME strucutre
 *
 * Return: QDF Status
 */
QDF_STATUS ucfg_mlme_psoc_open(struct wlan_objmgr_psoc *psoc);

/**
 * ucfg_mlme_psoc_close() - MLME component close
 * @psoc: pointer to psoc object
 *
 * Close the MLME component and clear the MLME structures
 *
 * Return: None
 */
void ucfg_mlme_psoc_close(struct wlan_objmgr_psoc *psoc);

#ifdef CONFIG_VDEV_SM
/**
 * ucfg_mlme_pdev_open() - MLME component pdev Open
 * @pdev: pointer to pdev object
 *
 * Open the MLME component and initialize the MLME pdev strucutre
 *
 * Return: QDF Status
 */
QDF_STATUS ucfg_mlme_pdev_open(struct wlan_objmgr_pdev *pdev);

/**
 * ucfg_mlme_pdev_close() - MLME component pdev close
 * @pdev: pointer to pdev object
 *
 * close the MLME pdev information
 *
 * Return: QDF Status
 */
QDF_STATUS ucfg_mlme_pdev_close(struct wlan_objmgr_pdev *pdev);

#else
/**
 * ucfg_mlme_pdev_open() - MLME component pdev Open
 * @pdev: pointer to pdev object
 *
 * Open the MLME component and initialize the MLME pdev strucutre
 *
 * Return: QDF Status
 */
static inline QDF_STATUS ucfg_mlme_pdev_open(struct wlan_objmgr_pdev *pdev)
{
	return QDF_STATUS_SUCCESS;
}

/**
 * ucfg_mlme_pdev_close() - MLME component pdev close
 * @pdev: pointer to pdev object
 *
 * close the MLME pdev information
 *
 * Return: QDF Status
 */
static inline QDF_STATUS ucfg_mlme_pdev_close(struct wlan_objmgr_pdev *pdev)
{
	return QDF_STATUS_SUCCESS;
}
#endif

/**
 * ucfg_mlme_get_ht_cap_info() - Get the HT cap info config
 * @psoc: pointer to psoc object
 * @value: pointer to the value which will be filled for the caller
 *
 * Inline UCFG API to be used by HDD/OSIF callers
 *
 * Return: QDF Status
 */
static inline
QDF_STATUS ucfg_mlme_get_ht_cap_info(struct wlan_objmgr_psoc *psoc,
				     struct mlme_ht_capabilities_info
				     *ht_cap_info)
{
	return wlan_mlme_get_ht_cap_info(psoc, ht_cap_info);
}

/**
 * ucfg_mlme_set_ht_cap_info() - Set the HT cap info config
 * @psoc: pointer to psoc object
 * @value: Value that needs to be set from the caller
 *
 * Inline UCFG API to be used by HDD/OSIF callers
 *
 * Return: QDF Status
 */
static inline
QDF_STATUS ucfg_mlme_set_ht_cap_info(struct wlan_objmgr_psoc *psoc,
				     struct mlme_ht_capabilities_info
				     ht_cap_info)
{
	return wlan_mlme_set_ht_cap_info(psoc, ht_cap_info);
}

/**
 * ucfg_mlme_get_acs_with_more_param() - Get the flag for acs with
 *					 more param
 * @psoc: pointer to psoc object
 * @value: Value that needs to be set from the caller
 *
 * Inline UCFG API to be used by HDD/OSIF callers
 *
 * Return: QDF Status
 */
static inline
QDF_STATUS ucfg_mlme_get_acs_with_more_param(struct wlan_objmgr_psoc *psoc,
					     bool *value)
{
	return wlan_mlme_get_acs_with_more_param(psoc, value);
}

/**
 * ucfg_mlme_get_auto_channel_weight() - Get the auto channel select weight
 * @psoc: pointer to psoc object
 * @value: Value that needs to be set from the caller
 *
 * Inline UCFG API to be used by HDD/OSIF callers
 *
 * Return: QDF Status
 */
static inline
QDF_STATUS ucfg_mlme_get_auto_channel_weight(struct wlan_objmgr_psoc *psoc,
					     uint32_t *value)
{
	return wlan_mlme_get_auto_channel_weight(psoc, value);
}

/**
 * ucfg_mlme_get_vendor_acs_support() - Get the flag for
 *					vendor acs support
 * @psoc: pointer to psoc object
 * @value: Value that needs to be set from the caller
 *
 * Inline UCFG API to be used by HDD/OSIF callers
 *
 * Return: QDF Status
 */
static inline
QDF_STATUS ucfg_mlme_get_vendor_acs_support(struct wlan_objmgr_psoc *psoc,
					    bool *value)
{
	return wlan_mlme_get_vendor_acs_support(psoc, value);
}

/**
 * ucfg_mlme_get_external_acs_policy() - Get flag for external control
 *					 acs policy
 * @psoc: pointer to psoc object
 * @value: Value that needs to be set from the caller
 *
 * Inline UCFG API to be used by HDD/OSIF callers
 *
 * Return: QDF Status
 */
static inline QDF_STATUS
ucfg_mlme_get_external_acs_policy(struct wlan_objmgr_psoc *psoc,
				  bool *value)
{
	return wlan_mlme_get_external_acs_policy(psoc, value);
}

/**
 * ucfg_mlme_set_ht_cap_info() - Set the HT cap info config
 * @psoc: pointer to psoc object
 * @value: Value that needs to be set from the caller
 *
 * Inline UCFG API to be used by HDD/OSIF callers
 *
 * Return: QDF Status
 */
static inline
QDF_STATUS
ucfg_mlme_get_acs_support_for_dfs_ltecoex(struct wlan_objmgr_psoc *psoc,
					  bool *value)
{
	return wlan_mlme_get_acs_support_for_dfs_ltecoex(psoc, value);
}

/**
 * ucfg_mlme_get_wmm_dir_ac_vo() - Get TSPEC direction for VO
 * @psoc: pointer to psoc object
 * @value: Value that needs to be set from the caller
 *
 * Inline UCFG API to be used by HDD/OSIF callers
 *
 * Return: QDF Status
 */
static inline QDF_STATUS
ucfg_mlme_get_wmm_dir_ac_vo(struct wlan_objmgr_psoc *psoc,
			    uint8_t *value)
{
	return wlan_mlme_get_wmm_dir_ac_vo(psoc, value);
}

/**
 * ucfg_mlme_get_wmm_nom_msdu_size_ac_vo() - Get normal
 * MSDU size for VO
 * @psoc: pointer to psoc object
 * @value: Value that needs to be set from the caller
 *
 * Inline UCFG API to be used by HDD/OSIF callers
 *
 * Return: QDF Status
 */
static inline QDF_STATUS
ucfg_mlme_get_wmm_nom_msdu_size_ac_vo(struct wlan_objmgr_psoc *psoc,
				      uint16_t *value)
{
	return wlan_mlme_get_wmm_nom_msdu_size_ac_vo(psoc, value);
}

/**
 * ucfg_mlme_get_wmm_mean_data_rate_ac_vo() - mean data rate for VO
 * @psoc: pointer to psoc object
 * @value: Value that needs to be set from the caller
 *
 * Inline UCFG API to be used by HDD/OSIF callers
 *
 * Return: QDF Status
 */
static inline QDF_STATUS
ucfg_mlme_get_wmm_mean_data_rate_ac_vo(struct wlan_objmgr_psoc *psoc,
				       uint32_t *value)
{
	return wlan_mlme_get_wmm_mean_data_rate_ac_vo(psoc, value);
}

/**
 * ucfg_mlme_get_wmm_min_phy_rate_ac_vo() - min PHY
 * rate for VO
 * @psoc: pointer to psoc object
 * @value: Value that needs to be set from the caller
 *
 * Inline UCFG API to be used by HDD/OSIF callers
 *
 * Return: QDF Status
 */
static inline QDF_STATUS
ucfg_mlme_get_wmm_min_phy_rate_ac_vo(struct wlan_objmgr_psoc *psoc,
				     uint32_t *value)
{
	return wlan_mlme_get_wmm_min_phy_rate_ac_vo(psoc, value);
}

/**
 * ucfg_mlme_get_wmm_sba_ac_vo() - surplus bandwidth
 * allowance for VO
 * @psoc: pointer to psoc object
 * @value: Value that needs to be set from the caller
 *
 * Inline UCFG API to be used by HDD/OSIF callers
 *
 * Return: QDF Status
 */
static inline QDF_STATUS
ucfg_mlme_get_wmm_sba_ac_vo(struct wlan_objmgr_psoc *psoc,
			    uint16_t *value)
{
	return wlan_mlme_get_wmm_sba_ac_vo(psoc, value);
}

/**
 * ucfg_mlme_get_wmm_uapsd_vo_srv_intv() - Get Uapsd service
 * interval for voice
 * @psoc: pointer to psoc object
 * @value: pointer to the value which will be filled for the caller
 *
 * Inline UCFG API to be used by HDD/OSIF callers
 *
 * Return: QDF Status
 */
static inline QDF_STATUS
ucfg_mlme_get_wmm_uapsd_vo_srv_intv(struct wlan_objmgr_psoc *psoc,
				    uint32_t *value)
{
	return wlan_mlme_get_wmm_uapsd_vo_srv_intv(psoc, value);
}

/**
 * ucfg_mlme_get_wmm_uapsd_vo_sus_intv() - Get Uapsd suspension
 * interval for voice
 * @psoc: pointer to psoc object
 * @value: Value that needs to be set from the caller
 *
 * Inline UCFG API to be used by HDD/OSIF callers
 *
 * Return: QDF Status
 */
static inline QDF_STATUS
ucfg_mlme_get_wmm_uapsd_vo_sus_intv(struct wlan_objmgr_psoc *psoc,
				    uint32_t *value)
{
	return wlan_mlme_get_wmm_uapsd_vo_sus_intv(psoc, value);
}

/**
 *
 * ucfg_mlme_get_sap_inactivity_override() - Check if sap max inactivity
 * override flag is set.
 * @psoc: pointer to psoc object
 * @sme_config - Sme config struct
 *
 * Inline UCFG API to be used by HDD/OSIF callers to call
 * the mlme function wlan_mlme_get_sap_inactivity_override
 *
 * Return: QDF Status
 */
static inline
void ucfg_mlme_get_sap_inactivity_override(struct wlan_objmgr_psoc *psoc,
					   bool *value)
{
	wlan_mlme_get_sap_inactivity_override(psoc, value);
}

/**
 * ucfg_mlme_get_tx_chainmask_1ss() - Get the tx_chainmask_1ss value
 *
 * @psoc: pointer to psoc object
 * @value: Value that needs to be set from the caller
 *
 * Return: QDF_STATUS_FAILURE or QDF_STATUS_SUCCESS
 */
static inline
QDF_STATUS ucfg_mlme_get_tx_chainmask_1ss(struct wlan_objmgr_psoc *psoc,
					  uint8_t *value)
{
	return wlan_mlme_get_tx_chainmask_1ss(psoc, value);
}

/**
 * ucfg_mlme_get_num_11b_tx_chains() -  Get the number of 11b only tx chains
 *
 * @psoc: pointer to psoc object
 * @value: Value that needs to be set from the caller
 *
 * Return: QDF_STATUS_FAILURE or QDF_STATUS_SUCCESS
 */
static inline
QDF_STATUS ucfg_mlme_get_num_11b_tx_chains(struct wlan_objmgr_psoc *psoc,
					   uint16_t *value)
{
	return wlan_mlme_get_num_11b_tx_chains(psoc, value);
}

/**
 * ucfg_mlme_get_num_11ag_tx_chains() - get the total number of 11a/g tx chains
 *
 * @psoc: pointer to psoc object
 * @value: Value that needs to be set from the caller
 *
 * Return: QDF_STATUS_FAILURE or QDF_STATUS_SUCCESS
 */
static inline
QDF_STATUS ucfg_mlme_get_num_11ag_tx_chains(struct wlan_objmgr_psoc *psoc,
					    uint16_t *value)
{
	return wlan_mlme_get_num_11ag_tx_chains(psoc, value);
}

/**
 * ucfg_mlme_configure_chain_mask() - configure chainmask parameters
 *
 * @psoc: pointer to psoc object
 * @session_id: vdev_id
 *
 * Return: QDF_STATUS_FAILURE or QDF_STATUS_SUCCESS
 */
static inline
QDF_STATUS ucfg_mlme_configure_chain_mask(struct wlan_objmgr_psoc *psoc,
					  uint8_t session_id)
{
	return wlan_mlme_configure_chain_mask(psoc, session_id);
}

/*
 * ucfg_mlme_get_sta_keep_alive_period() - Get the sta keep alive period
 * @psoc: pointer to psoc object
 * @val:  Pointer to the value which will be filled for the caller
 *
 * Return: QDF Status
 */
QDF_STATUS
ucfg_mlme_get_sta_keep_alive_period(struct wlan_objmgr_psoc *psoc,
				    uint32_t *val);

/**
 * ucfg_mlme_get_pmkid_modes() - Get PMKID modes
 * @psoc: pointer to psoc object
 * @val:  Pointer to the value which will be filled for the caller
 *
 * Return: QDF Status
 */
QDF_STATUS
ucfg_mlme_get_pmkid_modes(struct wlan_objmgr_psoc *psoc,
			  uint32_t *val);

/**
 * ucfg_mlme_set_pmkid_modes() - Set PMKID modes
 * @psoc: pointer to psoc object
 * @val:  Pointer to the value which will be filled for the caller
 *
 * Return: QDF Status
 */
QDF_STATUS
ucfg_mlme_set_pmkid_modes(struct wlan_objmgr_psoc *psoc,
			  uint32_t val);

/**
 * ucfg_mlme_get_dot11p_mode() - Get the setting about 802.11p mode
 * @psoc: pointer to psoc object
 * @val:  Pointer to the value which will be filled for the caller
 *
 * Return: QDF Status
 */
QDF_STATUS
ucfg_mlme_get_dot11p_mode(struct wlan_objmgr_psoc *psoc,
			  uint8_t *val);

/**
 * ucfg_mlme_get_go_cts2self_for_sta() - Stop NOA and start using cts2self
 * @psoc: pointer to psoc object
 * @val:  Pointer to the value which will be filled for the caller
 *
 * Return: QDF Status
 */
QDF_STATUS
ucfg_mlme_get_go_cts2self_for_sta(struct wlan_objmgr_psoc *psoc,
				  bool *val);

/**
 * ucfg_mlme_get_force_rsne_override() - Force rsnie override from user
 * @psoc: pointer to psoc object
 * @val:  Pointer to the value which will be filled for the caller
 *
 * Return: QDF Status
 */
QDF_STATUS
ucfg_mlme_get_force_rsne_override(struct wlan_objmgr_psoc *psoc,
				  bool *val);

/**
 * ucfg_mlme_get_qcn_ie_support() - QCN IE support or not
 * @psoc: pointer to psoc object
 * @val:  Pointer to the value which will be filled for the caller
 *
 * Return: QDF Status
 */
QDF_STATUS
ucfg_mlme_get_qcn_ie_support(struct wlan_objmgr_psoc *psoc,
			     bool *val);

/**
 * ucfg_mlme_get_tgt_gtx_usr_cfg() - Get the target gtx user config
 * @psoc: pointer to psoc object
 * @val:  Pointer to the value which will be filled for the caller
 *
 * Return: QDF Status
 */
QDF_STATUS
ucfg_mlme_get_tgt_gtx_usr_cfg(struct wlan_objmgr_psoc *psoc,
			      uint32_t *val);

/**
 * ucfg_mlme_get_roaming_offload() - Get roaming offload setting
 * @psoc: pointer to psoc object
 * @val:  Pointer to enable/disable roaming offload
 *
 * Return: QDF Status
 */
QDF_STATUS
ucfg_mlme_get_roaming_offload(struct wlan_objmgr_psoc *psoc,
			      bool *val);

/**
 * ucfg_mlme_set_roaming_offload() - Enable/disable roaming offload
 * @psoc: pointer to psoc object
 * @val:  enable/disable roaming offload
 *
 * Return: QDF Status
 */
QDF_STATUS
ucfg_mlme_set_roaming_offload(struct wlan_objmgr_psoc *psoc,
			      bool val);

/**
 * ucfg_mlme_get_first_scan_bucket_threshold() - Get first scan bucket thre
 * @psoc: pointer to psoc object
 * @val:  first scan bucket threshold
 *
 * Return: QDF Status
 */
QDF_STATUS
ucfg_mlme_get_first_scan_bucket_threshold(struct wlan_objmgr_psoc *psoc,
					  uint8_t *val);
/**
 * ucfg_mlme_set_sap_listen_interval() - Set the Sap listen interval
 * @psoc: pointer to psoc object
 * @value: Value that needs to be set from the caller
 *
 * Inline UCFG API to be used by HDD/OSIF callers
 *
 * Return: QDF Status
 */
static inline
QDF_STATUS ucfg_mlme_set_sap_listen_interval(struct wlan_objmgr_psoc *psoc,
					     int value)
{
	return wlan_mlme_set_sap_listen_interval(psoc, value);
}

/**
 * ucfg_mlme_set_assoc_sta_limit() - Set the assoc sta limit
 * @psoc: pointer to psoc object
 * @value: Value that needs to be set from the caller
 *
 * Inline UCFG API to be used by HDD/OSIF callers
 *
 * Return: QDF Status
 */
static inline
QDF_STATUS ucfg_mlme_set_assoc_sta_limit(struct wlan_objmgr_psoc *psoc,
					 int value)
{
	return wlan_mlme_set_assoc_sta_limit(psoc, value);
}

/**
 * ucfg_mlme_set_rmc_action_period_freq() - Set the rmc action period frequency
 * @psoc: pointer to psoc object
 * @value: Value that needs to be set from the caller
 *
 * Inline UCFG API to be used by HDD/OSIF callers
 *
 * Return: QDF Status
 */
static inline
QDF_STATUS ucfg_mlme_set_rmc_action_period_freq(struct wlan_objmgr_psoc *psoc,
						int value)
{
	return wlan_mlme_set_rmc_action_period_freq(psoc, value);
}

/**
 * ucfg_mlme_get_sap_get_peer_info() - get the sap get peer info
 * @psoc: pointer to psoc object
 * @value: Value that needs to be set from the caller
 *
 * Inline UCFG API to be used by HDD/OSIF callers
 *
 * Return: QDF Status
 */
static inline
QDF_STATUS ucfg_mlme_get_sap_get_peer_info(struct wlan_objmgr_psoc *psoc,
					   bool *value)
{
	return wlan_mlme_get_sap_get_peer_info(psoc, value);
}

/**
 * ucfg_mlme_get_sap_allow_all_channels() - get the sap allow all channels
 * @psoc: pointer to psoc object
 * @value: Value that needs to be set from the caller
 *
 * Inline UCFG API to be used by HDD/OSIF callers
 *
 * Return: QDF Status
 */
static inline
QDF_STATUS ucfg_mlme_get_sap_allow_all_channels(struct wlan_objmgr_psoc *psoc,
						bool *value)
{
	return wlan_mlme_get_sap_allow_all_channels(psoc, value);
}

/**
 * ucfg_mlme_get_sap_max_peers() - get the sap max peers
 * @psoc: pointer to psoc object
 * @value: Value that needs to be set from the caller
 *
 * Inline UCFG API to be used by HDD/OSIF callers
 *
 * Return: QDF Status
 */
static inline
QDF_STATUS ucfg_mlme_get_sap_max_peers(struct wlan_objmgr_psoc *psoc,
				       int *value)
{
	return wlan_mlme_get_sap_max_peers(psoc, value);
}

/**
 * ucfg_mlme_set_sap_max_peers() - Set the sap max peers
 * @psoc: pointer to psoc object
 * @value: Value that needs to be set from the caller
 *
 * Inline UCFG API to be used by HDD/OSIF callers
 *
 * Return: QDF Status
 */
static inline
QDF_STATUS ucfg_mlme_set_sap_max_peers(struct wlan_objmgr_psoc *psoc, int value)
{
	return wlan_mlme_set_sap_max_peers(psoc, value);
}

/**
 * ucfg_mlme_get_sap_max_offload_peers() - get the sap max offload peers
 * @psoc: pointer to psoc object
 * @value: Value that needs to be set from the caller
 *
 * Inline UCFG API to be used by HDD/OSIF callers
 *
 * Return: QDF Status
 */
static inline
QDF_STATUS ucfg_mlme_get_sap_max_offload_peers(struct wlan_objmgr_psoc *psoc,
					       int *value)
{
	return wlan_mlme_get_sap_max_offload_peers(psoc, value);
}

/**
 * ucfg_mlme_get_sap_max_offload_reorder_buffs() - get the sap max offload
 * reorder buffs
 * @psoc: pointer to psoc object
 * @value: Value that needs to be set from the caller
 *
 * Inline UCFG API to be used by HDD/OSIF callers
 *
 * Return: QDF Status
 */
static inline
QDF_STATUS ucfg_mlme_get_sap_max_offload_reorder_buffs(struct wlan_objmgr_psoc
						       *psoc, int *value)
{
	return wlan_mlme_get_sap_max_offload_reorder_buffs(psoc, value);
}

/**
 * ucfg_mlme_get_sap_chn_switch_bcn_count() - get the sap channel
 * switch beacon count
 * @psoc: pointer to psoc object
 * @value: Value that needs to be set from the caller
 *
 * Inline UCFG API to be used by HDD/OSIF callers
 *
 * Return: QDF Status
 */
static inline
QDF_STATUS ucfg_mlme_get_sap_chn_switch_bcn_count(struct wlan_objmgr_psoc *psoc,
						  int *value)
{
	return wlan_mlme_get_sap_chn_switch_bcn_count(psoc, value);
}

/**
 * ucfg_mlme_get_sap_channel_switch_mode() - get the sap channel switch mode
 * @psoc: pointer to psoc object
 * @value: Value that needs to be set from the caller
 *
 * Inline UCFG API to be used by HDD/OSIF callers
 *
 * Return: QDF Status
 */
static inline
QDF_STATUS ucfg_mlme_get_sap_channel_switch_mode(struct wlan_objmgr_psoc *psoc,
						 bool *value)
{
	return wlan_mlme_get_sap_chn_switch_mode(psoc, value);
}

/**
 * ucfg_mlme_get_sap_internal_restart() - get sap internal restart value
 * @psoc: pointer to psoc object
 * @value: Value that needs to be set from the caller
 *
 * Inline UCFG API to be used by HDD/OSIF callers
 *
 * Return: QDF Status
 */
static inline
QDF_STATUS ucfg_mlme_get_sap_internal_restart(struct wlan_objmgr_psoc *psoc,
					      bool *value)
{
	return wlan_mlme_get_sap_internal_restart(psoc, value);
}

/**
 * ucfg_mlme_get_sap_reduces_beacon_interval() - get the sap reduces beacon
 * interval
 * @psoc: pointer to psoc object
 * @value: Value that needs to be set from the caller
 *
 * Inline UCFG API to be used by HDD/OSIF callers
 *
 * Return: QDF Status
 */
static inline
QDF_STATUS ucfg_mlme_get_sap_reduces_beacon_interval(struct wlan_objmgr_psoc
						     *psoc, int *value)
{
	return wlan_mlme_get_sap_reduced_beacon_interval(psoc, value);
}

/**
 * ucfg_mlme_get_sap_chan_switch_rate_enabled() - get the sap channel
 * switch rate enabled.
 * @psoc: pointer to psoc object
 * @value: Value that needs to be set from the caller
 *
 * Inline UCFG API to be used by HDD/OSIF callers
 *
 * Return: QDF Status
 */
static inline
QDF_STATUS ucfg_mlme_get_sap_chan_switch_rate_enabled(struct wlan_objmgr_psoc
						      *psoc, bool *value)
{
	return wlan_mlme_get_sap_chan_switch_rate_enabled(psoc, value);
}

/**
 * ucfg_mlme_get_oce_sta_enabled_info() - Get OCE feature enable/disable
 * info for STA
 *
 * @psoc: pointer to psoc object
 * @value: pointer to the value which will be filled for the caller
 *
 * Inline UCFG API to be used by HDD/OSIF callers to get the
 * OCE STA feature enable value
 *
 * Return: QDF_STATUS_SUCCESS or QDF_STATUS_FAILURE
 */
static inline
QDF_STATUS ucfg_mlme_get_oce_sta_enabled_info(struct wlan_objmgr_psoc *psoc,
					      bool *value)
{
	return wlan_mlme_get_oce_sta_enabled_info(psoc, value);
}

/**
 * ucfg_mlme_get_oce_sap_enabled_info() - Get OCE feature enable/disable
 * info for SAP
 *
 * @psoc: pointer to psoc object
 * @value: pointer to the value which will be filled for the caller
 *
 * Inline UCFG API to be used by HDD/OSIF callers to get the
 * OCE SAP feature enable value
 *
 * Return: QDF_STATUS_SUCCESS or QDF_STATUS_FAILURE
 */
static inline
QDF_STATUS ucfg_mlme_get_oce_sap_enabled_info(struct wlan_objmgr_psoc *psoc,
					      bool *value)
{
	return wlan_mlme_get_oce_sap_enabled_info(psoc, value);
}

/**
 * ucfg_mlme_get_rts_threshold() - Get the rts threshold config
 * @psoc: pointer to psoc object
 * @value: pointer to the value which will be filled for the caller
 *
 * Inline UCFG API to be used by HDD/OSIF callers
 *
 * Return: QDF Status
 */
static inline
QDF_STATUS ucfg_mlme_get_rts_threshold(struct wlan_objmgr_psoc *psoc,
				       uint32_t *value)
{
	return wlan_mlme_get_rts_threshold(psoc, value);
}

/**
 * ucfg_mlme_set_rts_threshold() - Set the rts threshold config
 * @psoc: pointer to psoc object
 * @value: pointer to the value which will be filled for the caller
 *
 * Inline UCFG API to be used by HDD/OSIF callers
 *
 * Return: QDF Status
 */
static inline
QDF_STATUS ucfg_mlme_set_rts_threshold(struct wlan_objmgr_psoc *psoc,
				       uint32_t value)
{
	return wlan_mlme_set_rts_threshold(psoc, value);
}

/**
 * ucfg_mlme_get_frag_threshold() - Get the fragmentation threshold
 *                                  config
 * @psoc: pointer to psoc object
 * @value: Value that needs to be set from the caller
 *
 * Inline UCFG API to be used by HDD/OSIF callers
 *
 * Return: QDF Status
 */
static inline
QDF_STATUS ucfg_mlme_get_frag_threshold(struct wlan_objmgr_psoc *psoc,
					uint32_t *value)
{
	return wlan_mlme_get_frag_threshold(psoc, value);
}

/**
 * ucfg_mlme_set_frag_threshold() - set the frag threshold config
 * @psoc: pointer to psoc object
 * @value: pointer to the value which will be filled for the caller
 *
 * Inline UCFG API to be used by HDD/OSIF callers
 *
 * Return: QDF Status
 */
static inline
QDF_STATUS ucfg_mlme_set_frag_threshold(struct wlan_objmgr_psoc *psoc,
					uint32_t value)
{
	return wlan_mlme_set_frag_threshold(psoc, value);
}

/**
 * ucfg_mlme_get_fils_enabled_info() - Get fils enable/disable info
 *
 * @psoc: pointer to psoc object
 * @value: pointer to the value which will be filled for the caller
 *
 * Inline UCFG API to be used by HDD/OSIF callers to get the
 * fils enable value
 *
 * Return: QDF_STATUS_SUCCESS or QDF_STATUS_FAILURE
 */
static inline
QDF_STATUS ucfg_mlme_get_fils_enabled_info(struct wlan_objmgr_psoc *psoc,
					   bool *value)
{
	return wlan_mlme_get_fils_enabled_info(psoc, value);
}

/**
 * ucfg_mlme_set_fils_enabled_info() - Set fils enable info
 *
 * @psoc: pointer to psoc object
 * @value: value that needs to be set from the caller
 *
 * Inline UCFG API to be used by HDD/OSIF callers to set the
 * fils enable value
 *
 * Return: QDF_STATUS_SUCCESS or QDF_STATUS_FAILURE
 */
static inline
QDF_STATUS ucfg_mlme_set_fils_enabled_info(struct wlan_objmgr_psoc *psoc,
					   bool value)
{
	return wlan_mlme_set_fils_enabled_info(psoc, value);
}

/**
 * ucfg_mlme_set_enable_bcast_probe_rsp() - Set enable bcast probe resp info
 *
 * @psoc: pointer to psoc object
 * @value: value that needs to be set from the caller
 *
 * Inline UCFG API to be used by HDD/OSIF callers to set the
 * enable bcast probe resp info
 *
 * Return: QDF_STATUS_SUCCESS or QDF_STATUS_FAILURE
 */
static inline
QDF_STATUS ucfg_mlme_set_enable_bcast_probe_rsp(struct wlan_objmgr_psoc *psoc,
						bool value)
{
	return wlan_mlme_set_enable_bcast_probe_rsp(psoc, value);
}
#endif /* _WLAN_MLME_UCFG_API_H_ */
