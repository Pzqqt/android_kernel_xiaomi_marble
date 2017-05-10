/*
 * Copyright (c) 2017 The Linux Foundation. All rights reserved.
 *
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
#include <ol_if_athvar.h>
#include <wlan_objmgr_cmn.h>
#include <wlan_objmgr_psoc_obj.h>
#include <wlan_objmgr_pdev_obj.h>
#include <wlan_objmgr_vdev_obj.h>
#include <wlan_objmgr_peer_obj.h>

void target_if_son_register_tx_ops(struct wlan_lmac_if_tx_ops *tx_ops);

bool son_ol_is_peer_inact(struct wlan_objmgr_peer *);

u_int32_t son_ol_get_peer_rate(struct wlan_objmgr_peer *peer, u_int8_t type);

int8_t son_ol_sanitize_util_invtl(struct wlan_objmgr_pdev *pdev,
				  u_int32_t *sample_period,
				  u_int32_t *num_of_sample);

bool son_ol_enable(struct wlan_objmgr_pdev *pdev, bool enable);

/* Function pointer to set overload status */

void son_ol_set_overload(struct wlan_objmgr_pdev *pdev, bool overload);

/* Function pointer to set band steering parameters */

bool son_ol_set_params(struct wlan_objmgr_pdev *pdev,
			      u_int32_t inactivity_check_period,
			      u_int32_t inactivity_threshold_normal,
			      u_int32_t inactivity_threshold_overload);

QDF_STATUS son_ol_send_null(struct wlan_objmgr_pdev *pdev,
			    u_int8_t *macaddr,
			    struct wlan_objmgr_vdev *vdev);

int son_ol_lmac_create(struct wlan_objmgr_pdev *pdev);


int son_ol_lmac_destroy(struct wlan_objmgr_pdev *pdev);


void  son_ol_rx_rssi_update(struct wlan_objmgr_pdev *pdev, u_int8_t *macaddres,
			    u_int8_t status, int8_t rssi, u_int8_t subtype);

void son_ol_rx_rate_update(struct wlan_objmgr_pdev *pdev, u_int8_t *macaddres,
			   u_int8_t status, u_int32_t rateKbps);
