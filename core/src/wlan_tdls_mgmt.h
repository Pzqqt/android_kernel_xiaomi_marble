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
 * DOC: wlan_tdls_mgmt.h
 *
 * TDLS management frames include file
 */

#ifndef _WLAN_TDLS_MGMT_H_
#define _WLAN_TDLS_MGMT_H_

/* default tdls serialize timeout is set to 10 secs */
#define TDLS_DEFAULT_SERIALIZE_CMD_TIMEOUT  10000

/*
 * struct tdls_send_mgmt_request - tdls management request
 * @message_type: type of pe message
 * @length: length of the frame.
 * @session_id: session id
 * @transaction_id: transaction ID for cmd
 * @req_type: type of action frame
 * @dialog: dialog token used in the frame.
 * @status_code: status to be incuded in the frame.
 * @responder: tdls request type
 * @peer_capability: peer capability information
 * @bssid: bssid
 * @peer_mac: mac address of the peer
 * @add_ie: additional ie's to be included
 */
struct tdls_send_mgmt_request {
	uint16_t message_type;
	uint16_t length;
	uint8_t session_id;
	uint16_t transaction_id;
	uint8_t req_type;
	uint8_t dialog;
	uint16_t status_code;
	uint8_t responder;
	uint32_t peer_capability;
	struct qdf_mac_addr bssid;
	struct qdf_mac_addr peer_mac;
	/* Variable length. Dont add any field after this. */
	uint8_t add_ie[1];
};

/**
 * tdls_process_mgmt_req() - send a TDLS mgmt request to serialize module
 * @tdls_mgmt_req: tdls management request
 *
 * TDLS request API, called from cfg80211 to send a TDLS frame in
 * serialized manner to PE
 *
 *Return: QDF_STATUS
 */
QDF_STATUS tdls_process_mgmt_req(
			struct tdls_action_frame_request *tdls_mgmt_req);
#endif

