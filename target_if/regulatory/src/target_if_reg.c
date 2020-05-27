/*
 * Copyright (c) 2017-2020 The Linux Foundation. All rights reserved.
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
 * DOC: target_if_reg.c
 * This file contains regulatory target interfaces.
 */

#include <wmi_unified_api.h>
#include <reg_services_public_struct.h>
#include <wlan_reg_tgt_api.h>
#include <target_if.h>
#include <target_if_reg.h>
#include <wmi_unified_reg_api.h>
#include <qdf_platform.h>
#include <target_if_reg_11d.h>
#include <target_if_reg_lte.h>
#include <wlan_reg_ucfg_api.h>

/**
 * get_chan_list_cc_event_id() - Get chan_list_cc event i
 *
 * Return: Event id
 */
static inline uint32_t get_chan_list_cc_event_id(void)
{
	return wmi_reg_chan_list_cc_event_id;
}

/**
 * tgt_if_regulatory_is_regdb_offloaded() - Check if regdb is offloaded
 * @psoc: Pointer to psoc
 *
 * Return: true if regdb if offloaded, else false
 */
static bool tgt_if_regulatory_is_regdb_offloaded(struct wlan_objmgr_psoc *psoc)
{
	struct wlan_lmac_if_reg_rx_ops *reg_rx_ops;

	wmi_unified_t wmi_handle = get_wmi_unified_hdl_from_psoc(psoc);

	reg_rx_ops = target_if_regulatory_get_rx_ops(psoc);
	if (!reg_rx_ops) {
		target_if_err("reg_rx_ops is NULL");
		return false;
	}

	if (!wmi_handle)
		return false;

	if (reg_rx_ops->reg_ignore_fw_reg_offload_ind &&
	    reg_rx_ops->reg_ignore_fw_reg_offload_ind(psoc)) {
		target_if_debug("User disabled regulatory offload from ini");
		return 0;
	}

	return wmi_service_enabled(wmi_handle, wmi_service_regulatory_db);
}

/**
 * tgt_if_regulatory_is_6ghz_supported() - Check if 6ghz is supported
 * @psoc: Pointer to psoc
 *
 * Return: true if regdb if offloaded, else false
 */
static bool tgt_if_regulatory_is_6ghz_supported(struct wlan_objmgr_psoc *psoc)
{
	wmi_unified_t wmi_handle = get_wmi_unified_hdl_from_psoc(psoc);

	if (!wmi_handle)
		return false;

	return wmi_service_enabled(wmi_handle, wmi_service_6ghz_support);
}

/**
 * tgt_if_regulatory_is_5dot9_ghz_supported() - Check if 5.9ghz is supported
 * @psoc: Pointer to psoc
 *
 * Return: true if regdb if offloaded, else false
 */
static bool
tgt_if_regulatory_is_5dot9_ghz_supported(struct wlan_objmgr_psoc *psoc)
{
	wmi_unified_t wmi_handle = get_wmi_unified_hdl_from_psoc(psoc);

	if (!wmi_handle)
		return false;

	return wmi_service_enabled(wmi_handle, wmi_service_5dot9_ghz_support);
}

/**
 * tgt_if_regulatory_is_there_serv_ready_extn() - Check for service ready
 * extension
 * @psoc: Pointer to psoc object
 *
 * Return: true if service ready extension is present, else false.
 */
static bool tgt_if_regulatory_is_there_serv_ready_extn(
		struct wlan_objmgr_psoc *psoc)
{
	wmi_unified_t wmi_handle = get_wmi_unified_hdl_from_psoc(psoc);

	if (!wmi_handle)
		return false;

	return wmi_service_enabled(wmi_handle, wmi_service_ext_msg);
}

/**
 * target_if_regulatory_get_rx_ops() - Get regdb rx ops
 * @psoc: Pointer to psoc object
 *
 * Return: Reg rx_ops
 */
struct wlan_lmac_if_reg_rx_ops *
target_if_regulatory_get_rx_ops(struct wlan_objmgr_psoc *psoc)
{
	struct wlan_lmac_if_rx_ops *rx_ops;

	rx_ops = wlan_psoc_get_lmac_if_rxops(psoc);
	if (!rx_ops) {
		target_if_err("rx_ops is NULL");
		return NULL;
	}

	return &rx_ops->reg_rx_ops;
}

QDF_STATUS target_if_reg_set_offloaded_info(struct wlan_objmgr_psoc *psoc)
{
	struct wlan_lmac_if_reg_rx_ops *reg_rx_ops;

	reg_rx_ops = target_if_regulatory_get_rx_ops(psoc);
	if (!reg_rx_ops) {
		target_if_err("reg_rx_ops is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	if (reg_rx_ops->reg_set_regdb_offloaded)
		reg_rx_ops->reg_set_regdb_offloaded(
				psoc,
				tgt_if_regulatory_is_regdb_offloaded(psoc));

	if (reg_rx_ops->reg_set_11d_offloaded)
		reg_rx_ops->reg_set_11d_offloaded(
				psoc, tgt_if_regulatory_is_11d_offloaded(psoc));

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS target_if_reg_set_6ghz_info(struct wlan_objmgr_psoc *psoc)
{
	struct wlan_lmac_if_reg_rx_ops *reg_rx_ops;

	reg_rx_ops = target_if_regulatory_get_rx_ops(psoc);
	if (!reg_rx_ops) {
		target_if_err("reg_rx_ops is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	if (reg_rx_ops->reg_set_6ghz_supported)
		reg_rx_ops->reg_set_6ghz_supported(
			psoc,
			tgt_if_regulatory_is_6ghz_supported(psoc));

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS target_if_reg_set_5dot9_ghz_info(struct wlan_objmgr_psoc *psoc)
{
	struct wlan_lmac_if_reg_rx_ops *reg_rx_ops;

	reg_rx_ops = target_if_regulatory_get_rx_ops(psoc);
	if (!reg_rx_ops) {
		target_if_err("reg_rx_ops is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	if (reg_rx_ops->reg_set_5dot9_ghz_supported)
		reg_rx_ops->reg_set_5dot9_ghz_supported(
			psoc,
			tgt_if_regulatory_is_5dot9_ghz_supported(psoc));

	return QDF_STATUS_SUCCESS;
}

/**
 * tgt_reg_chan_list_update_handler() - Channel list update handler
 * @handle: scn handle
 * @event_buf: pointer to event buffer
 * @len: buffer length
 *
 * Return: 0 on success
 */
static int tgt_reg_chan_list_update_handler(ol_scn_t handle, uint8_t *event_buf,
					    uint32_t len)
{
	struct wlan_objmgr_psoc *psoc;
	struct wlan_lmac_if_reg_rx_ops *reg_rx_ops;
	struct cur_regulatory_info *reg_info;
	QDF_STATUS status;
	struct wmi_unified *wmi_handle;
	int ret_val = 0;

	TARGET_IF_ENTER();

	psoc = target_if_get_psoc_from_scn_hdl(handle);
	if (!psoc) {
		target_if_err("psoc ptr is NULL");
		return -EINVAL;
	}

	reg_rx_ops = target_if_regulatory_get_rx_ops(psoc);
	if (!reg_rx_ops) {
		target_if_err("reg_rx_ops is NULL");
		return -EINVAL;
	}

	if (!reg_rx_ops->master_list_handler) {
		target_if_err("master_list_handler is NULL");
		return -EINVAL;
	}

	wmi_handle = get_wmi_unified_hdl_from_psoc(psoc);
	if (!wmi_handle) {
		target_if_err("invalid wmi handle");
		return -EINVAL;
	}

	reg_info = qdf_mem_malloc(sizeof(*reg_info));
	if (!reg_info)
		return -ENOMEM;

	if (wmi_extract_reg_chan_list_update_event(wmi_handle,
						   event_buf, reg_info, len)
	    != QDF_STATUS_SUCCESS) {
		target_if_err("Extraction of channel list event failed");
		ret_val = -EFAULT;
		goto clean;
	}

	if (reg_info->phy_id >= PSOC_MAX_PHY_REG_CAP) {
		target_if_err_rl("phy_id %d is out of bounds",
				 reg_info->phy_id);
		ret_val = -EFAULT;
		goto clean;
	}

	reg_info->psoc = psoc;

	status = reg_rx_ops->master_list_handler(reg_info);
	if (status != QDF_STATUS_SUCCESS) {
		target_if_err("Failed to process master channel list handler");
		ret_val = -EFAULT;
	}

clean:
	qdf_mem_free(reg_info->reg_rules_2g_ptr);
	qdf_mem_free(reg_info->reg_rules_5g_ptr);
	qdf_mem_free(reg_info);

	TARGET_IF_EXIT();

	return ret_val;
}

/**
 * tgt_if_regulatory_register_master_list_handler() - Register master channel
 * list
 * @psoc: Pointer to psoc
 * @arg: Pointer to argument list
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS tgt_if_regulatory_register_master_list_handler(
	struct wlan_objmgr_psoc *psoc, void *arg)
{
	wmi_unified_t wmi_handle = get_wmi_unified_hdl_from_psoc(psoc);

	if (!wmi_handle)
		return QDF_STATUS_E_FAILURE;

	return wmi_unified_register_event_handler(
			wmi_handle, wmi_reg_chan_list_cc_event_id,
			tgt_reg_chan_list_update_handler, WMI_RX_WORK_CTX);
}

/**
 * tgt_if_regulatory_unregister_master_list_handler() - Unregister master
 * channel list
 * @psoc: Pointer to psoc
 * @arg: Pointer to argument list
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS tgt_if_regulatory_unregister_master_list_handler(
	struct wlan_objmgr_psoc *psoc, void *arg)
{
	wmi_unified_t wmi_handle = get_wmi_unified_hdl_from_psoc(psoc);

	if (!wmi_handle)
		return QDF_STATUS_E_FAILURE;

	return wmi_unified_unregister_event_handler(
			wmi_handle, wmi_reg_chan_list_cc_event_id);
}

/**
 * tgt_if_regulatory_set_country_code() - Set country code
 * @psoc: Pointer to psoc
 * @arg: Pointer to argument list
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS tgt_if_regulatory_set_country_code(
	struct wlan_objmgr_psoc *psoc, void *arg)
{
	wmi_unified_t wmi_handle = get_wmi_unified_hdl_from_psoc(psoc);

	if (!wmi_handle)
		return QDF_STATUS_E_FAILURE;

	return wmi_unified_set_country_cmd_send(wmi_handle, arg);
}

/**
 * tgt_if_regulatory_set_user_country_code() - Set user country code
 * @psoc: Pointer to psoc
 * @pdev_id: Pdev id
 * @rd: Pointer to regdomain structure
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS tgt_if_regulatory_set_user_country_code(
	struct wlan_objmgr_psoc *psoc, uint8_t pdev_id, struct cc_regdmn_s *rd)
{
	wmi_unified_t wmi_handle = get_wmi_unified_hdl_from_psoc(psoc);

	if (!wmi_handle)
		return QDF_STATUS_E_FAILURE;

	if (wmi_unified_set_user_country_code_cmd_send(
				wmi_handle, pdev_id, rd) != QDF_STATUS_SUCCESS
			) {
		target_if_err("Set user country code failed");
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS tgt_if_regulatory_modify_freq_range(struct wlan_objmgr_psoc *psoc)
{
	struct wlan_psoc_host_hal_reg_capabilities_ext *reg_cap;

	reg_cap = ucfg_reg_get_hal_reg_cap(psoc);
	if (!reg_cap) {
		target_if_err("reg cap is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	if (!(reg_cap->wireless_modes & WMI_HOST_REGDMN_MODE_11A)) {
		reg_cap->low_5ghz_chan = 0;
		reg_cap->high_5ghz_chan = 0;
	}

	if (!(reg_cap->wireless_modes &
	     (WMI_HOST_REGDMN_MODE_11B | WMI_HOST_REGDMN_MODE_PUREG))) {
		reg_cap->low_2ghz_chan = 0;
		reg_cap->high_2ghz_chan = 0;
	}

	target_if_debug("phy_id = %d - low_2ghz_chan = %d high_2ghz_chan = %d low_5ghz_chan = %d high_5ghz_chan = %d",
			reg_cap->phy_id,
			reg_cap->low_2ghz_chan,
			reg_cap->high_2ghz_chan,
			reg_cap->low_5ghz_chan,
			reg_cap->high_5ghz_chan);

	return QDF_STATUS_SUCCESS;
}

#ifdef CONFIG_REG_CLIENT
/**
 * tgt_if_regulatory_send_ctl_info() - Send CTL info to firmware
 * @psoc: Pointer to psoc
 * @params: Pointer to reg control params
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS
tgt_if_regulatory_send_ctl_info(struct wlan_objmgr_psoc *psoc,
				struct reg_ctl_params *params)
{
	wmi_unified_t wmi_handle = get_wmi_unified_hdl_from_psoc(psoc);

	if (!wmi_handle)
		return QDF_STATUS_E_FAILURE;

	return wmi_unified_send_regdomain_info_to_fw_cmd(wmi_handle,
							 params->regd,
							 params->regd_2g,
							 params->regd_5g,
							 params->ctl_2g,
							 params->ctl_5g);
}
#else
static QDF_STATUS
tgt_if_regulatory_send_ctl_info(struct wlan_objmgr_psoc *psoc,
				struct reg_ctl_params *params)
{
	return QDF_STATUS_SUCCESS;
}
#endif

/**
 * tgt_if_regulatory_get_phy_id_from_pdev_id() - Get phy_id from pdev_id
 * @psoc: Pointer to psoc
 * @pdev_id: Pdev id
 * @phy_id: phy_id
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS tgt_if_regulatory_get_phy_id_from_pdev_id(
	struct wlan_objmgr_psoc *psoc, uint8_t pdev_id, uint8_t *phy_id)
{
	struct target_psoc_info *tgt_if_handle = psoc->tgt_if_handle;
	uint8_t ret;

	if (pdev_id >= WLAN_UMAC_MAX_PDEVS) {
		target_if_err("pdev_id is greater than WLAN_UMAC_MAX_PDEVS");
		return QDF_STATUS_E_FAILURE;
	}

	/* By default pdev_id and phy_id have one to one mapping */
	*phy_id = pdev_id;

	if (!(tgt_if_handle &&
	      tgt_if_handle->info.is_pdevid_to_phyid_map))
		return QDF_STATUS_SUCCESS;

	ret = tgt_if_handle->info.pdev_id_to_phy_id_map[pdev_id];

	if (ret < PSOC_MAX_PHY_REG_CAP) {
		*phy_id = ret;
	} else {
		target_if_err("phy_id is greater than PSOC_MAX_PHY_REG_CAP");
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

/**
 * tgt_if_regulatory_get_pdev_id_from_phy_id() - Get pdev_id for phy_id
 * @psoc: Pointer to psoc
 * @phy_id: Phy id
 * @pdev_id: Pdev id
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS tgt_if_regulatory_get_pdev_id_from_phy_id(
	struct wlan_objmgr_psoc *psoc, uint8_t phy_id, uint8_t *pdev_id)
{
	struct target_psoc_info *tgt_if_handle = psoc->tgt_if_handle;
	uint8_t i;

	if (phy_id >= PSOC_MAX_PHY_REG_CAP) {
		target_if_err("phy_id is greater than PSOC_MAX_PHY_REG_CAP");
		return QDF_STATUS_E_FAILURE;
	}

	/* By default pdev_id and phy_id have one to one mapping */
	*pdev_id = phy_id;

	if (!(tgt_if_handle &&
	      tgt_if_handle->info.is_pdevid_to_phyid_map))
		return QDF_STATUS_SUCCESS;

	for (i = 0; i < WLAN_UMAC_MAX_PDEVS; i++) {
		if (tgt_if_handle->info.pdev_id_to_phy_id_map[i] == phy_id)
			break;
	}

	if (i < WLAN_UMAC_MAX_PDEVS) {
		*pdev_id = i;
	} else {
		target_if_err("pdev_id is greater than WLAN_UMAC_MAX_PDEVS");
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS target_if_register_regulatory_tx_ops(
		struct wlan_lmac_if_tx_ops *tx_ops)
{
	struct wlan_lmac_if_reg_tx_ops *reg_ops = &tx_ops->reg_ops;

	reg_ops->register_master_handler =
		tgt_if_regulatory_register_master_list_handler;

	reg_ops->unregister_master_handler =
		tgt_if_regulatory_unregister_master_list_handler;

	reg_ops->set_country_code = tgt_if_regulatory_set_country_code;

	reg_ops->fill_umac_legacy_chanlist = NULL;

	reg_ops->set_country_failed = NULL;

	reg_ops->register_11d_new_cc_handler =
		tgt_if_regulatory_register_11d_new_cc_handler;

	reg_ops->unregister_11d_new_cc_handler =
		tgt_if_regulatory_unregister_11d_new_cc_handler;

	reg_ops->start_11d_scan = tgt_if_regulatory_start_11d_scan;

	reg_ops->stop_11d_scan = tgt_if_regulatory_stop_11d_scan;

	reg_ops->is_there_serv_ready_extn =
		tgt_if_regulatory_is_there_serv_ready_extn;

	reg_ops->set_user_country_code =
		tgt_if_regulatory_set_user_country_code;

	reg_ops->register_ch_avoid_event_handler =
		tgt_if_regulatory_register_ch_avoid_event_handler;

	reg_ops->unregister_ch_avoid_event_handler =
		tgt_if_regulatory_unregister_ch_avoid_event_handler;

	reg_ops->send_ctl_info = tgt_if_regulatory_send_ctl_info;

	reg_ops->get_phy_id_from_pdev_id =
			tgt_if_regulatory_get_phy_id_from_pdev_id;

	reg_ops->get_pdev_id_from_phy_id =
			tgt_if_regulatory_get_pdev_id_from_phy_id;

	return QDF_STATUS_SUCCESS;
}
