/*
 * Copyright (c) 2020-2021, The Linux Foundation. All rights reserved.
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

/*
 * DOC: contains interface manager public api
 */
#include "wlan_objmgr_psoc_obj.h"
#include "wlan_objmgr_vdev_obj.h"
#include "wlan_if_mgr_public_struct.h"
#include "wlan_if_mgr_sta.h"
#include "wlan_if_mgr_roam.h"
#include "wlan_if_mgr_main.h"
#include "nan_ucfg_api.h"
#include "wlan_policy_mgr_api.h"
#include "wlan_p2p_ucfg_api.h"
#include "wlan_tdls_ucfg_api.h"
#include "wlan_tdls_api.h"
#include <wlan_cm_api.h>
#include <wlan_mlo_mgr_public_structs.h>
#include <wlan_mlo_mgr_cmn.h>
#include <wlan_cm_roam_api.h>

QDF_STATUS if_mgr_connect_start(struct wlan_objmgr_vdev *vdev,
				struct if_mgr_event_data *event_data)
{
	uint8_t sta_cnt, sap_cnt;
	struct wlan_objmgr_pdev *pdev;
	struct wlan_objmgr_psoc *psoc;
	enum QDF_OPMODE op_mode;
	uint8_t vdev_id_list[MAX_NUMBER_OF_CONC_CONNECTIONS], i;
	bool disable_nan = true;

	pdev = wlan_vdev_get_pdev(vdev);
	if (!pdev)
		return QDF_STATUS_E_FAILURE;

	psoc = wlan_pdev_get_psoc(pdev);
	if (!psoc)
		return QDF_STATUS_E_FAILURE;

	/*
	 * Disable NAN Discovery if incoming connection is P2P or if a STA
	 * connection already exists and if this is a case of STA+STA
	 * or SAP+STA concurrency
	 */
	sta_cnt = policy_mgr_get_mode_specific_conn_info(psoc, NULL,
							 vdev_id_list,
							 PM_STA_MODE);
	sap_cnt = policy_mgr_get_mode_specific_conn_info(psoc, NULL,
							 &vdev_id_list[sta_cnt],
							 PM_SAP_MODE);
	op_mode = wlan_vdev_mlme_get_opmode(vdev);
	if (op_mode == QDF_P2P_CLIENT_MODE || sap_cnt || sta_cnt) {
		for (i = 0; i < sta_cnt + sap_cnt; i++)
			if (vdev_id_list[i] == wlan_vdev_get_id(vdev))
				disable_nan = false;
		if (disable_nan)
			ucfg_nan_disable_concurrency(psoc);
	}

	/*
	 * STA+NDI concurrency gets preference over NDI+NDI. Disable
	 * first NDI in case an NDI+NDI concurrency exists if FW does
	 * not support 4 port concurrency of two NDI + NAN with STA.
	 */
	if (!ucfg_nan_is_sta_nan_ndi_4_port_allowed(psoc))
		ucfg_nan_check_and_disable_unsupported_ndi(psoc,
							   false);
	if (!wlan_vdev_mlme_is_mlo_link_vdev(vdev)) {
		/*
		 * In case of STA+STA concurrency, firmware might try to roam
		 * to same AP where host is trying to do association on the other
		 * STA iface. Roaming is disabled on all the ifaces to avoid
		 * this scenario.
		 */
		if_mgr_disable_roaming(pdev, vdev, RSO_CONNECT_START);
	}

	return QDF_STATUS_SUCCESS;
}

#ifdef WLAN_FEATURE_11BE_MLO
/*
 * if_mgr_get_mlo_and_legacy_sta_count() - get mlo and legacy sta count
 * also fills the freq and legacy/mlo list
 * @psoc: Objmgr psoc
 * @num_mlo: num mlo as output
 * @mlo_idx: mlo vdev index as output
 * @num_legacy: num legacy as output
 * @legacy_idx: legacy vdev index as output
 * @freq_list: freq list of each sta vdev
 * @vdev_id_list: vdev id list
 *
 * Return: void
 */
static void if_mgr_get_mlo_and_legacy_sta_count(struct wlan_objmgr_psoc *psoc,
						uint8_t *num_mlo,
						uint8_t *mlo_idx,
						uint8_t *num_legacy,
						uint8_t *legacy_idx,
						qdf_freq_t *freq_list,
						uint8_t *vdev_id_list)
{
	uint32_t sta_num = 0;
	uint8_t i;
	struct wlan_objmgr_vdev *temp_vdev;

	*num_mlo = 0;
	*num_legacy = 0;

	sta_num = policy_mgr_get_mode_specific_conn_info(psoc, freq_list,
							 vdev_id_list,
							 PM_STA_MODE);
	if (!sta_num)
		return;

	for (i = 0; i < sta_num; i++) {
		temp_vdev =
			wlan_objmgr_get_vdev_by_id_from_psoc(psoc,
							     vdev_id_list[i],
							     WLAN_IF_MGR_ID);
		if (!temp_vdev) {
			ifmgr_err("invalid vdev for id %d", vdev_id_list[i]);
			return;
		}

		if (wlan_vdev_mlme_is_mlo_vdev(temp_vdev))
			mlo_idx[(*num_mlo)++] = i;
		else
			legacy_idx[(*num_legacy)++] = i;

		wlan_objmgr_vdev_release_ref(temp_vdev, WLAN_IF_MGR_ID);
	}
}

static uint8_t
if_mgr_mlo_get_affected_links_for_sbs(struct wlan_objmgr_psoc *psoc,
				      uint8_t num_mlo, qdf_freq_t *freq_list,
				      uint8_t *vdev_id_list,
				      uint8_t *mlo_vdev_lst,
				      uint8_t *mlo_idx, qdf_freq_t freq)
{
	uint8_t i = 0;
	bool same_band_sta_allowed;

	/*
	 * STA freq:      ML STA combo:  SBS Action
	 * ---------------------------------------------------
	 * 2Ghz           2Ghz+5/6Ghz    Disable 2Ghz(Same MAC)
	 * 5Ghz           2Ghz+5/6Ghz    Disable 2.4Ghz if 5Ghz lead to SBS
	 *                               (SBS, same MAC) and same band STA
	 *                               allowed, else disable 5/6Ghz
	 *                               (NON SBS, same MAC)
	 * 5Ghz(lower)    5Ghz+6Ghz      Disable 5Ghz (NON SBS, same MAC)
	 * 5Ghz(higher)   5Ghz+6Ghz      Disable 6Ghz (NON SBS, Same MAC)
	 * 2Ghz           5Ghz+6Ghz      Disable Any
	 */

	/* If non-ML STA is 2.4Ghz disable 2.4Ghz if present OR disable any */
	if (wlan_reg_is_24ghz_ch_freq(freq)) {
		while (i < num_mlo) {
			if (wlan_reg_is_24ghz_ch_freq(freq_list[mlo_idx[i]])) {
				/* Affected ML STA link on 2.4Ghz */
				mlo_vdev_lst[0] = vdev_id_list[mlo_idx[i]];
				return 1;
			}
			/* Fill non effected vdev in list */
			mlo_vdev_lst[i] = vdev_id_list[mlo_idx[i]];
			i++;
		}
		/* No link affected return mlo_num to disable any */
		return i;
	}

	/* This mean non-ML STA is 5Ghz */

	/* check if ML STA is DBS */
	i = 0;
	while (i < num_mlo &&
	       !wlan_reg_is_24ghz_ch_freq(freq_list[mlo_idx[i]]))
		i++;

	same_band_sta_allowed = wlan_cm_same_band_sta_allowed(psoc);

	/*
	 * if ML STA is DBS ie 2.4Ghz link present and if same_band_sta_allowed
	 * is false, disable 5/6Ghz link to make sure we dont have all link
	 * on 5Ghz
	 */
	if (i < num_mlo && !same_band_sta_allowed)
		goto check_dbs_mlo;

	/* check if any link lead to SBS, so that we can disable the other*/
	i = 0;
	while (i < num_mlo &&
	       !policy_mgr_are_sbs_chan(psoc, freq, freq_list[mlo_idx[i]]))
		i++;

	/*
	 * if i < num_mlo then i is the SBS link, in this case disable the other
	 * non SBS link, this mean ML STA is 5+6 or 2+5/6.
	 */
	if (i < num_mlo) {
		i = 0;
		while (i < num_mlo) {
			if (!policy_mgr_are_sbs_chan(psoc, freq,
						     freq_list[mlo_idx[i]])) {
				/* Affected non SBS ML STA link */
				mlo_vdev_lst[0] = vdev_id_list[mlo_idx[i]];
				return 1;
			}
			/* Fill non effected vdev in list */
			mlo_vdev_lst[i] = vdev_id_list[mlo_idx[i]];
			i++;
		}
		/* All link lead to SBS, disable any, This should not happen */
		return i;
	}

check_dbs_mlo:
	/*
	 * None of the link can lead to SBS, i.e. its 2+ 5/6 ML STA in this case
	 * disable 5Ghz link.
	 */
	i = 0;
	while (i < num_mlo) {
		if (!wlan_reg_is_24ghz_ch_freq(freq_list[mlo_idx[i]])) {
			/* Affected 5/6Ghz ML STA link */
			mlo_vdev_lst[0] = vdev_id_list[mlo_idx[i]];
			return 1;
		}
		/* Fill non effected vdev in list */
		mlo_vdev_lst[i] = vdev_id_list[mlo_idx[i]];
		i++;
	}

	/* No link affected, This should not happen */
	return i;
}

/*
 * if_mgr_mlo_get_concurrent_num_links() - get MLO links which are affected
 * if no affected then return mlo num. Also fills the mlo_vdev_id to send.
 * @num_mlo: number of mlo vdev
 * @freq_list: freq list of all vdev
 * @vdev_id_list: vdev id list
 * @mlo_vdev_lst: mlo vdev list
 * @mlo_idx: mlo index
 * @freq: legacy sta freq
 *
 * Return: number of the affected links, else total link and mlo_vdev_id list.
 */
static uint8_t
if_mgr_mlo_get_concurrent_num_links(struct wlan_objmgr_vdev *vdev,
				    uint8_t num_mlo, qdf_freq_t *freq_list,
				    uint8_t *vdev_id_list,
				    uint8_t *mlo_vdev_lst,
				    uint8_t *mlo_idx, qdf_freq_t freq)
{
	uint8_t i = 0;
	struct wlan_objmgr_psoc *psoc = wlan_vdev_get_psoc(vdev);

	if (!psoc)
		return 0;

	while (i < num_mlo && (freq_list[mlo_idx[i]] != freq))
		i++;

	if (i < num_mlo) {
		/* if one link is SCC then no need to disable any link */
		ifmgr_debug("vdev %d: link vdev %d lead to SCC, STA freq %d ML freq %d, no need to disable link",
			    wlan_vdev_get_id(vdev), vdev_id_list[mlo_idx[i]],
			    freq, freq_list[mlo_idx[i]]);
		return 0;
	}

	if (policy_mgr_is_hw_sbs_capable(psoc))
		return if_mgr_mlo_get_affected_links_for_sbs(psoc, num_mlo,
							     freq_list,
							     vdev_id_list,
							     mlo_vdev_lst,
							     mlo_idx, freq);

	/*
	 * STA freq:      STA ML combo:  NON SBS Action:
	 * -------------------------------------------------
	 * 2Ghz           2Ghz+5/6Ghz    Disable 2Ghz (Same MAC)
	 * 5Ghz           2Ghz+5/6Ghz    Disable 5Ghz (Same MAC)
	 * 5Ghz           5Ghz+6Ghz      Disable Any of 5/6Ghz (Same MAC)
	 * 2Ghz           5Ghz+6Ghz      Disable Any
	 */
	/*
	 * Check if any of the link is on same MAC/band(for non SBS) as legacy
	 * STA's freq. Same MAC/band mean both are either 5Ghz/6Ghz/2.4Ghz
	 * OR both are non 2.4Ghz (ie one is 5Ghz and other is 6Ghz)
	 */
	i = 0;
	while (i < num_mlo &&
	       !(wlan_reg_is_same_band_freqs(freq_list[mlo_idx[i]], freq) ||
		 (!wlan_reg_is_24ghz_ch_freq(freq_list[mlo_idx[i]]) &&
		  !wlan_reg_is_24ghz_ch_freq(freq)))) {
		/* Fill non effected vdev in list */
		mlo_vdev_lst[i] = vdev_id_list[mlo_idx[i]];
		i++;
	}

	if (i < num_mlo) {
		/* affected MLO link on the same MAC/band with legacy STA */
		mlo_vdev_lst[0] = vdev_id_list[mlo_idx[i]];
		return 1;
	}

	/* No link affected return mlo_num to disable any */
	return i;
}

static void
if_mgr_handle_sap_plus_sta_mlo_connect(struct wlan_objmgr_psoc *psoc,
				       struct wlan_objmgr_vdev *vdev)
{
	uint32_t sap_num = 0;
	qdf_freq_t sap_freq_list[MAX_NUMBER_OF_CONC_CONNECTIONS] = {0};
	uint8_t sap_vdev_id_list[MAX_NUMBER_OF_CONC_CONNECTIONS] = {0};
	bool is_mlo_sbs;
	uint8_t mlo_vdev_lst[MAX_NUMBER_OF_CONC_CONNECTIONS] = {0};
	uint8_t num_mlo;
	uint8_t vdev_id = wlan_vdev_get_id(vdev);

	sap_num = policy_mgr_get_mode_specific_conn_info(psoc, sap_freq_list,
							 sap_vdev_id_list,
							 PM_SAP_MODE);

	ifmgr_debug("vdev %d: sap_num %d sap_chan %d", vdev_id, sap_num,
		    sap_freq_list[0]);
	if (sap_num != 1)
		return;

	if (!wlan_reg_is_24ghz_ch_freq(sap_freq_list[0]))
		return;
	is_mlo_sbs = policy_mgr_is_mlo_in_mode_sbs(psoc, PM_STA_MODE,
						   mlo_vdev_lst, &num_mlo);
	if (num_mlo < 2)
		return;

	ifmgr_debug("vdev %d: num_mlo %d is_mlo_sbs %d sap_chan %d", vdev_id,
		    num_mlo, is_mlo_sbs, sap_freq_list[0]);

	if (!is_mlo_sbs) {
		/*
		 * re-enable both link in case if this was roaming from sbs to
		 * dbs MLO, with sap on 2.4Ghz.
		 */
		if (wlan_cm_is_vdev_roaming(vdev))
			wlan_mlo_sta_mlo_concurency_set_link(vdev,
					MLO_LINK_FORCE_REASON_DISCONNECT,
					MLO_LINK_FORCE_MODE_NO_FORCE,
					num_mlo, mlo_vdev_lst);

		return;
	}

	/* If MLO STA is SBS and SAP is 2.4Ghz, Disable one of the links. */
	wlan_mlo_sta_mlo_concurency_set_link(vdev,
					     MLO_LINK_FORCE_REASON_CONNECT,
					     MLO_LINK_FORCE_MODE_ACTIVE_NUM,
					     num_mlo, mlo_vdev_lst);
}

static void
if_mgr_sta_mlo_concurency_on_connect(struct wlan_objmgr_psoc *psoc,
				     struct wlan_objmgr_vdev *vdev,
				     uint8_t num_mlo, uint8_t *mlo_idx,
				     uint8_t num_legacy, uint8_t *legacy_idx,
				     qdf_freq_t *freq_list,
				     uint8_t *vdev_id_list)
{
	qdf_freq_t freq = 0;
	struct wlan_channel *bss_chan;
	uint8_t vdev_id = wlan_vdev_get_id(vdev);
	uint8_t mlo_vdev_lst[MAX_NUMBER_OF_CONC_CONNECTIONS] = {0};
	uint8_t affected_links = 0;
	enum mlo_link_force_mode mode = MLO_LINK_FORCE_MODE_ACTIVE_NUM;

	/* Legacy STA doesn't exist, no need to change to link.*/
	if (!num_legacy) {
		/* Check if SAP exist and any link change is required */
		if_mgr_handle_sap_plus_sta_mlo_connect(psoc, vdev);
		return;
	}

	if (wlan_vdev_mlme_is_mlo_vdev(vdev)) {
		freq = freq_list[legacy_idx[0]];
	} else {
		bss_chan = wlan_vdev_mlme_get_bss_chan(vdev);
		if (bss_chan)
			freq = bss_chan->ch_freq;
	}
	ifmgr_debug("vdev %d: Freq %d (legacy vdev id %d), is MLO %d", vdev_id,
		    freq, legacy_idx[0], wlan_vdev_mlme_is_mlo_vdev(vdev));
	if (!freq)
		return;

	affected_links =
		if_mgr_mlo_get_concurrent_num_links(vdev, num_mlo, freq_list,
						    vdev_id_list, mlo_vdev_lst,
						    mlo_idx, freq);

	if (!affected_links) {
		ifmgr_debug("vdev %d: no affected link found", vdev_id);
		return;
	}

	/*
	 * If affected link is less than mlo_num, ie not all link are affected,
	 * send MLO_LINK_FORCE_MODE_INACTIVE.
	 */
	if (affected_links < num_mlo)
		mode = MLO_LINK_FORCE_MODE_INACTIVE;

	wlan_mlo_sta_mlo_concurency_set_link(vdev,
					     MLO_LINK_FORCE_REASON_CONNECT,
					     mode, affected_links,
					     mlo_vdev_lst);
}

static void
if_mgr_sta_mlo_concurency_on_disconnect(struct wlan_objmgr_vdev *vdev,
					uint8_t num_mlo, uint8_t *mlo_idx,
					uint8_t num_legacy,
					uint8_t *vdev_id_list)
{
	uint8_t i = 0;
	uint8_t mlo_vdev_list[MAX_NUMBER_OF_CONC_CONNECTIONS] = {0};

	/*
	 * If Legacy STA exist, no need to change to link.
	 * Only change when legasy sta is disconnected and
	 * only ML STA is present.
	 */
	if (num_legacy)
		return;

	/*
	 * On legacy STA disconnect if  MLO has >= 2 links, need to send
	 * MLO_LINK_FORCE_MODE_NO_FORCE for all MLO Vdevs for letting FW enable
	 * all the links.
	 */
	while (i < num_mlo) {
		mlo_vdev_list[i] = vdev_id_list[mlo_idx[i]];
		i++;
	}

	wlan_mlo_sta_mlo_concurency_set_link(vdev,
					     MLO_LINK_FORCE_REASON_DISCONNECT,
					     MLO_LINK_FORCE_MODE_NO_FORCE,
					     num_mlo, mlo_vdev_list);
}

static void
if_mgr_handle_sta_mlo_link_concurrency(struct wlan_objmgr_psoc *psoc,
				       struct wlan_objmgr_vdev *vdev,
				       bool is_connect)
{
	uint8_t num_mlo = 0, num_legacy = 0;
	uint8_t mlo_idx[MAX_NUMBER_OF_CONC_CONNECTIONS] = {0};
	uint8_t legacy_idx[MAX_NUMBER_OF_CONC_CONNECTIONS] = {0};
	qdf_freq_t freq_list[MAX_NUMBER_OF_CONC_CONNECTIONS] = {0};
	uint8_t vdev_id_list[MAX_NUMBER_OF_CONC_CONNECTIONS] = {0};

	if_mgr_get_mlo_and_legacy_sta_count(psoc, &num_mlo, mlo_idx,
					    &num_legacy, legacy_idx, freq_list,
					    vdev_id_list);

	ifmgr_debug("vdev %d: num_mlo %d num_legacy %d is_connect %d",
		    wlan_vdev_get_id(vdev), num_mlo, num_legacy, is_connect);
	/* ML STA is not up */
	if (num_mlo < 2)
		return;

	if (is_connect)
		if_mgr_sta_mlo_concurency_on_connect(psoc, vdev, num_mlo,
						     mlo_idx, num_legacy,
						     legacy_idx, freq_list,
						     vdev_id_list);
	else
		if_mgr_sta_mlo_concurency_on_disconnect(vdev, num_mlo, mlo_idx,
							num_legacy,
							vdev_id_list);
}
#else
static inline void
if_mgr_handle_sta_mlo_link_concurrency(struct wlan_objmgr_psoc *psoc,
				       struct wlan_objmgr_vdev *vdev,
				       bool is_connect)
{}
#endif

QDF_STATUS if_mgr_connect_complete(struct wlan_objmgr_vdev *vdev,
				   struct if_mgr_event_data *event_data)
{
	struct wlan_objmgr_psoc *psoc;
	struct wlan_objmgr_pdev *pdev;
	QDF_STATUS status = event_data->status;

	pdev = wlan_vdev_get_pdev(vdev);
	if (!pdev)
		return QDF_STATUS_E_FAILURE;

	psoc = wlan_pdev_get_psoc(pdev);
	if (!psoc)
		return QDF_STATUS_E_FAILURE;

	if (QDF_IS_STATUS_SUCCESS(status)) {
		/*
		 * Due to audio share glitch with P2P clients caused by roam
		 * scan on concurrent interface, disable roaming if
		 * "p2p_disable_roam" ini is enabled. Donot re-enable roaming
		 * again on other STA interface if p2p client connection is
		 * active on any vdev.
		 */
		if (ucfg_p2p_is_roam_config_disabled(psoc) &&
		    wlan_vdev_mlme_get_opmode(vdev) == QDF_P2P_CLIENT_MODE) {
			ifmgr_debug("p2p client active, keep roam disabled");
		} else {
			ifmgr_debug("set pcl when connection on vdev id:%d",
				     vdev->vdev_objmgr.vdev_id);
			policy_mgr_set_pcl_for_connected_vdev(psoc,
					      vdev->vdev_objmgr.vdev_id, false);
			/*
			 * Enable roaming on other STA iface except this one.
			 * Firmware doesn't support connection on one STA iface
			 * while roaming on other STA iface.
			 */
			if_mgr_enable_roaming(pdev, vdev, RSO_CONNECT_START);
		}
		if (wlan_vdev_mlme_get_opmode(vdev) == QDF_STA_MODE)
			if_mgr_handle_sta_mlo_link_concurrency(psoc, vdev,
							       true);
	} else {
		/* notify connect failure on final failure */
		ucfg_tdls_notify_connect_failure(psoc);

		/*
		 * Enable roaming on other STA iface except this one.
		 * Firmware doesn't support connection on one STA iface
		 * while roaming on other STA iface.
		 */
		if_mgr_enable_roaming(pdev, vdev, RSO_CONNECT_START);
	}

	policy_mgr_check_n_start_opportunistic_timer(psoc);

	if (!wlan_cm_is_vdev_roaming(vdev))
		policy_mgr_check_concurrent_intf_and_restart_sap(psoc);

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS if_mgr_disconnect_start(struct wlan_objmgr_vdev *vdev,
				   struct if_mgr_event_data *event_data)
{
	struct wlan_objmgr_psoc *psoc;
	struct wlan_objmgr_pdev *pdev;

	pdev = wlan_vdev_get_pdev(vdev);
	if (!pdev)
		return QDF_STATUS_E_FAILURE;

	psoc = wlan_pdev_get_psoc(pdev);
	if (!psoc)
		return QDF_STATUS_E_FAILURE;

	/* Leaving as stub to fill in later */

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS if_mgr_disconnect_complete(struct wlan_objmgr_vdev *vdev,
				      struct if_mgr_event_data *event_data)
{
	struct wlan_objmgr_psoc *psoc;
	struct wlan_objmgr_pdev *pdev;
	QDF_STATUS status;

	pdev = wlan_vdev_get_pdev(vdev);
	if (!pdev)
		return QDF_STATUS_E_FAILURE;

	psoc = wlan_pdev_get_psoc(pdev);
	if (!psoc)
		return QDF_STATUS_E_FAILURE;

	status = if_mgr_enable_roaming_after_p2p_disconnect(pdev, vdev,
							RSO_INVALID_REQUESTOR);
	if (status) {
		ifmgr_err("Failed to enable roaming after p2p disconnect");
		return status;
	}

	policy_mgr_check_concurrent_intf_and_restart_sap(psoc);

	status = if_mgr_enable_roaming_on_connected_sta(pdev, vdev);
	if (status) {
		ifmgr_err("Failed to enable roaming on connected sta");
		return status;
	}

	if (wlan_vdev_mlme_get_opmode(vdev) == QDF_STA_MODE)
		if_mgr_handle_sta_mlo_link_concurrency(psoc, vdev, false);

	return QDF_STATUS_SUCCESS;
}
