/*
 * Copyright (c) 2022 Qualcomm Innovation Center, Inc. All rights reserved.
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
 * DOC: wlan_twt_ucfg_ext_cfg.h
 * This file provides TWT ucfg cfg param related apis.
 */
#ifndef _WLAN_TWT_UCFG_EXT_CFG_H_
#define _WLAN_TWT_UCFG_EXT_CFG_H_

#include <wlan_objmgr_psoc_obj.h>
#include <wlan_twt_public_structs.h>

/**
 * ucfg_twt_cfg_get_requestor() - get TWT requestor
 * @psoc: Pointer to global PSOC object
 * @val: pointer to output variable
 *
 * Return: QDF_STATUS_SUCCESS
 */
QDF_STATUS ucfg_twt_cfg_get_requestor(struct wlan_objmgr_psoc *psoc, bool *val);

/**
 * ucfg_twt_cfg_get_responder() - get TWT responder
 * @psoc: Pointer to global PSOC object
 * @val: pointer to output variable
 *
 * Return: QDF_STATUS_SUCCESS
 */
QDF_STATUS ucfg_twt_cfg_get_responder(struct wlan_objmgr_psoc *psoc, bool *val);

/**
 * ucfg_twt_cfg_get_congestion_timeout() - Get TWT congestion timeout
 * @psoc: Pointer to global psoc object
 * @val: pointer to output variable
 *
 * Return: QDF_STATUS_SUCCESS
 */
QDF_STATUS
ucfg_twt_cfg_get_congestion_timeout(struct wlan_objmgr_psoc *psoc,
				    uint32_t *val);

/**
 * ucfg_twt_cfg_set_congestion_timeout() - Set TWT congestion timeout
 * @psoc: Pointer to global psoc object
 * @val: congestion timeout
 *
 * Return: QDF_STATUS_SUCCESS
 */
QDF_STATUS
ucfg_twt_cfg_set_congestion_timeout(struct wlan_objmgr_psoc *psoc,
				    uint32_t val);

/**
 * ucfg_twt_cfg_get_24ghz_enabled() - Get TWT 24ghz enabled
 * @psoc: Pointer to global psoc object
 * @val: pointer to output variable
 *
 * Return: QDF_STATUS_SUCCESS
 */
QDF_STATUS
ucfg_twt_cfg_get_24ghz_enabled(struct wlan_objmgr_psoc *psoc, bool *val);

/**
 * ucfg_twt_cfg_get_bcast_requestor() - Get TWT broadcast requestor
 * @psoc: Pointer to global psoc object
 * @val: pointer to output variable
 *
 * Return: QDF_STATUS_SUCCESS
 */
QDF_STATUS
ucfg_twt_cfg_get_bcast_requestor(struct wlan_objmgr_psoc *psoc, bool *val);

/**
 * ucfg_twt_cfg_get_flex_sched() - Get TWT flex scheduling
 * @psoc: Pointer to global psoc object
 * @val: pointer to output variable
 *
 * Return: QDF_STATUS_SUCCESS
 */
QDF_STATUS
ucfg_twt_cfg_get_flex_sched(struct wlan_objmgr_psoc *psoc, bool *val);
#else
static inline
QDF_STATUS ucfg_twt_cfg_get_requestor(struct wlan_objmgr_psoc *psoc, bool *val)
{
	*val = false;
	return QDF_STATUS_SUCCESS;
}

static inline
QDF_STATUS ucfg_twt_cfg_get_responder(struct wlan_objmgr_psoc *psoc, bool *val)
{
	*val = false;
	return QDF_STATUS_SUCCESS;
}

static inline
QDF_STATUS ucfg_twt_cfg_get_congestion_timeout(struct wlan_objmgr_psoc *psoc,
					       uint32_t *val)
{
	*val = 0;
	return QDF_STATUS_SUCCESS;
}

static inline
QDF_STATUS ucfg_twt_cfg_set_congestion_timeout(struct wlan_objmgr_psoc *psoc,
					       uint32_t val)
{
	return QDF_STATUS_SUCCESS;
}

static inline QDF_STATUS
ucfg_twt_cfg_get_24ghz_enabled(struct wlan_objmgr_psoc *psoc, bool *val)
{
	*val = false;
	return QDF_STATUS_SUCCESS;
}

static inline QDF_STATUS
ucfg_twt_cfg_get_bcast_requestor(struct wlan_objmgr_psoc *psoc, bool *val)
{
	*val = false;
	return QDF_STATUS_SUCCESS;
}

static inline QDF_STATUS
ucfg_twt_cfg_get_flex_sched(struct wlan_objmgr_psoc *psoc, bool *val)
{
	*val = false;
	return QDF_STATUS_SUCCESS;
}

#endif /* _WLAN_TWT_UCFG_EXT_CFG_H_ */
