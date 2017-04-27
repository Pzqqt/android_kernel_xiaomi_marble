/*
 * Copyright (c) 2017 The Linux Foundation. All rights reserved.
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
 * This file contains regulatory target interface
 */


#include <wmi_unified_api.h>
#include <reg_services_public_struct.h>
#include <wlan_reg_tgt_api.h>
#include <target_if.h>
#include <target_if_reg.h>
#include <wmi_unified_reg_api.h>

#ifdef CONFIG_MCL
static inline uint32_t get_chan_list_cc_event_id(void)
{
	return WMI_REG_CHAN_LIST_CC_EVENTID;
}
#else
static inline uint32_t get_chan_list_cc_event_id(void)
{
	return wmi_reg_chan_list_cc_event_id;
}
#endif

static inline struct wlan_lmac_if_reg_rx_ops *
target_if_regulatory_get_rx_ops(struct wlan_objmgr_psoc *psoc)
{
	return &psoc->soc_cb.rx_ops.reg_rx_ops;
}

static int tgt_reg_chan_list_update_handler(ol_scn_t handle,
					    uint8_t *event_buf,
					    uint32_t len)
{
	struct wlan_objmgr_psoc *psoc;
	struct wlan_lmac_if_reg_rx_ops *reg_rx_ops;
	struct cur_regulatory_info *reg_info;
	QDF_STATUS status;

	TARGET_IF_ENTER();

	psoc = target_if_get_psoc_from_scn_hdl(handle);
	if (!psoc) {
		target_if_err("psoc ptr is NULL");
		return -EINVAL;
	}

	reg_rx_ops = target_if_regulatory_get_rx_ops(psoc);
	if (!reg_rx_ops->master_list_handler) {
		target_if_err("master_list_handler is NULL");
		return -EINVAL;
	}

	reg_info = qdf_mem_malloc(sizeof(*reg_info));
	if (!reg_info) {
		target_if_err("memory allocation failed");
		return -ENOMEM;
	}

	if (wmi_extract_reg_chan_list_update_event(GET_WMI_HDL_FROM_PSOC(psoc),
						   event_buf, reg_info, len)
	    != QDF_STATUS_SUCCESS) {

		target_if_err("Extraction of channel list event failed");
		qdf_mem_free(reg_info->reg_rules_2g_ptr);
		qdf_mem_free(reg_info->reg_rules_5g_ptr);
		qdf_mem_free(reg_info);
		return -EFAULT;
	}

	reg_info->psoc = psoc;

	status = reg_rx_ops->master_list_handler(reg_info);
	if (status != QDF_STATUS_SUCCESS) {
		target_if_err("Failed to process master channel list handler");
		qdf_mem_free(reg_info->reg_rules_2g_ptr);
		qdf_mem_free(reg_info->reg_rules_5g_ptr);
		qdf_mem_free(reg_info);
		return -EFAULT;
	}

	qdf_mem_free(reg_info->reg_rules_2g_ptr);
	qdf_mem_free(reg_info->reg_rules_5g_ptr);
	qdf_mem_free(reg_info);

	target_if_debug("processed regulatory channel list");

	return 0;
}

static int tgt_reg_11d_new_cc_handler(ol_scn_t handle,
		uint8_t *event_buf, uint32_t len)
{
	struct wlan_objmgr_psoc *psoc;
	struct wlan_lmac_if_reg_rx_ops *reg_rx_ops;
	struct reg_11d_new_country reg_11d_new_cc;
	QDF_STATUS status;

	TARGET_IF_ENTER();

	psoc = target_if_get_psoc_from_scn_hdl(handle);
	if (!psoc) {
		target_if_err("psoc ptr is NULL");
		return -EINVAL;
	}

	reg_rx_ops = target_if_regulatory_get_rx_ops(psoc);

	if (!reg_rx_ops->reg_11d_new_cc_handler) {
		target_if_err("reg_11d_new_cc_handler is NULL");
		return -EINVAL;
	}

	if (wmi_extract_reg_11d_new_cc_event(GET_WMI_HDL_FROM_PSOC(psoc),
				event_buf, &reg_11d_new_cc, len) !=
			QDF_STATUS_SUCCESS) {

		target_if_err("Extraction of new country event failed");
		return -EFAULT;
	}

	status = reg_rx_ops->reg_11d_new_cc_handler(psoc, &reg_11d_new_cc);
	if (status != QDF_STATUS_SUCCESS) {
		target_if_err("Failed to process new country code event");
		return -EFAULT;
	}

	target_if_debug("processed 11d new country code event");

	return 0;
}

static QDF_STATUS tgt_if_regulatory_register_master_list_handler(
	struct wlan_objmgr_psoc *psoc, void *arg)
{
	wmi_unified_t wmi_handle = GET_WMI_HDL_FROM_PSOC(psoc);

	return wmi_unified_register_event_handler(wmi_handle,
					       get_chan_list_cc_event_id(),
					       tgt_reg_chan_list_update_handler,
					       WMI_RX_UMAC_CTX);

}

static QDF_STATUS tgt_if_regulatory_unregister_master_list_handler(
	struct wlan_objmgr_psoc *psoc, void *arg)
{
	wmi_unified_t wmi_handle = GET_WMI_HDL_FROM_PSOC(psoc);

	return wmi_unified_unregister_event_handler(wmi_handle,
					       get_chan_list_cc_event_id());
}

static QDF_STATUS tgt_if_regulatory_set_country_code(
	struct wlan_objmgr_psoc *psoc, void *arg)
{
	wmi_unified_t wmi_handle = GET_WMI_HDL_FROM_PSOC(psoc);

	return wmi_unified_set_country_cmd_send(wmi_handle, arg);

}

static QDF_STATUS tgt_if_regulatory_register_11d_new_cc_handler(
	struct wlan_objmgr_psoc *psoc, void *arg)
{
	wmi_unified_t wmi_handle = GET_WMI_HDL_FROM_PSOC(psoc);

	return wmi_unified_register_event_handler(wmi_handle,
						  WMI_11D_NEW_COUNTRY_EVENTID,
						  tgt_reg_11d_new_cc_handler,
						  WMI_RX_UMAC_CTX);
}

static QDF_STATUS tgt_if_regulatory_unregister_11d_new_cc_handler(
	struct wlan_objmgr_psoc *psoc, void *arg)
{
	wmi_unified_t wmi_handle = GET_WMI_HDL_FROM_PSOC(psoc);

	return wmi_unified_unregister_event_handler(wmi_handle,
			WMI_11D_NEW_COUNTRY_EVENTID);
}

static QDF_STATUS tgt_if_regulatory_start_11d_scan(
		struct wlan_objmgr_psoc *psoc,
		struct reg_start_11d_scan_req *reg_start_11d_scan_req)
{
	wmi_unified_t wmi_handle = GET_WMI_HDL_FROM_PSOC(psoc);

	return wmi_unified_send_start_11d_scan_cmd(wmi_handle,
			reg_start_11d_scan_req);
}

static QDF_STATUS tgt_if_regulatory_stop_11d_scan(
		struct wlan_objmgr_psoc *psoc,
		struct reg_stop_11d_scan_req *reg_stop_11d_scan_req)
{
	wmi_unified_t wmi_handle = GET_WMI_HDL_FROM_PSOC(psoc);

	return wmi_unified_send_stop_11d_scan_cmd(wmi_handle,
			reg_stop_11d_scan_req);
}

QDF_STATUS target_if_register_regulatory_tx_ops(struct wlan_lmac_if_tx_ops
						*tx_ops)
{
	struct wlan_lmac_if_reg_tx_ops *reg_ops = &tx_ops->reg_ops;

	reg_ops->register_master_handler =
		tgt_if_regulatory_register_master_list_handler;

	reg_ops->unregister_master_handler =
		tgt_if_regulatory_unregister_master_list_handler;

	reg_ops->set_country_code = tgt_if_regulatory_set_country_code;

	reg_ops->fill_umac_legacy_chanlist = NULL;

	reg_ops->register_11d_new_cc_handler =
		tgt_if_regulatory_register_11d_new_cc_handler;

	reg_ops->unregister_11d_new_cc_handler =
		tgt_if_regulatory_unregister_11d_new_cc_handler;

	reg_ops->start_11d_scan = tgt_if_regulatory_start_11d_scan;

	reg_ops->stop_11d_scan = tgt_if_regulatory_stop_11d_scan;

	return QDF_STATUS_SUCCESS;
}

