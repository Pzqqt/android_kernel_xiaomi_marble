/*
 * Copyright (c) 2012-2018 The Linux Foundation. All rights reserved.
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

/**
 * DOC: This file contains centralized definitions of converged configuration.
 */

#ifndef __CFG_MLME_FE_WMM_H
#define __CFG_MLME_FE_WMM_H

#define CFG_QOS_ENABLED CFG_BOOL( \
		"qos_enabled", \
		0, \
		"QOS Enabled")

#define CFG_WME_ENABLED CFG_BOOL( \
		"wme_enabled", \
		1, \
		"WME Enabled")

#define CFG_MAX_SP_LENGTH CFG_UINT( \
		"max_sp_length", \
		0, \
		3, \
		0, \
		CFG_VALUE_OR_DEFAULT, \
		"MAX sp length")

#define CFG_WSM_ENABLED CFG_BOOL( \
		"wsm_enabled", \
		0, \
		"WSM Enabled")

#define CFG_EDCA_PROFILE CFG_UINT( \
		"edca_profile", \
		0, \
		4, \
		1, \
		CFG_VALUE_OR_DEFAULT, \
		"Edca Profile")

#define CFG_WMM_PARAMS_ALL \
	CFG(CFG_QOS_ENABLED) \
	CFG(CFG_WME_ENABLED) \
	CFG(CFG_MAX_SP_LENGTH) \
	CFG(CFG_WSM_ENABLED) \
	CFG(CFG_EDCA_PROFILE)

#endif /* __CFG_MLME_FE_WMM_H */
