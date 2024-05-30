/*
 * Copyright (c) 2019, The Linux Foundation. All rights reserved.
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

#ifndef _WMI_UNIFIED_CFR_PARAM_H_
#define _WMI_UNIFIED_CFR_PARAM_H_

#ifdef WLAN_CFR_ENABLE

#define WMI_HOST_PEER_CFR_TIMER_ENABLE   1
#define WMI_HOST_PEER_CFR_TIMER_DISABLE  0


/**
 * struct peer_cfr_params - peer cfr capture cmd parameter
 * @request: enable/disable cfr capture
 * @macaddr: macaddr of the client
 * @vdev_id: vdev id
 * @periodicity: cfr capture period
 * @bandwidth: bandwidth of cfr capture
 * @capture_method: cfr capture method/type
 */
struct peer_cfr_params {
	uint32_t request;
	uint8_t  *macaddr;
	uint32_t vdev_id;
	uint32_t periodicity;
	uint32_t bandwidth;
	uint32_t capture_method;
};


#endif /* WLAN_CFR_ENABLE */
#endif /* _WMI_UNIFIED_CFR_PARAM_H_ */
