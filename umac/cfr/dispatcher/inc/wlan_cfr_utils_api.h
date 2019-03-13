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

#define DBR_EVENT_TIMEOUT_IN_MS_CFR 1
#define DBR_NUM_RESP_PER_EVENT_CFR 1

#define IEEE80211_ADDR_LEN 6
enum cfrmetaversion {
	CFR_META_VERSION_NONE,
	CFR_META_VERSION_1,
	CFR_META_VERSION_MAX = 0xFF,
};

enum cfrdataversion {
	CFR_DATA_VERSION_NONE,
	CFR_DATA_VERSION_1,
	CFR_DATA_VERSION_MAX = 0xFF,
};

enum cfrplatformtype {
	CFR_PLATFORM_TYPE_NONE,
	CFR_PLATFORM_TYPE_MIPS,
	CFR_PLATFORM_TYPE_ARM,
	CFR_PLATFFORM_TYPE_MAX = 0xFF,
};

enum cfrradiotype {
	CFR_CAPTURE_RADIO_NONE,
	CFR_CAPTURE_RADIO_OSPREY,
	CFR_CAPTURE_RADIO_PEAKCOCK,
	CFR_CAPTURE_RADIO_SCORPION,
	CFR_CAPTURE_RADIO_HONEYBEE,
	CFR_CAPTURE_RADIO_DRAGONFLY,
	CFR_CAPTURE_RADIO_JET,
	CFR_CAPTURE_RADIO_PEREGRINE = 17,
	CFR_CAPTURE_RADIO_SWIFT,
	CFR_CAPTURE_RADIO_BEELINER,
	CFR_CAPTURE_RADIO_CASCADE,
	CFR_CAPTURE_RADIO_DAKOTA,
	CFR_CAPTURE_RADIO_BESRA,
	CFR_CAPTURE_RADIO_HKV2,
	CFR_CAPTURE_RADIO_MAX = 0xFF,
};

struct cfr_metadata_version_1 {
	u_int8_t    peer_addr[IEEE80211_ADDR_LEN];
	u_int8_t    status;
	u_int8_t    capture_bw;
	u_int8_t    channel_bw;
	u_int8_t    phy_mode;
	u_int16_t   prim20_chan;
	u_int16_t   center_freq1;
	u_int16_t   center_freq2;
	u_int8_t    capture_mode;
	u_int8_t    capture_type;
	u_int8_t    sts_count;
	u_int8_t    num_rx_chain;
	u_int32_t   timestamp;
	u_int32_t   length;
} __attribute__ ((__packed__));

struct csi_cfr_header {
	u_int32_t   start_magic_num;
	u_int32_t   vendorid;
	u_int8_t    cfr_metadata_version;
	u_int8_t    cfr_data_version;
	u_int8_t    chip_type;
	u_int8_t    pltform_type;
	u_int32_t   Reserved;

	union {
		struct cfr_metadata_version_1 meta_v1;
	} u;
} __attribute__ ((__packed__));

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

struct whal_cfir_dma_hdr {
	uint16_t
		// 'BA'
		tag                 : 8,
		// '02', length of header in 4 octet units
		length              : 6,
		// 00
		reserved            : 2;
	uint16_t
		// [16]
		upload_done         : 1,
		// [17:18], 0: invalid, 1: CFR, 2: CIR, 3: DebugH
		capture_type        : 3,
		// [19:20], 0: Legacy, 1: HT, 2: VHT, 3: HE
		preamble_type       : 2,
		// [21:23], 0: 1-stream, 1: 2-stream, ..., 7: 8-stream
		nss                 : 3,
		// [24:27], 0: invalid, 1: 1-chain, 2: 2-chain, etc.
		num_chains          : 3,
		// [28:30], 0: 20 MHz, 1: 40 MHz, 2: 80 MHz, 3: 160 MHz
		upload_pkt_bw       : 3,    // [31]
		sw_peer_id_valid    : 1;
	uint16_t
		sw_peer_id          : 16;   // [15:0]
	uint16_t
		phy_ppdu_id         : 16;   // [15:0]
};

#define MAX_LUT_ENTRIES 140 /* For HKv2 136 is max */

struct look_up_table {
	bool dbr_recv;
	bool tx_recv;
	uint8_t *data; /* capture payload */
	uint32_t data_len; /* capture len */
	uint16_t dbr_ppdu_id; /* ppdu id from dbr */
	uint16_t tx_ppdu_id; /* ppdu id from TX event */
	qdf_dma_addr_t dbr_address; /* capture len */
	uint32_t tx_address1; /* capture len */
	uint32_t tx_address2; /* capture len */
	struct csi_cfr_header header;
	struct whal_cfir_dma_hdr dma_hdr;
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
	struct look_up_table lut[MAX_LUT_ENTRIES];
	uint32_t dbr_buf_size;
	uint32_t dbr_num_bufs;
	uint32_t tx_evt_cnt;
	uint32_t dbr_evt_cnt;
	uint32_t release_cnt;
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
