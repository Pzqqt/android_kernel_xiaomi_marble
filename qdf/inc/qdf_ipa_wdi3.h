/*
 * Copyright (c) 2017, The Linux Foundation. All rights reserved.
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
 * This file provides OS abstraction for IPA WDI3 APIs.
 */

#ifndef _QDF_IPA_WDI3_H
#define _QDF_IPA_WDI3_H

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

#endif
