/*
 * Copyright (c) 2012-2015, 2020-2021, The Linux Foundation. All rights reserved.
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
 * DOC: Implements legacy connect specific APIs of connection manager to
 * initiate vdev manager operations
 */

#include "wlan_cm_vdev_api.h"
#include "wlan_scan_utils_api.h"
#include "wlan_mlme_dbg.h"
#include "wlan_cm_api.h"
#include "wlan_policy_mgr_api.h"
#include "wlan_p2p_api.h"
#include "wlan_tdls_api.h"
#include "wlan_mlme_vdev_mgr_interface.h"
#include "wni_api.h"
#include "wlan_crypto_global_api.h"
#include "wlan_scan_api.h"
#include "wlan_logging_sock_svc.h"
#include "cfg_ucfg_api.h"
#include "wlan_roam_debug.h"
#include "wlan_mlo_mgr_sta.h"

#ifdef WLAN_FEATURE_FILS_SK
void cm_update_hlp_info(struct wlan_objmgr_vdev *vdev,
			const uint8_t *gen_ie, uint16_t len,
			bool flush)
{
	struct mlme_legacy_priv *mlme_priv;

	mlme_priv = wlan_vdev_mlme_get_ext_hdl(vdev);
	if (!mlme_priv)
		return;

	if (flush) {
		mlme_priv->connect_info.hlp_ie_len = 0;
		if (mlme_priv->connect_info.hlp_ie) {
			qdf_mem_free(mlme_priv->connect_info.hlp_ie);
			mlme_priv->connect_info.hlp_ie = NULL;
		}
	}

	if (!len || !gen_ie)
		return;

	if ((mlme_priv->connect_info.hlp_ie_len + len) >
	    FILS_MAX_HLP_DATA_LEN) {
		mlme_err("HLP len exceeds: hlp_ie_len %d len %d",
			 mlme_priv->connect_info.hlp_ie_len, len);
		return;
	}

	if (!mlme_priv->connect_info.hlp_ie) {
		mlme_priv->connect_info.hlp_ie =
				qdf_mem_malloc(FILS_MAX_HLP_DATA_LEN);
		if (!mlme_priv->connect_info.hlp_ie)
			return;
	}

	qdf_mem_copy(mlme_priv->connect_info.hlp_ie +
		     mlme_priv->connect_info.hlp_ie_len, gen_ie, len);
	mlme_priv->connect_info.hlp_ie_len += len;
	mlme_debug("hlp_ie_len %d len %d", mlme_priv->connect_info.hlp_ie_len,
		   len);
}
#endif

static bool wlan_cm_is_vdev_id_roam_reassoc_state(struct wlan_objmgr_vdev *vdev)
{
	return wlan_cm_is_vdev_roam_reassoc_state(vdev);
}

static void
wlan_cm_disconnect_on_wait_key_timeout(struct wlan_objmgr_psoc *psoc,
				       struct wlan_objmgr_vdev *vdev)
{
	mlo_disconnect(vdev, CM_MLME_DISCONNECT, REASON_KEY_TIMEOUT, NULL);
}

void cm_wait_for_key_time_out_handler(void *data)
{
	uint8_t vdev_id;
	struct wlan_objmgr_vdev *vdev;
	struct wlan_objmgr_psoc *psoc;
	struct wlan_mlme_psoc_ext_obj *mlme_obj;
	struct wait_for_key_timer *wait_key_timer =
				(struct wait_for_key_timer *)data;

	vdev = wait_key_timer->vdev;
	vdev_id = vdev->vdev_objmgr.vdev_id;

	psoc = wlan_vdev_get_psoc(vdev);
	if (!psoc) {
		mlme_err("psoc obj is NULL for vdev id %d", vdev_id);
		return;
	}

	mlme_obj = mlme_get_psoc_ext_obj(psoc);
	if (!mlme_obj) {
		mlme_err("psoc mlme obj is NULL for vdev id %d", vdev_id);
		return;
	}

	if (cm_csr_is_ss_wait_for_key(vdev_id)) {
		cm_csr_set_ss_none(vdev_id);
		if (wlan_cm_is_vdev_id_roam_reassoc_state(vdev))
			mlme_obj->cfg.timeouts.heart_beat_threshold =
					cfg_default(CFG_HEART_BEAT_THRESHOLD);

		wlan_cm_disconnect_on_wait_key_timeout(psoc, vdev);
	}
}

QDF_STATUS cm_start_wait_for_key_timer(struct wlan_objmgr_vdev *vdev,
				       uint32_t interval)
{
	uint8_t vdev_id;
	struct wlan_objmgr_psoc *psoc;
	struct wlan_mlme_psoc_ext_obj *mlme_obj;
	struct vdev_mlme_obj *vdev_mlme;

	vdev_mlme = wlan_vdev_mlme_get_cmpt_obj(vdev);
	if (!vdev_mlme) {
		mlme_err("vdev priv mlme obj is NULL");
		return QDF_STATUS_E_INVAL;
	}
	vdev_id = vdev->vdev_objmgr.vdev_id;

	psoc = wlan_vdev_get_psoc(vdev);
	if (!psoc) {
		mlme_err("psoc obj is NULL for vdev id %d", vdev_id);
		return QDF_STATUS_E_INVAL;
	}

	mlme_obj = mlme_get_psoc_ext_obj(psoc);
	if (!mlme_obj) {
		mlme_err("psoc mlme obj is NULL for vdev id %d", vdev_id);
		return QDF_STATUS_E_INVAL;
	}

	if (wlan_cm_is_vdev_id_roam_reassoc_state(vdev))
		mlme_obj->cfg.timeouts.heart_beat_threshold = 0;

	return qdf_mc_timer_start(&vdev_mlme->ext_vdev_ptr->wait_key_timer.timer,
				  interval / QDF_MC_TIMER_TO_MS_UNIT);
}

void cm_stop_wait_for_key_timer(struct wlan_objmgr_psoc *psoc, uint8_t vdev_id)
{
	struct wlan_objmgr_vdev *vdev;
	struct vdev_mlme_obj *vdev_mlme;
	struct wlan_mlme_psoc_ext_obj *mlme_obj;

	vdev = wlan_objmgr_get_vdev_by_id_from_psoc(psoc, vdev_id,
						    WLAN_MLME_CM_ID);
	if (!vdev) {
		mlme_err("vdev is NULL for vdev id %d", vdev_id);
		return;
	}
	vdev_mlme = wlan_vdev_mlme_get_cmpt_obj(vdev);
	if (!vdev_mlme) {
		mlme_err("vdev priv mlme obj is NULL");
		goto end;
	}

	mlme_obj = mlme_get_psoc_ext_obj(psoc);
	if (!mlme_obj) {
		mlme_err("psoc mlme obj is NULL for vdev id %d", vdev_id);
		goto end;
	}

	if (wlan_cm_is_vdev_id_roam_reassoc_state(vdev))
		mlme_obj->cfg.timeouts.heart_beat_threshold =
					cfg_default(CFG_HEART_BEAT_THRESHOLD);

	qdf_mc_timer_stop(&vdev_mlme->ext_vdev_ptr->wait_key_timer.timer);

end:
	wlan_objmgr_vdev_release_ref(vdev, WLAN_MLME_CM_ID);
}

void cm_update_wait_for_key_timer(struct wlan_objmgr_vdev *vdev,
				  uint8_t vdev_id, uint32_t interval)
{
	cm_csr_set_ss_wait_for_key(vdev_id);

	if (QDF_IS_STATUS_ERROR(cm_start_wait_for_key_timer(vdev,  interval))) {
		mlme_err("Failed wait for key timer start");
		cm_csr_set_ss_none(vdev_id);
	}
}

void cm_update_prev_ap_ie(struct wlan_objmgr_psoc *psoc, uint8_t vdev_id,
			  uint32_t len, uint8_t *bcn_ptr)
{
	struct wlan_objmgr_vdev *vdev;
	struct rso_config *rso_cfg;
	struct element_info *bcn_ie;

	if (!len || !bcn_ptr)
		return;

	vdev = wlan_objmgr_get_vdev_by_id_from_psoc(psoc, vdev_id,
						    WLAN_MLME_CM_ID);
	if (!vdev) {
		mlme_err("vdev is NULL for vdev id %d", vdev_id);
		return;
	}
	rso_cfg = wlan_cm_get_rso_config(vdev);
	if (!rso_cfg)
		goto end;

	bcn_ie = &rso_cfg->prev_ap_bcn_ie;
	if (bcn_ie->ptr) {
		qdf_mem_free(bcn_ie->ptr);
		bcn_ie->ptr = NULL;
		bcn_ie->len = 0;
	}
	bcn_ie->ptr = qdf_mem_malloc(len);
	if (!bcn_ie->ptr) {
		bcn_ie->len = 0;
		goto end;
	}

	qdf_mem_copy(bcn_ie->ptr, bcn_ptr, len);
end:
	wlan_objmgr_vdev_release_ref(vdev, WLAN_MLME_CM_ID);
}

QDF_STATUS cm_get_rssi_snr_by_bssid(struct wlan_objmgr_pdev *pdev,
				    struct qdf_mac_addr *bssid,
				    int8_t *rssi, int8_t *snr)
{
	struct scan_filter *scan_filter;
	qdf_list_t *list = NULL;
	struct scan_cache_node *first_node = NULL;
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	if (snr)
		*snr = 0;
	if (rssi)
		*rssi = 0;
	scan_filter = qdf_mem_malloc(sizeof(*scan_filter));
	if (!scan_filter)
		return QDF_STATUS_E_NOMEM;

	scan_filter->num_of_bssid = 1;
	qdf_mem_copy(scan_filter->bssid_list[0].bytes,
		     bssid, sizeof(struct qdf_mac_addr));
	scan_filter->ignore_auth_enc_type = true;
	list = wlan_scan_get_result(pdev, scan_filter);
	qdf_mem_free(scan_filter);

	if (!list || (list && !qdf_list_size(list))) {
		mlme_debug("scan list empty");
		status = QDF_STATUS_E_NULL_VALUE;
		goto error;
	}

	qdf_list_peek_front(list, (qdf_list_node_t **) &first_node);
	if (first_node && first_node->entry) {
		if (rssi)
			*rssi = first_node->entry->rssi_raw;
		if (snr)
			*snr = first_node->entry->snr;
	}

error:
	if (list)
		wlan_scan_purge_results(list);

	return status;
}

#ifdef FEATURE_WLAN_DIAG_SUPPORT_CSR
#ifdef WLAN_FEATURE_11BE
static const
char *cm_diag_get_eht_dot11_mode_str(enum mgmt_dot11_mode dot11mode)
{

	switch (dot11mode) {
	CASE_RETURN_STRING(DOT11_MODE_11BE);
	CASE_RETURN_STRING(DOT11_MODE_11BE_ONLY);
	default:
		return "Unknown";
	}
}

static const char *cm_diag_get_320_ch_width_str(uint8_t ch_width)
{
	switch (ch_width) {
	CASE_RETURN_STRING(BW_320MHZ);
	default:
		return "Unknown";
	}
}
#else
static const
char *cm_diag_get_eht_dot11_mode_str(enum mgmt_dot11_mode dot11mode)
{
	return "Unknown";
}

static const char *cm_diag_get_320_ch_width_str(uint8_t ch_width)
{
	return "Unknown";
}
#endif

static const char *cm_diag_get_ch_width_str(uint8_t ch_width)
{
	switch (ch_width) {
	CASE_RETURN_STRING(BW_20MHZ);
	CASE_RETURN_STRING(BW_40MHZ);
	CASE_RETURN_STRING(BW_80MHZ);
	CASE_RETURN_STRING(BW_160MHZ);
	CASE_RETURN_STRING(BW_80P80MHZ);
	CASE_RETURN_STRING(BW_5MHZ);
	CASE_RETURN_STRING(BW_10MHZ);
	default:
		return cm_diag_get_320_ch_width_str(ch_width);
	}
}

static const char *cm_diag_get_dot11_mode_str(enum mgmt_dot11_mode dot11mode)
{
	switch (dot11mode) {
	CASE_RETURN_STRING(DOT11_MODE_AUTO);
	CASE_RETURN_STRING(DOT11_MODE_ABG);
	CASE_RETURN_STRING(DOT11_MODE_11A);
	CASE_RETURN_STRING(DOT11_MODE_11B);
	CASE_RETURN_STRING(DOT11_MODE_11G);
	CASE_RETURN_STRING(DOT11_MODE_11N);
	CASE_RETURN_STRING(DOT11_MODE_11AC);
	CASE_RETURN_STRING(DOT11_MODE_11G_ONLY);
	CASE_RETURN_STRING(DOT11_MODE_11N_ONLY);
	CASE_RETURN_STRING(DOT11_MODE_11AC_ONLY);
	CASE_RETURN_STRING(DOT11_MODE_11AX);
	CASE_RETURN_STRING(DOT11_MODE_11AX_ONLY);
	default:
		return cm_diag_get_eht_dot11_mode_str(dot11mode);
	}
}

static const char *cm_diag_get_encr_type_str(uint8_t encr_type)
{
	switch (encr_type) {
	CASE_RETURN_STRING(ENC_MODE_OPEN);
	CASE_RETURN_STRING(ENC_MODE_WEP40);
	CASE_RETURN_STRING(ENC_MODE_WEP104);
	CASE_RETURN_STRING(ENC_MODE_TKIP);
	CASE_RETURN_STRING(ENC_MODE_AES);
	CASE_RETURN_STRING(ENC_MODE_AES_GCMP);
	CASE_RETURN_STRING(ENC_MODE_AES_GCMP_256);
	CASE_RETURN_STRING(ENC_MODE_SMS4);
	default:
		return "Unknown";
	}
}

static const uint8_t *cm_diag_get_akm_str(enum mgmt_auth_type auth_type,
					  uint32_t akm)
{
	if (auth_type == AUTH_OPEN)
		return "Open";
	else if (auth_type == AUTH_SHARED)
		return "Shared Key";
	else if (QDF_HAS_PARAM(akm, WLAN_CRYPTO_KEY_MGMT_FT_FILS_SHA384))
		return "FT-FILS-SHA384";
	else if (QDF_HAS_PARAM(akm, WLAN_CRYPTO_KEY_MGMT_FT_FILS_SHA256))
		return "FT-FILS-SHA256";
	else if (QDF_HAS_PARAM(akm, WLAN_CRYPTO_KEY_MGMT_FILS_SHA384))
		return "FILS-SHA384";
	else if (QDF_HAS_PARAM(akm, WLAN_CRYPTO_KEY_MGMT_FILS_SHA256))
		return "FILS-SHA256";
	else if (QDF_HAS_PARAM(akm, WLAN_CRYPTO_KEY_MGMT_FT_SAE))
		return "FT-SAE";
	else if (QDF_HAS_PARAM(akm, WLAN_CRYPTO_KEY_MGMT_SAE))
		return "SAE";
	else if (QDF_HAS_PARAM(akm, WLAN_CRYPTO_KEY_MGMT_DPP))
		return "DPP";
	else if (QDF_HAS_PARAM(akm, WLAN_CRYPTO_KEY_MGMT_OSEN))
		return "OSEN";
	else if (QDF_HAS_PARAM(akm, WLAN_CRYPTO_KEY_MGMT_OWE))
		return "OWE";
	else if (QDF_HAS_PARAM(akm, WLAN_CRYPTO_KEY_MGMT_FT_IEEE8021X))
		return "FT-802.1x";
	else if (QDF_HAS_PARAM(akm, WLAN_CRYPTO_KEY_MGMT_FT_PSK))
		return "FT-PSK";
	else if (QDF_HAS_PARAM(akm, WLAN_CRYPTO_KEY_MGMT_IEEE8021X))
		return "EAP 802.1x";
	else if (QDF_HAS_PARAM(akm, WLAN_CRYPTO_KEY_MGMT_PSK))
		return "WPA2-PSK";
	else if (QDF_HAS_PARAM(akm, WLAN_CRYPTO_KEY_MGMT_CCKM))
		return "RSN-CCKM";
	else if (QDF_HAS_PARAM(akm, WLAN_CRYPTO_KEY_MGMT_PSK_SHA256))
		return "PSK-SHA256";
	else if (QDF_HAS_PARAM(akm, WLAN_CRYPTO_KEY_MGMT_IEEE8021X_SHA256))
		return "EAP 802.1x-SHA256";
	else if (QDF_HAS_PARAM(akm, WLAN_CRYPTO_KEY_MGMT_IEEE8021X_SUITE_B))
		return "EAP Suite-B SHA256";
	else if (QDF_HAS_PARAM(akm, WLAN_CRYPTO_KEY_MGMT_IEEE8021X_SUITE_B_192))
		return "EAP Suite-B SHA384";
	else if (QDF_HAS_PARAM(akm, WLAN_CRYPTO_KEY_MGMT_FT_IEEE8021X_SHA384))
		return "FT-Suite-B SHA384";
	else if (QDF_HAS_PARAM(akm, WLAN_CRYPTO_KEY_MGMT_IEEE8021X))
		return "WPA";
	else if (QDF_HAS_PARAM(akm, WLAN_CRYPTO_KEY_MGMT_PSK))
		return "WPA-PSK";
	else if (QDF_HAS_PARAM(akm, WLAN_CRYPTO_KEY_MGMT_CCKM))
		return "WPA-CCKM";
	else if (QDF_HAS_PARAM(akm, WLAN_CRYPTO_KEY_MGMT_WPA_NONE))
		return "WPA-NONE";
	else if (QDF_HAS_PARAM(akm, WLAN_CRYPTO_KEY_MGMT_WAPI_CERT))
		return "WAPI-CERT";
	else if (QDF_HAS_PARAM(akm, WLAN_CRYPTO_KEY_MGMT_WAPI_PSK))
		return "WAPI-PSK";
	else
		return "NONE";
}

#ifdef WLAN_FEATURE_11BE
static enum mgmt_dot11_mode
cm_diag_eht_dot11_mode_from_phy_mode(enum wlan_phymode phymode)
{
	if (IS_WLAN_PHYMODE_EHT(phymode))
		return DOT11_MODE_11BE;
	else
		return DOT11_MODE_MAX;
}

static enum mgmt_ch_width
cm_get_diag_eht_320_ch_width(enum phy_ch_width ch_width)
{
	if (ch_width == CH_WIDTH_320MHZ)
		return BW_320MHZ;
	else
		return BW_MAX;
}
#else
static enum mgmt_dot11_mode
cm_diag_eht_dot11_mode_from_phy_mode(enum wlan_phymode phymode)
{
	return DOT11_MODE_MAX;
}

static enum mgmt_ch_width
cm_get_diag_eht_320_ch_width(enum phy_ch_width ch_width)
{
	return BW_MAX;
}
#endif

static enum mgmt_dot11_mode
cm_diag_dot11_mode_from_phy_mode(enum wlan_phymode phymode)
{
	switch (phymode) {
	case WLAN_PHYMODE_11A:
		return DOT11_MODE_11A;
	case WLAN_PHYMODE_11B:
		return DOT11_MODE_11B;
	case WLAN_PHYMODE_11G:
		return DOT11_MODE_11G;
	case WLAN_PHYMODE_11G_ONLY:
		return DOT11_MODE_11G_ONLY;
	case WLAN_PHYMODE_AUTO:
		return DOT11_MODE_AUTO;
	default:
		if (IS_WLAN_PHYMODE_HT(phymode))
			return DOT11_MODE_11N;
		else if (IS_WLAN_PHYMODE_VHT(phymode))
			return DOT11_MODE_11AC;
		else if (IS_WLAN_PHYMODE_HE(phymode))
			return DOT11_MODE_11AX;
		else
			return cm_diag_eht_dot11_mode_from_phy_mode(phymode);
	}
}

static enum mgmt_ch_width cm_get_diag_ch_width(enum phy_ch_width ch_width)
{
	switch (ch_width) {
	case CH_WIDTH_20MHZ:
		return BW_20MHZ;
	case CH_WIDTH_40MHZ:
		return BW_40MHZ;
	case CH_WIDTH_80MHZ:
		return BW_80MHZ;
	case CH_WIDTH_160MHZ:
		return BW_160MHZ;
	case CH_WIDTH_80P80MHZ:
		return BW_80P80MHZ;
	case CH_WIDTH_5MHZ:
		return BW_5MHZ;
	case CH_WIDTH_10MHZ:
		return BW_10MHZ;
	default:
		return cm_get_diag_eht_320_ch_width(ch_width);
	}
}

static enum mgmt_bss_type cm_get_diag_persona(enum QDF_OPMODE persona)
{
	switch (persona) {
	case QDF_STA_MODE:
		return STA_PERSONA;
	case QDF_SAP_MODE:
		return SAP_PERSONA;
	case QDF_P2P_CLIENT_MODE:
		return P2P_CLIENT_PERSONA;
	case QDF_P2P_GO_MODE:
		return P2P_GO_PERSONA;
	case QDF_FTM_MODE:
		return FTM_PERSONA;
	case QDF_MONITOR_MODE:
		return MONITOR_PERSONA;
	case QDF_P2P_DEVICE_MODE:
		return P2P_DEVICE_PERSONA;
	case QDF_OCB_MODE:
		return OCB_PERSONA;
	case QDF_EPPING_MODE:
		return EPPING_PERSONA;
	case QDF_QVIT_MODE:
		return QVIT_PERSONA;
	case QDF_NDI_MODE:
		return NDI_PERSONA;
	case QDF_WDS_MODE:
		return WDS_PERSONA;
	case QDF_BTAMP_MODE:
		return BTAMP_PERSONA;
	case QDF_AHDEMO_MODE:
		return AHDEMO_PERSONA;
	default:
		return MAX_PERSONA;
	}
}

static enum mgmt_encrypt_type cm_get_diag_enc_type(uint32_t cipherset)
{
	enum mgmt_encrypt_type n = ENC_MODE_OPEN;

	if (!cipherset)
		return n;

	if (QDF_HAS_PARAM(cipherset, WLAN_CRYPTO_CIPHER_AES_GCM_256))
		n = ENC_MODE_AES_GCMP_256;
	else if (QDF_HAS_PARAM(cipherset, WLAN_CRYPTO_CIPHER_AES_GCM))
		n = ENC_MODE_AES_GCMP;
	else if (QDF_HAS_PARAM(cipherset, WLAN_CRYPTO_CIPHER_AES_CCM) ||
		 QDF_HAS_PARAM(cipherset, WLAN_CRYPTO_CIPHER_AES_OCB) ||
		 QDF_HAS_PARAM(cipherset, WLAN_CRYPTO_CIPHER_AES_CCM_256) ||
		 QDF_HAS_PARAM(cipherset, WLAN_CRYPTO_CIPHER_AES_CMAC) ||
		 QDF_HAS_PARAM(cipherset, WLAN_CRYPTO_CIPHER_AES_CMAC_256) ||
		 QDF_HAS_PARAM(cipherset, WLAN_CRYPTO_CIPHER_AES_GMAC) ||
		 QDF_HAS_PARAM(cipherset, WLAN_CRYPTO_CIPHER_AES_GMAC_256))
		n = ENC_MODE_AES;
	else if (QDF_HAS_PARAM(cipherset, WLAN_CRYPTO_CIPHER_TKIP))
		n = ENC_MODE_TKIP;
	else if (QDF_HAS_PARAM(cipherset, WLAN_CRYPTO_CIPHER_WAPI_GCM4) ||
		 QDF_HAS_PARAM(cipherset, WLAN_CRYPTO_CIPHER_WAPI_SMS4))
		n = ENC_MODE_SMS4;
	else if (QDF_HAS_PARAM(cipherset, WLAN_CRYPTO_CIPHER_WEP))
		n = ENC_MODE_WEP40;
	else if (QDF_HAS_PARAM(cipherset, WLAN_CRYPTO_CIPHER_WEP_40))
		n = ENC_MODE_WEP40;
	else if (QDF_HAS_PARAM(cipherset, WLAN_CRYPTO_CIPHER_WEP_104))
		n = ENC_MODE_WEP104;

	return n;
}

static void cm_diag_fill_rsn_auth_type(uint8_t *auth_type, uint32_t akm)
{
	if (QDF_HAS_PARAM(akm, WLAN_CRYPTO_KEY_MGMT_FT_IEEE8021X) ||
	    QDF_HAS_PARAM(akm, WLAN_CRYPTO_KEY_MGMT_IEEE8021X) ||
	    QDF_HAS_PARAM(akm, WLAN_CRYPTO_KEY_MGMT_IEEE8021X_SHA256) ||
	    QDF_HAS_PARAM(akm, WLAN_CRYPTO_KEY_MGMT_IEEE8021X_SUITE_B) ||
	    QDF_HAS_PARAM(akm, WLAN_CRYPTO_KEY_MGMT_IEEE8021X_SUITE_B_192) ||
	    QDF_HAS_PARAM(akm, WLAN_CRYPTO_KEY_MGMT_FT_IEEE8021X_SHA384))
		*auth_type = AUTH_WPA2_EAP;
	else
		*auth_type = AUTH_WPA2_PSK;
}

static void cm_diag_fill_wpa_auth_type(uint8_t *auth_type, uint32_t akm)
{
	/* Try the more preferred ones first. */
	if (QDF_HAS_PARAM(akm, WLAN_CRYPTO_KEY_MGMT_IEEE8021X))
		*auth_type = AUTH_WPA_EAP;
	else
		*auth_type = AUTH_WPA_PSK;
}

static void cm_diag_fill_wapi_auth_type(uint8_t *auth_type, uint32_t akm)
{
	/* Try the more preferred ones first. */
	if (QDF_HAS_PARAM(akm, WLAN_CRYPTO_KEY_MGMT_WAPI_CERT))
		*auth_type = AUTH_WAPI_CERT;
	else
		*auth_type = AUTH_WAPI_PSK;
}

static void cm_diag_get_auth_type(uint8_t *auth_type,
				  uint32_t authmodeset, uint32_t akm,
				  uint32_t ucastcipherset)
{
	if (!authmodeset) {
		*auth_type = AUTH_OPEN;
		return;
	}

	if (QDF_HAS_PARAM(authmodeset, WLAN_CRYPTO_AUTH_NONE) ||
	    QDF_HAS_PARAM(authmodeset, WLAN_CRYPTO_AUTH_OPEN)) {
		*auth_type = AUTH_OPEN;
		return;
	}

	if (QDF_HAS_PARAM(authmodeset, WLAN_CRYPTO_AUTH_AUTO)) {
		if ((QDF_HAS_PARAM(ucastcipherset, WLAN_CRYPTO_CIPHER_WEP) ||
		     QDF_HAS_PARAM(ucastcipherset, WLAN_CRYPTO_CIPHER_WEP_40) ||
		     QDF_HAS_PARAM(ucastcipherset, WLAN_CRYPTO_CIPHER_WEP_104)))
			*auth_type = AUTH_SHARED;
		else
			*auth_type = AUTH_OPEN;

		return;
	}

	if (QDF_HAS_PARAM(authmodeset, WLAN_CRYPTO_AUTH_SHARED)) {
		*auth_type = AUTH_SHARED;
		return;
	}

	if (QDF_HAS_PARAM(authmodeset, WLAN_CRYPTO_AUTH_8021X) ||
	    QDF_HAS_PARAM(authmodeset, WLAN_CRYPTO_AUTH_RSNA) ||
	    QDF_HAS_PARAM(authmodeset, WLAN_CRYPTO_AUTH_CCKM) ||
	    QDF_HAS_PARAM(authmodeset, WLAN_CRYPTO_AUTH_SAE) ||
	    QDF_HAS_PARAM(authmodeset, WLAN_CRYPTO_AUTH_FILS_SK)) {
		cm_diag_fill_rsn_auth_type(auth_type, akm);
		return;
	}

	if (QDF_HAS_PARAM(authmodeset, WLAN_CRYPTO_AUTH_WPA)) {
		cm_diag_fill_wpa_auth_type(auth_type, akm);
		return;
	}

	if (QDF_HAS_PARAM(authmodeset, WLAN_CRYPTO_AUTH_WAPI)) {
		cm_diag_fill_wapi_auth_type(auth_type, akm);
		return;
	}

	*auth_type = AUTH_OPEN;
}

static void
cm_connect_success_diag(struct wlan_mlme_psoc_ext_obj *mlme_obj,
			struct host_event_wlan_connection_stats *stats)
{
	WLAN_HOST_DIAG_EVENT_DEF(connect_status,
				 host_event_wlan_status_payload_type);

	qdf_mem_zero(&connect_status,
		     sizeof(host_event_wlan_status_payload_type));

	connect_status.eventId = DIAG_WLAN_STATUS_CONNECT;
	connect_status.bssType = 0;
	mlme_obj->cfg.sta.current_rssi = stats->rssi;

	connect_status.qosCapability = stats->qos_capability;
	connect_status.authType = stats->auth_type;
	connect_status.encryptionType = stats->encryption_type;
	qdf_mem_copy(connect_status.ssid, stats->ssid, stats->ssid_len);
	connect_status.reason = DIAG_REASON_UNSPECIFIED;
	qdf_mem_copy(&mlme_obj->cfg.sta.event_payload, &connect_status,
		     sizeof(host_event_wlan_status_payload_type));
	WLAN_HOST_DIAG_EVENT_REPORT(&connect_status, EVENT_WLAN_STATUS_V2);
}

void cm_connect_info(struct wlan_objmgr_vdev *vdev, bool connect_success,
		     struct qdf_mac_addr *bssid, struct wlan_ssid *ssid,
		     qdf_freq_t freq)
{
	struct wlan_channel *des_chan;
	struct vdev_mlme_obj *vdev_mlme;
	struct wlan_crypto_params *crypto_params;
	uint8_t max_supported_nss;
	enum QDF_OPMODE opmode;
	struct wlan_objmgr_pdev *pdev;
	struct wlan_objmgr_psoc *psoc;
	struct wlan_mlme_psoc_ext_obj *mlme_obj;
	WLAN_HOST_DIAG_EVENT_DEF(conn_stats,
				 struct host_event_wlan_connection_stats);

	if (!ssid || !bssid)
		return;
	pdev = wlan_vdev_get_pdev(vdev);
	if (!pdev)
		return;
	psoc = wlan_pdev_get_psoc(pdev);
	if (!psoc)
		return;
	mlme_obj = mlme_get_psoc_ext_obj(psoc);
	if (!mlme_obj)
		return;
	qdf_mem_zero(&conn_stats,
		     sizeof(struct host_event_wlan_connection_stats));

	qdf_mem_copy(conn_stats.bssid, bssid->bytes,
		     QDF_MAC_ADDR_SIZE);

	conn_stats.ssid_len = ssid->length;
	if (conn_stats.ssid_len > WLAN_SSID_MAX_LEN)
		conn_stats.ssid_len = WLAN_SSID_MAX_LEN;
	qdf_mem_copy(conn_stats.ssid, ssid->ssid, conn_stats.ssid_len);

	cm_get_rssi_snr_by_bssid(pdev, bssid, &conn_stats.rssi, NULL);
	conn_stats.est_link_speed = 0;

	des_chan = wlan_vdev_mlme_get_des_chan(vdev);

	conn_stats.chnl_bw =
		cm_get_diag_ch_width(des_chan->ch_width);
	conn_stats.dot11mode =
		cm_diag_dot11_mode_from_phy_mode(des_chan->ch_phymode);

	opmode = wlan_vdev_mlme_get_opmode(vdev);
	conn_stats.bss_type = cm_get_diag_persona(opmode);

	if (freq)
		conn_stats.operating_channel =
			wlan_reg_freq_to_chan(pdev, freq);

	vdev_mlme = wlan_vdev_mlme_get_cmpt_obj(vdev);
	if (!vdev_mlme) {
		mlme_err("vdev component object is NULL");
		return;
	}
	conn_stats.qos_capability =
			vdev_mlme->ext_vdev_ptr->connect_info.qos_enabled;

	crypto_params = wlan_crypto_vdev_get_crypto_params(vdev);

	if (!crypto_params) {
		mlme_err("crypto params is null");
		return;
	}

	cm_diag_get_auth_type(&conn_stats.auth_type,
			   crypto_params->authmodeset,
			   crypto_params->key_mgmt,
			   crypto_params->ucastcipherset);

	conn_stats.encryption_type =
	     cm_get_diag_enc_type(crypto_params->ucastcipherset);

	conn_stats.result_code = connect_success;
	conn_stats.reason_code = 0;
	conn_stats.op_freq = freq;

	max_supported_nss = mlme_obj->cfg.vht_caps.vht_cap_info.enable2x2 ?
			    MAX_VDEV_NSS : 1;
	mlme_nofl_debug("+---------CONNECTION INFO START------------+");
	mlme_nofl_debug("VDEV-ID: %d self_mac:"QDF_MAC_ADDR_FMT,
			wlan_vdev_get_id(vdev),
			QDF_MAC_ADDR_REF(wlan_vdev_mlme_get_macaddr(vdev)));
	mlme_nofl_debug("ssid: %.*s bssid: "QDF_MAC_ADDR_FMT" RSSI: %d dBm",
			conn_stats.ssid_len, conn_stats.ssid,
			QDF_MAC_ADDR_REF(conn_stats.bssid), conn_stats.rssi);
	mlme_nofl_debug("Channel Freq: %d channel_bw: %s dot11Mode: %s",
			conn_stats.op_freq,
			cm_diag_get_ch_width_str(conn_stats.chnl_bw),
			cm_diag_get_dot11_mode_str(conn_stats.dot11mode));
	mlme_nofl_debug("AKM: %s Encry-type: %s",
			cm_diag_get_akm_str(conn_stats.auth_type,
					    crypto_params->key_mgmt),
			cm_diag_get_encr_type_str(conn_stats.encryption_type));
	mlme_nofl_debug("DUT_NSS: %d | Intersected NSS:%d",
			max_supported_nss, wlan_vdev_mlme_get_nss(vdev));
	mlme_nofl_debug("Qos enable: %d | Associated: %s",
			conn_stats.qos_capability,
			(conn_stats.result_code ? "yes" : "no"));
	mlme_nofl_debug("+---------CONNECTION INFO END------------+");

	WLAN_HOST_DIAG_EVENT_REPORT(&conn_stats, EVENT_WLAN_CONN_STATS_V2);

	if (!connect_success)
		return;

	/* store connect info on success */
	cm_connect_success_diag(mlme_obj, &conn_stats);
}

void cm_diag_get_auth_enc_type_vdev_id(struct wlan_objmgr_psoc *psoc,
				       uint8_t *auth_type,
				       uint8_t *ucast_cipher,
				       uint8_t *mcast_cipher,
				       uint8_t vdev_id)
{
	struct wlan_objmgr_vdev *vdev;
	struct wlan_crypto_params *crypto_params;

	vdev = wlan_objmgr_get_vdev_by_id_from_psoc(psoc, vdev_id,
						    WLAN_MLME_CM_ID);
	if (!vdev) {
		mlme_err("vdev object is NULL for vdev %d", vdev_id);
		return;
	}

	crypto_params = wlan_crypto_vdev_get_crypto_params(vdev);
	if (!crypto_params) {
		mlme_err("crypto params is null");
		wlan_objmgr_vdev_release_ref(vdev, WLAN_MLME_CM_ID);
		return;
	}
	if (auth_type)
		cm_diag_get_auth_type(auth_type,
				      crypto_params->authmodeset,
				      crypto_params->key_mgmt,
				      crypto_params->ucastcipherset);
	if (ucast_cipher)
		*ucast_cipher =
			cm_get_diag_enc_type(crypto_params->ucastcipherset);
	if (mcast_cipher)
		*mcast_cipher =
			cm_get_diag_enc_type(crypto_params->ucastcipherset);
	wlan_objmgr_vdev_release_ref(vdev, WLAN_MLME_CM_ID);

}

#ifdef WLAN_UNIT_TEST
static const char *cm_diag_get_persona(enum mgmt_bss_type persona)
{
	switch (persona) {
	CASE_RETURN_STRING(STA_PERSONA);
	CASE_RETURN_STRING(SAP_PERSONA);
	CASE_RETURN_STRING(P2P_CLIENT_PERSONA);
	CASE_RETURN_STRING(P2P_GO_PERSONA);
	CASE_RETURN_STRING(FTM_PERSONA);
	CASE_RETURN_STRING(MONITOR_PERSONA);
	CASE_RETURN_STRING(P2P_DEVICE_PERSONA);
	CASE_RETURN_STRING(NDI_PERSONA);
	CASE_RETURN_STRING(WDS_PERSONA);
	default:
		return "Unknown";
	}
}

static const char *cm_diag_get_auth_type_str(uint8_t auth_type)
{
	switch (auth_type) {
	CASE_RETURN_STRING(AUTH_OPEN);
	CASE_RETURN_STRING(AUTH_SHARED);
	CASE_RETURN_STRING(AUTH_WPA_EAP);
	CASE_RETURN_STRING(AUTH_WPA_PSK);
	CASE_RETURN_STRING(AUTH_WPA2_EAP);
	CASE_RETURN_STRING(AUTH_WPA2_PSK);
	CASE_RETURN_STRING(AUTH_WAPI_CERT);
	CASE_RETURN_STRING(AUTH_WAPI_PSK);
	default:
		return "Unknown";
	}
}

void cm_get_sta_cxn_info(struct wlan_objmgr_vdev *vdev,
			 char *buf, uint32_t buf_sz)
{
	struct qdf_mac_addr bss_peer_mac;
	struct wlan_ssid ssid;
	QDF_STATUS status;
	struct wlan_channel *des_chan;
	struct vdev_mlme_obj *vdev_mlme;
	struct wlan_crypto_params *crypto_params;
	qdf_freq_t oper_freq;
	int8_t rssi = 0;
	uint32_t nss, hw_mode;
	struct policy_mgr_conc_connection_info *conn_info;
	uint32_t i = 0, len = 0, max_cxn = 0;
	enum mgmt_ch_width ch_width;
	enum mgmt_dot11_mode dot11mode;
	enum mgmt_bss_type type;
	uint8_t authtype;
	enum mgmt_encrypt_type enctype;
	enum QDF_OPMODE opmode;
	struct wlan_objmgr_pdev *pdev;
	struct wlan_objmgr_psoc *psoc;

	pdev = wlan_vdev_get_pdev(vdev);
	if (!pdev)
		return;
	psoc = wlan_pdev_get_psoc(pdev);
	if (!psoc)
		return;

	qdf_mem_set(buf, buf_sz, '\0');

	status = wlan_vdev_get_bss_peer_mac(vdev, &bss_peer_mac);
	if (QDF_IS_STATUS_ERROR(status))
		return;

	len += qdf_scnprintf(buf + len, buf_sz - len,
			     "\n\tbssid: "QDF_MAC_ADDR_FMT,
			     QDF_MAC_ADDR_REF(bss_peer_mac.bytes));

	status = wlan_vdev_mlme_get_ssid(vdev, ssid.ssid, &ssid.length);
	if (QDF_IS_STATUS_ERROR(status))
		return;
	if (ssid.length > WLAN_SSID_MAX_LEN)
		ssid.length = WLAN_SSID_MAX_LEN;
	len += qdf_scnprintf(buf + len, buf_sz - len,
			     "\n\tssid: %.*s", ssid.length,
			     ssid.ssid);

	cm_get_rssi_snr_by_bssid(pdev, &bss_peer_mac, &rssi, NULL);
	len += qdf_scnprintf(buf + len, buf_sz - len,
			     "\n\trssi: %d", rssi);

	des_chan = wlan_vdev_mlme_get_des_chan(vdev);
	ch_width = cm_get_diag_ch_width(des_chan->ch_width);
	len += qdf_scnprintf(buf + len, buf_sz - len,
			     "\n\tbw: %s", cm_diag_get_ch_width_str(ch_width));
	dot11mode = cm_diag_dot11_mode_from_phy_mode(des_chan->ch_phymode);
	len += qdf_scnprintf(buf + len, buf_sz - len,
			     "\n\tdot11mode: %s",
			     cm_diag_get_dot11_mode_str(dot11mode));

	opmode = wlan_vdev_mlme_get_opmode(vdev);
	type = cm_get_diag_persona(opmode);
	len += qdf_scnprintf(buf + len, buf_sz - len,
			     "\n\tbss_type: %s", cm_diag_get_persona(type));

	oper_freq = wlan_get_operation_chan_freq(vdev);
	len += qdf_scnprintf(buf + len, buf_sz - len,
			     "\n\tch_freq: %d", oper_freq);

	vdev_mlme = wlan_vdev_mlme_get_cmpt_obj(vdev);
	if (!vdev_mlme) {
		mlme_err("vdev component object is NULL");
		return;
	}
	len += qdf_scnprintf(buf + len, buf_sz - len,
			     "\n\tQoS: %d",
			     vdev_mlme->ext_vdev_ptr->connect_info.qos_enabled);

	crypto_params = wlan_crypto_vdev_get_crypto_params(vdev);

	if (!crypto_params) {
		mlme_err("crypto params is null");
		return;
	}

	cm_diag_get_auth_type(&authtype, crypto_params->authmodeset,
			   crypto_params->key_mgmt,
			   crypto_params->ucastcipherset);
	len += qdf_scnprintf(buf + len, buf_sz - len,
			     "\n\tauth_type: %s",
			     cm_diag_get_auth_type_str(authtype));

	enctype = cm_get_diag_enc_type(crypto_params->ucastcipherset);
	len += qdf_scnprintf(buf + len, buf_sz - len,
			     "\n\tencry_type: %s",
			     cm_diag_get_encr_type_str(enctype));

	conn_info = policy_mgr_get_conn_info(&max_cxn);
	for (i = 0; i < max_cxn; i++)
		if ((conn_info->vdev_id == wlan_vdev_get_id(vdev)) &&
		    (conn_info->in_use))
			break;
	len += qdf_scnprintf(buf + len, buf_sz - len,
			     "\n\tmac: %d", conn_info->mac);
	nss = wlan_vdev_mlme_get_nss(vdev);
	len += qdf_scnprintf(buf + len, buf_sz - len,
			     "\n\torig_nss: %dx%d neg_nss: %dx%d",
			     conn_info->original_nss, conn_info->original_nss,
			     nss, nss);
	hw_mode = policy_mgr_is_current_hwmode_dbs(psoc);
	len += qdf_scnprintf(buf + len, buf_sz - len,
			     "\n\tis_current_hw_mode_dbs: %s",
			     ((hw_mode != 0) ? "yes" : "no"));
}
#endif
#endif

QDF_STATUS cm_connect_start_ind(struct wlan_objmgr_vdev *vdev,
				struct wlan_cm_connect_req *req)
{
	struct wlan_objmgr_psoc *psoc;
	struct rso_config *rso_cfg;
	struct cm_roam_values_copy src_cfg;

	if (!vdev || !req) {
		mlme_err("vdev or req is NULL");
		return QDF_STATUS_E_INVAL;
	}

	psoc = wlan_vdev_get_psoc(vdev);
	if (!psoc) {
		mlme_err("vdev_id: %d psoc not found", req->vdev_id);
		return QDF_STATUS_E_INVAL;
	}
	if (policy_mgr_is_sta_mon_concurrency(psoc))
		return QDF_STATUS_E_NOSUPPORT;

	rso_cfg = wlan_cm_get_rso_config(vdev);
	if (rso_cfg)
		rso_cfg->rsn_cap = req->crypto.rsn_caps;

	if (wlan_get_ext_ie_ptr_from_ext_id(HS20_OUI_TYPE,
					    HS20_OUI_TYPE_SIZE,
					    req->assoc_ie.ptr,
					    req->assoc_ie.len)) {
		src_cfg.bool_value = true;
		wlan_cm_roam_cfg_set_value(wlan_vdev_get_psoc(vdev),
					   wlan_vdev_get_id(vdev),
					   HS_20_AP, &src_cfg);
	}

	return QDF_STATUS_SUCCESS;
}

void cm_free_join_req(struct cm_vdev_join_req *join_req)
{
	if (!join_req)
		return;

	util_scan_free_cache_entry(join_req->entry);
	join_req->entry = NULL;
	qdf_mem_free(join_req->assoc_ie.ptr);
	qdf_mem_free(join_req->scan_ie.ptr);
	join_req->assoc_ie.ptr = NULL;
	join_req->scan_ie.ptr = NULL;
	qdf_mem_free(join_req);
}

QDF_STATUS cm_flush_join_req(struct scheduler_msg *msg)
{
	struct cm_vdev_join_req *join_req;

	if (!msg || !msg->bodyptr) {
		mlme_err("msg or msg->bodyptr is NULL");
		return QDF_STATUS_E_INVAL;
	}

	join_req = msg->bodyptr;
	cm_free_join_req(join_req);

	return QDF_STATUS_SUCCESS;
}

#ifdef WLAN_FEATURE_11BE_MLO
static void cm_fill_ml_info(struct cm_vdev_join_req *join_req)
{
	if (QDF_IS_STATUS_SUCCESS(
		util_scan_get_ml_partner_info(join_req->entry,
					      &join_req->partner_info))) {
		join_req->assoc_link_id = join_req->entry->ml_info.self_link_id;
		mlme_debug("Assoc link ID:%d", join_req->assoc_link_id);
	}
}
#else
static void cm_fill_ml_info(struct cm_vdev_join_req *join_req)
{
}
#endif

static QDF_STATUS
cm_copy_join_params(struct cm_vdev_join_req *join_req,
		    struct wlan_cm_vdev_connect_req *req)
{
	if (req->assoc_ie.len) {
		join_req->assoc_ie.ptr = qdf_mem_malloc(req->assoc_ie.len);
		if (!join_req->assoc_ie.ptr)
		return QDF_STATUS_E_NOMEM;
		qdf_mem_copy(join_req->assoc_ie.ptr, req->assoc_ie.ptr,
			     req->assoc_ie.len);
		join_req->assoc_ie.len = req->assoc_ie.len;
	}
	if (req->scan_ie.len) {
		join_req->scan_ie.ptr = qdf_mem_malloc(req->scan_ie.len);
		if (!join_req->scan_ie.ptr)
			return QDF_STATUS_E_NOMEM;
		join_req->scan_ie.len = req->scan_ie.len;
		qdf_mem_copy(join_req->scan_ie.ptr, req->scan_ie.ptr,
			     req->scan_ie.len);
	}
	join_req->entry = util_scan_copy_cache_entry(req->bss->entry);
	if (!join_req->entry)
		return QDF_STATUS_E_NOMEM;

	cm_fill_ml_info(join_req);

	if (req->owe_trans_ssid.length)
		join_req->owe_trans_ssid = req->owe_trans_ssid;

	join_req->vdev_id = req->vdev_id;
	join_req->cm_id = req->cm_id;
	join_req->force_rsne_override = req->force_rsne_override;
	join_req->is_wps_connection = req->is_wps_connection;
	join_req->is_osen_connection = req->is_osen_connection;

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS wlan_cm_send_connect_rsp(struct scheduler_msg *msg)
{
	struct cm_vdev_join_rsp *rsp;
	struct wlan_objmgr_vdev *vdev;
	QDF_STATUS status;
	struct wlan_objmgr_peer *peer;

	if (!msg || !msg->bodyptr)
		return QDF_STATUS_E_FAILURE;

	rsp = msg->bodyptr;

	vdev = wlan_objmgr_get_vdev_by_id_from_psoc(rsp->psoc,
						    rsp->connect_rsp.vdev_id,
						    WLAN_MLME_CM_ID);
	if (!vdev) {
		mlme_err(CM_PREFIX_FMT "vdev not found",
			 CM_PREFIX_REF(rsp->connect_rsp.vdev_id,
				       rsp->connect_rsp.cm_id));
		wlan_cm_free_connect_rsp(rsp);
		return QDF_STATUS_E_INVAL;
	}

	/*  check and delete bss peer in case of failure */
	if (QDF_IS_STATUS_ERROR(rsp->connect_rsp.connect_status)) {
		peer = wlan_objmgr_vdev_try_get_bsspeer(vdev,
							WLAN_MLME_CM_ID);
		if (peer) {
			cm_send_bss_peer_delete_req(vdev);
			wlan_objmgr_peer_release_ref(peer, WLAN_MLME_CM_ID);
		}
	}
	cm_csr_connect_rsp(vdev, rsp);
	if (rsp->connect_rsp.is_reassoc)
		status = wlan_cm_reassoc_rsp(vdev, &rsp->connect_rsp);
	else
		status = wlan_cm_connect_rsp(vdev, &rsp->connect_rsp);
	wlan_objmgr_vdev_release_ref(vdev, WLAN_MLME_CM_ID);

	wlan_cm_free_connect_rsp(rsp);

	return status;
}

#define FILS_HLP_OUI_TYPE "\x5"
#define FILS_HLP_OUI_LEN 1

static void
cm_update_hlp_data_from_assoc_ie(struct wlan_objmgr_vdev *vdev,
				 struct wlan_cm_vdev_connect_req *req)
{
	const uint8_t *hlp_ext_ie;
	const uint8_t *fragment_ie;

	/* clear hlp IE */
	cm_update_hlp_info(vdev, NULL, 0, true);

	hlp_ext_ie = wlan_get_ext_ie_ptr_from_ext_id(FILS_HLP_OUI_TYPE,
						     FILS_HLP_OUI_LEN,
						     req->assoc_ie.ptr,
						     req->assoc_ie.len);
	if (hlp_ext_ie)
		cm_update_hlp_info(vdev, hlp_ext_ie, hlp_ext_ie[1] + 2, false);

	fragment_ie = wlan_get_ie_ptr_from_eid(DOT11F_EID_FRAGMENT_IE,
					       req->assoc_ie.ptr,
					       req->assoc_ie.len);
	if (fragment_ie)
		cm_update_hlp_info(vdev, fragment_ie,
				   fragment_ie[1] + 2, false);
}

QDF_STATUS
cm_handle_connect_req(struct wlan_objmgr_vdev *vdev,
		      struct wlan_cm_vdev_connect_req *req)
{
	struct cm_vdev_join_req *join_req;
	struct scheduler_msg msg;
	QDF_STATUS status;
	struct wlan_objmgr_psoc *psoc;
	struct wlan_mlme_psoc_ext_obj *mlme_obj;

	if (!vdev || !req)
		return QDF_STATUS_E_FAILURE;

	psoc = wlan_vdev_get_psoc(vdev);
	if (!psoc) {
		mlme_err(CM_PREFIX_FMT "psoc not found",
			 CM_PREFIX_REF(req->vdev_id, req->cm_id));
		return QDF_STATUS_E_INVAL;
	}

	mlme_obj = mlme_get_psoc_ext_obj(psoc);
	if (!mlme_obj)
		return QDF_STATUS_E_INVAL;

	qdf_mem_zero(&msg, sizeof(msg));
	join_req = qdf_mem_malloc(sizeof(*join_req));
	if (!join_req)
		return QDF_STATUS_E_NOMEM;

	status = cm_copy_join_params(join_req, req);
	if (QDF_IS_STATUS_ERROR(status)) {
		mlme_err(CM_PREFIX_FMT "Failed to copy join req",
			 CM_PREFIX_REF(req->vdev_id, req->cm_id));
		cm_free_join_req(join_req);
		return QDF_STATUS_E_FAILURE;
	}

	cm_update_hlp_data_from_assoc_ie(vdev, req);

	status = cm_csr_handle_join_req(vdev, req, join_req, false);
	if (QDF_IS_STATUS_ERROR(status)) {
		mlme_err(CM_PREFIX_FMT "fail to fill params from legacy",
			 CM_PREFIX_REF(req->vdev_id, req->cm_id));
		cm_free_join_req(join_req);
		return QDF_STATUS_E_FAILURE;
	}

	mlme_debug(CM_PREFIX_FMT "HT cap %x",
		   CM_PREFIX_REF(req->vdev_id, req->cm_id), req->ht_caps);
	wlan_rec_conn_info(req->vdev_id, DEBUG_CONN_CONNECTING,
			   req->bss->entry->bssid.bytes,
			   req->bss->entry->neg_sec_info.key_mgmt,
			   req->bss->entry->channel.chan_freq);
	if (mlme_obj->cfg.obss_ht40.is_override_ht20_40_24g &&
	    !(req->ht_caps & WLAN_HTCAP_C_CHWIDTH40))
		join_req->force_24ghz_in_ht20 = true;

	msg.bodyptr = join_req;
	msg.type = CM_CONNECT_REQ;
	msg.flush_callback = cm_flush_join_req;

	status = scheduler_post_message(QDF_MODULE_ID_MLME,
					QDF_MODULE_ID_PE,
					QDF_MODULE_ID_PE, &msg);
	if (QDF_IS_STATUS_ERROR(status)) {
		mlme_err(CM_PREFIX_FMT "msg post fail",
			 CM_PREFIX_REF(req->vdev_id, req->cm_id));
		cm_free_join_req(join_req);
	}

	if (wlan_vdev_mlme_get_opmode(vdev) == QDF_STA_MODE)
		wlan_register_txrx_packetdump(OL_TXRX_PDEV_ID);

	return status;
}

#ifdef WLAN_FEATURE_11BE_MLO
/**
 * cm_set_peer_mld_info() - set mld_mac and is_assoc_peer flag
 * @req: cm_peer_create_req
 * @mld_mac: mld mac addr
 * @is_assoc_peer: is assoc peer
 *
 * Return: void
 */
static void cm_set_peer_mld_info(struct cm_peer_create_req *req,
				 struct qdf_mac_addr *mld_mac,
				 bool is_assoc_peer)
{
	if (req) {
		qdf_copy_macaddr(&req->mld_mac, mld_mac);
		req->is_assoc_peer = is_assoc_peer;
	}
}
#else
static void cm_set_peer_mld_info(struct cm_peer_create_req *req,
				 struct qdf_mac_addr *mld_mac,
				 bool is_assoc_peer)
{
}
#endif

QDF_STATUS
cm_send_bss_peer_create_req(struct wlan_objmgr_vdev *vdev,
			    struct qdf_mac_addr *peer_mac,
			    struct qdf_mac_addr *mld_mac,
			    bool is_assoc_peer)
{
	struct scheduler_msg msg;
	QDF_STATUS status;
	struct cm_peer_create_req *req;

	if (!vdev || !peer_mac)
		return QDF_STATUS_E_FAILURE;

	qdf_mem_zero(&msg, sizeof(msg));
	req = qdf_mem_malloc(sizeof(*req));

	if (!req)
		return QDF_STATUS_E_NOMEM;

	req->vdev_id = wlan_vdev_get_id(vdev);
	qdf_copy_macaddr(&req->peer_mac, peer_mac);
	cm_set_peer_mld_info(req, mld_mac, is_assoc_peer);
	msg.bodyptr = req;
	msg.type = CM_BSS_PEER_CREATE_REQ;

	status = scheduler_post_message(QDF_MODULE_ID_MLME,
					QDF_MODULE_ID_PE,
					QDF_MODULE_ID_PE, &msg);
	if (QDF_IS_STATUS_ERROR(status)) {
		mlme_err("vdev %d: post fail", req->vdev_id);
		qdf_mem_free(req);
	}

	return status;

}

#ifdef WLAN_FEATURE_FILS_SK
static inline bool is_fils_connection(struct wlan_cm_connect_resp *resp)
{
	return resp->is_fils_connection;
}
#else
static inline bool is_fils_connection(struct wlan_cm_connect_resp *resp)
{
	return false;
}
#endif

static void cm_process_connect_complete(struct wlan_objmgr_psoc *psoc,
					struct wlan_objmgr_pdev *pdev,
					struct wlan_objmgr_vdev *vdev,
					struct wlan_cm_connect_resp *rsp)
{
	uint8_t vdev_id = wlan_vdev_get_id(vdev);
	int32_t ucast_cipher, akm;
	uint32_t key_interval;
	struct element_info *bcn_probe_rsp = &rsp->connect_ies.bcn_probe_rsp;

	if (bcn_probe_rsp->ptr &&
	    bcn_probe_rsp->len > sizeof(struct wlan_frame_hdr)) {
		cm_update_prev_ap_ie(psoc, vdev_id,
				     bcn_probe_rsp->len -
				     sizeof(struct wlan_frame_hdr),
				     bcn_probe_rsp->ptr +
				     sizeof(struct wlan_frame_hdr));
	}
	akm = wlan_crypto_get_param(vdev, WLAN_CRYPTO_PARAM_KEY_MGMT);
	if (QDF_HAS_PARAM(akm, WLAN_CRYPTO_KEY_MGMT_FT_SAE)) {
		mlme_debug("Update the MDID in PMK cache for FT-SAE case");
		cm_update_pmk_cache_ft(psoc, vdev_id);
	}

	cm_update_owe_info(vdev, rsp, vdev_id);
	cm_csr_set_joined(vdev_id);
	cm_csr_send_set_ie(vdev);

	ucast_cipher = wlan_crypto_get_param(vdev,
					     WLAN_CRYPTO_PARAM_UCAST_CIPHER);
	if (!rsp->is_wps_connection &&
	    (is_fils_connection(rsp) || !ucast_cipher ||
	    QDF_HAS_PARAM(ucast_cipher, WLAN_CRYPTO_CIPHER_NONE) ==
			  ucast_cipher ||
	    QDF_HAS_PARAM(ucast_cipher, WLAN_CRYPTO_CIPHER_WEP_40) ||
	    QDF_HAS_PARAM(ucast_cipher, WLAN_CRYPTO_CIPHER_WEP_104) ||
	    QDF_HAS_PARAM(ucast_cipher, WLAN_CRYPTO_CIPHER_WEP))) {
		cm_csr_set_ss_none(vdev_id);
		cm_roam_start_init_on_connect(pdev, vdev_id);
	} else {
		if (rsp->is_wps_connection)
			key_interval =
				WAIT_FOR_WPS_KEY_TIMEOUT_PERIOD;
		else
			key_interval =
				WAIT_FOR_KEY_TIMEOUT_PERIOD;
		cm_update_wait_for_key_timer(vdev, vdev_id, key_interval);
	}

}

QDF_STATUS
cm_connect_complete_ind(struct wlan_objmgr_vdev *vdev,
			struct wlan_cm_connect_resp *rsp)
{
	uint8_t vdev_id;
	struct wlan_objmgr_pdev *pdev;
	struct wlan_objmgr_psoc *psoc;
	enum QDF_OPMODE op_mode;

	if (!vdev || !rsp) {
		mlme_err("vdev or rsp is NULL");
		return QDF_STATUS_E_INVAL;
	}

	vdev_id = wlan_vdev_get_id(vdev);
	op_mode = wlan_vdev_mlme_get_opmode(vdev);
	pdev = wlan_vdev_get_pdev(vdev);
	if (!pdev) {
		mlme_err(CM_PREFIX_FMT "pdev not found",
			 CM_PREFIX_REF(vdev_id, rsp->cm_id));
		return QDF_STATUS_E_INVAL;
	}
	psoc = wlan_pdev_get_psoc(pdev);
	if (!psoc) {
		mlme_err(CM_PREFIX_FMT "psoc not found",
			 CM_PREFIX_REF(vdev_id, rsp->cm_id));
		return QDF_STATUS_E_INVAL;
	}

	cm_csr_connect_done_ind(vdev, rsp);

	cm_connect_info(vdev, QDF_IS_STATUS_SUCCESS(rsp->connect_status) ?
			true : false, &rsp->bssid, &rsp->ssid,
			rsp->freq);

	if (QDF_IS_STATUS_SUCCESS(rsp->connect_status)) {
		policy_mgr_incr_active_session(psoc, op_mode, vdev_id);
		cm_process_connect_complete(psoc, pdev, vdev, rsp);
		wlan_tdls_notify_sta_connect(vdev_id,
					     mlme_get_tdls_chan_switch_prohibited(vdev),
					     mlme_get_tdls_prohibited(vdev),
					     vdev);
		wlan_p2p_status_connect(vdev);
	}

	if (op_mode == QDF_STA_MODE)
		wlan_cm_roam_state_change(pdev, vdev_id, WLAN_ROAM_INIT,
					  REASON_CONNECT);

	return QDF_STATUS_SUCCESS;
}

#ifdef WLAN_FEATURE_FILS_SK
static inline void cm_free_fils_ie(struct wlan_connect_rsp_ies *connect_ie)
{
	if (!connect_ie->fils_ie)
		return;

	if (connect_ie->fils_ie->fils_pmk) {
		qdf_mem_zero(connect_ie->fils_ie->fils_pmk,
			     connect_ie->fils_ie->fils_pmk_len);
		qdf_mem_free(connect_ie->fils_ie->fils_pmk);
	}
	qdf_mem_zero(connect_ie->fils_ie, sizeof(*connect_ie->fils_ie));
	qdf_mem_free(connect_ie->fils_ie);
}
#else
static inline void cm_free_fils_ie(struct wlan_connect_rsp_ies *connect_ie)
{
}
#endif

#ifdef FEATURE_WLAN_ESE
static void cm_free_tspec_ie(struct cm_vdev_join_rsp *rsp)
{
	qdf_mem_free(rsp->tspec_ie.ptr);
	rsp->tspec_ie.ptr = NULL;
	rsp->tspec_ie.len = 0;
}

#else
static void cm_free_tspec_ie(struct cm_vdev_join_rsp *rsp)
{}
#endif

#ifdef WLAN_FEATURE_ROAM_OFFLOAD
static void cm_free_roaming_info(struct wlan_cm_connect_resp *connect_rsp)
{
	qdf_mem_free(connect_rsp->roaming_info);
	connect_rsp->roaming_info = NULL;
}
#else
static inline void
cm_free_roaming_info(struct wlan_cm_connect_resp *connect_rsp)
{}
#endif

void wlan_cm_free_connect_rsp(struct cm_vdev_join_rsp *rsp)
{
	struct wlan_connect_rsp_ies *connect_ie =
						&rsp->connect_rsp.connect_ies;

	qdf_mem_free(connect_ie->assoc_req.ptr);
	qdf_mem_free(connect_ie->bcn_probe_rsp.ptr);
	qdf_mem_free(connect_ie->assoc_rsp.ptr);
	cm_free_fils_ie(connect_ie);
	cm_free_tspec_ie(rsp);
	qdf_mem_free(rsp->ric_resp_ie.ptr);
	cm_free_roaming_info(&rsp->connect_rsp);
	qdf_mem_zero(rsp, sizeof(*rsp));
	qdf_mem_free(rsp);
}

bool cm_is_vdevid_connected(struct wlan_objmgr_pdev *pdev, uint8_t vdev_id)
{
	struct wlan_objmgr_vdev *vdev;
	bool connected;
	enum QDF_OPMODE opmode;

	vdev = wlan_objmgr_get_vdev_by_id_from_pdev(pdev, vdev_id,
						    WLAN_MLME_CM_ID);
	if (!vdev) {
		mlme_err("vdev %d: vdev not found", vdev_id);
		return false;
	}
	opmode = wlan_vdev_mlme_get_opmode(vdev);
	if (opmode != QDF_STA_MODE && opmode != QDF_P2P_CLIENT_MODE) {
		wlan_objmgr_vdev_release_ref(vdev, WLAN_MLME_CM_ID);
		return false;
	}
	connected = cm_is_vdev_connected(vdev);
	wlan_objmgr_vdev_release_ref(vdev, WLAN_MLME_CM_ID);

	return connected;
}

bool cm_is_vdevid_active(struct wlan_objmgr_pdev *pdev, uint8_t vdev_id)
{
	struct wlan_objmgr_vdev *vdev;
	bool active;
	enum QDF_OPMODE opmode;

	vdev = wlan_objmgr_get_vdev_by_id_from_pdev(pdev, vdev_id,
						    WLAN_MLME_CM_ID);
	if (!vdev) {
		mlme_err("vdev %d: vdev not found", vdev_id);
		return false;
	}
	opmode = wlan_vdev_mlme_get_opmode(vdev);
	if (opmode != QDF_STA_MODE && opmode != QDF_P2P_CLIENT_MODE) {
		wlan_objmgr_vdev_release_ref(vdev, WLAN_MLME_CM_ID);
		return false;
	}
	active = cm_is_vdev_active(vdev);
	wlan_objmgr_vdev_release_ref(vdev, WLAN_MLME_CM_ID);

	return active;
}
