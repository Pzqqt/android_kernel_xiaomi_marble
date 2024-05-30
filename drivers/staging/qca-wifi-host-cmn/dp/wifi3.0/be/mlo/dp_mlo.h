/*
 * Copyright (c) 2021 Qualcomm Innovation Center, Inc. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */
#ifndef __DP_MLO_H
#define __DP_MLO_H

#include <dp_types.h>
#include <dp_peer.h>

/* Max number of chips that can participate in MLO */
#define DP_MAX_MLO_CHIPS WLAN_MAX_MLO_CHIPS

/* Max number of peers supported */
#define DP_MAX_MLO_PEER 512
/*
 * dp_mlo_ctxt
 *
 * @ctrl_ctxt: opaque handle of cp mlo mgr
 * @ml_soc_list: list of socs which are mlo enabled. This also maintains
 *               mlo_chip_id to dp_soc mapping
 * @ml_soc_list_lock: lock to protect ml_soc_list
 * @mld_peer_hash: peer hash table for ML peers
 *           Associated peer with this MAC address)
 * @mld_peer_hash_lock: lock to protect mld_peer_hash
 */
struct dp_mlo_ctxt {
	struct cdp_ctrl_mlo_mgr *ctrl_ctxt;
	struct dp_soc *ml_soc_list[DP_MAX_MLO_CHIPS];
	qdf_spinlock_t ml_soc_list_lock;
	struct {
		uint32_t mask;
		uint32_t idx_bits;

		TAILQ_HEAD(, dp_peer) * bins;
	} mld_peer_hash;

	qdf_spinlock_t mld_peer_hash_lock;
};

/**
 * dp_mlo_ctx_to_cdp() - typecast dp mlo context to CDP context
 * @mlo_ctxt: DP MLO context
 *
 * Return: struct cdp_mlo_ctxt pointer
 */
static inline
struct cdp_mlo_ctxt *dp_mlo_ctx_to_cdp(struct dp_mlo_ctxt *mlo_ctxt)
{
	return (struct cdp_mlo_ctxt *)mlo_ctxt;
}

/**
 * cdp_mlo_ctx_to_dp() - typecast cdp_soc_t to
 * dp soc handle
 * @psoc: CDP psoc handle
 *
 * Return: struct dp_soc pointer
 */
static inline
struct dp_mlo_ctxt *cdp_mlo_ctx_to_dp(struct cdp_mlo_ctxt *mlo_ctxt)
{
	return (struct dp_mlo_ctxt *)mlo_ctxt;
}

/**
 * dp_soc_mlo_fill_params() - update SOC mlo params
 * @soc: DP soc
 * @params: soc attach params
 *
 * Return: struct dp_soc pointer
 */
void dp_soc_mlo_fill_params(struct dp_soc *soc,
			    struct cdp_soc_attach_params *params);

/**
 * dp_pdev_mlo_fill_params() - update PDEV mlo params
 * @pdev: DP PDEV
 * @params: PDEV attach params
 *
 * Return: struct dp_soc pointer
 */
void dp_pdev_mlo_fill_params(struct dp_pdev *pdev,
			     struct cdp_pdev_attach_params *params);
#endif /* __DP_MLO_H */
