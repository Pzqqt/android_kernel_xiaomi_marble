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

#include "cdp_txrx_cmn_struct.h"
#include "dp_types.h"
#include "dp_tx.h"
#include "dp_be_tx.h"
#include "dp_tx_desc.h"
#include "hal_tx.h"
#include <hal_be_api.h>
#include <hal_be_tx.h>

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
	if (vdev->csum_enabled &&
	    ((qdf_nbuf_get_tx_cksum(tx_desc->nbuf) ==
					QDF_NBUF_TX_CKSUM_TCP_UDP) ||
	      qdf_nbuf_is_tso(tx_desc->nbuf))) {
		hal_tx_desc_set_l3_checksum_en(hal_tx_desc_cached, 1);
		hal_tx_desc_set_l4_checksum_en(hal_tx_desc_cached, 1);
	}

	hal_tx_desc_set_bank_id(hal_tx_desc_cached, be_vdev->bank_id);

	hal_tx_desc_set_vdev_id(hal_tx_desc_cached, vdev->vdev_id);

	if (tid != HTT_TX_EXT_TID_INVALID)
		hal_tx_desc_set_hlos_tid(hal_tx_desc_cached, tid);

	if (qdf_unlikely(vdev->pdev->delay_stats_flag) ||
	    qdf_unlikely(wlan_cfg_is_peer_ext_stats_enabled(soc->wlan_cfg_ctx)))
		tx_desc->timestamp = qdf_ktime_to_ms(qdf_ktime_real_get());

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

	coalesce = dp_tx_attempt_coalescing(soc, vdev, tx_desc, tid);

	DP_STATS_INC_PKT(vdev, tx_i.processed, 1, tx_desc->length);
	dp_tx_update_stats(soc, tx_desc->nbuf);
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

	qdf_mutex_create(&be_soc->tx_bank_lock);

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
	qdf_mutex_destroy(&be_soc->tx_bank_lock);
}

static
void dp_tx_get_vdev_bank_config(struct dp_vdev_be *be_vdev,
				union hal_tx_bank_config *bank_config)
{
	struct dp_vdev *vdev = &be_vdev->vdev;
	struct dp_soc *soc = vdev->pdev->soc;

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

	if ((soc->sta_mode_search_policy == HAL_TX_ADDR_INDEX_SEARCH) &&
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

	qdf_mutex_acquire(&be_soc->tx_bank_lock);
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
	qdf_mutex_release(&be_soc->tx_bank_lock);

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
	qdf_mutex_acquire(&be_soc->tx_bank_lock);
	qdf_atomic_dec(&be_soc->bank_profiles[be_vdev->bank_id].ref_count);
	qdf_mutex_release(&be_soc->tx_bank_lock);
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
	struct dp_soc_be *be_soc;
	struct dp_spt_page_desc *page_desc;
	struct dp_spt_page_desc_list *page_desc_list;
	struct dp_tx_desc_s *tx_desc;

	if (!num_elem) {
		dp_err("desc_num 0 !!");
		return QDF_STATUS_E_FAILURE;
	}

	be_soc = dp_get_be_soc_from_dp_soc(soc);
	tx_desc_pool = &soc->tx_desc[pool_id];
	page_desc_list = &be_soc->tx_spt_page_desc[pool_id];

	/* allocate SPT pages from page desc pool */
	page_desc_list->num_spt_pages =
		dp_cc_spt_page_desc_alloc(be_soc,
					  &page_desc_list->spt_page_list_head,
					  &page_desc_list->spt_page_list_tail,
					  num_elem);

	if (!page_desc_list->num_spt_pages) {
		dp_err("fail to allocate cookie conversion spt pages");
		return QDF_STATUS_E_FAILURE;
	}

	/* put each TX Desc VA to SPT pages and get corresponding ID */
	page_desc = page_desc_list->spt_page_list_head;
	tx_desc = tx_desc_pool->freelist;
	while (tx_desc) {
		DP_CC_SPT_PAGE_UPDATE_VA(page_desc->page_v_addr,
					 page_desc->avail_entry_index,
					 tx_desc);
		tx_desc->id =
			dp_cc_desc_id_generate(page_desc->ppt_index,
					       page_desc->avail_entry_index);
		tx_desc->pool_id = pool_id;
		tx_desc = tx_desc->next;

		page_desc->avail_entry_index++;
		if (page_desc->avail_entry_index >=
				DP_CC_SPT_PAGE_MAX_ENTRIES)
			page_desc = page_desc->next;
	}

	return QDF_STATUS_SUCCESS;
}

void dp_tx_desc_pool_deinit_be(struct dp_soc *soc,
			       struct dp_tx_desc_pool_s *tx_desc_pool,
			       uint8_t pool_id)
{
	struct dp_soc_be *be_soc;
	struct dp_spt_page_desc *page_desc;
	struct dp_spt_page_desc_list *page_desc_list;

	be_soc = dp_get_be_soc_from_dp_soc(soc);
	page_desc_list = &be_soc->tx_spt_page_desc[pool_id];

	if (!page_desc_list->num_spt_pages) {
		dp_warn("page_desc_list is empty for pool_id %d", pool_id);
		return;
	}

	/* cleanup for each page */
	page_desc = page_desc_list->spt_page_list_head;
	while (page_desc) {
		page_desc->avail_entry_index = 0;
		qdf_mem_zero(page_desc->page_v_addr, qdf_page_size);
		page_desc = page_desc->next;
	}

	/* free pages desc back to pool */
	dp_cc_spt_page_desc_free(be_soc,
				 &page_desc_list->spt_page_list_head,
				 &page_desc_list->spt_page_list_tail,
				 page_desc_list->num_spt_pages);
	page_desc_list->num_spt_pages = 0;
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
