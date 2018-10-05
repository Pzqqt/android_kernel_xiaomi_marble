/*
 * Copyright (c) 2019 The Linux Foundation. All rights reserved.
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
 * DOC: Define PDEV MLME structure and APIs
 */
#ifndef _WLAN_PDEV_MLME_H_
#define _WLAN_PDEV_MLME_H_

#include <qdf_timer.h>
#include <include/wlan_vdev_mlme.h>
/**
 * struct pdev_mlme_obj -  PDEV MLME component object
 * @pdev:                  PDEV object
 * @ext_pdev_ptr:          PDEV MLME legacy pointer
 * @mlme_register_ops:     Call back to register MLME legacy APIs
 * @vdev_restart_lock:     Lock for VDEVs restart
 * @restart_req_timer:     Timer handle for VDEVs restart
 * @restart_pend_vdev_bmap: Bitmap for VDEV RESTART command pending
 * @restart_send_vdev_bmap: Bitmap for VDEV RESTART command sending
 * @start_send_vdev_arr:   Bitmap for VDEV START command sending
 */
struct pdev_mlme_obj {
	struct wlan_objmgr_pdev *pdev;
	void *ext_pdev_ptr;
	QDF_STATUS (*mlme_register_ops)(struct vdev_mlme_obj *vdev_mlme);
	qdf_spinlock_t vdev_restart_lock;
	qdf_timer_t restart_req_timer;
	unsigned long restart_pend_vdev_bmap[2];
	unsigned long restart_send_vdev_bmap[2];
	unsigned long start_send_vdev_arr[2];
};

#endif
