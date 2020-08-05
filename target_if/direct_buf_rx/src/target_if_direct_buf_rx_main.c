/*
 * Copyright (c) 2017-2020 The Linux Foundation. All rights reserved.
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
#include "wlan_lmac_if_def.h"
#include "target_if_direct_buf_rx_main.h"
#include <target_if_direct_buf_rx_api.h>
#include "hal_api.h"
#include <service_ready_util.h>
#include <init_deinit_lmac.h>

/**
 * struct module_name : Module name information structure
 * @module_name_str : Module name subscribing to DBR
 */
struct module_name {
	unsigned char module_name_str[QDF_MAX_NAME_SIZE];
};

static const struct module_name g_dbr_module_name[DBR_MODULE_MAX] = {
	[DBR_MODULE_SPECTRAL] = {"SPECTRAL"},
	[DBR_MODULE_CFR]      = {"CFR"},
};

static uint8_t get_num_dbr_modules_per_pdev(struct wlan_objmgr_pdev *pdev)
{
	struct wlan_objmgr_psoc *psoc;
	struct wlan_psoc_host_dbr_ring_caps *dbr_ring_cap;
	uint8_t num_dbr_ring_caps, cap_idx, pdev_id, num_modules;
	struct target_psoc_info *tgt_psoc_info;

	psoc = wlan_pdev_get_psoc(pdev);

	if (!psoc) {
		direct_buf_rx_err("psoc is null");
		return 0;
	}

	tgt_psoc_info = wlan_psoc_get_tgt_if_handle(psoc);
	if (!tgt_psoc_info) {
		direct_buf_rx_err("target_psoc_info is null");
		return 0;
	}
	num_dbr_ring_caps = target_psoc_get_num_dbr_ring_caps(tgt_psoc_info);
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

	psoc = wlan_pdev_get_psoc(pdev);

	if (!psoc) {
		direct_buf_rx_err("psoc is null");
		return QDF_STATUS_E_INVAL;
	}

	tgt_psoc_info = wlan_psoc_get_tgt_if_handle(psoc);
	if (!tgt_psoc_info) {
		direct_buf_rx_err("target_psoc_info is null");
		return QDF_STATUS_E_INVAL;
	}

	num_dbr_ring_caps = target_psoc_get_num_dbr_ring_caps(tgt_psoc_info);
	dbr_ring_cap = target_psoc_get_dbr_ring_caps(tgt_psoc_info);
	pdev_id = mod_param->pdev_id;

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
#ifdef DIRECT_BUF_RX_DEBUG
static inline struct direct_buf_rx_module_debug *
target_if_get_dbr_mod_debug_from_dbr_pdev_obj(
	struct direct_buf_rx_pdev_obj *dbr_pdev_obj,
	uint8_t mod_id)
{
	if (!dbr_pdev_obj) {
		direct_buf_rx_err("dir buf rx object is null");
		return NULL;
	}

	if (mod_id >= DBR_MODULE_MAX) {
		direct_buf_rx_err("Invalid module id");
		return NULL;
	}

	if (!dbr_pdev_obj->dbr_mod_debug) {
		direct_buf_rx_err("dbr_pdev_obj->dbr_mod_debug is NULL");
		return NULL;
	}

	if (mod_id >= dbr_pdev_obj->num_modules) {
		direct_buf_rx_err("Module %d not supported in target", mod_id);
		return NULL;
	}
	return &dbr_pdev_obj->dbr_mod_debug[mod_id];
}

static inline struct direct_buf_rx_module_debug *
target_if_get_dbr_mod_debug_from_pdev(
	struct wlan_objmgr_pdev *pdev,
	uint8_t mod_id)
{
	struct direct_buf_rx_pdev_obj *dbr_pdev_obj;

	if (!pdev) {
		direct_buf_rx_err("pdev is null");
		return NULL;
	}

	dbr_pdev_obj = wlan_objmgr_pdev_get_comp_private_obj(
				pdev, WLAN_TARGET_IF_COMP_DIRECT_BUF_RX);

	return target_if_get_dbr_mod_debug_from_dbr_pdev_obj(
				dbr_pdev_obj, mod_id);
}
#endif

#ifdef DIRECT_BUF_RX_DEBUG
#define RING_DEBUG_EVENT_NAME_SIZE 12
static const unsigned char
g_dbr_ring_debug_event[DBR_RING_DEBUG_EVENT_MAX][RING_DEBUG_EVENT_NAME_SIZE] = {
	[DBR_RING_DEBUG_EVENT_RX]                  = "Rx",
	[DBR_RING_DEBUG_EVENT_REPLENISH_RING]      = "Replenish",
};

/**
 * target_if_dbr_print_ring_debug_entries() - Print ring debug entries
 * @print: The print adapter function
 * @print_priv: The private data to be consumed by @print
 * @dbr_pdev_obj: Pdev object of the DBR module
 * @mod_id: Module ID
 *
 * Print ring debug entries of the ring identified by @dbr_pdev_obj and @mod_id
 * using the  given print adapter function
 *
 * Return: QDF_STATUS of operation
 */
static QDF_STATUS target_if_dbr_print_ring_debug_entries(
	qdf_abstract_print print, void *print_priv,
	struct direct_buf_rx_pdev_obj *dbr_pdev_obj,
	uint8_t mod_id, uint8_t srng_id)
{
	struct direct_buf_rx_module_debug *mod_debug;
	struct direct_buf_rx_ring_debug *ring_debug;
	int idx;

	mod_debug = target_if_get_dbr_mod_debug_from_dbr_pdev_obj(dbr_pdev_obj,
								  mod_id);
	if (!mod_debug)
		return QDF_STATUS_E_INVAL;

	mod_debug = &dbr_pdev_obj->dbr_mod_debug[mod_id];
	ring_debug = &mod_debug->dbr_ring_debug[srng_id];

	if (ring_debug->entries) {
		print(print_priv, "Current debug entry is %d",
		      ring_debug->ring_debug_idx);
		print(print_priv, "---------------------------------------------------------");
		print(print_priv, "| Number | Head Idx | Tail Idx | Timestamp |    event   |");
		print(print_priv, "---------------------------------------------------------");
		for (idx = 0; idx < ring_debug->num_ring_debug_entries; ++idx) {
			print(print_priv, "|%8u|%10u|%10u|%11llu|%12s|", idx,
			      ring_debug->entries[idx].head_idx,
			      ring_debug->entries[idx].tail_idx,
			      ring_debug->entries[idx].timestamp,
			      g_dbr_ring_debug_event[
				ring_debug->entries[idx].event]);
		}
		print(print_priv, "---------------------------------------------------------");
	}

	return QDF_STATUS_SUCCESS;
}

/**
 * target_if_dbr_qdf_err_printer() - QDF error level printer for DBR module
 * @print_priv: The private data
 * @fmt: Format string
 *
 * This function should be passed in place of the 'print' argument to
 * target_if_dbr_print_ring_debug_entries function for the logs that should be
 * printed via QDF trace
 *
 * Return: QDF_STATUS of operation
 */
static int target_if_dbr_qdf_err_printer(void *priv, const char *fmt, ...)
{
	va_list args;

	va_start(args, fmt);
	QDF_VTRACE(QDF_MODULE_ID_DIRECT_BUF_RX, QDF_TRACE_LEVEL_ERROR,
		   (char *)fmt, args);
	va_end(args);

	return 0;
}

static inline void target_if_direct_buf_rx_free_mod_debug(
	struct direct_buf_rx_pdev_obj *dbr_pdev_obj)
{
	if (!dbr_pdev_obj) {
		direct_buf_rx_err("dir buf rx object is null");
		return;
	}
	/* Free the debug data structures of all modules */
	if (dbr_pdev_obj->dbr_mod_debug) {
		qdf_mem_free(dbr_pdev_obj->dbr_mod_debug);
		dbr_pdev_obj->dbr_mod_debug = NULL;
	}
}

static inline QDF_STATUS target_if_direct_buf_rx_alloc_mod_debug(
	struct direct_buf_rx_pdev_obj *dbr_pdev_obj)
{
	if (!dbr_pdev_obj) {
		direct_buf_rx_err("dir buf rx object is null");
		return QDF_STATUS_E_FAILURE;
	}
	/* Allocate the debug data structure for each module */
	dbr_pdev_obj->dbr_mod_debug = qdf_mem_malloc(
				dbr_pdev_obj->num_modules *
				sizeof(struct direct_buf_rx_module_debug));

	if (!dbr_pdev_obj->dbr_mod_debug)
		return QDF_STATUS_E_NOMEM;

	return QDF_STATUS_SUCCESS;
}
#else
static inline QDF_STATUS target_if_direct_buf_rx_alloc_mod_debug(
	struct direct_buf_rx_pdev_obj *dbr_pdev_obj)
{
	return QDF_STATUS_SUCCESS;
}

static inline void target_if_direct_buf_rx_free_mod_debug(
	struct direct_buf_rx_pdev_obj *dbr_pdev_obj)
{
}
#endif

#if defined(WLAN_DEBUGFS) && defined(DIRECT_BUF_RX_DEBUG)
static inline void target_if_direct_buf_pdev_debugfs_init(
	struct wlan_objmgr_pdev *pdev)
{
	char dir_name[32];
	struct wlan_objmgr_psoc *psoc;
	struct direct_buf_rx_pdev_obj *dbr_pdev_obj;

	if (!pdev) {
		direct_buf_rx_err("pdev is null");
		return;
	}

	psoc = wlan_pdev_get_psoc(pdev);
	dbr_pdev_obj = wlan_objmgr_pdev_get_comp_private_obj(
		pdev, WLAN_TARGET_IF_COMP_DIRECT_BUF_RX);

	if (!dbr_pdev_obj) {
		direct_buf_rx_err("dir buf rx object is null");
		return;
	}

	qdf_snprintf(dir_name, sizeof(dir_name), "SOC%u_PDEV%u",
		     wlan_psoc_get_id(psoc),
		     wlan_objmgr_pdev_get_pdev_id(pdev));

	/* Create debugfs entry for this radio */
	dbr_pdev_obj->debugfs_entry = qdf_debugfs_create_dir(
					dir_name, dbr_debugfs_entry);

	if (!dbr_pdev_obj->debugfs_entry)
		direct_buf_rx_err("error while creating direct_buf debugfs dir");
}

static inline void target_if_direct_buf_pdev_debugfs_deinit(
	struct direct_buf_rx_pdev_obj *dbr_pdev_obj)
{
	if (!dbr_pdev_obj) {
		direct_buf_rx_err("dir buf rx object is null");
		return;
	}
	/* Remove the debugfs entry of the radio */
	if (dbr_pdev_obj->debugfs_entry) {
		qdf_debugfs_remove_dir_recursive(dbr_pdev_obj->debugfs_entry);
		dbr_pdev_obj->debugfs_entry = NULL;
	}
}
#else
static inline void target_if_direct_buf_pdev_debugfs_init(
	struct wlan_objmgr_pdev *pdev)
{
}

static inline void target_if_direct_buf_pdev_debugfs_deinit(
	struct direct_buf_rx_pdev_obj *dbr_pdev_obj)
{
}
#endif /* WLAN_DEBUGFS && DIRECT_BUF_RX_DEBUG */

QDF_STATUS target_if_direct_buf_rx_pdev_create_handler(
	struct wlan_objmgr_pdev *pdev, void *data)
{
	struct direct_buf_rx_pdev_obj *dbr_pdev_obj;
	struct direct_buf_rx_psoc_obj *dbr_psoc_obj;
	struct wlan_objmgr_psoc *psoc;
	uint8_t num_modules;
	QDF_STATUS status;

	direct_buf_rx_enter();

	if (!pdev) {
		direct_buf_rx_err("pdev context passed is null");
		return QDF_STATUS_E_INVAL;
	}

	psoc = wlan_pdev_get_psoc(pdev);

	if (!psoc) {
		direct_buf_rx_err("psoc is null");
		return QDF_STATUS_E_INVAL;
	}

	dbr_psoc_obj =
	wlan_objmgr_psoc_get_comp_private_obj(psoc,
					      WLAN_TARGET_IF_COMP_DIRECT_BUF_RX);

	if (!dbr_psoc_obj) {
		direct_buf_rx_err("dir buf rx psoc object is null");
		return QDF_STATUS_E_FAILURE;
	}

	dbr_pdev_obj = qdf_mem_malloc(sizeof(*dbr_pdev_obj));

	if (!dbr_pdev_obj)
		return QDF_STATUS_E_NOMEM;

	status = wlan_objmgr_pdev_component_obj_attach(pdev,
					WLAN_TARGET_IF_COMP_DIRECT_BUF_RX,
					dbr_pdev_obj, QDF_STATUS_SUCCESS);

	if (status != QDF_STATUS_SUCCESS) {
		direct_buf_rx_err("Failed to attach dir buf rx component %d",
				  status);
		qdf_mem_free(dbr_pdev_obj);
		return status;
	}

	dbr_psoc_obj->dbr_pdev_obj[wlan_objmgr_pdev_get_pdev_id(pdev)] =
								dbr_pdev_obj;

	num_modules = get_num_dbr_modules_per_pdev(pdev);
	direct_buf_rx_debug("Number of modules = %d pdev %d DBR pdev obj %pK",
			    num_modules, wlan_objmgr_pdev_get_pdev_id(pdev),
			    dbr_pdev_obj);
	dbr_pdev_obj->num_modules = num_modules;

	if (!dbr_pdev_obj->num_modules) {
		direct_buf_rx_info("Number of modules = %d", num_modules);
		return QDF_STATUS_SUCCESS;
	}

	direct_buf_rx_debug("sring number = %d", DBR_SRNG_NUM);
	dbr_pdev_obj->dbr_mod_param = qdf_mem_malloc(num_modules *
				DBR_SRNG_NUM *
				sizeof(struct direct_buf_rx_module_param));

	if (!dbr_pdev_obj->dbr_mod_param) {
		direct_buf_rx_err("alloc dbr mod param fail");
		goto dbr_mod_param_fail;
	}

	if (target_if_direct_buf_rx_alloc_mod_debug(dbr_pdev_obj) !=
		QDF_STATUS_SUCCESS)
		goto dbr_mod_debug_fail;

	target_if_direct_buf_pdev_debugfs_init(pdev);

	return QDF_STATUS_SUCCESS;

dbr_mod_debug_fail:
	qdf_mem_free(dbr_pdev_obj->dbr_mod_param);

dbr_mod_param_fail:
	wlan_objmgr_pdev_component_obj_detach(
				pdev, WLAN_TARGET_IF_COMP_DIRECT_BUF_RX,
				dbr_pdev_obj);
	qdf_mem_free(dbr_pdev_obj);

	return QDF_STATUS_E_NOMEM;
}

QDF_STATUS target_if_direct_buf_rx_pdev_destroy_handler(
	struct wlan_objmgr_pdev *pdev, void *data)
{
	struct direct_buf_rx_pdev_obj *dbr_pdev_obj;
	QDF_STATUS status;
	uint8_t num_modules, mod_idx, srng_id;

	if (!pdev) {
		direct_buf_rx_err("pdev context passed is null");
		return QDF_STATUS_E_INVAL;
	}

	dbr_pdev_obj = wlan_objmgr_pdev_get_comp_private_obj(pdev,
				WLAN_TARGET_IF_COMP_DIRECT_BUF_RX);

	if (!dbr_pdev_obj) {
		direct_buf_rx_err("dir buf rx object is null");
		return QDF_STATUS_E_FAILURE;
	}

	num_modules = dbr_pdev_obj->num_modules;
	for (mod_idx = 0; mod_idx < num_modules; mod_idx++) {
		/*
		 * If the module didn't stop the ring debug by this time,
		 * it will result in memory leak of its ring debug entries.
		 * So, stop the ring debug
		 */
		target_if_dbr_stop_ring_debug(pdev, mod_idx);
		for (srng_id = 0; srng_id < DBR_SRNG_NUM; srng_id++)
			target_if_deinit_dbr_ring(pdev, dbr_pdev_obj,
						  mod_idx, srng_id);
	}

	target_if_direct_buf_pdev_debugfs_deinit(dbr_pdev_obj);
	target_if_direct_buf_rx_free_mod_debug(dbr_pdev_obj);
	qdf_mem_free(dbr_pdev_obj->dbr_mod_param);
	dbr_pdev_obj->dbr_mod_param = NULL;

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

	if (!psoc) {
		direct_buf_rx_err("psoc context passed is null");
		return QDF_STATUS_E_INVAL;
	}

	dbr_psoc_obj = qdf_mem_malloc(sizeof(*dbr_psoc_obj));

	if (!dbr_psoc_obj)
		return QDF_STATUS_E_NOMEM;

	direct_buf_rx_debug("Dbr psoc obj %pK", dbr_psoc_obj);

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

#if defined(WLAN_DEBUGFS) && defined(DIRECT_BUF_RX_DEBUG)
/**
 * target_if_dbr_debugfs_show_ring_debug() - Function to display ring debug
 * entries in debugfs
 * @file: qdf debugfs file handler
 * @arg: pointer to DBR debugfs private object
 *
 * Return: QDF_STATUS of operation
 */
static QDF_STATUS target_if_dbr_debugfs_show_ring_debug(
	qdf_debugfs_file_t file, void *arg)
{
	struct dbr_debugfs_priv *priv = arg;

	return target_if_dbr_print_ring_debug_entries(qdf_debugfs_printer,
						      file, priv->dbr_pdev_obj,
						      priv->mod_id,
						      priv->srng_id);
}

/**
 * target_if_dbr_mod_debugfs_init() - Init debugfs for a given module
 * @dbr_pdev_obj: Pointer to the pdev obj of Direct buffer rx module
 * @mod_id: Module ID corresponding to this ring
 *
 * Return: QDF_STATUS of operation
 */
static QDF_STATUS target_if_dbr_mod_debugfs_init(
	struct direct_buf_rx_pdev_obj *dbr_pdev_obj,
	enum DBR_MODULE mod_id)
{
	struct direct_buf_rx_module_debug *mod_debug;

	mod_debug = target_if_get_dbr_mod_debug_from_dbr_pdev_obj(dbr_pdev_obj,
								  mod_id);

	if (!mod_debug)
		return QDF_STATUS_E_INVAL;

	if (mod_debug->debugfs_entry) {
		direct_buf_rx_err("debugfs mod entry was already created for %s module",
				  g_dbr_module_name[mod_id].module_name_str);
		return QDF_STATUS_SUCCESS;
	}

	mod_debug->debugfs_entry =
	    qdf_debugfs_create_dir(g_dbr_module_name[mod_id].module_name_str,
				   dbr_pdev_obj->debugfs_entry);

	if (!mod_debug->debugfs_entry) {
		direct_buf_rx_err("error while creating direct_buf debugfs entry for %s module",
				  g_dbr_module_name[mod_id].module_name_str);
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

/**
 * target_if_dbr_ring_debugfs_init() - Init debugfs for a given ring
 * @dbr_pdev_obj: Pointer to the pdev obj of Direct buffer rx module
 * @mod_id: Module ID corresponding to this ring
 * @srng_id: srng ID corresponding to this ring
 *
 * Return: QDF_STATUS of operation
 */
static QDF_STATUS target_if_dbr_ring_debugfs_init(
	struct direct_buf_rx_pdev_obj *dbr_pdev_obj,
	enum DBR_MODULE mod_id, uint8_t srng_id)
{
	struct direct_buf_rx_module_debug *mod_debug;
	struct direct_buf_rx_ring_debug *ring_debug;
	struct dbr_debugfs_priv *priv;
	char debug_file_name[32];

	mod_debug = target_if_get_dbr_mod_debug_from_dbr_pdev_obj(dbr_pdev_obj,
								  mod_id);

	if (!mod_debug)
		return QDF_STATUS_E_INVAL;

	ring_debug = &mod_debug->dbr_ring_debug[srng_id];

	if (!mod_debug->debugfs_entry) {
		direct_buf_rx_err("error mod_debug->debugfs_entry not created");
		return QDF_STATUS_E_FAILURE;
	}

	if (ring_debug->debugfs_entry) {
		direct_buf_rx_err("debugfs file for %d ring under %s module already created",
				   srng_id,
				   g_dbr_module_name[mod_id].module_name_str);
		return QDF_STATUS_SUCCESS;
	}

	qdf_snprintf(debug_file_name, sizeof(debug_file_name),
		     "ring_%d", srng_id);

	// Allocate debugfs ops
	ring_debug->debugfs_fops =
		qdf_mem_malloc(sizeof(*ring_debug->debugfs_fops));
	if (!ring_debug->debugfs_fops) {
		direct_buf_rx_err("error in allocating debugfs ops");
		return QDF_STATUS_E_NOMEM;
	}

	// Allocate private data
	priv = qdf_mem_malloc(sizeof(*priv));
	if (!priv) {
		direct_buf_rx_err("error in creating debugfs private data");
		goto priv_alloc_fail;
	}
	priv->dbr_pdev_obj = dbr_pdev_obj;
	priv->mod_id = mod_id;
	priv->srng_id = srng_id;

	/* Fill in the debugfs ops for this ring.
	 * When the output time comes, the 'show' function will be
	 * called with 'priv' as an argument.
	 */
	ring_debug->debugfs_fops->show = target_if_dbr_debugfs_show_ring_debug;
	ring_debug->debugfs_fops->priv = priv;

	ring_debug->debugfs_entry =
		qdf_debugfs_create_file_simplified(
				    debug_file_name,
				    (QDF_FILE_USR_READ | QDF_FILE_GRP_READ |
				    QDF_FILE_OTH_READ),
				    mod_debug->debugfs_entry,
				    ring_debug->debugfs_fops);

	if (!ring_debug->debugfs_entry) {
		direct_buf_rx_err("error while creating direct_buf debugfs file for %d ring under %s module",
				  srng_id,
				  g_dbr_module_name[mod_id].module_name_str);
		goto file_creation_fail;
	}

	return QDF_STATUS_SUCCESS;

file_creation_fail:
	qdf_mem_free(ring_debug->debugfs_fops->priv);

priv_alloc_fail:
	qdf_mem_free(ring_debug->debugfs_fops);
	ring_debug->debugfs_fops = NULL;
	return QDF_STATUS_E_NOMEM;
}

/**
 * target_if_dbr_mod_debugfs_deinit() - De-init debugfs for a given module
 * @mod_debug: Pointer to direct_buf_rx_module_debug structure
 *
 * Return: void
 */
static void target_if_dbr_mod_debugfs_deinit(
			struct direct_buf_rx_module_debug *mod_debug)
{
	if (!mod_debug) {
		direct_buf_rx_err("mod_debug is null");
		return;
	}

	if (mod_debug->debugfs_entry) {
		qdf_debugfs_remove_file(mod_debug->debugfs_entry);
		mod_debug->debugfs_entry = NULL;
	}
}

/**
 * target_if_dbr_ring_debugfs_deinit() - De-init debugfs for a given ring
 * @ring_debug: Pointer to direct_buf_rx_ring_debug structure
 *
 * Return: void
 */
static void target_if_dbr_ring_debugfs_deinit(
	struct direct_buf_rx_ring_debug *ring_debug)
{
	if (!ring_debug) {
		direct_buf_rx_err("ring_debug is null");
		return;
	}

	if (ring_debug->debugfs_entry) {
		qdf_debugfs_remove_file(ring_debug->debugfs_entry);
		ring_debug->debugfs_entry = NULL;
	}

	// Free the private data and debugfs ops of this ring
	if (ring_debug->debugfs_fops) {
		qdf_mem_free(ring_debug->debugfs_fops->priv);
		qdf_mem_free(ring_debug->debugfs_fops);
		ring_debug->debugfs_fops = NULL;
	}
}
#endif /* WLAN_DEBUGFS && DIRECT_BUF_RX_DEBUG */

#ifdef DIRECT_BUF_RX_DEBUG
QDF_STATUS target_if_dbr_stop_ring_debug(struct wlan_objmgr_pdev *pdev,
					 uint8_t mod_id)
{
	struct direct_buf_rx_module_debug *mod_debug;
	struct direct_buf_rx_ring_debug *ring_debug;
	uint8_t srng_id;

	mod_debug = target_if_get_dbr_mod_debug_from_pdev(pdev, mod_id);
	if (!mod_debug)
		return QDF_STATUS_E_INVAL;

	for (srng_id = 0; srng_id < DBR_SRNG_NUM; srng_id++) {
		ring_debug = &mod_debug->dbr_ring_debug[srng_id];
		if (!ring_debug->entries) {
			direct_buf_rx_debug("DBR ring debug for module %d srng %d was already disabled",
					   mod_id, srng_id);
			continue;
		}
		/* De-init debugsfs for this ring */
		target_if_dbr_ring_debugfs_deinit(ring_debug);
		qdf_mem_free(ring_debug->entries);
		ring_debug->entries = NULL;
		ring_debug->ring_debug_idx = 0;
		ring_debug->num_ring_debug_entries = 0;
		direct_buf_rx_info("DBR ring debug for module %d srng %d is now stopped",
				   mod_id, srng_id);
	}
	target_if_dbr_mod_debugfs_deinit(mod_debug);

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS target_if_dbr_start_ring_debug(struct wlan_objmgr_pdev *pdev,
					  uint8_t mod_id,
					  uint32_t num_ring_debug_entries)
{
	struct direct_buf_rx_pdev_obj *dbr_pdev_obj;
	struct direct_buf_rx_module_debug *mod_debug;
	struct direct_buf_rx_ring_debug *ring_debug;
	uint8_t srng_id;

	mod_debug = target_if_get_dbr_mod_debug_from_pdev(pdev, mod_id);

	if (!mod_debug)
		return QDF_STATUS_E_INVAL;

	if (num_ring_debug_entries > DIRECT_BUF_RX_MAX_RING_DEBUG_ENTRIES) {
		direct_buf_rx_err("Requested number of ring debug entries(%d) exceed the maximum entries allowed(%d)",
				  num_ring_debug_entries,
				  DIRECT_BUF_RX_MAX_RING_DEBUG_ENTRIES);

		return QDF_STATUS_E_FAILURE;
	}

	dbr_pdev_obj = wlan_objmgr_pdev_get_comp_private_obj(
				pdev, WLAN_TARGET_IF_COMP_DIRECT_BUF_RX);

	target_if_dbr_mod_debugfs_init(dbr_pdev_obj, mod_id);

	for (srng_id = 0; srng_id < DBR_SRNG_NUM; srng_id++) {
		ring_debug = &mod_debug->dbr_ring_debug[srng_id];

		if (ring_debug->entries) {
			direct_buf_rx_err("DBR ring debug for module %d srng %d was already enabled",
					  mod_id, srng_id);
			continue;
		}

		ring_debug->entries = qdf_mem_malloc(
					num_ring_debug_entries *
					sizeof(*ring_debug->entries));

		if (!ring_debug->entries)
			return QDF_STATUS_E_NOMEM;

		ring_debug->ring_debug_idx = 0;
		ring_debug->num_ring_debug_entries = num_ring_debug_entries;
		/* Init debugsfs for this ring */
		target_if_dbr_ring_debugfs_init(
			dbr_pdev_obj,
			mod_id, srng_id);
		direct_buf_rx_info("DBR ring debug for module %d srng %d is now started",
				    mod_id, srng_id);
	}
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS target_if_dbr_start_buffer_poisoning(struct wlan_objmgr_pdev *pdev,
						uint8_t mod_id, uint32_t value)
{
	struct direct_buf_rx_module_debug *mod_debug;

	mod_debug = target_if_get_dbr_mod_debug_from_pdev(pdev, mod_id);

	if (!mod_debug)
		return QDF_STATUS_E_INVAL;

	mod_debug->poisoning_enabled = true;
	mod_debug->poison_value = value; /* Save the poison value */

	direct_buf_rx_debug("DBR buffer poisoning for module %d is now started",
			    mod_id);
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS target_if_dbr_stop_buffer_poisoning(
	struct wlan_objmgr_pdev *pdev,
	uint8_t mod_id)
{
	struct direct_buf_rx_module_debug *mod_debug;

	mod_debug = target_if_get_dbr_mod_debug_from_pdev(pdev, mod_id);

	if (!mod_debug)
		return QDF_STATUS_E_INVAL;

	mod_debug->poisoning_enabled = false;
	mod_debug->poison_value = 0;

	direct_buf_rx_debug("DBR buffer poisoning for module %d is now stopped",
			    mod_id);
	return QDF_STATUS_SUCCESS;
}

/**
 * target_if_dbr_fill_buffer_u32() - Fill buffer with an unsigned 32-bit value
 * @buffer: pointer to the buffer
 * @num_bytes: Size of the destination buffer in bytes
 * @value: Unsigned 32-bit value to be copied
 *
 * Return : void
 */
static void
target_if_dbr_fill_buffer_u32(uint8_t *buffer, uint32_t num_bytes,
			      uint32_t value)
{
	uint32_t *bufp;
	uint32_t idx;
	uint32_t size = (num_bytes >> 2);

	if (!buffer) {
		direct_buf_rx_err("buffer empty");
		return;
	}

	bufp = (uint32_t *)buffer;

	for (idx = 0; idx < size; ++idx) {
		*bufp = value;
		++bufp;
	}
}

/**
 * target_if_dbr_debug_poison_buffer() - Poison a given DBR buffer
 * @pdev: pointer to pdev object
 * @mod_id: Module ID of the owner of the buffer
 * @aligned_vaddr: Virtual address(aligned) of the buffer
 * @size: Size of the buffer
 *
 * Value with which the buffers will be poisoned would have been saved
 * while starting the buffer poisoning for the module, use that value.
 *
 * Return : QDF status of operation
 */
static QDF_STATUS target_if_dbr_debug_poison_buffer(
	struct wlan_objmgr_pdev *pdev,
	uint32_t mod_id, void *aligned_vaddr, uint32_t size)
{
	struct direct_buf_rx_module_debug *mod_debug;

	mod_debug = target_if_get_dbr_mod_debug_from_pdev(pdev, mod_id);

	if (!mod_debug)
		return QDF_STATUS_E_INVAL;

	if (mod_debug->poisoning_enabled) {
		target_if_dbr_fill_buffer_u32(aligned_vaddr, size,
					      mod_debug->poison_value);
	}

	return QDF_STATUS_SUCCESS;
}

static inline void target_if_dbr_qdf_show_ring_debug(
	struct wlan_objmgr_pdev *pdev,
	uint8_t mod_id, uint8_t srng_id)
{
	struct direct_buf_rx_pdev_obj *dbr_pdev_obj =
			wlan_objmgr_pdev_get_comp_private_obj(
				pdev, WLAN_TARGET_IF_COMP_DIRECT_BUF_RX);

	target_if_dbr_print_ring_debug_entries(
			target_if_dbr_qdf_err_printer,
			NULL, dbr_pdev_obj,
			mod_id, srng_id);
}
#else
QDF_STATUS target_if_dbr_stop_ring_debug(struct wlan_objmgr_pdev *pdev,
					 uint8_t mod_id)
{
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS target_if_dbr_start_ring_debug(struct wlan_objmgr_pdev *pdev,
					  uint8_t mod_id,
					  uint32_t num_ring_debug_entries)
{
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS target_if_dbr_start_buffer_poisoning(struct wlan_objmgr_pdev *pdev,
						uint8_t mod_id, uint32_t value)
{
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS target_if_dbr_stop_buffer_poisoning(
		struct wlan_objmgr_pdev *pdev,
		uint8_t mod_id)
{
	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS target_if_dbr_debug_poison_buffer(
	struct wlan_objmgr_pdev *pdev,
	uint32_t mod_id, void *aligned_vaddr, uint32_t size)
{
	return QDF_STATUS_SUCCESS;
}

static inline void target_if_dbr_qdf_show_ring_debug(
	struct wlan_objmgr_pdev *pdev,
	uint8_t mod_id, uint8_t srng_id)
{
}
#endif /* DIRECT_BUF_RX_DEBUG */

static QDF_STATUS target_if_dbr_replenish_ring(struct wlan_objmgr_pdev *pdev,
			struct direct_buf_rx_module_param *mod_param,
			void *aligned_vaddr, uint32_t cookie)
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

	if (!dbr_psoc_obj) {
		direct_buf_rx_err("dir buf rx psoc object is null");
		return QDF_STATUS_E_FAILURE;
	}

	hal_soc = dbr_psoc_obj->hal_soc;
	srng = dbr_ring_cfg->srng;
	if (!aligned_vaddr) {
		direct_buf_rx_err("aligned vaddr is null");
		return QDF_STATUS_SUCCESS;
	}

	target_if_dbr_debug_poison_buffer(
			pdev, mod_param->mod_id, aligned_vaddr,
			dbr_ring_cap->min_buf_size);

	map_status = qdf_mem_map_nbytes_single(dbr_psoc_obj->osdev,
					       aligned_vaddr,
					       QDF_DMA_FROM_DEVICE,
					       dbr_ring_cap->min_buf_size,
					       &paddr);
	if (map_status) {
		direct_buf_rx_err("mem map failed status = %d", map_status);
		return QDF_STATUS_E_FAILURE;
	}

	QDF_ASSERT(!((uint64_t)paddr % dbr_ring_cap->min_buf_align));
	dbr_buf_pool[cookie].paddr = paddr;

	hal_srng_access_start(hal_soc, srng);
	ring_entry = hal_srng_src_get_next(hal_soc, srng);

	if (!ring_entry) {
		target_if_dbr_qdf_show_ring_debug(pdev, mod_param->mod_id,
						  mod_param->srng_id);
		QDF_BUG(0);
	}

	dw_lo = (uint64_t)paddr & 0xFFFFFFFF;
	WMI_HOST_DBR_RING_ADDR_HI_SET(dw_hi, (uint64_t)paddr >> 32);
	WMI_HOST_DBR_DATA_ADDR_HI_HOST_DATA_SET(dw_hi, cookie);
	*ring_entry = (uint64_t)dw_hi << 32 | dw_lo;
	hal_srng_access_end(hal_soc, srng);

	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS target_if_dbr_fill_ring(struct wlan_objmgr_pdev *pdev,
			  struct direct_buf_rx_module_param *mod_param)
{
	uint32_t idx;
	struct direct_buf_rx_ring_cfg *dbr_ring_cfg;
	struct direct_buf_rx_ring_cap *dbr_ring_cap;
	struct direct_buf_rx_buf_info *dbr_buf_pool;
	QDF_STATUS status;

	direct_buf_rx_enter();

	dbr_ring_cfg = mod_param->dbr_ring_cfg;
	dbr_ring_cap = mod_param->dbr_ring_cap;
	dbr_buf_pool = mod_param->dbr_buf_pool;

	for (idx = 0; idx < dbr_ring_cfg->num_ptr - 1; idx++) {
		void *buf_vaddr_unaligned = NULL, *buf_vaddr_aligned;
		dma_addr_t buf_paddr_aligned, buf_paddr_unaligned;

		buf_vaddr_aligned = qdf_aligned_malloc(
			&dbr_ring_cap->min_buf_size, &buf_vaddr_unaligned,
			&buf_paddr_unaligned, &buf_paddr_aligned,
			dbr_ring_cap->min_buf_align);

		if (!buf_vaddr_aligned) {
			direct_buf_rx_err("dir buf rx ring alloc failed");
			return QDF_STATUS_E_NOMEM;
		}
		dbr_buf_pool[idx].vaddr = buf_vaddr_unaligned;
		dbr_buf_pool[idx].offset = buf_vaddr_aligned -
		    buf_vaddr_unaligned;
		dbr_buf_pool[idx].cookie = idx;
		status = target_if_dbr_replenish_ring(pdev, mod_param,
						      buf_vaddr_aligned, idx);
		if (QDF_IS_STATUS_ERROR(status)) {
			direct_buf_rx_err("replenish failed with status : %d",
					  status);
			qdf_mem_free(buf_vaddr_unaligned);
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

	if (!dbr_psoc_obj) {
		direct_buf_rx_err("dir buf rx psoc object is null");
		return QDF_STATUS_E_FAILURE;
	}

	if (!dbr_psoc_obj->hal_soc ||
	    !dbr_psoc_obj->osdev) {
		direct_buf_rx_err("dir buf rx target attach failed");
		return QDF_STATUS_E_FAILURE;
	}

	max_entries = hal_srng_max_entries(dbr_psoc_obj->hal_soc,
					   DIR_BUF_RX_DMA_SRC);
	entry_size = hal_srng_get_entrysize(dbr_psoc_obj->hal_soc,
					    DIR_BUF_RX_DMA_SRC);
	direct_buf_rx_debug("Max Entries = %d", max_entries);
	direct_buf_rx_debug("Entry Size = %d", entry_size);

	status = populate_dbr_cap_mod_param(pdev, mod_param);
	if (QDF_IS_STATUS_ERROR(status)) {
		direct_buf_rx_err("Module cap population failed");
		return QDF_STATUS_E_FAILURE;
	}

	dbr_ring_cap = mod_param->dbr_ring_cap;
	dbr_ring_cfg = mod_param->dbr_ring_cfg;
	num_entries = dbr_ring_cap->ring_elems_min > max_entries ?
			max_entries : dbr_ring_cap->ring_elems_min;
	direct_buf_rx_debug("Num entries = %d", num_entries);
	dbr_ring_cfg->num_ptr = num_entries;
	mod_param->dbr_buf_pool = qdf_mem_malloc(num_entries * sizeof(
				struct direct_buf_rx_buf_info));
	if (!mod_param->dbr_buf_pool)
		return QDF_STATUS_E_NOMEM;

	ring_alloc_size = (num_entries * entry_size) + DBR_RING_BASE_ALIGN - 1;
	dbr_ring_cfg->ring_alloc_size = ring_alloc_size;
	direct_buf_rx_debug("dbr_psoc_obj %pK", dbr_psoc_obj);
	dbr_ring_cfg->base_vaddr_unaligned = qdf_mem_alloc_consistent(
		dbr_psoc_obj->osdev, dbr_psoc_obj->osdev->dev, ring_alloc_size,
		&paddr);
	direct_buf_rx_debug("vaddr aligned allocated");
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
	srng = hal_srng_setup(dbr_psoc_obj->hal_soc, DIR_BUF_RX_DMA_SRC,
			      mod_param->mod_id,
			      mod_param->pdev_id, &ring_params);

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

	direct_buf_rx_debug("Init DBR srng");

	if (!mod_param) {
		direct_buf_rx_err("dir buf rx module param is null");
		return QDF_STATUS_E_INVAL;
	}

	mod_param->dbr_ring_cap = qdf_mem_malloc(sizeof(
					struct direct_buf_rx_ring_cap));

	if (!mod_param->dbr_ring_cap)
		return QDF_STATUS_E_NOMEM;

	/* Allocate memory for DBR Ring Config */
	mod_param->dbr_ring_cfg = qdf_mem_malloc(sizeof(
					struct direct_buf_rx_ring_cfg));

	if (!mod_param->dbr_ring_cfg) {
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
	wmi_unified_t wmi_hdl;
	struct direct_buf_rx_cfg_req dbr_cfg_req = {0};
	struct direct_buf_rx_ring_cfg *dbr_ring_cfg;
	struct direct_buf_rx_ring_cap *dbr_ring_cap;
	struct dbr_module_config *dbr_config;

	direct_buf_rx_enter();

	psoc = wlan_pdev_get_psoc(pdev);
	if (!psoc) {
		direct_buf_rx_err("psoc is null");
		return QDF_STATUS_E_FAILURE;
	}

	dbr_ring_cfg = mod_param->dbr_ring_cfg;
	dbr_ring_cap = mod_param->dbr_ring_cap;
	dbr_config = &mod_param->dbr_config;
	wmi_hdl = lmac_get_pdev_wmi_handle(pdev);
	if (!wmi_hdl) {
		direct_buf_rx_err("WMI handle null. Can't send WMI CMD");
		return QDF_STATUS_E_INVAL;
	}

	direct_buf_rx_debug("Sending DBR Ring CFG to target");
	dbr_cfg_req.pdev_id = mod_param->pdev_id;
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
	dbr_cfg_req.num_resp_per_event = dbr_config->num_resp_per_event;
	dbr_cfg_req.event_timeout_ms = dbr_config->event_timeout_in_ms;
	direct_buf_rx_debug("pdev id %d mod id %d base addr lo %x\n"
			    "base addr hi %x head idx addr lo %x\n"
			    "head idx addr hi %x tail idx addr lo %x\n"
			    "tail idx addr hi %x num ptr %d\n"
			    "num resp %d event timeout %d\n",
			    dbr_cfg_req.pdev_id, dbr_cfg_req.mod_id,
			    dbr_cfg_req.base_paddr_lo,
			    dbr_cfg_req.base_paddr_hi,
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
				enum DBR_MODULE mod_id, uint8_t srng_id)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	struct direct_buf_rx_module_param *mod_param;

	direct_buf_rx_debug("Init DBR ring for module %d, srng %d",
			    mod_id, srng_id);

	if (!dbr_pdev_obj) {
		direct_buf_rx_err("dir buf rx object is null");
		return QDF_STATUS_E_INVAL;
	}

	mod_param = &(dbr_pdev_obj->dbr_mod_param[mod_id][srng_id]);

	if (!mod_param) {
		direct_buf_rx_err("dir buf rx module param is null");
		return QDF_STATUS_E_FAILURE;
	}

	direct_buf_rx_debug("mod_param %pK", mod_param);

	mod_param->mod_id = mod_id;
	mod_param->pdev_id = dbr_get_pdev_id(
				srng_id, wlan_objmgr_pdev_get_pdev_id(pdev));
	mod_param->srng_id = srng_id;

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
	target_if_deinit_dbr_ring(pdev, dbr_pdev_obj, mod_id, srng_id);
	return status;
}

QDF_STATUS target_if_direct_buf_rx_module_register(
			struct wlan_objmgr_pdev *pdev, uint8_t mod_id,
			struct dbr_module_config *dbr_config,
			bool (*dbr_rsp_handler)
			     (struct wlan_objmgr_pdev *pdev,
			      struct direct_buf_rx_data *dbr_data))
{
	QDF_STATUS status;
	struct direct_buf_rx_pdev_obj *dbr_pdev_obj;
	struct dbr_module_config *config = NULL;
	struct direct_buf_rx_module_param *mod_param;
	uint8_t srng_id;

	if (!pdev) {
		direct_buf_rx_err("pdev context passed is null");
		return QDF_STATUS_E_INVAL;
	}

	if (!dbr_rsp_handler) {
		direct_buf_rx_err("Response handler is null");
		return QDF_STATUS_E_INVAL;
	}

	if (mod_id >= DBR_MODULE_MAX) {
		direct_buf_rx_err("Invalid module id");
		return QDF_STATUS_E_INVAL;
	}

	dbr_pdev_obj = wlan_objmgr_pdev_get_comp_private_obj(pdev,
				WLAN_TARGET_IF_COMP_DIRECT_BUF_RX);

	if (!dbr_pdev_obj) {
		direct_buf_rx_err("dir buf rx object is null");
		return QDF_STATUS_E_FAILURE;
	}

	direct_buf_rx_debug("Dbr pdev obj %pK", dbr_pdev_obj);

	if (!dbr_pdev_obj->dbr_mod_param) {
		direct_buf_rx_err("dbr_pdev_obj->dbr_mod_param is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	if (mod_id >= dbr_pdev_obj->num_modules) {
		direct_buf_rx_err("Module %d not supported in target", mod_id);
		return QDF_STATUS_E_FAILURE;
	}

	for (srng_id = 0; srng_id < DBR_SRNG_NUM; srng_id++) {
		mod_param = &dbr_pdev_obj->dbr_mod_param[mod_id][srng_id];
		config = &mod_param->dbr_config;
		mod_param->dbr_rsp_handler = dbr_rsp_handler;
		*config = *dbr_config;

		status = target_if_init_dbr_ring(pdev, dbr_pdev_obj,
						 (enum DBR_MODULE)mod_id,
						 srng_id);
		if (QDF_IS_STATUS_ERROR(status))
			direct_buf_rx_err("init dbr ring fail, srng_id %d, status %d",
					  srng_id, status);
	}

	return status;
}

QDF_STATUS target_if_direct_buf_rx_module_unregister(
			struct wlan_objmgr_pdev *pdev, uint8_t mod_id)
{
	QDF_STATUS status;
	struct direct_buf_rx_pdev_obj *dbr_pdev_obj;
	uint8_t srng_id;

	if (!pdev) {
		direct_buf_rx_err("pdev context passed is null");
		return QDF_STATUS_E_INVAL;
	}

	if (mod_id >= DBR_MODULE_MAX) {
		direct_buf_rx_err("Invalid module id");
		return QDF_STATUS_E_INVAL;
	}

	dbr_pdev_obj = wlan_objmgr_pdev_get_comp_private_obj
			(pdev,
			 WLAN_TARGET_IF_COMP_DIRECT_BUF_RX);

	if (!dbr_pdev_obj) {
		direct_buf_rx_err("dir buf rx object is null");
		return QDF_STATUS_E_FAILURE;
	}

	direct_buf_rx_debug("Dbr pdev obj %pK", dbr_pdev_obj);

	if (!dbr_pdev_obj->dbr_mod_param) {
		direct_buf_rx_err("dbr_pdev_obj->dbr_mod_param is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	if (mod_id >= dbr_pdev_obj->num_modules) {
		direct_buf_rx_err("Module %d not supported in target", mod_id);
		return QDF_STATUS_E_FAILURE;
	}

	for (srng_id = 0; srng_id < DBR_SRNG_NUM; srng_id++) {
		status = target_if_deinit_dbr_ring(pdev, dbr_pdev_obj,
						   mod_id, srng_id);
		direct_buf_rx_info("status %d", status);
	}

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

	direct_buf_rx_debug("Incorrect paddr found on cookie slot");
	return NULL;
}

QDF_STATUS target_if_dbr_cookie_lookup(struct wlan_objmgr_pdev *pdev,
				       uint8_t mod_id, qdf_dma_addr_t paddr,
				       uint32_t *cookie, uint8_t srng_id)
{
	struct direct_buf_rx_buf_info *dbr_buf_pool;
	struct direct_buf_rx_ring_cfg *dbr_ring_cfg;
	struct direct_buf_rx_pdev_obj *dbr_pdev_obj;
	struct direct_buf_rx_module_param *mod_param;
	enum wlan_umac_comp_id dbr_comp_id = WLAN_TARGET_IF_COMP_DIRECT_BUF_RX;
	uint32_t idx;

	dbr_pdev_obj = wlan_objmgr_pdev_get_comp_private_obj(pdev, dbr_comp_id);
	if (!dbr_pdev_obj) {
		direct_buf_rx_err("dir buf rx object is null");
		return QDF_STATUS_E_FAILURE;
	}

	mod_param = &dbr_pdev_obj->dbr_mod_param[mod_id][srng_id];
	if (!mod_param) {
		direct_buf_rx_err("dir buf rx module param is null");
		return QDF_STATUS_E_FAILURE;
	}

	dbr_ring_cfg = mod_param->dbr_ring_cfg;
	dbr_buf_pool = mod_param->dbr_buf_pool;

	for (idx = 0; idx < dbr_ring_cfg->num_ptr - 1; idx++) {
		if (dbr_buf_pool[idx].paddr &&
		    dbr_buf_pool[idx].paddr == paddr) {
			*cookie = idx;
			return QDF_STATUS_SUCCESS;
		}
	}

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS target_if_dbr_buf_release(struct wlan_objmgr_pdev *pdev,
				     uint8_t mod_id, qdf_dma_addr_t paddr,
				     uint32_t cookie, uint8_t srng_id)
{
	struct direct_buf_rx_module_param *mod_param;
	struct direct_buf_rx_pdev_obj *dbr_pdev_obj;
	enum wlan_umac_comp_id dbr_comp_id = WLAN_TARGET_IF_COMP_DIRECT_BUF_RX;
	void *vaddr;
	QDF_STATUS status;

	dbr_pdev_obj = wlan_objmgr_pdev_get_comp_private_obj(pdev, dbr_comp_id);
	if (!dbr_pdev_obj) {
		direct_buf_rx_err("dir buf rx object is null");
		return QDF_STATUS_E_FAILURE;
	}

	mod_param = &dbr_pdev_obj->dbr_mod_param[mod_id][srng_id];
	if (!mod_param) {
		direct_buf_rx_err("dir buf rx module param is null");
		return QDF_STATUS_E_FAILURE;
	}

	vaddr = target_if_dbr_vaddr_lookup(mod_param, paddr, cookie);
	if (!vaddr)
		return QDF_STATUS_E_FAILURE;

	status = target_if_dbr_replenish_ring(pdev, mod_param,
					      vaddr, cookie);
	if (QDF_IS_STATUS_ERROR(status)) {
		direct_buf_rx_err("Ring replenish failed");
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
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

	if (!dbr_psoc_obj) {
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
	dbr_data->vaddr = target_if_dbr_vaddr_lookup(mod_param, paddr, *cookie);

	if (!dbr_data->vaddr) {
		direct_buf_rx_err("dbr vaddr lookup failed, vaddr NULL");
		return QDF_STATUS_E_FAILURE;
	}

	dbr_data->cookie = *cookie;
	dbr_data->paddr = paddr;
	direct_buf_rx_debug("Cookie = %d Vaddr look up = %pK",
			    dbr_data->cookie, dbr_data->vaddr);
	dbr_data->dbr_len = dbr_rsp->dbr_entries[idx].len;
	qdf_mem_unmap_nbytes_single(dbr_psoc_obj->osdev, (qdf_dma_addr_t)paddr,
				    QDF_DMA_FROM_DEVICE,
				    dbr_ring_cap->min_buf_size);

	return QDF_STATUS_SUCCESS;
}

#ifdef DBR_MULTI_SRNG_ENABLE
/**
 * dbr_get_pdev_and_srng_id() - get pdev object and srng id
 *
 * @psoc: pointer to psoc object
 * @pdev_id: pdev id from wmi_pdev_dma_ring_buf_release eventid
 * @srng_id:  pointer to return srng id
 *
 * Return : pointer to pdev
 */
static struct wlan_objmgr_pdev *
dbr_get_pdev_and_srng_id(struct wlan_objmgr_psoc *psoc, uint8_t pdev_id,
			 uint8_t *srng_id)
{
	struct wlan_objmgr_pdev *pdev;
	wlan_objmgr_ref_dbgid dbr_mod_id = WLAN_DIRECT_BUF_RX_ID;

	pdev = wlan_objmgr_get_pdev_by_id(psoc, pdev_id, dbr_mod_id);
	if (!pdev) {
		pdev = wlan_objmgr_get_pdev_by_id(psoc, TGT_WMI_PDEV_ID_SOC,
						  dbr_mod_id);
		if (pdev) {
			direct_buf_rx_debug("update srng id from %d to %d",
					    *srng_id, pdev_id);
			*srng_id = pdev_id;
		}
	}

	return pdev;
}
#else
static struct wlan_objmgr_pdev *
dbr_get_pdev_and_srng_id(struct wlan_objmgr_psoc *psoc, uint8_t pdev_id,
			 uint8_t *srng_id)
{
	struct wlan_objmgr_pdev *pdev;
	wlan_objmgr_ref_dbgid dbr_mod_id = WLAN_DIRECT_BUF_RX_ID;

	pdev = wlan_objmgr_get_pdev_by_id(psoc, pdev_id, dbr_mod_id);

	return pdev;
}
#endif

#ifdef DIRECT_BUF_RX_DEBUG
/**
 * target_if_dbr_add_ring_debug_entry() - Add a DBR ring debug entry
 * @pdev: pointer to pdev object
 * @mod_id: Module ID
 * @event: ring debug event
 *
 * Log the given event, head and tail pointers of DBR ring of the given module
 * into its ring debug data structure.
 * Also, log the timestamp at the time of logging.
 */
static void target_if_dbr_add_ring_debug_entry(
	struct wlan_objmgr_pdev *pdev,
	uint32_t mod_id,
	enum DBR_RING_DEBUG_EVENT event,
	uint8_t srng_id)
{
	struct wlan_objmgr_psoc *psoc;
	void *hal_soc, *srng;
	uint32_t hp = 0, tp = 0;
	struct direct_buf_rx_psoc_obj *dbr_psoc_obj;
	struct direct_buf_rx_pdev_obj *dbr_pdev_obj;
	struct direct_buf_rx_ring_cfg *dbr_ring_cfg;
	struct direct_buf_rx_module_debug *mod_debug;
	struct direct_buf_rx_module_param *mod_param;
	struct direct_buf_rx_ring_debug *ring_debug;
	struct direct_buf_rx_ring_debug_entry *entry;

	mod_debug = target_if_get_dbr_mod_debug_from_pdev(pdev, mod_id);

	if (!mod_debug)
		return;

	psoc = wlan_pdev_get_psoc(pdev);

	dbr_pdev_obj = wlan_objmgr_pdev_get_comp_private_obj(
				pdev, WLAN_TARGET_IF_COMP_DIRECT_BUF_RX);

	dbr_psoc_obj = wlan_objmgr_psoc_get_comp_private_obj(
				psoc, WLAN_TARGET_IF_COMP_DIRECT_BUF_RX);

	mod_param = &dbr_pdev_obj->dbr_mod_param[mod_id][srng_id];
	if (!mod_param) {
		direct_buf_rx_err("dir buf rx module param is null");
		return;
	}

	hal_soc = dbr_psoc_obj->hal_soc;
	dbr_ring_cfg = mod_param->dbr_ring_cfg;
	srng = dbr_ring_cfg->srng;
	ring_debug = &mod_debug->dbr_ring_debug[srng_id];

	if (ring_debug->entries) {
		if (hal_srng_access_start(hal_soc, srng)) {
			direct_buf_rx_err("module %d - HAL srng access failed",
					  mod_id);
			return;
		}
		hal_get_sw_hptp(hal_soc, srng, &tp, &hp);
		hal_srng_access_end(hal_soc, srng);
		entry = &ring_debug->entries[ring_debug->ring_debug_idx];

		entry->head_idx = hp;
		entry->tail_idx = tp;
		entry->timestamp = qdf_get_log_timestamp();
		entry->event = event;

		ring_debug->ring_debug_idx++;
		if (ring_debug->ring_debug_idx ==
			ring_debug->num_ring_debug_entries)
			ring_debug->ring_debug_idx = 0;
	}
}

#else
static void target_if_dbr_add_ring_debug_entry(
	struct wlan_objmgr_pdev *pdev,
	uint32_t mod_id,
	enum DBR_RING_DEBUG_EVENT event,
	uint8_t srng_id)
{
}
#endif /* DIRECT_BUF_RX_DEBUG */

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
	struct wmi_unified *wmi_handle;
	wlan_objmgr_ref_dbgid dbr_mod_id = WLAN_DIRECT_BUF_RX_ID;
	uint8_t srng_id = 0;

	direct_buf_rx_enter();

	psoc = target_if_get_psoc_from_scn_hdl(scn);
	if (!psoc) {
		direct_buf_rx_err("psoc is null");
		return QDF_STATUS_E_FAILURE;
	}

	wmi_handle = GET_WMI_HDL_FROM_PSOC(psoc);
	if (!wmi_handle) {
		direct_buf_rx_err("WMI handle is null");
		return QDF_STATUS_E_FAILURE;
	}

	if (wmi_extract_dbr_buf_release_fixed(
		wmi_handle, data_buf, &dbr_rsp) != QDF_STATUS_SUCCESS) {
		direct_buf_rx_err("unable to extract DBR rsp fixed param");
		return QDF_STATUS_E_FAILURE;
	}

	direct_buf_rx_debug("Num buf release entry = %d",
			    dbr_rsp.num_buf_release_entry);

	pdev = dbr_get_pdev_and_srng_id(psoc, (uint8_t)dbr_rsp.pdev_id,
					&srng_id);
	if (!pdev || (srng_id >= DBR_SRNG_NUM)) {
		direct_buf_rx_err("invalid pdev or srng, pdev %pK, srng %d",
				  pdev, srng_id);
		return QDF_STATUS_E_INVAL;
	}

	dbr_pdev_obj = wlan_objmgr_pdev_get_comp_private_obj(pdev,
				WLAN_TARGET_IF_COMP_DIRECT_BUF_RX);

	if (!dbr_pdev_obj) {
		direct_buf_rx_err("dir buf rx object is null");
		wlan_objmgr_pdev_release_ref(pdev, dbr_mod_id);
		return QDF_STATUS_E_FAILURE;
	}

	if (dbr_rsp.mod_id >= dbr_pdev_obj->num_modules) {
		direct_buf_rx_err("Invalid module id:%d", dbr_rsp.mod_id);
		wlan_objmgr_pdev_release_ref(pdev, dbr_mod_id);
		return QDF_STATUS_E_FAILURE;
	}
	mod_param = &(dbr_pdev_obj->dbr_mod_param[dbr_rsp.mod_id][srng_id]);

	if (!mod_param) {
		direct_buf_rx_err("dir buf rx module param is null");
		wlan_objmgr_pdev_release_ref(pdev, dbr_mod_id);
		return QDF_STATUS_E_FAILURE;
	}

	dbr_buf_pool = mod_param->dbr_buf_pool;
	dbr_rsp.dbr_entries = qdf_mem_malloc(dbr_rsp.num_buf_release_entry *
					sizeof(struct direct_buf_rx_entry));

	if (dbr_rsp.num_meta_data_entry > dbr_rsp.num_buf_release_entry) {
		direct_buf_rx_err("More than expected number of metadata");
		wlan_objmgr_pdev_release_ref(pdev, dbr_mod_id);
		return QDF_STATUS_E_FAILURE;
	}

	for (i = 0; i < dbr_rsp.num_buf_release_entry; i++) {
		if (wmi_extract_dbr_buf_release_entry(
			wmi_handle, data_buf, i,
			&dbr_rsp.dbr_entries[i]) != QDF_STATUS_SUCCESS) {
			direct_buf_rx_err("Unable to extract DBR buf entry %d",
					  i+1);
			qdf_mem_free(dbr_rsp.dbr_entries);
			wlan_objmgr_pdev_release_ref(pdev, dbr_mod_id);
			return QDF_STATUS_E_FAILURE;
		}
		status = target_if_get_dbr_data(pdev, mod_param, &dbr_rsp,
						&dbr_data, i, &cookie);

		if (QDF_IS_STATUS_ERROR(status)) {
			direct_buf_rx_err("DBR data get failed");
			qdf_mem_free(dbr_rsp.dbr_entries);
			wlan_objmgr_pdev_release_ref(pdev, dbr_mod_id);
			return QDF_STATUS_E_FAILURE;
		}

		dbr_data.meta_data_valid = false;
		if (i < dbr_rsp.num_meta_data_entry) {
			if (wmi_extract_dbr_buf_metadata(
				wmi_handle, data_buf, i,
				&dbr_data.meta_data) == QDF_STATUS_SUCCESS)
				dbr_data.meta_data_valid = true;
		}

		target_if_dbr_add_ring_debug_entry(pdev, dbr_rsp.mod_id,
						   DBR_RING_DEBUG_EVENT_RX,
						   srng_id);
		if (mod_param->dbr_rsp_handler(pdev, &dbr_data)) {
			status = target_if_dbr_replenish_ring(pdev, mod_param,
							      dbr_data.vaddr,
							      cookie);

			target_if_dbr_add_ring_debug_entry(
				pdev, dbr_rsp.mod_id,
				DBR_RING_DEBUG_EVENT_REPLENISH_RING,
				srng_id);

			if (QDF_IS_STATUS_ERROR(status)) {
				direct_buf_rx_err("Ring replenish failed");
				qdf_mem_free(dbr_rsp.dbr_entries);
				wlan_objmgr_pdev_release_ref(pdev, dbr_mod_id);
				return QDF_STATUS_E_FAILURE;
			}
		}
	}

	qdf_mem_free(dbr_rsp.dbr_entries);
	wlan_objmgr_pdev_release_ref(pdev, dbr_mod_id);

	return ret;
}

static QDF_STATUS target_if_dbr_empty_ring(struct wlan_objmgr_pdev *pdev,
			struct direct_buf_rx_psoc_obj *dbr_psoc_obj,
			struct direct_buf_rx_module_param *mod_param)
{
	uint32_t idx;
	struct direct_buf_rx_ring_cfg *dbr_ring_cfg;
	struct direct_buf_rx_ring_cap *dbr_ring_cap;
	struct direct_buf_rx_buf_info *dbr_buf_pool;

	direct_buf_rx_enter();
	dbr_ring_cfg = mod_param->dbr_ring_cfg;
	dbr_ring_cap = mod_param->dbr_ring_cap;
	dbr_buf_pool = mod_param->dbr_buf_pool;

	direct_buf_rx_debug("dbr_ring_cfg %pK, ring_cap %pK buf_pool %pK",
			   dbr_ring_cfg, dbr_ring_cap, dbr_buf_pool);

	for (idx = 0; idx < dbr_ring_cfg->num_ptr - 1; idx++) {
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

	if (!dbr_psoc_obj) {
		direct_buf_rx_err("dir buf rx psoc object is null");
		return QDF_STATUS_E_FAILURE;
	}
	direct_buf_rx_debug("dbr_psoc_obj %pK", dbr_psoc_obj);

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
	direct_buf_rx_debug("dbr buf pool %pK", dbr_buf_pool);
	target_if_dbr_deinit_ring(pdev, mod_param);
	if (mod_param->dbr_buf_pool)
		qdf_mem_free(dbr_buf_pool);
	mod_param->dbr_buf_pool = NULL;

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS target_if_deinit_dbr_ring(struct wlan_objmgr_pdev *pdev,
			struct direct_buf_rx_pdev_obj *dbr_pdev_obj,
			enum DBR_MODULE mod_id, uint8_t srng_id)
{
	struct direct_buf_rx_module_param *mod_param;

	direct_buf_rx_enter();
	mod_param = &(dbr_pdev_obj->dbr_mod_param[mod_id][srng_id]);

	if (!mod_param) {
		direct_buf_rx_err("dir buf rx module param is null");
		return QDF_STATUS_E_FAILURE;
	}
	direct_buf_rx_debug("mod_param %pK, dbr_ring_cap %pK",
			    mod_param, mod_param->dbr_ring_cap);
	target_if_dbr_deinit_srng(pdev, mod_param);
	if (mod_param->dbr_ring_cap)
		qdf_mem_free(mod_param->dbr_ring_cap);
	mod_param->dbr_ring_cap = NULL;
	if (mod_param->dbr_ring_cfg)
		qdf_mem_free(mod_param->dbr_ring_cfg);
	mod_param->dbr_ring_cfg = NULL;

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

	ret = wmi_unified_register_event_handler(
			get_wmi_unified_hdl_from_psoc(psoc),
			wmi_dma_buf_release_event_id,
			target_if_direct_buf_rx_rsp_event_handler,
			WMI_RX_UMAC_CTX);

	if (ret)
		direct_buf_rx_debug("event handler not supported, ret=%d", ret);

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS target_if_direct_buf_rx_unregister_events(
				struct wlan_objmgr_psoc *psoc)
{
	if (!psoc || !GET_WMI_HDL_FROM_PSOC(psoc)) {
		direct_buf_rx_err("psoc or psoc->tgt_if_handle is null");
		return QDF_STATUS_E_INVAL;
	}

	wmi_unified_unregister_event_handler(
			get_wmi_unified_hdl_from_psoc(psoc),
			wmi_dma_buf_release_event_id);

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS target_if_direct_buf_rx_print_ring_stat(
				struct wlan_objmgr_pdev *pdev)
{
	struct direct_buf_rx_psoc_obj *dbr_psoc_obj;
	struct direct_buf_rx_pdev_obj *dbr_pdev_obj;
	struct wlan_objmgr_psoc *psoc;
	void *srng, *hal_soc;
	uint32_t hp = 0, tp = 0;
	struct direct_buf_rx_module_param *mod_param;
	struct direct_buf_rx_ring_cfg *dbr_ring_cfg;
	uint8_t num_modules, mod_idx;
	uint8_t srng_id;

	if (!pdev) {
		direct_buf_rx_err("pdev is null");
		return QDF_STATUS_E_INVAL;
	}

	psoc = wlan_pdev_get_psoc(pdev);
	dbr_pdev_obj = wlan_objmgr_pdev_get_comp_private_obj(pdev,
				WLAN_TARGET_IF_COMP_DIRECT_BUF_RX);
	dbr_psoc_obj = wlan_objmgr_psoc_get_comp_private_obj(psoc,
				WLAN_TARGET_IF_COMP_DIRECT_BUF_RX);
	hal_soc = dbr_psoc_obj->hal_soc;
	num_modules = dbr_pdev_obj->num_modules;
	direct_buf_rx_debug("--------------------------------------------------");
	direct_buf_rx_debug("| Module ID |    Module    | Head Idx | Tail Idx |");
	direct_buf_rx_debug("--------------------------------------------------");
	for (mod_idx = 0; mod_idx < num_modules; mod_idx++) {
		for (srng_id = 0; srng_id < DBR_SRNG_NUM; srng_id++) {
			mod_param =
				&dbr_pdev_obj->dbr_mod_param[mod_idx][srng_id];
			dbr_ring_cfg = mod_param->dbr_ring_cfg;
			srng = dbr_ring_cfg->srng;
			hal_get_sw_hptp(hal_soc, srng, &tp, &hp);
			direct_buf_rx_debug("|%11d|%14s|%10x|%10x|",
					    mod_idx, g_dbr_module_name[mod_idx].
					    module_name_str,
					    hp, tp);
		}
	}
	direct_buf_rx_debug("--------------------------------------------------");

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
target_if_direct_buf_rx_get_ring_params(struct wlan_objmgr_pdev *pdev,
					struct module_ring_params *param,
					uint8_t mod_id, uint8_t srng_id)
{
	struct direct_buf_rx_pdev_obj *dbr_pdev_obj;
	struct direct_buf_rx_module_param *dbr_mod_param;

	if (!pdev) {
		direct_buf_rx_err("pdev context passed is null");
		return QDF_STATUS_E_INVAL;
	}

	dbr_pdev_obj = wlan_objmgr_pdev_get_comp_private_obj
			(pdev, WLAN_TARGET_IF_COMP_DIRECT_BUF_RX);

	if (!dbr_pdev_obj) {
		direct_buf_rx_err("dir buf rx object is null");
		return QDF_STATUS_E_FAILURE;
	}

	if ((mod_id >= DBR_MODULE_MAX) || (srng_id >= DBR_SRNG_NUM)) {
		direct_buf_rx_err("invalid params, mod id %d, srng id %d",
				  mod_id, srng_id);
		return QDF_STATUS_E_INVAL;
	}

	dbr_mod_param = &dbr_pdev_obj->dbr_mod_param[mod_id][srng_id];
	param->num_bufs = dbr_mod_param->dbr_ring_cfg->num_ptr;
	param->buf_size = dbr_mod_param->dbr_ring_cfg->buf_size;

	return QDF_STATUS_SUCCESS;
}
