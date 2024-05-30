/*
 * Copyright (c) 2013-2018,2020-2021 The Linux Foundation. All rights reserved.
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

#ifndef __AHB_API_H
#define __AHB_API_H
struct hif_exec_context;

QDF_STATUS hif_ahb_open(struct hif_softc *hif_ctx,
			 enum qdf_bus_type bus_type);
void hif_ahb_close(struct hif_softc *hif_ctx);


void hif_ahb_disable_isr(struct hif_softc *hif_ctx);
void hif_ahb_nointrs(struct hif_softc *scn);
void hif_ahb_reset_soc(struct hif_softc *hif_ctx);
QDF_STATUS hif_ahb_enable_bus(struct hif_softc *ol_sc,
			  struct device *dev, void *bdev,
			  const struct hif_bus_id *bid,
			  enum hif_enable_type type);
void hif_ahb_disable_bus(struct hif_softc *scn);
int hif_ahb_bus_configure(struct hif_softc *scn);
void hif_ahb_irq_disable(struct hif_softc *scn, int ce_id);
void hif_ahb_irq_enable(struct hif_softc *scn, int ce_id);
void hif_ahb_exec_grp_irq_disable(struct hif_exec_context *hif_ext_grp);
void hif_ahb_exec_grp_irq_enable(struct hif_exec_context *hif_ext_grp);
int hif_ahb_dump_registers(struct hif_softc *scn);

int hif_ahb_configure_legacy_irq(struct hif_pci_softc *sc);
int hif_ahb_clk_enable_disable(struct device *dev, int enable);
void hif_ahb_device_reset(struct hif_softc *scn);
int hif_ahb_enable_radio(struct hif_pci_softc *sc,
		struct platform_device *pdev,
		const struct platform_device_id *id);
int hif_ahb_configure_irq(struct hif_pci_softc *sc);
int hif_ahb_configure_irq_by_ceid(struct hif_softc *sc, int ce_id);
int hif_ahb_configure_grp_irq(struct hif_softc *scn,
			      struct hif_exec_context *hif_ext_grp);
void hif_ahb_deconfigure_grp_irq(struct hif_softc *scn);
bool hif_ahb_needs_bmi(struct hif_softc *scn);
void hif_ahb_display_stats(struct hif_softc *scn);
void hif_ahb_clear_stats(struct hif_softc *scn);
#endif
