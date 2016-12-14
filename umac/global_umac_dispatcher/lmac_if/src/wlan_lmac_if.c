/*
 * Copyright (c) 2016 The Linux Foundation. All rights reserved.
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

#include "qdf_mem.h"
#include "wlan_lmac_if_def.h"
#include "wlan_lmac_if_api.h"

/* Function pointer to call DA/OL specific tx_ops registration function */
QDF_STATUS (*wlan_lmac_if_tx_ops_create_handler[MAX_DEV_TYPE])
				(struct wlan_lmac_if_tx_ops *tx_ops);

/**
 * wlan_lmac_if_register_rx_handlers() - UMAC rx handler register
 * @rx_ops: Pointer to rx_ops structure to be populated
 *
 * Register umac RX callabacks which will be called by DA/OL/WMA/WMI
 *
 * Return: QDF_STATUS_SUCCESS - in case of success
 */
QDF_STATUS
wlan_lmac_if_register_rx_handlers(struct wlan_lmac_if_rx_ops *rx_ops)
{
	/* Component specific public api's to be called to register
	 * respective callbacks
	 * Ex: rx_ops->fp = function;
	 */

	return QDF_STATUS_SUCCESS;
}

/**
 * wlan_lmac_if_open() - lmac_if open
 * @psoc: psoc context
 *
 * Opens up lmac_if southbound layer. This function calls OL,DA and UMAC
 * modules to register respective tx and rx callbacks.
 *
 * Return: Pointer to global psoc object
 */
struct wlan_objmgr_psoc *wlan_lmac_if_open(struct wlan_objmgr_psoc *psoc)
{
	WLAN_DEV_TYPE dev_type;

	dev_type = psoc->soc_nif.phy_type;

	if (dev_type == WLAN_DEV_DA) {
		wlan_lmac_if_tx_ops_create_handler[WLAN_DEV_DA]
					(&psoc->soc_cb.tx_ops);
	} else if (dev_type == WLAN_DEV_OL) {
		wlan_lmac_if_tx_ops_create_handler[WLAN_DEV_OL]
					(&psoc->soc_cb.tx_ops);
	} else {
		/* Control should ideally not reach here */
		qdf_print("Invalid device type\n");
		return psoc;
	}

	/* Function call into umac to register rx-ops handlers */
	wlan_lmac_if_register_rx_handlers(&psoc->soc_cb.rx_ops);

	return psoc;
}
EXPORT_SYMBOL(wlan_lmac_if_open);

/**
 * wlan_lmac_if_close() - Close lmac_if
 * @psoc: psoc context
 *
 * Deregister lmac_if TX and RX handlers
 *
 * Return: QDF_STATUS_SUCCESS - in case of success
 */
QDF_STATUS wlan_lmac_if_close(struct wlan_objmgr_psoc *psoc)
{
	qdf_mem_set(&psoc->soc_cb.tx_ops, 0, sizeof(psoc->soc_cb.tx_ops));
	qdf_mem_set(&psoc->soc_cb.rx_ops, 0, sizeof(psoc->soc_cb.rx_ops));

	return QDF_STATUS_SUCCESS;
}
EXPORT_SYMBOL(wlan_lmac_if_close);

/**
 * wlan_lmac_if_assign_tx_registration_cb() -tx registration callback assignment
 * @dev_type: Dev type can be either Direct attach or Offload
 * @handler: handler to be called for LMAC tx ops registration
 *
 * API to assign appropriate tx registration callback handler based on the
 * device type(Offload or Direct attach)
 *
 * Return: QDF_STATUS_SUCCESS - in case of success
 */
QDF_STATUS wlan_lmac_if_assign_tx_registration_cb(WLAN_DEV_TYPE dev_type,
			QDF_STATUS (*handler)(struct wlan_lmac_if_tx_ops *))
{
	wlan_lmac_if_tx_ops_create_handler[dev_type] = handler;
	return QDF_STATUS_SUCCESS;
}
EXPORT_SYMBOL(wlan_lmac_if_assign_tx_registration_cb);
