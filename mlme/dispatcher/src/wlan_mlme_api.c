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
#include "wma.h"
#include "wma_internal.h"

QDF_STATUS wlan_mlme_get_cfg_str(uint8_t *dst, struct mlme_cfg_str *cfg_str,
				 qdf_size_t *len)
{
	if (*len < cfg_str->len) {
		mlme_err("Invalid len %zd", *len);
		return QDF_STATUS_E_INVAL;
	}

	*len = cfg_str->len;
	qdf_mem_copy(dst, cfg_str->data, *len);

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS wlan_mlme_set_cfg_str(uint8_t *src, struct mlme_cfg_str *dst_cfg_str,
				 qdf_size_t len)
{
	if (len > dst_cfg_str->max_len) {
		mlme_err("Invalid len %zd (>%zd)", len,
			 dst_cfg_str->max_len);
		return QDF_STATUS_E_INVAL;
	}

	dst_cfg_str->len = len;
	qdf_mem_copy(dst_cfg_str->data, src, len);

	return QDF_STATUS_SUCCESS;
}

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

QDF_STATUS wlan_mlme_get_band_capability(struct wlan_objmgr_psoc *psoc,
					 uint8_t *band_capability)
{
	struct wlan_mlme_psoc_obj *mlme_obj;

	mlme_obj = mlme_get_psoc_obj(psoc);
	if (!mlme_obj) {
		mlme_err("Failed to get MLME Obj");
		return QDF_STATUS_E_FAILURE;
	}

	*band_capability = mlme_obj->cfg.gen.band_capability;

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS wlan_mlme_set_band_capability(struct wlan_objmgr_psoc *psoc,
					 uint8_t band_capability)

{
	struct wlan_mlme_psoc_obj *mlme_obj;

	mlme_obj = mlme_get_psoc_obj(psoc);
	if (!mlme_obj) {
		mlme_err("Failed to get MLME Obj");
		return QDF_STATUS_E_FAILURE;
	}

	mlme_obj->cfg.gen.band_capability = band_capability;

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS wlan_mlme_get_prevent_link_down(struct wlan_objmgr_psoc *psoc,
					   bool *prevent_link_down)
{
	struct wlan_mlme_psoc_obj *mlme_obj;

	mlme_obj = mlme_get_psoc_obj(psoc);
	if (!mlme_obj) {
		mlme_err("Failed to get MLME Obj");
		return QDF_STATUS_E_FAILURE;
	}

	*prevent_link_down = mlme_obj->cfg.gen.prevent_link_down;

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS wlan_mlme_get_select_5ghz_margin(struct wlan_objmgr_psoc *psoc,
					    uint8_t *select_5ghz_margin)
{
	struct wlan_mlme_psoc_obj *mlme_obj;

	mlme_obj = mlme_get_psoc_obj(psoc);
	if (!mlme_obj) {
		mlme_err("Failed to get MLME Obj");
		return QDF_STATUS_E_FAILURE;
	}

	*select_5ghz_margin = mlme_obj->cfg.gen.select_5ghz_margin;

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS wlan_mlme_get_crash_inject(struct wlan_objmgr_psoc *psoc,
				      bool *crash_inject)
{
	struct wlan_mlme_psoc_obj *mlme_obj;

	mlme_obj = mlme_get_psoc_obj(psoc);
	if (!mlme_obj) {
		mlme_err("Failed to get MLME Obj");
		return QDF_STATUS_E_FAILURE;
	}

	*crash_inject = mlme_obj->cfg.gen.crash_inject;

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS wlan_mlme_get_lpass_support(struct wlan_objmgr_psoc *psoc,
				       bool *lpass_support)
{
	struct wlan_mlme_psoc_obj *mlme_obj;

	mlme_obj = mlme_get_psoc_obj(psoc);
	if (!mlme_obj) {
		mlme_err("Failed to get MLME Obj");
		return QDF_STATUS_E_FAILURE;
	}

	*lpass_support = mlme_obj->cfg.gen.lpass_support;

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS wlan_mlme_get_self_recovery(struct wlan_objmgr_psoc *psoc,
				       bool *self_recovery)
{
	struct wlan_mlme_psoc_obj *mlme_obj;

	mlme_obj = mlme_get_psoc_obj(psoc);
	if (!mlme_obj) {
		mlme_err("Failed to get MLME Obj");
		return QDF_STATUS_E_FAILURE;
	}

	*self_recovery = mlme_obj->cfg.gen.self_recovery;

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS wlan_mlme_get_sub_20_chan_width(struct wlan_objmgr_psoc *psoc,
					   uint8_t *sub_20_chan_width)
{
	struct wlan_mlme_psoc_obj *mlme_obj;

	mlme_obj = mlme_get_psoc_obj(psoc);
	if (!mlme_obj) {
		mlme_err("Failed to get MLME Obj");
		return QDF_STATUS_E_FAILURE;
	}

	*sub_20_chan_width = mlme_obj->cfg.gen.sub_20_chan_width;

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS wlan_mlme_get_fw_timeout_crash(struct wlan_objmgr_psoc *psoc,
					  bool *fw_timeout_crash)
{
	struct wlan_mlme_psoc_obj *mlme_obj;

	mlme_obj = mlme_get_psoc_obj(psoc);
	if (!mlme_obj) {
		mlme_err("Failed to get MLME Obj");
		return QDF_STATUS_E_FAILURE;
	}

	*fw_timeout_crash = mlme_obj->cfg.gen.fw_timeout_crash;

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS wlan_mlme_get_ito_repeat_count(struct wlan_objmgr_psoc *psoc,
					  uint8_t *ito_repeat_count)
{
	struct wlan_mlme_psoc_obj *mlme_obj;

	mlme_obj = mlme_get_psoc_obj(psoc);
	if (!mlme_obj) {
		mlme_err("Failed to get MLME Obj");
		return QDF_STATUS_E_FAILURE;
	}

	*ito_repeat_count = mlme_obj->cfg.gen.ito_repeat_count;

	return QDF_STATUS_SUCCESS;
}

void wlan_mlme_get_sap_inactivity_override(struct wlan_objmgr_psoc *psoc,
					   bool *val)
{
	struct wlan_mlme_psoc_obj *mlme_obj;

	mlme_obj = mlme_get_psoc_obj(psoc);
	if (!mlme_obj) {
		mlme_err("Failed to get MLME Obj");
		return;
	}
	*val = mlme_obj->cfg.qos_mlme_params.sap_max_inactivity_override;
}

QDF_STATUS wlan_mlme_get_acs_with_more_param(struct wlan_objmgr_psoc *psoc,
					     bool *value)
{
	struct wlan_mlme_psoc_obj *mlme_obj = mlme_get_psoc_obj(psoc);

	if (!mlme_obj) {
		mlme_err("Failed to get MLME Obj");
		return QDF_STATUS_E_FAILURE;
	}

	*value = mlme_obj->cfg.acs.is_acs_with_more_param;
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS wlan_mlme_get_auto_channel_weight(struct wlan_objmgr_psoc *psoc,
					     uint32_t *value)
{
	struct wlan_mlme_psoc_obj *mlme_obj = mlme_get_psoc_obj(psoc);

	if (!mlme_obj) {
		mlme_err("Failed to get MLME Obj");
		return QDF_STATUS_E_FAILURE;
	}

	*value = mlme_obj->cfg.acs.auto_channel_select_weight;
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS wlan_mlme_get_vendor_acs_support(struct wlan_objmgr_psoc *psoc,
					    bool *value)
{
	struct wlan_mlme_psoc_obj *mlme_obj = mlme_get_psoc_obj(psoc);

	if (!mlme_obj) {
		mlme_err("Failed to get MLME Obj");
		return QDF_STATUS_E_FAILURE;
	}

	*value = mlme_obj->cfg.acs.is_vendor_acs_support;
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
wlan_mlme_get_acs_support_for_dfs_ltecoex(struct wlan_objmgr_psoc *psoc,
					  bool *value)
{
	struct wlan_mlme_psoc_obj *mlme_obj = mlme_get_psoc_obj(psoc);

	if (!mlme_obj) {
		mlme_err("Failed to get MLME Obj");
		return QDF_STATUS_E_FAILURE;
	}

	*value = mlme_obj->cfg.acs.is_acs_support_for_dfs_ltecoex;
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
wlan_mlme_get_external_acs_policy(struct wlan_objmgr_psoc *psoc,
				  bool *value)
{
	struct wlan_mlme_psoc_obj *mlme_obj = mlme_get_psoc_obj(psoc);

	if (!mlme_obj) {
		mlme_err("Failed to get MLME Obj");
		return QDF_STATUS_E_FAILURE;
	}

	*value = mlme_obj->cfg.acs.is_external_acs_policy;
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

QDF_STATUS
wlan_mlme_get_manufacturer_name(struct wlan_objmgr_psoc *psoc,
				uint8_t *pbuf, uint32_t *plen)
{
	struct wlan_mlme_psoc_obj *mlme_obj;

	mlme_obj = mlme_get_psoc_obj(psoc);
	if (!mlme_obj) {
		mlme_err("Failed to get MLME Obj");
		return QDF_STATUS_E_FAILURE;
	}

	*plen = qdf_str_lcopy(pbuf,
			      mlme_obj->cfg.product_details.manufacturer_name,
			      *plen);
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
wlan_mlme_get_model_number(struct wlan_objmgr_psoc *psoc,
			   uint8_t *pbuf, uint32_t *plen)
{
	struct wlan_mlme_psoc_obj *mlme_obj;

	mlme_obj = mlme_get_psoc_obj(psoc);
	if (!mlme_obj) {
		mlme_err("Failed to get MLME Obj");
		return QDF_STATUS_E_FAILURE;
	}

	*plen = qdf_str_lcopy(pbuf,
			      mlme_obj->cfg.product_details.model_number,
			      *plen);

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
wlan_mlme_get_model_name(struct wlan_objmgr_psoc *psoc,
			 uint8_t *pbuf, uint32_t *plen)
{
	struct wlan_mlme_psoc_obj *mlme_obj;

	mlme_obj = mlme_get_psoc_obj(psoc);
	if (!mlme_obj) {
		mlme_err("Failed to get MLME Obj");
		return QDF_STATUS_E_FAILURE;
	}
	*plen = qdf_str_lcopy(pbuf,
			      mlme_obj->cfg.product_details.model_name,
			      *plen);
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
wlan_mlme_get_manufacture_product_version(struct wlan_objmgr_psoc *psoc,
					  uint8_t *pbuf, uint32_t *plen)
{
	struct wlan_mlme_psoc_obj *mlme_obj;

	mlme_obj = mlme_get_psoc_obj(psoc);
	if (!mlme_obj) {
		mlme_err("Failed to get MLME Obj");
		return QDF_STATUS_E_FAILURE;
	}

	*plen = qdf_str_lcopy(pbuf,
		     mlme_obj->cfg.product_details.manufacture_product_version,
		     *plen);
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
wlan_mlme_get_manufacture_product_name(struct wlan_objmgr_psoc *psoc,
				       uint8_t *pbuf, uint32_t *plen)
{
	struct wlan_mlme_psoc_obj *mlme_obj;

	mlme_obj = mlme_get_psoc_obj(psoc);
	if (!mlme_obj) {
		mlme_err("Failed to get MLME Obj");
		return QDF_STATUS_E_FAILURE;
	}

	*plen = qdf_str_lcopy(pbuf,
			mlme_obj->cfg.product_details.manufacture_product_name,
			*plen);
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS wlan_mlme_get_wmm_dir_ac_vo(struct wlan_objmgr_psoc *psoc,
				       uint8_t *value)
{
	struct wlan_mlme_psoc_obj *mlme_obj;

	mlme_obj = mlme_get_psoc_obj(psoc);
	if (!mlme_obj) {
		mlme_err("Failed to get MLME Obj");
		return QDF_STATUS_E_FAILURE;
	}
	*value = mlme_obj->cfg.wmm_params.ac_vo.dir_ac_vo;

	return QDF_STATUS_SUCCESS;
}


QDF_STATUS wlan_mlme_get_wmm_nom_msdu_size_ac_vo(struct wlan_objmgr_psoc *psoc,
						 uint16_t *value)
{
	struct wlan_mlme_psoc_obj *mlme_obj;

	mlme_obj = mlme_get_psoc_obj(psoc);
	if (!mlme_obj) {
		mlme_err("Failed to get MLME Obj");
		return QDF_STATUS_E_FAILURE;
	}

	*value = mlme_obj->cfg.wmm_params.ac_vo.nom_msdu_size_ac_vo;
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
wlan_mlme_get_wmm_mean_data_rate_ac_vo(struct wlan_objmgr_psoc *psoc,
				       uint32_t *value)
{
	struct wlan_mlme_psoc_obj *mlme_obj;

	mlme_obj = mlme_get_psoc_obj(psoc);
	if (!mlme_obj) {
		mlme_err("failed to get mlme obj");
		return QDF_STATUS_E_FAILURE;
	}

	*value = mlme_obj->cfg.wmm_params.ac_vo.mean_data_rate_ac_vo;

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS wlan_mlme_get_wmm_min_phy_rate_ac_vo(struct wlan_objmgr_psoc *psoc,
						uint32_t *value)
{
	struct wlan_mlme_psoc_obj *mlme_obj;

	mlme_obj = mlme_get_psoc_obj(psoc);
	if (!mlme_obj) {
		mlme_err("failed to get mlme obj");
		return QDF_STATUS_E_FAILURE;
	}

	*value = mlme_obj->cfg.wmm_params.ac_vo.min_phy_rate_ac_vo;
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS wlan_mlme_get_wmm_sba_ac_vo(struct wlan_objmgr_psoc *psoc,
				       uint16_t *value)
{
	struct wlan_mlme_psoc_obj *mlme_obj;

	mlme_obj = mlme_get_psoc_obj(psoc);
	if (!mlme_obj) {
		mlme_err("Failed to get MLME Obj");
		return QDF_STATUS_E_FAILURE;
	}

	*value = mlme_obj->cfg.wmm_params.ac_vo.sba_ac_vo;

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS wlan_mlme_get_wmm_uapsd_vo_srv_intv(struct wlan_objmgr_psoc *psoc,
					       uint32_t *value)
{
	struct wlan_mlme_psoc_obj *mlme_obj;

	mlme_obj = mlme_get_psoc_obj(psoc);
	if (!mlme_obj) {
		mlme_err("Failed to get MLME Obj");
		return QDF_STATUS_E_FAILURE;
	}

	*value = mlme_obj->cfg.wmm_params.ac_vo.uapsd_vo_srv_intv;

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS wlan_mlme_get_wmm_uapsd_vo_sus_intv(struct wlan_objmgr_psoc *psoc,
					       uint32_t *value)
{
	struct wlan_mlme_psoc_obj *mlme_obj;

	mlme_obj = mlme_get_psoc_obj(psoc);
	if (!mlme_obj) {
		mlme_err("Failed to get MLME Obj");
		return QDF_STATUS_E_FAILURE;
	}

	*value = mlme_obj->cfg.wmm_params.ac_vo.uapsd_vo_sus_intv;

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS wlan_mlme_set_sap_listen_interval(struct wlan_objmgr_psoc *psoc,
					     int value)
{
	struct wlan_mlme_psoc_obj *mlme_obj;

	mlme_obj = mlme_get_psoc_obj(psoc);
	if (!mlme_obj) {
		mlme_err("Failed to get MLME Obj");
		return QDF_STATUS_E_FAILURE;
	}

	if (cfg_in_range(CFG_LISTEN_INTERVAL, value))
		mlme_obj->cfg.sap_cfg.listen_interval = value;
	else
		return QDF_STATUS_E_FAILURE;

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS wlan_mlme_set_assoc_sta_limit(struct wlan_objmgr_psoc *psoc,
					 int value)
{
	struct wlan_mlme_psoc_obj *mlme_obj;

	mlme_obj = mlme_get_psoc_obj(psoc);
	if (!mlme_obj) {
		mlme_err("Failed to get MLME Obj");
		return QDF_STATUS_E_FAILURE;
	}

	if (cfg_in_range(CFG_ASSOC_STA_LIMIT, value))
		mlme_obj->cfg.sap_cfg.assoc_sta_limit = value;
	else
		return QDF_STATUS_E_FAILURE;

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS wlan_mlme_set_rmc_action_period_freq(struct wlan_objmgr_psoc *psoc,
						int value)
{
	struct wlan_mlme_psoc_obj *mlme_obj;

	mlme_obj = mlme_get_psoc_obj(psoc);
	if (!mlme_obj) {
		mlme_err("Failed to get MLME Obj");
		return QDF_STATUS_E_FAILURE;
	}

	if (cfg_in_range(CFG_RMC_ACTION_PERIOD_FREQUENCY, value))
		mlme_obj->cfg.sap_cfg.rmc_action_period_freq = value;
	else
		return QDF_STATUS_E_FAILURE;

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS wlan_mlme_get_sap_get_peer_info(struct wlan_objmgr_psoc *psoc,
					   bool *value)
{
	struct wlan_mlme_psoc_obj *mlme_obj;

	mlme_obj = mlme_get_psoc_obj(psoc);
	if (!mlme_obj) {
		mlme_err("Failed to get MLME Obj");
		return QDF_STATUS_E_FAILURE;
	}

	*value = mlme_obj->cfg.sap_cfg.sap_get_peer_info;

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS wlan_mlme_get_sap_allow_all_channels(struct wlan_objmgr_psoc *psoc,
						bool *value)
{
	struct wlan_mlme_psoc_obj *mlme_obj;

	mlme_obj = mlme_get_psoc_obj(psoc);
	if (!mlme_obj) {
		mlme_err("Failed to get MLME Obj");
		return QDF_STATUS_E_FAILURE;
	}

	*value = mlme_obj->cfg.sap_cfg.sap_allow_all_chan_param_name;

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS wlan_mlme_get_sap_max_peers(struct wlan_objmgr_psoc *psoc,
				       int *value)
{
	struct wlan_mlme_psoc_obj *mlme_obj;

	mlme_obj = mlme_get_psoc_obj(psoc);
	if (!mlme_obj) {
		mlme_err("Failed to get MLME Obj");
		return QDF_STATUS_E_FAILURE;
	}

	*value = mlme_obj->cfg.sap_cfg.sap_max_no_peers;

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS wlan_mlme_set_sap_max_peers(struct wlan_objmgr_psoc *psoc,
				       int value)
{
	struct wlan_mlme_psoc_obj *mlme_obj;

	mlme_obj = mlme_get_psoc_obj(psoc);
	if (!mlme_obj) {
		mlme_err("Failed to get MLME Obj");
		return QDF_STATUS_E_FAILURE;
	}

	if (cfg_in_range(CFG_RMC_ACTION_PERIOD_FREQUENCY, value))
		mlme_obj->cfg.sap_cfg.sap_max_no_peers = value;
	else
		return QDF_STATUS_E_FAILURE;

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS wlan_mlme_get_sap_max_offload_peers(struct wlan_objmgr_psoc *psoc,
					       int *value)
{
	struct wlan_mlme_psoc_obj *mlme_obj;

	mlme_obj = mlme_get_psoc_obj(psoc);
	if (!mlme_obj) {
		mlme_err("Failed to get MLME Obj");
		return QDF_STATUS_E_FAILURE;
	}

	*value = mlme_obj->cfg.sap_cfg.sap_max_offload_peers;

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS wlan_mlme_get_sap_max_offload_reorder_buffs(struct wlan_objmgr_psoc
						       *psoc, int *value)
{
	struct wlan_mlme_psoc_obj *mlme_obj;

	mlme_obj = mlme_get_psoc_obj(psoc);
	if (!mlme_obj) {
		mlme_err("Failed to get MLME Obj");
		return QDF_STATUS_E_FAILURE;
	}

	*value = mlme_obj->cfg.sap_cfg.sap_max_offload_reorder_buffs;

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS wlan_mlme_get_sap_chn_switch_bcn_count(struct wlan_objmgr_psoc *psoc,
						  int *value)
{
	struct wlan_mlme_psoc_obj *mlme_obj;

	mlme_obj = mlme_get_psoc_obj(psoc);
	if (!mlme_obj) {
		mlme_err("Failed to get MLME Obj");
		return QDF_STATUS_E_FAILURE;
	}

	*value = mlme_obj->cfg.sap_cfg.sap_ch_switch_beacon_cnt;

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS wlan_mlme_get_sap_chn_switch_mode(struct wlan_objmgr_psoc *psoc,
					     bool *value)
{
	struct wlan_mlme_psoc_obj *mlme_obj;

	mlme_obj = mlme_get_psoc_obj(psoc);
	if (!mlme_obj) {
		mlme_err("Failed to get MLME Obj");
		return QDF_STATUS_E_FAILURE;
	}

	*value = mlme_obj->cfg.sap_cfg.sap_ch_switch_mode;

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS wlan_mlme_get_sap_internal_restart(struct wlan_objmgr_psoc *psoc,
					      bool *value)
{
	struct wlan_mlme_psoc_obj *mlme_obj;

	mlme_obj = mlme_get_psoc_obj(psoc);
	if (!mlme_obj) {
		mlme_err("Failed to get MLME Obj");
		return QDF_STATUS_E_FAILURE;
	}

	*value = mlme_obj->cfg.sap_cfg.sap_internal_restart;

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS wlan_mlme_get_sap_reduced_beacon_interval(struct wlan_objmgr_psoc
						     *psoc, int *value)
{
	struct wlan_mlme_psoc_obj *mlme_obj;

	mlme_obj = mlme_get_psoc_obj(psoc);
	if (!mlme_obj) {
		mlme_err("Failed to get MLME Obj");
		return QDF_STATUS_E_FAILURE;
	}

	*value = mlme_obj->cfg.sap_cfg.reduced_beacon_interval;

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS wlan_mlme_get_sap_chan_switch_rate_enabled(struct wlan_objmgr_psoc
						      *psoc, bool *value)
{
	struct wlan_mlme_psoc_obj *mlme_obj;

	mlme_obj = mlme_get_psoc_obj(psoc);
	if (!mlme_obj) {
		mlme_err("Failed to get MLME Obj");
		return QDF_STATUS_E_FAILURE;
	}

	*value = mlme_obj->cfg.sap_cfg.chan_switch_hostapd_rate_enabled_name;

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS wlan_mlme_get_oce_sta_enabled_info(struct wlan_objmgr_psoc *psoc,
					      bool *value)
{
	struct wlan_mlme_psoc_obj *mlme_obj = mlme_get_psoc_obj(psoc);

	if (!mlme_obj) {
		mlme_err("Failed to get MLME Obj");
		return QDF_STATUS_E_FAILURE;
	}

	*value = mlme_obj->cfg.oce.oce_sta_enabled;
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS wlan_mlme_get_oce_sap_enabled_info(struct wlan_objmgr_psoc *psoc,
					      bool *value)
{
	struct wlan_mlme_psoc_obj *mlme_obj = mlme_get_psoc_obj(psoc);

	if (!mlme_obj) {
		mlme_err("Failed to get MLME Obj");
		return QDF_STATUS_E_FAILURE;
	}

	*value = mlme_obj->cfg.oce.oce_sap_enabled;
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS wlan_mlme_get_rts_threshold(struct wlan_objmgr_psoc *psoc,
				       uint32_t *value)
{
	struct wlan_mlme_psoc_obj *mlme_obj;

	mlme_obj = mlme_get_psoc_obj(psoc);
	if (!mlme_obj) {
		mlme_err("Failed to get MLME Obj");
		return QDF_STATUS_E_FAILURE;
	}
	*value = mlme_obj->cfg.threshold.rts_threshold;

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS wlan_mlme_set_rts_threshold(struct wlan_objmgr_psoc *psoc,
				       uint32_t value)
{
	struct wlan_mlme_psoc_obj *mlme_obj;
	tp_wma_handle wma_handle;

	wma_handle = cds_get_context(QDF_MODULE_ID_WMA);

	if (NULL == wma_handle) {
		WMA_LOGE("%s: wma_handle is NULL", __func__);
		return QDF_STATUS_E_INVAL;
	}

	mlme_obj = mlme_get_psoc_obj(psoc);

	if (!mlme_obj) {
		mlme_err("Failed to get MLME Obj");
		return QDF_STATUS_E_FAILURE;
	}

	mlme_obj->cfg.threshold.rts_threshold = value;
	wma_update_rts_params(wma_handle, value);

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS wlan_mlme_get_frag_threshold(struct wlan_objmgr_psoc *psoc,
					uint32_t *value)
{
	struct wlan_mlme_psoc_obj *mlme_obj;

	mlme_obj = mlme_get_psoc_obj(psoc);
	if (!mlme_obj) {
		mlme_err("Failed to get MLME Obj");
		return QDF_STATUS_E_FAILURE;
	}

	*value = mlme_obj->cfg.threshold.frag_threshold;

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS wlan_mlme_set_frag_threshold(struct wlan_objmgr_psoc *psoc,
					uint32_t value)
{
	struct wlan_mlme_psoc_obj *mlme_obj;
	tp_wma_handle wma_handle;

	wma_handle = cds_get_context(QDF_MODULE_ID_WMA);

	if (NULL == wma_handle) {
		WMA_LOGE("%s: wma_handle is NULL", __func__);
		return QDF_STATUS_E_INVAL;
	}

	mlme_obj = mlme_get_psoc_obj(psoc);

	if (!mlme_obj) {
		mlme_err("Failed to get MLME Obj");
		return QDF_STATUS_E_FAILURE;
	}

	mlme_obj->cfg.threshold.frag_threshold = value;
	wma_update_frag_params(wma_handle,
			       value);

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS wlan_mlme_get_fils_enabled_info(struct wlan_objmgr_psoc *psoc,
					   bool *value)
{
	struct wlan_mlme_psoc_obj *mlme_obj = mlme_get_psoc_obj(psoc);

	if (!mlme_obj) {
		mlme_err("Failed to get MLME Obj");
		return QDF_STATUS_E_FAILURE;
	}

	*value = mlme_obj->cfg.oce.fils_enabled;
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS wlan_mlme_set_fils_enabled_info(struct wlan_objmgr_psoc *psoc,
					   bool value)
{
	struct wlan_mlme_psoc_obj *mlme_obj = mlme_get_psoc_obj(psoc);

	if (!mlme_obj) {
		mlme_err("Failed to get MLME Obj");
		return QDF_STATUS_E_FAILURE;
	}

	mlme_obj->cfg.oce.fils_enabled = value;
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS wlan_mlme_set_enable_bcast_probe_rsp(struct wlan_objmgr_psoc *psoc,
						bool value)
{
	struct wlan_mlme_psoc_obj *mlme_obj = mlme_get_psoc_obj(psoc);

	if (!mlme_obj) {
		mlme_err("Failed to get MLME Obj");
		return QDF_STATUS_E_FAILURE;
	}

	mlme_obj->cfg.oce.enable_bcast_probe_rsp = value;
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS wlan_mlme_get_edca_params(struct wlan_mlme_edca_params *edca_params,
				     uint8_t *data, enum e_edca_type edca_ac)
{
	qdf_size_t len;

	switch (edca_ac) {
	case edca_ani_acbe_local:
		len = edca_params->ani_acbe_l.len;
		wlan_mlme_get_cfg_str(data, &edca_params->ani_acbe_l, &len);
		break;

	case edca_ani_acbk_local:
		len = edca_params->ani_acbk_l.len;
		wlan_mlme_get_cfg_str(data, &edca_params->ani_acbk_l, &len);
		break;

	case edca_ani_acvi_local:
		len = edca_params->ani_acvi_l.len;
		wlan_mlme_get_cfg_str(data, &edca_params->ani_acvi_l, &len);
		break;

	case edca_ani_acvo_local:
		len = edca_params->ani_acvo_l.len;
		wlan_mlme_get_cfg_str(data, &edca_params->ani_acvo_l, &len);
		break;

	case edca_ani_acbk_bcast:
		len = edca_params->ani_acbk_b.len;
		wlan_mlme_get_cfg_str(data, &edca_params->ani_acbk_b, &len);
		break;

	case edca_ani_acbe_bcast:
		len = edca_params->ani_acbe_b.len;
		wlan_mlme_get_cfg_str(data, &edca_params->ani_acbe_b, &len);
		break;

	case edca_ani_acvi_bcast:
		len = edca_params->ani_acvi_b.len;
		wlan_mlme_get_cfg_str(data, &edca_params->ani_acvi_b, &len);
		break;

	case edca_ani_acvo_bcast:
		len = edca_params->ani_acvo_b.len;
		wlan_mlme_get_cfg_str(data, &edca_params->ani_acvo_b, &len);
		break;

	case edca_wme_acbe_local:
		len = edca_params->wme_acbe_l.len;
		wlan_mlme_get_cfg_str(data, &edca_params->wme_acbe_l, &len);
		break;

	case edca_wme_acbk_local:
		len = edca_params->wme_acbk_l.len;
		wlan_mlme_get_cfg_str(data, &edca_params->wme_acbk_l, &len);
		break;

	case edca_wme_acvi_local:
		len = edca_params->wme_acvi_l.len;
		wlan_mlme_get_cfg_str(data, &edca_params->wme_acvi_l, &len);
		break;

	case edca_wme_acvo_local:
		len = edca_params->wme_acvo_l.len;
		wlan_mlme_get_cfg_str(data, &edca_params->wme_acvo_l, &len);
		break;

	case edca_wme_acbe_bcast:
		len = edca_params->wme_acbe_b.len;
		wlan_mlme_get_cfg_str(data, &edca_params->wme_acbe_b, &len);
		break;

	case edca_wme_acbk_bcast:
		len = edca_params->wme_acbk_b.len;
		wlan_mlme_get_cfg_str(data, &edca_params->wme_acbk_b, &len);
		break;

	case edca_wme_acvi_bcast:
		len = edca_params->wme_acvi_b.len;
		wlan_mlme_get_cfg_str(data, &edca_params->wme_acvi_b, &len);
		break;

	case edca_wme_acvo_bcast:
		len = edca_params->wme_acvo_b.len;
		wlan_mlme_get_cfg_str(data, &edca_params->wme_acvo_b, &len);
		break;

	case edca_etsi_acbe_local:
		len = edca_params->etsi_acbe_l.len;
		wlan_mlme_get_cfg_str(data, &edca_params->etsi_acbe_l, &len);
		break;

	case edca_etsi_acbk_local:
		len = edca_params->etsi_acbk_l.len;
		wlan_mlme_get_cfg_str(data, &edca_params->etsi_acbk_l, &len);
		break;

	case edca_etsi_acvi_local:
		len = edca_params->etsi_acvi_l.len;
		wlan_mlme_get_cfg_str(data, &edca_params->etsi_acvi_l, &len);
		break;

	case edca_etsi_acvo_local:
		len = edca_params->etsi_acvo_l.len;
		wlan_mlme_get_cfg_str(data, &edca_params->etsi_acvo_l, &len);
		break;

	case edca_etsi_acbe_bcast:
		len = edca_params->etsi_acbe_b.len;
		wlan_mlme_get_cfg_str(data, &edca_params->etsi_acbe_b, &len);
		break;

	case edca_etsi_acbk_bcast:
		len = edca_params->etsi_acbk_b.len;
		wlan_mlme_get_cfg_str(data, &edca_params->etsi_acbk_b, &len);
		break;

	case edca_etsi_acvi_bcast:
		len = edca_params->etsi_acvi_b.len;
		wlan_mlme_get_cfg_str(data, &edca_params->etsi_acvi_b, &len);
		break;

	case edca_etsi_acvo_bcast:
		len = edca_params->etsi_acvo_b.len;
		wlan_mlme_get_cfg_str(data, &edca_params->etsi_acvo_b, &len);
		break;
	default:
		mlme_err("Invalid edca access category");
		return QDF_STATUS_E_INVAL;
	}
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS mlme_get_wep_key(struct wlan_mlme_wep_cfg *wep_params,
			    enum wep_key_id wep_keyid, uint8_t *default_key,
			    qdf_size_t key_len)
{
	switch (wep_keyid) {
	case MLME_WEP_DEFAULT_KEY_1:
		wlan_mlme_get_cfg_str(default_key,
				      &wep_params->wep_default_key_1,
				      &key_len);
		break;

	case MLME_WEP_DEFAULT_KEY_2:
		wlan_mlme_get_cfg_str(default_key,
				      &wep_params->wep_default_key_2,
				      &key_len);
		break;

	case MLME_WEP_DEFAULT_KEY_3:
		wlan_mlme_get_cfg_str(default_key,
				      &wep_params->wep_default_key_3,
				      &key_len);
		break;

	case MLME_WEP_DEFAULT_KEY_4:
		wlan_mlme_get_cfg_str(default_key,
				      &wep_params->wep_default_key_4,
				      &key_len);
		break;

	default:
		mlme_err("Invalid key id:%d", wep_keyid);
		return QDF_STATUS_E_INVAL;
	}
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS mlme_set_wep_key(struct wlan_mlme_wep_cfg *wep_params,
			    enum wep_key_id wep_keyid, uint8_t *key_to_set,
			    qdf_size_t len)
{
	if (len == 0) {
		mlme_debug("WEP set key length is zero");
		return QDF_STATUS_E_FAILURE;
	}

	switch (wep_keyid) {
	case MLME_WEP_DEFAULT_KEY_1:
		wlan_mlme_set_cfg_str(key_to_set,
				      &wep_params->wep_default_key_1,
				      len);
		break;

	case MLME_WEP_DEFAULT_KEY_2:
		wlan_mlme_set_cfg_str(key_to_set,
				      &wep_params->wep_default_key_2,
				      len);
		break;

	case MLME_WEP_DEFAULT_KEY_3:
		wlan_mlme_set_cfg_str(key_to_set,
				      &wep_params->wep_default_key_3,
				      len);
		break;

	case MLME_WEP_DEFAULT_KEY_4:
		wlan_mlme_set_cfg_str(key_to_set,
				      &wep_params->wep_default_key_4,
				      len);
		break;

	default:
		mlme_err("Invalid key id:%d", wep_keyid);
		return QDF_STATUS_E_INVAL;
	}

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
wlan_mlme_cfg_set_vht_chan_width(struct wlan_objmgr_psoc *psoc, uint8_t value)
{
	struct wlan_mlme_psoc_obj *mlme_obj;

	mlme_obj = mlme_get_psoc_obj(psoc);
	if (!mlme_obj) {
		mlme_err("Failed to get MLME Obj");
		return QDF_STATUS_E_FAILURE;
	}

	mlme_obj->cfg.vht_caps.vht_cap_info.supp_chan_width = value;

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
wlan_mlme_cfg_get_vht_chan_width(struct wlan_objmgr_psoc *psoc, uint8_t *value)
{
	struct wlan_mlme_psoc_obj *mlme_obj;

	mlme_obj = mlme_get_psoc_obj(psoc);
	if (!mlme_obj) {
		mlme_err("Failed to get MLME Obj");
		return QDF_STATUS_E_FAILURE;
	}

	*value = mlme_obj->cfg.vht_caps.vht_cap_info.supp_chan_width;

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
wlan_mlme_cfg_set_vht_ldpc_coding_cap(struct wlan_objmgr_psoc *psoc,
				      bool value)
{
	struct wlan_mlme_psoc_obj *mlme_obj;

	mlme_obj = mlme_get_psoc_obj(psoc);
	if (!mlme_obj) {
		mlme_err("Failed to get MLME Obj");
		return QDF_STATUS_E_FAILURE;
	}

	mlme_obj->cfg.vht_caps.vht_cap_info.ldpc_coding_cap = value;

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
wlan_mlme_cfg_set_short_gi_160_mhz(struct wlan_objmgr_psoc *psoc,
				   bool value)
{
	struct wlan_mlme_psoc_obj *mlme_obj;

	mlme_obj = mlme_get_psoc_obj(psoc);
	if (!mlme_obj) {
		mlme_err("Failed to get MLME Obj");
		return QDF_STATUS_E_FAILURE;
	}

	mlme_obj->cfg.vht_caps.vht_cap_info.short_gi_160mhz = value;

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
wlan_mlme_cfg_get_short_gi_160_mhz(struct wlan_objmgr_psoc *psoc, bool *value)
{
	struct wlan_mlme_psoc_obj *mlme_obj;

	mlme_obj = mlme_get_psoc_obj(psoc);
	if (!mlme_obj) {
		mlme_err("Failed to get MLME Obj");
		return QDF_STATUS_E_FAILURE;
	}

	*value = mlme_obj->cfg.vht_caps.vht_cap_info.short_gi_160mhz;

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
wlan_mlme_cfg_get_vht_tx_stbc(struct wlan_objmgr_psoc *psoc, bool *value)
{
	struct wlan_mlme_psoc_obj *mlme_obj;

	mlme_obj = mlme_get_psoc_obj(psoc);
	if (!mlme_obj) {
		mlme_err("Failed to get MLME Obj");
		return QDF_STATUS_E_FAILURE;
	}

	*value = mlme_obj->cfg.vht_caps.vht_cap_info.tx_stbc;

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
wlan_mlme_cfg_get_vht_rx_stbc(struct wlan_objmgr_psoc *psoc, bool *value)
{
	struct wlan_mlme_psoc_obj *mlme_obj;

	mlme_obj = mlme_get_psoc_obj(psoc);
	if (!mlme_obj) {
		mlme_err("Failed to get MLME Obj");
		return QDF_STATUS_E_FAILURE;
	}

	*value = mlme_obj->cfg.vht_caps.vht_cap_info.rx_stbc;

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
wlan_mlme_cfg_set_vht_tx_bfee_ant_supp(struct wlan_objmgr_psoc *psoc,
				       uint8_t value)
{
	struct wlan_mlme_psoc_obj *mlme_obj;

	mlme_obj = mlme_get_psoc_obj(psoc);
	if (!mlme_obj) {
		mlme_err("Failed to get MLME Obj");
		return QDF_STATUS_E_FAILURE;
	}

	mlme_obj->cfg.vht_caps.vht_cap_info.tx_bfee_ant_supp = value;

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
wlan_mlme_cfg_get_vht_tx_bfee_ant_supp(struct wlan_objmgr_psoc *psoc,
				       uint8_t *value)
{
	struct wlan_mlme_psoc_obj *mlme_obj;

	mlme_obj = mlme_get_psoc_obj(psoc);
	if (!mlme_obj) {
		mlme_err("Failed to get MLME Obj");
		return QDF_STATUS_E_FAILURE;
	}

	*value = mlme_obj->cfg.vht_caps.vht_cap_info.tx_bfee_ant_supp;

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
wlan_mlme_cfg_get_vht_rx_mcs_map(struct wlan_objmgr_psoc *psoc,
				 uint32_t *value)
{
	struct wlan_mlme_psoc_obj *mlme_obj;

	mlme_obj = mlme_get_psoc_obj(psoc);
	if (!mlme_obj) {
		mlme_err("Failed to get MLME Obj");
		return QDF_STATUS_E_FAILURE;
	}

	*value = mlme_obj->cfg.vht_caps.vht_cap_info.rx_mcs_map;

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
wlan_mlme_cfg_set_vht_rx_mcs_map(struct wlan_objmgr_psoc *psoc,
				 uint32_t value)
{
	struct wlan_mlme_psoc_obj *mlme_obj;

	mlme_obj = mlme_get_psoc_obj(psoc);
	if (!mlme_obj) {
		mlme_err("Failed to get MLME Obj");
		return QDF_STATUS_E_FAILURE;
	}

	mlme_obj->cfg.vht_caps.vht_cap_info.rx_mcs_map = value;

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
wlan_mlme_cfg_get_vht_tx_mcs_map(struct wlan_objmgr_psoc *psoc,
				 uint32_t *value)
{
	struct wlan_mlme_psoc_obj *mlme_obj;

	mlme_obj = mlme_get_psoc_obj(psoc);
	if (!mlme_obj) {
		mlme_err("Failed to get MLME Obj");
		return QDF_STATUS_E_FAILURE;
	}

	*value = mlme_obj->cfg.vht_caps.vht_cap_info.tx_mcs_map;

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
wlan_mlme_cfg_set_vht_tx_mcs_map(struct wlan_objmgr_psoc *psoc,
				 uint32_t value)
{
	struct wlan_mlme_psoc_obj *mlme_obj;

	mlme_obj = mlme_get_psoc_obj(psoc);
	if (!mlme_obj) {
		mlme_err("Failed to get MLME Obj");
		return QDF_STATUS_E_FAILURE;
	}

	mlme_obj->cfg.vht_caps.vht_cap_info.tx_mcs_map = value;

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
wlan_mlme_cfg_set_vht_rx_supp_data_rate(struct wlan_objmgr_psoc *psoc,
				        uint32_t value)
{
	struct wlan_mlme_psoc_obj *mlme_obj;

	mlme_obj = mlme_get_psoc_obj(psoc);
	if (!mlme_obj) {
		mlme_err("Failed to get MLME Obj");
		return QDF_STATUS_E_FAILURE;
	}

	mlme_obj->cfg.vht_caps.vht_cap_info.rx_supp_data_rate = value;

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
wlan_mlme_cfg_set_vht_tx_supp_data_rate(struct wlan_objmgr_psoc *psoc,
				        uint32_t value)
{
	struct wlan_mlme_psoc_obj *mlme_obj;

	mlme_obj = mlme_get_psoc_obj(psoc);
	if (!mlme_obj) {
		mlme_err("Failed to get MLME Obj");
		return QDF_STATUS_E_FAILURE;
	}

	mlme_obj->cfg.vht_caps.vht_cap_info.tx_supp_data_rate = value;

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
wlan_mlme_cfg_get_vht_basic_mcs_set(struct wlan_objmgr_psoc *psoc,
				    uint32_t *value)
{
	struct wlan_mlme_psoc_obj *mlme_obj;

	mlme_obj = mlme_get_psoc_obj(psoc);
	if (!mlme_obj) {
		mlme_err("Failed to get MLME Obj");
		return QDF_STATUS_E_FAILURE;
	}

	*value = mlme_obj->cfg.vht_caps.vht_cap_info.basic_mcs_set;

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
wlan_mlme_cfg_set_vht_basic_mcs_set(struct wlan_objmgr_psoc *psoc,
				    uint32_t value)
{
	struct wlan_mlme_psoc_obj *mlme_obj;

	mlme_obj = mlme_get_psoc_obj(psoc);
	if (!mlme_obj) {
		mlme_err("Failed to get MLME Obj");
		return QDF_STATUS_E_FAILURE;
	}

	mlme_obj->cfg.vht_caps.vht_cap_info.basic_mcs_set = value;

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
wlan_mlme_get_vht_enable_tx_bf(struct wlan_objmgr_psoc *psoc, bool *value)
{
	struct wlan_mlme_psoc_obj *mlme_obj;

	mlme_obj = mlme_get_psoc_obj(psoc);
	if (!mlme_obj) {
		mlme_err("Failed to get MLME Obj");
		return QDF_STATUS_E_FAILURE;
	}

	*value = mlme_obj->cfg.vht_caps.vht_cap_info.enable_txbf_20mhz;

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
wlan_mlme_get_vht_enable_tx_su_beam(struct wlan_objmgr_psoc *psoc, bool *value)
{
	struct wlan_mlme_psoc_obj *mlme_obj;

	mlme_obj = mlme_get_psoc_obj(psoc);
	if (!mlme_obj) {
		mlme_err("Failed to get MLME Obj");
		return QDF_STATUS_E_FAILURE;
	}

	*value = mlme_obj->cfg.vht_caps.vht_cap_info.enable_tx_su;

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
wlan_mlme_get_vht_channel_width(struct wlan_objmgr_psoc *psoc, uint8_t *value)
{
	struct wlan_mlme_psoc_obj *mlme_obj;

	mlme_obj = mlme_get_psoc_obj(psoc);
	if (!mlme_obj) {
		mlme_err("Failed to get MLME Obj");
		return QDF_STATUS_E_FAILURE;
	}

	*value = mlme_obj->cfg.vht_caps.vht_cap_info.channel_width;

	return QDF_STATUS_SUCCESS;
}


QDF_STATUS
wlan_mlme_get_vht_rx_mcs_8_9(struct wlan_objmgr_psoc *psoc, uint8_t *value)
{
	struct wlan_mlme_psoc_obj *mlme_obj;

	mlme_obj = mlme_get_psoc_obj(psoc);
	if (!mlme_obj) {
		mlme_err("Failed to get MLME Obj");
		return QDF_STATUS_E_FAILURE;
	}

	*value = mlme_obj->cfg.vht_caps.vht_cap_info.rx_mcs;

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
wlan_mlme_get_vht_tx_mcs_8_9(struct wlan_objmgr_psoc *psoc, uint8_t *value)
{
	struct wlan_mlme_psoc_obj *mlme_obj;

	mlme_obj = mlme_get_psoc_obj(psoc);
	if (!mlme_obj) {
		mlme_err("Failed to get MLME Obj");
		return QDF_STATUS_E_FAILURE;
	}

	*value = mlme_obj->cfg.vht_caps.vht_cap_info.tx_mcs;

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
wlan_mlme_get_vht_rx_mcs_2x2(struct wlan_objmgr_psoc *psoc, uint8_t *value)
{
	struct wlan_mlme_psoc_obj *mlme_obj;

	mlme_obj = mlme_get_psoc_obj(psoc);
	if (!mlme_obj) {
		mlme_err("Failed to get MLME Obj");
		return QDF_STATUS_E_FAILURE;
	}

	*value = mlme_obj->cfg.vht_caps.vht_cap_info.rx_mcs2x2;

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
wlan_mlme_get_vht_tx_mcs_2x2(struct wlan_objmgr_psoc *psoc, uint8_t *value)
{
	struct wlan_mlme_psoc_obj *mlme_obj;

	mlme_obj = mlme_get_psoc_obj(psoc);
	if (!mlme_obj) {
		mlme_err("Failed to get MLME Obj");
		return QDF_STATUS_E_FAILURE;
	}

	*value = mlme_obj->cfg.vht_caps.vht_cap_info.tx_mcs2x2;

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
wlan_mlme_get_vht20_mcs9(struct wlan_objmgr_psoc *psoc, bool *value)
{
	struct wlan_mlme_psoc_obj *mlme_obj;

	mlme_obj = mlme_get_psoc_obj(psoc);
	if (!mlme_obj) {
		mlme_err("Failed to get MLME Obj");
		return QDF_STATUS_E_FAILURE;
	}

	*value = mlme_obj->cfg.vht_caps.vht_cap_info.enable_vht20_mcs9;

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
wlan_mlme_get_vht_enable2x2(struct wlan_objmgr_psoc *psoc, bool *value)
{
	struct wlan_mlme_psoc_obj *mlme_obj;

	mlme_obj = mlme_get_psoc_obj(psoc);
	if (!mlme_obj) {
		mlme_err("Failed to get MLME Obj");
		return QDF_STATUS_E_FAILURE;
	}

	*value = mlme_obj->cfg.vht_caps.vht_cap_info.enable2x2;

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
wlan_mlme_set_vht_enable2x2(struct wlan_objmgr_psoc *psoc, bool value)
{
	struct wlan_mlme_psoc_obj *mlme_obj;

	mlme_obj = mlme_get_psoc_obj(psoc);
	if (!mlme_obj) {
		mlme_err("Failed to get MLME Obj");
		return QDF_STATUS_E_FAILURE;
	}

	mlme_obj->cfg.vht_caps.vht_cap_info.enable2x2 = value;

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
wlan_mlme_get_vht_enable_paid(struct wlan_objmgr_psoc *psoc, bool *value)
{
	struct wlan_mlme_psoc_obj *mlme_obj;

	mlme_obj = mlme_get_psoc_obj(psoc);
	if (!mlme_obj) {
		mlme_err("Failed to get MLME Obj");
		return QDF_STATUS_E_FAILURE;
	}

	*value = mlme_obj->cfg.vht_caps.vht_cap_info.enable_paid;

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
wlan_mlme_get_vht_enable_gid(struct wlan_objmgr_psoc *psoc, bool *value)
{
	struct wlan_mlme_psoc_obj *mlme_obj;

	mlme_obj = mlme_get_psoc_obj(psoc);
	if (!mlme_obj) {
		mlme_err("Failed to get MLME Obj");
		return QDF_STATUS_E_FAILURE;
	}

	*value = mlme_obj->cfg.vht_caps.vht_cap_info.enable_gid;

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
wlan_mlme_get_vht_for_24ghz(struct wlan_objmgr_psoc *psoc, bool *value)
{
	struct wlan_mlme_psoc_obj *mlme_obj;

	mlme_obj = mlme_get_psoc_obj(psoc);
	if (!mlme_obj) {
		mlme_err("Failed to get MLME Obj");
		return QDF_STATUS_E_FAILURE;
	}

	*value = mlme_obj->cfg.vht_caps.vht_cap_info.b24ghz_band;

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
wlan_mlme_get_vendor_vht_for_24ghz(struct wlan_objmgr_psoc *psoc, bool *value)
{
	struct wlan_mlme_psoc_obj *mlme_obj;

	mlme_obj = mlme_get_psoc_obj(psoc);
	if (!mlme_obj) {
		mlme_err("Failed to get MLME Obj");
		return QDF_STATUS_E_FAILURE;
	}

	*value = mlme_obj->cfg.vht_caps.vht_cap_info.vendor_24ghz_band;

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS mlme_update_vht_cap(struct wlan_objmgr_psoc *psoc,
			       struct wma_tgt_vht_cap *cfg)
{
	struct wlan_mlme_psoc_obj *mlme_obj;
	struct mlme_vht_capabilities_info vht_cap_info;
	uint32_t value = 0;
	bool hw_rx_ldpc_enabled;

	mlme_obj = mlme_get_psoc_obj(psoc);
	if (!mlme_obj) {
		mlme_err("Failed to get MLME Obj");
		return QDF_STATUS_E_FAILURE;
	}

	vht_cap_info = mlme_obj->cfg.vht_caps.vht_cap_info;

	/*
	 * VHT max MPDU length:
	 * override if user configured value is too high
	 * that the target cannot support
	 */
	if (vht_cap_info.ampdu_len > cfg->vht_max_mpdu)
		vht_cap_info.ampdu_len = cfg->vht_max_mpdu;

	value = (CFG_VHT_BASIC_MCS_SET_STADEF & VHT_MCS_1x1) |
		vht_cap_info.basic_mcs_set;
	if (vht_cap_info.enable2x2)
		value = (value & VHT_MCS_2x2) | (vht_cap_info.rx_mcs2x2 << 2);
	vht_cap_info.basic_mcs_set = value;

	value = (CFG_VHT_RX_MCS_MAP_STADEF & VHT_MCS_1x1) | vht_cap_info.rx_mcs;
	if (vht_cap_info.enable2x2)
		value = (value & VHT_MCS_2x2) | (vht_cap_info.rx_mcs2x2 << 2);
	vht_cap_info.rx_mcs_map = value;

	value = (CFG_VHT_TX_MCS_MAP_STADEF & VHT_MCS_1x1) | vht_cap_info.tx_mcs;
	if (vht_cap_info.enable2x2)
		value = (value & VHT_MCS_2x2) | (vht_cap_info.tx_mcs2x2 << 2);
	vht_cap_info.tx_mcs_map = value;

	 /* Set HW RX LDPC capability */
	hw_rx_ldpc_enabled = !!cfg->vht_rx_ldpc;
	if (hw_rx_ldpc_enabled != vht_cap_info.ldpc_coding_cap)
		vht_cap_info.ldpc_coding_cap = hw_rx_ldpc_enabled;

	/* set the Guard interval 80MHz */
	if (vht_cap_info.short_gi_80mhz && !cfg->vht_short_gi_80)
		vht_cap_info.short_gi_80mhz = cfg->vht_short_gi_80;

	/* Set VHT TX STBC cap */
	if (vht_cap_info.tx_stbc && !cfg->vht_tx_stbc)
		vht_cap_info.tx_stbc = cfg->vht_tx_stbc;

	/* Set VHT RX STBC cap */
	if (vht_cap_info.rx_stbc && !cfg->vht_rx_stbc)
		vht_cap_info.rx_stbc = cfg->vht_rx_stbc;

	/* Set VHT SU Beamformer cap */
	if (vht_cap_info.su_bformer && !cfg->vht_su_bformer)
		vht_cap_info.su_bformer = cfg->vht_su_bformer;

	/* check and update SU BEAMFORMEE capabality */
	if (vht_cap_info.enable_txbf_20mhz && !cfg->vht_su_bformee)
		vht_cap_info.su_bformee = cfg->vht_su_bformee;

	/* Set VHT MU Beamformer cap */
	if (vht_cap_info.mu_bformer && !cfg->vht_mu_bformer)
		vht_cap_info.mu_bformer = cfg->vht_mu_bformer;

	/* Set VHT MU Beamformee cap */
	if (vht_cap_info.enable_mu_bformee && !cfg->vht_mu_bformee)
		vht_cap_info.enable_mu_bformee = cfg->vht_mu_bformee;

	/*
	 * VHT max AMPDU len exp:
	 * override if user configured value is too high
	 * that the target cannot support.
	 * Even though Rome publish ampdu_len=7, it can
	 * only support 4 because of some h/w bug.
	 */
	if (vht_cap_info.ampdu_len_exponent > cfg->vht_max_ampdu_len_exp)
		vht_cap_info.ampdu_len_exponent = cfg->vht_max_ampdu_len_exp;

	/* Set VHT TXOP PS CAP */
	if (vht_cap_info.txop_ps && !cfg->vht_txop_ps)
		vht_cap_info.txop_ps = cfg->vht_txop_ps;

	/* set the Guard interval 160MHz */
	if (vht_cap_info.short_gi_160mhz && !cfg->vht_short_gi_160)
		vht_cap_info.short_gi_160mhz = cfg->vht_short_gi_160;

	return QDF_STATUS_SUCCESS;

}

QDF_STATUS mlme_update_nss_vht_cap(struct wlan_objmgr_psoc *psoc)
{
	struct wlan_mlme_psoc_obj *mlme_obj;
	struct mlme_vht_capabilities_info vht_cap_info;
	uint32_t temp = 0;

	mlme_obj = mlme_get_psoc_obj(psoc);
	if (!mlme_obj) {
		mlme_err("Failed to get MLME Obj");
		return QDF_STATUS_E_FAILURE;
	}

	vht_cap_info = mlme_obj->cfg.vht_caps.vht_cap_info;

	temp = vht_cap_info.basic_mcs_set;
	temp = (temp & 0xFFFC) | vht_cap_info.rx_mcs;
	if (vht_cap_info.enable2x2)
		temp = (temp & 0xFFF3) | (vht_cap_info.rx_mcs2x2 << 2);
	else
		temp |= 0x000C;

	vht_cap_info.basic_mcs_set = temp;

	temp = vht_cap_info.rx_mcs_map;
	temp = (temp & 0xFFFC) | vht_cap_info.rx_mcs;
	if (vht_cap_info.enable2x2)
		temp = (temp & 0xFFF3) | (vht_cap_info.rx_mcs2x2 << 2);
	else
		temp |= 0x000C;

	vht_cap_info.rx_mcs_map = temp;

	temp = vht_cap_info.tx_mcs_map;
	temp = (temp & 0xFFFC) | vht_cap_info.tx_mcs;
	if (vht_cap_info.enable2x2)
		temp = (temp & 0xFFF3) | (vht_cap_info.tx_mcs2x2 << 2);
	else
		temp |= 0x000C;

	vht_cap_info.tx_mcs_map = temp;

	return QDF_STATUS_SUCCESS;
}
