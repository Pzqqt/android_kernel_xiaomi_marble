/*
 * Copyright (c) 2016 The Linux Foundation. All rights reserved.
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
 * @file cdp_txrx_mon.h
 * @brief Define the monitor mode API functions
 * called by the host control SW and the OS interface module
 */

#ifndef _CDP_TXRX_MON_H_
#define _CDP_TXRX_MON_H_
void ol_txrx_monitor_set_filter_ucast_data(ol_txrx_pdev_handle, u_int8_t val);
void ol_txrx_monitor_set_filter_mcast_data(ol_txrx_pdev_handle, u_int8_t val);
void ol_txrx_monitor_set_filter_non_data(ol_txrx_pdev_handle, u_int8_t val);

u_int8_t ol_txrx_monitor_get_filter_ucast_data(
				ol_txrx_vdev_handle vdev_txrx_handle);
u_int8_t ol_txrx_monitor_get_filter_mcast_data(
				ol_txrx_vdev_handle vdev_txrx_handle);
u_int8_t ol_txrx_monitor_get_filter_non_data(
				ol_txrx_vdev_handle vdev_txrx_handle);


int ol_txrx_reset_monitor_mode(ol_txrx_pdev_handle pdev);

#endif
