/*
 * Copyright (c) 2012-2015, 2020-2021, The Linux Foundation. All rights reserved.
 * Copyright (c) 2022 Qualcomm Innovation Center, Inc. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

/**
 * DOC: wlan_cm_ucfg_api.c
 *
 * This file maintains definitaions public ucfg apis.
 */

#include <wlan_cm_ucfg_api.h>
#include "connection_mgr/core/src/wlan_cm_main_api.h"
#include <wlan_reg_services_api.h>

QDF_STATUS ucfg_cm_start_connect(struct wlan_objmgr_vdev *vdev,
				 struct wlan_cm_connect_req *req)
{
	return cm_connect_start_req(vdev, req);
}

QDF_STATUS ucfg_cm_start_disconnect(struct wlan_objmgr_vdev *vdev,
				    struct wlan_cm_disconnect_req *req)
{
	return cm_disconnect_start_req(vdev, req);
}

QDF_STATUS ucfg_cm_disconnect_sync(struct wlan_objmgr_vdev *vdev,
				   enum wlan_cm_source source,
				   enum wlan_reason_code reason_code)
{
	return wlan_cm_disconnect_sync(vdev, source, reason_code);
}

bool ucfg_cm_is_vdev_connecting(struct wlan_objmgr_vdev *vdev)
{
	return cm_is_vdev_connecting(vdev);
}

bool ucfg_cm_is_vdev_connected(struct wlan_objmgr_vdev *vdev)
{
	return cm_is_vdev_connected(vdev);
}

bool ucfg_cm_is_vdev_active(struct wlan_objmgr_vdev *vdev)
{
	return cm_is_vdev_active(vdev);
}

bool ucfg_cm_is_vdev_disconnecting(struct wlan_objmgr_vdev *vdev)
{
	return cm_is_vdev_disconnecting(vdev);
}

bool ucfg_cm_is_vdev_disconnected(struct wlan_objmgr_vdev *vdev)
{
	return cm_is_vdev_disconnected(vdev);
}

bool ucfg_cm_is_vdev_roaming(struct wlan_objmgr_vdev *vdev)
{
	return cm_is_vdev_roaming(vdev);
}

#ifdef WLAN_FEATURE_ROAM_OFFLOAD
bool ucfg_cm_is_vdev_roam_started(struct wlan_objmgr_vdev *vdev)
{
	return cm_is_vdev_roam_started(vdev);
}

bool ucfg_cm_is_vdev_roam_sync_inprogress(struct wlan_objmgr_vdev *vdev)
{
	return cm_is_vdev_roam_sync_inprogress(vdev);
}
#endif

#ifdef WLAN_FEATURE_HOST_ROAM
bool ucfg_cm_is_vdev_roam_preauth_state(struct wlan_objmgr_vdev *vdev)
{
	return cm_is_vdev_roam_preauth_state(vdev);
}

bool ucfg_cm_is_vdev_roam_reassoc_state(struct wlan_objmgr_vdev *vdev)
{
	return cm_is_vdev_roam_reassoc_state(vdev);
}
#endif

enum band_info ucfg_cm_get_connected_band(struct wlan_objmgr_vdev *vdev)
{
	struct wlan_channel *bss_chan;
	uint32_t sta_freq = 0;

	if (wlan_cm_is_vdev_connected(vdev)) {
		bss_chan = wlan_vdev_mlme_get_bss_chan(vdev);
		sta_freq = bss_chan->ch_freq;
	}

	if (wlan_reg_is_24ghz_ch_freq(sta_freq))
		return BAND_2G;
	else if (wlan_reg_is_5ghz_ch_freq(sta_freq))
		return BAND_5G;
	else	/* If station is not connected return as BAND_ALL */
		return BAND_ALL;
}
