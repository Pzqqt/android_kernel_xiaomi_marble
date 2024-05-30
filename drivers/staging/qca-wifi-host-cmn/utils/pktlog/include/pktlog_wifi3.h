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

#include "pktlog_ac_i.h"
#include "wlan_logging_sock_svc.h"

#ifndef REMOVE_PKT_LOG
/**
 * process_offload_pktlog_wifi3() - Process full pktlog events
 * pdev: abstract pdev handle
 * data: pktlog buffer
 *
 * Return: zero on success, non-zero on failure
 */
A_STATUS
process_offload_pktlog_wifi3(struct cdp_pdev *pdev, void *data);

/**
 * process_rx_desc_remote_wifi3() - Process pktlog buffers received
 *                                  from monitor status ring
 * @pdev: pdev handle
 * @data: pktlog buffer pointer
 *
 * Return: 0 - success/non-zero - failure
 */
int process_rx_desc_remote_wifi3(void *pdev, void *data);

/**
 * process_pktlog_lite_wifi3() - Process pktlog buffers received
 *                               from monitor status ring
 * @pdev: pdev handle
 * @data: pktlog buffer pointer
 *
 * Return: 0 - success/non-zero - failure
 */
int process_pktlog_lite_wifi3(void *context, void *log_data,
			      uint16_t log_type);
#else
static inline A_STATUS
process_offload_pktlog_wifi3(struct cdp_pdev *pdev, void *data)
{
	return 0;
}

static inline
int process_rx_desc_remote_wifi3(void *pdev, void *data)
{
	return 0;
}

static inline int
process_pktlog_lite_wifi3(void *context, void *log_data,
			  uint16_t log_type)
{
	return 0;
}
#endif /* REMOVE_PKT_LOG */

/**
 * process_tx_info() - process tx pktlog buffers
 * @txrx_pdev: ol pdev handle
 * @data: pktlog buffer
 *
 * Return: 0 - success/non-zero - failure
 */
static inline
A_STATUS process_tx_info(struct cdp_pdev *txrx_pdev, void *data)
{
	return 0;
}

/**
 * process_rx_info_remote() - process rx pktlog buffers
 * @txrx_pdev: ol pdev handle
 * @data: pktlog buffer
 *
 * Return: 0 - success/non-zero - failure
 */
static inline
A_STATUS process_rx_info_remote(void *pdev, void *data)
{
	return 0;
}

/**
 * process_rx_remote() - process rx pktlog buffers
 * @txrx_pdev: ol pdev handle
 * @data: pktlog buffer
 *
 * Return: 0 - success/non-zero - failure
 */
static inline
A_STATUS process_rx_info(void *pdev, void *data)
{
	return 0;
}

/**
 * process_rate_find() - process rate event pktlog buffers
 * @txrx_pdev: ol pdev handle
 * @data: pktlog buffer
 *
 * Return: 0 - success/non-zero - failure
 */
static inline
A_STATUS process_rate_find(void *pdev, void *data)
{
	return 0;
}

/**
 * process_rate_update() - process rate event pktlog buffers
 * @txrx_pdev: ol pdev handle
 * @data: pktlog buffer
 *
 * Return: 0 - success/non-zero - failure
 */
static inline
A_STATUS process_rate_update(void *pdev, void *data)
{
	return 0;
}

/**
 * process_sw_event() - process sw event pktlog buffers
 * @txrx_pdev: ol pdev handle
 * @data: pktlog buffer
 *
 * Return: 0 - success/non-zero - failure
 */
static inline
A_STATUS process_sw_event(void *pdev, void *data)
{
	return 0;
}
