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
#include <wmi.h>
#include <osdep.h>
#include <reg_services_public_struct.h>
#include <wlan_reg_tgt_api.h>
#include <target_if.h>
#include <target_if_reg.h>

static struct cur_reg_rule
*create_reg_rules_from_wmi(uint32_t num_reg_rules,
			   wmi_regulatory_rule_struct *wmi_reg_rule)
{
	struct cur_reg_rule *reg_rule_ptr;
	uint32_t count;

	reg_rule_ptr = qdf_mem_malloc(num_reg_rules * sizeof(*reg_rule_ptr));

	if (NULL == reg_rule_ptr) {
		target_if_err("memory allocation failure");
		return NULL;
	}

	for (count = 0; count < num_reg_rules; count++) {
		reg_rule_ptr[count].start_freq =
			WMI_REG_RULE_START_FREQ_GET(
				wmi_reg_rule[count].freq_info);
		reg_rule_ptr[count].end_freq =
			WMI_REG_RULE_END_FREQ_GET(
				wmi_reg_rule[count].freq_info);
		reg_rule_ptr[count].max_bw =
			WMI_REG_RULE_MAX_BW_GET(
				wmi_reg_rule[count].bw_info);
		reg_rule_ptr[count].reg_power =
			WMI_REG_RULE_REG_POWER_GET(
				wmi_reg_rule[count].bw_info);
		reg_rule_ptr[count].flags =
			WMI_REG_RULE_FLAGS_GET(
				wmi_reg_rule[count].power_flag_info);
	}

	return reg_rule_ptr;
}


/**
 * reg_chan_list_update_handler() - function to update channel list
 * @handle: wma handle
 * @event_buf: event buffer
 * @len: length of buffer
 *
 * Return: 0 for success or error code
 */
static int reg_chan_list_update_handler(ol_scn_t handle, uint8_t *event_buf,
				 uint32_t len)
{
	WMI_REG_CHAN_LIST_CC_EVENTID_param_tlvs *param_buf;
	struct cur_regulatory_info *reg_info;
	wmi_reg_chan_list_cc_event_fixed_param *chan_list_event_hdr;
	wmi_regulatory_rule_struct *wmi_reg_rule;
	uint32_t num_2g_reg_rules, num_5g_reg_rules;
	struct wlan_objmgr_psoc *psoc;
	QDF_STATUS status;

	target_if_info("processing regulatory channel list");

	param_buf = (WMI_REG_CHAN_LIST_CC_EVENTID_param_tlvs *)event_buf;
	if (!param_buf) {
		target_if_err("invalid channel list event buf");
		return -EINVAL;
	}

	chan_list_event_hdr = param_buf->fixed_param;

	reg_info = qdf_mem_malloc(sizeof(*reg_info));

	if (NULL == reg_info) {
		target_if_err("memory allocation failure");
		return -ENOMEM;
	}

	psoc = target_if_get_psoc_from_scn_hdl(handle);

	reg_info->psoc = psoc;
	reg_info->num_2g_reg_rules = chan_list_event_hdr->num_2g_reg_rules;
	reg_info->num_5g_reg_rules = chan_list_event_hdr->num_5g_reg_rules;
	qdf_mem_copy(reg_info->alpha2, &(chan_list_event_hdr->alpha2),
		     REG_ALPHA2_LEN);
	reg_info->dfs_region = chan_list_event_hdr->dfs_region;
	reg_info->phybitmap = chan_list_event_hdr->phybitmap;
	reg_info->min_bw_2g = chan_list_event_hdr->min_bw_2g;
	reg_info->max_bw_2g = chan_list_event_hdr->max_bw_2g;
	reg_info->min_bw_5g = chan_list_event_hdr->min_bw_5g;
	reg_info->max_bw_5g = chan_list_event_hdr->max_bw_5g;

	num_2g_reg_rules = reg_info->num_2g_reg_rules;
	num_5g_reg_rules = reg_info->num_5g_reg_rules;

	wmi_reg_rule = (wmi_regulatory_rule_struct *)(chan_list_event_hdr
			    + sizeof(wmi_reg_chan_list_cc_event_fixed_param));

	reg_info->reg_rules_2g_ptr = create_reg_rules_from_wmi(num_2g_reg_rules,
							       wmi_reg_rule);
	wmi_reg_rule += num_2g_reg_rules;

	reg_info->reg_rules_5g_ptr = create_reg_rules_from_wmi(num_5g_reg_rules,
							       wmi_reg_rule);

	status = psoc->soc_cb.rx_ops.reg_rx_ops.master_list_handler(reg_info);

	if (status != QDF_STATUS_SUCCESS) {
		target_if_err("component could not process regulatory message");
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


static QDF_STATUS tgt_if_regulatory_register_master_list_handler(
	struct wlan_objmgr_psoc *psoc, void *arg)
{
	wmi_unified_t wmi_handle = GET_WMI_HDL_FROM_PSOC(psoc);

	return wmi_unified_register_event_handler(wmi_handle,
						  WMI_REG_CHAN_LIST_CC_EVENTID,
						  reg_chan_list_update_handler,
						  WMI_RX_UMAC_CTX);

}

static QDF_STATUS tgt_if_regulatory_unregister_master_list_handler(
	struct wlan_objmgr_psoc *psoc, void *arg)
{
	wmi_unified_t wmi_handle = GET_WMI_HDL_FROM_PSOC(psoc);

	return wmi_unified_unregister_event_handler(wmi_handle,
					    WMI_REG_CHAN_LIST_CC_EVENTID);

}

QDF_STATUS target_if_register_regulatory_tx_ops(struct wlan_lmac_if_tx_ops
						*tx_ops)
{
	struct wlan_lmac_if_reg_tx_ops *reg_ops = &tx_ops->reg_ops;

	reg_ops->register_master_handler =
		tgt_if_regulatory_register_master_list_handler;

	reg_ops->unregister_master_handler =
		tgt_if_regulatory_unregister_master_list_handler;

	return QDF_STATUS_SUCCESS;
}



