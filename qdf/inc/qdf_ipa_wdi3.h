/*
 * Copyright (c) 2017-2018, The Linux Foundation. All rights reserved.
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

#ifdef CONFIG_IPA_WDI_UNIFIED_API

#include <qdf_ipa.h>
#include <i_qdf_ipa_wdi3.h>

/**
 * qdf_ipa_wdi_version_t - IPA WDI version
 */
typedef __qdf_ipa_wdi_version_t qdf_ipa_wdi_version_t;

/**
 * qdf_ipa_wdi_init_in_params_t - wdi init input parameters
 */
typedef __qdf_ipa_wdi_init_in_params_t qdf_ipa_wdi_init_in_params_t;

/**
 * qdf_ipa_wdi_init_out_params_t - wdi init output parameters
 */
typedef __qdf_ipa_wdi_init_out_params_t qdf_ipa_wdi_init_out_params_t;

/**
 * qdf_ipa_wdi_pipe_setup_info_smmu_t - WDI TX/Rx configuration
 */
typedef __qdf_ipa_wdi_pipe_setup_info_smmu_t qdf_ipa_wdi_pipe_setup_info_smmu_t;

typedef __qdf_ipa_ep_cfg_t qdf_ipa_ep_cfg_t;

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

/**
 * qdf_ipa_wdi_hdr_info_t - Header to install on IPA HW
 */
typedef __qdf_ipa_wdi_hdr_info_t qdf_ipa_wdi_hdr_info_t;

/**
 * qdf_ipa_wdi_reg_intf_in_params_t - parameters for uC offload
 *	interface registration
 */
typedef __qdf_ipa_wdi_reg_intf_in_params_t qdf_ipa_wdi_reg_intf_in_params_t;

/**
 * qdf_ipa_wdi_pipe_setup_info_t - WDI TX/Rx configuration
 */
typedef __qdf_ipa_wdi_pipe_setup_info_t qdf_ipa_wdi_pipe_setup_info_t;

/**
 * qdf_ipa_wdi_conn_in_params_t - information provided by
 *		uC offload client
 */
typedef __qdf_ipa_wdi_conn_in_params_t qdf_ipa_wdi_conn_in_params_t;

/**
 * qdf_ipa_wdi_conn_out_params_t - information provided
 *				to WLAN druver
 */
typedef __qdf_ipa_wdi_conn_out_params_t qdf_ipa_wdi_conn_out_params_t;

/**
 * qdf_ipa_wdi_perf_profile_t - To set BandWidth profile
 */
typedef __qdf_ipa_wdi_perf_profile_t qdf_ipa_wdi_perf_profile_t;

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

#else /* CONFIG_IPA_WDI_UNIFIED_API */

#include <i_qdf_ipa_wdi3.h>

/**
 * qdf_ipa_wdi3_hdr_info_t - Header to install on IPA HW
 */
typedef __qdf_ipa_wdi3_hdr_info_t qdf_ipa_wdi3_hdr_info_t;

/**
 * qdf_ipa_wdi3_reg_intf_in_params_t - parameters for uC offload
 *	interface registration
 */
typedef __qdf_ipa_wdi3_reg_intf_in_params_t qdf_ipa_wdi3_reg_intf_in_params_t;

/**
 * qdf_ipa_wdi3_setup_info_t - WDI3 TX/Rx configuration
 */
typedef __qdf_ipa_wdi3_setup_info_t qdf_ipa_wdi3_setup_info_t;

/**
 * qdf_ipa_wdi3_conn_in_params_t - information provided by
 *		uC offload client
 */
typedef __qdf_ipa_wdi3_conn_in_params_t qdf_ipa_wdi3_conn_in_params_t;

/**
 * qdf_ipa_wdi3_conn_out_params_t - information provided
 *				to WLAN druver
 */
typedef __qdf_ipa_wdi3_conn_out_params_t qdf_ipa_wdi3_conn_out_params_t;

/**
 * qdf_ipa_wdi3_perf_profile_t - To set BandWidth profile
 */
typedef __qdf_ipa_wdi3_perf_profile_t qdf_ipa_wdi3_perf_profile_t;

/**
 * qdf_ipa_wdi3_reg_intf - Client should call this function to
 * init WDI3 IPA offload data path
 *
 * Note: Should not be called from atomic context and only
 * after checking IPA readiness using ipa_register_ipa_ready_cb()
 *
 * @Return 0 on success, negative on failure
 */
static inline int qdf_ipa_wdi3_reg_intf(
	struct ipa_wdi3_reg_intf_in_params *in)
{
	return __qdf_ipa_wdi3_reg_intf(in);
}

/**
 * qdf_ipa_wdi3_dereg_intf - Client Driver should call this
 * function to deregister before unload and after disconnect
 *
 * @Return 0 on success, negative on failure
 */
static inline int qdf_ipa_wdi3_dereg_intf(const char *netdev_name)
{
	return __qdf_ipa_wdi3_dereg_intf(netdev_name);
}

/**
 * qdf_ipa_wdi3_conn_pipes - Client should call this
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
static inline int qdf_ipa_wdi3_conn_pipes(struct ipa_wdi3_conn_in_params *in,
			struct ipa_wdi3_conn_out_params *out)
{
	return __qdf_ipa_wdi3_conn_pipes(in, out);
}

/**
 * qdf_ipa_wdi3_disconn_pipes() - Client should call this
 *		function to disconnect pipes
 *
 * Note: Should not be called from atomic context
 *
 * Returns: 0 on success, negative on failure
 */
static inline int qdf_ipa_wdi3_disconn_pipes(void)
{
	return __qdf_ipa_wdi3_disconn_pipes();
}

/**
 * qdf_ipa_wdi3_enable_pipes() - Client should call this
 *		function to enable IPA offload data path
 *
 * Note: Should not be called from atomic context
 *
 * Returns: 0 on success, negative on failure
 */
static inline int qdf_ipa_wdi3_enable_pipes(void)
{
	return __qdf_ipa_wdi3_enable_pipes();
}

/**
 * qdf_ipa_wdi3_disable_pipes() - Client should call this
 *		function to disable IPA offload data path
 *
 * Note: Should not be called from atomic context
 *
 * Returns: 0 on success, negative on failure
 */
static inline int qdf_ipa_wdi3_disable_pipes(void)
{
	return __qdf_ipa_wdi3_disable_pipes();
}

/**
 * qdf_ipa_wdi3_set_perf_profile() - Client should call this function to
 *		set IPA clock bandwidth based on data rates
 *
 * @profile: [in] BandWidth profile to use
 *
 * Returns: 0 on success, negative on failure
 */
static inline int qdf_ipa_wdi3_set_perf_profile(
			struct ipa_wdi3_perf_profile *profile)
{
	return __qdf_ipa_wdi3_set_perf_profile(profile);
}

#endif /* CONFIG_IPA_WDI_UNIFIED_API */

#endif /* IPA_OFFLOAD */
#endif /* _QDF_IPA_WDI3_H */
