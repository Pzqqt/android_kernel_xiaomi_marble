/*
 * Copyright (c) 2016-2018 The Linux Foundation. All rights reserved.
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
int hif_pci_bus_configure(struct hif_softc *scn);
void hif_pci_irq_disable(struct hif_softc *scn, int ce_id);
void hif_pci_irq_enable(struct hif_softc *scn, int ce_id);
int hif_pci_dump_registers(struct hif_softc *scn);
void hif_pci_enable_power_management(struct hif_softc *hif_ctx,
				 bool is_packet_log_enabled);
void hif_pci_disable_power_management(struct hif_softc *hif_ctx);
int hif_pci_configure_grp_irq(struct hif_softc *scn,
			      struct hif_exec_context *exec);
void hif_pci_display_stats(struct hif_softc *hif_ctx);
void hif_pci_clear_stats(struct hif_softc *hif_ctx);
int hif_pci_legacy_map_ce_to_irq(struct hif_softc *scn, int ce_id);
bool hif_pci_needs_bmi(struct hif_softc *scn);
#endif /* _PCI_API_H_ */
