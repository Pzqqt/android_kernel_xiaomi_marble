/*
 * Copyright (c) 2013-2015 The Linux Foundation. All rights reserved.
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
#include <osdep.h>
#include "a_types.h"
#include "athdefs.h"
#include "osapi_linux.h"
#include "targcfg.h"
#include "cdf_lock.h"
#include "cdf_status.h"
#include <cdf_atomic.h>         /* cdf_atomic_read */
#include <targaddrs.h>
#include <bmi_msg.h>
#include "hif_io32.h"
#include <hif.h>
#include "regtable.h"
#define ATH_MODULE_NAME hif
#include <a_debug.h>
#include "hif_main.h"
#ifdef HIF_PCI
#include "ce_bmi.h"
#endif
#include "ce_api.h"
#include "cdf_trace.h"
#include "cds_api.h"
#ifdef CONFIG_CNSS
#include <net/cnss.h>
#endif
#include "epping_main.h"
#include "hif_debug.h"
#include "ce_internal.h"
#include "ce_reg.h"
#include "ce_assignment.h"
#include "ce_tasklet.h"
#ifdef HIF_PCI
#include "icnss_stub.h"
#else
#include <soc/qcom/icnss.h>
#endif
#include "qwlan_version.h"
#include "cds_concurrency.h"

#define CE_POLL_TIMEOUT 10      /* ms */

/* Forward references */
static int hif_post_recv_buffers_for_pipe(struct HIF_CE_pipe_info *pipe_info);

/*
 * Fix EV118783, poll to check whether a BMI response comes
 * other than waiting for the interruption which may be lost.
 */
/* #define BMI_RSP_POLLING */
#define BMI_RSP_TO_MILLISEC  1000


static int hif_post_recv_buffers(struct ol_softc *scn);
static void hif_config_rri_on_ddr(struct ol_softc *scn);

static void ce_poll_timeout(void *arg)
{
	struct CE_state *CE_state = (struct CE_state *)arg;
	if (CE_state->timer_inited) {
		ce_per_engine_service(CE_state->scn, CE_state->id);
		cdf_softirq_timer_mod(&CE_state->poll_timer, CE_POLL_TIMEOUT);
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

	CDF_ASSERT(0); /* n too large */
	return 0;
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
struct CE_handle *ce_init(struct ol_softc *scn,
			  unsigned int CE_id, struct CE_attr *attr)
{
	struct CE_state *CE_state;
	uint32_t ctrl_addr;
	unsigned int nentries;
	cdf_dma_addr_t base_addr;
	bool malloc_CE_state = false;
	bool malloc_src_ring = false;

	CDF_ASSERT(CE_id < scn->ce_count);
	ctrl_addr = CE_BASE_ADDRESS(CE_id);
	CE_state = scn->ce_id_to_state[CE_id];

	if (!CE_state) {
		CE_state =
		    (struct CE_state *)cdf_mem_malloc(sizeof(*CE_state));
		if (!CE_state) {
			HIF_ERROR("%s: CE_state has no mem", __func__);
			return NULL;
		}
		malloc_CE_state = true;
		cdf_mem_zero(CE_state, sizeof(*CE_state));
		scn->ce_id_to_state[CE_id] = CE_state;

		CE_state->id = CE_id;
		CE_state->ctrl_addr = ctrl_addr;
		CE_state->state = CE_RUNNING;
		CE_state->attr_flags = attr->flags;
	}
	CE_state->scn = scn;

	cdf_atomic_init(&CE_state->rx_pending);
	if (attr == NULL) {
		/* Already initialized; caller wants the handle */
		return (struct CE_handle *)CE_state;
	}

#ifdef ADRASTEA_SHADOW_REGISTERS
	HIF_ERROR("%s: Using Shadow Registers instead of CE Registers\n",
		  __func__);
#endif

	if (CE_state->src_sz_max)
		CDF_ASSERT(CE_state->src_sz_max == attr->src_sz_max);
	else
		CE_state->src_sz_max = attr->src_sz_max;

	/* source ring setup */
	nentries = attr->src_nentries;
	if (nentries) {
		struct CE_ring_state *src_ring;
		unsigned CE_nbytes;
		char *ptr;
		uint64_t dma_addr;
		nentries = roundup_pwr2(nentries);
		if (CE_state->src_ring) {
			CDF_ASSERT(CE_state->src_ring->nentries == nentries);
		} else {
			CE_nbytes = sizeof(struct CE_ring_state)
				    + (nentries * sizeof(void *));
			ptr = cdf_mem_malloc(CE_nbytes);
			if (!ptr) {
				/* cannot allocate src ring. If the
				 * CE_state is allocated locally free
				 * CE_State and return error.
				 */
				HIF_ERROR("%s: src ring has no mem", __func__);
				if (malloc_CE_state) {
					/* allocated CE_state locally */
					scn->ce_id_to_state[CE_id] = NULL;
					cdf_mem_free(CE_state);
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
			cdf_mem_zero(ptr, CE_nbytes);

			src_ring = CE_state->src_ring =
					   (struct CE_ring_state *)ptr;
			ptr += sizeof(struct CE_ring_state);
			src_ring->nentries = nentries;
			src_ring->nentries_mask = nentries - 1;
			A_TARGET_ACCESS_BEGIN_RET_PTR(scn);
			src_ring->hw_index =
				CE_SRC_RING_READ_IDX_GET(scn, ctrl_addr);
			src_ring->sw_index = src_ring->hw_index;
			src_ring->write_index =
				CE_SRC_RING_WRITE_IDX_GET(scn, ctrl_addr);
			A_TARGET_ACCESS_END_RET_PTR(scn);
			src_ring->low_water_mark_nentries = 0;
			src_ring->high_water_mark_nentries = nentries;
			src_ring->per_transfer_context = (void **)ptr;

			/* Legacy platforms that do not support cache
			 * coherent DMA are unsupported
			 */
			src_ring->base_addr_owner_space_unaligned =
				cdf_os_mem_alloc_consistent(scn->cdf_dev,
						(nentries *
						sizeof(struct CE_src_desc) +
						CE_DESC_RING_ALIGN),
						&base_addr, 0);
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
				cdf_mem_malloc(nentries *
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

			A_TARGET_ACCESS_BEGIN_RET_PTR(scn);
			dma_addr = src_ring->base_addr_CE_space;
			CE_SRC_RING_BASE_ADDR_SET(scn, ctrl_addr,
				 (uint32_t)(dma_addr & 0xFFFFFFFF));
#ifdef WLAN_ENABLE_QCA6180
			{
				uint32_t tmp;
				tmp = CE_SRC_RING_BASE_ADDR_HIGH_GET(
				   scn, ctrl_addr);
				tmp &= ~0x1F;
				dma_addr = ((dma_addr >> 32) & 0x1F)|tmp;
				CE_SRC_RING_BASE_ADDR_HIGH_SET(scn,
					 ctrl_addr, (uint32_t)dma_addr);
			}
#endif
			CE_SRC_RING_SZ_SET(scn, ctrl_addr, nentries);
			CE_SRC_RING_DMAX_SET(scn, ctrl_addr, attr->src_sz_max);
#ifdef BIG_ENDIAN_HOST
			/* Enable source ring byte swap for big endian host */
			CE_SRC_RING_BYTE_SWAP_SET(scn, ctrl_addr, 1);
#endif
			CE_SRC_RING_LOWMARK_SET(scn, ctrl_addr, 0);
			CE_SRC_RING_HIGHMARK_SET(scn, ctrl_addr, nentries);
			A_TARGET_ACCESS_END_RET_PTR(scn);
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
			CDF_ASSERT(CE_state->dest_ring->nentries == nentries);
		} else {
			CE_nbytes = sizeof(struct CE_ring_state)
				    + (nentries * sizeof(void *));
			ptr = cdf_mem_malloc(CE_nbytes);
			if (!ptr) {
				/* cannot allocate dst ring. If the CE_state
				 * or src ring is allocated locally free
				 * CE_State and src ring and return error.
				 */
				HIF_ERROR("%s: dest ring has no mem",
					  __func__);
				if (malloc_src_ring) {
					cdf_mem_free(CE_state->src_ring);
					CE_state->src_ring = NULL;
					malloc_src_ring = false;
				}
				if (malloc_CE_state) {
					/* allocated CE_state locally */
					scn->ce_id_to_state[CE_id] = NULL;
					cdf_mem_free(CE_state);
					malloc_CE_state = false;
				}
				return NULL;
			}
			cdf_mem_zero(ptr, CE_nbytes);

			dest_ring = CE_state->dest_ring =
					    (struct CE_ring_state *)ptr;
			ptr += sizeof(struct CE_ring_state);
			dest_ring->nentries = nentries;
			dest_ring->nentries_mask = nentries - 1;
			A_TARGET_ACCESS_BEGIN_RET_PTR(scn);
			dest_ring->sw_index =
				CE_DEST_RING_READ_IDX_GET(scn, ctrl_addr);
			dest_ring->write_index =
				CE_DEST_RING_WRITE_IDX_GET(scn, ctrl_addr);
			A_TARGET_ACCESS_END_RET_PTR(scn);
			dest_ring->low_water_mark_nentries = 0;
			dest_ring->high_water_mark_nentries = nentries;
			dest_ring->per_transfer_context = (void **)ptr;

			/* Legacy platforms that do not support cache
			 * coherent DMA are unsupported */
			dest_ring->base_addr_owner_space_unaligned =
				cdf_os_mem_alloc_consistent(scn->cdf_dev,
						(nentries *
						sizeof(struct CE_dest_desc) +
						CE_DESC_RING_ALIGN),
						&base_addr, 0);
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
			cdf_mem_zero(dest_ring->base_addr_owner_space_unaligned,
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

			A_TARGET_ACCESS_BEGIN_RET_PTR(scn);
			dma_addr = dest_ring->base_addr_CE_space;
			CE_DEST_RING_BASE_ADDR_SET(scn, ctrl_addr,
				 (uint32_t)(dma_addr & 0xFFFFFFFF));
#ifdef WLAN_ENABLE_QCA6180
			{
				uint32_t tmp;
				tmp = CE_DEST_RING_BASE_ADDR_HIGH_GET(scn,
						ctrl_addr);
				tmp &= ~0x1F;
				dma_addr = ((dma_addr >> 32) & 0x1F)|tmp;
				CE_DEST_RING_BASE_ADDR_HIGH_SET(scn,
					ctrl_addr, (uint32_t)dma_addr);
			}
#endif
			CE_DEST_RING_SZ_SET(scn, ctrl_addr, nentries);
#ifdef BIG_ENDIAN_HOST
			/* Enable Dest ring byte swap for big endian host */
			CE_DEST_RING_BYTE_SWAP_SET(scn, ctrl_addr, 1);
#endif
			CE_DEST_RING_LOWMARK_SET(scn, ctrl_addr, 0);
			CE_DEST_RING_HIGHMARK_SET(scn, ctrl_addr, nentries);
			A_TARGET_ACCESS_END_RET_PTR(scn);

			/* epping */
			/* poll timer */
			if ((CE_state->attr_flags & CE_ATTR_ENABLE_POLL)) {
				cdf_softirq_timer_init(scn->cdf_dev,
						       &CE_state->poll_timer,
						       ce_poll_timeout,
						       CE_state,
						       CDF_TIMER_TYPE_SW);
				CE_state->timer_inited = true;
				cdf_softirq_timer_mod(&CE_state->poll_timer,
						      CE_POLL_TIMEOUT);
			}
		}
	}

	/* Enable CE error interrupts */
	A_TARGET_ACCESS_BEGIN_RET_PTR(scn);
	CE_ERROR_INTR_ENABLE(scn, ctrl_addr);
	A_TARGET_ACCESS_END_RET_PTR(scn);

	return (struct CE_handle *)CE_state;

error_no_dma_mem:
	ce_fini((struct CE_handle *)CE_state);
	return NULL;
}

#ifdef WLAN_FEATURE_FASTPATH
/**
 * ce_h2t_tx_ce_cleanup() Place holder function for H2T CE cleanup.
 * No processing is required inside this function.
 * @ce_hdl: Cope engine handle
 * Using an assert, this function makes sure that,
 * the TX CE has been processed completely.
 * Return: none
 */
void
ce_h2t_tx_ce_cleanup(struct CE_handle *ce_hdl)
{
	struct CE_state *ce_state = (struct CE_state *)ce_hdl;
	struct CE_ring_state *src_ring = ce_state->src_ring;
	struct ol_softc *sc = ce_state->scn;
	uint32_t sw_index, write_index;

	if (sc->fastpath_mode_on && (ce_state->id == CE_HTT_H2T_MSG)) {
		HIF_INFO("%s %d Fastpath mode ON, Cleaning up HTT Tx CE\n",
			  __func__, __LINE__);
		cdf_spin_lock_bh(&sc->target_lock);
		sw_index = src_ring->sw_index;
		write_index = src_ring->sw_index;
		cdf_spin_unlock_bh(&sc->target_lock);

		/* At this point Tx CE should be clean */
		cdf_assert_always(sw_index == write_index);
	}
}
#else
void ce_h2t_tx_ce_cleanup(struct CE_handle *ce_hdl)
{
}
#endif /* WLAN_FEATURE_FASTPATH */

void ce_fini(struct CE_handle *copyeng)
{
	struct CE_state *CE_state = (struct CE_state *)copyeng;
	unsigned int CE_id = CE_state->id;
	struct ol_softc *scn = CE_state->scn;

	CE_state->state = CE_UNUSED;
	scn->ce_id_to_state[CE_id] = NULL;
	if (CE_state->src_ring) {
		/* Cleanup the HTT Tx ring */
		ce_h2t_tx_ce_cleanup(copyeng);

		if (CE_state->src_ring->shadow_base_unaligned)
			cdf_mem_free(CE_state->src_ring->shadow_base_unaligned);
		if (CE_state->src_ring->base_addr_owner_space_unaligned)
			cdf_os_mem_free_consistent(scn->cdf_dev,
					    (CE_state->src_ring->nentries *
					     sizeof(struct CE_src_desc) +
					     CE_DESC_RING_ALIGN),
					    CE_state->src_ring->
					    base_addr_owner_space_unaligned,
					    CE_state->src_ring->
					    base_addr_CE_space, 0);
		cdf_mem_free(CE_state->src_ring);
	}
	if (CE_state->dest_ring) {
		if (CE_state->dest_ring->base_addr_owner_space_unaligned)
			cdf_os_mem_free_consistent(scn->cdf_dev,
					    (CE_state->dest_ring->nentries *
					     sizeof(struct CE_dest_desc) +
					     CE_DESC_RING_ALIGN),
					    CE_state->dest_ring->
					    base_addr_owner_space_unaligned,
					    CE_state->dest_ring->
					    base_addr_CE_space, 0);
		cdf_mem_free(CE_state->dest_ring);

		/* epping */
		if (CE_state->timer_inited) {
			CE_state->timer_inited = false;
			cdf_softirq_timer_free(&CE_state->poll_timer);
		}
	}
	cdf_mem_free(CE_state);
}

void hif_detach_htc(struct ol_softc *scn)
{
	struct HIF_CE_state *hif_state = (struct HIF_CE_state *)scn->hif_hdl;

	cdf_mem_zero(&hif_state->msg_callbacks_pending,
		  sizeof(hif_state->msg_callbacks_pending));
	cdf_mem_zero(&hif_state->msg_callbacks_current,
		  sizeof(hif_state->msg_callbacks_current));
}

/* Send the first nbytes bytes of the buffer */
CDF_STATUS
hif_send_head(struct ol_softc *scn,
	      uint8_t pipe, unsigned int transfer_id, unsigned int nbytes,
	      cdf_nbuf_t nbuf, unsigned int data_attr)
{
	struct HIF_CE_state *hif_state = (struct HIF_CE_state *)scn->hif_hdl;
	struct HIF_CE_pipe_info *pipe_info = &(hif_state->pipe_info[pipe]);
	struct CE_handle *ce_hdl = pipe_info->ce_hdl;
	int bytes = nbytes, nfrags = 0;
	struct ce_sendlist sendlist;
	int status, i = 0;
	unsigned int mux_id = 0;

	CDF_ASSERT(nbytes <= cdf_nbuf_len(nbuf));

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
		uint32_t frag_paddr;
		int frag_bytes;

		frag_paddr = cdf_nbuf_get_frag_paddr_lo(nbuf, nfrags);
		frag_bytes = cdf_nbuf_get_frag_len(nbuf, nfrags);
		/*
		 * Clear the packet offset for all but the first CE desc.
		 */
		if (i++ > 0)
			data_attr &= ~CDF_CE_TX_PKT_OFFSET_BIT_M;

		status = ce_sendlist_buf_add(&sendlist, frag_paddr,
				    frag_bytes >
				    bytes ? bytes : frag_bytes,
				    cdf_nbuf_get_frag_is_wordstream
				    (nbuf,
				    nfrags) ? 0 :
				    CE_SEND_FLAG_SWAP_DISABLE,
				    data_attr);
		if (status != CDF_STATUS_SUCCESS) {
			HIF_ERROR("%s: error, frag_num %d larger than limit",
				__func__, nfrags);
			return status;
		}
		bytes -= frag_bytes;
		nfrags++;
	} while (bytes > 0);

	/* Make sure we have resources to handle this request */
	cdf_spin_lock_bh(&pipe_info->completion_freeq_lock);
	if (pipe_info->num_sends_allowed < nfrags) {
		cdf_spin_unlock_bh(&pipe_info->completion_freeq_lock);
		ce_pkt_error_count_incr(hif_state, HIF_PIPE_NO_RESOURCE);
		return CDF_STATUS_E_RESOURCES;
	}
	pipe_info->num_sends_allowed -= nfrags;
	cdf_spin_unlock_bh(&pipe_info->completion_freeq_lock);

	if (cdf_unlikely(ce_hdl == NULL)) {
		HIF_ERROR("%s: error CE handle is null", __func__);
		return A_ERROR;
	}

	NBUF_UPDATE_TX_PKT_COUNT(nbuf, NBUF_TX_PKT_HIF);
	DPTRACE(cdf_dp_trace(nbuf, CDF_DP_TRACE_HIF_PACKET_PTR_RECORD,
				(uint8_t *)(cdf_nbuf_data(nbuf)),
				sizeof(cdf_nbuf_data(nbuf))));
	status = ce_sendlist_send(ce_hdl, nbuf, &sendlist, transfer_id);
	CDF_ASSERT(status == CDF_STATUS_SUCCESS);

	return status;
}

void hif_send_complete_check(struct ol_softc *scn, uint8_t pipe, int force)
{
	if (!force) {
		int resources;
		/*
		 * Decide whether to actually poll for completions, or just
		 * wait for a later chance. If there seem to be plenty of
		 * resources left, then just wait, since checking involves
		 * reading a CE register, which is a relatively expensive
		 * operation.
		 */
		resources = hif_get_free_queue_number(scn, pipe);
		/*
		 * If at least 50% of the total resources are still available,
		 * don't bother checking again yet.
		 */
		if (resources > (host_ce_config[pipe].src_nentries >> 1)) {
			return;
		}
	}
#ifdef  ATH_11AC_TXCOMPACT
	ce_per_engine_servicereap(scn, pipe);
#else
	ce_per_engine_service(scn, pipe);
#endif
}

uint16_t hif_get_free_queue_number(struct ol_softc *scn, uint8_t pipe)
{
	struct HIF_CE_state *hif_state = (struct HIF_CE_state *)scn->hif_hdl;
	struct HIF_CE_pipe_info *pipe_info = &(hif_state->pipe_info[pipe]);
	uint16_t rv;

	cdf_spin_lock_bh(&pipe_info->completion_freeq_lock);
	rv = pipe_info->num_sends_allowed;
	cdf_spin_unlock_bh(&pipe_info->completion_freeq_lock);
	return rv;
}

/* Called by lower (CE) layer when a send to Target completes. */
void
hif_pci_ce_send_done(struct CE_handle *copyeng, void *ce_context,
		     void *transfer_context, cdf_dma_addr_t CE_data,
		     unsigned int nbytes, unsigned int transfer_id,
		     unsigned int sw_index, unsigned int hw_index,
		     unsigned int toeplitz_hash_result)
{
	struct HIF_CE_pipe_info *pipe_info =
		(struct HIF_CE_pipe_info *)ce_context;
	struct HIF_CE_state *hif_state = pipe_info->HIF_CE_state;
	unsigned int sw_idx = sw_index, hw_idx = hw_index;
	struct hif_msg_callbacks *msg_callbacks =
		&hif_state->msg_callbacks_current;

	do {
		/*
		 * The upper layer callback will be triggered
		 * when last fragment is complteted.
		 */
		if (transfer_context != CE_SENDLIST_ITEM_CTXT) {
			if (hif_state->scn->target_status
					== OL_TRGET_STATUS_RESET)
				return;

			msg_callbacks->txCompletionHandler(
					msg_callbacks->Context,
					transfer_context, transfer_id,
					toeplitz_hash_result);
		}

		cdf_spin_lock(&pipe_info->completion_freeq_lock);
		pipe_info->num_sends_allowed++;
		cdf_spin_unlock(&pipe_info->completion_freeq_lock);
	} while (ce_completed_send_next(copyeng,
			&ce_context, &transfer_context,
			&CE_data, &nbytes, &transfer_id,
			&sw_idx, &hw_idx,
			&toeplitz_hash_result) == CDF_STATUS_SUCCESS);
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
		cdf_nbuf_t netbuf, int nbytes,
		struct HIF_CE_pipe_info *pipe_info) {
	if (nbytes <= pipe_info->buf_sz) {
		cdf_nbuf_set_pktlen(netbuf, nbytes);
		msg_callbacks->
			rxCompletionHandler(msg_callbacks->Context,
					netbuf, pipe_info->pipe_num);
	} else {
		HIF_ERROR("%s: Invalid Rx msg buf:%p nbytes:%d",
				__func__, netbuf, nbytes);
		cdf_nbuf_free(netbuf);
	}
}

/* Called by lower (CE) layer when data is received from the Target. */
void
hif_pci_ce_recv_data(struct CE_handle *copyeng, void *ce_context,
		     void *transfer_context, cdf_dma_addr_t CE_data,
		     unsigned int nbytes, unsigned int transfer_id,
		     unsigned int flags)
{
	struct HIF_CE_pipe_info *pipe_info =
		(struct HIF_CE_pipe_info *)ce_context;
	struct HIF_CE_state *hif_state = pipe_info->HIF_CE_state;
	struct CE_state *ce_state = (struct CE_state *) copyeng;
	struct ol_softc *scn = hif_state->scn;
	struct hif_msg_callbacks *msg_callbacks =
		&hif_state->msg_callbacks_current;

	do {
		cdf_nbuf_unmap_single(scn->cdf_dev,
				      (cdf_nbuf_t) transfer_context,
				      CDF_DMA_FROM_DEVICE);

		atomic_inc(&pipe_info->recv_bufs_needed);
		hif_post_recv_buffers_for_pipe(pipe_info);
		if (hif_state->scn->target_status == OL_TRGET_STATUS_RESET)
			return;

		hif_ce_do_recv(msg_callbacks, transfer_context,
				nbytes, pipe_info);

		/* Set up force_break flag if num of receices reaches
		 * MAX_NUM_OF_RECEIVES */
		ce_state->receive_count++;
		if (cdf_unlikely(hif_max_num_receives_reached(
				ce_state->receive_count))) {
			ce_state->force_break = 1;
			break;
		}
	} while (ce_completed_recv_next(copyeng, &ce_context, &transfer_context,
					&CE_data, &nbytes, &transfer_id,
					&flags) == CDF_STATUS_SUCCESS);
}

/* TBDXXX: Set CE High Watermark; invoke txResourceAvailHandler in response */

void
hif_post_init(struct ol_softc *scn, void *unused,
	      struct hif_msg_callbacks *callbacks)
{
	struct HIF_CE_state *hif_state = (struct HIF_CE_state *)scn->hif_hdl;

#ifdef CONFIG_ATH_PCIE_ACCESS_DEBUG
	spin_lock_init(&pcie_access_log_lock);
#endif
	/* Save callbacks for later installation */
	cdf_mem_copy(&hif_state->msg_callbacks_pending, callbacks,
		 sizeof(hif_state->msg_callbacks_pending));

}

int hif_completion_thread_startup(struct HIF_CE_state *hif_state)
{
	struct CE_handle *ce_diag = hif_state->ce_diag;
	int pipe_num;
	struct ol_softc *scn = hif_state->scn;
	struct hif_msg_callbacks *hif_msg_callbacks =
		&hif_state->msg_callbacks_current;

	/* daemonize("hif_compl_thread"); */

	if (scn->ce_count == 0) {
		HIF_ERROR("%s: Invalid ce_count\n", __func__);
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
static void hif_msg_callbacks_install(struct ol_softc *scn)
{
	struct HIF_CE_state *hif_state = (struct HIF_CE_state *)scn->hif_hdl;

	cdf_mem_copy(&hif_state->msg_callbacks_current,
		 &hif_state->msg_callbacks_pending,
		 sizeof(hif_state->msg_callbacks_pending));
}

void hif_claim_device(struct ol_softc *scn, void *claimedContext)
{
	struct HIF_CE_state *hif_state = (struct HIF_CE_state *)scn->hif_hdl;

	hif_state->claimedContext = claimedContext;
}

void hif_release_device(struct ol_softc *scn)
{
	struct HIF_CE_state *hif_state = (struct HIF_CE_state *)scn->hif_hdl;

	hif_state->claimedContext = NULL;
}

void
hif_get_default_pipe(struct ol_softc *scn, uint8_t *ULPipe, uint8_t *DLPipe)
{
	int ul_is_polled, dl_is_polled;

	(void)hif_map_service_to_pipe(scn, HTC_CTRL_RSVD_SVC,
		ULPipe, DLPipe, &ul_is_polled, &dl_is_polled);
}

/**
 * hif_dump_pipe_debug_count() - Log error count
 * @scn: ol_softc pointer.
 *
 * Output the pipe error counts of each pipe to log file
 *
 * Return: N/A
 */
void hif_dump_pipe_debug_count(struct ol_softc *scn)
{
	struct HIF_CE_state *hif_state;
	int pipe_num;

	if (scn == NULL) {
		HIF_ERROR("%s scn is NULL", __func__);
		return;
	}
	hif_state = (struct HIF_CE_state *)scn->hif_hdl;
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
	cdf_size_t buf_sz;
	struct HIF_CE_state *hif_state = pipe_info->HIF_CE_state;
	struct ol_softc *scn = hif_state->scn;
	CDF_STATUS ret;
	uint32_t bufs_posted = 0;

	buf_sz = pipe_info->buf_sz;
	if (buf_sz == 0) {
		/* Unused Copy Engine */
		return 0;
	}

	ce_hdl = pipe_info->ce_hdl;

	cdf_spin_lock_bh(&pipe_info->recv_bufs_needed_lock);
	while (atomic_read(&pipe_info->recv_bufs_needed) > 0) {
		cdf_dma_addr_t CE_data;      /* CE space buffer address */
		cdf_nbuf_t nbuf;
		int status;

		atomic_dec(&pipe_info->recv_bufs_needed);
		cdf_spin_unlock_bh(&pipe_info->recv_bufs_needed_lock);

		nbuf = cdf_nbuf_alloc(scn->cdf_dev, buf_sz, 0, 4, false);
		if (!nbuf) {
			cdf_spin_lock_bh(&pipe_info->recv_bufs_needed_lock);
			pipe_info->nbuf_alloc_err_count++;
			cdf_spin_unlock_bh(
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
		 * cdf_nbuf_peek_header(nbuf, &data, &unused);
		 * CE_data = dma_map_single(dev, data, buf_sz, );
		 * DMA_FROM_DEVICE);
		 */
		ret =
			cdf_nbuf_map_single(scn->cdf_dev, nbuf,
					    CDF_DMA_FROM_DEVICE);

		if (unlikely(ret != CDF_STATUS_SUCCESS)) {
			cdf_spin_lock_bh(&pipe_info->recv_bufs_needed_lock);
			pipe_info->nbuf_dma_err_count++;
			cdf_spin_unlock_bh(&pipe_info->recv_bufs_needed_lock);
			HIF_ERROR(
				"%s buf alloc error [%d] needed %d, nbuf_dma_err_count = %u",
				 __func__, pipe_info->pipe_num,
				 atomic_read(&pipe_info->recv_bufs_needed),
				pipe_info->nbuf_dma_err_count);
			cdf_nbuf_free(nbuf);
			atomic_inc(&pipe_info->recv_bufs_needed);
			return 1;
		}

		CE_data = cdf_nbuf_get_frag_paddr_lo(nbuf, 0);

		cdf_os_mem_dma_sync_single_for_device(scn->cdf_dev, CE_data,
					       buf_sz, DMA_FROM_DEVICE);
		status = ce_recv_buf_enqueue(ce_hdl, (void *)nbuf, CE_data);
		CDF_ASSERT(status == CDF_STATUS_SUCCESS);
		if (status != EOK) {
			cdf_spin_lock_bh(&pipe_info->recv_bufs_needed_lock);
			pipe_info->nbuf_ce_enqueue_err_count++;
			cdf_spin_unlock_bh(&pipe_info->recv_bufs_needed_lock);
			HIF_ERROR(
				"%s buf alloc error [%d] needed %d, nbuf_alloc_err_count = %u",
				__func__, pipe_info->pipe_num,
				atomic_read(&pipe_info->recv_bufs_needed),
				pipe_info->nbuf_ce_enqueue_err_count);
			atomic_inc(&pipe_info->recv_bufs_needed);
			cdf_nbuf_free(nbuf);
			return 1;
		}

		cdf_spin_lock_bh(&pipe_info->recv_bufs_needed_lock);
		bufs_posted++;
	}
	pipe_info->nbuf_alloc_err_count =
		(pipe_info->nbuf_alloc_err_count > bufs_posted)?
		pipe_info->nbuf_alloc_err_count - bufs_posted : 0;
	pipe_info->nbuf_dma_err_count =
		(pipe_info->nbuf_dma_err_count > bufs_posted)?
		pipe_info->nbuf_dma_err_count - bufs_posted : 0;
	pipe_info->nbuf_ce_enqueue_err_count =
		(pipe_info->nbuf_ce_enqueue_err_count > bufs_posted)?
	     pipe_info->nbuf_ce_enqueue_err_count - bufs_posted : 0;

	cdf_spin_unlock_bh(&pipe_info->recv_bufs_needed_lock);

	return 0;
}

/*
 * Try to post all desired receive buffers for all pipes.
 * Returns 0 if all desired buffers are posted,
 * non-zero if were were unable to completely
 * replenish receive buffers.
 */
static int hif_post_recv_buffers(struct ol_softc *scn)
{
	struct HIF_CE_state *hif_state = (struct HIF_CE_state *)scn->hif_hdl;
	int pipe_num, rv = 0;

	A_TARGET_ACCESS_LIKELY(scn);
	for (pipe_num = 0; pipe_num < scn->ce_count; pipe_num++) {
		struct HIF_CE_pipe_info *pipe_info;

		pipe_info = &hif_state->pipe_info[pipe_num];
		if (hif_post_recv_buffers_for_pipe(pipe_info)) {
			rv = 1;
			goto done;
		}
	}

done:
	A_TARGET_ACCESS_UNLIKELY(scn);

	return rv;
}

CDF_STATUS hif_start(struct ol_softc *scn)
{
	struct HIF_CE_state *hif_state = (struct HIF_CE_state *)scn->hif_hdl;

	hif_msg_callbacks_install(scn);

	if (hif_completion_thread_startup(hif_state))
		return CDF_STATUS_E_FAILURE;

	/* Post buffers once to start things off. */
	(void)hif_post_recv_buffers(scn);

	hif_state->started = true;

	return CDF_STATUS_SUCCESS;
}

#ifdef WLAN_FEATURE_FASTPATH
/**
 * hif_enable_fastpath() Update that we have enabled fastpath mode
 * @hif_device: HIF context
 *
 * For use in data path
 *
 * Retrun: void
 */
void
hif_enable_fastpath(struct ol_softc *hif_device)
{
	HIF_INFO("Enabling fastpath mode\n");
	hif_device->fastpath_mode_on = 1;
}
#endif /* WLAN_FEATURE_FASTPATH */

void hif_recv_buffer_cleanup_on_pipe(struct HIF_CE_pipe_info *pipe_info)
{
	struct ol_softc *scn;
	struct CE_handle *ce_hdl;
	uint32_t buf_sz;
	struct HIF_CE_state *hif_state;
	cdf_nbuf_t netbuf;
	cdf_dma_addr_t CE_data;
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

	scn = hif_state->scn;
	ce_hdl = pipe_info->ce_hdl;

	if (scn->cdf_dev == NULL) {
		return;
	}
	while (ce_revoke_recv_next
		       (ce_hdl, &per_CE_context, (void **)&netbuf,
			&CE_data) == CDF_STATUS_SUCCESS) {
		cdf_nbuf_unmap_single(scn->cdf_dev, netbuf,
				      CDF_DMA_FROM_DEVICE);
		cdf_nbuf_free(netbuf);
	}
}

void hif_send_buffer_cleanup_on_pipe(struct HIF_CE_pipe_info *pipe_info)
{
	struct CE_handle *ce_hdl;
	struct HIF_CE_state *hif_state;
	cdf_nbuf_t netbuf;
	void *per_CE_context;
	cdf_dma_addr_t CE_data;
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

	ce_hdl = pipe_info->ce_hdl;

	while (ce_cancel_send_next
		       (ce_hdl, &per_CE_context,
		       (void **)&netbuf, &CE_data, &nbytes,
		       &id, &toeplitz_hash_result) == CDF_STATUS_SUCCESS) {
		if (netbuf != CE_SENDLIST_ITEM_CTXT) {
			/*
			 * Packets enqueued by htt_h2t_ver_req_msg() and
			 * htt_h2t_rx_ring_cfg_msg_ll() have already been
			 * freed in htt_htc_misc_pkt_pool_free() in
			 * wlantl_close(), so do not free them here again
			 * by checking whether it's the EndPoint
			 * which they are queued in.
			 */
			if (id == hif_state->scn->htc_endpoint)
				return;
			/* Indicate the completion to higer
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

	for (pipe_num = 0; pipe_num < hif_state->scn->ce_count; pipe_num++) {
		struct HIF_CE_pipe_info *pipe_info;

		pipe_info = &hif_state->pipe_info[pipe_num];
		hif_recv_buffer_cleanup_on_pipe(pipe_info);
		hif_send_buffer_cleanup_on_pipe(pipe_info);
	}
}

void hif_flush_surprise_remove(struct ol_softc *scn)
{
	struct HIF_CE_state *hif_state = (struct HIF_CE_state *)scn->hif_hdl;
	hif_buffer_cleanup(hif_state);
}

void hif_stop(struct ol_softc *scn)
{
	struct HIF_CE_state *hif_state = (struct HIF_CE_state *)scn->hif_hdl;
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
		cdf_softirq_timer_cancel(&hif_state->sleep_timer);
		cdf_softirq_timer_free(&hif_state->sleep_timer);
		hif_state->sleep_timer_init = false;
	}

	hif_state->started = false;
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

#ifdef HIF_PCI
/*
 * Send an interrupt to the device to wake up the Target CPU
 * so it has an opportunity to notice any changed state.
 */
void hif_wake_target_cpu(struct ol_softc *scn)
{
	CDF_STATUS rv;
	uint32_t core_ctrl;

	rv = hif_diag_read_access(scn,
				  SOC_CORE_BASE_ADDRESS | CORE_CTRL_ADDRESS,
				  &core_ctrl);
	CDF_ASSERT(rv == CDF_STATUS_SUCCESS);
	/* A_INUM_FIRMWARE interrupt to Target CPU */
	core_ctrl |= CORE_CTRL_CPU_INTR_MASK;

	rv = hif_diag_write_access(scn,
				   SOC_CORE_BASE_ADDRESS | CORE_CTRL_ADDRESS,
				   core_ctrl);
	CDF_ASSERT(rv == CDF_STATUS_SUCCESS);
}
#endif

static void hif_sleep_entry(void *arg)
{
	struct HIF_CE_state *hif_state = (struct HIF_CE_state *)arg;
	struct ol_softc *scn = hif_state->scn;
	uint32_t idle_ms;
	if (scn->recovery)
		return;

	if (cds_is_unload_in_progress())
		return;

	cdf_spin_lock_irqsave(&hif_state->keep_awake_lock);
	if (hif_state->verified_awake == false) {
		idle_ms = cdf_system_ticks_to_msecs(cdf_system_ticks()
						    - hif_state->sleep_ticks);
		if (idle_ms >= HIF_MIN_SLEEP_INACTIVITY_TIME_MS) {
			if (!cdf_atomic_read(&scn->link_suspended)) {
				soc_wake_reset(scn);
				hif_state->fake_sleep = false;
			}
		} else {
			cdf_softirq_timer_cancel(&hif_state->sleep_timer);
			cdf_softirq_timer_start(&hif_state->sleep_timer,
				    HIF_SLEEP_INACTIVITY_TIMER_PERIOD_MS);
		}
	} else {
		cdf_softirq_timer_cancel(&hif_state->sleep_timer);
		cdf_softirq_timer_start(&hif_state->sleep_timer,
					HIF_SLEEP_INACTIVITY_TIMER_PERIOD_MS);
	}
	cdf_spin_unlock_irqrestore(&hif_state->keep_awake_lock);
}
#define HIF_HIA_MAX_POLL_LOOP    1000000
#define HIF_HIA_POLLING_DELAY_MS 10

#ifndef HIF_PCI
int hif_set_hia(struct ol_softc *scn)
{
	return 0;
}
#else
int hif_set_hia(struct ol_softc *scn)
{
	CDF_STATUS rv;
	uint32_t interconnect_targ_addr = 0;
	uint32_t pcie_state_targ_addr = 0;
	uint32_t pipe_cfg_targ_addr = 0;
	uint32_t svc_to_pipe_map = 0;
	uint32_t pcie_config_flags = 0;
	uint32_t flag2_value = 0;
	uint32_t flag2_targ_addr = 0;
#ifdef QCA_WIFI_3_0
	uint32_t host_interest_area = 0;
	uint8_t i;
#else
	uint32_t ealloc_value = 0;
	uint32_t ealloc_targ_addr = 0;
	uint8_t banks_switched = 1;
	uint32_t chip_id;
#endif
	uint32_t pipe_cfg_addr;

	HIF_TRACE("%s: E", __func__);

	if (IHELIUM_BU || ADRASTEA_BU)
		return CDF_STATUS_SUCCESS;

#ifdef QCA_WIFI_3_0
	i = 0;
	while (i < HIF_HIA_MAX_POLL_LOOP) {
		host_interest_area = hif_read32_mb(scn->mem +
						A_SOC_CORE_SCRATCH_0_ADDRESS);
		if ((host_interest_area & 0x01) == 0) {
			cdf_mdelay(HIF_HIA_POLLING_DELAY_MS);
			host_interest_area = 0;
			i++;
			if (i > HIF_HIA_MAX_POLL_LOOP && (i % 1000 == 0)) {
				HIF_ERROR("%s: poll timeout(%d)", __func__, i);
			}
		} else {
			host_interest_area &= (~0x01);
			hif_write32_mb(scn->mem + 0x113014, 0);
			break;
		}
	}

	if (i >= HIF_HIA_MAX_POLL_LOOP) {
		HIF_ERROR("%s: hia polling timeout", __func__);
		return -EIO;
	}

	if (host_interest_area == 0) {
		HIF_ERROR("%s: host_interest_area = 0", __func__);
		return -EIO;
	}

	interconnect_targ_addr = host_interest_area +
			offsetof(struct host_interest_area_t,
			hi_interconnect_state);

	flag2_targ_addr = host_interest_area +
			offsetof(struct host_interest_area_t, hi_option_flag2);

#else
	interconnect_targ_addr = hif_hia_item_address(scn->target_type,
		offsetof(struct host_interest_s, hi_interconnect_state));
	ealloc_targ_addr = hif_hia_item_address(scn->target_type,
		offsetof(struct host_interest_s, hi_early_alloc));
	flag2_targ_addr = hif_hia_item_address(scn->target_type,
		offsetof(struct host_interest_s, hi_option_flag2));
#endif
	/* Supply Target-side CE configuration */
	rv = hif_diag_read_access(scn, interconnect_targ_addr,
			  &pcie_state_targ_addr);
	if (rv != CDF_STATUS_SUCCESS) {
		HIF_ERROR("%s: interconnect_targ_addr = 0x%0x, ret = %d",
			  __func__, interconnect_targ_addr, rv);
		goto done;
	}
	if (pcie_state_targ_addr == 0) {
		rv = CDF_STATUS_E_FAILURE;
		HIF_ERROR("%s: pcie state addr is 0", __func__);
		goto done;
	}
	pipe_cfg_addr = pcie_state_targ_addr +
			  offsetof(struct pcie_state_s,
			  pipe_cfg_addr);
	rv = hif_diag_read_access(scn,
			  pipe_cfg_addr,
			  &pipe_cfg_targ_addr);
	if (rv != CDF_STATUS_SUCCESS) {
		HIF_ERROR("%s: pipe_cfg_addr = 0x%0x, ret = %d",
			__func__, pipe_cfg_addr, rv);
		goto done;
	}
	if (pipe_cfg_targ_addr == 0) {
		rv = CDF_STATUS_E_FAILURE;
		HIF_ERROR("%s: pipe cfg addr is 0", __func__);
		goto done;
	}

	rv = hif_diag_write_mem(scn, pipe_cfg_targ_addr,
			(uint8_t *) target_ce_config,
			target_ce_config_sz);

	if (rv != CDF_STATUS_SUCCESS) {
		HIF_ERROR("%s: write pipe cfg (%d)", __func__, rv);
		goto done;
	}

	rv = hif_diag_read_access(scn,
			  pcie_state_targ_addr +
			  offsetof(struct pcie_state_s,
			   svc_to_pipe_map),
			  &svc_to_pipe_map);
	if (rv != CDF_STATUS_SUCCESS) {
		HIF_ERROR("%s: get svc/pipe map (%d)", __func__, rv);
		goto done;
	}
	if (svc_to_pipe_map == 0) {
		rv = CDF_STATUS_E_FAILURE;
		HIF_ERROR("%s: svc_to_pipe map is 0", __func__);
		goto done;
	}

	rv = hif_diag_write_mem(scn,
			svc_to_pipe_map,
			(uint8_t *) target_service_to_ce_map,
			target_service_to_ce_map_sz);
	if (rv != CDF_STATUS_SUCCESS) {
		HIF_ERROR("%s: write svc/pipe map (%d)", __func__, rv);
		goto done;
	}

	rv = hif_diag_read_access(scn,
			pcie_state_targ_addr +
			offsetof(struct pcie_state_s,
			config_flags),
			&pcie_config_flags);
	if (rv != CDF_STATUS_SUCCESS) {
		HIF_ERROR("%s: get pcie config_flags (%d)", __func__, rv);
		goto done;
	}
#if (CONFIG_PCIE_ENABLE_L1_CLOCK_GATE)
	pcie_config_flags |= PCIE_CONFIG_FLAG_ENABLE_L1;
#else
	pcie_config_flags &= ~PCIE_CONFIG_FLAG_ENABLE_L1;
#endif /* CONFIG_PCIE_ENABLE_L1_CLOCK_GATE */
	pcie_config_flags |= PCIE_CONFIG_FLAG_CLK_SWITCH_WAIT;
#if (CONFIG_PCIE_ENABLE_AXI_CLK_GATE)
	pcie_config_flags |= PCIE_CONFIG_FLAG_AXI_CLK_GATE;
#endif
	rv = hif_diag_write_mem(scn,
			pcie_state_targ_addr +
			offsetof(struct pcie_state_s,
			config_flags),
			(uint8_t *) &pcie_config_flags,
			sizeof(pcie_config_flags));
	if (rv != CDF_STATUS_SUCCESS) {
		HIF_ERROR("%s: write pcie config_flags (%d)", __func__, rv);
		goto done;
	}

#ifndef QCA_WIFI_3_0
	/* configure early allocation */
	ealloc_targ_addr = hif_hia_item_address(scn->target_type,
							offsetof(
							struct host_interest_s,
							hi_early_alloc));

	rv = hif_diag_read_access(scn, ealloc_targ_addr,
			&ealloc_value);
	if (rv != CDF_STATUS_SUCCESS) {
		HIF_ERROR("%s: get early alloc val (%d)", __func__, rv);
		goto done;
	}

	/* 1 bank is switched to IRAM, except ROME 1.0 */
	ealloc_value |=
		((HI_EARLY_ALLOC_MAGIC << HI_EARLY_ALLOC_MAGIC_SHIFT) &
		 HI_EARLY_ALLOC_MAGIC_MASK);

	rv = hif_diag_read_access(scn,
			  CHIP_ID_ADDRESS |
			  RTC_SOC_BASE_ADDRESS, &chip_id);
	if (rv != CDF_STATUS_SUCCESS) {
		HIF_ERROR("%s: get chip id val (%d)", __func__, rv);
		goto done;
	}
	if (CHIP_ID_VERSION_GET(chip_id) == 0xD) {
		scn->target_revision =
			CHIP_ID_REVISION_GET(chip_id);
		switch (CHIP_ID_REVISION_GET(chip_id)) {
		case 0x2:       /* ROME 1.3 */
			/* 2 banks are switched to IRAM */
			banks_switched = 2;
			break;
		case 0x4:       /* ROME 2.1 */
		case 0x5:       /* ROME 2.2 */
			banks_switched = 6;
			break;
		case 0x8:       /* ROME 3.0 */
		case 0x9:       /* ROME 3.1 */
		case 0xA:       /* ROME 3.2 */
			banks_switched = 9;
			break;
		case 0x0:       /* ROME 1.0 */
		case 0x1:       /* ROME 1.1 */
		default:
			/* 3 banks are switched to IRAM */
			banks_switched = 3;
			break;
		}
	}

	ealloc_value |=
		((banks_switched << HI_EARLY_ALLOC_IRAM_BANKS_SHIFT)
		 & HI_EARLY_ALLOC_IRAM_BANKS_MASK);

	rv = hif_diag_write_access(scn,
				ealloc_targ_addr,
				ealloc_value);
	if (rv != CDF_STATUS_SUCCESS) {
		HIF_ERROR("%s: set early alloc val (%d)", __func__, rv);
		goto done;
	}
#endif

	/* Tell Target to proceed with initialization */
	flag2_targ_addr = hif_hia_item_address(scn->target_type,
						offsetof(
						struct host_interest_s,
						hi_option_flag2));

	rv = hif_diag_read_access(scn, flag2_targ_addr,
			  &flag2_value);
	if (rv != CDF_STATUS_SUCCESS) {
		HIF_ERROR("%s: get option val (%d)", __func__, rv);
		goto done;
	}

	flag2_value |= HI_OPTION_EARLY_CFG_DONE;
	rv = hif_diag_write_access(scn, flag2_targ_addr,
			   flag2_value);
	if (rv != CDF_STATUS_SUCCESS) {
		HIF_ERROR("%s: set option val (%d)", __func__, rv);
		goto done;
	}

	hif_wake_target_cpu(scn);

done:

	return rv;
}
#endif

/**
 * hif_wlan_enable(): call the platform driver to enable wlan
 *
 * This function passes the con_mode and CE configuration to
 * platform driver to enable wlan.
 *
 * Return: void
 */
static int hif_wlan_enable(void)
{
	struct icnss_wlan_enable_cfg cfg;
	enum icnss_driver_mode mode;
	uint32_t con_mode = cds_get_conparam();

	cfg.num_ce_tgt_cfg = target_ce_config_sz /
		sizeof(struct CE_pipe_config);
	cfg.ce_tgt_cfg = (struct ce_tgt_pipe_cfg *)target_ce_config;
	cfg.num_ce_svc_pipe_cfg = target_service_to_ce_map_sz /
		sizeof(struct service_to_pipe);
	cfg.ce_svc_cfg = (struct ce_svc_pipe_cfg *)target_service_to_ce_map;
	cfg.num_shadow_reg_cfg = shadow_cfg_sz / sizeof(struct shadow_reg_cfg);
	cfg.shadow_reg_cfg = (struct icnss_shadow_reg_cfg *) target_shadow_reg_cfg;

	switch (con_mode) {
	case CDF_FTM_MODE:
		mode = ICNSS_FTM;
		break;
	case CDF_EPPING_MODE:
		mode = ICNSS_EPPING;
		break;
	default:
		mode = ICNSS_MISSION;
		break;
	}
	return icnss_wlan_enable(&cfg, mode, QWLAN_VERSIONSTR);
}

#if ((!defined(QCA_WIFI_3_0_IHELIUM) && !defined(QCA_WIFI_3_0_ADRASTEA)) || defined(CONFIG_ICNSS))
static inline void cnss_pcie_notify_q6(void)
{
	return;
}
#endif

/*
 * Called from PCI layer whenever a new PCI device is probed.
 * Initializes per-device HIF state and notifies the main
 * driver that a new HIF device is present.
 */
int hif_config_ce(hif_handle_t hif_hdl)
{
	struct HIF_CE_state *hif_state;
	struct HIF_CE_pipe_info *pipe_info;
	int pipe_num;
#ifdef ADRASTEA_SHADOW_REGISTERS
	int i;
#endif
	CDF_STATUS rv = CDF_STATUS_SUCCESS;
	int ret;
	struct ol_softc *scn = hif_hdl;
	struct icnss_soc_info soc_info;

	/* if epping is enabled we need to use the epping configuration. */
	if (WLAN_IS_EPPING_ENABLED(cds_get_conparam())) {
		if (WLAN_IS_EPPING_IRQ(cds_get_conparam()))
			host_ce_config = host_ce_config_wlan_epping_irq;
		else
			host_ce_config = host_ce_config_wlan_epping_poll;
		target_ce_config = target_ce_config_wlan_epping;
		target_ce_config_sz = sizeof(target_ce_config_wlan_epping);
		target_service_to_ce_map =
		    target_service_to_ce_map_wlan_epping;
		target_service_to_ce_map_sz =
			sizeof(target_service_to_ce_map_wlan_epping);
	}

	ret = hif_wlan_enable();

	if (ret) {
		HIF_ERROR("%s: hif_wlan_enable error = %d", __func__, ret);
		return CDF_STATUS_NOT_INITIALIZED;
	}
	if (IHELIUM_BU) {
		cnss_pcie_notify_q6();
		HIF_TRACE("%s: cnss_pcie_notify_q6 done, notice_send= %d",
			  __func__, scn->notice_send);
	}

	scn->notice_send = true;

	cdf_mem_zero(&soc_info, sizeof(soc_info));
	ret = icnss_get_soc_info(&soc_info);
	if (ret < 0) {
		HIF_ERROR("%s: icnss_get_soc_info error = %d", __func__, ret);
		return CDF_STATUS_NOT_INITIALIZED;
	}

	hif_state = (struct HIF_CE_state *)cdf_mem_malloc(sizeof(*hif_state));
	if (!hif_state) {
		return -ENOMEM;
	}
	cdf_mem_zero(hif_state, sizeof(*hif_state));

	hif_state->scn = scn;
	scn->hif_hdl = hif_state;
	scn->mem = soc_info.v_addr;
	scn->mem_pa = soc_info.p_addr;
	scn->soc_version = soc_info.version;

	cdf_spinlock_init(&hif_state->keep_awake_lock);

	hif_state->keep_awake_count = 0;

	hif_state->fake_sleep = false;
	hif_state->sleep_ticks = 0;
	cdf_softirq_timer_init(NULL, &hif_state->sleep_timer,
			       hif_sleep_entry, (void *)hif_state,
			       CDF_TIMER_TYPE_WAKE_APPS);
	hif_state->sleep_timer_init = true;
	hif_state->fw_indicator_address = FW_INDICATOR_ADDRESS;
#ifdef HIF_PCI
#if CONFIG_ATH_PCIE_MAX_PERF || CONFIG_ATH_PCIE_AWAKE_WHILE_DRIVER_LOAD
	/* Force AWAKE forever/till the driver is loaded */
	if (hif_target_sleep_state_adjust(scn, false, true) < 0)
		return -EACCES;
#endif
#endif

	hif_config_rri_on_ddr(scn);

	/* During CE initializtion */
	scn->ce_count = HOST_CE_COUNT;
	A_TARGET_ACCESS_LIKELY(scn);
	for (pipe_num = 0; pipe_num < scn->ce_count; pipe_num++) {
		struct CE_attr *attr;

		pipe_info = &hif_state->pipe_info[pipe_num];
		pipe_info->pipe_num = pipe_num;
		pipe_info->HIF_CE_state = hif_state;
		attr = &host_ce_config[pipe_num];
		pipe_info->ce_hdl = ce_init(scn, pipe_num, attr);
		CDF_ASSERT(pipe_info->ce_hdl != NULL);
		if (pipe_info->ce_hdl == NULL) {
			rv = CDF_STATUS_E_FAILURE;
			A_TARGET_ACCESS_UNLIKELY(scn);
			goto err;
		}

		if (pipe_num == DIAG_CE_ID) {
			/* Reserve the ultimate CE for
			 * Diagnostic Window support */
			hif_state->ce_diag =
				hif_state->pipe_info[scn->ce_count - 1].ce_hdl;
			continue;
		}

		pipe_info->buf_sz = (cdf_size_t) (attr->src_sz_max);
		cdf_spinlock_init(&pipe_info->recv_bufs_needed_lock);
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

	/*
	 * Initially, establish CE completion handlers for use with BMI.
	 * These are overwritten with generic handlers after we exit BMI phase.
	 */
	pipe_info = &hif_state->pipe_info[BMI_CE_NUM_TO_TARG];
#ifdef HIF_PCI
	ce_send_cb_register(
	   pipe_info->ce_hdl, hif_bmi_send_done, pipe_info, 0);
#ifndef BMI_RSP_POLLING
	pipe_info = &hif_state->pipe_info[BMI_CE_NUM_TO_HOST];
	ce_recv_cb_register(
	   pipe_info->ce_hdl, hif_bmi_recv_data, pipe_info, 0);
#endif
#endif
	HIF_INFO_MED("%s: ce_init done", __func__);

	rv = hif_set_hia(scn);

	HIF_INFO_MED("%s: hif_set_hia done", __func__);

	A_TARGET_ACCESS_UNLIKELY(scn);

	if (rv != CDF_STATUS_SUCCESS)
		goto err;
	else
		init_tasklet_workers();

	HIF_TRACE("%s: X, ret = %d\n", __func__, rv);

#ifdef ADRASTEA_SHADOW_REGISTERS
	HIF_ERROR("Using Shadow Registers instead of CE Registers\n");
	for (i = 0; i < NUM_SHADOW_REGISTERS; i++) {
		HIF_ERROR("%s Shadow Register%d is mapped to address %x\n",
			  __func__, i,
			  (A_TARGET_READ(scn, (SHADOW_ADDRESS(i))) << 2));
	}
#endif


	return rv != CDF_STATUS_SUCCESS;

err:
	/* Failure, so clean up */
	for (pipe_num = 0; pipe_num < scn->ce_count; pipe_num++) {
		pipe_info = &hif_state->pipe_info[pipe_num];
		if (pipe_info->ce_hdl) {
			ce_unregister_irq(hif_state, (1 << pipe_num));
			scn->request_irq_done = false;
			ce_fini(pipe_info->ce_hdl);
			pipe_info->ce_hdl = NULL;
			pipe_info->buf_sz = 0;
		}
	}
	if (hif_state->sleep_timer_init) {
		cdf_softirq_timer_cancel(&hif_state->sleep_timer);
		cdf_softirq_timer_free(&hif_state->sleep_timer);
		hif_state->sleep_timer_init = false;
	}
	if (scn->hif_hdl) {
		scn->hif_hdl = NULL;
		cdf_mem_free(hif_state);
	}
	athdiag_procfs_remove();
	scn->athdiag_procfs_inited = false;
	HIF_TRACE("%s: X, ret = %d\n", __func__, rv);
	return CDF_STATUS_SUCCESS != CDF_STATUS_E_FAILURE;
}






#ifdef IPA_OFFLOAD
void hif_ipa_get_ce_resource(struct ol_softc *scn,
			     uint32_t *ce_sr_base_paddr,
			     uint32_t *ce_sr_ring_size,
			     cdf_dma_addr_t *ce_reg_paddr)
{
	struct HIF_CE_state *hif_state = (struct HIF_CE_state *)scn->hif_hdl;
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

u32 shadow_sr_wr_ind_addr(struct ol_softc *scn, u32 ctrl_addr)
{
	u32 addr = 0;

	switch (COPY_ENGINE_ID(ctrl_addr)) {
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
		HIF_ERROR("invalid CE ctrl_addr\n");
		CDF_ASSERT(0);

	}
	return addr;

}

u32 shadow_dst_wr_ind_addr(struct ol_softc *scn, u32 ctrl_addr)
{
	u32 addr = 0;

	switch (COPY_ENGINE_ID(ctrl_addr)) {
	case 1:
		addr = SHADOW_VALUE13;
		break;
	case 2:
		addr = SHADOW_VALUE14;
		break;
	case 7:
		addr = SHADOW_VALUE19;
		break;
	case 8:
		addr = SHADOW_VALUE20;
		break;
	default:
		HIF_ERROR("invalid CE ctrl_addr\n");
		CDF_ASSERT(0);
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
 * Return: none
 */
void ce_lro_flush_cb_register(struct ol_softc *scn,
	 void (handler)(void *), void *data)
{
	uint8_t ul, dl;
	int ul_polled, dl_polled;

	CDF_ASSERT(scn != NULL);

	if (CDF_STATUS_SUCCESS !=
		 hif_map_service_to_pipe(scn, HTT_DATA_MSG_SVC,
			 &ul, &dl, &ul_polled, &dl_polled)) {
		printk("%s cannot map service to pipe\n", __FUNCTION__);
		return;
	} else {
		struct CE_state *ce_state;
		ce_state = scn->ce_id_to_state[dl];
		ce_state->lro_flush_cb = handler;
		ce_state->lro_data = data;
	}
}

/**
 * ce_lro_flush_cb_deregister() - deregister the LRO flush
 * callback
 * @scn: HIF context
 *
 * Remove the LRO flush callback
 *
 * Return: none
 */
void ce_lro_flush_cb_deregister(struct ol_softc *scn)
{
	uint8_t ul, dl;
	int ul_polled, dl_polled;

	CDF_ASSERT(scn != NULL);

	if (CDF_STATUS_SUCCESS !=
		 hif_map_service_to_pipe(scn, HTT_DATA_MSG_SVC,
			 &ul, &dl, &ul_polled, &dl_polled)) {
		printk("%s cannot map service to pipe\n", __FUNCTION__);
		return;
	} else {
		struct CE_state *ce_state;
		ce_state = scn->ce_id_to_state[dl];
		ce_state->lro_flush_cb = NULL;
		ce_state->lro_data = NULL;
	}
}
#endif

/**
 * hif_map_service_to_pipe() - returns  the ce ids pertaining to
 * this service
 * @scn: ol_softc pointer.
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
 * Return: Indicates whether this operation was successful.
 */

int hif_map_service_to_pipe(struct ol_softc *scn, uint16_t svc_id,
			uint8_t *ul_pipe, uint8_t *dl_pipe, int *ul_is_polled,
			int *dl_is_polled)
{
	int status = CDF_STATUS_SUCCESS;

	unsigned int i;
	struct service_to_pipe element;

	struct service_to_pipe *tgt_svc_map_to_use;
	size_t sz_tgt_svc_map_to_use;

	if (WLAN_IS_EPPING_ENABLED(cds_get_conparam())) {
		tgt_svc_map_to_use = target_service_to_ce_map_wlan_epping;
		sz_tgt_svc_map_to_use =
			sizeof(target_service_to_ce_map_wlan_epping);
	} else {
		tgt_svc_map_to_use = target_service_to_ce_map_wlan;
		sz_tgt_svc_map_to_use = sizeof(target_service_to_ce_map_wlan);
	}

	*dl_is_polled = 0;  /* polling for received messages not supported */

	for (i = 0; i < (sz_tgt_svc_map_to_use/sizeof(element)); i++) {

		memcpy(&element, &tgt_svc_map_to_use[i], sizeof(element));
		if (element.service_id == svc_id) {

			if (element.pipedir == PIPEDIR_OUT)
				*ul_pipe = element.pipenum;

			else if (element.pipedir == PIPEDIR_IN)
				*dl_pipe = element.pipenum;
		}
	}

	*ul_is_polled =
		(host_ce_config[*ul_pipe].flags & CE_ATTR_DISABLE_INTR) != 0;

	return status;
}

#ifdef SHADOW_REG_DEBUG
inline uint32_t DEBUG_CE_SRC_RING_READ_IDX_GET(struct ol_softc *scn,
		uint32_t CE_ctrl_addr)
{
	uint32_t read_from_hw, srri_from_ddr = 0;

	read_from_hw = A_TARGET_READ(scn, CE_ctrl_addr + CURRENT_SRRI_ADDRESS);

	srri_from_ddr = SRRI_FROM_DDR_ADDR(VADDR_FOR_CE(scn, CE_ctrl_addr));

	if (read_from_hw != srri_from_ddr) {
		HIF_ERROR("error: read from ddr = %d actual read from register = %d, CE_MISC_INT_STATUS_GET = 0x%x\n",
		       srri_from_ddr, read_from_hw,
		       CE_MISC_INT_STATUS_GET(scn, CE_ctrl_addr));
		CDF_ASSERT(0);
	}
	return srri_from_ddr;
}


inline uint32_t DEBUG_CE_DEST_RING_READ_IDX_GET(struct ol_softc *scn,
		uint32_t CE_ctrl_addr)
{
	uint32_t read_from_hw, drri_from_ddr = 0;

	read_from_hw = A_TARGET_READ(scn, CE_ctrl_addr + CURRENT_DRRI_ADDRESS);

	drri_from_ddr = DRRI_FROM_DDR_ADDR(VADDR_FOR_CE(scn, CE_ctrl_addr));

	if (read_from_hw != drri_from_ddr) {
		HIF_ERROR("error: read from ddr = %d actual read from register = %d, CE_MISC_INT_STATUS_GET = 0x%x\n",
		       drri_from_ddr, read_from_hw,
		       CE_MISC_INT_STATUS_GET(scn, CE_ctrl_addr));
		CDF_ASSERT(0);
	}
	return drri_from_ddr;
}

#endif

/**
 * hif_get_src_ring_read_index(): Called to get the SRRI
 *
 * @scn: ol_softc pointer
 * @CE_ctrl_addr: base address of the CE whose RRI is to be read
 *
 * This function returns the SRRI to the caller. For CEs that
 * dont have interrupts enabled, we look at the DDR based SRRI
 *
 * Return: SRRI
 */
inline unsigned int hif_get_src_ring_read_index(struct ol_softc *scn,
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
 * @scn: ol_softc pointer
 * @CE_ctrl_addr: base address of the CE whose RRI is to be read
 *
 * This function returns the DRRI to the caller. For CEs that
 * dont have interrupts enabled, we look at the DDR based DRRI
 *
 * Return: DRRI
 */
inline unsigned int hif_get_dst_ring_read_index(struct ol_softc *scn,
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

#ifdef ADRASTEA_RRI_ON_DDR
/**
 * hif_config_rri_on_ddr(): Configure the RRI on DDR mechanism
 *
 * @scn: ol_softc pointer
 *
 * This function allocates non cached memory on ddr and sends
 * the physical address of this memory to the CE hardware. The
 * hardware updates the RRI on this particular location.
 *
 * Return: None
 */
static inline void hif_config_rri_on_ddr(struct ol_softc *scn)
{
	unsigned int i;
	cdf_dma_addr_t paddr_rri_on_ddr;
	uint32_t high_paddr, low_paddr;
	scn->vaddr_rri_on_ddr =
		(uint32_t *)cdf_os_mem_alloc_consistent(scn->cdf_dev,
		(CE_COUNT*sizeof(uint32_t)), &paddr_rri_on_ddr, 0);

	low_paddr  = BITS0_TO_31(paddr_rri_on_ddr);
	high_paddr = BITS32_TO_35(paddr_rri_on_ddr);

	HIF_ERROR("%s using srri and drri from DDR\n", __func__);

	WRITE_CE_DDR_ADDRESS_FOR_RRI_LOW(scn, low_paddr);
	WRITE_CE_DDR_ADDRESS_FOR_RRI_HIGH(scn, high_paddr);

	for (i = 0; i < CE_COUNT; i++)
		CE_IDX_UPD_EN_SET(scn, CE_BASE_ADDRESS(i));

	cdf_mem_zero(scn->vaddr_rri_on_ddr, CE_COUNT*sizeof(uint32_t));

	return;
}
#else

/**
 * hif_config_rri_on_ddr(): Configure the RRI on DDR mechanism
 *
 * @scn: ol_softc pointer
 *
 * This is a dummy implementation for platforms that don't
 * support this functionality.
 *
 * Return: None
 */
static inline void hif_config_rri_on_ddr(struct ol_softc *scn)
{
	return;
}
#endif
