/*
 * Copyright (c) 2016-2017 The Linux Foundation. All rights reserved.
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

#include "qdf_mem.h"
#include "wlan_lmac_if_def.h"
#include "wlan_lmac_if_api.h"
#include "wlan_mgmt_txrx_tgt_api.h"
#include "wlan_scan_tgt_api.h"
#ifdef WLAN_ATF_ENABLE
#include "wlan_atf_tgt_api.h"
#endif
#ifdef WIFI_POS_CONVERGED
#include "target_if_wifi_pos.h"
#endif /* WIFI_POS_CONVERGED */
#ifdef WLAN_FEATURE_NAN_CONVERGENCE
#include "target_if_nan.h"
#endif /* WLAN_FEATURE_NAN_CONVERGENCE */

#ifdef WLAN_CONV_CRYPTO_SUPPORTED
#include "wlan_crypto_global_api.h"
#endif
/* Function pointer for OL/WMA specific UMAC tx_ops
 * registration.
 */
QDF_STATUS (*wlan_lmac_if_umac_tx_ops_register)
				(struct wlan_lmac_if_tx_ops *tx_ops);
EXPORT_SYMBOL(wlan_lmac_if_umac_tx_ops_register);

#ifdef WLAN_ATF_ENABLE
/**
 * wlan_lmac_if_atf_rx_ops_register() - Function to register ATF RX ops.
 */
static void
wlan_lmac_if_atf_rx_ops_register(struct wlan_lmac_if_rx_ops *rx_ops)
{
	struct wlan_lmac_if_atf_rx_ops *atf_rx_ops = &rx_ops->atf_rx_ops;

	/* ATF rx ops */
	atf_rx_ops->atf_get_atf_commit = tgt_atf_get_atf_commit;
	atf_rx_ops->atf_get_fmcap = tgt_atf_get_fmcap;
	atf_rx_ops->atf_get_obss_scale = tgt_atf_get_obss_scale;
	atf_rx_ops->atf_get_mode = tgt_atf_get_mode;
	atf_rx_ops->atf_get_msdu_desc = tgt_atf_get_msdu_desc;
	atf_rx_ops->atf_get_max_vdevs = tgt_atf_get_max_vdevs;
	atf_rx_ops->atf_get_peers = tgt_atf_get_peers;
	atf_rx_ops->atf_get_tput_based = tgt_atf_get_tput_based;
	atf_rx_ops->atf_get_logging = tgt_atf_get_logging;
	atf_rx_ops->atf_get_txbuf_share = tgt_atf_get_txbuf_share;
	atf_rx_ops->atf_get_txbuf_max = tgt_atf_get_txbuf_max;
	atf_rx_ops->atf_get_txbuf_min = tgt_atf_get_txbuf_min;
	atf_rx_ops->atf_get_ssidgroup = tgt_atf_get_ssidgroup;
	atf_rx_ops->atf_get_tx_block_count = tgt_atf_get_tx_block_count;
	atf_rx_ops->atf_get_peer_blk_txtraffic = tgt_atf_get_peer_blk_txtraffic;
	atf_rx_ops->atf_get_vdev_blk_txtraffic = tgt_atf_get_vdev_blk_txtraffic;
	atf_rx_ops->atf_get_sched = tgt_atf_get_sched;
	atf_rx_ops->atf_get_tx_tokens = tgt_atf_get_tx_tokens;
	atf_rx_ops->atf_get_shadow_tx_tokens = tgt_atf_get_shadow_tx_tokens;
	atf_rx_ops->atf_get_shadow_alloted_tx_tokens =
					tgt_atf_get_shadow_alloted_tx_tokens;
	atf_rx_ops->atf_get_txtokens_common = tgt_atf_get_txtokens_common;
	atf_rx_ops->atf_get_peer_stats = tgt_atf_get_peer_stats;
	atf_rx_ops->atf_get_token_allocated = tgt_atf_get_token_allocated;
	atf_rx_ops->atf_get_token_utilized = tgt_atf_get_token_utilized;

	atf_rx_ops->atf_set_sched = tgt_atf_set_sched;
	atf_rx_ops->atf_set_fmcap = tgt_atf_set_fmcap;
	atf_rx_ops->atf_set_obss_scale = tgt_atf_set_obss_scale;
	atf_rx_ops->atf_set_mode = tgt_atf_set_mode;
	atf_rx_ops->atf_set_msdu_desc = tgt_atf_set_msdu_desc;
	atf_rx_ops->atf_set_max_vdevs = tgt_atf_set_max_vdevs;
	atf_rx_ops->atf_set_peers = tgt_atf_set_peers;
	atf_rx_ops->atf_set_peer_stats = tgt_atf_set_peer_stats;
	atf_rx_ops->atf_set_vdev_blk_txtraffic = tgt_atf_set_vdev_blk_txtraffic;
	atf_rx_ops->atf_set_peer_blk_txtraffic = tgt_atf_set_peer_blk_txtraffic;
	atf_rx_ops->atf_set_tx_block_count = tgt_atf_set_tx_block_count;
	atf_rx_ops->atf_set_token_allocated = tgt_atf_set_token_allocated;
	atf_rx_ops->atf_set_token_utilized = tgt_atf_set_token_utilized;
}
#else
static void
wlan_lmac_if_atf_rx_ops_register(struct wlan_lmac_if_rx_ops *rx_ops)
{
}
#endif

#ifdef WLAN_CONV_CRYPTO_SUPPORTED
static void
wlan_lmac_if_crypto_rx_ops_register(struct wlan_lmac_if_rx_ops *rx_ops)
{
	wlan_crypto_register_crypto_rx_ops(&rx_ops->crypto_rx_ops);
}
#else
static void
wlan_lmac_if_crypto_rx_ops_register(struct wlan_lmac_if_rx_ops *rx_ops)
{
}
#endif

#ifdef WIFI_POS_CONVERGED
static void wlan_lmac_if_umac_rx_ops_register_wifi_pos(
				struct wlan_lmac_if_rx_ops *rx_ops)
{
	target_if_wifi_pos_register_rx_ops(rx_ops);
}
#else
static void wlan_lmac_if_umac_rx_ops_register_wifi_pos(
				struct wlan_lmac_if_rx_ops *rx_ops)
{
}
#endif /* WIFI_POS_CONVERGED */

#ifdef WLAN_FEATURE_NAN_CONVERGENCE
static void wlan_lmac_if_register_nan_rx_ops(struct wlan_lmac_if_rx_ops *rx_ops)
{
	target_if_nan_register_rx_ops(rx_ops);
}
#else
static void wlan_lmac_if_register_nan_rx_ops(struct wlan_lmac_if_rx_ops *rx_ops)
{
}
#endif /* WLAN_FEATURE_NAN_CONVERGENCE */

/**
 * wlan_lmac_if_umac_rx_ops_register() - UMAC rx handler register
 * @rx_ops: Pointer to rx_ops structure to be populated
 *
 * Register umac RX callabacks which will be called by DA/OL/WMA/WMI
 *
 * Return: QDF_STATUS_SUCCESS - in case of success
 */
QDF_STATUS
wlan_lmac_if_umac_rx_ops_register(struct wlan_lmac_if_rx_ops *rx_ops)
{
	/* Component specific public api's to be called to register
	 * respective callbacks
	 * Ex: rx_ops->fp = function;
	 */
	struct wlan_lmac_if_mgmt_txrx_rx_ops *mgmt_txrx_rx_ops;

	if (!rx_ops) {
		qdf_print("%s: lmac if rx ops pointer is NULL", __func__);
		return QDF_STATUS_E_INVAL;
	}

	/* mgmt txrx rx ops */
	mgmt_txrx_rx_ops = &rx_ops->mgmt_txrx_rx_ops;

	mgmt_txrx_rx_ops->mgmt_tx_completion_handler =
			tgt_mgmt_txrx_tx_completion_handler;
	mgmt_txrx_rx_ops->mgmt_rx_frame_handler =
			tgt_mgmt_txrx_rx_frame_handler;
	mgmt_txrx_rx_ops->mgmt_txrx_get_nbuf_from_desc_id =
			tgt_mgmt_txrx_get_nbuf_from_desc_id;
	mgmt_txrx_rx_ops->mgmt_txrx_get_peer_from_desc_id =
			tgt_mgmt_txrx_get_peer_from_desc_id;
	mgmt_txrx_rx_ops->mgmt_txrx_get_vdev_id_from_desc_id =
			tgt_mgmt_txrx_get_vdev_id_from_desc_id;
	/* scan rx ops */
	rx_ops->scan.scan_ev_handler = tgt_scan_event_handler;
	wlan_lmac_if_atf_rx_ops_register(rx_ops);

	wlan_lmac_if_crypto_rx_ops_register(rx_ops);
	/* wifi_pos rx ops */
	wlan_lmac_if_umac_rx_ops_register_wifi_pos(rx_ops);

	wlan_lmac_if_register_nan_rx_ops(rx_ops);

	return QDF_STATUS_SUCCESS;
}

/**
 * wlan_lmac_if_set_umac_txops_registration_cb() - tx registration
 * callback assignment
 * @dev_type: Dev type can be either Direct attach or Offload
 * @handler: handler to be called for LMAC tx ops registration
 *
 * API to assign appropriate tx registration callback handler based on the
 * device type(Offload or Direct attach)
 *
 * Return: QDF_STATUS_SUCCESS - in case of success
 */
QDF_STATUS wlan_lmac_if_set_umac_txops_registration_cb(QDF_STATUS (*handler)
				(struct wlan_lmac_if_tx_ops *))
{
	wlan_lmac_if_umac_tx_ops_register = handler;
	return QDF_STATUS_SUCCESS;
}
EXPORT_SYMBOL(wlan_lmac_if_set_umac_txops_registration_cb);

