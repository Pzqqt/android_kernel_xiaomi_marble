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
 * DOC: nan_datapath.c
 *
 * MAC NAN Data path API implementation
 */

#include "nan_datapath.h"

/**
 * handle_ndp_request_message() - Function to handle NDP requests from SME
 * @mac_ctx: handle to mac structure
 * @msg: pointer to message
 *
 * Return: QDF_STATUS_SUCCESS on success; error number otherwise
 */
QDF_STATUS handle_ndp_request_message(tpAniSirGlobal mac_ctx, tpSirMsgQ msg)
{
	return QDF_STATUS_SUCCESS;
}

/**
 * handle_ndp_event_message() - Handler for NDP events from WMA
 * @mac_ctx: handle to mac structure
 * @msg: pointer to message
 *
 * Return: QDF_STATUS_SUCCESS on success; error number otherwise
 */
QDF_STATUS handle_ndp_event_message(tpAniSirGlobal mac_ctx, tpSirMsgQ msg)
{
	return QDF_STATUS_SUCCESS;
}
