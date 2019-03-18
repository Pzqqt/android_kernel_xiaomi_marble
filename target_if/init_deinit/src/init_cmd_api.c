/*
 * Copyright (c) 2018-2019 The Linux Foundation. All rights reserved.
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
 * DOC: init_cmd_api.c
 *
 * WMI Init command prepare & send APIs
 */
#include <qdf_status.h>
#include <qdf_types.h>
#include <wlan_objmgr_psoc_obj.h>
#include <wlan_objmgr_pdev_obj.h>
#include <target_if.h>
#include <service_ready_util.h>
#include <wlan_tgt_def_config.h>
#include <wlan_reg_ucfg_api.h>
#include <init_cmd_api.h>
#include <wlan_defs.h>
#include <target_if_scan.h>
#include <target_if_reg.h>

/**
 *  init_deinit_alloc_host_mem_chunk() - allocates chunk of memory requested
 *                                       by FW.
 *  @psoc: PSOC object
 *  @tgt_hdl: Target PSOC info
 *  @req_id: request id
 *  @idx: chunk id
 *  @num_units: Number of units
 *  @unit_len: Unit length
 *  @num_unit_info: Num unit info
 *
 *  API to allocate host memory chunk requested by FW
 *
 *  Return: num_units on successful allocation
 *          0 on failure
 */
static uint32_t init_deinit_alloc_host_mem_chunk(struct wlan_objmgr_psoc *psoc,
			struct target_psoc_info *tgt_hdl,
			u_int32_t req_id, u_int32_t idx, u_int32_t num_units,
			u_int32_t unit_len, u_int32_t num_unit_info)
{
	qdf_dma_addr_t paddr;
	uint32_t ichunk = 0;
	struct tgt_info *info;
	qdf_device_t qdf_dev;

	info = (&tgt_hdl->info);

	if (!num_units  || !unit_len)
		return 0;

	qdf_dev = wlan_psoc_get_qdf_dev(psoc);
	if (!qdf_dev)
		return 0;

	/*
	 * We have skip smaller chunks memory allocation for TXBF_CV buffer
	 * as Firmware is expecting continuous memory
	 */
	if (!((num_unit_info & HOST_CONTIGUOUS_MEM_CHUNK_REQUIRED) &&
	      (req_id == TXBF_CV_POOL0 || req_id == TXBF_CV_POOL1 ||
	      req_id == TXBF_CV_POOL2))) {
		ichunk = ((num_units * unit_len) >>
			HOST_MEM_CHUNK_MAX_SIZE_POWER2);
		if (ichunk)
			num_units = num_units / (ichunk + 1);
	}

	info->mem_chunks[idx].vaddr = NULL;
	/* reduce the requested allocation by half until allocation succeeds */
	while (!info->mem_chunks[idx].vaddr && num_units) {
		info->mem_chunks[idx].vaddr = qdf_mem_alloc_consistent(qdf_dev,
				qdf_dev->dev, num_units * unit_len, &paddr);
		if (!info->mem_chunks[idx].vaddr) {
			if (num_unit_info &
					HOST_CONTIGUOUS_MEM_CHUNK_REQUIRED) {
				num_units = 0;
				target_if_err("mem chink alloc failed for %d",
					      idx);
				break;
			}
			/* reduce length by half */
			num_units = (num_units >> 1);
		} else {
			info->mem_chunks[idx].paddr = paddr;
			info->mem_chunks[idx].len = num_units*unit_len;
			info->mem_chunks[idx].req_id =  req_id;
		}
	}
	target_if_debug("req_id %d idx %d num_units %d unit_len %d",
			req_id, idx, num_units, unit_len);

	return num_units;
}

/* Host mem size units, it is used for round-off */
#define HOST_MEM_SIZE_UNIT 4

/**
 *  init_deinit_alloc_host_mem() - allocates amount of memory requested by FW.
 *  @psoc: PSOC object
 *  @tgt_hdl: Target PSOC info
 *  @req_id: request id
 *  @num_units: Number of units
 *  @unit_len: Unit length
 *  @num_unit_info: Num unit info
 *
 *  API to allocate host memory requested by FW
 *
 *  Return: QDF_STATUS_SUCCESS on successful allocation
 *          QDF_STATUS_E_FAILURE on failure
 */
static QDF_STATUS init_deinit_alloc_host_mem(struct wlan_objmgr_psoc *psoc,
		struct target_psoc_info *tgt_hdl, u_int32_t req_id,
		u_int32_t num_units, u_int32_t unit_len,
		u_int32_t num_unit_info)
{
	struct tgt_info *info;
	uint32_t remaining_units;
	uint32_t allocated_units = 0;
	uint32_t idx;

	info = (&tgt_hdl->info);
	/* adjust the length to nearest multiple of unit size */
	unit_len = (unit_len + (HOST_MEM_SIZE_UNIT - 1)) &
				(~(HOST_MEM_SIZE_UNIT - 1));
	idx = info->num_mem_chunks;
	remaining_units = num_units;

	while (remaining_units) {
		if (idx == MAX_MEM_CHUNKS) {
			target_if_err(
				"REACHED MAX CHUNK LIMIT for mem units %d",
					num_units);
			target_if_err(
			"unit len %d requested by FW, only allocated %d",
				unit_len, (num_units - remaining_units));
			info->num_mem_chunks = idx;
			return QDF_STATUS_E_FAILURE;
		}

		if ((tgt_hdl->tif_ops) &&
		    (tgt_hdl->tif_ops->mem_mgr_alloc_chunk))
			allocated_units = tgt_hdl->tif_ops->mem_mgr_alloc_chunk(
						psoc, tgt_hdl, req_id, idx,
						remaining_units,
						unit_len, num_unit_info);
		else
			allocated_units = init_deinit_alloc_host_mem_chunk(
						psoc, tgt_hdl, req_id, idx,
						remaining_units,
						unit_len, num_unit_info);
		if (allocated_units == 0) {
			target_if_err("FAILED TO ALLOC mem unit len %d",
				      unit_len);
			target_if_err("units requested %d units allocated %d",
				      num_units, (num_units - remaining_units));
			info->num_mem_chunks = idx;
			return QDF_STATUS_E_NOMEM;
		}
		remaining_units -= allocated_units;
		++idx;
	}
	info->num_mem_chunks = idx;

	return QDF_STATUS_SUCCESS;
}

/**
 *  init_deinit_alloc_num_units() - allocates num units requested by FW.
 *  @psoc: PSOC object
 *  @tgt_hdl: Target PSOC info
 *  @mem_reqs: pointer to mem req
 *  @num_units: Number
 *  @i: FW priority
 *  @idx: Index
 *
 *  API to allocate num units of host memory requested by FW
 *
 *  Return: QDF_STATUS_SUCCESS on successful allocation
 *          QDF_STATUS_E_FAILURE on failure
 */
static QDF_STATUS init_deinit_alloc_num_units(struct wlan_objmgr_psoc *psoc,
			struct target_psoc_info *tgt_hdl,
			host_mem_req *mem_reqs, uint16_t fw_prio,
			uint16_t idx)
{
	struct tgt_info *info;
	uint32_t num_units;
	QDF_STATUS status;

	if (!tgt_hdl || !mem_reqs) {
		target_if_err("Invalid parameters, tgt_hdl: %pK, mem_reqs: %pK",
			      tgt_hdl, mem_reqs);
		return QDF_STATUS_E_INVAL;
	}

	info = (&tgt_hdl->info);

	if (((fw_prio == FW_MEM_HIGH_PRIORITY) &&
	     (mem_reqs[idx].num_unit_info &
			HOST_CONTIGUOUS_MEM_CHUNK_REQUIRED)) ||
	    ((fw_prio == FW_MEM_LOW_PRIORITY) &&
			(!(mem_reqs[idx].num_unit_info &
				HOST_CONTIGUOUS_MEM_CHUNK_REQUIRED)))) {
		/* First allocate the memory that requires contiguous memory */
		num_units = mem_reqs[idx].num_units;
		if (mem_reqs[idx].num_unit_info) {
			if (mem_reqs[idx].num_unit_info &
					NUM_UNITS_IS_NUM_PEERS) {
				/*
				 * number of units allocated is equal to number
				 * of peers, 1 extra for self peer on target.
				 * this needs to be fixed, host and target can
				 * get out of sync
				 */
				num_units = info->wlan_res_cfg.num_peers + 1;
			}
			if (mem_reqs[idx].num_unit_info &
				NUM_UNITS_IS_NUM_ACTIVE_PEERS) {
				/*
				 * Requesting allocation of memory using
				 * num_active_peers in qcache. if qcache is
				 * disabled in host, then it should allocate
				 * memory for num_peers instead of
				 * num_active_peers.
				 */
				if (info->wlan_res_cfg.num_active_peers)
					num_units =
					info->wlan_res_cfg.num_active_peers + 1;
				else
					num_units =
					info->wlan_res_cfg.num_peers + 1;
			}
		}

		target_if_debug("idx %d req %d  num_units %d num_unit_info %d unit size %d actual units %d",
				idx, mem_reqs[idx].req_id,
				mem_reqs[idx].num_units,
				mem_reqs[idx].num_unit_info,
				mem_reqs[idx].unit_size, num_units);

		status = init_deinit_alloc_host_mem(psoc, tgt_hdl,
				mem_reqs[idx].req_id, num_units,
				mem_reqs[idx].unit_size,
				mem_reqs[idx].num_unit_info);
		if (status == QDF_STATUS_E_FAILURE) {
			target_if_err(
				"psoc:(%pK) num_mem_chunk exceeds supp number",
									psoc);
			return QDF_STATUS_E_FAILURE;
		} else if (status == QDF_STATUS_E_NOMEM) {
			target_if_err("soc:(%pK) mem alloc failure", psoc);
			return QDF_STATUS_E_NOMEM;
		}
	}

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS init_deinit_free_num_units(struct wlan_objmgr_psoc *psoc,
			struct target_psoc_info *tgt_hdl)
{
	struct tgt_info *info;
	qdf_device_t qdf_dev;
	uint32_t idx;
	QDF_STATUS status;

	if (!tgt_hdl) {
		target_if_err("target_psoc_info is null");
		return QDF_STATUS_E_INVAL;
	}

	if ((tgt_hdl->tif_ops) &&
	    (tgt_hdl->tif_ops->mem_mgr_free_chunks)) {
		status = tgt_hdl->tif_ops->mem_mgr_free_chunks(psoc, tgt_hdl);
	} else {
		qdf_dev = wlan_psoc_get_qdf_dev(psoc);
		if (!qdf_dev) {
			target_if_err("qdf_dev is null");
			QDF_BUG(0);
			return QDF_STATUS_E_INVAL;
		}
		info = (&tgt_hdl->info);
		for (idx = 0; idx < info->num_mem_chunks; idx++) {
			qdf_mem_free_consistent(
				qdf_dev, qdf_dev->dev,
				info->mem_chunks[idx].len,
				info->mem_chunks[idx].vaddr,
				info->mem_chunks[idx].paddr,
				qdf_get_dma_mem_context(
					(&(info->mem_chunks[idx])), memctx));

			info->mem_chunks[idx].vaddr = NULL;
			info->mem_chunks[idx].paddr = 0;
			info->mem_chunks[idx].len = 0;
		}
		info->num_mem_chunks = 0;
		status = QDF_STATUS_SUCCESS;
	}

	return status;
}

QDF_STATUS init_deinit_handle_host_mem_req(
		 struct wlan_objmgr_psoc *psoc,
		 struct target_psoc_info *tgt_hdl, uint8_t *event)
{
	uint8_t num_mem_reqs;
	host_mem_req *mem_reqs;
	uint32_t i;
	uint32_t idx;
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	struct common_wmi_handle *wmi_handle;
	struct tgt_info *info;

	if (!tgt_hdl) {
		target_if_err("target_psoc_info is null");
		return QDF_STATUS_E_INVAL;
	}

	wmi_handle = target_psoc_get_wmi_hdl(tgt_hdl);
	info = (&tgt_hdl->info);

	mem_reqs = wmi_extract_host_mem_req_from_service_ready(
					wmi_handle, event, &num_mem_reqs);
	if (!num_mem_reqs)
		return QDF_STATUS_SUCCESS;

	if (num_mem_reqs > MAX_MEM_CHUNKS) {
		target_if_err_rl("num_mem_reqs:%u is out of bounds",
				 num_mem_reqs);
		return QDF_STATUS_E_FAILURE;
	}

	for (i = 0; i < FW_PRIORITY_MAX; i++) {
		for (idx = 0; idx < num_mem_reqs; idx++) {
			status = init_deinit_alloc_num_units(psoc, tgt_hdl,
				mem_reqs, i, idx);
			if (status != QDF_STATUS_SUCCESS)
				return status;
		}
	}

	return status;
}

void init_deinit_derive_band_to_mac_param(
		struct wlan_objmgr_psoc *psoc,
		struct target_psoc_info *tgt_hdl,
		struct wmi_host_pdev_band_to_mac *band_to_mac)
{
	uint8_t i;
	struct wlan_psoc_host_mac_phy_caps *mac_phy_cap;
	struct wlan_psoc_host_hal_reg_capabilities_ext *reg_cap;
	struct tgt_info *info;

	if (!tgt_hdl) {
		target_if_err("target_psoc_info is null ");
		return;
	}

	info = (&tgt_hdl->info);

	reg_cap = ucfg_reg_get_hal_reg_cap(psoc);
	if (!reg_cap) {
		target_if_err("reg cap is NULL");
		return;
	}

	for (i = 0; i < target_psoc_get_num_radios(tgt_hdl); i++) {
		mac_phy_cap = &info->mac_phy_cap[i];
		if (mac_phy_cap->supported_bands ==
			(WMI_HOST_WLAN_5G_CAPABILITY |
					WMI_HOST_WLAN_2G_CAPABILITY)) {
			/*Supports both 5G and 2G. Use freq from both radios*/
			target_if_debug("Supports both 2G and 5G");
			band_to_mac[i].pdev_id = mac_phy_cap->pdev_id;
			band_to_mac[i].start_freq =
					reg_cap[i].low_2ghz_chan;
			band_to_mac[i].end_freq =
					reg_cap[i].high_5ghz_chan;

		} else if (mac_phy_cap->supported_bands ==
				WMI_HOST_WLAN_2G_CAPABILITY) {
			band_to_mac[i].pdev_id = mac_phy_cap->pdev_id;
			band_to_mac[i].start_freq =
					reg_cap[i].low_2ghz_chan;
			band_to_mac[i].end_freq =
					reg_cap[i].high_2ghz_chan;

			reg_cap[mac_phy_cap->phy_id].low_5ghz_chan = 0;
			reg_cap[mac_phy_cap->phy_id].high_5ghz_chan = 0;

			target_if_debug("2G radio - pdev_id = %d start_freq = %d end_freq= %d",
					band_to_mac[i].pdev_id,
					band_to_mac[i].start_freq,
					band_to_mac[i].end_freq);

		} else if (mac_phy_cap->supported_bands ==
					WMI_HOST_WLAN_5G_CAPABILITY) {
			band_to_mac[i].pdev_id = mac_phy_cap->pdev_id;
			band_to_mac[i].start_freq =
						reg_cap[i].low_5ghz_chan;
			band_to_mac[i].end_freq =
						reg_cap[i].high_5ghz_chan;

			reg_cap[mac_phy_cap->phy_id].low_2ghz_chan = 0;
			reg_cap[mac_phy_cap->phy_id].high_2ghz_chan = 0;

			target_if_debug("5G radio -pdev_id = %d start_freq = %d end_freq =%d\n",
					band_to_mac[i].pdev_id,
					band_to_mac[i].start_freq,
					band_to_mac[i].end_freq);
		}
	}
}

void init_deinit_prepare_send_init_cmd(
		 struct wlan_objmgr_psoc *psoc,
		 struct target_psoc_info *tgt_hdl)
{
	struct wmi_init_cmd_param init_param = {0};
	struct tgt_info *info;
	struct common_wmi_handle *wmi_handle;
	QDF_STATUS ret_val;

	if (!tgt_hdl) {
		target_if_err("target_psoc_info is null");
		return;
	}

	wmi_handle = target_psoc_get_wmi_hdl(tgt_hdl);
	info = (&tgt_hdl->info);

	init_param.res_cfg = &info->wlan_res_cfg;
	init_param.num_mem_chunks = info->num_mem_chunks;
	init_param.mem_chunks = info->mem_chunks;

	if (init_deinit_is_service_ext_msg(psoc, tgt_hdl) ==
			QDF_STATUS_SUCCESS) {
		init_param.hw_mode_id = info->preferred_hw_mode;
		/* Temp change, until FW submits support for handling this TLV
		 * For single mode, skip sending hw_mode
		 */
		if (info->preferred_hw_mode == WMI_HOST_HW_MODE_SINGLE)
			init_param.hw_mode_id = WMI_HOST_HW_MODE_MAX;

		init_param.num_band_to_mac = target_psoc_get_num_radios(
								tgt_hdl);

		init_deinit_derive_band_to_mac_param(psoc, tgt_hdl,
						     init_param.band_to_mac);
	} else {
		ret_val = tgt_if_regulatory_modify_freq_range(psoc);
		if (QDF_IS_STATUS_ERROR(ret_val)) {
			target_if_err("Modify freq range is failed");
			return;
		}
	}

	ret_val = target_if_alloc_pdevs(psoc, tgt_hdl);
	if (ret_val != QDF_STATUS_SUCCESS)
		return;

	ret_val = target_if_update_pdev_tgt_info(psoc, tgt_hdl);
	if (ret_val != QDF_STATUS_SUCCESS)
		return;

	target_if_debug("FW version 0x%x ", info->target_caps.fw_version);
	if (init_deinit_is_service_ext_msg(psoc, tgt_hdl) == QDF_STATUS_SUCCESS)
		target_if_debug("0x%x\n",
				info->service_ext_param.fw_build_vers_ext);
	else
		target_if_debug("0x%x\n", info->target_caps.fw_version_1);

	wmi_unified_init_cmd_send(wmi_handle, &init_param);

	/* Set Max scans allowed */
	target_if_scan_set_max_active_scans(psoc,
					    WLAN_MAX_ACTIVE_SCANS_ALLOWED);

	if (wmi_service_enabled(wmi_handle, wmi_service_hw_db2dbm_support))
		wlan_psoc_nif_fw_ext_cap_set(psoc, WLAN_SOC_CEXT_HW_DB2DBM);
}
