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

/*
 * This file contains the API definitions for the Unified Wireless Module
 * Interface (WMI) specific to beacon component
 */

#ifndef _WMI_UNIFIED_BCN_API_H_
#define _WMI_UNIFIED_BCN_API_H_

#include "wmi_unified_param.h"
#include "wmi_unified.h"
#include "wmi_unified_priv.h"

/**
 * wmi_unified_bcn_buf_ll_cmd() - prepare and send beacon buffer to fw for LL
 * @wmi_handle: wmi handle
 * @param: bcn ll cmd parameter
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */

QDF_STATUS
wmi_unified_bcn_buf_ll_cmd(wmi_unified_t wmi_handle,
			   wmi_bcn_send_from_host_cmd_fixed_param * param);
#endif

