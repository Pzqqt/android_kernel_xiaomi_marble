/*
 * Copyright (c) 2021, The Linux Foundation. All rights reserved.
 * Copyright (c) 2021-2022 Qualcomm Innovation Center, Inc. All rights reserved.
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
 * DOC: This file contains definitions for MLME TWT functionality.
 */

#ifndef _WLAN_MLME_TWT_PUBLIC_STRUCT_H_
#define _WLAN_MLME_TWT_PUBLIC_STRUCT_H_

#include <wlan_twt_public_structs.h>

#ifdef WLAN_SUPPORT_TWT
/**
 * struct twt_session_info  - TWT session related parameters
 * @dialog_id: TWT session dialog id
 * @state: TWT session state
 * @setup_done: TWT session setup is complete
 * @active_cmd: bitmap to indicate which command is
 * in progress. Bits are provided by enum wlan_twt_commands.
 */
struct twt_session_info {
	uint8_t dialog_id;
	uint8_t state;
	bool setup_done;
	enum wlan_twt_commands active_cmd;
};

/**
 * struct twt_context  - TWT context
 * @peer_capability: TWT peer capability bitmap. Refer enum
 * wlan_twt_capabilities for representation.
 * @num_twt_sessions: Maximum supported TWT sessions.
 * @session_info: TWT session related parameters for each session
 */
struct twt_context {
	uint8_t peer_capability;
	uint8_t num_twt_sessions;
	struct twt_session_info session_info[WLAN_MAX_TWT_SESSIONS_PER_PEER];
};
#endif /* WLAN_SUPPORT_TWT */
#endif /* _WLAN_MLME_TWT_PUBLIC_STRUCT_H_ */
