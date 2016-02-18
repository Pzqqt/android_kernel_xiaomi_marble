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

#if !defined(__CDS_MQ_H)
#define __CDS_MQ_H

/**=========================================================================

   \file  cds_mq.h

   \brief virtual Operating System Services (CDF) message queue APIs

   Message Queue Definitions and API

   ========================================================================*/

/*--------------------------------------------------------------------------
   Include Files
   ------------------------------------------------------------------------*/
#include <cdf_types.h>
#include <qdf_status.h>

/*--------------------------------------------------------------------------
   Preprocessor definitions and constants
   ------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------
   Type declarations
   ------------------------------------------------------------------------*/

/* cds Message Type.
   This represnets a message that can be posted to another module through
   the cds Message Queues.
   \note This is mapped directly to the tSirMsgQ for backward
   compatibility with the legacy MAC code */

typedef struct cds_msg_s {
	uint16_t type;
	/*
	 * This field can be used as sequence number/dialog token for matching
	 * requests and responses.
	 */
	uint16_t reserved;
	/**
	 * Based on the type either a bodyptr pointer into
	 * memory or bodyval as a 32 bit data is used.
	 * bodyptr: is always a freeable pointer, one should always
	 * make sure that bodyptr is always freeable.
	 *
	 * Messages should use either bodyptr or bodyval; not both !!!.
	 */
	void *bodyptr;

	uint32_t bodyval;

	/*
	 * Some messages provide a callback function.  The function signature
	 * must be agreed upon between the two entities exchanging the message
	 */
	void *callback;

} cds_msg_t;

/*-------------------------------------------------------------------------
   Function declarations and documenation
   ------------------------------------------------------------------------*/

/* Message Queue IDs */
typedef enum {
	/* Message Queue ID for messages bound for SME */
	CDS_MQ_ID_SME = CDF_MODULE_ID_SME,

	/* Message Queue ID for messages bound for PE */
	CDS_MQ_ID_PE = CDF_MODULE_ID_PE,

	/* Message Queue ID for messages bound for WMA */
	CDS_MQ_ID_WMA = CDF_MODULE_ID_WMA,

	/* Message Queue ID for messages bound for the SYS module */
	CDS_MQ_ID_SYS = CDF_MODULE_ID_SYS,

} CDS_MQ_ID;

/**---------------------------------------------------------------------------

   \brief cds_mq_post_message() - post a message to a message queue

   This API allows messages to be posted to a specific message queue.  Messages
   can be posted to the following message queues:

   <ul>
    <li> SME
    <li> PE
    <li> HAL
    <li> TL
   </ul>

   \param msgQueueId - identifies the message queue upon which the message
   will be posted.

   \param message - a pointer to a message buffer.  Memory for this message
   buffer is allocated by the caller and free'd by the CDF after the
   message is posted to the message queue.  If the consumer of the
   message needs anything in this message, it needs to copy the contents
   before returning from the message queue handler.

   \return QDF_STATUS_SUCCESS - the message has been successfully posted
   to the message queue.

   QDF_STATUS_E_INVAL - The value specified by msgQueueId does not
   refer to a valid Message Queue Id.

   QDF_STATUS_E_FAULT  - message is an invalid pointer.

   QDF_STATUS_E_FAILURE - the message queue handler has reported
   an unknown failure.

   \sa

   --------------------------------------------------------------------------*/
QDF_STATUS cds_mq_post_message(CDS_MQ_ID msgQueueId, cds_msg_t *message);

/**---------------------------------------------------------------------------

   \brief cds_send_mb_message_to_mac() - post a message to a message queue

   \param pBuf is a buffer allocated by caller. The actual structure varies
   base on message type

   \return QDF_STATUS_SUCCESS - the message has been successfully posted
   to the message queue.

   QDF_STATUS_E_FAILURE - the message queue handler has reported
   an unknown failure.

   \sa
   --------------------------------------------------------------------------*/

QDF_STATUS cds_send_mb_message_to_mac(void *pBuf);

#endif /* if !defined __CDS_MQ_H */
