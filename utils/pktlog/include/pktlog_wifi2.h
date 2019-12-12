/**
 * Copyright (c) 2013-2020, The Linux Foundation. All rights reserved.
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

#include "ol_txrx_types.h"
#include "ol_htt_tx_api.h"
#include "ol_tx_desc.h"
#include "qdf_mem.h"
#include "htt.h"
#include "htt_internal.h"
#include "pktlog_ac_i.h"
#include "wma_api.h"
#include "wlan_logging_sock_svc.h"

#define TX_DESC_ID_LOW_MASK     0xffff
#define TX_DESC_ID_LOW_SHIFT    0
#define TX_DESC_ID_HIGH_MASK    0xffff0000
#define TX_DESC_ID_HIGH_SHIFT   16

#ifndef REMOVE_PKT_LOG
/**
 * process_tx_info() - process tx pktlog buffers
 * @txrx_pdev: ol pdev handle
 * @data: pktlog buffer
 *
 * Return: 0 - success/non-zero - failure
 */
A_STATUS process_tx_info(struct cdp_pdev *txrx_pdev, void *data);
#else
static inline
A_STATUS process_tx_info(struct cdp_pdev *txrx_pdev, void *data)
{
	return 0;
}
#endif /* REMOVE_PKT_LOG */
