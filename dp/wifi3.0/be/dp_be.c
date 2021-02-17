/*
 * Copyright (c) 2021 The Linux Foundation. All rights reserved.
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
#include "dp_be.h"
#include "dp_be_tx.h"
#include "dp_be_rx.h"

qdf_size_t dp_get_context_size_be(enum dp_context_type context_type)
{
	switch (context_type) {
	case DP_CONTEXT_TYPE_SOC:
		return sizeof(struct dp_soc_be);
	case DP_CONTEXT_TYPE_PDEV:
		return sizeof(struct dp_pdev_be);
	case DP_CONTEXT_TYPE_VDEV:
		return sizeof(struct dp_vdev_be);
	case DP_CONTEXT_TYPE_PEER:
		return sizeof(struct dp_peer_be);
	default:
		return 0;
	}
}

static QDF_STATUS dp_soc_attach_be(struct dp_soc *soc)
{
	struct dp_soc_be *be_soc = dp_get_be_soc_from_dp_soc(soc);
	QDF_STATUS qdf_status = QDF_STATUS_SUCCESS;

	soc->wbm_sw0_bm_id = hal_tx_get_wbm_sw0_bm_id();
	qdf_status = dp_tx_init_bank_profiles(be_soc);

	return qdf_status;
}

static QDF_STATUS dp_soc_detach_be(struct dp_soc *soc)
{
	struct dp_soc_be *be_soc = dp_get_be_soc_from_dp_soc(soc);

	dp_tx_deinit_bank_profiles(be_soc);

	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS dp_pdev_attach_be(struct dp_pdev *pdev)
{
	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS dp_pdev_detach_be(struct dp_pdev *pdev)
{
	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS dp_vdev_attach_be(struct dp_soc *soc, struct dp_vdev *vdev)
{
	struct dp_soc_be *be_soc = dp_get_be_soc_from_dp_soc(soc);
	struct dp_vdev_be *be_vdev = dp_get_be_vdev_from_dp_vdev(vdev);

	be_vdev->bank_id = dp_tx_get_bank_profile(be_soc, be_vdev);

	/* Needs to be enabled after bring-up*/
	be_vdev->vdev_id_check_en = false;

	if (be_vdev->bank_id == DP_BE_INVALID_BANK_ID) {
		QDF_BUG(0);
		return QDF_STATUS_E_FAULT;
	}
	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS dp_vdev_detach_be(struct dp_soc *soc, struct dp_vdev *vdev)
{
	struct dp_soc_be *be_soc = dp_get_be_soc_from_dp_soc(soc);
	struct dp_vdev_be *be_vdev = dp_get_be_vdev_from_dp_vdev(vdev);

	dp_tx_put_bank_profile(be_soc, be_vdev);
	return QDF_STATUS_SUCCESS;
}

qdf_size_t dp_get_soc_context_size_be(void)
{
	return sizeof(struct dp_soc_be);
}

void dp_initialize_arch_ops_be(struct dp_arch_ops *arch_ops)
{
	arch_ops->tx_hw_enqueue = dp_tx_hw_enqueue_be;
	arch_ops->txrx_get_context_size = dp_get_context_size_be;
	arch_ops->dp_rx_process = dp_rx_process_be;

	arch_ops->txrx_soc_attach = dp_soc_attach_be;
	arch_ops->txrx_soc_detach = dp_soc_detach_be;
	arch_ops->txrx_pdev_attach = dp_pdev_attach_be;
	arch_ops->txrx_pdev_detach = dp_pdev_detach_be;
	arch_ops->txrx_vdev_attach = dp_vdev_attach_be;
	arch_ops->txrx_vdev_detach = dp_vdev_detach_be;
	arch_ops->tx_comp_get_params_from_hal_desc =
		dp_tx_comp_get_params_from_hal_desc_be;
}
