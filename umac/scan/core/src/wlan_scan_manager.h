/*
 * Copyright (c) 2017 The Linux Foundation. All rights reserved.
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
 * DOC: Defines internal scan manager api
 * Core routines which deal with starting a scan,
 * serializing scan requests, scan cancellation, scan completion,
 * scan event processing.
 */

#ifndef _WLAN_SCAN_MANAGER_API_H_
#define _WLAN_SCAN_MANAGER_API_H_

#include "wlan_scan_main.h"

/*
 * Maximum numbers of callback functions that may be invoked
 * for a particular scan event.
 */
#define MAX_SCAN_EVENT_LISTENERS (MAX_SCAN_EVENT_HANDLERS_PER_PDEV + 1)

/**
 * struct scan_event_listners - listeners interested in a particular scan event
 * @count: number of listners
 * @cb: callback handler
 */
struct scan_event_listeners {
	uint32_t count;
	struct cb_handler cb[MAX_SCAN_EVENT_LISTENERS];
};

/**
 * scm_scan_start_req() - scan start req core api
 * @msg: scheduler message object containing start scan req params
 * @req: start scan req params
 *
 * The API to start a scan
 *
 * Return: QDF_STATUS
 */
QDF_STATUS scm_scan_start_req(struct scheduler_msg *msg);

/**
 * scm_scan_cancel_req() - scan cancel req core api
 * @msg: scheduler message object containing stop scan params
 * @req: stop scan params
 *
 * The API to cancel a scan
 *
 * Return: QDF_STATUS
 */
QDF_STATUS scm_scan_cancel_req(struct scheduler_msg *msg);


/**
 * scm_scan_event_handler() - core scan event handler from tgt interface
 * @msg: scheduler message object containing scan event
 *
 * This function calls registered event handlers of various modules
 *
 * Return: QDF_STATUS
 */
QDF_STATUS scm_scan_event_handler(struct scheduler_msg *msg);

/**
 * scm_scan_free_scan_request_mem() - Free scan request memory
 * @req: scan_start_request object
 *
 * Return: QDF_STATUS
 */
QDF_STATUS scm_scan_free_scan_request_mem(struct scan_start_request *req);

/**
 * scm_scan_event_flush_callback() - flush scan event
 * @msg: scheduler message object containing scan event
 *
 * This function call is invoked when scheduler thread is going down
 *
 * Return: QDF_STATUS
 */
QDF_STATUS scm_scan_event_flush_callback(struct scheduler_msg *msg);

/**
 * scm_bcn_probe_flush_callback() - flush beacon/probe response
 * @msg: scheduler message object containing scan event
 *
 * This function call is invoked when scheduler thread is going down
 *
 * Return: QDF_STATUS
 */
QDF_STATUS scm_bcn_probe_flush_callback(struct scheduler_msg *msg);

/**
 * scm_scan_start_flush_callback() - flush scan start request
 * @msg: scheduler message object containing scan event
 *
 * This function call is invoked when scheduler thread is going down
 *
 * Return: QDF_STATUS
 */
QDF_STATUS scm_scan_start_flush_callback(struct scheduler_msg *msg);

/**
 * scm_scan_cancel_flush_callback() - flush scan cancel request
 * @msg: scheduler message object containing scan event
 *
 * This function call is invoked when scheduler thread is going down
 *
 * Return: QDF_STATUS
 */
QDF_STATUS scm_scan_cancel_flush_callback(struct scheduler_msg *msg);

#endif
