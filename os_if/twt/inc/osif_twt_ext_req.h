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
#else
static inline
int osif_twt_setup_req(struct wlan_objmgr_vdev *vdev,
		       struct nlattr *twt_param_attr)
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

#endif
#endif /* _OSIF_TWT_EXT_REQ_H_ */

