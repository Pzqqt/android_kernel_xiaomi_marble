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
 * DOC: nan_datapath_api.c
 *
 * SME NAN Data path API implementation
 */
#include "sme_nan_datapath.h"

/**
 * sme_ndp_initiator_req_handler() - ndp initiator req handler
 * @session_id: session id over which the ndp is being created
 * @req_params: request parameters
 *
 * Return: QDF_STATUS_SUCCESS on success; error number otherwise
 */
QDF_STATUS sme_ndp_initiator_req_handler(uint32_t session_id,
	struct ndp_initiator_req *req_params)
{
	return QDF_STATUS_SUCCESS;
}

/**
 * sme_ndp_responder_req_handler() - ndp responder request handler
 * @session_id: session id over which the ndp is being created
 * @req_params: request parameters
 *
 * Return: QDF_STATUS_SUCCESS on success; error number otherwise
 */
QDF_STATUS sme_ndp_responder_req_handler(uint32_t session_id,
	struct ndp_responder_req *req_params)
{
	return QDF_STATUS_SUCCESS;
}

/**
 * sme_ndp_end_req_handler() - ndp end request handler
 * @session_id: session id over which the ndp is being created
 * @req_params: request parameters
 *
 * Return: QDF_STATUS_SUCCESS on success; error number otherwise
 */
QDF_STATUS sme_ndp_end_req_handler(uint32_t session_id,
	struct ndp_end_req *req_params)
{
	return QDF_STATUS_SUCCESS;
}


/**
 * sme_ndp_sched_req_handler() - ndp schedule request handler
 * @session_id: session id over which the ndp is being created
 * @req_params: request parameters
 *
 * Return: QDF_STATUS_SUCCESS on success; error number otherwise
 */
QDF_STATUS sme_ndp_sched_req_handler(uint32_t session_id,
	struct ndp_schedule_update_req *req_params)
{
	return QDF_STATUS_SUCCESS;
}


