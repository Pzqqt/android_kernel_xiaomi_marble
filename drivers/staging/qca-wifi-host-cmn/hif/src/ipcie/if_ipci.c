/*
 * Copyright (c) 2013-2021 The Linux Foundation. All rights reserved.
 * Copyright (c) 2021-2022 Qualcomm Innovation Center, Inc. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.

 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <linux/slab.h>
#include <linux/interrupt.h>
#include <linux/if_arp.h>
#include "hif_io32.h"
#include "if_ipci.h"
#include "hif.h"
#include "target_type.h"
#include "hif_main.h"
#include "ce_main.h"
#include "ce_api.h"
#include "ce_internal.h"
#include "ce_reg.h"
#include "ce_bmi.h"
#include "regtable.h"
#include "hif_hw_version.h"
#include <linux/debugfs.h>
#include <linux/seq_file.h>
#include "qdf_status.h"
#include "qdf_atomic.h"
#include "pld_common.h"
#include "mp_dev.h"
#include "hif_debug.h"

#include "ce_tasklet.h"
#include "targaddrs.h"
#include "hif_exec.h"

#include "ipci_api.h"

#ifdef FEATURE_RUNTIME_PM
inline struct hif_runtime_pm_ctx *hif_ipci_get_rpm_ctx(struct hif_softc *scn)
{
	struct hif_ipci_softc *sc = HIF_GET_IPCI_SOFTC(scn);

	return &sc->rpm_ctx;
}

inline struct device *hif_ipci_get_dev(struct hif_softc *scn)
{
	struct hif_ipci_softc *sc = HIF_GET_IPCI_SOFTC(scn);

	return sc->dev;
}
#endif

void hif_ipci_enable_power_management(struct hif_softc *hif_sc,
				      bool is_packet_log_enabled)
{
	hif_pm_runtime_start(hif_sc);
}

void hif_ipci_disable_power_management(struct hif_softc *hif_ctx)
{
	hif_pm_runtime_stop(hif_ctx);
}

void hif_ipci_display_stats(struct hif_softc *hif_ctx)
{
	hif_display_ce_stats(hif_ctx);
}

void hif_ipci_clear_stats(struct hif_softc *hif_ctx)
{
	struct hif_ipci_softc *ipci_ctx = HIF_GET_IPCI_SOFTC(hif_ctx);

	if (!ipci_ctx) {
		hif_err("hif_ctx null");
		return;
	}
	hif_clear_ce_stats(&ipci_ctx->ce_sc);
}

QDF_STATUS hif_ipci_open(struct hif_softc *hif_ctx, enum qdf_bus_type bus_type)
{
	struct hif_ipci_softc *sc = HIF_GET_IPCI_SOFTC(hif_ctx);

	hif_ctx->bus_type = bus_type;
	hif_pm_runtime_open(hif_ctx);

	qdf_spinlock_create(&sc->irq_lock);

	return hif_ce_open(hif_ctx);
}

/**
 * hif_ce_msi_map_ce_to_irq() - map CE to IRQ
 * @scn: hif context
 * @ce_id: CE Id
 *
 * Return: IRQ number
 */
static int hif_ce_msi_map_ce_to_irq(struct hif_softc *scn, int ce_id)
{
	struct hif_ipci_softc *ipci_scn = HIF_GET_IPCI_SOFTC(scn);

	return ipci_scn->ce_msi_irq_num[ce_id];
}

int hif_ipci_bus_configure(struct hif_softc *hif_sc)
{
	int status = 0;
	uint8_t wake_ce_id;

	hif_ce_prepare_config(hif_sc);

	status = hif_wlan_enable(hif_sc);
	if (status) {
		hif_err("hif_wlan_enable error = %d", status);
		return status;
	}

	A_TARGET_ACCESS_LIKELY(hif_sc);

	status = hif_config_ce(hif_sc);
	if (status)
		goto disable_wlan;

	status = hif_get_wake_ce_id(hif_sc, &wake_ce_id);
	if (status)
		goto unconfig_ce;

	status = hif_configure_irq(hif_sc);
	if (status < 0)
		goto unconfig_ce;

	hif_sc->wake_irq = hif_ce_msi_map_ce_to_irq(hif_sc, wake_ce_id);
	hif_sc->wake_irq_type = HIF_PM_CE_WAKE;

	hif_info("expecting wake from ce %d, irq %d",
		 wake_ce_id, hif_sc->wake_irq);

	A_TARGET_ACCESS_UNLIKELY(hif_sc);

	return status;

unconfig_ce:
	hif_unconfig_ce(hif_sc);
disable_wlan:
	A_TARGET_ACCESS_UNLIKELY(hif_sc);
	hif_wlan_disable(hif_sc);

	hif_err("Failed, status = %d", status);
	return status;
}

void hif_ipci_close(struct hif_softc *hif_sc)
{
	hif_pm_runtime_close(hif_sc);
	hif_ce_close(hif_sc);
}

/**
 * hif_ce_srng_msi_free_irq(): free CE msi IRQ
 * @scn: struct hif_softc
 *
 * Return: ErrorNo
 */
static int hif_ce_srng_msi_free_irq(struct hif_softc *scn)
{
	int ret;
	int ce_id, irq;
	uint32_t msi_data_start;
	uint32_t msi_data_count;
	uint32_t msi_irq_start;
	struct HIF_CE_state *ce_sc = HIF_GET_CE_STATE(scn);

	ret = pld_get_user_msi_assignment(scn->qdf_dev->dev, "CE",
					  &msi_data_count, &msi_data_start,
					  &msi_irq_start);
	if (ret)
		return ret;

	/* needs to match the ce_id -> irq data mapping
	 * used in the srng parameter configuration
	 */
	for (ce_id = 0; ce_id < scn->ce_count; ce_id++) {
		unsigned int msi_data;

		if (!ce_sc->tasklets[ce_id].inited)
			continue;

		msi_data = (ce_id % msi_data_count) + msi_irq_start;
		irq = pld_get_msi_irq(scn->qdf_dev->dev, msi_data);

		hif_ce_irq_remove_affinity_hint(irq);

		hif_debug("%s: (ce_id %d, msi_data %d, irq %d)", __func__,
			  ce_id, msi_data, irq);

		pfrm_free_irq(scn->qdf_dev->dev, irq, &ce_sc->tasklets[ce_id]);
	}

	return ret;
}

/**
 * hif_ipci_deconfigure_grp_irq(): deconfigure HW block IRQ
 * @scn: struct hif_softc
 *
 * Return: none
 */
void hif_ipci_deconfigure_grp_irq(struct hif_softc *scn)
{
	int i, j, irq;
	struct HIF_CE_state *hif_state = HIF_GET_CE_STATE(scn);
	struct hif_exec_context *hif_ext_group;

	for (i = 0; i < hif_state->hif_num_extgroup; i++) {
		hif_ext_group = hif_state->hif_ext_group[i];
		if (hif_ext_group->irq_requested) {
			hif_ext_group->irq_requested = false;
			for (j = 0; j < hif_ext_group->numirq; j++) {
				irq = hif_ext_group->os_irq[j];
				pfrm_free_irq(scn->qdf_dev->dev,
					      irq, hif_ext_group);
			}
			hif_ext_group->numirq = 0;
		}
	}
}

void hif_ipci_nointrs(struct hif_softc *scn)
{
	int ret;
	struct HIF_CE_state *hif_state = HIF_GET_CE_STATE(scn);

	scn->free_irq_done = true;
	ce_unregister_irq(hif_state, CE_ALL_BITMAP);

	if (scn->request_irq_done == false)
		return;

	hif_ipci_deconfigure_grp_irq(scn);

	ret = hif_ce_srng_msi_free_irq(scn);

	scn->request_irq_done = false;
}

void hif_ipci_disable_bus(struct hif_softc *scn)
{
	struct hif_ipci_softc *sc = HIF_GET_IPCI_SOFTC(scn);
	void __iomem *mem;

	/* Attach did not succeed, all resources have been
	 * freed in error handler
	 */
	if (!sc)
		return;

	mem = (void __iomem *)sc->mem;
	if (mem) {
		hif_dump_pipe_debug_count(scn);
		if (scn->athdiag_procfs_inited) {
			athdiag_procfs_remove();
			scn->athdiag_procfs_inited = false;
		}
		scn->mem = NULL;
	}
	hif_info("X");
}

#ifdef CONFIG_PLD_PCIE_CNSS
void hif_ipci_prevent_linkdown(struct hif_softc *scn, bool flag)
{
	int errno;

	hif_info("wlan: %s pcie power collapse", flag ? "disable" : "enable");
	hif_runtime_prevent_linkdown(scn, flag);

	errno = pld_wlan_pm_control(scn->qdf_dev->dev, flag);
	if (errno)
		hif_err("Failed pld_wlan_pm_control; errno %d", errno);
}
#else
void hif_ipci_prevent_linkdown(struct hif_softc *scn, bool flag)
{
}
#endif

int hif_ipci_bus_suspend(struct hif_softc *scn)
{
	int ret;

	ret = hif_apps_disable_irqs_except_wake_irq(GET_HIF_OPAQUE_HDL(scn));
	if (ret) {
		hif_err("Failed to disable IRQs");
		goto disable_irq_fail;
	}

	ret = hif_apps_enable_irq_wake(GET_HIF_OPAQUE_HDL(scn));
	if (ret) {
		hif_err("Failed to enable Wake-IRQ");
		goto enable_wake_irq_fail;
	}

	if (QDF_IS_STATUS_ERROR(hif_try_complete_tasks(scn))) {
		hif_err("hif_try_complete_tasks timed-out, so abort suspend");
		ret = -EBUSY;
		goto drain_tasks_fail;
	}

	/*
	 * In an unlikely case, if draining becomes infinite loop,
	 * it returns an error, shall abort the bus suspend.
	 */
	ret = hif_drain_fw_diag_ce(scn);
	if (ret) {
		hif_err("draining fw_diag_ce goes infinite, so abort suspend");
		goto drain_tasks_fail;
	}

	scn->bus_suspended = true;

	return 0;

drain_tasks_fail:
	hif_apps_disable_irq_wake(GET_HIF_OPAQUE_HDL(scn));

enable_wake_irq_fail:
	hif_apps_enable_irqs_except_wake_irq(GET_HIF_OPAQUE_HDL(scn));

disable_irq_fail:
	return ret;
}

int hif_ipci_bus_resume(struct hif_softc *scn)
{
	int ret = 0;

	ret = hif_apps_disable_irq_wake(GET_HIF_OPAQUE_HDL(scn));
	if (ret) {
		hif_err("Failed to disable Wake-IRQ");
		goto fail;
	}

	ret = hif_apps_enable_irqs_except_wake_irq(GET_HIF_OPAQUE_HDL(scn));
	if (ret)
		hif_err("Failed to enable IRQs");

	scn->bus_suspended = false;

fail:
	return ret;
}

int hif_ipci_bus_suspend_noirq(struct hif_softc *scn)
{
	/*
	 * If it is system suspend case and wake-IRQ received
	 * just before Kernel issuing suspend_noirq, that must
	 * have scheduled CE2 tasklet, so suspend activity can
	 * be aborted.
	 * Similar scenario for runtime suspend case, would be
	 * handled by hif_pm_runtime_check_and_request_resume
	 * in hif_ce_interrupt_handler.
	 *
	 */
	if (!hif_pm_runtime_get_monitor_wake_intr(GET_HIF_OPAQUE_HDL(scn)) &&
	    hif_get_num_active_tasklets(scn)) {
		hif_err("Tasklets are pending, abort sys suspend_noirq");
		return -EBUSY;
	}

	return 0;
}

int hif_ipci_bus_resume_noirq(struct hif_softc *scn)
{
	return 0;
}

void hif_ipci_disable_isr(struct hif_softc *scn)
{
	struct hif_ipci_softc *sc = HIF_GET_IPCI_SOFTC(scn);

	hif_exec_kill(&scn->osc);
	hif_nointrs(scn);
	/* Cancel the pending tasklet */
	ce_tasklet_kill(scn);
	tasklet_kill(&sc->intr_tq);
	qdf_atomic_set(&scn->active_tasklet_cnt, 0);
	qdf_atomic_set(&scn->active_grp_tasklet_cnt, 0);
}

int hif_ipci_dump_registers(struct hif_softc *hif_ctx)
{
	int status;
	struct hif_softc *scn = HIF_GET_SOFTC(hif_ctx);

	status = hif_dump_ce_registers(scn);

	if (status)
		hif_err("Dump CE Registers Failed");

	return 0;
}

/**
 * hif_ce_interrupt_handler() - interrupt handler for copy engine
 * @irq: irq number
 * @context: tasklet context
 *
 * Return: irqreturn_t
 */
static irqreturn_t hif_ce_interrupt_handler(int irq, void *context)
{
	struct ce_tasklet_entry *tasklet_entry = context;

	hif_pm_runtime_check_and_request_resume(
			GET_HIF_OPAQUE_HDL(tasklet_entry->hif_ce_state));
	return ce_dispatch_interrupt(tasklet_entry->ce_id, tasklet_entry);
}

extern const char *ce_name[];

/* hif_ce_srng_msi_irq_disable() - disable the irq for msi
 * @hif_sc: hif context
 * @ce_id: which ce to disable copy complete interrupts for
 *
 * @Return: none
 */
static void hif_ce_srng_msi_irq_disable(struct hif_softc *hif_sc, int ce_id)
{
	pfrm_disable_irq_nosync(hif_sc->qdf_dev->dev,
				hif_ce_msi_map_ce_to_irq(hif_sc, ce_id));

}

/* hif_ce_srng_msi_irq_enable() - enable the irq for msi
 * @hif_sc: hif context
 * @ce_id: which ce to enable copy complete interrupts for
 *
 * @Return: none
 */
static void hif_ce_srng_msi_irq_enable(struct hif_softc *hif_sc, int ce_id)
{
	pfrm_enable_irq(hif_sc->qdf_dev->dev,
			hif_ce_msi_map_ce_to_irq(hif_sc, ce_id));

}

/* hif_ce_msi_configure_irq() - configure the irq
 * @scn: hif context
 *
 * @Return: none
 */
static int hif_ce_msi_configure_irq(struct hif_softc *scn)
{
	int ret;
	int ce_id, irq;
	uint32_t msi_data_start;
	uint32_t msi_data_count;
	uint32_t msi_irq_start;
	struct HIF_CE_state *ce_sc = HIF_GET_CE_STATE(scn);
	struct hif_ipci_softc *ipci_sc = HIF_GET_IPCI_SOFTC(scn);
	uint8_t wake_ce_id;

	ret = hif_get_wake_ce_id(scn, &wake_ce_id);
	if (ret)
		return ret;

	/* do ce irq assignments */
	ret = pld_get_user_msi_assignment(scn->qdf_dev->dev, "CE",
					  &msi_data_count, &msi_data_start,
					  &msi_irq_start);
	if (ret)
		return ret;

	scn->bus_ops.hif_irq_disable = &hif_ce_srng_msi_irq_disable;
	scn->bus_ops.hif_irq_enable = &hif_ce_srng_msi_irq_enable;
	scn->bus_ops.hif_map_ce_to_irq = &hif_ce_msi_map_ce_to_irq;

	/* needs to match the ce_id -> irq data mapping
	 * used in the srng parameter configuration
	 */
	for (ce_id = 0; ce_id < scn->ce_count; ce_id++) {
		unsigned long irqflags = IRQF_SHARED;
		unsigned int msi_data = (ce_id % msi_data_count) +
			msi_irq_start;
		irq = pld_get_msi_irq(scn->qdf_dev->dev, msi_data);
		hif_debug("(ce_id %d, msi_data %d, irq %d tasklet %pK)",
			 ce_id, msi_data, irq,
			 &ce_sc->tasklets[ce_id]);

		/* implies the ce is also initialized */
		if (!ce_sc->tasklets[ce_id].inited)
			continue;

		ipci_sc->ce_msi_irq_num[ce_id] = irq;
		ret = pfrm_request_irq(scn->qdf_dev->dev,
				       irq, hif_ce_interrupt_handler,
				       irqflags,
				       ce_name[ce_id],
				       &ce_sc->tasklets[ce_id]);
		if (ret)
			goto free_irq;
	}

	return ret;

free_irq:
	/* the request_irq for the last ce_id failed so skip it. */
	while (ce_id > 0 && ce_id < scn->ce_count) {
		unsigned int msi_data;

		ce_id--;
		msi_data = (ce_id % msi_data_count) + msi_irq_start;
		irq = pld_get_msi_irq(scn->qdf_dev->dev, msi_data);
		pfrm_free_irq(scn->qdf_dev->dev, irq, &ce_sc->tasklets[ce_id]);
	}

	return ret;
}

/**
 * hif_exec_grp_irq_disable() - disable the irq for group
 * @hif_ext_group: hif exec context
 *
 * Return: none
 */
static void hif_exec_grp_irq_disable(struct hif_exec_context *hif_ext_group)
{
	int i;
	struct hif_softc *scn = HIF_GET_SOFTC(hif_ext_group->hif);

	for (i = 0; i < hif_ext_group->numirq; i++)
		pfrm_disable_irq_nosync(scn->qdf_dev->dev,
					hif_ext_group->os_irq[i]);
}

/**
 * hif_exec_grp_irq_enable() - enable the irq for group
 * @hif_ext_group: hif exec context
 *
 * Return: none
 */
static void hif_exec_grp_irq_enable(struct hif_exec_context *hif_ext_group)
{
	int i;
	struct hif_softc *scn = HIF_GET_SOFTC(hif_ext_group->hif);

	for (i = 0; i < hif_ext_group->numirq; i++)
		pfrm_enable_irq(scn->qdf_dev->dev, hif_ext_group->os_irq[i]);
}

const char *hif_ipci_get_irq_name(int irq_no)
{
	return "pci-dummy";
}

#ifdef FEATURE_IRQ_AFFINITY
static
void hif_ipci_irq_set_affinity_hint(struct hif_exec_context *hif_ext_group,
				    bool perf)
{
	int i, ret;
	unsigned int cpus;
	bool mask_set = false;
	int cpu_cluster = perf ? CPU_CLUSTER_TYPE_PERF :
						CPU_CLUSTER_TYPE_LITTLE;

	for (i = 0; i < hif_ext_group->numirq; i++)
		qdf_cpumask_clear(&hif_ext_group->new_cpu_mask[i]);

	for (i = 0; i < hif_ext_group->numirq; i++) {
		qdf_for_each_online_cpu(cpus) {
			if (qdf_topology_physical_package_id(cpus) ==
			    cpu_cluster) {
				qdf_cpumask_set_cpu(cpus,
						    &hif_ext_group->
						    new_cpu_mask[i]);
				mask_set = true;
			}
		}
	}
	for (i = 0; i < hif_ext_group->numirq; i++) {
		if (mask_set) {
			qdf_dev_modify_irq_status(hif_ext_group->os_irq[i],
						  IRQ_NO_BALANCING, 0);
			ret = qdf_dev_set_irq_affinity(hif_ext_group->os_irq[i],
						       (struct qdf_cpu_mask *)
						       &hif_ext_group->
						       new_cpu_mask[i]);
			qdf_dev_modify_irq_status(hif_ext_group->os_irq[i],
						  0, IRQ_NO_BALANCING);
			if (ret)
				qdf_debug("Set affinity %*pbl fails for IRQ %d ",
					  qdf_cpumask_pr_args(&hif_ext_group->
							      new_cpu_mask[i]),
					  hif_ext_group->os_irq[i]);
		} else {
			qdf_err("Offline CPU: Set affinity fails for IRQ: %d",
				hif_ext_group->os_irq[i]);
		}
	}
}

void hif_ipci_set_grp_intr_affinity(struct hif_softc *scn,
				    uint32_t grp_intr_bitmask, bool perf)
{
	int i;
	struct HIF_CE_state *hif_state = HIF_GET_CE_STATE(scn);
	struct hif_exec_context *hif_ext_group;

	for (i = 0; i < hif_state->hif_num_extgroup; i++) {
		if (!(grp_intr_bitmask & BIT(i)))
			continue;

		hif_ext_group = hif_state->hif_ext_group[i];
		hif_ipci_irq_set_affinity_hint(hif_ext_group, perf);
		qdf_atomic_set(&hif_ext_group->force_napi_complete, -1);
	}
}
#endif

#ifdef HIF_CPU_PERF_AFFINE_MASK
static void hif_ipci_ce_irq_set_affinity_hint(struct hif_softc *scn)
{
	int ret;
	unsigned int cpus;
	struct HIF_CE_state *ce_sc = HIF_GET_CE_STATE(scn);
	struct hif_ipci_softc *ipci_sc = HIF_GET_IPCI_SOFTC(scn);
	struct CE_attr *host_ce_conf;
	int ce_id;
	qdf_cpu_mask ce_cpu_mask;

	host_ce_conf = ce_sc->host_ce_config;
	qdf_cpumask_clear(&ce_cpu_mask);

	qdf_for_each_online_cpu(cpus) {
		if (qdf_topology_physical_package_id(cpus) ==
			CPU_CLUSTER_TYPE_PERF) {
			qdf_cpumask_set_cpu(cpus,
					    &ce_cpu_mask);
		}
	}
	if (qdf_cpumask_empty(&ce_cpu_mask)) {
		hif_err_rl("Empty cpu_mask, unable to set CE IRQ affinity");
		return;
	}
	for (ce_id = 0; ce_id < scn->ce_count; ce_id++) {
		if (host_ce_conf[ce_id].flags & CE_ATTR_DISABLE_INTR)
			continue;
		qdf_cpumask_clear(&ipci_sc->ce_irq_cpu_mask[ce_id]);
		qdf_cpumask_copy(&ipci_sc->ce_irq_cpu_mask[ce_id],
				 &ce_cpu_mask);
		qdf_dev_modify_irq_status(ipci_sc->ce_msi_irq_num[ce_id],
					  IRQ_NO_BALANCING, 0);
		ret = qdf_dev_set_irq_affinity(
		       ipci_sc->ce_msi_irq_num[ce_id],
		       (struct qdf_cpu_mask *)&ipci_sc->ce_irq_cpu_mask[ce_id]);
		qdf_dev_modify_irq_status(ipci_sc->ce_msi_irq_num[ce_id],
					  0, IRQ_NO_BALANCING);
		if (ret)
			hif_err_rl("Set affinity %*pbl fails for CE IRQ %d",
				   qdf_cpumask_pr_args(
					&ipci_sc->ce_irq_cpu_mask[ce_id]),
					ipci_sc->ce_msi_irq_num[ce_id]);
		else
			hif_debug_rl("Set affinity %*pbl for CE IRQ: %d",
				     qdf_cpumask_pr_args(
				     &ipci_sc->ce_irq_cpu_mask[ce_id]),
				     ipci_sc->ce_msi_irq_num[ce_id]);
	}
}

void hif_ipci_config_irq_affinity(struct hif_softc *scn)
{
	hif_core_ctl_set_boost(true);
	/* Set IRQ affinity for CE interrupts*/
	hif_ipci_ce_irq_set_affinity_hint(scn);
}
#endif /* #ifdef HIF_CPU_PERF_AFFINE_MASK */

#ifdef HIF_CPU_CLEAR_AFFINITY
void hif_ipci_config_irq_clear_cpu_affinity(struct hif_softc *scn,
					    int intr_ctxt_id, int cpu)
{
	struct HIF_CE_state *hif_state = HIF_GET_CE_STATE(scn);
	struct hif_exec_context *hif_ext_group;
	int i, ret;

	if (intr_ctxt_id < hif_state->hif_num_extgroup) {
		hif_ext_group = hif_state->hif_ext_group[intr_ctxt_id];
		for (i = 0; i < hif_ext_group->numirq; i++) {
			qdf_cpumask_setall(&hif_ext_group->new_cpu_mask[i]);
			qdf_cpumask_clear_cpu(cpu,
					      &hif_ext_group->new_cpu_mask[i]);
			qdf_dev_modify_irq_status(hif_ext_group->os_irq[i],
						  IRQ_NO_BALANCING, 0);
			ret = qdf_dev_set_irq_affinity(hif_ext_group->os_irq[i],
						       (struct qdf_cpu_mask *)
						       &hif_ext_group->
						       new_cpu_mask[i]);
			qdf_dev_modify_irq_status(hif_ext_group->os_irq[i],
						  0, IRQ_NO_BALANCING);
			if (ret)
				hif_err("Set affinity %*pbl fails for IRQ %d ",
					qdf_cpumask_pr_args(&hif_ext_group->
							    new_cpu_mask[i]),
					hif_ext_group->os_irq[i]);
			else
				hif_debug("Set affinity %*pbl for IRQ: %d",
					  qdf_cpumask_pr_args(&hif_ext_group->
							      new_cpu_mask[i]),
					  hif_ext_group->os_irq[0]);
		}
	}
}
#endif

int hif_ipci_configure_grp_irq(struct hif_softc *scn,
			       struct hif_exec_context *hif_ext_group)
{
	int ret = 0;
	int irq = 0;
	int j;

	hif_ext_group->irq_enable = &hif_exec_grp_irq_enable;
	hif_ext_group->irq_disable = &hif_exec_grp_irq_disable;
	hif_ext_group->irq_name = &hif_ipci_get_irq_name;
	hif_ext_group->work_complete = &hif_dummy_grp_done;

	for (j = 0; j < hif_ext_group->numirq; j++) {
		irq = hif_ext_group->irq[j];

		hif_info("request_irq = %d for grp %d",
			 irq, hif_ext_group->grp_id);
		ret = pfrm_request_irq(scn->qdf_dev->dev, irq,
				       hif_ext_group_interrupt_handler,
				       IRQF_SHARED | IRQF_NO_SUSPEND,
				       "wlan_EXT_GRP",
				       hif_ext_group);
		if (ret) {
			hif_err("request_irq failed ret = %d", ret);
			return -EFAULT;
		}
		hif_ext_group->os_irq[j] = irq;
	}
	hif_ext_group->irq_requested = true;
	return 0;
}

int hif_configure_irq(struct hif_softc *scn)
{
	int ret = 0;

	if (hif_is_polled_mode_enabled(GET_HIF_OPAQUE_HDL(scn))) {
		scn->request_irq_done = false;
		return 0;
	}

	ret = hif_ce_msi_configure_irq(scn);
	if (ret == 0)
		goto end;

	if (ret < 0) {
		hif_err("hif_ipci_configure_irq error = %d", ret);
		return ret;
	}
end:
	scn->request_irq_done = true;
	return 0;
}

/**
 * hif_ipci_get_soc_info_pld() - get soc info for ipcie bus from pld target
 * @sc: ipci context
 * @dev: device structure
 *
 * Return: none
 */
static void hif_ipci_get_soc_info_pld(struct hif_ipci_softc *sc,
				      struct device *dev)
{
	struct pld_soc_info info;
	struct hif_softc *scn = HIF_GET_SOFTC(sc);

	pld_get_soc_info(dev, &info);
	sc->mem = info.v_addr;
	sc->ce_sc.ol_sc.mem    = info.v_addr;
	sc->ce_sc.ol_sc.mem_pa = info.p_addr;

	scn->target_info.target_version = info.soc_id;
	scn->target_info.target_revision = 0;
}

/**
 * hif_ipci_get_soc_info_nopld() - get soc info for ipcie bus for non pld target
 * @sc: ipci context
 * @dev: device structure
 *
 * Return: none
 */
static void hif_ipci_get_soc_info_nopld(struct hif_ipci_softc *sc,
					struct device *dev)
{}

/**
 * hif_is_pld_based_target() - verify if the target is pld based
 * @sc: ipci context
 * @device_id: device id
 *
 * Return: none
 */
static bool hif_is_pld_based_target(struct hif_ipci_softc *sc,
				    int device_id)
{
	if (!pld_have_platform_driver_support(sc->dev))
		return false;

	switch (device_id) {
#ifdef QCA_WIFI_QCA6750
	case QCA6750_DEVICE_ID:
#endif
		return true;
	}
	return false;
}

/**
 * hif_ipci_init_deinit_ops_attach() - attach ops for ipci
 * @sc: ipci context
 * @device_id: device id
 *
 * Return: none
 */
static void hif_ipci_init_deinit_ops_attach(struct hif_ipci_softc *sc,
					    int device_id)
{
	if (hif_is_pld_based_target(sc, device_id))
		sc->hif_ipci_get_soc_info = hif_ipci_get_soc_info_pld;
	else
		sc->hif_ipci_get_soc_info = hif_ipci_get_soc_info_nopld;
}

QDF_STATUS hif_ipci_enable_bus(struct hif_softc *ol_sc,
			       struct device *dev, void *bdev,
			       const struct hif_bus_id *bid,
			       enum hif_enable_type type)
{
	int ret = 0;
	uint32_t hif_type, target_type;
	struct hif_ipci_softc *sc = HIF_GET_IPCI_SOFTC(ol_sc);
	struct hif_opaque_softc *hif_hdl = GET_HIF_OPAQUE_HDL(ol_sc);
	uint16_t revision_id = 0;
	struct hif_target_info *tgt_info;
	int device_id = QCA6750_DEVICE_ID;

	if (!ol_sc) {
		hif_err("hif_ctx is NULL");
		return QDF_STATUS_E_NOMEM;
	}

	ret = qdf_set_dma_coherent_mask(dev,
					DMA_COHERENT_MASK_DEFAULT);
	if (ret) {
		hif_err("Failed to set dma mask error = %d", ret);
		return qdf_status_from_os_return(ret);
	}

	sc->dev = dev;
	tgt_info = hif_get_target_info_handle(hif_hdl);
	hif_ipci_init_deinit_ops_attach(sc, device_id);
	sc->hif_ipci_get_soc_info(sc, dev);
	hif_debug("hif_enable_pci done");

	ret = hif_get_device_type(device_id, revision_id,
				  &hif_type, &target_type);
	if (ret < 0) {
		hif_err("Invalid device id/revision_id");
		return QDF_STATUS_E_ABORTED;
	}
	hif_debug("hif_type = 0x%x, target_type = 0x%x",
		 hif_type, target_type);

	hif_register_tbl_attach(ol_sc, hif_type);
	hif_target_register_tbl_attach(ol_sc, target_type);
	sc->use_register_windowing = false;
	tgt_info->target_type = target_type;

	if (!ol_sc->mem_pa) {
		hif_err("BAR0 uninitialized");
		return QDF_STATUS_E_ABORTED;
	}

	return QDF_STATUS_SUCCESS;
}

bool hif_ipci_needs_bmi(struct hif_softc *scn)
{
	return !ce_srng_based(scn);
}

#ifdef FORCE_WAKE
int hif_force_wake_request(struct hif_opaque_softc *hif_handle)
{
	uint32_t timeout = 0;
	struct hif_softc *scn = (struct hif_softc *)hif_handle;
	struct hif_ipci_softc *ipci_scn = HIF_GET_IPCI_SOFTC(scn);

	if (pld_force_wake_request(scn->qdf_dev->dev)) {
		hif_err_rl("force wake request send failed");
		return -EINVAL;
	}

	HIF_STATS_INC(ipci_scn, mhi_force_wake_request_vote, 1);
	while (!pld_is_device_awake(scn->qdf_dev->dev) &&
	       timeout <= FORCE_WAKE_DELAY_TIMEOUT_MS) {
		if (qdf_in_interrupt())
			qdf_mdelay(FORCE_WAKE_DELAY_MS);
		else
			qdf_sleep(FORCE_WAKE_DELAY_MS);

		timeout += FORCE_WAKE_DELAY_MS;
	}

	if (pld_is_device_awake(scn->qdf_dev->dev) <= 0) {
		hif_err("Unable to wake up mhi");
		HIF_STATS_INC(ipci_scn, mhi_force_wake_failure, 1);
		hif_force_wake_release(hif_handle);
		return -EINVAL;
	}
	HIF_STATS_INC(ipci_scn, mhi_force_wake_success, 1);

	HIF_STATS_INC(ipci_scn, soc_force_wake_success, 1);

	return 0;
}

int hif_force_wake_release(struct hif_opaque_softc *hif_handle)
{
	int ret;
	struct hif_softc *scn = (struct hif_softc *)hif_handle;
	struct hif_ipci_softc *ipci_scn = HIF_GET_IPCI_SOFTC(scn);

	ret = pld_force_wake_release(scn->qdf_dev->dev);
	if (ret) {
		hif_err("force wake release failure");
		HIF_STATS_INC(ipci_scn, mhi_force_wake_release_failure, 1);
		return ret;
	}

	HIF_STATS_INC(ipci_scn, mhi_force_wake_release_success, 1);

	HIF_STATS_INC(ipci_scn, soc_force_wake_release_success, 1);
	return 0;
}

void hif_print_ipci_stats(struct hif_ipci_softc *ipci_handle)
{
	hif_debug("mhi_force_wake_request_vote: %d",
		  ipci_handle->stats.mhi_force_wake_request_vote);
	hif_debug("mhi_force_wake_failure: %d",
		  ipci_handle->stats.mhi_force_wake_failure);
	hif_debug("mhi_force_wake_success: %d",
		  ipci_handle->stats.mhi_force_wake_success);
	hif_debug("soc_force_wake_register_write_success: %d",
		  ipci_handle->stats.soc_force_wake_register_write_success);
	hif_debug("soc_force_wake_failure: %d",
		  ipci_handle->stats.soc_force_wake_failure);
	hif_debug("soc_force_wake_success: %d",
		  ipci_handle->stats.soc_force_wake_success);
	hif_debug("mhi_force_wake_release_failure: %d",
		  ipci_handle->stats.mhi_force_wake_release_failure);
	hif_debug("mhi_force_wake_release_success: %d",
		  ipci_handle->stats.mhi_force_wake_release_success);
	hif_debug("oc_force_wake_release_success: %d",
		  ipci_handle->stats.soc_force_wake_release_success);
}
#endif /* FORCE_WAKE */

#ifdef FEATURE_HAL_DELAYED_REG_WRITE
int hif_prevent_link_low_power_states(struct hif_opaque_softc *hif)
{
	struct hif_softc *scn = HIF_GET_SOFTC(hif);
	struct hif_ipci_softc *ipci_scn = HIF_GET_IPCI_SOFTC(scn);
	uint32_t start_time = 0, curr_time = 0;
	uint32_t count = 0;

	if (pld_is_pci_ep_awake(scn->qdf_dev->dev) == -ENOTSUPP)
		return 0;

	if ((qdf_atomic_read(&scn->dp_ep_vote_access) ==
	     HIF_EP_VOTE_ACCESS_DISABLE) &&
	    (qdf_atomic_read(&scn->ep_vote_access) ==
	    HIF_EP_VOTE_ACCESS_DISABLE)) {
		hif_info_high("EP access disabled in flight skip vote");
		return 0;
	}

	start_time = curr_time = qdf_system_ticks_to_msecs(qdf_system_ticks());
	while (pld_is_pci_ep_awake(scn->qdf_dev->dev) &&
	       curr_time <= start_time + EP_WAKE_RESET_DELAY_TIMEOUT_MS) {
		if (count < EP_VOTE_POLL_TIME_CNT) {
			qdf_udelay(EP_VOTE_POLL_TIME_US);
			count++;
		} else {
			qdf_sleep_us(EP_WAKE_RESET_DELAY_US);
		}
		curr_time = qdf_system_ticks_to_msecs(qdf_system_ticks());
	}


	if (pld_is_pci_ep_awake(scn->qdf_dev->dev)) {
		hif_err_rl(" EP state reset is not done to prevent l1");
		ipci_scn->ep_awake_reset_fail++;
		return 0;
	}

	if (pld_prevent_l1(scn->qdf_dev->dev)) {
		hif_err_rl("pld prevent l1 failed");
		ipci_scn->prevent_l1_fail++;
		return 0;
	}

	count = 0;
	ipci_scn->prevent_l1 = true;
	start_time = curr_time = qdf_system_ticks_to_msecs(qdf_system_ticks());
	while (!pld_is_pci_ep_awake(scn->qdf_dev->dev) &&
	       curr_time <= start_time + EP_WAKE_DELAY_TIMEOUT_MS) {
		if (count < EP_VOTE_POLL_TIME_CNT) {
			qdf_udelay(EP_WAKE_RESET_DELAY_US);
			count++;
		} else {
			qdf_sleep_us(EP_WAKE_DELAY_US);
		}

		curr_time = qdf_system_ticks_to_msecs(qdf_system_ticks());
	}

	if (pld_is_pci_ep_awake(scn->qdf_dev->dev) <= 0) {
		hif_err_rl("Unable to wakeup pci ep");
		ipci_scn->ep_awake_set_fail++;
		return  0;
	}

	return 0;
}

void hif_allow_link_low_power_states(struct hif_opaque_softc *hif)
{
	struct hif_softc *scn = HIF_GET_SOFTC(hif);
	struct hif_ipci_softc *ipci_scn = HIF_GET_IPCI_SOFTC(scn);

	if (qdf_likely(ipci_scn->prevent_l1)) {
		pld_allow_l1(scn->qdf_dev->dev);
		ipci_scn->prevent_l1 = false;
	}
}
#endif

int hif_ipci_enable_grp_irqs(struct hif_softc *scn)
{
	struct hif_ipci_softc *ipci_scn = HIF_GET_IPCI_SOFTC(scn);
	int status;

	if (!ipci_scn->grp_irqs_disabled) {
		hif_err("Unbalanced group IRQs Enable called");
		qdf_assert_always(0);
	}

	status = hif_apps_grp_irqs_enable(GET_HIF_OPAQUE_HDL(scn));
	if (!status)
		ipci_scn->grp_irqs_disabled = false;

	return status;
}

int hif_ipci_disable_grp_irqs(struct hif_softc *scn)
{
	struct hif_ipci_softc *ipci_scn = HIF_GET_IPCI_SOFTC(scn);
	int status;

	if (ipci_scn->grp_irqs_disabled) {
		hif_err("Unbalanced group IRQs disable called");
		qdf_assert_always(0);
	}

	status = hif_apps_grp_irqs_disable(GET_HIF_OPAQUE_HDL(scn));
	if (!status)
		ipci_scn->grp_irqs_disabled = true;

	return status;
}
