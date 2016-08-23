/*
 * Copyright (c) 2013-2016 The Linux Foundation. All rights reserved.
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
#include "targcfg.h"
#include "qdf_lock.h"
#include "qdf_status.h"
#include "qdf_status.h"
#include <qdf_atomic.h>         /* qdf_atomic_read */
#include <targaddrs.h>
#include "hif_io32.h"
#include <hif.h>
#include "regtable.h"
#define ATH_MODULE_NAME hif
#include <a_debug.h>
#include "hif_main.h"
#include "ce_api.h"
#include "qdf_trace.h"
#include "pld_common.h"
#include "hif_debug.h"
#include "ce_internal.h"
#include "ce_reg.h"
#include "ce_assignment.h"
#include "ce_tasklet.h"
#ifndef CONFIG_WIN
#include "qwlan_version.h"
#endif

#define CE_POLL_TIMEOUT 10      /* ms */

#define AGC_DUMP         1
#define CHANINFO_DUMP    2
#define BB_WATCHDOG_DUMP 3
#ifdef CONFIG_ATH_PCIE_ACCESS_DEBUG
#define PCIE_ACCESS_DUMP 4
#endif
#include "mp_dev.h"

/* Forward references */
static int hif_post_recv_buffers_for_pipe(struct HIF_CE_pipe_info *pipe_info);

/*
 * Fix EV118783, poll to check whether a BMI response comes
 * other than waiting for the interruption which may be lost.
 */
/* #define BMI_RSP_POLLING */
#define BMI_RSP_TO_MILLISEC  1000

#ifdef CONFIG_BYPASS_QMI
#define BYPASS_QMI 1
#else
#define BYPASS_QMI 0
#endif

#ifdef CONFIG_WIN
#define WDI_IPA_SERVICE_GROUP 5
#define WDI_IPA_TX_SVC MAKE_SERVICE_ID(WDI_IPA_SERVICE_GROUP, 0)
#define HTT_DATA2_MSG_SVC MAKE_SERVICE_ID(HTT_SERVICE_GROUP, 1)
#define HTT_DATA3_MSG_SVC MAKE_SERVICE_ID(HTT_SERVICE_GROUP, 2)
#endif

static int hif_post_recv_buffers(struct hif_softc *scn);
static void hif_config_rri_on_ddr(struct hif_softc *scn);

/**
 * hif_target_access_log_dump() - dump access log
 *
 * dump access log
 *
 * Return: n/a
 */
#ifdef CONFIG_ATH_PCIE_ACCESS_DEBUG
static void hif_target_access_log_dump(void)
{
	hif_target_dump_access_log();
}
#endif


void hif_trigger_dump(struct hif_opaque_softc *hif_ctx,
		      uint8_t cmd_id, bool start)
{
	struct hif_softc *scn = HIF_GET_SOFTC(hif_ctx);

	switch (cmd_id) {
	case AGC_DUMP:
		if (start)
			priv_start_agc(scn);
		else
			priv_dump_agc(scn);
		break;
	case CHANINFO_DUMP:
		if (start)
			priv_start_cap_chaninfo(scn);
		else
			priv_dump_chaninfo(scn);
		break;
	case BB_WATCHDOG_DUMP:
		priv_dump_bbwatchdog(scn);
		break;
#ifdef CONFIG_ATH_PCIE_ACCESS_DEBUG
	case PCIE_ACCESS_DUMP:
		hif_target_access_log_dump();
		break;
#endif
	default:
		HIF_ERROR("%s: Invalid htc dump command", __func__);
		break;
	}
}

static void ce_poll_timeout(void *arg)
{
	struct CE_state *CE_state = (struct CE_state *)arg;
	if (CE_state->timer_inited) {
		ce_per_engine_service(CE_state->scn, CE_state->id);
		qdf_timer_mod(&CE_state->poll_timer, CE_POLL_TIMEOUT);
	}
}

static unsigned int roundup_pwr2(unsigned int n)
{
	int i;
	unsigned int test_pwr2;

	if (!(n & (n - 1)))
		return n; /* already a power of 2 */

	test_pwr2 = 4;
	for (i = 0; i < 29; i++) {
		if (test_pwr2 > n)
			return test_pwr2;
		test_pwr2 = test_pwr2 << 1;
	}

	QDF_ASSERT(0); /* n too large */
	return 0;
}

#define ADRASTEA_SRC_WR_INDEX_OFFSET 0x3C
#define ADRASTEA_DST_WR_INDEX_OFFSET 0x40

static struct shadow_reg_cfg target_shadow_reg_cfg_map[] = {
	{ 0, ADRASTEA_SRC_WR_INDEX_OFFSET},
	{ 3, ADRASTEA_SRC_WR_INDEX_OFFSET},
	{ 4, ADRASTEA_SRC_WR_INDEX_OFFSET},
	{ 5, ADRASTEA_SRC_WR_INDEX_OFFSET},
	{ 7, ADRASTEA_SRC_WR_INDEX_OFFSET},
	{ 1, ADRASTEA_DST_WR_INDEX_OFFSET},
	{ 2, ADRASTEA_DST_WR_INDEX_OFFSET},
	{ 7, ADRASTEA_DST_WR_INDEX_OFFSET},
	{ 8, ADRASTEA_DST_WR_INDEX_OFFSET},
#ifdef QCA_WIFI_3_0_ADRASTEA
	{ 9, ADRASTEA_DST_WR_INDEX_OFFSET},
	{ 10, ADRASTEA_DST_WR_INDEX_OFFSET},
	{ 11, ADRASTEA_DST_WR_INDEX_OFFSET},
#endif
};

static struct shadow_reg_cfg target_shadow_reg_cfg_epping[] = {
	{ 0, ADRASTEA_SRC_WR_INDEX_OFFSET},
	{ 3, ADRASTEA_SRC_WR_INDEX_OFFSET},
	{ 4, ADRASTEA_SRC_WR_INDEX_OFFSET},
	{ 7, ADRASTEA_SRC_WR_INDEX_OFFSET},
	{ 1, ADRASTEA_DST_WR_INDEX_OFFSET},
	{ 2, ADRASTEA_DST_WR_INDEX_OFFSET},
	{ 5, ADRASTEA_DST_WR_INDEX_OFFSET},
	{ 7, ADRASTEA_DST_WR_INDEX_OFFSET},
	{ 8, ADRASTEA_DST_WR_INDEX_OFFSET},
};

/* CE_PCI TABLE */
/*
 * NOTE: the table below is out of date, though still a useful reference.
 * Refer to target_service_to_ce_map and hif_map_service_to_pipe for the actual
 * mapping of HTC services to HIF pipes.
 */
/*
 * This authoritative table defines Copy Engine configuration and the mapping
 * of services/endpoints to CEs.  A subset of this information is passed to
 * the Target during startup as a prerequisite to entering BMI phase.
 * See:
 *    target_service_to_ce_map - Target-side mapping
 *    hif_map_service_to_pipe      - Host-side mapping
 *    target_ce_config         - Target-side configuration
 *    host_ce_config           - Host-side configuration
   ============================================================================
   Purpose    | Service / Endpoint   | CE   | Dire | Xfer     | Xfer
 |                      |      | ctio | Size     | Frequency
 |                      |      | n    |          |
   ============================================================================
   tx         | HTT_DATA (downlink)  | CE 0 | h->t | medium - | very frequent
   descriptor |                      |      |      | O(100B)  | and regular
   download   |                      |      |      |          |
   ----------------------------------------------------------------------------
   rx         | HTT_DATA (uplink)    | CE 1 | t->h | small -  | frequent and
   indication |                      |      |      | O(10B)   | regular
   upload     |                      |      |      |          |
   ----------------------------------------------------------------------------
   MSDU       | DATA_BK (uplink)     | CE 2 | t->h | large -  | rare
   upload     |                      |      |      | O(1000B) | (frequent
   e.g. noise |                      |      |      |          | during IP1.0
   packets    |                      |      |      |          | testing)
   ----------------------------------------------------------------------------
   MSDU       | DATA_BK (downlink)   | CE 3 | h->t | large -  | very rare
   download   |                      |      |      | O(1000B) | (frequent
   e.g.       |                      |      |      |          | during IP1.0
   misdirecte |                      |      |      |          | testing)
   d EAPOL    |                      |      |      |          |
   packets    |                      |      |      |          |
   ----------------------------------------------------------------------------
   n/a        | DATA_BE, DATA_VI     | CE 2 | t->h |          | never(?)
 | DATA_VO (uplink)     |      |      |          |
   ----------------------------------------------------------------------------
   n/a        | DATA_BE, DATA_VI     | CE 3 | h->t |          | never(?)
 | DATA_VO (downlink)   |      |      |          |
   ----------------------------------------------------------------------------
   WMI events | WMI_CONTROL (uplink) | CE 4 | t->h | medium - | infrequent
 |                      |      |      | O(100B)  |
   ----------------------------------------------------------------------------
   WMI        | WMI_CONTROL          | CE 5 | h->t | medium - | infrequent
   messages   | (downlink)           |      |      | O(100B)  |
 |                      |      |      |          |
   ----------------------------------------------------------------------------
   n/a        | HTC_CTRL_RSVD,       | CE 1 | t->h |          | never(?)
 | HTC_RAW_STREAMS      |      |      |          |
 | (uplink)             |      |      |          |
   ----------------------------------------------------------------------------
   n/a        | HTC_CTRL_RSVD,       | CE 0 | h->t |          | never(?)
 | HTC_RAW_STREAMS      |      |      |          |
 | (downlink)           |      |      |          |
   ----------------------------------------------------------------------------
   diag       | none (raw CE)        | CE 7 | t<>h |    4     | Diag Window
 |                      |      |      |          | infrequent
   ============================================================================
 */

/*
 * Map from service/endpoint to Copy Engine.
 * This table is derived from the CE_PCI TABLE, above.
 * It is passed to the Target at startup for use by firmware.
 */
static struct service_to_pipe target_service_to_ce_map_wlan[] = {
	{
		WMI_DATA_VO_SVC,
		PIPEDIR_OUT,    /* out = UL = host -> target */
		3,
	},
	{
		WMI_DATA_VO_SVC,
		PIPEDIR_IN,     /* in = DL = target -> host */
		2,
	},
	{
		WMI_DATA_BK_SVC,
		PIPEDIR_OUT,    /* out = UL = host -> target */
		3,
	},
	{
		WMI_DATA_BK_SVC,
		PIPEDIR_IN,     /* in = DL = target -> host */
		2,
	},
	{
		WMI_DATA_BE_SVC,
		PIPEDIR_OUT,    /* out = UL = host -> target */
		3,
	},
	{
		WMI_DATA_BE_SVC,
		PIPEDIR_IN,     /* in = DL = target -> host */
		2,
	},
	{
		WMI_DATA_VI_SVC,
		PIPEDIR_OUT,    /* out = UL = host -> target */
		3,
	},
	{
		WMI_DATA_VI_SVC,
		PIPEDIR_IN,     /* in = DL = target -> host */
		2,
	},
	{
		WMI_CONTROL_SVC,
		PIPEDIR_OUT,    /* out = UL = host -> target */
		3,
	},
	{
		WMI_CONTROL_SVC,
		PIPEDIR_IN,     /* in = DL = target -> host */
		2,
	},
	{
		HTC_CTRL_RSVD_SVC,
		PIPEDIR_OUT,    /* out = UL = host -> target */
		0,              /* could be moved to 3 (share with WMI) */
	},
	{
		HTC_CTRL_RSVD_SVC,
		PIPEDIR_IN,     /* in = DL = target -> host */
		2,
	},
	{
		HTC_RAW_STREAMS_SVC, /* not currently used */
		PIPEDIR_OUT,    /* out = UL = host -> target */
		0,
	},
	{
		HTC_RAW_STREAMS_SVC, /* not currently used */
		PIPEDIR_IN,     /* in = DL = target -> host */
		2,
	},
	{
		HTT_DATA_MSG_SVC,
		PIPEDIR_OUT,    /* out = UL = host -> target */
		4,
	},
	{
		HTT_DATA_MSG_SVC,
		PIPEDIR_IN,     /* in = DL = target -> host */
		1,
	},
	{
		WDI_IPA_TX_SVC,
		PIPEDIR_OUT,    /* in = DL = target -> host */
		5,
	},
#if defined(QCA_WIFI_3_0_ADRASTEA)
	{
		HTT_DATA2_MSG_SVC,
		PIPEDIR_IN,    /* in = DL = target -> host */
		9,
	},
	{
		HTT_DATA3_MSG_SVC,
		PIPEDIR_IN,    /* in = DL = target -> host */
		10,
	},
	{
		PACKET_LOG_SVC,
		PIPEDIR_IN,    /* in = DL = target -> host */
		11,
	},
#endif
	/* (Additions here) */

	{                       /* Must be last */
		0,
		0,
		0,
	},
};

static struct service_to_pipe target_service_to_ce_map_ar900b[] = {
	{
		WMI_DATA_VO_SVC,
		PIPEDIR_OUT,    /* out = UL = host -> target */
		3,
	},
	{
		WMI_DATA_VO_SVC,
		PIPEDIR_IN,     /* in = DL = target -> host */
		2,
	},
	{
		WMI_DATA_BK_SVC,
		PIPEDIR_OUT,    /* out = UL = host -> target */
		3,
	},
	{
		WMI_DATA_BK_SVC,
		PIPEDIR_IN,     /* in = DL = target -> host */
		2,
	},
	{
		WMI_DATA_BE_SVC,
		PIPEDIR_OUT,    /* out = UL = host -> target */
		3,
	},
	{
		WMI_DATA_BE_SVC,
		PIPEDIR_IN,     /* in = DL = target -> host */
		2,
	},
	{
		WMI_DATA_VI_SVC,
		PIPEDIR_OUT,    /* out = UL = host -> target */
		3,
	},
	{
		WMI_DATA_VI_SVC,
		PIPEDIR_IN,     /* in = DL = target -> host */
		2,
	},
	{
		WMI_CONTROL_SVC,
		PIPEDIR_OUT,    /* out = UL = host -> target */
		3,
	},
	{
		WMI_CONTROL_SVC,
		PIPEDIR_IN,     /* in = DL = target -> host */
		2,
	},
	{
		HTC_CTRL_RSVD_SVC,
		PIPEDIR_OUT,    /* out = UL = host -> target */
		0,              /* could be moved to 3 (share with WMI) */
	},
	{
		HTC_CTRL_RSVD_SVC,
		PIPEDIR_IN,     /* in = DL = target -> host */
		1,
	},
	{
		HTC_RAW_STREAMS_SVC, /* not currently used */
		PIPEDIR_OUT,    /* out = UL = host -> target */
		0,
	},
	{
		HTC_RAW_STREAMS_SVC, /* not currently used */
		PIPEDIR_IN,     /* in = DL = target -> host */
		1,
	},
	{
		HTT_DATA_MSG_SVC,
		PIPEDIR_OUT,    /* out = UL = host -> target */
		4,
	},
#if WLAN_FEATURE_FASTPATH
	{
		HTT_DATA_MSG_SVC,
		PIPEDIR_IN,     /* in = DL = target -> host */
		5,
	},
#else /* WLAN_FEATURE_FASTPATH */
	{
		HTT_DATA_MSG_SVC,
		PIPEDIR_IN,  /* in = DL = target -> host */
		1,
	},
#endif /* WLAN_FEATURE_FASTPATH */

	/* (Additions here) */

	{                       /* Must be last */
		0,
		0,
		0,
	},
};


static struct service_to_pipe *target_service_to_ce_map =
	target_service_to_ce_map_wlan;
static int target_service_to_ce_map_sz = sizeof(target_service_to_ce_map_wlan);

static struct shadow_reg_cfg *target_shadow_reg_cfg = target_shadow_reg_cfg_map;
static int shadow_cfg_sz = sizeof(target_shadow_reg_cfg_map);

static struct service_to_pipe target_service_to_ce_map_wlan_epping[] = {
	{WMI_DATA_VO_SVC, PIPEDIR_OUT, 3,},     /* out = UL = host -> target */
	{WMI_DATA_VO_SVC, PIPEDIR_IN, 2,},      /* in = DL = target -> host */
	{WMI_DATA_BK_SVC, PIPEDIR_OUT, 4,},     /* out = UL = host -> target */
	{WMI_DATA_BK_SVC, PIPEDIR_IN, 1,},      /* in = DL = target -> host */
	{WMI_DATA_BE_SVC, PIPEDIR_OUT, 3,},     /* out = UL = host -> target */
	{WMI_DATA_BE_SVC, PIPEDIR_IN, 2,},      /* in = DL = target -> host */
	{WMI_DATA_VI_SVC, PIPEDIR_OUT, 3,},     /* out = UL = host -> target */
	{WMI_DATA_VI_SVC, PIPEDIR_IN, 2,},      /* in = DL = target -> host */
	{WMI_CONTROL_SVC, PIPEDIR_OUT, 3,},     /* out = UL = host -> target */
	{WMI_CONTROL_SVC, PIPEDIR_IN, 2,},      /* in = DL = target -> host */
	{HTC_CTRL_RSVD_SVC, PIPEDIR_OUT, 0,},   /* out = UL = host -> target */
	{HTC_CTRL_RSVD_SVC, PIPEDIR_IN, 2,},    /* in = DL = target -> host */
	{HTC_RAW_STREAMS_SVC, PIPEDIR_OUT, 0,}, /* out = UL = host -> target */
	{HTC_RAW_STREAMS_SVC, PIPEDIR_IN, 2,},  /* in = DL = target -> host */
	{HTT_DATA_MSG_SVC, PIPEDIR_OUT, 4,},    /* out = UL = host -> target */
	{HTT_DATA_MSG_SVC, PIPEDIR_IN, 1,},     /* in = DL = target -> host */
	{0, 0, 0,},             /* Must be last */
};

/**
 * ce_mark_datapath() - marks the ce_state->htt_rx_data accordingly
 * @ce_state : pointer to the state context of the CE
 *
 * Description:
 *   Sets htt_rx_data attribute of the state structure if the
 *   CE serves one of the HTT DATA services.
 *
 * Return:
 *  false (attribute set to false)
 *  true  (attribute set to true);
 */
bool ce_mark_datapath(struct CE_state *ce_state)
{
	struct service_to_pipe *svc_map;
	size_t map_sz;
	int    i;
	bool   rc = false;
	struct hif_opaque_softc *hif_hdl = GET_HIF_OPAQUE_HDL(ce_state->scn);
	struct hif_target_info *tgt_info = hif_get_target_info_handle(hif_hdl);

	if (ce_state != NULL) {
		if (QDF_IS_EPPING_ENABLED(hif_get_conparam(ce_state->scn))) {
			svc_map = target_service_to_ce_map_wlan_epping;
			map_sz = sizeof(target_service_to_ce_map_wlan_epping) /
				sizeof(struct service_to_pipe);
		} else {
			switch (tgt_info->target_type) {
			default:
				svc_map = target_service_to_ce_map_wlan;
				map_sz =
					sizeof(target_service_to_ce_map_wlan) /
					sizeof(struct service_to_pipe);
				break;
			case TARGET_TYPE_AR900B:
			case TARGET_TYPE_QCA9984:
			case TARGET_TYPE_IPQ4019:
			case TARGET_TYPE_QCA9888:
			case TARGET_TYPE_AR9888:
			case TARGET_TYPE_AR9888V2:
				svc_map = target_service_to_ce_map_ar900b;
				map_sz =
					sizeof(target_service_to_ce_map_ar900b)
					/ sizeof(struct service_to_pipe);
				break;
			}
		}
		for (i = 0; i < map_sz; i++) {
			if ((svc_map[i].pipenum == ce_state->id) &&
			    ((svc_map[i].service_id == HTT_DATA_MSG_SVC)  ||
			     (svc_map[i].service_id == HTT_DATA2_MSG_SVC) ||
			     (svc_map[i].service_id == HTT_DATA3_MSG_SVC))) {
				/* HTT CEs are unidirectional */
				if (svc_map[i].pipedir == PIPEDIR_IN)
					ce_state->htt_rx_data = true;
				else
					ce_state->htt_tx_data = true;
				rc = true;
			}
		}
	}
	return rc;
}

/**
 * ce_ring_test_initial_indexes() - tests the initial ce ring indexes
 * @ce_id: ce in question
 * @ring: ring state being examined
 * @type: "src_ring" or "dest_ring" string for identifying the ring
 *
 * Warns on non-zero index values.
 * Causes a kernel panic if the ring is not empty durring initialization.
 */
static void ce_ring_test_initial_indexes(int ce_id, struct CE_ring_state *ring,
					 char *type)
{
	if (ring->write_index != 0 || ring->sw_index != 0)
		HIF_ERROR("ce %d, %s, initial sw_index = %d, initial write_index =%d",
			  ce_id, type, ring->sw_index, ring->write_index);
	if (ring->write_index != ring->sw_index)
		QDF_BUG(0);
}

/*
 * Initialize a Copy Engine based on caller-supplied attributes.
 * This may be called once to initialize both source and destination
 * rings or it may be called twice for separate source and destination
 * initialization. It may be that only one side or the other is
 * initialized by software/firmware.
 *
 * This should be called durring the initialization sequence before
 * interupts are enabled, so we don't have to worry about thread safety.
 */
struct CE_handle *ce_init(struct hif_softc *scn,
			  unsigned int CE_id, struct CE_attr *attr)
{
	struct CE_state *CE_state;
	uint32_t ctrl_addr;
	unsigned int nentries;
	qdf_dma_addr_t base_addr;
	bool malloc_CE_state = false;
	bool malloc_src_ring = false;

	QDF_ASSERT(CE_id < scn->ce_count);
	ctrl_addr = CE_BASE_ADDRESS(CE_id);
	CE_state = scn->ce_id_to_state[CE_id];

	if (!CE_state) {
		CE_state =
		    (struct CE_state *)qdf_mem_malloc(sizeof(*CE_state));
		if (!CE_state) {
			HIF_ERROR("%s: CE_state has no mem", __func__);
			return NULL;
		}
		malloc_CE_state = true;
		qdf_mem_zero(CE_state, sizeof(*CE_state));
		scn->ce_id_to_state[CE_id] = CE_state;
		qdf_spinlock_create(&CE_state->ce_index_lock);

		CE_state->id = CE_id;
		CE_state->ctrl_addr = ctrl_addr;
		CE_state->state = CE_RUNNING;
		CE_state->attr_flags = attr->flags;
	}
	CE_state->scn = scn;

	qdf_atomic_init(&CE_state->rx_pending);
	if (attr == NULL) {
		/* Already initialized; caller wants the handle */
		return (struct CE_handle *)CE_state;
	}

	if (CE_state->src_sz_max)
		QDF_ASSERT(CE_state->src_sz_max == attr->src_sz_max);
	else
		CE_state->src_sz_max = attr->src_sz_max;

	ce_init_ce_desc_event_log(CE_id,
			attr->src_nentries + attr->dest_nentries);

	/* source ring setup */
	nentries = attr->src_nentries;
	if (nentries) {
		struct CE_ring_state *src_ring;
		unsigned CE_nbytes;
		char *ptr;
		uint64_t dma_addr;
		nentries = roundup_pwr2(nentries);
		if (CE_state->src_ring) {
			QDF_ASSERT(CE_state->src_ring->nentries == nentries);
		} else {
			CE_nbytes = sizeof(struct CE_ring_state)
				    + (nentries * sizeof(void *));
			ptr = qdf_mem_malloc(CE_nbytes);
			if (!ptr) {
				/* cannot allocate src ring. If the
				 * CE_state is allocated locally free
				 * CE_State and return error.
				 */
				HIF_ERROR("%s: src ring has no mem", __func__);
				if (malloc_CE_state) {
					/* allocated CE_state locally */
					scn->ce_id_to_state[CE_id] = NULL;
					qdf_mem_free(CE_state);
					malloc_CE_state = false;
				}
				return NULL;
			} else {
				/* we can allocate src ring.
				 * Mark that the src ring is
				 * allocated locally
				 */
				malloc_src_ring = true;
			}
			qdf_mem_zero(ptr, CE_nbytes);

			src_ring = CE_state->src_ring =
					   (struct CE_ring_state *)ptr;
			ptr += sizeof(struct CE_ring_state);
			src_ring->nentries = nentries;
			src_ring->nentries_mask = nentries - 1;
			if (Q_TARGET_ACCESS_BEGIN(scn) < 0)
				goto error_target_access;
			src_ring->hw_index =
				CE_SRC_RING_READ_IDX_GET_FROM_REGISTER(scn,
					ctrl_addr);
			src_ring->sw_index = src_ring->hw_index;
			src_ring->write_index =
				CE_SRC_RING_WRITE_IDX_GET_FROM_REGISTER(scn,
					ctrl_addr);

			ce_ring_test_initial_indexes(CE_id, src_ring,
						     "src_ring");

			if (Q_TARGET_ACCESS_END(scn) < 0)
				goto error_target_access;

			src_ring->low_water_mark_nentries = 0;
			src_ring->high_water_mark_nentries = nentries;
			src_ring->per_transfer_context = (void **)ptr;

			/* Legacy platforms that do not support cache
			 * coherent DMA are unsupported
			 */
			src_ring->base_addr_owner_space_unaligned =
				qdf_mem_alloc_consistent(scn->qdf_dev,
						scn->qdf_dev->dev,
						(nentries *
						sizeof(struct CE_src_desc) +
						CE_DESC_RING_ALIGN),
						&base_addr);
			if (src_ring->base_addr_owner_space_unaligned
					== NULL) {
				HIF_ERROR("%s: src ring has no DMA mem",
					  __func__);
				goto error_no_dma_mem;
			}
			src_ring->base_addr_CE_space_unaligned = base_addr;

			if (src_ring->
			    base_addr_CE_space_unaligned & (CE_DESC_RING_ALIGN
							- 1)) {
				src_ring->base_addr_CE_space =
					(src_ring->base_addr_CE_space_unaligned
					+ CE_DESC_RING_ALIGN -
					 1) & ~(CE_DESC_RING_ALIGN - 1);

				src_ring->base_addr_owner_space =
					(void
					 *)(((size_t) src_ring->
					     base_addr_owner_space_unaligned +
					     CE_DESC_RING_ALIGN -
					     1) & ~(CE_DESC_RING_ALIGN - 1));
			} else {
				src_ring->base_addr_CE_space =
					src_ring->base_addr_CE_space_unaligned;
				src_ring->base_addr_owner_space =
					src_ring->
					base_addr_owner_space_unaligned;
			}
			/*
			 * Also allocate a shadow src ring in
			 * regular mem to use for faster access.
			 */
			src_ring->shadow_base_unaligned =
				qdf_mem_malloc(nentries *
					       sizeof(struct CE_src_desc) +
					       CE_DESC_RING_ALIGN);
			if (src_ring->shadow_base_unaligned == NULL) {
				HIF_ERROR("%s: src ring no shadow_base mem",
					  __func__);
				goto error_no_dma_mem;
			}
			src_ring->shadow_base = (struct CE_src_desc *)
				(((size_t) src_ring->shadow_base_unaligned +
				CE_DESC_RING_ALIGN - 1) &
				 ~(CE_DESC_RING_ALIGN - 1));

			if (Q_TARGET_ACCESS_BEGIN(scn) < 0)
				goto error_target_access;
			dma_addr = src_ring->base_addr_CE_space;
			CE_SRC_RING_BASE_ADDR_SET(scn, ctrl_addr,
				 (uint32_t)(dma_addr & 0xFFFFFFFF));

			/* if SR_BA_ADDRESS_HIGH register exists */
			if (is_register_supported(SR_BA_ADDRESS_HIGH)) {
				uint32_t tmp;
				tmp = CE_SRC_RING_BASE_ADDR_HIGH_GET(
				   scn, ctrl_addr);
				tmp &= ~0x1F;
				dma_addr = ((dma_addr >> 32) & 0x1F)|tmp;
				CE_SRC_RING_BASE_ADDR_HIGH_SET(scn,
					 ctrl_addr, (uint32_t)dma_addr);
			}
			CE_SRC_RING_SZ_SET(scn, ctrl_addr, nentries);
			CE_SRC_RING_DMAX_SET(scn, ctrl_addr, attr->src_sz_max);
#ifdef BIG_ENDIAN_HOST
			/* Enable source ring byte swap for big endian host */
			CE_SRC_RING_BYTE_SWAP_SET(scn, ctrl_addr, 1);
#endif
			CE_SRC_RING_LOWMARK_SET(scn, ctrl_addr, 0);
			CE_SRC_RING_HIGHMARK_SET(scn, ctrl_addr, nentries);
			if (Q_TARGET_ACCESS_END(scn) < 0)
				goto error_target_access;
		}
	}

	/* destination ring setup */
	nentries = attr->dest_nentries;
	if (nentries) {
		struct CE_ring_state *dest_ring;
		unsigned CE_nbytes;
		char *ptr;
		uint64_t dma_addr;

		nentries = roundup_pwr2(nentries);
		if (CE_state->dest_ring) {
			QDF_ASSERT(CE_state->dest_ring->nentries == nentries);
		} else {
			CE_nbytes = sizeof(struct CE_ring_state)
				    + (nentries * sizeof(void *));
			ptr = qdf_mem_malloc(CE_nbytes);
			if (!ptr) {
				/* cannot allocate dst ring. If the CE_state
				 * or src ring is allocated locally free
				 * CE_State and src ring and return error.
				 */
				HIF_ERROR("%s: dest ring has no mem",
					  __func__);
				if (malloc_src_ring) {
					qdf_mem_free(CE_state->src_ring);
					CE_state->src_ring = NULL;
					malloc_src_ring = false;
				}
				if (malloc_CE_state) {
					/* allocated CE_state locally */
					scn->ce_id_to_state[CE_id] = NULL;
					qdf_mem_free(CE_state);
					malloc_CE_state = false;
				}
				return NULL;
			}
			qdf_mem_zero(ptr, CE_nbytes);

			dest_ring = CE_state->dest_ring =
					    (struct CE_ring_state *)ptr;
			ptr += sizeof(struct CE_ring_state);
			dest_ring->nentries = nentries;
			dest_ring->nentries_mask = nentries - 1;
			if (Q_TARGET_ACCESS_BEGIN(scn) < 0)
				goto error_target_access;
			dest_ring->sw_index =
				CE_DEST_RING_READ_IDX_GET_FROM_REGISTER(scn,
					ctrl_addr);
			dest_ring->write_index =
				CE_DEST_RING_WRITE_IDX_GET_FROM_REGISTER(scn,
					ctrl_addr);

			ce_ring_test_initial_indexes(CE_id, dest_ring,
						     "dest_ring");

			if (Q_TARGET_ACCESS_END(scn) < 0)
				goto error_target_access;

			dest_ring->low_water_mark_nentries = 0;
			dest_ring->high_water_mark_nentries = nentries;
			dest_ring->per_transfer_context = (void **)ptr;

			/* Legacy platforms that do not support cache
			 * coherent DMA are unsupported */
			dest_ring->base_addr_owner_space_unaligned =
				qdf_mem_alloc_consistent(scn->qdf_dev,
						scn->qdf_dev->dev,
						(nentries *
						sizeof(struct CE_dest_desc) +
						CE_DESC_RING_ALIGN),
						&base_addr);
			if (dest_ring->base_addr_owner_space_unaligned
				== NULL) {
				HIF_ERROR("%s: dest ring has no DMA mem",
					  __func__);
				goto error_no_dma_mem;
			}
			dest_ring->base_addr_CE_space_unaligned = base_addr;

			/* Correctly initialize memory to 0 to
			 * prevent garbage data crashing system
			 * when download firmware
			 */
			qdf_mem_zero(dest_ring->base_addr_owner_space_unaligned,
				  nentries * sizeof(struct CE_dest_desc) +
				  CE_DESC_RING_ALIGN);

			if (dest_ring->
			    base_addr_CE_space_unaligned & (CE_DESC_RING_ALIGN -
							    1)) {

				dest_ring->base_addr_CE_space =
					(dest_ring->
					 base_addr_CE_space_unaligned +
					 CE_DESC_RING_ALIGN -
					 1) & ~(CE_DESC_RING_ALIGN - 1);

				dest_ring->base_addr_owner_space =
					(void
					 *)(((size_t) dest_ring->
					     base_addr_owner_space_unaligned +
					     CE_DESC_RING_ALIGN -
					     1) & ~(CE_DESC_RING_ALIGN - 1));
			} else {
				dest_ring->base_addr_CE_space =
					dest_ring->base_addr_CE_space_unaligned;
				dest_ring->base_addr_owner_space =
					dest_ring->
					base_addr_owner_space_unaligned;
			}

			if (Q_TARGET_ACCESS_BEGIN(scn) < 0)
				goto error_target_access;
			dma_addr = dest_ring->base_addr_CE_space;
			CE_DEST_RING_BASE_ADDR_SET(scn, ctrl_addr,
				 (uint32_t)(dma_addr & 0xFFFFFFFF));

			/* if DR_BA_ADDRESS_HIGH exists */
			if (is_register_supported(DR_BA_ADDRESS_HIGH)) {
				uint32_t tmp;
				tmp = CE_DEST_RING_BASE_ADDR_HIGH_GET(scn,
						ctrl_addr);
				tmp &= ~0x1F;
				dma_addr = ((dma_addr >> 32) & 0x1F)|tmp;
				CE_DEST_RING_BASE_ADDR_HIGH_SET(scn,
					ctrl_addr, (uint32_t)dma_addr);
			}

			CE_DEST_RING_SZ_SET(scn, ctrl_addr, nentries);
#ifdef BIG_ENDIAN_HOST
			/* Enable Dest ring byte swap for big endian host */
			CE_DEST_RING_BYTE_SWAP_SET(scn, ctrl_addr, 1);
#endif
			CE_DEST_RING_LOWMARK_SET(scn, ctrl_addr, 0);
			CE_DEST_RING_HIGHMARK_SET(scn, ctrl_addr, nentries);
			if (Q_TARGET_ACCESS_END(scn) < 0)
				goto error_target_access;

			/* epping */
			/* poll timer */
			if ((CE_state->attr_flags & CE_ATTR_ENABLE_POLL)) {
				qdf_timer_init(scn->qdf_dev,
						       &CE_state->poll_timer,
						       ce_poll_timeout,
						       CE_state,
						       QDF_TIMER_TYPE_SW);
				CE_state->timer_inited = true;
				qdf_timer_mod(&CE_state->poll_timer,
						      CE_POLL_TIMEOUT);
			}
		}
	}

	/* Enable CE error interrupts */
	if (Q_TARGET_ACCESS_BEGIN(scn) < 0)
		goto error_target_access;
	CE_ERROR_INTR_ENABLE(scn, ctrl_addr);
	if (Q_TARGET_ACCESS_END(scn) < 0)
		goto error_target_access;

	/* update the htt_data attribute */
	ce_mark_datapath(CE_state);

	return (struct CE_handle *)CE_state;

error_target_access:
error_no_dma_mem:
	ce_fini((struct CE_handle *)CE_state);
	return NULL;
}

#ifdef WLAN_FEATURE_FASTPATH
/**
 * hif_enable_fastpath() Update that we have enabled fastpath mode
 * @hif_ctx: HIF context
 *
 * For use in data path
 *
 * Retrun: void
 */
void hif_enable_fastpath(struct hif_opaque_softc *hif_ctx)
{
	struct hif_softc *scn = HIF_GET_SOFTC(hif_ctx);

	HIF_INFO("%s, Enabling fastpath mode", __func__);
	scn->fastpath_mode_on = true;
}

/**
 * hif_is_fastpath_mode_enabled - API to query if fasthpath mode is enabled
 * @hif_ctx: HIF Context
 *
 * For use in data path to skip HTC
 *
 * Return: bool
 */
bool hif_is_fastpath_mode_enabled(struct hif_opaque_softc *hif_ctx)
{
	struct hif_softc *scn = HIF_GET_SOFTC(hif_ctx);

	return scn->fastpath_mode_on;
}

/**
 * hif_get_ce_handle - API to get CE handle for FastPath mode
 * @hif_ctx: HIF Context
 * @id: CopyEngine Id
 *
 * API to return CE handle for fastpath mode
 *
 * Return: void
 */
void *hif_get_ce_handle(struct hif_opaque_softc *hif_ctx, int id)
{
	struct hif_softc *scn = HIF_GET_SOFTC(hif_ctx);

	return scn->ce_id_to_state[id];
}

/**
 * ce_h2t_tx_ce_cleanup() Place holder function for H2T CE cleanup.
 * No processing is required inside this function.
 * @ce_hdl: Cope engine handle
 * Using an assert, this function makes sure that,
 * the TX CE has been processed completely.
 *
 * This is called while dismantling CE structures. No other thread
 * should be using these structures while dismantling is occuring
 * therfore no locking is needed.
 *
 * Return: none
 */
void
ce_h2t_tx_ce_cleanup(struct CE_handle *ce_hdl)
{
	struct CE_state *ce_state = (struct CE_state *)ce_hdl;
	struct CE_ring_state *src_ring = ce_state->src_ring;
	struct hif_softc *sc = ce_state->scn;
	uint32_t sw_index, write_index;
	if (hif_is_nss_wifi_enabled(sc))
		return;

	if (sc->fastpath_mode_on && ce_state->htt_tx_data) {
		HIF_INFO("%s %d Fastpath mode ON, Cleaning up HTT Tx CE",
			 __func__, __LINE__);
		sw_index = src_ring->sw_index;
		write_index = src_ring->sw_index;

		/* At this point Tx CE should be clean */
		qdf_assert_always(sw_index == write_index);
	}
}

/**
 * ce_t2h_msg_ce_cleanup() - Cleanup buffers on the t2h datapath msg queue.
 * @ce_hdl: Handle to CE
 *
 * These buffers are never allocated on the fly, but
 * are allocated only once during HIF start and freed
 * only once during HIF stop.
 * NOTE:
 * The assumption here is there is no in-flight DMA in progress
 * currently, so that buffers can be freed up safely.
 *
 * Return: NONE
 */
void ce_t2h_msg_ce_cleanup(struct CE_handle *ce_hdl)
{
	struct CE_state *ce_state = (struct CE_state *)ce_hdl;
	struct CE_ring_state *dst_ring = ce_state->dest_ring;
	qdf_nbuf_t nbuf;
	int i;

	if (!ce_state->fastpath_handler)
		return;
	/*
	 * when fastpath_mode is on and for datapath CEs. Unlike other CE's,
	 * this CE is completely full: does not leave one blank space, to
	 * distinguish between empty queue & full queue. So free all the
	 * entries.
	 */
	for (i = 0; i < dst_ring->nentries; i++) {
		nbuf = dst_ring->per_transfer_context[i];

		/*
		 * The reasons for doing this check are:
		 * 1) Protect against calling cleanup before allocating buffers
		 * 2) In a corner case, FASTPATH_mode_on may be set, but we
		 *    could have a partially filled ring, because of a memory
		 *    allocation failure in the middle of allocating ring.
		 *    This check accounts for that case, checking
		 *    fastpath_mode_on flag or started flag would not have
		 *    covered that case. This is not in performance path,
		 *    so OK to do this.
		 */
		if (nbuf)
			qdf_nbuf_free(nbuf);
	}
}

/**
 * hif_update_fastpath_recv_bufs_cnt() - Increments the Rx buf count by 1
 * @scn: HIF handle
 *
 * Datapath Rx CEs are special case, where we reuse all the message buffers.
 * Hence we have to post all the entries in the pipe, even, in the beginning
 * unlike for other CE pipes where one less than dest_nentries are filled in
 * the beginning.
 *
 * Return: None
 */
static void hif_update_fastpath_recv_bufs_cnt(struct hif_softc *scn)
{
	int pipe_num;
	struct HIF_CE_state *hif_state = HIF_GET_CE_STATE(scn);

	if (scn->fastpath_mode_on == false)
		return;

	for (pipe_num = 0; pipe_num < scn->ce_count; pipe_num++) {
		struct HIF_CE_pipe_info *pipe_info =
			&hif_state->pipe_info[pipe_num];
		struct CE_state *ce_state =
			scn->ce_id_to_state[pipe_info->pipe_num];

		if (ce_state->htt_rx_data)
			atomic_inc(&pipe_info->recv_bufs_needed);
	}
}
#else
static inline void hif_update_fastpath_recv_bufs_cnt(struct hif_softc *scn)
{
}

static inline bool ce_is_fastpath_enabled(struct hif_softc *scn)
{
	return false;
}

static inline bool ce_is_fastpath_handler_registered(struct CE_state *ce_state)
{
	return false;
}
#endif /* WLAN_FEATURE_FASTPATH */

void ce_fini(struct CE_handle *copyeng)
{
	struct CE_state *CE_state = (struct CE_state *)copyeng;
	unsigned int CE_id = CE_state->id;
	struct hif_softc *scn = CE_state->scn;

	CE_state->state = CE_UNUSED;
	scn->ce_id_to_state[CE_id] = NULL;
	if (CE_state->src_ring) {
		/* Cleanup the datapath Tx ring */
		ce_h2t_tx_ce_cleanup(copyeng);

		if (CE_state->src_ring->shadow_base_unaligned)
			qdf_mem_free(CE_state->src_ring->shadow_base_unaligned);
		if (CE_state->src_ring->base_addr_owner_space_unaligned)
			qdf_mem_free_consistent(scn->qdf_dev,
						scn->qdf_dev->dev,
					    (CE_state->src_ring->nentries *
					     sizeof(struct CE_src_desc) +
					     CE_DESC_RING_ALIGN),
					    CE_state->src_ring->
					    base_addr_owner_space_unaligned,
					    CE_state->src_ring->
					    base_addr_CE_space, 0);
		qdf_mem_free(CE_state->src_ring);
	}
	if (CE_state->dest_ring) {
		/* Cleanup the datapath Rx ring */
		ce_t2h_msg_ce_cleanup(copyeng);

		if (CE_state->dest_ring->base_addr_owner_space_unaligned)
			qdf_mem_free_consistent(scn->qdf_dev,
						scn->qdf_dev->dev,
					    (CE_state->dest_ring->nentries *
					     sizeof(struct CE_dest_desc) +
					     CE_DESC_RING_ALIGN),
					    CE_state->dest_ring->
					    base_addr_owner_space_unaligned,
					    CE_state->dest_ring->
					    base_addr_CE_space, 0);
		qdf_mem_free(CE_state->dest_ring);

		/* epping */
		if (CE_state->timer_inited) {
			CE_state->timer_inited = false;
			qdf_timer_free(&CE_state->poll_timer);
		}
	}
	qdf_mem_free(CE_state);
}

void hif_detach_htc(struct hif_opaque_softc *hif_ctx)
{
	struct HIF_CE_state *hif_state = HIF_GET_CE_STATE(hif_ctx);

	qdf_mem_zero(&hif_state->msg_callbacks_pending,
		  sizeof(hif_state->msg_callbacks_pending));
	qdf_mem_zero(&hif_state->msg_callbacks_current,
		  sizeof(hif_state->msg_callbacks_current));
}

/* Send the first nbytes bytes of the buffer */
QDF_STATUS
hif_send_head(struct hif_opaque_softc *hif_ctx,
	      uint8_t pipe, unsigned int transfer_id, unsigned int nbytes,
	      qdf_nbuf_t nbuf, unsigned int data_attr)
{
	struct hif_softc *scn = HIF_GET_SOFTC(hif_ctx);
	struct HIF_CE_state *hif_state = HIF_GET_CE_STATE(hif_ctx);
	struct HIF_CE_pipe_info *pipe_info = &(hif_state->pipe_info[pipe]);
	struct CE_handle *ce_hdl = pipe_info->ce_hdl;
	int bytes = nbytes, nfrags = 0;
	struct ce_sendlist sendlist;
	int status, i = 0;
	unsigned int mux_id = 0;

	QDF_ASSERT(nbytes <= qdf_nbuf_len(nbuf));

	transfer_id =
		(mux_id & MUX_ID_MASK) |
		(transfer_id & TRANSACTION_ID_MASK);
	data_attr &= DESC_DATA_FLAG_MASK;
	/*
	 * The common case involves sending multiple fragments within a
	 * single download (the tx descriptor and the tx frame header).
	 * So, optimize for the case of multiple fragments by not even
	 * checking whether it's necessary to use a sendlist.
	 * The overhead of using a sendlist for a single buffer download
	 * is not a big deal, since it happens rarely (for WMI messages).
	 */
	ce_sendlist_init(&sendlist);
	do {
		qdf_dma_addr_t frag_paddr;
		int frag_bytes;

		frag_paddr = qdf_nbuf_get_frag_paddr(nbuf, nfrags);
		frag_bytes = qdf_nbuf_get_frag_len(nbuf, nfrags);
		/*
		 * Clear the packet offset for all but the first CE desc.
		 */
		if (i++ > 0)
			data_attr &= ~QDF_CE_TX_PKT_OFFSET_BIT_M;

		status = ce_sendlist_buf_add(&sendlist, frag_paddr,
				    frag_bytes >
				    bytes ? bytes : frag_bytes,
				    qdf_nbuf_get_frag_is_wordstream
				    (nbuf,
				    nfrags) ? 0 :
				    CE_SEND_FLAG_SWAP_DISABLE,
				    data_attr);
		if (status != QDF_STATUS_SUCCESS) {
			HIF_ERROR("%s: error, frag_num %d larger than limit",
				__func__, nfrags);
			return status;
		}
		bytes -= frag_bytes;
		nfrags++;
	} while (bytes > 0);

	/* Make sure we have resources to handle this request */
	qdf_spin_lock_bh(&pipe_info->completion_freeq_lock);
	if (pipe_info->num_sends_allowed < nfrags) {
		qdf_spin_unlock_bh(&pipe_info->completion_freeq_lock);
		ce_pkt_error_count_incr(hif_state, HIF_PIPE_NO_RESOURCE);
		return QDF_STATUS_E_RESOURCES;
	}
	pipe_info->num_sends_allowed -= nfrags;
	qdf_spin_unlock_bh(&pipe_info->completion_freeq_lock);

	if (qdf_unlikely(ce_hdl == NULL)) {
		HIF_ERROR("%s: error CE handle is null", __func__);
		return A_ERROR;
	}

	QDF_NBUF_UPDATE_TX_PKT_COUNT(nbuf, QDF_NBUF_TX_PKT_HIF);
	DPTRACE(qdf_dp_trace(nbuf, QDF_DP_TRACE_HIF_PACKET_PTR_RECORD,
				qdf_nbuf_data_addr(nbuf),
				sizeof(qdf_nbuf_data(nbuf)), QDF_TX));
	status = ce_sendlist_send(ce_hdl, nbuf, &sendlist, transfer_id);
	QDF_ASSERT(status == QDF_STATUS_SUCCESS);

	return status;
}

void hif_send_complete_check(struct hif_opaque_softc *hif_ctx, uint8_t pipe,
								int force)
{
	struct hif_softc *scn = HIF_GET_SOFTC(hif_ctx);

	if (!force) {
		int resources;
		/*
		 * Decide whether to actually poll for completions, or just
		 * wait for a later chance. If there seem to be plenty of
		 * resources left, then just wait, since checking involves
		 * reading a CE register, which is a relatively expensive
		 * operation.
		 */
		resources = hif_get_free_queue_number(hif_ctx, pipe);
		/*
		 * If at least 50% of the total resources are still available,
		 * don't bother checking again yet.
		 */
		if (resources > (host_ce_config[pipe].src_nentries >> 1)) {
			return;
		}
	}
#if ATH_11AC_TXCOMPACT
	ce_per_engine_servicereap(scn, pipe);
#else
	ce_per_engine_service(scn, pipe);
#endif
}

uint16_t
hif_get_free_queue_number(struct hif_opaque_softc *hif_ctx, uint8_t pipe)
{
	struct HIF_CE_state *hif_state = HIF_GET_CE_STATE(hif_ctx);
	struct HIF_CE_pipe_info *pipe_info = &(hif_state->pipe_info[pipe]);
	uint16_t rv;

	qdf_spin_lock_bh(&pipe_info->completion_freeq_lock);
	rv = pipe_info->num_sends_allowed;
	qdf_spin_unlock_bh(&pipe_info->completion_freeq_lock);
	return rv;
}

/* Called by lower (CE) layer when a send to Target completes. */
void
hif_pci_ce_send_done(struct CE_handle *copyeng, void *ce_context,
		     void *transfer_context, qdf_dma_addr_t CE_data,
		     unsigned int nbytes, unsigned int transfer_id,
		     unsigned int sw_index, unsigned int hw_index,
		     unsigned int toeplitz_hash_result)
{
	struct HIF_CE_pipe_info *pipe_info =
		(struct HIF_CE_pipe_info *)ce_context;
	struct HIF_CE_state *hif_state = pipe_info->HIF_CE_state;
	struct hif_softc *scn = HIF_GET_SOFTC(hif_state);
	unsigned int sw_idx = sw_index, hw_idx = hw_index;
	struct hif_msg_callbacks *msg_callbacks =
		&hif_state->msg_callbacks_current;

	do {
		/*
		 * The upper layer callback will be triggered
		 * when last fragment is complteted.
		 */
		if (transfer_context != CE_SENDLIST_ITEM_CTXT) {
			if (scn->target_status == TARGET_STATUS_RESET)
				qdf_nbuf_free(transfer_context);
			else
				msg_callbacks->txCompletionHandler(
					msg_callbacks->Context,
					transfer_context, transfer_id,
					toeplitz_hash_result);
		}

		qdf_spin_lock(&pipe_info->completion_freeq_lock);
		pipe_info->num_sends_allowed++;
		qdf_spin_unlock(&pipe_info->completion_freeq_lock);
	} while (ce_completed_send_next(copyeng,
			&ce_context, &transfer_context,
			&CE_data, &nbytes, &transfer_id,
			&sw_idx, &hw_idx,
			&toeplitz_hash_result) == QDF_STATUS_SUCCESS);
}

/**
 * hif_ce_do_recv(): send message from copy engine to upper layers
 * @msg_callbacks: structure containing callback and callback context
 * @netbuff: skb containing message
 * @nbytes: number of bytes in the message
 * @pipe_info: used for the pipe_number info
 *
 * Checks the packet length, configures the lenght in the netbuff,
 * and calls the upper layer callback.
 *
 * return: None
 */
static inline void hif_ce_do_recv(struct hif_msg_callbacks *msg_callbacks,
		qdf_nbuf_t netbuf, int nbytes,
		struct HIF_CE_pipe_info *pipe_info) {
	if (nbytes <= pipe_info->buf_sz) {
		qdf_nbuf_set_pktlen(netbuf, nbytes);
		msg_callbacks->
			rxCompletionHandler(msg_callbacks->Context,
					netbuf, pipe_info->pipe_num);
	} else {
		HIF_ERROR("%s: Invalid Rx msg buf:%p nbytes:%d",
				__func__, netbuf, nbytes);
		qdf_nbuf_free(netbuf);
	}
}

/* Called by lower (CE) layer when data is received from the Target. */
void
hif_pci_ce_recv_data(struct CE_handle *copyeng, void *ce_context,
		     void *transfer_context, qdf_dma_addr_t CE_data,
		     unsigned int nbytes, unsigned int transfer_id,
		     unsigned int flags)
{
	struct HIF_CE_pipe_info *pipe_info =
		(struct HIF_CE_pipe_info *)ce_context;
	struct HIF_CE_state *hif_state = pipe_info->HIF_CE_state;
	struct CE_state *ce_state = (struct CE_state *) copyeng;
	struct hif_softc *scn = HIF_GET_SOFTC(hif_state);
#ifdef HIF_PCI
	struct hif_pci_softc *hif_pci_sc = HIF_GET_PCI_SOFTC(hif_state);
#endif
	struct hif_msg_callbacks *msg_callbacks =
		&hif_state->msg_callbacks_current;

	do {
#ifdef HIF_PCI
		hif_pm_runtime_mark_last_busy(hif_pci_sc->dev);
#endif
		qdf_nbuf_unmap_single(scn->qdf_dev,
				      (qdf_nbuf_t) transfer_context,
				      QDF_DMA_FROM_DEVICE);

		atomic_inc(&pipe_info->recv_bufs_needed);
		hif_post_recv_buffers_for_pipe(pipe_info);
		if (scn->target_status == TARGET_STATUS_RESET)
			qdf_nbuf_free(transfer_context);
		else
			hif_ce_do_recv(msg_callbacks, transfer_context,
				nbytes, pipe_info);

		/* Set up force_break flag if num of receices reaches
		 * MAX_NUM_OF_RECEIVES */
		ce_state->receive_count++;
		if (qdf_unlikely(hif_ce_service_should_yield(scn, ce_state))) {
			ce_state->force_break = 1;
			break;
		}
	} while (ce_completed_recv_next(copyeng, &ce_context, &transfer_context,
					&CE_data, &nbytes, &transfer_id,
					&flags) == QDF_STATUS_SUCCESS);

}

/* TBDXXX: Set CE High Watermark; invoke txResourceAvailHandler in response */

void
hif_post_init(struct hif_opaque_softc *hif_ctx, void *unused,
	      struct hif_msg_callbacks *callbacks)
{
	struct HIF_CE_state *hif_state = HIF_GET_CE_STATE(hif_ctx);

#ifdef CONFIG_ATH_PCIE_ACCESS_DEBUG
	spin_lock_init(&pcie_access_log_lock);
#endif
	/* Save callbacks for later installation */
	qdf_mem_copy(&hif_state->msg_callbacks_pending, callbacks,
		 sizeof(hif_state->msg_callbacks_pending));

}

int hif_completion_thread_startup(struct HIF_CE_state *hif_state)
{
	struct CE_handle *ce_diag = hif_state->ce_diag;
	int pipe_num;
	struct hif_softc *scn = HIF_GET_SOFTC(hif_state);
	struct hif_msg_callbacks *hif_msg_callbacks =
		&hif_state->msg_callbacks_current;

	/* daemonize("hif_compl_thread"); */

	if (scn->ce_count == 0) {
		HIF_ERROR("%s: Invalid ce_count", __func__);
		return -EINVAL;
	}

	if (!hif_msg_callbacks ||
			!hif_msg_callbacks->rxCompletionHandler ||
			!hif_msg_callbacks->txCompletionHandler) {
		HIF_ERROR("%s: no completion handler registered", __func__);
		return -EFAULT;
	}

	A_TARGET_ACCESS_LIKELY(scn);
	for (pipe_num = 0; pipe_num < scn->ce_count; pipe_num++) {
		struct CE_attr attr;
		struct HIF_CE_pipe_info *pipe_info;

		pipe_info = &hif_state->pipe_info[pipe_num];
		if (pipe_info->ce_hdl == ce_diag) {
			continue;       /* Handle Diagnostic CE specially */
		}
		attr = host_ce_config[pipe_num];
		if (attr.src_nentries) {
			/* pipe used to send to target */
			HIF_INFO_MED("%s: pipe_num:%d pipe_info:0x%p",
					 __func__, pipe_num, pipe_info);
			ce_send_cb_register(pipe_info->ce_hdl,
					    hif_pci_ce_send_done, pipe_info,
					    attr.flags & CE_ATTR_DISABLE_INTR);
			pipe_info->num_sends_allowed = attr.src_nentries - 1;
		}
		if (attr.dest_nentries) {
			/* pipe used to receive from target */
			ce_recv_cb_register(pipe_info->ce_hdl,
					    hif_pci_ce_recv_data, pipe_info,
					    attr.flags & CE_ATTR_DISABLE_INTR);
		}

		if (attr.src_nentries)
			qdf_spinlock_create(&pipe_info->completion_freeq_lock);
	}

	A_TARGET_ACCESS_UNLIKELY(scn);
	return 0;
}

/*
 * Install pending msg callbacks.
 *
 * TBDXXX: This hack is needed because upper layers install msg callbacks
 * for use with HTC before BMI is done; yet this HIF implementation
 * needs to continue to use BMI msg callbacks. Really, upper layers
 * should not register HTC callbacks until AFTER BMI phase.
 */
static void hif_msg_callbacks_install(struct hif_softc *scn)
{
	struct HIF_CE_state *hif_state = HIF_GET_CE_STATE(scn);

	qdf_mem_copy(&hif_state->msg_callbacks_current,
		 &hif_state->msg_callbacks_pending,
		 sizeof(hif_state->msg_callbacks_pending));
}

void hif_get_default_pipe(struct hif_opaque_softc *hif_hdl, uint8_t *ULPipe,
							uint8_t *DLPipe)
{
	int ul_is_polled, dl_is_polled;

	(void)hif_map_service_to_pipe(hif_hdl, HTC_CTRL_RSVD_SVC,
		ULPipe, DLPipe, &ul_is_polled, &dl_is_polled);
}

/**
 * hif_dump_pipe_debug_count() - Log error count
 * @scn: hif_softc pointer.
 *
 * Output the pipe error counts of each pipe to log file
 *
 * Return: N/A
 */
void hif_dump_pipe_debug_count(struct hif_softc *scn)
{
	struct HIF_CE_state *hif_state = HIF_GET_CE_STATE(scn);
	int pipe_num;

	if (hif_state == NULL) {
		HIF_ERROR("%s hif_state is NULL", __func__);
		return;
	}
	for (pipe_num = 0; pipe_num < scn->ce_count; pipe_num++) {
		struct HIF_CE_pipe_info *pipe_info;

	pipe_info = &hif_state->pipe_info[pipe_num];

	if (pipe_info->nbuf_alloc_err_count > 0 ||
			pipe_info->nbuf_dma_err_count > 0 ||
			pipe_info->nbuf_ce_enqueue_err_count)
		HIF_ERROR(
			"%s: pipe_id = %d, recv_bufs_needed = %d, nbuf_alloc_err_count = %u, nbuf_dma_err_count = %u, nbuf_ce_enqueue_err_count = %u",
			__func__, pipe_info->pipe_num,
			atomic_read(&pipe_info->recv_bufs_needed),
			pipe_info->nbuf_alloc_err_count,
			pipe_info->nbuf_dma_err_count,
			pipe_info->nbuf_ce_enqueue_err_count);
	}
}

static int hif_post_recv_buffers_for_pipe(struct HIF_CE_pipe_info *pipe_info)
{
	struct CE_handle *ce_hdl;
	qdf_size_t buf_sz;
	struct hif_softc *scn = HIF_GET_SOFTC(pipe_info->HIF_CE_state);
	QDF_STATUS ret;
	uint32_t bufs_posted = 0;

	buf_sz = pipe_info->buf_sz;
	if (buf_sz == 0) {
		/* Unused Copy Engine */
		return 0;
	}

	ce_hdl = pipe_info->ce_hdl;

	qdf_spin_lock_bh(&pipe_info->recv_bufs_needed_lock);
	while (atomic_read(&pipe_info->recv_bufs_needed) > 0) {
		qdf_dma_addr_t CE_data;      /* CE space buffer address */
		qdf_nbuf_t nbuf;
		int status;

		atomic_dec(&pipe_info->recv_bufs_needed);
		qdf_spin_unlock_bh(&pipe_info->recv_bufs_needed_lock);

		nbuf = qdf_nbuf_alloc(scn->qdf_dev, buf_sz, 0, 4, false);
		if (!nbuf) {
			qdf_spin_lock_bh(&pipe_info->recv_bufs_needed_lock);
			pipe_info->nbuf_alloc_err_count++;
			qdf_spin_unlock_bh(
				&pipe_info->recv_bufs_needed_lock);
			HIF_ERROR(
				"%s buf alloc error [%d] needed %d, nbuf_alloc_err_count = %u",
				 __func__, pipe_info->pipe_num,
				 atomic_read(&pipe_info->recv_bufs_needed),
				pipe_info->nbuf_alloc_err_count);
			atomic_inc(&pipe_info->recv_bufs_needed);
			return 1;
		}

		/*
		 * qdf_nbuf_peek_header(nbuf, &data, &unused);
		 * CE_data = dma_map_single(dev, data, buf_sz, );
		 * DMA_FROM_DEVICE);
		 */
		ret =
			qdf_nbuf_map_single(scn->qdf_dev, nbuf,
					    QDF_DMA_FROM_DEVICE);

		if (unlikely(ret != QDF_STATUS_SUCCESS)) {
			qdf_spin_lock_bh(&pipe_info->recv_bufs_needed_lock);
			pipe_info->nbuf_dma_err_count++;
			qdf_spin_unlock_bh(&pipe_info->recv_bufs_needed_lock);
			HIF_ERROR(
				"%s buf alloc error [%d] needed %d, nbuf_dma_err_count = %u",
				 __func__, pipe_info->pipe_num,
				 atomic_read(&pipe_info->recv_bufs_needed),
				pipe_info->nbuf_dma_err_count);
			qdf_nbuf_free(nbuf);
			atomic_inc(&pipe_info->recv_bufs_needed);
			return 1;
		}

		CE_data = qdf_nbuf_get_frag_paddr(nbuf, 0);

		qdf_mem_dma_sync_single_for_device(scn->qdf_dev, CE_data,
					       buf_sz, DMA_FROM_DEVICE);
		status = ce_recv_buf_enqueue(ce_hdl, (void *)nbuf, CE_data);
		QDF_ASSERT(status == QDF_STATUS_SUCCESS);
		if (status != EOK) {
			qdf_spin_lock_bh(&pipe_info->recv_bufs_needed_lock);
			pipe_info->nbuf_ce_enqueue_err_count++;
			qdf_spin_unlock_bh(&pipe_info->recv_bufs_needed_lock);
			HIF_ERROR(
				"%s buf alloc error [%d] needed %d, nbuf_alloc_err_count = %u",
				__func__, pipe_info->pipe_num,
				atomic_read(&pipe_info->recv_bufs_needed),
				pipe_info->nbuf_ce_enqueue_err_count);
			atomic_inc(&pipe_info->recv_bufs_needed);
			qdf_nbuf_free(nbuf);
			return 1;
		}

		qdf_spin_lock_bh(&pipe_info->recv_bufs_needed_lock);
		bufs_posted++;
	}
	pipe_info->nbuf_alloc_err_count =
		(pipe_info->nbuf_alloc_err_count > bufs_posted) ?
		pipe_info->nbuf_alloc_err_count - bufs_posted : 0;
	pipe_info->nbuf_dma_err_count =
		(pipe_info->nbuf_dma_err_count > bufs_posted) ?
		pipe_info->nbuf_dma_err_count - bufs_posted : 0;
	pipe_info->nbuf_ce_enqueue_err_count =
		(pipe_info->nbuf_ce_enqueue_err_count > bufs_posted) ?
	     pipe_info->nbuf_ce_enqueue_err_count - bufs_posted : 0;

	qdf_spin_unlock_bh(&pipe_info->recv_bufs_needed_lock);

	return 0;
}

/*
 * Try to post all desired receive buffers for all pipes.
 * Returns 0 if all desired buffers are posted,
 * non-zero if were were unable to completely
 * replenish receive buffers.
 */
static int hif_post_recv_buffers(struct hif_softc *scn)
{
	struct HIF_CE_state *hif_state = HIF_GET_CE_STATE(scn);
	int pipe_num, rv = 0;
	struct CE_state *ce_state;

	A_TARGET_ACCESS_LIKELY(scn);
	for (pipe_num = 0; pipe_num < scn->ce_count; pipe_num++) {
		struct HIF_CE_pipe_info *pipe_info;
		ce_state = scn->ce_id_to_state[pipe_num];
		pipe_info = &hif_state->pipe_info[pipe_num];

		if (hif_is_nss_wifi_enabled(scn) &&
		    ce_state && (ce_state->htt_rx_data)) {
			continue;
		}

		if (hif_post_recv_buffers_for_pipe(pipe_info)) {
			rv = 1;
			goto done;
		}
	}

done:
	A_TARGET_ACCESS_UNLIKELY(scn);

	return rv;
}

QDF_STATUS hif_start(struct hif_opaque_softc *hif_ctx)
{
	struct hif_softc *scn = HIF_GET_SOFTC(hif_ctx);
	struct HIF_CE_state *hif_state = HIF_GET_CE_STATE(scn);

	hif_update_fastpath_recv_bufs_cnt(scn);

	hif_msg_callbacks_install(scn);

	if (hif_completion_thread_startup(hif_state))
		return QDF_STATUS_E_FAILURE;

	/* Post buffers once to start things off. */
	(void)hif_post_recv_buffers(scn);

	hif_state->started = true;

	return QDF_STATUS_SUCCESS;
}

void hif_recv_buffer_cleanup_on_pipe(struct HIF_CE_pipe_info *pipe_info)
{
	struct hif_softc *scn;
	struct CE_handle *ce_hdl;
	uint32_t buf_sz;
	struct HIF_CE_state *hif_state;
	qdf_nbuf_t netbuf;
	qdf_dma_addr_t CE_data;
	void *per_CE_context;

	buf_sz = pipe_info->buf_sz;
	if (buf_sz == 0) {
		/* Unused Copy Engine */
		return;
	}

	hif_state = pipe_info->HIF_CE_state;
	if (!hif_state->started) {
		return;
	}

	scn = HIF_GET_SOFTC(hif_state);
	ce_hdl = pipe_info->ce_hdl;

	if (scn->qdf_dev == NULL) {
		return;
	}
	while (ce_revoke_recv_next
		       (ce_hdl, &per_CE_context, (void **)&netbuf,
			&CE_data) == QDF_STATUS_SUCCESS) {
		qdf_nbuf_unmap_single(scn->qdf_dev, netbuf,
				      QDF_DMA_FROM_DEVICE);
		qdf_nbuf_free(netbuf);
	}
}

void hif_send_buffer_cleanup_on_pipe(struct HIF_CE_pipe_info *pipe_info)
{
	struct CE_handle *ce_hdl;
	struct HIF_CE_state *hif_state;
	struct hif_softc *scn;
	qdf_nbuf_t netbuf;
	void *per_CE_context;
	qdf_dma_addr_t CE_data;
	unsigned int nbytes;
	unsigned int id;
	uint32_t buf_sz;
	uint32_t toeplitz_hash_result;

	buf_sz = pipe_info->buf_sz;
	if (buf_sz == 0) {
		/* Unused Copy Engine */
		return;
	}

	hif_state = pipe_info->HIF_CE_state;
	if (!hif_state->started) {
		return;
	}

	scn = HIF_GET_SOFTC(hif_state);

	ce_hdl = pipe_info->ce_hdl;

	while (ce_cancel_send_next
		       (ce_hdl, &per_CE_context,
		       (void **)&netbuf, &CE_data, &nbytes,
		       &id, &toeplitz_hash_result) == QDF_STATUS_SUCCESS) {
		if (netbuf != CE_SENDLIST_ITEM_CTXT) {
			/*
			 * Packets enqueued by htt_h2t_ver_req_msg() and
			 * htt_h2t_rx_ring_cfg_msg_ll() have already been
			 * freed in htt_htc_misc_pkt_pool_free() in
			 * wlantl_close(), so do not free them here again
			 * by checking whether it's the endpoint
			 * which they are queued in.
			 */
			if (id == scn->htc_htt_tx_endpoint)
				return;
			/* Indicate the completion to higher
			 * layer to free the buffer */
			hif_state->msg_callbacks_current.
			txCompletionHandler(hif_state->
					    msg_callbacks_current.Context,
					    netbuf, id, toeplitz_hash_result);
		}
	}
}

/*
 * Cleanup residual buffers for device shutdown:
 *    buffers that were enqueued for receive
 *    buffers that were to be sent
 * Note: Buffers that had completed but which were
 * not yet processed are on a completion queue. They
 * are handled when the completion thread shuts down.
 */
void hif_buffer_cleanup(struct HIF_CE_state *hif_state)
{
	int pipe_num;
	struct hif_softc *scn = HIF_GET_SOFTC(hif_state);
	struct CE_state *ce_state;

	for (pipe_num = 0; pipe_num < scn->ce_count; pipe_num++) {
		struct HIF_CE_pipe_info *pipe_info;

		ce_state = scn->ce_id_to_state[pipe_num];
		if (hif_is_nss_wifi_enabled(scn) && ce_state &&
				((ce_state->htt_tx_data) ||
				 (ce_state->htt_rx_data))) {
			continue;
		}

		pipe_info = &hif_state->pipe_info[pipe_num];
		hif_recv_buffer_cleanup_on_pipe(pipe_info);
		hif_send_buffer_cleanup_on_pipe(pipe_info);
	}
}

void hif_flush_surprise_remove(struct hif_opaque_softc *hif_ctx)
{
	struct hif_softc *scn = HIF_GET_SOFTC(hif_ctx);
	struct HIF_CE_state *hif_state = HIF_GET_CE_STATE(scn);

	hif_buffer_cleanup(hif_state);
}

void hif_ce_stop(struct hif_softc *scn)
{
	struct HIF_CE_state *hif_state = HIF_GET_CE_STATE(scn);
	int pipe_num;

	scn->hif_init_done = false;

	/*
	 * At this point, asynchronous threads are stopped,
	 * The Target should not DMA nor interrupt, Host code may
	 * not initiate anything more.  So we just need to clean
	 * up Host-side state.
	 */

	if (scn->athdiag_procfs_inited) {
		athdiag_procfs_remove();
		scn->athdiag_procfs_inited = false;
	}

	hif_buffer_cleanup(hif_state);

	for (pipe_num = 0; pipe_num < scn->ce_count; pipe_num++) {
		struct HIF_CE_pipe_info *pipe_info;

		pipe_info = &hif_state->pipe_info[pipe_num];
		if (pipe_info->ce_hdl) {
			ce_fini(pipe_info->ce_hdl);
			pipe_info->ce_hdl = NULL;
			pipe_info->buf_sz = 0;
		}
	}

	if (hif_state->sleep_timer_init) {
		qdf_timer_stop(&hif_state->sleep_timer);
		qdf_timer_free(&hif_state->sleep_timer);
		hif_state->sleep_timer_init = false;
	}

	hif_state->started = false;
}

/**
 * hif_get_target_ce_config() - get copy engine configuration
 * @target_ce_config_ret: basic copy engine configuration
 * @target_ce_config_sz_ret: size of the basic configuration in bytes
 * @target_service_to_ce_map_ret: service mapping for the copy engines
 * @target_service_to_ce_map_sz_ret: size of the mapping in bytes
 * @target_shadow_reg_cfg_ret: shadow register configuration
 * @shadow_cfg_sz_ret: size of the shadow register configuration in bytes
 *
 * providing accessor to these values outside of this file.
 * currently these are stored in static pointers to const sections.
 * there are multiple configurations that are selected from at compile time.
 * Runtime selection would need to consider mode, target type and bus type.
 *
 * Return: return by parameter.
 */
void hif_get_target_ce_config(struct CE_pipe_config **target_ce_config_ret,
		int *target_ce_config_sz_ret,
		struct service_to_pipe **target_service_to_ce_map_ret,
		int *target_service_to_ce_map_sz_ret,
		struct shadow_reg_cfg **target_shadow_reg_cfg_ret,
		int *shadow_cfg_sz_ret)
{
	*target_ce_config_ret = target_ce_config;
	*target_ce_config_sz_ret = target_ce_config_sz;
	*target_service_to_ce_map_ret = target_service_to_ce_map;
	*target_service_to_ce_map_sz_ret = target_service_to_ce_map_sz;

	if (target_shadow_reg_cfg_ret)
		*target_shadow_reg_cfg_ret = target_shadow_reg_cfg;

	if (shadow_cfg_sz_ret)
		*shadow_cfg_sz_ret = shadow_cfg_sz;
}

/**
 * hif_wlan_enable(): call the platform driver to enable wlan
 * @scn: HIF Context
 *
 * This function passes the con_mode and CE configuration to
 * platform driver to enable wlan.
 *
 * Return: linux error code
 */
int hif_wlan_enable(struct hif_softc *scn)
{
	struct pld_wlan_enable_cfg cfg;
	enum pld_driver_mode mode;
	uint32_t con_mode = hif_get_conparam(scn);

	hif_get_target_ce_config((struct CE_pipe_config **)&cfg.ce_tgt_cfg,
			&cfg.num_ce_tgt_cfg,
			(struct service_to_pipe **)&cfg.ce_svc_cfg,
			&cfg.num_ce_svc_pipe_cfg,
			(struct shadow_reg_cfg **)&cfg.shadow_reg_cfg,
			&cfg.num_shadow_reg_cfg);

	/* translate from structure size to array size */
	cfg.num_ce_tgt_cfg /= sizeof(struct CE_pipe_config);
	cfg.num_ce_svc_pipe_cfg /= sizeof(struct service_to_pipe);
	cfg.num_shadow_reg_cfg /= sizeof(struct shadow_reg_cfg);

	if (QDF_GLOBAL_FTM_MODE == con_mode)
		mode = PLD_FTM;
	else if (QDF_IS_EPPING_ENABLED(con_mode))
		mode = PLD_EPPING;
	else
		mode = PLD_MISSION;

	if (BYPASS_QMI)
		return 0;
	else
		return pld_wlan_enable(scn->qdf_dev->dev, &cfg,
				       mode, QWLAN_VERSIONSTR);
}

#define CE_EPPING_USES_IRQ true

/**
 * hif_ce_prepare_config() - load the correct static tables.
 * @scn: hif context
 *
 * Epping uses different static attribute tables than mission mode.
 */
void hif_ce_prepare_config(struct hif_softc *scn)
{
	uint32_t mode = hif_get_conparam(scn);
	struct hif_opaque_softc *hif_hdl = GET_HIF_OPAQUE_HDL(scn);
	struct hif_target_info *tgt_info = hif_get_target_info_handle(hif_hdl);

	/* if epping is enabled we need to use the epping configuration. */
	if (QDF_IS_EPPING_ENABLED(mode)) {
		if (CE_EPPING_USES_IRQ)
			host_ce_config = host_ce_config_wlan_epping_irq;
		else
			host_ce_config = host_ce_config_wlan_epping_poll;
		target_ce_config = target_ce_config_wlan_epping;
		target_ce_config_sz = sizeof(target_ce_config_wlan_epping);
		target_service_to_ce_map =
		    target_service_to_ce_map_wlan_epping;
		target_service_to_ce_map_sz =
			sizeof(target_service_to_ce_map_wlan_epping);
		target_shadow_reg_cfg = target_shadow_reg_cfg_epping;
		shadow_cfg_sz = sizeof(target_shadow_reg_cfg_epping);
	}

	switch (tgt_info->target_type) {
	default:
		break;
	case TARGET_TYPE_AR900B:
	case TARGET_TYPE_QCA9984:
	case TARGET_TYPE_IPQ4019:
	case TARGET_TYPE_QCA9888:
		host_ce_config = host_ce_config_wlan_ar900b;
		target_ce_config = target_ce_config_wlan_ar900b;
		target_ce_config_sz = sizeof(target_ce_config_wlan_ar900b);

		target_service_to_ce_map = target_service_to_ce_map_ar900b;
		target_service_to_ce_map_sz =
			sizeof(target_service_to_ce_map_ar900b);
		break;

	case TARGET_TYPE_AR9888:
	case TARGET_TYPE_AR9888V2:
		host_ce_config = host_ce_config_wlan_ar9888;
		target_ce_config = target_ce_config_wlan_ar9888;
		target_ce_config_sz = sizeof(target_ce_config_wlan_ar9888);

		target_service_to_ce_map = target_service_to_ce_map_ar900b;
		target_service_to_ce_map_sz =
			sizeof(target_service_to_ce_map_ar900b);
		break;
	}
}

/**
 * hif_ce_open() - do ce specific allocations
 * @hif_sc: pointer to hif context
 *
 * return: 0 for success or QDF_STATUS_E_NOMEM
 */
QDF_STATUS hif_ce_open(struct hif_softc *hif_sc)
{
	struct HIF_CE_state *hif_state = HIF_GET_CE_STATE(hif_sc);

	qdf_spinlock_create(&hif_state->keep_awake_lock);
	return QDF_STATUS_SUCCESS;
}

/**
 * hif_ce_close() - do ce specific free
 * @hif_sc: pointer to hif context
 */
void hif_ce_close(struct hif_softc *hif_sc)
{
}

/**
 * hif_unconfig_ce() - ensure resources from hif_config_ce are freed
 * @hif_sc: hif context
 *
 * uses state variables to support cleaning up when hif_config_ce fails.
 */
void hif_unconfig_ce(struct hif_softc *hif_sc)
{
	int pipe_num;
	struct HIF_CE_pipe_info *pipe_info;
	struct HIF_CE_state *hif_state = HIF_GET_CE_STATE(hif_sc);

	for (pipe_num = 0; pipe_num < hif_sc->ce_count; pipe_num++) {
		pipe_info = &hif_state->pipe_info[pipe_num];
		if (pipe_info->ce_hdl) {
			ce_unregister_irq(hif_state, (1 << pipe_num));
			hif_sc->request_irq_done = false;
			ce_fini(pipe_info->ce_hdl);
			pipe_info->ce_hdl = NULL;
			pipe_info->buf_sz = 0;
		}
	}
	if (hif_sc->athdiag_procfs_inited) {
		athdiag_procfs_remove();
		hif_sc->athdiag_procfs_inited = false;
	}
}

#ifdef CONFIG_BYPASS_QMI
#define FW_SHARED_MEM (2 * 1024 * 1024)

/**
 * hif_post_static_buf_to_target() - post static buffer to WLAN FW
 * @scn: pointer to HIF structure
 *
 * WLAN FW needs 2MB memory from DDR when QMI is disabled.
 *
 * Return: void
 */
static void hif_post_static_buf_to_target(struct hif_softc *scn)
{
	void *target_va;
	phys_addr_t target_pa;

	target_va = qdf_mem_alloc_consistent(scn->qdf_dev, scn->qdf_dev->dev,
				FW_SHARED_MEM, &target_pa);
	if (NULL == target_va) {
		HIF_TRACE("Memory allocation failed could not post target buf");
		return;
	}
	hif_write32_mb(scn->mem + BYPASS_QMI_TEMP_REGISTER, target_pa);
	HIF_TRACE("target va %pK target pa %pa", target_va, &target_pa);
}
#else
static inline void hif_post_static_buf_to_target(struct hif_softc *scn)
{
	return;
}
#endif

/**
 * hif_config_ce() - configure copy engines
 * @scn: hif context
 *
 * Prepares fw, copy engine hardware and host sw according
 * to the attributes selected by hif_ce_prepare_config.
 *
 * also calls athdiag_procfs_init
 *
 * return: 0 for success nonzero for failure.
 */
int hif_config_ce(struct hif_softc *scn)
{
	struct HIF_CE_state *hif_state = HIF_GET_CE_STATE(scn);
	struct hif_opaque_softc *hif_hdl = GET_HIF_OPAQUE_HDL(scn);
	struct HIF_CE_pipe_info *pipe_info;
	int pipe_num;
	struct CE_state *ce_state;
#ifdef ADRASTEA_SHADOW_REGISTERS
	int i;
#endif
	QDF_STATUS rv = QDF_STATUS_SUCCESS;

	scn->notice_send = true;

	hif_post_static_buf_to_target(scn);

	hif_state->fw_indicator_address = FW_INDICATOR_ADDRESS;

	hif_config_rri_on_ddr(scn);

	/* During CE initializtion */
	scn->ce_count = HOST_CE_COUNT;
	for (pipe_num = 0; pipe_num < scn->ce_count; pipe_num++) {
		struct CE_attr *attr;
		pipe_info = &hif_state->pipe_info[pipe_num];
		pipe_info->pipe_num = pipe_num;
		pipe_info->HIF_CE_state = hif_state;
		attr = &host_ce_config[pipe_num];
		pipe_info->ce_hdl = ce_init(scn, pipe_num, attr);
		ce_state = scn->ce_id_to_state[pipe_num];
		QDF_ASSERT(pipe_info->ce_hdl != NULL);
		if (pipe_info->ce_hdl == NULL) {
			rv = QDF_STATUS_E_FAILURE;
			A_TARGET_ACCESS_UNLIKELY(scn);
			goto err;
		}

		if (pipe_num == DIAG_CE_ID) {
			/* Reserve the ultimate CE for
			 * Diagnostic Window support */
			hif_state->ce_diag = pipe_info->ce_hdl;
			continue;
		}

		if (hif_is_nss_wifi_enabled(scn) && ce_state &&
				(ce_state->htt_rx_data))
			continue;

		pipe_info->buf_sz = (qdf_size_t) (attr->src_sz_max);
		qdf_spinlock_create(&pipe_info->recv_bufs_needed_lock);
		if (attr->dest_nentries > 0) {
			atomic_set(&pipe_info->recv_bufs_needed,
				   init_buffer_count(attr->dest_nentries - 1));
		} else {
			atomic_set(&pipe_info->recv_bufs_needed, 0);
		}
		ce_tasklet_init(hif_state, (1 << pipe_num));
		ce_register_irq(hif_state, (1 << pipe_num));
		scn->request_irq_done = true;
	}

	if (athdiag_procfs_init(scn) != 0) {
		A_TARGET_ACCESS_UNLIKELY(scn);
		goto err;
	}
	scn->athdiag_procfs_inited = true;

	HIF_INFO_MED("%s: ce_init done", __func__);

	init_tasklet_workers(hif_hdl);

	HIF_TRACE("%s: X, ret = %d", __func__, rv);

#ifdef ADRASTEA_SHADOW_REGISTERS
	HIF_INFO("%s, Using Shadow Registers instead of CE Registers", __func__);
	for (i = 0; i < NUM_SHADOW_REGISTERS; i++) {
		HIF_INFO("%s Shadow Register%d is mapped to address %x",
			  __func__, i,
			  (A_TARGET_READ(scn, (SHADOW_ADDRESS(i))) << 2));
	}
#endif

	return rv != QDF_STATUS_SUCCESS;

err:
	/* Failure, so clean up */
	hif_unconfig_ce(scn);
	HIF_TRACE("%s: X, ret = %d", __func__, rv);
	return QDF_STATUS_SUCCESS != QDF_STATUS_E_FAILURE;
}

#ifdef WLAN_FEATURE_FASTPATH
/**
 * hif_ce_fastpath_cb_register() - Register callback for fastpath msg handler
 * @handler: Callback funtcion
 * @context: handle for callback function
 *
 * Return: QDF_STATUS_SUCCESS on success or QDF_STATUS_E_FAILURE
 */
int hif_ce_fastpath_cb_register(struct hif_opaque_softc *hif_ctx,
				fastpath_msg_handler handler,
				void *context)
{
	struct CE_state *ce_state;
	struct hif_softc *scn = HIF_GET_SOFTC(hif_ctx);
	int i;

	if (!scn) {
		HIF_ERROR("%s: scn is NULL", __func__);
		QDF_ASSERT(0);
		return QDF_STATUS_E_FAILURE;
	}

	if (!scn->fastpath_mode_on) {
		HIF_WARN("%s: Fastpath mode disabled", __func__);
		return QDF_STATUS_E_FAILURE;
	}

	for (i = 0; i < scn->ce_count; i++) {
		ce_state = scn->ce_id_to_state[i];
		if (ce_state->htt_rx_data) {
			ce_state->fastpath_handler = handler;
			ce_state->context = context;
		}
	}

	return QDF_STATUS_SUCCESS;
}
#endif

#ifdef IPA_OFFLOAD
/**
 * hif_ce_ipa_get_ce_resource() - get uc resource on hif
 * @scn: bus context
 * @ce_sr_base_paddr: copyengine source ring base physical address
 * @ce_sr_ring_size: copyengine source ring size
 * @ce_reg_paddr: copyengine register physical address
 *
 * IPA micro controller data path offload feature enabled,
 * HIF should release copy engine related resource information to IPA UC
 * IPA UC will access hardware resource with released information
 *
 * Return: None
 */
void hif_ce_ipa_get_ce_resource(struct hif_softc *scn,
			     qdf_dma_addr_t *ce_sr_base_paddr,
			     uint32_t *ce_sr_ring_size,
			     qdf_dma_addr_t *ce_reg_paddr)
{
	struct HIF_CE_state *hif_state = HIF_GET_CE_STATE(scn);
	struct HIF_CE_pipe_info *pipe_info =
		&(hif_state->pipe_info[HIF_PCI_IPA_UC_ASSIGNED_CE]);
	struct CE_handle *ce_hdl = pipe_info->ce_hdl;

	ce_ipa_get_resource(ce_hdl, ce_sr_base_paddr, ce_sr_ring_size,
			    ce_reg_paddr);
	return;
}
#endif /* IPA_OFFLOAD */


#ifdef ADRASTEA_SHADOW_REGISTERS

/*
	Current shadow register config

	-----------------------------------------------------------
	Shadow Register      |     CE   |    src/dst write index
	-----------------------------------------------------------
		0            |     0    |           src
		1     No Config - Doesn't point to anything
		2     No Config - Doesn't point to anything
		3            |     3    |           src
		4            |     4    |           src
		5            |     5    |           src
		6     No Config - Doesn't point to anything
		7            |     7    |           src
		8     No Config - Doesn't point to anything
		9     No Config - Doesn't point to anything
		10    No Config - Doesn't point to anything
		11    No Config - Doesn't point to anything
	-----------------------------------------------------------
		12    No Config - Doesn't point to anything
		13           |     1    |           dst
		14           |     2    |           dst
		15    No Config - Doesn't point to anything
		16    No Config - Doesn't point to anything
		17    No Config - Doesn't point to anything
		18    No Config - Doesn't point to anything
		19           |     7    |           dst
		20           |     8    |           dst
		21    No Config - Doesn't point to anything
		22    No Config - Doesn't point to anything
		23    No Config - Doesn't point to anything
	-----------------------------------------------------------


	ToDo - Move shadow register config to following in the future
	This helps free up a block of shadow registers towards the end.
	Can be used for other purposes

	-----------------------------------------------------------
	Shadow Register      |     CE   |    src/dst write index
	-----------------------------------------------------------
		0            |     0    |           src
		1            |     3    |           src
		2            |     4    |           src
		3            |     5    |           src
		4            |     7    |           src
	-----------------------------------------------------------
		5            |     1    |           dst
		6            |     2    |           dst
		7            |     7    |           dst
		8            |     8    |           dst
	-----------------------------------------------------------
		9     No Config - Doesn't point to anything
		12    No Config - Doesn't point to anything
		13    No Config - Doesn't point to anything
		14    No Config - Doesn't point to anything
		15    No Config - Doesn't point to anything
		16    No Config - Doesn't point to anything
		17    No Config - Doesn't point to anything
		18    No Config - Doesn't point to anything
		19    No Config - Doesn't point to anything
		20    No Config - Doesn't point to anything
		21    No Config - Doesn't point to anything
		22    No Config - Doesn't point to anything
		23    No Config - Doesn't point to anything
	-----------------------------------------------------------
*/

u32 shadow_sr_wr_ind_addr(struct hif_softc *scn, u32 ctrl_addr)
{
	u32 addr = 0;
	u32 ce = COPY_ENGINE_ID(ctrl_addr);

	switch (ce) {
	case 0:
		addr = SHADOW_VALUE0;
		break;
	case 3:
		addr = SHADOW_VALUE3;
		break;
	case 4:
		addr = SHADOW_VALUE4;
		break;
	case 5:
		addr = SHADOW_VALUE5;
		break;
	case 7:
		addr = SHADOW_VALUE7;
		break;
	default:
		HIF_ERROR("invalid CE ctrl_addr (CE=%d)", ce);
		QDF_ASSERT(0);
	}
	return addr;

}

u32 shadow_dst_wr_ind_addr(struct hif_softc *scn, u32 ctrl_addr)
{
	u32 addr = 0;
	u32 ce = COPY_ENGINE_ID(ctrl_addr);

	switch (ce) {
	case 1:
		addr = SHADOW_VALUE13;
		break;
	case 2:
		addr = SHADOW_VALUE14;
		break;
	case 5:
		addr = SHADOW_VALUE17;
		break;
	case 7:
		addr = SHADOW_VALUE19;
		break;
	case 8:
		addr = SHADOW_VALUE20;
		break;
	case 9:
		addr = SHADOW_VALUE21;
		break;
	case 10:
		addr = SHADOW_VALUE22;
		break;
	case 11:
		addr = SHADOW_VALUE23;
		break;
	default:
		HIF_ERROR("invalid CE ctrl_addr (CE=%d)", ce);
		QDF_ASSERT(0);
	}

	return addr;

}
#endif

#if defined(FEATURE_LRO)
/**
 * ce_lro_flush_cb_register() - register the LRO flush
 * callback
 * @scn: HIF context
 * @handler: callback function
 * @data: opaque data pointer to be passed back
 *
 * Store the LRO flush callback provided
 *
 * Return: Number of instances the callback is registered for
 */
int ce_lro_flush_cb_register(struct hif_opaque_softc *hif_hdl,
			     void (handler)(void *), void *data)
{
	int rc = 0;
	int i;
	struct CE_state *ce_state;
	struct hif_softc *scn = HIF_GET_SOFTC(hif_hdl);

	QDF_ASSERT(scn != NULL);

	if (scn != NULL) {
		for (i = 0; i < scn->ce_count; i++) {
			ce_state = scn->ce_id_to_state[i];
			if ((ce_state != NULL) && (ce_state->htt_rx_data)) {
				ce_state->lro_flush_cb = handler;
				ce_state->lro_data = data;
				rc++;
			}
		}
	} else {
		HIF_ERROR("%s: hif_state NULL!", __func__);
	}
	return rc;
}

/**
 * ce_lro_flush_cb_deregister() - deregister the LRO flush
 * callback
 * @scn: HIF context
 *
 * Remove the LRO flush callback
 *
 * Return: Number of instances the callback is de-registered
 */
int ce_lro_flush_cb_deregister(struct hif_opaque_softc *hif_hdl)
{
	int rc = 0;
	int i;
	struct CE_state *ce_state;
	struct hif_softc *scn = HIF_GET_SOFTC(hif_hdl);

	QDF_ASSERT(scn != NULL);
	if (scn != NULL) {
		for (i = 0; i < scn->ce_count; i++) {
			ce_state = scn->ce_id_to_state[i];
			if ((ce_state != NULL) && (ce_state->htt_rx_data)) {
				ce_state->lro_flush_cb = NULL;
				ce_state->lro_data = NULL;
				rc++;
			}
		}
	} else {
		HIF_ERROR("%s: hif_state NULL!", __func__);
	}
	return rc;
}
#endif

/**
 * hif_map_service_to_pipe() - returns  the ce ids pertaining to
 * this service
 * @scn: hif_softc pointer.
 * @svc_id: Service ID for which the mapping is needed.
 * @ul_pipe: address of the container in which ul pipe is returned.
 * @dl_pipe: address of the container in which dl pipe is returned.
 * @ul_is_polled: address of the container in which a bool
 *			indicating if the UL CE for this service
 *			is polled is returned.
 * @dl_is_polled: address of the container in which a bool
 *			indicating if the DL CE for this service
 *			is polled is returned.
 *
 * Return: Indicates whether the service has been found in the table.
 *         Upon return, ul_is_polled is updated only if ul_pipe is updated.
 *         There will be warning logs if either leg has not been updated
 *         because it missed the entry in the table (but this is not an err).
 */
int hif_map_service_to_pipe(struct hif_opaque_softc *hif_hdl, uint16_t svc_id,
			uint8_t *ul_pipe, uint8_t *dl_pipe, int *ul_is_polled,
			int *dl_is_polled)
{
	int status = QDF_STATUS_E_INVAL;
	unsigned int i;
	struct service_to_pipe element;
	struct service_to_pipe *tgt_svc_map_to_use;
	size_t sz_tgt_svc_map_to_use;
	struct hif_softc *scn = HIF_GET_SOFTC(hif_hdl);
	uint32_t mode = hif_get_conparam(scn);
	struct hif_target_info *tgt_info = hif_get_target_info_handle(hif_hdl);
	bool dl_updated = false;
	bool ul_updated = false;

	if (QDF_IS_EPPING_ENABLED(mode)) {
		tgt_svc_map_to_use = target_service_to_ce_map_wlan_epping;
		sz_tgt_svc_map_to_use =
			sizeof(target_service_to_ce_map_wlan_epping);
	} else {
		switch (tgt_info->target_type) {
		default:
			tgt_svc_map_to_use = target_service_to_ce_map_wlan;
			sz_tgt_svc_map_to_use =
				sizeof(target_service_to_ce_map_wlan);
			break;
		case TARGET_TYPE_AR900B:
		case TARGET_TYPE_QCA9984:
		case TARGET_TYPE_IPQ4019:
		case TARGET_TYPE_QCA9888:
		case TARGET_TYPE_AR9888:
		case TARGET_TYPE_AR9888V2:
			tgt_svc_map_to_use = target_service_to_ce_map_ar900b;
			sz_tgt_svc_map_to_use =
				sizeof(target_service_to_ce_map_ar900b);
			break;
		}
	}

	*dl_is_polled = 0;  /* polling for received messages not supported */

	for (i = 0; i < (sz_tgt_svc_map_to_use/sizeof(element)); i++) {

		memcpy(&element, &tgt_svc_map_to_use[i], sizeof(element));
		if (element.service_id == svc_id) {
			if (element.pipedir == PIPEDIR_OUT) {
				*ul_pipe = element.pipenum;
				*ul_is_polled =
					(host_ce_config[*ul_pipe].flags &
					 CE_ATTR_DISABLE_INTR) != 0;
				ul_updated = true;
			} else if (element.pipedir == PIPEDIR_IN) {
				*dl_pipe = element.pipenum;
				dl_updated = true;
			}
			status = QDF_STATUS_SUCCESS;
		}
	}
	if (ul_updated == false)
		HIF_WARN("%s: ul pipe is NOT updated for service %d",
			 __func__, svc_id);
	if (dl_updated == false)
		HIF_WARN("%s: dl pipe is NOT updated for service %d",
			 __func__, svc_id);

	return status;
}

#ifdef SHADOW_REG_DEBUG
inline uint32_t DEBUG_CE_SRC_RING_READ_IDX_GET(struct hif_softc *scn,
		uint32_t CE_ctrl_addr)
{
	uint32_t read_from_hw, srri_from_ddr = 0;

	read_from_hw = A_TARGET_READ(scn, CE_ctrl_addr + CURRENT_SRRI_ADDRESS);

	srri_from_ddr = SRRI_FROM_DDR_ADDR(VADDR_FOR_CE(scn, CE_ctrl_addr));

	if (read_from_hw != srri_from_ddr) {
		HIF_ERROR("%s: error: read from ddr = %d actual read from register = %d, CE_MISC_INT_STATUS_GET = 0x%x",
		       __func__, srri_from_ddr, read_from_hw,
		       CE_MISC_INT_STATUS_GET(scn, CE_ctrl_addr));
		QDF_ASSERT(0);
	}
	return srri_from_ddr;
}


inline uint32_t DEBUG_CE_DEST_RING_READ_IDX_GET(struct hif_softc *scn,
		uint32_t CE_ctrl_addr)
{
	uint32_t read_from_hw, drri_from_ddr = 0;

	read_from_hw = A_TARGET_READ(scn, CE_ctrl_addr + CURRENT_DRRI_ADDRESS);

	drri_from_ddr = DRRI_FROM_DDR_ADDR(VADDR_FOR_CE(scn, CE_ctrl_addr));

	if (read_from_hw != drri_from_ddr) {
		HIF_ERROR("error: read from ddr = %d actual read from register = %d, CE_MISC_INT_STATUS_GET = 0x%x",
		       drri_from_ddr, read_from_hw,
		       CE_MISC_INT_STATUS_GET(scn, CE_ctrl_addr));
		QDF_ASSERT(0);
	}
	return drri_from_ddr;
}

#endif

#ifdef ADRASTEA_RRI_ON_DDR
/**
 * hif_get_src_ring_read_index(): Called to get the SRRI
 *
 * @scn: hif_softc pointer
 * @CE_ctrl_addr: base address of the CE whose RRI is to be read
 *
 * This function returns the SRRI to the caller. For CEs that
 * dont have interrupts enabled, we look at the DDR based SRRI
 *
 * Return: SRRI
 */
inline unsigned int hif_get_src_ring_read_index(struct hif_softc *scn,
		uint32_t CE_ctrl_addr)
{
	struct CE_attr attr;

	attr = host_ce_config[COPY_ENGINE_ID(CE_ctrl_addr)];
	if (attr.flags & CE_ATTR_DISABLE_INTR)
		return CE_SRC_RING_READ_IDX_GET_FROM_DDR(scn, CE_ctrl_addr);
	else
		return A_TARGET_READ(scn,
				(CE_ctrl_addr) + CURRENT_SRRI_ADDRESS);
}

/**
 * hif_get_dst_ring_read_index(): Called to get the DRRI
 *
 * @scn: hif_softc pointer
 * @CE_ctrl_addr: base address of the CE whose RRI is to be read
 *
 * This function returns the DRRI to the caller. For CEs that
 * dont have interrupts enabled, we look at the DDR based DRRI
 *
 * Return: DRRI
 */
inline unsigned int hif_get_dst_ring_read_index(struct hif_softc *scn,
		uint32_t CE_ctrl_addr)
{
	struct CE_attr attr;

	attr = host_ce_config[COPY_ENGINE_ID(CE_ctrl_addr)];

	if (attr.flags & CE_ATTR_DISABLE_INTR)
		return CE_DEST_RING_READ_IDX_GET_FROM_DDR(scn, CE_ctrl_addr);
	else
		return A_TARGET_READ(scn,
				(CE_ctrl_addr) + CURRENT_DRRI_ADDRESS);
}

/**
 * hif_config_rri_on_ddr(): Configure the RRI on DDR mechanism
 *
 * @scn: hif_softc pointer
 *
 * This function allocates non cached memory on ddr and sends
 * the physical address of this memory to the CE hardware. The
 * hardware updates the RRI on this particular location.
 *
 * Return: None
 */
static inline void hif_config_rri_on_ddr(struct hif_softc *scn)
{
	unsigned int i;
	qdf_dma_addr_t paddr_rri_on_ddr;
	uint32_t high_paddr, low_paddr;
	scn->vaddr_rri_on_ddr =
		(uint32_t *)qdf_mem_alloc_consistent(scn->qdf_dev,
		scn->qdf_dev->dev, (CE_COUNT*sizeof(uint32_t)),
		&paddr_rri_on_ddr);

	low_paddr  = BITS0_TO_31(paddr_rri_on_ddr);
	high_paddr = BITS32_TO_35(paddr_rri_on_ddr);

	HIF_INFO("%s using srri and drri from DDR", __func__);

	WRITE_CE_DDR_ADDRESS_FOR_RRI_LOW(scn, low_paddr);
	WRITE_CE_DDR_ADDRESS_FOR_RRI_HIGH(scn, high_paddr);

	for (i = 0; i < CE_COUNT; i++)
		CE_IDX_UPD_EN_SET(scn, CE_BASE_ADDRESS(i));

	qdf_mem_zero(scn->vaddr_rri_on_ddr, CE_COUNT*sizeof(uint32_t));

	return;
}
#else

/**
 * hif_config_rri_on_ddr(): Configure the RRI on DDR mechanism
 *
 * @scn: hif_softc pointer
 *
 * This is a dummy implementation for platforms that don't
 * support this functionality.
 *
 * Return: None
 */
static inline void hif_config_rri_on_ddr(struct hif_softc *scn)
{
	return;
}
#endif

/**
 * hif_dump_ce_registers() - dump ce registers
 * @scn: hif_opaque_softc pointer.
 *
 * Output the copy engine registers
 *
 * Return: 0 for success or error code
 */
int hif_dump_ce_registers(struct hif_softc *scn)
{
	struct hif_opaque_softc *hif_hdl = GET_HIF_OPAQUE_HDL(scn);
	uint32_t ce_reg_address = CE0_BASE_ADDRESS;
	uint32_t ce_reg_values[CE_USEFUL_SIZE >> 2];
	uint32_t ce_reg_word_size = CE_USEFUL_SIZE >> 2;
	uint16_t i;
	QDF_STATUS status;

	for (i = 0; i < scn->ce_count; i++, ce_reg_address += CE_OFFSET) {
		if (scn->ce_id_to_state[i] == NULL) {
			HIF_DBG("CE%d not used.", i);
			continue;
		}

		status = hif_diag_read_mem(hif_hdl, ce_reg_address,
					   (uint8_t *) &ce_reg_values[0],
					   ce_reg_word_size * sizeof(uint32_t));

		if (status != QDF_STATUS_SUCCESS) {
				HIF_ERROR("Dumping CE register failed!");
				return -EACCES;
		}
		HIF_ERROR("CE%d Registers:", i);
		qdf_trace_hex_dump(QDF_MODULE_ID_HIF, QDF_TRACE_LEVEL_DEBUG,
				   (uint8_t *) &ce_reg_values[0],
				   ce_reg_word_size * sizeof(uint32_t));
	}
	return 0;
}

#ifdef QCA_NSS_WIFI_OFFLOAD_SUPPORT
struct hif_pipe_addl_info *hif_get_addl_pipe_info(struct hif_opaque_softc *osc,
		struct hif_pipe_addl_info *hif_info, uint32_t pipe)
{
	struct hif_softc *scn = HIF_GET_SOFTC(osc);
	struct hif_pci_softc *sc = HIF_GET_PCI_SOFTC(scn);
	struct HIF_CE_state *hif_state = HIF_GET_CE_STATE(osc);
	struct HIF_CE_pipe_info *pipe_info = &(hif_state->pipe_info[pipe]);
	struct CE_handle *ce_hdl = pipe_info->ce_hdl;
	struct CE_state *ce_state = (struct CE_state *)ce_hdl;
	struct CE_ring_state *src_ring = ce_state->src_ring;
	struct CE_ring_state *dest_ring = ce_state->dest_ring;

	if (src_ring) {
		hif_info->ul_pipe.nentries = src_ring->nentries;
		hif_info->ul_pipe.nentries_mask = src_ring->nentries_mask;
		hif_info->ul_pipe.sw_index = src_ring->sw_index;
		hif_info->ul_pipe.write_index = src_ring->write_index;
		hif_info->ul_pipe.hw_index = src_ring->hw_index;
		hif_info->ul_pipe.base_addr_CE_space =
			src_ring->base_addr_CE_space;
		hif_info->ul_pipe.base_addr_owner_space =
			src_ring->base_addr_owner_space;
	}


	if (dest_ring) {
		hif_info->dl_pipe.nentries = dest_ring->nentries;
		hif_info->dl_pipe.nentries_mask = dest_ring->nentries_mask;
		hif_info->dl_pipe.sw_index = dest_ring->sw_index;
		hif_info->dl_pipe.write_index = dest_ring->write_index;
		hif_info->dl_pipe.hw_index = dest_ring->hw_index;
		hif_info->dl_pipe.base_addr_CE_space =
			dest_ring->base_addr_CE_space;
		hif_info->dl_pipe.base_addr_owner_space =
			dest_ring->base_addr_owner_space;
	}

	hif_info->pci_mem = pci_resource_start(sc->pdev, 0);
	hif_info->ctrl_addr = ce_state->ctrl_addr;

	return hif_info;
}

uint32_t hif_set_nss_wifiol_mode(struct hif_opaque_softc *osc, uint32_t mode)
{
	struct hif_softc *scn = HIF_GET_SOFTC(osc);

	scn->nss_wifi_ol_mode = mode;
	return 0;
}

#endif

void hif_disable_interrupt(struct hif_opaque_softc *osc, uint32_t pipe_num)
{
	struct hif_softc *scn = HIF_GET_SOFTC(osc);
	struct CE_state *CE_state = scn->ce_id_to_state[pipe_num];
	uint32_t ctrl_addr = CE_state->ctrl_addr;

	Q_TARGET_ACCESS_BEGIN(scn);
	CE_COPY_COMPLETE_INTR_DISABLE(scn, ctrl_addr);
	Q_TARGET_ACCESS_END(scn);
}

/**
 * hif_fw_event_handler() - hif fw event handler
 * @hif_state: pointer to hif ce state structure
 *
 * Process fw events and raise HTC callback to process fw events.
 *
 * Return: none
 */
static inline void hif_fw_event_handler(struct HIF_CE_state *hif_state)
{
	struct hif_msg_callbacks *msg_callbacks =
		&hif_state->msg_callbacks_current;

	if (!msg_callbacks->fwEventHandler)
		return;

	msg_callbacks->fwEventHandler(msg_callbacks->Context,
			QDF_STATUS_E_FAILURE);
}

#ifndef QCA_WIFI_3_0
/**
 * hif_fw_interrupt_handler() - FW interrupt handler
 * @irq: irq number
 * @arg: the user pointer
 *
 * Called from the PCI interrupt handler when a
 * firmware-generated interrupt to the Host.
 *
 * Return: status of handled irq
 */
irqreturn_t hif_fw_interrupt_handler(int irq, void *arg)
{
	struct hif_softc *scn = arg;
	struct HIF_CE_state *hif_state = HIF_GET_CE_STATE(scn);
	uint32_t fw_indicator_address, fw_indicator;

	if (Q_TARGET_ACCESS_BEGIN(scn) < 0)
		return ATH_ISR_NOSCHED;

	fw_indicator_address = hif_state->fw_indicator_address;
	/* For sudden unplug this will return ~0 */
	fw_indicator = A_TARGET_READ(scn, fw_indicator_address);

	if ((fw_indicator != ~0) && (fw_indicator & FW_IND_EVENT_PENDING)) {
		/* ACK: clear Target-side pending event */
		A_TARGET_WRITE(scn, fw_indicator_address,
			       fw_indicator & ~FW_IND_EVENT_PENDING);
		if (Q_TARGET_ACCESS_END(scn) < 0)
			return ATH_ISR_SCHED;

		if (hif_state->started) {
			hif_fw_event_handler(hif_state);
		} else {
			/*
			 * Probable Target failure before we're prepared
			 * to handle it.  Generally unexpected.
			 */
			AR_DEBUG_PRINTF(ATH_DEBUG_ERR,
				("%s: Early firmware event indicated\n",
				 __func__));
		}
	} else {
		if (Q_TARGET_ACCESS_END(scn) < 0)
			return ATH_ISR_SCHED;
	}

	return ATH_ISR_SCHED;
}
#else
irqreturn_t hif_fw_interrupt_handler(int irq, void *arg)
{
	return ATH_ISR_SCHED;
}
#endif /* #ifdef QCA_WIFI_3_0 */


/**
 * hif_wlan_disable(): call the platform driver to disable wlan
 * @scn: HIF Context
 *
 * This function passes the con_mode to platform driver to disable
 * wlan.
 *
 * Return: void
 */
void hif_wlan_disable(struct hif_softc *scn)
{
	enum pld_driver_mode mode;
	uint32_t con_mode = hif_get_conparam(scn);

	if (QDF_GLOBAL_FTM_MODE == con_mode)
		mode = PLD_FTM;
	else if (QDF_IS_EPPING_ENABLED(con_mode))
		mode = PLD_EPPING;
	else
		mode = PLD_MISSION;

	pld_wlan_disable(scn->qdf_dev->dev, mode);
}
