/*
 * Copyright (c) 2018-2020 The Linux Foundation. All rights reserved.
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
 * DOC: init_event_handler.c
 *
 * WMI common event handler implementation source file
 */

#include <qdf_status.h>
#include <wlan_objmgr_psoc_obj.h>
#include <wlan_objmgr_pdev_obj.h>
#include <target_if.h>
#include <target_if_reg.h>
#include <init_event_handler.h>
#include <service_ready_util.h>
#include <service_ready_param.h>
#include <init_cmd_api.h>
#include <cdp_txrx_cmn.h>
#include <wlan_reg_ucfg_api.h>

static void init_deinit_set_send_init_cmd(struct wlan_objmgr_psoc *psoc,
					  struct target_psoc_info *tgt_hdl)
{
	tgt_hdl->info.wmi_service_ready = TRUE;
	/* send init command */
	init_deinit_prepare_send_init_cmd(psoc, tgt_hdl);
}

static int init_deinit_service_ready_event_handler(ol_scn_t scn_handle,
							uint8_t *event,
							uint32_t data_len)
{
	int err_code;
	struct wlan_objmgr_psoc *psoc;
	struct target_psoc_info *tgt_hdl;
	wmi_legacy_service_ready_callback legacy_callback;
	struct wmi_unified *wmi_handle;
	QDF_STATUS ret_val;

	if (!scn_handle) {
		target_if_err("scn handle NULL in service ready handler");
		return -EINVAL;
	}

	psoc = target_if_get_psoc_from_scn_hdl(scn_handle);
	if (!psoc) {
		target_if_err("psoc is null in service ready handler");
		return -EINVAL;
	}

	tgt_hdl = wlan_psoc_get_tgt_if_handle(psoc);
	if (!tgt_hdl) {
		target_if_err("target_psoc_info is null in service ready ev");
		return -EINVAL;
	}

	ret_val = target_if_sw_version_check(psoc, tgt_hdl, event);

	wmi_handle = target_psoc_get_wmi_hdl(tgt_hdl);

	err_code = init_deinit_populate_service_bitmap(wmi_handle, event,
			tgt_hdl->info.service_bitmap);
	if (err_code)
		goto exit;

	err_code = init_deinit_populate_fw_version_cmd(wmi_handle, event);
	if (err_code)
		goto exit;

	err_code = init_deinit_populate_target_cap(wmi_handle, event,
				   &(tgt_hdl->info.target_caps));
	if (err_code)
		goto exit;

	err_code = init_deinit_populate_phy_reg_cap(psoc, wmi_handle, event,
				    &(tgt_hdl->info), true);
	if (err_code)
		goto exit;

	if (init_deinit_validate_160_80p80_fw_caps(psoc, tgt_hdl) !=
			QDF_STATUS_SUCCESS) {
		wlan_psoc_nif_op_flag_set(psoc, WLAN_SOC_OP_VHT_INVALID_CAP);
	}

	target_if_ext_res_cfg_enable(psoc, tgt_hdl, event);

	if (wmi_service_enabled(wmi_handle, wmi_service_tt))
		wlan_psoc_nif_fw_ext_cap_set(psoc, WLAN_SOC_CEXT_TT_SUPPORT);

	if (wmi_service_enabled(wmi_handle, wmi_service_widebw_scan))
		wlan_psoc_nif_fw_ext_cap_set(psoc, WLAN_SOC_CEXT_WIDEBAND_SCAN);

	if (wmi_service_enabled(wmi_handle, wmi_service_check_cal_version))
		wlan_psoc_nif_fw_ext_cap_set(psoc, WLAN_SOC_CEXT_SW_CAL);

	if (wmi_service_enabled(wmi_handle, wmi_service_twt_requestor))
		wlan_psoc_nif_fw_ext_cap_set(psoc, WLAN_SOC_CEXT_TWT_REQUESTER);

	if (wmi_service_enabled(wmi_handle, wmi_service_twt_responder))
		wlan_psoc_nif_fw_ext_cap_set(psoc, WLAN_SOC_CEXT_TWT_RESPONDER);

	if (wmi_service_enabled(wmi_handle, wmi_service_bss_color_offload))
		target_if_debug(" BSS COLOR OFFLOAD supported");

	if (wmi_service_enabled(wmi_handle, wmi_service_ul_ru26_allowed))
		wlan_psoc_nif_fw_ext_cap_set(psoc, WLAN_SOC_CEXT_OBSS_NBW_RU);

	if (wmi_service_enabled(wmi_handle, wmi_service_infra_mbssid))
		wlan_psoc_nif_fw_ext_cap_set(psoc, WLAN_SOC_CEXT_MBSS_IE);

	if (wmi_service_enabled(wmi_handle,
				wmi_service_mbss_param_in_vdev_start_support))
		wlan_psoc_nif_fw_ext_cap_set(psoc,
					     WLAN_SOC_CEXT_MBSS_PARAM_IN_START);

	if (wmi_service_enabled(wmi_handle, wmi_service_dynamic_hw_mode))
		wlan_psoc_nif_fw_ext_cap_set(psoc, WLAN_SOC_CEXT_DYNAMIC_HW_MODE);

	if (wmi_service_enabled(wmi_handle,
				wmi_service_bw_restricted_80p80_support))
		wlan_psoc_nif_fw_ext_cap_set(psoc,
					     WLAN_SOC_RESTRICTED_80P80_SUPPORT);

	if (wmi_service_enabled(wmi_handle,
				wmi_service_nss_ratio_to_host_support))
		wlan_psoc_nif_fw_ext_cap_set(
				psoc, WLAN_SOC_NSS_RATIO_TO_HOST_SUPPORT);

	target_if_debug(" TT support %d, Wide BW Scan %d, SW cal %d",
		wlan_psoc_nif_fw_ext_cap_get(psoc, WLAN_SOC_CEXT_TT_SUPPORT),
		wlan_psoc_nif_fw_ext_cap_get(psoc, WLAN_SOC_CEXT_WIDEBAND_SCAN),
		wlan_psoc_nif_fw_ext_cap_get(psoc, WLAN_SOC_CEXT_SW_CAL));

	target_if_mesh_support_enable(psoc, tgt_hdl, event);

	target_if_eapol_minrate_enable(psoc, tgt_hdl, event);

	target_if_smart_antenna_enable(psoc, tgt_hdl, event);

	target_if_cfr_support_enable(psoc, tgt_hdl, event);

	target_if_peer_cfg_enable(psoc, tgt_hdl, event);

	target_if_atf_cfg_enable(psoc, tgt_hdl, event);

	if (!wmi_service_enabled(wmi_handle, wmi_service_ext_msg))
		target_if_qwrap_cfg_enable(psoc, tgt_hdl, event);

	target_if_lteu_cfg_enable(psoc, tgt_hdl, event);

	if (wmi_service_enabled(wmi_handle, wmi_service_rx_fse_support))
		wlan_psoc_nif_fw_ext_cap_set(psoc,
					     WLAN_SOC_CEXT_RX_FSE_SUPPORT);

	/* override derived value, if it exceeds max peer count */
	if ((wlan_psoc_get_max_peer_count(psoc) >
		tgt_hdl->info.wlan_res_cfg.num_active_peers) &&
		(wlan_psoc_get_max_peer_count(psoc) <
			(tgt_hdl->info.wlan_res_cfg.num_peers -
				tgt_hdl->info.wlan_res_cfg.num_vdevs))) {
		tgt_hdl->info.wlan_res_cfg.num_peers =
				wlan_psoc_get_max_peer_count(psoc) +
					tgt_hdl->info.wlan_res_cfg.num_vdevs;
	}
	legacy_callback = target_if_get_psoc_legacy_service_ready_cb();
	if (!legacy_callback) {
		err_code = -EINVAL;
		goto exit;
	}

	err_code = legacy_callback(wmi_service_ready_event_id,
				  scn_handle, event, data_len);
	init_deinit_chainmask_config(psoc, tgt_hdl);

	if (wmi_service_enabled(wmi_handle, wmi_service_mgmt_tx_wmi)) {
		wlan_psoc_nif_fw_ext_cap_set(psoc, WLAN_SOC_CEXT_WMI_MGMT_REF);
		target_if_debug("WMI mgmt service enabled");
	} else {
		wlan_psoc_nif_fw_ext_cap_clear(psoc,
					       WLAN_SOC_CEXT_WMI_MGMT_REF);
		target_if_debug("WMI mgmt service disabled");
	}

	err_code = init_deinit_handle_host_mem_req(psoc, tgt_hdl, event);
	if (err_code != QDF_STATUS_SUCCESS)
		goto exit;

	target_if_reg_set_offloaded_info(psoc);
	target_if_reg_set_6ghz_info(psoc);
	target_if_reg_set_5dot9_ghz_info(psoc);

	/* Send num_msdu_desc to DP layer */
	cdp_soc_set_param(wlan_psoc_get_dp_handle(psoc),
			  DP_SOC_PARAM_MSDU_EXCEPTION_DESC,
			  tgt_hdl->info.target_caps.num_msdu_desc);

	/* Send CMEM FSE support to DP layer */
	if (wmi_service_enabled(wmi_handle, wmi_service_fse_cmem_alloc_support))
		cdp_soc_set_param(wlan_psoc_get_dp_handle(psoc),
				  DP_SOC_PARAM_CMEM_FSE_SUPPORT, 1);

	if (wmi_service_enabled(wmi_handle, wmi_service_ext_msg)) {
		target_if_debug("Wait for EXT message");
	} else {
		target_if_debug("No EXT message, send init command");
		target_psoc_set_num_radios(tgt_hdl, 1);
		init_deinit_set_send_init_cmd(psoc, tgt_hdl);
	}

exit:
	return err_code;
}

static int init_deinit_service_ext2_ready_event_handler(ol_scn_t scn_handle,
							uint8_t *event,
							uint32_t data_len)
{
	int err_code = 0;
	struct wlan_objmgr_psoc *psoc;
	struct target_psoc_info *tgt_hdl;
	struct wmi_unified *wmi_handle;
	struct tgt_info *info;

	if (!scn_handle) {
		target_if_err("scn handle NULL in service ready ext2 handler");
		return -EINVAL;
	}

	psoc = target_if_get_psoc_from_scn_hdl(scn_handle);
	if (!psoc) {
		target_if_err("psoc is null in service ready ext2 handler");
		return -EINVAL;
	}

	tgt_hdl = wlan_psoc_get_tgt_if_handle(psoc);
	if (!tgt_hdl) {
		target_if_err("target_psoc_info is null in service ready ext2 handler");
		return -EINVAL;
	}

	wmi_handle = target_psoc_get_wmi_hdl(tgt_hdl);
	if (!wmi_handle) {
		target_if_err("wmi_handle is null in service ready ext2 handler");
		return -EINVAL;
	}

	info = (&tgt_hdl->info);

	err_code = init_deinit_populate_service_ready_ext2_param(wmi_handle,
								 event, info);
	if (err_code)
		goto exit;

	/* dbr_ring_caps could have already come as part of EXT event */
	if (info->service_ext2_param.num_dbr_ring_caps) {
		err_code = init_deinit_populate_dbr_ring_cap_ext2(psoc,
								  wmi_handle,
								  event, info);
		if (err_code)
			goto exit;
	}

	err_code = init_deinit_populate_hal_reg_cap_ext2(wmi_handle, event,
							 info);
	if (err_code) {
		target_if_err("failed to populate hal reg cap ext2");
		goto exit;
	}

	err_code = init_deinit_populate_mac_phy_cap_ext2(wmi_handle, event,
							 info);
	if (err_code) {
		target_if_err("failed to populate mac phy cap ext2");
		goto exit;
	}

	target_if_add_11ax_modes(psoc, tgt_hdl);

	err_code = init_deinit_populate_scan_radio_cap_ext2(wmi_handle, event,
							    info);
	if (err_code) {
		target_if_err("failed to populate scan radio cap ext2");
		goto exit;
	}

	/* send init command */
	init_deinit_set_send_init_cmd(psoc, tgt_hdl);

exit:
	return err_code;
}

static int init_deinit_service_ext_ready_event_handler(ol_scn_t scn_handle,
						uint8_t *event,
						uint32_t data_len)
{
	int err_code;
	uint8_t num_radios;
	struct wlan_objmgr_psoc *psoc;
	struct target_psoc_info *tgt_hdl;
	struct wmi_unified *wmi_handle;
	struct tgt_info *info;
	wmi_legacy_service_ready_callback legacy_callback;

	if (!scn_handle) {
		target_if_err("scn handle NULL in service ready handler");
		return -EINVAL;
	}

	psoc = target_if_get_psoc_from_scn_hdl(scn_handle);
	if (!psoc) {
		target_if_err("psoc is null in service ready handler");
		return -EINVAL;
	}

	tgt_hdl = wlan_psoc_get_tgt_if_handle(psoc);
	if (!tgt_hdl) {
		target_if_err("target_psoc_info is null in service ready ev");
		return -EINVAL;
	}

	wmi_handle = target_psoc_get_wmi_hdl(tgt_hdl);
	info = (&tgt_hdl->info);

	err_code = init_deinit_populate_service_ready_ext_param(wmi_handle,
				event, &(info->service_ext_param));
	if (err_code)
		goto exit;

	target_psoc_set_num_radios(tgt_hdl, 0);
	err_code =  init_deinit_populate_hw_mode_capability(wmi_handle,
					    event, tgt_hdl);
	if (err_code)
		goto exit;

	if (init_deinit_is_preferred_hw_mode_supported(psoc, tgt_hdl)
			== FALSE) {
		target_if_err("Preferred mode %d not supported",
			      info->preferred_hw_mode);
		return -EINVAL;
	}

	num_radios = target_psoc_get_num_radios_for_mode(tgt_hdl,
							 info->preferred_hw_mode);

	/* set number of radios based on current mode */
	target_psoc_set_num_radios(tgt_hdl, num_radios);

	target_if_print_service_ready_ext_param(psoc, tgt_hdl);

	err_code = init_deinit_populate_phy_reg_cap(psoc, wmi_handle,
					   event, info, false);
	if (err_code)
		goto exit;

	/* Host receives 11AX wireless modes from target in service ext2
	 * message. Therefore, call target_if_add_11ax_modes() from service ext2
	 * event handler as well.
	 */
	if (!wmi_service_enabled(wmi_handle, wmi_service_ext2_msg))
		target_if_add_11ax_modes(psoc, tgt_hdl);

	if (init_deinit_chainmask_table_alloc(
				&(info->service_ext_param)) ==
							QDF_STATUS_SUCCESS) {
		err_code = init_deinit_populate_chainmask_tables(wmi_handle,
				event,
				&(info->service_ext_param.chainmask_table[0]));
		if (err_code)
			goto exit;
	}

	/* dbr_ring_caps can be absent if enough space is not available */
	if (info->service_ext_param.num_dbr_ring_caps) {
		err_code = init_deinit_populate_dbr_ring_cap(psoc, wmi_handle,
							     event, info);
		if (err_code)
			goto exit;
	}

	err_code = init_deinit_populate_spectral_bin_scale_params(psoc,
								  wmi_handle,
								  event, info);
	if (err_code)
		goto exit;

	legacy_callback = target_if_get_psoc_legacy_service_ready_cb();
	if (legacy_callback)
		legacy_callback(wmi_service_ready_ext_event_id,
				scn_handle, event, data_len);

	target_if_qwrap_cfg_enable(psoc, tgt_hdl, event);

	target_if_set_twt_ap_pdev_count(info, tgt_hdl);

	info->wlan_res_cfg.max_bssid_indicator =
				info->service_ext_param.max_bssid_indicator;

	if (wmi_service_enabled(wmi_handle, wmi_service_ext2_msg)) {
		target_if_debug("Wait for EXT2 message");
	} else {
		target_if_debug("No EXT2 message, send init command");
		init_deinit_set_send_init_cmd(psoc, tgt_hdl);
	}

exit:
	return err_code;
}

static int init_deinit_service_available_handler(ol_scn_t scn_handle,
						uint8_t *event,
						uint32_t data_len)
{
	struct wlan_objmgr_psoc *psoc;
	struct target_psoc_info *tgt_hdl;
	struct wmi_unified *wmi_handle;

	if (!scn_handle) {
		target_if_err("scn handle NULL");
		return -EINVAL;
	}

	psoc = target_if_get_psoc_from_scn_hdl(scn_handle);
	if (!psoc) {
		target_if_err("psoc is null");
		return -EINVAL;
	}

	tgt_hdl = wlan_psoc_get_tgt_if_handle(psoc);
	if (!tgt_hdl) {
		target_if_err("target_psoc_info is null");
		return -EINVAL;
	}

	wmi_handle = target_psoc_get_wmi_hdl(tgt_hdl);

	if (wmi_save_ext_service_bitmap(wmi_handle, event, NULL) !=
					QDF_STATUS_SUCCESS) {
		target_if_err("Failed to save ext service bitmap");
		return -EINVAL;
	}

	return 0;
}

/* MAC address fourth byte index */
#define MAC_BYTE_4 4

static int init_deinit_ready_event_handler(ol_scn_t scn_handle,
						uint8_t *event,
						uint32_t data_len)
{
	struct wlan_objmgr_psoc *psoc;
	struct wlan_objmgr_pdev *pdev;
	struct target_psoc_info *tgt_hdl;
	struct wmi_unified *wmi_handle;
	struct wmi_host_fw_abi_ver fw_ver;
	uint8_t myaddr[QDF_MAC_ADDR_SIZE];
	struct tgt_info *info;
	struct wmi_host_ready_ev_param ready_ev;
	wmi_legacy_service_ready_callback legacy_callback;
	uint8_t num_radios, i;
	uint32_t max_peers;
	uint32_t max_ast_index;
	target_resource_config *tgt_cfg;

	if (!scn_handle) {
		target_if_err("scn handle NULL");
		return -EINVAL;
	}

	psoc = target_if_get_psoc_from_scn_hdl(scn_handle);
	if (!psoc) {
		target_if_err("psoc is null");
		return -EINVAL;
	}

	tgt_hdl = wlan_psoc_get_tgt_if_handle(psoc);
	if (!tgt_hdl) {
		target_if_err("target_psoc_info is null");
		return -EINVAL;
	}

	wmi_handle = target_psoc_get_wmi_hdl(tgt_hdl);
	info = (&tgt_hdl->info);

	if (wmi_extract_fw_abi_version(wmi_handle, event, &fw_ver) ==
				QDF_STATUS_SUCCESS) {
		info->version.wlan_ver = fw_ver.sw_version;
		info->version.wlan_ver = fw_ver.abi_version;
	}

	if (wmi_check_and_update_fw_version(wmi_handle, event) < 0) {
		target_if_err("Version mismatch with FW");
		return -EINVAL;
	}

	if (wmi_extract_ready_event_params(wmi_handle, event, &ready_ev) !=
				QDF_STATUS_SUCCESS) {
		target_if_err("Failed to extract ready event");
		return -EINVAL;
	}

	if (!ready_ev.agile_capability)
		target_if_err("agile capability disabled in HW");
	else
		info->wlan_res_cfg.agile_capability = ready_ev.agile_capability;

	/* Indicate to the waiting thread that the ready
	 * event was received
	 */
	info->wlan_init_status = wmi_ready_extract_init_status(
						wmi_handle, event);

	legacy_callback = target_if_get_psoc_legacy_service_ready_cb();
	if (legacy_callback)
		if (legacy_callback(wmi_ready_event_id,
				    scn_handle, event, data_len)) {
			target_if_err("Legacy callback returned error!");
			tgt_hdl->info.wmi_ready = FALSE;
			goto exit;
		}

	num_radios = target_psoc_get_num_radios(tgt_hdl);

	if ((ready_ev.num_total_peer != 0) &&
	    (info->wlan_res_cfg.num_peers != ready_ev.num_total_peer)) {
		uint16_t num_peers = 0;
		/* FW allocated number of peers is different than host
		 * requested. Update host max with FW reported value.
		 */
		target_if_err("Host Requested %d peers. FW Supports %d peers",
			       info->wlan_res_cfg.num_peers,
			       ready_ev.num_total_peer);
		info->wlan_res_cfg.num_peers = ready_ev.num_total_peer;
		num_peers = info->wlan_res_cfg.num_peers / num_radios;

		for (i = 0; i < num_radios; i++) {
			pdev = wlan_objmgr_get_pdev_by_id(psoc, i,
							  WLAN_INIT_DEINIT_ID);
			if (!pdev) {
				target_if_err(" PDEV %d is NULL", i);
				return -EINVAL;
			}

			wlan_pdev_set_max_peer_count(pdev, num_peers);
			wlan_objmgr_pdev_release_ref(pdev, WLAN_INIT_DEINIT_ID);
		}

		wlan_psoc_set_max_peer_count(psoc,
					     info->wlan_res_cfg.num_peers);
	}

	/* for non legacy  num_total_peer will be non zero
	 * allocate peer memory in this case
	 */
	if (ready_ev.num_total_peer != 0) {
		tgt_cfg = &info->wlan_res_cfg;
		max_peers = tgt_cfg->num_peers + ready_ev.num_extra_peer + 1;
		max_ast_index = ready_ev.max_ast_index + 1;

		if (cdp_peer_map_attach(wlan_psoc_get_dp_handle(psoc),
					max_peers, max_ast_index,
					tgt_cfg->peer_map_unmap_v2) !=
				QDF_STATUS_SUCCESS) {
			target_if_err("DP peer map attach failed");
			return -EINVAL;
		}
	}


	if (ready_ev.pktlog_defs_checksum) {
		for (i = 0; i < num_radios; i++) {
			pdev = wlan_objmgr_get_pdev_by_id(psoc, i,
							  WLAN_INIT_DEINIT_ID);
			if (!pdev) {
				target_if_err(" PDEV %d is NULL", i);
				return -EINVAL;
			}
			target_if_set_pktlog_checksum(pdev, tgt_hdl,
						      ready_ev.
						      pktlog_defs_checksum);
			wlan_objmgr_pdev_release_ref(pdev, WLAN_INIT_DEINIT_ID);
		}
	}

	/*
	 * For non-legacy HW, MAC addr list is extracted.
	 */
	if (num_radios > 1) {
		uint8_t num_mac_addr;
		wmi_host_mac_addr *addr_list;
		int i;

		addr_list = wmi_ready_extract_mac_addr_list(wmi_handle, event,
							    &num_mac_addr);
		if ((num_mac_addr >= num_radios) && (addr_list)) {
			for (i = 0; i < num_radios; i++) {
				WMI_HOST_MAC_ADDR_TO_CHAR_ARRAY(&addr_list[i],
								myaddr);
				pdev = wlan_objmgr_get_pdev_by_id(psoc, i,
								  WLAN_INIT_DEINIT_ID);
				if (!pdev) {
					target_if_err(" PDEV %d is NULL", i);
					return -EINVAL;
				}
				wlan_pdev_set_hw_macaddr(pdev, myaddr);
				wlan_objmgr_pdev_release_ref(pdev,
							WLAN_INIT_DEINIT_ID);

				/* assign 1st radio addr to psoc */
				if (i == 0)
					wlan_psoc_set_hw_macaddr(psoc, myaddr);
			}
			goto out;
		} else {
			target_if_err("Using default MAC addr for all radios..");
		}
	}

	/*
	 * We extract single MAC address in two scenarios:
	 * 1. In non-legacy case, if addr list is NULL or num_mac_addr < num_radios
	 * 2. In all legacy cases
	 */
	for (i = 0; i < num_radios; i++) {
		wmi_ready_extract_mac_addr(wmi_handle, event, myaddr);
		myaddr[MAC_BYTE_4] += i;
		pdev = wlan_objmgr_get_pdev_by_id(psoc, i, WLAN_INIT_DEINIT_ID);
		if (!pdev) {
			target_if_err(" PDEV %d is NULL", i);
			return -EINVAL;
		}
		wlan_pdev_set_hw_macaddr(pdev, myaddr);
		wlan_objmgr_pdev_release_ref(pdev, WLAN_INIT_DEINIT_ID);
		/* assign 1st radio addr to psoc */
		if (i == 0)
			wlan_psoc_set_hw_macaddr(psoc, myaddr);
	}

out:
	target_if_btcoex_cfg_enable(psoc, tgt_hdl, event);
	tgt_hdl->info.wmi_ready = TRUE;
exit:
	init_deinit_wakeup_host_wait(psoc, tgt_hdl);

	return 0;
}


QDF_STATUS init_deinit_register_tgt_psoc_ev_handlers(
				struct wlan_objmgr_psoc *psoc)
{
	struct target_psoc_info *tgt_hdl;
	wmi_unified_t wmi_handle;
	QDF_STATUS retval = QDF_STATUS_SUCCESS;

	if (!psoc) {
		target_if_err("psoc is null in register wmi handler");
		return QDF_STATUS_E_FAILURE;
	}

	tgt_hdl = wlan_psoc_get_tgt_if_handle(psoc);
	if (!tgt_hdl) {
		target_if_err("target_psoc_info null in register wmi hadler");
		return QDF_STATUS_E_FAILURE;
	}

	wmi_handle = (wmi_unified_t)target_psoc_get_wmi_hdl(tgt_hdl);

	retval = wmi_unified_register_event_handler(wmi_handle,
				wmi_service_ready_event_id,
				init_deinit_service_ready_event_handler,
				WMI_RX_WORK_CTX);
	retval = wmi_unified_register_event_handler(wmi_handle,
				wmi_service_ready_ext_event_id,
				init_deinit_service_ext_ready_event_handler,
				WMI_RX_WORK_CTX);
	retval = wmi_unified_register_event_handler(wmi_handle,
				wmi_service_available_event_id,
				init_deinit_service_available_handler,
				WMI_RX_UMAC_CTX);
	retval = wmi_unified_register_event_handler(wmi_handle,
				wmi_ready_event_id,
				init_deinit_ready_event_handler,
				WMI_RX_WORK_CTX);
	retval = wmi_unified_register_event_handler(
				wmi_handle,
				wmi_service_ready_ext2_event_id,
				init_deinit_service_ext2_ready_event_handler,
				WMI_RX_WORK_CTX);


	return retval;
}

