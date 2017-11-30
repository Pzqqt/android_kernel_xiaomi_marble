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

#endif /* _QDF_NBUF_W_H */
