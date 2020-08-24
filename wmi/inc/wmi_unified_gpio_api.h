/*
 * Copyright (c) 2020, The Linux Foundation. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */
/**
 * DOC: Implement API's specific to gpio component.
 */

#ifndef _WMI_UNIFIED_GPIO_API_H_
#define _WMI_UNIFIED_GPIO_API_H_

#include <wmi_unified_param.h>

/**
 * wmi_unified_gpio_config_cmd_send() - WMI gpio config function
 * @wmi_handle: handle to WMI.
 * @param: pointer to hold gpio config param
 *
 * Send WMI set gpio configuration to firmware.
 *
 * Return QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_gpio_config_cmd_send(wmi_unified_t wmi_handle,
					    struct gpio_config_params *param);

/**
 * wmi_unified_gpio_output_cmd_send() - WMI gpio output function
 * @wmi_handle: handle to WMI.
 * @param: pointer to hold gpio output param
 *
 * Send WMI set gpio output value to firmware.
 *
 * Return QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_gpio_output_cmd_send(wmi_unified_t wmi_handle,
					    struct gpio_output_params *param);

#endif /* _WMI_UNIFIED_GPIO_API_H_ */
