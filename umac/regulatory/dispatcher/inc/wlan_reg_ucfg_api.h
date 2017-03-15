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

#include <qdf_types.h>
#include <qdf_trace.h>

/**
 * DOC: wlan_reg_ucfg_api.h
 * This file provides prototypes of the regulatory component user
 * config interface routines
 */

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
