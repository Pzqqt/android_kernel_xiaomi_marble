/*
 * Copyright (c) 2011-2019 The Linux Foundation. All rights reserved.
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

/*
 * DO NOT EDIT - This file is generated automatically
 */

/*
 * IMPORTANT:  This file is for system that supports STA mode ONLY.
 */
#include "cfg_priv.h"

/**
 * cfg_get_string() - return string conversion of HE WNI CFG
 * @cfg_id: Config ID.
 *
 * This utility function helps log string conversion of WNI config ID.
 *
 * Return: string conversion of the HE WNI config ID, if match found;
 *	"Invalid" otherwise.
 */
const char *cfg_get_string(uint16_t cfg_id)
{
	switch (cfg_id) {
	default:
		break;
	CASE_RETURN_STRING(WNI_CFG_STA_ID);
	CASE_RETURN_STRING(WNI_CFG_VALID_CHANNEL_LIST);
	CASE_RETURN_STRING(WNI_CFG_APSD_ENABLED);
	CASE_RETURN_STRING(WNI_CFG_CURRENT_TX_POWER_LEVEL);
	CASE_RETURN_STRING(WNI_CFG_COUNTRY_CODE);
	CASE_RETURN_STRING(WNI_CFG_LOCAL_POWER_CONSTRAINT);
	CASE_RETURN_STRING(WNI_CFG_ENABLE_MCC_ADAPTIVE_SCHED);
	CASE_RETURN_STRING(WNI_CFG_PS_WOW_DATA_INACTIVITY_TIMEOUT);
	CASE_RETURN_STRING(WNI_CFG_OBSS_DETECTION_OFFLOAD);
	CASE_RETURN_STRING(WNI_CFG_OBSS_COLOR_COLLISION_OFFLOAD);
	}

	return "invalid";
}
