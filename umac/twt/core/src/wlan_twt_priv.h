/*
 * Copyright (c) 2022 Qualcomm Innovation Center, Inc. All rights reserved.
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
 * DOC: Declare private data structures and APIs which shall be used
 * internally only in twt component.
 *
 * Note: This API should be never accessed out of twt component.
 */

#ifndef _WLAN_TWT_PRIV_H_
#define _WLAN_TWT_PRIV_H_

#include <wlan_twt_public_structs.h>
#include <wlan_twt_ext_defs.h>
#include <wlan_twt_ext_type.h>

/**
 * struct twt_tgt_caps -
 * @twt_requestor: twt requestor
 * @twt_responder: twt responder
 * @legacy_bcast_twt_support: legacy bcast twt support
 * @twt_bcast_req_support: bcast requestor support
 * @twt_bcast_res_support: bcast responder support
 * @twt_nudge_enabled: twt nudge enabled
 * @all_twt_enabled: all twt enabled
 * @twt_stats_enabled: twt stats enabled
 * @twt_ack_supported: twt ack supported
 */
struct twt_tgt_caps {
	bool twt_requestor;
	bool twt_responder;
	bool legacy_bcast_twt_support;
	bool twt_bcast_req_support;
	bool twt_bcast_res_support;
	bool twt_nudge_enabled;
	bool all_twt_enabled;
	bool twt_stats_enabled;
	bool twt_ack_supported;
};

/**
 * struct twt_psoc_priv_obj -
 * @cfg_params: cfg params
 * @twt_caps: twt caps
 * @enable_context: enable context
 * @disable_context: disable context
 */
struct twt_psoc_priv_obj {
	psoc_twt_ext_cfg_params_t cfg_params;
	struct twt_tgt_caps twt_caps;
	struct twt_en_dis_context enable_context;
	struct twt_en_dis_context disable_context;
};

/**
 * struct twt_vdev_priv_obj -
 * @twt_wait_for_notify: wait for notify
 */
struct twt_vdev_priv_obj {
	bool twt_wait_for_notify;
};

/**
 * struct twt_session -
 * @dialog_id: dialog id
 * @state: state
 * @setup_done: setup done
 * @active_cmd: active command
 * @twt_ack_ctx: twt ack context
 */
struct twt_session {
	uint8_t dialog_id;
	uint8_t state;
	bool setup_done;
	enum wlan_twt_commands active_cmd;
	void *twt_ack_ctx;
};

/**
 * struct twt_peer_priv_obj -
 * @twt_peer_lock: peer lock
 * @peer_capability: peer capability
 * @num_twt_sessions: number of twt sessions
 * @session_info: session info
 */
struct twt_peer_priv_obj {
	qdf_mutex_t twt_peer_lock;
	uint8_t peer_capability;
	uint8_t num_twt_sessions;
	struct twt_session session_info
		[WLAN_MAX_TWT_SESSIONS_PER_PEER];
};

#endif /* End  of _WLAN_TWT_PRIV_H_ */

