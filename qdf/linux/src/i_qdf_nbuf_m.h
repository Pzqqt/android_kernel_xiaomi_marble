/*
 * Copyright (c) 2014-2017 The Linux Foundation. All rights reserved.
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

#endif /*_I_QDF_NBUF_M_H */
