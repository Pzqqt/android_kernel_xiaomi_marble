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
 * DOC: This file contains gpio north bound interface definitions
 */
#include <wlan_gpio_ucfg_api.h>
#include <wlan_gpio_tgt_api.h>
#include "qdf_module.h"

QDF_STATUS
ucfg_set_gpio_config(struct wlan_objmgr_psoc *psoc,
		     struct gpio_config_params *param)
{
	return tgt_set_gpio_config_req(psoc, param);
}
qdf_export_symbol(ucfg_set_gpio_config);

QDF_STATUS
ucfg_set_gpio_output(struct wlan_objmgr_psoc *psoc,
		     struct gpio_output_params *param)
{
	return tgt_set_gpio_output_req(psoc, param);
}
qdf_export_symbol(ucfg_set_gpio_output);
