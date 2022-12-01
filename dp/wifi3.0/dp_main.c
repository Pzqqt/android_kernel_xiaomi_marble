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

#include <qdf_types.h>
#include <qdf_lock.h>
#include <qdf_net_types.h>
#include <qdf_lro.h>
#include <qdf_module.h>
#include <hal_hw_headers.h>
#include <hal_api.h>
#include <hif.h>
#include <htt.h>
#include <wdi_event.h>
#include <queue.h>
#include "dp_types.h"
#include "dp_internal.h"
#include "dp_tx.h"
#include "dp_tx_desc.h"
#include "dp_rx.h"
#ifdef DP_RATETABLE_SUPPORT
#include "dp_ratetable.h"
#endif
#include <cdp_txrx_handle.h>
#include <wlan_cfg.h>
#include <wlan_utility.h>
#include "cdp_txrx_cmn_struct.h"
#include "cdp_txrx_stats_struct.h"
#include "cdp_txrx_cmn_reg.h"
#include <qdf_util.h>
#include "dp_peer.h"
#include "htt_stats.h"
#include "dp_htt.h"
#ifdef WLAN_SUPPORT_RX_FISA
#include <dp_fisa_rx.h>
#endif
#include "htt_ppdu_stats.h"
#include "qdf_mem.h"   /* qdf_mem_malloc,free */
#include "cfg_ucfg_api.h"
#include <wlan_module_ids.h>

#ifdef QCA_LL_TX_FLOW_CONTROL_V2
#include "cdp_txrx_flow_ctrl_v2.h"
#else

static inline void
cdp_dump_flow_pool_info(struct cdp_soc_t *soc)
{
	return;
}
#endif
#ifdef WIFI_MONITOR_SUPPORT
#include <dp_mon.h>
#endif
#include "dp_ipa.h"
#ifdef FEATURE_WDS
#include "dp_txrx_wds.h"
#endif
#ifdef WLAN_SUPPORT_MSCS
#include "dp_mscs.h"
#endif
#ifdef WLAN_SUPPORT_MESH_LATENCY
#include "dp_mesh_latency.h"
#endif
#ifdef ATH_SUPPORT_IQUE
#include "dp_txrx_me.h"
#endif
#if defined(DP_CON_MON)
#ifndef REMOVE_PKT_LOG
#include <pktlog_ac_api.h>
#include <pktlog_ac.h>
#endif
#endif
#ifdef WLAN_DP_FEATURE_SW_LATENCY_MGR
#include <dp_swlm.h>
#endif

#ifdef WLAN_FEATURE_STATS_EXT
#define INIT_RX_HW_STATS_LOCK(_soc) \
	qdf_spinlock_create(&(_soc)->rx_hw_stats_lock)
#define DEINIT_RX_HW_STATS_LOCK(_soc) \
	qdf_spinlock_destroy(&(_soc)->rx_hw_stats_lock)
#else
#define INIT_RX_HW_STATS_LOCK(_soc)  /* no op */
#define DEINIT_RX_HW_STATS_LOCK(_soc) /* no op */
#endif

#if defined(DP_PEER_EXTENDED_API) || defined(WLAN_DP_PENDING_MEM_FLUSH)
#define SET_PEER_REF_CNT_ONE(_peer) \
	qdf_atomic_set(&(_peer)->ref_cnt, 1)
#else
#define SET_PEER_REF_CNT_ONE(_peer)
#endif

#ifdef WLAN_SYSFS_DP_STATS
/* sysfs event wait time for firmware stat request unit millseconds */
#define WLAN_SYSFS_STAT_REQ_WAIT_MS 3000
#endif

QDF_COMPILE_TIME_ASSERT(max_rx_rings_check,
			MAX_REO_DEST_RINGS == CDP_MAX_RX_RINGS);

QDF_COMPILE_TIME_ASSERT(max_tx_rings_check,
			MAX_TCL_DATA_RINGS == CDP_MAX_TX_COMP_RINGS);

#define dp_init_alert(params...) QDF_TRACE_FATAL(QDF_MODULE_ID_DP_INIT, params)
#define dp_init_err(params...) QDF_TRACE_ERROR(QDF_MODULE_ID_DP_INIT, params)
#define dp_init_warn(params...) QDF_TRACE_WARN(QDF_MODULE_ID_DP_INIT, params)
#define dp_init_info(params...) \
	__QDF_TRACE_FL(QDF_TRACE_LEVEL_INFO_HIGH, QDF_MODULE_ID_DP_INIT, ## params)
#define dp_init_debug(params...) QDF_TRACE_DEBUG(QDF_MODULE_ID_DP_INIT, params)

#define dp_vdev_alert(params...) QDF_TRACE_FATAL(QDF_MODULE_ID_DP_VDEV, params)
#define dp_vdev_err(params...) QDF_TRACE_ERROR(QDF_MODULE_ID_DP_VDEV, params)
#define dp_vdev_warn(params...) QDF_TRACE_WARN(QDF_MODULE_ID_DP_VDEV, params)
#define dp_vdev_info(params...) \
	__QDF_TRACE_FL(QDF_TRACE_LEVEL_INFO_HIGH, QDF_MODULE_ID_DP_VDEV, ## params)
#define dp_vdev_debug(params...) QDF_TRACE_DEBUG(QDF_MODULE_ID_DP_VDEV, params)

void dp_configure_arch_ops(struct dp_soc *soc);
qdf_size_t dp_get_soc_context_size(uint16_t device_id);

/*
 * The max size of cdp_peer_stats_param_t is limited to 16 bytes.
 * If the buffer size is exceeding this size limit,
 * dp_txrx_get_peer_stats is to be used instead.
 */
QDF_COMPILE_TIME_ASSERT(cdp_peer_stats_param_t_max_size,
			(sizeof(cdp_peer_stats_param_t) <= 16));

#ifdef WLAN_FEATURE_DP_EVENT_HISTORY
/*
 * If WLAN_CFG_INT_NUM_CONTEXTS is changed, HIF_NUM_INT_CONTEXTS
 * also should be updated accordingly
 */
QDF_COMPILE_TIME_ASSERT(num_intr_grps,
			HIF_NUM_INT_CONTEXTS == WLAN_CFG_INT_NUM_CONTEXTS);

/*
 * HIF_EVENT_HIST_MAX should always be power of 2
 */
QDF_COMPILE_TIME_ASSERT(hif_event_history_size,
			(HIF_EVENT_HIST_MAX & (HIF_EVENT_HIST_MAX - 1)) == 0);
#endif /* WLAN_FEATURE_DP_EVENT_HISTORY */

/*
 * If WLAN_CFG_INT_NUM_CONTEXTS is changed,
 * WLAN_CFG_INT_NUM_CONTEXTS_MAX should also be updated
 */
QDF_COMPILE_TIME_ASSERT(wlan_cfg_num_int_ctxs,
			WLAN_CFG_INT_NUM_CONTEXTS_MAX >=
			WLAN_CFG_INT_NUM_CONTEXTS);

static QDF_STATUS dp_sysfs_deinitialize_stats(struct dp_soc *soc_hdl);
static QDF_STATUS dp_sysfs_initialize_stats(struct dp_soc *soc_hdl);

static void dp_pdev_srng_deinit(struct dp_pdev *pdev);
static QDF_STATUS dp_pdev_srng_init(struct dp_pdev *pdev);
static void dp_pdev_srng_free(struct dp_pdev *pdev);
static QDF_STATUS dp_pdev_srng_alloc(struct dp_pdev *pdev);

static void dp_soc_srng_deinit(struct dp_soc *soc);
static QDF_STATUS dp_soc_srng_init(struct dp_soc *soc);
static void dp_soc_srng_free(struct dp_soc *soc);
static QDF_STATUS dp_soc_srng_alloc(struct dp_soc *soc);

static void dp_soc_cfg_init(struct dp_soc *soc);
static void dp_soc_cfg_attach(struct dp_soc *soc);

static inline
QDF_STATUS dp_pdev_attach_wifi3(struct cdp_soc_t *txrx_soc,
				struct cdp_pdev_attach_params *params);

static int dp_pdev_post_attach_wifi3(struct cdp_soc_t *psoc, uint8_t pdev_id);

static QDF_STATUS
dp_pdev_init_wifi3(struct cdp_soc_t *txrx_soc,
		   HTC_HANDLE htc_handle,
		   qdf_device_t qdf_osdev,
		   uint8_t pdev_id);

static QDF_STATUS
dp_pdev_deinit_wifi3(struct cdp_soc_t *psoc, uint8_t pdev_id, int force);

static void dp_soc_detach_wifi3(struct cdp_soc_t *txrx_soc);
static void dp_soc_deinit_wifi3(struct cdp_soc_t *txrx_soc);

void *dp_soc_init(struct dp_soc *soc, HTC_HANDLE htc_handle,
		  struct hif_opaque_softc *hif_handle);
static void dp_pdev_detach(struct cdp_pdev *txrx_pdev, int force);
static QDF_STATUS dp_pdev_detach_wifi3(struct cdp_soc_t *psoc,
				       uint8_t pdev_id,
				       int force);
static struct dp_soc *
dp_soc_attach(struct cdp_ctrl_objmgr_psoc *ctrl_psoc,
	      struct cdp_soc_attach_params *params);
static inline QDF_STATUS dp_peer_create_wifi3(struct cdp_soc_t *soc_hdl,
					      uint8_t vdev_id,
					      uint8_t *peer_mac_addr,
					      enum cdp_peer_type peer_type);
static QDF_STATUS dp_peer_delete_wifi3(struct cdp_soc_t *soc_hdl,
				       uint8_t vdev_id,
				       uint8_t *peer_mac, uint32_t bitmap);
static void dp_vdev_flush_peers(struct cdp_vdev *vdev_handle,
				bool unmap_only);
#ifdef ENABLE_VERBOSE_DEBUG
bool is_dp_verbose_debug_enabled;
#endif

#if defined(WLAN_CFR_ENABLE) && defined(WLAN_ENH_CFR_ENABLE)
static bool dp_get_cfr_rcc(struct cdp_soc_t *soc_hdl, uint8_t pdev_id);
static void dp_set_cfr_rcc(struct cdp_soc_t *soc_hdl, uint8_t pdev_id,
			   bool enable);
static inline void
dp_get_cfr_dbg_stats(struct cdp_soc_t *soc_hdl, uint8_t pdev_id,
		     struct cdp_cfr_rcc_stats *cfr_rcc_stats);
static inline void
dp_clear_cfr_dbg_stats(struct cdp_soc_t *soc_hdl, uint8_t pdev_id);
#endif

static QDF_STATUS dp_init_tx_ring_pair_by_index(struct dp_soc *soc,
						uint8_t index);
static void dp_deinit_tx_pair_by_index(struct dp_soc *soc, int index);
static void dp_free_tx_ring_pair_by_index(struct dp_soc *soc, uint8_t index);
static QDF_STATUS dp_alloc_tx_ring_pair_by_index(struct dp_soc *soc,
						 uint8_t index);
static uint8_t dp_soc_ring_if_nss_offloaded(struct dp_soc *soc,
					    enum hal_ring_type ring_type,
					    int ring_num);

#define DP_INTR_POLL_TIMER_MS	5

#define MON_VDEV_TIMER_INIT 0x1
#define MON_VDEV_TIMER_RUNNING 0x2

#define DP_MCS_LENGTH (6*MAX_MCS)

#define DP_CURR_FW_STATS_AVAIL 19
#define DP_HTT_DBG_EXT_STATS_MAX 256
#define DP_MAX_SLEEP_TIME 100
#ifndef QCA_WIFI_3_0_EMU
#define SUSPEND_DRAIN_WAIT 500
#else
#define SUSPEND_DRAIN_WAIT 3000
#endif

#ifdef IPA_OFFLOAD
/* Exclude IPA rings from the interrupt context */
#define TX_RING_MASK_VAL	0xb
#define RX_RING_MASK_VAL	0x7
#else
#define TX_RING_MASK_VAL	0xF
#define RX_RING_MASK_VAL	0xF
#endif

#define STR_MAXLEN	64

#define RNG_ERR		"SRNG setup failed for"

/* Threshold for peer's cached buf queue beyond which frames are dropped */
#define DP_RX_CACHED_BUFQ_THRESH 64

/**
 * default_dscp_tid_map - Default DSCP-TID mapping
 *
 * DSCP        TID
 * 000000      0
 * 001000      1
 * 010000      2
 * 011000      3
 * 100000      4
 * 101000      5
 * 110000      6
 * 111000      7
 */
static uint8_t default_dscp_tid_map[DSCP_TID_MAP_MAX] = {
	0, 0, 0, 0, 0, 0, 0, 0,
	1, 1, 1, 1, 1, 1, 1, 1,
	2, 2, 2, 2, 2, 2, 2, 2,
	3, 3, 3, 3, 3, 3, 3, 3,
	4, 4, 4, 4, 4, 4, 4, 4,
	5, 5, 5, 5, 5, 5, 5, 5,
	6, 6, 6, 6, 6, 6, 6, 6,
	7, 7, 7, 7, 7, 7, 7, 7,
};

/**
 * default_pcp_tid_map - Default PCP-TID mapping
 *
 * PCP     TID
 * 000      0
 * 001      1
 * 010      2
 * 011      3
 * 100      4
 * 101      5
 * 110      6
 * 111      7
 */
static uint8_t default_pcp_tid_map[PCP_TID_MAP_MAX] = {
	0, 1, 2, 3, 4, 5, 6, 7,
};

/**
 * @brief Cpu to tx ring map
 */
uint8_t
dp_cpu_ring_map[DP_NSS_CPU_RING_MAP_MAX][WLAN_CFG_INT_NUM_CONTEXTS_MAX] = {
	{0x0, 0x1, 0x2, 0x0, 0x0, 0x1, 0x2, 0x0, 0x0, 0x1, 0x2},
	{0x1, 0x2, 0x1, 0x2, 0x1, 0x2, 0x1, 0x2, 0x1, 0x2, 0x1},
	{0x0, 0x2, 0x0, 0x2, 0x0, 0x2, 0x0, 0x2, 0x0, 0x2, 0x0},
	{0x2, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2},
	{0x3, 0x3, 0x3, 0x3, 0x3, 0x3, 0x3, 0x3, 0x3, 0x3, 0x3},
#ifdef WLAN_TX_PKT_CAPTURE_ENH
	{0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1}
#endif
};

qdf_export_symbol(dp_cpu_ring_map);

/**
 * @brief Select the type of statistics
 */
enum dp_stats_type {
	STATS_FW = 0,
	STATS_HOST = 1,
	STATS_TYPE_MAX = 2,
};

/**
 * @brief General Firmware statistics options
 *
 */
enum dp_fw_stats {
	TXRX_FW_STATS_INVALID	= -1,
};

/**
 * dp_stats_mapping_table - Firmware and Host statistics
 * currently supported
 */
const int dp_stats_mapping_table[][STATS_TYPE_MAX] = {
	{HTT_DBG_EXT_STATS_RESET, TXRX_HOST_STATS_INVALID},
	{HTT_DBG_EXT_STATS_PDEV_TX, TXRX_HOST_STATS_INVALID},
	{HTT_DBG_EXT_STATS_PDEV_RX, TXRX_HOST_STATS_INVALID},
	{HTT_DBG_EXT_STATS_PDEV_TX_HWQ, TXRX_HOST_STATS_INVALID},
	{HTT_DBG_EXT_STATS_PDEV_TX_SCHED, TXRX_HOST_STATS_INVALID},
	{HTT_DBG_EXT_STATS_PDEV_ERROR, TXRX_HOST_STATS_INVALID},
	{HTT_DBG_EXT_STATS_PDEV_TQM, TXRX_HOST_STATS_INVALID},
	{HTT_DBG_EXT_STATS_TQM_CMDQ, TXRX_HOST_STATS_INVALID},
	{HTT_DBG_EXT_STATS_TX_DE_INFO, TXRX_HOST_STATS_INVALID},
	{HTT_DBG_EXT_STATS_PDEV_TX_RATE, TXRX_HOST_STATS_INVALID},
	{HTT_DBG_EXT_STATS_PDEV_RX_RATE, TXRX_HOST_STATS_INVALID},
	{TXRX_FW_STATS_INVALID, TXRX_HOST_STATS_INVALID},
	{HTT_DBG_EXT_STATS_TX_SELFGEN_INFO, TXRX_HOST_STATS_INVALID},
	{HTT_DBG_EXT_STATS_TX_MU_HWQ, TXRX_HOST_STATS_INVALID},
	{HTT_DBG_EXT_STATS_RING_IF_INFO, TXRX_HOST_STATS_INVALID},
	{HTT_DBG_EXT_STATS_SRNG_INFO, TXRX_HOST_STATS_INVALID},
	{HTT_DBG_EXT_STATS_SFM_INFO, TXRX_HOST_STATS_INVALID},
	{HTT_DBG_EXT_STATS_PDEV_TX_MU, TXRX_HOST_STATS_INVALID},
	{HTT_DBG_EXT_STATS_ACTIVE_PEERS_LIST, TXRX_HOST_STATS_INVALID},
	/* Last ENUM for HTT FW STATS */
	{DP_HTT_DBG_EXT_STATS_MAX, TXRX_HOST_STATS_INVALID},
	{TXRX_FW_STATS_INVALID, TXRX_CLEAR_STATS},
	{TXRX_FW_STATS_INVALID, TXRX_RX_RATE_STATS},
	{TXRX_FW_STATS_INVALID, TXRX_TX_RATE_STATS},
	{TXRX_FW_STATS_INVALID, TXRX_TX_HOST_STATS},
	{TXRX_FW_STATS_INVALID, TXRX_RX_HOST_STATS},
	{TXRX_FW_STATS_INVALID, TXRX_AST_STATS},
	{TXRX_FW_STATS_INVALID, TXRX_SRNG_PTR_STATS},
	{TXRX_FW_STATS_INVALID, TXRX_RX_MON_STATS},
	{TXRX_FW_STATS_INVALID, TXRX_REO_QUEUE_STATS},
	{TXRX_FW_STATS_INVALID, TXRX_SOC_CFG_PARAMS},
	{TXRX_FW_STATS_INVALID, TXRX_PDEV_CFG_PARAMS},
	{TXRX_FW_STATS_INVALID, TXRX_SOC_INTERRUPT_STATS},
	{TXRX_FW_STATS_INVALID, TXRX_SOC_FSE_STATS},
	{TXRX_FW_STATS_INVALID, TXRX_HAL_REG_WRITE_STATS},
	{TXRX_FW_STATS_INVALID, TXRX_SOC_REO_HW_DESC_DUMP},
	{HTT_DBG_EXT_STATS_PDEV_RX_RATE_EXT, TXRX_HOST_STATS_INVALID}
};

/* MCL specific functions */
#if defined(DP_CON_MON)

#ifdef DP_CON_MON_MSI_ENABLED
/**
 * dp_soc_get_mon_mask_for_interrupt_mode() - get mon mode mask for intr mode
 * @soc: pointer to dp_soc handle
 * @intr_ctx_num: interrupt context number for which mon mask is needed
 *
 * For MCL, monitor mode rings are being processed in timer contexts (polled).
 * This function is returning 0, since in interrupt mode(softirq based RX),
 * we donot want to process monitor mode rings in a softirq.
 *
 * So, in case packet log is enabled for SAP/STA/P2P modes,
 * regular interrupt processing will not process monitor mode rings. It would be
 * done in a separate timer context.
 *
 * Return: 0
 */
static inline uint32_t
dp_soc_get_mon_mask_for_interrupt_mode(struct dp_soc *soc, int intr_ctx_num)
{
	return wlan_cfg_get_rx_mon_ring_mask(soc->wlan_cfg_ctx, intr_ctx_num);
}
#else
/**
 * dp_soc_get_mon_mask_for_interrupt_mode() - get mon mode mask for intr mode
 * @soc: pointer to dp_soc handle
 * @intr_ctx_num: interrupt context number for which mon mask is needed
 *
 * For MCL, monitor mode rings are being processed in timer contexts (polled).
 * This function is returning 0, since in interrupt mode(softirq based RX),
 * we donot want to process monitor mode rings in a softirq.
 *
 * So, in case packet log is enabled for SAP/STA/P2P modes,
 * regular interrupt processing will not process monitor mode rings. It would be
 * done in a separate timer context.
 *
 * Return: 0
 */
static inline uint32_t
dp_soc_get_mon_mask_for_interrupt_mode(struct dp_soc *soc, int intr_ctx_num)
{
	return 0;
}
#endif

#ifdef IPA_OFFLOAD
/**
 * dp_get_num_rx_contexts() - get number of RX contexts
 * @soc_hdl: cdp opaque soc handle
 *
 * Return: number of RX contexts
 */
static int dp_get_num_rx_contexts(struct cdp_soc_t *soc_hdl)
{
	int num_rx_contexts;
	uint32_t reo_ring_map;
	struct dp_soc *soc = (struct dp_soc *)soc_hdl;

	reo_ring_map = wlan_cfg_get_reo_rings_mapping(soc->wlan_cfg_ctx);

	switch (soc->arch_id) {
	case CDP_ARCH_TYPE_BE:
		/* 2 REO rings are used for IPA */
		reo_ring_map &=  ~(BIT(3) | BIT(7));

		break;
	case CDP_ARCH_TYPE_LI:
		/* 1 REO ring is used for IPA */
		reo_ring_map &=  ~BIT(3);
		break;
	default:
		dp_err("unkonwn arch_id 0x%x", soc->arch_id);
		QDF_BUG(0);
	}
	/*
	 * qdf_get_hweight32 prefer over qdf_get_hweight8 in case map is scaled
	 * in future
	 */
	num_rx_contexts = qdf_get_hweight32(reo_ring_map);

	return num_rx_contexts;
}
#else
static int dp_get_num_rx_contexts(struct cdp_soc_t *soc_hdl)
{
	int num_rx_contexts;
	uint32_t reo_config;
	struct dp_soc *soc = (struct dp_soc *)soc_hdl;

	reo_config = wlan_cfg_get_reo_rings_mapping(soc->wlan_cfg_ctx);
	/*
	 * qdf_get_hweight32 prefer over qdf_get_hweight8 in case map is scaled
	 * in future
	 */
	num_rx_contexts = qdf_get_hweight32(reo_config);

	return num_rx_contexts;
}
#endif

#else

/**
 * dp_soc_get_mon_mask_for_interrupt_mode() - get mon mode mask for intr mode
 * @soc: pointer to dp_soc handle
 * @intr_ctx_num: interrupt context number for which mon mask is needed
 *
 * Return: mon mask value
 */
static inline
uint32_t dp_soc_get_mon_mask_for_interrupt_mode(struct dp_soc *soc, int intr_ctx_num)
{
	return wlan_cfg_get_rx_mon_ring_mask(soc->wlan_cfg_ctx, intr_ctx_num);
}

/**
 * dp_soc_reset_mon_intr_mask() - reset mon intr mask
 * @soc: pointer to dp_soc handle
 *
 * Return:
 */
void dp_soc_reset_mon_intr_mask(struct dp_soc *soc)
{
	int i;

	for (i = 0; i < wlan_cfg_get_num_contexts(soc->wlan_cfg_ctx); i++) {
		soc->intr_ctx[i].rx_mon_ring_mask = 0;
		soc->intr_ctx[i].host2rxdma_mon_ring_mask = 0;
	}
}

qdf_export_symbol(dp_soc_reset_mon_intr_mask);

/*
 * dp_service_lmac_rings()- timer to reap lmac rings
 * @arg: SoC Handle
 *
 * Return:
 *
 */
static void dp_service_lmac_rings(void *arg)
{
	struct dp_soc *soc = (struct dp_soc *)arg;
	int ring = 0, i;
	struct dp_pdev *pdev = NULL;
	union dp_rx_desc_list_elem_t *desc_list = NULL;
	union dp_rx_desc_list_elem_t *tail = NULL;

	/* Process LMAC interrupts */
	for  (ring = 0 ; ring < MAX_NUM_LMAC_HW; ring++) {
		int mac_for_pdev = ring;
		struct dp_srng *rx_refill_buf_ring;

		pdev = dp_get_pdev_for_lmac_id(soc, mac_for_pdev);
		if (!pdev)
			continue;

		rx_refill_buf_ring = &soc->rx_refill_buf_ring[mac_for_pdev];

		dp_monitor_process(soc, NULL, mac_for_pdev,
				   QCA_NAPI_BUDGET);

		for (i = 0;
		     i < wlan_cfg_get_num_contexts(soc->wlan_cfg_ctx); i++)
			dp_rxdma_err_process(&soc->intr_ctx[i], soc,
					     mac_for_pdev,
					     QCA_NAPI_BUDGET);

		if (!dp_soc_ring_if_nss_offloaded(soc, RXDMA_BUF,
						  mac_for_pdev))
			dp_rx_buffers_replenish(soc, mac_for_pdev,
						rx_refill_buf_ring,
						&soc->rx_desc_buf[mac_for_pdev],
						0, &desc_list, &tail);
	}

	qdf_timer_mod(&soc->lmac_reap_timer, DP_INTR_POLL_TIMER_MS);
}

#endif

#ifdef FEATURE_MEC
void dp_peer_mec_flush_entries(struct dp_soc *soc)
{
	unsigned int index;
	struct dp_mec_entry *mecentry, *mecentry_next;

	TAILQ_HEAD(, dp_mec_entry) free_list;
	TAILQ_INIT(&free_list);

	if (!soc->mec_hash.mask)
		return;

	if (!soc->mec_hash.bins)
		return;

	if (!qdf_atomic_read(&soc->mec_cnt))
		return;

	qdf_spin_lock_bh(&soc->mec_lock);
	for (index = 0; index <= soc->mec_hash.mask; index++) {
		if (!TAILQ_EMPTY(&soc->mec_hash.bins[index])) {
			TAILQ_FOREACH_SAFE(mecentry, &soc->mec_hash.bins[index],
					   hash_list_elem, mecentry_next) {
			    dp_peer_mec_detach_entry(soc, mecentry, &free_list);
			}
		}
	}
	qdf_spin_unlock_bh(&soc->mec_lock);

	dp_peer_mec_free_list(soc, &free_list);
}

/**
 * dp_print_mec_entries() - Dump MEC entries in table
 * @soc: Datapath soc handle
 *
 * Return: none
 */
static void dp_print_mec_stats(struct dp_soc *soc)
{
	int i;
	uint32_t index;
	struct dp_mec_entry *mecentry = NULL, *mec_list;
	uint32_t num_entries = 0;

	DP_PRINT_STATS("MEC Stats:");
	DP_PRINT_STATS("   Entries Added   = %d", soc->stats.mec.added);
	DP_PRINT_STATS("   Entries Deleted = %d", soc->stats.mec.deleted);

	if (!qdf_atomic_read(&soc->mec_cnt))
		return;

	mec_list = qdf_mem_malloc(sizeof(*mecentry) * DP_PEER_MAX_MEC_ENTRY);
	if (!mec_list) {
		dp_peer_warn("%pK: failed to allocate mec_list", soc);
		return;
	}

	DP_PRINT_STATS("MEC Table:");
	for (index = 0; index <= soc->mec_hash.mask; index++) {
		qdf_spin_lock_bh(&soc->mec_lock);
		if (TAILQ_EMPTY(&soc->mec_hash.bins[index])) {
			qdf_spin_unlock_bh(&soc->mec_lock);
			continue;
		}

		TAILQ_FOREACH(mecentry, &soc->mec_hash.bins[index],
			      hash_list_elem) {
			qdf_mem_copy(&mec_list[num_entries], mecentry,
				     sizeof(*mecentry));
			num_entries++;
		}
		qdf_spin_unlock_bh(&soc->mec_lock);
	}

	if (!num_entries) {
		qdf_mem_free(mec_list);
		return;
	}

	for (i = 0; i < num_entries; i++) {
		DP_PRINT_STATS("%6d mac_addr = " QDF_MAC_ADDR_FMT
			       " is_active = %d pdev_id = %d vdev_id = %d",
			       i,
			       QDF_MAC_ADDR_REF(mec_list[i].mac_addr.raw),
			       mec_list[i].is_active,
			       mec_list[i].pdev_id,
			       mec_list[i].vdev_id);
	}
	qdf_mem_free(mec_list);
}
#else
static void dp_print_mec_stats(struct dp_soc *soc)
{
}
#endif

static int dp_peer_add_ast_wifi3(struct cdp_soc_t *soc_hdl,
				 uint8_t vdev_id,
				 uint8_t *peer_mac,
				 uint8_t *mac_addr,
				 enum cdp_txrx_ast_entry_type type,
				 uint32_t flags)
{
	int ret = -1;
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	struct dp_peer *peer = dp_peer_find_hash_find((struct dp_soc *)soc_hdl,
						       peer_mac, 0, vdev_id,
						       DP_MOD_ID_CDP);

	if (!peer) {
		dp_peer_debug("Peer is NULL!");
		return ret;
	}

	status = dp_peer_add_ast((struct dp_soc *)soc_hdl,
				 peer,
				 mac_addr,
				 type,
				 flags);
	if ((status == QDF_STATUS_SUCCESS) ||
	    (status == QDF_STATUS_E_ALREADY) ||
	    (status == QDF_STATUS_E_AGAIN))
		ret = 0;

	dp_hmwds_ast_add_notify(peer, mac_addr,
				type, status, false);

	dp_peer_unref_delete(peer, DP_MOD_ID_CDP);

	return ret;
}

static int dp_peer_update_ast_wifi3(struct cdp_soc_t *soc_hdl,
						uint8_t vdev_id,
						uint8_t *peer_mac,
						uint8_t *wds_macaddr,
						uint32_t flags)
{
	int status = -1;
	struct dp_soc *soc = (struct dp_soc *)soc_hdl;
	struct dp_ast_entry  *ast_entry = NULL;
	struct dp_peer *peer;

	if (soc->ast_offload_support)
		return status;

	peer = dp_peer_find_hash_find((struct dp_soc *)soc_hdl,
				      peer_mac, 0, vdev_id,
				      DP_MOD_ID_CDP);

	if (!peer) {
		dp_peer_debug("Peer is NULL!");
		return status;
	}

	qdf_spin_lock_bh(&soc->ast_lock);
	ast_entry = dp_peer_ast_hash_find_by_pdevid(soc, wds_macaddr,
						    peer->vdev->pdev->pdev_id);

	if (ast_entry) {
		status = dp_peer_update_ast(soc,
					    peer,
					    ast_entry, flags);
	}
	qdf_spin_unlock_bh(&soc->ast_lock);

	dp_peer_unref_delete(peer, DP_MOD_ID_CDP);

	return status;
}

/*
 * dp_peer_reset_ast_entries() - Deletes all HMWDS entries for a peer
 * @soc_handle:		Datapath SOC handle
 * @peer:		DP peer
 * @arg:		callback argument
 *
 * Return: None
 */
static void
dp_peer_reset_ast_entries(struct dp_soc *soc, struct dp_peer *peer, void *arg)
{
	struct dp_ast_entry *ast_entry = NULL;
	struct dp_ast_entry *tmp_ast_entry;

	DP_PEER_ITERATE_ASE_LIST(peer, ast_entry, tmp_ast_entry) {
		if ((ast_entry->type == CDP_TXRX_AST_TYPE_WDS_HM) ||
		    (ast_entry->type == CDP_TXRX_AST_TYPE_WDS_HM_SEC))
			dp_peer_del_ast(soc, ast_entry);
	}
}

/*
 * dp_wds_reset_ast_wifi3() - Reset the is_active param for ast entry
 * @soc_handle:		Datapath SOC handle
 * @wds_macaddr:	WDS entry MAC Address
 * @peer_macaddr:	WDS entry MAC Address
 * @vdev_id:		id of vdev handle
 * Return: QDF_STATUS
 */
static QDF_STATUS dp_wds_reset_ast_wifi3(struct cdp_soc_t *soc_hdl,
					 uint8_t *wds_macaddr,
					 uint8_t *peer_mac_addr,
					 uint8_t vdev_id)
{
	struct dp_soc *soc = (struct dp_soc *)soc_hdl;
	struct dp_ast_entry *ast_entry = NULL;
	struct dp_peer *peer;
	struct dp_pdev *pdev;
	struct dp_vdev *vdev;

	if (soc->ast_offload_support)
		return QDF_STATUS_E_FAILURE;

	vdev = dp_vdev_get_ref_by_id(soc, vdev_id, DP_MOD_ID_CDP);

	if (!vdev)
		return QDF_STATUS_E_FAILURE;

	pdev = vdev->pdev;

	if (peer_mac_addr) {
		peer = dp_peer_find_hash_find(soc, peer_mac_addr,
					      0, vdev->vdev_id,
					      DP_MOD_ID_CDP);
		if (!peer) {
			dp_vdev_unref_delete(soc, vdev, DP_MOD_ID_CDP);
			return QDF_STATUS_E_FAILURE;
		}

		qdf_spin_lock_bh(&soc->ast_lock);
		dp_peer_reset_ast_entries(soc, peer, NULL);
		qdf_spin_unlock_bh(&soc->ast_lock);
		dp_peer_unref_delete(peer, DP_MOD_ID_CDP);
	} else if (wds_macaddr) {
		qdf_spin_lock_bh(&soc->ast_lock);
		ast_entry = dp_peer_ast_hash_find_by_pdevid(soc, wds_macaddr,
							    pdev->pdev_id);

		if (ast_entry) {
			if ((ast_entry->type == CDP_TXRX_AST_TYPE_WDS_HM) ||
			    (ast_entry->type == CDP_TXRX_AST_TYPE_WDS_HM_SEC))
				dp_peer_del_ast(soc, ast_entry);
		}
		qdf_spin_unlock_bh(&soc->ast_lock);
	}

	dp_vdev_unref_delete(soc, vdev, DP_MOD_ID_CDP);
	return QDF_STATUS_SUCCESS;
}

/*
 * dp_wds_reset_ast_table_wifi3() - Reset the is_active param for all ast entry
 * @soc:		Datapath SOC handle
 * @vdev_id:		id of vdev object
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS
dp_wds_reset_ast_table_wifi3(struct cdp_soc_t  *soc_hdl,
			     uint8_t vdev_id)
{
	struct dp_soc *soc = (struct dp_soc *) soc_hdl;

	if (soc->ast_offload_support)
		return QDF_STATUS_SUCCESS;

	qdf_spin_lock_bh(&soc->ast_lock);

	dp_soc_iterate_peer(soc, dp_peer_reset_ast_entries, NULL,
			    DP_MOD_ID_CDP);
	qdf_spin_unlock_bh(&soc->ast_lock);

	return QDF_STATUS_SUCCESS;
}

/*
 * dp_peer_flush_ast_entries() - Delete all wds and hmwds ast entries of a peer
 * @soc:		Datapath SOC
 * @peer:		Datapath peer
 * @arg:		arg to callback
 *
 * Return: None
 */
static void
dp_peer_flush_ast_entries(struct dp_soc *soc, struct dp_peer *peer, void *arg)
{
	struct dp_ast_entry *ase = NULL;
	struct dp_ast_entry *temp_ase;

	DP_PEER_ITERATE_ASE_LIST(peer, ase, temp_ase) {
		if ((ase->type ==
			CDP_TXRX_AST_TYPE_STATIC) ||
			(ase->type ==
			 CDP_TXRX_AST_TYPE_SELF) ||
			(ase->type ==
			 CDP_TXRX_AST_TYPE_STA_BSS))
			continue;
		dp_peer_del_ast(soc, ase);
	}
}

/*
 * dp_wds_flush_ast_table_wifi3() - Delete all wds and hmwds ast entry
 * @soc:		Datapath SOC handle
 *
 * Return: None
 */
static void dp_wds_flush_ast_table_wifi3(struct cdp_soc_t  *soc_hdl)
{
	struct dp_soc *soc = (struct dp_soc *) soc_hdl;

	qdf_spin_lock_bh(&soc->ast_lock);

	dp_soc_iterate_peer(soc, dp_peer_flush_ast_entries, NULL,
			    DP_MOD_ID_CDP);

	qdf_spin_unlock_bh(&soc->ast_lock);
	dp_peer_mec_flush_entries(soc);
}

/**
 * dp_peer_get_ast_info_by_soc_wifi3() - search the soc AST hash table
 *                                       and return ast entry information
 *                                       of first ast entry found in the
 *                                       table with given mac address
 *
 * @soc : data path soc handle
 * @ast_mac_addr : AST entry mac address
 * @ast_entry_info : ast entry information
 *
 * return : true if ast entry found with ast_mac_addr
 *          false if ast entry not found
 */
static bool dp_peer_get_ast_info_by_soc_wifi3
	(struct cdp_soc_t *soc_hdl,
	 uint8_t *ast_mac_addr,
	 struct cdp_ast_entry_info *ast_entry_info)
{
	struct dp_ast_entry *ast_entry = NULL;
	struct dp_soc *soc = (struct dp_soc *)soc_hdl;
	struct dp_peer *peer = NULL;

	if (soc->ast_offload_support)
		return false;

	qdf_spin_lock_bh(&soc->ast_lock);

	ast_entry = dp_peer_ast_hash_find_soc(soc, ast_mac_addr);
	if ((!ast_entry) ||
	    (ast_entry->delete_in_progress && !ast_entry->callback)) {
		qdf_spin_unlock_bh(&soc->ast_lock);
		return false;
	}

	peer = dp_peer_get_ref_by_id(soc, ast_entry->peer_id,
				     DP_MOD_ID_AST);
	if (!peer) {
		qdf_spin_unlock_bh(&soc->ast_lock);
		return false;
	}

	ast_entry_info->type = ast_entry->type;
	ast_entry_info->pdev_id = ast_entry->pdev_id;
	ast_entry_info->vdev_id = ast_entry->vdev_id;
	ast_entry_info->peer_id = ast_entry->peer_id;
	qdf_mem_copy(&ast_entry_info->peer_mac_addr[0],
		     &peer->mac_addr.raw[0],
		     QDF_MAC_ADDR_SIZE);
	dp_peer_unref_delete(peer, DP_MOD_ID_AST);
	qdf_spin_unlock_bh(&soc->ast_lock);
	return true;
}

/**
 * dp_peer_get_ast_info_by_pdevid_wifi3() - search the soc AST hash table
 *                                          and return ast entry information
 *                                          if mac address and pdev_id matches
 *
 * @soc : data path soc handle
 * @ast_mac_addr : AST entry mac address
 * @pdev_id : pdev_id
 * @ast_entry_info : ast entry information
 *
 * return : true if ast entry found with ast_mac_addr
 *          false if ast entry not found
 */
static bool dp_peer_get_ast_info_by_pdevid_wifi3
		(struct cdp_soc_t *soc_hdl,
		 uint8_t *ast_mac_addr,
		 uint8_t pdev_id,
		 struct cdp_ast_entry_info *ast_entry_info)
{
	struct dp_ast_entry *ast_entry;
	struct dp_soc *soc = (struct dp_soc *)soc_hdl;
	struct dp_peer *peer = NULL;

	if (soc->ast_offload_support)
		return false;

	qdf_spin_lock_bh(&soc->ast_lock);

	ast_entry = dp_peer_ast_hash_find_by_pdevid(soc, ast_mac_addr,
						    pdev_id);

	if ((!ast_entry) ||
	    (ast_entry->delete_in_progress && !ast_entry->callback)) {
		qdf_spin_unlock_bh(&soc->ast_lock);
		return false;
	}

	peer = dp_peer_get_ref_by_id(soc, ast_entry->peer_id,
				     DP_MOD_ID_AST);
	if (!peer) {
		qdf_spin_unlock_bh(&soc->ast_lock);
		return false;
	}

	ast_entry_info->type = ast_entry->type;
	ast_entry_info->pdev_id = ast_entry->pdev_id;
	ast_entry_info->vdev_id = ast_entry->vdev_id;
	ast_entry_info->peer_id = ast_entry->peer_id;
	qdf_mem_copy(&ast_entry_info->peer_mac_addr[0],
		     &peer->mac_addr.raw[0],
		     QDF_MAC_ADDR_SIZE);
	dp_peer_unref_delete(peer, DP_MOD_ID_AST);
	qdf_spin_unlock_bh(&soc->ast_lock);
	return true;
}

/**
 * dp_peer_ast_entry_del_by_soc() - delete the ast entry from soc AST hash table
 *                            with given mac address
 *
 * @soc : data path soc handle
 * @ast_mac_addr : AST entry mac address
 * @callback : callback function to called on ast delete response from FW
 * @cookie : argument to be passed to callback
 *
 * return : QDF_STATUS_SUCCESS if ast entry found with ast_mac_addr and delete
 *          is sent
 *          QDF_STATUS_E_INVAL false if ast entry not found
 */
static QDF_STATUS dp_peer_ast_entry_del_by_soc(struct cdp_soc_t *soc_handle,
					       uint8_t *mac_addr,
					       txrx_ast_free_cb callback,
					       void *cookie)

{
	struct dp_soc *soc = (struct dp_soc *)soc_handle;
	struct dp_ast_entry *ast_entry = NULL;
	txrx_ast_free_cb cb = NULL;
	void *arg = NULL;

	if (soc->ast_offload_support)
		return -QDF_STATUS_E_INVAL;

	qdf_spin_lock_bh(&soc->ast_lock);
	ast_entry = dp_peer_ast_hash_find_soc(soc, mac_addr);
	if (!ast_entry) {
		qdf_spin_unlock_bh(&soc->ast_lock);
		return -QDF_STATUS_E_INVAL;
	}

	if (ast_entry->callback) {
		cb = ast_entry->callback;
		arg = ast_entry->cookie;
	}

	ast_entry->callback = callback;
	ast_entry->cookie = cookie;

	/*
	 * if delete_in_progress is set AST delete is sent to target
	 * and host is waiting for response should not send delete
	 * again
	 */
	if (!ast_entry->delete_in_progress)
		dp_peer_del_ast(soc, ast_entry);

	qdf_spin_unlock_bh(&soc->ast_lock);
	if (cb) {
		cb(soc->ctrl_psoc,
		   dp_soc_to_cdp_soc(soc),
		   arg,
		   CDP_TXRX_AST_DELETE_IN_PROGRESS);
	}
	return QDF_STATUS_SUCCESS;
}

/**
 * dp_peer_ast_entry_del_by_pdev() - delete the ast entry from soc AST hash
 *                                   table if mac address and pdev_id matches
 *
 * @soc : data path soc handle
 * @ast_mac_addr : AST entry mac address
 * @pdev_id : pdev id
 * @callback : callback function to called on ast delete response from FW
 * @cookie : argument to be passed to callback
 *
 * return : QDF_STATUS_SUCCESS if ast entry found with ast_mac_addr and delete
 *          is sent
 *          QDF_STATUS_E_INVAL false if ast entry not found
 */

static QDF_STATUS dp_peer_ast_entry_del_by_pdev(struct cdp_soc_t *soc_handle,
						uint8_t *mac_addr,
						uint8_t pdev_id,
						txrx_ast_free_cb callback,
						void *cookie)

{
	struct dp_soc *soc = (struct dp_soc *)soc_handle;
	struct dp_ast_entry *ast_entry;
	txrx_ast_free_cb cb = NULL;
	void *arg = NULL;

	if (soc->ast_offload_support)
		return -QDF_STATUS_E_INVAL;

	qdf_spin_lock_bh(&soc->ast_lock);
	ast_entry = dp_peer_ast_hash_find_by_pdevid(soc, mac_addr, pdev_id);

	if (!ast_entry) {
		qdf_spin_unlock_bh(&soc->ast_lock);
		return -QDF_STATUS_E_INVAL;
	}

	if (ast_entry->callback) {
		cb = ast_entry->callback;
		arg = ast_entry->cookie;
	}

	ast_entry->callback = callback;
	ast_entry->cookie = cookie;

	/*
	 * if delete_in_progress is set AST delete is sent to target
	 * and host is waiting for response should not sent delete
	 * again
	 */
	if (!ast_entry->delete_in_progress)
		dp_peer_del_ast(soc, ast_entry);

	qdf_spin_unlock_bh(&soc->ast_lock);

	if (cb) {
		cb(soc->ctrl_psoc,
		   dp_soc_to_cdp_soc(soc),
		   arg,
		   CDP_TXRX_AST_DELETE_IN_PROGRESS);
	}
	return QDF_STATUS_SUCCESS;
}

/**
 * dp_srng_find_ring_in_mask() - find which ext_group a ring belongs
 * @ring_num: ring num of the ring being queried
 * @grp_mask: the grp_mask array for the ring type in question.
 *
 * The grp_mask array is indexed by group number and the bit fields correspond
 * to ring numbers.  We are finding which interrupt group a ring belongs to.
 *
 * Return: the index in the grp_mask array with the ring number.
 * -QDF_STATUS_E_NOENT if no entry is found
 */
static int dp_srng_find_ring_in_mask(int ring_num, uint8_t *grp_mask)
{
	int ext_group_num;
	uint8_t mask = 1 << ring_num;

	for (ext_group_num = 0; ext_group_num < WLAN_CFG_INT_NUM_CONTEXTS;
	     ext_group_num++) {
		if (mask & grp_mask[ext_group_num])
			return ext_group_num;
	}

	return -QDF_STATUS_E_NOENT;
}

/**
 * dp_is_msi_group_number_invalid() - check msi_group_number valid or not
 * @msi_group_number: MSI group number.
 * @msi_data_count: MSI data count.
 *
 * Return: true if msi_group_number is invalid.
 */
#ifdef WLAN_ONE_MSI_VECTOR
static bool dp_is_msi_group_number_invalid(int msi_group_number,
					   int msi_data_count)
{
	return false;
}
#else
static bool dp_is_msi_group_number_invalid(int msi_group_number,
					   int msi_data_count)
{
	return msi_group_number > msi_data_count;
}
#endif

#ifdef WLAN_FEATURE_NEAR_FULL_IRQ
/**
 * dp_is_reo_ring_num_in_nf_grp1() - Check if the current reo ring is part of
 *				rx_near_full_grp1 mask
 * @soc: Datapath SoC Handle
 * @ring_num: REO ring number
 *
 * Return: 1 if the ring_num belongs to reo_nf_grp1,
 *	   0, otherwise.
 */
static inline int
dp_is_reo_ring_num_in_nf_grp1(struct dp_soc *soc, int ring_num)
{
	return (WLAN_CFG_RX_NEAR_FULL_IRQ_MASK_1 & (1 << ring_num));
}

/**
 * dp_is_reo_ring_num_in_nf_grp2() - Check if the current reo ring is part of
 *				rx_near_full_grp2 mask
 * @soc: Datapath SoC Handle
 * @ring_num: REO ring number
 *
 * Return: 1 if the ring_num belongs to reo_nf_grp2,
 *	   0, otherwise.
 */
static inline int
dp_is_reo_ring_num_in_nf_grp2(struct dp_soc *soc, int ring_num)
{
	return (WLAN_CFG_RX_NEAR_FULL_IRQ_MASK_2 & (1 << ring_num));
}

/**
 * dp_srng_get_near_full_irq_mask() - Get near-full irq mask for a particular
 *				ring type and number
 * @soc: Datapath SoC handle
 * @ring_type: SRNG type
 * @ring_num: ring num
 *
 * Return: near ful irq mask pointer
 */
static inline
uint8_t *dp_srng_get_near_full_irq_mask(struct dp_soc *soc,
					enum hal_ring_type ring_type,
					int ring_num)
{
	struct wlan_cfg_dp_soc_ctxt *cfg_ctx = soc->wlan_cfg_ctx;
	uint8_t wbm2_sw_rx_rel_ring_id;
	uint8_t *nf_irq_mask = NULL;

	switch (ring_type) {
	case WBM2SW_RELEASE:
		wbm2_sw_rx_rel_ring_id =
			wlan_cfg_get_rx_rel_ring_id(cfg_ctx);
		if (ring_num != wbm2_sw_rx_rel_ring_id) {
			nf_irq_mask = &soc->wlan_cfg_ctx->
					int_tx_ring_near_full_irq_mask[0];
		}
		break;
	case REO_DST:
		if (dp_is_reo_ring_num_in_nf_grp1(soc, ring_num))
			nf_irq_mask =
			&soc->wlan_cfg_ctx->int_rx_ring_near_full_irq_1_mask[0];
		else if (dp_is_reo_ring_num_in_nf_grp2(soc, ring_num))
			nf_irq_mask =
			&soc->wlan_cfg_ctx->int_rx_ring_near_full_irq_2_mask[0];
		else
			qdf_assert(0);
		break;
	default:
		break;
	}

	return nf_irq_mask;
}

/**
 * dp_srng_set_msi2_ring_params() - Set the msi2 addr/data in the ring params
 * @soc: Datapath SoC handle
 * @ring_params: srng params handle
 * @msi2_addr: MSI2 addr to be set for the SRNG
 * @msi2_data: MSI2 data to be set for the SRNG
 *
 * Return: None
 */
static inline
void dp_srng_set_msi2_ring_params(struct dp_soc *soc,
				  struct hal_srng_params *ring_params,
				  qdf_dma_addr_t msi2_addr,
				  uint32_t msi2_data)
{
	ring_params->msi2_addr = msi2_addr;
	ring_params->msi2_data = msi2_data;
}

/**
 * dp_srng_msi2_setup() - Setup MSI2 details for near full IRQ of an SRNG
 * @soc: Datapath SoC handle
 * @ring_params: ring_params for SRNG
 * @ring_type: SENG type
 * @ring_num: ring number for the SRNG
 * @nf_msi_grp_num: near full msi group number
 *
 * Return: None
 */
static inline void
dp_srng_msi2_setup(struct dp_soc *soc,
		   struct hal_srng_params *ring_params,
		   int ring_type, int ring_num, int nf_msi_grp_num)
{
	uint32_t msi_data_start, msi_irq_start, addr_low, addr_high;
	int msi_data_count, ret;

	ret = pld_get_user_msi_assignment(soc->osdev->dev, "DP",
					  &msi_data_count, &msi_data_start,
					  &msi_irq_start);
	if (ret)
		return;

	if (nf_msi_grp_num < 0) {
		dp_init_info("%pK: ring near full IRQ not part of an ext_group; ring_type: %d,ring_num %d",
			     soc, ring_type, ring_num);
		ring_params->msi2_addr = 0;
		ring_params->msi2_data = 0;
		return;
	}

	if (dp_is_msi_group_number_invalid(nf_msi_grp_num, msi_data_count)) {
		dp_init_warn("%pK: 2 msi_groups will share an msi for near full IRQ; msi_group_num %d",
			     soc, nf_msi_grp_num);
		QDF_ASSERT(0);
	}

	pld_get_msi_address(soc->osdev->dev, &addr_low, &addr_high);

	ring_params->nf_irq_support = 1;
	ring_params->msi2_addr = addr_low;
	ring_params->msi2_addr |= (qdf_dma_addr_t)(((uint64_t)addr_high) << 32);
	ring_params->msi2_data = (nf_msi_grp_num % msi_data_count)
		+ msi_data_start;
	ring_params->flags |= HAL_SRNG_MSI_INTR;
}

/* Percentage of ring entries considered as nearly full */
#define DP_NF_HIGH_THRESH_PERCENTAGE	75
/* Percentage of ring entries considered as critically full */
#define DP_NF_CRIT_THRESH_PERCENTAGE	90
/* Percentage of ring entries considered as safe threshold */
#define DP_NF_SAFE_THRESH_PERCENTAGE	50

/**
 * dp_srng_configure_nf_interrupt_thresholds() - Configure the thresholds for
 *			near full irq
 * @soc: Datapath SoC handle
 * @ring_params: ring params for SRNG
 * @ring_type: ring type
 */
static inline void
dp_srng_configure_nf_interrupt_thresholds(struct dp_soc *soc,
					  struct hal_srng_params *ring_params,
					  int ring_type)
{
	if (ring_params->nf_irq_support) {
		ring_params->high_thresh = (ring_params->num_entries *
					    DP_NF_HIGH_THRESH_PERCENTAGE) / 100;
		ring_params->crit_thresh = (ring_params->num_entries *
					    DP_NF_CRIT_THRESH_PERCENTAGE) / 100;
		ring_params->safe_thresh = (ring_params->num_entries *
					    DP_NF_SAFE_THRESH_PERCENTAGE) /100;
	}
}

/**
 * dp_srng_set_nf_thresholds() - Set the near full thresholds to srng data
 *			structure from the ring params
 * @soc: Datapath SoC handle
 * @srng: SRNG handle
 * @ring_params: ring params for a SRNG
 *
 * Return: None
 */
static inline void
dp_srng_set_nf_thresholds(struct dp_soc *soc, struct dp_srng *srng,
			  struct hal_srng_params *ring_params)
{
	srng->crit_thresh = ring_params->crit_thresh;
	srng->safe_thresh = ring_params->safe_thresh;
}

#else
static inline
uint8_t *dp_srng_get_near_full_irq_mask(struct dp_soc *soc,
					enum hal_ring_type ring_type,
					int ring_num)
{
	return NULL;
}

static inline
void dp_srng_set_msi2_ring_params(struct dp_soc *soc,
				  struct hal_srng_params *ring_params,
				  qdf_dma_addr_t msi2_addr,
				  uint32_t msi2_data)
{
}

static inline void
dp_srng_msi2_setup(struct dp_soc *soc,
		   struct hal_srng_params *ring_params,
		   int ring_type, int ring_num, int nf_msi_grp_num)
{
}

static inline void
dp_srng_configure_nf_interrupt_thresholds(struct dp_soc *soc,
					  struct hal_srng_params *ring_params,
					  int ring_type)
{
}

static inline void
dp_srng_set_nf_thresholds(struct dp_soc *soc, struct dp_srng *srng,
			  struct hal_srng_params *ring_params)
{
}
#endif

static int dp_srng_calculate_msi_group(struct dp_soc *soc,
				       enum hal_ring_type ring_type,
				       int ring_num,
				       int *reg_msi_grp_num,
				       bool nf_irq_support,
				       int *nf_msi_grp_num)
{
	struct wlan_cfg_dp_soc_ctxt *cfg_ctx = soc->wlan_cfg_ctx;
	uint8_t *grp_mask, *nf_irq_mask = NULL;
	bool nf_irq_enabled = false;
	uint8_t wbm2_sw_rx_rel_ring_id;

	switch (ring_type) {
	case WBM2SW_RELEASE:
		wbm2_sw_rx_rel_ring_id =
			wlan_cfg_get_rx_rel_ring_id(cfg_ctx);
		if (ring_num == wbm2_sw_rx_rel_ring_id) {
			/* dp_rx_wbm_err_process - soc->rx_rel_ring */
			grp_mask = &cfg_ctx->int_rx_wbm_rel_ring_mask[0];
			ring_num = 0;
		} else { /* dp_tx_comp_handler - soc->tx_comp_ring */
			grp_mask = &soc->wlan_cfg_ctx->int_tx_ring_mask[0];
			nf_irq_mask = dp_srng_get_near_full_irq_mask(soc,
								     ring_type,
								     ring_num);
			if (nf_irq_mask)
				nf_irq_enabled = true;
		}
	break;

	case REO_EXCEPTION:
		/* dp_rx_err_process - &soc->reo_exception_ring */
		grp_mask = &soc->wlan_cfg_ctx->int_rx_err_ring_mask[0];
	break;

	case REO_DST:
		/* dp_rx_process - soc->reo_dest_ring */
		grp_mask = &soc->wlan_cfg_ctx->int_rx_ring_mask[0];
		nf_irq_mask = dp_srng_get_near_full_irq_mask(soc, ring_type,
							     ring_num);
		if (nf_irq_mask)
			nf_irq_enabled = true;
	break;

	case REO_STATUS:
		/* dp_reo_status_ring_handler - soc->reo_status_ring */
		grp_mask = &soc->wlan_cfg_ctx->int_reo_status_ring_mask[0];
	break;

	/* dp_rx_mon_status_srng_process - pdev->rxdma_mon_status_ring*/
	case RXDMA_MONITOR_STATUS:
	/* dp_rx_mon_dest_process - pdev->rxdma_mon_dst_ring */
	case RXDMA_MONITOR_DST:
		/* dp_mon_process */
		grp_mask = &soc->wlan_cfg_ctx->int_rx_mon_ring_mask[0];
	break;
	case TX_MONITOR_DST:
		/* dp_tx_mon_process */
		grp_mask = &soc->wlan_cfg_ctx->int_tx_mon_ring_mask[0];
	break;
	case RXDMA_DST:
		/* dp_rxdma_err_process */
		grp_mask = &soc->wlan_cfg_ctx->int_rxdma2host_ring_mask[0];
	break;

	case RXDMA_BUF:
		grp_mask = &soc->wlan_cfg_ctx->int_host2rxdma_ring_mask[0];
	break;

	case RXDMA_MONITOR_BUF:
		grp_mask = &soc->wlan_cfg_ctx->int_host2rxdma_mon_ring_mask[0];
	break;

	case TCL_DATA:
	/* CMD_CREDIT_RING is used as command in 8074 and credit in 9000 */
	case TCL_CMD_CREDIT:
	case REO_CMD:
	case SW2WBM_RELEASE:
	case WBM_IDLE_LINK:
		/* normally empty SW_TO_HW rings */
		return -QDF_STATUS_E_NOENT;
	break;

	case TCL_STATUS:
	case REO_REINJECT:
		/* misc unused rings */
		return -QDF_STATUS_E_NOENT;
	break;

	case CE_SRC:
	case CE_DST:
	case CE_DST_STATUS:
		/* CE_rings - currently handled by hif */
	default:
		return -QDF_STATUS_E_NOENT;
	break;
	}

	*reg_msi_grp_num = dp_srng_find_ring_in_mask(ring_num, grp_mask);

	if (nf_irq_support && nf_irq_enabled) {
		*nf_msi_grp_num = dp_srng_find_ring_in_mask(ring_num,
							    nf_irq_mask);
	}

	return QDF_STATUS_SUCCESS;
}

/*
 * dp_get_num_msi_available()- API to get number of MSIs available
 * @dp_soc: DP soc Handle
 * @interrupt_mode: Mode of interrupts
 *
 * Return: Number of MSIs available or 0 in case of integrated
 */
#if defined(WLAN_MAX_PDEVS) && (WLAN_MAX_PDEVS == 1)
static int dp_get_num_msi_available(struct dp_soc *soc, int interrupt_mode)
{
	return 0;
}
#else
/*
 * dp_get_num_msi_available()- API to get number of MSIs available
 * @dp_soc: DP soc Handle
 * @interrupt_mode: Mode of interrupts
 *
 * Return: Number of MSIs available or 0 in case of integrated
 */
static int dp_get_num_msi_available(struct dp_soc *soc, int interrupt_mode)
{
	int msi_data_count;
	int msi_data_start;
	int msi_irq_start;
	int ret;

	if (interrupt_mode == DP_INTR_INTEGRATED) {
		return 0;
	} else if (interrupt_mode == DP_INTR_MSI || interrupt_mode ==
		   DP_INTR_POLL) {
		ret = pld_get_user_msi_assignment(soc->osdev->dev, "DP",
						  &msi_data_count,
						  &msi_data_start,
						  &msi_irq_start);
		if (ret) {
			qdf_err("Unable to get DP MSI assignment %d",
				interrupt_mode);
			return -EINVAL;
		}
		return msi_data_count;
	}
	qdf_err("Interrupt mode invalid %d", interrupt_mode);
	return -EINVAL;
}
#endif

static void dp_srng_msi_setup(struct dp_soc *soc, struct hal_srng_params
			      *ring_params, int ring_type, int ring_num)
{
	int reg_msi_grp_num;
	/*
	 * nf_msi_grp_num needs to be initialized with negative value,
	 * to avoid configuring near-full msi for WBM2SW3 ring
	 */
	int nf_msi_grp_num = -1;
	int msi_data_count;
	int ret;
	uint32_t msi_data_start, msi_irq_start, addr_low, addr_high;
	bool nf_irq_support;

	ret = pld_get_user_msi_assignment(soc->osdev->dev, "DP",
					    &msi_data_count, &msi_data_start,
					    &msi_irq_start);

	if (ret)
		return;

	nf_irq_support = hal_srng_is_near_full_irq_supported(soc->hal_soc,
							     ring_type,
							     ring_num);
	ret = dp_srng_calculate_msi_group(soc, ring_type, ring_num,
					  &reg_msi_grp_num,
					  nf_irq_support,
					  &nf_msi_grp_num);
	if (ret < 0) {
		dp_init_info("%pK: ring not part of an ext_group; ring_type: %d,ring_num %d",
			     soc, ring_type, ring_num);
		ring_params->msi_addr = 0;
		ring_params->msi_data = 0;
		dp_srng_set_msi2_ring_params(soc, ring_params, 0, 0);
		return;
	}

	if (reg_msi_grp_num < 0) {
		dp_init_info("%pK: ring not part of an ext_group; ring_type: %d,ring_num %d",
			     soc, ring_type, ring_num);
		ring_params->msi_addr = 0;
		ring_params->msi_data = 0;
		goto configure_msi2;
	}

	if (dp_is_msi_group_number_invalid(reg_msi_grp_num, msi_data_count)) {
		dp_init_warn("%pK: 2 msi_groups will share an msi; msi_group_num %d",
			     soc, reg_msi_grp_num);
		QDF_ASSERT(0);
	}

	pld_get_msi_address(soc->osdev->dev, &addr_low, &addr_high);

	ring_params->msi_addr = addr_low;
	ring_params->msi_addr |= (qdf_dma_addr_t)(((uint64_t)addr_high) << 32);
	ring_params->msi_data = (reg_msi_grp_num % msi_data_count)
		+ msi_data_start;
	ring_params->flags |= HAL_SRNG_MSI_INTR;

	dp_debug("ring type %u ring_num %u msi->data %u msi_addr %llx",
		 ring_type, ring_num, ring_params->msi_data,
		 (uint64_t)ring_params->msi_addr);

configure_msi2:
	if (!nf_irq_support) {
		dp_srng_set_msi2_ring_params(soc, ring_params, 0, 0);
		return;
	}

	dp_srng_msi2_setup(soc, ring_params, ring_type, ring_num,
			   nf_msi_grp_num);
}

#ifdef FEATURE_AST
/**
 * dp_print_peer_ast_entries() - Dump AST entries of peer
 * @soc: Datapath soc handle
 * @peer: Datapath peer
 * @arg: argument to iterate function
 *
 * return void
 */
static void
dp_print_peer_ast_entries(struct dp_soc *soc, struct dp_peer *peer, void *arg)
{
	struct dp_ast_entry *ase, *tmp_ase;
	uint32_t num_entries = 0;
	char type[CDP_TXRX_AST_TYPE_MAX][10] = {
			"NONE", "STATIC", "SELF", "WDS", "HMWDS", "BSS",
			"DA", "HMWDS_SEC"};

	DP_PEER_ITERATE_ASE_LIST(peer, ase, tmp_ase) {
	    DP_PRINT_STATS("%6d mac_addr = "QDF_MAC_ADDR_FMT
		    " peer_mac_addr = "QDF_MAC_ADDR_FMT
		    " peer_id = %u"
		    " type = %s"
		    " next_hop = %d"
		    " is_active = %d"
		    " ast_idx = %d"
		    " ast_hash = %d"
		    " delete_in_progress = %d"
		    " pdev_id = %d"
		    " vdev_id = %d",
		    ++num_entries,
		    QDF_MAC_ADDR_REF(ase->mac_addr.raw),
		    QDF_MAC_ADDR_REF(peer->mac_addr.raw),
		    ase->peer_id,
		    type[ase->type],
		    ase->next_hop,
		    ase->is_active,
		    ase->ast_idx,
		    ase->ast_hash_value,
		    ase->delete_in_progress,
		    ase->pdev_id,
		    ase->vdev_id);
	}
}

/**
 * dp_print_ast_stats() - Dump AST table contents
 * @soc: Datapath soc handle
 *
 * return void
 */
void dp_print_ast_stats(struct dp_soc *soc)
{
	DP_PRINT_STATS("AST Stats:");
	DP_PRINT_STATS("	Entries Added   = %d", soc->stats.ast.added);
	DP_PRINT_STATS("	Entries Deleted = %d", soc->stats.ast.deleted);
	DP_PRINT_STATS("	Entries Agedout = %d", soc->stats.ast.aged_out);
	DP_PRINT_STATS("	Entries MAP ERR  = %d", soc->stats.ast.map_err);
	DP_PRINT_STATS("	Entries Mismatch ERR  = %d",
		       soc->stats.ast.ast_mismatch);

	DP_PRINT_STATS("AST Table:");

	qdf_spin_lock_bh(&soc->ast_lock);

	dp_soc_iterate_peer(soc, dp_print_peer_ast_entries, NULL,
			    DP_MOD_ID_GENERIC_STATS);

	qdf_spin_unlock_bh(&soc->ast_lock);
}
#else
void dp_print_ast_stats(struct dp_soc *soc)
{
	DP_PRINT_STATS("AST Stats not available.Enable FEATURE_AST");
	return;
}
#endif

/**
 * dp_print_peer_info() - Dump peer info
 * @soc: Datapath soc handle
 * @peer: Datapath peer handle
 * @arg: argument to iter function
 *
 * return void
 */
static void
dp_print_peer_info(struct dp_soc *soc, struct dp_peer *peer, void *arg)
{
	DP_PRINT_STATS("    peer_mac_addr = "QDF_MAC_ADDR_FMT
		       " nawds_enabled = %d"
		       " bss_peer = %d"
		       " wds_enabled = %d"
		       " tx_cap_enabled = %d"
		       " rx_cap_enabled = %d"
		       " peer id = %d",
		       QDF_MAC_ADDR_REF(peer->mac_addr.raw),
		       peer->nawds_enabled,
		       peer->bss_peer,
		       peer->wds_enabled,
		       peer->tx_cap_enabled,
		       peer->rx_cap_enabled,
		       peer->peer_id);
}

/**
 * dp_print_peer_table() - Dump all Peer stats
 * @vdev: Datapath Vdev handle
 *
 * return void
 */
static void dp_print_peer_table(struct dp_vdev *vdev)
{
	DP_PRINT_STATS("Dumping Peer Table  Stats:");
	dp_vdev_iterate_peer(vdev, dp_print_peer_info, NULL,
			     DP_MOD_ID_GENERIC_STATS);
}

#ifdef WLAN_DP_PER_RING_TYPE_CONFIG
/**
 * dp_srng_configure_interrupt_thresholds() - Retrieve interrupt
 * threshold values from the wlan_srng_cfg table for each ring type
 * @soc: device handle
 * @ring_params: per ring specific parameters
 * @ring_type: Ring type
 * @ring_num: Ring number for a given ring type
 *
 * Fill the ring params with the interrupt threshold
 * configuration parameters available in the per ring type wlan_srng_cfg
 * table.
 *
 * Return: None
 */
static void
dp_srng_configure_interrupt_thresholds(struct dp_soc *soc,
				       struct hal_srng_params *ring_params,
				       int ring_type, int ring_num,
				       int num_entries)
{
	if (ring_type == REO_DST) {
		ring_params->intr_timer_thres_us =
			wlan_cfg_get_int_timer_threshold_rx(soc->wlan_cfg_ctx);
		ring_params->intr_batch_cntr_thres_entries =
			wlan_cfg_get_int_batch_threshold_rx(soc->wlan_cfg_ctx);
	} else if (ring_type == WBM2SW_RELEASE && (ring_num == 3)) {
		ring_params->intr_timer_thres_us =
				wlan_cfg_get_int_timer_threshold_other(soc->wlan_cfg_ctx);
		ring_params->intr_batch_cntr_thres_entries =
				wlan_cfg_get_int_batch_threshold_other(soc->wlan_cfg_ctx);
	} else {
		ring_params->intr_timer_thres_us =
				soc->wlan_srng_cfg[ring_type].timer_threshold;
		ring_params->intr_batch_cntr_thres_entries =
				soc->wlan_srng_cfg[ring_type].batch_count_threshold;
	}
	ring_params->low_threshold =
			soc->wlan_srng_cfg[ring_type].low_threshold;
	if (ring_params->low_threshold)
		ring_params->flags |= HAL_SRNG_LOW_THRES_INTR_ENABLE;

	dp_srng_configure_nf_interrupt_thresholds(soc, ring_params, ring_type);
}
#else
static void
dp_srng_configure_interrupt_thresholds(struct dp_soc *soc,
				       struct hal_srng_params *ring_params,
				       int ring_type, int ring_num,
				       int num_entries)
{
	if (ring_type == REO_DST) {
		ring_params->intr_timer_thres_us =
			wlan_cfg_get_int_timer_threshold_rx(soc->wlan_cfg_ctx);
		ring_params->intr_batch_cntr_thres_entries =
			wlan_cfg_get_int_batch_threshold_rx(soc->wlan_cfg_ctx);
	} else if (ring_type == WBM2SW_RELEASE && (ring_num < 3)) {
		ring_params->intr_timer_thres_us =
			wlan_cfg_get_int_timer_threshold_tx(soc->wlan_cfg_ctx);
		ring_params->intr_batch_cntr_thres_entries =
			wlan_cfg_get_int_batch_threshold_tx(soc->wlan_cfg_ctx);
	} else {
		ring_params->intr_timer_thres_us =
			wlan_cfg_get_int_timer_threshold_other(soc->wlan_cfg_ctx);
		ring_params->intr_batch_cntr_thres_entries =
			wlan_cfg_get_int_batch_threshold_other(soc->wlan_cfg_ctx);
	}

	/* Enable low threshold interrupts for rx buffer rings (regular and
	 * monitor buffer rings.
	 * TODO: See if this is required for any other ring
	 */
	if ((ring_type == RXDMA_BUF) || (ring_type == RXDMA_MONITOR_BUF) ||
	    (ring_type == RXDMA_MONITOR_STATUS ||
	    (ring_type == TX_MONITOR_BUF))) {
		/* TODO: Setting low threshold to 1/8th of ring size
		 * see if this needs to be configurable
		 */
		ring_params->low_threshold = num_entries >> 3;
		ring_params->intr_timer_thres_us =
			wlan_cfg_get_int_timer_threshold_rx(soc->wlan_cfg_ctx);
		ring_params->flags |= HAL_SRNG_LOW_THRES_INTR_ENABLE;
		ring_params->intr_batch_cntr_thres_entries = 0;
	}

	/* During initialisation monitor rings are only filled with
	 * MON_BUF_MIN_ENTRIES entries. So low threshold needs to be set to
	 * a value less than that. Low threshold value is reconfigured again
	 * to 1/8th of the ring size when monitor vap is created.
	 */
	if (ring_type == RXDMA_MONITOR_BUF)
		ring_params->low_threshold = MON_BUF_MIN_ENTRIES >> 1;

	/* In case of PCI chipsets, we dont have PPDU end interrupts,
	 * so MONITOR STATUS ring is reaped by receiving MSI from srng.
	 * Keep batch threshold as 8 so that interrupt is received for
	 * every 4 packets in MONITOR_STATUS ring
	 */
	if ((ring_type == RXDMA_MONITOR_STATUS) &&
	    (soc->intr_mode == DP_INTR_MSI))
		ring_params->intr_batch_cntr_thres_entries = 4;
}
#endif

#ifdef DP_MEM_PRE_ALLOC

void *dp_context_alloc_mem(struct dp_soc *soc, enum dp_ctxt_type ctxt_type,
			   size_t ctxt_size)
{
	void *ctxt_mem;

	if (!soc->cdp_soc.ol_ops->dp_prealloc_get_context) {
		dp_warn("dp_prealloc_get_context null!");
		goto dynamic_alloc;
	}

	ctxt_mem = soc->cdp_soc.ol_ops->dp_prealloc_get_context(ctxt_type);

	if (ctxt_mem)
		goto end;

dynamic_alloc:
	dp_info("Pre-alloc of ctxt failed. Dynamic allocation");
	ctxt_mem = qdf_mem_malloc(ctxt_size);
end:
	return ctxt_mem;
}

void dp_context_free_mem(struct dp_soc *soc, enum dp_ctxt_type ctxt_type,
			 void *vaddr)
{
	QDF_STATUS status;

	if (soc->cdp_soc.ol_ops->dp_prealloc_put_context) {
		status = soc->cdp_soc.ol_ops->dp_prealloc_put_context(
								ctxt_type,
								vaddr);
	} else {
		dp_warn("dp_prealloc_get_context null!");
		status = QDF_STATUS_E_NOSUPPORT;
	}

	if (QDF_IS_STATUS_ERROR(status)) {
		dp_info("Context not pre-allocated");
		qdf_mem_free(vaddr);
	}
}

static inline
void *dp_srng_aligned_mem_alloc_consistent(struct dp_soc *soc,
					   struct dp_srng *srng,
					   uint32_t ring_type)
{
	void *mem;

	qdf_assert(!srng->is_mem_prealloc);

	if (!soc->cdp_soc.ol_ops->dp_prealloc_get_consistent) {
		dp_warn("dp_prealloc_get_consistent is null!");
		goto qdf;
	}

	mem =
		soc->cdp_soc.ol_ops->dp_prealloc_get_consistent
						(&srng->alloc_size,
						 &srng->base_vaddr_unaligned,
						 &srng->base_paddr_unaligned,
						 &srng->base_paddr_aligned,
						 DP_RING_BASE_ALIGN, ring_type);

	if (mem) {
		srng->is_mem_prealloc = true;
		goto end;
	}
qdf:
	mem =  qdf_aligned_mem_alloc_consistent(soc->osdev, &srng->alloc_size,
						&srng->base_vaddr_unaligned,
						&srng->base_paddr_unaligned,
						&srng->base_paddr_aligned,
						DP_RING_BASE_ALIGN);
end:
	dp_info("%s memory %pK dp_srng %pK ring_type %d alloc_size %d num_entries %d",
		srng->is_mem_prealloc ? "pre-alloc" : "dynamic-alloc", mem,
		srng, ring_type, srng->alloc_size, srng->num_entries);
	return mem;
}

static inline void dp_srng_mem_free_consistent(struct dp_soc *soc,
					       struct dp_srng *srng)
{
	if (srng->is_mem_prealloc) {
		if (!soc->cdp_soc.ol_ops->dp_prealloc_put_consistent) {
			dp_warn("dp_prealloc_put_consistent is null!");
			QDF_BUG(0);
			return;
		}
		soc->cdp_soc.ol_ops->dp_prealloc_put_consistent
						(srng->alloc_size,
						 srng->base_vaddr_unaligned,
						 srng->base_paddr_unaligned);

	} else {
		qdf_mem_free_consistent(soc->osdev, soc->osdev->dev,
					srng->alloc_size,
					srng->base_vaddr_unaligned,
					srng->base_paddr_unaligned, 0);
	}
}

void dp_desc_multi_pages_mem_alloc(struct dp_soc *soc,
				   enum dp_desc_type desc_type,
				   struct qdf_mem_multi_page_t *pages,
				   size_t element_size,
				   uint16_t element_num,
				   qdf_dma_context_t memctxt,
				   bool cacheable)
{
	if (!soc->cdp_soc.ol_ops->dp_get_multi_pages) {
		dp_warn("dp_get_multi_pages is null!");
		goto qdf;
	}

	pages->num_pages = 0;
	pages->is_mem_prealloc = 0;
	soc->cdp_soc.ol_ops->dp_get_multi_pages(desc_type,
						element_size,
						element_num,
						pages,
						cacheable);
	if (pages->num_pages)
		goto end;

qdf:
	qdf_mem_multi_pages_alloc(soc->osdev, pages, element_size,
				  element_num, memctxt, cacheable);
end:
	dp_info("%s desc_type %d element_size %d element_num %d cacheable %d",
		pages->is_mem_prealloc ? "pre-alloc" : "dynamic-alloc",
		desc_type, (int)element_size, element_num, cacheable);
}

void dp_desc_multi_pages_mem_free(struct dp_soc *soc,
				  enum dp_desc_type desc_type,
				  struct qdf_mem_multi_page_t *pages,
				  qdf_dma_context_t memctxt,
				  bool cacheable)
{
	if (pages->is_mem_prealloc) {
		if (!soc->cdp_soc.ol_ops->dp_put_multi_pages) {
			dp_warn("dp_put_multi_pages is null!");
			QDF_BUG(0);
			return;
		}

		soc->cdp_soc.ol_ops->dp_put_multi_pages(desc_type, pages);
		qdf_mem_zero(pages, sizeof(*pages));
	} else {
		qdf_mem_multi_pages_free(soc->osdev, pages,
					 memctxt, cacheable);
	}
}

#else

static inline
void *dp_srng_aligned_mem_alloc_consistent(struct dp_soc *soc,
					   struct dp_srng *srng,
					   uint32_t ring_type)

{
	void *mem;

	mem = qdf_aligned_mem_alloc_consistent(soc->osdev, &srng->alloc_size,
					       &srng->base_vaddr_unaligned,
					       &srng->base_paddr_unaligned,
					       &srng->base_paddr_aligned,
					       DP_RING_BASE_ALIGN);
	if (mem)
		qdf_mem_set(srng->base_vaddr_unaligned, 0, srng->alloc_size);

	return mem;
}

static inline void dp_srng_mem_free_consistent(struct dp_soc *soc,
					       struct dp_srng *srng)
{
	qdf_mem_free_consistent(soc->osdev, soc->osdev->dev,
				srng->alloc_size,
				srng->base_vaddr_unaligned,
				srng->base_paddr_unaligned, 0);
}

#endif /* DP_MEM_PRE_ALLOC */

/*
 * dp_srng_free() - Free SRNG memory
 * @soc  : Data path soc handle
 * @srng : SRNG pointer
 *
 * return: None
 */
void dp_srng_free(struct dp_soc *soc, struct dp_srng *srng)
{
	if (srng->alloc_size && srng->base_vaddr_unaligned) {
		if (!srng->cached) {
			dp_srng_mem_free_consistent(soc, srng);
		} else {
			qdf_mem_free(srng->base_vaddr_unaligned);
		}
		srng->alloc_size = 0;
		srng->base_vaddr_unaligned = NULL;
	}
	srng->hal_srng = NULL;
}

qdf_export_symbol(dp_srng_free);

#ifdef DISABLE_MON_RING_MSI_CFG
/*
 * dp_skip_msi_cfg() - Check if msi cfg has to be skipped for ring_type
 * @ring_type: sring type
 *
 * Return: True if msi cfg should be skipped for srng type else false
 */
static inline bool dp_skip_msi_cfg(struct dp_soc *soc, int ring_type)
{
	if (ring_type == RXDMA_MONITOR_STATUS)
		return true;

	return false;
}
#else
#ifdef DP_CON_MON_MSI_ENABLED
static inline bool dp_skip_msi_cfg(struct dp_soc *soc, int ring_type)
{
	if (soc->cdp_soc.ol_ops->get_con_mode &&
	    soc->cdp_soc.ol_ops->get_con_mode() == QDF_GLOBAL_MONITOR_MODE) {
		if (ring_type == REO_DST)
			return true;
	} else if (ring_type == RXDMA_MONITOR_STATUS) {
		return true;
	}

	return false;
}
#else
static inline bool dp_skip_msi_cfg(struct dp_soc *soc, int ring_type)
{
	return false;
}
#endif /* DP_CON_MON_MSI_ENABLED */
#endif /* DISABLE_MON_RING_MSI_CFG */

/*
 * dp_srng_init() - Initialize SRNG
 * @soc  : Data path soc handle
 * @srng : SRNG pointer
 * @ring_type : Ring Type
 * @ring_num: Ring number
 * @mac_id: mac_id
 *
 * return: QDF_STATUS
 */
QDF_STATUS dp_srng_init(struct dp_soc *soc, struct dp_srng *srng,
			int ring_type, int ring_num, int mac_id)
{
	hal_soc_handle_t hal_soc = soc->hal_soc;
	struct hal_srng_params ring_params;

	if (srng->hal_srng) {
		dp_init_err("%pK: Ring type: %d, num:%d is already initialized",
			    soc, ring_type, ring_num);
		return QDF_STATUS_SUCCESS;
	}

	/* memset the srng ring to zero */
	qdf_mem_zero(srng->base_vaddr_unaligned, srng->alloc_size);

	qdf_mem_zero(&ring_params, sizeof(struct hal_srng_params));
	ring_params.ring_base_paddr = srng->base_paddr_aligned;
	ring_params.ring_base_vaddr = srng->base_vaddr_aligned;

	ring_params.num_entries = srng->num_entries;

	dp_info("Ring type: %d, num:%d vaddr %pK paddr %pK entries %u",
		ring_type, ring_num,
		(void *)ring_params.ring_base_vaddr,
		(void *)ring_params.ring_base_paddr,
		ring_params.num_entries);

	if (soc->intr_mode == DP_INTR_MSI && !dp_skip_msi_cfg(soc, ring_type)) {
		dp_srng_msi_setup(soc, &ring_params, ring_type, ring_num);
		dp_verbose_debug("Using MSI for ring_type: %d, ring_num %d",
				 ring_type, ring_num);
	} else {
		ring_params.msi_data = 0;
		ring_params.msi_addr = 0;
		dp_srng_set_msi2_ring_params(soc, &ring_params, 0, 0);
		dp_verbose_debug("Skipping MSI for ring_type: %d, ring_num %d",
				 ring_type, ring_num);
	}

	dp_srng_configure_interrupt_thresholds(soc, &ring_params,
					       ring_type, ring_num,
					       srng->num_entries);

	dp_srng_set_nf_thresholds(soc, srng, &ring_params);

	if (srng->cached)
		ring_params.flags |= HAL_SRNG_CACHED_DESC;

	srng->hal_srng = hal_srng_setup(hal_soc, ring_type, ring_num,
					mac_id, &ring_params);

	if (!srng->hal_srng) {
		dp_srng_free(soc, srng);
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

qdf_export_symbol(dp_srng_init);

/*
 * dp_srng_alloc() - Allocate memory for SRNG
 * @soc  : Data path soc handle
 * @srng : SRNG pointer
 * @ring_type : Ring Type
 * @num_entries: Number of entries
 * @cached: cached flag variable
 *
 * return: QDF_STATUS
 */
QDF_STATUS dp_srng_alloc(struct dp_soc *soc, struct dp_srng *srng,
			 int ring_type, uint32_t num_entries,
			 bool cached)
{
	hal_soc_handle_t hal_soc = soc->hal_soc;
	uint32_t entry_size = hal_srng_get_entrysize(hal_soc, ring_type);
	uint32_t max_entries = hal_srng_max_entries(hal_soc, ring_type);

	if (srng->base_vaddr_unaligned) {
		dp_init_err("%pK: Ring type: %d, is already allocated",
			    soc, ring_type);
		return QDF_STATUS_SUCCESS;
	}

	num_entries = (num_entries > max_entries) ? max_entries : num_entries;
	srng->hal_srng = NULL;
	srng->alloc_size = num_entries * entry_size;
	srng->num_entries = num_entries;
	srng->cached = cached;

	if (!cached) {
		srng->base_vaddr_aligned =
		    dp_srng_aligned_mem_alloc_consistent(soc,
							 srng,
							 ring_type);
	} else {
		srng->base_vaddr_aligned = qdf_aligned_malloc(
					&srng->alloc_size,
					&srng->base_vaddr_unaligned,
					&srng->base_paddr_unaligned,
					&srng->base_paddr_aligned,
					DP_RING_BASE_ALIGN);
	}

	if (!srng->base_vaddr_aligned)
		return QDF_STATUS_E_NOMEM;

	return QDF_STATUS_SUCCESS;
}

qdf_export_symbol(dp_srng_alloc);

/*
 * dp_srng_deinit() - Internal function to deinit SRNG rings used by data path
 * @soc: DP SOC handle
 * @srng: source ring structure
 * @ring_type: type of ring
 * @ring_num: ring number
 *
 * Return: None
 */
void dp_srng_deinit(struct dp_soc *soc, struct dp_srng *srng,
		    int ring_type, int ring_num)
{
	if (!srng->hal_srng) {
		dp_init_err("%pK: Ring type: %d, num:%d not setup",
			    soc, ring_type, ring_num);
		return;
	}

	hal_srng_cleanup(soc->hal_soc, srng->hal_srng);
	srng->hal_srng = NULL;
}

qdf_export_symbol(dp_srng_deinit);

/* TODO: Need this interface from HIF */
void *hif_get_hal_handle(struct hif_opaque_softc *hif_handle);

#ifdef WLAN_FEATURE_DP_EVENT_HISTORY
int dp_srng_access_start(struct dp_intr *int_ctx, struct dp_soc *dp_soc,
			 hal_ring_handle_t hal_ring_hdl)
{
	hal_soc_handle_t hal_soc = dp_soc->hal_soc;
	uint32_t hp, tp;
	uint8_t ring_id;

	if (!int_ctx)
		return dp_hal_srng_access_start(hal_soc, hal_ring_hdl);

	hal_get_sw_hptp(hal_soc, hal_ring_hdl, &tp, &hp);
	ring_id = hal_srng_ring_id_get(hal_ring_hdl);

	hif_record_event(dp_soc->hif_handle, int_ctx->dp_intr_id,
			 ring_id, hp, tp, HIF_EVENT_SRNG_ACCESS_START);

	return dp_hal_srng_access_start(hal_soc, hal_ring_hdl);
}

void dp_srng_access_end(struct dp_intr *int_ctx, struct dp_soc *dp_soc,
			hal_ring_handle_t hal_ring_hdl)
{
	hal_soc_handle_t hal_soc = dp_soc->hal_soc;
	uint32_t hp, tp;
	uint8_t ring_id;

	if (!int_ctx)
		return dp_hal_srng_access_end(hal_soc, hal_ring_hdl);

	hal_get_sw_hptp(hal_soc, hal_ring_hdl, &tp, &hp);
	ring_id = hal_srng_ring_id_get(hal_ring_hdl);

	hif_record_event(dp_soc->hif_handle, int_ctx->dp_intr_id,
			 ring_id, hp, tp, HIF_EVENT_SRNG_ACCESS_END);

	return dp_hal_srng_access_end(hal_soc, hal_ring_hdl);
}

static inline void dp_srng_record_timer_entry(struct dp_soc *dp_soc,
					      uint8_t hist_group_id)
{
	hif_record_event(dp_soc->hif_handle, hist_group_id,
			 0, 0, 0, HIF_EVENT_TIMER_ENTRY);
}

static inline void dp_srng_record_timer_exit(struct dp_soc *dp_soc,
					     uint8_t hist_group_id)
{
	hif_record_event(dp_soc->hif_handle, hist_group_id,
			 0, 0, 0, HIF_EVENT_TIMER_EXIT);
}
#else

static inline void dp_srng_record_timer_entry(struct dp_soc *dp_soc,
					      uint8_t hist_group_id)
{
}

static inline void dp_srng_record_timer_exit(struct dp_soc *dp_soc,
					     uint8_t hist_group_id)
{
}

#endif /* WLAN_FEATURE_DP_EVENT_HISTORY */

/*
 * dp_should_timer_irq_yield() - Decide if the bottom half should yield
 * @soc: DP soc handle
 * @work_done: work done in softirq context
 * @start_time: start time for the softirq
 *
 * Return: enum with yield code
 */
enum timer_yield_status
dp_should_timer_irq_yield(struct dp_soc *soc, uint32_t work_done,
			  uint64_t start_time)
{
	uint64_t cur_time = qdf_get_log_timestamp();

	if (!work_done)
		return DP_TIMER_WORK_DONE;

	if (cur_time - start_time > DP_MAX_TIMER_EXEC_TIME_TICKS)
		return DP_TIMER_TIME_EXHAUST;

	return DP_TIMER_NO_YIELD;
}

qdf_export_symbol(dp_should_timer_irq_yield);

#ifdef DP_CON_MON_MSI_ENABLED
static int dp_process_rxdma_dst_ring(struct dp_soc *soc,
				     struct dp_intr *int_ctx,
				     int mac_for_pdev,
				     int total_budget)
{
	if (dp_soc_get_con_mode(soc) == QDF_GLOBAL_MONITOR_MODE)
		return dp_monitor_process(soc, int_ctx, mac_for_pdev,
					  total_budget);
	else
		return dp_rxdma_err_process(int_ctx, soc, mac_for_pdev,
					    total_budget);
}
#else
static int dp_process_rxdma_dst_ring(struct dp_soc *soc,
				     struct dp_intr *int_ctx,
				     int mac_for_pdev,
				     int total_budget)
{
	return dp_rxdma_err_process(int_ctx, soc, mac_for_pdev,
				    total_budget);
}
#endif

/**
 * dp_process_lmac_rings() - Process LMAC rings
 * @int_ctx: interrupt context
 * @total_budget: budget of work which can be done
 *
 * Return: work done
 */
static int dp_process_lmac_rings(struct dp_intr *int_ctx, int total_budget)
{
	struct dp_intr_stats *intr_stats = &int_ctx->intr_stats;
	struct dp_soc *soc = int_ctx->soc;
	uint32_t remaining_quota = total_budget;
	struct dp_pdev *pdev = NULL;
	uint32_t work_done  = 0;
	int budget = total_budget;
	int ring = 0;

	/* Process LMAC interrupts */
	for  (ring = 0 ; ring < MAX_NUM_LMAC_HW; ring++) {
		int mac_for_pdev = ring;

		pdev = dp_get_pdev_for_lmac_id(soc, mac_for_pdev);
		if (!pdev)
			continue;
		if (int_ctx->rx_mon_ring_mask & (1 << mac_for_pdev)) {
			work_done = dp_monitor_process(soc, int_ctx,
						       mac_for_pdev,
						       remaining_quota);
			if (work_done)
				intr_stats->num_rx_mon_ring_masks++;
			budget -= work_done;
			if (budget <= 0)
				goto budget_done;
			remaining_quota = budget;
		}

		if (int_ctx->tx_mon_ring_mask & (1 << mac_for_pdev)) {
			work_done = dp_tx_mon_process(soc, int_ctx,
						      mac_for_pdev,
						      remaining_quota);
			if (work_done)
				intr_stats->num_tx_mon_ring_masks++;
			budget -= work_done;
			if (budget <= 0)
				goto budget_done;
			remaining_quota = budget;
		}

		if (int_ctx->rxdma2host_ring_mask &
				(1 << mac_for_pdev)) {
			work_done = dp_process_rxdma_dst_ring(soc, int_ctx,
							      mac_for_pdev,
							      remaining_quota);
			if (work_done)
				intr_stats->num_rxdma2host_ring_masks++;
			budget -=  work_done;
			if (budget <= 0)
				goto budget_done;
			remaining_quota = budget;
		}

		if (int_ctx->host2rxdma_ring_mask &
					(1 << mac_for_pdev)) {
			union dp_rx_desc_list_elem_t *desc_list = NULL;
			union dp_rx_desc_list_elem_t *tail = NULL;
			struct dp_srng *rx_refill_buf_ring;

			if (wlan_cfg_per_pdev_lmac_ring(soc->wlan_cfg_ctx))
				rx_refill_buf_ring =
					&soc->rx_refill_buf_ring[mac_for_pdev];
			else
				rx_refill_buf_ring =
					&soc->rx_refill_buf_ring[pdev->lmac_id];

			intr_stats->num_host2rxdma_ring_masks++;
			DP_STATS_INC(pdev, replenish.low_thresh_intrs,
				     1);
			dp_rx_buffers_replenish(soc, mac_for_pdev,
						rx_refill_buf_ring,
						&soc->rx_desc_buf[mac_for_pdev],
						0, &desc_list, &tail);
		}
	}

budget_done:
	return total_budget - budget;
}

#ifdef WLAN_FEATURE_NEAR_FULL_IRQ
/**
 * dp_service_near_full_srngs() - Bottom half handler to process the near
 *				full IRQ on a SRNG
 * @dp_ctx: Datapath SoC handle
 * @dp_budget: Number of SRNGs which can be processed in a single attempt
 *		without rescheduling
 *
 * Return: remaining budget/quota for the soc device
 */
static uint32_t dp_service_near_full_srngs(void *dp_ctx, uint32_t dp_budget)
{
	struct dp_intr *int_ctx = (struct dp_intr *)dp_ctx;
	struct dp_soc *soc = int_ctx->soc;

	/*
	 * dp_service_near_full_srngs arch ops should be initialized always
	 * if the NEAR FULL IRQ feature is enabled.
	 */
	return soc->arch_ops.dp_service_near_full_srngs(soc, int_ctx,
							dp_budget);
}
#endif

#ifndef QCA_HOST_MODE_WIFI_DISABLED

/*
 * dp_service_srngs() - Top level interrupt handler for DP Ring interrupts
 * @dp_ctx: DP SOC handle
 * @budget: Number of frames/descriptors that can be processed in one shot
 *
 * Return: remaining budget/quota for the soc device
 */
static uint32_t dp_service_srngs(void *dp_ctx, uint32_t dp_budget)
{
	struct dp_intr *int_ctx = (struct dp_intr *)dp_ctx;
	struct dp_intr_stats *intr_stats = &int_ctx->intr_stats;
	struct dp_soc *soc = int_ctx->soc;
	int ring = 0;
	int index;
	uint32_t work_done  = 0;
	int budget = dp_budget;
	uint8_t tx_mask = int_ctx->tx_ring_mask;
	uint8_t rx_mask = int_ctx->rx_ring_mask;
	uint8_t rx_err_mask = int_ctx->rx_err_ring_mask;
	uint8_t rx_wbm_rel_mask = int_ctx->rx_wbm_rel_ring_mask;
	uint8_t reo_status_mask = int_ctx->reo_status_ring_mask;
	uint32_t remaining_quota = dp_budget;

	dp_verbose_debug("tx %x rx %x rx_err %x rx_wbm_rel %x reo_status %x rx_mon_ring %x host2rxdma %x rxdma2host %x\n",
			 tx_mask, rx_mask, rx_err_mask, rx_wbm_rel_mask,
			 reo_status_mask,
			 int_ctx->rx_mon_ring_mask,
			 int_ctx->host2rxdma_ring_mask,
			 int_ctx->rxdma2host_ring_mask);

	/* Process Tx completion interrupts first to return back buffers */
	for (index = 0; index < soc->num_tcl_data_rings; index++) {
		if (!(1 << wlan_cfg_get_wbm_ring_num_for_index(soc->wlan_cfg_ctx, index) & tx_mask))
			continue;
		work_done = dp_tx_comp_handler(int_ctx,
					       soc,
					       soc->tx_comp_ring[index].hal_srng,
					       index, remaining_quota);
		if (work_done) {
			intr_stats->num_tx_ring_masks[index]++;
			dp_verbose_debug("tx mask 0x%x index %d, budget %d, work_done %d",
					 tx_mask, index, budget,
					 work_done);
		}
		budget -= work_done;
		if (budget <= 0)
			goto budget_done;

		remaining_quota = budget;
	}

	/* Process REO Exception ring interrupt */
	if (rx_err_mask) {
		work_done = dp_rx_err_process(int_ctx, soc,
					      soc->reo_exception_ring.hal_srng,
					      remaining_quota);

		if (work_done) {
			intr_stats->num_rx_err_ring_masks++;
			dp_verbose_debug("REO Exception Ring: work_done %d budget %d",
					 work_done, budget);
		}

		budget -=  work_done;
		if (budget <= 0) {
			goto budget_done;
		}
		remaining_quota = budget;
	}

	/* Process Rx WBM release ring interrupt */
	if (rx_wbm_rel_mask) {
		work_done = dp_rx_wbm_err_process(int_ctx, soc,
						  soc->rx_rel_ring.hal_srng,
						  remaining_quota);

		if (work_done) {
			intr_stats->num_rx_wbm_rel_ring_masks++;
			dp_verbose_debug("WBM Release Ring: work_done %d budget %d",
					 work_done, budget);
		}

		budget -=  work_done;
		if (budget <= 0) {
			goto budget_done;
		}
		remaining_quota = budget;
	}

	/* Process Rx interrupts */
	if (rx_mask) {
		for (ring = 0; ring < soc->num_reo_dest_rings; ring++) {
			if (!(rx_mask & (1 << ring)))
				continue;
			work_done = soc->arch_ops.dp_rx_process(int_ctx,
						  soc->reo_dest_ring[ring].hal_srng,
						  ring,
						  remaining_quota);
			if (work_done) {
				intr_stats->num_rx_ring_masks[ring]++;
				dp_verbose_debug("rx mask 0x%x ring %d, work_done %d budget %d",
						 rx_mask, ring,
						 work_done, budget);
				budget -=  work_done;
				if (budget <= 0)
					goto budget_done;
				remaining_quota = budget;
			}
		}
	}

	if (reo_status_mask) {
		if (dp_reo_status_ring_handler(int_ctx, soc))
			int_ctx->intr_stats.num_reo_status_ring_masks++;
	}

	if (qdf_unlikely(!dp_monitor_is_vdev_timer_running(soc))) {
		work_done = dp_process_lmac_rings(int_ctx, remaining_quota);
		if (work_done) {
			budget -=  work_done;
			if (budget <= 0)
				goto budget_done;
			remaining_quota = budget;
		}
	}

	qdf_lro_flush(int_ctx->lro_ctx);
	intr_stats->num_masks++;

budget_done:
	return dp_budget - budget;
}

#else /* QCA_HOST_MODE_WIFI_DISABLED */

/*
 * dp_service_srngs() - Top level handler for DP Monitor Ring interrupts
 * @dp_ctx: DP SOC handle
 * @budget: Number of frames/descriptors that can be processed in one shot
 *
 * Return: remaining budget/quota for the soc device
 */
static uint32_t dp_service_srngs(void *dp_ctx, uint32_t dp_budget)
{
	struct dp_intr *int_ctx = (struct dp_intr *)dp_ctx;
	struct dp_intr_stats *intr_stats = &int_ctx->intr_stats;
	struct dp_soc *soc = int_ctx->soc;
	uint32_t remaining_quota = dp_budget;
	uint32_t work_done  = 0;
	int budget = dp_budget;
	uint8_t reo_status_mask = int_ctx->reo_status_ring_mask;

	if (reo_status_mask) {
		if (dp_reo_status_ring_handler(int_ctx, soc))
			int_ctx->intr_stats.num_reo_status_ring_masks++;
	}

	if (qdf_unlikely(!dp_monitor_is_vdev_timer_running(soc))) {
		work_done = dp_process_lmac_rings(int_ctx, remaining_quota);
		if (work_done) {
			budget -=  work_done;
			if (budget <= 0)
				goto budget_done;
			remaining_quota = budget;
		}
	}

	qdf_lro_flush(int_ctx->lro_ctx);
	intr_stats->num_masks++;

budget_done:
	return dp_budget - budget;
}

#endif /* QCA_HOST_MODE_WIFI_DISABLED */

/* dp_interrupt_timer()- timer poll for interrupts
 *
 * @arg: SoC Handle
 *
 * Return:
 *
 */
static void dp_interrupt_timer(void *arg)
{
	struct dp_soc *soc = (struct dp_soc *) arg;
	struct dp_pdev *pdev = soc->pdev_list[0];
	enum timer_yield_status yield = DP_TIMER_NO_YIELD;
	uint32_t work_done  = 0, total_work_done = 0;
	int budget = 0xffff, i;
	uint32_t remaining_quota = budget;
	uint64_t start_time;
	uint32_t lmac_id = DP_MON_INVALID_LMAC_ID;
	uint8_t dp_intr_id = wlan_cfg_get_num_contexts(soc->wlan_cfg_ctx);
	uint32_t lmac_iter;
	int max_mac_rings = wlan_cfg_get_num_mac_rings(pdev->wlan_cfg_ctx);
	enum reg_wifi_band mon_band;

	/*
	 * this logic makes all data path interfacing rings (UMAC/LMAC)
	 * and Monitor rings polling mode when NSS offload is disabled
	 */
	if (wlan_cfg_is_poll_mode_enabled(soc->wlan_cfg_ctx) &&
	    !wlan_cfg_get_dp_soc_nss_cfg(soc->wlan_cfg_ctx)) {
		if (qdf_atomic_read(&soc->cmn_init_done)) {
			for (i = 0; i < wlan_cfg_get_num_contexts(
						soc->wlan_cfg_ctx); i++)
				dp_service_srngs(&soc->intr_ctx[i], 0xffff);

			qdf_timer_mod(&soc->int_timer, DP_INTR_POLL_TIMER_MS);
		}
		return;
	}

	if (!qdf_atomic_read(&soc->cmn_init_done))
		return;

	if (dp_monitor_is_chan_band_known(pdev)) {
		mon_band = dp_monitor_get_chan_band(pdev);
		lmac_id = pdev->ch_band_lmac_id_mapping[mon_band];
		if (qdf_likely(lmac_id != DP_MON_INVALID_LMAC_ID)) {
			dp_intr_id = soc->mon_intr_id_lmac_map[lmac_id];
			dp_srng_record_timer_entry(soc, dp_intr_id);
		}
	}

	start_time = qdf_get_log_timestamp();
	dp_update_num_mac_rings_for_dbs(soc, &max_mac_rings);

	while (yield == DP_TIMER_NO_YIELD) {
		for (lmac_iter = 0; lmac_iter < max_mac_rings; lmac_iter++) {
			if (lmac_iter == lmac_id)
				work_done = dp_monitor_process(soc,
						&soc->intr_ctx[dp_intr_id],
						lmac_iter, remaining_quota);
			else
				work_done =
					dp_monitor_drop_packets_for_mac(pdev,
							     lmac_iter,
							     remaining_quota);
			if (work_done) {
				budget -=  work_done;
				if (budget <= 0) {
					yield = DP_TIMER_WORK_EXHAUST;
					goto budget_done;
				}
				remaining_quota = budget;
				total_work_done += work_done;
			}
		}

		yield = dp_should_timer_irq_yield(soc, total_work_done,
						  start_time);
		total_work_done = 0;
	}

budget_done:
	if (yield == DP_TIMER_WORK_EXHAUST ||
	    yield == DP_TIMER_TIME_EXHAUST)
		qdf_timer_mod(&soc->int_timer, 1);
	else
		qdf_timer_mod(&soc->int_timer, DP_INTR_POLL_TIMER_MS);

	if (lmac_id != DP_MON_INVALID_LMAC_ID)
		dp_srng_record_timer_exit(soc, dp_intr_id);
}

#ifdef WLAN_FEATURE_DP_EVENT_HISTORY
static inline bool dp_is_mon_mask_valid(struct dp_soc *soc,
					struct dp_intr *intr_ctx)
{
	if (intr_ctx->rx_mon_ring_mask)
		return true;

	return false;
}
#else
static inline bool dp_is_mon_mask_valid(struct dp_soc *soc,
					struct dp_intr *intr_ctx)
{
	return false;
}
#endif

/*
 * dp_soc_attach_poll() - Register handlers for DP interrupts
 * @txrx_soc: DP SOC handle
 *
 * Host driver will register for “DP_NUM_INTERRUPT_CONTEXTS” number of NAPI
 * contexts. Each NAPI context will have a tx_ring_mask , rx_ring_mask ,and
 * rx_monitor_ring mask to indicate the rings that are processed by the handler.
 *
 * Return: 0 for success, nonzero for failure.
 */
static QDF_STATUS dp_soc_attach_poll(struct cdp_soc_t *txrx_soc)
{
	struct dp_soc *soc = (struct dp_soc *)txrx_soc;
	int i;
	int lmac_id = 0;

	qdf_mem_set(&soc->mon_intr_id_lmac_map,
		    sizeof(soc->mon_intr_id_lmac_map), DP_MON_INVALID_LMAC_ID);
	soc->intr_mode = DP_INTR_POLL;

	for (i = 0; i < wlan_cfg_get_num_contexts(soc->wlan_cfg_ctx); i++) {
		soc->intr_ctx[i].dp_intr_id = i;
		soc->intr_ctx[i].tx_ring_mask =
			wlan_cfg_get_tx_ring_mask(soc->wlan_cfg_ctx, i);
		soc->intr_ctx[i].rx_ring_mask =
			wlan_cfg_get_rx_ring_mask(soc->wlan_cfg_ctx, i);
		soc->intr_ctx[i].rx_mon_ring_mask =
			wlan_cfg_get_rx_mon_ring_mask(soc->wlan_cfg_ctx, i);
		soc->intr_ctx[i].rx_err_ring_mask =
			wlan_cfg_get_rx_err_ring_mask(soc->wlan_cfg_ctx, i);
		soc->intr_ctx[i].rx_wbm_rel_ring_mask =
			wlan_cfg_get_rx_wbm_rel_ring_mask(soc->wlan_cfg_ctx, i);
		soc->intr_ctx[i].reo_status_ring_mask =
			wlan_cfg_get_reo_status_ring_mask(soc->wlan_cfg_ctx, i);
		soc->intr_ctx[i].rxdma2host_ring_mask =
			wlan_cfg_get_rxdma2host_ring_mask(soc->wlan_cfg_ctx, i);
		soc->intr_ctx[i].soc = soc;
		soc->intr_ctx[i].lro_ctx = qdf_lro_init();

		if (dp_is_mon_mask_valid(soc, &soc->intr_ctx[i])) {
			hif_event_history_init(soc->hif_handle, i);
			soc->mon_intr_id_lmac_map[lmac_id] = i;
			lmac_id++;
		}
	}

	qdf_timer_init(soc->osdev, &soc->int_timer,
			dp_interrupt_timer, (void *)soc,
			QDF_TIMER_TYPE_WAKE_APPS);

	return QDF_STATUS_SUCCESS;
}

/**
 * dp_soc_set_interrupt_mode() - Set the interrupt mode in soc
 * soc: DP soc handle
 *
 * Set the appropriate interrupt mode flag in the soc
 */
static void dp_soc_set_interrupt_mode(struct dp_soc *soc)
{
	uint32_t msi_base_data, msi_vector_start;
	int msi_vector_count, ret;

	soc->intr_mode = DP_INTR_INTEGRATED;

	if (!(soc->wlan_cfg_ctx->napi_enabled) ||
	    (dp_is_monitor_mode_using_poll(soc) &&
	     soc->cdp_soc.ol_ops->get_con_mode &&
	     soc->cdp_soc.ol_ops->get_con_mode() == QDF_GLOBAL_MONITOR_MODE)) {
		soc->intr_mode = DP_INTR_POLL;
	} else {
		ret = pld_get_user_msi_assignment(soc->osdev->dev, "DP",
						  &msi_vector_count,
						  &msi_base_data,
						  &msi_vector_start);
		if (ret)
			return;

		soc->intr_mode = DP_INTR_MSI;
	}
}

static QDF_STATUS dp_soc_interrupt_attach(struct cdp_soc_t *txrx_soc);
#if defined(DP_INTR_POLL_BOTH)
/*
 * dp_soc_interrupt_attach_wrapper() - Register handlers for DP interrupts
 * @txrx_soc: DP SOC handle
 *
 * Call the appropriate attach function based on the mode of operation.
 * This is a WAR for enabling monitor mode.
 *
 * Return: 0 for success. nonzero for failure.
 */
static QDF_STATUS dp_soc_interrupt_attach_wrapper(struct cdp_soc_t *txrx_soc)
{
	struct dp_soc *soc = (struct dp_soc *)txrx_soc;

	if (!(soc->wlan_cfg_ctx->napi_enabled) ||
	    (dp_is_monitor_mode_using_poll(soc) &&
	     soc->cdp_soc.ol_ops->get_con_mode &&
	     soc->cdp_soc.ol_ops->get_con_mode() ==
	     QDF_GLOBAL_MONITOR_MODE)) {
		dp_info("Poll mode");
		return dp_soc_attach_poll(txrx_soc);
	} else {
		dp_info("Interrupt  mode");
		return dp_soc_interrupt_attach(txrx_soc);
	}
}
#else
#if defined(DP_INTR_POLL_BASED) && DP_INTR_POLL_BASED
static QDF_STATUS dp_soc_interrupt_attach_wrapper(struct cdp_soc_t *txrx_soc)
{
	return dp_soc_attach_poll(txrx_soc);
}
#else
static QDF_STATUS dp_soc_interrupt_attach_wrapper(struct cdp_soc_t *txrx_soc)
{
	struct dp_soc *soc = (struct dp_soc *)txrx_soc;

	if (wlan_cfg_is_poll_mode_enabled(soc->wlan_cfg_ctx))
		return dp_soc_attach_poll(txrx_soc);
	else
		return dp_soc_interrupt_attach(txrx_soc);
}
#endif
#endif

static void dp_soc_interrupt_map_calculate_integrated(struct dp_soc *soc,
		int intr_ctx_num, int *irq_id_map, int *num_irq_r)
{
	int j;
	int num_irq = 0;

	int tx_mask =
		wlan_cfg_get_tx_ring_mask(soc->wlan_cfg_ctx, intr_ctx_num);
	int rx_mask =
		wlan_cfg_get_rx_ring_mask(soc->wlan_cfg_ctx, intr_ctx_num);
	int rx_mon_mask =
		wlan_cfg_get_rx_mon_ring_mask(soc->wlan_cfg_ctx, intr_ctx_num);
	int rx_err_ring_mask = wlan_cfg_get_rx_err_ring_mask(
					soc->wlan_cfg_ctx, intr_ctx_num);
	int rx_wbm_rel_ring_mask = wlan_cfg_get_rx_wbm_rel_ring_mask(
					soc->wlan_cfg_ctx, intr_ctx_num);
	int reo_status_ring_mask = wlan_cfg_get_reo_status_ring_mask(
					soc->wlan_cfg_ctx, intr_ctx_num);
	int rxdma2host_ring_mask = wlan_cfg_get_rxdma2host_ring_mask(
					soc->wlan_cfg_ctx, intr_ctx_num);
	int host2rxdma_ring_mask = wlan_cfg_get_host2rxdma_ring_mask(
					soc->wlan_cfg_ctx, intr_ctx_num);
	int host2rxdma_mon_ring_mask = wlan_cfg_get_host2rxdma_mon_ring_mask(
					soc->wlan_cfg_ctx, intr_ctx_num);

	soc->intr_mode = DP_INTR_INTEGRATED;

	for (j = 0; j < HIF_MAX_GRP_IRQ; j++) {

		if (tx_mask & (1 << j)) {
			irq_id_map[num_irq++] =
				(wbm2host_tx_completions_ring1 - j);
		}

		if (rx_mask & (1 << j)) {
			irq_id_map[num_irq++] =
				(reo2host_destination_ring1 - j);
		}

		if (rxdma2host_ring_mask & (1 << j)) {
			irq_id_map[num_irq++] =
				rxdma2host_destination_ring_mac1 - j;
		}

		if (host2rxdma_ring_mask & (1 << j)) {
			irq_id_map[num_irq++] =
				host2rxdma_host_buf_ring_mac1 -	j;
		}

		if (host2rxdma_mon_ring_mask & (1 << j)) {
			irq_id_map[num_irq++] =
				host2rxdma_monitor_ring1 - j;
		}

		if (rx_mon_mask & (1 << j)) {
			irq_id_map[num_irq++] =
				ppdu_end_interrupts_mac1 - j;
			irq_id_map[num_irq++] =
				rxdma2host_monitor_status_ring_mac1 - j;
			irq_id_map[num_irq++] =
				rxdma2host_monitor_destination_mac1 - j;
		}

		if (rx_wbm_rel_ring_mask & (1 << j))
			irq_id_map[num_irq++] = wbm2host_rx_release;

		if (rx_err_ring_mask & (1 << j))
			irq_id_map[num_irq++] = reo2host_exception;

		if (reo_status_ring_mask & (1 << j))
			irq_id_map[num_irq++] = reo2host_status;

	}
	*num_irq_r = num_irq;
}

static void dp_soc_interrupt_map_calculate_msi(struct dp_soc *soc,
		int intr_ctx_num, int *irq_id_map, int *num_irq_r,
		int msi_vector_count, int msi_vector_start)
{
	int tx_mask = wlan_cfg_get_tx_ring_mask(
					soc->wlan_cfg_ctx, intr_ctx_num);
	int rx_mask = wlan_cfg_get_rx_ring_mask(
					soc->wlan_cfg_ctx, intr_ctx_num);
	int rx_mon_mask = wlan_cfg_get_rx_mon_ring_mask(
					soc->wlan_cfg_ctx, intr_ctx_num);
	int tx_mon_mask = wlan_cfg_get_tx_mon_ring_mask(
					soc->wlan_cfg_ctx, intr_ctx_num);
	int rx_err_ring_mask = wlan_cfg_get_rx_err_ring_mask(
					soc->wlan_cfg_ctx, intr_ctx_num);
	int rx_wbm_rel_ring_mask = wlan_cfg_get_rx_wbm_rel_ring_mask(
					soc->wlan_cfg_ctx, intr_ctx_num);
	int reo_status_ring_mask = wlan_cfg_get_reo_status_ring_mask(
					soc->wlan_cfg_ctx, intr_ctx_num);
	int rxdma2host_ring_mask = wlan_cfg_get_rxdma2host_ring_mask(
					soc->wlan_cfg_ctx, intr_ctx_num);
	int host2rxdma_ring_mask = wlan_cfg_get_host2rxdma_ring_mask(
					soc->wlan_cfg_ctx, intr_ctx_num);
	int host2rxdma_mon_ring_mask = wlan_cfg_get_host2rxdma_mon_ring_mask(
					soc->wlan_cfg_ctx, intr_ctx_num);
	int rx_near_full_grp_1_mask =
		wlan_cfg_get_rx_near_full_grp_1_mask(soc->wlan_cfg_ctx,
						     intr_ctx_num);
	int rx_near_full_grp_2_mask =
		wlan_cfg_get_rx_near_full_grp_2_mask(soc->wlan_cfg_ctx,
						     intr_ctx_num);
	int tx_ring_near_full_mask =
		wlan_cfg_get_tx_ring_near_full_mask(soc->wlan_cfg_ctx,
						    intr_ctx_num);

	unsigned int vector =
		(intr_ctx_num % msi_vector_count) + msi_vector_start;
	int num_irq = 0;

	soc->intr_mode = DP_INTR_MSI;

	if (tx_mask | rx_mask | rx_mon_mask | tx_mon_mask | rx_err_ring_mask |
	    rx_wbm_rel_ring_mask | reo_status_ring_mask | rxdma2host_ring_mask |
	    host2rxdma_ring_mask | host2rxdma_mon_ring_mask |
	    rx_near_full_grp_1_mask | rx_near_full_grp_2_mask |
	    tx_ring_near_full_mask)
		irq_id_map[num_irq++] =
			pld_get_msi_irq(soc->osdev->dev, vector);

	*num_irq_r = num_irq;
}

static void dp_soc_interrupt_map_calculate(struct dp_soc *soc, int intr_ctx_num,
				    int *irq_id_map, int *num_irq)
{
	int msi_vector_count, ret;
	uint32_t msi_base_data, msi_vector_start;

	ret = pld_get_user_msi_assignment(soc->osdev->dev, "DP",
					    &msi_vector_count,
					    &msi_base_data,
					    &msi_vector_start);
	if (ret)
		return dp_soc_interrupt_map_calculate_integrated(soc,
				intr_ctx_num, irq_id_map, num_irq);

	else
		dp_soc_interrupt_map_calculate_msi(soc,
				intr_ctx_num, irq_id_map, num_irq,
				msi_vector_count, msi_vector_start);
}

#ifdef WLAN_FEATURE_NEAR_FULL_IRQ
/**
 * dp_soc_near_full_interrupt_attach() - Register handler for DP near fill irq
 * @soc: DP soc handle
 * @num_irq: IRQ number
 * @irq_id_map: IRQ map
 * intr_id: interrupt context ID
 *
 * Return: 0 for success. nonzero for failure.
 */
static inline int
dp_soc_near_full_interrupt_attach(struct dp_soc *soc, int num_irq,
				  int irq_id_map[], int intr_id)
{
	return hif_register_ext_group(soc->hif_handle,
				      num_irq, irq_id_map,
				      dp_service_near_full_srngs,
				      &soc->intr_ctx[intr_id], "dp_nf_intr",
				      HIF_EXEC_NAPI_TYPE,
				      QCA_NAPI_DEF_SCALE_BIN_SHIFT);
}
#else
static inline int
dp_soc_near_full_interrupt_attach(struct dp_soc *soc, int num_irq,
				  int *irq_id_map, int intr_id)
{
	return 0;
}
#endif

/*
 * dp_soc_interrupt_detach() - Deregister any allocations done for interrupts
 * @txrx_soc: DP SOC handle
 *
 * Return: none
 */
static void dp_soc_interrupt_detach(struct cdp_soc_t *txrx_soc)
{
	struct dp_soc *soc = (struct dp_soc *)txrx_soc;
	int i;

	if (soc->intr_mode == DP_INTR_POLL) {
		qdf_timer_free(&soc->int_timer);
	} else {
		hif_deconfigure_ext_group_interrupts(soc->hif_handle);
		hif_deregister_exec_group(soc->hif_handle, "dp_intr");
		hif_deregister_exec_group(soc->hif_handle, "dp_nf_intr");
	}

	for (i = 0; i < wlan_cfg_get_num_contexts(soc->wlan_cfg_ctx); i++) {
		soc->intr_ctx[i].tx_ring_mask = 0;
		soc->intr_ctx[i].rx_ring_mask = 0;
		soc->intr_ctx[i].rx_mon_ring_mask = 0;
		soc->intr_ctx[i].rx_err_ring_mask = 0;
		soc->intr_ctx[i].rx_wbm_rel_ring_mask = 0;
		soc->intr_ctx[i].reo_status_ring_mask = 0;
		soc->intr_ctx[i].rxdma2host_ring_mask = 0;
		soc->intr_ctx[i].host2rxdma_ring_mask = 0;
		soc->intr_ctx[i].host2rxdma_mon_ring_mask = 0;
		soc->intr_ctx[i].rx_near_full_grp_1_mask = 0;
		soc->intr_ctx[i].rx_near_full_grp_2_mask = 0;
		soc->intr_ctx[i].tx_ring_near_full_mask = 0;

		hif_event_history_deinit(soc->hif_handle, i);
		qdf_lro_deinit(soc->intr_ctx[i].lro_ctx);
	}

	qdf_mem_set(&soc->mon_intr_id_lmac_map,
		    sizeof(soc->mon_intr_id_lmac_map),
		    DP_MON_INVALID_LMAC_ID);
}

/*
 * dp_soc_interrupt_attach() - Register handlers for DP interrupts
 * @txrx_soc: DP SOC handle
 *
 * Host driver will register for “DP_NUM_INTERRUPT_CONTEXTS” number of NAPI
 * contexts. Each NAPI context will have a tx_ring_mask , rx_ring_mask ,and
 * rx_monitor_ring mask to indicate the rings that are processed by the handler.
 *
 * Return: 0 for success. nonzero for failure.
 */
static QDF_STATUS dp_soc_interrupt_attach(struct cdp_soc_t *txrx_soc)
{
	struct dp_soc *soc = (struct dp_soc *)txrx_soc;

	int i = 0;
	int num_irq = 0;
	int rx_err_ring_intr_ctxt_id = HIF_MAX_GROUP;
	int lmac_id = 0;

	qdf_mem_set(&soc->mon_intr_id_lmac_map,
		    sizeof(soc->mon_intr_id_lmac_map), DP_MON_INVALID_LMAC_ID);

	for (i = 0; i < wlan_cfg_get_num_contexts(soc->wlan_cfg_ctx); i++) {
		int ret = 0;

		/* Map of IRQ ids registered with one interrupt context */
		int irq_id_map[HIF_MAX_GRP_IRQ];

		int tx_mask =
			wlan_cfg_get_tx_ring_mask(soc->wlan_cfg_ctx, i);
		int rx_mask =
			wlan_cfg_get_rx_ring_mask(soc->wlan_cfg_ctx, i);
		int rx_mon_mask =
			dp_soc_get_mon_mask_for_interrupt_mode(soc, i);
		int tx_mon_ring_mask =
			wlan_cfg_get_tx_mon_ring_mask(soc->wlan_cfg_ctx, i);
		int rx_err_ring_mask =
			wlan_cfg_get_rx_err_ring_mask(soc->wlan_cfg_ctx, i);
		int rx_wbm_rel_ring_mask =
			wlan_cfg_get_rx_wbm_rel_ring_mask(soc->wlan_cfg_ctx, i);
		int reo_status_ring_mask =
			wlan_cfg_get_reo_status_ring_mask(soc->wlan_cfg_ctx, i);
		int rxdma2host_ring_mask =
			wlan_cfg_get_rxdma2host_ring_mask(soc->wlan_cfg_ctx, i);
		int host2rxdma_ring_mask =
			wlan_cfg_get_host2rxdma_ring_mask(soc->wlan_cfg_ctx, i);
		int host2rxdma_mon_ring_mask =
			wlan_cfg_get_host2rxdma_mon_ring_mask(
				soc->wlan_cfg_ctx, i);
		int rx_near_full_grp_1_mask =
			wlan_cfg_get_rx_near_full_grp_1_mask(soc->wlan_cfg_ctx,
							     i);
		int rx_near_full_grp_2_mask =
			wlan_cfg_get_rx_near_full_grp_2_mask(soc->wlan_cfg_ctx,
							     i);
		int tx_ring_near_full_mask =
			wlan_cfg_get_tx_ring_near_full_mask(soc->wlan_cfg_ctx,
							    i);

		soc->intr_ctx[i].dp_intr_id = i;
		soc->intr_ctx[i].tx_ring_mask = tx_mask;
		soc->intr_ctx[i].rx_ring_mask = rx_mask;
		soc->intr_ctx[i].rx_mon_ring_mask = rx_mon_mask;
		soc->intr_ctx[i].tx_mon_ring_mask = tx_mon_ring_mask;
		soc->intr_ctx[i].rx_err_ring_mask = rx_err_ring_mask;
		soc->intr_ctx[i].rxdma2host_ring_mask = rxdma2host_ring_mask;
		soc->intr_ctx[i].host2rxdma_ring_mask = host2rxdma_ring_mask;
		soc->intr_ctx[i].rx_wbm_rel_ring_mask = rx_wbm_rel_ring_mask;
		soc->intr_ctx[i].reo_status_ring_mask = reo_status_ring_mask;
		soc->intr_ctx[i].host2rxdma_mon_ring_mask =
			 host2rxdma_mon_ring_mask;
		soc->intr_ctx[i].rx_near_full_grp_1_mask =
						rx_near_full_grp_1_mask;
		soc->intr_ctx[i].rx_near_full_grp_2_mask =
						rx_near_full_grp_2_mask;
		soc->intr_ctx[i].tx_ring_near_full_mask =
						tx_ring_near_full_mask;

		soc->intr_ctx[i].soc = soc;

		num_irq = 0;

		dp_soc_interrupt_map_calculate(soc, i, &irq_id_map[0],
					       &num_irq);

		if (rx_near_full_grp_1_mask | rx_near_full_grp_2_mask |
		    tx_ring_near_full_mask) {
			dp_soc_near_full_interrupt_attach(soc, num_irq,
							  irq_id_map, i);
		} else {
			ret = hif_register_ext_group(soc->hif_handle,
				num_irq, irq_id_map, dp_service_srngs,
				&soc->intr_ctx[i], "dp_intr",
				HIF_EXEC_NAPI_TYPE,
				QCA_NAPI_DEF_SCALE_BIN_SHIFT);
		}

		dp_debug(" int ctx %u num_irq %u irq_id_map %u %u",
			 i, num_irq, irq_id_map[0], irq_id_map[1]);

		if (ret) {
			dp_init_err("%pK: failed, ret = %d", soc, ret);
			dp_soc_interrupt_detach(txrx_soc);
			return QDF_STATUS_E_FAILURE;
		}

		hif_event_history_init(soc->hif_handle, i);
		soc->intr_ctx[i].lro_ctx = qdf_lro_init();

		if (rx_err_ring_mask)
			rx_err_ring_intr_ctxt_id = i;

		if (dp_is_mon_mask_valid(soc, &soc->intr_ctx[i])) {
			soc->mon_intr_id_lmac_map[lmac_id] = i;
			lmac_id++;
		}
	}

	hif_configure_ext_group_interrupts(soc->hif_handle);
	if (rx_err_ring_intr_ctxt_id != HIF_MAX_GROUP)
		hif_config_irq_clear_cpu_affinity(soc->hif_handle,
						  rx_err_ring_intr_ctxt_id, 0);

	return QDF_STATUS_SUCCESS;
}

#define AVG_MAX_MPDUS_PER_TID 128
#define AVG_TIDS_PER_CLIENT 2
#define AVG_FLOWS_PER_TID 2
#define AVG_MSDUS_PER_FLOW 128
#define AVG_MSDUS_PER_MPDU 4

/*
 * dp_hw_link_desc_pool_banks_free() - Free h/w link desc pool banks
 * @soc: DP SOC handle
 * @mac_id: mac id
 *
 * Return: none
 */
void dp_hw_link_desc_pool_banks_free(struct dp_soc *soc, uint32_t mac_id)
{
	struct qdf_mem_multi_page_t *pages;

	if (mac_id != WLAN_INVALID_PDEV_ID) {
		pages = dp_monitor_get_link_desc_pages(soc, mac_id);
	} else {
		pages = &soc->link_desc_pages;
	}

	if (!pages) {
		dp_err("can not get link desc pages");
		QDF_ASSERT(0);
		return;
	}

	if (pages->dma_pages) {
		wlan_minidump_remove((void *)
				     pages->dma_pages->page_v_addr_start,
				     pages->num_pages * pages->page_size,
				     soc->ctrl_psoc,
				     WLAN_MD_DP_SRNG_WBM_IDLE_LINK,
				     "hw_link_desc_bank");
		dp_desc_multi_pages_mem_free(soc, DP_HW_LINK_DESC_TYPE,
					     pages, 0, false);
	}
}

qdf_export_symbol(dp_hw_link_desc_pool_banks_free);

/*
 * dp_hw_link_desc_pool_banks_alloc() - Allocate h/w link desc pool banks
 * @soc: DP SOC handle
 * @mac_id: mac id
 *
 * Allocates memory pages for link descriptors, the page size is 4K for
 * MCL and 2MB for WIN. if the mac_id is invalid link descriptor pages are
 * allocated for regular RX/TX and if the there is a proper mac_id link
 * descriptors are allocated for RX monitor mode.
 *
 * Return: QDF_STATUS_SUCCESS: Success
 *	   QDF_STATUS_E_FAILURE: Failure
 */
QDF_STATUS dp_hw_link_desc_pool_banks_alloc(struct dp_soc *soc, uint32_t mac_id)
{
	hal_soc_handle_t hal_soc = soc->hal_soc;
	int link_desc_size = hal_get_link_desc_size(soc->hal_soc);
	int link_desc_align = hal_get_link_desc_align(soc->hal_soc);
	uint32_t max_clients = wlan_cfg_get_max_clients(soc->wlan_cfg_ctx);
	uint32_t num_mpdus_per_link_desc = hal_num_mpdus_per_link_desc(hal_soc);
	uint32_t num_msdus_per_link_desc = hal_num_msdus_per_link_desc(hal_soc);
	uint32_t num_mpdu_links_per_queue_desc =
		hal_num_mpdu_links_per_queue_desc(hal_soc);
	uint32_t max_alloc_size = wlan_cfg_max_alloc_size(soc->wlan_cfg_ctx);
	uint32_t *total_link_descs, total_mem_size;
	uint32_t num_mpdu_link_descs, num_mpdu_queue_descs;
	uint32_t num_tx_msdu_link_descs, num_rx_msdu_link_descs;
	uint32_t num_entries;
	struct qdf_mem_multi_page_t *pages;
	struct dp_srng *dp_srng;
	uint8_t minidump_str[MINIDUMP_STR_SIZE];

	/* Only Tx queue descriptors are allocated from common link descriptor
	 * pool Rx queue descriptors are not included in this because (REO queue
	 * extension descriptors) they are expected to be allocated contiguously
	 * with REO queue descriptors
	 */
	if (mac_id != WLAN_INVALID_PDEV_ID) {
		pages = dp_monitor_get_link_desc_pages(soc, mac_id);
		/* dp_monitor_get_link_desc_pages returns NULL only
		 * if monitor SOC is  NULL
		 */
		if (!pages) {
			dp_err("can not get link desc pages");
			QDF_ASSERT(0);
			return QDF_STATUS_E_FAULT;
		}
		dp_srng = &soc->rxdma_mon_desc_ring[mac_id];
		num_entries = dp_srng->alloc_size /
			hal_srng_get_entrysize(soc->hal_soc,
					       RXDMA_MONITOR_DESC);
		total_link_descs = dp_monitor_get_total_link_descs(soc, mac_id);
		qdf_str_lcopy(minidump_str, "mon_link_desc_bank",
			      MINIDUMP_STR_SIZE);
	} else {
		num_mpdu_link_descs = (max_clients * AVG_TIDS_PER_CLIENT *
			AVG_MAX_MPDUS_PER_TID) / num_mpdus_per_link_desc;

		num_mpdu_queue_descs = num_mpdu_link_descs /
			num_mpdu_links_per_queue_desc;

		num_tx_msdu_link_descs = (max_clients * AVG_TIDS_PER_CLIENT *
			AVG_FLOWS_PER_TID * AVG_MSDUS_PER_FLOW) /
			num_msdus_per_link_desc;

		num_rx_msdu_link_descs = (max_clients * AVG_TIDS_PER_CLIENT *
			AVG_MAX_MPDUS_PER_TID * AVG_MSDUS_PER_MPDU) / 6;

		num_entries = num_mpdu_link_descs + num_mpdu_queue_descs +
			num_tx_msdu_link_descs + num_rx_msdu_link_descs;

		pages = &soc->link_desc_pages;
		total_link_descs = &soc->total_link_descs;
		qdf_str_lcopy(minidump_str, "link_desc_bank",
			      MINIDUMP_STR_SIZE);
	}

	/* If link descriptor banks are allocated, return from here */
	if (pages->num_pages)
		return QDF_STATUS_SUCCESS;

	/* Round up to power of 2 */
	*total_link_descs = 1;
	while (*total_link_descs < num_entries)
		*total_link_descs <<= 1;

	dp_init_info("%pK: total_link_descs: %u, link_desc_size: %d",
		     soc, *total_link_descs, link_desc_size);
	total_mem_size =  *total_link_descs * link_desc_size;
	total_mem_size += link_desc_align;

	dp_init_info("%pK: total_mem_size: %d",
		     soc, total_mem_size);

	dp_set_max_page_size(pages, max_alloc_size);
	dp_desc_multi_pages_mem_alloc(soc, DP_HW_LINK_DESC_TYPE,
				      pages,
				      link_desc_size,
				      *total_link_descs,
				      0, false);
	if (!pages->num_pages) {
		dp_err("Multi page alloc fail for hw link desc pool");
		return QDF_STATUS_E_FAULT;
	}

	wlan_minidump_log(pages->dma_pages->page_v_addr_start,
			  pages->num_pages * pages->page_size,
			  soc->ctrl_psoc,
			  WLAN_MD_DP_SRNG_WBM_IDLE_LINK,
			  "hw_link_desc_bank");

	return QDF_STATUS_SUCCESS;
}

/*
 * dp_hw_link_desc_ring_free() - Free h/w link desc rings
 * @soc: DP SOC handle
 *
 * Return: none
 */
static void dp_hw_link_desc_ring_free(struct dp_soc *soc)
{
	uint32_t i;
	uint32_t size = soc->wbm_idle_scatter_buf_size;
	void *vaddr = soc->wbm_idle_link_ring.base_vaddr_unaligned;
	qdf_dma_addr_t paddr;

	if (soc->wbm_idle_scatter_buf_base_vaddr[0]) {
		for (i = 0; i < MAX_IDLE_SCATTER_BUFS; i++) {
			vaddr = soc->wbm_idle_scatter_buf_base_vaddr[i];
			paddr = soc->wbm_idle_scatter_buf_base_paddr[i];
			if (vaddr) {
				qdf_mem_free_consistent(soc->osdev,
							soc->osdev->dev,
							size,
							vaddr,
							paddr,
							0);
				vaddr = NULL;
			}
		}
	} else {
		wlan_minidump_remove(soc->wbm_idle_link_ring.base_vaddr_unaligned,
				     soc->wbm_idle_link_ring.alloc_size,
				     soc->ctrl_psoc,
				     WLAN_MD_DP_SRNG_WBM_IDLE_LINK,
				     "wbm_idle_link_ring");
		dp_srng_free(soc, &soc->wbm_idle_link_ring);
	}
}

/*
 * dp_hw_link_desc_ring_alloc() - Allocate hw link desc rings
 * @soc: DP SOC handle
 *
 * Allocate memory for WBM_IDLE_LINK srng ring if the number of
 * link descriptors is less then the max_allocated size. else
 * allocate memory for wbm_idle_scatter_buffer.
 *
 * Return: QDF_STATUS_SUCCESS: success
 *         QDF_STATUS_E_NO_MEM: No memory (Failure)
 */
static QDF_STATUS dp_hw_link_desc_ring_alloc(struct dp_soc *soc)
{
	uint32_t entry_size, i;
	uint32_t total_mem_size;
	qdf_dma_addr_t *baseaddr = NULL;
	struct dp_srng *dp_srng;
	uint32_t ring_type;
	uint32_t max_alloc_size = wlan_cfg_max_alloc_size(soc->wlan_cfg_ctx);
	uint32_t tlds;

	ring_type = WBM_IDLE_LINK;
	dp_srng = &soc->wbm_idle_link_ring;
	tlds = soc->total_link_descs;

	entry_size = hal_srng_get_entrysize(soc->hal_soc, ring_type);
	total_mem_size = entry_size * tlds;

	if (total_mem_size <= max_alloc_size) {
		if (dp_srng_alloc(soc, dp_srng, ring_type, tlds, 0)) {
			dp_init_err("%pK: Link desc idle ring setup failed",
				    soc);
			goto fail;
		}

		wlan_minidump_log(soc->wbm_idle_link_ring.base_vaddr_unaligned,
				  soc->wbm_idle_link_ring.alloc_size,
				  soc->ctrl_psoc,
				  WLAN_MD_DP_SRNG_WBM_IDLE_LINK,
				  "wbm_idle_link_ring");
	} else {
		uint32_t num_scatter_bufs;
		uint32_t num_entries_per_buf;
		uint32_t buf_size = 0;

		soc->wbm_idle_scatter_buf_size =
			hal_idle_list_scatter_buf_size(soc->hal_soc);
		num_entries_per_buf = hal_idle_scatter_buf_num_entries(
			soc->hal_soc, soc->wbm_idle_scatter_buf_size);
		num_scatter_bufs = hal_idle_list_num_scatter_bufs(
					soc->hal_soc, total_mem_size,
					soc->wbm_idle_scatter_buf_size);

		if (num_scatter_bufs > MAX_IDLE_SCATTER_BUFS) {
			QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
				  FL("scatter bufs size out of bounds"));
			goto fail;
		}

		for (i = 0; i < num_scatter_bufs; i++) {
			baseaddr = &soc->wbm_idle_scatter_buf_base_paddr[i];
			buf_size = soc->wbm_idle_scatter_buf_size;
			soc->wbm_idle_scatter_buf_base_vaddr[i] =
				qdf_mem_alloc_consistent(soc->osdev,
							 soc->osdev->dev,
							 buf_size,
							 baseaddr);

			if (!soc->wbm_idle_scatter_buf_base_vaddr[i]) {
				QDF_TRACE(QDF_MODULE_ID_DP,
					  QDF_TRACE_LEVEL_ERROR,
					  FL("Scatter lst memory alloc fail"));
				goto fail;
			}
		}
		soc->num_scatter_bufs = num_scatter_bufs;
	}
	return QDF_STATUS_SUCCESS;

fail:
	for (i = 0; i < MAX_IDLE_SCATTER_BUFS; i++) {
		void *vaddr = soc->wbm_idle_scatter_buf_base_vaddr[i];
		qdf_dma_addr_t paddr = soc->wbm_idle_scatter_buf_base_paddr[i];

		if (vaddr) {
			qdf_mem_free_consistent(soc->osdev, soc->osdev->dev,
						soc->wbm_idle_scatter_buf_size,
						vaddr,
						paddr, 0);
			vaddr = NULL;
		}
	}
	return QDF_STATUS_E_NOMEM;
}

qdf_export_symbol(dp_hw_link_desc_pool_banks_alloc);

/*
 * dp_hw_link_desc_ring_init() - Initialize hw link desc rings
 * @soc: DP SOC handle
 *
 * Return: QDF_STATUS_SUCCESS: success
 *         QDF_STATUS_E_FAILURE: failure
 */
static QDF_STATUS dp_hw_link_desc_ring_init(struct dp_soc *soc)
{
	struct dp_srng *dp_srng = &soc->wbm_idle_link_ring;

	if (dp_srng->base_vaddr_unaligned) {
		if (dp_srng_init(soc, dp_srng, WBM_IDLE_LINK, 0, 0))
			return QDF_STATUS_E_FAILURE;
	}
	return QDF_STATUS_SUCCESS;
}

/*
 * dp_hw_link_desc_ring_deinit() - Reset hw link desc rings
 * @soc: DP SOC handle
 *
 * Return: None
 */
static void dp_hw_link_desc_ring_deinit(struct dp_soc *soc)
{
	dp_srng_deinit(soc, &soc->wbm_idle_link_ring, WBM_IDLE_LINK, 0);
}

/*
 * dp_hw_link_desc_ring_replenish() - Replenish hw link desc rings
 * @soc: DP SOC handle
 * @mac_id: mac id
 *
 * Return: None
 */
void dp_link_desc_ring_replenish(struct dp_soc *soc, uint32_t mac_id)
{
	uint32_t cookie = 0;
	uint32_t page_idx = 0;
	struct qdf_mem_multi_page_t *pages;
	struct qdf_mem_dma_page_t *dma_pages;
	uint32_t offset = 0;
	uint32_t count = 0;
	void *desc_srng;
	int link_desc_size = hal_get_link_desc_size(soc->hal_soc);
	uint32_t *total_link_descs_addr;
	uint32_t total_link_descs;
	uint32_t scatter_buf_num;
	uint32_t num_entries_per_buf = 0;
	uint32_t rem_entries;
	uint32_t num_descs_per_page;
	uint32_t num_scatter_bufs = 0;
	uint8_t *scatter_buf_ptr;
	void *desc;

	num_scatter_bufs = soc->num_scatter_bufs;

	if (mac_id == WLAN_INVALID_PDEV_ID) {
		pages = &soc->link_desc_pages;
		total_link_descs = soc->total_link_descs;
		desc_srng = soc->wbm_idle_link_ring.hal_srng;
	} else {
		pages = dp_monitor_get_link_desc_pages(soc, mac_id);
		/* dp_monitor_get_link_desc_pages returns NULL only
		 * if monitor SOC is  NULL
		 */
		if (!pages) {
			dp_err("can not get link desc pages");
			QDF_ASSERT(0);
			return;
		}
		total_link_descs_addr =
				dp_monitor_get_total_link_descs(soc, mac_id);
		total_link_descs = *total_link_descs_addr;
		desc_srng = soc->rxdma_mon_desc_ring[mac_id].hal_srng;
	}

	dma_pages = pages->dma_pages;
	do {
		qdf_mem_zero(dma_pages[page_idx].page_v_addr_start,
			     pages->page_size);
		page_idx++;
	} while (page_idx < pages->num_pages);

	if (desc_srng) {
		hal_srng_access_start_unlocked(soc->hal_soc, desc_srng);
		page_idx = 0;
		count = 0;
		offset = 0;
		pages = &soc->link_desc_pages;
		while ((desc = hal_srng_src_get_next(soc->hal_soc,
						     desc_srng)) &&
			(count < total_link_descs)) {
			page_idx = count / pages->num_element_per_page;
			offset = count % pages->num_element_per_page;
			cookie = LINK_DESC_COOKIE(count, page_idx,
						  soc->link_desc_id_start);

			hal_set_link_desc_addr(soc->hal_soc, desc, cookie,
					       dma_pages[page_idx].page_p_addr
					       + (offset * link_desc_size),
					       soc->idle_link_bm_id);
			count++;
		}
		hal_srng_access_end_unlocked(soc->hal_soc, desc_srng);
	} else {
		/* Populate idle list scatter buffers with link descriptor
		 * pointers
		 */
		scatter_buf_num = 0;
		num_entries_per_buf = hal_idle_scatter_buf_num_entries(
					soc->hal_soc,
					soc->wbm_idle_scatter_buf_size);

		scatter_buf_ptr = (uint8_t *)(
			soc->wbm_idle_scatter_buf_base_vaddr[scatter_buf_num]);
		rem_entries = num_entries_per_buf;
		pages = &soc->link_desc_pages;
		page_idx = 0; count = 0;
		offset = 0;
		num_descs_per_page = pages->num_element_per_page;

		while (count < total_link_descs) {
			page_idx = count / num_descs_per_page;
			offset = count % num_descs_per_page;
			cookie = LINK_DESC_COOKIE(count, page_idx,
						  soc->link_desc_id_start);
			hal_set_link_desc_addr(soc->hal_soc,
					       (void *)scatter_buf_ptr,
					       cookie,
					       dma_pages[page_idx].page_p_addr +
					       (offset * link_desc_size),
					       soc->idle_link_bm_id);
			rem_entries--;
			if (rem_entries) {
				scatter_buf_ptr += link_desc_size;
			} else {
				rem_entries = num_entries_per_buf;
				scatter_buf_num++;
				if (scatter_buf_num >= num_scatter_bufs)
					break;
				scatter_buf_ptr = (uint8_t *)
					(soc->wbm_idle_scatter_buf_base_vaddr[
					 scatter_buf_num]);
			}
			count++;
		}
		/* Setup link descriptor idle list in HW */
		hal_setup_link_idle_list(soc->hal_soc,
			soc->wbm_idle_scatter_buf_base_paddr,
			soc->wbm_idle_scatter_buf_base_vaddr,
			num_scatter_bufs, soc->wbm_idle_scatter_buf_size,
			(uint32_t)(scatter_buf_ptr -
			(uint8_t *)(soc->wbm_idle_scatter_buf_base_vaddr[
			scatter_buf_num-1])), total_link_descs);
	}
}

qdf_export_symbol(dp_link_desc_ring_replenish);

#ifdef IPA_OFFLOAD
#define USE_1_IPA_RX_REO_RING 1
#define USE_2_IPA_RX_REO_RINGS 2
#define REO_DST_RING_SIZE_QCA6290 1023
#ifndef CONFIG_WIFI_EMULATION_WIFI_3_0
#define REO_DST_RING_SIZE_QCA8074 1023
#define REO_DST_RING_SIZE_QCN9000 2048
#else
#define REO_DST_RING_SIZE_QCA8074 8
#define REO_DST_RING_SIZE_QCN9000 8
#endif /* CONFIG_WIFI_EMULATION_WIFI_3_0 */

#ifdef IPA_WDI3_TX_TWO_PIPES
#ifdef DP_MEMORY_OPT
static int dp_ipa_init_alt_tx_ring(struct dp_soc *soc)
{
	return dp_init_tx_ring_pair_by_index(soc, IPA_TX_ALT_RING_IDX);
}

static void dp_ipa_deinit_alt_tx_ring(struct dp_soc *soc)
{
	dp_deinit_tx_pair_by_index(soc, IPA_TX_ALT_RING_IDX);
}

static int dp_ipa_alloc_alt_tx_ring(struct dp_soc *soc)
{
	return dp_alloc_tx_ring_pair_by_index(soc, IPA_TX_ALT_RING_IDX);
}

static void dp_ipa_free_alt_tx_ring(struct dp_soc *soc)
{
	dp_free_tx_ring_pair_by_index(soc, IPA_TX_ALT_RING_IDX);
}

#else /* !DP_MEMORY_OPT */
static int dp_ipa_init_alt_tx_ring(struct dp_soc *soc)
{
	return 0;
}

static void dp_ipa_deinit_alt_tx_ring(struct dp_soc *soc)
{
}

static int dp_ipa_alloc_alt_tx_ring(struct dp_soc *soc)
{
	return 0
}

static void dp_ipa_free_alt_tx_ring(struct dp_soc *soc)
{
}
#endif /* DP_MEMORY_OPT */

static void dp_ipa_hal_tx_init_alt_data_ring(struct dp_soc *soc)
{
	hal_tx_init_data_ring(soc->hal_soc,
			      soc->tcl_data_ring[IPA_TX_ALT_RING_IDX].hal_srng);
}

#else /* !IPA_WDI3_TX_TWO_PIPES */
static int dp_ipa_init_alt_tx_ring(struct dp_soc *soc)
{
	return 0;
}

static void dp_ipa_deinit_alt_tx_ring(struct dp_soc *soc)
{
}

static int dp_ipa_alloc_alt_tx_ring(struct dp_soc *soc)
{
	return 0;
}

static void dp_ipa_free_alt_tx_ring(struct dp_soc *soc)
{
}

static void dp_ipa_hal_tx_init_alt_data_ring(struct dp_soc *soc)
{
}

#endif /* IPA_WDI3_TX_TWO_PIPES */

#else

#define REO_DST_RING_SIZE_QCA6290 1024

static int dp_ipa_init_alt_tx_ring(struct dp_soc *soc)
{
	return 0;
}

static void dp_ipa_deinit_alt_tx_ring(struct dp_soc *soc)
{
}

static int dp_ipa_alloc_alt_tx_ring(struct dp_soc *soc)
{
	return 0;
}

static void dp_ipa_free_alt_tx_ring(struct dp_soc *soc)
{
}

static void dp_ipa_hal_tx_init_alt_data_ring(struct dp_soc *soc)
{
}

#endif /* IPA_OFFLOAD */

/*
 * dp_soc_reset_ring_map() - Reset cpu ring map
 * @soc: Datapath soc handler
 *
 * This api resets the default cpu ring map
 */

static void dp_soc_reset_cpu_ring_map(struct dp_soc *soc)
{
	uint8_t i;
	int nss_config = wlan_cfg_get_dp_soc_nss_cfg(soc->wlan_cfg_ctx);

	for (i = 0; i < WLAN_CFG_INT_NUM_CONTEXTS; i++) {
		switch (nss_config) {
		case dp_nss_cfg_first_radio:
			/*
			 * Setting Tx ring map for one nss offloaded radio
			 */
			soc->tx_ring_map[i] = dp_cpu_ring_map[DP_NSS_FIRST_RADIO_OFFLOADED_MAP][i];
			break;

		case dp_nss_cfg_second_radio:
			/*
			 * Setting Tx ring for two nss offloaded radios
			 */
			soc->tx_ring_map[i] = dp_cpu_ring_map[DP_NSS_SECOND_RADIO_OFFLOADED_MAP][i];
			break;

		case dp_nss_cfg_dbdc:
			/*
			 * Setting Tx ring map for 2 nss offloaded radios
			 */
			soc->tx_ring_map[i] =
				dp_cpu_ring_map[DP_NSS_DBDC_OFFLOADED_MAP][i];
			break;

		case dp_nss_cfg_dbtc:
			/*
			 * Setting Tx ring map for 3 nss offloaded radios
			 */
			soc->tx_ring_map[i] =
				dp_cpu_ring_map[DP_NSS_DBTC_OFFLOADED_MAP][i];
			break;

		default:
			dp_err("tx_ring_map failed due to invalid nss cfg");
			break;
		}
	}
}

/*
 * dp_soc_ring_if_nss_offloaded() - find if ring is offloaded to NSS
 * @dp_soc - DP soc handle
 * @ring_type - ring type
 * @ring_num - ring_num
 *
 * return 0 or 1
 */
static uint8_t dp_soc_ring_if_nss_offloaded(struct dp_soc *soc, enum hal_ring_type ring_type, int ring_num)
{
	uint8_t nss_config = wlan_cfg_get_dp_soc_nss_cfg(soc->wlan_cfg_ctx);
	uint8_t status = 0;

	switch (ring_type) {
	case WBM2SW_RELEASE:
	case REO_DST:
	case RXDMA_BUF:
	case REO_EXCEPTION:
		status = ((nss_config) & (1 << ring_num));
		break;
	default:
		break;
	}

	return status;
}

/*
 * dp_soc_disable_unused_mac_intr_mask() - reset interrupt mask for
 *					  unused WMAC hw rings
 * @dp_soc - DP Soc handle
 * @mac_num - wmac num
 *
 * Return: Return void
 */
static void dp_soc_disable_unused_mac_intr_mask(struct dp_soc *soc,
						int mac_num)
{
	uint8_t *grp_mask = NULL;
	int group_number;

	grp_mask = &soc->wlan_cfg_ctx->int_host2rxdma_ring_mask[0];
	group_number = dp_srng_find_ring_in_mask(mac_num, grp_mask);
	wlan_cfg_set_host2rxdma_ring_mask(soc->wlan_cfg_ctx,
					  group_number, 0x0);

	grp_mask = &soc->wlan_cfg_ctx->int_rx_mon_ring_mask[0];
	group_number = dp_srng_find_ring_in_mask(mac_num, grp_mask);
	wlan_cfg_set_rx_mon_ring_mask(soc->wlan_cfg_ctx,
				      group_number, 0x0);

	grp_mask = &soc->wlan_cfg_ctx->int_rxdma2host_ring_mask[0];
	group_number = dp_srng_find_ring_in_mask(mac_num, grp_mask);
	wlan_cfg_set_rxdma2host_ring_mask(soc->wlan_cfg_ctx,
					  group_number, 0x0);

	grp_mask = &soc->wlan_cfg_ctx->int_host2rxdma_mon_ring_mask[0];
	group_number = dp_srng_find_ring_in_mask(mac_num, grp_mask);
	wlan_cfg_set_host2rxdma_mon_ring_mask(soc->wlan_cfg_ctx,
					      group_number, 0x0);
}

/*
 * dp_soc_reset_intr_mask() - reset interrupt mask
 * @dp_soc - DP Soc handle
 *
 * Return: Return void
 */
static void dp_soc_reset_intr_mask(struct dp_soc *soc)
{
	uint8_t j;
	uint8_t *grp_mask = NULL;
	int group_number, mask, num_ring;

	/* number of tx ring */
	num_ring = soc->num_tcl_data_rings;

	/*
	 * group mask for tx completion  ring.
	 */
	grp_mask =  &soc->wlan_cfg_ctx->int_tx_ring_mask[0];

	/* loop and reset the mask for only offloaded ring */
	for (j = 0; j < WLAN_CFG_NUM_TCL_DATA_RINGS; j++) {
		/*
		 * Group number corresponding to tx offloaded ring.
		 */
		group_number = dp_srng_find_ring_in_mask(j, grp_mask);
		if (group_number < 0) {
			dp_init_debug("%pK: ring not part of any group; ring_type: %d,ring_num %d",
				      soc, WBM2SW_RELEASE, j);
			continue;
		}

		mask = wlan_cfg_get_tx_ring_mask(soc->wlan_cfg_ctx, group_number);
		if (!dp_soc_ring_if_nss_offloaded(soc, WBM2SW_RELEASE, j) &&
		    (!mask)) {
			continue;
		}

		/* reset the tx mask for offloaded ring */
		mask &= (~(1 << j));

		/*
		 * reset the interrupt mask for offloaded ring.
		 */
		wlan_cfg_set_tx_ring_mask(soc->wlan_cfg_ctx, group_number, mask);
	}

	/* number of rx rings */
	num_ring = soc->num_reo_dest_rings;

	/*
	 * group mask for reo destination ring.
	 */
	grp_mask = &soc->wlan_cfg_ctx->int_rx_ring_mask[0];

	/* loop and reset the mask for only offloaded ring */
	for (j = 0; j < WLAN_CFG_NUM_REO_DEST_RING; j++) {
		/*
		 * Group number corresponding to rx offloaded ring.
		 */
		group_number = dp_srng_find_ring_in_mask(j, grp_mask);
		if (group_number < 0) {
			dp_init_debug("%pK: ring not part of any group; ring_type: %d,ring_num %d",
				      soc, REO_DST, j);
			continue;
		}

		mask =  wlan_cfg_get_rx_ring_mask(soc->wlan_cfg_ctx, group_number);
		if (!dp_soc_ring_if_nss_offloaded(soc, REO_DST, j) &&
		    (!mask)) {
			continue;
		}

		/* reset the interrupt mask for offloaded ring */
		mask &= (~(1 << j));

		/*
		 * set the interrupt mask to zero for rx offloaded radio.
		 */
		wlan_cfg_set_rx_ring_mask(soc->wlan_cfg_ctx, group_number, mask);
	}

	/*
	 * group mask for Rx buffer refill ring
	 */
	grp_mask = &soc->wlan_cfg_ctx->int_host2rxdma_ring_mask[0];

	/* loop and reset the mask for only offloaded ring */
	for (j = 0; j < MAX_PDEV_CNT; j++) {
		int lmac_id = wlan_cfg_get_hw_mac_idx(soc->wlan_cfg_ctx, j);

		if (!dp_soc_ring_if_nss_offloaded(soc, RXDMA_BUF, j)) {
			continue;
		}

		/*
		 * Group number corresponding to rx offloaded ring.
		 */
		group_number = dp_srng_find_ring_in_mask(lmac_id, grp_mask);
		if (group_number < 0) {
			dp_init_debug("%pK: ring not part of any group; ring_type: %d,ring_num %d",
				      soc, REO_DST, lmac_id);
			continue;
		}

		/* set the interrupt mask for offloaded ring */
		mask =  wlan_cfg_get_host2rxdma_ring_mask(soc->wlan_cfg_ctx,
				group_number);
		mask &= (~(1 << lmac_id));

		/*
		 * set the interrupt mask to zero for rx offloaded radio.
		 */
		wlan_cfg_set_host2rxdma_ring_mask(soc->wlan_cfg_ctx,
			group_number, mask);
	}

	grp_mask = &soc->wlan_cfg_ctx->int_rx_err_ring_mask[0];

	for (j = 0; j < num_ring; j++) {
		if (!dp_soc_ring_if_nss_offloaded(soc, REO_EXCEPTION, j)) {
			continue;
		}

		/*
		 * Group number corresponding to rx err ring.
		 */
		group_number = dp_srng_find_ring_in_mask(j, grp_mask);
		if (group_number < 0) {
			dp_init_debug("%pK: ring not part of any group; ring_type: %d,ring_num %d",
				      soc, REO_EXCEPTION, j);
			continue;
		}

		wlan_cfg_set_rx_err_ring_mask(soc->wlan_cfg_ctx,
					      group_number, 0);
	}
}

#ifdef IPA_OFFLOAD
bool dp_reo_remap_config(struct dp_soc *soc, uint32_t *remap0,
			 uint32_t *remap1, uint32_t *remap2)
{
	uint32_t ring[WLAN_CFG_NUM_REO_DEST_RING_MAX] = {
				REO_REMAP_SW1, REO_REMAP_SW2, REO_REMAP_SW3,
				REO_REMAP_SW5, REO_REMAP_SW6, REO_REMAP_SW7};

	switch (soc->arch_id) {
	case CDP_ARCH_TYPE_BE:
		hal_compute_reo_remap_ix2_ix3(soc->hal_soc, ring,
					      soc->num_reo_dest_rings -
					      USE_2_IPA_RX_REO_RINGS, remap1,
					      remap2);
		break;

	case CDP_ARCH_TYPE_LI:
		hal_compute_reo_remap_ix2_ix3(soc->hal_soc, ring,
					      soc->num_reo_dest_rings -
					      USE_1_IPA_RX_REO_RING, remap1,
					      remap2);
		hal_compute_reo_remap_ix0(soc->hal_soc, remap0);
		break;
	default:
		dp_err("unkonwn arch_id 0x%x", soc->arch_id);
		QDF_BUG(0);

	}

	dp_debug("remap1 %x remap2 %x", *remap1, *remap2);

	return true;
}

#ifdef IPA_WDI3_TX_TWO_PIPES
static bool dp_ipa_is_alt_tx_ring(int index)
{
	return index == IPA_TX_ALT_RING_IDX;
}

static bool dp_ipa_is_alt_tx_comp_ring(int index)
{
	return index == IPA_TX_ALT_COMP_RING_IDX;
}
#else /* !IPA_WDI3_TX_TWO_PIPES */
static bool dp_ipa_is_alt_tx_ring(int index)
{
	return false;
}

static bool dp_ipa_is_alt_tx_comp_ring(int index)
{
	return false;
}
#endif /* IPA_WDI3_TX_TWO_PIPES */

/**
 * dp_ipa_get_tx_ring_size() - Get Tx ring size for IPA
 *
 * @tx_ring_num: Tx ring number
 * @tx_ipa_ring_sz: Return param only updated for IPA.
 * @soc_cfg_ctx: dp soc cfg context
 *
 * Return: None
 */
static void dp_ipa_get_tx_ring_size(int tx_ring_num, int *tx_ipa_ring_sz,
				    struct wlan_cfg_dp_soc_ctxt *soc_cfg_ctx)
{
	if (!soc_cfg_ctx->ipa_enabled)
		return;

	if (tx_ring_num == IPA_TCL_DATA_RING_IDX)
		*tx_ipa_ring_sz = wlan_cfg_ipa_tx_ring_size(soc_cfg_ctx);
	else if (dp_ipa_is_alt_tx_ring(tx_ring_num))
		*tx_ipa_ring_sz = wlan_cfg_ipa_tx_alt_ring_size(soc_cfg_ctx);
}

/**
 * dp_ipa_get_tx_comp_ring_size() - Get Tx comp ring size for IPA
 *
 * @tx_comp_ring_num: Tx comp ring number
 * @tx_comp_ipa_ring_sz: Return param only updated for IPA.
 * @soc_cfg_ctx: dp soc cfg context
 *
 * Return: None
 */
static void dp_ipa_get_tx_comp_ring_size(int tx_comp_ring_num,
					 int *tx_comp_ipa_ring_sz,
				       struct wlan_cfg_dp_soc_ctxt *soc_cfg_ctx)
{
	if (!soc_cfg_ctx->ipa_enabled)
		return;

	if (tx_comp_ring_num == IPA_TCL_DATA_RING_IDX)
		*tx_comp_ipa_ring_sz =
				wlan_cfg_ipa_tx_comp_ring_size(soc_cfg_ctx);
	else if (dp_ipa_is_alt_tx_comp_ring(tx_comp_ring_num))
		*tx_comp_ipa_ring_sz =
				wlan_cfg_ipa_tx_alt_comp_ring_size(soc_cfg_ctx);
}
#else
static uint8_t dp_reo_ring_selection(uint32_t value, uint32_t *ring)
{
	uint8_t num = 0;

	switch (value) {
	/* should we have all the different possible ring configs */
	case 0xFF:
		num = 8;
		ring[0] = REO_REMAP_SW1;
		ring[1] = REO_REMAP_SW2;
		ring[2] = REO_REMAP_SW3;
		ring[3] = REO_REMAP_SW4;
		ring[4] = REO_REMAP_SW5;
		ring[5] = REO_REMAP_SW6;
		ring[6] = REO_REMAP_SW7;
		ring[7] = REO_REMAP_SW8;
		break;

	case 0x3F:
		num = 6;
		ring[0] = REO_REMAP_SW1;
		ring[1] = REO_REMAP_SW2;
		ring[2] = REO_REMAP_SW3;
		ring[3] = REO_REMAP_SW4;
		ring[4] = REO_REMAP_SW5;
		ring[5] = REO_REMAP_SW6;
		break;

	case 0xF:
		num = 4;
		ring[0] = REO_REMAP_SW1;
		ring[1] = REO_REMAP_SW2;
		ring[2] = REO_REMAP_SW3;
		ring[3] = REO_REMAP_SW4;
		break;
	case 0xE:
		num = 3;
		ring[0] = REO_REMAP_SW2;
		ring[1] = REO_REMAP_SW3;
		ring[2] = REO_REMAP_SW4;
		break;
	case 0xD:
		num = 3;
		ring[0] = REO_REMAP_SW1;
		ring[1] = REO_REMAP_SW3;
		ring[2] = REO_REMAP_SW4;
		break;
	case 0xC:
		num = 2;
		ring[0] = REO_REMAP_SW3;
		ring[1] = REO_REMAP_SW4;
		break;
	case 0xB:
		num = 3;
		ring[0] = REO_REMAP_SW1;
		ring[1] = REO_REMAP_SW2;
		ring[2] = REO_REMAP_SW4;
		break;
	case 0xA:
		num = 2;
		ring[0] = REO_REMAP_SW2;
		ring[1] = REO_REMAP_SW4;
		break;
	case 0x9:
		num = 2;
		ring[0] = REO_REMAP_SW1;
		ring[1] = REO_REMAP_SW4;
		break;
	case 0x8:
		num = 1;
		ring[0] = REO_REMAP_SW4;
		break;
	case 0x7:
		num = 3;
		ring[0] = REO_REMAP_SW1;
		ring[1] = REO_REMAP_SW2;
		ring[2] = REO_REMAP_SW3;
		break;
	case 0x6:
		num = 2;
		ring[0] = REO_REMAP_SW2;
		ring[1] = REO_REMAP_SW3;
		break;
	case 0x5:
		num = 2;
		ring[0] = REO_REMAP_SW1;
		ring[1] = REO_REMAP_SW3;
		break;
	case 0x4:
		num = 1;
		ring[0] = REO_REMAP_SW3;
		break;
	case 0x3:
		num = 2;
		ring[0] = REO_REMAP_SW1;
		ring[1] = REO_REMAP_SW2;
		break;
	case 0x2:
		num = 1;
		ring[0] = REO_REMAP_SW2;
		break;
	case 0x1:
		num = 1;
		ring[0] = REO_REMAP_SW1;
		break;
	default:
		dp_err("unkonwn reo ring map 0x%x", value);
		QDF_BUG(0);
	}
	return num;
}

bool dp_reo_remap_config(struct dp_soc *soc,
			 uint32_t *remap0,
			 uint32_t *remap1,
			 uint32_t *remap2)
{
	uint8_t offload_radio = wlan_cfg_get_dp_soc_nss_cfg(soc->wlan_cfg_ctx);
	uint32_t reo_config = wlan_cfg_get_reo_rings_mapping(soc->wlan_cfg_ctx);
	uint8_t target_type, num;
	uint32_t ring[WLAN_CFG_NUM_REO_DEST_RING_MAX];
	uint32_t value;

	target_type = hal_get_target_type(soc->hal_soc);

	switch (offload_radio) {
	case dp_nss_cfg_default:
		value = reo_config & WLAN_CFG_NUM_REO_RINGS_MAP_MAX;
		num = dp_reo_ring_selection(value, ring);
		hal_compute_reo_remap_ix2_ix3(soc->hal_soc, ring,
					      num, remap1, remap2);
		hal_compute_reo_remap_ix0(soc->hal_soc, remap0);

		break;
	case dp_nss_cfg_first_radio:
		value = reo_config & 0xE;
		num = dp_reo_ring_selection(value, ring);
		hal_compute_reo_remap_ix2_ix3(soc->hal_soc, ring,
					      num, remap1, remap2);

		break;
	case dp_nss_cfg_second_radio:
		value = reo_config & 0xD;
		num = dp_reo_ring_selection(value, ring);
		hal_compute_reo_remap_ix2_ix3(soc->hal_soc, ring,
					      num, remap1, remap2);

		break;
	case dp_nss_cfg_dbdc:
	case dp_nss_cfg_dbtc:
		/* return false if both or all are offloaded to NSS */
		return false;

	}

	dp_debug("remap1 %x remap2 %x offload_radio %u",
		 *remap1, *remap2, offload_radio);
	return true;
}

static void dp_ipa_get_tx_ring_size(int ring_num, int *tx_ipa_ring_sz,
				    struct wlan_cfg_dp_soc_ctxt *soc_cfg_ctx)
{
}

static void dp_ipa_get_tx_comp_ring_size(int tx_comp_ring_num,
					 int *tx_comp_ipa_ring_sz,
				       struct wlan_cfg_dp_soc_ctxt *soc_cfg_ctx)
{
}
#endif /* IPA_OFFLOAD */

/*
 * dp_reo_frag_dst_set() - configure reo register to set the
 *                        fragment destination ring
 * @soc : Datapath soc
 * @frag_dst_ring : output parameter to set fragment destination ring
 *
 * Based on offload_radio below fragment destination rings is selected
 * 0 - TCL
 * 1 - SW1
 * 2 - SW2
 * 3 - SW3
 * 4 - SW4
 * 5 - Release
 * 6 - FW
 * 7 - alternate select
 *
 * return: void
 */
static void dp_reo_frag_dst_set(struct dp_soc *soc, uint8_t *frag_dst_ring)
{
	uint8_t offload_radio = wlan_cfg_get_dp_soc_nss_cfg(soc->wlan_cfg_ctx);

	switch (offload_radio) {
	case dp_nss_cfg_default:
		*frag_dst_ring = REO_REMAP_TCL;
		break;
	case dp_nss_cfg_first_radio:
		/*
		 * This configuration is valid for single band radio which
		 * is also NSS offload.
		 */
	case dp_nss_cfg_dbdc:
	case dp_nss_cfg_dbtc:
		*frag_dst_ring = HAL_SRNG_REO_ALTERNATE_SELECT;
		break;
	default:
		dp_init_err("%pK: dp_reo_frag_dst_set invalid offload radio config", soc);
		break;
	}
}

#ifdef ENABLE_VERBOSE_DEBUG
static void dp_enable_verbose_debug(struct dp_soc *soc)
{
	struct wlan_cfg_dp_soc_ctxt *soc_cfg_ctx;

	soc_cfg_ctx = soc->wlan_cfg_ctx;

	if (soc_cfg_ctx->per_pkt_trace & dp_verbose_debug_mask)
		is_dp_verbose_debug_enabled = true;

	if (soc_cfg_ctx->per_pkt_trace & hal_verbose_debug_mask)
		hal_set_verbose_debug(true);
	else
		hal_set_verbose_debug(false);
}
#else
static void dp_enable_verbose_debug(struct dp_soc *soc)
{
}
#endif

#ifdef WLAN_FEATURE_STATS_EXT
static inline void dp_create_ext_stats_event(struct dp_soc *soc)
{
	qdf_event_create(&soc->rx_hw_stats_event);
}
#else
static inline void dp_create_ext_stats_event(struct dp_soc *soc)
{
}
#endif

static void dp_deinit_tx_pair_by_index(struct dp_soc *soc, int index)
{
	int tcl_ring_num, wbm_ring_num;

	wlan_cfg_get_tcl_wbm_ring_num_for_index(soc->wlan_cfg_ctx,
						index,
						&tcl_ring_num,
						&wbm_ring_num);

	if (tcl_ring_num == -1) {
		dp_err("incorrect tcl ring num for index %u", index);
		return;
	}

	wlan_minidump_remove(soc->tcl_data_ring[index].base_vaddr_unaligned,
			     soc->tcl_data_ring[index].alloc_size,
			     soc->ctrl_psoc,
			     WLAN_MD_DP_SRNG_TCL_DATA,
			     "tcl_data_ring");
	dp_info("index %u tcl %u wbm %u", index, tcl_ring_num, wbm_ring_num);
	dp_srng_deinit(soc, &soc->tcl_data_ring[index], TCL_DATA,
		       tcl_ring_num);

	if (wbm_ring_num == INVALID_WBM_RING_NUM)
		return;

	wlan_minidump_remove(soc->tx_comp_ring[index].base_vaddr_unaligned,
			     soc->tx_comp_ring[index].alloc_size,
			     soc->ctrl_psoc,
			     WLAN_MD_DP_SRNG_TX_COMP,
			     "tcl_comp_ring");
	dp_srng_deinit(soc, &soc->tx_comp_ring[index], WBM2SW_RELEASE,
		       wbm_ring_num);
}

/**
 * dp_init_tx_ring_pair_by_index() - The function inits tcl data/wbm completion
 * ring pair
 * @soc: DP soc pointer
 * @index: index of soc->tcl_data or soc->tx_comp to initialize
 *
 * Return: QDF_STATUS_SUCCESS on success, error code otherwise.
 */
static QDF_STATUS dp_init_tx_ring_pair_by_index(struct dp_soc *soc,
						uint8_t index)
{
	int tcl_ring_num, wbm_ring_num;
	uint8_t bm_id;

	if (index >= MAX_TCL_DATA_RINGS) {
		dp_err("unexpected index!");
		QDF_BUG(0);
		goto fail1;
	}

	wlan_cfg_get_tcl_wbm_ring_num_for_index(soc->wlan_cfg_ctx,
						index,
						&tcl_ring_num,
						&wbm_ring_num);

	if (tcl_ring_num == -1) {
		dp_err("incorrect tcl ring num for index %u", index);
		goto fail1;
	}

	dp_info("index %u tcl %u wbm %u", index, tcl_ring_num, wbm_ring_num);
	if (dp_srng_init(soc, &soc->tcl_data_ring[index], TCL_DATA,
			 tcl_ring_num, 0)) {
		dp_err("dp_srng_init failed for tcl_data_ring");
		goto fail1;
	}
	wlan_minidump_log(soc->tcl_data_ring[index].base_vaddr_unaligned,
			  soc->tcl_data_ring[index].alloc_size,
			  soc->ctrl_psoc,
			  WLAN_MD_DP_SRNG_TCL_DATA,
			  "tcl_data_ring");

	if (wbm_ring_num == INVALID_WBM_RING_NUM)
		goto set_rbm;

	if (dp_srng_init(soc, &soc->tx_comp_ring[index], WBM2SW_RELEASE,
			 wbm_ring_num, 0)) {
		dp_err("dp_srng_init failed for tx_comp_ring");
		goto fail1;
	}

	wlan_minidump_log(soc->tx_comp_ring[index].base_vaddr_unaligned,
			  soc->tx_comp_ring[index].alloc_size,
			  soc->ctrl_psoc,
			  WLAN_MD_DP_SRNG_TX_COMP,
			  "tcl_comp_ring");
set_rbm:
	bm_id = wlan_cfg_get_rbm_id_for_index(soc->wlan_cfg_ctx, tcl_ring_num);

	soc->arch_ops.tx_implicit_rbm_set(soc, tcl_ring_num, bm_id);

	return QDF_STATUS_SUCCESS;

fail1:
	return QDF_STATUS_E_FAILURE;
}

static void dp_free_tx_ring_pair_by_index(struct dp_soc *soc, uint8_t index)
{
	dp_debug("index %u", index);
	dp_srng_free(soc, &soc->tcl_data_ring[index]);
	dp_srng_free(soc, &soc->tx_comp_ring[index]);
}

/**
 * dp_alloc_tx_ring_pair_by_index() - The function allocs tcl data/wbm2sw
 * ring pair for the given "index"
 * @soc: DP soc pointer
 * @index: index of soc->tcl_data or soc->tx_comp to initialize
 *
 * Return: QDF_STATUS_SUCCESS on success, error code otherwise.
 */
static QDF_STATUS dp_alloc_tx_ring_pair_by_index(struct dp_soc *soc,
						 uint8_t index)
{
	int tx_ring_size;
	int tx_comp_ring_size;
	struct wlan_cfg_dp_soc_ctxt *soc_cfg_ctx = soc->wlan_cfg_ctx;
	int cached = 0;

	if (index >= MAX_TCL_DATA_RINGS) {
		dp_err("unexpected index!");
		QDF_BUG(0);
		goto fail1;
	}

	dp_debug("index %u", index);
	tx_ring_size = wlan_cfg_tx_ring_size(soc_cfg_ctx);
	dp_ipa_get_tx_ring_size(index, &tx_ring_size, soc_cfg_ctx);

	if (dp_srng_alloc(soc, &soc->tcl_data_ring[index], TCL_DATA,
			  tx_ring_size, cached)) {
		dp_err("dp_srng_alloc failed for tcl_data_ring");
		goto fail1;
	}

	tx_comp_ring_size = wlan_cfg_tx_comp_ring_size(soc_cfg_ctx);
	dp_ipa_get_tx_comp_ring_size(index, &tx_comp_ring_size, soc_cfg_ctx);
	/* Enable cached TCL desc if NSS offload is disabled */
	if (!wlan_cfg_get_dp_soc_nss_cfg(soc_cfg_ctx))
		cached = WLAN_CFG_DST_RING_CACHED_DESC;

	if (wlan_cfg_get_wbm_ring_num_for_index(soc->wlan_cfg_ctx, index) ==
	    INVALID_WBM_RING_NUM)
		return QDF_STATUS_SUCCESS;

	if (dp_srng_alloc(soc, &soc->tx_comp_ring[index], WBM2SW_RELEASE,
			  tx_comp_ring_size, cached)) {
		dp_err("dp_srng_alloc failed for tx_comp_ring");
		goto fail1;
	}

	return QDF_STATUS_SUCCESS;

fail1:
	return QDF_STATUS_E_FAILURE;
}

static QDF_STATUS dp_lro_hash_setup(struct dp_soc *soc, struct dp_pdev *pdev)
{
	struct cdp_lro_hash_config lro_hash;
	QDF_STATUS status;

	if (!wlan_cfg_is_lro_enabled(soc->wlan_cfg_ctx) &&
	    !wlan_cfg_is_gro_enabled(soc->wlan_cfg_ctx) &&
	    !wlan_cfg_is_rx_hash_enabled(soc->wlan_cfg_ctx)) {
		dp_err("LRO, GRO and RX hash disabled");
		return QDF_STATUS_E_FAILURE;
	}

	qdf_mem_zero(&lro_hash, sizeof(lro_hash));

	if (wlan_cfg_is_lro_enabled(soc->wlan_cfg_ctx) ||
	    wlan_cfg_is_gro_enabled(soc->wlan_cfg_ctx)) {
		lro_hash.lro_enable = 1;
		lro_hash.tcp_flag = QDF_TCPHDR_ACK;
		lro_hash.tcp_flag_mask = QDF_TCPHDR_FIN | QDF_TCPHDR_SYN |
			 QDF_TCPHDR_RST | QDF_TCPHDR_ACK | QDF_TCPHDR_URG |
			 QDF_TCPHDR_ECE | QDF_TCPHDR_CWR;
	}

	qdf_get_random_bytes(lro_hash.toeplitz_hash_ipv4,
			     (sizeof(lro_hash.toeplitz_hash_ipv4[0]) *
			      LRO_IPV4_SEED_ARR_SZ));
	qdf_get_random_bytes(lro_hash.toeplitz_hash_ipv6,
			     (sizeof(lro_hash.toeplitz_hash_ipv6[0]) *
			      LRO_IPV6_SEED_ARR_SZ));

	qdf_assert(soc->cdp_soc.ol_ops->lro_hash_config);

	if (!soc->cdp_soc.ol_ops->lro_hash_config) {
		QDF_BUG(0);
		dp_err("lro_hash_config not configured");
		return QDF_STATUS_E_FAILURE;
	}

	status = soc->cdp_soc.ol_ops->lro_hash_config(soc->ctrl_psoc,
						      pdev->pdev_id,
						      &lro_hash);
	if (!QDF_IS_STATUS_SUCCESS(status)) {
		dp_err("failed to send lro_hash_config to FW %u", status);
		return status;
	}

	dp_info("LRO CMD config: lro_enable: 0x%x tcp_flag 0x%x tcp_flag_mask 0x%x",
		lro_hash.lro_enable, lro_hash.tcp_flag,
		lro_hash.tcp_flag_mask);

	dp_info("toeplitz_hash_ipv4:");
	qdf_trace_hex_dump(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_INFO,
			   lro_hash.toeplitz_hash_ipv4,
			   (sizeof(lro_hash.toeplitz_hash_ipv4[0]) *
			   LRO_IPV4_SEED_ARR_SZ));

	dp_info("toeplitz_hash_ipv6:");
	qdf_trace_hex_dump(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_INFO,
			   lro_hash.toeplitz_hash_ipv6,
			   (sizeof(lro_hash.toeplitz_hash_ipv6[0]) *
			   LRO_IPV6_SEED_ARR_SZ));

	return status;
}

#if defined(WLAN_MAX_PDEVS) && (WLAN_MAX_PDEVS == 1)
/*
 * dp_reap_timer_init() - initialize the reap timer
 * @soc: data path SoC handle
 *
 * Return: void
 */
static void dp_reap_timer_init(struct dp_soc *soc)
{
	/*
	 * Timer to reap rxdma status rings.
	 * Needed until we enable ppdu end interrupts
	 */
	dp_monitor_reap_timer_init(soc);
	dp_monitor_vdev_timer_init(soc);
}

/*
 * dp_reap_timer_deinit() - de-initialize the reap timer
 * @soc: data path SoC handle
 *
 * Return: void
 */
static void dp_reap_timer_deinit(struct dp_soc *soc)
{
	dp_monitor_reap_timer_deinit(soc);
}
#else
/* WIN use case */
static void dp_reap_timer_init(struct dp_soc *soc)
{
	/* Configure LMAC rings in Polled mode */
	if (soc->lmac_polled_mode) {
		/*
		 * Timer to reap lmac rings.
		 */
		qdf_timer_init(soc->osdev, &soc->lmac_reap_timer,
			       dp_service_lmac_rings, (void *)soc,
			       QDF_TIMER_TYPE_WAKE_APPS);
		soc->lmac_timer_init = 1;
		qdf_timer_mod(&soc->lmac_reap_timer, DP_INTR_POLL_TIMER_MS);
	}
}

static void dp_reap_timer_deinit(struct dp_soc *soc)
{
	if (soc->lmac_timer_init) {
		qdf_timer_stop(&soc->lmac_reap_timer);
		qdf_timer_free(&soc->lmac_reap_timer);
		soc->lmac_timer_init = 0;
	}
}
#endif

#ifdef QCA_HOST2FW_RXBUF_RING
/*
 * dp_rxdma_ring_alloc() - allocate the RXDMA rings
 * @soc: data path SoC handle
 * @pdev: Physical device handle
 *
 * Return: 0 - success, > 0 - failure
 */
static int dp_rxdma_ring_alloc(struct dp_soc *soc, struct dp_pdev *pdev)
{
	struct wlan_cfg_dp_pdev_ctxt *pdev_cfg_ctx;
	int max_mac_rings;
	int i;
	int ring_size;

	pdev_cfg_ctx = pdev->wlan_cfg_ctx;
	max_mac_rings = wlan_cfg_get_num_mac_rings(pdev_cfg_ctx);
	ring_size =  wlan_cfg_get_rx_dma_buf_ring_size(pdev_cfg_ctx);

	for (i = 0; i < max_mac_rings; i++) {
		dp_verbose_debug("pdev_id %d mac_id %d", pdev->pdev_id, i);
		if (dp_srng_alloc(soc, &pdev->rx_mac_buf_ring[i],
				  RXDMA_BUF, ring_size, 0)) {
			dp_init_err("%pK: failed rx mac ring setup", soc);
			return QDF_STATUS_E_FAILURE;
		}
	}
	return QDF_STATUS_SUCCESS;
}

/*
 * dp_rxdma_ring_setup() - configure the RXDMA rings
 * @soc: data path SoC handle
 * @pdev: Physical device handle
 *
 * Return: 0 - success, > 0 - failure
 */
static int dp_rxdma_ring_setup(struct dp_soc *soc, struct dp_pdev *pdev)
{
	struct wlan_cfg_dp_pdev_ctxt *pdev_cfg_ctx;
	int max_mac_rings;
	int i;

	pdev_cfg_ctx = pdev->wlan_cfg_ctx;
	max_mac_rings = wlan_cfg_get_num_mac_rings(pdev_cfg_ctx);

	for (i = 0; i < max_mac_rings; i++) {
		dp_verbose_debug("pdev_id %d mac_id %d", pdev->pdev_id, i);
		if (dp_srng_init(soc, &pdev->rx_mac_buf_ring[i],
				 RXDMA_BUF, 1, i)) {
			dp_init_err("%pK: failed rx mac ring setup", soc);
			return QDF_STATUS_E_FAILURE;
		}
	}
	return QDF_STATUS_SUCCESS;
}

/*
 * dp_rxdma_ring_cleanup() - Deinit the RXDMA rings and reap timer
 * @soc: data path SoC handle
 * @pdev: Physical device handle
 *
 * Return: void
 */
static void dp_rxdma_ring_cleanup(struct dp_soc *soc, struct dp_pdev *pdev)
{
	int i;

	for (i = 0; i < MAX_RX_MAC_RINGS; i++)
		dp_srng_deinit(soc, &pdev->rx_mac_buf_ring[i], RXDMA_BUF, 1);

	dp_reap_timer_deinit(soc);
}

/*
 * dp_rxdma_ring_free() - Free the RXDMA rings
 * @pdev: Physical device handle
 *
 * Return: void
 */
static void dp_rxdma_ring_free(struct dp_pdev *pdev)
{
	int i;

	for (i = 0; i < MAX_RX_MAC_RINGS; i++)
		dp_srng_free(pdev->soc, &pdev->rx_mac_buf_ring[i]);
}

#else
static int dp_rxdma_ring_alloc(struct dp_soc *soc, struct dp_pdev *pdev)
{
	return QDF_STATUS_SUCCESS;
}

static int dp_rxdma_ring_setup(struct dp_soc *soc, struct dp_pdev *pdev)
{
	return QDF_STATUS_SUCCESS;
}

static void dp_rxdma_ring_cleanup(struct dp_soc *soc, struct dp_pdev *pdev)
{
	dp_reap_timer_deinit(soc);
}

static void dp_rxdma_ring_free(struct dp_pdev *pdev)
{
}
#endif

/**
 * dp_dscp_tid_map_setup(): Initialize the dscp-tid maps
 * @pdev - DP_PDEV handle
 *
 * Return: void
 */
static inline void
dp_dscp_tid_map_setup(struct dp_pdev *pdev)
{
	uint8_t map_id;
	struct dp_soc *soc = pdev->soc;

	if (!soc)
		return;

	for (map_id = 0; map_id < DP_MAX_TID_MAPS; map_id++) {
		qdf_mem_copy(pdev->dscp_tid_map[map_id],
			     default_dscp_tid_map,
			     sizeof(default_dscp_tid_map));
	}

	for (map_id = 0; map_id < soc->num_hw_dscp_tid_map; map_id++) {
		hal_tx_set_dscp_tid_map(soc->hal_soc,
					default_dscp_tid_map,
					map_id);
	}
}

/**
 * dp_pcp_tid_map_setup(): Initialize the pcp-tid maps
 * @pdev - DP_PDEV handle
 *
 * Return: void
 */
static inline void
dp_pcp_tid_map_setup(struct dp_pdev *pdev)
{
	struct dp_soc *soc = pdev->soc;

	if (!soc)
		return;

	qdf_mem_copy(soc->pcp_tid_map, default_pcp_tid_map,
		     sizeof(default_pcp_tid_map));
	hal_tx_set_pcp_tid_map_default(soc->hal_soc, default_pcp_tid_map);
}

#ifdef IPA_OFFLOAD
/**
 * dp_setup_ipa_rx_refill_buf_ring - Setup second Rx refill buffer ring
 * @soc: data path instance
 * @pdev: core txrx pdev context
 *
 * Return: QDF_STATUS_SUCCESS: success
 *         QDF_STATUS_E_RESOURCES: Error return
 */
static int dp_setup_ipa_rx_refill_buf_ring(struct dp_soc *soc,
					   struct dp_pdev *pdev)
{
	struct wlan_cfg_dp_soc_ctxt *soc_cfg_ctx;
	int entries;

	if (wlan_cfg_is_ipa_enabled(soc->wlan_cfg_ctx)) {
		soc_cfg_ctx = soc->wlan_cfg_ctx;
		entries =
			wlan_cfg_get_dp_soc_rxdma_refill_ring_size(soc_cfg_ctx);

		/* Setup second Rx refill buffer ring */
		if (dp_srng_alloc(soc, &pdev->rx_refill_buf_ring2, RXDMA_BUF,
				  entries, 0)) {
			dp_init_err("%pK: dp_srng_alloc failed second"
				    "rx refill ring", soc);
			return QDF_STATUS_E_FAILURE;
		}
	}

	return QDF_STATUS_SUCCESS;
}

/**
 * dp_init_ipa_rx_refill_buf_ring - Init second Rx refill buffer ring
 * @soc: data path instance
 * @pdev: core txrx pdev context
 *
 * Return: QDF_STATUS_SUCCESS: success
 *         QDF_STATUS_E_RESOURCES: Error return
 */
static int dp_init_ipa_rx_refill_buf_ring(struct dp_soc *soc,
					  struct dp_pdev *pdev)
{
	if (wlan_cfg_is_ipa_enabled(soc->wlan_cfg_ctx)) {
		if (dp_srng_init(soc, &pdev->rx_refill_buf_ring2, RXDMA_BUF,
				 IPA_RX_REFILL_BUF_RING_IDX, pdev->pdev_id)) {
			dp_init_err("%pK: dp_srng_init failed second"
				    "rx refill ring", soc);
			return QDF_STATUS_E_FAILURE;
		}
	}
	return QDF_STATUS_SUCCESS;
}

/**
 * dp_deinit_ipa_rx_refill_buf_ring - deinit second Rx refill buffer ring
 * @soc: data path instance
 * @pdev: core txrx pdev context
 *
 * Return: void
 */
static void dp_deinit_ipa_rx_refill_buf_ring(struct dp_soc *soc,
					     struct dp_pdev *pdev)
{
	if (wlan_cfg_is_ipa_enabled(soc->wlan_cfg_ctx))
		dp_srng_deinit(soc, &pdev->rx_refill_buf_ring2, RXDMA_BUF, 0);
}

/**
 * dp_free_ipa_rx_refill_buf_ring - free second Rx refill buffer ring
 * @soc: data path instance
 * @pdev: core txrx pdev context
 *
 * Return: void
 */
static void dp_free_ipa_rx_refill_buf_ring(struct dp_soc *soc,
					   struct dp_pdev *pdev)
{
	if (wlan_cfg_is_ipa_enabled(soc->wlan_cfg_ctx))
		dp_srng_free(soc, &pdev->rx_refill_buf_ring2);
}
#else
static int dp_setup_ipa_rx_refill_buf_ring(struct dp_soc *soc,
					   struct dp_pdev *pdev)
{
	return QDF_STATUS_SUCCESS;
}

static int dp_init_ipa_rx_refill_buf_ring(struct dp_soc *soc,
					  struct dp_pdev *pdev)
{
	return QDF_STATUS_SUCCESS;
}

static void dp_deinit_ipa_rx_refill_buf_ring(struct dp_soc *soc,
					     struct dp_pdev *pdev)
{
}

static void dp_free_ipa_rx_refill_buf_ring(struct dp_soc *soc,
					   struct dp_pdev *pdev)
{
}
#endif

#ifdef DP_TX_HW_DESC_HISTORY
/**
 * dp_soc_tx_hw_desc_history_attach - Attach TX HW descriptor history
 *
 * @soc: DP soc handle
 *
 * Return: None
 */
static void dp_soc_tx_hw_desc_history_attach(struct dp_soc *soc)
{
	soc->tx_hw_desc_history = dp_context_alloc_mem(
			soc, DP_TX_HW_DESC_HIST_TYPE,
			sizeof(*soc->tx_hw_desc_history));
	if (soc->tx_hw_desc_history)
		soc->tx_hw_desc_history->index = 0;
}

static void dp_soc_tx_hw_desc_history_detach(struct dp_soc *soc)
{
	dp_context_free_mem(soc, DP_TX_HW_DESC_HIST_TYPE,
			    soc->tx_hw_desc_history);
}

#else /* DP_TX_HW_DESC_HISTORY */
static inline void
dp_soc_tx_hw_desc_history_attach(struct dp_soc *soc)
{
}

static inline void
dp_soc_tx_hw_desc_history_detach(struct dp_soc *soc)
{
}
#endif /* DP_TX_HW_DESC_HISTORY */

#ifdef WLAN_FEATURE_DP_RX_RING_HISTORY
#ifndef RX_DEFRAG_DO_NOT_REINJECT
/**
 * dp_soc_rx_reinject_ring_history_attach - Attach the reo reinject ring
 *					    history.
 * @soc: DP soc handle
 *
 * Return: None
 */
static void dp_soc_rx_reinject_ring_history_attach(struct dp_soc *soc)
{
	soc->rx_reinject_ring_history =
		dp_context_alloc_mem(soc, DP_RX_REINJECT_RING_HIST_TYPE,
				     sizeof(struct dp_rx_reinject_history));
	if (soc->rx_reinject_ring_history)
		qdf_atomic_init(&soc->rx_reinject_ring_history->index);
}
#else /* RX_DEFRAG_DO_NOT_REINJECT */
static inline void
dp_soc_rx_reinject_ring_history_attach(struct dp_soc *soc)
{
}
#endif /* RX_DEFRAG_DO_NOT_REINJECT */

/**
 * dp_soc_rx_history_attach() - Attach the ring history record buffers
 * @soc: DP soc structure
 *
 * This function allocates the memory for recording the rx ring, rx error
 * ring and the reinject ring entries. There is no error returned in case
 * of allocation failure since the record function checks if the history is
 * initialized or not. We do not want to fail the driver load in case of
 * failure to allocate memory for debug history.
 *
 * Returns: None
 */
static void dp_soc_rx_history_attach(struct dp_soc *soc)
{
	int i;
	uint32_t rx_ring_hist_size;
	uint32_t rx_refill_ring_hist_size;

	rx_ring_hist_size = sizeof(*soc->rx_ring_history[0]);
	rx_refill_ring_hist_size = sizeof(*soc->rx_refill_ring_history[0]);

	for (i = 0; i < MAX_REO_DEST_RINGS; i++) {
		soc->rx_ring_history[i] = dp_context_alloc_mem(
				soc, DP_RX_RING_HIST_TYPE, rx_ring_hist_size);
		if (soc->rx_ring_history[i])
			qdf_atomic_init(&soc->rx_ring_history[i]->index);
	}

	soc->rx_err_ring_history = dp_context_alloc_mem(
			soc, DP_RX_ERR_RING_HIST_TYPE, rx_ring_hist_size);
	if (soc->rx_err_ring_history)
		qdf_atomic_init(&soc->rx_err_ring_history->index);

	dp_soc_rx_reinject_ring_history_attach(soc);

	for (i = 0; i < MAX_PDEV_CNT; i++) {
		soc->rx_refill_ring_history[i] = dp_context_alloc_mem(
						soc,
						DP_RX_REFILL_RING_HIST_TYPE,
						rx_refill_ring_hist_size);

		if (soc->rx_refill_ring_history[i])
			qdf_atomic_init(&soc->rx_refill_ring_history[i]->index);
	}
}

static void dp_soc_rx_history_detach(struct dp_soc *soc)
{
	int i;

	for (i = 0; i < MAX_REO_DEST_RINGS; i++)
		dp_context_free_mem(soc, DP_RX_RING_HIST_TYPE,
				    soc->rx_ring_history[i]);

	dp_context_free_mem(soc, DP_RX_ERR_RING_HIST_TYPE,
			    soc->rx_err_ring_history);

	/*
	 * No need for a featurized detach since qdf_mem_free takes
	 * care of NULL pointer.
	 */
	dp_context_free_mem(soc, DP_RX_REINJECT_RING_HIST_TYPE,
			    soc->rx_reinject_ring_history);

	for (i = 0; i < MAX_PDEV_CNT; i++)
		dp_context_free_mem(soc, DP_RX_REFILL_RING_HIST_TYPE,
				    soc->rx_refill_ring_history[i]);
}

#else
static inline void dp_soc_rx_history_attach(struct dp_soc *soc)
{
}

static inline void dp_soc_rx_history_detach(struct dp_soc *soc)
{
}
#endif

#ifdef WLAN_FEATURE_DP_TX_DESC_HISTORY
/**
 * dp_soc_tx_history_attach() - Attach the ring history record buffers
 * @soc: DP soc structure
 *
 * This function allocates the memory for recording the tx tcl ring and
 * the tx comp ring entries. There is no error returned in case
 * of allocation failure since the record function checks if the history is
 * initialized or not. We do not want to fail the driver load in case of
 * failure to allocate memory for debug history.
 *
 * Returns: None
 */
static void dp_soc_tx_history_attach(struct dp_soc *soc)
{
	uint32_t tx_tcl_hist_size;
	uint32_t tx_comp_hist_size;

	tx_tcl_hist_size = sizeof(*soc->tx_tcl_history);
	soc->tx_tcl_history = dp_context_alloc_mem(soc, DP_TX_TCL_HIST_TYPE,
						   tx_tcl_hist_size);
	if (soc->tx_tcl_history)
		qdf_atomic_init(&soc->tx_tcl_history->index);

	tx_comp_hist_size = sizeof(*soc->tx_comp_history);
	soc->tx_comp_history = dp_context_alloc_mem(soc, DP_TX_COMP_HIST_TYPE,
						    tx_comp_hist_size);
	if (soc->tx_comp_history)
		qdf_atomic_init(&soc->tx_comp_history->index);
}

/**
 * dp_soc_tx_history_detach() - Detach the ring history record buffers
 * @soc: DP soc structure
 *
 * This function frees the memory for recording the tx tcl ring and
 * the tx comp ring entries.
 *
 * Returns: None
 */
static void dp_soc_tx_history_detach(struct dp_soc *soc)
{
	dp_context_free_mem(soc, DP_TX_TCL_HIST_TYPE, soc->tx_tcl_history);
	dp_context_free_mem(soc, DP_TX_COMP_HIST_TYPE, soc->tx_comp_history);
}

#else
static inline void dp_soc_tx_history_attach(struct dp_soc *soc)
{
}

static inline void dp_soc_tx_history_detach(struct dp_soc *soc)
{
}
#endif /* WLAN_FEATURE_DP_TX_DESC_HISTORY */

/*
* dp_pdev_attach_wifi3() - attach txrx pdev
* @txrx_soc: Datapath SOC handle
* @params: Params for PDEV attach
*
* Return: QDF_STATUS
*/
static inline
QDF_STATUS dp_pdev_attach_wifi3(struct cdp_soc_t *txrx_soc,
				struct cdp_pdev_attach_params *params)
{
	qdf_size_t pdev_context_size;
	struct dp_soc *soc = (struct dp_soc *)txrx_soc;
	struct dp_pdev *pdev = NULL;
	uint8_t pdev_id = params->pdev_id;
	struct wlan_cfg_dp_soc_ctxt *soc_cfg_ctx;
	int nss_cfg;

	pdev_context_size =
		soc->arch_ops.txrx_get_context_size(DP_CONTEXT_TYPE_PDEV);
	pdev = dp_context_alloc_mem(soc, DP_PDEV_TYPE, pdev_context_size);
	if (!pdev) {
		dp_init_err("%pK: DP PDEV memory allocation failed",
			    soc);
		goto fail0;
	}
	wlan_minidump_log(pdev, sizeof(*pdev), soc->ctrl_psoc,
			  WLAN_MD_DP_PDEV, "dp_pdev");

	soc_cfg_ctx = soc->wlan_cfg_ctx;
	pdev->wlan_cfg_ctx = wlan_cfg_pdev_attach(soc->ctrl_psoc);

	if (!pdev->wlan_cfg_ctx) {
		dp_init_err("%pK: pdev cfg_attach failed", soc);
		goto fail1;
	}

	/*
	 * set nss pdev config based on soc config
	 */
	nss_cfg = wlan_cfg_get_dp_soc_nss_cfg(soc_cfg_ctx);
	wlan_cfg_set_dp_pdev_nss_enabled(pdev->wlan_cfg_ctx,
					 (nss_cfg & (1 << pdev_id)));

	pdev->soc = soc;
	pdev->pdev_id = pdev_id;
	soc->pdev_list[pdev_id] = pdev;

	pdev->lmac_id = wlan_cfg_get_hw_mac_idx(soc->wlan_cfg_ctx, pdev_id);
	soc->pdev_count++;

	/* Allocate memory for pdev srng rings */
	if (dp_pdev_srng_alloc(pdev)) {
		dp_init_err("%pK: dp_pdev_srng_alloc failed", soc);
		goto fail2;
	}

	/* Setup second Rx refill buffer ring */
	if (dp_setup_ipa_rx_refill_buf_ring(soc, pdev)) {
		dp_init_err("%pK: dp_srng_alloc failed rxrefill2 ring",
			    soc);
		goto fail3;
	}

	/* Allocate memory for pdev rxdma rings */
	if (dp_rxdma_ring_alloc(soc, pdev)) {
		dp_init_err("%pK: dp_rxdma_ring_alloc failed", soc);
		goto fail4;
	}

	/* Rx specific init */
	if (dp_rx_pdev_desc_pool_alloc(pdev)) {
		dp_init_err("%pK: dp_rx_pdev_attach failed", soc);
		goto fail4;
	}

	if (dp_monitor_pdev_attach(pdev)) {
		dp_init_err("%pK: dp_monitor_pdev_attach failed", soc);
		goto fail5;
	}

	soc->arch_ops.txrx_pdev_attach(pdev, params);

	return QDF_STATUS_SUCCESS;
fail5:
	dp_rx_pdev_desc_pool_free(pdev);
fail4:
	dp_rxdma_ring_free(pdev);
	dp_free_ipa_rx_refill_buf_ring(soc, pdev);
fail3:
	dp_pdev_srng_free(pdev);
fail2:
	wlan_cfg_pdev_detach(pdev->wlan_cfg_ctx);
fail1:
	soc->pdev_list[pdev_id] = NULL;
	qdf_mem_free(pdev);
fail0:
	return QDF_STATUS_E_FAILURE;
}



#ifdef WLAN_DP_PENDING_MEM_FLUSH
/**
 * dp_pdev_flush_pending_vdevs() - Flush all delete pending vdevs in pdev
 * @pdev: Datapath PDEV handle
 *
 * This is the last chance to flush all pending dp vdevs/peers,
 * some peer/vdev leak case like Non-SSR + peer unmap missing
 * will be covered here.
 *
 * Return: None
 */
static void dp_pdev_flush_pending_vdevs(struct dp_pdev *pdev)
{
	struct dp_vdev *vdev = NULL;
	struct dp_soc *soc = pdev->soc;

	if (TAILQ_EMPTY(&soc->inactive_vdev_list))
		return;

	while (true) {
		qdf_spin_lock_bh(&soc->inactive_vdev_list_lock);
		TAILQ_FOREACH(vdev, &soc->inactive_vdev_list,
			      inactive_list_elem) {
			if (vdev->pdev == pdev)
				break;
		}
		qdf_spin_unlock_bh(&soc->inactive_vdev_list_lock);

		/* vdev will be freed when all peers get cleanup */
		if (vdev)
			dp_vdev_flush_peers((struct cdp_vdev *)vdev, 0);
		else
			break;
	}
}
#else
static void dp_pdev_flush_pending_vdevs(struct dp_pdev *pdev)
{
}
#endif

#ifdef QCA_VDEV_STATS_HW_OFFLOAD_SUPPORT
/**
 * dp_vdev_stats_hw_offload_target_config() - Send HTT command to FW
 *                                          for enable/disable of HW vdev stats
 * @soc: Datapath soc handle
 * @pdev_id: INVALID_PDEV_ID for all pdevs or 0,1,2 for individual pdev
 * @enable: flag to reprsent enable/disable of hw vdev stats
 *
 * Return: none
 */
static void dp_vdev_stats_hw_offload_target_config(struct dp_soc *soc,
						   uint8_t pdev_id,
						   bool enable)
{
	/* Check SOC level config for HW offload vdev stats support */
	if (!wlan_cfg_get_vdev_stats_hw_offload_config(soc->wlan_cfg_ctx)) {
		dp_debug("%pK: HW vdev offload stats is disabled", soc);
		return;
	}

	/* Send HTT command to FW for enable of stats */
	dp_h2t_hw_vdev_stats_config_send(soc, pdev_id, enable, false, 0);
}

/**
 * dp_vdev_stats_hw_offload_target_clear() - Clear HW vdev stats on target
 * @soc: Datapath soc handle
 * @pdev_id: pdev_id (0,1,2)
 * @bitmask: bitmask with vdev_id(s) for which stats are to be cleared on HW
 *
 * Return: none
 */
static
void dp_vdev_stats_hw_offload_target_clear(struct dp_soc *soc, uint8_t pdev_id,
					   uint64_t vdev_id_bitmask)
{
	/* Check SOC level config for HW offload vdev stats support */
	if (!wlan_cfg_get_vdev_stats_hw_offload_config(soc->wlan_cfg_ctx)) {
		dp_debug("%pK: HW vdev offload stats is disabled", soc);
		return;
	}

	/* Send HTT command to FW for reset of stats */
	dp_h2t_hw_vdev_stats_config_send(soc, pdev_id, true, true,
					 vdev_id_bitmask);
}
#else
static void
dp_vdev_stats_hw_offload_target_config(struct dp_soc *soc, uint8_t pdev_id,
				       bool enable)
{
}

static
void dp_vdev_stats_hw_offload_target_clear(struct dp_soc *soc, uint8_t pdev_id,
					   uint64_t vdev_id_bitmask)
{
}
#endif /*QCA_VDEV_STATS_HW_OFFLOAD_SUPPORT */

/**
 * dp_pdev_deinit() - Deinit txrx pdev
 * @txrx_pdev: Datapath PDEV handle
 * @force: Force deinit
 *
 * Return: None
 */
static void dp_pdev_deinit(struct cdp_pdev *txrx_pdev, int force)
{
	struct dp_pdev *pdev = (struct dp_pdev *)txrx_pdev;
	qdf_nbuf_t curr_nbuf, next_nbuf;

	if (pdev->pdev_deinit)
		return;

	dp_tx_me_exit(pdev);
	dp_rx_fst_detach(pdev->soc, pdev);
	dp_rx_pdev_buffers_free(pdev);
	dp_rx_pdev_desc_pool_deinit(pdev);
	dp_pdev_bkp_stats_detach(pdev);
	qdf_event_destroy(&pdev->fw_peer_stats_event);
	if (pdev->sojourn_buf)
		qdf_nbuf_free(pdev->sojourn_buf);

	dp_pdev_flush_pending_vdevs(pdev);
	dp_tx_desc_flush(pdev, NULL, true);

	qdf_spinlock_destroy(&pdev->tx_mutex);
	qdf_spinlock_destroy(&pdev->vdev_list_lock);

	if (pdev->invalid_peer)
		qdf_mem_free(pdev->invalid_peer);

	dp_monitor_pdev_deinit(pdev);

	dp_pdev_srng_deinit(pdev);

	dp_ipa_uc_detach(pdev->soc, pdev);
	dp_deinit_ipa_rx_refill_buf_ring(pdev->soc, pdev);
	dp_rxdma_ring_cleanup(pdev->soc, pdev);

	curr_nbuf = pdev->invalid_peer_head_msdu;
	while (curr_nbuf) {
		next_nbuf = qdf_nbuf_next(curr_nbuf);
		qdf_nbuf_free(curr_nbuf);
		curr_nbuf = next_nbuf;
	}
	pdev->invalid_peer_head_msdu = NULL;
	pdev->invalid_peer_tail_msdu = NULL;

	dp_wdi_event_detach(pdev);
	pdev->pdev_deinit = 1;
}

/**
 * dp_pdev_deinit_wifi3() - Deinit txrx pdev
 * @psoc: Datapath psoc handle
 * @pdev_id: Id of datapath PDEV handle
 * @force: Force deinit
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS
dp_pdev_deinit_wifi3(struct cdp_soc_t *psoc, uint8_t pdev_id,
		     int force)
{
	struct dp_pdev *txrx_pdev;

	txrx_pdev = dp_get_pdev_from_soc_pdev_id_wifi3((struct dp_soc *)psoc,
						       pdev_id);

	if (!txrx_pdev)
		return QDF_STATUS_E_FAILURE;

	dp_pdev_deinit((struct cdp_pdev *)txrx_pdev, force);

	return QDF_STATUS_SUCCESS;
}

/*
 * dp_pdev_post_attach() - Do post pdev attach after dev_alloc_name
 * @txrx_pdev: Datapath PDEV handle
 *
 * Return: None
 */
static void dp_pdev_post_attach(struct cdp_pdev *txrx_pdev)
{
	struct dp_pdev *pdev = (struct dp_pdev *)txrx_pdev;

	dp_monitor_tx_capture_debugfs_init(pdev);

	if (dp_pdev_htt_stats_dbgfs_init(pdev)) {
		dp_init_err("%pK: Failed to initialize pdev HTT stats debugfs", pdev->soc);
	}
}

/*
 * dp_pdev_post_attach_wifi3() - attach txrx pdev post
 * @psoc: Datapath soc handle
 * @pdev_id: pdev id of pdev
 *
 * Return: QDF_STATUS
 */
static int dp_pdev_post_attach_wifi3(struct cdp_soc_t *soc,
				     uint8_t pdev_id)
{
	struct dp_pdev *pdev;

	pdev = dp_get_pdev_from_soc_pdev_id_wifi3((struct dp_soc *)soc,
						  pdev_id);

	if (!pdev) {
		dp_init_err("%pK: DP PDEV is Null for pdev id %d",
			    (struct dp_soc *)soc, pdev_id);
		return QDF_STATUS_E_FAILURE;
	}

	dp_pdev_post_attach((struct cdp_pdev *)pdev);
	return QDF_STATUS_SUCCESS;
}

/*
 * dp_pdev_detach() - Complete rest of pdev detach
 * @txrx_pdev: Datapath PDEV handle
 * @force: Force deinit
 *
 * Return: None
 */
static void dp_pdev_detach(struct cdp_pdev *txrx_pdev, int force)
{
	struct dp_pdev *pdev = (struct dp_pdev *)txrx_pdev;
	struct dp_soc *soc = pdev->soc;

	dp_pdev_htt_stats_dbgfs_deinit(pdev);
	dp_rx_pdev_desc_pool_free(pdev);
	dp_monitor_pdev_detach(pdev);
	dp_rxdma_ring_free(pdev);
	dp_free_ipa_rx_refill_buf_ring(soc, pdev);
	dp_pdev_srng_free(pdev);

	soc->pdev_count--;
	soc->pdev_list[pdev->pdev_id] = NULL;

	wlan_cfg_pdev_detach(pdev->wlan_cfg_ctx);
	wlan_minidump_remove(pdev, sizeof(*pdev), soc->ctrl_psoc,
			     WLAN_MD_DP_PDEV, "dp_pdev");
	dp_context_free_mem(soc, DP_PDEV_TYPE, pdev);
}

/*
 * dp_pdev_detach_wifi3() - detach txrx pdev
 * @psoc: Datapath soc handle
 * @pdev_id: pdev id of pdev
 * @force: Force detach
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS dp_pdev_detach_wifi3(struct cdp_soc_t *psoc, uint8_t pdev_id,
				       int force)
{
	struct dp_pdev *pdev;

	pdev = dp_get_pdev_from_soc_pdev_id_wifi3((struct dp_soc *)psoc,
						  pdev_id);

	if (!pdev) {
		dp_init_err("%pK: DP PDEV is Null for pdev id %d",
			    (struct dp_soc *)psoc, pdev_id);
		return QDF_STATUS_E_FAILURE;
	}

	dp_pdev_detach((struct cdp_pdev *)pdev, force);
	return QDF_STATUS_SUCCESS;
}

/*
 * dp_reo_desc_freelist_destroy() - Flush REO descriptors from deferred freelist
 * @soc: DP SOC handle
 */
static inline void dp_reo_desc_freelist_destroy(struct dp_soc *soc)
{
	struct reo_desc_list_node *desc;
	struct dp_rx_tid *rx_tid;

	qdf_spin_lock_bh(&soc->reo_desc_freelist_lock);
	while (qdf_list_remove_front(&soc->reo_desc_freelist,
		(qdf_list_node_t **)&desc) == QDF_STATUS_SUCCESS) {
		rx_tid = &desc->rx_tid;
		qdf_mem_unmap_nbytes_single(soc->osdev,
			rx_tid->hw_qdesc_paddr,
			QDF_DMA_BIDIRECTIONAL,
			rx_tid->hw_qdesc_alloc_size);
		qdf_mem_free(rx_tid->hw_qdesc_vaddr_unaligned);
		qdf_mem_free(desc);
	}
	qdf_spin_unlock_bh(&soc->reo_desc_freelist_lock);
	qdf_list_destroy(&soc->reo_desc_freelist);
	qdf_spinlock_destroy(&soc->reo_desc_freelist_lock);
}

#ifdef WLAN_DP_FEATURE_DEFERRED_REO_QDESC_DESTROY
/*
 * dp_reo_desc_deferred_freelist_create() - Initialize the resources used
 *                                          for deferred reo desc list
 * @psoc: Datapath soc handle
 *
 * Return: void
 */
static void dp_reo_desc_deferred_freelist_create(struct dp_soc *soc)
{
	qdf_spinlock_create(&soc->reo_desc_deferred_freelist_lock);
	qdf_list_create(&soc->reo_desc_deferred_freelist,
			REO_DESC_DEFERRED_FREELIST_SIZE);
	soc->reo_desc_deferred_freelist_init = true;
}

/*
 * dp_reo_desc_deferred_freelist_destroy() - loop the deferred free list &
 *                                           free the leftover REO QDESCs
 * @psoc: Datapath soc handle
 *
 * Return: void
 */
static void dp_reo_desc_deferred_freelist_destroy(struct dp_soc *soc)
{
	struct reo_desc_deferred_freelist_node *desc;

	qdf_spin_lock_bh(&soc->reo_desc_deferred_freelist_lock);
	soc->reo_desc_deferred_freelist_init = false;
	while (qdf_list_remove_front(&soc->reo_desc_deferred_freelist,
	       (qdf_list_node_t **)&desc) == QDF_STATUS_SUCCESS) {
		qdf_mem_unmap_nbytes_single(soc->osdev,
					    desc->hw_qdesc_paddr,
					    QDF_DMA_BIDIRECTIONAL,
					    desc->hw_qdesc_alloc_size);
		qdf_mem_free(desc->hw_qdesc_vaddr_unaligned);
		qdf_mem_free(desc);
	}
	qdf_spin_unlock_bh(&soc->reo_desc_deferred_freelist_lock);

	qdf_list_destroy(&soc->reo_desc_deferred_freelist);
	qdf_spinlock_destroy(&soc->reo_desc_deferred_freelist_lock);
}
#else
static inline void dp_reo_desc_deferred_freelist_create(struct dp_soc *soc)
{
}

static inline void dp_reo_desc_deferred_freelist_destroy(struct dp_soc *soc)
{
}
#endif /* !WLAN_DP_FEATURE_DEFERRED_REO_QDESC_DESTROY */

/*
 * dp_soc_reset_txrx_ring_map() - reset tx ring map
 * @soc: DP SOC handle
 *
 */
static void dp_soc_reset_txrx_ring_map(struct dp_soc *soc)
{
	uint32_t i;

	for (i = 0; i < WLAN_CFG_INT_NUM_CONTEXTS; i++)
		soc->tx_ring_map[i] = 0;
}

/*
 * dp_soc_print_inactive_objects() - prints inactive peer and vdev list
 * @soc: DP SOC handle
 *
 */
static void dp_soc_print_inactive_objects(struct dp_soc *soc)
{
	struct dp_peer *peer = NULL;
	struct dp_peer *tmp_peer = NULL;
	struct dp_vdev *vdev = NULL;
	struct dp_vdev *tmp_vdev = NULL;
	int i = 0;
	uint32_t count;

	if (TAILQ_EMPTY(&soc->inactive_peer_list) &&
	    TAILQ_EMPTY(&soc->inactive_vdev_list))
		return;

	TAILQ_FOREACH_SAFE(peer, &soc->inactive_peer_list,
			   inactive_list_elem, tmp_peer) {
		for (i = 0; i < DP_MOD_ID_MAX; i++) {
			count = qdf_atomic_read(&peer->mod_refs[i]);
			if (count)
				DP_PRINT_STATS("peer %pK Module id %u ==> %u",
					       peer, i, count);
		}
	}

	TAILQ_FOREACH_SAFE(vdev, &soc->inactive_vdev_list,
			   inactive_list_elem, tmp_vdev) {
		for (i = 0; i < DP_MOD_ID_MAX; i++) {
			count = qdf_atomic_read(&vdev->mod_refs[i]);
			if (count)
				DP_PRINT_STATS("vdev %pK Module id %u ==> %u",
					       vdev, i, count);
		}
	}
	QDF_BUG(0);
}

/**
 * dp_soc_deinit() - Deinitialize txrx SOC
 * @txrx_soc: Opaque DP SOC handle
 *
 * Return: None
 */
static void dp_soc_deinit(void *txrx_soc)
{
	struct dp_soc *soc = (struct dp_soc *)txrx_soc;
	struct htt_soc *htt_soc = soc->htt_handle;

	qdf_atomic_set(&soc->cmn_init_done, 0);

	soc->arch_ops.txrx_soc_deinit(soc);

	/* free peer tables & AST tables allocated during peer_map_attach */
	if (soc->peer_map_attach_success) {
		dp_peer_find_detach(soc);
		soc->arch_ops.txrx_peer_map_detach(soc);
		soc->peer_map_attach_success = FALSE;
	}

	qdf_flush_work(&soc->htt_stats.work);
	qdf_disable_work(&soc->htt_stats.work);

	qdf_spinlock_destroy(&soc->htt_stats.lock);

	dp_soc_reset_txrx_ring_map(soc);

	dp_reo_desc_freelist_destroy(soc);
	dp_reo_desc_deferred_freelist_destroy(soc);

	DEINIT_RX_HW_STATS_LOCK(soc);

	qdf_spinlock_destroy(&soc->ast_lock);

	dp_peer_mec_spinlock_destroy(soc);

	qdf_nbuf_queue_free(&soc->htt_stats.msg);

	qdf_nbuf_queue_free(&soc->invalid_buf_queue);

	qdf_spinlock_destroy(&soc->rx.defrag.defrag_lock);

	qdf_spinlock_destroy(&soc->vdev_map_lock);

	dp_reo_cmdlist_destroy(soc);
	qdf_spinlock_destroy(&soc->rx.reo_cmd_lock);

	dp_soc_tx_desc_sw_pools_deinit(soc);

	dp_soc_srng_deinit(soc);

	dp_hw_link_desc_ring_deinit(soc);

	dp_soc_print_inactive_objects(soc);
	qdf_spinlock_destroy(&soc->inactive_peer_list_lock);
	qdf_spinlock_destroy(&soc->inactive_vdev_list_lock);

	htt_soc_htc_dealloc(soc->htt_handle);

	htt_soc_detach(htt_soc);

	/* Free wbm sg list and reset flags in down path */
	dp_rx_wbm_sg_list_deinit(soc);

	wlan_minidump_remove(soc, sizeof(*soc), soc->ctrl_psoc,
			     WLAN_MD_DP_SOC, "dp_soc");
}

/**
 * dp_soc_deinit_wifi3() - Deinitialize txrx SOC
 * @txrx_soc: Opaque DP SOC handle
 *
 * Return: None
 */
static void dp_soc_deinit_wifi3(struct cdp_soc_t *txrx_soc)
{
	dp_soc_deinit(txrx_soc);
}

/*
 * dp_soc_detach() - Detach rest of txrx SOC
 * @txrx_soc: DP SOC handle, struct cdp_soc_t is first element of struct dp_soc.
 *
 * Return: None
 */
static void dp_soc_detach(struct cdp_soc_t *txrx_soc)
{
	struct dp_soc *soc = (struct dp_soc *)txrx_soc;

	soc->arch_ops.txrx_soc_detach(soc);

	dp_sysfs_deinitialize_stats(soc);
	dp_soc_swlm_detach(soc);
	dp_soc_tx_desc_sw_pools_free(soc);
	dp_soc_srng_free(soc);
	dp_hw_link_desc_ring_free(soc);
	dp_hw_link_desc_pool_banks_free(soc, WLAN_INVALID_PDEV_ID);
	wlan_cfg_soc_detach(soc->wlan_cfg_ctx);
	dp_soc_tx_hw_desc_history_detach(soc);
	dp_soc_tx_history_detach(soc);
	dp_soc_rx_history_detach(soc);

	if (!dp_monitor_modularized_enable()) {
		dp_mon_soc_detach_wrapper(soc);
	}

	qdf_mem_free(soc->cdp_soc.ops);
	qdf_mem_free(soc);
}

/*
 * dp_soc_detach_wifi3() - Detach txrx SOC
 * @txrx_soc: DP SOC handle, struct cdp_soc_t is first element of struct dp_soc.
 *
 * Return: None
 */
static void dp_soc_detach_wifi3(struct cdp_soc_t *txrx_soc)
{
	dp_soc_detach(txrx_soc);
}

/*
 * dp_rxdma_ring_config() - configure the RX DMA rings
 *
 * This function is used to configure the MAC rings.
 * On MCL host provides buffers in Host2FW ring
 * FW refills (copies) buffers to the ring and updates
 * ring_idx in register
 *
 * @soc: data path SoC handle
 *
 * Return: zero on success, non-zero on failure
 */
#ifdef QCA_HOST2FW_RXBUF_RING
static inline void
dp_htt_setup_rxdma_err_dst_ring(struct dp_soc *soc, int mac_id,
				int lmac_id)
{
	if (soc->rxdma_err_dst_ring[lmac_id].hal_srng)
		htt_srng_setup(soc->htt_handle, mac_id,
			       soc->rxdma_err_dst_ring[lmac_id].hal_srng,
			       RXDMA_DST);
}

static QDF_STATUS dp_rxdma_ring_config(struct dp_soc *soc)
{
	int i;
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	for (i = 0; i < MAX_PDEV_CNT; i++) {
		struct dp_pdev *pdev = soc->pdev_list[i];

		if (pdev) {
			int mac_id;
			int max_mac_rings =
				 wlan_cfg_get_num_mac_rings
				(pdev->wlan_cfg_ctx);
			int lmac_id = dp_get_lmac_id_for_pdev_id(soc, 0, i);

			htt_srng_setup(soc->htt_handle, i,
				       soc->rx_refill_buf_ring[lmac_id]
				       .hal_srng,
				       RXDMA_BUF);

			if (pdev->rx_refill_buf_ring2.hal_srng)
				htt_srng_setup(soc->htt_handle, i,
					       pdev->rx_refill_buf_ring2
					       .hal_srng,
					       RXDMA_BUF);

			dp_update_num_mac_rings_for_dbs(soc, &max_mac_rings);
			dp_err("pdev_id %d max_mac_rings %d",
			       pdev->pdev_id, max_mac_rings);

			for (mac_id = 0; mac_id < max_mac_rings; mac_id++) {
				int mac_for_pdev =
					dp_get_mac_id_for_pdev(mac_id,
							       pdev->pdev_id);
				/*
				 * Obtain lmac id from pdev to access the LMAC
				 * ring in soc context
				 */
				lmac_id =
				dp_get_lmac_id_for_pdev_id(soc,
							   mac_id,
							   pdev->pdev_id);
				QDF_TRACE(QDF_MODULE_ID_TXRX,
					 QDF_TRACE_LEVEL_ERROR,
					 FL("mac_id %d"), mac_for_pdev);

				htt_srng_setup(soc->htt_handle, mac_for_pdev,
					 pdev->rx_mac_buf_ring[mac_id]
						.hal_srng,
					 RXDMA_BUF);

				if (!soc->rxdma2sw_rings_not_supported)
					dp_htt_setup_rxdma_err_dst_ring(soc,
						mac_for_pdev, lmac_id);

				/* Configure monitor mode rings */
				status = dp_monitor_htt_srng_setup(soc, pdev,
								   lmac_id,
								   mac_for_pdev);
				if (status != QDF_STATUS_SUCCESS) {
					dp_err("Failed to send htt monitor messages to target");
					return status;
				}

			}
		}
	}

	dp_reap_timer_init(soc);
	return status;
}
#else
/* This is only for WIN */
static QDF_STATUS dp_rxdma_ring_config(struct dp_soc *soc)
{
	int i;
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	int mac_for_pdev;
	int lmac_id;

	/* Configure monitor mode rings */
	dp_monitor_soc_htt_srng_setup(soc);

	for (i = 0; i < MAX_PDEV_CNT; i++) {
		struct dp_pdev *pdev =  soc->pdev_list[i];

		if (!pdev)
			continue;

		mac_for_pdev = i;
		lmac_id = dp_get_lmac_id_for_pdev_id(soc, 0, i);

		if (soc->rx_refill_buf_ring[lmac_id].hal_srng)
			htt_srng_setup(soc->htt_handle, mac_for_pdev,
				       soc->rx_refill_buf_ring[lmac_id].
				       hal_srng, RXDMA_BUF);

		/* Configure monitor mode rings */
		dp_monitor_htt_srng_setup(soc, pdev,
					  lmac_id,
					  mac_for_pdev);
		if (!soc->rxdma2sw_rings_not_supported)
			htt_srng_setup(soc->htt_handle, mac_for_pdev,
				       soc->rxdma_err_dst_ring[lmac_id].hal_srng,
				       RXDMA_DST);
	}

	dp_reap_timer_init(soc);
	return status;
}
#endif

/*
 * dp_rx_target_fst_config() - configure the RXOLE Flow Search Engine
 *
 * This function is used to configure the FSE HW block in RX OLE on a
 * per pdev basis. Here, we will be programming parameters related to
 * the Flow Search Table.
 *
 * @soc: data path SoC handle
 *
 * Return: zero on success, non-zero on failure
 */
#ifdef WLAN_SUPPORT_RX_FLOW_TAG
static QDF_STATUS
dp_rx_target_fst_config(struct dp_soc *soc)
{
	int i;
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	for (i = 0; i < MAX_PDEV_CNT; i++) {
		struct dp_pdev *pdev = soc->pdev_list[i];

		/* Flow search is not enabled if NSS offload is enabled */
		if (pdev &&
		    !wlan_cfg_get_dp_pdev_nss_enabled(pdev->wlan_cfg_ctx)) {
			status = dp_rx_flow_send_fst_fw_setup(pdev->soc, pdev);
			if (status != QDF_STATUS_SUCCESS)
				break;
		}
	}
	return status;
}
#elif defined(WLAN_SUPPORT_RX_FISA)
/**
 * dp_rx_target_fst_config() - Configure RX OLE FSE engine in HW
 * @soc: SoC handle
 *
 * Return: Success
 */
static inline QDF_STATUS dp_rx_target_fst_config(struct dp_soc *soc)
{
	/* Check if it is enabled in the INI */
	if (!soc->fisa_enable) {
		dp_err("RX FISA feature is disabled");
		return QDF_STATUS_E_NOSUPPORT;
	}

	return dp_rx_flow_send_fst_fw_setup(soc, soc->pdev_list[0]);
}

#define FISA_MAX_TIMEOUT 0xffffffff
#define FISA_DISABLE_TIMEOUT 0
static QDF_STATUS dp_rx_fisa_config(struct dp_soc *soc)
{
	struct dp_htt_rx_fisa_cfg fisa_config;

	fisa_config.pdev_id = 0;
	fisa_config.fisa_timeout = FISA_MAX_TIMEOUT;

	return dp_htt_rx_fisa_config(soc->pdev_list[0], &fisa_config);
}
#else /* !WLAN_SUPPORT_RX_FISA */
static inline QDF_STATUS dp_rx_target_fst_config(struct dp_soc *soc)
{
	return QDF_STATUS_SUCCESS;
}
#endif /* !WLAN_SUPPORT_RX_FISA */

#ifndef WLAN_SUPPORT_RX_FISA
static QDF_STATUS dp_rx_fisa_config(struct dp_soc *soc)
{
	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS dp_rx_dump_fisa_stats(struct dp_soc *soc)
{
	return QDF_STATUS_SUCCESS;
}

static void dp_rx_dump_fisa_table(struct dp_soc *soc)
{
}

static void dp_suspend_fse_cache_flush(struct dp_soc *soc)
{
}

static void dp_resume_fse_cache_flush(struct dp_soc *soc)
{
}
#endif /* !WLAN_SUPPORT_RX_FISA */

#ifndef WLAN_DP_FEATURE_SW_LATENCY_MGR
static inline QDF_STATUS dp_print_swlm_stats(struct dp_soc *soc)
{
	return QDF_STATUS_SUCCESS;
}
#endif /* !WLAN_DP_FEATURE_SW_LATENCY_MGR */

/*
 * dp_soc_attach_target_wifi3() - SOC initialization in the target
 * @cdp_soc: Opaque Datapath SOC handle
 *
 * Return: zero on success, non-zero on failure
 */
static QDF_STATUS
dp_soc_attach_target_wifi3(struct cdp_soc_t *cdp_soc)
{
	struct dp_soc *soc = (struct dp_soc *)cdp_soc;
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	htt_soc_attach_target(soc->htt_handle);

	status = dp_rxdma_ring_config(soc);
	if (status != QDF_STATUS_SUCCESS) {
		dp_err("Failed to send htt srng setup messages to target");
		return status;
	}

	status = soc->arch_ops.dp_rxdma_ring_sel_cfg(soc);
	if (status != QDF_STATUS_SUCCESS) {
		dp_err("Failed to send htt ring config message to target");
		return status;
	}

	status = dp_rx_target_fst_config(soc);
	if (status != QDF_STATUS_SUCCESS &&
	    status != QDF_STATUS_E_NOSUPPORT) {
		dp_err("Failed to send htt fst setup config message to target");
		return status;
	}

	if (status == QDF_STATUS_SUCCESS) {
		status = dp_rx_fisa_config(soc);
		if (status != QDF_STATUS_SUCCESS) {
			dp_err("Failed to send htt FISA config message to target");
			return status;
		}
	}

	DP_STATS_INIT(soc);

	dp_runtime_init(soc);

	/* Enable HW vdev offload stats if feature is supported */
	dp_vdev_stats_hw_offload_target_config(soc, INVALID_PDEV_ID, true);

	/* initialize work queue for stats processing */
	qdf_create_work(0, &soc->htt_stats.work, htt_t2h_stats_handler, soc);

	return QDF_STATUS_SUCCESS;
}

/*
 * dp_vdev_id_map_tbl_add() - Add vdev into vdev_id table
 * @soc: SoC handle
 * @vdev: vdev handle
 * @vdev_id: vdev_id
 *
 * Return: None
 */
static void dp_vdev_id_map_tbl_add(struct dp_soc *soc,
				   struct dp_vdev *vdev,
				   uint8_t vdev_id)
{
	QDF_ASSERT(vdev_id <= MAX_VDEV_CNT);

	qdf_spin_lock_bh(&soc->vdev_map_lock);

	if (dp_vdev_get_ref(soc, vdev, DP_MOD_ID_CONFIG) !=
			QDF_STATUS_SUCCESS) {
		dp_vdev_info("%pK: unable to get vdev reference at MAP vdev %pK vdev_id %u",
			     soc, vdev, vdev_id);
		qdf_spin_unlock_bh(&soc->vdev_map_lock);
		return;
	}

	if (!soc->vdev_id_map[vdev_id])
		soc->vdev_id_map[vdev_id] = vdev;
	else
		QDF_ASSERT(0);

	qdf_spin_unlock_bh(&soc->vdev_map_lock);
}

/*
 * dp_vdev_id_map_tbl_remove() - remove vdev from vdev_id table
 * @soc: SoC handle
 * @vdev: vdev handle
 *
 * Return: None
 */
static void dp_vdev_id_map_tbl_remove(struct dp_soc *soc,
				      struct dp_vdev *vdev)
{
	qdf_spin_lock_bh(&soc->vdev_map_lock);
	QDF_ASSERT(soc->vdev_id_map[vdev->vdev_id] == vdev);

	soc->vdev_id_map[vdev->vdev_id] = NULL;
	dp_vdev_unref_delete(soc, vdev, DP_MOD_ID_CONFIG);
	qdf_spin_unlock_bh(&soc->vdev_map_lock);
}

/*
 * dp_vdev_pdev_list_add() - add vdev into pdev's list
 * @soc: soc handle
 * @pdev: pdev handle
 * @vdev: vdev handle
 *
 * return: none
 */
static void dp_vdev_pdev_list_add(struct dp_soc *soc,
				  struct dp_pdev *pdev,
				  struct dp_vdev *vdev)
{
	qdf_spin_lock_bh(&pdev->vdev_list_lock);
	if (dp_vdev_get_ref(soc, vdev, DP_MOD_ID_CONFIG) !=
			QDF_STATUS_SUCCESS) {
		dp_vdev_info("%pK: unable to get vdev reference at MAP vdev %pK",
			     soc, vdev);
		qdf_spin_unlock_bh(&pdev->vdev_list_lock);
		return;
	}
	/* add this vdev into the pdev's list */
	TAILQ_INSERT_TAIL(&pdev->vdev_list, vdev, vdev_list_elem);
	qdf_spin_unlock_bh(&pdev->vdev_list_lock);
}

/*
 * dp_vdev_pdev_list_remove() - remove vdev from pdev's list
 * @soc: SoC handle
 * @pdev: pdev handle
 * @vdev: VDEV handle
 *
 * Return: none
 */
static void dp_vdev_pdev_list_remove(struct dp_soc *soc,
				     struct dp_pdev *pdev,
				     struct dp_vdev *vdev)
{
	uint8_t found = 0;
	struct dp_vdev *tmpvdev = NULL;

	qdf_spin_lock_bh(&pdev->vdev_list_lock);
	TAILQ_FOREACH(tmpvdev, &pdev->vdev_list, vdev_list_elem) {
		if (tmpvdev == vdev) {
			found = 1;
			break;
		}
	}

	if (found) {
		TAILQ_REMOVE(&pdev->vdev_list, vdev, vdev_list_elem);
		dp_vdev_unref_delete(soc, vdev, DP_MOD_ID_CONFIG);
	} else {
		dp_vdev_debug("%pK: vdev:%pK not found in pdev:%pK vdevlist:%pK",
			      soc, vdev, pdev, &pdev->vdev_list);
		QDF_ASSERT(0);
	}
	qdf_spin_unlock_bh(&pdev->vdev_list_lock);
}

#ifdef QCA_SUPPORT_EAPOL_OVER_CONTROL_PORT
/*
 * dp_vdev_init_rx_eapol() - initializing osif_rx_eapol
 * @vdev: Datapath VDEV handle
 *
 * Return: None
 */
static inline void dp_vdev_init_rx_eapol(struct dp_vdev *vdev)
{
	vdev->osif_rx_eapol = NULL;
}

/*
 * dp_vdev_register_rx_eapol() - Register VDEV operations for rx_eapol
 * @vdev: DP vdev handle
 * @txrx_ops: Tx and Rx operations
 *
 * Return: None
 */
static inline void dp_vdev_register_rx_eapol(struct dp_vdev *vdev,
					     struct ol_txrx_ops *txrx_ops)
{
	vdev->osif_rx_eapol = txrx_ops->rx.rx_eapol;
}
#else
static inline void dp_vdev_init_rx_eapol(struct dp_vdev *vdev)
{
}

static inline void dp_vdev_register_rx_eapol(struct dp_vdev *vdev,
					     struct ol_txrx_ops *txrx_ops)
{
}
#endif

#ifdef WLAN_FEATURE_11BE_MLO
static inline void dp_vdev_save_mld_addr(struct dp_vdev *vdev,
					 struct cdp_vdev_info *vdev_info)
{
	if (vdev_info->mld_mac_addr)
		qdf_mem_copy(&vdev->mld_mac_addr.raw[0],
			     vdev_info->mld_mac_addr, QDF_MAC_ADDR_SIZE);
}
#else
static inline void dp_vdev_save_mld_addr(struct dp_vdev *vdev,
					 struct cdp_vdev_info *vdev_info)
{

}
#endif

/*
* dp_vdev_attach_wifi3() - attach txrx vdev
* @txrx_pdev: Datapath PDEV handle
* @pdev_id: PDEV ID for vdev creation
* @vdev_info: parameters used for vdev creation
*
* Return: status
*/
static QDF_STATUS dp_vdev_attach_wifi3(struct cdp_soc_t *cdp_soc,
				       uint8_t pdev_id,
				       struct cdp_vdev_info *vdev_info)
{
	int i = 0;
	qdf_size_t vdev_context_size;
	struct dp_soc *soc = (struct dp_soc *)cdp_soc;
	struct dp_pdev *pdev =
		dp_get_pdev_from_soc_pdev_id_wifi3((struct dp_soc *)soc,
						   pdev_id);
	struct dp_vdev *vdev;
	uint8_t *vdev_mac_addr = vdev_info->vdev_mac_addr;
	uint8_t vdev_id = vdev_info->vdev_id;
	enum wlan_op_mode op_mode = vdev_info->op_mode;
	enum wlan_op_subtype subtype = vdev_info->subtype;
	uint8_t vdev_stats_id = vdev_info->vdev_stats_id;

	vdev_context_size =
		soc->arch_ops.txrx_get_context_size(DP_CONTEXT_TYPE_VDEV);
	vdev = qdf_mem_malloc(vdev_context_size);

	if (!pdev) {
		dp_init_err("%pK: DP PDEV is Null for pdev id %d",
			    cdp_soc, pdev_id);
		qdf_mem_free(vdev);
		goto fail0;
	}

	if (!vdev) {
		dp_init_err("%pK: DP VDEV memory allocation failed",
			    cdp_soc);
		goto fail0;
	}

	wlan_minidump_log(vdev, sizeof(*vdev), soc->ctrl_psoc,
			  WLAN_MD_DP_VDEV, "dp_vdev");

	vdev->pdev = pdev;
	vdev->vdev_id = vdev_id;
	vdev->vdev_stats_id = vdev_stats_id;
	vdev->opmode = op_mode;
	vdev->subtype = subtype;
	vdev->osdev = soc->osdev;

	vdev->osif_rx = NULL;
	vdev->osif_rsim_rx_decap = NULL;
	vdev->osif_get_key = NULL;
	vdev->osif_tx_free_ext = NULL;
	vdev->osif_vdev = NULL;

	vdev->delete.pending = 0;
	vdev->safemode = 0;
	vdev->drop_unenc = 1;
	vdev->sec_type = cdp_sec_type_none;
	vdev->multipass_en = false;
	dp_vdev_init_rx_eapol(vdev);
	qdf_atomic_init(&vdev->ref_cnt);
	for (i = 0; i < DP_MOD_ID_MAX; i++)
		qdf_atomic_init(&vdev->mod_refs[i]);

	/* Take one reference for create*/
	qdf_atomic_inc(&vdev->ref_cnt);
	qdf_atomic_inc(&vdev->mod_refs[DP_MOD_ID_CONFIG]);
	vdev->num_peers = 0;
#ifdef notyet
	vdev->filters_num = 0;
#endif
	vdev->lmac_id = pdev->lmac_id;

	qdf_mem_copy(&vdev->mac_addr.raw[0], vdev_mac_addr, QDF_MAC_ADDR_SIZE);

	dp_vdev_save_mld_addr(vdev, vdev_info);

	/* TODO: Initialize default HTT meta data that will be used in
	 * TCL descriptors for packets transmitted from this VDEV
	 */

	qdf_spinlock_create(&vdev->peer_list_lock);
	TAILQ_INIT(&vdev->peer_list);
	dp_peer_multipass_list_init(vdev);
	if ((soc->intr_mode == DP_INTR_POLL) &&
	    wlan_cfg_get_num_contexts(soc->wlan_cfg_ctx) != 0) {
		if ((pdev->vdev_count == 0) ||
		    (wlan_op_mode_monitor == vdev->opmode))
			qdf_timer_mod(&soc->int_timer, DP_INTR_POLL_TIMER_MS);
	} else if (dp_soc_get_con_mode(soc) == QDF_GLOBAL_MISSION_MODE &&
		   soc->intr_mode == DP_INTR_MSI &&
		   wlan_op_mode_monitor == vdev->opmode) {
		/* Timer to reap status ring in mission mode */
		dp_monitor_vdev_timer_start(soc);
	}

	dp_vdev_id_map_tbl_add(soc, vdev, vdev_id);

	if (wlan_op_mode_monitor == vdev->opmode) {
		if (dp_monitor_vdev_attach(vdev) == QDF_STATUS_SUCCESS) {
			dp_monitor_pdev_set_mon_vdev(vdev);
			dp_monitor_vdev_set_monitor_mode_buf_rings(pdev);
			return QDF_STATUS_SUCCESS;
		}
		return QDF_STATUS_E_FAILURE;
	}

	vdev->tx_encap_type = wlan_cfg_pkt_type(soc->wlan_cfg_ctx);
	vdev->rx_decap_type = wlan_cfg_pkt_type(soc->wlan_cfg_ctx);
	vdev->dscp_tid_map_id = 0;
	vdev->mcast_enhancement_en = 0;
	vdev->igmp_mcast_enhanc_en = 0;
	vdev->raw_mode_war = wlan_cfg_get_raw_mode_war(soc->wlan_cfg_ctx);
	vdev->prev_tx_enq_tstamp = 0;
	vdev->prev_rx_deliver_tstamp = 0;
	vdev->skip_sw_tid_classification = DP_TX_HW_DSCP_TID_MAP_VALID;

	dp_vdev_pdev_list_add(soc, pdev, vdev);
	pdev->vdev_count++;

	if (wlan_op_mode_sta != vdev->opmode &&
	    wlan_op_mode_ndi != vdev->opmode)
		vdev->ap_bridge_enabled = true;
	else
		vdev->ap_bridge_enabled = false;
	dp_init_info("%pK: wlan_cfg_ap_bridge_enabled %d",
		     cdp_soc, vdev->ap_bridge_enabled);

	dp_tx_vdev_attach(vdev);

	dp_monitor_vdev_attach(vdev);
	if (!pdev->is_lro_hash_configured) {
		if (QDF_IS_STATUS_SUCCESS(dp_lro_hash_setup(soc, pdev)))
			pdev->is_lro_hash_configured = true;
		else
			dp_err("LRO hash setup failure!");
	}

	dp_info("Created vdev %pK ("QDF_MAC_ADDR_FMT")", vdev,
		QDF_MAC_ADDR_REF(vdev->mac_addr.raw));
	DP_STATS_INIT(vdev);

	if (QDF_IS_STATUS_ERROR(soc->arch_ops.txrx_vdev_attach(soc, vdev)))
		goto fail0;

	if (wlan_op_mode_sta == vdev->opmode)
		dp_peer_create_wifi3((struct cdp_soc_t *)soc, vdev_id,
				     vdev->mac_addr.raw, CDP_LINK_PEER_TYPE);
	return QDF_STATUS_SUCCESS;

fail0:
	return QDF_STATUS_E_FAILURE;
}

#ifndef QCA_HOST_MODE_WIFI_DISABLED
/**
 * dp_vdev_register_tx_handler() - Register Tx handler
 * @vdev: struct dp_vdev *
 * @soc: struct dp_soc *
 * @txrx_ops: struct ol_txrx_ops *
 */
static inline void dp_vdev_register_tx_handler(struct dp_vdev *vdev,
					       struct dp_soc *soc,
					       struct ol_txrx_ops *txrx_ops)
{
	/* Enable vdev_id check only for ap, if flag is enabled */
	if (vdev->mesh_vdev)
		txrx_ops->tx.tx = dp_tx_send_mesh;
	else if ((wlan_cfg_is_tx_per_pkt_vdev_id_check_enabled(soc->wlan_cfg_ctx)) &&
		 (vdev->opmode == wlan_op_mode_ap))
		txrx_ops->tx.tx = dp_tx_send_vdev_id_check;
	else
		txrx_ops->tx.tx = dp_tx_send;

	/* Avoid check in regular exception Path */
	if ((wlan_cfg_is_tx_per_pkt_vdev_id_check_enabled(soc->wlan_cfg_ctx)) &&
	    (vdev->opmode == wlan_op_mode_ap))
		txrx_ops->tx.tx_exception = dp_tx_send_exception_vdev_id_check;
	else
		txrx_ops->tx.tx_exception = dp_tx_send_exception;

	dp_info("Configure tx_vdev_id_chk_handler Feature Flag: %d and mode:%d for vdev_id:%d",
		wlan_cfg_is_tx_per_pkt_vdev_id_check_enabled(soc->wlan_cfg_ctx),
		vdev->opmode, vdev->vdev_id);
}
#else /* QCA_HOST_MODE_WIFI_DISABLED */
static inline void dp_vdev_register_tx_handler(struct dp_vdev *vdev,
					       struct dp_soc *soc,
					       struct ol_txrx_ops *txrx_ops)
{
}
#endif /* QCA_HOST_MODE_WIFI_DISABLED */

/**
 * dp_vdev_register_wifi3() - Register VDEV operations from osif layer
 * @soc: Datapath soc handle
 * @vdev_id: id of Datapath VDEV handle
 * @osif_vdev: OSIF vdev handle
 * @txrx_ops: Tx and Rx operations
 *
 * Return: DP VDEV handle on success, NULL on failure
 */
static QDF_STATUS dp_vdev_register_wifi3(struct cdp_soc_t *soc_hdl,
					 uint8_t vdev_id,
					 ol_osif_vdev_handle osif_vdev,
					 struct ol_txrx_ops *txrx_ops)
{
	struct dp_soc *soc = cdp_soc_t_to_dp_soc(soc_hdl);
	struct dp_vdev *vdev =	dp_vdev_get_ref_by_id(soc, vdev_id,
						      DP_MOD_ID_CDP);

	if (!vdev)
		return QDF_STATUS_E_FAILURE;

	vdev->osif_vdev = osif_vdev;
	vdev->osif_rx = txrx_ops->rx.rx;
	vdev->osif_rx_stack = txrx_ops->rx.rx_stack;
	vdev->osif_rx_flush = txrx_ops->rx.rx_flush;
	vdev->osif_gro_flush = txrx_ops->rx.rx_gro_flush;
	vdev->osif_rsim_rx_decap = txrx_ops->rx.rsim_rx_decap;
	vdev->osif_fisa_rx = txrx_ops->rx.osif_fisa_rx;
	vdev->osif_fisa_flush = txrx_ops->rx.osif_fisa_flush;
	vdev->osif_get_key = txrx_ops->get_key;
	dp_monitor_vdev_register_osif(vdev, txrx_ops);
	vdev->osif_tx_free_ext = txrx_ops->tx.tx_free_ext;
	vdev->tx_comp = txrx_ops->tx.tx_comp;
	vdev->stats_cb = txrx_ops->rx.stats_rx;
#ifdef notyet
#if ATH_SUPPORT_WAPI
	vdev->osif_check_wai = txrx_ops->rx.wai_check;
#endif
#endif
#ifdef UMAC_SUPPORT_PROXY_ARP
	vdev->osif_proxy_arp = txrx_ops->proxy_arp;
#endif
	vdev->me_convert = txrx_ops->me_convert;
	vdev->get_tsf_time = txrx_ops->get_tsf_time;

	dp_vdev_register_rx_eapol(vdev, txrx_ops);

	dp_vdev_register_tx_handler(vdev, soc, txrx_ops);

	dp_init_info("%pK: DP Vdev Register success", soc);

	dp_vdev_unref_delete(soc, vdev, DP_MOD_ID_CDP);
	return QDF_STATUS_SUCCESS;
}

void dp_peer_delete(struct dp_soc *soc,
		    struct dp_peer *peer,
		    void *arg)
{
	if (!peer->valid)
		return;

	dp_peer_delete_wifi3((struct cdp_soc_t *)soc,
			     peer->vdev->vdev_id,
			     peer->mac_addr.raw, 0);
}

/**
 * dp_vdev_flush_peers() - Forcibily Flush peers of vdev
 * @vdev: Datapath VDEV handle
 * @unmap_only: Flag to indicate "only unmap"
 *
 * Return: void
 */
static void dp_vdev_flush_peers(struct cdp_vdev *vdev_handle, bool unmap_only)
{
	struct dp_vdev *vdev = (struct dp_vdev *)vdev_handle;
	struct dp_pdev *pdev = vdev->pdev;
	struct dp_soc *soc = pdev->soc;
	struct dp_peer *peer;
	uint32_t i = 0;


	if (!unmap_only)
		dp_vdev_iterate_peer_lock_safe(vdev, dp_peer_delete, NULL,
					       DP_MOD_ID_CDP);

	for (i = 0; i < soc->max_peer_id ; i++) {
		peer = __dp_peer_get_ref_by_id(soc, i, DP_MOD_ID_CDP);

		if (!peer)
			continue;

		if (peer->vdev != vdev) {
			dp_peer_unref_delete(peer, DP_MOD_ID_CDP);
			continue;
		}

		dp_info("peer: "QDF_MAC_ADDR_FMT" is getting unmap",
			QDF_MAC_ADDR_REF(peer->mac_addr.raw));

		dp_rx_peer_unmap_handler(soc, i,
					 vdev->vdev_id,
					 peer->mac_addr.raw, 0,
					 DP_PEER_WDS_COUNT_INVALID);
		SET_PEER_REF_CNT_ONE(peer);
		dp_peer_unref_delete(peer, DP_MOD_ID_CDP);
	}

}

#ifdef QCA_VDEV_STATS_HW_OFFLOAD_SUPPORT
/*
 * dp_txrx_alloc_vdev_stats_id()- Allocate vdev_stats_id
 * @soc_hdl: Datapath soc handle
 * @vdev_stats_id: Address of vdev_stats_id
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS dp_txrx_alloc_vdev_stats_id(struct cdp_soc_t *soc_hdl,
					      uint8_t *vdev_stats_id)
{
	struct dp_soc *soc = cdp_soc_t_to_dp_soc(soc_hdl);
	uint8_t id = 0;

	if (!wlan_cfg_get_vdev_stats_hw_offload_config(soc->wlan_cfg_ctx)) {
		*vdev_stats_id = CDP_INVALID_VDEV_STATS_ID;
		return QDF_STATUS_E_FAILURE;
	}

	while (id < CDP_MAX_VDEV_STATS_ID) {
		if (!qdf_atomic_test_and_set_bit(id, &soc->vdev_stats_id_map)) {
			*vdev_stats_id = id;
			return QDF_STATUS_SUCCESS;
		}
		id++;
	}

	*vdev_stats_id = CDP_INVALID_VDEV_STATS_ID;
	return QDF_STATUS_E_FAILURE;
}

/*
 * dp_txrx_reset_vdev_stats_id() - Reset vdev_stats_id in dp_soc
 * @soc_hdl: Datapath soc handle
 * @vdev_stats_id: vdev_stats_id to reset in dp_soc
 *
 * Return: none
 */
static void dp_txrx_reset_vdev_stats_id(struct cdp_soc_t *soc_hdl,
					uint8_t vdev_stats_id)
{
	struct dp_soc *soc = cdp_soc_t_to_dp_soc(soc_hdl);

	if ((!wlan_cfg_get_vdev_stats_hw_offload_config(soc->wlan_cfg_ctx)) ||
	    (vdev_stats_id >= CDP_MAX_VDEV_STATS_ID))
		return;

	qdf_atomic_clear_bit(vdev_stats_id, &soc->vdev_stats_id_map);
}
#else
static void dp_txrx_reset_vdev_stats_id(struct cdp_soc_t *soc,
					uint8_t vdev_stats_id)
{}
#endif
/*
 * dp_vdev_detach_wifi3() - Detach txrx vdev
 * @cdp_soc: Datapath soc handle
 * @vdev_id: VDEV Id
 * @callback: Callback OL_IF on completion of detach
 * @cb_context:	Callback context
 *
 */
static QDF_STATUS dp_vdev_detach_wifi3(struct cdp_soc_t *cdp_soc,
				       uint8_t vdev_id,
				       ol_txrx_vdev_delete_cb callback,
				       void *cb_context)
{
	struct dp_soc *soc = (struct dp_soc *)cdp_soc;
	struct dp_pdev *pdev;
	struct dp_neighbour_peer *peer = NULL;
	struct dp_peer *vap_self_peer = NULL;
	struct dp_vdev *vdev = dp_vdev_get_ref_by_id(soc, vdev_id,
						     DP_MOD_ID_CDP);

	if (!vdev)
		return QDF_STATUS_E_FAILURE;

	soc->arch_ops.txrx_vdev_detach(soc, vdev);

	pdev = vdev->pdev;

	vap_self_peer = dp_sta_vdev_self_peer_ref_n_get(soc, vdev,
							DP_MOD_ID_CONFIG);
	if (vap_self_peer) {
		qdf_spin_lock_bh(&soc->ast_lock);
		if (vap_self_peer->self_ast_entry) {
			dp_peer_del_ast(soc, vap_self_peer->self_ast_entry);
			vap_self_peer->self_ast_entry = NULL;
		}
		qdf_spin_unlock_bh(&soc->ast_lock);

		dp_peer_delete_wifi3((struct cdp_soc_t *)soc, vdev->vdev_id,
				     vap_self_peer->mac_addr.raw, 0);
		dp_peer_unref_delete(vap_self_peer, DP_MOD_ID_CONFIG);
	}

	/*
	 * If Target is hung, flush all peers before detaching vdev
	 * this will free all references held due to missing
	 * unmap commands from Target
	 */
	if (!hif_is_target_ready(HIF_GET_SOFTC(soc->hif_handle)))
		dp_vdev_flush_peers((struct cdp_vdev *)vdev, false);
	else if (hif_get_target_status(soc->hif_handle) == TARGET_STATUS_RESET)
		dp_vdev_flush_peers((struct cdp_vdev *)vdev, true);

	/* indicate that the vdev needs to be deleted */
	vdev->delete.pending = 1;
	dp_rx_vdev_detach(vdev);
	/*
	 * move it after dp_rx_vdev_detach(),
	 * as the call back done in dp_rx_vdev_detach()
	 * still need to get vdev pointer by vdev_id.
	 */
	dp_vdev_id_map_tbl_remove(soc, vdev);

	dp_monitor_neighbour_peer_list_remove(pdev, vdev, peer);

	dp_txrx_reset_vdev_stats_id(cdp_soc, vdev->vdev_stats_id);

	dp_tx_vdev_multipass_deinit(vdev);

	if (vdev->vdev_dp_ext_handle) {
		qdf_mem_free(vdev->vdev_dp_ext_handle);
		vdev->vdev_dp_ext_handle = NULL;
	}
	vdev->delete.callback = callback;
	vdev->delete.context = cb_context;

	if (vdev->opmode != wlan_op_mode_monitor)
		dp_vdev_pdev_list_remove(soc, pdev, vdev);

	pdev->vdev_count--;
	/* release reference taken above for find */
	dp_vdev_unref_delete(soc, vdev, DP_MOD_ID_CDP);

	qdf_spin_lock_bh(&soc->inactive_vdev_list_lock);
	TAILQ_INSERT_TAIL(&soc->inactive_vdev_list, vdev, inactive_list_elem);
	qdf_spin_unlock_bh(&soc->inactive_vdev_list_lock);

	/* release reference taken at dp_vdev_create */
	dp_vdev_unref_delete(soc, vdev, DP_MOD_ID_CONFIG);

	return QDF_STATUS_SUCCESS;
}

#ifdef WLAN_FEATURE_11BE_MLO
/**
 * is_dp_peer_can_reuse() - check if the dp_peer match condition to be reused
 * @vdev: Target DP vdev handle
 * @peer: DP peer handle to be checked
 * @peer_mac_addr: Target peer mac address
 * @peer_type: Target peer type
 *
 * Return: true - if match, false - not match
 */
static inline
bool is_dp_peer_can_reuse(struct dp_vdev *vdev,
			  struct dp_peer *peer,
			  uint8_t *peer_mac_addr,
			  enum cdp_peer_type peer_type)
{
	if (peer->bss_peer && (peer->vdev == vdev) &&
	    (peer->peer_type == peer_type) &&
	    (qdf_mem_cmp(peer_mac_addr, peer->mac_addr.raw,
			 QDF_MAC_ADDR_SIZE) == 0))
		return true;

	return false;
}
#else
static inline
bool is_dp_peer_can_reuse(struct dp_vdev *vdev,
			  struct dp_peer *peer,
			  uint8_t *peer_mac_addr,
			  enum cdp_peer_type peer_type)
{
	if (peer->bss_peer && (peer->vdev == vdev) &&
	    (qdf_mem_cmp(peer_mac_addr, peer->mac_addr.raw,
			 QDF_MAC_ADDR_SIZE) == 0))
		return true;

	return false;
}
#endif

static inline struct dp_peer *dp_peer_can_reuse(struct dp_vdev *vdev,
						uint8_t *peer_mac_addr,
						enum cdp_peer_type peer_type)
{
	struct dp_peer *peer;
	struct dp_soc *soc = vdev->pdev->soc;

	qdf_spin_lock_bh(&soc->inactive_peer_list_lock);
	TAILQ_FOREACH(peer, &soc->inactive_peer_list,
		      inactive_list_elem) {

		/* reuse bss peer only when vdev matches*/
		if (is_dp_peer_can_reuse(vdev, peer,
					 peer_mac_addr, peer_type)) {
			/* increment ref count for cdp_peer_create*/
			if (dp_peer_get_ref(soc, peer, DP_MOD_ID_CONFIG) ==
						QDF_STATUS_SUCCESS) {
				TAILQ_REMOVE(&soc->inactive_peer_list, peer,
					     inactive_list_elem);
				qdf_spin_unlock_bh
					(&soc->inactive_peer_list_lock);
				return peer;
			}
		}
	}

	qdf_spin_unlock_bh(&soc->inactive_peer_list_lock);
	return NULL;
}

#ifdef FEATURE_AST
static inline void dp_peer_ast_handle_roam_del(struct dp_soc *soc,
					       struct dp_pdev *pdev,
					       uint8_t *peer_mac_addr)
{
	struct dp_ast_entry *ast_entry;

	if (soc->ast_offload_support)
		return;

	qdf_spin_lock_bh(&soc->ast_lock);
	if (soc->ast_override_support)
		ast_entry = dp_peer_ast_hash_find_by_pdevid(soc, peer_mac_addr,
							    pdev->pdev_id);
	else
		ast_entry = dp_peer_ast_hash_find_soc(soc, peer_mac_addr);

	if (ast_entry && ast_entry->next_hop && !ast_entry->delete_in_progress)
		dp_peer_del_ast(soc, ast_entry);

	qdf_spin_unlock_bh(&soc->ast_lock);
}
#endif

#ifdef PEER_CACHE_RX_PKTS
static inline void dp_peer_rx_bufq_resources_init(struct dp_peer *peer)
{
	qdf_spinlock_create(&peer->bufq_info.bufq_lock);
	peer->bufq_info.thresh = DP_RX_CACHED_BUFQ_THRESH;
	qdf_list_create(&peer->bufq_info.cached_bufq, DP_RX_CACHED_BUFQ_THRESH);
}
#else
static inline void dp_peer_rx_bufq_resources_init(struct dp_peer *peer)
{
}
#endif

#ifdef QCA_VDEV_STATS_HW_OFFLOAD_SUPPORT
/*
 * dp_peer_hw_txrx_stats_init() - Initialize hw_txrx_stats_en in dp_peer
 * @soc: Datapath soc handle
 * @peer: Datapath peer handle
 *
 * Return: none
 */
static inline
void dp_peer_hw_txrx_stats_init(struct dp_soc *soc, struct dp_peer *peer)
{
	peer->hw_txrx_stats_en =
		wlan_cfg_get_vdev_stats_hw_offload_config(soc->wlan_cfg_ctx);
}
#else
static inline
void dp_peer_hw_txrx_stats_init(struct dp_soc *soc, struct dp_peer *peer)
{
	peer->hw_txrx_stats_en = 0;
}
#endif
/*
 * dp_peer_create_wifi3() - attach txrx peer
 * @soc_hdl: Datapath soc handle
 * @vdev_id: id of vdev
 * @peer_mac_addr: Peer MAC address
 * @peer_type: link or MLD peer type
 *
 * Return: 0 on success, -1 on failure
 */
static QDF_STATUS
dp_peer_create_wifi3(struct cdp_soc_t *soc_hdl, uint8_t vdev_id,
		     uint8_t *peer_mac_addr, enum cdp_peer_type peer_type)
{
	struct dp_peer *peer;
	int i;
	struct dp_soc *soc = (struct dp_soc *)soc_hdl;
	struct dp_pdev *pdev;
	struct cdp_peer_cookie peer_cookie;
	enum cdp_txrx_ast_entry_type ast_type = CDP_TXRX_AST_TYPE_STATIC;
	struct dp_vdev *vdev = NULL;

	if (!peer_mac_addr)
		return QDF_STATUS_E_FAILURE;

	vdev = dp_vdev_get_ref_by_id(soc, vdev_id, DP_MOD_ID_CDP);

	if (!vdev)
		return QDF_STATUS_E_FAILURE;

	pdev = vdev->pdev;
	soc = pdev->soc;

	/*
	 * If a peer entry with given MAC address already exists,
	 * reuse the peer and reset the state of peer.
	 */
	peer = dp_peer_can_reuse(vdev, peer_mac_addr, peer_type);

	if (peer) {
		qdf_atomic_init(&peer->is_default_route_set);
		dp_peer_cleanup(vdev, peer);

		dp_peer_vdev_list_add(soc, vdev, peer);
		dp_peer_find_hash_add(soc, peer);

		dp_peer_rx_tids_create(peer);
		if (IS_MLO_DP_MLD_PEER(peer))
			dp_mld_peer_init_link_peers_info(peer);

		qdf_spin_lock_bh(&soc->ast_lock);
		dp_peer_delete_ast_entries(soc, peer);
		qdf_spin_unlock_bh(&soc->ast_lock);

		if ((vdev->opmode == wlan_op_mode_sta) &&
		    !qdf_mem_cmp(peer_mac_addr, &vdev->mac_addr.raw[0],
		     QDF_MAC_ADDR_SIZE)) {
			ast_type = CDP_TXRX_AST_TYPE_SELF;
		}
		dp_peer_add_ast(soc, peer, peer_mac_addr, ast_type, 0);

		peer->valid = 1;
		peer->is_tdls_peer = false;
		dp_local_peer_id_alloc(pdev, peer);

		qdf_spinlock_create(&peer->peer_info_lock);
		dp_peer_rx_bufq_resources_init(peer);

		DP_STATS_INIT(peer);
		DP_STATS_UPD(peer, rx.avg_snr, CDP_INVALID_SNR);

		/*
		 * In tx_monitor mode, filter may be set for unassociated peer
		 * when unassociated peer get associated peer need to
		 * update tx_cap_enabled flag to support peer filter.
		 */
		dp_monitor_peer_tx_capture_filter_check(pdev, peer);

		dp_set_peer_isolation(peer, false);

		dp_wds_ext_peer_init(peer);
		dp_peer_hw_txrx_stats_init(soc, peer);
		dp_peer_update_state(soc, peer, DP_PEER_STATE_INIT);

		dp_vdev_unref_delete(soc, vdev, DP_MOD_ID_CDP);
		return QDF_STATUS_SUCCESS;
	} else {
		/*
		 * When a STA roams from RPTR AP to ROOT AP and vice versa, we
		 * need to remove the AST entry which was earlier added as a WDS
		 * entry.
		 * If an AST entry exists, but no peer entry exists with a given
		 * MAC addresses, we could deduce it as a WDS entry
		 */
		dp_peer_ast_handle_roam_del(soc, pdev, peer_mac_addr);
	}

#ifdef notyet
	peer = (struct dp_peer *)qdf_mempool_alloc(soc->osdev,
		soc->mempool_ol_ath_peer);
#else
	peer = (struct dp_peer *)qdf_mem_malloc(sizeof(*peer));
#endif
	wlan_minidump_log(peer,
			  sizeof(*peer),
			  soc->ctrl_psoc,
			  WLAN_MD_DP_PEER, "dp_peer");
	if (!peer) {
		dp_vdev_unref_delete(soc, vdev, DP_MOD_ID_CDP);
		return QDF_STATUS_E_FAILURE; /* failure */
	}

	qdf_mem_zero(peer, sizeof(struct dp_peer));

	TAILQ_INIT(&peer->ast_entry_list);

	/* store provided params */
	peer->vdev = vdev;
	DP_PEER_SET_TYPE(peer, peer_type);
	/* get the vdev reference for new peer */
	dp_vdev_get_ref(soc, vdev, DP_MOD_ID_CHILD);

	if ((vdev->opmode == wlan_op_mode_sta) &&
	    !qdf_mem_cmp(peer_mac_addr, &vdev->mac_addr.raw[0],
			 QDF_MAC_ADDR_SIZE)) {
		ast_type = CDP_TXRX_AST_TYPE_SELF;
	}
	qdf_spinlock_create(&peer->peer_state_lock);
	dp_peer_add_ast(soc, peer, peer_mac_addr, ast_type, 0);
	qdf_spinlock_create(&peer->peer_info_lock);
	dp_wds_ext_peer_init(peer);
	dp_peer_hw_txrx_stats_init(soc, peer);
	dp_peer_rx_bufq_resources_init(peer);

	qdf_mem_copy(
		&peer->mac_addr.raw[0], peer_mac_addr, QDF_MAC_ADDR_SIZE);

	/* initialize the peer_id */
	peer->peer_id = HTT_INVALID_PEER;

	/* reset the ast index to flowid table */
	dp_peer_reset_flowq_map(peer);

	qdf_atomic_init(&peer->ref_cnt);

	for (i = 0; i < DP_MOD_ID_MAX; i++)
		qdf_atomic_init(&peer->mod_refs[i]);

	/* keep one reference for attach */
	qdf_atomic_inc(&peer->ref_cnt);
	qdf_atomic_inc(&peer->mod_refs[DP_MOD_ID_CONFIG]);

	dp_peer_vdev_list_add(soc, vdev, peer);

	/* TODO: See if hash based search is required */
	dp_peer_find_hash_add(soc, peer);

	/* Initialize the peer state */
	peer->state = OL_TXRX_PEER_STATE_DISC;

	dp_info("vdev %pK created peer %pK ("QDF_MAC_ADDR_FMT") ref_cnt: %d",
		vdev, peer, QDF_MAC_ADDR_REF(peer->mac_addr.raw),
		qdf_atomic_read(&peer->ref_cnt));
	/*
	 * For every peer MAp message search and set if bss_peer
	 */
	if (qdf_mem_cmp(peer->mac_addr.raw, vdev->mac_addr.raw,
			QDF_MAC_ADDR_SIZE) == 0 &&
			(wlan_op_mode_sta != vdev->opmode)) {
		dp_info("vdev bss_peer!!");
		peer->bss_peer = 1;
	}

	if (wlan_op_mode_sta == vdev->opmode &&
	    qdf_mem_cmp(peer->mac_addr.raw, vdev->mac_addr.raw,
			QDF_MAC_ADDR_SIZE) == 0) {
		peer->sta_self_peer = 1;
	}

	dp_peer_rx_tids_create(peer);
	if (IS_MLO_DP_MLD_PEER(peer))
		dp_mld_peer_init_link_peers_info(peer);

	peer->valid = 1;
	dp_local_peer_id_alloc(pdev, peer);
	DP_STATS_INIT(peer);
	DP_STATS_UPD(peer, rx.avg_snr, CDP_INVALID_SNR);

	qdf_mem_copy(peer_cookie.mac_addr, peer->mac_addr.raw,
		     QDF_MAC_ADDR_SIZE);
	peer_cookie.ctx = NULL;
	peer_cookie.pdev_id = pdev->pdev_id;
	peer_cookie.cookie = pdev->next_peer_cookie++;
#if defined(FEATURE_PERPKT_INFO) && WDI_EVENT_ENABLE
	dp_wdi_event_handler(WDI_EVENT_PEER_CREATE, pdev->soc,
			     (void *)&peer_cookie,
			     peer->peer_id, WDI_NO_VAL, pdev->pdev_id);
#endif
	if (soc->rdkstats_enabled) {
		if (!peer_cookie.ctx) {
			pdev->next_peer_cookie--;
			qdf_err("Failed to initialize peer rate stats");
		} else {
			peer->rdkstats_ctx = (struct cdp_peer_rate_stats_ctx *)
						peer_cookie.ctx;
		}
	}

	/*
	 * Allocate peer extended stats context. Fall through in
	 * case of failure as its not an implicit requirement to have
	 * this object for regular statistics updates.
	 */
	if (dp_peer_ext_stats_ctx_alloc(soc, peer) !=
			QDF_STATUS_SUCCESS)
		dp_warn("peer ext_stats ctx alloc failed");

	if (dp_monitor_peer_attach(soc, peer) !=
	    QDF_STATUS_SUCCESS)
		dp_warn("peer monitor ctx alloc failed");

	dp_set_peer_isolation(peer, false);

	dp_peer_update_state(soc, peer, DP_PEER_STATE_INIT);

	dp_vdev_unref_delete(soc, vdev, DP_MOD_ID_CDP);

	return QDF_STATUS_SUCCESS;
}

#ifdef WLAN_FEATURE_11BE_MLO
QDF_STATUS dp_peer_mlo_setup(
			struct dp_soc *soc,
			struct dp_peer *peer,
			uint8_t vdev_id,
			struct cdp_peer_setup_info *setup_info)
{
	struct dp_peer *mld_peer = NULL;

	/* Non-MLO connection, do nothing */
	if (!setup_info || !setup_info->mld_peer_mac)
		return QDF_STATUS_SUCCESS;

	/* To do: remove this check if link/mld peer mac_addr allow to same */
	if (!qdf_mem_cmp(setup_info->mld_peer_mac, peer->mac_addr.raw,
			 QDF_MAC_ADDR_SIZE)) {
		dp_peer_err("Same mac addres for link/mld peer");
		return QDF_STATUS_E_FAILURE;
	}

	/* if this is the first link peer */
	if (setup_info->is_first_link)
		/* create MLD peer */
		dp_peer_create_wifi3((struct cdp_soc_t *)soc,
				     vdev_id,
				     setup_info->mld_peer_mac,
				     CDP_MLD_PEER_TYPE);

	peer->first_link = setup_info->is_first_link;
	peer->primary_link = setup_info->is_primary_link;
	mld_peer = dp_peer_find_hash_find(soc,
					  setup_info->mld_peer_mac,
					  0, DP_VDEV_ALL, DP_MOD_ID_CDP);
	if (mld_peer) {
		if (setup_info->is_first_link) {
			/* assign rx_tid to mld peer */
			mld_peer->rx_tid = peer->rx_tid;
			/* no cdp_peer_setup for MLD peer,
			 * set it for addba processing
			 */
			qdf_atomic_set(&mld_peer->is_default_route_set, 1);
		} else {
			/* free link peer origial rx_tids mem */
			dp_peer_rx_tids_destroy(peer);
			/* assign mld peer rx_tid to link peer */
			peer->rx_tid = mld_peer->rx_tid;
		}

		if (setup_info->is_primary_link &&
		    !setup_info->is_first_link) {
			/*
			 * if first link is not the primary link,
			 * then need to change mld_peer->vdev as
			 * primary link dp_vdev is not same one
			 * during mld peer creation.
			 */

			/* relase the ref to original dp_vdev */
			dp_vdev_unref_delete(soc, mld_peer->vdev,
					     DP_MOD_ID_CHILD);
			/*
			 * get the ref to new dp_vdev,
			 * increase dp_vdev ref_cnt
			 */
			mld_peer->vdev = dp_vdev_get_ref_by_id(soc, vdev_id,
							       DP_MOD_ID_CHILD);
		}

		/* associate mld and link peer */
		dp_link_peer_add_mld_peer(peer, mld_peer);
		dp_mld_peer_add_link_peer(mld_peer, peer);

		dp_peer_unref_delete(mld_peer, DP_MOD_ID_CDP);
	} else {
		peer->mld_peer = NULL;
		dp_err("mld peer" QDF_MAC_ADDR_FMT "not found!",
		       QDF_MAC_ADDR_REF(setup_info->mld_peer_mac));
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

/*
 * dp_mlo_peer_authorize() - authorize MLO peer
 * @soc: soc handle
 * @peer: pointer to link peer
 *
 * return void
 */
static void dp_mlo_peer_authorize(struct dp_soc *soc,
				  struct dp_peer *peer)
{
	int i;
	struct dp_peer *link_peer = NULL;
	struct dp_peer *mld_peer = peer->mld_peer;
	struct dp_mld_link_peers link_peers_info;

	if (!mld_peer)
		return;

	/* get link peers with reference */
	dp_get_link_peers_ref_from_mld_peer(soc, mld_peer,
					    &link_peers_info,
					    DP_MOD_ID_CDP);

	for (i = 0; i < link_peers_info.num_links; i++) {
		link_peer = link_peers_info.link_peers[i];

		if (!link_peer->authorize) {
			dp_release_link_peers_ref(&link_peers_info,
						  DP_MOD_ID_CDP);
			mld_peer->authorize = false;
			return;
		}
	}

	/* if we are here all link peers are authorized,
	 * authorize ml_peer also
	 */
	mld_peer->authorize = true;

	/* release link peers reference */
	dp_release_link_peers_ref(&link_peers_info, DP_MOD_ID_CDP);
}
#endif

void dp_vdev_get_default_reo_hash(struct dp_vdev *vdev,
				  enum cdp_host_reo_dest_ring *reo_dest,
				  bool *hash_based)
{
	struct dp_soc *soc;
	struct dp_pdev *pdev;

	pdev = vdev->pdev;
	soc = pdev->soc;
	/*
	 * hash based steering is disabled for Radios which are offloaded
	 * to NSS
	 */
	if (!wlan_cfg_get_dp_pdev_nss_enabled(pdev->wlan_cfg_ctx))
		*hash_based = wlan_cfg_is_rx_hash_enabled(soc->wlan_cfg_ctx);

	/*
	 * Below line of code will ensure the proper reo_dest ring is chosen
	 * for cases where toeplitz hash cannot be generated (ex: non TCP/UDP)
	 */
	*reo_dest = pdev->reo_dest;
}

#ifdef IPA_OFFLOAD
/**
 * dp_is_vdev_subtype_p2p() - Check if the subtype for vdev is P2P
 * @vdev: Virtual device
 *
 * Return: true if the vdev is of subtype P2P
 *	   false if the vdev is of any other subtype
 */
static inline bool dp_is_vdev_subtype_p2p(struct dp_vdev *vdev)
{
	if (vdev->subtype == wlan_op_subtype_p2p_device ||
	    vdev->subtype == wlan_op_subtype_p2p_cli ||
	    vdev->subtype == wlan_op_subtype_p2p_go)
		return true;

	return false;
}

/*
 * dp_peer_setup_get_reo_hash() - get reo dest ring and hash values for a peer
 * @vdev: Datapath VDEV handle
 * @reo_dest: pointer to default reo_dest ring for vdev to be populated
 * @hash_based: pointer to hash value (enabled/disabled) to be populated
 *
 * If IPA is enabled in ini, for SAP mode, disable hash based
 * steering, use default reo_dst ring for RX. Use config values for other modes.
 * Return: None
 */
static void dp_peer_setup_get_reo_hash(struct dp_vdev *vdev,
				       struct cdp_peer_setup_info *setup_info,
				       enum cdp_host_reo_dest_ring *reo_dest,
				       bool *hash_based,
				       uint8_t *lmac_peer_id_msb)
{
	struct dp_soc *soc;
	struct dp_pdev *pdev;

	pdev = vdev->pdev;
	soc = pdev->soc;

	dp_vdev_get_default_reo_hash(vdev, reo_dest, hash_based);

	/* For P2P-GO interfaces we do not need to change the REO
	 * configuration even if IPA config is enabled
	 */
	if (dp_is_vdev_subtype_p2p(vdev))
		return;

	/*
	 * If IPA is enabled, disable hash-based flow steering and set
	 * reo_dest_ring_4 as the REO ring to receive packets on.
	 * IPA is configured to reap reo_dest_ring_4.
	 *
	 * Note - REO DST indexes are from 0 - 3, while cdp_host_reo_dest_ring
	 * value enum value is from 1 - 4.
	 * Hence, *reo_dest = IPA_REO_DEST_RING_IDX + 1
	 */
	if (wlan_cfg_is_ipa_enabled(soc->wlan_cfg_ctx)) {
		if (vdev->opmode == wlan_op_mode_ap) {
			*reo_dest = IPA_REO_DEST_RING_IDX + 1;
			*hash_based = 0;
		} else if (vdev->opmode == wlan_op_mode_sta &&
			   dp_ipa_is_mdm_platform()) {
			*reo_dest = IPA_REO_DEST_RING_IDX + 1;
		}
	}
}

#else

/*
 * dp_peer_setup_get_reo_hash() - get reo dest ring and hash values for a peer
 * @vdev: Datapath VDEV handle
 * @reo_dest: pointer to default reo_dest ring for vdev to be populated
 * @hash_based: pointer to hash value (enabled/disabled) to be populated
 *
 * Use system config values for hash based steering.
 * Return: None
 */
static void dp_peer_setup_get_reo_hash(struct dp_vdev *vdev,
				       struct cdp_peer_setup_info *setup_info,
				       enum cdp_host_reo_dest_ring *reo_dest,
				       bool *hash_based,
				       uint8_t *lmac_peer_id_msb)
{
	struct dp_soc *soc = vdev->pdev->soc;

	soc->arch_ops.peer_get_reo_hash(vdev, setup_info, reo_dest, hash_based,
					lmac_peer_id_msb);
}
#endif /* IPA_OFFLOAD */

/*
 * dp_peer_setup_wifi3() - initialize the peer
 * @soc_hdl: soc handle object
 * @vdev_id : vdev_id of vdev object
 * @peer_mac: Peer's mac address
 * @peer_setup_info: peer setup info for MLO
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS
dp_peer_setup_wifi3(struct cdp_soc_t *soc_hdl, uint8_t vdev_id,
		    uint8_t *peer_mac,
		    struct cdp_peer_setup_info *setup_info)
{
	struct dp_soc *soc = (struct dp_soc *)soc_hdl;
	struct dp_pdev *pdev;
	bool hash_based = 0;
	enum cdp_host_reo_dest_ring reo_dest;
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	struct dp_vdev *vdev = NULL;
	struct dp_peer *peer =
			dp_peer_find_hash_find(soc, peer_mac, 0, vdev_id,
					       DP_MOD_ID_CDP);
	enum wlan_op_mode vdev_opmode;
	uint8_t lmac_peer_id_msb = 0;

	if (!peer)
		return QDF_STATUS_E_FAILURE;

	vdev = peer->vdev;
	if (!vdev) {
		status = QDF_STATUS_E_FAILURE;
		goto fail;
	}

	/* save vdev related member in case vdev freed */
	vdev_opmode = vdev->opmode;
	pdev = vdev->pdev;
	dp_peer_setup_get_reo_hash(vdev, setup_info,
				   &reo_dest, &hash_based,
				   &lmac_peer_id_msb);

	dp_info("pdev: %d vdev :%d opmode:%u hash-based-steering:%d default-reo_dest:%u",
		pdev->pdev_id, vdev->vdev_id,
		vdev->opmode, hash_based, reo_dest);

	/*
	 * There are corner cases where the AD1 = AD2 = "VAPs address"
	 * i.e both the devices have same MAC address. In these
	 * cases we want such pkts to be processed in NULL Q handler
	 * which is REO2TCL ring. for this reason we should
	 * not setup reo_queues and default route for bss_peer.
	 */
	dp_monitor_peer_tx_init(pdev, peer);
	if (peer->bss_peer && vdev->opmode == wlan_op_mode_ap) {
		status = QDF_STATUS_E_FAILURE;
		goto fail;
	}

	if (soc->cdp_soc.ol_ops->peer_set_default_routing) {
		/* TODO: Check the destination ring number to be passed to FW */
		soc->cdp_soc.ol_ops->peer_set_default_routing(
				soc->ctrl_psoc,
				peer->vdev->pdev->pdev_id,
				peer->mac_addr.raw,
				peer->vdev->vdev_id, hash_based, reo_dest,
				lmac_peer_id_msb);
	}

	qdf_atomic_set(&peer->is_default_route_set, 1);

	status = dp_peer_mlo_setup(soc, peer, vdev->vdev_id, setup_info);
	if (QDF_IS_STATUS_ERROR(status)) {
		dp_peer_err("peer mlo setup failed");
		qdf_assert_always(0);
	}

	if (vdev_opmode != wlan_op_mode_monitor)
		dp_peer_rx_init(pdev, peer);

	dp_peer_ppdu_delayed_ba_init(peer);

fail:
	dp_peer_unref_delete(peer, DP_MOD_ID_CDP);
	return status;
}

/*
 * dp_cp_peer_del_resp_handler - Handle the peer delete response
 * @soc_hdl: Datapath SOC handle
 * @vdev_id: id of virtual device object
 * @mac_addr: Mac address of the peer
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS dp_cp_peer_del_resp_handler(struct cdp_soc_t *soc_hdl,
					      uint8_t vdev_id,
					      uint8_t *mac_addr)
{
	struct dp_soc *soc = (struct dp_soc *)soc_hdl;
	struct dp_ast_entry  *ast_entry = NULL;
	txrx_ast_free_cb cb = NULL;
	void *cookie;

	if (soc->ast_offload_support)
		return QDF_STATUS_E_INVAL;

	qdf_spin_lock_bh(&soc->ast_lock);

	ast_entry =
		dp_peer_ast_hash_find_by_vdevid(soc, mac_addr,
						vdev_id);

	/* in case of qwrap we have multiple BSS peers
	 * with same mac address
	 *
	 * AST entry for this mac address will be created
	 * only for one peer hence it will be NULL here
	 */
	if ((!ast_entry || !ast_entry->delete_in_progress) ||
	    (ast_entry->peer_id != HTT_INVALID_PEER)) {
		qdf_spin_unlock_bh(&soc->ast_lock);
		return QDF_STATUS_E_FAILURE;
	}

	if (ast_entry->is_mapped)
		soc->ast_table[ast_entry->ast_idx] = NULL;

	DP_STATS_INC(soc, ast.deleted, 1);
	dp_peer_ast_hash_remove(soc, ast_entry);

	cb = ast_entry->callback;
	cookie = ast_entry->cookie;
	ast_entry->callback = NULL;
	ast_entry->cookie = NULL;

	soc->num_ast_entries--;
	qdf_spin_unlock_bh(&soc->ast_lock);

	if (cb) {
		cb(soc->ctrl_psoc,
		   dp_soc_to_cdp_soc(soc),
		   cookie,
		   CDP_TXRX_AST_DELETED);
	}
	qdf_mem_free(ast_entry);

	return QDF_STATUS_SUCCESS;
}

/*
 * dp_set_ba_aging_timeout() - set ba aging timeout per AC
 * @txrx_soc: cdp soc handle
 * @ac: Access category
 * @value: timeout value in millisec
 *
 * Return: void
 */
static void dp_set_ba_aging_timeout(struct cdp_soc_t *txrx_soc,
				    uint8_t ac, uint32_t value)
{
	struct dp_soc *soc = (struct dp_soc *)txrx_soc;

	hal_set_ba_aging_timeout(soc->hal_soc, ac, value);
}

/*
 * dp_get_ba_aging_timeout() - get ba aging timeout per AC
 * @txrx_soc: cdp soc handle
 * @ac: access category
 * @value: timeout value in millisec
 *
 * Return: void
 */
static void dp_get_ba_aging_timeout(struct cdp_soc_t *txrx_soc,
				    uint8_t ac, uint32_t *value)
{
	struct dp_soc *soc = (struct dp_soc *)txrx_soc;

	hal_get_ba_aging_timeout(soc->hal_soc, ac, value);
}

/*
 * dp_set_pdev_reo_dest() - set the reo destination ring for this pdev
 * @txrx_soc: cdp soc handle
 * @pdev_id: id of physical device object
 * @val: reo destination ring index (1 - 4)
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS
dp_set_pdev_reo_dest(struct cdp_soc_t *txrx_soc, uint8_t pdev_id,
		     enum cdp_host_reo_dest_ring val)
{
	struct dp_pdev *pdev =
		dp_get_pdev_from_soc_pdev_id_wifi3((struct dp_soc *)txrx_soc,
						   pdev_id);

	if (pdev) {
		pdev->reo_dest = val;
		return QDF_STATUS_SUCCESS;
	}

	return QDF_STATUS_E_FAILURE;
}

/*
 * dp_get_pdev_reo_dest() - get the reo destination for this pdev
 * @txrx_soc: cdp soc handle
 * @pdev_id: id of physical device object
 *
 * Return: reo destination ring index
 */
static enum cdp_host_reo_dest_ring
dp_get_pdev_reo_dest(struct cdp_soc_t *txrx_soc, uint8_t pdev_id)
{
	struct dp_pdev *pdev =
		dp_get_pdev_from_soc_pdev_id_wifi3((struct dp_soc *)txrx_soc,
						   pdev_id);

	if (pdev)
		return pdev->reo_dest;
	else
		return cdp_host_reo_dest_ring_unknown;
}

#ifdef WLAN_SUPPORT_SCS
/*
 * dp_enable_scs_params - Enable/Disable SCS procedures
 * @soc - Datapath soc handle
 * @peer_mac - STA Mac address
 * @vdev_id - ID of the vdev handle
 * @active - Flag to set SCS active/inactive
 * return type - QDF_STATUS - Success/Invalid
 */
static QDF_STATUS
dp_enable_scs_params(struct cdp_soc_t *soc_hdl, struct qdf_mac_addr
		     *peer_mac,
		     uint8_t vdev_id,
		     bool is_active)
{
	struct dp_peer *peer;
	QDF_STATUS status = QDF_STATUS_E_INVAL;
	struct dp_soc *soc = (struct dp_soc *)soc_hdl;

	peer = dp_peer_find_hash_find(soc, peer_mac->bytes, 0, vdev_id,
				      DP_MOD_ID_CDP);

	if (!peer) {
		dp_err("Peer is NULL!");
		goto fail;
	}

	peer->scs_is_active = is_active;
	status = QDF_STATUS_SUCCESS;

fail:
	if (peer)
		dp_peer_unref_delete(peer, DP_MOD_ID_CDP);
	return status;
}

/*
 * @brief dp_copy_scs_params - SCS Parameters sent by STA
 * is copied from the cdp layer to the dp layer
 * These parameters are then used by the peer
 * for traffic classification.
 *
 * @param peer - peer struct
 * @param scs_params - cdp layer params
 * @idx - SCS_entry index obtained from the
 * node database with a given SCSID
 * @return void
 */
void
dp_copy_scs_params(struct dp_peer *peer,
		   struct cdp_scs_params *scs_params,
		   uint8_t idx)
{
	uint8_t tidx = 0;
	uint8_t tclas_elem;

	peer->scs[idx].scsid = scs_params->scsid;
	peer->scs[idx].access_priority =
		scs_params->access_priority;
	peer->scs[idx].tclas_elements =
		scs_params->tclas_elements;
	peer->scs[idx].tclas_process =
		scs_params->tclas_process;

	tclas_elem = peer->scs[idx].tclas_elements;

	while (tidx < tclas_elem) {
		qdf_mem_copy(&peer->scs[idx].tclas[tidx],
			     &scs_params->tclas[tidx],
			     sizeof(struct cdp_tclas_tuple));
		tidx++;
	}
}

/*
 * @brief dp_record_scs_params() - Copying the SCS params to a
 * peer based database.
 *
 * @soc - Datapath soc handle
 * @peer_mac - STA Mac address
 * @vdev_id - ID of the vdev handle
 * @scs_params - Structure having SCS parameters obtained
 * from handshake
 * @idx - SCS_entry index obtained from the
 * node database with a given SCSID
 * @scs_sessions - Total # of SCS sessions active
 *
 * @details
 * SCS parameters sent by the STA in
 * the SCS Request to the AP. The AP makes a note of these
 * parameters while sending the MSDUs to the STA, to
 * send the downlink traffic with correct User priority.
 *
 * return type - QDF_STATUS - Success/Invalid
 */
static QDF_STATUS
dp_record_scs_params(struct cdp_soc_t *soc_hdl, struct qdf_mac_addr
		     *peer_mac,
		     uint8_t vdev_id,
		     struct cdp_scs_params *scs_params,
		     uint8_t idx,
		     uint8_t scs_sessions)
{
	struct dp_peer *peer;
	QDF_STATUS status = QDF_STATUS_E_INVAL;
	struct dp_soc *soc = (struct dp_soc *)soc_hdl;

	peer = dp_peer_find_hash_find(soc, peer_mac->bytes, 0, vdev_id,
				      DP_MOD_ID_CDP);

	if (!peer) {
		dp_err("Peer is NULL!");
		goto fail;
	}

	if (idx >= IEEE80211_SCS_MAX_NO_OF_ELEM)
		goto fail;

	/* SCS procedure for the peer is activated
	 * as soon as we get this information from
	 * the control path, unless explicitly disabled.
	 */
	peer->scs_is_active = 1;
	dp_copy_scs_params(peer, scs_params, idx);
	status = QDF_STATUS_SUCCESS;
	peer->no_of_scs_sessions = scs_sessions;

fail:
	if (peer)
		dp_peer_unref_delete(peer, DP_MOD_ID_CDP);
	return status;
}
#endif

#ifdef WLAN_SUPPORT_MSCS
/*
 * dp_record_mscs_params - MSCS parameters sent by the STA in
 * the MSCS Request to the AP. The AP makes a note of these
 * parameters while comparing the MSDUs sent by the STA, to
 * send the downlink traffic with correct User priority.
 * @soc - Datapath soc handle
 * @peer_mac - STA Mac address
 * @vdev_id - ID of the vdev handle
 * @mscs_params - Structure having MSCS parameters obtained
 * from handshake
 * @active - Flag to set MSCS active/inactive
 * return type - QDF_STATUS - Success/Invalid
 */
static QDF_STATUS
dp_record_mscs_params(struct cdp_soc_t *soc_hdl, uint8_t *peer_mac,
		      uint8_t vdev_id, struct cdp_mscs_params *mscs_params,
		      bool active)
{
	struct dp_peer *peer;
	QDF_STATUS status = QDF_STATUS_E_INVAL;
	struct dp_soc *soc = (struct dp_soc *)soc_hdl;

	peer = dp_peer_find_hash_find(soc, peer_mac, 0, vdev_id,
				      DP_MOD_ID_CDP);

	if (!peer) {
		dp_err("Peer is NULL!");
		goto fail;
	}
	if (!active) {
		dp_info("MSCS Procedure is terminated");
		peer->mscs_active = active;
		goto fail;
	}

	if (mscs_params->classifier_type == IEEE80211_TCLAS_MASK_CLA_TYPE_4) {
		/* Populate entries inside IPV4 database first */
		peer->mscs_ipv4_parameter.user_priority_bitmap =
			mscs_params->user_pri_bitmap;
		peer->mscs_ipv4_parameter.user_priority_limit =
			mscs_params->user_pri_limit;
		peer->mscs_ipv4_parameter.classifier_mask =
			mscs_params->classifier_mask;

		/* Populate entries inside IPV6 database */
		peer->mscs_ipv6_parameter.user_priority_bitmap =
			mscs_params->user_pri_bitmap;
		peer->mscs_ipv6_parameter.user_priority_limit =
			mscs_params->user_pri_limit;
		peer->mscs_ipv6_parameter.classifier_mask =
			mscs_params->classifier_mask;
		peer->mscs_active = 1;
		dp_info("\n\tMSCS Procedure request based parameters for "QDF_MAC_ADDR_FMT"\n"
			"\tClassifier_type = %d\tUser priority bitmap = %x\n"
			"\tUser priority limit = %x\tClassifier mask = %x",
			QDF_MAC_ADDR_REF(peer_mac),
			mscs_params->classifier_type,
			peer->mscs_ipv4_parameter.user_priority_bitmap,
			peer->mscs_ipv4_parameter.user_priority_limit,
			peer->mscs_ipv4_parameter.classifier_mask);
	}

	status = QDF_STATUS_SUCCESS;
fail:
	if (peer)
		dp_peer_unref_delete(peer, DP_MOD_ID_CDP);
	return status;
}
#endif

/*
 * dp_get_sec_type() - Get the security type
 * @soc: soc handle
 * @vdev_id: id of dp handle
 * @peer_mac: mac of datapath PEER handle
 * @sec_idx:    Security id (mcast, ucast)
 *
 * return sec_type: Security type
 */
static int dp_get_sec_type(struct cdp_soc_t *soc, uint8_t vdev_id,
			   uint8_t *peer_mac, uint8_t sec_idx)
{
	int sec_type = 0;
	struct dp_peer *peer = dp_peer_find_hash_find((struct dp_soc *)soc,
						       peer_mac, 0, vdev_id,
						       DP_MOD_ID_CDP);

	if (!peer) {
		dp_cdp_err("%pK: Peer is NULL!\n", (struct dp_soc *)soc);
		return sec_type;
	}

	sec_type = peer->security[sec_idx].sec_type;

	dp_peer_unref_delete(peer, DP_MOD_ID_CDP);
	return sec_type;
}

/*
 * dp_peer_authorize() - authorize txrx peer
 * @soc: soc handle
 * @vdev_id: id of dp handle
 * @peer_mac: mac of datapath PEER handle
 * @authorize
 *
 */
static QDF_STATUS
dp_peer_authorize(struct cdp_soc_t *soc_hdl, uint8_t vdev_id,
		  uint8_t *peer_mac, uint32_t authorize)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	struct dp_soc *soc = (struct dp_soc *)soc_hdl;
	struct dp_peer *peer = dp_peer_find_hash_find(soc, peer_mac,
						      0, vdev_id,
						      DP_MOD_ID_CDP);

	if (!peer) {
		dp_cdp_debug("%pK: Peer is NULL!\n", soc);
		status = QDF_STATUS_E_FAILURE;
	} else {
		peer->authorize = authorize ? 1 : 0;

		if (!peer->authorize)
			dp_peer_flush_frags(soc_hdl, vdev_id, peer_mac);

		dp_mlo_peer_authorize(soc, peer);
		dp_peer_unref_delete(peer, DP_MOD_ID_CDP);
	}

	return status;
}

/*
 * dp_peer_get_authorize() - get peer authorize status
 * @soc: soc handle
 * @vdev_id: id of dp handle
 * @peer_mac: mac of datapath PEER handle
 *
 * Retusn: true is peer is authorized, false otherwise
 */
static bool
dp_peer_get_authorize(struct cdp_soc_t *soc_hdl, uint8_t vdev_id,
		      uint8_t *peer_mac)
{
	struct dp_soc *soc = (struct dp_soc *)soc_hdl;
	bool authorize = false;
	struct dp_peer *peer = dp_peer_find_hash_find(soc, peer_mac,
						      0, vdev_id,
						      DP_MOD_ID_CDP);

	if (!peer) {
		dp_cdp_debug("%pK: Peer is NULL!\n", soc);
		return authorize;
	}

	authorize = peer->authorize;
	dp_peer_unref_delete(peer, DP_MOD_ID_CDP);

	return authorize;
}

/**
 * dp_vdev_unref_delete() - check and process vdev delete
 * @soc : DP specific soc pointer
 * @vdev: DP specific vdev pointer
 * @mod_id: module id
 *
 */
void dp_vdev_unref_delete(struct dp_soc *soc, struct dp_vdev *vdev,
			  enum dp_mod_id mod_id)
{
	ol_txrx_vdev_delete_cb vdev_delete_cb = NULL;
	void *vdev_delete_context = NULL;
	uint8_t vdev_id = vdev->vdev_id;
	struct dp_pdev *pdev = vdev->pdev;
	struct dp_vdev *tmp_vdev = NULL;
	uint8_t found = 0;

	QDF_ASSERT(qdf_atomic_dec_return(&vdev->mod_refs[mod_id]) >= 0);

	/* Return if this is not the last reference*/
	if (!qdf_atomic_dec_and_test(&vdev->ref_cnt))
		return;

	/*
	 * This should be set as last reference need to released
	 * after cdp_vdev_detach() is called
	 *
	 * if this assert is hit there is a ref count issue
	 */
	QDF_ASSERT(vdev->delete.pending);

	vdev_delete_cb = vdev->delete.callback;
	vdev_delete_context = vdev->delete.context;

	dp_info("deleting vdev object %pK ("QDF_MAC_ADDR_FMT")- its last peer is done",
		vdev, QDF_MAC_ADDR_REF(vdev->mac_addr.raw));

	if (wlan_op_mode_monitor == vdev->opmode) {
		dp_monitor_vdev_delete(soc, vdev);
		goto free_vdev;
	}

	/* all peers are gone, go ahead and delete it */
	dp_tx_flow_pool_unmap_handler(pdev, vdev_id,
			FLOW_TYPE_VDEV, vdev_id);
	dp_tx_vdev_detach(vdev);
	dp_monitor_vdev_detach(vdev);

free_vdev:
	qdf_spinlock_destroy(&vdev->peer_list_lock);

	qdf_spin_lock_bh(&soc->inactive_vdev_list_lock);
	TAILQ_FOREACH(tmp_vdev, &soc->inactive_vdev_list,
		      inactive_list_elem) {
		if (tmp_vdev == vdev) {
			found = 1;
			break;
		}
	}
	if (found)
		TAILQ_REMOVE(&soc->inactive_vdev_list, vdev,
			     inactive_list_elem);
	/* delete this peer from the list */
	qdf_spin_unlock_bh(&soc->inactive_vdev_list_lock);

	dp_info("deleting vdev object %pK ("QDF_MAC_ADDR_FMT")",
		vdev, QDF_MAC_ADDR_REF(vdev->mac_addr.raw));
	wlan_minidump_remove(vdev, sizeof(*vdev), soc->ctrl_psoc,
			     WLAN_MD_DP_VDEV, "dp_vdev");
	qdf_mem_free(vdev);
	vdev = NULL;

	if (vdev_delete_cb)
		vdev_delete_cb(vdev_delete_context);
}

qdf_export_symbol(dp_vdev_unref_delete);

/*
 * dp_peer_unref_delete() - unref and delete peer
 * @peer_handle:    Datapath peer handle
 * @mod_id:         ID of module releasing reference
 *
 */
void dp_peer_unref_delete(struct dp_peer *peer, enum dp_mod_id mod_id)
{
	struct dp_vdev *vdev = peer->vdev;
	struct dp_pdev *pdev = vdev->pdev;
	struct dp_soc *soc = pdev->soc;
	uint16_t peer_id;
	struct cdp_peer_cookie peer_cookie;
	struct dp_peer *tmp_peer;
	bool found = false;

	if (mod_id > DP_MOD_ID_RX)
		QDF_ASSERT(qdf_atomic_dec_return(&peer->mod_refs[mod_id]) >= 0);

	/*
	 * Hold the lock all the way from checking if the peer ref count
	 * is zero until the peer references are removed from the hash
	 * table and vdev list (if the peer ref count is zero).
	 * This protects against a new HL tx operation starting to use the
	 * peer object just after this function concludes it's done being used.
	 * Furthermore, the lock needs to be held while checking whether the
	 * vdev's list of peers is empty, to make sure that list is not modified
	 * concurrently with the empty check.
	 */
	if (qdf_atomic_dec_and_test(&peer->ref_cnt)) {
		peer_id = peer->peer_id;

		/*
		 * Make sure that the reference to the peer in
		 * peer object map is removed
		 */
		QDF_ASSERT(peer_id == HTT_INVALID_PEER);

		dp_peer_debug("Deleting peer %pK ("QDF_MAC_ADDR_FMT")", peer,
			      QDF_MAC_ADDR_REF(peer->mac_addr.raw));

		/*
		 * Deallocate the extended stats contenxt
		 */
		dp_peer_ext_stats_ctx_dealloc(soc, peer);

		/* send peer destroy event to upper layer */
		qdf_mem_copy(peer_cookie.mac_addr, peer->mac_addr.raw,
			     QDF_MAC_ADDR_SIZE);
		peer_cookie.ctx = NULL;
		peer_cookie.ctx = (struct cdp_stats_cookie *)
					peer->rdkstats_ctx;
#if defined(FEATURE_PERPKT_INFO) && WDI_EVENT_ENABLE
		dp_wdi_event_handler(WDI_EVENT_PEER_DESTROY,
				     soc,
				     (void *)&peer_cookie,
				     peer->peer_id,
				     WDI_NO_VAL,
				     pdev->pdev_id);
#endif
		peer->rdkstats_ctx = NULL;
		wlan_minidump_remove(peer, sizeof(*peer), soc->ctrl_psoc,
				     WLAN_MD_DP_PEER, "dp_peer");

		qdf_spin_lock_bh(&soc->inactive_peer_list_lock);
		TAILQ_FOREACH(tmp_peer, &soc->inactive_peer_list,
			      inactive_list_elem) {
			if (tmp_peer == peer) {
				found = 1;
				break;
			}
		}
		if (found)
			TAILQ_REMOVE(&soc->inactive_peer_list, peer,
				     inactive_list_elem);
		/* delete this peer from the list */
		qdf_spin_unlock_bh(&soc->inactive_peer_list_lock);
		DP_AST_ASSERT(TAILQ_EMPTY(&peer->ast_entry_list));
		dp_peer_update_state(soc, peer, DP_PEER_STATE_FREED);

		/* cleanup the peer data */
		dp_peer_cleanup(vdev, peer);
		dp_monitor_peer_detach(soc, peer);

		qdf_spinlock_destroy(&peer->peer_state_lock);
		qdf_mem_free(peer);

		/*
		 * Decrement ref count taken at peer create
		 */
		dp_vdev_unref_delete(soc, vdev, DP_MOD_ID_CHILD);
	}
}

qdf_export_symbol(dp_peer_unref_delete);

#ifdef PEER_CACHE_RX_PKTS
static inline void dp_peer_rx_bufq_resources_deinit(struct dp_peer *peer)
{
	qdf_list_destroy(&peer->bufq_info.cached_bufq);
	qdf_spinlock_destroy(&peer->bufq_info.bufq_lock);
}
#else
static inline void dp_peer_rx_bufq_resources_deinit(struct dp_peer *peer)
{
}
#endif

/*
 * dp_peer_detach_wifi3() – Detach txrx peer
 * @soc_hdl: soc handle
 * @vdev_id: id of dp handle
 * @peer_mac: mac of datapath PEER handle
 * @bitmap: bitmap indicating special handling of request.
 *
 */
static QDF_STATUS dp_peer_delete_wifi3(struct cdp_soc_t *soc_hdl,
				       uint8_t vdev_id,
				       uint8_t *peer_mac, uint32_t bitmap)
{
	struct dp_soc *soc = cdp_soc_t_to_dp_soc(soc_hdl);
	struct dp_peer *peer = dp_peer_find_hash_find(soc, peer_mac,
						      0, vdev_id,
						      DP_MOD_ID_CDP);
	struct dp_vdev *vdev = NULL;

	/* Peer can be null for monitor vap mac address */
	if (!peer) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_DEBUG,
			  "%s: Invalid peer\n", __func__);
		return QDF_STATUS_E_FAILURE;
	}

	if (!peer->valid) {
		dp_peer_unref_delete(peer, DP_MOD_ID_CDP);
		dp_err("Invalid peer: "QDF_MAC_ADDR_FMT,
			QDF_MAC_ADDR_REF(peer_mac));
		return QDF_STATUS_E_ALREADY;
	}

	vdev = peer->vdev;

	if (!vdev)
		return QDF_STATUS_E_FAILURE;
	peer->valid = 0;

	dp_init_info("%pK: peer %pK (" QDF_MAC_ADDR_FMT ")",
		     soc, peer, QDF_MAC_ADDR_REF(peer->mac_addr.raw));

	dp_local_peer_id_free(peer->vdev->pdev, peer);

	/* Drop all rx packets before deleting peer */
	dp_clear_peer_internal(soc, peer);

	dp_peer_rx_bufq_resources_deinit(peer);

	qdf_spinlock_destroy(&peer->peer_info_lock);
	dp_peer_multipass_list_remove(peer);

	/* remove the reference to the peer from the hash table */
	dp_peer_find_hash_remove(soc, peer);

	dp_peer_vdev_list_remove(soc, vdev, peer);

	dp_peer_mlo_delete(soc, peer);

	qdf_spin_lock_bh(&soc->inactive_peer_list_lock);
	TAILQ_INSERT_TAIL(&soc->inactive_peer_list, peer,
			  inactive_list_elem);
	qdf_spin_unlock_bh(&soc->inactive_peer_list_lock);

	/*
	 * Remove the reference added during peer_attach.
	 * The peer will still be left allocated until the
	 * PEER_UNMAP message arrives to remove the other
	 * reference, added by the PEER_MAP message.
	 */
	dp_peer_unref_delete(peer, DP_MOD_ID_CONFIG);
	/*
	 * Remove the reference taken above
	 */
	dp_peer_unref_delete(peer, DP_MOD_ID_CDP);

	return QDF_STATUS_SUCCESS;
}

/*
 * dp_get_vdev_mac_addr_wifi3() – Detach txrx peer
 * @soc_hdl: Datapath soc handle
 * @vdev_id: virtual interface id
 *
 * Return: MAC address on success, NULL on failure.
 *
 */
static uint8_t *dp_get_vdev_mac_addr_wifi3(struct cdp_soc_t *soc_hdl,
					   uint8_t vdev_id)
{
	struct dp_soc *soc = cdp_soc_t_to_dp_soc(soc_hdl);
	struct dp_vdev *vdev = dp_vdev_get_ref_by_id(soc, vdev_id,
						     DP_MOD_ID_CDP);
	uint8_t *mac = NULL;

	if (!vdev)
		return NULL;

	mac = vdev->mac_addr.raw;
	dp_vdev_unref_delete(soc, vdev, DP_MOD_ID_CDP);

	return mac;
}

/*
 * dp_vdev_set_wds() - Enable per packet stats
 * @soc: DP soc handle
 * @vdev_id: id of DP VDEV handle
 * @val: value
 *
 * Return: none
 */
static int dp_vdev_set_wds(struct cdp_soc_t *soc_hdl, uint8_t vdev_id,
			   uint32_t val)
{
	struct dp_soc *soc = cdp_soc_t_to_dp_soc(soc_hdl);
	struct dp_vdev *vdev =
		dp_vdev_get_ref_by_id((struct dp_soc *)soc, vdev_id,
				      DP_MOD_ID_CDP);

	if (!vdev)
		return QDF_STATUS_E_FAILURE;

	vdev->wds_enabled = val;
	dp_vdev_unref_delete(soc, vdev, DP_MOD_ID_CDP);

	return QDF_STATUS_SUCCESS;
}

static int dp_get_opmode(struct cdp_soc_t *soc_hdl, uint8_t vdev_id)
{
	struct dp_soc *soc = cdp_soc_t_to_dp_soc(soc_hdl);
	struct dp_vdev *vdev = dp_vdev_get_ref_by_id(soc, vdev_id,
						     DP_MOD_ID_CDP);
	int opmode;

	if (!vdev) {
		dp_err("vdev for id %d is NULL", vdev_id);
		return -EINVAL;
	}
	opmode = vdev->opmode;
	dp_vdev_unref_delete(soc, vdev, DP_MOD_ID_CDP);

	return opmode;
}

/**
 * dp_get_os_rx_handles_from_vdev_wifi3() - Get os rx handles for a vdev
 * @soc_hdl: ol_txrx_soc_handle handle
 * @vdev_id: vdev id for which os rx handles are needed
 * @stack_fn_p: pointer to stack function pointer
 * @osif_handle_p: pointer to ol_osif_vdev_handle
 *
 * Return: void
 */
static
void dp_get_os_rx_handles_from_vdev_wifi3(struct cdp_soc_t *soc_hdl,
					  uint8_t vdev_id,
					  ol_txrx_rx_fp *stack_fn_p,
					  ol_osif_vdev_handle *osif_vdev_p)
{
	struct dp_soc *soc = cdp_soc_t_to_dp_soc(soc_hdl);
	struct dp_vdev *vdev = dp_vdev_get_ref_by_id(soc, vdev_id,
						     DP_MOD_ID_CDP);

	if (qdf_unlikely(!vdev)) {
		*stack_fn_p = NULL;
		*osif_vdev_p = NULL;
		return;
	}
	*stack_fn_p = vdev->osif_rx_stack;
	*osif_vdev_p = vdev->osif_vdev;
	dp_vdev_unref_delete(soc, vdev, DP_MOD_ID_CDP);
}

/**
 * dp_get_ctrl_pdev_from_vdev() - Get control pdev of vdev
 * @soc_hdl: datapath soc handle
 * @vdev_id: virtual device/interface id
 *
 * Return: Handle to control pdev
 */
static struct cdp_cfg *dp_get_ctrl_pdev_from_vdev_wifi3(
						struct cdp_soc_t *soc_hdl,
						uint8_t vdev_id)
{
	struct dp_soc *soc = cdp_soc_t_to_dp_soc(soc_hdl);
	struct dp_vdev *vdev = dp_vdev_get_ref_by_id(soc, vdev_id,
						     DP_MOD_ID_CDP);
	struct dp_pdev *pdev;

	if (!vdev)
		return NULL;

	pdev = vdev->pdev;
	dp_vdev_unref_delete(soc, vdev, DP_MOD_ID_CDP);
	return pdev ? (struct cdp_cfg *)pdev->wlan_cfg_ctx : NULL;
}

/**
 * dp_get_tx_pending() - read pending tx
 * @pdev_handle: Datapath PDEV handle
 *
 * Return: outstanding tx
 */
static int32_t dp_get_tx_pending(struct cdp_pdev *pdev_handle)
{
	struct dp_pdev *pdev = (struct dp_pdev *)pdev_handle;

	return qdf_atomic_read(&pdev->num_tx_outstanding);
}

/**
 * dp_get_peer_mac_from_peer_id() - get peer mac
 * @pdev_handle: Datapath PDEV handle
 * @peer_id: Peer ID
 * @peer_mac: MAC addr of PEER
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS dp_get_peer_mac_from_peer_id(struct cdp_soc_t *soc,
					       uint32_t peer_id,
					       uint8_t *peer_mac)
{
	struct dp_peer *peer;

	if (soc && peer_mac) {
		peer = dp_peer_get_ref_by_id((struct dp_soc *)soc,
					     (uint16_t)peer_id,
					     DP_MOD_ID_CDP);
		if (peer) {
			qdf_mem_copy(peer_mac, peer->mac_addr.raw,
				     QDF_MAC_ADDR_SIZE);
			dp_peer_unref_delete(peer, DP_MOD_ID_CDP);
			return QDF_STATUS_SUCCESS;
		}
	}

	return QDF_STATUS_E_FAILURE;
}

#ifdef MESH_MODE_SUPPORT
static
void dp_vdev_set_mesh_mode(struct cdp_vdev *vdev_hdl, uint32_t val)
{
	struct dp_vdev *vdev = (struct dp_vdev *)vdev_hdl;

	dp_cdp_info("%pK: val %d", vdev->pdev->soc, val);
	vdev->mesh_vdev = val;
	if (val)
		vdev->skip_sw_tid_classification |=
			DP_TX_MESH_ENABLED;
	else
		vdev->skip_sw_tid_classification &=
			~DP_TX_MESH_ENABLED;
}

/*
 * dp_peer_set_mesh_rx_filter() - to set the mesh rx filter
 * @vdev_hdl: virtual device object
 * @val: value to be set
 *
 * Return: void
 */
static
void dp_vdev_set_mesh_rx_filter(struct cdp_vdev *vdev_hdl, uint32_t val)
{
	struct dp_vdev *vdev = (struct dp_vdev *)vdev_hdl;

	dp_cdp_info("%pK: val %d", vdev->pdev->soc, val);
	vdev->mesh_rx_filter = val;
}
#endif

/*
 * dp_vdev_set_hlos_tid_override() - to set hlos tid override
 * @vdev_hdl: virtual device object
 * @val: value to be set
 *
 * Return: void
 */
static
void dp_vdev_set_hlos_tid_override(struct dp_vdev *vdev, uint32_t val)
{
	dp_cdp_info("%pK: val %d", vdev->pdev->soc, val);
	if (val)
		vdev->skip_sw_tid_classification |=
			DP_TXRX_HLOS_TID_OVERRIDE_ENABLED;
	else
		vdev->skip_sw_tid_classification &=
			~DP_TXRX_HLOS_TID_OVERRIDE_ENABLED;
}

/*
 * dp_vdev_get_hlos_tid_override() - to get hlos tid override flag
 * @vdev_hdl: virtual device object
 * @val: value to be set
 *
 * Return: 1 if this flag is set
 */
static
uint8_t dp_vdev_get_hlos_tid_override(struct cdp_vdev *vdev_hdl)
{
	struct dp_vdev *vdev = (struct dp_vdev *)vdev_hdl;

	return !!(vdev->skip_sw_tid_classification &
			DP_TXRX_HLOS_TID_OVERRIDE_ENABLED);
}

#ifdef VDEV_PEER_PROTOCOL_COUNT
static void dp_enable_vdev_peer_protocol_count(struct cdp_soc_t *soc_hdl,
					       int8_t vdev_id,
					       bool enable)
{
	struct dp_soc *soc = cdp_soc_t_to_dp_soc(soc_hdl);
	struct dp_vdev *vdev;

	vdev = dp_vdev_get_ref_by_id(soc, vdev_id, DP_MOD_ID_CDP);
	if (!vdev)
		return;

	dp_info("enable %d vdev_id %d", enable, vdev_id);
	vdev->peer_protocol_count_track = enable;
	dp_vdev_unref_delete(soc, vdev, DP_MOD_ID_CDP);
}

static void dp_enable_vdev_peer_protocol_drop_mask(struct cdp_soc_t *soc_hdl,
						   int8_t vdev_id,
						   int drop_mask)
{
	struct dp_soc *soc = cdp_soc_t_to_dp_soc(soc_hdl);
	struct dp_vdev *vdev;

	vdev = dp_vdev_get_ref_by_id(soc, vdev_id, DP_MOD_ID_CDP);
	if (!vdev)
		return;

	dp_info("drop_mask %d vdev_id %d", drop_mask, vdev_id);
	vdev->peer_protocol_count_dropmask = drop_mask;
	dp_vdev_unref_delete(soc, vdev, DP_MOD_ID_CDP);
}

static int dp_is_vdev_peer_protocol_count_enabled(struct cdp_soc_t *soc_hdl,
						  int8_t vdev_id)
{
	struct dp_soc *soc = cdp_soc_t_to_dp_soc(soc_hdl);
	struct dp_vdev *vdev;
	int peer_protocol_count_track;

	vdev = dp_vdev_get_ref_by_id(soc, vdev_id, DP_MOD_ID_CDP);
	if (!vdev)
		return 0;

	dp_info("enable %d vdev_id %d", vdev->peer_protocol_count_track,
		vdev_id);
	peer_protocol_count_track =
		vdev->peer_protocol_count_track;

	dp_vdev_unref_delete(soc, vdev, DP_MOD_ID_CDP);
	return peer_protocol_count_track;
}

static int dp_get_vdev_peer_protocol_drop_mask(struct cdp_soc_t *soc_hdl,
					       int8_t vdev_id)
{
	struct dp_soc *soc = cdp_soc_t_to_dp_soc(soc_hdl);
	struct dp_vdev *vdev;
	int peer_protocol_count_dropmask;

	vdev = dp_vdev_get_ref_by_id(soc, vdev_id, DP_MOD_ID_CDP);
	if (!vdev)
		return 0;

	dp_info("drop_mask %d vdev_id %d", vdev->peer_protocol_count_dropmask,
		vdev_id);
	peer_protocol_count_dropmask =
		vdev->peer_protocol_count_dropmask;

	dp_vdev_unref_delete(soc, vdev, DP_MOD_ID_CDP);
	return peer_protocol_count_dropmask;
}

#endif

bool dp_check_pdev_exists(struct dp_soc *soc, struct dp_pdev *data)
{
	uint8_t pdev_count;

	for (pdev_count = 0; pdev_count < MAX_PDEV_CNT; pdev_count++) {
		if (soc->pdev_list[pdev_count] &&
		    soc->pdev_list[pdev_count] == data)
			return true;
	}
	return false;
}

/**
 * dp_rx_bar_stats_cb(): BAR received stats callback
 * @soc: SOC handle
 * @cb_ctxt: Call back context
 * @reo_status: Reo status
 *
 * return: void
 */
void dp_rx_bar_stats_cb(struct dp_soc *soc, void *cb_ctxt,
	union hal_reo_status *reo_status)
{
	struct dp_pdev *pdev = (struct dp_pdev *)cb_ctxt;
	struct hal_reo_queue_status *queue_status = &(reo_status->queue_status);

	if (!dp_check_pdev_exists(soc, pdev)) {
		dp_err_rl("pdev doesn't exist");
		return;
	}

	if (!qdf_atomic_read(&soc->cmn_init_done))
		return;

	if (queue_status->header.status != HAL_REO_CMD_SUCCESS) {
		DP_PRINT_STATS("REO stats failure %d",
			       queue_status->header.status);
		qdf_atomic_set(&(pdev->stats_cmd_complete), 1);
		return;
	}

	pdev->stats.rx.bar_recv_cnt += queue_status->bar_rcvd_cnt;
	qdf_atomic_set(&(pdev->stats_cmd_complete), 1);

}

/**
 * dp_aggregate_vdev_stats(): Consolidate stats at VDEV level
 * @vdev: DP VDEV handle
 *
 * return: void
 */
void dp_aggregate_vdev_stats(struct dp_vdev *vdev,
			     struct cdp_vdev_stats *vdev_stats)
{
	struct dp_soc *soc = NULL;

	if (!vdev || !vdev->pdev)
		return;

	soc = vdev->pdev->soc;

	dp_update_vdev_ingress_stats(vdev);

	qdf_mem_copy(vdev_stats, &vdev->stats, sizeof(vdev->stats));

	dp_vdev_iterate_peer(vdev, dp_update_vdev_stats, vdev_stats,
			     DP_MOD_ID_GENERIC_STATS);

#if defined(FEATURE_PERPKT_INFO) && WDI_EVENT_ENABLE
	dp_wdi_event_handler(WDI_EVENT_UPDATE_DP_STATS, vdev->pdev->soc,
			     vdev_stats, vdev->vdev_id,
			     UPDATE_VDEV_STATS, vdev->pdev->pdev_id);
#endif
}

void dp_aggregate_pdev_stats(struct dp_pdev *pdev)
{
	struct dp_vdev *vdev = NULL;
	struct dp_soc *soc;
	struct cdp_vdev_stats *vdev_stats =
			qdf_mem_malloc_atomic(sizeof(struct cdp_vdev_stats));

	if (!vdev_stats) {
		dp_cdp_err("%pK: DP alloc failure - unable to get alloc vdev stats",
			   pdev->soc);
		return;
	}

	soc = pdev->soc;

	qdf_mem_zero(&pdev->stats.tx, sizeof(pdev->stats.tx));
	qdf_mem_zero(&pdev->stats.rx, sizeof(pdev->stats.rx));
	qdf_mem_zero(&pdev->stats.tx_i, sizeof(pdev->stats.tx_i));

	if (dp_monitor_is_enable_mcopy_mode(pdev))
		DP_UPDATE_STATS(pdev, pdev->invalid_peer);

	qdf_spin_lock_bh(&pdev->vdev_list_lock);
	TAILQ_FOREACH(vdev, &pdev->vdev_list, vdev_list_elem) {

		dp_aggregate_vdev_stats(vdev, vdev_stats);
		dp_update_pdev_stats(pdev, vdev_stats);
		dp_update_pdev_ingress_stats(pdev, vdev);
	}
	qdf_spin_unlock_bh(&pdev->vdev_list_lock);
	qdf_mem_free(vdev_stats);

#if defined(FEATURE_PERPKT_INFO) && WDI_EVENT_ENABLE
	dp_wdi_event_handler(WDI_EVENT_UPDATE_DP_STATS, pdev->soc, &pdev->stats,
			     pdev->pdev_id, UPDATE_PDEV_STATS, pdev->pdev_id);
#endif
}

/**
 * dp_vdev_getstats() - get vdev packet level stats
 * @vdev_handle: Datapath VDEV handle
 * @stats: cdp network device stats structure
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS dp_vdev_getstats(struct cdp_vdev *vdev_handle,
				   struct cdp_dev_stats *stats)
{
	struct dp_vdev *vdev = (struct dp_vdev *)vdev_handle;
	struct dp_pdev *pdev;
	struct dp_soc *soc;
	struct cdp_vdev_stats *vdev_stats;

	if (!vdev)
		return QDF_STATUS_E_FAILURE;

	pdev = vdev->pdev;
	if (!pdev)
		return QDF_STATUS_E_FAILURE;

	soc = pdev->soc;

	vdev_stats = qdf_mem_malloc(sizeof(struct cdp_vdev_stats));

	if (!vdev_stats) {
		dp_cdp_err("%pK: DP alloc failure - unable to get alloc vdev stats",
			   soc);
		return QDF_STATUS_E_FAILURE;
	}

	dp_aggregate_vdev_stats(vdev, vdev_stats);

	stats->tx_packets = vdev_stats->tx.comp_pkt.num;
	stats->tx_bytes = vdev_stats->tx.comp_pkt.bytes;

	stats->tx_errors = vdev_stats->tx.tx_failed;
	stats->tx_dropped = vdev_stats->tx_i.dropped.dropped_pkt.num +
			    vdev_stats->tx_i.sg.dropped_host.num +
			    vdev_stats->tx_i.mcast_en.dropped_map_error +
			    vdev_stats->tx_i.mcast_en.dropped_self_mac +
			    vdev_stats->tx_i.mcast_en.dropped_send_fail +
			    vdev_stats->tx.nawds_mcast_drop;

	if (!wlan_cfg_get_vdev_stats_hw_offload_config(soc->wlan_cfg_ctx)) {
		stats->rx_packets = vdev_stats->rx.to_stack.num;
		stats->rx_bytes = vdev_stats->rx.to_stack.bytes;
	} else {
		stats->rx_packets = vdev_stats->rx_i.reo_rcvd_pkt.num +
				    vdev_stats->rx_i.null_q_desc_pkt.num +
				    vdev_stats->rx_i.routed_eapol_pkt.num;
		stats->rx_bytes = vdev_stats->rx_i.reo_rcvd_pkt.bytes +
				  vdev_stats->rx_i.null_q_desc_pkt.bytes +
				  vdev_stats->rx_i.routed_eapol_pkt.bytes;
	}

	stats->rx_errors = vdev_stats->rx.err.mic_err +
			   vdev_stats->rx.err.decrypt_err +
			   vdev_stats->rx.err.fcserr +
			   vdev_stats->rx.err.pn_err +
			   vdev_stats->rx.err.oor_err +
			   vdev_stats->rx.err.jump_2k_err +
			   vdev_stats->rx.err.rxdma_wifi_parse_err;

	stats->rx_dropped = vdev_stats->rx.mec_drop.num +
			    vdev_stats->rx.multipass_rx_pkt_drop +
			    vdev_stats->rx.peer_unauth_rx_pkt_drop +
			    vdev_stats->rx.policy_check_drop +
			    vdev_stats->rx.nawds_mcast_drop;

	qdf_mem_free(vdev_stats);

	return QDF_STATUS_SUCCESS;
}

/**
 * dp_pdev_getstats() - get pdev packet level stats
 * @pdev_handle: Datapath PDEV handle
 * @stats: cdp network device stats structure
 *
 * Return: QDF_STATUS
 */
static void dp_pdev_getstats(struct cdp_pdev *pdev_handle,
			     struct cdp_dev_stats *stats)
{
	struct dp_pdev *pdev = (struct dp_pdev *)pdev_handle;

	dp_aggregate_pdev_stats(pdev);

	stats->tx_packets = pdev->stats.tx.comp_pkt.num;
	stats->tx_bytes = pdev->stats.tx.comp_pkt.bytes;

	stats->tx_errors = pdev->stats.tx.tx_failed;
	stats->tx_dropped = pdev->stats.tx_i.dropped.dropped_pkt.num +
			    pdev->stats.tx_i.sg.dropped_host.num +
			    pdev->stats.tx_i.mcast_en.dropped_map_error +
			    pdev->stats.tx_i.mcast_en.dropped_self_mac +
			    pdev->stats.tx_i.mcast_en.dropped_send_fail +
			    pdev->stats.tx.nawds_mcast_drop +
			    pdev->stats.tso_stats.dropped_host.num;

	if (!wlan_cfg_get_vdev_stats_hw_offload_config(pdev->soc->wlan_cfg_ctx)) {
		stats->rx_packets = pdev->stats.rx.to_stack.num;
		stats->rx_bytes = pdev->stats.rx.to_stack.bytes;
	} else {
		stats->rx_packets = pdev->stats.rx_i.reo_rcvd_pkt.num +
				    pdev->stats.rx_i.null_q_desc_pkt.num +
				    pdev->stats.rx_i.routed_eapol_pkt.num;
		stats->rx_bytes = pdev->stats.rx_i.reo_rcvd_pkt.bytes +
				  pdev->stats.rx_i.null_q_desc_pkt.bytes +
				  pdev->stats.rx_i.routed_eapol_pkt.bytes;
	}

	stats->rx_errors = pdev->stats.err.ip_csum_err +
		pdev->stats.err.tcp_udp_csum_err +
		pdev->stats.rx.err.mic_err +
		pdev->stats.rx.err.decrypt_err +
		pdev->stats.rx.err.fcserr +
		pdev->stats.rx.err.pn_err +
		pdev->stats.rx.err.oor_err +
		pdev->stats.rx.err.jump_2k_err +
		pdev->stats.rx.err.rxdma_wifi_parse_err +
		pdev->stats.err.rxdma_error +
		pdev->stats.err.reo_error;
	stats->rx_dropped = pdev->stats.dropped.msdu_not_done +
		pdev->stats.dropped.mec +
		pdev->stats.dropped.mesh_filter +
		pdev->stats.dropped.wifi_parse +
		pdev->stats.dropped.mon_rx_drop +
		pdev->stats.dropped.mon_radiotap_update_err +
		pdev->stats.rx.mec_drop.num +
		pdev->stats.rx.multipass_rx_pkt_drop +
		pdev->stats.rx.peer_unauth_rx_pkt_drop +
		pdev->stats.rx.policy_check_drop +
		pdev->stats.rx.nawds_mcast_drop;
}

/**
 * dp_get_device_stats() - get interface level packet stats
 * @soc: soc handle
 * @id : vdev_id or pdev_id based on type
 * @stats: cdp network device stats structure
 * @type: device type pdev/vdev
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS dp_get_device_stats(struct cdp_soc_t *soc_hdl, uint8_t id,
				      struct cdp_dev_stats *stats,
				      uint8_t type)
{
	struct dp_soc *soc = cdp_soc_t_to_dp_soc(soc_hdl);
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	struct dp_vdev *vdev;

	switch (type) {
	case UPDATE_VDEV_STATS:
		vdev = dp_vdev_get_ref_by_id(soc, id, DP_MOD_ID_CDP);

		if (vdev) {
			status = dp_vdev_getstats((struct cdp_vdev *)vdev,
						  stats);
			dp_vdev_unref_delete(soc, vdev, DP_MOD_ID_CDP);
		}
		return status;
	case UPDATE_PDEV_STATS:
		{
			struct dp_pdev *pdev =
				dp_get_pdev_from_soc_pdev_id_wifi3(
						(struct dp_soc *)soc,
						 id);
			if (pdev) {
				dp_pdev_getstats((struct cdp_pdev *)pdev,
						 stats);
				return QDF_STATUS_SUCCESS;
			}
		}
		break;
	default:
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
			"apstats cannot be updated for this input "
			"type %d", type);
		break;
	}

	return QDF_STATUS_E_FAILURE;
}

const
char *dp_srng_get_str_from_hal_ring_type(enum hal_ring_type ring_type)
{
	switch (ring_type) {
	case REO_DST:
		return "Reo_dst";
	case REO_EXCEPTION:
		return "Reo_exception";
	case REO_CMD:
		return "Reo_cmd";
	case REO_REINJECT:
		return "Reo_reinject";
	case REO_STATUS:
		return "Reo_status";
	case WBM2SW_RELEASE:
		return "wbm2sw_release";
	case TCL_DATA:
		return "tcl_data";
	case TCL_CMD_CREDIT:
		return "tcl_cmd_credit";
	case TCL_STATUS:
		return "tcl_status";
	case SW2WBM_RELEASE:
		return "sw2wbm_release";
	case RXDMA_BUF:
		return "Rxdma_buf";
	case RXDMA_DST:
		return "Rxdma_dst";
	case RXDMA_MONITOR_BUF:
		return "Rxdma_monitor_buf";
	case RXDMA_MONITOR_DESC:
		return "Rxdma_monitor_desc";
	case RXDMA_MONITOR_STATUS:
		return "Rxdma_monitor_status";
	case RXDMA_MONITOR_DST:
		return "Rxdma_monitor_destination";
	case WBM_IDLE_LINK:
		return "WBM_hw_idle_link";
	default:
		dp_err("Invalid ring type");
		break;
	}
	return "Invalid";
}

/*
 * dp_print_napi_stats(): NAPI stats
 * @soc - soc handle
 */
void dp_print_napi_stats(struct dp_soc *soc)
{
	hif_print_napi_stats(soc->hif_handle);
}

#ifdef QCA_PEER_EXT_STATS
/**
 * dp_txrx_host_peer_ext_stats_clr: Reinitialize the txrx peer ext stats
 *
 */
static inline void dp_txrx_host_peer_ext_stats_clr(struct dp_peer *peer)
{
	if (peer->pext_stats)
		qdf_mem_zero(peer->pext_stats, sizeof(*peer->pext_stats));
}
#else
static inline void dp_txrx_host_peer_ext_stats_clr(struct dp_peer *peer)
{
}
#endif

/**
 * dp_txrx_host_peer_stats_clr): Reinitialize the txrx peer stats
 * @soc: Datapath soc
 * @peer: Datatpath peer
 * @arg: argument to iter function
 *
 * Return: QDF_STATUS
 */
static inline void
dp_txrx_host_peer_stats_clr(struct dp_soc *soc,
			    struct dp_peer *peer,
			    void *arg)
{
	struct dp_rx_tid *rx_tid;
	uint8_t tid;

	for (tid = 0; tid < DP_MAX_TIDS; tid++) {
		rx_tid = &peer->rx_tid[tid];
		DP_STATS_CLR(rx_tid);
	}

	DP_STATS_CLR(peer);

	dp_txrx_host_peer_ext_stats_clr(peer);

#if defined(FEATURE_PERPKT_INFO) && WDI_EVENT_ENABLE
	dp_wdi_event_handler(WDI_EVENT_UPDATE_DP_STATS, peer->vdev->pdev->soc,
			     &peer->stats,  peer->peer_id,
			     UPDATE_PEER_STATS, peer->vdev->pdev->pdev_id);
#endif
}

/**
 * dp_txrx_host_stats_clr(): Reinitialize the txrx stats
 * @vdev: DP_VDEV handle
 * @dp_soc: DP_SOC handle
 *
 * Return: QDF_STATUS
 */
static inline QDF_STATUS
dp_txrx_host_stats_clr(struct dp_vdev *vdev, struct dp_soc *soc)
{
	if (!vdev || !vdev->pdev)
		return QDF_STATUS_E_FAILURE;

	/*
	 * if NSS offload is enabled, then send message
	 * to NSS FW to clear the stats. Once NSS FW clears the statistics
	 * then clear host statistics.
	 */
	if (wlan_cfg_get_dp_soc_nss_cfg(soc->wlan_cfg_ctx)) {
		if (soc->cdp_soc.ol_ops->nss_stats_clr)
			soc->cdp_soc.ol_ops->nss_stats_clr(soc->ctrl_psoc,
							   vdev->vdev_id);
	}

	dp_vdev_stats_hw_offload_target_clear(soc, vdev->pdev->pdev_id,
					      vdev->vdev_id);

	DP_STATS_CLR(vdev->pdev);
	DP_STATS_CLR(vdev->pdev->soc);
	DP_STATS_CLR(vdev);

	hif_clear_napi_stats(vdev->pdev->soc->hif_handle);

	dp_vdev_iterate_peer(vdev, dp_txrx_host_peer_stats_clr, NULL,
			     DP_MOD_ID_GENERIC_STATS);

#if defined(FEATURE_PERPKT_INFO) && WDI_EVENT_ENABLE
	dp_wdi_event_handler(WDI_EVENT_UPDATE_DP_STATS, vdev->pdev->soc,
			     &vdev->stats,  vdev->vdev_id,
			     UPDATE_VDEV_STATS, vdev->pdev->pdev_id);
#endif
	return QDF_STATUS_SUCCESS;
}

/*
 * dp_get_host_peer_stats()- function to print peer stats
 * @soc: dp_soc handle
 * @mac_addr: mac address of the peer
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS
dp_get_host_peer_stats(struct cdp_soc_t *soc, uint8_t *mac_addr)
{
	struct dp_peer *peer = NULL;

	if (!mac_addr) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
			  "%s: NULL peer mac addr\n", __func__);
		return QDF_STATUS_E_FAILURE;
	}

	peer = dp_peer_find_hash_find((struct dp_soc *)soc,
				      mac_addr, 0,
				      DP_VDEV_ALL,
				      DP_MOD_ID_CDP);
	if (!peer) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
			  "%s: Invalid peer\n", __func__);
		return QDF_STATUS_E_FAILURE;
	}

	dp_print_peer_stats(peer);
	dp_peer_rxtid_stats(peer, dp_rx_tid_stats_cb, NULL);

	dp_peer_unref_delete(peer, DP_MOD_ID_CDP);

	return QDF_STATUS_SUCCESS;
}

/**
 * dp_txrx_stats_help() - Helper function for Txrx_Stats
 *
 * Return: None
 */
static void dp_txrx_stats_help(void)
{
	dp_info("Command: iwpriv wlan0 txrx_stats <stats_option> <mac_id>");
	dp_info("stats_option:");
	dp_info("  1 -- HTT Tx Statistics");
	dp_info("  2 -- HTT Rx Statistics");
	dp_info("  3 -- HTT Tx HW Queue Statistics");
	dp_info("  4 -- HTT Tx HW Sched Statistics");
	dp_info("  5 -- HTT Error Statistics");
	dp_info("  6 -- HTT TQM Statistics");
	dp_info("  7 -- HTT TQM CMDQ Statistics");
	dp_info("  8 -- HTT TX_DE_CMN Statistics");
	dp_info("  9 -- HTT Tx Rate Statistics");
	dp_info(" 10 -- HTT Rx Rate Statistics");
	dp_info(" 11 -- HTT Peer Statistics");
	dp_info(" 12 -- HTT Tx SelfGen Statistics");
	dp_info(" 13 -- HTT Tx MU HWQ Statistics");
	dp_info(" 14 -- HTT RING_IF_INFO Statistics");
	dp_info(" 15 -- HTT SRNG Statistics");
	dp_info(" 16 -- HTT SFM Info Statistics");
	dp_info(" 17 -- HTT PDEV_TX_MU_MIMO_SCHED INFO Statistics");
	dp_info(" 18 -- HTT Peer List Details");
	dp_info(" 20 -- Clear Host Statistics");
	dp_info(" 21 -- Host Rx Rate Statistics");
	dp_info(" 22 -- Host Tx Rate Statistics");
	dp_info(" 23 -- Host Tx Statistics");
	dp_info(" 24 -- Host Rx Statistics");
	dp_info(" 25 -- Host AST Statistics");
	dp_info(" 26 -- Host SRNG PTR Statistics");
	dp_info(" 27 -- Host Mon Statistics");
	dp_info(" 28 -- Host REO Queue Statistics");
	dp_info(" 29 -- Host Soc cfg param Statistics");
	dp_info(" 30 -- Host pdev cfg param Statistics");
	dp_info(" 31 -- Host FISA stats");
	dp_info(" 32 -- Host Register Work stats");
}

/**
 * dp_print_host_stats()- Function to print the stats aggregated at host
 * @vdev_handle: DP_VDEV handle
 * @req: host stats type
 * @soc: dp soc handler
 *
 * Return: 0 on success, print error message in case of failure
 */
static int
dp_print_host_stats(struct dp_vdev *vdev,
		    struct cdp_txrx_stats_req *req,
		    struct dp_soc *soc)
{
	struct dp_pdev *pdev = (struct dp_pdev *)vdev->pdev;
	enum cdp_host_txrx_stats type =
			dp_stats_mapping_table[req->stats][STATS_HOST];

	dp_aggregate_pdev_stats(pdev);

	switch (type) {
	case TXRX_CLEAR_STATS:
		dp_txrx_host_stats_clr(vdev, soc);
		break;
	case TXRX_RX_RATE_STATS:
		dp_print_rx_rates(vdev);
		break;
	case TXRX_TX_RATE_STATS:
		dp_print_tx_rates(vdev);
		break;
	case TXRX_TX_HOST_STATS:
		dp_print_pdev_tx_stats(pdev);
		dp_print_soc_tx_stats(pdev->soc);
		break;
	case TXRX_RX_HOST_STATS:
		dp_print_pdev_rx_stats(pdev);
		dp_print_soc_rx_stats(pdev->soc);
		break;
	case TXRX_AST_STATS:
		dp_print_ast_stats(pdev->soc);
		dp_print_mec_stats(pdev->soc);
		dp_print_peer_table(vdev);
		break;
	case TXRX_SRNG_PTR_STATS:
		dp_print_ring_stats(pdev);
		break;
	case TXRX_RX_MON_STATS:
		dp_monitor_print_pdev_rx_mon_stats(pdev);
		break;
	case TXRX_REO_QUEUE_STATS:
		dp_get_host_peer_stats((struct cdp_soc_t *)pdev->soc,
				       req->peer_addr);
		break;
	case TXRX_SOC_CFG_PARAMS:
		dp_print_soc_cfg_params(pdev->soc);
		break;
	case TXRX_PDEV_CFG_PARAMS:
		dp_print_pdev_cfg_params(pdev);
		break;
	case TXRX_NAPI_STATS:
		dp_print_napi_stats(pdev->soc);
		break;
	case TXRX_SOC_INTERRUPT_STATS:
		dp_print_soc_interrupt_stats(pdev->soc);
		break;
	case TXRX_SOC_FSE_STATS:
		dp_rx_dump_fisa_table(pdev->soc);
		break;
	case TXRX_HAL_REG_WRITE_STATS:
		hal_dump_reg_write_stats(pdev->soc->hal_soc);
		hal_dump_reg_write_srng_stats(pdev->soc->hal_soc);
		break;
	case TXRX_SOC_REO_HW_DESC_DUMP:
		dp_get_rx_reo_queue_info((struct cdp_soc_t *)pdev->soc,
					 vdev->vdev_id);
		break;
	default:
		dp_info("Wrong Input For TxRx Host Stats");
		dp_txrx_stats_help();
		break;
	}
	return 0;
}

/*
 * dp_pdev_tid_stats_ingress_inc
 * @pdev: pdev handle
 * @val: increase in value
 *
 * Return: void
 */
static void
dp_pdev_tid_stats_ingress_inc(struct dp_pdev *pdev, uint32_t val)
{
	pdev->stats.tid_stats.ingress_stack += val;
}

/*
 * dp_pdev_tid_stats_osif_drop
 * @pdev: pdev handle
 * @val: increase in value
 *
 * Return: void
 */
static void
dp_pdev_tid_stats_osif_drop(struct dp_pdev *pdev, uint32_t val)
{
	pdev->stats.tid_stats.osif_drop += val;
}

/*
 * dp_get_fw_peer_stats()- function to print peer stats
 * @soc: soc handle
 * @pdev_id : id of the pdev handle
 * @mac_addr: mac address of the peer
 * @cap: Type of htt stats requested
 * @is_wait: if set, wait on completion from firmware response
 *
 * Currently Supporting only MAC ID based requests Only
 *	1: HTT_PEER_STATS_REQ_MODE_NO_QUERY
 *	2: HTT_PEER_STATS_REQ_MODE_QUERY_TQM
 *	3: HTT_PEER_STATS_REQ_MODE_FLUSH_TQM
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS
dp_get_fw_peer_stats(struct cdp_soc_t *soc, uint8_t pdev_id,
		     uint8_t *mac_addr,
		     uint32_t cap, uint32_t is_wait)
{
	int i;
	uint32_t config_param0 = 0;
	uint32_t config_param1 = 0;
	uint32_t config_param2 = 0;
	uint32_t config_param3 = 0;
	struct dp_pdev *pdev =
		dp_get_pdev_from_soc_pdev_id_wifi3((struct dp_soc *)soc,
						   pdev_id);

	if (!pdev)
		return QDF_STATUS_E_FAILURE;

	HTT_DBG_EXT_STATS_PEER_INFO_IS_MAC_ADDR_SET(config_param0, 1);
	config_param0 |= (1 << (cap + 1));

	for (i = 0; i < HTT_PEER_STATS_MAX_TLV; i++) {
		config_param1 |= (1 << i);
	}

	config_param2 |= (mac_addr[0] & 0x000000ff);
	config_param2 |= ((mac_addr[1] << 8) & 0x0000ff00);
	config_param2 |= ((mac_addr[2] << 16) & 0x00ff0000);
	config_param2 |= ((mac_addr[3] << 24) & 0xff000000);

	config_param3 |= (mac_addr[4] & 0x000000ff);
	config_param3 |= ((mac_addr[5] << 8) & 0x0000ff00);

	if (is_wait) {
		qdf_event_reset(&pdev->fw_peer_stats_event);
		dp_h2t_ext_stats_msg_send(pdev, HTT_DBG_EXT_STATS_PEER_INFO,
					  config_param0, config_param1,
					  config_param2, config_param3,
					  0, DBG_STATS_COOKIE_DP_STATS, 0);
		qdf_wait_single_event(&pdev->fw_peer_stats_event,
				      DP_FW_PEER_STATS_CMP_TIMEOUT_MSEC);
	} else {
		dp_h2t_ext_stats_msg_send(pdev, HTT_DBG_EXT_STATS_PEER_INFO,
					  config_param0, config_param1,
					  config_param2, config_param3,
					  0, DBG_STATS_COOKIE_DEFAULT, 0);
	}

	return QDF_STATUS_SUCCESS;

}

/* This struct definition will be removed from here
 * once it get added in FW headers*/
struct httstats_cmd_req {
    uint32_t    config_param0;
    uint32_t    config_param1;
    uint32_t    config_param2;
    uint32_t    config_param3;
    int cookie;
    u_int8_t    stats_id;
};

/*
 * dp_get_htt_stats: function to process the httstas request
 * @soc: DP soc handle
 * @pdev_id: id of pdev handle
 * @data: pointer to request data
 * @data_len: length for request data
 *
 * return: QDF_STATUS
 */
static QDF_STATUS
dp_get_htt_stats(struct cdp_soc_t *soc, uint8_t pdev_id, void *data,
		 uint32_t data_len)
{
	struct httstats_cmd_req *req = (struct httstats_cmd_req *)data;
	struct dp_pdev *pdev =
		dp_get_pdev_from_soc_pdev_id_wifi3((struct dp_soc *)soc,
						   pdev_id);

	if (!pdev)
		return QDF_STATUS_E_FAILURE;

	QDF_ASSERT(data_len == sizeof(struct httstats_cmd_req));
	dp_h2t_ext_stats_msg_send(pdev, req->stats_id,
				req->config_param0, req->config_param1,
				req->config_param2, req->config_param3,
				req->cookie, DBG_STATS_COOKIE_DEFAULT, 0);

	return QDF_STATUS_SUCCESS;
}

/**
 * dp_set_pdev_tidmap_prty_wifi3(): update tidmap priority in pdev
 * @pdev: DP_PDEV handle
 * @prio: tidmap priority value passed by the user
 *
 * Return: QDF_STATUS_SUCCESS on success
 */
static QDF_STATUS dp_set_pdev_tidmap_prty_wifi3(struct dp_pdev *pdev,
						uint8_t prio)
{
	struct dp_soc *soc = pdev->soc;

	soc->tidmap_prty = prio;

	hal_tx_set_tidmap_prty(soc->hal_soc, prio);
	return QDF_STATUS_SUCCESS;
}

/*
 * dp_get_peer_param: function to get parameters in peer
 * @cdp_soc: DP soc handle
 * @vdev_id: id of vdev handle
 * @peer_mac: peer mac address
 * @param: parameter type to be set
 * @val : address of buffer
 *
 * Return: val
 */
static QDF_STATUS dp_get_peer_param(struct cdp_soc_t *cdp_soc,  uint8_t vdev_id,
				    uint8_t *peer_mac,
				    enum cdp_peer_param_type param,
				    cdp_config_param_type *val)
{
	return QDF_STATUS_SUCCESS;
}

/*
 * dp_set_peer_param: function to set parameters in peer
 * @cdp_soc: DP soc handle
 * @vdev_id: id of vdev handle
 * @peer_mac: peer mac address
 * @param: parameter type to be set
 * @val: value of parameter to be set
 *
 * Return: 0 for success. nonzero for failure.
 */
static QDF_STATUS dp_set_peer_param(struct cdp_soc_t *cdp_soc,  uint8_t vdev_id,
				    uint8_t *peer_mac,
				    enum cdp_peer_param_type param,
				    cdp_config_param_type val)
{
	struct dp_peer *peer = dp_peer_find_hash_find((struct dp_soc *)cdp_soc,
						      peer_mac, 0, vdev_id,
						      DP_MOD_ID_CDP);

	if (!peer)
		return QDF_STATUS_E_FAILURE;

	switch (param) {
	case CDP_CONFIG_NAWDS:
		peer->nawds_enabled = val.cdp_peer_param_nawds;
		break;
	case CDP_CONFIG_NAC:
		peer->nac = !!(val.cdp_peer_param_nac);
		break;
	case CDP_CONFIG_ISOLATION:
		dp_set_peer_isolation(peer, val.cdp_peer_param_isolation);
		break;
	case CDP_CONFIG_IN_TWT:
		peer->in_twt = !!(val.cdp_peer_param_in_twt);
		break;
	default:
		break;
	}

	dp_peer_unref_delete(peer, DP_MOD_ID_CDP);

	return QDF_STATUS_SUCCESS;
}

/*
 * dp_get_pdev_param: function to get parameters from pdev
 * @cdp_soc: DP soc handle
 * @pdev_id: id of pdev handle
 * @param: parameter type to be get
 * @value : buffer for value
 *
 * Return: status
 */
static QDF_STATUS dp_get_pdev_param(struct cdp_soc_t *cdp_soc, uint8_t pdev_id,
				    enum cdp_pdev_param_type param,
				    cdp_config_param_type *val)
{
	struct cdp_pdev *pdev = (struct cdp_pdev *)
		dp_get_pdev_from_soc_pdev_id_wifi3((struct dp_soc *)cdp_soc,
						   pdev_id);
	if (!pdev)
		return QDF_STATUS_E_FAILURE;

	switch (param) {
	case CDP_CONFIG_VOW:
		val->cdp_pdev_param_cfg_vow =
				((struct dp_pdev *)pdev)->delay_stats_flag;
		break;
	case CDP_TX_PENDING:
		val->cdp_pdev_param_tx_pending = dp_get_tx_pending(pdev);
		break;
	case CDP_FILTER_MCAST_DATA:
		val->cdp_pdev_param_fltr_mcast =
				dp_monitor_pdev_get_filter_mcast_data(pdev);
		break;
	case CDP_FILTER_NO_DATA:
		val->cdp_pdev_param_fltr_none =
				dp_monitor_pdev_get_filter_non_data(pdev);
		break;
	case CDP_FILTER_UCAST_DATA:
		val->cdp_pdev_param_fltr_ucast =
				dp_monitor_pdev_get_filter_ucast_data(pdev);
		break;
	default:
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

/*
 * dp_set_pdev_param: function to set parameters in pdev
 * @cdp_soc: DP soc handle
 * @pdev_id: id of pdev handle
 * @param: parameter type to be set
 * @val: value of parameter to be set
 *
 * Return: 0 for success. nonzero for failure.
 */
static QDF_STATUS dp_set_pdev_param(struct cdp_soc_t *cdp_soc, uint8_t pdev_id,
				    enum cdp_pdev_param_type param,
				    cdp_config_param_type val)
{
	int target_type;
	struct dp_soc *soc = (struct dp_soc *)cdp_soc;
	struct dp_pdev *pdev =
		dp_get_pdev_from_soc_pdev_id_wifi3((struct dp_soc *)cdp_soc,
						   pdev_id);
	enum reg_wifi_band chan_band;

	if (!pdev)
		return QDF_STATUS_E_FAILURE;

	target_type = hal_get_target_type(soc->hal_soc);
	switch (target_type) {
	case TARGET_TYPE_QCA6750:
		pdev->ch_band_lmac_id_mapping[REG_BAND_2G] = DP_MAC0_LMAC_ID;
		pdev->ch_band_lmac_id_mapping[REG_BAND_5G] = DP_MAC0_LMAC_ID;
		pdev->ch_band_lmac_id_mapping[REG_BAND_6G] = DP_MAC0_LMAC_ID;
		break;
	case TARGET_TYPE_KIWI:
		pdev->ch_band_lmac_id_mapping[REG_BAND_2G] = DP_MAC0_LMAC_ID;
		pdev->ch_band_lmac_id_mapping[REG_BAND_5G] = DP_MAC0_LMAC_ID;
		pdev->ch_band_lmac_id_mapping[REG_BAND_6G] = DP_MAC0_LMAC_ID;
		break;
	default:
		pdev->ch_band_lmac_id_mapping[REG_BAND_2G] = DP_MAC1_LMAC_ID;
		pdev->ch_band_lmac_id_mapping[REG_BAND_5G] = DP_MAC0_LMAC_ID;
		pdev->ch_band_lmac_id_mapping[REG_BAND_6G] = DP_MAC0_LMAC_ID;
		break;
	}

	switch (param) {
	case CDP_CONFIG_TX_CAPTURE:
		return dp_monitor_config_debug_sniffer(pdev,
						val.cdp_pdev_param_tx_capture);
	case CDP_CONFIG_DEBUG_SNIFFER:
		return dp_monitor_config_debug_sniffer(pdev,
						val.cdp_pdev_param_dbg_snf);
	case CDP_CONFIG_BPR_ENABLE:
		return dp_monitor_set_bpr_enable(pdev,
						 val.cdp_pdev_param_bpr_enable);
	case CDP_CONFIG_PRIMARY_RADIO:
		pdev->is_primary = val.cdp_pdev_param_primary_radio;
		break;
	case CDP_CONFIG_CAPTURE_LATENCY:
		pdev->latency_capture_enable = val.cdp_pdev_param_cptr_latcy;
		break;
	case CDP_INGRESS_STATS:
		dp_pdev_tid_stats_ingress_inc(pdev,
					      val.cdp_pdev_param_ingrs_stats);
		break;
	case CDP_OSIF_DROP:
		dp_pdev_tid_stats_osif_drop(pdev,
					    val.cdp_pdev_param_osif_drop);
		break;
	case CDP_CONFIG_ENH_RX_CAPTURE:
		return dp_monitor_config_enh_rx_capture(pdev,
						val.cdp_pdev_param_en_rx_cap);
	case CDP_CONFIG_ENH_TX_CAPTURE:
		return dp_monitor_config_enh_tx_capture(pdev,
						val.cdp_pdev_param_en_tx_cap);
	case CDP_CONFIG_HMMC_TID_OVERRIDE:
		pdev->hmmc_tid_override_en = val.cdp_pdev_param_hmmc_tid_ovrd;
		break;
	case CDP_CONFIG_HMMC_TID_VALUE:
		pdev->hmmc_tid = val.cdp_pdev_param_hmmc_tid;
		break;
	case CDP_CHAN_NOISE_FLOOR:
		pdev->chan_noise_floor = val.cdp_pdev_param_chn_noise_flr;
		break;
	case CDP_TIDMAP_PRTY:
		dp_set_pdev_tidmap_prty_wifi3(pdev,
					      val.cdp_pdev_param_tidmap_prty);
		break;
	case CDP_FILTER_NEIGH_PEERS:
		dp_monitor_set_filter_neigh_peers(pdev,
					val.cdp_pdev_param_fltr_neigh_peers);
		break;
	case CDP_MONITOR_CHANNEL:
		dp_monitor_set_chan_num(pdev, val.cdp_pdev_param_monitor_chan);
		break;
	case CDP_MONITOR_FREQUENCY:
		chan_band = wlan_reg_freq_to_band(val.cdp_pdev_param_mon_freq);
		dp_monitor_set_chan_freq(pdev, val.cdp_pdev_param_mon_freq);
		dp_monitor_set_chan_band(pdev, chan_band);
		break;
	case CDP_CONFIG_BSS_COLOR:
		dp_monitor_set_bsscolor(pdev, val.cdp_pdev_param_bss_color);
		break;
	case CDP_SET_ATF_STATS_ENABLE:
		dp_monitor_set_atf_stats_enable(pdev,
					val.cdp_pdev_param_atf_stats_enable);
		break;
	case CDP_CONFIG_SPECIAL_VAP:
		dp_monitor_pdev_config_scan_spcl_vap(pdev,
					val.cdp_pdev_param_config_special_vap);
		dp_monitor_vdev_set_monitor_mode_buf_rings(pdev);
		break;
	case CDP_RESET_SCAN_SPCL_VAP_STATS_ENABLE:
		dp_monitor_pdev_reset_scan_spcl_vap_stats_enable(pdev,
				val.cdp_pdev_param_reset_scan_spcl_vap_stats_enable);
		break;
	default:
		return QDF_STATUS_E_INVAL;
	}
	return QDF_STATUS_SUCCESS;
}

#ifdef QCA_PEER_EXT_STATS
static void dp_rx_update_peer_delay_stats(struct dp_soc *soc,
					  qdf_nbuf_t nbuf)
{
	struct dp_peer *peer = NULL;
	uint16_t peer_id, ring_id;
	uint8_t tid = qdf_nbuf_get_tid_val(nbuf);
	struct cdp_peer_ext_stats *pext_stats = NULL;

	peer_id = QDF_NBUF_CB_RX_PEER_ID(nbuf);
	if (peer_id > soc->max_peer_id)
		return;

	peer = dp_peer_get_ref_by_id(soc, peer_id, DP_MOD_ID_CDP);
	if (qdf_unlikely(!peer))
		return;

	if (qdf_likely(peer->pext_stats)) {
		pext_stats = peer->pext_stats;
		ring_id = QDF_NBUF_CB_RX_CTX_ID(nbuf);
		dp_rx_compute_tid_delay(&pext_stats->delay_stats[tid][ring_id],
					nbuf);
	}
	dp_peer_unref_delete(peer, DP_MOD_ID_CDP);
}
#else
static inline void dp_rx_update_peer_delay_stats(struct dp_soc *soc,
						 qdf_nbuf_t nbuf)
{
}
#endif

/*
 * dp_calculate_delay_stats: function to get rx delay stats
 * @cdp_soc: DP soc handle
 * @vdev_id: id of DP vdev handle
 * @nbuf: skb
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS
dp_calculate_delay_stats(struct cdp_soc_t *cdp_soc, uint8_t vdev_id,
			 qdf_nbuf_t nbuf)
{
	struct dp_soc *soc = cdp_soc_t_to_dp_soc(cdp_soc);
	struct dp_vdev *vdev = dp_vdev_get_ref_by_id(soc, vdev_id,
						     DP_MOD_ID_CDP);

	if (!vdev)
		return QDF_STATUS_SUCCESS;

	if (vdev->pdev->delay_stats_flag)
		dp_rx_compute_delay(vdev, nbuf);
	else
		dp_rx_update_peer_delay_stats(soc, nbuf);

	dp_vdev_unref_delete(soc, vdev, DP_MOD_ID_CDP);
	return QDF_STATUS_SUCCESS;
}

/*
 * dp_get_vdev_param: function to get parameters from vdev
 * @cdp_soc : DP soc handle
 * @vdev_id: id of DP vdev handle
 * @param: parameter type to get value
 * @val: buffer address
 *
 * return: status
 */
static QDF_STATUS dp_get_vdev_param(struct cdp_soc_t *cdp_soc, uint8_t vdev_id,
				    enum cdp_vdev_param_type param,
				    cdp_config_param_type *val)
{
	struct dp_soc *soc = cdp_soc_t_to_dp_soc(cdp_soc);
	struct dp_vdev *vdev = dp_vdev_get_ref_by_id(soc, vdev_id,
						     DP_MOD_ID_CDP);

	if (!vdev)
		return QDF_STATUS_E_FAILURE;

	switch (param) {
	case CDP_ENABLE_WDS:
		val->cdp_vdev_param_wds = vdev->wds_enabled;
		break;
	case CDP_ENABLE_MEC:
		val->cdp_vdev_param_mec = vdev->mec_enabled;
		break;
	case CDP_ENABLE_DA_WAR:
		val->cdp_vdev_param_da_war = vdev->pdev->soc->da_war_enabled;
		break;
	case CDP_ENABLE_IGMP_MCAST_EN:
		val->cdp_vdev_param_igmp_mcast_en = vdev->igmp_mcast_enhanc_en;
		break;
	case CDP_ENABLE_MCAST_EN:
		val->cdp_vdev_param_mcast_en = vdev->mcast_enhancement_en;
		break;
	case CDP_ENABLE_HLOS_TID_OVERRIDE:
		val->cdp_vdev_param_hlos_tid_override =
			    dp_vdev_get_hlos_tid_override((struct cdp_vdev *)vdev);
		break;
	case CDP_ENABLE_PEER_AUTHORIZE:
		val->cdp_vdev_param_peer_authorize =
			    vdev->peer_authorize;
		break;
#ifdef WLAN_SUPPORT_MESH_LATENCY
	case CDP_ENABLE_PEER_TID_LATENCY:
		val->cdp_vdev_param_peer_tid_latency_enable =
			vdev->peer_tid_latency_enabled;
		break;
	case CDP_SET_VAP_MESH_TID:
		val->cdp_vdev_param_mesh_tid =
				vdev->mesh_tid_latency_config.latency_tid;
		break;
#endif
	default:
		dp_cdp_err("%pK: param value %d is wrong",
			   soc, param);
		dp_vdev_unref_delete(soc, vdev, DP_MOD_ID_CDP);
		return QDF_STATUS_E_FAILURE;
	}

	dp_vdev_unref_delete(soc, vdev, DP_MOD_ID_CDP);
	return QDF_STATUS_SUCCESS;
}

/*
 * dp_set_vdev_param: function to set parameters in vdev
 * @cdp_soc : DP soc handle
 * @vdev_id: id of DP vdev handle
 * @param: parameter type to get value
 * @val: value
 *
 * return: QDF_STATUS
 */
static QDF_STATUS
dp_set_vdev_param(struct cdp_soc_t *cdp_soc, uint8_t vdev_id,
		  enum cdp_vdev_param_type param, cdp_config_param_type val)
{
	struct dp_soc *dsoc = (struct dp_soc *)cdp_soc;
	struct dp_vdev *vdev =
		dp_vdev_get_ref_by_id(dsoc, vdev_id, DP_MOD_ID_CDP);
	uint32_t var = 0;

	if (!vdev)
		return QDF_STATUS_E_FAILURE;

	switch (param) {
	case CDP_ENABLE_WDS:
		dp_cdp_err("%pK: wds_enable %d for vdev(%pK) id(%d)\n",
			   dsoc, val.cdp_vdev_param_wds, vdev, vdev->vdev_id);
		vdev->wds_enabled = val.cdp_vdev_param_wds;
		break;
	case CDP_ENABLE_MEC:
		dp_cdp_err("%pK: mec_enable %d for vdev(%pK) id(%d)\n",
			   dsoc, val.cdp_vdev_param_mec, vdev, vdev->vdev_id);
		vdev->mec_enabled = val.cdp_vdev_param_mec;
		break;
	case CDP_ENABLE_DA_WAR:
		dp_cdp_err("%pK: da_war_enable %d for vdev(%pK) id(%d)\n",
			   dsoc, val.cdp_vdev_param_da_war, vdev, vdev->vdev_id);
		vdev->pdev->soc->da_war_enabled = val.cdp_vdev_param_da_war;
		dp_wds_flush_ast_table_wifi3(((struct cdp_soc_t *)
					     vdev->pdev->soc));
		break;
	case CDP_ENABLE_NAWDS:
		vdev->nawds_enabled = val.cdp_vdev_param_nawds;
		break;
	case CDP_ENABLE_MCAST_EN:
		vdev->mcast_enhancement_en = val.cdp_vdev_param_mcast_en;
		break;
	case CDP_ENABLE_IGMP_MCAST_EN:
		vdev->igmp_mcast_enhanc_en = val.cdp_vdev_param_igmp_mcast_en;
		break;
	case CDP_ENABLE_PROXYSTA:
		vdev->proxysta_vdev = val.cdp_vdev_param_proxysta;
		break;
	case CDP_UPDATE_TDLS_FLAGS:
		vdev->tdls_link_connected = val.cdp_vdev_param_tdls_flags;
		break;
	case CDP_CFG_WDS_AGING_TIMER:
		var = val.cdp_vdev_param_aging_tmr;
		if (!var)
			qdf_timer_stop(&vdev->pdev->soc->ast_aging_timer);
		else if (var != vdev->wds_aging_timer_val)
			qdf_timer_mod(&vdev->pdev->soc->ast_aging_timer, var);

		vdev->wds_aging_timer_val = var;
		break;
	case CDP_ENABLE_AP_BRIDGE:
		if (wlan_op_mode_sta != vdev->opmode)
			vdev->ap_bridge_enabled = val.cdp_vdev_param_ap_brdg_en;
		else
			vdev->ap_bridge_enabled = false;
		break;
	case CDP_ENABLE_CIPHER:
		vdev->sec_type = val.cdp_vdev_param_cipher_en;
		break;
	case CDP_ENABLE_QWRAP_ISOLATION:
		vdev->isolation_vdev = val.cdp_vdev_param_qwrap_isolation;
		break;
	case CDP_UPDATE_MULTIPASS:
		vdev->multipass_en = val.cdp_vdev_param_update_multipass;
		break;
	case CDP_TX_ENCAP_TYPE:
		vdev->tx_encap_type = val.cdp_vdev_param_tx_encap;
		break;
	case CDP_RX_DECAP_TYPE:
		vdev->rx_decap_type = val.cdp_vdev_param_rx_decap;
		break;
	case CDP_TID_VDEV_PRTY:
		vdev->tidmap_prty = val.cdp_vdev_param_tidmap_prty;
		break;
	case CDP_TIDMAP_TBL_ID:
		vdev->tidmap_tbl_id = val.cdp_vdev_param_tidmap_tbl_id;
		break;
#ifdef MESH_MODE_SUPPORT
	case CDP_MESH_RX_FILTER:
		dp_vdev_set_mesh_rx_filter((struct cdp_vdev *)vdev,
					   val.cdp_vdev_param_mesh_rx_filter);
		break;
	case CDP_MESH_MODE:
		dp_vdev_set_mesh_mode((struct cdp_vdev *)vdev,
				      val.cdp_vdev_param_mesh_mode);
		break;
#endif
	case CDP_ENABLE_HLOS_TID_OVERRIDE:
		dp_info("vdev_id %d enable hlod tid override %d", vdev_id,
			val.cdp_vdev_param_hlos_tid_override);
		dp_vdev_set_hlos_tid_override(vdev,
				val.cdp_vdev_param_hlos_tid_override);
		break;
#ifdef QCA_SUPPORT_WDS_EXTENDED
	case CDP_CFG_WDS_EXT:
		vdev->wds_ext_enabled = val.cdp_vdev_param_wds_ext;
		break;
#endif
	case CDP_ENABLE_PEER_AUTHORIZE:
		vdev->peer_authorize = val.cdp_vdev_param_peer_authorize;
		break;
#ifdef WLAN_SUPPORT_MESH_LATENCY
	case CDP_ENABLE_PEER_TID_LATENCY:
		dp_info("vdev_id %d enable peer tid latency %d", vdev_id,
			val.cdp_vdev_param_peer_tid_latency_enable);
		vdev->peer_tid_latency_enabled =
			val.cdp_vdev_param_peer_tid_latency_enable;
		break;
	case CDP_SET_VAP_MESH_TID:
		dp_info("vdev_id %d enable peer tid latency %d", vdev_id,
			val.cdp_vdev_param_mesh_tid);
		vdev->mesh_tid_latency_config.latency_tid
				= val.cdp_vdev_param_mesh_tid;
		break;
#endif
#ifdef WLAN_VENDOR_SPECIFIC_BAR_UPDATE
	case CDP_SKIP_BAR_UPDATE_AP:
		dp_info("vdev_id %d skip BAR update: %u", vdev_id,
			val.cdp_skip_bar_update);
		vdev->skip_bar_update = val.cdp_skip_bar_update;
		vdev->skip_bar_update_last_ts = 0;
		break;
#endif
	default:
		break;
	}

	dp_tx_vdev_update_search_flags((struct dp_vdev *)vdev);
	dsoc->arch_ops.txrx_set_vdev_param(dsoc, vdev, param, val);
	dp_vdev_unref_delete(dsoc, vdev, DP_MOD_ID_CDP);

	return QDF_STATUS_SUCCESS;
}

/*
 * dp_set_psoc_param: function to set parameters in psoc
 * @cdp_soc : DP soc handle
 * @param: parameter type to be set
 * @val: value of parameter to be set
 *
 * return: QDF_STATUS
 */
static QDF_STATUS
dp_set_psoc_param(struct cdp_soc_t *cdp_soc,
		  enum cdp_psoc_param_type param, cdp_config_param_type val)
{
	struct dp_soc *soc = (struct dp_soc *)cdp_soc;
	struct wlan_cfg_dp_soc_ctxt *wlan_cfg_ctx = soc->wlan_cfg_ctx;

	switch (param) {
	case CDP_ENABLE_RATE_STATS:
		soc->rdkstats_enabled = val.cdp_psoc_param_en_rate_stats;
		break;
	case CDP_SET_NSS_CFG:
		wlan_cfg_set_dp_soc_nss_cfg(wlan_cfg_ctx,
					    val.cdp_psoc_param_en_nss_cfg);
		/*
		 * TODO: masked out based on the per offloaded radio
		 */
		switch (val.cdp_psoc_param_en_nss_cfg) {
		case dp_nss_cfg_default:
			break;
		case dp_nss_cfg_first_radio:
		/*
		 * This configuration is valid for single band radio which
		 * is also NSS offload.
		 */
		case dp_nss_cfg_dbdc:
		case dp_nss_cfg_dbtc:
			wlan_cfg_set_num_tx_desc_pool(wlan_cfg_ctx, 0);
			wlan_cfg_set_num_tx_ext_desc_pool(wlan_cfg_ctx, 0);
			wlan_cfg_set_num_tx_desc(wlan_cfg_ctx, 0);
			wlan_cfg_set_num_tx_ext_desc(wlan_cfg_ctx, 0);
			break;
		default:
			dp_cdp_err("%pK: Invalid offload config %d",
				   soc, val.cdp_psoc_param_en_nss_cfg);
		}

			dp_cdp_err("%pK: nss-wifi<0> nss config is enabled"
				   , soc);
		break;
	case CDP_SET_PREFERRED_HW_MODE:
		soc->preferred_hw_mode = val.cdp_psoc_param_preferred_hw_mode;
		break;
	case CDP_IPA_ENABLE:
		soc->wlan_cfg_ctx->ipa_enabled = val.cdp_ipa_enabled;
		break;
	case CDP_SET_VDEV_STATS_HW_OFFLOAD:
		wlan_cfg_set_vdev_stats_hw_offload_config(wlan_cfg_ctx,
				val.cdp_psoc_param_vdev_stats_hw_offload);
		break;
	default:
		break;
	}

	return QDF_STATUS_SUCCESS;
}

/*
 * dp_get_psoc_param: function to get parameters in soc
 * @cdp_soc : DP soc handle
 * @param: parameter type to be set
 * @val: address of buffer
 *
 * return: status
 */
static QDF_STATUS dp_get_psoc_param(struct cdp_soc_t *cdp_soc,
				    enum cdp_psoc_param_type param,
				    cdp_config_param_type *val)
{
	struct dp_soc *soc = (struct dp_soc *)cdp_soc;

	if (!soc)
		return QDF_STATUS_E_FAILURE;

	switch (param) {
	case CDP_CFG_PEER_EXT_STATS:
		val->cdp_psoc_param_pext_stats =
			wlan_cfg_is_peer_ext_stats_enabled(soc->wlan_cfg_ctx);
		break;
	default:
		dp_warn("Invalid param");
		break;
	}

	return QDF_STATUS_SUCCESS;
}

/*
 * dp_set_vdev_dscp_tid_map_wifi3(): Update Map ID selected for particular vdev
 * @soc: DP_SOC handle
 * @vdev_id: id of DP_VDEV handle
 * @map_id:ID of map that needs to be updated
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS dp_set_vdev_dscp_tid_map_wifi3(ol_txrx_soc_handle cdp_soc,
						 uint8_t vdev_id,
						 uint8_t map_id)
{
	cdp_config_param_type val;
	struct dp_soc *soc = cdp_soc_t_to_dp_soc(cdp_soc);
	struct dp_vdev *vdev = dp_vdev_get_ref_by_id(soc, vdev_id,
						     DP_MOD_ID_CDP);
	if (vdev) {
		vdev->dscp_tid_map_id = map_id;
		val.cdp_vdev_param_dscp_tid_map_id = map_id;
		soc->arch_ops.txrx_set_vdev_param(soc,
						  vdev,
						  CDP_UPDATE_DSCP_TO_TID_MAP,
						  val);
		/* Updatr flag for transmit tid classification */
		if (vdev->dscp_tid_map_id < soc->num_hw_dscp_tid_map)
			vdev->skip_sw_tid_classification |=
				DP_TX_HW_DSCP_TID_MAP_VALID;
		else
			vdev->skip_sw_tid_classification &=
				~DP_TX_HW_DSCP_TID_MAP_VALID;
		dp_vdev_unref_delete(soc, vdev, DP_MOD_ID_CDP);
		return QDF_STATUS_SUCCESS;
	}

	return QDF_STATUS_E_FAILURE;
}

#ifdef DP_RATETABLE_SUPPORT
static int dp_txrx_get_ratekbps(int preamb, int mcs,
				int htflag, int gintval)
{
	uint32_t rix;
	uint16_t ratecode;

	return dp_getrateindex((uint32_t)gintval, (uint16_t)mcs, 1,
			       (uint8_t)preamb, 1, &rix, &ratecode);
}
#else
static int dp_txrx_get_ratekbps(int preamb, int mcs,
				int htflag, int gintval)
{
	return 0;
}
#endif

/* dp_txrx_get_pdev_stats - Returns cdp_pdev_stats
 * @soc: DP soc handle
 * @pdev_id: id of DP pdev handle
 * @pdev_stats: buffer to copy to
 *
 * return : status success/failure
 */
static QDF_STATUS
dp_txrx_get_pdev_stats(struct cdp_soc_t *soc, uint8_t pdev_id,
		       struct cdp_pdev_stats *pdev_stats)
{
	struct dp_pdev *pdev =
		dp_get_pdev_from_soc_pdev_id_wifi3((struct dp_soc *)soc,
						   pdev_id);
	if (!pdev)
		return QDF_STATUS_E_FAILURE;

	dp_aggregate_pdev_stats(pdev);

	qdf_mem_copy(pdev_stats, &pdev->stats, sizeof(struct cdp_pdev_stats));
	return QDF_STATUS_SUCCESS;
}

/* dp_txrx_update_vdev_me_stats(): Update vdev ME stats sent from CDP
 * @vdev: DP vdev handle
 * @buf: buffer containing specific stats structure
 *
 * Returns: void
 */
static void dp_txrx_update_vdev_me_stats(struct dp_vdev *vdev,
					 void *buf)
{
	struct cdp_tx_ingress_stats *host_stats = NULL;

	if (!buf) {
		dp_cdp_err("%pK: Invalid host stats buf", vdev->pdev->soc);
		return;
	}
	host_stats = (struct cdp_tx_ingress_stats *)buf;

	DP_STATS_INC_PKT(vdev, tx_i.mcast_en.mcast_pkt,
			 host_stats->mcast_en.mcast_pkt.num,
			 host_stats->mcast_en.mcast_pkt.bytes);
	DP_STATS_INC(vdev, tx_i.mcast_en.dropped_map_error,
		     host_stats->mcast_en.dropped_map_error);
	DP_STATS_INC(vdev, tx_i.mcast_en.dropped_self_mac,
		     host_stats->mcast_en.dropped_self_mac);
	DP_STATS_INC(vdev, tx_i.mcast_en.dropped_send_fail,
		     host_stats->mcast_en.dropped_send_fail);
	DP_STATS_INC(vdev, tx_i.mcast_en.ucast,
		     host_stats->mcast_en.ucast);
	DP_STATS_INC(vdev, tx_i.mcast_en.fail_seg_alloc,
		     host_stats->mcast_en.fail_seg_alloc);
	DP_STATS_INC(vdev, tx_i.mcast_en.clone_fail,
		     host_stats->mcast_en.clone_fail);
}

/* dp_txrx_update_vdev_igmp_me_stats(): Update vdev IGMP ME stats sent from CDP
 * @vdev: DP vdev handle
 * @buf: buffer containing specific stats structure
 *
 * Returns: void
 */
static void dp_txrx_update_vdev_igmp_me_stats(struct dp_vdev *vdev,
					      void *buf)
{
	struct cdp_tx_ingress_stats *host_stats = NULL;

	if (!buf) {
		dp_cdp_err("%pK: Invalid host stats buf", vdev->pdev->soc);
		return;
	}
	host_stats = (struct cdp_tx_ingress_stats *)buf;

	DP_STATS_INC(vdev, tx_i.igmp_mcast_en.igmp_rcvd,
		     host_stats->igmp_mcast_en.igmp_rcvd);
	DP_STATS_INC(vdev, tx_i.igmp_mcast_en.igmp_ucast_converted,
		     host_stats->igmp_mcast_en.igmp_ucast_converted);
}

/* dp_txrx_update_vdev_host_stats(): Update stats sent through CDP
 * @soc: DP soc handle
 * @vdev_id: id of DP vdev handle
 * @buf: buffer containing specific stats structure
 * @stats_id: stats type
 *
 * Returns: QDF_STATUS
 */
static QDF_STATUS dp_txrx_update_vdev_host_stats(struct cdp_soc_t *soc_hdl,
						 uint8_t vdev_id,
						 void *buf,
						 uint16_t stats_id)
{
	struct dp_soc *soc = cdp_soc_t_to_dp_soc(soc_hdl);
	struct dp_vdev *vdev = dp_vdev_get_ref_by_id(soc, vdev_id,
						     DP_MOD_ID_CDP);

	if (!vdev) {
		dp_cdp_err("%pK: Invalid vdev handle", soc);
		return QDF_STATUS_E_FAILURE;
	}

	switch (stats_id) {
	case DP_VDEV_STATS_PKT_CNT_ONLY:
		break;
	case DP_VDEV_STATS_TX_ME:
		dp_txrx_update_vdev_me_stats(vdev, buf);
		dp_txrx_update_vdev_igmp_me_stats(vdev, buf);
		break;
	default:
		qdf_info("Invalid stats_id %d", stats_id);
		break;
	}

	dp_vdev_unref_delete(soc, vdev, DP_MOD_ID_CDP);
	return QDF_STATUS_SUCCESS;
}

/* dp_txrx_get_soc_stats - will return cdp_soc_stats
 * @soc_hdl: soc handle
 * @soc_stats: buffer to hold the values
 *
 * return: status success/failure
 */
static QDF_STATUS
dp_txrx_get_soc_stats(struct cdp_soc_t *soc_hdl,
		      struct cdp_soc_stats *soc_stats)
{
	struct dp_soc *soc = (struct dp_soc *)soc_hdl;

	soc_stats->tx.egress = soc->stats.tx.egress[0];
	soc_stats->rx.ingress = soc->stats.rx.ingress;
	soc_stats->rx.err_ring_pkts = soc->stats.rx.err_ring_pkts;
	soc_stats->rx.rx_frags = soc->stats.rx.rx_frags;
	soc_stats->rx.reo_reinject = soc->stats.rx.reo_reinject;
	soc_stats->rx.bar_frame = soc->stats.rx.bar_frame;
	soc_stats->rx.err.rx_rejected = soc->stats.rx.err.rejected;
	soc_stats->rx.err.rx_raw_frm_drop = soc->stats.rx.err.raw_frm_drop;

	return QDF_STATUS_SUCCESS;
}

#ifdef QCA_PEER_EXT_STATS
/* dp_txrx_get_peer_delay_stats - to get peer delay stats per TIDs
 * @soc: soc handle
 * @vdev_id: id of vdev handle
 * @peer_mac: mac of DP_PEER handle
 * @delay_stats: pointer to delay stats array
 * return: status success/failure
 */
static QDF_STATUS
dp_txrx_get_peer_delay_stats(struct cdp_soc_t *soc_hdl, uint8_t vdev_id,
			     uint8_t *peer_mac,
			     struct cdp_delay_tid_stats *delay_stats)
{
	struct dp_soc *soc = (struct dp_soc *)soc_hdl;
	struct dp_peer *peer = dp_peer_find_hash_find(soc, peer_mac, 0, vdev_id,
						      DP_MOD_ID_CDP);
	struct cdp_peer_ext_stats *pext_stats;
	struct cdp_delay_rx_stats *rx_delay;
	struct cdp_delay_tx_stats *tx_delay;
	uint8_t tid;

	if (!peer)
		return QDF_STATUS_E_FAILURE;

	if (!wlan_cfg_is_peer_ext_stats_enabled(soc->wlan_cfg_ctx)) {
		dp_peer_unref_delete(peer, DP_MOD_ID_CDP);
		return QDF_STATUS_E_FAILURE;
	}

	pext_stats = peer->pext_stats;
	if (!pext_stats) {
		dp_peer_unref_delete(peer, DP_MOD_ID_CDP);
		return QDF_STATUS_E_FAILURE;
	}

	for (tid = 0; tid < CDP_MAX_DATA_TIDS; tid++) {
		rx_delay = &delay_stats[tid].rx_delay;
		dp_accumulate_delay_tid_stats(soc, pext_stats->delay_stats,
					      &rx_delay->to_stack_delay, tid,
					      CDP_HIST_TYPE_REAP_STACK);
		tx_delay = &delay_stats[tid].tx_delay;
		dp_accumulate_delay_tid_stats(soc, pext_stats->delay_stats,
					      &tx_delay->tx_swq_delay, tid,
					      CDP_HIST_TYPE_SW_ENQEUE_DELAY);
		dp_accumulate_delay_tid_stats(soc, pext_stats->delay_stats,
					      &tx_delay->hwtx_delay, tid,
					      CDP_HIST_TYPE_HW_COMP_DELAY);
	}
	dp_peer_unref_delete(peer, DP_MOD_ID_CDP);

	return QDF_STATUS_SUCCESS;
}
#else
static QDF_STATUS
dp_txrx_get_peer_delay_stats(struct cdp_soc_t *soc_hdl, uint8_t vdev_id,
			     uint8_t *peer_mac,
			     struct cdp_delay_tid_stats *delay_stats)
{
	return QDF_STATUS_E_FAILURE;
}
#endif /* QCA_PEER_EXT_STATS */

#ifdef WLAN_PEER_JITTER
/* dp_txrx_get_peer_jitter_stats - to get peer jitter stats per TIDs
 * @soc: soc handle
 * @pdev_id: id of pdev handle
 * @vdev_id: id of vdev handle
 * @peer_mac: mac of DP_PEER handle
 * @tid_stats: pointer to jitter stats array
 * return: status success/failure
 */
static QDF_STATUS
dp_txrx_get_peer_jitter_stats(struct cdp_soc_t *soc_hdl, uint8_t pdev_id,
			      uint8_t vdev_id, uint8_t *peer_mac,
			      struct cdp_peer_tid_stats *tid_stats)
{
	struct dp_soc *soc = (struct dp_soc *)soc_hdl;
	struct dp_pdev *pdev = dp_get_pdev_from_soc_pdev_id_wifi3(soc, pdev_id);
	struct dp_peer *peer;
	uint8_t tid;

	if (!pdev)
		return QDF_STATUS_E_FAILURE;

	if (!wlan_cfg_get_dp_pdev_nss_enabled(pdev->wlan_cfg_ctx))
		return QDF_STATUS_E_FAILURE;

	peer = dp_peer_find_hash_find(soc, peer_mac, 0, vdev_id, DP_MOD_ID_CDP);
	if (!peer)
		return QDF_STATUS_E_FAILURE;

	for (tid = 0; tid < qdf_min(CDP_DATA_TID_MAX, DP_MAX_TIDS); tid++) {
		struct dp_rx_tid *rx_tid = &peer->rx_tid[tid];

		tid_stats[tid].tx_avg_jitter = rx_tid->stats.tx_avg_jitter;
		tid_stats[tid].tx_avg_delay = rx_tid->stats.tx_avg_delay;
		tid_stats[tid].tx_avg_err = rx_tid->stats.tx_avg_err;
		tid_stats[tid].tx_total_success =
					rx_tid->stats.tx_total_success;
		tid_stats[tid].tx_drop = rx_tid->stats.tx_drop;
	}
	dp_peer_unref_delete(peer, DP_MOD_ID_CDP);

	return QDF_STATUS_SUCCESS;
}
#else
static QDF_STATUS
dp_txrx_get_peer_jitter_stats(struct cdp_soc_t *soc_hdl, uint8_t pdev_id,
			      uint8_t vdev_id, uint8_t *peer_mac,
			      struct cdp_peer_tid_stats *tid_stats)
{
	return QDF_STATUS_E_FAILURE;
}
#endif /* WLAN_PEER_JITTER */

/* dp_txrx_get_peer_stats - will return cdp_peer_stats
 * @soc: soc handle
 * @vdev_id: id of vdev handle
 * @peer_mac: mac of DP_PEER handle
 * @peer_stats: buffer to copy to
 * return : status success/failure
 */
static QDF_STATUS
dp_txrx_get_peer_stats(struct cdp_soc_t *soc, uint8_t vdev_id,
		       uint8_t *peer_mac, struct cdp_peer_stats *peer_stats)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	struct dp_peer *peer = dp_peer_find_hash_find((struct dp_soc *)soc,
						       peer_mac, 0, vdev_id,
						       DP_MOD_ID_CDP);

	if (!peer)
		return QDF_STATUS_E_FAILURE;

	qdf_mem_copy(peer_stats, &peer->stats,
		     sizeof(struct cdp_peer_stats));

	dp_peer_unref_delete(peer, DP_MOD_ID_CDP);

	return status;
}

/* dp_txrx_get_peer_stats_param - will return specified cdp_peer_stats
 * @param soc - soc handle
 * @param vdev_id - vdev_id of vdev object
 * @param peer_mac - mac address of the peer
 * @param type - enum of required stats
 * @param buf - buffer to hold the value
 * return : status success/failure
 */
static QDF_STATUS
dp_txrx_get_peer_stats_param(struct cdp_soc_t *soc, uint8_t vdev_id,
			     uint8_t *peer_mac, enum cdp_peer_stats_type type,
			     cdp_peer_stats_param_t *buf)
{
	QDF_STATUS ret = QDF_STATUS_SUCCESS;
	struct dp_peer *peer = dp_peer_find_hash_find((struct dp_soc *)soc,
						      peer_mac, 0, vdev_id,
						      DP_MOD_ID_CDP);

	if (!peer) {
		dp_peer_err("%pK: Invalid Peer for Mac " QDF_MAC_ADDR_FMT,
			    soc, QDF_MAC_ADDR_REF(peer_mac));
		return QDF_STATUS_E_FAILURE;
	} else if (type < cdp_peer_stats_max) {
		switch (type) {
		case cdp_peer_tx_ucast:
			buf->tx_ucast = peer->stats.tx.ucast;
			break;
		case cdp_peer_tx_mcast:
			buf->tx_mcast = peer->stats.tx.mcast;
			break;
		case cdp_peer_tx_rate:
			buf->tx_rate = peer->stats.tx.tx_rate;
			break;
		case cdp_peer_tx_last_tx_rate:
			buf->last_tx_rate = peer->stats.tx.last_tx_rate;
			break;
		case cdp_peer_tx_inactive_time:
			buf->tx_inactive_time = peer->stats.tx.inactive_time;
			break;
		case cdp_peer_tx_ratecode:
			buf->tx_ratecode = peer->stats.tx.tx_ratecode;
			break;
		case cdp_peer_tx_flags:
			buf->tx_flags = peer->stats.tx.tx_flags;
			break;
		case cdp_peer_tx_power:
			buf->tx_power = peer->stats.tx.tx_power;
			break;
		case cdp_peer_rx_rate:
			buf->rx_rate = peer->stats.rx.rx_rate;
			break;
		case cdp_peer_rx_last_rx_rate:
			buf->last_rx_rate = peer->stats.rx.last_rx_rate;
			break;
		case cdp_peer_rx_ratecode:
			buf->rx_ratecode = peer->stats.rx.rx_ratecode;
			break;
		case cdp_peer_rx_ucast:
			buf->rx_ucast = peer->stats.rx.unicast;
			break;
		case cdp_peer_rx_flags:
			buf->rx_flags = peer->stats.rx.rx_flags;
			break;
		case cdp_peer_rx_avg_snr:
			buf->rx_avg_snr = peer->stats.rx.avg_snr;
			break;
		default:
			dp_peer_err("%pK: Invalid value", soc);
			ret = QDF_STATUS_E_FAILURE;
			break;
		}
	} else {
		dp_peer_err("%pK: Invalid value", soc);
		ret = QDF_STATUS_E_FAILURE;
	}

	dp_peer_unref_delete(peer, DP_MOD_ID_CDP);

	return ret;
}

/* dp_txrx_reset_peer_stats - reset cdp_peer_stats for particular peer
 * @soc: soc handle
 * @vdev_id: id of vdev handle
 * @peer_mac: mac of DP_PEER handle
 *
 * return : QDF_STATUS
 */
static QDF_STATUS
dp_txrx_reset_peer_stats(struct cdp_soc_t *soc, uint8_t vdev_id,
			 uint8_t *peer_mac)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	struct dp_peer *peer = dp_peer_find_hash_find((struct dp_soc *)soc,
						      peer_mac, 0, vdev_id,
						      DP_MOD_ID_CDP);

	if (!peer)
		return QDF_STATUS_E_FAILURE;

	qdf_mem_zero(&peer->stats, sizeof(peer->stats));

	dp_peer_unref_delete(peer, DP_MOD_ID_CDP);

	return status;
}

/* dp_txrx_get_vdev_stats - Update buffer with cdp_vdev_stats
 * @vdev_handle: DP_VDEV handle
 * @buf: buffer for vdev stats
 *
 * return : int
 */
static int dp_txrx_get_vdev_stats(struct cdp_soc_t *soc_hdl, uint8_t vdev_id,
				  void *buf, bool is_aggregate)
{
	struct dp_soc *soc = cdp_soc_t_to_dp_soc(soc_hdl);
	struct cdp_vdev_stats *vdev_stats;
	struct dp_vdev *vdev = dp_vdev_get_ref_by_id(soc, vdev_id,
						     DP_MOD_ID_CDP);

	if (!vdev)
		return 1;

	vdev_stats = (struct cdp_vdev_stats *)buf;

	if (is_aggregate) {
		dp_aggregate_vdev_stats(vdev, buf);
	} else {
		qdf_mem_copy(vdev_stats, &vdev->stats, sizeof(vdev->stats));
	}

	dp_vdev_unref_delete(soc, vdev, DP_MOD_ID_CDP);
	return 0;
}

/*
 * dp_get_total_per(): get total per
 * @soc: DP soc handle
 * @pdev_id: id of DP_PDEV handle
 *
 * Return: % error rate using retries per packet and success packets
 */
static int dp_get_total_per(struct cdp_soc_t *soc, uint8_t pdev_id)
{
	struct dp_pdev *pdev =
		dp_get_pdev_from_soc_pdev_id_wifi3((struct dp_soc *)soc,
						   pdev_id);

	if (!pdev)
		return 0;

	dp_aggregate_pdev_stats(pdev);
	if ((pdev->stats.tx.tx_success.num + pdev->stats.tx.retries) == 0)
		return 0;
	return ((pdev->stats.tx.retries * 100) /
		((pdev->stats.tx.tx_success.num) + (pdev->stats.tx.retries)));
}

/*
 * dp_txrx_stats_publish(): publish pdev stats into a buffer
 * @soc: DP soc handle
 * @pdev_id: id of DP_PDEV handle
 * @buf: to hold pdev_stats
 *
 * Return: int
 */
static int
dp_txrx_stats_publish(struct cdp_soc_t *soc, uint8_t pdev_id,
		      struct cdp_stats_extd *buf)
{
	struct cdp_txrx_stats_req req = {0,};
	struct dp_pdev *pdev =
		dp_get_pdev_from_soc_pdev_id_wifi3((struct dp_soc *)soc,
						   pdev_id);

	if (!pdev)
		return TXRX_STATS_LEVEL_OFF;

	dp_aggregate_pdev_stats(pdev);
	req.stats = (enum cdp_stats)HTT_DBG_EXT_STATS_PDEV_TX;
	req.cookie_val = DBG_STATS_COOKIE_DP_STATS;
	dp_h2t_ext_stats_msg_send(pdev, req.stats, req.param0,
				req.param1, req.param2, req.param3, 0,
				req.cookie_val, 0);

	msleep(DP_MAX_SLEEP_TIME);

	req.stats = (enum cdp_stats)HTT_DBG_EXT_STATS_PDEV_RX;
	req.cookie_val = DBG_STATS_COOKIE_DP_STATS;
	dp_h2t_ext_stats_msg_send(pdev, req.stats, req.param0,
				req.param1, req.param2, req.param3, 0,
				req.cookie_val, 0);

	msleep(DP_MAX_SLEEP_TIME);
	qdf_mem_copy(buf, &pdev->stats, sizeof(struct cdp_pdev_stats));

	return TXRX_STATS_LEVEL;
}

/**
 * dp_set_pdev_dscp_tid_map_wifi3(): update dscp tid map in pdev
 * @soc: soc handle
 * @pdev_id: id of DP_PDEV handle
 * @map_id: ID of map that needs to be updated
 * @tos: index value in map
 * @tid: tid value passed by the user
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS
dp_set_pdev_dscp_tid_map_wifi3(struct cdp_soc_t *soc_handle,
			       uint8_t pdev_id,
			       uint8_t map_id,
			       uint8_t tos, uint8_t tid)
{
	uint8_t dscp;
	struct dp_soc *soc = (struct dp_soc *)soc_handle;
	struct dp_pdev *pdev = dp_get_pdev_from_soc_pdev_id_wifi3(soc, pdev_id);

	if (!pdev)
		return QDF_STATUS_E_FAILURE;

	dscp = (tos >> DP_IP_DSCP_SHIFT) & DP_IP_DSCP_MASK;
	pdev->dscp_tid_map[map_id][dscp] = tid;

	if (map_id < soc->num_hw_dscp_tid_map)
		hal_tx_update_dscp_tid(soc->hal_soc, tid,
				       map_id, dscp);
	else
		return QDF_STATUS_E_FAILURE;

	return QDF_STATUS_SUCCESS;
}

#ifdef WLAN_SYSFS_DP_STATS
/*
 * dp_sysfs_event_trigger(): Trigger event to wait for firmware
 * stats request response.
 * @soc: soc handle
 * @cookie_val: cookie value
 *
 * @Return: QDF_STATUS
 */
static QDF_STATUS
dp_sysfs_event_trigger(struct dp_soc *soc, uint32_t cookie_val)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	/* wait for firmware response for sysfs stats request */
	if (cookie_val == DBG_SYSFS_STATS_COOKIE) {
		if (!soc) {
			dp_cdp_err("soc is NULL");
			return QDF_STATUS_E_FAILURE;
		}
		/* wait for event completion */
		status = qdf_wait_single_event(&soc->sysfs_config->sysfs_txrx_fw_request_done,
					       WLAN_SYSFS_STAT_REQ_WAIT_MS);
		if (status == QDF_STATUS_SUCCESS)
			dp_cdp_info("sysfs_txrx_fw_request_done event completed");
		else if (status == QDF_STATUS_E_TIMEOUT)
			dp_cdp_warn("sysfs_txrx_fw_request_done event expired");
		else
			dp_cdp_warn("sysfs_txrx_fw_request_done event erro code %d", status);
	}

	return status;
}
#else /* WLAN_SYSFS_DP_STATS */
/*
 * dp_sysfs_event_trigger(): Trigger event to wait for firmware
 * stats request response.
 * @soc: soc handle
 * @cookie_val: cookie value
 *
 * @Return: QDF_STATUS
 */
static QDF_STATUS
dp_sysfs_event_trigger(struct dp_soc *soc, uint32_t cookie_val)
{
	return QDF_STATUS_SUCCESS;
}
#endif /* WLAN_SYSFS_DP_STATS */

/**
 * dp_fw_stats_process(): Process TXRX FW stats request.
 * @vdev_handle: DP VDEV handle
 * @req: stats request
 *
 * return: QDF_STATUS
 */
static QDF_STATUS
dp_fw_stats_process(struct dp_vdev *vdev,
		    struct cdp_txrx_stats_req *req)
{
	struct dp_pdev *pdev = NULL;
	struct dp_soc *soc = NULL;
	uint32_t stats = req->stats;
	uint8_t mac_id = req->mac_id;
	uint32_t cookie_val = DBG_STATS_COOKIE_DEFAULT;

	if (!vdev) {
		DP_TRACE(NONE, "VDEV not found");
		return QDF_STATUS_E_FAILURE;
	}

	pdev = vdev->pdev;
	if (!pdev) {
		DP_TRACE(NONE, "PDEV not found");
		return QDF_STATUS_E_FAILURE;
	}

	soc = pdev->soc;
	if (!soc) {
		DP_TRACE(NONE, "soc not found");
		return QDF_STATUS_E_FAILURE;
	}

	/* In case request is from host sysfs for displaying stats on console */
	if (req->cookie_val == DBG_SYSFS_STATS_COOKIE)
		cookie_val = DBG_SYSFS_STATS_COOKIE;

	/*
	 * For HTT_DBG_EXT_STATS_RESET command, FW need to config
	 * from param0 to param3 according to below rule:
	 *
	 * PARAM:
	 *   - config_param0 : start_offset (stats type)
	 *   - config_param1 : stats bmask from start offset
	 *   - config_param2 : stats bmask from start offset + 32
	 *   - config_param3 : stats bmask from start offset + 64
	 */
	if (req->stats == CDP_TXRX_STATS_0) {
		req->param0 = HTT_DBG_EXT_STATS_PDEV_TX;
		req->param1 = 0xFFFFFFFF;
		req->param2 = 0xFFFFFFFF;
		req->param3 = 0xFFFFFFFF;
	} else if (req->stats == (uint8_t)HTT_DBG_EXT_STATS_PDEV_TX_MU) {
		req->param0 = HTT_DBG_EXT_STATS_SET_VDEV_MASK(vdev->vdev_id);
	}

	if (req->stats == (uint8_t)HTT_DBG_EXT_STATS_PDEV_RX_RATE_EXT) {
		dp_h2t_ext_stats_msg_send(pdev,
					  HTT_DBG_EXT_STATS_PDEV_RX_RATE_EXT,
					  req->param0, req->param1, req->param2,
					  req->param3, 0, cookie_val,
					  mac_id);
	} else {
		dp_h2t_ext_stats_msg_send(pdev, stats, req->param0,
					  req->param1, req->param2, req->param3,
					  0, cookie_val, mac_id);
	}

	dp_sysfs_event_trigger(soc, cookie_val);

	return QDF_STATUS_SUCCESS;
}

/**
 * dp_txrx_stats_request - function to map to firmware and host stats
 * @soc: soc handle
 * @vdev_id: virtual device ID
 * @req: stats request
 *
 * Return: QDF_STATUS
 */
static
QDF_STATUS dp_txrx_stats_request(struct cdp_soc_t *soc_handle,
				 uint8_t vdev_id,
				 struct cdp_txrx_stats_req *req)
{
	struct dp_soc *soc = cdp_soc_t_to_dp_soc(soc_handle);
	int host_stats;
	int fw_stats;
	enum cdp_stats stats;
	int num_stats;
	struct dp_vdev *vdev = dp_vdev_get_ref_by_id(soc, vdev_id,
						     DP_MOD_ID_CDP);
	QDF_STATUS status = QDF_STATUS_E_INVAL;

	if (!vdev || !req) {
		dp_cdp_err("%pK: Invalid vdev/req instance", soc);
		status = QDF_STATUS_E_INVAL;
		goto fail0;
	}

	if (req->mac_id >= WLAN_CFG_MAC_PER_TARGET) {
		dp_err("Invalid mac id request");
		status = QDF_STATUS_E_INVAL;
		goto fail0;
	}

	stats = req->stats;
	if (stats >= CDP_TXRX_MAX_STATS) {
		status = QDF_STATUS_E_INVAL;
		goto fail0;
	}

	/*
	 * DP_CURR_FW_STATS_AVAIL: no of FW stats currently available
	 *			has to be updated if new FW HTT stats added
	 */
	if (stats > CDP_TXRX_STATS_HTT_MAX)
		stats = stats + DP_CURR_FW_STATS_AVAIL - DP_HTT_DBG_EXT_STATS_MAX;

	num_stats  = QDF_ARRAY_SIZE(dp_stats_mapping_table);

	if (stats >= num_stats) {
		dp_cdp_err("%pK : Invalid stats option: %d", soc, stats);
		status = QDF_STATUS_E_INVAL;
		goto fail0;
	}

	req->stats = stats;
	fw_stats = dp_stats_mapping_table[stats][STATS_FW];
	host_stats = dp_stats_mapping_table[stats][STATS_HOST];

	dp_info("stats: %u fw_stats_type: %d host_stats: %d",
		stats, fw_stats, host_stats);

	if (fw_stats != TXRX_FW_STATS_INVALID) {
		/* update request with FW stats type */
		req->stats = fw_stats;
		status = dp_fw_stats_process(vdev, req);
	} else if ((host_stats != TXRX_HOST_STATS_INVALID) &&
			(host_stats <= TXRX_HOST_STATS_MAX))
		status = dp_print_host_stats(vdev, req, soc);
	else
		dp_cdp_info("%pK: Wrong Input for TxRx Stats", soc);
fail0:
	if (vdev)
		dp_vdev_unref_delete(soc, vdev, DP_MOD_ID_CDP);
	return status;
}

/*
 * dp_txrx_dump_stats() -  Dump statistics
 * @value - Statistics option
 */
static QDF_STATUS dp_txrx_dump_stats(struct cdp_soc_t *psoc, uint16_t value,
				     enum qdf_stats_verbosity_level level)
{
	struct dp_soc *soc =
		(struct dp_soc *)psoc;
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	if (!soc) {
		dp_cdp_err("%pK: soc is NULL", soc);
		return QDF_STATUS_E_INVAL;
	}

	switch (value) {
	case CDP_TXRX_PATH_STATS:
		dp_txrx_path_stats(soc);
		dp_print_soc_interrupt_stats(soc);
		hal_dump_reg_write_stats(soc->hal_soc);
		dp_pdev_print_tx_delay_stats(soc);
		break;

	case CDP_RX_RING_STATS:
		dp_print_per_ring_stats(soc);
		break;

	case CDP_TXRX_TSO_STATS:
		dp_print_tso_stats(soc, level);
		break;

	case CDP_DUMP_TX_FLOW_POOL_INFO:
		if (level == QDF_STATS_VERBOSITY_LEVEL_HIGH)
			cdp_dump_flow_pool_info((struct cdp_soc_t *)soc);
		else
			dp_tx_dump_flow_pool_info_compact(soc);
		break;

	case CDP_DP_NAPI_STATS:
		dp_print_napi_stats(soc);
		break;

	case CDP_TXRX_DESC_STATS:
		/* TODO: NOT IMPLEMENTED */
		break;

	case CDP_DP_RX_FISA_STATS:
		dp_rx_dump_fisa_stats(soc);
		break;

	case CDP_DP_SWLM_STATS:
		dp_print_swlm_stats(soc);
		break;

	case CDP_DP_TX_HW_LATENCY_STATS:
		dp_pdev_print_tx_delay_stats(soc);
		break;

	default:
		status = QDF_STATUS_E_INVAL;
		break;
	}

	return status;

}

#ifdef WLAN_SYSFS_DP_STATS
static
void dp_sysfs_get_stat_type(struct dp_soc *soc, uint32_t *mac_id,
			    uint32_t *stat_type)
{
	qdf_spinlock_acquire(&soc->sysfs_config->rw_stats_lock);
	*stat_type = soc->sysfs_config->stat_type_requested;
	*mac_id   = soc->sysfs_config->mac_id;

	qdf_spinlock_release(&soc->sysfs_config->rw_stats_lock);
}

static
void dp_sysfs_update_config_buf_params(struct dp_soc *soc,
				       uint32_t curr_len,
				       uint32_t max_buf_len,
				       char *buf)
{
	qdf_spinlock_acquire(&soc->sysfs_config->sysfs_write_user_buffer);
	/* set sysfs_config parameters */
	soc->sysfs_config->buf = buf;
	soc->sysfs_config->curr_buffer_length = curr_len;
	soc->sysfs_config->max_buffer_length = max_buf_len;
	qdf_spinlock_release(&soc->sysfs_config->sysfs_write_user_buffer);
}

static
QDF_STATUS dp_sysfs_fill_stats(ol_txrx_soc_handle soc_hdl,
			       char *buf, uint32_t buf_size)
{
	uint32_t mac_id = 0;
	uint32_t stat_type = 0;
	uint32_t fw_stats = 0;
	uint32_t host_stats = 0;
	enum cdp_stats stats;
	struct cdp_txrx_stats_req req;
	uint32_t num_stats;
	struct dp_soc *soc = NULL;

	if (!soc_hdl) {
		dp_cdp_err("%pK: soc_hdl is NULL", soc_hdl);
		return QDF_STATUS_E_INVAL;
	}

	soc = cdp_soc_t_to_dp_soc(soc_hdl);

	if (!soc) {
		dp_cdp_err("%pK: soc is NULL", soc);
		return QDF_STATUS_E_INVAL;
	}

	dp_sysfs_get_stat_type(soc, &mac_id, &stat_type);

	stats = stat_type;
	if (stats >= CDP_TXRX_MAX_STATS) {
		dp_cdp_info("sysfs stat type requested is invalid");
		return QDF_STATUS_E_INVAL;
	}
	/*
	 * DP_CURR_FW_STATS_AVAIL: no of FW stats currently available
	 *			has to be updated if new FW HTT stats added
	 */
	if (stats > CDP_TXRX_MAX_STATS)
		stats = stats + DP_CURR_FW_STATS_AVAIL - DP_HTT_DBG_EXT_STATS_MAX;

	num_stats = QDF_ARRAY_SIZE(dp_stats_mapping_table);

	if (stats >= num_stats) {
		dp_cdp_err("%pK : Invalid stats option: %d, max num stats: %d",
				soc, stats, num_stats);
		return QDF_STATUS_E_INVAL;
	}

	/* build request */
	fw_stats = dp_stats_mapping_table[stats][STATS_FW];
	host_stats = dp_stats_mapping_table[stats][STATS_HOST];

	req.stats = stat_type;
	req.mac_id = mac_id;
	/* request stats to be printed */
	qdf_mutex_acquire(&soc->sysfs_config->sysfs_read_lock);

	if (fw_stats != TXRX_FW_STATS_INVALID) {
		/* update request with FW stats type */
		req.cookie_val = DBG_SYSFS_STATS_COOKIE;
	} else if ((host_stats != TXRX_HOST_STATS_INVALID) &&
			(host_stats <= TXRX_HOST_STATS_MAX)) {
		req.cookie_val = DBG_STATS_COOKIE_DEFAULT;
		soc->sysfs_config->process_id = qdf_get_current_pid();
		soc->sysfs_config->printing_mode = PRINTING_MODE_ENABLED;
	}

	dp_sysfs_update_config_buf_params(soc, 0, buf_size, buf);

	dp_txrx_stats_request(soc_hdl, mac_id, &req);
	soc->sysfs_config->process_id = 0;
	soc->sysfs_config->printing_mode = PRINTING_MODE_DISABLED;

	dp_sysfs_update_config_buf_params(soc, 0, 0, NULL);

	qdf_mutex_release(&soc->sysfs_config->sysfs_read_lock);
	return QDF_STATUS_SUCCESS;
}

static
QDF_STATUS dp_sysfs_set_stat_type(ol_txrx_soc_handle soc_hdl,
				  uint32_t stat_type, uint32_t mac_id)
{
	struct dp_soc *soc = cdp_soc_t_to_dp_soc(soc_hdl);

	if (!soc_hdl) {
		dp_cdp_err("%pK: soc is NULL", soc);
		return QDF_STATUS_E_INVAL;
	}

	qdf_spinlock_acquire(&soc->sysfs_config->rw_stats_lock);

	soc->sysfs_config->stat_type_requested = stat_type;
	soc->sysfs_config->mac_id = mac_id;

	qdf_spinlock_release(&soc->sysfs_config->rw_stats_lock);

	return QDF_STATUS_SUCCESS;
}

static
QDF_STATUS dp_sysfs_initialize_stats(struct dp_soc *soc_hdl)
{
	struct dp_soc *soc;
	QDF_STATUS status;

	if (!soc_hdl) {
		dp_cdp_err("%pK: soc_hdl is NULL", soc_hdl);
		return QDF_STATUS_E_INVAL;
	}

	soc = soc_hdl;

	soc->sysfs_config = qdf_mem_malloc(sizeof(struct sysfs_stats_config));
	if (!soc->sysfs_config) {
		dp_cdp_err("failed to allocate memory for sysfs_config no memory");
		return QDF_STATUS_E_NOMEM;
	}

	status = qdf_event_create(&soc->sysfs_config->sysfs_txrx_fw_request_done);
	/* create event for fw stats request from sysfs */
	if (status != QDF_STATUS_SUCCESS) {
		dp_cdp_err("failed to create event sysfs_txrx_fw_request_done");
		qdf_mem_free(soc->sysfs_config);
		soc->sysfs_config = NULL;
		return QDF_STATUS_E_FAILURE;
	}

	qdf_spinlock_create(&soc->sysfs_config->rw_stats_lock);
	qdf_mutex_create(&soc->sysfs_config->sysfs_read_lock);
	qdf_spinlock_create(&soc->sysfs_config->sysfs_write_user_buffer);

	return QDF_STATUS_SUCCESS;
}

static
QDF_STATUS dp_sysfs_deinitialize_stats(struct dp_soc *soc_hdl)
{
	struct dp_soc *soc;
	QDF_STATUS status;

	if (!soc_hdl) {
		dp_cdp_err("%pK: soc_hdl is NULL", soc_hdl);
		return QDF_STATUS_E_INVAL;
	}

	soc = soc_hdl;
	if (!soc->sysfs_config) {
		dp_cdp_err("soc->sysfs_config is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	status = qdf_event_destroy(&soc->sysfs_config->sysfs_txrx_fw_request_done);
	if (status != QDF_STATUS_SUCCESS)
		dp_cdp_err("Failed to detroy event sysfs_txrx_fw_request_done ");

	qdf_mutex_destroy(&soc->sysfs_config->sysfs_read_lock);
	qdf_spinlock_destroy(&soc->sysfs_config->rw_stats_lock);
	qdf_spinlock_destroy(&soc->sysfs_config->sysfs_write_user_buffer);

	qdf_mem_free(soc->sysfs_config);

	return QDF_STATUS_SUCCESS;
}

#else /* WLAN_SYSFS_DP_STATS */

static
QDF_STATUS dp_sysfs_deinitialize_stats(struct dp_soc *soc_hdl)
{
	return QDF_STATUS_SUCCESS;
}

static
QDF_STATUS dp_sysfs_initialize_stats(struct dp_soc *soc_hdl)
{
	return QDF_STATUS_SUCCESS;
}
#endif /* WLAN_SYSFS_DP_STATS */

/**
 * dp_txrx_clear_dump_stats() - clear dumpStats
 * @soc- soc handle
 * @value - stats option
 *
 * Return: 0 - Success, non-zero - failure
 */
static
QDF_STATUS dp_txrx_clear_dump_stats(struct cdp_soc_t *soc_hdl, uint8_t pdev_id,
				    uint8_t value)
{
	struct dp_soc *soc = cdp_soc_t_to_dp_soc(soc_hdl);
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	if (!soc) {
		dp_err("soc is NULL");
		return QDF_STATUS_E_INVAL;
	}

	switch (value) {
	case CDP_TXRX_TSO_STATS:
		dp_txrx_clear_tso_stats(soc);
		break;

	case CDP_DP_TX_HW_LATENCY_STATS:
		dp_pdev_clear_tx_delay_stats(soc);
		break;

	default:
		status = QDF_STATUS_E_INVAL;
		break;
	}

	return status;
}

#ifdef QCA_LL_TX_FLOW_CONTROL_V2
/**
 * dp_update_flow_control_parameters() - API to store datapath
 *                            config parameters
 * @soc: soc handle
 * @cfg: ini parameter handle
 *
 * Return: void
 */
static inline
void dp_update_flow_control_parameters(struct dp_soc *soc,
				struct cdp_config_params *params)
{
	soc->wlan_cfg_ctx->tx_flow_stop_queue_threshold =
					params->tx_flow_stop_queue_threshold;
	soc->wlan_cfg_ctx->tx_flow_start_queue_offset =
					params->tx_flow_start_queue_offset;
}
#else
static inline
void dp_update_flow_control_parameters(struct dp_soc *soc,
				struct cdp_config_params *params)
{
}
#endif

#ifdef WLAN_FEATURE_RX_SOFTIRQ_TIME_LIMIT
/* Max packet limit for TX Comp packet loop (dp_tx_comp_handler) */
#define DP_TX_COMP_LOOP_PKT_LIMIT_MAX 1024

/* Max packet limit for RX REAP Loop (dp_rx_process) */
#define DP_RX_REAP_LOOP_PKT_LIMIT_MAX 1024

static
void dp_update_rx_soft_irq_limit_params(struct dp_soc *soc,
					struct cdp_config_params *params)
{
	soc->wlan_cfg_ctx->tx_comp_loop_pkt_limit =
				params->tx_comp_loop_pkt_limit;

	if (params->tx_comp_loop_pkt_limit < DP_TX_COMP_LOOP_PKT_LIMIT_MAX)
		soc->wlan_cfg_ctx->tx_comp_enable_eol_data_check = true;
	else
		soc->wlan_cfg_ctx->tx_comp_enable_eol_data_check = false;

	soc->wlan_cfg_ctx->rx_reap_loop_pkt_limit =
				params->rx_reap_loop_pkt_limit;

	if (params->rx_reap_loop_pkt_limit < DP_RX_REAP_LOOP_PKT_LIMIT_MAX)
		soc->wlan_cfg_ctx->rx_enable_eol_data_check = true;
	else
		soc->wlan_cfg_ctx->rx_enable_eol_data_check = false;

	soc->wlan_cfg_ctx->rx_hp_oos_update_limit =
				params->rx_hp_oos_update_limit;

	dp_info("tx_comp_loop_pkt_limit %u tx_comp_enable_eol_data_check %u rx_reap_loop_pkt_limit %u rx_enable_eol_data_check %u rx_hp_oos_update_limit %u",
		soc->wlan_cfg_ctx->tx_comp_loop_pkt_limit,
		soc->wlan_cfg_ctx->tx_comp_enable_eol_data_check,
		soc->wlan_cfg_ctx->rx_reap_loop_pkt_limit,
		soc->wlan_cfg_ctx->rx_enable_eol_data_check,
		soc->wlan_cfg_ctx->rx_hp_oos_update_limit);
}

static void dp_update_soft_irq_limits(struct dp_soc *soc, uint32_t tx_limit,
				      uint32_t rx_limit)
{
	soc->wlan_cfg_ctx->tx_comp_loop_pkt_limit = tx_limit;
	soc->wlan_cfg_ctx->rx_reap_loop_pkt_limit = rx_limit;
}

#else
static inline
void dp_update_rx_soft_irq_limit_params(struct dp_soc *soc,
					struct cdp_config_params *params)
{ }

static inline
void dp_update_soft_irq_limits(struct dp_soc *soc, uint32_t tx_limit,
			       uint32_t rx_limit)
{
}
#endif /* WLAN_FEATURE_RX_SOFTIRQ_TIME_LIMIT */

/**
 * dp_update_config_parameters() - API to store datapath
 *                            config parameters
 * @soc: soc handle
 * @cfg: ini parameter handle
 *
 * Return: status
 */
static
QDF_STATUS dp_update_config_parameters(struct cdp_soc *psoc,
				struct cdp_config_params *params)
{
	struct dp_soc *soc = (struct dp_soc *)psoc;

	if (!(soc)) {
		dp_cdp_err("%pK: Invalid handle", soc);
		return QDF_STATUS_E_INVAL;
	}

	soc->wlan_cfg_ctx->tso_enabled = params->tso_enable;
	soc->wlan_cfg_ctx->lro_enabled = params->lro_enable;
	soc->wlan_cfg_ctx->rx_hash = params->flow_steering_enable;
	soc->wlan_cfg_ctx->p2p_tcp_udp_checksumoffload =
				params->p2p_tcp_udp_checksumoffload;
	soc->wlan_cfg_ctx->nan_tcp_udp_checksumoffload =
				params->nan_tcp_udp_checksumoffload;
	soc->wlan_cfg_ctx->tcp_udp_checksumoffload =
				params->tcp_udp_checksumoffload;
	soc->wlan_cfg_ctx->napi_enabled = params->napi_enable;
	soc->wlan_cfg_ctx->ipa_enabled = params->ipa_enable;
	soc->wlan_cfg_ctx->gro_enabled = params->gro_enable;

	dp_update_rx_soft_irq_limit_params(soc, params);
	dp_update_flow_control_parameters(soc, params);

	return QDF_STATUS_SUCCESS;
}

static struct cdp_wds_ops dp_ops_wds = {
	.vdev_set_wds = dp_vdev_set_wds,
#ifdef WDS_VENDOR_EXTENSION
	.txrx_set_wds_rx_policy = dp_txrx_set_wds_rx_policy,
	.txrx_wds_peer_tx_policy_update = dp_txrx_peer_wds_tx_policy_update,
#endif
};

/*
 * dp_txrx_data_tx_cb_set(): set the callback for non standard tx
 * @soc_hdl - datapath soc handle
 * @vdev_id - virtual interface id
 * @callback - callback function
 * @ctxt: callback context
 *
 */
static void
dp_txrx_data_tx_cb_set(struct cdp_soc_t *soc_hdl, uint8_t vdev_id,
		       ol_txrx_data_tx_cb callback, void *ctxt)
{
	struct dp_soc *soc = cdp_soc_t_to_dp_soc(soc_hdl);
	struct dp_vdev *vdev = dp_vdev_get_ref_by_id(soc, vdev_id,
						     DP_MOD_ID_CDP);

	if (!vdev)
		return;

	vdev->tx_non_std_data_callback.func = callback;
	vdev->tx_non_std_data_callback.ctxt = ctxt;

	dp_vdev_unref_delete(soc, vdev, DP_MOD_ID_CDP);
}

/**
 * dp_pdev_get_dp_txrx_handle() - get dp handle from pdev
 * @soc: datapath soc handle
 * @pdev_id: id of datapath pdev handle
 *
 * Return: opaque pointer to dp txrx handle
 */
static void *dp_pdev_get_dp_txrx_handle(struct cdp_soc_t *soc, uint8_t pdev_id)
{
	struct dp_pdev *pdev =
		dp_get_pdev_from_soc_pdev_id_wifi3((struct dp_soc *)soc,
						   pdev_id);
	if (qdf_unlikely(!pdev))
		return NULL;

	return pdev->dp_txrx_handle;
}

/**
 * dp_pdev_set_dp_txrx_handle() - set dp handle in pdev
 * @soc: datapath soc handle
 * @pdev_id: id of datapath pdev handle
 * @dp_txrx_hdl: opaque pointer for dp_txrx_handle
 *
 * Return: void
 */
static void
dp_pdev_set_dp_txrx_handle(struct cdp_soc_t *soc, uint8_t pdev_id,
			   void *dp_txrx_hdl)
{
	struct dp_pdev *pdev =
		dp_get_pdev_from_soc_pdev_id_wifi3((struct dp_soc *)soc,
						   pdev_id);

	if (!pdev)
		return;

	pdev->dp_txrx_handle = dp_txrx_hdl;
}

/**
 * dp_vdev_get_dp_ext_handle() - get dp handle from vdev
 * @soc: datapath soc handle
 * @vdev_id: vdev id
 *
 * Return: opaque pointer to dp txrx handle
 */
static void *dp_vdev_get_dp_ext_handle(ol_txrx_soc_handle soc_hdl,
				       uint8_t vdev_id)
{
	struct dp_soc *soc = cdp_soc_t_to_dp_soc(soc_hdl);
	struct dp_vdev *vdev = dp_vdev_get_ref_by_id(soc, vdev_id,
						     DP_MOD_ID_CDP);
	void *dp_ext_handle;

	if (!vdev)
		return NULL;
	dp_ext_handle = vdev->vdev_dp_ext_handle;

	dp_vdev_unref_delete(soc, vdev, DP_MOD_ID_CDP);
	return dp_ext_handle;
}

/**
 * dp_vdev_set_dp_ext_handle() - set dp handle in vdev
 * @soc: datapath soc handle
 * @vdev_id: vdev id
 * @size: size of advance dp handle
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS
dp_vdev_set_dp_ext_handle(ol_txrx_soc_handle soc_hdl, uint8_t vdev_id,
			  uint16_t size)
{
	struct dp_soc *soc = cdp_soc_t_to_dp_soc(soc_hdl);
	struct dp_vdev *vdev = dp_vdev_get_ref_by_id(soc, vdev_id,
						     DP_MOD_ID_CDP);
	void *dp_ext_handle;

	if (!vdev)
		return QDF_STATUS_E_FAILURE;

	dp_ext_handle = qdf_mem_malloc(size);

	if (!dp_ext_handle) {
		dp_vdev_unref_delete(soc, vdev, DP_MOD_ID_CDP);
		return QDF_STATUS_E_FAILURE;
	}

	vdev->vdev_dp_ext_handle = dp_ext_handle;

	dp_vdev_unref_delete(soc, vdev, DP_MOD_ID_CDP);
	return QDF_STATUS_SUCCESS;
}

/**
 * dp_vdev_inform_ll_conn() - Inform vdev to add/delete a latency critical
 *			      connection for this vdev
 * @soc_hdl: CDP soc handle
 * @vdev_id: vdev ID
 * @action: Add/Delete action
 *
 * Returns: QDF_STATUS.
 */
static QDF_STATUS
dp_vdev_inform_ll_conn(struct cdp_soc_t *soc_hdl, uint8_t vdev_id,
		       enum vdev_ll_conn_actions action)
{
	struct dp_soc *soc = cdp_soc_t_to_dp_soc(soc_hdl);
	struct dp_vdev *vdev = dp_vdev_get_ref_by_id(soc, vdev_id,
						     DP_MOD_ID_CDP);

	if (!vdev) {
		dp_err("LL connection action for invalid vdev %d", vdev_id);
		return QDF_STATUS_E_FAILURE;
	}

	switch (action) {
	case CDP_VDEV_LL_CONN_ADD:
		vdev->num_latency_critical_conn++;
		break;

	case CDP_VDEV_LL_CONN_DEL:
		vdev->num_latency_critical_conn--;
		break;

	default:
		dp_err("LL connection action invalid %d", action);
		break;
	}

	dp_vdev_unref_delete(soc, vdev, DP_MOD_ID_CDP);
	return QDF_STATUS_SUCCESS;
}

#ifdef WLAN_DP_FEATURE_SW_LATENCY_MGR
/**
 * dp_soc_set_swlm_enable() - Enable/Disable SWLM if initialized.
 * @soc_hdl: CDP Soc handle
 * @value: Enable/Disable value
 *
 * Returns: QDF_STATUS
 */
static QDF_STATUS dp_soc_set_swlm_enable(struct cdp_soc_t *soc_hdl,
					 uint8_t value)
{
	struct dp_soc *soc = cdp_soc_t_to_dp_soc(soc_hdl);

	if (!soc->swlm.is_init) {
		dp_err("SWLM is not initialized");
		return QDF_STATUS_E_FAILURE;
	}

	soc->swlm.is_enabled = !!value;

	return QDF_STATUS_SUCCESS;
}

/**
 * dp_soc_is_swlm_enabled() - Check if SWLM is enabled.
 * @soc_hdl: CDP Soc handle
 *
 * Returns: QDF_STATUS
 */
static uint8_t dp_soc_is_swlm_enabled(struct cdp_soc_t *soc_hdl)
{
	struct dp_soc *soc = cdp_soc_t_to_dp_soc(soc_hdl);

	return soc->swlm.is_enabled;
}
#endif

/**
 * dp_display_srng_info() - Dump the srng HP TP info
 * @soc_hdl: CDP Soc handle
 *
 * This function dumps the SW hp/tp values for the important rings.
 * HW hp/tp values are not being dumped, since it can lead to
 * READ NOC error when UMAC is in low power state. MCC does not have
 * device force wake working yet.
 *
 * Return: none
 */
static void dp_display_srng_info(struct cdp_soc_t *soc_hdl)
{
	struct dp_soc *soc = cdp_soc_t_to_dp_soc(soc_hdl);
	hal_soc_handle_t hal_soc = soc->hal_soc;
	uint32_t hp, tp, i;

	dp_info("SRNG HP-TP data:");
	for (i = 0; i < soc->num_tcl_data_rings; i++) {
		hal_get_sw_hptp(hal_soc, soc->tcl_data_ring[i].hal_srng,
				&tp, &hp);
		dp_info("TCL DATA ring[%d]: hp=0x%x, tp=0x%x", i, hp, tp);

		if (wlan_cfg_get_wbm_ring_num_for_index(soc->wlan_cfg_ctx, i) ==
		    INVALID_WBM_RING_NUM)
			continue;

		hal_get_sw_hptp(hal_soc, soc->tx_comp_ring[i].hal_srng,
				&tp, &hp);
		dp_info("TX comp ring[%d]: hp=0x%x, tp=0x%x", i, hp, tp);
	}

	for (i = 0; i < soc->num_reo_dest_rings; i++) {
		hal_get_sw_hptp(hal_soc, soc->reo_dest_ring[i].hal_srng,
				&tp, &hp);
		dp_info("REO DST ring[%d]: hp=0x%x, tp=0x%x", i, hp, tp);
	}

	hal_get_sw_hptp(hal_soc, soc->reo_exception_ring.hal_srng, &tp, &hp);
	dp_info("REO exception ring: hp=0x%x, tp=0x%x", hp, tp);

	hal_get_sw_hptp(hal_soc, soc->rx_rel_ring.hal_srng, &tp, &hp);
	dp_info("WBM RX release ring: hp=0x%x, tp=0x%x", hp, tp);

	hal_get_sw_hptp(hal_soc, soc->wbm_desc_rel_ring.hal_srng, &tp, &hp);
	dp_info("WBM desc release ring: hp=0x%x, tp=0x%x", hp, tp);
}

/**
 * dp_soc_get_dp_txrx_handle() - get context for external-dp from dp soc
 * @soc_handle: datapath soc handle
 *
 * Return: opaque pointer to external dp (non-core DP)
 */
static void *dp_soc_get_dp_txrx_handle(struct cdp_soc *soc_handle)
{
	struct dp_soc *soc = (struct dp_soc *)soc_handle;

	return soc->external_txrx_handle;
}

/**
 * dp_soc_set_dp_txrx_handle() - set external dp handle in soc
 * @soc_handle: datapath soc handle
 * @txrx_handle: opaque pointer to external dp (non-core DP)
 *
 * Return: void
 */
static void
dp_soc_set_dp_txrx_handle(struct cdp_soc *soc_handle, void *txrx_handle)
{
	struct dp_soc *soc = (struct dp_soc *)soc_handle;

	soc->external_txrx_handle = txrx_handle;
}

/**
 * dp_soc_map_pdev_to_lmac() - Save pdev_id to lmac_id mapping
 * @soc_hdl: datapath soc handle
 * @pdev_id: id of the datapath pdev handle
 * @lmac_id: lmac id
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS
dp_soc_map_pdev_to_lmac
	(struct cdp_soc_t *soc_hdl, uint8_t pdev_id,
	 uint32_t lmac_id)
{
	struct dp_soc *soc = (struct dp_soc *)soc_hdl;

	wlan_cfg_set_hw_mac_idx(soc->wlan_cfg_ctx,
				pdev_id,
				lmac_id);

	/*Set host PDEV ID for lmac_id*/
	wlan_cfg_set_pdev_idx(soc->wlan_cfg_ctx,
			      pdev_id,
			      lmac_id);

	return QDF_STATUS_SUCCESS;
}

/**
 * dp_soc_handle_pdev_mode_change() - Update pdev to lmac mapping
 * @soc_hdl: datapath soc handle
 * @pdev_id: id of the datapath pdev handle
 * @lmac_id: lmac id
 *
 * In the event of a dynamic mode change, update the pdev to lmac mapping
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS
dp_soc_handle_pdev_mode_change
	(struct cdp_soc_t *soc_hdl, uint8_t pdev_id,
	 uint32_t lmac_id)
{
	struct dp_soc *soc = (struct dp_soc *)soc_hdl;
	struct dp_vdev *vdev = NULL;
	uint8_t hw_pdev_id, mac_id;
	struct dp_pdev *pdev = dp_get_pdev_from_soc_pdev_id_wifi3(soc,
								  pdev_id);
	int nss_config = wlan_cfg_get_dp_soc_nss_cfg(soc->wlan_cfg_ctx);

	if (qdf_unlikely(!pdev))
		return QDF_STATUS_E_FAILURE;

	pdev->lmac_id = lmac_id;
	pdev->target_pdev_id =
		dp_calculate_target_pdev_id_from_host_pdev_id(soc, pdev_id);
	dp_info(" mode change %d %d\n", pdev->pdev_id, pdev->lmac_id);

	/*Set host PDEV ID for lmac_id*/
	wlan_cfg_set_pdev_idx(soc->wlan_cfg_ctx,
			      pdev->pdev_id,
			      lmac_id);

	hw_pdev_id =
		dp_get_target_pdev_id_for_host_pdev_id(soc,
						       pdev->pdev_id);

	/*
	 * When NSS offload is enabled, send pdev_id->lmac_id
	 * and pdev_id to hw_pdev_id to NSS FW
	 */
	if (nss_config) {
		mac_id = pdev->lmac_id;
		if (soc->cdp_soc.ol_ops->pdev_update_lmac_n_target_pdev_id)
			soc->cdp_soc.ol_ops->
				pdev_update_lmac_n_target_pdev_id(
				soc->ctrl_psoc,
				&pdev_id, &mac_id, &hw_pdev_id);
	}

	qdf_spin_lock_bh(&pdev->vdev_list_lock);
	TAILQ_FOREACH(vdev, &pdev->vdev_list, vdev_list_elem) {
		HTT_TX_TCL_METADATA_PDEV_ID_SET(vdev->htt_tcl_metadata,
						hw_pdev_id);
		vdev->lmac_id = pdev->lmac_id;
	}
	qdf_spin_unlock_bh(&pdev->vdev_list_lock);

	return QDF_STATUS_SUCCESS;
}

/**
 * dp_soc_set_pdev_status_down() - set pdev down/up status
 * @soc: datapath soc handle
 * @pdev_id: id of datapath pdev handle
 * @is_pdev_down: pdev down/up status
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS
dp_soc_set_pdev_status_down(struct cdp_soc_t *soc, uint8_t pdev_id,
			    bool is_pdev_down)
{
	struct dp_pdev *pdev =
		dp_get_pdev_from_soc_pdev_id_wifi3((struct dp_soc *)soc,
						   pdev_id);
	if (!pdev)
		return QDF_STATUS_E_FAILURE;

	pdev->is_pdev_down = is_pdev_down;
	return QDF_STATUS_SUCCESS;
}

/**
 * dp_get_cfg_capabilities() - get dp capabilities
 * @soc_handle: datapath soc handle
 * @dp_caps: enum for dp capabilities
 *
 * Return: bool to determine if dp caps is enabled
 */
static bool
dp_get_cfg_capabilities(struct cdp_soc_t *soc_handle,
			enum cdp_capabilities dp_caps)
{
	struct dp_soc *soc = (struct dp_soc *)soc_handle;

	return wlan_cfg_get_dp_caps(soc->wlan_cfg_ctx, dp_caps);
}

#ifdef FEATURE_AST
static QDF_STATUS
dp_peer_teardown_wifi3(struct cdp_soc_t *soc_hdl, uint8_t vdev_id,
		       uint8_t *peer_mac)
{
	struct dp_soc *soc = (struct dp_soc *)soc_hdl;
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	struct dp_peer *peer =
			dp_peer_find_hash_find(soc, peer_mac, 0, vdev_id,
					       DP_MOD_ID_CDP);

	/* Peer can be null for monitor vap mac address */
	if (!peer) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_DEBUG,
			  "%s: Invalid peer\n", __func__);
		return QDF_STATUS_E_FAILURE;
	}

	dp_peer_update_state(soc, peer, DP_PEER_STATE_LOGICAL_DELETE);

	qdf_spin_lock_bh(&soc->ast_lock);
	dp_peer_delete_ast_entries(soc, peer);
	qdf_spin_unlock_bh(&soc->ast_lock);

	dp_peer_unref_delete(peer, DP_MOD_ID_CDP);
	return status;
}
#endif

#ifndef WLAN_SUPPORT_RX_TAG_STATISTICS
/**
 * dp_dump_pdev_rx_protocol_tag_stats - dump the number of packets tagged for
 * given protocol type (RX_PROTOCOL_TAG_ALL indicates for all protocol)
 * @soc: cdp_soc handle
 * @pdev_id: id of cdp_pdev handle
 * @protocol_type: protocol type for which stats should be displayed
 *
 * Return: none
 */
static inline void
dp_dump_pdev_rx_protocol_tag_stats(struct cdp_soc_t  *soc, uint8_t pdev_id,
				   uint16_t protocol_type)
{
}
#endif /* WLAN_SUPPORT_RX_TAG_STATISTICS */

#ifndef WLAN_SUPPORT_RX_PROTOCOL_TYPE_TAG
/**
 * dp_update_pdev_rx_protocol_tag - Add/remove a protocol tag that should be
 * applied to the desired protocol type packets
 * @soc: soc handle
 * @pdev_id: id of cdp_pdev handle
 * @enable_rx_protocol_tag - bitmask that indicates what protocol types
 * are enabled for tagging. zero indicates disable feature, non-zero indicates
 * enable feature
 * @protocol_type: new protocol type for which the tag is being added
 * @tag: user configured tag for the new protocol
 *
 * Return: Success
 */
static inline QDF_STATUS
dp_update_pdev_rx_protocol_tag(struct cdp_soc_t  *soc, uint8_t pdev_id,
			       uint32_t enable_rx_protocol_tag,
			       uint16_t protocol_type,
			       uint16_t tag)
{
	return QDF_STATUS_SUCCESS;
}
#endif /* WLAN_SUPPORT_RX_PROTOCOL_TYPE_TAG */

#ifndef WLAN_SUPPORT_RX_FLOW_TAG
/**
 * dp_set_rx_flow_tag - add/delete a flow
 * @soc: soc handle
 * @pdev_id: id of cdp_pdev handle
 * @flow_info: flow tuple that is to be added to/deleted from flow search table
 *
 * Return: Success
 */
static inline QDF_STATUS
dp_set_rx_flow_tag(struct cdp_soc_t *cdp_soc, uint8_t pdev_id,
		   struct cdp_rx_flow_info *flow_info)
{
	return QDF_STATUS_SUCCESS;
}
/**
 * dp_dump_rx_flow_tag_stats - dump the number of packets tagged for
 * given flow 5-tuple
 * @cdp_soc: soc handle
 * @pdev_id: id of cdp_pdev handle
 * @flow_info: flow 5-tuple for which stats should be displayed
 *
 * Return: Success
 */
static inline QDF_STATUS
dp_dump_rx_flow_tag_stats(struct cdp_soc_t *cdp_soc, uint8_t pdev_id,
			  struct cdp_rx_flow_info *flow_info)
{
	return QDF_STATUS_SUCCESS;
}
#endif /* WLAN_SUPPORT_RX_FLOW_TAG */

static QDF_STATUS dp_peer_map_attach_wifi3(struct cdp_soc_t  *soc_hdl,
					   uint32_t max_peers,
					   uint32_t max_ast_index,
					   uint8_t peer_map_unmap_versions)
{
	struct dp_soc *soc = (struct dp_soc *)soc_hdl;
	QDF_STATUS status;

	soc->max_peers = max_peers;

	wlan_cfg_set_max_ast_idx(soc->wlan_cfg_ctx, max_ast_index);

	status = soc->arch_ops.txrx_peer_map_attach(soc);
	if (!QDF_IS_STATUS_SUCCESS(status)) {
		dp_err("failure in allocating peer tables");
		return QDF_STATUS_E_FAILURE;
	}

	dp_info("max_peers %u, calculated max_peers %u max_ast_index: %u\n",
		max_peers, soc->max_peer_id, max_ast_index);

	status = dp_peer_find_attach(soc);
	if (!QDF_IS_STATUS_SUCCESS(status)) {
		dp_err("Peer find attach failure");
		goto fail;
	}

	soc->peer_map_unmap_versions = peer_map_unmap_versions;
	soc->peer_map_attach_success = TRUE;

	return QDF_STATUS_SUCCESS;
fail:
	soc->arch_ops.txrx_peer_map_detach(soc);

	return status;
}

static QDF_STATUS dp_soc_set_param(struct cdp_soc_t  *soc_hdl,
				   enum cdp_soc_param_t param,
				   uint32_t value)
{
	struct dp_soc *soc = (struct dp_soc *)soc_hdl;

	switch (param) {
	case DP_SOC_PARAM_MSDU_EXCEPTION_DESC:
		soc->num_msdu_exception_desc = value;
		dp_info("num_msdu exception_desc %u",
			value);
		break;
	case DP_SOC_PARAM_CMEM_FSE_SUPPORT:
		if (wlan_cfg_is_fst_in_cmem_enabled(soc->wlan_cfg_ctx))
			soc->fst_in_cmem = !!value;
		dp_info("FW supports CMEM FSE %u", value);
		break;
	case DP_SOC_PARAM_MAX_AST_AGEOUT:
		soc->max_ast_ageout_count = value;
		dp_info("Max ast ageout count %u", soc->max_ast_ageout_count);
		break;
	case DP_SOC_PARAM_EAPOL_OVER_CONTROL_PORT:
		soc->eapol_over_control_port = value;
		dp_info("Eapol over control_port:%d",
			soc->eapol_over_control_port);
		break;
	default:
		dp_info("not handled param %d ", param);
		break;
	}

	return QDF_STATUS_SUCCESS;
}

static void dp_soc_set_rate_stats_ctx(struct cdp_soc_t *soc_handle,
				      void *stats_ctx)
{
	struct dp_soc *soc = (struct dp_soc *)soc_handle;

	soc->rate_stats_ctx = (struct cdp_soc_rate_stats_ctx *)stats_ctx;
}

#if defined(FEATURE_PERPKT_INFO) && WDI_EVENT_ENABLE
/**
 * dp_peer_flush_rate_stats_req(): Flush peer rate stats
 * @soc: Datapath SOC handle
 * @peer: Datapath peer
 * @arg: argument to iter function
 *
 * Return: QDF_STATUS
 */
static void
dp_peer_flush_rate_stats_req(struct dp_soc *soc, struct dp_peer *peer,
			     void *arg)
{
	if (peer->bss_peer)
		return;

	dp_wdi_event_handler(
		WDI_EVENT_FLUSH_RATE_STATS_REQ,
		soc, peer->rdkstats_ctx,
		peer->peer_id,
		WDI_NO_VAL, peer->vdev->pdev->pdev_id);
}

/**
 * dp_flush_rate_stats_req(): Flush peer rate stats in pdev
 * @soc_hdl: Datapath SOC handle
 * @pdev_id: pdev_id
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS dp_flush_rate_stats_req(struct cdp_soc_t *soc_hdl,
					  uint8_t pdev_id)
{
	struct dp_soc *soc = (struct dp_soc *)soc_hdl;
	struct dp_pdev *pdev =
		dp_get_pdev_from_soc_pdev_id_wifi3((struct dp_soc *)soc,
						   pdev_id);
	if (!pdev)
		return QDF_STATUS_E_FAILURE;

	dp_pdev_iterate_peer(pdev, dp_peer_flush_rate_stats_req, NULL,
			     DP_MOD_ID_CDP);

	return QDF_STATUS_SUCCESS;
}
#else
static inline QDF_STATUS
dp_flush_rate_stats_req(struct cdp_soc_t *soc_hdl,
			uint8_t pdev_id)
{
	return QDF_STATUS_SUCCESS;
}
#endif

static void *dp_peer_get_rdkstats_ctx(struct cdp_soc_t *soc_hdl,
				      uint8_t vdev_id,
				      uint8_t *mac_addr)
{
	struct dp_soc *soc = (struct dp_soc *)soc_hdl;
	struct dp_peer *peer;
	void *rdkstats_ctx = NULL;

	if (mac_addr) {
		peer = dp_peer_find_hash_find(soc, mac_addr,
					      0, vdev_id,
					      DP_MOD_ID_CDP);
		if (!peer)
			return NULL;

		rdkstats_ctx = peer->rdkstats_ctx;

		dp_peer_unref_delete(peer, DP_MOD_ID_CDP);
	}

	return rdkstats_ctx;
}

#if defined(FEATURE_PERPKT_INFO) && WDI_EVENT_ENABLE
static QDF_STATUS dp_peer_flush_rate_stats(struct cdp_soc_t *soc,
					   uint8_t pdev_id,
					   void *buf)
{
	 dp_wdi_event_handler(WDI_EVENT_PEER_FLUSH_RATE_STATS,
			      (struct dp_soc *)soc, buf, HTT_INVALID_PEER,
			      WDI_NO_VAL, pdev_id);
	return QDF_STATUS_SUCCESS;
}
#else
static inline QDF_STATUS
dp_peer_flush_rate_stats(struct cdp_soc_t *soc,
			 uint8_t pdev_id,
			 void *buf)
{
	return QDF_STATUS_SUCCESS;
}
#endif

static void *dp_soc_get_rate_stats_ctx(struct cdp_soc_t *soc_handle)
{
	struct dp_soc *soc = (struct dp_soc *)soc_handle;

	return soc->rate_stats_ctx;
}

/*
 * dp_get_cfg() - get dp cfg
 * @soc: cdp soc handle
 * @cfg: cfg enum
 *
 * Return: cfg value
 */
static uint32_t dp_get_cfg(struct cdp_soc_t *soc, enum cdp_dp_cfg cfg)
{
	struct dp_soc *dpsoc = (struct dp_soc *)soc;
	uint32_t value = 0;

	switch (cfg) {
	case cfg_dp_enable_data_stall:
		value = dpsoc->wlan_cfg_ctx->enable_data_stall_detection;
		break;
	case cfg_dp_enable_p2p_ip_tcp_udp_checksum_offload:
		value = dpsoc->wlan_cfg_ctx->p2p_tcp_udp_checksumoffload;
		break;
	case cfg_dp_enable_nan_ip_tcp_udp_checksum_offload:
		value = dpsoc->wlan_cfg_ctx->nan_tcp_udp_checksumoffload;
		break;
	case cfg_dp_enable_ip_tcp_udp_checksum_offload:
		value = dpsoc->wlan_cfg_ctx->tcp_udp_checksumoffload;
		break;
	case cfg_dp_disable_legacy_mode_csum_offload:
		value = dpsoc->wlan_cfg_ctx->
					legacy_mode_checksumoffload_disable;
		break;
	case cfg_dp_tso_enable:
		value = dpsoc->wlan_cfg_ctx->tso_enabled;
		break;
	case cfg_dp_lro_enable:
		value = dpsoc->wlan_cfg_ctx->lro_enabled;
		break;
	case cfg_dp_gro_enable:
		value = dpsoc->wlan_cfg_ctx->gro_enabled;
		break;
	case cfg_dp_tc_based_dyn_gro_enable:
		value = dpsoc->wlan_cfg_ctx->tc_based_dynamic_gro;
		break;
	case cfg_dp_tc_ingress_prio:
		value = dpsoc->wlan_cfg_ctx->tc_ingress_prio;
		break;
	case cfg_dp_sg_enable:
		value = dpsoc->wlan_cfg_ctx->sg_enabled;
		break;
	case cfg_dp_tx_flow_start_queue_offset:
		value = dpsoc->wlan_cfg_ctx->tx_flow_start_queue_offset;
		break;
	case cfg_dp_tx_flow_stop_queue_threshold:
		value = dpsoc->wlan_cfg_ctx->tx_flow_stop_queue_threshold;
		break;
	case cfg_dp_disable_intra_bss_fwd:
		value = dpsoc->wlan_cfg_ctx->disable_intra_bss_fwd;
		break;
	case cfg_dp_pktlog_buffer_size:
		value = dpsoc->wlan_cfg_ctx->pktlog_buffer_size;
		break;
	case cfg_dp_wow_check_rx_pending:
		value = dpsoc->wlan_cfg_ctx->wow_check_rx_pending_enable;
		break;
	default:
		value =  0;
	}

	return value;
}

#ifdef PEER_FLOW_CONTROL
/**
 * dp_tx_flow_ctrl_configure_pdev() - Configure flow control params
 * @soc_handle: datapath soc handle
 * @pdev_id: id of datapath pdev handle
 * @param: ol ath params
 * @value: value of the flag
 * @buff: Buffer to be passed
 *
 * Implemented this function same as legacy function. In legacy code, single
 * function is used to display stats and update pdev params.
 *
 * Return: 0 for success. nonzero for failure.
 */
static uint32_t dp_tx_flow_ctrl_configure_pdev(struct cdp_soc_t *soc_handle,
					       uint8_t pdev_id,
					       enum _dp_param_t param,
					       uint32_t value, void *buff)
{
	struct dp_soc *soc = (struct dp_soc *)soc_handle;
	struct dp_pdev *pdev =
		dp_get_pdev_from_soc_pdev_id_wifi3((struct dp_soc *)soc,
						   pdev_id);

	if (qdf_unlikely(!pdev))
		return 1;

	soc = pdev->soc;
	if (!soc)
		return 1;

	switch (param) {
#ifdef QCA_ENH_V3_STATS_SUPPORT
	case DP_PARAM_VIDEO_DELAY_STATS_FC:
		if (value)
			pdev->delay_stats_flag = true;
		else
			pdev->delay_stats_flag = false;
		break;
	case DP_PARAM_VIDEO_STATS_FC:
		qdf_print("------- TID Stats ------\n");
		dp_pdev_print_tid_stats(pdev);
		qdf_print("------ Delay Stats ------\n");
		dp_pdev_print_delay_stats(pdev);
		qdf_print("------ Rx Error Stats ------\n");
		dp_pdev_print_rx_error_stats(pdev);
		break;
#endif
	case DP_PARAM_TOTAL_Q_SIZE:
		{
			uint32_t tx_min, tx_max;

			tx_min = wlan_cfg_get_min_tx_desc(soc->wlan_cfg_ctx);
			tx_max = wlan_cfg_get_num_tx_desc(soc->wlan_cfg_ctx);

			if (!buff) {
				if ((value >= tx_min) && (value <= tx_max)) {
					pdev->num_tx_allowed = value;
				} else {
					dp_tx_info("%pK: Failed to update num_tx_allowed, Q_min = %d Q_max = %d",
						   soc, tx_min, tx_max);
					break;
				}
			} else {
				*(int *)buff = pdev->num_tx_allowed;
			}
		}
		break;
	default:
		dp_tx_info("%pK: not handled param %d ", soc, param);
		break;
	}

	return 0;
}
#endif

/**
 * dp_set_pdev_pcp_tid_map_wifi3(): update pcp tid map in pdev
 * @psoc: dp soc handle
 * @pdev_id: id of DP_PDEV handle
 * @pcp: pcp value
 * @tid: tid value passed by the user
 *
 * Return: QDF_STATUS_SUCCESS on success
 */
static QDF_STATUS dp_set_pdev_pcp_tid_map_wifi3(ol_txrx_soc_handle psoc,
						uint8_t pdev_id,
						uint8_t pcp, uint8_t tid)
{
	struct dp_soc *soc = (struct dp_soc *)psoc;

	soc->pcp_tid_map[pcp] = tid;

	hal_tx_update_pcp_tid_map(soc->hal_soc, pcp, tid);
	return QDF_STATUS_SUCCESS;
}

/**
 * dp_set_vdev_pcp_tid_map_wifi3(): update pcp tid map in vdev
 * @soc: DP soc handle
 * @vdev_id: id of DP_VDEV handle
 * @pcp: pcp value
 * @tid: tid value passed by the user
 *
 * Return: QDF_STATUS_SUCCESS on success
 */
static QDF_STATUS dp_set_vdev_pcp_tid_map_wifi3(struct cdp_soc_t *soc_hdl,
						uint8_t vdev_id,
						uint8_t pcp, uint8_t tid)
{
	struct dp_soc *soc = cdp_soc_t_to_dp_soc(soc_hdl);
	struct dp_vdev *vdev = dp_vdev_get_ref_by_id(soc, vdev_id,
						     DP_MOD_ID_CDP);

	if (!vdev)
		return QDF_STATUS_E_FAILURE;

	vdev->pcp_tid_map[pcp] = tid;

	dp_vdev_unref_delete(soc, vdev, DP_MOD_ID_CDP);
	return QDF_STATUS_SUCCESS;
}

#if defined(FEATURE_RUNTIME_PM) || defined(DP_POWER_SAVE)
static void dp_drain_txrx(struct cdp_soc_t *soc_handle)
{
	struct dp_soc *soc = (struct dp_soc *)soc_handle;
	uint32_t cur_tx_limit, cur_rx_limit;
	uint32_t budget = 0xffff;
	uint32_t val;
	int i;

	cur_tx_limit = soc->wlan_cfg_ctx->tx_comp_loop_pkt_limit;
	cur_rx_limit = soc->wlan_cfg_ctx->rx_reap_loop_pkt_limit;

	/* Temporarily increase soft irq limits when going to drain
	 * the UMAC/LMAC SRNGs and restore them after polling.
	 * Though the budget is on higher side, the TX/RX reaping loops
	 * will not execute longer as both TX and RX would be suspended
	 * by the time this API is called.
	 */
	dp_update_soft_irq_limits(soc, budget, budget);

	for (i = 0; i < wlan_cfg_get_num_contexts(soc->wlan_cfg_ctx); i++)
		dp_service_srngs(&soc->intr_ctx[i], budget);

	dp_update_soft_irq_limits(soc, cur_tx_limit, cur_rx_limit);

	/* Do a dummy read at offset 0; this will ensure all
	 * pendings writes(HP/TP) are flushed before read returns.
	 */
	val = HAL_REG_READ((struct hal_soc *)soc->hal_soc, 0);
	dp_debug("Register value at offset 0: %u\n", val);
}
#endif

#ifdef WLAN_FEATURE_PKT_CAPTURE_V2
static void
dp_set_pkt_capture_mode(struct cdp_soc_t *soc_handle, bool val)
{
	struct dp_soc *soc = (struct dp_soc *)soc_handle;

	soc->wlan_cfg_ctx->pkt_capture_mode = val;
}
#endif

#ifdef HW_TX_DELAY_STATS_ENABLE
/**
 * dp_enable_disable_vdev_tx_delay_stats(): Start/Stop tx delay stats capture
 * @soc: DP soc handle
 * @vdev_id: vdev id
 * @value: value
 *
 * Return: None
 */
static void
dp_enable_disable_vdev_tx_delay_stats(struct cdp_soc_t *soc_hdl,
				      uint8_t vdev_id,
				      uint8_t value)
{
	struct dp_soc *soc = cdp_soc_t_to_dp_soc(soc_hdl);
	struct dp_vdev *vdev = NULL;

	vdev = dp_vdev_get_ref_by_id(soc, vdev_id, DP_MOD_ID_CDP);
	if (!vdev)
		return;

	vdev->hw_tx_delay_stats_enabled = value;

	dp_vdev_unref_delete(soc, vdev, DP_MOD_ID_CDP);
}

/**
 * dp_check_vdev_tx_delay_stats_enabled() - check the feature is enabled or not
 * @soc: DP soc handle
 * @vdev_id: vdev id
 *
 * Returns: 1 if enabled, 0 if disabled
 */
static uint8_t
dp_check_vdev_tx_delay_stats_enabled(struct cdp_soc_t *soc_hdl,
				     uint8_t vdev_id)
{
	struct dp_soc *soc = cdp_soc_t_to_dp_soc(soc_hdl);
	struct dp_vdev *vdev;
	uint8_t ret_val = 0;

	vdev = dp_vdev_get_ref_by_id(soc, vdev_id, DP_MOD_ID_CDP);
	if (!vdev)
		return ret_val;

	ret_val = vdev->hw_tx_delay_stats_enabled;
	dp_vdev_unref_delete(soc, vdev, DP_MOD_ID_CDP);

	return ret_val;
}
#endif

/**
 * dp_set_tx_pause() - Pause or resume tx path
 * @soc_hdl: Datapath soc handle
 * @flag: set or clear is_tx_pause
 *
 * Return: None.
 */
static inline
void dp_set_tx_pause(struct cdp_soc_t *soc_hdl, bool flag)
{
	struct dp_soc *soc = cdp_soc_t_to_dp_soc(soc_hdl);

	soc->is_tx_pause = flag;
}

static struct cdp_cmn_ops dp_ops_cmn = {
	.txrx_soc_attach_target = dp_soc_attach_target_wifi3,
	.txrx_vdev_attach = dp_vdev_attach_wifi3,
	.txrx_vdev_detach = dp_vdev_detach_wifi3,
	.txrx_pdev_attach = dp_pdev_attach_wifi3,
	.txrx_pdev_post_attach = dp_pdev_post_attach_wifi3,
	.txrx_pdev_detach = dp_pdev_detach_wifi3,
	.txrx_pdev_deinit = dp_pdev_deinit_wifi3,
	.txrx_peer_create = dp_peer_create_wifi3,
	.txrx_peer_setup = dp_peer_setup_wifi3,
#ifdef FEATURE_AST
	.txrx_peer_teardown = dp_peer_teardown_wifi3,
#else
	.txrx_peer_teardown = NULL,
#endif
	.txrx_peer_add_ast = dp_peer_add_ast_wifi3,
	.txrx_peer_update_ast = dp_peer_update_ast_wifi3,
	.txrx_peer_get_ast_info_by_soc = dp_peer_get_ast_info_by_soc_wifi3,
	.txrx_peer_get_ast_info_by_pdev =
		dp_peer_get_ast_info_by_pdevid_wifi3,
	.txrx_peer_ast_delete_by_soc =
		dp_peer_ast_entry_del_by_soc,
	.txrx_peer_ast_delete_by_pdev =
		dp_peer_ast_entry_del_by_pdev,
	.txrx_peer_delete = dp_peer_delete_wifi3,
	.txrx_vdev_register = dp_vdev_register_wifi3,
	.txrx_soc_detach = dp_soc_detach_wifi3,
	.txrx_soc_deinit = dp_soc_deinit_wifi3,
	.txrx_soc_init = dp_soc_init_wifi3,
#ifndef QCA_HOST_MODE_WIFI_DISABLED
	.txrx_tso_soc_attach = dp_tso_soc_attach,
	.txrx_tso_soc_detach = dp_tso_soc_detach,
	.tx_send = dp_tx_send,
	.tx_send_exc = dp_tx_send_exception,
#endif
	.set_tx_pause = dp_set_tx_pause,
	.txrx_pdev_init = dp_pdev_init_wifi3,
	.txrx_get_vdev_mac_addr = dp_get_vdev_mac_addr_wifi3,
	.txrx_get_ctrl_pdev_from_vdev = dp_get_ctrl_pdev_from_vdev_wifi3,
	.txrx_ath_getstats = dp_get_device_stats,
	.addba_requestprocess = dp_addba_requestprocess_wifi3,
	.addba_responsesetup = dp_addba_responsesetup_wifi3,
	.addba_resp_tx_completion = dp_addba_resp_tx_completion_wifi3,
	.delba_process = dp_delba_process_wifi3,
	.set_addba_response = dp_set_addba_response,
	.flush_cache_rx_queue = NULL,
	.tid_update_ba_win_size = dp_rx_tid_update_ba_win_size,
	/* TODO: get API's for dscp-tid need to be added*/
	.set_vdev_dscp_tid_map = dp_set_vdev_dscp_tid_map_wifi3,
	.set_pdev_dscp_tid_map = dp_set_pdev_dscp_tid_map_wifi3,
	.txrx_get_total_per = dp_get_total_per,
	.txrx_stats_request = dp_txrx_stats_request,
	.txrx_get_peer_mac_from_peer_id = dp_get_peer_mac_from_peer_id,
	.display_stats = dp_txrx_dump_stats,
	.txrx_intr_attach = dp_soc_interrupt_attach_wrapper,
	.txrx_intr_detach = dp_soc_interrupt_detach,
	.set_pn_check = dp_set_pn_check_wifi3,
	.set_key_sec_type = dp_set_key_sec_type_wifi3,
	.update_config_parameters = dp_update_config_parameters,
	/* TODO: Add other functions */
	.txrx_data_tx_cb_set = dp_txrx_data_tx_cb_set,
	.get_dp_txrx_handle = dp_pdev_get_dp_txrx_handle,
	.set_dp_txrx_handle = dp_pdev_set_dp_txrx_handle,
	.get_vdev_dp_ext_txrx_handle = dp_vdev_get_dp_ext_handle,
	.set_vdev_dp_ext_txrx_handle = dp_vdev_set_dp_ext_handle,
	.get_soc_dp_txrx_handle = dp_soc_get_dp_txrx_handle,
	.set_soc_dp_txrx_handle = dp_soc_set_dp_txrx_handle,
	.map_pdev_to_lmac = dp_soc_map_pdev_to_lmac,
	.handle_mode_change = dp_soc_handle_pdev_mode_change,
	.set_pdev_status_down = dp_soc_set_pdev_status_down,
	.txrx_set_ba_aging_timeout = dp_set_ba_aging_timeout,
	.txrx_get_ba_aging_timeout = dp_get_ba_aging_timeout,
	.txrx_peer_reset_ast = dp_wds_reset_ast_wifi3,
	.txrx_peer_reset_ast_table = dp_wds_reset_ast_table_wifi3,
	.txrx_peer_flush_ast_table = dp_wds_flush_ast_table_wifi3,
	.txrx_peer_map_attach = dp_peer_map_attach_wifi3,
	.set_soc_param = dp_soc_set_param,
	.txrx_get_os_rx_handles_from_vdev =
					dp_get_os_rx_handles_from_vdev_wifi3,
	.delba_tx_completion = dp_delba_tx_completion_wifi3,
	.get_dp_capabilities = dp_get_cfg_capabilities,
	.txrx_get_cfg = dp_get_cfg,
	.set_rate_stats_ctx = dp_soc_set_rate_stats_ctx,
	.get_rate_stats_ctx = dp_soc_get_rate_stats_ctx,
	.txrx_peer_flush_rate_stats = dp_peer_flush_rate_stats,
	.txrx_flush_rate_stats_request = dp_flush_rate_stats_req,
	.txrx_peer_get_rdkstats_ctx = dp_peer_get_rdkstats_ctx,

	.set_pdev_pcp_tid_map = dp_set_pdev_pcp_tid_map_wifi3,
	.set_vdev_pcp_tid_map = dp_set_vdev_pcp_tid_map_wifi3,

	.txrx_cp_peer_del_response = dp_cp_peer_del_resp_handler,
#ifdef QCA_MULTIPASS_SUPPORT
	.set_vlan_groupkey = dp_set_vlan_groupkey,
#endif
	.get_peer_mac_list = dp_get_peer_mac_list,
#ifdef QCA_SUPPORT_WDS_EXTENDED
	.get_wds_ext_peer_id = dp_wds_ext_get_peer_id,
	.set_wds_ext_peer_rx = dp_wds_ext_set_peer_rx,
#endif /* QCA_SUPPORT_WDS_EXTENDED */

#if defined(FEATURE_RUNTIME_PM) || defined(DP_POWER_SAVE)
	.txrx_drain = dp_drain_txrx,
#endif
#if defined(FEATURE_RUNTIME_PM)
	.set_rtpm_tput_policy = dp_set_rtpm_tput_policy_requirement,
#endif
#ifdef WLAN_SYSFS_DP_STATS
	.txrx_sysfs_fill_stats = dp_sysfs_fill_stats,
	.txrx_sysfs_set_stat_type = dp_sysfs_set_stat_type,
#endif /* WLAN_SYSFS_DP_STATS */
#ifdef WLAN_FEATURE_PKT_CAPTURE_V2
	.set_pkt_capture_mode = dp_set_pkt_capture_mode,
#endif
};

static struct cdp_ctrl_ops dp_ops_ctrl = {
	.txrx_peer_authorize = dp_peer_authorize,
	.txrx_peer_get_authorize = dp_peer_get_authorize,
#ifdef VDEV_PEER_PROTOCOL_COUNT
	.txrx_enable_peer_protocol_count = dp_enable_vdev_peer_protocol_count,
	.txrx_set_peer_protocol_drop_mask =
		dp_enable_vdev_peer_protocol_drop_mask,
	.txrx_is_peer_protocol_count_enabled =
		dp_is_vdev_peer_protocol_count_enabled,
	.txrx_get_peer_protocol_drop_mask = dp_get_vdev_peer_protocol_drop_mask,
#endif
	.txrx_set_vdev_param = dp_set_vdev_param,
	.txrx_set_psoc_param = dp_set_psoc_param,
	.txrx_get_psoc_param = dp_get_psoc_param,
	.txrx_set_pdev_reo_dest = dp_set_pdev_reo_dest,
	.txrx_get_pdev_reo_dest = dp_get_pdev_reo_dest,
	.txrx_get_sec_type = dp_get_sec_type,
	.txrx_wdi_event_sub = dp_wdi_event_sub,
	.txrx_wdi_event_unsub = dp_wdi_event_unsub,
	.txrx_set_pdev_param = dp_set_pdev_param,
	.txrx_get_pdev_param = dp_get_pdev_param,
	.txrx_set_peer_param = dp_set_peer_param,
	.txrx_get_peer_param = dp_get_peer_param,
#ifdef VDEV_PEER_PROTOCOL_COUNT
	.txrx_peer_protocol_cnt = dp_peer_stats_update_protocol_cnt,
#endif
#ifdef WLAN_SUPPORT_MSCS
	.txrx_record_mscs_params = dp_record_mscs_params,
#endif
#ifdef WLAN_SUPPORT_SCS
	.txrx_enable_scs_params = dp_enable_scs_params,
	.txrx_record_scs_params = dp_record_scs_params,
#endif
	.set_key = dp_set_michael_key,
	.txrx_get_vdev_param = dp_get_vdev_param,
	.calculate_delay_stats = dp_calculate_delay_stats,
#ifdef WLAN_SUPPORT_RX_PROTOCOL_TYPE_TAG
	.txrx_update_pdev_rx_protocol_tag = dp_update_pdev_rx_protocol_tag,
#ifdef WLAN_SUPPORT_RX_TAG_STATISTICS
	.txrx_dump_pdev_rx_protocol_tag_stats =
				dp_dump_pdev_rx_protocol_tag_stats,
#endif /* WLAN_SUPPORT_RX_TAG_STATISTICS */
#endif /* WLAN_SUPPORT_RX_PROTOCOL_TYPE_TAG */
#ifdef WLAN_SUPPORT_RX_FLOW_TAG
	.txrx_set_rx_flow_tag = dp_set_rx_flow_tag,
	.txrx_dump_rx_flow_tag_stats = dp_dump_rx_flow_tag_stats,
#endif /* WLAN_SUPPORT_RX_FLOW_TAG */
#ifdef QCA_MULTIPASS_SUPPORT
	.txrx_peer_set_vlan_id = dp_peer_set_vlan_id,
#endif /*QCA_MULTIPASS_SUPPORT*/
#ifdef WLAN_FEATURE_TSF_UPLINK_DELAY
	.txrx_set_delta_tsf = dp_set_delta_tsf,
	.txrx_set_tsf_ul_delay_report = dp_set_tsf_ul_delay_report,
	.txrx_get_uplink_delay = dp_get_uplink_delay,
#endif
};

static struct cdp_me_ops dp_ops_me = {
#ifndef QCA_HOST_MODE_WIFI_DISABLED
#ifdef ATH_SUPPORT_IQUE
	.tx_me_alloc_descriptor = dp_tx_me_alloc_descriptor,
	.tx_me_free_descriptor = dp_tx_me_free_descriptor,
	.tx_me_convert_ucast = dp_tx_me_send_convert_ucast,
#endif
#endif
};

static struct cdp_host_stats_ops dp_ops_host_stats = {
	.txrx_per_peer_stats = dp_get_host_peer_stats,
	.get_fw_peer_stats = dp_get_fw_peer_stats,
	.get_htt_stats = dp_get_htt_stats,
	.txrx_stats_publish = dp_txrx_stats_publish,
	.txrx_get_vdev_stats  = dp_txrx_get_vdev_stats,
	.txrx_get_peer_stats = dp_txrx_get_peer_stats,
	.txrx_get_soc_stats = dp_txrx_get_soc_stats,
	.txrx_get_peer_stats_param = dp_txrx_get_peer_stats_param,
	.txrx_reset_peer_stats = dp_txrx_reset_peer_stats,
	.txrx_get_pdev_stats = dp_txrx_get_pdev_stats,
	.txrx_get_ratekbps = dp_txrx_get_ratekbps,
	.txrx_update_vdev_stats = dp_txrx_update_vdev_host_stats,
	.txrx_get_peer_delay_stats = dp_txrx_get_peer_delay_stats,
	.txrx_get_peer_jitter_stats = dp_txrx_get_peer_jitter_stats,
#ifdef QCA_VDEV_STATS_HW_OFFLOAD_SUPPORT
	.txrx_alloc_vdev_stats_id = dp_txrx_alloc_vdev_stats_id,
	.txrx_reset_vdev_stats_id = dp_txrx_reset_vdev_stats_id,
#endif
#ifdef HW_TX_DELAY_STATS_ENABLE
	.enable_disable_vdev_tx_delay_stats =
				dp_enable_disable_vdev_tx_delay_stats,
	.is_tx_delay_stats_enabled = dp_check_vdev_tx_delay_stats_enabled,
#endif
	/* TODO */
};

static struct cdp_raw_ops dp_ops_raw = {
	/* TODO */
};

#ifdef PEER_FLOW_CONTROL
static struct cdp_pflow_ops dp_ops_pflow = {
	dp_tx_flow_ctrl_configure_pdev,
};
#endif /* CONFIG_WIN */

#if defined(WLAN_CFR_ENABLE) && defined(WLAN_ENH_CFR_ENABLE)
static struct cdp_cfr_ops dp_ops_cfr = {
	.txrx_cfr_filter = NULL,
	.txrx_get_cfr_rcc = dp_get_cfr_rcc,
	.txrx_set_cfr_rcc = dp_set_cfr_rcc,
	.txrx_get_cfr_dbg_stats = dp_get_cfr_dbg_stats,
	.txrx_clear_cfr_dbg_stats = dp_clear_cfr_dbg_stats,
	.txrx_enable_mon_reap_timer = NULL,
};
#endif

#ifdef WLAN_SUPPORT_MSCS
static struct cdp_mscs_ops dp_ops_mscs = {
	.mscs_peer_lookup_n_get_priority = dp_mscs_peer_lookup_n_get_priority,
};
#endif

#ifdef WLAN_SUPPORT_MESH_LATENCY
static struct cdp_mesh_latency_ops dp_ops_mesh_latency = {
	.mesh_latency_update_peer_parameter =
		dp_mesh_latency_update_peer_parameter,
};
#endif

#if defined(DP_POWER_SAVE) || defined(FEATURE_RUNTIME_PM)
/**
 * dp_flush_ring_hptp() - Update ring shadow
 *			  register HP/TP address when runtime
 *                        resume
 * @opaque_soc: DP soc context
 *
 * Return: None
 */
static
void dp_flush_ring_hptp(struct dp_soc *soc, hal_ring_handle_t hal_srng)
{
	if (hal_srng && hal_srng_get_clear_event(hal_srng,
						 HAL_SRNG_FLUSH_EVENT)) {
		/* Acquire the lock */
		hal_srng_access_start(soc->hal_soc, hal_srng);

		hal_srng_access_end(soc->hal_soc, hal_srng);

		hal_srng_set_flush_last_ts(hal_srng);

		dp_debug("flushed");
	}
}
#endif

#ifdef DP_TX_TRACKING

#define DP_TX_COMP_MAX_LATENCY_MS 60000
/**
 * dp_tx_comp_delay_check() - calculate time latency for tx completion per pkt
 * @timestamp - tx descriptor timestamp
 *
 * Calculate time latency for tx completion per pkt and trigger self recovery
 * when the delay is more than threshold value.
 *
 * Return: True if delay is more than threshold
 */
static bool dp_tx_comp_delay_check(uint64_t timestamp)
{
	uint64_t time_latency, current_time;

	if (!timestamp)
		return false;

	if (dp_tx_pkt_tracepoints_enabled()) {
		current_time = qdf_ktime_to_ms(qdf_ktime_real_get());
		time_latency = current_time - timestamp;
		if (time_latency >= DP_TX_COMP_MAX_LATENCY_MS) {
			dp_err_rl("enqueued: %llu ms, current : %llu ms",
				  timestamp, current_time);
			return true;
		}
	} else {
		current_time = qdf_system_ticks();
		time_latency = qdf_system_ticks_to_msecs(current_time -
							 timestamp);
		if (time_latency >= DP_TX_COMP_MAX_LATENCY_MS) {
			dp_err_rl("enqueued: %u ms, current : %u ms",
				  qdf_system_ticks_to_msecs(timestamp),
				  qdf_system_ticks_to_msecs(current_time));
			return true;
		}
	}

	return false;
}

/**
 * dp_find_missing_tx_comp() - check for leaked descriptor in tx path
 * @soc - DP SOC context
 *
 * Parse through descriptors in all pools and validate magic number and
 * completion time. Trigger self recovery if magic value is corrupted.
 *
 * Return: None.
 */
static void dp_find_missing_tx_comp(struct dp_soc *soc)
{
	uint8_t i;
	uint32_t j;
	uint32_t num_desc, page_id, offset;
	uint16_t num_desc_per_page;
	struct dp_tx_desc_s *tx_desc = NULL;
	struct dp_tx_desc_pool_s *tx_desc_pool = NULL;

	for (i = 0; i < MAX_TXDESC_POOLS; i++) {
		tx_desc_pool = &soc->tx_desc[i];
		if (!(tx_desc_pool->pool_size) ||
		    IS_TX_DESC_POOL_STATUS_INACTIVE(tx_desc_pool) ||
		    !(tx_desc_pool->desc_pages.cacheable_pages))
			continue;

		num_desc = tx_desc_pool->pool_size;
		num_desc_per_page =
			tx_desc_pool->desc_pages.num_element_per_page;
		for (j = 0; j < num_desc; j++) {
			page_id = j / num_desc_per_page;
			offset = j % num_desc_per_page;

			if (qdf_unlikely(!(tx_desc_pool->
					 desc_pages.cacheable_pages)))
				break;

			tx_desc = dp_tx_desc_find(soc, i, page_id, offset);
			if (tx_desc->magic == DP_TX_MAGIC_PATTERN_FREE) {
				continue;
			} else if (tx_desc->magic ==
				   DP_TX_MAGIC_PATTERN_INUSE) {
				if (dp_tx_comp_delay_check(
							tx_desc->timestamp)) {
					dp_err_rl("Tx completion not rcvd for id: %u",
						  tx_desc->id);
					if (tx_desc->vdev_id == DP_INVALID_VDEV_ID) {
						tx_desc->flags |= DP_TX_DESC_FLAG_FLUSH;
						dp_err_rl("Freed tx_desc %u",
							  tx_desc->id);
						dp_tx_comp_free_buf(soc,
								    tx_desc);
						dp_tx_desc_release(tx_desc, i);
						DP_STATS_INC(soc,
							     tx.tx_comp_force_freed, 1);
					}
				}
			} else {
				dp_err_rl("tx desc %u corrupted, flags: 0x%x",
					  tx_desc->id, tx_desc->flags);
			}
		}
	}
}
#else
static inline void dp_find_missing_tx_comp(struct dp_soc *soc)
{
}
#endif

#ifdef FEATURE_RUNTIME_PM
/**
 * dp_runtime_suspend() - ensure DP is ready to runtime suspend
 * @soc_hdl: Datapath soc handle
 * @pdev_id: id of data path pdev handle
 *
 * DP is ready to runtime suspend if there are no pending TX packets.
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS dp_runtime_suspend(struct cdp_soc_t *soc_hdl, uint8_t pdev_id)
{
	struct dp_soc *soc = cdp_soc_t_to_dp_soc(soc_hdl);
	struct dp_pdev *pdev;
	uint8_t i;
	int32_t tx_pending;

	pdev = dp_get_pdev_from_soc_pdev_id_wifi3(soc, pdev_id);
	if (!pdev) {
		dp_err("pdev is NULL");
		return QDF_STATUS_E_INVAL;
	}

	/* Abort if there are any pending TX packets */
	tx_pending = dp_get_tx_pending(dp_pdev_to_cdp_pdev(pdev));
	if (tx_pending) {
		dp_info_rl("%pK: Abort suspend due to pending TX packets %d",
			   soc, tx_pending);
		dp_find_missing_tx_comp(soc);
		/* perform a force flush if tx is pending */
		for (i = 0; i < soc->num_tcl_data_rings; i++) {
			hal_srng_set_event(soc->tcl_data_ring[i].hal_srng,
					   HAL_SRNG_FLUSH_EVENT);
			dp_flush_ring_hptp(soc, soc->tcl_data_ring[i].hal_srng);
		}
		qdf_atomic_set(&soc->tx_pending_rtpm, 0);

		return QDF_STATUS_E_AGAIN;
	}

	if (dp_runtime_get_refcount(soc)) {
		dp_init_info("refcount: %d", dp_runtime_get_refcount(soc));

		return QDF_STATUS_E_AGAIN;
	}

	if (soc->intr_mode == DP_INTR_POLL)
		qdf_timer_stop(&soc->int_timer);

	dp_rx_fst_update_pm_suspend_status(soc, true);

	return QDF_STATUS_SUCCESS;
}

#define DP_FLUSH_WAIT_CNT 10
#define DP_RUNTIME_SUSPEND_WAIT_MS 10
/**
 * dp_runtime_resume() - ensure DP is ready to runtime resume
 * @soc_hdl: Datapath soc handle
 * @pdev_id: id of data path pdev handle
 *
 * Resume DP for runtime PM.
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS dp_runtime_resume(struct cdp_soc_t *soc_hdl, uint8_t pdev_id)
{
	struct dp_soc *soc = cdp_soc_t_to_dp_soc(soc_hdl);
	int i, suspend_wait = 0;

	if (soc->intr_mode == DP_INTR_POLL)
		qdf_timer_mod(&soc->int_timer, DP_INTR_POLL_TIMER_MS);

	/*
	 * Wait until dp runtime refcount becomes zero or time out, then flush
	 * pending tx for runtime suspend.
	 */
	while (dp_runtime_get_refcount(soc) &&
	       suspend_wait < DP_FLUSH_WAIT_CNT) {
		qdf_sleep(DP_RUNTIME_SUSPEND_WAIT_MS);
		suspend_wait++;
	}

	for (i = 0; i < MAX_TCL_DATA_RINGS; i++) {
		dp_flush_ring_hptp(soc, soc->tcl_data_ring[i].hal_srng);
	}
	qdf_atomic_set(&soc->tx_pending_rtpm, 0);

	dp_flush_ring_hptp(soc, soc->reo_cmd_ring.hal_srng);
	dp_rx_fst_update_pm_suspend_status(soc, false);

	return QDF_STATUS_SUCCESS;
}
#endif /* FEATURE_RUNTIME_PM */

/**
 * dp_tx_get_success_ack_stats() - get tx success completion count
 * @soc_hdl: Datapath soc handle
 * @vdevid: vdev identifier
 *
 * Return: tx success ack count
 */
static uint32_t dp_tx_get_success_ack_stats(struct cdp_soc_t *soc_hdl,
					    uint8_t vdev_id)
{
	struct dp_soc *soc = cdp_soc_t_to_dp_soc(soc_hdl);
	struct cdp_vdev_stats *vdev_stats = NULL;
	uint32_t tx_success;
	struct dp_vdev *vdev = dp_vdev_get_ref_by_id(soc, vdev_id,
						     DP_MOD_ID_CDP);

	if (!vdev) {
		dp_cdp_err("%pK: Invalid vdev id %d", soc, vdev_id);
		return 0;
	}

	vdev_stats = qdf_mem_malloc_atomic(sizeof(struct cdp_vdev_stats));
	if (!vdev_stats) {
		dp_cdp_err("%pK: DP alloc failure - unable to get alloc vdev stats", soc);
		dp_vdev_unref_delete(soc, vdev, DP_MOD_ID_CDP);
		return 0;
	}

	dp_aggregate_vdev_stats(vdev, vdev_stats);

	tx_success = vdev_stats->tx.tx_success.num;
	qdf_mem_free(vdev_stats);

	dp_vdev_unref_delete(soc, vdev, DP_MOD_ID_CDP);
	return tx_success;
}

#ifdef WLAN_SUPPORT_DATA_STALL
/**
 * dp_register_data_stall_detect_cb() - register data stall callback
 * @soc_hdl: Datapath soc handle
 * @pdev_id: id of data path pdev handle
 * @data_stall_detect_callback: data stall callback function
 *
 * Return: QDF_STATUS Enumeration
 */
static
QDF_STATUS dp_register_data_stall_detect_cb(
			struct cdp_soc_t *soc_hdl, uint8_t pdev_id,
			data_stall_detect_cb data_stall_detect_callback)
{
	struct dp_soc *soc = cdp_soc_t_to_dp_soc(soc_hdl);
	struct dp_pdev *pdev;

	pdev = dp_get_pdev_from_soc_pdev_id_wifi3(soc, pdev_id);
	if (!pdev) {
		dp_err("pdev NULL!");
		return QDF_STATUS_E_INVAL;
	}

	pdev->data_stall_detect_callback = data_stall_detect_callback;
	return QDF_STATUS_SUCCESS;
}

/**
 * dp_deregister_data_stall_detect_cb() - de-register data stall callback
 * @soc_hdl: Datapath soc handle
 * @pdev_id: id of data path pdev handle
 * @data_stall_detect_callback: data stall callback function
 *
 * Return: QDF_STATUS Enumeration
 */
static
QDF_STATUS dp_deregister_data_stall_detect_cb(
			struct cdp_soc_t *soc_hdl, uint8_t pdev_id,
			data_stall_detect_cb data_stall_detect_callback)
{
	struct dp_soc *soc = cdp_soc_t_to_dp_soc(soc_hdl);
	struct dp_pdev *pdev;

	pdev = dp_get_pdev_from_soc_pdev_id_wifi3(soc, pdev_id);
	if (!pdev) {
		dp_err("pdev NULL!");
		return QDF_STATUS_E_INVAL;
	}

	pdev->data_stall_detect_callback = NULL;
	return QDF_STATUS_SUCCESS;
}

/**
 * dp_txrx_post_data_stall_event() - post data stall event
 * @soc_hdl: Datapath soc handle
 * @indicator: Module triggering data stall
 * @data_stall_type: data stall event type
 * @pdev_id: pdev id
 * @vdev_id_bitmap: vdev id bitmap
 * @recovery_type: data stall recovery type
 *
 * Return: None
 */
static void
dp_txrx_post_data_stall_event(struct cdp_soc_t *soc_hdl,
			      enum data_stall_log_event_indicator indicator,
			      enum data_stall_log_event_type data_stall_type,
			      uint32_t pdev_id, uint32_t vdev_id_bitmap,
			      enum data_stall_log_recovery_type recovery_type)
{
	struct dp_soc *soc = cdp_soc_t_to_dp_soc(soc_hdl);
	struct data_stall_event_info data_stall_info;
	struct dp_pdev *pdev;

	pdev = dp_get_pdev_from_soc_pdev_id_wifi3(soc, pdev_id);
	if (!pdev) {
		dp_err("pdev NULL!");
		return;
	}

	if (!pdev->data_stall_detect_callback) {
		dp_err("data stall cb not registered!");
		return;
	}

	dp_info("data_stall_type: %x pdev_id: %d",
		data_stall_type, pdev_id);

	data_stall_info.indicator = indicator;
	data_stall_info.data_stall_type = data_stall_type;
	data_stall_info.vdev_id_bitmap = vdev_id_bitmap;
	data_stall_info.pdev_id = pdev_id;
	data_stall_info.recovery_type = recovery_type;

	pdev->data_stall_detect_callback(&data_stall_info);
}
#endif /* WLAN_SUPPORT_DATA_STALL */

#ifdef WLAN_FEATURE_STATS_EXT
/* rx hw stats event wait timeout in ms */
#define DP_REO_STATUS_STATS_TIMEOUT 1500
/**
 * dp_txrx_ext_stats_request - request dp txrx extended stats request
 * @soc_hdl: soc handle
 * @pdev_id: pdev id
 * @req: stats request
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS
dp_txrx_ext_stats_request(struct cdp_soc_t *soc_hdl, uint8_t pdev_id,
			  struct cdp_txrx_ext_stats *req)
{
	struct dp_soc *soc = (struct dp_soc *)soc_hdl;
	struct dp_pdev *pdev = dp_get_pdev_from_soc_pdev_id_wifi3(soc, pdev_id);
	int i = 0;
	int tcl_ring_full = 0;

	if (!pdev) {
		dp_err("pdev is null");
		return QDF_STATUS_E_INVAL;
	}

	dp_aggregate_pdev_stats(pdev);

	for(i = 0 ; i < MAX_TCL_DATA_RINGS; i++)
		tcl_ring_full += soc->stats.tx.tcl_ring_full[i];

	req->tx_msdu_enqueue = pdev->stats.tx_i.processed.num;
	req->tx_msdu_overflow = tcl_ring_full;
	req->rx_mpdu_received = soc->ext_stats.rx_mpdu_received;
	req->rx_mpdu_delivered = soc->ext_stats.rx_mpdu_received;
	req->rx_mpdu_missed = pdev->stats.err.reo_error;
	/* only count error source from RXDMA */
	req->rx_mpdu_error = pdev->stats.err.rxdma_error;

	dp_info("ext stats: tx_msdu_enq = %u, tx_msdu_overflow = %u, "
		"tx_mpdu_recieve = %u, rx_mpdu_delivered = %u, "
		"rx_mpdu_missed = %u, rx_mpdu_error = %u",
		req->tx_msdu_enqueue,
		req->tx_msdu_overflow,
		req->rx_mpdu_received,
		req->rx_mpdu_delivered,
		req->rx_mpdu_missed,
		req->rx_mpdu_error);

	return QDF_STATUS_SUCCESS;
}

/**
 * dp_rx_hw_stats_cb - request rx hw stats response callback
 * @soc: soc handle
 * @cb_ctxt: callback context
 * @reo_status: reo command response status
 *
 * Return: None
 */
static void dp_rx_hw_stats_cb(struct dp_soc *soc, void *cb_ctxt,
			      union hal_reo_status *reo_status)
{
	struct dp_req_rx_hw_stats_t *rx_hw_stats = cb_ctxt;
	struct hal_reo_queue_status *queue_status = &reo_status->queue_status;
	bool is_query_timeout;

	qdf_spin_lock_bh(&soc->rx_hw_stats_lock);
	is_query_timeout = rx_hw_stats->is_query_timeout;
	/* free the cb_ctxt if all pending tid stats query is received */
	if (qdf_atomic_dec_and_test(&rx_hw_stats->pending_tid_stats_cnt)) {
		if (!is_query_timeout) {
			qdf_event_set(&soc->rx_hw_stats_event);
			soc->is_last_stats_ctx_init = false;
		}

		qdf_mem_free(rx_hw_stats);
	}

	if (queue_status->header.status != HAL_REO_CMD_SUCCESS) {
		dp_info("REO stats failure %d",
			queue_status->header.status);
		qdf_spin_unlock_bh(&soc->rx_hw_stats_lock);
		return;
	}

	if (!is_query_timeout) {
		soc->ext_stats.rx_mpdu_received +=
					queue_status->mpdu_frms_cnt;
		soc->ext_stats.rx_mpdu_missed +=
					queue_status->hole_cnt;
	}
	qdf_spin_unlock_bh(&soc->rx_hw_stats_lock);
}

/**
 * dp_request_rx_hw_stats - request rx hardware stats
 * @soc_hdl: soc handle
 * @vdev_id: vdev id
 *
 * Return: None
 */
static QDF_STATUS
dp_request_rx_hw_stats(struct cdp_soc_t *soc_hdl, uint8_t vdev_id)
{
	struct dp_soc *soc = (struct dp_soc *)soc_hdl;
	struct dp_vdev *vdev = dp_vdev_get_ref_by_id(soc, vdev_id,
						     DP_MOD_ID_CDP);
	struct dp_peer *peer = NULL;
	QDF_STATUS status;
	struct dp_req_rx_hw_stats_t *rx_hw_stats;
	int rx_stats_sent_cnt = 0;
	uint32_t last_rx_mpdu_received;
	uint32_t last_rx_mpdu_missed;

	if (!vdev) {
		dp_err("vdev is null for vdev_id: %u", vdev_id);
		status = QDF_STATUS_E_INVAL;
		goto out;
	}

	peer = dp_vdev_bss_peer_ref_n_get(soc, vdev, DP_MOD_ID_CDP);

	if (!peer) {
		dp_err("Peer is NULL");
		status = QDF_STATUS_E_INVAL;
		goto out;
	}

	rx_hw_stats = qdf_mem_malloc(sizeof(*rx_hw_stats));

	if (!rx_hw_stats) {
		dp_err("malloc failed for hw stats structure");
		status = QDF_STATUS_E_INVAL;
		goto out;
	}

	qdf_event_reset(&soc->rx_hw_stats_event);
	qdf_spin_lock_bh(&soc->rx_hw_stats_lock);
	/* save the last soc cumulative stats and reset it to 0 */
	last_rx_mpdu_received = soc->ext_stats.rx_mpdu_received;
	last_rx_mpdu_missed = soc->ext_stats.rx_mpdu_missed;
	soc->ext_stats.rx_mpdu_received = 0;

	rx_stats_sent_cnt =
		dp_peer_rxtid_stats(peer, dp_rx_hw_stats_cb, rx_hw_stats);
	if (!rx_stats_sent_cnt) {
		dp_err("no tid stats sent successfully");
		qdf_mem_free(rx_hw_stats);
		qdf_spin_unlock_bh(&soc->rx_hw_stats_lock);
		status = QDF_STATUS_E_INVAL;
		goto out;
	}
	qdf_atomic_set(&rx_hw_stats->pending_tid_stats_cnt,
		       rx_stats_sent_cnt);
	rx_hw_stats->is_query_timeout = false;
	soc->is_last_stats_ctx_init = true;
	qdf_spin_unlock_bh(&soc->rx_hw_stats_lock);

	status = qdf_wait_single_event(&soc->rx_hw_stats_event,
				       DP_REO_STATUS_STATS_TIMEOUT);

	qdf_spin_lock_bh(&soc->rx_hw_stats_lock);
	if (status != QDF_STATUS_SUCCESS) {
		dp_info("rx hw stats event timeout");
		if (soc->is_last_stats_ctx_init)
			rx_hw_stats->is_query_timeout = true;
		/**
		 * If query timeout happened, use the last saved stats
		 * for this time query.
		 */
		soc->ext_stats.rx_mpdu_received = last_rx_mpdu_received;
		soc->ext_stats.rx_mpdu_missed = last_rx_mpdu_missed;
	}
	qdf_spin_unlock_bh(&soc->rx_hw_stats_lock);

out:
	if (peer)
		dp_peer_unref_delete(peer, DP_MOD_ID_CDP);
	if (vdev)
		dp_vdev_unref_delete(soc, vdev, DP_MOD_ID_CDP);

	return status;
}

/**
 * dp_reset_rx_hw_ext_stats - Reset rx hardware ext stats
 * @soc_hdl: soc handle
 *
 * Return: None
 */
static
void dp_reset_rx_hw_ext_stats(struct cdp_soc_t *soc_hdl)
{
	struct dp_soc *soc = (struct dp_soc *)soc_hdl;

	soc->ext_stats.rx_mpdu_received = 0;
	soc->ext_stats.rx_mpdu_missed = 0;
}
#endif /* WLAN_FEATURE_STATS_EXT */

static
uint32_t dp_get_tx_rings_grp_bitmap(struct cdp_soc_t *soc_hdl)
{
	struct dp_soc *soc = (struct dp_soc *)soc_hdl;

	return soc->wlan_cfg_ctx->tx_rings_grp_bitmap;
}

#ifdef WLAN_FEATURE_MARK_FIRST_WAKEUP_PACKET
/**
 * dp_mark_first_wakeup_packet() - set flag to indicate that
 *    fw is compatible for marking first packet after wow wakeup
 * @soc_hdl: Datapath soc handle
 * @pdev_id: id of data path pdev handle
 * @value: 1 for enabled/ 0 for disabled
 *
 * Return: None
 */
static void dp_mark_first_wakeup_packet(struct cdp_soc_t *soc_hdl,
					uint8_t pdev_id, uint8_t value)
{
	struct dp_soc *soc = cdp_soc_t_to_dp_soc(soc_hdl);
	struct dp_pdev *pdev;

	pdev = dp_get_pdev_from_soc_pdev_id_wifi3(soc, pdev_id);
	if (!pdev) {
		dp_err("pdev is NULL");
		return;
	}

	pdev->is_first_wakeup_packet = value;
}
#endif

#ifdef CONNECTIVITY_PKTLOG
/**
 * dp_register_packetdump_callback() - registers
 *  tx data packet, tx mgmt. packet and rx data packet
 *  dump callback handler.
 *
 * @soc_hdl: Datapath soc handle
 * @pdev_id: id of data path pdev handle
 * @dp_tx_packetdump_cb: tx packetdump cb
 * @dp_rx_packetdump_cb: rx packetdump cb
 *
 * This function is used to register tx data pkt, tx mgmt.
 * pkt and rx data pkt dump callback
 *
 * Return: None
 *
 */
static inline
void dp_register_packetdump_callback(struct cdp_soc_t *soc_hdl, uint8_t pdev_id,
				     ol_txrx_pktdump_cb dp_tx_packetdump_cb,
				     ol_txrx_pktdump_cb dp_rx_packetdump_cb)
{
	struct dp_soc *soc = cdp_soc_t_to_dp_soc(soc_hdl);
	struct dp_pdev *pdev;

	pdev = dp_get_pdev_from_soc_pdev_id_wifi3(soc, pdev_id);
	if (!pdev) {
		dp_err("pdev is NULL!");
		return;
	}

	pdev->dp_tx_packetdump_cb = dp_tx_packetdump_cb;
	pdev->dp_rx_packetdump_cb = dp_rx_packetdump_cb;
}

/**
 * dp_deregister_packetdump_callback() - deregidters
 *  tx data packet, tx mgmt. packet and rx data packet
 *  dump callback handler
 * @soc_hdl: Datapath soc handle
 * @pdev_id: id of data path pdev handle
 *
 * This function is used to deregidter tx data pkt.,
 * tx mgmt. pkt and rx data pkt. dump callback
 *
 * Return: None
 *
 */
static inline
void dp_deregister_packetdump_callback(struct cdp_soc_t *soc_hdl,
				       uint8_t pdev_id)
{
	struct dp_soc *soc = cdp_soc_t_to_dp_soc(soc_hdl);
	struct dp_pdev *pdev;

	pdev = dp_get_pdev_from_soc_pdev_id_wifi3(soc, pdev_id);
	if (!pdev) {
		dp_err("pdev is NULL!");
		return;
	}

	pdev->dp_tx_packetdump_cb = NULL;
	pdev->dp_rx_packetdump_cb = NULL;
}
#endif

#ifdef DP_PEER_EXTENDED_API
static struct cdp_misc_ops dp_ops_misc = {
#ifdef FEATURE_WLAN_TDLS
	.tx_non_std = dp_tx_non_std,
#endif /* FEATURE_WLAN_TDLS */
	.get_opmode = dp_get_opmode,
#ifdef FEATURE_RUNTIME_PM
	.runtime_suspend = dp_runtime_suspend,
	.runtime_resume = dp_runtime_resume,
#endif /* FEATURE_RUNTIME_PM */
	.get_num_rx_contexts = dp_get_num_rx_contexts,
	.get_tx_ack_stats = dp_tx_get_success_ack_stats,
#ifdef WLAN_SUPPORT_DATA_STALL
	.txrx_data_stall_cb_register = dp_register_data_stall_detect_cb,
	.txrx_data_stall_cb_deregister = dp_deregister_data_stall_detect_cb,
	.txrx_post_data_stall_event = dp_txrx_post_data_stall_event,
#endif

#ifdef WLAN_FEATURE_STATS_EXT
	.txrx_ext_stats_request = dp_txrx_ext_stats_request,
	.request_rx_hw_stats = dp_request_rx_hw_stats,
	.reset_rx_hw_ext_stats = dp_reset_rx_hw_ext_stats,
#endif /* WLAN_FEATURE_STATS_EXT */
	.vdev_inform_ll_conn = dp_vdev_inform_ll_conn,
#ifdef WLAN_DP_FEATURE_SW_LATENCY_MGR
	.set_swlm_enable = dp_soc_set_swlm_enable,
	.is_swlm_enabled = dp_soc_is_swlm_enabled,
#endif
	.display_txrx_hw_info = dp_display_srng_info,
	.get_tx_rings_grp_bitmap = dp_get_tx_rings_grp_bitmap,
#ifdef WLAN_FEATURE_MARK_FIRST_WAKEUP_PACKET
	.mark_first_wakeup_packet = dp_mark_first_wakeup_packet,
#endif
#ifdef CONNECTIVITY_PKTLOG
	.register_pktdump_cb = dp_register_packetdump_callback,
	.unregister_pktdump_cb = dp_deregister_packetdump_callback,
#endif
};
#endif

#ifdef DP_FLOW_CTL
static struct cdp_flowctl_ops dp_ops_flowctl = {
	/* WIFI 3.0 DP implement as required. */
#ifdef QCA_LL_TX_FLOW_CONTROL_V2
	.flow_pool_map_handler = dp_tx_flow_pool_map,
	.flow_pool_unmap_handler = dp_tx_flow_pool_unmap,
	.register_pause_cb = dp_txrx_register_pause_cb,
	.dump_flow_pool_info = dp_tx_dump_flow_pool_info,
	.tx_desc_thresh_reached = dp_tx_desc_thresh_reached,
#endif /* QCA_LL_TX_FLOW_CONTROL_V2 */
};

static struct cdp_lflowctl_ops dp_ops_l_flowctl = {
	/* WIFI 3.0 DP NOT IMPLEMENTED YET */
};
#endif

#ifdef IPA_OFFLOAD
static struct cdp_ipa_ops dp_ops_ipa = {
	.ipa_get_resource = dp_ipa_get_resource,
	.ipa_set_doorbell_paddr = dp_ipa_set_doorbell_paddr,
	.ipa_op_response = dp_ipa_op_response,
	.ipa_register_op_cb = dp_ipa_register_op_cb,
	.ipa_deregister_op_cb = dp_ipa_deregister_op_cb,
	.ipa_get_stat = dp_ipa_get_stat,
	.ipa_tx_data_frame = dp_tx_send_ipa_data_frame,
	.ipa_enable_autonomy = dp_ipa_enable_autonomy,
	.ipa_disable_autonomy = dp_ipa_disable_autonomy,
	.ipa_setup = dp_ipa_setup,
	.ipa_cleanup = dp_ipa_cleanup,
	.ipa_setup_iface = dp_ipa_setup_iface,
	.ipa_cleanup_iface = dp_ipa_cleanup_iface,
	.ipa_enable_pipes = dp_ipa_enable_pipes,
	.ipa_disable_pipes = dp_ipa_disable_pipes,
	.ipa_set_perf_level = dp_ipa_set_perf_level,
	.ipa_rx_intrabss_fwd = dp_ipa_rx_intrabss_fwd,
	.ipa_tx_buf_smmu_mapping = dp_ipa_tx_buf_smmu_mapping,
	.ipa_tx_buf_smmu_unmapping = dp_ipa_tx_buf_smmu_unmapping
};
#endif

#ifdef DP_POWER_SAVE
static QDF_STATUS dp_bus_suspend(struct cdp_soc_t *soc_hdl, uint8_t pdev_id)
{
	struct dp_soc *soc = cdp_soc_t_to_dp_soc(soc_hdl);
	struct dp_pdev *pdev = dp_get_pdev_from_soc_pdev_id_wifi3(soc, pdev_id);
	int timeout = SUSPEND_DRAIN_WAIT;
	int drain_wait_delay = 50; /* 50 ms */
	int32_t tx_pending;

	if (qdf_unlikely(!pdev)) {
		dp_err("pdev is NULL");
		return QDF_STATUS_E_INVAL;
	}

	/* Abort if there are any pending TX packets */
	while ((tx_pending = dp_get_tx_pending((struct cdp_pdev *)pdev))) {
		qdf_sleep(drain_wait_delay);
		if (timeout <= 0) {
			dp_info("TX frames are pending %d, abort suspend",
				tx_pending);
			dp_find_missing_tx_comp(soc);
			return QDF_STATUS_E_TIMEOUT;
		}
		timeout = timeout - drain_wait_delay;
	}

	if (soc->intr_mode == DP_INTR_POLL)
		qdf_timer_stop(&soc->int_timer);

	/* Stop monitor reap timer and reap any pending frames in ring */
	dp_monitor_pktlog_reap_pending_frames(pdev);

	dp_suspend_fse_cache_flush(soc);

	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS dp_bus_resume(struct cdp_soc_t *soc_hdl, uint8_t pdev_id)
{
	struct dp_soc *soc = cdp_soc_t_to_dp_soc(soc_hdl);
	struct dp_pdev *pdev = dp_get_pdev_from_soc_pdev_id_wifi3(soc, pdev_id);
	uint8_t i;

	if (qdf_unlikely(!pdev)) {
		dp_err("pdev is NULL");
		return QDF_STATUS_E_INVAL;
	}

	if (soc->intr_mode == DP_INTR_POLL)
		qdf_timer_mod(&soc->int_timer, DP_INTR_POLL_TIMER_MS);

	/* Start monitor reap timer */
	dp_monitor_pktlog_start_reap_timer(pdev);

	dp_resume_fse_cache_flush(soc);

	for (i = 0; i < soc->num_tcl_data_rings; i++)
		dp_flush_ring_hptp(soc, soc->tcl_data_ring[i].hal_srng);

	return QDF_STATUS_SUCCESS;
}

/**
 * dp_process_wow_ack_rsp() - process wow ack response
 * @soc_hdl: datapath soc handle
 * @pdev_id: data path pdev handle id
 *
 * Return: none
 */
static void dp_process_wow_ack_rsp(struct cdp_soc_t *soc_hdl, uint8_t pdev_id)
{
	struct dp_soc *soc = cdp_soc_t_to_dp_soc(soc_hdl);
	struct dp_pdev *pdev = dp_get_pdev_from_soc_pdev_id_wifi3(soc, pdev_id);

	if (qdf_unlikely(!pdev)) {
		dp_err("pdev is NULL");
		return;
	}

	/*
	 * As part of wow enable FW disables the mon status ring and in wow ack
	 * response from FW reap mon status ring to make sure no packets pending
	 * in the ring.
	 */
	dp_monitor_pktlog_reap_pending_frames(pdev);
}

/**
 * dp_process_target_suspend_req() - process target suspend request
 * @soc_hdl: datapath soc handle
 * @pdev_id: data path pdev handle id
 *
 * Return: none
 */
static void dp_process_target_suspend_req(struct cdp_soc_t *soc_hdl,
					  uint8_t pdev_id)
{
	struct dp_soc *soc = cdp_soc_t_to_dp_soc(soc_hdl);
	struct dp_pdev *pdev = dp_get_pdev_from_soc_pdev_id_wifi3(soc, pdev_id);

	if (qdf_unlikely(!pdev)) {
		dp_err("pdev is NULL");
		return;
	}

	/* Stop monitor reap timer and reap any pending frames in ring */
	dp_monitor_pktlog_reap_pending_frames(pdev);
}

static struct cdp_bus_ops dp_ops_bus = {
	.bus_suspend = dp_bus_suspend,
	.bus_resume = dp_bus_resume,
	.process_wow_ack_rsp = dp_process_wow_ack_rsp,
	.process_target_suspend_req = dp_process_target_suspend_req
};
#endif

#ifdef DP_FLOW_CTL
static struct cdp_throttle_ops dp_ops_throttle = {
	/* WIFI 3.0 DP NOT IMPLEMENTED YET */
};

static struct cdp_cfg_ops dp_ops_cfg = {
	/* WIFI 3.0 DP NOT IMPLEMENTED YET */
};
#endif

#ifdef DP_PEER_EXTENDED_API
static struct cdp_ocb_ops dp_ops_ocb = {
	/* WIFI 3.0 DP NOT IMPLEMENTED YET */
};

static struct cdp_mob_stats_ops dp_ops_mob_stats = {
	.clear_stats = dp_txrx_clear_dump_stats,
};

static struct cdp_peer_ops dp_ops_peer = {
	.register_peer = dp_register_peer,
	.clear_peer = dp_clear_peer,
	.find_peer_exist = dp_find_peer_exist,
	.find_peer_exist_on_vdev = dp_find_peer_exist_on_vdev,
	.find_peer_exist_on_other_vdev = dp_find_peer_exist_on_other_vdev,
	.peer_state_update = dp_peer_state_update,
	.get_vdevid = dp_get_vdevid,
	.get_vdev_by_peer_addr = dp_get_vdev_by_peer_addr,
	.peer_get_peer_mac_addr = dp_peer_get_peer_mac_addr,
	.get_peer_state = dp_get_peer_state,
	.peer_flush_frags = dp_peer_flush_frags,
	.set_peer_as_tdls_peer = dp_set_peer_as_tdls_peer,
};
#endif

static void dp_soc_txrx_ops_attach(struct dp_soc *soc)
{
	soc->cdp_soc.ops->cmn_drv_ops = &dp_ops_cmn;
	soc->cdp_soc.ops->ctrl_ops = &dp_ops_ctrl;
	soc->cdp_soc.ops->me_ops = &dp_ops_me;
	soc->cdp_soc.ops->host_stats_ops = &dp_ops_host_stats;
	soc->cdp_soc.ops->wds_ops = &dp_ops_wds;
	soc->cdp_soc.ops->raw_ops = &dp_ops_raw;
#ifdef PEER_FLOW_CONTROL
	soc->cdp_soc.ops->pflow_ops = &dp_ops_pflow;
#endif /* PEER_FLOW_CONTROL */
#ifdef DP_PEER_EXTENDED_API
	soc->cdp_soc.ops->misc_ops = &dp_ops_misc;
	soc->cdp_soc.ops->ocb_ops = &dp_ops_ocb;
	soc->cdp_soc.ops->peer_ops = &dp_ops_peer;
	soc->cdp_soc.ops->mob_stats_ops = &dp_ops_mob_stats;
#endif
#ifdef DP_FLOW_CTL
	soc->cdp_soc.ops->cfg_ops = &dp_ops_cfg;
	soc->cdp_soc.ops->flowctl_ops = &dp_ops_flowctl;
	soc->cdp_soc.ops->l_flowctl_ops = &dp_ops_l_flowctl;
	soc->cdp_soc.ops->throttle_ops = &dp_ops_throttle;
#endif
#ifdef IPA_OFFLOAD
	soc->cdp_soc.ops->ipa_ops = &dp_ops_ipa;
#endif
#ifdef DP_POWER_SAVE
	soc->cdp_soc.ops->bus_ops = &dp_ops_bus;
#endif
#if defined(WLAN_CFR_ENABLE) && defined(WLAN_ENH_CFR_ENABLE)
	soc->cdp_soc.ops->cfr_ops = &dp_ops_cfr;
#endif
#ifdef WLAN_SUPPORT_MSCS
	soc->cdp_soc.ops->mscs_ops = &dp_ops_mscs;
#endif
#ifdef WLAN_SUPPORT_MESH_LATENCY
	soc->cdp_soc.ops->mesh_latency_ops = &dp_ops_mesh_latency;
#endif
};

/*
 * dp_soc_set_txrx_ring_map()
 * @dp_soc: DP handler for soc
 *
 * Return: Void
 */
void dp_soc_set_txrx_ring_map(struct dp_soc *soc)
{
	uint32_t i;
	for (i = 0; i < WLAN_CFG_INT_NUM_CONTEXTS; i++) {
		soc->tx_ring_map[i] = dp_cpu_ring_map[DP_NSS_DEFAULT_MAP][i];
	}
}

qdf_export_symbol(dp_soc_set_txrx_ring_map);

#if defined(QCA_WIFI_QCA8074) || defined(QCA_WIFI_QCA6018) || \
	defined(QCA_WIFI_QCA5018) || defined(QCA_WIFI_QCA9574)
/**
 * dp_soc_attach_wifi3() - Attach txrx SOC
 * @ctrl_psoc: Opaque SOC handle from control plane
 * @params: SOC attach params
 *
 * Return: DP SOC handle on success, NULL on failure
 */
struct cdp_soc_t *
dp_soc_attach_wifi3(struct cdp_ctrl_objmgr_psoc *ctrl_psoc,
		    struct cdp_soc_attach_params *params)
{
	struct dp_soc *dp_soc = NULL;

	dp_soc = dp_soc_attach(ctrl_psoc, params);

	return dp_soc_to_cdp_soc_t(dp_soc);
}

static inline void dp_soc_set_def_pdev(struct dp_soc *soc)
{
	int lmac_id;

	for (lmac_id = 0; lmac_id < MAX_NUM_LMAC_HW; lmac_id++) {
		/*Set default host PDEV ID for lmac_id*/
		wlan_cfg_set_pdev_idx(soc->wlan_cfg_ctx,
				      INVALID_PDEV_ID, lmac_id);
	}
}

static uint32_t
dp_get_link_desc_id_start(uint16_t arch_id)
{
	switch (arch_id) {
	case CDP_ARCH_TYPE_LI:
		return LINK_DESC_ID_START_21_BITS_COOKIE;
	case CDP_ARCH_TYPE_BE:
		return LINK_DESC_ID_START_20_BITS_COOKIE;
	default:
		dp_err("unkonwn arch_id 0x%x", arch_id);
		QDF_BUG(0);
		return LINK_DESC_ID_START_21_BITS_COOKIE;
	}
}

/**
 * dp_soc_attach() - Attach txrx SOC
 * @ctrl_psoc: Opaque SOC handle from control plane
 * @params: SOC attach params
 *
 * Return: DP SOC handle on success, NULL on failure
 */
static struct dp_soc *
dp_soc_attach(struct cdp_ctrl_objmgr_psoc *ctrl_psoc,
	      struct cdp_soc_attach_params *params)
{
	int int_ctx;
	struct dp_soc *soc =  NULL;
	uint16_t arch_id;
	struct hif_opaque_softc *hif_handle = params->hif_handle;
	qdf_device_t qdf_osdev = params->qdf_osdev;
	struct ol_if_ops *ol_ops = params->ol_ops;
	uint16_t device_id = params->device_id;

	if (!hif_handle) {
		dp_err("HIF handle is NULL");
		goto fail0;
	}
	arch_id = cdp_get_arch_type_from_devid(device_id);
	soc = qdf_mem_malloc(dp_get_soc_context_size(device_id));
	if (!soc) {
		dp_err("DP SOC memory allocation failed");
		goto fail0;
	}

	dp_info("soc memory allocated %pk", soc);
	soc->hif_handle = hif_handle;
	soc->hal_soc = hif_get_hal_handle(soc->hif_handle);
	if (!soc->hal_soc)
		goto fail1;

	hif_get_cmem_info(soc->hif_handle,
			  &soc->cmem_base,
			  &soc->cmem_size);
	int_ctx = 0;
	soc->device_id = device_id;
	soc->cdp_soc.ops =
		(struct cdp_ops *)qdf_mem_malloc(sizeof(struct cdp_ops));
	if (!soc->cdp_soc.ops)
		goto fail1;

	dp_soc_txrx_ops_attach(soc);
	soc->cdp_soc.ol_ops = ol_ops;
	soc->ctrl_psoc = ctrl_psoc;
	soc->osdev = qdf_osdev;
	soc->num_hw_dscp_tid_map = HAL_MAX_HW_DSCP_TID_MAPS;
	hal_rx_get_tlv_size(soc->hal_soc, &soc->rx_pkt_tlv_size,
			    &soc->rx_mon_pkt_tlv_size);
	soc->idle_link_bm_id = hal_get_idle_link_bm_id(soc->hal_soc,
						       params->mlo_chip_id);
	soc->features.dmac_cmn_src_rxbuf_ring_enabled =
		hal_dmac_cmn_src_rxbuf_ring_get(soc->hal_soc);
	soc->arch_id = arch_id;
	soc->link_desc_id_start =
			dp_get_link_desc_id_start(soc->arch_id);
	dp_configure_arch_ops(soc);

	/* Reset wbm sg list and flags */
	dp_rx_wbm_sg_list_reset(soc);

	dp_soc_tx_hw_desc_history_attach(soc);
	dp_soc_rx_history_attach(soc);
	dp_soc_tx_history_attach(soc);
	wlan_set_srng_cfg(&soc->wlan_srng_cfg);
	soc->wlan_cfg_ctx = wlan_cfg_soc_attach(soc->ctrl_psoc);
	if (!soc->wlan_cfg_ctx) {
		dp_err("wlan_cfg_ctx failed\n");
		goto fail2;
	}
	dp_soc_cfg_attach(soc);

	if (dp_hw_link_desc_pool_banks_alloc(soc, WLAN_INVALID_PDEV_ID)) {
		dp_err("failed to allocate link desc pool banks");
		goto fail3;
	}

	if (dp_hw_link_desc_ring_alloc(soc)) {
		dp_err("failed to allocate link_desc_ring");
		goto fail4;
	}

	if (!QDF_IS_STATUS_SUCCESS(soc->arch_ops.txrx_soc_attach(soc,
								 params))) {
		dp_err("unable to do target specific attach");
		goto fail5;
	}

	if (dp_soc_srng_alloc(soc)) {
		dp_err("failed to allocate soc srng rings");
		goto fail6;
	}

	if (dp_soc_tx_desc_sw_pools_alloc(soc)) {
		dp_err("dp_soc_tx_desc_sw_pools_alloc failed");
		goto fail7;
	}

	if (!dp_monitor_modularized_enable()) {
		if (dp_mon_soc_attach_wrapper(soc)) {
			dp_err("failed to attach monitor");
			goto fail8;
		}
	}

	if (dp_sysfs_initialize_stats(soc) != QDF_STATUS_SUCCESS) {
		dp_err("failed to initialize dp stats sysfs file");
		dp_sysfs_deinitialize_stats(soc);
	}

	dp_soc_swlm_attach(soc);
	dp_soc_set_interrupt_mode(soc);
	dp_soc_set_def_pdev(soc);

	dp_info("Mem stats: DMA = %u HEAP = %u SKB = %u",
		qdf_dma_mem_stats_read(),
		qdf_heap_mem_stats_read(),
		qdf_skb_total_mem_stats_read());

	return soc;
fail8:
	dp_soc_tx_desc_sw_pools_free(soc);
fail7:
	dp_soc_srng_free(soc);
fail6:
	soc->arch_ops.txrx_soc_detach(soc);
fail5:
	dp_hw_link_desc_ring_free(soc);
fail4:
	dp_hw_link_desc_pool_banks_free(soc, WLAN_INVALID_PDEV_ID);
fail3:
	wlan_cfg_soc_detach(soc->wlan_cfg_ctx);
fail2:
	qdf_mem_free(soc->cdp_soc.ops);
fail1:
	qdf_mem_free(soc);
fail0:
	return NULL;
}

/**
 * dp_soc_init() - Initialize txrx SOC
 * @dp_soc: Opaque DP SOC handle
 * @htc_handle: Opaque HTC handle
 * @hif_handle: Opaque HIF handle
 *
 * Return: DP SOC handle on success, NULL on failure
 */
void *dp_soc_init(struct dp_soc *soc, HTC_HANDLE htc_handle,
		  struct hif_opaque_softc *hif_handle)
{
	struct htt_soc *htt_soc = (struct htt_soc *)soc->htt_handle;
	bool is_monitor_mode = false;
	struct hal_reo_params reo_params;
	uint8_t i;
	int num_dp_msi;

	wlan_minidump_log(soc, sizeof(*soc), soc->ctrl_psoc,
			  WLAN_MD_DP_SOC, "dp_soc");

	soc->hif_handle = hif_handle;

	soc->hal_soc = hif_get_hal_handle(soc->hif_handle);
	if (!soc->hal_soc)
		goto fail0;

	if (!QDF_IS_STATUS_SUCCESS(soc->arch_ops.txrx_soc_init(soc))) {
		dp_err("unable to do target specific init");
		goto fail0;
	}

	htt_soc = htt_soc_attach(soc, htc_handle);
	if (!htt_soc)
		goto fail1;

	soc->htt_handle = htt_soc;

	if (htt_soc_htc_prealloc(htt_soc) != QDF_STATUS_SUCCESS)
		goto fail2;

	htt_set_htc_handle(htt_soc, htc_handle);

	dp_soc_cfg_init(soc);

	dp_monitor_soc_cfg_init(soc);
	/* Reset/Initialize wbm sg list and flags */
	dp_rx_wbm_sg_list_reset(soc);

	/* Note: Any SRNG ring initialization should happen only after
	 * Interrupt mode is set and followed by filling up the
	 * interrupt mask. IT SHOULD ALWAYS BE IN THIS ORDER.
	 */
	dp_soc_set_interrupt_mode(soc);
	if (soc->cdp_soc.ol_ops->get_con_mode &&
	    soc->cdp_soc.ol_ops->get_con_mode() ==
	    QDF_GLOBAL_MONITOR_MODE)
		is_monitor_mode = true;

	num_dp_msi = dp_get_num_msi_available(soc, soc->intr_mode);
	if (num_dp_msi < 0) {
		dp_init_err("%pK: dp_interrupt assignment failed", soc);
		goto fail3;
	}

	wlan_cfg_fill_interrupt_mask(soc->wlan_cfg_ctx, num_dp_msi,
				     soc->intr_mode, is_monitor_mode);

	/* initialize WBM_IDLE_LINK ring */
	if (dp_hw_link_desc_ring_init(soc)) {
		dp_init_err("%pK: dp_hw_link_desc_ring_init failed", soc);
		goto fail3;
	}

	dp_link_desc_ring_replenish(soc, WLAN_INVALID_PDEV_ID);

	if (dp_soc_srng_init(soc)) {
		dp_init_err("%pK: dp_soc_srng_init failed", soc);
		goto fail4;
	}

	if (htt_soc_initialize(soc->htt_handle, soc->ctrl_psoc,
			       htt_get_htc_handle(htt_soc),
			       soc->hal_soc, soc->osdev) == NULL)
		goto fail5;

	/* Initialize descriptors in TCL Rings */
	for (i = 0; i < soc->num_tcl_data_rings; i++) {
		hal_tx_init_data_ring(soc->hal_soc,
				      soc->tcl_data_ring[i].hal_srng);
	}

	if (dp_soc_tx_desc_sw_pools_init(soc)) {
		dp_init_err("%pK: dp_tx_soc_attach failed", soc);
		goto fail6;
	}

	wlan_cfg_set_rx_hash(soc->wlan_cfg_ctx,
			     cfg_get(soc->ctrl_psoc, CFG_DP_RX_HASH));
	soc->cce_disable = false;
	soc->max_ast_ageout_count = MAX_AST_AGEOUT_COUNT;

	soc->sta_mode_search_policy = DP_TX_ADDR_SEARCH_ADDR_POLICY;
	qdf_mem_zero(&soc->vdev_id_map, sizeof(soc->vdev_id_map));
	qdf_spinlock_create(&soc->vdev_map_lock);
	qdf_atomic_init(&soc->num_tx_outstanding);
	qdf_atomic_init(&soc->num_tx_exception);
	soc->num_tx_allowed =
		wlan_cfg_get_dp_soc_tx_device_limit(soc->wlan_cfg_ctx);

	if (soc->cdp_soc.ol_ops->get_dp_cfg_param) {
		int ret = soc->cdp_soc.ol_ops->get_dp_cfg_param(soc->ctrl_psoc,
				CDP_CFG_MAX_PEER_ID);

		if (ret != -EINVAL)
			wlan_cfg_set_max_peer_id(soc->wlan_cfg_ctx, ret);

		ret = soc->cdp_soc.ol_ops->get_dp_cfg_param(soc->ctrl_psoc,
				CDP_CFG_CCE_DISABLE);
		if (ret == 1)
			soc->cce_disable = true;
	}

	/*
	 * Skip registering hw ring interrupts for WMAC2 on IPQ6018
	 * and IPQ5018 WMAC2 is not there in these platforms.
	 */
	if (hal_get_target_type(soc->hal_soc) == TARGET_TYPE_QCA6018 ||
	    soc->disable_mac2_intr)
		dp_soc_disable_unused_mac_intr_mask(soc, 0x2);

	/*
	 * Skip registering hw ring interrupts for WMAC1 on IPQ5018
	 * WMAC1 is not there in this platform.
	 */
	if (soc->disable_mac1_intr)
		dp_soc_disable_unused_mac_intr_mask(soc, 0x1);

	/* Setup HW REO */
	qdf_mem_zero(&reo_params, sizeof(reo_params));

	if (wlan_cfg_is_rx_hash_enabled(soc->wlan_cfg_ctx)) {
		/*
		 * Reo ring remap is not required if both radios
		 * are offloaded to NSS
		 */
		if (soc->arch_ops.reo_remap_config(soc, &reo_params.remap0,
						   &reo_params.remap1,
						   &reo_params.remap2))
			reo_params.rx_hash_enabled = true;
		else
			reo_params.rx_hash_enabled = false;
	}

	/* setup the global rx defrag waitlist */
	TAILQ_INIT(&soc->rx.defrag.waitlist);
	soc->rx.defrag.timeout_ms =
		wlan_cfg_get_rx_defrag_min_timeout(soc->wlan_cfg_ctx);
	soc->rx.defrag.next_flush_ms = 0;
	soc->rx.flags.defrag_timeout_check =
		wlan_cfg_get_defrag_timeout_check(soc->wlan_cfg_ctx);
	qdf_spinlock_create(&soc->rx.defrag.defrag_lock);

	/*
	 * set the fragment destination ring
	 */
	dp_reo_frag_dst_set(soc, &reo_params.frag_dst_ring);

	if (wlan_cfg_get_dp_soc_nss_cfg(soc->wlan_cfg_ctx))
		reo_params.alt_dst_ind_0 = REO_REMAP_RELEASE;

	hal_reo_setup(soc->hal_soc, &reo_params);

	hal_reo_set_err_dst_remap(soc->hal_soc);

	soc->features.pn_in_reo_dest = hal_reo_enable_pn_in_dest(soc->hal_soc);

	qdf_atomic_set(&soc->cmn_init_done, 1);

	qdf_nbuf_queue_init(&soc->htt_stats.msg);

	qdf_spinlock_create(&soc->ast_lock);
	dp_peer_mec_spinlock_create(soc);

	qdf_spinlock_create(&soc->reo_desc_freelist_lock);
	qdf_list_create(&soc->reo_desc_freelist, REO_DESC_FREELIST_SIZE);
	INIT_RX_HW_STATS_LOCK(soc);

	qdf_nbuf_queue_init(&soc->invalid_buf_queue);
	/* fill the tx/rx cpu ring map*/
	dp_soc_set_txrx_ring_map(soc);

	TAILQ_INIT(&soc->inactive_peer_list);
	qdf_spinlock_create(&soc->inactive_peer_list_lock);
	TAILQ_INIT(&soc->inactive_vdev_list);
	qdf_spinlock_create(&soc->inactive_vdev_list_lock);
	qdf_spinlock_create(&soc->htt_stats.lock);
	/* initialize work queue for stats processing */
	qdf_create_work(0, &soc->htt_stats.work, htt_t2h_stats_handler, soc);

	dp_reo_desc_deferred_freelist_create(soc);

	dp_info("Mem stats: DMA = %u HEAP = %u SKB = %u",
		qdf_dma_mem_stats_read(),
		qdf_heap_mem_stats_read(),
		qdf_skb_total_mem_stats_read());

	soc->vdev_stats_id_map = 0;

	return soc;
fail6:
	htt_soc_htc_dealloc(soc->htt_handle);
fail5:
	dp_soc_srng_deinit(soc);
fail4:
	dp_hw_link_desc_ring_deinit(soc);
fail3:
	htt_htc_pkt_pool_free(htt_soc);
fail2:
	htt_soc_detach(htt_soc);
fail1:
	soc->arch_ops.txrx_soc_deinit(soc);
fail0:
	return NULL;
}

/**
 * dp_soc_init_wifi3() - Initialize txrx SOC
 * @soc: Opaque DP SOC handle
 * @ctrl_psoc: Opaque SOC handle from control plane(Unused)
 * @hif_handle: Opaque HIF handle
 * @htc_handle: Opaque HTC handle
 * @qdf_osdev: QDF device (Unused)
 * @ol_ops: Offload Operations (Unused)
 * @device_id: Device ID (Unused)
 *
 * Return: DP SOC handle on success, NULL on failure
 */
void *dp_soc_init_wifi3(struct cdp_soc_t *soc,
			struct cdp_ctrl_objmgr_psoc *ctrl_psoc,
			struct hif_opaque_softc *hif_handle,
			HTC_HANDLE htc_handle, qdf_device_t qdf_osdev,
			struct ol_if_ops *ol_ops, uint16_t device_id)
{
	return dp_soc_init((struct dp_soc *)soc, htc_handle, hif_handle);
}

#endif

/*
 * dp_get_pdev_for_mac_id() -  Return pdev for mac_id
 *
 * @soc: handle to DP soc
 * @mac_id: MAC id
 *
 * Return: Return pdev corresponding to MAC
 */
void *dp_get_pdev_for_mac_id(struct dp_soc *soc, uint32_t mac_id)
{
	if (wlan_cfg_per_pdev_lmac_ring(soc->wlan_cfg_ctx))
		return (mac_id < MAX_PDEV_CNT) ? soc->pdev_list[mac_id] : NULL;

	/* Typically for MCL as there only 1 PDEV*/
	return soc->pdev_list[0];
}

void dp_update_num_mac_rings_for_dbs(struct dp_soc *soc,
				     int *max_mac_rings)
{
	bool dbs_enable = false;

	if (soc->cdp_soc.ol_ops->is_hw_dbs_capable)
		dbs_enable = soc->cdp_soc.ol_ops->
				is_hw_dbs_capable((void *)soc->ctrl_psoc);

	*max_mac_rings = dbs_enable ? (*max_mac_rings) : 1;
	dp_info("dbs_enable %d, max_mac_rings %d",
		dbs_enable, *max_mac_rings);
}

qdf_export_symbol(dp_update_num_mac_rings_for_dbs);

#if defined(WLAN_CFR_ENABLE) && defined(WLAN_ENH_CFR_ENABLE)
/**
 * dp_get_cfr_rcc() - get cfr rcc config
 * @soc_hdl: Datapath soc handle
 * @pdev_id: id of objmgr pdev
 *
 * Return: true/false based on cfr mode setting
 */
static
bool dp_get_cfr_rcc(struct cdp_soc_t *soc_hdl, uint8_t pdev_id)
{
	struct dp_soc *soc = cdp_soc_t_to_dp_soc(soc_hdl);
	struct dp_pdev *pdev = NULL;

	pdev = dp_get_pdev_from_soc_pdev_id_wifi3(soc, pdev_id);
	if (!pdev) {
		dp_err("pdev is NULL");
		return false;
	}

	return pdev->cfr_rcc_mode;
}

/**
 * dp_set_cfr_rcc() - enable/disable cfr rcc config
 * @soc_hdl: Datapath soc handle
 * @pdev_id: id of objmgr pdev
 * @enable: Enable/Disable cfr rcc mode
 *
 * Return: none
 */
static
void dp_set_cfr_rcc(struct cdp_soc_t *soc_hdl, uint8_t pdev_id, bool enable)
{
	struct dp_soc *soc = cdp_soc_t_to_dp_soc(soc_hdl);
	struct dp_pdev *pdev = NULL;

	pdev = dp_get_pdev_from_soc_pdev_id_wifi3(soc, pdev_id);
	if (!pdev) {
		dp_err("pdev is NULL");
		return;
	}

	pdev->cfr_rcc_mode = enable;
}

/*
 * dp_get_cfr_dbg_stats - Get the debug statistics for CFR
 * @soc_hdl: Datapath soc handle
 * @pdev_id: id of data path pdev handle
 * @cfr_rcc_stats: CFR RCC debug statistics buffer
 *
 * Return: none
 */
static inline void
dp_get_cfr_dbg_stats(struct cdp_soc_t *soc_hdl, uint8_t pdev_id,
		     struct cdp_cfr_rcc_stats *cfr_rcc_stats)
{
	struct dp_soc *soc = cdp_soc_t_to_dp_soc(soc_hdl);
	struct dp_pdev *pdev = dp_get_pdev_from_soc_pdev_id_wifi3(soc, pdev_id);

	if (!pdev) {
		dp_err("Invalid pdev");
		return;
	}

	qdf_mem_copy(cfr_rcc_stats, &pdev->stats.rcc,
		     sizeof(struct cdp_cfr_rcc_stats));
}

/*
 * dp_clear_cfr_dbg_stats - Clear debug statistics for CFR
 * @soc_hdl: Datapath soc handle
 * @pdev_id: id of data path pdev handle
 *
 * Return: none
 */
static void dp_clear_cfr_dbg_stats(struct cdp_soc_t *soc_hdl,
				   uint8_t pdev_id)
{
	struct dp_soc *soc = cdp_soc_t_to_dp_soc(soc_hdl);
	struct dp_pdev *pdev = dp_get_pdev_from_soc_pdev_id_wifi3(soc, pdev_id);

	if (!pdev) {
		dp_err("dp pdev is NULL");
		return;
	}

	qdf_mem_zero(&pdev->stats.rcc, sizeof(pdev->stats.rcc));
}
#endif

/**
 * dp_bucket_index() - Return index from array
 *
 * @delay: delay measured
 * @array: array used to index corresponding delay
 *
 * Return: index
 */
static uint8_t dp_bucket_index(uint32_t delay, uint16_t *array)
{
	uint8_t i = CDP_DELAY_BUCKET_0;

	for (; i < CDP_DELAY_BUCKET_MAX - 1; i++) {
		if (delay >= array[i] && delay <= array[i + 1])
			return i;
	}

	return (CDP_DELAY_BUCKET_MAX - 1);
}

#ifdef HW_TX_DELAY_STATS_ENABLE
/*
 * cdp_fw_to_hw_delay_range
 * Fw to hw delay ranges in milliseconds
 */
static uint16_t cdp_fw_to_hw_delay[CDP_DELAY_BUCKET_MAX] = {
	0, 10, 20, 30, 40, 50, 60, 70, 80, 90, 100, 250, 500};
#else
static uint16_t cdp_fw_to_hw_delay[CDP_DELAY_BUCKET_MAX] = {
	0, 2, 4, 6, 8, 10, 20, 30, 40, 50, 100, 250, 500};
#endif

/*
 * cdp_sw_enq_delay_range
 * Software enqueue delay ranges in milliseconds
 */
static uint16_t cdp_sw_enq_delay[CDP_DELAY_BUCKET_MAX] = {
	0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};

/*
 * cdp_intfrm_delay_range
 * Interframe delay ranges in milliseconds
 */
static uint16_t cdp_intfrm_delay[CDP_DELAY_BUCKET_MAX] = {
	0, 5, 10, 15, 20, 25, 30, 35, 40, 45, 50, 55, 60};

/**
 * dp_fill_delay_buckets() - Fill delay statistics bucket for each
 *				type of delay
 * @tstats: tid tx stats
 * @rstats: tid rx stats
 * @delay: delay in ms
 * @tid: tid value
 * @mode: type of tx delay mode
 * @ring_id: ring number
 * Return: pointer to cdp_delay_stats structure
 */
static struct cdp_delay_stats *
dp_fill_delay_buckets(struct cdp_tid_tx_stats *tstats,
		      struct cdp_tid_rx_stats *rstats, uint32_t delay,
		      uint8_t tid, uint8_t mode, uint8_t ring_id)
{
	uint8_t delay_index = 0;
	struct cdp_delay_stats *stats = NULL;

	/*
	 * Update delay stats in proper bucket
	 */
	switch (mode) {
	/* Software Enqueue delay ranges */
	case CDP_DELAY_STATS_SW_ENQ:
		if (!tstats)
			break;

		delay_index = dp_bucket_index(delay, cdp_sw_enq_delay);
		tstats->swq_delay.delay_bucket[delay_index]++;
		stats = &tstats->swq_delay;
		break;

	/* Tx Completion delay ranges */
	case CDP_DELAY_STATS_FW_HW_TRANSMIT:
		if (!tstats)
			break;

		delay_index = dp_bucket_index(delay, cdp_fw_to_hw_delay);
		tstats->hwtx_delay.delay_bucket[delay_index]++;
		stats = &tstats->hwtx_delay;
		break;

	/* Interframe tx delay ranges */
	case CDP_DELAY_STATS_TX_INTERFRAME:
		if (!tstats)
			break;

		delay_index = dp_bucket_index(delay, cdp_intfrm_delay);
		tstats->intfrm_delay.delay_bucket[delay_index]++;
		stats = &tstats->intfrm_delay;
		break;

	/* Interframe rx delay ranges */
	case CDP_DELAY_STATS_RX_INTERFRAME:
		if (!rstats)
			break;

		delay_index = dp_bucket_index(delay, cdp_intfrm_delay);
		rstats->intfrm_delay.delay_bucket[delay_index]++;
		stats = &rstats->intfrm_delay;
		break;

	/* Ring reap to indication to network stack */
	case CDP_DELAY_STATS_REAP_STACK:
		if (!rstats)
			break;

		delay_index = dp_bucket_index(delay, cdp_intfrm_delay);
		rstats->to_stack_delay.delay_bucket[delay_index]++;
		stats = &rstats->to_stack_delay;
		break;
	default:
		dp_debug("Incorrect delay mode: %d", mode);
	}

	return stats;
}

void dp_update_delay_stats(struct cdp_tid_tx_stats *tstats,
			   struct cdp_tid_rx_stats *rstats, uint32_t delay,
			   uint8_t tid, uint8_t mode, uint8_t ring_id)
{
	struct cdp_delay_stats *dstats = NULL;

	/*
	 * Delay ranges are different for different delay modes
	 * Get the correct index to update delay bucket
	 */
	dstats = dp_fill_delay_buckets(tstats, rstats, delay, tid, mode,
				       ring_id);
	if (qdf_unlikely(!dstats))
		return;

	if (delay != 0) {
		/*
		 * Compute minimum,average and maximum
		 * delay
		 */
		if (delay < dstats->min_delay)
			dstats->min_delay = delay;

		if (delay > dstats->max_delay)
			dstats->max_delay = delay;

		/*
		 * Average over delay measured till now
		 */
		if (!dstats->avg_delay)
			dstats->avg_delay = delay;
		else
			dstats->avg_delay = ((delay + dstats->avg_delay) >> 1);
	}
}

/**
 * dp_get_peer_mac_list(): function to get peer mac list of vdev
 * @soc: Datapath soc handle
 * @vdev_id: vdev id
 * @newmac: Table of the clients mac
 * @mac_cnt: No. of MACs required
 * @limit: Limit the number of clients
 *
 * return: no of clients
 */
uint16_t dp_get_peer_mac_list(ol_txrx_soc_handle soc, uint8_t vdev_id,
			      u_int8_t newmac[][QDF_MAC_ADDR_SIZE],
			      u_int16_t mac_cnt, bool limit)
{
	struct dp_soc *dp_soc = (struct dp_soc *)soc;
	struct dp_vdev *vdev =
		dp_vdev_get_ref_by_id(dp_soc, vdev_id, DP_MOD_ID_CDP);
	struct dp_peer *peer;
	uint16_t new_mac_cnt = 0;

	if (!vdev)
		return new_mac_cnt;

	if (limit && (vdev->num_peers > mac_cnt))
		return 0;

	qdf_spin_lock_bh(&vdev->peer_list_lock);
	TAILQ_FOREACH(peer, &vdev->peer_list, peer_list_elem) {
		if (peer->bss_peer)
			continue;
		if (new_mac_cnt < mac_cnt) {
			WLAN_ADDR_COPY(newmac[new_mac_cnt], peer->mac_addr.raw);
			new_mac_cnt++;
		}
	}
	qdf_spin_unlock_bh(&vdev->peer_list_lock);
	dp_vdev_unref_delete(dp_soc, vdev, DP_MOD_ID_CDP);
	return new_mac_cnt;
}

#ifdef QCA_SUPPORT_WDS_EXTENDED
uint16_t dp_wds_ext_get_peer_id(ol_txrx_soc_handle soc,
				uint8_t vdev_id,
				uint8_t *mac)
{
	struct dp_peer *peer = dp_peer_find_hash_find((struct dp_soc *)soc,
						       mac, 0, vdev_id,
						       DP_MOD_ID_CDP);
	uint16_t peer_id = HTT_INVALID_PEER;

	if (!peer) {
		dp_cdp_debug("%pK: Peer is NULL!\n", (struct dp_soc *)soc);
		return peer_id;
	}

	peer_id = peer->peer_id;
	dp_peer_unref_delete(peer, DP_MOD_ID_CDP);
	return peer_id;
}

QDF_STATUS dp_wds_ext_set_peer_rx(ol_txrx_soc_handle soc,
				  uint8_t vdev_id,
				  uint8_t *mac,
				  ol_txrx_rx_fp rx,
				  ol_osif_peer_handle osif_peer)
{
	struct dp_peer *peer = dp_peer_find_hash_find((struct dp_soc *)soc,
						       mac, 0, vdev_id,
						       DP_MOD_ID_CDP);
	QDF_STATUS status = QDF_STATUS_E_INVAL;

	if (!peer) {
		dp_cdp_debug("%pK: Peer is NULL!\n", (struct dp_soc *)soc);
		return status;
	}

	if (rx) {
		if (peer->osif_rx) {
		    status = QDF_STATUS_E_ALREADY;
		} else {
		    peer->osif_rx = rx;
		    status = QDF_STATUS_SUCCESS;
		}
	} else {
		if (peer->osif_rx) {
		    peer->osif_rx = NULL;
		    status = QDF_STATUS_SUCCESS;
		} else {
		    status = QDF_STATUS_E_ALREADY;
		}
	}

	peer->wds_ext.osif_peer = osif_peer;
	dp_peer_unref_delete(peer, DP_MOD_ID_CDP);

	return status;
}
#endif /* QCA_SUPPORT_WDS_EXTENDED */

/**
 * dp_pdev_srng_deinit() - de-initialize all pdev srng ring including
 *			   monitor rings
 * @pdev: Datapath pdev handle
 *
 */
static void dp_pdev_srng_deinit(struct dp_pdev *pdev)
{
	struct dp_soc *soc = pdev->soc;
	uint8_t i;

	if (!soc->features.dmac_cmn_src_rxbuf_ring_enabled)
		dp_srng_deinit(soc, &soc->rx_refill_buf_ring[pdev->lmac_id],
			       RXDMA_BUF,
			       pdev->lmac_id);

	if (!soc->rxdma2sw_rings_not_supported) {
		for (i = 0;
		     i < soc->wlan_cfg_ctx->num_rxdma_dst_rings_per_pdev; i++) {
			int lmac_id = dp_get_lmac_id_for_pdev_id(soc, i,
								 pdev->pdev_id);

			wlan_minidump_remove(soc->rxdma_err_dst_ring[lmac_id].
							base_vaddr_unaligned,
					     soc->rxdma_err_dst_ring[lmac_id].
								alloc_size,
					     soc->ctrl_psoc,
					     WLAN_MD_DP_SRNG_RXDMA_ERR_DST,
					     "rxdma_err_dst");
			dp_srng_deinit(soc, &soc->rxdma_err_dst_ring[lmac_id],
				       RXDMA_DST, lmac_id);
		}
	}


}

/**
 * dp_pdev_srng_init() - initialize all pdev srng rings including
 *			   monitor rings
 * @pdev: Datapath pdev handle
 *
 * return: QDF_STATUS_SUCCESS on success
 *	   QDF_STATUS_E_NOMEM on failure
 */
static QDF_STATUS dp_pdev_srng_init(struct dp_pdev *pdev)
{
	struct dp_soc *soc = pdev->soc;
	struct wlan_cfg_dp_soc_ctxt *soc_cfg_ctx;
	uint32_t i;

	soc_cfg_ctx = soc->wlan_cfg_ctx;

	if (!soc->features.dmac_cmn_src_rxbuf_ring_enabled) {
		if (dp_srng_init(soc, &soc->rx_refill_buf_ring[pdev->lmac_id],
				 RXDMA_BUF, 0, pdev->lmac_id)) {
			dp_init_err("%pK: dp_srng_init failed rx refill ring",
				    soc);
			goto fail1;
		}
	}

	/* LMAC RxDMA to SW Rings configuration */
	if (!wlan_cfg_per_pdev_lmac_ring(soc_cfg_ctx))
		/* Only valid for MCL */
		pdev = soc->pdev_list[0];

	if (!soc->rxdma2sw_rings_not_supported) {
		for (i = 0;
		     i < soc->wlan_cfg_ctx->num_rxdma_dst_rings_per_pdev; i++) {
			int lmac_id = dp_get_lmac_id_for_pdev_id(soc, i,
								 pdev->pdev_id);
			struct dp_srng *srng =
				&soc->rxdma_err_dst_ring[lmac_id];

			if (srng->hal_srng)
				continue;

			if (dp_srng_init(soc, srng, RXDMA_DST, 0, lmac_id)) {
				dp_init_err("%pK:" RNG_ERR "rxdma_err_dst_ring",
					    soc);
				goto fail1;
			}
			wlan_minidump_log(soc->rxdma_err_dst_ring[lmac_id].
						base_vaddr_unaligned,
					  soc->rxdma_err_dst_ring[lmac_id].
						alloc_size,
					  soc->ctrl_psoc,
					  WLAN_MD_DP_SRNG_RXDMA_ERR_DST,
					  "rxdma_err_dst");
		}
	}
	return QDF_STATUS_SUCCESS;

fail1:
	dp_pdev_srng_deinit(pdev);
	return QDF_STATUS_E_NOMEM;
}

/**
 * dp_pdev_srng_free() - free all pdev srng rings including monitor rings
 * pdev: Datapath pdev handle
 *
 */
static void dp_pdev_srng_free(struct dp_pdev *pdev)
{
	struct dp_soc *soc = pdev->soc;
	uint8_t i;

	if (!soc->features.dmac_cmn_src_rxbuf_ring_enabled)
		dp_srng_free(soc, &soc->rx_refill_buf_ring[pdev->lmac_id]);

	if (!soc->rxdma2sw_rings_not_supported) {
		for (i = 0;
		     i < soc->wlan_cfg_ctx->num_rxdma_dst_rings_per_pdev; i++) {
			int lmac_id = dp_get_lmac_id_for_pdev_id(soc, i,
								 pdev->pdev_id);

			dp_srng_free(soc, &soc->rxdma_err_dst_ring[lmac_id]);
		}
	}
}

/**
 * dp_pdev_srng_alloc() - allocate memory for all pdev srng rings including
 *			  monitor rings
 * pdev: Datapath pdev handle
 *
 * return: QDF_STATUS_SUCCESS on success
 *	   QDF_STATUS_E_NOMEM on failure
 */
static QDF_STATUS dp_pdev_srng_alloc(struct dp_pdev *pdev)
{
	struct dp_soc *soc = pdev->soc;
	struct wlan_cfg_dp_soc_ctxt *soc_cfg_ctx;
	uint32_t ring_size;
	uint32_t i;

	soc_cfg_ctx = soc->wlan_cfg_ctx;

	ring_size = wlan_cfg_get_dp_soc_rxdma_refill_ring_size(soc_cfg_ctx);
	if (!soc->features.dmac_cmn_src_rxbuf_ring_enabled) {
		if (dp_srng_alloc(soc, &soc->rx_refill_buf_ring[pdev->lmac_id],
				  RXDMA_BUF, ring_size, 0)) {
			dp_init_err("%pK: dp_srng_alloc failed rx refill ring",
				    soc);
			goto fail1;
		}
	}

	ring_size = wlan_cfg_get_dp_soc_rxdma_err_dst_ring_size(soc_cfg_ctx);
	/* LMAC RxDMA to SW Rings configuration */
	if (!wlan_cfg_per_pdev_lmac_ring(soc_cfg_ctx))
		/* Only valid for MCL */
		pdev = soc->pdev_list[0];

	if (!soc->rxdma2sw_rings_not_supported) {
		for (i = 0;
		     i < soc->wlan_cfg_ctx->num_rxdma_dst_rings_per_pdev; i++) {
			int lmac_id = dp_get_lmac_id_for_pdev_id(soc, i,
								 pdev->pdev_id);
			struct dp_srng *srng =
				&soc->rxdma_err_dst_ring[lmac_id];

			if (srng->base_vaddr_unaligned)
				continue;

			if (dp_srng_alloc(soc, srng, RXDMA_DST, ring_size, 0)) {
				dp_init_err("%pK:" RNG_ERR "rxdma_err_dst_ring",
					    soc);
				goto fail1;
			}
		}
	}

	return QDF_STATUS_SUCCESS;
fail1:
	dp_pdev_srng_free(pdev);
	return QDF_STATUS_E_NOMEM;
}

#ifndef WLAN_DP_DISABLE_TCL_CMD_CRED_SRNG
static inline QDF_STATUS dp_soc_tcl_cmd_cred_srng_init(struct dp_soc *soc)
{
	QDF_STATUS status;

	if (soc->init_tcl_cmd_cred_ring) {
		status =  dp_srng_init(soc, &soc->tcl_cmd_credit_ring,
				       TCL_CMD_CREDIT, 0, 0);
		if (QDF_IS_STATUS_ERROR(status))
			return status;

		wlan_minidump_log(soc->tcl_cmd_credit_ring.base_vaddr_unaligned,
				  soc->tcl_cmd_credit_ring.alloc_size,
				  soc->ctrl_psoc,
				  WLAN_MD_DP_SRNG_TCL_CMD,
				  "wbm_desc_rel_ring");
	}

	return QDF_STATUS_SUCCESS;
}

static inline void dp_soc_tcl_cmd_cred_srng_deinit(struct dp_soc *soc)
{
	if (soc->init_tcl_cmd_cred_ring) {
		wlan_minidump_remove(soc->tcl_cmd_credit_ring.base_vaddr_unaligned,
				     soc->tcl_cmd_credit_ring.alloc_size,
				     soc->ctrl_psoc, WLAN_MD_DP_SRNG_TCL_CMD,
				     "wbm_desc_rel_ring");
		dp_srng_deinit(soc, &soc->tcl_cmd_credit_ring,
			       TCL_CMD_CREDIT, 0);
	}
}

static inline QDF_STATUS dp_soc_tcl_cmd_cred_srng_alloc(struct dp_soc *soc)
{
	struct wlan_cfg_dp_soc_ctxt *soc_cfg_ctx = soc->wlan_cfg_ctx;
	uint32_t entries;
	QDF_STATUS status;

	entries = wlan_cfg_get_dp_soc_tcl_cmd_credit_ring_size(soc_cfg_ctx);
	if (soc->init_tcl_cmd_cred_ring) {
		status = dp_srng_alloc(soc, &soc->tcl_cmd_credit_ring,
				       TCL_CMD_CREDIT, entries, 0);
		if (QDF_IS_STATUS_ERROR(status))
			return status;
	}

	return QDF_STATUS_SUCCESS;
}

static inline void dp_soc_tcl_cmd_cred_srng_free(struct dp_soc *soc)
{
	if (soc->init_tcl_cmd_cred_ring)
		dp_srng_free(soc, &soc->tcl_cmd_credit_ring);
}

static inline void dp_tx_init_cmd_credit_ring(struct dp_soc *soc)
{
	if (soc->init_tcl_cmd_cred_ring)
		hal_tx_init_cmd_credit_ring(soc->hal_soc,
					    soc->tcl_cmd_credit_ring.hal_srng);
}
#else
static inline QDF_STATUS dp_soc_tcl_cmd_cred_srng_init(struct dp_soc *soc)
{
	return QDF_STATUS_SUCCESS;
}

static inline void dp_soc_tcl_cmd_cred_srng_deinit(struct dp_soc *soc)
{
}

static inline QDF_STATUS dp_soc_tcl_cmd_cred_srng_alloc(struct dp_soc *soc)
{
	return QDF_STATUS_SUCCESS;
}

static inline void dp_soc_tcl_cmd_cred_srng_free(struct dp_soc *soc)
{
}

static inline void dp_tx_init_cmd_credit_ring(struct dp_soc *soc)
{
}
#endif

#ifndef WLAN_DP_DISABLE_TCL_STATUS_SRNG
static inline QDF_STATUS dp_soc_tcl_status_srng_init(struct dp_soc *soc)
{
	QDF_STATUS status;

	status =  dp_srng_init(soc, &soc->tcl_status_ring, TCL_STATUS, 0, 0);
	if (QDF_IS_STATUS_ERROR(status))
		return status;

	wlan_minidump_log(soc->tcl_status_ring.base_vaddr_unaligned,
			  soc->tcl_status_ring.alloc_size,
			  soc->ctrl_psoc,
			  WLAN_MD_DP_SRNG_TCL_STATUS,
			  "wbm_desc_rel_ring");

	return QDF_STATUS_SUCCESS;
}

static inline void dp_soc_tcl_status_srng_deinit(struct dp_soc *soc)
{
	wlan_minidump_remove(soc->tcl_status_ring.base_vaddr_unaligned,
			     soc->tcl_status_ring.alloc_size,
			     soc->ctrl_psoc, WLAN_MD_DP_SRNG_TCL_STATUS,
			     "wbm_desc_rel_ring");
	dp_srng_deinit(soc, &soc->tcl_status_ring, TCL_STATUS, 0);
}

static inline QDF_STATUS dp_soc_tcl_status_srng_alloc(struct dp_soc *soc)
{
	struct wlan_cfg_dp_soc_ctxt *soc_cfg_ctx = soc->wlan_cfg_ctx;
	uint32_t entries;
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	entries = wlan_cfg_get_dp_soc_tcl_status_ring_size(soc_cfg_ctx);
	status = dp_srng_alloc(soc, &soc->tcl_status_ring,
			       TCL_STATUS, entries, 0);

	return status;
}

static inline void dp_soc_tcl_status_srng_free(struct dp_soc *soc)
{
	dp_srng_free(soc, &soc->tcl_status_ring);
}
#else
static inline QDF_STATUS dp_soc_tcl_status_srng_init(struct dp_soc *soc)
{
	return QDF_STATUS_SUCCESS;
}

static inline void dp_soc_tcl_status_srng_deinit(struct dp_soc *soc)
{
}

static inline QDF_STATUS dp_soc_tcl_status_srng_alloc(struct dp_soc *soc)
{
	return QDF_STATUS_SUCCESS;
}

static inline void dp_soc_tcl_status_srng_free(struct dp_soc *soc)
{
}
#endif

/**
 * dp_soc_srng_deinit() - de-initialize soc srng rings
 * @soc: Datapath soc handle
 *
 */
static void dp_soc_srng_deinit(struct dp_soc *soc)
{
	uint32_t i;

	if (soc->arch_ops.txrx_soc_srng_deinit)
		soc->arch_ops.txrx_soc_srng_deinit(soc);

	/* Free the ring memories */
	/* Common rings */
	wlan_minidump_remove(soc->wbm_desc_rel_ring.base_vaddr_unaligned,
			     soc->wbm_desc_rel_ring.alloc_size,
			     soc->ctrl_psoc, WLAN_MD_DP_SRNG_WBM_DESC_REL,
			     "wbm_desc_rel_ring");
	dp_srng_deinit(soc, &soc->wbm_desc_rel_ring, SW2WBM_RELEASE, 0);

	/* Tx data rings */
	for (i = 0; i < soc->num_tcl_data_rings; i++)
		dp_deinit_tx_pair_by_index(soc, i);

	if (wlan_cfg_is_ipa_enabled(soc->wlan_cfg_ctx)) {
		dp_deinit_tx_pair_by_index(soc, IPA_TCL_DATA_RING_IDX);
		dp_ipa_deinit_alt_tx_ring(soc);
	}

	/* TCL command and status rings */
	dp_soc_tcl_cmd_cred_srng_deinit(soc);
	dp_soc_tcl_status_srng_deinit(soc);

	for (i = 0; i < soc->num_reo_dest_rings; i++) {
		/* TODO: Get number of rings and ring sizes
		 * from wlan_cfg
		 */
		wlan_minidump_remove(soc->reo_dest_ring[i].base_vaddr_unaligned,
				     soc->reo_dest_ring[i].alloc_size,
				     soc->ctrl_psoc, WLAN_MD_DP_SRNG_REO_DEST,
				     "reo_dest_ring");
		dp_srng_deinit(soc, &soc->reo_dest_ring[i], REO_DST, i);
	}

	/* REO reinjection ring */
	wlan_minidump_remove(soc->reo_reinject_ring.base_vaddr_unaligned,
			     soc->reo_reinject_ring.alloc_size,
			     soc->ctrl_psoc, WLAN_MD_DP_SRNG_REO_REINJECT,
			     "reo_reinject_ring");
	dp_srng_deinit(soc, &soc->reo_reinject_ring, REO_REINJECT, 0);

	/* Rx release ring */
	wlan_minidump_remove(soc->rx_rel_ring.base_vaddr_unaligned,
			     soc->rx_rel_ring.alloc_size,
			     soc->ctrl_psoc, WLAN_MD_DP_SRNG_RX_REL,
			     "reo_release_ring");
	dp_srng_deinit(soc, &soc->rx_rel_ring, WBM2SW_RELEASE, 0);

	/* Rx exception ring */
	/* TODO: Better to store ring_type and ring_num in
	 * dp_srng during setup
	 */
	wlan_minidump_remove(soc->reo_exception_ring.base_vaddr_unaligned,
			     soc->reo_exception_ring.alloc_size,
			     soc->ctrl_psoc, WLAN_MD_DP_SRNG_REO_EXCEPTION,
			     "reo_exception_ring");
	dp_srng_deinit(soc, &soc->reo_exception_ring, REO_EXCEPTION, 0);

	/* REO command and status rings */
	wlan_minidump_remove(soc->reo_cmd_ring.base_vaddr_unaligned,
			     soc->reo_cmd_ring.alloc_size,
			     soc->ctrl_psoc, WLAN_MD_DP_SRNG_REO_CMD,
			     "reo_cmd_ring");
	dp_srng_deinit(soc, &soc->reo_cmd_ring, REO_CMD, 0);
	wlan_minidump_remove(soc->reo_status_ring.base_vaddr_unaligned,
			     soc->reo_status_ring.alloc_size,
			     soc->ctrl_psoc, WLAN_MD_DP_SRNG_REO_STATUS,
			     "reo_status_ring");
	dp_srng_deinit(soc, &soc->reo_status_ring, REO_STATUS, 0);
}

/**
 * dp_soc_srng_init() - Initialize soc level srng rings
 * @soc: Datapath soc handle
 *
 * return: QDF_STATUS_SUCCESS on success
 *	   QDF_STATUS_E_FAILURE on failure
 */
static QDF_STATUS dp_soc_srng_init(struct dp_soc *soc)
{
	struct wlan_cfg_dp_soc_ctxt *soc_cfg_ctx;
	uint8_t i;
	uint8_t wbm2_sw_rx_rel_ring_id;

	soc_cfg_ctx = soc->wlan_cfg_ctx;

	dp_enable_verbose_debug(soc);

	/* WBM descriptor release ring */
	if (dp_srng_init(soc, &soc->wbm_desc_rel_ring, SW2WBM_RELEASE, 0, 0)) {
		dp_init_err("%pK: dp_srng_init failed for wbm_desc_rel_ring", soc);
		goto fail1;
	}

	wlan_minidump_log(soc->wbm_desc_rel_ring.base_vaddr_unaligned,
			  soc->wbm_desc_rel_ring.alloc_size,
			  soc->ctrl_psoc,
			  WLAN_MD_DP_SRNG_WBM_DESC_REL,
			  "wbm_desc_rel_ring");

	/* TCL command and status rings */
	if (dp_soc_tcl_cmd_cred_srng_init(soc)) {
		dp_init_err("%pK: dp_srng_init failed for tcl_cmd_ring", soc);
		goto fail1;
	}

	if (dp_soc_tcl_status_srng_init(soc)) {
		dp_init_err("%pK: dp_srng_init failed for tcl_status_ring", soc);
		goto fail1;
	}

	/* REO reinjection ring */
	if (dp_srng_init(soc, &soc->reo_reinject_ring, REO_REINJECT, 0, 0)) {
		dp_init_err("%pK: dp_srng_init failed for reo_reinject_ring", soc);
		goto fail1;
	}

	wlan_minidump_log(soc->reo_reinject_ring.base_vaddr_unaligned,
			  soc->reo_reinject_ring.alloc_size,
			  soc->ctrl_psoc,
			  WLAN_MD_DP_SRNG_REO_REINJECT,
			  "reo_reinject_ring");

	wbm2_sw_rx_rel_ring_id = wlan_cfg_get_rx_rel_ring_id(soc_cfg_ctx);
	/* Rx release ring */
	if (dp_srng_init(soc, &soc->rx_rel_ring, WBM2SW_RELEASE,
			 wbm2_sw_rx_rel_ring_id, 0)) {
		dp_init_err("%pK: dp_srng_init failed for rx_rel_ring", soc);
		goto fail1;
	}

	wlan_minidump_log(soc->rx_rel_ring.base_vaddr_unaligned,
			  soc->rx_rel_ring.alloc_size,
			  soc->ctrl_psoc,
			  WLAN_MD_DP_SRNG_RX_REL,
			  "reo_release_ring");

	/* Rx exception ring */
	if (dp_srng_init(soc, &soc->reo_exception_ring,
			 REO_EXCEPTION, 0, MAX_REO_DEST_RINGS)) {
		dp_init_err("%pK: dp_srng_init failed - reo_exception", soc);
		goto fail1;
	}

	wlan_minidump_log(soc->reo_exception_ring.base_vaddr_unaligned,
			  soc->reo_exception_ring.alloc_size,
			  soc->ctrl_psoc,
			  WLAN_MD_DP_SRNG_REO_EXCEPTION,
			  "reo_exception_ring");

	/* REO command and status rings */
	if (dp_srng_init(soc, &soc->reo_cmd_ring, REO_CMD, 0, 0)) {
		dp_init_err("%pK: dp_srng_init failed for reo_cmd_ring", soc);
		goto fail1;
	}

	wlan_minidump_log(soc->reo_cmd_ring.base_vaddr_unaligned,
			  soc->reo_cmd_ring.alloc_size,
			  soc->ctrl_psoc,
			  WLAN_MD_DP_SRNG_REO_CMD,
			  "reo_cmd_ring");

	hal_reo_init_cmd_ring(soc->hal_soc, soc->reo_cmd_ring.hal_srng);
	TAILQ_INIT(&soc->rx.reo_cmd_list);
	qdf_spinlock_create(&soc->rx.reo_cmd_lock);

	if (dp_srng_init(soc, &soc->reo_status_ring, REO_STATUS, 0, 0)) {
		dp_init_err("%pK: dp_srng_init failed for reo_status_ring", soc);
		goto fail1;
	}

	wlan_minidump_log(soc->reo_status_ring.base_vaddr_unaligned,
			  soc->reo_status_ring.alloc_size,
			  soc->ctrl_psoc,
			  WLAN_MD_DP_SRNG_REO_STATUS,
			  "reo_status_ring");

	for (i = 0; i < soc->num_tcl_data_rings; i++) {
		if (dp_init_tx_ring_pair_by_index(soc, i))
			goto fail1;
	}

	if (wlan_cfg_is_ipa_enabled(soc->wlan_cfg_ctx)) {
		if (dp_init_tx_ring_pair_by_index(soc, IPA_TCL_DATA_RING_IDX))
			goto fail1;

		if (dp_ipa_init_alt_tx_ring(soc))
			goto fail1;
	}

	dp_create_ext_stats_event(soc);

	for (i = 0; i < soc->num_reo_dest_rings; i++) {
		/* Initialize REO destination ring */
		if (dp_srng_init(soc, &soc->reo_dest_ring[i], REO_DST, i, 0)) {
			dp_init_err("%pK: dp_srng_init failed for reo_dest_ringn", soc);
			goto fail1;
		}

		wlan_minidump_log(soc->reo_dest_ring[i].base_vaddr_unaligned,
				  soc->reo_dest_ring[i].alloc_size,
				  soc->ctrl_psoc,
				  WLAN_MD_DP_SRNG_REO_DEST,
				  "reo_dest_ring");
	}

	if (soc->arch_ops.txrx_soc_srng_init) {
		if (soc->arch_ops.txrx_soc_srng_init(soc)) {
			dp_init_err("%pK: dp_srng_init failed for arch rings",
				    soc);
			goto fail1;
		}
	}

	return QDF_STATUS_SUCCESS;
fail1:
	/*
	 * Cleanup will be done as part of soc_detach, which will
	 * be called on pdev attach failure
	 */
	dp_soc_srng_deinit(soc);
	return QDF_STATUS_E_FAILURE;
}

/**
 * dp_soc_srng_free() - free soc level srng rings
 * @soc: Datapath soc handle
 *
 */
static void dp_soc_srng_free(struct dp_soc *soc)
{
	uint32_t i;

	if (soc->arch_ops.txrx_soc_srng_free)
		soc->arch_ops.txrx_soc_srng_free(soc);

	dp_srng_free(soc, &soc->wbm_desc_rel_ring);

	for (i = 0; i < soc->num_tcl_data_rings; i++)
		dp_free_tx_ring_pair_by_index(soc, i);

	/* Free IPA rings for TCL_TX and TCL_COMPL ring */
	if (wlan_cfg_is_ipa_enabled(soc->wlan_cfg_ctx)) {
		dp_free_tx_ring_pair_by_index(soc, IPA_TCL_DATA_RING_IDX);
		dp_ipa_free_alt_tx_ring(soc);
	}

	dp_soc_tcl_cmd_cred_srng_free(soc);
	dp_soc_tcl_status_srng_free(soc);

	for (i = 0; i < soc->num_reo_dest_rings; i++)
		dp_srng_free(soc, &soc->reo_dest_ring[i]);

	dp_srng_free(soc, &soc->reo_reinject_ring);
	dp_srng_free(soc, &soc->rx_rel_ring);

	dp_srng_free(soc, &soc->reo_exception_ring);

	dp_srng_free(soc, &soc->reo_cmd_ring);
	dp_srng_free(soc, &soc->reo_status_ring);
}

/**
 * dp_soc_srng_alloc() - Allocate memory for soc level srng rings
 * @soc: Datapath soc handle
 *
 * return: QDF_STATUS_SUCCESS on success
 *	   QDF_STATUS_E_NOMEM on failure
 */
static QDF_STATUS dp_soc_srng_alloc(struct dp_soc *soc)
{
	uint32_t entries;
	uint32_t i;
	struct wlan_cfg_dp_soc_ctxt *soc_cfg_ctx;
	uint32_t cached = WLAN_CFG_DST_RING_CACHED_DESC;
	uint32_t tx_comp_ring_size, tx_ring_size, reo_dst_ring_size;

	soc_cfg_ctx = soc->wlan_cfg_ctx;

	/* sw2wbm link descriptor release ring */
	entries = wlan_cfg_get_dp_soc_wbm_release_ring_size(soc_cfg_ctx);
	if (dp_srng_alloc(soc, &soc->wbm_desc_rel_ring, SW2WBM_RELEASE,
			  entries, 0)) {
		dp_init_err("%pK: dp_srng_alloc failed for wbm_desc_rel_ring", soc);
		goto fail1;
	}

	/* TCL command and status rings */
	if (dp_soc_tcl_cmd_cred_srng_alloc(soc)) {
		dp_init_err("%pK: dp_srng_alloc failed for tcl_cmd_ring", soc);
		goto fail1;
	}

	if (dp_soc_tcl_status_srng_alloc(soc)) {
		dp_init_err("%pK: dp_srng_alloc failed for tcl_status_ring", soc);
		goto fail1;
	}

	/* REO reinjection ring */
	entries = wlan_cfg_get_dp_soc_reo_reinject_ring_size(soc_cfg_ctx);
	if (dp_srng_alloc(soc, &soc->reo_reinject_ring, REO_REINJECT,
			  entries, 0)) {
		dp_init_err("%pK: dp_srng_alloc failed for reo_reinject_ring", soc);
		goto fail1;
	}

	/* Rx release ring */
	entries = wlan_cfg_get_dp_soc_rx_release_ring_size(soc_cfg_ctx);
	if (dp_srng_alloc(soc, &soc->rx_rel_ring, WBM2SW_RELEASE,
			  entries, 0)) {
		dp_init_err("%pK: dp_srng_alloc failed for rx_rel_ring", soc);
		goto fail1;
	}

	/* Rx exception ring */
	entries = wlan_cfg_get_dp_soc_reo_exception_ring_size(soc_cfg_ctx);
	if (dp_srng_alloc(soc, &soc->reo_exception_ring, REO_EXCEPTION,
			  entries, 0)) {
		dp_init_err("%pK: dp_srng_alloc failed - reo_exception", soc);
		goto fail1;
	}

	/* REO command and status rings */
	entries = wlan_cfg_get_dp_soc_reo_cmd_ring_size(soc_cfg_ctx);
	if (dp_srng_alloc(soc, &soc->reo_cmd_ring, REO_CMD, entries, 0)) {
		dp_init_err("%pK: dp_srng_alloc failed for reo_cmd_ring", soc);
		goto fail1;
	}

	entries = wlan_cfg_get_dp_soc_reo_status_ring_size(soc_cfg_ctx);
	if (dp_srng_alloc(soc, &soc->reo_status_ring, REO_STATUS,
			  entries, 0)) {
		dp_init_err("%pK: dp_srng_alloc failed for reo_status_ring", soc);
		goto fail1;
	}

	tx_comp_ring_size = wlan_cfg_tx_comp_ring_size(soc_cfg_ctx);
	tx_ring_size = wlan_cfg_tx_ring_size(soc_cfg_ctx);
	reo_dst_ring_size = wlan_cfg_get_reo_dst_ring_size(soc_cfg_ctx);

	/* Disable cached desc if NSS offload is enabled */
	if (wlan_cfg_get_dp_soc_nss_cfg(soc_cfg_ctx))
		cached = 0;

	for (i = 0; i < soc->num_tcl_data_rings; i++) {
		if (dp_alloc_tx_ring_pair_by_index(soc, i))
			goto fail1;
	}

	/* IPA rings for TCL_TX and TX_COMP will be allocated here */
	if (wlan_cfg_is_ipa_enabled(soc->wlan_cfg_ctx)) {
		if (dp_alloc_tx_ring_pair_by_index(soc, IPA_TCL_DATA_RING_IDX))
			goto fail1;

		if (dp_ipa_alloc_alt_tx_ring(soc))
			goto fail1;
	}

	for (i = 0; i < soc->num_reo_dest_rings; i++) {
		/* Setup REO destination ring */
		if (dp_srng_alloc(soc, &soc->reo_dest_ring[i], REO_DST,
				  reo_dst_ring_size, cached)) {
			dp_init_err("%pK: dp_srng_alloc failed for reo_dest_ring", soc);
			goto fail1;
		}
	}

	if (soc->arch_ops.txrx_soc_srng_alloc) {
		if (soc->arch_ops.txrx_soc_srng_alloc(soc)) {
			dp_init_err("%pK: dp_srng_alloc failed for arch rings",
				    soc);
			goto fail1;
		}
	}

	return QDF_STATUS_SUCCESS;

fail1:
	dp_soc_srng_free(soc);
	return QDF_STATUS_E_NOMEM;
}

static void dp_soc_cfg_dump(struct dp_soc *soc, uint32_t target_type)
{
	dp_init_info("DP soc Dump for Target = %d", target_type);
	dp_init_info("ast_override_support = %d, da_war_enabled = %d,",
		     soc->ast_override_support, soc->da_war_enabled);

	wlan_cfg_dp_soc_ctx_dump(soc->wlan_cfg_ctx);
}

/**
 * dp_soc_cfg_init() - initialize target specific configuration
 *		       during dp_soc_init
 * @soc: dp soc handle
 */
static void dp_soc_cfg_init(struct dp_soc *soc)
{
	uint32_t target_type;

	target_type = hal_get_target_type(soc->hal_soc);
	switch (target_type) {
	case TARGET_TYPE_QCA6290:
		wlan_cfg_set_reo_dst_ring_size(soc->wlan_cfg_ctx,
					       REO_DST_RING_SIZE_QCA6290);
		soc->ast_override_support = 1;
		soc->da_war_enabled = false;
		break;
	case TARGET_TYPE_QCA6390:
	case TARGET_TYPE_QCA6490:
	case TARGET_TYPE_QCA6750:
		wlan_cfg_set_reo_dst_ring_size(soc->wlan_cfg_ctx,
					       REO_DST_RING_SIZE_QCA6290);
		wlan_cfg_set_raw_mode_war(soc->wlan_cfg_ctx, true);
		soc->ast_override_support = 1;
		if (soc->cdp_soc.ol_ops->get_con_mode &&
		    soc->cdp_soc.ol_ops->get_con_mode() ==
		    QDF_GLOBAL_MONITOR_MODE) {
			int int_ctx;

			for (int_ctx = 0; int_ctx < WLAN_CFG_INT_NUM_CONTEXTS; int_ctx++) {
				soc->wlan_cfg_ctx->int_rx_ring_mask[int_ctx] = 0;
				soc->wlan_cfg_ctx->int_rxdma2host_ring_mask[int_ctx] = 0;
			}
		}
		soc->wlan_cfg_ctx->rxdma1_enable = 0;
		break;
	case TARGET_TYPE_KIWI:
		wlan_cfg_set_reo_dst_ring_size(soc->wlan_cfg_ctx,
					       REO_DST_RING_SIZE_QCA6290);
		soc->ast_override_support = 1;

		if (soc->cdp_soc.ol_ops->get_con_mode &&
		    soc->cdp_soc.ol_ops->get_con_mode() ==
		    QDF_GLOBAL_MONITOR_MODE) {
			int int_ctx;

			for (int_ctx = 0; int_ctx < WLAN_CFG_INT_NUM_CONTEXTS;
			     int_ctx++) {
				soc->wlan_cfg_ctx->int_rx_ring_mask[int_ctx] = 0;
				if (dp_is_monitor_mode_using_poll(soc))
					soc->wlan_cfg_ctx->int_rxdma2host_ring_mask[int_ctx] = 0;
			}
		}

		soc->wlan_cfg_ctx->rxdma1_enable = 0;
		soc->wlan_cfg_ctx->num_rxdma_dst_rings_per_pdev = 1;
		break;
	case TARGET_TYPE_QCA8074:
		wlan_cfg_set_raw_mode_war(soc->wlan_cfg_ctx, true);
		soc->da_war_enabled = true;
		soc->is_rx_fse_full_cache_invalidate_war_enabled = true;
		break;
	case TARGET_TYPE_QCA8074V2:
	case TARGET_TYPE_QCA6018:
	case TARGET_TYPE_QCA9574:
		wlan_cfg_set_raw_mode_war(soc->wlan_cfg_ctx, false);
		soc->ast_override_support = 1;
		soc->per_tid_basize_max_tid = 8;
		soc->num_hw_dscp_tid_map = HAL_MAX_HW_DSCP_TID_V2_MAPS;
		soc->da_war_enabled = false;
		soc->is_rx_fse_full_cache_invalidate_war_enabled = true;
		break;
	case TARGET_TYPE_QCN9000:
		soc->ast_override_support = 1;
		soc->da_war_enabled = false;
		wlan_cfg_set_raw_mode_war(soc->wlan_cfg_ctx, false);
		soc->per_tid_basize_max_tid = 8;
		soc->num_hw_dscp_tid_map = HAL_MAX_HW_DSCP_TID_V2_MAPS;
		soc->lmac_polled_mode = 0;
		soc->wbm_release_desc_rx_sg_support = 1;
		break;
	case TARGET_TYPE_QCA5018:
	case TARGET_TYPE_QCN6122:
		soc->ast_override_support = 1;
		soc->da_war_enabled = false;
		wlan_cfg_set_raw_mode_war(soc->wlan_cfg_ctx, false);
		soc->per_tid_basize_max_tid = 8;
		soc->num_hw_dscp_tid_map = HAL_MAX_HW_DSCP_TID_MAPS_11AX;
		soc->disable_mac1_intr = 1;
		soc->disable_mac2_intr = 1;
		soc->wbm_release_desc_rx_sg_support = 1;
		break;
	case TARGET_TYPE_QCN9224:
		soc->ast_override_support = 1;
		soc->da_war_enabled = false;
		wlan_cfg_set_raw_mode_war(soc->wlan_cfg_ctx, false);
		soc->per_tid_basize_max_tid = 8;
		soc->wbm_release_desc_rx_sg_support = 1;
		soc->rxdma2sw_rings_not_supported = 1;
		soc->ast_offload_support = AST_OFFLOAD_ENABLE_STATUS;
		soc->mec_fw_offload = FW_MEC_FW_OFFLOAD_ENABLED;
		break;
	default:
		qdf_print("%s: Unknown tgt type %d\n", __func__, target_type);
		qdf_assert_always(0);
		break;
	}
	dp_soc_cfg_dump(soc, target_type);
}

/**
 * dp_soc_cfg_attach() - set target specific configuration in
 *			 dp soc cfg.
 * @soc: dp soc handle
 */
static void dp_soc_cfg_attach(struct dp_soc *soc)
{
	int target_type;
	int nss_cfg = 0;

	target_type = hal_get_target_type(soc->hal_soc);
	switch (target_type) {
	case TARGET_TYPE_QCA6290:
		wlan_cfg_set_reo_dst_ring_size(soc->wlan_cfg_ctx,
					       REO_DST_RING_SIZE_QCA6290);
		break;
	case TARGET_TYPE_QCA6390:
	case TARGET_TYPE_QCA6490:
	case TARGET_TYPE_QCA6750:
		wlan_cfg_set_reo_dst_ring_size(soc->wlan_cfg_ctx,
					       REO_DST_RING_SIZE_QCA6290);
		soc->wlan_cfg_ctx->rxdma1_enable = 0;
		break;
	case TARGET_TYPE_KIWI:
		wlan_cfg_set_reo_dst_ring_size(soc->wlan_cfg_ctx,
					       REO_DST_RING_SIZE_QCA6290);
		soc->wlan_cfg_ctx->rxdma1_enable = 0;
		break;
	case TARGET_TYPE_QCA8074:
		wlan_cfg_set_tso_desc_attach_defer(soc->wlan_cfg_ctx, 1);
		break;
	case TARGET_TYPE_QCA8074V2:
	case TARGET_TYPE_QCA6018:
	case TARGET_TYPE_QCA9574:
	case TARGET_TYPE_QCN6122:
	case TARGET_TYPE_QCA5018:
		wlan_cfg_set_tso_desc_attach_defer(soc->wlan_cfg_ctx, 1);
		wlan_cfg_set_rxdma1_enable(soc->wlan_cfg_ctx);
		break;
	case TARGET_TYPE_QCN9000:
		wlan_cfg_set_tso_desc_attach_defer(soc->wlan_cfg_ctx, 1);
		wlan_cfg_set_rxdma1_enable(soc->wlan_cfg_ctx);
		break;
	case TARGET_TYPE_QCN9224:
		wlan_cfg_set_tso_desc_attach_defer(soc->wlan_cfg_ctx, 1);
		wlan_cfg_set_rxdma1_enable(soc->wlan_cfg_ctx);
		break;
	default:
		qdf_print("%s: Unknown tgt type %d\n", __func__, target_type);
		qdf_assert_always(0);
		break;
	}

	if (soc->cdp_soc.ol_ops->get_soc_nss_cfg)
		nss_cfg = soc->cdp_soc.ol_ops->get_soc_nss_cfg(soc->ctrl_psoc);

	wlan_cfg_set_dp_soc_nss_cfg(soc->wlan_cfg_ctx, nss_cfg);

	if (wlan_cfg_get_dp_soc_nss_cfg(soc->wlan_cfg_ctx)) {
		wlan_cfg_set_num_tx_desc_pool(soc->wlan_cfg_ctx, 0);
		wlan_cfg_set_num_tx_ext_desc_pool(soc->wlan_cfg_ctx, 0);
		wlan_cfg_set_num_tx_desc(soc->wlan_cfg_ctx, 0);
		wlan_cfg_set_num_tx_ext_desc(soc->wlan_cfg_ctx, 0);
		soc->init_tcl_cmd_cred_ring = false;
		soc->num_tcl_data_rings =
			wlan_cfg_num_nss_tcl_data_rings(soc->wlan_cfg_ctx);
		soc->num_reo_dest_rings =
			wlan_cfg_num_nss_reo_dest_rings(soc->wlan_cfg_ctx);

	} else {
		soc->init_tcl_cmd_cred_ring = true;
		soc->num_tcl_data_rings =
			wlan_cfg_num_tcl_data_rings(soc->wlan_cfg_ctx);
		soc->num_reo_dest_rings =
			wlan_cfg_num_reo_dest_rings(soc->wlan_cfg_ctx);
	}

	soc->arch_ops.soc_cfg_attach(soc);
}

static inline  void dp_pdev_set_default_reo(struct dp_pdev *pdev)
{
	struct dp_soc *soc = pdev->soc;

	switch (pdev->pdev_id) {
	case 0:
		pdev->reo_dest =
			wlan_cfg_radio0_default_reo_get(soc->wlan_cfg_ctx);
		break;

	case 1:
		pdev->reo_dest =
			wlan_cfg_radio1_default_reo_get(soc->wlan_cfg_ctx);
		break;

	case 2:
		pdev->reo_dest =
			wlan_cfg_radio2_default_reo_get(soc->wlan_cfg_ctx);
		break;

	default:
		dp_init_err("%pK: Invalid pdev_id %d for reo selection",
			    soc, pdev->pdev_id);
		break;
	}
}

static QDF_STATUS dp_pdev_init(struct cdp_soc_t *txrx_soc,
				      HTC_HANDLE htc_handle,
				      qdf_device_t qdf_osdev,
				      uint8_t pdev_id)
{
	struct wlan_cfg_dp_soc_ctxt *soc_cfg_ctx;
	int nss_cfg;
	void *sojourn_buf;
	QDF_STATUS ret;

	struct dp_soc *soc = (struct dp_soc *)txrx_soc;
	struct dp_pdev *pdev = soc->pdev_list[pdev_id];

	soc_cfg_ctx = soc->wlan_cfg_ctx;
	pdev->soc = soc;
	pdev->pdev_id = pdev_id;

	/*
	 * Variable to prevent double pdev deinitialization during
	 * radio detach execution .i.e. in the absence of any vdev.
	 */
	pdev->pdev_deinit = 0;

	if (dp_wdi_event_attach(pdev)) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
			  "dp_wdi_evet_attach failed");
		goto fail0;
	}

	if (dp_pdev_srng_init(pdev)) {
		dp_init_err("%pK: Failed to initialize pdev srng rings", soc);
		goto fail1;
	}

	/* Initialize descriptors in TCL Rings used by IPA */
	if (wlan_cfg_is_ipa_enabled(soc->wlan_cfg_ctx)) {
		hal_tx_init_data_ring(soc->hal_soc,
				      soc->tcl_data_ring[IPA_TCL_DATA_RING_IDX].hal_srng);
		dp_ipa_hal_tx_init_alt_data_ring(soc);
	}

	/*
	 * Initialize command/credit ring descriptor
	 * Command/CREDIT ring also used for sending DATA cmds
	 */
	dp_tx_init_cmd_credit_ring(soc);

	dp_tx_pdev_init(pdev);
	/*
	 * Variable to prevent double pdev deinitialization during
	 * radio detach execution .i.e. in the absence of any vdev.
	 */
	pdev->invalid_peer = qdf_mem_malloc(sizeof(struct dp_peer));

	if (!pdev->invalid_peer) {
		dp_init_err("%pK: Invalid peer memory allocation failed", soc);
		goto fail2;
	}

	/*
	 * set nss pdev config based on soc config
	 */
	nss_cfg = wlan_cfg_get_dp_soc_nss_cfg(soc_cfg_ctx);
	wlan_cfg_set_dp_pdev_nss_enabled(pdev->wlan_cfg_ctx,
					 (nss_cfg & (1 << pdev_id)));
	pdev->target_pdev_id =
		dp_calculate_target_pdev_id_from_host_pdev_id(soc, pdev_id);

	if (soc->preferred_hw_mode == WMI_HOST_HW_MODE_2G_PHYB &&
	    pdev->lmac_id == PHYB_2G_LMAC_ID) {
		pdev->target_pdev_id = PHYB_2G_TARGET_PDEV_ID;
	}

	/* Reset the cpu ring map if radio is NSS offloaded */
	if (wlan_cfg_get_dp_soc_nss_cfg(soc->wlan_cfg_ctx)) {
		dp_soc_reset_cpu_ring_map(soc);
		dp_soc_reset_intr_mask(soc);
	}

	TAILQ_INIT(&pdev->vdev_list);
	qdf_spinlock_create(&pdev->vdev_list_lock);
	pdev->vdev_count = 0;

	qdf_spinlock_create(&pdev->tx_mutex);
	pdev->ch_band_lmac_id_mapping[REG_BAND_2G] = DP_MON_INVALID_LMAC_ID;
	pdev->ch_band_lmac_id_mapping[REG_BAND_5G] = DP_MON_INVALID_LMAC_ID;
	pdev->ch_band_lmac_id_mapping[REG_BAND_6G] = DP_MON_INVALID_LMAC_ID;

	DP_STATS_INIT(pdev);

	dp_local_peer_id_pool_init(pdev);

	dp_dscp_tid_map_setup(pdev);
	dp_pcp_tid_map_setup(pdev);

	/* set the reo destination during initialization */
	dp_pdev_set_default_reo(pdev);

	qdf_mem_zero(&pdev->sojourn_stats, sizeof(struct cdp_tx_sojourn_stats));

	pdev->sojourn_buf = qdf_nbuf_alloc(pdev->soc->osdev,
			      sizeof(struct cdp_tx_sojourn_stats), 0, 4,
			      TRUE);

	if (!pdev->sojourn_buf) {
		dp_init_err("%pK: Failed to allocate sojourn buf", soc);
		goto fail3;
	}
	sojourn_buf = qdf_nbuf_data(pdev->sojourn_buf);
	qdf_mem_zero(sojourn_buf, sizeof(struct cdp_tx_sojourn_stats));

	qdf_event_create(&pdev->fw_peer_stats_event);

	pdev->num_tx_allowed = wlan_cfg_get_num_tx_desc(soc->wlan_cfg_ctx);

	if (dp_rxdma_ring_setup(soc, pdev)) {
		dp_init_err("%pK: RXDMA ring config failed", soc);
		goto fail4;
	}

	if (dp_init_ipa_rx_refill_buf_ring(soc, pdev))
		goto fail4;

	if (dp_ipa_ring_resource_setup(soc, pdev))
		goto fail5;

	if (dp_ipa_uc_attach(soc, pdev) != QDF_STATUS_SUCCESS) {
		dp_init_err("%pK: dp_ipa_uc_attach failed", soc);
		goto fail5;
	}

	ret = dp_rx_fst_attach(soc, pdev);
	if ((ret != QDF_STATUS_SUCCESS) &&
	    (ret != QDF_STATUS_E_NOSUPPORT)) {
		dp_init_err("%pK: RX Flow Search Table attach failed: pdev %d err %d",
			    soc, pdev_id, ret);
		goto fail6;
	}

	if (dp_pdev_bkp_stats_attach(pdev) != QDF_STATUS_SUCCESS) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
			  FL("dp_pdev_bkp_stats_attach failed"));
		goto fail7;
	}

	if (dp_monitor_pdev_init(pdev)) {
		dp_init_err("%pK: dp_monitor_pdev_init failed\n", soc);
		goto fail8;
	}

	/* initialize sw rx descriptors */
	dp_rx_pdev_desc_pool_init(pdev);
	/* allocate buffers and replenish the RxDMA ring */
	dp_rx_pdev_buffers_alloc(pdev);

	dp_init_tso_stats(pdev);

	dp_info("Mem stats: DMA = %u HEAP = %u SKB = %u",
		qdf_dma_mem_stats_read(),
		qdf_heap_mem_stats_read(),
		qdf_skb_total_mem_stats_read());

	return QDF_STATUS_SUCCESS;
fail8:
	dp_pdev_bkp_stats_detach(pdev);
fail7:
	dp_rx_fst_detach(soc, pdev);
fail6:
	dp_ipa_uc_detach(soc, pdev);
fail5:
	dp_deinit_ipa_rx_refill_buf_ring(soc, pdev);
fail4:
	dp_rxdma_ring_cleanup(soc, pdev);
	qdf_nbuf_free(pdev->sojourn_buf);
fail3:
	qdf_spinlock_destroy(&pdev->tx_mutex);
	qdf_spinlock_destroy(&pdev->vdev_list_lock);
	qdf_mem_free(pdev->invalid_peer);
fail2:
	dp_pdev_srng_deinit(pdev);
fail1:
	dp_wdi_event_detach(pdev);
fail0:
	return QDF_STATUS_E_FAILURE;
}

/*
 * dp_pdev_init_wifi3() - Init txrx pdev
 * @htc_handle: HTC handle for host-target interface
 * @qdf_osdev: QDF OS device
 * @force: Force deinit
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS dp_pdev_init_wifi3(struct cdp_soc_t *txrx_soc,
				     HTC_HANDLE htc_handle,
				     qdf_device_t qdf_osdev,
				     uint8_t pdev_id)
{
	return dp_pdev_init(txrx_soc, htc_handle, qdf_osdev, pdev_id);
}

