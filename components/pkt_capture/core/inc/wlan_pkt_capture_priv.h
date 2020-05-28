/*
 * Copyright (c) 2020 The Linux Foundation. All rights reserved.
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
 * DOC: Declare private API which shall be used internally only
 * in pkt_capture component. This file shall include prototypes of
 * pkt_capture parsing and send logic.
 *
 * Note: This API should be never accessed out of pkt_capture component.
 */

#ifndef _WLAN_PKT_CAPTURE_PRIV_STRUCT_H_
#define _WLAN_PKT_CAPTURE_PRIV_STRUCT_H_

#include "wlan_pkt_capture_objmgr.h"
#include "wlan_pkt_capture_public_structs.h"
#include "wlan_pkt_capture_mon_thread.h"

/**
 * struct pkt_capture_cfg - packet capture cfg to store ini values
 * @pkt_capture_mode: packet capture mode
 */
struct pkt_capture_cfg {
	enum pkt_capture_mode pkt_capture_mode;
};

/**
 * struct pkt_capture_cb_context - packet capture callback context
 * @mon_cb: monitor callback function pointer
 * @mon_ctx: monitor callback context
 * @pkt_capture_mode: packet capture mode
 */
struct pkt_capture_cb_context {
	QDF_STATUS (*mon_cb)(void *, qdf_nbuf_t);
	void *mon_ctx;
	enum pkt_capture_mode pkt_capture_mode;
};

/**
 * struct pkt_capture_vdev_priv - Private object to be stored in vdev
 * @vdev: pointer to vdev object
 * @mon_ctx: pointer to packet capture mon context
 * @cb_ctx: pointer to packet capture mon callback context
 * @rx_ops: rx ops
 * @tx_ops: tx ops
 */
struct pkt_capture_vdev_priv {
	struct wlan_objmgr_vdev *vdev;
	struct pkt_capture_mon_context *mon_ctx;
	struct pkt_capture_cb_context *cb_ctx;
	struct wlan_pkt_capture_rx_ops rx_ops;
	struct wlan_pkt_capture_tx_ops tx_ops;
};

/**
 * struct pkt_psoc_priv - Private object to be stored in psoc
 * @psoc: pointer to psoc object
 * @cfg_param: INI config params for packet capture
 * @cb_obj: struct contaning callback pointers
 */
struct pkt_psoc_priv {
	struct wlan_objmgr_psoc *psoc;
	struct pkt_capture_cfg cfg_param;
	struct pkt_capture_callbacks cb_obj;
};
#endif /* End  of _WLAN_PKT_CAPTURE_PRIV_STRUCT_H_ */
