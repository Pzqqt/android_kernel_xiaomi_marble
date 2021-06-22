/*
 * Copyright (c) 2017-2021, The Linux Foundation. All rights reserved.
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
 * DOC: i_qdf_ipa_wdi3.h
 * This file provides OS dependent IPA WDI APIs.
 */

#ifndef I_QDF_IPA_WDI_H
#define I_QDF_IPA_WDI_H

#ifdef IPA_OFFLOAD

#include <qdf_status.h>         /* QDF_STATUS */
#include <linux/ipa_wdi3.h>

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0)) || \
	defined(CONFIG_IPA_WDI_UNIFIED_API)

/**
 * __qdf_ipa_wdi_version_t - IPA WDI version
 */
typedef enum ipa_wdi_version __qdf_ipa_wdi_version_t;

/**
 * __qdf_ipa_wdi_init_in_params_t - wdi init input parameters
 */
typedef struct ipa_wdi_init_in_params __qdf_ipa_wdi_init_in_params_t;

#define __QDF_IPA_WDI_INIT_IN_PARAMS_WDI_VERSION(in_params)	\
	(((struct ipa_wdi_init_in_params *)(in_params))->wdi_version)
#define __QDF_IPA_WDI_INIT_IN_PARAMS_NOTIFY(in_params)	\
	(((struct ipa_wdi_init_in_params *)(in_params))->notify)
#define __QDF_IPA_WDI_INIT_IN_PARAMS_PRIV(in_params)	\
	(((struct ipa_wdi_init_in_params *)(in_params))->priv)
#define __QDF_IPA_WDI_INIT_IN_PARAMS_WDI_NOTIFY(in_params)	\
	(((struct ipa_wdi_init_in_params *)(in_params))->wdi_notify)

/**
 * __qdf_ipa_wdi_init_out_params_t - wdi init output parameters
 */
typedef struct ipa_wdi_init_out_params __qdf_ipa_wdi_init_out_params_t;

#define __QDF_IPA_WDI_INIT_OUT_PARAMS_IS_UC_READY(out_params)	\
	(((struct ipa_wdi_init_out_params *)(out_params))->is_uC_ready)
#define __QDF_IPA_WDI_INIT_OUT_PARAMS_IS_SMMU_ENABLED(out_params)	\
	(((struct ipa_wdi_init_out_params *)(out_params))->is_smmu_enabled)
#if (defined(IPA_WDI3_GSI)) || (defined(IPA_WDI2_GSI))
#define QDF_IPA_WDI_INIT_OUT_PARAMS_IS_OVER_GSI(out_params)	\
	(((struct ipa_wdi_init_out_params *)(out_params))->is_over_gsi)
#else
#define QDF_IPA_WDI_INIT_OUT_PARAMS_IS_OVER_GSI(out_params)	\
	false
#endif

/**
 * __qdf_ipa_wdi_hdr_info_t - Header to install on IPA HW
 */
typedef struct ipa_wdi_hdr_info  __qdf_ipa_wdi_hdr_info_t;

#define __QDF_IPA_WDI_HDR_INFO_HDR(hdr_info)	\
	(((struct ipa_wdi_hdr_info *)(hdr_info))->hdr)
#define __QDF_IPA_WDI_HDR_INFO_HDR_LEN(hdr_info)	\
	(((struct ipa_wdi_hdr_info *)(hdr_info))->hdr_len)
#define __QDF_IPA_WDI_HDR_INFO_DST_MAC_ADDR_OFFSET(hdr_info)	\
	(((struct ipa_wdi_hdr_info *)(hdr_info))->dst_mac_addr_offset)
#define __QDF_IPA_WDI_HDR_INFO_HDR_TYPE(hdr_info)	\
	(((struct ipa_wdi_hdr_info *)(hdr_info))->hdr_type)

/**
 * __qdf_ipa_wdi_reg_intf_in_params_t - parameters for uC offload
 *	interface registration
 */
typedef struct ipa_wdi_reg_intf_in_params  __qdf_ipa_wdi_reg_intf_in_params_t;

#define __QDF_IPA_WDI_REG_INTF_IN_PARAMS_NETDEV_NAME(in)	\
	(((struct ipa_wdi_reg_intf_in_params *)(in))->netdev_name)
#define __QDF_IPA_WDI_REG_INTF_IN_PARAMS_HDR_INFO(in)	\
	(((struct ipa_wdi_reg_intf_in_params *)(in))->hdr_info)
#define __QDF_IPA_WDI_REG_INTF_IN_PARAMS_ALT_DST_PIPE(in)	\
	(((struct ipa_wdi_reg_intf_in_params *)(in))->alt_dst_pipe)
#define __QDF_IPA_WDI_REG_INTF_IN_PARAMS_IS_META_DATA_VALID(in)	\
	(((struct ipa_wdi_reg_intf_in_params *)(in))->is_meta_data_valid)
#define __QDF_IPA_WDI_REG_INTF_IN_PARAMS_META_DATA(in)	\
	(((struct ipa_wdi_reg_intf_in_params *)(in))->meta_data)
#define __QDF_IPA_WDI_REG_INTF_IN_PARAMS_META_DATA_MASK(in)	\
	(((struct ipa_wdi_reg_intf_in_params *)(in))->meta_data_mask)
#ifdef IPA_WDI3_TX_TWO_PIPES
#define __QDF_IPA_WDI_REG_INTF_IN_PARAMS_IS_TX1_USED(in)	\
	(((struct ipa_wdi_reg_intf_in_params *)(in))->is_tx1_used)
#endif

typedef struct ipa_ep_cfg __qdf_ipa_ep_cfg_t;

#define __QDF_IPA_EP_CFG_NAT_EN(cfg)	\
	(((struct ipa_ep_cfg *)(cfg))->nat.nat_en)
#define __QDF_IPA_EP_CFG_HDR_LEN(cfg)	\
	(((struct ipa_ep_cfg *)(cfg))->hdr.hdr_len)
#define __QDF_IPA_EP_CFG_HDR_OFST_METADATA_VALID(cfg)	\
	(((struct ipa_ep_cfg *)(cfg))->hdr.hdr_ofst_metadata_valid)
#define __QDF_IPA_EP_CFG_HDR_METADATA_REG_VALID(cfg)	\
	(((struct ipa_ep_cfg *)(cfg))->hdr.hdr_metadata_reg_valid)
#define __QDF_IPA_EP_CFG_HDR_OFST_PKT_SIZE_VALID(cfg)	\
	(((struct ipa_ep_cfg *)(cfg))->hdr.hdr_ofst_pkt_size_valid)
#define __QDF_IPA_EP_CFG_HDR_OFST_PKT_SIZE(cfg)	\
	(((struct ipa_ep_cfg *)(cfg))->hdr.hdr_ofst_pkt_size)
#define __QDF_IPA_EP_CFG_HDR_ADDITIONAL_CONST_LEN(cfg)	\
	(((struct ipa_ep_cfg *)(cfg))->hdr.hdr_additional_const_len)
#define __QDF_IPA_EP_CFG_MODE(cfg)	\
	(((struct ipa_ep_cfg *)(cfg))->mode.mode)
#define __QDF_IPA_EP_CFG_HDR_LITTLE_ENDIAN(cfg)	\
	(((struct ipa_ep_cfg *)(cfg))->hdr_ext.hdr_little_endian)

/**
 * __qdf_ipa_wdi_pipe_setup_info_t - WDI TX/Rx configuration
 */
typedef struct ipa_wdi_pipe_setup_info  __qdf_ipa_wdi_pipe_setup_info_t;

#define __QDF_IPA_WDI_SETUP_INFO_EP_CFG(txrx)	\
	(((struct ipa_wdi_pipe_setup_info *)(txrx))->ipa_ep_cfg)

#define __QDF_IPA_WDI_SETUP_INFO_CLIENT(txrx)	\
	(((struct ipa_wdi_pipe_setup_info *)(txrx))->client)
#define __QDF_IPA_WDI_SETUP_INFO_TRANSFER_RING_BASE_PA(txrx)	\
	(((struct ipa_wdi_pipe_setup_info *)(txrx))->transfer_ring_base_pa)
#define __QDF_IPA_WDI_SETUP_INFO_TRANSFER_RING_SIZE(txrx)	\
	(((struct ipa_wdi_pipe_setup_info *)(txrx))->transfer_ring_size)
#define __QDF_IPA_WDI_SETUP_INFO_TRANSFER_RING_DOORBELL_PA(txrx)	\
	(((struct ipa_wdi_pipe_setup_info *)(txrx))->transfer_ring_doorbell_pa)
#define __QDF_IPA_WDI_SETUP_INFO_IS_TXR_RN_DB_PCIE_ADDR(txrx)  \
	(((struct ipa_wdi_pipe_setup_info *)(txrx))->is_txr_rn_db_pcie_addr)
#define __QDF_IPA_WDI_SETUP_INFO_EVENT_RING_BASE_PA(txrx)	\
	(((struct ipa_wdi_pipe_setup_info *)(txrx))->event_ring_base_pa)
#define __QDF_IPA_WDI_SETUP_INFO_EVENT_RING_SIZE(txrx)	\
	(((struct ipa_wdi_pipe_setup_info *)(txrx))->event_ring_size)
#define __QDF_IPA_WDI_SETUP_INFO_EVENT_RING_DOORBELL_PA(txrx)	\
	(((struct ipa_wdi_pipe_setup_info *)(txrx))->event_ring_doorbell_pa)
#define __QDF_IPA_WDI_SETUP_INFO_IS_EVT_RN_DB_PCIE_ADDR(txrx) \
	(((struct ipa_wdi_pipe_setup_info *)(txrx))->is_evt_rn_db_pcie_addr)
#define __QDF_IPA_WDI_SETUP_INFO_NUM_PKT_BUFFERS(txrx)	\
	(((struct ipa_wdi_pipe_setup_info *)(txrx))->num_pkt_buffers)
#define __QDF_IPA_WDI_SETUP_INFO_PKT_OFFSET(txrx)	\
	(((struct ipa_wdi_pipe_setup_info *)(txrx))->pkt_offset)
#define __QDF_IPA_WDI_SETUP_INFO_DESC_FORMAT_TEMPLATE(txrx)	\
	(((struct ipa_wdi_pipe_setup_info *)(txrx))->desc_format_template)

/**
 * __qdf_ipa_wdi_pipe_setup_info_smmu_t - WDI TX/Rx configuration
 */
typedef struct ipa_wdi_pipe_setup_info_smmu __qdf_ipa_wdi_pipe_setup_info_smmu_t;

#define __QDF_IPA_WDI_SETUP_INFO_SMMU_EP_CFG(txrx)	\
	(((struct ipa_wdi_pipe_setup_info_smmu *)(txrx))->ipa_ep_cfg)

#define __QDF_IPA_WDI_SETUP_INFO_SMMU_CLIENT(txrx)	\
	(((struct ipa_wdi_pipe_setup_info_smmu *)(txrx))->client)
#define __QDF_IPA_WDI_SETUP_INFO_SMMU_TRANSFER_RING_BASE(txrx)	\
	(((struct ipa_wdi_pipe_setup_info_smmu *)(txrx))->transfer_ring_base)
#define __QDF_IPA_WDI_SETUP_INFO_SMMU_TRANSFER_RING_SIZE(txrx)	\
	(((struct ipa_wdi_pipe_setup_info_smmu *)(txrx))->transfer_ring_size)
#define __QDF_IPA_WDI_SETUP_INFO_SMMU_TRANSFER_RING_DOORBELL_PA(txrx)	\
	(((struct ipa_wdi_pipe_setup_info_smmu *)(txrx))->transfer_ring_doorbell_pa)
#define __QDF_IPA_WDI_SETUP_INFO_SMMU_IS_TXR_RN_DB_PCIE_ADDR(txrx)  \
	(((struct ipa_wdi_pipe_setup_info_smmu *) \
	  (txrx))->is_txr_rn_db_pcie_addr)
#define __QDF_IPA_WDI_SETUP_INFO_SMMU_EVENT_RING_BASE(txrx)	\
	(((struct ipa_wdi_pipe_setup_info_smmu *)(txrx))->event_ring_base)
#define __QDF_IPA_WDI_SETUP_INFO_SMMU_EVENT_RING_SIZE(txrx)	\
	(((struct ipa_wdi_pipe_setup_info_smmu *)(txrx))->event_ring_size)
#define __QDF_IPA_WDI_SETUP_INFO_SMMU_EVENT_RING_DOORBELL_PA(txrx)	\
	(((struct ipa_wdi_pipe_setup_info_smmu *)(txrx))->event_ring_doorbell_pa)
#define __QDF_IPA_WDI_SETUP_INFO_SMMU_IS_EVT_RN_DB_PCIE_ADDR(txrx) \
	(((struct ipa_wdi_pipe_setup_info_smmu *) \
	  (txrx))->is_evt_rn_db_pcie_addr)
#define __QDF_IPA_WDI_SETUP_INFO_SMMU_NUM_PKT_BUFFERS(txrx)	\
	(((struct ipa_wdi_pipe_setup_info_smmu *)(txrx))->num_pkt_buffers)
#define __QDF_IPA_WDI_SETUP_INFO_SMMU_PKT_OFFSET(txrx)	\
	(((struct ipa_wdi_pipe_setup_info_smmu *)(txrx))->pkt_offset)
#define __QDF_IPA_WDI_SETUP_INFO_SMMU_DESC_FORMAT_TEMPLATE(txrx)	\
	(((struct ipa_wdi_pipe_setup_info_smmu *)(txrx))->desc_format_template)

/**
 * __qdf_ipa_wdi_conn_in_params_t - information provided by
 *		uC offload client
 */
typedef struct ipa_wdi_conn_in_params  __qdf_ipa_wdi_conn_in_params_t;

#define __QDF_IPA_WDI_CONN_IN_PARAMS_NOTIFY(pipe_in)	\
	(((struct ipa_wdi_conn_in_params *)(pipe_in))->notify)
#define __QDF_IPA_WDI_CONN_IN_PARAMS_PRIV(pipe_in)	\
	(((struct ipa_wdi_conn_in_params *)(pipe_in))->priv)
#define __QDF_IPA_WDI_CONN_IN_PARAMS_SMMU_ENABLED(pipe_in)	\
	(((struct ipa_wdi_conn_in_params *)(pipe_in))->is_smmu_enabled)
#define __QDF_IPA_WDI_CONN_IN_PARAMS_NUM_SYS_PIPE_NEEDED(pipe_in)	\
	(((struct ipa_wdi_conn_in_params *)(pipe_in))->num_sys_pipe_needed)
#define __QDF_IPA_WDI_CONN_IN_PARAMS_SYS_IN(in)	\
	(((struct ipa_wdi_conn_in_params *)(pipe_in))->sys_in)
#define __QDF_IPA_WDI_CONN_IN_PARAMS_TX(pipe_in)	\
	(((struct ipa_wdi_conn_in_params *)(pipe_in))->u_tx.tx)
#define __QDF_IPA_WDI_CONN_IN_PARAMS_TX_SMMU(pipe_in)	\
	(((struct ipa_wdi_conn_in_params *)(pipe_in))->u_tx.tx_smmu)
#ifdef IPA_WDI3_TX_TWO_PIPES
#define __QDF_IPA_WDI_CONN_IN_PARAMS_IS_TX1_USED(pipe_in)	\
	(((struct ipa_wdi_conn_in_params *)(pipe_in))->is_tx1_used)
#define __QDF_IPA_WDI_CONN_IN_PARAMS_TX_ALT_PIPE(pipe_in)	\
	(((struct ipa_wdi_conn_in_params *)(pipe_in))->u_tx1.tx)
#define __QDF_IPA_WDI_CONN_IN_PARAMS_TX_ALT_PIPE_SMMU(pipe_in)	\
	(((struct ipa_wdi_conn_in_params *)(pipe_in))->u_tx1.tx_smmu)
#endif
#define __QDF_IPA_WDI_CONN_IN_PARAMS_RX(pipe_in)	\
	(((struct ipa_wdi_conn_in_params *)(pipe_in))->u_rx.rx)
#define __QDF_IPA_WDI_CONN_IN_PARAMS_RX_SMMU(pipe_in)	\
	(((struct ipa_wdi_conn_in_params *)(pipe_in))->u_rx.rx_smmu)

/**
 * __qdf_ipa_wdi_conn_out_params_t - information provided
 *				to WLAN druver
 */
typedef struct ipa_wdi_conn_out_params  __qdf_ipa_wdi_conn_out_params_t;

#define __QDF_IPA_WDI_CONN_OUT_PARAMS_TX_UC_DB_PA(pipe_out)	\
	(((struct ipa_wdi_conn_out_params *)(pipe_out))->tx_uc_db_pa)
#ifdef IPA_WDI3_TX_TWO_PIPES
#define __QDF_IPA_WDI_CONN_OUT_PARAMS_TX_UC_ALT_DB_PA(pipe_out)	\
	(((struct ipa_wdi_conn_out_params *)(pipe_out))->tx1_uc_db_pa)
#endif
#define __QDF_IPA_WDI_CONN_OUT_PARAMS_RX_UC_DB_PA(pipe_out)	\
	(((struct ipa_wdi_conn_out_params *)(pipe_out))->rx_uc_db_pa)
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0))
#define __QDF_IPA_WDI_CONN_OUT_PARAMS_IS_DB_DDR_MAPPED(pipe_out)	\
	(((struct ipa_wdi_conn_out_params *)(pipe_out))->is_ddr_mapped)
#else
#define __QDF_IPA_WDI_CONN_OUT_PARAMS_IS_DB_DDR_MAPPED(pipe_out) false
#endif

/**
 * __qdf_ipa_wdi_perf_profile_t - To set BandWidth profile
 */
typedef struct ipa_wdi_perf_profile  __qdf_ipa_wdi_perf_profile_t;

#define __QDF_IPA_WDI_PERF_PROFILE_CLIENT(profile)	\
	(((struct ipa_wdi_perf_profile *)(profile))->client)
#define __QDF_IPA_WDI_PERF_PROFILE_MAX_SUPPORTED_BW_MBPS(profile)	\
	(((struct ipa_wdi_perf_profile *)(profile))->max_supported_bw_mbps)

/**
 * __qdf_ipa_wdi_init - Client should call this function to
 * init WDI IPA offload data path
 *
 * Note: Should not be called from atomic context and only
 * after checking IPA readiness using ipa_register_ipa_ready_cb()
 *
 * @Return 0 on success, negative on failure
 */
static inline int __qdf_ipa_wdi_init(struct ipa_wdi_init_in_params *in,
		 struct ipa_wdi_init_out_params *out)
{
	return ipa_wdi_init(in, out);
}

/**
 * __qdf_ipa_wdi_cleanup - Client should call this function to
 * clean up WDI IPA offload data path
 *
 * @Return 0 on success, negative on failure
 */
static inline int __qdf_ipa_wdi_cleanup(void)
{
	return ipa_wdi_cleanup();
}

/**
 * __qdf_ipa_wdi_reg_intf - Client should call this function to
 * init WDI IPA offload data path
 *
 * Note: Should not be called from atomic context and only
 * after checking IPA readiness using ipa_register_ipa_ready_cb()
 *
 * @Return 0 on success, negative on failure
 */
static inline int __qdf_ipa_wdi_reg_intf(
	struct ipa_wdi_reg_intf_in_params *in)
{
	return ipa_wdi_reg_intf(in);
}

/**
 * __qdf_ipa_wdi_dereg_intf - Client Driver should call this
 * function to deregister before unload and after disconnect
 *
 * @Return 0 on success, negative on failure
 */
static inline int __qdf_ipa_wdi_dereg_intf(const char *netdev_name)
{
	return ipa_wdi_dereg_intf(netdev_name);
}

/**
 * __qdf_ipa_wdi_conn_pipes - Client should call this
 * function to connect pipes
 *
 * @in:	[in] input parameters from client
 * @out: [out] output params to client
 *
 * Note: Should not be called from atomic context and only
 * after checking IPA readiness using ipa_register_ipa_ready_cb()
 *
 * @Return 0 on success, negative on failure
 */
static inline int __qdf_ipa_wdi_conn_pipes(struct ipa_wdi_conn_in_params *in,
			struct ipa_wdi_conn_out_params *out)
{
	return ipa_wdi_conn_pipes(in, out);
}

/**
 * __qdf_ipa_wdi_disconn_pipes() - Client should call this
 *		function to disconnect pipes
 *
 * Note: Should not be called from atomic context
 *
 * Returns: 0 on success, negative on failure
 */
static inline int __qdf_ipa_wdi_disconn_pipes(void)
{
	return ipa_wdi_disconn_pipes();
}

/**
 * __qdf_ipa_wdi_enable_pipes() - Client should call this
 *		function to enable IPA offload data path
 *
 * Note: Should not be called from atomic context
 *
 * Returns: 0 on success, negative on failure
 */
static inline int __qdf_ipa_wdi_enable_pipes(void)
{
	return ipa_wdi_enable_pipes();
}

/**
 * __qdf_ipa_wdi_disable_pipes() - Client should call this
 *		function to disable IPA offload data path
 *
 * Note: Should not be called from atomic context
 *
 * Returns: 0 on success, negative on failure
 */
static inline int __qdf_ipa_wdi_disable_pipes(void)
{
	return ipa_wdi_disable_pipes();
}

/**
 * __qdf_ipa_wdi_set_perf_profile() - Client should call this function to
 *		set IPA clock bandwidth based on data rates
 *
 * @profile: [in] BandWidth profile to use
 *
 * Returns: 0 on success, negative on failure
 */
static inline int __qdf_ipa_wdi_set_perf_profile(
			struct ipa_wdi_perf_profile *profile)
{
	return ipa_wdi_set_perf_profile(profile);
}

/**
 * __qdf_ipa_wdi_create_smmu_mapping() - Client should call this function to
 *		create smmu mapping
 *
 * @num_buffers: [in] number of buffers
 * @info: [in] wdi buffer info
 *
 * Returns: 0 on success, negative on failure
 */
static inline int __qdf_ipa_wdi_create_smmu_mapping(u32 num_buffers,
		struct ipa_wdi_buffer_info *info)
{
	return ipa_wdi_create_smmu_mapping(num_buffers, info);
}

/**
 * __qdf_ipa_wdi_release_smmu_mapping() - Client should call this function to
 *		release smmu mapping
 *
 * @num_buffers: [in] number of buffers
 * @info: [in] wdi buffer info
 *
 * Returns: 0 on success, negative on failure
 */
static inline int __qdf_ipa_wdi_release_smmu_mapping(u32 num_buffers,
		struct ipa_wdi_buffer_info *info)
{
	return ipa_wdi_release_smmu_mapping(num_buffers, info);
}

#ifdef WDI3_STATS_UPDATE
/**
 * __qdf_ipa_wdi_wlan_stats() - Client should call this function to
 *		send Tx byte counts to IPA driver
 * @tx_stats: number of Tx bytes on STA and SAP
 *
 * Returns: 0 on success, negative on failure
 */
static inline int __qdf_ipa_wdi_wlan_stats(struct ipa_wdi_tx_info *tx_stats)
{
	return ipa_wdi_sw_stats(tx_stats);
}

/**
 * ipa_uc_bw_monitor() - start/stop uc bw monitoring
 * @bw_info: set bw info levels to monitor
 *
 * Returns: 0 on success, negative on failure
 */
static inline int __qdf_ipa_uc_bw_monitor(struct ipa_wdi_bw_info *bw_info)
{
	return ipa_uc_bw_monitor(bw_info);
}
#endif
#else /* CONFIG_IPA_WDI_UNIFIED_API */

/**
 * __qdf_ipa_wdi_hdr_info_t - Header to install on IPA HW
 */
typedef struct ipa_wdi3_hdr_info  __qdf_ipa_wdi_hdr_info_t;

#define __QDF_IPA_WDI_HDR_INFO_HDR(hdr_info)	\
	(((struct ipa_wdi3_hdr_info *)(hdr_info))->hdr)
#define __QDF_IPA_WDI_HDR_INFO_HDR_LEN(hdr_info)	\
	(((struct ipa_wdi3_hdr_info *)(hdr_info))->hdr_len)
#define __QDF_IPA_WDI_HDR_INFO_DST_MAC_ADDR_OFFSET(hdr_info)	\
	(((struct ipa_wdi3_hdr_info *)(hdr_info))->dst_mac_addr_offset)
#define __QDF_IPA_WDI_HDR_INFO_HDR_TYPE(hdr_info)	\
	(((struct ipa_wdi3_hdr_info *)(hdr_info))->hdr_type)

/**
 * __qdf_ipa_wdi_reg_intf_in_params_t - parameters for uC offload
 *	interface registration
 */
typedef struct ipa_wdi3_reg_intf_in_params  __qdf_ipa_wdi_reg_intf_in_params_t;

#define __QDF_IPA_WDI_REG_INTF_IN_PARAMS_NETDEV_NAME(in)	\
	(((struct ipa_wdi3_reg_intf_in_params *)(in))->netdev_name)
#define __QDF_IPA_WDI_REG_INTF_IN_PARAMS_HDR_INFO(in)	\
	(((struct ipa_wdi3_reg_intf_in_params *)(in))->hdr_info)
#define __QDF_IPA_WDI_REG_INTF_IN_PARAMS_IS_META_DATA_VALID(in)	\
	(((struct ipa_wdi3_reg_intf_in_params *)(in))->is_meta_data_valid)
#define __QDF_IPA_WDI_REG_INTF_IN_PARAMS_META_DATA(in)	\
	(((struct ipa_wdi3_reg_intf_in_params *)(in))->meta_data)
#define __QDF_IPA_WDI_REG_INTF_IN_PARAMS_META_DATA_MASK(in)	\
	(((struct ipa_wdi3_reg_intf_in_params *)(in))->meta_data_mask)

/**
 * __qdf_ipa_wdi_setup_info_t - WDI3 TX/Rx configuration
 */
typedef struct ipa_wdi3_setup_info  __qdf_ipa_wdi_pipe_setup_info_t;

#define __QDF_IPA_WDI_SETUP_INFO_NAT_EN(txrx)	\
	(((struct ipa_wdi3_setup_info *)(txrx))->ipa_ep_cfg.nat.nat_en)
#define __QDF_IPA_WDI_SETUP_INFO_HDR_LEN(txrx)	\
	(((struct ipa_wdi3_setup_info *)(txrx))->ipa_ep_cfg.hdr.hdr_len)
#define __QDF_IPA_WDI_SETUP_INFO_HDR_OFST_METADATA_VALID(txrx)	\
	(((struct ipa_wdi3_setup_info *)(txrx))->ipa_ep_cfg.hdr.hdr_ofst_metadata_valid)
#define __QDF_IPA_WDI_SETUP_INFO_HDR_METADATA_REG_VALID(txrx)	\
	(((struct ipa_wdi3_setup_info *)(txrx))->ipa_ep_cfg.hdr.hdr_metadata_reg_valid)
#define __QDF_IPA_WDI_SETUP_INFO_HDR_OFST_PKT_SIZE_VALID(txrx)	\
	(((struct ipa_wdi3_setup_info *)(txrx))->ipa_ep_cfg.hdr.hdr_ofst_pkt_size_valid)
#define __QDF_IPA_WDI_SETUP_INFO_HDR_OFST_PKT_SIZE(txrx)	\
	(((struct ipa_wdi3_setup_info *)(txrx))->ipa_ep_cfg.hdr.hdr_ofst_pkt_size)
#define __QDF_IPA_WDI_SETUP_INFO_HDR_ADDITIONAL_CONST_LEN(txrx)	\
	(((struct ipa_wdi3_setup_info *)(txrx))->ipa_ep_cfg.hdr.hdr_additional_const_len)
#define __QDF_IPA_WDI_SETUP_INFO_MODE(txrx)	\
	(((struct ipa_wdi3_setup_info *)(txrx))->ipa_ep_cfg.mode.mode)
#define __QDF_IPA_WDI_SETUP_INFO_HDR_LITTLE_ENDIAN(txrx)	\
	(((struct ipa_wdi3_setup_info *)(txrx))->ipa_ep_cfg.hdr_ext.hdr_little_endian)

#define __QDF_IPA_WDI_SETUP_INFO_CLIENT(txrx)	\
	(((struct ipa_wdi3_setup_info *)(txrx))->client)
#define __QDF_IPA_WDI_SETUP_INFO_TRANSFER_RING_BASE_PA(txrx)	\
	(((struct ipa_wdi3_setup_info *)(txrx))->transfer_ring_base_pa)
#define __QDF_IPA_WDI_SETUP_INFO_TRANSFER_RING_SIZE(txrx)	\
	(((struct ipa_wdi3_setup_info *)(txrx))->transfer_ring_size)
#define __QDF_IPA_WDI_SETUP_INFO_TRANSFER_RING_DOORBELL_PA(txrx)	\
	(((struct ipa_wdi3_setup_info *)(txrx))->transfer_ring_doorbell_pa)
#define __QDF_IPA_WDI_SETUP_INFO_EVENT_RING_BASE_PA(txrx)	\
	(((struct ipa_wdi3_setup_info *)(txrx))->event_ring_base_pa)
#define __QDF_IPA_WDI_SETUP_INFO_EVENT_RING_SIZE(txrx)	\
	(((struct ipa_wdi3_setup_info *)(txrx))->event_ring_size)
#define __QDF_IPA_WDI_SETUP_INFO_EVENT_RING_DOORBELL_PA(txrx)	\
	(((struct ipa_wdi3_setup_info *)(txrx))->event_ring_doorbell_pa)
#define __QDF_IPA_WDI_SETUP_INFO_NUM_PKT_BUFFERS(txrx)	\
	(((struct ipa_wdi3_setup_info *)(txrx))->num_pkt_buffers)
#define __QDF_IPA_WDI_SETUP_INFO_PKT_OFFSET(txrx)	\
	(((struct ipa_wdi3_setup_info *)(txrx))->pkt_offset)
#define __QDF_IPA_WDI_SETUP_INFO_DESC_FORMAT_TEMPLATE(txrx)	\
	(((struct ipa_wdi3_setup_info *)(txrx))->desc_format_template)

/**
 * __qdf_ipa_wdi_conn_in_params_t - information provided by
 *		uC offload client
 */
typedef struct ipa_wdi3_conn_in_params  __qdf_ipa_wdi_conn_in_params_t;

#define __QDF_IPA_WDI_CONN_IN_PARAMS_NOTIFY(pipe_in)	\
	(((struct ipa_wdi3_conn_in_params *)(pipe_in))->notify)
#define __QDF_IPA_WDI_CONN_IN_PARAMS_PRIV(pipe_in)	\
	(((struct ipa_wdi3_conn_in_params *)(pipe_in))->priv)
#define __QDF_IPA_WDI_CONN_IN_PARAMS_TX(pipe_in)	\
	(((struct ipa_wdi3_conn_in_params *)(pipe_in))->tx)
#define __QDF_IPA_WDI_CONN_IN_PARAMS_RX(pipe_in)	\
	(((struct ipa_wdi3_conn_in_params *)(pipe_in))->rx)

/**
 * __qdf_ipa_wdi_conn_out_params_t - information provided
 *				to WLAN druver
 */
typedef struct ipa_wdi3_conn_out_params  __qdf_ipa_wdi_conn_out_params_t;

#define __QDF_IPA_WDI_CONN_OUT_PARAMS_TX_UC_DB_PA(pipe_out)	\
	(((struct ipa_wdi3_conn_out_params *)(pipe_out))->tx_uc_db_pa)
#define __QDF_IPA_WDI_CONN_OUT_PARAMS_TX_UC_DB_VA(pipe_out)	\
	(((struct ipa_wdi3_conn_out_params *)(pipe_out))->tx_uc_db_va)
#define __QDF_IPA_WDI_CONN_OUT_PARAMS_RX_UC_DB_PA(pipe_out)	\
	(((struct ipa_wdi3_conn_out_params *)(pipe_out))->rx_uc_db_pa)

/**
 * __qdf_ipa_wdi_perf_profile_t - To set BandWidth profile
 */
typedef struct ipa_wdi3_perf_profile  __qdf_ipa_wdi_perf_profile_t;

#define __QDF_IPA_WDI_PERF_PROFILE_CLIENT(profile)	\
	(((struct ipa_wdi3_perf_profile *)(profile))->client)
#define __QDF_IPA_WDI_PERF_PROFILE_MAX_SUPPORTED_BW_MBPS(profile)	\
	(((struct ipa_wdi3_perf_profile *)(profile))->max_supported_bw_mbps)

/**
 * __qdf_ipa_wdi_reg_intf - Client should call this function to
 * init WDI3 IPA offload data path
 *
 * Note: Should not be called from atomic context and only
 * after checking IPA readiness using ipa_register_ipa_ready_cb()
 *
 * @Return 0 on success, negative on failure
 */
static inline int __qdf_ipa_wdi_reg_intf(
	struct ipa_wdi3_reg_intf_in_params *in)
{
	return ipa_wdi3_reg_intf(in);
}

/**
 * __qdf_ipa_wdi_dereg_intf - Client Driver should call this
 * function to deregister before unload and after disconnect
 *
 * @Return 0 on success, negative on failure
 */
static inline int __qdf_ipa_wdi_dereg_intf(const char *netdev_name)
{
	return ipa_wdi3_dereg_intf(netdev_name);
}

/**
 * __qdf_ipa_wdi_conn_pipes - Client should call this
 * function to connect pipes
 *
 * @in:	[in] input parameters from client
 * @out: [out] output params to client
 *
 * Note: Should not be called from atomic context and only
 * after checking IPA readiness using ipa_register_ipa_ready_cb()
 *
 * @Return 0 on success, negative on failure
 */
static inline int __qdf_ipa_wdi_conn_pipes(struct ipa_wdi3_conn_in_params *in,
					   struct ipa_wdi3_conn_out_params *out)
{
	return ipa_wdi3_conn_pipes(in, out);
}

/**
 * __qdf_ipa_wdi_disconn_pipes() - Client should call this
 *		function to disconnect pipes
 *
 * Note: Should not be called from atomic context
 *
 * Returns: 0 on success, negative on failure
 */
static inline int __qdf_ipa_wdi_disconn_pipes(void)
{
	return ipa_wdi3_disconn_pipes();
}

/**
 * __qdf_ipa_wdi_enable_pipes() - Client should call this
 *		function to enable IPA offload data path
 *
 * Note: Should not be called from atomic context
 *
 * Returns: 0 on success, negative on failure
 */
static inline int __qdf_ipa_wdi_enable_pipes(void)
{
	return ipa_wdi3_enable_pipes();
}

/**
 * __qdf_ipa_wdi_disable_pipes() - Client should call this
 *		function to disable IPA offload data path
 *
 * Note: Should not be called from atomic context
 *
 * Returns: 0 on success, negative on failure
 */
static inline int __qdf_ipa_wdi_disable_pipes(void)
{
	return ipa_wdi3_disable_pipes();
}

/**
 * __qdf_ipa_wdi_set_perf_profile() - Client should call this function to
 *		set IPA clock bandwidth based on data rates
 *
 * @profile: [in] BandWidth profile to use
 *
 * Returns: 0 on success, negative on failure
 */
static inline int __qdf_ipa_wdi_set_perf_profile(
			struct ipa_wdi3_perf_profile *profile)
{
	return ipa_wdi3_set_perf_profile(profile);
}

#endif /* CONFIG_IPA_WDI_UNIFIED_API */

#endif /* IPA_OFFLOAD */
#endif /* I_QDF_IPA_WDI_H */
