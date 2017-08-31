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
#include "usb_api.h"
#include "hif_io32.h"
#include "dummy.h"
#include "if_usb.h"

/**
 * hif_initialize_usb_ops() - initialize the usb ops
 * @bus_ops: hif_bus_ops table pointer to initialize
 *
 * Return: QDF_STATUS_SUCCESS
 */
QDF_STATUS hif_initialize_usb_ops(struct hif_bus_ops *bus_ops)
{
	bus_ops->hif_bus_open = &hif_usb_open;
	bus_ops->hif_bus_close = &hif_usb_close;
	bus_ops->hif_bus_prevent_linkdown = &hif_dummy_bus_prevent_linkdown;
	bus_ops->hif_reset_soc = &hif_dummy_reset_soc;
	bus_ops->hif_bus_suspend = &hif_usb_bus_suspend;
	bus_ops->hif_bus_resume = &hif_usb_bus_resume;
	bus_ops->hif_target_sleep_state_adjust =
			&hif_dummy_target_sleep_state_adjust;
	bus_ops->hif_disable_isr = &hif_usb_disable_isr;
	bus_ops->hif_nointrs = &hif_usb_nointrs;
	bus_ops->hif_enable_bus = &hif_usb_enable_bus;
	bus_ops->hif_disable_bus = &hif_usb_disable_bus;
	bus_ops->hif_bus_configure = &hif_usb_bus_configure;
	bus_ops->hif_get_config_item = &hif_dummy_get_config_item;
	bus_ops->hif_set_mailbox_swap = &hif_dummy_set_mailbox_swap;
	bus_ops->hif_claim_device = &hif_dummy_claim_device;
	bus_ops->hif_shutdown_device = &hif_usb_shutdown_bus_device;
	bus_ops->hif_stop = &hif_usb_stop_device;
	bus_ops->hif_bus_pkt_dl_len_set = &hif_dummy_bus_pkt_dl_len_set;
	bus_ops->hif_cancel_deferred_target_sleep =
				&hif_dummy_cancel_deferred_target_sleep;
	bus_ops->hif_irq_disable = &hif_usb_irq_disable;
	bus_ops->hif_irq_enable = &hif_usb_irq_enable;
	bus_ops->hif_dump_registers = &hif_dummy_dump_registers;
	bus_ops->hif_dump_target_memory = &hif_dummy_dump_target_memory;
	bus_ops->hif_ipa_get_ce_resource = &hif_dummy_ipa_get_ce_resource;
	bus_ops->hif_enable_power_management =
			&hif_dummy_enable_power_management;
	bus_ops->hif_disable_power_management =
			&hif_dummy_disable_power_management;
	bus_ops->hif_set_bundle_mode = hif_usb_set_bundle_mode;
	bus_ops->hif_bus_reset_resume = hif_usb_bus_reset_resume;
	bus_ops->hif_addr_in_boundary = &hif_dummy_addr_in_boundary;

	return QDF_STATUS_SUCCESS;
}

/**
 * hif_usb_get_context_size() - return the size of the usb context
 *
 * Return the size of the context.  (0 for invalid bus)
 */
int hif_usb_get_context_size(void)
{
	return sizeof(struct hif_usb_softc);
}
