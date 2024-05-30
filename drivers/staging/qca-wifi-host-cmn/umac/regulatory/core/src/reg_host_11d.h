/*
 * Copyright (c) 2018-2019 The Linux Foundation. All rights reserved.
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
 * DOC: Defines host 11d scan utility functions
 */
#ifndef _REG_HOST_11D_H_
#define _REG_HOST_11D_H_

#ifdef HOST_11D_SCAN
/**
 * reg_11d_host_scan() - Start/stop 11d scan
 * @soc_reg: soc regulatory context
 *
 * This function gets called upon 11d scan enable/disable changed.
 *
 * Return: QDF_STATUS_SUCCESS - in case of success
 */
QDF_STATUS reg_11d_host_scan(struct wlan_regulatory_psoc_priv_obj *soc_reg);

/**
 * reg_11d_host_scan_init() - Init 11d host scan resource
 * @psoc: soc context
 *
 * This function gets called during pdev create notification callback to
 * init the 11d scan related resource.
 *
 * Return: QDF_STATUS_SUCCESS - in case of success
 */
QDF_STATUS reg_11d_host_scan_init(struct wlan_objmgr_psoc *psoc);

/**
 * reg_11d_host_scan_deinit() - Deinit 11d host scan resource
 * @psoc: soc context
 *
 * This function gets called during pdev destroy notification callback to
 * deinit the 11d scan related resource.
 *
 * Return: QDF_STATUS_SUCCESS - in case of success
 */
QDF_STATUS reg_11d_host_scan_deinit(struct wlan_objmgr_psoc *psoc);
#else

static inline QDF_STATUS reg_11d_host_scan(
	struct wlan_regulatory_psoc_priv_obj *soc_reg)
{
	return QDF_STATUS_SUCCESS;
}

static inline QDF_STATUS reg_11d_host_scan_init(struct wlan_objmgr_psoc *psoc)
{
	return QDF_STATUS_SUCCESS;
}

static inline QDF_STATUS reg_11d_host_scan_deinit(struct wlan_objmgr_psoc *psoc)
{
	return QDF_STATUS_SUCCESS;
}
#endif
#endif
