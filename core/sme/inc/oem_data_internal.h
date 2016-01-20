/*
 * Copyright (c) 2014-2015 The Linux Foundation. All rights reserved.
 *
 * Previously licensed under the ISC license by Qualcomm Atheros, Inc.
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

/*
 * This file was originally distributed by Qualcomm Atheros, Inc.
 * under proprietary terms before Copyright ownership was assigned
 * to the Linux Foundation.
 */

#ifdef FEATURE_OEM_DATA_SUPPORT

/**
 *
 * \file oem_data_internal.h
 *
 * Exports and types for the Common OEM DATA REQ/RSP Module interfaces.
 */

#ifndef __OEM_DATA_INTERNAL_H__
#define __OEM_DATA_INTERNAL_H__

#include "csr_support.h"
#include "cds_reg_service.h"
#include "oem_data_api.h"

typedef struct tagOemDataStruct {
	/* a global req id */
	uint32_t nextOemReqId;
	/* indicates that currently a request has been posted and */
	bool oemDataReqActive;
	/* callback function pointer for returning the response */
	oem_data_oem_data_reqCompleteCallback callback;
	/* context of the original caller */
	void *pContext;
	uint32_t oemDataReqID;              /* original request ID */
	uint8_t sessionId;  /* Session on which oem data req is active */
	/* callback for sending data response to oem application */
	sme_send_oem_data_rsp_msg oem_data_rsp_callback;
} tOemDataStruct;

typedef struct tagOemDataCmd {
	uint32_t oemDataReqID;
	tOemDataReq oemDataReq;
} tOemDataCmd;
#endif /* __OEM_DATA_INTERNAL_H__ */

#endif /* FEATURE_OEM_DATA_SUPPORT */
