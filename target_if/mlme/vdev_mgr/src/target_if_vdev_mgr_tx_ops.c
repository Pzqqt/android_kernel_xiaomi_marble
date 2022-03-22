/*
 * Copyright (c) 2019-2021 The Linux Foundation. All rights reserved.
 * Copyright (c) 2021-2022 Qualcomm Innovation Center, Inc. All rights reserved.
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
 * DOC: target_if_vdev_mgr_tx_ops.c
 *
 * This file provide definition for APIs registered through lmac Tx Ops
 */
#include <wlan_objmgr_pdev_obj.h>
#include <wlan_objmgr_vdev_obj.h>
#include <wmi_unified_api.h>
#include <wmi_unified_param.h>
#include <init_deinit_lmac.h>
#include <target_if_vdev_mgr_tx_ops.h>
#include <target_if_vdev_mgr_rx_ops.h>
#include <target_if.h>
#include <target_type.h>
#include <wlan_mlme_dbg.h>
#include <wlan_vdev_mgr_tgt_if_tx_defs.h>
#include <wlan_vdev_mgr_utils_api.h>
#include <wlan_cmn.h>
#include <wmi_unified_vdev_api.h>
#include <cdp_txrx_ctrl.h>
#include <target_if_psoc_timer_tx_ops.h>
#include <target_if_psoc_wake_lock.h>

static QDF_STATUS target_if_vdev_mgr_register_event_handler(
					struct wlan_objmgr_psoc *psoc)
{
	return target_if_vdev_mgr_wmi_event_register(psoc);
}

static QDF_STATUS target_if_vdev_mgr_unregister_event_handler(
					struct wlan_objmgr_psoc *psoc)
{
	return target_if_vdev_mgr_wmi_event_unregister(psoc);
}

QDF_STATUS
target_if_vdev_mgr_rsp_timer_stop(struct wlan_objmgr_psoc *psoc,
				  struct vdev_response_timer *vdev_rsp,
				  enum wlan_vdev_mgr_tgt_if_rsp_bit clear_bit)
{
	struct wlan_lmac_if_mlme_tx_ops *txops;

	txops = target_if_vdev_mgr_get_tx_ops(psoc);
	if (!txops || !txops->psoc_vdev_rsp_timer_deinit) {
		mlme_err("Failed to get mlme txrx_ops VDEV_%d PSOC_%d",
			 vdev_rsp->vdev_id, wlan_psoc_get_id(psoc));
		return QDF_STATUS_E_FAILURE;
	}

	if (qdf_atomic_test_and_clear_bit(clear_bit, &vdev_rsp->rsp_status)) {
		/*
		 * This is triggered from timer expiry case only for
		 * which timer stop is not required
		 */
		if (vdev_rsp->timer_status == QDF_STATUS_E_TIMEOUT) {
			if (clear_bit == DELETE_RESPONSE_BIT) {
				qdf_atomic_set(&vdev_rsp->rsp_timer_inuse, 0);
				vdev_rsp->psoc = NULL;
			}
		} else {
			if (clear_bit == DELETE_RESPONSE_BIT) {
				txops->psoc_vdev_rsp_timer_deinit(psoc,
								  vdev_rsp->vdev_id);
			} else {
				qdf_timer_stop(&vdev_rsp->rsp_timer);
			}
		}

		/*
		 * Reset the timer_status to clear any error state. As this
		 * variable is persistent, any leftover error status can cause
		 * undesirable effects.
		 */
		vdev_rsp->timer_status = QDF_STATUS_SUCCESS;
		/*
		 * Releasing reference taken at the time of
		 * starting response timer
		 */
		wlan_objmgr_psoc_release_ref(psoc, WLAN_PSOC_TARGET_IF_ID);
		return QDF_STATUS_SUCCESS;
	}
	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS
target_if_vdev_mgr_rsp_timer_start(struct wlan_objmgr_psoc *psoc,
				   struct vdev_response_timer *vdev_rsp,
				   enum wlan_vdev_mgr_tgt_if_rsp_bit set_bit)
{
	uint8_t rsp_pos;
	uint8_t vdev_id;

	/* it is expected to be only one command with FW at a time */
	for (rsp_pos = START_RESPONSE_BIT; rsp_pos <= RESPONSE_BIT_MAX;
	     rsp_pos++) {
		if (rsp_pos != set_bit) {
			if (qdf_atomic_test_bit(rsp_pos,
						&vdev_rsp->rsp_status)) {
				vdev_id = vdev_rsp->vdev_id;
				mlme_err("PSOC_%d VDEV_%d: %s requested, waiting for %s response",
					 wlan_psoc_get_id(psoc),
					 vdev_id,
					 string_from_rsp_bit(set_bit),
					 string_from_rsp_bit(rsp_pos));
				target_if_vdev_mgr_assert_mgmt(psoc,
							       vdev_id);
				target_if_vdev_mgr_rsp_timer_stop(psoc,
								  vdev_rsp,
								  rsp_pos);
			}
		}
	}

	if (qdf_atomic_test_and_set_bit(set_bit, &vdev_rsp->rsp_status)) {
		mlme_err("PSOC_%d VDEV_%d: %s requested, waiting for %s response",
			 wlan_psoc_get_id(psoc),
			 vdev_rsp->vdev_id, string_from_rsp_bit(set_bit),
			 string_from_rsp_bit(set_bit));
		target_if_vdev_mgr_assert_mgmt(psoc, vdev_rsp->vdev_id);
		target_if_vdev_mgr_rsp_timer_stop(psoc, vdev_rsp, set_bit);

		qdf_atomic_set_bit(set_bit, &vdev_rsp->rsp_status);
	}

	/* reference taken for timer start, will be released with stop */
	wlan_objmgr_psoc_get_ref(psoc, WLAN_PSOC_TARGET_IF_ID);
	qdf_timer_start(&vdev_rsp->rsp_timer, vdev_rsp->expire_time);

	return QDF_STATUS_SUCCESS;
}


struct wmi_unified
*target_if_vdev_mgr_wmi_handle_get(struct wlan_objmgr_vdev *vdev)
{
	struct wlan_objmgr_pdev *pdev;
	struct wmi_unified *wmi_handle;

	pdev = wlan_vdev_get_pdev(vdev);
	if (!pdev) {
		mlme_err("PDEV is NULL");
		return NULL;
	}

	wmi_handle = get_wmi_unified_hdl_from_pdev(pdev);
	if (!wmi_handle) {
		mlme_err("wmi_handle is null");
		return NULL;
	}

	return wmi_handle;
}

static inline uint32_t
target_if_vdev_mlme_build_txbf_caps(struct wlan_objmgr_vdev *vdev)
{
	uint32_t txbf_cap;
	uint32_t subfer;
	uint32_t mubfer;
	uint32_t subfee;
	uint32_t mubfee;
	uint32_t implicit_bf;
	uint32_t sounding_dimension;
	uint32_t bfee_sts_cap;

	txbf_cap = 0;
	/*
	 * ensure to set these after mlme component is attached to objmgr
	 */
	wlan_util_vdev_get_param(vdev, WLAN_MLME_CFG_SUBFEE, &subfee);
	wlan_util_vdev_get_param(vdev, WLAN_MLME_CFG_MUBFEE, &mubfee);
	wlan_util_vdev_get_param(vdev, WLAN_MLME_CFG_SUBFER, &subfer);
	wlan_util_vdev_get_param(vdev, WLAN_MLME_CFG_MUBFER, &mubfer);
	wlan_util_vdev_get_param(vdev, WLAN_MLME_CFG_BFEE_STS_CAP,
			&bfee_sts_cap);
	wlan_util_vdev_get_param(vdev, WLAN_MLME_CFG_IMLICIT_BF,
			&implicit_bf);
	wlan_util_vdev_get_param(vdev, WLAN_MLME_CFG_SOUNDING_DIM,
			&sounding_dimension);

	WMI_HOST_TXBF_CONF_SU_TX_BFEE_SET(txbf_cap, subfee);
	WMI_HOST_TXBF_CONF_MU_TX_BFEE_SET(txbf_cap, mubfee);
	WMI_HOST_TXBF_CONF_SU_TX_BFER_SET(txbf_cap, subfer);
	WMI_HOST_TXBF_CONF_MU_TX_BFER_SET(txbf_cap, mubfer);
	WMI_HOST_TXBF_CONF_STS_CAP_SET(txbf_cap, bfee_sts_cap);
	WMI_HOST_TXBF_CONF_IMPLICIT_BF_SET(txbf_cap, implicit_bf);
	WMI_HOST_TXBF_CONF_BF_SND_DIM_SET(txbf_cap, sounding_dimension);

	mlme_debug("VHT su bfee:%d mu bfee:%d su bfer:%d "
		   "mu bfer:%d impl bf:%d sounding dim:%d",
		   WMI_HOST_TXBF_CONF_SU_TX_BFEE_GET(txbf_cap),
		   WMI_HOST_TXBF_CONF_MU_TX_BFEE_GET(txbf_cap),
		   WMI_HOST_TXBF_CONF_SU_TX_BFER_GET(txbf_cap),
		   WMI_HOST_TXBF_CONF_MU_TX_BFER_GET(txbf_cap),
		   WMI_HOST_TXBF_CONF_IMPLICIT_BF_GET(txbf_cap),
		   WMI_HOST_TXBF_CONF_BF_SND_DIM_GET(txbf_cap));

	return txbf_cap;
}

static inline uint32_t
target_if_vdev_mlme_id_2_wmi(uint32_t cfg_id)
{
	int wmi_id;

	switch (cfg_id) {
	case WLAN_MLME_CFG_DTIM_PERIOD:
		wmi_id = wmi_vdev_param_dtim_period;
		break;
	case WLAN_MLME_CFG_SLOT_TIME:
		wmi_id = wmi_vdev_param_slot_time;
		break;
	case WLAN_MLME_CFG_PROTECTION_MODE:
		wmi_id = wmi_vdev_param_protection_mode;
		break;
	case WLAN_MLME_CFG_BEACON_INTERVAL:
		wmi_id = wmi_vdev_param_beacon_interval;
		break;
	case WLAN_MLME_CFG_LDPC:
		wmi_id = wmi_vdev_param_ldpc;
		break;
	case WLAN_MLME_CFG_NSS:
		wmi_id = wmi_vdev_param_nss;
		break;
	case WLAN_MLME_CFG_SUBFER:
	case WLAN_MLME_CFG_MUBFER:
	case WLAN_MLME_CFG_SUBFEE:
	case WLAN_MLME_CFG_MUBFEE:
	case WLAN_MLME_CFG_IMLICIT_BF:
	case WLAN_MLME_CFG_SOUNDING_DIM:
	case WLAN_MLME_CFG_TXBF_CAPS:
		wmi_id = wmi_vdev_param_txbf;
		break;
	case WLAN_MLME_CFG_HE_OPS:
		wmi_id = wmi_vdev_param_set_heop;
		break;
#ifdef WLAN_FEATURE_11BE
	case WLAN_MLME_CFG_EHT_OPS:
		wmi_id = wmi_vdev_param_set_ehtop;
		break;
#endif
	case WLAN_MLME_CFG_RTS_THRESHOLD:
		wmi_id = wmi_vdev_param_rts_threshold;
		break;
	case WLAN_MLME_CFG_FRAG_THRESHOLD:
		wmi_id = wmi_vdev_param_fragmentation_threshold;
		break;
	case WLAN_MLME_CFG_DROP_UNENCRY:
		wmi_id = wmi_vdev_param_drop_unencry;
		break;
	case WLAN_MLME_CFG_TX_POWER:
		wmi_id = wmi_vdev_param_tx_power;
		break;
	case WLAN_MLME_CFG_AMPDU:
		wmi_id = wmi_vdev_param_ampdu_subframe_size_per_ac;
		break;
	case WLAN_MLME_CFG_AMSDU:
		wmi_id = wmi_vdev_param_amsdu_subframe_size_per_ac;
		break;
	case WLAN_MLME_CFG_MIN_IDLE_INACTIVE_TIME:
		wmi_id =
			wmi_vdev_param_ap_keepalive_min_idle_inactive_time_secs;
		break;
	case WLAN_MLME_CFG_MAX_IDLE_INACTIVE_TIME:
		wmi_id =
			wmi_vdev_param_ap_keepalive_max_idle_inactive_time_secs;
		break;
	case WLAN_MLME_CFG_MAX_UNRESPONSIVE_INACTIVE_TIME:
		wmi_id =
			wmi_vdev_param_ap_keepalive_max_unresponsive_time_secs;
		break;
	case WLAN_MLME_CFG_UAPSD:
		wmi_id = WMI_HOST_STA_PS_PARAM_UAPSD;
		break;
	case WLAN_MLME_CFG_BCN_TX_RATE_CODE:
		wmi_id = wmi_vdev_param_beacon_rate;
		break;
	case WLAN_MLME_CFG_TX_MGMT_RATE_CODE:
		wmi_id = wmi_vdev_param_mgmt_rate;
		break;
	case WLAN_MLME_CFG_LISTEN_INTERVAL:
		wmi_id = wmi_vdev_param_listen_interval;
		break;
	case WLAN_MLME_CFG_ENABLE_MULTI_GROUP_KEY:
		wmi_id = wmi_vdev_param_enable_multi_group_key;
		break;
	case WLAN_MLME_CFG_MAX_GROUP_KEYS:
		wmi_id = wmi_vdev_param_max_group_keys;
		break;
	case WLAN_MLME_CFG_TX_ENCAP_TYPE:
		wmi_id = wmi_vdev_param_tx_encap_type;
		break;
	case WLAN_MLME_CFG_RX_DECAP_TYPE:
		wmi_id = wmi_vdev_param_rx_decap_type;
		break;
	case WLAN_MLME_CFG_ENABLE_DISABLE_RTT_RESPONDER_ROLE:
		wmi_id = wmi_vdev_param_enable_disable_rtt_responder_role;
		break;
	case WLAN_MLME_CFG_ENABLE_DISABLE_RTT_INITIATOR_ROLE:
		wmi_id = wmi_vdev_param_enable_disable_rtt_initiator_role;
		break;
	default:
		wmi_id = cfg_id;
		break;
	}

	return wmi_id;
}

static
QDF_STATUS target_if_vdev_set_tx_rx_decap_type(struct wlan_objmgr_vdev *vdev,
					       enum wlan_mlme_cfg_id param_id,
					       uint32_t value)
{
	ol_txrx_soc_handle soc_txrx_handle;
	struct wlan_objmgr_psoc *psoc;
	uint8_t vdev_id = wlan_vdev_get_id(vdev);
	cdp_config_param_type val = {0};

	psoc = wlan_vdev_get_psoc(vdev);
	soc_txrx_handle = wlan_psoc_get_dp_handle(psoc);

	if (!soc_txrx_handle)
		return QDF_STATUS_E_INVAL;

	if (param_id ==  WLAN_MLME_CFG_TX_ENCAP_TYPE) {
		val.cdp_vdev_param_tx_encap = value;
		return cdp_txrx_set_vdev_param(soc_txrx_handle,
					       vdev_id, CDP_TX_ENCAP_TYPE,
					       val);
	} else if (param_id == WLAN_MLME_CFG_RX_DECAP_TYPE) {
		val.cdp_vdev_param_rx_decap = value;
		return cdp_txrx_set_vdev_param(soc_txrx_handle,
					       vdev_id, CDP_RX_DECAP_TYPE,
					       val);
	}

	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS target_if_vdev_mgr_set_param_send(
					struct wlan_objmgr_vdev *vdev,
					struct vdev_set_params *param)
{
	QDF_STATUS status;
	struct wmi_unified *wmi_handle;
	int param_id;

	if (!vdev || !param) {
		mlme_err("Invalid input");
		return QDF_STATUS_E_INVAL;
	}

	wmi_handle = target_if_vdev_mgr_wmi_handle_get(vdev);
	if (!wmi_handle) {
		mlme_err("Failed to get WMI handle!");
		return QDF_STATUS_E_INVAL;
	}

	param_id = target_if_vdev_mlme_id_2_wmi(param->param_id);
	param->param_id = param_id;
	if (param->param_id == wmi_vdev_param_txbf)
		param->param_value = target_if_vdev_mlme_build_txbf_caps(vdev);
	status = wmi_unified_vdev_set_param_send(wmi_handle, param);

	return status;
}

static QDF_STATUS target_if_vdev_mgr_create_send(
					struct wlan_objmgr_vdev *vdev,
					struct vdev_create_params *param)
{
	QDF_STATUS status;
	struct wmi_unified *wmi_handle;
	uint8_t vap_addr[QDF_MAC_ADDR_SIZE] = {0};
	struct wlan_lmac_if_mlme_tx_ops *txops;
	struct wlan_objmgr_psoc *psoc;
	uint8_t vdev_id;

	if (!vdev || !param) {
		mlme_err("Invalid input");
		return QDF_STATUS_E_INVAL;
	}

	psoc = wlan_vdev_get_psoc(vdev);
	if (!psoc) {
		mlme_err("Failed to get psoc for VDEV_%d",
			 wlan_vdev_get_id(vdev));
		return QDF_STATUS_E_INVAL;
	}

	txops = wlan_mlme_get_lmac_tx_ops(psoc);
	if (!txops || !txops->psoc_vdev_rsp_timer_init) {
		mlme_err("Failed to get mlme txrx_ops for VDEV_%d PSOC_%d",
			 wlan_vdev_get_id(vdev), wlan_psoc_get_id(psoc));
		return QDF_STATUS_E_INVAL;
	}

	wmi_handle = target_if_vdev_mgr_wmi_handle_get(vdev);
	if (!wmi_handle) {
		mlme_err("Failed to get WMI handle!");
		return QDF_STATUS_E_INVAL;
	}

	WLAN_ADDR_COPY(vap_addr, wlan_vdev_mlme_get_macaddr(vdev));
	status = wmi_unified_vdev_create_send(wmi_handle, vap_addr,
					      param);

	vdev_id = wlan_vdev_get_id(vdev);
	if (QDF_IS_STATUS_SUCCESS(status))
		status = txops->psoc_vdev_rsp_timer_init(psoc, vdev_id);

	return status;
}

static QDF_STATUS target_if_vdev_mgr_start_send(
					struct wlan_objmgr_vdev *vdev,
					struct vdev_start_params *param)
{
	QDF_STATUS status;
	struct wmi_unified *wmi_handle;
	struct wlan_objmgr_psoc *psoc;
	struct wlan_lmac_if_mlme_rx_ops *rx_ops;
	uint8_t vdev_id;
	struct vdev_response_timer *vdev_rsp;

	if (!vdev || !param) {
		mlme_err("Invalid input");
		return QDF_STATUS_E_INVAL;
	}

	wmi_handle = target_if_vdev_mgr_wmi_handle_get(vdev);
	if (!wmi_handle) {
		mlme_err("Failed to get WMI handle!");
		return QDF_STATUS_E_INVAL;
	}

	vdev_id = wlan_vdev_get_id(vdev);
	psoc = wlan_vdev_get_psoc(vdev);
	rx_ops = target_if_vdev_mgr_get_rx_ops(psoc);
	if (!rx_ops || !rx_ops->psoc_get_vdev_response_timer_info) {
		mlme_err("VEV_%d: PSOC_%d No Rx Ops", vdev_id,
			 wlan_psoc_get_id(psoc));
		return QDF_STATUS_E_INVAL;
	}

	vdev_rsp = rx_ops->psoc_get_vdev_response_timer_info(psoc, vdev_id);
	if (!vdev_rsp) {
		mlme_err("VDEV_%d: PSOC_%d No vdev rsp timer", vdev_id,
			 wlan_psoc_get_id(psoc));
		return QDF_STATUS_E_INVAL;
	}

	vdev_rsp->expire_time = START_RESPONSE_TIMER;
	target_if_wake_lock_timeout_acquire(psoc, START_WAKELOCK);

	if (param->is_restart)
		target_if_vdev_mgr_rsp_timer_start(psoc, vdev_rsp,
						   RESTART_RESPONSE_BIT);
	else
		target_if_vdev_mgr_rsp_timer_start(psoc, vdev_rsp,
						   START_RESPONSE_BIT);

	status = wmi_unified_vdev_start_send(wmi_handle, param);
	if (QDF_IS_STATUS_ERROR(status)) {
		vdev_rsp->timer_status = QDF_STATUS_E_CANCELED;
		vdev_rsp->expire_time = 0;
		target_if_wake_lock_timeout_release(psoc, START_WAKELOCK);
		if (param->is_restart)
			target_if_vdev_mgr_rsp_timer_stop(psoc, vdev_rsp,
							  RESTART_RESPONSE_BIT);
		else
			target_if_vdev_mgr_rsp_timer_stop(psoc, vdev_rsp,
							  START_RESPONSE_BIT);
	} else {
		target_if_vdev_start_link_handler(vdev,
						  param->is_restart);
	}
	return status;
}

static QDF_STATUS target_if_vdev_mgr_delete_response_send(
				struct wlan_objmgr_vdev *vdev,
				struct wlan_lmac_if_mlme_rx_ops *rx_ops)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	struct wlan_objmgr_psoc *psoc = wlan_vdev_get_psoc(vdev);
	struct vdev_delete_response rsp = {0};

	rsp.vdev_id = wlan_vdev_get_id(vdev);
	status = rx_ops->vdev_mgr_delete_response(psoc, &rsp);
	target_if_wake_lock_timeout_release(psoc, DELETE_WAKELOCK);

	return status;
}

static QDF_STATUS target_if_vdev_mgr_delete_send(
					struct wlan_objmgr_vdev *vdev,
					struct vdev_delete_params *param)
{
	QDF_STATUS status;
	struct wlan_objmgr_psoc *psoc;
	struct wmi_unified *wmi_handle;
	struct wlan_lmac_if_mlme_rx_ops *rx_ops;
	uint8_t vdev_id;
	struct vdev_response_timer *vdev_rsp;

	if (!vdev || !param) {
		mlme_err("Invalid input");
		return QDF_STATUS_E_INVAL;
	}

	wmi_handle = target_if_vdev_mgr_wmi_handle_get(vdev);
	if (!wmi_handle) {
		mlme_err("Failed to get WMI handle!");
		return QDF_STATUS_E_INVAL;
	}

	vdev_id = wlan_vdev_get_id(vdev);
	psoc = wlan_vdev_get_psoc(vdev);
	rx_ops = target_if_vdev_mgr_get_rx_ops(psoc);
	if (!rx_ops || !rx_ops->psoc_get_vdev_response_timer_info) {
		mlme_err("VDEV_%d PSOC_%d No Rx Ops", vdev_id,
			 wlan_psoc_get_id(psoc));
		return QDF_STATUS_E_INVAL;
	}

	vdev_rsp = rx_ops->psoc_get_vdev_response_timer_info(psoc, vdev_id);
	if (!vdev_rsp) {
		mlme_err("VDEV_%d: PSOC_%d No vdev rsp timer", vdev_id,
			 wlan_psoc_get_id(psoc));
		return QDF_STATUS_E_INVAL;
	}

	vdev_rsp->expire_time = DELETE_RESPONSE_TIMER;
	target_if_vdev_mgr_rsp_timer_start(psoc, vdev_rsp,
					   DELETE_RESPONSE_BIT);
	target_if_wake_lock_timeout_acquire(psoc, DELETE_WAKELOCK);

	status = wmi_unified_vdev_delete_send(wmi_handle, param->vdev_id);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		/*
		 * Simulate delete response if target doesn't support
		 */
		if (!wmi_service_enabled(wmi_handle,
					 wmi_service_sync_delete_cmds) ||
		    wlan_psoc_nif_feat_cap_get(psoc,
					       WLAN_SOC_F_TESTMODE_ENABLE)) {
			target_if_vdev_mgr_rsp_timer_stop(psoc, vdev_rsp,
							  DELETE_RESPONSE_BIT);
			target_if_vdev_mgr_delete_response_send(vdev, rx_ops);
		}
	} else {
		vdev_rsp->expire_time = 0;
		vdev_rsp->timer_status = QDF_STATUS_E_CANCELED;
		target_if_vdev_mgr_rsp_timer_stop(psoc, vdev_rsp,
						  DELETE_RESPONSE_BIT);
		target_if_wake_lock_timeout_release(psoc, DELETE_WAKELOCK);
	}
	return status;
}

static QDF_STATUS target_if_vdev_mgr_stop_send(
					struct wlan_objmgr_vdev *vdev,
					struct vdev_stop_params *param)
{
	QDF_STATUS status;
	struct wmi_unified *wmi_handle;
	struct wlan_lmac_if_mlme_rx_ops *rx_ops;
	struct wlan_objmgr_psoc *psoc;
	uint8_t vdev_id;
	struct vdev_response_timer *vdev_rsp;


	if (!vdev || !param) {
		mlme_err("Invalid input");
		return QDF_STATUS_E_INVAL;
	}

	wmi_handle = target_if_vdev_mgr_wmi_handle_get(vdev);
	if (!wmi_handle) {
		mlme_err("Failed to get WMI handle!");
		return QDF_STATUS_E_INVAL;
	}

	vdev_id = wlan_vdev_get_id(vdev);
	psoc = wlan_vdev_get_psoc(vdev);
	rx_ops = target_if_vdev_mgr_get_rx_ops(psoc);
	if (!rx_ops || !rx_ops->psoc_get_vdev_response_timer_info) {
		mlme_err("VDEV_%d PSOC_%d No Rx Ops", vdev_id,
			 wlan_psoc_get_id(psoc));
		return QDF_STATUS_E_INVAL;
	}

	vdev_rsp = rx_ops->psoc_get_vdev_response_timer_info(psoc, vdev_id);
	if (!vdev_rsp) {
		mlme_err("VDEV_%d: PSOC_%d No vdev rsp timer", vdev_id,
			 wlan_psoc_get_id(psoc));
		return QDF_STATUS_E_INVAL;
	}

	vdev_rsp->expire_time = STOP_RESPONSE_TIMER;
	target_if_vdev_mgr_rsp_timer_start(psoc, vdev_rsp, STOP_RESPONSE_BIT);
	/*
	 * START wakelock is acquired before sending the start command and
	 * released after sending up command to fw. This is to prevent the
	 * system to go into suspend state during the connection.
	 * In auth/assoc failure scenario UP command is not sent
	 * so release the START wakelock here.
	 */
	target_if_wake_lock_timeout_release(psoc, START_WAKELOCK);
	target_if_wake_lock_timeout_acquire(psoc, STOP_WAKELOCK);

	status = wmi_unified_vdev_stop_send(wmi_handle, param->vdev_id);
	if (QDF_IS_STATUS_ERROR(status)) {
		vdev_rsp->expire_time = 0;
		vdev_rsp->timer_status = QDF_STATUS_E_CANCELED;
		target_if_vdev_mgr_rsp_timer_stop(psoc, vdev_rsp,
						  STOP_RESPONSE_BIT);
		target_if_wake_lock_timeout_release(psoc, STOP_WAKELOCK);
	} else {
		target_if_vdev_stop_link_handler(vdev);
	}
	return status;
}

static QDF_STATUS target_if_vdev_mgr_down_send(
					struct wlan_objmgr_vdev *vdev,
					struct vdev_down_params *param)
{
	QDF_STATUS status;
	struct wmi_unified *wmi_handle;
	struct wlan_objmgr_psoc *psoc;

	if (!vdev || !param) {
		mlme_err("Invalid input");
		return QDF_STATUS_E_INVAL;
	}

	wmi_handle = target_if_vdev_mgr_wmi_handle_get(vdev);
	if (!wmi_handle) {
		mlme_err("Failed to get WMI handle!");
		return QDF_STATUS_E_INVAL;
	}

	psoc = wlan_vdev_get_psoc(vdev);
	if (!psoc) {
		mlme_err("Failed to get PSOC Object");
		return QDF_STATUS_E_INVAL;
	}

	status = wmi_unified_vdev_down_send(wmi_handle, param->vdev_id);
	target_if_wake_lock_timeout_release(psoc, STOP_WAKELOCK);

	return status;
}

static QDF_STATUS target_if_vdev_mgr_up_send(
					struct wlan_objmgr_vdev *vdev,
					struct vdev_up_params *param)
{
	QDF_STATUS status;
	struct wmi_unified *wmi_handle;
	uint8_t bssid[QDF_MAC_ADDR_SIZE];
	struct wlan_objmgr_psoc *psoc;

	if (!vdev || !param) {
		mlme_err("Invalid input");
		return QDF_STATUS_E_INVAL;
	}

	wmi_handle = target_if_vdev_mgr_wmi_handle_get(vdev);
	if (!wmi_handle) {
		mlme_err("Failed to get WMI handle!");
		return QDF_STATUS_E_INVAL;
	}

	psoc = wlan_vdev_get_psoc(vdev);
	if (!psoc) {
		mlme_err("Failed to get PSOC Object");
		return QDF_STATUS_E_INVAL;
	}
	ucfg_wlan_vdev_mgr_get_param_bssid(vdev, bssid);

	status = wmi_unified_vdev_up_send(wmi_handle, bssid, param);
	target_if_wake_lock_timeout_release(psoc, START_WAKELOCK);

	return status;
}

static QDF_STATUS target_if_vdev_mgr_beacon_tmpl_send(
					struct wlan_objmgr_vdev *vdev,
					struct beacon_tmpl_params *param)
{
	QDF_STATUS status;
	struct wmi_unified *wmi_handle;

	if (!vdev || !param) {
		mlme_err("Invalid input");
		return QDF_STATUS_E_INVAL;
	}

	wmi_handle = target_if_vdev_mgr_wmi_handle_get(vdev);
	if (!wmi_handle) {
		mlme_err("Failed to get WMI handle!");
		return QDF_STATUS_E_INVAL;
	}

	status = wmi_unified_beacon_tmpl_send_cmd(wmi_handle, param);
	return status;
}

QDF_STATUS target_if_vdev_mgr_send_fd_tmpl(struct wlan_objmgr_vdev *vdev,
					   struct fils_discovery_tmpl_params *param)
{
	QDF_STATUS status;
	struct wmi_unified *wmi_handle;

	if (!vdev || !param) {
		mlme_err("Invalid input");
		return QDF_STATUS_E_INVAL;
	}

	wmi_handle = target_if_vdev_mgr_wmi_handle_get(vdev);
	if (!wmi_handle) {
		mlme_err("Failed to get WMI handle!");
		return QDF_STATUS_E_INVAL;
	}

	status = wmi_unified_fd_tmpl_send_cmd(wmi_handle, param);

	return status;
}

static QDF_STATUS target_if_vdev_mgr_set_nac_rssi_send(
				struct wlan_objmgr_vdev *vdev,
				struct vdev_scan_nac_rssi_params *param)
{
	QDF_STATUS status;
	struct wmi_unified *wmi_handle;

	if (!vdev || !param) {
		mlme_err("Invalid input");
		return QDF_STATUS_E_INVAL;
	}

	wmi_handle = target_if_vdev_mgr_wmi_handle_get(vdev);
	if (!wmi_handle) {
		mlme_err("Failed to get WMI handle!");
		return QDF_STATUS_E_INVAL;
	}

	status = wmi_unified_vdev_set_nac_rssi_send(wmi_handle, param);

	return status;
}

static QDF_STATUS target_if_vdev_mgr_set_neighbour_rx_cmd_send(
					struct wlan_objmgr_vdev *vdev,
					struct set_neighbour_rx_params *param,
					uint8_t *mac)
{
	QDF_STATUS status;
	struct wmi_unified *wmi_handle;

	if (!vdev || !param) {
		mlme_err("Invalid input");
		return QDF_STATUS_E_INVAL;
	}

	wmi_handle = target_if_vdev_mgr_wmi_handle_get(vdev);
	if (!wmi_handle) {
		mlme_err("Failed to get WMI handle!");
		return QDF_STATUS_E_INVAL;
	}

	status = wmi_unified_vdev_set_neighbour_rx_cmd_send(wmi_handle,
							    mac, param);

	return status;
}

static QDF_STATUS target_if_vdev_mgr_sifs_trigger_send(
					struct wlan_objmgr_vdev *vdev,
					struct sifs_trigger_param *param)
{
	QDF_STATUS status;
	struct wmi_unified *wmi_handle;

	if (!vdev || !param) {
		mlme_err("Invalid input");
		return QDF_STATUS_E_INVAL;
	}

	wmi_handle = target_if_vdev_mgr_wmi_handle_get(vdev);
	if (!wmi_handle) {
		mlme_err("Failed to get WMI handle!");
		return QDF_STATUS_E_INVAL;
	}

	status = wmi_unified_sifs_trigger_send(wmi_handle, param);

	return status;
}

static QDF_STATUS target_if_vdev_mgr_set_custom_aggr_size_cmd_send(
				struct wlan_objmgr_vdev *vdev,
				struct set_custom_aggr_size_params *param)
{
	QDF_STATUS status;
	struct wmi_unified *wmi_handle;

	if (!vdev || !param) {
		mlme_err("Invalid input");
		return QDF_STATUS_E_INVAL;
	}

	wmi_handle = target_if_vdev_mgr_wmi_handle_get(vdev);
	if (!wmi_handle) {
		mlme_err("Failed to get WMI handle!");
		return QDF_STATUS_E_INVAL;
	}

	status = wmi_unified_vdev_set_custom_aggr_size_cmd_send(wmi_handle,
								param);

	return status;
}

static QDF_STATUS target_if_vdev_mgr_config_ratemask_cmd_send(
					struct wlan_objmgr_vdev *vdev,
					struct config_ratemask_params *param)
{
	QDF_STATUS status;
	struct wmi_unified *wmi_handle;

	if (!vdev || !param) {
		mlme_err("Invalid input");
		return QDF_STATUS_E_INVAL;
	}

	wmi_handle = target_if_vdev_mgr_wmi_handle_get(vdev);
	if (!wmi_handle) {
		mlme_err("Failed to get WMI handle!");
		return QDF_STATUS_E_INVAL;
	}

	status = wmi_unified_vdev_config_ratemask_cmd_send(wmi_handle,
							   param);
	return status;
}

static QDF_STATUS target_if_vdev_mgr_peer_flush_tids_send(
					struct wlan_objmgr_vdev *vdev,
					struct peer_flush_params *param)
{
	QDF_STATUS status;
	struct wmi_unified *wmi_handle;

	if (!vdev || !param) {
		mlme_err("Invalid input");
		return QDF_STATUS_E_INVAL;
	}

	wmi_handle = target_if_vdev_mgr_wmi_handle_get(vdev);
	if (!wmi_handle) {
		mlme_err("Failed to get WMI handle!");
		return QDF_STATUS_E_INVAL;
	}

	status = wmi_unified_peer_flush_tids_send(wmi_handle, param->peer_mac,
						  param);

	return status;
}

static int32_t target_if_vdev_mgr_multi_vdev_restart_get_ref(
			struct wlan_objmgr_pdev *pdev,
			struct multiple_vdev_restart_params *param,
			struct wlan_objmgr_vdev **vdev_list,
			bool *vdev_timer_started)
{
	struct wlan_objmgr_psoc *psoc;
	struct wlan_objmgr_vdev *tvdev;
	struct wlan_lmac_if_mlme_rx_ops *rx_ops;
	int32_t vdev_idx = -1;
	int32_t last_vdev_idx = -1;
	struct vdev_response_timer *vdev_rsp;

	psoc = wlan_pdev_get_psoc(pdev);
	rx_ops = target_if_vdev_mgr_get_rx_ops(psoc);

	if (!rx_ops || !rx_ops->psoc_get_vdev_response_timer_info) {
		mlme_err("VDEV_%d: No Rx Ops", vdev_idx);
		return last_vdev_idx;
	}

	for (vdev_idx = 0; vdev_idx < param->num_vdevs ; vdev_idx++) {
		vdev_list[vdev_idx] = wlan_objmgr_get_vdev_by_id_from_pdev(
						pdev,
						param->vdev_ids[vdev_idx],
						WLAN_VDEV_TARGET_IF_ID);
		tvdev = vdev_list[vdev_idx];
		if (!tvdev) {
			mlme_err("VDEV_%d is NULL", vdev_idx);
			return last_vdev_idx;
		}

		vdev_rsp = rx_ops->psoc_get_vdev_response_timer_info(
						psoc,
						wlan_vdev_get_id(tvdev));
		if (!vdev_rsp) {
			wlan_objmgr_vdev_release_ref(tvdev,
						     WLAN_VDEV_TARGET_IF_ID);
			vdev_list[vdev_idx] = NULL;
			mlme_err("VDEV_%d PSOC_%d No vdev rsp timer",
				 vdev_idx, wlan_psoc_get_id(psoc));
			return last_vdev_idx;
		}

		last_vdev_idx = vdev_idx;
		target_if_vdev_mgr_rsp_timer_start(psoc, vdev_rsp,
						   RESTART_RESPONSE_BIT);
		vdev_timer_started[vdev_idx] = true;
	}

	return last_vdev_idx;
}

static void target_if_vdev_mgr_multi_vdev_restart_rel_ref(
				struct wlan_objmgr_pdev *pdev,
				struct wlan_objmgr_vdev **vdev_list,
				bool *vdev_timer_started,
				int32_t last_vdev_idx,
				QDF_STATUS status)
{
	struct wlan_objmgr_psoc *psoc;
	struct wlan_objmgr_vdev *tvdev;
	struct wlan_lmac_if_mlme_rx_ops *rx_ops;
	int32_t vdev_idx;
	struct vdev_response_timer *vdev_rsp;

	psoc = wlan_pdev_get_psoc(pdev);
	rx_ops = target_if_vdev_mgr_get_rx_ops(psoc);
	if (!rx_ops || !rx_ops->psoc_get_vdev_response_timer_info) {
		mlme_err("VDEV_%d: No Rx Ops", last_vdev_idx);
		return;
	}

	for (vdev_idx = 0; vdev_idx <= last_vdev_idx; vdev_idx++) {
		tvdev = vdev_list[vdev_idx];
		vdev_rsp = rx_ops->psoc_get_vdev_response_timer_info(psoc,
								     vdev_idx);
		if (!vdev_rsp) {
			mlme_err("VDEV_%d: PSOC_%d No vdev rsp timer",
				 vdev_idx, wlan_psoc_get_id(psoc));
			return;
		}

		if (QDF_IS_STATUS_ERROR(status)) {
			if (vdev_timer_started[vdev_idx]) {
				target_if_vdev_mgr_rsp_timer_stop(
							psoc, vdev_rsp,
							RESTART_RESPONSE_BIT);
				vdev_timer_started[vdev_idx] = false;
			}
		}
		wlan_objmgr_vdev_release_ref(tvdev,
					     WLAN_VDEV_TARGET_IF_ID);
	}
}

static QDF_STATUS target_if_vdev_mgr_multiple_vdev_restart_req_cmd(
				struct wlan_objmgr_pdev *pdev,
				struct multiple_vdev_restart_params *param)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	struct wmi_unified *wmi_handle;
	struct wlan_objmgr_psoc *psoc;
	struct wlan_objmgr_vdev *vdev_list[WLAN_UMAC_PDEV_MAX_VDEVS] = {NULL};
	bool vdev_timer_started[WLAN_UMAC_PDEV_MAX_VDEVS] = {false};
	int32_t last_vdev_idx = -1;

	if (!pdev || !param) {
		mlme_err("Invalid input");
		return QDF_STATUS_E_INVAL;
	}

	psoc = wlan_pdev_get_psoc(pdev);
	if (!psoc) {
		mlme_err("PSOC is NULL");
		return QDF_STATUS_E_INVAL;
	}

	wmi_handle = get_wmi_unified_hdl_from_pdev(pdev);
	if (!wmi_handle) {
		mlme_err("PDEV WMI Handle is NULL!");
		return QDF_STATUS_E_INVAL;
	}

	if (param->num_vdevs > WLAN_UMAC_PDEV_MAX_VDEVS) {
		mlme_err("param->num_vdevs: %u exceed the limit",
			 param->num_vdevs);
		return QDF_STATUS_E_INVAL;
	}

	last_vdev_idx = target_if_vdev_mgr_multi_vdev_restart_get_ref(
							pdev, param,
							vdev_list,
							vdev_timer_started);
	if (last_vdev_idx < 0 || (last_vdev_idx != (param->num_vdevs - 1))) {
		target_if_vdev_mgr_multi_vdev_restart_rel_ref(
						pdev, vdev_list,
						vdev_timer_started,
						last_vdev_idx,
						QDF_STATUS_E_FAILURE);
		return QDF_STATUS_E_INVAL;
	}

	status = wmi_unified_send_multiple_vdev_restart_req_cmd(wmi_handle,
								param);

	target_if_vdev_mgr_multi_vdev_restart_rel_ref(
						pdev, vdev_list,
						vdev_timer_started,
						last_vdev_idx, status);

	return status;
}

static QDF_STATUS target_if_vdev_mgr_multiple_vdev_set_param_cmd(
				struct wlan_objmgr_pdev *pdev,
				struct multiple_vdev_set_param *param)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	struct wmi_unified *wmi_handle;
	struct wlan_objmgr_psoc *psoc;
	int param_id;

	if (!pdev || !param) {
		mlme_err("Invalid input");
		return QDF_STATUS_E_INVAL;
	}

	psoc = wlan_pdev_get_psoc(pdev);
	if (!psoc) {
		mlme_err("PSOC is NULL");
		return QDF_STATUS_E_INVAL;
	}

	wmi_handle = get_wmi_unified_hdl_from_pdev(pdev);
	if (!wmi_handle) {
		mlme_err("PDEV WMI Handle is NULL!");
		return QDF_STATUS_E_INVAL;
	}

	if (param->num_vdevs > WLAN_UMAC_PDEV_MAX_VDEVS) {
		mlme_err("param->num_vdevs: %u exceed the limit",
			 param->num_vdevs);
		return QDF_STATUS_E_INVAL;
	}

	param_id = target_if_vdev_mlme_id_2_wmi(param->param_id);
	param->param_id = param_id;

	status = wmi_unified_send_multiple_vdev_set_param_cmd(wmi_handle,
							      param);

	return status;
}

static QDF_STATUS target_if_vdev_mgr_beacon_send(
					struct wlan_objmgr_vdev *vdev,
					struct beacon_params *param)
{
	QDF_STATUS status;
	struct wmi_unified *wmi_handle;

	if (!vdev || !param) {
		mlme_err("Invalid input");
		return QDF_STATUS_E_INVAL;
	}

	wmi_handle = target_if_vdev_mgr_wmi_handle_get(vdev);
	if (!wmi_handle) {
		mlme_err("Failed to get WMI handle!");
		return QDF_STATUS_E_INVAL;
	}

	status = wmi_unified_beacon_send_cmd(wmi_handle, param);

	return status;
}

static QDF_STATUS target_if_vdev_mgr_sta_ps_param_send(
					struct wlan_objmgr_vdev *vdev,
					struct sta_ps_params *param)
{
	QDF_STATUS status;
	struct wmi_unified *wmi_handle;
	int param_id;

	if (!vdev || !param) {
		mlme_err("Invalid input");
		return QDF_STATUS_E_INVAL;
	}

	wmi_handle = target_if_vdev_mgr_wmi_handle_get(vdev);
	if (!wmi_handle) {
		mlme_err("Failed to get WMI handle!");
		return QDF_STATUS_E_INVAL;
	}

	param_id = target_if_vdev_mlme_id_2_wmi(param->param_id);
	param->param_id = param_id;

	status = wmi_unified_sta_ps_cmd_send(wmi_handle, param);

	return status;
}

static QDF_STATUS target_if_vdev_mgr_peer_delete_all_send(
					struct wlan_objmgr_vdev *vdev,
					struct peer_delete_all_params *param)
{
	QDF_STATUS status;
	struct wmi_unified *wmi_handle;
	struct wlan_lmac_if_mlme_rx_ops *rx_ops;
	struct wlan_objmgr_psoc *psoc;
	uint8_t vdev_id;
	struct vdev_response_timer *vdev_rsp;

	if (!vdev || !param) {
		mlme_err("Invalid input");
		return QDF_STATUS_E_INVAL;
	}

	wmi_handle = target_if_vdev_mgr_wmi_handle_get(vdev);
	if (!wmi_handle) {
		mlme_err("Failed to get WMI handle!");
		return QDF_STATUS_E_INVAL;
	}

	vdev_id = wlan_vdev_get_id(vdev);
	psoc = wlan_vdev_get_psoc(vdev);
	rx_ops = target_if_vdev_mgr_get_rx_ops(psoc);

	if (!rx_ops || !rx_ops->psoc_get_vdev_response_timer_info) {
		mlme_err("VDEV_%d PSOC_%d No Rx Ops", vdev_id,
			 wlan_psoc_get_id(psoc));
		return QDF_STATUS_E_INVAL;
	}

	vdev_rsp = rx_ops->psoc_get_vdev_response_timer_info(psoc, vdev_id);
	if (!vdev_rsp) {
		mlme_err("VDEV_%d: PSOC_%d No vdev rsp timer", vdev_id,
			 wlan_psoc_get_id(psoc));
		return QDF_STATUS_E_INVAL;
	}

	vdev_rsp->expire_time = PEER_DELETE_ALL_RESPONSE_TIMER;
	target_if_vdev_mgr_rsp_timer_start(psoc, vdev_rsp,
					   PEER_DELETE_ALL_RESPONSE_BIT);

	status = wmi_unified_peer_delete_all_send(wmi_handle, param);
	if (QDF_IS_STATUS_ERROR(status)) {
		vdev_rsp->expire_time = 0;
		vdev_rsp->timer_status = QDF_STATUS_E_CANCELED;
		target_if_vdev_mgr_rsp_timer_stop(psoc, vdev_rsp,
						  PEER_DELETE_ALL_RESPONSE_BIT);
	}
	return status;
}

#if defined(WLAN_SUPPORT_FILS) || defined(CONFIG_BAND_6GHZ)
static QDF_STATUS target_if_vdev_mgr_fils_enable_send(
					struct wlan_objmgr_vdev *vdev,
					struct config_fils_params *param)
{
	QDF_STATUS status;
	struct wmi_unified *wmi_handle;

	if (!vdev || !param) {
		mlme_err("Invalid input");
		return QDF_STATUS_E_INVAL;
	}

	wmi_handle = target_if_vdev_mgr_wmi_handle_get(vdev);
	if (!wmi_handle) {
		mlme_err("Failed to get WMI handle!");
		return QDF_STATUS_E_INVAL;
	}

	status = wmi_unified_vdev_fils_enable_cmd_send(wmi_handle, param);

	return status;
}

static void target_if_vdev_register_tx_fils(
		struct wlan_lmac_if_mlme_tx_ops *mlme_tx_ops)
{
	mlme_tx_ops->vdev_fils_enable_send =
		target_if_vdev_mgr_fils_enable_send;
}
#else
static void target_if_vdev_register_tx_fils(
		struct wlan_lmac_if_mlme_tx_ops *mlme_tx_ops)
{
}
#endif

#ifdef WLAN_FEATURE_DYNAMIC_MAC_ADDR_UPDATE
static QDF_STATUS
target_if_vdev_mgr_set_mac_address_send(struct qdf_mac_addr mac_addr,
					struct qdf_mac_addr mld_addr,
					struct wlan_objmgr_vdev *vdev)
{
	struct set_mac_addr_params params = {0};
	struct wmi_unified *wmi_handle;

	wmi_handle = target_if_vdev_mgr_wmi_handle_get(vdev);
	if (!wmi_handle) {
		mlme_err("Failed to get WMI handle!");
		return QDF_STATUS_E_INVAL;
	}

	params.vdev_id = wlan_vdev_get_id(vdev);
	params.mac_addr = mac_addr;
	params.mld_addr = mld_addr;

	return wmi_unified_send_set_mac_addr(wmi_handle, &params);
}

static void target_if_vdev_register_set_mac_address(
		struct wlan_lmac_if_mlme_tx_ops *mlme_tx_ops)
{
	mlme_tx_ops->vdev_send_set_mac_addr =
				target_if_vdev_mgr_set_mac_address_send;
}
#else
static void target_if_vdev_register_set_mac_address(
		struct wlan_lmac_if_mlme_tx_ops *mlme_tx_ops)
{
}
#endif

QDF_STATUS
target_if_vdev_mgr_register_tx_ops(struct wlan_lmac_if_tx_ops *tx_ops)
{
	struct wlan_lmac_if_mlme_tx_ops *mlme_tx_ops;

	if (!tx_ops) {
		mlme_err("Invalid input");
		return QDF_STATUS_E_INVAL;
	}

	mlme_tx_ops = &tx_ops->mops;
	if (!mlme_tx_ops) {
		mlme_err("No Tx Ops");
		return QDF_STATUS_E_FAILURE;
	}

	mlme_tx_ops->vdev_mlme_attach =
			target_if_vdev_mgr_register_event_handler;
	mlme_tx_ops->vdev_mlme_detach =
			target_if_vdev_mgr_unregister_event_handler;
	mlme_tx_ops->vdev_create_send = target_if_vdev_mgr_create_send;
	mlme_tx_ops->vdev_start_send = target_if_vdev_mgr_start_send;
	mlme_tx_ops->vdev_up_send = target_if_vdev_mgr_up_send;
	mlme_tx_ops->vdev_delete_send = target_if_vdev_mgr_delete_send;
	mlme_tx_ops->vdev_stop_send = target_if_vdev_mgr_stop_send;
	mlme_tx_ops->vdev_down_send = target_if_vdev_mgr_down_send;
	mlme_tx_ops->vdev_set_nac_rssi_send =
			target_if_vdev_mgr_set_nac_rssi_send;
	mlme_tx_ops->vdev_set_neighbour_rx_cmd_send =
			target_if_vdev_mgr_set_neighbour_rx_cmd_send;
	mlme_tx_ops->vdev_sifs_trigger_send =
			target_if_vdev_mgr_sifs_trigger_send;
	mlme_tx_ops->vdev_set_custom_aggr_size_cmd_send =
			target_if_vdev_mgr_set_custom_aggr_size_cmd_send;
	mlme_tx_ops->vdev_config_ratemask_cmd_send =
			target_if_vdev_mgr_config_ratemask_cmd_send;
	mlme_tx_ops->peer_flush_tids_send =
			target_if_vdev_mgr_peer_flush_tids_send;
	mlme_tx_ops->multiple_vdev_restart_req_cmd =
			target_if_vdev_mgr_multiple_vdev_restart_req_cmd;
	mlme_tx_ops->multiple_vdev_set_param_cmd =
			target_if_vdev_mgr_multiple_vdev_set_param_cmd;
	mlme_tx_ops->beacon_cmd_send = target_if_vdev_mgr_beacon_send;
	mlme_tx_ops->beacon_tmpl_send = target_if_vdev_mgr_beacon_tmpl_send;
	mlme_tx_ops->vdev_set_param_send =
			target_if_vdev_mgr_set_param_send;
	mlme_tx_ops->vdev_set_tx_rx_decap_type =
			target_if_vdev_set_tx_rx_decap_type;
	mlme_tx_ops->vdev_sta_ps_param_send =
			target_if_vdev_mgr_sta_ps_param_send;
	mlme_tx_ops->psoc_vdev_rsp_timer_mod =
			target_if_vdev_mgr_rsp_timer_mod;
	mlme_tx_ops->peer_delete_all_send =
			target_if_vdev_mgr_peer_delete_all_send;
	target_if_vdev_register_tx_fils(mlme_tx_ops);

	mlme_tx_ops->psoc_vdev_rsp_timer_init =
			target_if_psoc_vdev_rsp_timer_init;
	mlme_tx_ops->psoc_vdev_rsp_timer_deinit =
			target_if_psoc_vdev_rsp_timer_deinit;
	mlme_tx_ops->psoc_vdev_rsp_timer_inuse =
			target_if_psoc_vdev_rsp_timer_inuse;
	mlme_tx_ops->psoc_wake_lock_init =
			target_if_wake_lock_init;
	mlme_tx_ops->psoc_wake_lock_deinit =
			target_if_wake_lock_deinit;
	mlme_tx_ops->vdev_mgr_rsp_timer_stop =
			target_if_vdev_mgr_rsp_timer_stop;
	target_if_vdev_register_set_mac_address(mlme_tx_ops);
	return QDF_STATUS_SUCCESS;
}
