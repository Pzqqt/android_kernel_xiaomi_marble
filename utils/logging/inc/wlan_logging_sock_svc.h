/*
 * Copyright (c) 2014-2018 The Linux Foundation. All rights reserved.
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

/******************************************************************************
* wlan_logging_sock_svc.h
*
******************************************************************************/

#ifndef WLAN_LOGGING_SOCK_SVC_H
#define WLAN_LOGGING_SOCK_SVC_H

#include <wlan_nlink_srv.h>
#include <qdf_status.h>
#include <qdf_trace.h>
#include <wlan_nlink_common.h>

int wlan_logging_sock_init_svc(void);
int wlan_logging_sock_deinit_svc(void);
int wlan_log_to_user(QDF_TRACE_LEVEL log_level, char *to_be_sent, int length);

#ifdef WLAN_LOGGING_SOCK_SVC_ENABLE
void wlan_logging_set_per_pkt_stats(void);
void wlan_logging_set_fw_flush_complete(void);
void wlan_flush_host_logs_for_fatal(void);
void wlan_logging_set_active(bool active);
void wlan_logging_set_log_to_console(bool log_to_console);
void wlan_deregister_txrx_packetdump(void);
void wlan_register_txrx_packetdump(void);
#else
static inline void wlan_flush_host_logs_for_fatal(void) {}
static inline void wlan_logging_set_per_pkt_stats(void) {}
static inline void wlan_logging_set_fw_flush_complete(void) {}
static inline void wlan_logging_set_active(bool active) {}
static inline void wlan_logging_set_log_to_console(bool log_to_console) {}
static inline void wlan_deregister_txrx_packetdump(void) {}
static inline void wlan_register_txrx_packetdump(void) {}
#endif /* WLAN_LOGGING_SOCK_SVC_ENABLE */

#if defined(WLAN_LOGGING_SOCK_SVC_ENABLE) && defined(FEATURE_WLAN_DIAG_SUPPORT)
void wlan_report_log_completion(uint32_t is_fatal,
		uint32_t indicator,
		uint32_t reason_code,
		uint8_t ring_id);
#else
static inline void wlan_report_log_completion(uint32_t is_fatal,
		uint32_t indicator,
		uint32_t reason_code,
		uint8_t ring_id)
{
	return;
}

#endif /* FEATURE_WLAN_DIAG_SUPPORT */

void wlan_pkt_stats_to_logger_thread(void *pl_hdr, void *pkt_dump, void *data);


#endif /* WLAN_LOGGING_SOCK_SVC_H */
