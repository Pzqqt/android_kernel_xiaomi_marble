/*
 * Copyright (c) 2019-2020 The Linux Foundation. All rights reserved.
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
 * DOC: contains fw offload structure definations
 */

#ifndef _WLAN_FWOL_PUBLIC_STRUCTS_H_
#define _WLAN_FWOL_PUBLIC_STRUCTS_H_

#include "wlan_objmgr_psoc_obj.h"

#ifdef WLAN_FEATURE_ELNA
/**
 * struct set_elna_bypass_request - set eLNA bypass request
 * @vdev_id: vdev id
 * @en_dis: 0 - disable eLNA bypass
 *          1 - enable eLNA bypass
 */
struct set_elna_bypass_request {
	uint8_t vdev_id;
	uint8_t en_dis;
};

/**
 * struct get_elna_bypass_request - get eLNA bypass request
 * @vdev_id: vdev id
 */
struct get_elna_bypass_request {
	uint8_t vdev_id;
};

/**
 * struct get_elna_bypass_response - get eLNA bypass response
 * @vdev_id: vdev id
 * @en_dis: 0 - disable eLNA bypass
 *          1 - enable eLNA bypass
 */
struct get_elna_bypass_response {
	uint8_t vdev_id;
	uint8_t en_dis;
};
#endif

/**
 * struct wlan_fwol_callbacks - fw offload callbacks
 * @get_elna_bypass_callback: callback for get eLNA bypass
 * @get_elna_bypass_context: context for get eLNA bypass
 */
struct wlan_fwol_callbacks {
#ifdef WLAN_FEATURE_ELNA
	void (*get_elna_bypass_callback)(void *context,
				     struct get_elna_bypass_response *response);
	void *get_elna_bypass_context;
#endif
};

/**
 * struct wlan_fwol_tx_ops - structure of tx func pointers
 * @set_elna_bypass: set eLNA bypass
 * @get_elna_bypass: get eLNA bypass
 * @reg_evt_handler: register event handler
 * @unreg_evt_handler: unregister event handler
 * @send_dscp_up_map_to_fw: send dscp-to-up map values to FW
 */
struct wlan_fwol_tx_ops {
#ifdef WLAN_FEATURE_ELNA
	QDF_STATUS (*set_elna_bypass)(struct wlan_objmgr_psoc *psoc,
				      struct set_elna_bypass_request *req);
	QDF_STATUS (*get_elna_bypass)(struct wlan_objmgr_psoc *psoc,
				      struct get_elna_bypass_request *req);
#endif
	QDF_STATUS (*reg_evt_handler)(struct wlan_objmgr_psoc *psoc,
				      void *arg);
	QDF_STATUS (*unreg_evt_handler)(struct wlan_objmgr_psoc *psoc,
					void *arg);
#ifdef WLAN_SEND_DSCP_UP_MAP_TO_FW
	QDF_STATUS (*send_dscp_up_map_to_fw)(
			struct wlan_objmgr_psoc *psoc,
			uint32_t *dscp_to_up_map);
#endif
};

/**
 * struct wlan_fwol_rx_ops - structure of rx func pointers
 * @get_elna_bypass_resp: get eLNA bypass response
 */
struct wlan_fwol_rx_ops {
#ifdef WLAN_FEATURE_ELNA
	QDF_STATUS (*get_elna_bypass_resp)(struct wlan_objmgr_psoc *psoc,
					 struct get_elna_bypass_response *resp);
#endif
};

#endif /* _WLAN_FWOL_PUBLIC_STRUCTS_H_ */

