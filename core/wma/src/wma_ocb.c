/*
 * Copyright (c) 2013-2018 The Linux Foundation. All rights reserved.
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
 * DOC: wma_ocb.c
 *
 * WLAN Host Device Driver 802.11p OCB implementation
 */

#include "wma_ocb.h"
#include "cds_utils.h"
#include "cds_api.h"
#include "wlan_ocb_ucfg_api.h"

/**
 * wma_start_ocb_vdev() - start OCB vdev
 * @config: ocb channel config
 *
 * Return: QDF_STATUS_SUCCESS on success
 */
static QDF_STATUS wma_start_ocb_vdev(struct ocb_config *config)
{
	struct wma_target_req *msg;
	struct wma_vdev_start_req req;
	QDF_STATUS status;
	tp_wma_handle wma = cds_get_context(QDF_MODULE_ID_WMA);

	qdf_mem_zero(&req, sizeof(req));
	msg = wma_fill_vdev_req(wma, config->vdev_id,
				WMA_OCB_SET_CONFIG_CMD,
				WMA_TARGET_REQ_TYPE_VDEV_START,
				(void *)config, 1000);
	if (!msg) {
		WMA_LOGE(FL("Failed to fill vdev req %d"), config->vdev_id);

		return QDF_STATUS_E_NOMEM;
	}
	req.chan = cds_freq_to_chan(config->channels[0].chan_freq);
	req.vdev_id = msg->vdev_id;
	if (cds_chan_to_band(req.chan) == CDS_BAND_2GHZ)
		req.dot11_mode = WNI_CFG_DOT11_MODE_11G;
	else
		req.dot11_mode = WNI_CFG_DOT11_MODE_11A;

	req.preferred_rx_streams = 2;
	req.preferred_tx_streams = 2;

	status = wma_vdev_start(wma, &req, false);
	if (status != QDF_STATUS_SUCCESS) {
		wma_remove_vdev_req(wma, req.vdev_id,
				    WMA_TARGET_REQ_TYPE_VDEV_START);
		WMA_LOGE(FL("vdev_start failed, status = %d"), status);
	}

	return status;
}

QDF_STATUS wma_ocb_register_callbacks(tp_wma_handle wma_handle)
{
	ucfg_ocb_register_vdev_start(wma_handle->pdev, wma_start_ocb_vdev);

	return QDF_STATUS_SUCCESS;
}
