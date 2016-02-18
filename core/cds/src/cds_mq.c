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

/**
 * DOC: cds_mq.c
 *
 * Connectivity driver services (CDS) message queue APIs
 *
 * Message Queue Definitions and API
 */

/* Include Files */
#include <cds_mq.h>
#include "cds_sched.h"
#include <cds_api.h>
#include "sir_types.h"

/* Preprocessor definitions and constants */

/* Type declarations */

/* Function declarations and documenation */

tSirRetStatus u_mac_post_ctrl_msg(void *pSirGlobal, void *pMb);

/**
 * cds_mq_init() - initialize cds message queue
 * @pMq: Pointer to the message queue
 *
 * This function initializes the Message queue.
 *
 * Return: cdf status
 */
inline QDF_STATUS cds_mq_init(p_cds_mq_type pMq)
{

	if (pMq == NULL) {
		CDF_TRACE(CDF_MODULE_ID_CDF, CDF_TRACE_LEVEL_ERROR,
			  "%s: NULL pointer passed", __func__);
		return QDF_STATUS_E_FAILURE;
	}

	/* Now initialize the lock */
	spin_lock_init(&pMq->mqLock);

	/* Now initialize the List data structure */
	INIT_LIST_HEAD(&pMq->mqList);

	return QDF_STATUS_SUCCESS;
} /* cds_mq_init() */

/**
 * cds_mq_deinit() - de-initialize cds message queue
 * @pMq: Pointer to the message queue
 *
 * This function de-initializes cds message queue
 *
 * Return: none
 */
inline void cds_mq_deinit(p_cds_mq_type pMq)
{
	if (pMq == NULL) {
		CDF_TRACE(CDF_MODULE_ID_CDF, CDF_TRACE_LEVEL_ERROR,
			  "%s: NULL pointer passed", __func__);
		return;
	}

	/* we don't have to do anything with the embedded list or spinlock */
} /* cds_mq_deinit() */

/**
 * cds_mq_put() - add a message to the message queue
 * @pMq: Pointer to the message queue
 * @pMsgWrapper: Msg wrapper containing the message
 *
 * Return: none
 */
inline void cds_mq_put(p_cds_mq_type pMq, p_cds_msg_wrapper pMsgWrapper)
{
	unsigned long flags;

	if ((pMq == NULL) || (pMsgWrapper == NULL)) {
		CDF_TRACE(CDF_MODULE_ID_CDF, CDF_TRACE_LEVEL_ERROR,
			  "%s: NULL pointer passed", __func__);
		return;
	}

	spin_lock_irqsave(&pMq->mqLock, flags);

	list_add_tail(&pMsgWrapper->msgNode, &pMq->mqList);

	spin_unlock_irqrestore(&pMq->mqLock, flags);

} /* cds_mq_put() */

/**
 * cds_mq_get() - get a message with its wrapper from a message queue
 * @pMq: Pointer to the message queue
 *
 * Return: pointer to the message wrapper
 */
inline p_cds_msg_wrapper cds_mq_get(p_cds_mq_type pMq)
{
	p_cds_msg_wrapper pMsgWrapper = NULL;

	struct list_head *listptr;
	unsigned long flags;

	if (pMq == NULL) {
		CDF_TRACE(CDF_MODULE_ID_CDF, CDF_TRACE_LEVEL_ERROR,
			  "%s: NULL pointer passed", __func__);
		return NULL;
	}

	spin_lock_irqsave(&pMq->mqLock, flags);

	if (list_empty(&pMq->mqList)) {
		CDF_TRACE(CDF_MODULE_ID_CDF, CDF_TRACE_LEVEL_WARN,
			  "%s: CDS Message Queue is empty", __func__);
	} else {
		listptr = pMq->mqList.next;
		pMsgWrapper =
			(p_cds_msg_wrapper) list_entry(listptr, cds_msg_wrapper,
						       msgNode);
		list_del(pMq->mqList.next);
	}

	spin_unlock_irqrestore(&pMq->mqLock, flags);

	return pMsgWrapper;

} /* cds_mq_get() */

/**
 * cds_is_mq_empty() - check if the message queue is empty
 * @pMq: Pointer to the message queue
 *
 * Return: true if message queue is emtpy
 *	   false otherwise
 */
inline bool cds_is_mq_empty(p_cds_mq_type pMq)
{
	bool state = false;
	unsigned long flags;

	if (pMq == NULL) {
		CDF_TRACE(CDF_MODULE_ID_CDF, CDF_TRACE_LEVEL_ERROR,
			  "%s: NULL pointer passed", __func__);
		return QDF_STATUS_E_FAILURE;
	}

	spin_lock_irqsave(&pMq->mqLock, flags);
	state = list_empty(&pMq->mqList) ? true : false;
	spin_unlock_irqrestore(&pMq->mqLock, flags);

	return state;
} /* cds_mq_get() */

/**
 * cds_send_mb_message_to_mac() - post a message to a message queue
 * @pBuf: Pointer to buffer allocated by caller
 *
 * Return: cdf status
 */
QDF_STATUS cds_send_mb_message_to_mac(void *pBuf)
{
	QDF_STATUS cdf_ret_status = QDF_STATUS_E_FAILURE;
	tSirRetStatus sirStatus;
	v_CONTEXT_t cds_context;
	void *hHal;

	cds_context = cds_get_global_context();
	if (NULL == cds_context) {
		CDF_TRACE(CDF_MODULE_ID_SYS, CDF_TRACE_LEVEL_ERROR,
			  "%s: invalid cds_context", __func__);
	} else {
		hHal = cds_get_context(CDF_MODULE_ID_SME);
		if (NULL == hHal) {
			CDF_TRACE(CDF_MODULE_ID_SYS, CDF_TRACE_LEVEL_ERROR,
				  "%s: invalid hHal", __func__);
		} else {
			sirStatus = u_mac_post_ctrl_msg(hHal, pBuf);
			if (eSIR_SUCCESS == sirStatus)
				cdf_ret_status = QDF_STATUS_SUCCESS;
		}
	}

	cdf_mem_free(pBuf);

	return cdf_ret_status;
}
