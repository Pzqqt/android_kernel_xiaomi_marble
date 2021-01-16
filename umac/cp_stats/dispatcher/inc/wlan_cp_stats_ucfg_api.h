/*
 * Copyright (c) 2018, 2021 The Linux Foundation. All rights reserved.
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
 * DOC: wlan_cp_stats_ucfg_api.h
 *
 * This header file maintain API declaration required for northbound interaction
 */

#ifndef __WLAN_CP_STATS_UCFG_API_H__
#define __WLAN_CP_STATS_UCFG_API_H__

#ifdef QCA_SUPPORT_CP_STATS
#include <wlan_cp_stats_utils_api.h>
#include "../../core/src/wlan_cp_stats_defs.h"
#include "../../core/src/wlan_cp_stats_cmn_api_i.h"

/**
 * @ucfg_infra_cp_stats_register_resp_cb() - Register the response callback
 * and cookie in the psoc mc_stats object
 * @psoc: pointer to psoc object
 * @req: pointer to request parameter structure
 *
 * Return: QDF_STATUS_SUCCESS on Success, other QDF_STATUS error codes on
 * failure
 */
QDF_STATUS
ucfg_infra_cp_stats_register_resp_cb(struct wlan_objmgr_psoc *psoc,
				     struct infra_cp_stats_cmd_info *req);

/**
 * @ucfg_send_infra_cp_stats_request() - send a infra cp stats command
 * @vdev: pointer to vdev object
 * @req: pointer to request parameter structure
 *
 * Return: QDF_STATUS_SUCCESS on Success, other QDF_STATUS error codes
 * on failure
 */
QDF_STATUS
ucfg_send_infra_cp_stats_request(struct wlan_objmgr_vdev *vdev,
				 struct infra_cp_stats_cmd_info *req);
#endif /* QCA_SUPPORT_CP_STATS */
#endif /* __WLAN_CP_STATS_UCFG_API_H__ */
