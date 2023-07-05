/*
 * Copyright (c) 2018 The Linux Foundation. All rights reserved.
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
 * DOC: declare the ftm service data structure and apis
 */
#ifndef _WLAN_FTM_UCFG_API_H_
#define _WLAN_FTM_UCFG_API_H_

#include <qdf_types.h>
#include <qdf_status.h>
#include <wlan_objmgr_cmn.h>

#ifdef QCA_WIFI_FTM
/**
 * dispatcher_ftm_init() - FTM testmode initialization API
 *
 * Return: QDF_STATUS_SUCCESS on success, QDF_STATUS_E_** on error
 */
QDF_STATUS dispatcher_ftm_init(void);

/**
 * dispatcher_ftm_deinit() - FTM testmode deinitialization API
 *
 * Return: QDF_STATUS_SUCCESS on success, QDF_STATUS_E_** on error
 */
QDF_STATUS dispatcher_ftm_deinit(void);

/**
 * dispatcher_ftm_psoc_open() - FTM module open API
 * @psoc: psoc object
 *
 * Return: QDF_STATUS_SUCCESS on success, QDF_STATUS_E_** on error
 */
QDF_STATUS dispatcher_ftm_psoc_open(struct wlan_objmgr_psoc *psoc);

/**
 * dispatcher_ftm_psoc_close() - FTM module close API
 * @psoc: psoc object
 *
 * Return: QDF_STATUS_SUCCESS on success, QDF_STATUS_E_** on error
 */
QDF_STATUS dispatcher_ftm_psoc_close(struct wlan_objmgr_psoc *psoc);

#else
static inline QDF_STATUS dispatcher_ftm_init(void)
{
	return QDF_STATUS_SUCCESS;
}

static inline QDF_STATUS dispatcher_ftm_deinit(void)
{
	return QDF_STATUS_SUCCESS;
}

static inline QDF_STATUS
dispatcher_ftm_psoc_open(struct wlan_objmgr_psoc *psoc)
{
	return QDF_STATUS_SUCCESS;
}

static inline QDF_STATUS
dispatcher_ftm_psoc_close(struct wlan_objmgr_psoc *psoc)
{
	return QDF_STATUS_SUCCESS;
}
#endif
#endif /* _WLAN_FTM_UCFG_API_H_ */
