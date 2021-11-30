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
#include <wlan_twt_ext_type.h>
#include <wlan_twt_api.h>
#include <cfg_ucfg_api.h>
#include <cfg_twt.h>
#include "wlan_twt_cfg.h"
#include "twt/core/src/wlan_twt_priv.h"

QDF_STATUS wlan_twt_cfg_init(struct wlan_objmgr_psoc *psoc)
{
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS wlan_twt_cfg_update(struct wlan_objmgr_psoc *psoc)
{
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
wlan_twt_cfg_get_requestor(struct wlan_objmgr_psoc *psoc, bool *val)
{
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
wlan_twt_cfg_get_responder(struct wlan_objmgr_psoc *psoc, bool *val)
{
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
wlan_twt_cfg_get_congestion_timeout(struct wlan_objmgr_psoc *psoc,
				    uint32_t *val)
{
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
wlan_twt_cfg_set_congestion_timeout(struct wlan_objmgr_psoc *psoc, uint32_t val)
{
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
wlan_twt_cfg_get_requestor_flag(struct wlan_objmgr_psoc *psoc, bool *val)
{
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
wlan_twt_cfg_set_requestor_flag(struct wlan_objmgr_psoc *psoc, bool val)
{
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
wlan_twt_cfg_get_responder_flag(struct wlan_objmgr_psoc *psoc, bool *val)
{
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
wlan_twt_cfg_set_responder_flag(struct wlan_objmgr_psoc *psoc, bool val)
{
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
wlan_twt_cfg_get_flex_sched(struct wlan_objmgr_psoc *psoc, bool *val)
{
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
wlan_twt_cfg_get_24ghz_enabled(struct wlan_objmgr_psoc *psoc, bool *val)
{
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
wlan_twt_cfg_get_bcast_requestor(struct wlan_objmgr_psoc *psoc, bool *val)
{
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
wlan_twt_cfg_get_bcast_responder(struct wlan_objmgr_psoc *psoc, bool *val)
{
	return QDF_STATUS_SUCCESS;
}

