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
struct hif_softc;

void hif_dummy_bus_prevent_linkdown(struct hif_softc *scn, bool flag);
void hif_dummy_reset_soc(struct hif_softc *scn);
int hif_dummy_bus_suspend(struct hif_softc *hif_ctx);
int hif_dummy_bus_resume(struct hif_softc *hif_ctx);
int hif_dummy_bus_suspend_noirq(struct hif_softc *hif_ctx);
int hif_dummy_bus_resume_noirq(struct hif_softc *hif_ctx);
int hif_dummy_target_sleep_state_adjust(struct hif_softc *scn,
					bool sleep_ok, bool wait_for_it);
void hif_dummy_enable_power_management(struct hif_softc *hif_ctx,
				 bool is_packet_log_enabled);
void hif_dummy_disable_power_management(struct hif_softc *hif_ctx);
void hif_dummy_disable_isr(struct hif_softc *scn);
void hif_dummy_nointrs(struct hif_softc *hif_sc);
int hif_dummy_bus_configure(struct hif_softc *hif_sc);
QDF_STATUS hif_dummy_get_config_item(struct hif_softc *hif_sc,
		     int opcode, void *config, uint32_t config_len);
void hif_dummy_set_mailbox_swap(struct hif_softc *hif_sc);
void hif_dummy_claim_device(struct hif_softc *hif_sc);
void hif_dummy_cancel_deferred_target_sleep(struct hif_softc *hif_sc);
void hif_dummy_irq_enable(struct hif_softc *hif_sc, int irq_id);
void hif_dummy_irq_disable(struct hif_softc *hif_sc, int irq_id);
int hif_dummy_dump_registers(struct hif_softc *hif_sc);
void hif_dummy_dump_target_memory(struct hif_softc *hif_sc, void *ramdump_base,
				  uint32_t address, uint32_t size);
void hif_dummy_ipa_get_ce_resource(struct hif_softc *hif_sc,
				   qdf_dma_addr_t *sr_base_paddr,
				   uint32_t *sr_ring_size,
				   qdf_dma_addr_t *reg_paddr);
void hif_dummy_mask_interrupt_call(struct hif_softc *hif_sc);
void hif_dummy_display_stats(struct hif_softc *hif_ctx);
void hif_dummy_clear_stats(struct hif_softc *hif_ctx);
void hif_dummy_set_bundle_mode(struct hif_softc *hif_ctx,
					bool enabled, int rx_bundle_cnt);
int hif_dummy_bus_reset_resume(struct hif_softc *hif_ctx);

