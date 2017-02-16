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
 * DOC: This file contains off channel tx API definitions
 */

#include <wlan_utility.h>
#include "../inc/wlan_p2p_off_chan_tx.h"

QDF_STATUS p2p_process_mgmt_tx(struct tx_action_context *tx_ctx)
{
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS p2p_process_mgmt_tx_cancel(
	struct cancel_roc_context *cancel_tx)
{
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS p2p_process_mgmt_tx_ack_cnf(
	struct p2p_tx_conf_event *tx_cnf_event)
{
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS p2p_process_rx_mgmt(
	struct p2p_rx_mgmt_event *rx_mgmt_event)
{
	return QDF_STATUS_SUCCESS;
}
