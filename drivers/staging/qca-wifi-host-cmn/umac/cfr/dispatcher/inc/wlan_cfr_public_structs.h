/*
 * Copyright (c) 2020-2021, The Linux Foundation. All rights reserved.
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
 * DOC: declare the data structure to hold CFR specific configurations
 */
#ifndef _WLAN_CFR_PUBLIC_STRUCTS_H_
#define _WLAN_CFR_PUBLIC_STRUCTS_H_

#define MAC_ADDR_LEN 6

#define MAX_CAPTURE_COUNT_VAL 0xFFFF

/**
 * cfr_cwm_width : Capture bandwidth
 * 0 : 20MHz, 1 : 40MHz, 2 : 80MHz, 3 : 160MHz, 4 : 80+80MHz
 * 5 : 320MHz
 */
enum cfr_cwm_width {
	CFR_CWM_WIDTH20,
	CFR_CWM_WIDTH40,
	CFR_CWM_WIDTH80,
	CFR_CWM_WIDTH160,
	CFR_CWM_WIDTH80_80,
	CFR_CWM_WIDTH320,

	CFR_CWM_WIDTH_MAX,
	CFR_CWM_WIDTHINVALID = 0xff
};

/**
 * cfr_capture_method : Tx based CFR capture method
 * @CFR_CAPTURE_METHOD_QOS_NULL : Send QOS Null frame and capture CFR on ACK
 * @CFR_CAPTURE_METHOD_QOS_NULL_WITH_PHASE: Send QoS Null frame with phase
 * @CFR_CAPTURE_METHOD_PROBE_RESPONSE : Capture is enabled on probe response
 * If node is not found, trigger unassociated capture.
 */
enum cfr_capture_method {
	CFR_CAPTURE_METHOD_QOS_NULL = 0,
	CFR_CAPTURE_METHOD_QOS_NULL_WITH_PHASE = 1,
	CFR_CAPTURE_METHOD_PROBE_RESPONSE = 2,
	CFR_CAPTURE_METHOD_LAST_VALID,
	CFR_CAPTURE_METHOD_AUTO = 0xff,
	CFR_CAPTURE_METHOD_MAX,
};

/**
 * cfr_wlanconfig_param : CFR params used to store user provided inputs
 * @bandwidth : CFR capture bandwidth
 * @periodicity : CFR capture periodicity in milli seconds
 * @capture_method : CFR capture method
 * @mac : peer mac address
 * @ta : Tx address
 * @ra : Rx Address
 * @ta_mask: Tx address mask
 * @ra_mask; Rx address mask
 * *** Controls for different capture modes in RCC ***
 * @en_directed_ftm: Enable capture for directed RTT FTM Packet
 * @en_directed_ndpa_ndp: Enable NDPA filter followed by directed NDP capture
 * @en_ta_ra_filter: Enable MAC TA/RA/type filtering channel capture
 * @en_all_ftm_ack: Enable all FTM and ACK capture
 * @en_ndpa_ndp_all: Enable NDPA filter followed by NDP capture,
 *                   capture includes both directed and non-directed packets.
 * @en_all_pkt: Enable capture mode to filter in all packets
 * @dis_directed_ftm: Drop directed RTT FTM packets
 * @dis_directed_ndpa_ndp: Drop directed NDPA and NDP packets
 * @dis_ta_ra_filter: Disable MAC TA/RA/type filtering channel capture
 * @dis_all_ftm_ack: Drop all FTM and ACK capture
 * @dis_ndpa_ndp_all: Drop all NDPA and NDP packets
 * @dis_all_pkt: Do not filter in any packet
 * @en_ta_ra_filter_in_as_fp: Filter in frames as FP/MO in m_ta_ra_filter mode
 *		0: as MO
 *		1: as FP
 *
 * **** Fixed parameters ****
 * @cap_dur: Capture duration
 * @cap_intvl: Capture interval
 * FW may limit the interval and duration during which HW may attempt
 * to capture by programming the user provided values.
 * These values(cap_dur, cap_intvl) range from 1 us to roughly 16.8 in 1 us
 * units. Max value is 0xFFFFFF, i.e., 16.777215 s
 * @bw: Bandwidth: 20, 40, 80, 160, 240, 320MHz
 * @nss: 8 bits are allotted for NSS mask. Specifies which numbers of
 * spatial streams (MIMO factor) are permitted
 * @grp_id:  Group id could of any value between 0 and 15
 * @expected_mgmt_subtype/ expected_ctrl_subtype / expected_data_subtype:
 * corresponds to mgmt/ ctrl/ data, all are bitmasks, in which each bit
 * represents the corresponding type/ subtype value as per IEEE80211.
 *
 * @en_cfg and reset_cfg: This bitmap of 16 bits, indicates 16 groups.
 * Valid entry should be in between 0 to 0xFFFF.
 * Turning on a bit in en_cfg will enable MAC TA_RA filter
 * for corresponding group; whereas turning on a bit in reset_cfg
 * will reset all 9 params in the corresponding group to default values.
 *
 * @ul_mu_user_mask_lower, ul_mu_user_mask_upper :
 * Since Cypress supports max bandwidth of 80Mhz, maximum number
 * of users in a UL MU-MIMO transmission would be 37.
 * mask_lower_32: Bits from 31 to 0 indicate user indices for 32 users.
 * mask_upper_32: Bits from 0 to 4 indicate user indices from 33 to 37.
 *
 * @ freeze_tlv_delay_cnt_en, freeze_tlv_delay_cnt_thr :
 * freeze_tlv_delay_cnt_thr will decide the threshold for MAC to drop the
 * freeze TLV. freeze_tlv_delay_cnt_thr will only be applicable if
 * freeze_tlv_delay_cnt_en is enabled.
 *
 * @cap_count: After capture_count+1 number of captures, MAC stops RCC and
 * waits for capture_interval duration before enabling again
 *
 * @cap_intval_mode_sel: 0 indicates capture_duration mode, 1 indicates the
 * capture_count mode.
 */
struct cfr_wlanconfig_param {
	enum cfr_cwm_width bandwidth;
	uint32_t periodicity;
	enum cfr_capture_method capture_method;
	uint8_t mac[MAC_ADDR_LEN];
#ifdef WLAN_ENH_CFR_ENABLE
	uint8_t ta[MAC_ADDR_LEN];
	uint8_t ra[MAC_ADDR_LEN];
	uint8_t ta_mask[MAC_ADDR_LEN];
	uint8_t ra_mask[MAC_ADDR_LEN];
	uint16_t en_directed_ftm             :1,
		 en_directed_ndpa_ndp        :1,
		 en_ta_ra_filter             :1,
		 en_all_ftm_ack              :1,
		 en_ndpa_ndp_all             :1,
		 en_all_pkt                  :1,
		 dis_directed_ftm            :1,
		 dis_directed_ndpa_ndp       :1,
		 dis_ta_ra_filter            :1,
		 dis_all_ftm_ack             :1,
		 dis_ndpa_ndp_all            :1,
		 dis_all_pkt                 :1,
		 en_ta_ra_filter_in_as_fp    :1,
		 rsvd0                       :3;

	uint32_t cap_dur                     :24,
		 rsvd1                       :8;
	uint32_t cap_intvl                   :24,
		 rsvd2                       :8;
	uint32_t bw                          :6,
		 nss                         :8,
		 grp_id                      :4,
		 rsvd3                       :14;

	uint32_t expected_mgmt_subtype       :16,
		 expected_ctrl_subtype       :16;

	uint32_t expected_data_subtype       :16,
		 rsvd5                       :16;

	uint32_t en_cfg                      :16,
		 reset_cfg                   :16;

	uint32_t ul_mu_user_mask_lower;
	uint32_t ul_mu_user_mask_upper;

	uint32_t freeze_tlv_delay_cnt_en :1,
		 freeze_tlv_delay_cnt_thr :8,
		 rsvd6 :23;

	uint32_t cap_count                   :16,
		 cap_intval_mode_sel         :1,
		 rsvd7                       :15;
#endif
#ifdef WLAN_FEATURE_11BE
	uint32_t puncture_bitmap;
#endif
};

#endif /* _WLAN_CFR_PUBLIC_STRUCTS_H_ */

