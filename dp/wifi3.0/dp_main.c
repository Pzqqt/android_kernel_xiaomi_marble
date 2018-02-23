/*
 * Copyright (c) 2016-2018 The Linux Foundation. All rights reserved.
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
#include <hal_api.h>
#include <hif.h>
#include <htt.h>
#include <wdi_event.h>
#include <queue.h>
#include "dp_htt.h"
#include "dp_types.h"
#include "dp_internal.h"
#include "dp_tx.h"
#include "dp_tx_desc.h"
#include "dp_rx.h"
#include <cdp_txrx_handle.h>
#include <wlan_cfg.h>
#include "cdp_txrx_cmn_struct.h"
#include "cdp_txrx_stats_struct.h"
#include <qdf_util.h>
#include "dp_peer.h"
#include "dp_rx_mon.h"
#include "htt_stats.h"
#include "qdf_mem.h"   /* qdf_mem_malloc,free */
#ifdef QCA_LL_TX_FLOW_CONTROL_V2
#include "cdp_txrx_flow_ctrl_v2.h"
#else
static inline void
cdp_dump_flow_pool_info(struct cdp_soc_t *soc)
{
	return;
}
#endif
#include "dp_ipa.h"

#ifdef CONFIG_MCL
static void dp_service_mon_rings(void *arg);
#ifndef REMOVE_PKT_LOG
#include <pktlog_ac_api.h>
#include <pktlog_ac.h>
static void dp_pkt_log_con_service(struct cdp_pdev *ppdev, void *scn);
#endif
#endif
static void dp_pktlogmod_exit(struct dp_pdev *handle);
static void *dp_peer_create_wifi3(struct cdp_vdev *vdev_handle,
					uint8_t *peer_mac_addr);
static void dp_peer_delete_wifi3(void *peer_handle, uint32_t bitmap);

#define DP_INTR_POLL_TIMER_MS	10
#define DP_WDS_AGING_TIMER_DEFAULT_MS	120000
#define DP_MCS_LENGTH (6*MAX_MCS)
#define DP_NSS_LENGTH (6*SS_COUNT)
#define DP_RXDMA_ERR_LENGTH (6*HAL_RXDMA_ERR_MAX)
#define DP_REO_ERR_LENGTH (6*HAL_REO_ERR_MAX)
#define DP_MAX_MCS_STRING_LEN 30
#define DP_CURR_FW_STATS_AVAIL 19
#define DP_HTT_DBG_EXT_STATS_MAX 256
#define DP_MAX_SLEEP_TIME 100

#ifdef IPA_OFFLOAD
/* Exclude IPA rings from the interrupt context */
#define TX_RING_MASK_VAL	0xb
#define RX_RING_MASK_VAL	0x7
#else
#define TX_RING_MASK_VAL	0xF
#define RX_RING_MASK_VAL	0xF
#endif

bool rx_hash = 1;
qdf_declare_param(rx_hash, bool);

#define STR_MAXLEN	64

#define DP_PPDU_STATS_CFG_ALL 0xFFFF

/* PPDU stats mask sent to FW to enable enhanced stats */
#define DP_PPDU_STATS_CFG_ENH_STATS 0xE67
/* PPDU stats mask sent to FW to support debug sniffer feature */
#define DP_PPDU_STATS_CFG_SNIFFER 0x2FFF
/**
 * default_dscp_tid_map - Default DSCP-TID mapping
 *
 * DSCP        TID     AC
 * 000000      0       WME_AC_BE
 * 001000      1       WME_AC_BK
 * 010000      1       WME_AC_BK
 * 011000      0       WME_AC_BE
 * 100000      5       WME_AC_VI
 * 101000      5       WME_AC_VI
 * 110000      6       WME_AC_VO
 * 111000      6       WME_AC_VO
 */
static uint8_t default_dscp_tid_map[DSCP_TID_MAP_MAX] = {
	0, 0, 0, 0, 0, 0, 0, 0,
	1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1,
	0, 0, 0, 0, 0, 0, 0, 0,
	5, 5, 5, 5, 5, 5, 5, 5,
	5, 5, 5, 5, 5, 5, 5, 5,
	6, 6, 6, 6, 6, 6, 6, 6,
	6, 6, 6, 6, 6, 6, 6, 6,
};

/*
 * struct dp_rate_debug
 *
 * @mcs_type: print string for a given mcs
 * @valid: valid mcs rate?
 */
struct dp_rate_debug {
	char mcs_type[DP_MAX_MCS_STRING_LEN];
	uint8_t valid;
};

#define MCS_VALID 1
#define MCS_INVALID 0

static const struct dp_rate_debug dp_rate_string[DOT11_MAX][MAX_MCS] = {

	{
		{"OFDM 48 Mbps", MCS_VALID},
		{"OFDM 24 Mbps", MCS_VALID},
		{"OFDM 12 Mbps", MCS_VALID},
		{"OFDM 6 Mbps ", MCS_VALID},
		{"OFDM 54 Mbps", MCS_VALID},
		{"OFDM 36 Mbps", MCS_VALID},
		{"OFDM 18 Mbps", MCS_VALID},
		{"OFDM 9 Mbps ", MCS_VALID},
		{"INVALID ", MCS_INVALID},
		{"INVALID ", MCS_INVALID},
		{"INVALID ", MCS_INVALID},
		{"INVALID ", MCS_INVALID},
		{"INVALID ", MCS_VALID},
	},
	{
		{"CCK 11 Mbps Long  ", MCS_VALID},
		{"CCK 5.5 Mbps Long ", MCS_VALID},
		{"CCK 2 Mbps Long   ", MCS_VALID},
		{"CCK 1 Mbps Long   ", MCS_VALID},
		{"CCK 11 Mbps Short ", MCS_VALID},
		{"CCK 5.5 Mbps Short", MCS_VALID},
		{"CCK 2 Mbps Short  ", MCS_VALID},
		{"INVALID ", MCS_INVALID},
		{"INVALID ", MCS_INVALID},
		{"INVALID ", MCS_INVALID},
		{"INVALID ", MCS_INVALID},
		{"INVALID ", MCS_INVALID},
		{"INVALID ", MCS_VALID},
	},
	{
		{"HT MCS 0 (BPSK 1/2)  ", MCS_VALID},
		{"HT MCS 1 (QPSK 1/2)  ", MCS_VALID},
		{"HT MCS 2 (QPSK 3/4)  ", MCS_VALID},
		{"HT MCS 3 (16-QAM 1/2)", MCS_VALID},
		{"HT MCS 4 (16-QAM 3/4)", MCS_VALID},
		{"HT MCS 5 (64-QAM 2/3)", MCS_VALID},
		{"HT MCS 6 (64-QAM 3/4)", MCS_VALID},
		{"HT MCS 7 (64-QAM 5/6)", MCS_VALID},
		{"INVALID ", MCS_INVALID},
		{"INVALID ", MCS_INVALID},
		{"INVALID ", MCS_INVALID},
		{"INVALID ", MCS_INVALID},
		{"INVALID ", MCS_VALID},
	},
	{
		{"VHT MCS 0 (BPSK 1/2)     ", MCS_VALID},
		{"VHT MCS 1 (QPSK 1/2)     ", MCS_VALID},
		{"VHT MCS 2 (QPSK 3/4)     ", MCS_VALID},
		{"VHT MCS 3 (16-QAM 1/2)   ", MCS_VALID},
		{"VHT MCS 4 (16-QAM 3/4)   ", MCS_VALID},
		{"VHT MCS 5 (64-QAM 2/3)   ", MCS_VALID},
		{"VHT MCS 6 (64-QAM 3/4)   ", MCS_VALID},
		{"VHT MCS 7 (64-QAM 5/6)   ", MCS_VALID},
		{"VHT MCS 8 (256-QAM 3/4)  ", MCS_VALID},
		{"VHT MCS 9 (256-QAM 5/6)  ", MCS_VALID},
		{"VHT MCS 10 (1024-QAM 3/4)", MCS_VALID},
		{"VHT MCS 11 (1024-QAM 5/6)", MCS_VALID},
		{"INVALID ", MCS_VALID},
	},
	{
		{"HE MCS 0 (BPSK 1/2)     ", MCS_VALID},
		{"HE MCS 1 (QPSK 1/2)     ", MCS_VALID},
		{"HE MCS 2 (QPSK 3/4)     ", MCS_VALID},
		{"HE MCS 3 (16-QAM 1/2)   ", MCS_VALID},
		{"HE MCS 4 (16-QAM 3/4)   ", MCS_VALID},
		{"HE MCS 5 (64-QAM 2/3)   ", MCS_VALID},
		{"HE MCS 6 (64-QAM 3/4)   ", MCS_VALID},
		{"HE MCS 7 (64-QAM 5/6)   ", MCS_VALID},
		{"HE MCS 8 (256-QAM 3/4)  ", MCS_VALID},
		{"HE MCS 9 (256-QAM 5/6)  ", MCS_VALID},
		{"HE MCS 10 (1024-QAM 3/4)", MCS_VALID},
		{"HE MCS 11 (1024-QAM 5/6)", MCS_VALID},
		{"INVALID ", MCS_VALID},
	}
};

/**
 * @brief Cpu ring map types
 */
enum dp_cpu_ring_map_types {
	DP_DEFAULT_MAP,
	DP_NSS_FIRST_RADIO_OFFLOADED_MAP,
	DP_NSS_SECOND_RADIO_OFFLOADED_MAP,
	DP_NSS_ALL_RADIO_OFFLOADED_MAP,
	DP_CPU_RING_MAP_MAX
};

/**
 * @brief Cpu to tx ring map
 */
static uint8_t dp_cpu_ring_map[DP_CPU_RING_MAP_MAX][WLAN_CFG_INT_NUM_CONTEXTS] = {
	{0x0, 0x1, 0x2, 0x0},
	{0x1, 0x2, 0x1, 0x2},
	{0x0, 0x2, 0x0, 0x2},
	{0x2, 0x2, 0x2, 0x2}
};

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
};

static int dp_peer_add_ast_wifi3(struct cdp_soc_t *soc_hdl,
					struct cdp_peer *peer_hdl,
					uint8_t *mac_addr,
					enum cdp_txrx_ast_entry_type type,
					uint32_t flags)
{

	return dp_peer_add_ast((struct dp_soc *)soc_hdl,
				(struct dp_peer *)peer_hdl,
				mac_addr,
				type,
				flags);
}

static void dp_peer_del_ast_wifi3(struct cdp_soc_t *soc_hdl,
					 void *ast_entry_hdl)
{
	struct dp_soc *soc = (struct dp_soc *)soc_hdl;
	qdf_spin_lock_bh(&soc->ast_lock);
	dp_peer_del_ast((struct dp_soc *)soc_hdl,
			(struct dp_ast_entry *)ast_entry_hdl);
	qdf_spin_unlock_bh(&soc->ast_lock);
}

static int dp_peer_update_ast_wifi3(struct cdp_soc_t *soc_hdl,
						struct cdp_peer *peer_hdl,
						void *ast_entry_hdl,
						uint32_t flags)
{
	int status;
	struct dp_soc *soc = (struct dp_soc *)soc_hdl;
	qdf_spin_lock_bh(&soc->ast_lock);
	status = dp_peer_update_ast(soc,
					(struct dp_peer *)peer_hdl,
					(struct dp_ast_entry *)ast_entry_hdl,
					flags);
	qdf_spin_unlock_bh(&soc->ast_lock);
	return status;
}

static void *dp_peer_ast_hash_find_wifi3(struct cdp_soc_t *soc_hdl,
						uint8_t *ast_mac_addr)
{
	struct dp_ast_entry *ast_entry;
	struct dp_soc *soc = (struct dp_soc *)soc_hdl;
	qdf_spin_lock_bh(&soc->ast_lock);
	ast_entry = dp_peer_ast_hash_find(soc, ast_mac_addr);
	qdf_spin_unlock_bh(&soc->ast_lock);
	return (void *)ast_entry;
}

static uint8_t dp_peer_ast_get_pdev_id_wifi3(struct cdp_soc_t *soc_hdl,
							void *ast_entry_hdl)
{
	return dp_peer_ast_get_pdev_id((struct dp_soc *)soc_hdl,
					(struct dp_ast_entry *)ast_entry_hdl);
}

static uint8_t dp_peer_ast_get_next_hop_wifi3(struct cdp_soc_t *soc_hdl,
							void *ast_entry_hdl)
{
	return dp_peer_ast_get_next_hop((struct dp_soc *)soc_hdl,
					(struct dp_ast_entry *)ast_entry_hdl);
}

static void dp_peer_ast_set_type_wifi3(
					struct cdp_soc_t *soc_hdl,
					void *ast_entry_hdl,
					enum cdp_txrx_ast_entry_type type)
{
	dp_peer_ast_set_type((struct dp_soc *)soc_hdl,
				(struct dp_ast_entry *)ast_entry_hdl,
				type);
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
static int dp_srng_find_ring_in_mask(int ring_num, int *grp_mask)
{
	int ext_group_num;
	int mask = 1 << ring_num;

	for (ext_group_num = 0; ext_group_num < WLAN_CFG_INT_NUM_CONTEXTS;
	     ext_group_num++) {
		if (mask & grp_mask[ext_group_num])
			return ext_group_num;
	}

	return -QDF_STATUS_E_NOENT;
}

static int dp_srng_calculate_msi_group(struct dp_soc *soc,
				       enum hal_ring_type ring_type,
				       int ring_num)
{
	int *grp_mask;

	switch (ring_type) {
	case WBM2SW_RELEASE:
		/* dp_tx_comp_handler - soc->tx_comp_ring */
		if (ring_num < 3)
			grp_mask = &soc->wlan_cfg_ctx->int_tx_ring_mask[0];

		/* dp_rx_wbm_err_process - soc->rx_rel_ring */
		else if (ring_num == 3) {
			/* sw treats this as a separate ring type */
			grp_mask = &soc->wlan_cfg_ctx->
				int_rx_wbm_rel_ring_mask[0];
			ring_num = 0;
		} else {
			qdf_assert(0);
			return -QDF_STATUS_E_NOENT;
		}
	break;

	case REO_EXCEPTION:
		/* dp_rx_err_process - &soc->reo_exception_ring */
		grp_mask = &soc->wlan_cfg_ctx->int_rx_err_ring_mask[0];
	break;

	case REO_DST:
		/* dp_rx_process - soc->reo_dest_ring */
		grp_mask = &soc->wlan_cfg_ctx->int_rx_ring_mask[0];
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
	case RXDMA_DST:
		/* dp_rxdma_err_process */
		grp_mask = &soc->wlan_cfg_ctx->int_rxdma2host_ring_mask[0];
	break;

	case RXDMA_BUF:
		grp_mask = &soc->wlan_cfg_ctx->int_host2rxdma_ring_mask[0];
	break;

	case RXDMA_MONITOR_BUF:
		/* TODO: support low_thresh interrupt */
		return -QDF_STATUS_E_NOENT;
	break;

	case TCL_DATA:
	case TCL_CMD:
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

	return dp_srng_find_ring_in_mask(ring_num, grp_mask);
}

static void dp_srng_msi_setup(struct dp_soc *soc, struct hal_srng_params
			      *ring_params, int ring_type, int ring_num)
{
	int msi_group_number;
	int msi_data_count;
	int ret;
	uint32_t msi_data_start, msi_irq_start, addr_low, addr_high;

	ret = pld_get_user_msi_assignment(soc->osdev->dev, "DP",
					    &msi_data_count, &msi_data_start,
					    &msi_irq_start);

	if (ret)
		return;

	msi_group_number = dp_srng_calculate_msi_group(soc, ring_type,
						       ring_num);
	if (msi_group_number < 0) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_INFO_LOW,
			FL("ring not part of an ext_group; ring_type: %d,ring_num %d"),
			ring_type, ring_num);
		ring_params->msi_addr = 0;
		ring_params->msi_data = 0;
		return;
	}

	if (msi_group_number > msi_data_count) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_WARN,
			FL("2 msi_groups will share an msi; msi_group_num %d"),
			msi_group_number);

		QDF_ASSERT(0);
	}

	pld_get_msi_address(soc->osdev->dev, &addr_low, &addr_high);

	ring_params->msi_addr = addr_low;
	ring_params->msi_addr |= (qdf_dma_addr_t)(((uint64_t)addr_high) << 32);
	ring_params->msi_data = (msi_group_number % msi_data_count)
		+ msi_data_start;
	ring_params->flags |= HAL_SRNG_MSI_INTR;
}

/**
 * dp_print_ast_stats() - Dump AST table contents
 * @soc: Datapath soc handle
 *
 * return void
 */
#ifdef FEATURE_WDS
static void dp_print_ast_stats(struct dp_soc *soc)
{
	uint8_t i;
	uint8_t num_entries = 0;
	struct dp_vdev *vdev;
	struct dp_pdev *pdev;
	struct dp_peer *peer;
	struct dp_ast_entry *ase, *tmp_ase;

	DP_PRINT_STATS("AST Stats:");
	DP_PRINT_STATS("	Entries Added   = %d", soc->stats.ast.added);
	DP_PRINT_STATS("	Entries Deleted = %d", soc->stats.ast.deleted);
	DP_PRINT_STATS("	Entries Agedout = %d", soc->stats.ast.aged_out);
	DP_PRINT_STATS("AST Table:");
	for (i = 0; i < MAX_PDEV_CNT && soc->pdev_list[i]; i++) {
		pdev = soc->pdev_list[i];
		DP_PDEV_ITERATE_VDEV_LIST(pdev, vdev) {
			DP_VDEV_ITERATE_PEER_LIST(vdev, peer) {
				DP_PEER_ITERATE_ASE_LIST(peer, ase, tmp_ase) {
					DP_PRINT_STATS("%6d mac_addr = %pM"
							" peer_mac_addr = %pM"
							" type = %d"
							" next_hop = %d"
							" is_active = %d"
							" is_bss = %d"
							" ast_idx = %d"
							" pdev_id = %d"
							" vdev_id = %d",
							++num_entries,
							ase->mac_addr.raw,
							ase->peer->mac_addr.raw,
							ase->type,
							ase->next_hop,
							ase->is_active,
							ase->is_bss,
							ase->ast_idx,
							ase->pdev_id,
							ase->vdev_id);
				}
			}
		}
	}
}
#else
static void dp_print_ast_stats(struct dp_soc *soc)
{
	DP_PRINT_STATS("AST Stats not available.Enable FEATURE_WDS");
	return;
}
#endif

/*
 * dp_setup_srng - Internal function to setup SRNG rings used by data path
 */
static int dp_srng_setup(struct dp_soc *soc, struct dp_srng *srng,
	int ring_type, int ring_num, int mac_id, uint32_t num_entries)
{
	void *hal_soc = soc->hal_soc;
	uint32_t entry_size = hal_srng_get_entrysize(hal_soc, ring_type);
	/* TODO: See if we should get align size from hal */
	uint32_t ring_base_align = 8;
	struct hal_srng_params ring_params;
	uint32_t max_entries = hal_srng_max_entries(hal_soc, ring_type);

	/* TODO: Currently hal layer takes care of endianness related settings.
	 * See if these settings need to passed from DP layer
	 */
	ring_params.flags = 0;
	QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_INFO_LOW,
		  FL("Ring type: %d, num:%d"), ring_type, ring_num);

	num_entries = (num_entries > max_entries) ? max_entries : num_entries;
	srng->hal_srng = NULL;
	srng->alloc_size = (num_entries * entry_size) + ring_base_align - 1;
	srng->num_entries = num_entries;
	srng->base_vaddr_unaligned = qdf_mem_alloc_consistent(
		soc->osdev, soc->osdev->dev, srng->alloc_size,
		&(srng->base_paddr_unaligned));

	if (!srng->base_vaddr_unaligned) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
			FL("alloc failed - ring_type: %d, ring_num %d"),
			ring_type, ring_num);
		return QDF_STATUS_E_NOMEM;
	}

	ring_params.ring_base_vaddr = srng->base_vaddr_unaligned +
		((unsigned long)srng->base_vaddr_unaligned % ring_base_align);
	ring_params.ring_base_paddr = srng->base_paddr_unaligned +
		((unsigned long)(ring_params.ring_base_vaddr) -
		(unsigned long)srng->base_vaddr_unaligned);
	ring_params.num_entries = num_entries;

	if (soc->intr_mode == DP_INTR_MSI) {
		dp_srng_msi_setup(soc, &ring_params, ring_type, ring_num);
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
			FL("Using MSI for ring_type: %d, ring_num %d"),
			ring_type, ring_num);

	} else {
		ring_params.msi_data = 0;
		ring_params.msi_addr = 0;
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
			FL("Skipping MSI for ring_type: %d, ring_num %d"),
			ring_type, ring_num);
	}

	/*
	 * Setup interrupt timer and batch counter thresholds for
	 * interrupt mitigation based on ring type
	 */
	if (ring_type == REO_DST) {
		ring_params.intr_timer_thres_us =
			wlan_cfg_get_int_timer_threshold_rx(soc->wlan_cfg_ctx);
		ring_params.intr_batch_cntr_thres_entries =
			wlan_cfg_get_int_batch_threshold_rx(soc->wlan_cfg_ctx);
	} else if (ring_type == WBM2SW_RELEASE && (ring_num < 3)) {
		ring_params.intr_timer_thres_us =
			wlan_cfg_get_int_timer_threshold_tx(soc->wlan_cfg_ctx);
		ring_params.intr_batch_cntr_thres_entries =
			wlan_cfg_get_int_batch_threshold_tx(soc->wlan_cfg_ctx);
	} else {
		ring_params.intr_timer_thres_us =
			wlan_cfg_get_int_timer_threshold_other(soc->wlan_cfg_ctx);
		ring_params.intr_batch_cntr_thres_entries =
			wlan_cfg_get_int_batch_threshold_other(soc->wlan_cfg_ctx);
	}

	/* Enable low threshold interrupts for rx buffer rings (regular and
	 * monitor buffer rings.
	 * TODO: See if this is required for any other ring
	 */
	if ((ring_type == RXDMA_BUF) || (ring_type == RXDMA_MONITOR_BUF) ||
		(ring_type == RXDMA_MONITOR_STATUS)) {
		/* TODO: Setting low threshold to 1/8th of ring size
		 * see if this needs to be configurable
		 */
		ring_params.low_threshold = num_entries >> 3;
		ring_params.flags |= HAL_SRNG_LOW_THRES_INTR_ENABLE;
		ring_params.intr_timer_thres_us = 0x1000;
	}

	srng->hal_srng = hal_srng_setup(hal_soc, ring_type, ring_num,
		mac_id, &ring_params);

	if (!srng->hal_srng) {
		qdf_mem_free_consistent(soc->osdev, soc->osdev->dev,
				srng->alloc_size,
				srng->base_vaddr_unaligned,
				srng->base_paddr_unaligned, 0);
	}

	return 0;
}

/**
 * dp_srng_cleanup - Internal function to cleanup SRNG rings used by data path
 * Any buffers allocated and attached to ring entries are expected to be freed
 * before calling this function.
 */
static void dp_srng_cleanup(struct dp_soc *soc, struct dp_srng *srng,
	int ring_type, int ring_num)
{
	if (!srng->hal_srng) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
			FL("Ring type: %d, num:%d not setup"),
			ring_type, ring_num);
		return;
	}

	hal_srng_cleanup(soc->hal_soc, srng->hal_srng);

	qdf_mem_free_consistent(soc->osdev, soc->osdev->dev,
				srng->alloc_size,
				srng->base_vaddr_unaligned,
				srng->base_paddr_unaligned, 0);
	srng->hal_srng = NULL;
}

/* TODO: Need this interface from HIF */
void *hif_get_hal_handle(void *hif_handle);

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
	struct dp_soc *soc = int_ctx->soc;
	int ring = 0;
	uint32_t work_done  = 0;
	int budget = dp_budget;
	uint8_t tx_mask = int_ctx->tx_ring_mask;
	uint8_t rx_mask = int_ctx->rx_ring_mask;
	uint8_t rx_err_mask = int_ctx->rx_err_ring_mask;
	uint8_t rx_wbm_rel_mask = int_ctx->rx_wbm_rel_ring_mask;
	uint8_t reo_status_mask = int_ctx->reo_status_ring_mask;
	uint32_t remaining_quota = dp_budget;
	struct dp_pdev *pdev = NULL;

	/* Process Tx completion interrupts first to return back buffers */
	while (tx_mask) {
		if (tx_mask & 0x1) {
			work_done = dp_tx_comp_handler(soc,
					soc->tx_comp_ring[ring].hal_srng,
					remaining_quota);

			QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_DEBUG,
				"tx mask 0x%x ring %d, budget %d, work_done %d",
				tx_mask, ring, budget, work_done);

			budget -= work_done;
			if (budget <= 0)
				goto budget_done;

			remaining_quota = budget;
		}
		tx_mask = tx_mask >> 1;
		ring++;
	}


	/* Process REO Exception ring interrupt */
	if (rx_err_mask) {
		work_done = dp_rx_err_process(soc,
				soc->reo_exception_ring.hal_srng,
				remaining_quota);

		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_DEBUG,
			"REO Exception Ring: work_done %d budget %d",
			work_done, budget);

		budget -=  work_done;
		if (budget <= 0) {
			goto budget_done;
		}
		remaining_quota = budget;
	}

	/* Process Rx WBM release ring interrupt */
	if (rx_wbm_rel_mask) {
		work_done = dp_rx_wbm_err_process(soc,
				soc->rx_rel_ring.hal_srng, remaining_quota);

		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_DEBUG,
			"WBM Release Ring: work_done %d budget %d",
			work_done, budget);

		budget -=  work_done;
		if (budget <= 0) {
			goto budget_done;
		}
		remaining_quota = budget;
	}

	/* Process Rx interrupts */
	if (rx_mask) {
		for (ring = 0; ring < soc->num_reo_dest_rings; ring++) {
			if (rx_mask & (1 << ring)) {
				work_done = dp_rx_process(int_ctx,
					    soc->reo_dest_ring[ring].hal_srng,
					    remaining_quota);

				QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_DEBUG,
					"rx mask 0x%x ring %d, work_done %d budget %d",
					rx_mask, ring, work_done, budget);

				budget -=  work_done;
				if (budget <= 0)
					goto budget_done;
				remaining_quota = budget;
			}
		}
		for (ring = 0; ring < MAX_RX_MAC_RINGS; ring++) {
			/* Need to check on this, why is required */
			work_done = dp_rxdma_err_process(soc, ring,
						remaining_quota);
			budget -= work_done;
		}
	}

	if (reo_status_mask)
		dp_reo_status_ring_handler(soc);

	/* Process LMAC interrupts */
	for  (ring = 0 ; ring < MAX_PDEV_CNT; ring++) {
		pdev = soc->pdev_list[ring];
		if (pdev == NULL)
			continue;
		if (int_ctx->rx_mon_ring_mask & (1 << ring)) {
			work_done = dp_mon_process(soc, ring, remaining_quota);
			budget -= work_done;
			if (budget <= 0)
				goto budget_done;
			remaining_quota = budget;
		}

		if (int_ctx->rxdma2host_ring_mask & (1 << ring)) {
			work_done = dp_rxdma_err_process(soc, ring,
						remaining_quota);
			budget -=  work_done;
			if (budget <= 0)
				goto budget_done;
			remaining_quota = budget;
		}

		if (int_ctx->host2rxdma_ring_mask & (1 << ring)) {
			union dp_rx_desc_list_elem_t *desc_list = NULL;
			union dp_rx_desc_list_elem_t *tail = NULL;
			struct dp_srng *rx_refill_buf_ring =
				&pdev->rx_refill_buf_ring;

			DP_STATS_INC(pdev, replenish.low_thresh_intrs, 1);
			dp_rx_buffers_replenish(soc, ring,
				rx_refill_buf_ring,
				&soc->rx_desc_buf[ring], 0,
				&desc_list, &tail, HAL_RX_BUF_RBM_SW3_BM);
		}
	}

	qdf_lro_flush(int_ctx->lro_ctx);

budget_done:
	return dp_budget - budget;
}

#ifdef DP_INTR_POLL_BASED
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
	int i;

	if (qdf_atomic_read(&soc->cmn_init_done)) {
		for (i = 0;
			i < wlan_cfg_get_num_contexts(soc->wlan_cfg_ctx); i++)
			dp_service_srngs(&soc->intr_ctx[i], 0xffff);

		qdf_timer_mod(&soc->int_timer, DP_INTR_POLL_TIMER_MS);
	}
}

/*
 * dp_soc_interrupt_attach_poll() - Register handlers for DP interrupts
 * @txrx_soc: DP SOC handle
 *
 * Host driver will register for “DP_NUM_INTERRUPT_CONTEXTS” number of NAPI
 * contexts. Each NAPI context will have a tx_ring_mask , rx_ring_mask ,and
 * rx_monitor_ring mask to indicate the rings that are processed by the handler.
 *
 * Return: 0 for success. nonzero for failure.
 */
static QDF_STATUS dp_soc_interrupt_attach_poll(void *txrx_soc)
{
	struct dp_soc *soc = (struct dp_soc *)txrx_soc;
	int i;

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
	}

	qdf_timer_init(soc->osdev, &soc->int_timer,
			dp_interrupt_timer, (void *)soc,
			QDF_TIMER_TYPE_WAKE_APPS);

	return QDF_STATUS_SUCCESS;
}

#if defined(CONFIG_MCL)
extern int con_mode_monitor;
static QDF_STATUS dp_soc_interrupt_attach(void *txrx_soc);
/*
 * dp_soc_interrupt_attach_wrapper() - Register handlers for DP interrupts
 * @txrx_soc: DP SOC handle
 *
 * Call the appropriate attach function based on the mode of operation.
 * This is a WAR for enabling monitor mode.
 *
 * Return: 0 for success. nonzero for failure.
 */
static QDF_STATUS dp_soc_interrupt_attach_wrapper(void *txrx_soc)
{
	struct dp_soc *soc = (struct dp_soc *)txrx_soc;

	if (!(soc->wlan_cfg_ctx->napi_enabled) ||
	     con_mode_monitor == QDF_GLOBAL_MONITOR_MODE) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_INFO,
				  "%s: Poll mode", __func__);
		return dp_soc_interrupt_attach_poll(txrx_soc);
	} else {

		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_INFO,
				  "%s: Interrupt  mode", __func__);
		return dp_soc_interrupt_attach(txrx_soc);
	}
}
#else
static QDF_STATUS dp_soc_interrupt_attach_wrapper(void *txrx_soc)
{
	return dp_soc_interrupt_attach_poll(txrx_soc);
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
				rxdma2host_destination_ring_mac1 -
				wlan_cfg_get_hw_mac_idx(soc->wlan_cfg_ctx, j);
		}

		if (host2rxdma_ring_mask & (1 << j)) {
			irq_id_map[num_irq++] =
				host2rxdma_host_buf_ring_mac1 -
				wlan_cfg_get_hw_mac_idx(soc->wlan_cfg_ctx, j);
		}

		if (rx_mon_mask & (1 << j)) {
			irq_id_map[num_irq++] =
				ppdu_end_interrupts_mac1 -
				wlan_cfg_get_hw_mac_idx(soc->wlan_cfg_ctx, j);
			irq_id_map[num_irq++] =
				rxdma2host_monitor_status_ring_mac1 -
				wlan_cfg_get_hw_mac_idx(soc->wlan_cfg_ctx, j);
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
	int rx_err_ring_mask = wlan_cfg_get_rx_err_ring_mask(
					soc->wlan_cfg_ctx, intr_ctx_num);
	int rx_wbm_rel_ring_mask = wlan_cfg_get_rx_wbm_rel_ring_mask(
					soc->wlan_cfg_ctx, intr_ctx_num);
	int reo_status_ring_mask = wlan_cfg_get_reo_status_ring_mask(
					soc->wlan_cfg_ctx, intr_ctx_num);
	int rxdma2host_ring_mask = wlan_cfg_get_rxdma2host_ring_mask(
					soc->wlan_cfg_ctx, intr_ctx_num);

	unsigned int vector =
		(intr_ctx_num % msi_vector_count) + msi_vector_start;
	int num_irq = 0;

	soc->intr_mode = DP_INTR_MSI;

	if (tx_mask | rx_mask | rx_mon_mask | rx_err_ring_mask |
	    rx_wbm_rel_ring_mask | reo_status_ring_mask | rxdma2host_ring_mask)
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
static QDF_STATUS dp_soc_interrupt_attach(void *txrx_soc)
{
	struct dp_soc *soc = (struct dp_soc *)txrx_soc;

	int i = 0;
	int num_irq = 0;

	for (i = 0; i < wlan_cfg_get_num_contexts(soc->wlan_cfg_ctx); i++) {
		int ret = 0;

		/* Map of IRQ ids registered with one interrupt context */
		int irq_id_map[HIF_MAX_GRP_IRQ];

		int tx_mask =
			wlan_cfg_get_tx_ring_mask(soc->wlan_cfg_ctx, i);
		int rx_mask =
			wlan_cfg_get_rx_ring_mask(soc->wlan_cfg_ctx, i);
		int rx_mon_mask =
			wlan_cfg_get_rx_mon_ring_mask(soc->wlan_cfg_ctx, i);
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


		soc->intr_ctx[i].dp_intr_id = i;
		soc->intr_ctx[i].tx_ring_mask = tx_mask;
		soc->intr_ctx[i].rx_ring_mask = rx_mask;
		soc->intr_ctx[i].rx_mon_ring_mask = rx_mon_mask;
		soc->intr_ctx[i].rx_err_ring_mask = rx_err_ring_mask;
		soc->intr_ctx[i].rxdma2host_ring_mask = rxdma2host_ring_mask;
		soc->intr_ctx[i].host2rxdma_ring_mask = host2rxdma_ring_mask;
		soc->intr_ctx[i].rx_wbm_rel_ring_mask = rx_wbm_rel_ring_mask;
		soc->intr_ctx[i].reo_status_ring_mask = reo_status_ring_mask;

		soc->intr_ctx[i].soc = soc;

		num_irq = 0;

		dp_soc_interrupt_map_calculate(soc, i, &irq_id_map[0],
					       &num_irq);

		ret = hif_register_ext_group(soc->hif_handle,
				num_irq, irq_id_map, dp_service_srngs,
				&soc->intr_ctx[i], "dp_intr",
				HIF_EXEC_NAPI_TYPE, QCA_NAPI_DEF_SCALE_BIN_SHIFT);

		if (ret) {
			QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
			FL("failed, ret = %d"), ret);

			return QDF_STATUS_E_FAILURE;
		}
		soc->intr_ctx[i].lro_ctx = qdf_lro_init();
	}

	hif_configure_ext_group_interrupts(soc->hif_handle);

	return QDF_STATUS_SUCCESS;
}

/*
 * dp_soc_interrupt_detach() - Deregister any allocations done for interrupts
 * @txrx_soc: DP SOC handle
 *
 * Return: void
 */
static void dp_soc_interrupt_detach(void *txrx_soc)
{
	struct dp_soc *soc = (struct dp_soc *)txrx_soc;
	int i;

	if (soc->intr_mode == DP_INTR_POLL) {
		qdf_timer_stop(&soc->int_timer);
		qdf_timer_free(&soc->int_timer);
	} else {
		hif_deregister_exec_group(soc->hif_handle, "dp_intr");
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

		qdf_lro_deinit(soc->intr_ctx[i].lro_ctx);
	}
}

#define AVG_MAX_MPDUS_PER_TID 128
#define AVG_TIDS_PER_CLIENT 2
#define AVG_FLOWS_PER_TID 2
#define AVG_MSDUS_PER_FLOW 128
#define AVG_MSDUS_PER_MPDU 4

/*
 * Allocate and setup link descriptor pool that will be used by HW for
 * various link and queue descriptors and managed by WBM
 */
static int dp_hw_link_desc_pool_setup(struct dp_soc *soc)
{
	int link_desc_size = hal_get_link_desc_size(soc->hal_soc);
	int link_desc_align = hal_get_link_desc_align(soc->hal_soc);
	uint32_t max_clients = wlan_cfg_get_max_clients(soc->wlan_cfg_ctx);
	uint32_t num_mpdus_per_link_desc =
		hal_num_mpdus_per_link_desc(soc->hal_soc);
	uint32_t num_msdus_per_link_desc =
		hal_num_msdus_per_link_desc(soc->hal_soc);
	uint32_t num_mpdu_links_per_queue_desc =
		hal_num_mpdu_links_per_queue_desc(soc->hal_soc);
	uint32_t max_alloc_size = wlan_cfg_max_alloc_size(soc->wlan_cfg_ctx);
	uint32_t total_link_descs, total_mem_size;
	uint32_t num_mpdu_link_descs, num_mpdu_queue_descs;
	uint32_t num_tx_msdu_link_descs, num_rx_msdu_link_descs;
	uint32_t num_link_desc_banks;
	uint32_t last_bank_size = 0;
	uint32_t entry_size, num_entries;
	int i;
	uint32_t desc_id = 0;

	/* Only Tx queue descriptors are allocated from common link descriptor
	 * pool Rx queue descriptors are not included in this because (REO queue
	 * extension descriptors) they are expected to be allocated contiguously
	 * with REO queue descriptors
	 */
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

	/* Round up to power of 2 */
	total_link_descs = 1;
	while (total_link_descs < num_entries)
		total_link_descs <<= 1;

	QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_INFO_HIGH,
		FL("total_link_descs: %u, link_desc_size: %d"),
		total_link_descs, link_desc_size);
	total_mem_size =  total_link_descs * link_desc_size;

	total_mem_size += link_desc_align;

	if (total_mem_size <= max_alloc_size) {
		num_link_desc_banks = 0;
		last_bank_size = total_mem_size;
	} else {
		num_link_desc_banks = (total_mem_size) /
			(max_alloc_size - link_desc_align);
		last_bank_size = total_mem_size %
			(max_alloc_size - link_desc_align);
	}

	QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_INFO_HIGH,
		FL("total_mem_size: %d, num_link_desc_banks: %u"),
		total_mem_size, num_link_desc_banks);

	for (i = 0; i < num_link_desc_banks; i++) {
		soc->link_desc_banks[i].base_vaddr_unaligned =
			qdf_mem_alloc_consistent(soc->osdev, soc->osdev->dev,
			max_alloc_size,
			&(soc->link_desc_banks[i].base_paddr_unaligned));
		soc->link_desc_banks[i].size = max_alloc_size;

		soc->link_desc_banks[i].base_vaddr = (void *)((unsigned long)(
			soc->link_desc_banks[i].base_vaddr_unaligned) +
			((unsigned long)(
			soc->link_desc_banks[i].base_vaddr_unaligned) %
			link_desc_align));

		soc->link_desc_banks[i].base_paddr = (unsigned long)(
			soc->link_desc_banks[i].base_paddr_unaligned) +
			((unsigned long)(soc->link_desc_banks[i].base_vaddr) -
			(unsigned long)(
			soc->link_desc_banks[i].base_vaddr_unaligned));

		if (!soc->link_desc_banks[i].base_vaddr_unaligned) {
			QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
				FL("Link descriptor memory alloc failed"));
			goto fail;
		}
	}

	if (last_bank_size) {
		/* Allocate last bank in case total memory required is not exact
		 * multiple of max_alloc_size
		 */
		soc->link_desc_banks[i].base_vaddr_unaligned =
			qdf_mem_alloc_consistent(soc->osdev, soc->osdev->dev,
			last_bank_size,
			&(soc->link_desc_banks[i].base_paddr_unaligned));
		soc->link_desc_banks[i].size = last_bank_size;

		soc->link_desc_banks[i].base_vaddr = (void *)((unsigned long)
			(soc->link_desc_banks[i].base_vaddr_unaligned) +
			((unsigned long)(
			soc->link_desc_banks[i].base_vaddr_unaligned) %
			link_desc_align));

		soc->link_desc_banks[i].base_paddr =
			(unsigned long)(
			soc->link_desc_banks[i].base_paddr_unaligned) +
			((unsigned long)(soc->link_desc_banks[i].base_vaddr) -
			(unsigned long)(
			soc->link_desc_banks[i].base_vaddr_unaligned));
	}


	/* Allocate and setup link descriptor idle list for HW internal use */
	entry_size = hal_srng_get_entrysize(soc->hal_soc, WBM_IDLE_LINK);
	total_mem_size = entry_size * total_link_descs;

	if (total_mem_size <= max_alloc_size) {
		void *desc;

		if (dp_srng_setup(soc, &soc->wbm_idle_link_ring,
			WBM_IDLE_LINK, 0, 0, total_link_descs)) {
			QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
				FL("Link desc idle ring setup failed"));
			goto fail;
		}

		hal_srng_access_start_unlocked(soc->hal_soc,
			soc->wbm_idle_link_ring.hal_srng);

		for (i = 0; i < MAX_LINK_DESC_BANKS &&
			soc->link_desc_banks[i].base_paddr; i++) {
			uint32_t num_entries = (soc->link_desc_banks[i].size -
				((unsigned long)(
				soc->link_desc_banks[i].base_vaddr) -
				(unsigned long)(
				soc->link_desc_banks[i].base_vaddr_unaligned)))
				/ link_desc_size;
			unsigned long paddr = (unsigned long)(
				soc->link_desc_banks[i].base_paddr);

			while (num_entries && (desc = hal_srng_src_get_next(
				soc->hal_soc,
				soc->wbm_idle_link_ring.hal_srng))) {
				hal_set_link_desc_addr(desc,
					LINK_DESC_COOKIE(desc_id, i), paddr);
				num_entries--;
				desc_id++;
				paddr += link_desc_size;
			}
		}
		hal_srng_access_end_unlocked(soc->hal_soc,
			soc->wbm_idle_link_ring.hal_srng);
	} else {
		uint32_t num_scatter_bufs;
		uint32_t num_entries_per_buf;
		uint32_t rem_entries;
		uint8_t *scatter_buf_ptr;
		uint16_t scatter_buf_num;

		soc->wbm_idle_scatter_buf_size =
			hal_idle_list_scatter_buf_size(soc->hal_soc);
		num_entries_per_buf = hal_idle_scatter_buf_num_entries(
			soc->hal_soc, soc->wbm_idle_scatter_buf_size);
		num_scatter_bufs = hal_idle_list_num_scatter_bufs(
					soc->hal_soc, total_mem_size,
					soc->wbm_idle_scatter_buf_size);

		for (i = 0; i < num_scatter_bufs; i++) {
			soc->wbm_idle_scatter_buf_base_vaddr[i] =
				qdf_mem_alloc_consistent(soc->osdev, soc->osdev->dev,
				soc->wbm_idle_scatter_buf_size,
				&(soc->wbm_idle_scatter_buf_base_paddr[i]));
			if (soc->wbm_idle_scatter_buf_base_vaddr[i] == NULL) {
				QDF_TRACE(QDF_MODULE_ID_DP,
					QDF_TRACE_LEVEL_ERROR,
					FL("Scatter list memory alloc failed"));
				goto fail;
			}
		}

		/* Populate idle list scatter buffers with link descriptor
		 * pointers
		 */
		scatter_buf_num = 0;
		scatter_buf_ptr = (uint8_t *)(
			soc->wbm_idle_scatter_buf_base_vaddr[scatter_buf_num]);
		rem_entries = num_entries_per_buf;

		for (i = 0; i < MAX_LINK_DESC_BANKS &&
			soc->link_desc_banks[i].base_paddr; i++) {
			uint32_t num_link_descs =
				(soc->link_desc_banks[i].size -
				((unsigned long)(
				soc->link_desc_banks[i].base_vaddr) -
				(unsigned long)(
				soc->link_desc_banks[i].base_vaddr_unaligned)))
				/ link_desc_size;
			unsigned long paddr = (unsigned long)(
				soc->link_desc_banks[i].base_paddr);

			while (num_link_descs) {
				hal_set_link_desc_addr((void *)scatter_buf_ptr,
					LINK_DESC_COOKIE(desc_id, i), paddr);
				num_link_descs--;
				desc_id++;
				paddr += link_desc_size;
				rem_entries--;
				if (rem_entries) {
					scatter_buf_ptr += entry_size;
				} else {
					rem_entries = num_entries_per_buf;
					scatter_buf_num++;

					if (scatter_buf_num >= num_scatter_bufs)
						break;

					scatter_buf_ptr = (uint8_t *)(
						soc->wbm_idle_scatter_buf_base_vaddr[
						scatter_buf_num]);
				}
			}
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
	return 0;

fail:
	if (soc->wbm_idle_link_ring.hal_srng) {
		dp_srng_cleanup(soc->hal_soc, &soc->wbm_idle_link_ring,
			WBM_IDLE_LINK, 0);
	}

	for (i = 0; i < MAX_IDLE_SCATTER_BUFS; i++) {
		if (soc->wbm_idle_scatter_buf_base_vaddr[i]) {
			qdf_mem_free_consistent(soc->osdev, soc->osdev->dev,
				soc->wbm_idle_scatter_buf_size,
				soc->wbm_idle_scatter_buf_base_vaddr[i],
				soc->wbm_idle_scatter_buf_base_paddr[i], 0);
			soc->wbm_idle_scatter_buf_base_vaddr[i] = NULL;
		}
	}

	for (i = 0; i < MAX_LINK_DESC_BANKS; i++) {
		if (soc->link_desc_banks[i].base_vaddr_unaligned) {
			qdf_mem_free_consistent(soc->osdev, soc->osdev->dev,
				soc->link_desc_banks[i].size,
				soc->link_desc_banks[i].base_vaddr_unaligned,
				soc->link_desc_banks[i].base_paddr_unaligned,
				0);
			soc->link_desc_banks[i].base_vaddr_unaligned = NULL;
		}
	}
	return QDF_STATUS_E_FAILURE;
}

/*
 * Free link descriptor pool that was setup HW
 */
static void dp_hw_link_desc_pool_cleanup(struct dp_soc *soc)
{
	int i;

	if (soc->wbm_idle_link_ring.hal_srng) {
		dp_srng_cleanup(soc, &soc->wbm_idle_link_ring,
			WBM_IDLE_LINK, 0);
	}

	for (i = 0; i < MAX_IDLE_SCATTER_BUFS; i++) {
		if (soc->wbm_idle_scatter_buf_base_vaddr[i]) {
			qdf_mem_free_consistent(soc->osdev, soc->osdev->dev,
				soc->wbm_idle_scatter_buf_size,
				soc->wbm_idle_scatter_buf_base_vaddr[i],
				soc->wbm_idle_scatter_buf_base_paddr[i], 0);
			soc->wbm_idle_scatter_buf_base_vaddr[i] = NULL;
		}
	}

	for (i = 0; i < MAX_LINK_DESC_BANKS; i++) {
		if (soc->link_desc_banks[i].base_vaddr_unaligned) {
			qdf_mem_free_consistent(soc->osdev, soc->osdev->dev,
				soc->link_desc_banks[i].size,
				soc->link_desc_banks[i].base_vaddr_unaligned,
				soc->link_desc_banks[i].base_paddr_unaligned,
				0);
			soc->link_desc_banks[i].base_vaddr_unaligned = NULL;
		}
	}
}

/* TODO: Following should be configurable */
#define WBM_RELEASE_RING_SIZE 64
#define TCL_CMD_RING_SIZE 32
#define TCL_STATUS_RING_SIZE 32
#if defined(QCA_WIFI_QCA6290)
#define REO_DST_RING_SIZE 1024
#else
#define REO_DST_RING_SIZE 2048
#endif
#define REO_REINJECT_RING_SIZE 32
#define RX_RELEASE_RING_SIZE 1024
#define REO_EXCEPTION_RING_SIZE 128
#define REO_CMD_RING_SIZE 64
#define REO_STATUS_RING_SIZE 128
#define RXDMA_BUF_RING_SIZE 1024
#define RXDMA_REFILL_RING_SIZE 4096
#define RXDMA_MONITOR_BUF_RING_SIZE 4096
#define RXDMA_MONITOR_DST_RING_SIZE 2048
#define RXDMA_MONITOR_STATUS_RING_SIZE 1024
#define RXDMA_MONITOR_DESC_RING_SIZE 4096
#define RXDMA_ERR_DST_RING_SIZE 1024

/*
 * dp_wds_aging_timer_fn() - Timer callback function for WDS aging
 * @soc: Datapath SOC handle
 *
 * This is a timer function used to age out stale WDS nodes from
 * AST table
 */
#ifdef FEATURE_WDS
static void dp_wds_aging_timer_fn(void *soc_hdl)
{
	struct dp_soc *soc = (struct dp_soc *) soc_hdl;
	struct dp_pdev *pdev;
	struct dp_vdev *vdev;
	struct dp_peer *peer;
	struct dp_ast_entry *ase, *temp_ase;
	int i;

	qdf_spin_lock_bh(&soc->ast_lock);

	for (i = 0; i < MAX_PDEV_CNT && soc->pdev_list[i]; i++) {
		pdev = soc->pdev_list[i];
		DP_PDEV_ITERATE_VDEV_LIST(pdev, vdev) {
			DP_VDEV_ITERATE_PEER_LIST(vdev, peer) {
				DP_PEER_ITERATE_ASE_LIST(peer, ase, temp_ase) {
					/*
					 * Do not expire static ast entries
					 * and HM WDS entries
					 */
					if (ase->type ==
						CDP_TXRX_AST_TYPE_STATIC ||
						ase->type ==
						CDP_TXRX_AST_TYPE_WDS_HM)
						continue;

					if (ase->is_active) {
						ase->is_active = FALSE;
						continue;
					}

					DP_STATS_INC(soc, ast.aged_out, 1);
					dp_peer_del_ast(soc, ase);
				}
			}
		}

	}

	qdf_spin_unlock_bh(&soc->ast_lock);

	if (qdf_atomic_read(&soc->cmn_init_done))
		qdf_timer_mod(&soc->wds_aging_timer, DP_WDS_AGING_TIMER_DEFAULT_MS);
}

/*
 * dp_soc_wds_attach() - Setup WDS timer and AST table
 * @soc:		Datapath SOC handle
 *
 * Return: None
 */
static void dp_soc_wds_attach(struct dp_soc *soc)
{
	qdf_timer_init(soc->osdev, &soc->wds_aging_timer,
			dp_wds_aging_timer_fn, (void *)soc,
			QDF_TIMER_TYPE_WAKE_APPS);

	qdf_timer_mod(&soc->wds_aging_timer, DP_WDS_AGING_TIMER_DEFAULT_MS);
}

/*
 * dp_soc_wds_detach() - Detach WDS data structures and timers
 * @txrx_soc: DP SOC handle
 *
 * Return: None
 */
static void dp_soc_wds_detach(struct dp_soc *soc)
{
	qdf_timer_stop(&soc->wds_aging_timer);
	qdf_timer_free(&soc->wds_aging_timer);
}
#else
static void dp_soc_wds_attach(struct dp_soc *soc)
{
}

static void dp_soc_wds_detach(struct dp_soc *soc)
{
}
#endif

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
		if (nss_config == 1) {
			/*
			 * Setting Tx ring map for one nss offloaded radio
			 */
			soc->tx_ring_map[i] = dp_cpu_ring_map[DP_NSS_FIRST_RADIO_OFFLOADED_MAP][i];
		} else if (nss_config == 2) {
			/*
			 * Setting Tx ring for two nss offloaded radios
			 */
			soc->tx_ring_map[i] = dp_cpu_ring_map[DP_NSS_SECOND_RADIO_OFFLOADED_MAP][i];
		} else {
			/*
			 * Setting Tx ring map for all nss offloaded radios
			 */
			soc->tx_ring_map[i] = dp_cpu_ring_map[DP_NSS_ALL_RADIO_OFFLOADED_MAP][i];
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
		status = ((nss_config) & (1 << ring_num));
		break;
	default:
		break;
	}

	return status;
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
	int *grp_mask = NULL;
	int group_number, mask, num_ring;

	/* number of tx ring */
	num_ring = wlan_cfg_num_tcl_data_rings(soc->wlan_cfg_ctx);

	/*
	 * group mask for tx completion  ring.
	 */
	grp_mask =  &soc->wlan_cfg_ctx->int_tx_ring_mask[0];

	/* loop and reset the mask for only offloaded ring */
	for (j = 0; j < num_ring; j++) {
		if (!dp_soc_ring_if_nss_offloaded(soc, WBM2SW_RELEASE, j)) {
			continue;
		}

		/*
		 * Group number corresponding to tx offloaded ring.
		 */
		group_number = dp_srng_find_ring_in_mask(j, grp_mask);
		if (group_number < 0) {
			QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_DEBUG,
					FL("ring not part of any group; ring_type: %d,ring_num %d"),
					WBM2SW_RELEASE, j);
			return;
		}

		/* reset the tx mask for offloaded ring */
		mask = wlan_cfg_get_tx_ring_mask(soc->wlan_cfg_ctx, group_number);
		mask &= (~(1 << j));

		/*
		 * reset the interrupt mask for offloaded ring.
		 */
		wlan_cfg_set_tx_ring_mask(soc->wlan_cfg_ctx, group_number, mask);
	}

	/* number of rx rings */
	num_ring = wlan_cfg_num_reo_dest_rings(soc->wlan_cfg_ctx);

	/*
	 * group mask for reo destination ring.
	 */
	grp_mask = &soc->wlan_cfg_ctx->int_rx_ring_mask[0];

	/* loop and reset the mask for only offloaded ring */
	for (j = 0; j < num_ring; j++) {
		if (!dp_soc_ring_if_nss_offloaded(soc, REO_DST, j)) {
			continue;
		}

		/*
		 * Group number corresponding to rx offloaded ring.
		 */
		group_number = dp_srng_find_ring_in_mask(j, grp_mask);
		if (group_number < 0) {
			QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_DEBUG,
					FL("ring not part of any group; ring_type: %d,ring_num %d"),
					REO_DST, j);
			return;
		}

		/* set the interrupt mask for offloaded ring */
		mask =  wlan_cfg_get_rx_ring_mask(soc->wlan_cfg_ctx, group_number);
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
		if (!dp_soc_ring_if_nss_offloaded(soc, RXDMA_BUF, j)) {
			continue;
		}

		/*
		 * Group number corresponding to rx offloaded ring.
		 */
		group_number = dp_srng_find_ring_in_mask(j, grp_mask);
		if (group_number < 0) {
			QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_DEBUG,
					FL("ring not part of any group; ring_type: %d,ring_num %d"),
					REO_DST, j);
			return;
		}

		/* set the interrupt mask for offloaded ring */
		mask =  wlan_cfg_get_host2rxdma_ring_mask(soc->wlan_cfg_ctx,
				group_number);
		mask &= (~(1 << j));

		/*
		 * set the interrupt mask to zero for rx offloaded radio.
		 */
		wlan_cfg_set_host2rxdma_ring_mask(soc->wlan_cfg_ctx,
			group_number, mask);
	}
}

#ifdef IPA_OFFLOAD
/**
 * dp_reo_remap_config() - configure reo remap register value based
 *                         nss configuration.
 *		based on offload_radio value below remap configuration
 *		get applied.
 *		0 - both Radios handled by host (remap rings 1, 2, 3 & 4)
 *		1 - 1st Radio handled by NSS (remap rings 2, 3 & 4)
 *		2 - 2nd Radio handled by NSS (remap rings 1, 2 & 4)
 *		3 - both Radios handled by NSS (remap not required)
 *		4 - IPA OFFLOAD enabled (remap rings 1,2 & 3)
 *
 * @remap1: output parameter indicates reo remap 1 register value
 * @remap2: output parameter indicates reo remap 2 register value
 * Return: bool type, true if remap is configured else false.
 */
static bool dp_reo_remap_config(struct dp_soc *soc,
				uint32_t *remap1,
				uint32_t *remap2)
{

	*remap1 = ((0x1 << 0) | (0x2 << 3) | (0x3 << 6) | (0x1 << 9) |
		(0x2 << 12) | (0x3 << 15) | (0x1 << 18) | (0x2 << 21)) << 8;

	*remap2 = ((0x3 << 0) | (0x1 << 3) | (0x2 << 6) | (0x3 << 9) |
		(0x1 << 12) | (0x2 << 15) | (0x3 << 18) | (0x1 << 21)) << 8;

	return true;
}
#else
static bool dp_reo_remap_config(struct dp_soc *soc,
				uint32_t *remap1,
				uint32_t *remap2)
{
	uint8_t offload_radio = wlan_cfg_get_dp_soc_nss_cfg(soc->wlan_cfg_ctx);

	switch (offload_radio) {
	case 0:
		*remap1 = ((0x1 << 0) | (0x2 << 3) | (0x3 << 6) |
			(0x4 << 9) | (0x1 << 12) | (0x2 << 15) |
			(0x3 << 18) | (0x4 << 21)) << 8;

		*remap2 = ((0x1 << 0) | (0x2 << 3) | (0x3 << 6) |
			(0x4 << 9) | (0x1 << 12) | (0x2 << 15) |
			(0x3 << 18) | (0x4 << 21)) << 8;
		break;

	case 1:
		*remap1 = ((0x2 << 0) | (0x3 << 3) | (0x4 << 6) |
			(0x2 << 9) | (0x3 << 12) | (0x4 << 15) |
			(0x2 << 18) | (0x3 << 21)) << 8;

		*remap2 = ((0x4 << 0) | (0x2 << 3) | (0x3 << 6) |
			(0x4 << 9) | (0x2 << 12) | (0x3 << 15) |
			(0x4 << 18) | (0x2 << 21)) << 8;
		break;

	case 2:
		*remap1 = ((0x1 << 0) | (0x3 << 3) | (0x4 << 6) |
			(0x1 << 9) | (0x3 << 12) | (0x4 << 15) |
			(0x1 << 18) | (0x3 << 21)) << 8;

		*remap2 = ((0x4 << 0) | (0x1 << 3) | (0x3 << 6) |
			(0x4 << 9) | (0x1 << 12) | (0x3 << 15) |
			(0x4 << 18) | (0x1 << 21)) << 8;
		break;

	case 3:
		/* return false if both radios are offloaded to NSS */
		return false;
	}
	return true;
}
#endif

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
	case 0:
		*frag_dst_ring = HAL_SRNG_REO_EXCEPTION;
		break;
	case 3:
		*frag_dst_ring = HAL_SRNG_REO_ALTERNATE_SELECT;
		break;
	default:
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
				FL("dp_reo_frag_dst_set invalid offload radio config"));
		break;
	}
}

/*
 * dp_soc_cmn_setup() - Common SoC level initializion
 * @soc:		Datapath SOC handle
 *
 * This is an internal function used to setup common SOC data structures,
 * to be called from PDEV attach after receiving HW mode capabilities from FW
 */
static int dp_soc_cmn_setup(struct dp_soc *soc)
{
	int i;
	struct hal_reo_params reo_params;
	int tx_ring_size;
	int tx_comp_ring_size;

	if (qdf_atomic_read(&soc->cmn_init_done))
		return 0;

	if (dp_peer_find_attach(soc))
		goto fail0;

	if (dp_hw_link_desc_pool_setup(soc))
		goto fail1;

	/* Setup SRNG rings */
	/* Common rings */
	if (dp_srng_setup(soc, &soc->wbm_desc_rel_ring, SW2WBM_RELEASE, 0, 0,
		WBM_RELEASE_RING_SIZE)) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
			FL("dp_srng_setup failed for wbm_desc_rel_ring"));
		goto fail1;
	}


	soc->num_tcl_data_rings = 0;
	/* Tx data rings */
	if (!wlan_cfg_per_pdev_tx_ring(soc->wlan_cfg_ctx)) {
		soc->num_tcl_data_rings =
			wlan_cfg_num_tcl_data_rings(soc->wlan_cfg_ctx);
		tx_comp_ring_size =
			wlan_cfg_tx_comp_ring_size(soc->wlan_cfg_ctx);
		tx_ring_size =
			wlan_cfg_tx_ring_size(soc->wlan_cfg_ctx);
		for (i = 0; i < soc->num_tcl_data_rings; i++) {
			if (dp_srng_setup(soc, &soc->tcl_data_ring[i],
				TCL_DATA, i, 0, tx_ring_size)) {
				QDF_TRACE(QDF_MODULE_ID_DP,
					QDF_TRACE_LEVEL_ERROR,
					FL("dp_srng_setup failed for tcl_data_ring[%d]"), i);
				goto fail1;
			}
			/*
			 * TBD: Set IPA WBM ring size with ini IPA UC tx buffer
			 * count
			 */
			if (dp_srng_setup(soc, &soc->tx_comp_ring[i],
				WBM2SW_RELEASE, i, 0, tx_comp_ring_size)) {
				QDF_TRACE(QDF_MODULE_ID_DP,
					QDF_TRACE_LEVEL_ERROR,
					FL("dp_srng_setup failed for tx_comp_ring[%d]"), i);
				goto fail1;
			}
		}
	} else {
		/* This will be incremented during per pdev ring setup */
		soc->num_tcl_data_rings = 0;
	}

	if (dp_tx_soc_attach(soc)) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
				FL("dp_tx_soc_attach failed"));
		goto fail1;
	}

	/* TCL command and status rings */
	if (dp_srng_setup(soc, &soc->tcl_cmd_ring, TCL_CMD, 0, 0,
		TCL_CMD_RING_SIZE)) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
			FL("dp_srng_setup failed for tcl_cmd_ring"));
		goto fail1;
	}

	if (dp_srng_setup(soc, &soc->tcl_status_ring, TCL_STATUS, 0, 0,
		TCL_STATUS_RING_SIZE)) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
			FL("dp_srng_setup failed for tcl_status_ring"));
		goto fail1;
	}


	/* TBD: call dp_tx_init to setup Tx SW descriptors and MSDU extension
	 * descriptors
	 */

	/* Rx data rings */
	if (!wlan_cfg_per_pdev_rx_ring(soc->wlan_cfg_ctx)) {
		soc->num_reo_dest_rings =
			wlan_cfg_num_reo_dest_rings(soc->wlan_cfg_ctx);
		QDF_TRACE(QDF_MODULE_ID_DP,
			QDF_TRACE_LEVEL_ERROR,
			FL("num_reo_dest_rings %d\n"), soc->num_reo_dest_rings);
		for (i = 0; i < soc->num_reo_dest_rings; i++) {
			if (dp_srng_setup(soc, &soc->reo_dest_ring[i], REO_DST,
				i, 0, REO_DST_RING_SIZE)) {
				QDF_TRACE(QDF_MODULE_ID_DP,
					QDF_TRACE_LEVEL_ERROR,
					FL("dp_srng_setup failed for reo_dest_ring[%d]"), i);
				goto fail1;
			}
		}
	} else {
		/* This will be incremented during per pdev ring setup */
		soc->num_reo_dest_rings = 0;
	}

	/* LMAC RxDMA to SW Rings configuration */
	if (!wlan_cfg_per_pdev_lmac_ring(soc->wlan_cfg_ctx)) {
		/* Only valid for MCL */
		struct dp_pdev *pdev = soc->pdev_list[0];

		for (i = 0; i < MAX_RX_MAC_RINGS; i++) {
			if (dp_srng_setup(soc, &pdev->rxdma_err_dst_ring[i],
				RXDMA_DST, 0, i, RXDMA_ERR_DST_RING_SIZE)) {
				QDF_TRACE(QDF_MODULE_ID_DP,
					QDF_TRACE_LEVEL_ERROR,
					FL("dp_srng_setup failed for rxdma_err_dst_ring"));
				goto fail1;
			}
		}
	}
	/* TBD: call dp_rx_init to setup Rx SW descriptors */

	/* REO reinjection ring */
	if (dp_srng_setup(soc, &soc->reo_reinject_ring, REO_REINJECT, 0, 0,
		REO_REINJECT_RING_SIZE)) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
			FL("dp_srng_setup failed for reo_reinject_ring"));
		goto fail1;
	}


	/* Rx release ring */
	if (dp_srng_setup(soc, &soc->rx_rel_ring, WBM2SW_RELEASE, 3, 0,
		RX_RELEASE_RING_SIZE)) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
			FL("dp_srng_setup failed for rx_rel_ring"));
		goto fail1;
	}


	/* Rx exception ring */
	if (dp_srng_setup(soc, &soc->reo_exception_ring, REO_EXCEPTION, 0,
		MAX_REO_DEST_RINGS, REO_EXCEPTION_RING_SIZE)) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
			FL("dp_srng_setup failed for reo_exception_ring"));
		goto fail1;
	}


	/* REO command and status rings */
	if (dp_srng_setup(soc, &soc->reo_cmd_ring, REO_CMD, 0, 0,
		REO_CMD_RING_SIZE)) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
			FL("dp_srng_setup failed for reo_cmd_ring"));
		goto fail1;
	}

	hal_reo_init_cmd_ring(soc->hal_soc, soc->reo_cmd_ring.hal_srng);
	TAILQ_INIT(&soc->rx.reo_cmd_list);
	qdf_spinlock_create(&soc->rx.reo_cmd_lock);

	if (dp_srng_setup(soc, &soc->reo_status_ring, REO_STATUS, 0, 0,
		REO_STATUS_RING_SIZE)) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
			FL("dp_srng_setup failed for reo_status_ring"));
		goto fail1;
	}

	qdf_spinlock_create(&soc->ast_lock);
	dp_soc_wds_attach(soc);

	/* Reset the cpu ring map if radio is NSS offloaded */
	if (wlan_cfg_get_dp_soc_nss_cfg(soc->wlan_cfg_ctx)) {
		dp_soc_reset_cpu_ring_map(soc);
		dp_soc_reset_intr_mask(soc);
	}

	/* Setup HW REO */
	qdf_mem_zero(&reo_params, sizeof(reo_params));

	if (wlan_cfg_is_rx_hash_enabled(soc->wlan_cfg_ctx)) {

		/*
		 * Reo ring remap is not required if both radios
		 * are offloaded to NSS
		 */
		if (!dp_reo_remap_config(soc,
					&reo_params.remap1,
					&reo_params.remap2))
			goto out;

		reo_params.rx_hash_enabled = true;
	}

	/* setup the global rx defrag waitlist */
	TAILQ_INIT(&soc->rx.defrag.waitlist);
	soc->rx.defrag.timeout_ms =
		wlan_cfg_get_rx_defrag_min_timeout(soc->wlan_cfg_ctx);
	soc->rx.flags.defrag_timeout_check =
		wlan_cfg_get_defrag_timeout_check(soc->wlan_cfg_ctx);

out:
	/*
	 * set the fragment destination ring
	 */
	dp_reo_frag_dst_set(soc, &reo_params.frag_dst_ring);

	hal_reo_setup(soc->hal_soc, &reo_params);

	qdf_atomic_set(&soc->cmn_init_done, 1);
	qdf_nbuf_queue_init(&soc->htt_stats.msg);
	return 0;
fail1:
	/*
	 * Cleanup will be done as part of soc_detach, which will
	 * be called on pdev attach failure
	 */
fail0:
	return QDF_STATUS_E_FAILURE;
}

static void dp_pdev_detach_wifi3(struct cdp_pdev *txrx_pdev, int force);

static void dp_lro_hash_setup(struct dp_soc *soc)
{
	struct cdp_lro_hash_config lro_hash;

	if (!wlan_cfg_is_lro_enabled(soc->wlan_cfg_ctx) &&
		!wlan_cfg_is_rx_hash_enabled(soc->wlan_cfg_ctx)) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
			 FL("LRO disabled RX hash disabled"));
		return;
	}

	qdf_mem_zero(&lro_hash, sizeof(lro_hash));

	if (wlan_cfg_is_lro_enabled(soc->wlan_cfg_ctx)) {
		lro_hash.lro_enable = 1;
		lro_hash.tcp_flag = QDF_TCPHDR_ACK;
		lro_hash.tcp_flag_mask = QDF_TCPHDR_FIN | QDF_TCPHDR_SYN |
			 QDF_TCPHDR_RST | QDF_TCPHDR_ACK | QDF_TCPHDR_URG |
			 QDF_TCPHDR_ECE | QDF_TCPHDR_CWR;
	}

	QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_INFO_LOW, FL("enabled"));
	qdf_get_random_bytes(lro_hash.toeplitz_hash_ipv4,
		 (sizeof(lro_hash.toeplitz_hash_ipv4[0]) *
		 LRO_IPV4_SEED_ARR_SZ));
	qdf_get_random_bytes(lro_hash.toeplitz_hash_ipv6,
		 (sizeof(lro_hash.toeplitz_hash_ipv6[0]) *
		 LRO_IPV6_SEED_ARR_SZ));

	QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_INFO_LOW,
		 "lro_hash: lro_enable: 0x%x tcp_flag 0x%x tcp_flag_mask 0x%x",
		 lro_hash.lro_enable, lro_hash.tcp_flag,
		 lro_hash.tcp_flag_mask);

	qdf_trace_hex_dump(QDF_MODULE_ID_DP,
		 QDF_TRACE_LEVEL_ERROR,
		 (void *)lro_hash.toeplitz_hash_ipv4,
		 (sizeof(lro_hash.toeplitz_hash_ipv4[0]) *
		 LRO_IPV4_SEED_ARR_SZ));

	qdf_trace_hex_dump(QDF_MODULE_ID_DP,
		 QDF_TRACE_LEVEL_ERROR,
		 (void *)lro_hash.toeplitz_hash_ipv6,
		 (sizeof(lro_hash.toeplitz_hash_ipv6[0]) *
		 LRO_IPV6_SEED_ARR_SZ));

	qdf_assert(soc->cdp_soc.ol_ops->lro_hash_config);

	if (soc->cdp_soc.ol_ops->lro_hash_config)
		(void)soc->cdp_soc.ol_ops->lro_hash_config
			(soc->ctrl_psoc, &lro_hash);
}

/*
* dp_rxdma_ring_setup() - configure the RX DMA rings
* @soc: data path SoC handle
* @pdev: Physical device handle
*
* Return: 0 - success, > 0 - failure
*/
#ifdef QCA_HOST2FW_RXBUF_RING
static int dp_rxdma_ring_setup(struct dp_soc *soc,
	 struct dp_pdev *pdev)
{
	int max_mac_rings =
		 wlan_cfg_get_num_mac_rings
			(pdev->wlan_cfg_ctx);
	int i;

	for (i = 0; i < max_mac_rings; i++) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
			 "%s: pdev_id %d mac_id %d\n",
			 __func__, pdev->pdev_id, i);
		if (dp_srng_setup(soc, &pdev->rx_mac_buf_ring[i],
			 RXDMA_BUF, 1, i, RXDMA_BUF_RING_SIZE)) {
			QDF_TRACE(QDF_MODULE_ID_DP,
				 QDF_TRACE_LEVEL_ERROR,
				 FL("failed rx mac ring setup"));
			return QDF_STATUS_E_FAILURE;
		}
	}
	return QDF_STATUS_SUCCESS;
}
#else
static int dp_rxdma_ring_setup(struct dp_soc *soc,
	 struct dp_pdev *pdev)
{
	return QDF_STATUS_SUCCESS;
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
	for (map_id = 0; map_id < DP_MAX_TID_MAPS; map_id++) {
		qdf_mem_copy(pdev->dscp_tid_map[map_id], default_dscp_tid_map,
				sizeof(default_dscp_tid_map));
	}
	for (map_id = 0; map_id < HAL_MAX_HW_DSCP_TID_MAPS; map_id++) {
		hal_tx_set_dscp_tid_map(pdev->soc->hal_soc,
				pdev->dscp_tid_map[map_id],
				map_id);
	}
}

#ifdef QCA_SUPPORT_SON
/**
 * dp_mark_peer_inact(): Update peer inactivity status
 * @peer_handle - datapath peer handle
 *
 * Return: void
 */
void dp_mark_peer_inact(void *peer_handle, bool inactive)
{
	struct dp_peer *peer = (struct dp_peer *)peer_handle;
	struct dp_pdev *pdev;
	struct dp_soc *soc;
	bool inactive_old;

	if (!peer)
		return;

	pdev = peer->vdev->pdev;
	soc = pdev->soc;

	inactive_old = peer->peer_bs_inact_flag == 1;
	if (!inactive)
		peer->peer_bs_inact = soc->pdev_bs_inact_reload;
	peer->peer_bs_inact_flag = inactive ? 1 : 0;

	if (inactive_old != inactive) {
		/**
		 * Note: a node lookup can happen in RX datapath context
		 * when a node changes from inactive to active (at most once
		 * per inactivity timeout threshold)
		 */
		if (soc->cdp_soc.ol_ops->record_act_change) {
			soc->cdp_soc.ol_ops->record_act_change(pdev->osif_pdev,
					peer->mac_addr.raw, !inactive);
		}
	}
}

/**
 * dp_txrx_peer_find_inact_timeout_handler(): Inactivity timeout function
 *
 * Periodically checks the inactivity status
 */
static os_timer_func(dp_txrx_peer_find_inact_timeout_handler)
{
	struct dp_pdev *pdev;
	struct dp_vdev *vdev;
	struct dp_peer *peer;
	struct dp_soc *soc;
	int i;

	OS_GET_TIMER_ARG(soc, struct dp_soc *);

	qdf_spin_lock(&soc->peer_ref_mutex);

	for (i = 0; i < soc->pdev_count; i++) {
	pdev = soc->pdev_list[i];
	TAILQ_FOREACH(vdev, &pdev->vdev_list, vdev_list_elem) {
		if (vdev->opmode != wlan_op_mode_ap)
			continue;

		TAILQ_FOREACH(peer, &vdev->peer_list, peer_list_elem) {
			if (!peer->authorize) {
				/**
				 * Inactivity check only interested in
				 * connected node
				 */
				continue;
			}
			if (peer->peer_bs_inact > soc->pdev_bs_inact_reload) {
				/**
				 * This check ensures we do not wait extra long
				 * due to the potential race condition
				 */
				peer->peer_bs_inact = soc->pdev_bs_inact_reload;
			}
			if (peer->peer_bs_inact > 0) {
				/* Do not let it wrap around */
				peer->peer_bs_inact--;
			}
			if (peer->peer_bs_inact == 0)
				dp_mark_peer_inact(peer, true);
		}
	}
	}

	qdf_spin_unlock(&soc->peer_ref_mutex);
	qdf_timer_mod(&soc->pdev_bs_inact_timer,
		      soc->pdev_bs_inact_interval * 1000);
}


/**
 * dp_free_inact_timer(): free inact timer
 * @timer - inact timer handle
 *
 * Return: bool
 */
void dp_free_inact_timer(struct dp_soc *soc)
{
	qdf_timer_free(&soc->pdev_bs_inact_timer);
}
#else

void dp_mark_peer_inact(void *peer, bool inactive)
{
	return;
}

void dp_free_inact_timer(struct dp_soc *soc)
{
	return;
}

#endif

/*
* dp_pdev_attach_wifi3() - attach txrx pdev
* @ctrl_pdev: Opaque PDEV object
* @txrx_soc: Datapath SOC handle
* @htc_handle: HTC handle for host-target interface
* @qdf_osdev: QDF OS device
* @pdev_id: PDEV ID
*
* Return: DP PDEV handle on success, NULL on failure
*/
static struct cdp_pdev *dp_pdev_attach_wifi3(struct cdp_soc_t *txrx_soc,
	struct cdp_cfg *ctrl_pdev,
	HTC_HANDLE htc_handle, qdf_device_t qdf_osdev, uint8_t pdev_id)
{
	int tx_ring_size;
	int tx_comp_ring_size;

	struct dp_soc *soc = (struct dp_soc *)txrx_soc;
	struct dp_pdev *pdev = qdf_mem_malloc(sizeof(*pdev));

	if (!pdev) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
			FL("DP PDEV memory allocation failed"));
		goto fail0;
	}

	pdev->wlan_cfg_ctx = wlan_cfg_pdev_attach();

	if (!pdev->wlan_cfg_ctx) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
			FL("pdev cfg_attach failed"));

		qdf_mem_free(pdev);
		goto fail0;
	}

	/*
	 * set nss pdev config based on soc config
	 */
	wlan_cfg_set_dp_pdev_nss_enabled(pdev->wlan_cfg_ctx,
			(wlan_cfg_get_dp_soc_nss_cfg(soc->wlan_cfg_ctx) & (1 << pdev_id)));

	pdev->soc = soc;
	pdev->osif_pdev = ctrl_pdev;
	pdev->pdev_id = pdev_id;
	soc->pdev_list[pdev_id] = pdev;
	soc->pdev_count++;

	TAILQ_INIT(&pdev->vdev_list);
	pdev->vdev_count = 0;

	qdf_spinlock_create(&pdev->tx_mutex);
	qdf_spinlock_create(&pdev->neighbour_peer_mutex);
	TAILQ_INIT(&pdev->neighbour_peers_list);

	if (dp_soc_cmn_setup(soc)) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
			FL("dp_soc_cmn_setup failed"));
		goto fail1;
	}

	/* Setup per PDEV TCL rings if configured */
	if (wlan_cfg_per_pdev_tx_ring(soc->wlan_cfg_ctx)) {
		tx_ring_size =
			wlan_cfg_tx_ring_size(soc->wlan_cfg_ctx);
		tx_comp_ring_size =
			wlan_cfg_tx_comp_ring_size(soc->wlan_cfg_ctx);

		if (dp_srng_setup(soc, &soc->tcl_data_ring[pdev_id], TCL_DATA,
			pdev_id, pdev_id, tx_ring_size)) {
			QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
				FL("dp_srng_setup failed for tcl_data_ring"));
			goto fail1;
		}
		if (dp_srng_setup(soc, &soc->tx_comp_ring[pdev_id],
			WBM2SW_RELEASE, pdev_id, pdev_id, tx_comp_ring_size)) {
			QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
				FL("dp_srng_setup failed for tx_comp_ring"));
			goto fail1;
		}
		soc->num_tcl_data_rings++;
	}

	/* Tx specific init */
	if (dp_tx_pdev_attach(pdev)) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
			FL("dp_tx_pdev_attach failed"));
		goto fail1;
	}

	/* Setup per PDEV REO rings if configured */
	if (wlan_cfg_per_pdev_rx_ring(soc->wlan_cfg_ctx)) {
		if (dp_srng_setup(soc, &soc->reo_dest_ring[pdev_id], REO_DST,
			pdev_id, pdev_id, REO_DST_RING_SIZE)) {
			QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
				FL("dp_srng_setup failed for reo_dest_ringn"));
			goto fail1;
		}
		soc->num_reo_dest_rings++;

	}
	if (dp_srng_setup(soc, &pdev->rx_refill_buf_ring, RXDMA_BUF, 0, pdev_id,
		RXDMA_REFILL_RING_SIZE)) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
			 FL("dp_srng_setup failed rx refill ring"));
		goto fail1;
	}

	if (dp_rxdma_ring_setup(soc, pdev)) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
			 FL("RXDMA ring config failed"));
		goto fail1;
	}

	if (dp_srng_setup(soc, &pdev->rxdma_mon_buf_ring, RXDMA_MONITOR_BUF, 0,
		pdev_id, RXDMA_MONITOR_BUF_RING_SIZE)) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
			FL("dp_srng_setup failed for rxdma_mon_buf_ring"));
		goto fail1;
	}

	if (dp_srng_setup(soc, &pdev->rxdma_mon_dst_ring, RXDMA_MONITOR_DST, 0,
		pdev_id, RXDMA_MONITOR_DST_RING_SIZE)) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
			FL("dp_srng_setup failed for rxdma_mon_dst_ring"));
		goto fail1;
	}


	if (dp_srng_setup(soc, &pdev->rxdma_mon_status_ring,
		RXDMA_MONITOR_STATUS, 0, pdev_id,
		RXDMA_MONITOR_STATUS_RING_SIZE)) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
			FL("dp_srng_setup failed for rxdma_mon_status_ring"));
		goto fail1;
	}

	if (dp_srng_setup(soc, &pdev->rxdma_mon_desc_ring,
		RXDMA_MONITOR_DESC, 0, pdev_id, RXDMA_MONITOR_DESC_RING_SIZE)) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
			"dp_srng_setup failed for rxdma_mon_desc_ring\n");
		goto fail1;
	}

	if (wlan_cfg_per_pdev_lmac_ring(soc->wlan_cfg_ctx)) {
		if (dp_srng_setup(soc, &pdev->rxdma_err_dst_ring[0], RXDMA_DST,
				  0, pdev_id, RXDMA_ERR_DST_RING_SIZE)) {
			QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
				FL("dp_srng_setup failed for rxdma_err_dst_ring"));
			goto fail1;
		}
	}

	/* Setup second Rx refill buffer ring */
	if (dp_srng_setup(soc, &pdev->rx_refill_buf_ring2, RXDMA_BUF, 2,
				pdev->pdev_id, RXDMA_REFILL_RING_SIZE)) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
			FL("dp_srng_setup failed second rx refill ring"));
		goto fail1;
	}

	if (dp_ipa_ring_resource_setup(soc, pdev))
		goto fail1;

	if (dp_ipa_uc_attach(soc, pdev) != QDF_STATUS_SUCCESS) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
			FL("dp_ipa_uc_attach failed"));
		goto fail1;
	}

	/* Rx specific init */
	if (dp_rx_pdev_attach(pdev)) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
			FL("dp_rx_pdev_attach failed"));
		goto fail0;
	}
	DP_STATS_INIT(pdev);

	/* Monitor filter init */
	pdev->mon_filter_mode = MON_FILTER_ALL;
	pdev->fp_mgmt_filter = FILTER_MGMT_ALL;
	pdev->fp_ctrl_filter = FILTER_CTRL_ALL;
	pdev->fp_data_filter = FILTER_DATA_ALL;
	pdev->mo_mgmt_filter = FILTER_MGMT_ALL;
	pdev->mo_ctrl_filter = FILTER_CTRL_ALL;
	pdev->mo_data_filter = FILTER_DATA_ALL;

#ifndef CONFIG_WIN
	/* MCL */
	dp_local_peer_id_pool_init(pdev);
#endif
	dp_dscp_tid_map_setup(pdev);

	/* Rx monitor mode specific init */
	if (dp_rx_pdev_mon_attach(pdev)) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
				"dp_rx_pdev_attach failed\n");
		goto fail1;
	}

	if (dp_wdi_event_attach(pdev)) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
				"dp_wdi_evet_attach failed\n");
		goto fail1;
	}

	/* set the reo destination during initialization */
	pdev->reo_dest = pdev->pdev_id + 1;

	return (struct cdp_pdev *)pdev;

fail1:
	dp_pdev_detach_wifi3((struct cdp_pdev *)pdev, 0);

fail0:
	return NULL;
}

/*
* dp_rxdma_ring_cleanup() - configure the RX DMA rings
* @soc: data path SoC handle
* @pdev: Physical device handle
*
* Return: void
*/
#ifdef QCA_HOST2FW_RXBUF_RING
static void dp_rxdma_ring_cleanup(struct dp_soc *soc,
	 struct dp_pdev *pdev)
{
	int max_mac_rings =
		 wlan_cfg_get_num_mac_rings(pdev->wlan_cfg_ctx);
	int i;

	max_mac_rings = max_mac_rings < MAX_RX_MAC_RINGS ?
				max_mac_rings : MAX_RX_MAC_RINGS;
	for (i = 0; i < MAX_RX_MAC_RINGS; i++)
		dp_srng_cleanup(soc, &pdev->rx_mac_buf_ring[i],
			 RXDMA_BUF, 1);

	qdf_timer_free(&soc->mon_reap_timer);
}
#else
static void dp_rxdma_ring_cleanup(struct dp_soc *soc,
	 struct dp_pdev *pdev)
{
}
#endif

/*
 * dp_neighbour_peers_detach() - Detach neighbour peers(nac clients)
 * @pdev: device object
 *
 * Return: void
 */
static void dp_neighbour_peers_detach(struct dp_pdev *pdev)
{
	struct dp_neighbour_peer *peer = NULL;
	struct dp_neighbour_peer *temp_peer = NULL;

	TAILQ_FOREACH_SAFE(peer, &pdev->neighbour_peers_list,
			neighbour_peer_list_elem, temp_peer) {
		/* delete this peer from the list */
		TAILQ_REMOVE(&pdev->neighbour_peers_list,
				peer, neighbour_peer_list_elem);
		qdf_mem_free(peer);
	}

	qdf_spinlock_destroy(&pdev->neighbour_peer_mutex);
}

/*
* dp_pdev_detach_wifi3() - detach txrx pdev
* @txrx_pdev: Datapath PDEV handle
* @force: Force detach
*
*/
static void dp_pdev_detach_wifi3(struct cdp_pdev *txrx_pdev, int force)
{
	struct dp_pdev *pdev = (struct dp_pdev *)txrx_pdev;
	struct dp_soc *soc = pdev->soc;
	qdf_nbuf_t curr_nbuf, next_nbuf;

	dp_wdi_event_detach(pdev);

	dp_tx_pdev_detach(pdev);

	if (wlan_cfg_per_pdev_tx_ring(soc->wlan_cfg_ctx)) {
		dp_srng_cleanup(soc, &soc->tcl_data_ring[pdev->pdev_id],
			TCL_DATA, pdev->pdev_id);
		dp_srng_cleanup(soc, &soc->tx_comp_ring[pdev->pdev_id],
			WBM2SW_RELEASE, pdev->pdev_id);
	}

	dp_pktlogmod_exit(pdev);

	dp_rx_pdev_detach(pdev);

	dp_rx_pdev_mon_detach(pdev);

	dp_neighbour_peers_detach(pdev);
	qdf_spinlock_destroy(&pdev->tx_mutex);

	dp_ipa_uc_detach(soc, pdev);

	dp_srng_cleanup(soc, &pdev->rx_refill_buf_ring2, RXDMA_BUF, 2);

	/* Cleanup per PDEV REO rings if configured */
	if (wlan_cfg_per_pdev_rx_ring(soc->wlan_cfg_ctx)) {
		dp_srng_cleanup(soc, &soc->reo_dest_ring[pdev->pdev_id],
			REO_DST, pdev->pdev_id);
	}

	dp_srng_cleanup(soc, &pdev->rx_refill_buf_ring, RXDMA_BUF, 0);

	dp_rxdma_ring_cleanup(soc, pdev);

	dp_srng_cleanup(soc, &pdev->rxdma_mon_buf_ring, RXDMA_MONITOR_BUF, 0);

	dp_srng_cleanup(soc, &pdev->rxdma_mon_dst_ring, RXDMA_MONITOR_DST, 0);

	dp_srng_cleanup(soc, &pdev->rxdma_mon_status_ring,
		RXDMA_MONITOR_STATUS, 0);

	dp_srng_cleanup(soc, &pdev->rxdma_mon_desc_ring,
		RXDMA_MONITOR_DESC, 0);

	if (wlan_cfg_per_pdev_lmac_ring(soc->wlan_cfg_ctx)) {
		dp_srng_cleanup(soc, &pdev->rxdma_err_dst_ring[0], RXDMA_DST, 0);
	} else {
		int i;

		for (i = 0; i < MAX_RX_MAC_RINGS; i++)
			dp_srng_cleanup(soc, &pdev->rxdma_err_dst_ring[i],
				RXDMA_DST, 0);
	}

	curr_nbuf = pdev->invalid_peer_head_msdu;
	while (curr_nbuf) {
		next_nbuf = qdf_nbuf_next(curr_nbuf);
		qdf_nbuf_free(curr_nbuf);
		curr_nbuf = next_nbuf;
	}

	soc->pdev_list[pdev->pdev_id] = NULL;
	soc->pdev_count--;
	wlan_cfg_pdev_detach(pdev->wlan_cfg_ctx);
	qdf_mem_free(pdev->dp_txrx_handle);
	qdf_mem_free(pdev);
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

/*
 * dp_soc_detach_wifi3() - Detach txrx SOC
 * @txrx_soc: DP SOC handle, struct cdp_soc_t is first element of struct dp_soc.
 */
static void dp_soc_detach_wifi3(void *txrx_soc)
{
	struct dp_soc *soc = (struct dp_soc *)txrx_soc;
	int i;

	qdf_atomic_set(&soc->cmn_init_done, 0);

	qdf_flush_work(&soc->htt_stats.work);
	qdf_disable_work(&soc->htt_stats.work);

	/* Free pending htt stats messages */
	qdf_nbuf_queue_free(&soc->htt_stats.msg);

	dp_free_inact_timer(soc);

	for (i = 0; i < MAX_PDEV_CNT; i++) {
		if (soc->pdev_list[i])
			dp_pdev_detach_wifi3(
				(struct cdp_pdev *)soc->pdev_list[i], 1);
	}

	dp_peer_find_detach(soc);

	/* TBD: Call Tx and Rx cleanup functions to free buffers and
	 * SW descriptors
	 */

	/* Free the ring memories */
	/* Common rings */
	dp_srng_cleanup(soc, &soc->wbm_desc_rel_ring, SW2WBM_RELEASE, 0);

	dp_tx_soc_detach(soc);
	/* Tx data rings */
	if (!wlan_cfg_per_pdev_tx_ring(soc->wlan_cfg_ctx)) {
		for (i = 0; i < soc->num_tcl_data_rings; i++) {
			dp_srng_cleanup(soc, &soc->tcl_data_ring[i],
				TCL_DATA, i);
			dp_srng_cleanup(soc, &soc->tx_comp_ring[i],
				WBM2SW_RELEASE, i);
		}
	}

	/* TCL command and status rings */
	dp_srng_cleanup(soc, &soc->tcl_cmd_ring, TCL_CMD, 0);
	dp_srng_cleanup(soc, &soc->tcl_status_ring, TCL_STATUS, 0);

	/* Rx data rings */
	if (!wlan_cfg_per_pdev_rx_ring(soc->wlan_cfg_ctx)) {
		soc->num_reo_dest_rings =
			wlan_cfg_num_reo_dest_rings(soc->wlan_cfg_ctx);
		for (i = 0; i < soc->num_reo_dest_rings; i++) {
			/* TODO: Get number of rings and ring sizes
			 * from wlan_cfg
			 */
			dp_srng_cleanup(soc, &soc->reo_dest_ring[i],
				REO_DST, i);
		}
	}
	/* REO reinjection ring */
	dp_srng_cleanup(soc, &soc->reo_reinject_ring, REO_REINJECT, 0);

	/* Rx release ring */
	dp_srng_cleanup(soc, &soc->rx_rel_ring, WBM2SW_RELEASE, 0);

	/* Rx exception ring */
	/* TODO: Better to store ring_type and ring_num in
	 * dp_srng during setup
	 */
	dp_srng_cleanup(soc, &soc->reo_exception_ring, REO_EXCEPTION, 0);

	/* REO command and status rings */
	dp_srng_cleanup(soc, &soc->reo_cmd_ring, REO_CMD, 0);
	dp_srng_cleanup(soc, &soc->reo_status_ring, REO_STATUS, 0);
	dp_hw_link_desc_pool_cleanup(soc);

	qdf_spinlock_destroy(&soc->peer_ref_mutex);
	qdf_spinlock_destroy(&soc->htt_stats.lock);

	htt_soc_detach(soc->htt_handle);

	dp_reo_cmdlist_destroy(soc);
	qdf_spinlock_destroy(&soc->rx.reo_cmd_lock);
	dp_reo_desc_freelist_destroy(soc);

	wlan_cfg_soc_detach(soc->wlan_cfg_ctx);

	dp_soc_wds_detach(soc);
	qdf_spinlock_destroy(&soc->ast_lock);

	qdf_mem_free(soc);
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
 * Return: void
 */
#ifdef QCA_HOST2FW_RXBUF_RING
static void dp_rxdma_ring_config(struct dp_soc *soc)
{
	int i;

	for (i = 0; i < MAX_PDEV_CNT; i++) {
		struct dp_pdev *pdev = soc->pdev_list[i];

		if (pdev) {
			int mac_id = 0;
			int j;
			bool dbs_enable = 0;
			int max_mac_rings =
				 wlan_cfg_get_num_mac_rings
				(pdev->wlan_cfg_ctx);

			htt_srng_setup(soc->htt_handle, 0,
				 pdev->rx_refill_buf_ring.hal_srng,
				 RXDMA_BUF);

			if (pdev->rx_refill_buf_ring2.hal_srng)
				htt_srng_setup(soc->htt_handle, 0,
					pdev->rx_refill_buf_ring2.hal_srng,
					RXDMA_BUF);

			if (soc->cdp_soc.ol_ops->
				is_hw_dbs_2x2_capable) {
				dbs_enable = soc->cdp_soc.ol_ops->
					is_hw_dbs_2x2_capable(soc->ctrl_psoc);
			}

			if (dbs_enable) {
				QDF_TRACE(QDF_MODULE_ID_TXRX,
				QDF_TRACE_LEVEL_ERROR,
				FL("DBS enabled max_mac_rings %d\n"),
					 max_mac_rings);
			} else {
				max_mac_rings = 1;
				QDF_TRACE(QDF_MODULE_ID_TXRX,
					 QDF_TRACE_LEVEL_ERROR,
					 FL("DBS disabled, max_mac_rings %d\n"),
					 max_mac_rings);
			}

			QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
					 FL("pdev_id %d max_mac_rings %d\n"),
					 pdev->pdev_id, max_mac_rings);

			for (j = 0; j < max_mac_rings; j++) {
				QDF_TRACE(QDF_MODULE_ID_TXRX,
					 QDF_TRACE_LEVEL_ERROR,
					 FL("mac_id %d\n"), mac_id);
				htt_srng_setup(soc->htt_handle, mac_id,
					 pdev->rx_mac_buf_ring[j]
						.hal_srng,
					 RXDMA_BUF);
				htt_srng_setup(soc->htt_handle, mac_id,
					pdev->rxdma_err_dst_ring[j]
						.hal_srng,
					RXDMA_DST);
				mac_id++;
			}

			/* Configure monitor mode rings */
			htt_srng_setup(soc->htt_handle, i,
					pdev->rxdma_mon_buf_ring.hal_srng,
					RXDMA_MONITOR_BUF);

			htt_srng_setup(soc->htt_handle, i,
					pdev->rxdma_mon_dst_ring.hal_srng,
					RXDMA_MONITOR_DST);

			htt_srng_setup(soc->htt_handle, i,
				pdev->rxdma_mon_status_ring.hal_srng,
				RXDMA_MONITOR_STATUS);

			htt_srng_setup(soc->htt_handle, i,
				pdev->rxdma_mon_desc_ring.hal_srng,
				RXDMA_MONITOR_DESC);
		}
	}

	/*
	 * Timer to reap rxdma status rings.
	 * Needed until we enable ppdu end interrupts
	 */
	qdf_timer_init(soc->osdev, &soc->mon_reap_timer,
			dp_service_mon_rings, (void *)soc,
			QDF_TIMER_TYPE_WAKE_APPS);
	soc->reap_timer_init = 1;
}
#else
static void dp_rxdma_ring_config(struct dp_soc *soc)
{
	int i;

	for (i = 0; i < MAX_PDEV_CNT; i++) {
		struct dp_pdev *pdev = soc->pdev_list[i];

		if (pdev) {
			int ring_idx = dp_get_ring_id_for_mac_id(soc, i);

			htt_srng_setup(soc->htt_handle, i,
				pdev->rx_refill_buf_ring.hal_srng, RXDMA_BUF);

			htt_srng_setup(soc->htt_handle, i,
					pdev->rxdma_mon_buf_ring.hal_srng,
					RXDMA_MONITOR_BUF);
			htt_srng_setup(soc->htt_handle, i,
					pdev->rxdma_mon_dst_ring.hal_srng,
					RXDMA_MONITOR_DST);
			htt_srng_setup(soc->htt_handle, i,
				pdev->rxdma_mon_status_ring.hal_srng,
				RXDMA_MONITOR_STATUS);
			htt_srng_setup(soc->htt_handle, i,
				pdev->rxdma_mon_desc_ring.hal_srng,
				RXDMA_MONITOR_DESC);
			htt_srng_setup(soc->htt_handle, i,
				pdev->rxdma_err_dst_ring[ring_idx].hal_srng,
				RXDMA_DST);
		}
	}
}
#endif

/*
 * dp_soc_attach_target_wifi3() - SOC initialization in the target
 * @txrx_soc: Datapath SOC handle
 */
static int dp_soc_attach_target_wifi3(struct cdp_soc_t *cdp_soc)
{
	struct dp_soc *soc = (struct dp_soc *)cdp_soc;

	htt_soc_attach_target(soc->htt_handle);

	dp_rxdma_ring_config(soc);

	DP_STATS_INIT(soc);

	/* initialize work queue for stats processing */
	qdf_create_work(0, &soc->htt_stats.work, htt_t2h_stats_handler, soc);

	return 0;
}

/*
 * dp_soc_get_nss_cfg_wifi3() - SOC get nss config
 * @txrx_soc: Datapath SOC handle
 */
static int dp_soc_get_nss_cfg_wifi3(struct cdp_soc_t *cdp_soc)
{
	struct dp_soc *dsoc = (struct dp_soc *)cdp_soc;
	return wlan_cfg_get_dp_soc_nss_cfg(dsoc->wlan_cfg_ctx);
}
/*
 * dp_soc_set_nss_cfg_wifi3() - SOC set nss config
 * @txrx_soc: Datapath SOC handle
 * @nss_cfg: nss config
 */
static void dp_soc_set_nss_cfg_wifi3(struct cdp_soc_t *cdp_soc, int config)
{
	struct dp_soc *dsoc = (struct dp_soc *)cdp_soc;
	struct wlan_cfg_dp_soc_ctxt *wlan_cfg_ctx = dsoc->wlan_cfg_ctx;

	wlan_cfg_set_dp_soc_nss_cfg(wlan_cfg_ctx, config);

	/*
	 * TODO: masked out based on the per offloaded radio
	 */
	if (config == dp_nss_cfg_dbdc) {
		wlan_cfg_set_num_tx_desc_pool(wlan_cfg_ctx, 0);
		wlan_cfg_set_num_tx_ext_desc_pool(wlan_cfg_ctx, 0);
		wlan_cfg_set_num_tx_desc(wlan_cfg_ctx, 0);
		wlan_cfg_set_num_tx_ext_desc(wlan_cfg_ctx, 0);
	}

	QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
				FL("nss-wifi<0> nss config is enabled"));
}
/*
* dp_vdev_attach_wifi3() - attach txrx vdev
* @txrx_pdev: Datapath PDEV handle
* @vdev_mac_addr: MAC address of the virtual interface
* @vdev_id: VDEV Id
* @wlan_op_mode: VDEV operating mode
*
* Return: DP VDEV handle on success, NULL on failure
*/
static struct cdp_vdev *dp_vdev_attach_wifi3(struct cdp_pdev *txrx_pdev,
	uint8_t *vdev_mac_addr, uint8_t vdev_id, enum wlan_op_mode op_mode)
{
	struct dp_pdev *pdev = (struct dp_pdev *)txrx_pdev;
	struct dp_soc *soc = pdev->soc;
	struct dp_vdev *vdev = qdf_mem_malloc(sizeof(*vdev));
	int tx_ring_size;

	if (!vdev) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
			FL("DP VDEV memory allocation failed"));
		goto fail0;
	}

	vdev->pdev = pdev;
	vdev->vdev_id = vdev_id;
	vdev->opmode = op_mode;
	vdev->osdev = soc->osdev;

	vdev->osif_rx = NULL;
	vdev->osif_rsim_rx_decap = NULL;
	vdev->osif_get_key = NULL;
	vdev->osif_rx_mon = NULL;
	vdev->osif_tx_free_ext = NULL;
	vdev->osif_vdev = NULL;

	vdev->delete.pending = 0;
	vdev->safemode = 0;
	vdev->drop_unenc = 1;
	vdev->sec_type = cdp_sec_type_none;
#ifdef notyet
	vdev->filters_num = 0;
#endif

	qdf_mem_copy(
		&vdev->mac_addr.raw[0], vdev_mac_addr, OL_TXRX_MAC_ADDR_LEN);

	vdev->tx_encap_type = wlan_cfg_pkt_type(soc->wlan_cfg_ctx);
	vdev->rx_decap_type = wlan_cfg_pkt_type(soc->wlan_cfg_ctx);
	vdev->dscp_tid_map_id = 0;
	vdev->mcast_enhancement_en = 0;
	tx_ring_size = wlan_cfg_tx_ring_size(soc->wlan_cfg_ctx);

	/* TODO: Initialize default HTT meta data that will be used in
	 * TCL descriptors for packets transmitted from this VDEV
	 */

	TAILQ_INIT(&vdev->peer_list);

	/* add this vdev into the pdev's list */
	TAILQ_INSERT_TAIL(&pdev->vdev_list, vdev, vdev_list_elem);
	pdev->vdev_count++;

	dp_tx_vdev_attach(vdev);

	if (QDF_STATUS_SUCCESS != dp_tx_flow_pool_map_handler(pdev, vdev_id,
					FLOW_TYPE_VDEV, vdev_id, tx_ring_size))
		goto fail1;


	if ((soc->intr_mode == DP_INTR_POLL) &&
			wlan_cfg_get_num_contexts(soc->wlan_cfg_ctx) != 0) {
		if (pdev->vdev_count == 1)
			qdf_timer_mod(&soc->int_timer, DP_INTR_POLL_TIMER_MS);
	}

	dp_lro_hash_setup(soc);

	/* LRO */
	if (wlan_cfg_is_lro_enabled(soc->wlan_cfg_ctx) &&
		wlan_op_mode_sta == vdev->opmode)
		vdev->lro_enable = true;

	QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
		 "LRO: vdev_id %d lro_enable %d", vdev_id, vdev->lro_enable);

	QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
		"Created vdev %pK (%pM)", vdev, vdev->mac_addr.raw);
	DP_STATS_INIT(vdev);

	if (wlan_op_mode_sta == vdev->opmode)
		dp_peer_create_wifi3((struct cdp_vdev *)vdev,
							vdev->mac_addr.raw);

	return (struct cdp_vdev *)vdev;

fail1:
	dp_tx_vdev_detach(vdev);
	qdf_mem_free(vdev);
fail0:
	return NULL;
}

/**
 * dp_vdev_register_wifi3() - Register VDEV operations from osif layer
 * @vdev: Datapath VDEV handle
 * @osif_vdev: OSIF vdev handle
 * @txrx_ops: Tx and Rx operations
 *
 * Return: DP VDEV handle on success, NULL on failure
 */
static void dp_vdev_register_wifi3(struct cdp_vdev *vdev_handle,
	void *osif_vdev,
	struct ol_txrx_ops *txrx_ops)
{
	struct dp_vdev *vdev = (struct dp_vdev *)vdev_handle;
	vdev->osif_vdev = osif_vdev;
	vdev->osif_rx = txrx_ops->rx.rx;
	vdev->osif_rsim_rx_decap = txrx_ops->rx.rsim_rx_decap;
	vdev->osif_get_key = txrx_ops->get_key;
	vdev->osif_rx_mon = txrx_ops->rx.mon;
	vdev->osif_tx_free_ext = txrx_ops->tx.tx_free_ext;
#ifdef notyet
#if ATH_SUPPORT_WAPI
	vdev->osif_check_wai = txrx_ops->rx.wai_check;
#endif
#endif
#ifdef UMAC_SUPPORT_PROXY_ARP
	vdev->osif_proxy_arp = txrx_ops->proxy_arp;
#endif
	vdev->me_convert = txrx_ops->me_convert;

	/* TODO: Enable the following once Tx code is integrated */
	if (vdev->mesh_vdev)
		txrx_ops->tx.tx = dp_tx_send_mesh;
	else
		txrx_ops->tx.tx = dp_tx_send;

	txrx_ops->tx.tx_exception = dp_tx_send_exception;

	QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_INFO_LOW,
		"DP Vdev Register success");
}

/*
 * dp_vdev_detach_wifi3() - Detach txrx vdev
 * @txrx_vdev:		Datapath VDEV handle
 * @callback:		Callback OL_IF on completion of detach
 * @cb_context:	Callback context
 *
 */
static void dp_vdev_detach_wifi3(struct cdp_vdev *vdev_handle,
	ol_txrx_vdev_delete_cb callback, void *cb_context)
{
	struct dp_vdev *vdev = (struct dp_vdev *)vdev_handle;
	struct dp_pdev *pdev = vdev->pdev;
	struct dp_soc *soc = pdev->soc;

	/* preconditions */
	qdf_assert(vdev);

	/* remove the vdev from its parent pdev's list */
	TAILQ_REMOVE(&pdev->vdev_list, vdev, vdev_list_elem);

	if (wlan_op_mode_sta == vdev->opmode)
		dp_peer_delete_wifi3(vdev->vap_bss_peer, 0);

	/*
	 * Use peer_ref_mutex while accessing peer_list, in case
	 * a peer is in the process of being removed from the list.
	 */
	qdf_spin_lock_bh(&soc->peer_ref_mutex);
	/* check that the vdev has no peers allocated */
	if (!TAILQ_EMPTY(&vdev->peer_list)) {
		/* debug print - will be removed later */
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_WARN,
			FL("not deleting vdev object %pK (%pM)"
			"until deletion finishes for all its peers"),
			vdev, vdev->mac_addr.raw);
		/* indicate that the vdev needs to be deleted */
		vdev->delete.pending = 1;
		vdev->delete.callback = callback;
		vdev->delete.context = cb_context;
		qdf_spin_unlock_bh(&soc->peer_ref_mutex);
		return;
	}
	qdf_spin_unlock_bh(&soc->peer_ref_mutex);

	dp_tx_flow_pool_unmap_handler(pdev, vdev->vdev_id, FLOW_TYPE_VDEV,
		vdev->vdev_id);
	dp_tx_vdev_detach(vdev);
	QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_INFO_HIGH,
		FL("deleting vdev object %pK (%pM)"), vdev, vdev->mac_addr.raw);

	qdf_mem_free(vdev);

	if (callback)
		callback(cb_context);
}

/*
 * dp_peer_create_wifi3() - attach txrx peer
 * @txrx_vdev: Datapath VDEV handle
 * @peer_mac_addr: Peer MAC address
 *
 * Return: DP peeer handle on success, NULL on failure
 */
static void *dp_peer_create_wifi3(struct cdp_vdev *vdev_handle,
		uint8_t *peer_mac_addr)
{
	struct dp_peer *peer;
	int i;
	struct dp_vdev *vdev = (struct dp_vdev *)vdev_handle;
	struct dp_pdev *pdev;
	struct dp_soc *soc;

	/* preconditions */
	qdf_assert(vdev);
	qdf_assert(peer_mac_addr);

	pdev = vdev->pdev;
	soc = pdev->soc;
#ifdef notyet
	peer = (struct dp_peer *)qdf_mempool_alloc(soc->osdev,
		soc->mempool_ol_ath_peer);
#else
	peer = (struct dp_peer *)qdf_mem_malloc(sizeof(*peer));
#endif

	if (!peer)
		return NULL; /* failure */

	qdf_mem_zero(peer, sizeof(struct dp_peer));

	TAILQ_INIT(&peer->ast_entry_list);

	/* store provided params */
	peer->vdev = vdev;

	dp_peer_add_ast(soc, peer, peer_mac_addr, CDP_TXRX_AST_TYPE_STATIC, 0);

	qdf_spinlock_create(&peer->peer_info_lock);

	qdf_mem_copy(
		&peer->mac_addr.raw[0], peer_mac_addr, OL_TXRX_MAC_ADDR_LEN);

	/* TODO: See of rx_opt_proc is really required */
	peer->rx_opt_proc = soc->rx_opt_proc;

	/* initialize the peer_id */
	for (i = 0; i < MAX_NUM_PEER_ID_PER_PEER; i++)
		peer->peer_ids[i] = HTT_INVALID_PEER;

	qdf_spin_lock_bh(&soc->peer_ref_mutex);

	qdf_atomic_init(&peer->ref_cnt);

	/* keep one reference for attach */
	qdf_atomic_inc(&peer->ref_cnt);

	/* add this peer into the vdev's list */
	if (wlan_op_mode_sta == vdev->opmode)
		TAILQ_INSERT_HEAD(&vdev->peer_list, peer, peer_list_elem);
	else
		TAILQ_INSERT_TAIL(&vdev->peer_list, peer, peer_list_elem);

	qdf_spin_unlock_bh(&soc->peer_ref_mutex);

	/* TODO: See if hash based search is required */
	dp_peer_find_hash_add(soc, peer);

	/* Initialize the peer state */
	peer->state = OL_TXRX_PEER_STATE_DISC;

	QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_INFO_HIGH,
		"vdev %pK created peer %pK (%pM) ref_cnt: %d",
		vdev, peer, peer->mac_addr.raw,
		qdf_atomic_read(&peer->ref_cnt));
	/*
	 * For every peer MAp message search and set if bss_peer
	 */
	if (memcmp(peer->mac_addr.raw, vdev->mac_addr.raw, 6) == 0) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_INFO_HIGH,
			"vdev bss_peer!!!!");
		peer->bss_peer = 1;
		vdev->vap_bss_peer = peer;
	}


#ifndef CONFIG_WIN
	dp_local_peer_id_alloc(pdev, peer);
#endif
	DP_STATS_INIT(peer);
	return (void *)peer;
}

/*
 * dp_peer_setup_wifi3() - initialize the peer
 * @vdev_hdl: virtual device object
 * @peer: Peer object
 *
 * Return: void
 */
static void dp_peer_setup_wifi3(struct cdp_vdev *vdev_hdl, void *peer_hdl)
{
	struct dp_peer *peer = (struct dp_peer *)peer_hdl;
	struct dp_vdev *vdev = (struct dp_vdev *)vdev_hdl;
	struct dp_pdev *pdev;
	struct dp_soc *soc;
	bool hash_based = 0;
	enum cdp_host_reo_dest_ring reo_dest;

	/* preconditions */
	qdf_assert(vdev);
	qdf_assert(peer);

	pdev = vdev->pdev;
	soc = pdev->soc;

	peer->last_assoc_rcvd = 0;
	peer->last_disassoc_rcvd = 0;
	peer->last_deauth_rcvd = 0;

	/*
	 * hash based steering is disabled for Radios which are offloaded
	 * to NSS
	 */
	if (!wlan_cfg_get_dp_pdev_nss_enabled(pdev->wlan_cfg_ctx))
		hash_based = wlan_cfg_is_rx_hash_enabled(soc->wlan_cfg_ctx);

	QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_INFO,
		FL("hash based steering for pdev: %d is %d\n"),
		pdev->pdev_id, hash_based);

	/*
	 * Below line of code will ensure the proper reo_dest ring is choosen
	 * for cases where toeplitz hash cannot be generated (ex: non TCP/UDP)
	 */
	reo_dest = pdev->reo_dest;

	if (soc->cdp_soc.ol_ops->peer_set_default_routing) {
		/* TODO: Check the destination ring number to be passed to FW */
		soc->cdp_soc.ol_ops->peer_set_default_routing(
			pdev->osif_pdev, peer->mac_addr.raw,
			 peer->vdev->vdev_id, hash_based, reo_dest);
	}

	dp_peer_rx_init(pdev, peer);
	return;
}

/*
 * dp_set_vdev_tx_encap_type() - set the encap type of the vdev
 * @vdev_handle: virtual device object
 * @htt_pkt_type: type of pkt
 *
 * Return: void
 */
static void dp_set_vdev_tx_encap_type(struct cdp_vdev *vdev_handle,
	 enum htt_cmn_pkt_type val)
{
	struct dp_vdev *vdev = (struct dp_vdev *)vdev_handle;
	vdev->tx_encap_type = val;
}

/*
 * dp_set_vdev_rx_decap_type() - set the decap type of the vdev
 * @vdev_handle: virtual device object
 * @htt_pkt_type: type of pkt
 *
 * Return: void
 */
static void dp_set_vdev_rx_decap_type(struct cdp_vdev *vdev_handle,
	 enum htt_cmn_pkt_type val)
{
	struct dp_vdev *vdev = (struct dp_vdev *)vdev_handle;
	vdev->rx_decap_type = val;
}

/*
 * dp_set_pdev_reo_dest() - set the reo destination ring for this pdev
 * @pdev_handle: physical device object
 * @val: reo destination ring index (1 - 4)
 *
 * Return: void
 */
static void dp_set_pdev_reo_dest(struct cdp_pdev *pdev_handle,
	 enum cdp_host_reo_dest_ring val)
{
	struct dp_pdev *pdev = (struct dp_pdev *)pdev_handle;

	if (pdev)
		pdev->reo_dest = val;
}

/*
 * dp_get_pdev_reo_dest() - get the reo destination for this pdev
 * @pdev_handle: physical device object
 *
 * Return: reo destination ring index
 */
static enum cdp_host_reo_dest_ring
dp_get_pdev_reo_dest(struct cdp_pdev *pdev_handle)
{
	struct dp_pdev *pdev = (struct dp_pdev *)pdev_handle;

	if (pdev)
		return pdev->reo_dest;
	else
		return cdp_host_reo_dest_ring_unknown;
}

#ifdef QCA_SUPPORT_SON
static void dp_son_peer_authorize(struct dp_peer *peer)
{
	struct dp_soc *soc;
	soc = peer->vdev->pdev->soc;
	peer->peer_bs_inact_flag = 0;
	peer->peer_bs_inact = soc->pdev_bs_inact_reload;
	return;
}
#else
static void dp_son_peer_authorize(struct dp_peer *peer)
{
	return;
}
#endif
/*
 * dp_set_filter_neighbour_peers() - set filter neighbour peers for smart mesh
 * @pdev_handle: device object
 * @val: value to be set
 *
 * Return: void
 */
static int dp_set_filter_neighbour_peers(struct cdp_pdev *pdev_handle,
	 uint32_t val)
{
	struct dp_pdev *pdev = (struct dp_pdev *)pdev_handle;

	/* Enable/Disable smart mesh filtering. This flag will be checked
	 * during rx processing to check if packets are from NAC clients.
	 */
	pdev->filter_neighbour_peers = val;
	return 0;
}

/*
 * dp_update_filter_neighbour_peers() - set neighbour peers(nac clients)
 * address for smart mesh filtering
 * @pdev_handle: device object
 * @cmd: Add/Del command
 * @macaddr: nac client mac address
 *
 * Return: void
 */
static int dp_update_filter_neighbour_peers(struct cdp_pdev *pdev_handle,
	 uint32_t cmd, uint8_t *macaddr)
{
	struct dp_pdev *pdev = (struct dp_pdev *)pdev_handle;
	struct dp_neighbour_peer *peer = NULL;

	if (!macaddr)
		goto fail0;

	/* Store address of NAC (neighbour peer) which will be checked
	 * against TA of received packets.
	 */
	if (cmd == DP_NAC_PARAM_ADD) {
		peer = (struct dp_neighbour_peer *) qdf_mem_malloc(
				sizeof(*peer));

		if (!peer) {
			QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
				FL("DP neighbour peer node memory allocation failed"));
			goto fail0;
		}

		qdf_mem_copy(&peer->neighbour_peers_macaddr.raw[0],
			macaddr, DP_MAC_ADDR_LEN);


		qdf_spin_lock_bh(&pdev->neighbour_peer_mutex);
		/* add this neighbour peer into the list */
		TAILQ_INSERT_TAIL(&pdev->neighbour_peers_list, peer,
				neighbour_peer_list_elem);
		qdf_spin_unlock_bh(&pdev->neighbour_peer_mutex);

		return 1;

	} else if (cmd == DP_NAC_PARAM_DEL) {
		qdf_spin_lock_bh(&pdev->neighbour_peer_mutex);
		TAILQ_FOREACH(peer, &pdev->neighbour_peers_list,
				neighbour_peer_list_elem) {
			if (!qdf_mem_cmp(&peer->neighbour_peers_macaddr.raw[0],
				macaddr, DP_MAC_ADDR_LEN)) {
				/* delete this peer from the list */
				TAILQ_REMOVE(&pdev->neighbour_peers_list,
					peer, neighbour_peer_list_elem);
				qdf_mem_free(peer);
				break;
			}
		}
		qdf_spin_unlock_bh(&pdev->neighbour_peer_mutex);

		return 1;

	}

fail0:
	return 0;
}

/*
 * dp_get_sec_type() - Get the security type
 * @peer:		Datapath peer handle
 * @sec_idx:    Security id (mcast, ucast)
 *
 * return sec_type: Security type
 */
static int dp_get_sec_type(struct cdp_peer *peer, uint8_t sec_idx)
{
	struct dp_peer *dpeer = (struct dp_peer *)peer;

	return dpeer->security[sec_idx].sec_type;
}

/*
 * dp_peer_authorize() - authorize txrx peer
 * @peer_handle:		Datapath peer handle
 * @authorize
 *
 */
static void dp_peer_authorize(struct cdp_peer *peer_handle, uint32_t authorize)
{
	struct dp_peer *peer = (struct dp_peer *)peer_handle;
	struct dp_soc *soc;

	if (peer != NULL) {
		soc = peer->vdev->pdev->soc;
		qdf_spin_lock_bh(&soc->peer_ref_mutex);
		dp_son_peer_authorize(peer);
		peer->authorize = authorize ? 1 : 0;
		qdf_spin_unlock_bh(&soc->peer_ref_mutex);
	}
}

#ifdef QCA_SUPPORT_SON
/*
 * dp_txrx_update_inact_threshold() - Update inact timer threshold
 * @pdev_handle: Device handle
 * @new_threshold : updated threshold value
 *
 */
static void
dp_txrx_update_inact_threshold(struct cdp_pdev *pdev_handle,
			       u_int16_t new_threshold)
{
	struct dp_vdev *vdev;
	struct dp_peer *peer;
	struct dp_pdev *pdev = (struct dp_pdev *)pdev_handle;
	struct dp_soc *soc = pdev->soc;
	u_int16_t old_threshold = soc->pdev_bs_inact_reload;

	if (old_threshold == new_threshold)
		return;

	soc->pdev_bs_inact_reload = new_threshold;

	qdf_spin_lock_bh(&soc->peer_ref_mutex);
	TAILQ_FOREACH(vdev, &pdev->vdev_list, vdev_list_elem) {
		if (vdev->opmode != wlan_op_mode_ap)
			continue;

		TAILQ_FOREACH(peer, &vdev->peer_list, peer_list_elem) {
			if (!peer->authorize)
				continue;

			if (old_threshold - peer->peer_bs_inact >=
					new_threshold) {
				dp_mark_peer_inact((void *)peer, true);
				peer->peer_bs_inact = 0;
			} else {
				peer->peer_bs_inact = new_threshold -
					(old_threshold - peer->peer_bs_inact);
			}
		}
	}
	qdf_spin_unlock_bh(&soc->peer_ref_mutex);
}

/**
 * dp_txrx_reset_inact_count(): Reset inact count
 * @pdev_handle - device handle
 *
 * Return: void
 */
static void
dp_txrx_reset_inact_count(struct cdp_pdev *pdev_handle)
{
	struct dp_vdev *vdev = NULL;
	struct dp_peer *peer = NULL;
	struct dp_pdev *pdev = (struct dp_pdev *)pdev_handle;
	struct dp_soc *soc = pdev->soc;

	qdf_spin_lock_bh(&soc->peer_ref_mutex);
	TAILQ_FOREACH(vdev, &pdev->vdev_list, vdev_list_elem) {
		if (vdev->opmode != wlan_op_mode_ap)
			continue;

		TAILQ_FOREACH(peer, &vdev->peer_list, peer_list_elem) {
			if (!peer->authorize)
				continue;

			peer->peer_bs_inact = soc->pdev_bs_inact_reload;
		}
	}
	qdf_spin_unlock_bh(&soc->peer_ref_mutex);
}

/**
 * dp_set_inact_params(): set inactivity params
 * @pdev_handle - device handle
 * @inact_check_interval - inactivity interval
 * @inact_normal - Inactivity normal
 * @inact_overload - Inactivity overload
 *
 * Return: bool
 */
bool dp_set_inact_params(struct cdp_pdev *pdev_handle,
			 u_int16_t inact_check_interval,
			 u_int16_t inact_normal, u_int16_t inact_overload)
{
	struct dp_soc *soc;
	struct dp_pdev *pdev = (struct dp_pdev *)pdev_handle;

	if (!pdev)
		return false;

	soc = pdev->soc;
	if (!soc)
		return false;

	soc->pdev_bs_inact_interval = inact_check_interval;
	soc->pdev_bs_inact_normal = inact_normal;
	soc->pdev_bs_inact_overload = inact_overload;

	dp_txrx_update_inact_threshold((struct cdp_pdev *)pdev,
					soc->pdev_bs_inact_normal);

	return true;
}

/**
 * dp_start_inact_timer(): Inactivity timer start
 * @pdev_handle - device handle
 * @enable - Inactivity timer start/stop
 *
 * Return: bool
 */
bool dp_start_inact_timer(struct cdp_pdev *pdev_handle, bool enable)
{
	struct dp_soc *soc;
	struct dp_pdev *pdev = (struct dp_pdev *)pdev_handle;

	if (!pdev)
		return false;

	soc = pdev->soc;
	if (!soc)
		return false;

	if (enable) {
		dp_txrx_reset_inact_count((struct cdp_pdev *)pdev);
		qdf_timer_mod(&soc->pdev_bs_inact_timer,
			      soc->pdev_bs_inact_interval * 1000);
	} else {
		qdf_timer_stop(&soc->pdev_bs_inact_timer);
	}

	return true;
}

/**
 * dp_set_overload(): Set inactivity overload
 * @pdev_handle - device handle
 * @overload - overload status
 *
 * Return: void
 */
void dp_set_overload(struct cdp_pdev *pdev_handle, bool overload)
{
	struct dp_soc *soc;
	struct dp_pdev *pdev = (struct dp_pdev *)pdev_handle;

	if (!pdev)
		return;

	soc = pdev->soc;
	if (!soc)
		return;

	dp_txrx_update_inact_threshold((struct cdp_pdev *)pdev,
			overload ? soc->pdev_bs_inact_overload :
			soc->pdev_bs_inact_normal);
}

/**
 * dp_peer_is_inact(): check whether peer is inactive
 * @peer_handle - datapath peer handle
 *
 * Return: bool
 */
bool dp_peer_is_inact(void *peer_handle)
{
	struct dp_peer *peer = (struct dp_peer *)peer_handle;

	if (!peer)
		return false;

	return peer->peer_bs_inact_flag == 1;
}

/**
 * dp_init_inact_timer: initialize the inact timer
 * @soc - SOC handle
 *
 * Return: void
 */
void dp_init_inact_timer(struct dp_soc *soc)
{
	qdf_timer_init(soc->osdev, &soc->pdev_bs_inact_timer,
		dp_txrx_peer_find_inact_timeout_handler,
		(void *)soc, QDF_TIMER_TYPE_WAKE_APPS);
}

#else

bool dp_set_inact_params(struct cdp_pdev *pdev, u_int16_t inact_check_interval,
			 u_int16_t inact_normal, u_int16_t inact_overload)
{
	return false;
}

bool dp_start_inact_timer(struct cdp_pdev *pdev, bool enable)
{
	return false;
}

void dp_set_overload(struct cdp_pdev *pdev, bool overload)
{
	return;
}

void dp_init_inact_timer(struct dp_soc *soc)
{
	return;
}

bool dp_peer_is_inact(void *peer)
{
	return false;
}
#endif

/*
 * dp_peer_unref_delete() - unref and delete peer
 * @peer_handle:		Datapath peer handle
 *
 */
void dp_peer_unref_delete(void *peer_handle)
{
	struct dp_peer *peer = (struct dp_peer *)peer_handle;
	struct dp_peer *bss_peer = NULL;
	struct dp_vdev *vdev = peer->vdev;
	struct dp_pdev *pdev = vdev->pdev;
	struct dp_soc *soc = pdev->soc;
	struct dp_peer *tmppeer;
	int found = 0;
	uint16_t peer_id;
	uint16_t vdev_id;

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
	qdf_spin_lock_bh(&soc->peer_ref_mutex);
	QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
		  "%s: peer %pK ref_cnt(before decrement): %d\n", __func__,
		  peer, qdf_atomic_read(&peer->ref_cnt));
	if (qdf_atomic_dec_and_test(&peer->ref_cnt)) {
		peer_id = peer->peer_ids[0];
		vdev_id = vdev->vdev_id;

		/*
		 * Make sure that the reference to the peer in
		 * peer object map is removed
		 */
		if (peer_id != HTT_INVALID_PEER)
			soc->peer_id_to_obj_map[peer_id] = NULL;

		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_INFO_HIGH,
			"Deleting peer %pK (%pM)", peer, peer->mac_addr.raw);

		/* remove the reference to the peer from the hash table */
		dp_peer_find_hash_remove(soc, peer);

		TAILQ_FOREACH(tmppeer, &peer->vdev->peer_list, peer_list_elem) {
			if (tmppeer == peer) {
				found = 1;
				break;
			}
		}
		if (found) {
			TAILQ_REMOVE(&peer->vdev->peer_list, peer,
				peer_list_elem);
		} else {
			/*Ignoring the remove operation as peer not found*/
			QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_WARN,
				"peer %pK not found in vdev (%pK)->peer_list:%pK",
				peer, vdev, &peer->vdev->peer_list);
		}

		/* cleanup the peer data */
		dp_peer_cleanup(vdev, peer);

		/* check whether the parent vdev has no peers left */
		if (TAILQ_EMPTY(&vdev->peer_list)) {
			/*
			 * Now that there are no references to the peer, we can
			 * release the peer reference lock.
			 */
			qdf_spin_unlock_bh(&soc->peer_ref_mutex);
			/*
			 * Check if the parent vdev was waiting for its peers
			 * to be deleted, in order for it to be deleted too.
			 */
			if (vdev->delete.pending) {
				ol_txrx_vdev_delete_cb vdev_delete_cb =
					vdev->delete.callback;
				void *vdev_delete_context =
					vdev->delete.context;

				QDF_TRACE(QDF_MODULE_ID_DP,
					QDF_TRACE_LEVEL_INFO_HIGH,
					FL("deleting vdev object %pK (%pM)"
					" - its last peer is done"),
					vdev, vdev->mac_addr.raw);
				/* all peers are gone, go ahead and delete it */
				dp_tx_flow_pool_unmap_handler(pdev, vdev_id,
								FLOW_TYPE_VDEV,
								vdev_id);
				dp_tx_vdev_detach(vdev);
				QDF_TRACE(QDF_MODULE_ID_DP,
					QDF_TRACE_LEVEL_INFO_HIGH,
					FL("deleting vdev object %pK (%pM)"),
					vdev, vdev->mac_addr.raw);

				qdf_mem_free(vdev);
				vdev = NULL;
				if (vdev_delete_cb)
					vdev_delete_cb(vdev_delete_context);
			}
		} else {
			qdf_spin_unlock_bh(&soc->peer_ref_mutex);
		}

		if (vdev) {
			if (vdev->vap_bss_peer == peer) {
				vdev->vap_bss_peer = NULL;
			}
		}

		if (soc->cdp_soc.ol_ops->peer_unref_delete) {
			soc->cdp_soc.ol_ops->peer_unref_delete(pdev->osif_pdev,
					vdev_id, peer->mac_addr.raw);
		}

		if (!vdev || !vdev->vap_bss_peer) {
			goto free_peer;
		}

#ifdef notyet
		qdf_mempool_free(soc->osdev, soc->mempool_ol_ath_peer, peer);
#else
		bss_peer = vdev->vap_bss_peer;
		DP_UPDATE_STATS(bss_peer, peer);

free_peer:
		qdf_mem_free(peer);

#endif
	} else {
		qdf_spin_unlock_bh(&soc->peer_ref_mutex);
	}
}

/*
 * dp_peer_detach_wifi3() – Detach txrx peer
 * @peer_handle: Datapath peer handle
 * @bitmap: bitmap indicating special handling of request.
 *
 */
static void dp_peer_delete_wifi3(void *peer_handle, uint32_t bitmap)
{
	struct dp_peer *peer = (struct dp_peer *)peer_handle;

	/* redirect the peer's rx delivery function to point to a
	 * discard func
	 */
	peer->rx_opt_proc = dp_rx_discard;

	QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_INFO_HIGH,
		FL("peer %pK (%pM)"),  peer, peer->mac_addr.raw);

#ifndef CONFIG_WIN
	dp_local_peer_id_free(peer->vdev->pdev, peer);
#endif
	qdf_spinlock_destroy(&peer->peer_info_lock);

	/*
	 * Remove the reference added during peer_attach.
	 * The peer will still be left allocated until the
	 * PEER_UNMAP message arrives to remove the other
	 * reference, added by the PEER_MAP message.
	 */
	dp_peer_unref_delete(peer_handle);
}

/*
 * dp_get_vdev_mac_addr_wifi3() – Detach txrx peer
 * @peer_handle:		Datapath peer handle
 *
 */
static uint8 *dp_get_vdev_mac_addr_wifi3(struct cdp_vdev *pvdev)
{
	struct dp_vdev *vdev = (struct dp_vdev *)pvdev;
	return vdev->mac_addr.raw;
}

/*
 * dp_vdev_set_wds() - Enable per packet stats
 * @vdev_handle: DP VDEV handle
 * @val: value
 *
 * Return: none
 */
static int dp_vdev_set_wds(void *vdev_handle, uint32_t val)
{
	struct dp_vdev *vdev = (struct dp_vdev *)vdev_handle;

	vdev->wds_enabled = val;
	return 0;
}

/*
 * dp_get_vdev_from_vdev_id_wifi3() – Detach txrx peer
 * @peer_handle:		Datapath peer handle
 *
 */
static struct cdp_vdev *dp_get_vdev_from_vdev_id_wifi3(struct cdp_pdev *dev,
						uint8_t vdev_id)
{
	struct dp_pdev *pdev = (struct dp_pdev *)dev;
	struct dp_vdev *vdev = NULL;

	if (qdf_unlikely(!pdev))
		return NULL;

	TAILQ_FOREACH(vdev, &pdev->vdev_list, vdev_list_elem) {
		if (vdev->vdev_id == vdev_id)
			break;
	}

	return (struct cdp_vdev *)vdev;
}

static int dp_get_opmode(struct cdp_vdev *vdev_handle)
{
	struct dp_vdev *vdev = (struct dp_vdev *)vdev_handle;

	return vdev->opmode;
}

static struct cdp_cfg *dp_get_ctrl_pdev_from_vdev_wifi3(struct cdp_vdev *pvdev)
{
	struct dp_vdev *vdev = (struct dp_vdev *)pvdev;
	struct dp_pdev *pdev = vdev->pdev;

	return (struct cdp_cfg *)pdev->wlan_cfg_ctx;
}

/**
 * dp_reset_monitor_mode() - Disable monitor mode
 * @pdev_handle: Datapath PDEV handle
 *
 * Return: 0 on success, not 0 on failure
 */
static int dp_reset_monitor_mode(struct cdp_pdev *pdev_handle)
{
	struct dp_pdev *pdev = (struct dp_pdev *)pdev_handle;
	struct htt_rx_ring_tlv_filter htt_tlv_filter;
	struct dp_soc *soc;
	uint8_t pdev_id;

	pdev_id = pdev->pdev_id;
	soc = pdev->soc;

	pdev->monitor_vdev = NULL;
	qdf_mem_set(&(htt_tlv_filter), sizeof(htt_tlv_filter), 0x0);

	htt_h2t_rx_ring_cfg(soc->htt_handle, pdev_id,
		pdev->rxdma_mon_buf_ring.hal_srng,
		RXDMA_MONITOR_BUF, RX_BUFFER_SIZE, &htt_tlv_filter);

	htt_h2t_rx_ring_cfg(soc->htt_handle, pdev_id,
		pdev->rxdma_mon_status_ring.hal_srng, RXDMA_MONITOR_STATUS,
		RX_BUFFER_SIZE, &htt_tlv_filter);

	return 0;
}

/**
 * dp_set_nac() - set peer_nac
 * @peer_handle: Datapath PEER handle
 *
 * Return: void
 */
static void dp_set_nac(struct cdp_peer *peer_handle)
{
	struct dp_peer *peer = (struct dp_peer *)peer_handle;

	peer->nac = 1;
}

/**
 * dp_get_tx_pending() - read pending tx
 * @pdev_handle: Datapath PDEV handle
 *
 * Return: outstanding tx
 */
static int dp_get_tx_pending(struct cdp_pdev *pdev_handle)
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
 * Return: void
 */
static void dp_get_peer_mac_from_peer_id(struct cdp_pdev *pdev_handle,
	uint32_t peer_id, uint8_t *peer_mac)
{
	struct dp_pdev *pdev = (struct dp_pdev *)pdev_handle;
	struct dp_peer *peer;

	if (pdev && peer_mac) {
		peer = dp_peer_find_by_id(pdev->soc, (uint16_t)peer_id);
		if (peer && peer->mac_addr.raw) {
			qdf_mem_copy(peer_mac, peer->mac_addr.raw,
					DP_MAC_ADDR_LEN);
		}
	}
}

/**
 * dp_vdev_set_monitor_mode() - Set DP VDEV to monitor mode
 * @vdev_handle: Datapath VDEV handle
 * @smart_monitor: Flag to denote if its smart monitor mode
 *
 * Return: 0 on success, not 0 on failure
 */
static int dp_vdev_set_monitor_mode(struct cdp_vdev *vdev_handle,
		uint8_t smart_monitor)
{
	/* Many monitor VAPs can exists in a system but only one can be up at
	 * anytime
	 */
	struct dp_vdev *vdev = (struct dp_vdev *)vdev_handle;
	struct dp_pdev *pdev;
	struct htt_rx_ring_tlv_filter htt_tlv_filter;
	struct dp_soc *soc;
	uint8_t pdev_id;

	qdf_assert(vdev);

	pdev = vdev->pdev;
	pdev_id = pdev->pdev_id;
	soc = pdev->soc;

	QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_WARN,
		"pdev=%pK, pdev_id=%d, soc=%pK vdev=%pK\n",
		pdev, pdev_id, soc, vdev);

	/*Check if current pdev's monitor_vdev exists */
	if (pdev->monitor_vdev) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
			"vdev=%pK\n", vdev);
		qdf_assert(vdev);
	}

	pdev->monitor_vdev = vdev;

	/* If smart monitor mode, do not configure monitor ring */
	if (smart_monitor)
		return QDF_STATUS_SUCCESS;

	QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_INFO_HIGH,
		"MODE[%x] FP[%02x|%02x|%02x] MO[%02x|%02x|%02x]\n",
		pdev->mon_filter_mode, pdev->fp_mgmt_filter,
		pdev->fp_ctrl_filter, pdev->fp_data_filter,
		pdev->mo_mgmt_filter, pdev->mo_ctrl_filter,
		pdev->mo_data_filter);

	htt_tlv_filter.mpdu_start = 1;
	htt_tlv_filter.msdu_start = 1;
	htt_tlv_filter.packet = 1;
	htt_tlv_filter.msdu_end = 1;
	htt_tlv_filter.mpdu_end = 1;
	htt_tlv_filter.packet_header = 1;
	htt_tlv_filter.attention = 1;
	htt_tlv_filter.ppdu_start = 0;
	htt_tlv_filter.ppdu_end = 0;
	htt_tlv_filter.ppdu_end_user_stats = 0;
	htt_tlv_filter.ppdu_end_user_stats_ext = 0;
	htt_tlv_filter.ppdu_end_status_done = 0;
	htt_tlv_filter.header_per_msdu = 1;
	htt_tlv_filter.enable_fp =
		(pdev->mon_filter_mode & MON_FILTER_PASS) ? 1 : 0;
	htt_tlv_filter.enable_md = 0;
	htt_tlv_filter.enable_mo =
		(pdev->mon_filter_mode & MON_FILTER_OTHER) ? 1 : 0;
	htt_tlv_filter.fp_mgmt_filter = pdev->fp_mgmt_filter;
	htt_tlv_filter.fp_ctrl_filter = pdev->fp_ctrl_filter;
	htt_tlv_filter.fp_data_filter = pdev->fp_data_filter;
	htt_tlv_filter.mo_mgmt_filter = pdev->mo_mgmt_filter;
	htt_tlv_filter.mo_ctrl_filter = pdev->mo_ctrl_filter;
	htt_tlv_filter.mo_data_filter = pdev->mo_data_filter;

	htt_h2t_rx_ring_cfg(soc->htt_handle, pdev_id,
		pdev->rxdma_mon_buf_ring.hal_srng,
		RXDMA_MONITOR_BUF, RX_BUFFER_SIZE, &htt_tlv_filter);

	htt_tlv_filter.mpdu_start = 1;
	htt_tlv_filter.msdu_start = 1;
	htt_tlv_filter.packet = 0;
	htt_tlv_filter.msdu_end = 1;
	htt_tlv_filter.mpdu_end = 1;
	htt_tlv_filter.packet_header = 1;
	htt_tlv_filter.attention = 1;
	htt_tlv_filter.ppdu_start = 1;
	htt_tlv_filter.ppdu_end = 1;
	htt_tlv_filter.ppdu_end_user_stats = 1;
	htt_tlv_filter.ppdu_end_user_stats_ext = 1;
	htt_tlv_filter.ppdu_end_status_done = 1;
	htt_tlv_filter.header_per_msdu = 0;
	htt_tlv_filter.enable_fp =
		(pdev->mon_filter_mode & MON_FILTER_PASS) ? 1 : 0;
	htt_tlv_filter.enable_md = 0;
	htt_tlv_filter.enable_mo =
		(pdev->mon_filter_mode & MON_FILTER_OTHER) ? 1 : 0;
	htt_tlv_filter.fp_mgmt_filter = pdev->fp_mgmt_filter;
	htt_tlv_filter.fp_ctrl_filter = pdev->fp_ctrl_filter;
	htt_tlv_filter.fp_data_filter = pdev->fp_data_filter;
	htt_tlv_filter.mo_mgmt_filter = pdev->mo_mgmt_filter;
	htt_tlv_filter.mo_ctrl_filter = pdev->mo_ctrl_filter;
	htt_tlv_filter.mo_data_filter = pdev->mo_data_filter;

	htt_h2t_rx_ring_cfg(soc->htt_handle, pdev_id,
		pdev->rxdma_mon_status_ring.hal_srng, RXDMA_MONITOR_STATUS,
		RX_BUFFER_SIZE, &htt_tlv_filter);

	return QDF_STATUS_SUCCESS;
}

/**
 * dp_pdev_set_advance_monitor_filter() - Set DP PDEV monitor filter
 * @pdev_handle: Datapath PDEV handle
 * @filter_val: Flag to select Filter for monitor mode
 * Return: 0 on success, not 0 on failure
 */
static int dp_pdev_set_advance_monitor_filter(struct cdp_pdev *pdev_handle,
	struct cdp_monitor_filter *filter_val)
{
	/* Many monitor VAPs can exists in a system but only one can be up at
	 * anytime
	 */
	struct dp_pdev *pdev = (struct dp_pdev *)pdev_handle;
	struct dp_vdev *vdev = pdev->monitor_vdev;
	struct htt_rx_ring_tlv_filter htt_tlv_filter;
	struct dp_soc *soc;
	uint8_t pdev_id;

	pdev_id = pdev->pdev_id;
	soc = pdev->soc;

	QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_WARN,
		"pdev=%pK, pdev_id=%d, soc=%pK vdev=%pK\n",
		pdev, pdev_id, soc, vdev);

	/*Check if current pdev's monitor_vdev exists */
	if (!pdev->monitor_vdev) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
			"vdev=%pK\n", vdev);
		qdf_assert(vdev);
	}

	/* update filter mode, type in pdev structure */
	pdev->mon_filter_mode = filter_val->mode;
	pdev->fp_mgmt_filter = filter_val->fp_mgmt;
	pdev->fp_ctrl_filter = filter_val->fp_ctrl;
	pdev->fp_data_filter = filter_val->fp_data;
	pdev->mo_mgmt_filter = filter_val->mo_mgmt;
	pdev->mo_ctrl_filter = filter_val->mo_ctrl;
	pdev->mo_data_filter = filter_val->mo_data;

	QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_INFO_HIGH,
		"MODE[%x] FP[%02x|%02x|%02x] MO[%02x|%02x|%02x]\n",
		pdev->mon_filter_mode, pdev->fp_mgmt_filter,
		pdev->fp_ctrl_filter, pdev->fp_data_filter,
		pdev->mo_mgmt_filter, pdev->mo_ctrl_filter,
		pdev->mo_data_filter);

	qdf_mem_set(&(htt_tlv_filter), sizeof(htt_tlv_filter), 0x0);

	htt_h2t_rx_ring_cfg(soc->htt_handle, pdev_id,
		pdev->rxdma_mon_buf_ring.hal_srng,
		RXDMA_MONITOR_BUF, RX_BUFFER_SIZE, &htt_tlv_filter);

	htt_h2t_rx_ring_cfg(soc->htt_handle, pdev_id,
		pdev->rxdma_mon_status_ring.hal_srng, RXDMA_MONITOR_STATUS,
		RX_BUFFER_SIZE, &htt_tlv_filter);

	htt_tlv_filter.mpdu_start = 1;
	htt_tlv_filter.msdu_start = 1;
	htt_tlv_filter.packet = 1;
	htt_tlv_filter.msdu_end = 1;
	htt_tlv_filter.mpdu_end = 1;
	htt_tlv_filter.packet_header = 1;
	htt_tlv_filter.attention = 1;
	htt_tlv_filter.ppdu_start = 0;
	htt_tlv_filter.ppdu_end = 0;
	htt_tlv_filter.ppdu_end_user_stats = 0;
	htt_tlv_filter.ppdu_end_user_stats_ext = 0;
	htt_tlv_filter.ppdu_end_status_done = 0;
	htt_tlv_filter.header_per_msdu = 1;
	htt_tlv_filter.enable_fp =
		(pdev->mon_filter_mode & MON_FILTER_PASS) ? 1 : 0;
	htt_tlv_filter.enable_md = 0;
	htt_tlv_filter.enable_mo =
		(pdev->mon_filter_mode & MON_FILTER_OTHER) ? 1 : 0;
	htt_tlv_filter.fp_mgmt_filter = pdev->fp_mgmt_filter;
	htt_tlv_filter.fp_ctrl_filter = pdev->fp_ctrl_filter;
	htt_tlv_filter.fp_data_filter = pdev->fp_data_filter;
	htt_tlv_filter.mo_mgmt_filter = pdev->mo_mgmt_filter;
	htt_tlv_filter.mo_ctrl_filter = pdev->mo_ctrl_filter;
	htt_tlv_filter.mo_data_filter = pdev->mo_data_filter;

	htt_h2t_rx_ring_cfg(soc->htt_handle, pdev_id,
		pdev->rxdma_mon_buf_ring.hal_srng, RXDMA_MONITOR_BUF,
		RX_BUFFER_SIZE, &htt_tlv_filter);

	htt_tlv_filter.mpdu_start = 1;
	htt_tlv_filter.msdu_start = 1;
	htt_tlv_filter.packet = 0;
	htt_tlv_filter.msdu_end = 1;
	htt_tlv_filter.mpdu_end = 1;
	htt_tlv_filter.packet_header = 1;
	htt_tlv_filter.attention = 1;
	htt_tlv_filter.ppdu_start = 1;
	htt_tlv_filter.ppdu_end = 1;
	htt_tlv_filter.ppdu_end_user_stats = 1;
	htt_tlv_filter.ppdu_end_user_stats_ext = 1;
	htt_tlv_filter.ppdu_end_status_done = 1;
	htt_tlv_filter.header_per_msdu = 0;
	htt_tlv_filter.enable_fp =
		(pdev->mon_filter_mode & MON_FILTER_PASS) ? 1 : 0;
	htt_tlv_filter.enable_md = 0;
	htt_tlv_filter.enable_mo =
		(pdev->mon_filter_mode & MON_FILTER_OTHER) ? 1 : 0;
	htt_tlv_filter.fp_mgmt_filter = pdev->fp_mgmt_filter;
	htt_tlv_filter.fp_ctrl_filter = pdev->fp_ctrl_filter;
	htt_tlv_filter.fp_data_filter = pdev->fp_data_filter;
	htt_tlv_filter.mo_mgmt_filter = pdev->mo_mgmt_filter;
	htt_tlv_filter.mo_ctrl_filter = pdev->mo_ctrl_filter;
	htt_tlv_filter.mo_data_filter = pdev->mo_data_filter;

	htt_h2t_rx_ring_cfg(soc->htt_handle, pdev_id,
		pdev->rxdma_mon_status_ring.hal_srng, RXDMA_MONITOR_STATUS,
		RX_BUFFER_SIZE, &htt_tlv_filter);

	return QDF_STATUS_SUCCESS;
}

/**
 * dp_get_pdev_id_frm_pdev() - get pdev_id
 * @pdev_handle: Datapath PDEV handle
 *
 * Return: pdev_id
 */
static
uint8_t dp_get_pdev_id_frm_pdev(struct cdp_pdev *pdev_handle)
{
	struct dp_pdev *pdev = (struct dp_pdev *)pdev_handle;

	return pdev->pdev_id;
}

/**
 * dp_vdev_get_filter_ucast_data() - get DP VDEV monitor ucast filter
 * @vdev_handle: Datapath VDEV handle
 * Return: true on ucast filter flag set
 */
static bool dp_vdev_get_filter_ucast_data(struct cdp_vdev *vdev_handle)
{
	struct dp_vdev *vdev = (struct dp_vdev *)vdev_handle;
	struct dp_pdev *pdev;

	pdev = vdev->pdev;

	if ((pdev->fp_data_filter & FILTER_DATA_UCAST) ||
	    (pdev->mo_data_filter & FILTER_DATA_UCAST))
		return true;

	return false;
}

/**
 * dp_vdev_get_filter_mcast_data() - get DP VDEV monitor mcast filter
 * @vdev_handle: Datapath VDEV handle
 * Return: true on mcast filter flag set
 */
static bool dp_vdev_get_filter_mcast_data(struct cdp_vdev *vdev_handle)
{
	struct dp_vdev *vdev = (struct dp_vdev *)vdev_handle;
	struct dp_pdev *pdev;

	pdev = vdev->pdev;

	if ((pdev->fp_data_filter & FILTER_DATA_MCAST) ||
	    (pdev->mo_data_filter & FILTER_DATA_MCAST))
		return true;

	return false;
}

/**
 * dp_vdev_get_filter_non_data() - get DP VDEV monitor non_data filter
 * @vdev_handle: Datapath VDEV handle
 * Return: true on non data filter flag set
 */
static bool dp_vdev_get_filter_non_data(struct cdp_vdev *vdev_handle)
{
	struct dp_vdev *vdev = (struct dp_vdev *)vdev_handle;
	struct dp_pdev *pdev;

	pdev = vdev->pdev;

	if ((pdev->fp_mgmt_filter & FILTER_MGMT_ALL) ||
	    (pdev->mo_mgmt_filter & FILTER_MGMT_ALL)) {
		if ((pdev->fp_ctrl_filter & FILTER_CTRL_ALL) ||
		    (pdev->mo_ctrl_filter & FILTER_CTRL_ALL)) {
			return true;
		}
	}

	return false;
}

#ifdef MESH_MODE_SUPPORT
void dp_peer_set_mesh_mode(struct cdp_vdev *vdev_hdl, uint32_t val)
{
	struct dp_vdev *vdev = (struct dp_vdev *)vdev_hdl;

	QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_INFO,
		FL("val %d"), val);
	vdev->mesh_vdev = val;
}

/*
 * dp_peer_set_mesh_rx_filter() - to set the mesh rx filter
 * @vdev_hdl: virtual device object
 * @val: value to be set
 *
 * Return: void
 */
void dp_peer_set_mesh_rx_filter(struct cdp_vdev *vdev_hdl, uint32_t val)
{
	struct dp_vdev *vdev = (struct dp_vdev *)vdev_hdl;

	QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_INFO,
		FL("val %d"), val);
	vdev->mesh_rx_filter = val;
}
#endif

/*
 * dp_aggregate_pdev_ctrl_frames_stats()- function to agreegate peer stats
 * Current scope is bar recieved count
 *
 * @pdev_handle: DP_PDEV handle
 *
 * Return: void
 */
#define STATS_PROC_TIMEOUT        (HZ/1000)

static void
dp_aggregate_pdev_ctrl_frames_stats(struct dp_pdev *pdev)
{
	struct dp_vdev *vdev;
	struct dp_peer *peer;
	uint32_t waitcnt;

	TAILQ_FOREACH(vdev, &pdev->vdev_list, vdev_list_elem) {
		TAILQ_FOREACH(peer, &vdev->peer_list, peer_list_elem) {
			if (!peer) {
				QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
					FL("DP Invalid Peer refernce"));
				return;
			}

			if (peer->delete_in_progress) {
				QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
					FL("DP Peer deletion in progress"));
				continue;
			}

			qdf_atomic_inc(&peer->ref_cnt);
			waitcnt = 0;
			dp_peer_rxtid_stats(peer, dp_rx_bar_stats_cb, pdev);
			while (!(qdf_atomic_read(&(pdev->stats_cmd_complete)))
				&& waitcnt < 10) {
				schedule_timeout_interruptible(
						STATS_PROC_TIMEOUT);
				waitcnt++;
			}
			qdf_atomic_set(&(pdev->stats_cmd_complete), 0);
			dp_peer_unref_delete(peer);
		}
	}
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

	if (queue_status->header.status != HAL_REO_CMD_SUCCESS) {
		DP_TRACE_STATS(FATAL, "REO stats failure %d \n",
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
void dp_aggregate_vdev_stats(struct dp_vdev *vdev)
{
	struct dp_peer *peer = NULL;
	struct dp_soc *soc = vdev->pdev->soc;

	qdf_mem_set(&(vdev->stats.tx), sizeof(vdev->stats.tx), 0x0);
	qdf_mem_set(&(vdev->stats.rx), sizeof(vdev->stats.rx), 0x0);

	TAILQ_FOREACH(peer, &vdev->peer_list, peer_list_elem)
		DP_UPDATE_STATS(vdev, peer);

	if (soc->cdp_soc.ol_ops->update_dp_stats)
		soc->cdp_soc.ol_ops->update_dp_stats(vdev->pdev->osif_pdev,
			&vdev->stats, (uint16_t) vdev->vdev_id,
			UPDATE_VDEV_STATS);

}

/**
 * dp_aggregate_pdev_stats(): Consolidate stats at PDEV level
 * @pdev: DP PDEV handle
 *
 * return: void
 */
static inline void dp_aggregate_pdev_stats(struct dp_pdev *pdev)
{
	struct dp_vdev *vdev = NULL;
	struct dp_soc *soc = pdev->soc;

	qdf_mem_set(&(pdev->stats.tx), sizeof(pdev->stats.tx), 0x0);
	qdf_mem_set(&(pdev->stats.rx), sizeof(pdev->stats.rx), 0x0);
	qdf_mem_set(&(pdev->stats.tx_i), sizeof(pdev->stats.tx_i), 0x0);

	TAILQ_FOREACH(vdev, &pdev->vdev_list, vdev_list_elem) {

		dp_aggregate_vdev_stats(vdev);
		DP_UPDATE_STATS(pdev, vdev);

		DP_STATS_AGGR_PKT(pdev, vdev, tx_i.nawds_mcast);

		DP_STATS_AGGR_PKT(pdev, vdev, tx_i.rcvd);
		DP_STATS_AGGR_PKT(pdev, vdev, tx_i.processed);
		DP_STATS_AGGR_PKT(pdev, vdev, tx_i.reinject_pkts);
		DP_STATS_AGGR_PKT(pdev, vdev, tx_i.inspect_pkts);
		DP_STATS_AGGR_PKT(pdev, vdev, tx_i.raw.raw_pkt);
		DP_STATS_AGGR(pdev, vdev, tx_i.raw.dma_map_error);
		DP_STATS_AGGR_PKT(pdev, vdev, tx_i.tso.tso_pkt);
		DP_STATS_AGGR(pdev, vdev, tx_i.tso.dropped_host);
		DP_STATS_AGGR(pdev, vdev, tx_i.tso.dropped_target);
		DP_STATS_AGGR(pdev, vdev, tx_i.sg.dropped_host);
		DP_STATS_AGGR(pdev, vdev, tx_i.sg.dropped_target);
		DP_STATS_AGGR_PKT(pdev, vdev, tx_i.sg.sg_pkt);
		DP_STATS_AGGR_PKT(pdev, vdev, tx_i.mcast_en.mcast_pkt);
		DP_STATS_AGGR(pdev, vdev,
				tx_i.mcast_en.dropped_map_error);
		DP_STATS_AGGR(pdev, vdev,
				tx_i.mcast_en.dropped_self_mac);
		DP_STATS_AGGR(pdev, vdev,
				tx_i.mcast_en.dropped_send_fail);
		DP_STATS_AGGR(pdev, vdev, tx_i.mcast_en.ucast);
		DP_STATS_AGGR(pdev, vdev, tx_i.dropped.dma_error);
		DP_STATS_AGGR(pdev, vdev, tx_i.dropped.ring_full);
		DP_STATS_AGGR(pdev, vdev, tx_i.dropped.enqueue_fail);
		DP_STATS_AGGR(pdev, vdev, tx_i.dropped.desc_na);
		DP_STATS_AGGR(pdev, vdev, tx_i.dropped.res_full);
		DP_STATS_AGGR(pdev, vdev, tx_i.cce_classified);
		DP_STATS_AGGR(pdev, vdev, tx_i.cce_classified_raw);
		DP_STATS_AGGR(pdev, vdev, tx_i.mesh.exception_fw);
		DP_STATS_AGGR(pdev, vdev, tx_i.mesh.completion_fw);

		pdev->stats.tx_i.dropped.dropped_pkt.num =
			pdev->stats.tx_i.dropped.dma_error +
			pdev->stats.tx_i.dropped.ring_full +
			pdev->stats.tx_i.dropped.enqueue_fail +
			pdev->stats.tx_i.dropped.desc_na +
			pdev->stats.tx_i.dropped.res_full;

		pdev->stats.tx.last_ack_rssi =
			vdev->stats.tx.last_ack_rssi;
		pdev->stats.tx_i.tso.num_seg =
			vdev->stats.tx_i.tso.num_seg;
	}
	if (soc->cdp_soc.ol_ops->update_dp_stats)
		soc->cdp_soc.ol_ops->update_dp_stats(pdev->osif_pdev,
				&pdev->stats, pdev->pdev_id, UPDATE_PDEV_STATS);

}

/**
 * dp_print_pdev_tx_stats(): Print Pdev level TX stats
 * @pdev: DP_PDEV Handle
 *
 * Return:void
 */
static inline void
dp_print_pdev_tx_stats(struct dp_pdev *pdev)
{
	uint8_t index = 0;
	DP_PRINT_STATS("PDEV Tx Stats:\n");
	DP_PRINT_STATS("Received From Stack:");
	DP_PRINT_STATS("	Packets = %d",
			pdev->stats.tx_i.rcvd.num);
	DP_PRINT_STATS("	Bytes = %llu",
			pdev->stats.tx_i.rcvd.bytes);
	DP_PRINT_STATS("Processed:");
	DP_PRINT_STATS("	Packets = %d",
			pdev->stats.tx_i.processed.num);
	DP_PRINT_STATS("	Bytes = %llu",
			pdev->stats.tx_i.processed.bytes);
	DP_PRINT_STATS("Total Completions:");
	DP_PRINT_STATS("	Packets = %u",
			pdev->stats.tx.comp_pkt.num);
	DP_PRINT_STATS("	Bytes = %llu",
			pdev->stats.tx.comp_pkt.bytes);
	DP_PRINT_STATS("Successful Completions:");
	DP_PRINT_STATS("	Packets = %u",
			pdev->stats.tx.tx_success.num);
	DP_PRINT_STATS("	Bytes = %llu",
			pdev->stats.tx.tx_success.bytes);
	DP_PRINT_STATS("Dropped:");
	DP_PRINT_STATS("	Total = %d",
			pdev->stats.tx_i.dropped.dropped_pkt.num);
	DP_PRINT_STATS("	Dma_map_error = %d",
			pdev->stats.tx_i.dropped.dma_error);
	DP_PRINT_STATS("	Ring Full = %d",
			pdev->stats.tx_i.dropped.ring_full);
	DP_PRINT_STATS("	Descriptor Not available = %d",
			pdev->stats.tx_i.dropped.desc_na);
	DP_PRINT_STATS("	HW enqueue failed= %d",
			pdev->stats.tx_i.dropped.enqueue_fail);
	DP_PRINT_STATS("	Resources Full = %d",
			pdev->stats.tx_i.dropped.res_full);
	DP_PRINT_STATS("	FW removed = %d",
			pdev->stats.tx.dropped.fw_rem);
	DP_PRINT_STATS("	FW removed transmitted = %d",
			pdev->stats.tx.dropped.fw_rem_tx);
	DP_PRINT_STATS("	FW removed untransmitted = %d",
			pdev->stats.tx.dropped.fw_rem_notx);
	DP_PRINT_STATS("	FW removed untransmitted fw_reason1 = %d",
			pdev->stats.tx.dropped.fw_reason1);
	DP_PRINT_STATS("	FW removed untransmitted fw_reason2 = %d",
			pdev->stats.tx.dropped.fw_reason2);
	DP_PRINT_STATS("	FW removed untransmitted fw_reason3 = %d",
			pdev->stats.tx.dropped.fw_reason3);
	DP_PRINT_STATS("	Aged Out from msdu/mpdu queues = %d",
			pdev->stats.tx.dropped.age_out);
	DP_PRINT_STATS("Scatter Gather:");
	DP_PRINT_STATS("	Packets = %d",
			pdev->stats.tx_i.sg.sg_pkt.num);
	DP_PRINT_STATS("	Bytes = %llu",
			pdev->stats.tx_i.sg.sg_pkt.bytes);
	DP_PRINT_STATS("	Dropped By Host = %d",
			pdev->stats.tx_i.sg.dropped_host);
	DP_PRINT_STATS("	Dropped By Target = %d",
			pdev->stats.tx_i.sg.dropped_target);
	DP_PRINT_STATS("TSO:");
	DP_PRINT_STATS("	Number of Segments = %d",
			pdev->stats.tx_i.tso.num_seg);
	DP_PRINT_STATS("	Packets = %d",
			pdev->stats.tx_i.tso.tso_pkt.num);
	DP_PRINT_STATS("	Bytes = %llu",
			pdev->stats.tx_i.tso.tso_pkt.bytes);
	DP_PRINT_STATS("	Dropped By Host = %d",
			pdev->stats.tx_i.tso.dropped_host);
	DP_PRINT_STATS("Mcast Enhancement:");
	DP_PRINT_STATS("	Packets = %d",
			pdev->stats.tx_i.mcast_en.mcast_pkt.num);
	DP_PRINT_STATS("	Bytes = %llu",
			pdev->stats.tx_i.mcast_en.mcast_pkt.bytes);
	DP_PRINT_STATS("	Dropped: Map Errors = %d",
			pdev->stats.tx_i.mcast_en.dropped_map_error);
	DP_PRINT_STATS("	Dropped: Self Mac = %d",
			pdev->stats.tx_i.mcast_en.dropped_self_mac);
	DP_PRINT_STATS("	Dropped: Send Fail = %d",
			pdev->stats.tx_i.mcast_en.dropped_send_fail);
	DP_PRINT_STATS("	Unicast sent = %d",
			pdev->stats.tx_i.mcast_en.ucast);
	DP_PRINT_STATS("Raw:");
	DP_PRINT_STATS("	Packets = %d",
			pdev->stats.tx_i.raw.raw_pkt.num);
	DP_PRINT_STATS("	Bytes = %llu",
			pdev->stats.tx_i.raw.raw_pkt.bytes);
	DP_PRINT_STATS("	DMA map error = %d",
			pdev->stats.tx_i.raw.dma_map_error);
	DP_PRINT_STATS("Reinjected:");
	DP_PRINT_STATS("	Packets = %d",
			pdev->stats.tx_i.reinject_pkts.num);
	DP_PRINT_STATS("Bytes = %llu\n",
				pdev->stats.tx_i.reinject_pkts.bytes);
	DP_PRINT_STATS("Inspected:");
	DP_PRINT_STATS("	Packets = %d",
			pdev->stats.tx_i.inspect_pkts.num);
	DP_PRINT_STATS("	Bytes = %llu",
			pdev->stats.tx_i.inspect_pkts.bytes);
	DP_PRINT_STATS("Nawds Multicast:");
	DP_PRINT_STATS("	Packets = %d",
			pdev->stats.tx_i.nawds_mcast.num);
	DP_PRINT_STATS("	Bytes = %llu",
			pdev->stats.tx_i.nawds_mcast.bytes);
	DP_PRINT_STATS("CCE Classified:");
	DP_PRINT_STATS("	CCE Classified Packets: %u",
			pdev->stats.tx_i.cce_classified);
	DP_PRINT_STATS("	RAW CCE Classified Packets: %u",
			pdev->stats.tx_i.cce_classified_raw);
	DP_PRINT_STATS("Mesh stats:");
	DP_PRINT_STATS("	frames to firmware: %u",
			pdev->stats.tx_i.mesh.exception_fw);
	DP_PRINT_STATS("	completions from fw: %u",
			pdev->stats.tx_i.mesh.completion_fw);
	DP_PRINT_STATS("PPDU stats counter");
	for (index = 0; index < CDP_PPDU_STATS_MAX_TAG; index++) {
		DP_PRINT_STATS("	Tag[%d] = %llu", index,
				pdev->stats.ppdu_stats_counter[index]);
	}
}

/**
 * dp_print_pdev_rx_stats(): Print Pdev level RX stats
 * @pdev: DP_PDEV Handle
 *
 * Return: void
 */
static inline void
dp_print_pdev_rx_stats(struct dp_pdev *pdev)
{
	DP_PRINT_STATS("PDEV Rx Stats:\n");
	DP_PRINT_STATS("Received From HW (Per Rx Ring):");
	DP_PRINT_STATS("	Packets = %d %d %d %d",
			pdev->stats.rx.rcvd_reo[0].num,
			pdev->stats.rx.rcvd_reo[1].num,
			pdev->stats.rx.rcvd_reo[2].num,
			pdev->stats.rx.rcvd_reo[3].num);
	DP_PRINT_STATS("	Bytes = %llu %llu %llu %llu",
			pdev->stats.rx.rcvd_reo[0].bytes,
			pdev->stats.rx.rcvd_reo[1].bytes,
			pdev->stats.rx.rcvd_reo[2].bytes,
			pdev->stats.rx.rcvd_reo[3].bytes);
	DP_PRINT_STATS("Replenished:");
	DP_PRINT_STATS("	Packets = %d",
			pdev->stats.replenish.pkts.num);
	DP_PRINT_STATS("	Bytes = %llu",
			pdev->stats.replenish.pkts.bytes);
	DP_PRINT_STATS("	Buffers Added To Freelist = %d",
			pdev->stats.buf_freelist);
	DP_PRINT_STATS("	Low threshold intr = %d",
			pdev->stats.replenish.low_thresh_intrs);
	DP_PRINT_STATS("Dropped:");
	DP_PRINT_STATS("	msdu_not_done = %d",
			pdev->stats.dropped.msdu_not_done);
	DP_PRINT_STATS("Sent To Stack:");
	DP_PRINT_STATS("	Packets = %d",
			pdev->stats.rx.to_stack.num);
	DP_PRINT_STATS("	Bytes = %llu",
			pdev->stats.rx.to_stack.bytes);
	DP_PRINT_STATS("Multicast/Broadcast:");
	DP_PRINT_STATS("	Packets = %d",
			pdev->stats.rx.multicast.num);
	DP_PRINT_STATS("	Bytes = %llu",
			pdev->stats.rx.multicast.bytes);
	DP_PRINT_STATS("Errors:");
	DP_PRINT_STATS("	Rxdma Ring Un-inititalized = %d",
			pdev->stats.replenish.rxdma_err);
	DP_PRINT_STATS("	Desc Alloc Failed: = %d",
			pdev->stats.err.desc_alloc_fail);

	/* Get bar_recv_cnt */
	dp_aggregate_pdev_ctrl_frames_stats(pdev);
	DP_PRINT_STATS("BAR Received Count: = %d",
			pdev->stats.rx.bar_recv_cnt);

}

/**
 * dp_print_soc_tx_stats(): Print SOC level  stats
 * @soc DP_SOC Handle
 *
 * Return: void
 */
static inline void
dp_print_soc_tx_stats(struct dp_soc *soc)
{
	DP_PRINT_STATS("SOC Tx Stats:\n");
	DP_PRINT_STATS("Tx Descriptors In Use = %d",
			soc->stats.tx.desc_in_use);
	DP_PRINT_STATS("Invalid peer:");
	DP_PRINT_STATS("	Packets = %d",
			soc->stats.tx.tx_invalid_peer.num);
	DP_PRINT_STATS("	Bytes = %llu",
			soc->stats.tx.tx_invalid_peer.bytes);
	DP_PRINT_STATS("Packets dropped due to TCL ring full = %d %d %d",
			soc->stats.tx.tcl_ring_full[0],
			soc->stats.tx.tcl_ring_full[1],
			soc->stats.tx.tcl_ring_full[2]);

}


/**
 * dp_print_soc_rx_stats: Print SOC level Rx stats
 * @soc: DP_SOC Handle
 *
 * Return:void
 */
static inline void
dp_print_soc_rx_stats(struct dp_soc *soc)
{
	uint32_t i;
	char reo_error[DP_REO_ERR_LENGTH];
	char rxdma_error[DP_RXDMA_ERR_LENGTH];
	uint8_t index = 0;

	DP_PRINT_STATS("SOC Rx Stats:\n");
	DP_PRINT_STATS("Errors:\n");
	DP_PRINT_STATS("Rx Decrypt Errors = %d",
			(soc->stats.rx.err.rxdma_error[HAL_RXDMA_ERR_DECRYPT] +
			soc->stats.rx.err.rxdma_error[HAL_RXDMA_ERR_TKIP_MIC]));
	DP_PRINT_STATS("Invalid RBM = %d",
			soc->stats.rx.err.invalid_rbm);
	DP_PRINT_STATS("Invalid Vdev = %d",
			soc->stats.rx.err.invalid_vdev);
	DP_PRINT_STATS("Invalid Pdev = %d",
			soc->stats.rx.err.invalid_pdev);
	DP_PRINT_STATS("Invalid Peer = %d",
			soc->stats.rx.err.rx_invalid_peer.num);
	DP_PRINT_STATS("HAL Ring Access Fail = %d",
			soc->stats.rx.err.hal_ring_access_fail);

	for (i = 0; i < HAL_RXDMA_ERR_MAX; i++) {
		index += qdf_snprint(&rxdma_error[index],
				DP_RXDMA_ERR_LENGTH - index,
				" %d", soc->stats.rx.err.rxdma_error[i]);
	}
	DP_PRINT_STATS("RXDMA Error (0-31):%s",
			rxdma_error);

	index = 0;
	for (i = 0; i < HAL_REO_ERR_MAX; i++) {
		index += qdf_snprint(&reo_error[index],
				DP_REO_ERR_LENGTH - index,
				" %d", soc->stats.rx.err.reo_error[i]);
	}
	DP_PRINT_STATS("REO Error(0-14):%s",
			reo_error);
}


/**
 * dp_print_ring_stat_from_hal(): Print hal level ring stats
 * @soc: DP_SOC handle
 * @srng: DP_SRNG handle
 * @ring_name: SRNG name
 *
 * Return: void
 */
static inline void
dp_print_ring_stat_from_hal(struct dp_soc *soc,  struct dp_srng *srng,
	char *ring_name)
{
	uint32_t tailp;
	uint32_t headp;

	if (srng->hal_srng != NULL) {
		hal_api_get_tphp(soc->hal_soc, srng->hal_srng, &tailp, &headp);
		DP_PRINT_STATS("%s : Head pointer = %d  Tail Pointer = %d\n",
				ring_name, headp, tailp);
	}
}

/**
 * dp_print_ring_stats(): Print tail and head pointer
 * @pdev: DP_PDEV handle
 *
 * Return:void
 */
static inline void
dp_print_ring_stats(struct dp_pdev *pdev)
{
	uint32_t i;
	char ring_name[STR_MAXLEN + 1];

	dp_print_ring_stat_from_hal(pdev->soc,
			&pdev->soc->reo_exception_ring,
			"Reo Exception Ring");
	dp_print_ring_stat_from_hal(pdev->soc,
			&pdev->soc->reo_reinject_ring,
			"Reo Inject Ring");
	dp_print_ring_stat_from_hal(pdev->soc,
			&pdev->soc->reo_cmd_ring,
			"Reo Command Ring");
	dp_print_ring_stat_from_hal(pdev->soc,
			&pdev->soc->reo_status_ring,
			"Reo Status Ring");
	dp_print_ring_stat_from_hal(pdev->soc,
			&pdev->soc->rx_rel_ring,
			"Rx Release ring");
	dp_print_ring_stat_from_hal(pdev->soc,
			&pdev->soc->tcl_cmd_ring,
			"Tcl command Ring");
	dp_print_ring_stat_from_hal(pdev->soc,
			&pdev->soc->tcl_status_ring,
			"Tcl Status Ring");
	dp_print_ring_stat_from_hal(pdev->soc,
			&pdev->soc->wbm_desc_rel_ring,
			"Wbm Desc Rel Ring");
	for (i = 0; i < MAX_REO_DEST_RINGS; i++) {
		snprintf(ring_name, STR_MAXLEN, "Reo Dest Ring %d", i);
		dp_print_ring_stat_from_hal(pdev->soc,
				&pdev->soc->reo_dest_ring[i],
				ring_name);
	}
	for (i = 0; i < pdev->soc->num_tcl_data_rings; i++) {
		snprintf(ring_name, STR_MAXLEN, "Tcl Data Ring %d", i);
		dp_print_ring_stat_from_hal(pdev->soc,
				&pdev->soc->tcl_data_ring[i],
				ring_name);
	}
	for (i = 0; i < MAX_TCL_DATA_RINGS; i++) {
		snprintf(ring_name, STR_MAXLEN, "Tx Comp Ring %d", i);
		dp_print_ring_stat_from_hal(pdev->soc,
				&pdev->soc->tx_comp_ring[i],
				ring_name);
	}
	dp_print_ring_stat_from_hal(pdev->soc,
			&pdev->rx_refill_buf_ring,
			"Rx Refill Buf Ring");

	dp_print_ring_stat_from_hal(pdev->soc,
			&pdev->rx_refill_buf_ring2,
			"Second Rx Refill Buf Ring");

	dp_print_ring_stat_from_hal(pdev->soc,
			&pdev->rxdma_mon_buf_ring,
			"Rxdma Mon Buf Ring");
	dp_print_ring_stat_from_hal(pdev->soc,
			&pdev->rxdma_mon_dst_ring,
			"Rxdma Mon Dst Ring");
	dp_print_ring_stat_from_hal(pdev->soc,
			&pdev->rxdma_mon_status_ring,
			"Rxdma Mon Status Ring");
	dp_print_ring_stat_from_hal(pdev->soc,
			&pdev->rxdma_mon_desc_ring,
			"Rxdma mon desc Ring");

	for (i = 0; i < MAX_RX_MAC_RINGS; i++) {
		snprintf(ring_name, STR_MAXLEN, "Rxdma err dst ring %d", i);
		dp_print_ring_stat_from_hal(pdev->soc,
			&pdev->rxdma_err_dst_ring[i],
			ring_name);
	}

	for (i = 0; i < MAX_RX_MAC_RINGS; i++) {
		snprintf(ring_name, STR_MAXLEN, "Rx mac buf ring %d", i);
		dp_print_ring_stat_from_hal(pdev->soc,
				&pdev->rx_mac_buf_ring[i],
				ring_name);
	}
}

/**
 * dp_txrx_host_stats_clr(): Reinitialize the txrx stats
 * @vdev: DP_VDEV handle
 *
 * Return:void
 */
static inline void
dp_txrx_host_stats_clr(struct dp_vdev *vdev)
{
	struct dp_peer *peer = NULL;
	struct dp_soc *soc = (struct dp_soc *)vdev->pdev->soc;

	DP_STATS_CLR(vdev->pdev);
	DP_STATS_CLR(vdev->pdev->soc);
	DP_STATS_CLR(vdev);
	TAILQ_FOREACH(peer, &vdev->peer_list, peer_list_elem) {
		if (!peer)
			return;
		DP_STATS_CLR(peer);

		if (soc->cdp_soc.ol_ops->update_dp_stats) {
			soc->cdp_soc.ol_ops->update_dp_stats(
					vdev->pdev->osif_pdev,
					&peer->stats,
					peer->peer_ids[0],
					UPDATE_PEER_STATS);
		}

	}

	if (soc->cdp_soc.ol_ops->update_dp_stats)
		soc->cdp_soc.ol_ops->update_dp_stats(vdev->pdev->osif_pdev,
				&vdev->stats, (uint16_t)vdev->vdev_id,
				UPDATE_VDEV_STATS);
}

/**
 * dp_print_rx_rates(): Print Rx rate stats
 * @vdev: DP_VDEV handle
 *
 * Return:void
 */
static inline void
dp_print_rx_rates(struct dp_vdev *vdev)
{
	struct dp_pdev *pdev = (struct dp_pdev *)vdev->pdev;
	uint8_t i, mcs, pkt_type;
	uint8_t index = 0;
	char nss[DP_NSS_LENGTH];

	DP_PRINT_STATS("Rx Rate Info:\n");

	for (pkt_type = 0; pkt_type < DOT11_MAX; pkt_type++) {
		index = 0;
		for (mcs = 0; mcs < MAX_MCS; mcs++) {
			if (!dp_rate_string[pkt_type][mcs].valid)
				continue;

			DP_PRINT_STATS("	%s = %d",
					dp_rate_string[pkt_type][mcs].mcs_type,
					pdev->stats.rx.pkt_type[pkt_type].
					mcs_count[mcs]);
		}

		DP_PRINT_STATS("\n");
	}

	index = 0;
	for (i = 0; i < SS_COUNT; i++) {
		index += qdf_snprint(&nss[index], DP_NSS_LENGTH - index,
				" %d", pdev->stats.rx.nss[i]);
	}
	DP_PRINT_STATS("NSS(1-8) = %s",
			nss);

	DP_PRINT_STATS("SGI ="
			" 0.8us %d,"
			" 0.4us %d,"
			" 1.6us %d,"
			" 3.2us %d,",
			pdev->stats.rx.sgi_count[0],
			pdev->stats.rx.sgi_count[1],
			pdev->stats.rx.sgi_count[2],
			pdev->stats.rx.sgi_count[3]);
	DP_PRINT_STATS("BW Counts = 20MHZ %d, 40MHZ %d, 80MHZ %d, 160MHZ %d",
			pdev->stats.rx.bw[0], pdev->stats.rx.bw[1],
			pdev->stats.rx.bw[2], pdev->stats.rx.bw[3]);
	DP_PRINT_STATS("Reception Type ="
			" SU: %d,"
			" MU_MIMO:%d,"
			" MU_OFDMA:%d,"
			" MU_OFDMA_MIMO:%d\n",
			pdev->stats.rx.reception_type[0],
			pdev->stats.rx.reception_type[1],
			pdev->stats.rx.reception_type[2],
			pdev->stats.rx.reception_type[3]);
	DP_PRINT_STATS("Aggregation:\n");
	DP_PRINT_STATS("Number of Msdu's Part of Ampdus = %d",
			pdev->stats.rx.ampdu_cnt);
	DP_PRINT_STATS("Number of Msdu's With No Mpdu Level Aggregation : %d",
			pdev->stats.rx.non_ampdu_cnt);
	DP_PRINT_STATS("Number of Msdu's Part of Amsdu: %d",
			pdev->stats.rx.amsdu_cnt);
	DP_PRINT_STATS("Number of Msdu's With No Msdu Level Aggregation: %d",
			pdev->stats.rx.non_amsdu_cnt);
}

/**
 * dp_print_tx_rates(): Print tx rates
 * @vdev: DP_VDEV handle
 *
 * Return:void
 */
static inline void
dp_print_tx_rates(struct dp_vdev *vdev)
{
	struct dp_pdev *pdev = (struct dp_pdev *)vdev->pdev;
	uint8_t mcs, pkt_type;
	uint32_t index;

	DP_PRINT_STATS("Tx Rate Info:\n");

	for (pkt_type = 0; pkt_type < DOT11_MAX; pkt_type++) {
		index = 0;
		for (mcs = 0; mcs < MAX_MCS; mcs++) {
			if (!dp_rate_string[pkt_type][mcs].valid)
				continue;

			DP_PRINT_STATS("	%s = %d",
					dp_rate_string[pkt_type][mcs].mcs_type,
					pdev->stats.tx.pkt_type[pkt_type].
					mcs_count[mcs]);
		}

		DP_PRINT_STATS("\n");
	}

	DP_PRINT_STATS("SGI ="
			" 0.8us %d"
			" 0.4us %d"
			" 1.6us %d"
			" 3.2us %d",
			pdev->stats.tx.sgi_count[0],
			pdev->stats.tx.sgi_count[1],
			pdev->stats.tx.sgi_count[2],
			pdev->stats.tx.sgi_count[3]);

	DP_PRINT_STATS("BW Counts = 20MHZ %d, 40MHZ %d, 80MHZ %d, 160MHZ %d",
			pdev->stats.tx.bw[0], pdev->stats.tx.bw[1],
			pdev->stats.tx.bw[2], pdev->stats.tx.bw[3]);

	DP_PRINT_STATS("OFDMA = %d", pdev->stats.tx.ofdma);
	DP_PRINT_STATS("STBC = %d", pdev->stats.tx.stbc);
	DP_PRINT_STATS("LDPC = %d", pdev->stats.tx.ldpc);
	DP_PRINT_STATS("Retries = %d", pdev->stats.tx.retries);
	DP_PRINT_STATS("Last ack rssi = %d\n", pdev->stats.tx.last_ack_rssi);

	DP_PRINT_STATS("Aggregation:\n");
	DP_PRINT_STATS("Number of Msdu's Part of Amsdu = %d",
			pdev->stats.tx.amsdu_cnt);
	DP_PRINT_STATS("Number of Msdu's With No Msdu Level Aggregation = %d",
			pdev->stats.tx.non_amsdu_cnt);
}

/**
 * dp_print_peer_stats():print peer stats
 * @peer: DP_PEER handle
 *
 * return void
 */
static inline void dp_print_peer_stats(struct dp_peer *peer)
{
	uint8_t i, mcs, pkt_type;
	uint32_t index;
	char nss[DP_NSS_LENGTH];
	DP_PRINT_STATS("Node Tx Stats:\n");
	DP_PRINT_STATS("Total Packet Completions = %d",
			peer->stats.tx.comp_pkt.num);
	DP_PRINT_STATS("Total Bytes Completions = %llu",
			peer->stats.tx.comp_pkt.bytes);
	DP_PRINT_STATS("Success Packets = %d",
			peer->stats.tx.tx_success.num);
	DP_PRINT_STATS("Success Bytes = %llu",
			peer->stats.tx.tx_success.bytes);
	DP_PRINT_STATS("Unicast Success Packets = %d",
			peer->stats.tx.ucast.num);
	DP_PRINT_STATS("Unicast Success Bytes = %llu",
			peer->stats.tx.ucast.bytes);
	DP_PRINT_STATS("Multicast Success Packets = %d",
			peer->stats.tx.mcast.num);
	DP_PRINT_STATS("Multicast Success Bytes = %llu",
			peer->stats.tx.mcast.bytes);
	DP_PRINT_STATS("Broadcast Success Packets = %d",
			peer->stats.tx.bcast.num);
	DP_PRINT_STATS("Broadcast Success Bytes = %llu",
			peer->stats.tx.bcast.bytes);
	DP_PRINT_STATS("Packets Failed = %d",
			peer->stats.tx.tx_failed);
	DP_PRINT_STATS("Packets In OFDMA = %d",
			peer->stats.tx.ofdma);
	DP_PRINT_STATS("Packets In STBC = %d",
			peer->stats.tx.stbc);
	DP_PRINT_STATS("Packets In LDPC = %d",
			peer->stats.tx.ldpc);
	DP_PRINT_STATS("Packet Retries = %d",
			peer->stats.tx.retries);
	DP_PRINT_STATS("MSDU's Part of AMSDU = %d",
			peer->stats.tx.amsdu_cnt);
	DP_PRINT_STATS("Last Packet RSSI = %d",
			peer->stats.tx.last_ack_rssi);
	DP_PRINT_STATS("Dropped At FW: Removed = %d",
			peer->stats.tx.dropped.fw_rem);
	DP_PRINT_STATS("Dropped At FW: Removed transmitted = %d",
			peer->stats.tx.dropped.fw_rem_tx);
	DP_PRINT_STATS("Dropped At FW: Removed Untransmitted = %d",
			peer->stats.tx.dropped.fw_rem_notx);
	DP_PRINT_STATS("Dropped : Age Out = %d",
			peer->stats.tx.dropped.age_out);
	DP_PRINT_STATS("NAWDS : ");
	DP_PRINT_STATS("	Nawds multicast Drop Tx Packet = %d",
			peer->stats.tx.nawds_mcast_drop);
	DP_PRINT_STATS("	Nawds multicast  Tx Packet Count = %d",
			peer->stats.tx.nawds_mcast.num);
	DP_PRINT_STATS("	Nawds multicast  Tx Packet Bytes = %llu",
			peer->stats.tx.nawds_mcast.bytes);

	DP_PRINT_STATS("Rate Info:");

	for (pkt_type = 0; pkt_type < DOT11_MAX; pkt_type++) {
		index = 0;
		for (mcs = 0; mcs < MAX_MCS; mcs++) {
			if (!dp_rate_string[pkt_type][mcs].valid)
				continue;

			DP_PRINT_STATS("	%s = %d",
					dp_rate_string[pkt_type][mcs].mcs_type,
					peer->stats.tx.pkt_type[pkt_type].
					mcs_count[mcs]);
		}

		DP_PRINT_STATS("\n");
	}

	DP_PRINT_STATS("SGI = "
			" 0.8us %d"
			" 0.4us %d"
			" 1.6us %d"
			" 3.2us %d",
			peer->stats.tx.sgi_count[0],
			peer->stats.tx.sgi_count[1],
			peer->stats.tx.sgi_count[2],
			peer->stats.tx.sgi_count[3]);
	DP_PRINT_STATS("Excess Retries per AC ");
	DP_PRINT_STATS("	 Best effort = %d",
			peer->stats.tx.excess_retries_per_ac[0]);
	DP_PRINT_STATS("	 Background= %d",
			peer->stats.tx.excess_retries_per_ac[1]);
	DP_PRINT_STATS("	 Video = %d",
			peer->stats.tx.excess_retries_per_ac[2]);
	DP_PRINT_STATS("	 Voice = %d",
			peer->stats.tx.excess_retries_per_ac[3]);
	DP_PRINT_STATS("BW Counts = 20MHZ %d 40MHZ %d 80MHZ %d 160MHZ %d\n",
			peer->stats.tx.bw[2], peer->stats.tx.bw[3],
			peer->stats.tx.bw[4], peer->stats.tx.bw[5]);

	index = 0;
	for (i = 0; i < SS_COUNT; i++) {
		index += qdf_snprint(&nss[index], DP_NSS_LENGTH - index,
				" %d", peer->stats.tx.nss[i]);
	}
	DP_PRINT_STATS("NSS(1-8) = %s",
			nss);

	DP_PRINT_STATS("Aggregation:");
	DP_PRINT_STATS("	Number of Msdu's Part of Amsdu = %d",
			peer->stats.tx.amsdu_cnt);
	DP_PRINT_STATS("	Number of Msdu's With No Msdu Level Aggregation = %d\n",
			peer->stats.tx.non_amsdu_cnt);

	DP_PRINT_STATS("Node Rx Stats:");
	DP_PRINT_STATS("Packets Sent To Stack = %d",
			peer->stats.rx.to_stack.num);
	DP_PRINT_STATS("Bytes Sent To Stack = %llu",
			peer->stats.rx.to_stack.bytes);
	for (i = 0; i <  CDP_MAX_RX_RINGS; i++) {
		DP_PRINT_STATS("Ring Id = %d", i);
		DP_PRINT_STATS("	Packets Received = %d",
				peer->stats.rx.rcvd_reo[i].num);
		DP_PRINT_STATS("	Bytes Received = %llu",
				peer->stats.rx.rcvd_reo[i].bytes);
	}
	DP_PRINT_STATS("Multicast Packets Received = %d",
			peer->stats.rx.multicast.num);
	DP_PRINT_STATS("Multicast Bytes Received = %llu",
			peer->stats.rx.multicast.bytes);
	DP_PRINT_STATS("Broadcast Packets Received = %d",
			peer->stats.rx.bcast.num);
	DP_PRINT_STATS("Broadcast Bytes Received = %llu",
			peer->stats.rx.bcast.bytes);
	DP_PRINT_STATS("Intra BSS Packets Received = %d",
			peer->stats.rx.intra_bss.pkts.num);
	DP_PRINT_STATS("Intra BSS Bytes Received = %llu",
			peer->stats.rx.intra_bss.pkts.bytes);
	DP_PRINT_STATS("Raw Packets Received = %d",
			peer->stats.rx.raw.num);
	DP_PRINT_STATS("Raw Bytes Received = %llu",
			peer->stats.rx.raw.bytes);
	DP_PRINT_STATS("Errors: MIC Errors = %d",
			peer->stats.rx.err.mic_err);
	DP_PRINT_STATS("Erros: Decryption Errors = %d",
			peer->stats.rx.err.decrypt_err);
	DP_PRINT_STATS("Msdu's Received As Part of Ampdu = %d",
			peer->stats.rx.non_ampdu_cnt);
	DP_PRINT_STATS("Msdu's Recived As Ampdu = %d",
			peer->stats.rx.ampdu_cnt);
	DP_PRINT_STATS("Msdu's Received Not Part of Amsdu's = %d",
			peer->stats.rx.non_amsdu_cnt);
	DP_PRINT_STATS("MSDUs Received As Part of Amsdu = %d",
			peer->stats.rx.amsdu_cnt);
	DP_PRINT_STATS("NAWDS : ");
	DP_PRINT_STATS("	Nawds multicast Drop Rx Packet = %d",
			peer->stats.rx.nawds_mcast_drop.num);
	DP_PRINT_STATS("	Nawds multicast Drop Rx Packet Bytes = %llu",
			peer->stats.rx.nawds_mcast_drop.bytes);
	DP_PRINT_STATS("SGI ="
			" 0.8us %d"
			" 0.4us %d"
			" 1.6us %d"
			" 3.2us %d",
			peer->stats.rx.sgi_count[0],
			peer->stats.rx.sgi_count[1],
			peer->stats.rx.sgi_count[2],
			peer->stats.rx.sgi_count[3]);
	DP_PRINT_STATS("BW Counts = 20MHZ %d 40MHZ %d 80MHZ %d 160MHZ %d",
			peer->stats.rx.bw[0], peer->stats.rx.bw[1],
			peer->stats.rx.bw[2], peer->stats.rx.bw[3]);
	DP_PRINT_STATS("Reception Type ="
			" SU %d,"
			" MU_MIMO %d,"
			" MU_OFDMA %d,"
			" MU_OFDMA_MIMO %d",
			peer->stats.rx.reception_type[0],
			peer->stats.rx.reception_type[1],
			peer->stats.rx.reception_type[2],
			peer->stats.rx.reception_type[3]);


	for (pkt_type = 0; pkt_type < DOT11_MAX; pkt_type++) {
		index = 0;
		for (mcs = 0; mcs < MAX_MCS; mcs++) {
			if (!dp_rate_string[pkt_type][mcs].valid)
				continue;

			DP_PRINT_STATS("	%s = %d",
					dp_rate_string[pkt_type][mcs].mcs_type,
					peer->stats.rx.pkt_type[pkt_type].
					mcs_count[mcs]);
		}

		DP_PRINT_STATS("\n");
	}

	index = 0;
	for (i = 0; i < SS_COUNT; i++) {
		index += qdf_snprint(&nss[index], DP_NSS_LENGTH - index,
				" %d", peer->stats.rx.nss[i]);
	}
	DP_PRINT_STATS("NSS(1-8) = %s",
			nss);

	DP_PRINT_STATS("Aggregation:");
	DP_PRINT_STATS("	Msdu's Part of Ampdu = %d",
			peer->stats.rx.ampdu_cnt);
	DP_PRINT_STATS("	Msdu's With No Mpdu Level Aggregation = %d",
			peer->stats.rx.non_ampdu_cnt);
	DP_PRINT_STATS("	Msdu's Part of Amsdu = %d",
			peer->stats.rx.amsdu_cnt);
	DP_PRINT_STATS("	Msdu's With No Msdu Level Aggregation = %d",
			peer->stats.rx.non_amsdu_cnt);
}

/**
 * dp_print_host_stats()- Function to print the stats aggregated at host
 * @vdev_handle: DP_VDEV handle
 * @type: host stats type
 *
 * Available Stat types
 * TXRX_CLEAR_STATS  : Clear the stats
 * TXRX_RX_RATE_STATS: Print Rx Rate Info
 * TXRX_TX_RATE_STATS: Print Tx Rate Info
 * TXRX_TX_HOST_STATS: Print Tx Stats
 * TXRX_RX_HOST_STATS: Print Rx Stats
 * TXRX_AST_STATS: Print AST Stats
 * TXRX_SRNG_PTR_STATS: Print SRNG ring pointer stats
 *
 * Return: 0 on success, print error message in case of failure
 */
static int
dp_print_host_stats(struct cdp_vdev *vdev_handle, enum cdp_host_txrx_stats type)
{
	struct dp_vdev *vdev = (struct dp_vdev *)vdev_handle;
	struct dp_pdev *pdev = (struct dp_pdev *)vdev->pdev;

	dp_aggregate_pdev_stats(pdev);

	switch (type) {
	case TXRX_CLEAR_STATS:
		dp_txrx_host_stats_clr(vdev);
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
		break;
	case TXRX_SRNG_PTR_STATS:
		 dp_print_ring_stats(pdev);
		 break;
	default:
		DP_TRACE(FATAL, "Wrong Input For TxRx Host Stats");
		break;
	}
	return 0;
}

/*
 * dp_get_host_peer_stats()- function to print peer stats
 * @pdev_handle: DP_PDEV handle
 * @mac_addr: mac address of the peer
 *
 * Return: void
 */
static void
dp_get_host_peer_stats(struct cdp_pdev *pdev_handle, char *mac_addr)
{
	struct dp_peer *peer;
	uint8_t local_id;
	peer = (struct dp_peer *)dp_find_peer_by_addr(pdev_handle, mac_addr,
			&local_id);

	if (!peer) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
			"%s: Invalid peer\n", __func__);
		return;
	}

	dp_print_peer_stats(peer);
	dp_peer_rxtid_stats(peer, dp_rx_tid_stats_cb, NULL);
	return;
}

/*
 * dp_ppdu_ring_reset()- Reset PPDU Stats ring
 * @pdev: DP_PDEV handle
 *
 * Return: void
 */
static void
dp_ppdu_ring_reset(struct dp_pdev *pdev)
{
	struct htt_rx_ring_tlv_filter htt_tlv_filter;

	qdf_mem_set(&(htt_tlv_filter), sizeof(htt_tlv_filter), 0x0);

	htt_h2t_rx_ring_cfg(pdev->soc->htt_handle, pdev->pdev_id,
		pdev->rxdma_mon_status_ring.hal_srng, RXDMA_MONITOR_STATUS,
		RX_BUFFER_SIZE, &htt_tlv_filter);

}

/*
 * dp_ppdu_ring_cfg()- Configure PPDU Stats ring
 * @pdev: DP_PDEV handle
 *
 * Return: void
 */
static void
dp_ppdu_ring_cfg(struct dp_pdev *pdev)
{
	struct htt_rx_ring_tlv_filter htt_tlv_filter = {0};

	htt_tlv_filter.mpdu_start = 0;
	htt_tlv_filter.msdu_start = 0;
	htt_tlv_filter.packet = 0;
	htt_tlv_filter.msdu_end = 0;
	htt_tlv_filter.mpdu_end = 0;
	htt_tlv_filter.packet_header = 1;
	htt_tlv_filter.attention = 1;
	htt_tlv_filter.ppdu_start = 1;
	htt_tlv_filter.ppdu_end = 1;
	htt_tlv_filter.ppdu_end_user_stats = 1;
	htt_tlv_filter.ppdu_end_user_stats_ext = 1;
	htt_tlv_filter.ppdu_end_status_done = 1;
	htt_tlv_filter.enable_fp = 1;
	htt_tlv_filter.enable_md = 0;
	htt_tlv_filter.enable_mo = 0;
	htt_tlv_filter.fp_mgmt_filter = FILTER_MGMT_ALL;
	htt_tlv_filter.fp_ctrl_filter = FILTER_CTRL_ALL;
	htt_tlv_filter.fp_data_filter = FILTER_DATA_ALL;
	htt_tlv_filter.mo_mgmt_filter = FILTER_MGMT_ALL;
	htt_tlv_filter.mo_ctrl_filter = FILTER_CTRL_ALL;
	htt_tlv_filter.mo_data_filter = FILTER_DATA_ALL;

	htt_h2t_rx_ring_cfg(pdev->soc->htt_handle, pdev->pdev_id,
		pdev->rxdma_mon_status_ring.hal_srng, RXDMA_MONITOR_STATUS,
		RX_BUFFER_SIZE, &htt_tlv_filter);
}

/*
 * dp_config_debug_sniffer()- API to enable/disable debug sniffer
 * @pdev_handle: DP_PDEV handle
 * @val: user provided value
 *
 * Return: void
 */
static void
dp_config_debug_sniffer(struct cdp_pdev *pdev_handle, int val)
{
	struct dp_pdev *pdev = (struct dp_pdev *)pdev_handle;

	switch (val) {
	case 0:
		pdev->tx_sniffer_enable = 0;
		pdev->mcopy_mode = 0;

		if (!pdev->pktlog_ppdu_stats && !pdev->enhanced_stats_en) {
			dp_h2t_cfg_stats_msg_send(pdev, 0, pdev->pdev_id);
			dp_ppdu_ring_reset(pdev);
		} else if (pdev->enhanced_stats_en) {
			dp_h2t_cfg_stats_msg_send(pdev,
				DP_PPDU_STATS_CFG_ENH_STATS, pdev->pdev_id);
		}
		break;

	case 1:
		pdev->tx_sniffer_enable = 1;
		pdev->mcopy_mode = 0;

		if (!pdev->pktlog_ppdu_stats)
			dp_h2t_cfg_stats_msg_send(pdev,
				DP_PPDU_STATS_CFG_SNIFFER, pdev->pdev_id);
		break;
	case 2:
		pdev->mcopy_mode = 1;
		pdev->tx_sniffer_enable = 0;
		if (!pdev->enhanced_stats_en)
			dp_ppdu_ring_cfg(pdev);

		if (!pdev->pktlog_ppdu_stats)
			dp_h2t_cfg_stats_msg_send(pdev,
				DP_PPDU_STATS_CFG_SNIFFER, pdev->pdev_id);
		break;
	default:
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
			"Invalid value\n");
		break;
	}
}

/*
 * dp_enable_enhanced_stats()- API to enable enhanced statistcs
 * @pdev_handle: DP_PDEV handle
 *
 * Return: void
 */
static void
dp_enable_enhanced_stats(struct cdp_pdev *pdev_handle)
{
	struct dp_pdev *pdev = (struct dp_pdev *)pdev_handle;
	pdev->enhanced_stats_en = 1;

	if (!pdev->mcopy_mode)
		dp_ppdu_ring_cfg(pdev);

	if (!pdev->pktlog_ppdu_stats && !pdev->tx_sniffer_enable && !pdev->mcopy_mode)
		dp_h2t_cfg_stats_msg_send(pdev, DP_PPDU_STATS_CFG_ENH_STATS, pdev->pdev_id);
}

/*
 * dp_disable_enhanced_stats()- API to disable enhanced statistcs
 * @pdev_handle: DP_PDEV handle
 *
 * Return: void
 */
static void
dp_disable_enhanced_stats(struct cdp_pdev *pdev_handle)
{
	struct dp_pdev *pdev = (struct dp_pdev *)pdev_handle;

	pdev->enhanced_stats_en = 0;

	if (!pdev->pktlog_ppdu_stats && !pdev->tx_sniffer_enable && !pdev->mcopy_mode)
		dp_h2t_cfg_stats_msg_send(pdev, 0, pdev->pdev_id);

	if (!pdev->mcopy_mode)
		dp_ppdu_ring_reset(pdev);
}

/*
 * dp_get_fw_peer_stats()- function to print peer stats
 * @pdev_handle: DP_PDEV handle
 * @mac_addr: mac address of the peer
 * @cap: Type of htt stats requested
 *
 * Currently Supporting only MAC ID based requests Only
 *	1: HTT_PEER_STATS_REQ_MODE_NO_QUERY
 *	2: HTT_PEER_STATS_REQ_MODE_QUERY_TQM
 *	3: HTT_PEER_STATS_REQ_MODE_FLUSH_TQM
 *
 * Return: void
 */
static void
dp_get_fw_peer_stats(struct cdp_pdev *pdev_handle, uint8_t *mac_addr,
		uint32_t cap)
{
	struct dp_pdev *pdev = (struct dp_pdev *)pdev_handle;
	int i;
	uint32_t config_param0 = 0;
	uint32_t config_param1 = 0;
	uint32_t config_param2 = 0;
	uint32_t config_param3 = 0;

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

	dp_h2t_ext_stats_msg_send(pdev, HTT_DBG_EXT_STATS_PEER_INFO,
			config_param0, config_param1, config_param2,
			config_param3, 0, 0);

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
 * @pdev_handle: DP pdev handle
 * @data: pointer to request data
 * @data_len: length for request data
 *
 * return: void
 */
static void
dp_get_htt_stats(struct cdp_pdev *pdev_handle, void *data, uint32_t data_len)
{
	struct dp_pdev *pdev = (struct dp_pdev *)pdev_handle;
	struct httstats_cmd_req *req = (struct httstats_cmd_req *)data;

	QDF_ASSERT(data_len == sizeof(struct httstats_cmd_req));
	dp_h2t_ext_stats_msg_send(pdev, req->stats_id,
				req->config_param0, req->config_param1,
				req->config_param2, req->config_param3,
				req->cookie, 0);
}
/*
 * dp_set_pdev_param: function to set parameters in pdev
 * @pdev_handle: DP pdev handle
 * @param: parameter type to be set
 * @val: value of parameter to be set
 *
 * return: void
 */
static void dp_set_pdev_param(struct cdp_pdev *pdev_handle,
		enum cdp_pdev_param_type param, uint8_t val)
{
	switch (param) {
	case CDP_CONFIG_DEBUG_SNIFFER:
		dp_config_debug_sniffer(pdev_handle, val);
		break;
	default:
		break;
	}
}

/*
 * dp_set_vdev_param: function to set parameters in vdev
 * @param: parameter type to be set
 * @val: value of parameter to be set
 *
 * return: void
 */
static void dp_set_vdev_param(struct cdp_vdev *vdev_handle,
		enum cdp_vdev_param_type param, uint32_t val)
{
	struct dp_vdev *vdev = (struct dp_vdev *)vdev_handle;
	switch (param) {
	case CDP_ENABLE_WDS:
		vdev->wds_enabled = val;
		break;
	case CDP_ENABLE_NAWDS:
		vdev->nawds_enabled = val;
		break;
	case CDP_ENABLE_MCAST_EN:
		vdev->mcast_enhancement_en = val;
		break;
	case CDP_ENABLE_PROXYSTA:
		vdev->proxysta_vdev = val;
		break;
	case CDP_UPDATE_TDLS_FLAGS:
		vdev->tdls_link_connected = val;
		break;
	case CDP_CFG_WDS_AGING_TIMER:
		if (val == 0)
			qdf_timer_stop(&vdev->pdev->soc->wds_aging_timer);
		else if (val != vdev->wds_aging_timer_val)
			qdf_timer_mod(&vdev->pdev->soc->wds_aging_timer, val);

		vdev->wds_aging_timer_val = val;
		break;
	case CDP_ENABLE_AP_BRIDGE:
		if (wlan_op_mode_sta != vdev->opmode)
			vdev->ap_bridge_enabled = val;
		else
			vdev->ap_bridge_enabled = false;
		break;
	case CDP_ENABLE_CIPHER:
		vdev->sec_type = val;
		break;
	case CDP_ENABLE_QWRAP_ISOLATION:
		vdev->isolation_vdev = val;
		break;
	default:
		break;
	}

	dp_tx_vdev_update_search_flags(vdev);
}

/**
 * dp_peer_set_nawds: set nawds bit in peer
 * @peer_handle: pointer to peer
 * @value: enable/disable nawds
 *
 * return: void
 */
static void dp_peer_set_nawds(struct cdp_peer *peer_handle, uint8_t value)
{
	struct dp_peer *peer = (struct dp_peer *)peer_handle;
	peer->nawds_enabled = value;
}

/*
 * dp_set_vdev_dscp_tid_map_wifi3(): Update Map ID selected for particular vdev
 * @vdev_handle: DP_VDEV handle
 * @map_id:ID of map that needs to be updated
 *
 * Return: void
 */
static void dp_set_vdev_dscp_tid_map_wifi3(struct cdp_vdev *vdev_handle,
		uint8_t map_id)
{
	struct dp_vdev *vdev = (struct dp_vdev *)vdev_handle;
	vdev->dscp_tid_map_id = map_id;
	return;
}

/*
 * dp_txrx_stats_publish(): publish pdev stats into a buffer
 * @pdev_handle: DP_PDEV handle
 * @buf: to hold pdev_stats
 *
 * Return: int
 */
static int
dp_txrx_stats_publish(struct cdp_pdev *pdev_handle, void *buf)
{
	struct dp_pdev *pdev = (struct dp_pdev *)pdev_handle;
	struct cdp_pdev_stats *buffer = (struct cdp_pdev_stats *) buf;
	struct cdp_txrx_stats_req req = {0,};

	dp_aggregate_pdev_stats(pdev);
	req.stats = HTT_DBG_EXT_STATS_PDEV_TX;
	req.cookie_val = 1;
	dp_h2t_ext_stats_msg_send(pdev, req.stats, req.param0,
				req.param1, req.param2, req.param3, 0, req.cookie_val);
	msleep(DP_MAX_SLEEP_TIME);

	req.stats = HTT_DBG_EXT_STATS_PDEV_RX;
	req.cookie_val = 1;
	dp_h2t_ext_stats_msg_send(pdev, req.stats, req.param0,
				req.param1, req.param2, req.param3, 0, req.cookie_val);
	msleep(DP_MAX_SLEEP_TIME);
	qdf_mem_copy(buffer, &pdev->stats, sizeof(pdev->stats));

	return TXRX_STATS_LEVEL;
}

/**
 * dp_set_pdev_dscp_tid_map_wifi3(): update dscp tid map in pdev
 * @pdev: DP_PDEV handle
 * @map_id: ID of map that needs to be updated
 * @tos: index value in map
 * @tid: tid value passed by the user
 *
 * Return: void
 */
static void dp_set_pdev_dscp_tid_map_wifi3(struct cdp_pdev *pdev_handle,
		uint8_t map_id, uint8_t tos, uint8_t tid)
{
	uint8_t dscp;
	struct dp_pdev *pdev = (struct dp_pdev *) pdev_handle;
	dscp = (tos >> DP_IP_DSCP_SHIFT) & DP_IP_DSCP_MASK;
	pdev->dscp_tid_map[map_id][dscp] = tid;
	if (map_id < HAL_MAX_HW_DSCP_TID_MAPS)
		hal_tx_update_dscp_tid(pdev->soc->hal_soc, tid,
			map_id, dscp);
	return;
}

/**
 * dp_fw_stats_process(): Process TxRX FW stats request
 * @vdev_handle: DP VDEV handle
 * @req: stats request
 *
 * return: int
 */
static int dp_fw_stats_process(struct cdp_vdev *vdev_handle,
		struct cdp_txrx_stats_req *req)
{
	struct dp_vdev *vdev = (struct dp_vdev *)vdev_handle;
	struct dp_pdev *pdev = NULL;
	uint32_t stats = req->stats;

	if (!vdev) {
		DP_TRACE(NONE, "VDEV not found");
		return 1;
	}
	pdev = vdev->pdev;

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
	}

	return dp_h2t_ext_stats_msg_send(pdev, stats, req->param0,
				req->param1, req->param2, req->param3, 0, 0);
}

/**
 * dp_txrx_stats_request - function to map to firmware and host stats
 * @vdev: virtual handle
 * @req: stats request
 *
 * Return: integer
 */
static int dp_txrx_stats_request(struct cdp_vdev *vdev,
		struct cdp_txrx_stats_req *req)
{
	int host_stats;
	int fw_stats;
	enum cdp_stats stats;

	if (!vdev || !req) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
				"Invalid vdev/req instance");
		return 0;
	}
	stats = req->stats;
	if (stats >= CDP_TXRX_MAX_STATS)
		return 0;

	/*
	 * DP_CURR_FW_STATS_AVAIL: no of FW stats currently available
	 *			has to be updated if new FW HTT stats added
	 */
	if (stats > CDP_TXRX_STATS_HTT_MAX)
		stats = stats + DP_CURR_FW_STATS_AVAIL - DP_HTT_DBG_EXT_STATS_MAX;
	fw_stats = dp_stats_mapping_table[stats][STATS_FW];
	host_stats = dp_stats_mapping_table[stats][STATS_HOST];

	QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_INFO,
		 "stats: %u fw_stats_type: %d host_stats_type: %d",
		  stats, fw_stats, host_stats);

	if (fw_stats != TXRX_FW_STATS_INVALID) {
		/* update request with FW stats type */
		req->stats = fw_stats;
		return dp_fw_stats_process(vdev, req);
	}

	if ((host_stats != TXRX_HOST_STATS_INVALID) &&
			(host_stats <= TXRX_HOST_STATS_MAX))
		return dp_print_host_stats(vdev, host_stats);
	else
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_INFO,
				"Wrong Input for TxRx Stats");

	return 0;
}

/**
 * dp_txrx_stats() - function to map to firmware and host stats
 * @vdev: virtual handle
 * @stats: type of statistics requested
 *
 * Return: integer
 */
static int dp_txrx_stats(struct cdp_vdev *vdev, enum cdp_stats stats)
{
	struct cdp_txrx_stats_req req = {0,};

	req.stats = stats;

	return dp_txrx_stats_request(vdev, &req);
}

/*
 * dp_print_napi_stats(): NAPI stats
 * @soc - soc handle
 */
static void dp_print_napi_stats(struct dp_soc *soc)
{
	hif_print_napi_stats(soc->hif_handle);
}

/*
 * dp_print_per_ring_stats(): Packet count per ring
 * @soc - soc handle
 */
static void dp_print_per_ring_stats(struct dp_soc *soc)
{
	uint8_t ring;
	uint16_t core;
	uint64_t total_packets;

	DP_TRACE(FATAL, "Reo packets per ring:");
	for (ring = 0; ring < MAX_REO_DEST_RINGS; ring++) {
		total_packets = 0;
		DP_TRACE(FATAL, "Packets on ring %u:", ring);
		for (core = 0; core < NR_CPUS; core++) {
			DP_TRACE(FATAL, "Packets arriving on core %u: %llu",
				core, soc->stats.rx.ring_packets[core][ring]);
			total_packets += soc->stats.rx.ring_packets[core][ring];
		}
		DP_TRACE(FATAL, "Total packets on ring %u: %llu",
			ring, total_packets);
	}
}

/*
 * dp_txrx_path_stats() - Function to display dump stats
 * @soc - soc handle
 *
 * return: none
 */
static void dp_txrx_path_stats(struct dp_soc *soc)
{
	uint8_t error_code;
	uint8_t loop_pdev;
	struct dp_pdev *pdev;
	uint8_t i;

	for (loop_pdev = 0; loop_pdev < soc->pdev_count; loop_pdev++) {

		pdev = soc->pdev_list[loop_pdev];
		dp_aggregate_pdev_stats(pdev);
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
			"Tx path Statistics:");

		DP_TRACE(FATAL, "from stack: %u msdus (%llu bytes)",
			pdev->stats.tx_i.rcvd.num,
			pdev->stats.tx_i.rcvd.bytes);
		DP_TRACE(FATAL, "processed from host: %u msdus (%llu bytes)",
			pdev->stats.tx_i.processed.num,
			pdev->stats.tx_i.processed.bytes);
		DP_TRACE(FATAL, "successfully transmitted: %u msdus (%llu bytes)",
			pdev->stats.tx.tx_success.num,
			pdev->stats.tx.tx_success.bytes);

		DP_TRACE(FATAL, "Dropped in host:");
		DP_TRACE(FATAL, "Total packets dropped: %u,",
			pdev->stats.tx_i.dropped.dropped_pkt.num);
		DP_TRACE(FATAL, "Descriptor not available: %u",
			pdev->stats.tx_i.dropped.desc_na);
		DP_TRACE(FATAL, "Ring full: %u",
			pdev->stats.tx_i.dropped.ring_full);
		DP_TRACE(FATAL, "Enqueue fail: %u",
			pdev->stats.tx_i.dropped.enqueue_fail);
		DP_TRACE(FATAL, "DMA Error: %u",
			pdev->stats.tx_i.dropped.dma_error);

		DP_TRACE(FATAL, "Dropped in hardware:");
		DP_TRACE(FATAL, "total packets dropped: %u",
			pdev->stats.tx.tx_failed);
		DP_TRACE(FATAL, "mpdu age out: %u",
			pdev->stats.tx.dropped.age_out);
		DP_TRACE(FATAL, "firmware removed: %u",
			pdev->stats.tx.dropped.fw_rem);
		DP_TRACE(FATAL, "firmware removed tx: %u",
			pdev->stats.tx.dropped.fw_rem_tx);
		DP_TRACE(FATAL, "firmware removed notx %u",
			pdev->stats.tx.dropped.fw_rem_notx);
		DP_TRACE(FATAL, "peer_invalid: %u",
			pdev->soc->stats.tx.tx_invalid_peer.num);


		DP_TRACE(FATAL, "Tx packets sent per interrupt:");
		DP_TRACE(FATAL, "Single Packet: %u",
			pdev->stats.tx_comp_histogram.pkts_1);
		DP_TRACE(FATAL, "2-20 Packets:  %u",
			pdev->stats.tx_comp_histogram.pkts_2_20);
		DP_TRACE(FATAL, "21-40 Packets: %u",
			pdev->stats.tx_comp_histogram.pkts_21_40);
		DP_TRACE(FATAL, "41-60 Packets: %u",
			pdev->stats.tx_comp_histogram.pkts_41_60);
		DP_TRACE(FATAL, "61-80 Packets: %u",
			pdev->stats.tx_comp_histogram.pkts_61_80);
		DP_TRACE(FATAL, "81-100 Packets: %u",
			pdev->stats.tx_comp_histogram.pkts_81_100);
		DP_TRACE(FATAL, "101-200 Packets: %u",
			pdev->stats.tx_comp_histogram.pkts_101_200);
		DP_TRACE(FATAL, "   201+ Packets: %u",
			pdev->stats.tx_comp_histogram.pkts_201_plus);

		DP_TRACE(FATAL, "Rx path statistics");

		DP_TRACE(FATAL, "delivered %u msdus ( %llu bytes),",
			pdev->stats.rx.to_stack.num,
			pdev->stats.rx.to_stack.bytes);
		for (i = 0; i <  CDP_MAX_RX_RINGS; i++)
			DP_TRACE(FATAL, "received on reo[%d] %u msdus ( %llu bytes),",
					i, pdev->stats.rx.rcvd_reo[i].num,
					pdev->stats.rx.rcvd_reo[i].bytes);
		DP_TRACE(FATAL, "intra-bss packets %u msdus ( %llu bytes),",
			pdev->stats.rx.intra_bss.pkts.num,
			pdev->stats.rx.intra_bss.pkts.bytes);
		DP_TRACE(FATAL, "intra-bss fails %u msdus ( %llu bytes),",
			pdev->stats.rx.intra_bss.fail.num,
			pdev->stats.rx.intra_bss.fail.bytes);
		DP_TRACE(FATAL, "raw packets %u msdus ( %llu bytes),",
			pdev->stats.rx.raw.num,
			pdev->stats.rx.raw.bytes);
		DP_TRACE(FATAL, "dropped: error %u msdus",
			pdev->stats.rx.err.mic_err);
		DP_TRACE(FATAL, "peer invalid %u",
			pdev->soc->stats.rx.err.rx_invalid_peer.num);

		DP_TRACE(FATAL, "Reo Statistics");
		DP_TRACE(FATAL, "rbm error: %u msdus",
			pdev->soc->stats.rx.err.invalid_rbm);
		DP_TRACE(FATAL, "hal ring access fail: %u msdus",
			pdev->soc->stats.rx.err.hal_ring_access_fail);

		DP_TRACE(FATAL, "Reo errors");

		for (error_code = 0; error_code < HAL_REO_ERR_MAX;
				error_code++) {
			DP_TRACE(FATAL, "Reo error number (%u): %u msdus",
				error_code,
				pdev->soc->stats.rx.err.reo_error[error_code]);
		}

		for (error_code = 0; error_code < HAL_RXDMA_ERR_MAX;
				error_code++) {
			DP_TRACE(FATAL, "Rxdma error number (%u): %u msdus",
				error_code,
				pdev->soc->stats.rx.err
				.rxdma_error[error_code]);
		}

		DP_TRACE(FATAL, "Rx packets reaped per interrupt:");
		DP_TRACE(FATAL, "Single Packet: %u",
			 pdev->stats.rx_ind_histogram.pkts_1);
		DP_TRACE(FATAL, "2-20 Packets:  %u",
			 pdev->stats.rx_ind_histogram.pkts_2_20);
		DP_TRACE(FATAL, "21-40 Packets: %u",
			 pdev->stats.rx_ind_histogram.pkts_21_40);
		DP_TRACE(FATAL, "41-60 Packets: %u",
			 pdev->stats.rx_ind_histogram.pkts_41_60);
		DP_TRACE(FATAL, "61-80 Packets: %u",
			 pdev->stats.rx_ind_histogram.pkts_61_80);
		DP_TRACE(FATAL, "81-100 Packets: %u",
			 pdev->stats.rx_ind_histogram.pkts_81_100);
		DP_TRACE(FATAL, "101-200 Packets: %u",
			 pdev->stats.rx_ind_histogram.pkts_101_200);
		DP_TRACE(FATAL, "   201+ Packets: %u",
			 pdev->stats.rx_ind_histogram.pkts_201_plus);

		DP_TRACE_STATS(ERROR, "%s: tso_enable: %u lro_enable: %u rx_hash: %u napi_enable: %u",
			__func__,
			pdev->soc->wlan_cfg_ctx->tso_enabled,
			pdev->soc->wlan_cfg_ctx->lro_enabled,
			pdev->soc->wlan_cfg_ctx->rx_hash,
			pdev->soc->wlan_cfg_ctx->napi_enabled);
#ifdef QCA_LL_TX_FLOW_CONTROL_V2
		DP_TRACE_STATS(ERROR, "%s: Tx flow stop queue: %u tx flow start queue offset: %u",
			__func__,
			pdev->soc->wlan_cfg_ctx->tx_flow_stop_queue_threshold,
			pdev->soc->wlan_cfg_ctx->tx_flow_start_queue_offset);
#endif
	}
}

/*
 * dp_txrx_dump_stats() -  Dump statistics
 * @value - Statistics option
 */
static QDF_STATUS dp_txrx_dump_stats(void *psoc, uint16_t value,
				     enum qdf_stats_verbosity_level level)
{
	struct dp_soc *soc =
		(struct dp_soc *)psoc;
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	if (!soc) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
			"%s: soc is NULL", __func__);
		return QDF_STATUS_E_INVAL;
	}

	switch (value) {
	case CDP_TXRX_PATH_STATS:
		dp_txrx_path_stats(soc);
		break;

	case CDP_RX_RING_STATS:
		dp_print_per_ring_stats(soc);
		break;

	case CDP_TXRX_TSO_STATS:
		/* TODO: NOT IMPLEMENTED */
		break;

	case CDP_DUMP_TX_FLOW_POOL_INFO:
		cdp_dump_flow_pool_info((struct cdp_soc_t *)soc);
		break;

	case CDP_DP_NAPI_STATS:
		dp_print_napi_stats(soc);
		break;

	case CDP_TXRX_DESC_STATS:
		/* TODO: NOT IMPLEMENTED */
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
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
				"%s: Invalid handle", __func__);
		return QDF_STATUS_E_INVAL;
	}

	soc->wlan_cfg_ctx->tso_enabled = params->tso_enable;
	soc->wlan_cfg_ctx->lro_enabled = params->lro_enable;
	soc->wlan_cfg_ctx->rx_hash = params->flow_steering_enable;
	soc->wlan_cfg_ctx->tcp_udp_checksumoffload =
				params->tcp_udp_checksumoffload;
	soc->wlan_cfg_ctx->napi_enabled = params->napi_enable;

	dp_update_flow_control_parameters(soc, params);

	return QDF_STATUS_SUCCESS;
}

/**
 * dp_txrx_set_wds_rx_policy() - API to store datapath
 *                            config parameters
 * @vdev_handle - datapath vdev handle
 * @cfg: ini parameter handle
 *
 * Return: status
 */
#ifdef WDS_VENDOR_EXTENSION
void
dp_txrx_set_wds_rx_policy(
		struct cdp_vdev *vdev_handle,
		u_int32_t val)
{
	struct dp_vdev *vdev = (struct dp_vdev *)vdev_handle;
	struct dp_peer *peer;
	if (vdev->opmode == wlan_op_mode_ap) {
		/* for ap, set it on bss_peer */
		TAILQ_FOREACH(peer, &vdev->peer_list, peer_list_elem) {
			if (peer->bss_peer) {
				peer->wds_ecm.wds_rx_filter = 1;
				peer->wds_ecm.wds_rx_ucast_4addr = (val & WDS_POLICY_RX_UCAST_4ADDR) ? 1:0;
				peer->wds_ecm.wds_rx_mcast_4addr = (val & WDS_POLICY_RX_MCAST_4ADDR) ? 1:0;
				break;
			}
		}
	} else if (vdev->opmode == wlan_op_mode_sta) {
		peer = TAILQ_FIRST(&vdev->peer_list);
		peer->wds_ecm.wds_rx_filter = 1;
		peer->wds_ecm.wds_rx_ucast_4addr = (val & WDS_POLICY_RX_UCAST_4ADDR) ? 1:0;
		peer->wds_ecm.wds_rx_mcast_4addr = (val & WDS_POLICY_RX_MCAST_4ADDR) ? 1:0;
	}
}

/**
 * dp_txrx_peer_wds_tx_policy_update() - API to set tx wds policy
 *
 * @peer_handle - datapath peer handle
 * @wds_tx_ucast: policy for unicast transmission
 * @wds_tx_mcast: policy for multicast transmission
 *
 * Return: void
 */
void
dp_txrx_peer_wds_tx_policy_update(struct cdp_peer *peer_handle,
		int wds_tx_ucast, int wds_tx_mcast)
{
	struct dp_peer *peer = (struct dp_peer *)peer_handle;
	if (wds_tx_ucast || wds_tx_mcast) {
		peer->wds_enabled = 1;
		peer->wds_ecm.wds_tx_ucast_4addr = wds_tx_ucast;
		peer->wds_ecm.wds_tx_mcast_4addr = wds_tx_mcast;
	} else {
		peer->wds_enabled = 0;
		peer->wds_ecm.wds_tx_ucast_4addr = 0;
		peer->wds_ecm.wds_tx_mcast_4addr = 0;
	}

	QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_INFO,
			FL("Policy Update set to :\
				peer->wds_enabled %d\
				peer->wds_ecm.wds_tx_ucast_4addr %d\
				peer->wds_ecm.wds_tx_mcast_4addr %d\n"),
				peer->wds_enabled, peer->wds_ecm.wds_tx_ucast_4addr,
				peer->wds_ecm.wds_tx_mcast_4addr);
	return;
}
#endif

static struct cdp_wds_ops dp_ops_wds = {
	.vdev_set_wds = dp_vdev_set_wds,
#ifdef WDS_VENDOR_EXTENSION
	.txrx_set_wds_rx_policy = dp_txrx_set_wds_rx_policy,
	.txrx_wds_peer_tx_policy_update = dp_txrx_peer_wds_tx_policy_update,
#endif
};

/*
 * dp_peer_delete_ast_entries(): Delete all AST entries for a peer
 * @soc - datapath soc handle
 * @peer - datapath peer handle
 *
 * Delete the AST entries belonging to a peer
 */
#ifdef FEATURE_WDS
static inline void dp_peer_delete_ast_entries(struct dp_soc *soc,
		struct dp_peer *peer)
{
	struct dp_ast_entry *ast_entry, *temp_ast_entry;

	qdf_spin_lock_bh(&soc->ast_lock);
	DP_PEER_ITERATE_ASE_LIST(peer, ast_entry, temp_ast_entry)
		dp_peer_del_ast(soc, ast_entry);

	qdf_spin_unlock_bh(&soc->ast_lock);
}
#else
static inline void dp_peer_delete_ast_entries(struct dp_soc *soc,
		struct dp_peer *peer)
{
}
#endif

/*
 * dp_txrx_data_tx_cb_set(): set the callback for non standard tx
 * @vdev_handle - datapath vdev handle
 * @callback - callback function
 * @ctxt: callback context
 *
 */
static void
dp_txrx_data_tx_cb_set(struct cdp_vdev *vdev_handle,
		       ol_txrx_data_tx_cb callback, void *ctxt)
{
	struct dp_vdev *vdev = (struct dp_vdev *)vdev_handle;

	vdev->tx_non_std_data_callback.func = callback;
	vdev->tx_non_std_data_callback.ctxt = ctxt;
}

/**
 * dp_pdev_get_dp_txrx_handle() - get dp handle from pdev
 * @pdev_hdl: datapath pdev handle
 *
 * Return: opaque pointer to dp txrx handle
 */
static void *dp_pdev_get_dp_txrx_handle(struct cdp_pdev *pdev_hdl)
{
	struct dp_pdev *pdev = (struct dp_pdev *)pdev_hdl;

	return pdev->dp_txrx_handle;
}

/**
 * dp_pdev_set_dp_txrx_handle() - set dp handle in pdev
 * @pdev_hdl: datapath pdev handle
 * @dp_txrx_hdl: opaque pointer for dp_txrx_handle
 *
 * Return: void
 */
static void
dp_pdev_set_dp_txrx_handle(struct cdp_pdev *pdev_hdl, void *dp_txrx_hdl)
{
	struct dp_pdev *pdev = (struct dp_pdev *)pdev_hdl;

	pdev->dp_txrx_handle = dp_txrx_hdl;
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

#ifdef CONFIG_WIN
static void dp_peer_teardown_wifi3(struct cdp_vdev *vdev_hdl, void *peer_hdl)
{
	struct dp_vdev *vdev = (struct dp_vdev *) vdev_hdl;
	struct dp_peer *peer = (struct dp_peer *) peer_hdl;
	struct dp_soc *soc = (struct dp_soc *) vdev->pdev->soc;

	peer->delete_in_progress = true;
	dp_peer_delete_ast_entries(soc, peer);
}
#endif

#ifdef ATH_SUPPORT_NAC_RSSI
static QDF_STATUS dp_config_for_nac_rssi(struct cdp_vdev *vdev_handle,
		enum cdp_nac_param_cmd cmd, char *bssid, char *client_macaddr,
		uint8_t chan_num)
{

	struct dp_vdev *vdev = (struct dp_vdev *)vdev_handle;
	struct dp_pdev *pdev = (struct dp_pdev *)vdev->pdev;
	struct dp_soc *soc = (struct dp_soc *) vdev->pdev->soc;

	pdev->nac_rssi_filtering = 1;
	/* Store address of NAC (neighbour peer) which will be checked
	 * against TA of received packets.
	 */

	if (cmd == CDP_NAC_PARAM_ADD) {
		qdf_mem_copy(vdev->cdp_nac_rssi.client_mac,
				client_macaddr, DP_MAC_ADDR_LEN);
		vdev->cdp_nac_rssi_enabled = 1;
	} else if (cmd == CDP_NAC_PARAM_DEL) {
		if (!qdf_mem_cmp(vdev->cdp_nac_rssi.client_mac,
			client_macaddr, DP_MAC_ADDR_LEN)) {
				/* delete this peer from the list */
			qdf_mem_zero(vdev->cdp_nac_rssi.client_mac,
				DP_MAC_ADDR_LEN);
		}
		vdev->cdp_nac_rssi_enabled = 0;
	}

	if (soc->cdp_soc.ol_ops->config_bssid_in_fw_for_nac_rssi)
		soc->cdp_soc.ol_ops->config_bssid_in_fw_for_nac_rssi
			(vdev->pdev->osif_pdev, vdev->vdev_id, cmd, bssid);

	return QDF_STATUS_SUCCESS;
}
#endif

static struct cdp_cmn_ops dp_ops_cmn = {
	.txrx_soc_attach_target = dp_soc_attach_target_wifi3,
	.txrx_vdev_attach = dp_vdev_attach_wifi3,
	.txrx_vdev_detach = dp_vdev_detach_wifi3,
	.txrx_pdev_attach = dp_pdev_attach_wifi3,
	.txrx_pdev_detach = dp_pdev_detach_wifi3,
	.txrx_peer_create = dp_peer_create_wifi3,
	.txrx_peer_setup = dp_peer_setup_wifi3,
#ifdef CONFIG_WIN
	.txrx_peer_teardown = dp_peer_teardown_wifi3,
#else
	.txrx_peer_teardown = NULL,
#endif
	.txrx_peer_add_ast = dp_peer_add_ast_wifi3,
	.txrx_peer_del_ast = dp_peer_del_ast_wifi3,
	.txrx_peer_update_ast = dp_peer_update_ast_wifi3,
	.txrx_peer_ast_hash_find = dp_peer_ast_hash_find_wifi3,
	.txrx_peer_ast_get_pdev_id = dp_peer_ast_get_pdev_id_wifi3,
	.txrx_peer_ast_get_next_hop = dp_peer_ast_get_next_hop_wifi3,
	.txrx_peer_ast_set_type = dp_peer_ast_set_type_wifi3,
	.txrx_peer_delete = dp_peer_delete_wifi3,
	.txrx_vdev_register = dp_vdev_register_wifi3,
	.txrx_soc_detach = dp_soc_detach_wifi3,
	.txrx_get_vdev_mac_addr = dp_get_vdev_mac_addr_wifi3,
	.txrx_get_vdev_from_vdev_id = dp_get_vdev_from_vdev_id_wifi3,
	.txrx_get_ctrl_pdev_from_vdev = dp_get_ctrl_pdev_from_vdev_wifi3,
	.addba_requestprocess = dp_addba_requestprocess_wifi3,
	.addba_responsesetup = dp_addba_responsesetup_wifi3,
	.delba_process = dp_delba_process_wifi3,
	.set_addba_response = dp_set_addba_response,
	.get_peer_mac_addr_frm_id = dp_get_peer_mac_addr_frm_id,
	.flush_cache_rx_queue = NULL,
	/* TODO: get API's for dscp-tid need to be added*/
	.set_vdev_dscp_tid_map = dp_set_vdev_dscp_tid_map_wifi3,
	.set_pdev_dscp_tid_map = dp_set_pdev_dscp_tid_map_wifi3,
	.txrx_stats = dp_txrx_stats,
	.txrx_stats_request = dp_txrx_stats_request,
	.txrx_set_monitor_mode = dp_vdev_set_monitor_mode,
	.txrx_get_pdev_id_frm_pdev = dp_get_pdev_id_frm_pdev,
	.txrx_set_nac = dp_set_nac,
	.txrx_get_tx_pending = dp_get_tx_pending,
	.txrx_set_pdev_tx_capture = dp_config_debug_sniffer,
	.txrx_get_peer_mac_from_peer_id = dp_get_peer_mac_from_peer_id,
	.display_stats = dp_txrx_dump_stats,
	.txrx_soc_set_nss_cfg = dp_soc_set_nss_cfg_wifi3,
	.txrx_soc_get_nss_cfg = dp_soc_get_nss_cfg_wifi3,
#ifdef DP_INTR_POLL_BASED
	.txrx_intr_attach = dp_soc_interrupt_attach_wrapper,
#else
	.txrx_intr_attach = dp_soc_interrupt_attach,
#endif
	.txrx_intr_detach = dp_soc_interrupt_detach,
	.set_pn_check = dp_set_pn_check_wifi3,
	.update_config_parameters = dp_update_config_parameters,
	/* TODO: Add other functions */
	.txrx_data_tx_cb_set = dp_txrx_data_tx_cb_set,
	.get_dp_txrx_handle = dp_pdev_get_dp_txrx_handle,
	.set_dp_txrx_handle = dp_pdev_set_dp_txrx_handle,
	.get_soc_dp_txrx_handle = dp_soc_get_dp_txrx_handle,
	.set_soc_dp_txrx_handle = dp_soc_set_dp_txrx_handle,
	.tx_send = dp_tx_send,
};

static struct cdp_ctrl_ops dp_ops_ctrl = {
	.txrx_peer_authorize = dp_peer_authorize,
#ifdef QCA_SUPPORT_SON
	.txrx_set_inact_params = dp_set_inact_params,
	.txrx_start_inact_timer = dp_start_inact_timer,
	.txrx_set_overload = dp_set_overload,
	.txrx_peer_is_inact = dp_peer_is_inact,
	.txrx_mark_peer_inact = dp_mark_peer_inact,
#endif
	.txrx_set_vdev_rx_decap_type = dp_set_vdev_rx_decap_type,
	.txrx_set_tx_encap_type = dp_set_vdev_tx_encap_type,
#ifdef MESH_MODE_SUPPORT
	.txrx_set_mesh_mode  = dp_peer_set_mesh_mode,
	.txrx_set_mesh_rx_filter = dp_peer_set_mesh_rx_filter,
#endif
	.txrx_set_vdev_param = dp_set_vdev_param,
	.txrx_peer_set_nawds = dp_peer_set_nawds,
	.txrx_set_pdev_reo_dest = dp_set_pdev_reo_dest,
	.txrx_get_pdev_reo_dest = dp_get_pdev_reo_dest,
	.txrx_set_filter_neighbour_peers = dp_set_filter_neighbour_peers,
	.txrx_update_filter_neighbour_peers =
		dp_update_filter_neighbour_peers,
	.txrx_get_sec_type = dp_get_sec_type,
	/* TODO: Add other functions */
	.txrx_wdi_event_sub = dp_wdi_event_sub,
	.txrx_wdi_event_unsub = dp_wdi_event_unsub,
#ifdef WDI_EVENT_ENABLE
	.txrx_get_pldev = dp_get_pldev,
#endif
	.txrx_set_pdev_param = dp_set_pdev_param,
#ifdef ATH_SUPPORT_NAC_RSSI
	.txrx_vdev_config_for_nac_rssi = dp_config_for_nac_rssi,
#endif
};

static struct cdp_me_ops dp_ops_me = {
#ifdef ATH_SUPPORT_IQUE
	.tx_me_alloc_descriptor = dp_tx_me_alloc_descriptor,
	.tx_me_free_descriptor = dp_tx_me_free_descriptor,
	.tx_me_convert_ucast = dp_tx_me_send_convert_ucast,
#endif
};

static struct cdp_mon_ops dp_ops_mon = {
	.txrx_monitor_set_filter_ucast_data = NULL,
	.txrx_monitor_set_filter_mcast_data = NULL,
	.txrx_monitor_set_filter_non_data = NULL,
	.txrx_monitor_get_filter_ucast_data = dp_vdev_get_filter_ucast_data,
	.txrx_monitor_get_filter_mcast_data = dp_vdev_get_filter_mcast_data,
	.txrx_monitor_get_filter_non_data = dp_vdev_get_filter_non_data,
	.txrx_reset_monitor_mode = dp_reset_monitor_mode,
	/* Added support for HK advance filter */
	.txrx_set_advance_monitor_filter = dp_pdev_set_advance_monitor_filter,
};

static struct cdp_host_stats_ops dp_ops_host_stats = {
	.txrx_per_peer_stats = dp_get_host_peer_stats,
	.get_fw_peer_stats = dp_get_fw_peer_stats,
	.get_htt_stats = dp_get_htt_stats,
	.txrx_enable_enhanced_stats = dp_enable_enhanced_stats,
	.txrx_disable_enhanced_stats = dp_disable_enhanced_stats,
	.txrx_stats_publish = dp_txrx_stats_publish,
	/* TODO */
};

static struct cdp_raw_ops dp_ops_raw = {
	/* TODO */
};

#ifdef CONFIG_WIN
static struct cdp_pflow_ops dp_ops_pflow = {
	/* TODO */
};
#endif /* CONFIG_WIN */

#ifdef FEATURE_RUNTIME_PM
/**
 * dp_runtime_suspend() - ensure DP is ready to runtime suspend
 * @opaque_pdev: DP pdev context
 *
 * DP is ready to runtime suspend if there are no pending TX packets.
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS dp_runtime_suspend(struct cdp_pdev *opaque_pdev)
{
	struct dp_pdev *pdev = (struct dp_pdev *)opaque_pdev;
	struct dp_soc *soc = pdev->soc;

	/* Call DP TX flow control API to check if there is any
	   pending packets */

	if (soc->intr_mode == DP_INTR_POLL)
		qdf_timer_stop(&soc->int_timer);

	return QDF_STATUS_SUCCESS;
}

/**
 * dp_runtime_resume() - ensure DP is ready to runtime resume
 * @opaque_pdev: DP pdev context
 *
 * Resume DP for runtime PM.
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS dp_runtime_resume(struct cdp_pdev *opaque_pdev)
{
	struct dp_pdev *pdev = (struct dp_pdev *)opaque_pdev;
	struct dp_soc *soc = pdev->soc;
	void *hal_srng;
	int i;

	if (soc->intr_mode == DP_INTR_POLL)
		qdf_timer_mod(&soc->int_timer, DP_INTR_POLL_TIMER_MS);

	for (i = 0; i < MAX_TCL_DATA_RINGS; i++) {
		hal_srng = soc->tcl_data_ring[i].hal_srng;
		if (hal_srng) {
			/* We actually only need to acquire the lock */
			hal_srng_access_start(soc->hal_soc, hal_srng);
			/* Update SRC ring head pointer for HW to send
			   all pending packets */
			hal_srng_access_end(soc->hal_soc, hal_srng);
		}
	}

	return QDF_STATUS_SUCCESS;
}
#endif /* FEATURE_RUNTIME_PM */

static QDF_STATUS dp_bus_suspend(struct cdp_pdev *opaque_pdev)
{
	struct dp_pdev *pdev = (struct dp_pdev *)opaque_pdev;
	struct dp_soc *soc = pdev->soc;

	if (soc->intr_mode == DP_INTR_POLL)
		qdf_timer_stop(&soc->int_timer);

	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS dp_bus_resume(struct cdp_pdev *opaque_pdev)
{
	struct dp_pdev *pdev = (struct dp_pdev *)opaque_pdev;
	struct dp_soc *soc = pdev->soc;

	if (soc->intr_mode == DP_INTR_POLL)
		qdf_timer_mod(&soc->int_timer, DP_INTR_POLL_TIMER_MS);

	return QDF_STATUS_SUCCESS;
}

#ifndef CONFIG_WIN
static struct cdp_misc_ops dp_ops_misc = {
	.tx_non_std = dp_tx_non_std,
	.get_opmode = dp_get_opmode,
#ifdef FEATURE_RUNTIME_PM
	.runtime_suspend = dp_runtime_suspend,
	.runtime_resume = dp_runtime_resume,
#endif /* FEATURE_RUNTIME_PM */
	.pkt_log_init = dp_pkt_log_init,
	.pkt_log_con_service = dp_pkt_log_con_service,
};

static struct cdp_flowctl_ops dp_ops_flowctl = {
	/* WIFI 3.0 DP implement as required. */
#ifdef QCA_LL_TX_FLOW_CONTROL_V2
	.register_pause_cb = dp_txrx_register_pause_cb,
	.dump_flow_pool_info = dp_tx_dump_flow_pool_info,
#endif /* QCA_LL_TX_FLOW_CONTROL_V2 */
};

static struct cdp_lflowctl_ops dp_ops_l_flowctl = {
	/* WIFI 3.0 DP NOT IMPLEMENTED YET */
};

#ifdef IPA_OFFLOAD
static struct cdp_ipa_ops dp_ops_ipa = {
	.ipa_get_resource = dp_ipa_get_resource,
	.ipa_set_doorbell_paddr = dp_ipa_set_doorbell_paddr,
	.ipa_op_response = dp_ipa_op_response,
	.ipa_register_op_cb = dp_ipa_register_op_cb,
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
	.ipa_set_perf_level = dp_ipa_set_perf_level
};
#endif

static struct cdp_bus_ops dp_ops_bus = {
	.bus_suspend = dp_bus_suspend,
	.bus_resume = dp_bus_resume
};

static struct cdp_ocb_ops dp_ops_ocb = {
	/* WIFI 3.0 DP NOT IMPLEMENTED YET */
};


static struct cdp_throttle_ops dp_ops_throttle = {
	/* WIFI 3.0 DP NOT IMPLEMENTED YET */
};

static struct cdp_mob_stats_ops dp_ops_mob_stats = {
	/* WIFI 3.0 DP NOT IMPLEMENTED YET */
};

static struct cdp_cfg_ops dp_ops_cfg = {
	/* WIFI 3.0 DP NOT IMPLEMENTED YET */
};

/*
 * dp_wrapper_peer_get_ref_by_addr - wrapper function to get to peer
 * @dev: physical device instance
 * @peer_mac_addr: peer mac address
 * @local_id: local id for the peer
 * @debug_id: to track enum peer access

 * Return: peer instance pointer
 */
static inline void *
dp_wrapper_peer_get_ref_by_addr(struct cdp_pdev *dev, u8 *peer_mac_addr,
				u8 *local_id,
				enum peer_debug_id_type debug_id)
{
	/*
	 * Currently this function does not implement the "get ref"
	 * functionality and is mapped to dp_find_peer_by_addr which does not
	 * increment the peer ref count. So the peer state is uncertain after
	 * calling this API. The functionality needs to be implemented.
	 * Accordingly the corresponding release_ref function is NULL.
	 */
	return dp_find_peer_by_addr(dev, peer_mac_addr, local_id);
}

static struct cdp_peer_ops dp_ops_peer = {
	.register_peer = dp_register_peer,
	.clear_peer = dp_clear_peer,
	.find_peer_by_addr = dp_find_peer_by_addr,
	.find_peer_by_addr_and_vdev = dp_find_peer_by_addr_and_vdev,
	.peer_get_ref_by_addr = dp_wrapper_peer_get_ref_by_addr,
	.peer_release_ref = NULL,
	.local_peer_id = dp_local_peer_id,
	.peer_find_by_local_id = dp_peer_find_by_local_id,
	.peer_state_update = dp_peer_state_update,
	.get_vdevid = dp_get_vdevid,
	.get_vdev_by_sta_id = dp_get_vdev_by_sta_id,
	.peer_get_peer_mac_addr = dp_peer_get_peer_mac_addr,
	.get_vdev_for_peer = dp_get_vdev_for_peer,
	.get_peer_state = dp_get_peer_state,
	.last_assoc_received = dp_get_last_assoc_received,
	.last_disassoc_received = dp_get_last_disassoc_received,
	.last_deauth_received = dp_get_last_deauth_received,
};
#endif

static struct cdp_ops dp_txrx_ops = {
	.cmn_drv_ops = &dp_ops_cmn,
	.ctrl_ops = &dp_ops_ctrl,
	.me_ops = &dp_ops_me,
	.mon_ops = &dp_ops_mon,
	.host_stats_ops = &dp_ops_host_stats,
	.wds_ops = &dp_ops_wds,
	.raw_ops = &dp_ops_raw,
#ifdef CONFIG_WIN
	.pflow_ops = &dp_ops_pflow,
#endif /* CONFIG_WIN */
#ifndef CONFIG_WIN
	.misc_ops = &dp_ops_misc,
	.cfg_ops = &dp_ops_cfg,
	.flowctl_ops = &dp_ops_flowctl,
	.l_flowctl_ops = &dp_ops_l_flowctl,
#ifdef IPA_OFFLOAD
	.ipa_ops = &dp_ops_ipa,
#endif
	.bus_ops = &dp_ops_bus,
	.ocb_ops = &dp_ops_ocb,
	.peer_ops = &dp_ops_peer,
	.throttle_ops = &dp_ops_throttle,
	.mob_stats_ops = &dp_ops_mob_stats,
#endif
};

/*
 * dp_soc_set_txrx_ring_map()
 * @dp_soc: DP handler for soc
 *
 * Return: Void
 */
static void dp_soc_set_txrx_ring_map(struct dp_soc *soc)
{
	uint32_t i;
	for (i = 0; i < WLAN_CFG_INT_NUM_CONTEXTS; i++) {
		soc->tx_ring_map[i] = dp_cpu_ring_map[DP_DEFAULT_MAP][i];
	}
}

/*
 * dp_soc_attach_wifi3() - Attach txrx SOC
 * @ctrl_psoc:	Opaque SOC handle from control plane
 * @htc_handle:	Opaque HTC handle
 * @hif_handle:	Opaque HIF handle
 * @qdf_osdev:	QDF device
 *
 * Return: DP SOC handle on success, NULL on failure
 */
/*
 * Local prototype added to temporarily address warning caused by
 * -Wmissing-prototypes. A more correct solution, namely to expose
 * a prototype in an appropriate header file, will come later.
 */
void *dp_soc_attach_wifi3(void *ctrl_psoc, void *hif_handle,
	HTC_HANDLE htc_handle, qdf_device_t qdf_osdev,
	struct ol_if_ops *ol_ops);
void *dp_soc_attach_wifi3(void *ctrl_psoc, void *hif_handle,
	HTC_HANDLE htc_handle, qdf_device_t qdf_osdev,
	struct ol_if_ops *ol_ops)
{
	struct dp_soc *soc = qdf_mem_malloc(sizeof(*soc));

	if (!soc) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
			FL("DP SOC memory allocation failed"));
		goto fail0;
	}

	soc->cdp_soc.ops = &dp_txrx_ops;
	soc->cdp_soc.ol_ops = ol_ops;
	soc->ctrl_psoc = ctrl_psoc;
	soc->osdev = qdf_osdev;
	soc->hif_handle = hif_handle;

	soc->hal_soc = hif_get_hal_handle(hif_handle);
	soc->htt_handle = htt_soc_attach(soc, ctrl_psoc, htc_handle,
		soc->hal_soc, qdf_osdev);
	if (!soc->htt_handle) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
			FL("HTT attach failed"));
		goto fail1;
	}

	soc->wlan_cfg_ctx = wlan_cfg_soc_attach();
	if (!soc->wlan_cfg_ctx) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
				FL("wlan_cfg_soc_attach failed"));
		goto fail2;
	}

	wlan_cfg_set_rx_hash(soc->wlan_cfg_ctx, rx_hash);
	soc->cce_disable = false;

	if (soc->cdp_soc.ol_ops->get_dp_cfg_param) {
		int ret = soc->cdp_soc.ol_ops->get_dp_cfg_param(soc->ctrl_psoc,
				CDP_CFG_MAX_PEER_ID);

		if (ret != -EINVAL) {
			wlan_cfg_set_max_peer_id(soc->wlan_cfg_ctx, ret);
		}

		ret = soc->cdp_soc.ol_ops->get_dp_cfg_param(soc->ctrl_psoc,
				CDP_CFG_CCE_DISABLE);
		if (ret)
			soc->cce_disable = true;
	}

	qdf_spinlock_create(&soc->peer_ref_mutex);

	qdf_spinlock_create(&soc->reo_desc_freelist_lock);
	qdf_list_create(&soc->reo_desc_freelist, REO_DESC_FREELIST_SIZE);

	/* fill the tx/rx cpu ring map*/
	dp_soc_set_txrx_ring_map(soc);

	qdf_spinlock_create(&soc->htt_stats.lock);
	/* initialize work queue for stats processing */
	qdf_create_work(0, &soc->htt_stats.work, htt_t2h_stats_handler, soc);

	/*Initialize inactivity timer for wifison */
	dp_init_inact_timer(soc);

	return (void *)soc;

fail2:
	htt_soc_detach(soc->htt_handle);
fail1:
	qdf_mem_free(soc);
fail0:
	return NULL;
}

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
		return soc->pdev_list[mac_id];

	/* Typically for MCL as there only 1 PDEV*/
	return soc->pdev_list[0];
}

/*
 * dp_get_ring_id_for_mac_id() -  Return pdev for mac_id
 *
 * @soc: handle to DP soc
 * @mac_id: MAC id
 *
 * Return: ring id
 */
int dp_get_ring_id_for_mac_id(struct dp_soc *soc, uint32_t mac_id)
{
	/*
	 * Single pdev using both MACs will operate on both MAC rings,
	 * which is the case for MCL.
	 */
	if (!wlan_cfg_per_pdev_lmac_ring(soc->wlan_cfg_ctx))
		return mac_id;

	/* For WIN each PDEV will operate one ring, so index is zero. */
	return 0;
}

/*
 * dp_is_hw_dbs_enable() - Procedure to check if DBS is supported
 * @soc:		DP SoC context
 * @max_mac_rings:	No of MAC rings
 *
 * Return: None
 */
static
void dp_is_hw_dbs_enable(struct dp_soc *soc,
				int *max_mac_rings)
{
	bool dbs_enable = false;
	if (soc->cdp_soc.ol_ops->is_hw_dbs_2x2_capable)
		dbs_enable = soc->cdp_soc.ol_ops->
		is_hw_dbs_2x2_capable(soc->ctrl_psoc);

	*max_mac_rings = (dbs_enable)?(*max_mac_rings):1;
}

/*
* dp_set_pktlog_wifi3() - attach txrx vdev
* @pdev: Datapath PDEV handle
* @event: which event's notifications are being subscribed to
* @enable: WDI event subscribe or not. (True or False)
*
* Return: Success, NULL on failure
*/
#ifdef WDI_EVENT_ENABLE
int dp_set_pktlog_wifi3(struct dp_pdev *pdev, uint32_t event,
	bool enable)
{
	struct dp_soc *soc = pdev->soc;
	struct htt_rx_ring_tlv_filter htt_tlv_filter = {0};
	int max_mac_rings = wlan_cfg_get_num_mac_rings
					(pdev->wlan_cfg_ctx);
	uint8_t mac_id = 0;

	dp_is_hw_dbs_enable(soc, &max_mac_rings);

	QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_DEBUG,
			FL("Max_mac_rings %d \n"),
			max_mac_rings);

	if (enable) {
		switch (event) {
		case WDI_EVENT_RX_DESC:
			if (pdev->monitor_vdev) {
				/* Nothing needs to be done if monitor mode is
				 * enabled
				 */
				return 0;
			}
			if (pdev->rx_pktlog_mode != DP_RX_PKTLOG_FULL) {
				pdev->rx_pktlog_mode = DP_RX_PKTLOG_FULL;
				htt_tlv_filter.mpdu_start = 1;
				htt_tlv_filter.msdu_start = 1;
				htt_tlv_filter.msdu_end = 1;
				htt_tlv_filter.mpdu_end = 1;
				htt_tlv_filter.packet_header = 1;
				htt_tlv_filter.attention = 1;
				htt_tlv_filter.ppdu_start = 1;
				htt_tlv_filter.ppdu_end = 1;
				htt_tlv_filter.ppdu_end_user_stats = 1;
				htt_tlv_filter.ppdu_end_user_stats_ext = 1;
				htt_tlv_filter.ppdu_end_status_done = 1;
				htt_tlv_filter.enable_fp = 1;
				htt_tlv_filter.fp_mgmt_filter = FILTER_MGMT_ALL;
				htt_tlv_filter.fp_ctrl_filter = FILTER_CTRL_ALL;
				htt_tlv_filter.fp_data_filter = FILTER_DATA_ALL;
				htt_tlv_filter.mo_mgmt_filter = FILTER_MGMT_ALL;
				htt_tlv_filter.mo_ctrl_filter = FILTER_CTRL_ALL;
				htt_tlv_filter.mo_data_filter = FILTER_DATA_ALL;

				for (mac_id = 0; mac_id < max_mac_rings;
								mac_id++) {
					htt_h2t_rx_ring_cfg(soc->htt_handle,
							pdev->pdev_id + mac_id,
							pdev->rxdma_mon_status_ring
							.hal_srng,
							RXDMA_MONITOR_STATUS,
							RX_BUFFER_SIZE,
							&htt_tlv_filter);

				}

				if (soc->reap_timer_init)
					qdf_timer_mod(&soc->mon_reap_timer,
					DP_INTR_POLL_TIMER_MS);
			}
			break;

		case WDI_EVENT_LITE_RX:
			if (pdev->monitor_vdev) {
				/* Nothing needs to be done if monitor mode is
				 * enabled
				 */
				return 0;
			}

			if (pdev->rx_pktlog_mode != DP_RX_PKTLOG_LITE) {
				pdev->rx_pktlog_mode = DP_RX_PKTLOG_LITE;

				htt_tlv_filter.ppdu_start = 1;
				htt_tlv_filter.ppdu_end = 1;
				htt_tlv_filter.ppdu_end_user_stats = 1;
				htt_tlv_filter.ppdu_end_user_stats_ext = 1;
				htt_tlv_filter.ppdu_end_status_done = 1;
				htt_tlv_filter.mpdu_start = 1;
				htt_tlv_filter.enable_fp = 1;
				htt_tlv_filter.fp_mgmt_filter = FILTER_MGMT_ALL;
				htt_tlv_filter.fp_ctrl_filter = FILTER_CTRL_ALL;
				htt_tlv_filter.fp_data_filter = FILTER_DATA_ALL;
				htt_tlv_filter.mo_mgmt_filter = FILTER_MGMT_ALL;
				htt_tlv_filter.mo_ctrl_filter = FILTER_CTRL_ALL;
				htt_tlv_filter.mo_data_filter = FILTER_DATA_ALL;

				for (mac_id = 0; mac_id < max_mac_rings;
								mac_id++) {
					htt_h2t_rx_ring_cfg(soc->htt_handle,
					pdev->pdev_id + mac_id,
					pdev->rxdma_mon_status_ring
					.hal_srng,
					RXDMA_MONITOR_STATUS,
					RX_BUFFER_SIZE_PKTLOG_LITE,
					&htt_tlv_filter);
				}

				if (soc->reap_timer_init)
					qdf_timer_mod(&soc->mon_reap_timer,
					DP_INTR_POLL_TIMER_MS);
			}
			break;

		case WDI_EVENT_LITE_T2H:
			if (pdev->monitor_vdev) {
				/* Nothing needs to be done if monitor mode is
				 * enabled
				 */
				return 0;
			}
			/* To enable HTT_H2T_MSG_TYPE_PPDU_STATS_CFG in FW
			 * passing value 0xffff. Once these macros will define
			 * in htt header file will use proper macros
			*/
			for (mac_id = 0; mac_id < max_mac_rings; mac_id++) {
				pdev->pktlog_ppdu_stats = true;
				dp_h2t_cfg_stats_msg_send(pdev, 0xffff,
						pdev->pdev_id + mac_id);
			}
			break;

		default:
			/* Nothing needs to be done for other pktlog types */
			break;
		}
	} else {
		switch (event) {
		case WDI_EVENT_RX_DESC:
		case WDI_EVENT_LITE_RX:
			if (pdev->monitor_vdev) {
				/* Nothing needs to be done if monitor mode is
				 * enabled
				 */
				return 0;
			}
			if (pdev->rx_pktlog_mode != DP_RX_PKTLOG_DISABLED) {
				pdev->rx_pktlog_mode = DP_RX_PKTLOG_DISABLED;

				for (mac_id = 0; mac_id < max_mac_rings;
								mac_id++) {
					htt_h2t_rx_ring_cfg(soc->htt_handle,
							pdev->pdev_id + mac_id,
							pdev->rxdma_mon_status_ring
							.hal_srng,
							RXDMA_MONITOR_STATUS,
							RX_BUFFER_SIZE,
							&htt_tlv_filter);
				}

				if (soc->reap_timer_init)
					qdf_timer_stop(&soc->mon_reap_timer);
			}
			break;
		case WDI_EVENT_LITE_T2H:
			if (pdev->monitor_vdev) {
				/* Nothing needs to be done if monitor mode is
				 * enabled
				 */
				return 0;
			}
			/* To disable HTT_H2T_MSG_TYPE_PPDU_STATS_CFG in FW
			 * passing value 0. Once these macros will define in htt
			 * header file will use proper macros
			*/
			for (mac_id = 0; mac_id < max_mac_rings; mac_id++) {
				pdev->pktlog_ppdu_stats = false;
				if (!pdev->enhanced_stats_en && !pdev->tx_sniffer_enable && !pdev->mcopy_mode) {
					dp_h2t_cfg_stats_msg_send(pdev, 0,
							pdev->pdev_id + mac_id);
				} else if (pdev->tx_sniffer_enable || pdev->mcopy_mode) {
					dp_h2t_cfg_stats_msg_send(pdev, DP_PPDU_STATS_CFG_SNIFFER,
							pdev->pdev_id + mac_id);
				} else if (pdev->enhanced_stats_en) {
					dp_h2t_cfg_stats_msg_send(pdev, DP_PPDU_STATS_CFG_ENH_STATS,
							pdev->pdev_id + mac_id);
				}
			}

			break;
		default:
			/* Nothing needs to be done for other pktlog types */
			break;
		}
	}
	return 0;
}
#endif

#ifdef CONFIG_MCL
/*
 * dp_service_mon_rings()- timer to reap monitor rings
 * reqd as we are not getting ppdu end interrupts
 * @arg: SoC Handle
 *
 * Return:
 *
 */
static void dp_service_mon_rings(void *arg)
{
	struct dp_soc *soc = (struct dp_soc *) arg;
	int ring = 0, work_done;

	work_done = dp_mon_process(soc, ring, QCA_NAPI_BUDGET);
	QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_DEBUG,
		FL("Reaped %d descs from Monitor rings"), work_done);

	qdf_timer_mod(&soc->mon_reap_timer, DP_INTR_POLL_TIMER_MS);
}

#ifndef REMOVE_PKT_LOG
/**
 * dp_pkt_log_init() - API to initialize packet log
 * @ppdev: physical device handle
 * @scn: HIF context
 *
 * Return: none
 */
void dp_pkt_log_init(struct cdp_pdev *ppdev, void *scn)
{
	struct dp_pdev *handle = (struct dp_pdev *)ppdev;

	if (handle->pkt_log_init) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
			 "%s: Packet log not initialized", __func__);
		return;
	}

	pktlog_sethandle(&handle->pl_dev, scn);
	pktlog_set_callback_regtype(PKTLOG_LITE_CALLBACK_REGISTRATION);

	if (pktlogmod_init(scn)) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
			 "%s: pktlogmod_init failed", __func__);
		handle->pkt_log_init = false;
	} else {
		handle->pkt_log_init = true;
	}
}

/**
 * dp_pkt_log_con_service() - connect packet log service
 * @ppdev: physical device handle
 * @scn: device context
 *
 * Return: none
 */
static void dp_pkt_log_con_service(struct cdp_pdev *ppdev, void *scn)
{
	struct dp_pdev *pdev = (struct dp_pdev *)ppdev;

	dp_pkt_log_init((struct cdp_pdev *)pdev, scn);
	pktlog_htc_attach();
}

/**
 * dp_pktlogmod_exit() - API to cleanup pktlog info
 * @handle: Pdev handle
 *
 * Return: none
 */
static void dp_pktlogmod_exit(struct dp_pdev *handle)
{
	void *scn = (void *)handle->soc->hif_handle;

	if (!scn) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
			 "%s: Invalid hif(scn) handle", __func__);
		return;
	}

	pktlogmod_exit(scn);
	handle->pkt_log_init = false;
}
#endif
#else
static void dp_pktlogmod_exit(struct dp_pdev *handle) { }
#endif

