/*
 * Copyright (c) 2014-2016 The Linux Foundation. All rights reserved.
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

#include "sme_api.h"
#include "sms_debug.h"
#include "csr_inside_api.h"
#include "sme_inside.h"
#include "nan_api.h"
#include "cfg_api.h"
#include "wma_types.h"

/******************************************************************************
 * Function: sme_nan_register_callback
 *
 * Description:
 * This function gets called when HDD wants register nan rsp callback with
 * sme layer.
 *
 * Args:
 * hHal and callback which needs to be registered.
 *
 * Returns:
 * void
 *****************************************************************************/
void sme_nan_register_callback(tHalHandle hHal, nan_callback callback)
{
	tpAniSirGlobal pMac = NULL;

	if (NULL == hHal) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
			  FL("hHal is not valid"));
		return;
	}
	pMac = PMAC_STRUCT(hHal);
	pMac->sme.nanCallback = callback;
}

/**
 * sme_nan_deregister_callback() - NAN De-register cb function
 * @h_hal: Hal handle
 *
 * De-register nan rsp callback with sme layer.
 *
 * Return: void
 */
void sme_nan_deregister_callback(tHalHandle h_hal)
{
	tpAniSirGlobal pmac;

	if (!h_hal) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
			  FL("hHal is not valid"));
		return;
	}
	pmac = PMAC_STRUCT(h_hal);
	pmac->sme.nanCallback = NULL;
}


/******************************************************************************
 * Function: sme_nan_request
 *
 * Description:
 * This function gets called when HDD receives NAN vendor command
 * from userspace
 *
 * Args:
 * Nan Request structure ptr
 *
 * Returns:
 * QDF_STATUS
 *****************************************************************************/
QDF_STATUS sme_nan_request(tpNanRequestReq input)
{
	cds_msg_t msg;
	tpNanRequest data;
	size_t data_len;

	data_len = sizeof(tNanRequest) + input->request_data_len;
	data = qdf_mem_malloc(data_len);

	if (data == NULL) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
			  FL("Memory allocation failure"));
		return QDF_STATUS_E_NOMEM;
	}

	qdf_mem_zero(data, data_len);
	data->request_data_len = input->request_data_len;
	if (input->request_data_len) {
		qdf_mem_copy(data->request_data,
			     input->request_data, input->request_data_len);
	}

	msg.type = WMA_NAN_REQUEST;
	msg.reserved = 0;
	msg.bodyptr = data;

	if (QDF_STATUS_SUCCESS != cds_mq_post_message(QDF_MODULE_ID_WMA, &msg)) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
			  FL
				  ("Not able to post WMA_NAN_REQUEST message to WMA"));
		qdf_mem_free(data);
		return QDF_STATUS_SUCCESS;
	}

	return QDF_STATUS_SUCCESS;
}

/******************************************************************************
* Function: sme_nan_event
*
* Description:
* This callback function will be called when SME received eWNI_SME_NAN_EVENT
* event from WMA
*
* Args:
* hHal - HAL handle for device
* pMsg - Message body passed from WMA; includes NAN header
*
* Returns:
* QDF_STATUS
******************************************************************************/
QDF_STATUS sme_nan_event(tHalHandle hHal, void *pMsg)
{
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	if (NULL == pMsg) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
			  FL("msg ptr is NULL"));
		status = QDF_STATUS_E_FAILURE;
	} else {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_INFO_MED,
			  FL("SME: Received sme_nan_event"));
		if (pMac->sme.nanCallback) {
			pMac->sme.nanCallback(pMac->hHdd,
					      (tSirNanEvent *) pMsg);
		}
	}

	return status;
}
