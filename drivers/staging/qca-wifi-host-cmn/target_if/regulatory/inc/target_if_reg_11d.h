/*
 * Copyright (c) 2017-2019 The Linux Foundation. All rights reserved.
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
 * DOC: target_if_reg_11d.h
 * This file contains regulatory target 11d scan interface.
 */

#ifndef __TARGET_IF_REG_11D_H__
#define __TARGET_IF_REG_11D_H__

#include "qdf_types.h"
#include "target_if.h"
#include <wlan_objmgr_psoc_obj.h>
#include "target_if_reg.h"
#include "wmi_unified_api.h"
#include "wmi_unified_reg_api.h"

#ifdef HOST_11D_SCAN
/**
 * tgt_if_regulatory_is_11d_offloaded() - Check if reg 11d is offloaded.
 * @psoc: psoc pointer
 *
 * Return: true if 11d is offloaded, else false.
 */
bool tgt_if_regulatory_is_11d_offloaded(struct wlan_objmgr_psoc *psoc);

/**
 * tgt_if_regulatory_register_11d_new_cc_handler() - Register for 11d country
 * code event ID.
 * @psoc: psoc pointer
 * @args: Pointer to args.
 *
 * Return: QDF_STATUS
 */
QDF_STATUS tgt_if_regulatory_register_11d_new_cc_handler(
		struct wlan_objmgr_psoc *psoc, void *arg);

/**
 * tgt_if_regulatory_unregister_11d_new_cc_handler() - Unregister 11d country
 * code event ID.
 * @psoc: psoc pointer
 * @args: Pointer to args.
 *
 * Return: QDF_STATUS
 */
QDF_STATUS tgt_if_regulatory_unregister_11d_new_cc_handler(
		struct wlan_objmgr_psoc *psoc, void *arg);

/**
 * tgt_if_regulatory_start_11d_scan() - Start 11d scan.
 * @psoc: psoc pointer
 * @reg_start_11d_scan_req: Pointer to 11d scan start request.
 *
 * Return: QDF_STATUS
 */
QDF_STATUS tgt_if_regulatory_start_11d_scan(
		struct wlan_objmgr_psoc *psoc,
		struct reg_start_11d_scan_req *reg_start_11d_scan_req);

/**
 * tgt_if_regulatory_stop_11d_scan() - Stop 11d scan.
 * @psoc: psoc pointer
 * @reg_stop_11d_scan_req: Pointer to 11d scan stop request.
 *
 * Return: QDF_STATUS
 */
QDF_STATUS tgt_if_regulatory_stop_11d_scan(
		struct wlan_objmgr_psoc *psoc,
		struct reg_stop_11d_scan_req *reg_stop_11d_scan_req);

#else

static inline bool
tgt_if_regulatory_is_11d_offloaded(struct wlan_objmgr_psoc *psoc)
{
	return false;
}

static inline QDF_STATUS
tgt_if_regulatory_register_11d_new_cc_handler(struct wlan_objmgr_psoc *psoc,
					      void *arg)
{
	return QDF_STATUS_SUCCESS;
}

static inline QDF_STATUS
tgt_if_regulatory_unregister_11d_new_cc_handler(struct wlan_objmgr_psoc *psoc,
						void *arg)
{
	return QDF_STATUS_SUCCESS;
}

static inline QDF_STATUS tgt_if_regulatory_start_11d_scan(
	struct wlan_objmgr_psoc *psoc,
	struct reg_start_11d_scan_req *reg_start_11d_scan_req)
{
	return QDF_STATUS_SUCCESS;
}

static inline QDF_STATUS tgt_if_regulatory_stop_11d_scan(
		struct wlan_objmgr_psoc *psoc,
		struct reg_stop_11d_scan_req *reg_stop_11d_scan_req)
{
	return QDF_STATUS_SUCCESS;
}
#endif
#endif
