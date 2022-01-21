/*
 * Copyright (c) 2019-2021 The Linux Foundation. All rights reserved.
 * Copyright (c) 2021 Qualcomm Innovation Center, Inc. All rights reserved.
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

#include <target_if_cfr.h>
#include <wlan_tgt_def_config.h>
#include <target_type.h>
#include <hif_hw_version.h>
#include <target_if.h>
#include <wlan_lmac_if_def.h>
#include <wlan_osif_priv.h>
#include <init_deinit_lmac.h>
#include <wlan_cfr_utils_api.h>
#include <wlan_objmgr_pdev_obj.h>
#include <target_if_cfr_enh.h>
#ifdef CFR_USE_FIXED_FOLDER
#include "target_if_cfr_6490.h"
#include "target_if_cfr_adrastea.h"
#include "wlan_reg_services_api.h"
#else
#include <target_if_cfr_dbr.h>
#endif

int target_if_cfr_stop_capture(struct wlan_objmgr_pdev *pdev,
			       struct wlan_objmgr_peer *peer)
{
	struct peer_cfr *pe;
	struct peer_cfr_params param = {0};
	struct wmi_unified *pdev_wmi_handle = NULL;
	struct wlan_objmgr_vdev *vdev = {0};
	struct pdev_cfr *pdev_cfrobj;
	int retv = 0;

	pe = wlan_objmgr_peer_get_comp_private_obj(peer, WLAN_UMAC_COMP_CFR);
	if (pe == NULL)
		return -EINVAL;

	pdev_wmi_handle = lmac_get_pdev_wmi_handle(pdev);
	if (!pdev_wmi_handle) {
		cfr_err("pdev wmi handle NULL");
		return -EINVAL;
	}
	vdev = wlan_peer_get_vdev(peer);

	qdf_mem_set(&param, sizeof(param), 0);

	param.request = PEER_CFR_CAPTURE_DISABLE;
	param.macaddr = wlan_peer_get_macaddr(peer);
	param.vdev_id = wlan_vdev_get_id(vdev);

	param.periodicity = pe->period;
	param.bandwidth = pe->bandwidth;
	param.capture_method = pe->capture_method;

	retv = wmi_unified_send_peer_cfr_capture_cmd(pdev_wmi_handle, &param);

	pdev_cfrobj = wlan_objmgr_pdev_get_comp_private_obj(pdev,
							    WLAN_UMAC_COMP_CFR);
	if (!pdev_cfrobj) {
		cfr_err("pdev object for CFR is null");
		return -EINVAL;
	}
	cfr_err("CFR capture stats for this capture:");
	cfr_err("DBR event count = %llu, Tx event count = %llu "
		"Release count = %llu",
		pdev_cfrobj->dbr_evt_cnt, pdev_cfrobj->tx_evt_cnt,
		pdev_cfrobj->release_cnt);
	cfr_err("tx_peer_status_cfr_fail = %llu",
		pdev_cfrobj->tx_peer_status_cfr_fail = 0);
	cfr_err("tx_evt_status_cfr_fail = %llu",
		pdev_cfrobj->tx_evt_status_cfr_fail);
	cfr_err("tx_dbr_cookie_lookup_fail = %llu",
		pdev_cfrobj->tx_dbr_cookie_lookup_fail);

	pdev_cfrobj->dbr_evt_cnt = 0;
	pdev_cfrobj->tx_evt_cnt  = 0;
	pdev_cfrobj->release_cnt = 0;
	pdev_cfrobj->tx_peer_status_cfr_fail = 0;
	pdev_cfrobj->tx_evt_status_cfr_fail = 0;
	pdev_cfrobj->tx_dbr_cookie_lookup_fail = 0;

	return retv;
}

int target_if_cfr_start_capture(struct wlan_objmgr_pdev *pdev,
				struct wlan_objmgr_peer *peer,
				struct cfr_capture_params *cfr_params)
{
	struct peer_cfr_params param = {0};
	struct wmi_unified *pdev_wmi_handle = NULL;
	struct wlan_objmgr_vdev *vdev;
	int retv = 0;

	pdev_wmi_handle = lmac_get_pdev_wmi_handle(pdev);
	if (!pdev_wmi_handle) {
		cfr_err("pdev wmi handle NULL");
		return -EINVAL;
	}
	vdev = wlan_peer_get_vdev(peer);
	qdf_mem_set(&param, sizeof(param), 0);

	param.request = PEER_CFR_CAPTURE_ENABLE;
	param.macaddr = wlan_peer_get_macaddr(peer);
	param.vdev_id = wlan_vdev_get_id(vdev);

	param.periodicity = cfr_params->period;
	param.bandwidth = cfr_params->bandwidth;
	param.capture_method = cfr_params->method;

	retv = wmi_unified_send_peer_cfr_capture_cmd(pdev_wmi_handle, &param);
	return retv;
}

#ifdef ENABLE_HOST_TO_TARGET_CONVERSION
int target_if_cfr_periodic_peer_cfr_enable(struct wlan_objmgr_pdev *pdev,
					   uint32_t param_value)
{
	struct pdev_params pparam;
	uint32_t pdev_id;
	struct wmi_unified *pdev_wmi_handle = NULL;

	pdev_id = wlan_objmgr_pdev_get_pdev_id(pdev);
	if (pdev_id < 0)
		return -EINVAL;

	pdev_wmi_handle = lmac_get_pdev_wmi_handle(pdev);
	if (!pdev_wmi_handle) {
		cfr_err("pdev wmi handle NULL");
		return -EINVAL;
	}
	qdf_mem_set(&pparam, sizeof(pparam), 0);
	pparam.param_id = wmi_pdev_param_per_peer_prd_cfr_enable;
	pparam.param_value = param_value;

	return wmi_unified_pdev_param_send(pdev_wmi_handle,
					   &pparam, pdev_id);
}
#else
int target_if_cfr_periodic_peer_cfr_enable(struct wlan_objmgr_pdev *pdev,
					   uint32_t param_value)
{
	struct pdev_params pparam;
	uint32_t pdev_id;
	struct wmi_unified *pdev_wmi_handle = NULL;

	pdev_id = wlan_objmgr_pdev_get_pdev_id(pdev);
	if (pdev_id < 0)
		return -EINVAL;

	pdev_wmi_handle = lmac_get_pdev_wmi_handle(pdev);
	if (!pdev_wmi_handle) {
		cfr_err("pdev wmi handle NULL");
		return -EINVAL;
	}
	qdf_mem_set(&pparam, sizeof(pparam), 0);
	pparam.param_id = WMI_PDEV_PARAM_PER_PEER_PERIODIC_CFR_ENABLE;
	pparam.param_value = param_value;

	return wmi_unified_pdev_param_send(pdev_wmi_handle,
					   &pparam, pdev_id);
}
#endif

int target_if_cfr_enable_cfr_timer(struct wlan_objmgr_pdev *pdev,
				   uint32_t cfr_timer)
{
	struct pdev_cfr *pa;
	int retval;

	pa = wlan_objmgr_pdev_get_comp_private_obj(pdev, WLAN_UMAC_COMP_CFR);
	if (pa == NULL)
		return QDF_STATUS_E_FAILURE;

	if (!cfr_timer) {
	     /* disable periodic cfr capture */
		retval =
	target_if_cfr_periodic_peer_cfr_enable(pdev,
					       WMI_HOST_PEER_CFR_TIMER_DISABLE);

		if (retval == QDF_STATUS_SUCCESS)
			pa->cfr_timer_enable = 0;
	} else {
	    /* enable periodic cfr capture (default base timer is 10ms ) */
		retval =
	target_if_cfr_periodic_peer_cfr_enable(pdev,
					       WMI_HOST_PEER_CFR_TIMER_ENABLE);

		if (retval == QDF_STATUS_SUCCESS)
			pa->cfr_timer_enable = 1;
	}

	return retval;
}

int target_if_cfr_get_target_type(struct wlan_objmgr_psoc *psoc)
{
	uint32_t target_type = 0;
	struct wlan_lmac_if_target_tx_ops *target_type_tx_ops;
	struct wlan_lmac_if_tx_ops *tx_ops;

	tx_ops = wlan_psoc_get_lmac_if_txops(psoc);
	if (!tx_ops) {
		cfr_err("tx_ops is NULL");
		return target_type;
	}
	target_type_tx_ops = &tx_ops->target_tx_ops;

	if (target_type_tx_ops->tgt_get_tgt_type)
		target_type = target_type_tx_ops->tgt_get_tgt_type(psoc);

	return target_type;
}

void target_if_cfr_fill_header(struct csi_cfr_header *hdr,
			       bool is_wifi_2_0,
			       uint32_t target_type,
			       bool is_rcc)
{
	hdr->cmn.start_magic_num = 0xDEADBEAF;
	hdr->cmn.vendorid = 0x8cfdf0;
	hdr->cmn.pltform_type = CFR_PLATFORM_TYPE_ARM;
	hdr->cmn.cfr_metadata_len = CFR_META_DATA_LEN;
	hdr->cmn.cfr_data_version = CFR_DATA_VERSION_1;

	if (is_wifi_2_0) {
		hdr->cmn.cfr_metadata_version  = CFR_META_VERSION_1;
		if (target_type == TARGET_TYPE_IPQ4019)
			hdr->cmn.chip_type = CFR_CAPTURE_RADIO_DAKOTA;
		else if (target_type == TARGET_TYPE_QCA9888)
			hdr->cmn.chip_type = CFR_CAPTURE_RADIO_BESRA;
		else if (target_type == TARGET_TYPE_QCA9984)
			hdr->cmn.chip_type = CFR_CAPTURE_RADIO_CASCADE;
		else
			hdr->cmn.chip_type = CFR_CAPTURE_RADIO_NONE;
	} else if (target_type == TARGET_TYPE_QCA8074V2) {
		hdr->cmn.cfr_metadata_version = CFR_META_VERSION_6;
		hdr->cmn.chip_type = CFR_CAPTURE_RADIO_HKV2;
	} else if (target_type == TARGET_TYPE_QCA9574) {
		hdr->cmn.cfr_metadata_version = CFR_META_VERSION_6;
		hdr->cmn.chip_type = CFR_CAPTURE_RADIO_ALDER;
	} else {
		if (target_type == TARGET_TYPE_QCN9000 ||
		    target_type == TARGET_TYPE_QCN9224 ||
		    target_type == TARGET_TYPE_QCA6490 ||
		    target_type == TARGET_TYPE_QCA6750 ||
		    target_type == TARGET_TYPE_KIWI)
			hdr->cmn.cfr_metadata_version = CFR_META_VERSION_7;
		else if ((target_type == TARGET_TYPE_QCA6018) ||
			 ((target_type == TARGET_TYPE_QCA5018) && (!is_rcc)))
			hdr->cmn.cfr_metadata_version = CFR_META_VERSION_5;
		else
			hdr->cmn.cfr_metadata_version = CFR_META_VERSION_3;

		if (target_type == TARGET_TYPE_QCN9000)
			hdr->cmn.chip_type = CFR_CAPTURE_RADIO_PINE;
		else if (target_type == TARGET_TYPE_QCA5018)
			hdr->cmn.chip_type = CFR_CAPTURE_RADIO_MAPLE;
		else if (target_type == TARGET_TYPE_QCN6122)
			hdr->cmn.chip_type = CFR_CAPTURE_RADIO_SPRUCE;
		else if (target_type == TARGET_TYPE_QCN9224)
			hdr->cmn.chip_type = CFR_CAPTURE_RADIO_WAIKIKI;
		else if (target_type == TARGET_TYPE_QCA6490)
			hdr->cmn.chip_type = CFR_CAPTURE_RADIO_HSP;
		else if (target_type == TARGET_TYPE_QCA6750)
			hdr->cmn.chip_type = CFR_CAPTURE_RADIO_MOSELLE;
		else if (target_type == TARGET_TYPE_KIWI)
			hdr->cmn.chip_type = CFR_CAPTURE_RADIO_KIWI;
		else
			hdr->cmn.chip_type = CFR_CAPTURE_RADIO_CYP;
	}
}

#ifdef CFR_USE_FIXED_FOLDER
static QDF_STATUS target_if_cfr_init_target(struct wlan_objmgr_psoc *psoc,
					    struct wlan_objmgr_pdev *pdev,
					    uint32_t target)
{
	struct pdev_cfr *cfr_pdev;
	struct psoc_cfr *cfr_psoc;
	struct wmi_unified *wmi_handle = NULL;
	bool cfr_capable;
	QDF_STATUS status;

	if (!psoc || !pdev) {
		cfr_err("null pdev or psoc");
		return QDF_STATUS_E_FAILURE;
	}

	cfr_pdev = wlan_objmgr_pdev_get_comp_private_obj(pdev,
							 WLAN_UMAC_COMP_CFR);
	if (!cfr_pdev) {
		cfr_err("null pdev cfr");
		return QDF_STATUS_E_FAILURE;
	}

	cfr_psoc = wlan_objmgr_psoc_get_comp_private_obj(psoc,
							 WLAN_UMAC_COMP_CFR);

	if (!cfr_psoc) {
		cfr_err("null psoc cfr");
		return QDF_STATUS_E_FAILURE;
	}

	wmi_handle = lmac_get_pdev_wmi_handle(pdev);
	if (!wmi_handle) {
		cfr_err("null wmi handle");
		return QDF_STATUS_E_FAILURE;
	}

	if (wlan_cfr_is_feature_disabled(pdev)) {
		cfr_pdev->is_cfr_capable = 0;
		cfr_psoc->is_cfr_capable = 0;
		cfr_info("cfr disabled");
		return QDF_STATUS_SUCCESS;
	}

	cfr_capable = wmi_service_enabled(wmi_handle,
					  wmi_service_cfr_capture_support);
	cfr_pdev->is_cfr_capable = cfr_capable;
	cfr_psoc->is_cfr_capable = cfr_capable;
	if (!cfr_capable) {
		cfr_err("FW doesn't support CFR");
		return QDF_STATUS_SUCCESS;
	}

	status = cfr_enh_init_pdev(psoc, pdev);
	if (target == TARGET_TYPE_QCA6490)
		cfr_pdev->chip_type = CFR_CAPTURE_RADIO_HSP;
	else if (target == TARGET_TYPE_QCA6750)
		cfr_pdev->chip_type = CFR_CAPTURE_RADIO_MOSELLE;
	else if (target == TARGET_TYPE_KIWI)
		cfr_pdev->chip_type = CFR_CAPTURE_RADIO_KIWI;

	return status;
}

static QDF_STATUS target_if_cfr_deinit_target(struct wlan_objmgr_psoc *psoc,
					      struct wlan_objmgr_pdev *pdev)
{
	struct pdev_cfr *pcfr;

	if (!psoc || !pdev) {
		cfr_err("null pdev or psoc");
		return QDF_STATUS_E_FAILURE;
	}

	pcfr = wlan_objmgr_pdev_get_comp_private_obj(pdev,
						     WLAN_UMAC_COMP_CFR);
	if (!pcfr) {
		cfr_err("null pdev cfr");
		return QDF_STATUS_E_FAILURE;
	}

	if (!pcfr->is_cfr_capable) {
		cfr_info("cfr disabled or FW not support");
		return QDF_STATUS_SUCCESS;
	}

	return cfr_enh_deinit_pdev(psoc, pdev);
}

QDF_STATUS
target_if_cfr_init_pdev(struct wlan_objmgr_psoc *psoc,
			struct wlan_objmgr_pdev *pdev)
{
	uint32_t target_type;
	QDF_STATUS status;

	target_type = target_if_cfr_get_target_type(psoc);

	if (target_type == TARGET_TYPE_QCA6490 ||
	    target_type == TARGET_TYPE_QCA6750 ||
	    target_type == TARGET_TYPE_KIWI) {
		status = target_if_cfr_init_target(psoc,
						   pdev, target_type);
	} else if (target_type == TARGET_TYPE_ADRASTEA) {
		status = cfr_adrastea_init_pdev(psoc, pdev);
	} else {
		cfr_info("unsupport chip");
		status = QDF_STATUS_SUCCESS;
	}

	return status;
}

QDF_STATUS
target_if_cfr_deinit_pdev(struct wlan_objmgr_psoc *psoc,
			  struct wlan_objmgr_pdev *pdev)
{
	uint32_t target_type;
	QDF_STATUS status;

	target_type = target_if_cfr_get_target_type(psoc);

	if (target_type == TARGET_TYPE_QCA6490 ||
	    target_type == TARGET_TYPE_QCA6750 ||
	    target_type == TARGET_TYPE_KIWI) {
		status = target_if_cfr_deinit_target(psoc, pdev);
	} else if (target_type == TARGET_TYPE_ADRASTEA) {
		status = cfr_adrastea_deinit_pdev(psoc, pdev);
	} else {
		cfr_info("unsupport chip");
		status = QDF_STATUS_SUCCESS;
	}

	return status;
}
#else
QDF_STATUS
target_if_cfr_init_pdev(struct wlan_objmgr_psoc *psoc,
			struct wlan_objmgr_pdev *pdev)
{
	uint32_t target_type;
	struct pdev_cfr *pa;
	struct psoc_cfr *cfr_sc;

	if (wlan_cfr_is_feature_disabled(pdev)) {
		cfr_err("cfr is disabled");
		return QDF_STATUS_E_NOSUPPORT;
	}

	pa = wlan_objmgr_pdev_get_comp_private_obj(pdev, WLAN_UMAC_COMP_CFR);
	if (pa == NULL)
		return QDF_STATUS_E_FAILURE;

	/* Reset unassociated entries for every init */
	qdf_mem_zero(&pa->unassoc_pool[0], MAX_CFR_ENABLED_CLIENTS *
		     sizeof(struct unassoc_pool_entry));

	cfr_sc = wlan_objmgr_psoc_get_comp_private_obj(psoc,
						       WLAN_UMAC_COMP_CFR);

	if (cfr_sc == NULL)
		return QDF_STATUS_E_FAILURE;

	target_type = target_if_cfr_get_target_type(psoc);

	if ((target_type == TARGET_TYPE_QCA8074V2) ||
	    (target_type == TARGET_TYPE_QCA9574)) {
		pa->is_cfr_capable = cfr_sc->is_cfr_capable;
		return cfr_dbr_init_pdev(psoc, pdev);
	} else if ((target_type == TARGET_TYPE_IPQ4019) ||
		   (target_type == TARGET_TYPE_QCA9984) ||
		   (target_type == TARGET_TYPE_QCA9888)) {

		pa->is_cfr_capable = cfr_sc->is_cfr_capable;

		return cfr_wifi2_0_init_pdev(psoc, pdev);
	} else if ((target_type == TARGET_TYPE_QCA6018) ||
		   (target_type == TARGET_TYPE_QCN9000) ||
		   (target_type == TARGET_TYPE_QCN6122) ||
		   (target_type == TARGET_TYPE_QCA5018)) {
		pa->is_cfr_capable = cfr_sc->is_cfr_capable;
		return cfr_enh_init_pdev(psoc, pdev);
	} else
		return QDF_STATUS_E_NOSUPPORT;
}

QDF_STATUS
target_if_cfr_deinit_pdev(struct wlan_objmgr_psoc *psoc,
			  struct wlan_objmgr_pdev *pdev)
{
	uint32_t target_type;

	if (wlan_cfr_is_feature_disabled(pdev)) {
		cfr_err("cfr is disabled");
		return QDF_STATUS_E_NOSUPPORT;
	}

	target_type = target_if_cfr_get_target_type(psoc);

	if ((target_type == TARGET_TYPE_QCA8074V2) ||
	    (target_type == TARGET_TYPE_QCA9574)) {
		return cfr_dbr_deinit_pdev(psoc, pdev);
	} else if ((target_type == TARGET_TYPE_IPQ4019) ||
		   (target_type == TARGET_TYPE_QCA9984) ||
		   (target_type == TARGET_TYPE_QCA9888)) {

		return cfr_wifi2_0_deinit_pdev(psoc, pdev);
	} else if ((target_type == TARGET_TYPE_QCA6018) ||
		   (target_type == TARGET_TYPE_QCN9000) ||
		   (target_type == TARGET_TYPE_QCN6122) ||
		   (target_type == TARGET_TYPE_QCA5018)) {
		return cfr_enh_deinit_pdev(psoc, pdev);
	} else
		return QDF_STATUS_E_NOSUPPORT;
}
#endif

#ifdef WLAN_ENH_CFR_ENABLE
#if defined(QCA_WIFI_QCA6490) || defined(QCA_WIFI_KIWI)
static uint8_t target_if_cfr_get_mac_id(struct wlan_objmgr_pdev *pdev)
{
	struct wlan_objmgr_vdev *vdev;
	struct wlan_channel *bss_chan;
	struct pdev_cfr *pcfr;
	uint8_t mac_id = 0;

	if (!pdev) {
		cfr_err("null pdev");
		return mac_id;
	}

	mac_id = wlan_objmgr_pdev_get_pdev_id(pdev);
	pcfr = wlan_objmgr_pdev_get_comp_private_obj(pdev, WLAN_UMAC_COMP_CFR);
	if (!pcfr)  {
		cfr_err("null pcfr");
		return mac_id;
	}

	if (pcfr->rcc_param.vdev_id == CFR_INVALID_VDEV_ID)
		return mac_id;

	vdev = wlan_objmgr_get_vdev_by_id_from_pdev(pdev,
						    pcfr->rcc_param.vdev_id,
						    WLAN_CFR_ID);
	if (!vdev) {
		cfr_err("null vdev");
		return mac_id;
	}

	bss_chan = wlan_vdev_mlme_get_bss_chan(vdev);
	if (!bss_chan) {
		cfr_info("null bss chan");
		wlan_objmgr_vdev_release_ref(vdev, WLAN_CFR_ID);
		return mac_id;
	}

	cfr_debug("bss freq %d", bss_chan->ch_freq);
	if (wlan_reg_is_24ghz_ch_freq(bss_chan->ch_freq))
		mac_id = CFR_MAC_ID_24G;
	else
		mac_id = CFR_MAC_ID_5G;

	pcfr->rcc_param.srng_id = mac_id;
	wlan_objmgr_vdev_release_ref(vdev, WLAN_CFR_ID);

	return mac_id;
}

static uint8_t target_if_cfr_get_pdev_id(struct wlan_objmgr_pdev *pdev)
{
	return target_if_cfr_get_mac_id(pdev);
}
#else
static uint8_t target_if_cfr_get_pdev_id(struct wlan_objmgr_pdev *pdev)
{
	return wlan_objmgr_pdev_get_pdev_id(pdev);
}
#endif /* QCA_WIFI_QCA6490 || QCA_WIFI_KIWI */

QDF_STATUS target_if_cfr_config_rcc(struct wlan_objmgr_pdev *pdev,
				    struct cfr_rcc_param *rcc_info)
{
	QDF_STATUS status;
	struct wmi_unified *pdev_wmi_handle = NULL;

	pdev_wmi_handle = lmac_get_pdev_wmi_handle(pdev);
	if (!pdev_wmi_handle) {
		cfr_err("pdev_wmi_handle is null");
		return QDF_STATUS_E_NULL_VALUE;
	}

	rcc_info->pdev_id = target_if_cfr_get_pdev_id(pdev);
	rcc_info->num_grp_tlvs =
		count_set_bits(rcc_info->modified_in_curr_session);

	status = wmi_unified_send_cfr_rcc_cmd(pdev_wmi_handle, rcc_info);
	return status;
}

void target_if_cfr_default_ta_ra_config(struct cfr_rcc_param *rcc_info,
					bool allvalid, uint16_t reset_cfg)
{
	struct ta_ra_cfr_cfg *curr_cfg = NULL;
	int grp_id;
	unsigned long bitmap = reset_cfg;
	uint8_t def_mac[QDF_MAC_ADDR_SIZE] = {0xFF, 0xFF, 0xFF,
		0xFF, 0xFF, 0xFF};
	uint8_t null_mac[QDF_MAC_ADDR_SIZE] = {0x00, 0x00, 0x00,
		0x00, 0x00, 0x00};

	for (grp_id = 0; grp_id < MAX_TA_RA_ENTRIES; grp_id++) {
		if (qdf_test_bit(grp_id, &bitmap)) {
			curr_cfg = &rcc_info->curr[grp_id];
			qdf_mem_copy(curr_cfg->tx_addr,
				     null_mac, QDF_MAC_ADDR_SIZE);
			qdf_mem_copy(curr_cfg->tx_addr_mask,
				     def_mac, QDF_MAC_ADDR_SIZE);
			qdf_mem_copy(curr_cfg->rx_addr,
				     null_mac, QDF_MAC_ADDR_SIZE);
			qdf_mem_copy(curr_cfg->rx_addr_mask,
				     def_mac, QDF_MAC_ADDR_SIZE);
			curr_cfg->bw = 0xf;
			curr_cfg->nss = 0xff;
			curr_cfg->mgmt_subtype_filter = 0;
			curr_cfg->ctrl_subtype_filter = 0;
			curr_cfg->data_subtype_filter = 0;
			if (!allvalid) {
				curr_cfg->valid_ta = 0;
				curr_cfg->valid_ta_mask = 0;
				curr_cfg->valid_ra = 0;
				curr_cfg->valid_ra_mask = 0;
				curr_cfg->valid_bw_mask = 0;
				curr_cfg->valid_nss_mask = 0;
				curr_cfg->valid_mgmt_subtype = 0;
				curr_cfg->valid_ctrl_subtype = 0;
				curr_cfg->valid_data_subtype = 0;
			} else {
				curr_cfg->valid_ta = 1;
				curr_cfg->valid_ta_mask = 1;
				curr_cfg->valid_ra = 1;
				curr_cfg->valid_ra_mask = 1;
				curr_cfg->valid_bw_mask = 1;
				curr_cfg->valid_nss_mask = 1;
				curr_cfg->valid_mgmt_subtype = 1;
				curr_cfg->valid_ctrl_subtype = 1;
				curr_cfg->valid_data_subtype = 1;
			}
		}
	}
}
#endif

#ifdef WLAN_ENH_CFR_ENABLE
#ifdef CFR_USE_FIXED_FOLDER
static void target_if_enh_cfr_add_ops(struct wlan_lmac_if_tx_ops *tx_ops)
{
	tx_ops->cfr_tx_ops.cfr_subscribe_ppdu_desc =
				target_if_cfr_subscribe_ppdu_desc;
}
#else
static void target_if_enh_cfr_add_ops(struct wlan_lmac_if_tx_ops *tx_ops)
{
}
#endif /* CFR_USE_FIXED_FOLDER */
static void target_if_enh_cfr_tx_ops(struct wlan_lmac_if_tx_ops *tx_ops)
{
	tx_ops->cfr_tx_ops.cfr_config_rcc =
		target_if_cfr_config_rcc;
	tx_ops->cfr_tx_ops.cfr_start_lut_timer =
		target_if_cfr_start_lut_age_timer;
	tx_ops->cfr_tx_ops.cfr_stop_lut_timer =
		target_if_cfr_stop_lut_age_timer;
	tx_ops->cfr_tx_ops.cfr_default_ta_ra_cfg =
		target_if_cfr_default_ta_ra_config;
	tx_ops->cfr_tx_ops.cfr_dump_lut_enh =
		target_if_cfr_dump_lut_enh;
	tx_ops->cfr_tx_ops.cfr_rx_tlv_process =
		target_if_cfr_rx_tlv_process;
	tx_ops->cfr_tx_ops.cfr_update_global_cfg =
		target_if_cfr_update_global_cfg;
	target_if_enh_cfr_add_ops(tx_ops);
}
#else
static void target_if_enh_cfr_tx_ops(struct wlan_lmac_if_tx_ops *tx_ops)
{
}
#endif

void target_if_cfr_tx_ops_register(struct wlan_lmac_if_tx_ops *tx_ops)
{
	tx_ops->cfr_tx_ops.cfr_init_pdev =
		target_if_cfr_init_pdev;
	tx_ops->cfr_tx_ops.cfr_deinit_pdev =
		target_if_cfr_deinit_pdev;
	tx_ops->cfr_tx_ops.cfr_enable_cfr_timer =
		target_if_cfr_enable_cfr_timer;
	tx_ops->cfr_tx_ops.cfr_start_capture =
		target_if_cfr_start_capture;
	tx_ops->cfr_tx_ops.cfr_stop_capture =
		target_if_cfr_stop_capture;
	target_if_enh_cfr_tx_ops(tx_ops);
}

void target_if_cfr_set_cfr_support(struct wlan_objmgr_psoc *psoc,
				   uint8_t value)
{
	struct wlan_lmac_if_rx_ops *rx_ops;

	rx_ops = wlan_psoc_get_lmac_if_rxops(psoc);
	if (!rx_ops) {
		cfr_err("rx_ops is NULL");
		return;
	}
	if (rx_ops->cfr_rx_ops.cfr_support_set)
		rx_ops->cfr_rx_ops.cfr_support_set(psoc, value);
}

QDF_STATUS
target_if_cfr_set_capture_count_support(struct wlan_objmgr_psoc *psoc,
					uint8_t value)
{
	struct wlan_lmac_if_rx_ops *rx_ops;

	rx_ops = wlan_psoc_get_lmac_if_rxops(psoc);
	if (!rx_ops) {
		cfr_err("rx_ops is NULL");
		return QDF_STATUS_E_INVAL;
	}

	if (rx_ops->cfr_rx_ops.cfr_capture_count_support_set)
		return rx_ops->cfr_rx_ops.cfr_capture_count_support_set(
						psoc, value);

	return QDF_STATUS_E_INVAL;
}

QDF_STATUS
target_if_cfr_set_mo_marking_support(struct wlan_objmgr_psoc *psoc,
				     uint8_t value)
{
	struct wlan_lmac_if_rx_ops *rx_ops;

	rx_ops = wlan_psoc_get_lmac_if_rxops(psoc);
	if (!rx_ops) {
		cfr_err("rx_ops is NULL");
		return QDF_STATUS_E_INVAL;
	}

	if (rx_ops->cfr_rx_ops.cfr_mo_marking_support_set)
		return rx_ops->cfr_rx_ops.cfr_mo_marking_support_set(
						psoc, value);

	return QDF_STATUS_E_INVAL;
}

QDF_STATUS
target_if_cfr_set_aoa_for_rcc_support(struct wlan_objmgr_psoc *psoc,
				      uint8_t value)
{
	struct wlan_lmac_if_rx_ops *rx_ops;

	rx_ops = wlan_psoc_get_lmac_if_rxops(psoc);
	if (!rx_ops) {
		cfr_err("rx_ops is NULL");
		return QDF_STATUS_E_INVAL;
	}

	if (rx_ops->cfr_rx_ops.cfr_aoa_for_rcc_support_set)
		return rx_ops->cfr_rx_ops.cfr_aoa_for_rcc_support_set(
						psoc, value);

	return QDF_STATUS_E_INVAL;
}

void target_if_cfr_info_send(struct wlan_objmgr_pdev *pdev, void *head,
			     size_t hlen, void *data, size_t dlen, void *tail,
			     size_t tlen)
{
	struct wlan_objmgr_psoc *psoc;
	struct wlan_lmac_if_rx_ops *rx_ops;

	psoc = wlan_pdev_get_psoc(pdev);

	rx_ops = wlan_psoc_get_lmac_if_rxops(psoc);
	if (!rx_ops) {
		cfr_err("rx_ops is NULL");
		return;
	}
	if (rx_ops->cfr_rx_ops.cfr_info_send)
		rx_ops->cfr_rx_ops.cfr_info_send(pdev, head, hlen, data, dlen,
						 tail, tlen);
}
