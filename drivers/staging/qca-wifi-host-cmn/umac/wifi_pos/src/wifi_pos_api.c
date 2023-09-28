/*
 * Copyright (c) 2017-2020 The Linux Foundation. All rights reserved.
 * Copyright (c) 2021 Qualcomm Innovation Center, Inc. All rights reserved.
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
/**
 * DOC: wifi_pos_api.c
 * This file defines the APIs wifi_pos component.
 */

#include <wlan_lmac_if_def.h>
#include "wifi_pos_api.h"
#include "wifi_pos_utils_i.h"
#include "wifi_pos_main_i.h"
#include "os_if_wifi_pos.h"
#include "target_if_wifi_pos.h"
#include "wlan_objmgr_cmn.h"
#include "wlan_objmgr_global_obj.h"
#include "wlan_objmgr_psoc_obj.h"
#include "wlan_lmac_if_def.h"

QDF_STATUS wifi_pos_init(void)
{
	QDF_STATUS status;

	wifi_pos_lock_init();

	/* register psoc create handler functions. */
	status = wlan_objmgr_register_psoc_create_handler(
		WLAN_UMAC_COMP_WIFI_POS,
		wifi_pos_psoc_obj_created_notification,
		NULL);
	if (QDF_IS_STATUS_ERROR(status)) {
		wifi_pos_err("register_psoc_create_handler failed, status: %d",
			     status);
		return status;
	}

	/* register psoc delete handler functions. */
	status = wlan_objmgr_register_psoc_destroy_handler(
		WLAN_UMAC_COMP_WIFI_POS,
		wifi_pos_psoc_obj_destroyed_notification,
		NULL);
	if (QDF_IS_STATUS_ERROR(status)) {
		wifi_pos_err("register_psoc_destroy_handler failed, status: %d",
			     status);
	}

	return status;
}

QDF_STATUS wifi_pos_deinit(void)
{
	QDF_STATUS status;

	/* deregister psoc create handler functions. */
	status = wlan_objmgr_unregister_psoc_create_handler(
				WLAN_UMAC_COMP_WIFI_POS,
				wifi_pos_psoc_obj_created_notification,
				NULL);
	if (QDF_IS_STATUS_ERROR(status)) {
		wifi_pos_err("unregister_psoc_create_handler failed, status: %d",
			     status);
		return status;
	}

	/* deregister psoc delete handler functions. */
	status = wlan_objmgr_unregister_psoc_destroy_handler(
				WLAN_UMAC_COMP_WIFI_POS,
				wifi_pos_psoc_obj_destroyed_notification,
				NULL);
	if (QDF_IS_STATUS_ERROR(status)) {
		wifi_pos_err("unregister_psoc_destroy_handler failed, status: %d",
			     status);
	}

	wifi_pos_lock_deinit();

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS wifi_pos_psoc_enable(struct wlan_objmgr_psoc *psoc)
{
	QDF_STATUS status;
	struct wlan_lmac_if_wifi_pos_tx_ops *tx_ops;

	tx_ops = wifi_pos_get_tx_ops(psoc);
	if (!tx_ops) {
		wifi_pos_err("tx_ops is null");
		return QDF_STATUS_E_NULL_VALUE;
	}

	status = tx_ops->wifi_pos_register_events(psoc);

	if (QDF_IS_STATUS_ERROR(status))
		wifi_pos_err("target_if_wifi_pos_register_events failed");

	return status;
}

QDF_STATUS wifi_pos_psoc_disable(struct wlan_objmgr_psoc *psoc)
{
	QDF_STATUS status;
	struct wlan_lmac_if_wifi_pos_tx_ops *tx_ops;

	tx_ops = wifi_pos_get_tx_ops(psoc);
	if (!tx_ops) {
		wifi_pos_err("tx_ops is null");
		return QDF_STATUS_E_NULL_VALUE;
	}

	status = tx_ops->wifi_pos_deregister_events(psoc);

	if (QDF_IS_STATUS_ERROR(status))
		wifi_pos_err("target_if_wifi_pos_deregister_events failed");

	return QDF_STATUS_SUCCESS;
}

void wifi_pos_set_oem_target_type(struct wlan_objmgr_psoc *psoc, uint32_t val)
{
	struct wifi_pos_psoc_priv_obj *wifi_pos_psoc =
			wifi_pos_get_psoc_priv_obj(psoc);

	if (!wifi_pos_psoc) {
		wifi_pos_err("wifi_pos priv obj is null");
		return;
	}

	qdf_spin_lock_bh(&wifi_pos_psoc->wifi_pos_lock);
	wifi_pos_psoc->oem_target_type = val;
	qdf_spin_unlock_bh(&wifi_pos_psoc->wifi_pos_lock);
}

void wifi_pos_set_oem_fw_version(struct wlan_objmgr_psoc *psoc, uint32_t val)
{
	struct wifi_pos_psoc_priv_obj *wifi_pos_psoc =
			wifi_pos_get_psoc_priv_obj(psoc);

	if (!wifi_pos_psoc) {
		wifi_pos_err("wifi_pos priv obj is null");
		return;
	}

	qdf_spin_lock_bh(&wifi_pos_psoc->wifi_pos_lock);
	wifi_pos_psoc->oem_fw_version = val;
	qdf_spin_unlock_bh(&wifi_pos_psoc->wifi_pos_lock);
}

void wifi_pos_set_drv_ver_major(struct wlan_objmgr_psoc *psoc, uint8_t val)
{
	struct wifi_pos_psoc_priv_obj *wifi_pos_psoc =
			wifi_pos_get_psoc_priv_obj(psoc);

	if (!wifi_pos_psoc) {
		wifi_pos_err("wifi_pos priv obj is null");
		return;
	}

	qdf_spin_lock_bh(&wifi_pos_psoc->wifi_pos_lock);
	wifi_pos_psoc->driver_version.major = val;
	qdf_spin_unlock_bh(&wifi_pos_psoc->wifi_pos_lock);
}

void wifi_pos_set_drv_ver_minor(struct wlan_objmgr_psoc *psoc, uint8_t val)
{
	struct wifi_pos_psoc_priv_obj *wifi_pos_psoc =
			wifi_pos_get_psoc_priv_obj(psoc);

	if (!wifi_pos_psoc) {
		wifi_pos_err("wifi_pos priv obj is null");
		return;
	}

	qdf_spin_lock_bh(&wifi_pos_psoc->wifi_pos_lock);
	wifi_pos_psoc->driver_version.minor = val;
	qdf_spin_unlock_bh(&wifi_pos_psoc->wifi_pos_lock);
}

void wifi_pos_set_drv_ver_patch(struct wlan_objmgr_psoc *psoc, uint8_t val)
{
	struct wifi_pos_psoc_priv_obj *wifi_pos_psoc =
			wifi_pos_get_psoc_priv_obj(psoc);

	if (!wifi_pos_psoc) {
		wifi_pos_err("wifi_pos priv obj is null");
		return;
	}

	qdf_spin_lock_bh(&wifi_pos_psoc->wifi_pos_lock);
	wifi_pos_psoc->driver_version.patch = val;
	qdf_spin_unlock_bh(&wifi_pos_psoc->wifi_pos_lock);
}

void wifi_pos_set_drv_ver_build(struct wlan_objmgr_psoc *psoc, uint8_t val)
{
	struct wifi_pos_psoc_priv_obj *wifi_pos_psoc =
			wifi_pos_get_psoc_priv_obj(psoc);

	if (!wifi_pos_psoc) {
		wifi_pos_err("wifi_pos priv obj is null");
		return;
	}

	qdf_spin_lock_bh(&wifi_pos_psoc->wifi_pos_lock);
	wifi_pos_psoc->driver_version.build = val;
	qdf_spin_unlock_bh(&wifi_pos_psoc->wifi_pos_lock);
}

void wifi_pos_set_dwell_time_min(struct wlan_objmgr_psoc *psoc, uint16_t val)
{
	struct wifi_pos_psoc_priv_obj *wifi_pos_psoc =
			wifi_pos_get_psoc_priv_obj(psoc);

	if (!wifi_pos_psoc) {
		wifi_pos_err("wifi_pos priv obj is null");
		return;
	}

	qdf_spin_lock_bh(&wifi_pos_psoc->wifi_pos_lock);
	wifi_pos_psoc->allowed_dwell_time_min = val;
	qdf_spin_unlock_bh(&wifi_pos_psoc->wifi_pos_lock);
}
void wifi_pos_set_dwell_time_max(struct wlan_objmgr_psoc *psoc, uint16_t val)
{
	struct wifi_pos_psoc_priv_obj *wifi_pos_psoc =
			wifi_pos_get_psoc_priv_obj(psoc);

	if (!wifi_pos_psoc) {
		wifi_pos_err("wifi_pos priv obj is null");
		return;
	}

	qdf_spin_lock_bh(&wifi_pos_psoc->wifi_pos_lock);
	wifi_pos_psoc->allowed_dwell_time_max = val;
	qdf_spin_unlock_bh(&wifi_pos_psoc->wifi_pos_lock);
}

void wifi_pos_set_current_dwell_time_max(struct wlan_objmgr_psoc *psoc,
					 uint16_t val)
{
	struct wifi_pos_psoc_priv_obj *wifi_pos_psoc =
			wifi_pos_get_psoc_priv_obj(psoc);

	if (!wifi_pos_psoc) {
		wifi_pos_err("wifi_pos priv obj is null");
		return;
	}

	qdf_spin_lock_bh(&wifi_pos_psoc->wifi_pos_lock);
	wifi_pos_psoc->current_dwell_time_max = val;
	qdf_spin_unlock_bh(&wifi_pos_psoc->wifi_pos_lock);
}

void wifi_pos_set_current_dwell_time_min(struct wlan_objmgr_psoc *psoc,
					 uint16_t val)
{
	struct wifi_pos_psoc_priv_obj *wifi_pos_psoc =
			wifi_pos_get_psoc_priv_obj(psoc);

	if (!wifi_pos_psoc) {
		wifi_pos_err("wifi_pos priv obj is null");
		return;
	}

	qdf_spin_lock_bh(&wifi_pos_psoc->wifi_pos_lock);
	wifi_pos_psoc->current_dwell_time_max = val;
	qdf_spin_unlock_bh(&wifi_pos_psoc->wifi_pos_lock);
}

uint32_t wifi_pos_get_app_pid(struct wlan_objmgr_psoc *psoc)
{
	uint32_t app_pid;
	struct wifi_pos_psoc_priv_obj *wifi_pos_psoc =
				wifi_pos_get_psoc_priv_obj(psoc);

	if (!wifi_pos_psoc) {
		wifi_pos_err("wifi_pos priv obj is null");
		return 0;
	}

	qdf_spin_lock_bh(&wifi_pos_psoc->wifi_pos_lock);
	app_pid = wifi_pos_psoc->app_pid;
	qdf_spin_unlock_bh(&wifi_pos_psoc->wifi_pos_lock);

	return app_pid;

}

bool wifi_pos_is_app_registered(struct wlan_objmgr_psoc *psoc)
{
	bool is_app_registered;
	struct wifi_pos_psoc_priv_obj *wifi_pos_psoc =
				wifi_pos_get_psoc_priv_obj(psoc);

	if (!wifi_pos_psoc) {
		wifi_pos_err("wifi_pos priv obj is null");
		return false;
	}

	qdf_spin_lock_bh(&wifi_pos_psoc->wifi_pos_lock);
	is_app_registered = wifi_pos_psoc->is_app_registered;
	qdf_spin_unlock_bh(&wifi_pos_psoc->wifi_pos_lock);

	return is_app_registered;
}

#ifdef WLAN_FEATURE_CIF_CFR
QDF_STATUS wifi_pos_init_cir_cfr_rings(struct wlan_objmgr_psoc *psoc,
				   void *hal_soc, uint8_t num_mac, void *buf)
{
	return target_if_wifi_pos_init_cir_cfr_rings(psoc, hal_soc,
						     num_mac, buf);
}
#endif

QDF_STATUS
wifi_pos_register_get_phy_mode_cb(struct wlan_objmgr_psoc *psoc,
				  void (*handler)(qdf_freq_t, uint32_t,
						  uint32_t *))
{
	struct wifi_pos_psoc_priv_obj *wifi_pos_psoc;

	if (!psoc) {
		wifi_pos_err("psoc is null");
		return QDF_STATUS_E_NULL_VALUE;
	}

	if (!handler) {
		wifi_pos_err("Null callback");
		return QDF_STATUS_E_NULL_VALUE;
	}
	wifi_pos_psoc = wifi_pos_get_psoc_priv_obj(psoc);
	if (!wifi_pos_psoc) {
		wifi_pos_err("wifi_pos priv obj is null");
		return QDF_STATUS_E_NULL_VALUE;
	}

	wifi_pos_psoc->wifi_pos_get_phy_mode = handler;

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS wifi_pos_register_get_fw_phy_mode_for_freq_cb(
				struct wlan_objmgr_psoc *psoc,
				void (*handler)(uint32_t, uint32_t, uint32_t *))
{
	struct wifi_pos_psoc_priv_obj *wifi_pos_psoc;

	if (!psoc) {
		wifi_pos_err("psoc is null");
		return QDF_STATUS_E_NULL_VALUE;
	}

	if (!handler) {
		wifi_pos_err("Null callback");
		return QDF_STATUS_E_NULL_VALUE;
	}
	wifi_pos_psoc = wifi_pos_get_psoc_priv_obj(psoc);
	if (!wifi_pos_psoc) {
		wifi_pos_err("wifi_pos priv obj is null");
		return QDF_STATUS_E_NULL_VALUE;
	}

	wifi_pos_psoc->wifi_pos_get_fw_phy_mode_for_freq = handler;

	return QDF_STATUS_SUCCESS;
}

#ifndef CNSS_GENL
QDF_STATUS wifi_pos_register_get_pdev_id_by_dev_name(
		struct wlan_objmgr_psoc *psoc,
		QDF_STATUS (*handler)(char *dev_name, uint8_t *pdev_id,
				      struct wlan_objmgr_psoc **psoc))
{
	struct wifi_pos_psoc_priv_obj *wifi_pos_psoc;

	if (!psoc) {
		wifi_pos_err("psoc is null");
		return QDF_STATUS_E_NULL_VALUE;
	}

	if (!handler) {
		wifi_pos_err("Null callback");
		return QDF_STATUS_E_NULL_VALUE;
	}

	wifi_pos_psoc = wifi_pos_get_psoc_priv_obj(psoc);
	if (!wifi_pos_psoc) {
		wifi_pos_err("wifi_pos priv obj is null");
		return QDF_STATUS_E_NULL_VALUE;
	}

	wifi_pos_psoc->wifi_pos_get_pdev_id_by_dev_name = handler;

	return QDF_STATUS_SUCCESS;
}

#ifdef WLAN_RTT_MEASUREMENT_NOTIFICATION
QDF_STATUS wifi_pos_register_measurement_request_notification(
		struct wlan_objmgr_psoc *psoc,
		QDF_STATUS (*handler)(struct wlan_objmgr_pdev *pdev,
				      struct rtt_channel_info *chinfo))
{
	struct wifi_pos_psoc_priv_obj *wifi_pos_psoc;

	if (!psoc) {
		wifi_pos_err("psoc is null");
		return QDF_STATUS_E_NULL_VALUE;
	}

	if (!handler) {
		wifi_pos_err("Null callback");
		return QDF_STATUS_E_NULL_VALUE;
	}

	wifi_pos_psoc = wifi_pos_get_psoc_priv_obj(psoc);
	if (!wifi_pos_psoc) {
		wifi_pos_err("wifi_pos priv obj is null");
		return QDF_STATUS_E_NULL_VALUE;
	}

	wifi_pos_psoc->wifi_pos_measurement_request_notification = handler;

	return QDF_STATUS_SUCCESS;
}
#endif /* WLAN_RTT_MEASUREMENT_NOTIFICATION */
#endif /* CNSS_GENL */

QDF_STATUS wifi_pos_register_send_action(
				struct wlan_objmgr_psoc *psoc,
				void (*handler)(struct wlan_objmgr_psoc *psoc,
						uint32_t sub_type,
						uint8_t *buf,
						uint32_t buf_len))
{
	struct wifi_pos_psoc_priv_obj *wifi_pos_psoc;

	if (!psoc) {
		wifi_pos_err("psoc is null");
		return QDF_STATUS_E_NULL_VALUE;
	}

	if (!handler) {
		wifi_pos_err("Null callback");
		return QDF_STATUS_E_NULL_VALUE;
	}
	wifi_pos_psoc = wifi_pos_get_psoc_priv_obj(psoc);
	if (!wifi_pos_psoc) {
		wifi_pos_err("wifi_pos priv obj is null");
		return QDF_STATUS_E_NULL_VALUE;
	}

	wifi_pos_psoc->wifi_pos_send_action = handler;

	return QDF_STATUS_SUCCESS;
}
