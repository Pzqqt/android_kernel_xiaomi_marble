/*
 * Copyright (c) 2013-2018 The Linux Foundation. All rights reserved.
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

#include "hif.h"
#include "hif_io32.h"
#include "ce_api.h"
#include "ce_main.h"
#include "ce_internal.h"
#include "ce_reg.h"
#include "qdf_lock.h"
#include "regtable.h"
#include "hif_main.h"
#include "hif_debug.h"
#include "hif_napi.h"
#include "qdf_module.h"

#ifdef IPA_OFFLOAD
#ifdef QCA_WIFI_3_0
#define CE_IPA_RING_INIT(ce_desc)                       \
	do {                                            \
		ce_desc->gather = 0;                    \
		ce_desc->enable_11h = 0;                \
		ce_desc->meta_data_low = 0;             \
		ce_desc->packet_result_offset = 64;     \
		ce_desc->toeplitz_hash_enable = 0;      \
		ce_desc->addr_y_search_disable = 0;     \
		ce_desc->addr_x_search_disable = 0;     \
		ce_desc->misc_int_disable = 0;          \
		ce_desc->target_int_disable = 0;        \
		ce_desc->host_int_disable = 0;          \
		ce_desc->dest_byte_swap = 0;            \
		ce_desc->byte_swap = 0;                 \
		ce_desc->type = 2;                      \
		ce_desc->tx_classify = 1;               \
		ce_desc->buffer_addr_hi = 0;            \
		ce_desc->meta_data = 0;                 \
		ce_desc->nbytes = 128;                  \
	} while (0)
#else
#define CE_IPA_RING_INIT(ce_desc)                       \
	do {                                            \
		ce_desc->byte_swap = 0;                 \
		ce_desc->nbytes = 60;                   \
		ce_desc->gather = 0;                    \
	} while (0)
#endif /* QCA_WIFI_3_0 */
#endif /* IPA_OFFLOAD */

#ifndef DATA_CE_SW_INDEX_NO_INLINE_UPDATE
#define DATA_CE_UPDATE_SWINDEX(x, scn, addr)				\
	do {                                            		\
		x = CE_SRC_RING_READ_IDX_GET_FROM_DDR(scn, addr); 	\
	} while (0);
#else
#define DATA_CE_UPDATE_SWINDEX(x, scn, addr)
#endif

static int war1_allow_sleep;
/* io32 write workaround */
static int hif_ce_war1;

/**
 * hif_ce_war_disable() - disable ce war gobally
 */
void hif_ce_war_disable(void)
{
	hif_ce_war1 = 0;
}

/**
 * hif_ce_war_enable() - enable ce war gobally
 */
void hif_ce_war_enable(void)
{
	hif_ce_war1 = 1;
}

/*
 * Note: For MCL, #if defined (HIF_CONFIG_SLUB_DEBUG_ON) needs to be checked
 * for defined here
 */
#if defined(HIF_CONFIG_SLUB_DEBUG_ON) || defined(HIF_CE_DEBUG_DATA_BUF)

#define CE_DEBUG_PRINT_BUF_SIZE(x) (((x) * 3) - 1)
#define CE_DEBUG_DATA_PER_ROW 16

qdf_mutex_t ce_dbg_datamem_lock[CE_COUNT_MAX];
static const char *ce_event_type_to_str(enum hif_ce_event_type type);

/**
 * get_next_record_index() - get the next record index
 * @table_index: atomic index variable to increment
 * @array_size: array size of the circular buffer
 *
 * Increment the atomic index and reserve the value.
 * Takes care of buffer wrap.
 * Guaranteed to be thread safe as long as fewer than array_size contexts
 * try to access the array.  If there are more than array_size contexts
 * trying to access the array, full locking of the recording process would
 * be needed to have sane logging.
 */
static int get_next_record_index(qdf_atomic_t *table_index, int array_size)
{
	int record_index = qdf_atomic_inc_return(table_index);

	if (record_index == array_size)
		qdf_atomic_sub(array_size, table_index);

	while (record_index >= array_size)
		record_index -= array_size;
	return record_index;
}

#if HIF_CE_DEBUG_DATA_BUF
/**
 * hif_ce_desc_data_record() - Record data pointed by the CE descriptor
 * @event: structure detailing a ce event
 * @len: length of the data
 * Return:
 */
static void hif_ce_desc_data_record(struct hif_ce_desc_event *event, int len)
{
	uint8_t *data = NULL;

	if (!event->data)
		return;

	if (event->memory && len > 0)
		data = qdf_nbuf_data((qdf_nbuf_t)event->memory);

	event->actual_data_len = 0;
	qdf_mem_zero(event->data, CE_DEBUG_MAX_DATA_BUF_SIZE);

	if (data && len > 0) {
		qdf_mem_copy(event->data, data,
				((len < CE_DEBUG_MAX_DATA_BUF_SIZE) ?
				 len : CE_DEBUG_MAX_DATA_BUF_SIZE));
		event->actual_data_len = len;
	}
}
#endif

/**
 * hif_record_ce_desc_event() - record ce descriptor events
 * @scn: hif_softc
 * @ce_id: which ce is the event occurring on
 * @type: what happened
 * @descriptor: pointer to the descriptor posted/completed
 * @memory: virtual address of buffer related to the descriptor
 * @index: index that the descriptor was/will be at.
 */
void hif_record_ce_desc_event(struct hif_softc *scn, int ce_id,
				enum hif_ce_event_type type,
				union ce_desc *descriptor,
				void *memory, int index,
				int len)
{
	int record_index;
	struct hif_ce_desc_event *event;

	struct ce_desc_hist *ce_hist = &scn->hif_ce_desc_hist;
	struct hif_ce_desc_event *hist_ev = NULL;

	if (ce_id < CE_COUNT_MAX)
		hist_ev = (struct hif_ce_desc_event *)ce_hist->hist_ev[ce_id];
	else
		return;

	if (ce_id >= CE_COUNT_MAX)
		return;

	if (!ce_hist->enable[ce_id])
		return;

	if (!hist_ev)
		return;

	record_index = get_next_record_index(
			&ce_hist->history_index[ce_id], HIF_CE_HISTORY_MAX);

	event = &hist_ev[record_index];

	event->type = type;
	event->time = qdf_get_log_timestamp();

	if (descriptor != NULL) {
		qdf_mem_copy(&event->descriptor, descriptor, sizeof(union ce_desc));
	} else {
		qdf_mem_zero(&event->descriptor, sizeof(union ce_desc));
	}

	event->memory = memory;
	event->index = index;

#if HIF_CE_DEBUG_DATA_BUF
	if (ce_hist->data_enable[ce_id])
		hif_ce_desc_data_record(event, len);
#endif
}
qdf_export_symbol(hif_record_ce_desc_event);

/**
 * ce_init_ce_desc_event_log() - initialize the ce event log
 * @ce_id: copy engine id for which we are initializing the log
 * @size: size of array to dedicate
 *
 * Currently the passed size is ignored in favor of a precompiled value.
 */
void ce_init_ce_desc_event_log(struct hif_softc *scn, int ce_id, int size)
{
	struct ce_desc_hist *ce_hist = &scn->hif_ce_desc_hist;
	qdf_atomic_init(&ce_hist->history_index[ce_id]);
	qdf_mutex_create(&ce_dbg_datamem_lock[ce_id]);
}

/**
 * ce_deinit_ce_desc_event_log() - deinitialize the ce event log
 * @ce_id: copy engine id for which we are deinitializing the log
 *
 */
inline void ce_deinit_ce_desc_event_log(struct hif_softc *scn, int ce_id)
{
	qdf_mutex_destroy(&ce_dbg_datamem_lock[ce_id]);
}

#else /* Note: For MCL, (HIF_CONFIG_SLUB_DEBUG_ON) || HIF_CE_DEBUG_DATA_BUF */
void hif_record_ce_desc_event(struct hif_softc *scn,
		int ce_id, enum hif_ce_event_type type,
		union ce_desc *descriptor, void *memory,
		int index, int len)
{
}
qdf_export_symbol(hif_record_ce_desc_event);

inline void ce_init_ce_desc_event_log(struct hif_softc *scn, int ce_id,
					int size)
{
}

void ce_deinit_ce_desc_event_log(struct hif_softc *scn, int ce_id)
{
}
#endif /* Note: for MCL, HIF_CONFIG_SLUB_DEBUG_ON || HIF_CE_DEBUG_DATA_BUF */

#ifdef NAPI_YIELD_BUDGET_BASED
bool hif_ce_service_should_yield(struct hif_softc *scn,
				 struct CE_state *ce_state)
{
	bool yield =  hif_max_num_receives_reached(scn, ce_state->receive_count);
	return yield;
}
#else
/**
 * hif_ce_service_should_yield() - return true if the service is hogging the cpu
 * @scn: hif context
 * @ce_state: context of the copy engine being serviced
 *
 * Return: true if the service should yield
 */
bool hif_ce_service_should_yield(struct hif_softc *scn,
				 struct CE_state *ce_state)
{
	bool yield, time_limit_reached, rxpkt_thresh_reached = 0;

	time_limit_reached =
		sched_clock() > ce_state->ce_service_yield_time ? 1 : 0;

	if (!time_limit_reached)
		rxpkt_thresh_reached = hif_max_num_receives_reached
					(scn, ce_state->receive_count);

	yield =  time_limit_reached || rxpkt_thresh_reached;

	if (yield && ce_state->htt_rx_data)
		hif_napi_update_yield_stats(ce_state,
					    time_limit_reached,
					    rxpkt_thresh_reached);
	return yield;
}
#endif

/*
 * Guts of ce_send, used by both ce_send and ce_sendlist_send.
 * The caller takes responsibility for any needed locking.
 */

void war_ce_src_ring_write_idx_set(struct hif_softc *scn,
				   u32 ctrl_addr, unsigned int write_index)
{
	if (hif_ce_war1) {
		void __iomem *indicator_addr;

		indicator_addr = scn->mem + ctrl_addr + DST_WATERMARK_ADDRESS;

		if (!war1_allow_sleep
		    && ctrl_addr == CE_BASE_ADDRESS(CDC_WAR_DATA_CE)) {
			hif_write32_mb(scn, indicator_addr,
				       (CDC_WAR_MAGIC_STR | write_index));
		} else {
			unsigned long irq_flags;

			local_irq_save(irq_flags);
			hif_write32_mb(scn, indicator_addr, 1);

			/*
			 * PCIE write waits for ACK in IPQ8K, there is no
			 * need to read back value.
			 */
			(void)hif_read32_mb(scn, indicator_addr);
			/* conservative */
			(void)hif_read32_mb(scn, indicator_addr);

			CE_SRC_RING_WRITE_IDX_SET(scn,
						  ctrl_addr, write_index);

			hif_write32_mb(scn, indicator_addr, 0);
			local_irq_restore(irq_flags);
		}
	} else {
		CE_SRC_RING_WRITE_IDX_SET(scn, ctrl_addr, write_index);
	}
}

qdf_export_symbol(war_ce_src_ring_write_idx_set);

int
ce_send(struct CE_handle *copyeng,
		void *per_transfer_context,
		qdf_dma_addr_t buffer,
		uint32_t nbytes,
		uint32_t transfer_id,
		uint32_t flags,
		uint32_t user_flag)
{
	struct CE_state *CE_state = (struct CE_state *)copyeng;
	int status;
	struct HIF_CE_state *hif_state = HIF_GET_CE_STATE(CE_state->scn);

	qdf_spin_lock_bh(&CE_state->ce_index_lock);
	status = hif_state->ce_services->ce_send_nolock(copyeng,
			per_transfer_context, buffer, nbytes,
			transfer_id, flags, user_flag);
	qdf_spin_unlock_bh(&CE_state->ce_index_lock);

	return status;
}

unsigned int ce_sendlist_sizeof(void)
{
	return sizeof(struct ce_sendlist);
}

void ce_sendlist_init(struct ce_sendlist *sendlist)
{
	struct ce_sendlist_s *sl = (struct ce_sendlist_s *)sendlist;

	sl->num_items = 0;
}

int
ce_sendlist_buf_add(struct ce_sendlist *sendlist,
					qdf_dma_addr_t buffer,
					uint32_t nbytes,
					uint32_t flags,
					uint32_t user_flags)
{
	struct ce_sendlist_s *sl = (struct ce_sendlist_s *)sendlist;
	unsigned int num_items = sl->num_items;
	struct ce_sendlist_item *item;

	if (num_items >= CE_SENDLIST_ITEMS_MAX) {
		QDF_ASSERT(num_items < CE_SENDLIST_ITEMS_MAX);
		return QDF_STATUS_E_RESOURCES;
	}

	item = &sl->item[num_items];
	item->send_type = CE_SIMPLE_BUFFER_TYPE;
	item->data = buffer;
	item->u.nbytes = nbytes;
	item->flags = flags;
	item->user_flags = user_flags;
	sl->num_items = num_items + 1;
	return QDF_STATUS_SUCCESS;
}

int
ce_sendlist_send(struct CE_handle *copyeng,
		 void *per_transfer_context,
		 struct ce_sendlist *sendlist, unsigned int transfer_id)
{
	struct CE_state *CE_state = (struct CE_state *)copyeng;
	struct HIF_CE_state *hif_state = HIF_GET_CE_STATE(CE_state->scn);

	return hif_state->ce_services->ce_sendlist_send(copyeng,
			per_transfer_context, sendlist, transfer_id);
}

/*
 * TODO : Fast path implementatiom must be de-coupled from generic service
 * APIs shared between SRNG and Legacy CE implementations and must be moved
 * to ce_service_legacy.c.
 * CR-2315620
 */
#ifdef WLAN_FEATURE_FASTPATH
#ifdef QCA_WIFI_3_0
static inline void
ce_buffer_addr_hi_set(struct CE_src_desc *shadow_src_desc,
		      uint64_t dma_addr,
		      uint32_t user_flags)
{
	shadow_src_desc->buffer_addr_hi =
			(uint32_t)((dma_addr >> 32) & 0x1F);
	user_flags |= shadow_src_desc->buffer_addr_hi;
	memcpy(&(((uint32_t *)shadow_src_desc)[1]), &user_flags,
			sizeof(uint32_t));
}
#else
static inline void
ce_buffer_addr_hi_set(struct CE_src_desc *shadow_src_desc,
		      uint64_t dma_addr,
		      uint32_t user_flags)
{
}
#endif

#define SLOTS_PER_DATAPATH_TX 2

/**
 * ce_send_fast() CE layer Tx buffer posting function
 * @copyeng: copy engine handle
 * @msdu: msdu to be sent
 * @transfer_id: transfer_id
 * @download_len: packet download length
 *
 * Assumption : Called with an array of MSDU's
 * Function:
 * For each msdu in the array
 * 1. Check no. of available entries
 * 2. Create src ring entries (allocated in consistent memory
 * 3. Write index to h/w
 *
 * Return: No. of packets that could be sent
 */
int ce_send_fast(struct CE_handle *copyeng, qdf_nbuf_t msdu,
		 unsigned int transfer_id, uint32_t download_len)
{
	struct CE_state *ce_state = (struct CE_state *)copyeng;
	struct hif_softc *scn = ce_state->scn;
	struct hif_opaque_softc *hif_hdl = GET_HIF_OPAQUE_HDL(scn);
	struct CE_ring_state *src_ring = ce_state->src_ring;
	u_int32_t ctrl_addr = ce_state->ctrl_addr;
	unsigned int nentries_mask = src_ring->nentries_mask;
	unsigned int write_index;
	unsigned int sw_index;
	unsigned int frag_len;
	uint64_t dma_addr;
	uint32_t user_flags;
	enum hif_ce_event_type type = FAST_TX_SOFTWARE_INDEX_UPDATE;
	bool ok_to_send = true;

	/*
	 * Create a log assuming the call will go through, and if not, we would
	 * add an error trace as well.
	 * Please add the same failure log for any additional error paths.
	 */
	DPTRACE(qdf_dp_trace(msdu,
			QDF_DP_TRACE_CE_FAST_PACKET_PTR_RECORD,
			QDF_TRACE_DEFAULT_PDEV_ID,
			qdf_nbuf_data_addr(msdu),
			sizeof(qdf_nbuf_data(msdu)), QDF_TX));

	qdf_spin_lock_bh(&ce_state->ce_index_lock);

	/*
	 * Request runtime PM resume if it has already suspended and make
	 * sure there is no PCIe link access.
	 */
	if (hif_pm_runtime_get(hif_hdl) != 0)
		ok_to_send = false;

	if (ok_to_send) {
		Q_TARGET_ACCESS_BEGIN(scn);
		DATA_CE_UPDATE_SWINDEX(src_ring->sw_index, scn, ctrl_addr);
	}

	write_index = src_ring->write_index;
	sw_index = src_ring->sw_index;
	hif_record_ce_desc_event(scn, ce_state->id,
				FAST_TX_SOFTWARE_INDEX_UPDATE,
				NULL, NULL, sw_index, 0);

	if (qdf_unlikely(CE_RING_DELTA(nentries_mask, write_index, sw_index - 1)
			 < SLOTS_PER_DATAPATH_TX)) {
		hif_err_rl("Source ring full, required %d, available %d",
			   SLOTS_PER_DATAPATH_TX,
			   CE_RING_DELTA(nentries_mask, write_index,
					 sw_index - 1));
		OL_ATH_CE_PKT_ERROR_COUNT_INCR(scn, CE_RING_DELTA_FAIL);
		if (ok_to_send)
			Q_TARGET_ACCESS_END(scn);
		qdf_spin_unlock_bh(&ce_state->ce_index_lock);

		DPTRACE(qdf_dp_trace(NULL,
				QDF_DP_TRACE_CE_FAST_PACKET_ERR_RECORD,
				QDF_TRACE_DEFAULT_PDEV_ID,
				NULL, 0, QDF_TX));

		return 0;
	}

	{
		struct CE_src_desc *src_ring_base =
			(struct CE_src_desc *)src_ring->base_addr_owner_space;
		struct CE_src_desc *shadow_base =
			(struct CE_src_desc *)src_ring->shadow_base;
		struct CE_src_desc *src_desc =
			CE_SRC_RING_TO_DESC(src_ring_base, write_index);
		struct CE_src_desc *shadow_src_desc =
			CE_SRC_RING_TO_DESC(shadow_base, write_index);

		hif_pm_runtime_get_noresume(hif_hdl);

		/*
		 * First fill out the ring descriptor for the HTC HTT frame
		 * header. These are uncached writes. Should we use a local
		 * structure instead?
		 */
		/* HTT/HTC header can be passed as a argument */
		dma_addr = qdf_nbuf_get_frag_paddr(msdu, 0);
		shadow_src_desc->buffer_addr = (uint32_t)(dma_addr &
							  0xFFFFFFFF);
		user_flags = qdf_nbuf_data_attr_get(msdu) & DESC_DATA_FLAG_MASK;
		ce_buffer_addr_hi_set(shadow_src_desc, dma_addr, user_flags);
			shadow_src_desc->meta_data = transfer_id;
		shadow_src_desc->nbytes = qdf_nbuf_get_frag_len(msdu, 0);
		ce_validate_nbytes(shadow_src_desc->nbytes, ce_state);
		download_len -= shadow_src_desc->nbytes;
		/*
		 * HTC HTT header is a word stream, so byte swap if CE byte
		 * swap enabled
		 */
		shadow_src_desc->byte_swap = ((ce_state->attr_flags &
					CE_ATTR_BYTE_SWAP_DATA) != 0);
		/* For the first one, it still does not need to write */
		shadow_src_desc->gather = 1;
		*src_desc = *shadow_src_desc;
		/* By default we could initialize the transfer context to this
		 * value
		 */
		src_ring->per_transfer_context[write_index] =
			CE_SENDLIST_ITEM_CTXT;
		write_index = CE_RING_IDX_INCR(nentries_mask, write_index);

		src_desc = CE_SRC_RING_TO_DESC(src_ring_base, write_index);
		shadow_src_desc = CE_SRC_RING_TO_DESC(shadow_base, write_index);
		/*
		 * Now fill out the ring descriptor for the actual data
		 * packet
		 */
		dma_addr = qdf_nbuf_get_frag_paddr(msdu, 1);
		shadow_src_desc->buffer_addr = (uint32_t)(dma_addr &
							  0xFFFFFFFF);
		/*
		 * Clear packet offset for all but the first CE desc.
		 */
		user_flags &= ~QDF_CE_TX_PKT_OFFSET_BIT_M;
		ce_buffer_addr_hi_set(shadow_src_desc, dma_addr, user_flags);
		shadow_src_desc->meta_data = transfer_id;

		/* get actual packet length */
		frag_len = qdf_nbuf_get_frag_len(msdu, 1);

		/* download remaining bytes of payload */
		shadow_src_desc->nbytes =  download_len;
		ce_validate_nbytes(shadow_src_desc->nbytes, ce_state);
		if (shadow_src_desc->nbytes > frag_len)
			shadow_src_desc->nbytes = frag_len;

		/*  Data packet is a byte stream, so disable byte swap */
		shadow_src_desc->byte_swap = 0;
		/* For the last one, gather is not set */
		shadow_src_desc->gather    = 0;
		*src_desc = *shadow_src_desc;
		src_ring->per_transfer_context[write_index] = msdu;

		hif_record_ce_desc_event(scn, ce_state->id, type,
					(union ce_desc *)src_desc,
				src_ring->per_transfer_context[write_index],
					write_index, shadow_src_desc->nbytes);

		write_index = CE_RING_IDX_INCR(nentries_mask, write_index);

		DPTRACE(qdf_dp_trace(msdu,
			QDF_DP_TRACE_CE_FAST_PACKET_PTR_RECORD,
			QDF_TRACE_DEFAULT_PDEV_ID, qdf_nbuf_data_addr(msdu),
			sizeof(qdf_nbuf_data(msdu)), QDF_TX));
	}

	src_ring->write_index = write_index;

	if (ok_to_send) {
		if (qdf_likely(ce_state->state == CE_RUNNING)) {
			type = FAST_TX_WRITE_INDEX_UPDATE;
			war_ce_src_ring_write_idx_set(scn, ctrl_addr,
				write_index);
			Q_TARGET_ACCESS_END(scn);
		} else
			ce_state->state = CE_PENDING;
		hif_pm_runtime_put(hif_hdl);
	}

	qdf_spin_unlock_bh(&ce_state->ce_index_lock);

	/* sent 1 packet */
	return 1;
}

/**
 * ce_is_fastpath_handler_registered() - return true for datapath CEs and if
 * fastpath is enabled.
 * @ce_state: handle to copy engine
 *
 * Return: true if fastpath handler is registered for datapath CE.
 */
static bool ce_is_fastpath_handler_registered(struct CE_state *ce_state)
{
	if (ce_state->fastpath_handler)
		return true;
	else
		return false;
}

#else
static inline bool ce_is_fastpath_handler_registered(struct CE_state *ce_state)
{
	return false;
}
#endif /* WLAN_FEATURE_FASTPATH */

#ifndef AH_NEED_TX_DATA_SWAP
#define AH_NEED_TX_DATA_SWAP 0
#endif

/**
 * ce_batch_send() - sends bunch of msdus at once
 * @ce_tx_hdl : pointer to CE handle
 * @msdu : list of msdus to be sent
 * @transfer_id : transfer id
 * @len : Downloaded length
 * @sendhead : sendhead
 *
 * Assumption : Called with an array of MSDU's
 * Function:
 * For each msdu in the array
 * 1. Send each msdu
 * 2. Increment write index accordinlgy.
 *
 * Return: list of msds not sent
 */
qdf_nbuf_t ce_batch_send(struct CE_handle *ce_tx_hdl,  qdf_nbuf_t msdu,
		uint32_t transfer_id, u_int32_t len, uint32_t sendhead)
{
	struct CE_state *ce_state = (struct CE_state *)ce_tx_hdl;
	struct hif_softc *scn = ce_state->scn;
	struct CE_ring_state *src_ring = ce_state->src_ring;
	u_int32_t ctrl_addr = ce_state->ctrl_addr;
	/*  A_target_id_t targid = TARGID(scn);*/

	uint32_t nentries_mask = src_ring->nentries_mask;
	uint32_t sw_index, write_index;

	struct CE_src_desc *src_desc_base =
		(struct CE_src_desc *)src_ring->base_addr_owner_space;
	uint32_t *src_desc;

	struct CE_src_desc lsrc_desc = {0};
	int deltacount = 0;
	qdf_nbuf_t freelist = NULL, hfreelist = NULL, tempnext;

	DATA_CE_UPDATE_SWINDEX(src_ring->sw_index, scn, ctrl_addr);
	sw_index = src_ring->sw_index;
	write_index = src_ring->write_index;

	deltacount = CE_RING_DELTA(nentries_mask, write_index, sw_index-1);

	while (msdu) {
		tempnext = qdf_nbuf_next(msdu);

		if (deltacount < 2) {
			if (sendhead)
				return msdu;
			HIF_ERROR("%s: Out of descriptors", __func__);
			src_ring->write_index = write_index;
			war_ce_src_ring_write_idx_set(scn, ctrl_addr,
					write_index);

			sw_index = src_ring->sw_index;
			write_index = src_ring->write_index;

			deltacount = CE_RING_DELTA(nentries_mask, write_index,
					sw_index-1);
			if (freelist == NULL) {
				freelist = msdu;
				hfreelist = msdu;
			} else {
				qdf_nbuf_set_next(freelist, msdu);
				freelist = msdu;
			}
			qdf_nbuf_set_next(msdu, NULL);
			msdu = tempnext;
			continue;
		}

		src_desc = (uint32_t *)CE_SRC_RING_TO_DESC(src_desc_base,
				write_index);

		src_desc[0]   = qdf_nbuf_get_frag_paddr(msdu, 0);

		lsrc_desc.meta_data = transfer_id;
		if (len  > msdu->len)
			len =  msdu->len;
		lsrc_desc.nbytes = len;
		/*  Data packet is a byte stream, so disable byte swap */
		lsrc_desc.byte_swap = AH_NEED_TX_DATA_SWAP;
		lsrc_desc.gather    = 0; /*For the last one, gather is not set*/

		src_desc[1] = ((uint32_t *)&lsrc_desc)[1];


		src_ring->per_transfer_context[write_index] = msdu;
		write_index = CE_RING_IDX_INCR(nentries_mask, write_index);

		if (sendhead)
			break;
		qdf_nbuf_set_next(msdu, NULL);
		msdu = tempnext;

	}


	src_ring->write_index = write_index;
	war_ce_src_ring_write_idx_set(scn, ctrl_addr, write_index);

	return hfreelist;
}

/**
 * ce_update_tx_ring() - Advance sw index.
 * @ce_tx_hdl : pointer to CE handle
 * @num_htt_cmpls : htt completions received.
 *
 * Function:
 * Increment the value of sw index of src ring
 * according to number of htt completions
 * received.
 *
 * Return: void
 */
#ifdef DATA_CE_SW_INDEX_NO_INLINE_UPDATE
void ce_update_tx_ring(struct CE_handle *ce_tx_hdl, uint32_t num_htt_cmpls)
{
	struct CE_state *ce_state = (struct CE_state *)ce_tx_hdl;
	struct CE_ring_state *src_ring = ce_state->src_ring;
	uint32_t nentries_mask = src_ring->nentries_mask;
	/*
	 * Advance the s/w index:
	 * This effectively simulates completing the CE ring descriptors
	 */
	src_ring->sw_index =
		CE_RING_IDX_ADD(nentries_mask, src_ring->sw_index,
				num_htt_cmpls);
}
#else
void ce_update_tx_ring(struct CE_handle *ce_tx_hdl, uint32_t num_htt_cmpls)
{}
#endif

/**
 * ce_send_single() - sends
 * @ce_tx_hdl : pointer to CE handle
 * @msdu : msdu to be sent
 * @transfer_id : transfer id
 * @len : Downloaded length
 *
 * Function:
 * 1. Send one msdu
 * 2. Increment write index of src ring accordinlgy.
 *
 * Return: int: CE sent status
 */
int ce_send_single(struct CE_handle *ce_tx_hdl, qdf_nbuf_t msdu,
		uint32_t transfer_id, u_int32_t len)
{
	struct CE_state *ce_state = (struct CE_state *)ce_tx_hdl;
	struct hif_softc *scn = ce_state->scn;
	struct CE_ring_state *src_ring = ce_state->src_ring;
	uint32_t ctrl_addr = ce_state->ctrl_addr;
	/*A_target_id_t targid = TARGID(scn);*/

	uint32_t nentries_mask = src_ring->nentries_mask;
	uint32_t sw_index, write_index;

	struct CE_src_desc *src_desc_base =
		(struct CE_src_desc *)src_ring->base_addr_owner_space;
	uint32_t *src_desc;

	struct CE_src_desc lsrc_desc = {0};
	enum hif_ce_event_type event_type;

	DATA_CE_UPDATE_SWINDEX(src_ring->sw_index, scn, ctrl_addr);
	sw_index = src_ring->sw_index;
	write_index = src_ring->write_index;

	if (qdf_unlikely(CE_RING_DELTA(nentries_mask, write_index,
					sw_index-1) < 1)) {
		/* ol_tx_stats_inc_ring_error(sc->scn->pdev_txrx_handle, 1); */
		HIF_ERROR("%s: ce send fail %d %d %d", __func__, nentries_mask,
			  write_index, sw_index);
		return 1;
	}

	src_desc = (uint32_t *)CE_SRC_RING_TO_DESC(src_desc_base, write_index);

	src_desc[0] = qdf_nbuf_get_frag_paddr(msdu, 0);

	lsrc_desc.meta_data = transfer_id;
	lsrc_desc.nbytes = len;
	/*  Data packet is a byte stream, so disable byte swap */
	lsrc_desc.byte_swap = AH_NEED_TX_DATA_SWAP;
	lsrc_desc.gather    = 0; /* For the last one, gather is not set */

	src_desc[1] = ((uint32_t *)&lsrc_desc)[1];


	src_ring->per_transfer_context[write_index] = msdu;

	if (((struct CE_src_desc *)src_desc)->gather)
		event_type = HIF_TX_GATHER_DESC_POST;
	else if (qdf_unlikely(ce_state->state != CE_RUNNING))
		event_type = HIF_TX_DESC_SOFTWARE_POST;
	else
		event_type = HIF_TX_DESC_POST;

	hif_record_ce_desc_event(scn, ce_state->id, event_type,
				(union ce_desc *)src_desc, msdu,
				write_index, len);

	write_index = CE_RING_IDX_INCR(nentries_mask, write_index);

	src_ring->write_index = write_index;

	war_ce_src_ring_write_idx_set(scn, ctrl_addr, write_index);

	return QDF_STATUS_SUCCESS;
}

/**
 * ce_recv_buf_enqueue() - enqueue a recv buffer into a copy engine
 * @coyeng: copy engine handle
 * @per_recv_context: virtual address of the nbuf
 * @buffer: physical address of the nbuf
 *
 * Return: 0 if the buffer is enqueued
 */
int
ce_recv_buf_enqueue(struct CE_handle *copyeng,
		    void *per_recv_context, qdf_dma_addr_t buffer)
{
	struct CE_state *CE_state = (struct CE_state *)copyeng;
	struct HIF_CE_state *hif_state = HIF_GET_CE_STATE(CE_state->scn);

	return hif_state->ce_services->ce_recv_buf_enqueue(copyeng,
			per_recv_context, buffer);
}

void
ce_send_watermarks_set(struct CE_handle *copyeng,
		       unsigned int low_alert_nentries,
		       unsigned int high_alert_nentries)
{
	struct CE_state *CE_state = (struct CE_state *)copyeng;
	uint32_t ctrl_addr = CE_state->ctrl_addr;
	struct hif_softc *scn = CE_state->scn;

	CE_SRC_RING_LOWMARK_SET(scn, ctrl_addr, low_alert_nentries);
	CE_SRC_RING_HIGHMARK_SET(scn, ctrl_addr, high_alert_nentries);
}

void
ce_recv_watermarks_set(struct CE_handle *copyeng,
		       unsigned int low_alert_nentries,
		       unsigned int high_alert_nentries)
{
	struct CE_state *CE_state = (struct CE_state *)copyeng;
	uint32_t ctrl_addr = CE_state->ctrl_addr;
	struct hif_softc *scn = CE_state->scn;

	CE_DEST_RING_LOWMARK_SET(scn, ctrl_addr,
				low_alert_nentries);
	CE_DEST_RING_HIGHMARK_SET(scn, ctrl_addr,
				high_alert_nentries);
}

unsigned int ce_send_entries_avail(struct CE_handle *copyeng)
{
	struct CE_state *CE_state = (struct CE_state *)copyeng;
	struct CE_ring_state *src_ring = CE_state->src_ring;
	unsigned int nentries_mask = src_ring->nentries_mask;
	unsigned int sw_index;
	unsigned int write_index;

	qdf_spin_lock(&CE_state->ce_index_lock);
	sw_index = src_ring->sw_index;
	write_index = src_ring->write_index;
	qdf_spin_unlock(&CE_state->ce_index_lock);

	return CE_RING_DELTA(nentries_mask, write_index, sw_index - 1);
}

unsigned int ce_recv_entries_avail(struct CE_handle *copyeng)
{
	struct CE_state *CE_state = (struct CE_state *)copyeng;
	struct CE_ring_state *dest_ring = CE_state->dest_ring;
	unsigned int nentries_mask = dest_ring->nentries_mask;
	unsigned int sw_index;
	unsigned int write_index;

	qdf_spin_lock(&CE_state->ce_index_lock);
	sw_index = dest_ring->sw_index;
	write_index = dest_ring->write_index;
	qdf_spin_unlock(&CE_state->ce_index_lock);

	return CE_RING_DELTA(nentries_mask, write_index, sw_index - 1);
}

/*
 * Guts of ce_send_entries_done.
 * The caller takes responsibility for any necessary locking.
 */
unsigned int ce_send_entries_done(struct CE_handle *copyeng)
{
	struct CE_state *CE_state = (struct CE_state *)copyeng;
	unsigned int nentries;
	struct hif_softc *scn = CE_state->scn;
	struct HIF_CE_state *hif_state = HIF_GET_CE_STATE(scn);

	qdf_spin_lock(&CE_state->ce_index_lock);
	nentries = hif_state->ce_services->ce_send_entries_done_nolock(
						CE_state->scn, CE_state);
	qdf_spin_unlock(&CE_state->ce_index_lock);

	return nentries;
}

/*
 * Guts of ce_recv_entries_done.
 * The caller takes responsibility for any necessary locking.
 */
unsigned int ce_recv_entries_done(struct CE_handle *copyeng)
{
	struct CE_state *CE_state = (struct CE_state *)copyeng;
	unsigned int nentries;
	struct hif_softc *scn = CE_state->scn;
	struct HIF_CE_state *hif_state = HIF_GET_CE_STATE(scn);

	qdf_spin_lock(&CE_state->ce_index_lock);
	nentries = hif_state->ce_services->ce_recv_entries_done_nolock(
						CE_state->scn, CE_state);
	qdf_spin_unlock(&CE_state->ce_index_lock);

	return nentries;
}

/*
 * Guts of ce_completed_recv_next.
 * The caller takes responsibility for any necessary locking.
 */
int
ce_completed_recv_next(struct CE_handle *copyeng,
		       void **per_CE_contextp,
		       void **per_transfer_contextp,
		       qdf_dma_addr_t *bufferp,
		       unsigned int *nbytesp,
		       unsigned int *transfer_idp, unsigned int *flagsp)
{
	struct CE_state *CE_state = (struct CE_state *)copyeng;
	int status;
	struct hif_softc *scn = CE_state->scn;
	struct HIF_CE_state *hif_state = HIF_GET_CE_STATE(scn);
	struct ce_ops *ce_services;

	ce_services = hif_state->ce_services;
	qdf_spin_lock_bh(&CE_state->ce_index_lock);
	status =
		ce_services->ce_completed_recv_next_nolock(CE_state,
				per_CE_contextp, per_transfer_contextp, bufferp,
					      nbytesp, transfer_idp, flagsp);
	qdf_spin_unlock_bh(&CE_state->ce_index_lock);

	return status;
}

QDF_STATUS
ce_revoke_recv_next(struct CE_handle *copyeng,
		    void **per_CE_contextp,
		    void **per_transfer_contextp, qdf_dma_addr_t *bufferp)
{
	struct CE_state *CE_state = (struct CE_state *)copyeng;
	struct HIF_CE_state *hif_state = HIF_GET_CE_STATE(CE_state->scn);

	return hif_state->ce_services->ce_revoke_recv_next(copyeng,
			per_CE_contextp, per_transfer_contextp, bufferp);
}

QDF_STATUS
ce_cancel_send_next(struct CE_handle *copyeng,
		void **per_CE_contextp,
		void **per_transfer_contextp,
		qdf_dma_addr_t *bufferp,
		unsigned int *nbytesp,
		unsigned int *transfer_idp,
		uint32_t *toeplitz_hash_result)
{
	struct CE_state *CE_state = (struct CE_state *)copyeng;
	struct HIF_CE_state *hif_state = HIF_GET_CE_STATE(CE_state->scn);

	return hif_state->ce_services->ce_cancel_send_next
		(copyeng, per_CE_contextp, per_transfer_contextp,
		 bufferp, nbytesp, transfer_idp, toeplitz_hash_result);
}

int
ce_completed_send_next(struct CE_handle *copyeng,
		       void **per_CE_contextp,
		       void **per_transfer_contextp,
		       qdf_dma_addr_t *bufferp,
		       unsigned int *nbytesp,
		       unsigned int *transfer_idp,
		       unsigned int *sw_idx,
		       unsigned int *hw_idx,
		       unsigned int *toeplitz_hash_result)
{
	struct CE_state *CE_state = (struct CE_state *)copyeng;
	struct hif_softc *scn = CE_state->scn;
	struct HIF_CE_state *hif_state = HIF_GET_CE_STATE(scn);
	struct ce_ops *ce_services;
	int status;

	ce_services = hif_state->ce_services;
	qdf_spin_lock_bh(&CE_state->ce_index_lock);
	status =
		ce_services->ce_completed_send_next_nolock(CE_state,
					per_CE_contextp, per_transfer_contextp,
					bufferp, nbytesp, transfer_idp, sw_idx,
					      hw_idx, toeplitz_hash_result);
	qdf_spin_unlock_bh(&CE_state->ce_index_lock);

	return status;
}

#ifdef ATH_11AC_TXCOMPACT
/* CE engine descriptor reap
 * Similar to ce_per_engine_service , Only difference is ce_per_engine_service
 * does receive and reaping of completed descriptor ,
 * This function only handles reaping of Tx complete descriptor.
 * The Function is called from threshold reap  poll routine
 * hif_send_complete_check so should not countain receive functionality
 * within it .
 */

void ce_per_engine_servicereap(struct hif_softc *scn, unsigned int ce_id)
{
	void *CE_context;
	void *transfer_context;
	qdf_dma_addr_t buf;
	unsigned int nbytes;
	unsigned int id;
	unsigned int sw_idx, hw_idx;
	uint32_t toeplitz_hash_result;
	struct CE_state *CE_state = scn->ce_id_to_state[ce_id];
	struct HIF_CE_state *hif_state = HIF_GET_CE_STATE(scn);

	if (Q_TARGET_ACCESS_BEGIN(scn) < 0)
		return;

	hif_record_ce_desc_event(scn, ce_id, HIF_CE_REAP_ENTRY,
			NULL, NULL, 0, 0);

	/* Since this function is called from both user context and
	 * tasklet context the spinlock has to lock the bottom halves.
	 * This fix assumes that ATH_11AC_TXCOMPACT flag is always
	 * enabled in TX polling mode. If this is not the case, more
	 * bottom halve spin lock changes are needed. Due to data path
	 * performance concern, after internal discussion we've decided
	 * to make minimum change, i.e., only address the issue occurred
	 * in this function. The possible negative effect of this minimum
	 * change is that, in the future, if some other function will also
	 * be opened to let the user context to use, those cases need to be
	 * addressed by change spin_lock to spin_lock_bh also.
	 */

	qdf_spin_lock_bh(&CE_state->ce_index_lock);

	if (CE_state->send_cb) {
		{
			struct ce_ops *ce_services = hif_state->ce_services;
			/* Pop completed send buffers and call the
			 * registered send callback for each
			 */
			while (ce_services->ce_completed_send_next_nolock
				 (CE_state, &CE_context,
				  &transfer_context, &buf,
				  &nbytes, &id, &sw_idx, &hw_idx,
				  &toeplitz_hash_result) ==
				  QDF_STATUS_SUCCESS) {
				if (ce_id != CE_HTT_H2T_MSG) {
					qdf_spin_unlock_bh(
						&CE_state->ce_index_lock);
					CE_state->send_cb(
						(struct CE_handle *)
						CE_state, CE_context,
						transfer_context, buf,
						nbytes, id, sw_idx, hw_idx,
						toeplitz_hash_result);
					qdf_spin_lock_bh(
						&CE_state->ce_index_lock);
				} else {
					struct HIF_CE_pipe_info *pipe_info =
						(struct HIF_CE_pipe_info *)
						CE_context;

					qdf_spin_lock_bh(&pipe_info->
						 completion_freeq_lock);
					pipe_info->num_sends_allowed++;
					qdf_spin_unlock_bh(&pipe_info->
						   completion_freeq_lock);
				}
			}
		}
	}

	qdf_spin_unlock_bh(&CE_state->ce_index_lock);

	hif_record_ce_desc_event(scn, ce_id, HIF_CE_REAP_EXIT,
			NULL, NULL, 0, 0);
	Q_TARGET_ACCESS_END(scn);
}

#endif /*ATH_11AC_TXCOMPACT */

/*
 * Number of times to check for any pending tx/rx completion on
 * a copy engine, this count should be big enough. Once we hit
 * this threashold we'll not check for any Tx/Rx comlpetion in same
 * interrupt handling. Note that this threashold is only used for
 * Rx interrupt processing, this can be used tor Tx as well if we
 * suspect any infinite loop in checking for pending Tx completion.
 */
#define CE_TXRX_COMP_CHECK_THRESHOLD 20

/*
 * TODO : Fast path implementatiom must be de-coupled from generic service
 * APIs shared between SRNG and Legacy CE implementations and must be moved
 * to ce_service_legacy.c.
 * CR-2315620
 */
#ifdef WLAN_FEATURE_FASTPATH
/**
 * ce_fastpath_rx_handle() - Updates write_index and calls fastpath msg handler
 * @ce_state: handle to copy engine state
 * @cmpl_msdus: Rx msdus
 * @num_cmpls: number of Rx msdus
 * @ctrl_addr: CE control address
 *
 * Return: None
 */
static void ce_fastpath_rx_handle(struct CE_state *ce_state,
				  qdf_nbuf_t *cmpl_msdus, uint32_t num_cmpls,
				  uint32_t ctrl_addr)
{
	struct hif_softc *scn = ce_state->scn;
	struct CE_ring_state *dest_ring = ce_state->dest_ring;
	uint32_t nentries_mask = dest_ring->nentries_mask;
	uint32_t write_index;

	qdf_spin_unlock(&ce_state->ce_index_lock);
	(ce_state->fastpath_handler)(ce_state->context,	cmpl_msdus, num_cmpls);
	qdf_spin_lock(&ce_state->ce_index_lock);

	/* Update Destination Ring Write Index */
	write_index = dest_ring->write_index;
	write_index = CE_RING_IDX_ADD(nentries_mask, write_index, num_cmpls);

	hif_record_ce_desc_event(scn, ce_state->id,
			FAST_RX_WRITE_INDEX_UPDATE,
			NULL, NULL, write_index, 0);

	CE_DEST_RING_WRITE_IDX_SET(scn, ctrl_addr, write_index);
	dest_ring->write_index = write_index;
}

/**
 * ce_per_engine_service_fast() - CE handler routine to service fastpath msgs
 * @scn: hif_context
 * @ce_id: Copy engine ID
 * 1) Go through the CE ring, and find the completions
 * 2) For valid completions retrieve context (nbuf) for per_transfer_context[]
 * 3) Unmap buffer & accumulate in an array.
 * 4) Call message handler when array is full or when exiting the handler
 *
 * Return: void
 */

static void ce_per_engine_service_fast(struct hif_softc *scn, int ce_id)
{
	struct CE_state *ce_state = scn->ce_id_to_state[ce_id];
	struct hif_opaque_softc *hif_hdl = GET_HIF_OPAQUE_HDL(scn);
	struct CE_ring_state *dest_ring = ce_state->dest_ring;
	struct CE_dest_desc *dest_ring_base =
		(struct CE_dest_desc *)dest_ring->base_addr_owner_space;

	uint32_t nentries_mask = dest_ring->nentries_mask;
	uint32_t sw_index = dest_ring->sw_index;
	uint32_t nbytes;
	qdf_nbuf_t nbuf;
	dma_addr_t paddr;
	struct CE_dest_desc *dest_desc;
	qdf_nbuf_t cmpl_msdus[MSG_FLUSH_NUM];
	uint32_t ctrl_addr = ce_state->ctrl_addr;
	uint32_t nbuf_cmpl_idx = 0;
	unsigned int more_comp_cnt = 0;
	struct HIF_CE_state *hif_state = HIF_GET_CE_STATE(scn);
	struct ce_ops *ce_services = hif_state->ce_services;

more_data:
	for (;;) {

		dest_desc = CE_DEST_RING_TO_DESC(dest_ring_base,
						 sw_index);

		/*
		 * The following 2 reads are from non-cached memory
		 */
		nbytes = dest_desc->nbytes;

		/* If completion is invalid, break */
		if (qdf_unlikely(nbytes == 0))
			break;


		/*
		 * Build the nbuf list from valid completions
		 */
		nbuf = dest_ring->per_transfer_context[sw_index];

		/*
		 * No lock is needed here, since this is the only thread
		 * that accesses the sw_index
		 */
		sw_index = CE_RING_IDX_INCR(nentries_mask, sw_index);

		/*
		 * CAREFUL : Uncached write, but still less expensive,
		 * since most modern caches use "write-combining" to
		 * flush multiple cache-writes all at once.
		 */
		dest_desc->nbytes = 0;

		/*
		 * Per our understanding this is not required on our
		 * since we are doing the same cache invalidation
		 * operation on the same buffer twice in succession,
		 * without any modifiication to this buffer by CPU in
		 * between.
		 * However, this code with 2 syncs in succession has
		 * been undergoing some testing at a customer site,
		 * and seemed to be showing no problems so far. Would
		 * like to validate from the customer, that this line
		 * is really not required, before we remove this line
		 * completely.
		 */
		paddr = QDF_NBUF_CB_PADDR(nbuf);

		qdf_mem_dma_sync_single_for_cpu(scn->qdf_dev, paddr,
				(skb_end_pointer(nbuf) - (nbuf)->data),
				DMA_FROM_DEVICE);

		qdf_nbuf_put_tail(nbuf, nbytes);

		qdf_assert_always(nbuf->data != NULL);

		QDF_NBUF_CB_RX_CTX_ID(nbuf) =
				hif_get_rx_ctx_id(ce_state->id, hif_hdl);
		cmpl_msdus[nbuf_cmpl_idx++] = nbuf;

		/*
		 * we are not posting the buffers back instead
		 * reusing the buffers
		 */
		if (nbuf_cmpl_idx == scn->ce_service_max_rx_ind_flush) {
			hif_record_ce_desc_event(scn, ce_state->id,
						 FAST_RX_SOFTWARE_INDEX_UPDATE,
						 NULL, NULL, sw_index, 0);
			dest_ring->sw_index = sw_index;
			ce_fastpath_rx_handle(ce_state, cmpl_msdus,
						nbuf_cmpl_idx, ctrl_addr);

			ce_state->receive_count += nbuf_cmpl_idx;
			if (qdf_unlikely(hif_ce_service_should_yield(
						scn, ce_state))) {
				ce_state->force_break = 1;
				qdf_atomic_set(&ce_state->rx_pending, 1);
				return;
			}

			nbuf_cmpl_idx = 0;
			more_comp_cnt = 0;
		}
	}

	hif_record_ce_desc_event(scn, ce_state->id,
				 FAST_RX_SOFTWARE_INDEX_UPDATE,
				 NULL, NULL, sw_index, 0);

	dest_ring->sw_index = sw_index;

	/*
	 * If there are not enough completions to fill the array,
	 * just call the message handler here
	 */
	if (nbuf_cmpl_idx) {
		ce_fastpath_rx_handle(ce_state, cmpl_msdus,
				      nbuf_cmpl_idx, ctrl_addr);

		ce_state->receive_count += nbuf_cmpl_idx;
		if (qdf_unlikely(hif_ce_service_should_yield(scn, ce_state))) {
			ce_state->force_break = 1;
			qdf_atomic_set(&ce_state->rx_pending, 1);
			return;
		}

		/* check for more packets after upper layer processing */
		nbuf_cmpl_idx = 0;
		more_comp_cnt = 0;
		goto more_data;
	}

	hif_update_napi_max_poll_time(ce_state, ce_id, qdf_get_cpu());

	qdf_atomic_set(&ce_state->rx_pending, 0);
	if (TARGET_REGISTER_ACCESS_ALLOWED(scn)) {
		CE_ENGINE_INT_STATUS_CLEAR(scn, ctrl_addr,
					   HOST_IS_COPY_COMPLETE_MASK);
	} else {
		hif_err_rl("%s: target access is not allowed", __func__);
		return;
	}

	if (ce_services->ce_recv_entries_done_nolock(scn, ce_state)) {
		if (more_comp_cnt++ < CE_TXRX_COMP_CHECK_THRESHOLD) {
			goto more_data;
		} else {
			HIF_ERROR("%s:Potential infinite loop detected during Rx processing nentries_mask:0x%x sw read_idx:0x%x hw read_idx:0x%x",
				  __func__, nentries_mask,
				  ce_state->dest_ring->sw_index,
				  CE_DEST_RING_READ_IDX_GET(scn, ctrl_addr));
		}
	}
#ifdef NAPI_YIELD_BUDGET_BASED
	/* Caution : Before you modify this code, please refer hif_napi_poll function
	to understand how napi_complete gets called and make the necessary changes
	Force break has to be done till WIN disables the interrupt at source */
	ce_state->force_break = 1;
#endif
}

#else
static void ce_per_engine_service_fast(struct hif_softc *scn, int ce_id)
{
}
#endif /* WLAN_FEATURE_FASTPATH */

/*
 * Guts of interrupt handler for per-engine interrupts on a particular CE.
 *
 * Invokes registered callbacks for recv_complete,
 * send_complete, and watermarks.
 *
 * Returns: number of messages processed
 */
int ce_per_engine_service(struct hif_softc *scn, unsigned int CE_id)
{
	struct CE_state *CE_state = scn->ce_id_to_state[CE_id];
	uint32_t ctrl_addr = CE_state->ctrl_addr;
	struct HIF_CE_state *hif_state = HIF_GET_CE_STATE(scn);
	void *CE_context;
	void *transfer_context;
	qdf_dma_addr_t buf;
	unsigned int nbytes;
	unsigned int id;
	unsigned int flags;
	unsigned int more_comp_cnt = 0;
	unsigned int more_snd_comp_cnt = 0;
	unsigned int sw_idx, hw_idx;
	uint32_t toeplitz_hash_result;
	uint32_t mode = hif_get_conparam(scn);

	if (hif_is_nss_wifi_enabled(scn) && (CE_state->htt_rx_data))
		return CE_state->receive_count;

	if (Q_TARGET_ACCESS_BEGIN(scn) < 0) {
		HIF_ERROR("[premature rc=0]");
		return 0; /* no work done */
	}

	/* Clear force_break flag and re-initialize receive_count to 0 */
	CE_state->receive_count = 0;
	CE_state->force_break = 0;
	CE_state->ce_service_start_time = sched_clock();
	CE_state->ce_service_yield_time =
		CE_state->ce_service_start_time +
		hif_get_ce_service_max_yield_time(
			(struct hif_opaque_softc *)scn);

	qdf_spin_lock(&CE_state->ce_index_lock);
	/*
	 * With below check we make sure CE we are handling is datapath CE and
	 * fastpath is enabled.
	 */
	if (ce_is_fastpath_handler_registered(CE_state)) {
		/* For datapath only Rx CEs */
		ce_per_engine_service_fast(scn, CE_id);
		goto unlock_end;
	}

more_completions:
	if (CE_state->recv_cb) {

		/* Pop completed recv buffers and call
		 * the registered recv callback for each
		 */
		while (hif_state->ce_services->ce_completed_recv_next_nolock
				(CE_state, &CE_context, &transfer_context,
				&buf, &nbytes, &id, &flags) ==
				QDF_STATUS_SUCCESS) {
			qdf_spin_unlock(&CE_state->ce_index_lock);
			CE_state->recv_cb((struct CE_handle *)CE_state,
					  CE_context, transfer_context, buf,
					  nbytes, id, flags);

			/*
			 * EV #112693 -
			 * [Peregrine][ES1][WB342][Win8x86][Performance]
			 * BSoD_0x133 occurred in VHT80 UDP_DL
			 * Break out DPC by force if number of loops in
			 * hif_pci_ce_recv_data reaches MAX_NUM_OF_RECEIVES
			 * to avoid spending too long time in
			 * DPC for each interrupt handling. Schedule another
			 * DPC to avoid data loss if we had taken
			 * force-break action before apply to Windows OS
			 * only currently, Linux/MAC os can expand to their
			 * platform if necessary
			 */

			/* Break the receive processes by
			 * force if force_break set up
			 */
			if (qdf_unlikely(CE_state->force_break)) {
				qdf_atomic_set(&CE_state->rx_pending, 1);
				goto target_access_end;
			}
			qdf_spin_lock(&CE_state->ce_index_lock);
		}
	}

	/*
	 * Attention: We may experience potential infinite loop for below
	 * While Loop during Sending Stress test.
	 * Resolve the same way as Receive Case (Refer to EV #112693)
	 */

	if (CE_state->send_cb) {
		/* Pop completed send buffers and call
		 * the registered send callback for each
		 */

#ifdef ATH_11AC_TXCOMPACT
		while (hif_state->ce_services->ce_completed_send_next_nolock
			 (CE_state, &CE_context,
			 &transfer_context, &buf, &nbytes,
			 &id, &sw_idx, &hw_idx,
			 &toeplitz_hash_result) == QDF_STATUS_SUCCESS) {

			if (CE_id != CE_HTT_H2T_MSG ||
			    QDF_IS_EPPING_ENABLED(mode)) {
				qdf_spin_unlock(&CE_state->ce_index_lock);
				CE_state->send_cb((struct CE_handle *)CE_state,
						  CE_context, transfer_context,
						  buf, nbytes, id, sw_idx,
						  hw_idx, toeplitz_hash_result);
				qdf_spin_lock(&CE_state->ce_index_lock);
			} else {
				struct HIF_CE_pipe_info *pipe_info =
					(struct HIF_CE_pipe_info *)CE_context;

				qdf_spin_lock_bh(&pipe_info->
					      completion_freeq_lock);
				pipe_info->num_sends_allowed++;
				qdf_spin_unlock_bh(&pipe_info->
						completion_freeq_lock);
			}
		}
#else                           /*ATH_11AC_TXCOMPACT */
		while (hif_state->ce_services->ce_completed_send_next_nolock
			 (CE_state, &CE_context,
			  &transfer_context, &buf, &nbytes,
			  &id, &sw_idx, &hw_idx,
			  &toeplitz_hash_result) == QDF_STATUS_SUCCESS) {
			qdf_spin_unlock(&CE_state->ce_index_lock);
			CE_state->send_cb((struct CE_handle *)CE_state,
				  CE_context, transfer_context, buf,
				  nbytes, id, sw_idx, hw_idx,
				  toeplitz_hash_result);
			qdf_spin_lock(&CE_state->ce_index_lock);
		}
#endif /*ATH_11AC_TXCOMPACT */
	}

more_watermarks:
	if (CE_state->misc_cbs) {
		if (CE_state->watermark_cb &&
				hif_state->ce_services->watermark_int(CE_state,
					&flags)) {
			qdf_spin_unlock(&CE_state->ce_index_lock);
			/* Convert HW IS bits to software flags */
			CE_state->watermark_cb((struct CE_handle *)CE_state,
					CE_state->wm_context, flags);
			qdf_spin_lock(&CE_state->ce_index_lock);
		}
	}

	/*
	 * Clear the misc interrupts (watermark) that were handled above,
	 * and that will be checked again below.
	 * Clear and check for copy-complete interrupts again, just in case
	 * more copy completions happened while the misc interrupts were being
	 * handled.
	 */
	if (!ce_srng_based(scn)) {
		if (TARGET_REGISTER_ACCESS_ALLOWED(scn)) {
			CE_ENGINE_INT_STATUS_CLEAR(scn, ctrl_addr,
					   CE_WATERMARK_MASK |
					   HOST_IS_COPY_COMPLETE_MASK);
		} else {
			hif_err_rl("%s: target access is not allowed",
				   __func__);
			goto unlock_end;
		}
	}

	/*
	 * Now that per-engine interrupts are cleared, verify that
	 * no recv interrupts arrive while processing send interrupts,
	 * and no recv or send interrupts happened while processing
	 * misc interrupts.Go back and check again.Keep checking until
	 * we find no more events to process.
	 */
	if (CE_state->recv_cb &&
		hif_state->ce_services->ce_recv_entries_done_nolock(scn,
				CE_state)) {
		if (QDF_IS_EPPING_ENABLED(mode) ||
		    more_comp_cnt++ < CE_TXRX_COMP_CHECK_THRESHOLD) {
			goto more_completions;
		} else {
			if (!ce_srng_based(scn)) {
				HIF_ERROR(
					"%s:Potential infinite loop detected during Rx processing nentries_mask:0x%x sw read_idx:0x%x hw read_idx:0x%x",
					__func__,
					CE_state->dest_ring->nentries_mask,
					CE_state->dest_ring->sw_index,
					CE_DEST_RING_READ_IDX_GET(scn,
							  CE_state->ctrl_addr));
			}
		}
	}

	if (CE_state->send_cb &&
		hif_state->ce_services->ce_send_entries_done_nolock(scn,
				CE_state)) {
		if (QDF_IS_EPPING_ENABLED(mode) ||
		    more_snd_comp_cnt++ < CE_TXRX_COMP_CHECK_THRESHOLD) {
			goto more_completions;
		} else {
			if (!ce_srng_based(scn)) {
				HIF_ERROR(
					"%s:Potential infinite loop detected during send completion nentries_mask:0x%x sw read_idx:0x%x hw read_idx:0x%x",
					__func__,
					CE_state->src_ring->nentries_mask,
					CE_state->src_ring->sw_index,
					CE_SRC_RING_READ_IDX_GET(scn,
							 CE_state->ctrl_addr));
			}
		}
	}

	if (CE_state->misc_cbs && CE_state->watermark_cb) {
		if (hif_state->ce_services->watermark_int(CE_state, &flags))
			goto more_watermarks;
	}

	qdf_atomic_set(&CE_state->rx_pending, 0);

unlock_end:
	qdf_spin_unlock(&CE_state->ce_index_lock);
target_access_end:
	if (Q_TARGET_ACCESS_END(scn) < 0)
		HIF_ERROR("<--[premature rc=%d]", CE_state->receive_count);
	return CE_state->receive_count;
}
qdf_export_symbol(ce_per_engine_service);

/*
 * Handler for per-engine interrupts on ALL active CEs.
 * This is used in cases where the system is sharing a
 * single interrput for all CEs
 */

void ce_per_engine_service_any(int irq, struct hif_softc *scn)
{
	int CE_id;
	uint32_t intr_summary;

	if (Q_TARGET_ACCESS_BEGIN(scn) < 0)
		return;

	if (!qdf_atomic_read(&scn->tasklet_from_intr)) {
		for (CE_id = 0; CE_id < scn->ce_count; CE_id++) {
			struct CE_state *CE_state = scn->ce_id_to_state[CE_id];

			if (qdf_atomic_read(&CE_state->rx_pending)) {
				qdf_atomic_set(&CE_state->rx_pending, 0);
				ce_per_engine_service(scn, CE_id);
			}
		}

		Q_TARGET_ACCESS_END(scn);
		return;
	}

	intr_summary = CE_INTERRUPT_SUMMARY(scn);

	for (CE_id = 0; intr_summary && (CE_id < scn->ce_count); CE_id++) {
		if (intr_summary & (1 << CE_id))
			intr_summary &= ~(1 << CE_id);
		else
			continue;       /* no intr pending on this CE */

		ce_per_engine_service(scn, CE_id);
	}

	Q_TARGET_ACCESS_END(scn);
}

/*Iterate the CE_state list and disable the compl interrupt
 * if it has been registered already.
 */
void ce_disable_any_copy_compl_intr_nolock(struct hif_softc *scn)
{
	int CE_id;

	if (Q_TARGET_ACCESS_BEGIN(scn) < 0)
		return;

	for (CE_id = 0; CE_id < scn->ce_count; CE_id++) {
		struct CE_state *CE_state = scn->ce_id_to_state[CE_id];
		uint32_t ctrl_addr = CE_state->ctrl_addr;

		/* if the interrupt is currently enabled, disable it */
		if (!CE_state->disable_copy_compl_intr
		    && (CE_state->send_cb || CE_state->recv_cb))
			CE_COPY_COMPLETE_INTR_DISABLE(scn, ctrl_addr);

		if (CE_state->watermark_cb)
			CE_WATERMARK_INTR_DISABLE(scn, ctrl_addr);
	}
	Q_TARGET_ACCESS_END(scn);
}

void ce_enable_any_copy_compl_intr_nolock(struct hif_softc *scn)
{
	int CE_id;

	if (Q_TARGET_ACCESS_BEGIN(scn) < 0)
		return;

	for (CE_id = 0; CE_id < scn->ce_count; CE_id++) {
		struct CE_state *CE_state = scn->ce_id_to_state[CE_id];
		uint32_t ctrl_addr = CE_state->ctrl_addr;

		/*
		 * If the CE is supposed to have copy complete interrupts
		 * enabled (i.e. there a callback registered, and the
		 * "disable" flag is not set), then re-enable the interrupt.
		 */
		if (!CE_state->disable_copy_compl_intr
		    && (CE_state->send_cb || CE_state->recv_cb))
			CE_COPY_COMPLETE_INTR_ENABLE(scn, ctrl_addr);

		if (CE_state->watermark_cb)
			CE_WATERMARK_INTR_ENABLE(scn, ctrl_addr);
	}
	Q_TARGET_ACCESS_END(scn);
}

/**
 * ce_send_cb_register(): register completion handler
 * @copyeng: CE_state representing the ce we are adding the behavior to
 * @fn_ptr: callback that the ce should use when processing tx completions
 * @disable_interrupts: if the interupts should be enabled or not.
 *
 * Caller should guarantee that no transactions are in progress before
 * switching the callback function.
 *
 * Registers the send context before the fn pointer so that if the cb is valid
 * the context should be valid.
 *
 * Beware that currently this function will enable completion interrupts.
 */
void
ce_send_cb_register(struct CE_handle *copyeng,
		    ce_send_cb fn_ptr,
		    void *ce_send_context, int disable_interrupts)
{
	struct CE_state *CE_state = (struct CE_state *)copyeng;
	struct hif_softc *scn;
	struct HIF_CE_state *hif_state;

	if (CE_state == NULL) {
		HIF_ERROR("%s: Error CE state = NULL", __func__);
		return;
	}
	scn = CE_state->scn;
	hif_state = HIF_GET_CE_STATE(scn);
	if (hif_state == NULL) {
		HIF_ERROR("%s: Error HIF state = NULL", __func__);
		return;
	}
	CE_state->send_context = ce_send_context;
	CE_state->send_cb = fn_ptr;
	hif_state->ce_services->ce_per_engine_handler_adjust(CE_state,
							disable_interrupts);
}

/**
 * ce_recv_cb_register(): register completion handler
 * @copyeng: CE_state representing the ce we are adding the behavior to
 * @fn_ptr: callback that the ce should use when processing rx completions
 * @disable_interrupts: if the interupts should be enabled or not.
 *
 * Registers the send context before the fn pointer so that if the cb is valid
 * the context should be valid.
 *
 * Caller should guarantee that no transactions are in progress before
 * switching the callback function.
 */
void
ce_recv_cb_register(struct CE_handle *copyeng,
		    CE_recv_cb fn_ptr,
		    void *CE_recv_context, int disable_interrupts)
{
	struct CE_state *CE_state = (struct CE_state *)copyeng;
	struct hif_softc *scn;
	struct HIF_CE_state *hif_state;

	if (CE_state == NULL) {
		HIF_ERROR("%s: ERROR CE state = NULL", __func__);
		return;
	}
	scn = CE_state->scn;
	hif_state = HIF_GET_CE_STATE(scn);
	if (hif_state == NULL) {
		HIF_ERROR("%s: Error HIF state = NULL", __func__);
		return;
	}
	CE_state->recv_context = CE_recv_context;
	CE_state->recv_cb = fn_ptr;
	hif_state->ce_services->ce_per_engine_handler_adjust(CE_state,
							disable_interrupts);
}

/**
 * ce_watermark_cb_register(): register completion handler
 * @copyeng: CE_state representing the ce we are adding the behavior to
 * @fn_ptr: callback that the ce should use when processing watermark events
 *
 * Caller should guarantee that no watermark events are being processed before
 * switching the callback function.
 */
void
ce_watermark_cb_register(struct CE_handle *copyeng,
			 CE_watermark_cb fn_ptr, void *CE_wm_context)
{
	struct CE_state *CE_state = (struct CE_state *)copyeng;
	struct hif_softc *scn = CE_state->scn;
	struct HIF_CE_state *hif_state = HIF_GET_CE_STATE(scn);

	CE_state->watermark_cb = fn_ptr;
	CE_state->wm_context = CE_wm_context;
	hif_state->ce_services->ce_per_engine_handler_adjust(CE_state,
							0);
	if (fn_ptr)
		CE_state->misc_cbs = 1;
}

bool ce_get_rx_pending(struct hif_softc *scn)
{
	int CE_id;

	for (CE_id = 0; CE_id < scn->ce_count; CE_id++) {
		struct CE_state *CE_state = scn->ce_id_to_state[CE_id];

		if (qdf_atomic_read(&CE_state->rx_pending))
			return true;
	}

	return false;
}

/**
 * ce_check_rx_pending() - ce_check_rx_pending
 * @CE_state: context of the copy engine to check
 *
 * Return: true if there per_engine_service
 *	didn't process all the rx descriptors.
 */
bool ce_check_rx_pending(struct CE_state *CE_state)
{
	if (qdf_atomic_read(&CE_state->rx_pending))
		return true;
	else
		return false;
}
qdf_export_symbol(ce_check_rx_pending);

#ifdef IPA_OFFLOAD
/**
 * ce_ipa_get_resource() - get uc resource on copyengine
 * @ce: copyengine context
 * @ce_sr: copyengine source ring resource info
 * @ce_sr_ring_size: copyengine source ring size
 * @ce_reg_paddr: copyengine register physical address
 *
 * Copy engine should release resource to micro controller
 * Micro controller needs
 *  - Copy engine source descriptor base address
 *  - Copy engine source descriptor size
 *  - PCI BAR address to access copy engine regiser
 *
 * Return: None
 */
void ce_ipa_get_resource(struct CE_handle *ce,
			 qdf_shared_mem_t **ce_sr,
			 uint32_t *ce_sr_ring_size,
			 qdf_dma_addr_t *ce_reg_paddr)
{
	struct CE_state *CE_state = (struct CE_state *)ce;
	uint32_t ring_loop;
	struct CE_src_desc *ce_desc;
	qdf_dma_addr_t phy_mem_base;
	struct hif_softc *scn = CE_state->scn;

	if (CE_UNUSED == CE_state->state) {
		*qdf_mem_get_dma_addr_ptr(scn->qdf_dev,
			&CE_state->scn->ipa_ce_ring->mem_info) = 0;
		*ce_sr_ring_size = 0;
		return;
	}

	/* Update default value for descriptor */
	for (ring_loop = 0; ring_loop < CE_state->src_ring->nentries;
	     ring_loop++) {
		ce_desc = (struct CE_src_desc *)
			  ((char *)CE_state->src_ring->base_addr_owner_space +
			   ring_loop * (sizeof(struct CE_src_desc)));
		CE_IPA_RING_INIT(ce_desc);
	}

	/* Get BAR address */
	hif_read_phy_mem_base(CE_state->scn, &phy_mem_base);

	*ce_sr = CE_state->scn->ipa_ce_ring;
	*ce_sr_ring_size = (uint32_t)(CE_state->src_ring->nentries *
		sizeof(struct CE_src_desc));
	*ce_reg_paddr = phy_mem_base + CE_BASE_ADDRESS(CE_state->id) +
			SR_WR_INDEX_ADDRESS;
}
#endif /* IPA_OFFLOAD */

#if HIF_CE_DEBUG_DATA_BUF
/**
 * hif_dump_desc_data_buf() - record ce descriptor events
 * @buf: buffer to copy to
 * @pos: Current position till which the buf is filled
 * @data: Data to be copied
 * @data_len: Length of the data to be copied
 */
static uint32_t hif_dump_desc_data_buf(uint8_t *buf, ssize_t pos,
					uint8_t *data, uint32_t data_len)
{
	pos += snprintf(buf + pos, PAGE_SIZE - pos, "Data:(Max%dBytes)\n",
			CE_DEBUG_MAX_DATA_BUF_SIZE);

	if ((data_len > 0) && data) {
		if (data_len < 16) {
			hex_dump_to_buffer(data,
						CE_DEBUG_DATA_PER_ROW,
						16, 1, buf + pos,
						(ssize_t)PAGE_SIZE - pos,
						false);
			pos += CE_DEBUG_PRINT_BUF_SIZE(data_len);
			pos += snprintf(buf + pos, PAGE_SIZE - pos, "\n");
		} else {
			uint32_t rows = (data_len / 16) + 1;
			uint32_t row = 0;

			for (row = 0; row < rows; row++) {
				hex_dump_to_buffer(data + (row * 16),
							CE_DEBUG_DATA_PER_ROW,
							16, 1, buf + pos,
							(ssize_t)PAGE_SIZE
							- pos, false);
				pos +=
				CE_DEBUG_PRINT_BUF_SIZE(CE_DEBUG_DATA_PER_ROW);
				pos += snprintf(buf + pos, PAGE_SIZE - pos,
						"\n");
			}
		}
	}

	return pos;
}
#endif

/*
 * Note: For MCL, #if defined (HIF_CONFIG_SLUB_DEBUG_ON) needs to be checked
 * for defined here
 */
#if defined(HIF_CONFIG_SLUB_DEBUG_ON) || defined(HIF_CE_DEBUG_DATA_BUF)
static const char *ce_event_type_to_str(enum hif_ce_event_type type)
{
	switch (type) {
	case HIF_RX_DESC_POST:
		return "HIF_RX_DESC_POST";
	case HIF_RX_DESC_COMPLETION:
		return "HIF_RX_DESC_COMPLETION";
	case HIF_TX_GATHER_DESC_POST:
		return "HIF_TX_GATHER_DESC_POST";
	case HIF_TX_DESC_POST:
		return "HIF_TX_DESC_POST";
	case HIF_TX_DESC_SOFTWARE_POST:
		return "HIF_TX_DESC_SOFTWARE_POST";
	case HIF_TX_DESC_COMPLETION:
		return "HIF_TX_DESC_COMPLETION";
	case FAST_RX_WRITE_INDEX_UPDATE:
		return "FAST_RX_WRITE_INDEX_UPDATE";
	case FAST_RX_SOFTWARE_INDEX_UPDATE:
		return "FAST_RX_SOFTWARE_INDEX_UPDATE";
	case FAST_TX_WRITE_INDEX_UPDATE:
		return "FAST_TX_WRITE_INDEX_UPDATE";
	case FAST_TX_WRITE_INDEX_SOFTWARE_UPDATE:
		return "FAST_TX_WRITE_INDEX_SOFTWARE_UPDATE";
	case FAST_TX_SOFTWARE_INDEX_UPDATE:
		return "FAST_TX_SOFTWARE_INDEX_UPDATE";
	case RESUME_WRITE_INDEX_UPDATE:
		return "RESUME_WRITE_INDEX_UPDATE";
	case HIF_IRQ_EVENT:
		return "HIF_IRQ_EVENT";
	case HIF_CE_TASKLET_ENTRY:
		return "HIF_CE_TASKLET_ENTRY";
	case HIF_CE_TASKLET_RESCHEDULE:
		return "HIF_CE_TASKLET_RESCHEDULE";
	case HIF_CE_TASKLET_EXIT:
		return "HIF_CE_TASKLET_EXIT";
	case HIF_CE_REAP_ENTRY:
		return "HIF_CE_REAP_ENTRY";
	case HIF_CE_REAP_EXIT:
		return "HIF_CE_REAP_EXIT";
	case NAPI_SCHEDULE:
		return "NAPI_SCHEDULE";
	case NAPI_POLL_ENTER:
		return "NAPI_POLL_ENTER";
	case NAPI_COMPLETE:
		return "NAPI_COMPLETE";
	case NAPI_POLL_EXIT:
		return "NAPI_POLL_EXIT";
	case HIF_RX_NBUF_ALLOC_FAILURE:
		return "HIF_RX_NBUF_ALLOC_FAILURE";
	case HIF_RX_NBUF_MAP_FAILURE:
		return "HIF_RX_NBUF_MAP_FAILURE";
	case HIF_RX_NBUF_ENQUEUE_FAILURE:
		return "HIF_RX_NBUF_ENQUEUE_FAILURE";
	default:
		return "invalid";
	}
}

/**
 * hif_dump_desc_event() - record ce descriptor events
 * @buf: Buffer to which to be copied
 * @ce_id: which ce is the event occurring on
 * @index: index that the descriptor was/will be at.
 */
ssize_t hif_dump_desc_event(struct hif_softc *scn, char *buf)
{
	struct hif_ce_desc_event *event;
	uint64_t secs, usecs;
	ssize_t len = 0;
	struct ce_desc_hist *ce_hist = NULL;
	struct hif_ce_desc_event *hist_ev = NULL;

	if (!scn)
		return -EINVAL;

	ce_hist = &scn->hif_ce_desc_hist;

	hist_ev =
		(struct hif_ce_desc_event *)ce_hist->hist_ev[ce_hist->hist_id];

	if (!hist_ev) {
		qdf_print("Low Memory");
		return -EINVAL;
	}

	event = &hist_ev[ce_hist->hist_index];

	if ((ce_hist->hist_id >= CE_COUNT_MAX) ||
		(ce_hist->hist_index >= HIF_CE_HISTORY_MAX)) {
		qdf_print("Invalid values");
		return -EINVAL;
	}

	qdf_log_timestamp_to_secs(event->time, &secs, &usecs);

	len += snprintf(buf, PAGE_SIZE - len,
			"\nTime:%lld.%06lld, CE:%d, EventType: %s, EventIndex: %d\nDataAddr=%pK",
			secs, usecs, ce_hist->hist_id,
			ce_event_type_to_str(event->type),
			event->index, event->memory);
#if HIF_CE_DEBUG_DATA_BUF
	len += snprintf(buf + len, PAGE_SIZE - len, ", Data len=%d",
			event->actual_data_len);
#endif

	len += snprintf(buf + len, PAGE_SIZE - len, "\nCE descriptor: ");

	hex_dump_to_buffer(&event->descriptor, sizeof(union ce_desc),
				16, 1, buf + len,
				(ssize_t)PAGE_SIZE - len, false);
	len += CE_DEBUG_PRINT_BUF_SIZE(sizeof(union ce_desc));
	len += snprintf(buf + len, PAGE_SIZE - len, "\n");

#if HIF_CE_DEBUG_DATA_BUF
	if (ce_hist->data_enable[ce_hist->hist_id])
		len = hif_dump_desc_data_buf(buf, len, event->data,
						(event->actual_data_len <
						 CE_DEBUG_MAX_DATA_BUF_SIZE) ?
						event->actual_data_len :
						CE_DEBUG_MAX_DATA_BUF_SIZE);
#endif /*HIF_CE_DEBUG_DATA_BUF*/

	len += snprintf(buf + len, PAGE_SIZE - len, "END\n");

	return len;
}

/*
 * hif_store_desc_trace_buf_index() -
 * API to get the CE id and CE debug storage buffer index
 *
 * @dev: network device
 * @attr: sysfs attribute
 * @buf: data got from the user
 *
 * Return total length
 */
ssize_t hif_input_desc_trace_buf_index(struct hif_softc *scn,
					const char *buf, size_t size)
{
	struct ce_desc_hist *ce_hist = NULL;

	if (!scn)
		return -EINVAL;

	ce_hist = &scn->hif_ce_desc_hist;

	if (!size) {
		pr_err("%s: Invalid input buffer.\n", __func__);
		return -EINVAL;
	}

	if (sscanf(buf, "%d %d", &ce_hist->hist_id,
			&ce_hist->hist_index) != 2) {
		pr_err("%s: Invalid input value.\n", __func__);
		return -EINVAL;
	}
	if ((ce_hist->hist_id >= CE_COUNT_MAX) ||
	   (ce_hist->hist_index >= HIF_CE_HISTORY_MAX)) {
		qdf_print("Invalid values");
		return -EINVAL;
	}

	return size;
}

#endif  /*For MCL,  HIF_CONFIG_SLUB_DEBUG_ON || HIF_CE_DEBUG_DATA_BUF */

#if HIF_CE_DEBUG_DATA_BUF
/*
 * hif_ce_en_desc_hist() -
 * API to enable recording the CE desc history
 *
 * @dev: network device
 * @attr: sysfs attribute
 * @buf: buffer to copy the data.
 *
 * Starts recording the ce desc history
 *
 * Return total length copied
 */
ssize_t hif_ce_en_desc_hist(struct hif_softc *scn, const char *buf, size_t size)
{
	struct ce_desc_hist *ce_hist = NULL;
	uint32_t cfg = 0;
	uint32_t ce_id = 0;

	if (!scn)
		return -EINVAL;

	ce_hist = &scn->hif_ce_desc_hist;

	if (!size) {
		pr_err("%s: Invalid input buffer.\n", __func__);
		return -EINVAL;
	}

	if (sscanf(buf, "%d %d", &ce_id, &cfg) != 2) {
		pr_err("%s: Invalid input: Enter CE Id<sp><1/0>.\n", __func__);
		return -EINVAL;
	}
	if (ce_id >= CE_COUNT_MAX) {
		qdf_print("Invalid value CE Id");
		return -EINVAL;
	}

	if ((cfg > 1 || cfg < 0)) {
		qdf_print("Invalid values: enter 0 or 1");
		return -EINVAL;
	}

	if (!ce_hist->hist_ev[ce_id])
		return -EINVAL;

	qdf_mutex_acquire(&ce_dbg_datamem_lock[ce_id]);
	if (cfg == 1) {
		if (ce_hist->data_enable[ce_id] == 1) {
			qdf_print("\nAlready Enabled");
		} else {
			if (alloc_mem_ce_debug_hist_data(scn, ce_id)
							== QDF_STATUS_E_NOMEM){
				ce_hist->data_enable[ce_id] = 0;
				qdf_print("%s:Memory Alloc failed");
			} else
				ce_hist->data_enable[ce_id] = 1;
		}
	} else if (cfg == 0) {
		if (ce_hist->data_enable[ce_id] == 0) {
			qdf_print("\nAlready Disabled");
		} else {
			ce_hist->data_enable[ce_id] = 0;
				free_mem_ce_debug_hist_data(scn, ce_id);
		}
	}
	qdf_mutex_release(&ce_dbg_datamem_lock[ce_id]);

	return size;
}

/*
 * hif_disp_ce_enable_desc_data_hist() -
 * API to display value of data_enable
 *
 * @dev: network device
 * @attr: sysfs attribute
 * @buf: buffer to copy the data.
 *
 * Return total length copied
 */
ssize_t hif_disp_ce_enable_desc_data_hist(struct hif_softc *scn, char *buf)
{
	ssize_t len = 0;
	uint32_t ce_id = 0;
	struct ce_desc_hist *ce_hist = NULL;

	if (!scn)
		return -EINVAL;

	ce_hist = &scn->hif_ce_desc_hist;

	for (ce_id = 0; ce_id < CE_COUNT_MAX; ce_id++) {
		len += snprintf(buf + len, PAGE_SIZE - len, " CE%d: %d\n",
				ce_id, ce_hist->data_enable[ce_id]);
	}

	return len;
}
#endif /* HIF_CE_DEBUG_DATA_BUF */

#ifdef OL_ATH_SMART_LOGGING
#define GUARD_SPACE 10
#define LOG_ID_SZ 4
/*
 * hif_log_src_ce_dump() - Copy all the CE SRC ring to buf
 * @src_ring: SRC ring state
 * @buf_cur: Current pointer in ring buffer
 * @buf_init:Start of the ring buffer
 * @buf_sz: Size of the ring buffer
 * @skb_sz: Max size of the SKB buffer to be copied
 *
 * Dumps all the CE SRC ring descriptors and buffers pointed by them in to
 * the given buf, skb_sz is the max buffer size to be copied
 *
 * Return: Current pointer in ring buffer
 */
static uint8_t *hif_log_src_ce_dump(struct CE_ring_state *src_ring,
				    uint8_t *buf_cur, uint8_t *buf_init,
				    uint32_t buf_sz, uint32_t skb_sz)
{
	struct CE_src_desc *src_ring_base;
	uint32_t len, entry;
	struct CE_src_desc  *src_desc;
	qdf_nbuf_t nbuf;
	uint32_t available_buf;

	src_ring_base = (struct CE_src_desc *)src_ring->base_addr_owner_space;
	len = sizeof(struct CE_ring_state);
	available_buf = buf_sz - (buf_cur - buf_init);
	if (available_buf < (len + GUARD_SPACE)) {
		buf_cur = buf_init;
	}

	qdf_mem_copy(buf_cur, src_ring, sizeof(struct CE_ring_state));
	buf_cur += sizeof(struct CE_ring_state);

	for (entry = 0; entry < src_ring->nentries; entry++) {
		src_desc = CE_SRC_RING_TO_DESC(src_ring_base, entry);
		nbuf = src_ring->per_transfer_context[entry];
		if (nbuf) {
			uint32_t skb_len  = qdf_nbuf_len(nbuf);
			uint32_t skb_cp_len = qdf_min(skb_len, skb_sz);

			len = sizeof(struct CE_src_desc) + skb_cp_len
				+ LOG_ID_SZ + sizeof(skb_cp_len);
			available_buf = buf_sz - (buf_cur - buf_init);
			if (available_buf < (len + GUARD_SPACE)) {
				buf_cur = buf_init;
			}
			qdf_mem_copy(buf_cur, src_desc,
				     sizeof(struct CE_src_desc));
			buf_cur += sizeof(struct CE_src_desc);

			available_buf = buf_sz - (buf_cur - buf_init);
			buf_cur += snprintf(buf_cur, available_buf, "SKB%d",
						skb_cp_len);

			if (skb_cp_len) {
				qdf_mem_copy(buf_cur, qdf_nbuf_data(nbuf),
					     skb_cp_len);
				buf_cur += skb_cp_len;
			}
		} else {
			len = sizeof(struct CE_src_desc) + LOG_ID_SZ;
			available_buf = buf_sz - (buf_cur - buf_init);
			if (available_buf < (len + GUARD_SPACE)) {
				buf_cur = buf_init;
			}
			qdf_mem_copy(buf_cur, src_desc,
				     sizeof(struct CE_src_desc));
			buf_cur += sizeof(struct CE_src_desc);
			available_buf = buf_sz - (buf_cur - buf_init);
			buf_cur += snprintf(buf_cur, available_buf, "NUL");
		}
	}

	return buf_cur;
}

/*
 * hif_log_dest_ce_dump() - Copy all the CE DEST ring to buf
 * @dest_ring: SRC ring state
 * @buf_cur: Current pointer in ring buffer
 * @buf_init:Start of the ring buffer
 * @buf_sz: Size of the ring buffer
 * @skb_sz: Max size of the SKB buffer to be copied
 *
 * Dumps all the CE SRC ring descriptors and buffers pointed by them in to
 * the given buf, skb_sz is the max buffer size to be copied
 *
 * Return: Current pointer in ring buffer
 */
static uint8_t *hif_log_dest_ce_dump(struct CE_ring_state *dest_ring,
				     uint8_t *buf_cur, uint8_t *buf_init,
				     uint32_t buf_sz, uint32_t skb_sz)
{
	struct CE_dest_desc *dest_ring_base;
	uint32_t len, entry;
	struct CE_dest_desc  *dest_desc;
	qdf_nbuf_t nbuf;
	uint32_t available_buf;

	dest_ring_base =
		(struct CE_dest_desc *)dest_ring->base_addr_owner_space;

	len = sizeof(struct CE_ring_state);
	available_buf = buf_sz - (buf_cur - buf_init);
	if (available_buf < (len + GUARD_SPACE)) {
		buf_cur = buf_init;
	}

	qdf_mem_copy(buf_cur, dest_ring, sizeof(struct CE_ring_state));
	buf_cur += sizeof(struct CE_ring_state);

	for (entry = 0; entry < dest_ring->nentries; entry++) {
		dest_desc = CE_DEST_RING_TO_DESC(dest_ring_base, entry);

		nbuf = dest_ring->per_transfer_context[entry];
		if (nbuf) {
			uint32_t skb_len  = qdf_nbuf_len(nbuf);
			uint32_t skb_cp_len = qdf_min(skb_len, skb_sz);

			len = sizeof(struct CE_dest_desc) + skb_cp_len
				+ LOG_ID_SZ + sizeof(skb_cp_len);

			available_buf = buf_sz - (buf_cur - buf_init);
			if (available_buf < (len + GUARD_SPACE)) {
				buf_cur = buf_init;
			}

			qdf_mem_copy(buf_cur, dest_desc,
				     sizeof(struct CE_dest_desc));
			buf_cur += sizeof(struct CE_dest_desc);
			available_buf = buf_sz - (buf_cur - buf_init);
			buf_cur += snprintf(buf_cur, available_buf, "SKB%d",
						skb_cp_len);
			if (skb_cp_len) {
				qdf_mem_copy(buf_cur, qdf_nbuf_data(nbuf),
					     skb_cp_len);
				buf_cur += skb_cp_len;
			}
		} else {
			len = sizeof(struct CE_dest_desc) + LOG_ID_SZ;
			available_buf = buf_sz - (buf_cur - buf_init);
			if (available_buf < (len + GUARD_SPACE)) {
				buf_cur = buf_init;
			}
			qdf_mem_copy(buf_cur, dest_desc,
				     sizeof(struct CE_dest_desc));
			buf_cur += sizeof(struct CE_dest_desc);
			available_buf = buf_sz - (buf_cur - buf_init);
			buf_cur += snprintf(buf_cur, available_buf, "NUL");
		}
	}
	return buf_cur;
}

/**
 * hif_log_ce_dump() - Copy all the CE DEST ring to buf
 * Calls the respective function to dump all the CE SRC/DEST ring descriptors
 * and buffers pointed by them in to the given buf
 */
uint8_t *hif_log_dump_ce(struct hif_softc *scn, uint8_t *buf_cur,
			 uint8_t *buf_init, uint32_t buf_sz,
			 uint32_t ce, uint32_t skb_sz)
{
	struct CE_state *ce_state;
	struct CE_ring_state *src_ring;
	struct CE_ring_state *dest_ring;

	ce_state = scn->ce_id_to_state[ce];
	src_ring = ce_state->src_ring;
	dest_ring = ce_state->dest_ring;

	if (src_ring) {
		buf_cur = hif_log_src_ce_dump(src_ring, buf_cur,
					      buf_init, buf_sz, skb_sz);
	} else if (dest_ring) {
		buf_cur = hif_log_dest_ce_dump(dest_ring, buf_cur,
					       buf_init, buf_sz, skb_sz);
	}

	return buf_cur;
}
#endif /* OL_ATH_SMART_LOGGING */

