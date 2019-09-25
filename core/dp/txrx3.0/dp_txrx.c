/*
 * Copyright (c) 2017-2019 The Linux Foundation. All rights reserved.
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

#include <wlan_objmgr_pdev_obj.h>
#include <dp_txrx.h>
#include <dp_internal.h>
#include <cdp_txrx_cmn.h>
#include <cdp_txrx_misc.h>

QDF_STATUS dp_txrx_init(ol_txrx_soc_handle soc, uint8_t pdev_id,
			struct dp_txrx_config *config)
{
	struct dp_txrx_handle *dp_ext_hdl;
	QDF_STATUS qdf_status = QDF_STATUS_SUCCESS;
	uint8_t num_dp_rx_threads;
	struct dp_pdev *pdev;

	if (qdf_unlikely(!soc)) {
		dp_err("soc is NULL");
		return 0;
	}

	pdev = dp_get_pdev_from_soc_pdev_id_wifi3(cdp_soc_t_to_dp_soc(soc),
						  pdev_id);
	if (!pdev) {
		dp_err("pdev is NULL");
		return 0;
	}

	dp_ext_hdl = qdf_mem_malloc(sizeof(*dp_ext_hdl));
	if (!dp_ext_hdl) {
		QDF_ASSERT(0);
		return QDF_STATUS_E_NOMEM;
	}

	dp_info("dp_txrx_handle allocated");
	dp_ext_hdl->soc = soc;
	dp_ext_hdl->pdev = dp_pdev_to_cdp_pdev(pdev);
	cdp_soc_set_dp_txrx_handle(soc, dp_ext_hdl);
	qdf_mem_copy(&dp_ext_hdl->config, config, sizeof(*config));
	dp_ext_hdl->rx_tm_hdl.txrx_handle_cmn =
				dp_txrx_get_cmn_hdl_frm_ext_hdl(dp_ext_hdl);

	num_dp_rx_threads = cdp_get_num_rx_contexts(soc);

	if (dp_ext_hdl->config.enable_rx_threads) {
		qdf_status = dp_rx_tm_init(&dp_ext_hdl->rx_tm_hdl,
					   num_dp_rx_threads);
	}

	return qdf_status;
}

QDF_STATUS dp_txrx_deinit(ol_txrx_soc_handle soc)
{
	struct dp_txrx_handle *dp_ext_hdl;

	if (!soc)
		return QDF_STATUS_E_INVAL;

	dp_ext_hdl = cdp_soc_get_dp_txrx_handle(soc);
	if (!dp_ext_hdl)
		return QDF_STATUS_E_FAULT;

	if (dp_ext_hdl->config.enable_rx_threads)
		dp_rx_tm_deinit(&dp_ext_hdl->rx_tm_hdl);

	qdf_mem_free(dp_ext_hdl);
	dp_info("dp_txrx_handle_t de-allocated");

	cdp_soc_set_dp_txrx_handle(soc, NULL);

	return QDF_STATUS_SUCCESS;
}
