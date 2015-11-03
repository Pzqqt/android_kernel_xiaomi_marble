/*
 * Copyright (c) 2014 The Linux Foundation. All rights reserved.
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

#if !defined( __CDS_GETBIN_H )
#define __CDS_GETBIN_H

/**=========================================================================

   \file  cds_getBin.h

   \brief Connectivity driver services (CDS) binary APIs

   Binary retrieval definitions and APIs.

   These APIs allow components to retrieve binary contents (firmware,
   configuration data, etc.) from a storage medium on the platform.

   ========================================================================*/

/*--------------------------------------------------------------------------
   Include Files
   ------------------------------------------------------------------------*/
#include <cdf_types.h>
#include <cdf_status.h>

/*--------------------------------------------------------------------------
   Preprocessor definitions and constants
   ------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------
   Type declarations
   ------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------
   Function declarations and documenation
   ------------------------------------------------------------------------*/

/**----------------------------------------------------------------------------
   \brief cds_get_conparam()- function to read the insmod parameters
   -----------------------------------------------------------------------------*/
tCDF_CON_MODE cds_get_conparam(void);
bool cds_concurrent_open_sessions_running(void);
bool cds_max_concurrent_connections_reached(void);
void cds_clear_concurrent_session_count(void);
bool cds_is_multiple_active_sta_sessions(void);
bool cds_is_sta_active_connection_exists(void);

#ifdef WLAN_FEATURE_MBSSID
bool cds_concurrent_beaconing_sessions_running(void);
#endif
#endif /* !defined __CDS_GETBIN_H */
