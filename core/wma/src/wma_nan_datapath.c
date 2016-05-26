/*
 * Copyright (c) 2016 The Linux Foundation. All rights reserved.
 *
 * Previously licensed under the ISC license by Qualcomm Atheros, Inc.
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
 * DOC: wma_nan_datapath.c
 *
 * WMA NAN Data path API implementation
 */

#include "wma_nan_datapath.h"


/**
 * wma_handle_ndp_initiator_req() - NDP initiator request handler
 * @wma_handle: wma handle
 * @req_params: request parameters
 *
 * Return: QDF_STATUS_SUCCESS on success; error number otherwise
 */
QDF_STATUS wma_handle_ndp_initiator_req(tp_wma_handle wma_handle,
					struct ndp_initiator_req *req_params)
{
	return QDF_STATUS_SUCCESS;
}

/**
 * wma_handle_ndp_responder_req() - NDP responder request handler
 * @wma_handle: wma handle
 * @req_params: request parameters
 *
 * Return: QDF_STATUS_SUCCESS on success; error number otherwise
 */
QDF_STATUS wma_handle_ndp_responder_req(tp_wma_handle wma_handle,
					struct ndp_responder_req *req_params)
{
	return QDF_STATUS_SUCCESS;
}

/**
 * wma_handle_ndp_end_req() - NDP end request handler
 * @wma_handle: wma handle
 * @req_params: request parameters
 *
 * Return: QDF_STATUS_SUCCESS on success; error number otherwise
 */
QDF_STATUS wma_handle_ndp_end_req(tp_wma_handle wma_handle,
				struct ndp_end_req *req_params)
{
	return QDF_STATUS_SUCCESS;
}

/**
 * wma_handle_ndp_sched_update_req() - NDP schedule update request handler
 * @wma_handle: wma handle
 * @req_params: request parameters
 *
 * Return: QDF_STATUS_SUCCESS on success; error number otherwise
 */
QDF_STATUS wma_handle_ndp_sched_update_req(tp_wma_handle wma_handle,
					struct ndp_end_req *req_params)
{
	return QDF_STATUS_SUCCESS;
}

/**
 * wma_ndp_indication_event_handler() - NDP indication event handler
 * @handle: wma handle
 * @event_info: event handler data
 * @len: length of event_info
 *
 * Return: QDF_STATUS_SUCCESS on success; error number otherwise
 */
QDF_STATUS wma_ndp_indication_event_handler(void *handle,
					uint8_t  *event_info, uint32_t len)
{
	return QDF_STATUS_SUCCESS;
}

/**
 * wma_ndp_responder_resp_event_handler() - NDP responder response event handler
 * @handle: wma handle
 * @event_info: event handler data
 * @len: length of event_info
 *
 * Return: QDF_STATUS_SUCCESS on success; error number otherwise
 */
QDF_STATUS wma_ndp_responder_resp_event_handler(void *handle,
					uint8_t  *event_info, uint32_t len)
{
	return QDF_STATUS_SUCCESS;
}

/**
 * wma_ndp_confirm_event_handler() - NDP confirm event handler
 * @handle: wma handle
 * @event_info: event handler data
 * @len: length of event_info
 *
 * Return: QDF_STATUS_SUCCESS on success; error number otherwise
 */
QDF_STATUS wma_ndp_confirm_event_handler(void *handle,
					uint8_t  *event_info, uint32_t len)
{
	return QDF_STATUS_SUCCESS;
}

/**
 * wma_ndp_end_response_event_handler() - NDP end response event handler
 * @handle: wma handle
 * @event_info: event handler data
 * @len: length of event_info
 *
 * Return: QDF_STATUS_SUCCESS on success; error number otherwise
 */
QDF_STATUS wma_ndp_end_response_event_handler(void *handle,
					uint8_t  *event_info, uint32_t len)
{
	return QDF_STATUS_SUCCESS;
}

/**
 * wma_ndp_end_indication_event_handler() - NDP end indication event handler
 * @handle: wma handle
 * @event_info: event handler data
 * @len: length of event_info
 *
 * Return: QDF_STATUS_SUCCESS on success; error number otherwise
 */
QDF_STATUS wma_ndp_end_indication_event_handler(void *handle,
					uint8_t  *event_info, uint32_t len)
{
	return QDF_STATUS_SUCCESS;
}

/**
 * wma_ndp_sched_update_rsp_event_handler() -NDP sched update rsp event handler
 * @handle: wma handle
 * @event_info: event handler data
 * @len: length of event_info
 *
 * Return: QDF_STATUS_SUCCESS on success; error number otherwise
 */
QDF_STATUS wma_ndp_sched_update_rsp_event_handler(void *handle,
					uint8_t  *event_info, uint32_t len)
{
	return QDF_STATUS_SUCCESS;
}
