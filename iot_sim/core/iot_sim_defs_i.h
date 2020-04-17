/*
 * Copyright (c) 2020, The Linux Foundation. All rights reserved.
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

#ifndef _IOT_SIM_DEFS_I_H_
#define _IOT_SIM_DEFS_I_H_

#include <wlan_objmgr_cmn.h>
#include <wlan_objmgr_global_obj.h>
#include <wlan_objmgr_psoc_obj.h>
#include <qdf_list.h>
#include <qdf_util.h>
#include <qdf_types.h>
#include <qdf_timer.h>
#include <wbuf.h>
#include <wlan_iot_sim_utils_api.h>

#define iot_sim_fatal(format, args...) \
	QDF_TRACE_FATAL(QDF_MODULE_ID_IOT_SIM, format, ## args)
#define iot_sim_err(format, args...) \
	QDF_TRACE_ERROR(QDF_MODULE_ID_IOT_SIM, format, ## args)
#define iot_sim_warn(format, args...) \
	QDF_TRACE_WARN(QDF_MODULE_ID_IOT_SIM, format, ## args)
#define iot_sim_info(format, args...) \
	QDF_TRACE_INFO(QDF_MODULE_ID_IOT_SIM, format, ## args)
#define iot_sim_debug(format, args...) \
	QDF_TRACE_DEBUG(QDF_MODULE_ID_IOT_SIM, format, ## args)

#define iot_sim_fatal_nofl(format, args...) \
	QDF_TRACE_FATAL_NO_FL(QDF_MODULE_ID_IOT_SIM, format, ## args)
#define iot_sim_err_nofl(format, args...) \
	QDF_TRACE_ERROR_NO_FL(QDF_MODULE_ID_IOT_SIM, format, ## args)
#define iot_sim_warn_nofl(format, args...) \
	QDF_TRACE_WARN_NO_FL(QDF_MODULE_ID_IOT_SIM, format, ## args)
#define iot_sim_info_nofl(format, args...) \
	QDF_TRACE_INFO_NO_FL(QDF_MODULE_ID_IOT_SIM, format, ## args)
#define iot_sim_debug_nofl(format, args...) \
	QDF_TRACE_DEBUG_NO_FL(QDF_MODULE_ID_IOT_SIM, format, ## args)

#define iot_sim_fatal_rl(format, args...) \
	QDF_TRACE_FATAL_RL(QDF_MODULE_ID_IOT_SIM, format, ## args)
#define iot_sim_err_rl(format, args...) \
	QDF_TRACE_ERROR_RL(QDF_MODULE_ID_IOT_SIM, format, ## args)
#define iot_sim_warn_rl(format, args...) \
	QDF_TRACE_WARN_RL(QDF_MODULE_ID_IOT_SIM, format, ## args)
#define iot_sim_info_rl(format, args...) \
	QDF_TRACE_INFO_RL(QDF_MODULE_ID_IOT_SIM, format, ## args)
#define iot_sim_debug_rl(format, args...) \
	QDF_TRACE_DEBUG_RL(QDF_MODULE_ID_IOT_SIM, format, ## args)

#define iot_sim_fatal_rl_nofl(format, args...) \
	QDF_TRACE_FATAL_RL_NO_FL(QDF_MODULE_ID_IOT_SIM, format, ## args)
#define iot_sim_err_rl_nofl(format, args...) \
	QDF_TRACE_ERROR_RL_NO_FL(QDF_MODULE_ID_IOT_SIM, format, ## args)
#define iot_sim_warn_rl_nofl(format, args...) \
	QDF_TRACE_WARN_RL_NO_FL(QDF_MODULE_ID_IOT_SIM, format, ## args)
#define iot_sim_info_rl_nofl(format, args...) \
	QDF_TRACE_INFO_RL_NO_FL(QDF_MODULE_ID_IOT_SIM, format, ## args)
#define iot_sim_debug_rl_nofl(format, args...) \
	QDF_TRACE_DEBUG_RL_NO_FL(QDF_MODULE_ID_IOT_SIM, format, ## args)

#define N_FRAME_TYPE 0x4
#define N_FRAME_SUBTYPE 0xF
#define MAX_SEQ 0x4

/* struct iot_sim_content - represent user content
 * @len - Length of the content provided by user
 * @offset - offset at which modification done in capture frame
 * @frm_content - actual user data in hex
 * @drop - frame marked for drop
 * @delay_dur - duration of delay
 */
struct iot_sim_content {
	uint16_t len;
	uint16_t offset;
	uint8_t *frm_content;
	bool drop;
	uint16_t delay_dur;
};

/* Per Peer iot_sim data per frame type/subtype */
struct iot_sim_fb_array_peer {
	struct qdf_mac_addr peer_addr;
	struct iot_sim_content iot_sim_fb_array[N_FRAME_TYPE][N_FRAME_SUBTYPE];
	qdf_list_t p_list;
};

/**
 * struct iot_sim_context - iot_sim psoc private object
 * @psoc_obj:Reference to psoc global object
 * @iot_sim_lock: spinlock for synchronization
 *
 * Call back functions
 * @iot_sim_opertion_handler: iot sim operations handler
 */

struct iot_sim_context {
	struct wlan_objmgr_psoc *psoc_obj;
	qdf_spinlock_t iot_sim_lock;
	/* IOT_SIM Peer list */
	struct iot_sim_fb_arrary_peer *iot_sim_peer_list;
	void *p_iot_sim_target_handle;
	QDF_STATUS (*iot_sim_operation_handler)(struct wlan_objmgr_psoc *psoc,
						wbuf_t wbuf);
};

#endif /* _IOT_SIM_DEFS_I_H_ */
