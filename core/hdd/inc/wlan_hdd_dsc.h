/*
 * Copyright (c) 2018-2019 The Linux Foundation. All rights reserved.
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

#include "qdf_types.h"
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
 * struct hdd_psoc_sync - opaque synchronization handle for a psoc
 */
struct hdd_psoc_sync;

/**
 * hdd_psoc_sync_create() - create a psoc synchronization context
 * @dsc_driver: parent dsc_driver to the psoc
 * @out_psoc_sync: out parameter for the new synchronization context
 *
 * Return: Errno
 */
qdf_must_check int
hdd_psoc_sync_create(struct dsc_driver *dsc_driver,
		     struct hdd_psoc_sync **out_psoc_sync);

/**
 * hdd_psoc_sync_create_with_trans() - create a psoc synchronization context
 * @dsc_driver: parent dsc_driver to the psoc
 * @out_psoc_sync: out parameter for the new synchronization context
 *
 * For protecting the device creation process.
 *
 * Return: Errno
 */
#define hdd_psoc_sync_create_with_trans(dsc_driver, out_psoc_sync) \
	__hdd_psoc_sync_create_with_trans(dsc_driver, out_psoc_sync, __func__)

qdf_must_check int
__hdd_psoc_sync_create_with_trans(struct dsc_driver *dsc_driver,
				  struct hdd_psoc_sync **out_psoc_sync,
				  const char *desc);

/**
 * hdd_psoc_sync_destroy() - destroy a psoc synchronization context
 * @psoc_sync: the context to destroy
 *
 * Return: none
 */
void hdd_psoc_sync_destroy(struct hdd_psoc_sync *psoc_sync);

/**
 * hdd_psoc_sync_register() - register a psoc for operations/transitions
 * @dev: the device to use as the operation/transition lookup key
 * @psoc_sync: the psoc synchronization context to register
 *
 * Return: none
 */
void hdd_psoc_sync_register(struct device *dev,
			    struct hdd_psoc_sync *psoc_sync);

/**
 * hdd_psoc_sync_unregister() - unregister a psoc for operations/transitions
 * @dev: the device originally used to register the psoc_sync context
 *
 * Return: the psoc synchronization context that was registered for @dev
 */
struct hdd_psoc_sync *hdd_psoc_sync_unregister(struct device *dev);

/**
 * hdd_psoc_sync_trans_start() - attempt to start a transition on @dev
 * @dev: the device to transition
 * @out_psoc_sync: out parameter for the synchronization context registered with
 *	@dev, populated on success
 *
 * Return: Errno
 */
#define hdd_psoc_sync_trans_start(dev, out_psoc_sync) \
	__hdd_psoc_sync_trans_start(dev, out_psoc_sync, __func__)

qdf_must_check int
__hdd_psoc_sync_trans_start(struct device *dev,
			    struct hdd_psoc_sync **out_psoc_sync,
			    const char *desc);

/**
 * hdd_psoc_sync_trans_start_wait() - attempt to start a transition on @dev,
 *	blocking if a conflicting transition is in flight
 * @dev: the device to transition
 * @out_psoc_sync: out parameter for the synchronization context registered with
 *	@dev, populated on success
 *
 * Return: Errno
 */
#define hdd_psoc_sync_trans_start_wait(dev, out_psoc_sync) \
	__hdd_psoc_sync_trans_start_wait(dev, out_psoc_sync, __func__)

qdf_must_check int
__hdd_psoc_sync_trans_start_wait(struct device *dev,
				 struct hdd_psoc_sync **out_psoc_sync,
				 const char *desc);

/**
 * hdd_psoc_sync_trans_resume() - resume a transition on @dev
 * @dev: the device under transition
 * @out_psoc_sync: out parameter for the synchronization context registered with
 *	@dev, populated on success
 *
 * Return: Errno
 */
int hdd_psoc_sync_trans_resume(struct device *dev,
			       struct hdd_psoc_sync **out_psoc_sync);

/**
 * hdd_psoc_sync_trans_stop() - stop a transition associated with @psoc_sync
 * @psoc_sync: the synchonization context tracking the transition
 *
 * Return: none
 */
void hdd_psoc_sync_trans_stop(struct hdd_psoc_sync *psoc_sync);

/**
 * hdd_psoc_sync_assert_trans_protected() - assert that @dev is currently
 *	protected by a transition
 * @dev: the device to check
 *
 * Return: none
 */
void hdd_psoc_sync_assert_trans_protected(struct device *dev);

/**
 * hdd_psoc_sync_op_start() - attempt to start an operation on @dev
 * @dev: the device to operate against
 * @out_psoc_sync: out parameter for the synchronization context registered with
 *	@dev, populated on success
 *
 * Return: Errno
 */
#define hdd_psoc_sync_op_start(dev, out_psoc_sync) \
	__hdd_psoc_sync_op_start(dev, out_psoc_sync, __func__)

qdf_must_check int
__hdd_psoc_sync_op_start(struct device *dev,
			 struct hdd_psoc_sync **out_psoc_sync,
			 const char *func);

/**
 * hdd_psoc_sync_op_stop() - stop an operation associated with @psoc_sync
 * @psoc_sync: the synchonization context tracking the operation
 *
 * Return: none
 */
#define hdd_psoc_sync_op_stop(dev) \
	__hdd_psoc_sync_op_stop(dev, __func__)

void __hdd_psoc_sync_op_stop(struct hdd_psoc_sync *psoc_sync,
			     const char *func);

/**
 * hdd_psoc_sync_wait_for_ops() - wait until all @psoc_sync operations complete
 * @psoc_sync: the synchonization context tracking the operations
 *
 * Return: None
 */
void hdd_psoc_sync_wait_for_ops(struct hdd_psoc_sync *psoc_sync);

/**
 * struct hdd_vdev_sync - opaque synchronization handle for a vdev
 */
struct hdd_vdev_sync;

/**
 * hdd_vdev_sync_create() - create a vdev synchronization context
 * @wiphy: parent wiphy to the vdev
 * @out_vdev_sync: out parameter for the new synchronization context
 *
 * Return: Errno
 */
qdf_must_check int
hdd_vdev_sync_create(struct wiphy *wiphy, struct hdd_vdev_sync **out_vdev_sync);

/**
 * hdd_vdev_sync_create_with_trans() - create a vdev synchronization context
 * @wiphy: parent wiphy to the vdev
 * @out_vdev_sync: out parameter for the new synchronization context
 *
 * For protecting the net_device creation process.
 *
 * Return: Errno
 */
#define hdd_vdev_sync_create_with_trans(wiphy, out_vdev_sync) \
	__hdd_vdev_sync_create_with_trans(wiphy, out_vdev_sync, __func__)

qdf_must_check int
__hdd_vdev_sync_create_with_trans(struct wiphy *wiphy,
				  struct hdd_vdev_sync **out_vdev_sync,
				  const char *desc);

/**
 * hdd_vdev_sync_destroy() - destroy a vdev synchronization context
 * @vdev_sync: the context to destroy
 *
 * Return: none
 */
void hdd_vdev_sync_destroy(struct hdd_vdev_sync *vdev_sync);

/**
 * hdd_vdev_sync_register() - register a vdev for operations/transitions
 * @net_dev: the net_device to use as the operation/transition lookup key
 * @vdev_sync: the vdev synchronization context to register
 *
 * Return: none
 */
void hdd_vdev_sync_register(struct net_device *net_dev,
			    struct hdd_vdev_sync *vdev_sync);

/**
 * hdd_vdev_sync_unregister() - unregister a vdev for operations/transitions
 * @net_dev: the net_device originally used to register the vdev_sync context
 *
 * Return: the vdev synchronization context that was registered for @net_dev
 */
struct hdd_vdev_sync *hdd_vdev_sync_unregister(struct net_device *net_dev);

/**
 * hdd_vdev_sync_trans_start() - attempt to start a transition on @net_dev
 * @net_dev: the net_device to transition
 * @out_vdev_sync: out parameter for the synchronization context registered with
 *	@net_dev, populated on success
 *
 * Return: Errno
 */
#define hdd_vdev_sync_trans_start(net_dev, out_vdev_sync) \
	__hdd_vdev_sync_trans_start(net_dev, out_vdev_sync, __func__)

qdf_must_check int
__hdd_vdev_sync_trans_start(struct net_device *net_dev,
			    struct hdd_vdev_sync **out_vdev_sync,
			    const char *desc);

/**
 * hdd_vdev_sync_trans_start_wait() - attempt to start a transition on @net_dev,
 *	blocking if a conflicting transition is in flight
 * @net_dev: the net_device to transition
 * @out_vdev_sync: out parameter for the synchronization context registered with
 *	@net_dev, populated on success
 *
 * Return: Errno
 */
#define hdd_vdev_sync_trans_start_wait(net_dev, out_vdev_sync) \
	__hdd_vdev_sync_trans_start_wait(net_dev, out_vdev_sync, __func__)

qdf_must_check int
__hdd_vdev_sync_trans_start_wait(struct net_device *net_dev,
				 struct hdd_vdev_sync **out_vdev_sync,
				 const char *desc);

/**
 * hdd_vdev_sync_trans_stop() - stop a transition associated with @vdev_sync
 * @vdev_sync: the synchonization context tracking the transition
 *
 * Return: none
 */
void hdd_vdev_sync_trans_stop(struct hdd_vdev_sync *vdev_sync);

/**
 * hdd_vdev_sync_assert_trans_protected() - assert that @net_dev is currently
 *	protected by a transition
 * @net_dev: the net_device to check
 *
 * Return: none
 */
void hdd_vdev_sync_assert_trans_protected(struct net_device *net_dev);

/**
 * hdd_vdev_sync_op_start() - attempt to start an operation on @net_dev
 * @net_dev: the net_device to operate against
 * @out_vdev_sync: out parameter for the synchronization context registered with
 *	@net_dev, populated on success
 *
 * Return: Errno
 */
#define hdd_vdev_sync_op_start(net_dev, out_vdev_sync) \
	__hdd_vdev_sync_op_start(net_dev, out_vdev_sync, __func__)

qdf_must_check int
__hdd_vdev_sync_op_start(struct net_device *net_dev,
			 struct hdd_vdev_sync **out_vdev_sync,
			 const char *func);

/**
 * hdd_vdev_sync_op_stop() - stop an operation associated with @vdev_sync
 * @vdev_sync: the synchonization context tracking the operation
 *
 * Return: none
 */
#define hdd_vdev_sync_op_stop(net_dev) \
	__hdd_vdev_sync_op_stop(net_dev, __func__)

void __hdd_vdev_sync_op_stop(struct hdd_vdev_sync *vdev_sync,
			     const char *func);

/**
 * hdd_vdev_sync_wait_for_ops() - wait until all @vdev_sync operations complete
 * @vdev_sync: the synchonization context tracking the operations
 *
 * Return: None
 */
void hdd_vdev_sync_wait_for_ops(struct hdd_vdev_sync *vdev_sync);

#endif
