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
#include <dp_htt.h>
#include <dp_internal.h>

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

	if (dp_mlo_peer_find_hash_attach_be
			(mlo_ctxt, DP_MAX_MLO_PEER) != QDF_STATUS_SUCCESS) {
		dp_err("Failed to allocate peer hash");
		qdf_mem_free(mlo_ctxt);
		return NULL;
	}

	qdf_spinlock_create(&mlo_ctxt->ml_soc_list_lock);
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
void dp_mlo_ctxt_detach_wifi3(struct cdp_mlo_ctxt *cdp_ml_ctxt)
{
	struct dp_mlo_ctxt *mlo_ctxt = cdp_mlo_ctx_to_dp(cdp_ml_ctxt);

	if (!cdp_ml_ctxt)
		return;

	qdf_spinlock_destroy(&mlo_ctxt->ml_soc_list_lock);
	dp_mlo_peer_find_hash_detach_be(mlo_ctxt);
	qdf_mem_free(mlo_ctxt);
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
	soc = ml_ctxt->ml_soc_list[chip_id];

	if (!soc) {
		qdf_spin_unlock_bh(&ml_ctxt->ml_soc_list_lock);
		return NULL;
	}

	qdf_atomic_inc(&soc->ref_count);
	qdf_spin_unlock_bh(&ml_ctxt->ml_soc_list_lock);

	return soc;
}

static void dp_mlo_soc_setup(struct cdp_soc_t *soc_hdl,
			     struct cdp_mlo_ctxt *cdp_ml_ctxt)
{
	struct dp_soc *soc = cdp_soc_t_to_dp_soc(soc_hdl);
	struct dp_mlo_ctxt *mlo_ctxt = cdp_mlo_ctx_to_dp(cdp_ml_ctxt);
	struct dp_soc_be *be_soc = dp_get_be_soc_from_dp_soc(soc);

	if (!cdp_ml_ctxt)
		return;

	dp_mlo_set_soc_by_chip_id(mlo_ctxt, soc, be_soc->mlo_chip_id);
}

static void dp_mlo_soc_teardown(struct cdp_soc_t *soc_hdl,
				struct cdp_mlo_ctxt *cdp_ml_ctxt)
{
	struct dp_soc *soc = cdp_soc_t_to_dp_soc(soc_hdl);
	struct dp_mlo_ctxt *mlo_ctxt = cdp_mlo_ctx_to_dp(cdp_ml_ctxt);
	struct dp_soc_be *be_soc = dp_get_be_soc_from_dp_soc(soc);

	if (!cdp_ml_ctxt)
		return;

	dp_mlo_set_soc_by_chip_id(mlo_ctxt, NULL, be_soc->mlo_chip_id);
}

static struct cdp_mlo_ops dp_mlo_ops = {
	.mlo_soc_setup = dp_mlo_soc_setup,
	.mlo_soc_teardown = dp_mlo_soc_teardown,
};

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
	soc->cdp_soc.ops->mlo_ops = &dp_mlo_ops;
}

void dp_pdev_mlo_fill_params(struct dp_pdev *pdev,
			     struct cdp_pdev_attach_params *params)
{
	struct dp_soc_be *be_soc = dp_get_be_soc_from_dp_soc(pdev->soc);
	struct dp_pdev_be *be_pdev = dp_get_be_pdev_from_dp_pdev(pdev);

	if (!be_soc->mlo_enabled) {
		dp_info("MLO not enabled on SOC");
		return;
	}

	be_pdev->mlo_link_id = params->mlo_link_id;
}

void dp_mlo_partner_chips_map(struct dp_soc *soc,
			      struct dp_peer *peer,
			      uint16_t peer_id)
{
	struct dp_soc_be *be_soc = dp_get_be_soc_from_dp_soc(soc);
	struct dp_mlo_ctxt *mlo_ctxt = be_soc->ml_ctxt;
	bool is_ml_peer_id = peer_id & HTT_RX_PEER_META_DATA_V1_ML_PEER_VALID_S;
	uint8_t chip_id;
	struct dp_soc *temp_soc;

	if (!mlo_ctxt)
		return;

	/* for non ML peer dont map on partner chips*/
	if (!is_ml_peer_id)
		return;

	qdf_spin_lock_bh(&mlo_ctxt->ml_soc_list_lock);
	for (chip_id = 0; chip_id < DP_MAX_MLO_CHIPS; chip_id++) {
		temp_soc = mlo_ctxt->ml_soc_list[chip_id];

		if (!temp_soc)
			continue;

		/* skip if this is current soc */
		if (temp_soc == soc)
			continue;

		dp_peer_find_id_to_obj_add(temp_soc, peer, peer_id);
	}
	qdf_spin_unlock_bh(&mlo_ctxt->ml_soc_list_lock);
}

qdf_export_symbol(dp_mlo_partner_chips_map);

void dp_mlo_partner_chips_unmap(struct dp_soc *soc,
				uint16_t peer_id)
{
	struct dp_soc_be *be_soc = dp_get_be_soc_from_dp_soc(soc);
	struct dp_mlo_ctxt *mlo_ctxt = be_soc->ml_ctxt;
	bool is_ml_peer_id = peer_id & HTT_RX_PEER_META_DATA_V1_ML_PEER_VALID_S;
	uint8_t chip_id;
	struct dp_soc *temp_soc;

	if (!is_ml_peer_id)
		return;

	if (!mlo_ctxt)
		return;

	qdf_spin_lock_bh(&mlo_ctxt->ml_soc_list_lock);
	for (chip_id = 0; chip_id < DP_MAX_MLO_CHIPS; chip_id++) {
		temp_soc = mlo_ctxt->ml_soc_list[chip_id];

		if (!temp_soc)
			continue;

		/* skip if this is current soc */
		if (temp_soc == soc)
			continue;

		dp_peer_find_id_to_obj_remove(temp_soc, peer_id);
	}
	qdf_spin_unlock_bh(&mlo_ctxt->ml_soc_list_lock);
}

qdf_export_symbol(dp_mlo_partner_chips_unmap);

uint8_t dp_mlo_get_chip_id(struct dp_soc *soc)
{
	struct dp_soc_be *be_soc = dp_get_be_soc_from_dp_soc(soc);

	return be_soc->mlo_chip_id;
}

qdf_export_symbol(dp_mlo_get_chip_id);

struct dp_peer *
dp_link_peer_hash_find_by_chip_id(struct dp_soc *soc,
				  uint8_t *peer_mac_addr,
				  int mac_addr_is_aligned,
				  uint8_t vdev_id,
				  uint8_t chip_id,
				  enum dp_mod_id mod_id)
{
	struct dp_soc_be *be_soc = dp_get_be_soc_from_dp_soc(soc);
	struct dp_mlo_ctxt *mlo_ctxt = be_soc->ml_ctxt;
	struct dp_soc *link_peer_soc = NULL;
	struct dp_peer *peer = NULL;

	if (!mlo_ctxt)
		return NULL;

	link_peer_soc = dp_mlo_get_soc_ref_by_chip_id(mlo_ctxt, chip_id);

	if (!link_peer_soc)
		return NULL;

	peer = dp_peer_find_hash_find(link_peer_soc, peer_mac_addr,
				      mac_addr_is_aligned, vdev_id,
				      mod_id);
	qdf_atomic_dec(&link_peer_soc->ref_count);
	return peer;
}

qdf_export_symbol(dp_link_peer_hash_find_by_chip_id);

struct dp_soc *
dp_rx_replensih_soc_get(struct dp_soc *soc, uint8_t reo_ring_num)
{
	struct dp_soc_be *be_soc = dp_get_be_soc_from_dp_soc(soc);
	struct dp_mlo_ctxt *mlo_ctxt = be_soc->ml_ctxt;
	uint8_t chip_id;
	uint8_t rx_ring_mask;

	if (!be_soc->mlo_enabled || !mlo_ctxt)
		return soc;

	for (chip_id = 0; chip_id < WLAN_MAX_MLO_CHIPS; chip_id++) {
		rx_ring_mask =
			wlan_cfg_mlo_rx_ring_map_get_by_chip_id
					(soc->wlan_cfg_ctx, chip_id);

		if (rx_ring_mask & (1 << reo_ring_num))
			return dp_mlo_get_soc_ref_by_chip_id(mlo_ctxt, chip_id);
	}

	return soc;
}
