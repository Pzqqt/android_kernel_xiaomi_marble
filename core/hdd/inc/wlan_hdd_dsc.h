/*
 * Copyright (c) 2018 The Linux Foundation. All rights reserved.
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

#ifndef __WLAN_HDD_DSC_H__
#define __WLAN_HDD_DSC_H__

#include "wlan_dsc.h"
#include "wlan_hdd_main.h"

/**
 * hdd_dsc_init() - global initializer for HDD DSC
 *
 * Return: None
 */
void hdd_dsc_init(void);

/**
 * hdd_dsc_deinit() - global deinitializer for HDD DSC
 *
 * Return: None
 */
void hdd_dsc_deinit(void);

/**
 * hdd_dsc_psoc_from_wiphy() - get dsc psoc from wiphy
 * @wiphy: Pointer to wireless hardware description
 *
 * Return: dsc_psoc on success, NULL on failure
 */
struct dsc_psoc *hdd_dsc_psoc_from_wiphy(struct wiphy *wiphy);

/**
 * hdd_vdev_ops_register() - register a dsc_vdev to accept new operations
 * @net_dev: the net_device which will be used for lookup during op start
 * @dsc_vdev: the dsc_vdev to use to protect operations on @net_dev
 *
 * Return: None
 */
void hdd_vdev_ops_register(struct net_device *net_dev,
			   struct dsc_vdev *dsc_vdev);

/**
 * hdd_vdev_ops_unregister() - unregister a dsc_vdev to reject future operations
 * @net_dev: the net_device to use for lookup
 *
 * Return: None
 */
void hdd_vdev_ops_unregister(struct net_device *net_dev);

/**
 * struct hdd_vdev_op - opaque handle used to identify a specific vdev operation
 */
struct hdd_vdev_op;

/**
 * hdd_vdev_op_start() - attempt to start a vdev-level driver operation
 * @net_dev: the net_device to start the operation on
 *
 * Return: an operation handle on success, NULL on failure
 */
#define hdd_vdev_op_start(net_dev) __hdd_vdev_op_start(net_dev, __func__)

/**
 * hdd_vdev_op_start_with_wdev() - attempt to start a vdev-level driver
 *	operation using a wireless_dev instance
 * @wdev: the wireless_dev to start the operation on
 *
 * Return: an operation handle on success, NULL on failure
 */
#define hdd_vdev_op_start_with_wdev(wdev) hdd_vdev_op_start((wdev)->netdev)

struct hdd_vdev_op *__hdd_vdev_op_start(struct net_device *net_dev,
					const char *func);

/**
 * hdd_vdev_op_stop() - stop a given vdev-level operation
 * @op: an operation handle identifying the operation to stop
 *
 * Return: None
 */
#define hdd_vdev_op_stop(op) __hdd_vdev_op_stop(op, __func__)

void __hdd_vdev_op_stop(struct hdd_vdev_op *op, const char *func);

#endif
