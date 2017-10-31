/*
 * Copyright (c) 2017 The Linux Foundation. All rights reserved.
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
 *  DOC: wlan_hdd_packet_filter.c
 *
 *  WLAN Host Device Driver implementation
 *
 */

/* Include Files */
#include "wlan_hdd_packet_filter_api.h"
#include "wlan_hdd_packet_filter_rules.h"

int hdd_enable_default_pkt_filters(struct hdd_adapter *adapter)
{
	struct hdd_context *hdd_ctx;
	uint8_t filters = 0, i = 0, filter_id = 1;

	hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	if (hdd_ctx == NULL) {
		hdd_err("HDD context is Null!!!");
		return -EINVAL;
	}
	if (hdd_ctx->user_configured_pkt_filter_rules) {
		hdd_info("user has defined pkt filter run hence skipping default packet filter rule");
		return 0;
	}

	filters = hdd_ctx->config->packet_filters_bitmap;

	while (filters != 0) {
		if (filters & 0x1) {
			hdd_err("setting filter[%d], of id = %d",
				i+1, filter_id);
			packet_filter_default_rules[i].filter_id = filter_id;
			wlan_hdd_set_filter(hdd_ctx,
					    &packet_filter_default_rules[i],
					    adapter->session_id);
			filter_id++;
		}
		filters = filters >> 1;
		i++;
	}

	return 0;
}

int hdd_disable_default_pkt_filters(struct hdd_adapter *adapter)
{
	struct hdd_context *hdd_ctx;
	uint8_t filters = 0, i = 0, filter_id = 1;

	struct pkt_filter_cfg packet_filter_default_rules;

	hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	if (hdd_ctx == NULL) {
		hdd_err("HDD context is Null!!!");
		return -EINVAL;
	}

	if (hdd_ctx->user_configured_pkt_filter_rules) {
		hdd_info("user has defined pkt filter run hence skipping default packet filter rule");
		return 0;
	}

	filters = hdd_ctx->config->packet_filters_bitmap;

	while (filters != 0) {
		if (filters & 0x1) {
			hdd_err("Clearing filter[%d], of id = %d",
				i+1, filter_id);
			packet_filter_default_rules.filter_action =
						HDD_RCV_FILTER_CLEAR;
			packet_filter_default_rules.filter_id = filter_id;
			wlan_hdd_set_filter(hdd_ctx,
					    &packet_filter_default_rules,
					    adapter->session_id);
			filter_id++;
		}
		filters = filters >> 1;
		i++;
	}

	return 0;
}
