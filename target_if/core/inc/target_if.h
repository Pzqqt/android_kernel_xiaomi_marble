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

/**
 * DOC: This target interface shall be used
 *      to communicate with target using WMI.
 */
#ifndef _WLAN_TARGET_IF_H_
#define _WLAN_TARGET_IF_H_

#include "qdf_types.h"
#include "qdf_util.h"
#include "wlan_objmgr_psoc_obj.h"
#include "wmi_unified_api.h"
#include "wmi_unified_priv.h"
#include "wmi_unified_param.h"

#define TGT_WMI_PDEV_ID_SOC	0	/* WMI SOC ID */

/* ASCII "TGT\0" */
#define TGT_MAGIC 0x54575400

#define target_if_fatal(params...) \
	QDF_TRACE_FATAL(QDF_MODULE_ID_TARGET_IF, params)
#define target_if_err(params...) \
	QDF_TRACE_ERROR(QDF_MODULE_ID_TARGET_IF, params)
#define target_if_warn(params...) \
	QDF_TRACE_WARN(QDF_MODULE_ID_TARGET_IF, params)
#define target_if_info(params...) \
	QDF_TRACE_INFO(QDF_MODULE_ID_TARGET_IF, params)
#define target_if_debug(params...) \
	QDF_TRACE_DEBUG(QDF_MODULE_ID_TARGET_IF, params)
#define TARGET_IF_ENTER() \
	QDF_TRACE_ENTER(QDF_MODULE_ID_TARGET_IF, "enter")
#define TARGET_IF_EXIT() \
	QDF_TRACE_EXIT(QDF_MODULE_ID_TARGET_IF, "exit")
#define target_if_err_rl(params...) \
	QDF_TRACE_ERROR_RL(QDF_MODULE_ID_TARGET_IF, params)


#define targetif_nofl_fatal(params...) \
	QDF_TRACE_FATAL_NO_FL(QDF_MODULE_ID_TARGET_IF, params)
#define targetif_nofl_err(params...) \
	QDF_TRACE_ERROR_NO_FL(QDF_MODULE_ID_TARGET_IF, params)
#define targetif_nofl_warn(params...) \
	QDF_TRACE_WARN_NO_FL(QDF_MODULE_ID_TARGET_IF, params)
#define targetif_nofl_info(params...) \
	QDF_TRACE_INFO_NO_FL(QDF_MODULE_ID_TARGET_IF, params)
#define targetif_nofl_debug(params...) \
	QDF_TRACE_DEBUG_NO_FL(QDF_MODULE_ID_TARGET_IF, params)

typedef struct wlan_objmgr_psoc *(*get_psoc_handle_callback)(
			void *scn_handle);

typedef struct wlan_objmgr_pdev *(*get_pdev_handle_callback)(
			void *scn_handle);

typedef int (*wmi_legacy_service_ready_callback)(uint32_t event_id,
						void *handle,
						uint8_t *event_data,
						uint32_t length);

/**
 * struct target_if_ctx - target_interface context
 * @magic: magic for target if ctx
 * @get_psoc_hdl_cb:  function pointer to get psoc
 * @get_pdev_hdl_cb:  function pointer to get pdev
 * @lock: spin lock for protecting the ctx
 */
struct target_if_ctx {
	uint32_t magic;
	get_psoc_handle_callback get_psoc_hdl_cb;
	get_pdev_handle_callback get_pdev_hdl_cb;
	wmi_legacy_service_ready_callback service_ready_cb;
	qdf_spinlock_t lock;
};

struct target_psoc_info;
/**
 * struct host_fw_ver - holds host fw version
 * @host_ver: Host version
 * @target_ver: Target version ID
 * @target_rev: Target revision ID
 * @wlan_ver: FW SW version
 * @wlan_ver_1: FW SW version second dword
 * @abi_ver: ABI version
 */
struct host_fw_ver {
	uint32_t host_ver;
	uint32_t target_ver;
	uint32_t target_rev;
	uint32_t wlan_ver;
	uint32_t wlan_ver_1;
	uint32_t abi_ver;
};

struct common_dbglog_handle;
struct common_accelerator_handle;

/**
 * struct comp_hdls - Non-umac/lower layer components handles, it is a sub
 *                    structure of target psoc information
 * @hif_hdl: HIF handle
 * @htc_hdl: HTC handle
 * @wmi_hdl: WMI handle
 * @accelerator_hdl: NSS offload/IPA handle
 * @dbglog_hdl: Debug log handle
 */
struct comp_hdls {
	struct hif_opaque_softc *hif_hdl;
	HTC_HANDLE htc_hdl;
	struct wmi_unified *wmi_hdl;
	struct common_accelerator_handle *accelerator_hdl;
	struct common_dbglog_handle *dbglog_hdl;
};

/**
 * struct target_supported_modes - List of HW modes supported by target.
 *
 * @num_modes: Number of modes supported
 * @hw_mode_ids: List of HW mode ids
 * @phy_bit_map: List of Phy bit maps
 */
struct target_supported_modes {
	uint8_t num_modes;
	uint32_t hw_mode_ids[WMI_HOST_HW_MODE_MAX];
	uint32_t phy_bit_map[WMI_HOST_HW_MODE_MAX];
};

/**
 * struct target_version_info - Target version information
 *
 * @reg_db_version_major: REG DB version major
 * @reg_db_version_minor: REG DB version minor
 * @bdf_reg_db_version_major: BDF REG DB version major
 * @bdf_reg_db_version_minor: BDF REG DB version minor
 */
struct target_version_info {
	uint8_t reg_db_version_major;
	uint8_t reg_db_version_minor;
	uint8_t bdf_reg_db_version_major;
	uint8_t bdf_reg_db_version_minor;
};

/**
 * struct tgt_info - FW or lower layer related info(required by target_if),
 *                   it is a sub structure of taarget psoc information
 * @version: Host FW version struct
 * @wlan_res_cfg:  target_resource_config info
 * @wlan_ext_res_cfg: wmi_host_ext_resource_config info
 * @wmi_service_ready: is service ready received
 * @wmi_ready: is ready event received
 * @total_mac_phy_cnt: num of mac phys
 * @num_radios: number of radios
 * @wlan_init_status: Target init status
 * @target_type: Target type
 * @max_descs: Max descriptors
 * @preferred_hw_mode: preferred hw mode
 * @wmi_timeout: wait timeout for target events
 * @event: qdf_event for target events
 * @service_bitmap: WMI service bitmap
 * @target_cap: target capabilities
 * @service_ext2_param: service ready ext2 event params
 * @service_ext_param: ext service params
 * @mac_phy_cap: phy caps array
 * @dbr_ring_cap: dbr_ring capability info
 * @reg_cap: regulatory caps array
 * @scaling_params: Spectral bin scaling parameters
 * @num_mem_chunks: number of mem chunks allocated
 * @hw_mode_caps: HW mode caps of preferred mode
 * @mem_chunks: allocated memory blocks for FW
 * @scan_radio_caps: scan radio capabilities
 */
struct tgt_info {
	struct host_fw_ver version;
	target_resource_config wlan_res_cfg;
	wmi_host_ext_resource_config wlan_ext_res_cfg;
	bool wmi_service_ready;
	bool wmi_ready;
	uint8_t total_mac_phy_cnt;
	uint8_t num_radios;
	uint32_t wlan_init_status;
	uint32_t target_type;
	uint32_t max_descs;
	uint32_t preferred_hw_mode;
	uint32_t wmi_timeout;
	qdf_event_t event;
	uint32_t service_bitmap[PSOC_SERVICE_BM_SIZE];
	struct wlan_psoc_target_capability_info target_caps;
	struct wlan_psoc_host_service_ext_param service_ext_param;
	struct wlan_psoc_host_service_ext2_param service_ext2_param;
	struct wlan_psoc_host_mac_phy_caps
			mac_phy_cap[PSOC_MAX_MAC_PHY_CAP];
	struct wlan_psoc_host_dbr_ring_caps *dbr_ring_cap;
	struct wlan_psoc_host_spectral_scaling_params *scaling_params;
	uint32_t num_mem_chunks;
	struct wmi_host_mem_chunk mem_chunks[MAX_MEM_CHUNKS];
	struct wlan_psoc_host_hw_mode_caps hw_mode_cap;
	struct target_supported_modes hw_modes;
	uint8_t pdev_id_to_phy_id_map[WLAN_UMAC_MAX_PDEVS];
	bool is_pdevid_to_phyid_map;
	struct wlan_psoc_host_scan_radio_caps *scan_radio_caps;
};

/**
 * struct target_ops - Holds feature specific function pointers, which would be
 *                     invoked as part of service ready or ext service ready
 * @ext_resource_config_enable: Ext resource config
 * @peer_config: Peer config enable
 * @mesh_support_enable: Mesh support enable
 * @smart_antenna_enable: Smart antenna enable
 * @atf_config_enable: ATF config enable
 * @qwrap_config_enable: QWRAP config enable
 * @btcoex_config_enable: BTCOEX config enable
 * @lteu_ext_support_enable: LTE-U Ext config enable
 * @set_init_cmd_dev_based_params: Sets Init command params
 * @alloc_pdevs: Allocates PDEVs
 * @update_pdev_tgt_info: Updates PDEV target info
 * @mem_mgr_alloc_chunk: Allocates memory through MEM manager
 * @mem_mgr_free_chunks: Free memory chunks through MEM manager
 * @print_svc_ready_ex_param: Print service ready ext params
 * @add_11ax_modes: Adds 11ax modes to reg cap
 * @set_default_tgt_config: Sets target config with default values
 * @sw_version_check: Checks the SW version
 * @smart_log_enable: Enable Smart Logs feature
 * @cfr_support_enable: CFR support enable
 * @set_pktlog_checksum: Set the pktlog checksum from FW ready event to pl_dev
 */
struct target_ops {
	QDF_STATUS (*ext_resource_config_enable)
		(struct wlan_objmgr_psoc *psoc,
		 struct target_psoc_info *tgt_info, uint8_t *event);
	void (*peer_config)
		(struct wlan_objmgr_psoc *psoc,
		 struct target_psoc_info *tgt_info, uint8_t *event);
	void (*mesh_support_enable)
		(struct wlan_objmgr_psoc *psoc,
		 struct target_psoc_info *tgt_info, uint8_t *event);
	void (*smart_antenna_enable)
		(struct wlan_objmgr_psoc *psoc,
		 struct target_psoc_info *tgt_info, uint8_t *event);
	void (*atf_config_enable)
		(struct wlan_objmgr_psoc *psoc,
		 struct target_psoc_info *tgt_info, uint8_t *event);
	void (*qwrap_config_enable)
		(struct wlan_objmgr_psoc *psoc,
		 struct target_psoc_info *tgt_info, uint8_t *event);
	void (*btcoex_config_enable)
		(struct wlan_objmgr_psoc *psoc,
		 struct target_psoc_info *tgt_info, uint8_t *event);
	void (*lteu_ext_support_enable)
		(struct wlan_objmgr_psoc *psoc,
		 struct target_psoc_info *tgt_info, uint8_t *event);
	void (*set_init_cmd_dev_based_params)
		(struct wlan_objmgr_psoc *psoc,
		 struct target_psoc_info *tgt_info);
	QDF_STATUS (*alloc_pdevs)
		(struct wlan_objmgr_psoc *psoc,
		 struct target_psoc_info *tgt_info);
	QDF_STATUS (*update_pdev_tgt_info)
		(struct wlan_objmgr_psoc *psoc,
		 struct target_psoc_info *tgt_info);
	uint32_t (*mem_mgr_alloc_chunk)(struct wlan_objmgr_psoc *psoc,
		struct target_psoc_info *tgt_info,
		u_int32_t req_id, u_int32_t idx, u_int32_t num_units,
		u_int32_t unit_len, u_int32_t num_unit_info);
	QDF_STATUS (*mem_mgr_free_chunks)(struct wlan_objmgr_psoc *psoc,
			struct target_psoc_info *tgt_hdl);
	void (*print_svc_ready_ex_param)(
		 struct wlan_objmgr_psoc *psoc,
		 struct target_psoc_info *tgt_info);
	void (*add_11ax_modes)(
		 struct wlan_objmgr_psoc *psoc,
		 struct target_psoc_info *tgt_info);
	void (*set_default_tgt_config)(
		 struct wlan_objmgr_psoc *psoc,
		 struct target_psoc_info *tgt_info);
	QDF_STATUS (*sw_version_check)(
		 struct wlan_objmgr_psoc *psoc,
		 struct target_psoc_info *tgt_hdl,
		 uint8_t *evt_buf);
	void (*eapol_minrate_enable)
		(struct wlan_objmgr_psoc *psoc,
		 struct target_psoc_info *tgt_info, uint8_t *event);
	void (*cfr_support_enable)
		(struct wlan_objmgr_psoc *psoc,
		 struct target_psoc_info *tgt_info, uint8_t *event);
	void (*set_pktlog_checksum)
		(struct wlan_objmgr_pdev *pdev, uint32_t checksum);
};



/**
 * struct target_psoc_info - target psoc information
 * @hdls: component handles (htc/htt/wmi) sub structure
 * @info: target related info sub structure
 * @feature_ptr: stores legacy pointer or few driver specific structures
 * @tif_ops: holds driver specific function pointers
 */
struct target_psoc_info {
	struct comp_hdls hdls;
	struct tgt_info info;
	void *feature_ptr;
	struct target_ops *tif_ops;
};

/**
 * struct target_pdev_info - target pdev information
 * @wmi_handle: WMI handle
 * @accelerator_hdl: NSS offload/IPA handles
 * @pdev_idx: pdev id (of FW)
 * @phy_idx: phy id (of FW)
 * @feature_ptr: stores legacy pointer or few driver specific structures
 */
struct target_pdev_info {
	struct wmi_unified *wmi_handle;
	struct common_accelerator_handle *accelerator_hdl;
	int32_t pdev_idx;
	int32_t phy_idx;
	void *feature_ptr;
};

/**
 * struct target_mu_caps - max number of users per-PPDU for OFDMA/MU-MIMO
 * @ofdma_dl: max users for Downlink OFDMA transmissions
 * @ofdma_ul: max users for Uplink OFDMA transmissions
 * @mumimo_dl: max users for Downlink MU-MIMO transmissions
 * @mumimo_ul: max users for Uplink MU-MIMO transmissions
 */
struct target_mu_caps {
	uint16_t ofdma_dl;
	uint16_t ofdma_ul;
	uint16_t mumimo_dl;
	uint16_t mumimo_ul;
};


/**
 * target_if_init() - target_if Initialization
 * @get_wmi_handle: function pointer to get wmi handle
 *
 *
 * Return: QDF_STATUS
 */
QDF_STATUS target_if_init(get_psoc_handle_callback psoc_hdl_cb);

/**
 * target_if_deinit() - Close target_if
 * @scn_handle: scn handle
 *
 * Return: QDF_STATUS_SUCCESS - in case of success
 */
QDF_STATUS target_if_deinit(void);

/**
 * target_if_store_pdev_target_if_ctx() - stores objmgr pdev in target if ctx
 * @pdev_hdl_cb: function pointer to get objmgr pdev
 *
 * Return: QDF_STATUS_SUCCESS - in case of success
 */
QDF_STATUS target_if_store_pdev_target_if_ctx(
		get_pdev_handle_callback pdev_hdl_cb);

/**
 * wlan_get_tgt_if_ctx() -Get target if ctx
 *
 * Return: target if ctx
 */
struct target_if_ctx *target_if_get_ctx(void);

/**
 * target_if_get_psoc_from_scn_hdl() - get psoc from scn handle
 * @scn_handle: scn handle
 *
 * This API is generally used while processing wmi event.
 * In wmi event SCN handle will be passed by wmi hence
 * using this API we can get psoc from scn handle.
 *
 * Return: index for matching scn handle
 */
struct wlan_objmgr_psoc *target_if_get_psoc_from_scn_hdl(void *scn_handle);

/**
 * target_if_get_pdev_from_scn_hdl() - get pdev from scn handle
 * @scn_handle: scn handle
 *
 * This API is generally used while processing wmi event.
 * In wmi event SCN handle will be passed by wmi hence
 * using this API we can get pdev from scn handle.
 *
 * Return: pdev for matching scn handle
 */
struct wlan_objmgr_pdev *target_if_get_pdev_from_scn_hdl(void *scn_handle);

/** target_if_register_tx_ops() - register tx_ops
 * @tx_ops: tx_ops structure
 *
 * This function is to be used by components to populate
 * the OL function pointers (tx_ops) required by the component
 * for UMAC-LMAC interaction, with the appropriate handler
 *
 * Return: QDF STATUS
 */
QDF_STATUS target_if_register_tx_ops(struct wlan_lmac_if_tx_ops *tx_ops);

/**
 * target_if_get_psoc_legacy_service_ready_cb() - get psoc from scn handle
 *
 * This API is generally used while processing wmi event.
 * In wmi event SCN handle will be passed by wmi hence
 * using this API we can get psoc from scn handle.
 *
 * Return: wmi_legacy_service_ready_callback
 */
wmi_legacy_service_ready_callback
		target_if_get_psoc_legacy_service_ready_cb(void);

/**
 * target_if_register_legacy_service_ready_cb() - get legacy
 *                                       service ready handler from scn handle
 *
 * @service_ready_cb: function pointer to service ready callback
 *
 * Return: QDF Status
 */
QDF_STATUS target_if_register_legacy_service_ready_cb(
	wmi_legacy_service_ready_callback service_ready_cb);

/**
 * target_if_alloc_pdev_tgt_info() - alloc pdev tgt info
 * @pdev: pointer to pdev
 *
 * API to allocate memory for target_pdev_info
 *
 * Return: SUCCESS on successful memory allocation or Failure
 */
QDF_STATUS target_if_alloc_pdev_tgt_info(struct wlan_objmgr_pdev *pdev);

/**
 * target_if_free_pdev_tgt_info() - free pdev tgt info
 * @pdev: pointer to pdev
 *
 * API to free allocated memory for target_pdev_info
 *
 * Return: SUCCESS on successful memory deallocation or Failure
 */
QDF_STATUS target_if_free_pdev_tgt_info(struct wlan_objmgr_pdev *pdev);

/**
 * target_if_alloc_psoc_tgt_info() - alloc psoc tgt info
 * @psoc: pointer to psoc
 *
 * API to allocate memory for target_psoc_info
 *
 * Return: SUCCESS on successful memory allocation or Failure
 */
QDF_STATUS target_if_alloc_psoc_tgt_info(struct wlan_objmgr_psoc *psoc);

/**
 * target_if_free_psoc_tgt_info() - free psoc tgt info
 * @psoc: pointer to psoc
 *
 * API to free allocated memory for target_psoc_info
 *
 * Return: SUCCESS on successful memory deallocation or Failure
 */
QDF_STATUS target_if_free_psoc_tgt_info(struct wlan_objmgr_psoc *psoc);

/**
 * target_is_tgt_type_ar900b() - Check if the target type is AR900B
 * @target_type: target type to be checked.
 *
 * Return: true if the target_type is AR900B, else false.
 */
bool target_is_tgt_type_ar900b(uint32_t target_type);

/**
 * target_is_tgt_type_ipq4019() - Check if the target type is IPQ4019
 * @target_type: target type to be checked.
 *
 * Return: true if the target_type is IPQ4019, else false.
 */
bool target_is_tgt_type_ipq4019(uint32_t target_type);

/**
 * target_is_tgt_type_qca9984() - Check if the target type is QCA9984
 * @target_type: target type to be checked.
 *
 * Return: true if the target_type is QCA9984, else false.
 */
bool target_is_tgt_type_qca9984(uint32_t target_type);

/**
 * target_is_tgt_type_qca9888() - Check if the target type is QCA9888
 * @target_type: target type to be checked.
 *
 * Return: true if the target_type is QCA9888, else false.
 */
bool target_is_tgt_type_qca9888(uint32_t target_type);

/**
 * target_is_tgt_type_adrastea() - Check if the target type is QCS40X
 * @target_type: target type to be checked.
 *
 * Return: true if the target_type is QCS40X, else false.
 */
bool target_is_tgt_type_adrastea(uint32_t target_type);

/**
 * target_is_tgt_type_qcn9000() - Check if the target type is QCN9000 (pine)
 * @target_type: target type to be checked.
 *
 * Return: true if the target_type is QCN9000, else false.
 */
bool target_is_tgt_type_qcn9000(uint32_t target_type);

/**
 * target_is_tgt_type_qcn9100() - Check if the target type is QCN9100 (Spruce)
 * @target_type: target type to be checked.
 *
 * Return: true if the target_type is QCN9100, else false.
 */
bool target_is_tgt_type_qcn9100(uint32_t target_type);

/**
 * target_psoc_set_wlan_init_status() - set info wlan_init_status
 * @psoc_info:          pointer to structure target_psoc_info
 * @wlan_init_status:   FW init status
 *
 * API to set wlan_init_status
 *
 * Return: void
 */
static inline void target_psoc_set_wlan_init_status
		(struct target_psoc_info *psoc_info, uint32_t wlan_init_status)
{
	if (!psoc_info)
		return;

	psoc_info->info.wlan_init_status = wlan_init_status;
}

/**
 * target_psoc_get_wlan_init_status() - get info wlan_init_status
 * @psoc_info:  pointer to structure target_psoc_info
 *
 * API to get wlan_init_status
 *
 * Return: uint32_t
 */
static inline uint32_t target_psoc_get_wlan_init_status
		(struct target_psoc_info *psoc_info)
{
	if (!psoc_info)
		return (uint32_t)-1;

	return psoc_info->info.wlan_init_status;
}

/**
 * target_psoc_set_target_type() - set info target_type
 * @psoc_info:  pointer to structure target_psoc_info
 * @target_type: Target type
 *
 * API to set target_type
 *
 * Return: void
 */
static inline void target_psoc_set_target_type
		(struct target_psoc_info *psoc_info, uint32_t target_type)
{
	if (!psoc_info)
		return;

	psoc_info->info.target_type = target_type;
}

/**
 * target_psoc_get_target_type() - get info target_type
 * @psoc_info:  pointer to structure target_psoc_info
 *
 * API to get target_type
 *
 * Return: unit32_t
 */
static inline uint32_t target_psoc_get_target_type
		(struct target_psoc_info *psoc_info)
{
	if (!psoc_info)
		return (uint32_t)-1;

	return psoc_info->info.target_type;
}

/**
 * target_psoc_set_max_descs() - set info max_descs
 * @psoc_info:  pointer to structure target_psoc_info
 * @max_descs:  Max descriptors
 *
 * API to set max_descs
 *
 * Return: void
 */
static inline void target_psoc_set_max_descs
		(struct target_psoc_info *psoc_info, uint32_t max_descs)
{
	if (!psoc_info)
		return;

	psoc_info->info.max_descs = max_descs;
}

/**
 * target_psoc_get_max_descs() - get info max_descs
 * @psoc_info:  pointer to structure target_psoc_info
 *
 * API to get max_descs
 *
 * Return: unint32_t
 */
static inline uint32_t target_psoc_get_max_descs
		(struct target_psoc_info *psoc_info)
{
	if (!psoc_info)
		return (uint32_t)-1;

	return psoc_info->info.max_descs;
}

/**
 * target_psoc_set_wmi_service_ready() - set info wmi_service_ready
 * @psoc_info:         pointer to structure target_psoc_info
 * @wmi_service_ready: service ready flag
 *
 * API to set wmi_service_ready
 *
 * Return: void
 */
static inline void target_psoc_set_wmi_service_ready
		(struct target_psoc_info *psoc_info, bool wmi_service_ready)
{
	if (!psoc_info)
		return;

	psoc_info->info.wmi_service_ready = wmi_service_ready;
}

/**
 * target_psoc_get_wmi_service_ready() - get info wmi_service_ready
 * @psoc_info:  pointer to structure target_psoc_info
 *
 * API to get wmi_service_ready
 *
 * Return: bool
 */
static inline bool target_psoc_get_wmi_service_ready
		(struct target_psoc_info *psoc_info)
{
	return psoc_info->info.wmi_service_ready;
}

/**
 * target_psoc_set_wmi_ready() - set info wmi_ready
 * @psoc_info:  pointer to structure target_psoc_info
 * @wmi_ready:  Ready event flag
 *
 * API to set wmi_ready
 *
 * Return: void
 */
static inline void target_psoc_set_wmi_ready
		(struct target_psoc_info *psoc_info, bool wmi_ready)
{
	if (!psoc_info)
		return;

	psoc_info->info.wmi_ready = wmi_ready;
}

/**
 * target_psoc_get_wmi_ready() - get info wmi_ready
 * @psoc_info:  pointer to structure target_psoc_info
 *
 * API to get wmi_ready
 *
 * Return: bool
 */
static inline bool target_psoc_get_wmi_ready
		(struct target_psoc_info *psoc_info)
{
	return psoc_info->info.wmi_ready;
}

/**
 * target_psoc_set_preferred_hw_mode() - set preferred_hw_mode
 * @psoc_info:  pointer to structure target_psoc_info
 * @preferred_hw_mode: Preferred HW mode
 *
 * API to set preferred_hw_mode
 *
 * Return: void
 */
static inline void target_psoc_set_preferred_hw_mode(
		struct target_psoc_info *psoc_info, uint32_t preferred_hw_mode)
{
	if (!psoc_info)
		return;

	psoc_info->info.preferred_hw_mode = preferred_hw_mode;
}

/**
 * target_psoc_get_preferred_hw_mode() - get preferred_hw_mode
 * @psoc_info:  pointer to structure target_psoc_info
 *
 * API to get preferred_hw_mode
 *
 * Return: unint32_t
 */
static inline uint32_t target_psoc_get_preferred_hw_mode
		(struct target_psoc_info *psoc_info)
{
	if (!psoc_info)
		return WMI_HOST_HW_MODE_MAX;

	return psoc_info->info.preferred_hw_mode;
}

/**
 * target_psoc_get_supported_hw_modes() - get supported_hw_mode in target
 * @psoc_info:  pointer to structure target_psoc_info
 *
 * API to get list of supported HW modes
 *
 * Return: pointer to target_supported_modes
 */
static inline struct target_supported_modes *target_psoc_get_supported_hw_modes
		(struct target_psoc_info *psoc_info)
{
	if (!psoc_info)
		return NULL;

	return &psoc_info->info.hw_modes;
}

/**
 * target_psoc_set_wmi_timeout() - set wmi_timeout
 * @psoc_info:  pointer to structure target_psoc_info
 * @wmi_timeout: WMI timeout value in sec
 *
 * API to set wmi_timeout
 *
 * Return: void
 */
static inline void target_psoc_set_wmi_timeout
		(struct target_psoc_info *psoc_info, uint32_t wmi_timeout)
{
	if (!psoc_info)
		return;

	psoc_info->info.wmi_timeout = wmi_timeout;
}

/**
 * target_psoc_get_wmi_timeout() - get wmi_timeout
 * @psoc_info:  pointer to structure target_psoc_info
 *
 * API to get wmi_timeout
 *
 * Return: unint32_t
 */
static inline uint32_t target_psoc_get_wmi_timeout
		(struct target_psoc_info *psoc_info)
{
	if (!psoc_info)
		return (uint32_t)-1;

	return psoc_info->info.wmi_timeout;
}

/**
 * target_psoc_set_total_mac_phy_cnt() - set total_mac_phy
 * @psoc_info:  pointer to structure target_psoc_infoa
 * @total_mac_phy_cnt: Total MAC PHY cnt
 *
 * API to set total_mac_phy
 *
 * Return: void
 */
static inline void target_psoc_set_total_mac_phy_cnt
		(struct target_psoc_info *psoc_info, uint8_t total_mac_phy_cnt)
{
	if (!psoc_info)
		return;

	psoc_info->info.total_mac_phy_cnt = total_mac_phy_cnt;
}

/**
 * target_psoc_get_total_mac_phy_cnt() - get total_mac_phy
 * @psoc_info:  pointer to structure target_psoc_info
 *
 * API to get total_mac_phy
 *
 * Return: unint8_t
 */
static inline uint8_t target_psoc_get_total_mac_phy_cnt(
		struct target_psoc_info *psoc_info)
{
	if (!psoc_info)
		return 0;

	return psoc_info->info.total_mac_phy_cnt;
}

/**
 * target_psoc_set_num_radios() - set num of radios
 * @psoc_info:  pointer to structure target_psoc_info
 * @num_radios: Number of radios
 *
 * API to set number of radios
 *
 * Return: number of radios
 */
static inline void target_psoc_set_num_radios(
		struct target_psoc_info *psoc_info, uint8_t num_radios)
{
	if (!psoc_info)
		return;

	psoc_info->info.num_radios = num_radios;
}

/**
 * target_psoc_set_pdev_id_to_phy_id_map() - set pdev to phy id mapping
 * @psoc_info:  pointer to structure target_psoc_info
 * @pdev_id: pdev id
 * @phy_id: phy_id
 *
 * API to set pdev id to phy id mapping
 *
 * Return: void
 */
static inline void target_psoc_set_pdev_id_to_phy_id_map(
		struct target_psoc_info *psoc_info,
		uint8_t *phy_id_map)
{
	if (!psoc_info)
		return;

	psoc_info->info.is_pdevid_to_phyid_map = true;
	qdf_mem_copy(psoc_info->info.pdev_id_to_phy_id_map, phy_id_map,
		     PSOC_MAX_PHY_REG_CAP);
}

/**
 * target_psoc_get_num_radios() - get number of radios
 * @psoc_info:  pointer to structure target_psoc_info
 *
 * API to get number_of_radios
 *
 * Return: number of radios
 */
static inline uint8_t target_psoc_get_num_radios
		(struct target_psoc_info *psoc_info)
{
	if (!psoc_info)
		return 0;

	return psoc_info->info.num_radios;
}

/**
 * target_psoc_get_num_radios_for_mode() - get number of radios for a hw-mode
 * @psoc_info:  pointer to structure target_psoc_info
 *
 * API to get number_of_radios for a HW mode
 *
 * Return: number of radios
 */

static inline uint8_t target_psoc_get_num_radios_for_mode
		(struct target_psoc_info *psoc_info, uint8_t mode)
{
	uint8_t mac_phy_count;
	uint8_t num_radios = 0;
	struct tgt_info *info = &psoc_info->info;

	if (!psoc_info)
		return 0;

	for (mac_phy_count = 0;
		mac_phy_count < target_psoc_get_total_mac_phy_cnt(psoc_info);
		mac_phy_count++) {
		num_radios +=
		(info->mac_phy_cap[mac_phy_count].hw_mode_id == mode);
	}

	return num_radios;
}

/**
 * target_psoc_set_service_bitmap() - set service_bitmap
 * @psoc_info:  pointer to structure target_psoc_info
 * @service_bitmap: FW service bitmap
 *
 * API to set service_bitmap
 *
 * Return: void
 */
static inline void target_psoc_set_service_bitmap
		(struct target_psoc_info *psoc_info, uint32_t *service_bitmap)
{
	qdf_mem_copy(psoc_info->info.service_bitmap, service_bitmap,
			sizeof(psoc_info->info.service_bitmap));
}

/**
 * target_psoc_get_service_bitmap() - get service_bitmap
 * @psoc_info:  pointer to structure target_psoc_info
 *
 * API to get service_bitmap
 *
 * Return: unint32_t
 */
static inline uint32_t *target_psoc_get_service_bitmap
		(struct target_psoc_info *psoc_info)
{
	return psoc_info->info.service_bitmap;
}

/**
 * target_psoc_set_num_mem_chunks - set num_mem_chunks
 * @psoc_info:  pointer to structure target_psoc_info
 & @num_mem_chunks: Num Memory chunks allocated for FW
 *
 * API to set num_mem_chunks
 *
 * Return: void
 */
static inline void target_psoc_set_num_mem_chunks(
		struct target_psoc_info *psoc_info, uint32_t num_mem_chunks)
{
	if (!psoc_info)
		return;
	psoc_info->info.num_mem_chunks = num_mem_chunks;
}

/**
 * target_psoc_get_num_mem_chunks() - get num_mem_chunks
 * @psoc_info:  pointer to structure target_psoc_info
 *
 * API to get total_mac_phy
 *
 * Return: unint8_t
 */
static inline uint32_t target_psoc_get_num_mem_chunks
		(struct target_psoc_info *psoc_info)
{
	if (!psoc_info)
		return (uint32_t)-1;

	return psoc_info->info.num_mem_chunks;
}
/**
 * target_psoc_set_hif_hdl - set hif_hdl
 * @psoc_info:  pointer to structure target_psoc_info
 * @hif_hdl:    HIF handle
 *
 * API to set hif_hdl
 *
 * Return: void
 */
static inline void target_psoc_set_hif_hdl
		(struct target_psoc_info *psoc_info,
		 struct hif_opaque_softc *hif_hdl)
{
	if (!psoc_info)
		return;

	psoc_info->hdls.hif_hdl = hif_hdl;
}

/**
 * target_psoc_get_hif_hdl() - get hif_hdl
 * @psoc_info:  pointer to structure target_psoc_info
 *
 * API to get hif_hdl
 *
 * Return: hif_hdl
 */
static inline struct hif_opaque_softc *target_psoc_get_hif_hdl
		(struct target_psoc_info *psoc_info)
{
	if (!psoc_info)
		return NULL;

	return psoc_info->hdls.hif_hdl;
}

/**
 * target_psoc_set_hif_hdl - set htc_hdl
 * @psoc_info:  pointer to structure target_psoc_info
 * @htc_hdl:    HTC handle
 *
 * API to set htc_hdl
 *
 * Return: void
 */
static inline void target_psoc_set_htc_hdl(
		struct target_psoc_info *psoc_info,
		HTC_HANDLE htc_hdl)
{
	if (!psoc_info)
		return;

	psoc_info->hdls.htc_hdl = htc_hdl;
}

/**
 * target_psoc_get_htc_hdl() - get htc_hdl
 * @psoc_info:  pointer to structure target_psoc_info
 *
 * API to get htc_hdl
 *
 * Return: htc_hdl
 */
static inline HTC_HANDLE target_psoc_get_htc_hdl
		(struct target_psoc_info *psoc_info)
{
	if (!psoc_info)
		return NULL;

	return psoc_info->hdls.htc_hdl;
}
/**
 * target_psoc_set_wmi_hdl - set wmi_hdl
 * @psoc_info:  pointer to structure target_psoc_info
 * @wmi_hdl:    WMI handle
 *
 * API to set wmi_hdl
 *
 * Return: void
 */
static inline void target_psoc_set_wmi_hdl
		(struct target_psoc_info *psoc_info,
		 struct wmi_unified *wmi_hdl)
{
	if (!psoc_info)
		return;

	psoc_info->hdls.wmi_hdl = wmi_hdl;
}

/**
 * target_psoc_get_wmi_hdl() - get wmi_hdl
 * @psoc_info:  pointer to structure target_psoc_info
 *
 * API to get wmi_hdl
 *
 * Return: wmi_hdl
 */
static inline struct wmi_unified *target_psoc_get_wmi_hdl
		(struct target_psoc_info *psoc_info)
{
	if (!psoc_info)
		return NULL;

	return psoc_info->hdls.wmi_hdl;
}

/**
 * target_psoc_set_accelerator_hdl - set accelerator_hdl
 * @psoc_info:  pointer to structure target_psoc_info
 * @accelerator_hdl: Accelator handle
 *
 * API to set accelerator_hdl
 *
 * Return: void
 */
static inline void target_psoc_set_accelerator_hdl
		(struct target_psoc_info *psoc_info,
		 struct common_accelerator_handle *accelerator_hdl)
{
	if (!psoc_info)
		return;

	psoc_info->hdls.accelerator_hdl = accelerator_hdl;
}

/**
 * target_psoc_get_accelerator_hdl() - get accelerator_hdl
 * @psoc_info:  pointer to structure target_psoc_info
 *
 * API to get accelerator_hdl
 *
 * Return: accelerator_hdl
 */
static inline
struct common_accelerator_handle *target_psoc_get_accelerator_hdl
		(struct target_psoc_info *psoc_info)
{
	if (!psoc_info)
		return NULL;

	return psoc_info->hdls.accelerator_hdl;
}

/**
 * target_psoc_set_feature_ptr - set feature_ptr
 * @psoc_info:  pointer to structure target_psoc_info
 * @feature_ptr: set feature pointer
 *
 * API to set feature_ptr
 *
 * Return: void
 */
static inline void target_psoc_set_feature_ptr
		(struct target_psoc_info *psoc_info, void *feature_ptr)
{
	if (!psoc_info)
		return;

	psoc_info->feature_ptr = feature_ptr;
}

/**
 * target_psoc_get_feature_ptr() - get feature_ptr
 * @psoc_info:  pointer to structure target_psoc_info
 *
 * API to get feature_ptr
 *
 * Return: feature_ptr
 */
static inline void *target_psoc_get_feature_ptr
		(struct target_psoc_info *psoc_info)
{
	if (!psoc_info)
		return NULL;

	return psoc_info->feature_ptr;
}

/**
 * target_psoc_get_version()- get host_fw_ver version
 * @psoc_info:  pointer to structure target_psoc_info
 *
 * API to get host_fw_ver version
 *
 * Return: void
 */
static inline struct host_fw_ver *target_psoc_get_version
		(struct target_psoc_info *psoc_info)
{
	return &psoc_info->info.version;
}

/**
 * target_psoc_get_target_ver()- get target version
 * @psoc_info:  pointer to structure target_psoc_info
 *
 * API to get target version
 *
 * Return: target version
 */
static inline uint32_t target_psoc_get_target_ver
		(struct target_psoc_info *psoc_info)
{
	return psoc_info->info.version.target_ver;
}

/**
 * target_psoc_set_target_ver()- set target version
 * @psoc_info:  pointer to structure target_psoc_info
 * @target_ver: Target version
 *
 * API to set target version
 *
 * Return: void
 */
static inline void target_psoc_set_target_ver
		(struct target_psoc_info *psoc_info, uint32_t target_ver)
{
	if (!psoc_info)
		return;

	psoc_info->info.version.target_ver = target_ver;
}

/**
 * target_psoc_set_target_rev()- set target revision
 * @psoc_info:  pointer to structure target_psoc_info
 * @target_rev: Target revision
 *
 * API to get target version
 *
 * Return: void
 */
static inline void target_psoc_set_target_rev
		(struct target_psoc_info *psoc_info, uint32_t target_rev)
{
	if (!psoc_info)
		return;

	psoc_info->info.version.target_rev = target_rev;
}

/**
 * target_psoc_get_target_rev()- get target revision
 * @psoc_info:  pointer to structure target_psoc_info
 *
 * API to get target revision
 *
 * Return: target revision
 */
static inline uint32_t target_psoc_get_target_rev
		(struct target_psoc_info *psoc_info)
{
	return psoc_info->info.version.target_rev;
}

/**
 * target_psoc_set_dbglog_hdl - set dbglog_hdl
 * @psoc_info:  pointer to structure target_psoc_info
 * @dbglog_hdl:    dbglog handle
 *
 * API to set dbglog_hdl
 *
 * Return: void
 */
static inline void target_psoc_set_dbglog_hdl
		(struct target_psoc_info *psoc_info,
		 struct common_dbglog_handle *dbglog_hdl)
{
	if (!psoc_info)
		return;

	psoc_info->hdls.dbglog_hdl = dbglog_hdl;
}

/**
 * target_psoc_get_dbglog_hdl() - get dbglog_hdl
 * @psoc_info:  pointer to structure target_psoc_info
 *
 * API to get dbglog_hdl
 *
 * Return: dbglog_hdl
 */
static inline struct common_dbglog_handle *target_psoc_get_dbglog_hdl
		(struct target_psoc_info *psoc_info)
{
	if (!psoc_info)
		return NULL;

	return psoc_info->hdls.dbglog_hdl;
}

/**
 * target_psoc_get_wlan_res_cfg() - get wlan_res_cfg
 * @psoc_info:  pointer to structure target_psoc_info
 *
 * API to get wlan_res_cfg
 *
 * Return: structure pointer to host_fw_ver
 */
static inline target_resource_config *target_psoc_get_wlan_res_cfg
		(struct target_psoc_info *psoc_info)
{
	if (!psoc_info)
		return NULL;

	return &psoc_info->info.wlan_res_cfg;
}

/**
 * target_psoc_get_wlan_ext_res_cfg() - get wlan_ext_res_cfg
 * @psoc_info:  pointer to structure target_psoc_info
 *
 * API to get wlan_ext_res_cfg
 *
 * Return: structure pointer to wmi_host_ext_resource_config
 */
static inline wmi_host_ext_resource_config *target_psoc_get_wlan_ext_res_cfg
		(struct target_psoc_info *psoc_info)
{
	if (!psoc_info)
		return NULL;

	return &psoc_info->info.wlan_ext_res_cfg;
}

/**
 * target_psoc_get_event_queue() - get event_queue
 * @psoc_info:  pointer to structure target_psoc_info
 *
 * API to get event_queue
 *
 * Return: structure pointer to qdf_wait_queue_head_t
 */
static inline qdf_event_t *target_psoc_get_event
		(struct target_psoc_info *psoc_info)
{
	if (!psoc_info)
		return NULL;

	return &psoc_info->info.event;
}

/**
 * target_psoc_get_target_caps() - get target_caps
 * @psoc_info:  pointer to structure target_psoc_info
 *
 * API to get target_caps
 *
 * Return: structure pointer to wlan_psoc_target_capability_info
 */
static inline struct wlan_psoc_target_capability_info
		*target_psoc_get_target_caps(struct target_psoc_info *psoc_info)
{
	if (!psoc_info)
		return NULL;

	return &psoc_info->info.target_caps;
}

/**
 * target_psoc_get_service_ext_param() - get service_ext_param
 * @psoc_info:  pointer to structure target_psoc_info
 *
 * API to get service_ext_param
 *
 * Return: structure pointer to wlan_psoc_host_service_ext_param
 */
static inline struct wlan_psoc_host_service_ext_param
		*target_psoc_get_service_ext_param
		(struct target_psoc_info *psoc_info)
{
	if (!psoc_info)
		return NULL;

	return &psoc_info->info.service_ext_param;
}

/**
 * target_psoc_get_num_dbr_ring_caps() - get no of dbr_ring_caps
 * @psoc_info:  pointer to structure target_psoc_info
 *
 * API to get num_dbr_ring_caps
 *
 * Return: no of dbr_ring_caps
 */
static inline uint32_t target_psoc_get_num_dbr_ring_caps
		(struct target_psoc_info *psoc_info)
{
	if (!psoc_info)
		return 0;

	if (psoc_info->info.service_ext_param.num_dbr_ring_caps)
		return psoc_info->info.service_ext_param.num_dbr_ring_caps;

	return psoc_info->info.service_ext2_param.num_dbr_ring_caps;
}

/**
 * target_psoc_get_num_scan_radio_caps() - get no of scan_radio_caps
 * @psoc_info:  pointer to structure target_psoc_info
 *
 * API to get num_scan_radio_caps
 *
 * Return: no of scan_radio_caps
 */
static inline uint32_t target_psoc_get_num_scan_radio_caps
		(struct target_psoc_info *psoc_info)
{
	if (!psoc_info)
		return 0;

	return psoc_info->info.service_ext2_param.num_scan_radio_caps;
}

/**
 * target_psoc_get_mac_phy_cap_for_mode() - get mac_phy_cap for a hw-mode
 * @psoc_info:  pointer to structure target_psoc_info
 *
 * API to get mac_phy_cap for a specified hw-mode
 *
 * Return: structure pointer to wlan_psoc_host_mac_phy_caps
 */

static inline struct wlan_psoc_host_mac_phy_caps
		*target_psoc_get_mac_phy_cap_for_mode
		(struct target_psoc_info *psoc_info, uint8_t mode)
{
	uint8_t mac_phy_idx;
	struct tgt_info *info = &psoc_info->info;

	if (!psoc_info)
		return NULL;

	for (mac_phy_idx = 0;
		mac_phy_idx < PSOC_MAX_MAC_PHY_CAP;
			mac_phy_idx++)
		if (info->mac_phy_cap[mac_phy_idx].hw_mode_id == mode)
			break;

	if (mac_phy_idx == PSOC_MAX_MAC_PHY_CAP)
		return NULL;

	return &info->mac_phy_cap[mac_phy_idx];
}

/**
 * target_psoc_get_mac_phy_cap() - get mac_phy_cap
 * @psoc_info:  pointer to structure target_psoc_info
 *
 * API to get mac_phy_cap
 *
 * Return: structure pointer to wlan_psoc_host_mac_phy_caps
 */
static inline struct wlan_psoc_host_mac_phy_caps *target_psoc_get_mac_phy_cap
		(struct target_psoc_info *psoc_info)
{
	uint32_t preferred_hw_mode;
	struct wlan_psoc_host_mac_phy_caps *mac_phy_cap;

	if (!psoc_info)
		return NULL;

	preferred_hw_mode =
		target_psoc_get_preferred_hw_mode(psoc_info);

	if (preferred_hw_mode < WMI_HOST_HW_MODE_MAX) {
		mac_phy_cap =
			target_psoc_get_mac_phy_cap_for_mode
			(psoc_info, preferred_hw_mode);
	} else {
		mac_phy_cap = psoc_info->info.mac_phy_cap;
	}

	return mac_phy_cap;
}

/**
 * target_psoc_get_dbr_ring_caps() - get dbr_ring_cap
 * @psoc_info:  pointer to structure target_psoc_info
 *
 * API to get dbr_ring_cap
 *
 * Return: structure pointer to wlan_psoc_host_dbr_ring_caps
 */
static inline struct wlan_psoc_host_dbr_ring_caps
	*target_psoc_get_dbr_ring_caps(struct target_psoc_info *psoc_info)
{
	if (!psoc_info)
		return NULL;

	return psoc_info->info.dbr_ring_cap;
}

/**
 * target_psoc_get_scan_radio_caps() - get scan_radio_cap
 * @psoc_info:  pointer to structure target_psoc_info
 *
 * API to get scan_radio_cap
 *
 * Return: structure pointer to wlan_psoc_host_scan_radio_caps
 */
static inline struct wlan_psoc_host_scan_radio_caps
	*target_psoc_get_scan_radio_caps(struct target_psoc_info *psoc_info)
{
	if (!psoc_info)
		return NULL;

	return psoc_info->info.scan_radio_caps;
}

/**
 * target_psoc_get_spectral_scaling_params() - get Spectral scaling params
 * @psoc_info:  pointer to structure target_psoc_info
 *
 * API to get Spectral scaling params
 *
 * Return: structure pointer to wlan_psoc_host_spectral_scaling_params
 */
static inline struct wlan_psoc_host_spectral_scaling_params
		*target_psoc_get_spectral_scaling_params(
		struct target_psoc_info *psoc_info)
{
	if (!psoc_info)
		return NULL;

	return psoc_info->info.scaling_params;
}

/**
 * target_psoc_get_mem_chunks() - get mem_chunks
 * @psoc_info:  pointer to structure target_psoc_info
 *
 * API to get mem_chunks
 *
 * Return: structure pointer to wmi_host_mem_chunk
 */
static inline struct wmi_host_mem_chunk *target_psoc_get_mem_chunks
		(struct target_psoc_info *psoc_info)
{
	if (!psoc_info)
		return NULL;

	return psoc_info->info.mem_chunks;
}

/**
 * target_psoc_get_tif_ops() - get tif_ops
 * @psoc_info:  pointer to structure target_psoc_info
 *
 * API to get tif_ops
 *
 * Return: structure pointer to target_ops
 */
static inline struct target_ops *target_psoc_get_tif_ops
		(struct target_psoc_info *psoc_info)
{
	if (!psoc_info)
		return NULL;

	return psoc_info->tif_ops;
}

/**
 * target_pdev_set_feature_ptr - set feature_ptr
 * @pdev_info:  pointer to structure target_pdev_info
 * @feature_ptr: Feature pointer
 *
 * API to set feature_ptr
 *
 * Return: void
 */
static inline void target_pdev_set_feature_ptr
		(struct target_pdev_info *pdev_info, void *feature_ptr)
{
	if (!pdev_info)
		return;

	pdev_info->feature_ptr = feature_ptr;
}

/**
 * target_pdev_get_feature_ptr() - get feature_ptr
 * @pdev_info:  pointer to structure target_pdev_info
 *
 * API to get feature_ptr
 *
 * Return: feature_ptr
 */
static inline void *target_pdev_get_feature_ptr
		(struct target_pdev_info *pdev_info)
{
	if (!pdev_info)
		return NULL;

	return pdev_info->feature_ptr;
}

/**
 * target_pdev_set_wmi_handle - set wmi_handle
 * @pdev_info:  pointer to structure target_pdev_info
 * @wmi_handle: WMI handle
 *
 * API to set wmi_handle
 *
 * Return: void
 */
static inline void target_pdev_set_wmi_handle
		(struct target_pdev_info *pdev_info,
		 struct wmi_unified *wmi_handle)
{
	if (!pdev_info)
		return;

	pdev_info->wmi_handle = wmi_handle;
}

/**
 * target_pdev_get_wmi_handle - get wmi_handle
 * @pdev_info:  pointer to structure target_dev_info
 *
 * API to get wmi_handle
 *
 * Return: wmi_handle
 */
static inline struct wmi_unified *target_pdev_get_wmi_handle
		(struct target_pdev_info *pdev_info)
{
	if (!pdev_info)
		return NULL;

	return pdev_info->wmi_handle;
}

/**
 * target_pdev_set_accelerator_hdl - set accelerator_hdl
 * @pdev_info:  pointer to structure target_pdev_info
 * @accelerator_hdl: Accelator handle
 *
 * API to set accelerator_hdl
 *
 * Return: void
 */
static inline void target_pdev_set_accelerator_hdl
		(struct target_pdev_info *pdev_info,
		 struct common_accelerator_handle *accelerator_hdl)
{
	if (!pdev_info)
		return;

	pdev_info->accelerator_hdl = accelerator_hdl;
}

/**
 * target_pdev_get_accelerator_hdl - get accelerator_hdl
 * @pdev_info:  pointer to structure target_dev_info
 *
 * API to get accelerator_hdl
 *
 * Return: accelerator_hdl
 */
static inline struct common_accelerator_handle *
target_pdev_get_accelerator_hdl(struct target_pdev_info *pdev_info)
{
	if (!pdev_info)
		return NULL;

	return pdev_info->accelerator_hdl;
}

/**
 * target_pdev_set_pdev_idx - set pdev_idx
 * @pdev_info:  pointer to structure target_pdev_info
 * @pdev_idx:   PDEV id of FW
 *
 * API to set pdev_idx
 *
 * Return: void
 */
static inline void target_pdev_set_pdev_idx
		(struct target_pdev_info *pdev_info, int32_t pdev_idx)
{
	if (!pdev_info)
		return;

	pdev_info->pdev_idx = pdev_idx;
}

/**
 * target_pdev_get_pdev_idx  - get pdev_idx
 * @pdev_info:  pointer to structure target_dev_info
 *
 * API to get pdev_idx
 *
 * Return: int32_t
 */
static inline int32_t  target_pdev_get_pdev_idx
		(struct target_pdev_info *pdev_info)
{
	if (!pdev_info)
		return -EINVAL;

	return pdev_info->pdev_idx;
}

/**
 * target_pdev_set_phy_idx - set phy_idx
 * @pdev_info:  pointer to structure target_pdev_info
 * @phy_idx:    phy ID of FW
 *
 * API to set phy_idx
 *
 * Return: void
 */
static inline void target_pdev_set_phy_idx
		(struct target_pdev_info *pdev_info, int32_t phy_idx)
{
	if (!pdev_info)
		return;

	pdev_info->phy_idx  = phy_idx;
}

/**
 * target_pdev_get_phy_idx  - get phy_idx
 * @pdev_info:  pointer to structure target_dev_info
 *
 * API to get phy_idx
 *
 * Return: int32_t
 */
static inline int32_t target_pdev_get_phy_idx
		(struct target_pdev_info *pdev_info)
{
	if (!pdev_info)
		return -EINVAL;

	return pdev_info->phy_idx;
}

/**
 * GET_WMI_HDL_FROM_PSOC - get wmi handle from psoc
 * @psoc:  psoc object
 *
 * API to get wmi_handle from psoc
 *
 * Return: wmi_handle on success
 *         if tgt handle is not initialized, it returns NULL
 */
static inline struct wmi_unified *GET_WMI_HDL_FROM_PSOC(
		struct wlan_objmgr_psoc *psoc)
{
	struct target_psoc_info *tgt_if_handle;

	if (psoc) {
		tgt_if_handle = psoc->tgt_if_handle;

		if (tgt_if_handle)
			return target_psoc_get_wmi_hdl(tgt_if_handle);
		else
			return NULL;
	}

	return NULL;
}

/**
 * GET_WMI_HDL_FROM_PDEV - get wmi handle from pdev
 * @pdev:  pdev object
 *
 * API to get wmi_handle from pdev
 *
 * Return: wmi_handle on success
 *         if tgt handle is not initialized, it returns NULL
 */
static inline struct wmi_unified *GET_WMI_HDL_FROM_PDEV(
		struct wlan_objmgr_pdev *pdev)
{
	struct target_pdev_info *tgt_if_handle;

	if (pdev) {
		tgt_if_handle =  pdev->tgt_if_handle;

		if (tgt_if_handle)
			return target_pdev_get_wmi_handle(tgt_if_handle);
		else
			return NULL;
	}

	return NULL;
}

/**
 * get_wmi_unified_hdl_from_psoc - get wmi handle from psoc
 * @psoc:  psoc object
 *
 * API to get wmi_handle from psoc
 *
 * Return: wmi_handle on success
 *         if tgt handle is not initialized, it returns NULL
 */
static inline wmi_unified_t
get_wmi_unified_hdl_from_psoc(struct wlan_objmgr_psoc *psoc)
{
	return (wmi_unified_t)GET_WMI_HDL_FROM_PSOC(psoc);
}

/**
 * get_wmi_unified_hdl_from_pdev - get wmi handle from pdev
 * @pdev:  pdev object
 *
 * API to get wmi_handle from pdev
 *
 * Return: wmi_handle on success
 *         if tgt handle is not initialized, it returns NULL
 */
static inline wmi_unified_t
get_wmi_unified_hdl_from_pdev(struct wlan_objmgr_pdev *pdev)
{
	return (wmi_unified_t)GET_WMI_HDL_FROM_PDEV(pdev);
}

/**
 * target_if_ext_res_cfg_enable - Enable ext resource config
 * @psoc:  psoc object
 * @tgt_hdl: target_psoc_info pointer
 * @evt_buf: Event buffer received from FW
 *
 * API to enable Ext resource config
 *
 * Return: none
 */
static inline void target_if_ext_res_cfg_enable(struct wlan_objmgr_psoc *psoc,
			struct target_psoc_info *tgt_hdl, uint8_t *evt_buf)
{
	if ((tgt_hdl->tif_ops) &&
		(tgt_hdl->tif_ops->ext_resource_config_enable))
		tgt_hdl->tif_ops->ext_resource_config_enable(psoc,
				tgt_hdl, evt_buf);
}

/**
 * target_if_peer_cfg_enable - Enable peer config
 * @psoc:  psoc object
 * @tgt_hdl: target_psoc_info pointer
 * @evt_buf: Event buffer received from FW
 *
 * API to enable peer config
 *
 * Return: none
 */
static inline void target_if_peer_cfg_enable(struct wlan_objmgr_psoc *psoc,
			struct target_psoc_info *tgt_hdl, uint8_t *evt_buf)
{
	if ((tgt_hdl->tif_ops) &&
		(tgt_hdl->tif_ops->peer_config))
		tgt_hdl->tif_ops->peer_config(psoc, tgt_hdl, evt_buf);
}

/**
 * target_if_mesh_support_enable - Enable MESH mode support
 * @psoc:  psoc object
 * @tgt_hdl: target_psoc_info pointer
 * @evt_buf: Event buffer received from FW
 *
 * API to enable Mesh mode
 *
 * Return: none
 */
static inline void target_if_mesh_support_enable(struct wlan_objmgr_psoc *psoc,
			struct target_psoc_info *tgt_hdl, uint8_t *evt_buf)
{
	if ((tgt_hdl->tif_ops) &&
		(tgt_hdl->tif_ops->mesh_support_enable))
		tgt_hdl->tif_ops->mesh_support_enable(psoc, tgt_hdl, evt_buf);
}

/**
 * target_if_eapol_minrate_enable - Enable EAPOL Minrate in Tunnel Mode
 * @psoc: psoc object
 * @tgt_hdl: target_psoc_info pointer
 * @evt_buf: Event buffer received from FW
 *
 * API to enable eapol minrate
 *
 * Return: none
 */
static inline void target_if_eapol_minrate_enable(struct wlan_objmgr_psoc *psoc,
			struct target_psoc_info *tgt_hdl, uint8_t *evt_buf)
{
	if ((tgt_hdl->tif_ops) &&
	    (tgt_hdl->tif_ops->eapol_minrate_enable))
		tgt_hdl->tif_ops->eapol_minrate_enable(psoc, tgt_hdl, evt_buf);
}

/**
 * target_if_smart_antenna_enable - Enable Smart antenna module
 * @psoc:  psoc object
 * @tgt_hdl: target_psoc_info pointer
 * @evt_buf: Event buffer received from FW
 *
 * API to enable Smart antenna
 *
 * Return: none
 */
static inline void target_if_smart_antenna_enable(struct wlan_objmgr_psoc *psoc,
			struct target_psoc_info *tgt_hdl, uint8_t *evt_buf)
{
	if ((tgt_hdl->tif_ops) &&
		(tgt_hdl->tif_ops->smart_antenna_enable))
		tgt_hdl->tif_ops->smart_antenna_enable(psoc, tgt_hdl, evt_buf);
}

/**
 * target_if_cfr_support_enable - Enable cfr support
 * @psoc:  psoc object
 * @tgt_hdl: target_psoc_info pointer
 * @evt_buf: Event buffer received from FW
 *
 * API to enable cfr support
 *
 * Return: none
 */
static inline void target_if_cfr_support_enable(struct wlan_objmgr_psoc *psoc,
			struct target_psoc_info *tgt_hdl, uint8_t *evt_buf)
{
	if ((tgt_hdl->tif_ops) &&
	    (tgt_hdl->tif_ops->cfr_support_enable))
		tgt_hdl->tif_ops->cfr_support_enable(psoc, tgt_hdl, evt_buf);
}

/**
 * target_if_set_pktlog_checksum - Set pktlog checksum
 * @pdev: pdev object
 * @tgt_hdl: target_psoc_info pointer
 * @checksum: checksum received from FW
 *
 * API to set pktlog checksum
 *
 * Return: none
 */
static inline void target_if_set_pktlog_checksum(struct wlan_objmgr_pdev *pdev,
			struct target_psoc_info *tgt_hdl, uint32_t checksum)
{
	if ((tgt_hdl->tif_ops) &&
	    (tgt_hdl->tif_ops->set_pktlog_checksum))
		tgt_hdl->tif_ops->set_pktlog_checksum(pdev, checksum);
}

/**
 * target_if_atf_cfg_enable - Enable ATF config
 * @psoc:  psoc object
 * @tgt_hdl: target_psoc_info pointer
 * @evt_buf: Event buffer received from FW
 *
 * API to enable ATF config
 *
 * Return: none
 */
static inline void target_if_atf_cfg_enable(struct wlan_objmgr_psoc *psoc,
			struct target_psoc_info *tgt_hdl, uint8_t *evt_buf)
{
	if ((tgt_hdl->tif_ops) &&
		(tgt_hdl->tif_ops->atf_config_enable))
		tgt_hdl->tif_ops->atf_config_enable(psoc, tgt_hdl, evt_buf);
}

/**
 * target_if_qwrap_cfg_enable - Enable QWRAP config
 * @psoc:  psoc object
 * @tgt_hdl: target_psoc_info pointer
 * @evt_buf: Event buffer received from FW
 *
 * API to enable QWRAP config
 *
 * Return: none
 */
static inline void target_if_qwrap_cfg_enable(struct wlan_objmgr_psoc *psoc,
			struct target_psoc_info *tgt_hdl, uint8_t *evt_buf)
{
	if ((tgt_hdl->tif_ops) &&
		(tgt_hdl->tif_ops->qwrap_config_enable))
		tgt_hdl->tif_ops->qwrap_config_enable(psoc, tgt_hdl, evt_buf);
}

/**
 * target_if_btcoex_cfg_enable - Enable BT coex config
 * @psoc:  psoc object
 * @tgt_hdl: target_psoc_info pointer
 * @evt_buf: Event buffer received from FW
 *
 * API to enable BT coex config
 *
 * Return: none
 */
static inline void target_if_btcoex_cfg_enable(struct wlan_objmgr_psoc *psoc,
			struct target_psoc_info *tgt_hdl, uint8_t *evt_buf)
{
	if ((tgt_hdl->tif_ops) &&
		(tgt_hdl->tif_ops->btcoex_config_enable))
		tgt_hdl->tif_ops->btcoex_config_enable(psoc, tgt_hdl, evt_buf);
}

/**
 * target_if_lteu_cfg_enable - Enable LTEU config
 * @psoc:  psoc object
 * @tgt_hdl: target_psoc_info pointer
 * @evt_buf: Event buffer received from FW
 *
 * API to enable LTEU coex config
 *
 * Return: none
 */
static inline void target_if_lteu_cfg_enable(struct wlan_objmgr_psoc *psoc,
			struct target_psoc_info *tgt_hdl, uint8_t *evt_buf)
{
	if ((tgt_hdl->tif_ops) &&
		(tgt_hdl->tif_ops->lteu_ext_support_enable))
		tgt_hdl->tif_ops->lteu_ext_support_enable(psoc,	tgt_hdl,
								evt_buf);
}

/**
 * target_if_set_init_cmd_dev_param - Set init command params
 * @psoc:  psoc object
 * @tgt_hdl: target_psoc_info pointer
 *
 * API to set init command param based on config
 *
 * Return: none
 */
static inline void target_if_set_init_cmd_dev_param(
	struct wlan_objmgr_psoc *psoc, struct target_psoc_info *tgt_hdl)
{
	if ((tgt_hdl->tif_ops) &&
		(tgt_hdl->tif_ops->set_init_cmd_dev_based_params)) {
		tgt_hdl->tif_ops->set_init_cmd_dev_based_params(psoc,
					tgt_hdl);
	}
}

/**
 * target_if_alloc_pdevs - Allocate PDEVs
 * @psoc:  psoc object
 * @tgt_hdl: target_psoc_info pointer
 *
 * API allocates PDEVs based on ext service ready param
 *
 * Return: SUCCESS on pdev allocation or PDEV allocation is not needed
 *         FAILURE, if allocation fails
 */
static inline QDF_STATUS target_if_alloc_pdevs(struct wlan_objmgr_psoc *psoc,
					struct target_psoc_info *tgt_hdl)
{
	QDF_STATUS ret_val;

	if ((tgt_hdl->tif_ops) &&
		(tgt_hdl->tif_ops->alloc_pdevs))
		ret_val = tgt_hdl->tif_ops->alloc_pdevs(psoc, tgt_hdl);
	else
		ret_val = QDF_STATUS_SUCCESS;

	return ret_val;
}

/**
 * target_if_update_pdev_tgt_info - Update PDEVs info
 * @psoc:  psoc object
 * @tgt_hdl: target_psoc_info pointer
 *
 * API updates PDEVs info based on config
 *
 * Return: SUCCESS on pdev updation or PDEV updation is not needed
 *         FAILURE, if updation fails
 */
static inline QDF_STATUS target_if_update_pdev_tgt_info(
	struct wlan_objmgr_psoc *psoc, struct target_psoc_info *tgt_hdl)
{
	QDF_STATUS ret_val;

	if ((tgt_hdl->tif_ops) &&
		(tgt_hdl->tif_ops->update_pdev_tgt_info))
		ret_val = tgt_hdl->tif_ops->update_pdev_tgt_info(psoc,
							tgt_hdl);
	else
		ret_val = QDF_STATUS_SUCCESS;

	return ret_val;
}

/**
 * target_if_print_service_ready_ext_param - Print Service ready ext param
 * @psoc:  psoc object
 * @tgt_hdl: target_psoc_info pointer
 *
 * API to print service ready ext param
 *
 * Return: none
 */
static inline void target_if_print_service_ready_ext_param(
	struct wlan_objmgr_psoc *psoc, struct target_psoc_info *tgt_hdl)
{
	if ((tgt_hdl->tif_ops) &&
		(tgt_hdl->tif_ops->print_svc_ready_ex_param)) {
		tgt_hdl->tif_ops->print_svc_ready_ex_param(psoc,
			tgt_hdl);
	}
}

/**
 * target_if_add_11ax_modes - Add 11ax modes explicitly
 * @psoc:  psoc object
 * @tgt_hdl: target_psoc_info pointer
 *
 * API to adds 11ax modes
 *
 * Return: none
 */
#ifdef QCA_HOST_ADD_11AX_MODE_WAR
static inline void target_if_add_11ax_modes(struct wlan_objmgr_psoc *psoc,
					    struct target_psoc_info *tgt_hdl)
{
	if ((tgt_hdl->tif_ops) &&
		(tgt_hdl->tif_ops->add_11ax_modes)) {
		tgt_hdl->tif_ops->add_11ax_modes(psoc, tgt_hdl);
	}
}
#else
static inline void target_if_add_11ax_modes(struct wlan_objmgr_psoc *psoc,
					    struct target_psoc_info *tgt_hdl)
{
}
#endif

/**
 * target_if_set_default_config - Set default config in init command
 * @psoc:  psoc object
 * @tgt_hdl: target_psoc_info pointer
 *
 * API to set default config in init command
 *
 * Return: none
 */
static inline void target_if_set_default_config(struct wlan_objmgr_psoc *psoc,
					struct target_psoc_info *tgt_hdl)
{
	if ((tgt_hdl->tif_ops) &&
		(tgt_hdl->tif_ops->set_default_tgt_config)) {
		tgt_hdl->tif_ops->set_default_tgt_config(psoc, tgt_hdl);
	}
}

/**
 * target_if_sw_version_check - SW version check
 * @psoc:  psoc object
 * @tgt_hdl: target_psoc_info pointer
 * @evt_buf: Event buffer received from FW
 *
 * API checks the SW version
 *
 * Return: SUCCESS on version matches or version check is not needed
 *         FAILURE, if check fails
 */
static inline QDF_STATUS target_if_sw_version_check(
			struct wlan_objmgr_psoc *psoc,
			struct target_psoc_info *tgt_hdl, uint8_t *evt_buf)
{
	QDF_STATUS ret_val;

	if ((tgt_hdl->tif_ops) &&
		(tgt_hdl->tif_ops->sw_version_check))
		ret_val = tgt_hdl->tif_ops->sw_version_check(psoc, tgt_hdl,
								evt_buf);
	else
		ret_val = QDF_STATUS_SUCCESS;

	return ret_val;
}

/**
 * target_if_get_phy_capability  - get phy capability
 * @target_psoc_info:  pointer to structure target_psoc_info
 *
 * API to get phy capability from the target caps
 *
 * Return: int32_t
 */
static inline int32_t target_if_get_phy_capability
			(struct target_psoc_info *target_psoc_info)
{
	if (!target_psoc_info)
		return -EINVAL;

	return target_psoc_info->info.target_caps.phy_capability;
}

/**
 * target_if_set_phy_capability  - set phy capability
 * @target_psoc_info:  pointer to structure target_psoc_info
 * @phy_capab: PHY capabilities
 *
 * API to set phy capability in the target caps
 *
 * Return: None
 */
static inline void target_if_set_phy_capability
		(struct target_psoc_info *target_psoc_info, int phy_capability)
{
	if (!target_psoc_info)
		return;

	target_psoc_info->info.target_caps.phy_capability = phy_capability;
}

/**
 * target_if_set_max_frag_entry  - set Maximum frag entries
 * @target_psoc_info:  pointer to structure target_psoc_info
 * @max_frag_entry: Maximum frag entries
 *
 * API to set Maximum frag entries from the target caps
 *
 * Return: None
 */
static inline void target_if_set_max_frag_entry
		(struct target_psoc_info *target_psoc_info, int max_frag_entry)
{
	if (!target_psoc_info)
		return;

	target_psoc_info->info.target_caps.max_frag_entry = max_frag_entry;
}

/**
 * target_if_get_max_frag_entry  - get Maximum frag entries
 * @target_psoc_info:  pointer to structure target_psoc_info
 *
 * API to get Maximum frag entries from the target caps
 *
 * Return: int32_t
 */
static inline int32_t target_if_get_max_frag_entry
		(struct target_psoc_info *target_psoc_info)
{
	if (!target_psoc_info)
		return -EINVAL;

	return target_psoc_info->info.target_caps.max_frag_entry;
}

/**
 * target_if_get_ht_cap_info  - get ht capabilities info
 * @target_psoc_info:  pointer to structure target_psoc_info
 *
 * API to get ht capabilities info from the target caps
 *
 * Return: int32_t
 */
static inline int32_t target_if_get_ht_cap_info
		(struct target_psoc_info *target_psoc_info)
{
	if (!target_psoc_info)
		return -EINVAL;

	return target_psoc_info->info.target_caps.ht_cap_info;
}

/**
 * target_if_get_vht_cap_info  - get vht capabilities info
 * @target_psoc_info:  pointer to structure target_psoc_info
 *
 * API to get vht capabilities info from the target caps
 *
 * Return: int32_t
 */
static inline int32_t target_if_get_vht_cap_info
		(struct target_psoc_info *target_psoc_info)
{
	if (!target_psoc_info)
		return -EINVAL;

	return target_psoc_info->info.target_caps.vht_cap_info;
}

/**
 * target_if_get_num_rf_chains  - get Number of RF chains supported
 * @target_psoc_info:  pointer to structure target_psoc_info
 *
 * API to get Number of RF chains supported from the target caps
 *
 * Return: int32_t
 */
static inline int32_t target_if_get_num_rf_chains
		(struct target_psoc_info *target_psoc_info)
{
	if (!target_psoc_info)
		return -EINVAL;

	return target_psoc_info->info.target_caps.num_rf_chains;
}

/**
 * target_if_get_fw_version  - get firmware version
 * @target_psoc_info:  pointer to structure target_psoc_info
 *
 * API to get firmware version from the target caps
 *
 * Return: int32_t
 */
static inline int32_t target_if_get_fw_version
		(struct target_psoc_info *target_psoc_info)
{
	if (!target_psoc_info)
		return 0;

	return target_psoc_info->info.target_caps.fw_version;
}

/**
 * target_if_get_wmi_fw_sub_feat_caps  - FW sub feature capabilities
 * @target_psoc_info:  pointer to structure target_psoc_info
 *
 * API to get FW sub feature capabilities from the target caps
 *
 * Return: int32_t
 */
static inline int32_t target_if_get_wmi_fw_sub_feat_caps
		(struct target_psoc_info *target_psoc_info)
{
	if (!target_psoc_info)
		return -EINVAL;

	return target_psoc_info->info.target_caps.wmi_fw_sub_feat_caps;
}

/**
 * target_if_get_conc_scan_config_bits  - Default concurrenct scan config
 * @tgt_hdl:  pointer to structure target_psoc_info
 *
 * API to get Default concurrenct scan config from the target caps
 *
 * Return: int32_t
 */
static inline int32_t target_if_get_conc_scan_config_bits
		(struct target_psoc_info *tgt_hdl)
{
	if (!tgt_hdl)
		return -EINVAL;

	return tgt_hdl->info.service_ext_param.default_conc_scan_config_bits;
}

/**
 * target_if_get_fw_config_bits  - Default HW config bits
 * @tgt_hdl:  pointer to structure target_psoc_info
 *
 * API to get Default HW config bits from the target caps
 *
 * Return: int32_t
 */
static inline int32_t target_if_get_fw_config_bits
		(struct target_psoc_info *tgt_hdl)
{
	if (!tgt_hdl)
		return -EINVAL;

	return tgt_hdl->info.service_ext_param.default_fw_config_bits;
}

/**
 * target_psoc_get_num_hw_modes  - get number of dbs hardware modes
 * @tgt_hdl:  pointer to structure target_psoc_info
 *
 * API to get Number of Dual Band Simultaneous (DBS) hardware modes
 *
 * Return: int32_t
 */
static inline int32_t target_psoc_get_num_hw_modes
		(struct target_psoc_info *tgt_hdl)
{
	if (!tgt_hdl)
		return -EINVAL;

	return tgt_hdl->info.service_ext_param.num_hw_modes;
}

#ifdef WLAN_SUPPORT_TWT
#ifdef WLAN_TWT_AP_PDEV_COUNT_NUM_PHY
static inline void target_if_set_twt_ap_pdev_count
		(struct tgt_info *info, struct target_psoc_info *tgt_hdl)
{
	if (!tgt_hdl)
		return;

	info->wlan_res_cfg.twt_ap_pdev_count =
					info->service_ext_param.num_phy;
}
#else
static inline void target_if_set_twt_ap_pdev_count
		(struct tgt_info *info, struct target_psoc_info *tgt_hdl)
{
	if (!tgt_hdl)
		return;

	info->wlan_res_cfg.twt_ap_pdev_count =
					target_psoc_get_num_radios(tgt_hdl);
}
#endif /* WLAN_TWT_AP_PDEV_COUNT_NUM_PHY */
#else
static inline void target_if_set_twt_ap_pdev_count
		(struct tgt_info *info, struct target_psoc_info *tgt_hdl)
{
}
#endif /* WLAN_SUPPORT_TWT */

/**
 * target_psoc_get_version_info() - Get version info from tgt info
 * @psoc_info: pointer to structure target_psoc_info
 * @reg_major: reg db version major
 * @reg_minor: reg db version minor
 * @bdf_major: bdf reg db version major
 * @bdf_minor: bdf reg db version minor
 *
 * API to get target version information.
 *
 * Return: void
 */
static inline void target_psoc_get_version_info(
					struct target_psoc_info *psoc_info,
					uint8_t *reg_major, uint8_t *reg_minor,
					uint8_t *bdf_major, uint8_t *bdf_minor)
{
	if (!psoc_info)
		return;

	*reg_major = psoc_info->info.service_ext2_param.reg_db_version_major;
	*reg_minor = psoc_info->info.service_ext2_param.reg_db_version_minor;
	*bdf_major =
		psoc_info->info.service_ext2_param.bdf_reg_db_version_major;
	*bdf_minor =
		psoc_info->info.service_ext2_param.bdf_reg_db_version_minor;
}

/**
 * target_psoc_get_chan_width_switch_num_peers() - Get peer limit
 * @psoc_info: pointer to structure target_psoc_info
 *
 * API to get the number of peers supported per WMI command with the ID
 * WMI_PEER_CHAN_WIDTH_SWITCH_CMDID.
 *
 * Return: maximum peers allowed in a single WMI command with the given ID.
 */
static inline uint32_t target_psoc_get_chan_width_switch_num_peers(
					    struct target_psoc_info *psoc_info)
{
	if (!psoc_info)
		return 0;

	return psoc_info->info.service_ext2_param.chwidth_num_peer_caps;
}

/**
 * target_if_is_scan_radio_supported() - API to check scan radio
 * support for the given radio
 * @pdev: pointer to pdev
 * @is_scan_radio_supported: pointer to scan radio support flag
 *
 * API to check scan radio support for the given radio
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
target_pdev_is_scan_radio_supported(struct wlan_objmgr_pdev *pdev,
				    bool *is_scan_radio_supported);

/**
 * target_pdev_scan_radio_is_dfs_enabled() - API to check
 * whether DFS needs to be enabled/disabled for scan radio.
 * @pdev:  pointer to pdev
 * @is_dfs_en: Pointer to DFS enable flag
 *
 * API to check whether DFS needs to be enabled/disabled for
 * scan radio. This API should be used only for a scan radio
 * pdev.
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
target_pdev_scan_radio_is_dfs_enabled(struct wlan_objmgr_pdev *pdev,
				      bool *is_dfs_en);

/**
 * target_psoc_get_preamble_puncture_cap() - Get Preamble Puncturing capability
 * @psoc_info: pointer to structure target_psoc_info
 *
 * API to get the target capability for Preamble Punctured Tx
 *
 * Return: target capability for Preamble Punctured Tx.
 */
static inline uint32_t target_psoc_get_preamble_puncture_cap(
					    struct target_psoc_info *psoc_info)
{
	if (!psoc_info)
		return 0;

	return psoc_info->info.service_ext2_param.preamble_puncture_bw_cap;
}

/**
 * target_psoc_get_mu_max_users() - Get max users for MU transmissions
 * @psoc_info: pointer to structure target_psoc_info
 * @mu_caps: pointer to structure for max OFDMA/MU-MIMO users per-PPDU
 *
 * API to get the max number of users per-PPDU supported for Uplink/Downlink
 * MU transmissions.
 *
 * Return: void
 */
static inline void target_psoc_get_mu_max_users(
					struct target_psoc_info *psoc_info,
					struct target_mu_caps *mu_caps)
{
	struct wlan_psoc_host_service_ext2_param *service_ext2_param;

	if (!psoc_info || !mu_caps)
		return;

	service_ext2_param = &psoc_info->info.service_ext2_param;

	mu_caps->ofdma_dl = service_ext2_param->max_users_dl_ofdma;
	mu_caps->ofdma_ul = service_ext2_param->max_users_ul_ofdma;
	mu_caps->mumimo_dl = service_ext2_param->max_users_dl_mumimo;
	mu_caps->mumimo_ul = service_ext2_param->max_users_ul_mumimo;
}

#endif
