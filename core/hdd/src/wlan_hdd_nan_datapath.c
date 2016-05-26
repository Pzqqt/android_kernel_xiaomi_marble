/*
 * Copyright (c) 2016 The Linux Foundation. All rights reserved.
 *
 * Previously licensed under the ISC license by Qualcomm Atheros, Inc.
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
 * DOC: wlan_hdd_nan_datapath.c
 *
 * WLAN Host Device Driver nan datapath API implementation
 */
#include <wlan_hdd_includes.h>
#include "wlan_hdd_nan_datapath.h"

/**
 * hdd_ndp_print_ini_config()- Print nan datapath specific INI configuration
 * @hdd_ctx: handle to hdd context
 *
 * Return: None
 */
void hdd_ndp_print_ini_config(hdd_context_t *hdd_ctx)
{
	hddLog(LOG2, "Name = [%s] Value = [%u]",
		CFG_ENABLE_NAN_DATAPATH_NAME,
		hdd_ctx->config->enable_nan_datapath);
	hddLog(LOG2, "Name = [%s] Value = [%u]",
		CFG_ENABLE_NAN_NDI_CHANNEL_NAME,
		hdd_ctx->config->nan_datapath_ndi_channel);
}

/**
 * hdd_nan_datapath_target_config() - Configure NAN datapath features
 * @hdd_ctx: Pointer to HDD context
 * @cfg: Pointer to target device capability information
 *
 * NaN datapath functionality is enabled if it is enabled in
 * .ini file and also supported in target device.
 *
 * Return: None
 */

void hdd_nan_datapath_target_config(hdd_context_t *hdd_ctx,
					struct wma_tgt_cfg *cfg)
{
	hdd_ctx->config->enable_nan_datapath &= cfg->nan_datapath_enabled;
	hddLog(LOG1, FL("enable_nan_datapath: %d"),
		hdd_ctx->config->enable_nan_datapath);
}
