/*
 * Copyright (c) 2015-2017 The Linux Foundation. All rights reserved.
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

/**
 * qdf_lro_info_s - LRO information
 * @iph: IP header
 * @tcph: TCP header
 */
struct qdf_lro_info {
	uint8_t *iph;
	uint8_t *tcph;
};

#if defined(FEATURE_LRO)

/**
 * qdf_lro_init() - LRO initialization function
 *
 * Return: LRO context
 */
qdf_lro_ctx_t qdf_lro_init(void);

/**
 * qdf_lro_deinit() - LRO deinitialization function
 * @lro_ctx: LRO context
 *
 * Return: nothing
 */
void qdf_lro_deinit(qdf_lro_ctx_t lro_ctx);

/**
 *  qdf_lro_get_info() - Update the LRO information
 *
 * @lro_ctx: LRO context
 * @nbuf: network buffer
 * @info: LRO related information passed in by the caller
 * @plro_desc: lro information returned as output
 *
 * Look-up the LRO descriptor based on the LRO information and
 * the network buffer provided. Update the skb cb with the
 * descriptor found
 *
 * Return: true: LRO eligible false: LRO ineligible
 */
bool qdf_lro_get_info(qdf_lro_ctx_t lro_ctx, qdf_nbuf_t nbuf,
						 struct qdf_lro_info *info,
						 void **plro_desc);

/**
 * qdf_lro_flush_pkt() - function to flush the LRO flow
 * @info: LRO related information passed by the caller
 * @lro_ctx: LRO context
 *
 * Flush all the packets aggregated in the LRO manager for the
 * flow indicated by the TCP and IP header
 *
 * Return: none
 */
void qdf_lro_flush_pkt(qdf_lro_ctx_t lro_ctx,
	 struct qdf_lro_info *info);

/**
 * qdf_lro_flush() - LRO flush API
 * @lro_ctx: LRO context
 *
 * Flush all the packets aggregated in the LRO manager for all
 * the flows
 *
 * Return: none
 */
void qdf_lro_flush(qdf_lro_ctx_t lro_ctx);

/**
 * qdf_lro_desc_free() - Free the LRO descriptor
 * @desc: LRO descriptor
 * @lro_ctx: LRO context
 *
 * Return the LRO descriptor to the free pool
 *
 * Return: none
 */
void qdf_lro_desc_free(qdf_lro_ctx_t lro_ctx, void *desc);

#else

static inline qdf_lro_ctx_t qdf_lro_init(void)
{
	return NULL;
}

static inline void qdf_lro_deinit(qdf_lro_ctx_t lro_ctx)
{
}

static inline void qdf_lro_flush(qdf_lro_ctx_t lro_ctx)
{
}
#endif /* FEATURE_LRO */
#endif
