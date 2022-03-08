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
#include <wlan_twt_ucfg_ext_api.h>
#include <wlan_twt_ucfg_ext_cfg.h>
#include "twt/core/src/wlan_twt_cfg.h"
#include "twt/core/src/wlan_twt_main.h"

QDF_STATUS ucfg_twt_psoc_open(struct wlan_objmgr_psoc *psoc)
{
	return wlan_twt_cfg_init(psoc);
}

QDF_STATUS ucfg_twt_psoc_close(struct wlan_objmgr_psoc *psoc)
{
	return wlan_twt_cfg_deinit(psoc);
}

QDF_STATUS ucfg_twt_update_psoc_config(struct wlan_objmgr_psoc *psoc)
{
	return wlan_twt_cfg_update(psoc);
}

QDF_STATUS
ucfg_twt_cfg_get_requestor(struct wlan_objmgr_psoc *psoc, bool *val)
{
	return wlan_twt_cfg_get_requestor(psoc, val);
}

QDF_STATUS
ucfg_twt_cfg_get_responder(struct wlan_objmgr_psoc *psoc, bool *val)
{
	return wlan_twt_cfg_get_responder(psoc, val);
}

QDF_STATUS
ucfg_twt_setup_req(struct wlan_objmgr_psoc *psoc,
		   struct twt_add_dialog_param *params,
		   void *context)
{
	return wlan_twt_setup_req(psoc, params, context);
}

QDF_STATUS ucfg_twt_teardown_req(struct wlan_objmgr_psoc *psoc,
				 struct twt_del_dialog_param *params,
				 void *context)
{
	return wlan_twt_teardown_req(psoc, params, context);
}

QDF_STATUS
ucfg_twt_pause_req(struct wlan_objmgr_psoc *psoc,
		   struct twt_pause_dialog_cmd_param *params,
		   void *context)
{
	return wlan_twt_pause_req(psoc, params, context);
}

QDF_STATUS
ucfg_twt_resume_req(struct wlan_objmgr_psoc *psoc,
		    struct twt_resume_dialog_cmd_param *params,
		    void *context)
{
	return wlan_twt_resume_req(psoc, params, context);
}

QDF_STATUS
ucfg_twt_nudge_req(struct wlan_objmgr_psoc *psoc,
				   struct twt_nudge_dialog_cmd_param *params,
				   void *context)
{
	return wlan_twt_nudge_req(psoc, params, context);
}

bool ucfg_twt_is_max_sessions_reached(struct wlan_objmgr_psoc *psoc,
				      struct qdf_mac_addr *peer_mac,
				      uint8_t dialog_id)
{
	return wlan_twt_is_max_sessions_reached(psoc, peer_mac, dialog_id);
}

bool ucfg_twt_is_setup_in_progress(struct wlan_objmgr_psoc *psoc,
				   struct qdf_mac_addr *peer_mac,
				   uint8_t dialog_id)
{
	return wlan_twt_is_setup_in_progress(psoc, peer_mac, dialog_id);
}

QDF_STATUS
ucfg_twt_cfg_get_congestion_timeout(struct wlan_objmgr_psoc *psoc,
				    uint32_t *val)
{
	return wlan_twt_cfg_get_congestion_timeout(psoc, val);
}

QDF_STATUS
ucfg_twt_cfg_set_congestion_timeout(struct wlan_objmgr_psoc *psoc, uint32_t val)
{
	return wlan_twt_cfg_set_congestion_timeout(psoc, val);
}

QDF_STATUS
ucfg_twt_cfg_get_24ghz_enabled(struct wlan_objmgr_psoc *psoc, bool *val)
{
	return wlan_twt_cfg_get_24ghz_enabled(psoc, val);
}

QDF_STATUS
ucfg_twt_cfg_get_bcast_requestor(struct wlan_objmgr_psoc *psoc, bool *val)
{
	return wlan_twt_cfg_get_bcast_requestor(psoc, val);
}

QDF_STATUS
ucfg_twt_cfg_get_flex_sched(struct wlan_objmgr_psoc *psoc, bool *val)
{
	return wlan_twt_cfg_get_flex_sched(psoc, val);
}

QDF_STATUS
ucfg_twt_init_context(struct wlan_objmgr_psoc *psoc,
		      struct qdf_mac_addr *peer_mac,
		      uint8_t dialog_id)
{
	return wlan_twt_init_context(psoc, peer_mac, dialog_id);
}

QDF_STATUS
ucfg_twt_set_osif_cb(osif_twt_get_global_ops_cb osif_twt_ops)
{
	mlme_set_osif_twt_cb(osif_twt_ops);
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
ucfg_twt_set_command_in_progress(struct wlan_objmgr_psoc *psoc,
				 struct qdf_mac_addr *peer_mac,
				 uint8_t dialog_id,
				 enum wlan_twt_commands cmd)
{
	return wlan_twt_set_command_in_progress(psoc, peer_mac,
						dialog_id, cmd);
}

QDF_STATUS
ucfg_twt_reset_active_command(struct wlan_objmgr_psoc *psoc,
			      struct qdf_mac_addr *peer_mac,
			      uint8_t dialog_id)
{
	return wlan_twt_set_command_in_progress(psoc, peer_mac, dialog_id,
						WLAN_TWT_NONE);
}

QDF_STATUS ucfg_twt_update_beacon_template(void)
{
	return wlan_twt_update_beacon_template();
}

bool
ucfg_twt_is_setup_done(struct wlan_objmgr_psoc *psoc,
		       struct qdf_mac_addr *peer_mac, uint8_t dialog_id)
{
	return wlan_twt_is_setup_done(psoc, peer_mac, dialog_id);
}

enum wlan_twt_session_state
ucfg_twt_get_session_state(struct wlan_objmgr_psoc *psoc,
			   struct qdf_mac_addr *peer_mac, uint8_t dialog_id)
{
	return wlan_twt_get_session_state(psoc, peer_mac, dialog_id);
}

bool ucfg_twt_is_command_in_progress(struct wlan_objmgr_psoc *psoc,
				     struct qdf_mac_addr *peer_mac,
				     uint8_t dialog_id,
				     enum wlan_twt_commands cmd,
				     enum wlan_twt_commands *pactive_cmd)
{
	return wlan_twt_is_command_in_progress(psoc, peer_mac, dialog_id, cmd,
					       pactive_cmd);
}
