/* SPDX-License-Identifier: GPL-2.0-only */
/* Copyright (c) 2018-2020, The Linux Foundation. All rights reserved.
 */
#ifndef LPASS_CDC_WSA2_MACRO_H
#define LPASS_CDC_WSA2_MACRO_H

/*
 * Selects compander and smart boost settings
 * for a given speaker mode
 */
enum {
	LPASS_CDC_WSA2_MACRO_SPKR_MODE_DEFAULT,
	LPASS_CDC_WSA2_MACRO_SPKR_MODE_1, /* COMP Gain = 12dB, Smartboost Max = 5.5V */
};

/* Rx path gain offsets */
enum {
	LPASS_CDC_WSA2_MACRO_GAIN_OFFSET_M1P5_DB,
	LPASS_CDC_WSA2_MACRO_GAIN_OFFSET_0_DB,
};

#endif
