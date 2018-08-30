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

#endif /* _WLAN_MLME_API_H_ */
