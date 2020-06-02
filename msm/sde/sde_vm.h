/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (c) 2020, The Linux Foundation. All rights reserved.
 */

#ifndef __SDE_VM_H__
#define __SDE_VM_H__

#include "msm_drv.h"

struct sde_kms;

/**
 * sde_vm_irq_entry - VM irq specification
 * @label - VM_IRQ_LABEL assigned by Hyp RM
 * @irq - linux mapped irq number
 */
struct sde_vm_irq_entry {
	u32 label;
	u32 irq;
};

/**
 * sde_vm_irq_desc - list of IRQ's to be handled
 * @n_irq - irq count
 * @irq_entries - list of sde_vm_irq_entry
 */
struct sde_vm_irq_desc {
	u32 n_irq;
	struct sde_vm_irq_entry *irq_entries;
};

enum sde_crtc_vm_req;

/**
 * sde_vm_ops - VM specific function hooks
 */
struct sde_vm_ops {
	/**
	 * vm_acquire - hook to handle HW accept
	 * @kms - handle to sde_kms
	 * @return - return 0 on success
	 */
	int (*vm_acquire)(struct sde_kms *kms);

	/**
	 * vm_release - hook to handle HW release
	 * @kms - handle to sde_kms
	 * @return - return 0 on success
	 */
	int (*vm_release)(struct sde_kms *kms);

	/**
	 * vm_owns_hw - hook to query the HW status of the VM
	 * @kms - handle to sde_kms
	 * @return - return true when vm owns the hw
	 */
	bool (*vm_owns_hw)(struct sde_kms *kms);

	/**
	 * vm_prepare_commit - hook to handle operations before the first
			       commit after acquiring the HW
	 * @sde_kms - handle to sde_kms
	 * @state - global atomic state to be parsed
	 * @return - return 0 on success
	 */
	int (*vm_prepare_commit)(struct sde_kms *sde_kms,
			struct drm_atomic_state *state);

	/**
	 * vm_post_commit - hook to handle operations after
			    last commit before release
	 * @sde_kms - handle to sde_kms
	 * @state - global atomic state to be parsed
	 * @return - return 0 on success
	 */
	int (*vm_post_commit)(struct sde_kms *sde_kms,
			struct drm_atomic_state *state);

	/**
	 * vm_deinit - deinitialize VM layer
	 * @kms - pointer to sde_kms
	 * @ops - primary VM specific ops functions
	 */
	void (*vm_deinit)(struct sde_kms *kms, struct sde_vm_ops *ops);

	/**
	 * vm_check - hook to check with vm_clients for its readiness to release
		      the HW reasources
	 */
	int (*vm_check)(void);

	/**
	 * vm_client_pre_release - hook to invoke vm_client list for pre_release
				   handling
	 * @kms - handle to sde_kms
	 */
	int (*vm_client_pre_release)(struct sde_kms *kms);

	/**
	 * vm_client_post_acquire - hook to invoke vm_client list for
	 *			    post_acquire resource handling
	 * @kms - handle to sde_kms
	 */
	int (*vm_client_post_acquire)(struct sde_kms *kms);

	int (*vm_request_valid)(struct sde_kms *sde_kms,
			enum sde_crtc_vm_req old_state,
			enum sde_crtc_vm_req new_state);
};

/**
 * sde_vm - VM layer descriptor. Abstract for all the VM's
 * @vm_res_lock - mutex to protect resource updates
 * @mem_notificaiton_cookie - Hyp RM notification identifier
 * @n_irq_lent - irq count
 * @io_mem_handle - RM identifier for the IO range
 * @sde_kms - handle to sde_kms
 * @vm_ops - VM operation hooks for respective VM type
 */
struct sde_vm {
	struct mutex vm_res_lock;
	void *mem_notification_cookie;
	atomic_t n_irq_lent;
	int io_mem_handle;
	struct sde_kms *sde_kms;
	struct sde_vm_ops vm_ops;
};

/**
 * sde_vm_primary - VM layer descriptor for Primary VM
 * @base - parent struct object
 * @irq_desc - cache copy of irq list for validating reclaim
 */
struct sde_vm_primary {
	struct sde_vm base;
	struct sde_vm_irq_desc *irq_desc;
};

/**
 * sde_vm_trusted - VM layer descriptor for Trusted VM
 * @base - parent struct object
 * @sgl_desc - hyp RM sgl list descriptor for IO ranges
 * @irq_desc - irq list
 */
struct sde_vm_trusted {
	struct sde_vm base;
	struct sde_vm_irq_desc *irq_desc;
	struct hh_sgl_desc *sgl_desc;
};

#if IS_ENABLED(CONFIG_DRM_SDE_VM)
/**
 * sde_vm_primary_init - Initialize primary VM layer
 * @kms - pointer to sde_kms
 * @return - 0 on success
 */
int sde_vm_primary_init(struct sde_kms *kms);

/**
 * sde_vm_trusted_init - Initialize Trusted VM layer
 * @kms - pointer to sde_kms
 * @ops - primary VM specific ops functions
 * @return - 0 on success
 */
int sde_vm_trusted_init(struct sde_kms *kms);
#else
static inline int sde_vm_primary_init(struct sde_kms *kms)
{
	return 0;
}

static inline int sde_vm_trusted_init(struct sde_kms *kms)
{
	return 0;
}

#endif /* IS_ENABLED(CONFIG_DRM_SDE_VM) */
#endif /* __SDE_VM_H__ */
