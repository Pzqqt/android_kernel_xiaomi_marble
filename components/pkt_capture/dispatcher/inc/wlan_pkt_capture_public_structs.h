/*
 * Copyright (c) 2020 The Linux Foundation. All rights reserved.
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

#ifndef _WLAN_PKT_CAPTURE_PUBLIC_STRUCTS_H_
#define _WLAN_PKT_CAPTURE_PUBLIC_STRUCTS_H_

/**
 * enum pkt_capture_mode - packet capture modes
 * @PACKET_CAPTURE_MODE_DISABLE: packet capture mode disable
 * @PACKET_CAPTURE_MODE_MGMT_ONLY: capture mgmt packets only
 * @PACKET_CAPTURE_MODE_DATA_ONLY: capture data packets only
 * @PACKET_CAPTURE_MODE_DATA_MGMT: capture both data and mgmt packets
 */
enum pkt_capture_mode {
	PACKET_CAPTURE_MODE_DISABLE = 0,
	PACKET_CAPTURE_MODE_MGMT_ONLY,
	PACKET_CAPTURE_MODE_DATA_ONLY,
	PACKET_CAPTURE_MODE_DATA_MGMT,
};

/**
 * struct mgmt_offload_event_params - Management offload event params
 * @tsf_l32: The lower 32 bits of the TSF
 * @chan_freq: channel frequency in MHz
 * @rate_kbps: Rate kbps
 * @rssi: combined RSSI, i.e. the sum of the snr + noise floor (dBm units)
 * @buf_len: length of the frame in bytes
 * @tx_status: 0: xmit ok
 *             1: excessive retries
 *             2: blocked by tx filtering
 *             4: fifo underrun
 *             8: swabort
 * @buf: management frame buffer
 * @tx_retry_cnt: tx retry count
 */
struct mgmt_offload_event_params {
	uint32_t tsf_l32;
	uint32_t chan_freq;
	uint32_t rate_kbps;
	uint32_t rssi;
	uint32_t buf_len;
	uint32_t tx_status;
	uint8_t *buf;
	uint8_t tx_retry_cnt;
};

/**
 * struct pkt_capture_callbacks - callbacks to non-converged driver
 * @get_rmf_status: callback to get rmf status
 */
struct pkt_capture_callbacks {
	int (*get_rmf_status)(uint8_t vdev_id);
};

/**
 * struct wlan_pkt_capture_tx_ops - structure of tx operation function
 * pointers for packet capture component
 * @pkt_capture_send_mode: send packet capture mode
 *
 */
struct wlan_pkt_capture_tx_ops {
	QDF_STATUS (*pkt_capture_send_mode)(struct wlan_objmgr_psoc *psoc,
					    uint8_t vdev_id,
					    enum pkt_capture_mode mode);
};

/**
 * struct wlan_pkt_capture_rx_ops - structure of rx operation function
 * pointers for packet capture component
 * @pkt_capture_register_ev_handlers: register mgmt offload event
 * @pkt_capture_unregister_ev_handlers: unregister mgmt offload event
 */
struct wlan_pkt_capture_rx_ops {
	QDF_STATUS (*pkt_capture_register_ev_handlers)
					(struct wlan_objmgr_psoc *psoc);
	QDF_STATUS (*pkt_capture_unregister_ev_handlers)
					(struct wlan_objmgr_psoc *psoc);
};

#endif /* _WLAN_PKT_CAPTURE_PUBLIC_STRUCTS_H_ */
