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
 * DOC: Target interface file for pmo component to
 * Implement api's which shall be used by pmo component
 * in target if internally.
 */

#include "target_if_pmo.h"
#include "target_if_pmo_main.h"

void target_if_pmo_register_tx_ops(struct wlan_lmac_if_tx_ops *tx_ops)
{
	struct wlan_lmac_if_pmo_tx_ops *pmo_tx_ops;

	if (!tx_ops) {
		target_if_err("lmac tx_ops is null");
		return;
	}

	pmo_tx_ops = &tx_ops->pmo_tx_ops;
	pmo_tx_ops->send_arp_offload_req =
		target_if_pmo_send_arp_offload_req;
	pmo_tx_ops->send_ns_offload_req =
		target_if_pmo_send_ns_offload_req;
	pmo_tx_ops->send_enable_wow_wakeup_event_req =
		target_if_pmo_enable_wow_wakeup_event;
	pmo_tx_ops->send_disable_wow_wakeup_event_req =
		target_if_pmo_disable_wow_wakeup_event;
	pmo_tx_ops->send_add_wow_pattern =
		target_if_pmo_send_wow_patterns_to_fw;
	pmo_tx_ops->send_enhance_mc_offload_req =
		target_if_pmo_send_enhance_mc_offload_req;
	pmo_tx_ops->send_set_mc_filter_req =
		target_if_pmo_set_mc_filter_req;
	pmo_tx_ops->send_clear_mc_filter_req =
		target_if_pmo_clear_mc_filter_req;
	pmo_tx_ops->send_ra_filter_req =
		target_if_pmo_send_ra_filter_req;
	pmo_tx_ops->send_gtk_offload_req =
		target_if_pmo_send_gtk_offload_req;
	pmo_tx_ops->send_get_gtk_rsp_cmd =
		target_if_pmo_send_gtk_response_req;
	pmo_tx_ops->send_action_frame_pattern_req =
		target_if_pmo_send_action_frame_patterns;

}

