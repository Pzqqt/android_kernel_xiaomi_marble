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
 * DOC: define public APIs exposed by the mlme component
 */

#include "cfg_ucfg_api.h"
#include "wlan_mlme_main.h"
#include "wlan_mlme_ucfg_api.h"
#include "wma_types.h"
#include "wmi_unified.h"

QDF_STATUS wlan_mlme_get_ht_cap_info(struct wlan_objmgr_psoc *psoc,
				     struct mlme_ht_capabilities_info
				     *ht_cap_info)
{
	struct wlan_mlme_psoc_obj *mlme_obj;

	mlme_obj = mlme_get_psoc_obj(psoc);
	if (!mlme_obj) {
		mlme_err("Failed to get MLME Obj");
		return QDF_STATUS_E_FAILURE;
	}

	*ht_cap_info = mlme_obj->cfg.ht_caps.ht_cap_info;

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS wlan_mlme_set_ht_cap_info(struct wlan_objmgr_psoc *psoc,
				     struct mlme_ht_capabilities_info
				     ht_cap_info)
{
	struct wlan_mlme_psoc_obj *mlme_obj;

	mlme_obj = mlme_get_psoc_obj(psoc);
	if (!mlme_obj) {
		mlme_err("Failed to get MLME Obj");
		return QDF_STATUS_E_FAILURE;
	}

	mlme_obj->cfg.ht_caps.ht_cap_info = ht_cap_info;

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS wlan_mlme_get_ignore_peer_ht_mode(struct wlan_objmgr_psoc *psoc,
					     bool *value)
{
	struct wlan_mlme_psoc_obj *mlme_obj = mlme_get_psoc_obj(psoc);

	if (!mlme_obj) {
		mlme_err("Failed to get MLME Obj");
		return QDF_STATUS_E_FAILURE;
	}

	*value = mlme_obj->cfg.sap_protection_cfg.ignore_peer_ht_mode;
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS wlan_mlme_get_tx_chainmask_cck(struct wlan_objmgr_psoc *psoc,
					  bool *value)
{
	struct wlan_mlme_psoc_obj *mlme_obj = mlme_get_psoc_obj(psoc);

	if (!mlme_obj) {
		mlme_err("Failed to get MLME Obj");
		return QDF_STATUS_E_FAILURE;
	}

	*value = mlme_obj->cfg.chainmask_cfg.tx_chain_mask_cck;
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS wlan_mlme_get_tx_chainmask_1ss(struct wlan_objmgr_psoc *psoc,
					  uint8_t *value)
{
	struct wlan_mlme_psoc_obj *mlme_obj = mlme_get_psoc_obj(psoc);

	if (!mlme_obj) {
		mlme_err("Failed to get MLME Obj");
		return QDF_STATUS_E_FAILURE;
	}

	*value = mlme_obj->cfg.chainmask_cfg.tx_chain_mask_1ss;
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS wlan_mlme_get_num_11b_tx_chains(struct wlan_objmgr_psoc *psoc,
					   uint16_t *value)
{
	struct wlan_mlme_psoc_obj *mlme_obj = mlme_get_psoc_obj(psoc);

	if (!mlme_obj) {
		mlme_err("Failed to get MLME Obj");
		return QDF_STATUS_E_FAILURE;
	}

	*value = mlme_obj->cfg.chainmask_cfg.num_11b_tx_chains;
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS wlan_mlme_get_num_11ag_tx_chains(struct wlan_objmgr_psoc *psoc,
					    uint16_t *value)
{
	struct wlan_mlme_psoc_obj *mlme_obj = mlme_get_psoc_obj(psoc);

	if (!mlme_obj) {
		mlme_err("Failed to get MLME Obj");
		return QDF_STATUS_E_FAILURE;
	}

	*value = mlme_obj->cfg.chainmask_cfg.num_11ag_tx_chains;
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS wlan_mlme_configure_chain_mask(struct wlan_objmgr_psoc *psoc,
					  uint8_t session_id)
{
	int ret_val;
	uint8_t ch_msk_val;
	struct wlan_mlme_psoc_obj *mlme_obj = mlme_get_psoc_obj(psoc);

	if (!mlme_obj) {
		mlme_err("Failed to get MLME Obj");
		return QDF_STATUS_E_FAILURE;
	}

	mlme_debug("txchainmask1x1: %d rxchainmask1x1: %d",
		   mlme_obj->cfg.chainmask_cfg.txchainmask1x1,
		   mlme_obj->cfg.chainmask_cfg.rxchainmask1x1);
	mlme_debug("tx_chain_mask_2g: %d, rx_chain_mask_2g: %d",
		   mlme_obj->cfg.chainmask_cfg.tx_chain_mask_2g,
		   mlme_obj->cfg.chainmask_cfg.rx_chain_mask_2g);
	mlme_debug("tx_chain_mask_5g: %d, rx_chain_mask_5g: %d",
		   mlme_obj->cfg.chainmask_cfg.tx_chain_mask_5g,
		   mlme_obj->cfg.chainmask_cfg.rx_chain_mask_5g);

	if (mlme_obj->cfg.chainmask_cfg.txchainmask1x1) {
		ch_msk_val = mlme_obj->cfg.chainmask_cfg.txchainmask1x1;
		ret_val = wma_cli_set_command(session_id,
					      WMI_PDEV_PARAM_TX_CHAIN_MASK,
					      ch_msk_val, PDEV_CMD);
		if (ret_val)
			return QDF_STATUS_E_FAILURE;
	}

	if (mlme_obj->cfg.chainmask_cfg.rxchainmask1x1) {
		ch_msk_val = mlme_obj->cfg.chainmask_cfg.rxchainmask1x1;
		ret_val = wma_cli_set_command(session_id,
					      WMI_PDEV_PARAM_RX_CHAIN_MASK,
					      ch_msk_val, PDEV_CMD);
		if (ret_val)
			return QDF_STATUS_E_FAILURE;
	}

	if (mlme_obj->cfg.chainmask_cfg.txchainmask1x1 ||
	    mlme_obj->cfg.chainmask_cfg.rxchainmask1x1) {
		mlme_debug("band agnostic tx/rx chain mask set. skip per band chain mask");
		return QDF_STATUS_SUCCESS;
	}

	if (mlme_obj->cfg.chainmask_cfg.tx_chain_mask_2g) {
		ch_msk_val = mlme_obj->cfg.chainmask_cfg.tx_chain_mask_2g;
		ret_val = wma_cli_set_command(session_id,
					      WMI_PDEV_PARAM_TX_CHAIN_MASK_2G,
					      ch_msk_val, PDEV_CMD);
		if (0 != ret_val)
			return QDF_STATUS_E_FAILURE;
	}

	if (mlme_obj->cfg.chainmask_cfg.rx_chain_mask_2g) {
		ch_msk_val = mlme_obj->cfg.chainmask_cfg.rx_chain_mask_2g;
		ret_val = wma_cli_set_command(session_id,
					      WMI_PDEV_PARAM_RX_CHAIN_MASK_2G,
					      ch_msk_val, PDEV_CMD);
		if (0 != ret_val)
			return QDF_STATUS_E_FAILURE;
	}

	if (mlme_obj->cfg.chainmask_cfg.tx_chain_mask_5g) {
		ch_msk_val = mlme_obj->cfg.chainmask_cfg.tx_chain_mask_5g;
		ret_val = wma_cli_set_command(session_id,
					      WMI_PDEV_PARAM_TX_CHAIN_MASK_5G,
					      ch_msk_val, PDEV_CMD);
		if (0 != ret_val)
			return QDF_STATUS_E_FAILURE;
	}

	if (mlme_obj->cfg.chainmask_cfg.rx_chain_mask_5g) {
		ch_msk_val = mlme_obj->cfg.chainmask_cfg.rx_chain_mask_5g;
		ret_val = wma_cli_set_command(session_id,
					      WMI_PDEV_PARAM_RX_CHAIN_MASK_5G,
					      ch_msk_val, PDEV_CMD);
		if (0 != ret_val)
			return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}
