/*
 * Copyright (c) 2018 The Linux Foundation. All rights reserved.
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
 * DOC: wma_twt.c
 *
 * WLAN Host Device Driver TWT - Target Wake Time Implementation
 */

#include "wma_twt.h"
#include "wmi_unified_twt_api.h"

void wma_send_twt_enable_cmd(uint32_t pdev_id, uint32_t congestion_timeout)
{
	t_wma_handle *wma = cds_get_context(QDF_MODULE_ID_WMA);
	struct wmi_twt_enable_param twt_enable_params = {0};
	int32_t ret;

	twt_enable_params.pdev_id = pdev_id;
	twt_enable_params.sta_cong_timer_ms = congestion_timeout;
	ret = wmi_unified_twt_enable_cmd(wma->wmi_handle, &twt_enable_params);

	if (ret)
		WMA_LOGE("Failed to enable TWT");
}

