/*
 * Copyright (c) 2014-2021 The Linux Foundation. All rights reserved.
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

/**
 * DOC: i_qdf_nbuf_m.h
 *
 * This file provides platform specific nbuf API's.
 * Included by i_qdf_nbuf.h and should not be included
 * directly from other files.
 */

#ifndef _I_QDF_NBUF_M_H
#define _I_QDF_NBUF_M_H

#define QDF_NBUF_CB_RX_TCP_SEQ_NUM(skb) \
	(((struct qdf_nbuf_cb *)((skb)->cb))->u.rx.dev.priv_cb_m.tcp_seq_num)
#define QDF_NBUF_CB_RX_TCP_ACK_NUM(skb) \
	(((struct qdf_nbuf_cb *)((skb)->cb))->u.rx.dev.priv_cb_m.tcp_ack_num)
#define QDF_NBUF_CB_RX_LRO_CTX(skb) \
	(((struct qdf_nbuf_cb *)((skb)->cb))->u.rx.dev.priv_cb_m.lro_ctx)

#define QDF_NBUF_CB_TX_IPA_OWNED(skb) \
	(((struct qdf_nbuf_cb *)((skb)->cb))->u.tx.dev.priv_cb_m.ipa.owned)
#define QDF_NBUF_CB_TX_IPA_PRIV(skb) \
	(((struct qdf_nbuf_cb *)((skb)->cb))->u.tx.dev.priv_cb_m.ipa.priv)
#define QDF_NBUF_CB_TX_DESC_ID(skb)\
	(((struct qdf_nbuf_cb *)((skb)->cb))->u.tx.dev.priv_cb_m.desc_id)
#define QDF_NBUF_CB_MGMT_TXRX_DESC_ID(skb)\
	(((struct qdf_nbuf_cb *)((skb)->cb))->u.tx.dev.priv_cb_m.mgmt_desc_id)
#define QDF_NBUF_CB_TX_DMA_BI_MAP(skb) \
	(((struct qdf_nbuf_cb *)((skb)->cb))->u.tx.dev.priv_cb_m. \
	dma_option.bi_map)
#define QDF_NBUF_CB_TX_EXTRA_FRAG_FLAGS_NOTIFY_COMP(skb) \
	(((struct qdf_nbuf_cb *)((skb)->cb))->u.tx.dev.priv_cb_m. \
	flag_notify_comp)

#define QDF_NBUF_CB_RX_PEER_ID(skb) \
	(((struct qdf_nbuf_cb *)((skb)->cb))->u.rx.dev.priv_cb_m.dp. \
	wifi3.peer_id)

#define QDF_NBUF_CB_RX_PKT_LEN(skb) \
	(((struct qdf_nbuf_cb *)((skb)->cb))->u.rx.dev.priv_cb_m.dp. \
	wifi3.msdu_len)

#define QDF_NBUF_CB_RX_MAP_IDX(skb) \
	(((struct qdf_nbuf_cb *)((skb)->cb))->u.rx.dev.priv_cb_m.dp. \
	wifi2.map_index)

#define  QDF_NBUF_CB_RX_PEER_CACHED_FRM(skb) \
	 (((struct qdf_nbuf_cb *)((skb)->cb))->u.rx.dev.priv_cb_m. \
	 peer_cached_buf_frm)

#define  QDF_NBUF_CB_RX_FLUSH_IND(skb) \
	 (((struct qdf_nbuf_cb *)((skb)->cb))->u.rx.dev.priv_cb_m.flush_ind)

#define  QDF_NBUF_CB_RX_PACKET_BUFF_POOL(skb) \
	 (((struct qdf_nbuf_cb *)((skb)->cb))->u.rx.dev.priv_cb_m. \
	 packet_buf_pool)

#define  QDF_NBUF_CB_RX_PACKET_L3_HDR_PAD(skb) \
	 (((struct qdf_nbuf_cb *)((skb)->cb))->u.rx.dev.priv_cb_m. \
	 l3_hdr_pad)

#define  QDF_NBUF_CB_RX_PACKET_EXC_FRAME(skb) \
	 (((struct qdf_nbuf_cb *)((skb)->cb))->u.rx.dev.priv_cb_m. \
	 exc_frm)

#define  QDF_NBUF_CB_RX_PACKET_IPA_SMMU_MAP(skb) \
	 (((struct qdf_nbuf_cb *)((skb)->cb))->u.rx.dev.priv_cb_m. \
	 ipa_smmu_map)

#define  QDF_NBUF_CB_RX_PACKET_REO_DEST_IND_OR_SW_EXCPT(skb) \
	(((struct qdf_nbuf_cb *)((skb)->cb))->u.rx.dev.priv_cb_m. \
	reo_dest_ind_or_sw_excpt)

#define __qdf_nbuf_ipa_owned_get(skb) \
	QDF_NBUF_CB_TX_IPA_OWNED(skb)

#define __qdf_nbuf_ipa_owned_set(skb) \
	(QDF_NBUF_CB_TX_IPA_OWNED(skb) = 1)

#define __qdf_nbuf_ipa_owned_clear(skb) \
	(QDF_NBUF_CB_TX_IPA_OWNED(skb) = 0)

#define __qdf_nbuf_ipa_priv_get(skb)	\
	QDF_NBUF_CB_TX_IPA_PRIV(skb)

#define __qdf_nbuf_ipa_priv_set(skb, priv) \
	(QDF_NBUF_CB_TX_IPA_PRIV(skb) = (priv))

/**
 * qdf_nbuf_cb_update_vdev_id() - update vdev id in skb cb
 * @skb: skb pointer whose cb is updated with vdev id information
 * @vdev_id: vdev id to be updated in cb
 *
 * Return: void
 */
static inline void
qdf_nbuf_cb_update_vdev_id(struct sk_buff *skb, uint8_t vdev_id)
{
	QDF_NBUF_CB_RX_VDEV_ID(skb) = vdev_id;
}

void __qdf_nbuf_init_replenish_timer(void);
void __qdf_nbuf_deinit_replenish_timer(void);

/**
 * __qdf_nbuf_push_head() - Push data in the front
 * @skb: Pointer to network buffer
 * @size: size to be pushed
 *
 * Return: New data pointer of this buf after data has been pushed,
 *         or NULL if there is not enough room in this buf.
 */
static inline uint8_t *__qdf_nbuf_push_head(struct sk_buff *skb, size_t size)
{
	if (QDF_NBUF_CB_PADDR(skb))
		QDF_NBUF_CB_PADDR(skb) -= size;

	return skb_push(skb, size);
}


/**
 * __qdf_nbuf_pull_head() - pull data out from the front
 * @skb: Pointer to network buffer
 * @size: size to be popped
 *
 * Return: New data pointer of this buf after data has been popped,
 *	   or NULL if there is not sufficient data to pull.
 */
static inline uint8_t *__qdf_nbuf_pull_head(struct sk_buff *skb, size_t size)
{
	if (QDF_NBUF_CB_PADDR(skb))
		QDF_NBUF_CB_PADDR(skb) += size;

	return skb_pull(skb, size);
}

/**
 * qdf_nbuf_is_intra_bss() - get intra bss bit
 * @buf: Network buffer
 *
 * Return: integer value - 0/1
 */
static inline int qdf_nbuf_is_intra_bss(struct sk_buff *buf)
{
	return 0;
}

/**
 * qdf_nbuf_set_intra_bss() - set intra bss bit
 * @buf: Network buffer
 * @val: 0/1
 *
 * Return: void
 */
static inline void qdf_nbuf_set_intra_bss(struct sk_buff *buf, uint8_t val)
{
}

/**
 * qdf_nbuf_init_replenish_timer - Initialize the alloc replenish timer
 *
 * This function initializes the nbuf alloc fail replenish timer.
 *
 * Return: void
 */
static inline void
qdf_nbuf_init_replenish_timer(void)
{
	__qdf_nbuf_init_replenish_timer();
}

/**
 * qdf_nbuf_deinit_replenish_timer - Deinitialize the alloc replenish timer
 *
 * This function deinitializes the nbuf alloc fail replenish timer.
 *
 * Return: void
 */
static inline void
qdf_nbuf_deinit_replenish_timer(void)
{
	__qdf_nbuf_deinit_replenish_timer();
}

static inline void
__qdf_nbuf_dma_inv_range(const void *buf_start, const void *buf_end) {}

#endif /*_I_QDF_NBUF_M_H */
