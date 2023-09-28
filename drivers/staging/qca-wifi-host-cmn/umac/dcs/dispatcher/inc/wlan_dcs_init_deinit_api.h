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

/**
 * DOC: wlan_dcs_init_deinit_api.h
 *
 * This header file provide declaration to public APIs exposed for other UMAC
 * components to init/deinit, (de)register to required WMI events on
 * soc enable/disable
 */

#ifndef __WLAN_DCS_INIT_DEINIT_API_H__
#define __WLAN_DCS_INIT_DEINIT_API_H__

#ifdef DCS_INTERFERENCE_DETECTION

#include <qdf_types.h>
#include <qdf_status.h>
#include <wlan_objmgr_cmn.h>

/**
 * wlan_dcs_init(): API to init dcs component
 *
 * This API is invoked from dispatcher init during all component init.
 * This API will register all required handlers for pdev object
 * create/delete notification.
 *
 * Return: QDF_STATUS_SUCCESS on success, QDF_STATUS_E_** on error
 */
QDF_STATUS wlan_dcs_init(void);

/**
 * wlan_dcs_deinit(): API to deinit dcs component
 *
 * This API is invoked from dispatcher deinit during all component deinit.
 * This API will unregister all required handlers for pdev object
 * create/delete notification.
 *
 * Return: QDF_STATUS_SUCCESS on success, QDF_STATUS_E_** on error
 */
QDF_STATUS wlan_dcs_deinit(void);

/**
 * wlan_dcs_enable(): API to enable dcs component
 * @psoc: pointer to psoc
 *
 * This API is invoked from dispatcher psoc enable.
 * This API will register dcs WMI event handlers.
 *
 * Return: QDF_STATUS_SUCCESS on success, QDF_STATUS_E_** on error
 */
QDF_STATUS wlan_dcs_enable(struct wlan_objmgr_psoc *psoc);

/**
 * wlan_dcs_disable(): API to disable dcs component
 * @psoc: pointer to psoc
 *
 * This API is invoked from dispatcher psoc disable.
 * This API will unregister dcs WMI event handlers.
 *
 * Return: QDF_STATUS_SUCCESS on success, QDF_STATUS_E_** on error
 */
QDF_STATUS wlan_dcs_disable(struct wlan_objmgr_psoc *psoc);

/**
 * wlan_dcs_psoc_open() - Dcs psoc open handler
 * @psoc:  pointer to psoc object
 *
 * API to execute operations on psoc open
 *
 * Return: QDF_STATUS_SUCCESS upon successful registration,
 *         QDF_STATUS_E_FAILURE upon failure
 */
QDF_STATUS wlan_dcs_psoc_open(struct wlan_objmgr_psoc *psoc);

#endif /* DCS_INTERFERENCE_DETECTION */
#endif /* __WLAN_DCS_INIT_DEINIT_API_H__ */

