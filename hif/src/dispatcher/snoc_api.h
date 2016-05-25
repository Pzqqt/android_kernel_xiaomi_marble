/*
 * Copyright (c) 2016 The Linux Foundation. All rights reserved.
 *
 * Previously licensed under the ISC license by Qualcomm Atheros, Inc.
 *
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

/*
 * This file was originally distributed by Qualcomm Atheros, Inc.
 * under proprietary terms before Copyright ownership was assigned
 * to the Linux Foundation.
 */

QDF_STATUS hif_snoc_open(struct hif_softc *hif_ctx,
			 enum qdf_bus_type bus_type);
void hif_snoc_close(struct hif_softc *hif_ctx);


void hif_snoc_disable_isr(struct hif_softc *hif_ctx);
void hif_snoc_nointrs(struct hif_softc *scn);
QDF_STATUS hif_snoc_enable_bus(struct hif_softc *ol_sc,
			  struct device *dev, void *bdev,
			  const hif_bus_id *bid,
			  enum hif_enable_type type);
void hif_snoc_disable_bus(struct hif_softc *scn);
int hif_snoc_bus_configure(struct hif_softc *scn);
void hif_snoc_irq_disable(struct hif_softc *scn, int ce_id);
void hif_snoc_irq_enable(struct hif_softc *scn, int ce_id);
int hif_snoc_dump_registers(struct hif_softc *scn);
void hif_snoc_display_stats(struct hif_softc *hif_ctx);
void hif_snoc_clear_stats(struct hif_softc *hif_ctx);
