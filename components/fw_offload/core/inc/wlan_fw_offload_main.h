/*
 * Copyright (c) 2018 The Linux Foundation. All rights reserved.
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
 * DOC: declare utility API related to the fw_offload component
 * called by other components
 */

#ifndef _WLAN_FW_OFFLOAD_MAIN_H_
#define _WLAN_FW_OFFLOAD_MAIN_H_

#include <wlan_objmgr_psoc_obj.h>
#include <wlan_objmgr_global_obj.h>
#include <wlan_cmn.h>

#define fwol_alert(params...) QDF_TRACE_FATAL(QDF_MODULE_ID_FWOL, params)
#define fwol_err(params...) QDF_TRACE_ERROR(QDF_MODULE_ID_FWOL, params)
#define fwol_warn(params...) QDF_TRACE_WARN(QDF_MODULE_ID_FWOL, params)
#define fwol_info(params...) QDF_TRACE_INFO(QDF_MODULE_ID_FWOL, params)
#define fwol_debug(params...) QDF_TRACE_DEBUG(QDF_MODULE_ID_FWOL, params)

/**
 * struct wlan_fwol_cfg - fwol config items
 */
struct wlan_fwol_cfg {
	/* Add CFG and INI items here */
	int test;
};

/**
 * struct wlan_fwol_psoc_obj - FW offload psoc priv object
 * @cfg:     cfg items
 */
struct wlan_fwol_psoc_obj {
	struct wlan_fwol_cfg cfg;
};

/**
 * wlan_psoc_get_fwol_obj() - private API to get fwol object from psoc
 * @psoc: psoc object
 *
 * Return: fwol object
 */
struct wlan_fwol_psoc_obj *fwol_get_psoc_obj(struct wlan_objmgr_psoc *psoc);

/*
 * fwol_cfg_on_psoc_enable() - Populate FWOL structure from CFG and INI
 * @psoc: pointer to the psoc object
 *
 * Populate the FWOL CFG structure from CFG and INI values using CFG APIs
 *
 * Return: QDF_STATUS
 */
QDF_STATUS fwol_cfg_on_psoc_enable(struct wlan_objmgr_psoc *psoc);

/*
 * fwol_cfg_on_psoc_disable() - Clear the CFG structure on psoc disable
 * @psoc: pointer to the psoc object
 *
 * Clear the FWOL CFG structure on psoc disable
 *
 * Return: QDF_STATUS
 */
QDF_STATUS fwol_cfg_on_psoc_disable(struct wlan_objmgr_psoc *psoc);
#endif
