/*
 * Copyright (c) 2013-2020 The Linux Foundation. All rights reserved.
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

static const char *ce_event_type_to_str(enum hif_ce_event_type type);

int get_next_record_index(qdf_atomic_t *table_index, int array_size)
{
	int record_index = qdf_atomic_inc_return(table_index);

	if (record_index == array_size)
		qdf_atomic_sub(array_size, table_index);

	while (record_index >= array_size)
		record_index -= array_size;

	return record_index;
}

qdf_export_symbol(get_next_record_index);

#ifdef HIF_CE_DEBUG_DATA_BUF
void hif_ce_desc_data_record(struct hif_ce_desc_event *event, int len)
{
	uint8_t *data = NULL;

	if (!event->data) {
		hif_err_rl("No ce debug memory allocated");
		return;
	}

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

qdf_export_symbol(hif_ce_desc_data_record);

void hif_clear_ce_desc_debug_data(struct hif_ce_desc_event *event)
{
	qdf_mem_zero(event,
		     offsetof(struct hif_ce_desc_event, data));
}

qdf_export_symbol(hif_clear_ce_desc_debug_data);
#else
void hif_clear_ce_desc_debug_data(struct hif_ce_desc_event *event)
{
	qdf_mem_zero(event, sizeof(struct hif_ce_desc_event));
}

qdf_export_symbol(hif_clear_ce_desc_debug_data);
#endif /* HIF_CE_DEBUG_DATA_BUF */

#if defined(HIF_RECORD_PADDR)
void hif_ce_desc_record_rx_paddr(struct hif_softc *scn,
				 struct hif_ce_desc_event *event,
				 qdf_nbuf_t memory)
{
	if (memory) {
		event->dma_addr = QDF_NBUF_CB_PADDR(memory);
		event->dma_to_phy = qdf_mem_paddr_from_dmaaddr(
					scn->qdf_dev,
					event->dma_addr);

		event->virt_to_phy =
			virt_to_phys(qdf_nbuf_data(memory));
	}
}
#endif /* HIF_RECORD_RX_PADDR */

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

	hif_clear_ce_desc_debug_data(event);

	event->type = type;
	event->time = qdf_get_log_timestamp();
	event->cpu_id = qdf_get_cpu();

	if (descriptor)
		qdf_mem_copy(&event->descriptor, descriptor,
			     sizeof(union ce_desc));

	event->memory = memory;
	event->index = index;

	if (event->type == HIF_RX_DESC_POST ||
	    event->type == HIF_RX_DESC_COMPLETION)
		hif_ce_desc_record_rx_paddr(scn, event, memory);

	if (ce_hist->data_enable[ce_id])
		hif_ce_desc_data_record(event, len);
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
	qdf_mutex_create(&ce_hist->ce_dbg_datamem_lock[ce_id]);
}

/**
 * ce_deinit_ce_desc_event_log() - deinitialize the ce event log
 * @ce_id: copy engine id for which we are deinitializing the log
 *
 */
inline void ce_deinit_ce_desc_event_log(struct hif_softc *scn, int ce_id)
{
	struct ce_desc_hist *ce_hist = &scn->hif_ce_desc_hist;

	qdf_mutex_destroy(&ce_hist->ce_dbg_datamem_lock[ce_id]);
}

#else /* (HIF_CONFIG_SLUB_DEBUG_ON) || defined(HIF_CE_DEBUG_DATA_BUF) */
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
#endif /*defined(HIF_CONFIG_SLUB_DEBUG_ON) || defined(HIF_CE_DEBUG_DATA_BUF) */

#ifdef NAPI_YIELD_BUDGET_BASED
bool hif_ce_service_should_yield(struct hif_softc *scn,
				 struct CE_state *ce_state)
{
	bool yield =  hif_max_num_receives_reached(scn, ce_state->receive_count);

	/* Setting receive_count to MAX_NUM_OF_RECEIVES when this count goes
	 * beyond MAX_NUM_OF_RECEIVES for NAPI backet calulation issue. This
	 * can happen in fast path handling as processing is happenning in
	 * batches.
	 */
	if (yield)
		ce_state->receive_count = MAX_NUM_OF_RECEIVES;

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

	/* Setting receive_count to MAX_NUM_OF_RECEIVES when this count goes
	 * beyond MAX_NUM_OF_RECEIVES for NAPI backet calulation issue. This
	 * can happen in fast path handling as processing is happenning in
	 * batches.
	 */
	if (rxpkt_thresh_reached)
		ce_state->receive_count = MAX_NUM_OF_RECEIVES;

	yield =  time_limit_reached || rxpkt_thresh_reached;

	if (yield &&
	    ce_state->htt_rx_data &&
	    hif_napi_enabled(GET_HIF_OPAQUE_HDL(scn), ce_state->id)) {
		hif_napi_update_yield_stats(ce_state,
					    time_limit_reached,
					    rxpkt_thresh_reached);
	}

	return yield;
}
qdf_export_symbol(hif_ce_service_should_yield);
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

QDF_STATUS
ce_send(struct CE_handle *copyeng,
		void *per_transfer_context,
		qdf_dma_addr_t buffer,
		uint32_t nbytes,
		uint32_t transfer_id,
		uint32_t flags,
		uint32_t user_flag)
{
	struct CE_state *CE_state = (struct CE_state *)copyeng;
	QDF_STATUS status;
	struct HIF_CE_state *hif_state = HIF_GET_CE_STATE(CE_state->scn);

	qdf_spin_lock_bh(&CE_state->ce_index_lock);
	status = hif_state->ce_services->ce_send_nolock(copyeng,
			per_transfer_context, buffer, nbytes,
			transfer_id, flags, user_flag);
	qdf_spin_unlock_bh(&CE_state->ce_index_lock);

	return status;
}
qdf_export_symbol(ce_send);

unsigned int ce_sendlist_sizeof(void)
{
	return sizeof(struct ce_sendlist);
}

void ce_sendlist_init(struct ce_sendlist *sendlist)
{
	struct ce_sendlist_s *sl = (struct ce_sendlist_s *)sendlist;

	sl->num_items = 0;
}

QDF_STATUS
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

QDF_STATUS
ce_sendlist_send(struct CE_handle *copyeng,
		 void *per_transfer_context,
		 struct ce_sendlist *sendlist, unsigned int transfer_id)
{
	struct CE_state *CE_state = (struct CE_state *)copyeng;
	struct HIF_CE_state *hif_state = HIF_GET_CE_STATE(CE_state->scn);

	return hif_state->ce_services->ce_sendlist_send(copyeng,
			per_transfer_context, sendlist, transfer_id);
}

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
			hif_err("Out of descriptors");
			src_ring->write_index = write_index;
			war_ce_src_ring_write_idx_set(scn, ctrl_addr,
					write_index);

			sw_index = src_ring->sw_index;
			write_index = src_ring->write_index;

			deltacount = CE_RING_DELTA(nentries_mask, write_index,
					sw_index-1);
			if (!freelist) {
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
 * Return: QDF_STATUS: CE sent status
 */
QDF_STATUS ce_send_single(struct CE_handle *ce_tx_hdl, qdf_nbuf_t msdu,
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
		hif_err("ce send fail %d %d %d", nentries_mask,
		       write_index, sw_index);
		return QDF_STATUS_E_RESOURCES;
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
 * Return: QDF_STATUS_SUCCESS if the buffer is enqueued
 */
QDF_STATUS
ce_recv_buf_enqueue(struct CE_handle *copyeng,
		    void *per_recv_context, qdf_dma_addr_t buffer)
{
	struct CE_state *CE_state = (struct CE_state *)copyeng;
	struct HIF_CE_state *hif_state = HIF_GET_CE_STATE(CE_state->scn);

	return hif_state->ce_services->ce_recv_buf_enqueue(copyeng,
			per_recv_context, buffer);
}
qdf_export_symbol(ce_recv_buf_enqueue);

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
 * Guts of ce_completed_recv_next.
 * The caller takes responsibility for any necessary locking.
 */
QDF_STATUS
ce_completed_recv_next(struct CE_handle *copyeng,
		       void **per_CE_contextp,
		       void **per_transfer_contextp,
		       qdf_dma_addr_t *bufferp,
		       unsigned int *nbytesp,
		       unsigned int *transfer_idp, unsigned int *flagsp)
{
	struct CE_state *CE_state = (struct CE_state *)copyeng;
	QDF_STATUS status;
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
qdf_export_symbol(ce_cancel_send_next);

QDF_STATUS
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
	QDF_STATUS status;

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
 * ce_engine_service_reg:
 *
 * Called from ce_per_engine_service and goes through the regular interrupt
 * handling that does not involve the WLAN fast path feature.
 *
 * Returns void
 */
void ce_engine_service_reg(struct hif_softc *scn, int CE_id)
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

			qdf_spin_lock(&CE_state->ce_index_lock);
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
				return;
			}
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
			qdf_atomic_set(&CE_state->rx_pending, 0);
			hif_err_rl("%s: target access is not allowed",
				   __func__);
			return;
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
				hif_err(
					"Potential infinite loop detected during Rx processing nentries_mask:0x%x sw read_idx:0x%x hw read_idx:0x%x",
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
				hif_err(
					"Potential infinite loop detected during send completion nentries_mask:0x%x sw read_idx:0x%x hw read_idx:0x%x",
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
}

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

	if (hif_is_nss_wifi_enabled(scn) && (CE_state->htt_rx_data))
		return CE_state->receive_count;

	if (Q_TARGET_ACCESS_BEGIN(scn) < 0) {
		hif_err("[premature rc=0]");
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

	CE_state->service(scn, CE_id);

	qdf_spin_unlock(&CE_state->ce_index_lock);

	if (Q_TARGET_ACCESS_END(scn) < 0)
		hif_err("<--[premature rc=%d]", CE_state->receive_count);
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

	if (!CE_state) {
		hif_err("Error CE state = NULL");
		return;
	}
	scn = CE_state->scn;
	hif_state = HIF_GET_CE_STATE(scn);
	if (!hif_state) {
		hif_err("Error HIF state = NULL");
		return;
	}
	CE_state->send_context = ce_send_context;
	CE_state->send_cb = fn_ptr;
	hif_state->ce_services->ce_per_engine_handler_adjust(CE_state,
							disable_interrupts);
}
qdf_export_symbol(ce_send_cb_register);

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

	if (!CE_state) {
		hif_err("ERROR CE state = NULL");
		return;
	}
	scn = CE_state->scn;
	hif_state = HIF_GET_CE_STATE(scn);
	if (!hif_state) {
		hif_err("Error HIF state = NULL");
		return;
	}
	CE_state->recv_context = CE_recv_context;
	CE_state->recv_cb = fn_ptr;
	hif_state->ce_services->ce_per_engine_handler_adjust(CE_state,
							disable_interrupts);
}
qdf_export_symbol(ce_recv_cb_register);

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
#ifdef QCN7605_SUPPORT
static qdf_dma_addr_t ce_ipa_get_wr_index_addr(struct CE_state *CE_state)
{
	u_int32_t ctrl_addr = CE_state->ctrl_addr;
	struct hif_softc *scn = CE_state->scn;
	qdf_dma_addr_t wr_index_addr;

	wr_index_addr = shadow_sr_wr_ind_addr(scn, ctrl_addr);
	return wr_index_addr;
}
#else
static qdf_dma_addr_t ce_ipa_get_wr_index_addr(struct CE_state *CE_state)
{
	struct hif_softc *scn = CE_state->scn;
	qdf_dma_addr_t wr_index_addr;

	wr_index_addr = CE_BASE_ADDRESS(CE_state->id) +
			SR_WR_INDEX_ADDRESS;
	return wr_index_addr;
}
#endif

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
	*ce_reg_paddr = phy_mem_base + ce_ipa_get_wr_index_addr(CE_state);

}

#endif /* IPA_OFFLOAD */

#ifdef HIF_CE_DEBUG_DATA_BUF
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

	if (ce_hist->hist_id >= CE_COUNT_MAX ||
	    ce_hist->hist_index >= HIF_CE_HISTORY_MAX) {
		qdf_print("Invalid values");
		return -EINVAL;
	}

	hist_ev =
		(struct hif_ce_desc_event *)ce_hist->hist_ev[ce_hist->hist_id];

	if (!hist_ev) {
		qdf_print("Low Memory");
		return -EINVAL;
	}

	event = &hist_ev[ce_hist->hist_index];

	qdf_log_timestamp_to_secs(event->time, &secs, &usecs);

	len += snprintf(buf, PAGE_SIZE - len,
			"\nTime:%lld.%06lld, CE:%d, EventType: %s, EventIndex: %d\nDataAddr=%pK",
			secs, usecs, ce_hist->hist_id,
			ce_event_type_to_str(event->type),
			event->index, event->memory);
#ifdef HIF_CE_DEBUG_DATA_BUF
	len += snprintf(buf + len, PAGE_SIZE - len, ", Data len=%zu",
			event->actual_data_len);
#endif

	len += snprintf(buf + len, PAGE_SIZE - len, "\nCE descriptor: ");

	hex_dump_to_buffer(&event->descriptor, sizeof(union ce_desc),
				16, 1, buf + len,
				(ssize_t)PAGE_SIZE - len, false);
	len += CE_DEBUG_PRINT_BUF_SIZE(sizeof(union ce_desc));
	len += snprintf(buf + len, PAGE_SIZE - len, "\n");

#ifdef HIF_CE_DEBUG_DATA_BUF
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
		qdf_nofl_err("%s: Invalid input buffer.", __func__);
		return -EINVAL;
	}

	if (sscanf(buf, "%u %u", (unsigned int *)&ce_hist->hist_id,
		   (unsigned int *)&ce_hist->hist_index) != 2) {
		qdf_nofl_err("%s: Invalid input value.", __func__);
		return -EINVAL;
	}
	if ((ce_hist->hist_id >= CE_COUNT_MAX) ||
	   (ce_hist->hist_index >= HIF_CE_HISTORY_MAX)) {
		qdf_print("Invalid values");
		return -EINVAL;
	}

	return size;
}

#endif /*defined(HIF_CONFIG_SLUB_DEBUG_ON) || defined(HIF_CE_DEBUG_DATA_BUF) */

#ifdef HIF_CE_DEBUG_DATA_BUF
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
		qdf_nofl_err("%s: Invalid input buffer.", __func__);
		return -EINVAL;
	}

	if (sscanf(buf, "%u %u", (unsigned int *)&ce_id,
		   (unsigned int *)&cfg) != 2) {
		qdf_nofl_err("%s: Invalid input: Enter CE Id<sp><1/0>.",
			     __func__);
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

	qdf_mutex_acquire(&ce_hist->ce_dbg_datamem_lock[ce_id]);
	if (cfg == 1) {
		if (ce_hist->data_enable[ce_id] == 1) {
			qdf_debug("Already Enabled");
		} else {
			if (alloc_mem_ce_debug_hist_data(scn, ce_id)
							== QDF_STATUS_E_NOMEM){
				ce_hist->data_enable[ce_id] = 0;
				qdf_err("%s:Memory Alloc failed", __func__);
			} else
				ce_hist->data_enable[ce_id] = 1;
		}
	} else if (cfg == 0) {
		if (ce_hist->data_enable[ce_id] == 0) {
			qdf_debug("Already Disabled");
		} else {
			ce_hist->data_enable[ce_id] = 0;
				free_mem_ce_debug_hist_data(scn, ce_id);
		}
	}
	qdf_mutex_release(&ce_hist->ce_dbg_datamem_lock[ce_id]);

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

qdf_export_symbol(hif_log_dump_ce);
#endif /* OL_ATH_SMART_LOGGING */

