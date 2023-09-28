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
 * DOC: target_if_twt_cmd.h
 */

#ifndef _TARGET_IF_TWT_CMD_H_
#define _TARGET_IF_TWT_CMD_H_

#include <wmi_unified_param.h>
#include <wlan_lmac_if_def.h>
#include <target_if.h>
#include <wlan_twt_public_structs.h>

#if defined(WLAN_SUPPORT_TWT) && defined(WLAN_TWT_CONV_SUPPORTED)
/**
 * target_if_twt_enable_req() - target if twt enable request
 * @psoc: psoc handle
 * @req: TWT enable request structure
 *
 * return: QDF_STATUS
 */
QDF_STATUS
target_if_twt_enable_req(struct wlan_objmgr_psoc *psoc,
			 struct twt_enable_param *req);

/**
 * target_if_twt_disable_req() - target if twt disable request
 * @psoc: psoc handle
 * @req: TWT disable request structure
 *
 * return: QDF_STATUS
 */
QDF_STATUS
target_if_twt_disable_req(struct wlan_objmgr_psoc *psoc,
			  struct twt_disable_param *req);
#else
static inline QDF_STATUS
target_if_twt_enable_req(struct wlan_objmgr_psoc *psoc,
			 struct twt_enable_param *params)
{
	return QDF_STATUS_SUCCESS;
}

static inline QDF_STATUS
target_if_twt_disable_req(struct wlan_objmgr_psoc *psoc,
			  struct twt_disable_param *params)
{
	return QDF_STATUS_SUCCESS;
}

#endif

#endif /* _TARGET_IF_TWT_CMD_H_ */
