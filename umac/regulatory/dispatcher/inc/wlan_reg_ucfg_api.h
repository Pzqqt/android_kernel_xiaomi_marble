/*
 * Copyright (c) 2017 The Linux Foundation. All rights reserved.
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

/**
 * DOC: wlan_reg_ucfg_api.h
 * This file provides prototypes of the regulatory component user
 * config interface routines
 */

#ifndef __WLAN_REG_UCFG_API_H
#define __WLAN_REG_UCFG_API_H

#include <qdf_types.h>
#include <qdf_status.h>
#include "../../core/src/reg_services.h"
#include <reg_services_public_struct.h>

typedef QDF_STATUS (*reg_event_cb)(void *status_struct);

QDF_STATUS ucfg_reg_set_band(uint8_t vdev_id, uint8_t pdev_id,
		struct set_band_req *req);
QDF_STATUS ucfg_reg_reset_country(uint8_t vdev_id, uint8_t pdev_id);
QDF_STATUS ucfg_reg_set_default_country(uint8_t vdev_id, uint8_t pdev_id,
		struct country_info *cc_info);
QDF_STATUS ucfg_reg_update_country(uint8_t vdev_id, uint8_t pdev_id,
		struct reg_country_update *country_update);
QDF_STATUS ucfg_reg_register_event_handler(uint8_t vdev_id, reg_event_cb cb,
		void *arg);
QDF_STATUS ucfg_reg_unregister_event_handler(uint8_t vdev_id, reg_event_cb cb,
		void *arg);
QDF_STATUS ucfg_reg_init_handler(uint8_t pdev_id);

/**
 * ucfg_reg_set_config_vars () - Set the config vars in reg component
 * @psoc: psoc ptr
 * @config_vars: config variables structure
 *
 * Return: QDF_STATUS
 */
QDF_STATUS ucfg_reg_set_config_vars(struct wlan_objmgr_psoc *psoc,
				    struct reg_config_vars config_vars);

/**
 * ucfg_reg_get_current_chan_list () - get current channel list
 * @pdev: pdev ptr
 * @chan_list: channel list
 *
 * Return: QDF_STATUS
 */
QDF_STATUS ucfg_reg_get_current_chan_list(struct wlan_objmgr_pdev *pdev,
				    struct regulatory_channel *chan_list);

/**
 * ucfg_reg_is_regdb_offloaded () - is regulatory database offloaded
 * @psoc: psoc ptr
 *
 * Return: bool
 */
bool ucfg_reg_is_regdb_offloaded(struct wlan_objmgr_psoc *psoc);

/**
 * ucfg_reg_program_mas_chan_list () - program master channel list
 * @psoc: psoc ptr
 * @reg_channels: regulatory channels
 * @alpha2: country code
 * @dfs_region: dfs region
 *
 * Return: void
 */
void ucfg_reg_program_mas_chan_list(struct wlan_objmgr_psoc *psoc,
				    struct regulatory_channel *reg_channels,
				    uint8_t *alpha2,
				    enum dfs_reg dfs_region);

#endif
