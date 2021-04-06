/*
 * Copyright (c) 2021, The Linux Foundation. All rights reserved.
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

/*
 * DOC: contains MLO manager ap related functionality
 */
#include "wlan_objmgr_vdev_obj.h"
#include "wlan_mlo_mgr_ap.h"

void mlo_ap_link_start_rsp_notify(struct wlan_objmgr_vdev *vdev)
{
	/* This is to notify the link is completed start*/
	// mlo_is_ap_vdev_up_allowed()
}

/* Need to take care of DFS CAC WAIT state*/
bool mlo_is_ap_vdev_up_allowed(struct wlan_objmgr_vdev *vdev)
{
// max_links = get the number of links(internal ml dev ctx);

/* if ( max_links == vdev_list_count)
 *	while ( iterate over vdev object)
 *		check the vdev object status;
 * If ( All sub states are vdev UP WAIT)
 *	Call VDEV MLME to send the VDEV_UP command by sending
 *	WLAN_VDEV_SM_SYNC_COMPLETE on the vdev's which are in UP_WAIT_STATE.
 */

	return true;
}

void mlo_ap_link_down_notify(struct wlan_objmgr_vdev *vdev)
{
/* No op */
}

bool mlo_is_mld_ap(struct wlan_objmgr_vdev *vdev)
{
	return true;
}
