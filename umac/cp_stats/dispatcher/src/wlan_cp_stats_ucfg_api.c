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
 * DOC: wlan_cp_stats_ucfg_api.c
 *
 * This file provide API definitions required for northbound interaction
 */

#include <wlan_cp_stats_utils_api.h>
#include <wlan_cp_stats_ucfg_api.h>
#include "../../core/src/wlan_cp_stats_obj_mgr_handler.h"

#ifdef WLAN_SUPPORT_INFRA_CTRL_PATH_STATS
QDF_STATUS
ucfg_infra_cp_stats_register_resp_cb(struct wlan_objmgr_psoc *psoc,
				     struct infra_cp_stats_cmd_info *req)
{
	return wlan_cp_stats_infra_cp_register_resp_cb(psoc, req);
}

QDF_STATUS
ucfg_send_infra_cp_stats_request(struct wlan_objmgr_vdev *vdev,
				 struct infra_cp_stats_cmd_info *req)
{
	return wlan_cp_stats_send_infra_cp_req(wlan_vdev_get_psoc(vdev), req);
}
#endif /* WLAN_SUPPORT_INFRA_CTRL_PATH_STATS */
