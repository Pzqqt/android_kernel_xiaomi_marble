/*
 * Copyright (c) 2016-2020 The Linux Foundation. All rights reserved.
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

#include "qdf_types.h"
#include "dummy.h"
#include "hif_debug.h"

/**
 * hif_dummy_bus_prevent_linkdown() - prevent linkdown
 * @hif_ctx: hif context
 * @flag: weather to keep the bus alive or not
 *
 * Dummy function for busses and platforms that do not support
 * link down.  This may need to be replaced with a wakelock.
 */
void hif_dummy_bus_prevent_linkdown(struct hif_softc *scn, bool flag)
{
	hif_debug("wlan: %s pcie power collapse ignored",
			(flag ? "disable" : "enable"));
}

/**
 * hif_reset_soc(): reset soc
 *
 * this function resets soc
 *
 * @hif_ctx: HIF context
 *
 * Return: void
 */
/* Function to reset SoC */
void hif_dummy_reset_soc(struct hif_softc *hif_ctx)
{
}

/**
 * hif_dummy_suspend() - suspend the bus
 * @hif_ctx: hif context
 *
 * dummy for busses that don't need to suspend.
 *
 * Return: 0 for success and non-zero for failure
 */
int hif_dummy_bus_suspend(struct hif_softc *hif_ctx)
{
	return 0;
}

/**
 * hif_dummy_resume() - hif resume API
 *
 * This function resumes the bus. but snoc doesn't need to resume.
 * Therefore do nothing.
 *
 * Return: 0 for success and non-zero for failure
 */
int hif_dummy_bus_resume(struct hif_softc *hif_ctx)
{
	return 0;
}

/**
 * hif_dummy_suspend_noirq() - suspend the bus
 * @hif_ctx: hif context
 *
 * dummy for busses that don't need to synchronize
 * with interrupt disable.
 *
 * Return: 0 for success and non-zero for failure
 */
int hif_dummy_bus_suspend_noirq(struct hif_softc *hif_ctx)
{
	return 0;
}

/**
 * hif_dummy_resume_noirq() - resume the bus
 * @hif_ctx: hif context
 *
 * dummy for busses that don't need to synchronize
 * with interrupt disable.
 *
 * Return: 0 for success and non-zero for failure
 */
int hif_dummy_bus_resume_noirq(struct hif_softc *hif_ctx)
{
	return 0;
}

/**
 * hif_dummy_target_sleep_state_adjust() - api to adjust state of target
 * @scn: hif context
 * @sleep_ok: allow or deny target to go to sleep
 * @wait_for_it: ensure target has change
 */
int hif_dummy_target_sleep_state_adjust(struct hif_softc *scn,
						bool sleep_ok, bool wait_for_it)
{
	return 0;
}

/**
 * hif_dummy_enable_power_management - dummy call
 * hif_ctx: hif context
 * is_packet_log_enabled: true if packet log is enabled
 */
void hif_dummy_enable_power_management(struct hif_softc *hif_ctx,
				 bool is_packet_log_enabled)
{}

/**
 * hif_dummy_disable_power_management - dummy call
 * hif_ctx: hif context
 *
 * Return: none
 */
void hif_dummy_disable_power_management(struct hif_softc *hif_ctx)
{}

/**
 * hif_dummy_disable_isr - dummy call
 * hif_ctx: hif context
 *
 * Return: none
 */
void hif_dummy_disable_isr(struct hif_softc *scn)
{}

/**
 * hif_dummy_nointrs - dummy call
 * hif_sc: hif context
 *
 * Return: none
 */
void hif_dummy_nointrs(struct hif_softc *hif_sc)
{}

/**
 * hif_dummy_bus_configure - dummy call
 * hif_ctx: hif context
 *
 * Return: 0 for success
 */
int hif_dummy_bus_configure(struct hif_softc *hif_sc)
{
	return 0;
}

/**
 * hif_dummy_get_config_item - dummy call
 * @hif_sc: hif context
 * @opcode: configuration type
 * @config: configuration value to set
 * @config_len: configuration length
 *
 * Return: QDF_STATUS_SUCCESS for success
 */
QDF_STATUS
hif_dummy_get_config_item(struct hif_softc *hif_sc,
		     int opcode, void *config, uint32_t config_len)
{
	return QDF_STATUS_SUCCESS;
}

/**
 * hif_dummy_set_mailbox_swap - dummy call
 * @hif_sc: hif context
 *
 * Return: None
 */
void
hif_dummy_set_mailbox_swap(struct hif_softc *hif_sc)
{
}

/**
 * hif_dummy_claim_device - dummy call
 * @hif_sc: hif context
 *
 * Return: None
 */
void
hif_dummy_claim_device(struct hif_softc *hif_sc)
{
}

/**
 * hif_dummy_cancel_deferred_target_sleep - dummy call
 * @hif_sc: hif context
 *
 * Return: None
 */
void
hif_dummy_cancel_deferred_target_sleep(struct hif_softc *hif_sc)
{
}

/**
 * hif_dummy_irq_enable - dummy call
 * hif_ctx: hif context
 * @irq_id: irq id
 *
 * Return: none
 */
void hif_dummy_irq_enable(struct hif_softc *hif_sc, int irq_id)
{}

/**
 * hif_dummy_grp_irq_enable - dummy call
 * hif_ctx: hif context
 * @irq_id: grp id
 *
 * Return: none
 */
void hif_dummy_grp_irq_enable(struct hif_softc *hif_sc, uint32_t grp_id)
{}

/**
 * hif_dummy_irq_disable - dummy call
 * hif_ctx: hif context
 * @irq_id: irq id
 *
 * Return: none
 */
void hif_dummy_irq_disable(struct hif_softc *hif_sc, int irq_id)
{}

/**
 * hif_dummy_grp_irq_disable- dummy call
 * hif_ctx: hif context
 * @grp_id: grp id
 *
 * Return: none
 */
void hif_dummy_grp_irq_disable(struct hif_softc *hif_sc, uint32_t grp_id)
{}

/**
 * hif_dummy_grp_irq_configure - dummy call
 * hif_ctx: hif context
 *
 * Return: none
 */
int hif_dummy_grp_irq_configure(struct hif_softc *hif_sc,
				struct hif_exec_context *exec)
{
    return 0;
}

/**
 * hif_dummy_dump_registers - dummy call
 * hif_sc: hif context
 *
 * Return: 0 for success
 */
int hif_dummy_dump_registers(struct hif_softc *hif_sc)
{
	return 0;
}

/**
 * hif_dummy_dump_target_memory - dummy call
 * @hif_sc: hif context
 * @ramdump_base: base
 * @address: address
 * @size: size
 *
 * Return: None
 */
void hif_dummy_dump_target_memory(struct hif_softc *hif_sc, void *ramdump_base,
						uint32_t address, uint32_t size)
{
}

/**
 * hif_dummy_ipa_get_ce_resource - dummy call
 * @scn: HIF context
 * @ce_sr: copyengine source ring resource info
 * @sr_ring_size: source ring size
 * @reg_paddr: bus physical address
 *
 * Return: None
 */
void hif_dummy_ipa_get_ce_resource(struct hif_softc *hif_sc,
				   qdf_shared_mem_t **ce_sr,
				   uint32_t *sr_ring_size,
				   qdf_dma_addr_t *reg_paddr)
{
}

/**
 * hif_dummy_mask_interrupt_call - dummy call
 * @hif_sc: hif context
 *
 * Return: None
 */
void
hif_dummy_mask_interrupt_call(struct hif_softc *hif_sc)
{
}

/**
 * hif_dummy_display_stats - dummy call
 * hif_ctx: hif context
 *
 * Return: none
 */
void hif_dummy_display_stats(struct hif_softc *hif_ctx)
{}

/**
 * hif_dummy_clear_stats - dummy call
 * hif_ctx: hif context
 *
 * Return: none
 */
void hif_dummy_clear_stats(struct hif_softc *hif_ctx)
{}
/**
 * hif_dummy_set_bundle_mode() - dummy call
 * @hif_sc: hif context
 * @enabled: flag to enable/disable bundling
 * @rx_bundle_cnt: bundle count to be used for RX
 *
 * Return: none
 */
void hif_dummy_set_bundle_mode(struct hif_softc *hif_ctx,
					bool enabled, int rx_bundle_cnt)
{
}

/**
 * hif_dummy_bus_reset_resume() - dummy call
 * @hif_sc: hif context
 *
 * Return: int 0 for success, non zero for failure
 */
int hif_dummy_bus_reset_resume(struct hif_softc *hif_ctx)
{
	return 0;
}

int hif_dummy_map_ce_to_irq(struct hif_softc *scn, int ce_id)
{
	hif_err("hif_map_ce_to_irq is not implemented on this platform");
	QDF_BUG(0);
	return -(1);
}

int hif_dummy_addr_in_boundary(struct hif_softc *scn, uint32_t offset)
{
	return 0;
}

/**
 * hif_dummy_config_irq_affinity - dummy call
 * @scn: hif context
 *
 * Return: None
 */
void hif_dummy_config_irq_affinity(struct hif_softc *scn)
{
}
