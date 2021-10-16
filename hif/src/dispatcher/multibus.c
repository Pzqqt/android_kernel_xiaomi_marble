/*
 * Copyright (c) 2016-2018, 2020-2021 The Linux Foundation. All rights reserved.
 * Copyright (c) 2021 Qualcomm Innovation Center, Inc. All rights reserved.
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

/* this file dispatches functions to bus specific definitions */
#include "hif_debug.h"
#include "hif.h"
#include "hif_main.h"
#include "hif_io32.h"
#include "multibus.h"
#include "dummy.h"
#if defined(HIF_PCI) || defined(HIF_SNOC) || defined(HIF_AHB) || \
    defined(HIF_IPCI)
#include "ce_main.h"
#include "ce_api.h"
#include "ce_internal.h"
#endif
#include "htc_services.h"
#include "a_types.h"
#include "dummy.h"
#include "qdf_module.h"

/**
 * hif_initialize_default_ops() - initializes default operations values
 *
 * bus specific features should assign their dummy implementations here.
 */
static void hif_initialize_default_ops(struct hif_softc *hif_sc)
{
	struct hif_bus_ops *bus_ops = &hif_sc->bus_ops;

	/* must be filled in by hif_bus_open */
	bus_ops->hif_bus_close = NULL;
	/* dummy implementations */
	bus_ops->hif_display_stats =
		&hif_dummy_display_stats;
	bus_ops->hif_clear_stats =
		&hif_dummy_clear_stats;
	bus_ops->hif_set_bundle_mode = &hif_dummy_set_bundle_mode;
	bus_ops->hif_bus_reset_resume = &hif_dummy_bus_reset_resume;
	bus_ops->hif_bus_suspend_noirq = &hif_dummy_bus_suspend_noirq;
	bus_ops->hif_bus_resume_noirq = &hif_dummy_bus_resume_noirq;
	bus_ops->hif_bus_early_suspend = &hif_dummy_bus_suspend;
	bus_ops->hif_bus_late_resume = &hif_dummy_bus_resume;
	bus_ops->hif_map_ce_to_irq = &hif_dummy_map_ce_to_irq;
	bus_ops->hif_grp_irq_configure = &hif_dummy_grp_irq_configure;
	bus_ops->hif_grp_irq_deconfigure = &hif_dummy_grp_irq_deconfigure;
	bus_ops->hif_config_irq_affinity =
		&hif_dummy_config_irq_affinity;
	bus_ops->hif_config_irq_by_ceid = &hif_dummy_config_irq_by_ceid;
	bus_ops->hif_enable_grp_irqs = &hif_dummy_enable_grp_irqs;
	bus_ops->hif_disable_grp_irqs = &hif_dummy_enable_grp_irqs;
	bus_ops->hif_config_irq_clear_cpu_affinity =
		&hif_dummy_config_irq_clear_cpu_affinity;
	bus_ops->hif_set_grp_intr_affinity = &hif_dummy_set_grp_intr_affinity;
}

#define NUM_OPS (sizeof(struct hif_bus_ops) / sizeof(void *))

/**
 * hif_verify_basic_ops() - ensure required bus apis are defined
 *
 * all bus operations must be defined to avoid crashes
 * itterate over the structure and ensure all function pointers
 * are non null.
 *
 * Return: QDF_STATUS_SUCCESS if all the operations are defined
 */
static QDF_STATUS hif_verify_basic_ops(struct hif_softc *hif_sc)
{
	struct hif_bus_ops *bus_ops = &hif_sc->bus_ops;
	void **ops_array = (void *)bus_ops;
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	int i;

	for (i = 0; i < NUM_OPS; i++) {
		if (!ops_array[i]) {
			hif_err("ops_array[%d] is null", i);
			status = QDF_STATUS_E_NOSUPPORT;
		}
	}
	return status;
}

/**
 * hif_bus_get_context_size - API to return size of the bus specific structure
 *
 * Return: sizeof of hif_pci_softc
 */
int hif_bus_get_context_size(enum qdf_bus_type bus_type)
{
	switch (bus_type) {
	case QDF_BUS_TYPE_PCI:
		return hif_pci_get_context_size();
	case QDF_BUS_TYPE_IPCI:
		return hif_ipci_get_context_size();
	case QDF_BUS_TYPE_AHB:
		return hif_ahb_get_context_size();
	case QDF_BUS_TYPE_SNOC:
		return hif_snoc_get_context_size();
	case QDF_BUS_TYPE_SDIO:
		return hif_sdio_get_context_size();
	case QDF_BUS_TYPE_USB:
		return hif_usb_get_context_size();
	default:
		return 0;
	}
}

/**
 * hif_bus_open() - initialize the bus_ops and call the bus specific open
 * hif_sc: hif_context
 * bus_type: type of bus being enumerated
 *
 * Return: QDF_STATUS_SUCCESS or error
 */
QDF_STATUS hif_bus_open(struct hif_softc *hif_sc,
			enum qdf_bus_type bus_type)
{
	QDF_STATUS status = QDF_STATUS_E_INVAL;

	hif_initialize_default_ops(hif_sc);

	switch (bus_type) {
	case QDF_BUS_TYPE_PCI:
		status = hif_initialize_pci_ops(hif_sc);
		break;
	case QDF_BUS_TYPE_IPCI:
		status = hif_initialize_ipci_ops(hif_sc);
		break;
	case QDF_BUS_TYPE_SNOC:
		status = hif_initialize_snoc_ops(&hif_sc->bus_ops);
		break;
	case QDF_BUS_TYPE_AHB:
		status = hif_initialize_ahb_ops(&hif_sc->bus_ops);
		break;
	case QDF_BUS_TYPE_SDIO:
		status = hif_initialize_sdio_ops(hif_sc);
		break;
	case QDF_BUS_TYPE_USB:
		status = hif_initialize_usb_ops(&hif_sc->bus_ops);
		break;
	default:
		status = QDF_STATUS_E_NOSUPPORT;
		break;
	}

	if (status != QDF_STATUS_SUCCESS) {
		hif_err("bus_type: %d not supported", bus_type);
		return status;
	}

	status = hif_verify_basic_ops(hif_sc);
	if (status != QDF_STATUS_SUCCESS)
		return status;

	return hif_sc->bus_ops.hif_bus_open(hif_sc, bus_type);
}

/**
 * hif_bus_close() - close the bus
 * @hif_sc: hif_context
 */
void hif_bus_close(struct hif_softc *hif_sc)
{
	hif_sc->bus_ops.hif_bus_close(hif_sc);
}

/**
 * hif_bus_prevent_linkdown() - prevent linkdown
 * @hif_ctx: hif context
 * @flag: true = keep bus alive false = let bus go to sleep
 *
 * Keeps the bus awake durring suspend.
 */
void hif_bus_prevent_linkdown(struct hif_softc *hif_sc, bool flag)
{
	hif_sc->bus_ops.hif_bus_prevent_linkdown(hif_sc, flag);
}


void hif_reset_soc(struct hif_opaque_softc *hif_ctx)
{
	struct hif_softc *hif_sc = HIF_GET_SOFTC(hif_ctx);

	hif_sc->bus_ops.hif_reset_soc(hif_sc);
}

int hif_bus_early_suspend(struct hif_opaque_softc *hif_ctx)
{
	struct hif_softc *hif_sc = HIF_GET_SOFTC(hif_ctx);

	return hif_sc->bus_ops.hif_bus_early_suspend(hif_sc);
}

int hif_bus_late_resume(struct hif_opaque_softc *hif_ctx)
{
	struct hif_softc *hif_sc = HIF_GET_SOFTC(hif_ctx);

	return hif_sc->bus_ops.hif_bus_late_resume(hif_sc);
}

int hif_bus_suspend(struct hif_opaque_softc *hif_ctx)
{
	struct hif_softc *hif_sc = HIF_GET_SOFTC(hif_ctx);

	return hif_sc->bus_ops.hif_bus_suspend(hif_sc);
}

int hif_bus_resume(struct hif_opaque_softc *hif_ctx)
{
	struct hif_softc *hif_sc = HIF_GET_SOFTC(hif_ctx);

	return hif_sc->bus_ops.hif_bus_resume(hif_sc);
}

int hif_bus_suspend_noirq(struct hif_opaque_softc *hif_ctx)
{
	struct hif_softc *hif_sc = HIF_GET_SOFTC(hif_ctx);

	return hif_sc->bus_ops.hif_bus_suspend_noirq(hif_sc);
}

int hif_bus_resume_noirq(struct hif_opaque_softc *hif_ctx)
{
	struct hif_softc *hif_sc = HIF_GET_SOFTC(hif_ctx);

	return hif_sc->bus_ops.hif_bus_resume_noirq(hif_sc);
}

int hif_target_sleep_state_adjust(struct hif_softc *hif_sc,
			      bool sleep_ok, bool wait_for_it)
{
	return hif_sc->bus_ops.hif_target_sleep_state_adjust(hif_sc,
			sleep_ok, wait_for_it);
}
qdf_export_symbol(hif_target_sleep_state_adjust);

void hif_disable_isr(struct hif_opaque_softc *hif_hdl)
{
	struct hif_softc *hif_sc = HIF_GET_SOFTC(hif_hdl);

	hif_sc->bus_ops.hif_disable_isr(hif_sc);
}

void hif_nointrs(struct hif_softc *hif_sc)
{
	hif_sc->bus_ops.hif_nointrs(hif_sc);
}

QDF_STATUS hif_enable_bus(struct hif_softc *hif_sc, struct device *dev,
			  void *bdev, const struct hif_bus_id *bid,
			  enum hif_enable_type type)
{
	return hif_sc->bus_ops.hif_enable_bus(hif_sc, dev, bdev, bid, type);
}

void hif_disable_bus(struct hif_softc *hif_sc)
{
	hif_sc->bus_ops.hif_disable_bus(hif_sc);
}

#ifdef FEATURE_RUNTIME_PM
struct hif_runtime_pm_ctx *hif_bus_get_rpm_ctx(struct hif_softc *hif_sc)
{
	return hif_sc->bus_ops.hif_bus_get_rpm_ctx(hif_sc);
}

struct device *hif_bus_get_dev(struct hif_softc *hif_sc)
{
	return hif_sc->bus_ops.hif_bus_get_dev(hif_sc);
}
#endif

int hif_bus_configure(struct hif_softc *hif_sc)
{
	return hif_sc->bus_ops.hif_bus_configure(hif_sc);
}

QDF_STATUS hif_get_config_item(struct hif_opaque_softc *hif_ctx,
		     int opcode, void *config, uint32_t config_len)
{
	struct hif_softc *hif_sc = HIF_GET_SOFTC(hif_ctx);

	return hif_sc->bus_ops.hif_get_config_item(hif_sc, opcode, config,
						 config_len);
}

void hif_set_mailbox_swap(struct hif_opaque_softc *hif_ctx)
{
	struct hif_softc *hif_sc = HIF_GET_SOFTC(hif_ctx);

	hif_sc->bus_ops.hif_set_mailbox_swap(hif_sc);
}

void hif_claim_device(struct hif_opaque_softc *hif_ctx)
{
	struct hif_softc *hif_sc = HIF_GET_SOFTC(hif_ctx);

	hif_sc->bus_ops.hif_claim_device(hif_sc);
}

void hif_shutdown_device(struct hif_opaque_softc *hif_ctx)
{
	struct hif_softc *hif_sc = HIF_GET_SOFTC(hif_ctx);

	hif_sc->bus_ops.hif_shutdown_device(hif_sc);
}

void hif_stop(struct hif_opaque_softc *hif_ctx)
{
	struct hif_softc *hif_sc = HIF_GET_SOFTC(hif_ctx);

	hif_sc->bus_ops.hif_stop(hif_sc);
}

void hif_cancel_deferred_target_sleep(struct hif_softc *hif_sc)
{
	return hif_sc->bus_ops.hif_cancel_deferred_target_sleep(hif_sc);
}

void hif_irq_enable(struct hif_softc *hif_sc, int irq_id)
{
	hif_sc->bus_ops.hif_irq_enable(hif_sc, irq_id);
}
qdf_export_symbol(hif_irq_enable);

void hif_irq_disable(struct hif_softc *hif_sc, int irq_id)
{
	hif_sc->bus_ops.hif_irq_disable(hif_sc, irq_id);
}

int hif_grp_irq_configure(struct hif_softc *hif_sc,
			  struct hif_exec_context *hif_exec)
{
	return hif_sc->bus_ops.hif_grp_irq_configure(hif_sc, hif_exec);
}

void hif_grp_irq_deconfigure(struct hif_softc *hif_sc)
{
	hif_sc->bus_ops.hif_grp_irq_deconfigure(hif_sc);
}

int hif_dump_registers(struct hif_opaque_softc *hif_hdl)
{
	struct hif_softc *hif_sc = HIF_GET_SOFTC(hif_hdl);

	return hif_sc->bus_ops.hif_dump_registers(hif_sc);
}

void hif_dump_target_memory(struct hif_opaque_softc *hif_hdl,
			    void *ramdump_base,
			    uint32_t address, uint32_t size)
{
	struct hif_softc *hif_sc = HIF_GET_SOFTC(hif_hdl);

	hif_sc->bus_ops.hif_dump_target_memory(hif_sc, ramdump_base,
					       address, size);
}

void hif_ipa_get_ce_resource(struct hif_opaque_softc *hif_hdl,
			     qdf_shared_mem_t **ce_sr,
			     uint32_t *ce_sr_ring_size,
			     qdf_dma_addr_t *ce_reg_paddr)
{
	struct hif_softc *hif_sc = HIF_GET_SOFTC(hif_hdl);

	hif_sc->bus_ops.hif_ipa_get_ce_resource(hif_sc, ce_sr,
			ce_sr_ring_size, ce_reg_paddr);
}

void hif_mask_interrupt_call(struct hif_opaque_softc *hif_hdl)
{
	struct hif_softc *hif_sc = HIF_GET_SOFTC(hif_hdl);

	hif_sc->bus_ops.hif_mask_interrupt_call(hif_sc);
}

void hif_display_bus_stats(struct hif_opaque_softc *scn)
{
	struct hif_softc *hif_sc = HIF_GET_SOFTC(scn);

	hif_sc->bus_ops.hif_display_stats(hif_sc);
}

void hif_clear_bus_stats(struct hif_opaque_softc *scn)
{
	struct hif_softc *hif_sc = HIF_GET_SOFTC(scn);

	hif_sc->bus_ops.hif_clear_stats(hif_sc);
}

/**
 * hif_enable_power_management() - enable power management after driver load
 * @hif_hdl: opaque pointer to the hif context
 * is_packet_log_enabled: true if packet log is enabled
 *
 * Driver load and firmware download are done in a high performance mode.
 * Enable power management after the driver is loaded.
 * packet log can require fewer power management features to be enabled.
 */
void hif_enable_power_management(struct hif_opaque_softc *hif_hdl,
				 bool is_packet_log_enabled)
{
	struct hif_softc *hif_sc = HIF_GET_SOFTC(hif_hdl);

	hif_sc->bus_ops.hif_enable_power_management(hif_sc,
				    is_packet_log_enabled);
}

/**
 * hif_disable_power_management() - reset the bus power management
 * @hif_hdl: opaque pointer to the hif context
 *
 * return the power management of the bus to its default state.
 * This isn't necessarily a complete reversal of its counterpart.
 * This should be called when unloading the driver.
 */
void hif_disable_power_management(struct hif_opaque_softc *hif_hdl)
{
	struct hif_softc *hif_sc = HIF_GET_SOFTC(hif_hdl);

	hif_sc->bus_ops.hif_disable_power_management(hif_sc);
}

/**
 * hif_set_bundle_mode() - enable bundling and set default rx bundle cnt
 * @scn: pointer to hif_opaque_softc structure
 * @enabled: flag to enable/disable bundling
 * @rx_bundle_cnt: bundle count to be used for RX
 *
 * Return: none
 */
void hif_set_bundle_mode(struct hif_opaque_softc *scn, bool enabled,
				int rx_bundle_cnt)
{
	struct hif_softc *hif_sc = HIF_GET_SOFTC(scn);

	hif_sc->bus_ops.hif_set_bundle_mode(hif_sc, enabled, rx_bundle_cnt);
}

/**
 * hif_bus_reset_resume() - resume the bus after reset
 * @scn: struct hif_opaque_softc
 *
 * This function is called to tell the driver that USB device has been resumed
 * and it has also been reset. The driver should redo any necessary
 * initialization. This function resets WLAN SOC.
 *
 * Return: int 0 for success, non zero for failure
 */
int hif_bus_reset_resume(struct hif_opaque_softc *scn)
{
	struct hif_softc *hif_sc = HIF_GET_SOFTC(scn);

	return hif_sc->bus_ops.hif_bus_reset_resume(hif_sc);
}

int hif_apps_irqs_disable(struct hif_opaque_softc *hif_ctx)
{
	struct hif_softc *scn;
	int i;

	QDF_BUG(hif_ctx);
	scn = HIF_GET_SOFTC(hif_ctx);
	if (!scn)
		return -EINVAL;

	/* if the wake_irq is shared, don't disable it twice */
	for (i = 0; i < scn->ce_count; ++i) {
		int irq = scn->bus_ops.hif_map_ce_to_irq(scn, i);

		if (irq != scn->wake_irq)
			disable_irq(irq);
	}

	return 0;
}

int hif_apps_irqs_enable(struct hif_opaque_softc *hif_ctx)
{
	struct hif_softc *scn;
	int i;

	QDF_BUG(hif_ctx);
	scn = HIF_GET_SOFTC(hif_ctx);
	if (!scn)
		return -EINVAL;

	/* if the wake_irq is shared, don't enable it twice */
	for (i = 0; i < scn->ce_count; ++i) {
		int irq = scn->bus_ops.hif_map_ce_to_irq(scn, i);

		if (irq != scn->wake_irq)
			enable_irq(irq);
	}

	return 0;
}

int hif_apps_wake_irq_disable(struct hif_opaque_softc *hif_ctx)
{
	struct hif_softc *scn;

	QDF_BUG(hif_ctx);
	scn = HIF_GET_SOFTC(hif_ctx);
	if (!scn)
		return -EINVAL;

	disable_irq(scn->wake_irq);

	return 0;
}

int hif_apps_wake_irq_enable(struct hif_opaque_softc *hif_ctx)
{
	struct hif_softc *scn;

	QDF_BUG(hif_ctx);
	scn = HIF_GET_SOFTC(hif_ctx);
	if (!scn)
		return -EINVAL;

	enable_irq(scn->wake_irq);

	return 0;
}

int hif_apps_disable_irq_wake(struct hif_opaque_softc *hif_ctx)
{
	struct hif_softc *scn;

	QDF_BUG(hif_ctx);
	scn = HIF_GET_SOFTC(hif_ctx);
	if (!scn)
		return -EINVAL;

	return disable_irq_wake(scn->wake_irq);
}

int hif_apps_enable_irq_wake(struct hif_opaque_softc *hif_ctx)
{
	struct hif_softc *scn;

	QDF_BUG(hif_ctx);
	scn = HIF_GET_SOFTC(hif_ctx);
	if (!scn)
		return -EINVAL;

	return enable_irq_wake(scn->wake_irq);
}

int hif_apps_disable_irqs_except_wake_irq(struct hif_opaque_softc *hif_ctx)
{
	struct hif_softc *scn;
	int i;

	QDF_BUG(hif_ctx);
	scn = HIF_GET_SOFTC(hif_ctx);
	if (!scn)
		return -EINVAL;

	for (i = 0; i < scn->ce_count; ++i) {
		int irq = scn->bus_ops.hif_map_ce_to_irq(scn, i);

		if (irq != scn->wake_irq)
			disable_irq(irq);
	}

	return 0;
}

int hif_apps_enable_irqs_except_wake_irq(struct hif_opaque_softc *hif_ctx)
{
	struct hif_softc *scn;
	int i;

	QDF_BUG(hif_ctx);
	scn = HIF_GET_SOFTC(hif_ctx);
	if (!scn)
		return -EINVAL;

	for (i = 0; i < scn->ce_count; ++i) {
		int irq = scn->bus_ops.hif_map_ce_to_irq(scn, i);

		if (irq != scn->wake_irq)
			enable_irq(irq);
	}

	return 0;
}

#ifdef WLAN_FEATURE_BMI
bool hif_needs_bmi(struct hif_opaque_softc *scn)
{
	struct hif_softc *hif_sc = HIF_GET_SOFTC(scn);

	return hif_sc->bus_ops.hif_needs_bmi(hif_sc);
}
qdf_export_symbol(hif_needs_bmi);
#endif /* WLAN_FEATURE_BMI */

void hif_config_irq_affinity(struct hif_softc *hif_sc)
{
	hif_sc->bus_ops.hif_config_irq_affinity(hif_sc);
}

int hif_config_irq_by_ceid(struct hif_softc *hif_sc, int ce_id)
{
	return hif_sc->bus_ops.hif_config_irq_by_ceid(hif_sc, ce_id);
}

#ifdef HIF_CPU_CLEAR_AFFINITY
void hif_config_irq_clear_cpu_affinity(struct hif_opaque_softc *scn,
				       int intr_ctxt_id, int cpu)
{
	struct hif_softc *hif_sc = HIF_GET_SOFTC(scn);

	hif_sc->bus_ops.hif_config_irq_clear_cpu_affinity(hif_sc,
							  intr_ctxt_id, cpu);
}

qdf_export_symbol(hif_config_irq_clear_affinity);
#endif

#ifdef HIF_BUS_LOG_INFO
bool hif_log_bus_info(struct hif_softc *hif_sc, uint8_t *data,
		      unsigned int *offset)
{
	if (hif_sc->bus_ops.hif_log_bus_info)
		return hif_sc->bus_ops.hif_log_bus_info(hif_sc, data, offset);

	return false;
}
#endif

int hif_apps_grp_irqs_enable(struct hif_opaque_softc *hif_ctx)
{
	struct hif_exec_context *hif_exec;
	struct hif_softc *scn;
	int i, j;

	QDF_BUG(hif_ctx);
	scn = HIF_GET_SOFTC(hif_ctx);
	if (!scn)
		return -EINVAL;

	for (i = 0 ; i < HIF_MAX_GROUP; i++) {
		hif_exec = hif_exec_get_ctx(hif_ctx, i);
		if (!hif_exec)
			continue;

		for (j = 0; j < hif_exec->numirq; j++)
			pfrm_enable_irq(scn->qdf_dev->dev,
					hif_exec->os_irq[j]);
	}

	return 0;
}

int hif_apps_grp_irqs_disable(struct hif_opaque_softc *hif_ctx)
{
	struct hif_exec_context *hif_exec;
	struct hif_softc *scn;
	int i, j;

	QDF_BUG(hif_ctx);
	scn = HIF_GET_SOFTC(hif_ctx);
	if (!scn)
		return -EINVAL;

	for (i = 0 ; i < HIF_MAX_GROUP; i++) {
		hif_exec = hif_exec_get_ctx(hif_ctx, i);
		if (!hif_exec)
			continue;

		for (j = 0; j < hif_exec->numirq; j++)
			pfrm_disable_irq(scn->qdf_dev->dev,
					 hif_exec->os_irq[j]);
	}

	return 0;
}

int hif_disable_grp_irqs(struct hif_opaque_softc *scn)
{
	struct hif_softc *hif_sc = HIF_GET_SOFTC(scn);

	return hif_sc->bus_ops.hif_disable_grp_irqs(hif_sc);
}

int hif_enable_grp_irqs(struct hif_opaque_softc *scn)
{
	struct hif_softc *hif_sc = HIF_GET_SOFTC(scn);

	return hif_sc->bus_ops.hif_enable_grp_irqs(hif_sc);
}

void hif_set_grp_intr_affinity(struct hif_opaque_softc *scn,
			       uint32_t grp_intr_bitmask, bool perf)
{
	struct hif_softc *hif_sc = HIF_GET_SOFTC(scn);

	if (!hif_sc)
		return;

	hif_sc->bus_ops.hif_set_grp_intr_affinity(hif_sc, grp_intr_bitmask,
						  perf);
}
