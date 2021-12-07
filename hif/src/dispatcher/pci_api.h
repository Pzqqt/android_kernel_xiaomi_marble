/*
 * Copyright (c) 2016-2021 The Linux Foundation. All rights reserved.
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

#ifndef _PCI_API_H_
#define _PCI_API_H_
struct hif_exec_context;

QDF_STATUS hif_pci_open(struct hif_softc *hif_ctx,
			enum qdf_bus_type bus_type);
void hif_pci_close(struct hif_softc *hif_ctx);
void hif_pci_prevent_linkdown(struct hif_softc *scn, bool flag);
void hif_pci_reset_soc(struct hif_softc *ol_sc);
int hif_pci_bus_suspend(struct hif_softc *scn);
int hif_pci_bus_suspend_noirq(struct hif_softc *scn);
int hif_pci_bus_resume(struct hif_softc *scn);
int hif_pci_bus_resume_noirq(struct hif_softc *scn);
int hif_pci_target_sleep_state_adjust(struct hif_softc *scn,
			bool sleep_ok, bool wait_for_it);

void hif_pci_disable_isr(struct hif_softc *scn);
void hif_pci_nointrs(struct hif_softc *scn);
QDF_STATUS hif_pci_enable_bus(struct hif_softc *scn,
			struct device *dev, void *bdev,
			const struct hif_bus_id *bid,
			enum hif_enable_type type);
void hif_pci_disable_bus(struct hif_softc *scn);
#ifdef FEATURE_RUNTIME_PM
struct hif_runtime_pm_ctx *hif_pci_get_rpm_ctx(struct hif_softc *hif_sc);
struct device *hif_pci_get_dev(struct hif_softc *hif_sc);
#endif
int hif_pci_bus_configure(struct hif_softc *scn);
void hif_pci_irq_disable(struct hif_softc *scn, int ce_id);
void hif_pci_irq_enable(struct hif_softc *scn, int ce_id);
int hif_pci_dump_registers(struct hif_softc *scn);
void hif_pci_enable_power_management(struct hif_softc *hif_ctx,
				 bool is_packet_log_enabled);
void hif_pci_disable_power_management(struct hif_softc *hif_ctx);
int hif_pci_configure_grp_irq(struct hif_softc *scn,
			      struct hif_exec_context *exec);
void hif_pci_deconfigure_grp_irq(struct hif_softc *scn);

/**
 * hif_pci_reg_read32() - Read register in 32bits
 * @hif_sc: PCIe control struct
 * @offset: The register offset
 *
 * This function will read register in 32bits
 *
 * Return: return value for register with specified offset
 */
uint32_t hif_pci_reg_read32(struct hif_softc *hif_sc,
			    uint32_t offset);

/**
 * hif_pci_reg_write32() - Write register in 32bits
 * @hif_sc: PCIe control struct
 * @offset: The register offset
 * @value: The value need to be written
 *
 * This function will write register in 32bits
 *
 * Return: None
 */
void hif_pci_reg_write32(struct hif_softc *hif_sc,
			 uint32_t offset,
			 uint32_t value);

void hif_pci_display_stats(struct hif_softc *hif_ctx);
void hif_pci_clear_stats(struct hif_softc *hif_ctx);
int hif_pci_legacy_map_ce_to_irq(struct hif_softc *scn, int ce_id);
bool hif_pci_needs_bmi(struct hif_softc *scn);
const char *hif_pci_get_irq_name(int irq_no);

/** hif_pci_config_irq_affinity() - Set the IRQ affinity
 * @scn: hif context
 *
 * Set IRQ affinity hint for WLAN IRQs to gold cores only for
 * defconfig builds.
 *
 * Return: None
 */
void hif_pci_config_irq_affinity(struct hif_softc *scn);
int hif_ce_msi_configure_irq_by_ceid(struct hif_softc *scn, int ce_id);

#ifdef FEATURE_IRQ_AFFINITY
/*
 * hif_pci_set_grp_intr_affinity() - Set irq affinity hint for grp
 *  intrs based on bitmask
 * @scn: hif context
 * @grp_intr_bitmask:
 * @perf: affine to perf or non-perf cluster
 *
 * Return: None
 */
void hif_pci_set_grp_intr_affinity(struct hif_softc *scn,
				   uint32_t grp_intr_bitmask, bool perf);
#endif
#endif /* _PCI_API_H_ */
