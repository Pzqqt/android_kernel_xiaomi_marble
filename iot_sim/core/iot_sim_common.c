/*
 * Copyright (c) 2020, The Linux Foundation. All rights reserved.
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

#include "iot_sim_cmn_api_i.h"
#include <qdf_mem.h>
#include <qdf_types.h>

QDF_STATUS
iot_sim_control_cmn(struct wlan_objmgr_psoc *psoc, wbuf_t wbuf)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	struct iot_sim_context *isc;

	if (!psoc) {
		iot_sim_err("PDEV is NULL!");
		goto bad;
	}

	isc = iot_sim_get_ctx_from_psoc(psoc);
	if (!isc) {
		iot_sim_err("iot_sim context is NULL!");
		goto bad;
	}

	status = QDF_STATUS_SUCCESS;
bad:
	return status;
}

/**
 * iot_sim_ctx_deinit() - De-initialize function pointers from iot_sim context
 * @sc - Reference to iot_sim_context object
 *
 * Return: None
 */
static void
iot_sim_ctx_deinit(struct iot_sim_context *isc)
{
	if (isc)
		isc->iot_sim_operation_handler = NULL;
}

QDF_STATUS
wlan_iot_sim_psoc_obj_create_handler(struct wlan_objmgr_psoc *psoc, void *arg)
{
	struct iot_sim_context *isc = NULL;

	if (!psoc) {
		iot_sim_err("PSOC is NULL");
		return QDF_STATUS_E_NULL_VALUE;
	}

	isc = (struct iot_sim_context *)
	    qdf_mem_malloc(sizeof(struct iot_sim_context));
	if (!isc)
		return QDF_STATUS_E_NOMEM;

	isc->psoc_obj = psoc;
	wlan_objmgr_psoc_component_obj_attach(psoc, WLAN_IOT_SIM_COMP,
					      (void *)isc, QDF_STATUS_SUCCESS);

	iot_sim_info("iot_sim component psoc object created");

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
wlan_iot_sim_psoc_obj_destroy_handler(struct wlan_objmgr_psoc *psoc,
				      void *arg)
{
	struct iot_sim_context *isc = NULL;

	if (!psoc) {
		iot_sim_err("PSOC is NULL");
		return QDF_STATUS_E_NULL_VALUE;
	}

	isc = wlan_objmgr_psoc_get_comp_private_obj(psoc,
						    WLAN_IOT_SIM_COMP);
	if (isc) {
		wlan_objmgr_psoc_component_obj_detach(psoc,
						      WLAN_IOT_SIM_COMP,
						      (void *)isc);
		/* Deinitilise function pointers from iot_sim context */
		iot_sim_ctx_deinit(isc);
		qdf_mem_free(isc);
	}
	iot_sim_info("iot_sim component psoc object destroyed");

	return QDF_STATUS_SUCCESS;
}
