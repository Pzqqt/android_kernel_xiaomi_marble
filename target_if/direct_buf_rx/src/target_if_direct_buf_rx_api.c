/*
 * Copyright (c) 2017-2021 The Linux Foundation. All rights reserved.
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
#include <qdf_module.h>
#include <wlan_lmac_if_def.h>

#if defined(WLAN_DEBUGFS) && defined(DIRECT_BUF_RX_DEBUG)
/* Base debugfs entry for DBR module */
qdf_dentry_t dbr_debugfs_entry;

static inline void
target_if_direct_buf_rx_debugfs_init(void)
{
	dbr_debugfs_entry = qdf_debugfs_create_dir("dbr_ring_debug", NULL);

	if (!dbr_debugfs_entry)
		direct_buf_rx_err("error while creating direct_buf rx debugfs dir");
}

static inline void
target_if_direct_buf_rx_debugfs_deinit(void)
{
	if (dbr_debugfs_entry) {
		qdf_debugfs_remove_dir_recursive(dbr_debugfs_entry);
		dbr_debugfs_entry = NULL;
	}
}
#else
static inline void
target_if_direct_buf_rx_debugfs_init(void)
{
}

static inline void
target_if_direct_buf_rx_debugfs_deinit(void)
{
}
#endif /* WLAN_DEBUGFS && DIRECT_BUF_RX_DEBUG */

QDF_STATUS direct_buf_rx_init(void)
{
	QDF_STATUS status;

	status = wlan_objmgr_register_psoc_create_handler(
			WLAN_TARGET_IF_COMP_DIRECT_BUF_RX,
			target_if_direct_buf_rx_psoc_create_handler,
			NULL);

	if (QDF_IS_STATUS_ERROR(status)) {
		direct_buf_rx_err("Failed to register psoc create handler");
		return status;
	}

	status = wlan_objmgr_register_psoc_destroy_handler(
			WLAN_TARGET_IF_COMP_DIRECT_BUF_RX,
			target_if_direct_buf_rx_psoc_destroy_handler,
			NULL);

	if (QDF_IS_STATUS_ERROR(status)) {
		direct_buf_rx_err("Failed to register psoc destroy handler");
		goto dbr_unreg_psoc_create;
	}

	status = wlan_objmgr_register_pdev_create_handler(
			WLAN_TARGET_IF_COMP_DIRECT_BUF_RX,
			target_if_direct_buf_rx_pdev_create_handler,
			NULL);

	if (QDF_IS_STATUS_ERROR(status)) {
		direct_buf_rx_err("Failed to register pdev create handler");
		goto dbr_unreg_psoc_destroy;
	}

	status = wlan_objmgr_register_pdev_destroy_handler(
			WLAN_TARGET_IF_COMP_DIRECT_BUF_RX,
			target_if_direct_buf_rx_pdev_destroy_handler,
			NULL);

	if (QDF_IS_STATUS_ERROR(status)) {
		direct_buf_rx_err("Failed to register pdev destroy handler");
		goto dbr_unreg_pdev_create;
	}

	target_if_direct_buf_rx_debugfs_init();

	direct_buf_rx_info("Direct Buffer RX pdev,psoc create and destroy handlers registered");

	return QDF_STATUS_SUCCESS;

dbr_unreg_pdev_create:
	status = wlan_objmgr_unregister_pdev_create_handler(
			WLAN_TARGET_IF_COMP_DIRECT_BUF_RX,
			target_if_direct_buf_rx_pdev_create_handler,
			NULL);

dbr_unreg_psoc_destroy:
	status = wlan_objmgr_unregister_psoc_destroy_handler(
			WLAN_TARGET_IF_COMP_DIRECT_BUF_RX,
			target_if_direct_buf_rx_psoc_destroy_handler,
			NULL);

dbr_unreg_psoc_create:
	status = wlan_objmgr_unregister_psoc_create_handler(
			WLAN_TARGET_IF_COMP_DIRECT_BUF_RX,
			target_if_direct_buf_rx_psoc_create_handler,
			NULL);

	return QDF_STATUS_E_FAILURE;
}
qdf_export_symbol(direct_buf_rx_init);

QDF_STATUS direct_buf_rx_deinit(void)
{
	QDF_STATUS status;

	target_if_direct_buf_rx_debugfs_deinit();

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

	status = wlan_objmgr_unregister_psoc_destroy_handler(
			WLAN_TARGET_IF_COMP_DIRECT_BUF_RX,
			target_if_direct_buf_rx_psoc_destroy_handler,
			NULL);

	if (QDF_IS_STATUS_ERROR(status))
		direct_buf_rx_err("Failed to unregister psoc destroy handler");

	status = wlan_objmgr_unregister_psoc_create_handler(
			WLAN_TARGET_IF_COMP_DIRECT_BUF_RX,
			target_if_direct_buf_rx_psoc_create_handler,
			NULL);

	if (QDF_IS_STATUS_ERROR(status))
		direct_buf_rx_err("Failed to unregister psoc create handler");

	direct_buf_rx_debug("Direct Buffer RX pdev,psoc create and destroy handlers unregistered");

	return status;
}
qdf_export_symbol(direct_buf_rx_deinit);

QDF_STATUS direct_buf_rx_target_attach(struct wlan_objmgr_psoc *psoc,
				void *hal_soc, qdf_device_t osdev)
{
	struct direct_buf_rx_psoc_obj *dbr_psoc_obj;

	if (!hal_soc || !osdev) {
		direct_buf_rx_err("hal soc or osdev is null");
		return QDF_STATUS_E_INVAL;
	}

	dbr_psoc_obj = wlan_objmgr_psoc_get_comp_private_obj(psoc,
				WLAN_TARGET_IF_COMP_DIRECT_BUF_RX);

	direct_buf_rx_debug("Dbr psoc obj %pK", dbr_psoc_obj);

	if (!dbr_psoc_obj) {
		direct_buf_rx_err("dir buf rx psoc obj is null");
		return QDF_STATUS_E_FAILURE;
	}

	dbr_psoc_obj->hal_soc = hal_soc;
	dbr_psoc_obj->osdev = osdev;

	return QDF_STATUS_SUCCESS;
}

#ifdef DIRECT_BUF_RX_DEBUG
static inline void
target_if_direct_buf_rx_debug_register_tx_ops(
	struct wlan_lmac_if_tx_ops *tx_ops)
{
	tx_ops->dbr_tx_ops.direct_buf_rx_start_ring_debug =
				target_if_dbr_start_ring_debug;
	tx_ops->dbr_tx_ops.direct_buf_rx_stop_ring_debug =
				target_if_dbr_stop_ring_debug;
	tx_ops->dbr_tx_ops.direct_buf_rx_start_buffer_poisoning =
				target_if_dbr_start_buffer_poisoning;
	tx_ops->dbr_tx_ops.direct_buf_rx_stop_buffer_poisoning =
				target_if_dbr_stop_buffer_poisoning;
}
#else
static inline void
target_if_direct_buf_rx_debug_register_tx_ops(
	struct wlan_lmac_if_tx_ops *tx_ops)
{
}
#endif /* DIRECT_BUF_RX_DEBUG */

void target_if_direct_buf_rx_register_tx_ops(struct wlan_lmac_if_tx_ops *tx_ops)
{
	tx_ops->dbr_tx_ops.direct_buf_rx_module_register =
				target_if_direct_buf_rx_module_register;
	tx_ops->dbr_tx_ops.direct_buf_rx_module_unregister =
				target_if_direct_buf_rx_module_unregister;
	tx_ops->dbr_tx_ops.direct_buf_rx_register_events =
				target_if_direct_buf_rx_register_events;
	tx_ops->dbr_tx_ops.direct_buf_rx_unregister_events =
				target_if_direct_buf_rx_unregister_events;
	tx_ops->dbr_tx_ops.direct_buf_rx_print_ring_stat =
				target_if_direct_buf_rx_print_ring_stat;
	tx_ops->dbr_tx_ops.direct_buf_rx_get_ring_params =
				target_if_direct_buf_rx_get_ring_params;
	target_if_direct_buf_rx_debug_register_tx_ops(tx_ops);
}
qdf_export_symbol(target_if_direct_buf_rx_register_tx_ops);

QDF_STATUS target_if_dbr_update_pdev_for_hw_mode_change(
		struct wlan_objmgr_pdev *pdev, int phy_idx)
{
	struct wlan_objmgr_psoc *psoc;
	struct direct_buf_rx_psoc_obj *dbr_psoc_obj;

	psoc = wlan_pdev_get_psoc(pdev);
	if (!psoc) {
		direct_buf_rx_err("psoc is null");
		return QDF_STATUS_E_NULL_VALUE;
	}

	dbr_psoc_obj = wlan_objmgr_psoc_get_comp_private_obj(
			psoc, WLAN_TARGET_IF_COMP_DIRECT_BUF_RX);
	if (!dbr_psoc_obj) {
		direct_buf_rx_err("dir buf rx psoc object is null");
		return QDF_STATUS_E_NULL_VALUE;
	}

	/* Update DBR object in pdev */
	pdev->pdev_comp_priv_obj[WLAN_TARGET_IF_COMP_DIRECT_BUF_RX] =
		(void *)dbr_psoc_obj->dbr_pdev_obj[phy_idx];

	return QDF_STATUS_SUCCESS;
}
