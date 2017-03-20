/*
 * Copyright (c) 2016-2017 The Linux Foundation. All rights reserved.
 *
 * Previously licensed under the ISC license by Qualcomm Atheros, Inc.
 *
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
 * This file was originally distributed by Qualcomm Atheros, Inc.
 * under proprietary terms before Copyright ownership was assigned
 * to the Linux Foundation.
 */
 /**
 * @file cdp_txrx_host_stats.h
 * @brief Define the host data path stats API functions
 * called by the host control SW and the OS interface module
 */
#ifndef _CDP_TXRX_HOST_STATS_H_
#define _CDP_TXRX_HOST_STATS_H_
#include "cdp_txrx_handle.h"
/* WIN */
/* Need to remove the "req" parameter */
/* Need to rename the function to reflect the functionality "show" / "display"
 * WIN -- to figure out whether to change OSIF to converge (not an immediate AI)
 * */
/**
 * cdp_host_stats_get: cdp call to get host stats
 * @soc: SOC handle
 * @req: Requirement type
 * @type: Host stat type
 *
 * return: 0 for Success, Failure returns error message
 */
static inline int cdp_host_stats_get(ol_txrx_soc_handle soc,
	struct cdp_vdev *vdev,
	struct ol_txrx_stats_req *req, enum cdp_host_txrx_stats type)
{
	if (soc->ops->host_stats_ops->txrx_host_stats_get)
		return soc->ops->host_stats_ops->txrx_host_stats_get(vdev, req,
				type);
	return 0;
}

static inline void
cdp_host_ce_stats(ol_txrx_soc_handle soc, struct cdp_vdev *vdev)
{
	if (soc->ops->host_stats_ops->txrx_host_ce_stats)
		return soc->ops->host_stats_ops->txrx_host_ce_stats(vdev);
	return;
}

static inline int cdp_stats_publish
	(ol_txrx_soc_handle soc, struct cdp_pdev *pdev,
	struct ol_txrx_stats *buf)
{
	if (soc->ops->host_stats_ops->txrx_stats_publish)
		return soc->ops->host_stats_ops->txrx_stats_publish(pdev, buf);
	return 0;
}
/**
 * @brief Enable enhanced stats functionality.
 *
 * @param pdev - the physical device object
 * @return - void
 */
static inline void
cdp_enable_enhanced_stats(ol_txrx_soc_handle soc, struct cdp_pdev *pdev)
{
	if (soc->ops->host_stats_ops->txrx_enable_enhanced_stats)
		return soc->ops->host_stats_ops->txrx_enable_enhanced_stats
			(pdev);
	return;
}

/**
 * @brief Disable enhanced stats functionality.
 *
 * @param pdev - the physical device object
 * @return - void
 */
static inline void
cdp_disable_enhanced_stats(ol_txrx_soc_handle soc, struct cdp_pdev *pdev)
{
	if (soc->ops->host_stats_ops->txrx_disable_enhanced_stats)
		return soc->ops->host_stats_ops->txrx_disable_enhanced_stats
			(pdev);
	return;
}

/**
 * @brief Get the desired stats from the message.
 *
 * @param pdev - the physical device object
 * @param stats_base - stats buffer recieved from FW
 * @param type - stats type.
 * @return - pointer to requested stat identified by type
 */
static inline uint32_t *cdp_get_stats_base
	(ol_txrx_soc_handle soc, struct cdp_pdev *pdev,
	uint32_t *stats_base, uint32_t msg_len, uint8_t type)
{
	if (soc->ops->host_stats_ops->txrx_get_stats_base)
		return (uint32_t *)soc->ops->host_stats_ops->txrx_get_stats_base
			(pdev, stats_base, msg_len, type);
	return 0;
}
static inline void
cdp_tx_print_tso_stats(ol_txrx_soc_handle soc,
	struct cdp_vdev *vdev)
{
	if (soc->ops->host_stats_ops->tx_print_tso_stats)
		return soc->ops->host_stats_ops->tx_print_tso_stats(vdev);
	return;
}

static inline void
cdp_tx_rst_tso_stats(ol_txrx_soc_handle soc, struct cdp_vdev *vdev)
{
	if (soc->ops->host_stats_ops->tx_rst_tso_stats)
		return soc->ops->host_stats_ops->tx_rst_tso_stats(vdev);
	return;
}

static inline void
cdp_tx_print_sg_stats(ol_txrx_soc_handle soc,
	struct cdp_vdev *vdev)
{
	if (soc->ops->host_stats_ops->tx_print_sg_stats)
		return soc->ops->host_stats_ops->tx_print_sg_stats(vdev);
	return;
}

static inline void
cdp_tx_rst_sg_stats(ol_txrx_soc_handle soc, struct cdp_vdev *vdev)
{
	if (soc->ops->host_stats_ops->tx_rst_sg_stats)
		return soc->ops->host_stats_ops->tx_rst_sg_stats(vdev);
	return;
}

static inline void
cdp_print_rx_cksum_stats(ol_txrx_soc_handle soc,
	struct cdp_vdev *vdev)
{
	if (soc->ops->host_stats_ops->print_rx_cksum_stats)
		return soc->ops->host_stats_ops->print_rx_cksum_stats(vdev);
	return;
}

static inline void
cdp_rst_rx_cksum_stats(ol_txrx_soc_handle soc, struct cdp_vdev *vdev)
{
	if (soc->ops->host_stats_ops->rst_rx_cksum_stats)
		return soc->ops->host_stats_ops->rst_rx_cksum_stats(vdev);
	return;
}

static inline A_STATUS
cdp_host_me_stats(ol_txrx_soc_handle soc, struct cdp_vdev *vdev)
{
	if (soc->ops->host_stats_ops->txrx_host_me_stats)
		return soc->ops->host_stats_ops->txrx_host_me_stats(vdev);
	return 0;
}
static inline void cdp_per_peer_stats
	(ol_txrx_soc_handle soc, struct cdp_pdev *pdev, char *addr)
{
	if (soc->ops->host_stats_ops->txrx_per_peer_stats)
		return soc->ops->host_stats_ops->txrx_per_peer_stats
			(pdev, addr);
	return;
}

static inline int cdp_host_msdu_ttl_stats(ol_txrx_soc_handle soc,
	struct cdp_vdev *vdev,
	struct ol_txrx_stats_req *req)
{
	if (soc->ops->host_stats_ops->txrx_host_msdu_ttl_stats)
		return soc->ops->host_stats_ops->txrx_host_msdu_ttl_stats
			(vdev, req);
	return 0;
}



static inline void
cdp_print_lro_stats(ol_txrx_soc_handle soc, struct cdp_vdev *vdev)
{
	if (soc->ops->host_stats_ops->print_lro_stats)
		return soc->ops->host_stats_ops->print_lro_stats(vdev);
	return;
}

static inline void
cdp_reset_lro_stats(ol_txrx_soc_handle soc, struct cdp_vdev *vdev)
{
	if (soc->ops->host_stats_ops->reset_lro_stats)
		return soc->ops->host_stats_ops->reset_lro_stats(vdev);
	return;
}

/**
 * @brief Parse the stats header and get the payload from the message.
 *
 * @param pdev - the physical device object
 * @param msg_word - stats buffer recieved from FW
 * @param msg_len - length of the message
 * @param type - place holder for parsed message type
 * @param status - place holder for parsed message status
 * @return - pointer to received stat payload
 */

#if defined(QCA_SUPPORT_SON) || defined(ENHANCED_STATS)
uint32_t *ol_txrx_get_en_stats_base(ol_txrx_pdev_handle txrx_pdev, uint32_t *msg_word,
    uint32_t msg_len, enum htt_cmn_t2h_en_stats_type *type,  enum htt_cmn_t2h_en_stats_status *status);
#endif
#endif /* _CDP_TXRX_HOST_STATS_H_ */
