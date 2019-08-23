/*
 * Copyright (c) 2016-2019 The Linux Foundation. All rights reserved.
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
 * @file cdp_txrx_host_stats.h
 * @brief Define the host data path stats API functions
 * called by the host control SW and the OS interface module
 */
#ifndef _CDP_TXRX_HOST_STATS_H_
#define _CDP_TXRX_HOST_STATS_H_
#include "cdp_txrx_handle.h"
/**
 * cdp_host_stats_get: cdp call to get host stats
 * @soc: SOC handle
 * @req: Requirement type
 *
 * return: 0 for Success, Failure returns error message
 */
static inline int cdp_host_stats_get(ol_txrx_soc_handle soc,
		struct cdp_vdev *vdev,
		struct ol_txrx_stats_req *req)
{
	if (!soc || !soc->ops) {
		QDF_TRACE(QDF_MODULE_ID_CDP, QDF_TRACE_LEVEL_DEBUG,
				"%s: Invalid Instance", __func__);
		QDF_BUG(0);
		return 0;
	}

	if (!soc->ops->host_stats_ops ||
	    !soc->ops->host_stats_ops->txrx_host_stats_get)
		return 0;

	return soc->ops->host_stats_ops->txrx_host_stats_get(vdev, req);
}

/**
 * cdp_host_stats_get_ratekbps: cdp call to get rate in kbps
 * @soc: SOC handle
 * @preamb: Preamble
 * @mcs: Modulation and Coding scheme index
 * @htflag: Flag to identify HT or VHT
 * @gintval: Gaurd Interval value
 *
 * return: 0 for Failure, Returns rate on Success
 */
static inline int cdp_host_stats_get_ratekbps(ol_txrx_soc_handle soc,
					      int preamb, int mcs,
					      int htflag, int gintval)
{
	if (!soc || !soc->ops) {
		QDF_TRACE(QDF_MODULE_ID_CDP, QDF_TRACE_LEVEL_DEBUG,
			  "%s: Invalid Instance", __func__);
		QDF_BUG(0);
		return 0;
	}

	if (!soc->ops->host_stats_ops ||
	    !soc->ops->host_stats_ops->txrx_get_ratekbps)
		return 0;

	return soc->ops->host_stats_ops->txrx_get_ratekbps(preamb,
							   mcs, htflag,
							   gintval);
}

/**
 * cdp_host_stats_clr: cdp call to clear host stats
 * @vdev: vdev handle
 *
 * return: void
 */
static inline void
cdp_host_stats_clr(ol_txrx_soc_handle soc, struct cdp_vdev *vdev)
{
	if (!soc || !soc->ops) {
		QDF_TRACE(QDF_MODULE_ID_CDP, QDF_TRACE_LEVEL_DEBUG,
				"%s: Invalid Instance", __func__);
		QDF_BUG(0);
		return;
	}

	if (!soc->ops->host_stats_ops ||
	    !soc->ops->host_stats_ops->txrx_host_stats_clr)
		return;

	soc->ops->host_stats_ops->txrx_host_stats_clr(vdev);
}

static inline void
cdp_host_ce_stats(ol_txrx_soc_handle soc, struct cdp_vdev *vdev)
{
	if (!soc || !soc->ops) {
		QDF_TRACE(QDF_MODULE_ID_CDP, QDF_TRACE_LEVEL_DEBUG,
				"%s: Invalid Instance", __func__);
		QDF_BUG(0);
		return;
	}

	if (!soc->ops->host_stats_ops ||
	    !soc->ops->host_stats_ops->txrx_host_ce_stats)
		return;

	soc->ops->host_stats_ops->txrx_host_ce_stats(vdev);
}

static inline int cdp_stats_publish
	(ol_txrx_soc_handle soc, struct cdp_pdev *pdev,
	struct cdp_stats_extd *buf)
{
	if (!soc || !soc->ops) {
		QDF_TRACE(QDF_MODULE_ID_CDP, QDF_TRACE_LEVEL_DEBUG,
				"%s: Invalid Instance", __func__);
		QDF_BUG(0);
		return 0;
	}

	if (!soc->ops->host_stats_ops ||
	    !soc->ops->host_stats_ops->txrx_stats_publish)
		return 0;

	return soc->ops->host_stats_ops->txrx_stats_publish(pdev, buf);
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
	if (!soc || !soc->ops) {
		QDF_TRACE(QDF_MODULE_ID_CDP, QDF_TRACE_LEVEL_DEBUG,
				"%s: Invalid Instance", __func__);
		QDF_BUG(0);
		return;
	}

	if (!soc->ops->host_stats_ops ||
	    !soc->ops->host_stats_ops->txrx_enable_enhanced_stats)
		return;

	soc->ops->host_stats_ops->txrx_enable_enhanced_stats
			(pdev);
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
	if (!soc || !soc->ops) {
		QDF_TRACE(QDF_MODULE_ID_CDP, QDF_TRACE_LEVEL_DEBUG,
				"%s: Invalid Instance", __func__);
		QDF_BUG(0);
		return;
	}

	if (!soc->ops->host_stats_ops ||
	    !soc->ops->host_stats_ops->txrx_disable_enhanced_stats)
		return;

	soc->ops->host_stats_ops->txrx_disable_enhanced_stats
			(pdev);
}

/**
 * @brief Get the desired stats from the message.
 *
 * @param pdev - the physical device object
 * @param stats_base - stats buffer received from FW
 * @param type - stats type.
 * @return - pointer to requested stat identified by type
 */
static inline uint32_t *cdp_get_stats_base
	(ol_txrx_soc_handle soc, struct cdp_pdev *pdev,
	uint32_t *stats_base, uint32_t msg_len, uint8_t type)
{
	if (!soc || !soc->ops) {
		QDF_TRACE(QDF_MODULE_ID_CDP, QDF_TRACE_LEVEL_DEBUG,
				"%s: Invalid Instance", __func__);
		QDF_BUG(0);
		return 0;
	}

	if (!soc->ops->host_stats_ops ||
	    !soc->ops->host_stats_ops->txrx_get_stats_base)
		return 0;

	return (uint32_t *)soc->ops->host_stats_ops->txrx_get_stats_base
			(pdev, stats_base, msg_len, type);
}

static inline void
cdp_tx_print_tso_stats(ol_txrx_soc_handle soc,
	struct cdp_vdev *vdev)
{
	if (!soc || !soc->ops) {
		QDF_TRACE(QDF_MODULE_ID_CDP, QDF_TRACE_LEVEL_DEBUG,
				"%s: Invalid Instance", __func__);
		QDF_BUG(0);
		return;
	}

	if (!soc->ops->host_stats_ops ||
	    !soc->ops->host_stats_ops->tx_print_tso_stats)
		return;

	soc->ops->host_stats_ops->tx_print_tso_stats(vdev);
}

static inline void
cdp_tx_rst_tso_stats(ol_txrx_soc_handle soc, struct cdp_vdev *vdev)
{
	if (!soc || !soc->ops) {
		QDF_TRACE(QDF_MODULE_ID_CDP, QDF_TRACE_LEVEL_DEBUG,
				"%s: Invalid Instance", __func__);
		QDF_BUG(0);
		return;
	}

	if (!soc->ops->host_stats_ops ||
	    !soc->ops->host_stats_ops->tx_rst_tso_stats)
		return;

	soc->ops->host_stats_ops->tx_rst_tso_stats(vdev);
}

static inline void
cdp_tx_print_sg_stats(ol_txrx_soc_handle soc,
	struct cdp_vdev *vdev)
{
	if (!soc || !soc->ops) {
		QDF_TRACE(QDF_MODULE_ID_CDP, QDF_TRACE_LEVEL_DEBUG,
				"%s: Invalid Instance", __func__);
		QDF_BUG(0);
		return;
	}

	if (!soc->ops->host_stats_ops ||
	    !soc->ops->host_stats_ops->tx_print_sg_stats)
		return;

	soc->ops->host_stats_ops->tx_print_sg_stats(vdev);
}

static inline void
cdp_tx_rst_sg_stats(ol_txrx_soc_handle soc, struct cdp_vdev *vdev)
{
	if (!soc || !soc->ops) {
		QDF_TRACE(QDF_MODULE_ID_CDP, QDF_TRACE_LEVEL_DEBUG,
				"%s: Invalid Instance", __func__);
		QDF_BUG(0);
		return;
	}

	if (!soc->ops->host_stats_ops ||
	    !soc->ops->host_stats_ops->tx_rst_sg_stats)
		return;

	soc->ops->host_stats_ops->tx_rst_sg_stats(vdev);
}

static inline void
cdp_print_rx_cksum_stats(ol_txrx_soc_handle soc,
	struct cdp_vdev *vdev)
{
	if (!soc || !soc->ops) {
		QDF_TRACE(QDF_MODULE_ID_CDP, QDF_TRACE_LEVEL_DEBUG,
				"%s: Invalid Instance", __func__);
		QDF_BUG(0);
		return;
	}

	if (!soc->ops->host_stats_ops ||
	    !soc->ops->host_stats_ops->print_rx_cksum_stats)
		return;

	soc->ops->host_stats_ops->print_rx_cksum_stats(vdev);
}

static inline void
cdp_rst_rx_cksum_stats(ol_txrx_soc_handle soc, struct cdp_vdev *vdev)
{
	if (!soc || !soc->ops) {
		QDF_TRACE(QDF_MODULE_ID_CDP, QDF_TRACE_LEVEL_DEBUG,
				"%s: Invalid Instance", __func__);
		QDF_BUG(0);
		return;
	}

	if (!soc->ops->host_stats_ops ||
	    !soc->ops->host_stats_ops->rst_rx_cksum_stats)
		return;

	soc->ops->host_stats_ops->rst_rx_cksum_stats(vdev);
}

static inline A_STATUS
cdp_host_me_stats(ol_txrx_soc_handle soc, struct cdp_vdev *vdev)
{
	if (!soc || !soc->ops) {
		QDF_TRACE(QDF_MODULE_ID_CDP, QDF_TRACE_LEVEL_DEBUG,
				"%s: Invalid Instance", __func__);
		QDF_BUG(0);
		return 0;
	}

	if (!soc->ops->host_stats_ops ||
	    !soc->ops->host_stats_ops->txrx_host_me_stats)
		return 0;

	return soc->ops->host_stats_ops->txrx_host_me_stats(vdev);
}

/**
 * cdp_per_peer_stats(): function to print per peer REO Queue stats
 * @soc: soc handle
 * @pdev: physical device
 * @addr: peer address
 *
 * return: status
 */
static inline void cdp_per_peer_stats(ol_txrx_soc_handle soc,
				      struct cdp_pdev *pdev, char *addr)
{
	if (!soc || !soc->ops) {
		QDF_TRACE(QDF_MODULE_ID_CDP, QDF_TRACE_LEVEL_DEBUG,
			  "%s: Invalid Instance", __func__);
		QDF_BUG(0);
		return;
	}

	if (!soc->ops->host_stats_ops ||
	    !soc->ops->host_stats_ops->txrx_per_peer_stats)
		return;

	soc->ops->host_stats_ops->txrx_per_peer_stats(pdev, addr);
}

static inline int cdp_host_msdu_ttl_stats(ol_txrx_soc_handle soc,
	struct cdp_vdev *vdev,
	struct ol_txrx_stats_req *req)
{
	if (!soc || !soc->ops) {
		QDF_TRACE(QDF_MODULE_ID_CDP, QDF_TRACE_LEVEL_DEBUG,
				"%s: Invalid Instance", __func__);
		QDF_BUG(0);
		return 0;
	}

	if (!soc->ops->host_stats_ops ||
	    !soc->ops->host_stats_ops->txrx_host_msdu_ttl_stats)
		return 0;

	return soc->ops->host_stats_ops->txrx_host_msdu_ttl_stats
			(vdev, req);
}

static inline void
cdp_print_lro_stats(ol_txrx_soc_handle soc, struct cdp_vdev *vdev)
{
	if (!soc || !soc->ops) {
		QDF_TRACE(QDF_MODULE_ID_CDP, QDF_TRACE_LEVEL_DEBUG,
				"%s: Invalid Instance", __func__);
		QDF_BUG(0);
		return;
	}

	if (!soc->ops->host_stats_ops ||
	    !soc->ops->host_stats_ops->print_lro_stats)
		return;

	soc->ops->host_stats_ops->print_lro_stats(vdev);
}

static inline void
cdp_reset_lro_stats(ol_txrx_soc_handle soc, struct cdp_vdev *vdev)
{
	if (!soc || !soc->ops) {
		QDF_TRACE(QDF_MODULE_ID_CDP, QDF_TRACE_LEVEL_DEBUG,
				"%s: Invalid Instance", __func__);
		QDF_BUG(0);
		return;
	}

	if (!soc->ops->host_stats_ops ||
	    !soc->ops->host_stats_ops->reset_lro_stats)
		return;

	soc->ops->host_stats_ops->reset_lro_stats(vdev);
}

static inline void cdp_get_dp_fw_peer_stats(ol_txrx_soc_handle soc,
		struct cdp_pdev *pdev, uint8_t *mac, uint32_t caps,
		uint32_t copy_stats)
{
	if (!soc || !soc->ops) {
		QDF_TRACE(QDF_MODULE_ID_CDP, QDF_TRACE_LEVEL_DEBUG,
				"%s: Invalid Instance", __func__);
		QDF_BUG(0);
		return;
	}

	if (!soc->ops->host_stats_ops ||
	    !soc->ops->host_stats_ops->get_fw_peer_stats)
		return;

	soc->ops->host_stats_ops->get_fw_peer_stats
			(pdev, mac, caps, copy_stats);
}

static inline void cdp_get_dp_htt_stats(ol_txrx_soc_handle soc,
					struct cdp_pdev *pdev,
					void *data, uint32_t data_len)
{
	if (soc && soc->ops && soc->ops->host_stats_ops &&
		soc->ops->host_stats_ops->get_htt_stats)
		return soc->ops->host_stats_ops->get_htt_stats
			(pdev, data, data_len);
	return;
}

/**
 * @brief Update pdev host stats received from firmware
 * (wmi_host_pdev_stats and wmi_host_pdev_ext_stats) into dp
 *
 * @param pdev - the physical device object
 * @param data - pdev stats
 * @return - void
 */
static inline void
cdp_update_pdev_host_stats(ol_txrx_soc_handle soc,
			   struct cdp_pdev *pdev,
			   void *data,
			   uint16_t stats_id)
{
	if (soc && soc->ops && soc->ops->host_stats_ops &&
	    soc->ops->host_stats_ops->txrx_update_pdev_stats)
		return soc->ops->host_stats_ops->txrx_update_pdev_stats
			(pdev, data, stats_id);
}

/**
 * @brief Update vdev host stats
 *
 * @param soc	   - soc handle
 * @param vdev     - the physical device object
 * @param data     - pdev stats
 * @param stats_id - type of stats
 *
 * @return - void
 */
static inline void
cdp_update_vdev_host_stats(ol_txrx_soc_handle soc,
			   struct cdp_vdev *vdev,
			   void *data,
			   uint16_t stats_id)
{
	if (!soc || !soc->ops) {
		QDF_TRACE(QDF_MODULE_ID_CDP, QDF_TRACE_LEVEL_DEBUG,
			  "%s: Invalid Instance", __func__);
		QDF_BUG(0);
		return;
	}

	if (!soc->ops->host_stats_ops ||
	    !soc->ops->host_stats_ops->txrx_update_vdev_stats)
		return;

	return soc->ops->host_stats_ops->txrx_update_vdev_stats(vdev, data,
								stats_id);
}

/**
 * @brief Call to get peer stats
 *
 * @param peer - dp peer object
 * @return - struct cdp_peer_stats
 */
static inline struct cdp_peer_stats *
cdp_host_get_peer_stats(ol_txrx_soc_handle soc, struct cdp_peer *peer)
{
	if (!soc || !soc->ops) {
		QDF_TRACE(QDF_MODULE_ID_CDP, QDF_TRACE_LEVEL_DEBUG,
			  "%s: Invalid Instance", __func__);
		QDF_BUG(0);
		return NULL;
	}

	if (!soc->ops->host_stats_ops ||
	    !soc->ops->host_stats_ops->txrx_get_peer_stats)
		return NULL;

	return soc->ops->host_stats_ops->txrx_get_peer_stats(peer);
}

/**
 * @brief Call to reset ald stats
 *
 * @param peer - dp peer object
 * @return - void
 */
static inline void
cdp_host_reset_peer_ald_stats(ol_txrx_soc_handle soc,
			      struct cdp_peer *peer)
{
	if (!soc || !soc->ops) {
		QDF_TRACE(QDF_MODULE_ID_CDP, QDF_TRACE_LEVEL_DEBUG,
			  "%s: Invalid Instance", __func__);
		QDF_BUG(0);
		return;
	}

	if (!soc->ops->host_stats_ops ||
	    !soc->ops->host_stats_ops->txrx_reset_peer_ald_stats)
		return;

	return soc->ops->host_stats_ops->txrx_reset_peer_ald_stats(peer);
}

/**
 * @brief Call to reset peer stats
 *
 * @param peer - dp peer object
 * @return - void
 */
static inline void
cdp_host_reset_peer_stats(ol_txrx_soc_handle soc,
			  struct cdp_peer *peer)
{
	if (!soc || !soc->ops) {
		QDF_TRACE(QDF_MODULE_ID_CDP, QDF_TRACE_LEVEL_DEBUG,
			  "%s: Invalid Instance", __func__);
		QDF_BUG(0);
		return;
	}

	if (!soc->ops->host_stats_ops ||
	    !soc->ops->host_stats_ops->txrx_reset_peer_stats)
		return;

	return soc->ops->host_stats_ops->txrx_reset_peer_stats(peer);
}

/**
 * @brief Call to get vdev stats
 *
 * @param vdev - dp vdev object
 * @param buf - buffer
 * @return - int
 */
static inline int
cdp_host_get_vdev_stats(ol_txrx_soc_handle soc,
			struct cdp_vdev *vdev,
			struct cdp_vdev_stats *buf,
			bool is_aggregate)
{
	if (!soc || !soc->ops) {
		QDF_TRACE(QDF_MODULE_ID_CDP, QDF_TRACE_LEVEL_DEBUG,
			  "%s: Invalid Instance", __func__);
		QDF_BUG(0);
		return 0;
	}

	if (!soc->ops->host_stats_ops ||
	    !soc->ops->host_stats_ops->txrx_get_vdev_stats)
		return 0;

	return soc->ops->host_stats_ops->txrx_get_vdev_stats(vdev,
							     buf,
							     is_aggregate);
}

/**
 * @brief Call to update vdev stats received from firmware
 * (wmi_host_vdev_stats and wmi_host_vdev_extd_stats) into dp
 *
 * @param data - stats data to be updated
 * @param size - size of stats data
 * @param stats_id - stats id
 * @return - int
 */
static inline int
cdp_update_host_vdev_stats(ol_txrx_soc_handle soc,
			   void *data,
			   uint32_t size,
			   uint32_t stats_id)
{
	if (!soc || !soc->ops) {
		QDF_TRACE(QDF_MODULE_ID_CDP, QDF_TRACE_LEVEL_DEBUG,
			  "%s: Invalid Instance", __func__);
		QDF_BUG(0);
		return 0;
	}

	if (!soc->ops->host_stats_ops ||
	    !soc->ops->host_stats_ops->txrx_process_wmi_host_vdev_stats)
		return 0;

	return soc->ops->host_stats_ops->txrx_process_wmi_host_vdev_stats
								(soc,
								 data,
								 size,
								 stats_id);
}

/**
 * @brief Call to get vdev extd stats
 *
 * @param vdev - dp vdev object
 * @param buf - buffer
 * @return - int
 */
static inline int
cdp_get_vdev_extd_stats(ol_txrx_soc_handle soc,
			struct cdp_vdev *vdev,
			void *buf)
{
	if (!soc || !soc->ops) {
		QDF_TRACE(QDF_MODULE_ID_CDP, QDF_TRACE_LEVEL_DEBUG,
			  "%s: Invalid Instance", __func__);
		QDF_BUG(0);
		return 0;
	}

	if (!soc->ops->host_stats_ops ||
	    !soc->ops->host_stats_ops->txrx_get_vdev_extd_stats)
		return 0;

	return soc->ops->host_stats_ops->txrx_get_vdev_extd_stats(vdev, buf);
}

/**
 * @brief Call to get cdp_pdev_stats
 *
 * @param pdev - dp pdev object
 * @return - cdp_pdev_stats
 */
static inline struct cdp_pdev_stats*
cdp_host_get_pdev_stats(ol_txrx_soc_handle soc,
			struct cdp_pdev *pdev)
{
	if (!soc || !soc->ops) {
		QDF_TRACE(QDF_MODULE_ID_CDP, QDF_TRACE_LEVEL_DEBUG,
			  "%s: Invalid Instance", __func__);
		QDF_BUG(0);
		return 0;
	}

	if (!soc->ops->host_stats_ops ||
	    !soc->ops->host_stats_ops->txrx_get_pdev_stats)
		return 0;

	return soc->ops->host_stats_ops->txrx_get_pdev_stats(pdev);
}

/**
 * @brief Call to get radio stats
 *
 * @param pdev - dp pdev object
 * @param scn_stats_user - stats buffer
 * @return - int
 */
static inline int
cdp_host_get_radio_stats(ol_txrx_soc_handle soc,
			 struct cdp_pdev *pdev,
			 void *buf)
{
	if (!soc || !soc->ops) {
		QDF_TRACE(QDF_MODULE_ID_CDP, QDF_TRACE_LEVEL_DEBUG,
			  "%s: Invalid Instance", __func__);
		QDF_BUG(0);
		return 0;
	}

	if (!soc->ops->host_stats_ops ||
	    !soc->ops->host_stats_ops->txrx_get_radio_stats)
		return 0;

	return soc->ops->host_stats_ops->txrx_get_radio_stats(pdev,
							      buf);
}

/**
 * @brief confgure rate stats at soc
 *
 * @param soc - opaque soc handle
 * @param vap - capabilities
 * @return - void
 */
static inline void
cdp_soc_configure_rate_stats(ol_txrx_soc_handle soc, uint8_t val)
{
	if (!soc || !soc->ops) {
		QDF_TRACE(QDF_MODULE_ID_CDP, QDF_TRACE_LEVEL_DEBUG,
			  "%s: Invalid Instance", __func__);
		QDF_BUG(0);
		return;
	}

	if (!soc->ops->host_stats_ops ||
	    !soc->ops->host_stats_ops->configure_rate_stats)
		return;

	return soc->ops->host_stats_ops->configure_rate_stats(soc, val);
}

/**
 * @brief Parse the stats header and get the payload from the message.
 *
 * @param pdev - the physical device object
 * @param msg_word - stats buffer received from FW
 * @param msg_len - length of the message
 * @param type - place holder for parsed message type
 * @param status - place holder for parsed message status
 * @return - pointer to received stat payload
 */

#if defined(QCA_SUPPORT_SON) || defined(ENHANCED_STATS)
uint32_t *ol_txrx_get_en_stats_base(struct cdp_pdev *pdev, uint32_t *msg_word,
    uint32_t msg_len, enum htt_cmn_t2h_en_stats_type *type,  enum htt_cmn_t2h_en_stats_status *status);
#endif
#endif /* _CDP_TXRX_HOST_STATS_H_ */
