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
#include <qdf_list.h>
#include <qdf_types.h>

#ifdef WLAN_ENABLE_AGEIE_ON_SCAN_RESULTS
/* GPS application requirement */
#define QCOM_VENDOR_IE_ID 221
#define QCOM_OUI1         0x00
#define QCOM_OUI2         0xA0
#define QCOM_OUI3         0xC6
#define QCOM_VENDOR_IE_AGE_TYPE  0x100
#define QCOM_VENDOR_IE_AGE_LEN   (sizeof(qcom_ie_age) - 2)

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
*/
struct osif_scan_pdev{
	qdf_list_t scan_req_q;
};

/**
 * wlan_cfg80211_scan() - API to process cfg80211 scan request
 * @wiphy: Pointer to wiphy
 * @dev: Pointer to net device
 * @request: Pointer to scan request
 *
 * API to trigger scan and update cfg80211 scan database.
 * scan dump command can be used to fetch scan results
 * on receipt of scan complete event.
 *
 * Return: 0 for success, non zero for failure
 */
int wlan_cfg80211_scan(struct wiphy *wiphy,
#if (LINUX_VERSION_CODE < KERNEL_VERSION(3, 6, 0))
		struct net_device *dev,
#endif
		struct cfg80211_scan_request *request);


#endif
