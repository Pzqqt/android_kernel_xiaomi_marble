/*
 * Copyright (c) 2019 The Linux Foundation. All rights reserved.
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

#ifndef _WLAN_CFR_UTILS_API_H_
#define _WLAN_CFR_UTILS_API_H_
#include <wlan_objmgr_cmn.h>

/**
 * struct cfr_capture_params - structure to store cfr config param
 * bandwidth: bandwitdh of capture
 * period: period of capture
 * method: enum of method being followed to capture cfr data. 0-QoS null data
 */
struct cfr_capture_params {
	u_int8_t   bandwidth;
	u_int32_t  period;
	u_int8_t   method;
};

/**
 * struct psoc_cfr - private psoc object for cfr
 * psoc_obj: pointer to psoc object
 * is_cfr_capable: flag to determine if cfr is enabled or not
 */
struct psoc_cfr {
	struct wlan_objmgr_psoc *psoc_obj;
	uint8_t is_cfr_capable;
};

/**
 * struct pdev_cfr - private pdev object for cfr
 * pdev_obj: pointer to pdev object
 * is_cfr_capable: flag to determine if cfr is enabled or not
 * cfr_timer_enable: flag to enable/disable timer
 */
/*
 * To be extended if we get more capbality info
 * from FW's extended service ready event.
 */
struct pdev_cfr {
	struct wlan_objmgr_pdev *pdev_obj;
	uint8_t is_cfr_capable;
	uint8_t cfr_timer_enable;
	/*
	 * RealyFS data stucts can be here
	 * or add RealyFS object speperately per pdev
	 */

	/*
	 * More fileds will come for data interface
	 * to stitch Tx completion & D-DMA completions
	 */
};

/**
 * struct peer_cfr - private peer object for cfr
 * peer_obj: pointer to peer_obj
 * request: Type of request (start/stop)
 * bandwidth: bandwitdth of capture for this peer
 * capture_method: enum determining type of cfr data capture.
 *                 0-Qos null data
 */
struct peer_cfr {
	struct wlan_objmgr_peer *peer_obj;
	u_int8_t   request; /* start/stop */
	u_int8_t   bandwidth;
	u_int32_t  period;
	u_int8_t   capture_method;
};

/**
 * wlan_cfr_init() - Global init for cfr.
 *
 * Return: status of global init pass/fail
 */
QDF_STATUS wlan_cfr_init(void);

/**
 * wlan_cfr_deinit() - Global de-init for cfr.
 *
 * Return: status of global de-init pass/fail
 */
QDF_STATUS wlan_cfr_deinit(void);

/**
 * wlan_cfr_pdev_open() - pdev_open function for cfr.
 * @pdev: pointer to pdev object
 *
 * Return: status of pdev_open pass/fail
 */
QDF_STATUS wlan_cfr_pdev_open(struct wlan_objmgr_pdev *pdev);

/**
 * wlan_cfr_pdev_close() - pdev_close function for cfr.
 * @pdev: pointer to pdev object
 *
 * Return: status of pdev_close pass/fail
 */
QDF_STATUS wlan_cfr_pdev_close(struct wlan_objmgr_pdev *pdev);
#endif
