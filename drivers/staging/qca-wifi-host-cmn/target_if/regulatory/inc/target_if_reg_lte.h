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
 * DOC: target_if_reg_lte.h
 * This file contains regulatory target LTE interface
 */

#ifndef __TARGET_IF_REG_LTE_H__
#define __TARGET_IF_REG_LTE_H__

#include "qdf_types.h"
#include "target_if.h"
#include <wlan_objmgr_psoc_obj.h>
#include "target_if_reg.h"
#include "wmi_unified_api.h"
#include "wmi_unified_reg_api.h"

#ifdef LTE_COEX
/**
 * tgt_if_regulatory_register_ch_avoid_event_handler() - Register avoid channel
 * list event handler
 * @psoc: Pointer to psoc
 * @arg: Pointer to argumemt list
 *
 * Return: QDF_STATUS
 */
QDF_STATUS tgt_if_regulatory_register_ch_avoid_event_handler(
		struct wlan_objmgr_psoc *psoc, void *arg);

/**
 * tgt_if_regulatory_unregister_ch_avoid_event_handler() - Unregister avoid
 * channel list event handler
 * @psoc: Pointer to psoc
 * @arg: Pointer to argumemt list
 *
 * Return: QDF_STATUS
 */
QDF_STATUS tgt_if_regulatory_unregister_ch_avoid_event_handler(
		struct wlan_objmgr_psoc *psoc, void *arg);

#else

static inline QDF_STATUS tgt_if_regulatory_register_ch_avoid_event_handler(
		struct wlan_objmgr_psoc *psoc, void *arg)
{
	return QDF_STATUS_SUCCESS;
}

static inline QDF_STATUS tgt_if_regulatory_unregister_ch_avoid_event_handler(
		struct wlan_objmgr_psoc *psoc, void *arg)
{
	return QDF_STATUS_SUCCESS;
}
#endif
#endif /* __TARGET_IF_REG_H__ */
