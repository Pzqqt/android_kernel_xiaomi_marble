/*
 * Copyright (c) 2014-2018 The Linux Foundation. All rights reserved.
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

#include <sir_common.h>
#include <ani_global.h>
#include "sme_api.h"
#include "csr_inside_api.h"
#include "sme_inside.h"
#include "nan_api.h"
#include "cfg_api.h"
#include "wma_types.h"

void sme_nan_register_callback(mac_handle_t mac_handle, nan_callback callback)
{
	struct mac_context *mac = MAC_CONTEXT(mac_handle);

	if (!mac) {
		sme_err("Invalid MAC handle");
		return;
	}
	mac->sme.nan_callback = callback;
}

void sme_nan_deregister_callback(mac_handle_t mac_handle)
{
	struct mac_context *mac = MAC_CONTEXT(mac_handle);

	if (!mac) {
		sme_err("Invalid MAC handle");
		return;
	}
	mac->sme.nan_callback = NULL;
}

QDF_STATUS sme_nan_request(tpNanRequestReq input)
{
	struct scheduler_msg msg = {0};
	QDF_STATUS status;
	tpNanRequest data;
	size_t data_len;

	data_len = sizeof(tNanRequest) + input->request_data_len;
	data = qdf_mem_malloc(data_len);
	if (!data)
		return QDF_STATUS_E_NOMEM;

	data->request_data_len = input->request_data_len;
	if (input->request_data_len) {
		qdf_mem_copy(data->request_data,
			     input->request_data, input->request_data_len);
	}

	msg.type = WMA_NAN_REQUEST;
	msg.reserved = 0;
	msg.bodyptr = data;

	status = scheduler_post_message(QDF_MODULE_ID_SME, QDF_MODULE_ID_WMA,
					QDF_MODULE_ID_WMA, &msg);

	if (QDF_IS_STATUS_ERROR(status)) {
		sme_err("Not able to post WMA_NAN_REQUEST message to WMA");
		qdf_mem_free(data);
	}

	return status;
}

void sme_nan_event(mac_handle_t mac_handle, tSirNanEvent *event)
{
	struct mac_context *mac = MAC_CONTEXT(mac_handle);

	sme_debug("Received eWNI_SME_NAN_EVENT");

	if (!mac) {
		sme_err("Invalid MAC handle");
		return;
	}

	if (!event) {
		sme_err("NULL event");
		return;
	}

	if (mac->sme.nan_callback)
		mac->sme.nan_callback(mac->hdd_handle, event);
}
