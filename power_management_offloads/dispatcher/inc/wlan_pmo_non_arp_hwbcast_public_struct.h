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
  * DOC: Declare various struct, macros which shall be used in
  * pmo arp offload feature.
  *
  * Note: This file shall not contain public API's prototype/declarations.
  *
  */

#ifndef _WLAN_PMO_NON_ARP_HW_BCAST_FILTER_PUBLIC_STRUCT_H_
#define _WLAN_PMO_NON_ARP_HW_BCAST_FILTER_PUBLIC_STRUCT_H_

#include "wlan_pmo_common_public_struct.h"

/**
 * struct pmo_bcast_filter_req - pmo arp request
 * @psoc: objmgr psoc
 * @vdev_id: vdev id on which arp offload needed
 */
struct pmo_bcast_filter_req {
	struct wlan_objmgr_psoc *psoc;
	uint8_t vdev_id;
};

/**
 * struct pmo_bcast_filter_params - For enable/disable pmo HW Broadcast Filter
 * @enable: value to enable disable feature
 * @bss_id: bss_id for get session.
 */
struct pmo_bcast_filter_params {
	bool enable;
	struct qdf_mac_addr bssid;
};

#endif /* end  of _WLAN_PMO_NON_ARP_HW_BCAST_FILTER_PUBLIC_STRUCT_H_ */

