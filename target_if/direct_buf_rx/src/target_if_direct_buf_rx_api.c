/*
 * Copyright (c) 2017 The Linux Foundation. All rights reserved.
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

#include <qdf_status.h>
#include <target_if_direct_buf_rx_api.h>
#include <wlan_objmgr_cmn.h>
#include <wlan_objmgr_global_obj.h>
#include <wlan_objmgr_psoc_obj.h>
#include <wlan_objmgr_cmn.h>
#include "target_if_direct_buf_rx_main.h"

QDF_STATUS direct_buf_rx_attach(struct wlan_objmgr_psoc *psoc)
{
	QDF_STATUS status;

	status = target_if_direct_buf_rx_psoc_obj_create(psoc);

	if (QDF_IS_STATUS_ERROR(status)) {
		direct_buf_rx_err("Failed to create psoc priv obj");
		return status;
	}

	status = wlan_objmgr_register_pdev_create_handler(
			WLAN_TARGET_IF_COMP_DIRECT_BUF_RX,
			target_if_direct_buf_rx_pdev_create_handler,
			NULL);

	if (QDF_IS_STATUS_ERROR(status)) {
		direct_buf_rx_err("Failed to register pdev create handler");
		goto dbr_destroy_psoc_handler;
	}

	status = wlan_objmgr_register_pdev_destroy_handler(
			WLAN_TARGET_IF_COMP_DIRECT_BUF_RX,
			target_if_direct_buf_rx_pdev_destroy_handler,
			NULL);

	if (QDF_IS_STATUS_ERROR(status)) {
		direct_buf_rx_err("Failed to register pdev destroy handler");
		goto dbr_unreg_pdev_create;
	}

	status = target_if_direct_buf_rx_register_events(psoc);

	if (QDF_IS_STATUS_ERROR(status)) {
		direct_buf_rx_err("Direct Buffer RX register events failed");
		goto dbr_unreg_pdev_destroy;
	}

	direct_buf_rx_info("Direct Buffer RX pdev,psoc create and destroy handlers registered");

	return QDF_STATUS_SUCCESS;

dbr_unreg_pdev_destroy:
	status = wlan_objmgr_unregister_pdev_destroy_handler(
			WLAN_TARGET_IF_COMP_DIRECT_BUF_RX,
			target_if_direct_buf_rx_pdev_destroy_handler,
			NULL);

dbr_unreg_pdev_create:
	status = wlan_objmgr_unregister_pdev_create_handler(
			WLAN_TARGET_IF_COMP_DIRECT_BUF_RX,
			target_if_direct_buf_rx_pdev_create_handler,
			NULL);

dbr_destroy_psoc_handler:
	status = target_if_direct_buf_rx_psoc_obj_destroy(psoc);

	return QDF_STATUS_E_FAILURE;
}
qdf_export_symbol(direct_buf_rx_attach);

QDF_STATUS direct_buf_rx_detach(struct wlan_objmgr_psoc *psoc)
{
	QDF_STATUS status;

	status = target_if_direct_buf_rx_unregister_events(psoc);

	if (QDF_IS_STATUS_ERROR(status))
		direct_buf_rx_err("Direct Buffer RX unregister events failed");

	status = wlan_objmgr_unregister_pdev_destroy_handler(
			WLAN_TARGET_IF_COMP_DIRECT_BUF_RX,
			target_if_direct_buf_rx_pdev_destroy_handler,
			NULL);

	if (QDF_IS_STATUS_ERROR(status))
		direct_buf_rx_err("Failed to unregister pdev destroy handler");

	status = wlan_objmgr_unregister_pdev_create_handler(
			WLAN_TARGET_IF_COMP_DIRECT_BUF_RX,
			target_if_direct_buf_rx_pdev_create_handler,
			NULL);

	if (QDF_IS_STATUS_ERROR(status))
		direct_buf_rx_err("Failed to unregister pdev create handler");

	status = target_if_direct_buf_rx_psoc_obj_destroy(psoc);

	if (QDF_IS_STATUS_ERROR(status))
		direct_buf_rx_err("Failed to destroy psoc priv obj");

	direct_buf_rx_info("Direct Buffer RX pdev,psoc create and destroy handlers unregistered");

	return status;
}
qdf_export_symbol(direct_buf_rx_detach);

QDF_STATUS direct_buf_rx_psoc_obj_fill(struct wlan_objmgr_psoc *psoc,
				void *hal_soc, qdf_device_t osdev)
{
	struct direct_buf_rx_psoc_obj *dbr_psoc_obj;

	if (hal_soc == NULL || osdev == NULL) {
		direct_buf_rx_err("hal soc or osdev is null");
		return QDF_STATUS_E_INVAL;
	}

	dbr_psoc_obj = wlan_objmgr_psoc_get_comp_private_obj(psoc,
				WLAN_TARGET_IF_COMP_DIRECT_BUF_RX);

	direct_buf_rx_info("Dbr psoc obj %pK", dbr_psoc_obj);

	if (dbr_psoc_obj == NULL) {
		direct_buf_rx_err("dir buf rx psoc obj is null");
		return QDF_STATUS_E_FAILURE;
	}

	dbr_psoc_obj->hal_soc = hal_soc;
	dbr_psoc_obj->osdev = osdev;

	return QDF_STATUS_SUCCESS;
}

void target_if_direct_buf_rx_register_tx_ops(struct wlan_lmac_if_tx_ops *tx_ops)
{
	tx_ops->dbr_tx_ops.direct_buf_rx_module_register =
				target_if_direct_buf_rx_module_register;
}
qdf_export_symbol(target_if_direct_buf_rx_register_tx_ops);
