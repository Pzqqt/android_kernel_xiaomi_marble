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

#include <wmi_unified_priv.h>
#include <wmi_unified_gpio_api.h>

QDF_STATUS wmi_unified_gpio_config_cmd_send(wmi_unified_t wmi_handle,
					    struct gpio_config_params *param)
{
	if (wmi_handle->ops->send_gpio_config_cmd)
		return wmi_handle->ops->send_gpio_config_cmd(wmi_handle, param);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wmi_unified_gpio_output_cmd_send(wmi_unified_t wmi_handle,
					    struct gpio_output_params *param)
{
	if (wmi_handle->ops->send_gpio_output_cmd)
		return wmi_handle->ops->send_gpio_output_cmd(wmi_handle, param);

	return QDF_STATUS_E_FAILURE;
}

