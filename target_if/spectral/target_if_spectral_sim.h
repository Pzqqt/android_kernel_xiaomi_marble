/*
 * Copyright (c) 2015,2017-2018 The Linux Foundation. All rights reserved.
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
 * target_if_spectral_sim_attach() - Initialize Spectral Simulation
 * functionality
 * @spectral - pointer to spectral internal data structure
 *
 * Setup data structures to be used for serving out data corresponding to
 * various bandwidths and configurations.
 *
 * Return: Integer status value. 0:Success, -1:Failure
 */
int target_if_spectral_sim_attach(struct target_if_spectral *spectral);

/**
 * target_if_spectral_sim_detach() - De-initialize Spectral Simulation
 * functionality
 * @spectral - pointer to spectral internal data structure
 *
 * Free up data structures used for serving out data corresponding to various
 * bandwidths and configurations.
 *
 * Return: None
 */
void target_if_spectral_sim_detach(struct target_if_spectral *spectral);

/**
 * target_if_spectral_sops_sim_is_active() - Check if Spectral(simulated) is
 * active
 * @arg - pointer to spectral internal data structure
 *
 * Check if Spectral (simulated) is active
 *
 * Return: Integer status value. 0: Not active, 1: Active
 */
uint32_t target_if_spectral_sops_sim_is_active(void *arg);

/**
 * target_if_spectral_sops_sim_is_enabled() - Check if Spectral(simulated) is
 * enabled
 * @arg - pointer to spectral internal data structure
 *
 * Check if Spectral(simulated) is enabled
 *
 * Return: Integer status value. 0: Not enabled, 1: Enabled
 */
uint32_t target_if_spectral_sops_sim_is_enabled(void *arg);

/**
 * target_if_spectral_sops_sim_start_scan() - Start Spectral simulation
 * @arg - pointer to spectral internal data structure
 *
 * Start Spectral simulation
 *
 * Return: Integer status value. 0: Failure, 1: Success
 */
uint32_t target_if_spectral_sops_sim_start_scan(void *arg);

/**
 * target_if_spectral_sops_sim_stop_scan() - Stop Spectral simulation
 * @arg - pointer to spectral internal data structure
 *
 * Stop Spectral simulation
 *
 * Return: Integer status value. 0: Failure, 1: Success
 */
uint32_t target_if_spectral_sops_sim_stop_scan(void *arg);

/**
 * target_if_spectral_sops_sim_configure_params() - Configure Spectral
 * parameters into simulation
 * arg - pointer to ath_spectral structure
 * params - pointer to struct spectral_config structure bearing Spectral
 *          configuration
 *
 *  Internally, this function actually searches if a record set with the desired
 *  configuration has been loaded. If so, it points to the record set for
 *  later usage when the simulation is started. If not, it returns an error.
 *
 * Return: Integer status value. 0: Failure, 1: Success
 */
uint32_t target_if_spectral_sops_sim_configure_params(
	void *arg,
	struct spectral_config *params);

/**
 * target_if_spectral_sops_sim_get_params() - Get Spectral parameters configured
 * into simulation
 * arg - pointer to ath_spectral structure
 * params - pointer to struct spectral_config structure which should be
 *          populated with Spectral configuration
 *
 * Get Spectral parameters configured into simulation
 *
 * Return: Integer status value. 0: Failure, 1: Success
 */
uint32_t target_if_spectral_sops_sim_get_params(
	void *arg,
	struct spectral_config *params);

#endif				/* QCA_SUPPORT_SPECTRAL_SIMULATION */
#endif				/* _SPECTRAL_SIM_H_ */
