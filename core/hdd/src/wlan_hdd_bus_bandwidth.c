/*
 * Copyright (c) 2020 The Linux Foundation. All rights reserved.
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
 * DOC: wlan_hdd_bus_bandwidth.c
 *
 * Bus Bandwidth Manager implementation
 */

#include <wlan_hdd_includes.h>
#include "qca_vendor.h"
#include "wlan_hdd_bus_bandwidth.h"

/**
 * bus_bw_table_default - default table which provides bus bandwidth level
 *  corresonding to a given connection mode and throughput level.
 */
static bus_bw_table_type bus_bw_table_default = {
	[QCA_WLAN_802_11_MODE_11B] = {BUS_BW_LEVEL_NONE, BUS_BW_LEVEL_1,
				      BUS_BW_LEVEL_2, BUS_BW_LEVEL_3,
				      BUS_BW_LEVEL_4, BUS_BW_LEVEL_6},
	[QCA_WLAN_802_11_MODE_11G] = {BUS_BW_LEVEL_NONE, BUS_BW_LEVEL_5,
				      BUS_BW_LEVEL_5, BUS_BW_LEVEL_5,
				      BUS_BW_LEVEL_5, BUS_BW_LEVEL_5},
	[QCA_WLAN_802_11_MODE_11A] = {BUS_BW_LEVEL_NONE, BUS_BW_LEVEL_5,
				      BUS_BW_LEVEL_5, BUS_BW_LEVEL_5,
				      BUS_BW_LEVEL_5, BUS_BW_LEVEL_5},
	[QCA_WLAN_802_11_MODE_11N] = {BUS_BW_LEVEL_NONE, BUS_BW_LEVEL_1,
				      BUS_BW_LEVEL_2, BUS_BW_LEVEL_3,
				      BUS_BW_LEVEL_4, BUS_BW_LEVEL_6},
	[QCA_WLAN_802_11_MODE_11AC] = {BUS_BW_LEVEL_NONE, BUS_BW_LEVEL_1,
				       BUS_BW_LEVEL_2, BUS_BW_LEVEL_3,
				       BUS_BW_LEVEL_4, BUS_BW_LEVEL_6},
	[QCA_WLAN_802_11_MODE_11AX] = {BUS_BW_LEVEL_NONE, BUS_BW_LEVEL_1,
				       BUS_BW_LEVEL_2, BUS_BW_LEVEL_3,
				       BUS_BW_LEVEL_4, BUS_BW_LEVEL_6},
};

/**
 * bus_bw_table_low_latency - table which provides bus bandwidth level
 *  corresonding to a given connection mode and throughput level in low
 *  latency setting.
 */
static bus_bw_table_type bus_bw_table_low_latency = {
	[QCA_WLAN_802_11_MODE_11B] = {BUS_BW_LEVEL_NONE, BUS_BW_LEVEL_6,
				      BUS_BW_LEVEL_6, BUS_BW_LEVEL_6,
				      BUS_BW_LEVEL_6, BUS_BW_LEVEL_6},
	[QCA_WLAN_802_11_MODE_11G] = {BUS_BW_LEVEL_NONE, BUS_BW_LEVEL_6,
				      BUS_BW_LEVEL_6, BUS_BW_LEVEL_6,
				      BUS_BW_LEVEL_6, BUS_BW_LEVEL_6},
	[QCA_WLAN_802_11_MODE_11A] = {BUS_BW_LEVEL_NONE, BUS_BW_LEVEL_6,
				      BUS_BW_LEVEL_6, BUS_BW_LEVEL_6,
				      BUS_BW_LEVEL_6, BUS_BW_LEVEL_6},
	[QCA_WLAN_802_11_MODE_11N] = {BUS_BW_LEVEL_NONE, BUS_BW_LEVEL_6,
				      BUS_BW_LEVEL_6, BUS_BW_LEVEL_6,
				      BUS_BW_LEVEL_6, BUS_BW_LEVEL_6},
	[QCA_WLAN_802_11_MODE_11AC] = {BUS_BW_LEVEL_NONE, BUS_BW_LEVEL_6,
				       BUS_BW_LEVEL_6, BUS_BW_LEVEL_6,
				       BUS_BW_LEVEL_6, BUS_BW_LEVEL_6},
	[QCA_WLAN_802_11_MODE_11AX] = {BUS_BW_LEVEL_NONE, BUS_BW_LEVEL_6,
				       BUS_BW_LEVEL_6, BUS_BW_LEVEL_6,
				       BUS_BW_LEVEL_6, BUS_BW_LEVEL_6},
};

int hdd_bbm_context_init(struct hdd_context *hdd_ctx)
{
	struct bbm_context *bbm_ctx;
	QDF_STATUS status;

	bbm_ctx = qdf_mem_malloc(sizeof(*bbm_ctx));
	if (!bbm_ctx)
		return -ENOMEM;

	bbm_ctx->curr_bus_bw_lookup_table = &bus_bw_table_default;

	status = qdf_mutex_create(&bbm_ctx->bbm_lock);
	if (QDF_IS_STATUS_ERROR(status))
		goto free_ctx;

	hdd_ctx->bbm_ctx = bbm_ctx;

	return 0;

free_ctx:
	qdf_mem_free(bbm_ctx);

	return qdf_status_to_os_return(status);
}

void hdd_bbm_context_deinit(struct hdd_context *hdd_ctx)
{
	struct bbm_context *bbm_ctx = hdd_ctx->bbm_ctx;

	if (!bbm_ctx)
		return;

	hdd_ctx->bbm_ctx = NULL;
	qdf_mutex_destroy(&bbm_ctx->bbm_lock);

	qdf_mem_free(bbm_ctx);
}
