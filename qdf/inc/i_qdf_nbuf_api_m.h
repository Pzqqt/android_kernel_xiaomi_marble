/*
 * Copyright (c) 2014-2017,2019 The Linux Foundation. All rights reserved.
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
 * @val: Value to be set
 * Return: void
 */
static inline void qdf_nbuf_set_rx_protocol_tag(qdf_nbuf_t buf, uint32_t val)
{
}

/**
 * qdf_nbuf_get_rx_protocol_tag()
 * @buf: Network buffer
 * Return: void
 */
static inline int qdf_nbuf_get_rx_protocol_tag(qdf_nbuf_t buf)
{
	return 0;
}
#endif /* _QDF_NBUF_M_H */
