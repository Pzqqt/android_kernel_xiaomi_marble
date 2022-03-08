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
 * DOC : osif_twt_ext_req.h
 *
 * WLAN Host Device Driver file for TWT (Target Wake Time) support.
 *
 */
#ifndef _OSIF_TWT_EXT_REQ_H_
#define _OSIF_TWT_EXT_REQ_H_

#include <wlan_objmgr_psoc_obj.h>
#include <wlan_twt_public_structs.h>
#include <qca_vendor.h>
#include <wlan_objmgr_peer_obj.h>

#define TWT_WAKE_INTVL_MULTIPLICATION_FACTOR    1024
#define TWT_WAKE_DURATION_MULTIPLICATION_FACTOR 256

#if defined(WLAN_SUPPORT_TWT) && defined(WLAN_TWT_CONV_SUPPORTED)
/**
 * osif_twt_setup_req() - Process TWT setup operation
 * in the received vendor command and send it to firmware
 * @vdev: vdev
 * @twt_param_attr: nl attributes
 *
 * sets up TWT setup request from HDD. request is passed
 * to TWT core
 *
 * Handles QCA_WLAN_TWT_SETUP
 *
 * Return: success on 0, failure on non-zero
 */
int osif_twt_setup_req(struct wlan_objmgr_vdev *vdev,
		       struct nlattr *twt_param_attr);

/**
 * osif_twt_sta_teardown_req() - Process TWT sta teardown operation
 * in the received vendor command and send it to firmware
 * @vdev: vdev
 * @twt_param_attr: nl attributes
 *
 * sets up TWT teardown request from HDD. request is passed
 * to TWT core
 *
 * Handles QCA_WLAN_TWT_TEARDOWN
 *
 * Return: success on 0, failure on non-zero
 */
int osif_twt_sta_teardown_req(struct wlan_objmgr_vdev *vdev,
			      struct nlattr *twt_param_attr);

/**
 * osif_twt_sap_teardown_req() - Process TWT sap teardown operation
 * in the received vendor command and send it to firmware
 * @vdev: vdev
 * @twt_param_attr: nl attributes
 *
 * sets up TWT teardown request from HDD. request is passed
 * to TWT core
 *
 * Handles QCA_WLAN_TWT_TEARDOWN
 *
 * Return: success on 0, failure on non-zero
 */
int osif_twt_sap_teardown_req(struct wlan_objmgr_vdev *vdev,
			      struct nlattr *twt_param_attr);

/**
 * osif_twt_handle_renego_failure() - Handle twt renegotiation failure
 * @psoc: pointer to psoc
 * @add_dialog_event: event data
 *
 * Return: none
 */
void
osif_twt_handle_renego_failure(struct wlan_objmgr_psoc *psoc,
		       struct twt_add_dialog_complete_event *add_dialog_event);

/**
 * osif_twt_pause_req() - Process TWT pause operation
 * in the received vendor command and send it to firmware
 * @vdev: vdev
 * @twt_param_attr: nl attributes
 *
 * sets up TWT pause request from HDD. request is passed
 * to TWT core
 *
 * Handles QCA_WLAN_TWT_SUSPEND
 *
 * Return: success on 0, failure on non-zero
 */
int osif_twt_pause_req(struct wlan_objmgr_vdev *vdev,
		       struct nlattr *twt_param_attr);

/**
 * osif_twt_resume_req() - Process TWT resume operation
 * in the received vendor command and send it to firmware
 * @vdev: vdev
 * @twt_param_attr: nl attributes
 *
 * sets up TWT resume request from HDD. request is passed
 * to TWT core
 *
 * Handles QCA_WLAN_TWT_RESUME
 *
 * Return: success on 0, failure on non-zero
 */
int osif_twt_resume_req(struct wlan_objmgr_vdev *vdev,
		       struct nlattr *twt_param_attr);

/**
 * osif_twt_nudge_req() - Process TWT nudge operation
 * in the received vendor command and send it to firmware
 * @vdev: vdev
 * @twt_param_attr: nl attributes
 *
 * sets up TWT nudge request from HDD. request is passed
 * to TWT core
 *
 * Handles QCA_WLAN_TWT_NUDGE
 *
 * Return: success on 0, failure on non-zero
 */
int osif_twt_nudge_req(struct wlan_objmgr_vdev *vdev,
		       struct nlattr *twt_param_attr);

/**
 * osif_twt_get_capabilities() - Process TWT get capabilities
 * in the received vendor command.
 * @vdev: vdev
 *
 * Handles QCA_WLAN_TWT_GET_CAPABILITIES
 *
 * Return: 0 on success, negative value on failure
 */
int osif_twt_get_capabilities(struct wlan_objmgr_vdev *vdev);

/**
 * osif_twt_send_get_capabilities_response  - TWT pack and send response to
 * userspace for get capabilities command
 * @psoc: pointer to global psoc
 * @vdev: pointer to vdev
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
osif_twt_send_get_capabilities_response(struct wlan_objmgr_psoc *psoc,
					struct wlan_objmgr_vdev *vdev);

/**
 * osif_fill_peer_macaddr  - find peer from vdev and fill mac address
 * @vdev: vdev pointer
 * @mac_addr: output buffer to copy mac address
 *
 * This is the utility function, which finds peer bss info from the vdev
 * and fill the output buffer with mac address
 *
 * Return: errno
 */
int osif_fill_peer_macaddr(struct wlan_objmgr_vdev *vdev, uint8_t *mac_addr);

/**
 * osif_twt_get_session_req() - Extract get TWT NL attributes
 * @vdev: vdev pointer
 * @twt_param_attr: TWT NL attributes coming from the user space
 *
 * Return: errno
 */
int osif_twt_get_session_req(struct wlan_objmgr_vdev *vdev,
			     struct nlattr *twt_param_attr);

/**
 * osif_twt_get_session_traffic_stats() - Extract traffic stats NL attributes
 * @vdev: vdev pointer
 * @twt_param_attr: TWT NL attributes coming from the user space
 *
 * Return: errno
 */
int osif_twt_get_session_traffic_stats(struct wlan_objmgr_vdev *vdev,
				       struct nlattr *twt_param_attr);

/**
 * osif_twt_get_stats_response() - Post get stats response to user space
 * @vdev: vdev pointer
 * @params: cp stats event params
 * @num_session_stats: number of session stats
 *
 * Return: QDF_STATUS
 */
QDF_STATUS osif_twt_get_stats_response(struct wlan_objmgr_vdev *vdev,
				       struct twt_infra_cp_stats_event *params,
				       uint32_t num_session_stats);

int osif_twt_clear_session_traffic_stats(struct wlan_objmgr_vdev *vdev,
					 struct nlattr *twt_param_attr);

#else
static inline
int osif_twt_setup_req(struct wlan_objmgr_vdev *vdev,
		       struct nlattr *twt_param_attr)
{
	return 0;
}

static inline
int osif_twt_get_capabilities(struct wlan_objmgr_vdev *vdev)
{
	return 0;
}

static inline
int osif_twt_sta_teardown_req(struct wlan_objmgr_vdev *vdev,
			      struct nlattr *twt_param_attr)
{
	return 0;
}

static inline
int osif_twt_sap_teardown_req(struct wlan_objmgr_vdev *vdev,
			      struct nlattr *twt_param_attr)
{
	return 0;
}

static inline
int osif_twt_pause_req(struct wlan_objmgr_vdev *vdev,
		       struct nlattr *twt_param_attr)
{
	return 0;
}

static inline
int osif_twt_resume_req(struct wlan_objmgr_vdev *vdev,
		       struct nlattr *twt_param_attr)
{
	return 0;
}

static inline
int osif_twt_nudge_req(struct wlan_objmgr_vdev *vdev,
		       struct nlattr *twt_param_attr)
{
	return 0;
}

static inline
int osif_twt_get_session_req(struct wlan_objmgr_vdev *vdev,
			     struct nlattr *twt_param_attr)
{
	return 0;
}

static inline
int osif_twt_get_session_traffic_stats(struct wlan_objmgr_vdev *vdev,
				       struct nlattr *twt_param_attr)
{
	return 0;
}

static inline
int osif_twt_clear_session_traffic_stats(struct wlan_objmgr_vdev *vdev,
					 struct nlattr *twt_param_attr)
{
	return 0;
}

#endif
#endif /* _OSIF_TWT_EXT_REQ_H_ */

