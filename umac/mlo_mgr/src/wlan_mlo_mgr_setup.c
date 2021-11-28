/* Copyright (c) 2021 Qualcomm Innovation Center, Inc. All rights reserved.
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

/*
 * DOC: contains MLO manager ap related functionality
 */
#include "wlan_mlo_mgr_cmn.h"
#include "wlan_mlo_mgr_main.h"
#ifdef WLAN_MLO_MULTI_CHIP
#include "wlan_lmac_if_def.h"
#include <cdp_txrx_mlo.h>
#endif

#ifdef WLAN_MLO_MULTI_CHIP
void mlo_setup_update_total_socs(uint8_t tot_socs)
{
	struct mlo_mgr_context *mlo_ctx = wlan_objmgr_get_mlo_ctx();

	if (!mlo_ctx)
		return;

	mlo_ctx->setup_info.tot_socs = tot_socs;
}

qdf_export_symbol(mlo_setup_update_total_socs);

void mlo_setup_update_num_links(struct wlan_objmgr_psoc *psoc,
				uint8_t num_links)
{
	struct mlo_mgr_context *mlo_ctx = wlan_objmgr_get_mlo_ctx();

	if (!mlo_ctx)
		return;

	mlo_ctx->setup_info.tot_links += num_links;
}

qdf_export_symbol(mlo_setup_update_num_links);

void mlo_setup_update_soc_ready(struct wlan_objmgr_psoc *psoc)
{
	struct mlo_mgr_context *mlo_ctx = wlan_objmgr_get_mlo_ctx();
	uint8_t chip_idx;

	if (!mlo_ctx || !mlo_ctx->setup_info.tot_socs)
		return;

	chip_idx = mlo_ctx->setup_info.num_soc;
	qdf_assert_always(chip_idx < MAX_MLO_CHIPS);
	mlo_ctx->setup_info.soc_list[chip_idx] = psoc;
	mlo_ctx->setup_info.num_soc++;

	if (mlo_ctx->setup_info.num_soc != mlo_ctx->setup_info.tot_socs)
		return;

	for (chip_idx = 0; chip_idx < mlo_ctx->setup_info.num_soc;
			chip_idx++) {
		struct wlan_objmgr_psoc *tmp_soc =
			mlo_ctx->setup_info.soc_list[chip_idx];

		cdp_soc_mlo_soc_setup(wlan_psoc_get_dp_handle(tmp_soc),
				      mlo_ctx->dp_handle);
	}
}

qdf_export_symbol(mlo_setup_update_soc_ready);

void mlo_setup_link_ready(struct wlan_objmgr_pdev *pdev)
{
	struct mlo_mgr_context *mlo_ctx = wlan_objmgr_get_mlo_ctx();
	uint8_t link_idx;

	if (!mlo_ctx || !mlo_ctx->setup_info.tot_links)
		return;

	link_idx = mlo_ctx->setup_info.num_links;
	/* TODO: Get reference to PDEV */
	qdf_assert_always(link_idx < MAX_MLO_LINKS);
	mlo_ctx->setup_info.pdev_list[link_idx] = pdev;
	mlo_ctx->setup_info.state[link_idx] = MLO_LINK_SETUP_INIT;
	mlo_ctx->setup_info.num_links++;

	if (mlo_ctx->setup_info.num_links == mlo_ctx->setup_info.tot_links &&
	    mlo_ctx->setup_info.num_soc == mlo_ctx->setup_info.tot_socs) {
		struct wlan_objmgr_psoc *psoc;
		struct wlan_lmac_if_tx_ops *tx_ops;

		psoc = wlan_pdev_get_psoc(pdev);
		tx_ops = wlan_psoc_get_lmac_if_txops(psoc);
		/* Trigger MLO setup */
		if (tx_ops && tx_ops->mops.target_if_mlo_setup_req) {
			tx_ops->mops.target_if_mlo_setup_req(
					mlo_ctx->setup_info.pdev_list,
					mlo_ctx->setup_info.num_links,
					mlo_ctx->setup_info.ml_grp_id);
		}
	}
}

qdf_export_symbol(mlo_setup_link_ready);

void mlo_link_setup_complete(struct wlan_objmgr_pdev *pdev)
{
	struct mlo_mgr_context *mlo_ctx = wlan_objmgr_get_mlo_ctx();
	uint8_t link_idx;

	if (!mlo_ctx)
		return;

	for (link_idx = 0; link_idx < mlo_ctx->setup_info.tot_links; link_idx++)
		if (mlo_ctx->setup_info.pdev_list[link_idx] == pdev) {
			mlo_ctx->setup_info.state[link_idx] =
							MLO_LINK_SETUP_DONE;
			break;
		}

	for (link_idx = 0; link_idx < mlo_ctx->setup_info.tot_links; link_idx++)
		if (mlo_ctx->setup_info.state[link_idx] == MLO_LINK_SETUP_DONE)
			continue;
		else
			break;

	if (link_idx == mlo_ctx->setup_info.tot_links) {
		struct wlan_objmgr_psoc *psoc;
		struct wlan_lmac_if_tx_ops *tx_ops;

		psoc = wlan_pdev_get_psoc(pdev);
		tx_ops = wlan_psoc_get_lmac_if_txops(psoc);
		/* Trigger MLO ready */
		if (tx_ops && tx_ops->mops.target_if_mlo_ready) {
			tx_ops->mops.target_if_mlo_ready(
					mlo_ctx->setup_info.pdev_list,
					mlo_ctx->setup_info.num_links);
		}
	}
}

qdf_export_symbol(mlo_link_setup_complete);

void mlo_link_teardown_complete(struct wlan_objmgr_pdev *pdev)
{
	struct mlo_mgr_context *mlo_ctx = wlan_objmgr_get_mlo_ctx();

	if (!mlo_ctx)
		return;
}

qdf_export_symbol(mlo_link_teardown_complete);
#endif /*WLAN_MLO_MULTI_CHIP*/
