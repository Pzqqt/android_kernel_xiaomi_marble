// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2016-2017, 2020 The Linux Foundation. All rights reserved.
 */

#ifndef __SDE_ENCODER_DCE_H__
#define __SDE_ENCODER_DCE_H__

#include "sde_encoder.h"

bool sde_encoder_dsc_is_dirty(struct sde_encoder_virt *sde_enc);

void sde_encoder_dsc_disable(struct sde_encoder_virt *sde_enc);

int sde_encoder_dsc_setup(struct sde_encoder_virt *sde_enc,
		struct sde_encoder_kickoff_params *params);

void sde_encoder_dsc_helper_flush_dsc(struct sde_encoder_virt *sde_enc);

#endif /* __SDE_ENCODER_DCE_H__ */
