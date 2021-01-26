/*
 * Copyright (c) 2012-2021 The Linux Foundation. All rights reserved.
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

/*
 * DOC: wlan_cm_roam_fw_sync.c
 *
 * Implementation for the FW based roaming sync api interfaces.
 */
#include "qdf_types.h"
#include "wlan_objmgr_psoc_obj.h"
#include "wlan_objmgr_pdev_obj.h"
#include "wlan_objmgr_vdev_obj.h"
#include "wlan_cm_roam_i.h"

QDF_STATUS cm_fw_roam_sync_req(struct wlan_objmgr_psoc *psoc, uint8_t vdev_id)
{
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
cm_fw_roam_sync_propagation(struct wlan_objmgr_psoc *psoc, uint8_t vdev_id)
{
	return QDF_STATUS_SUCCESS;
}

