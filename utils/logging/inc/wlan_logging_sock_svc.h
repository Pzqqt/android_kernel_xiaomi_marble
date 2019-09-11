/*
 * Copyright (c) 2014-2019 The Linux Foundation. All rights reserved.
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

/**
 * wlan_logging_set_flush_timer() - Sets the time period for log flush timer
 * @milliseconds: Time period in milliseconds
 *
 * This function sets the time period interval during which the log buffers
 * will be flushed out to user space. Setting this interval can set an
 * approximate maximum delay after which any message logged through QDF_TRACE
 * will appear at user-space
 *
 * Return: void
 */
int wlan_logging_set_flush_timer(uint32_t milliseconds);

#ifdef WLAN_LOGGING_SOCK_SVC_ENABLE
void wlan_logging_set_per_pkt_stats(void);
void wlan_logging_set_fw_flush_complete(void);
void wlan_flush_host_logs_for_fatal(void);
void wlan_logging_set_active(bool active);
void wlan_logging_set_log_to_console(bool log_to_console);
#else
static inline void wlan_flush_host_logs_for_fatal(void) {}
static inline void wlan_logging_set_per_pkt_stats(void) {}
static inline void wlan_logging_set_fw_flush_complete(void) {}
static inline void wlan_logging_set_active(bool active) {}
static inline void wlan_logging_set_log_to_console(bool log_to_console) {}
#endif /* WLAN_LOGGING_SOCK_SVC_ENABLE */

#if defined(WLAN_LOGGING_SOCK_SVC_ENABLE) && \
	defined(FEATURE_PKTLOG) && !defined(REMOVE_PKT_LOG)
/**
 * wlan_deregister_txrx_packetdump() - tx/rx packet dump
 *  deregistration
 * @pdev_id: id of the datapath pdev handle
 *
 * This function is used to deregister tx/rx packet dump callbacks
 * with ol, pe and htt layers
 *
 * Return: None
 *
 */
void wlan_deregister_txrx_packetdump(uint8_t pdev_id);

/**
 * wlan_register_txrx_packetdump() - tx/rx packet dump
 * registration
 * @pdev_id: id of the datapath pdev handle
 *
 * This function is used to register tx/rx packet dump callbacks
 * with ol, pe and htt layers
 *
 * Return: None
 *
 */
void wlan_register_txrx_packetdump(uint8_t pdev_id);
#else
static inline void wlan_deregister_txrx_packetdump(uint8_t pdev_id) {}
static inline void wlan_register_txrx_packetdump(uint8_t pdev_id) {}
#endif

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

#if defined(WLAN_LOGGING_SOCK_SVC_ENABLE) && \
	defined(FEATURE_PKTLOG) && !defined(REMOVE_PKT_LOG)
void wlan_pkt_stats_to_logger_thread(void *pl_hdr, void *pkt_dump, void *data);
#else
static inline
void wlan_pkt_stats_to_logger_thread(void *pl_hdr, void *pkt_dump, void *data)
{
}
#endif

/**
 * enum tx_status - tx status
 * @tx_status_ok: successfully sent + acked
 * @tx_status_discard: discard - not sent (congestion control)
 * @tx_status_no_ack: no_ack - sent, but no ack
 * @tx_status_download_fail: download_fail -
 * the host could not deliver the tx frame to the target
 * @tx_status_peer_del: peer_del - tx completion for
 * already deleted peer used for HL case
 *
 * This enum has tx status types
 */
enum tx_status {
	tx_status_ok,
	tx_status_discard,
	tx_status_no_ack,
	tx_status_download_fail,
	tx_status_peer_del,
};

#endif /* WLAN_LOGGING_SOCK_SVC_H */
