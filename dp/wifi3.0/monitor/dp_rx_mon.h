/*
 * Copyright (c) 2016-2021 The Linux Foundation. All rights reserved.
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

#ifndef _DP_RX_MON_H_
#define _DP_RX_MON_H_

#define dp_rx_mon_status_alert(params...) QDF_TRACE_FATAL(QDF_MODULE_ID_DP_RX_MON_STATUS, params)
#define dp_rx_mon_status_err(params...) QDF_TRACE_ERROR(QDF_MODULE_ID_DP_RX_MON_STATUS, params)
#define dp_rx_mon_status_warn(params...) QDF_TRACE_WARN(QDF_MODULE_ID_DP_RX_MON_STATUS, params)
#define dp_rx_mon_status_info(params...) \
	__QDF_TRACE_FL(QDF_TRACE_LEVEL_INFO_HIGH, QDF_MODULE_ID_DP_RX_MON_STATUS, ## params)
#define dp_rx_mon_status_debug(params...) QDF_TRACE_DEBUG(QDF_MODULE_ID_DP_RX_MON_STATUS, params)

#define dp_rx_mon_dest_alert(params...) QDF_TRACE_FATAL(QDF_MODULE_ID_DP_RX_MON_DEST, params)
#define dp_rx_mon_dest_err(params...) QDF_TRACE_ERROR(QDF_MODULE_ID_DP_RX_MON_DEST, params)
#define dp_rx_mon_dest_warn(params...) QDF_TRACE_WARN(QDF_MODULE_ID_DP_RX_MON_DEST, params)
#define dp_rx_mon_dest_info(params...) \
	__QDF_TRACE_FL(QDF_TRACE_LEVEL_INFO_HIGH, QDF_MODULE_ID_DP_RX_MON_DEST, ## params)
#define dp_rx_mon_dest_debug(params...) QDF_TRACE_DEBUG(QDF_MODULE_ID_DP_RX_MON_DEST, params)

/* The maximum buffer length allocated for radiotap for monitor status buffer */
#define MAX_MONITOR_HEADER (512)

/**
 * enum dp_mon_reap_status - monitor status ring ppdu status
 *
 * @DP_MON_STATUS_NO_DMA - DMA not done for status ring entry
 * @DP_MON_STATUS_MATCH - status and dest ppdu id mathes
 * @DP_MON_STATUS_LAG - status ppdu id is lagging
 * @DP_MON_STATUS_LEAD - status ppdu id is leading
 * @DP_MON_STATUS_REPLENISH - status ring entry is NULL
 */
enum dp_mon_reap_status {
	DP_MON_STATUS_NO_DMA,
	DP_MON_STATUS_MATCH,
	DP_MON_STATUS_LAG,
	DP_MON_STATUS_LEAD,
	DP_MON_STATUS_REPLENISH
};

/*
 * dp_rx_mon_status_process() - Process monitor status ring and
 *			TLV in status ring.
 *
 * @soc: core txrx main context
 * @int_ctx: interrupt context
 * @mac_id: mac_id which is one of 3 mac_ids
 * @quota: No. of ring entry that can be serviced in one shot.

 * Return: uint32_t: No. of ring entry that is processed.
 */
uint32_t
dp_rx_mon_status_process(struct dp_soc *soc, struct dp_intr *int_ctx,
			 uint32_t mac_id, uint32_t quota);

/*
 * dp_rx_populate_cbf_hdr - Send CBF frame with htt header
 * @soc: Datapath soc handle
 * @mac_id: Datapath mac id
 * @event: WDI event
 * @mpdu: mpdu buffer
 * @msdu_timesstamp: time stamp
 *
 * Return: QDF_STATUS
 */
QDF_STATUS dp_rx_populate_cbf_hdr(struct dp_soc *soc,
				  uint32_t mac_id, uint32_t event,
				  qdf_nbuf_t data, uint32_t msdu_timestamp);

/**
 * dp_rx_mon_handle_status_buf_done () - Handle DMA not done case for
 * monitor status ring
 *
 * @pdev: DP pdev handle
 * @mon_status_srng: Monitor status SRNG
 *
 * Return: enum dp_mon_reap_status
 */
enum dp_mon_reap_status
dp_rx_mon_handle_status_buf_done(struct dp_pdev *pdev,
				 void *mon_status_srng);

#ifdef QCA_SUPPORT_FULL_MON

/**
 * dp_full_mon_attach() - Full monitor mode attach
 * This API initilises full monitor mode resources
 *
 * @pdev: dp pdev object
 *
 * Return: void
 *
 */
void dp_full_mon_attach(struct dp_pdev *pdev);

/**
 * dp_full_mon_detach() - Full monitor mode attach
 * This API deinitilises full monitor mode resources
 *
 * @pdev: dp pdev object
 *
 * Return: void
 *
 */
void dp_full_mon_detach(struct dp_pdev *pdev);

/**
 * dp_rx_mon_process ()- API to process monitor destination ring for
 * full monitor mode
 *
 * @soc: dp soc handle
 * @int_ctx: interrupt context
 * @mac_id: lmac id
 * @quota: No. of ring entry that can be serviced in one shot.
 */

uint32_t dp_rx_mon_process(struct dp_soc *soc, struct dp_intr *int_ctx,
			   uint32_t mac_id, uint32_t quota);

#else
/**
 * dp_full_mon_attach() - attach full monitor mode resources
 * @pdev: Datapath PDEV handle
 *
 * Return: void
 */
static inline void dp_full_mon_attach(struct dp_pdev *pdev)
{
}

/**
 * dp_full_mon_detach() - detach full monitor mode resources
 * @pdev: Datapath PDEV handle
 *
 * Return: void
 *
 */
static inline void dp_full_mon_detach(struct dp_pdev *pdev)
{
}
#endif

/**
 * dp_mon_link_free() - free monitor link desc pool
 * @pdev: core txrx pdev context
 *
 * This function will release DP link desc pool for monitor mode from
 * main device context.
 *
 * Return: QDF_STATUS_SUCCESS: success
 *         QDF_STATUS_E_RESOURCES: Error return
 */
QDF_STATUS dp_mon_link_free(struct dp_pdev *pdev);


/**
 * dp_mon_process() - Main monitor mode processing roution.
 * @soc: core txrx main context
 * @int_ctx: interrupt context
 * @mac_id: mac_id which is one of 3 mac_ids
 * @quota: No. of status ring entry that can be serviced in one shot.
 *
 * This call monitor status ring process then monitor
 * destination ring process.
 * Called from the bottom half (tasklet/NET_RX_SOFTIRQ)
 *
 * Return: uint32_t: No. of ring entry that is processed.
 */
uint32_t dp_mon_process(struct dp_soc *soc, struct dp_intr *int_ctx,
			uint32_t mac_id, uint32_t quota);

QDF_STATUS dp_rx_mon_deliver(struct dp_soc *soc, uint32_t mac_id,
	qdf_nbuf_t head_msdu, qdf_nbuf_t tail_msdu);
/*
 * dp_rx_mon_deliver_non_std() - deliver frames for non standard path
 * @soc: core txrx main contex
 * @mac_id: MAC ID
 *
 * This function delivers the radio tap and dummy MSDU
 * into user layer application for preamble only PPDU.
 *
 * Return: Operation status
 */
QDF_STATUS dp_rx_mon_deliver_non_std(struct dp_soc *soc, uint32_t mac_id);

#ifndef REMOVE_MON_DBG_STATS
/*
 * dp_rx_mon_update_dbg_ppdu_stats() - Update status ring TLV count
 * @ppdu_info: HAL RX PPDU info retrieved from status ring TLV
 * @rx_mon_stats: monitor mode status/destination ring PPDU and MPDU count
 *
 * Update status ring PPDU start and end count. Keep track TLV state on
 * PPDU start and end to find out if start and end is matching. Keep
 * track missing PPDU start and end count. Keep track matching PPDU
 * start and end count.
 *
 * Return: None
 */
static inline void
dp_rx_mon_update_dbg_ppdu_stats(struct hal_rx_ppdu_info *ppdu_info,
				struct cdp_pdev_mon_stats *rx_mon_stats)
{
	if (ppdu_info->rx_state ==
		HAL_RX_MON_PPDU_START) {
		rx_mon_stats->status_ppdu_start++;
		if (rx_mon_stats->status_ppdu_state
			!= CDP_MON_PPDU_END)
			rx_mon_stats->status_ppdu_end_mis++;
		rx_mon_stats->status_ppdu_state
			= CDP_MON_PPDU_START;
		ppdu_info->rx_state = HAL_RX_MON_PPDU_RESET;
	} else if (ppdu_info->rx_state ==
		HAL_RX_MON_PPDU_END) {
		rx_mon_stats->status_ppdu_end++;
		if (rx_mon_stats->status_ppdu_state
			!= CDP_MON_PPDU_START)
			rx_mon_stats->status_ppdu_start_mis++;
		else
			rx_mon_stats->status_ppdu_compl++;
		rx_mon_stats->status_ppdu_state
			= CDP_MON_PPDU_END;
		ppdu_info->rx_state = HAL_RX_MON_PPDU_RESET;
	}
}

/*
 * dp_rx_mon_init_dbg_ppdu_stats() - initialization for monitor mode stats
 * @ppdu_info: HAL RX PPDU info retrieved from status ring TLV
 * @rx_mon_stats: monitor mode status/destination ring PPDU and MPDU count
 *
 * Return: None
 */
static inline void
dp_rx_mon_init_dbg_ppdu_stats(struct hal_rx_ppdu_info *ppdu_info,
			      struct cdp_pdev_mon_stats *rx_mon_stats)
{
	ppdu_info->rx_state = HAL_RX_MON_PPDU_END;
	rx_mon_stats->status_ppdu_state
		= CDP_MON_PPDU_END;
}

#else
static inline void
dp_rx_mon_update_dbg_ppdu_stats(struct hal_rx_ppdu_info *ppdu_info,
				struct cdp_pdev_mon_stats *rx_mon_stats)
{
}

static inline void
dp_rx_mon_init_dbg_ppdu_stats(struct hal_rx_ppdu_info *ppdu_info,
			      struct cdp_pdev_mon_stats *rx_mon_stats)
{
}

#endif

#ifdef QCA_ENHANCED_STATS_SUPPORT
void
dp_rx_populate_rx_rssi_chain(struct hal_rx_ppdu_info *ppdu_info,
			     struct cdp_rx_indication_ppdu *cdp_rx_ppdu);

/*
 * dp_rx_populate_su_evm_details() - Populate su evm info
 * @ppdu_info: ppdu info structure from ppdu ring
 * @cdp_rx_ppdu: rx ppdu indication structure
 */
void
dp_rx_populate_su_evm_details(struct hal_rx_ppdu_info *ppdu_info,
			      struct cdp_rx_indication_ppdu *cdp_rx_ppdu);

/**
* dp_rx_handle_ppdu_stats() - Allocate and deliver ppdu stats to cdp layer
* @soc: core txrx main context
* @pdev: pdev strcuture
* @ppdu_info: structure for rx ppdu ring
*
* Return: none
*/
void
dp_rx_handle_ppdu_stats(struct dp_soc *soc, struct dp_pdev *pdev,
			struct hal_rx_ppdu_info *ppdu_info);
#else
static inline void
dp_rx_populate_rx_rssi_chain(struct hal_rx_ppdu_info *ppdu_info,
			     struct cdp_rx_indication_ppdu *cdp_rx_ppdu)
{
}

static inline void
dp_rx_populate_su_evm_details(struct hal_rx_ppdu_info *ppdu_info,
			      struct cdp_rx_indication_ppdu *cdp_rx_ppdu)
{
}

static inline void
dp_rx_populate_cdp_indication_ppdu_user(struct dp_pdev *pdev,
					struct hal_rx_ppdu_info *ppdu_info,
					struct cdp_rx_indication_ppdu
					*cdp_rx_ppdu)
{
}

static inline void
dp_rx_populate_cdp_indication_ppdu(struct dp_pdev *pdev,
				   struct hal_rx_ppdu_info *ppdu_info,
				   struct cdp_rx_indication_ppdu *cdp_rx_ppdu)
{
}

static inline void dp_rx_rate_stats_update(struct dp_peer *peer,
					   struct cdp_rx_indication_ppdu *ppdu,
					   uint32_t user)
{
}

static inline void
dp_rx_handle_ppdu_stats(struct dp_soc *soc, struct dp_pdev *pdev,
			struct hal_rx_ppdu_info *ppdu_info)
{
}
#endif /* QCA_ENHANCED_STATS_SUPPORT */

#ifdef QCA_MCOPY_SUPPORT
/**
 * dp_rx_handle_mcopy_mode() - Allocate and deliver first MSDU payload
 * @soc: core txrx main context
 * @pdev: pdev structure
 * @ppdu_info: structure for rx ppdu ring
 * @nbuf: QDF nbuf
 * @fcs_ok_mpdu_cnt: fcs passsed mpdu index
 * @deliver_frame: flag to deliver wdi event
 *
 * Return: QDF_STATUS_SUCCESS - If nbuf to be freed by caller
 *         QDF_STATUS_E_ALREADY - If nbuf not to be freed by caller
 */
QDF_STATUS
dp_rx_handle_mcopy_mode(struct dp_soc *soc, struct dp_pdev *pdev,
			struct hal_rx_ppdu_info *ppdu_info, qdf_nbuf_t nbuf,
			uint8_t fcs_ok_mpdu_cnt, bool deliver_frame);

/**
 * dp_rx_mcopy_handle_last_mpdu() - cache and delive last MPDU header in a
 * status buffer if MPDU end tlv is received in different buffer
 * @soc: core txrx main context
 * @pdev: pdev structure
 * @ppdu_info: structure for rx ppdu ring
 * @status_nbuf: QDF nbuf
 *
 * Return: void
 */
void
dp_rx_mcopy_handle_last_mpdu(struct dp_soc *soc, struct dp_pdev *pdev,
			     struct hal_rx_ppdu_info *ppdu_info,
			     qdf_nbuf_t status_nbuf);

/**
 * dp_rx_mcopy_process_ppdu_info() - update mcopy ppdu info
 * @ppdu_info: structure for rx ppdu ring
 * @tlv_status: processed TLV status
 *
 * Return: void
 */
void
dp_rx_mcopy_process_ppdu_info(struct dp_pdev *pdev,
			      struct hal_rx_ppdu_info *ppdu_info,
			      uint32_t tlv_status);

void
dp_rx_process_mcopy_mode(struct dp_soc *soc, struct dp_pdev *pdev,
			 struct hal_rx_ppdu_info *ppdu_info,
			 uint32_t tlv_status,
			 qdf_nbuf_t status_nbuf);
#else
static inline QDF_STATUS
dp_rx_handle_mcopy_mode(struct dp_soc *soc, struct dp_pdev *pdev,
			struct hal_rx_ppdu_info *ppdu_info, qdf_nbuf_t nbuf,
			uint8_t fcs_ok_cnt, bool deliver_frame)
{
	return QDF_STATUS_SUCCESS;
}

static inline void
dp_rx_mcopy_handle_last_mpdu(struct dp_soc *soc, struct dp_pdev *pdev,
			     struct hal_rx_ppdu_info *ppdu_info,
			     qdf_nbuf_t status_nbuf)
{
}

static inline void
dp_rx_mcopy_process_ppdu_info(struct dp_pdev *pdev,
			      struct hal_rx_ppdu_info *ppdu_info,
			      uint32_t tlv_status)
{
}

static inline void
dp_rx_process_mcopy_mode(struct dp_soc *soc, struct dp_pdev *pdev,
			 struct hal_rx_ppdu_info *ppdu_info,
			 uint32_t tlv_status,
			 qdf_nbuf_t status_nbuf)
{
}
#endif /* QCA_MCOPY_SUPPORT */

/**
 * dp_rx_handle_smart_mesh_mode() - Deliver header for smart mesh
 * @soc: Datapath SOC handle
 * @pdev: Datapath PDEV handle
 * @ppdu_info: Structure for rx ppdu info
 * @nbuf: Qdf nbuf abstraction for linux skb
 *
 * Return: 0 on success, 1 on failure
 */
int
dp_rx_handle_smart_mesh_mode(struct dp_soc *soc, struct dp_pdev *pdev,
			      struct hal_rx_ppdu_info *ppdu_info,
			      qdf_nbuf_t nbuf);
/*
 * dp_rx_nbuf_prepare() - prepare RX nbuf
 * @soc: core txrx main context
 * @pdev: core txrx pdev context
 *
 * This function alloc & map nbuf for RX dma usage, retry it if failed
 * until retry times reaches max threshold or succeeded.
 *
 * Return: qdf_nbuf_t pointer if succeeded, NULL if failed.
 */
qdf_nbuf_t
dp_rx_nbuf_prepare(struct dp_soc *soc, struct dp_pdev *pdev);

#if defined(WLAN_CFR_ENABLE) && defined(WLAN_ENH_CFR_ENABLE)

/*
 * dp_rx_mon_handle_cfr_mu_info() - Gather macaddr and ast_index of peer(s) in
 * the PPDU received, this will be used for correlation of CFR data captured
 * for an UL-MU-PPDU
 * @pdev: pdev ctx
 * @ppdu_info: pointer to ppdu info structure populated from ppdu status TLVs
 * @cdp_rx_ppdu: Rx PPDU indication structure
 *
 * Return: none
 */
void
dp_rx_mon_handle_cfr_mu_info(struct dp_pdev *pdev,
			     struct hal_rx_ppdu_info *ppdu_info,
			     struct cdp_rx_indication_ppdu *cdp_rx_ppdu);

/*
 * dp_rx_mon_populate_cfr_ppdu_info() - Populate cdp ppdu info from hal ppdu
 * info
 * @pdev: pdev ctx
 * @ppdu_info: ppdu info structure from ppdu ring
 * @cdp_rx_ppdu : Rx PPDU indication structure
 *
 * Return: none
 */
void
dp_rx_mon_populate_cfr_ppdu_info(struct dp_pdev *pdev,
				 struct hal_rx_ppdu_info *ppdu_info,
				 struct cdp_rx_indication_ppdu *cdp_rx_ppdu);

/**
 * dp_cfr_rcc_mode_status() - Return status of cfr rcc mode
 * @pdev: pdev ctx
 *
 * Return: True or False
 */
bool
dp_cfr_rcc_mode_status(struct dp_pdev *pdev);

/*
 * dp_rx_mon_populate_cfr_info() - Populate cdp ppdu info from hal cfr info
 * @pdev: pdev ctx
 * @ppdu_info: ppdu info structure from ppdu ring
 * @cdp_rx_ppdu: Rx PPDU indication structure
 *
 * Return: none
 */
void
dp_rx_mon_populate_cfr_info(struct dp_pdev *pdev,
			    struct hal_rx_ppdu_info *ppdu_info,
			    struct cdp_rx_indication_ppdu *cdp_rx_ppdu);

/**
 * dp_update_cfr_dbg_stats() - Increment RCC debug statistics
 * @pdev: pdev structure
 * @ppdu_info: structure for rx ppdu ring
 *
 * Return: none
 */
void
dp_update_cfr_dbg_stats(struct dp_pdev *pdev,
			struct hal_rx_ppdu_info *ppdu_info);

/*
 * dp_rx_handle_cfr() - Gather cfr info from hal ppdu info
 * @soc: core txrx main context
 * @pdev: pdev ctx
 * @ppdu_info: ppdu info structure from ppdu ring
 *
 * Return: none
 */
void
dp_rx_handle_cfr(struct dp_soc *soc, struct dp_pdev *pdev,
		 struct hal_rx_ppdu_info *ppdu_info);

/**
 * dp_rx_populate_cfr_non_assoc_sta() - Populate cfr ppdu info for PPDUs from
 * non-associated stations
 * @pdev: pdev ctx
 * @ppdu_info: ppdu info structure from ppdu ring
 * @cdp_rx_ppdu: Rx PPDU indication structure
 *
 * Return: none
 */
void
dp_rx_populate_cfr_non_assoc_sta(struct dp_pdev *pdev,
				 struct hal_rx_ppdu_info *ppdu_info,
				 struct cdp_rx_indication_ppdu *cdp_rx_ppdu);

#else
static inline void
dp_rx_mon_handle_cfr_mu_info(struct dp_pdev *pdev,
			     struct hal_rx_ppdu_info *ppdu_info,
			     struct cdp_rx_indication_ppdu *cdp_rx_ppdu)
{
}

static inline void
dp_rx_mon_populate_cfr_ppdu_info(struct dp_pdev *pdev,
				 struct hal_rx_ppdu_info *ppdu_info,
				 struct cdp_rx_indication_ppdu *cdp_rx_ppdu)
{
}

static inline void
dp_rx_mon_populate_cfr_info(struct dp_pdev *pdev,
			    struct hal_rx_ppdu_info *ppdu_info,
			    struct cdp_rx_indication_ppdu *cdp_rx_ppdu)
{
}

static inline void
dp_rx_handle_cfr(struct dp_soc *soc, struct dp_pdev *pdev,
		 struct hal_rx_ppdu_info *ppdu_info)
{
}

static inline void
dp_rx_populate_cfr_non_assoc_sta(struct dp_pdev *pdev,
				 struct hal_rx_ppdu_info *ppdu_info,
				 struct cdp_rx_indication_ppdu *cdp_rx_ppdu)
{
}

static inline void
dp_update_cfr_dbg_stats(struct dp_pdev *pdev,
			struct hal_rx_ppdu_info *ppdu_info)
{
}

static inline bool
dp_cfr_rcc_mode_status(struct dp_pdev *pdev)
{
	return false;
}
#endif /* WLAN_CFR_ENABLE && WLAN_ENH_CFR_ENABLE */

/*
 * dp_rx_mon_deliver(): function to deliver packets to stack
 * @soc: DP soc
 * @mac_id: MAC ID
 * @head_msdu: head of msdu list
 * @tail_msdu: tail of msdu list
 *
 * Return: status: 0 - Success, non-zero: Failure
 */
QDF_STATUS dp_rx_mon_deliver(struct dp_soc *soc,
			     uint32_t mac_id,
			     qdf_nbuf_t head_msdu,
			     qdf_nbuf_t tail_msdu);

/**
* dp_rx_mon_deliver_non_std()
* @soc: core txrx main contex
* @mac_id: MAC ID
*
* This function delivers the radio tap and dummy MSDU
* into user layer application for preamble only PPDU.
*
* Return: QDF_STATUS
*/
QDF_STATUS dp_rx_mon_deliver_non_std(struct dp_soc *soc,
				     uint32_t mac_id);

#ifdef DP_RX_MON_MEM_FRAG
#if defined(WLAN_SUPPORT_RX_PROTOCOL_TYPE_TAG) ||\
	defined(WLAN_SUPPORT_RX_FLOW_TAG)
void dp_rx_mon_update_pf_tag_to_buf_headroom(struct dp_soc *soc,
					     qdf_nbuf_t nbuf);
#else
static inline
void dp_rx_mon_update_pf_tag_to_buf_headroom(struct dp_soc *soc,
					     qdf_nbuf_t nbuf)
{
}
#endif
#else
static inline
void dp_rx_mon_update_pf_tag_to_buf_headroom(struct dp_soc *soc,
					     qdf_nbuf_t nbuf)
{
}
#endif

qdf_nbuf_t dp_rx_mon_restitch_mpdu(struct dp_soc *soc, uint32_t mac_id,
				   qdf_nbuf_t head_msdu, qdf_nbuf_t tail_msdu,
				   struct cdp_mon_status *rs);

#ifdef DP_RX_MON_MEM_FRAG
/**
 * dp_rx_mon_get_nbuf_80211_hdr() - Get 80211 hdr from nbuf
 * @nbuf: qdf_nbuf_t
 *
 * This function must be called after moving radiotap header.
 *
 * Return: Ptr pointing to 80211 header or NULL.
 */
static inline
qdf_frag_t dp_rx_mon_get_nbuf_80211_hdr(qdf_nbuf_t nbuf)
{
	/* Return NULL if nr_frag is Zero */
	if (!qdf_nbuf_get_nr_frags(nbuf))
		return NULL;

	return qdf_nbuf_get_frag_addr(nbuf, 0);
}
#else
static inline
qdf_frag_t dp_rx_mon_get_nbuf_80211_hdr(qdf_nbuf_t nbuf)
{
	return qdf_nbuf_data(nbuf);
}
#endif

/*
 * dp_rx_mon_process_dest_pktlog(): function to log packet contents to
 * pktlog buffer and send to pktlog module
 * @soc: DP soc
 * @mac_id: MAC ID
 * @mpdu: MPDU buf
 * Return: status: 0 - Success, non-zero: Failure
 */
QDF_STATUS dp_rx_mon_process_dest_pktlog(struct dp_soc *soc,
					 uint32_t mac_id,
					 qdf_nbuf_t mpdu);

#ifdef WLAN_TX_PKT_CAPTURE_ENH
void
dp_handle_tx_capture(struct dp_soc *soc, struct dp_pdev *pdev,
		     qdf_nbuf_t mon_mpdu);
#else
static inline void
dp_handle_tx_capture(struct dp_soc *soc, struct dp_pdev *pdev,
		     qdf_nbuf_t mon_mpdu)
{
}
#endif

/**
 * dp_rx_get_mon_desc_pool() - Return monitor descriptor pool
 *			       based on target
 * @soc: soc handle
 * @mac_id: mac id number
 * @pdev_id: pdev id number
 *
 * Return: descriptor pool address
 */
static inline
struct rx_desc_pool *dp_rx_get_mon_desc_pool(struct dp_soc *soc,
					     uint8_t mac_id,
					     uint8_t pdev_id)
{
	if (soc->wlan_cfg_ctx->rxdma1_enable)
		return &soc->rx_desc_mon[mac_id];

	return &soc->rx_desc_buf[pdev_id];
}

/**
 * dp_rx_process_peer_based_pktlog() - Process Rx pktlog if peer based
 *                                     filtering enabled
 * @soc: core txrx main context
 * @ppdu_info: Structure for rx ppdu info
 * @status_nbuf: Qdf nbuf abstraction for linux skb
 * @pdev_id: mac_id/pdev_id correspondinggly for MCL and WIN
 *
 * Return: none
 */
void
dp_rx_process_peer_based_pktlog(struct dp_soc *soc,
				struct hal_rx_ppdu_info *ppdu_info,
				qdf_nbuf_t status_nbuf, uint32_t pdev_id);
#endif /* _DP_RX_MON_H_ */
