/*
 * Copyright (c) 2019, 2021 The Linux Foundation. All rights reserved.
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

/*
 * This file contains the API definitions for the Unified Wireless Module
 * Interface (WMI) specific to crypto component.
 */

#ifndef _WMI_UNIFIED_CRYPTO_API_H_
#define _WMI_UNIFIED_CRYPTO_API_H_

/*
 * WMI_ADD_CIPHER_KEY_CMDID
 */
typedef enum {
	PAIRWISE_USAGE      = 0x00,
	GROUP_USAGE         = 0x01,
	TX_USAGE            = 0x02, /* default Tx Key - Static WEP only */
	PMK_USAGE           = 0x04, /* PMK cache */
} KEY_USAGE;

/**
 * wmi_extract_install_key_comp_event() - extract params of install key complete
 *                                        from event
 * @wmi_handle: wmi handle
 * @evt_buf: pointer to event buffer
 * @len: length of the event buffer
 * @params: Pointer to hold params of install key complete
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS
wmi_extract_install_key_comp_event(wmi_unified_t wmi_handle,
				   void *evt_buf, uint32_t len,
				   struct wmi_install_key_comp_event *param);
#endif

