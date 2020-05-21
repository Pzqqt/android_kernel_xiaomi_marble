/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (c) 2020, The Linux Foundation. All rights reserved.
 */

#ifndef __SDE_VM_EVENT_H__
#define __SDE_VM_EVENT_H__

#include <linux/list.h>
#include <linux/types.h>
#include <linux/slab.h>
#include <linux/platform_device.h>
#include <drm/drm_device.h>

/**
 * struct msm_vm_ops - hooks for communication with vm clients
 * @vm_pre_hw_release: invoked before releasing the HW
 * @vm_post_hw_acquire: invoked before pushing the first commit
 * @vm_check: invoked to check the readiness of the vm_clients
 *	      before releasing the HW
 */
struct msm_vm_ops {
	int (*vm_pre_hw_release)(void *priv_data);
	int (*vm_post_hw_acquire)(void *priv_data);
	int (*vm_check)(void *priv_data);
};

/**
 * msm_vm_client_entry - defines the vm client info
 * @ops: client vm_ops
 * @dev: clients device id. Used in unregister
 * @data: client custom data
 * @list: linked list entry
 */
struct msm_vm_client_entry {
	struct msm_vm_ops ops;
	struct device *dev;
	void *data;
	struct list_head list;
};

/**
 * msm_register_vm_event - api for display dependent drivers(clients) to
 *                         register for vm events
 * @dev: msm device
 * @client_dev: client device
 * @ops: vm event hooks
 * @priv_data: client custom data
 */
int msm_register_vm_event(struct device *dev, struct device *client_dev,
			  struct msm_vm_ops *ops, void *priv_data);

/**
 * msm_unregister_vm_event - api for display dependent drivers(clients) to
 *                           unregister from vm events
 * @dev: msm device
 * @client_dev: client device
 */
void msm_unregister_vm_event(struct device *dev, struct device *client_dev);

#endif //__SDE_VM_EVENT_H__
