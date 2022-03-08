/*
 * Copyright (c) 2022 Qualcomm Innovation Center, Inc. All rights reserved.
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
 * DOC: wlan_twt_tgt_if_tx_api.c
 *
 * This file provides definitions for twt tgt_if APIs, which will
 * further call target_if component using LMAC TWT txops
 */
#include <qdf_types.h>
#include <wlan_objmgr_psoc_obj.h>
#include <wlan_twt_public_structs.h>
#include <wlan_twt_tgt_if_tx_api.h>
#include <wlan_lmac_if_def.h>
#include <wlan_twt_api.h>

QDF_STATUS
tgt_twt_enable_req_send(struct wlan_objmgr_psoc *psoc,
			struct twt_enable_param *req)
{
	struct wlan_lmac_if_twt_tx_ops *tx_ops;
	QDF_STATUS status;

	if (!psoc) {
		twt_err("null psoc");
		return QDF_STATUS_E_INVAL;
	}

	if (!req) {
		twt_err("Invalid input");
		return QDF_STATUS_E_INVAL;
	}

	tx_ops = wlan_twt_get_tx_ops(psoc);
	if (!tx_ops || !tx_ops->enable_req) {
		twt_err("twt enable_req tx_ops is null");
		return QDF_STATUS_E_NULL_VALUE;
	}

	status = tx_ops->enable_req(psoc, req);
	if (QDF_IS_STATUS_ERROR(status)) {
		twt_err("tx_ops enable_req failed (status=%d)", status);
		return status;
	}

	return status;
}

QDF_STATUS
tgt_twt_disable_req_send(struct wlan_objmgr_psoc *psoc,
			 struct twt_disable_param *req)
{
	struct wlan_lmac_if_twt_tx_ops *tx_ops;
	QDF_STATUS status;

	if (!psoc) {
		twt_err("null psoc");
		return QDF_STATUS_E_INVAL;
	}

	if (!req) {
		twt_err("Invalid input");
		return QDF_STATUS_E_INVAL;
	}

	tx_ops = wlan_twt_get_tx_ops(psoc);
	if (!tx_ops || !tx_ops->disable_req) {
		twt_err("twt disable_req tx_ops is null");
		return QDF_STATUS_E_NULL_VALUE;
	}

	status = tx_ops->disable_req(psoc, req);
	if (QDF_IS_STATUS_ERROR(status)) {
		twt_err("tx_ops disable_req failed (status=%d)", status);
		return status;
	}

	return status;
}
