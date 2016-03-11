/*
 * Copyright (c) 2013-2016 The Linux Foundation. All rights reserved.
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
 * \file oem_data_api.h
 *
 * Exports and types for the Common OEM DATA REQ/RSP Module interfaces.
 */

#ifndef __OEM_DATA_API_H__
#define __OEM_DATA_API_H__
#include "sir_api.h"
#include "sir_mac_prot_def.h"
#include "csr_link_list.h"

#ifndef OEM_DATA_REQ_SIZE
#define OEM_DATA_REQ_SIZE 280
#endif

#ifndef OEM_DATA_RSP_SIZE
#define OEM_DATA_RSP_SIZE 1724
#endif

/* message subtype for internal purpose */
#define OEM_MESSAGE_SUBTYPE_INTERNAL   0xdeadbeef
#define OEM_MESSAGE_SUBTYPE_LEN 4

/* Structure for defining req sent to the PE */
typedef struct tagOemDataReq {
	uint8_t sessionId;
	uint8_t data_len;
	uint8_t *data;
} tOemDataReq, tOemDataReqConfig;

typedef struct tagOemDataRsp {
	uint32_t rsp_len;
	uint8_t *oem_data_rsp;
} tOemDataRsp;

typedef enum {
	eOEM_DATA_REQ_SUCCESS = 1,
	eOEM_DATA_REQ_FAILURE,
	eOEM_DATA_REQ_INVALID_MODE,
} eOemDataReqStatus;
QDF_STATUS oem_data_oem_data_req_open(tHalHandle hHal);
QDF_STATUS oem_data_oem_data_req_close(tHalHandle hHal);

/*
 * HDD Callback function for the sme to callback when
 * the oem data rsp is available
 */
typedef QDF_STATUS (*oem_data_oem_data_reqCompleteCallback)(tHalHandle,
		void *p2, uint32_t oemDataReqID, eOemDataReqStatus status);

QDF_STATUS oem_data_oem_data_req(tHalHandle, uint8_t, tOemDataReqConfig *,
				 uint32_t *pOemDataReqID);
QDF_STATUS sme_handle_oem_data_rsp(tHalHandle hHal, uint8_t *);
QDF_STATUS oem_data_is_oem_data_req_allowed(tHalHandle hHal);
typedef void (*sme_send_oem_data_rsp_msg)(int length, uint8_t *oem_data_rsp);
#endif /* _OEM_DATA_API_H__ */

#endif /* FEATURE_OEM_DATA_SUPPORT */
