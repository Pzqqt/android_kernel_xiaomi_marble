/*
 * Copyright (c) 2016-2021 The Linux Foundation. All rights reserved.
 * Copyright (c) 2021-2022 Qualcomm Innovation Center, Inc. All rights reserved.
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

#include "cdp_txrx_cmn_struct.h"
#include "dp_types.h"
#include "dp_tx.h"
#include "dp_be_tx.h"
#include "dp_tx_desc.h"
#include "hal_tx.h"
#include <hal_be_api.h>
#include <hal_be_tx.h>
#include <dp_htt.h>
#ifdef FEATURE_WDS
#include "dp_txrx_wds.h"
#endif

#if defined(WLAN_MAX_PDEVS) && (WLAN_MAX_PDEVS == 1)
#define DP_TX_BANK_LOCK_CREATE(lock) qdf_mutex_create(lock)
#define DP_TX_BANK_LOCK_DESTROY(lock) qdf_mutex_destroy(lock)
#define DP_TX_BANK_LOCK_ACQUIRE(lock) qdf_mutex_acquire(lock)
#define DP_TX_BANK_LOCK_RELEASE(lock) qdf_mutex_release(lock)
#else
#define DP_TX_BANK_LOCK_CREATE(lock) qdf_spinlock_create(lock)
#define DP_TX_BANK_LOCK_DESTROY(lock) qdf_spinlock_destroy(lock)
#define DP_TX_BANK_LOCK_ACQUIRE(lock) qdf_spin_lock_bh(lock)
#define DP_TX_BANK_LOCK_RELEASE(lock) qdf_spin_unlock_bh(lock)
#endif

#define DP_TX_WBM_COMPLETION_V3_VDEV_ID_GET(_var) \
	HTT_TX_WBM_COMPLETION_V2_VDEV_ID_GET(_var)
#define DP_TX_WBM_COMPLETION_V3_VALID_GET(_var) \
	HTT_TX_WBM_COMPLETION_V2_VALID_GET(_var)
#define DP_TX_WBM_COMPLETION_V3_SW_PEER_ID_GET(_var) \
	HTT_TX_WBM_COMPLETION_V2_SW_PEER_ID_GET(_var)
#define DP_TX_WBM_COMPLETION_V3_TID_NUM_GET(_var) \
	HTT_TX_WBM_COMPLETION_V2_TID_NUM_GET(_var)
#define DP_TX_WBM_COMPLETION_V3_SCH_CMD_ID_GET(_var) \
	HTT_TX_WBM_COMPLETION_V2_SCH_CMD_ID_GET(_var)
#define DP_TX_WBM_COMPLETION_V3_ACK_FRAME_RSSI_GET(_var) \
	HTT_TX_WBM_COMPLETION_V2_ACK_FRAME_RSSI_GET(_var)

extern uint8_t sec_type_map[MAX_CDP_SEC_TYPE];

#ifdef DP_USE_REDUCED_PEER_ID_FIELD_WIDTH
static inline uint16_t dp_tx_comp_get_peer_id(struct dp_soc *soc,
					      void *tx_comp_hal_desc)
{
	uint16_t peer_id = hal_tx_comp_get_peer_id(tx_comp_hal_desc);
	struct dp_tx_comp_peer_id *tx_peer_id =
			(struct dp_tx_comp_peer_id *)&peer_id;

	return (tx_peer_id->peer_id |
	        (tx_peer_id->ml_peer_valid << soc->peer_id_shift));
}
#else
/* Combine ml_peer_valid and peer_id field */
#define DP_BE_TX_COMP_PEER_ID_MASK	0x00003fff
#define DP_BE_TX_COMP_PEER_ID_SHIFT	0

static inline uint16_t dp_tx_comp_get_peer_id(struct dp_soc *soc,
					      void *tx_comp_hal_desc)
{
	uint16_t peer_id = hal_tx_comp_get_peer_id(tx_comp_hal_desc);

	return ((peer_id & DP_BE_TX_COMP_PEER_ID_MASK) >>
		DP_BE_TX_COMP_PEER_ID_SHIFT);
}
#endif

#ifdef DP_FEATURE_HW_COOKIE_CONVERSION
#ifdef DP_HW_COOKIE_CONVERT_EXCEPTION
void dp_tx_comp_get_params_from_hal_desc_be(struct dp_soc *soc,
					    void *tx_comp_hal_desc,
					    struct dp_tx_desc_s **r_tx_desc)
{
	uint32_t tx_desc_id;

	if (qdf_likely(
		hal_tx_comp_get_cookie_convert_done(tx_comp_hal_desc))) {
		/* HW cookie conversion done */
		*r_tx_desc = (struct dp_tx_desc_s *)
				hal_tx_comp_get_desc_va(tx_comp_hal_desc);
	} else {
		/* SW do cookie conversion to VA */
		tx_desc_id = hal_tx_comp_get_desc_id(tx_comp_hal_desc);
		*r_tx_desc =
		(struct dp_tx_desc_s *)dp_cc_desc_find(soc, tx_desc_id);
	}

	if (*r_tx_desc)
		(*r_tx_desc)->peer_id = dp_tx_comp_get_peer_id(soc,
							       tx_comp_hal_desc);
}
#else
void dp_tx_comp_get_params_from_hal_desc_be(struct dp_soc *soc,
					    void *tx_comp_hal_desc,
					    struct dp_tx_desc_s **r_tx_desc)
{
	*r_tx_desc = (struct dp_tx_desc_s *)
			hal_tx_comp_get_desc_va(tx_comp_hal_desc);

	if (*r_tx_desc)
		(*r_tx_desc)->peer_id = dp_tx_comp_get_peer_id(soc,
							       tx_comp_hal_desc);
}
#endif /* DP_HW_COOKIE_CONVERT_EXCEPTION */
#else

void dp_tx_comp_get_params_from_hal_desc_be(struct dp_soc *soc,
					    void *tx_comp_hal_desc,
					    struct dp_tx_desc_s **r_tx_desc)
{
	uint32_t tx_desc_id;

	/* SW do cookie conversion to VA */
	tx_desc_id = hal_tx_comp_get_desc_id(tx_comp_hal_desc);
	*r_tx_desc =
	(struct dp_tx_desc_s *)dp_cc_desc_find(soc, tx_desc_id);

	if (*r_tx_desc)
		(*r_tx_desc)->peer_id = dp_tx_comp_get_peer_id(soc,
							       tx_comp_hal_desc);
}
#endif /* DP_FEATURE_HW_COOKIE_CONVERSION */

static inline
void dp_tx_process_mec_notify_be(struct dp_soc *soc, uint8_t *status)
{
	struct dp_vdev *vdev;
	uint8_t vdev_id;
	uint32_t *htt_desc = (uint32_t *)status;

	qdf_assert_always(!soc->mec_fw_offload);

	/*
	 * Get vdev id from HTT status word in case of MEC
	 * notification
	 */
	vdev_id = DP_TX_WBM_COMPLETION_V3_VDEV_ID_GET(htt_desc[4]);
	if (qdf_unlikely(vdev_id >= MAX_VDEV_CNT))
		return;

	vdev = dp_vdev_get_ref_by_id(soc, vdev_id,
				     DP_MOD_ID_HTT_COMP);
	if (!vdev)
		return;
	dp_tx_mec_handler(vdev, status);
	dp_vdev_unref_delete(soc, vdev, DP_MOD_ID_HTT_COMP);
}

void dp_tx_process_htt_completion_be(struct dp_soc *soc,
				     struct dp_tx_desc_s *tx_desc,
				     uint8_t *status,
				     uint8_t ring_id)
{
	uint8_t tx_status;
	struct dp_pdev *pdev;
	struct dp_vdev *vdev = NULL;
	struct hal_tx_completion_status ts = {0};
	uint32_t *htt_desc = (uint32_t *)status;
	struct dp_peer *peer;
	struct cdp_tid_tx_stats *tid_stats = NULL;
	struct htt_soc *htt_handle;
	uint8_t vdev_id;

	tx_status = HTT_TX_WBM_COMPLETION_V3_TX_STATUS_GET(htt_desc[0]);
	htt_handle = (struct htt_soc *)soc->htt_handle;
	htt_wbm_event_record(htt_handle->htt_logger_handle, tx_status, status);

	/*
	 * There can be scenario where WBM consuming descriptor enqueued
	 * from TQM2WBM first and TQM completion can happen before MEC
	 * notification comes from FW2WBM. Avoid access any field of tx
	 * descriptor in case of MEC notify.
	 */
	if (tx_status == HTT_TX_FW2WBM_TX_STATUS_MEC_NOTIFY)
		return dp_tx_process_mec_notify_be(soc, status);

	/*
	 * If the descriptor is already freed in vdev_detach,
	 * continue to next descriptor
	 */
	if (qdf_unlikely(!tx_desc->flags)) {
		dp_tx_comp_info_rl("Descriptor freed in vdev_detach %d",
				   tx_desc->id);
		return;
	}

	if (qdf_unlikely(tx_desc->vdev_id == DP_INVALID_VDEV_ID)) {
		dp_tx_comp_info_rl("Invalid vdev_id %d", tx_desc->id);
		tx_desc->flags |= DP_TX_DESC_FLAG_TX_COMP_ERR;
		goto release_tx_desc;
	}

	pdev = tx_desc->pdev;

	if (qdf_unlikely(tx_desc->pdev->is_pdev_down)) {
		dp_tx_comp_info_rl("pdev in down state %d", tx_desc->id);
		tx_desc->flags |= DP_TX_DESC_FLAG_TX_COMP_ERR;
		goto release_tx_desc;
	}

	qdf_assert(tx_desc->pdev);

	vdev_id = tx_desc->vdev_id;
	vdev = dp_vdev_get_ref_by_id(soc, vdev_id,
				     DP_MOD_ID_HTT_COMP);

	if (qdf_unlikely(!vdev)) {
		dp_tx_comp_info_rl("Unable to get vdev ref  %d", tx_desc->id);
		tx_desc->flags |= DP_TX_DESC_FLAG_TX_COMP_ERR;
		goto release_tx_desc;
	}

	switch (tx_status) {
	case HTT_TX_FW2WBM_TX_STATUS_OK:
	case HTT_TX_FW2WBM_TX_STATUS_DROP:
	case HTT_TX_FW2WBM_TX_STATUS_TTL:
	{
		uint8_t tid;

		if (DP_TX_WBM_COMPLETION_V3_VALID_GET(htt_desc[3])) {
			ts.peer_id =
				DP_TX_WBM_COMPLETION_V3_SW_PEER_ID_GET(
						htt_desc[3]);
			ts.tid =
				DP_TX_WBM_COMPLETION_V3_TID_NUM_GET(
						htt_desc[3]);
		} else {
			ts.peer_id = HTT_INVALID_PEER;
			ts.tid = HTT_INVALID_TID;
		}
		ts.release_src = HAL_TX_COMP_RELEASE_SOURCE_FW;
		ts.ppdu_id =
			DP_TX_WBM_COMPLETION_V3_SCH_CMD_ID_GET(
					htt_desc[2]);
		ts.ack_frame_rssi =
			DP_TX_WBM_COMPLETION_V3_ACK_FRAME_RSSI_GET(
					htt_desc[2]);

		ts.tsf = htt_desc[4];
		ts.first_msdu = 1;
		ts.last_msdu = 1;
		ts.status = (tx_status == HTT_TX_FW2WBM_TX_STATUS_OK ?
			     HAL_TX_TQM_RR_FRAME_ACKED :
			     HAL_TX_TQM_RR_REM_CMD_REM);
		tid = ts.tid;
		if (qdf_unlikely(tid >= CDP_MAX_DATA_TIDS))
			tid = CDP_MAX_DATA_TIDS - 1;

		tid_stats = &pdev->stats.tid_stats.tid_tx_stats[ring_id][tid];

		if (qdf_unlikely(pdev->delay_stats_flag) ||
		    qdf_unlikely(dp_is_vdev_tx_delay_stats_enabled(vdev)))
			dp_tx_compute_delay(vdev, tx_desc, tid, ring_id);
		if (tx_status < CDP_MAX_TX_HTT_STATUS)
			tid_stats->htt_status_cnt[tx_status]++;

		peer = dp_peer_get_ref_by_id(soc, ts.peer_id,
					     DP_MOD_ID_HTT_COMP);
		if (qdf_likely(peer))
			dp_tx_update_peer_basic_stats(
						peer,
						qdf_nbuf_len(tx_desc->nbuf),
						tx_status,
						pdev->enhanced_stats_en);

		dp_tx_comp_process_tx_status(soc, tx_desc, &ts, peer, ring_id);
		dp_tx_comp_process_desc(soc, tx_desc, &ts, peer);
		dp_tx_desc_release(tx_desc, tx_desc->pool_id);

		if (qdf_likely(peer))
			dp_peer_unref_delete(peer, DP_MOD_ID_HTT_COMP);

		break;
	}
	case HTT_TX_FW2WBM_TX_STATUS_REINJECT:
	{
		dp_tx_reinject_handler(soc, vdev, tx_desc, status);
		break;
	}
	case HTT_TX_FW2WBM_TX_STATUS_INSPECT:
	{
		dp_tx_inspect_handler(soc, vdev, tx_desc, status);
		break;
	}
	case HTT_TX_FW2WBM_TX_STATUS_VDEVID_MISMATCH:
	{
		DP_STATS_INC(vdev, tx_i.dropped.fail_per_pkt_vdev_id_check, 1);
		goto release_tx_desc;
	}
	default:
		dp_tx_comp_err("Invalid HTT tx_status %d\n",
			       tx_status);
		goto release_tx_desc;
	}

	dp_vdev_unref_delete(soc, vdev, DP_MOD_ID_HTT_COMP);
	return;

release_tx_desc:
	dp_tx_comp_free_buf(soc, tx_desc);
	dp_tx_desc_release(tx_desc, tx_desc->pool_id);
	if (vdev)
		dp_vdev_unref_delete(soc, vdev, DP_MOD_ID_HTT_COMP);
}

#ifdef QCA_OL_TX_MULTIQ_SUPPORT
#ifdef DP_TX_IMPLICIT_RBM_MAPPING
/*
 * dp_tx_get_rbm_id()- Get the RBM ID for data transmission completion.
 * @dp_soc - DP soc structure pointer
 * @ring_id - Transmit Queue/ring_id to be used when XPS is enabled
 *
 * Return - RBM ID corresponding to TCL ring_id
 */
static inline uint8_t dp_tx_get_rbm_id_be(struct dp_soc *soc,
					  uint8_t ring_id)
{
	return 0;
}
#else
static inline uint8_t dp_tx_get_rbm_id_be(struct dp_soc *soc,
					  uint8_t ring_id)
{
	return (ring_id ? soc->wbm_sw0_bm_id + (ring_id - 1) :
			  HAL_WBM_SW2_BM_ID(soc->wbm_sw0_bm_id));
}
#endif /*DP_TX_IMPLICIT_RBM_MAPPING*/
#else
static inline uint8_t dp_tx_get_rbm_id_be(struct dp_soc *soc,
					  uint8_t tcl_index)
{
	uint8_t rbm;

	rbm = wlan_cfg_get_rbm_id_for_index(soc->wlan_cfg_ctx, tcl_index);
	dp_verbose_debug("tcl_id %u rbm %u", tcl_index, rbm);
	return rbm;
}
#endif

QDF_STATUS
dp_tx_hw_enqueue_be(struct dp_soc *soc, struct dp_vdev *vdev,
		    struct dp_tx_desc_s *tx_desc, uint16_t fw_metadata,
		    struct cdp_tx_exception_metadata *tx_exc_metadata,
		    struct dp_tx_msdu_info_s *msdu_info)
{
	void *hal_tx_desc;
	uint32_t *hal_tx_desc_cached;
	int coalesce = 0;
	struct dp_tx_queue *tx_q = &msdu_info->tx_queue;
	uint8_t ring_id = tx_q->ring_id;
	uint8_t tid = msdu_info->tid;
	struct dp_vdev_be *be_vdev;
	uint8_t cached_desc[HAL_TX_DESC_LEN_BYTES] = { 0 };
	uint8_t bm_id = dp_tx_get_rbm_id_be(soc, ring_id);
	hal_ring_handle_t hal_ring_hdl = NULL;
	QDF_STATUS status = QDF_STATUS_E_RESOURCES;

	be_vdev = dp_get_be_vdev_from_dp_vdev(vdev);

	if (!dp_tx_is_desc_id_valid(soc, tx_desc->id)) {
		dp_err_rl("Invalid tx desc id:%d", tx_desc->id);
		return QDF_STATUS_E_RESOURCES;
	}

	if (qdf_unlikely(tx_exc_metadata)) {
		qdf_assert_always((tx_exc_metadata->tx_encap_type ==
				   CDP_INVALID_TX_ENCAP_TYPE) ||
				   (tx_exc_metadata->tx_encap_type ==
				    vdev->tx_encap_type));

		if (tx_exc_metadata->tx_encap_type == htt_cmn_pkt_type_raw)
			qdf_assert_always((tx_exc_metadata->sec_type ==
					   CDP_INVALID_SEC_TYPE) ||
					   tx_exc_metadata->sec_type ==
					   vdev->sec_type);
	}

	hal_tx_desc_cached = (void *)cached_desc;

	hal_tx_desc_set_buf_addr_be(soc->hal_soc, hal_tx_desc_cached,
				    tx_desc->dma_addr, bm_id, tx_desc->id,
				    (tx_desc->flags & DP_TX_DESC_FLAG_FRAG));
	hal_tx_desc_set_lmac_id_be(soc->hal_soc, hal_tx_desc_cached,
				   vdev->lmac_id);

	hal_tx_desc_set_search_index_be(soc->hal_soc, hal_tx_desc_cached,
					vdev->bss_ast_idx);
	/*
	 * Bank_ID is used as DSCP_TABLE number in beryllium
	 * So there is no explicit field used for DSCP_TID_TABLE_NUM.
	 */

	hal_tx_desc_set_cache_set_num(soc->hal_soc, hal_tx_desc_cached,
				      (vdev->bss_ast_hash & 0xF));

	hal_tx_desc_set_fw_metadata(hal_tx_desc_cached, fw_metadata);
	hal_tx_desc_set_buf_length(hal_tx_desc_cached, tx_desc->length);
	hal_tx_desc_set_buf_offset(hal_tx_desc_cached, tx_desc->pkt_offset);

	if (tx_desc->flags & DP_TX_DESC_FLAG_TO_FW)
		hal_tx_desc_set_to_fw(hal_tx_desc_cached, 1);

	/* verify checksum offload configuration*/
	if ((qdf_nbuf_get_tx_cksum(tx_desc->nbuf) ==
				   QDF_NBUF_TX_CKSUM_TCP_UDP) ||
	      qdf_nbuf_is_tso(tx_desc->nbuf)) {
		hal_tx_desc_set_l3_checksum_en(hal_tx_desc_cached, 1);
		hal_tx_desc_set_l4_checksum_en(hal_tx_desc_cached, 1);
	}

	hal_tx_desc_set_bank_id(hal_tx_desc_cached, be_vdev->bank_id);

	hal_tx_desc_set_vdev_id(hal_tx_desc_cached, vdev->vdev_id);

	if (tid != HTT_TX_EXT_TID_INVALID)
		hal_tx_desc_set_hlos_tid(hal_tx_desc_cached, tid);

	dp_tx_desc_set_ktimestamp(vdev, tx_desc);

	dp_verbose_debug("length:%d , type = %d, dma_addr %llx, offset %d desc id %u",
			 tx_desc->length,
			 (tx_desc->flags & DP_TX_DESC_FLAG_FRAG),
			 (uint64_t)tx_desc->dma_addr, tx_desc->pkt_offset,
			 tx_desc->id);

	hal_ring_hdl = dp_tx_get_hal_ring_hdl(soc, ring_id);

	if (qdf_unlikely(dp_tx_hal_ring_access_start(soc, hal_ring_hdl))) {
		dp_err("HAL RING Access Failed -- %pK", hal_ring_hdl);
		DP_STATS_INC(soc, tx.tcl_ring_full[ring_id], 1);
		DP_STATS_INC(vdev, tx_i.dropped.enqueue_fail, 1);
		return status;
	}

	hal_tx_desc = hal_srng_src_get_next(soc->hal_soc, hal_ring_hdl);
	if (qdf_unlikely(!hal_tx_desc)) {
		dp_verbose_debug("TCL ring full ring_id:%d", ring_id);
		DP_STATS_INC(soc, tx.tcl_ring_full[ring_id], 1);
		DP_STATS_INC(vdev, tx_i.dropped.enqueue_fail, 1);
		goto ring_access_fail;
	}

	tx_desc->flags |= DP_TX_DESC_FLAG_QUEUED_TX;
	dp_vdev_peer_stats_update_protocol_cnt_tx(vdev, tx_desc->nbuf);

	/* Sync cached descriptor with HW */
	hal_tx_desc_sync(hal_tx_desc_cached, hal_tx_desc);

	coalesce = dp_tx_attempt_coalescing(soc, vdev, tx_desc, tid,
					    msdu_info, ring_id);

	DP_STATS_INC_PKT(vdev, tx_i.processed, 1, tx_desc->length);
	DP_STATS_INC(soc, tx.tcl_enq[ring_id], 1);
	dp_tx_update_stats(soc, tx_desc, ring_id);
	status = QDF_STATUS_SUCCESS;

	dp_tx_hw_desc_update_evt((uint8_t *)hal_tx_desc_cached,
				 hal_ring_hdl, soc);

ring_access_fail:
	dp_tx_ring_access_end_wrapper(soc, hal_ring_hdl, coalesce);

	return status;
}

QDF_STATUS dp_tx_init_bank_profiles(struct dp_soc_be *be_soc)
{
	int i, num_tcl_banks;

	num_tcl_banks = hal_tx_get_num_tcl_banks(be_soc->soc.hal_soc);

	qdf_assert_always(num_tcl_banks);
	be_soc->num_bank_profiles = num_tcl_banks;

	be_soc->bank_profiles = qdf_mem_malloc(num_tcl_banks *
					       sizeof(*be_soc->bank_profiles));
	if (!be_soc->bank_profiles) {
		dp_err("unable to allocate memory for DP TX Profiles!");
		return QDF_STATUS_E_NOMEM;
	}

	DP_TX_BANK_LOCK_CREATE(&be_soc->tx_bank_lock);

	for (i = 0; i < num_tcl_banks; i++) {
		be_soc->bank_profiles[i].is_configured = false;
		qdf_atomic_init(&be_soc->bank_profiles[i].ref_count);
	}
	dp_info("initialized %u bank profiles", be_soc->num_bank_profiles);
	return QDF_STATUS_SUCCESS;
}

void dp_tx_deinit_bank_profiles(struct dp_soc_be *be_soc)
{
	qdf_mem_free(be_soc->bank_profiles);
	DP_TX_BANK_LOCK_DESTROY(&be_soc->tx_bank_lock);
}

static
void dp_tx_get_vdev_bank_config(struct dp_vdev_be *be_vdev,
				union hal_tx_bank_config *bank_config)
{
	struct dp_vdev *vdev = &be_vdev->vdev;

	bank_config->epd = 0;

	bank_config->encap_type = vdev->tx_encap_type;

	/* Only valid for raw frames. Needs work for RAW mode */
	if (vdev->tx_encap_type == htt_cmn_pkt_type_raw) {
		bank_config->encrypt_type = sec_type_map[vdev->sec_type];
	} else {
		bank_config->encrypt_type = 0;
	}

	bank_config->src_buffer_swap = 0;
	bank_config->link_meta_swap = 0;

	if ((vdev->search_type == HAL_TX_ADDR_INDEX_SEARCH) &&
	    vdev->opmode == wlan_op_mode_sta) {
		bank_config->index_lookup_enable = 1;
		bank_config->mcast_pkt_ctrl = HAL_TX_MCAST_CTRL_MEC_NOTIFY;
		bank_config->addrx_en = 0;
		bank_config->addry_en = 0;
	} else {
		bank_config->index_lookup_enable = 0;
		bank_config->mcast_pkt_ctrl = HAL_TX_MCAST_CTRL_FW_EXCEPTION;
		bank_config->addrx_en =
			(vdev->hal_desc_addr_search_flags &
			 HAL_TX_DESC_ADDRX_EN) ? 1 : 0;
		bank_config->addry_en =
			(vdev->hal_desc_addr_search_flags &
			 HAL_TX_DESC_ADDRY_EN) ? 1 : 0;
	}

	bank_config->mesh_enable = vdev->mesh_vdev ? 1 : 0;

	bank_config->dscp_tid_map_id = vdev->dscp_tid_map_id;

	/* Disabling vdev id check for now. Needs revist. */
	bank_config->vdev_id_check_en = be_vdev->vdev_id_check_en;

	bank_config->pmac_id = vdev->lmac_id;
}

int dp_tx_get_bank_profile(struct dp_soc_be *be_soc,
			   struct dp_vdev_be *be_vdev)
{
	char *temp_str = "";
	bool found_match = false;
	int bank_id = DP_BE_INVALID_BANK_ID;
	int i;
	int unconfigured_slot = DP_BE_INVALID_BANK_ID;
	int zero_ref_count_slot = DP_BE_INVALID_BANK_ID;
	union hal_tx_bank_config vdev_config = {0};

	/* convert vdev params into hal_tx_bank_config */
	dp_tx_get_vdev_bank_config(be_vdev, &vdev_config);

	DP_TX_BANK_LOCK_ACQUIRE(&be_soc->tx_bank_lock);
	/* go over all banks and find a matching/unconfigured/unsed bank */
	for (i = 0; i < be_soc->num_bank_profiles; i++) {
		if (be_soc->bank_profiles[i].is_configured &&
		    (be_soc->bank_profiles[i].bank_config.val ^
						vdev_config.val) == 0) {
			found_match = true;
			break;
		}

		if (unconfigured_slot == DP_BE_INVALID_BANK_ID &&
		    !be_soc->bank_profiles[i].is_configured)
			unconfigured_slot = i;
		else if (zero_ref_count_slot  == DP_BE_INVALID_BANK_ID &&
		    !qdf_atomic_read(&be_soc->bank_profiles[i].ref_count))
			zero_ref_count_slot = i;
	}

	if (found_match) {
		temp_str = "matching";
		bank_id = i;
		goto inc_ref_and_return;
	}
	if (unconfigured_slot != DP_BE_INVALID_BANK_ID) {
		temp_str = "unconfigured";
		bank_id = unconfigured_slot;
		goto configure_and_return;
	}
	if (zero_ref_count_slot != DP_BE_INVALID_BANK_ID) {
		temp_str = "zero_ref_count";
		bank_id = zero_ref_count_slot;
	}
	if (bank_id == DP_BE_INVALID_BANK_ID) {
		dp_alert("unable to find TX bank!");
		QDF_BUG(0);
		return bank_id;
	}

configure_and_return:
	be_soc->bank_profiles[bank_id].is_configured = true;
	be_soc->bank_profiles[bank_id].bank_config.val = vdev_config.val;
	hal_tx_populate_bank_register(be_soc->soc.hal_soc,
				      &be_soc->bank_profiles[bank_id].bank_config,
				      bank_id);
inc_ref_and_return:
	qdf_atomic_inc(&be_soc->bank_profiles[bank_id].ref_count);
	DP_TX_BANK_LOCK_RELEASE(&be_soc->tx_bank_lock);

	dp_info("found %s slot at index %d, input:0x%x match:0x%x ref_count %u",
		temp_str, bank_id, vdev_config.val,
		be_soc->bank_profiles[bank_id].bank_config.val,
		qdf_atomic_read(&be_soc->bank_profiles[bank_id].ref_count));

	dp_info("epd:%x encap:%x encryp:%x src_buf_swap:%x link_meta_swap:%x addrx_en:%x addry_en:%x mesh_en:%x vdev_id_check:%x pmac_id:%x mcast_pkt_ctrl:%x",
		be_soc->bank_profiles[bank_id].bank_config.epd,
		be_soc->bank_profiles[bank_id].bank_config.encap_type,
		be_soc->bank_profiles[bank_id].bank_config.encrypt_type,
		be_soc->bank_profiles[bank_id].bank_config.src_buffer_swap,
		be_soc->bank_profiles[bank_id].bank_config.link_meta_swap,
		be_soc->bank_profiles[bank_id].bank_config.addrx_en,
		be_soc->bank_profiles[bank_id].bank_config.addry_en,
		be_soc->bank_profiles[bank_id].bank_config.mesh_enable,
		be_soc->bank_profiles[bank_id].bank_config.vdev_id_check_en,
		be_soc->bank_profiles[bank_id].bank_config.pmac_id,
		be_soc->bank_profiles[bank_id].bank_config.mcast_pkt_ctrl);

	return bank_id;
}

void dp_tx_put_bank_profile(struct dp_soc_be *be_soc,
			    struct dp_vdev_be *be_vdev)
{
	DP_TX_BANK_LOCK_ACQUIRE(&be_soc->tx_bank_lock);
	qdf_atomic_dec(&be_soc->bank_profiles[be_vdev->bank_id].ref_count);
	DP_TX_BANK_LOCK_RELEASE(&be_soc->tx_bank_lock);
}

void dp_tx_update_bank_profile(struct dp_soc_be *be_soc,
			       struct dp_vdev_be *be_vdev)
{
	dp_tx_put_bank_profile(be_soc, be_vdev);
	be_vdev->bank_id = dp_tx_get_bank_profile(be_soc, be_vdev);
}

QDF_STATUS dp_tx_desc_pool_init_be(struct dp_soc *soc,
				   uint16_t num_elem,
				   uint8_t pool_id)
{
	struct dp_tx_desc_pool_s *tx_desc_pool;
	struct dp_hw_cookie_conversion_t *cc_ctx;
	struct dp_soc_be *be_soc;
	struct dp_spt_page_desc *page_desc;
	struct dp_tx_desc_s *tx_desc;
	uint32_t ppt_idx = 0;
	uint32_t avail_entry_index = 0;

	if (!num_elem) {
		dp_err("desc_num 0 !!");
		return QDF_STATUS_E_FAILURE;
	}

	be_soc = dp_get_be_soc_from_dp_soc(soc);
	tx_desc_pool = &soc->tx_desc[pool_id];
	cc_ctx  = &be_soc->tx_cc_ctx[pool_id];

	tx_desc = tx_desc_pool->freelist;
	page_desc = &cc_ctx->page_desc_base[0];
	while (tx_desc) {
		if (avail_entry_index == 0) {
			if (ppt_idx >= cc_ctx->total_page_num) {
				dp_alert("insufficient secondary page tables");
				qdf_assert_always(0);
			}
			page_desc = &cc_ctx->page_desc_base[ppt_idx++];
		}

		/* put each TX Desc VA to SPT pages and
		 * get corresponding ID
		 */
		DP_CC_SPT_PAGE_UPDATE_VA(page_desc->page_v_addr,
					 avail_entry_index,
					 tx_desc);
		tx_desc->id =
			dp_cc_desc_id_generate(page_desc->ppt_index,
					       avail_entry_index);
		tx_desc->pool_id = pool_id;
		dp_tx_desc_set_magic(tx_desc, DP_TX_MAGIC_PATTERN_FREE);
		tx_desc = tx_desc->next;
		avail_entry_index = (avail_entry_index + 1) &
					DP_CC_SPT_PAGE_MAX_ENTRIES_MASK;
	}

	return QDF_STATUS_SUCCESS;
}

void dp_tx_desc_pool_deinit_be(struct dp_soc *soc,
			       struct dp_tx_desc_pool_s *tx_desc_pool,
			       uint8_t pool_id)
{
	struct dp_spt_page_desc *page_desc;
	struct dp_soc_be *be_soc;
	int i = 0;
	struct dp_hw_cookie_conversion_t *cc_ctx;

	be_soc = dp_get_be_soc_from_dp_soc(soc);
	cc_ctx  = &be_soc->tx_cc_ctx[pool_id];

	for (i = 0; i < cc_ctx->total_page_num; i++) {
		page_desc = &cc_ctx->page_desc_base[i];
		qdf_mem_zero(page_desc->page_v_addr, qdf_page_size);
	}
}

#ifdef WLAN_FEATURE_NEAR_FULL_IRQ
uint32_t dp_tx_comp_nf_handler(struct dp_intr *int_ctx, struct dp_soc *soc,
			       hal_ring_handle_t hal_ring_hdl, uint8_t ring_id,
			       uint32_t quota)
{
	struct dp_srng *tx_comp_ring = &soc->tx_comp_ring[ring_id];
	uint32_t work_done = 0;

	if (dp_srng_get_near_full_level(soc, tx_comp_ring) <
			DP_SRNG_THRESH_NEAR_FULL)
		return 0;

	qdf_atomic_set(&tx_comp_ring->near_full, 1);
	work_done++;

	return work_done;
}
#endif
