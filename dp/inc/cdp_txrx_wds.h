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
 * @file cdp_txrx_wds.h
 * @brief Define the host data path WDS API functions
 * called by the host control SW and the OS interface module
 */
#ifndef _CDP_TXRX_WDS_H_
#define _CDP_TXRX_WDS_H_

/**
 * @brief set the wds rx filter policy of the device
 * @details
 *  This flag sets the wds rx policy on the vdev. Rx frames not compliant
 *  with the policy will be dropped.
 *
 * @param vdev - the data virtual device object
 * @param val - the wds rx policy bitmask
 * @return - void
 */
#if WDS_VENDOR_EXTENSION
void
ol_txrx_set_wds_rx_policy(
	ol_txrx_vdev_handle vdev,
	u_int32_t val);
#endif
#endif
