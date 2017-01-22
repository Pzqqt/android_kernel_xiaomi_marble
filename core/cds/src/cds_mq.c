/*
 * Copyright (c) 2014-2017 The Linux Foundation. All rights reserved.
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

/**
 * DOC: cds_mq.c
 *
 * Connectivity driver services (CDS) message queue APIs
 *
 * Message Queue Definitions and API
 */

/* Include Files */
#include "cds_sched.h"
#include <cds_api.h>
#include "sir_types.h"

/* Preprocessor definitions and constants */

/* Type declarations */

/* Function declarations and documenation */

tSirRetStatus u_mac_post_ctrl_msg(void *pSirGlobal, void *pMb);

/**
 * cds_send_mb_message_to_mac() - post a message to a message queue
 * @pBuf: Pointer to buffer allocated by caller
 *
 * Return: qdf status
 */
QDF_STATUS cds_send_mb_message_to_mac(void *pBuf)
{
	QDF_STATUS qdf_ret_status = QDF_STATUS_E_FAILURE;
	tSirRetStatus sirStatus;
	v_CONTEXT_t cds_context;
	void *hHal;

	cds_context = cds_get_global_context();
	if (NULL == cds_context) {
		QDF_TRACE(QDF_MODULE_ID_SYS, QDF_TRACE_LEVEL_ERROR,
			  "%s: invalid cds_context", __func__);
	} else {
		hHal = cds_get_context(QDF_MODULE_ID_SME);
		if (NULL == hHal) {
			QDF_TRACE(QDF_MODULE_ID_SYS, QDF_TRACE_LEVEL_ERROR,
				  "%s: invalid hHal", __func__);
		} else {
			sirStatus = u_mac_post_ctrl_msg(hHal, pBuf);
			if (eSIR_SUCCESS == sirStatus)
				qdf_ret_status = QDF_STATUS_SUCCESS;
		}
	}

	qdf_mem_free(pBuf);

	return qdf_ret_status;
}
