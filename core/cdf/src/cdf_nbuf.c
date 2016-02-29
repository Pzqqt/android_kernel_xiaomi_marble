/*
 * Copyright (c) 2014-2016 The Linux Foundation. All rights reserved.
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

/**
 * DOC: cdf_nbuf.c
 *
 * Connectivity driver framework(CDF) network buffer management APIs
 */

#include <linux/kernel.h>
#include <linux/version.h>
#include <linux/skbuff.h>
#include <linux/module.h>
#if defined(FEATURE_TSO)
#include <linux/tcp.h>
#include <linux/if_vlan.h>
#include <linux/ip.h>
#endif /* FEATURE_TSO */
#include <qdf_types.h>
#include <cdf_nbuf.h>
#include <cdf_memory.h>
#include <qdf_trace.h>
#include <qdf_status.h>
#include <qdf_lock.h>

/* Packet Counter */
static uint32_t nbuf_tx_mgmt[NBUF_TX_PKT_STATE_MAX];
static uint32_t nbuf_tx_data[NBUF_TX_PKT_STATE_MAX];

/**
 * cdf_nbuf_tx_desc_count_display() - Displays the packet counter
 *
 * Return: none
 */
void cdf_nbuf_tx_desc_count_display(void)
{
	qdf_print("Current Snapshot of the Driver:\n");
	qdf_print("Data Packets:\n");
	qdf_print("HDD %d TXRX_Q %d TXRX %d HTT %d",
		nbuf_tx_data[NBUF_TX_PKT_HDD] -
		(nbuf_tx_data[NBUF_TX_PKT_TXRX] +
		nbuf_tx_data[NBUF_TX_PKT_TXRX_ENQUEUE] -
		nbuf_tx_data[NBUF_TX_PKT_TXRX_DEQUEUE]),
		nbuf_tx_data[NBUF_TX_PKT_TXRX_ENQUEUE] -
		nbuf_tx_data[NBUF_TX_PKT_TXRX_DEQUEUE],
		nbuf_tx_data[NBUF_TX_PKT_TXRX] - nbuf_tx_data[NBUF_TX_PKT_HTT],
		nbuf_tx_data[NBUF_TX_PKT_HTT]  - nbuf_tx_data[NBUF_TX_PKT_HTC]);
	qdf_print(" HTC %d  HIF %d CE %d TX_COMP %d\n",
		nbuf_tx_data[NBUF_TX_PKT_HTC]  - nbuf_tx_data[NBUF_TX_PKT_HIF],
		nbuf_tx_data[NBUF_TX_PKT_HIF]  - nbuf_tx_data[NBUF_TX_PKT_CE],
		nbuf_tx_data[NBUF_TX_PKT_CE]   - nbuf_tx_data[NBUF_TX_PKT_FREE],
		nbuf_tx_data[NBUF_TX_PKT_FREE]);
	qdf_print("Mgmt Packets:\n");
	qdf_print("TXRX_Q %d TXRX %d HTT %d HTC %d HIF %d CE %d TX_COMP %d\n",
		nbuf_tx_mgmt[NBUF_TX_PKT_TXRX_ENQUEUE] -
		nbuf_tx_mgmt[NBUF_TX_PKT_TXRX_DEQUEUE],
		nbuf_tx_mgmt[NBUF_TX_PKT_TXRX] - nbuf_tx_mgmt[NBUF_TX_PKT_HTT],
		nbuf_tx_mgmt[NBUF_TX_PKT_HTT]  - nbuf_tx_mgmt[NBUF_TX_PKT_HTC],
		nbuf_tx_mgmt[NBUF_TX_PKT_HTC]  - nbuf_tx_mgmt[NBUF_TX_PKT_HIF],
		nbuf_tx_mgmt[NBUF_TX_PKT_HIF]  - nbuf_tx_mgmt[NBUF_TX_PKT_CE],
		nbuf_tx_mgmt[NBUF_TX_PKT_CE]   - nbuf_tx_mgmt[NBUF_TX_PKT_FREE],
		nbuf_tx_mgmt[NBUF_TX_PKT_FREE]);
}

/**
 * cdf_nbuf_tx_desc_count_update() - Updates the layer packet counter
 * @packet_type   : packet type either mgmt/data
 * @current_state : layer at which the packet currently present
 *
 * Return: none
 */
static inline void cdf_nbuf_tx_desc_count_update(uint8_t packet_type,
							uint8_t current_state)
{
	switch (packet_type) {
	case NBUF_TX_PKT_MGMT_TRACK:
		nbuf_tx_mgmt[current_state]++;
		break;
	case NBUF_TX_PKT_DATA_TRACK:
		nbuf_tx_data[current_state]++;
		break;
	default:
		break;
	}
}

/**
 * cdf_nbuf_tx_desc_count_clear() - Clears packet counter for both data, mgmt
 *
 * Return: none
 */
void cdf_nbuf_tx_desc_count_clear(void)
{
	memset(nbuf_tx_mgmt, 0, sizeof(nbuf_tx_mgmt));
	memset(nbuf_tx_data, 0, sizeof(nbuf_tx_data));
}

/**
 * cdf_nbuf_set_state() - Updates the packet state
 * @nbuf:            network buffer
 * @current_state :  layer at which the packet currently is
 *
 * This function updates the packet state to the layer at which the packet
 * currently is
 *
 * Return: none
 */
void cdf_nbuf_set_state(cdf_nbuf_t nbuf, uint8_t current_state)
{
	/*
	 * Only Mgmt, Data Packets are tracked. WMI messages
	 * such as scan commands are not tracked
	 */
	uint8_t packet_type;
	packet_type = NBUF_CB_TX_PACKET_TRACK(nbuf);

	if ((packet_type != NBUF_TX_PKT_DATA_TRACK) &&
		(packet_type != NBUF_TX_PKT_MGMT_TRACK)) {
		return;
	}
	NBUF_CB_TX_PACKET_STATE(nbuf) = current_state;
	cdf_nbuf_tx_desc_count_update(packet_type,
					current_state);
}

/* globals do not need to be initialized to NULL/0 */
cdf_nbuf_trace_update_t trace_update_cb;

/**
 * __cdf_nbuf_alloc() - Allocate nbuf
 * @hdl: Device handle
 * @size: Netbuf requested size
 * @reserve: Reserve
 * @align: Align
 * @prio: Priority
 *
 * This allocates an nbuf aligns if needed and reserves some space in the front,
 * since the reserve is done after alignment the reserve value if being
 * unaligned will result in an unaligned address.
 *
 * Return: nbuf or %NULL if no memory
 */
struct sk_buff *__cdf_nbuf_alloc(qdf_device_t osdev, size_t size, int reserve,
				 int align, int prio)
{
	struct sk_buff *skb;
	unsigned long offset;

	if (align)
		size += (align - 1);

	skb = dev_alloc_skb(size);

	if (!skb) {
		pr_err("ERROR:NBUF alloc failed\n");
		return NULL;
	}
	memset(skb->cb, 0x0, sizeof(skb->cb));

	/*
	 * The default is for netbuf fragments to be interpreted
	 * as wordstreams rather than bytestreams.
	 */
	NBUF_CB_TX_EXTRA_FRAG_WORDSTR_EFRAG(skb) = 1;
	NBUF_CB_TX_EXTRA_FRAG_WORDSTR_NBUF(skb) = 1;

	/*
	 * XXX:how about we reserve first then align
	 * Align & make sure that the tail & data are adjusted properly
	 */

	if (align) {
		offset = ((unsigned long)skb->data) % align;
		if (offset)
			skb_reserve(skb, align - offset);
	}

	/*
	 * NOTE:alloc doesn't take responsibility if reserve unaligns the data
	 * pointer
	 */
	skb_reserve(skb, reserve);

	return skb;
}

/**
 * __cdf_nbuf_free() - free the nbuf its interrupt safe
 * @skb: Pointer to network buffer
 *
 * Return: none
 */
void __cdf_nbuf_free(struct sk_buff *skb)
{
	if (cdf_nbuf_ipa_owned_get(skb))
		/* IPA cleanup function will need to be called here */
		QDF_BUG(1);
	else
		dev_kfree_skb_any(skb);
}

/**
 * __cdf_nbuf_map() - get the dma map of the nbuf
 * @osdev: OS device
 * @bmap: Bitmap
 * @skb: Pointer to network buffer
 * @dir: Direction
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
__cdf_nbuf_map(qdf_device_t osdev, struct sk_buff *skb, qdf_dma_dir_t dir)
{
#ifdef CDF_OS_DEBUG
	struct skb_shared_info *sh = skb_shinfo(skb);
#endif
	qdf_assert((dir == QDF_DMA_TO_DEVICE)
		   || (dir == QDF_DMA_FROM_DEVICE));

	/*
	 * Assume there's only a single fragment.
	 * To support multiple fragments, it would be necessary to change
	 * cdf_nbuf_t to be a separate object that stores meta-info
	 * (including the bus address for each fragment) and a pointer
	 * to the underlying sk_buff.
	 */
	qdf_assert(sh->nr_frags == 0);

	return __cdf_nbuf_map_single(osdev, skb, dir);

	return QDF_STATUS_SUCCESS;
}

/**
 * __cdf_nbuf_unmap() - to unmap a previously mapped buf
 * @osdev: OS device
 * @skb: Pointer to network buffer
 * @dir: Direction
 *
 * Return: none
 */
void
__cdf_nbuf_unmap(qdf_device_t osdev, struct sk_buff *skb, qdf_dma_dir_t dir)
{
	qdf_assert((dir == QDF_DMA_TO_DEVICE)
		   || (dir == QDF_DMA_FROM_DEVICE));

	qdf_assert(((dir == QDF_DMA_TO_DEVICE)
		    || (dir == QDF_DMA_FROM_DEVICE)));
	/*
	 * Assume there's a single fragment.
	 * If this is not true, the assertion in __cdf_nbuf_map will catch it.
	 */
	__cdf_nbuf_unmap_single(osdev, skb, dir);
}

/**
 * __cdf_nbuf_map_single() - dma map of the nbuf
 * @osdev: OS device
 * @skb: Pointer to network buffer
 * @dir: Direction
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
__cdf_nbuf_map_single(qdf_device_t osdev, cdf_nbuf_t buf, qdf_dma_dir_t dir)
{
	qdf_dma_addr_t paddr;

/* tempory hack for simulation */
#ifdef A_SIMOS_DEVHOST
	NBUF_CB_PADDR(buf) = paddr = buf->data;
	return QDF_STATUS_SUCCESS;
#else
	/* assume that the OS only provides a single fragment */
	NBUF_CB_PADDR(buf) = paddr =
		dma_map_single(osdev->dev, buf->data,
			       skb_end_pointer(buf) - buf->data, dir);
	return dma_mapping_error(osdev->dev, paddr)
		? QDF_STATUS_E_FAILURE
		: QDF_STATUS_SUCCESS;
#endif /* #ifdef A_SIMOS_DEVHOST */
}

/**
 * __cdf_nbuf_unmap_single() - dma unmap nbuf
 * @osdev: OS device
 * @skb: Pointer to network buffer
 * @dir: Direction
 *
 * Return: none
 */
void
__cdf_nbuf_unmap_single(qdf_device_t osdev, cdf_nbuf_t buf, qdf_dma_dir_t dir)
{
#if !defined(A_SIMOS_DEVHOST)
	dma_unmap_single(osdev->dev, NBUF_CB_PADDR(buf),
			 skb_end_pointer(buf) - buf->data, dir);
#endif /* #if !defined(A_SIMOS_DEVHOST) */
}

/**
 * __cdf_nbuf_set_rx_cksum() - set rx checksum
 * @skb: Pointer to network buffer
 * @cksum: Pointer to checksum value
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
__cdf_nbuf_set_rx_cksum(struct sk_buff *skb, qdf_nbuf_rx_cksum_t *cksum)
{
	switch (cksum->l4_result) {
	case QDF_NBUF_RX_CKSUM_NONE:
		skb->ip_summed = CHECKSUM_NONE;
		break;
	case QDF_NBUF_RX_CKSUM_TCP_UDP_UNNECESSARY:
		skb->ip_summed = CHECKSUM_UNNECESSARY;
		break;
	case QDF_NBUF_RX_CKSUM_TCP_UDP_HW:
		skb->ip_summed = CHECKSUM_PARTIAL;
		skb->csum = cksum->val;
		break;
	default:
		pr_err("ADF_NET:Unknown checksum type\n");
		qdf_assert(0);
		return QDF_STATUS_E_NOSUPPORT;
	}
	return QDF_STATUS_SUCCESS;
}

/**
 * __cdf_nbuf_get_tx_cksum() - get tx checksum
 * @skb: Pointer to network buffer
 *
 * Return: TX checksum value
 */
qdf_nbuf_tx_cksum_t __cdf_nbuf_get_tx_cksum(struct sk_buff *skb)
{
	switch (skb->ip_summed) {
	case CHECKSUM_NONE:
		return QDF_NBUF_TX_CKSUM_NONE;
	case CHECKSUM_PARTIAL:
		/* XXX ADF and Linux checksum don't map with 1-to-1. This is
		 * not 100% correct */
		return QDF_NBUF_TX_CKSUM_TCP_UDP;
	case CHECKSUM_COMPLETE:
		return QDF_NBUF_TX_CKSUM_TCP_UDP_IP;
	default:
		return QDF_NBUF_TX_CKSUM_NONE;
	}
}

/**
 * __cdf_nbuf_get_tid() - get tid
 * @skb: Pointer to network buffer
 *
 * Return: tid
 */
uint8_t __cdf_nbuf_get_tid(struct sk_buff *skb)
{
	return skb->priority;
}

/**
 * __cdf_nbuf_set_tid() - set tid
 * @skb: Pointer to network buffer
 *
 * Return: none
 */
void __cdf_nbuf_set_tid(struct sk_buff *skb, uint8_t tid)
{
	skb->priority = tid;
}

/**
 * __cdf_nbuf_set_tid() - set tid
 * @skb: Pointer to network buffer
 *
 * Return: none
 */
uint8_t __cdf_nbuf_get_exemption_type(struct sk_buff *skb)
{
	return QDF_NBUF_EXEMPT_NO_EXEMPTION;
}

/**
 * __cdf_nbuf_reg_trace_cb() - register trace callback
 * @cb_func_ptr: Pointer to trace callback function
 *
 * Return: none
 */
void __cdf_nbuf_reg_trace_cb(cdf_nbuf_trace_update_t cb_func_ptr)
{
	trace_update_cb = cb_func_ptr;
	return;
}

#ifdef QCA_PKT_PROTO_TRACE
/**
 * __cdf_nbuf_trace_update() - update trace event
 * @skb: Pointer to network buffer
 * @event_string: Pointer to trace callback function
 *
 * Return: none
 */
void __cdf_nbuf_trace_update(struct sk_buff *buf, char *event_string)
{
	char string_buf[NBUF_PKT_TRAC_MAX_STRING];

	if ((!trace_update_cb) || (!event_string))
		return;

	if (!cdf_nbuf_trace_get_proto_type(buf))
		return;

	/* Buffer over flow */
	if (NBUF_PKT_TRAC_MAX_STRING <=
	    (cdf_str_len(event_string) + NBUF_PKT_TRAC_PROTO_STRING)) {
		return;
	}

	cdf_mem_zero(string_buf, NBUF_PKT_TRAC_MAX_STRING);
	cdf_mem_copy(string_buf, event_string, cdf_str_len(event_string));
	if (NBUF_PKT_TRAC_TYPE_EAPOL & cdf_nbuf_trace_get_proto_type(buf)) {
		cdf_mem_copy(string_buf + cdf_str_len(event_string),
			     "EPL", NBUF_PKT_TRAC_PROTO_STRING);
	} else if (NBUF_PKT_TRAC_TYPE_DHCP & cdf_nbuf_trace_get_proto_type(buf)) {
		cdf_mem_copy(string_buf + cdf_str_len(event_string),
			     "DHC", NBUF_PKT_TRAC_PROTO_STRING);
	} else if (NBUF_PKT_TRAC_TYPE_MGMT_ACTION &
		   cdf_nbuf_trace_get_proto_type(buf)) {
		cdf_mem_copy(string_buf + cdf_str_len(event_string),
			     "MACT", NBUF_PKT_TRAC_PROTO_STRING);
	}

	trace_update_cb(string_buf);
	return;
}
#endif /* QCA_PKT_PROTO_TRACE */

#ifdef MEMORY_DEBUG
#define CDF_NET_BUF_TRACK_MAX_SIZE    (1024)

/**
 * struct cdf_nbuf_track_t - Network buffer track structure
 *
 * @p_next: Pointer to next
 * @net_buf: Pointer to network buffer
 * @file_name: File name
 * @line_num: Line number
 * @size: Size
 */
struct cdf_nbuf_track_t {
	struct cdf_nbuf_track_t *p_next;
	cdf_nbuf_t net_buf;
	uint8_t *file_name;
	uint32_t line_num;
	size_t size;
};

spinlock_t g_cdf_net_buf_track_lock;
typedef struct cdf_nbuf_track_t CDF_NBUF_TRACK;

CDF_NBUF_TRACK *gp_cdf_net_buf_track_tbl[CDF_NET_BUF_TRACK_MAX_SIZE];

/**
 * cdf_net_buf_debug_init() - initialize network buffer debug functionality
 *
 * CDF network buffer debug feature tracks all SKBs allocated by WLAN driver
 * in a hash table and when driver is unloaded it reports about leaked SKBs.
 * WLAN driver module whose allocated SKB is freed by network stack are
 * suppose to call cdf_net_buf_debug_release_skb() such that the SKB is not
 * reported as memory leak.
 *
 * Return: none
 */
void cdf_net_buf_debug_init(void)
{
	uint32_t i;
	unsigned long irq_flag;

	spin_lock_init(&g_cdf_net_buf_track_lock);

	spin_lock_irqsave(&g_cdf_net_buf_track_lock, irq_flag);

	for (i = 0; i < CDF_NET_BUF_TRACK_MAX_SIZE; i++)
		gp_cdf_net_buf_track_tbl[i] = NULL;

	spin_unlock_irqrestore(&g_cdf_net_buf_track_lock, irq_flag);

	return;
}

/**
 * cdf_net_buf_debug_init() - exit network buffer debug functionality
 *
 * Exit network buffer tracking debug functionality and log SKB memory leaks
 *
 * Return: none
 */
void cdf_net_buf_debug_exit(void)
{
	uint32_t i;
	unsigned long irq_flag;
	CDF_NBUF_TRACK *p_node;
	CDF_NBUF_TRACK *p_prev;

	spin_lock_irqsave(&g_cdf_net_buf_track_lock, irq_flag);

	for (i = 0; i < CDF_NET_BUF_TRACK_MAX_SIZE; i++) {
		p_node = gp_cdf_net_buf_track_tbl[i];
		while (p_node) {
			p_prev = p_node;
			p_node = p_node->p_next;
			QDF_TRACE(QDF_MODULE_ID_QDF, QDF_TRACE_LEVEL_FATAL,
				  "SKB buf memory Leak@ File %s, @Line %d, size %zu\n",
				  p_prev->file_name, p_prev->line_num,
				  p_prev->size);
		}
	}

	spin_unlock_irqrestore(&g_cdf_net_buf_track_lock, irq_flag);

	return;
}

/**
 * cdf_net_buf_debug_clean() - clean up network buffer debug functionality
 *
 * Return: none
 */
void cdf_net_buf_debug_clean(void)
{
	uint32_t i;
	unsigned long irq_flag;
	CDF_NBUF_TRACK *p_node;
	CDF_NBUF_TRACK *p_prev;

	spin_lock_irqsave(&g_cdf_net_buf_track_lock, irq_flag);

	for (i = 0; i < CDF_NET_BUF_TRACK_MAX_SIZE; i++) {
		p_node = gp_cdf_net_buf_track_tbl[i];
		while (p_node) {
			p_prev = p_node;
			p_node = p_node->p_next;
			cdf_mem_free(p_prev);
		}
	}

	spin_unlock_irqrestore(&g_cdf_net_buf_track_lock, irq_flag);

	return;
}

/**
 * cdf_net_buf_debug_hash() - hash network buffer pointer
 *
 * Return: hash value
 */
uint32_t cdf_net_buf_debug_hash(cdf_nbuf_t net_buf)
{
	uint32_t i;

	i = (uint32_t) ((uintptr_t) net_buf & (CDF_NET_BUF_TRACK_MAX_SIZE - 1));

	return i;
}

/**
 * cdf_net_buf_debug_look_up() - look up network buffer in debug hash table
 *
 * Return: If skb is found in hash table then return pointer to network buffer
 *	else return %NULL
 */
CDF_NBUF_TRACK *cdf_net_buf_debug_look_up(cdf_nbuf_t net_buf)
{
	uint32_t i;
	CDF_NBUF_TRACK *p_node;

	i = cdf_net_buf_debug_hash(net_buf);
	p_node = gp_cdf_net_buf_track_tbl[i];

	while (p_node) {
		if (p_node->net_buf == net_buf)
			return p_node;
		p_node = p_node->p_next;
	}

	return NULL;

}

/**
 * cdf_net_buf_debug_add_node() - store skb in debug hash table
 *
 * Return: none
 */
void cdf_net_buf_debug_add_node(cdf_nbuf_t net_buf, size_t size,
				uint8_t *file_name, uint32_t line_num)
{
	uint32_t i;
	unsigned long irq_flag;
	CDF_NBUF_TRACK *p_node;

	spin_lock_irqsave(&g_cdf_net_buf_track_lock, irq_flag);

	i = cdf_net_buf_debug_hash(net_buf);
	p_node = cdf_net_buf_debug_look_up(net_buf);

	if (p_node) {
		QDF_TRACE(QDF_MODULE_ID_QDF, QDF_TRACE_LEVEL_ERROR,
			  "Double allocation of skb ! Already allocated from %s %d",
			  p_node->file_name, p_node->line_num);
		QDF_ASSERT(0);
		goto done;
	} else {
		p_node = (CDF_NBUF_TRACK *) cdf_mem_malloc(sizeof(*p_node));
		if (p_node) {
			p_node->net_buf = net_buf;
			p_node->file_name = file_name;
			p_node->line_num = line_num;
			p_node->size = size;
			p_node->p_next = gp_cdf_net_buf_track_tbl[i];
			gp_cdf_net_buf_track_tbl[i] = p_node;
		} else {
			QDF_TRACE(QDF_MODULE_ID_QDF, QDF_TRACE_LEVEL_ERROR,
				  "Mem alloc failed ! Could not track skb from %s %d of size %zu",
				  file_name, line_num, size);
			QDF_ASSERT(0);
		}
	}

done:
	spin_unlock_irqrestore(&g_cdf_net_buf_track_lock, irq_flag);

	return;
}

/**
 * cdf_net_buf_debug_delete_node() - remove skb from debug hash table
 *
 * Return: none
 */
void cdf_net_buf_debug_delete_node(cdf_nbuf_t net_buf)
{
	uint32_t i;
	bool found = false;
	CDF_NBUF_TRACK *p_head;
	CDF_NBUF_TRACK *p_node;
	unsigned long irq_flag;
	CDF_NBUF_TRACK *p_prev;

	spin_lock_irqsave(&g_cdf_net_buf_track_lock, irq_flag);

	i = cdf_net_buf_debug_hash(net_buf);
	p_head = gp_cdf_net_buf_track_tbl[i];

	/* Unallocated SKB */
	if (!p_head)
		goto done;

	p_node = p_head;
	/* Found at head of the table */
	if (p_head->net_buf == net_buf) {
		gp_cdf_net_buf_track_tbl[i] = p_node->p_next;
		cdf_mem_free((void *)p_node);
		found = true;
		goto done;
	}

	/* Search in collision list */
	while (p_node) {
		p_prev = p_node;
		p_node = p_node->p_next;
		if ((NULL != p_node) && (p_node->net_buf == net_buf)) {
			p_prev->p_next = p_node->p_next;
			cdf_mem_free((void *)p_node);
			found = true;
			break;
		}
	}

done:
	if (!found) {
		QDF_TRACE(QDF_MODULE_ID_QDF, QDF_TRACE_LEVEL_ERROR,
			  "Unallocated buffer ! Double free of net_buf %p ?",
			  net_buf);
		QDF_ASSERT(0);
	}

	spin_unlock_irqrestore(&g_cdf_net_buf_track_lock, irq_flag);

	return;
}

/**
 * cdf_net_buf_debug_release_skb() - release skb to avoid memory leak
 *
 * WLAN driver module whose allocated SKB is freed by network stack are
 * suppose to call this API before returning SKB to network stack such
 * that the SKB is not reported as memory leak.
 *
 * Return: none
 */
void cdf_net_buf_debug_release_skb(cdf_nbuf_t net_buf)
{
	cdf_net_buf_debug_delete_node(net_buf);
}

#endif /*MEMORY_DEBUG */
#if defined(FEATURE_TSO)

struct cdf_tso_cmn_seg_info_t {
	uint16_t ethproto;
	uint16_t ip_tcp_hdr_len;
	uint16_t l2_len;
	unsigned char *eit_hdr;
	unsigned int eit_hdr_len;
	struct tcphdr *tcphdr;
	uint16_t ipv4_csum_en;
	uint16_t tcp_ipv4_csum_en;
	uint16_t tcp_ipv6_csum_en;
	uint16_t ip_id;
	uint32_t tcp_seq_num;
};

/**
 * __cdf_nbuf_get_tso_cmn_seg_info() - get TSO common
 * information
 *
 * Get the TSO information that is common across all the TCP
 * segments of the jumbo packet
 *
 * Return: 0 - success 1 - failure
 */
uint8_t __cdf_nbuf_get_tso_cmn_seg_info(struct sk_buff *skb,
	struct cdf_tso_cmn_seg_info_t *tso_info)
{
	/* Get ethernet type and ethernet header length */
	tso_info->ethproto = vlan_get_protocol(skb);

	/* Determine whether this is an IPv4 or IPv6 packet */
	if (tso_info->ethproto == htons(ETH_P_IP)) { /* IPv4 */
		/* for IPv4, get the IP ID and enable TCP and IP csum */
		struct iphdr *ipv4_hdr = ip_hdr(skb);
		tso_info->ip_id = ntohs(ipv4_hdr->id);
		tso_info->ipv4_csum_en = 1;
		tso_info->tcp_ipv4_csum_en = 1;
		if (qdf_unlikely(ipv4_hdr->protocol != IPPROTO_TCP)) {
			qdf_print("TSO IPV4 proto 0x%x not TCP\n",
				 ipv4_hdr->protocol);
			return 1;
		}
	} else if (tso_info->ethproto == htons(ETH_P_IPV6)) { /* IPv6 */
		/* for IPv6, enable TCP csum. No IP ID or IP csum */
		tso_info->tcp_ipv6_csum_en = 1;
	} else {
		qdf_print("TSO: ethertype 0x%x is not supported!\n",
			 tso_info->ethproto);
		return 1;
	}

	tso_info->l2_len = (skb_network_header(skb) - skb_mac_header(skb));
	tso_info->tcphdr = tcp_hdr(skb);
	tso_info->tcp_seq_num = ntohl(tcp_hdr(skb)->seq);
	/* get pointer to the ethernet + IP + TCP header and their length */
	tso_info->eit_hdr = skb->data;
	tso_info->eit_hdr_len = (skb_transport_header(skb)
		 - skb_mac_header(skb)) + tcp_hdrlen(skb);
	tso_info->ip_tcp_hdr_len = tso_info->eit_hdr_len - tso_info->l2_len;
	return 0;
}

/**
 * cdf_dmaaddr_to_32s - return high and low parts of dma_addr
 *
 * Returns the high and low 32-bits of the DMA addr in the provided ptrs
 *
 * Return: N/A
*/
static inline void cdf_dmaaddr_to_32s(qdf_dma_addr_t dmaaddr,
				      uint32_t *lo, uint32_t *hi)
{
	if (sizeof(dmaaddr) > sizeof(uint32_t)) {
		*lo = (uint32_t) (dmaaddr & 0x0ffffffff);
		*hi = (uint32_t) (dmaaddr >> 32);
	} else {
		*lo = dmaaddr;
		*hi = 0;
	}
}
/**
 * __cdf_nbuf_get_tso_info() - function to divide a TSO nbuf
 * into segments
 * @nbuf:   network buffer to be segmented
 * @tso_info:  This is the output. The information about the
 *      TSO segments will be populated within this.
 *
 * This function fragments a TCP jumbo packet into smaller
 * segments to be transmitted by the driver. It chains the TSO
 * segments created into a list.
 *
 * Return: number of TSO segments
 */
uint32_t __cdf_nbuf_get_tso_info(qdf_device_t osdev, struct sk_buff *skb,
		struct qdf_tso_info_t *tso_info)
{
	/* common accross all segments */
	struct cdf_tso_cmn_seg_info_t tso_cmn_info;

	/* segment specific */
	char *tso_frag_vaddr;
	qdf_dma_addr_t tso_frag_paddr = 0;
	uint32_t       tso_frag_paddr_lo, tso_frag_paddr_hi;
	uint32_t num_seg = 0;
	struct cdf_tso_seg_elem_t *curr_seg;
	const struct skb_frag_struct *frag = NULL;
	uint32_t tso_frag_len = 0; /* tso segment's fragment length*/
	uint32_t skb_frag_len = 0; /* skb's fragment length (continous memory)*/
	uint32_t foffset = 0; /* offset into the skb's fragment */
	uint32_t skb_proc = 0; /* bytes of the skb that have been processed*/
	uint32_t tso_seg_size = skb_shinfo(skb)->gso_size;

	memset(&tso_cmn_info, 0x0, sizeof(tso_cmn_info));

	if (qdf_unlikely(__cdf_nbuf_get_tso_cmn_seg_info(skb, &tso_cmn_info))) {
		qdf_print("TSO: error getting common segment info\n");
		return 0;
	}
	curr_seg = tso_info->tso_seg_list;

	/* length of the first chunk of data in the skb */
	skb_proc = skb_frag_len = skb_headlen(skb);

	/* the 0th tso segment's 0th fragment always contains the EIT header */
	/* update the remaining skb fragment length and TSO segment length */
	skb_frag_len -= tso_cmn_info.eit_hdr_len;
	skb_proc -= tso_cmn_info.eit_hdr_len;

	/* get the address to the next tso fragment */
	tso_frag_vaddr = skb->data + tso_cmn_info.eit_hdr_len;
	/* get the length of the next tso fragment */
	tso_frag_len = min(skb_frag_len, tso_seg_size);
	tso_frag_paddr = dma_map_single(osdev->dev,
		 tso_frag_vaddr, tso_frag_len, DMA_TO_DEVICE);
	cdf_dmaaddr_to_32s(tso_frag_paddr, &tso_frag_paddr_lo, &tso_frag_paddr_hi);

	num_seg = tso_info->num_segs;
	tso_info->num_segs = 0;
	tso_info->is_tso = 1;

	while (num_seg && curr_seg) {
		int i = 1; /* tso fragment index */
		int j = 0; /* skb fragment index */
		uint8_t more_tso_frags = 1;
		uint8_t from_frag_table = 0;

		/* Initialize the flags to 0 */
		memset(&curr_seg->seg, 0x0, sizeof(curr_seg->seg));
		tso_info->num_segs++;

		/* The following fields remain the same across all segments of
		 a jumbo packet */
		curr_seg->seg.tso_flags.tso_enable = 1;
		curr_seg->seg.tso_flags.partial_checksum_en = 0;
		curr_seg->seg.tso_flags.ipv4_checksum_en =
			tso_cmn_info.ipv4_csum_en;
		curr_seg->seg.tso_flags.tcp_ipv6_checksum_en =
			tso_cmn_info.tcp_ipv6_csum_en;
		curr_seg->seg.tso_flags.tcp_ipv4_checksum_en =
			tso_cmn_info.tcp_ipv4_csum_en;
		curr_seg->seg.tso_flags.l2_len = 0;
		curr_seg->seg.tso_flags.tcp_flags_mask = 0x1FF;
		curr_seg->seg.num_frags = 0;

		/* The following fields change for the segments */
		curr_seg->seg.tso_flags.ip_id = tso_cmn_info.ip_id;
		tso_cmn_info.ip_id++;

		curr_seg->seg.tso_flags.syn = tso_cmn_info.tcphdr->syn;
		curr_seg->seg.tso_flags.rst = tso_cmn_info.tcphdr->rst;
		curr_seg->seg.tso_flags.psh = tso_cmn_info.tcphdr->psh;
		curr_seg->seg.tso_flags.ack = tso_cmn_info.tcphdr->ack;
		curr_seg->seg.tso_flags.urg = tso_cmn_info.tcphdr->urg;
		curr_seg->seg.tso_flags.ece = tso_cmn_info.tcphdr->ece;
		curr_seg->seg.tso_flags.cwr = tso_cmn_info.tcphdr->cwr;

		curr_seg->seg.tso_flags.tcp_seq_num = tso_cmn_info.tcp_seq_num;

		/* First fragment for each segment always contains the ethernet,
		IP and TCP header */
		curr_seg->seg.tso_frags[0].vaddr = tso_cmn_info.eit_hdr;
		curr_seg->seg.tso_frags[0].length = tso_cmn_info.eit_hdr_len;
		tso_info->total_len = curr_seg->seg.tso_frags[0].length;
		{
			qdf_dma_addr_t mapped;
			uint32_t       lo, hi;

			mapped = dma_map_single(osdev->dev, tso_cmn_info.eit_hdr,
				tso_cmn_info.eit_hdr_len, DMA_TO_DEVICE);
			cdf_dmaaddr_to_32s(mapped, &lo, &hi);
			curr_seg->seg.tso_frags[0].paddr_low_32 = lo;
			curr_seg->seg.tso_frags[0].paddr_upper_16 = (hi & 0xffff);
		}
		curr_seg->seg.tso_flags.ip_len = tso_cmn_info.ip_tcp_hdr_len;
		curr_seg->seg.num_frags++;

		while (more_tso_frags) {
			curr_seg->seg.tso_frags[i].vaddr = tso_frag_vaddr;
			curr_seg->seg.tso_frags[i].length = tso_frag_len;
			tso_info->total_len +=
				 curr_seg->seg.tso_frags[i].length;
			curr_seg->seg.tso_flags.ip_len +=
				 curr_seg->seg.tso_frags[i].length;
			curr_seg->seg.num_frags++;
			skb_proc = skb_proc - curr_seg->seg.tso_frags[i].length;

			/* increment the TCP sequence number */
			tso_cmn_info.tcp_seq_num += tso_frag_len;
			curr_seg->seg.tso_frags[i].paddr_upper_16 =
				(tso_frag_paddr_hi & 0xffff);
			curr_seg->seg.tso_frags[i].paddr_low_32 =
				 tso_frag_paddr_lo;

			/* if there is no more data left in the skb */
			if (!skb_proc)
				return tso_info->num_segs;

			/* get the next payload fragment information */
			/* check if there are more fragments in this segment */
			if ((tso_seg_size - tso_frag_len)) {
				more_tso_frags = 1;
				i++;
			} else {
				more_tso_frags = 0;
				/* reset i and the tso payload size */
				i = 1;
				tso_seg_size = skb_shinfo(skb)->gso_size;
			}

			/* if the next fragment is contiguous */
			if (tso_frag_len < skb_frag_len) {
				skb_frag_len = skb_frag_len - tso_frag_len;
				tso_frag_len = min(skb_frag_len, tso_seg_size);
				tso_frag_vaddr = tso_frag_vaddr + tso_frag_len;
				if (from_frag_table) {
					tso_frag_paddr =
						 skb_frag_dma_map(osdev->dev,
							 frag, foffset,
							 tso_frag_len,
							 DMA_TO_DEVICE);
					cdf_dmaaddr_to_32s(tso_frag_paddr, &tso_frag_paddr_lo, &tso_frag_paddr_hi);
				} else {
					tso_frag_paddr =
						 dma_map_single(osdev->dev,
							 tso_frag_vaddr,
							 tso_frag_len,
							 DMA_TO_DEVICE);
					cdf_dmaaddr_to_32s(tso_frag_paddr, &tso_frag_paddr_lo, &tso_frag_paddr_hi);
				}
			} else { /* the next fragment is not contiguous */
				tso_frag_len = min(skb_frag_len, tso_seg_size);
				frag = &skb_shinfo(skb)->frags[j];
				skb_frag_len = skb_frag_size(frag);

				tso_frag_vaddr = skb_frag_address(frag);
				tso_frag_paddr = skb_frag_dma_map(osdev->dev,
					 frag, 0, tso_frag_len,
					 DMA_TO_DEVICE);
				cdf_dmaaddr_to_32s(tso_frag_paddr, &tso_frag_paddr_lo, &tso_frag_paddr_hi);
				foffset += tso_frag_len;
				from_frag_table = 1;
				j++;
			}
		}
		num_seg--;
		/* if TCP FIN flag was set, set it in the last segment */
		if (!num_seg)
			curr_seg->seg.tso_flags.fin = tso_cmn_info.tcphdr->fin;

		curr_seg = curr_seg->next;
	}
	return tso_info->num_segs;
}

/**
 * __cdf_nbuf_get_tso_num_seg() - function to divide a TSO nbuf
 * into segments
 * @nbuf:   network buffer to be segmented
 * @tso_info:  This is the output. The information about the
 *      TSO segments will be populated within this.
 *
 * This function fragments a TCP jumbo packet into smaller
 * segments to be transmitted by the driver. It chains the TSO
 * segments created into a list.
 *
 * Return: 0 - success, 1 - failure
 */
uint32_t __cdf_nbuf_get_tso_num_seg(struct sk_buff *skb)
{
	uint32_t gso_size, tmp_len, num_segs = 0;

	gso_size = skb_shinfo(skb)->gso_size;
	tmp_len = skb->len - ((skb_transport_header(skb) - skb_mac_header(skb))
		+ tcp_hdrlen(skb));
	while (tmp_len) {
		num_segs++;
		if (tmp_len > gso_size)
			tmp_len -= gso_size;
		else
			break;
	}
	return num_segs;
}

#endif /* FEATURE_TSO */
