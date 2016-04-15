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

#if !defined(WLAN_QCT_SYS_H__)
#define WLAN_QCT_SYS_H__

/**===========================================================================

   \file  wlan_qct_sys.h

   \brief System module API

   ==========================================================================*/

/* $HEADER$ */

/*---------------------------------------------------------------------------
   Include files
   -------------------------------------------------------------------------*/
#include <qdf_types.h>
#include <qdf_status.h>
#include <cds_mq.h>

/*---------------------------------------------------------------------------
   Preprocessor definitions and constants
   -------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
   Type declarations
   -------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------

   \brief sysResponseCback() - SYS async resonse callback

   This is a protype for the callback function that SYS makes to various
   modules in the system.

   \param  pUserData - user data that is passed to the Callback function
   when it is invoked.

   \return Nothing

   \sa sysMcStart(), sysMcThreadProbe(), sysTxThreadProbe()

   --------------------------------------------------------------------------*/
typedef void (*sysResponseCback)(void *pUserData);

typedef enum {
	SYS_MSG_ID_MC_START,
	SYS_MSG_ID_MC_THR_PROBE,
	SYS_MSG_ID_MC_TIMER,
	SYS_MSG_ID_MC_STOP,
	SYS_MSG_ID_FTM_RSP,
	SYS_MSG_ID_QVIT,

} SYS_MSG_ID;

/*---------------------------------------------------------------------------
   Preprocessor definitions and constants
   -------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
   Function declarations and documenation
   -------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------

   \brief sys_build_message_header() - Build / initialize a SYS message header

   This function will initialize the SYS message header with the message type
   and any internal fields needed for a new SYS message.  This function sets
   all but the message body, which is up to the caller to setup based on the
   specific message being built.

   \note There are internal / reserved items in a SYS message that must be
   set correctly for the message to be recognized as a SYS message by
   the SYS message handlers.  It is important for every SYS message to
   be setup / built / initialized through this function.

   \param sysMsgId - a valid message ID for a SYS message.  See the
   SYS_MSG_ID enum for all the valid SYS message IDs.

   \param pMsg - pointer to the message structure to be setup.

   \return

   \sa

   --------------------------------------------------------------------------*/
QDF_STATUS sys_build_message_header(SYS_MSG_ID sysMsgId, cds_msg_t *pMsg);

/*----------------------------------------------------------------------------

   \brief sysMcStart() - start the system Main Controller thread.

   This function starts the SYS (Main Controller) module.  Starting this
   module triggers the CFG download to the 'legacy' MAC software.

   \param p_cds_context - pointer to the CDS Context

   \param userCallback - this is a callback that is called when the SYS
   has completed the 'start' funciton.

   \param pUserData - pointer to some user data entity that is passed to
   the callback function as a parameter when invoked.

   \return QDF_STATUS_SUCCESS -

   \todo:  We have not 'status' on the callback.  How do we notify the
   callback that there is a failure ?

   \sa

   --------------------------------------------------------------------------*/
QDF_STATUS sysMcStart(v_CONTEXT_t p_cds_context, sysResponseCback userCallback,
		      void *pUserData);

/*----------------------------------------------------------------------------

   \brief sys_stop() - Stop the SYS module.

   This function stops the SYS module.

   \todo: What else do we need to do on sys_stop()?

   \param p_cds_context - pointer to the CDS Context

   \return QDF_STATUS_SUCCESS - the SYS module is stopped.

   QDF_STATUS_E_FAILURE - the SYS module open failed to stop.

   \sa

   --------------------------------------------------------------------------*/
QDF_STATUS sys_stop(v_CONTEXT_t p_cds_context);

/*----------------------------------------------------------------------------

   \brief sys_mc_process_msg() - process SYS messages on the Main Controller thread

   This function processes SYS Messages on the Main Controller thread.
   SYS messages consist of all 'legacy' messages (messages bound for legacy
   modules like LIM, HAL, PE, etc.) as well as newly defined SYS message
   types.

   SYS messages are identified by their type (in the SYS_MESSAGES enum) as
   well as a 'cookie' that is in the reserved field of the message structure.
   This 'cookie' is introduced to prevent any message type/ID conflicts with
   the 'legacy' message types.

   Any module attempting to post a message to the SYS module must set the
   message type to one of the types in the SYS_MESSAGE enum *and* must also
   set the Reserved field in the message body to SYS_MSG_COOKIE.

   \param p_cds_context - pointer to the CDS Context

   \param pMsg - pointer to the message to be processed.

   \return - QDF_STATUS_SUCCESS - the message was processed successfully.

   QDF_STATUS_E_BADMSG - a bad (unknown type) message was received
   and subsequently not processed.
   \sa

   --------------------------------------------------------------------------*/
QDF_STATUS sys_mc_process_msg(v_CONTEXT_t p_cds_context, cds_msg_t *pMsg);

void wlan_sys_probe(void);

#endif /* WLAN_QCT_SYS_H__ */
