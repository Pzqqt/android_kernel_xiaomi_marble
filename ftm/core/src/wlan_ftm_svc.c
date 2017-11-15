/*
 * Copyright (c) 2018 The Linux Foundation. All rights reserved.
 *
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
 * DOC: This implementation of init/deint functions for FTM services.
 */

#include "wlan_ftm_svc_i.h"
#include <wlan_lmac_if_def.h>
#include <wlan_ftm_ucfg_api.h>

static inline struct wlan_lmac_if_ftm_tx_ops *
wlan_psoc_get_ftm_txops(struct wlan_objmgr_psoc *psoc)
{
	return &((psoc->soc_cb.tx_ops.ftm_tx_ops));
}

static QDF_STATUS
ftm_pdev_obj_init(struct wifi_ftm_pdev_priv_obj *ftm_pdev_obj)
{
	ftm_pdev_obj->data = qdf_mem_malloc(FTM_CMD_MAX_BUF_LENGTH);
	if (!ftm_pdev_obj->data) {
		ftm_err("Memory alloc failed for ftm pdev obj data");
		return QDF_STATUS_E_NOMEM;
	}

	ftm_pdev_obj->length = 0;

	ftm_pdev_obj->cmd_type = WIFI_FTM_CMD_UNKNOWN;
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
wlan_ftm_pdev_obj_create_notification(struct wlan_objmgr_pdev *pdev,
					void *arg_list)
{
	QDF_STATUS status;
	struct wifi_ftm_pdev_priv_obj *ftm_pdev_obj;

	ftm_pdev_obj = qdf_mem_malloc(sizeof(*ftm_pdev_obj));

	if (!ftm_pdev_obj) {
		ftm_err("Memory alloc failed for ftm pdev obj");
		return QDF_STATUS_E_NOMEM;
	}

	ftm_pdev_obj->pdev = pdev;
	status = ftm_pdev_obj_init(ftm_pdev_obj);

	if (QDF_IS_STATUS_ERROR(status)) {
		ftm_err("ftm pdev obj init failed");
		qdf_mem_free(ftm_pdev_obj);
		return status;
	}

	status = wlan_objmgr_pdev_component_obj_attach(pdev,
						WLAN_UMAC_COMP_FTM,
						ftm_pdev_obj,
						QDF_STATUS_SUCCESS);

	if (QDF_IS_STATUS_ERROR(status)) {
		ftm_err("ftm pdev obj attach failed");
		qdf_mem_free(ftm_pdev_obj);
		return status;
	}

	return status;
}

static QDF_STATUS
ftm_pdev_obj_deinit(struct wifi_ftm_pdev_priv_obj *ftm_pdev_obj)
{
	if (ftm_pdev_obj->data) {
		qdf_mem_free(ftm_pdev_obj->data);

		ftm_pdev_obj->data = NULL;
		ftm_pdev_obj->length = 0;
	}

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
wlan_ftm_pdev_obj_destroy_notification(struct wlan_objmgr_pdev *pdev,
					void *arg_list)
{
	QDF_STATUS status;
	struct wifi_ftm_pdev_priv_obj *ftm_pdev_obj =
		wlan_objmgr_pdev_get_comp_private_obj(pdev, WLAN_UMAC_COMP_FTM);

	if (NULL == ftm_pdev_obj) {
		ftm_err("invalid wifi ftm obj");
		return QDF_STATUS_E_FAULT;
	}

	status = wlan_objmgr_pdev_component_obj_detach(pdev, WLAN_UMAC_COMP_FTM,
							ftm_pdev_obj);

	status = ftm_pdev_obj_deinit(ftm_pdev_obj);
	ftm_pdev_obj->pdev = NULL;

	qdf_mem_free(ftm_pdev_obj);

	return status;
}

QDF_STATUS
wlan_ftm_testmode_attach(struct wlan_objmgr_psoc *psoc)
{
	struct wlan_lmac_if_ftm_tx_ops *ftm_tx_ops;

	ftm_tx_ops = wlan_psoc_get_ftm_txops(psoc);

	if (ftm_tx_ops->ftm_attach)
		return ftm_tx_ops->ftm_attach(psoc);
	else
		return QDF_STATUS_SUCCESS;
}

QDF_STATUS
wlan_ftm_testmode_detach(struct wlan_objmgr_psoc *psoc)
{
	struct wlan_lmac_if_ftm_tx_ops *ftm_tx_ops;

	ftm_tx_ops = wlan_psoc_get_ftm_txops(psoc);

	if (ftm_tx_ops->ftm_detach)
		return ftm_tx_ops->ftm_detach(psoc);
	else
		return QDF_STATUS_SUCCESS;
}

QDF_STATUS
wlan_ftm_cmd_send(struct wlan_objmgr_pdev *pdev, uint8_t *buf,
			uint32_t len, uint8_t pdev_id)
{
	struct wlan_lmac_if_ftm_tx_ops *ftm_tx_ops;
	struct wlan_objmgr_psoc *psoc;

	psoc = wlan_pdev_get_psoc(pdev);
	if (!psoc)
		return QDF_STATUS_E_NOENT;

	ftm_tx_ops = wlan_psoc_get_ftm_txops(psoc);

	if (ftm_tx_ops->ftm_cmd_send)
		return ftm_tx_ops->ftm_cmd_send(pdev, buf, len, pdev_id);

	return QDF_STATUS_SUCCESS;
}
