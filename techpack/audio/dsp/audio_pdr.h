/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (c) 2016-2018, 2020 The Linux Foundation. All rights reserved.
 */

#ifndef __AUDIO_PDR_H_
#define __AUDIO_PDR_H_

#include <linux/soc/qcom/pdr.h>

enum {
	AUDIO_PDR_DOMAIN_ADSP,
	AUDIO_PDR_DOMAIN_MAX
};

#ifdef CONFIG_MSM_QDSP6_PDR

/*
 * Use audio_pdr_service_register to register with a PDR service
 * Function should be called after nb callback registered with
 * audio_pdr_register has been called back with the
 * AUDIO_PDR_FRAMEWORK_UP ioctl.
 *
 * domain_id - Domain to use, example: AUDIO_PDR_ADSP
 * *cb - Pointer to a callback function that will be notified of the state
 *       of the domain requested. The ioctls received by the callback are
 *       defined in pdr.h.
 *
 * Returns: Success: Client handle
 *          Failure: Pointer error code
 */
void *audio_pdr_service_register(int domain_id, void (*cb)(int, char *, void *));

/*
 * Use audio_pdr_service_deregister to deregister with a PDR
 * service that was registered using the audio_pdr_service_register
 * API.
 *
 * domain_id - Domain to use, example: AUDIO_PDR_ADSP
 *
 * Returns: Success: zero
 *          Failure: Error code
 */
int audio_pdr_service_deregister(int domain_id);

#else

static inline void *audio_pdr_service_register(int domain_id, void (*cb)(int, char *, void *))
{
	return NULL;
}

static inline int audio_pdr_service_deregister(int domain_id)
{
	return 0;
}

#endif /* CONFIG_MSM_QDSP6_PDR */

#endif
