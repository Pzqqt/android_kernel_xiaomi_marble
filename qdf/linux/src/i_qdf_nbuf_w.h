/*
 * Copyright (c) 2014-2019 The Linux Foundation. All rights reserved.
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
 * DOC: i_qdf_nbuf_w.h
 *
 * This file provides platform specific nbuf API's.
 * Included by i_qdf_nbuf.h and should not be included
 * directly from other files.
 */

#ifndef _I_QDF_NBUF_W_H
#define _I_QDF_NBUF_W_H

/* ext_cb accesor macros and internal API's */

#define QDF_NBUF_CB_EXT_CB(skb) \
	(((struct qdf_nbuf_cb *)((skb)->cb))->u.tx.dev.priv_cb_w.ext_cb_ptr)

#define __qdf_nbuf_set_ext_cb(skb, ref) \
	do { \
		QDF_NBUF_CB_EXT_CB((skb)) = (ref); \
	} while (0)

#define __qdf_nbuf_get_ext_cb(skb) \
	QDF_NBUF_CB_EXT_CB((skb))

/* fctx accesor macros and internal API's*/

#define QDF_NBUF_CB_RX_FCTX(skb) \
	(((struct qdf_nbuf_cb *)((skb)->cb))->u.rx.dev.priv_cb_w.fctx)

#define QDF_NBUF_CB_TX_FCTX(skb) \
	(((struct qdf_nbuf_cb *)((skb)->cb))->u.tx.dev.priv_cb_w.fctx)

#define QDF_NBUF_CB_RX_PEER_ID(skb) \
	(((struct qdf_nbuf_cb *)((skb)->cb))->u.rx.dev.priv_cb_w.peer_id)

#define QDF_NBUF_CB_RX_PKT_LEN(skb) \
	(((struct qdf_nbuf_cb *)((skb)->cb))->u.rx.dev.priv_cb_w.msdu_len)

#define __qdf_nbuf_set_rx_fctx_type(skb, ctx, type) \
	do { \
		QDF_NBUF_CB_RX_FCTX((skb)) = (ctx); \
		QDF_NBUF_CB_RX_FTYPE((skb)) = (type); \
	} while (0)

#define __qdf_nbuf_get_rx_fctx(skb) \
		 QDF_NBUF_CB_RX_FCTX((skb))

#define __qdf_nbuf_set_tx_fctx_type(skb, ctx, type) \
	do { \
		QDF_NBUF_CB_TX_FCTX((skb)) = (ctx); \
		QDF_NBUF_CB_TX_FTYPE((skb)) = (type); \
	} while (0)

#define __qdf_nbuf_get_tx_fctx(skb) \
		 QDF_NBUF_CB_TX_FCTX((skb))

#define QDF_NBUF_CB_RX_PROTOCOL_TAG(skb) \
		(((struct qdf_nbuf_cb *) \
		((skb)->cb))->u.rx.dev.priv_cb_w.protocol_tag)

#define __qdf_nbuf_set_rx_protocol_tag(skb, val) \
		((QDF_NBUF_CB_RX_PROTOCOL_TAG((skb))) = val)

#define __qdf_nbuf_get_rx_protocol_tag(skb) \
		(QDF_NBUF_CB_RX_PROTOCOL_TAG((skb)))

#define QDF_NBUF_CB_RX_FLOW_TAG(skb) \
		(((struct qdf_nbuf_cb *) \
		((skb)->cb))->u.rx.dev.priv_cb_w.flow_tag)

#define __qdf_nbuf_set_rx_flow_tag(skb, val) \
		((QDF_NBUF_CB_RX_FLOW_TAG((skb))) = val)

#define __qdf_nbuf_get_rx_flow_tag(skb) \
		(QDF_NBUF_CB_RX_FLOW_TAG((skb)))

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
	/* Does not apply to WIN */
}

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
	return skb_push(skb, size);
}

/**
 * __qdf_nbuf_pull_head() - pull data out from the front
 * @skb: Pointer to network buffer
 * @size: size to be popped
 *
 * Return: New data pointer of this buf after data has been popped,
 * or NULL if there is not sufficient data to pull.
 */
static inline uint8_t *__qdf_nbuf_pull_head(struct sk_buff *skb, size_t size)
{
	return skb_pull(skb, size);
}

static inline void qdf_nbuf_init_replenish_timer(void) {}
static inline void qdf_nbuf_deinit_replenish_timer(void) {}

#endif /*_I_QDF_NBUF_W_H */
