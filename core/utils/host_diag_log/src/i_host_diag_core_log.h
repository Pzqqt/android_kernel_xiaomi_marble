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

#if !defined(__I_HOST_DIAG_CORE_LOG_H)
#define __I_HOST_DIAG_CORE_LOG_H

#ifdef FEATURE_WLAN_DIAG_SUPPORT
#include <log_codes.h>
#endif

/**=========================================================================

   \file  i_host_diag_core_event.h

   \brief android-specific definitions for WLAN UTIL DIAG logs

   ========================================================================*/

/* $Header$ */

/*--------------------------------------------------------------------------
   Include Files
   ------------------------------------------------------------------------*/
#include <qdf_types.h>
#include <qdf_mem.h>

/*--------------------------------------------------------------------------
   Preprocessor definitions and constants
   ------------------------------------------------------------------------*/
/* FIXME To be removed when DIAG support is added. This definiton should be */
/* picked from log.h file above. */
typedef struct {
	/* Specifies the length, in bytes of the entry, including this header. */
	uint16_t len;

	/* Specifies the log code for the entry */
	uint16_t code;

	/*Time Stamp lo */
	uint32_t ts_lo;

	/*Time Stamp hi */
	uint32_t ts_hi;
} __packed log_hdr_type;

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifdef FEATURE_WLAN_DIAG_SUPPORT
void host_diag_log_set_code(void *ptr, uint16_t code);
void host_diag_log_set_length(void *ptr, uint16_t length);
void host_diag_log_set_timestamp(void *plog_hdr_ptr);
void host_diag_log_submit(void *plog_hdr_ptr);

/*---------------------------------------------------------------------------
   Allocate an event payload holder
   ---------------------------------------------------------------------------*/

#define WLAN_HOST_DIAG_LOG_ALLOC(payload_ptr, payload_type, log_code)	\
	do { \
		payload_ptr = (payload_type *)qdf_mem_malloc(sizeof(payload_type)); \
		if (payload_ptr) { \
			host_diag_log_set_code(payload_ptr, log_code); \
			host_diag_log_set_length(payload_ptr, sizeof(payload_type)); \
		} \
	} while (0)

/*---------------------------------------------------------------------------
   Report the event
   ---------------------------------------------------------------------------*/
#define WLAN_HOST_DIAG_LOG_REPORT(payload_ptr) \
	do { \
		if (payload_ptr) { \
			host_diag_log_submit(payload_ptr); \
			qdf_mem_free(payload_ptr); \
		} \
	} while (0)

/*---------------------------------------------------------------------------
   Free the payload
   ---------------------------------------------------------------------------*/
#define WLAN_HOST_DIAG_LOG_FREE(payload_ptr) \
	do {				\
		if (payload_ptr) {	      \
			qdf_mem_free(payload_ptr); \
		}			      \
	} while (0)

#else                           /* FEATURE_WLAN_DIAG_SUPPORT */

#define WLAN_HOST_DIAG_LOG_ALLOC(payload_ptr, payload_type, log_code)
#define WLAN_HOST_DIAG_LOG_REPORT(payload_ptr)
#define WLAN_HOST_DIAG_LOG_FREE(payload_ptr)

#endif /* FEATURE_WLAN_DIAG_SUPPORT */

/*-------------------------------------------------------------------------
   Function declarations and documenation
   ------------------------------------------------------------------------*/

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* __I_HOST_DIAG_CORE_LOG_H */
