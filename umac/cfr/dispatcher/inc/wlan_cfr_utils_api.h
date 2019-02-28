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
#include <qal_streamfs.h>

#define cfr_alert(format, args...) \
		QDF_TRACE_FATAL(QDF_MODULE_ID_CFR, format, ## args)

#define cfr_err(format, args...) \
		QDF_TRACE_ERROR(QDF_MODULE_ID_CFR, format, ## args)

#define cfr_warn(format, args...) \
		QDF_TRACE_WARN(QDF_MODULE_ID_CFR, format, ## args)

#define cfr_info(format, args...) \
		QDF_TRACE_INFO(QDF_MODULE_ID_CFR, format, ## args)

#define cfr_debug(format, args...) \
		QDF_TRACE_DEBUG(QDF_MODULE_ID_CFR, format, ## args)

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
 * struct cfr_wmi_host_mem_chunk - wmi mem chunk related
 * vaddr: pointer to virtual address
 * paddr: physical address
 * len: len of the mem chunk allocated
 * req_id: reqid related to the mem chunk
 */
struct cfr_wmi_host_mem_chunk {
	uint32_t *vaddr;
	uint32_t paddr;
	uint32_t len;
	uint32_t req_id;
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
	struct cfr_wmi_host_mem_chunk cfr_mem_chunk;
	uint16_t cfr_max_sta_count;
	uint16_t cfr_current_sta_count;

	uint32_t num_subbufs;
	uint32_t subbuf_size;
	struct qal_streamfs_chan *chan_ptr;
	struct qal_dentry_t *dir_ptr;
	/*
	 * More fileds will come for data interface
	 * to stitch Tx completion & D-DMA completions
	 */
};

#define PEER_CFR_CAPTURE_ENABLE   1
#define PEER_CFR_CAPTURE_DISABLE  0
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
	u_int8_t   request;            /* start/stop */
	u_int8_t   bandwidth;
	u_int32_t  period;
	u_int8_t   capture_method;
};

/**
 * cfr_initialize_pdev() - cfr initialize pdev
 * @pdev: Pointer to pdev_obj
 *
 * Return: status of cfr pdev init
 */
QDF_STATUS cfr_initialize_pdev(struct wlan_objmgr_pdev *pdev);

/**
 * cfr_deinitialize_pdev() - cfr deinitialize pdev
 * @pdev: Pointer to pdev_obj
 *
 * Return: status of cfr pdev deinit
 */
QDF_STATUS cfr_deinitialize_pdev(struct wlan_objmgr_pdev *pdev);

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
