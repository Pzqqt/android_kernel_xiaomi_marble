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
 * DOC: define internal APIs related to the mlme component
 */

#include "wlan_mlme_main.h"
#include "cfg_ucfg_api.h"

struct wlan_mlme_psoc_obj *mlme_get_psoc_obj(struct wlan_objmgr_psoc *psoc)
{
	struct wlan_mlme_psoc_obj *mlme_obj;

	mlme_obj = (struct wlan_mlme_psoc_obj *)
		wlan_objmgr_psoc_get_comp_private_obj(psoc,
						      WLAN_UMAC_COMP_MLME);

	return mlme_obj;
}

QDF_STATUS mlme_psoc_object_created_notification(
		struct wlan_objmgr_psoc *psoc, void *arg)
{
	QDF_STATUS status;
	struct wlan_mlme_psoc_obj *mlme_obj;

	mlme_obj = qdf_mem_malloc(sizeof(struct wlan_mlme_psoc_obj));
	if (!mlme_obj) {
		mlme_err("Failed to allocate memory");
		return QDF_STATUS_E_NOMEM;
	}

	status = wlan_objmgr_psoc_component_obj_attach(psoc,
						       WLAN_UMAC_COMP_MLME,
						       mlme_obj,
						       QDF_STATUS_SUCCESS);
	if (status != QDF_STATUS_SUCCESS) {
		mlme_err("Failed to attach psoc_ctx with psoc");
		qdf_mem_free(mlme_obj);
	}

	return status;
}

QDF_STATUS mlme_psoc_object_destroyed_notification(
		struct wlan_objmgr_psoc *psoc, void *arg)
{
	struct wlan_mlme_psoc_obj *mlme_obj = NULL;
	QDF_STATUS status;

	mlme_obj = mlme_get_psoc_obj(psoc);

	status = wlan_objmgr_psoc_component_obj_detach(psoc,
						       WLAN_UMAC_COMP_MLME,
						       mlme_obj);
	if (status != QDF_STATUS_SUCCESS) {
		mlme_err("Failed to detach psoc_ctx from psoc");
		status = QDF_STATUS_E_FAILURE;
		goto out;
	}

	qdf_mem_free(mlme_obj);

out:
	return status;
}

static void mlme_init_chainmask_cfg(struct wlan_objmgr_psoc *psoc,
				    struct wlan_mlme_chainmask *chainmask_info)
{
	chainmask_info->txchainmask1x1 =
		cfg_get(psoc, CFG_VHT_ENABLE_1x1_TX_CHAINMASK);

	chainmask_info->rxchainmask1x1 =
		cfg_get(psoc, CFG_VHT_ENABLE_1x1_RX_CHAINMASK);

	chainmask_info->tx_chain_mask_cck =
		cfg_get(psoc, CFG_TX_CHAIN_MASK_CCK);

	chainmask_info->tx_chain_mask_1ss =
		cfg_get(psoc, CFG_TX_CHAIN_MASK_1SS);

	chainmask_info->num_11b_tx_chains =
		cfg_get(psoc, CFG_11B_NUM_TX_CHAIN);

	chainmask_info->num_11ag_tx_chains =
		cfg_get(psoc, CFG_11AG_NUM_TX_CHAIN);

	chainmask_info->tx_chain_mask_2g =
		cfg_get(psoc, CFG_TX_CHAIN_MASK_2G);

	chainmask_info->rx_chain_mask_2g =
		cfg_get(psoc, CFG_RX_CHAIN_MASK_2G);

	chainmask_info->tx_chain_mask_5g =
		cfg_get(psoc, CFG_TX_CHAIN_MASK_5G);

	chainmask_info->rx_chain_mask_5g =
		cfg_get(psoc, CFG_RX_CHAIN_MASK_5G);
}

static void mlme_update_ht_cap_in_cfg(struct wlan_objmgr_psoc *psoc,
				      struct mlme_ht_capabilities_info
				      *ht_cap_info)
{
	union {
		uint16_t val_16;
		struct mlme_ht_capabilities_info default_ht_cap_info;
	} u;

	u.val_16 = (uint16_t)cfg_default(CFG_HT_CAP_INFO);

	u.default_ht_cap_info.adv_coding_cap =
				cfg_get(psoc, CFG_RX_LDPC_ENABLE);
	u.default_ht_cap_info.rx_stbc = cfg_get(psoc, CFG_RX_STBC_ENABLE);
	u.default_ht_cap_info.tx_stbc = cfg_get(psoc, CFG_TX_STBC_ENABLE);
	u.default_ht_cap_info.short_gi_20_mhz =
				cfg_get(psoc, CFG_SHORT_GI_20MHZ);
	u.default_ht_cap_info.short_gi_40_mhz =
				cfg_get(psoc, CFG_SHORT_GI_40MHZ);

	*ht_cap_info = u.default_ht_cap_info;
}

static void mlme_update_rates_in_cfg(struct wlan_objmgr_psoc *psoc,
				     struct wlan_mlme_rates *rates)
{
	rates->cfp_period = cfg_default(CFG_CFP_PERIOD);
	rates->cfp_max_duration = cfg_default(CFG_CFP_MAX_DURATION);
	rates->max_htmcs_txdata = cfg_get(psoc, CFG_INI_MAX_HT_MCS_FOR_TX_DATA);
	rates->disable_abg_rate_txdata = cfg_get(psoc,
					CFG_INI_DISABLE_ABG_RATE_FOR_TX_DATA);
	rates->sap_max_mcs_txdata = cfg_get(psoc,
					CFG_INI_SAP_MAX_MCS_FOR_TX_DATA);
	rates->disable_high_ht_mcs_2x2 = cfg_get(psoc,
					 CFG_INI_DISABLE_HIGH_HT_RX_MCS_2x2);
}

static void mlme_update_sap_protection_cfg(struct wlan_objmgr_psoc *psoc,
					   struct wlan_mlme_sap_protection
					   *sap_protection_params)
{
	sap_protection_params->protection_enabled =
				cfg_default(CFG_PROTECTION_ENABLED);
	sap_protection_params->protection_force_policy =
				cfg_default(CFG_FORCE_POLICY_PROTECTION);
	sap_protection_params->ignore_peer_ht_mode =
				cfg_get(psoc, CFG_IGNORE_PEER_HT_MODE);
}

QDF_STATUS mlme_cfg_on_psoc_enable(struct wlan_objmgr_psoc *psoc)
{
	struct wlan_mlme_psoc_obj *mlme_obj;
	struct wlan_mlme_cfg *mlme_cfg;
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	mlme_obj = mlme_get_psoc_obj(psoc);
	if (!mlme_obj) {
		mlme_err("Failed to get MLME Obj");
		return QDF_STATUS_E_FAILURE;
	}

	mlme_cfg = &mlme_obj->cfg;
	mlme_update_ht_cap_in_cfg(psoc, &mlme_cfg->ht_caps.ht_cap_info);
	mlme_update_rates_in_cfg(psoc, &mlme_cfg->rates);
	mlme_update_sap_protection_cfg(psoc, &mlme_cfg->sap_protection_cfg);
	mlme_init_chainmask_cfg(psoc, &mlme_cfg->chainmask_cfg);

	return status;
}
