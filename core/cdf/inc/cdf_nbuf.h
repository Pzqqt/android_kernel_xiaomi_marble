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
 * DOC: cdf_nbuf_public network buffer API
 * This file defines the network buffer abstraction.
 */

#ifndef _CDF_NBUF_H
#define _CDF_NBUF_H
#include <cdf_util.h>
#include <cdf_types.h>
#include <cdf_net_types.h>
#include <cdf_lock.h>
#include <i_cdf_nbuf.h>
#include <asm/cacheflush.h>

#define IPA_NBUF_OWNER_ID 0xaa55aa55
#define NBUF_PKT_TRAC_TYPE_EAPOL   0x02
#define NBUF_PKT_TRAC_TYPE_DHCP    0x04
#define NBUF_PKT_TRAC_TYPE_MGMT_ACTION    0x08
#define NBUF_PKT_TRAC_MAX_STRING   12
#define NBUF_PKT_TRAC_PROTO_STRING 4
#define NBUF_PKT_ERROR  1

/* Tracked Packet types */
#define NBUF_TX_PKT_INVALID              0
#define NBUF_TX_PKT_DATA_TRACK           1
#define NBUF_TX_PKT_MGMT_TRACK           2

/* Different Packet states */
#define NBUF_TX_PKT_HDD                  1
#define NBUF_TX_PKT_TXRX_ENQUEUE         2
#define NBUF_TX_PKT_TXRX_DEQUEUE         3
#define NBUF_TX_PKT_TXRX                 4
#define NBUF_TX_PKT_HTT                  5
#define NBUF_TX_PKT_HTC                  6
#define NBUF_TX_PKT_HIF                  7
#define NBUF_TX_PKT_CE                   8
#define NBUF_TX_PKT_FREE                 9
#define NBUF_TX_PKT_STATE_MAX            10


/**
 * @cdf_nbuf_t - Platform indepedent packet abstraction
 */
typedef __cdf_nbuf_t cdf_nbuf_t;

/**
 * @cdf_dma_map_cb_t - Dma map callback prototype
 */
typedef void (*cdf_dma_map_cb_t)(void *arg, cdf_nbuf_t buf,
				 cdf_dma_map_t dmap);

/**
 * @__CDF_NBUF_NULL - invalid handle
 */
#define CDF_NBUF_NULL   __CDF_NBUF_NULL
/**
 * @cdf_nbuf_queue_t - Platform independent packet queue abstraction
 */
typedef __cdf_nbuf_queue_t cdf_nbuf_queue_t;

/* BUS/DMA mapping routines */

/**
 * cdf_nbuf_map() - map a buffer to local bus address space
 * @osdev: OS device
 * @buf: Buf to be mapped (mapping info is stored in the buf's meta-data area)
 * @dir: DMA direction
 *
 * Return: Status of the operation
 */
static inline QDF_STATUS
cdf_nbuf_map(cdf_device_t osdev, cdf_nbuf_t buf, cdf_dma_dir_t dir)
{
	return __cdf_nbuf_map(osdev, buf, dir);
}

/**
 * cdf_nbuf_unmap() - unmap a previously mapped buf
 * @osdev: OS device
 * @buf: Buf to be unmapped (mapping info is stored in the buf's meta-data area)
 * @dir: DMA direction
 *
 * Return: none
 */
static inline void
cdf_nbuf_unmap(cdf_device_t osdev, cdf_nbuf_t buf, cdf_dma_dir_t dir)
{
	__cdf_nbuf_unmap(osdev, buf, dir);
}

/**
 * cdf_nbuf_map_single() - map a single buffer to local bus address space
 * @osdev: OS device
 * @buf: Buf to be mapped (mapping info is stored in the buf's meta-data area)
 * @dir: DMA direction
 *
 * Return: Status of the operation
 */
static inline QDF_STATUS
cdf_nbuf_map_single(cdf_device_t osdev, cdf_nbuf_t buf, cdf_dma_dir_t dir)
{
	return __cdf_nbuf_map_single(osdev, buf, dir);
}

/**
 * cdf_nbuf_unmap_single() - unmap a previously mapped buf
 * @osdev: OS device
 * @buf: Buf to be unmapped (mapping info is stored in the buf's meta-data area)
 * @dir: DMA direction
 *
 * Return: none
 */
static inline void
cdf_nbuf_unmap_single(cdf_device_t osdev, cdf_nbuf_t buf, cdf_dma_dir_t dir)
{
	__cdf_nbuf_unmap_single(osdev, buf, dir);
}

/**
 * cdf_nbuf_get_num_frags() - get number of fragments
 * @buf: Network buffer
 *
 * Return: Number of fragments
 */
static inline int cdf_nbuf_get_num_frags(cdf_nbuf_t buf)
{
	return __cdf_nbuf_get_num_frags(buf);
}

/**
 * cdf_nbuf_get_frag_len() - get fragment length
 * @buf: Network buffer
 * @frag_num: Fragment number
 *
 * Return: Fragment length
 */
static inline int cdf_nbuf_get_frag_len(cdf_nbuf_t buf, int frag_num)
{
	BUG_ON(frag_num >= NBUF_CB_TX_MAX_EXTRA_FRAGS);
	return __cdf_nbuf_get_frag_len(buf, frag_num);
}

/**
 * cdf_nbuf_get_frag_vaddr() - get fragment virtual address
 * @buf: Network buffer
 * @frag_num: Fragment number
 *
 * Return: Fragment virtual address
 */
static inline unsigned char *cdf_nbuf_get_frag_vaddr(cdf_nbuf_t buf,
						     int frag_num)
{
	BUG_ON(frag_num >= NBUF_CB_TX_MAX_EXTRA_FRAGS);
	return __cdf_nbuf_get_frag_vaddr(buf, frag_num);
}

/**
 * cdf_nbuf_get_frag_paddr() - get fragment physical address
 * @buf: Network buffer
 * @frag_num: Fragment number
 *
 * Return: Fragment physical address
 */
static inline cdf_dma_addr_t cdf_nbuf_get_frag_paddr(cdf_nbuf_t buf, int frag_num)
{
	BUG_ON(frag_num >= NBUF_CB_TX_MAX_EXTRA_FRAGS);
	return __cdf_nbuf_get_frag_paddr(buf, frag_num);
}

/**
 * cdf_nbuf_get_frag_is_wordstream() - is fragment wordstream
 * @buf: Network buffer
 * @frag_num: Fragment number
 *
 * Return: Fragment wordstream or not
 */
static inline int cdf_nbuf_get_frag_is_wordstream(cdf_nbuf_t buf, int frag_num)
{
	BUG_ON(frag_num >= NBUF_CB_TX_MAX_EXTRA_FRAGS);
	return __cdf_nbuf_get_frag_is_wordstream(buf, frag_num);
}

/**
 * cdf_nbuf_set_frag_is_wordstream() - set fragment wordstream
 * @buf: Network buffer
 * @frag_num: Fragment number
 * @is_wordstream: Wordstream
 *
 * Return: none
 */
static inline void
cdf_nbuf_set_frag_is_wordstream(cdf_nbuf_t buf, int frag_num, int is_wordstream)
{
	BUG_ON(frag_num >= NBUF_CB_TX_MAX_EXTRA_FRAGS);
	__cdf_nbuf_set_frag_is_wordstream(buf, frag_num, is_wordstream);
}

/**
 * cdf_nbuf_ipa_owned_get - gets the ipa_owned flag
 * @buf: Network buffer
 *
 * Return: none
 */
static inline int cdf_nbuf_ipa_owned_get(cdf_nbuf_t buf)
{
	return __cdf_nbuf_ipa_owned_get(buf);
}

/**
 * cdf_nbuf_ipa_owned_set - sets the ipa_owned flag
 * @buf: Network buffer
 *
 * Return: none
 */
static inline void cdf_nbuf_ipa_owned_set(cdf_nbuf_t buf)
{
	__cdf_nbuf_ipa_owned_set(buf);
}

/**
 * cdf_nbuf_ipa_priv_get - gets the ipa_priv field
 * @buf: Network buffer
 *
 * Return: none
 */
static inline int cdf_nbuf_ipa_priv_get(cdf_nbuf_t buf)
{
	return __cdf_nbuf_ipa_priv_get(buf);
}

/**
 * cdf_nbuf_ipa_priv_set - sets the ipa_priv field
 * @buf: Network buffer
 *
 * Return: none
 */
static inline void cdf_nbuf_ipa_priv_set(cdf_nbuf_t buf, uint32_t priv)
{
	BUG_ON(priv & 0x80000000); /* priv is 31 bits only */
	__cdf_nbuf_ipa_priv_set(buf, priv);
}

/**
 * cdf_nbuf_mapped_paddr_get - gets the paddr of nbuf->data
 * @buf: Network buffer
 *
 * Return: none
 */
static inline cdf_dma_addr_t
cdf_nbuf_mapped_paddr_get(cdf_nbuf_t buf)
{
	return __cdf_nbuf_mapped_paddr_get(buf);
}

/**
 * cdf_nbuf_mapped_paddr_set - sets the paddr of nbuf->data
 * @buf: Network buffer
 *
 * Return: none
 */
static inline void
cdf_nbuf_mapped_paddr_set(cdf_nbuf_t buf, cdf_dma_addr_t paddr)
{
	__cdf_nbuf_mapped_paddr_set(buf, paddr);
}

/**
 * cdf_nbuf_frag_push_head() - push fragment head
 * @buf: Network buffer
 * @frag_len: Fragment length
 * @frag_vaddr: Fragment virtual address
 * @frag_paddr_lo: Fragment physical address lo
 * @frag_paddr_hi: Fragment physical address hi
 *
 * Return: none
 */
static inline void
cdf_nbuf_frag_push_head(cdf_nbuf_t buf,
			int frag_len,
			char *frag_vaddr,
			cdf_dma_addr_t frag_paddr)
{
	__cdf_nbuf_frag_push_head(buf, frag_len, frag_vaddr, frag_paddr);
}

#ifdef MEMORY_DEBUG
void cdf_net_buf_debug_init(void);
void cdf_net_buf_debug_exit(void);
void cdf_net_buf_debug_clean(void);
void cdf_net_buf_debug_add_node(cdf_nbuf_t net_buf, size_t size,
				uint8_t *file_name, uint32_t line_num);
void cdf_net_buf_debug_delete_node(cdf_nbuf_t net_buf);
void cdf_net_buf_debug_release_skb(cdf_nbuf_t net_buf);

/* nbuf allocation rouines */

/**
 * cdf_nbuf_alloc() - Allocate cdf_nbuf
 * @hdl: Platform device object
 * @size: Data buffer size for this cdf_nbuf including max header
 *	  size
 * @reserve: Headroom to start with.
 * @align: Alignment for the start buffer.
 * @prio: Indicate if the nbuf is high priority (some OSes e.g darwin
 *	   polls few times if allocation fails and priority is true)
 *
 * The nbuf created is guarenteed to have only 1 physical segment
 *
 * Return: The new cdf_nbuf instance or NULL if there's not enough memory.
 */

#define cdf_nbuf_alloc(d, s, r, a, p)			\
	cdf_nbuf_alloc_debug(d, s, r, a, p, __FILE__, __LINE__)
static inline cdf_nbuf_t
cdf_nbuf_alloc_debug(cdf_device_t osdev, cdf_size_t size, int reserve,
		     int align, int prio, uint8_t *file_name,
		     uint32_t line_num)
{
	cdf_nbuf_t net_buf;
	net_buf = __cdf_nbuf_alloc(osdev, size, reserve, align, prio);

	/* Store SKB in internal CDF tracking table */
	if (cdf_likely(net_buf))
		cdf_net_buf_debug_add_node(net_buf, size, file_name, line_num);

	return net_buf;
}

/**
 * cdf_nbuf_free() - free cdf_nbuf
 * @net_buf: Network buffer to free
 *
 * Return: none
 */
static inline void cdf_nbuf_free(cdf_nbuf_t net_buf)
{
	/* Remove SKB from internal CDF tracking table */
	if (cdf_likely(net_buf))
		cdf_net_buf_debug_delete_node(net_buf);

	__cdf_nbuf_free(net_buf);
}

#else

static inline void cdf_net_buf_debug_release_skb(cdf_nbuf_t net_buf)
{
	return;
}

/* Nbuf allocation rouines */

/**
 * cdf_nbuf_alloc() - allocate cdf_nbuf
 * @hdl: Platform device object
 * @size: Data buffer size for this cdf_nbuf including max header
 *	  size
 * @reserve: Headroom to start with.
 * @align: Alignment for the start buffer.
 * @prio: Indicate if the nbuf is high priority (some OSes e.g darwin
 *	  polls few times if allocation fails and priority is  true)
 *
 * The nbuf created is guarenteed to have only 1 physical segment
 *
 * Return: new cdf_nbuf instance or NULL if there's not enough memory.
 */
static inline cdf_nbuf_t
cdf_nbuf_alloc(cdf_device_t osdev,
	       cdf_size_t size, int reserve, int align, int prio)
{
	return __cdf_nbuf_alloc(osdev, size, reserve, align, prio);
}

/**
 * cdf_nbuf_free() - free cdf_nbuf
 * @buf: Network buffer to free
 *
 * Return: none
 */
static inline void cdf_nbuf_free(cdf_nbuf_t buf)
{
	__cdf_nbuf_free(buf);
}

#endif

/**
 * cdf_nbuf_tx_free() - free a list of cdf_nbufs and tell the OS their tx
 *			status (if req'd)
 * @bufs: List of netbufs to free
 * @tx_err: Whether the tx frames were transmitted successfully
 *
 * Return: none
 */
static inline void cdf_nbuf_tx_free(cdf_nbuf_t buf_list, int tx_err)
{
	__cdf_nbuf_tx_free(buf_list, tx_err);
}

/**
 * cdf_nbuf_copy() - copy src buffer into dst.
 * @buf: source nbuf to copy from
 *
 * This API is useful, for example, because most native buffer provide a way to
 * copy a chain into a single buffer. Therefore as a side effect, it also
 * "linearizes" a buffer (which is perhaps why you'll use it mostly). It
 * creates a writeable copy.
 *
 *
 * Return: new nbuf
 */
static inline cdf_nbuf_t cdf_nbuf_copy(cdf_nbuf_t buf)
{
	return __cdf_nbuf_copy(buf);
}

/**
 * cdf_nbuf_cat() - link two nbufs, the new buf is piggybacked into older one
 * @dst: Buffer to piggyback into
 * @src: Buffer to put
 *
 * Return: Status of the call - 0 successful
 */
static inline QDF_STATUS cdf_nbuf_cat(cdf_nbuf_t dst, cdf_nbuf_t src)
{
	return __cdf_nbuf_cat(dst, src);
}

/**
 * @cdf_nbuf_copy_bits() - return the length of the copy bits for skb
 * @skb: SKB pointer
 * @offset: offset
 * @len: Length
 * @to: To
 *
 * Return: int32_t
 */
static inline int32_t
cdf_nbuf_copy_bits(cdf_nbuf_t nbuf, uint32_t offset, uint32_t len, void *to)
{
	return __cdf_nbuf_copy_bits(nbuf, offset, len, to);
}

/**
 * cdf_nbuf_clone() - clone the nbuf (copy is readonly)
 * @buf: nbuf to clone from
 *
 * Return: cloned buffer
 */
static inline cdf_nbuf_t cdf_nbuf_clone(cdf_nbuf_t buf)
{
	return __cdf_nbuf_clone(buf);
}

/* nbuf manipulation routines */

/**
 * @cdf_nbuf_head() - return the address of an nbuf's buffer
 * @buf: netbuf
 *
 * Return: head address
 */
static inline uint8_t *cdf_nbuf_head(cdf_nbuf_t buf)
{
	return __cdf_nbuf_head(buf);
}

/**
 * cdf_nbuf_data() - Return the address of the start of data within an nbuf
 * @buf: Network buffer
 *
 * Return: Data address
 */
static inline uint8_t *cdf_nbuf_data(cdf_nbuf_t buf)
{
	return __cdf_nbuf_data(buf);
}

/**
 * cdf_nbuf_headroom() - amount of headroom int the current nbuf
 * @buf: Network buffer
 *
 * Return: Amount of head room
 */
static inline uint32_t cdf_nbuf_headroom(cdf_nbuf_t buf)
{
	return __cdf_nbuf_headroom(buf);
}

/**
 * cdf_nbuf_tailroom() - amount of tail space available
 * @buf: Network buffer
 *
 * Return: amount of tail room
 */
static inline uint32_t cdf_nbuf_tailroom(cdf_nbuf_t buf)
{
	return __cdf_nbuf_tailroom(buf);
}

/**
 * cdf_nbuf_push_head() - push data in the front
 * @buf: Network buf instance
 * @size: Size to be pushed
 *
 * Return: New data pointer of this buf after data has been pushed,
 *	   or NULL if there is not enough room in this buf.
 */
static inline uint8_t *cdf_nbuf_push_head(cdf_nbuf_t buf, cdf_size_t size)
{
	return __cdf_nbuf_push_head(buf, size);
}

/**
 * cdf_nbuf_put_tail() - puts data in the end
 * @buf: Network buf instance
 * @size: Size to be pushed
 *
 * Return: Data pointer of this buf where new data has to be
 *	   put, or NULL if there is not enough room in this buf.
 */
static inline uint8_t *cdf_nbuf_put_tail(cdf_nbuf_t buf, cdf_size_t size)
{
	return __cdf_nbuf_put_tail(buf, size);
}

/**
 * cdf_nbuf_pull_head() - pull data out from the front
 * @buf: Network buf instance
 * @size: Size to be popped
 *
 * Return: New data pointer of this buf after data has been popped,
 *	   or NULL if there is not sufficient data to pull.
 */
static inline uint8_t *cdf_nbuf_pull_head(cdf_nbuf_t buf, cdf_size_t size)
{
	return __cdf_nbuf_pull_head(buf, size);
}

/**
 * cdf_nbuf_trim_tail() - trim data out from the end
 * @buf: Network buf instance
 * @size: Size to be popped
 *
 * Return: none
 */
static inline void cdf_nbuf_trim_tail(cdf_nbuf_t buf, cdf_size_t size)
{
	__cdf_nbuf_trim_tail(buf, size);
}

/**
 * cdf_nbuf_len() - get the length of the buf
 * @buf: Network buf instance
 *
 * Return: total length of this buf.
 */
static inline cdf_size_t cdf_nbuf_len(cdf_nbuf_t buf)
{
	return __cdf_nbuf_len(buf);
}

/**
 * cdf_nbuf_set_pktlen() - set the length of the buf
 * @buf: Network buf instance
 * @size: Size to be set
 *
 * Return: none
 */
static inline void cdf_nbuf_set_pktlen(cdf_nbuf_t buf, uint32_t len)
{
	__cdf_nbuf_set_pktlen(buf, len);
}

/**
 * cdf_nbuf_reserve() - trim data out from the end
 * @buf: Network buf instance
 * @size: Size to be popped
 *
 * Return: none
 */
static inline void cdf_nbuf_reserve(cdf_nbuf_t buf, cdf_size_t size)
{
	__cdf_nbuf_reserve(buf, size);
}

/**
 * cdf_nbuf_peek_header() - return the data pointer & length of the header
 * @buf: Network nbuf
 * @addr: Data pointer
 * @len: Length of the data
 *
 * Return: none
 */
static inline void
cdf_nbuf_peek_header(cdf_nbuf_t buf, uint8_t **addr, uint32_t *len)
{
	__cdf_nbuf_peek_header(buf, addr, len);
}

/* nbuf private context routines */

/* nbuf queue routines */

/**
 * cdf_nbuf_queue_init() - initialize buf queue
 * @head: Network buf queue head
 *
 * Return: none
 */
static inline void cdf_nbuf_queue_init(cdf_nbuf_queue_t *head)
{
	__cdf_nbuf_queue_init(head);
}

/**
 * cdf_nbuf_queue_add() - append a nbuf to the tail of the buf queue
 * @head: Network buf queue head
 * @buf: Network buf
 *
 * Return: none
 */
static inline void cdf_nbuf_queue_add(cdf_nbuf_queue_t *head, cdf_nbuf_t buf)
{
	__cdf_nbuf_queue_add(head, buf);
}

/**
 * cdf_nbuf_queue_insert_head() - insert nbuf at the head of queue
 * @head: Network buf queue head
 * @buf: Network buf
 *
 * Return: none
 */
static inline void
cdf_nbuf_queue_insert_head(cdf_nbuf_queue_t *head, cdf_nbuf_t buf)
{
	__cdf_nbuf_queue_insert_head(head, buf);
}

/**
 * cdf_nbuf_queue_remove() - retrieve a buf from the head of the buf queue
 * @head: Network buf queue head
 *
 * Return: The head buf in the buf queue.
 */
static inline cdf_nbuf_t cdf_nbuf_queue_remove(cdf_nbuf_queue_t *head)
{
	return __cdf_nbuf_queue_remove(head);
}

/**
 * cdf_nbuf_queue_len() - get the length of the queue
 * @head: Network buf queue head
 *
 * Return: length of the queue
 */
static inline uint32_t cdf_nbuf_queue_len(cdf_nbuf_queue_t *head)
{
	return __cdf_nbuf_queue_len(head);
}

/**
 * cdf_nbuf_queue_next() - get the next guy/packet of the given buffer
 * @buf: Network buffer
 *
 * Return: next buffer/packet
 */
static inline cdf_nbuf_t cdf_nbuf_queue_next(cdf_nbuf_t buf)
{
	return __cdf_nbuf_queue_next(buf);
}

/**
 * @cdf_nbuf_is_queue_empty() - check if the buf queue is empty
 * @nbq: Network buf queue handle
 *
 * Return: true  if queue is empty
 *	   false if queue is not emty
 */
static inline bool cdf_nbuf_is_queue_empty(cdf_nbuf_queue_t *nbq)
{
	return __cdf_nbuf_is_queue_empty(nbq);
}

/**
 * cdf_nbuf_next() - get the next packet in the linked list
 * @buf: Network buffer
 *
 * This function can be used when nbufs are directly linked into a list,
 * rather than using a separate network buffer queue object.
 *
 * Return: next network buffer in the linked list
 */
static inline cdf_nbuf_t cdf_nbuf_next(cdf_nbuf_t buf)
{
	return __cdf_nbuf_next(buf);
}

/**
 * cdf_nbuf_get_protocol() - return the protocol value of the skb
 * @skb: Pointer to network buffer
 *
 * Return: skb protocol
 */
static inline uint16_t cdf_nbuf_get_protocol(struct sk_buff *skb)
{
	return __cdf_nbuf_get_protocol(skb);
}

/**
 * cdf_nbuf_get_ip_summed() - return the ip checksum value of the skb
 * @skb: Pointer to network buffer
 *
 * Return: skb ip_summed
 */
static inline uint8_t cdf_nbuf_get_ip_summed(struct sk_buff *skb)
{
	return __cdf_nbuf_get_ip_summed(skb);
}

/**
 * cdf_nbuf_set_ip_summed() - sets the ip_summed value of the skb
 * @skb: Pointer to network buffer
 * @ip_summed: ip checksum
 *
 * Return: none
 */
static inline void cdf_nbuf_set_ip_summed(struct sk_buff *skb, uint8_t ip_summed)
{
	__cdf_nbuf_set_ip_summed(skb, ip_summed);
}

/**
 * cdf_nbuf_set_next() - add a packet to a linked list
 * @this_buf: Predecessor buffer
 * @next_buf: Successor buffer
 *
 * This function can be used to directly link nbufs, rather than using
 * a separate network buffer queue object.
 *
 * Return: none
 */
static inline void cdf_nbuf_set_next(cdf_nbuf_t this_buf, cdf_nbuf_t next_buf)
{
	__cdf_nbuf_set_next(this_buf, next_buf);
}

/* nbuf extension routines */

/**
 * cdf_nbuf_set_next_ext() - link extension of this packet contained in a new
 *			     nbuf
 * @this_buf: predecessor buffer
 * @next_buf: successor buffer
 *
 * This function is used to link up many nbufs containing a single logical
 * packet - not a collection of packets. Do not use for linking the first
 * extension to the head
 *
 * Return: none
 */
static inline void
cdf_nbuf_set_next_ext(cdf_nbuf_t this_buf, cdf_nbuf_t next_buf)
{
	__cdf_nbuf_set_next_ext(this_buf, next_buf);
}

/**
 * cdf_nbuf_next_ext() - get the next packet extension in the linked list
 * @buf: Network buffer
 *
 * Return: Next network buffer in the linked list
 */
static inline cdf_nbuf_t cdf_nbuf_next_ext(cdf_nbuf_t buf)
{
	return __cdf_nbuf_next_ext(buf);
}

/**
 * cdf_nbuf_append_ext_list() - link list of packet extensions to the head
 *				segment
 * @head_buf: Network buf holding head segment (single)
 * @ext_list: Network buf list holding linked extensions to the head
 * @ext_len: Total length of all buffers in the extension list
 *
 * This function is used to link up a list of packet extensions (seg1, 2,
 * ...) to the nbuf holding the head segment (seg0)
 *
 * Return: none
 */
static inline void
cdf_nbuf_append_ext_list(cdf_nbuf_t head_buf, cdf_nbuf_t ext_list,
			 cdf_size_t ext_len)
{
	__cdf_nbuf_append_ext_list(head_buf, ext_list, ext_len);
}

/**
 * cdf_nbuf_get_tx_cksum() - gets the tx checksum offload demand
 * @buf: Network buffer
 *
 * Return: cdf_nbuf_tx_cksum_t checksum offload demand for the frame
 */
static inline cdf_nbuf_tx_cksum_t cdf_nbuf_get_tx_cksum(cdf_nbuf_t buf)
{
	return __cdf_nbuf_get_tx_cksum(buf);
}

/**
 * cdf_nbuf_set_rx_cksum() - drivers that support hw checksumming use this to
 *			     indicate checksum info to the stack.
 * @buf: Network buffer
 * @cksum: Checksum
 *
 * Return: none
 */
static inline void
cdf_nbuf_set_rx_cksum(cdf_nbuf_t buf, cdf_nbuf_rx_cksum_t *cksum)
{
	__cdf_nbuf_set_rx_cksum(buf, cksum);
}

/**
 * cdf_nbuf_get_tid() - this function extracts the TID value from nbuf
 * @buf: Network buffer
 *
 * Return: TID value
 */
static inline uint8_t cdf_nbuf_get_tid(cdf_nbuf_t buf)
{
	return __cdf_nbuf_get_tid(buf);
}

/**
 * cdf_nbuf_set_tid() - this function sets the TID value in nbuf
 * @buf: Network buffer
 * @tid: TID value
 *
 * Return: none
 */
static inline void cdf_nbuf_set_tid(cdf_nbuf_t buf, uint8_t tid)
{
	__cdf_nbuf_set_tid(buf, tid);
}

/**
 * cdf_nbuf_get_exemption_type() - this function extracts the exemption type
 *				   from nbuf
 * @buf: Network buffer
 *
 * Return: Exemption type
 */
static inline uint8_t cdf_nbuf_get_exemption_type(cdf_nbuf_t buf)
{
	return __cdf_nbuf_get_exemption_type(buf);
}

/**
 * cdf_nbuf_set_protocol() - this function peeks data into the buffer at given
 *			     offset
 * @buf: Network buffer
 * @proto: Protocol
 *
 * Return: none
 */
static inline void cdf_nbuf_set_protocol(cdf_nbuf_t buf, uint16_t proto)
{
	__cdf_nbuf_set_protocol(buf, proto);
}

/**
 * cdf_nbuf_trace_get_proto_type() - this function return packet proto type
 * @buf: Network buffer
 *
 * Return: Packet protocol type
 */
static inline uint8_t cdf_nbuf_trace_get_proto_type(cdf_nbuf_t buf)
{
	return __cdf_nbuf_trace_get_proto_type(buf);
}

#ifdef QCA_PKT_PROTO_TRACE
/**
 * cdf_nbuf_trace_set_proto_type() - this function updates packet proto type
 * @buf: Network buffer
 * @proto_type: Protocol type
 *
 * Return: none
 */
static inline void
cdf_nbuf_trace_set_proto_type(cdf_nbuf_t buf, uint8_t proto_type)
{
	__cdf_nbuf_trace_set_proto_type(buf, proto_type);
}
#else
#define cdf_nbuf_trace_set_proto_type(buf, proto_type) /*NO OP*/
#endif

/**
 * cdf_nbuf_reg_trace_cb() - this function registers protocol trace callback
 * @cb_func_ptr: Callback pointer
 *
 * Return: none
 */
static inline void cdf_nbuf_reg_trace_cb(cdf_nbuf_trace_update_t cb_func_ptr)
{
	__cdf_nbuf_reg_trace_cb(cb_func_ptr);
}

/**
 * cdf_nbuf_trace_update() - this function updates protocol event
 * @buf: Network buffer
 * @event_string: Event string pointer
 *
 * Return: none
 */
static inline void cdf_nbuf_trace_update(cdf_nbuf_t buf, char *event_string)
{
	__cdf_nbuf_trace_update(buf, event_string);
}

/**
 * cdf_nbuf_set_tx_parallel_dnload_frm() - set tx parallel download
 * @buf: Network buffer
 * @candi: Candidate of parallel download frame
 *
 * This function stores a flag specifying this TX frame is suitable for
 * downloading though a 2nd TX data pipe that is used for short frames for
 * protocols that can accept out-of-order delivery.
 *
 * Return: none
 */
static inline void
cdf_nbuf_set_tx_parallel_dnload_frm(cdf_nbuf_t buf, uint8_t candi)
{
	__cdf_nbuf_set_tx_htt2_frm(buf, candi);
}

/**
 * cdf_nbuf_get_tx_parallel_dnload_frm() - get tx parallel download
 * @buf: Network buffer
 *
 * This function return whether this TX frame is allow to download though a 2nd
 * TX data pipe or not.
 *
 * Return: none
 */
static inline uint8_t cdf_nbuf_get_tx_parallel_dnload_frm(cdf_nbuf_t buf)
{
	return __cdf_nbuf_get_tx_htt2_frm(buf);
}

/**
 * cdf_invalidate_range() - invalidate the virtual address range specified by
 *			    start and end addresses.
 * Note: This does not write back the cache entries.
 *
 * Return: none
 */

#ifdef MSM_PLATFORM
static inline void cdf_invalidate_range(void *start, void *end)
{
	dmac_inv_range(start, end);
}
#else
static inline void cdf_invalidate_range(void *start, void *end)
{
}
#endif

#if defined(FEATURE_TSO)
/**
 * cdf_nbuf_reset_num_frags() - resets the number of frags to 0 (valid range: 0..1)
 * @buf: Network buffer
 *
 * Return: Number of fragments
 */
static inline int cdf_nbuf_reset_num_frags(cdf_nbuf_t buf)
{
	return __cdf_nbuf_reset_num_frags(buf);
}

/**
 * cdf_nbuf_is_tso() - is the network buffer a jumbo packet?
 * @buf: Network buffer
 *
 * Return: true - jumbo packet false - not a jumbo packet
 */
static inline bool cdf_nbuf_is_tso(cdf_nbuf_t nbuf)
{
	return __cdf_nbuf_is_tso(nbuf);
}

/**
 * cdf_nbuf_get_tso_info() - function to divide a jumbo TSO
 * network buffer into segments
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
static inline uint32_t cdf_nbuf_get_tso_info(cdf_device_t osdev,
		 cdf_nbuf_t nbuf, struct cdf_tso_info_t *tso_info)
{
	return __cdf_nbuf_get_tso_info(osdev, nbuf, tso_info);
}

/**
 * cdf_nbuf_get_tso_num_seg() - function to calculate the number
 * of TCP segments within the TSO jumbo packet
 * @nbuf:   TSO jumbo network buffer to be segmented
 *
 * This function calculates the number of TCP segments that the
   network buffer can be divided into.
 *
 * Return: number of TCP segments
 */
static inline uint32_t cdf_nbuf_get_tso_num_seg(cdf_nbuf_t nbuf)
{
	return __cdf_nbuf_get_tso_num_seg(nbuf);
}

/**
 * cdf_nbuf_inc_users() - function to increment the number of
 * users referencing this network buffer
 *
 * @nbuf:   network buffer
 *
 * This function increments the number of users referencing this
 * network buffer
 *
 * Return: the network buffer
 */
static inline void cdf_nbuf_inc_users(cdf_nbuf_t nbuf)
{
	__cdf_nbuf_inc_users(nbuf);
}
#endif /*TSO*/

/**
 * cdf_nbuf_data_attr_get() - Get data_attr field from cvg_nbuf_cb
 *
 * @nbuf: Network buffer (skb on linux)
 *
 * This function returns the values of data_attr field
 * in struct cvg_nbuf_cb{}, to which skb->cb is typecast.
 * This value is actually the value programmed in CE descriptor.
 *
 * Return: Value of data_attr
 */
static inline
uint32_t cdf_nbuf_data_attr_get(cdf_nbuf_t buf)
{
	return __cdf_nbuf_data_attr_get(buf);
}

/**
 * cdf_nbuf_data_attr_set() - Sets data_attr field in cvg_nbuf_cb
 *
 * @nbuf: Network buffer (skb on linux)
 * @data_attr: Value to be stored cvg_nbuf_cb->data_attr
 *
 * This function stores the value to be programmed in CE
 * descriptor as part skb->cb which is typecast to struct cvg_nbuf_cb{}
 *
 * Return: void
 */
static inline
void cdf_nbuf_data_attr_set(cdf_nbuf_t buf, uint32_t data_attr)
{
	__cdf_nbuf_data_attr_set(buf, data_attr);
}

/**
 * cdf_nbuf_tx_info_get() - Parse skb and get Tx metadata
 *
 * @nbuf: Network buffer (skb on linux)
 *
 * This function parses the payload to figure out relevant
 * Tx meta-data e.g. whether to enable tx_classify bit
 * in CE.
 *
 * Return:	void
 */
#define cdf_nbuf_tx_info_get __cdf_nbuf_tx_info_get

void cdf_nbuf_set_state(cdf_nbuf_t nbuf, uint8_t current_state);
void cdf_nbuf_tx_desc_count_display(void);
void cdf_nbuf_tx_desc_count_clear(void);

#endif
