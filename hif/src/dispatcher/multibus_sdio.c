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

#include "hif.h"
#include "hif_main.h"
#include "multibus.h"
#include "sdio_api.h"
#include "hif_io32.h"
#include "dummy.h"
#include "if_sdio.h"

/**
 * hif_initialize_sdio_ops() - initialize the pci ops
 * @bus_ops: hif_bus_ops table pointer to initialize
 *
 * Return: QDF_STATUS_SUCCESS
 */
QDF_STATUS hif_initialize_sdio_ops(struct hif_softc *hif_sc)
{
	struct hif_bus_ops *bus_ops = &hif_sc->bus_ops;

	bus_ops->hif_bus_open = &hif_sdio_open;
	bus_ops->hif_bus_close = &hif_sdio_close;
	bus_ops->hif_bus_prevent_linkdown = &hif_dummy_bus_prevent_linkdown;
	bus_ops->hif_reset_soc = &hif_dummy_reset_soc;
	bus_ops->hif_bus_suspend = &hif_sdio_bus_suspend;
	bus_ops->hif_bus_resume = &hif_sdio_bus_resume;
	bus_ops->hif_target_sleep_state_adjust =
			&hif_dummy_target_sleep_state_adjust;
	bus_ops->hif_disable_isr = &hif_dummy_disable_isr;
	bus_ops->hif_nointrs = &hif_dummy_nointrs;
	bus_ops->hif_enable_bus = &hif_sdio_enable_bus;
	bus_ops->hif_disable_bus = &hif_sdio_disable_bus;
	bus_ops->hif_bus_configure = &hif_dummy_bus_configure;
	bus_ops->hif_get_config_item = &hif_sdio_get_config_item;
	bus_ops->hif_set_mailbox_swap = &hif_sdio_set_mailbox_swap;
	bus_ops->hif_claim_device = &hif_sdio_claim_device;
	bus_ops->hif_shutdown_device = &hif_sdio_shutdown;
	bus_ops->hif_stop = &hif_sdio_stop;
	bus_ops->hif_cancel_deferred_target_sleep =
				&hif_dummy_cancel_deferred_target_sleep;
	bus_ops->hif_irq_disable = &hif_dummy_irq_disable;
	bus_ops->hif_irq_enable = &hif_dummy_irq_enable;
	bus_ops->hif_dump_registers = &hif_dummy_dump_registers;
	bus_ops->hif_dump_target_memory = &hif_dummy_dump_target_memory;
	bus_ops->hif_ipa_get_ce_resource = &hif_dummy_ipa_get_ce_resource;
	bus_ops->hif_mask_interrupt_call = &hif_sdio_mask_interrupt_call;
	bus_ops->hif_enable_power_management =
		&hif_dummy_enable_power_management;
	bus_ops->hif_disable_power_management =
		&hif_dummy_disable_power_management;
	bus_ops->hif_addr_in_boundary = &hif_dummy_addr_in_boundary;

	return QDF_STATUS_SUCCESS;
}

/**
 * hif_sdio_get_context_size() - return the size of the sdio context
 *
 * Return the size of the context.  (0 for invalid bus)
 */
int hif_sdio_get_context_size(void)
{
	return sizeof(struct hif_sdio_softc);
}
