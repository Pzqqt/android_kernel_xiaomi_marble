/*
 * Copyright (c) 2015-2017 The Linux Foundation. All rights reserved.
 *
 * Previously licensed under the ISC license by Qualcomm Atheros, Inc.
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
 * DOC: Large Receive Offload API
 * This file defines the Large receive offload API.
 */
#ifndef _QDF_LRO_H
#define _QDF_LRO_H

#include <qdf_nbuf.h>
#include <i_qdf_lro.h>

/**
 * @qdf_nbuf_t - Platform indepedent LRO context abstraction
 */
typedef __qdf_lro_ctx_t qdf_lro_ctx_t;

#if defined(FEATURE_LRO)

qdf_lro_ctx_t qdf_lro_init(void);

void qdf_lro_deinit(qdf_lro_ctx_t lro_ctx);

bool qdf_lro_update_info(qdf_lro_ctx_t lro_ctx, qdf_nbuf_t nbuf);

void qdf_lro_flow_free(qdf_nbuf_t nbuf);

void qdf_lro_flush_pkt(struct iphdr *iph,
	 struct tcphdr *tcph, qdf_lro_ctx_t lro_ctx);

void qdf_lro_flush(qdf_lro_ctx_t lro_ctx);

#else

static inline qdf_lro_ctx_t qdf_lro_init(void)
{
	return NULL;
}

static inline void qdf_lro_deinit(qdf_lro_ctx_t lro_ctx)
{
}

static inline void qdf_lro_flush_pkt(struct iphdr *iph,
	 struct tcphdr *tcph, qdf_lro_ctx_t lro_ctx)
{
}

static inline void qdf_lro_flush(qdf_lro_ctx_t lro_ctx)
{
}

#endif /* FEATURE_LRO */
#endif
