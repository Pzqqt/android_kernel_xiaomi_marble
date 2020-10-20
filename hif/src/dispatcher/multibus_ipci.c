/*
 * Copyright (c) 2020 The Linux Foundation. All rights reserved.
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

#include "hif.h"
#include "hif_main.h"
#include "multibus.h"
#include "ipci_api.h"
#include "hif_io32.h"
#include "dummy.h"
#include "ce_api.h"

/**
 * hif_initialize_ipci_ops() - initialize the pci ops
 * @bus_ops: hif_bus_ops table pointer to initialize
 *
 * Return: QDF_STATUS_SUCCESS
 */
QDF_STATUS hif_initialize_ipci_ops(struct hif_softc *hif_sc)
{
	struct hif_bus_ops *bus_ops = &hif_sc->bus_ops;

	bus_ops->hif_bus_open = &hif_ipci_open;
	bus_ops->hif_bus_close = &hif_ipci_close;
	bus_ops->hif_bus_prevent_linkdown = &hif_ipci_prevent_linkdown;
	bus_ops->hif_reset_soc = &hif_dummy_reset_soc;
	bus_ops->hif_bus_suspend = &hif_ipci_bus_suspend;
	bus_ops->hif_bus_resume = &hif_ipci_bus_resume;
	bus_ops->hif_bus_suspend_noirq = &hif_ipci_bus_suspend_noirq;
	bus_ops->hif_bus_resume_noirq = &hif_ipci_bus_resume_noirq;
	bus_ops->hif_target_sleep_state_adjust =
		&hif_dummy_target_sleep_state_adjust;
	bus_ops->hif_disable_isr = &hif_ipci_disable_isr;
	bus_ops->hif_nointrs = &hif_ipci_nointrs;
	bus_ops->hif_enable_bus = &hif_ipci_enable_bus;
	bus_ops->hif_disable_bus = &hif_ipci_disable_bus;
#ifdef FEATURE_RUNTIME_PM
	bus_ops->hif_bus_get_rpm_ctx = &hif_ipci_get_rpm_ctx;
	bus_ops->hif_bus_get_dev = &hif_ipci_get_dev;
#endif
	bus_ops->hif_bus_configure = &hif_ipci_bus_configure;
	bus_ops->hif_get_config_item = &hif_dummy_get_config_item;
	bus_ops->hif_set_mailbox_swap = &hif_dummy_set_mailbox_swap;
	bus_ops->hif_claim_device = &hif_dummy_claim_device;
	bus_ops->hif_shutdown_device = &hif_ce_stop;
	bus_ops->hif_stop = &hif_ce_stop;
	bus_ops->hif_cancel_deferred_target_sleep =
		&hif_dummy_cancel_deferred_target_sleep;
	bus_ops->hif_irq_disable = &hif_dummy_irq_disable;
	bus_ops->hif_irq_enable = &hif_dummy_irq_enable;
	bus_ops->hif_dump_registers = &hif_ipci_dump_registers;
	bus_ops->hif_dump_target_memory = &hif_ce_dump_target_memory;
	bus_ops->hif_ipa_get_ce_resource = &hif_ce_ipa_get_ce_resource;
	bus_ops->hif_mask_interrupt_call = &hif_dummy_mask_interrupt_call;
	bus_ops->hif_enable_power_management =
		&hif_ipci_enable_power_management;
	bus_ops->hif_disable_power_management =
		&hif_ipci_disable_power_management;
	bus_ops->hif_grp_irq_configure = &hif_ipci_configure_grp_irq;
	bus_ops->hif_display_stats =
		&hif_ipci_display_stats;
	bus_ops->hif_clear_stats =
		&hif_ipci_clear_stats;
	bus_ops->hif_addr_in_boundary = &hif_dummy_addr_in_boundary;
	bus_ops->hif_needs_bmi = &hif_ipci_needs_bmi;
	bus_ops->hif_config_irq_affinity =
		&hif_dummy_config_irq_affinity;
	bus_ops->hif_config_irq_by_ceid = &hif_dummy_config_irq_by_ceid;
	bus_ops->hif_log_bus_info = &hif_dummy_log_bus_info;

	return QDF_STATUS_SUCCESS;
}

/**
 * hif_ipci_get_context_size() - return the size of the ipci context
 *
 * Return the size of the context.  (0 for invalid bus)
 */
int hif_ipci_get_context_size(void)
{
	return sizeof(struct hif_ipci_softc);
}
