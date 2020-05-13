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
 * DOC: i_qdf_nbuf_api_m.h
 *
 * Platform specific qdf_nbuf_public network buffer API
 * This file defines the network buffer abstraction.
 * Included by qdf_nbuf.h and should not be included
 * directly from other files.
 */

#ifndef _QDF_NBUF_M_H
#define _QDF_NBUF_M_H

static inline int qdf_nbuf_ipa_owned_get(qdf_nbuf_t buf)
{
	return __qdf_nbuf_ipa_owned_get(buf);
}

static inline void qdf_nbuf_ipa_owned_set(qdf_nbuf_t buf)
{
	__qdf_nbuf_ipa_owned_set(buf);
}

static inline void qdf_nbuf_ipa_owned_clear(qdf_nbuf_t buf)
{
	__qdf_nbuf_ipa_owned_clear(buf);
}

static inline int qdf_nbuf_ipa_priv_get(qdf_nbuf_t buf)
{
	return __qdf_nbuf_ipa_priv_get(buf);
}

static inline void qdf_nbuf_ipa_priv_set(qdf_nbuf_t buf, uint32_t priv)
{

	QDF_BUG(!(priv & QDF_NBUF_IPA_CHECK_MASK));
	__qdf_nbuf_ipa_priv_set(buf, priv);
}

/**
 * qdf_nbuf_set_rx_protocol_tag()
 * @buf: Network buffer
 * @val: Value to be set in the nbuf
 * Return: None
 */
static inline void qdf_nbuf_set_rx_protocol_tag(qdf_nbuf_t buf, uint16_t val)
{
}

/**
 * qdf_nbuf_get_rx_protocol_tag()
 * @buf: Network buffer
 * Return: Value of rx protocol tag, here 0
 */
static inline uint16_t qdf_nbuf_get_rx_protocol_tag(qdf_nbuf_t buf)
{
	return 0;
}

/**
 * qdf_nbuf_set_rx_flow_tag() - set given value in flow tag field
 * of buf(skb->cb)
 * @buf: Network buffer
 * @val: Rx Flow Tag to be set in the nbuf
 * Return: None
 */
static inline void qdf_nbuf_set_rx_flow_tag(qdf_nbuf_t buf, uint16_t val)
{
}

/**
 * qdf_nbuf_get_rx_flow_tag() - Get the value of flow_tag
 * field of buf(skb->cb)
 * @buf: Network buffer
 * Return: Value of rx flow tag, here 0
 */
static inline uint16_t qdf_nbuf_get_rx_flow_tag(qdf_nbuf_t buf)
{
	return 0;
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
	QDF_NBUF_CB_RX_PACKET_EXC_FRAME(buf) = value;
}

/**
 * qdf_nbuf_is_exc_frame() - check exception frame flag bit
 * @buf: Network buffer to get exception flag
 *
 * Return: 0 or 1
 */
static inline uint8_t qdf_nbuf_is_exc_frame(qdf_nbuf_t buf)
{
	return QDF_NBUF_CB_RX_PACKET_EXC_FRAME(buf);
}
#endif /* _QDF_NBUF_M_H */
