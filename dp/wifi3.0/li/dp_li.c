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

#include "dp_types.h"
#include "dp_li.h"
#include "dp_li_tx.h"
#include "dp_li_rx.h"

qdf_size_t dp_get_context_size_li(enum dp_context_type context_type)
{
	switch (context_type) {
	case DP_CONTEXT_TYPE_SOC:
		return sizeof(struct dp_soc_li);
	case DP_CONTEXT_TYPE_PDEV:
		return sizeof(struct dp_pdev_li);
	case DP_CONTEXT_TYPE_VDEV:
		return sizeof(struct dp_vdev_li);
	case DP_CONTEXT_TYPE_PEER:
		return sizeof(struct dp_peer_li);
	default:
		return 0;
	}
}

static QDF_STATUS dp_soc_attach_li(struct dp_soc *soc)
{
	soc->wbm_sw0_bm_id = hal_tx_get_wbm_sw0_bm_id();

	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS dp_soc_detach_li(struct dp_soc *soc)
{
	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS dp_pdev_attach_li(struct dp_pdev *pdev)
{
	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS dp_pdev_detach_li(struct dp_pdev *pdev)
{
	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS dp_vdev_attach_li(struct dp_soc *soc, struct dp_vdev *vdev)
{
	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS dp_vdev_detach_li(struct dp_soc *soc, struct dp_vdev *vdev)
{
	return QDF_STATUS_SUCCESS;
}

qdf_size_t dp_get_soc_context_size_li(void)
{
	return sizeof(struct dp_soc);
}

void dp_initialize_arch_ops_li(struct dp_arch_ops *arch_ops)
{
	arch_ops->tx_hw_enqueue = dp_tx_hw_enqueue_li;
	arch_ops->txrx_get_context_size = dp_get_context_size_li;
	arch_ops->txrx_soc_attach = dp_soc_attach_li;
	arch_ops->txrx_soc_detach = dp_soc_detach_li;
	arch_ops->txrx_pdev_attach = dp_pdev_attach_li;
	arch_ops->txrx_pdev_detach = dp_pdev_detach_li;
	arch_ops->txrx_vdev_attach = dp_vdev_attach_li;
	arch_ops->txrx_vdev_detach = dp_vdev_detach_li;
	arch_ops->tx_comp_get_params_from_hal_desc =
		dp_tx_comp_get_params_from_hal_desc_li;
	arch_ops->dp_rx_process = dp_rx_process_li;
}
