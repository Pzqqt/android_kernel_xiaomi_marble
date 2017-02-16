/*
 * Copyright (c) 2017 The Linux Foundation. All rights reserved.
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
 * DOC: This file contains p2p north bound interface definitions
 */

#include <wlan_objmgr_psoc_obj.h>
#include "wlan_p2p_ucfg_api.h"
#include "wlan_p2p_public_struct.h"
#include "../../core/inc/wlan_p2p_main.h"
#include "../../core/inc/wlan_p2p_roc.h"
#include "../../core/inc/wlan_p2p_off_chan_tx.h"

QDF_STATUS ucfg_p2p_init(void)
{
	return p2p_component_init();
}

QDF_STATUS ucfg_p2p_deinit(void)
{
	return p2p_component_deinit();
}

QDF_STATUS ucfg_p2p_psoc_open(struct wlan_objmgr_psoc *soc)
{
	return p2p_psoc_open(soc);
}

QDF_STATUS ucfg_p2p_psoc_close(struct wlan_objmgr_psoc *soc)
{
	return p2p_psoc_close(soc);
}

QDF_STATUS ucfg_p2p_psoc_start(struct wlan_objmgr_psoc *soc,
	struct p2p_start_param *req)
{
	return p2p_psoc_start(soc, req);
}

QDF_STATUS ucfg_p2p_psoc_stop(struct wlan_objmgr_psoc *soc)
{
	return p2p_psoc_stop(soc);
}

QDF_STATUS ucfg_p2p_roc_req(struct wlan_objmgr_psoc *soc,
	struct p2p_roc_req *roc_req, uint64_t *cookie)
{
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS ucfg_p2p_roc_cancel_req(struct wlan_objmgr_psoc *soc,
	uint64_t cookie)
{
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS ucfg_p2p_mgmt_tx(struct wlan_objmgr_psoc *soc,
	struct p2p_mgmt_tx *mgmt_frm, uint64_t *cookie)
{
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS ucfg_p2p_mgmt_tx_cancel(struct wlan_objmgr_psoc *soc,
	uint64_t cookie)
{
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS ucfg_p2p_set_ps(struct wlan_objmgr_psoc *soc,
	struct p2p_ps_config *ps_config)
{
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS ucfg_p2p_lo_start(struct wlan_objmgr_psoc *soc,
	struct p2p_lo_start *p2p_lo_start)
{
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS ucfg_p2p_lo_stop(struct wlan_objmgr_psoc *soc,
	uint32_t vdev_id)
{
	return QDF_STATUS_SUCCESS;
}
