/*
 * Copyright (c) 2015,2017 The Linux Foundation. All rights reserved.
 *
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

#ifndef _SPECTRAL_SIM_H_
#define _SPECTRAL_SIM_H_

#ifdef QCA_SUPPORT_SPECTRAL_SIMULATION
#include "target_if_spectral.h"

/**
 * @brief Initialize Spectral Simulation functionality
 * @details
 *  Setup data structures to be used for serving out data corresponding to
 *  various bandwidths and configurations.
 *
 * @param spectral - ath_spectral structure
 * @return Integer status value. 0:Success, -1:Failure
 */
int target_if_spectral_sim_attach(struct target_if_spectral *spectral);

/**
 * @brief De-initialize Spectral Simulation functionality
 * @details
 *  Free up data structures used for serving out data corresponding to various
 *  bandwidths and configurations.
 *
 * @param spectral - ath_spectral structure
 */
void target_if_spectral_sim_detach(struct target_if_spectral *spectral);

/**
 * @brief Check if Spectral (simulated) is active
 *
 * @param arg - pointer to ath_spectral structure
 * @return Integer status value. 0: Not active, 1: Active
 */
u_int32_t tif_spectral_sim_is_spectral_active(void *arg);

/**
 * @brief Check if Spectral (simulated) is enabled
 *
 * @param arg - pointer to ath_spectral structure
 * @return Integer status value. 0: Not enabled, 1: Enabled
 */
u_int32_t tif_spectral_sim_is_spectral_enabled(void *arg);

/**
 * @brief Start Spectral simulation
 *
 * @param arg - pointer to ath_spectral structure
 * @return Integer status value. 0: Failure, 1: Success
 */
u_int32_t tif_spectral_sim_start_spectral_scan(void *arg);

/**
 * @brief Stop Spectral simulation
 *
 * @param arg - pointer to ath_spectral structure
 * @return Integer status value. 0: Failure, 1: Success
 */
u_int32_t tif_spectral_sim_stop_spectral_scan(void *arg);

/**
 * @brief Configure Spectral parameters into simulation
 * @details
 *  Internally, this function actually searches if a record set with the desired
 *  configuration has been loaded. If so, it points to the record set for
 *  later usage when the simulation is started. If not, it returns an error.
 *
 * @param arg - pointer to ath_spectral structure
 * @param params - pointer to struct spectral_config structure bearing Spectral
 *                 configuration
 * @return Integer status value. 0: Failure, 1: Success
 */
u_int32_t tif_spectral_sim_configure_params(
	void *arg,
	 struct spectral_config *params);

/**
 * @brief Get Spectral parameters configured into simulation
 *
 * @param arg - pointer to ath_spectral structure
 * @param params - pointer to struct spectral_config structure which should be
 *                 populated with Spectral configuration
 * @return Integer status value. 0: Failure, 1: Success
 */
u_int32_t tif_spectral_sim_get_params(
	void *arg,
	 struct spectral_config *params);

#endif /* QCA_SUPPORT_SPECTRAL_SIMULATION */
#endif /* _SPECTRAL_SIM_H_ */
