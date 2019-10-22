/*
 * Copyright (c) 2016-2020 The Linux Foundation. All rights reserved.
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
 * @file cdp_txrx_mon.h
 * @brief Define the monitor mode API functions
 * called by the host control SW and the OS interface module
 */

#ifndef _CDP_TXRX_MON_H_
#define _CDP_TXRX_MON_H_
#include "cdp_txrx_handle.h"

static inline QDF_STATUS cdp_reset_monitor_mode(ol_txrx_soc_handle soc,
						uint8_t pdev_id,
						u_int8_t smart_monitor)
{
	if (!soc || !soc->ops) {
		QDF_TRACE(QDF_MODULE_ID_CDP, QDF_TRACE_LEVEL_DEBUG,
				"%s: Invalid Instance", __func__);
		QDF_BUG(0);
		return 0;
	}

	if (!soc->ops->mon_ops ||
	    !soc->ops->mon_ops->txrx_reset_monitor_mode)
		return 0;

	return soc->ops->mon_ops->txrx_reset_monitor_mode(soc, pdev_id,
							  smart_monitor);
}

/**
 * cdp_deliver_tx_mgmt() - Deliver mgmt frame for tx capture
 * @soc: Datapath SOC handle
 * @pdev_id: id of datapath PDEV handle
 * @nbuf: Management frame buffer
 */
static inline QDF_STATUS
cdp_deliver_tx_mgmt(ol_txrx_soc_handle soc, uint8_t pdev_id,
		    qdf_nbuf_t nbuf)
{
	if (!soc || !soc->ops) {
		QDF_TRACE(QDF_MODULE_ID_CDP, QDF_TRACE_LEVEL_DEBUG,
			  "%s: Invalid Instance", __func__);
		QDF_BUG(0);
		return QDF_STATUS_E_FAILURE;
	}

	if (!soc->ops->mon_ops ||
	    !soc->ops->mon_ops->txrx_deliver_tx_mgmt)
		return QDF_STATUS_E_FAILURE;

	return soc->ops->mon_ops->txrx_deliver_tx_mgmt(soc, pdev_id, nbuf);
}

#ifdef WLAN_FEATURE_PKT_CAPTURE
static inline void
cdp_pktcapture_record_channel(
			ol_txrx_soc_handle soc,
			uint8_t pdev_id,
			int chan_num)
{
	if (!soc || !soc->ops) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_DEBUG,
			  "%s invalid instance", __func__);
		QDF_BUG(0);
		return;
	}

	if (!soc->ops->pktcapture_ops ||
	    !soc->ops->pktcapture_ops->txrx_pktcapture_record_channel)
		return;

	soc->ops->pktcapture_ops->txrx_pktcapture_record_channel(soc,
								 pdev_id,
								 chan_num);
}

static inline void
cdp_set_packet_capture_mode(ol_txrx_soc_handle soc,
			    uint8_t pdev_id,
			    uint8_t val)
{
	if (!soc || !soc->ops) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_DEBUG,
			  "%s invalid instance", __func__);
		QDF_BUG(0);
		return;
	}

	if (!soc->ops->pktcapture_ops ||
	    !soc->ops->pktcapture_ops->txrx_pktcapture_set_mode)
		return;

	soc->ops->pktcapture_ops->txrx_pktcapture_set_mode(soc, pdev_id, val);
}

static inline uint8_t
cdp_get_packet_capture_mode(ol_txrx_soc_handle soc, uint8_t pdev_id)
{
	if (!soc || !soc->ops) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_DEBUG,
			  "%s invalid instance", __func__);
		QDF_BUG(0);
		return 0;
	}

	if (!soc->ops->pktcapture_ops ||
	    !soc->ops->pktcapture_ops->txrx_pktcapture_get_mode)
		return 0;

	return soc->ops->pktcapture_ops->txrx_pktcapture_get_mode(soc,
								  pdev_id);
}

static inline QDF_STATUS
cdp_register_pktcapture_cb(
		ol_txrx_soc_handle soc, uint8_t pdev_id, void *ctx,
		QDF_STATUS(txrx_pktcapture_cb)(void *, qdf_nbuf_t))
{
	if (!soc || !soc->ops) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_FATAL,
			  "%s invalid instance", __func__);
		return QDF_STATUS_E_INVAL;
	}

	if (!soc->ops->pktcapture_ops ||
	    !soc->ops->pktcapture_ops->txrx_pktcapture_cb_register)
		return QDF_STATUS_E_INVAL;

	return soc->ops->pktcapture_ops->txrx_pktcapture_cb_register(
							soc,
							pdev_id,
							ctx,
							txrx_pktcapture_cb);
}

static inline QDF_STATUS
cdp_deregister_pktcapture_cb(ol_txrx_soc_handle soc, uint8_t pdev_id)
{
	if (!soc || !soc->ops) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_FATAL,
			  "%s invalid instance", __func__);
		return QDF_STATUS_E_INVAL;
	}

	if (!soc->ops->pktcapture_ops ||
	    !soc->ops->pktcapture_ops->txrx_pktcapture_cb_deregister)
		return QDF_STATUS_E_INVAL;

	return soc->ops->pktcapture_ops->txrx_pktcapture_cb_deregister(soc,
					pdev_id);
}

static inline QDF_STATUS
cdp_pktcapture_mgmtpkt_process(
			ol_txrx_soc_handle soc,
			uint8_t pdev_id,
			struct mon_rx_status *txrx_status,
			qdf_nbuf_t nbuf,
			uint8_t status)
{
	if (!soc || !soc->ops) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_FATAL,
			  "%s invalid instance", __func__);
		return QDF_STATUS_E_INVAL;
	}

	if (!soc->ops->pktcapture_ops ||
	    !soc->ops->pktcapture_ops->txrx_pktcapture_mgmtpkt_process)
		return QDF_STATUS_E_INVAL;

	return soc->ops->pktcapture_ops->txrx_pktcapture_mgmtpkt_process(
							soc,
							pdev_id,
							txrx_status,
							nbuf,
							status);
}
#else
static inline uint8_t
cdp_get_packet_capture_mode(ol_txrx_soc_handle soc, uint8_t pdev_id)
{
	return 0;
}

static inline void
cdp_pktcapture_record_channel(ol_txrx_soc_handle soc,
			      uint8_t pdev_id,
			      int chan_num)
{
}
#endif /* WLAN_FEATURE_PKT_CAPTURE */

#endif
