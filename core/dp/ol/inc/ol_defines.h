/*
 * Copyright (c) 2013-2014, 2016, 2018 The Linux Foundation. All rights reserved.
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

/*
 * Offload specific Opaque Data types.
 */
#ifndef _DEV_OL_DEFINES_H
#define _DEV_OL_DEFINES_H

/**
 * @brief Opaque handle of wmi structure
 */
struct wmi_unified;
typedef struct wmi_unified *wmi_unified_t;

typedef void *ol_scn_t;

 /**
  * ol_txrx_pdev_handle - opaque handle for txrx physical device
  * object
  */
struct ol_txrx_pdev_t;
typedef struct ol_txrx_pdev_t *ol_txrx_pdev_handle;

/**
 * ol_txrx_vdev_handle - opaque handle for txrx virtual device
 * object
 */
struct ol_txrx_vdev_t;
typedef struct ol_txrx_vdev_t *ol_txrx_vdev_handle;

/**
 * ol_pdev_handle - opaque handle for the configuration
 * associated with the physical device
 */
struct ol_pdev_t;
typedef struct ol_pdev_t *ol_pdev_handle;

/**
 * ol_txrx_peer_handle - opaque handle for txrx peer object
 */
struct ol_txrx_peer_t;
typedef struct ol_txrx_peer_t *ol_txrx_peer_handle;

#endif /* _DEV_OL_DEFINES_H */
