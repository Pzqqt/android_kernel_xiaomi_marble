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
#include <wlan_utility.h>
#include <dp_internal.h>
#include <dp_htt.h>
#include <hal_be_api.h>
#include "dp_mlo.h"
#include <dp_be.h>

/*
 * dp_mlo_ctxt_attach_wifi3 () – Attach DP MLO context
 *
 * Return: DP MLO context handle on success, NULL on failure
 */
struct cdp_mlo_ctxt *
dp_mlo_ctxt_attach_wifi3(struct cdp_ctrl_mlo_mgr *ctrl_ctxt)
{
	struct dp_mlo_ctxt *mlo_ctxt =
		qdf_mem_malloc(sizeof(struct dp_mlo_ctxt));

	if (!mlo_ctxt) {
		dp_err("Failed to allocate DP MLO Context");
		return NULL;
	}

	mlo_ctxt->ctrl_ctxt = ctrl_ctxt;

	return dp_mlo_ctx_to_cdp(mlo_ctxt);
}

qdf_export_symbol(dp_mlo_ctxt_attach_wifi3);

/*
 * dp_mlo_ctxt_detach_wifi3 () – Detach DP MLO context
 *
 * @ml_ctxt: pointer to DP MLO context
 *
 * Return: void
 */
void dp_mlo_ctxt_detach_wifi3(struct cdp_mlo_ctxt *ml_ctxt)
{
	qdf_mem_free(ml_ctxt);
}

qdf_export_symbol(dp_mlo_ctxt_detach_wifi3);

/*
 * dp_mlo_set_soc_by_chip_id() – Add DP soc to ML context soc list
 *
 * @ml_ctxt: DP ML context handle
 * @soc: DP soc handle
 * @chip_id: MLO chip id
 *
 * Return: void
 */
void dp_mlo_set_soc_by_chip_id(struct dp_mlo_ctxt *ml_ctxt,
			       struct dp_soc *soc,
			       uint8_t chip_id)
{
	qdf_spin_lock_bh(&ml_ctxt->ml_soc_list_lock);
	ml_ctxt->ml_soc_list[chip_id] = soc;
	qdf_spin_unlock_bh(&ml_ctxt->ml_soc_list_lock);
}

/*
 * dp_mlo_get_soc_ref_by_chip_id() – Get DP soc from DP ML context.
 * This API will increment a reference count for DP soc. Caller has
 * to take care for decrementing refcount.
 *
 * @ml_ctxt: DP ML context handle
 * @chip_id: MLO chip id
 *
 * Return: dp_soc
 */
struct dp_soc*
dp_mlo_get_soc_ref_by_chip_id(struct dp_mlo_ctxt *ml_ctxt,
			      uint8_t chip_id)
{
	struct dp_soc *soc = NULL;

	qdf_spin_lock_bh(&ml_ctxt->ml_soc_list_lock);
	qdf_atomic_inc(&soc->ref_count);
	soc = ml_ctxt->ml_soc_list[chip_id];
	qdf_spin_unlock_bh(&ml_ctxt->ml_soc_list_lock);

	return soc;
}

void dp_soc_mlo_fill_params(struct dp_soc *soc,
			    struct cdp_soc_attach_params *params)
{
	struct dp_soc_be *be_soc = dp_get_be_soc_from_dp_soc(soc);

	if (!params->mlo_enabled) {
		dp_warn("MLO not enabled on SOC");
		return;
	}

	be_soc->mlo_chip_id = params->mlo_chip_id;
	be_soc->ml_ctxt = cdp_mlo_ctx_to_dp(params->ml_context);
	be_soc->mlo_enabled = 1;
}
