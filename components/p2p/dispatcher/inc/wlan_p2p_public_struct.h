/*
 * Copyright (c) 2017-2018 The Linux Foundation. All rights reserved.
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
 * DOC: Contains p2p public data structure definations
 */

#ifndef _WLAN_P2P_PUBLIC_STRUCT_H_
#define _WLAN_P2P_PUBLIC_STRUCT_H_

#include <qdf_types.h>

#define P2P_MAX_NOA_DESC 4

/**
 * struct p2p_ps_params - P2P powersave related params
 * @opp_ps: opportunistic power save
 * @ctwindow: CT window
 * @count: count
 * @duration: duration
 * @interval: interval
 * @single_noa_duration: single shot noa duration
 * @ps_selection: power save selection
 * @session_id: session id
 */
struct p2p_ps_params {
	uint8_t opp_ps;
	uint32_t ctwindow;
	uint8_t count;
	uint32_t duration;
	uint32_t interval;
	uint32_t single_noa_duration;
	uint8_t ps_selection;
	uint8_t session_id;
};

/**
 * struct p2p_roc_req - P2P roc request
 * @vdev_id:     Vdev id on which this request has come
 * @chan:        Chan for which this RoC has been requested
 * @phy_mode:    PHY mode
 * @duration:    Duration for the RoC
 */
struct p2p_roc_req {
	uint32_t vdev_id;
	uint32_t chan;
	uint32_t phy_mode;
	uint32_t duration;
};

/**
 * enum p2p_roc_event - P2P RoC event
 * @ROC_EVENT_READY_ON_CHAN:  RoC has started now
 * @ROC_EVENT_COMPLETED:      RoC has been completed
 * @ROC_EVENT_INAVLID:        Invalid event
 */
enum p2p_roc_event {
	ROC_EVENT_READY_ON_CHAN = 0,
	ROC_EVENT_COMPLETED,
	ROC_EVENT_INAVLID,
};

/**
 * struct p2p_event - p2p event
 * @vdev_id:     Vdev id
 * @roc_event:   RoC event
 * @cookie:      Cookie which is given to supplicant for this roc req
 * @chan:        Chan for which this RoC has been requested
 * @duration:    Duration for the RoC
  */
struct p2p_event {
	uint32_t vdev_id;
	enum p2p_roc_event roc_event;
	uint64_t cookie;
	uint32_t chan;
	uint32_t duration;
};

/**
 * struct p2p_rx_mgmt_frame - rx mgmt frame structure
 * @frame_len:   Frame length
 * @rx_chan:     RX channel
 * @vdev_id:     Vdev id
 * @frm_type:    Frame type
 * @rx_rssi:     RX rssi
 * @buf:         Buffer address
 */
struct p2p_rx_mgmt_frame {
	uint32_t frame_len;
	uint32_t rx_chan;
	uint32_t vdev_id;
	uint32_t frm_type;
	uint32_t rx_rssi;
	uint8_t buf[1];
};

/**
 * struct p2p_tx_cnf - tx confirm structure
 * @vdev_id:        Vdev id
 * @action_cookie:  TX cookie for this action frame
 * @buf_len:        Frame length
 * @status:         TX status
 * @buf:            Buffer address
 */
struct p2p_tx_cnf {
	uint32_t vdev_id;
	uint64_t action_cookie;
	uint32_t buf_len;
	uint32_t status;
	uint8_t *buf;
};

/**
 * struct p2p_mgmt_tx - p2p mgmt tx structure
 * @vdev_id:             Vdev id
 * @chan:                Chan for which this RoC has been requested
 * @wait:                Duration for the RoC
 * @len:                 Length of tx buffer
 * @no_cck:              Required cck or not
 * @dont_wait_for_ack:   Wait for ack or not
 * @off_chan:            Off channel tx or not
 * @buf:                 TX buffer
 */
struct p2p_mgmt_tx {
	uint32_t vdev_id;
	uint32_t chan;
	uint32_t wait;
	uint32_t len;
	uint32_t no_cck;
	uint32_t dont_wait_for_ack;
	uint32_t off_chan;
	const uint8_t *buf;
};

/**
 * struct p2p_set_mac_filter
 * @vdev_id: Vdev id
 * @mac: mac addr
 * @freq: frequency
 * @set: set or clear
 */
struct p2p_set_mac_filter {
	uint32_t vdev_id;
	uint8_t mac[QDF_MAC_ADDR_SIZE];
	uint32_t freq;
	bool set;
};

/**
 * struct p2p_set_mac_filter_evt
 * @vdev_id: Vdev id
 * @status: target reported result of set mac addr filter
 */
struct p2p_set_mac_filter_evt {
	uint32_t vdev_id;
	uint32_t status;
};

/**
 * struct p2p_ps_config
 * @vdev_id:               Vdev id
 * @opp_ps:                Opportunistic power save
 * @ct_window:             CT window
 * @count:                 Count
 * @duration:              Duration
 * @interval:              Interval
 * @single_noa_duration:   Single shot noa duration
 * @ps_selection:          power save selection
 */
struct p2p_ps_config {
	uint32_t vdev_id;
	uint32_t opp_ps;
	uint32_t ct_window;
	uint32_t count;
	uint32_t duration;
	uint32_t interval;
	uint32_t single_noa_duration;
	uint32_t ps_selection;
};

/**
 * struct p2p_lo_start - p2p listen offload start
 * @vdev_id:            Vdev id
 * @ctl_flags:          Control flag
 * @freq:               P2P listen frequency
 * @period:             Listen offload period
 * @interval:           Listen offload interval
 * @count:              Number listen offload intervals
 * @dev_types_len:      Device types length
 * @probe_resp_len:     Probe response template length
 * @device_types:       Device types
 * @probe_resp_tmplt:   Probe response template
 */
struct p2p_lo_start {
	uint32_t vdev_id;
	uint32_t ctl_flags;
	uint32_t freq;
	uint32_t period;
	uint32_t interval;
	uint32_t count;
	uint32_t dev_types_len;
	uint32_t probe_resp_len;
	uint8_t  *device_types;
	uint8_t  *probe_resp_tmplt;
};

/**
 * struct p2p_lo_event
 * @vdev_id:        vdev id
 * @reason_code:    reason code
 */
struct p2p_lo_event {
	uint32_t vdev_id;
	uint32_t reason_code;
};

/**
 * struct noa_descriptor - noa descriptor
 * @type_count:     255: continuous schedule, 0: reserved
 * @duration:       Absent period duration in micro seconds
 * @interval:       Absent period interval in micro seconds
 * @start_time:     32 bit tsf time when in starts
 */
struct noa_descriptor {
	uint32_t type_count;
	uint32_t duration;
	uint32_t interval;
	uint32_t start_time;
};

/**
 * struct p2p_noa_info - p2p noa information
 * @index:             identifies instance of NOA su element
 * @opps_ps:           opps ps state of the AP
 * @ct_window:         ct window in TUs
 * @vdev_id:           vdev id
 * @num_descriptors:   number of NOA descriptors
 * @noa_desc:          noa descriptors
 */
struct p2p_noa_info {
	uint32_t index;
	uint32_t opps_ps;
	uint32_t ct_window;
	uint32_t vdev_id;
	uint32_t num_desc;
	struct noa_descriptor noa_desc[P2P_MAX_NOA_DESC];
};

/**
 * struct p2p_protocol_callbacks - callback to non-converged driver
 * @is_mgmt_protected: func to get 11w mgmt protection status
 */
struct p2p_protocol_callbacks {
	bool (*is_mgmt_protected)(uint32_t vdev_id, const uint8_t *peer_addr);
};

#endif /* _WLAN_P2P_PUBLIC_STRUCT_H_ */
