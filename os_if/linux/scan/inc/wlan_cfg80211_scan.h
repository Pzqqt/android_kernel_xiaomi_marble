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
 * DOC: declares driver functions interfacing with linux kernel
 */


#ifndef _WLAN_CFG80211_SCAN_H_
#define _WLAN_CFG80211_SCAN_H_

#include <linux/version.h>
#include <linux/netdevice.h>
#include <net/cfg80211.h>
#include <qca_vendor.h>
#include <wlan_scan_public_structs.h>
#include <qdf_list.h>
#include <qdf_types.h>
#include <wlan_scan_ucfg_api.h>


/* Max number of scans allowed from userspace */
#define WLAN_MAX_SCAN_COUNT 8

#ifdef WLAN_ENABLE_AGEIE_ON_SCAN_RESULTS
/* GPS application requirement */
#define QCOM_VENDOR_IE_ID 221
#define QCOM_OUI1         0x00
#define QCOM_OUI2         0xA0
#define QCOM_OUI3         0xC6
#define QCOM_VENDOR_IE_AGE_TYPE  0x100
#define QCOM_VENDOR_IE_AGE_LEN   (sizeof(qcom_ie_age) - 2)
#define SCAN_DONE_EVENT_BUF_SIZE 4096
#define INVAL_SCAN_ID        0xFFFFFFFF
#define INVAL_VDEV_ID        0xFFFFFFFF
#define INVAL_PDEV_ID        0xFFFFFFFF


/**
 * typedef struct qcom_ie_age - age ie
 *
 * @element_id: Element id
 * @len: Length
 * @oui_1: OUI 1
 * @oui_2: OUI 2
 * @oui_3: OUI 3
 * @type: Type
 * @age: Age
 * @tsf_delta: tsf delta from FW
 * @beacon_tsf: original beacon TSF
 * @seq_ctrl: sequence control field
 */
typedef struct {
	u8 element_id;
	u8 len;
	u8 oui_1;
	u8 oui_2;
	u8 oui_3;
	u32 type;
	u32 age;
	u32 tsf_delta;
	u64 beacon_tsf;
	u16 seq_ctrl;
} __attribute__ ((packed)) qcom_ie_age;
#endif

/**
 * struct osif_scan_pdev - OS scan private strcutre
 * scan_req_q: Scan request queue
 * req_id: Scan request Id
*/
struct osif_scan_pdev{
	qdf_list_t scan_req_q;
	wlan_scan_requester req_id;
};

/*
 * enum scan_source - scan request source
 * @NL_SCAN: Scan initiated from NL
 * @VENDOR_SCAN: Scan intiated from vendor command
 */
enum scan_source {
	NL_SCAN,
	VENDOR_SCAN,
};

/**
 * struct scan_req - Scan Request entry
 * @node : List entry element
 * @scan_request: scan request holder
 * @scan_id: scan identifier used across host layers which is generated at WMI
 * @source: scan request originator (NL/Vendor scan)
 *
 * Scan request linked list element
 */
struct scan_req {
	qdf_list_node_t node;
	struct cfg80211_scan_request *scan_request;
	uint32_t scan_id;
	uint8_t source;
};

#ifdef FEATURE_WLAN_SCAN_PNO
/**
 * wlan_cfg80211_sched_scan_start() - cfg80211 scheduled scan(pno) start
 * @pdev: pdev pointer
 * @dev: Pointer network device
 * @request: Pointer to cfg80211 scheduled scan start request
 *
 * Return: 0 for success, non zero for failure
 */
int wlan_cfg80211_sched_scan_start(struct wlan_objmgr_pdev *pdev,
	struct net_device *dev,
	struct cfg80211_sched_scan_request *request);

/**
 * wlan_cfg80211_sched_scan_stop() - cfg80211 scheduled scan(pno) stop
 * @pdev: pdev pointer
 * @dev: Pointer network device
 *
 * Return: 0 for success, non zero for failure
 */
int wlan_cfg80211_sched_scan_stop(struct wlan_objmgr_pdev *pdev,
	struct net_device *dev);
#endif

/**
 * wlan_cfg80211_scan_priv_init() - API to initialize cfg80211 scan
 * @pdev: Pointer to net device
 *
 * API to initialize cfg80211 scan module.
 *
 * Return: QDF_STATUS
 */
QDF_STATUS wlan_cfg80211_scan_priv_init(struct wlan_objmgr_pdev *pdev);

/**
 * wlan_cfg80211_scan_priv_deinit() - API to deinitialize cfg80211 scan
 * @pdev: Pointer to net device
 *
 * API to deinitialize cfg80211 scan module.
 *
 * Return: QDF_STATUS
 */
QDF_STATUS wlan_cfg80211_scan_priv_deinit(
		struct wlan_objmgr_pdev *pdev);

/**
 * wlan_cfg80211_scan() - API to process cfg80211 scan request
 * @pdev: Pointer to pdev
 * @request: Pointer to scan request
 * @source: source of scan request
 *
 * API to trigger scan and update cfg80211 scan database.
 * scan dump command can be used to fetch scan results
 * on receipt of scan complete event.
 *
 * Return: 0 for success, non zero for failure
 */
int wlan_cfg80211_scan(struct wlan_objmgr_pdev *pdev,
		struct cfg80211_scan_request *request,
		uint8_t source);

/**
 * wlan_cfg80211_inform_bss_frame() - API to inform beacon to cfg80211
 * @pdev: Pointer to pdev
 * @scan_params: scan entry
 * @request: Pointer to scan request
 *
 * API to inform beacon to cfg80211
 *
 * Return: void
 */
void wlan_cfg80211_inform_bss_frame(struct wlan_objmgr_pdev *pdev,
	struct scan_cache_entry *scan_params);

/**
 * wlan_vendor_abort_scan() - API to vendor abort scan
 * @pdev: Pointer to pdev
 * @data: pointer to data
 * @data_len: Data length
 *
 * API to abort scan through vendor command
 *
 * Return: 0 for success, non zero for failure
 */
int wlan_vendor_abort_scan(struct wlan_objmgr_pdev *pdev,
				const void *data, int data_len);

/**
 * wlan_cfg80211_abort_scan() - API to abort scan through cfg80211
 * @pdev: Pointer to pdev
 *
 * API to abort scan through cfg80211 request
 *
 * Return: 0 for success, non zero for failure
 */
int wlan_cfg80211_abort_scan(struct wlan_objmgr_pdev *pdev);

/**
 * wlan_abort_scan() - Generic API to abort scan request
 * @pdev: Pointer to pdev
 * @pdev_id: pdev id
 * @vdev_id: vdev id
 * @scan_id: scan id
 *
 * Generic API to abort scans
 *
 * Return: 0 for success, non zero for failure
 */
QDF_STATUS wlan_abort_scan(struct wlan_objmgr_pdev *pdev,
				   uint32_t pdev_id,
				   uint32_t vdev_id,
				   wlan_scan_id scan_id);

/**
 * wlan_cfg80211_cleanup_scan_queue() - remove entries in scan queue
 * @pdev: pdev pointer
 *
 * Removes entries in scan queue and sends scan complete event to NL
 *
 * Return: None
 */
void wlan_cfg80211_cleanup_scan_queue(struct wlan_objmgr_pdev *pdev);


#endif
