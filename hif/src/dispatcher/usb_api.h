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
#ifndef _USB_API_H_
#define _USB_API_H_
#include "if_usb.h"

QDF_STATUS hif_usb_open(struct hif_softc *hif_ctx,
			 enum qdf_bus_type bus_type);
void hif_usb_close(struct hif_softc *hif_ctx);


void hif_usb_disable_isr(struct hif_softc *hif_ctx);
void hif_usb_nointrs(struct hif_softc *scn);
QDF_STATUS hif_usb_enable_bus(struct hif_softc *ol_sc,
			  struct device *dev, void *bdev,
			  const struct hif_bus_id *bid,
			  enum hif_enable_type type);
void hif_usb_disable_bus(struct hif_softc *scn);
int hif_usb_bus_configure(struct hif_softc *scn);
void hif_usb_irq_disable(struct hif_softc *scn, int ce_id);
void hif_usb_irq_enable(struct hif_softc *scn, int ce_id);
int hif_usb_dump_registers(struct hif_softc *scn);
int hif_usb_bus_suspend(struct hif_softc *hif_ctx);
int hif_usb_bus_resume(struct hif_softc *hif_ctx);
void hif_usb_stop_device(struct hif_softc *hif_sc);
void hif_usb_shutdown_bus_device(struct hif_softc *scn);
int hif_usb_bus_reset_resume(struct hif_softc *hif_ctx);
void hif_usb_set_bundle_mode(struct hif_softc *scn,
			     bool enabled, int rx_bundle_cnt);
void hif_usb_reg_tbl_attach(struct hif_softc *scn);
void hif_fw_assert_ramdump_pattern(struct hif_usb_softc *sc);
void hif_usb_ramdump_handler(struct hif_opaque_softc *scn);
bool hif_usb_needs_bmi(struct hif_softc *scn);
bool hif_is_supported_rx_ctrl_pipe(struct hif_softc *scn);
#endif /*_USB_API_H_*/
