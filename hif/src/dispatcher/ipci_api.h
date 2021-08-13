/*
 * Copyright (c) 2020-2021 The Linux Foundation. All rights reserved.
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

#ifndef _IPCI_API_H_
#define _IPCI_API_H_
struct hif_exec_context;

/**
 * hif_ipci_open(): hif_bus_open
 * @hif_ctx: hif context
 * @bus_type: bus type
 *
 * Return: 0 for success or QDF_STATUS_E_NOMEM
 */
QDF_STATUS hif_ipci_open(struct hif_softc *hif_ctx,
			 enum qdf_bus_type bus_type);

/**
 * hif_ipci_close(): hif_bus_close
 * @hif_ctx: hif context
 *
 * Return: n/a
 */
void hif_ipci_close(struct hif_softc *hif_ctx);

/**
 * hif_bus_prevent_linkdown(): allow or permit linkdown
 * @scn: struct hif_softc
 * @flag: true prevents linkdown, false allows
 *
 * Calls into the platform driver to vote against taking down the
 * pcie link.
 *
 * Return: n/a
 */
void hif_ipci_prevent_linkdown(struct hif_softc *scn, bool flag);

/**
 * hif_ipci_bus_suspend(): prepare hif for suspend
 * @scn: struct hif_softc
 *
 * Return: Errno
 */
int hif_ipci_bus_suspend(struct hif_softc *scn);

/**
 * hif_ipci_bus_suspend_noirq() - ensure there are no pending transactions
 * @scn: hif context
 *
 * Ensure that if we received the wakeup message before the irq
 * was disabled that the message is pocessed before suspending.
 *
 * Return: -EBUSY if we fail to flush the tasklets.
 */
int hif_ipci_bus_suspend_noirq(struct hif_softc *scn);

/**
 * hif_ipci_bus_resume(): prepare hif for resume
 * @scn: struct hif_softc
 *
 * Return: Errno
 */
int hif_ipci_bus_resume(struct hif_softc *scn);

/**
 * hif_ipci_bus_resume_noirq() - ensure there are no pending transactions
 * @scn: hif context
 *
 * Ensure that if we received the wakeup message before the irq
 * was disabled that the message is pocessed before suspending.
 *
 * Return: -EBUSY if we fail to flush the tasklets.
 */
int hif_ipci_bus_resume_noirq(struct hif_softc *scn);

/**
 * hif_ipci_disable_isr(): disable interrupt
 * @scn: struct hif_softc
 *
 * Return: n/a
 */
void hif_ipci_disable_isr(struct hif_softc *scn);

/**
 * hif_ipci_nointrs(): disable IRQ
 * @scn: struct hif_softc
 *
 * This function stops interrupt(s)
 *
 * Return: none
 */
void hif_ipci_nointrs(struct hif_softc *scn);

/**
 * hif_ipci_dump_registers(): dump bus debug registers
 * @scn: struct hif_opaque_softc
 *
 * This function dumps hif bus debug registers
 *
 * Return: 0 for success or error code
 */
int hif_ipci_dump_registers(struct hif_softc *scn);

/**
 * hif_ipci_enable_bus(): enable bus
 *
 * This function enables the bus
 *
 * @ol_sc: soft_sc struct
 * @dev: device pointer
 * @bdev: bus dev pointer
 * bid: bus id pointer
 * type: enum hif_enable_type such as HIF_ENABLE_TYPE_PROBE
 * Return: QDF_STATUS
 */
QDF_STATUS hif_ipci_enable_bus(
			struct hif_softc *scn,
			struct device *dev, void *bdev,
			const struct hif_bus_id *bid,
			enum hif_enable_type type);

/**
 * hif_ipci_disable_bus(): hif_disable_bus
 *
 * This function disables the bus
 *
 * @scn: struct hif_softc
 *
 * Return: none
 */
void hif_ipci_disable_bus(struct hif_softc *scn);

#ifdef FEATURE_RUNTIME_PM
/**
 * hif_ipci_get_rpm_ctx() - Map corresponding hif_runtime_pm_ctx
 * @scn: hif context
 *
 * This function will map and return the corresponding
 * hif_runtime_pm_ctx based on ipcie interface.
 *
 * Return: struct hif_runtime_pm_ctx pointer
 */
struct hif_runtime_pm_ctx *hif_ipci_get_rpm_ctx(struct hif_softc *hif_sc);

/**
 * hif_ipci_get_dev() - Map corresponding device structure
 * @scn: hif context
 *
 * This function will map and return the corresponding
 * device structure based on ipcie interface.
 *
 * Return: struct device pointer
 */
struct device *hif_ipci_get_dev(struct hif_softc *hif_sc);
#endif

/**
 * hif_ipci_bus_configure() - configure the pcie bus
 * @hif_sc: pointer to the hif context.
 *
 * return: 0 for success. nonzero for failure.
 */
int hif_ipci_bus_configure(struct hif_softc *scn);

/**
 * hif_ipci_enable_power_management() - enable power management
 * @hif_ctx: hif context
 * @is_packet_log_enabled: pktlog enabled or disabled
 *
 * Return: none
 */
void hif_ipci_enable_power_management(
				struct hif_softc *hif_ctx,
				bool is_packet_log_enabled);

/**
 * hif_ipci_disable_power_management() - disable power management
 * @hif_ctx: hif context
 *
 * Return: none
 */
void hif_ipci_disable_power_management(struct hif_softc *hif_ctx);

/**
 * hif_ipci_configure_grp_irq() - configure HW block irq
 * @scn: hif context
 * @exec: hif exec context
 *
 * Return:Errno
 */
int hif_ipci_configure_grp_irq(
			struct hif_softc *scn,
			struct hif_exec_context *exec);

/**
 * hif_ipci_deconfigure_grp_irq() - deconfigure HW block irq
 * @scn: hif context
 *
 * Return: None
 */
void hif_ipci_deconfigure_grp_irq(struct hif_softc *scn);

/**
 * hif_ipci_display_stats() - display stats
 * @hif_ctx: hif context
 *
 * Return: none
 */
void hif_ipci_display_stats(struct hif_softc *hif_ctx);

/**
 * hif_ipci_clear_stats() - clear stats
 * @hif_ctx: hif context
 *
 * Return: none
 */
void hif_ipci_clear_stats(struct hif_softc *hif_ctx);

/**
 * hif_ipci_needs_bmi() - return true if the soc needs bmi through the driver
 * @scn: hif context
 *
 * Return: true if soc needs driver bmi otherwise false
 */
bool hif_ipci_needs_bmi(struct hif_softc *scn);

/**
 * hif_ipci_get_irq_name() - get irqname
 * This function gives irqnumber to irqname
 * mapping.
 *
 * @irq_no: irq number
 *
 * Return: irq name
 */
const char *hif_ipci_get_irq_name(int irq_no);

/**
 * hif_ipci_enable_grp_irqs(): enable grp IRQs
 * @scn: struct hif_softc
 *
 * This function enables grp irqs
 *
 * Return: 0 if success, error code if failure
 */
int hif_ipci_enable_grp_irqs(struct hif_softc *scn);

#ifdef HIF_CPU_PERF_AFFINE_MASK
/** hif_ipci_config_irq_affinity() - set the irq affinity
 * @scn: hif context
 *
 * set irq affinity hint for wlan irqs to gold cores only for
 * defconfig builds.
 *
 * return: none
 */
void hif_ipci_config_irq_affinity(struct hif_softc *scn);
#endif

#ifdef HIF_CPU_CLEAR_AFFINITY
/**
 * hif_ipci_config_irq_clear_cpu_affinity() - Remove cpu affinity of IRQ
 * @scn: HIF handle
 * @intr_ctxt_id: interrupt group index
 * @cpu: CPU core to clear
 *
 * Return: None
 */
void hif_ipci_config_irq_clear_cpu_affinity(struct hif_softc *scn,
					    int intr_ctxt_id, int cpu);
#else
static inline
void hif_ipci_config_irq_clear_cpu_affinity(struct hif_softc *scn,
					    int intr_ctxt_id, int cpu)
{
}
#endif

/**
 * hif_ipci_disable_grp_irqs(): disable grp IRQs
 * @scn: struct hif_softc
 *
 * This function disables grp irqs
 *
 * Return: 0 if success, error code if failure
 */
int hif_ipci_disable_grp_irqs(struct hif_softc *scn);
#endif /* _IPCI_API_H_ */
