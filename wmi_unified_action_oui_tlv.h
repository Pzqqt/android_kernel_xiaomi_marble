/*
 * Copyright (c) 2016-2018 The Linux Foundation. All rights reserved.
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

#ifndef _WMI_UNIFIED_ACTION_OUI_TLV_H_
#define _WMI_UNIFIED_ACTION_OUI_TLV_H_

#ifdef WLAN_FEATURE_ACTION_OUI

#include "wmi.h"
#include "wmi_unified.h"
#include "wmi_unified_api.h"
#include "wmi_unified_param.h"

/**
 * wmi_get_action_oui_info_mask() - convert info mask to firmware specific
 * @info_mask: host specific info mask
 *
 * Return: firmware specific information mask
 */
uint32_t wmi_get_action_oui_info_mask(uint32_t info_mask);

/**
 * wmi_get_action_oui_id() - convert action id to firmware specific
 * @action_id: host specific action id
 * @id: output pointer to hold converted fw specific action id
 *
 * Return: true on conversion else failure
 */
bool wmi_get_action_oui_id(enum action_oui_id action_id,
			   wmi_vendor_oui_action_id *id);


/**
 * wmi_fill_oui_extensions() - populates wmi_vendor_oui_ext array
 * @extension: pointer to user supplied action oui extensions
 * @no_oui_extns: number of action oui extensions
 * @cmd_ext: output pointer to TLV
 *
 * This function parses the user supplied input data and populates the
 * array of variable structures TLV in WMI_PDEV_CONFIG_VENDOR_OUI_ACTION_CMDID
 *
 * Return: None
 */
void wmi_fill_oui_extensions(struct action_oui_extension *extension,
			     uint32_t no_oui_extns,
			     wmi_vendor_oui_ext *cmd_ext);

/**
 * wmi_fill_oui_extensions_buffer() - populates data buffer in action oui cmd
 * @extension: pointer to user supplied action oui extensions
 * @cmd_ext: pointer to vendor_oui_ext TLV in action oui cmd
 * @no_oui_extns: number of action oui extensions
 * @rem_var_buf_len: remaining length of buffer to be populated
 * @var_buf: output pointer to hold variable length data
 *
 * This function parses the user supplied input data and populates the variable
 * buffer of type array byte TLV in WMI_PDEV_CONFIG_VENDOR_OUI_ACTION_CMDID
 *
 * Return: QDF_STATUS_SUCCESS for successful fill else QDF_STATUS_E_INVAL
 */
QDF_STATUS
wmi_fill_oui_extensions_buffer(struct action_oui_extension *extension,
			       wmi_vendor_oui_ext *cmd_ext,
			       uint32_t no_oui_extns, uint32_t rem_var_buf_len,
			       uint8_t *var_buf);

/**
 * send_action_oui_cmd_tlv() - send action oui cmd to firmware
 * @wmi_handle: wmi handler
 * @req: pointer to action oui info
 *
 * Return: QDF_STATUS_SUCCESS on successful transmission else
 *         QDF_STATUS_E_INVAL or QDF_STATUS_E_NOMEM
 */
QDF_STATUS
send_action_oui_cmd_tlv(wmi_unified_t wmi_handle,
			struct action_oui_request *req);

#endif /* WLAN_FEATURE_ACTION_OUI */

#endif /* _WMI_UNIFIED_ACTION_OUI_TLV_H_ */
