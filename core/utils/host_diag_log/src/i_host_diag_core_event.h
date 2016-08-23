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

#if !defined(__I_HOST_DIAG_CORE_EVENT_H)
#define __I_HOST_DIAG_CORE_EVENT_H

/**=========================================================================

   \file  i_host_diag_core_event.h

   \brief Android specific definitions for WLAN UTIL DIAG events

   ========================================================================*/

/* $Header$ */

/*--------------------------------------------------------------------------
   Include Files
   ------------------------------------------------------------------------*/
#include <qdf_types.h>
#ifdef FEATURE_WLAN_DIAG_SUPPORT
#include <host_diag_event_defs.h>
#endif

/*--------------------------------------------------------------------------
   Preprocessor definitions and constants
   ------------------------------------------------------------------------*/

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifdef FEATURE_WLAN_DIAG_SUPPORT

void host_diag_event_report_payload(uint16_t event_Id, uint16_t length,
				    void *pPayload);
/*---------------------------------------------------------------------------
   Allocate an event payload holder
   ---------------------------------------------------------------------------*/
#define WLAN_HOST_DIAG_EVENT_DEF(payload_name, payload_type) \
	payload_type(payload_name)

/*---------------------------------------------------------------------------
   Report the event
   ---------------------------------------------------------------------------*/
#define WLAN_HOST_DIAG_EVENT_REPORT(payload_ptr, ev_id) \
	do {							\
		host_diag_event_report_payload(ev_id,		\
						sizeof(*(payload_ptr)),			\
						(void *)(payload_ptr));			\
	} while (0)

#else                           /* FEATURE_WLAN_DIAG_SUPPORT */

#define WLAN_HOST_DIAG_EVENT_DEF(payload_name, payload_type)
#define WLAN_HOST_DIAG_EVENT_REPORT(payload_ptr, ev_id)

#endif /* FEATURE_WLAN_DIAG_SUPPORT */

/**
 * enum auth_timeout_type - authentication timeout type
 * @AUTH_FAILURE_TIMEOUT: auth failure timeout
 * @AUTH_RESPONSE_TIMEOUT: auth response timeout
 */
enum auth_timeout_type {
	AUTH_FAILURE_TIMEOUT,
	AUTH_RESPONSE_TIMEOUT,
};

/*-------------------------------------------------------------------------
   Function declarations and documenation
   ------------------------------------------------------------------------*/
#ifdef FEATURE_WLAN_DIAG_SUPPORT
void host_diag_log_wlock(uint32_t reason, const char *wake_lock_name,
		uint32_t timeout, uint32_t status);
#else
static inline void host_diag_log_wlock(uint32_t reason,
		const char *wake_lock_name,
		uint32_t timeout, uint32_t status)
{

}
#endif /* FEATURE_WLAN_DIAG_SUPPORT */

#ifdef FEATURE_WLAN_DIAG_SUPPORT
void host_log_low_resource_failure(uint8_t event_sub_type);
#else
static inline void host_log_low_resource_failure(uint8_t event_sub_type)
{

}
#endif /* FEATURE_WLAN_DIAG_SUPPORT */

#ifdef FEATURE_WLAN_DIAG_SUPPORT
void qdf_wow_wakeup_host_event(uint8_t wow_wakeup_cause);
#else
static inline void qdf_wow_wakeup_host_event(uint8_t wow_wakeup_cause)
{
	return;
}
#endif /* FEATURE_WLAN_DIAG_SUPPORT */
#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* __I_HOST_DIAG_CORE_EVENT_H */
