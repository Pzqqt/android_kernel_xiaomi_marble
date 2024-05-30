/*
 * Copyright (c) 2013-2019 The Linux Foundation. All rights reserved.
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
 * DOC: Implement API's specific to DBR component.
 */

#ifndef _WMI_UNIFIED_DBR_API_H_
#define _WMI_UNIFIED_DBR_API_H_

#include "wmi_unified_dbr_param.h"

/**
 * wmi_unified_dbr_ring_cfg: Configure direct buffer rx rings
 * @wmi_handle: WMI handle
 * @cfg: pointer to direct buffer rx config request
 *
 * Return: QDF status of operation
 */
QDF_STATUS wmi_unified_dbr_ring_cfg(wmi_unified_t wmi_handle,
				    struct direct_buf_rx_cfg_req *cfg);

/**
 * wmi_extract_dbr_buf_release_fixed : Extract direct buffer rx fixed param
 *				     from buffer release event
 * @wmi_handle: WMI handle
 * @evt_buf: Event buffer
 * @param: Pointer to direct buffer rx response struct
 *
 * Return: QDF status of operation
 */
QDF_STATUS wmi_extract_dbr_buf_release_fixed(
			wmi_unified_t wmi_handle,
			uint8_t *evt_buf,
			struct direct_buf_rx_rsp *param);

/**
 * wmi_extract_dbr_buf_release_entry: Extract direct buffer rx buffer tlv
 *
 * @wmi_handle: WMI handle
 * @evt_buf: Event buffer
 * @idx: Index of the module for which capability is received
 * @param: Pointer to direct buffer rx entry
 *
 * Return: QDF status of operation
 */
QDF_STATUS wmi_extract_dbr_buf_release_entry(
			wmi_unified_t wmi_handle,
			uint8_t *evt_buf, uint8_t idx,
			struct direct_buf_rx_entry *param);

/**
 * wmi_extract_dbr_buf_metadata: Extract direct buffer metadata
 *
 * @wmi_handle: WMI handle
 * @evt_buf: Event buffer
 * @idx: Index of the module for which capability is received
 * @param: Pointer to direct buffer metadata
 *
 * Return: QDF status of operation
 */
QDF_STATUS wmi_extract_dbr_buf_metadata(
			wmi_unified_t wmi_handle,
			uint8_t *evt_buf, uint8_t idx,
			struct direct_buf_rx_metadata *param);

#endif /* _WMI_UNIFIED_DBR_API_H_ */
