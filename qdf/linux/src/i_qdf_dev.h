/*
 * Copyright (c) 2019-2021 The Linux Foundation. All rights reserved.
 * Copyright (c) 2024 Qualcomm Innovation Center, Inc. All rights reserved.
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

/**
 * DOC: qdf_dev
 * QCA driver framework (QDF) device management APIs
 */

#if !defined(__I_QDF_DEV_H)
#define __I_QDF_DEV_H

/* Include Files */
#include <qdf_types.h>
#include "qdf_util.h"
#include <linux/irq.h>
#ifdef CONFIG_SCHED_CORE_CTL
#include <linux/sched/core_ctl.h>
#endif

struct qdf_cpu_mask;
struct qdf_devm;
struct qdf_dev;

#define __qdf_cpumask_pr_args(maskp) cpumask_pr_args(maskp)
#define __qdf_for_each_possible_cpu(cpu) for_each_possible_cpu(cpu)
#define __qdf_for_each_online_cpu(cpu) for_each_online_cpu(cpu)
#define __qdf_for_each_cpu(cpu, maskp) \
for_each_cpu(cpu, maskp)

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(6, 2, 0))
#define __qdf_for_each_cpu_not(cpu, maskp) \
for_each_cpu_andnot(cpu, cpu_possible_mask, maskp)
#else
#define __qdf_for_each_cpu_not(cpu, maskp) \
for_each_cpu_not(cpu, maskp)
#endif

/**
 * __qdf_dev_alloc_mem() - allocate memory
 * @qdfdev: Device handle
 * @mrptr: Pointer to the allocated memory
 * @reqsize: Allocation request in bytes
 * @mask: Property mask to be associated to the allocated memory
 *
 * This function will acquire memory to be associated with a device
 *
 * Return: QDF_STATUS_SUCCESS on success
 */
static inline QDF_STATUS
__qdf_dev_alloc_mem(struct qdf_dev *qdfdev, struct qdf_devm **mrptr,
		    uint32_t reqsize, uint32_t mask)
{
	*mrptr = devm_kzalloc((struct device *)qdfdev, reqsize, mask);

	if (!*mrptr)
		return QDF_STATUS_E_NOMEM;

	return QDF_STATUS_SUCCESS;
}

/**
 * __qdf_dev_release_mem() - release memory
 * @qdfdev: Device handle
 * @mrptr: Pointer to the allocated memory
 *
 * This function will acquire memory to be associated with a device
 *
 * Return: QDF_STATUS_SUCCESS on success
 */
static inline QDF_STATUS
__qdf_dev_release_mem(struct qdf_dev *qdfdev, struct qdf_devm *mrptr)
{
	devm_kfree((struct device *)qdfdev, mrptr);

	return QDF_STATUS_SUCCESS;
}

/**
 * __qdf_dev_modify_irq() - modify irq
 * @irnum: irq number
 * @cmask: Bitmap to be cleared for the property mask
 * @smask: Bitmap to be set for the property mask
 *
 * This function will modify the properties of the irq associated with a device
 *
 * Return: QDF_STATUS_SUCCESS on success
 */
static inline QDF_STATUS
__qdf_dev_modify_irq_status(uint32_t irnum, unsigned long cmask,
			    unsigned long smask)
{
	irq_modify_status(irnum, cmask, smask);

	return QDF_STATUS_SUCCESS;
}

/**
 * __qdf_dev_set_irq_affinity() - set irq affinity
 * @irnum: irq number
 * @cpmask: cpu affinity bitmap
 *
 * This function will set the affinity level for an irq
 *
 * Return: QDF_STATUS_SUCCESS on success
 */
static inline QDF_STATUS
__qdf_dev_set_irq_affinity(uint32_t irnum, struct qdf_cpu_mask *cpmask)
{
	int ret;

	ret = irq_set_affinity_hint(irnum, (struct cpumask *)cpmask);
	return qdf_status_from_os_return(ret);
}

/**
 * __qdf_topology_physical_package_id() - API to retrieve the
 * cluster info
 * @cpu: cpu core
 *
 * This function returns the cluster information for give cpu
 * core
 *
 * Return: 1 for perf and 0 for non-perf cluster
 */
static inline int __qdf_topology_physical_package_id(unsigned int cpu)
{
	return topology_physical_package_id(cpu);
}

/**
 * __qdf_cpumask_subset() - API to check for subset in cpumasks
 * @srcp1: first cpu mask
 * @srcp1: second cpu mask
 *
 * This checks for *srcp1 & ~*srcp2
 *
 * Return: 1 if srcp1 is subset of srcp2 else 0
 */
static inline int __qdf_cpumask_subset(qdf_cpu_mask *srcp1,
				       const qdf_cpu_mask *srcp2)
{
	return cpumask_subset(srcp1, srcp2);
}

/**
 * __qdf_cpumask_intersects() - API to check if cpumasks
 * intersect
 * @srcp1: first cpu mask
 * @srcp1: second cpu mask
 *
 * This checks for (*srcp1 & *srcp2) != 0
 *
 * Return: 1 if srcp1 and srcp2 intersect else 0
 */
static inline int __qdf_cpumask_intersects(qdf_cpu_mask *srcp1,
					   const qdf_cpu_mask *srcp2)
{
	return cpumask_intersects(srcp1, srcp2);
}

#ifdef CONFIG_SCHED_CORE_CTL
/**
 * __qdf_core_ctl_set_boost() - This API is used to move tasks
 * to CPUs with higher capacity
 *
 * This function moves tasks to higher capacity CPUs than those
 * where the tasks would have  normally ended up. This is
 * applicable only to defconfig builds.
 *
 * Return: 0 on success
 */
static inline int __qdf_core_ctl_set_boost(bool boost)
{
	return core_ctl_set_boost(boost);
}
#else
static inline int __qdf_core_ctl_set_boost(bool boost)
{
	return 0;
}
#endif

/**
 * __qdf_dev_set_irq_status_flags() - set irq status flags
 * @irnum: irq number
 * @set: status flag to set
 *
 * This function will set the status for an irq
 *
 * Return: QDF_STATUS_SUCCESS on success
 */
static inline QDF_STATUS
__qdf_dev_set_irq_status_flags(unsigned int irnum, unsigned long set)
{
	irq_set_status_flags(irnum, set);

	return QDF_STATUS_SUCCESS;
}

/**
 * __qdf_dev_clear_irq_status_flags() - clear irq status flags
 * @irnum: irq number
 * @clear: status flag to clear
 *
 * This function will set the status for an irq
 *
 * Return: QDF_STATUS_SUCCESS on success
 */
static inline QDF_STATUS
__qdf_dev_clear_irq_status_flags(unsigned int irnum, unsigned long clr)
{
	irq_clear_status_flags(irnum, clr);

	return QDF_STATUS_SUCCESS;
}

#endif /* __I_QDF_DEV_H */
