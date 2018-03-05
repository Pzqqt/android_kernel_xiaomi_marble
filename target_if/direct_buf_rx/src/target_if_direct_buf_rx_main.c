/*
 * Copyright (c) 2017-2018 The Linux Foundation. All rights reserved.
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

#include "target_if.h"
#include "wmi_unified_api.h"
#include "wlan_lmac_if_def.h"
#include "target_if_direct_buf_rx_main.h"
#include <target_if_direct_buf_rx_api.h>
#include "hal_api.h"
#include <service_ready_util.h>
#include <init_deinit_ucfg.h>

static uint8_t get_num_dbr_modules_per_pdev(struct wlan_objmgr_pdev *pdev)
{
	struct wlan_objmgr_psoc *psoc;
	struct wlan_psoc_host_dbr_ring_caps *dbr_ring_cap;
	uint8_t num_dbr_ring_caps, cap_idx, pdev_id, num_modules;
	struct target_psoc_info *tgt_psoc_info;
	struct wlan_psoc_host_service_ext_param *ext_svc_param;

	psoc = wlan_pdev_get_psoc(pdev);

	if (psoc == NULL) {
		direct_buf_rx_err("psoc is null");
		return 0;
	}

	tgt_psoc_info = wlan_psoc_get_tgt_if_handle(psoc);
	if (tgt_psoc_info == NULL) {
		direct_buf_rx_err("target_psoc_info is null");
		return 0;
	}
	ext_svc_param = target_psoc_get_service_ext_param(tgt_psoc_info);
	num_dbr_ring_caps = ext_svc_param->num_dbr_ring_caps;
	dbr_ring_cap = target_psoc_get_dbr_ring_caps(tgt_psoc_info);
	pdev_id = wlan_objmgr_pdev_get_pdev_id(pdev);
	num_modules = 0;

	for (cap_idx = 0; cap_idx < num_dbr_ring_caps; cap_idx++) {
		if (dbr_ring_cap[cap_idx].pdev_id == pdev_id)
			num_modules++;
	}

	return num_modules;
}

static QDF_STATUS populate_dbr_cap_mod_param(struct wlan_objmgr_pdev *pdev,
			struct direct_buf_rx_module_param *mod_param)
{
	struct wlan_objmgr_psoc *psoc;
	struct wlan_psoc_host_dbr_ring_caps *dbr_ring_cap;
	uint8_t cap_idx;
	bool cap_found = false;
	enum DBR_MODULE mod_id = mod_param->mod_id;
	uint32_t num_dbr_ring_caps, pdev_id;
	struct target_psoc_info *tgt_psoc_info;
	struct wlan_psoc_host_service_ext_param *ext_svc_param;

	psoc = wlan_pdev_get_psoc(pdev);

	if (psoc == NULL) {
		direct_buf_rx_err("psoc is null");
		return QDF_STATUS_E_INVAL;
	}

	tgt_psoc_info = wlan_psoc_get_tgt_if_handle(psoc);
	if (tgt_psoc_info == NULL) {
		direct_buf_rx_err("target_psoc_info is null");
		return QDF_STATUS_E_INVAL;
	}

	ext_svc_param = target_psoc_get_service_ext_param(tgt_psoc_info);
	num_dbr_ring_caps = ext_svc_param->num_dbr_ring_caps;
	dbr_ring_cap = target_psoc_get_dbr_ring_caps(tgt_psoc_info);
	pdev_id = wlan_objmgr_pdev_get_pdev_id(pdev);

	for (cap_idx = 0; cap_idx < num_dbr_ring_caps; cap_idx++) {
		if (dbr_ring_cap[cap_idx].pdev_id == pdev_id) {
			if (dbr_ring_cap[cap_idx].mod_id == mod_id) {
				mod_param->dbr_ring_cap->ring_elems_min =
					dbr_ring_cap[cap_idx].ring_elems_min;
				mod_param->dbr_ring_cap->min_buf_size =
					dbr_ring_cap[cap_idx].min_buf_size;
				mod_param->dbr_ring_cap->min_buf_align =
					dbr_ring_cap[cap_idx].min_buf_align;
				cap_found = true;
			}
		}
	}

	if (!cap_found) {
		direct_buf_rx_err("No cap found for module %d in pdev %d",
				  mod_id, pdev_id);
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS target_if_direct_buf_rx_pdev_create_handler(
	struct wlan_objmgr_pdev *pdev, void *data)
{
	struct direct_buf_rx_pdev_obj *dbr_pdev_obj;
	struct wlan_objmgr_psoc *psoc;
	uint8_t num_modules;
	QDF_STATUS status;

	direct_buf_rx_enter();

	if (pdev == NULL) {
		direct_buf_rx_err("pdev context passed is null");
		return QDF_STATUS_E_INVAL;
	}

	psoc = wlan_pdev_get_psoc(pdev);

	if (psoc == NULL) {
		direct_buf_rx_err("psoc is null");
		return QDF_STATUS_E_INVAL;
	}

	dbr_pdev_obj = qdf_mem_malloc(sizeof(*dbr_pdev_obj));

	if (dbr_pdev_obj == NULL) {
		direct_buf_rx_err("Failed to allocate dir buf rx pdev obj");
		return QDF_STATUS_E_NOMEM;
	}

	direct_buf_rx_info("Dbr pdev obj %pK", dbr_pdev_obj);

	status = wlan_objmgr_pdev_component_obj_attach(pdev,
					WLAN_TARGET_IF_COMP_DIRECT_BUF_RX,
					dbr_pdev_obj, QDF_STATUS_SUCCESS);

	if (status != QDF_STATUS_SUCCESS) {
		direct_buf_rx_err("Failed to attach dir buf rx component %d",
				  status);
		qdf_mem_free(dbr_pdev_obj);
		return status;
	}

	num_modules = get_num_dbr_modules_per_pdev(pdev);
	direct_buf_rx_info("Number of modules = %d pdev %d", num_modules,
			   wlan_objmgr_pdev_get_pdev_id(pdev));
	dbr_pdev_obj->num_modules = num_modules;

	if (!dbr_pdev_obj->num_modules) {
		direct_buf_rx_info("Number of modules = %d", num_modules);
		return QDF_STATUS_SUCCESS;
	}

	dbr_pdev_obj->dbr_mod_param = qdf_mem_malloc(num_modules *
				sizeof(struct direct_buf_rx_module_param));

	if (dbr_pdev_obj->dbr_mod_param == NULL) {
		direct_buf_rx_err("Failed to allocate dir buf rx mod param");
		wlan_objmgr_pdev_component_obj_detach(pdev,
					WLAN_TARGET_IF_COMP_DIRECT_BUF_RX,
					dbr_pdev_obj);
		qdf_mem_free(dbr_pdev_obj);
		return QDF_STATUS_E_NOMEM;
	}


	return QDF_STATUS_SUCCESS;
}

QDF_STATUS target_if_direct_buf_rx_pdev_destroy_handler(
	struct wlan_objmgr_pdev *pdev, void *data)
{
	struct direct_buf_rx_pdev_obj *dbr_pdev_obj;
	QDF_STATUS status;
	uint8_t num_modules, mod_idx;

	if (pdev == NULL) {
		direct_buf_rx_err("pdev context passed is null");
		return QDF_STATUS_E_INVAL;
	}

	dbr_pdev_obj = wlan_objmgr_pdev_get_comp_private_obj(pdev,
				WLAN_TARGET_IF_COMP_DIRECT_BUF_RX);

	if (dbr_pdev_obj == NULL) {
		direct_buf_rx_err("dir buf rx object is null");
		return QDF_STATUS_E_FAILURE;
	}

	num_modules = dbr_pdev_obj->num_modules;
	for (mod_idx = 0; mod_idx < num_modules; mod_idx++)
		target_if_deinit_dbr_ring(pdev, dbr_pdev_obj, mod_idx);

	status = wlan_objmgr_pdev_component_obj_detach(pdev,
					WLAN_TARGET_IF_COMP_DIRECT_BUF_RX,
					dbr_pdev_obj);

	if (status != QDF_STATUS_SUCCESS) {
		direct_buf_rx_err("failed to detach dir buf rx component %d",
				  status);
	}

	qdf_mem_free(dbr_pdev_obj);

	return status;
}

QDF_STATUS target_if_direct_buf_rx_psoc_create_handler(
	struct wlan_objmgr_psoc *psoc, void *data)
{
	struct direct_buf_rx_psoc_obj *dbr_psoc_obj;
	QDF_STATUS status;

	direct_buf_rx_enter();

	if (psoc == NULL) {
		direct_buf_rx_err("psoc context passed is null");
		return QDF_STATUS_E_INVAL;
	}

	dbr_psoc_obj = qdf_mem_malloc(sizeof(*dbr_psoc_obj));

	if (!dbr_psoc_obj) {
		direct_buf_rx_err("failed to alloc dir buf rx psoc obj");
		return QDF_STATUS_E_NOMEM;
	}

	direct_buf_rx_info("Dbr psoc obj %pK", dbr_psoc_obj);

	status = wlan_objmgr_psoc_component_obj_attach(psoc,
			WLAN_TARGET_IF_COMP_DIRECT_BUF_RX, dbr_psoc_obj,
			QDF_STATUS_SUCCESS);

	if (status != QDF_STATUS_SUCCESS) {
		direct_buf_rx_err("Failed to attach dir buf rx component %d",
				  status);
		goto attach_error;
	}

	return status;

attach_error:
	qdf_mem_free(dbr_psoc_obj);

	return status;
}

QDF_STATUS target_if_direct_buf_rx_psoc_destroy_handler(
	struct wlan_objmgr_psoc *psoc, void *data)
{
	QDF_STATUS status;
	struct direct_buf_rx_psoc_obj *dbr_psoc_obj;

	direct_buf_rx_enter();

	dbr_psoc_obj = wlan_objmgr_psoc_get_comp_private_obj(psoc,
				WLAN_TARGET_IF_COMP_DIRECT_BUF_RX);

	if (!dbr_psoc_obj) {
		direct_buf_rx_err("dir buf rx psoc obj is null");
		return QDF_STATUS_E_FAILURE;
	}

	status = wlan_objmgr_psoc_component_obj_detach(psoc,
				WLAN_TARGET_IF_COMP_DIRECT_BUF_RX,
				dbr_psoc_obj);

	if (status != QDF_STATUS_SUCCESS) {
		direct_buf_rx_err("failed to detach dir buf rx component %d",
				  status);
	}

	qdf_mem_free(dbr_psoc_obj);

	return status;
}

static QDF_STATUS target_if_dbr_replenish_ring(struct wlan_objmgr_pdev *pdev,
			struct direct_buf_rx_module_param *mod_param,
			void *aligned_vaddr, uint8_t cookie)
{
	uint64_t *ring_entry;
	uint32_t dw_lo, dw_hi = 0, map_status;
	void *hal_soc, *srng;
	qdf_dma_addr_t paddr;
	struct wlan_objmgr_psoc *psoc;
	struct direct_buf_rx_psoc_obj *dbr_psoc_obj;
	struct direct_buf_rx_ring_cfg *dbr_ring_cfg;
	struct direct_buf_rx_ring_cap *dbr_ring_cap;
	struct direct_buf_rx_buf_info *dbr_buf_pool;

	direct_buf_rx_enter();

	dbr_ring_cfg = mod_param->dbr_ring_cfg;
	dbr_ring_cap = mod_param->dbr_ring_cap;
	dbr_buf_pool = mod_param->dbr_buf_pool;

	psoc = wlan_pdev_get_psoc(pdev);

	if (!psoc) {
		direct_buf_rx_err("psoc is null");
		return QDF_STATUS_E_FAILURE;
	}

	dbr_psoc_obj = wlan_objmgr_psoc_get_comp_private_obj(psoc,
				WLAN_TARGET_IF_COMP_DIRECT_BUF_RX);

	if (dbr_psoc_obj == NULL) {
		direct_buf_rx_err("dir buf rx psoc object is null");
		return QDF_STATUS_E_FAILURE;
	}

	hal_soc = dbr_psoc_obj->hal_soc;
	srng = dbr_ring_cfg->srng;
	if (!aligned_vaddr) {
		direct_buf_rx_err("aligned vaddr is null");
		return QDF_STATUS_SUCCESS;
	}

	map_status = qdf_mem_map_nbytes_single(dbr_psoc_obj->osdev,
					       aligned_vaddr,
					       QDF_DMA_FROM_DEVICE,
					       dbr_ring_cap->min_buf_size,
					       &paddr);
	if (map_status) {
		direct_buf_rx_err("mem map failed status = %d", map_status);
		return QDF_STATUS_E_FAILURE;
	}

	QDF_ASSERT(!((uint64_t)paddr & dbr_ring_cap->min_buf_align));
	dbr_buf_pool[cookie].paddr = paddr;

	hal_srng_access_start(hal_soc, srng);
	ring_entry = hal_srng_src_get_next(hal_soc, srng);
	QDF_ASSERT(ring_entry != NULL);
	dw_lo = (uint64_t)paddr & 0xFFFFFFFF;
	WMI_HOST_DBR_RING_ADDR_HI_SET(dw_hi, (uint64_t)paddr >> 32);
	WMI_HOST_DBR_DATA_ADDR_HI_HOST_DATA_SET(dw_hi, cookie);
	direct_buf_rx_info("Cookie = %d", cookie);
	direct_buf_rx_info("dw_lo = %x dw_hi = %x", dw_lo, dw_hi);
	*ring_entry = (uint64_t)dw_hi << 32 | dw_lo;
	direct_buf_rx_info("Valid ring entry");
	hal_srng_access_end(hal_soc, srng);

	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS target_if_dbr_fill_ring(struct wlan_objmgr_pdev *pdev,
			  struct direct_buf_rx_module_param *mod_param)
{
	uint8_t idx;
	void *buf, *buf_aligned;
	struct direct_buf_rx_ring_cfg *dbr_ring_cfg;
	struct direct_buf_rx_ring_cap *dbr_ring_cap;
	struct direct_buf_rx_buf_info *dbr_buf_pool;
	QDF_STATUS status;

	direct_buf_rx_enter();

	dbr_ring_cfg = mod_param->dbr_ring_cfg;
	dbr_ring_cap = mod_param->dbr_ring_cap;
	dbr_buf_pool = mod_param->dbr_buf_pool;

	for (idx = 0; idx < dbr_ring_cfg->num_ptr - 1; idx++) {
		buf = qdf_mem_malloc(dbr_ring_cap->min_buf_size +
				dbr_ring_cap->min_buf_align - 1);
		if (!buf) {
			direct_buf_rx_err("dir buf rx ring buf alloc failed");
			return QDF_STATUS_E_NOMEM;
		}
		dbr_buf_pool[idx].vaddr = buf;
		buf_aligned = (void *)(uintptr_t)qdf_roundup(
				(uint64_t)(uintptr_t)buf, DBR_RING_BASE_ALIGN);
		dbr_buf_pool[idx].offset = buf_aligned - buf;
		dbr_buf_pool[idx].cookie = idx;
		status = target_if_dbr_replenish_ring(pdev, mod_param,
						      buf_aligned, idx);
		if (QDF_IS_STATUS_ERROR(status)) {
			direct_buf_rx_err("replenish failed with status : %d",
					  status);
			qdf_mem_free(buf);
			return QDF_STATUS_E_FAILURE;
		}
	}

	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS target_if_dbr_init_ring(struct wlan_objmgr_pdev *pdev,
			struct direct_buf_rx_module_param *mod_param)
{
	void *srng;
	uint32_t num_entries, ring_alloc_size, max_entries, entry_size;
	qdf_dma_addr_t paddr;
	struct hal_srng_params ring_params = {0};
	struct wlan_objmgr_psoc *psoc;
	struct direct_buf_rx_psoc_obj *dbr_psoc_obj;
	struct direct_buf_rx_ring_cap *dbr_ring_cap;
	struct direct_buf_rx_ring_cfg *dbr_ring_cfg;
	QDF_STATUS status;

	direct_buf_rx_enter();

	psoc = wlan_pdev_get_psoc(pdev);

	if (!psoc) {
		direct_buf_rx_err("psoc is null");
		return QDF_STATUS_E_FAILURE;
	}

	dbr_psoc_obj = wlan_objmgr_psoc_get_comp_private_obj(psoc,
				WLAN_TARGET_IF_COMP_DIRECT_BUF_RX);

	if (dbr_psoc_obj == NULL) {
		direct_buf_rx_err("dir buf rx psoc object is null");
		return QDF_STATUS_E_FAILURE;
	}

	if (dbr_psoc_obj->hal_soc == NULL ||
	    dbr_psoc_obj->osdev == NULL) {
		direct_buf_rx_err("dir buf rx target attach failed");
		return QDF_STATUS_E_FAILURE;
	}

	max_entries = hal_srng_max_entries(dbr_psoc_obj->hal_soc,
					   DIR_BUF_RX_DMA_SRC);
	entry_size = hal_srng_get_entrysize(dbr_psoc_obj->hal_soc,
					    DIR_BUF_RX_DMA_SRC);
	direct_buf_rx_info("Max Entries = %d", max_entries);
	direct_buf_rx_info("Entry Size = %d", entry_size);

	status = populate_dbr_cap_mod_param(pdev, mod_param);
	if (QDF_IS_STATUS_ERROR(status)) {
		direct_buf_rx_err("Module cap population failed");
		return QDF_STATUS_E_FAILURE;
	}

	dbr_ring_cap = mod_param->dbr_ring_cap;
	dbr_ring_cfg = mod_param->dbr_ring_cfg;
	num_entries = dbr_ring_cap->ring_elems_min > max_entries ?
			max_entries : dbr_ring_cap->ring_elems_min;
	direct_buf_rx_info("Num entries = %d", num_entries);
	dbr_ring_cfg->num_ptr = num_entries;
	mod_param->dbr_buf_pool = qdf_mem_malloc(num_entries * sizeof(
				struct direct_buf_rx_buf_info));
	if (!mod_param->dbr_buf_pool) {
		direct_buf_rx_err("dir buf rx buf pool alloc failed");
		return QDF_STATUS_E_NOMEM;
	}

	ring_alloc_size = (num_entries * entry_size) + DBR_RING_BASE_ALIGN - 1;
	dbr_ring_cfg->ring_alloc_size = ring_alloc_size;
	direct_buf_rx_info("dbr_psoc_obj %pK", dbr_psoc_obj);
	dbr_ring_cfg->base_vaddr_unaligned = qdf_mem_alloc_consistent(
		dbr_psoc_obj->osdev, dbr_psoc_obj->osdev->dev, ring_alloc_size,
		&paddr);
	direct_buf_rx_info("vaddr aligned allocated");
	dbr_ring_cfg->base_paddr_unaligned = paddr;
	if (!dbr_ring_cfg->base_vaddr_unaligned) {
		direct_buf_rx_err("dir buf rx vaddr alloc failed");
		qdf_mem_free(mod_param->dbr_buf_pool);
		return QDF_STATUS_E_NOMEM;
	}

	/* Alignment is defined to 8 for now. Will be advertised by FW */
	dbr_ring_cfg->base_vaddr_aligned = (void *)(uintptr_t)qdf_roundup(
		(uint64_t)(uintptr_t)dbr_ring_cfg->base_vaddr_unaligned,
		DBR_RING_BASE_ALIGN);
	ring_params.ring_base_vaddr = dbr_ring_cfg->base_vaddr_aligned;
	dbr_ring_cfg->base_paddr_aligned = qdf_roundup(
		(uint64_t)dbr_ring_cfg->base_paddr_unaligned,
		DBR_RING_BASE_ALIGN);
	ring_params.ring_base_paddr =
		(qdf_dma_addr_t)dbr_ring_cfg->base_paddr_aligned;
	ring_params.num_entries = num_entries;
	srng = hal_srng_setup(dbr_psoc_obj->hal_soc, DIR_BUF_RX_DMA_SRC, 0,
			      wlan_objmgr_pdev_get_pdev_id(pdev), &ring_params);

	if (!srng) {
		direct_buf_rx_err("srng setup failed");
		qdf_mem_free(mod_param->dbr_buf_pool);
		qdf_mem_free_consistent(dbr_psoc_obj->osdev,
					dbr_psoc_obj->osdev->dev,
					ring_alloc_size,
					dbr_ring_cfg->base_vaddr_unaligned,
			(qdf_dma_addr_t)dbr_ring_cfg->base_paddr_unaligned, 0);
		return QDF_STATUS_E_FAILURE;
	}
	dbr_ring_cfg->srng = srng;
	dbr_ring_cfg->tail_idx_addr =
		hal_srng_get_tp_addr(dbr_psoc_obj->hal_soc, srng);
	dbr_ring_cfg->head_idx_addr =
		hal_srng_get_hp_addr(dbr_psoc_obj->hal_soc, srng);
	dbr_ring_cfg->buf_size = dbr_ring_cap->min_buf_size;

	return target_if_dbr_fill_ring(pdev, mod_param);
}

static QDF_STATUS target_if_dbr_init_srng(struct wlan_objmgr_pdev *pdev,
			struct direct_buf_rx_module_param *mod_param)
{
	QDF_STATUS status;

	direct_buf_rx_info("Init DBR srng");

	if (!mod_param) {
		direct_buf_rx_err("dir buf rx module param is null");
		return QDF_STATUS_E_INVAL;
	}

	mod_param->dbr_ring_cap = qdf_mem_malloc(sizeof(
					struct direct_buf_rx_ring_cap));

	if (!mod_param->dbr_ring_cap) {
		direct_buf_rx_err("Ring cap alloc failed");
		return QDF_STATUS_E_NOMEM;
	}

	/* Allocate memory for DBR Ring Config */
	mod_param->dbr_ring_cfg = qdf_mem_malloc(sizeof(
					struct direct_buf_rx_ring_cfg));

	if (!mod_param->dbr_ring_cfg) {
		direct_buf_rx_err("Ring config alloc failed");
		qdf_mem_free(mod_param->dbr_ring_cap);
		return QDF_STATUS_E_NOMEM;
	}

	status = target_if_dbr_init_ring(pdev, mod_param);

	if (QDF_IS_STATUS_ERROR(status)) {
		direct_buf_rx_err("DBR ring init failed");
		qdf_mem_free(mod_param->dbr_ring_cfg);
		qdf_mem_free(mod_param->dbr_ring_cap);
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS target_if_dbr_cfg_tgt(struct wlan_objmgr_pdev *pdev,
			struct direct_buf_rx_module_param *mod_param)
{
	QDF_STATUS status;
	struct wlan_objmgr_psoc *psoc;
	void *wmi_hdl;
	struct direct_buf_rx_cfg_req dbr_cfg_req = {0};
	struct direct_buf_rx_ring_cfg *dbr_ring_cfg;
	struct direct_buf_rx_ring_cap *dbr_ring_cap;

	direct_buf_rx_enter();

	psoc = wlan_pdev_get_psoc(pdev);
	if (!psoc) {
		direct_buf_rx_err("psoc is null");
		return QDF_STATUS_E_FAILURE;
	}

	dbr_ring_cfg = mod_param->dbr_ring_cfg;
	dbr_ring_cap = mod_param->dbr_ring_cap;
	wmi_hdl = ucfg_get_pdev_wmi_handle(pdev);
	if (!wmi_hdl) {
		direct_buf_rx_err("WMI handle null. Can't send WMI CMD");
		return QDF_STATUS_E_INVAL;
	}

	direct_buf_rx_debug("Sending DBR Ring CFG to target");
	dbr_cfg_req.pdev_id = wlan_objmgr_pdev_get_pdev_id(pdev);
	/* Module ID numbering starts from 1 in FW. need to fix it */
	dbr_cfg_req.mod_id = mod_param->mod_id;
	dbr_cfg_req.base_paddr_lo = (uint64_t)dbr_ring_cfg->base_paddr_aligned
						& 0xFFFFFFFF;
	dbr_cfg_req.base_paddr_hi = (uint64_t)dbr_ring_cfg->base_paddr_aligned
						& 0xFFFFFFFF00000000;
	dbr_cfg_req.head_idx_paddr_lo = (uint64_t)dbr_ring_cfg->head_idx_addr
						& 0xFFFFFFFF;
	dbr_cfg_req.head_idx_paddr_hi = (uint64_t)dbr_ring_cfg->head_idx_addr
						& 0xFFFFFFFF00000000;
	dbr_cfg_req.tail_idx_paddr_lo = (uint64_t)dbr_ring_cfg->tail_idx_addr
						& 0xFFFFFFFF;
	dbr_cfg_req.tail_idx_paddr_hi = (uint64_t)dbr_ring_cfg->tail_idx_addr
						& 0xFFFFFFFF00000000;
	dbr_cfg_req.num_elems = dbr_ring_cap->ring_elems_min;
	dbr_cfg_req.buf_size = dbr_ring_cap->min_buf_size;
	dbr_cfg_req.num_resp_per_event = DBR_NUM_RESP_PER_EVENT;
	dbr_cfg_req.event_timeout_ms = DBR_EVENT_TIMEOUT_IN_MS;
	direct_buf_rx_info("pdev id %d mod id %d base addr lo %x\n"
			   "base addr hi %x head idx addr lo %x\n"
			   "head idx addr hi %x tail idx addr lo %x\n"
			   "tail idx addr hi %x num ptr %d\n"
			   "num resp %d event timeout %d\n",
			   dbr_cfg_req.pdev_id, dbr_cfg_req.mod_id,
			   dbr_cfg_req.base_paddr_lo, dbr_cfg_req.base_paddr_hi,
			   dbr_cfg_req.head_idx_paddr_lo,
			   dbr_cfg_req.head_idx_paddr_hi,
			   dbr_cfg_req.tail_idx_paddr_lo,
			   dbr_cfg_req.tail_idx_paddr_hi,
			   dbr_cfg_req.num_elems,
			   dbr_cfg_req.num_resp_per_event,
			   dbr_cfg_req.event_timeout_ms);
	status = wmi_unified_dbr_ring_cfg(wmi_hdl, &dbr_cfg_req);

	return status;
}

static QDF_STATUS target_if_init_dbr_ring(struct wlan_objmgr_pdev *pdev,
				struct direct_buf_rx_pdev_obj *dbr_pdev_obj,
				enum DBR_MODULE mod_id)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	struct direct_buf_rx_module_param *mod_param;

	direct_buf_rx_info("Init DBR ring for module %d", mod_id);

	if (!dbr_pdev_obj) {
		direct_buf_rx_err("dir buf rx object is null");
		return QDF_STATUS_E_INVAL;
	}

	mod_param = &(dbr_pdev_obj->dbr_mod_param[mod_id]);

	if (!mod_param) {
		direct_buf_rx_err("dir buf rx module param is null");
		return QDF_STATUS_E_FAILURE;
	}

	direct_buf_rx_info("mod_param %pK", mod_param);

	mod_param->mod_id = mod_id;

	/* Initialize DMA ring now */
	status = target_if_dbr_init_srng(pdev, mod_param);
	if (QDF_IS_STATUS_ERROR(status)) {
		direct_buf_rx_err("DBR ring init failed %d", status);
		return status;
	}

	/* Send CFG request command to firmware */
	status = target_if_dbr_cfg_tgt(pdev, mod_param);
	if (QDF_IS_STATUS_ERROR(status)) {
		direct_buf_rx_err("DBR config to target failed %d", status);
		goto dbr_srng_init_failed;
	}

	return QDF_STATUS_SUCCESS;

dbr_srng_init_failed:
	target_if_deinit_dbr_ring(pdev, dbr_pdev_obj, mod_id);
	return status;
}

QDF_STATUS target_if_direct_buf_rx_module_register(
			struct wlan_objmgr_pdev *pdev, uint8_t mod_id,
			int (*dbr_rsp_handler)(struct wlan_objmgr_pdev *pdev,
				struct direct_buf_rx_data *dbr_data))
{
	QDF_STATUS status;
	struct direct_buf_rx_pdev_obj *dbr_pdev_obj;

	if (pdev == NULL) {
		direct_buf_rx_err("pdev context passed is null");
		return QDF_STATUS_E_INVAL;
	}

	if (dbr_rsp_handler == NULL) {
		direct_buf_rx_err("Response handler is null");
		return QDF_STATUS_E_INVAL;
	}

	if (mod_id >= DBR_MODULE_MAX) {
		direct_buf_rx_err("Invalid module id");
		return QDF_STATUS_E_INVAL;
	}

	dbr_pdev_obj = wlan_objmgr_pdev_get_comp_private_obj(pdev,
				WLAN_TARGET_IF_COMP_DIRECT_BUF_RX);

	if (dbr_pdev_obj == NULL) {
		direct_buf_rx_err("dir buf rx object is null");
		return QDF_STATUS_E_FAILURE;
	}
	direct_buf_rx_info("Dbr pdev obj %pK", dbr_pdev_obj);

	dbr_pdev_obj->dbr_mod_param[mod_id].dbr_rsp_handler =
			dbr_rsp_handler;

	status = target_if_init_dbr_ring(pdev, dbr_pdev_obj,
					 (enum DBR_MODULE)mod_id);

	return status;
}

static void *target_if_dbr_vaddr_lookup(
			struct direct_buf_rx_module_param *mod_param,
			qdf_dma_addr_t paddr, uint32_t cookie)
{
	struct direct_buf_rx_buf_info *dbr_buf_pool;

	dbr_buf_pool = mod_param->dbr_buf_pool;

	if (dbr_buf_pool[cookie].paddr == paddr) {
		return dbr_buf_pool[cookie].vaddr +
				dbr_buf_pool[cookie].offset;
	}

	direct_buf_rx_err("Incorrect paddr found on cookie slot");
	return NULL;
}

static QDF_STATUS target_if_get_dbr_data(struct wlan_objmgr_pdev *pdev,
			struct direct_buf_rx_module_param *mod_param,
			struct direct_buf_rx_rsp *dbr_rsp,
			struct direct_buf_rx_data *dbr_data,
			uint8_t idx, uint32_t *cookie)
{
	qdf_dma_addr_t paddr = 0;
	uint32_t addr_hi;
	struct direct_buf_rx_psoc_obj *dbr_psoc_obj;
	struct direct_buf_rx_ring_cap *dbr_ring_cap;
	struct wlan_objmgr_psoc *psoc;

	psoc = wlan_pdev_get_psoc(pdev);
	if (!psoc) {
		direct_buf_rx_err("psoc is null");
		return QDF_STATUS_E_FAILURE;
	}

	dbr_psoc_obj = wlan_objmgr_psoc_get_comp_private_obj(psoc,
				WLAN_TARGET_IF_COMP_DIRECT_BUF_RX);

	if (dbr_psoc_obj == NULL) {
		direct_buf_rx_err("dir buf rx psoc object is null");
		return QDF_STATUS_E_FAILURE;
	}

	dbr_ring_cap = mod_param->dbr_ring_cap;
	addr_hi = (uint64_t)WMI_HOST_DBR_DATA_ADDR_HI_GET(
				dbr_rsp->dbr_entries[idx].paddr_hi);
	paddr = (qdf_dma_addr_t)((uint64_t)addr_hi << 32 |
				  dbr_rsp->dbr_entries[idx].paddr_lo);
	*cookie = WMI_HOST_DBR_DATA_ADDR_HI_HOST_DATA_GET(
				dbr_rsp->dbr_entries[idx].paddr_hi);
	direct_buf_rx_info("Cookie = %d", *cookie);
	dbr_data->vaddr = target_if_dbr_vaddr_lookup(mod_param, paddr, *cookie);
	direct_buf_rx_info("Vaddr look up = %x", dbr_data->vaddr);
	dbr_data->dbr_len = dbr_rsp->dbr_entries[idx].len;
	qdf_mem_unmap_nbytes_single(dbr_psoc_obj->osdev, (qdf_dma_addr_t)paddr,
				    QDF_DMA_FROM_DEVICE,
				    dbr_ring_cap->min_buf_size);

	return QDF_STATUS_SUCCESS;
}

static int target_if_direct_buf_rx_rsp_event_handler(ol_scn_t scn,
						uint8_t *data_buf,
						uint32_t data_len)
{
	int ret = 0;
	uint8_t i = 0;
	QDF_STATUS status;
	uint32_t cookie = 0;
	struct direct_buf_rx_rsp dbr_rsp = {0};
	struct direct_buf_rx_data dbr_data = {0};
	struct wlan_objmgr_psoc *psoc;
	struct wlan_objmgr_pdev *pdev;
	struct direct_buf_rx_buf_info *dbr_buf_pool;
	struct direct_buf_rx_pdev_obj *dbr_pdev_obj;
	struct direct_buf_rx_module_param *mod_param;

	direct_buf_rx_enter();

	psoc = target_if_get_psoc_from_scn_hdl(scn);
	if (!psoc) {
		direct_buf_rx_err("psoc is null");
		return QDF_STATUS_E_FAILURE;
	}

	if (wmi_extract_dbr_buf_release_fixed(GET_WMI_HDL_FROM_PSOC(psoc),
			data_buf, &dbr_rsp) != QDF_STATUS_SUCCESS) {
		direct_buf_rx_err("unable to extract DBR rsp fixed param");
		return QDF_STATUS_E_FAILURE;
	}

	direct_buf_rx_info("Num buf release entry = %d",
			   dbr_rsp.num_buf_release_entry);

	pdev = wlan_objmgr_get_pdev_by_id(psoc, dbr_rsp.pdev_id,
					  WLAN_DIRECT_BUF_RX_ID);
	if (!pdev) {
		direct_buf_rx_err("pdev is null");
		wlan_objmgr_pdev_release_ref(pdev, WLAN_DIRECT_BUF_RX_ID);
		return QDF_STATUS_E_INVAL;
	}

	dbr_pdev_obj = wlan_objmgr_pdev_get_comp_private_obj(pdev,
				WLAN_TARGET_IF_COMP_DIRECT_BUF_RX);

	if (dbr_pdev_obj == NULL) {
		direct_buf_rx_err("dir buf rx object is null");
		wlan_objmgr_pdev_release_ref(pdev, WLAN_DIRECT_BUF_RX_ID);
		return QDF_STATUS_E_FAILURE;
	}

	mod_param = &(dbr_pdev_obj->dbr_mod_param[dbr_rsp.mod_id]);

	if (!mod_param) {
		direct_buf_rx_err("dir buf rx module param is null");
		wlan_objmgr_pdev_release_ref(pdev, WLAN_DIRECT_BUF_RX_ID);
		return QDF_STATUS_E_FAILURE;
	}

	dbr_buf_pool = mod_param->dbr_buf_pool;
	dbr_rsp.dbr_entries = qdf_mem_malloc(dbr_rsp.num_buf_release_entry *
					sizeof(struct direct_buf_rx_entry));

	for (i = 0; i < dbr_rsp.num_buf_release_entry; i++) {
		if (wmi_extract_dbr_buf_release_entry(
			GET_WMI_HDL_FROM_PSOC(psoc), data_buf, i,
			&dbr_rsp.dbr_entries[i]) != QDF_STATUS_SUCCESS) {
			direct_buf_rx_err("Unable to extract DBR buf entry %d",
					  i+1);
			wlan_objmgr_pdev_release_ref(pdev,
						     WLAN_DIRECT_BUF_RX_ID);
			return QDF_STATUS_E_FAILURE;
		}
		status = target_if_get_dbr_data(pdev, mod_param, &dbr_rsp,
						&dbr_data, i, &cookie);

		if (QDF_IS_STATUS_ERROR(status)) {
			direct_buf_rx_err("DBR data get failed");
			wlan_objmgr_pdev_release_ref(pdev,
						     WLAN_DIRECT_BUF_RX_ID);
			return QDF_STATUS_E_FAILURE;
		}
		ret = mod_param->dbr_rsp_handler(pdev, &dbr_data);
		status = target_if_dbr_replenish_ring(pdev, mod_param,
						      dbr_data.vaddr, cookie);
		if (QDF_IS_STATUS_ERROR(status)) {
			direct_buf_rx_err("dir buf rx ring replenish failed");
			wlan_objmgr_pdev_release_ref(pdev,
						     WLAN_DIRECT_BUF_RX_ID);
			return QDF_STATUS_E_FAILURE;
		}
	}

	wlan_objmgr_pdev_release_ref(pdev, WLAN_DIRECT_BUF_RX_ID);

	return ret;
}

static QDF_STATUS target_if_dbr_empty_ring(struct wlan_objmgr_pdev *pdev,
			struct direct_buf_rx_psoc_obj *dbr_psoc_obj,
			struct direct_buf_rx_module_param *mod_param)
{
	uint8_t idx;
	struct direct_buf_rx_ring_cfg *dbr_ring_cfg;
	struct direct_buf_rx_ring_cap *dbr_ring_cap;
	struct direct_buf_rx_buf_info *dbr_buf_pool;

	direct_buf_rx_enter();
	dbr_ring_cfg = mod_param->dbr_ring_cfg;
	dbr_ring_cap = mod_param->dbr_ring_cap;
	dbr_buf_pool = mod_param->dbr_buf_pool;

	direct_buf_rx_info("dbr_ring_cfg %pK, dbr_ring_cap %pK dbr_buf_pool %pK",
			   dbr_ring_cfg, dbr_ring_cap, dbr_buf_pool);

	for (idx = 0; idx < dbr_ring_cfg->num_ptr - 1; idx++) {
		direct_buf_rx_info("dbr buf pool unmap and free for ptr %d",
				   idx);
		qdf_mem_unmap_nbytes_single(dbr_psoc_obj->osdev,
			(qdf_dma_addr_t)dbr_buf_pool[idx].paddr,
			QDF_DMA_FROM_DEVICE,
			dbr_ring_cap->min_buf_size);
		qdf_mem_free(dbr_buf_pool[idx].vaddr);
	}

	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS target_if_dbr_deinit_ring(struct wlan_objmgr_pdev *pdev,
			struct direct_buf_rx_module_param *mod_param)
{
	struct wlan_objmgr_psoc *psoc;
	struct direct_buf_rx_psoc_obj *dbr_psoc_obj;
	struct direct_buf_rx_ring_cfg *dbr_ring_cfg;

	direct_buf_rx_enter();
	psoc = wlan_pdev_get_psoc(pdev);
	if (!psoc) {
		direct_buf_rx_err("psoc is null");
		return QDF_STATUS_E_FAILURE;
	}

	dbr_psoc_obj = wlan_objmgr_psoc_get_comp_private_obj(psoc,
				WLAN_TARGET_IF_COMP_DIRECT_BUF_RX);

	if (dbr_psoc_obj == NULL) {
		direct_buf_rx_err("dir buf rx psoc object is null");
		return QDF_STATUS_E_FAILURE;
	}
	direct_buf_rx_info("dbr_psoc_obj %pK", dbr_psoc_obj);

	dbr_ring_cfg = mod_param->dbr_ring_cfg;
	if (dbr_ring_cfg) {
		target_if_dbr_empty_ring(pdev, dbr_psoc_obj, mod_param);
		hal_srng_cleanup(dbr_psoc_obj->hal_soc, dbr_ring_cfg->srng);
		qdf_mem_free_consistent(dbr_psoc_obj->osdev,
					dbr_psoc_obj->osdev->dev,
					dbr_ring_cfg->ring_alloc_size,
					dbr_ring_cfg->base_vaddr_unaligned,
			(qdf_dma_addr_t)dbr_ring_cfg->base_paddr_unaligned, 0);
	}

	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS target_if_dbr_deinit_srng(
			struct wlan_objmgr_pdev *pdev,
			struct direct_buf_rx_module_param *mod_param)
{
	struct direct_buf_rx_buf_info *dbr_buf_pool;

	direct_buf_rx_enter();
	dbr_buf_pool = mod_param->dbr_buf_pool;
	direct_buf_rx_info("dbr buf pool %pK", dbr_buf_pool);
	target_if_dbr_deinit_ring(pdev, mod_param);
	qdf_mem_free(dbr_buf_pool);
	dbr_buf_pool = NULL;

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS target_if_deinit_dbr_ring(struct wlan_objmgr_pdev *pdev,
			struct direct_buf_rx_pdev_obj *dbr_pdev_obj,
			enum DBR_MODULE mod_id)
{
	struct direct_buf_rx_module_param *mod_param;
	struct direct_buf_rx_ring_cap *dbr_ring_cap;

	direct_buf_rx_enter();
	mod_param = &(dbr_pdev_obj->dbr_mod_param[mod_id]);

	if (!mod_param) {
		direct_buf_rx_err("dir buf rx module param is null");
		return QDF_STATUS_E_FAILURE;
	}
	direct_buf_rx_info("mod_param %pK", mod_param);

	dbr_ring_cap = mod_param->dbr_ring_cap;
	direct_buf_rx_info("dbr_ring_cap %pK", dbr_ring_cap);
	target_if_dbr_deinit_srng(pdev, mod_param);
	qdf_mem_free(dbr_ring_cap);
	dbr_ring_cap = NULL;
	qdf_mem_free(mod_param);
	mod_param = NULL;

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS target_if_direct_buf_rx_register_events(
				struct wlan_objmgr_psoc *psoc)
{
	int ret;

	if (!psoc || !GET_WMI_HDL_FROM_PSOC(psoc)) {
		direct_buf_rx_err("psoc or psoc->tgt_if_handle is null");
		return QDF_STATUS_E_INVAL;
	}

	ret = wmi_unified_register_event_handler(GET_WMI_HDL_FROM_PSOC(psoc),
				wmi_dma_buf_release_event_id,
				target_if_direct_buf_rx_rsp_event_handler,
				WMI_RX_UMAC_CTX);

	if (ret)
		direct_buf_rx_info("event handler not supported", ret);

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS target_if_direct_buf_rx_unregister_events(
				struct wlan_objmgr_psoc *psoc)
{
	if (!psoc || !GET_WMI_HDL_FROM_PSOC(psoc)) {
		direct_buf_rx_err("psoc or psoc->tgt_if_handle is null");
		return QDF_STATUS_E_INVAL;
	}

	wmi_unified_unregister_event_handler(GET_WMI_HDL_FROM_PSOC(psoc),
					     wmi_dma_buf_release_event_id);

	return QDF_STATUS_SUCCESS;
}
