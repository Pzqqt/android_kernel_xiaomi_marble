/*
 * Copyright (c) 2015-2016 The Linux Foundation. All rights reserved.
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

#ifndef __CE_H__
#define __CE_H__

#include "qdf_atomic.h"
#include "qdf_lock.h"
#include "hif_main.h"
#include "qdf_util.h"

#define CE_HTT_T2H_MSG 1
#define CE_HTT_H2T_MSG 4

#define CE_OFFSET		0x00000400
#define CE_USEFUL_SIZE		0x00000058
#define CE_ALL_BITMAP  0xFFFF

/**
 * enum ce_id_type
 *
 * @ce_id_type: Copy engine ID
 */
enum ce_id_type {
	CE_ID_0,
	CE_ID_1,
	CE_ID_2,
	CE_ID_3,
	CE_ID_4,
	CE_ID_5,
	CE_ID_6,
	CE_ID_7,
	CE_ID_8,
	CE_ID_9,
	CE_ID_10,
	CE_ID_11,
	CE_ID_MAX
};

#ifdef CONFIG_WIN
#define QWLAN_VERSIONSTR "WIN"
#endif

enum ol_ath_hif_pkt_ecodes {
	HIF_PIPE_NO_RESOURCE = 0
};

struct HIF_CE_state;

/* Per-pipe state. */
struct HIF_CE_pipe_info {
	/* Handle of underlying Copy Engine */
	struct CE_handle *ce_hdl;

	/* Our pipe number; facilitiates use of pipe_info ptrs. */
	uint8_t pipe_num;

	/* Convenience back pointer to HIF_CE_state. */
	struct HIF_CE_state *HIF_CE_state;

	/* Instantaneous number of receive buffers that should be posted */
	atomic_t recv_bufs_needed;
	qdf_size_t buf_sz;
	qdf_spinlock_t recv_bufs_needed_lock;

	qdf_spinlock_t completion_freeq_lock;
	/* Limit the number of outstanding send requests. */
	int num_sends_allowed;

	/* adding three counts for debugging ring buffer errors */
	uint32_t nbuf_alloc_err_count;
	uint32_t nbuf_dma_err_count;
	uint32_t nbuf_ce_enqueue_err_count;
	struct hif_msg_callbacks pipe_callbacks;
};

/**
 * struct ce_tasklet_entry
 *
 * @intr_tq: intr_tq
 * @ce_id: ce_id
 * @inited: inited
 * @hif_ce_state: hif_ce_state
 * @from_irq: from_irq
 */
struct ce_tasklet_entry {
	struct tasklet_struct intr_tq;
	enum ce_id_type ce_id;
	bool inited;
	void *hif_ce_state;
};

struct hif_ext_group_entry {
	uint32_t numirq;
	uint32_t irq[HIF_MAX_GRP_IRQ];
	uint32_t grp_id;
	void *context;
	ext_intr_handler handler;
	struct tasklet_struct intr_tq;
	bool configured;
	bool inited;
	void *hif_state;
};

struct ce_intr_stats {
	uint32_t ce_per_cpu[CE_COUNT_MAX][QDF_MAX_AVAILABLE_CPU];
};

struct HIF_CE_state {
	struct hif_softc ol_sc;
	bool started;
	struct ce_tasklet_entry tasklets[CE_COUNT_MAX];
	struct hif_ext_group_entry hif_ext_group[HIF_MAX_GROUP];
	uint32_t hif_num_extgroup;
	qdf_spinlock_t keep_awake_lock;
	qdf_spinlock_t irq_reg_lock;
	unsigned int keep_awake_count;
	bool verified_awake;
	bool fake_sleep;
	qdf_timer_t sleep_timer;
	bool sleep_timer_init;
	qdf_time_t sleep_ticks;

	struct CE_pipe_config *target_ce_config;
	struct CE_attr *host_ce_config;
	uint32_t target_ce_config_sz;
	/* Per-pipe state. */
	struct HIF_CE_pipe_info pipe_info[CE_COUNT_MAX];
	/* to be activated after BMI_DONE */
	struct hif_msg_callbacks msg_callbacks_pending;
	/* current msg callbacks in use */
	struct hif_msg_callbacks msg_callbacks_current;

	/* Target address used to signal a pending firmware event */
	uint32_t fw_indicator_address;

	/* Copy Engine used for Diagnostic Accesses */
	struct CE_handle *ce_diag;
	struct ce_intr_stats stats;
	struct ce_ops *ce_services;
};

/*
 * HIA Map Definition
 */
struct host_interest_area_t {
	uint32_t hi_interconnect_state;
	uint32_t hi_early_alloc;
	uint32_t hi_option_flag2;
	uint32_t hi_board_data;
	uint32_t hi_board_data_initialized;
	uint32_t hi_failure_state;
	uint32_t hi_rddi_msi_num;
	uint32_t hi_pcie_perst_couple_en;
	uint32_t hi_sw_protocol_version;
};

struct shadow_reg_cfg {
	uint16_t ce_id;
	uint16_t reg_offset;
};

void hif_ce_stop(struct hif_softc *scn);
int hif_dump_ce_registers(struct hif_softc *scn);
void
hif_ce_dump_target_memory(struct hif_softc *scn, void *ramdump_base,
			  uint32_t address, uint32_t size);

#ifdef IPA_OFFLOAD
void hif_ce_ipa_get_ce_resource(struct hif_softc *scn,
			     qdf_dma_addr_t *ce_sr_base_paddr,
			     uint32_t *ce_sr_ring_size,
			     qdf_dma_addr_t *ce_reg_paddr);
#else
static inline
void hif_ce_ipa_get_ce_resource(struct hif_softc *scn,
			     qdf_dma_addr_t *ce_sr_base_paddr,
			     uint32_t *ce_sr_ring_size,
			     qdf_dma_addr_t *ce_reg_paddr)
{
	return;
}

#endif
int hif_wlan_enable(struct hif_softc *scn);
void hif_wlan_disable(struct hif_softc *scn);
void hif_get_target_ce_config(struct hif_softc *scn,
		struct CE_pipe_config **target_ce_config_ret,
		int *target_ce_config_sz_ret,
		struct service_to_pipe **target_service_to_ce_map_ret,
		int *target_service_to_ce_map_sz_ret,
		struct shadow_reg_cfg **target_shadow_reg_cfg_ret,
		int *shadow_cfg_sz_ret);
#endif /* __CE_H__ */
