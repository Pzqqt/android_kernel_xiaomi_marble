// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2020, The Linux Foundation. All rights reserved.
 */
#include <linux/notifier.h>
#include <linux/haven/hh_rm_drv.h>
#include <linux/haven/hh_irq_lend.h>
#include <linux/haven/hh_mem_notifier.h>
#include "sde_kms.h"
#include "sde_vm.h"
#include "sde_vm_common.h"

#define to_vm_primary(vm) ((struct sde_vm_primary *)vm)

static bool sde_vm_owns_hw(struct sde_kms *sde_kms)
{
	struct sde_vm_primary *sde_vm;
	bool owns_irq, owns_mem_io;

	sde_vm = to_vm_primary(sde_kms->vm);

	owns_irq = !atomic_read(&sde_vm->base.n_irq_lent);
	owns_mem_io = (sde_vm->base.io_mem_handle < 0);

	return (owns_irq & owns_mem_io);
}

void sde_vm_irq_release_notification_handler(void *req,
		unsigned long notif_type, enum hh_irq_label label)
{
	struct sde_vm_primary *sde_vm;
	int rc = 0;

	if (!req) {
		SDE_ERROR("invalid data on release notificaiton\n");
		return;
	}

	sde_vm = to_vm_primary(req);

	mutex_lock(&sde_vm->base.vm_res_lock);

	rc = hh_irq_reclaim(label);
	if (rc) {
		SDE_ERROR("failed to reclaim irq label: %d\n", label);
		goto notify_end;
	}

	/**
	 * Skipping per IRQ label verification since IRQ's are MDSS centric.
	 * Need to enable addition verifications when per-display IRQ's are
	 *  supported.
	 */
	atomic_dec(&sde_vm->base.n_irq_lent);

	SDE_INFO("irq reclaim succeeded for label: %d\n", label);
notify_end:
	mutex_unlock(&sde_vm->base.vm_res_lock);

}

static void sde_vm_mem_release_notification_handler(
		enum hh_mem_notifier_tag tag, unsigned long notif_type,
		void *entry_data, void *notif_msg)
{
	struct hh_rm_notif_mem_released_payload *payload;
	struct sde_vm_primary *sde_vm;
	struct sde_kms *sde_kms;
	int rc = 0;

	if (notif_type != HH_RM_NOTIF_MEM_RELEASED ||
			tag != HH_MEM_NOTIFIER_TAG_DISPLAY)
		return;

	if (!entry_data || !notif_msg)
		return;

	payload = (struct hh_rm_notif_mem_released_payload *)notif_msg;
	sde_vm = (struct sde_vm_primary *)entry_data;
	sde_kms = sde_vm->base.sde_kms;

	mutex_lock(&sde_vm->base.vm_res_lock);

	if (payload->mem_handle != sde_vm->base.io_mem_handle)
		goto notify_end;

	rc = hh_rm_mem_reclaim(payload->mem_handle, 0);
	if (rc) {
		SDE_ERROR("failed to reclaim IO memory, rc=%d\n", rc);
		goto notify_end;
	}

	sde_vm->base.io_mem_handle = -1;

	SDE_INFO("mem reclaim succeeded for tag: %d\n", tag);
notify_end:
	mutex_unlock(&sde_vm->base.vm_res_lock);
}

static int _sde_vm_lend_notify_registers(struct sde_vm *vm,
					 struct msm_io_res *io_res)
{
	struct sde_vm_primary *sde_vm;
	struct hh_acl_desc *acl_desc;
	struct hh_sgl_desc *sgl_desc;
	struct hh_notify_vmid_desc *vmid_desc;
	hh_memparcel_handle_t mem_handle;
	hh_vmid_t trusted_vmid;
	int rc = 0;

	sde_vm = to_vm_primary(vm);

	acl_desc = sde_vm_populate_acl(HH_TRUSTED_VM);
	if (IS_ERR(acl_desc)) {
		SDE_ERROR("failed to populate acl descriptor, rc = %d\n",
			   PTR_ERR(acl_desc));
		return rc;
	}

	sgl_desc = sde_vm_populate_sgl(io_res);
	if (IS_ERR_OR_NULL(sgl_desc)) {
		SDE_ERROR("failed to populate sgl descriptor, rc = %d\n",
			   PTR_ERR(sgl_desc));
		goto sgl_fail;
	}

	rc = hh_rm_mem_lend(HH_RM_MEM_TYPE_IO, 0, SDE_VM_MEM_LABEL,
				 acl_desc, sgl_desc, NULL, &mem_handle);
	if (rc) {
		SDE_ERROR("hyp lend failed with error, rc: %d\n", rc);
		goto fail;
	}

	hh_rm_get_vmid(HH_TRUSTED_VM, &trusted_vmid);

	vmid_desc = sde_vm_populate_vmid(trusted_vmid);

	rc = hh_rm_mem_notify(mem_handle, HH_RM_MEM_NOTIFY_RECIPIENT,
				  HH_MEM_NOTIFIER_TAG_DISPLAY, vmid_desc);
	if (rc) {
		SDE_ERROR("hyp mem notify failed, rc = %d\n", rc);
		goto notify_fail;
	}

	sde_vm->base.io_mem_handle = mem_handle;

	SDE_INFO("IO memory lend suceeded for tag: %d\n",
			HH_MEM_NOTIFIER_TAG_DISPLAY);

notify_fail:
	kfree(vmid_desc);
fail:
	kfree(sgl_desc);
sgl_fail:
	kfree(acl_desc);

	return rc;
}

static int _sde_vm_lend_irq(struct sde_vm *vm, struct msm_io_res *io_res)
{
	struct sde_vm_primary *sde_vm;
	struct sde_vm_irq_desc *irq_desc;
	int i, rc = 0;

	sde_vm = to_vm_primary(vm);

	irq_desc = sde_vm_populate_irq(io_res);

	for (i  = 0; i < irq_desc->n_irq; i++) {
		struct sde_vm_irq_entry *entry = &irq_desc->irq_entries[i];

		rc = hh_irq_lend_v2(entry->label, HH_TRUSTED_VM, entry->irq,
				 sde_vm_irq_release_notification_handler,
				 sde_vm);
		if (rc) {
			SDE_ERROR("irq lend failed for irq label: %d, rc=%d\n",
				  entry->label, rc);
			hh_irq_reclaim(entry->label);
			return rc;
		}

		rc = hh_irq_lend_notify(entry->label);
		if (rc) {
			SDE_ERROR("irq lend notify failed, label: %d, rc=%d\n",
				entry->label, rc);
			hh_irq_reclaim(entry->label);
			return rc;
		}

		SDE_INFO("vm lend suceeded for IRQ label: %d\n", entry->label);
	}

	// cache the irq list for validation during release
	sde_vm->irq_desc = irq_desc;
	atomic_set(&sde_vm->base.n_irq_lent, sde_vm->irq_desc->n_irq);

	return rc;
}

static int _sde_vm_release(struct sde_kms *kms)
{
	struct msm_io_res io_res;
	struct sde_vm_primary *sde_vm;
	int rc = 0;

	if (!kms->vm)
		return 0;

	sde_vm = to_vm_primary(kms->vm);

	INIT_LIST_HEAD(&io_res.mem);
	INIT_LIST_HEAD(&io_res.irq);

	rc = sde_vm_get_resources(kms, &io_res);
	if (rc) {
		SDE_ERROR("fail to get resources\n");
		goto assign_fail;
	}

	mutex_lock(&sde_vm->base.vm_res_lock);

	rc = _sde_vm_lend_notify_registers(kms->vm, &io_res);
	if (rc) {
		SDE_ERROR("fail to lend notify resources\n");
		goto assign_fail;
	}

	rc = _sde_vm_lend_irq(kms->vm, &io_res);
	if (rc) {
		SDE_ERROR("failed to lend irq's\n");
		goto assign_fail;
	}
assign_fail:
	sde_vm_free_resources(&io_res);
	mutex_unlock(&sde_vm->base.vm_res_lock);

	return rc;
}

static void _sde_vm_deinit(struct sde_kms *sde_kms, struct sde_vm_ops *ops)
{
	struct sde_vm_primary *sde_vm;

	if (!sde_kms->vm)
		return;

	memset(ops, 0, sizeof(*ops));

	sde_vm = to_vm_primary(sde_kms->vm);

	if (sde_vm->base.mem_notification_cookie)
		hh_mem_notifier_unregister(
				sde_vm->base.mem_notification_cookie);

	if (sde_vm->irq_desc)
		sde_vm_free_irq(sde_vm->irq_desc);

	kfree(sde_vm);
}

static void _sde_vm_set_ops(struct sde_vm_ops *ops)
{
	memset(ops, 0, sizeof(*ops));

	ops->vm_client_pre_release = sde_vm_pre_release;
	ops->vm_client_post_acquire = sde_vm_post_acquire;
	ops->vm_release = _sde_vm_release;
	ops->vm_owns_hw = sde_vm_owns_hw;
	ops->vm_deinit = _sde_vm_deinit;
	ops->vm_prepare_commit = sde_kms_vm_primary_prepare_commit;
	ops->vm_post_commit = sde_kms_vm_primary_post_commit;
	ops->vm_request_valid = sde_vm_request_valid;
}

int sde_vm_primary_init(struct sde_kms *kms)
{
	struct sde_vm_primary *sde_vm;
	void *cookie;
	int rc = 0;

	sde_vm = kzalloc(sizeof(*sde_vm), GFP_KERNEL);
	if (!sde_vm)
		return -ENOMEM;

	_sde_vm_set_ops(&sde_vm->base.vm_ops);

	cookie = hh_mem_notifier_register(HH_MEM_NOTIFIER_TAG_DISPLAY,
			       sde_vm_mem_release_notification_handler, sde_vm);
	if (!cookie) {
		SDE_ERROR("fails to register RM mem release notifier\n");
		rc = -EINVAL;
		goto init_fail;
	}

	sde_vm->base.mem_notification_cookie = cookie;
	sde_vm->base.sde_kms = kms;
	sde_vm->base.io_mem_handle = -1; // 0 is a valid handle
	kms->vm = &sde_vm->base;

	mutex_init(&sde_vm->base.vm_res_lock);

	return 0;
init_fail:
	_sde_vm_deinit(kms, &sde_vm->base.vm_ops);

	return rc;
}
