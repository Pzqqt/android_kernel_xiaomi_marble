/*
 * Copyright (c) 2016-2018, 2020 The Linux Foundation. All rights reserved.
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

#include "wmi_unified_action_oui_tlv.h"
#include "wmi_unified_priv.h"

bool wmi_get_action_oui_id(enum action_oui_id action_id,
			   wmi_vendor_oui_action_id *id)
{
	switch (action_id) {

	case ACTION_OUI_CONNECT_1X1:
		*id = WMI_VENDOR_OUI_ACTION_CONNECTION_1X1;
		return true;

	case ACTION_OUI_ITO_EXTENSION:
		*id = WMI_VENDOR_OUI_ACTION_ITO_EXTENSION;
		return true;

	case ACTION_OUI_CCKM_1X1:
		*id = WMI_VENDOR_OUI_ACTION_CCKM_1X1;
		return true;

	case ACTION_OUI_ITO_ALTERNATE:
		*id = WMI_VENDOR_OUI_ACTION_ALT_ITO;
		return true;

	case ACTION_OUI_SWITCH_TO_11N_MODE:
		*id = WMI_VENDOR_OUI_ACTION_SWITCH_TO_11N_MODE;
		return true;

	case ACTION_OUI_CONNECT_1X1_WITH_1_CHAIN:
		*id = WMI_VENDOR_OUI_ACTION_CONNECTION_1X1_NUM_TX_RX_CHAINS_1;
		return true;

	case ACTION_OUI_DISABLE_AGGRESSIVE_TX:
		*id = WMI_VENDOR_OUI_ACTION_DISABLE_AGGRESSIVE_TX;
		return true;

	case ACTION_OUI_DISABLE_TWT:
		*id = WMI_VENDOR_OUI_ACTION_DISABLE_FW_TRIGGERED_TWT;
		return true;

	default:
		return false;
	}
}

uint32_t wmi_get_action_oui_info_mask(uint32_t info_mask)
{
	uint32_t info_presence = 0;

	if (info_mask & ACTION_OUI_INFO_OUI)
		info_presence |= WMI_BEACON_INFO_PRESENCE_OUI_EXT;

	if (info_mask & ACTION_OUI_INFO_MAC_ADDRESS)
		info_presence |= WMI_BEACON_INFO_PRESENCE_MAC_ADDRESS;

	if (info_mask & ACTION_OUI_INFO_AP_CAPABILITY_NSS)
		info_presence |= WMI_BEACON_INFO_PRESENCE_AP_CAPABILITY_NSS;

	if (info_mask & ACTION_OUI_INFO_AP_CAPABILITY_HT)
		info_presence |= WMI_BEACON_INFO_PRESENCE_AP_CAPABILITY_HT;

	if (info_mask & ACTION_OUI_INFO_AP_CAPABILITY_VHT)
		info_presence |= WMI_BEACON_INFO_PRESENCE_AP_CAPABILITY_VHT;

	if (info_mask & ACTION_OUI_INFO_AP_CAPABILITY_BAND)
		info_presence |= WMI_BEACON_INFO_PRESENCE_AP_CAPABILITY_BAND;

	return info_presence;
}

void wmi_fill_oui_extensions(struct action_oui_extension *extension,
			     uint32_t no_oui_extns,
			     wmi_vendor_oui_ext *cmd_ext)
{
	uint32_t i;
	uint32_t buffer_length;

	for (i = 0; i < no_oui_extns; i++) {
		WMITLV_SET_HDR(&cmd_ext->tlv_header,
			       WMITLV_TAG_STRUC_wmi_vendor_oui_ext,
			       WMITLV_GET_STRUCT_TLVLEN(wmi_vendor_oui_ext));
		cmd_ext->info_presence_bit_mask =
			wmi_get_action_oui_info_mask(extension->info_mask);

		cmd_ext->oui_header_length = extension->oui_length;
		cmd_ext->oui_data_length = extension->data_length;
		cmd_ext->mac_address_length = extension->mac_addr_length;
		cmd_ext->capability_data_length =
					extension->capability_length;

		buffer_length = extension->oui_length +
				extension->data_length +
				extension->data_mask_length +
				extension->mac_addr_length +
				extension->mac_mask_length +
				extension->capability_length;

		cmd_ext->buf_data_length = buffer_length + 1;

		cmd_ext++;
		extension++;
	}

}

QDF_STATUS
wmi_fill_oui_extensions_buffer(struct action_oui_extension *extension,
			       wmi_vendor_oui_ext *cmd_ext,
			       uint32_t no_oui_extns, uint32_t rem_var_buf_len,
			       uint8_t *var_buf)
{
	uint8_t i;

	for (i = 0; i < (uint8_t)no_oui_extns; i++) {
		if ((rem_var_buf_len - cmd_ext->buf_data_length) < 0) {
			wmi_err("Invalid action oui command length");
			return QDF_STATUS_E_INVAL;
		}

		var_buf[0] = i;
		var_buf++;

		if (extension->oui_length) {
			qdf_mem_copy(var_buf, extension->oui,
				     extension->oui_length);
			var_buf += extension->oui_length;
		}

		if (extension->data_length) {
			qdf_mem_copy(var_buf, extension->data,
				     extension->data_length);
			var_buf += extension->data_length;
		}

		if (extension->data_mask_length) {
			qdf_mem_copy(var_buf, extension->data_mask,
				     extension->data_mask_length);
			var_buf += extension->data_mask_length;
		}

		if (extension->mac_addr_length) {
			qdf_mem_copy(var_buf, extension->mac_addr,
				     extension->mac_addr_length);
			var_buf += extension->mac_addr_length;
		}

		if (extension->mac_mask_length) {
			qdf_mem_copy(var_buf, extension->mac_mask,
				     extension->mac_mask_length);
			var_buf += extension->mac_mask_length;
		}

		if (extension->capability_length) {
			qdf_mem_copy(var_buf, extension->capability,
				     extension->capability_length);
			var_buf += extension->capability_length;
		}

		rem_var_buf_len -= cmd_ext->buf_data_length;
		cmd_ext++;
		extension++;
	}

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
send_action_oui_cmd_tlv(wmi_unified_t wmi_handle,
			struct action_oui_request *req)
{
	wmi_pdev_config_vendor_oui_action_fixed_param *cmd;
	wmi_vendor_oui_ext *cmd_ext;
	wmi_buf_t wmi_buf;
	struct action_oui_extension *extension;
	uint32_t len;
	uint32_t i;
	uint8_t *buf_ptr;
	uint32_t no_oui_extns;
	uint32_t total_no_oui_extns;
	uint32_t var_buf_len = 0;
	wmi_vendor_oui_action_id action_id;
	bool valid;
	uint32_t rem_var_buf_len;
	QDF_STATUS status;

	if (!req) {
		wmi_err("action oui is empty");
		return QDF_STATUS_E_INVAL;
	}

	no_oui_extns = req->no_oui_extensions;
	total_no_oui_extns = req->total_no_oui_extensions;

	len = sizeof(*cmd);
	len += WMI_TLV_HDR_SIZE; /* Array of wmi_vendor_oui_ext structures */

	if (!no_oui_extns ||
	    no_oui_extns > WMI_MAX_VENDOR_OUI_ACTION_SUPPORTED_PER_ACTION ||
	    (total_no_oui_extns > WMI_VENDOR_OUI_ACTION_MAX_ACTION_ID *
	     WMI_MAX_VENDOR_OUI_ACTION_SUPPORTED_PER_ACTION)) {
		wmi_err("Invalid number of action oui extensions");
		return QDF_STATUS_E_INVAL;
	}

	valid = wmi_get_action_oui_id(req->action_id, &action_id);
	if (!valid) {
		wmi_err("Invalid action id");
		return QDF_STATUS_E_INVAL;
	}

	len += no_oui_extns * sizeof(*cmd_ext);
	len += WMI_TLV_HDR_SIZE; /* Variable length buffer */

	extension = req->extension;
	for (i = 0; i < no_oui_extns; i++) {
		var_buf_len += extension->oui_length +
		       extension->data_length +
		       extension->data_mask_length +
		       extension->mac_addr_length +
		       extension->mac_mask_length +
		       extension->capability_length;
		extension++;
	}

	var_buf_len += no_oui_extns; /* to store indexes */
	rem_var_buf_len = var_buf_len;
	var_buf_len = (var_buf_len + 3) & ~0x3;
	len += var_buf_len;

	wmi_buf = wmi_buf_alloc(wmi_handle, len);
	if (!wmi_buf) {
		wmi_err("Failed to allocate wmi buffer");
		return QDF_STATUS_E_FAILURE;
	}

	buf_ptr = (uint8_t *)wmi_buf_data(wmi_buf);
	cmd = (wmi_pdev_config_vendor_oui_action_fixed_param *)buf_ptr;

	WMITLV_SET_HDR(&cmd->tlv_header,
		WMITLV_TAG_STRUC_wmi_pdev_config_vendor_oui_action_fixed_param,
		WMITLV_GET_STRUCT_TLVLEN(
			wmi_pdev_config_vendor_oui_action_fixed_param));

	cmd->action_id = action_id;
	cmd->total_num_vendor_oui = total_no_oui_extns;
	cmd->num_vendor_oui_ext = no_oui_extns;

	buf_ptr += sizeof(*cmd);
	WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_STRUC,
		       no_oui_extns * sizeof(*cmd_ext));
	buf_ptr += WMI_TLV_HDR_SIZE;
	cmd_ext = (wmi_vendor_oui_ext *)buf_ptr;
	wmi_fill_oui_extensions(req->extension, no_oui_extns, cmd_ext);

	buf_ptr += no_oui_extns * sizeof(*cmd_ext);
	WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_BYTE, var_buf_len);
	buf_ptr += WMI_TLV_HDR_SIZE;
	status = wmi_fill_oui_extensions_buffer(req->extension,
						cmd_ext, no_oui_extns,
						rem_var_buf_len, buf_ptr);
	if (!QDF_IS_STATUS_SUCCESS(status)) {
		wmi_buf_free(wmi_buf);
		wmi_buf = NULL;
		return QDF_STATUS_E_INVAL;
	}

	buf_ptr += var_buf_len;

	if (wmi_unified_cmd_send(wmi_handle, wmi_buf, len,
				 WMI_PDEV_CONFIG_VENDOR_OUI_ACTION_CMDID)) {
		wmi_err("WMI_PDEV_CONFIG_VENDOR_OUI_ACTION send fail");
		wmi_buf_free(wmi_buf);
		wmi_buf = NULL;
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}
