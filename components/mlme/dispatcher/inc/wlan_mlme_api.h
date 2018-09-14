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
 * DOC: declare public APIs exposed by the mlme component
 */

#ifndef _WLAN_MLME_API_H_
#define _WLAN_MLME_API_H_

#include <wlan_mlme_public_struct.h>
#include <wlan_objmgr_psoc_obj.h>
#include <wlan_cmn.h>
#include "sme_api.h"

/**
 * wlan_mlme_get_cfg_str() - Copy the uint8_t array for a particular CFG
 * @dst:       pointer to the destination buffer.
 * @cfg_str:   pointer to the cfg string structure
 * @len:       length to be copied
 *
 * Return: QDF_STATUS_SUCCESS or QDF_STATUS_E_FAILURE
 */
QDF_STATUS wlan_mlme_get_cfg_str(uint8_t *dst, struct mlme_cfg_str *cfg_str,
				 qdf_size_t *len);

/**
 * wlan_mlme_set_cfg_str() - Set values for a particular CFG
 * @src:            pointer to the source buffer.
 * @dst_cfg_str:    pointer to the cfg string structure to be modified
 * @len:            length to be written
 *
 * Return: QDF_STATUS_SUCCESS or QDF_STATUS_E_FAILURE
 */
QDF_STATUS wlan_mlme_set_cfg_str(uint8_t *src, struct mlme_cfg_str *dst_cfg_str,
				 qdf_size_t len);

/**
 * wlan_mlme_get_edca_params() - get the EDCA parameters corresponding to the
 * edca profile access category
 * @edca_params:   pointer to mlme edca parameters structure
 * @data:          data to which the parameter is to be copied
 * @edca_ac:       edca ac type enum passed to get the cfg value
 *
 * Return QDF_STATUS_SUCCESS or QDF_STATUS_E_FAILURE
 *
 */
QDF_STATUS wlan_mlme_get_edca_params(struct wlan_mlme_edca_params *edca_params,
				     uint8_t *data, enum e_edca_type edca_ac);

/*
 * mlme_get_wep_key() - get the wep key to process during auth frame
 * @wep_params: cfg wep parameters structure
 * @wep_key_id: default key number
 * @default_key: default key to be copied
 * @key_len: length of the key to copy
 *
 * Return QDF_STATUS
 */
QDF_STATUS mlme_get_wep_key(struct wlan_mlme_wep_cfg *wep_params,
			    enum wep_key_id wep_keyid, uint8_t *default_key,
			    qdf_size_t key_len);

/**
 * mlme_set_wep_key() - set the wep keys during auth
 * @wep_params: cfg wep parametrs structure
 * @wep_key_id: default key number that needs to be copied
 * @key_to_set: destination buffer to be copied
 * @len:        size to be copied
 */
QDF_STATUS mlme_set_wep_key(struct wlan_mlme_wep_cfg *wep_params,
			    enum wep_key_id wep_keyid, uint8_t *key_to_set,
			    qdf_size_t len);
/**
 * wlan_mlme_get_ht_cap_info() - Get the HT cap info config
 * @psoc: pointer to psoc object
 * @value: pointer to the value which will be filled for the caller
 *
 * Return: QDF Status
 */
QDF_STATUS wlan_mlme_get_ht_cap_info(struct wlan_objmgr_psoc *psoc,
				     struct mlme_ht_capabilities_info
				     *ht_cap_info);

/**
 * wlan_mlme_get_manufacturer_name() - get manufacturer name
 * @psoc: pointer to psoc object
 * @pbuf: pointer of the buff which will be filled for the caller
 * @plen: pointer of max buffer length
 *        actual length will be returned at this address
 * This function gets manufacturer name
 *
 * Return: QDF_STATUS_SUCCESS - in case of success
 */
QDF_STATUS
wlan_mlme_get_manufacturer_name(struct wlan_objmgr_psoc *psoc,
				uint8_t *pbuf, uint32_t *plen);

/**
 * wlan_mlme_get_model_number() - get model number
 * @psoc: pointer to psoc object
 * @pbuf: pointer of the buff which will be filled for the caller
 * @plen: pointer of max buffer length
 *        actual length will be returned at this address
 * This function gets model number
 *
 * Return: QDF_STATUS_SUCCESS - in case of success
 */
QDF_STATUS
wlan_mlme_get_model_number(struct wlan_objmgr_psoc *psoc,
			   uint8_t *pbuf, uint32_t *plen);

/**
 * wlan_mlme_get_model_name() - get model name
 * @psoc: pointer to psoc object
 * @pbuf: pointer of the buff which will be filled for the caller
 * @plen: pointer of max buffer length
 *        actual length will be returned at this address
 * This function gets model name
 *
 * Return: QDF_STATUS_SUCCESS - in case of success
 */
QDF_STATUS
wlan_mlme_get_model_name(struct wlan_objmgr_psoc *psoc,
			 uint8_t *pbuf, uint32_t *plen);

/**
 * wlan_mlme_get_manufacture_product_name() - get manufacture product name
 * @psoc: pointer to psoc object
 * @pbuf: pointer of the buff which will be filled for the caller
 * @plen: pointer of max buffer length
 *        actual length will be returned at this address
 * This function gets manufacture product name
 *
 * Return: QDF_STATUS_SUCCESS - in case of success
 */
QDF_STATUS
wlan_mlme_get_manufacture_product_name(struct wlan_objmgr_psoc *psoc,
				       uint8_t *pbuf, uint32_t *plen);

/**
 * wlan_mlme_get_manufacture_product_version() - get manufacture product version
 * @psoc: pointer to psoc object
 * @pbuf: pointer of the buff which will be filled for the caller
 * @plen: pointer of max buffer length
 *        actual length will be returned at this address
 * This function gets manufacture product version
 *
 * Return: QDF_STATUS_SUCCESS - in case of success
 */
QDF_STATUS
wlan_mlme_get_manufacture_product_version(struct wlan_objmgr_psoc *psoc,
					  uint8_t *pbuf, uint32_t *plen);

/**
 * wlan_mlme_set_ht_cap_info() - Set the HT cap info config
 * @psoc: pointer to psoc object
 * @value: Value that needs to be set from the caller
 *
 * Return: QDF Status
 */
QDF_STATUS wlan_mlme_set_ht_cap_info(struct wlan_objmgr_psoc *psoc,
				     struct mlme_ht_capabilities_info
				     ht_cap_info);

/**
 * wlan_mlme_get_acs_with_more_param() - Get the acs_with_more_param flag
 * @psoc: pointer to psoc object
 * @value: Value that needs to be set from the caller
 *
 * Return: QDF Status
 */
QDF_STATUS wlan_mlme_get_acs_with_more_param(struct wlan_objmgr_psoc *psoc,
					     bool *value);

/**
 * wlan_mlme_get_auto_channel_weight() - Get the auto channel weight
 * @psoc: pointer to psoc object
 * @value: Value that needs to be set from the caller
 *
 * Return: QDF Status
 */
QDF_STATUS wlan_mlme_get_auto_channel_weight(struct wlan_objmgr_psoc *psoc,
					     uint32_t *value);

/**
 * wlan_mlme_get_vendor_acs_support() - Get the vendor based channel selece
 * @psoc: pointer to psoc object
 * @value: Value that needs to be set from the caller
 *
 * Return: QDF Status
 */

QDF_STATUS wlan_mlme_get_vendor_acs_support(struct wlan_objmgr_psoc *psoc,
					    bool *value);

/**
 * wlan_mlme_get_acs_support_for_dfs_ltecoex() - Get the flag for
 *						 acs support for dfs ltecoex
 * @psoc: pointer to psoc object
 * @value: Value that needs to be set from the caller
 *
 * Return: QDF Status
 */
QDF_STATUS
wlan_mlme_get_acs_support_for_dfs_ltecoex(struct wlan_objmgr_psoc *psoc,
					  bool *value);

/**
 * wlan_mlme_get_external_acs_policy() - Get the flag for external acs policy
 *
 * @psoc: pointer to psoc object
 * @value: Value that needs to be set from the caller
 *
 * Return: QDF Status
 */
QDF_STATUS
wlan_mlme_get_external_acs_policy(struct wlan_objmgr_psoc *psoc,
				  bool *value);

/**
 *
 * wlan_mlme_get_sap_inactivity_override() - Check if sap max inactivity
 * override flag is set.
 * @psoc: pointer to psoc object
 * @sme_config - Sme config struct
 *
 * Return: QDF Status
 */
void wlan_mlme_get_sap_inactivity_override(struct wlan_objmgr_psoc *psoc,
					   bool *value);

/**
 * wlan_mlme_get_ignore_peer_ht_mode() - Get the ignore peer ht opmode flag
 * @psoc: pointer to psoc object
 * @value: Value that needs to be set from the caller
 *
 * Return: QDF Status
 */
QDF_STATUS wlan_mlme_get_ignore_peer_ht_mode(struct wlan_objmgr_psoc *psoc,
					bool *value);
/**
 * wlan_mlme_get_tx_chainmask_cck() - Get the tx_chainmask_cfg value
 *
 * @psoc: pointer to psoc object
 * @value: Value that needs to be set from the caller
 *
 * Return: QDF_STATUS_FAILURE or QDF_STATUS_SUCCESS
 */
QDF_STATUS wlan_mlme_get_tx_chainmask_cck(struct wlan_objmgr_psoc *psoc,
					  bool *value);

/**
 * wlan_mlme_get_tx_chainmask_1ss() - Get the tx_chainmask_1ss value
 *
 * @psoc: pointer to psoc object
 * @value: Value that needs to be set from the caller
 *
 * Return: QDF_STATUS_FAILURE or QDF_STATUS_SUCCESS
 */
QDF_STATUS wlan_mlme_get_tx_chainmask_1ss(struct wlan_objmgr_psoc *psoc,
					  uint8_t *value);

/**
 * wlan_mlme_get_num_11b_tx_chains() -  Get the number of 11b only tx chains
 *
 * @psoc: pointer to psoc object
 * @value: Value that needs to be set from the caller
 *
 * Return: QDF_STATUS_FAILURE or QDF_STATUS_SUCCESS
 */
QDF_STATUS wlan_mlme_get_num_11b_tx_chains(struct wlan_objmgr_psoc *psoc,
					   uint16_t *value);

/**
 * wlan_mlme_get_num_11ag_tx_chains() - get the total number of 11a/g tx chains
 *
 * @psoc: pointer to psoc object
 * @value: Value that needs to be set from the caller
 *
 * Return: QDF_STATUS_FAILURE or QDF_STATUS_SUCCESS
 */
QDF_STATUS wlan_mlme_get_num_11ag_tx_chains(struct wlan_objmgr_psoc *psoc,
					    uint16_t *value);

/**
 * wlan_mlme_configure_chain_mask() - configure chainmask parameters
 *
 * @psoc: pointer to psoc object
 * @session_id: vdev_id
 *
 * Return: QDF_STATUS_FAILURE or QDF_STATUS_SUCCESS
 */
QDF_STATUS wlan_mlme_configure_chain_mask(struct wlan_objmgr_psoc *psoc,
					  uint8_t session_id);

/**
 * wlan_mlme_set_sap_listen_interval() - Set the sap listen interval
 * @psoc: pointer to psoc object
 * @value: Value that needs to be set from the caller
 *
 * Return: QDF Status
 */
QDF_STATUS wlan_mlme_set_sap_listen_interval(struct wlan_objmgr_psoc *psoc,
					     int value);

/**
 * wlan_mlme_set_assoc_sta_limit() - Set the assoc sta limit
 * @psoc: pointer to psoc object
 * @value: Value that needs to be set from the caller
 *
 * Return: QDF Status
 */
QDF_STATUS wlan_mlme_set_assoc_sta_limit(struct wlan_objmgr_psoc *psoc,
					 int value);

/**
 * wlan_mlme_set_rmc_action_period_freq() - Set the rmc action period frequency
 * @psoc: pointer to psoc object
 * @value: Value that needs to be set from the caller
 *
 * Return: QDF Status
 */
QDF_STATUS wlan_mlme_set_rmc_action_period_freq(struct wlan_objmgr_psoc *psoc,
						int value);

/**
 * wlan_mlme_set_sap_get_peer_info() - get the sap get peer info
 * @psoc: pointer to psoc object
 * @value: Value that needs to be set from the caller
 *
 * Return: QDF Status
 */
QDF_STATUS wlan_mlme_get_sap_get_peer_info(struct wlan_objmgr_psoc *psoc,
					   bool *value);

/**
 * wlan_mlme_get_sap_allow_all_channels() - get the value of sap allow all
 * channels
 * @psoc: pointer to psoc object
 * @value: Value that needs to be set from the caller
 *
 * Return: QDF Status
 */
QDF_STATUS wlan_mlme_get_sap_allow_all_channels(struct wlan_objmgr_psoc *psoc,
						bool *value);

/**
 * wlan_mlme_get_sap_allow_all_channels() - get the value sap max peers
 * @psoc: pointer to psoc object
 * @value: Value that needs to be set from the caller
 *
 * Return: QDF Status
 */
QDF_STATUS wlan_mlme_get_sap_max_peers(struct wlan_objmgr_psoc *psoc,
				       int *value);

/**
 * wlan_mlme_set_sap_max_peers() - set the value sap max peers
 * @psoc: pointer to psoc object
 * @value: Value that needs to be set from the caller
 *
 * Return: QDF Status
 */
QDF_STATUS wlan_mlme_set_sap_max_peers(struct wlan_objmgr_psoc *psoc,
				       int value);

/**
 * wlan_mlme_get_sap_max_offload_peers() - get the value sap max offload peers
 * @psoc: pointer to psoc object
 * @value: Value that needs to be set from the caller
 *
 * Return: QDF Status
 */
QDF_STATUS wlan_mlme_get_sap_max_offload_peers(struct wlan_objmgr_psoc *psoc,
					       int *value);

/**
 * wlan_mlme_get_sap_max_offload_reorder_buffs() - get the value sap max offload
 * reorder buffs.
 * @psoc: pointer to psoc object
 * @value: Value that needs to be set from the caller
 *
 * Return: QDF Status
 */
QDF_STATUS wlan_mlme_get_sap_max_offload_reorder_buffs(struct wlan_objmgr_psoc
						       *psoc, int *value);

/**
 * wlan_mlme_get_sap_chn_switch_bcn_count() - get the value sap max channel
 * switch beacon count
 * @psoc: pointer to psoc object
 * @value: Value that needs to be set from the caller
 *
 * Return: QDF Status
 */
QDF_STATUS wlan_mlme_get_sap_chn_switch_bcn_count(struct wlan_objmgr_psoc *psoc,
						  int *value);

/**
 * wlan_mlme_get_sap_chn_switch_mode() - get the sap channel
 * switch mode
 * @psoc: pointer to psoc object
 * @value: Value that needs to be set from the caller
 *
 * Return: QDF Status
 */
QDF_STATUS wlan_mlme_get_sap_chn_switch_mode(struct wlan_objmgr_psoc *psoc,
					     bool *value);

/**
 * wlan_mlme_get_sap_internal_restart() - get the sap internal
 * restart
 * @psoc: pointer to psoc object
 * @value: Value that needs to be set from the caller
 *
 * Return: QDF Status
 */
QDF_STATUS wlan_mlme_get_sap_internal_restart(struct wlan_objmgr_psoc *psoc,
					      bool *value);

/**
 * wlan_mlme_get_sap_reduced_beacon_interval() - get the sap reduced
 * beacon interval
 * @psoc: pointer to psoc object
 * @value: Value that needs to be set from the caller
 *
 * Return: QDF Status
 */
QDF_STATUS wlan_mlme_get_sap_reduced_beacon_interval(struct wlan_objmgr_psoc
						     *psoc, int *value);

/**
 * wlan_mlme_get_sap_chan_switch_rate_enabled() - get the sap rate hostapd
 * enabled beacon interval
 * @psoc: pointer to psoc object
 * @value: Value that needs to be set from the caller
 *
 * Return: QDF Status
 */
QDF_STATUS wlan_mlme_get_sap_chan_switch_rate_enabled(struct wlan_objmgr_psoc
						      *psoc, bool *value);

/**
 * wlan_mlme_get_oce_sta_enabled_info() - Get the OCE feature enable
 * info for STA
 * @psoc: pointer to psoc object
 * @value: pointer to the value which will be filled for the caller
 *
 * Return: QDF Status
 */
QDF_STATUS wlan_mlme_get_oce_sta_enabled_info(struct wlan_objmgr_psoc *psoc,
					      bool *value);

/**
 * wlan_mlme_get_oce_sap_enabled_info() - Get the OCE feature enable
 * info for SAP
 * @psoc: pointer to psoc object
 * @value: pointer to the value which will be filled for the caller
 *
 * Return: QDF Status
 */
QDF_STATUS wlan_mlme_get_oce_sap_enabled_info(struct wlan_objmgr_psoc *psoc,
					      bool *value);

/**
 * wlan_mlme_get_rts_threshold() - Get the RTS threshold config
 * @psoc: pointer to psoc object
 * @value: pointer to the value which will be filled for the caller
 *
 * Return: QDF Status
 */
QDF_STATUS wlan_mlme_get_rts_threshold(struct wlan_objmgr_psoc *psoc,
				       uint32_t *value);

/**
 * wlan_mlme_set_rts_threshold() - Set the RTS threshold config
 * @psoc: pointer to psoc object
 * @value: Value that needs to be set from the caller
 *
 * Return: QDF Status
 */
QDF_STATUS wlan_mlme_set_rts_threshold(struct wlan_objmgr_psoc *psoc,
				       uint32_t value);

/**
 * wlan_mlme_get_frag_threshold() - Get the Fragmentation threshold
 *                                  config
 * @psoc: pointer to psoc object
 * @value: pointer to the value which will be filled for the caller
 *
 * Return: QDF Status
 */
QDF_STATUS wlan_mlme_get_frag_threshold(struct wlan_objmgr_psoc *psoc,
					uint32_t *value);

/**
 * wlan_mlme_set_frag_threshold() - Set the Fragmentation threshold
 *                                  config
 * @psoc: pointer to psoc object
 * @value: Value that needs to be set from the caller
 *
 * Return: QDF Status
 */
QDF_STATUS wlan_mlme_set_frag_threshold(struct wlan_objmgr_psoc *psoc,
					uint32_t value);

/**
 * wlan_mlme_get_fils_enabled_info() - Get the fils enable info for driver
 * @psoc: pointer to psoc object
 * @value: pointer to the value which will be filled for the caller
 *
 * Return: QDF Status
 */
QDF_STATUS wlan_mlme_get_fils_enabled_info(struct wlan_objmgr_psoc *psoc,
					   bool *value);
/**
 * wlan_mlme_set_fils_enabled_info() - Set the fils enable info for driver
 * @psoc: pointer to psoc object
 * @value: value that needs to be set from the caller
 *
 * Return: QDF Status
 */
QDF_STATUS wlan_mlme_set_fils_enabled_info(struct wlan_objmgr_psoc *psoc,
					   bool value);

/**
 * wlan_mlme_set_enable_bcast_probe_rsp() - Set enable bcast probe resp info
 * @psoc: pointer to psoc object
 * @value: value that needs to be set from the caller
 *
 * Return: QDF Status
 */
QDF_STATUS wlan_mlme_set_enable_bcast_probe_rsp(struct wlan_objmgr_psoc *psoc,
						bool value);
#endif /* _WLAN_MLME_API_H_ */
