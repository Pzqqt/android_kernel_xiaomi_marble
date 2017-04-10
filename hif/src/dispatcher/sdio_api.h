/*
 * Copyright (c) 2016-2017 The Linux Foundation. All rights reserved.
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

QDF_STATUS hif_sdio_open(struct hif_softc *hif_sc,
					enum qdf_bus_type bus_type);
void hif_sdio_close(struct hif_softc *hif_sc);
int hif_sdio_bus_suspend(struct hif_softc *hif_ctx);
int hif_sdio_bus_resume(struct hif_softc *hif_ctx);
QDF_STATUS hif_sdio_enable_bus(struct hif_softc *hif_sc,
			struct device *dev, void *bdev,
			const struct hif_bus_id *bid,
			enum hif_enable_type type);
void hif_sdio_disable_bus(struct hif_softc *hif_sc);
QDF_STATUS
hif_sdio_get_config_item(struct hif_softc *hif_sc,
		     int opcode, void *config, uint32_t config_len);
void hif_sdio_set_mailbox_swap(struct hif_softc *hif_sc);
void hif_sdio_claim_device(struct hif_softc *hif_sc);
void hif_sdio_mask_interrupt_call(struct hif_softc *scn);
