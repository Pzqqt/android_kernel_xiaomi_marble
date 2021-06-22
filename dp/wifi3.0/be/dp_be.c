/*
 * Copyright (c) 2021 The Linux Foundation. All rights reserved.
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

#include <wlan_utility.h>
#include <dp_internal.h>
#include <dp_htt.h>
#include "dp_be.h"
#include "dp_be_tx.h"
#include "dp_be_rx.h"
#include <hal_be_api.h>

/* Generic AST entry aging timer value */
#define DP_AST_AGING_TIMER_DEFAULT_MS	5000

#if defined(WLAN_MAX_PDEVS) && (WLAN_MAX_PDEVS == 1)
#define DP_TX_VDEV_ID_CHECK_ENABLE 0

static struct wlan_cfg_tcl_wbm_ring_num_map g_tcl_wbm_map_array[MAX_TCL_DATA_RINGS] = {
	{.tcl_ring_num = 0, .wbm_ring_num = 0, .wbm_rbm_id = HAL_BE_WBM_SW0_BM_ID, .for_ipa = 0},
	{1, 4, HAL_BE_WBM_SW4_BM_ID, 0},
	{2, 2, HAL_BE_WBM_SW2_BM_ID, 0},
	{3, 6, HAL_BE_WBM_SW5_BM_ID, 0},
	{4, 7, HAL_BE_WBM_SW6_BM_ID, 0}
};
#else
#define DP_TX_VDEV_ID_CHECK_ENABLE 1

static struct wlan_cfg_tcl_wbm_ring_num_map g_tcl_wbm_map_array[MAX_TCL_DATA_RINGS] = {
	{.tcl_ring_num = 0, .wbm_ring_num = 0, .wbm_rbm_id = HAL_BE_WBM_SW0_BM_ID, .for_ipa = 0},
	{1, 1, HAL_BE_WBM_SW1_BM_ID, 0},
	{2, 2, HAL_BE_WBM_SW2_BM_ID, 0},
	{3, 3, HAL_BE_WBM_SW3_BM_ID, 0},
	{4, 4, HAL_BE_WBM_SW4_BM_ID, 0}
};
#endif

static void dp_soc_cfg_attach_be(struct dp_soc *soc)
{
	struct wlan_cfg_dp_soc_ctxt *soc_cfg_ctx = soc->wlan_cfg_ctx;

	wlan_cfg_set_rx_rel_ring_id(soc_cfg_ctx, WBM2SW_REL_ERR_RING_NUM);

	soc->wlan_cfg_ctx->tcl_wbm_map_array = g_tcl_wbm_map_array;

	/* this is used only when dmac mode is enabled */
	soc->num_rx_refill_buf_rings = 1;
}

qdf_size_t dp_get_context_size_be(enum dp_context_type context_type)
{
	switch (context_type) {
	case DP_CONTEXT_TYPE_SOC:
		return sizeof(struct dp_soc_be);
	case DP_CONTEXT_TYPE_PDEV:
		return sizeof(struct dp_pdev_be);
	case DP_CONTEXT_TYPE_VDEV:
		return sizeof(struct dp_vdev_be);
	case DP_CONTEXT_TYPE_PEER:
		return sizeof(struct dp_peer_be);
	default:
		return 0;
	}
}

#ifdef DP_FEATURE_HW_COOKIE_CONVERSION
#if defined(WLAN_MAX_PDEVS) && (WLAN_MAX_PDEVS == 1)
/**
 * dp_cc_wbm_sw_en_cfg() - configure HW cookie conversion enablement
			   per wbm2sw ring
 * @cc_cfg: HAL HW cookie conversion configuration structure pointer
 *
 * Return: None
 */
static inline
void dp_cc_wbm_sw_en_cfg(struct hal_hw_cc_config *cc_cfg)
{
	cc_cfg->wbm2sw6_cc_en = 1;
	cc_cfg->wbm2sw5_cc_en = 1;
	cc_cfg->wbm2sw4_cc_en = 1;
	cc_cfg->wbm2sw3_cc_en = 1;
	cc_cfg->wbm2sw2_cc_en = 1;
	/* disable wbm2sw1 hw cc as it's for FW */
	cc_cfg->wbm2sw1_cc_en = 0;
	cc_cfg->wbm2sw0_cc_en = 1;
	cc_cfg->wbm2fw_cc_en = 0;
}
#else
static inline
void dp_cc_wbm_sw_en_cfg(struct hal_hw_cc_config *cc_cfg)
{
	cc_cfg->wbm2sw6_cc_en = 1;
	cc_cfg->wbm2sw5_cc_en = 1;
	cc_cfg->wbm2sw4_cc_en = 1;
	cc_cfg->wbm2sw3_cc_en = 1;
	cc_cfg->wbm2sw2_cc_en = 1;
	cc_cfg->wbm2sw1_cc_en = 1;
	cc_cfg->wbm2sw0_cc_en = 1;
	cc_cfg->wbm2fw_cc_en = 0;
}
#endif

/**
 * dp_cc_reg_cfg_init() - initialize and configure HW cookie
			  conversion register
 * @soc: SOC handle
 * @cc_ctx: cookie conversion context pointer
 * @is_4k_align: page address 4k alignd
 *
 * Return: None
 */
static void dp_cc_reg_cfg_init(struct dp_soc *soc,
			       struct dp_hw_cookie_conversion_t *cc_ctx,
			       bool is_4k_align)
{
	struct hal_hw_cc_config cc_cfg = { 0 };

	if (!soc->wlan_cfg_ctx->hw_cc_enabled) {
		dp_info("INI skip HW CC register setting");
		return;
	}

	cc_cfg.lut_base_addr_31_0 = cc_ctx->cmem_base;
	cc_cfg.cc_global_en = true;
	cc_cfg.page_4k_align = is_4k_align;
	cc_cfg.cookie_offset_msb = DP_CC_DESC_ID_SPT_VA_OS_MSB;
	cc_cfg.cookie_page_msb = DP_CC_DESC_ID_PPT_PAGE_OS_MSB;
	/* 36th bit should be 1 then HW know this is CMEM address */
	cc_cfg.lut_base_addr_39_32 = 0x10;

	cc_cfg.error_path_cookie_conv_en = true;
	cc_cfg.release_path_cookie_conv_en = true;
	dp_cc_wbm_sw_en_cfg(&cc_cfg);

	hal_cookie_conversion_reg_cfg_be(soc->hal_soc, &cc_cfg);
}

/**
 * dp_hw_cc_cmem_write() - DP wrapper function for CMEM buffer writing
 * @hal_soc_hdl: HAL SOC handle
 * @offset: CMEM address
 * @value: value to write
 *
 * Return: None.
 */
static inline void dp_hw_cc_cmem_write(hal_soc_handle_t hal_soc_hdl,
				       uint32_t offset,
				       uint32_t value)
{
	hal_cmem_write(hal_soc_hdl, offset, value);
}

/**
 * dp_hw_cc_cmem_addr_init() - Check and initialize CMEM base address for
			       HW cookie conversion
 * @soc: SOC handle
 * @cc_ctx: cookie conversion context pointer
 *
 * Return: 0 in case of success, else error value
 */
static inline QDF_STATUS dp_hw_cc_cmem_addr_init(
				struct dp_soc *soc,
				struct dp_hw_cookie_conversion_t *cc_ctx)
{
	dp_info("cmem base 0x%llx, size 0x%llx",
		soc->cmem_base, soc->cmem_size);
	/* get CMEM for cookie conversion */
	if (soc->cmem_size < DP_CC_PPT_MEM_SIZE) {
		dp_err("cmem_size %llu bytes < 4K", soc->cmem_size);
		return QDF_STATUS_E_RESOURCES;
	}
	cc_ctx->cmem_base = (uint32_t)(soc->cmem_base +
					DP_CC_MEM_OFFSET_IN_CMEM);

	return QDF_STATUS_SUCCESS;
}

#else

static inline void dp_cc_reg_cfg_init(struct dp_soc *soc,
				      struct dp_hw_cookie_conversion_t *cc_ctx,
				      bool is_4k_align) {}

static inline void dp_hw_cc_cmem_write(hal_soc_handle_t hal_soc_hdl,
				       uint32_t offset,
				       uint32_t value)
{ }

static inline QDF_STATUS dp_hw_cc_cmem_addr_init(
				struct dp_soc *soc,
				struct dp_hw_cookie_conversion_t *cc_ctx)
{
	return QDF_STATUS_SUCCESS;
}
#endif

static QDF_STATUS dp_hw_cookie_conversion_attach(struct dp_soc_be *be_soc)
{
	struct dp_soc *soc = DP_SOC_BE_GET_SOC(be_soc);
	struct dp_hw_cookie_conversion_t *cc_ctx = &be_soc->hw_cc_ctx;
	uint32_t max_tx_rx_desc_num, num_spt_pages, i = 0;
	struct dp_spt_page_desc *spt_desc;
	struct qdf_mem_dma_page_t *dma_page;
	QDF_STATUS qdf_status;

	if (soc->cdp_soc.ol_ops->get_con_mode &&
	    soc->cdp_soc.ol_ops->get_con_mode() == QDF_GLOBAL_FTM_MODE)
		return QDF_STATUS_SUCCESS;

	qdf_status = dp_hw_cc_cmem_addr_init(soc, cc_ctx);
	if (!QDF_IS_STATUS_SUCCESS(qdf_status))
		return qdf_status;

	/* estimate how many SPT DDR pages needed */
	max_tx_rx_desc_num = WLAN_CFG_NUM_TX_DESC_MAX * MAX_TXDESC_POOLS +
			WLAN_CFG_RX_SW_DESC_NUM_SIZE_MAX * MAX_RXDESC_POOLS;
	num_spt_pages = max_tx_rx_desc_num / DP_CC_SPT_PAGE_MAX_ENTRIES;
	num_spt_pages = num_spt_pages <= DP_CC_PPT_MAX_ENTRIES ?
					num_spt_pages : DP_CC_PPT_MAX_ENTRIES;
	dp_info("num_spt_pages needed %d", num_spt_pages);

	dp_desc_multi_pages_mem_alloc(soc, DP_HW_CC_SPT_PAGE_TYPE,
				      &cc_ctx->page_pool, qdf_page_size,
				      num_spt_pages, 0, false);
	if (!cc_ctx->page_pool.dma_pages) {
		dp_err("spt ddr pages allocation failed");
		return QDF_STATUS_E_RESOURCES;
	}
	cc_ctx->page_desc_base = qdf_mem_malloc(
			num_spt_pages * sizeof(struct dp_spt_page_desc));
	if (!cc_ctx->page_desc_base) {
		dp_err("spt page descs allocation failed");
		goto fail_0;
	}

	/* initial page desc */
	spt_desc = cc_ctx->page_desc_base;
	dma_page = cc_ctx->page_pool.dma_pages;
	while (i < num_spt_pages) {
		/* check if page address 4K aligned */
		if (qdf_unlikely(dma_page[i].page_p_addr & 0xFFF)) {
			dp_err("non-4k aligned pages addr %pK",
			       (void *)dma_page[i].page_p_addr);
			goto fail_1;
		}

		spt_desc[i].page_v_addr =
					dma_page[i].page_v_addr_start;
		spt_desc[i].page_p_addr =
					dma_page[i].page_p_addr;
		i++;
	}

	cc_ctx->total_page_num = num_spt_pages;
	qdf_spinlock_create(&cc_ctx->cc_lock);

	return QDF_STATUS_SUCCESS;
fail_1:
	qdf_mem_free(cc_ctx->page_desc_base);
fail_0:
	dp_desc_multi_pages_mem_free(soc, DP_HW_CC_SPT_PAGE_TYPE,
				     &cc_ctx->page_pool, 0, false);

	return QDF_STATUS_E_FAILURE;
}

static QDF_STATUS dp_hw_cookie_conversion_detach(struct dp_soc_be *be_soc)
{
	struct dp_soc *soc = DP_SOC_BE_GET_SOC(be_soc);
	struct dp_hw_cookie_conversion_t *cc_ctx = &be_soc->hw_cc_ctx;

	if (soc->cdp_soc.ol_ops->get_con_mode &&
	    soc->cdp_soc.ol_ops->get_con_mode() == QDF_GLOBAL_FTM_MODE)
		return QDF_STATUS_SUCCESS;

	qdf_mem_free(cc_ctx->page_desc_base);
	dp_desc_multi_pages_mem_free(soc, DP_HW_CC_SPT_PAGE_TYPE,
				     &cc_ctx->page_pool, 0, false);
	qdf_spinlock_destroy(&cc_ctx->cc_lock);

	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS dp_hw_cookie_conversion_init(struct dp_soc_be *be_soc)
{
	struct dp_soc *soc = DP_SOC_BE_GET_SOC(be_soc);
	struct dp_hw_cookie_conversion_t *cc_ctx = &be_soc->hw_cc_ctx;
	uint32_t i = 0;
	struct dp_spt_page_desc *spt_desc;

	if (soc->cdp_soc.ol_ops->get_con_mode &&
	    soc->cdp_soc.ol_ops->get_con_mode() == QDF_GLOBAL_FTM_MODE)
		return QDF_STATUS_SUCCESS;

	if (!cc_ctx->total_page_num) {
		dp_err("total page num is 0");
		return QDF_STATUS_E_INVAL;
	}

	spt_desc = cc_ctx->page_desc_base;
	while (i < cc_ctx->total_page_num) {
		/* write page PA to CMEM */
		dp_hw_cc_cmem_write(soc->hal_soc,
				    (cc_ctx->cmem_base +
				     i * DP_CC_PPT_ENTRY_SIZE_4K_ALIGNED),
				    (spt_desc[i].page_p_addr >>
				     DP_CC_PPT_ENTRY_HW_APEND_BITS_4K_ALIGNED));

		spt_desc[i].ppt_index = i;
		spt_desc[i].avail_entry_index = 0;
		/* link page desc */
		if ((i + 1) != cc_ctx->total_page_num)
			spt_desc[i].next = &spt_desc[i + 1];
		else
			spt_desc[i].next = NULL;
		i++;
	}

	cc_ctx->page_desc_freelist = cc_ctx->page_desc_base;
	cc_ctx->free_page_num = cc_ctx->total_page_num;

	/* write WBM/REO cookie conversion CFG register */
	dp_cc_reg_cfg_init(soc, cc_ctx, true);

	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS dp_hw_cookie_conversion_deinit(struct dp_soc_be *be_soc)
{
	struct dp_soc *soc = DP_SOC_BE_GET_SOC(be_soc);
	struct dp_hw_cookie_conversion_t *cc_ctx = &be_soc->hw_cc_ctx;

	if (soc->cdp_soc.ol_ops->get_con_mode &&
	    soc->cdp_soc.ol_ops->get_con_mode() == QDF_GLOBAL_FTM_MODE)
		return QDF_STATUS_SUCCESS;

	cc_ctx->page_desc_freelist = NULL;
	cc_ctx->free_page_num = 0;

	return QDF_STATUS_SUCCESS;
}

uint16_t dp_cc_spt_page_desc_alloc(struct dp_soc_be *be_soc,
				   struct dp_spt_page_desc **list_head,
				   struct dp_spt_page_desc **list_tail,
				   uint16_t num_desc)
{
	uint16_t num_pages, count;
	struct dp_hw_cookie_conversion_t *cc_ctx = &be_soc->hw_cc_ctx;

	num_pages = (num_desc / DP_CC_SPT_PAGE_MAX_ENTRIES) +
			(num_desc % DP_CC_SPT_PAGE_MAX_ENTRIES ? 1 : 0);

	if (num_pages > cc_ctx->free_page_num) {
		dp_err("fail: num_pages required %d > free_page_num %d",
		       num_pages,
		       cc_ctx->free_page_num);
		return 0;
	}

	qdf_spin_lock_bh(&cc_ctx->cc_lock);

	*list_head = *list_tail = cc_ctx->page_desc_freelist;
	for (count = 0; count < num_pages; count++) {
		if (qdf_unlikely(!cc_ctx->page_desc_freelist)) {
			cc_ctx->page_desc_freelist = *list_head;
			*list_head = *list_tail = NULL;
			qdf_spin_unlock_bh(&cc_ctx->cc_lock);
			return 0;
		}
		*list_tail = cc_ctx->page_desc_freelist;
		cc_ctx->page_desc_freelist = cc_ctx->page_desc_freelist->next;
	}
	(*list_tail)->next = NULL;
	cc_ctx->free_page_num -= count;

	qdf_spin_unlock_bh(&cc_ctx->cc_lock);

	return count;
}

void dp_cc_spt_page_desc_free(struct dp_soc_be *be_soc,
			      struct dp_spt_page_desc **list_head,
			      struct dp_spt_page_desc **list_tail,
			      uint16_t page_nums)
{
	struct dp_hw_cookie_conversion_t *cc_ctx = &be_soc->hw_cc_ctx;
	struct dp_spt_page_desc *temp_list = NULL;

	qdf_spin_lock_bh(&cc_ctx->cc_lock);

	temp_list = cc_ctx->page_desc_freelist;
	cc_ctx->page_desc_freelist = *list_head;
	(*list_tail)->next = temp_list;
	cc_ctx->free_page_num += page_nums;
	*list_tail = NULL;
	*list_head = NULL;

	qdf_spin_unlock_bh(&cc_ctx->cc_lock);
}

static QDF_STATUS dp_soc_attach_be(struct dp_soc *soc)
{
	struct dp_soc_be *be_soc = dp_get_be_soc_from_dp_soc(soc);
	QDF_STATUS qdf_status = QDF_STATUS_SUCCESS;

	soc->wbm_sw0_bm_id = hal_tx_get_wbm_sw0_bm_id();
	qdf_status = dp_tx_init_bank_profiles(be_soc);

	/* cookie conversion */
	qdf_status = dp_hw_cookie_conversion_attach(be_soc);

	return qdf_status;
}

static QDF_STATUS dp_soc_detach_be(struct dp_soc *soc)
{
	struct dp_soc_be *be_soc = dp_get_be_soc_from_dp_soc(soc);

	dp_tx_deinit_bank_profiles(be_soc);

	dp_hw_cookie_conversion_detach(be_soc);

	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS dp_soc_init_be(struct dp_soc *soc)
{
	struct dp_soc_be *be_soc = dp_get_be_soc_from_dp_soc(soc);
	QDF_STATUS qdf_status = QDF_STATUS_SUCCESS;

	qdf_status = dp_hw_cookie_conversion_init(be_soc);

	/* route vdev_id mismatch notification via FW completion */
	hal_tx_vdev_mismatch_routing_set(soc->hal_soc,
					 HAL_TX_VDEV_MISMATCH_FW_NOTIFY);
	return qdf_status;
}

static QDF_STATUS dp_soc_deinit_be(struct dp_soc *soc)
{
	struct dp_soc_be *be_soc = dp_get_be_soc_from_dp_soc(soc);

	dp_hw_cookie_conversion_deinit(be_soc);

	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS dp_pdev_attach_be(struct dp_pdev *pdev)
{
	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS dp_pdev_detach_be(struct dp_pdev *pdev)
{
	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS dp_vdev_attach_be(struct dp_soc *soc, struct dp_vdev *vdev)
{
	struct dp_soc_be *be_soc = dp_get_be_soc_from_dp_soc(soc);
	struct dp_vdev_be *be_vdev = dp_get_be_vdev_from_dp_vdev(vdev);

	be_vdev->vdev_id_check_en = DP_TX_VDEV_ID_CHECK_ENABLE;

	be_vdev->bank_id = dp_tx_get_bank_profile(be_soc, be_vdev);

	if (be_vdev->bank_id == DP_BE_INVALID_BANK_ID) {
		QDF_BUG(0);
		return QDF_STATUS_E_FAULT;
	}

	if (vdev->opmode == wlan_op_mode_sta) {
		if (soc->cdp_soc.ol_ops->set_mec_timer)
			soc->cdp_soc.ol_ops->set_mec_timer(
					soc->ctrl_psoc,
					vdev->vdev_id,
					DP_AST_AGING_TIMER_DEFAULT_MS);

		hal_tx_vdev_mcast_ctrl_set(soc->hal_soc, vdev->vdev_id,
					   HAL_TX_MCAST_CTRL_MEC_NOTIFY);
	}

	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS dp_vdev_detach_be(struct dp_soc *soc, struct dp_vdev *vdev)
{
	struct dp_soc_be *be_soc = dp_get_be_soc_from_dp_soc(soc);
	struct dp_vdev_be *be_vdev = dp_get_be_vdev_from_dp_vdev(vdev);

	dp_tx_put_bank_profile(be_soc, be_vdev);
	return QDF_STATUS_SUCCESS;
}

qdf_size_t dp_get_soc_context_size_be(void)
{
	return sizeof(struct dp_soc_be);
}

/**
 * dp_rxdma_ring_sel_cfg_be() - Setup RXDMA ring config
 * @soc: Common DP soc handle
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS
dp_rxdma_ring_sel_cfg_be(struct dp_soc *soc)
{
	int i;
	int mac_id;
	struct htt_rx_ring_tlv_filter htt_tlv_filter = {0};
	struct dp_srng *rx_mac_srng;
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	/*
	 * In Beryllium chipset msdu_start, mpdu_end
	 * and rx_attn are part of msdu_end/mpdu_start
	 */
	htt_tlv_filter.msdu_start = 0;
	htt_tlv_filter.mpdu_end = 0;
	htt_tlv_filter.attention = 0;
	htt_tlv_filter.mpdu_start = 1;
	htt_tlv_filter.msdu_end = 1;
	htt_tlv_filter.packet = 1;
	htt_tlv_filter.packet_header = 1;

	htt_tlv_filter.ppdu_start = 0;
	htt_tlv_filter.ppdu_end = 0;
	htt_tlv_filter.ppdu_end_user_stats = 0;
	htt_tlv_filter.ppdu_end_user_stats_ext = 0;
	htt_tlv_filter.ppdu_end_status_done = 0;
	htt_tlv_filter.enable_fp = 1;
	htt_tlv_filter.enable_md = 0;
	htt_tlv_filter.enable_md = 0;
	htt_tlv_filter.enable_mo = 0;

	htt_tlv_filter.fp_mgmt_filter = 0;
	htt_tlv_filter.fp_ctrl_filter = FILTER_CTRL_BA_REQ;
	htt_tlv_filter.fp_data_filter = (FILTER_DATA_UCAST |
					 FILTER_DATA_MCAST |
					 FILTER_DATA_DATA);
	htt_tlv_filter.mo_mgmt_filter = 0;
	htt_tlv_filter.mo_ctrl_filter = 0;
	htt_tlv_filter.mo_data_filter = 0;
	htt_tlv_filter.md_data_filter = 0;

	htt_tlv_filter.offset_valid = true;

	/* Not subscribing to mpdu_end, msdu_start and rx_attn */
	htt_tlv_filter.rx_mpdu_end_offset = 0;
	htt_tlv_filter.rx_msdu_start_offset = 0;
	htt_tlv_filter.rx_attn_offset = 0;

	htt_tlv_filter.rx_packet_offset = soc->rx_pkt_tlv_size;
	htt_tlv_filter.rx_header_offset =
				hal_rx_pkt_tlv_offset_get(soc->hal_soc);
	htt_tlv_filter.rx_mpdu_start_offset =
				hal_rx_mpdu_start_offset_get(soc->hal_soc);
	htt_tlv_filter.rx_msdu_end_offset =
				hal_rx_msdu_end_offset_get(soc->hal_soc);

	dp_info("TLV subscription\n"
		"msdu_start %d, mpdu_end %d, attention %d"
		"mpdu_start %d, msdu_end %d, pkt_hdr %d, pkt %d\n"
		"TLV offsets\n"
		"msdu_start %d, mpdu_end %d, attention %d"
		"mpdu_start %d, msdu_end %d, pkt_hdr %d, pkt %d\n",
		htt_tlv_filter.msdu_start,
		htt_tlv_filter.mpdu_end,
		htt_tlv_filter.attention,
		htt_tlv_filter.mpdu_start,
		htt_tlv_filter.msdu_end,
		htt_tlv_filter.packet_header,
		htt_tlv_filter.packet,
		htt_tlv_filter.rx_msdu_start_offset,
		htt_tlv_filter.rx_mpdu_end_offset,
		htt_tlv_filter.rx_attn_offset,
		htt_tlv_filter.rx_mpdu_start_offset,
		htt_tlv_filter.rx_msdu_end_offset,
		htt_tlv_filter.rx_header_offset,
		htt_tlv_filter.rx_packet_offset);

	for (i = 0; i < MAX_PDEV_CNT; i++) {
		struct dp_pdev *pdev = soc->pdev_list[i];

		if (!pdev)
			continue;

		for (mac_id = 0; mac_id < NUM_RXDMA_RINGS_PER_PDEV; mac_id++) {
			int mac_for_pdev =
				dp_get_mac_id_for_pdev(mac_id, pdev->pdev_id);
			/*
			 * Obtain lmac id from pdev to access the LMAC ring
			 * in soc context
			 */
			int lmac_id =
				dp_get_lmac_id_for_pdev_id(soc, mac_id,
							   pdev->pdev_id);

			rx_mac_srng = dp_get_rxdma_ring(pdev, lmac_id);

			if (!rx_mac_srng->hal_srng)
				continue;

			htt_h2t_rx_ring_cfg(soc->htt_handle, mac_for_pdev,
					    rx_mac_srng->hal_srng,
					    RXDMA_BUF, RX_DATA_BUFFER_SIZE,
					    &htt_tlv_filter);
		}
	}
	return status;

}

#ifdef WLAN_FEATURE_NEAR_FULL_IRQ
/**
 * dp_service_near_full_srngs_be() - Main bottom half callback for the
 *				near-full IRQs.
 * @soc: Datapath SoC handle
 * @int_ctx: Interrupt context
 * @dp_budget: Budget of the work that can be done in the bottom half
 *
 * Return: work done in the handler
 */
static uint32_t
dp_service_near_full_srngs_be(struct dp_soc *soc, struct dp_intr *int_ctx,
			      uint32_t dp_budget)
{
	int ring = 0;
	int budget = dp_budget;
	uint32_t work_done  = 0;
	uint32_t remaining_quota = dp_budget;
	struct dp_intr_stats *intr_stats = &int_ctx->intr_stats;
	int tx_ring_near_full_mask = int_ctx->tx_ring_near_full_mask;
	int rx_near_full_grp_1_mask = int_ctx->rx_near_full_grp_1_mask;
	int rx_near_full_grp_2_mask = int_ctx->rx_near_full_grp_2_mask;
	int rx_near_full_mask = rx_near_full_grp_1_mask |
				rx_near_full_grp_2_mask;

	dp_verbose_debug("rx_ring_near_full 0x%x tx_ring_near_full 0x%x",
			 rx_near_full_mask,
			 tx_ring_near_full_mask);

	if (rx_near_full_mask) {
		for (ring = 0; ring < soc->num_reo_dest_rings; ring++) {
			if (!(rx_near_full_mask & (1 << ring)))
				continue;

			work_done = dp_rx_nf_process(int_ctx,
					soc->reo_dest_ring[ring].hal_srng,
					ring, remaining_quota);
			if (work_done) {
				intr_stats->num_rx_ring_near_full_masks[ring]++;
				dp_verbose_debug("rx NF mask 0x%x ring %d, work_done %d budget %d",
						 rx_near_full_mask, ring,
						 work_done,
						 budget);
				budget -=  work_done;
				if (budget <= 0)
					goto budget_done;
				remaining_quota = budget;
			}
		}
	}

	if (tx_ring_near_full_mask) {
		for (ring = 0; ring < soc->num_tcl_data_rings; ring++) {
			if (!(tx_ring_near_full_mask & (1 << ring)))
				continue;

			work_done = dp_tx_comp_nf_handler(int_ctx, soc,
					soc->tx_comp_ring[ring].hal_srng,
					ring, remaining_quota);
			if (work_done) {
				intr_stats->num_tx_comp_ring_near_full_masks[ring]++;
				dp_verbose_debug("tx NF mask 0x%x ring %d, work_done %d budget %d",
						 tx_ring_near_full_mask, ring,
						 work_done, budget);
				budget -=  work_done;
				if (budget <= 0)
					break;
				remaining_quota = budget;
			}
		}
	}

	intr_stats->num_near_full_masks++;

budget_done:
	return dp_budget - budget;
}

/**
 * dp_srng_test_and_update_nf_params_be() - Check if the srng is in near full
 *				state and set the reap_limit appropriately
 *				as per the near full state
 * @soc: Datapath soc handle
 * @dp_srng: Datapath handle for SRNG
 * @max_reap_limit: [Output Buffer] Buffer to set the max reap limit as per
 *			the srng near-full state
 *
 * Return: 1, if the srng is in near-full state
 *	   0, if the srng is not in near-full state
 */
static int
dp_srng_test_and_update_nf_params_be(struct dp_soc *soc,
				     struct dp_srng *dp_srng,
				     int *max_reap_limit)
{
	return _dp_srng_test_and_update_nf_params(soc, dp_srng, max_reap_limit);
}

/**
 * dp_init_near_full_arch_ops_be() - Initialize the arch ops handler for the
 *			near full IRQ handling operations.
 * @arch_ops: arch ops handle
 *
 * Return: none
 */
static inline void
dp_init_near_full_arch_ops_be(struct dp_arch_ops *arch_ops)
{
	arch_ops->dp_service_near_full_srngs = dp_service_near_full_srngs_be;
	arch_ops->dp_srng_test_and_update_nf_params =
					dp_srng_test_and_update_nf_params_be;
}

#else
static inline void
dp_init_near_full_arch_ops_be(struct dp_arch_ops *arch_ops)
{
}
#endif

#ifdef WLAN_SUPPORT_PPEDS
static void dp_soc_ppe_srng_deinit(struct dp_soc *soc)
{
	struct dp_soc_be *be_soc = dp_get_be_soc_from_dp_soc(soc);
	struct wlan_cfg_dp_soc_ctxt *soc_cfg_ctx;

	soc_cfg_ctx = soc->wlan_cfg_ctx;

	if (!wlan_cfg_get_dp_soc_is_ppe_enabled(soc_cfg_ctx))
		return;

	dp_srng_deinit(soc, &be_soc->ppe_release_ring, PPE_RELEASE, 0);
	wlan_minidump_remove(be_soc->ppe_release_ring.base_vaddr_unaligned,
			     be_soc->ppe_release_ring.alloc_size,
			     soc->ctrl_psoc,
			     WLAN_MD_DP_SRNG_PPE_RELEASE,
			     "ppe_release_ring");

	dp_srng_deinit(soc, &be_soc->ppe2tcl_ring, PPE2TCL, 0);
	wlan_minidump_remove(be_soc->ppe2tcl_ring.base_vaddr_unaligned,
			     be_soc->ppe2tcl_ring.alloc_size,
			     soc->ctrl_psoc,
			     WLAN_MD_DP_SRNG_PPE2TCL,
			     "ppe2tcl_ring");

	dp_srng_deinit(soc, &be_soc->reo2ppe_ring, REO2PPE, 0);
	wlan_minidump_remove(be_soc->reo2ppe_ring.base_vaddr_unaligned,
			     be_soc->reo2ppe_ring.alloc_size,
			     soc->ctrl_psoc,
			     WLAN_MD_DP_SRNG_REO2PPE,
			     "reo2ppe_ring");
}

static void dp_soc_ppe_srng_free(struct dp_soc *soc)
{
	struct dp_soc_be *be_soc = dp_get_be_soc_from_dp_soc(soc);
	struct wlan_cfg_dp_soc_ctxt *soc_cfg_ctx;

	soc_cfg_ctx = soc->wlan_cfg_ctx;

	if (!wlan_cfg_get_dp_soc_is_ppe_enabled(soc_cfg_ctx))
		return;

	dp_srng_free(soc, &be_soc->ppe_release_ring);

	dp_srng_free(soc, &be_soc->ppe2tcl_ring);

	dp_srng_free(soc, &be_soc->reo2ppe_ring);
}

static QDF_STATUS dp_soc_ppe_srng_alloc(struct dp_soc *soc)
{
	struct dp_soc_be *be_soc = dp_get_be_soc_from_dp_soc(soc);
	uint32_t entries;
	struct wlan_cfg_dp_soc_ctxt *soc_cfg_ctx;

	soc_cfg_ctx = soc->wlan_cfg_ctx;

	if (!wlan_cfg_get_dp_soc_is_ppe_enabled(soc_cfg_ctx))
		return QDF_STATUS_SUCCESS;

	entries = wlan_cfg_get_dp_soc_reo2ppe_ring_size(soc_cfg_ctx);

	if (dp_srng_alloc(soc, &be_soc->reo2ppe_ring, REO2PPE,
			  entries, 0)) {
		dp_err("%pK: dp_srng_alloc failed for reo2ppe", soc);
		goto fail;
	}

	entries = wlan_cfg_get_dp_soc_ppe2tcl_ring_size(soc_cfg_ctx);
	if (dp_srng_alloc(soc, &be_soc->ppe2tcl_ring, PPE2TCL,
			  entries, 0)) {
		dp_err("%pK: dp_srng_alloc failed for ppe2tcl_ring", soc);
		goto fail;
	}

	entries = wlan_cfg_get_dp_soc_ppe_release_ring_size(soc_cfg_ctx);
	if (dp_srng_alloc(soc, &be_soc->ppe_release_ring, PPE_RELEASE,
			  entries, 0)) {
		dp_err("%pK: dp_srng_alloc failed for ppe_release_ring", soc);
		goto fail;
	}

	return QDF_STATUS_SUCCESS;
fail:
	dp_soc_ppe_srng_free(soc);
	return QDF_STATUS_E_NOMEM;
}

static QDF_STATUS dp_soc_ppe_srng_init(struct dp_soc *soc)
{
	struct dp_soc_be *be_soc = dp_get_be_soc_from_dp_soc(soc);
	struct wlan_cfg_dp_soc_ctxt *soc_cfg_ctx;

	soc_cfg_ctx = soc->wlan_cfg_ctx;

	if (!wlan_cfg_get_dp_soc_is_ppe_enabled(soc_cfg_ctx))
		return QDF_STATUS_SUCCESS;

	if (dp_srng_init(soc, &be_soc->reo2ppe_ring, REO2PPE, 0, 0)) {
		dp_err("%pK: dp_srng_init failed for reo2ppe", soc);
		goto fail;
	}

	wlan_minidump_log(be_soc->reo2ppe_ring.base_vaddr_unaligned,
			  be_soc->reo2ppe_ring.alloc_size,
			  soc->ctrl_psoc,
			  WLAN_MD_DP_SRNG_REO2PPE,
			  "reo2ppe_ring");

	if (dp_srng_init(soc, &be_soc->ppe2tcl_ring, PPE2TCL, 0, 0)) {
		dp_err("%pK: dp_srng_init failed for ppe2tcl_ring", soc);
		goto fail;
	}

	wlan_minidump_log(be_soc->ppe2tcl_ring.base_vaddr_unaligned,
			  be_soc->ppe2tcl_ring.alloc_size,
			  soc->ctrl_psoc,
			  WLAN_MD_DP_SRNG_PPE2TCL,
			  "ppe2tcl_ring");

	if (dp_srng_init(soc, &be_soc->ppe_release_ring, PPE_RELEASE, 0, 0)) {
		dp_err("%pK: dp_srng_init failed for ppe_release_ring", soc);
		goto fail;
	}

	wlan_minidump_log(be_soc->ppe_release_ring.base_vaddr_unaligned,
			  be_soc->ppe_release_ring.alloc_size,
			  soc->ctrl_psoc,
			  WLAN_MD_DP_SRNG_PPE_RELEASE,
			  "ppe_release_ring");

	return QDF_STATUS_SUCCESS;
fail:
	dp_soc_ppe_srng_deinit(soc);
	return QDF_STATUS_E_NOMEM;
}
#else
static void dp_soc_ppe_srng_deinit(struct dp_soc *soc)
{
}

static void dp_soc_ppe_srng_free(struct dp_soc *soc)
{
}

static QDF_STATUS dp_soc_ppe_srng_alloc(struct dp_soc *soc)
{
	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS dp_soc_ppe_srng_init(struct dp_soc *soc)
{
	return QDF_STATUS_SUCCESS;
}
#endif

static void dp_soc_srng_deinit_be(struct dp_soc *soc)
{
	uint32_t i;

	dp_soc_ppe_srng_deinit(soc);

	if (hal_dmac_cmn_src_rxbuf_ring_get(soc->hal_soc)) {
		for (i = 0; i < soc->num_rx_refill_buf_rings; i++) {
			dp_srng_deinit(soc, &soc->rx_refill_buf_ring[i],
				       RXDMA_BUF, 0);
		}
	}
}

static void dp_soc_srng_free_be(struct dp_soc *soc)
{
	uint32_t i;

	dp_soc_ppe_srng_free(soc);

	if (hal_dmac_cmn_src_rxbuf_ring_get(soc->hal_soc)) {
		for (i = 0; i < soc->num_rx_refill_buf_rings; i++)
			dp_srng_free(soc, &soc->rx_refill_buf_ring[i]);
	}
}

static QDF_STATUS dp_soc_srng_alloc_be(struct dp_soc *soc)
{
	struct wlan_cfg_dp_soc_ctxt *soc_cfg_ctx;
	uint32_t ring_size;
	uint32_t i;

	soc_cfg_ctx = soc->wlan_cfg_ctx;

	ring_size = wlan_cfg_get_dp_soc_rxdma_refill_ring_size(soc_cfg_ctx);
	if (hal_dmac_cmn_src_rxbuf_ring_get(soc->hal_soc)) {
		for (i = 0; i < soc->num_rx_refill_buf_rings; i++) {
			if (dp_srng_alloc(soc, &soc->rx_refill_buf_ring[i],
					  RXDMA_BUF, ring_size, 0)) {
				dp_err("%pK: dp_srng_alloc failed refill ring",
				       soc);
				goto fail;
			}
		}
	}

	if (dp_soc_ppe_srng_alloc(soc)) {
		dp_err("%pK: ppe rings alloc failed",
		       soc);
		goto fail;
	}

	return QDF_STATUS_SUCCESS;
fail:
	dp_soc_srng_free_be(soc);
	return QDF_STATUS_E_NOMEM;
}

static QDF_STATUS dp_soc_srng_init_be(struct dp_soc *soc)
{
	int i = 0;

	if (hal_dmac_cmn_src_rxbuf_ring_get(soc->hal_soc)) {
		for (i = 0; i < soc->num_rx_refill_buf_rings; i++) {
			if (dp_srng_init(soc, &soc->rx_refill_buf_ring[i],
					 RXDMA_BUF, 0, 0)) {
				dp_err("%pK: dp_srng_init failed refill ring",
				       soc);
				goto fail;
			}
		}
	}

	if (dp_soc_ppe_srng_init(soc)) {
		dp_err("%pK: ppe rings init failed",
		       soc);
		goto fail;
	}

	return QDF_STATUS_SUCCESS;
fail:
	dp_soc_srng_deinit_be(soc);
	return QDF_STATUS_E_NOMEM;
}

#ifdef DP_TX_IMPLICIT_RBM_MAPPING
static void dp_tx_implicit_rbm_set_be(struct dp_soc *soc,
				      uint8_t tx_ring_id,
				      uint8_t bm_id)
{
	hal_tx_config_rbm_mapping_be(soc->hal_soc,
				     soc->tcl_data_ring[tx_ring_id].hal_srng,
				     bm_id);
}
#else
static void dp_tx_implicit_rbm_set_be(struct dp_soc *soc,
				      uint8_t tx_ring_id,
				      uint8_t bm_id)
{
}
#endif

void dp_initialize_arch_ops_be(struct dp_arch_ops *arch_ops)
{
#ifndef QCA_HOST_MODE_WIFI_DISABLED
	arch_ops->tx_hw_enqueue = dp_tx_hw_enqueue_be;
	arch_ops->dp_rx_process = dp_rx_process_be;
	arch_ops->tx_comp_get_params_from_hal_desc =
		dp_tx_comp_get_params_from_hal_desc_be;
	arch_ops->dp_tx_desc_pool_init = dp_tx_desc_pool_init_be;
	arch_ops->dp_tx_desc_pool_deinit = dp_tx_desc_pool_deinit_be;
	arch_ops->dp_rx_desc_pool_init = dp_rx_desc_pool_init_be;
	arch_ops->dp_rx_desc_pool_deinit = dp_rx_desc_pool_deinit_be;
	arch_ops->dp_wbm_get_rx_desc_from_hal_desc =
				dp_wbm_get_rx_desc_from_hal_desc_be;
#endif
	arch_ops->txrx_get_context_size = dp_get_context_size_be;
	arch_ops->dp_rx_desc_cookie_2_va =
			dp_rx_desc_cookie_2_va_be;

	arch_ops->txrx_soc_attach = dp_soc_attach_be;
	arch_ops->txrx_soc_detach = dp_soc_detach_be;
	arch_ops->txrx_soc_init = dp_soc_init_be;
	arch_ops->txrx_soc_deinit = dp_soc_deinit_be;
	arch_ops->txrx_soc_srng_alloc = dp_soc_srng_alloc_be;
	arch_ops->txrx_soc_srng_init = dp_soc_srng_init_be;
	arch_ops->txrx_soc_srng_deinit = dp_soc_srng_deinit_be;
	arch_ops->txrx_soc_srng_free = dp_soc_srng_free_be;
	arch_ops->txrx_pdev_attach = dp_pdev_attach_be;
	arch_ops->txrx_pdev_detach = dp_pdev_detach_be;
	arch_ops->txrx_vdev_attach = dp_vdev_attach_be;
	arch_ops->txrx_vdev_detach = dp_vdev_detach_be;
	arch_ops->dp_rxdma_ring_sel_cfg = dp_rxdma_ring_sel_cfg_be;
	arch_ops->dp_rx_peer_metadata_peer_id_get =
					dp_rx_peer_metadata_peer_id_get_be;
	arch_ops->soc_cfg_attach = dp_soc_cfg_attach_be;
	arch_ops->tx_implicit_rbm_set = dp_tx_implicit_rbm_set_be;

	dp_init_near_full_arch_ops_be(arch_ops);
}
