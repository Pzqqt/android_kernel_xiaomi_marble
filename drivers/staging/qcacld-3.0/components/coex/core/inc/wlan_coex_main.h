/*
 * Copyright (c) 2020, The Linux Foundation. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

/*
 * DOC: contains declarations for coex core functions
 */

#ifndef _WLAN_COEX_MAIN_API_H_
#define _WLAN_COEX_MAIN_API_H_

#ifdef FEATURE_COEX
#include "wlan_coex_ucfg_api.h"
#include "wmi_unified_param.h"
#include "wlan_objmgr_psoc_obj.h"
#include "wlan_objmgr_vdev_obj.h"

#define coex_err(params...) \
	QDF_TRACE_ERROR(QDF_MODULE_ID_COEX, params)
#define coex_info(params...) \
	QDF_TRACE_INFO(QDF_MODULE_ID_COEX, params)
#define coex_debug(params...) \
	QDF_TRACE_DEBUG(QDF_MODULE_ID_COEX, params)

/**
 * struct coex_psoc_obj - coex object definition
 * @btc_chain_mode: BT Coex chain mode.
 * @coex_config_updated: callback functions for each config type, which will
 *  be called when config is updated.
 */
struct coex_psoc_obj {
	uint8_t btc_chain_mode;
	update_coex_cb coex_config_updated[COEX_CONFIG_TYPE_MAX];
};

/**
 * wlan_psoc_get_coex_obj() - private API to get coex object from psoc
 * @psoc: psoc object
 *
 * Return: coex object
 */
#define wlan_psoc_get_coex_obj(psoc) \
	wlan_psoc_get_coex_obj_fl(psoc, __func__, __LINE__)

static inline struct coex_psoc_obj *
wlan_psoc_get_coex_obj_fl(struct wlan_objmgr_psoc *psoc,
			  const char *func, uint32_t line)
{
	struct coex_psoc_obj *psoc_obj;

	psoc_obj = (struct coex_psoc_obj *)
		wlan_objmgr_psoc_get_comp_private_obj(psoc,
						      WLAN_UMAC_COMP_COEX);
	if (!psoc_obj) {
		coex_err("%s:%u, Failed to get coex psoc object", func, line);
		return NULL;
	}
	return psoc_obj;
}

/**
 * wlan_coex_psoc_init() - API to initialize coex component
 * @psoc: soc context
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
wlan_coex_psoc_init(struct wlan_objmgr_psoc *psoc);

/**
 * wlan_coex_psoc_deinit() - API to deinitialize coex component
 * @psoc: soc context
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
wlan_coex_psoc_deinit(struct wlan_objmgr_psoc *psoc);

/**
 * wlan_coex_config_send() - private API to send coex config
 * @vdev: pointer to vdev object
 * @param: parameters of coex config
 *
 * Return: status of operation
 */
QDF_STATUS wlan_coex_config_send(struct wlan_objmgr_vdev *vdev,
				 struct coex_config_params *param);

/**
 * wlan_coex_config_updated() - private API to notify that coex config
 * is updated.
 * @vdev: pointer to vdev object
 * @type: type of coex config
 *
 * Return: status of operation
 */
QDF_STATUS
wlan_coex_config_updated(struct wlan_objmgr_vdev *vdev, uint8_t type);

/**
 * wlan_coex_psoc_created_notification() - PSOC obj create callback
 * @psoc: PSOC object
 * @arg_list: Variable argument list
 *
 * This callback is registered with object manager during initialization to
 * get notified when the object is created.
 *
 * Return: Success or Failure
 */
QDF_STATUS wlan_coex_psoc_created_notification(struct wlan_objmgr_psoc *psoc,
					       void *arg_list);

/**
 * wlan_coex_psoc_destroyed_notification() - PSOC obj delete callback
 * @psoc: PSOC object
 * @arg_list: Variable argument list
 *
 * This callback is registered with object manager during initialization to
 * get notified when the object is deleted.
 *
 * Return: Success or Failure
 */
QDF_STATUS wlan_coex_psoc_destroyed_notification(struct wlan_objmgr_psoc *psoc,
						 void *arg_list);

/**
 * wlan_coex_psoc_set_btc_chain_mode() - private API to set BT coex chain mode
 * for psoc
 * @psoc: pointer to psoc object
 * @val: BT coex chain mode
 *
 * Return : status of operation
 */
QDF_STATUS
wlan_coex_psoc_set_btc_chain_mode(struct wlan_objmgr_psoc *psoc, uint8_t val);

/**
 * wlan_coex_psoc_get_btc_chain_mode() - private API to get BT coex chain mode
 * from psoc
 * @psoc: pointer to psoc object
 * @val: pointer to BT coex chain mode
 *
 * Return : status of operation
 */
QDF_STATUS
wlan_coex_psoc_get_btc_chain_mode(struct wlan_objmgr_psoc *psoc, uint8_t *val);
#endif
#endif
