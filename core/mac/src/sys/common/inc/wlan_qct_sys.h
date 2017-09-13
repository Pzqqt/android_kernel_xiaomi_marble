/*
 * Copyright (c) 2013-2017 The Linux Foundation. All rights reserved.
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
#include <scheduler_api.h>

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
QDF_STATUS sys_build_message_header(SYS_MSG_ID sysMsgId,
				    struct scheduler_msg *pMsg);

QDF_STATUS sys_mc_process_handler(struct scheduler_msg *msg);

void wlan_sys_probe(void);

#endif /* WLAN_QCT_SYS_H__ */
