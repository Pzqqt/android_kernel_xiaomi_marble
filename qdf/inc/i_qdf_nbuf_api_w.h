/*
 * Copyright (c) 2014-2017,2019-2020 The Linux Foundation. All rights reserved.
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
 * DOC: i_qdf_nbuf_api_w.h
 *
 * Platform specific qdf_nbuf_public network buffer API
 * This file defines the network buffer abstraction.
 * Included by qdf_nbuf.h and should not be included
 * directly from other files.
 */

#ifndef _QDF_NBUF_W_H
#define _QDF_NBUF_W_H

static inline void *qdf_nbuf_get_tx_fctx(qdf_nbuf_t buf)
{
	return  __qdf_nbuf_get_tx_fctx(buf);
}

static inline void *qdf_nbuf_get_rx_fctx(qdf_nbuf_t buf)
{
	return  __qdf_nbuf_get_rx_fctx(buf);
}


static inline void
qdf_nbuf_set_tx_fctx_type(qdf_nbuf_t buf, void *ctx, uint8_t type)
{
	__qdf_nbuf_set_tx_fctx_type(buf, ctx, type);
}

static inline void
qdf_nbuf_set_rx_fctx_type(qdf_nbuf_t buf, void *ctx, uint8_t type)
{
	__qdf_nbuf_set_rx_fctx_type(buf, ctx, type);
}


static inline void *
qdf_nbuf_get_ext_cb(qdf_nbuf_t buf)
{
	return  __qdf_nbuf_get_ext_cb(buf);
}

static inline void
qdf_nbuf_set_ext_cb(qdf_nbuf_t buf, void *ref)
{
	__qdf_nbuf_set_ext_cb(buf, ref);
}

/**
 * qdf_nbuf_set_rx_protocol_tag() - set given value in protocol_tag
 * field of buf(skb->cb)
 * @buf: Network buffer
 * @val: Value to be set in the nbuf
 * Return: None
 */
static inline void qdf_nbuf_set_rx_protocol_tag(qdf_nbuf_t buf, uint16_t val)
{
	__qdf_nbuf_set_rx_protocol_tag(buf, val);
}

/**
 * qdf_nbuf_get_rx_protocol_tag() - Get the value of protocol_tag
 * field of buf(skb->cb)
 * @buf: Network buffer
 * Return: Value of Rx protocol tag in the nbuf
 */
static inline uint16_t qdf_nbuf_get_rx_protocol_tag(qdf_nbuf_t buf)
{
	return __qdf_nbuf_get_rx_protocol_tag(buf);
}

/**
 * qdf_nbuf_set_rx_flow_tag() - set given value in flow tag field
 * of buf(skb->cb)
 * @buf: Network buffer
 * @val: Value of Rx flow tag to be set in the nbuf
 * Return: None
 */
static inline void qdf_nbuf_set_rx_flow_tag(qdf_nbuf_t buf, uint16_t val)
{
	__qdf_nbuf_set_rx_flow_tag(buf, val);
}

/**
 * qdf_nbuf_get_rx_flow_tag() - Get the value of flow_tag
 * field of buf(skb->cb)
 * @buf: Network buffer
 * Return: Value of the Rx flow tag in the nbuf
 */
static inline uint16_t qdf_nbuf_get_rx_flow_tag(qdf_nbuf_t buf)
{
	return __qdf_nbuf_get_rx_flow_tag(buf);
}

/**
 * qdf_nbuf_set_exc_frame() - set exception frame flag
 * @buf: Network buffer whose cb is to set exception frame flag
 * @value: exception frame flag, value 0 or 1.
 *
 * Return: none
 */
static inline void qdf_nbuf_set_exc_frame(qdf_nbuf_t buf, uint8_t value)
{
}
#endif /* _QDF_NBUF_W_H */
