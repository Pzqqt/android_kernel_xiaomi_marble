/*
 * Copyright (c) 2018 The Linux Foundation. All rights reserved.
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

static int init_deinit_service_ready_event_handler(ol_scn_t scn_handle,
							uint8_t *event,
							uint32_t data_len)
{
	int err_code;
	struct wlan_objmgr_psoc *psoc;
	struct target_psoc_info *tgt_hdl;
	wmi_legacy_service_ready_callback legacy_callback;
	void *wmi_handle;
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

	tgt_hdl = (struct target_psoc_info *)wlan_psoc_get_tgt_if_handle(
						psoc);
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

	target_if_info(" TT support %d, Wide BW Scan %d, SW cal %d",
		wlan_psoc_nif_fw_ext_cap_get(psoc, WLAN_SOC_CEXT_TT_SUPPORT),
		wlan_psoc_nif_fw_ext_cap_get(psoc, WLAN_SOC_CEXT_WIDEBAND_SCAN),
		wlan_psoc_nif_fw_ext_cap_get(psoc, WLAN_SOC_CEXT_SW_CAL));

	target_if_mesh_support_enable(psoc, tgt_hdl, event);

	target_if_smart_antenna_enable(psoc, tgt_hdl, event);

	target_if_peer_cfg_enable(psoc, tgt_hdl, event);

	target_if_atf_cfg_enable(psoc, tgt_hdl, event);

	target_if_qwrap_cfg_enable(psoc, tgt_hdl, event);

	target_if_lteu_cfg_enable(psoc, tgt_hdl, event);

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
		target_if_info("WMI mgmt service enabled");
	} else {
		wlan_psoc_nif_fw_ext_cap_clear(psoc,
					       WLAN_SOC_CEXT_WMI_MGMT_REF);
		target_if_info("WMI mgmt service disabled");
	}

	err_code = init_deinit_handle_host_mem_req(psoc, tgt_hdl, event);
	if (err_code != QDF_STATUS_SUCCESS)
		goto exit;

	target_if_reg_set_offloaded_info(psoc);
	if (!wmi_service_enabled(wmi_handle, wmi_service_ext_msg)) {
		target_if_info("No EXT message, send init command");
		tgt_hdl->info.wmi_service_ready = TRUE;
		target_psoc_set_num_radios(tgt_hdl, 1);
		/* send init command */
		init_deinit_prepare_send_init_cmd(psoc, tgt_hdl);
	} else {
		target_if_info("Wait for EXT message");
	}
	target_if_btcoex_cfg_enable(psoc, tgt_hdl, event);

exit:
	return err_code;
}

static int init_deinit_service_ext_ready_event_handler(ol_scn_t scn_handle,
						uint8_t *event,
						uint32_t data_len)
{
	int err_code;
	struct wlan_objmgr_psoc *psoc;
	struct target_psoc_info *tgt_hdl;
	void *wmi_handle;
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

	tgt_hdl = (struct target_psoc_info *)wlan_psoc_get_tgt_if_handle(
						psoc);
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
			== FALSE)
		return -EINVAL;

	target_if_print_service_ready_ext_param(psoc, tgt_hdl);

	err_code = init_deinit_populate_phy_reg_cap(psoc, wmi_handle,
					   event, info, false);
	if (err_code)
		goto exit;

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

	err_code = init_deinit_populate_dbr_ring_cap(psoc, wmi_handle,
						event, info);
	if (err_code)
		goto exit;

	legacy_callback = target_if_get_psoc_legacy_service_ready_cb();
	if (legacy_callback)
		legacy_callback(wmi_service_ready_ext_event_id,
				scn_handle, event, data_len);

	info->wlan_res_cfg.num_vdevs = (target_psoc_get_num_radios(tgt_hdl) *
					info->wlan_res_cfg.num_vdevs);
	info->wlan_res_cfg.beacon_tx_offload_max_vdev =
				(target_psoc_get_num_radios(tgt_hdl) *
				info->wlan_res_cfg.beacon_tx_offload_max_vdev);

	info->wmi_service_ready = TRUE;

	init_deinit_prepare_send_init_cmd(psoc, tgt_hdl);

exit:
	return err_code;
}

static int init_deinit_service_available_handler(ol_scn_t scn_handle,
						uint8_t *event,
						uint32_t data_len)
{
	struct wlan_objmgr_psoc *psoc;
	struct target_psoc_info *tgt_hdl;
	void *wmi_handle;

	if (!scn_handle) {
		target_if_err("scn handle NULL");
		return -EINVAL;
	}

	psoc = target_if_get_psoc_from_scn_hdl(scn_handle);
	if (!psoc) {
		target_if_err("psoc is null");
		return -EINVAL;
	}

	tgt_hdl = (struct target_psoc_info *)wlan_psoc_get_tgt_if_handle(
						psoc);
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
	void *wmi_handle;
	struct wmi_host_fw_abi_ver fw_ver;
	uint8_t myaddr[QDF_MAC_ADDR_SIZE];
	struct tgt_info *info;
	struct wmi_host_ready_ev_param ready_ev;
	wmi_legacy_service_ready_callback legacy_callback;
	uint8_t num_radios, i;

	if (!scn_handle) {
		target_if_err("scn handle NULL");
		return -EINVAL;
	}

	psoc = target_if_get_psoc_from_scn_hdl(scn_handle);
	if (!psoc) {
		target_if_err("psoc is null");
		return -EINVAL;
	}

	tgt_hdl = (struct target_psoc_info *)wlan_psoc_get_tgt_if_handle(
						psoc);
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

	if ((ready_ev.num_total_peer != 0) &&
	    (info->wlan_res_cfg.num_peers != ready_ev.num_total_peer)) {
		/* FW allocated number of peers is different than host
		 * requested. Update host max with FW reported value.
		 */
		target_if_info("Host Requested %d peers. FW Supports %d peers",
			       info->wlan_res_cfg.num_peers,
			       ready_ev.num_total_peer);
		info->wlan_res_cfg.num_peers = ready_ev.num_total_peer;
	}

	/* Indicate to the waiting thread that the ready
	 * event was received
	 */
	info->wlan_init_status = wmi_ready_extract_init_status(
						wmi_handle, event);

	legacy_callback = target_if_get_psoc_legacy_service_ready_cb();
	if (legacy_callback)
		legacy_callback(wmi_ready_event_id,
				scn_handle, event, data_len);

	num_radios = target_psoc_get_num_radios(tgt_hdl);
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
	tgt_hdl->info.wmi_ready = TRUE;
	init_deinit_wakeup_host_wait(psoc, tgt_hdl);

	return 0;
}


QDF_STATUS init_deinit_register_tgt_psoc_ev_handlers(
				struct wlan_objmgr_psoc *psoc)
{
	struct target_psoc_info *tgt_hdl;
	void *wmi_handle;
	QDF_STATUS retval = QDF_STATUS_SUCCESS;

	if (!psoc) {
		target_if_err("psoc is null in register wmi handler");
		return QDF_STATUS_E_FAILURE;
	}

	tgt_hdl = (struct target_psoc_info *)wlan_psoc_get_tgt_if_handle(
						psoc);
	if (!tgt_hdl) {
		target_if_err("target_psoc_info null in register wmi hadler");
		return QDF_STATUS_E_FAILURE;
	}

	wmi_handle = target_psoc_get_wmi_hdl(tgt_hdl);

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

	return retval;
}

