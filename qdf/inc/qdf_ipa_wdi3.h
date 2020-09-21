/*
 * Copyright (c) 2017-2020, The Linux Foundation. All rights reserved.
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
 * DOC: qdf_ipa_wdi3.h
 * This file provides OS abstraction for IPA WDI APIs.
 */

#ifndef _QDF_IPA_WDI3_H
#define _QDF_IPA_WDI3_H

#ifdef IPA_OFFLOAD

#include <qdf_ipa.h>
#include <i_qdf_ipa_wdi3.h>

#ifdef CONFIG_IPA_WDI_UNIFIED_API
/**
 * qdf_ipa_wdi_version_t - IPA WDI version
 */
typedef __qdf_ipa_wdi_version_t qdf_ipa_wdi_version_t;

/**
 * qdf_ipa_wdi_init_in_params_t - wdi init input parameters
 */
typedef __qdf_ipa_wdi_init_in_params_t qdf_ipa_wdi_init_in_params_t;

#define QDF_IPA_WDI_INIT_IN_PARAMS_WDI_VERSION(in_params)	\
	__QDF_IPA_WDI_INIT_IN_PARAMS_WDI_VERSION(in_params)
#define QDF_IPA_WDI_INIT_IN_PARAMS_NOTIFY(in_params)	\
	__QDF_IPA_WDI_INIT_IN_PARAMS_NOTIFY(in_params)
#define QDF_IPA_WDI_INIT_IN_PARAMS_PRIV(in_params)	\
	__QDF_IPA_WDI_INIT_IN_PARAMS_PRIV(in_params)
#define QDF_IPA_WDI_INIT_IN_PARAMS_WDI_NOTIFY(in_params)	\
	__QDF_IPA_WDI_INIT_IN_PARAMS_WDI_NOTIFY(in_params)

/**
 * qdf_ipa_wdi_init_out_params_t - wdi init output parameters
 */
typedef __qdf_ipa_wdi_init_out_params_t qdf_ipa_wdi_init_out_params_t;

#define QDF_IPA_WDI_INIT_OUT_PARAMS_IS_UC_READY(out_params)	\
	__QDF_IPA_WDI_INIT_OUT_PARAMS_IS_UC_READY(out_params)
#define QDF_IPA_WDI_INIT_OUT_PARAMS_IS_SMMU_ENABLED(out_params)	\
	__QDF_IPA_WDI_INIT_OUT_PARAMS_IS_SMMU_ENABLED(out_params)

/**
 * qdf_ipa_wdi_pipe_setup_info_smmu_t - WDI TX/Rx configuration
 */
typedef __qdf_ipa_wdi_pipe_setup_info_smmu_t qdf_ipa_wdi_pipe_setup_info_smmu_t;

#define QDF_IPA_WDI_SETUP_INFO_SMMU_EP_CFG(txrx)	\
	__QDF_IPA_WDI_SETUP_INFO_SMMU_EP_CFG(txrx)

#define QDF_IPA_WDI_SETUP_INFO_SMMU_CLIENT(txrx)	\
	__QDF_IPA_WDI_SETUP_INFO_SMMU_CLIENT(txrx)
#define QDF_IPA_WDI_SETUP_INFO_SMMU_TRANSFER_RING_BASE(txrx)	\
	__QDF_IPA_WDI_SETUP_INFO_SMMU_TRANSFER_RING_BASE(txrx)
#define QDF_IPA_WDI_SETUP_INFO_SMMU_TRANSFER_RING_SIZE(txrx)	\
	__QDF_IPA_WDI_SETUP_INFO_SMMU_TRANSFER_RING_SIZE(txrx)
#define QDF_IPA_WDI_SETUP_INFO_SMMU_TRANSFER_RING_DOORBELL_PA(txrx)	\
	__QDF_IPA_WDI_SETUP_INFO_SMMU_TRANSFER_RING_DOORBELL_PA(txrx)
#define QDF_IPA_WDI_SETUP_INFO_SMMU_IS_TXR_RN_DB_PCIE_ADDR(txrx)	\
	__QDF_IPA_WDI_SETUP_INFO_SMMU_IS_TXR_RN_DB_PCIE_ADDR(txrx)
#define QDF_IPA_WDI_SETUP_INFO_SMMU_EVENT_RING_BASE(txrx)	\
	__QDF_IPA_WDI_SETUP_INFO_SMMU_EVENT_RING_BASE(txrx)
#define QDF_IPA_WDI_SETUP_INFO_SMMU_EVENT_RING_SIZE(txrx)	\
	__QDF_IPA_WDI_SETUP_INFO_SMMU_EVENT_RING_SIZE(txrx)
#define QDF_IPA_WDI_SETUP_INFO_SMMU_EVENT_RING_DOORBELL_PA(txrx)	\
	__QDF_IPA_WDI_SETUP_INFO_SMMU_EVENT_RING_DOORBELL_PA(txrx)
#define QDF_IPA_WDI_SETUP_INFO_SMMU_IS_EVT_RN_DB_PCIE_ADDR(txrx)	\
	__QDF_IPA_WDI_SETUP_INFO_SMMU_IS_EVT_RN_DB_PCIE_ADDR(txrx)
#define QDF_IPA_WDI_SETUP_INFO_SMMU_NUM_PKT_BUFFERS(txrx)	\
	__QDF_IPA_WDI_SETUP_INFO_SMMU_NUM_PKT_BUFFERS(txrx)
#define QDF_IPA_WDI_SETUP_INFO_SMMU_PKT_OFFSET(txrx)	\
	__QDF_IPA_WDI_SETUP_INFO_SMMU_PKT_OFFSET(txrx)
#define QDF_IPA_WDI_SETUP_INFO_SMMU_DESC_FORMAT_TEMPLATE(txrx)	\
	__QDF_IPA_WDI_SETUP_INFO_SMMU_DESC_FORMAT_TEMPLATE(txrx)

typedef __qdf_ipa_ep_cfg_t qdf_ipa_ep_cfg_t;

#define QDF_IPA_EP_CFG_NAT_EN(cfg)	\
	__QDF_IPA_EP_CFG_NAT_EN(cfg)
#define QDF_IPA_EP_CFG_HDR_LEN(cfg)	\
	__QDF_IPA_EP_CFG_HDR_LEN(cfg)
#define QDF_IPA_EP_CFG_HDR_OFST_METADATA_VALID(cfg)	\
	__QDF_IPA_EP_CFG_HDR_OFST_METADATA_VALID(cfg)
#define QDF_IPA_EP_CFG_HDR_METADATA_REG_VALID(cfg)	\
	__QDF_IPA_EP_CFG_HDR_METADATA_REG_VALID(cfg)
#define QDF_IPA_EP_CFG_HDR_OFST_PKT_SIZE_VALID(cfg)	\
	__QDF_IPA_EP_CFG_HDR_OFST_PKT_SIZE_VALID(cfg)
#define QDF_IPA_EP_CFG_HDR_OFST_PKT_SIZE(cfg)	\
	__QDF_IPA_EP_CFG_HDR_OFST_PKT_SIZE(cfg)
#define QDF_IPA_EP_CFG_HDR_ADDITIONAL_CONST_LEN(cfg)	\
	__QDF_IPA_EP_CFG_HDR_ADDITIONAL_CONST_LEN(cfg)
#define QDF_IPA_EP_CFG_MODE(cfg)	\
	__QDF_IPA_EP_CFG_MODE(cfg)
#define QDF_IPA_EP_CFG_HDR_LITTLE_ENDIAN(cfg)	\
	__QDF_IPA_EP_CFG_HDR_LITTLE_ENDIAN(cfg)

/**
 * qdf_ipa_wdi_init - Client should call this function to
 * init WDI IPA offload data path
 *
 * Note: Should not be called from atomic context and only
 * after checking IPA readiness using ipa_register_ipa_ready_cb()
 *
 * @Return 0 on success, negative on failure
 */
static inline int qdf_ipa_wdi_init(qdf_ipa_wdi_init_in_params_t *in,
		 qdf_ipa_wdi_init_out_params_t *out)
{
	return __qdf_ipa_wdi_init(in, out);
}

/**
 * qdf_ipa_wdi_cleanup - Client should call this function to
 * clean up WDI IPA offload data path
 *
 * @Return 0 on success, negative on failure
 */
static inline int qdf_ipa_wdi_cleanup(void)
{
	return __qdf_ipa_wdi_cleanup();
}
#endif /* CONFIG_IPA_WDI_UNIFIED_API */

/**
 * qdf_ipa_wdi_hdr_info_t - Header to install on IPA HW
 */
typedef __qdf_ipa_wdi_hdr_info_t qdf_ipa_wdi_hdr_info_t;

#define QDF_IPA_WDI_HDR_INFO_HDR(hdr_info)	\
	__QDF_IPA_WDI_HDR_INFO_HDR(hdr_info)
#define QDF_IPA_WDI_HDR_INFO_HDR_LEN(hdr_info)	\
	__QDF_IPA_WDI_HDR_INFO_HDR_LEN(hdr_info)
#define QDF_IPA_WDI_HDR_INFO_DST_MAC_ADDR_OFFSET(hdr_info)	\
	__QDF_IPA_WDI_HDR_INFO_DST_MAC_ADDR_OFFSET(hdr_info)
#define QDF_IPA_WDI_HDR_INFO_HDR_TYPE(hdr_info)	\
	__QDF_IPA_WDI_HDR_INFO_HDR_TYPE(hdr_info)

/**
 * qdf_ipa_wdi_reg_intf_in_params_t - parameters for uC offload
 *	interface registration
 */
typedef __qdf_ipa_wdi_reg_intf_in_params_t qdf_ipa_wdi_reg_intf_in_params_t;

#define QDF_IPA_WDI_REG_INTF_IN_PARAMS_NETDEV_NAME(in)	\
	__QDF_IPA_WDI_REG_INTF_IN_PARAMS_NETDEV_NAME(in)
#define QDF_IPA_WDI_REG_INTF_IN_PARAMS_HDR_INFO(in)	\
	__QDF_IPA_WDI_REG_INTF_IN_PARAMS_HDR_INFO(in)
#define QDF_IPA_WDI_REG_INTF_IN_PARAMS_ALT_DST_PIPE(in)	\
	__QDF_IPA_WDI_REG_INTF_IN_PARAMS_ALT_DST_PIPE(in)
#define QDF_IPA_WDI_REG_INTF_IN_PARAMS_IS_META_DATA_VALID(in)	\
	__QDF_IPA_WDI_REG_INTF_IN_PARAMS_IS_META_DATA_VALID(in)
#define QDF_IPA_WDI_REG_INTF_IN_PARAMS_META_DATA(in)	\
	__QDF_IPA_WDI_REG_INTF_IN_PARAMS_META_DATA(in)
#define QDF_IPA_WDI_REG_INTF_IN_PARAMS_META_DATA_MASK(in)	\
	__QDF_IPA_WDI_REG_INTF_IN_PARAMS_META_DATA_MASK(in)

/**
 * qdf_ipa_wdi_pipe_setup_info_t - WDI TX/Rx configuration
 */
typedef __qdf_ipa_wdi_pipe_setup_info_t qdf_ipa_wdi_pipe_setup_info_t;

#define QDF_IPA_WDI_SETUP_INFO_EP_CFG(txrx)	\
	__QDF_IPA_WDI_SETUP_INFO_EP_CFG(txrx)

#define QDF_IPA_WDI_SETUP_INFO_NAT_EN(txrx)	\
	__QDF_IPA_WDI_SETUP_INFO_NAT_EN(txrx)
#define QDF_IPA_WDI_SETUP_INFO_HDR_LEN(txrx)	\
	__QDF_IPA_WDI_SETUP_INFO_HDR_LEN(txrx)
#define QDF_IPA_WDI_SETUP_INFO_HDR_OFST_METADATA_VALID(txrx)	\
	__QDF_IPA_WDI_SETUP_INFO_HDR_OFST_METADATA_VALID(txrx)
#define QDF_IPA_WDI_SETUP_INFO_HDR_METADATA_REG_VALID(txrx)	\
	__QDF_IPA_WDI_SETUP_INFO_HDR_METADATA_REG_VALID(txrx)
#define QDF_IPA_WDI_SETUP_INFO_HDR_OFST_PKT_SIZE_VALID(txrx)	\
	__QDF_IPA_WDI_SETUP_INFO_HDR_OFST_PKT_SIZE_VALID(txrx)
#define QDF_IPA_WDI_SETUP_INFO_HDR_OFST_PKT_SIZE(txrx)	\
	__QDF_IPA_WDI_SETUP_INFO_HDR_OFST_PKT_SIZE(txrx)
#define QDF_IPA_WDI_SETUP_INFO_HDR_ADDITIONAL_CONST_LEN(txrx)	\
	__QDF_IPA_WDI_SETUP_INFO_HDR_ADDITIONAL_CONST_LEN(txrx)
#define QDF_IPA_WDI_SETUP_INFO_MODE(txrx)	\
	__QDF_IPA_WDI_SETUP_INFO_MODE(txrx)
#define QDF_IPA_WDI_SETUP_INFO_HDR_LITTLE_ENDIAN(txrx)	\
	__QDF_IPA_WDI_SETUP_INFO_HDR_LITTLE_ENDIAN(txrx)

#define QDF_IPA_WDI_SETUP_INFO_CLIENT(txrx)	\
	__QDF_IPA_WDI_SETUP_INFO_CLIENT(txrx)
#define QDF_IPA_WDI_SETUP_INFO_TRANSFER_RING_BASE_PA(txrx)	\
	__QDF_IPA_WDI_SETUP_INFO_TRANSFER_RING_BASE_PA(txrx)
#define QDF_IPA_WDI_SETUP_INFO_TRANSFER_RING_SIZE(txrx)	\
	__QDF_IPA_WDI_SETUP_INFO_TRANSFER_RING_SIZE(txrx)
#define QDF_IPA_WDI_SETUP_INFO_TRANSFER_RING_DOORBELL_PA(txrx)	\
	__QDF_IPA_WDI_SETUP_INFO_TRANSFER_RING_DOORBELL_PA(txrx)
#define QDF_IPA_WDI_SETUP_INFO_IS_TXR_RN_DB_PCIE_ADDR(txrx)	\
	__QDF_IPA_WDI_SETUP_INFO_IS_TXR_RN_DB_PCIE_ADDR(txrx)
#define QDF_IPA_WDI_SETUP_INFO_EVENT_RING_BASE_PA(txrx)	\
	__QDF_IPA_WDI_SETUP_INFO_EVENT_RING_BASE_PA(txrx)
#define QDF_IPA_WDI_SETUP_INFO_EVENT_RING_SIZE(txrx)	\
	__QDF_IPA_WDI_SETUP_INFO_EVENT_RING_SIZE(txrx)
#define QDF_IPA_WDI_SETUP_INFO_EVENT_RING_DOORBELL_PA(txrx)	\
	__QDF_IPA_WDI_SETUP_INFO_EVENT_RING_DOORBELL_PA(txrx)
#define QDF_IPA_WDI_SETUP_INFO_IS_EVT_RN_DB_PCIE_ADDR(txrx)	\
	__QDF_IPA_WDI_SETUP_INFO_IS_EVT_RN_DB_PCIE_ADDR(txrx)
#define QDF_IPA_WDI_SETUP_INFO_NUM_PKT_BUFFERS(txrx)	\
	__QDF_IPA_WDI_SETUP_INFO_NUM_PKT_BUFFERS(txrx)
#define QDF_IPA_WDI_SETUP_INFO_PKT_OFFSET(txrx)	\
	__QDF_IPA_WDI_SETUP_INFO_PKT_OFFSET(txrx)
#define QDF_IPA_WDI_SETUP_INFO_DESC_FORMAT_TEMPLATE(txrx)	\
	__QDF_IPA_WDI_SETUP_INFO_DESC_FORMAT_TEMPLATE(txrx)

/**
 * qdf_ipa_wdi_conn_in_params_t - information provided by
 *		uC offload client
 */
typedef __qdf_ipa_wdi_conn_in_params_t qdf_ipa_wdi_conn_in_params_t;

#define QDF_IPA_WDI_CONN_IN_PARAMS_NOTIFY(pipe_in)	\
	__QDF_IPA_WDI_CONN_IN_PARAMS_NOTIFY(pipe_in)
#define QDF_IPA_WDI_CONN_IN_PARAMS_PRIV(pipe_in)	\
	__QDF_IPA_WDI_CONN_IN_PARAMS_PRIV(pipe_in)
#define QDF_IPA_WDI_CONN_IN_PARAMS_SMMU_ENABLED(pipe_in)	\
	__QDF_IPA_WDI_CONN_IN_PARAMS_SMMU_ENABLED(pipe_in)
#define QDF_IPA_WDI_CONN_IN_PARAMS_NUM_SYS_PIPE_NEEDED(pipe_in)	\
	__QDF_IPA_WDI_CONN_IN_PARAMS_NUM_SYS_PIPE_NEEDED(pipe_in)
#define QDF_IPA_WDI_CONN_IN_PARAMS_SYS_IN(in)	\
	__QDF_IPA_WDI_CONN_IN_PARAMS_SYS_IN(in)
#define QDF_IPA_WDI_CONN_IN_PARAMS_TX(pipe_in)	\
	__QDF_IPA_WDI_CONN_IN_PARAMS_TX(pipe_in)
#define QDF_IPA_WDI_CONN_IN_PARAMS_TX_SMMU(pipe_in)	\
	__QDF_IPA_WDI_CONN_IN_PARAMS_TX_SMMU(pipe_in)
#define QDF_IPA_WDI_CONN_IN_PARAMS_RX(pipe_in)	\
	__QDF_IPA_WDI_CONN_IN_PARAMS_RX(pipe_in)
#define QDF_IPA_WDI_CONN_IN_PARAMS_RX_SMMU(pipe_in)	\
	__QDF_IPA_WDI_CONN_IN_PARAMS_RX_SMMU(pipe_in)

/**
 * qdf_ipa_wdi_conn_out_params_t - information provided
 *				to WLAN druver
 */
typedef __qdf_ipa_wdi_conn_out_params_t qdf_ipa_wdi_conn_out_params_t;

#define QDF_IPA_WDI_CONN_OUT_PARAMS_TX_UC_DB_PA(pipe_out)	\
	__QDF_IPA_WDI_CONN_OUT_PARAMS_TX_UC_DB_PA(pipe_out)
#define QDF_IPA_WDI_CONN_OUT_PARAMS_TX_UC_DB_VA(pipe_out)	\
	__QDF_IPA_WDI_CONN_OUT_PARAMS_TX_UC_DB_VA(pipe_out)
#define QDF_IPA_WDI_CONN_OUT_PARAMS_RX_UC_DB_PA(pipe_out)	\
	__QDF_IPA_WDI_CONN_OUT_PARAMS_RX_UC_DB_PA(pipe_out)
#define QDF_IPA_WDI_CONN_OUT_PARAMS_IS_DB_DDR_MAPPED(pipe_out)	\
	__QDF_IPA_WDI_CONN_OUT_PARAMS_IS_DB_DDR_MAPPED(pipe_out)

/**
 * qdf_ipa_wdi_perf_profile_t - To set BandWidth profile
 */
typedef __qdf_ipa_wdi_perf_profile_t qdf_ipa_wdi_perf_profile_t;

#define QDF_IPA_WDI_PERF_PROFILE_CLIENT(profile)	\
	__QDF_IPA_WDI_PERF_PROFILE_CLIENT(profile)
#define QDF_IPA_WDI_PERF_PROFILE_MAX_SUPPORTED_BW_MBPS(profile)	\
	__QDF_IPA_WDI_PERF_PROFILE_MAX_SUPPORTED_BW_MBPS(profile)

/**
 * qdf_ipa_wdi_reg_intf - Client should call this function to
 * init WDI IPA offload data path
 *
 * Note: Should not be called from atomic context and only
 * after checking IPA readiness using ipa_register_ipa_ready_cb()
 *
 * @Return 0 on success, negative on failure
 */
static inline int qdf_ipa_wdi_reg_intf(
	qdf_ipa_wdi_reg_intf_in_params_t *in)
{
	return __qdf_ipa_wdi_reg_intf(in);
}

/**
 * qdf_ipa_wdi_dereg_intf - Client Driver should call this
 * function to deregister before unload and after disconnect
 *
 * @Return 0 on success, negative on failure
 */
static inline int qdf_ipa_wdi_dereg_intf(const char *netdev_name)
{
	return __qdf_ipa_wdi_dereg_intf(netdev_name);
}

/**
 * qdf_ipa_wdi_conn_pipes - Client should call this
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
static inline int qdf_ipa_wdi_conn_pipes(qdf_ipa_wdi_conn_in_params_t *in,
			qdf_ipa_wdi_conn_out_params_t *out)
{
	return __qdf_ipa_wdi_conn_pipes(in, out);
}

/**
 * qdf_ipa_wdi_disconn_pipes() - Client should call this
 *		function to disconnect pipes
 *
 * Note: Should not be called from atomic context
 *
 * Returns: 0 on success, negative on failure
 */
static inline int qdf_ipa_wdi_disconn_pipes(void)
{
	return __qdf_ipa_wdi_disconn_pipes();
}

/**
 * qdf_ipa_wdi_enable_pipes() - Client should call this
 *		function to enable IPA offload data path
 *
 * Note: Should not be called from atomic context
 *
 * Returns: 0 on success, negative on failure
 */
static inline int qdf_ipa_wdi_enable_pipes(void)
{
	return __qdf_ipa_wdi_enable_pipes();
}

/**
 * qdf_ipa_wdi_disable_pipes() - Client should call this
 *		function to disable IPA offload data path
 *
 * Note: Should not be called from atomic context
 *
 * Returns: 0 on success, negative on failure
 */
static inline int qdf_ipa_wdi_disable_pipes(void)
{
	return __qdf_ipa_wdi_disable_pipes();
}

/**
 * qdf_ipa_wdi_set_perf_profile() - Client should call this function to
 *		set IPA clock bandwidth based on data rates
 *
 * @profile: [in] BandWidth profile to use
 *
 * Returns: 0 on success, negative on failure
 */
static inline int qdf_ipa_wdi_set_perf_profile(
			qdf_ipa_wdi_perf_profile_t *profile)
{
	return __qdf_ipa_wdi_set_perf_profile(profile);
}

/**
 * qdf_ipa_wdi_create_smmu_mapping() - Client should call this function to
 *		create smmu mapping
 *
 * @num_buffers: [in] number of buffers
 * @info: [in] wdi buffer info
 *
 * Returns: 0 on success, negative on failure
 */
static inline int qdf_ipa_wdi_create_smmu_mapping(uint32_t num_buffers,
		qdf_ipa_wdi_buffer_info_t *info)
{
	return __qdf_ipa_wdi_create_smmu_mapping(num_buffers, info);
}

/**
 * qdf_ipa_wdi_release_smmu_mapping() - Client should call this function to
 *		release smmu mapping
 *
 * @num_buffers: [in] number of buffers
 * @info: [in] wdi buffer info
 *
 * Returns: 0 on success, negative on failure
 */
static inline int qdf_ipa_wdi_release_smmu_mapping(uint32_t num_buffers,
		qdf_ipa_wdi_buffer_info_t *info)
{
	return __qdf_ipa_wdi_release_smmu_mapping(num_buffers, info);
}

#ifdef WDI3_STATS_UPDATE
/**
 * qdf_ipa_wdi_wlan_stats() - Client should call this function to
 *		send Tx byte counts to IPA driver
 * @tx_count: number of Tx bytes
 *
 * Returns: 0 on success, negative on failure
 */
static inline int qdf_ipa_wdi_wlan_stats(qdf_ipa_wdi_tx_info_t *tx_stats)
{
	return __qdf_ipa_wdi_wlan_stats(tx_stats);
}

/**
 * qdf_ipa_uc_bw_monitor() - start/stop uc bw monitoring
 * @bw_info: set bw info levels to monitor
 *
 * Returns: 0 on success, negative on failure
 */
static inline int qdf_ipa_uc_bw_monitor(qdf_ipa_wdi_bw_info_t *bw_info)
{
	return __qdf_ipa_uc_bw_monitor(bw_info);
}
#endif

#endif /* IPA_OFFLOAD */
#endif /* _QDF_IPA_WDI3_H */
